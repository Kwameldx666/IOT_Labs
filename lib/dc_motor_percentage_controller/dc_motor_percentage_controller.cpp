#include "dc_motor_percentage_controller.hpp"

DCMotorPercentageController::DCMotorPercentageController(uint8_t enablePin, 
                                                         uint8_t dirPin1, 
                                                         uint8_t dirPin2)
    : m_motor(enablePin, dirPin1, dirPin2)
    , m_currentPower(0) {
}

void DCMotorPercentageController::begin() {
  m_motor.begin();
  stop();
}

void DCMotorPercentageController::setPower(int16_t powerPercent) {
  // Clamp to valid range
  if (powerPercent < -100) powerPercent = -100;
  if (powerPercent > 100) powerPercent = 100;
  
  m_currentPower = powerPercent;
  updateMotor();
}

void DCMotorPercentageController::stop() {
  m_currentPower = 0;
  m_motor.stop();
}

void DCMotorPercentageController::brake() {
  m_currentPower = 0;
  m_motor.brake();
}

uint8_t DCMotorPercentageController::getSpeedPercent() const {
  return (m_currentPower >= 0) ? m_currentPower : -m_currentPower;
}

const char* DCMotorPercentageController::getDirectionString() const {
  if (m_currentPower > 0) {
    return "FORWARD";
  } else if (m_currentPower < 0) {
    return "REVERSE";
  } else {
    return "STOPPED";
  }
}

const char* DCMotorPercentageController::getStateString() const {
  if (m_currentPower == 0) {
    return "STOPPED";
  } else if (m_currentPower > 0) {
    return "RUNNING FORWARD";
  } else {
    return "RUNNING REVERSE";
  }
}

void DCMotorPercentageController::updateMotor() {
  if (m_currentPower == 0) {
    // Stop motor
    m_motor.stop();
  } else if (m_currentPower > 0) {
    // Forward direction
    // Convert percentage (0-100) to PWM (0-255)
    uint8_t pwmValue = (uint8_t)((m_currentPower * 255) / 100);
    m_motor.move(pwmValue, MotorDirection::FORWARD);
  } else {
    // Reverse direction (negative power)
    // Convert percentage (0-100) to PWM (0-255)
    uint8_t pwmValue = (uint8_t)((-m_currentPower * 255) / 100);
    m_motor.move(pwmValue, MotorDirection::BACKWARD);
  }
}
#ifndef DC_MOTOR_PERCENTAGE_CONTROLLER_HPP
#define DC_MOTOR_PERCENTAGE_CONTROLLER_HPP

#include "dc_motor_driver.hpp"

/**
 * @file dc_motor_percentage_controller.hpp
 * @brief Extended DC Motor Controller with Percentage-based Control
 * 
 * Application layer extension for intuitive motor control.
 * Power range: -100% (full reverse) to +100% (full forward)
 * 
 * Architecture:
 * - Application Layer: setPower(-100 to +100)
 * - Driver Layer: DCMotorDriver (0-255 + direction)
 * - HAL Layer: PWM + GPIO
 */

/**
 * @class DCMotorPercentageController
 * @brief High-level motor controller with percentage-based interface
 */
class DCMotorPercentageController {
public:
  /**
   * @brief Constructor
   * @param enablePin PWM pin for speed control
   * @param dirPin1 Direction control pin 1
   * @param dirPin2 Direction control pin 2
   */
  DCMotorPercentageController(uint8_t enablePin, uint8_t dirPin1, uint8_t dirPin2);
  
  /**
   * @brief Initialize motor hardware
   */
  void begin();
  
  /**
   * @brief Set motor power as percentage
   * @param powerPercent Power: -100 (full reverse) to +100 (full forward)
   *                     0 = stop
   */
  void setPower(int16_t powerPercent);
  
  /**
   * @brief Stop motor
   */
  void stop();
  
  /**
   * @brief Brake motor
   */
  void brake();
  
  /**
   * @brief Get current power percentage
   * @return Power (-100 to +100)
   */
  int16_t getPower() const { return m_currentPower; }
  
  /**
   * @brief Get absolute speed (0-100%)
   * @return Speed percentage
   */
  uint8_t getSpeedPercent() const;
  
  /**
   * @brief Get direction string
   * @return "FORWARD", "REVERSE", or "STOPPED"
   */
  const char* getDirectionString() const;
  
  /**
   * @brief Get detailed state string
   * @return State description
   */
  const char* getStateString() const;
  
  /**
   * @brief Check if motor is running
   * @return true if motor is running
   */
  bool isRunning() const { return m_currentPower != 0; }

private:
  DCMotorDriver m_motor;
  int16_t m_currentPower;  // -100 to +100
  
  void updateMotor();
};

#endif // DC_MOTOR_PERCENTAGE_CONTROLLER_HPP

