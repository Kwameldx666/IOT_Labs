#include "led_controller.hpp"
#include <Arduino.h>

LedController::LedController(uint8_t pin)
    : m_pin(pin), m_state(false) {
}

void LedController::begin() {
  pinMode(m_pin, OUTPUT);
  off();
}

void LedController::on() {
  m_state = true;
  digitalWrite(m_pin, HIGH);
}

void LedController::off() {
  m_state = false;
  digitalWrite(m_pin, LOW);
}

void LedController::toggle() {
  if (m_state) {
    off();
  } else {
    on();
  }
}

void LedController::setState(bool state) {
  if (state) {
    on();
  } else {
    off();
  }
}
#ifndef LED_CONTROLLER_HPP
#define LED_CONTROLLER_HPP

#include <stdint.h>

/**
 * @file led_controller.hpp
 * @brief Reusable LED controller with blink state management
 * 
 * Provides a clean interface for controlling LED with timing and state.
 */

/**
 * @class LedController
 * @brief Controls a single LED with timing capabilities
 */
class LedController {
public:
  /**
   * @brief Constructor
   * @param pin Arduino pin number for the LED
   */
  explicit LedController(uint8_t pin);

  /**
   * @brief Initialize the LED pin (call in setup)
   */
  void begin();

  /**
   * @brief Turn LED on
   */
  void on();

  /**
   * @brief Turn LED off
   */
  void off();

  /**
   * @brief Toggle LED state
   */
  void toggle();

  /**
   * @brief Get current LED state
   * @return true if LED is on, false otherwise
   */
  bool isOn() const { return m_state; }

  /**
   * @brief Set LED state
   * @param state true to turn on, false to turn off
   */
  void setState(bool state);

private:
  uint8_t m_pin;
  bool m_state;
};

#endif // LED_CONTROLLER_HPP

