# This node is an example of a simple publisher monitors the input from the user and publishes commands to the "arduino/send" topic.

# Import Python Native Modules. We require the Barrier class from threading to synchronize the start of multiple threads.
from threading import Barrier
import signal

# Import the required pubsub modules. PubSubMsg class to extract the payload from a message.
from pubsub.pub_sub_manager import ManagedPubSubRunnable, PubSubMsg
from pubsub.pub_sub_manager import publish, subscribe, unsubscribe, getMessages, getCurrentExecutionContext

# Import the command parser from the control module
from control.alex_control import parseUserInput, waitForHelloRoutine
from control.alex_control_constants import *

import getch
import select
import sys
import tty
import termios

# Constants
ARDUINO_SEND_TOPIC = "arduino/send"
ARDUINO_RECEIVE_TOPIC = "arduino/recv"


def cliThread(setupBarrier:Barrier=None, readyBarrier:Barrier=None):
    """
    Starts a command thread that interacts with the user. Publishes commands to the "arduino/send" topic for the send thread to handle sending the commands to the Arduino.
    
    Args:
        setupBarrier (Barrier, optional): A threading barrier to synchronize initial setup steps. Defaults to None.
        readyBarrier (Barrier, optional): A threading barrier to synchronize the start of the thread. Defaults to None.
    
    The function performs the following steps:
    1. Sets up the execution context.
    2. Waits for setup to complete if barriers are provided.
    3. Initiates a user interaction loop to receive and parse commands.
    4. Exits gracefully when an exit condition is met.

    Note:
        input is a blocking call, so the thread will wait for user input before proceeding. This means that even if the exit condition is met while waiting for input, the thread remains blocked until input is received (i.e., the user enters a command).

    This provided function has been heavily modified to accept keypresses as commands instead of using the default
    "f 10 50" commands. Using the getch library on Linux, keypresses sent to the terminal can be captured, and the
    keypresses are decoded and corresponding packets are published to the Arduino Send Node.
    There are also many checks to ensure only 1 packet is being sent at a time, and that the Arduino has acknowledged
    the sent packets before accepting another keypress. There is also a bypass key '=' to bypass all the checks in case
    the Arduino returns any error instead of an acknowledgement, as a soft-reset method instead of restarting the
    Arduino by disconnecting and reconnecting the Serial port.
    """

    # Perform any setup here
    pass
    ctx: ManagedPubSubRunnable = getCurrentExecutionContext()

    subscribe(topic=ARDUINO_RECEIVE_TOPIC, ensureReply=True, replyTimeout=1)

    # Perform any setup here
    setupBarrier.wait() if readyBarrier is not None else None

    print(f"CLI Thread Ready. Publishing to {ARDUINO_SEND_TOPIC}")

    # Wait for all Threads ready
    readyBarrier.wait() if readyBarrier is not None else None

    # Dictionary of values to be mapped
    distance_map = {'4': "2", '5': "6", '6': "10"}
    angle_map = {'7': "7", '8': "25", '9': "85"}

    # Variables used for checking
    sent_packet = False             # To ensure only 1 packet is sent to Arduino Send Node
    sent_command = False            # To check if the command has been sent to the other Arduino Send Node
    num_messages = 0                # To check how many acknowledgement/error messages the Arduino has sent back

    # User Interaction Loop
    try:
        while not ctx.isExit():
            # Old, provided code:
            # input_str = input("Command (f=forward, b=reverse, l=turn left, r=turn right, s=stop, c=clear stats, g=get stats q=exit)\n")
            # parseResult = parseUserInput(input_str, exitFlag=ctx.exitEvent)

            # New, with key bindings:

            # Forming a new packet every loop
            packet_type = TPacketType.PACKET_TYPE_COMMAND
            command_type = None
            t_list = [0] * 16

            # To count the number of packets received before allowing the keybind checking to occur
            if sent_packet:
                messages = getMessages(block=False)     # This adds one every time the Arduino sends a packet/message
                if messages:
                    num_messages += 1

                # Most commands have 2 acknowledgement messages, color has a third message that returns the
                # identified color as a separate packet
                if num_messages >= 2:
                    num_messages = 0
                    sent_command = False
                    sent_packet = False
            
            if not kbhit():     # To make the loop faster if no key has been pressed
                continue

            try:                # If a key has been pressed, find what key has been pressed
                ch = getch.getch()
                key = ch.lower()
            except KeyboardInterrupt:
                break

            # In case the helloRoutine is not sent as part of the auto-reconnecting as part of the Arduino Receive
            # Node, this key executes the Hello Routine if necessary
            if key == '.' and not sent_packet:
                waitForHelloRoutine()
                continue

            #####################################
            # FORWARD commands, different distances
            #####################################

            elif key == 'q' and not sent_packet:
                move_distance = distance_map['4']
                print(f"\rMove forward {move_distance}cm", flush=True)
                command_type = TCommandType.COMMAND_FORWARD
                t_list[0] = move_distance
                sent_command = True

            elif key == 'w' and not sent_packet:
                move_distance = distance_map['5']
                print(f"\rMove forward {move_distance}cm", flush=True)
                command_type = TCommandType.COMMAND_FORWARD
                t_list[0] = move_distance
                sent_command = True

            elif key == 'e' and not sent_packet:
                move_distance = distance_map['6']
                print(f"\rMove forward {move_distance}cm", flush=True)
                command_type = TCommandType.COMMAND_FORWARD
                t_list[0] = move_distance
                sent_command = True

            #####################################
            # REVERSE commands, different distances
            #####################################

            elif key == 'a' and not sent_packet:
                move_distance = distance_map['4']
                print(f"\rMove reverse {move_distance}cm", flush=True)
                command_type = TCommandType.COMMAND_REVERSE
                t_list[0] = move_distance
                sent_command = True

            elif key == 's' and not sent_packet:
                move_distance = distance_map['5']
                print(f"\rMove reverse {move_distance}cm", flush=True)
                command_type = TCommandType.COMMAND_REVERSE
                t_list[0] = move_distance
                sent_command = True

            elif key == 'd' and not sent_packet:
                move_distance = distance_map['6']
                print(f"\rMove reverse {move_distance}cm", flush=True)
                command_type = TCommandType.COMMAND_REVERSE
                t_list[0] = move_distance
                sent_command = True

            #####################################
            # DIRECTION commands, different distances
            #####################################

            # TURN LEFT
            elif key == 'y' and not sent_packet:
                turn = angle_map['7']
                print(f"\rTurn left {turn} degrees", flush=True)
                command_type = TCommandType.COMMAND_TURN_LEFT
                t_list[0] = turn
                sent_command = True
    
            elif key == 'h' and not sent_packet:
                turn = angle_map['8']
                print(f"\rTurn left {turn} degrees", flush=True)
                command_type = TCommandType.COMMAND_TURN_LEFT
                t_list[0] = turn
                sent_command = True
    
            elif key == 'n' and not sent_packet:
                turn = angle_map['9']
                print(f"\rTurn left {turn} degrees", flush=True)
                command_type = TCommandType.COMMAND_TURN_LEFT
                t_list[0] = turn
                sent_command = True

            # TURN RIGHT					
            elif key == 'u' and not sent_packet:
                turn = angle_map['7']
                print(f"\rTurn right {turn} degrees", flush=True)
                command_type = TCommandType.COMMAND_TURN_RIGHT
                t_list[0] = turn
                sent_command = True
    
            elif key == 'j' and not sent_packet:
                turn = angle_map['8']
                print(f"\rTurn right {turn} degrees", flush=True)
                command_type = TCommandType.COMMAND_TURN_RIGHT
                t_list[0] = turn
                sent_command = True
    
            elif key == 'm' and not sent_packet:
                turn = angle_map['9']
                print(f"\rTurn right {turn} degrees", flush=True)
                command_type = TCommandType.COMMAND_TURN_RIGHT
                t_list[0] = turn
                sent_command = True		
            
            # STOP
            elif key == 'z' and not sent_packet:
                print(f"\rStop!!!", flush=True)
                command_type = TCommandType.COMMAND_STOP
                t_list[0] = 0
                t_list[1] = 0
                sent_command = True
        
            # Color sensor
            elif key == 'c' and not sent_packet:
                print(f"\rRead Colour Sensor:", flush=True)
                command_type = TCommandType.COMMAND_COLOR
                t_list[0] = 0
                t_list[1] = 0
                sent_command = True

            #####################################
            # Claw commands, open/close
            #####################################
        
            # Open Claw
            elif key == 'o' and not sent_packet:
                print(f"\rOpen Claw", flush=True)
                command_type = TCommandType.COMMAND_OPEN
                t_list[0] = 0
                t_list[1] = 0
                sent_command = True
        
            # Close Claw
            elif key == 'p' and not sent_packet:
                print(f"\rClose Claw", flush=True)
                command_type = TCommandType.COMMAND_CLOSE
                t_list[0] = 0
                t_list[1] = 0 
                sent_command = True

            #####################################
            # LCD commands, different messages
            #####################################

            elif key == '5' and not sent_packet:
                print(f"\rShow LCD 1", flush=True)
                command_type = TCommandType.COMMAND_SCREEN
                t_list[0] = 1
                t_list[1] = 0
                sent_command = True
            
            elif key == '6' and not sent_packet:
                print(f"\rShow LCD 2", flush=True)
                command_type = TCommandType.COMMAND_SCREEN
                t_list[0] = 2
                t_list[1] = 0
                sent_command = True
            
            elif key == '7' and not sent_packet:
                print(f"\rShow LCD 3", flush=True)
                command_type = TCommandType.COMMAND_SCREEN
                t_list[0] = 3
                t_list[1] = 0
                sent_command = True
            
            elif key == '8' and not sent_packet:
                print(f"\rShow LCD 4", flush=True)
                command_type = TCommandType.COMMAND_SCREEN
                t_list[0] = 4
                t_list[1] = 0
                sent_command = True

            # Bypass keybind, sets all boolean check variables to default state
            if key == '=':
                print("\rBypassing", flush=True)
                sent_command = False
                sent_packet = False

            # Quit keybind, allows all the nodes to quit gracefully
            if key == ']':
                print("\rQuit", flush=True)
                break

            # If a command has been sent by the operator but the packet has not been sent, parse the packet
            if sent_command and not sent_packet:
                # Build parseResult
                t_list[1] = 85          # Default power value of 85 for consistency
                parseResult = (packet_type, command_type, t_list)
                sent_packet = True
            else:
                parseResult = None
            
            # if the parse result is None then the user entered an invalid command
            if parseResult is None:
                # print("Invalid command. Please try again.")
                continue
            else:
                # if the parse result is not None then the user entered a valid command
                # and the command has been published to the "arduino/send" topic
                publish(ARDUINO_SEND_TOPIC, tuple(parseResult))
            # [Optional: Consider enforcing the user to wait for the arduino to respond before sending the next command]
        pass

    except KeyboardInterrupt:
        pass
    except Exception as e:
        print(f"\rCLI Thread Exception: {e}")
        pass

    # The getch function messes up the formatting of the terminal printing, this is to revert any changes
    print("\rRestoring", flush=True)
    termios.tcsetattr(sys.stdin.fileno(), termios.TCSADRAIN, termios.tcgetattr(sys.stdin.fileno()))

    # Shutdown and exit the thread gracefully
    ctx.doExit()
    print("\rExiting Command Thread")
    pass


def kbhit():
    """
    Function to check if a key has been pressed
    """
    fd = sys.stdin.fileno()
    old_settings = termios.tcgetattr(fd)
    try:
        tty.setraw(fd)
        dr, dw, de = select.select([sys.stdin], [], [], 0)
        return dr != []
    finally:
        termios.tcsetattr(fd, termios.TCSADRAIN, old_settings)
