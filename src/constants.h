#ifndef _CONSTANTS_INC_
#define _CONSTANTS_INC_

#include <stdint.h>

// For debugging purposes using Serial Monitor, uncomment to print, comment to disable
// #define PRINTLN(x) Serial.println(x)
// #define PRINT(x) Serial.print(x)

// Defined as blanks to compile without printing, if any
#define PRINTLN(x)
#define PRINT(x)

#define MAX_STR_LEN   32

// This packet has 1 + 1 + 2 + 32 + 16 * 4 = 100 bytes

// For direction commands, param[0] = distance in cm to move param[1] = speed
typedef struct {
	char packetType;
	char command;
	char dummy[2]; // Padding to make up 4 bytes
	char data[MAX_STR_LEN]; // String data
	uint32_t params[16];
} TPacket;


// This file containts all the packet types, commands and status constants


/**
 * ========================================================
 * Parameter types for packetType
 * ========================================================
 */


// Packet types
typedef enum {
	PACKET_TYPE_COMMAND = 0,
	PACKET_TYPE_RESPONSE = 1,
	PACKET_TYPE_ERROR = 2,
	PACKET_TYPE_MESSAGE = 3,
	PACKET_TYPE_HELLO = 4
} TPacketType;


/**
 * ========================================================
 * Respond types for command (response field for the Arduino)
 * ========================================================
 */


// Response types. This goes into the command field
typedef enum {
	RESP_OK = 0,
	RESP_STATUS = 1,
	RESP_BAD_PACKET = 2,
	RESP_BAD_CHECKSUM = 3,
	RESP_BAD_COMMAND = 4,
	RESP_BAD_RESPONSE = 5 
} TResponseType;


/**
 * ========================================================
 * Command types for command
 * ========================================================
 */


// Replaced some defaults for other commands
typedef enum {
	COMMAND_FORWARD = 0,
	COMMAND_REVERSE = 1,
	COMMAND_TURN_LEFT = 2,
	COMMAND_TURN_RIGHT = 3,
	COMMAND_STOP = 4,
	COMMAND_GET_STATS = 5,
	// COMMAND_CLEAR_STATS = 6,
	COMMAND_SCREEN = 6,
	COMMAND_COLOR = 7,
	COMMAND_OPEN = 8,
	COMMAND_CLOSE = 9
} TCommandType;


/**
 * ========================================================
 * Parameter types for params[16]
 * ========================================================
 */


// Direction commands
typedef enum {
	DIR_FORWARD = 1,
	DIR_BACKWARD = 2,
	LEFT = 3,
	RIGHT = 4,
} TDirection;

// Direction values
typedef enum Tdir {
	STOP,
	GO,
	BACK,
	CCW,
	CW
} Tdir;

#endif