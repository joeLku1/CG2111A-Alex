#ifndef _SCREEN_H_
#define _SCREEN_H_

#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

void screenSetup();

void showMessage(uint32_t params);

#endif