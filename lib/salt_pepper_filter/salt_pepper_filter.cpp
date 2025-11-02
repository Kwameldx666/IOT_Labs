#include "salt_pepper_filter.hpp"
#include <stdlib.h>

SaltPepperFilter::SaltPepperFilter(uint8_t windowSize)
    : m_windowSize(windowSize)
    , m_buffer(nullptr)
    , m_index(0)
    , m_count(0) {
  
  // Ensure odd window size
  if (m_windowSize % 2 == 0) {
    m_windowSize++;
  }
  
  // Limit window size
  if (m_windowSize < 3) m_windowSize = 3;
  if (m_windowSize > 15) m_windowSize = 15;
  
  m_buffer = new float[m_windowSize];
  for (uint8_t i = 0; i < m_windowSize; i++) {
    m_buffer[i] = 0.0f;
  }
}

SaltPepperFilter::~SaltPepperFilter() {
  if (m_buffer != nullptr) {
    delete[] m_buffer;
  }
}

float SaltPepperFilter::filter(float sample) {
  // Add sample to circular buffer
  m_buffer[m_index] = sample;
  m_index = (m_index + 1) % m_windowSize;
  
  if (m_count < m_windowSize) {
    m_count++;
  }
  
  // Return median
  return calculateMedian();
}

void SaltPepperFilter::reset() {
  m_index = 0;
  m_count = 0;
  for (uint8_t i = 0; i < m_windowSize; i++) {
    m_buffer[i] = 0.0f;
  }
}

float SaltPepperFilter::calculateMedian() {
  if (m_count == 0) return 0.0f;
  
  // Create temporary sorted array
  float* sorted = new float[m_count];
  sortBuffer(sorted);
  
  // Get median
  float median;
  uint8_t mid = m_count / 2;
  
  if (m_count % 2 == 0) {
    // Even number: average of two middle values
    median = (sorted[mid - 1] + sorted[mid]) / 2.0f;
  } else {
    // Odd number: middle value
    median = sorted[mid];
  }
  
  delete[] sorted;
  return median;
}

void SaltPepperFilter::sortBuffer(float* sorted) {
  // Copy valid samples
  for (uint8_t i = 0; i < m_count; i++) {
    sorted[i] = m_buffer[i];
  }
  
  // Simple bubble sort (sufficient for small windows)
  for (uint8_t i = 0; i < m_count - 1; i++) {
    for (uint8_t j = 0; j < m_count - i - 1; j++) {
      if (sorted[j] > sorted[j + 1]) {
        float temp = sorted[j];
        sorted[j] = sorted[j + 1];
        sorted[j + 1] = temp;
      }
    }
  }
}
#ifndef SALT_PEPPER_FILTER_HPP
#define SALT_PEPPER_FILTER_HPP

#include <stdint.h>

/**
 * @file salt_pepper_filter.hpp
 * @brief Reusable Salt & Pepper (Median) Filter
 * 
 * Removes impulse noise from signals using median filtering.
 * Effective against salt-and-pepper noise (random high/low spikes).
 */

/**
 * @class SaltPepperFilter
 * @brief Median filter for impulse noise removal
 */
class SaltPepperFilter {
public:
  /**
   * @brief Constructor
   * @param windowSize Size of the median window (must be odd, 3-15 recommended)
   */
  explicit SaltPepperFilter(uint8_t windowSize = 5);
  
  /**
   * @brief Destructor
   */
  ~SaltPepperFilter();

  /**
   * @brief Add sample and get filtered output
   * @param sample New sample value
   * @return Median-filtered value
   */
  float filter(float sample);

  /**
   * @brief Reset filter state
   */
  void reset();

  /**
   * @brief Get current window size
   * @return Window size
   */
  uint8_t getWindowSize() const { return m_windowSize; }

private:
  uint8_t m_windowSize;
  float* m_buffer;
  uint8_t m_index;
  uint8_t m_count;

  float calculateMedian();
  void sortBuffer(float* sorted);
};

#endif // SALT_PEPPER_FILTER_HPP

