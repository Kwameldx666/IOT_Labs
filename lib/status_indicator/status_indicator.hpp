#ifndef STATUS_INDICATOR_HPP
#define STATUS_INDICATOR_HPP

#include <stdint.h>

enum class StatusType {
  SUCCESS,
  FAILURE,
  WAITING,
  IDLE
};

class StatusIndicator {
public:
  StatusIndicator(uint8_t successPin, uint8_t failurePin);
  void begin();
  void show(StatusType status);
  void showAndHold(StatusType status, uint32_t durationMs, uint32_t tickMs = 10);
  void clear();

private:
  uint8_t m_successPin;
  uint8_t m_failurePin;
};

#endif
