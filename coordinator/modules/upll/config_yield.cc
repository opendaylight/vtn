/*
 * Copyright (c) 2014-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include "config_yield.hh"
#include "config_mgr.hh"

namespace unc {
namespace upll {
namespace config_momgr {

upll_rc_t NormalTaskYield::BlockBegin() {
  UPLL_FUNC_TRACE;
  upll_rc_t urc = UPLL_RC_SUCCESS;
  switch (op_) {
    case YIELD_OP_OPER_STATUS_CTR_EVENT:
      cfg_lock_->Lock(kNormalTaskPriority,
                      UPLL_DT_RUNNING, ConfigLock::CFG_READ_LOCK);
      break;
    default:
      UPLL_LOG_FATAL("Invalid operation(%d) for yield request", op_);
      urc = UPLL_RC_ERR_GENERIC;
      break;
  }
  return urc;
}

upll_rc_t NormalTaskYield::BlockEnd() {
  UPLL_FUNC_TRACE;
  upll_rc_t urc = UPLL_RC_SUCCESS;
  switch (op_) {
    case YIELD_OP_OPER_STATUS_CTR_EVENT:
      cfg_lock_->Unlock(kNormalTaskPriority, UPLL_DT_RUNNING);
      break;
    default:
      UPLL_LOG_FATAL("Invalid operation(%d) for yield request", op_);
      urc = UPLL_RC_ERR_GENERIC;
      break;
  }
  return urc;
}

}  // namespace config_momgr
}  // namespace upll
}  // namespace unc


