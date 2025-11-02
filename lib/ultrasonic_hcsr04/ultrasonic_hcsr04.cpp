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
#ifndef ULTRASONIC_HCSR04_HPP
#define ULTRASONIC_HCSR04_HPP

#include <stdint.h>

/**
 * @file ultrasonic_hcsr04.hpp
 * @brief HC-SR04 Ultrasonic Distance Sensor Driver
 * 
 * Measures distance using ultrasonic pulse timing.
 * Distance = (pulse_time_us * speed_of_sound) / 2
 * Speed of sound ≈ 343 m/s at 20°C
 */

/**
 * @class UltrasonicHCSR04
 * @brief HC-SR04 ultrasonic sensor driver
 */
class UltrasonicHCSR04 {
public:
  /**
   * @brief Constructor
   * @param trigPin Trigger pin (output)
   * @param echoPin Echo pin (input)
   * @param maxDistance Maximum distance to measure (cm)
   */
  UltrasonicHCSR04(uint8_t trigPin, uint8_t echoPin, uint16_t maxDistance = 400);
  
  /**
   * @brief Initialize sensor
   */
  void begin();
  
  /**
   * @brief Measure distance
   * @return Distance in centimeters (0 = error/timeout)
   */
  uint16_t measureDistanceCm();
  
  /**
   * @brief Measure distance in inches
   * @return Distance in inches
   */
  uint16_t measureDistanceInch();
  
  /**
   * @brief Get last measurement
   * @return Last measured distance (cm)
   */
  uint16_t getLastDistance() const { return m_lastDistance; }
  
  /**
   * @brief Check if last measurement was valid
   * @return true if valid
   */
  bool isLastMeasurementValid() const { return m_lastDistance > 0; }

private:
  uint8_t m_trigPin;
  uint8_t m_echoPin;
  uint16_t m_maxDistance;
  uint16_t m_lastDistance;
  uint32_t m_timeout;
  
  void sendTriggerPulse();
  uint32_t measureEchoPulse();
  uint16_t pulseDurationToDistance(uint32_t duration);
};

#endif // ULTRASONIC_HCSR04_HPP

