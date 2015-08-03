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

#define UNC_CANDIDATE_OPS_ARG_COUNT 3
#define UNC_CANDIDATE_TIMED_OPS_ARG_COUNT 5

std::deque<TcCandidateOperations*> TcCandidateOperations::candidate_req_queue_;
pfc::core::Mutex TcCandidateOperations::candidate_req_queue_lock_;

/*
 * @brief Constructor for TcCandidateOperations
 */
TcCandidateOperations::TcCandidateOperations(TcLock* tc_lock_,
                           pfc::core::ipc::ServerSession* sess_,
                           TcDbHandler* db_handler,
                           TcChannelNameMap& unc_map_)
    :TcOperations(tc_lock_, sess_, db_handler, unc_map_),
    cancel_audit_(PFC_FALSE),
    config_id_(0),
    abort_version_(0),
    timeout_(0),
    resp_tc_msg_(NULL),
    trans_result_(unc::tclib::TRANS_END_FAILURE),
    autosave_enabled_(PFC_FALSE),
    user_response_(TC_OPER_SUCCESS) {
      pthread_cond_init(&que_cond_, NULL);
      pthread_mutex_init(&que_mutex_, NULL);
    }

TcCandidateOperations::~TcCandidateOperations() {
  if ( resp_tc_msg_ )
    delete resp_tc_msg_;
}


/*
 * @brief Return minimum argument count
 */
uint32_t TcCandidateOperations::TcGetMinArgCount() {
  return UNC_CANDIDATE_OPS_ARG_COUNT;
}

/*
 * @brief Handle return value from TcLock
 */
TcOperStatus TcCandidateOperations::HandleLockRet(TcLockRet lock_ret) {
  TcOperStatus ret = TC_OPER_FAILURE;

  switch ( lock_ret ) {
    case TC_LOCK_INVALID_UNC_STATE:
    case TC_LOCK_OPERATION_NOT_ALLOWED:
      ret = TC_INVALID_STATE;
      break;
    case TC_LOCK_BUSY:
      ret = TC_SYSTEM_BUSY;
      break;
    case TC_LOCK_NO_CONFIG_SESSION_EXIST:
      ret = TC_CONFIG_NOT_PRESENT;
      break;
    default:
      ret = TC_OPER_FAILURE;
  }
  pfc_log_info("HandleLockRet: Received(%u), return (%u)",
               lock_ret, ret); 
  return ret;
}

/*
 * @brief Check Count of Arguments
 */
TcOperStatus TcCandidateOperations::TcCheckOperArgCount(uint32_t avail_count) {
  if (tc_oper_ == TC_OP_CANDIDATE_COMMIT ||
      tc_oper_ == TC_OP_CANDIDATE_ABORT) {
    if (avail_count != UNC_CANDIDATE_OPS_ARG_COUNT ) {
      pfc_log_error("TcCheckOperArgCount args expected(%u) received(%u)",
                    UNC_CANDIDATE_OPS_ARG_COUNT, avail_count);
      return TC_OPER_INVALID_INPUT;
    }
  } else if (tc_oper_ == TC_OP_CANDIDATE_COMMIT_TIMED ||
             tc_oper_ == TC_OP_CANDIDATE_ABORT_TIMED) {
    if (avail_count != UNC_CANDIDATE_TIMED_OPS_ARG_COUNT) {
      pfc_log_error("TcCheckOperArgCount args expected(%u) received(%u)",
                    UNC_CANDIDATE_TIMED_OPS_ARG_COUNT, avail_count);
      return TC_OPER_INVALID_INPUT;
    }
  }

  return TC_OPER_SUCCESS;
}

/*
 * @brief Validate the Oper types in input
 */
TcOperStatus TcCandidateOperations::TcValidateOperType() {
  if (tc_oper_ != TC_OP_CANDIDATE_ABORT &&
      tc_oper_ != TC_OP_CANDIDATE_ABORT_TIMED &&
      tc_oper_ != TC_OP_CANDIDATE_COMMIT &&
      tc_oper_ != TC_OP_CANDIDATE_COMMIT_TIMED) {
    pfc_log_error("TcValidateOperType: oper != TC_OP_CANDIDATE_ABORT or "
                  "TC_OP_CANDIDATE_COMMIT or TC_OP_CANDIDATE_ABORT_TIMED or "
                  "TC_OP_CANDIDATE_COMMIT_TIMED");
    return TC_INVALID_OPERATION_TYPE;
  }

  /*set IPC timeout to infinity for candidate operations*/
  TcUtilRet ret = TcServerSessionUtils::set_srv_timeout(ssess_, NULL);
  if (ret == TCUTIL_RET_FAILURE) {
    pfc_log_error("TcValidateOperType: Cannot set IPC timeout to Infinite "
                  "for candidate operation");
    return TC_OPER_FAILURE;
  }
  return TC_OPER_SUCCESS;
}

/*
 * @brief Validate the Oper Paramaters
 */
TcOperStatus TcCandidateOperations::TcValidateOperParams() {
  pfc_log_debug("candidate_oper:Collect Config ID");
  TcUtilRet ret= TcServerSessionUtils::get_uint32(ssess_,
                                              TC_REQ_ARG_INDEX, &config_id_);
  if ( ret != TCUTIL_RET_SUCCESS ) {
    pfc_log_error("%s Cannot read config_id from session", __FUNCTION__);
    return TC_OPER_FAILURE;
  }

  pfc_log_debug("candidate_oper:validate config ID and session ID");
  
  uint32_t validate_config_id = 0;
  TcConfigMode config_mode;
  std::string vtn_name;

  TcLockRet LockRet=
  tclock_->GetConfigData(session_id_,
                         validate_config_id,
                         config_mode,
                         vtn_name);

  pfc_log_debug("candidate_oper:check actual values");
  if (LockRet == TC_LOCK_INVALID_SESSION_ID) {
    pfc_log_error("TcValidateOperParams:: Invalid session-id[%u]", session_id_);
    return TC_INVALID_SESSION_ID;
  } else if (validate_config_id != config_id_ ) {
      pfc_log_error("TcValidateOperParams::Invalid conf-id[%u] expecting [%u]",
                    config_id_, validate_config_id);
      return TC_INVALID_CONFIG_ID;
  }

  if (LockRet != TC_LOCK_SUCCESS) {
    pfc_log_error("Error validating session/config data. Ret=%d", LockRet);
    return HandleLockRet(LockRet);
  }

  if (tc_oper_ == TC_OP_CANDIDATE_COMMIT_TIMED ||
      tc_oper_ == TC_OP_CANDIDATE_ABORT_TIMED) {
    ret = TcServerSessionUtils::get_int32(ssess_,
                                          TC_REQ_ARG_INDEX + 1,
                                          &timeout_);
    if (ret != TCUTIL_RET_SUCCESS) {
      pfc_log_error("TcValidateOperParams: Timeout missing for "
                    "candidate timed operation");
      return TC_OPER_FAILURE;
    }

    uint8_t cancel_audit = 0;
    ret = TcServerSessionUtils::get_uint8(ssess_,
                                          TC_REQ_ARG_INDEX + 2,
                                          &cancel_audit);
    if (ret != TCUTIL_RET_SUCCESS) {
      pfc_log_error("TcValidateOperParams: CancelAudit missing for "
                    "candidate timed operation");
      return TC_OPER_FAILURE;
    }
    cancel_audit_ = (pfc_bool_t)cancel_audit;
  }
  return TC_OPER_SUCCESS;
}

/*
 * @brief Secure Exclusion
 */
TcOperStatus TcCandidateOperations::TcGetExclusion() {
  if (tc_oper_ == TC_OP_CANDIDATE_COMMIT ||
      tc_oper_ == TC_OP_CANDIDATE_COMMIT_TIMED) {
    TcLockRet ret = TC_LOCK_FAILURE;
    ret = tclock_->GetLock(session_id_,
                          TC_ACQUIRE_WRITE_SESSION,
                          TC_COMMIT);
    if (ret != TC_LOCK_SUCCESS) {
       return HandleLockRet(ret);
     }
  } else {
    TcLockRet ret = TC_LOCK_FAILURE;
    ret = tclock_->GetLock(session_id_,
                           TC_ACQUIRE_WRITE_SESSION,
                           TC_ABORT_CANDIDATE_CONFIG);
    if (ret != TC_LOCK_SUCCESS) {
       return HandleLockRet(ret);
    }
  }
  return TC_OPER_SUCCESS;
}

/*
 * @brief Release Exclusion
 */
TcOperStatus TcCandidateOperations::TcReleaseExclusion() {
  if (tc_oper_ == TC_OP_CANDIDATE_COMMIT ||
      tc_oper_ == TC_OP_CANDIDATE_COMMIT_TIMED) {
    TcLockRet ret = TC_LOCK_FAILURE;
    ret = tclock_->ReleaseLock(session_id_,
                              config_id_,
                              TC_RELEASE_WRITE_SESSION,
                              TC_COMMIT);
    if (ret != TC_LOCK_SUCCESS) {
      return HandleLockRet(ret);
    }
  } else if (tc_oper_ == TC_OP_CANDIDATE_ABORT ||
             tc_oper_ == TC_OP_CANDIDATE_ABORT_TIMED) {
    TcLockRet ret = TC_LOCK_FAILURE;
    ret = tclock_->ReleaseLock(session_id_,
                              config_id_,
                              TC_RELEASE_WRITE_SESSION,
                              TC_ABORT_CANDIDATE_CONFIG);
    if (ret != TC_LOCK_SUCCESS) {
      return HandleLockRet(ret);
    }
  } else {
    pfc_log_error("%s Invalid operation %u", __FUNCTION__, tc_oper_);
  }

  if (tclock_->GetMarkedSessionId(session_id_) != session_id_) {
    // After release of current candidate operation, trigger next waiting op
    HandleCandidateRelease();
    return TC_OPER_SUCCESS;
  }
  TcConfigOperations tc_config_oper(tclock_,
                                    NULL,
                                    NULL,
                                    unc_oper_channel_map_);
  tc_config_oper.session_id_ = session_id_;
  tc_config_oper.config_id_ = config_id_;
  tc_config_oper.tc_oper_ = TC_OP_CONFIG_RELEASE;
  TcOperStatus conf_ret = tc_config_oper.Dispatch();

  // After release of current candidate operation, trigger next waiting op
  HandleCandidateRelease();

  return conf_ret;
}

/*
 * @brief  Crate the Message List for Operation
 */
TcOperStatus TcCandidateOperations::TcCreateMsgList() {
  TcOperRet ret=
      db_hdlr_->GetConfTable(&autosave_enabled_);
  if ( ret == TCOPER_RET_FAILURE ) {
    pfc_log_warn("Database Read Failed, autosave disabled");
  }
  if (TcOperMessageList. empty()) {
    if (tc_oper_ == TC_OP_CANDIDATE_ABORT ||
        tc_oper_ == TC_OP_CANDIDATE_ABORT_TIMED) {
        TcOperMessageList.push_back(unc::tclib::MSG_ABORT_CANDIDATE);
    }
  }
  return TC_OPER_SUCCESS;
}

/*
 * @brief  Fill the Message to forward to other modules.
 */
TcOperStatus TcCandidateOperations::FillTcMsgData(TcMsg* tc_msg,
                           TcMsgOperType oper_type) {
  if (tc_msg == NULL )
    return TC_OPER_FAILURE;

  tc_msg->SetData(config_id_, "", UNC_CT_UNKNOWN);

  tc_msg->SetData(UNC_DT_CANDIDATE, tc_oper_, abort_version_);
 
  if ( oper_type == unc::tclib::MSG_COMMIT_TRANS_END ) {
    tc_msg->SetTransResult(trans_result_);
  }
  return TC_OPER_SUCCESS;
}

/*
 * @brief  Execute the Sequence of Messages.
 */
TcOperStatus TcCandidateOperations::Execute() {
  if (tc_oper_ == TC_OP_CANDIDATE_COMMIT ||
      tc_oper_ == TC_OP_CANDIDATE_COMMIT_TIMED) {
    if ( TransStartMsg() == PFC_FALSE ) {
       return user_response_;
    }
    if ( TransVoteMsg() == PFC_FALSE ) {
       return user_response_;
    }
    pfc_bool_t autosave_enabled = PFC_FALSE;
    TcOperRet ret= 
          db_hdlr_->GetConfTable(&autosave_enabled);
    if ( ret == TCOPER_RET_FAILURE ) {
      pfc_log_error("Database Read Failed, autodave disabled");
    }

    TcReadStatusOperations::SetRunningStatus();
    if (autosave_enabled) {
      TcReadStatusOperations::SetStartupStatus();
    }
    if ( TransGlobalCommitMsg() == PFC_FALSE ) {
       return user_response_;
    }
    TcReadStatusOperations::SetRunningStatusIncr();
    if (autosave_enabled) {
      TcReadStatusOperations::SetStartupStatusIncr();
    }

    if ( TransEndMsg() == PFC_FALSE ) {
       return user_response_;
    }
    return TC_OPER_SUCCESS;
  }

  // Abort operation handling
  uint32_t config_id = 0;
  TcConfigMode config_mode = TC_CONFIG_INVALID;
  std::string vtn_name;

  if (tclock_->GetConfigData(session_id_,
                             config_id,
                             config_mode,
                             vtn_name) != TC_LOCK_SUCCESS) {
    pfc_log_warn("%s Failed to get config data for session[%u]",
                 __FUNCTION__, session_id_);
  }

  if (db_hdlr_->UpdateRecoveryTable(UNC_DT_CANDIDATE,
                                    TC_OP_CANDIDATE_ABORT,
                                    config_mode,
                                    vtn_name) != TCOPER_RET_SUCCESS) {
    pfc_log_warn("Recovery Table not updated");
  }
  
  if (db_hdlr_->GetRecoveryTableAbortVersion(abort_version_) 
                                  != TCOPER_RET_SUCCESS) {
    pfc_log_warn("Retrieving abort_version data from recovery table failed");
  } else {
    ++abort_version_;
    pfc_log_info("Sending abort version: %"PFC_PFMT_u64, abort_version_);
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
    TcOperRet MsgRet = tcmsg_->Execute();
    if (MsgRet != TCOPER_RET_SUCCESS) {
      user_response_ = HandleMsgRet(MsgRet);
    } else  {
      pfc_log_debug("Candidate abort:Setting abort version:%"PFC_PFMT_u64,
                   abort_version_);
      if (db_hdlr_->UpdateRecoveryTableAbortVersion(abort_version_)
                                          != TCOPER_RET_SUCCESS) {
        pfc_log_warn("Setting abort_version to recovery table failed");
      }
    }

    MsgIter++;
    delete tcmsg_;
    tcmsg_ = NULL;
  }

  return user_response_;
}



/*
 * @brief  Transaction START handler.
 */
pfc_bool_t TcCandidateOperations::TransStartMsg() {
  TcMsg* tc_start_msg= TcMsg::CreateInstance(session_id_,
                                             unc::tclib::MSG_COMMIT_TRANS_START,
                                             unc_oper_channel_map_);
  PFC_VERIFY(tc_start_msg != NULL);
  FillTcMsgData(tc_start_msg, unc::tclib::MSG_COMMIT_TRANS_START);
  TcOperRet oper_ret(tc_start_msg->Execute());
  if ( oper_ret != TCOPER_RET_SUCCESS ) {
    user_response_ = HandleMsgRet(oper_ret);
    resp_tc_msg_ = tc_start_msg;
    return PFC_FALSE;
  }
  delete tc_start_msg;
  return PFC_TRUE;
}

/*
 * @brief  Transaction VOTE handler.
 */
pfc_bool_t TcCandidateOperations::TransVoteMsg() {
  TcMsg* tc_vote_msg = TcMsg::CreateInstance(session_id_,
                                             unc::tclib::MSG_COMMIT_VOTE,
                                             unc_oper_channel_map_);

  PFC_VERIFY(tc_vote_msg != NULL);
  FillTcMsgData(tc_vote_msg, unc::tclib::MSG_COMMIT_VOTE);
  TcOperRet oper_ret(tc_vote_msg->Execute());
  if ( oper_ret != TCOPER_RET_SUCCESS ) {
    if ( oper_ret == TCOPER_RET_FATAL ) {
      user_response_ = HandleMsgRet(oper_ret);
      resp_tc_msg_ = tc_vote_msg;
      return PFC_FALSE;
    }
    if ( TransEndMsg() != PFC_FALSE ) {
      user_response_ = HandleMsgRet(oper_ret);
      resp_tc_msg_ = tc_vote_msg;
      return PFC_FALSE;
    } else {
      delete tc_vote_msg;
      return PFC_FALSE;
    }
  }
  delete tc_vote_msg;
  return PFC_TRUE;
}

/*
 * @brief  Transaction GLOBAL COMMIT handler.
 */
pfc_bool_t TcCandidateOperations::TransGlobalCommitMsg() {

  uint32_t config_id = 0;
  TcConfigMode config_mode = TC_CONFIG_INVALID;
  std::string vtn_name;

  if (tclock_->GetConfigData(session_id_,
                             config_id,
                             config_mode,
                             vtn_name) != TC_LOCK_SUCCESS) {
    pfc_log_warn("%s Failed to get config data for session[%u]",
                 __FUNCTION__, session_id_);
  }

  if (db_hdlr_->UpdateRecoveryTable(UNC_DT_RUNNING,
                                    TC_OP_CANDIDATE_COMMIT,
                                    config_mode,
                                    vtn_name) != TCOPER_RET_SUCCESS) {
    pfc_log_warn("Recovery Table not updated");
  }

  TcMsg* tc_commit_msg= TcMsg::CreateInstance(session_id_,
                                             unc::tclib::MSG_COMMIT_GLOBAL,
                                             unc_oper_channel_map_);

  PFC_VERIFY(tc_commit_msg != NULL);
  FillTcMsgData(tc_commit_msg, unc::tclib::MSG_COMMIT_GLOBAL);
  TcOperRet oper_ret(tc_commit_msg->Execute());
  trans_result_ = tc_commit_msg->GetTransResult();
  if ( oper_ret != TCOPER_RET_SUCCESS ) {
    user_response_ = TC_SYSTEM_FAILURE;
    delete tc_commit_msg;
    return PFC_FALSE;
  }
  resp_tc_msg_ = tc_commit_msg;
  return PFC_TRUE;
}

/*
 * @brief  Transaction END handler.
 */
pfc_bool_t TcCandidateOperations::TransEndMsg() {
  TcMsg* tc_end_msg= TcMsg::CreateInstance(session_id_,
                                           unc::tclib::MSG_COMMIT_TRANS_END,
                                           unc_oper_channel_map_);

  PFC_VERIFY(tc_end_msg != NULL);
  FillTcMsgData(tc_end_msg, unc::tclib::MSG_COMMIT_TRANS_END);
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
 * @brief Write ConfigId to output 
 */
TcOperStatus TcCandidateOperations::SetConfigId() {
  TcUtilRet ret=
      TcServerSessionUtils::set_uint32(ssess_,
                                       config_id_);
  if ( ret != TCUTIL_RET_SUCCESS ) {
    return TC_OPER_FAILURE;
  }
  return TC_OPER_SUCCESS;
}

/*
 * @brief Write Response to Client
 */
TcOperStatus
TcCandidateOperations::SendResponse(TcOperStatus oper_stat) {
  if ( ssess_ == NULL )
    return oper_stat;

  pfc_log_info("send_oper_status value=%d", oper_stat);
  if ( oper_stat != TC_OPER_INVALID_INPUT  &&
       oper_stat != TC_OPER_FAILURE ) {
    if (SetOperType() != TC_OPER_SUCCESS) {
      return TC_OPER_FAILURE;
    }
    if (SetSessionId() != TC_OPER_SUCCESS) {
      return TC_OPER_FAILURE;
    }
    if (SetConfigId() != TC_OPER_SUCCESS) {
      return TC_OPER_FAILURE;
    }
    if (SetOperStatus(oper_stat) != TC_OPER_SUCCESS) {
      return TC_OPER_FAILURE;
    }
  }
  if ( resp_tc_msg_ != NULL ) {
    TcOperRet ret(resp_tc_msg_->ForwardResponseToVTN(*ssess_));
    if ( ret != TCOPER_RET_SUCCESS )
      return TC_SYSTEM_FAILURE;
  }
  if ( oper_stat == TC_OPER_SUCCESS ) {
    if (db_hdlr_->UpdateRecoveryTable(UNC_DT_INVALID,
                                       TC_OP_INVALID,
                                       TC_CONFIG_INVALID,
                                       "")!= TCOPER_RET_SUCCESS) {
      pfc_log_warn("Recovery Table not reset");
    }
  }
  return oper_stat;
}

/*
 * @brief Write Additional Response to Client(Not Used)
 */
TcOperStatus
TcCandidateOperations::SendAdditionalResponse(TcOperStatus oper_stat) {
  return oper_stat;
}

TcOperStatus TcCandidateOperations::Dispatch() {

  TcOperStatus ret;
  tc_oper_status_ = INPUT_VALIDATION;

  pfc_log_debug("tccandidateoper:Read Input Paramaters");
  ret = HandleArgs();
  if (ret != TC_OPER_SUCCESS) {
    return RevokeOperation(ret);
  }

  //Send signal to audit secondary wait
  if (cancel_audit_ == PFC_TRUE) {
    pfc_log_info("%s Candidate request with cancel_audit option."
                 "Move any secondary waiting audit to primary wait",
                  __FUNCTION__);
    int32_t sig_ret = TcAuditOperations::SignalSecondaryWaitingAudit();
    pfc_log_info("%s Signal Audit secondary waiting return %d",
                 __FUNCTION__, sig_ret);
  }

  pfc::core::ScopedMutex m(TcCandidateOperations::candidate_req_queue_lock_);
  pfc_bool_t IsCandidateQueueEmpty =
                         TcCandidateOperations::candidate_req_queue_.empty();
  m.unlock();

  if ((tc_oper_ == TC_OP_CANDIDATE_COMMIT ||
       tc_oper_ == TC_OP_CANDIDATE_ABORT) &&
      !IsCandidateQueueEmpty) {
    pfc_log_error("%s Candidate wait queue not empty. Return BUSY",
                  __FUNCTION__);
    return RevokeOperation(TC_SYSTEM_BUSY);
  }

  if ( tc_oper_ == TC_OP_CANDIDATE_COMMIT_TIMED ||
       tc_oper_ == TC_OP_CANDIDATE_ABORT_TIMED ) {
    ret = HandleCandidateTimedRequest();
    if (ret != TC_OPER_SUCCESS) {
      return RevokeOperation(ret);
    }
  }

  if (tc_oper_ != TC_OP_CANDIDATE_COMMIT_TIMED &&
      tc_oper_ != TC_OP_CANDIDATE_ABORT_TIMED) {
    pfc_log_info("tc_oper:Secure Exclusion");
    tc_oper_status_ = GET_EXCLUSION_PHASE;
    ret = TcGetExclusion();
    if ( ret != TC_OPER_SUCCESS ) {
      return RevokeOperation(ret);
    }
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

    
/* @brief Handler for Timed candidate  acquisition requests */
TcOperStatus TcCandidateOperations::HandleCandidateTimedRequest() {
  pfc::core::ScopedMutex m(candidate_req_queue_lock_);
  int32_t pthread_ret = 0;
  TcOperRet ret = TCOPER_RET_SUCCESS;
  TcOperStatus oper_ret = TC_OPER_SUCCESS;
  pfc_bool_t is_infinite = PFC_FALSE;
  pfc_timespec_t  timeout;

  if (cancel_audit_ == PFC_TRUE) {

    pfc::core::ScopedMutex cand_excl(TcOperations::candidate_audit_excl_lock_);

    TcWriteOperation  curr_op = tclock_->TcGetCurrentWriteOperation();
    if (curr_op == TC_AUDIT_USER || curr_op == TC_AUDIT_DRIVER) {
      pfc_log_info("%s Current Write lock held by Audit[%d]."
                   "Setting CancelAudit flag",
                   __FUNCTION__, curr_op);
      TcMsg::SetAuditCancelFlag(PFC_TRUE);
    }

    if (GetAuditPhase() == AUDIT_NOT_STARTED) {
      pfc_log_info("%s: Audit not yet started. "
                   "Cancel Audit message not required", __FUNCTION__);
    } else if (GetAuditCancelStatus() == AUDIT_CANCEL_INPROGRESS) {
      pfc_log_info("%s: Audit Cancel already in-progres. "
                   "No Cancel Audit", __FUNCTION__);
    } else if (GetAuditPhase() > AUDIT_VOTE_REQUEST) {
      pfc_log_info("%s:Audit completed VOTE phase. "
                   "Cannot Cancel Audit", __FUNCTION__);
    } else {
      pfc_log_info("%s:CancelAudit conditions matching."
                   "Cancelling running Audit", __FUNCTION__);
      TcMsg::SetAuditCancelFlag(PFC_TRUE);
      SetAuditCancelStatus(AUDIT_CANCEL_INPROGRESS);

      cand_excl.unlock();

      ret = HandleCancelAudit();

      //Cancel Audit not successful case
      if ( ret != TCOPER_RET_SUCCESS ) {
        pfc_log_warn("%s Cancel Audit request failed", __FUNCTION__);
      }
    }
  }
  
  /* Timeout is set to zero */
  if (timeout_ == 0) {
    pfc_log_info("%s Candidate operation (sess=%u) with timeout = 0",
                 __FUNCTION__, session_id_);
    if (!candidate_req_queue_.empty()) {
      pfc_log_info("%s Candidate wait queue not empty. Ret BUSY", __FUNCTION__);
      return TC_SYSTEM_BUSY;
    }

    pfc_log_info("tc_CandidateOper:Secure Exclusion");
    tc_oper_status_ = GET_EXCLUSION_PHASE;
    oper_ret = TcGetExclusion();
    if ( oper_ret != TC_OPER_SUCCESS ) {
      return oper_ret;
    }
    pfc_log_info("%s GetLock OK. Proceed with execution", __FUNCTION__);
    return TC_OPER_SUCCESS;
  
  } else if (timeout_ > 0) {
    pfc_log_info("%s Candidate operation (sess=%u) with timeout = %u ms",
                 __FUNCTION__, session_id_, timeout_);
    //Commit request need to wait till the specific timeout_
    //Convert timeout value to seconds and nano-seconds
    struct timeval now;
    gettimeofday(&now, NULL);

    uint32_t millisec = 1000;
    uint64_t milli_to_nano = 1000000;

    timeout.tv_sec  = timeout_ / millisec;
    timeout.tv_nsec = (timeout_ % millisec) * milli_to_nano;

    pfc_log_debug("%s Starting timer with %ld sec, %ld nsec",
                 __FUNCTION__, timeout.tv_sec, timeout.tv_nsec);

    timeout.tv_sec += now.tv_sec;
    timeout.tv_nsec += now.tv_usec * millisec;

    timeout.tv_sec += timeout.tv_nsec / 1000000000L;
    timeout.tv_nsec = timeout.tv_nsec % 1000000000L;
  } else {
    pfc_log_info("%s Candidate operation (sess=%u) with infinite timeout",
                 __FUNCTION__, session_id_);
    is_infinite = PFC_TRUE;
  }

  // If wait queue is empty, try GetExclusion. If success, return without insert
  if (candidate_req_queue_.empty()) {
    tc_oper_status_ = GET_EXCLUSION_PHASE;
    oper_ret = TcGetExclusion();
    if ( oper_ret == TC_OPER_SUCCESS ) {
      pfc_log_info("%s Got Lock. Candidate operation can execute. Not waiting.",
                   __FUNCTION__);
      return oper_ret;
    } else {
      tc_oper_status_ = INPUT_VALIDATION; // Revert back oper_status
      pfc_log_info("%s Cannot acquire lock. Another write op running? "
                   "Moving to wait mode", __FUNCTION__);
    }
  }

  // Push into wait queue
  InsertCandidateRequest();

  pthread_mutex_lock(&que_mutex_);
  //write lock is not available, wait for it
  pfc_log_info("Waiting for write lock; sess[%u]", session_id_);
  m.unlock();
  if (is_infinite) {
    pthread_ret = pthread_cond_wait(&que_cond_, &que_mutex_);
  } else {
    pthread_ret = pthread_cond_timedwait(&que_cond_, &que_mutex_, &timeout);
  }
  pthread_mutex_unlock(&que_mutex_);

  pfc_log_info("%s pthread_cond_wait [%u] return %d",
               __FUNCTION__, session_id_, pthread_ret);
  if (pthread_ret != 0) {
    //returns Non-zero indicates timeout
    pfc::core::ScopedMutex rem_entry(candidate_req_queue_lock_);
    RemoveCandidateRequest(session_id_);
    rem_entry.unlock();
    return TC_SYSTEM_BUSY;
  }
  
  if (TcOperations::IsStateChangedToSby()) {
    pfc_log_info("%s Sess[%u] While waiting for write lock state changed",
                  __FUNCTION__, session_id_);
    return TC_STATE_CHANGED;
  }
           
  m.unlock(); //Unlock if any lock pending

  pfc::core::ScopedMutex m2(candidate_req_queue_lock_);// Acquire fresh lock
  pfc_log_info("Sess[%u] Wait complete. Execute candidate op", session_id_);
  pfc_log_info("tc_CandidateOper:Secure Exclusion");
  tc_oper_status_ = GET_EXCLUSION_PHASE;
  oper_ret = TcGetExclusion();
  
  
  // Pop the first waiting timed req (currently processed)  
  RemoveCandidateRequest(session_id_);
  return oper_ret;
}

TcOperRet TcCandidateOperations::HandleCancelAudit() {
  TcUtilRet util_resp = TCUTIL_RET_SUCCESS;
  pfc_ipcresp_t resp = 0;
  pfc_ipcconn_t conn = 0;
  TcOperRet ret_val = TCOPER_RET_SUCCESS;

  pfc::core::ScopedMutex m(audit_cancel_notify_lock_);	

  for (std::vector<TcDaemonName>::iterator
            list_iter = audit_cancel_notify_.begin();
            list_iter != audit_cancel_notify_.end();
            list_iter++) {
    std::string channel_name = GetDaemonName(*list_iter);

    if (channel_name.empty()) {
      pfc_log_error("%s::Cannot get ChannelName for %d",
                    __FUNCTION__, *list_iter);
      return TCOPER_RET_FAILURE;
    }

    pfc_log_info("%s Sending MSG_CANCEL_AUDIT to %s",
                 __FUNCTION__, channel_name.c_str());
   
    pfc::core::ipc::ClientSession* sess = NULL;
    sess = TcClientSessionUtils::create_tc_client_session(
                                             channel_name,
                                             tclib::TCLIB_AUDIT_CANCEL,
                                             conn);
    if ( sess == NULL ) {
      pfc_log_error("%s: sess creation failed", __FUNCTION__);
      return TCOPER_RET_FAILURE;
    }
    
    /* Append Data */

    //oper_type    
    util_resp = tc::TcClientSessionUtils::set_uint8(sess,
                                                    tclib::MSG_AUDIT_CANCEL);
    if (PFC_EXPECT_TRUE(util_resp != TCUTIL_RET_SUCCESS)) {
      pfc_log_error("%s: Setting oper failed", __FUNCTION__);
      return GetUtilResp(util_resp);
    }
    
    //Session_id  
    util_resp = TcClientSessionUtils::set_uint32(sess, 0);
    if (PFC_EXPECT_TRUE(util_resp != TCUTIL_RET_SUCCESS)) {
      pfc_log_error("%s::Set sess_id failed", __FUNCTION__);
      return GetUtilResp(util_resp);
    }
    
    //driver_id
    unc_keytype_ctrtype_t driver_id = TcMsg::GetNotifyDriverId();
    if (driver_id == UNC_CT_UNKNOWN) {
      pfc_log_error("%s Invalid driver-id", __FUNCTION__);
      return TCOPER_RET_FAILURE;
    } else {
      util_resp = TcClientSessionUtils::set_uint8(sess, driver_id);

      if (PFC_EXPECT_TRUE(util_resp != TCUTIL_RET_SUCCESS)) {
        pfc_log_error("%s::Set driver_id(%u) failed",
                      __FUNCTION__, driver_id);
        return GetUtilResp(util_resp);
      }
    }

    //controller_id
    std::string ctr_id = TcMsg::GetNotifyControllerId();
    if (ctr_id.empty()) {
      pfc_log_error("%s Controller-id empty", __FUNCTION__);
      return TCOPER_RET_FAILURE;
    } else {
      util_resp = TcClientSessionUtils::set_string( sess, ctr_id); 
      if (PFC_EXPECT_TRUE(util_resp != TCUTIL_RET_SUCCESS)) {
       pfc_log_error("%s::Set controller_id(%s) failed",
                     __FUNCTION__, ctr_id.c_str());
       return GetUtilResp(util_resp);
      }
    }
    
    /* invoke-Session */
    util_resp = tc::TcClientSessionUtils::tc_session_invoke(sess, resp);
    if (PFC_EXPECT_TRUE(util_resp != TCUTIL_RET_SUCCESS)) {
      pfc_log_error("Session invoke failed");
      return GetUtilResp(util_resp);
    }

    TcClientSessionUtils::tc_session_close(&sess, conn);
    
    /* Handle Server Response */  
    if (PFC_EXPECT_TRUE(resp == tclib::TC_SUCCESS)) {
      pfc_log_info("Success response from %s", channel_name.c_str());
      ret_val = TCOPER_RET_SUCCESS;
    } else if (PFC_EXPECT_TRUE(resp == tclib::TC_FAILURE)) {
      pfc_log_error("Failure response from %s", channel_name.c_str());
      ret_val = TCOPER_RET_ABORT;
      break;
    }
  }
  /*clear map*/
  audit_cancel_notify_.clear();
  return ret_val;
}


// Insert the candidate request into the queue
void TcCandidateOperations::InsertCandidateRequest() {
  pfc_log_debug("TcCandidateOperations::InsertCandidateRequest");
  candidate_req_queue_.push_back(this);
  pfc_log_info("InsertCommitRequest current wait queue size:%" PFC_PFMT_SIZE_T,
                                                  candidate_req_queue_.size());
}


void TcCandidateOperations::HandleCandidateRelease() {
  pfc_log_debug("HandleCandidateRelease");
  pfc::core::ScopedMutex m(candidate_req_queue_lock_);
  //Get next item from wait queue.
  TcCandidateOperations * tcop = 
                            TcCandidateOperations::RetrieveCandidateRequest();
  if (tcop) {
    //Get condition variable and signal it.
    pthread_mutex_lock(&tcop->que_mutex_);
    pfc_log_info("Signal first [%u] waiting timed_candidate request",
                 tcop->session_id_);
    int ret = pthread_cond_signal(&tcop->que_cond_);
    pthread_mutex_unlock(&tcop->que_mutex_);
    if (ret != 0) {
      pfc_log_error("Error signalling condition for session_id: %u",
                                                   tcop->session_id_);
    }
    return;
  }

  // If candidate queue is empty, trigger any audit request waiting
  pfc_log_info("%s Candidate queue empty. Trigger waiting audit if any.",
               __FUNCTION__);
  int32_t ret = TcAuditOperations::SignalPrimaryWaitingAudit();
  if (ret != 0) {
    pfc_log_warn("%s SignalFirstWaitingAudit ret = %d",
                 __FUNCTION__, ret);
  }
}

void TcCandidateOperations::ClearCandidateQueue() {
  pfc_log_debug("TcCandidateOperations::ClearCandidateQueue");
  pfc::core::ScopedMutex m(candidate_req_queue_lock_);
  while (!candidate_req_queue_.empty()) {

    TcCandidateOperations * candop = candidate_req_queue_.front();
    if(candop) {
      pthread_mutex_lock(&candop->que_mutex_);
      pfc_log_info("ClearCandidateQueue processing sess[%u]",
                   candop->session_id_);
      // Call cond_signal
      int32_t ret = pthread_cond_signal(&candop->que_cond_);
      pthread_mutex_unlock(&candop->que_mutex_);
      if (ret != 0) {
        pfc_log_error("Error signalling condition for session_id: %u",
                      candop->session_id_);
      }
    }
    candidate_req_queue_.pop_front();
  }
}

void TcCandidateOperations::RemoveCandidateRequest(uint32_t sess_id) {
  pfc_log_debug("RemoveCandidateRequest for sess[%u]",sess_id);
  std::deque<TcCandidateOperations*>::iterator it;
  for(it = candidate_req_queue_.begin();
      it != candidate_req_queue_.end();
      it++) {
    if ((*it)->session_id_ == sess_id) {
      pfc_log_debug("Found. Removing for sess[%d] done", sess_id);
      candidate_req_queue_.erase(it);
      break;
    }
  }
  pfc_log_info("RemoveCandidateRequest current wait queue size:%"
               PFC_PFMT_SIZE_T, candidate_req_queue_.size());
}


TcCandidateOperations * TcCandidateOperations::RetrieveCandidateRequest() {
  TcCandidateOperations * candidateoper = NULL;
  pfc_log_info("RetrieveCandidateRequest current wait queue size:%"
                    PFC_PFMT_SIZE_T, candidate_req_queue_.size());
  std::deque<TcCandidateOperations*>::iterator it= candidate_req_queue_.begin();
  if (it != candidate_req_queue_.end()) {
    candidateoper = *it;
    return candidateoper;
  }
  return NULL;
}


/*This method maps TcUtilRet ret to TcOperRet value.*/
TcOperRet
TcCandidateOperations::GetUtilResp(TcUtilRet ret) {
  TcOperRet oper_ret = TCOPER_RET_UNKNOWN;

  if (PFC_EXPECT_TRUE(TCUTIL_RET_SUCCESS == ret)) {
    oper_ret =  TCOPER_RET_SUCCESS;
  } else if (PFC_EXPECT_TRUE(TCUTIL_RET_FAILURE == ret)) {
    oper_ret =  TCOPER_RET_FAILURE;
  } else if (PFC_EXPECT_TRUE(TCUTIL_RET_FATAL == ret)) {
    oper_ret =  TCOPER_RET_FATAL;
  }
  pfc_log_info("ReturnUtilResp: Received(%u); Returned(%u)",
                      ret, oper_ret);
  return oper_ret;
}


/*!brief this method returns the channel name of daemon from TcChannelNameMap*/
std::string TcCandidateOperations::GetDaemonName(TcDaemonName daemon_id)  {
  std::string channel;

  if (PFC_EXPECT_TRUE(unc_oper_channel_map_.find(daemon_id)
                                          != unc_oper_channel_map_.end())) {
    channel = unc_oper_channel_map_.find(daemon_id)->second;
  } else {
    return "";
  }
  return channel;
}

}  // namespace tc
}  // namespace unc
