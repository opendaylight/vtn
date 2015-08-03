/*
 * Copyright (c) 2014-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef UPLL_PRIORITY_TASK_QUEUE_HH_
#define UPLL_PRIORITY_TASK_QUEUE_HH_

#include "pfcxx/task_queue.hh"
#include "cxx/pfcxx/synch.hh"
#include "uncxx/upll_log.hh"

namespace unc {
namespace upll {
namespace config_momgr {

  enum TaskPriority {
    // Priority tasks take ciritical lock. When critical write lock is
    // taken, read/write lock is not allowed for normal task.
    kCriticalTaskPriority = 1,
    // Normal tasks take normal lock
    kNormalTaskPriority = 2
  };

class TaskScheduler {
 public:
  static const uint32_t kUpllPriorTaskConcurrency = 1;
  static const uint32_t kUpllPriorTaskDetached = 0;

  // Task class is used to create the mutex and mutex cond
  // which are required while adding new task to FIFO scheduler
  class Task {
   public:
    Task(TaskPriority priority, const char *calling_func) {
      mutex_lock_ = new pfc::core::Mutex;
      mutex_cond_ = new pfc::core::Condition;
      priority_ = priority;
      calling_func_ = calling_func;
    }
    ~Task() {
      if (mutex_lock_)
        delete mutex_lock_;
      if (mutex_cond_)
        delete mutex_cond_;
    }
   public:
    pfc::core::Mutex *mutex_lock_;
    pfc::core::Condition *mutex_cond_;
    TaskPriority priority_;
    const char *calling_func_;          // calling function name
  };

  TaskScheduler() :
      prior_taskq_(PFC_TASKQ_INVALID_ID),
      prior_tasks_queued(0) {
  }
  ~TaskScheduler();
  // Create task queue for FIFO scheduler
  bool Init();
  // Allocate Task whenever new task is added to scheduler
  Task *NewTask(TaskPriority priority, const char *calling_func) {
    return new Task(priority, calling_func);
  }
  // To add priority task to the taskq of scheduler
  bool AllowExecution(Task *task);
  Task *AllowExecution(TaskPriority priority, const char *calling_func) {
    Task *task = new Task(priority, calling_func);
    if (!AllowExecution(task)) {
      return NULL;
    }
    return task;
  }
  // Signal to scheduler once the priority task is completed
  bool DoneExecution(Task *task);
  // Clear the taskq of scheduler
  bool Clear(const pfc_timespec_t *ts = NULL);

 private:
  static void ExecuteTask(void *arg_ptr);
  // Destructor which gets called at the end of prior task execution
  inline static void DtorTask(void *arg_ptr) {
    Task *task = reinterpret_cast<Task *>(arg_ptr);
    UPLL_LOG_DEBUG("Task end: %s", task->calling_func_);
    delete task;
    ++prior_tasks_processed;
  }
  pfc_taskq_t prior_taskq_;     // Priority task queue
  uint32_t prior_tasks_queued;  // No of prior tasks queued
  static uint32_t prior_tasks_processed;  // No of prior tasks processed
};

}  // namespace config_momgr
}  // namespace upll
}  // namespace unc

#endif  // UPLL_PRIORITY_TASK_QUEUE_HH_

