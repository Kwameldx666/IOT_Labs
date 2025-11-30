#ifndef MOTOR_DRIVER_H
#define MOTOR_DRIVER_H

#include <Arduino.h>

void motorSetup(int enablePin, int pin1, int pin2);
void setSpeed(int speed);
int getSpeed();

#endif
