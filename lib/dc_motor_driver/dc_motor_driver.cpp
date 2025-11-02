#include "dc_motor_driver.hpp"
#include <Arduino.h>

// ============================================================================
// Hardware Abstraction Layer (HAL) Implementation
// ============================================================================

namespace MotorHAL {
  void initPWM(uint8_t pin) {
    pinMode(pin, OUTPUT);
  }
  
  void setPWM(uint8_t pin, uint8_t value) {
    analogWrite(pin, value);
  }
  
  void initDirectionPin(uint8_t pin) {
    pinMode(pin, OUTPUT);
  }
  
  void setDirectionPin(uint8_t pin, bool state) {
    digitalWrite(pin, state ? HIGH : LOW);
  }
}

// ============================================================================
// Driver Layer Implementation
// ============================================================================

DCMotorDriver::DCMotorDriver(uint8_t enablePin, uint8_t dirPin1, uint8_t dirPin2)
    : m_enablePin(enablePin)
    , m_dirPin1(dirPin1)
    , m_dirPin2(dirPin2)
    , m_speed(0)
    , m_direction(MotorDirection::FORWARD)
    , m_state(MotorState::STOPPED) {
}

void DCMotorDriver::begin() {
  MotorHAL::initPWM(m_enablePin);
  MotorHAL::initDirectionPin(m_dirPin1);
  MotorHAL::initDirectionPin(m_dirPin2);
  stop();  // Safe default
}

void DCMotorDriver::setSpeed(uint8_t speed) {
  m_speed = speed;
  
  if (m_speed > 0) {
    m_state = MotorState::RUNNING;
  } else {
    m_state = MotorState::STOPPED;
  }
  
  updateHardware();
}

void DCMotorDriver::setDirection(MotorDirection direction) {
  m_direction = direction;
  updateHardware();
}

void DCMotorDriver::move(uint8_t speed, MotorDirection direction) {
  m_speed = speed;
  m_direction = direction;
  
  if (m_speed > 0) {
    m_state = MotorState::RUNNING;
  } else {
    m_state = MotorState::STOPPED;
  }
  
  updateHardware();
}

void DCMotorDriver::stop() {
  m_speed = 0;
  m_state = MotorState::STOPPED;
  updateHardware();
}

void DCMotorDriver::brake() {
  m_direction = MotorDirection::BRAKE;
  m_speed = 0;
  m_state = MotorState::STOPPED;
  updateHardware();
}

void DCMotorDriver::updateHardware() {
  // Set PWM speed
  MotorHAL::setPWM(m_enablePin, m_speed);
  
  // Set direction pins based on direction
  switch (m_direction) {
    case MotorDirection::FORWARD:
      MotorHAL::setDirectionPin(m_dirPin1, true);
      MotorHAL::setDirectionPin(m_dirPin2, false);
      break;
      
    case MotorDirection::BACKWARD:
      MotorHAL::setDirectionPin(m_dirPin1, false);
      MotorHAL::setDirectionPin(m_dirPin2, true);
      break;
      
    case MotorDirection::BRAKE:
      // Both pins HIGH for active braking (H-bridge short circuit)
      MotorHAL::setDirectionPin(m_dirPin1, true);
      MotorHAL::setDirectionPin(m_dirPin2, true);
      break;
  }
}

// ============================================================================
// Application Layer Utilities
// ============================================================================

const char* motorDirectionToString(MotorDirection dir) {
  switch (dir) {
    case MotorDirection::FORWARD:  return "FORWARD";
    case MotorDirection::BACKWARD: return "BACKWARD";
    case MotorDirection::BRAKE:    return "BRAKE";
    default:                       return "UNKNOWN";
  }
}

const char* motorStateToString(MotorState state) {
  switch (state) {
    case MotorState::STOPPED: return "STOPPED";
    case MotorState::RUNNING: return "RUNNING";
    default:                  return "UNKNOWN";
  }
}
#ifndef DC_MOTOR_DRIVER_HPP
#define DC_MOTOR_DRIVER_HPP

#include <stdint.h>

/**
 * @file dc_motor_driver.hpp
 * @brief DC Motor Driver with Layered Architecture
 * 
 * Three-layer architecture for L298N H-Bridge control:
 * - Hardware Abstraction Layer (HAL): PWM, GPIO
 * - Driver Layer: Motor control logic
 * - Application Layer: High-level motor commands
 */

// ============================================================================
// Hardware Abstraction Layer (HAL)
// ============================================================================

namespace MotorHAL {
  /**
   * @brief Initialize PWM pin
   * @param pin PWM-capable pin
   */
  void initPWM(uint8_t pin);
  
  /**
   * @brief Set PWM duty cycle
   * @param pin PWM pin
   * @param value PWM value (0-255)
   */
  void setPWM(uint8_t pin, uint8_t value);
  
  /**
   * @brief Initialize direction pin
   * @param pin GPIO pin
   */
  void initDirectionPin(uint8_t pin);
  
  /**
   * @brief Set direction pin state
   * @param pin GPIO pin
   * @param state Pin state
   */
  void setDirectionPin(uint8_t pin, bool state);
}

// ============================================================================
// Driver Layer
// ============================================================================

enum class MotorDirection {
  FORWARD,
  BACKWARD,
  BRAKE
};

enum class MotorState {
  STOPPED,
  RUNNING
};

/**
 * @class DCMotorDriver
 * @brief Low-level DC motor controller
 */
class DCMotorDriver {
public:
  /**
   * @brief Constructor
   * @param enablePin PWM pin for speed control
   * @param dirPin1 Direction control pin 1
   * @param dirPin2 Direction control pin 2
   */
  DCMotorDriver(uint8_t enablePin, uint8_t dirPin1, uint8_t dirPin2);
  
  /**
   * @brief Initialize motor hardware
   */
  void begin();
  
  /**
   * @brief Set motor speed
   * @param speed Speed value (0-255)
   */
  void setSpeed(uint8_t speed);
  
  /**
   * @brief Set motor direction
   * @param direction Desired direction
   */
  void setDirection(MotorDirection direction);
  
  /**
   * @brief Move motor with speed and direction
   * @param speed Speed (0-255)
   * @param direction Direction
   */
  void move(uint8_t speed, MotorDirection direction);
  
  /**
   * @brief Stop motor
   */
  void stop();
  
  /**
   * @brief Brake motor (active braking)
   */
  void brake();
  
  /**
   * @brief Get current speed
   * @return Speed value (0-255)
   */
  uint8_t getSpeed() const { return m_speed; }
  
  /**
   * @brief Get current direction
   * @return Current direction
   */
  MotorDirection getDirection() const { return m_direction; }
  
  /**
   * @brief Get motor state
   * @return Current state
   */
  MotorState getState() const { return m_state; }
  
  /**
   * @brief Check if motor is running
   * @return true if running
   */
  bool isRunning() const { return m_state == MotorState::RUNNING; }

private:
  uint8_t m_enablePin;
  uint8_t m_dirPin1;
  uint8_t m_dirPin2;
  uint8_t m_speed;
  MotorDirection m_direction;
  MotorState m_state;
  
  void updateHardware();
};

// ============================================================================
// Application Layer Extensions
// ============================================================================

/**
 * @brief Get direction as string
 * @param dir Direction
 * @return String representation
 */
const char* motorDirectionToString(MotorDirection dir);

/**
 * @brief Get state as string
 * @param state Motor state
 * @return String representation
 */
const char* motorStateToString(MotorState state);

#endif // DC_MOTOR_DRIVER_HPP

