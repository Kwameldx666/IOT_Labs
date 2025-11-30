#include "lab1/lab1_1.hpp"

#include "CustomSTDIO.h"
#include "DriverLed.h"

static constexpr int kLedPin = 7;

void setup_lab1_1() {
	StdioSerialSetup();
	pinMode(kLedPin, OUTPUT);
	ledOff(kLedPin);

	printf("System ready. Commands: 'led on', 'led off'.\n");
}

void loop_lab1_1() {
	char buffer[32];
	if (!fgets(buffer, sizeof(buffer), stdin)) {
		return;
	}

	buffer[strcspn(buffer, "\r\n")] = '\0';

	if (strcmp(buffer, "led on") == 0) {
		ledOn(kLedPin);
		printf("LED is ON\n");
	} else if (strcmp(buffer, "led off") == 0) {
		ledOff(kLedPin);
		printf("LED is OFF\n");
	} else {
		printf("Unknown command\n");
	}
}
