/**
 * @file lab2_1.cpp
 * @brief Lab 2.1 - Sequential Task Scheduler (Unified Implementation)
 * 
 * Complete implementation of Lab 2.1 in a single file.
 * Uses reusable libraries: TaskScheduler, Debouncer, dd_button, dd_led
 */

#include "lab2_1.hpp"

#include <Arduino.h>
#include <stdio.h>

#include "config.h"
#include "dd_button.hpp"
#include "dd_led.hpp"
#include "task_scheduler.hpp"
#include "debouncer.hpp"

// ============================================================================
// Shared State
// ============================================================================

struct Lab2State {
  bool mainLedOn = false;
  bool blinkLedOn = false;
  bool blinkSuppressed = false;
  uint8_t blinkWindowUnits = LAB2_BLINK_WINDOW_DEFAULT;
  uint32_t blinkOnTargetMs = 0;
  uint32_t blinkOnAccumMs = 0;
  uint32_t lastBlinkTransitionMs = 0;
  uint32_t lastBlinkSampleMs = 0;
  uint32_t lastMainToggleMs = 0;
  uint32_t mainToggleCount = 0;
  uint32_t blinkCycleCount = 0;
  uint32_t incAdjustCount = 0;
  uint32_t decAdjustCount = 0;
  uint32_t lastIdleReportMs = 0;
};

namespace {
  Lab2State gState;
  
  // Debouncers
  Debouncer toggleDebouncer(BUTTON_DEBOUNCE_MS);
  Debouncer incDebouncer(BUTTON_DEBOUNCE_MS);
  Debouncer decDebouncer(BUTTON_DEBOUNCE_MS);
}

// ============================================================================
// Task Implementations
// ============================================================================

void taskButtonAndLed(unsigned long now) {
  const bool pressed = ButtonCheckStatePin(BUTTON_TOGGLE_PIN);
  if (toggleDebouncer.updateRisingEdge(pressed, now)) {
    gState.mainLedOn = !gState.mainLedOn;
    gState.lastMainToggleMs = now;
    ++gState.mainToggleCount;

    if (gState.mainLedOn) {
      LedOn_13();
      if (gState.blinkLedOn) {
        LedOffGreen();
        gState.blinkLedOn = false;
      }
    } else {
      LedOff_13();
    }
  }
}

void taskBlinkController(unsigned long now) {
  if (gState.mainLedOn) {
    if (gState.blinkLedOn) {
      LedOffGreen();
      gState.blinkLedOn = false;
    }
    gState.blinkSuppressed = true;
    gState.lastBlinkTransitionMs = now;
    gState.lastBlinkSampleMs = now;
    return;
  }

  gState.blinkSuppressed = false;

  if (gState.blinkLedOn) {
    gState.blinkOnAccumMs += now - gState.lastBlinkSampleMs;
  }
  gState.lastBlinkSampleMs = now;

  const uint32_t elapsed = now - gState.lastBlinkTransitionMs;

  if (gState.blinkLedOn) {
    if (elapsed >= gState.blinkOnTargetMs) {
      LedOffGreen();
      gState.blinkLedOn = false;
      gState.lastBlinkTransitionMs = now;
    }
  } else {
    if (elapsed >= LAB2_BLINK_OFF_DURATION_MS) {
      LedOnGreen();
      gState.blinkLedOn = true;
      gState.lastBlinkTransitionMs = now;
      ++gState.blinkCycleCount;
    }
  }
}

void taskStateVariable(unsigned long now) {
  const bool incPressed = ButtonCheckStatePin(BUTTON_INC_PIN);
  if (incDebouncer.updateRisingEdge(incPressed, now)) {
    if (gState.blinkWindowUnits < LAB2_BLINK_WINDOW_MAX) {
      ++gState.blinkWindowUnits;
      ++gState.incAdjustCount;
    }
    gState.blinkOnTargetMs = 
        static_cast<uint32_t>(gState.blinkWindowUnits) * LAB2_BLINK_BASE_UNIT_MS;
  }

  const bool decPressed = ButtonCheckStatePin(BUTTON_DEC_PIN);
  if (decDebouncer.updateRisingEdge(decPressed, now)) {
    if (gState.blinkWindowUnits > LAB2_BLINK_WINDOW_MIN) {
      --gState.blinkWindowUnits;
      ++gState.decAdjustCount;
    }
    gState.blinkOnTargetMs = 
        static_cast<uint32_t>(gState.blinkWindowUnits) * LAB2_BLINK_BASE_UNIT_MS;
  }
}

void taskIdleReport(unsigned long now) {
  if ((now - gState.lastIdleReportMs) < LAB2_IDLE_REPORT_PERIOD_MS) {
    return;
  }
  gState.lastIdleReportMs = now;

  const char* blinkState = 
      gState.blinkLedOn ? "ON" : (gState.blinkSuppressed ? "HOLD" : "OFF");

  printf("[Lab2.1] main:%s blink:%s win:%u on:%lums off:%lums toggles:%lu "
         "cycles:%lu inc:%lu dec:%lu on_acc:%lums\n",
         gState.mainLedOn ? "ON" : "OFF", blinkState,
         static_cast<unsigned>(gState.blinkWindowUnits),
         static_cast<unsigned long>(gState.blinkOnTargetMs),
         static_cast<unsigned long>(LAB2_BLINK_OFF_DURATION_MS),
         static_cast<unsigned long>(gState.mainToggleCount),
         static_cast<unsigned long>(gState.blinkCycleCount),
         static_cast<unsigned long>(gState.incAdjustCount),
         static_cast<unsigned long>(gState.decAdjustCount),
         static_cast<unsigned long>(gState.blinkOnAccumMs));
}

// ============================================================================
// Scheduler Setup
// ============================================================================

namespace {
  Task gTasks[] = {
      {taskButtonAndLed, LAB2_TASK_BUTTON_PERIOD_MS, 0},
      {taskBlinkController, LAB2_TASK_BLINK_PERIOD_MS, 0},
      {taskStateVariable, LAB2_TASK_STATE_PERIOD_MS, 0},
  };

  const uint32_t kTaskOffsets[] = {
      LAB2_TASK_BUTTON_OFFSET_MS,
      LAB2_TASK_BLINK_OFFSET_MS,
      LAB2_TASK_STATE_OFFSET_MS
  };

  TaskScheduler gScheduler(gTasks, 3);
}

// ============================================================================
// Public Interface
// ============================================================================

void setup_lab2_1() {
  // Initialize hardware
  LedIni();
  LedOffAll();

  ButtonIniPin(BUTTON_TOGGLE_PIN);
  ButtonIniPin(BUTTON_INC_PIN);
  ButtonIniPin(BUTTON_DEC_PIN);

  // Initialize state
  const unsigned long start = millis();
  gState.blinkOnTargetMs = 
      static_cast<uint32_t>(gState.blinkWindowUnits) * LAB2_BLINK_BASE_UNIT_MS;
  gState.lastBlinkTransitionMs = start;
  gState.lastBlinkSampleMs = start;
  gState.lastIdleReportMs = start - LAB2_IDLE_REPORT_PERIOD_MS;

  // Initialize scheduler
  gScheduler.init(start, kTaskOffsets);

  printf("\n[Lab 2.1] Sequential scheduler ready\n");
  printf("  Buttons: TOGGLE=%u INC=%u DEC=%u\n", 
         BUTTON_TOGGLE_PIN, BUTTON_INC_PIN, BUTTON_DEC_PIN);
}

void loop_lab2_1() {
  gScheduler.runOnce();
  taskIdleReport(gScheduler.getCurrentTime());
}

