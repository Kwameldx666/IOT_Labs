#ifndef LAB2_1_SHARED_HPP
#define LAB2_1_SHARED_HPP

#include <stdint.h>

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
};

SharedState& lab2GetSharedState();
void lab2ResetSharedState(unsigned long startMillis);

#endif
