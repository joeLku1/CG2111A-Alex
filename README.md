# CG2111A Alex RPi Python Code
Code by Group B02-6A

Members:
- Low Zen Wei
- Ong Yi Siang Joel
- Rishabh Ramprasad Shenoy
- Joel Ku
- Jerry Andikko

For the Arduino and WSL code, check out the other branches of this repository.
<br><br>
# Overview
The RPi code is heavily modified from the original ```Alex.zip``` from Studio 19. The full list of modifications can be viewed through the commit history of this branch. 

To ease processing requirements of the RPi, the RPi has been configured to act only as an interface between the LIDAR and the Arduino, and does no data  processing on board. Thus, the only nodes enabled on ```alex_main.py``` are the Arduino interface, LIDAR interface and the TLS send/receive threads. 

The main program is split into 2 parts: the hardware communication part handled by the RPi, and the SLAM mapping part handled by another computer running a Windows Subsystem for Linux (WSL) instance. This reduces the processing load on the RPi, and allows commands to be parsed and handled quicker. 

The RPi connects to the Arduino and LIDAR through USB serial.

To run this successfully, it is important to ensure that the Python virtual environment (Studio 19) is set up and activated within the terminal.
```bash
sudo chmod +X setup_environment.sh
sudo ./setup_environment.sh
source ./env/bin/activate
```
> [!NOTE]
> It is also possible to add aliases to easily activate the .venv by modifying the ```.bashrc``` file within the home directory.

# Nodes Overview
The nodes consist of threads running in parallel and communicate through the provided PubSub library. Modifications are made to the nodes/threads itself, and do not affect the main file.

Enabling/disabling the various threads can be done by modifying ```alex_main.py```, setting the various boolean variables to ```True``` or ```False```.
> [!WARNING]
> Modifying which threads are active may affect the messages being published over the included PubSub library, and may cause other threads to crash spectacularly.
## Arduino Send/Receive Node
These threads handle the serializing and deserializing of message ```TPackets``` to and from the Arduino, and have not been modified. The only modification is within the provided Arduino control library, where ```libraries/epp2/control/control/alex_control_constants.py``` has been modified to fit the new commands, to have parity with the Arduino.

Messages to be sent to the Arduino are received by the receive thread over the ```arduino/send``` topic, and messages that are received from the Arduino are published over the ```arduino/recv``` topic.

## LIDAR Scan Node
This thread handles the communication between the LIDAR and RPi. After obtaining the raw LIDAR data, this thread is supposed to pubish the raw data to the ```lidar/scan``` topic. To lighten the processing load on the RPi, the raw data is instead sent over a TCP web socket, accessible using the RPi's local IPv4 address and port ```8765```. The receiving function can be found in the WSL code branch.

This thread starts a TCP WebSocket before entering the main loop. Within the main loop, the data originally published to the associated topic is instead encoded into a JSON with a newline character at the end to denote the end of 1 raw LIDAR data packet, and then sent to the socket, to be received by the WSL instance.

## TLS Node
TLS was used to secure the tele-operation of Alex. To ensure that no bad actors are able to get unauthorized control of Alex, the connecting client must be authenticated before being able to connect to the RPi and send commands.

Keys and certificates were made following instructions from Studio 15. Modifications include changing the ```CMD``` parameter to an ```integer``` in favor of a ```char```, which made it easier to decode, as the ```char``` may be decoded into it's ASCII equivalent. Also, the ```parseUserInput``` function was bypassed in favor of creating a DIY command packet within the thread itself and published directly to the ```arduino/send``` topic, as the commands found in the provided function were found to be very limiting.

## CLI Node
The CLI thread handles the tele-operation of Alex through the terminal. This thread has been heavily modified to use the ```getch``` library, which must be installed before running the main file. 
> [!IMPORTANT]
> After making and activating the virtual environment (Studio 19), run the following command to install the ```getch``` dependency.
> ```bash
> pip install getch
> ```

Enabling this thread is useful for bypassing the TLS connection to control Alex over the SSH terminal. This thread bypasses the the given ```parseUserInput``` function, and command packets are created after every keypress and published directly to the ```arduino/send``` topic, as the commands found in the provided function were found to be very limiting. 

To ensure that the Serial buffer between the RPi and the Arduino is not flooded, this thread also subscribes to the ```arduino/recv``` topic to check incoming messages from the Arduino. Importantly, at least 2 acknowledgement messages must be received from the Arduino before the next key press is accepted and sent to the Arduino. If the Arduino receives a bad packet and does not return the second acknowledgement message, the operator must bypass the in-built checks of this thread using the '=' key. This resets the boolean variables used in the checks, and resumes checking for key presses. This is made possible as the Arduino automatically clears the Serial buffer upon receiving any bad packet, which allows the next packet to be received cleanly without being affected by the bad packet.

The key bindings are detailed in the table below.

| Key  | Action            |
| :--: | :---------------: |
| q    | Forward 2cm       |
| w    | Forward 6cm       |
| e    | Forward 10cm      |
| a    | Backward 2cm      |
| s    | Backward 6cm      |
| d    | Backward 10cm     |
| y    | Left 7°           |
| h    | Left 25°          |
| n    | Left 85°          |
| u    | Right 7°          |
| j    | Right 25°         |
| m    | Right 85°         |
| z    | Stop              |
| c    | Color             |
| o    | Open claw         |
| p    | Close claw        |
| 5    | LCD message 1     |
| 6    | LCD message 2     |
| 7    | LCD message 3     |
| 8    | LCD message 4     |
| =    | Bypass            |
| ]    | Quit              |

# Camera Overview
The camera script cannot be modified and is provided as is from Studio 19. After ensuring that the Pi Camera is connected to the RPi, run ```AlexCameraStreamServer.py```, and using a web browser on a device connected to the same Wi-Fi network as the RPi, access the camera server using the RPi's IPv4 address on port 8000. (e.g. 192.168.1.1:8000).