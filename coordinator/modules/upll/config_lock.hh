/*
 * Copyright (c) 2012-2015 NEC Corporation
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
#include "task_sched.hh"

namespace unc {
namespace upll {
namespace config_momgr {

class ConfigLock {
 public:
  enum LockType {
    CFG_READ_LOCK = 1,  // Parallel read locks are allowed
    CFG_WRITE_LOCK = 2  // Write not allowed when read/write lock taken
  };

  class DataTypeLock {
  friend class ConfigLock;
   public:
    DataTypeLock(): rd_cnt(0), wr_cnt(0) {}
    virtual ~DataTypeLock() {}

    inline void IncrementRefCnt(LockType lock_type) {
      refcnt_mutex.lock();
      if (lock_type == CFG_READ_LOCK) {
        rd_cnt++;
      } else if (lock_type == CFG_WRITE_LOCK) {
        wr_cnt++;
      } else {
      }
      refcnt_mutex.unlock();
    }

    inline void DecrementRefCnt() {
      refcnt_mutex.lock();
      // During unlock(), lock_type(rd/wr) is unknown.
      // At a time, rd and wr lock cannot be taken together
      // so rd_cnt > 0 then wr_cnt=0 and vice versa
      if (rd_cnt > 0) {
        rd_cnt--;
      } else if (wr_cnt > 0) {
        wr_cnt--;
      }
      refcnt_mutex.unlock();
    }

    inline bool CriticalTaskExists() {
      bool ret_val;
      refcnt_mutex.lock();
      ret_val = (wr_cnt > 0);
      refcnt_mutex.unlock();
      return ret_val;
    }

   private:
    pfc::core::ReadWriteLock cfg_lock;
    uint8_t rd_cnt;  // priority read lock reference count
    uint8_t wr_cnt;  // priority write lock reference count
    pfc::core::Mutex refcnt_mutex;  // mutex to access rd_cnt/wr_cnt
  };

  ConfigLock() {}
  virtual ~ConfigLock() {}
  bool ReadLock(TaskPriority task_priority, upll_keytype_datatype_t dt);
  bool WriteLock(TaskPriority task_priority, upll_keytype_datatype_t dt);
  bool Lock(TaskPriority task_priority, upll_keytype_datatype_t dt,
            LockType locktype);
  bool Unlock(TaskPriority task_priority, upll_keytype_datatype_t dt);
  bool Lock(TaskPriority task_priority,
            upll_keytype_datatype_t dt1, LockType locktype1,
            upll_keytype_datatype_t dt2, LockType locktype2);
  bool Unlock(TaskPriority task_priority,
              upll_keytype_datatype_t dt1, upll_keytype_datatype_t dt2);
  bool Lock(TaskPriority task_priority,
            upll_keytype_datatype_t dt1, LockType locktype1,
            upll_keytype_datatype_t dt2, LockType locktype2,
            upll_keytype_datatype_t dt3, LockType locktype3);
  bool Unlock(TaskPriority task_priority,
              upll_keytype_datatype_t dt1,
              upll_keytype_datatype_t dt2,
              upll_keytype_datatype_t dt3);
  bool Lock(TaskPriority task_priority,
            upll_keytype_datatype_t dt1, LockType locktype1,
            upll_keytype_datatype_t dt2, LockType locktype2,
            upll_keytype_datatype_t dt3, LockType locktype3,
            upll_keytype_datatype_t dt4, LockType locktype4,
            upll_keytype_datatype_t dt5 = UPLL_DT_INVALID,
            LockType locktype5 = CFG_READ_LOCK);
  bool Unlock(TaskPriority task_priority,
              upll_keytype_datatype_t dt1, upll_keytype_datatype_t dt2,
              upll_keytype_datatype_t dt3, upll_keytype_datatype_t dt4,
              upll_keytype_datatype_t dt5);

  bool IsCriticalLockTakenOnDt(upll_keytype_datatype_t dt);
  bool IsCriticalLockTaken(upll_keytype_datatype_t dt1,
                           upll_keytype_datatype_t dt2 = UPLL_DT_INVALID,
                           upll_keytype_datatype_t dt3 = UPLL_DT_INVALID,
                           upll_keytype_datatype_t dt4 = UPLL_DT_INVALID,
                           upll_keytype_datatype_t dt5 = UPLL_DT_INVALID);

  void LockYield(upll_keytype_datatype_t dt1,
                 upll_keytype_datatype_t dt2 = UPLL_DT_INVALID,
                 upll_keytype_datatype_t dt3 = UPLL_DT_INVALID,
                 upll_keytype_datatype_t dt4 = UPLL_DT_INVALID,
                 upll_keytype_datatype_t dt5 = UPLL_DT_INVALID);

  // To yield from normwhen critical task
  inline static void NanoSleep(long nanosec) {  // NOLINT fedora compile
    struct timespec wait_time = {0, nanosec};
    nanosleep(&wait_time, NULL);
  }

 private:
  // LockP apis avoid IsCriticalLockTaken() check, when it is
  // already taken care in public Lock apis.
  bool LockP(TaskPriority task_priority,
             upll_keytype_datatype_t dt,
             LockType locktype);
  bool LockP(TaskPriority task_priority,
             upll_keytype_datatype_t dt1, LockType locktype1,
             upll_keytype_datatype_t dt2, LockType locktype2);

  DataTypeLock candidate_cfg_lock_;
  DataTypeLock running_cfg_lock_;
  DataTypeLock startup_cfg_lock_;
  DataTypeLock import_cfg_lock_;
  DataTypeLock audit_cfg_lock_;
};

class ScopedConfigLock {
 public:
  ScopedConfigLock(ConfigLock &clk, TaskPriority priority,  // NOLINT
                   upll_keytype_datatype_t dt,
                   ConfigLock::LockType locktype) : clk_(clk) {
    task_priority_ = priority;
    num_locks_ = 1;
    dt1_ = dt;
    dt2_ = UPLL_DT_INVALID;
    dt3_ = UPLL_DT_INVALID;
    dt4_ = UPLL_DT_INVALID;
    dt5_ = UPLL_DT_INVALID;
    clk_.Lock(priority, dt, locktype);
  }
  ScopedConfigLock(ConfigLock &clk, TaskPriority priority,  // NOLINT
                   upll_keytype_datatype_t dt1,
                   ConfigLock::LockType locktype1,
                   upll_keytype_datatype_t dt2,
                   ConfigLock::LockType locktype2) : clk_(clk) {
    task_priority_ = priority;
    num_locks_ = 2;
    dt1_ = dt1;
    dt2_ = dt2;
    dt3_ = UPLL_DT_INVALID;
    dt4_ = UPLL_DT_INVALID;
    dt5_ = UPLL_DT_INVALID;
    clk_.Lock(priority, dt1, locktype1, dt2, locktype2);
  }
  ScopedConfigLock(ConfigLock &clk, TaskPriority priority,  // NOLINT
                   upll_keytype_datatype_t dt1,
                   ConfigLock::LockType locktype1,
                   upll_keytype_datatype_t dt2,
                   ConfigLock::LockType locktype2,
                   upll_keytype_datatype_t dt3,
                   ConfigLock::LockType locktype3) : clk_(clk) {
    task_priority_ = priority;
    num_locks_ = 3;
    dt1_ = dt1;
    dt2_ = dt2;
    dt3_ = dt3;
    dt4_ = UPLL_DT_INVALID;
    dt5_ = UPLL_DT_INVALID;
    clk_.Lock(priority, dt1, locktype1, dt2, locktype2, dt3, locktype3);
  }
  ScopedConfigLock(ConfigLock &clk, TaskPriority priority,  // NOLINT
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
    task_priority_ = priority;
    num_locks_ = 5;
    dt1_ = dt1;
    dt2_ = dt2;
    dt3_ = dt3;
    dt4_ = dt4;
    dt5_ = dt5;
    clk_.Lock(priority, dt1, locktype1, dt2, locktype2, dt3, locktype3,
              dt4, locktype4, dt5, locktype5);
  }
  ~ScopedConfigLock() {
    if (num_locks_ == 1) {
      clk_.Unlock(task_priority_, dt1_);
    } else if (num_locks_ == 2) {
      clk_.Unlock(task_priority_, dt1_, dt2_);
    } else if (num_locks_ == 3) {
      clk_.Unlock(task_priority_, dt1_, dt2_, dt3_);
    } else if (num_locks_ == 5) {
      clk_.Unlock(task_priority_, dt1_, dt2_, dt3_, dt4_, dt5_);
    }
  }

 private:
  TaskPriority task_priority_;
  uint32_t num_locks_;
  ConfigLock &clk_;
  upll_keytype_datatype_t dt1_, dt2_, dt3_, dt4_, dt5_;
};

}  // namespace config_momgr
}  // namespace upll
}  // namespace unc

#endif  // UPLL_CONFIG_LOCK_HH_
