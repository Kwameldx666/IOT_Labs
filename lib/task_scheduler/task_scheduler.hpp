#ifndef TASK_SCHEDULER_HPP
#define TASK_SCHEDULER_HPP

#include <stdint.h>
#include <stddef.h>

typedef void (*TaskHandler)(unsigned long currentTimeMs);

struct Task {
  TaskHandler handler;
  uint32_t periodMs;
  uint32_t nextReleaseMs;
};

class TaskScheduler {
public:
  TaskScheduler(Task* tasks, size_t taskCount);
  void init(unsigned long startTimeMs, const uint32_t* offsets = nullptr);
  void runOnce();
  unsigned long getCurrentTime() const;

private:
  Task* m_tasks;
  size_t m_taskCount;
};

#endif
