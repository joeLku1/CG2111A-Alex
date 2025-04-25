#include "functions.h"

void setup() {
	cli();                // Disable interrupts
	setupEINT();          // Setup external interrupts
	pinSetup();           // Setup GPIO pins
	enablePullups();      // Enable pullup resistors for wheel encoders
	setupstartSerial();   // Setup serial port for communication with RPi (9600, 8N1)
	servoSetup();         // Setup servos
	colourSensorSetup();  // Setup color sensor
	sei();                // Enable interrupts
	flashRed();           // Flash red indicator LED
	waitForHello();       // Wait for hello packet from RPi
	screenSetup();        // Setup LCD screen
}

void loop() {
	waitForCommand();     // Wait for command packet from RPi
	// readSHandle();			// Uncomment for Serial Monitor debugging
}