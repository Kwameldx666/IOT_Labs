#include "relay_driver.hpp"
#include <Arduino.h>

// ============================================================================
// Hardware Abstraction Layer (HAL) Implementation
// ============================================================================

static void RelayHAL_initPin(uint8_t pin) {
  pinMode(pin, OUTPUT);
}

static void RelayHAL_setPinState(uint8_t pin, bool state) {
  digitalWrite(pin, state ? HIGH : LOW);
}

// ============================================================================
// Driver Layer Implementation
// ============================================================================

RelayDriver::RelayDriver(uint8_t pin, bool activeHigh)
    : m_pin(pin)
    , m_activeHigh(activeHigh)
    , m_state(RelayState::OFF) {
}

void RelayDriver::begin() {
  RelayHAL_initPin(m_pin);
  turnOff();  // Safe default state
}

void RelayDriver::turnOn() {
  m_state = RelayState::ON;
  updateHardware();
}

void RelayDriver::turnOff() {
  m_state = RelayState::OFF;
  updateHardware();
}

void RelayDriver::toggle() {
  if (m_state == RelayState::ON) {
    turnOff();
  } else {
    turnOn();
  }
}

void RelayDriver::setState(RelayState state) {
  if (state == RelayState::ON) {
    turnOn();
  } else {
    turnOff();
  }
}

void RelayDriver::updateHardware() {
  bool pinState;
  
  if (m_activeHigh) {
    // Active-high: ON = HIGH, OFF = LOW
    pinState = (m_state == RelayState::ON);
  } else {
    // Active-low: ON = LOW, OFF = HIGH
    pinState = (m_state == RelayState::OFF);
  }
  
  RelayHAL_setPinState(m_pin, pinState);
}
