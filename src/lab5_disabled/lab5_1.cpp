/**
 * @file lab5_1.cpp
 * @brief Lab 5.1 - ON-OFF Control with Hysteresis (Unified)
 * 
 * Complete control system implementation using reusable libraries:
 * - OnOffController (control logic with hysteresis)
 * - AnalogSensor (temperature/humidity reading)
 * - RelayDriver (actuator control)
 * - ButtonController (setpoint adjustment)
 * 
 * Features:
 * - ON-OFF control with configurable hysteresis
 * - Setpoint adjustment via Serial or buttons
 * - LCD status display
 * - Serial Plotter for real-time graphing
 * - STDIO interface
 * 
 * Commands:
 * - "setpoint <value>" - Set target value
 * - "hysteresis <value>" - Set hysteresis band
 * - "mode heating/cooling" - Set control mode
 * - "status" - Show system state
 * - "help" - Show commands
 */

#include "lab5_1.hpp"

#include <Arduino.h>
#include <stdio.h>
#include <string.h>

#include "config.h"
#include "dd_serial.hpp"
#include "dd_lcd.hpp"
#include "analog_sensor.hpp"
#include "relay_driver.hpp"
#include "button_controller.hpp"
#include "onoff_controller.hpp"

// ============================================================================
// Hardware Instances (Reusing Libraries!)
// ============================================================================

namespace {
  // Sensor (reused from Lab 3)
  AnalogSensor sensor(LAB51_SENSOR_PIN, LAB51_SENSOR_MIN, LAB51_SENSOR_MAX, 5);
  
  // Relay (reused from Lab 4)
  RelayDriver relay(LAB51_RELAY_PIN, true);
  
  // Buttons for setpoint adjustment
  ButtonController btnUp(LAB51_BUTTON_UP_PIN);
  ButtonController btnDown(LAB51_BUTTON_DOWN_PIN);
  
  // ON-OFF Controller (new library!)
  OnOffController controller(LAB51_SETPOINT_DEFAULT, 
                             LAB51_HYSTERESIS_DEFAULT,
                             ControlMode::HEATING);
  
  // State tracking
  unsigned long lastSensorRead = 0;
  unsigned long lastLCDUpdate = 0;
  unsigned long lastPlotterUpdate = 0;
  float currentValue = 0.0f;
}

// ============================================================================
// LCD Display
// ============================================================================

void updateLCDDisplay() {
  unsigned long now = millis();
  if (now - lastLCDUpdate < LAB51_LCD_UPDATE_PERIOD_MS) {
    return;
  }
  lastLCDUpdate = now;
  
  dd_lcd_clear();
  
  // Line 1: Setpoint and Current value
  char line1[17];
  snprintf(line1, sizeof(line1), "SP:%.1f T:%.1f%c", 
           controller.getSetpoint(),
           currentValue,
           controller.getModeString()[0]);  // H or C
  dd_lcd_write(0, 0, line1);
  
  // Line 2: Relay state and error
  char line2[17];
  float error = controller.getError();
  snprintf(line2, sizeof(line2), "Relay:%s E:%.1f", 
           controller.getStateString(),
           error);
  dd_lcd_write(1, 0, line2);
}

// ============================================================================
// Serial Plotter Output
// ============================================================================

void sendToPlotter() {
  unsigned long now = millis();
  if (now - lastPlotterUpdate < LAB51_PLOTTER_PERIOD_MS) {
    return;
  }
  lastPlotterUpdate = now;
  
  // Serial Plotter format: label:value label:value ...
  // Each value on same line, separated by space or tab
  printf("Current:%.2f ", currentValue);
  printf("Setpoint:%.2f ", controller.getSetpoint());
  printf("Upper:%.2f ", controller.getUpperThreshold());
  printf("Lower:%.2f ", controller.getLowerThreshold());
  printf("Relay:%d", controller.isOn() ? 1 : 0);
  printf("\n");
}

// ============================================================================
// Control Loop
// ============================================================================

void updateControl() {
  unsigned long now = millis();
  if (now - lastSensorRead < LAB51_SENSOR_READ_PERIOD_MS) {
    return;
  }
  lastSensorRead = now;
  
  // Read sensor
  currentValue = sensor.readFiltered();
  
  // Update controller
  ControlState state = controller.update(currentValue);
  
  // Update relay
  if (state == ControlState::ON) {
    relay.turnOn();
  } else {
    relay.turnOff();
  }
}

// ============================================================================
// Button Handling
// ============================================================================

void handleButtons() {
  unsigned long now = millis();
  
  // UP button: increase setpoint
  if (btnUp.wasPressed(now)) {
    float newSetpoint = controller.getSetpoint() + LAB51_SETPOINT_STEP;
    if (newSetpoint <= LAB51_SETPOINT_MAX) {
      controller.setSetpoint(newSetpoint);
      printf("[INFO] Setpoint increased to %.1f\n", newSetpoint);
    }
  }
  
  // DOWN button: decrease setpoint
  if (btnDown.wasPressed(now)) {
    float newSetpoint = controller.getSetpoint() - LAB51_SETPOINT_STEP;
    if (newSetpoint >= LAB51_SETPOINT_MIN) {
      controller.setSetpoint(newSetpoint);
      printf("[INFO] Setpoint decreased to %.1f\n", newSetpoint);
    }
  }
}

// ============================================================================
// Command Processing
// ============================================================================

void printHelp() {
  printf("\n");
  printf("╔════════════════════════════════════════════════════╗\n");
  printf("║    Lab 5.1: ON-OFF Control Commands               ║\n");
  printf("╚════════════════════════════════════════════════════╝\n");
  printf("\n");
  printf("Setpoint Control:\n");
  printf("  setpoint <value>     - Set target value (%.1f-%.1f)\n",
         LAB51_SETPOINT_MIN, LAB51_SETPOINT_MAX);
  printf("  up                   - Increase setpoint (+%.1f)\n", 
         LAB51_SETPOINT_STEP);
  printf("  down                 - Decrease setpoint (-%.1f)\n",
         LAB51_SETPOINT_STEP);
  printf("\n");
  printf("Controller Parameters:\n");
  printf("  hysteresis <value>   - Set hysteresis band (%.1f-%.1f)\n",
         LAB51_HYSTERESIS_MIN, LAB51_HYSTERESIS_MAX);
  printf("  mode heating         - Set heating mode\n");
  printf("  mode cooling         - Set cooling mode\n");
  printf("\n");
  printf("Information:\n");
  printf("  status               - Show system state\n");
  printf("  help                 - Show this help\n");
  printf("\n");
  printf("Physical Buttons:\n");
  printf("  UP button (Pin %d)   - Increase setpoint\n", LAB51_BUTTON_UP_PIN);
  printf("  DOWN button (Pin %d) - Decrease setpoint\n", LAB51_BUTTON_DOWN_PIN);
  printf("\n");
  printf("════════════════════════════════════════════════════\n");
}

void printStatus() {
  printf("\n");
  printf("╔════════════════════════════════════════════════════╗\n");
  printf("║         ON-OFF Control System Status              ║\n");
  printf("╚════════════════════════════════════════════════════╝\n");
  printf("\n");
  printf("Current Values:\n");
  printf("  Measured Value:  %.2f\n", currentValue);
  printf("  Setpoint:        %.2f\n", controller.getSetpoint());
  printf("  Error (SP-PV):   %.2f\n", controller.getError());
  printf("\n");
  printf("Controller State:\n");
  printf("  Mode:            %s\n", controller.getModeString());
  printf("  Output:          %s\n", controller.getStateString());
  printf("  Relay State:     %s\n", relay.isOn() ? "ON" : "OFF");
  printf("\n");
  printf("Control Parameters:\n");
  printf("  Hysteresis:      ±%.2f\n", controller.getHysteresis());
  printf("  Upper Threshold: %.2f (SP + hyst)\n", controller.getUpperThreshold());
  printf("  Lower Threshold: %.2f (SP - hyst)\n", controller.getLowerThreshold());
  printf("\n");
  printf("Hardware Configuration:\n");
  printf("  Sensor Pin:      A%d\n", LAB51_SENSOR_PIN - A0);
  printf("  Relay Pin:       %d\n", LAB51_RELAY_PIN);
  printf("  UP Button:       Pin %d\n", LAB51_BUTTON_UP_PIN);
  printf("  DOWN Button:     Pin %d\n", LAB51_BUTTON_DOWN_PIN);
  printf("\n");
  printf("Statistics:\n");
  printf("  Sensor Samples:  %lu\n", (unsigned long)sensor.getSampleCount());
  printf("  Min Recorded:    %.2f\n", sensor.getMin());
  printf("  Max Recorded:    %.2f\n", sensor.getMax());
  printf("\n");
  printf("════════════════════════════════════════════════════\n");
}

void processCommand(const char* command) {
  char cmdCopy[LAB51_COMMAND_BUFFER_SIZE];
  strncpy(cmdCopy, command, sizeof(cmdCopy) - 1);
  cmdCopy[sizeof(cmdCopy) - 1] = '\0';
  
  char* token1 = strtok(cmdCopy, " ");
  char* token2 = strtok(nullptr, " ");
  
  if (token1 == nullptr) {
    return;
  }
  
  // ========== Setpoint Command ==========
  if (strcmp(token1, "setpoint") == 0) {
    if (token2 == nullptr) {
      printf("[ERROR] Usage: setpoint <value>\n");
      return;
    }
    
    float value = atof(token2);
    if (value < LAB51_SETPOINT_MIN || value > LAB51_SETPOINT_MAX) {
      printf("[ERROR] Setpoint must be %.1f-%.1f\n", 
             LAB51_SETPOINT_MIN, LAB51_SETPOINT_MAX);
      return;
    }
    
    controller.setSetpoint(value);
    printf("[OK] Setpoint set to %.1f\n", value);
    return;
  }
  
  // ========== UP Command ==========
  if (strcmp(token1, "up") == 0) {
    float newSetpoint = controller.getSetpoint() + LAB51_SETPOINT_STEP;
    if (newSetpoint <= LAB51_SETPOINT_MAX) {
      controller.setSetpoint(newSetpoint);
      printf("[OK] Setpoint increased to %.1f\n", newSetpoint);
    } else {
      printf("[ERROR] Maximum setpoint reached\n");
    }
    return;
  }
  
  // ========== DOWN Command ==========
  if (strcmp(token1, "down") == 0) {
    float newSetpoint = controller.getSetpoint() - LAB51_SETPOINT_STEP;
    if (newSetpoint >= LAB51_SETPOINT_MIN) {
      controller.setSetpoint(newSetpoint);
      printf("[OK] Setpoint decreased to %.1f\n", newSetpoint);
    } else {
      printf("[ERROR] Minimum setpoint reached\n");
    }
    return;
  }
  
  // ========== Hysteresis Command ==========
  if (strcmp(token1, "hysteresis") == 0) {
    if (token2 == nullptr) {
      printf("[ERROR] Usage: hysteresis <value>\n");
      return;
    }
    
    float value = atof(token2);
    if (value < LAB51_HYSTERESIS_MIN || value > LAB51_HYSTERESIS_MAX) {
      printf("[ERROR] Hysteresis must be %.1f-%.1f\n",
             LAB51_HYSTERESIS_MIN, LAB51_HYSTERESIS_MAX);
      return;
    }
    
    controller.setHysteresis(value);
    printf("[OK] Hysteresis set to %.1f\n", value);
    return;
  }
  
  // ========== Mode Command ==========
  if (strcmp(token1, "mode") == 0) {
    if (token2 == nullptr) {
      printf("[ERROR] Usage: mode <heating|cooling>\n");
      return;
    }
    
    if (strcmp(token2, "heating") == 0) {
      controller.setMode(ControlMode::HEATING);
      printf("[OK] Mode set to HEATING\n");
    } else if (strcmp(token2, "cooling") == 0) {
      controller.setMode(ControlMode::COOLING);
      printf("[OK] Mode set to COOLING\n");
    } else {
      printf("[ERROR] Unknown mode: %s\n", token2);
    }
    return;
  }
  
  // ========== Status Command ==========
  if (strcmp(token1, "status") == 0) {
    printStatus();
    return;
  }
  
  // ========== Help Command ==========
  if (strcmp(token1, "help") == 0) {
    printHelp();
    return;
  }
  
  // Unknown command
  printf("[ERROR] Unknown command: %s\n", token1);
  printf("Type 'help' for available commands\n");
}

// ============================================================================
// Public Interface
// ============================================================================

void setup_lab5_1() {
  // Initialize Serial with STDIO
  SerialBegin();
  
  // Initialize LCD
  dd_lcd_init();
  dd_lcd_clear();
  
  // Initialize hardware
  sensor.begin();
  relay.begin();
  btnUp.begin();
  btnDown.begin();
  
  // Print welcome message
  printf("\n");
  printf("════════════════════════════════════════════════════\n");
  printf("   Lab 5.1: ON-OFF Control with Hysteresis\n");
  printf("════════════════════════════════════════════════════\n");
  printf("Control Type:      ON-OFF (Bang-Bang)\n");
  printf("Control Variable:  Temperature/Humidity\n");
  printf("Actuator:          Relay (Pin %d)\n", LAB51_RELAY_PIN);
  printf("Sensor:            Analog (Pin A%d)\n", LAB51_SENSOR_PIN - A0);
  printf("\n");
  printf("Initial Parameters:\n");
  printf("  Setpoint:        %.1f\n", controller.getSetpoint());
  printf("  Hysteresis:      ±%.1f\n", controller.getHysteresis());
  printf("  Mode:            %s\n", controller.getModeString());
  printf("  Upper Limit:     %.1f\n", controller.getUpperThreshold());
  printf("  Lower Limit:     %.1f\n", controller.getLowerThreshold());
  printf("\n");
  printf("Features:\n");
  printf("  • Serial commands for setpoint adjustment\n");
  printf("  • Physical buttons (UP/DOWN)\n");
  printf("  • LCD status display\n");
  printf("  • Serial Plotter for graphing\n");
  printf("  • Hysteresis prevents oscillation\n");
  printf("\n");
  printf("Type 'help' for available commands\n");
  printf("════════════════════════════════════════════════════\n");
  printf("\n");
  
  // Initial LCD display
  dd_lcd_write(0, 0, "Lab 5.1: Ready");
  dd_lcd_write(1, 0, "Type 'help'");
  
  delay(2000);
  
  // Read initial sensor value
  currentValue = sensor.readFiltered();
  updateLCDDisplay();
}

void loop_lab5_1() {
  static char commandBuffer[LAB51_COMMAND_BUFFER_SIZE];
  static bool commandReady = false;
  
  // Non-blocking command reading
  if (Serial.available()) {
    static int bufferIndex = 0;
    char c = Serial.read();
    
    if (c == '\n' || c == '\r') {
      if (bufferIndex > 0) {
        commandBuffer[bufferIndex] = '\0';
        commandReady = true;
      }
    } else if (bufferIndex < LAB51_COMMAND_BUFFER_SIZE - 1) {
      commandBuffer[bufferIndex++] = c;
    }
    
    if (commandReady) {
      processCommand(commandBuffer);
      bufferIndex = 0;
      commandReady = false;
      memset(commandBuffer, 0, sizeof(commandBuffer));
    }
  }
  
  // Control loop
  updateControl();
  
  // Button handling
  handleButtons();
  
  // LCD update
  updateLCDDisplay();
  
  // Serial Plotter output
  sendToPlotter();
}

