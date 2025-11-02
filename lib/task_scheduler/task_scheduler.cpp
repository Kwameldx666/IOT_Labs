#include "task_scheduler.hpp"
#include <Arduino.h>

TaskScheduler::TaskScheduler(Task* tasks, size_t taskCount)
    : m_tasks(tasks), m_taskCount(taskCount) {
}

void TaskScheduler::init(unsigned long startTimeMs, const uint32_t* offsets) {
  for (size_t i = 0; i < m_taskCount; ++i) {
    uint32_t offset = (offsets != nullptr) ? offsets[i] : 0;
    m_tasks[i].nextReleaseMs = startTimeMs + offset;
  }
}

void TaskScheduler::runOnce() {
  unsigned long now = millis();

  for (size_t i = 0; i < m_taskCount; ++i) {
    Task& task = m_tasks[i];
    // Use signed comparison to handle timer overflow correctly
    if ((long)(now - task.nextReleaseMs) >= 0) {
      if (task.handler != nullptr) {
        task.handler(now);
      }
      task.nextReleaseMs += task.periodMs;
      // Update time after task execution in case it took long
      now = millis();
    }
  }
}

unsigned long TaskScheduler::getCurrentTime() const {
  return millis();
}
#ifndef TASK_SCHEDULER_HPP
#define TASK_SCHEDULER_HPP

#include <stdint.h>
#include <stddef.h>

/**
 * @file task_scheduler.hpp
 * @brief Reusable sequential task scheduler for periodic tasks
 * 
 * This scheduler executes tasks at specified intervals without blocking.
 * Tasks are executed sequentially in a round-robin fashion.
 */

/// Function pointer type for task handlers
typedef void (*TaskHandler)(unsigned long currentTimeMs);

/**
 * @brief Task descriptor structure
 */
struct Task {
  TaskHandler handler;        ///< Function to execute
  uint32_t periodMs;          ///< Execution period in milliseconds
  uint32_t nextReleaseMs;     ///< Next scheduled execution time
};

/**
 * @class TaskScheduler
 * @brief Sequential task scheduler
 */
class TaskScheduler {
public:
  /**
   * @brief Constructor
   * @param tasks Array of task descriptors
   * @param taskCount Number of tasks in the array
   */
  TaskScheduler(Task* tasks, size_t taskCount);

  /**
   * @brief Initialize scheduler with offsets
   * @param startTimeMs Starting time in milliseconds
   * @param offsets Array of initial time offsets for each task (can be nullptr)
   */
  void init(unsigned long startTimeMs, const uint32_t* offsets = nullptr);

  /**
   * @brief Run one iteration of the scheduler
   * Executes all tasks that are due
   */
  void runOnce();

  /**
   * @brief Get current time in milliseconds
   * @return Current time from millis()
   */
  unsigned long getCurrentTime() const;

private:
  Task* m_tasks;
  size_t m_taskCount;
};

#endif // TASK_SCHEDULER_HPP

