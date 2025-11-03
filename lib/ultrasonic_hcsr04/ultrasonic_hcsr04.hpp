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
