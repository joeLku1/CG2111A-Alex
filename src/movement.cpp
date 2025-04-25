#include "movement.h"

volatile TDirection dir;

// Volatile variables for the left and right encoders
volatile unsigned long leftForwardTicks, rightForwardTicks, leftReverseTicks, rightReverseTicks;
volatile unsigned long leftRevs, rightRevs;
volatile unsigned long forwardDist, reverseDist; 
unsigned long deltaTicks, targetTicks;

float msd = 7.4;			// Delay milliseconds per degree
unsigned long startTime = 0;

AF_DCMotor motorFL(FRONT_LEFT);
AF_DCMotor motorFR(FRONT_RIGHT);
AF_DCMotor motorBL(BACK_LEFT);
AF_DCMotor motorBR(BACK_RIGHT);

/**
 * @brief ISR function for left encoder on INT2
 */
ISR(INT2_vect) {
	if (dir == DIR_FORWARD) leftForwardTicks++;			// If moving forward, increment forward ticks
	if (dir == DIR_BACKWARD) leftReverseTicks++;   	// If moving backward, increment reverse ticks  
	if (dir == DIR_FORWARD) forwardDist = (unsigned long) ((float) leftForwardTicks / COUNTS_PER_REV * WHEEL_CIRC);
	if (dir == DIR_BACKWARD) reverseDist = (unsigned long) ((float) leftReverseTicks / COUNTS_PER_REV * WHEEL_CIRC);
}


/**
 * @brief ISR function for right encoder on INT3
 */
ISR(INT3_vect) {
	if (dir == DIR_FORWARD) rightForwardTicks++;    // If moving forward, increment forward ticks
	if (dir == DIR_BACKWARD) rightReverseTicks++;   // If moving backward, increment reverse ticks
}

/**
 * @brief Main movement function
 * @param speed: 0-100 in %
 * @param direction: 0 = stop, 1 = forward, 2 = backward, 3 = ccw (left), 4 = cw (right)
 */
void move(float speed, int direction) {
	int speed_scaled = (speed / 100.0) * 255;
	motorFL.setSpeed(speed_scaled);
	motorFR.setSpeed(speed_scaled);
	motorBL.setSpeed(speed_scaled);
	motorBR.setSpeed(speed_scaled);

	switch(direction) {
		case BACK:
			motorFL.run(BACKWARD);
			motorFR.run(BACKWARD);
			motorBL.run(FORWARD);
			motorBR.run(FORWARD); 
			break;
		case GO:
			motorFL.run(FORWARD);
			motorFR.run(FORWARD);
			motorBL.run(BACKWARD);
			motorBR.run(BACKWARD); 
			break;
		case CW:
			motorFL.run(FORWARD);
			motorFR.run(BACKWARD);
			motorBL.run(BACKWARD);
			motorBR.run(FORWARD); 
			break;
		case CCW:
			motorFL.run(BACKWARD);
			motorFR.run(FORWARD);
			motorBL.run(FORWARD);
			motorBR.run(BACKWARD); 
			break;
		case STOP:
		default:
			motorFL.run(STOP);
			motorFR.run(STOP);
			motorBL.run(STOP);
			motorBR.run(STOP); 
	}
}

// Stop all motors
void stop() {
	move(0, STOP);
	dir = (TDirection) STOP; // Set direction to STOP
}

/**
 * @brief Move forward
 * @param dist: Distance in cm to move
 * @param speed: Speed in % of max speed
 */
void forward(float dist, float speed) {
	dir = (TDirection) DIR_FORWARD;
	move(speed, FORWARD);
	while (forwardDist < dist) {} // Wait until the distance is reached
	stop(); 											// Stop the motors when the distance is reached
	forwardDist = 0; 							// Reset the distance counter
	leftForwardTicks = 0; 				// Reset the encoder ticks
	rightForwardTicks = 0;
}

/**
 * @brief Move backward
 * @param dist: Distance in cm to move
 * @param speed: Speed in % of max speed
 */
void backward(float dist, float speed) {
	dir = (TDirection) DIR_BACKWARD;
	move(speed, BACKWARD);
	while (reverseDist < dist) {} // Wait until the distance is reached
	stop(); 											// Stop the motors when the distance is reached
	reverseDist = 0; 							// Reset the distance counter
	leftReverseTicks = 0; 				// Reset the encoder ticks
	rightReverseTicks = 0;
}

/**
 * @brief Turn clockwise (right)
 * @param angle: Angle in degrees to turn
 * @param speed: Speed in % of max speed
 * @note The speed parameter is locked to 90% for consistency in turning
 */
void cw(float ang, float speed) {
	dir = (TDirection) RIGHT;
	move(90, CW);																	// Lock speed to 90% for turning
	startTime = millis();													// Start the timer
	while (millis() - startTime < (ang * msd));		// Wait until the angle is reached
	stop();																				// Stop the motors when the angle is reached
}

/**
 * @brief Turn counter-clockwise (left)
 * @param angle: Angle in degrees to turn
 * @param speed: Speed in % of max speed
 * @note The speed parameter is locked to 90% for consistency in turning
 */
void ccw(float ang, float speed) {
	dir = (TDirection) LEFT;
	move(90, CCW);																// Lock speed to 90% for turning
	startTime = millis();													// Start the timer
	while (millis() - startTime < (ang * msd));		// Wait until the angle is reached
	stop();																				// Stop the motors when the angle is reached
}
