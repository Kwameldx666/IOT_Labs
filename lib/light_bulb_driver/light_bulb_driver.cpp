#include "light_bulb_driver.hpp"

LightBulbDriver::LightBulbDriver(uint8_t relayPin, bool activeHigh)
    : m_relay(relayPin, activeHigh) {
}

void LightBulbDriver::begin() {
  m_relay.begin();
  lightOff();  // Safe default: lights off
}

void LightBulbDriver::lightOn() {
  m_relay.turnOn();
}

void LightBulbDriver::lightOff() {
  m_relay.turnOff();
}

void LightBulbDriver::toggle() {
  m_relay.toggle();
}

void LightBulbDriver::setState(LightState state) {
  if (state == LightState::ON) {
    lightOn();
  } else {
    lightOff();
  }
}

LightState LightBulbDriver::getState() const {
  return m_relay.isOn() ? LightState::ON : LightState::OFF;
}

const char* LightBulbDriver::getStateString() const {
  return isOn() ? "ON" : "OFF";
}
#ifndef LIGHT_BULB_DRIVER_HPP
#define LIGHT_BULB_DRIVER_HPP

#include "relay_driver.hpp"

/**
 * @file light_bulb_driver.hpp
 * @brief Electric Light Bulb Driver with Layered Architecture
 * 
 * Application Layer for controlling light bulb through relay.
 * Provides high-level semantic interface.
 * 
 * Architecture layers:
 * - Application Layer (this): lightOn(), lightOff()
 * - Driver Layer (relay): turnOn(), turnOff()
 * - HAL Layer: pinMode(), digitalWrite()
 */

enum class LightState {
  OFF = 0,
  ON = 1
};

/**
 * @class LightBulbDriver
 * @brief Application-level light bulb controller
 */
class LightBulbDriver {
public:
  /**
   * @brief Constructor
   * @param relayPin Pin controlling the relay
   * @param activeHigh true if relay is active-high
   */
  explicit LightBulbDriver(uint8_t relayPin, bool activeHigh = true);
  
  /**
   * @brief Initialize light bulb hardware
   */
  void begin();
  
  /**
   * @brief Turn light ON
   */
  void lightOn();
  
  /**
   * @brief Turn light OFF
   */
  void lightOff();
  
  /**
   * @brief Toggle light state
   */
  void toggle();
  
  /**
   * @brief Set light state
   * @param state Desired light state
   */
  void setState(LightState state);
  
  /**
   * @brief Get current light state
   * @return Current state
   */
  LightState getState() const;
  
  /**
   * @brief Check if light is on
   * @return true if light is on
   */
  bool isOn() const { return getState() == LightState::ON; }
  
  /**
   * @brief Get state as string
   * @return "ON" or "OFF"
   */
  const char* getStateString() const;

private:
  RelayDriver m_relay;  // Driver layer
};

#endif // LIGHT_BULB_DRIVER_HPP

