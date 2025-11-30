#ifndef DRIVER_LED_H
#define DRIVER_LED_H

#include <Arduino.h>

void ledSetup(int pin);
void ledOn(int pin);
void ledOff(int pin);
void changeLedState(int pin, byte state);

#endif
