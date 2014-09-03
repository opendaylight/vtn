/*
 * Copyright (c) 2012-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef   __UNC_TC_LOCK_HH__
#define   __UNC_TC_LOCK_HH__

#include <pfc/log.h>
#include <pfcxx/synch.hh>
#include <tc_module_data.hh>
#include <algorithm>
#include <unc/usess_ipc.h>
#include <unistd.h>

namespace unc {
namespace tc {

#define TC_START_CONFIG_ID 1


/* Lock Return values */
typedef enum {
  TC_LOCK_SUCCESS = 1,
  TC_LOCK_NO_CONFIG_SESSION_EXIST,
  TC_LOCK_NOT_ACQUIRED,
  TC_LOCK_INVALID_CONFIG_ID,
  TC_LOCK_INVALID_OPERATION,
  TC_LOCK_INVALID_SESSION_ID,
  TC_LOCK_INVALID_UNC_STATE,
  TC_LOCK_ALREADY_ACQUIRED,
  TC_LOCK_BUSY,
  TC_LOCK_OPERATION_NOT_ALLOWED,
  TC_LOCK_INVALID_PARAMS,
  TC_LOCK_FAILURE
}TcLockRet;


/* TcLock Declarations  */

class TcLock {
  public:
  TcLock() {setup_complete_done_ = PFC_FALSE;}
  void ResetTcGlobalDataOnStateTransition(void);
  TcLockRet GetLock(uint32_t session_id, TcOperation operation,
                    TcWriteOperation write_operation);
  TcLockRet ReleaseLock(uint32_t session_id, uint32_t config_id,
                        TcOperation operation,
                        TcWriteOperation write_operation);
  TcLockRet AutoSaveEnable();
  TcLockRet AutoSaveDisable();
  TcLockRet GetConfigIdSessionId(uint32_t * session_id, uint32_t *config_id);
  TcLockRet NotifyConfigIdSessionIdDone(uint32_t config_id,
            uint32_t session_id, TcNotifyOperation config_notify_operation);
  TcSessionOperationProgress  GetSessionOperation(uint32_t session_id);
  TcLockRet  TcAcquireReadLockForStateTransition(uint32_t session_id);
  TcLockRet  TcReleaseReadLockForStateTransition(uint32_t session_id);
  void       TcUpdateUncState(TcState state);
  TcState    GetUncCurrentState(void);
  pfc_bool_t IsStateTransitionInProgress(void);
  TcLockRet  TcMarkSessionId(uint32_t session_id);
  uint32_t   GetMarkedSessionId();
  void       TcInitWRLock(pfc_bool_t is_simultaneous_read_write_allowed);

  void       TcSetSetupComplete(pfc_bool_t is_done);
  pfc_bool_t TcIsSetupCompleteDone();

  private:
  // Prohibit copy constuction and copy assignment.
  TcLock(const TcLock&);
  TcLock& operator=(const TcLock&);

  /* Config data */
  TcConfigLock tc_config_lock_;
  /* Read data */
  TcReadLock tc_read_lock_;
  /* Write data */
  TcReadWriteLock tc_rwlock_;
  /* save data */
  TcAutoSave tc_auto_save_;
  /* State data */
  TcStateLock tc_state_lock_;
  inline pfc::core::Mutex &
  getGlobalLock(void)  {
    return tc_rwlock_.rw_mutex;
  }

  pfc_bool_t setup_complete_done_;
  pfc::core::Mutex setup_complete_flag_lock_;

  /* Config lock methods */
  TcLockRet AcquireConfigLock(uint32_t session_id);
  uint32_t GetConfigId(uint32_t config_id);
  TcLockRet ReleaseConfigLock(uint32_t session_id, uint32_t config_id);
  TcLockRet ForceAcquireConfigLock(uint32_t session_id);
  void  UpdateConfigData(uint32_t session_id);
  /* Read lock methods */
  TcLockRet AcquireReadLock(uint32_t session_id);
  TcLockRet ReleaseReadLock(uint32_t session_id);
  /* Write lock methods */
  TcLockRet AcquireWriteLock(uint32_t session_id, TcWriteOperation operation);
  TcLockRet AcquireWriteLockForCommit(uint32_t session_id);
  TcLockRet AcquireWriteLockForAuditUser(uint32_t session_id);
  TcLockRet AcquireWriteLockForAuditDriver(uint32_t session_id);
  TcLockRet AcquireWriteLockForSaveStartupConfig(uint32_t session_id);
  TcLockRet AcquireWriteLockForClearStartupConfig(uint32_t session_id);
  TcLockRet AcquireWriteLockForAbortCandidateConfig(uint32_t session_id);
  TcLockRet ReleaseWriteLock(uint32_t session_id,
                             TcWriteOperation write_operation);
  /* State based methods */
  TcLockRet TcOperationIsAllowed(uint32_t session_id, TcOperation operation);
};
}
} /* unc */

#endif  // __UNC_TC_LOCK_HH__
