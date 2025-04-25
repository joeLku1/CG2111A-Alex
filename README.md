# CG2111A Alex WSL Python Code
Code by Group B02-6A

Members:
- Low Zen Wei
- Ong Yi Siang Joel
- Rishabh Ramprasad Shenoy
- Joel Ku
- Jerry Andikko

For the Arduino and RPi code, check out the other branches of this repository.
<br><br>
# Overview
Windows Subsystem for Linux (WSL) was chosen to maintain commonality with the RPi, and a Linux-compatible execution environment is neccesary to run the virtual environment setup script. By running a similar ```alex.zip``` with the RPi, it is possible to "outsource" the SLAM processing and mapping to a computer with significantly more processing power, as well as use the provided TLS networking libraries to establish communications with the RPi to control Alex securely over the network.

To run this successfully, it is important to ensure that the Python virtual environment (Studio 19) is set up and activated within the terminal.
```bash
sudo chmod +X setup_environment.sh
sudo ./setup_environment.sh
source ./env/bin/activate
```
> [!NOTE]
> It is also possible to add aliases to easily activate the .venv by modifying the ```.bashrc``` file within the home directory.

# Nodes Overview
The nodes consist of the remaining threads in ```alex_main.py``` that the RPi does not run, which are the data, SLAM, GUI threads, responsible for receiving the SLAM data over the TCP WebSocket from the RPi, SLAM localization with the raw data and subsequently map output. 

> [!WARNING]
> Modifying which threads are active will cause the program to attempt to connect to devices that are not connected, and may cause other threads to crash spectacularly.

## Data Thread
This new thread is based off the other threads, and is made to be run in parallel with the existing threads. To reduce modifications made to the SLAM thread, this new thread was created to receive the data sent over the TCP WebSocket from the RPi, then publish the data to the ```lidar_scan``` topic, which simulates how the LIDAR scan thread would do it if the LIDAR was connected locally.

> [!NOTE]
> The IP address within ```dataThread``` should be modified to the RPi's local IPv4 address on the network.
> It is possible to find the IP address of the RPi using the ```ifconfig``` command in the RPi terminal.

The raw LIDAR data is encoded into JSON format by the RPi before being sent, and must be decoded locally after being received before being published to the appropriate topic. As the LIDAR data has variable length, the newline character added at the end of the packet helps to signify the end of each data packet.

> [!TIP]
> It is possible to implement LIDAR data filtering either on the RPi before the data is sent over the WebSocket, or on the WSL after the data has been decoded.

## SLAM Thread
This thread is responsible for running BreezySLAM, the provided algorithm for SLAM processing of the LIDAR data. Raw LIDAR data is received using the internal PubSub library over the ```lidar_scan``` topic. 

Other than the updating the LIDAR offset degrees, nothing significant was modified from the provided code.

## Display Thread
This thread is responsible for outputting the results of BreezySLAM, and gives the operator a live map that is updated with the LIDAR data and the updated position of Alex. As the SLAM algorithm on WSL runs signficiantly faster compared to on the RPi, there was no need for signficant changes to this thread.

> [!TIP]
> It is possible to zoom in on the map if further clarity is required, and it is also possible make the map interactive.


# TLS Commands
The WSL also handles secure communications with the RPi through TLS. This is done with a separate script, ```tls_commands.py```. 

Keys and certificates were made following instructions from Studio 15. 

If the TLS threads on the RPi are activated, the CLI thread on the RPi must be deactivated. This script interfaces with the RPi TLS threads, and is an alternative to using the CLI thread in the operation of Alex. 

> [!NOTE]
> The IP address on the ```host``` variable should be modified to the RPi's local IPv4 address on the network.

Similarly to the RPi's modified CLI thread, this script is dependent on the ```getch``` library, which must be installed before running.
> [!IMPORTANT]
> After making and activating the virtual environment (Studio 19), run the following command to install the ```getch``` dependency.
> ```bash
> pip install getch
> ```

Similarly to the RPi's modified CLI thread, this thread counts the number of packets received from the Arduino, sent over the TLS send thread from the RPi, to ensure that the Serial buffer between the RPi and Arduino is not flooded. At least 2 acknowledgement messages from the Arduino must be received before the next key press is accepted and sent over the TLS connection to the RPi. If there are no incoming messages, the boolean conditionals used in the checks will automatically reset, though the bypass key is also available for manual reset. 

> [!NOTE]
> When receiving the color sensor data, it may be possible that the color sensor message packet containing the detected color results may take longer to be transmitted, causing a timeout and the acknowledgement messages may not be received in time before the booleans are reset. If the packet does not arrive on time, press 'z' a few times to print out the detected color packet without affecting the position of Alex.

> [!WARNING]
> There is currently no formatting for detected color packet, and the raw packet in bytes will be shown.

The key bindings are detailed in the table below. These are similar to the keybindings used in the RPi CLI thread.

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
