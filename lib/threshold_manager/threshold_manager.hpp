#ifndef THRESHOLD_MANAGER_HPP
#define THRESHOLD_MANAGER_HPP

#include <stdint.h>

enum class ThresholdLevel {
  NORMAL,
  WARNING,
  CRITICAL
};

class ThresholdManager {
public:
  ThresholdManager(float warningThreshold, float criticalThreshold, float hysteresis = 5.0f);
  ThresholdLevel update(float value);
  ThresholdLevel getCurrentLevel() const { return m_currentLevel; }
  bool hasLevelChanged() const { return m_levelChanged; }
  float getWarningThreshold() const { return m_warningThreshold; }
  float getCriticalThreshold() const { return m_criticalThreshold; }
  static const char* levelToString(ThresholdLevel level);

private:
  float m_warningThreshold;
  float m_criticalThreshold;
  float m_hysteresis;
  ThresholdLevel m_currentLevel;
  bool m_levelChanged;
};

#endif
