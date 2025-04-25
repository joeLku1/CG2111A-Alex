# This node is responsible for receiving the LIDAR data (in JSON form) from the RPi over the TCP websocket
# And publishing it to the lidar/scan topic, simulating how the LIDAR data would run on the RPi locally

import json
import socket

from threading import Barrier

from pubsub.pub_sub_manager import ManagedPubSubRunnable, PubSubMsg
from pubsub.pub_sub_manager import publish, subscribe, unsubscribe, getMessages, getCurrentExecutionContext

# Define host and port
HOST = '0.0.0.0'  # Listen on all available interfaces
PORT = 8765       # Change this to your desired port number

LIDAR_SCAN_TOPIC = "lidar/scan"


def createSocket(HOST, PORT):
    """
    Starts a new TCP listener at the specified host and port
    Args:
        HOST: IP address of the RPi
        PORT: Default 8765
    Returns: Opened socket object
    """
    # Create socket
    print(f"Connecting to server at {HOST}:{PORT}")
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    client_socket.connect((HOST, PORT))
    print(f"Connected to server at {HOST}:{PORT}")
    return client_socket


def dataThread(setupBarrier: Barrier=None, readyBarrier: Barrier=None):
    ctx: ManagedPubSubRunnable = getCurrentExecutionContext()
    setupBarrier.wait() if readyBarrier is not None else None
    readyBarrier.wait() if readyBarrier is not None else None

    try:
        client_socket = createSocket("192.168.128.48", 8765)
        buffer = b''
        packet = b''
        lidar_data = None
        while not ctx.isExit():
            chunk = client_socket.recv(1024)
            if not chunk:
                break
            buffer += chunk

            # Process data when a complete packet is detected
            while b'\n' in buffer:
                packet, buffer = buffer.split(b'\n', 1)  # Extract JSON, leave excess in buffer

            try:
                data_str = packet.decode('utf-8').strip()
                lidar_data = json.loads(data_str)
                print(f"Received LIDAR data: {len(lidar_data)}")
                print(f"Received lidar data1: {len(lidar_data[0])}")
                print(f"Received lidar data1: {len(lidar_data[1])}")
                print(f"Received lidar data1: {len(lidar_data[2])}")
                publish(LIDAR_SCAN_TOPIC, lidar_data)                   # no filter
            except json.JSONDecodeError as e:
                print(f"Error decoding JSON: {e}")

    except KeyboardInterrupt:
        pass
    except Exception as e:
        print(f"Data Thread Exception: {e}")
        pass
    
    ctx.doExit()
    print("Exiting Data Thread")
    pass
