/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <sys/param.h>

#include "config_lock.hh"

namespace unc {
namespace upll {
namespace config_momgr {

bool ConfigLock::Lock(TaskPriority task_priority,
                      upll_keytype_datatype_t dt,
                      LockType locktype) {
  if (task_priority == kNormalTaskPriority) {
    LockYield(dt);
  }
  return LockP(task_priority, dt, locktype);
}

bool ConfigLock::LockP(TaskPriority task_priority,
                           upll_keytype_datatype_t dt,
                           LockType locktype) {
  if (locktype == CFG_READ_LOCK) {
    return ReadLock(task_priority, dt);
  } else if (locktype == CFG_WRITE_LOCK) {
    return WriteLock(task_priority, dt);
  }
  return false;
}

bool ConfigLock::Unlock(TaskPriority task_priority,
                        upll_keytype_datatype_t dt) {
  switch (dt) {
    case UPLL_DT_CANDIDATE:
      if (task_priority == kCriticalTaskPriority) {
        candidate_cfg_lock_.DecrementRefCnt();
      }
      candidate_cfg_lock_.cfg_lock.unlock();
      break;
    case UPLL_DT_RUNNING:
      if (task_priority == kCriticalTaskPriority) {
        running_cfg_lock_.DecrementRefCnt();
      }
      running_cfg_lock_.cfg_lock.unlock();
      break;
    case UPLL_DT_STARTUP:
      if (task_priority == kCriticalTaskPriority) {
        startup_cfg_lock_.DecrementRefCnt();
      }
      startup_cfg_lock_.cfg_lock.unlock();
      break;
    case UPLL_DT_IMPORT:
      if (task_priority == kCriticalTaskPriority) {
        import_cfg_lock_.DecrementRefCnt();
      }
      import_cfg_lock_.cfg_lock.unlock();
      break;
    case UPLL_DT_AUDIT:
      if (task_priority == kCriticalTaskPriority) {
        audit_cfg_lock_.DecrementRefCnt();
      }
      audit_cfg_lock_.cfg_lock.unlock();
      break;
    default:
      return false;
  }
  return true;
}

bool ConfigLock::ReadLock(TaskPriority task_priority,
                          upll_keytype_datatype_t dt) {
  // refcnt should be incremented before taking critical lock
  switch (dt) {
    case UPLL_DT_CANDIDATE:
      if (task_priority == kCriticalTaskPriority) {
        candidate_cfg_lock_.IncrementRefCnt(CFG_READ_LOCK);
      }
      candidate_cfg_lock_.cfg_lock.rdlock();
      break;
    case UPLL_DT_RUNNING:
      if (task_priority == kCriticalTaskPriority) {
        running_cfg_lock_.IncrementRefCnt(CFG_READ_LOCK);
      }
      running_cfg_lock_.cfg_lock.rdlock();
      break;
    case UPLL_DT_STARTUP:
      if (task_priority == kCriticalTaskPriority) {
        startup_cfg_lock_.IncrementRefCnt(CFG_READ_LOCK);
      }
      startup_cfg_lock_.cfg_lock.rdlock();
      break;
    case UPLL_DT_IMPORT:
      if (task_priority == kCriticalTaskPriority) {
        import_cfg_lock_.IncrementRefCnt(CFG_READ_LOCK);
      }
      import_cfg_lock_.cfg_lock.rdlock();
      break;
    case UPLL_DT_AUDIT:
      if (task_priority == kCriticalTaskPriority) {
        audit_cfg_lock_.IncrementRefCnt(CFG_READ_LOCK);
      }
      audit_cfg_lock_.cfg_lock.rdlock();
      break;
    default:
      return false;
  }
  return true;
}

bool ConfigLock::WriteLock(TaskPriority task_priority,
                           upll_keytype_datatype_t dt) {
  // refcnt should be incremented before taking critical lock so that normal
  // tasks will not try to take lock
  switch (dt) {
    case UPLL_DT_CANDIDATE:
      if (task_priority == kCriticalTaskPriority) {
        candidate_cfg_lock_.IncrementRefCnt(CFG_WRITE_LOCK);
      }
      candidate_cfg_lock_.cfg_lock.wrlock();
      break;
    case UPLL_DT_RUNNING:
      if (task_priority == kCriticalTaskPriority) {
        running_cfg_lock_.IncrementRefCnt(CFG_WRITE_LOCK);
      }
      running_cfg_lock_.cfg_lock.wrlock();
      break;
    case UPLL_DT_STARTUP:
      if (task_priority == kCriticalTaskPriority) {
        startup_cfg_lock_.IncrementRefCnt(CFG_WRITE_LOCK);
      }
      startup_cfg_lock_.cfg_lock.wrlock();
      break;
    case UPLL_DT_IMPORT:
      if (task_priority == kCriticalTaskPriority) {
        import_cfg_lock_.IncrementRefCnt(CFG_WRITE_LOCK);
      }
      import_cfg_lock_.cfg_lock.wrlock();
      break;
    case UPLL_DT_AUDIT:
      if (task_priority == kCriticalTaskPriority) {
        audit_cfg_lock_.IncrementRefCnt(CFG_WRITE_LOCK);
      }
      audit_cfg_lock_.cfg_lock.wrlock();
      break;
    default:
      return false;
  }
  return true;
}

bool ConfigLock::Lock(TaskPriority task_priority,
                      upll_keytype_datatype_t dt1, LockType locktype1,
                      upll_keytype_datatype_t dt2, LockType locktype2) {
  if (task_priority == kNormalTaskPriority) {
    LockYield(dt1, dt2);
  }
  return LockP(task_priority, dt1, locktype1, dt2, locktype2);
}

bool ConfigLock::LockP(TaskPriority task_priority,
                           upll_keytype_datatype_t dt1, LockType locktype1,
                           upll_keytype_datatype_t dt2, LockType locktype2) {
  // To avoid deadlocks, we need to always get lock in the same order between
  // all the datatypes. We use numerical number of upll_keytype_datatype_t to
  // sort the locks.

  if (dt1 == dt2)
    return false;

  upll_keytype_datatype_t dt_lower, dt_higher;
  LockType lt_lower, lt_higher;

  if (dt1 < dt2) {
    dt_lower = dt1;
    lt_lower = locktype1;
    dt_higher = dt2;
    lt_higher = locktype2;
  } else {
    dt_lower = dt2;
    lt_lower = locktype2;
    dt_higher = dt1;
    lt_higher = locktype1;
  }

  if (LockP(task_priority, dt_lower, lt_lower))
    return LockP(task_priority, dt_higher, lt_higher);

  return false;
}

bool ConfigLock::Unlock(TaskPriority task_priority,
                        upll_keytype_datatype_t dt1,
                        upll_keytype_datatype_t dt2) {
  // To avoid deadlocks, we need to always get lock in the same order between
  // all the datatypes. We use numerical number of upll_keytype_datatype_t to
  // sort the datatypes.

  if (dt1 == dt2)
    return false;

  upll_keytype_datatype_t dt_lower, dt_higher;

  if (dt1 < dt2) {
    dt_lower = dt1;
    dt_higher = dt2;
  } else {
    dt_lower = dt2;
    dt_higher = dt1;
  }
  bool ret1 = Unlock(task_priority, dt_higher);
  bool ret2 = Unlock(task_priority, dt_lower);

  return (ret1 && ret2);
}


bool ConfigLock::Lock(TaskPriority task_priority,
                      upll_keytype_datatype_t dt1, LockType locktype1,
                      upll_keytype_datatype_t dt2, LockType locktype2,
                      upll_keytype_datatype_t dt3, LockType locktype3) {
  const upll_keytype_datatype_t min_dt = MIN(dt1, MIN(dt2, dt3));

  if (task_priority == kNormalTaskPriority) {
    LockYield(dt1, dt2, dt3);
  }

  if (min_dt == dt1) {
    if (LockP(task_priority, dt1, locktype1)) {
      return LockP(task_priority, dt2, locktype2, dt3, locktype3);
    }
  } else if (min_dt == dt2) {
    if (LockP(task_priority, dt2, locktype2)) {
      return LockP(task_priority, dt1, locktype1, dt3, locktype3);
    }
  } else {
    if (LockP(task_priority, dt3, locktype3)) {
      return LockP(task_priority, dt1, locktype1, dt2, locktype2);
    }
  }
  return false;
}

bool ConfigLock::Unlock(TaskPriority task_priority,
                        upll_keytype_datatype_t dt1,
                        upll_keytype_datatype_t dt2,
                        upll_keytype_datatype_t dt3) {
  const upll_keytype_datatype_t max_dt = MAX(dt1, MAX(dt2, dt3));
  if (max_dt == dt1) {
    if (Unlock(task_priority, dt1)) {
      return Unlock(task_priority, dt2, dt3);
    }
  } else if (max_dt == dt2) {
    if (Unlock(task_priority, dt2)) {
      return Unlock(task_priority, dt1, dt3);
    }
  } else {
    if (Unlock(task_priority, dt3)) {
      return Unlock(task_priority, dt1, dt2);
    }
  }
  return false;
}

void ConfigLock::LockYield(upll_keytype_datatype_t dt1,
                           upll_keytype_datatype_t dt2,
                           upll_keytype_datatype_t dt3,
                           upll_keytype_datatype_t dt4,
                           upll_keytype_datatype_t dt5) {
  while (IsCriticalLockTaken(dt1, dt2, dt3, dt4, dt5)) {
    NanoSleep(1);
    // Optimisation: Take mutex cond here wait for signal which can
    // be sent at the end of critical lock
    /*
    Lock();
    Wait();
    Unlock();
    */
    continue;
  }
}

bool ConfigLock::Lock(TaskPriority task_priority,
                      upll_keytype_datatype_t dt1, LockType locktype1,
                      upll_keytype_datatype_t dt2, LockType locktype2,
                      upll_keytype_datatype_t dt3, LockType locktype3,
                      upll_keytype_datatype_t dt4, LockType locktype4,
                      upll_keytype_datatype_t dt5, LockType locktype5) {
  // map does the sorting
  std::map<upll_keytype_datatype_t, LockType> req;
  req[dt1] = locktype1;
  req[dt2] = locktype2;
  req[dt3] = locktype3;
  req[dt4] = locktype4;
  req[dt5] = locktype5;

  if (task_priority == kNormalTaskPriority) {
    LockYield(dt1, dt2, dt3, dt4, dt5);
  }
  std::map<upll_keytype_datatype_t, LockType>::const_iterator dtlock_itr;
  for (dtlock_itr = req.begin(); dtlock_itr != req.end(); ++dtlock_itr) {
    if (UPLL_DT_INVALID != dtlock_itr->first) {
      if (!LockP(task_priority, dtlock_itr->first, dtlock_itr->second)) {
        return false;
      }
    }
  }
  return true;
}


bool ConfigLock::Unlock(TaskPriority task_priority,
                        upll_keytype_datatype_t dt1,
                        upll_keytype_datatype_t dt2,
                        upll_keytype_datatype_t dt3,
                        upll_keytype_datatype_t dt4,
                        upll_keytype_datatype_t dt5) {
  // map does the sorting and Locktype values are dummy here.
  std::map<upll_keytype_datatype_t, LockType> req;
  req[dt1] = CFG_READ_LOCK;
  req[dt2] = CFG_READ_LOCK;
  req[dt3] = CFG_READ_LOCK;
  req[dt4] = CFG_READ_LOCK;
  req[dt5] = CFG_READ_LOCK;
  std::map<upll_keytype_datatype_t, LockType>::const_reverse_iterator
      dtlock_itr;
  for (dtlock_itr = req.rbegin(); dtlock_itr != req.rend(); ++dtlock_itr) {
    if (UPLL_DT_INVALID != dtlock_itr->first) {
      if (!Unlock(task_priority, dtlock_itr->first)) {
        return false;
      }
    }
  }
  return true;
}

bool ConfigLock::IsCriticalLockTakenOnDt(upll_keytype_datatype_t dt) {
  switch (dt) {
    case UPLL_DT_CANDIDATE:
      return candidate_cfg_lock_.CriticalTaskExists();
      break;
    case UPLL_DT_RUNNING:
      return running_cfg_lock_.CriticalTaskExists();
      break;
    case UPLL_DT_STARTUP:
      return startup_cfg_lock_.CriticalTaskExists();
      break;
    case UPLL_DT_IMPORT:
      return import_cfg_lock_.CriticalTaskExists();
      break;
    case UPLL_DT_AUDIT:
      return audit_cfg_lock_.CriticalTaskExists();
      break;
    default:
      return false;
  }
  return false;
}

bool ConfigLock::IsCriticalLockTaken(upll_keytype_datatype_t dt1,
                                     upll_keytype_datatype_t dt2,
                                     upll_keytype_datatype_t dt3,
                                     upll_keytype_datatype_t dt4,
                                     upll_keytype_datatype_t dt5) {
  return (IsCriticalLockTakenOnDt(dt1) ||
          (dt2 != UPLL_DT_INVALID && IsCriticalLockTakenOnDt(dt2)) ||
          (dt3 != UPLL_DT_INVALID && IsCriticalLockTakenOnDt(dt3)) ||
          (dt4 != UPLL_DT_INVALID && IsCriticalLockTakenOnDt(dt4)) ||
          (dt5 != UPLL_DT_INVALID && IsCriticalLockTakenOnDt(dt5)));
}

}  // namespace config_momgr
}  // namespace upll
}  // namespace unc
