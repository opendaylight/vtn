/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */


#include <tc_operations.hh>

#define UNC_NO_ARGS 0

namespace unc {
namespace tc {

/*
 * @brief Start Up Operation Constructor
 */
TcStartUpOperations::TcStartUpOperations(TcLock* tclock,
                                         pfc::core::ipc::ServerSession* sess,
                                         TcDbHandler* tc_db_,
                                         TcChannelNameMap& unc_map_,
                                         pfc_bool_t is_switch)
  : TcOperations(tclock, sess, tc_db_, unc_map_), is_switch_(is_switch),
    fail_oper_(TC_OP_INVALID), database_type_(UNC_DT_INVALID) {}


/*
 * @brief return Argument count
 */
uint32_t TcStartUpOperations::TcGetMinArgCount() {
  return UNC_NO_ARGS;
}

/*
 * @brief Get Arguments for operation
 */
TcOperStatus
  TcStartUpOperations::HandleArgs() {
    if ( is_switch_ == PFC_FALSE ) {
      return TC_OPER_SUCCESS;
    }
  TcOperRet ret = db_hdlr_->GetRecoveryTable(&database_type_,
                                            &fail_oper_);
  if ( ret != TCOPER_RET_SUCCESS ) {
    return TC_OPER_FAILURE;
  }
  return TC_OPER_SUCCESS;
}

/*
 * @brief Check argument count in input
 */
TcOperStatus
  TcStartUpOperations::TcCheckOperArgCount(uint32_t avail_count) {
  return TC_OPER_SUCCESS;
}

/*
 * @brief Is Operation Type Valid
 */
TcOperStatus
  TcStartUpOperations::TcValidateOperType() {
  return TC_OPER_SUCCESS;
}

/*
 * @brief Is Operation Params Valid
 */
TcOperStatus
  TcStartUpOperations::TcValidateOperParams() {
  return TC_OPER_SUCCESS;
}

/*
 * @brief Secure Exclusion
 */
TcOperStatus
  TcStartUpOperations::TcGetExclusion() {
  return TC_OPER_SUCCESS;
}

/*
 * @brief Release Exclusion
 */
TcOperStatus
  TcStartUpOperations::TcReleaseExclusion() {
  return TC_OPER_SUCCESS;
}

/*
 * @brief Return from TcLock
 */
TcOperStatus
  TcStartUpOperations::HandleLockRet(TcLockRet ret) {
  return TC_OPER_SUCCESS;
}

/*
 * @brief List of Messages processed for this operation
 */
TcOperStatus
  TcStartUpOperations::TcCreateMsgList() {
  if (TcOperMessageList.empty()) {
    if (is_switch_ == PFC_FALSE) {
      TcOperMessageList.push_back(unc::tclib::MSG_SETUP);
    } else {
      if (database_type_ != UNC_DT_INVALID &&
         fail_oper_ != TC_OP_INVALID) {
        TcOperMessageList.push_back(unc::tclib::MSG_AUDITDB);
      }
    }
    TcOperMessageList.push_back(unc::tclib::MSG_SETUP_COMPLETE);
    return TC_OPER_SUCCESS;
  }
  return TC_OPER_FAILURE;
}

/*
 * @brief Contents filled for every message
 */
TcOperStatus
  TcStartUpOperations::FillTcMsgData(TcMsg* tc_msg,
                           TcMsgOperType oper_type) {
  if ( tc_msg == NULL ) {
    return TC_OPER_FAILURE;
  }
  if (oper_type == unc::tclib::MSG_SETUP_COMPLETE) {
    tclock_->TcUpdateUncState(TC_ACT);
  }
  tc_msg->SetData(database_type_, fail_oper_);
  return TC_OPER_SUCCESS;
}

/*
 * @brief Send Response
 */
TcOperStatus
  TcStartUpOperations::SendResponse(TcOperStatus ret_) {
  if (ret_ == TC_OPER_SUCCESS) {
    if ( is_switch_ == PFC_TRUE ) {
      TcOperRet ret= db_hdlr_->UpdateRecoveryTable(UNC_DT_INVALID,
                                                   TC_OP_INVALID);
      if ( ret != TCOPER_RET_SUCCESS )
        return TC_SYSTEM_FAILURE;
    }
  }
  return ret_;
}

/*
 * @brief Additional Response for the operation
 */
TcOperStatus
  TcStartUpOperations::SendAdditionalResponse(TcOperStatus oper_stat) {
    return oper_stat;
}

}  // namespace tc
}  // namespace unc
