#include "analog_sensor.hpp"
#include <Arduino.h>

AnalogSensor::AnalogSensor(uint8_t pin, float minValue, float maxValue, uint8_t filterSize)
    : m_pin(pin)
    , m_minValue(minValue)
    , m_maxValue(maxValue)
    , m_filterSize(filterSize > 0 ? filterSize : 1)
    , m_filterBuffer(nullptr)
    , m_filterIndex(0)
    , m_minRecorded(9999.9f)
    , m_maxRecorded(-9999.9f)
    , m_sampleCount(0) {
  
  if (m_filterSize > 1) {
    m_filterBuffer = new float[m_filterSize];
    for (uint8_t i = 0; i < m_filterSize; i++) {
      m_filterBuffer[i] = 0.0f;
    }
  }
}

void AnalogSensor::begin() {
  pinMode(m_pin, INPUT);
  
  // Read initial value to pre-fill filter buffer
  uint16_t initialRaw = analogRead(m_pin);
  float initialScaled = scale(initialRaw);
  
  if (m_filterBuffer != nullptr) {
    for (uint8_t i = 0; i < m_filterSize; i++) {
      m_filterBuffer[i] = initialScaled;
    }
  }
  
  // Now reset min/max to track from first real reading
  m_minRecorded = initialScaled;
  m_maxRecorded = initialScaled;
  m_sampleCount = 0;
}

uint16_t AnalogSensor::readRaw() {
  return analogRead(m_pin);
}

float AnalogSensor::scale(uint16_t rawValue) {
  // Scale from 0-1023 to minValue-maxValue
  return m_minValue + (rawValue / 1023.0f) * (m_maxValue - m_minValue);
}

float AnalogSensor::readScaled() {
  uint16_t raw = readRaw();
  float scaled = scale(raw);
  
  m_sampleCount++;
  
  // Track min/max
  if (scaled < m_minRecorded) m_minRecorded = scaled;
  if (scaled > m_maxRecorded) m_maxRecorded = scaled;
  
  return scaled;
}

float AnalogSensor::readFiltered() {
  float current = readScaled();
  
  if (m_filterSize <= 1 || m_filterBuffer == nullptr) {
    return current;
  }
  
  // Add to circular buffer
  m_filterBuffer[m_filterIndex] = current;
  m_filterIndex = (m_filterIndex + 1) % m_filterSize;
  
  // Calculate average
  float sum = 0.0f;
  for (uint8_t i = 0; i < m_filterSize; i++) {
    sum += m_filterBuffer[i];
  }
  
  return sum / m_filterSize;
}

void AnalogSensor::resetMinMax() {
  m_minRecorded = 9999.9f;
  m_maxRecorded = -9999.9f;
  m_sampleCount = 0;
}
// Note: removed mistakenly appended header duplication at end of file.

