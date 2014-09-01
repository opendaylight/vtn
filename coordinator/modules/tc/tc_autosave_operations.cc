/*
 * Copyright (c) 2012-2014 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */


#include <tc_operations.hh>


namespace unc {
namespace tc {

/* Mimimum number ofa rguments for AutoSave operation */
#define UNC_AUTOSAVE_OPER_ARG_COUNT_MIN  2


/*
 * @brief Constructor  
 */
TcAutoSaveOperations::TcAutoSaveOperations(TcLock* tc_lock_,
                           pfc::core::ipc::ServerSession* sess_,
                           TcDbHandler* db_handler,
                           TcChannelNameMap& unc_map_)
    : TcOperations(tc_lock_, sess_, db_handler, unc_map_),
      autosave_(PFC_FALSE) {}

/*
 * @brief Min Arg Count for this operation.
 */
uint32_t TcAutoSaveOperations::TcGetMinArgCount() {
  return UNC_AUTOSAVE_OPER_ARG_COUNT_MIN;
}

/*
 * @brief Write AutoSave value to response
 */
TcOperStatus TcAutoSaveOperations::SetAutoSave() {
  TcAutoSaveValue autosave_result_;
  if ( autosave_ == PFC_TRUE )
    autosave_result_ = TC_AUTOSAVE_ENABLED;
  else
    autosave_result_ = TC_AUTOSAVE_DISABLED;

  TcUtilRet ret=
      TcServerSessionUtils::set_uint32(ssess_,
                                       autosave_result_);
  if ( ret != TCUTIL_RET_SUCCESS ) {
    return TC_OPER_FAILURE;
  }
  return TC_OPER_SUCCESS;
}

/*
 * @brief convert TcLock return to TcOperStatus
 */
TcOperStatus TcAutoSaveOperations::HandleLockRet(TcLockRet lock_ret) {
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
  pfc_log_info("HandleLockRet: Received(%u) return (%u)",
               lock_ret, ret);
  return ret;
}

/*
 * @brief Check the argument count in the input
 */
TcOperStatus TcAutoSaveOperations::TcCheckOperArgCount(uint32_t avail_count) {
  if ( avail_count != UNC_AUTOSAVE_OPER_ARG_COUNT_MIN ) {
    pfc_log_error("TcCheckOperArgCount args expected(%u) received(%u)",
                  UNC_AUTOSAVE_OPER_ARG_COUNT_MIN, avail_count);
    return TC_OPER_INVALID_INPUT;
  }
  return TC_OPER_SUCCESS;
}

/*
 * @brief Check the opertype in the input
 */
TcOperStatus TcAutoSaveOperations::TcValidateOperType() {
  if (tc_oper_ > TC_OP_AUTOSAVE_DISABLE ||
      tc_oper_ < TC_OP_AUTOSAVE_GET) {
    pfc_log_error("TcValidateOperType opertype(%u) not in range"
                  " TC_OP_AUTOSAVE_DISABLE till TC_OP_AUTOSAVE_GET",
                  tc_oper_);
    return TC_INVALID_OPERATION_TYPE;
  }
  return TC_OPER_SUCCESS;
}

/*
 * @brief check Opertion Parameters
 */
TcOperStatus TcAutoSaveOperations::TcValidateOperParams() {
  return TC_OPER_SUCCESS;
}

/*
 * @brief Get Lock from TcLock
 */
TcOperStatus TcAutoSaveOperations::TcGetExclusion() {
  TcLockRet ret = TC_LOCK_FAILURE;

  if (tc_oper_ ==TC_OP_AUTOSAVE_GET) {
    ret = tclock_->GetLock(session_id_,
                         TC_AUTO_SAVE_GET,
                         TC_WRITE_NONE);
  } else if (tc_oper_ == TC_OP_AUTOSAVE_ENABLE) {
    ret = tclock_->GetLock(session_id_,
                         TC_AUTO_SAVE_ENABLE,
                         TC_WRITE_NONE);
  } else {
    ret = tclock_->GetLock(session_id_,
                         TC_AUTO_SAVE_DISABLE,
                         TC_WRITE_NONE);
  }
  if (ret != TC_LOCK_SUCCESS) {
     return HandleLockRet(ret);
  }
  return TC_OPER_SUCCESS;
}

/*
 * @brief Release Lock from TcLock
 */
TcOperStatus TcAutoSaveOperations::TcReleaseExclusion() {
  TcLockRet ret = TC_LOCK_FAILURE;
  if (tc_oper_ == TC_OP_AUTOSAVE_GET) {
    ret= tclock_->ReleaseLock(session_id_,
                              0,
                              TC_AUTO_SAVE_GET,
                              TC_WRITE_NONE);
  } else if (tc_oper_ == TC_OP_AUTOSAVE_ENABLE) {
    ret= tclock_->ReleaseLock(session_id_,
                              0,
                              TC_AUTO_SAVE_ENABLE,
                              TC_WRITE_NONE);
  } else {
    ret= tclock_->ReleaseLock(session_id_,
                              0,
                              TC_AUTO_SAVE_DISABLE,
                              TC_WRITE_NONE);
  }
  if (ret != TC_LOCK_SUCCESS) {
     return HandleLockRet(ret);
  }
  return TC_OPER_SUCCESS;
}

/*
 * @brief Execute the operation
 */
TcOperStatus TcAutoSaveOperations::Execute() {
  TcOperRet ret=
      db_hdlr_->GetConfTable(&autosave_);
  if ( ret != TCOPER_RET_SUCCESS )
    return TC_OPER_FAILURE;

  switch ( tc_oper_ ) {
    case TC_OP_AUTOSAVE_GET:
      {
       pfc_log_info("Get AutoSave");
      return TC_OPER_SUCCESS;
      }
    case TC_OP_AUTOSAVE_ENABLE:
      {
        pfc_log_info("Enable AutoSave");
        if ( autosave_ == PFC_TRUE ) {
          return TC_OPER_SUCCESS;
        } else {
          autosave_ = PFC_TRUE;
          ret = db_hdlr_->UpdateConfTable(autosave_);
          if ( ret != TCOPER_RET_SUCCESS )
            return TC_OPER_FAILURE;
        }
        break;
      }
    case TC_OP_AUTOSAVE_DISABLE:
      {
        pfc_log_info("Disable AutoSave");
        if ( autosave_ == PFC_FALSE ) {
          return TC_OPER_SUCCESS;
        } else {
          autosave_ = PFC_FALSE;
          ret = db_hdlr_->UpdateConfTable(autosave_);
          if ( ret != TCOPER_RET_SUCCESS )
            return TC_OPER_FAILURE;
        }
        break;
      }
    default:
      return TC_OPER_FAILURE;
  }

  // Send AutoSave status change request to TCLIB
  TcMsgOperType tclib_opertype = tclib::MSG_MAX;
  if (tc_oper_ == TC_OP_AUTOSAVE_ENABLE) {
    tclib_opertype = tclib::MSG_AUTOSAVE_ENABLE;
  } else if (tc_oper_ == TC_OP_AUTOSAVE_DISABLE) {
    tclib_opertype = tclib::MSG_AUTOSAVE_DISABLE;
  }
  TcMsg *tcmsg = TcMsg::CreateInstance(session_id_,
                                tclib_opertype,
                                unc_oper_channel_map_);
  if ( tcmsg == NULL ) {
    pfc_log_error("Cannot create instance for AutoSave notification");
    return TC_SYSTEM_FAILURE;
  }

  TcOperRet MsgRet = tcmsg->Execute();
  delete tcmsg;
  
  if ( MsgRet != TCOPER_RET_SUCCESS ) {
    return HandleMsgRet(MsgRet);
  }

  return TC_OPER_SUCCESS;
}

/*
 * @brief Create message list for operation (None)
 */
TcOperStatus TcAutoSaveOperations::TcCreateMsgList() {
  return TC_OPER_SUCCESS;
}

/*
 * @brief Data to be sent to other modules (None)
 */
TcOperStatus TcAutoSaveOperations::FillTcMsgData(TcMsg* tc_msg,
                           TcMsgOperType oper_type) {
  return TC_OPER_SUCCESS;
}

/*
 * @brief Write additional response (None)
 */
TcOperStatus TcAutoSaveOperations::
                  SendAdditionalResponse(TcOperStatus oper_stat) {
  if (SetAutoSave() != TC_OPER_SUCCESS) {
    pfc_log_error("SendAdditionalResponse setting AutoSave status failed");
    return TC_OPER_FAILURE;
  }
  return oper_stat;
}
}  // namespace tc
}  // namespace unc
