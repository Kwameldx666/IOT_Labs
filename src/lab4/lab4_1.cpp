#include "lab4/lab4_1.hpp"

#include "CustomSTDIO.h"
#include "LCDDisplay.h"
#include "relay_driver.hpp"

static RelayDriver g_relay(LAB41_LIGHT_RELAY_PIN, true);
static bool g_ledOn = false;

static void controlLed(bool turnOn) {
	if (turnOn) {
		g_relay.turnOn();
		clearScreen();
		lcdPrint("Led is ON");
	} else {
		g_relay.turnOff();
		clearScreen();
		lcdPrint("Led is OFF");
	}
	g_ledOn = turnOn;
}

void setup_lab4_1() {
	StdioSerialSetup();
	lcdSetup();
	g_relay.begin();

	controlLed(false);
	printf("Lab4.1 ready. Commands: on/off.\n");
}

void loop_lab4_1() {
	if (!stdioHasData()) {
		return;
	}

	String command = stdioGetString();
	command.trim();

	if (command.equalsIgnoreCase("on")) {
		controlLed(true);
	} else if (command.equalsIgnoreCase("off")) {
		controlLed(false);
	} else if (command.equalsIgnoreCase("status")) {
		printf("[Lab4.1] LED is %s\n", g_ledOn ? "ON" : "OFF");
	} else {
		clearScreen();
		lcdPrint("Unknown cmd");
	}
}
