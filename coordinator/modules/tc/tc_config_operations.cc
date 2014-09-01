/*
 * Copyright (c) 2012-2014 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */


#include <tc_operations.hh>
#include <sys/time.h>


namespace unc {
namespace tc {

#define UNC_CONFIG_OPER_ARG_COUNT_MIN  2
#define UNC_CONFIG_OPER_ARG_COUNT_MAX  3
#define UNC_CONFIG_ACQUIRE_ARG_COUNT 2
#define UNC_CONFIG_RELEASE_ARG_COUNT 3
#define UNC_CONFIG_ACQUIRE_TIMED_ARG_COUNT 3


pfc_bool_t TcConfigOperations::config_mode_available_ = PFC_TRUE;
pfc::core::Mutex TcConfigOperations::config_mode_available_lock_;

std::deque<TcConfigOperations*> TcConfigOperations::config_req_queue_;
pfc::core::Mutex TcConfigOperations::config_req_queue_lock_;

pfc_bool_t TcConfigOperations::state_changed_to_sby_ = PFC_FALSE;
pfc::core::Mutex TcConfigOperations::state_changed_lock_;

TcConfigOperations::TcConfigOperations(TcLock* tc_lock_,
                           pfc::core::ipc::ServerSession* sess_,
                           TcDbHandler* db_handler,
                           TcChannelNameMap& unc_map_):
    TcOperations(tc_lock_, sess_, db_handler, unc_map_) {
  config_id_ = 0;
  timeout_ = 0;
  cond_var_ = NULL;
  pthread_mutex_init(&mutex_var_, NULL);
}

TcConfigOperations::~TcConfigOperations() {
}

pfc_bool_t TcConfigOperations::IsConfigModeAvailable() {
  pfc::core::ScopedMutex m(config_mode_available_lock_);
  pfc_log_debug("IsConfigModeAvailable ? %d", config_mode_available_);
  return config_mode_available_;
}

void TcConfigOperations::SetConfigModeAvailability(pfc_bool_t status) {
  pfc::core::ScopedMutex m(config_mode_available_lock_);
  config_mode_available_ = status;
}

pfc_bool_t TcConfigOperations::IsStateChangedToSby() {
  pfc::core::ScopedMutex m(state_changed_lock_);
  return state_changed_to_sby_;
}

void  TcConfigOperations::SetStateChangedToSby(pfc_bool_t state) {
  pfc::core::ScopedMutex m(state_changed_lock_);
  state_changed_to_sby_ = state;
}

void TcConfigOperations::InsertConfigRequest() {
  pfc_log_debug("TcConfigOperations::InsertConfigRequest");
  config_req_queue_.push_back(this);
  pfc_log_info("InsertConfigRequest current wait queue size:%" PFC_PFMT_SIZE_T,
               config_req_queue_.size());
}

void TcConfigOperations::RemoveConfigRequest(uint32_t sess_id) {
  pfc_log_debug("RemoveConfigRequest for sess[%u]",sess_id);
  std::deque<TcConfigOperations*>::iterator it = config_req_queue_.begin();
  for(; it!= config_req_queue_.end(); it++) {
    if ((*it)->session_id_ == sess_id) {
      pfc_log_debug("Found. Removing for sess[%d] done", sess_id);
      config_req_queue_.erase(it);
      break;
    }
  }
  pfc_log_info("RemoveConfigRequest current wait queue size:%" PFC_PFMT_SIZE_T,
               config_req_queue_.size());
}

TcConfigOperations * TcConfigOperations::RetrieveConfigRequest() {
  TcConfigOperations * configoper = NULL;
  pfc_log_info("RetrieveConfigRequest current wait queue size:%"
               PFC_PFMT_SIZE_T, config_req_queue_.size());
  std::deque<TcConfigOperations*>::iterator it = config_req_queue_.begin();
  if (it != config_req_queue_.end()) {
    configoper = *it;
    return configoper;
  }

  return NULL;
}

pfc_bool_t TcConfigOperations::IsConfigReqQueueEmpty() {
  pfc::core::ScopedMutex m(config_req_queue_lock_);
  return config_req_queue_.empty();
}

/* 
 * @desc - If queue size if 1 => Only current request in queue.
 *            Acquire config request.
 *         If queue size > 1, other requests are waiting.
 *            Even if config mode is available, it is for earlier
 *            requests.
 */
pfc_bool_t TcConfigOperations::IsConfigAcquireAllowed() {
  return (config_req_queue_.size() <= 1);
}

void TcConfigOperations::ClearConfigAcquisitionQueue() {
  pfc_log_debug("TcConfigOperations::ClearConfigAcquisitionQueue");
  pfc::core::ScopedMutex m(config_req_queue_lock_);
  while (!config_req_queue_.empty()) {

    TcConfigOperations * configoper = config_req_queue_.front();
    if(configoper) {
      pfc_log_info("ClearConfigAcquisitionQueue processing sess[%u]",
                   configoper->session_id_);
      // Call cond_signal
      if (configoper->cond_var_) {
        pfc_log_debug("ClearConfigAcquisitionQueue: pthread_cond_signal");
        int32_t ret = pthread_cond_signal(configoper->cond_var_);
        if (ret != 0) {
          pfc_log_error("Error signalling condition for session_id: %u",
                        configoper->session_id_);
        }
      } else {
        pfc_log_error("%s cond_var_ NULL", __FUNCTION__);
      }
    }

    config_req_queue_.pop_front();
  }
}


/**
 *  @brief Write Config ID to Output session
 */
TcOperStatus TcConfigOperations::SetConfigId() {
  pfc_log_info("tc_config_oper: Setting configId to response");
  TcUtilRet ret=
      TcServerSessionUtils::set_uint32(ssess_,
                                       config_id_);
  if ( ret != TCUTIL_RET_SUCCESS ) {
    pfc_log_info("tc_config_oper: set config oper failure");
    return TC_OPER_FAILURE;
  }
  return TC_OPER_SUCCESS;
}

/**
 *  @brief Return Minimum argument count for config operations
 */
uint32_t TcConfigOperations::TcGetMinArgCount() {
  return UNC_CONFIG_OPER_ARG_COUNT_MIN;
}

/**
 *  @brief Handle return value from TcLock
 */
TcOperStatus TcConfigOperations::HandleLockRet(TcLockRet lock_ret) {
  TcOperStatus ret = TC_OPER_FAILURE;

  switch ( lock_ret ) {
    case TC_LOCK_INVALID_UNC_STATE:
    case TC_LOCK_OPERATION_NOT_ALLOWED:
      ret = TC_INVALID_STATE;
      break;
    case TC_LOCK_BUSY:
      ret = TC_SYSTEM_BUSY;
      break;
    case TC_LOCK_INVALID_SESSION_ID:
      ret = TC_INVALID_SESSION_ID;
      break;
    case TC_LOCK_INVALID_CONFIG_ID:
      ret = TC_INVALID_CONFIG_ID;
      break;
    case TC_LOCK_NOT_ACQUIRED:
    case TC_LOCK_NO_CONFIG_SESSION_EXIST:
      ret =  TC_CONFIG_NOT_PRESENT;
      break;
    default:
      ret = TC_OPER_FAILURE;
  }
  pfc_log_info("HandleLockRet: Received(%u), return(%u)",
               lock_ret, ret);
  return ret;
}

/**
 *  @brief Check the number of input arguments for the operation
 */
TcOperStatus TcConfigOperations::TcCheckOperArgCount(uint32_t avail_count) {
  pfc_log_info("tc_config_oper: Check count of Input Arguments");
  if ( tc_oper_ == TC_OP_CONFIG_ACQUIRE ||
        tc_oper_ == TC_OP_CONFIG_ACQUIRE_FORCE ) {
    if ( avail_count != UNC_CONFIG_ACQUIRE_ARG_COUNT ) {
      pfc_log_error("TcCheckOperArgCount aq args expected(%u) received(%u)",
                    UNC_CONFIG_ACQUIRE_ARG_COUNT, avail_count);
      return TC_OPER_INVALID_INPUT;
    }
  } else if ( tc_oper_ == TC_OP_CONFIG_RELEASE ) {
    if ( avail_count != UNC_CONFIG_RELEASE_ARG_COUNT ) {
      pfc_log_error("TcCheckOperArgCount rel args expected(%u) received(%u)",
                    UNC_CONFIG_RELEASE_ARG_COUNT, avail_count);
      return TC_OPER_INVALID_INPUT;
    }
  } else if (tc_oper_== TC_OP_CONFIG_ACQUIRE_TIMED) {
    if (avail_count != UNC_CONFIG_ACQUIRE_TIMED_ARG_COUNT) {
      pfc_log_error("TcCheckOperArgCount aqtime args expected(%u) recvd(%u)",
                    UNC_CONFIG_ACQUIRE_TIMED_ARG_COUNT, avail_count);
      return TC_OPER_INVALID_INPUT;
    }
  }
  return TC_OPER_SUCCESS;
}

/**
 *  @brief Validate the operation type from input
 */
TcOperStatus TcConfigOperations::TcValidateOperType() {
  pfc_log_info("tc_config_oper: Validate Oper Type");
  if (tc_oper_ < TC_OP_CONFIG_ACQUIRE ||
      tc_oper_ > TC_OP_CONFIG_ACQUIRE_FORCE) {
    pfc_log_error("TcValidateOperType opertype < TC_OP_CONFIG_ACQUIRE or"
                  " > TC_OP_CONFIG_ACQUIRE_FORCE");
    return TC_INVALID_OPERATION_TYPE;
  }
  return TC_OPER_SUCCESS;
}

/**
 *  @brief Validate the operation paramaters for the service
 */
TcOperStatus TcConfigOperations::TcValidateOperParams() {
  pfc_log_info("tc_config_oper: Validate Oper Params");
  if ( tc_oper_ != TC_OP_CONFIG_RELEASE &&
       tc_oper_ != TC_OP_CONFIG_ACQUIRE_TIMED) {
    return TC_OPER_SUCCESS;
  }

  if (tc_oper_ == TC_OP_CONFIG_RELEASE) {
    TcUtilRet ret(TcServerSessionUtils::get_uint32(ssess_,
                                           TC_REQ_ARG_INDEX, &config_id_));
    if ( ret == TCUTIL_RET_FAILURE ) {
      pfc_log_error("TcValidateOperParams: gettng config_id failed");
      return TC_OPER_INVALID_INPUT;
    } else if ( ret == TCUTIL_RET_FATAL ) {
      pfc_log_error("TcValidateOperParams: gettng config_id fatal");
      return TC_OPER_FAILURE;
    }

    uint32_t validate_session_id = 0, validate_config_id = 0;
    TcLockRet LockRet = tclock_->GetConfigIdSessionId(&validate_session_id,
                                                      &validate_config_id);
    if (LockRet != TC_LOCK_SUCCESS) {
      return HandleLockRet(LockRet);
    }
    if ( validate_session_id != session_id_ ) {
      pfc_log_error("TcValidateOperParams: sess_id(%u) != expected (%u)",
                    session_id_, validate_session_id);
      return TC_INVALID_SESSION_ID;
    } else if ( validate_config_id != config_id_ ) {
      pfc_log_error("TcValidateOperParams: config_id(%u) != expected (%u)",
                    config_id_, validate_config_id);
       return TC_INVALID_CONFIG_ID;
    }
    pfc_log_debug("TC_OP_CONFIG_RELEASE config_id:%d", config_id_);
  } else if (tc_oper_ == TC_OP_CONFIG_ACQUIRE_TIMED) {

    TcUtilRet ret(TcServerSessionUtils::get_int32(ssess_,
                                                  TC_REQ_ARG_INDEX,
                                                  &timeout_));
    if (ret == TCUTIL_RET_FAILURE ) {
      pfc_log_error("TcValidateOperParams: Reading timeout value failed");
      return TC_OPER_INVALID_INPUT;
    } else if ( ret == TCUTIL_RET_FATAL ) {
      pfc_log_error("TcValidateOperParams: Reading timeout value fatal");
      return TC_OPER_FAILURE;
    }

    pfc_log_debug("TC_OP_CONFIG_ACQUIRE_TIMED timeout:%d", timeout_);
  }
  return TC_OPER_SUCCESS;
}

/**
 *  @brief Secure Exclusion for the operation
 */
TcOperStatus TcConfigOperations::TcGetExclusion() {
  TcLockRet ret;
  if ( tc_oper_ == TC_OP_CONFIG_RELEASE ) {
    pfc_log_info("Release Config Request");
    ret= tclock_->ReleaseLock(session_id_,
                              config_id_,
                              TC_RELEASE_CONFIG_SESSION,
                              TC_WRITE_NONE);
    if (ret != TC_LOCK_SUCCESS) {
      return HandleLockRet(ret);
    }
  } else {
    if (tc_oper_ == TC_OP_CONFIG_ACQUIRE ||
        tc_oper_ == TC_OP_CONFIG_ACQUIRE_TIMED) {
      pfc_log_info("Acquire Config Request");
      ret= tclock_->GetLock(session_id_,
                           TC_ACQUIRE_CONFIG_SESSION,
                           TC_WRITE_NONE);
    } else {
      pfc_log_info("Force Acquire Config Request");
      ret= tclock_->GetLock(session_id_,
                           TC_FORCE_ACQUIRE_CONFIG_SESSION,
                           TC_WRITE_NONE);
    }
    if (ret != TC_LOCK_SUCCESS) {
      return HandleLockRet(ret);
    }
  }
  return TC_OPER_SUCCESS;
}

/**
 *  @brief Release Exclusion after operation
 */
TcOperStatus TcConfigOperations::TcReleaseExclusion() {
  TcLockRet ret;
  if (tc_oper_ == TC_OP_CONFIG_RELEASE) {
    ret=
       tclock_->NotifyConfigIdSessionIdDone(config_id_,
                                            session_id_,
                                            TC_NOTIFY_RELEASE);
  } else {
    ret=
       tclock_->NotifyConfigIdSessionIdDone(config_id_,
                                            session_id_,
                                            TC_NOTIFY_ACQUIRE);
  }
  if (ret != TC_LOCK_SUCCESS) {
    return HandleLockRet(ret);
  }
  return TC_OPER_SUCCESS;
}

TcOperStatus TcConfigOperations::TcCreateMsgList() {
  if (TcOperMessageList. empty()) {
    TcOperMessageList.push_back(unc::tclib::MSG_NOTIFY_CONFIGID);
    return TC_OPER_SUCCESS;
  }
  return TC_OPER_FAILURE;
}

/**
 *  @brief List of messages to be notified for this operation
 */
TcOperStatus TcConfigOperations::FillTcMsgData(TcMsg* tc_msg,
                           TcMsgOperType oper_type) {
  if ( tc_msg == NULL )
    return TC_OPER_FAILURE;
  // Update the values with sessionId and config ID
  if ( tc_oper_ != TC_OP_CONFIG_RELEASE ) {
    tclock_->GetConfigIdSessionId(&session_id_, &config_id_);
    tc_msg->SetData(config_id_, "", UNC_CT_UNKNOWN);
  } else {
    /*set config_id_=0 for TC_OP_CONFIG_RELEASE*/
    tc_msg->SetData(0, "", UNC_CT_UNKNOWN);
  }
  return TC_OPER_SUCCESS;
}

/**
 *  @brief Additional output of operation to be written to session
 */
TcOperStatus TcConfigOperations::
                      SendAdditionalResponse(TcOperStatus oper_stat) {
  if ( (tc_oper_ != TC_OP_CONFIG_RELEASE) &&
      oper_stat == TC_OPER_SUCCESS )  {
    TcOperStatus set_ret = SetConfigId();
    if ( set_ret != TC_OPER_SUCCESS ) {
      return set_ret;
    }
  }
  return oper_stat;
}

/*
 * @brief - Specific Dispatch for config request handling
 */
TcOperStatus TcConfigOperations::Dispatch() {
  TcOperStatus ret;
  tc_oper_status_ = INPUT_VALIDATION;

  pfc_log_info("tc_oper:Read Input Paramaters");
  ret = HandleArgs();
  if (ret != TC_OPER_SUCCESS) {
    return RevokeOperation(ret);
  }

  pfc_log_debug("TcConfigOperations::Dispatch oper = %d", tc_oper_);

  if (tc_oper_ == TC_OP_CONFIG_ACQUIRE &&
      !IsConfigReqQueueEmpty()) {
    return RevokeOperation(TC_SYSTEM_BUSY);
  }

  if (tc_oper_ == TC_OP_CONFIG_ACQUIRE_TIMED) {
    if(ssess_->setTimeout(NULL) != 0) {
      pfc_log_error("Error setting infinite timeout for ssess_");
    }
    ret = HandleTimedConfigAcquisition();  // Waits for timer / signal
    if (ret != TC_OPER_SUCCESS) {
      return RevokeOperation(ret);
    }
  }

  if (tc_oper_ != TC_OP_CONFIG_ACQUIRE_TIMED) {
    pfc_log_info("tc_oper:Secure Exclusion");
    tc_oper_status_ = GET_EXCLUSION_PHASE;
    ret = TcGetExclusion();
    if ( ret != TC_OPER_SUCCESS ) {
      return RevokeOperation(ret);
    }

    // Config mode is acquired; set availability to FALSE
    if(tc_oper_ == TC_OP_CONFIG_ACQUIRE ||
       tc_oper_ == TC_OP_CONFIG_ACQUIRE_FORCE) {
      SetConfigModeAvailability(PFC_FALSE);
    }
  }

  pfc_log_info("tc_oper:Accumulate Message List");
  tc_oper_status_ = CREATE_MSG_LIST;
  ret = TcCreateMsgList();
  if ( ret != TC_OPER_SUCCESS ) {
    return RevokeOperation(ret);
  }

  pfc_log_info("tc_oper:Execute Message List");
  tc_oper_status_ = EXECUTE_PHASE;
  ret = Execute();
  if ( ret != TC_OPER_SUCCESS ) {
    return RevokeOperation(ret);
  }

  pfc_log_info("tc_oper:Release Exclusion");
  tc_oper_status_ = RELEASE_EXCLUSION_PHASE;
  ret = TcReleaseExclusion();
  if ( ret != TC_OPER_SUCCESS ) {
    return RevokeOperation(ret);
  }

  if (tc_oper_ == TC_OP_CONFIG_RELEASE) {
    HandleConfigRelease();
  }

  pfc_log_info("tc_oper:Send Response to user");
  return SendResponse(TC_OPER_SUCCESS);
}

/* @brief Handler for Timed config mode acquisition requests */
TcOperStatus TcConfigOperations::HandleTimedConfigAcquisition() {
  pfc_log_debug("TcConfigOperations::HandleTimedConfigAcquisition");

  pfc::core::ScopedMutex m(config_req_queue_lock_);

  int32_t pthread_ret = 0;
  TcOperStatus ret = TC_OPER_SUCCESS;
  pfc_bool_t is_infinite = PFC_FALSE;
  pfc_timespec_t  timeout;

  if (timeout_ == 0) {
    pfc_log_debug("sess[%u] with timeout_ zero", session_id_);
    if (IsConfigModeAvailable()) {
      pfc_log_debug("Config-mode available - acquiring it...");
      pfc_log_info("tc_oper:Secure Exclusion");
      tc_oper_status_ = GET_EXCLUSION_PHASE;
      ret = TcGetExclusion();
      if ( ret != TC_OPER_SUCCESS ) {
        return ret;
      }

      // Config mode is acquired; set availability to FALSE
      SetConfigModeAvailability(PFC_FALSE);
      return TC_OPER_SUCCESS;
    } else {
      pfc_log_debug("Config mode not available - return busy");
      return TC_SYSTEM_BUSY;
    }

  } else if (timeout_ > 0) {
    pfc_log_debug("Positive timeout:%u", timeout_);

    struct timeval now;
    gettimeofday(&now, NULL);

    uint32_t millisec = 1000;
    uint64_t milli_to_nano = 1000000;

    timeout.tv_sec  = timeout_ / millisec;
    timeout.tv_nsec = (timeout_ % millisec) * milli_to_nano;
    pfc_log_info("Starting timer with %ld sec, %ld nsec",
                 timeout.tv_sec, timeout.tv_nsec);

    timeout.tv_sec += now.tv_sec;
    timeout.tv_nsec += now.tv_usec * millisec;

    timeout.tv_sec += timeout.tv_nsec / 1000000000L;
    timeout.tv_nsec = timeout.tv_nsec % 1000000000L;
    
  } else {
    is_infinite = PFC_TRUE;
    pfc_log_info("Timeout infinite");
  }

  cond_var_ = new pthread_cond_t;
  pthread_cond_init(cond_var_, NULL);

  // Push into wait queue
  InsertConfigRequest();

  // if timeout_ < 0 (infinite wait), do not start timer. Wait infinitely for
  // config-mode-availability
  // cond_wait   // Timer expiry or config-release triggers cond_signal
  if(!IsConfigModeAvailable() || !IsConfigAcquireAllowed()) {
    pthread_mutex_lock(&mutex_var_);
    pfc_log_info("Waiting for config mode; sess[%d]", session_id_); 

    m.unlock();

    if (is_infinite) {
      pthread_ret = pthread_cond_wait(cond_var_, &mutex_var_);
    } else {
      pthread_ret = pthread_cond_timedwait(cond_var_, &mutex_var_, &timeout);
    }

    pthread_mutex_unlock(&mutex_var_);
    pfc_log_info("pthread_cond_wait return %d", pthread_ret);
    if (pthread_ret != 0) {
      pfc::core::ScopedMutex rem_entry(config_req_queue_lock_);
      RemoveConfigRequest(session_id_);
      if (cond_var_) {
        delete cond_var_;
        cond_var_ = NULL;
      }
      rem_entry.unlock();
      return TC_SYSTEM_BUSY;
    }

  } else {
    pfc_log_info("Config mode available. Not waiting...");
  }

  if (IsStateChangedToSby()) {
    pfc_log_info("Sess[%u] While waiting for config acquisition state changed",
                 session_id_);
    pfc::core::ScopedMutex del_cond(config_req_queue_lock_);
    if (cond_var_) {
      delete cond_var_;
      cond_var_ = NULL;
    }
    del_cond.unlock();

    return TC_STATE_CHANGED;
  }

  m.unlock(); //Unlock if any lock pending

  pfc::core::ScopedMutex m2(config_req_queue_lock_); // Acquire fresh lock

  if (cond_var_) {
    delete cond_var_;
    cond_var_ = NULL;
  }

  pfc_log_info("Sess[%u] Wait complete. Acquiring config mode", session_id_);

  pfc_log_info("tc_oper:Secure Exclusion");
  tc_oper_status_ = GET_EXCLUSION_PHASE;
  ret = TcGetExclusion();
  if ( ret != TC_OPER_SUCCESS ) {
    RemoveConfigRequest(session_id_);   //For some reason if GetLock failed
    return ret;
  }

  // Config mode is acquired; set availability to FALSE
  SetConfigModeAvailability(PFC_FALSE);

  // Pop the first waiting timed req (currently processed)
  RemoveConfigRequest(session_id_);

  return TC_OPER_SUCCESS;
}

/**/
void TcConfigOperations::HandleConfigRelease() {
  pfc_log_debug("HandleConfigRelease");

  pfc::core::ScopedMutex m(config_req_queue_lock_);

  SetConfigModeAvailability(PFC_TRUE);

  // Get next item from wait queue.
  TcConfigOperations * tcop = RetrieveConfigRequest();
  if (tcop) {

    // Get condition variable and signal it.
    if (tcop->cond_var_) {
      pfc_log_info("Signal first waiting timed_config_acquire request");
      int ret = pthread_cond_signal(tcop->cond_var_);
      if (ret != 0) {
        pfc_log_error("Error signalling condition for session_id: %u",
                      tcop->session_id_);
      }
    } else {
      pfc_log_error("%s cond_var_ is NULL", __FUNCTION__);
    }
  } else {
    pfc_log_info("Config_mode_acq queue is empty");
  }
}
}  // namespace tc
}  // namespace unc
