/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef _PFCXX_TIMER_HH
#define _PFCXX_TIMER_HH

/*
 * Definition for C++ timer that's a wrapper
 * for PFC core library for C lang
 */

#include <pfc/timer.h>
#include <boost/noncopyable.hpp>
#include <boost/function.hpp>
#include <string>

namespace pfc {
namespace core {

typedef boost::function<void (void)> timer_func_t;

/*
 * Class Timer
 */

class Timer
    : boost::noncopyable {
 public:
  static Timer* create
      (pfc_taskq_t tqid, const pfc_timespec_t *resolution = NULL) {
    return new Timer(tqid);
  }
  int post(const pfc_timespec_t *timeout, const timer_func_t &func,
           pfc_timeout_t *toidp) {
    return 0;
  }

  /*
   * destructor that destroy a timer
   */
  ~Timer() {}

  inline pfc_timer_t getId() {
    return _timer_id;
  }
 private:
  explicit Timer(pfc_timer_t id) {
  }
  pfc_timer_t _timer_id;
};
}  // namespace core
}  // namespace pfc

#endif  /* !_PFCXX_TIMER_HH */
