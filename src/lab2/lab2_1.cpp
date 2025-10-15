#include "lab2_1.hpp"

#include <Arduino.h>
#include <stdio.h>

#include "config.h"
#include "dd_button.hpp"
#include "dd_led.hpp"

namespace {

constexpr uint32_t kTask1PeriodMs = 25;
constexpr uint32_t kTask2PeriodMs = 40;
constexpr uint32_t kTask3PeriodMs = 35;

constexpr uint32_t kTask1OffsetMs = 0;
constexpr uint32_t kTask2OffsetMs = 10;
constexpr uint32_t kTask3OffsetMs = 20;

constexpr uint32_t kBlinkBaseUnitMs = 120;
constexpr uint32_t kBlinkOffDurationMs = 220;
constexpr uint8_t kBlinkWindowMin = 1;
constexpr uint8_t kBlinkWindowMax = 10;

constexpr uint32_t kIdleReportPeriodMs = 1000;

struct SharedState {
  bool mainLedOn = false;
  bool blinkLedOn = false;
  bool blinkSuppressed = false;
  uint8_t blinkWindowUnits = 0;
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
} gShared;

struct SequentialTask {
  const char* name;
  void (*handler)(unsigned long now);
  uint32_t periodMs;
  uint32_t offsetMs;
  uint32_t nextReleaseMs;
};

void taskButtonAndLed(unsigned long now);
void taskBlinkController(unsigned long now);
void taskStateVariable(unsigned long now);
void runIdle(unsigned long now);

SequentialTask gTasks[] = {
    {"Button+LED", taskButtonAndLed, kTask1PeriodMs, kTask1OffsetMs, 0},
    {"Blink", taskBlinkController, kTask2PeriodMs, kTask2OffsetMs, 0},
    {"State", taskStateVariable, kTask3PeriodMs, kTask3OffsetMs, 0},
};

constexpr size_t kTaskCount = sizeof(gTasks) / sizeof(gTasks[0]);

void schedulerInit(unsigned long start) {
  for (size_t i = 0; i < kTaskCount; ++i) {
    gTasks[i].nextReleaseMs = start + gTasks[i].offsetMs;
  }
}

void runScheduler() {
  unsigned long now = millis();

  for (size_t i = 0; i < kTaskCount; ++i) {
    SequentialTask& task = gTasks[i];
    if ((long)(now - task.nextReleaseMs) >= 0) {
      task.handler(now);
      task.nextReleaseMs += task.periodMs;
      now = millis();
    }
  }

  runIdle(now);
}

void taskButtonAndLed(unsigned long now) {
  static bool lastButtonState = false;
  static unsigned long lastPressTimeMs = 0;

  const bool pressed = ButtonCheckStatePin(BUTTON_TOGGLE_PIN);

  if (pressed && !lastButtonState) {
    if ((now - lastPressTimeMs) >= BUTTON_DEBOUNCE_MS) {
      gShared.mainLedOn = !gShared.mainLedOn;
      gShared.lastMainToggleMs = now;
      ++gShared.mainToggleCount;

      if (gShared.mainLedOn) {
        LedOn_13();
        if (gShared.blinkLedOn) {
          LedOffGreen();
          gShared.blinkLedOn = false;
        }
      } else {
        LedOff_13();
      }
    }
    lastPressTimeMs = now;
  }

  lastButtonState = pressed;
}

void taskBlinkController(unsigned long now) {
  if (gShared.mainLedOn) {
    if (gShared.blinkLedOn) {
      LedOffGreen();
      gShared.blinkLedOn = false;
    }
    gShared.blinkSuppressed = true;
    gShared.lastBlinkTransitionMs = now;
    gShared.lastBlinkSampleMs = now;
    return;
  }

  gShared.blinkSuppressed = false;

  if (gShared.blinkLedOn) {
    gShared.blinkOnAccumMs += now - gShared.lastBlinkSampleMs;
  }
  gShared.lastBlinkSampleMs = now;

  const uint32_t elapsed = now - gShared.lastBlinkTransitionMs;

  if (gShared.blinkLedOn) {
    if (elapsed >= gShared.blinkOnTargetMs) {
      LedOffGreen();
      gShared.blinkLedOn = false;
      gShared.lastBlinkTransitionMs = now;
    }
  } else {
    if (elapsed >= kBlinkOffDurationMs) {
      LedOnGreen();
      gShared.blinkLedOn = true;
      gShared.lastBlinkTransitionMs = now;
      ++gShared.blinkCycleCount;
    }
  }
}

void taskStateVariable(unsigned long now) {
  static bool lastIncState = false;
  static bool lastDecState = false;
  static unsigned long lastIncTimeMs = 0;
  static unsigned long lastDecTimeMs = 0;

  const bool incPressed = ButtonCheckStatePin(BUTTON_INC_PIN);
  if (incPressed && !lastIncState) {
    if ((now - lastIncTimeMs) >= BUTTON_DEBOUNCE_MS) {
      if (gShared.blinkWindowUnits < kBlinkWindowMax) {
        ++gShared.blinkWindowUnits;
        ++gShared.incAdjustCount;
      }
      gShared.blinkOnTargetMs =
          (uint32_t)gShared.blinkWindowUnits * kBlinkBaseUnitMs;
      lastIncTimeMs = now;
    }
  }
  lastIncState = incPressed;

  const bool decPressed = ButtonCheckStatePin(BUTTON_DEC_PIN);
  if (decPressed && !lastDecState) {
    if ((now - lastDecTimeMs) >= BUTTON_DEBOUNCE_MS) {
      if (gShared.blinkWindowUnits > kBlinkWindowMin) {
        --gShared.blinkWindowUnits;
        ++gShared.decAdjustCount;
      }
      gShared.blinkOnTargetMs =
          (uint32_t)gShared.blinkWindowUnits * kBlinkBaseUnitMs;
      lastDecTimeMs = now;
    }
  }
  lastDecState = decPressed;
}

void runIdle(unsigned long now) {
  if ((now - gShared.lastIdleReportMs) < kIdleReportPeriodMs) {
    return;
  }

  gShared.lastIdleReportMs = now;

  const char* blinkState =
      gShared.blinkLedOn ? "ON" : (gShared.blinkSuppressed ? "HOLD" : "OFF");

  printf("[Lab2.1] main:%s blink:%s win:%u on:%lums off:%lums toggles:%lu "
         "cycles:%lu inc:%lu dec:%lu on_acc:%lums\n",
         gShared.mainLedOn ? "ON" : "OFF", blinkState,
         static_cast<unsigned>(gShared.blinkWindowUnits),
         static_cast<unsigned long>(gShared.blinkOnTargetMs),
         static_cast<unsigned long>(kBlinkOffDurationMs),
         static_cast<unsigned long>(gShared.mainToggleCount),
         static_cast<unsigned long>(gShared.blinkCycleCount),
         static_cast<unsigned long>(gShared.incAdjustCount),
         static_cast<unsigned long>(gShared.decAdjustCount),
         static_cast<unsigned long>(gShared.blinkOnAccumMs));
}

}  // namespace

void setup_lab2_1() {
  LedIni();
  LedOffAll();

  ButtonIniPin(BUTTON_TOGGLE_PIN);
  ButtonIniPin(BUTTON_INC_PIN);
  ButtonIniPin(BUTTON_DEC_PIN);

  gShared = {};
  gShared.blinkWindowUnits = 4;
  gShared.blinkOnTargetMs =
      static_cast<uint32_t>(gShared.blinkWindowUnits) * kBlinkBaseUnitMs;

  const unsigned long start = millis();
  gShared.lastBlinkTransitionMs = start;
  gShared.lastBlinkSampleMs = start;
  gShared.lastIdleReportMs = start - kIdleReportPeriodMs;

  schedulerInit(start);

  printf("\n[Lab 2.1] Sequential scheduler initialised\n");
  printf("  Toggle pin: %u | INC: %u | DEC: %u\n", BUTTON_TOGGLE_PIN,
         BUTTON_INC_PIN, BUTTON_DEC_PIN);
  printf("  Blink window: %u units -> %lums ON / %lums OFF\n",
         static_cast<unsigned>(gShared.blinkWindowUnits),
         static_cast<unsigned long>(gShared.blinkOnTargetMs),
         static_cast<unsigned long>(kBlinkOffDurationMs));
}

void loop_lab2_1() { runScheduler(); }
