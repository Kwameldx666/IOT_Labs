#ifndef SALT_PEPPER_FILTER_HPP
#define SALT_PEPPER_FILTER_HPP

#include <stdint.h>

class SaltPepperFilter {
public:
  explicit SaltPepperFilter(uint8_t windowSize = 5);
  ~SaltPepperFilter();
  float filter(float sample);
  void reset();
  uint8_t getWindowSize() const { return m_windowSize; }

private:
  uint8_t m_windowSize;
  float* m_buffer;
  uint8_t m_index;
  uint8_t m_count;
  float calculateMedian();
  void sortBuffer(float* sorted);
};

#endif
