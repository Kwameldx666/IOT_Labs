#include "lab3/tasks/task_gas.hpp"

#include <Arduino_FreeRTOS.h>

#include "config.h"
#include "lab3/lab3_1.hpp"

void taskGasSensor(void* /*pvParameters*/) {
	const TickType_t period = pdMS_TO_TICKS(500);
	TickType_t lastWake = xTaskGetTickCount();

	for (;;) {
		const byte mode = getRunningMode();
		if (mode == 2 || mode == 3) {
			const int analogValue = analogRead(LAB31_MQ2_PIN);
			const int voltageMv = static_cast<int>(analogValue * (5.0f / 1023.0f) *
																							 1000.0f);
			updateGasReadings(analogValue, voltageMv);
		}
		vTaskDelayUntil(&lastWake, period);
	}
}
