/*
 * Copyright (c) 2012-2015 NEC Corporation
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
#define UNC_CONFIG_OPER_ARG_COUNT_MAX  4 
#define UNC_CONFIG_ACQUIRE_ARG_COUNT 2
#define UNC_CONFIG_RELEASE_ARG_COUNT 3
#define UNC_CONFIG_ACQUIRE_TIMED_ARG_COUNT 3
#define UNC_CONFIG_ACQUIRE_PARTIAL_MIN_ARG_COUNT 3
#define UNC_CONFIG_ACQUIRE_PARTIAL_MAX_ARG_COUNT 4


std::deque<TcConfigOperations*> TcConfigOperations::config_req_queue_;
pfc::core::Mutex TcConfigOperations::config_req_queue_lock_;

TcConfigOperations::TcConfigOperations(TcLock* tc_lock_,
                           pfc::core::ipc::ServerSession* sess_,
                           TcDbHandler* db_handler,
                           TcChannelNameMap& unc_map_):
    TcOperations(tc_lock_, sess_, db_handler, unc_map_) {
  config_id_ = 0;
  timeout_ = 0;
  tc_mode_ = TC_CONFIG_INVALID;
  pthread_cond_init(&cond_var_, NULL);
  pthread_mutex_init(&mutex_var_, NULL);
}

TcConfigOperations::~TcConfigOperations() {
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
      pfc_log_debug("Found. Removing for sess[%u] done", sess_id);
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


void TcConfigOperations::ClearConfigAcquisitionQueue() {
  pfc_log_debug("TcConfigOperations::ClearConfigAcquisitionQueue");
  pfc::core::ScopedMutex m(config_req_queue_lock_);
  while (!config_req_queue_.empty()) {

    TcConfigOperations * configoper = config_req_queue_.front();
    if(configoper) {
      pthread_mutex_lock(&configoper->mutex_var_);
      pfc_log_info("ClearConfigAcquisitionQueue processing sess[%u]",
                   configoper->session_id_);
      // Call cond_signal
      int32_t ret = pthread_cond_signal(&configoper->cond_var_);
      pthread_mutex_unlock(&configoper->mutex_var_);
      if (ret != 0) {
        pfc_log_error("%s Error signalling condition for session_id: %u",
                      __FUNCTION__, configoper->session_id_);
      }
    }
    config_req_queue_.pop_front();
  }
}


/**
 *  @brief Write Config ID to Output session
 */
TcOperStatus TcConfigOperations::SetConfigId() {
  pfc_log_debug("tc_config_oper: Setting configId to response");
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
  pfc_log_trace("tc_config_oper: Check count of Input Arguments");
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
          pfc_log_error("TcCheckOperArgCount aqtime args expected(%u) \
            recvd(%u)", UNC_CONFIG_ACQUIRE_TIMED_ARG_COUNT, avail_count);
      return TC_OPER_INVALID_INPUT;
    }
  } else if (tc_oper_== TC_OP_CONFIG_ACQUIRE_PARTIAL) { 
    if (avail_count < UNC_CONFIG_ACQUIRE_PARTIAL_MIN_ARG_COUNT ||
        avail_count > UNC_CONFIG_ACQUIRE_PARTIAL_MAX_ARG_COUNT) {
      pfc_log_error("TcCheckOperArgCount aqpart args expected(%u)or(%u)"
                    " recvd(%u)", UNC_CONFIG_ACQUIRE_PARTIAL_MIN_ARG_COUNT,
                    UNC_CONFIG_ACQUIRE_PARTIAL_MAX_ARG_COUNT, avail_count);
      return TC_OPER_INVALID_INPUT;
    }
  }
  return TC_OPER_SUCCESS;
}

/**
 *  @brief Validate the operation type from input
 */
TcOperStatus TcConfigOperations::TcValidateOperType() {
  pfc_log_trace("tc_config_oper: Validate Oper Type");
  if (tc_oper_ < TC_OP_CONFIG_ACQUIRE ||
      tc_oper_ > TC_OP_CONFIG_ACQUIRE_FORCE) {
    pfc_log_error("TcValidateOperType opertype < TC_OP_CONFIG_ACQUIRE or"
                  " > TC_OP_CONFIG_ACQUIRE_FORCE");
    return TC_INVALID_OPERATION_TYPE;
  }

  if (tc_oper_ == TC_OP_CONFIG_ACQUIRE_TIMED) {
    if(ssess_ && ssess_->setTimeout(NULL) != 0) {
      pfc_log_error("Error setting infinite timeout for ssess_");
    }
  }

  return TC_OPER_SUCCESS;
}

/**
 *  @brief Validate the operation paramaters for the service
 */
TcOperStatus TcConfigOperations::TcValidateOperParams() {
  pfc_log_trace("tc_config_oper: Validate Oper Params");

  if (tc_oper_ == TC_OP_CONFIG_RELEASE) {
    TcUtilRet ret(TcServerSessionUtils::get_uint32(ssess_,
                                           TC_REQ_ARG_INDEX, &config_id_));
    if ( ret == TCUTIL_RET_FAILURE ) {
      pfc_log_error("TcValidateOperParams: getting config_id failed");
      return TC_OPER_INVALID_INPUT;
    } else if ( ret == TCUTIL_RET_FATAL ) {
      pfc_log_error("TcValidateOperParams: getting config_id fatal");
      return TC_OPER_FAILURE;
    }

    uint32_t validate_config_id = 0;
    ::TcConfigMode validate_config_mode;
    std::string validate_vtn_name;
    TcLockRet LockRet = tclock_->GetConfigData(session_id_,
                                               validate_config_id,
                                               validate_config_mode,
                                               validate_vtn_name);
    if (LockRet != TC_LOCK_SUCCESS) {
      pfc_log_error("%s Cannot get ConfigData info", __FUNCTION__);
      return HandleLockRet(LockRet);
    }
    if ( validate_config_id != config_id_ ) {
      pfc_log_error("TcValidateOperParams: config_id(%u) != expected (%u)",
                    config_id_, validate_config_id);
       return TC_INVALID_CONFIG_ID;
    }
    pfc_log_debug("TC_OP_CONFIG_RELEASE config_id:%d", config_id_);
  } else if (tc_oper_ == TC_OP_CONFIG_ACQUIRE) {
    tc_mode_ = TC_CONFIG_GLOBAL;
    pfc_log_info("TC_OP_CONFIG_ACQUIRE default mode : TC_CONFIG_GLOBAL");
  } else if (tc_oper_ == TC_OP_CONFIG_ACQUIRE_FORCE) {
    tc_mode_ = TC_CONFIG_GLOBAL;
    pfc_log_info("TC_OP_CONFIG_ACQUIRE_FORCE default mode : TC_CONFIG_GLOBAL");
  } else if (tc_oper_ == TC_OP_CONFIG_ACQUIRE_TIMED) {
    tc_mode_ = TC_CONFIG_GLOBAL; // Timed config is always global mode
    pfc_log_info("TC_OP_CONFIG_ACQUIRE_TIMED default mode : TC_CONFIG_GLOBAL");
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
  } else if (tc_oper_ == TC_OP_CONFIG_ACQUIRE_PARTIAL) {
   
    uint32_t config_mode;
    TcUtilRet ret(TcServerSessionUtils::get_uint32(ssess_,
                                        TC_REQ_ARG_INDEX,
                                          &config_mode));
    if (ret == TCUTIL_RET_FAILURE ) {
      pfc_log_error("TcValidateOperParams: Reading tc_mode_ failed");
      return TC_OPER_INVALID_INPUT;
    } else if ( ret == TCUTIL_RET_FATAL ) {
      pfc_log_error("TcValidateOperParams: Reading tc_mode fatal");
      return TC_OPER_FAILURE;
    }
    tc_mode_ = (TcConfigMode)config_mode; 
    if (tc_mode_ == TC_CONFIG_GLOBAL) {
      pfc_log_info("TC_OP_CONFIG_ACQUIRE_PARTIAL Mode : "
                    "TC_CONFIG_GLOBAL");
    } else if (tc_mode_ == TC_CONFIG_REAL) {
      pfc_log_info("TC_OP_CONFIG_ACQUIRE_PARTIAL Mode : "
                    "TC_CONFIG_REAL");
    } else if (tc_mode_ == TC_CONFIG_VIRTUAL) {
      pfc_log_info("TC_OP_CONFIG_ACQUIRE_PARTIAL Mode : "
                    "TC_CONFIG_VIRTUAL");
    } else if (tc_mode_ == TC_CONFIG_VTN) {
      std::string vtn_name;
      TcUtilRet ret(TcServerSessionUtils::get_string(ssess_,
                                              TC_REQ_ARG_INDEX+1,
                                              vtn_name));
      if (ret == TCUTIL_RET_FAILURE ) {
        pfc_log_error("TcValidateOperParams: Reading vtn-name value failed");
        return TC_OPER_INVALID_INPUT;
      } else if ( ret == TCUTIL_RET_FATAL ) {
        pfc_log_error("TcValidateOperParams: Reading vtn-name value fatal");
        return TC_OPER_FAILURE;
      }
      if (vtn_name.empty()) {
        pfc_log_error("TcValidateOperParams: vtn_name empty");
        return TC_OPER_INVALID_INPUT;
      }
      vtn_name_ = vtn_name;
      pfc_log_info("TC_OP_CONFIG_ACQUIRE_PARTIAL Mode : "
                   "TC_CONFIG_VTN => vtn-name: %s", vtn_name_.c_str());
    } else {
      pfc_log_error("%s Invalid config mode: %u", __FUNCTION__, config_mode);
      return TC_OPER_INVALID_INPUT;
    }
  }
  return TC_OPER_SUCCESS;
}

/**
 *  @brief Secure Exclusion for the operation
 */
TcOperStatus TcConfigOperations::TcGetExclusion() {
  TcLockRet ret;
  TcOperStatus val_global_mode = ValidateGlobalModeDirty();
  if (val_global_mode != TC_OPER_SUCCESS) {
    return val_global_mode;
  }

  if ( tc_oper_ == TC_OP_CONFIG_RELEASE ) {
    ret= tclock_->ReleaseLock(session_id_,
                              config_id_,
                              TC_RELEASE_CONFIG_SESSION,
                              TC_WRITE_NONE);
    if (ret != TC_LOCK_SUCCESS) {
      return HandleLockRet(ret);
    }
  } else {
    if (tc_oper_ == TC_OP_CONFIG_ACQUIRE ||
        tc_oper_ == TC_OP_CONFIG_ACQUIRE_TIMED ||
        tc_oper_ == TC_OP_CONFIG_ACQUIRE_PARTIAL) {
      ret= tclock_->GetLock(session_id_,
                           TC_ACQUIRE_CONFIG_SESSION,
                           TC_WRITE_NONE,
                           (::TcConfigMode)tc_mode_,
                           vtn_name_);      
    } else if (tc_oper_ == TC_OP_CONFIG_ACQUIRE_FORCE) {
      ret= tclock_->GetLock(session_id_,
                           TC_FORCE_ACQUIRE_CONFIG_SESSION,
                           TC_WRITE_NONE);
    } else {
      pfc_log_error("%s Invalid operation %d", __FUNCTION__, tc_oper_);
      return TC_INVALID_OPERATION_TYPE;
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
  if (TcOperMessageList.empty()) {
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
  TcConfigMode config_mode = TC_CONFIG_INVALID;
  std::string vtn_name;

  if ( tc_oper_ != TC_OP_CONFIG_RELEASE ) {
    TcLockRet lock_ret = tclock_->GetConfigData(session_id_,
                                                config_id_,
                                                config_mode,
                                                vtn_name);
    if (lock_ret != TC_LOCK_SUCCESS) {
      pfc_log_error("%s Cannot get config data for sess[%u]",
                    __FUNCTION__, session_id_);
      return TC_OPER_FAILURE;
    }

    tc_msg->SetData(config_id_,
                    config_mode,
                    vtn_name);
  } else {
    /*set config_id_=0 for TC_OP_CONFIG_RELEASE*/
    tc_msg->SetData(0, config_mode, vtn_name);
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

    // If acquired config mode is global, set global_mode_dirty
    if (tc_mode_ == TC_CONFIG_GLOBAL) {
      if (db_hdlr_->UpdateRecoveryTableGlobalModeDirty(PFC_TRUE)
                                        != TCOPER_RET_SUCCESS) {
        pfc_log_fatal("%s Failed to update GlobalModeDirty", __FUNCTION__);
        return TC_SYSTEM_FAILURE;
      }
    }
  }

  return oper_stat;
}

/*
 * @brief - Specific Dispatch for config request handling
 */
TcOperStatus TcConfigOperations::Dispatch() {
  TcOperStatus ret = TC_OPER_SUCCESS;
  tc_oper_status_ = INPUT_VALIDATION;

  pfc_log_debug("tc_oper:Read Input Paramaters");
  ret = HandleArgs();
  if (ret != TC_OPER_SUCCESS) {
    return RevokeOperation(ret);
  }

  pfc_log_debug("TcConfigOperations::Dispatch oper = %d", tc_oper_);

  if ( (tc_oper_ == TC_OP_CONFIG_ACQUIRE || 
        tc_oper_ == TC_OP_CONFIG_ACQUIRE_PARTIAL) &&
      !IsConfigReqQueueEmpty()) {
    pfc_log_info("%s Received config/config-partial request;"
                 " but config-timed queue is not empty",
                 __FUNCTION__);
    return RevokeOperation(TC_SYSTEM_BUSY);
  }

  if (tc_oper_ == TC_OP_CONFIG_ACQUIRE_TIMED) {
    ret = HandleTimedConfigAcquisition();  // Waits for timer / signal
  } else {
    pfc_log_info("tc_oper:Secure Exclusion");
    tc_oper_status_ = GET_EXCLUSION_PHASE;
    ret = TcGetExclusion();
  }

  if ( ret != TC_OPER_SUCCESS ) {
    return RevokeOperation(ret);
  }

  pfc_log_debug("tc_oper:Accumulate Message List");
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
  tc_config_name_map = tclock_->GetConfigMap();
  if (tc_oper_ == TC_OP_CONFIG_RELEASE &&
      tc_config_name_map.empty())  {
    HandleConfigRelease();
  }

  pfc_log_debug("tc_oper:Send Response to user");
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
    if (!config_req_queue_.empty()) {
      pfc_log_info("%s Config wait queue not empty. Return BUSY", __FUNCTION__);
      return TC_SYSTEM_BUSY;
    }

    pfc_log_info("tc_oper:Secure Exclusion");
    tc_oper_status_ = GET_EXCLUSION_PHASE;
    ret = TcGetExclusion();
    if ( ret != TC_OPER_SUCCESS ) {
      return ret;
    }

    pfc_log_info("%s Config mode available. Acquiring it.", __FUNCTION__);
    return TC_OPER_SUCCESS;

  } else if (timeout_ > 0) {
    pfc_log_debug("Positive timeout:%u", timeout_);

    struct timeval now;
    gettimeofday(&now, NULL);

    uint32_t millisec = 1000;
    uint64_t milli_to_nano = 1000000;

    timeout.tv_sec  = timeout_ / millisec;
    timeout.tv_nsec = (timeout_ % millisec) * milli_to_nano;
    pfc_log_debug("Starting timer with %ld sec, %ld nsec",
                 timeout.tv_sec, timeout.tv_nsec);

    timeout.tv_sec += now.tv_sec;
    timeout.tv_nsec += now.tv_usec * millisec;

    timeout.tv_sec += timeout.tv_nsec / 1000000000L;
    timeout.tv_nsec = timeout.tv_nsec % 1000000000L;
    
  } else {
    is_infinite = PFC_TRUE;
    pfc_log_info("Timeout infinite");
  }

  // If config wait queue is empty, try GetExclusion.
  // If success, return without inserting into queue.
  if (config_req_queue_.empty()) {
    pfc_log_info("tc_oper:Trying Exclusion");
    tc_oper_status_ = GET_EXCLUSION_PHASE;
    ret = TcGetExclusion();
    if ( ret == TC_OPER_SUCCESS ) {
      pfc_log_info("%s Queue empty && GetLock successful. Not waiting",
                   __FUNCTION__);
      return ret;
    } else {
      tc_oper_status_ = INPUT_VALIDATION; // Revert back oper_status
      pfc_log_info("%s Cannot acquire lock. Already config mode alloted? "
                   "Moving to wait mode", __FUNCTION__);
    }
  }

  // Push into wait queue
  InsertConfigRequest();

  // if timeout_ < 0 (infinite wait), do not start timer. Wait infinitely for
  // config-mode-availability
  // cond_wait   // Timer expiry or config-release triggers cond_signal
  pthread_mutex_lock(&mutex_var_);
  pfc_log_info("Waiting for config mode; sess[%u]", session_id_); 

  m.unlock();

  if (is_infinite) {
    pthread_ret = pthread_cond_wait(&cond_var_, &mutex_var_);
  } else {
    pthread_ret = pthread_cond_timedwait(&cond_var_, &mutex_var_, &timeout);
  }

  pthread_mutex_unlock(&mutex_var_);

  pfc_log_info("pthread_cond_wait [%u] return %d", session_id_, pthread_ret);
  if (pthread_ret != 0) {
    pfc::core::ScopedMutex rem_entry(config_req_queue_lock_);
    RemoveConfigRequest(session_id_);
    rem_entry.unlock();
    return TC_SYSTEM_BUSY;
  }


  if (TcOperations::IsStateChangedToSby()) {
    pfc_log_info("Sess[%u] While waiting for config acquisition state changed",
                 session_id_);
    return TC_STATE_CHANGED;
  }

  m.unlock(); //Unlock if any lock pending

  pfc::core::ScopedMutex m2(config_req_queue_lock_); // Acquire fresh lock


  pfc_log_info("tc_oper:Secure Exclusion");
  tc_oper_status_ = GET_EXCLUSION_PHASE;
  ret = TcGetExclusion();

  // Pop the first waiting timed req (currently processed)
  RemoveConfigRequest(session_id_); // Remove entry for both success & failure

  return ret;
}

/**/
void TcConfigOperations::HandleConfigRelease() {
  pfc_log_debug("HandleConfigRelease");

  pfc::core::ScopedMutex m(config_req_queue_lock_);

  // Get next item from wait queue.
  TcConfigOperations * tcop = RetrieveConfigRequest();
  if (tcop) {
    // Get condition variable and signal it.
    pthread_mutex_lock(&tcop->mutex_var_);
    pfc_log_info("Signal first waiting timed_config_acquire request[%u]",
                 tcop->session_id_);
    int ret = pthread_cond_signal(&tcop->cond_var_);
    pthread_mutex_unlock(&tcop->mutex_var_);
    if (ret != 0) {
      pfc_log_error("Error signalling condition for session_id: %u",
                    tcop->session_id_);
    }
  } else {
    pfc_log_info("Config_mode_acq queue is empty");
  }
}

/* @info - ValidateGlobalModeDirty
 */
TcOperStatus TcConfigOperations::ValidateGlobalModeDirty() {
  if (tc_oper_ == TC_OP_CONFIG_RELEASE) {
    
    uint32_t config_id = 0;
    ::TcConfigMode config_mode;
    std::string vtn_name;
    tclock_->GetConfigData(session_id_,
                           config_id,
                           config_mode,
                           vtn_name);
    if (config_mode != TC_CONFIG_GLOBAL) {
      return TC_OPER_SUCCESS;
    }

    if (IsCandidateDirty(session_id_, config_id_) == PFC_FALSE) {
      pfc_log_info("%s Candidate not dirty during config release. "
                   "Reseting global_mode_dirty = FALSE", __FUNCTION__);
      if (db_hdlr_->UpdateRecoveryTableGlobalModeDirty(PFC_FALSE)
                                                    != TCOPER_RET_SUCCESS) {
        pfc_log_fatal("%s Failed to update GlobalModeDirty ", __FUNCTION__);
        return TC_SYSTEM_FAILURE;
      }
    } else {
      pfc_log_info("%s CandidateDB dirty in upll/uppl. "
                   "Not reseting global_mode_dirty flag", __FUNCTION__);
    }
  } else if (tc_oper_ == TC_OP_CONFIG_ACQUIRE_PARTIAL) {
    pfc_bool_t global_mode_dirty = PFC_FALSE;
    if (db_hdlr_->GetRecoveryTableGlobalModeDirty(global_mode_dirty)
                                          != TCOPER_RET_SUCCESS) {
      pfc_log_error("%s Failed to fetch Global Mode Dirty", __FUNCTION__);
      return TC_SYSTEM_FAILURE;
    }

    if (global_mode_dirty == PFC_TRUE &&
        tc_mode_ != TC_CONFIG_GLOBAL) {
      pfc_log_error("%s Global Mode dirty; Partial configuration not allowed", 
                    __FUNCTION__);
      return TC_SYSTEM_BUSY;
    } else {
      pfc_log_debug("%s Global_mode not dirty; Partial config request allowed.",
                   __FUNCTION__);
    }
  }

  return TC_OPER_SUCCESS;
}

}  // namespace tc
}  // namespace unc
