#ifndef DEBOUNCER_HPP
#define DEBOUNCER_HPP

#include <stdint.h>

class Debouncer {
public:
  explicit Debouncer(uint32_t debounceTimeMs = 35);
  bool updateRisingEdge(bool currentState, unsigned long currentTimeMs);
  void reset();
  bool getLastState() const { return m_lastState; }

private:
  bool m_lastState;
  unsigned long m_lastTriggerTime;
  uint32_t m_debounceTime;
};

#endif
