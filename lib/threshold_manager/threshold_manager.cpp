#include "threshold_manager.hpp"

ThresholdManager::ThresholdManager(float warningThreshold, float criticalThreshold, float hysteresis)
    : m_warningThreshold(warningThreshold)
    , m_criticalThreshold(criticalThreshold)
    , m_hysteresis(hysteresis)
    , m_currentLevel(ThresholdLevel::NORMAL)
    , m_levelChanged(false) {
}

ThresholdLevel ThresholdManager::update(float value) {
  ThresholdLevel previousLevel = m_currentLevel;
  
  if (value >= m_criticalThreshold) {
    m_currentLevel = ThresholdLevel::CRITICAL;
  } else if (value >= m_warningThreshold) {
    m_currentLevel = ThresholdLevel::WARNING;
  } else if (value < m_warningThreshold - m_hysteresis) {
    m_currentLevel = ThresholdLevel::NORMAL;
  }
  
  m_levelChanged = (m_currentLevel != previousLevel);
  return m_currentLevel;
}

const char* ThresholdManager::levelToString(ThresholdLevel level) {
  switch (level) {
    case ThresholdLevel::NORMAL:   return "NORMAL";
    case ThresholdLevel::WARNING:  return "WARNING";
    case ThresholdLevel::CRITICAL: return "CRITICAL";
    default:                        return "UNKNOWN";
  }
}
