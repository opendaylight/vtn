/*
 * Copyright (c) 2012-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef UPLL_CONFIG_LOCK_HH_
#define UPLL_CONFIG_LOCK_HH_

#include <map>
#include "cxx/pfcxx/synch.hh"

#include "./keytype_upll_ext.h"

namespace unc {
namespace upll {
namespace config_momgr {

class ConfigLock {
 public:
  enum LockType {
    CFG_READ_LOCK = 1,
    CFG_WRITE_LOCK = 2
  };

  ConfigLock() {}
  virtual ~ConfigLock() {}
  bool ReadLock(upll_keytype_datatype_t dt);
  bool WriteLock(upll_keytype_datatype_t dt);
  bool Lock(upll_keytype_datatype_t dt, LockType locktype);
  bool Unlock(upll_keytype_datatype_t dt);
  bool Lock(upll_keytype_datatype_t dt1, LockType locktype1,
            upll_keytype_datatype_t dt2, LockType locktype2);
  bool Unlock(upll_keytype_datatype_t dt1, upll_keytype_datatype_t dt2);
  bool Lock(upll_keytype_datatype_t dt1, LockType locktype1,
            upll_keytype_datatype_t dt2, LockType locktype2,
            upll_keytype_datatype_t dt3, LockType locktype3);
  bool Unlock(upll_keytype_datatype_t dt1, upll_keytype_datatype_t dt2,
              upll_keytype_datatype_t dt3);
  bool Lock(upll_keytype_datatype_t dt1, LockType locktype1,
            upll_keytype_datatype_t dt2, LockType locktype2,
            upll_keytype_datatype_t dt3, LockType locktype3,
            upll_keytype_datatype_t dt4, LockType locktype4,
            upll_keytype_datatype_t dt5 = UPLL_DT_INVALID,
            LockType locktype5 = CFG_READ_LOCK);
  bool Unlock(upll_keytype_datatype_t dt1, upll_keytype_datatype_t dt2,
              upll_keytype_datatype_t dt3, upll_keytype_datatype_t dt4,
              upll_keytype_datatype_t dt5);

 private:
  pfc::core::ReadWriteLock candidate_cfg_lock_;
  pfc::core::ReadWriteLock running_cfg_lock_;
  pfc::core::ReadWriteLock startup_cfg_lock_;
  pfc::core::ReadWriteLock import_cfg_lock_;
  pfc::core::ReadWriteLock audit_cfg_lock_;
};

class ScopedConfigLock {
 public:
  ScopedConfigLock(ConfigLock &clk, upll_keytype_datatype_t dt,        // NOLINT
                   ConfigLock::LockType locktype) : clk_(clk) {
    num_locks_ = 1;
    dt1_ = dt;
    dt2_ = UPLL_DT_INVALID;
    dt3_ = UPLL_DT_INVALID;
    dt4_ = UPLL_DT_INVALID;
    dt5_ = UPLL_DT_INVALID;
    clk_.Lock(dt, locktype);
  }
  ScopedConfigLock(ConfigLock &clk,                                    // NOLINT
                   upll_keytype_datatype_t dt1,
                   ConfigLock::LockType locktype1,
                   upll_keytype_datatype_t dt2,
                   ConfigLock::LockType locktype2) : clk_(clk) {
    num_locks_ = 2;
    dt1_ = dt1;
    dt2_ = dt2;
    dt3_ = UPLL_DT_INVALID;
    dt4_ = UPLL_DT_INVALID;
    dt5_ = UPLL_DT_INVALID;
    clk_.Lock(dt1, locktype1, dt2, locktype2);
  }
  ScopedConfigLock(ConfigLock &clk,                                    // NOLINT
                   upll_keytype_datatype_t dt1,
                   ConfigLock::LockType locktype1,
                   upll_keytype_datatype_t dt2,
                   ConfigLock::LockType locktype2,
                   upll_keytype_datatype_t dt3,
                   ConfigLock::LockType locktype3) : clk_(clk) {
    num_locks_ = 3;
    dt1_ = dt1;
    dt2_ = dt2;
    dt3_ = dt3;
    dt4_ = UPLL_DT_INVALID;
    dt5_ = UPLL_DT_INVALID;
    clk_.Lock(dt1, locktype1, dt2, locktype2, dt3, locktype3);
  }
  ScopedConfigLock(ConfigLock &clk,                                    // NOLINT
                   upll_keytype_datatype_t dt1,
                   ConfigLock::LockType locktype1,
                   upll_keytype_datatype_t dt2,
                   ConfigLock::LockType locktype2,
                   upll_keytype_datatype_t dt3,
                   ConfigLock::LockType locktype3,
                   upll_keytype_datatype_t dt4,
                   ConfigLock::LockType locktype4,
                   upll_keytype_datatype_t dt5 = UPLL_DT_INVALID,
                   ConfigLock::LockType locktype5 = ConfigLock::CFG_READ_LOCK)
      : clk_(clk) {
    num_locks_ = 5;
    dt1_ = dt1;
    dt2_ = dt2;
    dt3_ = dt3;
    dt4_ = dt4;
    dt5_ = dt5;
    clk_.Lock(dt1, locktype1, dt2, locktype2, dt3, locktype3,
              dt4, locktype4, dt5, locktype5);
  }
  ~ScopedConfigLock() {
    if (num_locks_ == 1) {
      clk_.Unlock(dt1_);
    } else if (num_locks_ == 2) {
      clk_.Unlock(dt1_, dt2_);
    } else if (num_locks_ == 3) {
      clk_.Unlock(dt1_, dt2_, dt3_);
    } else if (num_locks_ == 5) {
      clk_.Unlock(dt1_, dt2_, dt3_, dt4_, dt5_);
    }
  }

 private:
  uint32_t num_locks_;
  ConfigLock &clk_;
  upll_keytype_datatype_t dt1_, dt2_, dt3_, dt4_, dt5_;
};

}  // namespace config_momgr
}  // namespace upll
}  // namespace unc

#endif  // UPLL_CONFIG_LOCK_HH_
