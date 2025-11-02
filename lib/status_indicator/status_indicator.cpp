#include "status_indicator.hpp"
#include <Arduino.h>

StatusIndicator::StatusIndicator(uint8_t successPin, uint8_t failurePin)
    : m_successPin(successPin), m_failurePin(failurePin) {
}

void StatusIndicator::begin() {
  pinMode(m_successPin, OUTPUT);
  pinMode(m_failurePin, OUTPUT);
  clear();
}

void StatusIndicator::show(StatusType status) {
  clear();
  
  switch (status) {
    case StatusType::SUCCESS:
      digitalWrite(m_successPin, HIGH);
      break;
      
    case StatusType::FAILURE:
      digitalWrite(m_failurePin, HIGH);
      break;
      
    case StatusType::WAITING:
    case StatusType::IDLE:
      // Both LEDs off
      break;
  }
}

void StatusIndicator::showAndHold(StatusType status, uint32_t durationMs, uint32_t tickMs) {
  show(status);
  
  unsigned long start = millis();
  while (millis() - start < durationMs) {
    delay(tickMs);  // Yield to scheduler
  }
}

void StatusIndicator::clear() {
  digitalWrite(m_successPin, LOW);
  digitalWrite(m_failurePin, LOW);
}
// Note: removed mistakenly appended header duplication at end of file.

