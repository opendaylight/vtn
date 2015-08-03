/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <tc_lock.hh>
#include <unistd.h>
#include <sys/time.h>
#include <tc_operations.hh>

namespace unc {
namespace tc {

uint32_t TcLock::prev_config_id_ = 0;

/**
 *@brief   Reset the TC global data structure .
 */
void TcLock::ResetTcGlobalDataOnStateTransition(void)  {
  pfc_log_info("%s (Re)Initializing attributes", __FUNCTION__);
  pfc::core::ScopedMutex m(getGlobalLock());

  /* Clearing TC Config Name Map */
  tc_config_name_map_.clear();

  /* Initialize read lock information */
  std::set<uint32_t> &sessions(tc_read_lock_.read_sessions);
  //if (tc_state_lock_.current_state != TC_STOP) {
  //  PFC_VERIFY(tc_rwlock_.rw_owner == sessions.size() + 1);
  //}
  sessions.clear();
  tc_rwlock_.rw_owner = 1;
  /* Initialize write lock information */
  tc_rwlock_.clearWriter();
  /* Initialize last write operation */
  tc_rwlock_.last_write_op = TC_WRITE_NONE;
  /* Initialize auto save data */
  tc_auto_save_.is_enable = PFC_FALSE;

  /* Initialize setup_complete_done_ flag */
  setup_complete_done_ = PFC_FALSE;
}

/**
 *@brief   Get UNC current state.
 *@param[in]  none.
 *@return     state    current state of UNC.
 */
TcState  TcLock::GetUncCurrentState(void) {
  pfc::core::ScopedMutex m(getGlobalLock());
  return tc_state_lock_.current_state;
}

/**
 *@brief   Get state transition in progress status.
 *@return  state_transition_status  PFC_TRUE/PFC_FALSE.
 */
pfc_bool_t   TcLock::IsStateTransitionInProgress(void) {
  pfc::core::ScopedMutex m(getGlobalLock());
  return tc_state_lock_.state_transition_in_progress;
}

/*
 *@brief      Find tc_config_name keyvalue from map based on session_id.
 *@param[in]  session_id  
 *@return     tc_config_name, which is associated to input session_id_.
 */
TcLockRet TcLock::FindConfigName(uint32_t session_id,
                                 std::string& config_name) {
  config_name.clear();
  TcConfigNameMap::const_iterator cit;
  for(cit=tc_config_name_map_.begin(); cit!=tc_config_name_map_.end(); cit++) {
    if ((cit->second).session_id == session_id) {
      config_name = cit->first;
      return TC_LOCK_SUCCESS;
    }
  }
  return TC_LOCK_INVALID_SESSION_ID;
}

/*
 *@brief      Get config map data
 *@return     tc_config_name_map_
*/
TcConfigNameMap TcLock::GetConfigMap() {
  pfc::core::ScopedMutex configmaplock(getGlobalLock());
  return tc_config_name_map_;
}

/*
*@brief      convert tc config mode as config_name
 *@param[in]  tc_mode  tc_configure_mode (global/real/virtual/vtn).
 *@param[in]  vtn_name vtn name if tc config mode is VTN.
 *@return     tc_config_name converting tc_mode as config names.
 *@return     TC_OPER_INVALID_INPUT if tc_mode or vtn_name read fail
 */
 
std::string TcLock::GetConfigName(TcConfigMode tc_mode, std::string vtn_name) {
  std::string tc_config_name;
  switch (tc_mode)
  {
    case TC_CONFIG_GLOBAL:
      tc_config_name = "global-mode";
    break;
    case TC_CONFIG_REAL:
      tc_config_name = "real-mode";
    break;
    case TC_CONFIG_VIRTUAL:
      tc_config_name = "virtual-mode";
    break;
    case TC_CONFIG_VTN:
      tc_config_name = vtn_name;
    break;
    default:
      pfc_log_error("TcGetExclusion: Reading tc_config_mode failed");
  }
  return tc_config_name;
}
/**
 *@brief   Increment or restart the configuration identifier
 *@return     TC_START_CONFIG_ID  Restart config id with 1,when it  \
                                  reaches maximum uint32_t.
 *@return     config_id           Incremented configuration ID.
 */
uint32_t TcLock::GetConfigId() {
  if (prev_config_id_ == UINT32_MAX) {
    prev_config_id_ = TC_START_CONFIG_ID;
  } else {
    ++prev_config_id_;
  }
  return prev_config_id_;
}

/**
 *@brief   Update config lock acquire data.
 *@param[in]  session_id  Session identifier.
 */
void TcLock::UpdateConfigData(uint32_t session_id, std::string tc_config_name){ 
  tc_config_name_map_[tc_config_name].session_id = session_id;
  tc_config_name_map_[tc_config_name].config_id = TcLock::GetConfigId();
  tc_config_name_map_[tc_config_name].is_taken = PFC_TRUE;
  tc_config_name_map_[tc_config_name].is_notify_pending = PFC_TRUE;
  tc_config_name_map_[tc_config_name].notify_operation = TC_NOTIFY_ACQUIRE;
}

/**
 *@brief   Acquire configuration session
 *@param[in]  session_id  Session identifier.
 *@param[in]  tc_mode TC config modes (global/real/virtual/vtn).
 *@param[in]  vtn_name string type vtn_name (if tc_mode is VTN).
 *@return     TC_LOCK_SUCCESS    When acquire configuration session.
 *@return     TC_LOCK_OPERATION_NOT_ALLOWED  When state transition in progress.
 *@return     TC_LOCK_BUSY     When configuration session already acquired .
 */
TcLockRet TcLock::AcquireConfigLock(uint32_t session_id,
                                    TcConfigMode tc_mode,
                                    std::string vtn_name) {
  if (tc_state_lock_.state_transition_in_progress == PFC_TRUE) {
    pfc_log_error("%s Config lock not allowed during state transition",
                  __FUNCTION__);
    return TC_LOCK_OPERATION_NOT_ALLOWED;
  }
  std::string tc_config_name = GetConfigName(tc_mode, vtn_name);

  if (tc_config_name.empty()) {
    return TC_LOCK_INVALID_OPERATION;
  }

  // For global mode
  if (tc_mode == TC_CONFIG_GLOBAL) {
    if (tc_config_name_map_.empty()) {
      UpdateConfigData(session_id, tc_config_name);
      return TC_LOCK_SUCCESS;
    } else {
      return TC_LOCK_BUSY;
    }
  } else { // For partial modes
    if (tc_config_name_map_.find("global-mode") != tc_config_name_map_.end() &&
        (tc_config_name_map_["global-mode"].is_taken ||
         tc_config_name_map_["global-mode"].is_notify_pending) ) {
      // Global lock is already present
      return TC_LOCK_BUSY; 

    } else if (tc_config_name_map_.find(tc_config_name)
                                != tc_config_name_map_.end() &&
               (tc_config_name_map_[tc_config_name].is_taken ||
                tc_config_name_map_[tc_config_name].is_notify_pending) ) {
      return TC_LOCK_BUSY;
    } else {
      UpdateConfigData(session_id, tc_config_name);
      return TC_LOCK_SUCCESS;
    }
  }
}

/**
 *@brief   Release configuration session
 *@param[in]  session_id  session identifier.
 *@return     TC_LOCK_SUCCESS    Will be able to release configuration session.
 *@return     TC_LOCK_INVALID_SESSION_ID  When session id does not match
 *@return     TC_LOCK_INVALID_CONFIG_ID  When config id does not match
 *@return     TC_LOCK_NOT_ACQUIRED  When config lock is acquired.
 *@return     TC_LOCK_BUSY     When Commit or Abort candidate  \
              configuration in progress.
 */
TcLockRet TcLock::ReleaseConfigLock(uint32_t session_id, uint32_t config_id ) {

  std::string tc_config_name;
  if (FindConfigName(session_id, tc_config_name) != TC_LOCK_SUCCESS) {
    return TC_LOCK_NO_CONFIG_SESSION_EXIST;
  }

  if (tc_config_name_map_[tc_config_name].is_taken) {
    if (config_id != tc_config_name_map_[tc_config_name].config_id) {
      pfc_log_error("%s Given config-id[%u] not matching existing[%u]",
                    __FUNCTION__, config_id,
                    tc_config_name_map_[tc_config_name].config_id);
      return TC_LOCK_INVALID_CONFIG_ID;
    }
    if (!tc_config_name_map_[tc_config_name].is_notify_pending &&
         !((tc_rwlock_.w_operation == TC_COMMIT ||
            tc_rwlock_.w_operation ==  TC_ABORT_CANDIDATE_CONFIG) &&
           tc_rwlock_.isWriteHeld(session_id))) {
      tc_config_name_map_[tc_config_name].is_notify_pending = PFC_TRUE;
      tc_config_name_map_[tc_config_name].is_taken = PFC_FALSE;
      tc_config_name_map_[tc_config_name].notify_operation = TC_NOTIFY_RELEASE;
      return TC_LOCK_SUCCESS;
    } else {
      return TC_LOCK_BUSY;
    }
  } else {
    pfc_log_error("%s Config mode not alloted to this seession[%u]",
                  __FUNCTION__, session_id);
    return TC_LOCK_NOT_ACQUIRED;
  }
}

/**
 *@brief   Force acquire configuration session
 *@param[in]  session_id  Session identifier.
 *@return     TC_LOCK_SUCCESS    When acquire configuration session.
 *@return     TC_LOCK_OPERATION_NOT_ALLOWED  When state transition in progress.
 *@return     TC_LOCK_BUSY    When Commit or Abort candidate  \
              configuration in progress.
 */
TcLockRet TcLock::ForceAcquireConfigLock(uint32_t session_id) {
  if (tc_state_lock_.state_transition_in_progress == PFC_TRUE) {
    pfc_log_error("%s Cannot get force config lock during state transition",
                  __FUNCTION__);
    return TC_LOCK_OPERATION_NOT_ALLOWED;
  }

  for (TcConfigNameMap::iterator it = tc_config_name_map_.begin();
       it != tc_config_name_map_.end();
       it++) {

    if (it->second.is_notify_pending ||
        (it->second.is_taken &&
         tc_rwlock_.isWriteHeld() &&
         (tc_rwlock_.w_operation == TC_COMMIT  ||
          tc_rwlock_.w_operation == TC_ABORT_CANDIDATE_CONFIG))) {
      pfc_log_error("%s Conditions not good for force config acquire",
                    __FUNCTION__);
      return TC_LOCK_BUSY;
    }
  }

  // Remove all entries from config map
  tc_config_name_map_.clear();

  // Add current force config entry to the map
  UpdateConfigData(session_id, "global-mode");
      return TC_LOCK_SUCCESS;
}

/**
 *@brief   Acquire read session
 *@param[in]  session_id  session identifier.
 *@return     TC_LOCK_SUCCESS    Will be able to acquire read session.
 *@return     TC_LOCK_ALREADY_ACQUIRED  when read session already  \
              acquired by same session ID.
 *@return     TC_LOCK_BUSY     When write in progress .
 */
TcLockRet TcLock::AcquireReadLock(uint32_t session_id) {
  // Acquire read lock.
  if (!tc_rwlock_.tryLock()) {
    return TC_LOCK_BUSY;
  }

  // Insert the given session ID into read lock set.
  std::set<uint32_t> &sessions(tc_read_lock_.read_sessions);
  std::pair<std::set<uint32_t>::iterator, bool>
    result(sessions.insert(session_id));
  if (!result.second) {
    tc_rwlock_.unlock();
    return TC_LOCK_ALREADY_ACQUIRED;
  }

  return TC_LOCK_SUCCESS;
}

/**
 *@brief   Acquire read for state transition.
 *@return     TC_LOCK_SUCCESS    Will be able to acquire read session.
 *@return     TC_LOCK_INVALID_UNC_STATE The lock can not be held in this state.
 */
TcLockRet TcLock::TcAcquireReadLockForStateTransition() {
  PFC_VERIFY(tc_state_lock_.current_state != TC_STOP);

  while (!tc_rwlock_.canReadLock() ||
         tc_state_lock_.state_transition_in_progress) {
    tc_rwlock_.waitForRead();

    // The current state must be checked again because it may be changed while
    // the global lock is released.
    if (PFC_EXPECT_FALSE(tc_state_lock_.current_state == TC_STOP)) {
      return TC_LOCK_INVALID_UNC_STATE;
    }
  }

  tc_state_lock_.state_transition_in_progress = PFC_TRUE;
  tc_rwlock_.setReader();
  return TC_LOCK_SUCCESS;
}

/**
 *@brief   Release read for state transition.
 *@return     TC_LOCK_SUCCESS    Will be able to release read lock.
 *@return     TC_LOCK_NOT_ACQUIRED     When state transition no in progress .
 */
TcLockRet TcLock::TcReleaseReadLockForStateTransition() {
  if (tc_state_lock_.state_transition_in_progress == PFC_TRUE) {
    tc_state_lock_.state_transition_in_progress = PFC_FALSE;
    tc_rwlock_.unlock();
    return TC_LOCK_SUCCESS;
  }
  return TC_LOCK_NOT_ACQUIRED;
}

/**
 *@brief   Release read session
 *@param[in]  session_id  session identifier.
 *@return     TC_LOCK_SUCCESS    Will be able to release read session.
 *@return     TC_LOCK_NOT_ACQUIRED  When read lock is acquired.
 */
TcLockRet TcLock::ReleaseReadLock(uint32_t session_id) {
  std::set<uint32_t> &sessions(tc_read_lock_.read_sessions);

  // Remove the given session ID from read lock set.
  if (sessions.erase(session_id) == 0) {
    return TC_LOCK_NOT_ACQUIRED;
  }

  tc_rwlock_.unlock();
  return TC_LOCK_SUCCESS;
}

/**
 *@brief   Acquire write lock for audit driver operation
 *@param[in]  session_id  session identifier.
 *@return     TC_LOCK_SUCCESS    When got the write lock.
 *@return     TC_LOCK_INVALID_UNC_STATE The lock can not be held in this state.
 */
TcLockRet TcLock::AcquireWriteLockForAuditDriver(uint32_t session_id) {
  while (!TcIsSetupCompleteDone()) {
    pfc_log_info("%s Waiting for SETUP_COMPLETE to finish", __FUNCTION__);
    struct timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = 100000000;
    nanosleep(&ts, NULL); //Sleep for 100ms
  }

  if (tc_rwlock_.tryLock(session_id, TC_AUDIT_DRIVER)) {
    return TC_LOCK_SUCCESS;
  }
  return TC_LOCK_BUSY;
}

/**
 *@brief   Acquire write lock for commit operation
 *@param[in]  session_id  session identifier.
 *@return     TC_LOCK_SUCCESS    When got the write lock.
 *@return     TC_LOCK_BUSY       When not able to the write lock.
 */
TcLockRet TcLock::AcquireWriteLockForCommit(uint32_t session_id) {
  std::string tc_config_name;
  if (FindConfigName(session_id, tc_config_name) != TC_LOCK_SUCCESS)  {
    pfc_log_error("%s Config not alloted for this session[%u]",
                  __FUNCTION__, session_id);
    return TC_LOCK_INVALID_SESSION_ID;
  }

  if (!tc_config_name_map_[tc_config_name].is_taken ||
      tc_config_name_map_[tc_config_name].is_notify_pending) {
    return TC_LOCK_BUSY;
  }

  if (tc_rwlock_.tryLock(session_id, TC_COMMIT)) {
    return TC_LOCK_SUCCESS;
  }
  return TC_LOCK_BUSY;
}

/**
 *@brief   Acquire write lock for candidate config operation
 *@param[in]  session_id  session identifier.
 *@return     TC_LOCK_SUCCESS    When got the write lock.
 *@return     TC_LOCK_BUSY       When not able to the write lock.
 */

TcLockRet TcLock::AcquireWriteLockForAbortCandidateConfig(uint32_t session_id) {
  std::string tc_config_name;
  if (FindConfigName(session_id, tc_config_name) != TC_LOCK_SUCCESS)  {
    return TC_LOCK_INVALID_SESSION_ID;
  }

  if (!tc_config_name_map_[tc_config_name].is_taken ||
      tc_config_name_map_[tc_config_name].is_notify_pending) {
    return TC_LOCK_BUSY;
  }

  if (tc_rwlock_.tryLock(session_id, TC_ABORT_CANDIDATE_CONFIG)) {
    return TC_LOCK_SUCCESS;
  }
  return TC_LOCK_BUSY;
}


/**
 *@brief   Acquire write lock for audit user operation
 *@param[in]  session_id  session identifier.
 *@return     TC_LOCK_SUCCESS    When got the write lock.
 *@return     TC_LOCK_BUSY       When not able to the write lock.
 */
TcLockRet TcLock::AcquireWriteLockForAuditUser(uint32_t session_id) {
  if (tc_rwlock_.tryLock(session_id, TC_AUDIT_USER)) {
    return TC_LOCK_SUCCESS;
  }
  return TC_LOCK_BUSY;
}

/**
 *@brief   Acquire write lock for save startup operation
 *@param[in]  session_id  session identifier.
 *@return     TC_LOCK_SUCCESS    When got the write lock.
 *@return     TC_LOCK_BUSY       When not able to the write lock.
 */
TcLockRet TcLock::AcquireWriteLockForSaveStartupConfig(uint32_t session_id) {
  if (tc_rwlock_.tryLock(session_id, TC_SAVE_STARTUP_CONFIG)) {
    return TC_LOCK_SUCCESS;
  }
  return TC_LOCK_BUSY;
}

/**
 *@brief   Acquire write lock for clear startup operation.
 *@param[in]  session_id  session identifier.
 *@return     TC_LOCK_SUCCESS    When got the write lock.
 *@return     TC_LOCK_BUSY       When not able to the write lock.
 */
TcLockRet TcLock::AcquireWriteLockForClearStartupConfig(uint32_t session_id) {
  if (tc_rwlock_.tryLock(session_id, TC_CLEAR_STARTUP_CONFIG)) {
    return TC_LOCK_SUCCESS;
  }
  return TC_LOCK_BUSY;
}

/**
 *@brief   Acquire write lock  main member function.
 *@param[in]  session_id  session identifier.
 *@param[in]  write_operation  write sub operation   \
            (commit,audit user or driver,candidate config,save/clear startup).
 *@return     TC_LOCK_SUCCESS    When got the write lock.
 *@return     TC_LOCK_BUSY       When not able to the write lock.
 *@return     TC_LOCK_INVALID_OPERATION When unknown write operation.
 */
TcLockRet TcLock::AcquireWriteLock(uint32_t session_id,
    TcWriteOperation write_operation) {
  TcLockRet ret = TC_LOCK_SUCCESS;
  switch (write_operation) {
  case TC_AUDIT_DRIVER:
    ret = AcquireWriteLockForAuditDriver(session_id);
    break;
  case TC_COMMIT:
    ret = AcquireWriteLockForCommit(session_id);
    break;
  case TC_ABORT_CANDIDATE_CONFIG:
    ret = AcquireWriteLockForAbortCandidateConfig(session_id);
    break;
  case TC_AUDIT_USER:
    ret = AcquireWriteLockForAuditUser(session_id);
    break;
  case TC_CLEAR_STARTUP_CONFIG:
    ret = AcquireWriteLockForClearStartupConfig(session_id);
    break;
  case TC_SAVE_STARTUP_CONFIG:
    ret = AcquireWriteLockForSaveStartupConfig(session_id);
    break;
  default:
    ret = TC_LOCK_INVALID_OPERATION;
    break;
  }
  return ret;
}

/**
 *@brief   Release write session .
 *@param[in]  session_id  session identifier.
 *@param[in]  write_operation  write sub operation   \
            (commit,audit user or driver,candidate config,save/clear startup).
 *@return     TC_LOCK_SUCCESS    when release the write lock for session.
 *@return     TC_LOCK_NOT_ACQUIRED   When no write lock is acquired.
 */
TcLockRet TcLock::ReleaseWriteLock(uint32_t session_id,
                                   TcWriteOperation write_operation) {
  return (tc_rwlock_.unlock(session_id, write_operation))
    ? TC_LOCK_SUCCESS : TC_LOCK_NOT_ACQUIRED;
}

/**
 *@brief   Auto save enable .
 *@param[in]  None.
 *@return     TC_LOCK_SUCCESS    When enable auto save.
 *@return     TC_LOCK_BUSY       When already enable auto save.
 */
TcLockRet TcLock::AutoSaveEnable() {
  if (tc_auto_save_.is_enable == PFC_TRUE) {
    return TC_LOCK_BUSY;
  } else  {
    tc_auto_save_.is_enable = PFC_TRUE;
    return TC_LOCK_SUCCESS;
  }
}

/**
 *@brief   Update state change
 *@param[in]  new_state      current state update to new state.
 */
void TcLock::TcUpdateUncState(TcState new_state) {
  pfc::core::ScopedMutex m(getGlobalLock());

  // Don't change TC_STOP state.
  TcState cur(tc_state_lock_.current_state);
  if (cur != new_state && cur != TC_STOP) {
    tc_state_lock_.current_state = new_state;

    // Wake up all threads blocked on the read/write lock.
    tc_rwlock_.wakeUp();
  }
}

/**
 *@brief   Check whether TC operation allowed based on current state.
 *@param[in]  operation  TC Operation .
 *@param[in]  tc_mode of type TcConfigMode (global/real/virtual/VTN)
 *@param[in]  vtn_name of type std::string if tc_mode is VTN.
 *@return     TC_LOCK_SUCCESS    When  TC operation is allowed.
 *@return     TC_LOCK_INVALID_UNC_STATE  TC operation is not allowed in this state.
 */
TcLockRet TcLock::TcOperationIsAllowed(uint32_t session_id,
                                       TcOperation operation) {
  TcState state(tc_state_lock_.current_state);

  /* check operation is allowed in this state */
  switch (state) {
  case TC_ACT:
    break;
  case TC_SBY:
  case TC_ACT_FAIL:
    if (operation != TC_ACQUIRE_READ_SESSION &&
        operation != TC_ACQUIRE_READ_LOCK_FOR_STATE_TRANSITION) {
      return TC_LOCK_INVALID_UNC_STATE;
    }
    break;

  case TC_INIT:
    if (operation != TC_ACQUIRE_READ_LOCK_FOR_STATE_TRANSITION) {
      return TC_LOCK_INVALID_UNC_STATE;
    }
    break;

  case TC_STOP:    // Disable further lock operation if already stopped.
  default:
    return TC_LOCK_INVALID_UNC_STATE;
  }

  /* if this session ID is marked for release don't allow any operation */
  if (state == TC_ACT) {
    std::string tc_config_name;
    if (FindConfigName(session_id, tc_config_name) == TC_LOCK_SUCCESS) {
      uint32_t marked = tc_config_name_map_[tc_config_name].marked_session_id;
      if (PFC_EXPECT_FALSE(marked != 0 && marked == session_id)) {
        return TC_LOCK_OPERATION_NOT_ALLOWED;
      }
    }
  }

  return TC_LOCK_SUCCESS;
}

/**
 *@brief   Mark session id to not allow any operation.
 *@param[in]  session_id  session identifier.
 *@return     TC_LOCK_SUCCESS    Set session id.
 *@return     TC_LOCK_FAILURE    When already marked session exist.
 */
TcLockRet TcLock::TcMarkSessionId(uint32_t session_id) {
  std::string tc_config_name;
  pfc::core::ScopedMutex config_map_lock(getGlobalLock());
  if (FindConfigName(session_id, tc_config_name) != TC_LOCK_SUCCESS) {
    pfc_log_error("%s Sess[%u] not available", __FUNCTION__, session_id);
    return TC_LOCK_INVALID_SESSION_ID;
  } else {
    if (tc_config_name_map_[tc_config_name].is_taken &&
        tc_config_name_map_[tc_config_name].marked_session_id == 0 &&
        session_id == tc_config_name_map_[tc_config_name].session_id) {
      tc_config_name_map_[tc_config_name].marked_session_id = session_id;
      return TC_LOCK_SUCCESS;
    } else  {
      return TC_LOCK_FAILURE;
    }
  }
}

/**
 *@brief   Check if any session ID is marked for release.
 *@param[in]   None.
 *@return     PFC_TRUE  If any  session is marked.
 *@return     PFC_FALSE No session is marked .
 */
uint32_t TcLock::GetMarkedSessionId(uint32_t session_id) {
  pfc::core::ScopedMutex m(getGlobalLock());
  std::string tc_config_name;
  if (FindConfigName(session_id, tc_config_name) != TC_LOCK_SUCCESS) {
    return TC_LOCK_INVALID_SESSION_ID;
  }
  return tc_config_name_map_[tc_config_name].marked_session_id;
}
/**
 *@brief   Interface provided to TcOperation class to acquire a   \
           lock(read,config,write).
 *@param[in]  session_id  session identifier.
 *@param[in]  tc_operation   TC main operations(acquire config,read,write).
 *@param[in]  write_operation  write sub operation   \
            (commit,audit user or driver,candidate config,save/clear startup).
 *@param[in]  tc_config_name TC config modes as name (global/real/virtual/vtn).
 *@return     TC_LOCK_SUCCESS    when TC operation acquire lock.
 *@return     TC_LOCK_INVALID_UNC_STATE  TC operation is not allowed in this state.
 *@return     TC_LOCK_BUSY     When TC operation already acquired lock .
 *@return     TC_LOCK_INVALID_OPERATION    When unkown TC operation.
 */
TcLockRet
TcLock::GetLock(uint32_t session_id, TcOperation tc_operation,
                             TcWriteOperation write_operation,
                             TcConfigMode tc_mode,
                             std::string vtn_name) {
  TcLockRet ret = TC_LOCK_SUCCESS;

  pfc::core::ScopedMutex m(getGlobalLock());
  ret = TcOperationIsAllowed(session_id, tc_operation);
  if (ret != TC_LOCK_SUCCESS)  {
    return ret;
  }

  /* TC Operation switch case handling */
  switch (tc_operation) {
  case TC_ACQUIRE_CONFIG_SESSION:
    ret = AcquireConfigLock(session_id, tc_mode, vtn_name);
    break;
  case TC_ACQUIRE_READ_SESSION:
    ret = AcquireReadLock(session_id);
    break;
  case TC_ACQUIRE_WRITE_SESSION:
    ret = AcquireWriteLock(session_id, write_operation);
    break;
  case TC_FORCE_ACQUIRE_CONFIG_SESSION:
    ret = ForceAcquireConfigLock(session_id);
    break;
  case TC_AUTO_SAVE_ENABLE:
    ret = AutoSaveEnable();
    if (ret == TC_LOCK_SUCCESS) {
      ret = AcquireWriteLock(session_id, TC_CLEAR_STARTUP_CONFIG);
      if (ret != TC_LOCK_SUCCESS) {
        AutoSaveDisable();
      }
    }
    break;
  case TC_AUTO_SAVE_DISABLE:
    ret = AutoSaveEnable();
    if (ret == TC_LOCK_SUCCESS) {
      ret = AcquireWriteLock(session_id, TC_SAVE_STARTUP_CONFIG);
      if (ret != TC_LOCK_SUCCESS) {
        AutoSaveDisable();
      }
    }
    break;
  case TC_AUTO_SAVE_GET:
    ret = AutoSaveEnable();
    break;
  case TC_ACQUIRE_READ_LOCK_FOR_STATE_TRANSITION:
    ret = TcAcquireReadLockForStateTransition();
    break;
  default:
    ret = TC_LOCK_INVALID_OPERATION;
  }
  return ret;
}

/**
 *@brief   Auto save disable .
 *@param[in]  None.
 *@return     TC_LOCK_SUCCESS    When enable auto save.
 *@return     TC_LOCK_FAILURE       When already disable auto save.
 */
TcLockRet TcLock::AutoSaveDisable() {
  if (tc_auto_save_.is_enable == PFC_TRUE) {
    tc_auto_save_.is_enable = PFC_FALSE;
    return TC_LOCK_SUCCESS;
  } else  {
    return TC_LOCK_FAILURE;
  }
}

/**
 *@brief  Acquire/Release  config. ID and session ID is notified to UPPL/UPLL.
 *@param[in]  session_id  session identifier.
 *@param[in]  config_id   configuration  identifier.
 *@return     TC_LOCK_SUCCESS    when acquire configuration session.
 *@return     TC_LOCK_INVALID_CONFIG_ID   Config. ID does not match with  \
              configuration session config. ID.
 *@return     TC_LOCK_INVALID_SESSION_ID  Session ID does not match with   \
              configuration session session ID.
 *@return     TC_LOCK_INVALID_OPERATION     For unkown Notify operation.
 */
TcLockRet TcLock::NotifyConfigIdSessionIdDone(uint32_t config_id,
                                   uint32_t session_id,
                                   TcNotifyOperation config_notify_operation) {
  pfc::core::ScopedMutex m(getGlobalLock());
  std::string tc_config_name;
  if (FindConfigName(session_id, tc_config_name) != TC_LOCK_SUCCESS) {
    pfc_log_error("%s session_id[%u] not found", __FUNCTION__, session_id);
    return TC_LOCK_INVALID_SESSION_ID;
  }

  if (tc_config_name_map_[tc_config_name].notify_operation != 
                                                     config_notify_operation) {
    pfc_log_error("%s Notify operation %d not matching required op %d",
                  __FUNCTION__, config_notify_operation,
                  tc_config_name_map_[tc_config_name].notify_operation);
    return TC_LOCK_INVALID_OPERATION;
  }

  if (tc_config_name_map_[tc_config_name].config_id != config_id)  {
    pfc_log_error("%s Given configid[%u] not matching required configid[%u]",
                  __FUNCTION__, config_id,
                  tc_config_name_map_[tc_config_name].config_id);
    return TC_LOCK_INVALID_CONFIG_ID;
  }

  tc_config_name_map_[tc_config_name].notify_operation = TC_NOTIFY_NONE;
  tc_config_name_map_[tc_config_name].is_notify_pending = PFC_FALSE;
  /* Erasing the specific entry from TC_CONFIG_NAME_MAP */
  if (config_notify_operation == TC_NOTIFY_RELEASE) {
    tc_config_name_map_.erase(tc_config_name);
  }

  return TC_LOCK_SUCCESS;
}

/**
 *@brief   Interface provided to TcOperation class to release a   \
           lock(read,config,write).
 *@param[in]  session_id  session identifier.
 *@param[in]  tc_operation   TC main operations(relase config,read,write).
 *@param[in]  write_operation  write sub operation(commit,  \
  audit user or driver,candidate config,save/clear startup).
 *@return     TC_LOCK_SUCCESS    when TC operation release lock.
 *@return     TC_LOCK_INVALID_UNC_STATE  TC operation is not allowed .
 *@return     TC_LOCK_ALREADY_ACQUIRED  when read session already   \
              acquired by same session ID.
 *@return     TC_LOCK_NOT_ACQUIRED   When no write lock is acquired.
 *@return     TC_LOCK_BUSY      TC operation already acquired lock .
 *@return     TC_LOCK_INVALID_OPERATION     For unkown TC operation.
 */
TcLockRet
TcLock::ReleaseLock(uint32_t session_id, uint32_t config_id,
                    TcOperation tc_operation,
                    TcWriteOperation write_operation) {
  TcLockRet ret = TC_LOCK_SUCCESS;

  pfc::core::ScopedMutex m(getGlobalLock());
  switch (tc_operation) {
  case TC_RELEASE_CONFIG_SESSION:
    ret = ReleaseConfigLock(session_id, config_id);
    break;
  case TC_RELEASE_READ_SESSION:
    ret = ReleaseReadLock(session_id);
    break;
  case TC_RELEASE_WRITE_SESSION:
    ret= ReleaseWriteLock(session_id, write_operation);
    break;
  case TC_AUTO_SAVE_ENABLE:
    ret= AutoSaveDisable();
    if (ret == TC_LOCK_SUCCESS) {
      ret = ReleaseWriteLock(session_id, TC_CLEAR_STARTUP_CONFIG);
      if (ret != TC_LOCK_SUCCESS) {
        AutoSaveEnable();
      }
    }
    break; 
  case TC_AUTO_SAVE_DISABLE:
    ret= AutoSaveDisable();
    if (ret == TC_LOCK_SUCCESS) {
      ret = ReleaseWriteLock(session_id, TC_SAVE_STARTUP_CONFIG);
      if (ret != TC_LOCK_SUCCESS) {
        AutoSaveEnable();
      }
    }
    break;
  case TC_AUTO_SAVE_GET:
    ret= AutoSaveDisable();
    break;
  case TC_RELEASE_READ_LOCK_FOR_STATE_TRANSITION:
    ret = TcReleaseReadLockForStateTransition();
    break;
  default:
    ret = TC_LOCK_INVALID_OPERATION;
  }
  return ret;
}

/*
 *@brief   Get configuration Data .
 *@param[in]  session_id  session identifier.
 *@param[out] config_id   configuration identifier.
 *@param[out] config mode operation (Global/Real/VTN).
 *@param[out] vtn_name, if config mode is VTN.
 *@return TC_LOCK_SUCCESS    When config session exist.
 *@return TC_LOCK_INVALID_UNC_STATE  When state is other than ACT.
 *@return TC_LOCK_NO_CONFIG_SESSION_EXIST  When no configuration session exist.
 *@return TC_LOCK_INVALID_PARAMS   When one/both  of passed pointer are NULL.
 */
TcLockRet
TcLock::GetConfigData(uint32_t session_id,
                      uint32_t& config_id,
                      TcConfigMode& tc_mode,
                      std::string& vtn_name) {

  vtn_name.clear();

  pfc::core::ScopedMutex m(getGlobalLock());
  if (tc_state_lock_.current_state != TC_ACT) {
    return TC_LOCK_INVALID_UNC_STATE;
  }
  std::string tc_config_name;
  if (FindConfigName(session_id, tc_config_name) != TC_LOCK_SUCCESS)  {
    return TC_LOCK_INVALID_SESSION_ID;
  }

  if (tc_config_name_map_[tc_config_name].is_taken) {
    config_id = tc_config_name_map_[tc_config_name].config_id;
    if (tc_config_name == "global-mode")  {
      tc_mode = TC_CONFIG_GLOBAL;
    } else if (tc_config_name == "real-mode") {
        tc_mode = TC_CONFIG_REAL;
    } else if (tc_config_name == "virtual-mode")  {
        tc_mode = TC_CONFIG_VIRTUAL;
    } else {
        tc_mode = TC_CONFIG_VTN;
        vtn_name = tc_config_name;
    }
    return TC_LOCK_SUCCESS;
  } else {
    config_id = 0;
    return TC_LOCK_NO_CONFIG_SESSION_EXIST;
  }
}

/**
 *@brief   Get current session in progress
 *@param[in]  session_id  session identifier.
 *@return     TC_CONFIG_COMMIT_PROGRESS  Read operation 
 *@return     TC_WRITE_AUDIT_USER_PROGRESS  Audit user 
 *@return     TC_WRITE_AUDIT_DRIVER_PROGRESS  Audit driver
 *@return     TC_CONFIG_NOTIFY_ACQUIRE_PROGRESS  Config. acquire notify
 *@return     TC_CONFIG_NOTIFY_RELEASE_PROGRESS  Config. release notify
 *@return     TC_CONFIG_NO_NOTIFY_PROGRESS       Config. no notify
 *@return     TC_READ_PROGRESS               Read 
 *@return     TC_WRITE_ABORT_CANDIDATE_CONFIG_PROGRESS Abort candidate config
 *@return     TC_WRITE_SAVE_STARTUP_CONFIG_PROGRESS    Save start up
 *@return     TC_WRITE_CLEAR_STARTUP_CONFIG_PROGRESS    Clear start up
 *@return     TC_NO_OPERATION_PROGRESS   No operation/unkown session id 
 */
TcSessionOperationProgress TcLock::GetSessionOperation(uint32_t session_id) {
  pfc::core::ScopedMutex m(getGlobalLock());
  std::string tc_config_name;

  /* session id matches with write lock session id */
  if (tc_rwlock_.isWriteHeld(session_id)) {
    TcWriteOperation  wop(tc_rwlock_.w_operation);
    if (wop == TC_COMMIT &&
        FindConfigName(session_id, tc_config_name) == TC_LOCK_SUCCESS &&
        tc_config_name_map_[tc_config_name].is_taken) {
      return TC_CONFIG_COMMIT_PROGRESS;
    } else if (wop == TC_AUDIT_USER) {
      return TC_WRITE_AUDIT_USER_PROGRESS;
    } else if (wop == TC_AUDIT_DRIVER) {
      return TC_WRITE_AUDIT_DRIVER_PROGRESS;
    } else if (wop == TC_ABORT_CANDIDATE_CONFIG  &&
               FindConfigName(session_id, tc_config_name) == TC_LOCK_SUCCESS &&
               tc_config_name_map_[tc_config_name].is_taken) {
      return TC_WRITE_ABORT_CANDIDATE_CONFIG_PROGRESS;
    } else if (wop == TC_SAVE_STARTUP_CONFIG) {
      return TC_WRITE_SAVE_STARTUP_CONFIG_PROGRESS;
    } else if (wop == TC_CLEAR_STARTUP_CONFIG) {
      return TC_WRITE_CLEAR_STARTUP_CONFIG_PROGRESS;
    }
  }

  /* session id matches with config lock session id  */
  tc_config_name.clear();
  if (FindConfigName(session_id, tc_config_name) == TC_LOCK_SUCCESS &&
      (tc_config_name_map_[tc_config_name].is_taken || 
       tc_config_name_map_[tc_config_name].is_notify_pending)) {
    if (tc_config_name_map_[tc_config_name].notify_operation 
                                               == TC_NOTIFY_NONE) {
      return TC_CONFIG_NO_NOTIFY_PROGRESS;
    } else if (tc_config_name_map_[tc_config_name].notify_operation 
                                            == TC_NOTIFY_ACQUIRE) {
      return TC_CONFIG_NOTIFY_ACQUIRE_PROGRESS;
    } else if (tc_config_name_map_[tc_config_name].notify_operation 
                                            == TC_NOTIFY_RELEASE) {
      return TC_CONFIG_NOTIFY_RELEASE_PROGRESS;
    }
  }

  /* session id matches with read lock session id  list */
  std::set<uint32_t> &sessions(tc_read_lock_.read_sessions);
  std::set<uint32_t>::iterator it(sessions.find(session_id));
  if (it != sessions.end()) {
    return TC_READ_PROGRESS;
  }
  return TC_NO_OPERATION_PROGRESS;
}

/**
 *@brief      Set whether SETUP_COMPLETE is completed
 *@param[in]  pfc_bool_t whether completed or not
 *@return     None
 */
void TcLock::TcSetSetupComplete(pfc_bool_t is_done) {
  pfc::core::ScopedMutex m(setup_complete_flag_lock_);
  setup_complete_done_ = is_done;
}

/**
 *@brief      Return whether SETUP_COMPLETE is completed
 *@param[in]  
 *@return     pfc_bool_t whether completed or not
 */
pfc_bool_t TcLock::TcIsSetupCompleteDone() {
  pfc::core::ScopedMutex m(setup_complete_flag_lock_);
  return setup_complete_done_;
}

/**
 *@brief      Return true if last op is commit or abort
 *@param[in]  None
 *@return     pfc_bool_t
 */
pfc_bool_t TcLock::IsLastOperationCandidate() {
  pfc::core::ScopedMutex m(write_op_lock);
  return (tc_rwlock_.last_write_op == TC_COMMIT ||
          tc_rwlock_.last_write_op == TC_ABORT_CANDIDATE_CONFIG);
}

/**
 *@brief      Return write operation that currently helds lock
 *@param[in]  None
 *@return     TcWriteOperation
 */
TcWriteOperation TcLock::TcGetCurrentWriteOperation() {
  pfc::core::ScopedMutex m(write_op_lock);
  return tc_rwlock_.w_operation;
}

/**
 *@brief      return whether write lock can be acquired
 *@param[in]  None
 *@return     pfc_bool_t
 */
pfc_bool_t TcLock::CanWriteLock() {
  pfc::core::ScopedMutex m(getGlobalLock());
  return tc_rwlock_.canWriteLock();
}

}   // namespace tc
}   // namespace unc
