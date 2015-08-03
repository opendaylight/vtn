/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include "tcmsg_audit.hh"
#include "tc_operations.hh"

namespace unc {
namespace tc {

/*!\brief Parameterized constructor of TcMsgAudit.
 *@param[in] sess_id - session identifier.
 *@param[in] oper - operation type
 **/
TcMsgAudit::TcMsgAudit(uint32_t sess_id, tclib::TcMsgOperType oper)
    :TcMsg(sess_id, oper),
     controller_id_(""), driver_id_(UNC_CT_UNKNOWN),
     audit_type_(TC_AUDIT_NORMAL) { }

/*!\brief setter function for data members.
 *@param[in] controller_id - controller identifier.
 *@param[in] driver_id - driver module identifier.
 **/
void TcMsgAudit::SetData(uint32_t config_id,
                         std::string controller_id,
                         unc_keytype_ctrtype_t driver_id) {
  controller_id_ = controller_id;
  driver_id_ = driver_id;
}

/*!\brief method to set audit_type option
 * @param[in] audit_type - option to perforn audit based on TcAuditType */
void TcMsgAudit::SetAuditType(TcAuditType audit_type) {
  audit_type_ = audit_type; 
}

/*!\brief method to send transaction abort requests to recipient modules.
 *@param[in] abort_on_fail_ - vector with channel_ids of recipients to which
 * abort request has to be sent.
 *@result TcOperRet - TCOPER_RET_SUCCESS/TCOPER_RET_FAILURE
 **/
TcOperRet
TcMsgAudit::SendAbortRequest(AbortOnFailVector abort_on_fail_) {
  pfc_log_debug("TcMsgAudit::SendAbortRequest() entry");
  pfc_ipcresp_t resp = 0;
  pfc_ipcconn_t conn = 0;
  std::string channel_name;
  AbortOnFailVector::iterator it;
  tclib::TcAuditOpAbortPhase fail_phase = tclib::AUDIT_START;
  TcUtilRet util_resp = TCUTIL_RET_SUCCESS;

  /*channel_names_ map should not be empty */
  PFC_ASSERT(TCOPER_RET_SUCCESS == channel_names_.empty());
  PFC_ASSERT(TCOPER_RET_SUCCESS == abort_on_fail_.empty());

  if (PFC_EXPECT_TRUE(opertype_== tclib::MSG_AUDIT_VOTE)) {
    fail_phase = tclib::AUDIT_VOTE_REQUEST;
  } else {
    pfc_log_error("Invalid opertype_:%d", opertype_);
    return TCOPER_RET_FAILURE;
  }
  for (it = abort_on_fail_.begin(); it != abort_on_fail_.end(); it++) {
    channel_name = GetChannelName(*it);
    PFC_ASSERT(TCOPER_RET_SUCCESS == channel_name.empty());

    /*Create session for the given module name and service id*/
    pfc::core::ipc::ClientSession* abortsess =
        tc::TcClientSessionUtils::create_tc_client_session(channel_name,
                                  tclib::TCLIB_AUDIT_GLOBAL_ABORT, conn);
    if (NULL == abortsess) {
      return TCOPER_RET_FATAL;
    }
    /*append data to channel */
    util_resp = tc::TcClientSessionUtils::set_uint8(abortsess,
                                                    tclib::MSG_AUDIT_ABORT);
    if (PFC_EXPECT_TRUE(util_resp != TCUTIL_RET_SUCCESS)) {
      TcClientSessionUtils::tc_session_close(&abortsess, conn);
      return ReturnUtilResp(util_resp);
    }
    util_resp = tc::TcClientSessionUtils::set_uint32(abortsess, session_id_);
    if (PFC_EXPECT_TRUE(util_resp != TCUTIL_RET_SUCCESS)) {
      TcClientSessionUtils::tc_session_close(&abortsess, conn);
      return ReturnUtilResp(util_resp);
    }

    if (PFC_EXPECT_TRUE(driver_id_ >= UNC_CT_PFC) &&
        PFC_EXPECT_TRUE(driver_id_ <= UNC_CT_ODC)) {
      util_resp = tc::TcClientSessionUtils::set_uint8(abortsess, driver_id_);
      if (PFC_EXPECT_TRUE(util_resp != TCUTIL_RET_SUCCESS)) {
        TcClientSessionUtils::tc_session_close(&abortsess, conn);
        return ReturnUtilResp(util_resp);
      }
    } else {
      pfc_log_error("Invalid Driver ID");
      return TCOPER_RET_FAILURE;
    }
    if (PFC_EXPECT_TRUE(controller_id_.empty())) {
      pfc_log_error("Controller-ID is empty");
      return TCOPER_RET_FAILURE;
    } else {
      util_resp = tc::TcClientSessionUtils::set_string(abortsess,
                                                       controller_id_);
      if (PFC_EXPECT_TRUE(util_resp != TCUTIL_RET_SUCCESS)) {
        TcClientSessionUtils::tc_session_close(&abortsess, conn);
        return ReturnUtilResp(util_resp);
      }
    }
    util_resp = tc::TcClientSessionUtils::set_uint8(abortsess, fail_phase);
    if (PFC_EXPECT_TRUE(util_resp != TCUTIL_RET_SUCCESS)) {
      TcClientSessionUtils::tc_session_close(&abortsess, conn);
      return ReturnUtilResp(util_resp);
    }
    pfc_log_info("notify ABORT to %s - fail_phase:%d", channel_name.c_str(),
                 fail_phase);
    /*invoke session*/
    util_resp = tc::TcClientSessionUtils::tc_session_invoke(abortsess, resp);
    if (PFC_EXPECT_TRUE(util_resp != TCUTIL_RET_SUCCESS)) {
      TcClientSessionUtils::tc_session_close(&abortsess, conn);
      return ReturnUtilResp(util_resp);
    }
    TcClientSessionUtils::tc_session_close(&abortsess, conn);

    if (PFC_EXPECT_TRUE(resp == tclib::TC_FAILURE)) {
      pfc_log_error("Failure response from %s", channel_name.c_str());
      break;
    }
  }  // for()
  /*return server response */
  pfc_log_debug("TcMsgAudit::SendAbortRequest() exit");
  return RespondToTc(resp);
}

/*!\brief method to send audit/transaction end requests to recipient modules.
 *@param[in] abort_on_fail_ - channel_id of recipient modules to which
 *                                  audit/transaction end has to be sent. 
 *@param[in] oper - operation type
 *@result TcOperRet - TCOPER_RET_SUCCESS/TCOPER_RET_FAILURE
 **/
TcOperRet
TcMsgAudit::SendAuditTransEndRequest(AbortOnFailVector abort_on_fail_,
                                     tclib::TcMsgOperType oper) {
  pfc_log_debug("TcMsgAudit::SendAuditTransEndRequest() entry");
  pfc_ipcresp_t resp = 0;
  pfc_ipcconn_t conn = 0;
  std::string channel_name;
  AbortOnFailVector::reverse_iterator it;
  TcUtilRet util_resp = TCUTIL_RET_SUCCESS;

  /*channel_names_ map should not be empty */
  PFC_ASSERT(TCOPER_RET_SUCCESS == channel_names_.empty());
  PFC_ASSERT(TCOPER_RET_SUCCESS == abort_on_fail_.empty());

  if (PFC_EXPECT_TRUE(oper != tclib::MSG_AUDIT_TRANS_END) &&
      PFC_EXPECT_TRUE(oper != tclib::MSG_AUDIT_END)) {
    pfc_log_error("Invalid opertype:%d", oper);
    return TCOPER_RET_FAILURE;
  }

  if (oper == tclib::MSG_AUDIT_TRANS_END) {
    TcOperations::SetAuditPhase(AUDIT_TRANSACTION_END);
  } else if (oper == tclib::MSG_AUDIT_END) {
    TcOperations::SetAuditPhase(AUDIT_END);
  }

  for ( it = abort_on_fail_.rbegin(); it != abort_on_fail_.rend(); it++ ) {
    channel_name = GetChannelName(*it);
    if (PFC_EXPECT_TRUE(channel_name.empty())) {
      pfc_log_error("channel_name is empty");
      return TCOPER_RET_FAILURE;
    }
    /*Create session for the given module name and service id*/
    pfc::core::ipc::ClientSession* end_sess =
        tc::TcClientSessionUtils::create_tc_client_session(channel_name,
                                  tclib::TCLIB_AUDIT_TRANSACTION, conn);
    if (NULL == end_sess) {
      pfc_log_error("IPC invoke failed for channel %s",
                    channel_name.c_str());
      return TCOPER_RET_FATAL;
    }
    /*append data to channel */
    pfc_log_info("notify Audit(Tx)End to %s", channel_name.c_str());
    util_resp = tc::TcClientSessionUtils::set_uint8(end_sess, oper);
    if (PFC_EXPECT_TRUE(util_resp != TCUTIL_RET_SUCCESS)) {
      TcClientSessionUtils::tc_session_close(&end_sess, conn);
      return ReturnUtilResp(util_resp);
    }
    util_resp = tc::TcClientSessionUtils::set_uint32(end_sess, session_id_);
    if (PFC_EXPECT_TRUE(util_resp != TCUTIL_RET_SUCCESS)) {
      TcClientSessionUtils::tc_session_close(&end_sess, conn);
      return ReturnUtilResp(util_resp);
    }
    if (PFC_EXPECT_TRUE(driver_id_ >= UNC_CT_PFC) &&
        PFC_EXPECT_TRUE(driver_id_ <= UNC_CT_ODC)) {
      util_resp = tc::TcClientSessionUtils::set_uint8(end_sess, driver_id_);
      if (PFC_EXPECT_TRUE(util_resp != TCUTIL_RET_SUCCESS)) {
        TcClientSessionUtils::tc_session_close(&end_sess, conn);
        return ReturnUtilResp(util_resp);
      }
    } else {
      pfc_log_error("Invalid Driver ID");
      return TCOPER_RET_FAILURE;
    }
    if (PFC_EXPECT_TRUE(controller_id_.empty())) {
      pfc_log_error("Controller-ID is empty");
      return TCOPER_RET_FAILURE;
    } else {
      util_resp = tc::TcClientSessionUtils::set_string(end_sess,
                                                       controller_id_);
      if (PFC_EXPECT_TRUE(util_resp != TCUTIL_RET_SUCCESS)) {
        TcClientSessionUtils::tc_session_close(&end_sess, conn);
        return ReturnUtilResp(util_resp);
      }
    }
    if (PFC_EXPECT_TRUE(oper == tclib::MSG_AUDIT_TRANS_END)) {
      util_resp = tc::TcClientSessionUtils::set_uint8(end_sess, trans_result_);
      if (PFC_EXPECT_TRUE(util_resp != TCUTIL_RET_SUCCESS)) {
        TcClientSessionUtils::tc_session_close(&end_sess, conn);
        return ReturnUtilResp(util_resp);
      }
      pfc_log_info("trans_result_:%d", trans_result_);
    }
    if (PFC_EXPECT_TRUE(oper == tclib::MSG_AUDIT_END)) {
      util_resp = tc::TcClientSessionUtils::set_uint8(end_sess, audit_result_);
      if (PFC_EXPECT_TRUE(util_resp != TCUTIL_RET_SUCCESS)) {
        TcClientSessionUtils::tc_session_close(&end_sess, conn);
        return ReturnUtilResp(util_resp);
      }
      pfc_log_info("audit_result_:%d", audit_result_);
    }
    /*invoke session*/
    util_resp = tc::TcClientSessionUtils::tc_session_invoke(end_sess, resp);
    if (PFC_EXPECT_TRUE(util_resp != TCUTIL_RET_SUCCESS)) {
      TcClientSessionUtils::tc_session_close(&end_sess, conn);
      return ReturnUtilResp(util_resp);
    }
    TcClientSessionUtils::tc_session_close(&end_sess, conn);

    if (PFC_EXPECT_TRUE(resp == tclib::TC_FAILURE)) {
      pfc_log_info("Failure response from %s", channel_name.c_str());
      break;
    }
  }  // for ()
  return RespondToTc(resp);
}

/*!\brief Parameterized constructor of GetDriverId.
 *@param[in] sess_id - session identifier.
 *@param[in] oper - operation type
 **/
GetDriverId::GetDriverId(uint32_t sess_id,
                         tclib::TcMsgOperType oper)
:TcMsgAudit(sess_id, oper) {}

/*!\brief TC service handler invokes this method to get driver_id from UPPL for
 * driver triggered audit operation.
 *@result TcOperRet - TCOPER_RET_SUCCESS/TCOPER_RET_FAILURE
 **/
TcOperRet GetDriverId::Execute() {
  pfc_log_debug("GetDriverId::Execute() entry");
  pfc_ipcresp_t resp = 0;
  TcUtilRet util_resp = TCUTIL_RET_SUCCESS;

  /*channel_names_ map should not be empty */
  PFC_ASSERT(TCOPER_RET_SUCCESS == channel_names_.empty());
  PFC_ASSERT(TCOPER_RET_SUCCESS == controller_id_.empty());

  std::string channel_name = GetChannelName(TC_UPPL);
  PFC_ASSERT(TCOPER_RET_SUCCESS == channel_name.empty());

  sess_ = tc::TcClientSessionUtils::create_tc_client_session(channel_name,
                                    tclib::TCLIB_GET_DRIVERID,
                                    conn_, PFC_FALSE);
  if (NULL == sess_) {
    return TCOPER_RET_FATAL;
  }
  PFC_ASSERT(TCOPER_RET_SUCCESS == controller_id_.empty());
  util_resp = tc::TcClientSessionUtils::set_string(sess_, controller_id_);
  if (PFC_EXPECT_TRUE(util_resp != TCUTIL_RET_SUCCESS)) {
    return ReturnUtilResp(util_resp);
  }
  pfc_log_info("notify %s - cntrlr_id:%s",
               channel_name.c_str(), controller_id_.c_str());
  /*invoke session*/
  util_resp = tc::TcClientSessionUtils::tc_session_invoke(sess_, resp);
  if (PFC_EXPECT_TRUE(util_resp != TCUTIL_RET_SUCCESS)) {
    return ReturnUtilResp(util_resp);
  }

  driver_id_ = (unc_keytype_ctrtype_t)resp;
  pfc_log_info("Controller Type from UPPL for User AUDIT:%d", driver_id_);

  return TCOPER_RET_SUCCESS;
}

/*!\brief getter function for driver id obtained from UPPL
 * for driver triggered audit operation.
 *@result driver id of type unc_keytype_ctrtype_t.
 **/
unc_keytype_ctrtype_t GetDriverId::GetResult() {
  return driver_id_;
}

/*!\brief Parameterized constructor of AuditTransaction.
 *@param sess_id - session identifier.
 *@param oper - operation type
 **/
AuditTransaction::AuditTransaction(uint32_t sess_id, tclib::TcMsgOperType oper)
    :TcMsgAudit(sess_id , oper), reconnect_controller_(PFC_FALSE) {
  commit_number_      = 0;
  commit_date_        = 0;
  commit_application_ = "";
  user_audit_         = PFC_FALSE;
}

/*!\brief method to set user audit
 * @param[in] user_audit - option to set user audit type*/ 
void AuditTransaction::IsUserAudit(pfc_bool_t user_audit) {
  user_audit_ = user_audit;
}

/*!\brief method to set reconnect option
 * @param[in] force_reconnect - option to perforn audit after reconnecting with
 * the controller*/ 
void AuditTransaction::SetReconnect(pfc_bool_t force_reconnect) {
  reconnect_controller_ = force_reconnect;
}

/*!\brief method to set PFC commit version,commit date and commit_application
 * @param[in] commit_number - latest commit version of PFC 
 * @param[in] commit_date - latest commit date of PFC 
 * @param[in] commit_application - Application that performed commit operation
 **/ 

void AuditTransaction::SetCommitInfo(uint64_t commit_number,
                                     uint64_t commit_date,
                                     std::string commit_application) {
  commit_number_ = commit_number;
  commit_date_ = commit_date;
  commit_application_ = commit_application;
}

/*!\brief this method sends send Audit/Transaction START/END
 *request to recipient modules.
 *@param[in] channel_name - channel name of recipient module
 *@result TcOperRet - TCOPER_RET_SUCCESS/TCOPER_RET_FAILURE
 **/
TcOperRet
AuditTransaction::SendRequest(std::string channel_name) {
  pfc_log_debug("AuditTransaction::SendRequest() entry");
  pfc_ipcresp_t resp = 0;
  TcOperRet ret_val = TCOPER_RET_SUCCESS;
  TcUtilRet util_resp = TCUTIL_RET_SUCCESS;

  PFC_ASSERT(TCOPER_RET_SUCCESS == channel_name.empty());

  /*Create session for the given module name and service id*/
  sess_ = tc::TcClientSessionUtils::create_tc_client_session(channel_name,
                                    tclib::TCLIB_AUDIT_TRANSACTION, conn_);
  if (NULL == sess_) {
    return TCOPER_RET_FATAL;
  }
  pfc_log_info("notify %s - session_id:%d cntrlr_id:%s",
               channel_name.c_str(), session_id_, controller_id_.c_str());

  /*append data to channel */
  util_resp = tc::TcClientSessionUtils::set_uint8(sess_, opertype_);
  if (PFC_EXPECT_TRUE(util_resp != TCUTIL_RET_SUCCESS)) {
    pfc_log_error("SendRequest: Setting opertype_ failed");
    return ReturnUtilResp(util_resp);
  }
  util_resp = tc::TcClientSessionUtils::set_uint32(sess_, session_id_);
  if (PFC_EXPECT_TRUE(util_resp != TCUTIL_RET_SUCCESS)) {
    pfc_log_error("SendRequest: Setting session_id_ failed");
    return ReturnUtilResp(util_resp);
  }
  util_resp = tc::TcClientSessionUtils::set_uint8(sess_, driver_id_);
  if (PFC_EXPECT_TRUE(util_resp != TCUTIL_RET_SUCCESS)) {
    pfc_log_error("SendRequest: Setting driver_id_ failed");
    return ReturnUtilResp(util_resp);
  }
  util_resp = tc::TcClientSessionUtils::set_string(sess_, controller_id_);
  if (PFC_EXPECT_TRUE(util_resp != TCUTIL_RET_SUCCESS)) {
    pfc_log_error("SendRequest: Setting controller_id_ failed");
    return ReturnUtilResp(util_resp);
  }
  if (opertype_ == tclib::MSG_AUDIT_START) {
    util_resp = tc::TcClientSessionUtils::
        set_uint8(sess_, (uint8_t)reconnect_controller_);
    if (PFC_EXPECT_TRUE(util_resp != TCUTIL_RET_SUCCESS)) {
      pfc_log_error("SendRequest: Setting reconnect_controller_ failed");
      return ReturnUtilResp(util_resp);
    }
     
    util_resp = tc::TcClientSessionUtils::
        set_uint8(sess_, (uint8_t)audit_type_);
    if (PFC_EXPECT_TRUE(util_resp != TCUTIL_RET_SUCCESS)) {
      pfc_log_error("SendRequest: Setting audit_type_ failed");
      return ReturnUtilResp(util_resp);
    }

    /* Updates controller commit details */
    util_resp = tc::TcClientSessionUtils::set_uint64(sess_, commit_number_);
    if (PFC_EXPECT_TRUE(util_resp != TCUTIL_RET_SUCCESS)) {
      pfc_log_error("SendRequest: Setting commit_number_ failed");
      return ReturnUtilResp(util_resp);
    }
    util_resp = tc::TcClientSessionUtils::set_uint64(sess_, commit_date_);
    if (PFC_EXPECT_TRUE(util_resp != TCUTIL_RET_SUCCESS)) {
      pfc_log_error("SendRequest: Setting commit_date failed");
      return ReturnUtilResp(util_resp);
    }
    util_resp = tc::TcClientSessionUtils::set_string(sess_,
                                                     commit_application_);
    if (PFC_EXPECT_TRUE(util_resp != TCUTIL_RET_SUCCESS)) {
      pfc_log_error("SendRequest: Setting commit_application failed");
      return ReturnUtilResp(util_resp);
    }
  }
  if (opertype_ == tclib::MSG_AUDIT_TRANS_END) {
    util_resp = tc::TcClientSessionUtils::set_uint8(sess_, trans_result_);
    if (PFC_EXPECT_TRUE(util_resp != TCUTIL_RET_SUCCESS)) {
      pfc_log_error("SendRequest: Setting trans_result_ failed");
      return ReturnUtilResp(util_resp);
    }
  }
  if (opertype_ == tclib::MSG_AUDIT_END) {
    util_resp = tc::TcClientSessionUtils::set_uint8(sess_, audit_result_);
    if (PFC_EXPECT_TRUE(util_resp != TCUTIL_RET_SUCCESS)) {
      pfc_log_error("SendRequest: Setting audit_result_ failed");
      return ReturnUtilResp(util_resp);
    }
  }
  /*invoke session*/
  util_resp = tc::TcClientSessionUtils::tc_session_invoke(sess_, resp);
  if (PFC_EXPECT_TRUE(util_resp != TCUTIL_RET_SUCCESS)) {
    pfc_log_error("SendRequest: Session invoke failed");
    return ReturnUtilResp(util_resp);
  }

  if (PFC_EXPECT_TRUE((resp == tclib::TC_SIMPLIFIED_AUDIT) ||
                      (resp == tclib::TC_SUCCESS)) && 
      PFC_EXPECT_TRUE(opertype_ == tclib::MSG_AUDIT_START)) {

    uint32_t respcount = 0;
    uint64_t commit_number = 0;
    uint64_t commit_date = 0;
    std::string commit_application;

    respcount = sess_->getResponseCount();
    
    if (respcount < 6) {
      pfc_log_info("SendRequest: No Commit Info Received ");
    } else {
      util_resp = unc::tc::TcClientSessionUtils::get_uint64(sess_,
                                                    COMMIT_INFO_START_POS,
                                                    &commit_number);
      if (util_resp != TCUTIL_RET_SUCCESS) {
        pfc_log_error("SendRequest: Getting commit_number failed");
        return ReturnUtilResp(util_resp);
      }
     
      util_resp = unc::tc::TcClientSessionUtils::get_uint64(sess_,
                                                    COMMIT_INFO_START_POS + 1,
                                                    &commit_date);
      if (util_resp != TCUTIL_RET_SUCCESS) {
        pfc_log_error("SendRequest: Getting commit_version failed");
        return ReturnUtilResp(util_resp);
      }
     
      util_resp = unc::tc::TcClientSessionUtils::get_string(sess_,
                                                    COMMIT_INFO_START_POS + 2,
                                                    commit_application);
      if (util_resp != TCUTIL_RET_SUCCESS) {
        pfc_log_error("SendRequest: Getting commit_application failed");
        return ReturnUtilResp(util_resp);
      }
     
      SetCommitInfo(commit_number, commit_date, commit_application);
    }
  }
  /*handle server response*/
  if (PFC_EXPECT_TRUE(resp == tclib::TC_SUCCESS)) {
    pfc_log_info("Success response from %s", channel_name.c_str());
    ret_val = TCOPER_RET_SUCCESS;
  } else if (PFC_EXPECT_TRUE(resp == tclib::TC_SIMPLIFIED_AUDIT) &&
             PFC_EXPECT_TRUE(opertype_ == tclib::MSG_AUDIT_START)) {
    pfc_log_info("Simplified Audit response from %s", channel_name.c_str());
    if (user_audit_ != PFC_TRUE) {
      pfc_log_info("Audit from Driver; Simplified Audit takes place");
      SetAuditType(TC_AUDIT_SIMPLIFIED);
      ret_val = TCOPER_RET_SUCCESS;
    }
    else {
      pfc_log_info("Audit from User, No Simplified Audit takes place");
      ret_val = TCOPER_RET_SUCCESS;
    }
  } else if (resp == tclib::TC_CANCELLED_AUDIT &&
             opertype_ == tclib::MSG_AUDIT_START) {
    TcOperations::SetAuditPhase(AUDIT_END);
    pfc_log_info("%s Received TC_CANCELLED_AUDIT response from %s",
                 __FUNCTION__, channel_name.c_str());
    TcDaemonName daemon = GetDaemonName(channel_name);
    if (daemon == TC_NONE) {
      pfc_log_fatal("%s Error getting Daemon name for channel:%s",
                    __FUNCTION__, channel_name.c_str());
      return TCOPER_RET_AUDIT_CANCELLED;
    }
    pfc_log_info("%s Adding current Daemon %d (channel:%s) to send AuditEnd",
                 __FUNCTION__, daemon, channel_name.c_str());
    abort_on_fail_.push_back(daemon);
    audit_result_ = tclib::TC_AUDIT_CANCELLED;
    if (PFC_EXPECT_TRUE(abort_on_fail_.empty())) {
      ret_val = TCOPER_RET_AUDIT_CANCELLED;
    } else {
      ret_val = SendAuditTransEndRequest(abort_on_fail_, tclib::MSG_AUDIT_END);
      if (ret_val == TCOPER_RET_SUCCESS) {
        ret_val = TCOPER_RET_AUDIT_CANCELLED;
      } else if (ret_val == TCOPER_RET_FAILURE) {
        pfc_log_error("%s Sending AuditEnd(abort) during AuditStart err failed",
                      __FUNCTION__);
        ret_val = TCOPER_RET_FATAL;
      }
      abort_on_fail_.clear();
    }
  } else if (PFC_EXPECT_TRUE(resp == tclib::TC_FAILURE) &&
            PFC_EXPECT_TRUE(opertype_ == tclib::MSG_AUDIT_START)) {
    TcOperations::SetAuditPhase(AUDIT_END);
    pfc_log_info("Failure response from %s", channel_name.c_str());
    audit_result_ = tclib::TC_AUDIT_FAILURE;
    /* Audit start failed in driver itself*/
    if (PFC_EXPECT_TRUE(abort_on_fail_.empty())) {
      ret_val = TCOPER_RET_ABORT;
    } else {
      ret_val = SendAuditTransEndRequest(abort_on_fail_, tclib::MSG_AUDIT_END);
      if (PFC_EXPECT_TRUE(ret_val == TCOPER_RET_SUCCESS)) {
        ret_val = TCOPER_RET_ABORT;
      } else if (PFC_EXPECT_TRUE(ret_val == TCOPER_RET_FAILURE)) {
        ret_val = TCOPER_RET_FATAL;
      }
      abort_on_fail_.clear();
    }
  } else if (resp == tclib::TC_CANCELLED_AUDIT &&
             opertype_ == tclib::MSG_AUDIT_TRANS_START) {
    TcOperations::SetAuditPhase(AUDIT_TRANSACTION_END);
    pfc_log_info("%s Received TC_CANCELLED_AUDIT resp from %s",
                 __FUNCTION__, channel_name.c_str());
    // AuditEnd sending is handled in TcAuditOperations::Execute
    ret_val = TCOPER_RET_AUDIT_CANCELLED;
  
    pfc_log_info("%s AuditTxStart received TC_CANCELLED_AUDIT response from %s",
                 __FUNCTION__, channel_name.c_str());
    TcDaemonName daemon = GetDaemonName(channel_name);
    if (daemon == TC_NONE) {
      pfc_log_fatal("%s Error getting Daemon name for channel:%s",
                    __FUNCTION__, channel_name.c_str());
      return TCOPER_RET_AUDIT_CANCELLED;
    }
    pfc_log_info("%s Adding current Daemon %d (channel:%s) to send AuditTxEnd",
                 __FUNCTION__, daemon, channel_name.c_str());
    abort_on_fail_.push_back(daemon);
    trans_result_ = tclib::TRANS_END_SUCCESS;
    audit_result_ = tclib::TC_AUDIT_CANCELLED;
    if (PFC_EXPECT_TRUE(abort_on_fail_.empty())) {
      ret_val = TCOPER_RET_AUDIT_CANCELLED;
    } else {
      ret_val = SendAuditTransEndRequest(abort_on_fail_,
                                         tclib::MSG_AUDIT_TRANS_END);
      if (ret_val == TCOPER_RET_SUCCESS) {
        ret_val = TCOPER_RET_AUDIT_CANCELLED;
      } else if (ret_val == TCOPER_RET_FAILURE) {
        pfc_log_error("%s Sending AuditTxEnd(abort) during AuditTxStart failed",
                      __FUNCTION__);
        ret_val = TCOPER_RET_FATAL;
      }
      abort_on_fail_.clear();
    }
  } else if (PFC_EXPECT_TRUE(resp == tclib::TC_FAILURE) &&
             PFC_EXPECT_TRUE(opertype_ == tclib::MSG_AUDIT_TRANS_START)) {
    TcOperations::SetAuditPhase(AUDIT_TRANSACTION_END);
    pfc_log_info("Failure response from %s", channel_name.c_str());
    /*append end result in audit end notification*/
    audit_result_ = tclib::TC_AUDIT_FAILURE;
    trans_result_ = tclib::TRANS_END_FAILURE;
    if (PFC_EXPECT_TRUE(abort_on_fail_.empty())) {
      ret_val = TCOPER_RET_ABORT;
    } else if (PFC_EXPECT_TRUE(!abort_on_fail_.empty())) {
      /*append end result in transaction end notification*/
      ret_val = SendAuditTransEndRequest(abort_on_fail_,
                                         tclib::MSG_AUDIT_TRANS_END);
      if (PFC_EXPECT_TRUE(ret_val != TCOPER_RET_SUCCESS)) {
        ret_val = TCOPER_RET_FATAL;
      } else {
        ret_val = TCOPER_RET_ABORT;
      }
      abort_on_fail_.clear();
    }
  } else if (PFC_EXPECT_TRUE(resp == tclib::TC_FAILURE) &&
            (PFC_EXPECT_TRUE(opertype_ == tclib::MSG_AUDIT_END ||
                             PFC_EXPECT_TRUE(opertype_ ==
                                             tclib::MSG_AUDIT_TRANS_END)))) {
    pfc_log_error("Audit end/Transaction end failure");
    audit_result_ = tclib::TC_AUDIT_FAILURE;
    ret_val = TCOPER_RET_FATAL;
  }
  /*session is not closed in case of failure as its contents
   * are forwarded to VTN*/
  if (PFC_EXPECT_TRUE((ret_val == TCOPER_RET_SUCCESS) ||
                     (ret_val == TCOPER_RET_SIMPLIFIED_AUDIT))) {
    TcClientSessionUtils::tc_session_close(&sess_, conn_);
  }
  pfc_log_debug("AuditTransaction::SendRequest() exit");
  return ret_val;
}

/*!\brief TC service handler invokes this method to send Audit/Transaction START/END
 * request to recipient modules.
 *@result TcOperRet - TCOPER_RET_SUCCESS/TCOPER_RET_FAILURE
 **/
TcOperRet AuditTransaction::Execute() {
  pfc_log_debug("AuditTransaction::Execute() entry");
  TcDaemonName tc_driverid;
  std::string channel_name;
  TcOperRet ret_val = TCOPER_RET_SUCCESS;

  /*channel_names_ map should not be empty */
  PFC_ASSERT(TCOPER_RET_SUCCESS == channel_names_.empty());
  /*validate driver_id_ and controller i
   * session-id can be 0 incase of driver-audit */
  if (PFC_EXPECT_TRUE(driver_id_ < UNC_CT_PFC) ||
      PFC_EXPECT_TRUE(driver_id_ > UNC_CT_ODC) ||
      PFC_EXPECT_TRUE(controller_id_.empty())) {
    pfc_log_error("Invalid Driver/Controller ID");
    return TCOPER_RET_FAILURE;
  }

  switch (opertype_) {
    /*audit start is send to specific driver_id alone.*/
    case tclib::MSG_AUDIT_START: {
      pfc_log_info("*** AUDIT START ***");
      tc_driverid = MapTcDriverId(driver_id_);

      notifyorder_.push_back(tc_driverid);
      notifyorder_.push_back(TC_UPPL);
      notifyorder_.push_back(TC_UPLL);
      break;
    }
    case tclib::MSG_AUDIT_END: {
      pfc_log_info("*** AUDIT END ***");
      tc_driverid = MapTcDriverId(driver_id_);

      notifyorder_.push_back(TC_UPPL);
      notifyorder_.push_back(TC_UPLL);
      notifyorder_.push_back(tc_driverid);
      break;
    }
    case tclib::MSG_AUDIT_TRANS_START: {
      pfc_log_info("*** AUDIT TxSTART ***");
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
    case tclib::MSG_AUDIT_TRANS_END: {
      pfc_log_info("*** AUDIT TxEND ***");
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
      pfc_log_error("Invalid opertype:%d", opertype_);
      return TCOPER_RET_FAILURE;
    }
  }

  for (NotifyList::iterator list_iter = notifyorder_.begin();
      list_iter != notifyorder_.end(); list_iter++) {
    pfc_log_debug("GetChannelName for %u", *list_iter);
    channel_name = GetChannelName(*list_iter);
    if (opertype_ == tclib::MSG_AUDIT_START && channel_name.empty()) {
      // Add-on driver: If driver not present, return error
      pfc_log_error("Driver not present. Return TCOPER_RET_NO_DRIVER");
      ret_val = TCOPER_RET_NO_DRIVER;
      break;
    } else if (channel_name.empty()) {
      // For TxStart or TxEnd msgs sent to all drivers.
      // If a driver is not present, skip
      pfc_log_info("Channel for %u not available. Skipping", *list_iter);
      continue;
    }

    if ( TcMsg::GetAuditCancelFlag() == PFC_TRUE &&
             audit_type_ != TC_AUDIT_REALNETWORK &&
         (opertype_ == tclib::MSG_AUDIT_START ||
          opertype_ == tclib::MSG_AUDIT_TRANS_START) ) {
      pfc_log_warn("%s AuditCancel flag is set. Stopping Audit process",
                   __FUNCTION__);

      if (!abort_on_fail_.empty()) {
        if (opertype_ == tclib::MSG_AUDIT_START) {
          pfc_log_info("%s Sending AuditEnd for AuditStart completed modules",
                       __FUNCTION__);
          audit_result_ = tclib::TC_AUDIT_CANCELLED;
          ret_val = SendAuditTransEndRequest(abort_on_fail_,
                                             tclib::MSG_AUDIT_END);
          if (ret_val != TCOPER_RET_SUCCESS) {
            pfc_log_error("%s Sending AuditEnd during AuditStart failed",
                          __FUNCTION__);
          }
        } else if (opertype_ == tclib::MSG_AUDIT_TRANS_START) {
          pfc_log_info("%s Sending AuditTxEnd for AuditTxStart comp modules",
                       __FUNCTION__);
          audit_result_ = tclib::TC_AUDIT_CANCELLED;
          trans_result_ = tclib::TRANS_END_SUCCESS;
          ret_val = SendAuditTransEndRequest(abort_on_fail_,
                                             tclib::MSG_AUDIT_TRANS_END);
          if (ret_val != TCOPER_RET_SUCCESS) {
            pfc_log_error("%s Sending AuditTxEnd during AuditTxStart failed",
                          __FUNCTION__);
          }
        }
      }
      ret_val = TCOPER_RET_AUDIT_CANCELLED;
      break;
    }

    // Save the modules for which AUDIT_START has been sent
    // In case of audit-cancel, all the modules which received
    // AUDIT_START needs to be notified
    if (opertype_ == tclib::MSG_AUDIT_START) {
      pfc::core::ScopedMutex m(TcOperations::audit_cancel_notify_lock_);
      TcOperations::audit_cancel_notify_.push_back(*list_iter);
    }

    ret_val = SendRequest(channel_name);

    if (PFC_EXPECT_TRUE(ret_val != TCOPER_RET_SUCCESS)) {
      return ret_val;
    } else if (PFC_EXPECT_TRUE(ret_val == TCOPER_RET_SUCCESS) ) {
      if (audit_type_ == TC_AUDIT_SIMPLIFIED) {
        ret_val = TCOPER_RET_SIMPLIFIED_AUDIT;
      }
      /*append channel info to handle failure scenario*/
      abort_on_fail_.push_back(*list_iter);
    }
  }

  /*clear map*/
  abort_on_fail_.clear();
  notifyorder_.clear();
  pfc_log_debug("AuditTransaction::Execute() exit");
  return ret_val;
}

/*!\brief Parameterized constructor of TwoPhaseAudit
 *@param sess_id - session identifier.
 *@param oper - operation type
 **/
TwoPhaseAudit::TwoPhaseAudit(uint32_t sess_id,
                             tclib::TcMsgOperType oper)
:TcMsgAudit(sess_id, oper) {}

/*!brief method to set the upll/uppl sessions with default attributes to send
 *driver result
 * @param[in] tmpsess - UPLL/UPPL session pointer
 * @result - TCOPER_RET_SUCCESS/TCOPER_RET_FAILURE
 * */
TcOperRet
TwoPhaseAudit::SetSessionToForwardDriverResult(pfc::core::ipc::ClientSession*
                                               tmpsess) {
  pfc_log_debug("TwoPhaseAudit::SetSessionToForwardDriverResult entry");
  tclib::TcCommitPhaseType phase;
  tclib::TcMsgOperType oper;
  TcUtilRet util_resp = TCUTIL_RET_SUCCESS;

  switch (opertype_) {
    case tclib::MSG_AUDIT_VOTE: {
      oper = tclib::MSG_AUDIT_VOTE_DRIVER_RESULT;
      phase = tclib::TC_AUDIT_VOTE_PHASE;
      break;
    }
    case tclib::MSG_AUDIT_GLOBAL: {
      oper = tclib::MSG_AUDIT_GLOBAL_DRIVER_RESULT;
      phase = tclib::TC_AUDIT_GLOBAL_COMMIT_PHASE;
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
    pfc_log_error("SetSessionToForwardDriverResult: Setting oper failed");
    return ReturnUtilResp(util_resp);
  }
  /*validate session_id_ and config_id_*/
  util_resp = TcClientSessionUtils::set_uint32(tmpsess, session_id_);
  if (PFC_EXPECT_TRUE(util_resp != TCUTIL_RET_SUCCESS)) {
    pfc_log_error("SetSessionToForwardDriverResult: Setting sess_id failed");
    return ReturnUtilResp(util_resp);
  }

  PFC_ASSERT(TCOPER_RET_SUCCESS == controller_id_.empty());
  util_resp = TcClientSessionUtils::set_string(tmpsess, controller_id_);
  if (PFC_EXPECT_TRUE(util_resp != TCUTIL_RET_SUCCESS)) {
    pfc_log_error("SetSessionToForwardDriverResult: Setting ctrl_id failed");
    return ReturnUtilResp(util_resp);
  }

  SetNotifyControllerId(controller_id_);

  util_resp = TcClientSessionUtils::set_uint8(tmpsess, phase);
  if (PFC_EXPECT_TRUE(util_resp != TCUTIL_RET_SUCCESS)) {
    pfc_log_error("SetSessionToForwardDriverResult: Setting phase failed");
    return ReturnUtilResp(util_resp);
  }
  pfc_log_debug("TwoPhaseAudit::SetSessionToForwardDriverResult exit");
  return TCOPER_RET_SUCCESS;
}


/*! brief method to create  UPLL/UPPL sessions to forward driver result
 *  * @result - TCOPER_RET_SUCCESS/FAILURE*/
TcOperRet TwoPhaseAudit::CreateSessionsToForwardDriverResult() {
  pfc_log_debug("TwoPhaseAudit::CreateSessionsToForwardDriverResult() entry");
  TcOperRet ret_val = TCOPER_RET_SUCCESS;

  std::string channel_name = GetChannelName(TC_UPLL);
  PFC_ASSERT(TCOPER_RET_SUCCESS == channel_name.empty());
  upll_sess_ = TcClientSessionUtils::create_tc_client_session(channel_name,
                                     tclib::TCLIB_AUDIT_DRV_RESULT,
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
                                     tclib::TCLIB_AUDIT_DRV_RESULT, uppl_conn_);
  if (NULL == uppl_sess_) {
    return TCOPER_RET_FATAL;
  }

  ret_val = SetSessionToForwardDriverResult(uppl_sess_);
  if (PFC_EXPECT_TRUE(ret_val != TCOPER_RET_SUCCESS)) {
    pfc_log_error("SetSessionToForwardDriverResult failed");
    return TCOPER_RET_FAILURE;
  }
  pfc_log_debug("TwoPhaseAudit::CreateSessionsToForwardDriverResult() exit");
  return TCOPER_RET_SUCCESS;
}

/*! brief method to invoke UPLL/UPPL sessions to forward driver result
 *  @param[in] tmpsess - UPLL/UPPL session pointer
 *  @result - TCOPER_RET_SUCCESS/FAILURE*/

TcOperRet
TwoPhaseAudit::HandleDriverResultResponse(pfc::core::ipc::ClientSession*
                                                    tmpsess) {
  pfc_log_debug("TwoPhaseAudit::HandleDriverResultResponse entry");
  TcUtilRet util_resp = TCUTIL_RET_SUCCESS;
  TcOperRet ret_val = TCOPER_RET_SUCCESS;
  pfc_ipcresp_t resp = 0;

  util_resp = TcClientSessionUtils::tc_session_invoke(tmpsess, resp);
  if (PFC_EXPECT_TRUE(util_resp != TCUTIL_RET_SUCCESS)) {
    return ReturnUtilResp(util_resp);
  }
  /*retrieve audit result from driver result response*/
  if (PFC_EXPECT_TRUE(opertype_ == tclib::MSG_AUDIT_GLOBAL) &&
      PFC_EXPECT_TRUE(resp == tclib::TC_SUCCESS)) {
    uint8_t audit_ret = 0;
    uint32_t respcount = tmpsess->getResponseCount();
    util_resp = unc::tc::TcClientSessionUtils::get_uint8(tmpsess,
                                                         respcount-1,
                                                         &audit_ret);
    if (util_resp != TCUTIL_RET_SUCCESS) {
      pfc_log_error("HandleDriverResultResponse: Error getting audit_ret");
      return ReturnUtilResp(util_resp);
    }
    audit_result_ = (tclib::TcAuditResult)(audit_result_ && audit_ret);
    pfc_log_info("audit_result:%d", audit_result_);
  }

  if (PFC_EXPECT_TRUE(resp == tclib::TC_FAILURE) &&
      PFC_EXPECT_TRUE(opertype_ ==  tclib::MSG_AUDIT_VOTE)) {
    pfc_log_info("failure response for vote-driver result");
    trans_result_ = tclib::TRANS_END_FAILURE;
    audit_result_ = tclib::TC_AUDIT_FAILURE;
    ret_val = SendAbortRequest(abort_on_fail_);
    if (PFC_EXPECT_TRUE(ret_val == TCOPER_RET_SUCCESS)) {
      ret_val = TCOPER_RET_ABORT;
    } else {
      pfc_log_error("SendAbortRequest failed");
      ret_val = TCOPER_RET_FATAL;
    }
  } else if (PFC_EXPECT_TRUE(resp == tclib::TC_FAILURE) &&
             PFC_EXPECT_TRUE(opertype_ == tclib::MSG_AUDIT_GLOBAL)) {
    pfc_log_info("failure response for global commit-driver result");
    audit_result_ = tclib::TC_AUDIT_FAILURE;
    ret_val = TCOPER_RET_FATAL;
  } else {
     pfc_log_info("success response for driver result");
  }
  pfc_log_debug("TwoPhaseAudit::HandleDriverResultResponse exit");
  return ret_val;
}



/*!\brief method to accumulate the controller info from UPLL/UPPL in
 *TcControllerInfoMap
 *@param sess_ - client session parameter
 *@result TcOperRet - TCOPER_RET_SUCCESS/TCOPER_RET_FAILURE
 **/
TcOperRet
TwoPhaseAudit::GetControllerInfo(pfc::core::ipc::ClientSession* sess_) {
  pfc_log_debug("GetControllerInfo() entry");

  uint8_t driver_count = 0, controller_count = 0, driver_id = 0;
  unc_keytype_ctrtype_t driver_type;
  uint32_t  idx = 0;
  ControllerList controllers, temp_list;
  std::string controller_id;
  TcUtilRet util_resp = TCUTIL_RET_SUCCESS;

  util_resp = tc::TcClientSessionUtils::get_uint8(sess_, idx, &driver_count);
  if (PFC_EXPECT_TRUE(util_resp != TCUTIL_RET_SUCCESS)) {
    pfc_log_error("GetControllerInfo: Getting driver_count failed");
    return ReturnUtilResp(util_resp);
  }
  pfc_log_debug("driver_count:%d", driver_count);

  for (int i = 1; i <= driver_count; i++) {
    idx++;
    util_resp = tc::TcClientSessionUtils::get_uint8(sess_, idx, &driver_id);
    if (PFC_EXPECT_TRUE(util_resp != TCUTIL_RET_SUCCESS)) {
      pfc_log_error("GetControllerInfo: Getting driver_id failed");
      return ReturnUtilResp(util_resp);
    }
    driver_type = (unc_keytype_ctrtype_t)driver_id;
    /*save the controller ids returned by UPLL/UPPL for validating driver vote
     * request*/
    driver_set_.insert(driver_type);

    idx++;
    util_resp = tc::TcClientSessionUtils::get_uint8(sess_, idx,
                                                    &controller_count);
    if (PFC_EXPECT_TRUE(util_resp != TCUTIL_RET_SUCCESS)) {
      pfc_log_error("GetControllerInfo: Getting ctrl_count failed");
      return ReturnUtilResp(util_resp);
    }
    pfc_log_info("driver_id:%d controller_count:%d",
                 driver_type, controller_count);
    for (int j = 1; j <= controller_count; j++) {
      idx++;
      util_resp = tc::TcClientSessionUtils::get_string(sess_, idx,
                                                       controller_id);
      if (PFC_EXPECT_TRUE(util_resp != TCUTIL_RET_SUCCESS)) {
        pfc_log_error("GetControllerInfo: Getting ctrl_id failed");
        return ReturnUtilResp(util_resp);
      }
      controllers.push_back(controller_id);
    }

    /*cntrlinfo map is empty or doesnt contain info on the driver type*/
    if (PFC_EXPECT_TRUE(driverinfo_map_.empty()) ||
        PFC_EXPECT_TRUE(driverinfo_map_.find(driver_type) ==
                        driverinfo_map_.end())) {
      pfc_log_debug("Insert driver_id:%d to driverinfo_map_", driver_type);
      driverinfo_map_.insert(std::pair<unc_keytype_ctrtype_t,
                             ControllerList>(driver_type, controllers));

    } else {
      /*appending info to existing drivertype*/
      TcDriverInfoMap::iterator it = driverinfo_map_.find(driver_type);
      if (PFC_EXPECT_TRUE(it != driverinfo_map_.end())) {
        temp_list = it->second;
        /*prevent duplicate entries*/
        temp_list.sort();
        controllers.sort();
        controllers.merge(temp_list);
        controllers.unique();
        driverinfo_map_.erase(driver_type);
        pfc_log_debug("overwrite driver_id:%d to driverinfo_map_", driver_type);
        driverinfo_map_.insert(std::pair<unc_keytype_ctrtype_t,
                               ControllerList>(driver_type, controllers));
      }
    }
    controllers.clear();
  }
  pfc_log_debug("%s exit", __FUNCTION__);
  return TCOPER_RET_SUCCESS;
}

/*!\brief method to send vote/global commit to driver modules with controller
 *info collected from UPLL/UPPL
 *@param channelname - recipient channel name
 *@param clist - list of controllers corresponding to the driver module.
 *@param dummy_sess_ - dummy client session to collect driver response.
 *@result TcOperRet - TCOPER_RET_SUCCESS/TCOPER_RET_FAILURE
 **/
TcOperRet
TwoPhaseAudit::SendRequestToDriver() {
  pfc_log_debug("TwoPhaseAudit::SendRequestToDriver() entry");
  pfc_ipcresp_t resp = 0;
  std::string channel_name;
  TcUtilRet util_resp = TCUTIL_RET_SUCCESS;
  TcOperRet ret_val = TCOPER_RET_SUCCESS;
  tclib::TcMsgOperType oper = tclib::MSG_NONE;
  TcDriverInfoMap::iterator it;
  ControllerList clist;
  ControllerList::iterator cntrl_it;
  TcDaemonName tc_driverid;

  if (PFC_EXPECT_TRUE(opertype_ ==  tclib::MSG_AUDIT_VOTE)) {
    pfc_log_info("*** VOTE to Driver ***");
    oper = tclib::MSG_AUDIT_DRIVER_VOTE;
  } else if (PFC_EXPECT_TRUE(opertype_ == tclib::MSG_AUDIT_GLOBAL)) {
    pfc_log_info("*** GLOBAL COMMIT to Driver ***");
    oper = tclib::MSG_AUDIT_DRIVER_GLOBAL;
  }

  if (TcMsg::GetAuditCancelFlag() == PFC_TRUE &&
      opertype_ == tclib::MSG_AUDIT_VOTE &&
      audit_type_ != TC_AUDIT_REALNETWORK) {
    pfc_log_warn("%s Audit Cancelled during vote", __FUNCTION__);
    return TCOPER_RET_AUDIT_CANCELLED;
  }

  /*create UPLL/UPPL clientsessions to forward driver result*/
  ret_val = CreateSessionsToForwardDriverResult();
  if (PFC_EXPECT_TRUE(ret_val != TCOPER_RET_SUCCESS)) {
    pfc_log_error("CreateSessionsToForwardDriverResult failed");
    return TCOPER_RET_FAILURE;
  }
  /*send controller info to driver modules*/
  for (it = driverinfo_map_.begin(); it != driverinfo_map_.end(); it++) {
    /*map the driver type and get the channel name*/
    tc_driverid = MapTcDriverId((*it).first);
    if (tc_driverid != TC_NONE) {
      channel_name = channel_names_.find(tc_driverid)->second;
      pfc_log_debug("tc_driverid:%d channel_name:%s",
                   tc_driverid, channel_name.c_str());
    } else {
      pfc_log_error("Driver daemon %d does not exist", (*it).first);
      return TCOPER_RET_FATAL;
    }
    PFC_ASSERT(TCOPER_RET_SUCCESS == channel_name.empty());

    /*Create session for the given module name and service id*/
    sess_ = tc::TcClientSessionUtils::create_tc_client_session(channel_name,
                                      tclib::TCLIB_AUDIT_DRV_VOTE_GLOBAL,
                                      conn_);
    if (NULL == sess_) {
      return TCOPER_RET_FATAL;
    }
    /*append data to channel */
    util_resp = tc::TcClientSessionUtils::set_uint8(sess_, oper);
    if (PFC_EXPECT_TRUE(util_resp != TCUTIL_RET_SUCCESS)) {
      pfc_log_error("SendRequestToDriver: Setting oper failed");
      return ReturnUtilResp(util_resp);
    }
    util_resp = tc::TcClientSessionUtils::set_uint32(sess_, session_id_);
    if (PFC_EXPECT_TRUE(util_resp != TCUTIL_RET_SUCCESS)) {
      pfc_log_error("SendRequestToDriver: Setting sess_id failed");
      return ReturnUtilResp(util_resp);
    }
    util_resp = tc::TcClientSessionUtils::set_string(sess_, controller_id_);
    if (PFC_EXPECT_TRUE(util_resp != TCUTIL_RET_SUCCESS)) {
      pfc_log_error("SendRequestToDriver: Setting driver_id failed");
      return ReturnUtilResp(util_resp);
    }
    /*add controller info*/
    clist = (*it).second;
    uint8_t controller_count = clist.size();
    util_resp = tc::TcClientSessionUtils::set_uint8(sess_, controller_count);
    if (PFC_EXPECT_TRUE(util_resp != TCUTIL_RET_SUCCESS)) {
      pfc_log_error("SendRequestToDriver: Setting ctrl_count failed");
      return ReturnUtilResp(util_resp);
    }

    for (cntrl_it = clist.begin(); cntrl_it != clist.end(); cntrl_it++) {
      util_resp = tc::TcClientSessionUtils::set_string(sess_, *cntrl_it);
      if (PFC_EXPECT_TRUE(util_resp != TCUTIL_RET_SUCCESS)) {
        pfc_log_error("SendRequestToDriver: Setting ctrl_id failed");
        return ReturnUtilResp(util_resp);
      }
    }

    // Check if audit is cancelled at this time
    if (TcMsg::GetAuditCancelFlag() == PFC_TRUE &&
            audit_type_ != TC_AUDIT_REALNETWORK &&
        opertype_ == tclib::MSG_AUDIT_VOTE) {
      pfc_log_warn("%s Audit Cancelled during invoke", __FUNCTION__);
      return TCOPER_RET_AUDIT_CANCELLED;
    }

    pfc_log_info("notify %s - controller_count:%d",
                 channel_name.c_str(), controller_count);
    /*Invoke the session */
    util_resp = tc::TcClientSessionUtils::tc_session_invoke(sess_, resp);
    if (PFC_EXPECT_TRUE(util_resp != TCUTIL_RET_SUCCESS)) {
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
          pfc_log_error("%s forward to upll failed; ipc_ret = %d",
                        __FUNCTION__, ipc_ret);
          return TCOPER_RET_FATAL;
        } else if (ipc_ret != TCOPER_RET_SUCCESS) {
          pfc_log_fatal("%s forward to upll failed; ipc_ret = %d",
                        __FUNCTION__, ipc_ret);
          return TCOPER_RET_FATAL;
        }
      }
      dmndrvinfo.clear();
      /*Controller list of UPLL is considered since
       *controller list of UPPL is always empty during VOTE/COMMIT phase*/
      dmndrvinfo = driverset_map_[TC_UPLL];
      /*validate the driver_id saved from UPPL controllerinfo*/
      if (PFC_EXPECT_TRUE(dmndrvinfo.find((*it).first) != dmndrvinfo.end())) {
        pfc_log_debug("forward response to UPPL session");
        int32_t ipc_ret = uppl_sess_->forward(*sess_, 0, UINT32_MAX);
        if (ipc_ret == ESHUTDOWN ||
            ipc_ret == ECANCELED ||
            ipc_ret == ECONNABORTED) {
          pfc_log_error("%s forward to uppl failed; ipc_ret = %d",
                        __FUNCTION__, ipc_ret);
          return TCOPER_RET_FATAL;
        } else if (ipc_ret != TCOPER_RET_SUCCESS) {
          pfc_log_fatal("%s forward to uppl failed; ipc_ret = %d",
                        __FUNCTION__, ipc_ret);
          return TCOPER_RET_FATAL;
        }
      }
      /*append channelname to handle failure*/
      abort_on_fail_.push_back(tc_driverid);
      ret_val = TCOPER_RET_SUCCESS;
    } else if (resp == tclib::TC_CANCELLED_AUDIT &&
               opertype_ ==  tclib::MSG_AUDIT_VOTE) {
      TcOperations::SetAuditPhase(AUDIT_TRANSACTION_END);
      pfc_log_info("%s TC_CANCELLED_AUDIT resp received from %s",
                   __FUNCTION__, channel_name.c_str());
      trans_result_ = tclib::TRANS_END_FAILURE;
      audit_result_ = tclib::TC_AUDIT_CANCELLED;

      abort_on_fail_.clear();
      driverinfo_map_.clear();

      ret_val = TCOPER_RET_AUDIT_CANCELLED;
  
    } else if (PFC_EXPECT_TRUE(tclib::TC_FAILURE == resp) &&
               PFC_EXPECT_TRUE(opertype_ ==  tclib::MSG_AUDIT_VOTE)) {
      TcOperations::SetAuditPhase(AUDIT_TRANSACTION_END);
      pfc_log_info("Failure response from %s", channel_name.c_str());
      trans_result_ = tclib::TRANS_END_FAILURE;
      audit_result_ = tclib::TC_AUDIT_FAILURE;

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
               PFC_EXPECT_TRUE(opertype_ ==  tclib::MSG_AUDIT_GLOBAL)) {
      pfc_log_error("Failure response from %s", channel_name.c_str());
      audit_result_ = tclib::TC_AUDIT_FAILURE;
      ret_val = TCOPER_RET_FATAL;
    }
    if (ret_val == TCOPER_RET_SUCCESS) {
      TcClientSessionUtils::tc_session_close(&sess_, conn_);
    } else {
      return ret_val;
    }
  }

  // Check if audit is cancelled at this time
  if (TcMsg::GetAuditCancelFlag() == PFC_TRUE &&
          audit_type_ != TC_AUDIT_REALNETWORK &&
      opertype_ == tclib::MSG_AUDIT_VOTE) {
    pfc_log_warn("%s Audit Cancelled after invoke", __FUNCTION__);
    return TCOPER_RET_AUDIT_CANCELLED;
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

  pfc_log_debug("TwoPhaseAudit::SendRequestToDriver exit");
  return ret_val;
}

/*!\brief this method sends VOTE/GLOBAL AUDIT
 *request of commit operation to UPLL/UPPL/DRIVER modules.
 *@result TcOperRet - TCOPER_RET_SUCCESS/TCOPER_RET_FAILURE/TCOPER_RET_ABORT
 **/
TcOperRet
TwoPhaseAudit::SendRequest(std::string channel_name) {
  pfc_log_debug("TwoPhaseAudit::SendRequest() entry");
  uint32_t respcount = 0;
  pfc_ipcresp_t resp = 0;
  uint8_t audit_ret = 0;
  TcUtilRet util_resp = TCUTIL_RET_SUCCESS;
  TcOperRet ret_val = TCOPER_RET_SUCCESS;

  PFC_ASSERT(TCOPER_RET_SUCCESS == channel_name.empty());
  /*Create session for the given module name and service id*/
  sess_ = tc::TcClientSessionUtils::create_tc_client_session(channel_name,
                                    tclib::TCLIB_AUDIT_TRANSACTION, conn_);
  if (NULL == sess_) {
    return TCOPER_RET_FATAL;
  }
  /*append data to channel */
  util_resp = tc::TcClientSessionUtils::set_uint8(sess_, opertype_);
  if (PFC_EXPECT_TRUE(util_resp != TCUTIL_RET_SUCCESS)) {
    pfc_log_error("SendRequest: Setting opertype failed");
    return ReturnUtilResp(util_resp);
  }
  util_resp = tc::TcClientSessionUtils::set_uint32(sess_, session_id_);
  if (PFC_EXPECT_TRUE(util_resp != TCUTIL_RET_SUCCESS)) {
    pfc_log_error("SendRequest: Setting sess_id failed");
    return ReturnUtilResp(util_resp);
  }
  util_resp = tc::TcClientSessionUtils::set_uint8(sess_, driver_id_);
  if (PFC_EXPECT_TRUE(util_resp != TCUTIL_RET_SUCCESS)) {
    pfc_log_error("SendRequest: Setting drv_id failed");
    return ReturnUtilResp(util_resp);
  }
  util_resp = tc::TcClientSessionUtils::set_string(sess_, controller_id_);
  if (PFC_EXPECT_TRUE(util_resp != TCUTIL_RET_SUCCESS)) {
    pfc_log_error("SendRequest: Setting ctrl_id failed");
    return ReturnUtilResp(util_resp);
  }
  pfc_log_info("notify %s - cntrl_id:%s",
                channel_name.c_str(), controller_id_.c_str());
  /*Invoke the session */
  util_resp = tc::TcClientSessionUtils::tc_session_invoke(sess_, resp);
  if (PFC_EXPECT_TRUE(util_resp != TCUTIL_RET_SUCCESS)) {
    pfc_log_error("SendRequest: Session invoke failed");
    return ReturnUtilResp(util_resp);
  }

  respcount = sess_->getResponseCount();
  /*retrieve audit result*/
  if (PFC_EXPECT_TRUE(opertype_ == tclib::MSG_AUDIT_GLOBAL) &&
      PFC_EXPECT_TRUE(resp == tclib::TC_SUCCESS)) {
    util_resp = tc::TcClientSessionUtils::get_uint8(sess_, respcount-1,
                                                    &audit_ret);
    if (util_resp != TCUTIL_RET_SUCCESS) {
      return ReturnUtilResp(util_resp);
    }
    audit_result_ = (tclib::TcAuditResult)(audit_result_ && audit_ret);
    pfc_log_info("audit_result:%d", audit_result_);
    ret_val = TCOPER_RET_SUCCESS;
  }

  /*handle server response*/
  if (PFC_EXPECT_TRUE(resp == tclib::TC_SUCCESS) &&
      PFC_EXPECT_TRUE(respcount > 1)) {
    /*append the driver info from log/phy in TcControllerInfoMap
     *            * driverinfo_map_*/
    if (TCOPER_RET_SUCCESS != GetControllerInfo(sess_)) {
      pfc_log_error("could not retrieve controller-info");
      return TCOPER_RET_FAILURE;
    }
    ret_val = TCOPER_RET_SUCCESS;
  } else if (resp == tclib::TC_CANCELLED_AUDIT &&
             opertype_ == tclib::MSG_AUDIT_VOTE) {
    TcOperations::SetAuditPhase(AUDIT_TRANSACTION_END);
    pfc_log_info("%s Recived TC_CANCELLED_AUDIT from %s",
                 __FUNCTION__, channel_name.c_str());
    trans_result_ = tclib::TRANS_END_FAILURE;
    audit_result_ = tclib::TC_AUDIT_CANCELLED;

    abort_on_fail_.clear();

    ret_val = TCOPER_RET_AUDIT_CANCELLED;

  } else if (PFC_EXPECT_TRUE(resp == tclib::TC_FAILURE) &&
             PFC_EXPECT_TRUE(opertype_ == tclib::MSG_AUDIT_VOTE)) {
    TcOperations::SetAuditPhase(AUDIT_TRANSACTION_END);
    pfc_log_info("Failure response from %s",
                 channel_name.c_str() );
    /*transamit transaction end result in trans-end notification*/
    trans_result_ = tclib::TRANS_END_FAILURE;
    audit_result_ = tclib::TC_AUDIT_FAILURE;
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
             PFC_EXPECT_TRUE(opertype_ == tclib::MSG_AUDIT_GLOBAL)) {
    pfc_log_error("Failure response from %s",
                  channel_name.c_str());
    audit_result_ = tclib::TC_AUDIT_FAILURE;
    ret_val = TCOPER_RET_FATAL;
  }
  /*delete the session pointer*/
  if (ret_val == TCOPER_RET_SUCCESS)  {
    pfc_log_info("success response from %s", channel_name.c_str());
    TcClientSessionUtils::tc_session_close(&sess_, conn_);
  }
  pfc_log_debug("TwoPhaseAudit::SendRequest() exit");
  return ret_val;
}

/*!\brief TC service handler invokes this method to send VOTE/GLOBAL AUDIT
 *request of commit operation to UPLL/UPPL/DRIVER modules.
 *@result TcOperRet - TCOPER_RET_SUCCESS/TCOPER_RET_FAILURE/TCOPER_RET_ABORT
 ***/
TcOperRet TwoPhaseAudit::Execute() {
  pfc_log_debug("TwoPhaseAudit::Execute() entry");
  TcOperRet ret_val = TCOPER_RET_SUCCESS;
  std::string channelname, channel_name;
  notifyorder_.clear();

  /*channel_names_ map should not be empty */
  PFC_ASSERT(TCOPER_RET_SUCCESS == channel_names_.empty());

  /*validate controller_id_ and driver_id_*/
  if ((PFC_EXPECT_TRUE(driver_id_ < UNC_CT_PFC) ||
       PFC_EXPECT_TRUE(driver_id_ > UNC_CT_ODC)) ||
      PFC_EXPECT_TRUE(controller_id_.empty())) {
    pfc_log_error("Invalid Driver/Controller ID");
    return TCOPER_RET_FAILURE;
  }

  if (opertype_ == tclib::MSG_AUDIT_VOTE) {
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

    if (TcMsg::GetAuditCancelFlag() == PFC_TRUE &&
            audit_type_ != TC_AUDIT_REALNETWORK &&
        opertype_ == tclib::MSG_AUDIT_VOTE) {
      pfc_log_warn("%s AuditCancel flag is set. Stopping Audit process",
                   __FUNCTION__);
      ret_val = TCOPER_RET_AUDIT_CANCELLED;
      break;
    }

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

  if (TcMsg::GetAuditCancelFlag() == PFC_TRUE &&
          audit_type_ != TC_AUDIT_REALNETWORK &&
      opertype_ == tclib::MSG_AUDIT_VOTE) {
    return TCOPER_RET_AUDIT_CANCELLED;
  }

  /*user might have given 'commit' without any config changes - ignore*/
  if (PFC_EXPECT_TRUE(ret_val == TCOPER_RET_SUCCESS) &&
      PFC_EXPECT_TRUE(driverinfo_map_.empty())) {
    pfc_log_info("Controller info from UPPL,UPLL is empty");
    pfc_log_info(" send dummy driver result to UPLL/UPPL");
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

  pfc_log_debug("TwoPhaseAudit::Execute() exit");
  return ret_val;
}


}  // namespace tc
}  // namespace unc
