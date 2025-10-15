#include "lab2/lab2_1_shared.hpp"

#include "lab2/lab2_1_config.hpp"

namespace {
SharedState gSharedState;
}

SharedState& lab2GetSharedState() {
  return gSharedState;
}

void lab2ResetSharedState(unsigned long startMillis) {
  SharedState& state = lab2GetSharedState();
  state = {};
  state.blinkWindowUnits = kBlinkWindowDefaultUnits;
  state.blinkOnTargetMs =
      static_cast<uint32_t>(state.blinkWindowUnits) * kBlinkBaseUnitMs;
  state.lastBlinkTransitionMs = startMillis;
  state.lastBlinkSampleMs = startMillis;
  state.lastIdleReportMs = startMillis - kIdleReportPeriodMs;
}
