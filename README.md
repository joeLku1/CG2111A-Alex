# CG2111A Alex Arduino Code
Code by Group B02-6A

Members:
- Low Zen Wei
- Ong Yi Siang Joel
- Rishabh Ramprasad Shenoy
- Joel Ku
- Jerry Andikko

For RPi and WSL codes, check out the other branches of this repository.
<br><br>
# Overview
This project's Arduino code uses the PlatformIO extension in VSCode to upload code to the Arduino Mega. This branch contains all the necessary files for setting up a standalone PlatformIO project. Source files are found within the ```src``` folder.

If Arduino IDE the intended Arduino code editor of choice, ```AlexMain.cpp``` should be renamed to ```AlexMain.ino``` for compatibility, and the following dependencies must be installed:
1. [Adafruit Motor Shield Library](https://github.com/adafruit/Adafruit-Motor-Shield-library)
2. [LiquidCrystal_I2C](https://github.com/johnrickman/LiquidCrystal_I2C)
3. [ArduinoSTL](https://github.com/mike-matera/ArduinoSTL)


<br/>The Arduino communicates to the RPi using the USB port, with Serial Communication set up at 9600 baud, 8N1. Communication is handled with the ```TPacket``` struct.

# Main Program
```AlexMain.cpp``` is the file that contains the main ```setup``` and ```loop``` functions.

Within ```setup```, interrupts and GPIO pins are set to their respective modes (in bare metal, by modifying registers), and all the attached devices/components are set up and initialized.

The Arduino will wait for the RPi to send a ```hello``` packet within the setup process before proceeding on to the main loop, under the ```waitForHelloCommand``` function. This is to ensure that Serial communication is set up properly before proceeding on to functional operation.

# Secondary Libraries
This section details the custom libraries made for the other subsystems.
## Movement
Movement is done with 4 motors, connected using the Adafruit Motor Shield. This shield uses a 74HC595 shift register to control 2 L293D motor drivers. The motors are wired as per the table below.

| Motor | Shield Connection |
| :---: | :---: |
| Back Left | M1 |
| Back Right | M2 |
| Front Right | M3 |
| Front Left | M4 |

### Forward/Backward Movement
For accurate forward and backward movement, magnetic wheel encoders are used to detect the revolutions made by the wheels. One full revolution causes 8 ticks on the encoder, and the wheel diameter is measured to be 20.42cm.

By using hardware interrupts on the encoders to continuously monitor the distance that has been travelled, forward/backward movement is accurate and controllable.

### Turning
The encoders are unable to verify turning due to wheel slippage, as the wheels are not positioned in a square shape. Thus, turns are time-based, with a calibrated constant ```msd``` that calculates the delay in microseconds per degree of turn required. 

## Arm/Color Sensor
The servo and color sensor functions are combined as the two are used together, when approaching a suspected astronaut to detect the color and subsequently releasing the medpak, or maneuvering to pick up the astronaut.

The claw is made out of 2 SG90 servos. These are powered by 5V from the Arduino, and require 2 PWM pins from the Arduino Mega for control. 

> [!NOTE]
> While the Adafruit Motor Shield has 2 in-built servo connection pins, the signal pins are connected to Pins 9 and 10, which are tied to Timer 2, which on the ATmega2560, is a 8-bit timer. The provided motor shield is built for the Arduino Uno board, which has pins 9 and 10 tied to Timer 1, a 16-bit timer.

Pins 44, 45 and 46 are connected to Timer 5 of the ATmega2560, and are the only exposed PWM pins tied to a 16-bit capable timer. The 16-bit timer is necessary to generate a 50Hz PWM square wave for the servo signal (Studio 4).

The left servo is connected to pin 44 and the right is to pin 45. The medpak is originally placed in the claw, and once the green astronaut is detected, the operator will open the claw to deposit the medpak, freeing the claw to pick up the red astronaut.

The ```servoClose``` function implements 2-step closing, such that the arm closes more gently, which prevents the astronaut from tipping over accidentally from rough handling. 

The color sensing algorithm is as follows:

1. The raw color from the color sensor is normalized using the black and white reference values
2. If the normalized values are within range, use a KNN algorithm to classify if the color is closer to red or green, else, return neither


> [!NOTE]
> For greater accuracy, color calibration can be done through updating the ```black_ref``` and ```white_ref``` variables with results from the color sensor.

## Communications
Serial communication is done at 9600 8N1. Messages between the Arduino and the RPi are serialized into the ```TPacket``` struct before being sent. This ensures commonality between both devices. 

```TPacket``` as provided is made out of 4 ```char``` variables and a list of 16 ```uint32_t```.

The function ```waitForCommand``` is run continuously in the main loop, to monitor the Serial buffer for any incoming messages from the RPi. For added redundancy and error handling purposes, if the Arduino receives a bad packet from the RPi, it replies with an error, while also clearing the Serial buffer to prepare for the next command. This ensures that the following packet received by the Arduino will not be affected by the preceding bad packet. 

The Arduino is also configured to send 2 acknowledgement packets per command received, 1 upon the verification that the command is a valid packet, and 1 after the command action has been completed. This is to allow the operator to know that the command has been completed, as well as to inform the RPi that the next command can be sent, to prevent the RPi from flooding the Serial buffer. 

For Serial debugging, as a ```TPacket``` is difficult to send using Serial Monitor, the function ```readSHandle``` is created to parse user input from the Serial Monitor into an action. This simplifies the testing and debugging process of the Arduino and associated components.

## LCD Screen
For communication between the operator and the astronaut, a screen is installed to show messages. There are a few messages to choose from, and the operator can control which message to show from the control station. 

The screen communicates using the I2C protocol, and uses an external library to handle the communications. 