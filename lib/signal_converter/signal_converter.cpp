#include "signal_converter.hpp"

SignalConverter::SignalConverter(float adcResolution,
                                 float referenceVoltage,
                                 float voltageScale,
                                 float voltageOffset,
                                 float minValue,
                                 float maxValue)
    : m_adcResolution(adcResolution)
    , m_referenceVoltage(referenceVoltage)
    , m_voltageScale(voltageScale)
    , m_voltageOffset(voltageOffset)
    , m_minValue(minValue)
    , m_maxValue(maxValue) {
}

float SignalConverter::adcToVoltage(uint16_t adcValue) const {
  // ADC → Voltage conversion
  // Voltage = (ADC_Value / ADC_Resolution) * Reference_Voltage
  return (static_cast<float>(adcValue) / m_adcResolution) * m_referenceVoltage;
}

float SignalConverter::voltageToPhysical(float voltage) const {
  // Voltage → Physical conversion
  // Physical = (Voltage - Offset) * Scale
  // Example: LM35 temp sensor: 10mV/°C → Scale = 100, Offset = 0
  return (voltage - m_voltageOffset) * m_voltageScale;
}

float SignalConverter::convert(uint16_t adcValue) const {
  // Full pipeline: ADC → Voltage → Physical → Saturate
  float voltage = adcToVoltage(adcValue);
  float physical = voltageToPhysical(voltage);
  return saturate(physical);
}

float SignalConverter::saturate(float value) const {
  // Clamp value to valid range
  if (value < m_minValue) return m_minValue;
  if (value > m_maxValue) return m_maxValue;
  return value;
}
#ifndef SIGNAL_CONVERTER_HPP
#define SIGNAL_CONVERTER_HPP

#include <stdint.h>

/**
 * @file signal_converter.hpp
 * @brief Reusable signal conversion utilities
 * 
 * Provides conversions:
 * - ADC → Voltage
 * - Voltage → Physical parameter
 * - Saturation (clamping)
 */

/**
 * @class SignalConverter
 * @brief Signal conversion and conditioning
 */
class SignalConverter {
public:
  /**
   * @brief Constructor
   * @param adcResolution ADC resolution (e.g., 1023 for 10-bit)
   * @param referenceVoltage ADC reference voltage (V)
   * @param voltageScale Scale factor for voltage→physical conversion
   * @param voltageOffset Offset for voltage→physical conversion
   * @param minValue Minimum allowed physical value (for saturation)
   * @param maxValue Maximum allowed physical value (for saturation)
   */
  SignalConverter(float adcResolution,
                  float referenceVoltage,
                  float voltageScale,
                  float voltageOffset,
                  float minValue,
                  float maxValue);

  /**
   * @brief Convert ADC reading to voltage
   * @param adcValue Raw ADC value (0-1023)
   * @return Voltage (V)
   */
  float adcToVoltage(uint16_t adcValue) const;

  /**
   * @brief Convert voltage to physical parameter
   * @param voltage Voltage (V)
   * @return Physical parameter value
   */
  float voltageToPhysical(float voltage) const;

  /**
   * @brief Full conversion: ADC → Voltage → Physical (with saturation)
   * @param adcValue Raw ADC value
   * @return Saturated physical parameter value
   */
  float convert(uint16_t adcValue) const;

  /**
   * @brief Apply saturation (clamping) to value
   * @param value Input value
   * @return Clamped value between min and max
   */
  float saturate(float value) const;

  /**
   * @brief Get reference voltage
   * @return Reference voltage (V)
   */
  float getReferenceVoltage() const { return m_referenceVoltage; }

  /**
   * @brief Get min/max range
   */
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

#endif // SIGNAL_CONVERTER_HPP

