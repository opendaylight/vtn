/*
 * Copyright (c) 2012-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef   __UNC_TC_DATA_HH__
#define   __UNC_TC_DATA_HH__

#include <pfcxx/synch.hh>
#include <pfc/debug.h>
#include<set>
#include <map>
#include <string>

namespace unc {
namespace tc {

const std::string tc_conf_block="tc_db_params";

const std::string tc_conf_db_dsn_name_param="tc_db_dsn_name";
const std::string tc_def_db_dsn_name_value="UNC_DB_DSN";

const std::string max_failover_instance_param="max_failover_instance";
const uint32_t max_failover_instance_value = 4;

const std::string
       simultaneous_read_write_allowed_param="simultaneous_read_write_allowed";
const pfc_bool_t simultaneous_read_write_allowed_value = PFC_TRUE;

const uint32_t TC_READ_CONCURRENCY = 1;
const uint32_t TC_AUDIT_CONCURRENCY = 1;


typedef enum {
  TC_INIT = 1,
  TC_ACT,
  TC_SBY,
  TC_STOP,
  TC_ACT_FAIL
}TcState;

/* TC Main Operations  */
typedef enum {
  TC_OPERATION_NONE = 0,
  TC_ACQUIRE_CONFIG_SESSION = 1,
  TC_RELEASE_CONFIG_SESSION,
  TC_FORCE_ACQUIRE_CONFIG_SESSION,
  TC_ACQUIRE_READ_SESSION,
  TC_RELEASE_READ_SESSION,
  TC_ACQUIRE_WRITE_SESSION,
  TC_RELEASE_WRITE_SESSION,
  TC_AUTO_SAVE_ENABLE,
  TC_AUTO_SAVE_DISABLE,
  TC_AUTO_SAVE_GET,
  TC_ACQUIRE_READ_LOCK_FOR_STATE_TRANSITION,
  TC_RELEASE_READ_LOCK_FOR_STATE_TRANSITION
}TcOperation;

/* write lock  operations  */
typedef enum {
  TC_WRITE_NONE = 0,
  TC_AUDIT_USER = 12,
  TC_AUDIT_DRIVER,
  TC_COMMIT,
  TC_ABORT_CANDIDATE_CONFIG,
  TC_CLEAR_STARTUP_CONFIG,
  TC_SAVE_STARTUP_CONFIG
}TcWriteOperation;

typedef enum {
  TC_NOTIFY_NONE = 0,
  TC_NOTIFY_ACQUIRE = 1,
  TC_NOTIFY_RELEASE
}TcNotifyOperation;

typedef enum {
  TC_NO_OPERATION_PROGRESS = 1,
  TC_CONFIG_COMMIT_PROGRESS,
  TC_WRITE_AUDIT_USER_PROGRESS,
  TC_WRITE_AUDIT_DRIVER_PROGRESS,
  TC_CONFIG_NOTIFY_ACQUIRE_PROGRESS,
  TC_CONFIG_NOTIFY_RELEASE_PROGRESS,
  TC_CONFIG_NO_NOTIFY_PROGRESS,
  TC_READ_PROGRESS,
  TC_WRITE_ABORT_CANDIDATE_CONFIG_PROGRESS,
  TC_WRITE_SAVE_STARTUP_CONFIG_PROGRESS,
  TC_WRITE_CLEAR_STARTUP_CONFIG_PROGRESS,
  TC_INVALID_OPERATION_PROGRESS
}TcSessionOperationProgress;

/* Configuration session data */
struct TcConfigLock {
  uint32_t config_id;
  uint32_t session_id;
  uint32_t marked_session_id;
  pfc_bool_t is_taken;
  pfc_bool_t is_notify_pending;
  TcNotifyOperation notify_operation;
TcConfigLock()
: config_id(0), session_id(0), marked_session_id(0), is_taken(PFC_FALSE),
    is_notify_pending(PFC_FALSE), notify_operation(TC_NOTIFY_NONE)
{}
};


/* Read sessions data */
struct TcReadLock {
  std::set<uint32_t> read_sessions;
};

/* Write/commit/audit  session data */
class TcLock;

/*
 * Read/Write lock instance.
 */
class TcReadWriteLock  {
  friend class ::unc::tc::TcLock;

  // Note that all TcReadWriteLock methods must be called with holding
  // rw_mutex.
  pfc::core::Mutex     rw_mutex;      // Global mutex lock.
  pfc::core::Condition rw_cond;       // Condition variable.

  // If read lock is held, rw_owner keeps the number of read lock owners.
  // rw_owner keeps RW_OWNER_WRLOCKED if write lock is held.
  uint32_t             rw_owner;

  // For simultaneous read and write lock, separate locks are maintained
  // for read and write.
  uint32_t             r_owner;
  pfc_bool_t           r_db_mgmt_session;
  pfc_bool_t           r_launcher_session;
  uint32_t             w_owner;

  uint32_t             r_waiter;      // Number of threads blocked on read lock.

  uint32_t             w_session_id;  // Owner of write lock.
  uint32_t             w_waiter;    // Number of threads blocked on write lock.
  TcWriteOperation     w_operation;   // Write lock operation.

  // SessionID of DB_MGMT and LAUNCHER for whose READ request
  // will be allowed only if there is no WRITE lock
  uint32_t             db_mgmt_session_id_;
  uint32_t             launcher_session_id_;

  // tc.conf param: simultaneous_read_write_allowed is set or not?
  pfc_bool_t           is_simultaneous_read_write_allowed_;

  // Maximum number of read lock owners.
  static const uint32_t R_READERS_MAX = UINT32_MAX - 1;

  // r_readers value which indicates write lock is held.
  static const uint32_t RW_OWNER_WRLOCKED = UINT32_MAX;

  TcReadWriteLock()
      : rw_owner(0), r_owner(0), r_db_mgmt_session(PFC_FALSE),
        r_launcher_session(PFC_FALSE), w_owner(0), r_waiter(0),
        w_session_id(0), w_waiter(0), w_operation(TC_WRITE_NONE),
        db_mgmt_session_id_(0), launcher_session_id_(0),
        is_simultaneous_read_write_allowed_(PFC_TRUE)
  {}

  inline void
      init(uint32_t db_mgmt_sess_id,
           uint32_t launcher_sess_id,
           pfc_bool_t is_allowed) {
        db_mgmt_session_id_ = db_mgmt_sess_id;
        launcher_session_id_ = launcher_sess_id;
        is_simultaneous_read_write_allowed_ = is_allowed;
      }

  inline bool
      isReadAlwaysSucess(uint32_t session_id) {
        // If request from user session and
        // Simultaneous read/write lock allowed?
        if (session_id != db_mgmt_session_id_ &&
            session_id != launcher_session_id_ &&
            is_simultaneous_read_write_allowed_ != PFC_FALSE) {
          return PFC_TRUE;
        }
        return PFC_FALSE;
      }

  /**
   * @brief  Determine whether at least one thread is blocked on this lock.
   * @return true  At least one thread is blocked on this lock.
   * @return false No thread is blocked.
   */
  inline bool
      isWaiting(void)  {
        return (r_waiter != 0 || w_waiter != 0);
      }

  /**
   * @brief  Wake up all threads blocked on this lock.
   */
  inline void
      wakeUp(void)  {
        if (isWaiting()) {
          rw_cond.broadcast();
        }
      }

  /**
   * @brief  Block the calling thread to acquire read lock.
   */
  inline void
      waitForRead()  {
        r_waiter++;
        rw_cond.wait(rw_mutex);
        r_waiter--;
      }
  /**
   * @brief  Block the calling thread to acquire write lock.
   */
  inline void
      waitForWrite()  {
        w_waiter++;
        rw_cond.wait(rw_mutex);
        w_waiter--;
      }

  /**
   * @brief  Determine whether the read lock can be held or not.
   * @return true  The read lock can be held.
   * @return false The read lock can not be held.
   */
  inline bool
      canReadLock(uint32_t session_id)  {
        if (is_simultaneous_read_write_allowed_) {
          if (session_id == db_mgmt_session_id_ ||
              session_id == launcher_session_id_) {
            // If Read request from mgmt_db or launcher,
            // if write is in-progress or waiting dont allow
            return (w_owner != RW_OWNER_WRLOCKED && w_waiter == 0);
          } else {
            // Always return true for user sessions
            return PFC_TRUE;
          }
        } else {
          return (rw_owner != RW_OWNER_WRLOCKED && w_waiter == 0);
        }
      }

  /**
   * @brief  Set read lock to the lock.
   */
  inline void
      setReader(uint32_t session_id)  {
        PFC_ASSERT(canReadLock(session_id));
        if (is_simultaneous_read_write_allowed_) {
          r_owner++;
          // If read req from db_mgmt or launcher, store it.
          if (session_id == db_mgmt_session_id_) {
            r_db_mgmt_session = PFC_TRUE;
          } else if (session_id == launcher_session_id_) {
            r_launcher_session = PFC_TRUE;
          }
          PFC_ASSERT(r_owner > 0 && r_owner <= R_READERS_MAX);
        } else {
          rw_owner++;
          PFC_ASSERT(rw_owner > 0 && rw_owner <= R_READERS_MAX);
        }
      }

  /**
   * @brief  Determine whether the write lock can be held or not.
   * @return true  The write lock can be held.
   * @return false The write lock can not be held.
   */
  inline bool
      canWriteLock(void)  {
        if (is_simultaneous_read_write_allowed_) {
          return (r_db_mgmt_session == PFC_FALSE &&
                  r_launcher_session == PFC_FALSE &&
                  w_owner == 0);
        } else {
          return (rw_owner == 0);
        }
      }

  /**
   * @brief  Clear information about the write lock session.
   */
  inline void
      clearWriter(void)  {
        w_session_id = 0;
        w_operation = TC_WRITE_NONE;
      }

  /**
   * @brief  Set information about the write lock session.
   * @param[in] session_id  The session identifier.
   * @param[in] operation   The write lock operation.
   */
  inline void
      setWriter(uint32_t session_id, TcWriteOperation operation)  {
        PFC_ASSERT(canWriteLock());
        if (is_simultaneous_read_write_allowed_) {
          w_owner = RW_OWNER_WRLOCKED;
        } else {
          rw_owner = RW_OWNER_WRLOCKED;
        }
        w_session_id = session_id;
        w_operation = operation;
      }

  /**
   * @brief  Determine whether the write lock is held or not.
   * @return true  The write lock is held.
   * @return false The write lock is not held.
   */
  inline bool
      isWriteHeld(void)  {
        if (is_simultaneous_read_write_allowed_) {
          return (w_owner == RW_OWNER_WRLOCKED);
        } else {
          return (rw_owner == RW_OWNER_WRLOCKED);
        }
      }

  /**
   * @brief  Determine whether the write lock is held for the given session.
   * @return true  The write lock is held.
   * @return false The write lock is not held.
   */
  inline bool
      isWriteHeld(uint32_t session_id)  {
        return (isWriteHeld() && w_session_id == session_id);
      }

  /**
   * @brief    Try to acquire the read lock.
   * @return   true   Operation succeeded.
   * @return   false  The write lock is held by another thread.
   */
  inline bool
      tryLock(uint32_t session_id)  {
        bool ret(canReadLock(session_id));
        if (ret) {
          setReader(session_id);
        }

        return ret;
      }

  /**
   * @brief    Try to acquire the write lock.
   * @param[in] session_id  The session identifier.
   * @param[in] operation   The write lock operation.
   * @return true   Operation succeeded.
   * @return false  The write lock is held by another thread.
   */
  inline bool
      tryLock(uint32_t session_id, TcWriteOperation operation)  {
        bool ret(canWriteLock());
        if (ret) {
          setWriter(session_id, operation);
        }

        return ret;
      }

  /**
   * @brief  Release the read lock.
   */
  inline void
      unlock(uint32_t session_id) {
        if (is_simultaneous_read_write_allowed_) {
          PFC_ASSERT(r_owner > 0 && r_owner <= R_READERS_MAX);

          if (session_id == db_mgmt_session_id_) {
            r_db_mgmt_session = PFC_FALSE;
          } else if (session_id == launcher_session_id_) {
            r_launcher_session = PFC_FALSE;
          }

          r_owner--;
          if (r_owner == 0) {
            // Wake up all blocked threads.
            wakeUp();
          }
        } else {
          PFC_ASSERT(rw_owner > 0 && rw_owner <= R_READERS_MAX);

          rw_owner--;
          if (rw_owner == 0) {
            // Wake up all blocked threads.
            wakeUp();
          }
        }
      }

  /**
   * @brief  Release the write lock.
   * @param[in] session_id  The session identifier.
   * @param[in] operation   The write lock operation.
   * @return true   Operation succeeded.
   * @return false  The write lock is not held for the given session.
   */
  inline bool
      unlock(uint32_t session_id, TcWriteOperation operation)  {
        if (PFC_EXPECT_FALSE(!isWriteHeld(session_id) ||
                             w_operation != operation)) {
          return false;
        }

        clearWriter();
        rw_owner = 0;
        w_owner = 0;

        // Wake up all blocked threads.
        wakeUp();

        return true;
      }
    };

  /* State data */
  struct TcStateLock {
    TcState current_state;
    pfc_bool_t state_transition_in_progress;
    TcStateLock()
         : current_state(TC_INIT), state_transition_in_progress(PFC_FALSE)
     {}
  };

  /* Auto save data */
  struct TcAutoSave {
    pfc_bool_t is_enable;

  TcAutoSave() : is_enable(PFC_FALSE) {}
  };

  enum TcDaemonName {
    TC_NONE = 0,
    TC_UPLL,
    TC_UPPL,
    TC_DRV_OPENFLOW,
    TC_DRV_OVERLAY,
    TC_DRV_POLC,
    TC_DRV_ODL
    //TC_DRV_LEGACY
  };

  typedef std::map<TcDaemonName, std::string> TcChannelNameMap;

}
} /* unc */

#endif  // __UNC_TC_LOCK_HH__
