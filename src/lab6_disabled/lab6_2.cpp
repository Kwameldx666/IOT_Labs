/**
 * @file lab6_2.cpp
 * @brief Lab 6.2 - Finite State Machine: Traffic Light (Unified)
 * 
 * Complete traffic light FSM implementation using:
 * - FSM library (generic state machine from Lab 6.1)
 * - Timer-based automatic transitions
 * - Manual control via button or Serial
 * 
 * State Diagram:
 * 
 *                    [TIMEOUT]
 *      ┌─────────────────────────────────┐
 *      │                                 ↓
 *   ┌───────┐  TIMEOUT   ┌────────┐  TIMEOUT   ┌─────┐
 *   │ GREEN │ ─────────→ │ YELLOW │ ─────────→ │ RED │
 *   └───────┘            └────────┘            └─────┘
 *      ↑                                          │
 *      │                 [TIMEOUT]                │
 *      └──────────────────────────────────────────┘
 * 
 * Transition Table:
 * ┌───────────┬───────────────┬──────────────┬──────────────┐
 * │ From      │ Event         │ To           │ Duration     │
 * ├───────────┼───────────────┼──────────────┼──────────────┤
 * │ GREEN     │ TIMEOUT       │ YELLOW       │ 5000 ms      │
 * │ YELLOW    │ TIMEOUT       │ RED          │ 2000 ms      │
 * │ RED       │ TIMEOUT       │ GREEN        │ 5000 ms      │
 * │ ANY       │ BUTTON_PRESS  │ NEXT         │ Immediate    │
 * └───────────┴───────────────┴──────────────┴──────────────┘
 */

#include "lab6_2.hpp"

#include <Arduino.h>
#include <stdio.h>
#include <string.h>

#include "config.h"
#include "dd_serial.hpp"
#include "dd_lcd.hpp"
#include "button_controller.hpp"
#include "fsm.hpp"

// ============================================================================
// State and Event Definitions
// ============================================================================

// State IDs
enum TrafficLightStates : StateId {
  STATE_GREEN = 0,
  STATE_YELLOW,
  STATE_RED,
  NUM_TRAFFIC_STATES
};

// Event IDs
enum TrafficLightEvents : EventId {
  EVENT_TIMEOUT = 0,
  EVENT_BUTTON_PRESS,
  EVENT_MANUAL_NEXT
};

// ============================================================================
// Hardware & Timing
// ============================================================================

namespace {
  ButtonController button(LAB62_BUTTON_PIN);
  
  unsigned long stateEntryTime = 0;
  unsigned long lastReportTime = 0;
  uint32_t cycleCount = 0;
  bool autoMode = true;  // Auto mode vs manual mode
}

// ============================================================================
// LED Control Functions
// ============================================================================

void setTrafficLight(bool red, bool yellow, bool green) {
  digitalWrite(LAB62_LED_RED_PIN, red ? HIGH : LOW);
  digitalWrite(LAB62_LED_YELLOW_PIN, yellow ? HIGH : LOW);
  digitalWrite(LAB62_LED_GREEN_PIN, green ? HIGH : LOW);
}

void allLightsOff() {
  setTrafficLight(false, false, false);
}

// ============================================================================
// State Durations
// ============================================================================

unsigned long getStateDuration(StateId state) {
  switch (state) {
    case STATE_GREEN:  return LAB62_GREEN_DURATION_MS;
    case STATE_YELLOW: return LAB62_YELLOW_DURATION_MS;
    case STATE_RED:    return LAB62_RED_DURATION_MS;
    default:           return 0;
  }
}

// ============================================================================
// State Entry/Exit Actions
// ============================================================================

void onEnterGreen() {
  setTrafficLight(false, false, true);
  stateEntryTime = millis();
  printf("[TRAFFIC LIGHT] → GREEN: GO\n");
  printf("[TRAFFIC LIGHT]   Duration: %lu ms\n", LAB62_GREEN_DURATION_MS);
}

void onExitGreen() {
  printf("[TRAFFIC LIGHT] ← Exiting GREEN\n");
}

void onEnterYellow() {
  setTrafficLight(false, true, false);
  stateEntryTime = millis();
  printf("[TRAFFIC LIGHT] → YELLOW: PREPARE TO STOP\n");
  printf("[TRAFFIC LIGHT]   Duration: %lu ms\n", LAB62_YELLOW_DURATION_MS);
}

void onExitYellow() {
  printf("[TRAFFIC LIGHT] ← Exiting YELLOW\n");
}

void onEnterRed() {
  setTrafficLight(true, false, false);
  stateEntryTime = millis();
  cycleCount++;
  printf("[TRAFFIC LIGHT] → RED: STOP\n");
  printf("[TRAFFIC LIGHT]   Duration: %lu ms\n", LAB62_RED_DURATION_MS);
  printf("[TRAFFIC LIGHT]   Cycle #%lu completed\n", (unsigned long)cycleCount);
}

void onExitRed() {
  printf("[TRAFFIC LIGHT] ← Exiting RED\n");
}

// ============================================================================
// State Handlers
// ============================================================================

StateId handleGreen(EventId event) {
  if (event == EVENT_TIMEOUT) {
    return STATE_YELLOW;
  } else if (event == EVENT_BUTTON_PRESS || event == EVENT_MANUAL_NEXT) {
    printf("[EVENT] Manual transition from GREEN\n");
    return STATE_YELLOW;
  }
  return STATE_GREEN;
}

StateId handleYellow(EventId event) {
  if (event == EVENT_TIMEOUT) {
    return STATE_RED;
  } else if (event == EVENT_BUTTON_PRESS || event == EVENT_MANUAL_NEXT) {
    printf("[EVENT] Manual transition from YELLOW\n");
    return STATE_RED;
  }
  return STATE_YELLOW;
}

StateId handleRed(EventId event) {
  if (event == EVENT_TIMEOUT) {
    return STATE_GREEN;
  } else if (event == EVENT_BUTTON_PRESS || event == EVENT_MANUAL_NEXT) {
    printf("[EVENT] Manual transition from RED\n");
    return STATE_GREEN;
  }
  return STATE_RED;
}

// ============================================================================
// FSM Definition
// ============================================================================

// State definitions
const State trafficStates[] = {
  {STATE_GREEN,  "GREEN",  handleGreen,  onEnterGreen,  onExitGreen},
  {STATE_YELLOW, "YELLOW", handleYellow, onEnterYellow, onExitYellow},
  {STATE_RED,    "RED",    handleRed,    onEnterRed,    onExitRed}
};

// Create FSM instance (no transition table, using handlers)
FiniteStateMachine trafficFSM(trafficStates, NUM_TRAFFIC_STATES, 
                              nullptr, 0,  // No table, use handlers
                              STATE_GREEN);

// ============================================================================
// LCD Display
// ============================================================================

void updateLCD() {
  dd_lcd_clear();
  
  char line1[17];
  snprintf(line1, sizeof(line1), "Light: %s", trafficFSM.getCurrentStateName());
  dd_lcd_write(0, 0, line1);
  
  char line2[17];
  unsigned long timeInState = millis() - stateEntryTime;
  unsigned long duration = getStateDuration(trafficFSM.getCurrentState());
  unsigned long remaining = (duration > timeInState) ? (duration - timeInState) : 0;
  
  if (autoMode) {
    snprintf(line2, sizeof(line2), "Auto %lu/%lus", 
             timeInState/1000, duration/1000);
  } else {
    snprintf(line2, sizeof(line2), "Manual C:%lu", (unsigned long)cycleCount);
  }
  dd_lcd_write(1, 0, line2);
}

// ============================================================================
// Timer Check for Auto Mode
// ============================================================================

void checkTimeout() {
  if (!autoMode) {
    return;  // Manual mode, no auto transitions
  }
  
  unsigned long timeInState = millis() - stateEntryTime;
  unsigned long duration = getStateDuration(trafficFSM.getCurrentState());
  
  if (timeInState >= duration) {
    printf("[AUTO] Timeout reached, transitioning...\n");
    trafficFSM.processEvent(EVENT_TIMEOUT);
  }
}

// ============================================================================
// Periodic Status Report
// ============================================================================

void printStatusReport() {
  unsigned long now = millis();
  if (now - lastReportTime < LAB62_REPORT_PERIOD_MS) {
    return;
  }
  lastReportTime = now;
  
  unsigned long timeInState = millis() - stateEntryTime;
  unsigned long duration = getStateDuration(trafficFSM.getCurrentState());
  unsigned long remaining = (duration > timeInState) ? (duration - timeInState) / 1000 : 0;
  
  printf("\n");
  printf("╔════════════════════════════════════════════════════╗\n");
  printf("║     Traffic Light Status Report                   ║\n");
  printf("╚════════════════════════════════════════════════════╝\n");
  printf("\n");
  printf("Current State:       %s\n", trafficFSM.getCurrentStateName());
  printf("Mode:                %s\n", autoMode ? "AUTOMATIC" : "MANUAL");
  printf("Time in State:       %lu seconds\n", timeInState / 1000);
  
  if (autoMode) {
    printf("Remaining Time:      %lu seconds\n", remaining);
  }
  
  printf("Total Cycles:        %lu\n", (unsigned long)cycleCount);
  printf("Total Transitions:   %lu\n", (unsigned long)trafficFSM.getTransitionCount());
  printf("\n");
  printf("Light Status:\n");
  printf("  🔴 Red:    %s\n", digitalRead(LAB62_LED_RED_PIN) ? "ON " : "OFF");
  printf("  🟡 Yellow: %s\n", digitalRead(LAB62_LED_YELLOW_PIN) ? "ON " : "OFF");
  printf("  🟢 Green:  %s\n", digitalRead(LAB62_LED_GREEN_PIN) ? "ON " : "OFF");
  printf("\n");
  printf("════════════════════════════════════════════════════\n");
}

// ============================================================================
// Command Processing
// ============================================================================

void printHelp() {
  printf("\n");
  printf("╔════════════════════════════════════════════════════╗\n");
  printf("║     Lab 6.2: Traffic Light FSM Commands           ║\n");
  printf("╚════════════════════════════════════════════════════╝\n");
  printf("\n");
  printf("Physical Interaction:\n");
  printf("  Button (Pin %d)      - Manual transition to next state\n", 
         LAB62_BUTTON_PIN);
  printf("\n");
  printf("Mode Control:\n");
  printf("  auto                 - Enable automatic mode\n");
  printf("  manual               - Enable manual mode\n");
  printf("\n");
  printf("Manual Control:\n");
  printf("  next                 - Transition to next state\n");
  printf("  green                - Force GREEN state\n");
  printf("  yellow               - Force YELLOW state\n");
  printf("  red                  - Force RED state\n");
  printf("\n");
  printf("Information:\n");
  printf("  diagram              - Show state diagram\n");
  printf("  table                - Show transition table\n");
  printf("  timing               - Show state timing\n");
  printf("  status               - Show current status\n");
  printf("  reset                - Reset to GREEN state\n");
  printf("  help                 - Show this help\n");
  printf("\n");
  printf("State Cycle:\n");
  printf("  GREEN → YELLOW → RED → GREEN (repeat)\n");
  printf("\n");
  printf("════════════════════════════════════════════════════\n");
}

void printStateDiagram() {
  printf("\n");
  printf("════════════════════════════════════════════════════\n");
  printf("         STATE DIAGRAM - Traffic Light FSM\n");
  printf("════════════════════════════════════════════════════\n");
  printf("\n");
  printf("                  [TIMEOUT]\n");
  printf("    ┌─────────────────────────────────┐\n");
  printf("    │                                 ↓\n");
  printf(" ┌───────┐  TIMEOUT   ┌────────┐  TIMEOUT   ┌─────┐\n");
  printf(" │ GREEN │ ─────────→ │ YELLOW │ ─────────→ │ RED │\n");
  printf(" │  🟢   │   5000ms   │   🟡   │   2000ms   │ 🔴  │\n");
  printf(" └───────┘            └────────┘            └─────┘\n");
  printf("    ↑                                          │\n");
  printf("    │              [TIMEOUT] 5000ms            │\n");
  printf("    └──────────────────────────────────────────┘\n");
  printf("\n");
  printf("States:\n");
  printf("  🟢 GREEN:  GO - Cars can pass (%lu ms)\n", 
         LAB62_GREEN_DURATION_MS);
  printf("  🟡 YELLOW: PREPARE TO STOP (%lu ms)\n", 
         LAB62_YELLOW_DURATION_MS);
  printf("  🔴 RED:    STOP - Wait (%lu ms)\n", 
         LAB62_RED_DURATION_MS);
  printf("\n");
  printf("Events:\n");
  printf("  TIMEOUT:       Automatic transition after duration\n");
  printf("  BUTTON_PRESS:  Manual immediate transition\n");
  printf("\n");
  printf("════════════════════════════════════════════════════\n");
}

void printTransitionTable() {
  printf("\n");
  printf("════════════════════════════════════════════════════\n");
  printf("       TRANSITION TABLE - Traffic Light FSM\n");
  printf("════════════════════════════════════════════════════\n");
  printf("\n");
  printf("┌─────────┬──────────────┬─────────┬──────────────┐\n");
  printf("│ From    │ Event        │ To      │ Duration     │\n");
  printf("├─────────┼──────────────┼─────────┼──────────────┤\n");
  printf("│ GREEN   │ TIMEOUT      │ YELLOW  │ %lu ms      │\n", 
         LAB62_GREEN_DURATION_MS);
  printf("│ YELLOW  │ TIMEOUT      │ RED     │ %lu ms      │\n", 
         LAB62_YELLOW_DURATION_MS);
  printf("│ RED     │ TIMEOUT      │ GREEN   │ %lu ms      │\n", 
         LAB62_RED_DURATION_MS);
  printf("│ ANY     │ BUTTON_PRESS │ NEXT    │ Immediate    │\n");
  printf("└─────────┴──────────────┴─────────┴──────────────┘\n");
  printf("\n");
  printf("State Actions:\n");
  printf("┌─────────┬────────────────────────────────────────┐\n");
  printf("│ State   │ On Entry Action                        │\n");
  printf("├─────────┼────────────────────────────────────────┤\n");
  printf("│ GREEN   │ Turn on Green LED only                 │\n");
  printf("│ YELLOW  │ Turn on Yellow LED only                │\n");
  printf("│ RED     │ Turn on Red LED only, increment cycle  │\n");
  printf("└─────────┴────────────────────────────────────────┘\n");
  printf("\n");
  printf("════════════════════════════════════════════════════\n");
}

void printTiming() {
  printf("\n");
  printf("════════════════════════════════════════════════════\n");
  printf("           STATE TIMING CONFIGURATION\n");
  printf("════════════════════════════════════════════════════\n");
  printf("\n");
  printf("State Durations:\n");
  printf("  GREEN:   %lu ms (%lu seconds)\n", 
         LAB62_GREEN_DURATION_MS, LAB62_GREEN_DURATION_MS/1000);
  printf("  YELLOW:  %lu ms (%lu seconds)\n", 
         LAB62_YELLOW_DURATION_MS, LAB62_YELLOW_DURATION_MS/1000);
  printf("  RED:     %lu ms (%lu seconds)\n", 
         LAB62_RED_DURATION_MS, LAB62_RED_DURATION_MS/1000);
  printf("\n");
  printf("Total Cycle Time: %lu ms (%lu seconds)\n",
         LAB62_GREEN_DURATION_MS + LAB62_YELLOW_DURATION_MS + LAB62_RED_DURATION_MS,
         (LAB62_GREEN_DURATION_MS + LAB62_YELLOW_DURATION_MS + LAB62_RED_DURATION_MS)/1000);
  printf("\n");
  printf("Timing can be modified in config.h:\n");
  printf("  LAB62_GREEN_DURATION_MS\n");
  printf("  LAB62_YELLOW_DURATION_MS\n");
  printf("  LAB62_RED_DURATION_MS\n");
  printf("\n");
  printf("════════════════════════════════════════════════════\n");
}

void processCommand(const char* command) {
  char cmdCopy[LAB62_COMMAND_BUFFER_SIZE];
  strncpy(cmdCopy, command, sizeof(cmdCopy) - 1);
  cmdCopy[sizeof(cmdCopy) - 1] = '\0';
  
  char* token1 = strtok(cmdCopy, " ");
  
  if (token1 == nullptr) {
    return;
  }
  
  // Mode control
  if (strcmp(token1, "auto") == 0) {
    autoMode = true;
    stateEntryTime = millis();  // Reset timer
    printf("[MODE] Switched to AUTOMATIC mode\n");
    updateLCD();
    return;
  }
  
  if (strcmp(token1, "manual") == 0) {
    autoMode = false;
    printf("[MODE] Switched to MANUAL mode\n");
    updateLCD();
    return;
  }
  
  // Manual transition
  if (strcmp(token1, "next") == 0) {
    printf("[CMD] Manual transition to next state\n");
    trafficFSM.processEvent(EVENT_MANUAL_NEXT);
    updateLCD();
    return;
  }
  
  // Force states
  if (strcmp(token1, "green") == 0) {
    printf("[CMD] Forcing GREEN state\n");
    trafficFSM.forceState(STATE_GREEN);
    updateLCD();
    return;
  }
  
  if (strcmp(token1, "yellow") == 0) {
    printf("[CMD] Forcing YELLOW state\n");
    trafficFSM.forceState(STATE_YELLOW);
    updateLCD();
    return;
  }
  
  if (strcmp(token1, "red") == 0) {
    printf("[CMD] Forcing RED state\n");
    trafficFSM.forceState(STATE_RED);
    updateLCD();
    return;
  }
  
  // Information commands
  if (strcmp(token1, "diagram") == 0) {
    printStateDiagram();
    return;
  }
  
  if (strcmp(token1, "table") == 0) {
    printTransitionTable();
    return;
  }
  
  if (strcmp(token1, "timing") == 0) {
    printTiming();
    return;
  }
  
  if (strcmp(token1, "status") == 0) {
    printStatusReport();
    return;
  }
  
  if (strcmp(token1, "reset") == 0) {
    printf("[CMD] Resetting to GREEN state\n");
    trafficFSM.forceState(STATE_GREEN);
    cycleCount = 0;
    updateLCD();
    return;
  }
  
  if (strcmp(token1, "help") == 0) {
    printHelp();
    return;
  }
  
  printf("[ERROR] Unknown command: %s\n", token1);
  printf("Type 'help' for available commands\n");
}

// ============================================================================
// Public Interface
// ============================================================================

void setup_lab6_2() {
  // Initialize Serial with STDIO
  SerialBegin();
  
  // Initialize LCD
  dd_lcd_init();
  dd_lcd_clear();
  
  // Initialize LED pins
  pinMode(LAB62_LED_RED_PIN, OUTPUT);
  pinMode(LAB62_LED_YELLOW_PIN, OUTPUT);
  pinMode(LAB62_LED_GREEN_PIN, OUTPUT);
  allLightsOff();
  
  // Initialize button
  button.begin();
  
  // Initialize FSM
  trafficFSM.begin();
  
  // Print welcome
  printf("\n");
  printf("════════════════════════════════════════════════════\n");
  printf("   Lab 6.2: Finite State Machine - Traffic Light\n");
  printf("════════════════════════════════════════════════════\n");
  printf("Automaton Type:     Traffic Light Controller\n");
  printf("Number of States:   %d (GREEN, YELLOW, RED)\n", NUM_TRAFFIC_STATES);
  printf("Mode:               %s\n", autoMode ? "AUTOMATIC" : "MANUAL");
  printf("Initial State:      GREEN\n");
  printf("\n");
  printf("Hardware:\n");
  printf("  Button:           Pin %d\n", LAB62_BUTTON_PIN);
  printf("  🟢 Green LED:     Pin %d\n", LAB62_LED_GREEN_PIN);
  printf("  🟡 Yellow LED:    Pin %d\n", LAB62_LED_YELLOW_PIN);
  printf("  🔴 Red LED:       Pin %d\n", LAB62_LED_RED_PIN);
  printf("\n");
  printf("Timing:\n");
  printf("  GREEN:            %lu seconds\n", LAB62_GREEN_DURATION_MS/1000);
  printf("  YELLOW:           %lu seconds\n", LAB62_YELLOW_DURATION_MS/1000);
  printf("  RED:              %lu seconds\n", LAB62_RED_DURATION_MS/1000);
  printf("  Cycle Duration:   %lu seconds\n",
         (LAB62_GREEN_DURATION_MS + LAB62_YELLOW_DURATION_MS + LAB62_RED_DURATION_MS)/1000);
  printf("\n");
  printf("Features:\n");
  printf("  • Reusable FSM library\n");
  printf("  • Timer-based automatic transitions\n");
  printf("  • Manual mode with button control\n");
  printf("  • STDIO status reports\n");
  printf("  • LCD real-time display\n");
  printf("\n");
  printf("Type 'help' for available commands\n");
  printf("Type 'diagram' to see state diagram\n");
  printf("Type 'table' to see transition table\n");
  printf("════════════════════════════════════════════════════\n");
  printf("\n");
  
  // Initial LCD
  dd_lcd_write(0, 0, "Traffic Light");
  dd_lcd_write(1, 0, "Starting...");
  
  delay(2000);
  updateLCD();
  
  printf("[TRAFFIC LIGHT] System initialized\n");
  printf("[TRAFFIC LIGHT] Starting in AUTO mode\n\n");
}

void loop_lab6_2() {
  static char commandBuffer[LAB62_COMMAND_BUFFER_SIZE];
  static bool commandReady = false;
  
  unsigned long now = millis();
  
  // Non-blocking command reading
  if (Serial.available()) {
    static int bufferIndex = 0;
    char c = Serial.read();
    
    if (c == '\n' || c == '\r') {
      if (bufferIndex > 0) {
        commandBuffer[bufferIndex] = '\0';
        commandReady = true;
      }
    } else if (bufferIndex < LAB62_COMMAND_BUFFER_SIZE - 1) {
      commandBuffer[bufferIndex++] = c;
    }
    
    if (commandReady) {
      processCommand(commandBuffer);
      bufferIndex = 0;
      commandReady = false;
      memset(commandBuffer, 0, sizeof(commandBuffer));
    }
  }
  
  // Check button press
  if (button.wasPressed(now)) {
    printf("[EVENT] Button pressed - manual transition\n");
    trafficFSM.processEvent(EVENT_BUTTON_PRESS);
    updateLCD();
  }
  
  // Check timeout for auto mode
  checkTimeout();
  
  // Update LCD
  updateLCD();
  
  // Periodic status report
  printStatusReport();
}
#ifndef LAB6_2_HPP
#define LAB6_2_HPP

/**
 * @file lab6_2.hpp
 * @brief Lab 6.2 - Finite State Machine: Traffic Light (Semafor)
 * 
 * Implements FSM for traffic light control:
 * - GREEN: Cars can pass
 * - YELLOW: Prepare to stop
 * - RED: Stop
 * 
 * Automatic transitions based on timers + manual control option.
 */

void setup_lab6_2();
void loop_lab6_2();

#endif // LAB6_2_HPP

