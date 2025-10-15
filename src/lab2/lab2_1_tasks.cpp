#include "lab2/lab2_1_tasks.hpp"

#include <stdio.h>

#include "config.h"
#include "dd_button.hpp"
#include "dd_led.hpp"
#include "lab2/lab2_1_config.hpp"
#include "lab2/lab2_1_shared.hpp"

namespace {

bool debounceRisingEdge(bool currentState, bool& lastState,
                        unsigned long now, unsigned long& lastTime) {
  bool triggered = false;
  if (currentState && !lastState) {
    if ((now - lastTime) >= BUTTON_DEBOUNCE_MS) {
      triggered = true;
      lastTime = now;
    }
  }
  lastState = currentState;
  return triggered;
}

}  // namespace

void taskButtonAndLed(unsigned long now) {
  static bool lastButtonState = false;
  static unsigned long lastPressTimeMs = 0;

  const bool pressed = ButtonCheckStatePin(BUTTON_TOGGLE_PIN);
  if (debounceRisingEdge(pressed, lastButtonState, now, lastPressTimeMs)) {
    SharedState& shared = lab2GetSharedState();
    shared.mainLedOn = !shared.mainLedOn;
    shared.lastMainToggleMs = now;
    ++shared.mainToggleCount;

    if (shared.mainLedOn) {
      LedOn_13();
      if (shared.blinkLedOn) {
        LedOffGreen();
        shared.blinkLedOn = false;
      }
    } else {
      LedOff_13();
    }
  }
}

void taskBlinkController(unsigned long now) {
  SharedState& shared = lab2GetSharedState();

  if (shared.mainLedOn) {
    if (shared.blinkLedOn) {
      LedOffGreen();
      shared.blinkLedOn = false;
    }
    shared.blinkSuppressed = true;
    shared.lastBlinkTransitionMs = now;
    shared.lastBlinkSampleMs = now;
    return;
  }

  shared.blinkSuppressed = false;

  if (shared.blinkLedOn) {
    shared.blinkOnAccumMs += now - shared.lastBlinkSampleMs;
  }
  shared.lastBlinkSampleMs = now;

  const uint32_t elapsed = now - shared.lastBlinkTransitionMs;

  if (shared.blinkLedOn) {
    if (elapsed >= shared.blinkOnTargetMs) {
      LedOffGreen();
      shared.blinkLedOn = false;
      shared.lastBlinkTransitionMs = now;
    }
  } else {
    if (elapsed >= kBlinkOffDurationMs) {
      LedOnGreen();
      shared.blinkLedOn = true;
      shared.lastBlinkTransitionMs = now;
      ++shared.blinkCycleCount;
    }
  }
}

void taskStateVariable(unsigned long now) {
  SharedState& shared = lab2GetSharedState();

  static bool lastIncState = false;
  static bool lastDecState = false;
  static unsigned long lastIncTimeMs = 0;
  static unsigned long lastDecTimeMs = 0;

  const bool incPressed = ButtonCheckStatePin(BUTTON_INC_PIN);
  if (debounceRisingEdge(incPressed, lastIncState, now, lastIncTimeMs)) {
    if (shared.blinkWindowUnits < kBlinkWindowMaxUnits) {
      ++shared.blinkWindowUnits;
      ++shared.incAdjustCount;
    }
    shared.blinkOnTargetMs =
        static_cast<uint32_t>(shared.blinkWindowUnits) * kBlinkBaseUnitMs;
  }

  const bool decPressed = ButtonCheckStatePin(BUTTON_DEC_PIN);
  if (debounceRisingEdge(decPressed, lastDecState, now, lastDecTimeMs)) {
    if (shared.blinkWindowUnits > kBlinkWindowMinUnits) {
      --shared.blinkWindowUnits;
      ++shared.decAdjustCount;
    }
    shared.blinkOnTargetMs =
        static_cast<uint32_t>(shared.blinkWindowUnits) * kBlinkBaseUnitMs;
  }
}

void taskIdleReport(unsigned long now) {
  SharedState& shared = lab2GetSharedState();
  if ((now - shared.lastIdleReportMs) < kIdleReportPeriodMs) {
    return;
  }
  shared.lastIdleReportMs = now;

  const char* blinkState =
      shared.blinkLedOn ? "ON" : (shared.blinkSuppressed ? "HOLD" : "OFF");

  printf("[Lab2.1] main:%s blink:%s win:%u on:%lums off:%lums toggles:%lu "
         "cycles:%lu inc:%lu dec:%lu on_acc:%lums\n",
         shared.mainLedOn ? "ON" : "OFF", blinkState,
         static_cast<unsigned>(shared.blinkWindowUnits),
         static_cast<unsigned long>(shared.blinkOnTargetMs),
         static_cast<unsigned long>(kBlinkOffDurationMs),
         static_cast<unsigned long>(shared.mainToggleCount),
         static_cast<unsigned long>(shared.blinkCycleCount),
         static_cast<unsigned long>(shared.incAdjustCount),
         static_cast<unsigned long>(shared.decAdjustCount),
         static_cast<unsigned long>(shared.blinkOnAccumMs));
}
