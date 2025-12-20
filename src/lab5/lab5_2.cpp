#include "lab5/lab5_2.hpp"

#include <math.h>
#include <stdio.h>

#include "CustomSTDIO.h"
#include "DriverLed.h"
#include "LCDDisplay.h"
#include "config.h"

enum ThermalZone { THERMAL_COLD = 0, THERMAL_NORMAL, THERMAL_HOT };

static double g_setPointC = LAB52_START_SETPOINT_C;
static double g_inputC = LAB52_START_SETPOINT_C;
static double g_outputPwm = 0.0;
static ThermalZone g_zone = THERMAL_NORMAL;
static int g_lastRawAdc = 0;
static float g_lastVoltage = 0.0f;

static unsigned long g_lastBtnUpMs = 0;
static unsigned long g_lastBtnDownMs = 0;
static unsigned long g_lastLcdMs = 0;
static unsigned long g_lastTelemetryMs = 0;

// PID state (pure C)
static double g_pidIntegral = 0.0;
static double g_pidPrevError = 0.0;
static unsigned long g_lastComputeMs = 0;

#ifndef LAB5_RGB_ACTIVE_HIGH
#define LAB5_RGB_ACTIVE_HIGH 0
#endif
static constexpr bool kLedActiveHigh = (LAB5_RGB_ACTIVE_HIGH != 0);

static const char* zoneToString(ThermalZone zone) {
	switch (zone) {
		case THERMAL_COLD: return "COLD";
		case THERMAL_NORMAL: return "NORMAL";
		case THERMAL_HOT: return "HOT";
		default: return "?";
	}
}

static float readTemperatureC() {
	const int raw = analogRead(LAB52_TMP36_PIN);
	g_lastRawAdc = raw;
	g_lastVoltage = raw * (LAB5_ADC_VREF / 1023.0f);
	float tempC = LAB5_USE_LM35
									? g_lastVoltage * 100.0f                   // LM35: 10 mV/°C
									: (g_lastVoltage - 0.5f) * 100.0f;          // TMP36: offset 0.5 V
	return tempC + LAB5_TEMP_OFFSET_C;
}

static bool tryAdjustSetPoint(int pin, unsigned long& lastTrigger, float delta) {
	const unsigned long now = millis();
	if (digitalRead(pin) == LOW && (now - lastTrigger) >= LAB52_BTN_DEBOUNCE_MS) {
		g_setPointC += delta;
		lastTrigger = now;
		return true;
	}
	return false;
}

static ThermalZone calculateZone(double tempC, double setPointC) {
	const double delta = tempC - setPointC;
	if (delta < -LAB52_LED_BAND_C) {
		return THERMAL_COLD;
	}
	if (delta > LAB52_LED_BAND_C) {
		return THERMAL_HOT;
	}
	return THERMAL_NORMAL;
}

static void writeLed(int pin, bool on) {
	digitalWrite(pin, kLedActiveHigh ? (on ? HIGH : LOW) : (on ? LOW : HIGH));
}

static void setLedState(ThermalZone zone) {
	// RED = HOT, YELLOW (R+G) = NORMAL, BLUE = COLD
	const bool redOn = (zone == THERMAL_HOT) || (zone == THERMAL_NORMAL);
	const bool greenOn = (zone == THERMAL_NORMAL);
	const bool blueOn = (zone == THERMAL_COLD);
	writeLed(LAB52_LED_RED_PIN, redOn);
	writeLed(LAB52_LED_GREEN_PIN, greenOn);
	writeLed(LAB52_LED_BLUE_PIN, blueOn);
}

static int clampPwm(double pwm) {
	int value = lround(pwm);
	if (value < LAB52_PID_OUTPUT_MIN) {
		value = LAB52_PID_OUTPUT_MIN;
	} else if (value > LAB52_PID_OUTPUT_MAX) {
		value = LAB52_PID_OUTPUT_MAX;
	}
	return value;
}

static double runPid(double setPoint, double input) {
	const unsigned long now = millis();
	if (g_lastComputeMs != 0 && (now - g_lastComputeMs) < LAB52_PID_SAMPLE_MS) {
		return g_outputPwm;  // respect sample time
	}

	const double dtSec = (g_lastComputeMs == 0)
												 ? (LAB52_PID_SAMPLE_MS / 1000.0)
												 : ((now - g_lastComputeMs) / 1000.0);
	g_lastComputeMs = now;

	const double error = setPoint - input;
	g_pidIntegral += error * dtSec;
	const double derivative = (dtSec > 0.0) ? (error - g_pidPrevError) / dtSec : 0.0;

	double output = (LAB52_PID_KP * error) + (LAB52_PID_KI * g_pidIntegral) +
								 (LAB52_PID_KD * derivative);

	g_pidPrevError = error;

	if (output < LAB52_PID_OUTPUT_MIN) output = LAB52_PID_OUTPUT_MIN;
	if (output > LAB52_PID_OUTPUT_MAX) output = LAB52_PID_OUTPUT_MAX;

	g_outputPwm = output;
	return output;
}

static void refreshLcd() {
	const int pwmValue = clampPwm(g_outputPwm);

	char bufSp[8];
	char bufT[8];
	dtostrf(g_setPointC, 4, 1, bufSp);
	dtostrf(g_inputC, 4, 1, bufT);

	char line0[17];
	snprintf(line0, sizeof(line0), "SP:%sC PWM:%3d", bufSp, pwmValue);
	lcdPrintLine(0, "%s", line0);

	char line1[17];
	snprintf(line1, sizeof(line1), "T:%sC %-6s", bufT, zoneToString(g_zone));
	lcdPrintLine(1, "%s", line1);
}

static void logState(int pwmValue) {
	char bufT[12];
	char bufSP[12];
	char bufV[12];
	dtostrf(g_inputC, 4, 1, bufT);
	dtostrf(g_setPointC, 4, 1, bufSP);
	dtostrf(g_lastVoltage, 4, 3, bufV);

	printf("[Lab5.2] T=%sC | SP=%sC | PWM=%3d | Zone=%s\r\n", bufT, bufSP,
				 pwmValue, zoneToString(g_zone));
	printf("[Lab5.2] ADC=%d | V=%sV\r\n", g_lastRawAdc, bufV);
	stdioFlush();
}

static void logPlotter() {
	char bufSP[12];
	char bufIn[12];
	char bufOut[12];
	dtostrf(g_setPointC, 5, 2, bufSP);
	dtostrf(g_inputC, 5, 2, bufIn);
	dtostrf(g_outputPwm, 5, 2, bufOut);
	printf("SetPoint:%s Input:%s Output:%s\r\n", bufSP, bufIn, bufOut);
	stdioFlush();
}

void setup_lab5_2() {
	StdioSerialSetup();
	lcdSetup();
	lcdPrintLine(0, "Lab5.2 starting");
	lcdPrintLine(1, "Init display...");
	analogReference(DEFAULT);

	pinMode(LAB52_TMP36_PIN, INPUT);
	pinMode(LAB52_PWM_PIN, OUTPUT);
	pinMode(LAB52_BUTTON_UP_PIN, INPUT_PULLUP);
	pinMode(LAB52_BUTTON_DOWN_PIN, INPUT_PULLUP);

	pinMode(LAB52_LED_RED_PIN, OUTPUT);
	pinMode(LAB52_LED_GREEN_PIN, OUTPUT);
	pinMode(LAB52_LED_BLUE_PIN, OUTPUT);
	writeLed(LAB52_LED_RED_PIN, false);
	writeLed(LAB52_LED_GREEN_PIN, false);
	writeLed(LAB52_LED_BLUE_PIN, false);

	g_inputC = readTemperatureC();
	g_zone = calculateZone(g_inputC, g_setPointC);
	setLedState(g_zone);

	g_pidIntegral = 0.0;
	g_pidPrevError = 0.0;
	g_lastComputeMs = 0;

	runPid(g_setPointC, g_inputC);  // prime controller
	analogWrite(LAB52_PWM_PIN, clampPwm(g_outputPwm));

	refreshLcd();
	const unsigned long now = millis();
	g_lastLcdMs = now;
	g_lastTelemetryMs = now;

	logState(clampPwm(g_outputPwm));
	logPlotter();

	char bufStep[8];
	char bufKp[10];
	char bufKi[10];
	char bufKd[10];
	dtostrf(LAB52_STEP_SETPOINT_C, 4, 1, bufStep);
	dtostrf(LAB52_PID_KP, 5, 1, bufKp);
	dtostrf(LAB52_PID_KI, 5, 1, bufKi);
	dtostrf(LAB52_PID_KD, 5, 1, bufKd);

	printf("[Lab5.2] PID temperature controller ready.\r\n");
	printf("Buttons D%d(up)/D%d(down), step=%sC.\r\n", LAB52_BUTTON_UP_PIN,
				 LAB52_BUTTON_DOWN_PIN, bufStep);
	printf("PID: Kp=%s Ki=%s Kd=%s | PWM on D%d | Sensor on A%d\r\n\r\n",
				 bufKp, bufKi, bufKd, LAB52_PWM_PIN, LAB52_TMP36_PIN);
}

void loop_lab5_2() {
	const unsigned long now = millis();

	bool setPointChanged = false;
	setPointChanged |= tryAdjustSetPoint(LAB52_BUTTON_UP_PIN, g_lastBtnUpMs,
																			 LAB52_STEP_SETPOINT_C);
	setPointChanged |= tryAdjustSetPoint(LAB52_BUTTON_DOWN_PIN, g_lastBtnDownMs,
																			 -LAB52_STEP_SETPOINT_C);

	g_inputC = readTemperatureC();

	runPid(g_setPointC, g_inputC);
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
