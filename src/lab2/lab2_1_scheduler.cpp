#include "lab2/lab2_1_scheduler.hpp"

#include <Arduino.h>

#include "lab2/lab2_1_config.hpp"
#include "lab2/lab2_1_tasks.hpp"

namespace {

struct SequentialTask {
  void (*handler)(unsigned long now);
  uint32_t periodMs;
  uint32_t nextReleaseMs;
};

SequentialTask gTasks[] = {
    {taskButtonAndLed, kTaskButtonPeriodMs, 0},
    {taskBlinkController, kTaskBlinkPeriodMs, 0},
    {taskStateVariable, kTaskStatePeriodMs, 0},
};

constexpr size_t kTaskCount = sizeof(gTasks) / sizeof(gTasks[0]);

}  // namespace

void lab2SchedulerInit(unsigned long startMillis) {
  gTasks[0].nextReleaseMs = startMillis + kTaskButtonOffsetMs;
  gTasks[1].nextReleaseMs = startMillis + kTaskBlinkOffsetMs;
  gTasks[2].nextReleaseMs = startMillis + kTaskStateOffsetMs;
}

void lab2SchedulerRunOnce() {
  unsigned long now = millis();

  for (size_t i = 0; i < kTaskCount; ++i) {
    SequentialTask& task = gTasks[i];
    if ((long)(now - task.nextReleaseMs) >= 0) {
      task.handler(now);
      task.nextReleaseMs += task.periodMs;
      now = millis();
    }
  }

  taskIdleReport(now);
}
