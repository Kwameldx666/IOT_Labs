#include "lab3/tasks/task_ultrasonic.hpp"

#include <Arduino_FreeRTOS.h>

#include "config.h"
#include "lab3/lab3_1.hpp"

void taskUltrasonicSensor(void* /*pvParameters*/) {
	const TickType_t period = pdMS_TO_TICKS(1000);
	TickType_t lastWake = xTaskGetTickCount();

	for (;;) {
		const byte mode = getRunningMode();
		if (mode == 1 || mode == 3) {
			digitalWrite(LAB31_PIN_TRIG, LOW);
			delayMicroseconds(2);
			digitalWrite(LAB31_PIN_TRIG, HIGH);
			delayMicroseconds(10);
			digitalWrite(LAB31_PIN_TRIG, LOW);

			const unsigned long duration =
					pulseIn(LAB31_PIN_ECHO, HIGH, 30000UL);  // timeout 30ms
			const int distanceCm = static_cast<int>((duration * 0.034f) / 2.0f);
			updateUltrasonicReadings(static_cast<int>(duration), distanceCm);
		}
		vTaskDelayUntil(&lastWake, period);
	}
}
