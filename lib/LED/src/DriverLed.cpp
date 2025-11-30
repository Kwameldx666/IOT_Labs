#include "DriverLed.h"

void ledSetup(int pin) {
	pinMode(pin, OUTPUT);
	digitalWrite(pin, LOW);
}

void ledOn(int pin) {
	digitalWrite(pin, HIGH);
}

void ledOff(int pin) {
	digitalWrite(pin, LOW);
}

void changeLedState(int pin, byte state) {
	digitalWrite(pin, state);
}
