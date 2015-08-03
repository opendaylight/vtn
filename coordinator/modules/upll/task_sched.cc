/*
 * Copyright (c) 2014-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <errno.h>
#include <time.h>

#include "uncxx/upll_log.hh"
#include "upll_util.hh"
#include "task_sched.hh"

namespace unc {
namespace upll {
namespace config_momgr {

uint32_t TaskScheduler::prior_tasks_processed = 0;

TaskScheduler::~TaskScheduler() {
  UPLL_FUNC_TRACE;
  UPLL_LOG_INFO("Prior tasks queued: %d, processed: %d",
                 prior_tasks_queued, prior_tasks_processed);
}

bool TaskScheduler::Init() {
  UPLL_FUNC_TRACE;
  if (prior_taskq_ != PFC_TASKQ_INVALID_ID) {
    UPLL_LOG_WARN("prior_taskq_ (%u) already created", prior_taskq_);
    return false;
  }

  prior_tasks_queued = prior_tasks_processed = 0;

  // create task queue for FIFO scheduler
  int err = pfc_taskq_create_named(&prior_taskq_, NULL,
                                   kUpllPriorTaskConcurrency,
                                   "upll_prior_taskq");
  if (err != 0) {
    UPLL_LOG_ERROR("Failed to create prior_taskq_ err=%d", err);
    return false;
  } else {
    UPLL_LOG_INFO("prior_taskq_ %u created successfully", prior_taskq_);
    return true;
  }
}

bool TaskScheduler::AllowExecution(Task *task) {
  UPLL_FUNC_TRACE;
  if (task == NULL) {
    UPLL_LOG_FATAL("NULL argument");
    return false;
  }

  if (prior_taskq_ == PFC_TASKQ_INVALID_ID) {
    UPLL_LOG_FATAL("prior_taskq_ is invalid");
    return false;
  }
  task->mutex_lock_->lock();
  pfc_task_t tid = PFC_TASKQ_INVALID_TASKID;
  int err = pfc_taskq_dispatch_dtor(prior_taskq_,
                                    &TaskScheduler::ExecuteTask,
                                    task,
                                    &TaskScheduler::DtorTask,
                                    kUpllPriorTaskDetached, &tid);
  if (err != 0) {
    task->mutex_lock_->unlock();
    delete task;
    UPLL_LOG_FATAL("Failed to add task err=%d, taskq=%u",
                   err, prior_taskq_);
    return false;
  } else {
    ++prior_tasks_queued;
    UPLL_LOG_TRACE("Task %u is added to taskq %u. queued: %d, processed: %d",
                   tid, prior_taskq_,
                   prior_tasks_queued, prior_tasks_processed);
    task->mutex_cond_->wait(*(task->mutex_lock_));
  }
  return true;
}

bool TaskScheduler::DoneExecution(Task *task) {
  UPLL_FUNC_TRACE;
  if (task == NULL) {
    UPLL_LOG_FATAL("NULL argument");
    return false;
  }
  task->mutex_cond_->signal();
  UPLL_LOG_DEBUG("IPC thread execution completed and "
                 "signalled to prior_taskq");
  task->mutex_lock_->unlock();
  return true;
}

bool TaskScheduler::Clear(const pfc_timespec_t *ts) {
  UPLL_FUNC_TRACE;
  if (prior_taskq_ == PFC_TASKQ_INVALID_ID) {
    UPLL_LOG_FATAL("prior_taskq_ is invalid");
    return false;
  }
  UPLL_LOG_INFO("Prior Tasks queued: %d, processed: %d",
                prior_tasks_queued, prior_tasks_processed);
  int err = pfc_taskq_clear(prior_taskq_, ts);
  if (err != 0) {
    UPLL_LOG_FATAL("Failed to clear prior_taskq_ err=%d", err);
    return false;
  } else {
    UPLL_LOG_INFO("prior_taskq_ cleared successfully");
  }
  prior_tasks_queued = prior_tasks_processed = 0;
  return true;
}

void TaskScheduler::ExecuteTask(void *arg_ptr) {
  UPLL_FUNC_TRACE;
  if (arg_ptr == NULL) {
    UPLL_LOG_ERROR("Arg pointer is NULL");
    return;
  }
  Task *task = reinterpret_cast<Task *>(arg_ptr);
  UPLL_LOG_DEBUG("Priority task start from %s", (task->calling_func_));
  task->mutex_lock_->lock();
  task->mutex_cond_->signal();
  UPLL_LOG_DEBUG("prior_taskq thread signalled and waiting....");
  task->mutex_cond_->wait(*(task->mutex_lock_));
  UPLL_LOG_DEBUG("prior_taskq thread got signal from IPC thread");
  task->mutex_lock_->unlock();
}

}  // namespace config_momgr
}  // namespace upll
}  // namespace unc
