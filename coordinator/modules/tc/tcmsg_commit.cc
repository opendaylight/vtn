/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include "tcmsg_commit.hh"

namespace unc {
namespace tc {

/*!\brief Parameterized constructor of TcMsgCommit.
 *@param sess_id[in] - session identifier.
 *@param oper[in] - operation type
 **/
TcMsgCommit::TcMsgCommit(uint32_t sess_id,
                         tclib::TcMsgOperType oper)
    :TcMsg(sess_id, oper), config_id_(0) {}


/*!\brief setter function for data member config_id_.
 *@param config_id[in] - configuration mode identifier. *
 **/
void TcMsgCommit::SetData(uint32_t config_id,
                          std::string controller_id,
                          unc_keytype_ctrtype_t driver_id) {
  config_id_ = config_id;
}

/*!\brief method to send transaction abort requests to recipient modules.
 *@param[in] abort_on_fail_ - channel names of recipient modules.
 *@result TcOperRet - TCOPER_RET_SUCCESS/TCOPER_RET_FAILURE/TCOPER_RET_FATAL
 **/
TcOperRet
TcMsgCommit::SendAbortRequest(AbortOnFailVector abort_on_fail_) {
  pfc_log_debug("TcMsgCommit::SendAbortRequest entry");

  AbortOnFailVector::iterator it;
  pfc_ipcconn_t conn = 0;
  pfc_ipcresp_t resp = 0;
  std::string channel_name;
  tclib::TcCommitOpAbortPhase fail_phase = unc::tclib::COMMIT_TRANSACTION_START;
  TcUtilRet util_resp = TCUTIL_RET_SUCCESS;

  /*vector with channel IDs shouldnt be empty*/
  PFC_ASSERT(TCOPER_RET_SUCCESS == channel_names_.empty());
  PFC_ASSERT(TCOPER_RET_SUCCESS == abort_on_fail_.empty());

  /*set fail_phase*/
  if (PFC_EXPECT_TRUE(opertype_ == tclib::MSG_COMMIT_VOTE)) {
    fail_phase = tclib::COMMIT_VOTE_REQUEST;
  } else {
    pfc_log_info("Invalid opertype_:%d", opertype_);
    return TCOPER_RET_FAILURE;
  }

  for (it = abort_on_fail_.begin() ; it != abort_on_fail_.end(); it++) {
    channel_name = GetChannelName(*it);
    PFC_ASSERT(TCOPER_RET_SUCCESS == channel_name.empty());

    pfc_log_info("send ABORT to %s - fail_phase:%d",
                  channel_name.c_str(), fail_phase);
    /*Create session for the given module name and service id*/
    pfc::core::ipc::ClientSession* abortsess =
        TcClientSessionUtils::create_tc_client_session(channel_name,
                              tclib::TCLIB_COMMIT_GLOBAL_ABORT, conn);
    if (NULL == abortsess) {
      pfc_log_error("SendAbortRequest:Error creating abort sess for channel:%s",
                    channel_name.c_str());
      return TCOPER_RET_FATAL;
    }

    /*append data to channel */
    util_resp = TcClientSessionUtils::set_uint8(abortsess,
                                                tclib::MSG_COMMIT_ABORT);
    if (PFC_EXPECT_TRUE(util_resp != TCUTIL_RET_SUCCESS)) {
      TcClientSessionUtils::tc_session_close(&abortsess, conn);
      pfc_log_error("SendAbortRequest: Setting ABORT msg failed");
      return ReturnUtilResp(util_resp);
    }
    /*validate session_id_ and config_id_*/
    if (PFC_EXPECT_TRUE(session_id_ > 0) &&
        PFC_EXPECT_TRUE(config_id_ > 0)) {
      util_resp = TcClientSessionUtils::set_uint32(abortsess, session_id_);
      if (PFC_EXPECT_TRUE(util_resp != TCUTIL_RET_SUCCESS)) {
        TcClientSessionUtils::tc_session_close(&abortsess, conn);
        pfc_log_error("SendAbortRequest: Setting sess_id failed");
        return ReturnUtilResp(util_resp);
      }
      util_resp = TcClientSessionUtils::set_uint32(abortsess, config_id_);
      if (PFC_EXPECT_TRUE(util_resp != TCUTIL_RET_SUCCESS)) {
        TcClientSessionUtils::tc_session_close(&abortsess, conn);
        pfc_log_error("SendAbortRequest: Setting config_id failed");
        return ReturnUtilResp(util_resp);
      }
    } else {
      pfc_log_error("Invalid Session/Config ID");
      return TCOPER_RET_FAILURE;
    }

    util_resp = TcClientSessionUtils::set_uint8(abortsess, fail_phase);
    if (PFC_EXPECT_TRUE(util_resp != TCUTIL_RET_SUCCESS)) {
      TcClientSessionUtils::tc_session_close(&abortsess, conn);
      pfc_log_error("SendAbortRequest: Setting fail_phase failed");
      return ReturnUtilResp(util_resp);
    }
    /*invoke session*/
    util_resp = TcClientSessionUtils::tc_session_invoke(abortsess, resp);
    if (PFC_EXPECT_TRUE(util_resp != TCUTIL_RET_SUCCESS)) {
      TcClientSessionUtils::tc_session_close(&abortsess, conn);
      pfc_log_error("SendAbortRequest: Session invoke failed");
      return ReturnUtilResp(util_resp);
    }

    TcClientSessionUtils::tc_session_close(&abortsess, conn);

    if (PFC_EXPECT_TRUE(tclib::TC_FAILURE == resp)) {
      pfc_log_error("Failure response from %s", channel_name.c_str());
      break;
    }
  }

  pfc_log_debug("TcMsgCommit::SendAbortRequest exit");
  /*return server response */
  return RespondToTc(resp);
}

/*!\brief method to send transaction end notification to recipient modules.
 *@param[in] - abort_on_fail_ - channel names of recipient modules.
 *@result TcOperRet - TCOPER_RET_SUCCESS/TCOPER_RET_FAILURE/TCOPER_RET_FATAL
 **/
TcOperRet
TcMsgCommit::SendTransEndRequest(AbortOnFailVector abort_on_fail_) {
  pfc_log_debug("TcMsgCommit::SendTransEndRequest entry");
  pfc_ipcconn_t conn = 0;
  pfc_ipcresp_t resp = 0;
  std::string channel_name;
  AbortOnFailVector::reverse_iterator rit;
  TcUtilRet util_resp = TCUTIL_RET_SUCCESS;

  /*vector with channel IDs shouldnt be empty*/
  PFC_ASSERT(TCOPER_RET_SUCCESS == channel_names_.empty());
  PFC_ASSERT(TCOPER_RET_SUCCESS == abort_on_fail_.empty());

  for (rit = abort_on_fail_.rbegin() ; rit != abort_on_fail_.rend(); rit++) {
    channel_name = GetChannelName(*rit);
    if (PFC_EXPECT_TRUE(channel_name.empty())) {
      pfc_log_error("SendTransEndRequest:channel name is empty");
      return TCOPER_RET_FAILURE;
    }
    pfc_log_info("notify TxEND to %s", channel_name.c_str());

    /*Create session for the given module name and service id*/
    pfc::core::ipc::ClientSession* end_sess =
        TcClientSessionUtils::create_tc_client_session(channel_name,
                              tclib::TCLIB_COMMIT_TRANSACTION, conn);
    if (NULL == end_sess) {
      pfc_log_error("SendTransEndRequest:Sess creation failed");
      return TCOPER_RET_FATAL;
    }
    /*append data to channel */
    util_resp = TcClientSessionUtils::set_uint8(end_sess,
                                                tclib::MSG_COMMIT_TRANS_END);
    if (PFC_EXPECT_TRUE(util_resp != TCUTIL_RET_SUCCESS)) {
      TcClientSessionUtils::tc_session_close(&end_sess, conn);
      pfc_log_error("SendTransEndRequest: Settng MSG_COMMIT_TRANS_END failed");
      return ReturnUtilResp(util_resp);
    }
    /*validate session_id_ and config_id_*/
    if (PFC_EXPECT_TRUE(session_id_ > 0) && PFC_EXPECT_TRUE(config_id_ > 0)) {
      util_resp = TcClientSessionUtils::set_uint32(end_sess, session_id_);
      if (PFC_EXPECT_TRUE(util_resp != TCUTIL_RET_SUCCESS)) {
        TcClientSessionUtils::tc_session_close(&end_sess, conn);
        pfc_log_error("SendTransEndRequest: Settng sess_id failed");
        return ReturnUtilResp(util_resp);
      }
      util_resp = TcClientSessionUtils::set_uint32(end_sess, config_id_);
      if (PFC_EXPECT_TRUE(util_resp != TCUTIL_RET_SUCCESS)) {
        TcClientSessionUtils::tc_session_close(&end_sess, conn);
        pfc_log_error("SendTransEndRequest: Settng config_id failed");
        return ReturnUtilResp(util_resp);
      }
    } else {
      pfc_log_error("Invalid Session/Config ID");
      TcClientSessionUtils::tc_session_close(&end_sess, conn);
      return TCOPER_RET_FAILURE;
    }
    /*transmit trans end result*/
    util_resp = TcClientSessionUtils::set_uint8(end_sess,
                                                tclib::TRANS_END_FAILURE);
    if (PFC_EXPECT_TRUE(util_resp != TCUTIL_RET_SUCCESS)) {
      TcClientSessionUtils::tc_session_close(&end_sess, conn);
      pfc_log_error("SendTransEndRequest: Settng TX_END_FAILURE failed");
      return ReturnUtilResp(util_resp);
    }
    /*invoke session*/
    util_resp = TcClientSessionUtils::tc_session_invoke(end_sess, resp);
    if (PFC_EXPECT_TRUE(util_resp != TCUTIL_RET_SUCCESS)) {
      TcClientSessionUtils::tc_session_close(&end_sess, conn);
      pfc_log_error("SendTransEndRequest: Session invoke failed");
      return ReturnUtilResp(util_resp);
    }

    TcClientSessionUtils::tc_session_close(&end_sess, conn);

    if (tclib::TC_FAILURE == resp) {
      pfc_log_error("Failure response from %s", channel_name.c_str());
      break;
    }
  }
  pfc_log_debug("TcMsgCommit::SendTransEndRequest exit");
  /*return server response */
  return RespondToTc(resp);
}

/*!\brief Parameterized constructor of AbortCandidateDB.
 *@param[in] sess_id - session identifier.
 *@param[in] oper - operation type
 **/
AbortCandidateDB::AbortCandidateDB(uint32_t sess_id,
                                   tclib::TcMsgOperType oper)
:TcMsgCommit(sess_id, oper) {
  abort_version_ = 0;
}

void AbortCandidateDB::SetData(unc_keytype_datatype_t target_db,
                               TcServiceType fail_oper,
                               uint64_t version) {
  abort_version_ = version;
}

/*!\brief TC service handler invokes this method to send abort candidate request
 * to UPLL and UPPL
 *@result TcOperRet - TCOPER_RET_SUCCESS/TCOPER_RET_FAILURE
 **/
TcOperRet AbortCandidateDB::Execute() {
  pfc_log_debug("AbortCandidateDB::Execute() entry");
  pfc_ipcresp_t resp = 0;
  std::string channel_name;
  TcUtilRet util_resp = TCUTIL_RET_SUCCESS;

  /*channel_names_ map should not be empty */
  PFC_ASSERT(TCOPER_RET_SUCCESS == channel_names_.empty());

  notifyorder_.push_back(TC_UPLL);
  notifyorder_.push_back(TC_UPPL);
  pfc_log_debug("sending CANDIDATE ABORT notification");
  for (NotifyList::iterator list_iter = notifyorder_.begin();
       list_iter != notifyorder_.end(); list_iter++) {
    channel_name = GetChannelName(*list_iter);
    if (PFC_EXPECT_TRUE(channel_name.empty())) {
      pfc_log_error("channel_name is empty");
      return TCOPER_RET_FAILURE;
    }
    pfc_log_info("notify %s - session_id:%d config_id:%d",
                 channel_name.c_str(), session_id_, config_id_);

    /*Create session for the given module name and service id*/
    sess_ = TcClientSessionUtils::create_tc_client_session(channel_name,
                                  tclib::TCLIB_USER_ABORT, conn_);
    if (NULL == sess_) {
      pfc_log_error("AbortCandidateDB::Execute sess creation failed");
      return TCOPER_RET_FATAL;
    }

    /*append data to channel - validate session_id_ and config_id_*/
    if (PFC_EXPECT_TRUE(session_id_ > 0) &&
        PFC_EXPECT_TRUE(config_id_ > 0)) {
      util_resp = TcClientSessionUtils::set_uint32(sess_, session_id_);
      if (PFC_EXPECT_TRUE(util_resp != TCUTIL_RET_SUCCESS)) {
        pfc_log_error("AbortCandidateDB::Execute Set sess_id failed");
        return ReturnUtilResp(util_resp);
      }
      util_resp = TcClientSessionUtils::set_uint32(sess_, config_id_);
      if (PFC_EXPECT_TRUE(util_resp != TCUTIL_RET_SUCCESS)) {
        pfc_log_error("AbortCandidateDB::Execute Set config_id failed");
        return ReturnUtilResp(util_resp);
      }
      util_resp = TcClientSessionUtils::set_uint64(sess_, abort_version_);
      if (PFC_EXPECT_TRUE(util_resp != TCUTIL_RET_SUCCESS)) {
        pfc_log_error("AbortCandidateDB::Execute Set abort_ver failed");
        return ReturnUtilResp(util_resp);
      }
    } else {
      pfc_log_error("Invalid Session/Config ID");
      return TCOPER_RET_FAILURE;
    }
    /*invoke session*/
    util_resp = TcClientSessionUtils::tc_session_invoke(sess_, resp);
    if (PFC_EXPECT_TRUE(util_resp != TCUTIL_RET_SUCCESS)) {
      pfc_log_error("AbortCandidateDB::Execute Session invoke failed");
      return ReturnUtilResp(util_resp);
    }

    TcClientSessionUtils::tc_session_close(&sess_, conn_);

    if (resp == tclib::TC_FAILURE) {
      pfc_log_info("Failure response from %s", channel_name.c_str());
      break;
    } else {
      pfc_log_info("Success response from %s", channel_name.c_str());
    }
  }
  /*return server response*/
  pfc_log_debug("AbortCandidateDB::Execute() exit");
  notifyorder_.clear();
  return RespondToTc(resp);
}

/*!\brief Parameterized constructor of CommitTransaction.
 *@param sess_id[in] - session identifier.
 *@param oper[in] - operation type
 **/
CommitTransaction::CommitTransaction(uint32_t sess_id,
                                     tclib::TcMsgOperType oper)
:TcMsgCommit(sess_id, oper) {}


/*!\brief this method sends Transaction START/END to recipient modules.
 *@param channel_name[in] - channel name of recipient module
 *@result TcOperRet - TCOPER_RET_SUCCESS/TCOPER_RET_FAILURE
 **/
TcOperRet
CommitTransaction::SendRequest(std::string channel_name) {
  pfc_log_debug("CommitTransaction::SendRequest() entry");
  pfc_ipcresp_t resp = 0;
  TcUtilRet util_resp = TCUTIL_RET_SUCCESS;
  TcOperRet ret_val = TCOPER_RET_SUCCESS;

  PFC_ASSERT(TCOPER_RET_SUCCESS == channel_name.empty());

  pfc_log_info("notify %s - session_id:%d config_id:%d",
               channel_name.c_str(), session_id_, config_id_);
  /*Create session for the given module name and service id*/
  sess_ = TcClientSessionUtils::create_tc_client_session(channel_name,
                                tclib::TCLIB_COMMIT_TRANSACTION, conn_);
  if (NULL == sess_) {
    pfc_log_error("CommitTx::SendRequest Create sess failed");
    return TCOPER_RET_FATAL;
  }

  /*append data to channel */
  util_resp = TcClientSessionUtils::set_uint8(sess_, opertype_);
  if (PFC_EXPECT_TRUE(util_resp != TCUTIL_RET_SUCCESS)) {
    pfc_log_error("CommitTx::SendRequest Set opertype failed");
    return ReturnUtilResp(util_resp);
  }
  util_resp = TcClientSessionUtils::set_uint32(sess_, session_id_);
  if (PFC_EXPECT_TRUE(util_resp != TCUTIL_RET_SUCCESS)) {
    pfc_log_error("CommitTx::SendRequest Set sess_id failed");
    return ReturnUtilResp(util_resp);
  }
  util_resp = TcClientSessionUtils::set_uint32(sess_, config_id_);
  if (PFC_EXPECT_TRUE(util_resp != TCUTIL_RET_SUCCESS)) {
    pfc_log_error("CommitTx::SendRequest Set config_id failed");
    return ReturnUtilResp(util_resp);
  }
  /*send end result of transaction operation */
  if (PFC_EXPECT_TRUE(opertype_ == tclib::MSG_COMMIT_TRANS_END)) {
    util_resp = TcClientSessionUtils::set_uint8(sess_, trans_result_);
    if (PFC_EXPECT_TRUE(util_resp != TCUTIL_RET_SUCCESS)) {
      pfc_log_error("CommitTx::SendRequest Set txn_result failed");
      return ReturnUtilResp(util_resp);
    }
  }
  /*invoke session*/
  util_resp = TcClientSessionUtils::tc_session_invoke(sess_, resp);
  if (PFC_EXPECT_TRUE(util_resp != TCUTIL_RET_SUCCESS)) {
    pfc_log_error("CommitTx::SendRequest Session invoke failed");
    return ReturnUtilResp(util_resp);
  }

  /*handle server response*/
  if (PFC_EXPECT_TRUE(resp == tclib::TC_SUCCESS)) {
    pfc_log_info("Success response from %s", channel_name.c_str());
    ret_val = TCOPER_RET_SUCCESS;
  } else if (PFC_EXPECT_TRUE(resp == tclib::TC_FAILURE) &&
             PFC_EXPECT_TRUE(opertype_ == tclib::MSG_COMMIT_TRANS_START)) {
    pfc_log_info("Failure response from %s", channel_name.c_str());
    /*append transaction result to trans-end notification*/
    trans_result_ = tclib::TRANS_END_FAILURE;
    if (PFC_EXPECT_TRUE(abort_on_fail_.empty())) {
      ret_val = TCOPER_RET_ABORT;
    } else {
      /*send transaction end to all daemons that successfully processed
       * transaction start*/
      ret_val = SendTransEndRequest(abort_on_fail_);
      if (PFC_EXPECT_TRUE(TCOPER_RET_SUCCESS == ret_val)) {
        ret_val = TCOPER_RET_ABORT;
      } else {
        pfc_log_error("SendTransactionEnd() failed");
        ret_val = TCOPER_RET_FATAL;
      }
    }
  } else if (PFC_EXPECT_TRUE(resp == tclib::TC_FAILURE) &&
             PFC_EXPECT_TRUE(opertype_ == tclib::MSG_COMMIT_TRANS_END)) {
    pfc_log_error("Transaction end request failed");
    ret_val =  TCOPER_RET_FATAL;
  }
  /*session is not closed in case of failure as
   * its contents are forwarded to VTN*/
  if (PFC_EXPECT_TRUE(ret_val == TCOPER_RET_SUCCESS)) {
    TcClientSessionUtils::tc_session_close(&sess_, conn_);
  }
  pfc_log_debug("CommitTransaction::SendRequest() exit");
  return ret_val;
}

/*!\brief TC service handler invokes this method to send Transaction START/END
 * request to recipient modules.
 *@result TcOperRet - TCOPER_RET_SUCCESS/TCOPER_RET_FAILURE
 **/
TcOperRet CommitTransaction::Execute() {
  pfc_log_debug("CommitTransaction::Execute() entry");
  TcOperRet ret_val = TCOPER_RET_SUCCESS;
  std::string channel_name;

  /*channel_names_ map should not be empty */
  PFC_ASSERT(TCOPER_RET_SUCCESS == channel_names_.empty());

  /*validate session_id_ and config_id_*/
  if (PFC_EXPECT_TRUE(session_id_ == 0) ||
      PFC_EXPECT_TRUE(config_id_ == 0)) {
    pfc_log_error("Invalid Session/Config ID");
    return TCOPER_RET_FAILURE;
  }

  switch (opertype_) {
    case tclib::MSG_COMMIT_TRANS_START: {
      pfc_log_info("*** TxSTART request ***");
      notifyorder_.push_back(TC_DRV_OPENFLOW);
      notifyorder_.push_back(TC_DRV_OVERLAY);
      notifyorder_.push_back(TC_DRV_POLC);
      notifyorder_.push_back(TC_DRV_VAN);
      notifyorder_.push_back(TC_DRV_ODC);
      // notifyorder_.push_back(TC_DRV_LEGACY);
      notifyorder_.push_back(TC_UPLL);
      notifyorder_.push_back(TC_UPPL);
      break;
    }
    case tclib::MSG_COMMIT_TRANS_END: {
      pfc_log_info("*** TxEND request ***");
      notifyorder_.push_back(TC_UPPL);
      notifyorder_.push_back(TC_UPLL);
      // notifyorder_.push_back(TC_DRV_LEGACY);
      notifyorder_.push_back(TC_DRV_ODC);
      notifyorder_.push_back(TC_DRV_VAN);
      notifyorder_.push_back(TC_DRV_POLC);
      notifyorder_.push_back(TC_DRV_OVERLAY);
      notifyorder_.push_back(TC_DRV_OPENFLOW);
      break;
    }
    default: {
      pfc_log_error("Invalid operation type: %d", opertype_);
      return TCOPER_RET_FAILURE;
    }
  }

  for (NotifyList::iterator list_iter = notifyorder_.begin();
       list_iter != notifyorder_.end(); list_iter++) {
    channel_name = GetChannelName(*list_iter);
    if (PFC_EXPECT_TRUE(channel_name.empty())) {
      /*channel names of drivers may be empty - ignore*/
      continue;
    }

    ret_val = SendRequest(channel_name);
    if (PFC_EXPECT_TRUE(ret_val != TCOPER_RET_SUCCESS)) {
      return ret_val;
    } else if (PFC_EXPECT_TRUE(ret_val == TCOPER_RET_SUCCESS)) {
      /*append channel_id in a vector to handle failure scenario*/
      abort_on_fail_.push_back(*list_iter);
    }
  }
  /*clear map*/
  abort_on_fail_.clear();
  notifyorder_.clear();
  pfc_log_debug("CommitTransaction::Execute() exit");
  return ret_val;
}

/*!\brief Parameterized constructor of TwoPhaseCommit.
 *@param[in] sess_id - session identifier.
 *@param[in] oper - operation type
 **/
TwoPhaseCommit:: TwoPhaseCommit(uint32_t sess_id,
                                tclib::TcMsgOperType oper)
:TcMsgCommit(sess_id, oper) {}


/*!brief method to set the upll/uppl sessions with default attributes to send
 * driver result
 * @param[in] tmpsess - UPLL/UPPL session pointer
 * @result - TCOPER_RET_SUCCESS/TCOPER_RET_FAILURE
 * */
TcOperRet
TwoPhaseCommit::SetSessionToForwardDriverResult(pfc::core::ipc::ClientSession*
                                                tmpsess) {
  pfc_log_debug("TwoPhaseCommit::SetSessionToForwardDriverResult entry");
  tclib::TcCommitPhaseType phase;
  tclib::TcMsgOperType oper;
  TcUtilRet util_resp = TCUTIL_RET_SUCCESS;

  switch (opertype_) {
    case tclib::MSG_COMMIT_VOTE: {
      oper = tclib::MSG_COMMIT_VOTE_DRIVER_RESULT;
      phase = tclib::TC_COMMIT_VOTE_PHASE;
      break;
    }
    case tclib::MSG_COMMIT_GLOBAL: {
      oper = tclib::MSG_COMMIT_GLOBAL_DRIVER_RESULT;
      phase = tclib::TC_COMMIT_GLOBAL_COMMIT_PHASE;
      break;
    }
    default: {
      pfc_log_error("Invalid opertype:%d", opertype_);
      return TCOPER_RET_FAILURE;
    }
  }

  /*append data to channel */
  util_resp = TcClientSessionUtils::set_uint8(tmpsess, oper);
  if (PFC_EXPECT_TRUE(util_resp != TCUTIL_RET_SUCCESS)) {
    pfc_log_error("SetSessionToForwardDriverResult: Set oper failed");
    return ReturnUtilResp(util_resp);
  }
  /*validate session_id_ and config_id_*/
  if (PFC_EXPECT_TRUE(session_id_ > 0) &&
      PFC_EXPECT_TRUE(config_id_ > 0)) {
    util_resp = TcClientSessionUtils::set_uint32(tmpsess, session_id_);
    if (PFC_EXPECT_TRUE(util_resp != TCUTIL_RET_SUCCESS)) {
      pfc_log_error("SetSessionToForwardDriverResult: Set sess_id failed");
      return ReturnUtilResp(util_resp);
    }
    util_resp = TcClientSessionUtils::set_uint32(tmpsess, config_id_);
    if (PFC_EXPECT_TRUE(util_resp != TCUTIL_RET_SUCCESS)) {
      pfc_log_error("SetSessionToForwardDriverResult: Set config_id failed");
      return ReturnUtilResp(util_resp);
    }
  } else {
    pfc_log_error("Invalid Session/Config ID");
    return TCOPER_RET_FAILURE;
  }
  util_resp = TcClientSessionUtils::set_uint8(tmpsess, phase);
  if (PFC_EXPECT_TRUE(util_resp != TCUTIL_RET_SUCCESS)) {
    pfc_log_error("SetSessionToForwardDriverResult: Set phase failed");
    return ReturnUtilResp(util_resp);
  }
  pfc_log_debug("TwoPhaseCommit::SetSessionToForwardDriverResult exit");
  return TCOPER_RET_SUCCESS;
}

/*! brief method to create  UPLL/UPPL sessions to forward driver result
 * @result - TCOPER_RET_SUCCESS/FAILURE*/
TcOperRet TwoPhaseCommit::CreateSessionsToForwardDriverResult() {
  pfc_log_debug("TwoPhaseCommit::CreateSessionsToForwardDriverResult() entry");
  TcOperRet ret_val = TCOPER_RET_SUCCESS;

  std::string channel_name = GetChannelName(TC_UPLL);
  PFC_ASSERT(TCOPER_RET_SUCCESS == channel_name.empty());
  upll_sess_ = TcClientSessionUtils::create_tc_client_session(channel_name,
                                     tclib::TCLIB_COMMIT_DRV_RESULT,
                                     upll_conn_);
  if (NULL == upll_sess_) {
    return TCOPER_RET_FATAL;
  }

  ret_val = SetSessionToForwardDriverResult(upll_sess_);
  if (PFC_EXPECT_TRUE(ret_val != TCOPER_RET_SUCCESS)) {
    pfc_log_error("SetSessionToForwardDriverResult failed");
    return TCOPER_RET_FAILURE;
  }
  channel_name.clear();

  channel_name = GetChannelName(TC_UPPL);
  PFC_ASSERT(TCOPER_RET_SUCCESS == channel_name.empty());
  uppl_sess_ = TcClientSessionUtils::create_tc_client_session(channel_name,
                                     tclib::TCLIB_COMMIT_DRV_RESULT,
                                     uppl_conn_);
  if (NULL == uppl_sess_) {
    pfc_log_error("CreateSessionsToForwardDriverResult sess creation failed");
    return TCOPER_RET_FATAL;
  }

  ret_val = SetSessionToForwardDriverResult(uppl_sess_);
  if (PFC_EXPECT_TRUE(ret_val != TCOPER_RET_SUCCESS)) {
    pfc_log_error("SetSessionToForwardDriverResult failed");
    return TCOPER_RET_FAILURE;
  }
  pfc_log_debug("TwoPhaseCommit::CreateSessionsToForwardDriverResult() exit");
  return TCOPER_RET_SUCCESS;
}


/*! brief method to invoke UPLL/UPPL sessions to forward driver result
 * @param[in] tmpsess - UPLL/UPPL session pointer
 * @result - TCOPER_RET_SUCCESS/FAILURE*/

TcOperRet
TwoPhaseCommit::HandleDriverResultResponse(pfc::core::ipc::ClientSession*
                                           tmpsess) {
  pfc_log_debug("TwoPhaseCommit::HandleDriverResultResponse entry");
  TcUtilRet util_resp = TCUTIL_RET_SUCCESS;
  TcOperRet ret_val = TCOPER_RET_SUCCESS;
  pfc_ipcresp_t resp = 0;

  util_resp = TcClientSessionUtils::tc_session_invoke(tmpsess, resp);
  if (PFC_EXPECT_TRUE(util_resp != TCUTIL_RET_SUCCESS)) {
    return ReturnUtilResp(util_resp);
  }
  if (PFC_EXPECT_TRUE(resp == tclib::TC_FAILURE) &&
      PFC_EXPECT_TRUE(opertype_ ==  tclib::MSG_COMMIT_VOTE)) {
    pfc_log_info("failure response for vote-driver result");
    trans_result_ = tclib::TRANS_END_FAILURE;
    ret_val = SendAbortRequest(abort_on_fail_);
    if (PFC_EXPECT_TRUE(ret_val == TCOPER_RET_SUCCESS)) {
      ret_val = TCOPER_RET_ABORT;
    } else {
      pfc_log_error("SendAbortRequest failed");
      ret_val = TCOPER_RET_FATAL;
    }
  } else if (PFC_EXPECT_TRUE(resp == tclib::TC_FAILURE) &&
             PFC_EXPECT_TRUE(opertype_ == tclib::MSG_COMMIT_GLOBAL)) {
    pfc_log_info("failure response for global commit-driver result");
    ret_val = TCOPER_RET_FATAL;
  } else {
    pfc_log_info("success response for driver-result");
  }

  pfc_log_debug("TwoPhaseCommit::HandleDriverResultResponse exit");
  return ret_val;
}


/*!\brief method to accumulate the controller info from UPLL/UPPL in
 * TcControllerInfoMap
 * @param[in] sess_ - client session parameter
 *@result TcOperRet - TCOPER_RET_SUCCESS/TCOPER_RET_FAILURE
 **/
TcOperRet
TwoPhaseCommit::GetControllerInfo(pfc::core::ipc::ClientSession* sess_) {
  pfc_log_debug("GetControllerInfo entry");

  uint8_t driver_count = 0, controller_count = 0, driver_id = 0;
  unc_keytype_ctrtype_t driver_type;
  uint32_t idx = 0;
  ControllerList controllers, temp_list;
  std::string controller_id;
  TcUtilRet util_resp = TCUTIL_RET_SUCCESS;

  util_resp = TcClientSessionUtils::get_uint8(sess_, idx, &driver_count);
  if (PFC_EXPECT_TRUE(util_resp != TCUTIL_RET_SUCCESS)) {
    return ReturnUtilResp(util_resp);
  }
  pfc_log_debug("driver_count:%d", driver_count);
  for (int i = 1; i <= driver_count; i++) {
    idx++;
    util_resp = TcClientSessionUtils::get_uint8(sess_, idx, &driver_id);
    if (PFC_EXPECT_TRUE(util_resp != TCUTIL_RET_SUCCESS)) {
      pfc_log_error("GetControllerInfo getting driver_id failed");
      return ReturnUtilResp(util_resp);
    }
    driver_type = (unc_keytype_ctrtype_t)driver_id;
    /*save the controller ids returned by UPLL/UPPL for validating driver vote
     * request*/
    driver_set_.insert(driver_type);

    idx++;
    util_resp = TcClientSessionUtils::get_uint8(sess_, idx, &controller_count);
    if (PFC_EXPECT_TRUE(util_resp != TCUTIL_RET_SUCCESS)) {
      pfc_log_error("GetControllerInfo getting ctrl_count failed");
      return ReturnUtilResp(util_resp);
    }
    pfc_log_info("driver_id:%d controller_count:%d", driver_id,
                 controller_count);
    for (int j = 1; j <= controller_count; j++) {
      idx++;
      util_resp = TcClientSessionUtils::get_string(sess_, idx, controller_id);
      if (PFC_EXPECT_TRUE(util_resp != TCUTIL_RET_SUCCESS)) {
        pfc_log_error("GetControllerInfo getting ctrl_id failed");
        return ReturnUtilResp(util_resp);
      }
      controllers.push_back(controller_id);
    }

    /*driverinfo map is empty or doesnt contain info on the driver type*/
    if (PFC_EXPECT_TRUE(driverinfo_map_.empty()) ||
        PFC_EXPECT_TRUE(driverinfo_map_.find(driver_type) ==
                        driverinfo_map_.end())) {
      pfc_log_debug("Insert driver_id:%d to driverinfo_map_", driver_type);
      driverinfo_map_.insert(std::pair<unc_keytype_ctrtype_t,
                             ControllerList>(driver_type, controllers));

    } else {
      /*appending info to existing drivertype*/
      pfc_log_debug("overwrite driver_id:%d to driverinfo_map_", driver_type);
      TcDriverInfoMap::iterator it = driverinfo_map_.find(driver_type);
      if (PFC_EXPECT_TRUE(it != driverinfo_map_.end())) {
        temp_list = it->second;
        /*prevent duplicate entries*/
        temp_list.sort();
        controllers.sort();
        controllers.merge(temp_list);
        controllers.unique();
        driverinfo_map_.erase(driver_type);
        driverinfo_map_.insert(std::pair<unc_keytype_ctrtype_t,
                               ControllerList>(driver_type, controllers));
      }
    }
    controllers.clear();
  }
  pfc_log_debug("GetControllerInfo exit");
  return TCOPER_RET_SUCCESS;
}

/*!\brief method to send vote/global commit to driver modules with controller
 * info collected from UPLL/UPPL
 * @param[in] channelname - recipient channel name
 * @param[in] clist - list of controllers corresponding to the driver module.
 * @param[in] dummy_sess_ - dummy client session to collect driver response.
 *@result TcOperRet - TCOPER_RET_SUCCESS/TCOPER_RET_FAILURE
 **/
TcOperRet
TwoPhaseCommit::SendRequestToDriver() {
  pfc_log_debug("TwoPhaseCommit::SendRequestToDriver entry");
  pfc_ipcresp_t resp = 0;
  std::string channel_name;
  TcUtilRet util_resp = TCUTIL_RET_SUCCESS;
  TcOperRet ret_val = TCOPER_RET_SUCCESS;
  tclib::TcMsgOperType oper = tclib::MSG_NONE;
  TcDriverInfoMap::iterator it;
  ControllerList clist;
  ControllerList::iterator cntrl_it;
  TcDaemonName tc_driverid;

  if (PFC_EXPECT_TRUE(opertype_ ==  tclib::MSG_COMMIT_VOTE)) {
    pfc_log_info("*** VOTE to Driver ***");
    oper = tclib::MSG_COMMIT_DRIVER_VOTE;
  } else if (PFC_EXPECT_TRUE(opertype_ == tclib::MSG_COMMIT_GLOBAL)) {
    pfc_log_info("*** GLOBAL COMMIT to Driver ***");
    oper = tclib::MSG_COMMIT_DRIVER_GLOBAL;
  }

  /*create UPLL/UPPL clientsessions to forward driver result*/
  ret_val = CreateSessionsToForwardDriverResult();
  if (PFC_EXPECT_TRUE(ret_val != TCOPER_RET_SUCCESS)) {
    pfc_log_error("creating UPLL/UPPL sessions to forward DriverResult failed");
    return TCOPER_RET_FAILURE;
  }
  /*send controller info to driver modules*/
  for (it = driverinfo_map_.begin(); it != driverinfo_map_.end(); it++) {
    /*map the driver type and get the channel name*/
    tc_driverid = MapTcDriverId((*it).first);
    if (tc_driverid != TC_NONE) {
      TcChannelNameMap::iterator chname = channel_names_.find(tc_driverid);
      if (chname != channel_names_.end()) {
        channel_name = chname->second;
        pfc_log_debug("tc_driverid:%d channel_name:%s",
                      tc_driverid, channel_name.c_str());
      } else {
        pfc_log_error("Channel not available for driver:%u", tc_driverid);
        channel_name.clear();
      }
    } else {
      pfc_log_error("Driver daemon %d does not exist", (*it).first);
      return TCOPER_RET_FATAL;
    }
    if (channel_name.empty()) {
      pfc_log_error("Driver not present; Adding dummy response");
      ret_val = HandleDriverNotPresent(it->first);
      if (PFC_EXPECT_TRUE(ret_val != TCOPER_RET_SUCCESS)) {
        pfc_log_error("HandleDriverNotPresent not successful");
        return TCOPER_RET_FATAL;
      }
      continue;
    }

    /*Create session for the given module name and service id*/
    sess_ = TcClientSessionUtils::create_tc_client_session(channel_name,
                                  tclib::TCLIB_COMMIT_DRV_VOTE_GLOBAL, conn_);
    if (NULL == sess_) {
      pfc_log_error("SendRequestToDriver: Session creation failed");
      return TCOPER_RET_FATAL;
    }

    /*append data to channel */
    util_resp = TcClientSessionUtils::set_uint8(sess_, oper);
    if (PFC_EXPECT_TRUE(util_resp != TCUTIL_RET_SUCCESS)) {
      pfc_log_error("SendRequestToDriver: Set oper failed");
      return ReturnUtilResp(util_resp);
    }
    /*validate session_id_ and config_id_*/
    if (PFC_EXPECT_TRUE(session_id_ > 0) &&
        PFC_EXPECT_TRUE(config_id_ > 0)) {
      util_resp = TcClientSessionUtils::set_uint32(sess_, session_id_);
      if (PFC_EXPECT_TRUE(util_resp != TCUTIL_RET_SUCCESS)) {
        pfc_log_error("SendRequestToDriver: Set sess_id failed");
        return ReturnUtilResp(util_resp);
      }
      util_resp = TcClientSessionUtils::set_uint32(sess_, config_id_);
      if (PFC_EXPECT_TRUE(util_resp != TCUTIL_RET_SUCCESS)) {
        pfc_log_error("SendRequestToDriver: Set config_id failed");
        return ReturnUtilResp(util_resp);
      }
    } else {
      pfc_log_error("Invalid Session/Config ID");
      return TCOPER_RET_FAILURE;
    }
    /*add controller info*/
    clist = (*it).second;
    uint8_t controller_count = clist.size();
    util_resp = TcClientSessionUtils::set_uint8(sess_, controller_count);
    if (PFC_EXPECT_TRUE(util_resp != TCUTIL_RET_SUCCESS)) {
      pfc_log_error("SendRequestToDriver: Set ctrl_count failed");
      return ReturnUtilResp(util_resp);
    }
    for (cntrl_it = clist.begin(); cntrl_it != clist.end(); cntrl_it++) {
      util_resp = TcClientSessionUtils::set_string(sess_, *cntrl_it);
      if (PFC_EXPECT_TRUE(util_resp != TCUTIL_RET_SUCCESS)) {
        pfc_log_error("SendRequestToDriver: Set ctrl_id failed");
        return ReturnUtilResp(util_resp);
      }
    }
    pfc_log_info("notify %s - controller_count:%d",
                 channel_name.c_str(), controller_count);
    /*Invoke the session */
    util_resp = TcClientSessionUtils::tc_session_invoke(sess_, resp);
    if (util_resp != TCUTIL_RET_SUCCESS) {
      pfc_log_error("SendRequestToDriver: Session invoke failed");
      return ReturnUtilResp(util_resp);
    }

    if (PFC_EXPECT_TRUE(tclib::TC_SUCCESS == resp)) {
      pfc_log_info("success response from %s", channel_name.c_str());
      /*accumulate respective driver response in client sessions*/
      DriverSet dmndrvinfo = driverset_map_[TC_UPLL];
      /*validate the driver_id saved from UPLL controllerinfo*/
      if (PFC_EXPECT_TRUE(dmndrvinfo.find((*it).first) != dmndrvinfo.end())) {
        pfc_log_debug("forward response to UPLL session");

        int32_t ipc_ret = upll_sess_->forward(*sess_, 0, UINT32_MAX);
        if (ipc_ret == ESHUTDOWN ||
            ipc_ret == ECANCELED ||
            ipc_ret == ECONNABORTED) {
          pfc_log_error("%s forward upll_sess_ failed; ipc_ret=%d",
                        __FUNCTION__, ipc_ret);
          return TCOPER_RET_FATAL;
        } else if (ipc_ret != TCOPER_RET_SUCCESS) {
          pfc_log_fatal("%s forward upll_sess_ failed; ipc_ret=%d",
                        __FUNCTION__, ipc_ret);
          return TCOPER_RET_FATAL;
        }
      }
      dmndrvinfo.clear();
      /*Controller list of UPLL is considered since
       *controller list from UPPL is always empty during VOTE/COMMIT phase*/
      dmndrvinfo = driverset_map_[TC_UPLL];
      /*validate the driver_id saved from UPPL controllerinfo*/
      if (PFC_EXPECT_TRUE(dmndrvinfo.find((*it).first) != dmndrvinfo.end())) {
        pfc_log_debug("forward response to UPPL session");

        int32_t ipc_ret = uppl_sess_->forward(*sess_, 0, UINT32_MAX);
        if (ipc_ret == ESHUTDOWN ||
            ipc_ret == ECANCELED ||
            ipc_ret == ECONNABORTED) {
          pfc_log_error("%s forward uppl_sess_ failed; ipc_ret=%d",
                        __FUNCTION__, ipc_ret);
          return TCOPER_RET_FATAL;
        } else if (ipc_ret != TCOPER_RET_SUCCESS) {
          pfc_log_fatal("%s forward uppl_sess_ failed; ipc_ret=%d",
                        __FUNCTION__, ipc_ret);
          return TCOPER_RET_FATAL;
        }
      }
      /*append channelname to handle failure*/
      abort_on_fail_.push_back(tc_driverid);
      ret_val = TCOPER_RET_SUCCESS;
    } else if (PFC_EXPECT_TRUE(tclib::TC_FAILURE == resp) &&
               PFC_EXPECT_TRUE(opertype_ ==  tclib::MSG_COMMIT_VOTE)) {
      pfc_log_info("Failure response from %s", channel_name.c_str());
      trans_result_ = tclib::TRANS_END_FAILURE;

      if (PFC_EXPECT_TRUE(abort_on_fail_.empty())) {
        ret_val = TCOPER_RET_ABORT;
      }
      if (PFC_EXPECT_TRUE(TCOPER_RET_SUCCESS ==
                          SendAbortRequest(abort_on_fail_))) {
        abort_on_fail_.clear();
        driverinfo_map_.clear();
        ret_val = TCOPER_RET_ABORT;
      } else {
        pfc_log_error("SendAbortRequest failed");
        abort_on_fail_.clear();
        ret_val = TCOPER_RET_FATAL;
      }
    } else if (PFC_EXPECT_TRUE(tclib::TC_FAILURE == resp) &&
               PFC_EXPECT_TRUE(opertype_ ==  tclib::MSG_COMMIT_GLOBAL)) {
      pfc_log_error("Failure response from %s", channel_name.c_str());
      ret_val = TCOPER_RET_FATAL;
    }

    if (ret_val == TCOPER_RET_SUCCESS) {
      TcClientSessionUtils::tc_session_close(&sess_, conn_);
    } else {
      return ret_val;
    }
  }

  /*forward driver result to UPLL and UPPL*/
  pfc_log_info("Forwarding DRIVER RESULT to UPLL");
  ret_val = HandleDriverResultResponse(upll_sess_);
  if (ret_val != TCOPER_RET_SUCCESS) {
    return ret_val;
  }
  pfc_log_info("Forwarding DRIVER RESULT to UPPL");
  ret_val = HandleDriverResultResponse(uppl_sess_);
  if (ret_val != TCOPER_RET_SUCCESS) {
    return ret_val;
  }

  pfc_log_debug("TwoPhaseCommit::SendRequestToDriver exit");
  return ret_val;
}

/*!\brief In case of driver-not-present, send dummy response
 *        to UPPL and UPLL for the specific driver.
 *        For all controllers of driver, set the following:
 *        Set controller_id
 *        Set resp code: UNC_RC_ERR_DRIVER_NOT_PRESENT
 *        Set num_of_errors = 0
 *        Set commit_number = 0
 *        Set commit_date = 0
 *        Set commit_application = ""
 **/
TcOperRet
TwoPhaseCommit::HandleDriverNotPresent(unc_keytype_ctrtype_t driver) {
  pfc_log_debug("TwoPhaseCommit::HandleDriverNotPresent entry");

  TcOperRet ret_val = TCOPER_RET_SUCCESS;
  TcUtilRet util_resp = TCUTIL_RET_SUCCESS;

  TcDriverInfoMap::iterator it = driverinfo_map_.find(driver);

  if (it != driverinfo_map_.end()) {
    ControllerList ctrl_list = it->second;
    ControllerList::iterator ctr;
    // Loop through controller list
    for (ctr = ctrl_list.begin(); ctr != ctrl_list.end(); ctr++) {
      pfc_log_info("Creating dummy resp for ctrl(%s) of driver:%d",
                   (*ctr).c_str(), driver);

      // Set the controller_id
      util_resp = TcClientSessionUtils::set_string(upll_sess_, *ctr);
      if (PFC_EXPECT_TRUE(util_resp != TCUTIL_RET_SUCCESS)) {
        pfc_log_error("Cannot set controller_id to upll_sess_");
        return ReturnUtilResp(util_resp);
      }
      util_resp = TcClientSessionUtils::set_string(uppl_sess_, *ctr);
      if (PFC_EXPECT_TRUE(util_resp != TCUTIL_RET_SUCCESS)) {
        pfc_log_error("Cannot set controller_id to uppl_sess_");
        return ReturnUtilResp(util_resp);
      }

      // Set resp_code
      uint32_t resp_code = TC_OPER_DRIVER_NOT_PRESENT;
      util_resp = TcClientSessionUtils::set_uint32(upll_sess_, resp_code);
      if (PFC_EXPECT_TRUE(util_resp != TCUTIL_RET_SUCCESS)) {
        pfc_log_error("Cannot set resp_code to upll_sess_");
        return ReturnUtilResp(util_resp);
      }
      util_resp = TcClientSessionUtils::set_uint32(uppl_sess_, resp_code);
      if (PFC_EXPECT_TRUE(util_resp != TCUTIL_RET_SUCCESS)) {
        pfc_log_error("Cannot set resp_code to uppl_sess_");
        return ReturnUtilResp(util_resp);
      }

      // Set No. of errors
      uint32_t no_of_err = 0;
      util_resp = TcClientSessionUtils::set_uint32(upll_sess_, no_of_err);
      if (PFC_EXPECT_TRUE(util_resp != TCUTIL_RET_SUCCESS)) {
        pfc_log_error("Cannot set num_of_err to upll_sess_");
        return ReturnUtilResp(util_resp);
      }
      util_resp = TcClientSessionUtils::set_uint32(uppl_sess_, no_of_err);
      if (PFC_EXPECT_TRUE(util_resp != TCUTIL_RET_SUCCESS)) {
        pfc_log_error("Cannot set num_of_err to uppl_sess_");
        return ReturnUtilResp(util_resp);
      }
      // Set commit number
      uint64_t commit_number = 0;
      util_resp = TcClientSessionUtils::set_uint64(upll_sess_, commit_number);
      if (PFC_EXPECT_TRUE(util_resp != TCUTIL_RET_SUCCESS)) {
        pfc_log_error("Cannot set commit_number to upll_sess_");
        return ReturnUtilResp(util_resp);
      }
      util_resp = TcClientSessionUtils::set_uint64(uppl_sess_, commit_number);
      if (PFC_EXPECT_TRUE(util_resp != TCUTIL_RET_SUCCESS)) {
        pfc_log_error("Cannot set commit_number to uppl_sess_");
        return ReturnUtilResp(util_resp);
      }
      // Set commit date
      uint64_t commit_date = 0;
      util_resp = TcClientSessionUtils::set_uint64(upll_sess_, commit_date);
      if (PFC_EXPECT_TRUE(util_resp != TCUTIL_RET_SUCCESS)) {
        pfc_log_error("Cannot set commit_date to upll_sess_");
        return ReturnUtilResp(util_resp);
      }
      util_resp = TcClientSessionUtils::set_uint64(uppl_sess_, commit_date);
      if (PFC_EXPECT_TRUE(util_resp != TCUTIL_RET_SUCCESS)) {
        pfc_log_error("Cannot set commit_date to uppl_sess_");
        return ReturnUtilResp(util_resp);
      }
      // Set commit applicateion
      std::string commit_application = "";
      util_resp = TcClientSessionUtils::set_string(upll_sess_,
                                                   commit_application);
      if (PFC_EXPECT_TRUE(util_resp != TCUTIL_RET_SUCCESS)) {
        pfc_log_error("Cannot set commit_application to upll_sess_");
        return ReturnUtilResp(util_resp);
      }
      util_resp = TcClientSessionUtils::set_string(uppl_sess_,
                                                   commit_application);
      if (PFC_EXPECT_TRUE(util_resp != TCUTIL_RET_SUCCESS)) {
        pfc_log_error("Cannot set commit_application to uppl_sess_");
        return ReturnUtilResp(util_resp);
      }
    }
  } else {
    pfc_log_error("Cannot find driver(%d) in driverinfo_map_", driver);
    ret_val = TCOPER_RET_FAILURE;
  }

  pfc_log_debug("TwoPhaseCommit::HandleDriverNotPresent exit");
  return ret_val;
}

/*!\brief this method sends VOTE/GLOBAL COMMIT
 * request of commit operation to UPLL/UPPL/DRIVER modules.
 *@result TcOperRet - TCOPER_RET_SUCCESS/TCOPER_RET_FAILURE/TCOPER_RET_ABORT
 **/
TcOperRet
TwoPhaseCommit::SendRequest(std::string channel_name) {
  pfc_log_debug("TwoPhaseCommit::SendRequest entry");
  pfc_ipcresp_t resp = 0;
  uint32_t respcount = 0;
  TcUtilRet util_resp = TCUTIL_RET_SUCCESS;
  TcOperRet ret_val = TCOPER_RET_SUCCESS;

  PFC_ASSERT(TCOPER_RET_SUCCESS == channel_name.empty());

  /*Create session for the given module name and service id*/
  sess_ = TcClientSessionUtils::create_tc_client_session(channel_name,
                                tclib::TCLIB_COMMIT_TRANSACTION, conn_);
  if (NULL == sess_) {
    pfc_log_error("SendRequest: Session creation failed");
    return TCOPER_RET_FATAL;
  }

  /*append data to channel */
  util_resp = TcClientSessionUtils::set_uint8(sess_, opertype_);
  if (PFC_EXPECT_TRUE(util_resp != TCUTIL_RET_SUCCESS)) {
    pfc_log_error("SendRequest: Set oper failed");
    return ReturnUtilResp(util_resp);
  }
  util_resp = TcClientSessionUtils::set_uint32(sess_, session_id_);
  if (PFC_EXPECT_TRUE(util_resp != TCUTIL_RET_SUCCESS)) {
    pfc_log_error("SendRequest: Set sess_id failed");
    return ReturnUtilResp(util_resp);
  }
  util_resp = TcClientSessionUtils::set_uint32(sess_, config_id_);
  if (PFC_EXPECT_TRUE(util_resp != TCUTIL_RET_SUCCESS)) {
    pfc_log_error("SendRequest: Set config_id failed");
    return ReturnUtilResp(util_resp);
  }
  pfc_log_info("notify %s - session_id:%d config_id:%d",
               channel_name.c_str(), session_id_, config_id_);
  /*invoke session*/
  util_resp = TcClientSessionUtils::tc_session_invoke(sess_, resp);
  if (PFC_EXPECT_TRUE(util_resp != TCUTIL_RET_SUCCESS)) {
    pfc_log_error("SendRequest: Session invoke failed");
    return ReturnUtilResp(util_resp);
  }

  respcount = sess_->getResponseCount();

  /*handle server response*/
  if (PFC_EXPECT_TRUE(resp == tclib::TC_SUCCESS) &&
      PFC_EXPECT_TRUE(respcount > 0)) {
    /*append the driver info from log/phy in
     * TcControllerInfoMap driverinfo_map_*/
    if (TCOPER_RET_SUCCESS != GetControllerInfo(sess_)) {
      pfc_log_error("could not retrieve controller-info");
      return TCOPER_RET_FAILURE;
    }
    ret_val = TCOPER_RET_SUCCESS;
  } else if (PFC_EXPECT_TRUE(resp == tclib::TC_FAILURE) &&
             PFC_EXPECT_TRUE(opertype_ == tclib::MSG_COMMIT_VOTE)) {
    pfc_log_info("Failure response from %s",
                 channel_name.c_str() );
    /*transamit transaction end result in trans-end notification*/
    trans_result_ = tclib::TRANS_END_FAILURE;
    if (PFC_EXPECT_TRUE(abort_on_fail_.empty())) {
      ret_val = TCOPER_RET_ABORT;
    } else {
      ret_val = SendAbortRequest(abort_on_fail_);
      if (PFC_EXPECT_TRUE(TCOPER_RET_SUCCESS == ret_val)) {
        abort_on_fail_.clear();
        ret_val = TCOPER_RET_ABORT;
      } else {
        pfc_log_error("SendAbortRequest failed");
        abort_on_fail_.clear();
        ret_val = TCOPER_RET_FATAL;
      }
    }
  } else if (PFC_EXPECT_TRUE(resp == tclib::TC_FAILURE) &&
             PFC_EXPECT_TRUE(opertype_ == tclib::MSG_COMMIT_GLOBAL)) {
    pfc_log_error("Failure response from %s",
                  channel_name.c_str());
    ret_val = TCOPER_RET_FATAL;
  }

  if (ret_val == TCOPER_RET_SUCCESS)  {
    pfc_log_info("Success response from %s", channel_name.c_str());
    TcClientSessionUtils::tc_session_close(&sess_, conn_);
  }
  pfc_log_debug("TwoPhaseCommit::SendRequest() exit");
  return ret_val;
}

/*!\brief TC service handler invokes this method to send VOTE/GLOBAL COMMIT
 * request of commit operation to UPLL/UPPL/DRIVER modules.
 *@result TcOperRet - TCOPER_RET_SUCCESS/TCOPER_RET_FAILURE/TCOPER_RET_ABORT
 **/
TcOperRet TwoPhaseCommit::Execute() {
  pfc_log_debug("TwoPhaseCommit::Execute()  entry");

  TcOperRet ret_val = TCOPER_RET_SUCCESS;
  std::string channelname, channel_name;

  /*channel_names_ map should not be empty */
  PFC_ASSERT(TCOPER_RET_SUCCESS == channel_names_.empty());

  /*validate session_id_ and config_id_*/
  if (PFC_EXPECT_TRUE(session_id_ == 0) ||
      PFC_EXPECT_TRUE(config_id_ == 0)) {
    pfc_log_error("Invalid Session/Config ID");
    return TCOPER_RET_FAILURE;
  }
  if (opertype_ == tclib::MSG_COMMIT_VOTE) {
    pfc_log_info("*** VOTE to UPLL/UPPL ***");
  } else {
    pfc_log_info("*** GLOBAL COMMIT to UPLL/UPPL ***");
  }

  /* Send request to UPLL/UPPL and gather the controller info*/
  notifyorder_.push_back(TC_UPLL);
  notifyorder_.push_back(TC_UPPL);

  for (NotifyList::iterator list_iter = notifyorder_.begin();
       list_iter != notifyorder_.end(); list_iter++) {
    channel_name = GetChannelName(*list_iter);
    if (PFC_EXPECT_TRUE(channel_name.empty())) {
      pfc_log_error("channel_name is empty");
      return TCOPER_RET_FAILURE;
    }
    /*send vote/global commit request to UPLL/UPPL*/
    ret_val = SendRequest(channel_name);
    if (PFC_EXPECT_TRUE(ret_val != TCOPER_RET_SUCCESS)) {
      abort_on_fail_.clear();
      return ret_val;
    } else if (PFC_EXPECT_TRUE(ret_val == TCOPER_RET_SUCCESS)) {
      /*append daemon name to handle failure scenarios*/
      abort_on_fail_.push_back(*list_iter);
      /*save UPPL/UPPL driver list to send respective driver response*/
      if (PFC_EXPECT_TRUE(TCOPER_RET_SUCCESS == driver_set_.empty())) {
        driverset_map_.insert(std::pair<TcDaemonName,
                              DriverSet>(*list_iter, driver_set_));
        driver_set_.clear();
      }
    }
  }
  /*commit configurations that doesn't relate to any controllers - ignore*/
  if (PFC_EXPECT_TRUE(ret_val == TCOPER_RET_SUCCESS) &&
      PFC_EXPECT_TRUE(driverinfo_map_.empty())) {
    pfc_log_info("Controller info from UPPL,UPLL is empty. "
                 "Sending dummy driver result to UPLL/UPPL");
    ret_val = CreateSessionsToForwardDriverResult();
    if (PFC_EXPECT_TRUE(ret_val != TCOPER_RET_SUCCESS)) {
      pfc_log_error("CreateSessionsToForwardDriverResult failed");
      return TCOPER_RET_FAILURE;
    }
    ret_val = HandleDriverResultResponse(upll_sess_);
    if (PFC_EXPECT_TRUE(ret_val != TCOPER_RET_SUCCESS)) {
      pfc_log_error("UPLL response for driver result failed");
      return TCOPER_RET_FAILURE;
    }
    ret_val = HandleDriverResultResponse(uppl_sess_);
    if (PFC_EXPECT_TRUE(ret_val != TCOPER_RET_SUCCESS)) {
      pfc_log_error("UPPL response for driver result failed");
      return TCOPER_RET_FAILURE;
    }
    abort_on_fail_.clear();
    return TCOPER_RET_SUCCESS;
  }

  /*send controller info to specific driver modules*/
  ret_val = SendRequestToDriver();

  abort_on_fail_.clear();
  driverset_map_.clear();
  notifyorder_.clear();
  pfc_log_debug("TwoPhaseCommit::Execute() exit");
  return ret_val;
}

}  // namespace tc
}  // namespace unc
