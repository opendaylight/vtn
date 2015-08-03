/*
 * Copyright (c) 2014-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef UPLL_CONFIG_YIELD_HH_
#define UPLL_CONFIG_YIELD_HH_

#include "config_lock.hh"
#include "unc/upll_errno.h"
#include "upll_util.hh"

namespace unc {
namespace upll {
namespace config_momgr {

class TaskYieldIntf {
 public:
  enum YieldOp {
    YIELD_OP_OPER_STATUS_CTR_EVENT = 0,
    YIELD_OP_LASTENUM  // add any new op above this line
  };

  explicit TaskYieldIntf(YieldOp op) {
    op_ = op;
    if (op >= YIELD_OP_LASTENUM) {
      UPLL_LOG_FATAL("Invalid operation(%d) to yield", op);
      return;
    }
  }
  virtual ~TaskYieldIntf() {}
  virtual upll_rc_t BlockBegin() = 0;
  virtual upll_rc_t BlockEnd() = 0;

 protected:
  YieldOp op_;
};

class NormalTaskYield : public TaskYieldIntf {
 public:
  NormalTaskYield(YieldOp op, ConfigLock *cfg_lock) : TaskYieldIntf(op) {
    cfg_lock_ = cfg_lock;
  }
  upll_rc_t BlockBegin();
  upll_rc_t BlockEnd();
 private:
  ConfigLock *cfg_lock_;
};

class ScopedYield {
 public:
  enum LockState {
    LOCKSTATE_INVALID,
    LOCKSTATE_BLOCKED
  };

  explicit ScopedYield(TaskYieldIntf *yld) {
    state_ = LOCKSTATE_INVALID;
    yld_ = yld;
    if (yld_ == NULL) {
      UPLL_LOG_FATAL("TaskYield pointer is NULL");
      return;
    }
    upll_rc_t urc;
    if ((urc = yld_->BlockBegin()) != UPLL_RC_SUCCESS) {
      UPLL_LOG_ERROR("BlockBegin failed, Urc=%d", urc);
      return;
    }
    // Set the state only if BlockBegin is successful
    state_ = LOCKSTATE_BLOCKED;
    return;
  }

  ~ScopedYield() {
    if (yld_ != NULL && state_ == LOCKSTATE_BLOCKED)
      yld_->BlockEnd();
  }

 private:
  TaskYieldIntf *yld_;
  LockState state_;
};

}  // namespace config_momgr
}  // namespace upll
}  // namespace unc

#endif  // UPLL_CONFIG_YIELD_HH_
