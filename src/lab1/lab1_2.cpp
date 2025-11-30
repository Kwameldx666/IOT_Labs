#include "lab1/lab1_2.hpp"

#include <ctype.h>
#include <string.h>

#include "CustomKeypad.h"
#include "CustomSTDIO.h"
#include "LCDDisplay.h"
#include "DriverLed.h"

static constexpr int kRedLedPin = 3;
static constexpr int kGreenLedPin = 2;
static constexpr char kCorrectCode[] = "1234";

static char g_input[5];
static int g_index = 0;

static void resetInput() {
	g_index = 0;
	clearScreen();
	lcdPrint("Enter Code:");
	lcdSetCursor(0, 1);
}

static void showMessage(const char* msg) {
	clearScreen();
	lcdPrint(msg);
}

static void setAccessLED(bool granted) {
	if (granted) {
		ledOn(kGreenLedPin);
		ledOff(kRedLedPin);
	} else {
		ledOff(kGreenLedPin);
		ledOn(kRedLedPin);
	}
}

void setup_lab1_2() {
	StdioSerialSetup();
	lcdSetup();

	pinMode(kRedLedPin, OUTPUT);
	pinMode(kGreenLedPin, OUTPUT);
	ledOff(kRedLedPin);
	ledOff(kGreenLedPin);

	resetInput();
}

void loop_lab1_2() {
	const char key = keypad.getKey();
	if (!key) {
		return;
	}

	printf("[KEYPAD] Pressed key: %c\n", key);

	if (isdigit(static_cast<unsigned char>(key)) && g_index < 4) {
		g_input[g_index++] = key;
		lcdSetCursor(g_index - 1, 1);
		lcdPrint("*");
	}

	if (key == '#') {
		g_input[g_index] = '\0';

		if (strcmp(g_input, kCorrectCode) == 0) {
			showMessage("Access Granted");
			printf("[SYSTEM] Correct code entered\n");
			setAccessLED(true);
		} else {
			showMessage("Access Denied");
			printf("[SYSTEM] Wrong code: %s\n", g_input);
			setAccessLED(false);
		}

		delay(2000);
		resetInput();
	}

	if (key == '*') {
		printf("[SYSTEM] Code cleared\n");
		resetInput();
	}
}
