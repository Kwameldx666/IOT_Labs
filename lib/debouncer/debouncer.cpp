#include "debouncer.hpp"

Debouncer::Debouncer(uint32_t debounceTimeMs)
    : m_lastState(false)
    , m_lastTriggerTime(0)
    , m_debounceTime(debounceTimeMs) {
}

bool Debouncer::updateRisingEdge(bool currentState, unsigned long currentTimeMs) {
  bool triggered = false;
  
  // Detect rising edge (transition from false to true)
  if (currentState && !m_lastState) {
    // Check if enough time has passed since last trigger
    if ((currentTimeMs - m_lastTriggerTime) >= m_debounceTime) {
      triggered = true;
      m_lastTriggerTime = currentTimeMs;
    }
  }
  
  m_lastState = currentState;
  return triggered;
}

void Debouncer::reset() {
  m_lastState = false;
  m_lastTriggerTime = 0;
}
#ifndef DEBOUNCER_HPP
#define DEBOUNCER_HPP

#include <stdint.h>

/**
 * @file debouncer.hpp
 * @brief Reusable button debouncing utility
 * 
 * Provides debouncing logic for digital inputs to prevent
 * multiple triggers from mechanical switch bounce.
 */

/**
 * @class Debouncer
 * @brief Debouncer for rising edge detection
 */
class Debouncer {
public:
  /**
   * @brief Constructor
   * @param debounceTimeMs Minimum time between valid state changes
   */
  explicit Debouncer(uint32_t debounceTimeMs = 35);

  /**
   * @brief Update debouncer with current state
   * @param currentState Current button/input state (true = pressed/active)
   * @param currentTimeMs Current time in milliseconds
   * @return true if a valid rising edge was detected
   */
  bool updateRisingEdge(bool currentState, unsigned long currentTimeMs);

  /**
   * @brief Reset debouncer state
   */
  void reset();

  /**
   * @brief Get last state
   * @return Last registered state
   */
  bool getLastState() const { return m_lastState; }

private:
  bool m_lastState;
  unsigned long m_lastTriggerTime;
  uint32_t m_debounceTime;
};

#endif // DEBOUNCER_HPP

