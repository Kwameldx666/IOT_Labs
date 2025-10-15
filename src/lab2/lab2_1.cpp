#include "lab2_1.hpp"

#include <Arduino.h>
#include <stdio.h>

#include "config.h"
#include "dd_button.hpp"
#include "dd_led.hpp"
#include "lab2/lab2_1_config.hpp"
#include "lab2/lab2_1_scheduler.hpp"
#include "lab2/lab2_1_shared.hpp"

void setup_lab2_1() {
  LedIni();
  LedOffAll();

  ButtonIniPin(BUTTON_TOGGLE_PIN);
  ButtonIniPin(BUTTON_INC_PIN);
  ButtonIniPin(BUTTON_DEC_PIN);

  const unsigned long start = millis();
  lab2ResetSharedState(start);
  lab2SchedulerInit(start);

  printf("\n[Lab 2.1] Sequential scheduler initialised\n");
  printf("  Toggle pin: %u | INC: %u | DEC: %u\n", BUTTON_TOGGLE_PIN,
         BUTTON_INC_PIN, BUTTON_DEC_PIN);

  const SharedState& shared = lab2GetSharedState();
  printf("  Blink window: %u units -> %lums ON / %lums OFF\n",
         static_cast<unsigned>(shared.blinkWindowUnits),
         static_cast<unsigned long>(shared.blinkOnTargetMs),
         static_cast<unsigned long>(kBlinkOffDurationMs));
}

void loop_lab2_1() {
  lab2SchedulerRunOnce();
}
