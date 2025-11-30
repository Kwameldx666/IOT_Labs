#ifndef DRIVER_BUTTON_H
#define DRIVER_BUTTON_H

#include <Arduino.h>

extern uint8_t previousButtonStates[20];
extern uint8_t currentButtonStates[20];

void buttonsSetup();
bool isButtonPressed(int pin);

#endif
