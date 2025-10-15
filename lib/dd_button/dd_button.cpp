#include "dd_button.hpp"

/**
 * Initializes the default button pin with the internal pull-up resistor.
 * Button is expected to short the pin to ground when pressed.
 */
void ButtonIni() {
  ButtonIniPin(BUTTON_PIN);
}

/**
 * Reads the default button state using the shared button helpers.
 * @return true if the button is currently pressed, false otherwise
 */
bool ButtonCheckState() {
  return ButtonCheckStatePin(BUTTON_PIN);
}

/**
 * Initializes an arbitrary button pin with the internal pull-up enabled.
 */
void ButtonIniPin(uint8_t pin) {
  pinMode(pin, INPUT_PULLUP);
}

/**
 * Reads the state of an arbitrary button using inverted logic.
 * @return true when the button is pressed (pin pulled low)
 */
bool ButtonCheckStatePin(uint8_t pin) {
  return digitalRead(pin) == LOW;
}
