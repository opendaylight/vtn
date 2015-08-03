/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */


#include <tc_operations.hh>


namespace unc {
namespace tc {

/* Minimum number of aguments in Read request */
#define UNC_READ_OPER_ARG_COUNT_MIN  2
/* Maximum number of aguments in Read request */
#define UNC_READ_OPER_ARG_COUNT_MAX  3
/* Number of aguments in Acquire Read request */
#define UNC_READ_ACQUIRE_ARG_COUNT 3
/* Number of aguments in Release Read request */
#define UNC_READ_RELEASE_ARG_COUNT 2


/*
 * @brief Constructor
 */
TcReadOperations::TcReadOperations(TcLock* tc_lock_,
                           pfc::core::ipc::ServerSession* sess_,
                           TcDbHandler* db_handler,
                           TcChannelNameMap& unc_map_,
                           TcTaskqUtil* readq_)
    :TcOperations(tc_lock_, sess_, db_handler, unc_map_),
     read_handle_(readq_),
     arg_timeout_(PFC_FALSE),
     timeout_(0) {}


/*
 * @brief Return Minumim argument Count
 */
uint32_t TcReadOperations::TcGetMinArgCount() {
  return UNC_READ_OPER_ARG_COUNT_MIN;
}

/*
 * @brief Convert return from TcLock to TcOperStatus
 */
TcOperStatus TcReadOperations::HandleLockRet(TcLockRet lock_ret) {
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
    case TC_LOCK_ALREADY_ACQUIRED:
      ret = TC_SESSION_ALREADY_ACTIVE;
      break;
    case TC_LOCK_NOT_ACQUIRED:
      ret = TC_SESSION_NOT_ACTIVE;
      break;
    default:
      ret = TC_OPER_FAILURE;
  }
  pfc_log_info("HandleLockRet: Received(%u), Return(%u)",
               lock_ret, ret);
  return ret;
}

/*
 * @brief Check if the count of arguments is valid
 */
TcOperStatus TcReadOperations::TcCheckOperArgCount(uint32_t avail_count) {
  if ( tc_oper_ == TC_OP_READ_ACQUIRE ) {
    if ( avail_count > UNC_READ_OPER_ARG_COUNT_MAX ) {
      pfc_log_error("TcCheckOperArgCount args recvd(%u) > expected(%u)",
                    avail_count, UNC_READ_OPER_ARG_COUNT_MAX);
      return TC_OPER_INVALID_INPUT;
    } else if ( avail_count == UNC_READ_OPER_ARG_COUNT_MAX ) {
      arg_timeout_ = PFC_TRUE;
    }
  } else if ( tc_oper_ == TC_OP_READ_RELEASE ) {
    if ( avail_count != UNC_READ_RELEASE_ARG_COUNT ) {
      pfc_log_error("TcCheckOperArgCount: args expected(%u) received(%u)",
                    UNC_READ_RELEASE_ARG_COUNT, avail_count);
      return TC_OPER_INVALID_INPUT;
    }
  }
  return TC_OPER_SUCCESS;
}

/*
 * @brief Check if the input operation type is valid
 */
TcOperStatus TcReadOperations::TcValidateOperType() {
  if (tc_oper_ != TC_OP_READ_ACQUIRE &&
      tc_oper_ != TC_OP_READ_RELEASE) {
    pfc_log_error("TcValidateOperType opertype != TC_OP_READ_ACQUIRE or "
                  "TC_OP_READ_RELEASE");
    return TC_INVALID_OPERATION_TYPE;
  }
  return TC_OPER_SUCCESS;
}

/*
 * @brief Check if the input paramaters are valid
 */
TcOperStatus TcReadOperations::TcValidateOperParams() {
  if ( (tc_oper_ != TC_OP_READ_ACQUIRE) ||
      (arg_timeout_  != PFC_TRUE) ) {
    pfc_log_info("Release Read Lock - session_id:%d", session_id_);
    return TC_OPER_SUCCESS;
  }
  TcUtilRet ret=
      TcServerSessionUtils::get_uint32(ssess_,
                                       TC_REQ_ARG_INDEX, &timeout_);
  if ( ret == TCUTIL_RET_FAILURE ) {
    pfc_log_error("TcValidateOperParams Reading timouet value failed");
    return TC_OPER_INVALID_INPUT;
  } else if ( ret == TCUTIL_RET_FATAL ) {
    pfc_log_error("TcValidateOperParams Reading timouet value fatal");
    return TC_OPER_FAILURE;
  }
  if ((tc_oper_ == TC_OP_READ_ACQUIRE) &&
      (timeout_ > 0) &&
     (read_handle_ == NULL)) {
    pfc_log_error("TcValidateOperParams READ_AQ timeout>0; read_handle NULL");
    return TC_OPER_FAILURE;
  }
  pfc_log_info("Acquire Read Lock - session_id:%d timeout:%d",
               session_id_, timeout_);
  return TC_OPER_SUCCESS;
}

/*
 * @brief Secure Exclusion for the operation
 */
TcOperStatus TcReadOperations::TcGetExclusion() {
  TcLockRet ret = TC_LOCK_FAILURE;
  if ( tc_oper_ == TC_OP_READ_RELEASE ) {
    ret= tclock_->ReleaseLock(session_id_,
                              0,
                              TC_RELEASE_READ_SESSION,
                              TC_WRITE_NONE);
    if ( ret != TC_LOCK_SUCCESS ) {
      return HandleLockRet(ret);
    }
  } else {
    if ( tc_oper_ == TC_OP_READ_ACQUIRE ) {
      ret= tclock_->GetLock(session_id_,
                            TC_ACQUIRE_READ_SESSION,
                            TC_WRITE_NONE);
    }
    if ( ret != TC_LOCK_SUCCESS ) {
      return HandleLockRet(ret);
    }
  }
  return TC_OPER_SUCCESS;
}

/*
 * @brief Release Exclusion after operation
 */
TcOperStatus TcReadOperations::TcReleaseExclusion() {
  if (tc_oper_ == TC_OP_READ_RELEASE) {
    if (tclock_->CanWriteLock()) {
      TcCandidateOperations::HandleCandidateRelease();
    }
  }
  return TC_OPER_SUCCESS;
}


/*
 * @brief Message List for this operation (None)
 */
TcOperStatus TcReadOperations::TcCreateMsgList() {
  return TC_OPER_SUCCESS;
}

/*
 * @brief Fill data to be sent to tclib
 */
TcOperStatus TcReadOperations::FillTcMsgData(TcMsg* tc_msg,
                           TcMsgOperType oper_type) {
  return TC_OPER_SUCCESS;
}

/*
 * @brief Final Send Response method
 */
TcOperStatus TcReadOperations::SendAdditionalResponse(TcOperStatus oper_stat) {
  return oper_stat;
}

/*
 * @brief Execute Read Operation
 */
TcOperStatus TcReadOperations::Execute() {
  if ( timeout_ == 0 ||
        tc_oper_ == TC_OP_READ_RELEASE ) {
    return TC_OPER_SUCCESS;
  }
  /* Create read release for taskq and dispatch */
  if (read_handle_->PostReadTimer(session_id_,
                                  timeout_,
                                  tclock_,
                                  unc_oper_channel_map_) != 0) {
    return TC_OPER_FAILURE;
  }
  return TC_OPER_SUCCESS;
}

}  // namespace tc
}  // namespace unc
