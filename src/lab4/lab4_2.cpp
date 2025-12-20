#include "lab4/lab4_2.hpp"

#include "CustomSTDIO.h"
#include "LCDDisplay.h"
#include "MotorDriver.h"
#include "relay_driver.hpp"

static bool g_ledOn = false;
static bool g_helpPrinted = false;

static void showLines(const char* first, const char* second) {
	clearScreen();
	lcdPrint(first);
	lcdSetCursor(0, 1);
	lcdPrint(second);
}

static void updateMotorLine(const char* text) {
	lcdSetCursor(0, 1);
	lcdPrint("                ");
	lcdSetCursor(0, 1);
	lcdPrint(text);
}

static void controlLed(bool turnOn) {
	if (turnOn) {
		relay_on();
	} else {
		relay_off();
	}
	showLines(turnOn ? "Led is ON" : "Led is OFF", " ");
	g_ledOn = turnOn;
	printf("[Lab4.2] LED switched %s\r\n", turnOn ? "ON" : "OFF");
}

static void controlMotor(int percent) {
	motorSetPercent(percent);
	const int appliedPercent = motorGetPercent();

	char line2[17];
	if (appliedPercent == 0) {
		snprintf(line2, sizeof(line2), "Motor is OFF     ");
	} else {
		snprintf(line2, sizeof(line2), "Motor Speed:%4d%%", appliedPercent);
	}
	updateMotorLine(line2);
	printf("[Lab4.2] Motor speed set to %d%%\r\n", appliedPercent);
}

void setup_lab4_2() {
	StdioSerialSetup();
	lcdSetup();

	motorBegin(LAB42_MOTOR_ENABLE_PIN, LAB42_MOTOR_PIN1, LAB42_MOTOR_PIN2);
	relay_init(LAB41_LIGHT_RELAY_PIN, true);

	lcdPrint("System Ready");
	controlMotor(0);
}

void loop_lab4_2() {
	if (!g_helpPrinted) {
		printf("[Lab4.2] Available commands:\r\n");
		printf("  on/off      -> turn LED relay on/off\r\n");
		printf("  speed<N>    -> set motor speed from -100 to 100\r\n");
		printf("  status      -> print current LED/motor state\r\n");
		printf("\r\nEnter command:\r\n");
		g_helpPrinted = true;
	}

	if (!stdioHasData()) {
		return;
	}

	String command = stdioGetString();
	command.trim();

	if (command.equalsIgnoreCase("on")) {
		controlLed(true);
	} else if (command.equalsIgnoreCase("off")) {
		controlLed(false);
	} else if (command.startsWith("speed")) {
		int value = command.substring(5).toInt();
		controlMotor(value);
	} else if (command.equalsIgnoreCase("status")) {
		printf("[Lab4.2] LED: %s | Motor: %d%%\n", g_ledOn ? "ON" : "OFF",
					 motorGetPercent());
	} else {
		clearScreen();
		lcdPrint("Unknown cmd");
		printf("[Lab4.2] Unknown command: %s\r\n", command.c_str());
		printf("Use: on | off | speed<N> | status\r\n");
	}

	printf("Enter command:\r\n");
}
