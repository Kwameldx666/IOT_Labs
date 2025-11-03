/**
 * @file lab3_1.cpp
 * @brief Lab 3.1 - Sensor Data Acquisition with FreeRTOS (Fixed for Wokwi)
 * 
 * Complete modular implementation using:
 * - AnalogSensor library for sensor reading
 * - ThresholdManager library for alert management
 * - StatusIndicator library for visual feedback
 * - FreeRTOS for task scheduling
 * 
 * Architecture:
 * - Task 1: Sensor reading (500ms period)
 * - Task 2: Status reporting (500ms period, 100ms offset)
 * - Shared data protected by mutex
 * 
 * Hardware (Wokwi):
 * - Potentiometer on A0 (simulates sensor)
 * - Green LED on Pin 10 (normal status)
 * - Red LED on Pin 12 (warning/critical)
 * - LCD I2C display (optional, for visual feedback)
 */

#include "lab3_1.hpp"

#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include <semphr.h>
#include <stdio.h>

#include "config.h"
#include "dd_serial.hpp"

// ============================================================================
// Shared State (Protected by Mutex)
// ============================================================================

enum class SimpleLevel { NORMAL, WARNING, CRITICAL };

struct Lab3State {
  float currentValue;
  float minValue;
  float maxValue;
  uint32_t sampleCount;
  SimpleLevel currentLevel;
  bool levelChanged;
  unsigned long lastUpdateTime;
};

namespace {
  Lab3State gState = {0.0f, 0.0f, 0.0f, 0, SimpleLevel::NORMAL, false, 0};
  
  // FreeRTOS synchronization
  SemaphoreHandle_t gStateMutex = nullptr;
  TaskHandle_t gSensorTaskHandle = nullptr;
  TaskHandle_t gReportTaskHandle = nullptr;

  // Simple threshold constants
  constexpr float kWarn = (float)LAB3_THRESHOLD_WARNING;
  constexpr float kCrit = (float)LAB3_THRESHOLD_CRITICAL;
  constexpr float kHyst = 10.0f; // hysteresis to return to NORMAL
}

// ============================================================================
// FreeRTOS Task 1: Sensor Reading
// ============================================================================

static void taskSensorRead(void* pvParameters) {
  (void)pvParameters;
  
  TickType_t xLastWakeTime = xTaskGetTickCount();
  const TickType_t xPeriod = pdMS_TO_TICKS(LAB3_SENSOR_READ_PERIOD_MS);
  
  // Initial offset
  vTaskDelay(pdMS_TO_TICKS(LAB3_SENSOR_TASK_OFFSET_MS));
  
  for (;;) {
    // Read sensor directly
    int raw = analogRead(LAB3_SENSOR_PIN);
    float value = (float)raw; // ADC units for simplicity

    // Determine level with simple hysteresis
    SimpleLevel newLevel;
    if (value >= kCrit) newLevel = SimpleLevel::CRITICAL;
    else if (value >= kWarn) newLevel = SimpleLevel::WARNING;
    else if (value <= (kWarn - kHyst)) newLevel = SimpleLevel::NORMAL;
    else newLevel = gState.currentLevel; // within dead-band, keep previous

    // Update shared state (protected by mutex)
    if (xSemaphoreTake(gStateMutex, portMAX_DELAY) == pdTRUE) {
      gState.currentValue = value;
      if (gState.sampleCount == 0) {
        gState.minValue = value;
        gState.maxValue = value;
      } else {
        if (value < gState.minValue) gState.minValue = value;
        if (value > gState.maxValue) gState.maxValue = value;
      }
      gState.sampleCount++;
      bool changed = (newLevel != gState.currentLevel);
      gState.currentLevel = newLevel;
      gState.levelChanged = changed;
      gState.lastUpdateTime = millis();
      xSemaphoreGive(gStateMutex);
    }

    // LED indication (simple)
    switch (newLevel) {
      case SimpleLevel::NORMAL:
        digitalWrite(GREEN_LED_PIN, HIGH);
        digitalWrite(RED_LED_PIN, LOW);
        break;
      case SimpleLevel::WARNING:
      case SimpleLevel::CRITICAL:
        digitalWrite(GREEN_LED_PIN, LOW);
        digitalWrite(RED_LED_PIN, HIGH);
        break;
    }

    // Wait for next period (FreeRTOS periodic task)
    vTaskDelayUntil(&xLastWakeTime, xPeriod);
  }
}

// ============================================================================
// FreeRTOS Task 2: Status Reporting
// ============================================================================

static void taskStatusReport(void* pvParameters) {
  (void)pvParameters;
  
  TickType_t xLastWakeTime = xTaskGetTickCount();
  const TickType_t xPeriod = pdMS_TO_TICKS(LAB3_REPORT_PERIOD_MS);
  
  // Initial offset
  vTaskDelay(pdMS_TO_TICKS(LAB3_REPORT_TASK_OFFSET_MS));
  
  for (;;) {
    // Read shared state (protected by mutex)
    Lab3State localState;
    if (xSemaphoreTake(gStateMutex, portMAX_DELAY) == pdTRUE) {
      localState = gState;
      xSemaphoreGive(gStateMutex);
    }
    
    // Print structured report using STDIO (printf)
    printf("\n========== Lab 3.1: Sensor Status Report ==========%s", "\n");
    printf("Time:        %lu ms\n", (unsigned long)localState.lastUpdateTime);
    printf("---------------------------------------------------\n");
  printf("Sensor Value:   %u (ADC units)\n", (unsigned)localState.currentValue);
  printf("Min Value:      %u\n", (unsigned)localState.minValue);
  printf("Max Value:      %u\n", (unsigned)localState.maxValue);
    printf("Sample Count:   %lu\n", (unsigned long)localState.sampleCount);
    printf("---------------------------------------------------\n");
    const char* levelStr = (localState.currentLevel == SimpleLevel::NORMAL)
         ? "NORMAL"
         : (localState.currentLevel == SimpleLevel::WARNING ? "WARNING" : "CRITICAL");
    printf("Alert Level:    %s%s\n", levelStr, localState.levelChanged ? " [CHANGED!]" : "");
  printf("Thresholds:     WARNING=%u, CRITICAL=%u\n", (unsigned)kWarn, (unsigned)kCrit);
    printf("===================================================\n");
    
    // Additional improvised behavior: Alert if critical
    if (localState.currentLevel == SimpleLevel::CRITICAL) {
      printf("*** ALERT: CRITICAL THRESHOLD EXCEEDED! ***\n");
      printf("*** Action Required: Check sensor immediately! ***\n");
    }
    // No LCD in simplified variant
    
    // Wait for next period
    vTaskDelayUntil(&xLastWakeTime, xPeriod);
  }
}

// ============================================================================
// Public Interface
// ============================================================================

void setup_lab3_1() {
  // Note: Serial already initialized by appManagerSetup()

  // Setup LED pins
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(RED_LED_PIN, OUTPUT);
  digitalWrite(GREEN_LED_PIN, LOW);
  digitalWrite(RED_LED_PIN, LOW);

  // Create mutex for shared state
  gStateMutex = xSemaphoreCreateMutex();
  
  if (gStateMutex == nullptr) {
    Serial.println(F("[Lab 3.1 ERROR] Failed to create mutex!"));
    while (1);  // Halt on error
  }
  
  // Print startup information using printf (STDIO)
  printf("\n================================================\n");
  printf("   Lab 3.1: Sensor Data Acquisition System\n");
  printf("================================================\n");
  printf("Sensor Pin:      A0\n");
  printf("Read Period:     %u ms\n", (unsigned)LAB3_SENSOR_READ_PERIOD_MS);
  printf("Report Period:   %u ms\n", (unsigned)LAB3_REPORT_PERIOD_MS);
  printf("Warning Thresh:  %u\n", (unsigned)LAB3_THRESHOLD_WARNING);
  printf("Critical Thresh: %u\n", (unsigned)LAB3_THRESHOLD_CRITICAL);
  printf("================================================\n");
  printf("Using FreeRTOS for task scheduling...\n");
  printf("Tasks: [Sensor Read] + [Status Report]\n\n");
  
  // Create FreeRTOS tasks (simplified)
  printf("Creating tasks...\n");
  printf("Creating Task 1...\n");
  
  BaseType_t result = xTaskCreate(
      taskSensorRead,
      "Sensor",
    LAB3_SENSOR_TASK_STACK,
      nullptr,
      LAB3_SENSOR_TASK_PRIORITY,
      &gSensorTaskHandle
  );
  
  if (result != pdPASS) {
    printf("[ERROR] Task 1 failed! Result: %d\n", (int)result);
    while (1);
  }
  printf("Task 1 OK\n");
  
  printf("Creating Task 2...\n");
  
  result = xTaskCreate(
      taskStatusReport,
      "Report",
    LAB3_REPORT_TASK_STACK,
      nullptr,
      LAB3_REPORT_TASK_PRIORITY,
      &gReportTaskHandle
  );
  
  if (result != pdPASS) {
    printf("[ERROR] Task 2 failed! Result: %d\n", (int)result);
    while (1);
  }
  printf("Task 2 OK\n");
  
  printf("[Lab 3.1] Tasks created successfully!\n");
  printf("[Lab 3.1] Scheduler will start automatically.\n\n");
}

void loop_lab3_1() {
  // Not used - FreeRTOS scheduler takes full control
  // This function is never called after vTaskStartScheduler()
}

