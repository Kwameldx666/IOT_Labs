#ifndef ANALOG_SENSOR_HPP
#define ANALOG_SENSOR_HPP

#include <stdint.h>

class AnalogSensor {
public:
  AnalogSensor(uint8_t pin, float minValue = 0.0f, float maxValue = 1023.0f, uint8_t filterSize = 1);
  void begin();
  uint16_t readRaw();
  float readScaled();
  float readFiltered();
  float getMin() const { return m_minRecorded; }
  float getMax() const { return m_maxRecorded; }
  void resetMinMax();
  uint32_t getSampleCount() const { return m_sampleCount; }

private:
  uint8_t m_pin;
  float m_minValue;
  float m_maxValue;
  uint8_t m_filterSize;
  float* m_filterBuffer;
  uint8_t m_filterIndex;
  float m_minRecorded;
  float m_maxRecorded;
  uint32_t m_sampleCount;
  float scale(uint16_t rawValue);
};

#endif
