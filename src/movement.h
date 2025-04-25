#ifndef MOVEMENT_H
#define MOVEMENT_H

#include <AFMotor.h>
#include <Arduino.h>
#include "constants.h"

#define FRONT_LEFT   4
#define FRONT_RIGHT  3
#define BACK_LEFT    1
#define BACK_RIGHT   2

#define COUNTS_PER_REV      8
#define WHEEL_CIRC          20.42

#define RANGE 2

void move(float speed, int direction);
void stop();
void forward(float dist, float speed);
void backward(float dist, float speed);
void ccw(float angle, float speed);
void cw(float angle, float speed);

#endif