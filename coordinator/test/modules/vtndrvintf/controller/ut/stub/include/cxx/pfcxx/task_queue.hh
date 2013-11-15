/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef _PFCXX_TASK_QUEUE_HH
#define _PFCXX_TASK_QUEUE_HH

/*
 * Definitions for C++ task queue management that's a wrapper
 * for PFC core library for C lang
 */

#include <pfc/taskq.h>
#include <pfc/log.h>
#include <boost/noncopyable.hpp>
#include <boost/function.hpp>
#include <string>
#include <exception>

namespace pfc {
namespace core {


class TaskQueue {
 public:
  ~TaskQueue(void) {}
  inline pfc_taskq_t getId() {
    return 1;
  }
  explicit TaskQueue(pfc_taskq_t id) {
  }
};

class Task {
  friend class TaskQueue;
 public:
  ~Task(void) {}

 private:
  pfc_task_t _t_id;
  pfc_taskq_t _tq_id;
};
}  // namespace core
}  // namespace pfc

#endif  /* !_PFCXX_TASK_QUEUE_HH */
