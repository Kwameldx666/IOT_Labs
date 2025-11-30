#include "lab3/tasks/task_button.hpp"

#include <Arduino_FreeRTOS.h>

#include "DriverButton.h"
#include "DriverLed.h"
#include "config.h"
#include "lab3/lab3_1.hpp"

void taskButtonComponent(void* /*pvParameters*/) {
	const TickType_t period = pdMS_TO_TICKS(50);
	TickType_t lastWake = xTaskGetTickCount();

	for (;;) {
		if (isButtonPressed(LAB31_BUTTON_PIN)) {
			changeRunningMode();
			vTaskDelay(pdMS_TO_TICKS(200));
		}
		vTaskDelayUntil(&lastWake, period);
	}
}
