/*
 * Copyright (c) 2012-2015 NEC Corporation
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

#define UNC_AUDIT_OPS_ARG_MIN_COUNT 4
#define UNC_AUDIT_OPS_ARG_MAX_COUNT 5

std::deque<TcAuditOperations*> TcAuditOperations::drv_audit_req_queue_;
pfc::core::Mutex TcAuditOperations::drv_audit_req_queue_lock_;
uint32_t TcAuditOperations::secondary_driver_audit_timeout_ = 30;

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
      force_reconnect_(PFC_FALSE),
      audit_type_(TC_AUDIT_NORMAL) {

  pthread_cond_init(&primary_wait_cond_, NULL);
  pthread_mutex_init(&primary_wait_mutex_, NULL);
  primary_wait_signalled = PFC_FALSE;

  pthread_cond_init(&secondary_wait_cond_, NULL);
  pthread_mutex_init(&secondary_wait_mutex_, NULL);
  secondary_wait_signalled_ = PFC_FALSE;
}

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
  return UNC_AUDIT_OPS_ARG_MIN_COUNT;
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
  if ( avail_count < UNC_AUDIT_OPS_ARG_MIN_COUNT || 
                               avail_count > UNC_AUDIT_OPS_ARG_MAX_COUNT) {
    pfc_log_error("TcCheckOperArgCount AuditOp args expected(%u)or(%u)"
                        " recvd(%u)", UNC_AUDIT_OPS_ARG_MIN_COUNT,
                        UNC_AUDIT_OPS_ARG_MAX_COUNT, avail_count);
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
  TcUtilRet utilret = TCUTIL_RET_SUCCESS;
  uint8_t reconnect = 0;
  if ( tc_oper_ == TC_OP_USER_AUDIT ) {
    ret= TcServerSessionUtils::get_string(ssess_,
                                          TC_REQ_ARG_INDEX,
                                          controller_id_);
    ret1= TcServerSessionUtils::get_uint8(ssess_,
                                          TC_REQ_ARG_INDEX+1,
                                          &reconnect);
    uint32_t count = ssess_->getArgCount();
    if (count == UNC_AUDIT_OPS_ARG_MAX_COUNT) {
      uint8_t audit_type = 0;
      utilret = TcServerSessionUtils::get_uint8(ssess_,
                                          TC_REQ_ARG_INDEX+2,
                                          &audit_type);
      audit_type_ = (TcAuditType)audit_type; 
      pfc_log_info("AuditType: %d", audit_type_);
    } else if(count == UNC_AUDIT_OPS_ARG_MIN_COUNT) {
        audit_type_ = TC_AUDIT_NORMAL;
    } else  {
        audit_type_ = TC_AUDIT_INVALID;
        pfc_log_error("TcValidateOperParams AuditOp args expected(%u)or(%u)"
                        " recvd(%u)", UNC_AUDIT_OPS_ARG_MIN_COUNT,
                        UNC_AUDIT_OPS_ARG_MAX_COUNT, count);
        return TC_OPER_INVALID_INPUT;
    }   
  } else {
    ret= TcServerSessionUtils::get_string(ssess_,
                                          TC_REQ_SESSION_ID_INDEX,
                                          controller_id_);
  }
  if (ret == TCUTIL_RET_FAILURE || ret1 == TCUTIL_RET_FAILURE ||
                                   utilret == TCUTIL_RET_FAILURE) {
    pfc_log_error("TcValidateOperParams: Fail reading session data");
    return TC_OPER_INVALID_INPUT;
  } else if (ret == TCUTIL_RET_FATAL || ret1 == TCUTIL_RET_FATAL ||
                                    utilret == TCUTIL_RET_FATAL) {
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
      pfc_log_error("API passed invalid controller type");
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

  pfc::core::ScopedMutex audit_excl(TcOperations::candidate_audit_excl_lock_);

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

  // If this is cancelled audit, set the status to cancel_done
  if (GetAuditCancelStatus() == AUDIT_CANCEL_INPROGRESS) {
    pfc_log_info("%s Setting status to AUDIT_CANCEL_DONE", __FUNCTION__);
    SetAuditCancelStatus(AUDIT_CANCEL_DONE);
  }
  TcMsg::SetAuditCancelFlag(PFC_FALSE);
  TcOperations::SetAuditPhase(AUDIT_NOT_STARTED);
  pfc::core::ScopedMutex m(TcOperations::audit_cancel_notify_lock_);
  TcOperations::audit_cancel_notify_.clear();
  m.unlock();

  audit_excl.unlock();

  // Signal the candidate waiting queue for processing
  // If candidate queue is empty, waiting audit if any is triggered 
  // from HandleCandidateRelease
  pfc_log_info("%s Signal candidate wait queue for processing", __FUNCTION__);
  TcCandidateOperations::HandleCandidateRelease();

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
    if ( oper_ret == TCOPER_RET_AUDIT_CANCELLED ) {
      audit_result_ = unc::tclib::TC_AUDIT_CANCELLED;
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
    if ( oper_ret == TCOPER_RET_AUDIT_CANCELLED ) {
      audit_result_ = unc::tclib::TC_AUDIT_CANCELLED;
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

  tc_msg->SetAuditType(audit_type_);

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
  TcMsg::SetNotifyDriverId(driver_id_);
  TcMsg::SetNotifyControllerId(controller_id_);

  // If audit is cancelled even before the AuditStart is sent,
  // just return. No cancel-audit required in this case.
  // But the audit request will go to wait state.
  if (TcMsg::GetAuditCancelFlag() == PFC_TRUE && 
             audit_type_ != TC_AUDIT_REALNETWORK) {
    pfc_log_info("%s Audit cancelled even before AUDIT_START", __FUNCTION__);
    user_response_ = TC_OPER_AUDIT_CANCELLED;
    audit_result_ = unc::tclib::TC_AUDIT_CANCELLED;
    return user_response_;
  }

  TcOperations::SetAuditPhase(AUDIT_START);
  if ( AuditStart() == PFC_FALSE ) {
    return user_response_;
  }

  if ( (tc_oper_ == TC_OP_USER_AUDIT && audit_type_ != TC_AUDIT_REALNETWORK) ||
       (tc_oper_ == TC_OP_DRIVER_AUDIT &&
        user_response_ != TC_OPER_SIMPLIFIED_AUDIT) ) {
    
    TcOperations::SetAuditPhase(AUDIT_TRANSACTION_START);
    if ( AuditTransStart() == PFC_FALSE ) {
      return user_response_;
    }

    TcOperations::SetAuditPhase(AUDIT_VOTE_REQUEST);
    if ( AuditVote() == PFC_FALSE ) {
      return user_response_;
    }

    TcOperations::SetAuditPhase(AUDIT_GLOBAL_COMMIT);
    // Just while setting phase to GLOBAL_COMMIT,
    // if the audit has been cancelled, do not send global commit request
    if (TcMsg::GetAuditCancelFlag() == PFC_TRUE &&
                audit_type_ != TC_AUDIT_REALNETWORK) {
      pfc_log_info("%s Audit cancelled before GLOBAL_COMMIT", __FUNCTION__);
      audit_result_ = unc::tclib::TC_AUDIT_CANCELLED;

      if ( AuditEnd() == PFC_FALSE ) {
        return user_response_;
      }
      user_response_ = TC_OPER_AUDIT_CANCELLED;
      return user_response_;
    }
    TcReadStatusOperations::SetRunningStatus();
    if ( AuditGlobalCommit() == PFC_FALSE ) {
      return user_response_;
    }
    TcReadStatusOperations::SetRunningStatusIncr();
   
    TcOperations::SetAuditPhase(AUDIT_TRANSACTION_END);
    if ( AuditTransEnd() == PFC_FALSE ) {
      return user_response_;
    }
  }

  if (user_response_ == TC_OPER_SIMPLIFIED_AUDIT &&
      tc_oper_ == TC_OP_DRIVER_AUDIT ) {
    pfc_log_info("%s Simplified Audit completed", __FUNCTION__);
    audit_result_ =  unc::tclib::TC_SIMPLIFIED_AUDIT_SUCCESS;
  }

  if (tc_oper_ == TC_OP_USER_AUDIT &&
      audit_type_ == TC_AUDIT_REALNETWORK) {
    pfc_log_info("%s REALNETWORK audit completed", __FUNCTION__);
    audit_result_ = unc::tclib::TC_AUDIT_SUCCESS;
  }

  TcOperations::SetAuditPhase(AUDIT_END);
  if ( AuditEnd() == PFC_FALSE ) {
    return user_response_;
  }
  if ( db_hdlr_->UpdateRecoveryTable(UNC_DT_INVALID,
                                     TC_OP_INVALID)!= TCOPER_RET_SUCCESS) {
    pfc_log_error("Recovery Table not updated");
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
TcOperStatus
TcAuditOperations::Dispatch() {
  TcOperStatus ret;

  pfc_log_debug("tc_oper:Read Input Paramaters");
  tc_oper_status_ = INPUT_VALIDATION;
  ret = HandleArgs();
  if (ret != TC_OPER_SUCCESS) {
    return RevokeOperation(ret);
  }

  pthread_mutex_lock(&primary_wait_mutex_);
  primary_wait_signalled = PFC_FALSE;
  pthread_mutex_unlock(&primary_wait_mutex_);

  pthread_mutex_lock(&secondary_wait_mutex_);
  secondary_wait_signalled_ = PFC_FALSE;
  pthread_mutex_unlock(&secondary_wait_mutex_);

  pfc::core::ScopedMutex m(TcCandidateOperations::candidate_req_queue_lock_);
  pfc_bool_t IsCandidateQueueEmpty =
                         TcCandidateOperations::candidate_req_queue_.empty();
  m.unlock();

  if (ssess_ == NULL && tc_oper_ == TC_OP_DRIVER_AUDIT &&
      !IsCandidateQueueEmpty) {
    pfc_log_info("%s Candidate Q not empty; Start DrvAudit wait",
                   __FUNCTION__);
    ret = DriverAuditWait();
  } else if (tc_oper_ == TC_OP_USER_AUDIT &&
             !IsCandidateQueueEmpty) {
    pfc_log_error("%s Candidate wait queue not empty. Return BUSY",
                  __FUNCTION__);
    return RevokeOperation(TC_SYSTEM_BUSY);
  } else {
    pfc_log_info("tc_oper:Secure Exclusion");
    tc_oper_status_ = GET_EXCLUSION_PHASE;
    ret = TcGetExclusion();
    if ( ret != TC_OPER_SUCCESS ) {
      if (ssess_ == NULL && tc_oper_ == TC_OP_DRIVER_AUDIT) {
         pfc_log_info("%s TcGetExclusion ret %d; Start DrvAudit wait",
                      __FUNCTION__, ret);
         ret = DriverAuditWait();
      } else {
        return RevokeOperation(ret);
      }
    }
  }

  // Check for return from DriverAuditWait
  if (ret != TC_OPER_SUCCESS) {
    pfc_log_error("%s DriverAuditWait return failure",
                  __FUNCTION__);
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
    if (tc_oper_ == TC_OP_DRIVER_AUDIT) {
      delete resp_tc_msg_;
      resp_tc_msg_ = NULL;
    }
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

TcOperStatus
TcAuditOperations::DriverAuditWait() {
  if (TcClientSessionUtils::get_sys_stop()) {
    pfc_log_info("%s SYS-STOP inprogress. Not starting driver audit wait",
                 __FUNCTION__);
    return TC_INVALID_STATE;
  }

  int32_t pthread_ret = 0;

primary_wait:

  pthread_mutex_lock(&primary_wait_mutex_);
  if (primary_wait_signalled == PFC_FALSE) {
    pfc_log_info("%s Starting primary wait for Audit[%d, %s]",
                 __FUNCTION__, driver_id_, controller_id_.c_str());
    pthread_ret = pthread_cond_wait(&primary_wait_cond_,
                                    &primary_wait_mutex_);
  } else {
    pfc_log_info("%s Primary signal set. No need to primary wait",
                 __FUNCTION__);
  }

  // Reset the signal flag
  primary_wait_signalled = PFC_FALSE;
  // Reset the secondary wait signal flag after primary wait.
  // The canidate requests sets this flag on start of dispatch
  // So everytime, the flag will be set. Therefore reset here
  pthread_mutex_lock(&secondary_wait_mutex_);
  secondary_wait_signalled_ = PFC_FALSE;
  pthread_mutex_unlock(&secondary_wait_mutex_);

  pthread_mutex_unlock(&primary_wait_mutex_);

  pfc_log_info("%s Primary wait for Driver audit return %d",
               __FUNCTION__, pthread_ret);

  if (TcClientSessionUtils::get_sys_stop()) {
    pfc_log_info("%s SYS-STOP in-progress. Return from Primary DriverAuditWait",
                 __FUNCTION__);
    return TC_INVALID_STATE;
  }

  if (tclock_->IsLastOperationCandidate()) {
    pfc_log_info("%s Last operation completed is candidate op. "
                 "Starting secondary wait for %u seconds",
                 __FUNCTION__, secondary_driver_audit_timeout_);

    pfc_timespec_t timeout;
    struct timeval now;
    gettimeofday(&now, NULL);

    pthread_ret = 0;

    timeout.tv_sec = now.tv_sec + secondary_driver_audit_timeout_;
    timeout.tv_nsec = now.tv_usec * 1000;

    pthread_mutex_lock(&secondary_wait_mutex_);
    if (secondary_wait_signalled_ == PFC_FALSE) {
      pfc_log_info("%s Starting secondary wait for Audit[%d, %s]",
                   __FUNCTION__, driver_id_, controller_id_.c_str());
      pthread_ret = pthread_cond_timedwait(&secondary_wait_cond_,
                                           &secondary_wait_mutex_,
                                           &timeout);
    } else {
      pfc_log_info("%s Secondary signal set. No need to secondary wait",
                   __FUNCTION__);
    }

    // Reset secondary_wait_signalled_ flag
    secondary_wait_signalled_ = PFC_FALSE;

    pthread_mutex_unlock(&secondary_wait_mutex_);

    if (TcClientSessionUtils::get_sys_stop()) {
      pfc_log_info("%s SYS-STOP in-progress. Return Secondary DriverAuditWait",
                   __FUNCTION__);
      return TC_INVALID_STATE;
    }

    if (pthread_ret == 0) {
      pfc_log_info("%s Secondary wait time cancelled by candidate operation",
                   __FUNCTION__);
      goto primary_wait;  // Go back to primary waiting
    } else {
      pfc_log_info("%s Secondary wait over; pthread_ret=%d. "
                   "Check if write lock can be acquired?",
                   __FUNCTION__, pthread_ret);
    }
  }

  pfc_log_info("%s All wait complete. Secure Exclusion", __FUNCTION__);
  TcOperStatus ret = TcGetExclusion();
  if (ret != TC_OPER_SUCCESS) {
    pfc_log_info("%s TcGetExclusion failed. Start primary wait again",
                 __FUNCTION__);
    goto primary_wait;
  }

  pfc_log_info("%s Lock acquired for audit [%d, %s]",
               __FUNCTION__, driver_id_, controller_id_.c_str());
  return ret;
}

void
TcAuditOperations::SetSecondaryWaitTime(uint32_t timeout) {
  secondary_driver_audit_timeout_ = timeout;
}

void
TcAuditOperations::PushToWaitQueue(TcAuditOperations * audit_op) {
  pfc::core::ScopedMutex m(drv_audit_req_queue_lock_);
  drv_audit_req_queue_.push_back(audit_op);
  pfc_log_info("%s Current wait size:%" PFC_PFMT_SIZE_T,
               __FUNCTION__, drv_audit_req_queue_.size()); 
}

void
TcAuditOperations::PopWaitQueue() {
  std::deque<TcAuditOperations*>::iterator it;

  pfc::core::ScopedMutex m(drv_audit_req_queue_lock_);

  it = drv_audit_req_queue_.begin();
  if (it != drv_audit_req_queue_.end()) {
    drv_audit_req_queue_.erase(it);
    pfc_log_info("%s After remove wait size:%" PFC_PFMT_SIZE_T,
                 __FUNCTION__, drv_audit_req_queue_.size()); 
  } else {
    pfc_log_info("%s Driver-Audit wait queue empty", __FUNCTION__);
  }
}

int32_t
TcAuditOperations::SignalPrimaryWaitingAudit() {

  uint32_t ret = 0;
  pfc::core::ScopedMutex m(drv_audit_req_queue_lock_);

  std::deque<TcAuditOperations*>::iterator it = 
                                  drv_audit_req_queue_.begin();

  if (it != drv_audit_req_queue_.end()) {
    pfc_log_info("%s Sending signal for primary waiting audit",
                 __FUNCTION__);
    pthread_mutex_lock(&(*it)->primary_wait_mutex_);
    (*it)->primary_wait_signalled = PFC_TRUE;
    ret = pthread_cond_signal(&((*it)->primary_wait_cond_));
    pthread_mutex_unlock(&(*it)->primary_wait_mutex_);
  } else {
    pfc_log_info("%s Audit wait queue empty", __FUNCTION__);
  }

  return ret;
}

int32_t
TcAuditOperations::SignalSecondaryWaitingAudit() {

  uint32_t ret = 0;
  pfc::core::ScopedMutex m(drv_audit_req_queue_lock_);

  std::deque<TcAuditOperations*>::iterator it = 
                                  drv_audit_req_queue_.begin();

  if (it != drv_audit_req_queue_.end()) {
    pfc_log_info("%s Sending signal for secondary waiting audit",
                 __FUNCTION__);
    pthread_mutex_lock(&(*it)->secondary_wait_mutex_);
    (*it)->secondary_wait_signalled_ = PFC_TRUE;
    ret = pthread_cond_signal(&((*it)->secondary_wait_cond_));
    pthread_mutex_unlock(&(*it)->secondary_wait_mutex_);
  } else {
    pfc_log_info("%s Audit wait queue empty", __FUNCTION__);
  }
  return ret;
}

}  // namespace  tc
}  // namespace unc
