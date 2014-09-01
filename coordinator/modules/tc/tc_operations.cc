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
      tc_oper_status_(INPUT_VALIDATION) {}



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
  pfc_log_info("Received oper_type:%d", tc_oper_);
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

  pfc_log_info("SessionId: %u", session_id_);

  return TC_OPER_SUCCESS;
}

/*
 * @brief Set the Result Opertype to the session
 */
TcOperStatus TcOperations::SetOperType() {
  uint32_t oper_type = tc_oper_;
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
  pfc_log_info("SetOperStatus: resp:%d", resp_status);
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
        if (tclock_->GetUncCurrentState() == TC_STOP) {
          pfc_log_error("Revoke Operation failed in Release Lock");
        } else {
          pfc_log_fatal("Revoke Operation failed in Release Lock");
        }
        return SendResponse(TC_SYSTEM_FAILURE);
      }

    case RELEASE_EXCLUSION_PHASE:
      if (tclock_->GetUncCurrentState() == TC_STOP) {
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

  pfc_log_info("tc_oper:Read Input Paramaters");
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

  pfc_log_info("tc_oper:Accumulate Message List");
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
  pfc_log_info("tc_oper:Send Response to user");
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
      if (*MsgIter == unc::tclib::MSG_AUDITDB) {
        /*set when Audit DB fails in startup phase*/
        audit_db_fail_ = PFC_TRUE;
      }
      return HandleMsgRet(MsgRet);
    }

    if (*MsgIter == unc::tclib::MSG_SETUP_COMPLETE) {
      tclock_->TcSetSetupComplete(PFC_TRUE);
      pfc_log_info("MSG_SETUP_COMPLETE completed");
    }

    MsgIter++;
    delete tcmsg_;
  }
  return TC_OPER_SUCCESS;
}
}  // namespace tc
}  // namespace unc
