#ifndef LAB2_1_CONFIG_HPP
#define LAB2_1_CONFIG_HPP

#include <stdint.h>

constexpr uint32_t kTaskButtonPeriodMs = 25;
constexpr uint32_t kTaskBlinkPeriodMs = 40;
constexpr uint32_t kTaskStatePeriodMs = 35;

constexpr uint32_t kTaskButtonOffsetMs = 0;
constexpr uint32_t kTaskBlinkOffsetMs = 10;
constexpr uint32_t kTaskStateOffsetMs = 20;

constexpr uint8_t kBlinkWindowMinUnits = 1;
constexpr uint8_t kBlinkWindowMaxUnits = 10;
constexpr uint8_t kBlinkWindowDefaultUnits = 4;

constexpr uint32_t kBlinkBaseUnitMs = 120;
constexpr uint32_t kBlinkOffDurationMs = 220;

constexpr uint32_t kIdleReportPeriodMs = 1000;

#endif
