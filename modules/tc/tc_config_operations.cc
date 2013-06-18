/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */


#include <tc_operations.hh>


namespace unc {
namespace tc {

#define UNC_CONFIG_OPER_ARG_COUNT_MIN  2
#define UNC_CONFIG_OPER_ARG_COUNT_MAX  3
#define UNC_CONFIG_ACQUIRE_ARG_COUNT 2
#define UNC_CONFIG_RELEASE_ARG_COUNT 3


TcConfigOperations::TcConfigOperations(TcLock* tc_lock_,
                           pfc::core::ipc::ServerSession* sess_,
                           TcDbHandler* db_handler,
                           TcChannelNameMap& unc_map_):
    TcOperations(tc_lock_, sess_, db_handler, unc_map_) , config_id_(0) {}

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
  pfc_log_info("tc_config_oper: Handle return from tc_lock");
  switch ( lock_ret ) {
    case     TC_LOCK_INVALID_UNC_STATE:
    case TC_LOCK_OPERATION_NOT_ALLOWED:
      return TC_INVALID_STATE;
    case TC_LOCK_BUSY:
      return TC_SYSTEM_BUSY;
    case TC_LOCK_INVALID_SESSION_ID:
      return TC_INVALID_SESSION_ID;
    case TC_LOCK_INVALID_CONFIG_ID:
      return TC_INVALID_CONFIG_ID;
    case TC_LOCK_NOT_ACQUIRED:
    case TC_LOCK_NO_CONFIG_SESSION_EXIST:
      return TC_CONFIG_NOT_PRESENT;
    default:
      return TC_OPER_FAILURE;
  }
  return TC_OPER_FAILURE;
}

/**
 *  @brief Check the number of input arguments for the operation
 */
TcOperStatus TcConfigOperations::TcCheckOperArgCount(uint32_t avail_count) {
  pfc_log_info("tc_config_oper: Check count of Input Arguments");
  if ( tc_oper_ == TC_OP_CONFIG_ACQUIRE ||
        tc_oper_ == TC_OP_CONFIG_ACQUIRE_FORCE ) {
    if ( avail_count != UNC_CONFIG_ACQUIRE_ARG_COUNT )
      return TC_OPER_INVALID_INPUT;
  } else if ( tc_oper_ == TC_OP_CONFIG_RELEASE ) {
    if ( avail_count != UNC_CONFIG_RELEASE_ARG_COUNT )
      return TC_OPER_INVALID_INPUT;
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
    return TC_INVALID_OPERATION_TYPE;
  }
  return TC_OPER_SUCCESS;
}

/**
 *  @brief Validate the operation paramaters for the service 
 */
TcOperStatus TcConfigOperations::TcValidateOperParams() {
  pfc_log_info("tc_config_oper: Validate Oper Params");
  if ( tc_oper_ != TC_OP_CONFIG_RELEASE ) {
    return TC_OPER_SUCCESS;
  }
  TcUtilRet ret(TcServerSessionUtils::get_uint32(ssess_,
                                             TC_REQ_ARG_INDEX, &config_id_));
  if ( ret == TCUTIL_RET_FAILURE ) {
    return TC_OPER_INVALID_INPUT;
  } else if ( ret == TCUTIL_RET_FATAL ) {
    return TC_OPER_FAILURE;
  }


  uint32_t validate_session_id = 0, validate_config_id = 0;
  TcLockRet LockRet=
      tclock_->GetConfigIdSessionId(&validate_session_id,
                                      &validate_config_id);
  if (LockRet != TC_LOCK_SUCCESS) {
    return HandleLockRet(LockRet);
  }
  if ( validate_session_id != session_id_ ) {
    return TC_INVALID_SESSION_ID;
  } else if ( validate_config_id != config_id_ ) {
      return TC_INVALID_CONFIG_ID;
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
    if (tc_oper_ == TC_OP_CONFIG_ACQUIRE) {
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
}  // namespace tc
}  // namespace unc
