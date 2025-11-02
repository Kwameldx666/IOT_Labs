#include "onoff_controller.hpp"

OnOffController::OnOffController(float setpoint, float hysteresis, ControlMode mode)
    : m_setpoint(setpoint)
    , m_hysteresis(hysteresis)
    , m_mode(mode)
    , m_state(ControlState::OFF)
    , m_currentValue(0.0f)
    , m_error(0.0f) {
}

ControlState OnOffController::update(float currentValue) {
  m_currentValue = currentValue;
  m_error = m_setpoint - currentValue;
  
  // Calculate thresholds
  float upperThreshold = m_setpoint + m_hysteresis;
  float lowerThreshold = m_setpoint - m_hysteresis;
  
  // ON-OFF logic depends on mode
  if (m_mode == ControlMode::HEATING) {
    // HEATING: Turn ON when cold, OFF when hot
    // ON when: value < (setpoint - hysteresis)
    // OFF when: value > (setpoint + hysteresis)
    
    if (currentValue < lowerThreshold) {
      m_state = ControlState::ON;  // Too cold, turn heater ON
    } else if (currentValue > upperThreshold) {
      m_state = ControlState::OFF;  // Too hot, turn heater OFF
    }
    // else: maintain current state (within hysteresis band)
    
  } else {  // COOLING
    // COOLING: Turn ON when hot, OFF when cold
    // ON when: value > (setpoint + hysteresis)
    // OFF when: value < (setpoint - hysteresis)
    
    if (currentValue > upperThreshold) {
      m_state = ControlState::ON;  // Too hot, turn cooler ON
    } else if (currentValue < lowerThreshold) {
      m_state = ControlState::OFF;  // Too cold, turn cooler OFF
    }
    // else: maintain current state (within hysteresis band)
  }
  
  return m_state;
}

void OnOffController::setSetpoint(float setpoint) {
  m_setpoint = setpoint;
}

void OnOffController::setHysteresis(float hysteresis) {
  m_hysteresis = hysteresis;
}

void OnOffController::setMode(ControlMode mode) {
  m_mode = mode;
}

const char* OnOffController::getStateString() const {
  return (m_state == ControlState::ON) ? "ON" : "OFF";
}

const char* OnOffController::getModeString() const {
  return (m_mode == ControlMode::HEATING) ? "HEATING" : "COOLING";
}
#ifndef ONOFF_CONTROLLER_HPP
#define ONOFF_CONTROLLER_HPP

#include <stdint.h>

/**
 * @file onoff_controller.hpp
 * @brief ON-OFF Controller with Hysteresis (Bang-Bang Control)
 * 
 * Implements classic ON-OFF control with hysteresis to prevent oscillation.
 * Commonly used for temperature, humidity, and level control.
 * 
 * Theory:
 *   ON when: value < (setpoint - hysteresis)
 *   OFF when: value > (setpoint + hysteresis)
 *   Maintain state when: within hysteresis band
 */

enum class ControlState {
  OFF = 0,
  ON = 1
};

enum class ControlMode {
  HEATING,    ///< Turn ON when below setpoint (heater)
  COOLING     ///< Turn ON when above setpoint (cooler)
};

/**
 * @class OnOffController
 * @brief ON-OFF controller with hysteresis
 */
class OnOffController {
public:
  /**
   * @brief Constructor
   * @param setpoint Initial setpoint value
   * @param hysteresis Hysteresis band (±)
   * @param mode Control mode (HEATING or COOLING)
   */
  OnOffController(float setpoint, float hysteresis, ControlMode mode = ControlMode::HEATING);
  
  /**
   * @brief Update controller with new measurement
   * @param currentValue Current measured value
   * @return Control state (ON or OFF)
   */
  ControlState update(float currentValue);
  
  /**
   * @brief Set new setpoint
   * @param setpoint New setpoint value
   */
  void setSetpoint(float setpoint);
  
  /**
   * @brief Set new hysteresis
   * @param hysteresis New hysteresis value
   */
  void setHysteresis(float hysteresis);
  
  /**
   * @brief Set control mode
   * @param mode Control mode
   */
  void setMode(ControlMode mode);
  
  /**
   * @brief Get current setpoint
   * @return Setpoint value
   */
  float getSetpoint() const { return m_setpoint; }
  
  /**
   * @brief Get hysteresis
   * @return Hysteresis value
   */
  float getHysteresis() const { return m_hysteresis; }
  
  /**
   * @brief Get control mode
   * @return Control mode
   */
  ControlMode getMode() const { return m_mode; }
  
  /**
   * @brief Get current state
   * @return Control state
   */
  ControlState getState() const { return m_state; }
  
  /**
   * @brief Get error (setpoint - current)
   * @return Error value
   */
  float getError() const { return m_error; }
  
  /**
   * @brief Get upper threshold (setpoint + hysteresis)
   * @return Upper threshold
   */
  float getUpperThreshold() const { return m_setpoint + m_hysteresis; }
  
  /**
   * @brief Get lower threshold (setpoint - hysteresis)
   * @return Lower threshold
   */
  float getLowerThreshold() const { return m_setpoint - m_hysteresis; }
  
  /**
   * @brief Check if output is ON
   * @return true if ON
   */
  bool isOn() const { return m_state == ControlState::ON; }
  
  /**
   * @brief Get state as string
   * @return "ON" or "OFF"
   */
  const char* getStateString() const;
  
  /**
   * @brief Get mode as string
   * @return "HEATING" or "COOLING"
   */
  const char* getModeString() const;

private:
  float m_setpoint;
  float m_hysteresis;
  ControlMode m_mode;
  ControlState m_state;
  float m_currentValue;
  float m_error;
};

#endif // ONOFF_CONTROLLER_HPP

