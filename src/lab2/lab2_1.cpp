#include "lab2/lab2_1.hpp"

#include "CustomSTDIO.h"
#include "DriverLed.h"
#include "config.h"

static bool g_redState = false;
static bool g_greenState = false;
static unsigned long g_lastBlinkMs = 0;
static const unsigned long kBlinkPeriodMs = 500;

void setup_lab2_1() {
	StdioSerialSetup();

	pinMode(LAB21_BUTTON_PIN, INPUT_PULLUP);
	ledSetup(LAB21_LED_RED_PIN);
	ledSetup(LAB21_LED_GREEN_PIN);

	printf("Lab 2.1 ready. Press button to toggle red LED.\n");
}

void loop_lab2_1() {
	const bool buttonPressed = digitalRead(LAB21_BUTTON_PIN) == LOW;
	if (buttonPressed) {
		g_redState = !g_redState;
		changeLedState(LAB21_LED_RED_PIN, g_redState ? HIGH : LOW);
		printf("[Lab2.1] Red LED -> %s\n", g_redState ? "ON" : "OFF");
		delay(250);  // crude debounce
	}

	const unsigned long now = millis();
	if ((now - g_lastBlinkMs) >= kBlinkPeriodMs) {
		g_greenState = !g_greenState;
		changeLedState(LAB21_LED_GREEN_PIN, g_greenState ? HIGH : LOW);
		g_lastBlinkMs = now;
	}
}
