#include "lab5/lab5_2.hpp"

#include <PID_v1.h>
#include <math.h>
#include <stdio.h>

#include "CustomSTDIO.h"
#include "DriverLed.h"
#include "LCDDisplay.h"
#include "config.h"

namespace {

enum class ThermalZone { Cold, Normal, Hot };

double g_setPointC = LAB52_START_SETPOINT_C;
double g_inputC = LAB52_START_SETPOINT_C;
double g_outputPwm = 0.0;

ThermalZone g_zone = ThermalZone::Normal;

PID g_pid(&g_inputC, &g_outputPwm, &g_setPointC, LAB52_PID_KP, LAB52_PID_KI,
				 LAB52_PID_KD, DIRECT);

unsigned long g_lastBtnUpMs = 0;
unsigned long g_lastBtnDownMs = 0;
unsigned long g_lastLcdMs = 0;
unsigned long g_lastTelemetryMs = 0;

const char* zoneToString(ThermalZone zone) {
	switch (zone) {
		case ThermalZone::Cold:
			return "COLD";
		case ThermalZone::Normal:
			return "NORMAL";
		case ThermalZone::Hot:
			return "HOT";
		default:
			return "?";
	}
}

float readTemperatureC() {
	const int raw = analogRead(LAB52_TMP36_PIN);
	const float voltage = raw * (5.0f / 1023.0f);
	return (voltage - 0.5f) * 100.0f;
}

bool tryAdjustSetPoint(int pin, unsigned long& lastTrigger, float delta) {
	const unsigned long now = millis();
	if (digitalRead(pin) == LOW && (now - lastTrigger) >= LAB52_BTN_DEBOUNCE_MS) {
		g_setPointC += delta;
		lastTrigger = now;
		return true;
	}
	return false;
}

ThermalZone calculateZone(double tempC, double setPointC) {
	const double delta = tempC - setPointC;
	if (delta < -LAB52_LED_BAND_C) {
		return ThermalZone::Cold;
	}
	if (delta > LAB52_LED_BAND_C) {
		return ThermalZone::Hot;
	}
	return ThermalZone::Normal;
}

void setLedState(ThermalZone zone) {
	switch (zone) {
		case ThermalZone::Cold:
			ledOff(LAB52_LED_RED_PIN);
			ledOff(LAB52_LED_GREEN_PIN);
			ledOn(LAB52_LED_BLUE_PIN);
			break;
		case ThermalZone::Normal:
			ledOff(LAB52_LED_RED_PIN);
			ledOn(LAB52_LED_GREEN_PIN);
			ledOff(LAB52_LED_BLUE_PIN);
			break;
		case ThermalZone::Hot:
			ledOn(LAB52_LED_RED_PIN);
			ledOff(LAB52_LED_GREEN_PIN);
			ledOff(LAB52_LED_BLUE_PIN);
			break;
	}
}

int clampPwm(double pwm) {
	int value = lround(pwm);
	if (value < LAB52_PID_OUTPUT_MIN) {
		value = LAB52_PID_OUTPUT_MIN;
	} else if (value > LAB52_PID_OUTPUT_MAX) {
		value = LAB52_PID_OUTPUT_MAX;
	}
	return value;
}

void refreshLcd() {
	const int pwmValue = clampPwm(g_outputPwm);
	lcdPrintLine(0, "SP:%4.1fC PWM:%3d", g_setPointC, pwmValue);
	lcdPrintLine(1, "T:%4.1fC %-6s", g_inputC, zoneToString(g_zone));
}

void logState(int pwmValue) {
	printf("[Lab5.2] T=%.1fC | SP=%.1fC | PWM=%3d | Zone=%s\r\n", g_inputC,
				 g_setPointC, pwmValue, zoneToString(g_zone));
}

void logPlotter() {
	printf("SetPoint:%.2f Input:%.2f Output:%.2f\r\n", g_setPointC, g_inputC,
				 g_outputPwm);
}

}  // namespace

void setup_lab5_2() {
	StdioSerialSetup();
	lcdSetup();

	pinMode(LAB52_TMP36_PIN, INPUT);
	pinMode(LAB52_PWM_PIN, OUTPUT);
	pinMode(LAB52_BUTTON_UP_PIN, INPUT_PULLUP);
	pinMode(LAB52_BUTTON_DOWN_PIN, INPUT_PULLUP);

	ledSetup(LAB52_LED_RED_PIN);
	ledSetup(LAB52_LED_GREEN_PIN);
	ledSetup(LAB52_LED_BLUE_PIN);
	setLedState(g_zone);

	g_inputC = readTemperatureC();
	g_zone = calculateZone(g_inputC, g_setPointC);
	setLedState(g_zone);

	g_pid.SetOutputLimits(LAB52_PID_OUTPUT_MIN, LAB52_PID_OUTPUT_MAX);
	g_pid.SetSampleTime(LAB52_PID_SAMPLE_MS);
	g_pid.SetMode(AUTOMATIC);

	// Prime PID with the initial reading
	g_pid.Compute();
	analogWrite(LAB52_PWM_PIN, clampPwm(g_outputPwm));

	refreshLcd();
	const unsigned long now = millis();
	g_lastLcdMs = now;
	g_lastTelemetryMs = now;

	logState(clampPwm(g_outputPwm));
	logPlotter();

	printf("[Lab5.2] PID temperature controller ready.\r\n");
	printf("Buttons D%d(up)/D%d(down), step=%.1fC.\r\n", LAB52_BUTTON_UP_PIN,
				 LAB52_BUTTON_DOWN_PIN, LAB52_STEP_SETPOINT_C);
	printf("PID: Kp=%.1f Ki=%.1f Kd=%.1f | PWM on D%d | Sensor on A%d\r\n\r\n",
				 LAB52_PID_KP, LAB52_PID_KI, LAB52_PID_KD, LAB52_PWM_PIN,
				 LAB52_TMP36_PIN);
}

void loop_lab5_2() {
	const unsigned long now = millis();

	bool setPointChanged = false;
	setPointChanged |= tryAdjustSetPoint(LAB52_BUTTON_UP_PIN, g_lastBtnUpMs,
																			 LAB52_STEP_SETPOINT_C);
	setPointChanged |= tryAdjustSetPoint(LAB52_BUTTON_DOWN_PIN, g_lastBtnDownMs,
																			 -LAB52_STEP_SETPOINT_C);

	g_inputC = readTemperatureC();

	g_pid.Compute();
	const int pwmValue = clampPwm(g_outputPwm);
	analogWrite(LAB52_PWM_PIN, pwmValue);

	const ThermalZone newZone = calculateZone(g_inputC, g_setPointC);
	const bool zoneChanged = newZone != g_zone;
	if (zoneChanged) {
		g_zone = newZone;
		setLedState(g_zone);
	}

	if (setPointChanged || zoneChanged ||
			(now - g_lastLcdMs) >= LAB52_LCD_REFRESH_PERIOD_MS) {
		refreshLcd();
		g_lastLcdMs = now;
	}

	if (setPointChanged || zoneChanged ||
			(now - g_lastTelemetryMs) >= LAB52_TELEMETRY_PERIOD_MS) {
		logState(pwmValue);
		logPlotter();
		g_lastTelemetryMs = now;
	}

	delay(LAB52_LOOP_DELAY_MS);
}
