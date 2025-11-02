#include "rotary_encoder.hpp"
#include <Arduino.h>

// Static instance for ISR
RotaryEncoder* RotaryEncoder::s_instance = nullptr;

// ISR functions (must be global)
void encoderISR_A() {
  if (RotaryEncoder::s_instance != nullptr) {
    RotaryEncoder::s_instance->handleInterruptA();
  }
}

void encoderISR_B() {
  if (RotaryEncoder::s_instance != nullptr) {
    RotaryEncoder::s_instance->handleInterruptB();
  }
}

RotaryEncoder::RotaryEncoder(uint8_t pinA, uint8_t pinB, uint16_t pulsesPerRevolution)
    : m_pinA(pinA)
    , m_pinB(pinB)
    , m_ppr(pulsesPerRevolution)
    , m_position(0)
    , m_lastPosition(0)
    , m_rpm(0.0f) {
  
  s_instance = this;  // Register for ISR
}

void RotaryEncoder::begin() {
  pinMode(m_pinA, INPUT_PULLUP);
  pinMode(m_pinB, INPUT_PULLUP);
  
  // Attach interrupts
  attachInterrupt(digitalPinToInterrupt(m_pinA), encoderISR_A, CHANGE);
  attachInterrupt(digitalPinToInterrupt(m_pinB), encoderISR_B, CHANGE);
  
  m_position = 0;
  m_lastPosition = 0;
  m_rpm = 0.0f;
}

void RotaryEncoder::update(float dt) {
  // Calculate RPM based on position change
  noInterrupts();  // Atomic read
  int32_t currentPosition = m_position;
  interrupts();
  
  int32_t deltaPosition = currentPosition - m_lastPosition;
  m_lastPosition = currentPosition;
  
  // RPM = (pulses / PPR) * (60 / dt)
  // dt in seconds, so (pulses * 60) / (PPR * dt)
  if (dt > 0.0f) {
    float revolutions = static_cast<float>(deltaPosition) / static_cast<float>(m_ppr);
    m_rpm = (revolutions * 60.0f) / dt;
  } else {
    m_rpm = 0.0f;
  }
}

void RotaryEncoder::resetPosition() {
  noInterrupts();
  m_position = 0;
  m_lastPosition = 0;
  interrupts();
}

void RotaryEncoder::handleInterruptA() {
  // Quadrature encoding: read both channels
  bool stateA = digitalRead(m_pinA);
  bool stateB = digitalRead(m_pinB);
  
  // Determine direction
  if (stateA == stateB) {
    m_position++;  // Forward
  } else {
    m_position--;  // Backward
  }
}

void RotaryEncoder::handleInterruptB() {
  // Quadrature encoding: read both channels
  bool stateA = digitalRead(m_pinA);
  bool stateB = digitalRead(m_pinB);
  
  // Determine direction (opposite of A)
  if (stateA != stateB) {
    m_position++;  // Forward
  } else {
    m_position--;  // Backward
  }
}
#ifndef ROTARY_ENCODER_HPP
#define ROTARY_ENCODER_HPP

#include <stdint.h>

/**
 * @file rotary_encoder.hpp
 * @brief Rotary Encoder with RPM calculation
 * 
 * Reads quadrature encoder for:
 * - Position tracking
 * - Speed (RPM) calculation
 * - Direction detection
 * 
 * Uses hardware interrupts for accuracy.
 */

/**
 * @class RotaryEncoder
 * @brief Quadrature encoder reader with RPM calculation
 */
class RotaryEncoder {
public:
  /**
   * @brief Constructor
   * @param pinA Encoder channel A (must support interrupts)
   * @param pinB Encoder channel B (must support interrupts)
   * @param pulsesPerRevolution Encoder pulses per revolution (PPR)
   */
  RotaryEncoder(uint8_t pinA, uint8_t pinB, uint16_t pulsesPerRevolution);
  
  /**
   * @brief Initialize encoder (setup interrupts)
   */
  void begin();
  
  /**
   * @brief Update RPM calculation
   * @param dt Time since last update (seconds)
   */
  void update(float dt);
  
  /**
   * @brief Get current RPM
   * @return Speed in RPM
   */
  float getRPM() const { return m_rpm; }
  
  /**
   * @brief Get position (pulse count)
   * @return Position in pulses
   */
  int32_t getPosition() const { return m_position; }
  
  /**
   * @brief Reset position counter
   */
  void resetPosition();
  
  /**
   * @brief ISR callback for channel A
   * Must be called from interrupt
   */
  void handleInterruptA();
  
  /**
   * @brief ISR callback for channel B
   * Must be called from interrupt
   */
  void handleInterruptB();

private:
  uint8_t m_pinA;
  uint8_t m_pinB;
  uint16_t m_ppr;
  
  volatile int32_t m_position;
  volatile int32_t m_lastPosition;
  float m_rpm;
  
  static RotaryEncoder* s_instance;  // For ISR
};

#endif // ROTARY_ENCODER_HPP

