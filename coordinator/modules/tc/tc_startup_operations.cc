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
  uint32_t failover_instance = 0;
  TcOperRet ret = db_hdlr_->GetRecoveryTable(&database_type_,
                                            &fail_oper_,
                                            &failover_instance);
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

  pfc_bool_t autosave_enabled = PFC_FALSE;
  TcOperRet ret = db_hdlr_->GetConfTable(&autosave_enabled);
  if ( ret == TCOPER_RET_FAILURE ) {
    pfc_log_info("Database Read Failed for getting autosave");
    return TC_OPER_FAILURE;
  }
  tc_msg->SetData(autosave_enabled);

  if (oper_type == unc::tclib::MSG_AUDITDB) {
    tc_msg->SetData(database_type_, fail_oper_);
  }

  return TC_OPER_SUCCESS;
}

/*
 * @brief Send Response
 */
TcOperStatus
  TcStartUpOperations::SendResponse(TcOperStatus ret_) {
  if (ret_ == TC_OPER_SUCCESS) {
    TcOperRet ret= db_hdlr_->UpdateRecoveryTable(UNC_DT_INVALID,
                                                 TC_OP_INVALID);
    if ( ret != TCOPER_RET_SUCCESS )
      return TC_SYSTEM_FAILURE;
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

/*method to raise alarm for Audit DB failure*/
TcOperStatus
TcStartUpOperations::SendAuditDBFailNotice(uint32_t alarm_id) {
  std::string alm_msg, dbType;
  std::string alm_msg_summary;
  std::string vtn_name = "";
  pfc::alarm::alarm_info_with_key_t* data =
      new pfc::alarm::alarm_info_with_key_t;

  if (database_type_ == UNC_DT_RUNNING) {
    alm_msg = "Audit of Running DB failed";
    alm_msg_summary = "Audit of Running DB failed";
    dbType = "DB_Running";
  } else if (database_type_ == UNC_DT_STARTUP) {
    alm_msg = "Audit of Startup DB failed";
    alm_msg_summary = "Audit of Startup DB failed";
    dbType = "DB_Startup";
  } else if (database_type_ == UNC_DT_CANDIDATE) {
    alm_msg = "Audit of Candidate DB failed";
    alm_msg_summary = "Audit of Candidate DB failed";
    dbType = "DB_Candidate";
  }
  data->alarm_class = pfc::alarm::ALM_CRITICAL;
  data->alarm_kind = 1;
  data->apl_No = UNCCID_TC;
  data->alarm_category = 1;
  data->alarm_key_size = dbType.length();
  data->alarm_key = new uint8_t[dbType.length()+1];
  memcpy(data->alarm_key, dbType.c_str(), dbType.length()+1);

  pfc::alarm::alarm_return_code_t ret =
      pfc::alarm::pfc_alarm_send_with_key(vtn_name,
                                          alm_msg,
                                          alm_msg_summary,
                                          data, alarm_id);
  if (ret != pfc::alarm::ALM_OK) {
    delete []data->alarm_key;
    delete data;
    return TC_OPER_FAILURE;
  }
  delete []data->alarm_key;
  delete data;
  return TC_OPER_SUCCESS;
}


}  // namespace tc
}  // namespace unc
