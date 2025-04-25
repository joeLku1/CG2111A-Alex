#ifndef _FUNCTIONS_H_
#define _FUNCTIONS_H_

#include <Arduino.h>
#include "serialize.h"
#include "movement.h"
#include "arm_color.h"
#include "screen.h"

/**
 * ========================================================
 * Interrupt stuff and pin setups
 * ========================================================
 */


// Setup external interrupts
void setupEINT() {
	// EINT2 and EINT3 interrupt, falling edge, respectively
	EICRA |= (1 << ISC31) | (1 << ISC21); // 1010 0000 (10 is falling edge, first 4 bits are for INT3 and INT2)
	EIMSK |= (1 << INT3) | (1 << INT2);   // 0000 1100 (INT7 to 0 respectively, INT3 and INT2 are 1)
}


// Enable pullup resistors for wheel encoders
void enablePullups() {
	// Enable the pull-up resistors on pins 19 and 18. These are pins PD2 and PD3 respectively.
	// Set bits 2 and 3 in DDRD to 0 to make them inputs.
	DDRD = 0;                               // PD3/2 (INT3/2) is now an input - both on Port D
	PORTD |= (1 << PORTD3) | (1 << PORTD2); // PD3/2 (INT3/2) is now HIGH
}

// Declare GPIO pins
void pinSetup() {
	// Pins 22, 23, 24, 25 are LED pins (output)
	DDRA |= (1 << DDA0) | (1 << DDA1) | (1 << DDA2) | (1 << DDA3);
	// Pin 52 is LED pin (output)
	DDRB |= (1 << DDB1);
}

// Flash the LED once
void flashRed() {
	PORTB |= (1 << PORTB1); // Turn on LED on pin 52
	delay(100);
	PORTB &= ~(1 << PORTB1); // Turn off LED on pin 52
	delay(100);
}

/**
 * ========================================================
 * Serial setup and basic functions
 * ========================================================
 */


/**
 * @brief Setup the serial port for communication with RPi, 9600, 8N1
 * @note Bare metal did not work, code left in for reference
 */
// 
void setupstartSerial() {
	Serial.begin(9600);
	// Bare Metal UART on UART0, 9600 8N1
	// UBRR0L = 103;
	// UBRR0H = 0;
	// // UCSR0C = (1 << UCSZ01) | (1 << UCSZ00); // Set frame format: 8 data bits, 1 stop bit
	// UCSR0C = 0b00000110;
	// UCSR0A = 0;
	// // UCSR0B = (1 << TXEN0) | (1 << RXEN0);   // Enable TX and RX
	// UCSR0B = 0b00011000;
}

/**
 * * @brief Waits for a hello packet from the RPi
 * * @param buffer: Pointer to buffer variable to store the incoming data
 */
int readSerial(char *buffer) {
	int count = 0;
	while (Serial.available()) buffer[count++] = Serial.read();
	return count;
}

// Write data to Serial
void writeSerial(const char *buffer, int len) {
	Serial.write(buffer, len);
}


/**
 * ========================================================
 * Communication setup and basic functions
 * ========================================================
 */


 /**
	* @brief Reads in data from the serial port and deserializes it. Returns deserialized data in "packet"
	* @param packet: Pointer to new packet to be filled with data
  */
TResult readPacket(TPacket *packet) {
	char buffer[PACKET_SIZE];
	int len;
	len = readSerial(buffer);
	if (len == 0) return PACKET_INCOMPLETE;
	else return deserialize(buffer, len, packet);
}

/**
 * @brief Takes a packet, serializes it then sends it out over the serial port.
 * @param packet: Pointer to the packet to be sent
 */
void sendResponse(TPacket *packet) {
	char buffer[PACKET_SIZE];
	int len;
	len = serialize(buffer, packet, sizeof(TPacket));
	writeSerial(buffer, len);
}

/**
 * @brief Sends a message back to the Pi. Useful for debugging.
 * @param message: The message to send back to the Pi.
 */
void sendMessage(const char *message) {
	TPacket messagePacket;
	messagePacket.packetType = PACKET_TYPE_MESSAGE;
	strncpy(messagePacket.data, message, MAX_STR_LEN);
	sendResponse(&messagePacket);
}

/**
 * @brief Debugging function to print messages to the serial port.
 * @param format: The pointer to the string to be printed.
 */
void dbprintf(const char *format, ...) {
	va_list args;
	char buffer[128];
	va_start(args, format);
	vsprintf(buffer, format, args);
	sendMessage(buffer);
}

/**
 * @brief 0: Bad Packet, 1: Bad Checksum, 2: Bad Command, 3: Bad Response
 * @param type: The type of error to send back to the RPi.
 */
void sendError(int type) {
	TPacket errorPacket;
	errorPacket.packetType = PACKET_TYPE_ERROR;
	switch(type) {
		case 0:
			errorPacket.command = RESP_BAD_PACKET;
			break;
		case 1:
			errorPacket.command = RESP_BAD_CHECKSUM;
			break;
		case 2:
			errorPacket.command = RESP_BAD_COMMAND;
			break;
		case 3:
			errorPacket.command = RESP_BAD_RESPONSE;
			break;
	}
	sendResponse(&errorPacket);
}

/**
 * @brief Sends OK response packet to the RPi
 */
void sendOK() {
	TPacket okPacket;
	okPacket.packetType = PACKET_TYPE_RESPONSE;
	okPacket.command = RESP_OK;
	sendResponse(&okPacket);
}

/**
 * ========================================================
 * Color sensor reply function
 * ========================================================
 */


/**
 * @brief Sends color response packet to the RPi
 * @param color: The color detected by the color sensor.
 */
void sendColor(String color) {
	TPacket ColorPacket;
	ColorPacket.packetType = PACKET_TYPE_RESPONSE;
	ColorPacket.command = RESP_STATUS;
	if (color == "green") {
		ColorPacket.params[0] = 1;
	}
	else if(color == "red") {
		ColorPacket.params[0] = 2;
	}
	else {
		ColorPacket.params[0] = 0;
	}
	ColorPacket.params[1] = 0;
	ColorPacket.params[2] = 0;
	ColorPacket.params[3] = 0;
	ColorPacket.params[4] = 0;
	ColorPacket.params[5] = 0;
	ColorPacket.params[6] = 0;
	ColorPacket.params[7] = 0;
	ColorPacket.params[8] = 0;
	ColorPacket.params[9] = 0;
	ColorPacket.params[10] = 0;
	ColorPacket.params[11] = 0;
	ColorPacket.params[12] = 0;
	ColorPacket.params[13] = 0;
	ColorPacket.params[14] = 0;
	ColorPacket.params[15] = 0;

	sendResponse(&ColorPacket);
}


/**
 * ========================================================
 * Main communication commands
 * ========================================================
 */


/**
 * @brief Handles incoming packets from the RPi
 * @param command: The command packet from the RPi
 */
void handleCommand(TPacket *command) { 
	flashRed();
	switch (command->command) {
		case COMMAND_FORWARD:
			sendOK();
			forward((double) command->params[0], (float) command->params[1]);
			sendOK();
			break;
		case COMMAND_REVERSE:
			sendOK();
			backward((double) command->params[0], (float) command->params[1]);
			sendOK();
			break;
		case COMMAND_TURN_LEFT:
			sendOK();
			ccw((double) command->params[0], (float) command->params[1]);
			sendOK();
			break;
		case COMMAND_TURN_RIGHT:
			sendOK();
			cw((double) command->params[0], (float) command->params[1]);
			sendOK();
			break;
		case COMMAND_STOP:
			sendOK();
			stop();
			sendOK();
			break;
		case COMMAND_COLOR:
			sendOK();
			sendColor(readColor());				// Sends packet based on color detected
			sendOK();
			break;
		case COMMAND_OPEN:
			sendOK();
			servoOpen();
			sendOK();
			break;
		case COMMAND_CLOSE:
			sendOK();
			servoClose();
			sendOK();
			break;
		case COMMAND_SCREEN:
			sendOK();
			showMessage(command->params[0]);
			sendOK();
			break;
		default:
			sendError(2);
			sendError(2);
			break;
	}
}

/**
 * @brief Upon startup, wait for Hello packet from RPi, send OK response back
 * Resets the serial buffer if any error occurs
 */
void waitForHello() {
  int exit = 0;
  while (!exit) {
    TPacket hello;
    TResult result;
    do {
      result = readPacket(&hello);
    } while (result == PACKET_INCOMPLETE);

    if (result == PACKET_OK) {
      if (hello.packetType == PACKET_TYPE_HELLO) {
        flashRed();
        sendOK();
        exit = 1;
      }
      else { 																		// Bad response
				sendError(3);
				while (Serial.available()) { Serial.read(); }
			}
    }
    else if (result == PACKET_BAD) { 						// Bad packet
			sendError(0);
			while (Serial.available()) { Serial.read(); }
		}
    else if (result == PACKET_CHECKSUM_BAD) {		// Bad checksum 
			sendError(1);
			while (Serial.available()) { Serial.read(); }
		}
  }
}

/**
 * @brief Waits for a command packet from RPi, handles packet with handleCommand
 * Runs every loop, sends error packets back if any error
 * Resets the serial buffer if any error occurs
 */
void waitForCommand() {
	int exit = 0;
	while (!exit) {
		TPacket recvPacket;
		TResult result;
		do {
			result = readPacket(&recvPacket);
		} while (result == PACKET_INCOMPLETE);
	
		if (result == PACKET_OK) {
			if (recvPacket.packetType == PACKET_TYPE_COMMAND) { 
				handleCommand(&recvPacket);
				exit = 1;
			}
			else {																		// Bad response
				sendError(3);
				while (Serial.available()) { Serial.read(); }
				exit = 1;
			} 
		}
		else if (result == PACKET_BAD) {						// Bad packet
			sendError(0);
			while (Serial.available()) { Serial.read(); }
			exit = 1;
		}
		else if (result == PACKET_CHECKSUM_BAD) {		// Bad checksum
			sendError(1);
			while (Serial.available()) { Serial.read(); }
			exit = 1;
		}
	}
}

/**
 * @brief Custom handleCommand for Serial Monitor input for debugging
 * @note Only works with 2 digits for distance/angle and power
 * @param input1: Command: 0 = forward, 1 = backward, 2 = ccw, 3 = cw, 4 = stop
 * @param input2: Distance in cm to move / angle in deg to turn
 * @param input3: Speed in % of max speed
 * @example 0 10 50 = forward 10cm at 50% speed
 */
void handleSerialM(int input1, int input2, int input3) {
  switch (input1) {
		case COMMAND_FORWARD:
			flashRed();
			sendOK();
			PRINTLN("forward");
			forward((double) input2, (float) input3);
			sendOK();
			break;
		case COMMAND_REVERSE:
			flashRed(); flashRed();
			sendOK();
			PRINTLN("backward");
			backward((double) input2, (float) input3);
			sendOK();
			break;
    case COMMAND_TURN_LEFT:
      flashRed(); flashRed(); flashRed();
      sendOK();
      PRINTLN("ccw/left");
      ccw((double) input2, (float) input3);
			sendOK();
      break;
    case COMMAND_TURN_RIGHT:
      flashRed(); flashRed(); flashRed(); flashRed();
      sendOK();
      PRINTLN("ccw/right");
      cw((double) input2, (float) input3);
			sendOK();
      break;
		case COMMAND_STOP:
			sendOK();
      PRINTLN("stop");
			stop();
			sendOK();
			break;
		case COMMAND_COLOR:
			sendOK();
			PRINTLN("color");
			PRINTLN(readColor());
			sendColor(readColor());					// Sends packet based on color detected
			sendOK();
			break;
		case COMMAND_OPEN:
			sendOK();
			PRINTLN("open");
			servoOpen();
			sendOK();
			break;
		case COMMAND_CLOSE:
			sendOK();
			PRINTLN("close");
			servoClose();
			sendOK();
			break;
		case COMMAND_SCREEN:
			sendOK();
			PRINTLN("screen");
			showMessage((uint32_t) input2);
			sendOK();
			break;
		default:
			sendError(2);
			sendError(2);
			break;
	}
}

/**
 * @brief Custom waitForCommand for Serial Monitor input
 * @note Only works with 2 digits for distance/angle and power
 * @note Remember to uncomment PRINT and PRINTLN in constants.h to see output in Serial Monitor
 * @example Sending "0 10 50" on Serial Monitor will move Alex forward 10cm at 50% speed
 */
void readSHandle() {
	if (Serial.available() > 0) {
		String input = Serial.readStringUntil('\n');
		int input1 = input.charAt(0) - '0'; // Convert char to int
		PRINT(input1); PRINT(" ");
		int input2 = input.substring(2, 4).toInt(); // Convert substring to int
		PRINT(input2); PRINT(" ");
		int input3 = input.substring(5, 7).toInt(); // Convert substring to int
		PRINTLN(input3);
		handleSerialM(input1, input2, input3);
	}
}

#endif