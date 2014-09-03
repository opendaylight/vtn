/*
 * Copyright (c) 2012-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */


#include <tc_operations.hh>
#include <alarm.hh>
#include <unc/component.h>

namespace unc {
namespace tc {

#define UNC_AUDIT_OPS_ARG_COUNT 4


TcAuditOperations::TcAuditOperations(TcLock* tc_lock_,
                           pfc::core::ipc::ServerSession* sess_,
                           TcDbHandler* db_handler,
                           TcChannelNameMap& unc_map_,
                           TcTaskqUtil* audit_)
    : TcOperations(tc_lock_, sess_, db_handler, unc_map_),
      driver_id_(UNC_CT_UNKNOWN),
      resp_tc_msg_(NULL),
      user_response_(TC_OPER_FAILURE),
      audit_handle_(audit_),
      audit_result_(unc::tclib::TC_AUDIT_FAILURE),
      trans_result_(unc::tclib::TRANS_END_FAILURE),
      api_audit_(PFC_FALSE),
      force_reconnect_(PFC_FALSE) {}

TcAuditOperations::~TcAuditOperations() {
  if (resp_tc_msg_) {
    delete resp_tc_msg_;
    resp_tc_msg_ = NULL;
  }
}

/*
 * @brief Minimun no of arguments for AUDIT
 */
uint32_t TcAuditOperations::TcGetMinArgCount() {
  return UNC_AUDIT_OPS_ARG_COUNT;
}

/*
 * @brief Handle Return from TcLock Class
 */
TcOperStatus TcAuditOperations::HandleLockRet(TcLockRet lock_ret) {
  TcOperStatus oper_status = TC_OPER_FAILURE;
  switch ( lock_ret ) {
    case TC_LOCK_INVALID_UNC_STATE:
    case TC_LOCK_OPERATION_NOT_ALLOWED:
      oper_status = TC_INVALID_STATE;
      break;
    case TC_LOCK_BUSY:
      oper_status = TC_SYSTEM_BUSY;
      break;
    case TC_LOCK_NO_CONFIG_SESSION_EXIST:
      oper_status = TC_CONFIG_NOT_PRESENT;
      break;
    default:
      oper_status = TC_OPER_FAILURE;
  }
  pfc_log_info("HandleLockRet: Received(%u), return(%u)",
               lock_ret, oper_status);
  return oper_status;
}

/*
 * @brief Check No of Arguments
 */
TcOperStatus TcAuditOperations::TcCheckOperArgCount(uint32_t avail_count) {
  if ( avail_count != UNC_AUDIT_OPS_ARG_COUNT ) {
    pfc_log_error("TcCheckOperArgCount avail_count(%u) != %u",
                  avail_count, UNC_AUDIT_OPS_ARG_COUNT);
    return TC_OPER_INVALID_INPUT;
  }
  return TC_OPER_SUCCESS;
}

/*
 * @brief Get Session Id from session
 */
TcOperStatus TcAuditOperations::GetSessionId() {
  if ( tc_oper_ == TC_OP_DRIVER_AUDIT )
    return TC_OPER_SUCCESS;
  TcUtilRet ret=
      TcServerSessionUtils::get_uint32(ssess_,
                                       TC_REQ_SESSION_ID_INDEX,
                                       &session_id_);
  if ( ret == TCUTIL_RET_FAILURE ) {
    pfc_log_error("GetSessionId: get_uint32 return TCUTIL_RET_FAILURE");
    return TC_OPER_INVALID_INPUT;
  } else if ( ret == TCUTIL_RET_FATAL ) {
    pfc_log_error("GetSessionId: get_uint32 return TCUTIL_RET_FATAL");
    return TC_OPER_FAILURE;
  }
  if ( session_id_ == 0 ) {
    pfc_log_error("GetSessionId: Received session_id is 0");
    return TC_OPER_INVALID_INPUT;
  }

  return TC_OPER_SUCCESS;
}

/*
 * @brief check for valid Operation Type
 */
TcOperStatus TcAuditOperations::TcValidateOperType() {
  if ((tc_oper_ != TC_OP_USER_AUDIT ) &&
      (tc_oper_ != TC_OP_DRIVER_AUDIT)) {
    pfc_log_error("TcValidateOperType opertype(%u) is not either of "
                  "TC_OP_USER_AUDIT or TC_OP_DRIVER_AUDIT", tc_oper_);
    return TC_INVALID_OPERATION_TYPE;
  }
  /*set IPC timeout to infinity for audit operations*/
  if (tc_oper_ == TC_OP_USER_AUDIT) {
    TcUtilRet ret = TcServerSessionUtils::set_srv_timeout(ssess_, NULL);
    if (ret == TCUTIL_RET_FAILURE) {
      pfc_log_error("TcValidateOperType set_srv_timeout to Infinite failed");
      return TC_OPER_FAILURE;
    }
  }
  return TC_OPER_SUCCESS;
}

/*
 * @brief check for valid Operation Paramaters
 */
TcOperStatus TcAuditOperations::TcValidateOperParams() {
  TcUtilRet ret = TCUTIL_RET_SUCCESS, ret1 = TCUTIL_RET_SUCCESS;
  uint8_t reconnect = 0;
  if ( tc_oper_ == TC_OP_USER_AUDIT ) {
    ret= TcServerSessionUtils::get_string(ssess_,
                                          TC_REQ_ARG_INDEX,
                                          controller_id_);
    ret1= TcServerSessionUtils::get_uint8(ssess_,
                                          TC_REQ_ARG_INDEX+1,
                                          &reconnect);
  } else {
    ret= TcServerSessionUtils::get_string(ssess_,
                                          TC_REQ_SESSION_ID_INDEX,
                                          controller_id_);
  }
  if (ret == TCUTIL_RET_FAILURE || ret1 == TCUTIL_RET_FAILURE) {
    pfc_log_error("TcValidateOperParams: Fail reading session data");
    return TC_OPER_INVALID_INPUT;
  } else if (ret == TCUTIL_RET_FATAL || ret1 == TCUTIL_RET_FATAL) {
    pfc_log_error("TcValidateOperParams: Fatal reading session data");
    return TC_OPER_FAILURE;
  }

  if ( controller_id_.length() == 0 ) {
    pfc_log_error("Zero length controller-id");
    return TC_OPER_INVALID_INPUT;
  }
  /*force-reconnect option*/
  force_reconnect_ = (pfc_bool_t)reconnect;
  pfc_log_info("controller %s force_reconnect %d",
               controller_id_.c_str(), force_reconnect_);

  uint8_t read_drv_id = 0;
  if ( tc_oper_ == TC_OP_DRIVER_AUDIT ) {
    TcUtilRet ret= TcServerSessionUtils::get_uint8(ssess_,
                                                   TC_REQ_ARG_INDEX,
                                                   &read_drv_id);
    if ( ret == TCUTIL_RET_FAILURE ) {
      pfc_log_error("TcValidateOperParams Failure reading read_drv_id");
      return TC_OPER_INVALID_INPUT;
    } else if ( ret == TCUTIL_RET_FATAL ) {
      pfc_log_error("TcValidateOperParams Fatal reading read_drv_id");
      return TC_OPER_FAILURE;
    }

    driver_id_ = (unc_keytype_ctrtype_t) read_drv_id;
    if ( driver_id_ == UNC_CT_UNKNOWN ) {
      pfc_log_info("API passed invalid controller type");
      return TC_OPER_INVALID_INPUT;
    }
    api_audit_ = PFC_TRUE;
    TcDbHandler* audit_db_hdlr(new TcDbHandler(*db_hdlr_));
    PFC_ASSERT(audit_db_hdlr != NULL);
    if (audit_handle_->DispatchAuditDriverRequest(controller_id_,
                                                  audit_db_hdlr,
                                                  tclock_,
                                                  unc_oper_channel_map_,
                                                  driver_id_) != 0) {
      pfc_log_error("TcValidateOperParams:DispatchAuditDriverRequest failed");
      return TC_OPER_FAILURE;
    }
  }
  return TC_OPER_SUCCESS;
}

/*
 * @brief Secure Exclusion for AUDIT Oepration
 */
TcOperStatus TcAuditOperations::TcGetExclusion() {
  if ( tc_oper_ == TC_OP_USER_AUDIT ) {
    TcLockRet ret = TC_LOCK_FAILURE;
    ret = tclock_->GetLock(session_id_,
                           TC_ACQUIRE_WRITE_SESSION,
                           TC_AUDIT_USER);
    if (ret != TC_LOCK_SUCCESS) {
       return HandleLockRet(ret);
    }

  } else {
    if ( api_audit_ == PFC_TRUE )
      return TC_OPER_SUCCESS;
    TcLockRet ret = TC_LOCK_FAILURE;
    ret= tclock_->GetLock(session_id_,
                          TC_ACQUIRE_WRITE_SESSION,
                          TC_AUDIT_DRIVER);
    if (ret != TC_LOCK_SUCCESS) {
       return HandleLockRet(ret);
    }
  }
  return TC_OPER_SUCCESS;
}

/*
 * @brief Release Exclusion after AUDIT Oepration
 */
TcOperStatus TcAuditOperations::TcReleaseExclusion() {
  if ( api_audit_ == PFC_TRUE ) {
    return TC_OPER_SUCCESS;
  }
  if (tc_oper_ == TC_OP_USER_AUDIT) {
    TcLockRet ret = TC_LOCK_FAILURE;
    ret = tclock_->ReleaseLock(session_id_,
                               0,
                               TC_RELEASE_WRITE_SESSION,
                               TC_AUDIT_USER);
    if (ret != TC_LOCK_SUCCESS) {
      return HandleLockRet(ret);
    }
  } else {
    TcLockRet ret = TC_LOCK_FAILURE;
    ret = tclock_->ReleaseLock(session_id_,
                              0,
                              TC_RELEASE_WRITE_SESSION,
                              TC_AUDIT_DRIVER);
    if (ret != TC_LOCK_SUCCESS) {
      return HandleLockRet(ret);
    }
  }
  return TC_OPER_SUCCESS;
}

/*
 * @brief Msg List for AUDIT (Not USed)
 */
TcOperStatus TcAuditOperations::TcCreateMsgList() {
  return TC_OPER_SUCCESS;
}

/*
 * @brief Get the type of driver for user AUDIT
 */
pfc_bool_t TcAuditOperations::GetDriverType() {
  if ( tc_oper_ == TC_OP_DRIVER_AUDIT )
     return PFC_TRUE;
  TcMsg* tc_drv_msg = TcMsg::CreateInstance(session_id_,
                                            unc::tclib::MSG_GET_DRIVERID,
                                            unc_oper_channel_map_);
  PFC_ASSERT(tc_drv_msg != NULL);
  FillTcMsgData(tc_drv_msg, unc::tclib::MSG_GET_DRIVERID);
  TcOperRet oper_ret(tc_drv_msg->Execute());
  if ( oper_ret != TCOPER_RET_SUCCESS ) {
    user_response_ = HandleMsgRet(oper_ret);
    delete tc_drv_msg;
    tc_drv_msg = NULL;
    return PFC_FALSE;
  }
  driver_id_ = tc_drv_msg->GetResult();
  if ( driver_id_ == UNC_CT_UNKNOWN ) {
    pfc_log_error("Controller of Unknown type");
    resp_tc_msg_ = tc_drv_msg;
    return PFC_FALSE;
  }
  delete tc_drv_msg;
  return PFC_TRUE;
}

/*
 * @brief Handle AUDIT Start Message
 */
pfc_bool_t TcAuditOperations::AuditStart() {
  TcMsg* tc_audit_start_msg= TcMsg::CreateInstance(session_id_,
                                             unc::tclib::MSG_AUDIT_START,
                                             unc_oper_channel_map_);
  PFC_ASSERT(tc_audit_start_msg != NULL);
  FillTcMsgData(tc_audit_start_msg, unc::tclib::MSG_AUDIT_START);
  TcOperRet oper_ret(tc_audit_start_msg->Execute());
  if ( oper_ret != TCOPER_RET_SUCCESS &&
       oper_ret != TCOPER_RET_SIMPLIFIED_AUDIT ) {
    user_response_ = HandleMsgRet(oper_ret);
    resp_tc_msg_ = tc_audit_start_msg;
    return PFC_FALSE;
  }
  if ( oper_ret == TCOPER_RET_SIMPLIFIED_AUDIT ) {
    user_response_ = TC_OPER_SIMPLIFIED_AUDIT;
  }
  delete tc_audit_start_msg;
  return PFC_TRUE;
}

/*
 * @brief Handle AUDIT Transaction Start Message
 */
pfc_bool_t TcAuditOperations::AuditTransStart() {
  TcMsg* tc_start_msg= TcMsg::CreateInstance(session_id_,
                                             unc::tclib::MSG_AUDIT_TRANS_START,
                                             unc_oper_channel_map_);
  PFC_ASSERT(tc_start_msg != NULL);
  FillTcMsgData(tc_start_msg, unc::tclib::MSG_AUDIT_TRANS_START);
  TcOperRet oper_ret(tc_start_msg->Execute());
  if ( oper_ret != TCOPER_RET_SUCCESS ) {
    if ( oper_ret == TCOPER_RET_FATAL ) {
      user_response_ = HandleMsgRet(oper_ret);
      resp_tc_msg_ = tc_start_msg;
      return PFC_FALSE;
    }
    if ( AuditEnd() != PFC_FALSE ) {
      user_response_ = HandleMsgRet(oper_ret);
      resp_tc_msg_ = tc_start_msg;
      return PFC_FALSE;
    } else {
      delete tc_start_msg;
      return PFC_FALSE;
    }
  }
  delete tc_start_msg;
  return PFC_TRUE;
}

/*
 * @brief Handle AUDIT Vote Message
 */
pfc_bool_t TcAuditOperations::AuditVote() {
  TcMsg* tc_vote_msg= TcMsg::CreateInstance(session_id_,
                                             unc::tclib::MSG_AUDIT_VOTE,
                                             unc_oper_channel_map_);
  PFC_ASSERT(tc_vote_msg != NULL);
  FillTcMsgData(tc_vote_msg, unc::tclib::MSG_AUDIT_VOTE);
  TcOperRet oper_ret(tc_vote_msg->Execute());
  if ( oper_ret != TCOPER_RET_SUCCESS ) {
    if ( oper_ret == TCOPER_RET_FATAL ) {
      user_response_ = HandleMsgRet(oper_ret);
      resp_tc_msg_ = tc_vote_msg;
      return PFC_FALSE;
    }
    if ( AuditTransEnd() != PFC_FALSE ) {
       if ( AuditEnd() != PFC_FALSE ) {
         user_response_ = HandleMsgRet(oper_ret);
         resp_tc_msg_ = tc_vote_msg;
         return PFC_FALSE;
       }
    }
    delete tc_vote_msg;
    return PFC_FALSE;
  }
  delete tc_vote_msg;
  return PFC_TRUE;
}


/*
 * @brief Handle AUDIT Global Commit Message
 */
pfc_bool_t TcAuditOperations::AuditGlobalCommit() {
  if ( db_hdlr_->UpdateRecoveryTable(UNC_DT_RUNNING,
                                     tc_oper_) != TCOPER_RET_SUCCESS) {
    pfc_log_info("Recovery Table not updated");
  }

  TcMsg* tc_commit_msg= TcMsg::CreateInstance(session_id_,
                                             unc::tclib::MSG_AUDIT_GLOBAL,
                                             unc_oper_channel_map_);

  PFC_ASSERT(tc_commit_msg != NULL);
  FillTcMsgData(tc_commit_msg, unc::tclib::MSG_AUDIT_GLOBAL);
  TcOperRet oper_ret(tc_commit_msg->Execute());
  trans_result_ = tc_commit_msg->GetTransResult();
  audit_result_ = tc_commit_msg->GetAuditResult();
  if ( oper_ret != TCOPER_RET_SUCCESS ) {
    user_response_ = TC_SYSTEM_FAILURE;
    delete tc_commit_msg;
    return PFC_FALSE;
  }
  resp_tc_msg_ = tc_commit_msg;
  return PFC_TRUE;
}

/*
 * @brief Handle AUDIT Transaction End Message
 */
pfc_bool_t TcAuditOperations::AuditTransEnd() {
  TcMsg* tc_end_msg= TcMsg::CreateInstance(session_id_,
                                           unc::tclib::MSG_AUDIT_TRANS_END,
                                           unc_oper_channel_map_);

  PFC_ASSERT(tc_end_msg != NULL);
  FillTcMsgData(tc_end_msg, unc::tclib::MSG_AUDIT_TRANS_END);
  TcOperRet oper_ret(tc_end_msg->Execute());
  if ( oper_ret != TCOPER_RET_SUCCESS ) {
    user_response_ = TC_SYSTEM_FAILURE;
    delete tc_end_msg;
    return PFC_FALSE;
  }
  delete tc_end_msg;
  return PFC_TRUE;
}

/*
 * @brief Handle AUDIT End Message
 */
pfc_bool_t TcAuditOperations::AuditEnd() {
  TcMsg* tc_end_msg= TcMsg::CreateInstance(session_id_,
                                           unc::tclib::MSG_AUDIT_END,
                                           unc_oper_channel_map_);

  PFC_ASSERT(tc_end_msg != NULL);
  FillTcMsgData(tc_end_msg, unc::tclib::MSG_AUDIT_END);
  TcOperRet oper_ret(tc_end_msg->Execute());
  if ( oper_ret != TCOPER_RET_SUCCESS ) {
    user_response_ = TC_SYSTEM_FAILURE;
    delete tc_end_msg;
    return PFC_FALSE;
  }
  delete tc_end_msg;
  return PFC_TRUE;
}

/*
 * @brief Fill Message for tclib
 */
TcOperStatus TcAuditOperations::FillTcMsgData(TcMsg* tc_msg,
                           TcMsgOperType oper_type) {
  if (tc_msg == NULL)
    return TC_OPER_FAILURE;

  if ( oper_type == unc::tclib::MSG_GET_DRIVERID ) {
    tc_msg->SetData(0, controller_id_, UNC_CT_UNKNOWN);
  } else {
    if ( driver_id_ == UNC_CT_UNKNOWN ) {
      return TC_OPER_INVALID_INPUT;
    }
    tc_msg->SetData(0, controller_id_, driver_id_);
  }
  if (oper_type == unc::tclib::MSG_AUDIT_START) {
    tc_msg->SetReconnect(force_reconnect_);
    if(tc_oper_ == TC_OP_USER_AUDIT)
      tc_msg->IsUserAudit(PFC_TRUE);
  }
  if (oper_type == unc::tclib::MSG_AUDIT_TRANS_END) {
    tc_msg->SetTransResult(trans_result_);
  }
  if (oper_type == unc::tclib::MSG_AUDIT_END) {
    tc_msg->SetAuditResult(audit_result_);
  }
  return TC_OPER_SUCCESS;
}


/*
 * @brief Execute Messge sequence
 */
TcOperStatus TcAuditOperations::Execute() {
  if ( api_audit_ == PFC_TRUE ) {
    return TC_OPER_SUCCESS;
  }

  if ( GetDriverType() == PFC_FALSE ) {
    return TC_OPER_ABORT;
  }

  pfc_log_info("Driver Type %d", driver_id_);

  if ( AuditStart() == PFC_FALSE ) {
    return user_response_;
  }
  if ( tc_oper_ == TC_OP_USER_AUDIT ||
       user_response_ != TC_OPER_SIMPLIFIED_AUDIT ) {
    if ( AuditTransStart() == PFC_FALSE ) {
      return user_response_;
    }
    if ( AuditVote() == PFC_FALSE ) {
      return user_response_;
    }
    if ( AuditGlobalCommit() == PFC_FALSE ) {
      return user_response_;
    }
    if ( AuditTransEnd() == PFC_FALSE ) {
      return user_response_;
    }
  }
  if ( user_response_ == TC_OPER_SIMPLIFIED_AUDIT &&
       tc_oper_ == TC_OP_DRIVER_AUDIT ) {
    audit_result_ =  unc::tclib::TC_SIMPLIFIED_AUDIT_SUCCESS;
  }
  if ( AuditEnd() == PFC_FALSE ) {
    return user_response_;
  }
  if ( db_hdlr_->UpdateRecoveryTable(UNC_DT_INVALID,
                                     TC_OP_INVALID)!= TCOPER_RET_SUCCESS) {
    pfc_log_info("Recovery Table not updated");
    user_response_ = TC_OPER_FAILURE;
    return TC_OPER_FAILURE;
  }
  return TC_OPER_SUCCESS;
}

/*set Audit operation status*/
TcOperStatus
TcAuditOperations::SetAuditOperationStatus() {
  pfc_log_info("Audit status:%d", audit_result_);

  TcUtilRet ret =
      TcServerSessionUtils::set_uint32(ssess_,
                                       audit_result_);
  if ( ret != TCUTIL_RET_SUCCESS ) {
    pfc_log_error("SetAuditOperationStatus: setting audit_result failed");
    return TC_OPER_FAILURE;
  }
  return TC_OPER_SUCCESS;
}

/*
 * @brief Add Additional Response
 */
TcOperStatus
TcAuditOperations::SendAdditionalResponse(TcOperStatus oper_stat) {
  if ( api_audit_ == PFC_TRUE ) {
    return oper_stat;
  }
  /*Append the status of Audit operation*/
  if (SetAuditOperationStatus() != TC_OPER_SUCCESS) {
    pfc_log_error("SendAdditionalResponse Setting Audit opstat failed");
    return TC_OPER_FAILURE;
  }
  if (tc_oper_ == TC_OP_USER_AUDIT &&
     resp_tc_msg_ != NULL) {
    TcOperRet ret = resp_tc_msg_->ForwardResponseToVTN(*ssess_);
    if (ret != TCOPER_RET_SUCCESS) {
      pfc_log_error("SendAdditionalResponse Forwarding resp to VTN failed");
      return TC_SYSTEM_FAILURE;
    }
  }
  return oper_stat;
}

}  // namespace  tc
}  // namespace unc
