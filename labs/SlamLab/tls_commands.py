import getch
import select
import sys
import tty
import termios
import struct

from control.alex_control_constants import *

import networking.sslClient as ss

host = "192.168.128.48"
# host = "192.168.137.242"
# host = "172.20.10.4"
port = 5000
clientKeyPath = "tls/laptop.key"
clientCertPath = "tls/laptop.crt"
caCertPath = "tls/signing.pem"
serverName = "mine.com"

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


if __name__ == "__main__":
    # This is a test script to run the TLS relay send node
    # It will connect to the TLS server and send messages over the connection
    # It will also handle any errors that occur during the connection
    # and will print the messages received from the server

    print("Hello!")
    
    # Setup server
    ss.disconnect()
    connected = False
    count = 0
    while not connected and count < 10:
        connected = ss.connect(host=host, port=port, client_cert_path=clientCertPath, client_key_path=clientKeyPath, ca_cert_path=caCertPath, server_canonical_name=serverName, timeout=2)
        count += 1
        print(f"\rConnecting attempt: {count}", flush=True)

    if not connected:
        print("\rFailed to connect to the TLS server.", flush=True)
        sys.exit(0)

    if connected:
        print("\rConnected to server", flush=True)
        print(ss.getTLSConnection())

    
    # Dictionary of values to be mapped
    distance_map = {'4': "2", '5': "6", '6': "10"}
    angle_map = {'7': "7", '8': "25", '9': "85"}

    # Variables used for checking
    sent_packet = False             # To ensure only 1 packet is sent to Arduino Send Node
    sent_command = False            # To check if the command has been sent to the other Arduino Send Node
    num_messages = 0                # To check how many acknowledgement/error messages the Arduino has sent back


    exitFlag = False
    # User Interaction Loop
    try:
        while not exitFlag:
            # Old, provided code:
            # input_str = input("Command (f=forward, b=reverse, l=turn left, r=turn right, s=stop, c=clear stats, g=get stats q=exit)\n")
            # parseResult = parseUserInput(input_str, exitFlag=ctx.exitEvent)

            # New, with key bindings:

            # Forming a new struct every loop
            cmdstruct = None


            # To count the number of packets received before allowing the keybind checking to occur
            if sent_packet:
                # messages = getMessages(block=False)     # This adds one every time the Arduino sends a packet/message
                messages = ss.recvNetworkData()
                if messages[0] is not None:
                    print(messages[0])
                    num_messages += 1
                if messages[0] is None:
                    print("\rNo messages, resetting\n", flush=True)
                    sent_command = False
                    sent_packet = False

                # Most commands have 2 acknowledgement messages, color has a third message that returns the
                # identified color as a separate packet
                if num_messages >= 2:
                    print("\rGot >= 2 messages, accepting new command\n", flush=True)
                    num_messages = 0
                    sent_command = False
                    sent_packet = False
            
            if not kbhit():     # To make the loop faster if no key has been pressed
                continue

            try:            # If a key has been pressed, find what key has been pressed
                ch = getch.getch()
                key = ch.lower()
            except KeyboardInterrupt:
                break

            #####################################
            # FORWARD commands, different distances
            #####################################

            if key == 'q' and not sent_packet:
                move_distance = distance_map['4']
                print(f"\rMove forward {move_distance}cm", flush=True)
                cmdstruct = struct.pack("=BIII", 3, 0, int(move_distance), 85)
                sent_command = True

            elif key == 'w' and not sent_packet:
                move_distance = distance_map['5']
                print(f"\rMove forward {move_distance}cm", flush=True)
                cmdstruct = struct.pack("=BIII", 3, 0, int(move_distance), 85)
                sent_command = True

            elif key == 'e' and not sent_packet:
                move_distance = distance_map['6']
                print(f"\rMove forward {move_distance}cm", flush=True)
                cmdstruct = struct.pack("=BIII", 3, 0, int(move_distance), 85)
                sent_command = True

            #####################################
            # REVERSE commands, different distances
            #####################################

            elif key == 'a' and not sent_packet:
                move_distance = distance_map['4']
                print(f"\rMove reverse {move_distance}cm", flush=True)
                cmdstruct = struct.pack("=BIII", 3, 1, int(move_distance), 85)
                sent_command = True

            elif key == 's' and not sent_packet:
                move_distance = distance_map['5']
                print(f"\rMove reverse {move_distance}cm", flush=True)
                cmdstruct = struct.pack("=BIII", 3, 1, int(move_distance), 85)
                sent_command = True

            elif key == 'd' and not sent_packet:
                move_distance = distance_map['6']
                print(f"\rMove reverse {move_distance}cm", flush=True)
                cmdstruct = struct.pack("=BIII", 3, 1, int(move_distance), 85)
                sent_command = True

            #####################################
            # DIRECTION commands, different distances
            #####################################

            # TURN LEFT
            elif key == 'y' and not sent_packet:
                turn = angle_map['7']
                print(f"\rTurn left {turn} degrees", flush=True)
                cmdstruct = struct.pack("=BIII", 3, 2, int(turn), 85)
                sent_command = True
    
            elif key == 'h' and not sent_packet:
                turn = angle_map['8']
                print(f"\rTurn left {turn} degrees", flush=True)
                cmdstruct = struct.pack("=BIII", 3, 2, int(turn), 85)
                sent_command = True
    
            elif key == 'n' and not sent_packet:
                turn = angle_map['9']
                print(f"\rTurn left {turn} degrees", flush=True)
                cmdstruct = struct.pack("=BIII", 3, 2, int(turn), 85)
                sent_command = True

            # TURN RIGHT					
            elif key == 'u' and not sent_packet:
                turn = angle_map['7']
                print(f"\rTurn right {turn} degrees", flush=True)
                cmdstruct = struct.pack("=BIII", 3, 3, int(turn), 85)
                sent_command = True
    
            elif key == 'j' and not sent_packet:
                turn = angle_map['8']
                print(f"\rTurn right {turn} degrees", flush=True)
                cmdstruct = struct.pack("=BIII", 3, 3, int(turn), 85)
                sent_command = True
    
            elif key == 'm' and not sent_packet:
                turn = angle_map['9']
                print(f"\rTurn right {turn} degrees", flush=True)
                cmdstruct = struct.pack("=BIII", 3, 3, int(turn), 85)
                sent_command = True		
            
            # STOP
            elif key == 'z' and not sent_packet:
                print(f"\rStop!!!", flush=True)
                cmdstruct = struct.pack("=BIII", 3, 4, 0, 0)
                sent_command = True
        
            # Color sensor
            elif key == 'c' and not sent_packet:
                print(f"\rRead Colour Sensor:", flush=True)
                cmdstruct = struct.pack("=BIII", 3, 7, 0, 0) 
                sent_command = True

            #####################################
            # Claw commands, open/close
            #####################################
        
            # Open Claw
            elif key == 'o' and not sent_packet:
                print(f"\rOpen Claw", flush=True)
                cmdstruct = struct.pack("=BIII", 3, 8, 0, 0)                
                sent_command = True
        
            # Close Claw
            elif key == 'p' and not sent_packet:
                print(f"\rClose Claw", flush=True)
                cmdstruct = struct.pack("=BIII", 3, 9, 0, 0)
                sent_command = True

            #####################################
            # LCD commands, different messages
            #####################################

            elif key == '5' and not sent_packet:
                print(f"\rShow LCD 1", flush=True)
                cmdstruct = struct.pack("=BIII", 3, 6, 1, 0)
                sent_command = True
            
            elif key == '6' and not sent_packet:
                print(f"\rShow LCD 2", flush=True)
                cmdstruct = struct.pack("=BIII", 3, 6, 2, 0)
                sent_command = True
            
            elif key == '7' and not sent_packet:
                print(f"\rShow LCD 3", flush=True)
                cmdstruct = struct.pack("=BIII", 3, 6, 3, 0)
                sent_command = True
            
            elif key == '8' and not sent_packet:
                print(f"\rShow LCD 4", flush=True)
                cmdstruct = struct.pack("=BIII", 3, 6, 4, 0)
                sent_command = True

            # Bypass keybind, sets all boolean check variables to default state
            if key == '=':
                print("\rBypassing", flush=True)
                sent_command = False
                sent_packet = False

            # Quit keybind, allows all the nodes to quit gracefully
            if key == ']':
                print("\rQuit", flush=True)
                exitFlag = True
                break
            
            if sent_command and not sent_packet:
                # print("Invalid command. Please try again.")
                ss.sendNetworkData(cmdstruct)
                sent_packet = True
            # [Optional: Consider enforcing the user to wait for the arduino to respond before sending the next command]
        pass

    except KeyboardInterrupt:
        exitFlag = True
        pass
    except Exception as e:
        print(f"\rException: {e}", flush=True)
        pass

    # The getch function messes up the formatting of the terminal printing, this is to revert any changes
    print("\rRestoring", flush=True)
    termios.tcsetattr(sys.stdin.fileno(), termios.TCSADRAIN, termios.tcgetattr(sys.stdin.fileno()))

    ss.disconnect()
    print("\rDisconnecting from TLS server", flush=True)
    print("\rExiting. Done.", flush=True)
