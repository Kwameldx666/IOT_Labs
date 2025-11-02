/**
 * @file lab3_2.cpp
 * @brief Lab 3.2 - Signal Conditioning with Digital Filters (FreeRTOS)
 * 
 * Features:
 * - Analog sensor sampling (two sensors on A0 and A1)
 * - Salt & Pepper filter library for impulse noise removal
 * - Weighted average filter library for smoothing
 * - SignalConverter library for ADC → Voltage → Physical conversion
 * - Saturation (value clamping)
 * - FreeRTOS tasks with precise timing using vTaskDelayUntil()
 * - STDIO reporting via printf with detailed pipeline visualization
 * - LCD display with signal processing results
 * 
 * Signal Processing Pipeline:
 * Raw ADC → Salt&Pepper → Weighted Avg → ADC→Voltage → Voltage→Physical → Saturation
 * 
 * Architecture:
 * - Task 1: Sampling & Conditioning (100ms period, high priority)
 * - Task 2: Status Reporting (500ms period, low priority, 50ms offset)
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
#include "signal_converter.hpp"

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

struct SignalData {
  // Raw data
  uint16_t rawADC;              // Latest ADC reading (0-1023)
  
  // Filtered values (intermediate steps)
  float filteredSP;             // After Salt & Pepper filter
  float filteredWA;             // After Weighted Average filter
  
  // Converted values
  float voltage;                // Converted voltage (0-5V)
  float physical;               // Physical parameter (e.g. temp in °C)
  
  // Statistics
  uint32_t sampleCount;         // Total samples taken
  float minValue;               // Minimum physical value observed
  float maxValue;               // Maximum physical value observed
  
  // Reset statistics
  void resetStats() {
    minValue = 999999.0f;
    maxValue = -999999.0f;
    sampleCount = 0;
  }
  
  // Update statistics
  void updateStats(float value) {
    if (value < minValue) minValue = value;
    if (value > maxValue) maxValue = value;
    sampleCount++;
  }
};

// Global state for two sensors
static SignalData g_sensor1 = {0};
static SignalData g_sensor2 = {0};
static SemaphoreHandle_t g_mutex = NULL;
static bool g_lcdAvailable = false;

// ============================================================================
// Filter and Converter Objects
// ============================================================================

// Filters for Sensor 1
static SaltPepperFilter* spFilter1 = nullptr;
static WeightedAverageFilter* waFilter1 = nullptr;
static SignalConverter* converter1 = nullptr;

// Filters for Sensor 2 (optional bonus sensor)
static SaltPepperFilter* spFilter2 = nullptr;
static WeightedAverageFilter* waFilter2 = nullptr;
static SignalConverter* converter2 = nullptr;

// ============================================================================
// Signal Processing Pipeline
// ============================================================================

/**
 * Process signal through complete filtering and conversion pipeline
 * @param rawADC Raw ADC reading (0-1023)
 * @param spFilter Salt & Pepper filter instance
 * @param waFilter Weighted Average filter instance
 * @param converter Signal converter instance
 * @param data Output signal data structure
 */
void processSignal(uint16_t rawADC,
                   SaltPepperFilter* spFilter,
                   WeightedAverageFilter* waFilter,
                   SignalConverter* converter,
                   SignalData& data) {
  // Store raw ADC
  data.rawADC = rawADC;
  
  // Step 1: Convert ADC to voltage (for filtering in voltage domain)
  float voltageRaw = converter->adcToVoltage(rawADC);
  
  // Step 2: Apply Salt & Pepper filter (median filter)
  data.filteredSP = spFilter->filter(voltageRaw);
  
  // Step 3: Apply Weighted Average filter
  data.filteredWA = waFilter->filter(data.filteredSP);
  
  // Step 4: Store final voltage
  data.voltage = data.filteredWA;
  
  // Step 5: Convert voltage to physical parameter
  float physical = converter->voltageToPhysical(data.voltage);
  
  // Step 6: Apply saturation
  data.physical = converter->saturate(physical);
  
  // Update statistics
  data.updateStats(data.physical);
}

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
void taskSignalSampling(void* pvParameters) {
  (void)pvParameters;
  
  // Configure pins
  pinMode(LAB32_SENSOR1_PIN, INPUT);
  pinMode(LAB32_SENSOR2_PIN, INPUT);
  
  // Initialize timing for periodic execution
  TickType_t xLastWakeTime = xTaskGetTickCount();
  const TickType_t xPeriod = pdMS_TO_TICKS(LAB32_SAMPLING_PERIOD_MS);
  
  for (;;) {
    // Read both sensors
    uint16_t rawADC1 = analogRead(LAB32_SENSOR1_PIN);
    uint16_t rawADC2 = analogRead(LAB32_SENSOR2_PIN);
    
    // Process Sensor 1 through complete pipeline
    SignalData localSensor1;
    processSignal(rawADC1, spFilter1, waFilter1, converter1, localSensor1);
    
    // Process Sensor 2 through complete pipeline
    SignalData localSensor2;
    processSignal(rawADC2, spFilter2, waFilter2, converter2, localSensor2);
    
    // Update shared state (protected by mutex)
    if (xSemaphoreTake(g_mutex, pdMS_TO_TICKS(50)) == pdTRUE) {
      g_sensor1 = localSensor1;
      g_sensor2 = localSensor2;
      xSemaphoreGive(g_mutex);
    }
    
    // Wait for next period (precise timing with vTaskDelayUntil)
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
void taskStatusReport(void* pvParameters) {
  (void)pvParameters;
  
  // Apply offset to stagger execution from sampling task
  vTaskDelay(pdMS_TO_TICKS(LAB32_REPORT_TASK_OFFSET_MS));
  
  // Initialize timing for periodic execution
  TickType_t xLastWakeTime = xTaskGetTickCount();
  const TickType_t xPeriod = pdMS_TO_TICKS(LAB32_REPORT_PERIOD_MS);
  
  uint32_t reportCount = 0;
  
  for (;;) {
    SignalData sensor1, sensor2;
    
    // Read shared state (protected by mutex)
    if (xSemaphoreTake(g_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
      sensor1 = g_sensor1;
      sensor2 = g_sensor2;
      xSemaphoreGive(g_mutex);
    } else {
      printf("[LAB3.2] ERROR: Mutex timeout!\n");
      vTaskDelayUntil(&xLastWakeTime, xPeriod);
      continue;
    }
    
    reportCount++;
    
    // Header
    printf("\n");
    printf("========================================\n");
    printf("  Lab 3.2: Signal Conditioning Report\n");
    printf("========================================\n");
    printf("Time: %lu ms | Report #%lu\n", millis(), reportCount);
    printf("\n");
    
    // Sensor 1 - Detailed Pipeline
    printf("--- SENSOR 1 (Pin A0) ---\n");
    printf("Signal Processing Pipeline:\n");
    
    // Convert to integers for AVR-compatible printf
    uint16_t raw1 = sensor1.rawADC;
    uint16_t sp1 = (uint16_t)(sensor1.filteredSP * 100);
    uint16_t wa1 = (uint16_t)(sensor1.filteredWA * 100);
    uint16_t volt1 = (uint16_t)(sensor1.voltage * 1000);
    int16_t phys1 = (int16_t)(sensor1.physical * 10);
    int16_t min1 = (int16_t)(sensor1.minValue * 10);
    int16_t max1 = (int16_t)(sensor1.maxValue * 10);
    int16_t range1 = (int16_t)((sensor1.maxValue - sensor1.minValue) * 10);
    
    printf("  Raw ADC:       %u (0-1023)\n", raw1);
    printf("  -> S&P Filter: %u.%02u V\n", sp1/100, sp1%100);
    printf("  -> Wtd Avg:    %u.%02u V\n", wa1/100, wa1%100);
    printf("  -> Voltage:    %u.%03u V\n", volt1/1000, volt1%1000);
    
    if (phys1 >= 0) {
      printf("  -> Physical:   %d.%u (saturated)\n", phys1/10, phys1%10);
    } else {
      printf("  -> Physical:   -%d.%u (saturated)\n", (-phys1)/10, (-phys1)%10);
    }
    
    printf("\nStatistics:\n");
    printf("  Samples:   %lu\n", sensor1.sampleCount);
    
    if (min1 >= 0) {
      printf("  Min Value: %d.%u\n", min1/10, min1%10);
    } else {
      printf("  Min Value: -%d.%u\n", (-min1)/10, (-min1)%10);
    }
    
    if (max1 >= 0) {
      printf("  Max Value: %d.%u\n", max1/10, max1%10);
    } else {
      printf("  Max Value: -%d.%u\n", (-max1)/10, (-max1)%10);
    }
    
    if (range1 >= 0) {
      printf("  Range:     %d.%u\n", range1/10, range1%10);
    } else {
      printf("  Range:     -%d.%u\n", (-range1)/10, (-range1)%10);
    }
    
    // Sensor 2 - Summary
    printf("\n--- SENSOR 2 (Pin A1) ---\n");
    int16_t phys2 = (int16_t)(sensor2.physical * 10);
    if (phys2 >= 0) {
      printf("Physical Value: %d.%u\n", phys2/10, phys2%10);
    } else {
      printf("Physical Value: -%d.%u\n", (-phys2)/10, (-phys2)%10);
    }
    printf("Samples:        %lu\n", sensor2.sampleCount);
    
    // Filter Configuration
    printf("\n[Filter Configuration]\n");
    printf("  Salt & Pepper Window:  %u samples (median)\n", LAB32_SALT_PEPPER_WINDOW);
    printf("  Weighted Avg Window:   %u samples (weighted)\n", LAB32_WEIGHTED_AVG_WINDOW);
    printf("  Sampling Period:       %u ms\n", LAB32_SAMPLING_PERIOD_MS);
    printf("  Report Period:         %u ms\n", LAB32_REPORT_PERIOD_MS);
    printf("========================================\n");
    
    // LCD output (if available) - simple display
    if (g_lcdAvailable) {
      lcdSetCursor(0, 0);
      lcdPrint("S1:");
      if (phys1 >= 0) {
        lcdPrintInt(phys1 / 10);
        lcdPrint(".");
        lcdPrintInt(phys1 % 10);
      } else {
        lcdPrint("-");
        lcdPrintInt((-phys1) / 10);
        lcdPrint(".");
        lcdPrintInt((-phys1) % 10);
      }
      
      lcdSetCursor(8, 0);
      lcdPrint("S2:");
      if (phys2 >= 0) {
        lcdPrintInt(phys2 / 10);
        lcdPrint(".");
        lcdPrintInt(phys2 % 10);
      } else {
        lcdPrint("-");
        lcdPrintInt((-phys2) / 10);
        lcdPrint(".");
        lcdPrintInt((-phys2) % 10);
      }
      
      lcdSetCursor(0, 1);
      lcdPrint("N:");
      lcdPrintInt(sensor1.sampleCount);
      lcdPrint("     ");
    }
    
    // Wait for next period (precise timing with vTaskDelayUntil)
    vTaskDelayUntil(&xLastWakeTime, xPeriod);
  }
}

// ============================================================================
// Public Interface
// ============================================================================

void setup_lab3_2() {
  printf("\n");
  printf("╔════════════════════════════════════════╗\n");
  printf("║  LAB 3.2: Signal Conditioning          ║\n");
  printf("╚════════════════════════════════════════╝\n");
  printf("\n");
  printf("Configuration:\n");
  printf("  Sensor 1:    Pin A0\n");
  printf("  Sensor 2:    Pin A1 (bonus)\n");
  printf("  Filters:     Salt&Pepper (N=%u), Weighted Avg (N=%u)\n", 
         LAB32_SALT_PEPPER_WINDOW, LAB32_WEIGHTED_AVG_WINDOW);
  printf("  Timing:      Sample=%ums, Report=%ums\n",
         LAB32_SAMPLING_PERIOD_MS, LAB32_REPORT_PERIOD_MS);
  printf("  Display:     LCD 16x2\n");
  printf("\n");
  
  // Initialize statistics
  g_sensor1.resetStats();
  g_sensor2.resetStats();
  
  // Create filter objects for Sensor 1
  printf("Creating Sensor 1 filters...\n");
  spFilter1 = new SaltPepperFilter(LAB32_SALT_PEPPER_WINDOW);
  waFilter1 = new WeightedAverageFilter(LAB32_WEIGHTED_AVG_WINDOW);
  converter1 = new SignalConverter(
    LAB32_ADC_RESOLUTION,
    LAB32_ADC_REFERENCE_VOLTAGE,
    LAB32_SENSOR1_VOLTAGE_SCALE,
    LAB32_SENSOR1_VOLTAGE_OFFSET,
    LAB32_SENSOR1_MIN_VALUE,
    LAB32_SENSOR1_MAX_VALUE
  );
  
  // Create filter objects for Sensor 2
  printf("Creating Sensor 2 filters...\n");
  spFilter2 = new SaltPepperFilter(LAB32_SALT_PEPPER_WINDOW);
  waFilter2 = new WeightedAverageFilter(LAB32_WEIGHTED_AVG_WINDOW);
  converter2 = new SignalConverter(
    LAB32_ADC_RESOLUTION,
    LAB32_ADC_REFERENCE_VOLTAGE,
    LAB32_SENSOR1_VOLTAGE_SCALE,
    LAB32_SENSOR1_VOLTAGE_OFFSET,
    LAB32_SENSOR1_MIN_VALUE,
    LAB32_SENSOR1_MAX_VALUE
  );
  
  // Configure pins
  pinMode(LAB32_SENSOR1_PIN, INPUT);
  pinMode(LAB32_SENSOR2_PIN, INPUT);
  
  // Initialize LCD
  printf("Initializing LCD...\n");
  lcdInit();
  lcdClear();
  lcdSetCursor(0, 0);
  lcdPrint("LAB 3.2");
  lcdSetCursor(0, 1);
  lcdPrint("Starting...");
  g_lcdAvailable = true;
  delay(1000);
  lcdClear();
  
  // Create mutex for shared data protection
  printf("Creating mutex...\n");
  g_mutex = xSemaphoreCreateMutex();
  if (g_mutex == NULL) {
    printf("ERROR: Failed to create mutex!\n");
    while (1) delay(1000);
  }
  
  // Create FreeRTOS tasks
  printf("Creating tasks...\n");
  
  BaseType_t result1 = xTaskCreate(
    taskSignalSampling,
    "Sampling",
    LAB32_SAMPLING_TASK_STACK / sizeof(StackType_t),
    NULL,
    LAB32_SAMPLING_TASK_PRIORITY,
    NULL
  );
  
  BaseType_t result2 = xTaskCreate(
    taskStatusReport,
    "Report",
    LAB32_REPORT_TASK_STACK / sizeof(StackType_t),
    NULL,
    LAB32_REPORT_TASK_PRIORITY,
    NULL
  );
  
  if (result1 != pdPASS || result2 != pdPASS) {
    printf("ERROR: Failed to create tasks!\n");
    printf("  Sampling task: %s\n", result1 == pdPASS ? "OK" : "FAIL");
    printf("  Report task:   %s\n", result2 == pdPASS ? "OK" : "FAIL");
    while (1) delay(1000);
  }
  
  printf("Tasks created successfully.\n");
  printf("Starting FreeRTOS scheduler...\n\n");
  
  // Start FreeRTOS scheduler (never returns)
  vTaskStartScheduler();
  
  // Should never reach here
  printf("ERROR: Scheduler failed to start!\n");
  while (1) delay(1000);
}

void loop_lab3_2() {
  // Never reached - scheduler takes over
}

