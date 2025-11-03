#include "ultrasonic_hcsr04.hpp"
#include <Arduino.h>

UltrasonicHCSR04::UltrasonicHCSR04(uint8_t trigPin, uint8_t echoPin, uint16_t maxDistance)
    : m_trigPin(trigPin)
    , m_echoPin(echoPin)
    , m_maxDistance(maxDistance)
    , m_lastDistance(0)
    , m_timeout(0) {
  
  // Calculate timeout based on max distance
  // Distance (cm) = (pulse_time_us * 0.0343) / 2
  // pulse_time_us = (distance * 2) / 0.0343
  m_timeout = (maxDistance * 2 * 1000) / 343 + 1000; // Add 1ms margin
}

void UltrasonicHCSR04::begin() {
  pinMode(m_trigPin, OUTPUT);
  pinMode(m_echoPin, INPUT);
  digitalWrite(m_trigPin, LOW);
  delayMicroseconds(2);
}

uint16_t UltrasonicHCSR04::measureDistanceCm() {
  sendTriggerPulse();
  uint32_t duration = measureEchoPulse();
  m_lastDistance = pulseDurationToDistance(duration);
  return m_lastDistance;
}

uint16_t UltrasonicHCSR04::measureDistanceInch() {
  uint16_t cm = measureDistanceCm();
  return (cm * 10) / 254;  // Convert cm to inches
}

void UltrasonicHCSR04::sendTriggerPulse() {
  // Send 10us HIGH pulse to trigger
  digitalWrite(m_trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(m_trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(m_trigPin, LOW);
}

uint32_t UltrasonicHCSR04::measureEchoPulse() {
  // Wait for echo to go HIGH
  uint32_t startTime = micros();
  while (digitalRead(m_echoPin) == LOW) {
    if (micros() - startTime > m_timeout) {
      return 0;  // Timeout
    }
  }
  
  // Measure HIGH pulse duration
  uint32_t pulseStart = micros();
  while (digitalRead(m_echoPin) == HIGH) {
    if (micros() - pulseStart > m_timeout) {
      return 0;  // Timeout
    }
  }
  uint32_t pulseEnd = micros();
  
  return pulseEnd - pulseStart;
}

uint16_t UltrasonicHCSR04::pulseDurationToDistance(uint32_t duration) {
  if (duration == 0) {
    return 0;  // Error
  }
  
  // Distance (cm) = (pulse_duration_us * 0.0343) / 2
  // Simplified: distance = duration / 58
  uint16_t distance = duration / 58;
  
  // Clamp to max distance
  if (distance > m_maxDistance) {
    return 0;  // Out of range
  }
  
  return distance;
}
