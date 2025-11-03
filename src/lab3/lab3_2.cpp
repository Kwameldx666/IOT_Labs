/**
 * @file lab3_2.cpp
 * @brief Lab 3.2 - Signal Conditioning (100% requirements)
 * 
 * Requirements:
 * 50% - Basic sensor reading + LCD/Serial display via STDIO (printf)
 * 10% - Salt & Pepper digital filter
 * 10% - Weighted Average filter
 * 10% - FreeRTOS tasks with periodic reporting (printf)
 * 10% - Second sensor with full conditioning (HC-SR04 Ultrasonic: SP+WA)
 * 10% - Physical demonstration (Wokwi simulation)
 * 
 * Sensors:
 * - Sensor 1: Potentiometer on A0
 * - Sensor 2: HC-SR04 Ultrasonic on pins 8,9
 * 
 * Signal Processing (both sensors):
 * - Salt & Pepper filter (median, window=5)
 * - Weighted Average filter (window=5)
 * 
 * FreeRTOS:
 * - Task 1: Sampling (100ms, priority 3)
 * - Task 2: Reporting with printf (500ms, priority 1)
 */

#include "lab3_2.hpp"

#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include <semphr.h>
#include <stdio.h>

#include "config.h"
#include "dd_serial.hpp"
#include "salt_pepper_filter.hpp"
#include "weighted_average_filter.hpp"

// Simple LCD 1602 driver (4-bit mode)
// RS=7, E=6, D4=5, D5=4, D6=3, D7=2
#define LCD_RS 7
#define LCD_E  6
#define LCD_D4 5
#define LCD_D5 4
#define LCD_D6 3
#define LCD_D7 2

void lcdPulseEnable() {
  digitalWrite(LCD_E, LOW);
  delayMicroseconds(1);
  digitalWrite(LCD_E, HIGH);
  delayMicroseconds(1);
  digitalWrite(LCD_E, LOW);
  delayMicroseconds(100);
}

void lcdWrite4Bits(uint8_t value) {
  digitalWrite(LCD_D4, (value >> 0) & 0x01);
  digitalWrite(LCD_D5, (value >> 1) & 0x01);
  digitalWrite(LCD_D6, (value >> 2) & 0x01);
  digitalWrite(LCD_D7, (value >> 3) & 0x01);
  lcdPulseEnable();
}

void lcdSend(uint8_t value, uint8_t mode) {
  digitalWrite(LCD_RS, mode);
  lcdWrite4Bits(value >> 4);
  lcdWrite4Bits(value);
}

void lcdCommand(uint8_t cmd) { lcdSend(cmd, LOW); }
void lcdData(uint8_t data) { lcdSend(data, HIGH); }

void lcdInit() {
  // Set all pins as outputs
  pinMode(LCD_RS, OUTPUT);
  pinMode(LCD_E, OUTPUT);
  pinMode(LCD_D4, OUTPUT);
  pinMode(LCD_D5, OUTPUT);
  pinMode(LCD_D6, OUTPUT);
  pinMode(LCD_D7, OUTPUT);
  
  // Initial state
  digitalWrite(LCD_RS, LOW);
  digitalWrite(LCD_E, LOW);
  digitalWrite(LCD_D4, LOW);
  digitalWrite(LCD_D5, LOW);
  digitalWrite(LCD_D6, LOW);
  digitalWrite(LCD_D7, LOW);
  
  // Wait for LCD power-up (HD44780 datasheet: min 15ms)
  delay(50);
  
  // Initialization sequence for 4-bit mode (by instruction)
  // Step 1: Function set (8-bit mode)
  lcdWrite4Bits(0x03);
  delay(5);
  
  // Step 2: Function set (8-bit mode)
  lcdWrite4Bits(0x03);
  delayMicroseconds(150);
  
  // Step 3: Function set (8-bit mode)
  lcdWrite4Bits(0x03);
  delayMicroseconds(150);
  
  // Step 4: Function set (4-bit mode)
  lcdWrite4Bits(0x02);
  delayMicroseconds(150);
  
  // Now in 4-bit mode, send full commands
  lcdCommand(0x28); // Function set: 4-bit, 2 lines, 5x8 font
  delayMicroseconds(50);
  
  lcdCommand(0x08); // Display off
  delayMicroseconds(50);
  
  lcdCommand(0x01); // Clear display
  delay(2);
  
  lcdCommand(0x06); // Entry mode: increment, no shift
  delayMicroseconds(50);
  
  lcdCommand(0x0C); // Display on, cursor off, blink off
  delayMicroseconds(50);
}

void lcdClear() {
  lcdCommand(0x01);
  delay(2);
}

void lcdSetCursor(uint8_t col, uint8_t row) {
  uint8_t addr = (row == 0) ? 0x00 : 0x40;
  lcdCommand(0x80 | (addr + col));
}

void lcdPrint(const char* str) {
  while (*str) lcdData(*str++);
}

void lcdPrintInt(int value) {
  char buf[12];
  sprintf(buf, "%d", value);
  lcdPrint(buf);
}

// ============================================================================
// Shared State (protected by mutex)
// ============================================================================

// Simplified data structure for sensor readings
struct SensorData {
  uint16_t raw;          // Raw sensor value
  uint16_t filtered;     // After digital filtering
  uint32_t count;        // Sample counter
  
  SensorData() : raw(0), filtered(0), count(0) {}
};

// Global state for two sensors
static SensorData g_sensor1;  // Potentiometer on A0
static SensorData g_sensor2;  // HC-SR04 ultrasonic
static SemaphoreHandle_t g_mutex = NULL;
static bool g_lcdReady = false;

// Digital filters (created in setup)
static SaltPepperFilter* spFilter1 = nullptr;      // Salt&Pepper for sensor 1
static WeightedAverageFilter* waFilter1 = nullptr; // WeightedAvg for sensor 1
static SaltPepperFilter* spFilter2 = nullptr;      // Salt&Pepper for sensor 2
static WeightedAverageFilter* waFilter2 = nullptr; // WeightedAvg for sensor 2

// ============================================================================
// FreeRTOS Tasks
// ============================================================================

/**
 * Task 1: Signal Sampling and Conditioning
 * - Reads both sensors
 * - Applies complete filtering pipeline
 * - Uses vTaskDelayUntil for precise periodic execution
 * - High priority (3) for real-time data acquisition
 */
static void taskSignalSampling(void* pvParameters) {
  (void)pvParameters;
  
  // Initialize timing for periodic execution
  TickType_t xLastWakeTime = xTaskGetTickCount();
  const TickType_t xPeriod = pdMS_TO_TICKS(LAB32_SAMPLING_PERIOD_MS);
  
  // Initial offset
  vTaskDelay(pdMS_TO_TICKS(LAB32_SAMPLING_TASK_OFFSET_MS));
  
  for (;;) {
    // Read Sensor 1: Potentiometer (A0)
    uint16_t adc = analogRead(A0);
    
    // Read Sensor 2: HC-SR04 Ultrasonic
    digitalWrite(8, LOW);
    delayMicroseconds(2);
    digitalWrite(8, HIGH);
    delayMicroseconds(10);
    digitalWrite(8, LOW);
    uint32_t duration = pulseIn(9, HIGH, 30000);
    uint16_t distance = (duration == 0) ? 0 : (duration / 58);
    
    // Apply digital filters and update shared data
    if (xSemaphoreTake(g_mutex, pdMS_TO_TICKS(50)) == pdTRUE) {
      // Sensor 1 (Potentiometer): Salt&Pepper + WeightedAvg filters
      float sp1 = spFilter1->filter((float)adc);
      float wa1 = waFilter1->filter(sp1);
      g_sensor1.raw = adc;
      g_sensor1.filtered = (uint16_t)wa1;
      g_sensor1.count++;
      
      // Sensor 2 (Ultrasonic): Salt&Pepper + WeightedAvg filters (full conditioning)
      float sp2 = spFilter2->filter((float)distance);
      float wa2 = waFilter2->filter(sp2);
      g_sensor2.raw = distance;
      g_sensor2.filtered = (uint16_t)wa2;
      g_sensor2.count++;
      
      xSemaphoreGive(g_mutex);
    }
    
    // LED indicator control based on sensor thresholds
    uint16_t value = adc;  // Use raw ADC for threshold detection
    if (value < LAB32_THRESHOLD_LOW) {
      // Green zone: Normal operation
      digitalWrite(LAB32_LED_GREEN, HIGH);
      digitalWrite(LAB32_LED_YELLOW, LOW);
      digitalWrite(LAB32_LED_RED, LOW);
    } else if (value < LAB32_THRESHOLD_HIGH) {
      // Yellow zone: Warning
      digitalWrite(LAB32_LED_GREEN, LOW);
      digitalWrite(LAB32_LED_YELLOW, HIGH);
      digitalWrite(LAB32_LED_RED, LOW);
    } else {
      // Red zone: Alarm
      digitalWrite(LAB32_LED_GREEN, LOW);
      digitalWrite(LAB32_LED_YELLOW, LOW);
      digitalWrite(LAB32_LED_RED, HIGH);
    }
    
    vTaskDelayUntil(&xLastWakeTime, xPeriod);
  }
}

/**
 * Task 2: Status Reporting
 * - Periodic reporting via STDIO (printf)
 * - Shows complete signal processing pipeline
 * - Displays statistics (min/max/samples)
 * - Uses vTaskDelayUntil for precise timing
 * - Low priority (1) to not interfere with sampling
 * - 50ms offset from sampling task
 */
static void taskStatusReport(void* pvParameters) {
  (void)pvParameters;
  
  // Initialize timing for periodic execution
  TickType_t xLastWakeTime = xTaskGetTickCount();
  const TickType_t xPeriod = pdMS_TO_TICKS(LAB32_REPORT_PERIOD_MS);
  
  // Apply offset to stagger execution from sampling task
  vTaskDelay(pdMS_TO_TICKS(LAB32_REPORT_TASK_OFFSET_MS));
  
  // Initialize LCD here (after FreeRTOS starts) and print startup info
  if (!g_lcdReady) {
    // Print startup info via STDIO
    printf("\n=== Lab 3.2 Started ===\n");
    printf("S1: Pot A0 (SP+WA)\n");
    printf("S2: HC-SR04 pins 8,9 (SP+WA)\n");
    printf("LED: G(10) Y(11) R(12)\n");
    printf("Thresholds: <300=G, 300-700=Y, >700=R\n");
    printf("Tasks: Sample 100ms, Report 500ms\n\n");
    
    // Initialize LCD
    lcdInit();
    g_lcdReady = true;
    vTaskDelay(pdMS_TO_TICKS(100));
    lcdClear();
    lcdSetCursor(0, 0);
    lcdPrint("Lab 3.2");
    lcdSetCursor(0, 1);
    lcdPrint("Starting...");
    vTaskDelay(pdMS_TO_TICKS(500));
  }
  
  uint32_t reportCount = 0;
  
  for (;;) {
    SensorData s1, s2;
    
    // Read shared state (protected by mutex)
    if (xSemaphoreTake(g_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
      s1 = g_sensor1;
      s2 = g_sensor2;
      xSemaphoreGive(g_mutex);
    } else {
      printf("ERROR: Mutex timeout\n");
      vTaskDelayUntil(&xLastWakeTime, xPeriod);
      continue;
    }
    
    reportCount++;
    
    // Determine LED status for display
    const char* ledStatus = "G";  // Green
    if (s1.raw >= LAB32_THRESHOLD_HIGH) {
      ledStatus = "R";  // Red
    } else if (s1.raw >= LAB32_THRESHOLD_LOW) {
      ledStatus = "Y";  // Yellow
    }
    
    // Periodic reporting via STDIO (printf) - requirement for 10%
    // Split into multiple short printf calls to avoid buffer overflow
    printf("\n");
    printf("R%u: ", (unsigned)reportCount);
    printf("Pot=%u/%u ", (unsigned)s1.raw, (unsigned)s1.filtered);
    printf("US=%u/%u ", (unsigned)s2.raw, (unsigned)s2.filtered);
    printf("LED=%s N=%u\n", ledStatus, (unsigned)s1.count);
    
    // LCD display
    if (g_lcdReady) {
      lcdClear();
      lcdSetCursor(0, 0);
      lcdPrint("Pot:");
      lcdPrintInt((int)s1.filtered);
      lcdPrint(" US:");
      lcdPrintInt((int)s2.filtered);
      
      lcdSetCursor(0, 1);
      lcdPrint("N:");
      lcdPrintInt(s1.count);
    }
    
    // Wait for next period (precise timing with vTaskDelayUntil)
    vTaskDelayUntil(&xLastWakeTime, xPeriod);
  }
}

// ============================================================================
// Public Interface
// ============================================================================

void setup_lab3_2() {
  // Initialize HC-SR04 pins
  pinMode(8, OUTPUT);
  pinMode(9, INPUT);
  digitalWrite(8, LOW);
  
  // Initialize LED indicator pins
  pinMode(LAB32_LED_GREEN, OUTPUT);
  pinMode(LAB32_LED_YELLOW, OUTPUT);
  pinMode(LAB32_LED_RED, OUTPUT);
  digitalWrite(LAB32_LED_GREEN, LOW);
  digitalWrite(LAB32_LED_YELLOW, LOW);
  digitalWrite(LAB32_LED_RED, LOW);
  
  // LCD will be initialized in first report task run (not in setup - causes crash)
  g_lcdReady = false;
  
  // Create filters - full conditioning for both sensors (SP + WA)
  spFilter1 = new SaltPepperFilter(LAB32_SALT_PEPPER_WINDOW);
  waFilter1 = new WeightedAverageFilter(LAB32_WEIGHTED_AVG_WINDOW);
  spFilter2 = new SaltPepperFilter(LAB32_SALT_PEPPER_WINDOW);
  waFilter2 = new WeightedAverageFilter(LAB32_WEIGHTED_AVG_WINDOW);
  
  // Create mutex for data protection
  g_mutex = xSemaphoreCreateMutex();
  if (g_mutex == NULL) {
    while (1); // halt on error
  }
  
  // Create FreeRTOS tasks
  BaseType_t r1 = xTaskCreate(
    taskSignalSampling, 
    "Sample", 
    LAB32_SAMPLING_TASK_STACK, 
    NULL, 
    LAB32_SAMPLING_TASK_PRIORITY, 
    NULL
  );
  BaseType_t r2 = xTaskCreate(
    taskStatusReport, 
    "Report", 
    LAB32_REPORT_TASK_STACK, 
    NULL, 
    LAB32_REPORT_TASK_PRIORITY, 
    NULL
  );
  
  if (r1 != pdPASS || r2 != pdPASS) {
    while (1); // halt on error
  }
}

void loop_lab3_2() {
  // Never reached - scheduler takes over
}

