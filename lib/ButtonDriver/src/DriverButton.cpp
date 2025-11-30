#include "DriverButton.h"

uint8_t previousButtonStates[20];
uint8_t currentButtonStates[20];

void buttonsSetup() {
  for (int i = 0; i < 20; ++i) {
    previousButtonStates[i] = HIGH;
    currentButtonStates[i] = HIGH;
  }
}

bool isButtonPressed(int pin) {
  previousButtonStates[pin] = currentButtonStates[pin];
  currentButtonStates[pin] = digitalRead(pin);
  if (currentButtonStates[pin] == LOW &&
      currentButtonStates[pin] != previousButtonStates[pin]) {
    return true;
  }
  return false;
}
