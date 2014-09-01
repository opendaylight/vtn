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

#define UNC_CANDIDATE_OPS_ARG_COUNT 3


/*
 * @brief Constructor for TcCandidateOperations
 */
TcCandidateOperations::TcCandidateOperations(TcLock* tc_lock_,
                           pfc::core::ipc::ServerSession* sess_,
                           TcDbHandler* db_handler,
                           TcChannelNameMap& unc_map_)
    :TcOperations(tc_lock_, sess_, db_handler, unc_map_),
    config_id_(0),
    resp_tc_msg_(NULL),
    trans_result_(unc::tclib::TRANS_END_FAILURE),
    autosave_enabled_(PFC_FALSE),
    user_response_(TC_OPER_SUCCESS) {}

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
  if ( avail_count != UNC_CANDIDATE_OPS_ARG_COUNT ) {
    pfc_log_error("TcCheckOperArgCount args expected(%u) received(%u)",
                  UNC_CANDIDATE_OPS_ARG_COUNT, avail_count);
    return TC_OPER_INVALID_INPUT;
  }
  return TC_OPER_SUCCESS;
}

/*
 * @brief Validate the Oper types in input
 */
TcOperStatus TcCandidateOperations::TcValidateOperType() {
  if ((tc_oper_ != TC_OP_CANDIDATE_ABORT) &&
      (tc_oper_ != TC_OP_CANDIDATE_COMMIT)) {
    pfc_log_error("TcValidateOperType: oper != TC_OP_CANDIDATE_ABORT or "
                  "TC_OP_CANDIDATE_COMMIT");
    return TC_INVALID_OPERATION_TYPE;
  }

  /*set IPC timeout to infinity for candidate operations*/
  TcUtilRet ret = TcServerSessionUtils::set_srv_timeout(ssess_, NULL);
  if (ret == TCUTIL_RET_FAILURE) {
    return TC_OPER_FAILURE;
  }
  return TC_OPER_SUCCESS;
}

/*
 * @brief Validate the Oper Paramaters
 */
TcOperStatus TcCandidateOperations::TcValidateOperParams() {
  pfc_log_info("candidate_oper:Collect Config ID");
  TcUtilRet ret= TcServerSessionUtils::get_uint32(ssess_,
                                              TC_REQ_ARG_INDEX, &config_id_);
  if ( ret != TCUTIL_RET_SUCCESS ) {
    return TC_OPER_FAILURE;
  }
  uint32_t validate_session_id = 0, validate_config_id = 0;
  pfc_log_info("candidate_oper:validate config ID and session ID");
  TcLockRet LockRet=
      tclock_->GetConfigIdSessionId(&validate_session_id,
                                    &validate_config_id);
  if (LockRet != TC_LOCK_SUCCESS) {
    return HandleLockRet(LockRet);
  }
  pfc_log_info("candidate_oper:check actual values");
  if ( validate_session_id != session_id_ ) {
    return TC_INVALID_SESSION_ID;
  } else if ( validate_config_id != config_id_ ) {
      return TC_INVALID_CONFIG_ID;
  }
  return TC_OPER_SUCCESS;
}

/*
 * @brief Secure Exclusion
 */
TcOperStatus TcCandidateOperations::TcGetExclusion() {
  if ( tc_oper_ == TC_OP_CANDIDATE_COMMIT ) {
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
  if ( tc_oper_ == TC_OP_CANDIDATE_COMMIT ) {
    TcLockRet ret = TC_LOCK_FAILURE;
    ret = tclock_->ReleaseLock(session_id_,
                              config_id_,
                              TC_RELEASE_WRITE_SESSION,
                              TC_COMMIT);
    if (ret != TC_LOCK_SUCCESS) {
      return HandleLockRet(ret);
    }
  } else {
    TcLockRet ret = TC_LOCK_FAILURE;
    ret = tclock_->ReleaseLock(session_id_,
                              config_id_,
                              TC_RELEASE_WRITE_SESSION,
                              TC_ABORT_CANDIDATE_CONFIG);
    if (ret != TC_LOCK_SUCCESS) {
      return HandleLockRet(ret);
    }
  }

  if (tclock_->GetMarkedSessionId() != session_id_) {
    return TC_OPER_SUCCESS;
  }
  TcConfigOperations tc_config_oper(tclock_,
                                    NULL,
                                    NULL,
                                    unc_oper_channel_map_);
  tc_config_oper.session_id_ = session_id_;
  tc_config_oper.config_id_ = config_id_;
  tc_config_oper.tc_oper_ = TC_OP_CONFIG_RELEASE;
  return tc_config_oper.Dispatch();
}

/*
 * @brief  Crate the Message List for Operation
 */
TcOperStatus TcCandidateOperations::TcCreateMsgList() {
  TcOperRet ret=
      db_hdlr_->GetConfTable(&autosave_enabled_);
  if ( ret == TCOPER_RET_FAILURE ) {
    pfc_log_info("Database Read Failed, autodave disabled");
  }
  if (TcOperMessageList. empty()) {
    if (tc_oper_ == TC_OP_CANDIDATE_ABORT) {
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

  if ( oper_type == unc::tclib::MSG_COMMIT_TRANS_END ) {
    tc_msg->SetTransResult(trans_result_);
  }
  return TC_OPER_SUCCESS;
}

/*
 * @brief  Execute the Sequence of Messages.
 */
TcOperStatus TcCandidateOperations::Execute() {
  if ( tc_oper_ == TC_OP_CANDIDATE_COMMIT ) {
    if ( TransStartMsg() == PFC_FALSE ) {
       return user_response_;
    }
    if ( TransVoteMsg() == PFC_FALSE ) {
       return user_response_;
    }
    if ( TransGlobalCommitMsg() == PFC_FALSE ) {
       return user_response_;
    }
    if ( TransEndMsg() == PFC_FALSE ) {
       return user_response_;
    }
    return TC_OPER_SUCCESS;
  }
  if (db_hdlr_->UpdateRecoveryTable(UNC_DT_CANDIDATE,
                                     tc_oper_) != TCOPER_RET_SUCCESS) {
    pfc_log_info("Recovery Table not updated");
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
  if (db_hdlr_->UpdateRecoveryTable(UNC_DT_RUNNING,
                                   tc_oper_) != TCOPER_RET_SUCCESS) {
    pfc_log_info("Recovery Table not updated");
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
                                       TC_OP_INVALID)!= TCOPER_RET_SUCCESS) {
      pfc_log_info("Recovery Table not updated");
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

}  // namespace tc
}  // namespace unc
