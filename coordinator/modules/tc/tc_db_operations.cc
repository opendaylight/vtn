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

/* Minimum number of arguments for DB operation*/
#define UNC_DB_OPER_ARG_COUNT_MIN  2
/* Maximum number of arguments for DB operation*/
#define UNC_DB_OPER_ARG_COUNT_MAX  2


/*
 * @brief Constructor
 */
TcDbOperations::TcDbOperations(TcLock* tc_lock_,
                           pfc::core::ipc::ServerSession* sess_,
                           TcDbHandler* db_handler,
                           TcChannelNameMap& unc_map_):
    TcOperations(tc_lock_, sess_, db_handler, unc_map_) {}

/*
 * @brief return minimum argument count for the operation
 */
uint32_t TcDbOperations::TcGetMinArgCount() {
  return UNC_DB_OPER_ARG_COUNT_MIN;
}

/*
 * @brief Convert TcLock return value to TcOperStatus
 */
TcOperStatus TcDbOperations::HandleLockRet(TcLockRet lock_ret) {
  switch ( lock_ret ) {
    case TC_LOCK_INVALID_UNC_STATE:
    case TC_LOCK_OPERATION_NOT_ALLOWED:
      return TC_INVALID_STATE;
    case TC_LOCK_BUSY:
      return TC_SYSTEM_BUSY;
    default:
      return TC_OPER_FAILURE;
  }
  return TC_OPER_FAILURE;
}

/*
 * @brief check argument count in the input
 */
TcOperStatus TcDbOperations::TcCheckOperArgCount(uint32_t avail_count) {
  if (avail_count != UNC_DB_OPER_ARG_COUNT_MIN)
    return TC_OPER_INVALID_INPUT;
  else
    return TC_OPER_SUCCESS;
}

/*
 * @brief Validate the opertype value in the input
 */
TcOperStatus TcDbOperations::TcValidateOperType() {
  if (tc_oper_ != TC_OP_RUNNING_SAVE &&
      tc_oper_ != TC_OP_CLEAR_STARTUP) {
    return TC_INVALID_OPERATION_TYPE;
  }
  return TC_OPER_SUCCESS;
}

/*
 * @brief Validate the oper params in the input
 */
TcOperStatus TcDbOperations::TcValidateOperParams() {
  return TC_OPER_SUCCESS;
}

/*
 * @brief Handle return value from TcMsg
 */
TcOperStatus TcDbOperations::HandleMsgRet(TcOperRet MsgRet) {
  if ( MsgRet == TCOPER_RET_FATAL ||
       MsgRet == TCOPER_RET_FAILURE ) {
    pfc_log_error("DB Operation failed session_id=%d", session_id_);
    return TC_SYSTEM_FAILURE;
  } else  if ( MsgRet == TCOPER_RET_ABORT ) {
    return TC_OPER_ABORT;
  }
  return TC_OPER_SUCCESS;
}

/*
 * @brief Get Exclsuion from TcLock
 */
TcOperStatus TcDbOperations::TcGetExclusion() {
  TcLockRet ret;
  if (tc_oper_ == TC_OP_RUNNING_SAVE) {
    ret = tclock_->GetLock(session_id_,
                          TC_ACQUIRE_WRITE_SESSION,
                          TC_SAVE_STARTUP_CONFIG);
  } else {
    ret = tclock_->GetLock(session_id_,
                          TC_ACQUIRE_WRITE_SESSION,
                          TC_CLEAR_STARTUP_CONFIG);
  }
  if (ret != TC_LOCK_SUCCESS) {
    return HandleLockRet(ret);
  }
  return TC_OPER_SUCCESS;
}

/*
 * @brief Release Exclsuion from TcLock
 */
TcOperStatus TcDbOperations::TcReleaseExclusion() {
  TcLockRet ret;
  if (tc_oper_ == TC_OP_RUNNING_SAVE) {
    ret = tclock_->ReleaseLock(session_id_,
                               0,
                               TC_RELEASE_WRITE_SESSION,
                               TC_SAVE_STARTUP_CONFIG);
  } else {
    ret = tclock_->ReleaseLock(session_id_,
                               0,
                               TC_RELEASE_WRITE_SESSION,
                               TC_CLEAR_STARTUP_CONFIG);
  }
  if (ret != TC_LOCK_SUCCESS) {
    return HandleLockRet(ret);
  }
  return TC_OPER_SUCCESS;
}

/*
 * @brief Message List for this operation
 */
TcOperStatus TcDbOperations::TcCreateMsgList() {
  /* Update DB for Save Operation */
  if (tc_oper_ == TC_OP_RUNNING_SAVE) {
    if (db_hdlr_->UpdateRecoveryTable(UNC_DT_STARTUP,
                                     TC_OP_RUNNING_SAVE) !=
                                     TCOPER_RET_SUCCESS) {
      return TC_OPER_FAILURE;
    }
  } else {
  /* Update DB for Clear Operation */
    if (db_hdlr_->UpdateRecoveryTable(UNC_DT_STARTUP,
                                     TC_OP_CLEAR_STARTUP)!=
                                     TCOPER_RET_SUCCESS) {
      return TC_OPER_FAILURE;
    }
  }
  if (TcOperMessageList. empty()) {
    if (tc_oper_ == TC_OP_RUNNING_SAVE) {
      TcOperMessageList.push_back(unc::tclib::MSG_SAVE_CONFIG);
    } else {
      TcOperMessageList.push_back(unc::tclib::MSG_CLEAR_CONFIG);
    }
    return TC_OPER_SUCCESS;
  }
  return TC_OPER_FAILURE;
}

/*
 * @brief Message Contents to be sent to tclib
 */
TcOperStatus TcDbOperations::FillTcMsgData(TcMsg* tc_msg,
                           TcMsgOperType oper_type) {
  return TC_OPER_SUCCESS;
}

/*
 * @brief Final Response Method
 */
TcOperStatus TcDbOperations::SendAdditionalResponse(TcOperStatus oper_stat) {
  if ( oper_stat == TC_OPER_SUCCESS ) {
    if (db_hdlr_->UpdateRecoveryTable(UNC_DT_INVALID,
                                     TC_OP_INVALID) != TCOPER_RET_SUCCESS) {
      return TC_OPER_FAILURE;
    }
    return TC_OPER_SUCCESS;
  }
  return oper_stat;
}

}  // namespace tc
}  // namespace unc
