/*
 * Copyright (c) 2012-2014 NEC Corporation
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

bool ConfigLock::Lock(upll_keytype_datatype_t dt, LockType locktype) {
  if (locktype == CFG_READ_LOCK)
    return ReadLock(dt);
  else if (locktype == CFG_WRITE_LOCK)
    return WriteLock(dt);
  return false;
}

bool ConfigLock::Unlock(upll_keytype_datatype_t dt) {
  switch (dt) {
    case UPLL_DT_CANDIDATE:
      candidate_cfg_lock_.unlock();
      break;
    case UPLL_DT_RUNNING:
      running_cfg_lock_.unlock();
      break;
    case UPLL_DT_STARTUP:
      startup_cfg_lock_.unlock();
      break;
    case UPLL_DT_IMPORT:
      import_cfg_lock_.unlock();
      break;
    case UPLL_DT_AUDIT:
      audit_cfg_lock_.unlock();
      break;
    default:
      return false;
  }
  return true;
}

bool ConfigLock::ReadLock(upll_keytype_datatype_t dt) {
  switch (dt) {
    case UPLL_DT_CANDIDATE:
      candidate_cfg_lock_.rdlock();
      break;
    case UPLL_DT_RUNNING:
      running_cfg_lock_.rdlock();
      break;
    case UPLL_DT_STARTUP:
      startup_cfg_lock_.rdlock();
      break;
    case UPLL_DT_IMPORT:
      import_cfg_lock_.rdlock();
      break;
    case UPLL_DT_AUDIT:
      audit_cfg_lock_.rdlock();
      break;
    default:
      return false;
  }
  return true;
}

bool ConfigLock::WriteLock(upll_keytype_datatype_t dt) {
  switch (dt) {
    case UPLL_DT_CANDIDATE:
      candidate_cfg_lock_.wrlock();
      break;
    case UPLL_DT_RUNNING:
      running_cfg_lock_.wrlock();
      break;
    case UPLL_DT_STARTUP:
      startup_cfg_lock_.wrlock();
      break;
    case UPLL_DT_IMPORT:
      import_cfg_lock_.wrlock();
      break;
    case UPLL_DT_AUDIT:
      audit_cfg_lock_.wrlock();
      break;
    default:
      return false;
  }
  return true;
}

bool ConfigLock::Lock(upll_keytype_datatype_t dt1, LockType locktype1,
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
  if (Lock(dt_lower, lt_lower))
    return Lock(dt_higher, lt_higher);

  return false;
}

bool ConfigLock::Unlock(upll_keytype_datatype_t dt1,
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
  bool ret1 = Unlock(dt_higher);
  bool ret2 = Unlock(dt_lower);

  return (ret1 && ret2);
}


bool ConfigLock::Lock(upll_keytype_datatype_t dt1, LockType locktype1,
                      upll_keytype_datatype_t dt2, LockType locktype2,
                      upll_keytype_datatype_t dt3, LockType locktype3) {
  const upll_keytype_datatype_t min_dt = MIN(dt1, MIN(dt2, dt3));
  if (min_dt == dt1) {
    if (Lock(dt1, locktype1)) {
      return Lock(dt2, locktype2, dt3, locktype3);
    }
  } else if (min_dt == dt2) {
    if (Lock(dt2, locktype2)) {
      return Lock(dt1, locktype1, dt3, locktype3);
    }
  } else {
    if (Lock(dt3, locktype3)) {
      return Lock(dt1, locktype1, dt2, locktype2);
    }
  }
  return false;
}

bool ConfigLock::Unlock(upll_keytype_datatype_t dt1,
                        upll_keytype_datatype_t dt2,
                        upll_keytype_datatype_t dt3) {
  const upll_keytype_datatype_t max_dt = MAX(dt1, MAX(dt2, dt3));
  if (max_dt == dt1) {
    if (Unlock(dt1)) {
      return Unlock(dt2, dt3);
    }
  } else if (max_dt == dt2) {
    if (Unlock(dt2)) {
      return Unlock(dt1, dt3);
    }
  } else {
    if (Unlock(dt3)) {
      return Unlock(dt1, dt2);
    }
  }
  return false;
}

bool ConfigLock::Lock(upll_keytype_datatype_t dt1, LockType locktype1,
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
  std::map<upll_keytype_datatype_t, LockType>::const_iterator dtlock_itr;
  for ( dtlock_itr = req.begin(); dtlock_itr != req.end(); ++dtlock_itr) {
    if (UPLL_DT_INVALID != dtlock_itr->first) {
      if (!Lock(dtlock_itr->first, dtlock_itr->second)) {
        return false;
      }
    }
  }
  return true;
}


bool ConfigLock::Unlock(upll_keytype_datatype_t dt1,
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
  std::map<upll_keytype_datatype_t, LockType>::const_reverse_iterator  dtlock_itr;
  for (dtlock_itr = req.rbegin(); dtlock_itr != req.rend(); ++dtlock_itr) {
    if (UPLL_DT_INVALID != dtlock_itr->first) {
      if (!Unlock(dtlock_itr->first)) {
        return false;
      }
    }
  }
  return true;
}

}  // namespace config_momgr
}  // namespace upll
}  // namespace unc
