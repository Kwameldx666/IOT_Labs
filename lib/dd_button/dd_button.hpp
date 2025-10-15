#ifndef DD_BUTTON_H_
#define DD_BUTTON_H_

#include <Arduino.h>
#include "config.h"

void ButtonIni();
bool ButtonCheckState();
void ButtonIniPin(uint8_t pin);
bool ButtonCheckStatePin(uint8_t pin);

#endif
