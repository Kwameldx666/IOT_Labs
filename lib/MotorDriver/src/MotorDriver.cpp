#include "MotorDriver.h"

static int g_enablePin = -1;
static int g_pin1 = -1;
static int g_pin2 = -1;
static int g_currentSpeed = 0;

void motorSetup(int enablePin, int pin1, int pin2) {
	g_enablePin = enablePin;
	g_pin1 = pin1;
	g_pin2 = pin2;

	pinMode(g_enablePin, OUTPUT);
	pinMode(g_pin1, OUTPUT);
	pinMode(g_pin2, OUTPUT);
	digitalWrite(g_enablePin, LOW);
	digitalWrite(g_pin1, LOW);
	digitalWrite(g_pin2, LOW);
	g_currentSpeed = 0;
}

void setSpeed(int speed) {
	if (g_enablePin < 0) {
		return;
	}

	speed = constrain(speed, -255, 255);
	g_currentSpeed = speed;

	if (speed == 0) {
		digitalWrite(g_enablePin, LOW);
		digitalWrite(g_pin1, LOW);
		digitalWrite(g_pin2, LOW);
		return;
	}

	const bool forward = (speed > 0);
	digitalWrite(g_pin1, forward ? HIGH : LOW);
	digitalWrite(g_pin2, forward ? LOW : HIGH);
	analogWrite(g_enablePin, abs(speed));
}

int getSpeed() {
	return g_currentSpeed;
}
