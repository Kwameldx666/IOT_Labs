#include "lab5/lab5_1.hpp"

#include <math.h>
#include <stdio.h>

#include "CustomSTDIO.h"
#include "DriverLed.h"
#include "LCDDisplay.h"
#include "config.h"
#include "relay_driver.hpp"

namespace {

// Три состояния температурной зоны относительно текущей уставки.
enum class ThermalZone { Cold, Normal, Hot };

static RelayDriver g_fanRelay(LAB51_MOTOR_RELAY_PIN, true);
static RelayDriver g_alarmRelay(LAB51_ALARM_RELAY_PIN, true);

// Рабочие переменные контроллера (уставка, температура, флаги).
static float g_setPointC = LAB51_START_SETPOINT_C;
static float g_currentTempC = LAB51_START_SETPOINT_C;
static ThermalZone g_zone = ThermalZone::Normal;
static bool g_fanEnabled = false;
static bool g_alarmEnabled = false;

// Метки времени: антидребезг кнопок и периодические обновления.
static unsigned long g_lastBtnUpMs = 0;
static unsigned long g_lastBtnDownMs = 0;
static unsigned long g_lastLcdMs = 0;
static unsigned long g_lastTelemetryMs = 0;

// Перевод значения перечисления в строку для LCD/Serial.
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

// Управление RGB-светодиодом для наглядного отображения зоны.
void setLedState(ThermalZone zone) {
	switch (zone) {
		case ThermalZone::Cold:
			ledOff(LAB51_LED_RED_PIN);
			ledOff(LAB51_LED_GREEN_PIN);
			ledOn(LAB51_LED_BLUE_PIN);
			break;
		case ThermalZone::Normal:
			ledOff(LAB51_LED_RED_PIN);
			ledOn(LAB51_LED_GREEN_PIN);
			ledOff(LAB51_LED_BLUE_PIN);
			break;
		case ThermalZone::Hot:
			ledOn(LAB51_LED_RED_PIN);
			ledOff(LAB51_LED_GREEN_PIN);
			ledOff(LAB51_LED_BLUE_PIN);
			break;
	}
}

// Чтение датчика TMP36 и пересчёт в градусы Цельсия.
float readTemperatureC() {
	const int raw = analogRead(LAB51_TMP36_PIN);
	const float voltage = raw * (5.0f / 1023.0f);
	return (voltage - 0.5f) * 100.0f;
}

// Возвращает true, если кнопка корректно нажата и уставка изменена.
bool tryAdjustSetPoint(int pin, unsigned long& lastTrigger, float delta) {
	const unsigned long now = millis();
	if (digitalRead(pin) == LOW && (now - lastTrigger) >= LAB51_DEBOUNCE_MS) {
		g_setPointC += delta;
		lastTrigger = now;
		return true;
	}
	return false;
}

// Основная логика: определяем зону, вентилятор и тревогу.
bool applyThermalLogic() {
	const float delta = g_currentTempC - g_setPointC;
	ThermalZone newZone = g_zone;
	bool fan = false;
	bool alarm = false;

	if (delta < -LAB51_NORMAL_RANGE_C) {
		newZone = ThermalZone::Cold;
	} else if (fabsf(delta) <= LAB51_NORMAL_RANGE_C) {
		newZone = ThermalZone::Normal;
	} else {
		newZone = ThermalZone::Hot;
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
		g_fanRelay.turnOn();
	} else {
		g_fanRelay.turnOff();
	}

	if (g_alarmEnabled) {
		g_alarmRelay.turnOn();
	} else {
		g_alarmRelay.turnOff();
	}

	setLedState(g_zone);
	return true;
}

// Две строки LCD: сверху уставка, снизу измерение и зона.
void refreshLcd() {
	lcdPrintLine(0, "SP:%5.1fC", g_setPointC);
	lcdPrintLine(1, "T:%5.1fC %-6s", g_currentTempC, zoneToString(g_zone));
}

// Лог в Serial, соответствующий показаниям на дисплее.
void logTelemetry() {
	printf("[Lab5.1] T=%.1fC | SP=%.1fC | Zone=%s | Fan=%s | Alarm=%s\r\n",
				 g_currentTempC, g_setPointC, zoneToString(g_zone),
				 g_fanEnabled ? "ON" : "OFF",
				 g_alarmEnabled ? "ON" : "OFF");
}

}  // namespace

void setup_lab5_1() {
	StdioSerialSetup();
	lcdSetup();

	pinMode(LAB51_TMP36_PIN, INPUT);
	pinMode(LAB51_BUTTON_UP_PIN, INPUT_PULLUP);
	pinMode(LAB51_BUTTON_DOWN_PIN, INPUT_PULLUP);

	ledSetup(LAB51_LED_RED_PIN);
	ledSetup(LAB51_LED_GREEN_PIN);
	ledSetup(LAB51_LED_BLUE_PIN);
	g_zone = ThermalZone::Normal;
	setLedState(g_zone);

	// Реле по умолчанию выключены (ждём решения логики).
	g_fanRelay.begin();
	g_alarmRelay.begin();
	g_fanRelay.turnOff();
	g_alarmRelay.turnOff();

	// Первичное чтение датчика нужно, чтобы не было рывка при старте.
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
