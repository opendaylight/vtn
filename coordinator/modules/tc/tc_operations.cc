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

TcAuditCancelStatus TcOperations::audit_cancel_status_ = AUDIT_CANCEL_DONE;
pfc::core::Mutex TcOperations::audit_cancel_status_mutex_;

TcAuditPhase TcOperations::audit_phase_ = AUDIT_NOT_STARTED;
pfc::core::Mutex TcOperations::audit_phase_mutex_;

std::vector<TcDaemonName> TcOperations::audit_cancel_notify_;
pfc::core::Mutex TcOperations::audit_cancel_notify_lock_;

pfc::core::Mutex TcOperations::candidate_audit_excl_lock_;

pfc_bool_t TcOperations::state_changed_to_sby_ = PFC_FALSE;
pfc::core::Mutex TcOperations::state_changed_lock_;

/*
 * @brief Constructor of TcOperations
 */
TcOperations::TcOperations(TcLock* tc_lock_,
                           pfc::core::ipc::ServerSession* sess_,
                           TcDbHandler* db_handler,
                           TcChannelNameMap& unc_map_)
    : ssess_(sess_),
      db_hdlr_(db_handler),
      unc_oper_channel_map_(unc_map_),
      tclock_(tc_lock_),
      session_id_(0),
      tc_oper_(TC_OP_INVALID),
      tc_oper_status_(INPUT_VALIDATION),
      audit_db_fail_(PFC_FALSE),
      csess_(NULL),
      cconn_(0) {}


/*
 * @brief Desctructor of TcOperations
 */
TcOperations::~TcOperations() {
  if ( db_hdlr_ != NULL )
    delete db_hdlr_;

  TcUtilRet ret = TCUTIL_RET_SUCCESS;
  if (csess_) {
    ret = TcClientSessionUtils::tc_session_close(&csess_, cconn_);
    if (ret != TCUTIL_RET_SUCCESS) {
      pfc_log_fatal("%s pfc_ipcclnt_altclose of csess_ failed", __FUNCTION__);
    }
  }
}
/*
 * @brief Get the Operation Type from the session
 */
TcOperStatus TcOperations::GetOperType() {
  uint32_t oper_type;
  TcUtilRet ret=
  TcServerSessionUtils::get_uint32(ssess_,
                                   TC_REQ_OP_TYPE_INDEX,
                                   &oper_type);
  if ( ret == TCUTIL_RET_FAILURE ) {
    pfc_log_error("GetOperType failed");
    return TC_OPER_INVALID_INPUT;
  } else if ( ret == TCUTIL_RET_FATAL ) {
    pfc_log_error("GetOperType fatal");
    return TC_OPER_FAILURE;
  }

  tc_oper_=(TcServiceType)oper_type;
  pfc_log_debug("Received oper_type:%d", tc_oper_);
  return TC_OPER_SUCCESS;
}


/*
 * @brief Get the Session ID from the session
 */
TcOperStatus TcOperations::GetSessionId() {
  TcUtilRet ret=
      TcServerSessionUtils::get_uint32(ssess_,
                                       TC_REQ_SESSION_ID_INDEX,
                                       &session_id_);
  if ( ret == TCUTIL_RET_FAILURE ) {
    pfc_log_error("GetSessionId failed");
    return TC_OPER_INVALID_INPUT;
  } else if ( ret == TCUTIL_RET_FATAL ) {
    pfc_log_error("GetSessionId fatal");
    return TC_OPER_FAILURE;
  }
  if ( session_id_ == 0 ) {
    pfc_log_error("GetSessionId - session_id(0) invalid");
    return TC_OPER_INVALID_INPUT;
  }

  pfc_log_info("Operation %s received from session[%u]",
               OperTypeToStr(tc_oper_).c_str(), session_id_);

  return TC_OPER_SUCCESS;
}

/*
 * @brief Set the Result Opertype to the session
 */
TcOperStatus TcOperations::SetOperType() {
  uint8_t oper_type = tc_oper_;
  TcUtilRet ret=
      TcServerSessionUtils::set_uint32(ssess_,
                                       oper_type);
  if ( ret == TCUTIL_RET_FAILURE ) {
    pfc_log_error("SetOperType failed");
    return TC_OPER_INVALID_INPUT;
  } else if ( ret == TCUTIL_RET_FATAL ) {
    pfc_log_error("SetOperType fatal");
    return TC_OPER_FAILURE;
  }
  return TC_OPER_SUCCESS;
}

/*
 * @brief Set the Result SessionId to the session
 */
TcOperStatus TcOperations::SetSessionId() {
  TcUtilRet ret=
      TcServerSessionUtils::set_uint32(ssess_,
                                       session_id_);
  if ( ret == TCUTIL_RET_FAILURE ) {
    pfc_log_error("SetSessionId  failed");
    return TC_OPER_INVALID_INPUT;
  } else if ( ret == TCUTIL_RET_FATAL ) {
    pfc_log_error("SetSessionId  fatal");
    return TC_OPER_FAILURE;
  }
  return TC_OPER_SUCCESS;
}

/*
 * @brief Set the Result of Operation to the session
 */
TcOperStatus TcOperations::SetOperStatus(TcOperStatus resp_status) {
  pfc_log_debug("SetOperStatus: resp:%d", resp_status);
  TcUtilRet ret=
      TcServerSessionUtils::set_uint32(ssess_,
                                       resp_status);
  if ( ret == TCUTIL_RET_FAILURE ) {
    pfc_log_error("SetOperStatus failed");
    return TC_OPER_INVALID_INPUT;
  } else if ( ret == TCUTIL_RET_FATAL ) {
    pfc_log_error("SetOperStatus fatal");
    return TC_OPER_FAILURE;
  }
  return TC_OPER_SUCCESS;
}

/*
 * @brief Method Collect/validate inputs from session
 *        will invokde the Get Methods
 */
TcOperStatus TcOperations::HandleArgs() {
  TcOperStatus ret;
  if ( ssess_ == NULL ) {
    ret = TcValidateOperType();
    if ( ret != TC_OPER_SUCCESS )
      return ret;
    return TC_OPER_SUCCESS;
  }
  uint32_t count = ssess_->getArgCount();
  if ( TcGetMinArgCount() &&
      count >= TcGetMinArgCount() ) {
    ret = GetOperType();
    if ( ret != TC_OPER_SUCCESS )
      return ret;
    ret = GetSessionId();
    if ( ret != TC_OPER_SUCCESS )
      return ret;
    ret = TcValidateOperType();
    if ( ret != TC_OPER_SUCCESS )
      return ret;
    ret = TcCheckOperArgCount(count);
    if ( ret != TC_OPER_SUCCESS )
      return ret;
    ret = TcValidateOperParams();
    if ( ret != TC_OPER_SUCCESS )
      return ret;
    return TC_OPER_SUCCESS;
  } else {
    return TC_OPER_INVALID_INPUT;
  }
}


/*
 * @brief  Method to write response to session
 *         Will invokde the Set Methods
 */
TcOperStatus TcOperations::SendResponse(TcOperStatus send_oper_status) {
  if ( ssess_ == NULL )
    return send_oper_status;

  pfc_log_info("send_oper_status value=%d", send_oper_status);
  if ( send_oper_status != TC_OPER_INVALID_INPUT  &&
       send_oper_status != TC_OPER_FAILURE ) {
    if (SetOperType() != TC_OPER_SUCCESS) {
      return TC_OPER_FAILURE;
    }
    if (SetSessionId() != TC_OPER_SUCCESS) {
      return TC_OPER_FAILURE;
    }
    if (SetOperStatus(send_oper_status) != TC_OPER_SUCCESS) {
      return TC_OPER_FAILURE;
    }
    return SendAdditionalResponse(send_oper_status);
  }
  return send_oper_status;
}

/*
 * @brief  Method to revoke in case of failure
 */
TcOperStatus TcOperations::RevokeOperation(TcOperStatus oper_status) {
  pfc_log_info("RevokeOperation Oper_status:%u", oper_status);
  switch ( tc_oper_status_ ) {
    case INPUT_VALIDATION:
      return SendResponse(oper_status);

    case GET_EXCLUSION_PHASE:
      return SendResponse(oper_status);

    case CREATE_MSG_LIST:
    case EXECUTE_PHASE:
      if ( TcReleaseExclusion() == TC_OPER_SUCCESS ) {
        return SendResponse(oper_status);
      } else {
        if (tclock_->GetUncCurrentState() != TC_ACT) {
          pfc_log_error("Revoke Operation failed in Release Lock");
        } else {
          pfc_log_fatal("Revoke Operation failed in Release Lock");
        }
        return SendResponse(TC_SYSTEM_FAILURE);
      }

    case RELEASE_EXCLUSION_PHASE:
      if (tclock_->GetUncCurrentState() != TC_ACT) {
        pfc_log_error("Revoke Operation failed in Release Lock");
      } else {
        pfc_log_fatal("Revoke Operation failed in Release Lock");
      }
      return SendResponse(TC_SYSTEM_FAILURE);

    default:
      return TC_OPER_FAILURE;
  }
}

/*
 * @brief  Method to trigger operation
 */
TcOperStatus TcOperations::Dispatch() {
  TcOperStatus ret;
  tc_oper_status_ = INPUT_VALIDATION;

  pfc_log_debug("tc_oper:Read Input Paramaters");
  ret = HandleArgs();
  if (ret != TC_OPER_SUCCESS) {
    return RevokeOperation(ret);
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
 * @brief  Method to map TcMsg return types
 */
TcOperStatus TcOperations::HandleMsgRet(TcOperRet MsgRet) {
  TcOperStatus oper_status = TC_OPER_SUCCESS;

  if ( MsgRet == TCOPER_RET_FATAL ||
       MsgRet == TCOPER_RET_FAILURE ) {
    oper_status = TC_SYSTEM_FAILURE;
  } else  if ( MsgRet == TCOPER_RET_ABORT ) {
    oper_status = TC_OPER_ABORT;
  } else if ( MsgRet == TCOPER_RET_NO_DRIVER) {
    oper_status = TC_OPER_DRIVER_NOT_PRESENT;
  } else if ( MsgRet == TCOPER_RET_AUDIT_CANCELLED) {
    oper_status = TC_OPER_AUDIT_CANCELLED;
  } else if ( MsgRet == TCOPER_RET_LAST_DB_OP_FAILED) {
    oper_status = TC_OPER_SUCCESS;
  }

  pfc_log_info("HandleMsgRet: Received(%u), returning (%u)",
               MsgRet, oper_status);
  return oper_status;
}


/*
 * @brief  Processing operation messages
 */
TcOperStatus TcOperations::Execute() {
  if ( TcOperMessageList.size() == 0 )
    return TC_OPER_FAILURE;
  std::list<TcMsgOperType>::iterator MsgIter = TcOperMessageList.begin();
  while ( MsgIter != TcOperMessageList.end() ) {
    TcMsg *tcmsg_=
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
    if ( MsgRet != TCOPER_RET_SUCCESS ) {
      delete tcmsg_;
      tcmsg_ = NULL;
      return HandleMsgRet(MsgRet);
    }

    MsgIter++;
    delete tcmsg_;
  }
  return TC_OPER_SUCCESS;
}

/* Set and Get Methods for Audit_Phase for cancel Audit */
void TcOperations::SetAuditPhase(TcAuditPhase phase) {
  pfc::core::ScopedMutex m(audit_phase_mutex_);
  audit_phase_ = phase;
}

TcAuditPhase TcOperations::GetAuditPhase() {
  pfc::core::ScopedMutex m(audit_phase_mutex_);
  return audit_phase_;
}

void TcOperations::SetAuditCancelStatus(TcAuditCancelStatus status) {
  pfc::core::ScopedMutex m(audit_cancel_status_mutex_);
  audit_cancel_status_ = status;
}

TcAuditCancelStatus TcOperations::GetAuditCancelStatus() {
  pfc::core::ScopedMutex m(audit_cancel_status_mutex_);
  return audit_cancel_status_;
}

pfc_bool_t TcOperations::IsStateChangedToSby() {
  pfc::core::ScopedMutex m(state_changed_lock_);
  return state_changed_to_sby_;
}

void TcOperations::SetStateChangedToSby(pfc_bool_t state) {
  pfc::core::ScopedMutex m(state_changed_lock_);
  state_changed_to_sby_ = state;
}

pfc_bool_t TcOperations::IsCandidateDirty(uint32_t session_id,
                                          uint32_t config_id) {
  pfc_log_debug("%s entry", __FUNCTION__);
  
  pfc_bool_t is_candidate_dirty = PFC_FALSE;

  pfc_ipcresp_t resp = 0;
  TcUtilRet util_resp = TCUTIL_RET_SUCCESS;

  std::string channel_name;
  uint32_t service_id = 0;
  uint32_t oper_type = 0;

  NotifyList notify_order;
  notify_order.push_back(TC_UPLL);
  notify_order.push_back(TC_UPPL);

  for (NotifyList::iterator list_iter = notify_order.begin();
       list_iter != notify_order.end();
       list_iter++) {
    TcChannelNameMap::iterator it = unc_oper_channel_map_.find(*list_iter);
    if (it != unc_oper_channel_map_.end()) {
      channel_name = it->second;
    }
    if (channel_name.empty()) {
      pfc_log_error("%s channel_name is empty for %d",__FUNCTION__, *list_iter);
      return PFC_FALSE;
    }

    if (*list_iter == TC_UPLL) {
      service_id = UPLL_GLOBAL_CONFIG_SVC_ID;
      oper_type = UPLL_IS_CANDIDATE_DIRTY_OP;
    } else if (*list_iter == TC_UPPL) {
      service_id = UPPL_SVC_GLOBAL_CONFIG;
      oper_type = UNC_OP_IS_CANDIDATE_DIRTY;
    }

    csess_ = TcClientSessionUtils::create_tc_client_session(channel_name,
                                                            service_id,
                                                            cconn_,
                                                            PFC_TRUE,
                                                            PFC_FALSE);
    if (csess_ == NULL) {
      pfc_log_fatal("%s ClientSession is NULL", __FUNCTION__);
      return PFC_FALSE;
    }

    // Set oper_type
    util_resp = TcClientSessionUtils::set_uint32(csess_, oper_type);
    if (util_resp != TCUTIL_RET_SUCCESS) {
      pfc_log_error("%s Failed to set oper_type in csess_", __FUNCTION__);
      return PFC_FALSE;
    }

    // Set session_id
    util_resp = TcClientSessionUtils::set_uint32(csess_, session_id);
    if (util_resp != TCUTIL_RET_SUCCESS) {
      pfc_log_error("%s Failed to set session_id in csess_", __FUNCTION__);
      return PFC_FALSE;
    }

    // Set config_id
    util_resp = TcClientSessionUtils::set_uint32(csess_, config_id);
    if (util_resp != TCUTIL_RET_SUCCESS) {
      pfc_log_error("%s Failed to set config_id in csess_", __FUNCTION__);
      return PFC_FALSE;
    }

    // Send request
    util_resp = TcClientSessionUtils::tc_session_invoke(csess_, resp);
    if (util_resp != TCUTIL_RET_SUCCESS) {
      pfc_log_error("%s Failed to invoke session", __FUNCTION__);
      return PFC_FALSE;
    }

    if (resp != TCUTIL_RET_SUCCESS) {
      pfc_log_error("%s Failure response from %s",
                    __FUNCTION__, channel_name.c_str());
      return PFC_FALSE;
    } else {
      pfc_log_info("%s Success response from %s",
                   __FUNCTION__, channel_name.c_str());
    }

    uint32_t recv_oper = 0;
    uint32_t idx = 0;

    util_resp = TcClientSessionUtils::get_uint32(csess_, idx, &recv_oper);
    if (util_resp != TCUTIL_RET_SUCCESS) {
      pfc_log_error("%s Failed to fetch oper_type", __FUNCTION__);
      return PFC_FALSE;
    }

    if (recv_oper != oper_type) {
      pfc_log_error("%s Incorrect opertype. Recv[%u] Expected[%u]",
                    __FUNCTION__, recv_oper, oper_type);
      return PFC_FALSE;
    }

    uint32_t result_code = 0;
    ++idx;
    util_resp = TcClientSessionUtils::get_uint32(csess_, idx, &result_code);
    if (util_resp != TCUTIL_RET_SUCCESS) {
      pfc_log_error("%s Failed to fetch result_code", __FUNCTION__);
      return PFC_FALSE;
    }

    if (result_code != UNC_RC_SUCCESS) {
      pfc_log_error("%s Received failure response code", __FUNCTION__);
      return PFC_FALSE;
    }

    uint8_t is_dirty = 0;
    ++idx;
    util_resp = TcClientSessionUtils::get_uint8(csess_, idx, &is_dirty);
    if (util_resp != TCUTIL_RET_SUCCESS) {
      pfc_log_error("%s Failed to fetch IsDirty flag", __FUNCTION__);
      return PFC_FALSE;
    }

    TcClientSessionUtils::tc_session_close(&csess_, cconn_); 

    pfc_log_info("%s Received IsCandidateDirty %s",
                 __FUNCTION__, (is_dirty ? "TRUE":"FALSE"));
    is_candidate_dirty = (pfc_bool_t) is_dirty;

    if (is_candidate_dirty == PFC_TRUE) {
      // If Candidate dirty in one module, no need to send to next
      break;
    }
  }

  return is_candidate_dirty;
}

std::string TcOperations::OperTypeToStr(TcServiceType op_type) {
  std::string str = "UNKNOWN";
  switch(op_type) {
    case TC_OP_CONFIG_ACQUIRE:
      str = "TC_OP_CONFIG_ACQUIRE";
      break;
    case TC_OP_CONFIG_RELEASE:
      str = "TC_OP_CONFIG_RELEASE";
      break;
    case TC_OP_CONFIG_ACQUIRE_TIMED:
      str = "TC_OP_CONFIG_ACQUIRE_TIMED";
      break;
    case TC_OP_CONFIG_ACQUIRE_PARTIAL:
      str = "TC_OP_CONFIG_ACQUIRE_PARTIAL";
      break;
    case TC_OP_CONFIG_ACQUIRE_FORCE:
      str = "TC_OP_CONFIG_ACQUIRE_FORCE";
      break;
    case TC_OP_CANDIDATE_COMMIT:
      str = "TC_OP_CANDIDATE_COMMIT";
      break;
    case TC_OP_CANDIDATE_COMMIT_TIMED:
      str = "TC_OP_CANDIDATE_COMMIT_TIMED";
      break;
    case TC_OP_CANDIDATE_ABORT:
      str = "TC_OP_CANDIDATE_ABORT";
      break;
    case TC_OP_CANDIDATE_ABORT_TIMED:
      str = "TC_OP_CANDIDATE_ABORT_TIMED";
      break;
    case TC_OP_RUNNING_SAVE:
      str = "TC_OP_RUNNING_SAVE";
      break;
    case TC_OP_CLEAR_STARTUP:
      str = "TC_OP_CLEAR_STARTUP";
      break;
    case TC_OP_READ_ACQUIRE:
      str = "TC_OP_READ_ACQUIRE";
      break;
    case TC_OP_READ_RELEASE:
      str = "TC_OP_READ_RELEASE";
      break;
    case TC_OP_READ_RUNNING_STATUS:
      str = "TC_OP_READ_RUNNING_STATUS";
      break;
    case TC_OP_READ_STARTUP_STATUS:
      str = "TC_OP_READ_STARTUP_STATUS";
      break;
    case TC_OP_AUTOSAVE_GET:
      str = "TC_OP_AUTOSAVE_GET";
      break;
    case TC_OP_AUTOSAVE_ENABLE:
      str = "TC_OP_AUTOSAVE_ENABLE";
      break;
    case TC_OP_AUTOSAVE_DISABLE:
      str = "TC_OP_AUTOSAVE_DISABLE";
      break;
    case TC_OP_USER_AUDIT:
      str = "TC_OP_USER_AUDIT";
      break;
    case TC_OP_DRIVER_AUDIT:
      str = "TC_OP_DRIVER_AUDIT";
      break;
    case TC_OP_INVALID:
      str = "TC_OP_INVALID";
      break;
    default:
      str = "UNKNOWN";
  }
  return str;
}

}  // namespace tc
}  // namespace unc
