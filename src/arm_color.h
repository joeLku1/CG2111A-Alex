#ifndef _ARM_COLOR_H_
#define _ARM_COLOR_H_

#include <Arduino.h>
#include <vector>
#include <string>
#include <algorithm>

// Servo angles
#define openAngleLeft 40
#define openAngleRight 140
#define closeAngleLeft1 60
#define closeAngleRight1 120
#define closeAngleLeft2 115
#define closeAngleRight2 65

// Pins for servos
#define leftServoPin 44
#define rightServoPin 45

// TCS230/TCS3200 Color Sensor pins
#define S0 30
#define S1 32
#define S2 33
#define S3 31
#define sensorOut 35

// Structure to store color and name
struct color {
  float r;
  float g;
  float b;
  String colorstring;
};

// Structure to store color distance from known colors and name
struct colordiff {
  float distsqr = -1;
  String colorstring;
};

// Max
template <typename T>
T cmax(T a, T b) {
	return (a > b) ? a : b;
}

// Min
template <typename T>
T cmin(T a, T b) {
	return (a < b) ? a : b;
}

void servoSetup();

void servoOpen();
void servoClose();

void colourSensorSetup();
String readColor();

color normalize_rgb(const color& raw_input, const color& black_ref, const color& white_ref);
colordiff eucdistsqr(const color& current, const color& sample);
String knn_classification_red_green(const color& input_color, int k);
String predict_color(const color& raw_input);

#endif