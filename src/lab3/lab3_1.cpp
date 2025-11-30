#include "lab3/lab3_1.hpp"

#include <Arduino_FreeRTOS.h>

#include "CustomSTDIO.h"
#include "DriverButton.h"
#include "DriverLed.h"
#include "LCDDisplay.h"
#include "config.h"

#include "lab3/tasks/task_button.hpp"
#include "lab3/tasks/task_gas.hpp"
#include "lab3/tasks/task_ultrasonic.hpp"

static int g_gasAnalogValue = 0;
static int g_gasVoltageMv = 0;
static int g_ultrasonicDurationUs = 0;
static int g_ultrasonicDistanceCm = 0;
static byte g_runningMode = 0;

static void taskReport(void* pvParameters);

int getGasAnalogValue() { return g_gasAnalogValue; }
int getGasVoltageMv() { return g_gasVoltageMv; }
int getUltrasonicDurationUs() { return g_ultrasonicDurationUs; }
int getUltrasonicDistanceCm() { return g_ultrasonicDistanceCm; }
byte getRunningMode() { return g_runningMode; }

void changeRunningMode() {
	g_runningMode = static_cast<byte>((g_runningMode + 1) % 4);
	switch (g_runningMode) {
		case 0:
			ledOff(LAB31_LED_RED_PIN);
			ledOff(LAB31_LED_GREEN_PIN);
			g_gasAnalogValue = g_gasVoltageMv = 0;
			g_ultrasonicDurationUs = g_ultrasonicDistanceCm = 0;
			break;
		case 1:
			ledOn(LAB31_LED_RED_PIN);
			ledOff(LAB31_LED_GREEN_PIN);
			g_gasAnalogValue = g_gasVoltageMv = 0;
			break;
		case 2:
			ledOff(LAB31_LED_RED_PIN);
			ledOn(LAB31_LED_GREEN_PIN);
			g_ultrasonicDurationUs = g_ultrasonicDistanceCm = 0;
			break;
		case 3:
			ledOn(LAB31_LED_RED_PIN);
			ledOn(LAB31_LED_GREEN_PIN);
			break;
	}
}

void updateGasReadings(int analogValue, int voltageMv) {
	g_gasAnalogValue = analogValue;
	g_gasVoltageMv = voltageMv;
}

void updateUltrasonicReadings(int durationUs, int distanceCm) {
	g_ultrasonicDurationUs = durationUs;
	g_ultrasonicDistanceCm = distanceCm;
}

void setup_lab3_1() {
	StdioSerialSetup();
	buttonsSetup();

	pinMode(LAB31_BUTTON_PIN, INPUT_PULLUP);
	ledSetup(LAB31_LED_RED_PIN);
	ledSetup(LAB31_LED_GREEN_PIN);

	pinMode(LAB31_PIN_TRIG, OUTPUT);
	pinMode(LAB31_PIN_ECHO, INPUT);
	pinMode(LAB31_MQ2_PIN, INPUT);

	xTaskCreate(taskButtonComponent, "TaskButton", 128, nullptr, 1, nullptr);
	xTaskCreate(taskUltrasonicSensor, "TaskUltrasonic", 256, nullptr, 1, nullptr);
	xTaskCreate(taskGasSensor, "TaskGas", 256, nullptr, 1, nullptr);
	xTaskCreate(taskReport, "TaskReport", 256, nullptr, 1, nullptr);

	vTaskStartScheduler();
}

void loop_lab3_1() {
	// FreeRTOS scheduler handles tasks.
}

static void taskReport(void* pvParameters) {
	const TickType_t kPeriod = pdMS_TO_TICKS(500);
	TickType_t lastWake = xTaskGetTickCount();

	for (;;) {
		printf("=======================\n");
		printf("Report:\n");
		printf("Gas analog: %d\n", g_gasAnalogValue);
		printf("Gas voltage: %d mV\n", g_gasVoltageMv);
		printf("Duration: %d us\n", g_ultrasonicDurationUs);
		printf("Distance: %d cm\n", g_ultrasonicDistanceCm);
		vTaskDelayUntil(&lastWake, kPeriod);
	}
}
