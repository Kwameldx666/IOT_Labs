#include "weighted_average_filter.hpp"

WeightedAverageFilter::WeightedAverageFilter(uint8_t windowSize)
    : m_windowSize(windowSize)
    , m_buffer(nullptr)
    , m_weights(nullptr)
    , m_index(0)
    , m_count(0)
    , m_weightSum(0.0f) {
  
  if (m_windowSize < 2) m_windowSize = 2;
  if (m_windowSize > 20) m_windowSize = 20;
  
  m_buffer = new float[m_windowSize];
  m_weights = new float[m_windowSize];
  
  for (uint8_t i = 0; i < m_windowSize; i++) {
    m_buffer[i] = 0.0f;
  }
  
  calculateWeights();
}

WeightedAverageFilter::~WeightedAverageFilter() {
  if (m_buffer != nullptr) {
    delete[] m_buffer;
  }
  if (m_weights != nullptr) {
    delete[] m_weights;
  }
}

void WeightedAverageFilter::calculateWeights() {
  // Linear weights: newer samples have higher weight
  // Weight[i] = (i + 1), where i=0 is oldest, i=windowSize-1 is newest
  m_weightSum = 0.0f;
  
  for (uint8_t i = 0; i < m_windowSize; i++) {
    m_weights[i] = (float)(i + 1);
    m_weightSum += m_weights[i];
  }
}

float WeightedAverageFilter::filter(float sample) {
  // Add sample to circular buffer
  m_buffer[m_index] = sample;
  m_index = (m_index + 1) % m_windowSize;
  
  if (m_count < m_windowSize) {
    m_count++;
  }
  
  // Calculate weighted average
  float sum = 0.0f;
  float weightSum = 0.0f;
  
  for (uint8_t i = 0; i < m_count; i++) {
    // Calculate actual index in buffer (newest first)
    uint8_t bufferIdx = (m_index - 1 - i + m_windowSize) % m_windowSize;
    
    // Weight increases for newer samples
    float weight = m_weights[m_count - 1 - i];
    
    sum += m_buffer[bufferIdx] * weight;
    weightSum += weight;
  }
  
  return (weightSum > 0.0f) ? (sum / weightSum) : 0.0f;
}

void WeightedAverageFilter::reset() {
  m_index = 0;
  m_count = 0;
  for (uint8_t i = 0; i < m_windowSize; i++) {
    m_buffer[i] = 0.0f;
  }
}
#ifndef WEIGHTED_AVERAGE_FILTER_HPP
#define WEIGHTED_AVERAGE_FILTER_HPP

#include <stdint.h>

/**
 * @file weighted_average_filter.hpp
 * @brief Reusable Weighted Moving Average Filter
 * 
 * Applies weighted averaging for signal smoothing.
 * More recent samples have higher weights.
 */

/**
 * @class WeightedAverageFilter
 * @brief Weighted moving average filter
 */
class WeightedAverageFilter {
public:
  /**
   * @brief Constructor
   * @param windowSize Number of samples to average
   */
  explicit WeightedAverageFilter(uint8_t windowSize = 5);
  
  /**
   * @brief Destructor
   */
  ~WeightedAverageFilter();

  /**
   * @brief Add sample and get filtered output
   * @param sample New sample value
   * @return Weighted average
   */
  float filter(float sample);

  /**
   * @brief Reset filter state
   */
  void reset();

  /**
   * @brief Get window size
   * @return Window size
   */
  uint8_t getWindowSize() const { return m_windowSize; }

private:
  uint8_t m_windowSize;
  float* m_buffer;
  float* m_weights;
  uint8_t m_index;
  uint8_t m_count;
  float m_weightSum;

  void calculateWeights();
};

#endif // WEIGHTED_AVERAGE_FILTER_HPP

