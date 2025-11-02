#include "button_controller.hpp"
#include <Arduino.h>

ButtonController::ButtonController(uint8_t pin, uint32_t debounceTimeMs, bool activeLow)
    : m_pin(pin)
    , m_activeLow(activeLow)
    , m_debouncer(debounceTimeMs) {
}

void ButtonController::begin() {
  if (m_activeLow) {
    pinMode(m_pin, INPUT_PULLUP);
  } else {
    pinMode(m_pin, INPUT);
  }
}

bool ButtonController::read() const {
  bool state = digitalRead(m_pin);
  // Invert if active-low (button pressed = LOW)
  return m_activeLow ? !state : state;
}

bool ButtonController::wasPressed(unsigned long currentTimeMs) {
  bool currentState = read();
  return m_debouncer.updateRisingEdge(currentState, currentTimeMs);
}
#ifndef BUTTON_CONTROLLER_HPP
#define BUTTON_CONTROLLER_HPP

#include <stdint.h>
#include "debouncer.hpp"

/**
 * @file button_controller.hpp
 * @brief Reusable button controller with integrated debouncing
 * 
 * Combines button reading with debouncing logic for easy use.
 */

/**
 * @class ButtonController
 * @brief Button controller with automatic debouncing
 */
class ButtonController {
public:
  /**
   * @brief Constructor
   * @param pin Arduino pin number for the button
   * @param debounceTimeMs Debounce time in milliseconds (default: 35ms)
   * @param activeLow True if button is active-low (default), false for active-high
   */
  explicit ButtonController(uint8_t pin, uint32_t debounceTimeMs = 35, bool activeLow = true);

  /**
   * @brief Initialize the button pin (call in setup)
   */
  void begin();

  /**
   * @brief Read current button state (debounced)
   * @return true if button is pressed, false otherwise
   */
  bool read() const;

  /**
   * @brief Update and check for press event (rising edge)
   * @param currentTimeMs Current time in milliseconds
   * @return true if button was just pressed (debounced rising edge)
   */
  bool wasPressed(unsigned long currentTimeMs);

  /**
   * @brief Get pin number
   * @return Pin number
   */
  uint8_t getPin() const { return m_pin; }

private:
  uint8_t m_pin;
  bool m_activeLow;
  Debouncer m_debouncer;
};

#endif // BUTTON_CONTROLLER_HPP

