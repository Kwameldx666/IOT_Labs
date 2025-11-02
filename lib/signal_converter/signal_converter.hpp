#ifndef SIGNAL_CONVERTER_HPP
#define SIGNAL_CONVERTER_HPP

#include <stdint.h>

class SignalConverter {
public:
  SignalConverter(float adcResolution, float referenceVoltage, float voltageScale, float voltageOffset, float minValue, float maxValue);
  float adcToVoltage(uint16_t adcValue) const;
  float voltageToPhysical(float voltage) const;
  float convert(uint16_t adcValue) const;
  float saturate(float value) const;
  float getReferenceVoltage() const { return m_referenceVoltage; }
  float getMinValue() const { return m_minValue; }
  float getMaxValue() const { return m_maxValue; }

private:
  float m_adcResolution;
  float m_referenceVoltage;
  float m_voltageScale;
  float m_voltageOffset;
  float m_minValue;
  float m_maxValue;
};

#endif
