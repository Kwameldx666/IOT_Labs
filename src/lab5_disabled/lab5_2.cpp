/**
 * @file lab5_2.cpp
 * @brief Lab 5.2 - Control Comparison: ON-OFF vs PID (Unified)
 * 
 * Complete dual-control system using reusable libraries:
 * - PIDController (new library for PID control)
 * - OnOffController (from Lab 5.1)
 * - RotaryEncoder (new library for RPM measurement)
 * - DCMotorPercentageController (from Lab 4.2)
 * - ButtonController, dd_serial, dd_lcd (existing)
 * 
 * Control Modes:
 * 1. ON-OFF Mode: Bang-bang control with hysteresis
 * 2. PID Mode: Smooth proportional-integral-derivative control
 * 
 * Features:
 * - Mode switching (ON-OFF ↔ PID)
 * - Setpoint adjustment (Serial + buttons)
 * - Real-time RPM measurement with encoder
 * - LCD status display
 * - Serial Plotter for comparison
 * - PID tuning interface
 */

#include "lab5_2.hpp"

#include <Arduino.h>
#include <stdio.h>
#include <string.h>

#include "config.h"
#include "dd_serial.hpp"
#include "dd_lcd.hpp"
#include "dc_motor_percentage_controller.hpp"
#include "rotary_encoder.hpp"
#include "pid_controller.hpp"
#include "onoff_controller.hpp"
#include "button_controller.hpp"

// ============================================================================
// Control Mode
// ============================================================================

enum class ControllerMode {
  ONOFF,  // ON-OFF control
  PID     // PID control
};

// ============================================================================
// Hardware Instances (Reusing Libraries!)
// ============================================================================

namespace {
  // Motor controller (reused from Lab 4.2)
  DCMotorPercentageController motor(LAB52_MOTOR_ENABLE_PIN,
                                     LAB52_MOTOR_DIR_PIN1,
                                     LAB52_MOTOR_DIR_PIN2);
  
  // Encoder for RPM measurement (new!)
  RotaryEncoder encoder(LAB52_ENCODER_PIN_A,
                        LAB52_ENCODER_PIN_B,
                        LAB52_ENCODER_PPR);
  
  // PID Controller (new!)
  PIDController pidController(LAB52_PID_KP_DEFAULT,
                               LAB52_PID_KI_DEFAULT,
                               LAB52_PID_KD_DEFAULT,
                               LAB52_PID_OUTPUT_MIN,
                               LAB52_PID_OUTPUT_MAX);
  
  // ON-OFF Controller (from Lab 5.1)
  OnOffController onoffController(LAB52_SETPOINT_DEFAULT,
                                   LAB52_ONOFF_HYSTERESIS,
                                   ControlMode::HEATING);  // Forward = heating
  
  // Buttons
  ButtonController btnUp(LAB52_BUTTON_UP_PIN);
  ButtonController btnDown(LAB52_BUTTON_DOWN_PIN);
  ButtonController btnMode(LAB52_BUTTON_MODE_PIN);
  
  // State
  ControllerMode currentMode = ControllerMode::PID;
  float setpoint = LAB52_SETPOINT_DEFAULT;
  float currentRPM = 0.0f;
  unsigned long lastControlUpdate = 0;
  unsigned long lastLCDUpdate = 0;
  unsigned long lastPlotterUpdate = 0;
}

// ============================================================================
// Control Loop
// ============================================================================

void updateControl() {
  unsigned long now = millis();
  if (now - lastControlUpdate < LAB52_CONTROL_PERIOD_MS) {
    return;
  }
  
  float dt = (now - lastControlUpdate) / 1000.0f;  // Convert to seconds
  lastControlUpdate = now;
  
  // Update encoder and get RPM
  encoder.update(dt);
  currentRPM = encoder.getRPM();
  
  // Apply control based on selected mode
  float controlOutput = 0.0f;
  
  if (currentMode == ControllerMode::PID) {
    // PID Control
    controlOutput = pidController.compute(setpoint, currentRPM, dt);
    
    // Convert PID output (0-255) to motor power (-100 to +100%)
    int16_t motorPower = static_cast<int16_t>((controlOutput / 255.0f) * 100.0f);
    motor.setPower(motorPower);
    
  } else {  // ON-OFF Mode
    // ON-OFF Control
    ControlState state = onoffController.update(currentRPM);
    
    if (state == ControlState::ON) {
      motor.setPower(100);  // Full forward
    } else {
      motor.setPower(0);    // Stop
    }
  }
}

// ============================================================================
// LCD Display
// ============================================================================

void updateLCDDisplay() {
  unsigned long now = millis();
  if (now - lastLCDUpdate < LAB52_LCD_UPDATE_PERIOD_MS) {
    return;
  }
  lastLCDUpdate = now;
  
  dd_lcd_clear();
  
  // Line 1: Mode, Setpoint, Current RPM
  char line1[17];
  snprintf(line1, sizeof(line1), "%s SP:%.0f R:%.0f",
           (currentMode == ControllerMode::PID) ? "PID" : "O/O",
           setpoint,
           currentRPM);
  dd_lcd_write(0, 0, line1);
  
  // Line 2: Mode-specific info
  char line2[17];
  if (currentMode == ControllerMode::PID) {
    float error = pidController.getError();
    float output = pidController.getOutput();
    snprintf(line2, sizeof(line2), "E:%.0f O:%.0f", error, output);
  } else {
    snprintf(line2, sizeof(line2), "Motor:%s", 
             onoffController.isOn() ? "ON " : "OFF");
  }
  dd_lcd_write(1, 0, line2);
}

// ============================================================================
// Serial Plotter
// ============================================================================

void sendToPlotter() {
  unsigned long now = millis();
  if (now - lastPlotterUpdate < LAB52_PLOTTER_PERIOD_MS) {
    return;
  }
  lastPlotterUpdate = now;
  
  // Format for Arduino Serial Plotter
  printf("Setpoint:%.2f ", setpoint);
  printf("RPM:%.2f ", currentRPM);
  printf("Error:%.2f ", setpoint - currentRPM);
  
  if (currentMode == ControllerMode::PID) {
    printf("PID_P:%.2f ", pidController.getPTerm());
    printf("PID_I:%.2f ", pidController.getITerm());
    printf("PID_D:%.2f ", pidController.getDTerm());
    printf("PID_Out:%.2f", pidController.getOutput());
  } else {
    printf("ONOFF:%d", onoffController.isOn() ? 100 : 0);
  }
  
  printf("\n");
}

// ============================================================================
// Button Handling
// ============================================================================

void handleButtons() {
  unsigned long now = millis();
  
  // UP button: increase setpoint
  if (btnUp.wasPressed(now)) {
    float newSetpoint = setpoint + LAB52_SETPOINT_STEP;
    if (newSetpoint <= LAB52_SETPOINT_MAX) {
      setpoint = newSetpoint;
      onoffController.setSetpoint(setpoint);
      printf("[INFO] Setpoint increased to %.0f RPM\n", setpoint);
    }
  }
  
  // DOWN button: decrease setpoint
  if (btnDown.wasPressed(now)) {
    float newSetpoint = setpoint - LAB52_SETPOINT_STEP;
    if (newSetpoint >= LAB52_SETPOINT_MIN) {
      setpoint = newSetpoint;
      onoffController.setSetpoint(setpoint);
      printf("[INFO] Setpoint decreased to %.0f RPM\n", setpoint);
    }
  }
  
  // MODE button: switch control mode
  if (btnMode.wasPressed(now)) {
    if (currentMode == ControllerMode::PID) {
      currentMode = ControllerMode::ONOFF;
      printf("[INFO] Switched to ON-OFF control\n");
    } else {
      currentMode = ControllerMode::PID;
      pidController.reset();  // Reset PID state
      printf("[INFO] Switched to PID control\n");
    }
  }
}

// ============================================================================
// Command Processing
// ============================================================================

void printHelp() {
  printf("\n");
  printf("╔════════════════════════════════════════════════════╗\n");
  printf("║  Lab 5.2: ON-OFF vs PID Control Commands          ║\n");
  printf("╚════════════════════════════════════════════════════╝\n");
  printf("\n");
  printf("Control Mode:\n");
  printf("  mode onoff           - Switch to ON-OFF control\n");
  printf("  mode pid             - Switch to PID control\n");
  printf("\n");
  printf("Setpoint Control:\n");
  printf("  setpoint <RPM>       - Set target RPM (%.0f-%.0f)\n",
         LAB52_SETPOINT_MIN, LAB52_SETPOINT_MAX);
  printf("  up                   - Increase setpoint (+%.0f)\n",
         LAB52_SETPOINT_STEP);
  printf("  down                 - Decrease setpoint (-%.0f)\n",
         LAB52_SETPOINT_STEP);
  printf("\n");
  printf("PID Tuning:\n");
  printf("  pid <Kp> <Ki> <Kd>   - Set PID gains\n");
  printf("                         Example: pid 2.0 0.5 0.1\n");
  printf("  pid reset            - Reset PID integrator\n");
  printf("\n");
  printf("ON-OFF Parameters:\n");
  printf("  hysteresis <value>   - Set hysteresis (RPM)\n");
  printf("\n");
  printf("Information:\n");
  printf("  status               - Show detailed system state\n");
  printf("  help                 - Show this help\n");
  printf("\n");
  printf("Physical Buttons:\n");
  printf("  UP (Pin %d)          - Increase setpoint\n", LAB52_BUTTON_UP_PIN);
  printf("  DOWN (Pin %d)        - Decrease setpoint\n", LAB52_BUTTON_DOWN_PIN);
  printf("  MODE (Pin %d)        - Switch control mode\n", LAB52_BUTTON_MODE_PIN);
  printf("\n");
  printf("════════════════════════════════════════════════════\n");
}

void printStatus() {
  printf("\n");
  printf("╔════════════════════════════════════════════════════╗\n");
  printf("║         Control System Status Report              ║\n");
  printf("╚════════════════════════════════════════════════════╝\n");
  printf("\n");
  printf("Current Values:\n");
  printf("  Setpoint:        %.2f RPM\n", setpoint);
  printf("  Current RPM:     %.2f RPM\n", currentRPM);
  printf("  Error:           %.2f RPM\n", setpoint - currentRPM);
  printf("\n");
  printf("Control Mode:      %s\n", 
         (currentMode == ControllerMode::PID) ? "PID" : "ON-OFF");
  printf("\n");
  
  if (currentMode == ControllerMode::PID) {
    printf("PID Parameters:\n");
    printf("  Kp (Proportional): %.3f\n", pidController.getKp());
    printf("  Ki (Integral):     %.3f\n", pidController.getKi());
    printf("  Kd (Derivative):   %.3f\n", pidController.getKd());
    printf("\n");
    printf("PID Terms:\n");
    printf("  P Term:            %.2f\n", pidController.getPTerm());
    printf("  I Term:            %.2f\n", pidController.getITerm());
    printf("  D Term:            %.2f\n", pidController.getDTerm());
    printf("  Total Output:      %.2f (0-255)\n", pidController.getOutput());
    printf("\n");
    printf("Motor Power:         %d%%\n", motor.getPower());
  } else {
    printf("ON-OFF Parameters:\n");
    printf("  Hysteresis:        ±%.1f RPM\n", onoffController.getHysteresis());
    printf("  Upper Threshold:   %.1f RPM\n", onoffController.getUpperThreshold());
    printf("  Lower Threshold:   %.1f RPM\n", onoffController.getLowerThreshold());
    printf("\n");
    printf("Controller State:\n");
    printf("  Output:            %s\n", onoffController.getStateString());
    printf("  Motor Power:       %s\n", onoffController.isOn() ? "100%" : "0%");
  }
  
  printf("\n");
  printf("Hardware Configuration:\n");
  printf("  Motor Enable:      Pin %d (PWM)\n", LAB52_MOTOR_ENABLE_PIN);
  printf("  Motor Direction:   Pins %d, %d\n", 
         LAB52_MOTOR_DIR_PIN1, LAB52_MOTOR_DIR_PIN2);
  printf("  Encoder A:         Pin %d (Interrupt)\n", LAB52_ENCODER_PIN_A);
  printf("  Encoder B:         Pin %d (Interrupt)\n", LAB52_ENCODER_PIN_B);
  printf("  Encoder PPR:       %d pulses/rev\n", LAB52_ENCODER_PPR);
  printf("\n");
  printf("Encoder Status:\n");
  printf("  Position:          %ld pulses\n", (long)encoder.getPosition());
  printf("  Current RPM:       %.2f\n", currentRPM);
  printf("\n");
  printf("════════════════════════════════════════════════════\n");
}

void processCommand(const char* command) {
  char cmdCopy[LAB52_COMMAND_BUFFER_SIZE];
  strncpy(cmdCopy, command, sizeof(cmdCopy) - 1);
  cmdCopy[sizeof(cmdCopy) - 1] = '\0';
  
  char* token1 = strtok(cmdCopy, " ");
  char* token2 = strtok(nullptr, " ");
  char* token3 = strtok(nullptr, " ");
  char* token4 = strtok(nullptr, " ");
  
  if (token1 == nullptr) {
    return;
  }
  
  // ========== Mode Command ==========
  if (strcmp(token1, "mode") == 0) {
    if (token2 == nullptr) {
      printf("[ERROR] Usage: mode <onoff|pid>\n");
      return;
    }
    
    if (strcmp(token2, "onoff") == 0) {
      currentMode = ControllerMode::ONOFF;
      printf("[OK] Switched to ON-OFF control\n");
    } else if (strcmp(token2, "pid") == 0) {
      currentMode = ControllerMode::PID;
      pidController.reset();
      printf("[OK] Switched to PID control\n");
    } else {
      printf("[ERROR] Unknown mode: %s\n", token2);
    }
    return;
  }
  
  // ========== Setpoint Command ==========
  if (strcmp(token1, "setpoint") == 0) {
    if (token2 == nullptr) {
      printf("[ERROR] Usage: setpoint <RPM>\n");
      return;
    }
    
    float value = atof(token2);
    if (value < LAB52_SETPOINT_MIN || value > LAB52_SETPOINT_MAX) {
      printf("[ERROR] Setpoint must be %.0f-%.0f RPM\n",
             LAB52_SETPOINT_MIN, LAB52_SETPOINT_MAX);
      return;
    }
    
    setpoint = value;
    onoffController.setSetpoint(setpoint);
    printf("[OK] Setpoint set to %.0f RPM\n", setpoint);
    return;
  }
  
  // ========== UP/DOWN Commands ==========
  if (strcmp(token1, "up") == 0) {
    float newSetpoint = setpoint + LAB52_SETPOINT_STEP;
    if (newSetpoint <= LAB52_SETPOINT_MAX) {
      setpoint = newSetpoint;
      onoffController.setSetpoint(setpoint);
      printf("[OK] Setpoint increased to %.0f RPM\n", setpoint);
    } else {
      printf("[ERROR] Maximum setpoint reached\n");
    }
    return;
  }
  
  if (strcmp(token1, "down") == 0) {
    float newSetpoint = setpoint - LAB52_SETPOINT_STEP;
    if (newSetpoint >= LAB52_SETPOINT_MIN) {
      setpoint = newSetpoint;
      onoffController.setSetpoint(setpoint);
      printf("[OK] Setpoint decreased to %.0f RPM\n", setpoint);
    } else {
      printf("[ERROR] Minimum setpoint reached\n");
    }
    return;
  }
  
  // ========== PID Command ==========
  if (strcmp(token1, "pid") == 0) {
    if (token2 == nullptr) {
      printf("[ERROR] Usage: pid <Kp> <Ki> <Kd> OR pid reset\n");
      return;
    }
    
    if (strcmp(token2, "reset") == 0) {
      pidController.reset();
      printf("[OK] PID integrator reset\n");
      return;
    }
    
    if (token3 == nullptr || token4 == nullptr) {
      printf("[ERROR] Usage: pid <Kp> <Ki> <Kd>\n");
      return;
    }
    
    float kp = atof(token2);
    float ki = atof(token3);
    float kd = atof(token4);
    
    pidController.setGains(kp, ki, kd);
    printf("[OK] PID gains set: Kp=%.3f Ki=%.3f Kd=%.3f\n", kp, ki, kd);
    return;
  }
  
  // ========== Hysteresis Command ==========
  if (strcmp(token1, "hysteresis") == 0) {
    if (token2 == nullptr) {
      printf("[ERROR] Usage: hysteresis <value>\n");
      return;
    }
    
    float value = atof(token2);
    onoffController.setHysteresis(value);
    printf("[OK] Hysteresis set to %.1f RPM\n", value);
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

void setup_lab5_2() {
  // Initialize Serial with STDIO
  SerialBegin();
  
  // Initialize LCD
  dd_lcd_init();
  dd_lcd_clear();
  
  // Initialize hardware
  motor.begin();
  encoder.begin();
  btnUp.begin();
  btnDown.begin();
  btnMode.begin();
  
  // Print welcome message
  printf("\n");
  printf("════════════════════════════════════════════════════\n");
  printf("   Lab 5.2: Control System Comparison\n");
  printf("   ON-OFF vs PID\n");
  printf("════════════════════════════════════════════════════\n");
  printf("Control Objective:  Motor Speed (RPM)\n");
  printf("Sensor:             Rotary Encoder\n");
  printf("Actuator:           DC Motor with L298N\n");
  printf("\n");
  printf("Available Control Modes:\n");
  printf("  1. ON-OFF:        Bang-bang with hysteresis\n");
  printf("  2. PID:           Proportional-Integral-Derivative\n");
  printf("\n");
  printf("Current Mode:       %s\n",
         (currentMode == ControllerMode::PID) ? "PID" : "ON-OFF");
  printf("Initial Setpoint:   %.0f RPM\n", setpoint);
  printf("\n");
  
  if (currentMode == ControllerMode::PID) {
    printf("PID Parameters:\n");
    printf("  Kp: %.3f  Ki: %.3f  Kd: %.3f\n",
           LAB52_PID_KP_DEFAULT, LAB52_PID_KI_DEFAULT, LAB52_PID_KD_DEFAULT);
  } else {
    printf("ON-OFF Parameters:\n");
    printf("  Hysteresis: ±%.0f RPM\n", LAB52_ONOFF_HYSTERESIS);
  }
  
  printf("\n");
  printf("Features:\n");
  printf("  • Mode switching (ON-OFF ↔ PID)\n");
  printf("  • Real-time RPM measurement\n");
  printf("  • Serial commands + Physical buttons\n");
  printf("  • LCD status display\n");
  printf("  • Serial Plotter for comparison\n");
  printf("  • PID tuning interface\n");
  printf("\n");
  printf("Type 'help' for available commands\n");
  printf("════════════════════════════════════════════════════\n");
  printf("\n");
  
  // Initial LCD display
  dd_lcd_write(0, 0, "Lab 5.2: Ready");
  dd_lcd_write(1, 0, "Press MODE btn");
  
  delay(2000);
  updateLCDDisplay();
}

void loop_lab5_2() {
  static char commandBuffer[LAB52_COMMAND_BUFFER_SIZE];
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
    } else if (bufferIndex < LAB52_COMMAND_BUFFER_SIZE - 1) {
      commandBuffer[bufferIndex++] = c;
    }
    
    if (commandReady) {
      processCommand(commandBuffer);
      bufferIndex = 0;
      commandReady = false;
      memset(commandBuffer, 0, sizeof(commandBuffer));
    }
  }
  
  // Control loop (20 Hz)
  updateControl();
  
  // Button handling
  handleButtons();
  
  // LCD update
  updateLCDDisplay();
  
  // Serial Plotter output
  sendToPlotter();
}

