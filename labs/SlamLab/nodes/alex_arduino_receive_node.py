# This node is an example of a simple publisher that receives messages from the arduino and publish it on the ardunio/recv topic
# Currently does not do anything else with the messages.

# Import Python Native Modules. We require the Barrier class from threading to synchronize the start of multiple threads.
from threading import Barrier

# Import the required pubsub modules. PubSubMsg class to extract the payload from a message.
from pubsub.pub_sub_manager import ManagedPubSubRunnable, PubSubMsg
from pubsub.pub_sub_manager import publish, subscribe, unsubscribe, getMessages, getCurrentExecutionContext  

# Import the required arduino communication modules. Replace or add to the handlers as needed.
from control.alex_control import receivePacket, waitForHelloRoutine
from control.alex_control_constants import  TPacket, TPacketType, PAYLOAD_PARAMS_COUNT, PAYLOAD_PACKET_SIZE
from control.alex_control_constants import  TResponseType, TResultType

from control.alex_control_serial import startSerial, closeSerial
import time

# Constants
PUBLISH_PACKETS = True
ARDUINO_RECV_TOPIC = "arduino/recv" 

# Arduino Serial Constants
PORT_NAME = "/dev/ttyACM0"
# PORT_NAME = "/dev/ttyUSB0"
BAUD_RATE = 9600
BYTE_SIZE = 8
PARITY = "N"
STOP_BITS = 1
MAX_ATTEMPTS = 5
SERIAL_TIMEOUT = 1
FAILED_ATTEMPT_WAIT_SEC = 5


def startArduino():
	"""
	Function to open Serial Port of Arduino, if Arduino crashes and there is a need to reset it
	"""
	print("\r==============SETTING UP==============", flush=True)

	res = startSerial(
		portName=PORT_NAME, baudRate=BAUD_RATE, byteSize=BYTE_SIZE, parity=PARITY, stopBits=STOP_BITS, maxAttempts=MAX_ATTEMPTS,
		serialTimeout=SERIAL_TIMEOUT, failedAttemptWaitSec=FAILED_ATTEMPT_WAIT_SEC
	)
	if not res:
		print ("\rFailed to open serial port. Exiting...")
		return
	print("\rSerial OK", flush=True)
	
	# Wait for arduino to reset
	print("\rWAITING TWO SECONDS FOR ARDUINO TO REBOOT.", end="", flush=True)
	# make sure to flush the print buffer before sleeping
	time.sleep(2/3)
	print(".", end="",  flush=True)
	time.sleep(2/3)
	print(".", end="", flush=True)
	time.sleep(2/3)
	print("DONE",  flush=True)


def receiveThread(setupBarrier:Barrier=None, readyBarrier:Barrier=None):
	"""
	Thread function to handle receiving arduino packets in a loop until the context signals an exit.
	Args:
		setupBarrier (Barrier, optional): A threading barrier to synchronize the start of the thread setup.
		readyBarrier (Barrier, optional): A threading barrier to synchronize the thread start.
										  If provided, the thread will wait for all parties to be ready before proceeding.
	The function performs the following steps:
	1. Sets up the execution context.
	2. Waits for all threads to be ready if barriers are provided.
	3. Enters a loop to receive arduino packets until the context signals an exit.
	4. Processes packets based on their type:
		- Handles response packets.
		- Handles error response packets.
		- Handles message packets.
		- Logs unknown packet types.
	5. Gracefully shuts down and exits the thread.
	This function has been modified such that if the Arduino happens to disconnect from the RPi, this node does not crash
	Attempts to receive a packet from the Arduino is now enclosed in a try-except, such that if the Arduino Serial port
	has closed, an exception will be raised (which would kill the node originally), and the node attempts to reconnect
	to the Arduino by re-opening the Serial port.
	"""
	# Setup
	ctx: ManagedPubSubRunnable = getCurrentExecutionContext()

	# Perform any setup here
	setupBarrier.wait() if readyBarrier is not None else None
	
	# Nothing to do there
	print(f"Arduino Receive Thread Ready. Publish to {ARDUINO_RECV_TOPIC}? --> {PUBLISH_PACKETS}")   

	# Wait for all Threads ready
	readyBarrier.wait() if readyBarrier is not None else None

	# Receiving Logic Loop
	try:
		while not ctx.isExit():
			# Old provided code, causes all threads to crash on Arduino disconnect
			# packet = receivePacket(exitFlag=ctx.exitEvent)

			# Modified new code, if Arduino disconnect, it throws an error
			# Handles disconnect by attempting to reconnect the Arduino again using startArduino
			try: 
				packet = receivePacket(exitFlag=ctx.exitEvent)
			except Exception as e:
				print(f"Receive Thread Exception: {e}")
				closeSerial()
				print("Waiting for Serial reconnect...")
				time.sleep(2)
				startArduino()  # Start the Arduino
				waitForHelloRoutine()       # Do HelloRoutine on startup, if not Arduino does not listen for commands
				time.sleep(2)               # Wait for HelloRoutine to be complete, in case

			if packet is None:
				# Continue if no packet received
				continue

			# Handle the packet based on its type
			# Default handlers are provided in the control module
			# Pleasee modify or replace to fit the application requirements
			packetType = TPacketType(packet.packetType)
			
			if packetType == TPacketType.PACKET_TYPE_RESPONSE:
				handleResponse(packet, publishPackets = PUBLISH_PACKETS)
			elif packetType == TPacketType.PACKET_TYPE_ERROR:
				handleErrorResponse(packet, publishPackets = PUBLISH_PACKETS)
			elif packetType == TPacketType.PACKET_TYPE_MESSAGE:
				handleMessage(packet, publishPackets = PUBLISH_PACKETS)
			else:
				print(f"Unknown Packet Type {packetType}")
	except KeyboardInterrupt:
		pass
	except Exception as e:
		print(f"Receive Thread Exception: {e}")
		pass

	# Shutdown and exit the thread gracefully
	ctx.doExit()
	print("Exiting Receive Thread")
	pass


##########################
##### PACKET HANDLERS ####
##########################


def handleResponse(res: TPacket, publishPackets:bool=False):
	"""
	Handles the response from the Arduino.

	Args:
		res (TPacket): The response packet received from the Arduino.

	Returns:
		Tuple[TResponseType, Tuple[ctypes.c_uint32]]: A tuple containing the response type and parameters

	Prints:
		- "Command OK" if the response type is RESP_OK.
		- "Status OK" if the response type is RESP_STATUS.
		- "Arduino sent unknown response type {res_type}" for any other response type.
	"""
	res_type = TResponseType(res.command)
	if res_type == TResponseType.RESP_OK:
		print("\rCommand OK", flush=True)

		if publishPackets:
			publish("arduino/recv", (res.packetType, res.command ))

	elif res_type == TResponseType.RESP_STATUS:
		# we assume that the status if stored in the parameters
		# we will print the status
		params = tuple([p for p in res.params])
		status_str = ""
		for idx, p in enumerate(params):
			# We don't know what your parameters are, so we will just print them as is
			# You can modify this to fit your application
			param_name = "param" + str(idx)
			status_str += f"{param_name}: {p}\n"
		print(f"\rStatus OK: \n{status_str}", flush=True)
		
		if publishPackets:
			publish("arduino/recv", (res.packetType, res.command, params))
	else:
		print(f"Arduino sent unknown response type {res_type}")


def handleErrorResponse(res: TPacket , publishPackets:bool=False):
	"""
	Handles error responses from the Arduino.

	This function takes a TPacket object, determines the type of error response
	from the Arduino, and prints an appropriate error message.

	Args:
		res (TPacket): The packet received from the Arduino containing the error response.

	Raises:
		ValueError: If the response type is unknown.
	"""
	res_type = TResponseType(res.command)
	if res_type == TResponseType.RESP_BAD_PACKET:
		print("\rArduino received bad magic number", flush=True)
	elif res_type == TResponseType.RESP_BAD_CHECKSUM:
		print("\rArduino received bad checksum", flush=True)
	elif res_type == TResponseType.RESP_BAD_COMMAND:
		print("\rArduino received bad command", flush=True)
	elif res_type == TResponseType.RESP_BAD_RESPONSE:
		print("\rArduino received unexpected response", flush=True)
	else:
		print(f"\rArduino reports unknown error type {res_type}")

	if publishPackets:
		publish("arduino/recv", (res.packetType, res.command ))


def handleMessage(res:TPacket, publishPackets:bool=False):
	"""
	Handles the incoming message from an Arduino device.

	Args:
		res (TPacket): The packet received from the Arduino, containing the data to be processed.

	Returns:
		None
	"""
	# message = str(res.data, 'utf-8')
	message = str(res.data)
	print(f"\rArduino says: {message}", flush=True)
	if publishPackets:
		publish("arduino/recv", (res.packetType, res.command, message))
	pass
