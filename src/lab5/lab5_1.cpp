#include "lab5/lab5_1.hpp"

#include <math.h>
#include <stdio.h>

#include "CustomSTDIO.h"
#include "DriverLed.h"
#include "LCDDisplay.h"
#include "config.h"
#include "relay_driver.hpp"

// Temperature bands around setpoint
enum ThermalZone { THERMAL_COLD = 0, THERMAL_NORMAL, THERMAL_HOT };

static RelayHandle g_fanRelay = {0, true, RelayState::OFF, false};
static RelayHandle g_alarmRelay = {0, true, RelayState::OFF, false};

static float g_setPointC = LAB51_START_SETPOINT_C;
static float g_currentTempC = LAB51_START_SETPOINT_C;
static ThermalZone g_zone = THERMAL_NORMAL;
static bool g_fanEnabled = false;
static bool g_alarmEnabled = false;

static unsigned long g_lastBtnUpMs = 0;
static unsigned long g_lastBtnDownMs = 0;
static unsigned long g_lastLcdMs = 0;
static unsigned long g_lastTelemetryMs = 0;

static const char* zoneToString(ThermalZone zone) {
	switch (zone) {
		case THERMAL_COLD: return "COLD";
		case THERMAL_NORMAL: return "NORMAL";
		case THERMAL_HOT: return "HOT";
		default: return "?";
	}
}

static void setLedState(ThermalZone zone) {
	switch (zone) {
		case THERMAL_COLD:
			ledOff(LAB51_LED_RED_PIN);
			ledOff(LAB51_LED_GREEN_PIN);
			ledOn(LAB51_LED_BLUE_PIN);
			break;
		case THERMAL_NORMAL:
			ledOff(LAB51_LED_RED_PIN);
			ledOn(LAB51_LED_GREEN_PIN);
			ledOff(LAB51_LED_BLUE_PIN);
			break;
		case THERMAL_HOT:
			ledOn(LAB51_LED_RED_PIN);
			ledOff(LAB51_LED_GREEN_PIN);
			ledOff(LAB51_LED_BLUE_PIN);
			break;
	}
}

static float readTemperatureC() {
	const int raw = analogRead(LAB51_TMP36_PIN);
	const float voltage = raw * (LAB5_ADC_VREF / 1023.0f);
	float tempC = LAB5_USE_LM35
									? voltage * 100.0f                   // LM35: 10 mV/°C
									: (voltage - 0.5f) * 100.0f;          // TMP36: offset 0.5 V
	return tempC + LAB5_TEMP_OFFSET_C;
}

static bool tryAdjustSetPoint(int pin, unsigned long& lastTrigger, float delta) {
	const unsigned long now = millis();
	if (digitalRead(pin) == LOW && (now - lastTrigger) >= LAB51_DEBOUNCE_MS) {
		g_setPointC += delta;
		lastTrigger = now;
		return true;
	}
	return false;
}

static bool applyThermalLogic() {
	const float delta = g_currentTempC - g_setPointC;
	ThermalZone newZone = g_zone;
	bool fan = false;
	bool alarm = false;

	if (delta < -LAB51_NORMAL_RANGE_C) {
		newZone = THERMAL_COLD;
	} else if (fabsf(delta) <= LAB51_NORMAL_RANGE_C) {
		newZone = THERMAL_NORMAL;
	} else {
		newZone = THERMAL_HOT;
		fan = true;
		alarm = (delta >= LAB51_EXTREME_DELTA_C);
	}

	const bool changed = (newZone != g_zone) || (fan != g_fanEnabled) ||
											 (alarm != g_alarmEnabled);
	if (!changed) {
		return false;
	}

	g_zone = newZone;
	g_fanEnabled = fan;
	g_alarmEnabled = alarm;

	if (g_fanEnabled) {
		relay_handle_on(&g_fanRelay);
	} else {
		relay_handle_off(&g_fanRelay);
	}

	if (g_alarmEnabled) {
		relay_handle_on(&g_alarmRelay);
	} else {
		relay_handle_off(&g_alarmRelay);
	}

	setLedState(g_zone);
	return true;
}

static void refreshLcd() {
	lcdPrintLine(0, "SP:%5.1fC", g_setPointC);
	lcdPrintLine(1, "T:%5.1fC %-6s", g_currentTempC, zoneToString(g_zone));
}

static void logTelemetry() {
	printf("[Lab5.1] T=%.1fC | SP=%.1fC | Zone=%s | Fan=%s | Alarm=%s\r\n",
				 g_currentTempC, g_setPointC, zoneToString(g_zone),
				 g_fanEnabled ? "ON" : "OFF",
				 g_alarmEnabled ? "ON" : "OFF");
}

void setup_lab5_1() {
	StdioSerialSetup();
	lcdSetup();

	pinMode(LAB51_TMP36_PIN, INPUT);
	pinMode(LAB51_BUTTON_UP_PIN, INPUT_PULLUP);
	pinMode(LAB51_BUTTON_DOWN_PIN, INPUT_PULLUP);

	ledSetup(LAB51_LED_RED_PIN);
	ledSetup(LAB51_LED_GREEN_PIN);
	ledSetup(LAB51_LED_BLUE_PIN);
	g_zone = THERMAL_NORMAL;
	setLedState(g_zone);

	relay_handle_init(&g_fanRelay, LAB51_MOTOR_RELAY_PIN, true);
	relay_handle_init(&g_alarmRelay, LAB51_ALARM_RELAY_PIN, true);
	relay_handle_off(&g_fanRelay);
	relay_handle_off(&g_alarmRelay);

	g_currentTempC = readTemperatureC();
	applyThermalLogic();
	refreshLcd();
	logTelemetry();

	const unsigned long now = millis();
	g_lastLcdMs = now;
	g_lastTelemetryMs = now;

	printf("[Lab5.1] Fan+Alarm controller ready.\r\n");
	printf("Buttons D%d (up) / D%d (down) adjust setpoint by %.1fC.\r\n",
				 LAB51_BUTTON_UP_PIN, LAB51_BUTTON_DOWN_PIN, LAB51_STEP_SETPOINT_C);
	printf("Normal zone: +/-%.1fC, Alarm delta: %.1fC\r\n\r\n",
				 LAB51_NORMAL_RANGE_C, LAB51_EXTREME_DELTA_C);
}

void loop_lab5_1() {
	bool setPointChanged = false;
	setPointChanged |= tryAdjustSetPoint(LAB51_BUTTON_UP_PIN, g_lastBtnUpMs,
																			 LAB51_STEP_SETPOINT_C);
	setPointChanged |= tryAdjustSetPoint(LAB51_BUTTON_DOWN_PIN, g_lastBtnDownMs,
																			 -LAB51_STEP_SETPOINT_C);

	g_currentTempC = readTemperatureC();
	const bool logicChanged = applyThermalLogic();

	const unsigned long now = millis();
	const bool needLcd = setPointChanged || logicChanged ||
											(now - g_lastLcdMs) >= LAB51_LCD_REFRESH_PERIOD_MS;
	if (needLcd) {
		refreshLcd();
		g_lastLcdMs = now;
	}

	const bool needLog = setPointChanged || logicChanged ||
											(now - g_lastTelemetryMs) >= LAB51_TELEMETRY_PERIOD_MS;
	if (needLog) {
		logTelemetry();
		g_lastTelemetryMs = now;
	}

	delay(10);
}
