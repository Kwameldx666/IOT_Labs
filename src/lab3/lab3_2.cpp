/**
 * @file lab3_2.cpp
 * @brief Lab 3.2 - Signal Conditioning with Digital Filters (FreeRTOS)
 * 
 * Features:
 * - Analog sensor sampling (potentiometer on A0)
 * - Salt & Pepper filter for impulse noise removal
 * - Weighted moving average filter for smoothing
 * - ADC → Voltage → Physical parameter conversion
 * - Saturation (value clamping)
 * - FreeRTOS tasks with precise timing
 * - STDIO reporting via printf
 * - Parallel LCD display (optional)
 * 
 * Filters:
 * 1. Salt & Pepper: Median filter (window=5) to remove spikes
 * 2. Weighted Average: Recent samples weighted more (window=5)
 * 
 * Conversion chain:
 * ADC (0-1023) → Voltage (0-5V) → Physical (e.g. temperature 0-100°C)
 */

#include "lab3_2.hpp"

#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include <semphr.h>
#include <stdio.h>

#include "config.h"
#include "dd_serial.hpp"

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

// Ultrasonic HC-SR04 (optional second sensor)
#define ULTRASONIC_TRIG_PIN 8
#define ULTRASONIC_ECHO_PIN 9

// ============================================================================
// Configuration
// ============================================================================

#define LAB32_SENSOR_PIN A0             // Analog sensor (potentiometer in Wokwi)
#define LAB32_SALT_PEPPER_WINDOW 5      // Salt & Pepper filter window
#define LAB32_WEIGHTED_AVG_WINDOW 5     // Weighted average window
#define LAB32_SAMPLING_RATE_MS 100      // Sample every 100ms
#define LAB32_REPORT_RATE_MS 500        // Report every 500ms

// Physical parameter mapping (example: 0-5V → 0-100°C)
#define LAB32_PHYS_MIN 0.0f
#define LAB32_PHYS_MAX 100.0f
#define LAB32_VOLTAGE_REF 5.0f

// ============================================================================
// Shared State (protected by mutex)
// ============================================================================

struct Lab32State {
  uint16_t rawADC;              // Latest ADC reading (0-1023)
  float voltage;                // Converted voltage (0-5V)
  float physical;               // Physical parameter (e.g. temp in °C)
  float filteredSP;             // After Salt & Pepper filter
  float filteredWA;             // After Weighted Average filter
  uint32_t sampleCount;         // Total samples taken
  uint16_t distanceCm;          // Ultrasonic distance (optional)
};

static Lab32State g_state = {0};
static SemaphoreHandle_t g_mutex = NULL;
static bool g_lcdAvailable = false;

// ============================================================================
// Digital Filters
// ============================================================================

/**
 * Salt & Pepper Filter (Median Filter)
 * Removes impulse noise by taking median of recent samples
 */
class SaltPepperFilter {
private:
  float buffer[LAB32_SALT_PEPPER_WINDOW];
  uint8_t index;
  bool filled;

public:
  SaltPepperFilter() : index(0), filled(false) {
    for (int i = 0; i < LAB32_SALT_PEPPER_WINDOW; i++) buffer[i] = 0;
  }

  float filter(float newSample) {
    buffer[index] = newSample;
    index = (index + 1) % LAB32_SALT_PEPPER_WINDOW;
    if (index == 0) filled = true;

    // Copy buffer for sorting
    float sorted[LAB32_SALT_PEPPER_WINDOW];
    for (int i = 0; i < LAB32_SALT_PEPPER_WINDOW; i++) sorted[i] = buffer[i];

    // Bubble sort
    for (int i = 0; i < LAB32_SALT_PEPPER_WINDOW - 1; i++) {
      for (int j = i + 1; j < LAB32_SALT_PEPPER_WINDOW; j++) {
        if (sorted[i] > sorted[j]) {
          float temp = sorted[i];
          sorted[i] = sorted[j];
          sorted[j] = temp;
        }
      }
    }

    // Return median
    return sorted[LAB32_SALT_PEPPER_WINDOW / 2];
  }
};

/**
 * Weighted Moving Average Filter
 * Recent samples weighted more heavily
 * Weights: [1, 2, 3, 4, 5] for 5-sample window
 */
class WeightedAverageFilter {
private:
  float buffer[LAB32_WEIGHTED_AVG_WINDOW];
  uint8_t index;

public:
  WeightedAverageFilter() : index(0) {
    for (int i = 0; i < LAB32_WEIGHTED_AVG_WINDOW; i++) buffer[i] = 0;
  }

  float filter(float newSample) {
    buffer[index] = newSample;
    index = (index + 1) % LAB32_WEIGHTED_AVG_WINDOW;

    // Weighted sum: most recent gets highest weight
    float weightedSum = 0;
    uint16_t totalWeight = 0;
    for (int i = 0; i < LAB32_WEIGHTED_AVG_WINDOW; i++) {
      int weight = i + 1; // Weights: 1, 2, 3, 4, 5
      int bufferIdx = (index + i) % LAB32_WEIGHTED_AVG_WINDOW;
      weightedSum += buffer[bufferIdx] * weight;
      totalWeight += weight;
    }

    return weightedSum / totalWeight;
  }
};

static SaltPepperFilter* spFilter = nullptr;
static WeightedAverageFilter* waFilter = nullptr;

// ============================================================================
// Conversion Functions
// ============================================================================

float adcToVoltage(uint16_t adc) {
  return (adc / 1023.0f) * LAB32_VOLTAGE_REF;
}

float voltageToPhysical(float voltage) {
  // Linear mapping: 0V→min, 5V→max
  return LAB32_PHYS_MIN + (voltage / LAB32_VOLTAGE_REF) * (LAB32_PHYS_MAX - LAB32_PHYS_MIN);
}

float saturate(float value, float min, float max) {
  if (value < min) return min;
  if (value > max) return max;
  return value;
}

// Read ultrasonic distance (optional)
uint16_t readUltrasonicDistance() {
  digitalWrite(ULTRASONIC_TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(ULTRASONIC_TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(ULTRASONIC_TRIG_PIN, LOW);
  
  long duration = pulseIn(ULTRASONIC_ECHO_PIN, HIGH, 30000); // 30ms timeout
  if (duration == 0) return 0; // No echo
  
  // Distance = duration / 58 (microseconds to cm)
  return (uint16_t)(duration / 58);
}

// ============================================================================
// FreeRTOS Tasks
// ============================================================================

/**
 * Task 1: Sampling and Signal Conditioning
 * Reads ADC, applies filters, performs conversions
 */
void taskSampling(void* pvParameters) {
  (void)pvParameters;
  
  pinMode(LAB32_SENSOR_PIN, INPUT);
  pinMode(ULTRASONIC_TRIG_PIN, OUTPUT);
  pinMode(ULTRASONIC_ECHO_PIN, INPUT);
  digitalWrite(ULTRASONIC_TRIG_PIN, LOW);
  
  for (;;) {
    // Read raw ADC from potentiometer
    uint16_t raw = analogRead(LAB32_SENSOR_PIN);
    
    // Convert to voltage
    float voltage = adcToVoltage(raw);
    
    // Apply Salt & Pepper filter (median)
    float filteredSP = spFilter->filter(voltage);
    
    // Apply Weighted Average filter
    float filteredWA = waFilter->filter(filteredSP);
    
    // Convert to physical parameter
    float physical = voltageToPhysical(filteredWA);
    
    // Apply saturation
    physical = saturate(physical, LAB32_PHYS_MIN, LAB32_PHYS_MAX);
    
    // Read ultrasonic distance (every 5th sample to save time)
    uint16_t distance = 0;
    static uint8_t ultrasonicCounter = 0;
    if (++ultrasonicCounter >= 5) {
      ultrasonicCounter = 0;
      distance = readUltrasonicDistance();
    }
    
    // Update shared state
    if (xSemaphoreTake(g_mutex, pdMS_TO_TICKS(50)) == pdTRUE) {
      g_state.rawADC = raw;
      g_state.voltage = voltage;
      g_state.filteredSP = filteredSP;
      g_state.filteredWA = filteredWA;
      g_state.physical = physical;
      g_state.sampleCount++;
      if (distance > 0) g_state.distanceCm = distance;
      xSemaphoreGive(g_mutex);
    }
    
    // Simple delay
    vTaskDelay(pdMS_TO_TICKS(LAB32_SAMPLING_RATE_MS));
  }
}

/**
 * Task 2: Periodic Reporting
 * Prints processed data via STDIO and updates LCD
 */
void taskReport(void* pvParameters) {
  (void)pvParameters;
  
  // Offset task start
  vTaskDelay(pdMS_TO_TICKS(50));
  
  for (;;) {
    Lab32State state;
    
    // Read shared state
    if (xSemaphoreTake(g_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
      state = g_state;
      xSemaphoreGive(g_mutex);
    } else {
      printf("[LAB3.2] Mutex timeout!\n");
      vTaskDelay(pdMS_TO_TICKS(LAB32_REPORT_RATE_MS));
      continue;
    }
    
    // Convert floats to integers for AVR-safe printf
    uint16_t voltMv = (uint16_t)(state.voltage * 1000);
    uint16_t spMv = (uint16_t)(state.filteredSP * 1000);
    uint16_t waMv = (uint16_t)(state.filteredWA * 1000);
    uint16_t physInt = (uint16_t)(state.physical * 10); // 1 decimal
    
    // Serial output
    printf("[LAB3.2] #%lu ADC=%u Raw=%umV SP=%umV WA=%umV Phys=%u.%u",
           state.sampleCount,
           state.rawADC,
           voltMv,
           spMv,
           waMv,
           physInt / 10,
           physInt % 10);
    
    if (state.distanceCm > 0) {
      printf(" Dist=%ucm", state.distanceCm);
    }
    printf("\n");
    
    // LCD output (if available)
    if (g_lcdAvailable) {
      lcdSetCursor(0, 0);
      lcdPrint("ADC:");
      lcdPrintInt(state.rawADC);
      lcdPrint(" V:");
      lcdPrintInt(voltMv / 1000);
      lcdPrint(".");
      lcdPrintInt((voltMv % 1000) / 100);
      lcdPrint("V  ");
      
      lcdSetCursor(0, 1);
      lcdPrint("P:");
      lcdPrintInt(physInt / 10);
      lcdPrint(".");
      lcdPrintInt(physInt % 10);
      
      if (state.distanceCm > 0) {
        lcdPrint(" D:");
        lcdPrintInt(state.distanceCm);
        lcdPrint("cm ");
      } else {
        lcdPrint("      ");
      }
    }
    
    // Simple delay instead of DelayUntil
    vTaskDelay(pdMS_TO_TICKS(LAB32_REPORT_RATE_MS));
  }
}

// ============================================================================
// Public Interface
// ============================================================================

void setup_lab3_2() {
  printf("\nLAB 3.2: Signal Conditioning\n");
  printf("Sensors: Pot(A0), Ultrasonic(D8/D9)\n");
  printf("Filters: SP(N=5), WA(N=5)\n");
  printf("Display: LCD1602\n\n");
  
  // Create filter objects
  spFilter = new SaltPepperFilter();
  waFilter = new WeightedAverageFilter();
  
  // Configure pins
  pinMode(LAB32_SENSOR_PIN, INPUT);
  pinMode(ULTRASONIC_TRIG_PIN, OUTPUT);
  pinMode(ULTRASONIC_ECHO_PIN, INPUT);
  
  // Initialize LCD
  lcdInit();
  
  printf("LCD init OK\n");
  
  lcdClear();
  lcdSetCursor(0, 0);
  lcdPrint("LAB 3.2");
  lcdSetCursor(0, 1);
  lcdPrint("Starting...");
  g_lcdAvailable = true;
  
  delay(1500);
  lcdClear();
  
  // Create mutex
  g_mutex = xSemaphoreCreateMutex();
  if (g_mutex == NULL) {
    printf("ERR: Mutex\n");
    while (1) delay(1000);
  }
  
  printf("Creating tasks\n");
  
  // Create tasks with larger stacks
  BaseType_t r1 = xTaskCreate(taskSampling, "Samp", 256, NULL, 2, NULL);
  BaseType_t r2 = xTaskCreate(taskReport, "Rpt", 256, NULL, 1, NULL);
  
  if (r1 != pdPASS || r2 != pdPASS) {
    printf("ERR: Tasks\n");
    while (1) delay(1000);
  }
  
  printf("Starting scheduler\n\n");
  
  // Start scheduler - never returns
  vTaskStartScheduler();
  
  // Should never reach here
  printf("ERR: Scheduler\n");
  while (1) delay(1000);
}

void loop_lab3_2() {
  // Never reached - scheduler takes over
}

