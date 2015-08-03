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
  TcOperStatus ret = TC_OPER_FAILURE;

  switch ( lock_ret ) {
    case TC_LOCK_INVALID_UNC_STATE:
    case TC_LOCK_OPERATION_NOT_ALLOWED:
      ret = TC_INVALID_STATE;
      break;
    case TC_LOCK_BUSY:
      ret = TC_SYSTEM_BUSY;
      break;
    default:
      ret = TC_OPER_FAILURE;
  }
  pfc_log_info("HandleLockRet: Received(%u) Return (%u)",
               lock_ret, ret);
  return ret;
}

/*
 * @brief check argument count in the input
 */
TcOperStatus TcDbOperations::TcCheckOperArgCount(uint32_t avail_count) {
  if (avail_count != UNC_DB_OPER_ARG_COUNT_MIN) {
    pfc_log_error("TcCheckOperArgCount: args expected(%u) received(%u)",
                  UNC_DB_OPER_ARG_COUNT_MIN, avail_count);
    return TC_OPER_INVALID_INPUT;
  }
  else
    return TC_OPER_SUCCESS;
}

/*
 * @brief Validate the opertype value in the input
 */
TcOperStatus TcDbOperations::TcValidateOperType() {
  if (tc_oper_ != TC_OP_RUNNING_SAVE &&
      tc_oper_ != TC_OP_CLEAR_STARTUP) {
    pfc_log_error("TcValidateOperType opertype != TC_OP_RUNNING_SAVE or "
                  "TC_OP_CLEAR_STARTUP");
    return TC_INVALID_OPERATION_TYPE;
  }
  /*set infinite timeout for startup operations*/
  TcUtilRet ret = TcServerSessionUtils::set_srv_timeout(ssess_, NULL);
  if (ret == TCUTIL_RET_FAILURE) {
    return TC_OPER_FAILURE;
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

  // After release of current write operation,
  // trigger next waiting candidate operation
  TcCandidateOperations::HandleCandidateRelease();

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
  if (tc_msg == NULL) {
    return TC_OPER_FAILURE;
  }

  tc_msg->SetData(UNC_DT_STARTUP, TC_OP_RUNNING_SAVE, save_version_);

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

/*
 * @brief  Method to trigger operation 
 */
TcOperStatus TcDbOperations::Dispatch() {
  TcOperStatus ret;
  tc_oper_status_ = INPUT_VALIDATION;

  pfc_log_debug("tc_oper:Read Input Paramaters");
  ret = HandleArgs();
  if (ret != TC_OPER_SUCCESS) {
    return RevokeOperation(ret);
  }

  pfc_bool_t autosave_enabled = PFC_FALSE;
  TcOperRet ret_val = db_hdlr_->GetConfTable(&autosave_enabled);
  if ( ret_val == TCOPER_RET_FAILURE ) {
    pfc_log_info("Database Read Failed, autosave enabled");
    return TC_OPER_FAILURE;
  }

  if (autosave_enabled == PFC_TRUE) {
    if (tc_oper_ == TC_OP_RUNNING_SAVE) {
      pfc_log_info("Autosave enabled: Skip save configuration command");
      return SendResponse(TC_OPER_SUCCESS);
    } else if (tc_oper_ == TC_OP_CLEAR_STARTUP) {
      pfc_log_error("Autosave enabled: Cannot clear startup-config");
      return RevokeOperation(TC_OPER_FORBIDDEN);
    }
  }

  pfc_log_info("tc_oper:Secure Exclusion");
  tc_oper_status_ = GET_EXCLUSION_PHASE;
  ret = TcGetExclusion();
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
  pfc_log_debug("tc_oper:Send Response to user");
  return SendResponse(TC_OPER_SUCCESS);
}

/*
 * @brief Execute the operation
 */

TcOperStatus TcDbOperations::Execute() {
  if ( TcOperMessageList.size() == 0 )
    return TC_OPER_FAILURE;

  if (tc_oper_ == TC_OP_RUNNING_SAVE) {
    if(db_hdlr_->GetRecoveryTableSaveVersion(save_version_)
                                          != TCOPER_RET_SUCCESS) {
      pfc_log_warn("Retrieving save_version data from recovery table failed");
    } else {
      ++save_version_;
      pfc_log_info("%s Setting save version %"PFC_PFMT_u64, __FUNCTION__,
                   save_version_);
    }
  }

  std::list<TcMsgOperType>::iterator MsgIter = TcOperMessageList.begin();
  while ( MsgIter != TcOperMessageList.end() ) {
    TcMsg *tcmsg_ =
        TcMsg::CreateInstance(session_id_,
                              (TcMsgOperType)*MsgIter,
                              unc_oper_channel_map_);
    if ( tcmsg_ == NULL ) {
      return TC_SYSTEM_FAILURE;
    }
    if (FillTcMsgData(tcmsg_, *MsgIter) != TC_OPER_SUCCESS) {
      delete tcmsg_;
      tcmsg_ = NULL;
      return TC_SYSTEM_FAILURE;
    }

    TcReadStatusOperations::SetStartupStatus();
    TcOperRet MsgRet = tcmsg_->Execute();
    TcReadStatusOperations::SetStartupStatusIncr();   
    
    if ( MsgRet != TCOPER_RET_SUCCESS ) {
      delete tcmsg_;
      tcmsg_ = NULL;
      return HandleMsgRet(MsgRet);
    }

    if (tc_oper_ == TC_OP_RUNNING_SAVE) {
      pfc_log_info("Save config: Setting save version: %"PFC_PFMT_u64,
                   save_version_);
      if (db_hdlr_->UpdateRecoveryTableSaveVersion(save_version_) !=
                                                TCOPER_RET_SUCCESS) {
        pfc_log_warn("Setting save_version to recovery table failed");
      }
    }

    MsgIter++;
    delete tcmsg_;
  }
  return TC_OPER_SUCCESS;
}

}  // namespace tc
}  // namespace unc
