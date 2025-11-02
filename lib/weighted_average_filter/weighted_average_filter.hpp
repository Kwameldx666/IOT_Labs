#ifndef WEIGHTED_AVERAGE_FILTER_HPP
#define WEIGHTED_AVERAGE_FILTER_HPP

#include <stdint.h>

class WeightedAverageFilter {
public:
  explicit WeightedAverageFilter(uint8_t windowSize = 3);
  ~WeightedAverageFilter();
  float filter(float sample);
  void reset();
  uint8_t getWindowSize() const { return m_windowSize; }

private:
  uint8_t m_windowSize;
  float* m_buffer;
  float* m_weights;
  float m_weightSum;
  uint8_t m_index;
  uint8_t m_count;
  void calculateWeights();
};

#endif
