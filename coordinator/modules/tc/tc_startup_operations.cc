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
    fail_oper_(TC_OP_INVALID), version_(0), database_type_(UNC_DT_INVALID),
    config_mode_(TC_CONFIG_INVALID) {}


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
  uint32_t failover_instance = 0;
  TcOperRet ret = db_hdlr_->GetRecoveryTable(&database_type_,
                                            &fail_oper_,
                                            &config_mode_,
                                            &vtn_name_,
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
      // Cold-start
      pfc_bool_t autosave_enabled = PFC_FALSE;
      TcOperRet ret = db_hdlr_->GetConfTable(&autosave_enabled);
      if ( ret == TCOPER_RET_FAILURE ) {
        pfc_log_error("Database Read Failed for getting autosave");
        return TC_OPER_FAILURE;
      }

      pfc_log_info("%s COLD-START; AutosaveEnabled? %d db=%d, failoper=%d"
                   "configmode=%d, vtn_name=%s",
                   __FUNCTION__, autosave_enabled, database_type_, fail_oper_,
                   config_mode_, vtn_name_.c_str());

      if(autosave_enabled == PFC_TRUE && database_type_ != UNC_DT_INVALID &&
                                         fail_oper_ == TC_OP_CANDIDATE_COMMIT) {
        TcOperMessageList.push_back(unc::tclib::MSG_AUDITDB);
      } else if (autosave_enabled == PFC_FALSE &&
                 database_type_ != UNC_DT_INVALID && 
                 (fail_oper_ == TC_OP_RUNNING_SAVE ||
                  fail_oper_ == TC_OP_CLEAR_STARTUP)) {
        TcOperMessageList.push_back(unc::tclib::MSG_AUDITDB);
      }
                
      TcOperMessageList.push_back(unc::tclib::MSG_SETUP);
    } else {
      // Switchover / Failover
      pfc_log_info("%s Switch/Failover; db=%d, failoper=%d, "
                   "configmode=%d, vtn_name=%s",
                   __FUNCTION__, database_type_, fail_oper_,
                   config_mode_, vtn_name_.c_str());
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
    tc_msg->SetData(database_type_, fail_oper_, version_);
    tc_msg->SetData(database_type_, fail_oper_);
    tc_msg->SetData(0, config_mode_, vtn_name_);
  }

  return TC_OPER_SUCCESS;
}

/*
 * @brief Execute the operation
 */

TcOperStatus 
  TcStartUpOperations::Execute() {
  TcOperRet MsgRet = TCOPER_RET_SUCCESS;

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

    version_ = 0;
    if (*MsgIter == unc::tclib::MSG_AUDITDB) {
      if ( fail_oper_ == TC_OP_CANDIDATE_ABORT ) {
        if (db_hdlr_->GetRecoveryTableAbortVersion(version_)
                          != TCOPER_RET_SUCCESS) {
          pfc_log_warn("AuditDB: Getting abort version from db failed");
        } else {
          pfc_log_info("AuditDB: Retrieved abort version:%"PFC_PFMT_u64,
                       version_);
        }
      } else if (fail_oper_ == TC_OP_RUNNING_SAVE) {
        if (db_hdlr_->GetRecoveryTableSaveVersion(version_)
                          != TCOPER_RET_SUCCESS) {
          pfc_log_warn("AuditDB: Getting save version from db failed");
        } else {
          pfc_log_info("AuditDB: Retrieved save version:%"PFC_PFMT_u64,
                       version_);
        }
      }
    }

    if (FillTcMsgData(tcmsg_, *MsgIter) != TC_OPER_SUCCESS) {
      delete tcmsg_;
      tcmsg_ = NULL;
      return TC_SYSTEM_FAILURE;
    }

    if (*MsgIter == unc::tclib::MSG_AUDITDB) {
      if (database_type_ == UNC_DT_RUNNING) {
          TcReadStatusOperations::SetRunningStatus();
          MsgRet = tcmsg_->Execute();
          TcReadStatusOperations::SetRunningStatusIncr();
      } else if (database_type_ == UNC_DT_STARTUP)  {
          TcReadStatusOperations::SetStartupStatus();
          MsgRet = tcmsg_->Execute();
          TcReadStatusOperations::SetStartupStatusIncr();
      } else {
        MsgRet = tcmsg_->Execute();
      }
    } else {
      MsgRet = tcmsg_->Execute();
    }
   
    if ( MsgRet != TCOPER_RET_SUCCESS ) {
      delete tcmsg_;
      tcmsg_ = NULL;
      if (*MsgIter == unc::tclib::MSG_AUDITDB) {
        if (MsgRet == TCOPER_RET_LAST_DB_OP_FAILED) {
          pfc_log_warn("AuditDB last DB operation failed. Continue...");    
          MsgIter++;
          continue;
        } else {
          /*set when Audit DB fails in startup phase*/
          pfc_log_error("AuditDB failed");
          audit_db_fail_ = PFC_TRUE;
        }
      }
      return HandleMsgRet(MsgRet);
    } else  {
      if (*MsgIter == unc::tclib::MSG_AUDITDB) {
        ++version_;
        if (fail_oper_ == TC_OP_CANDIDATE_ABORT) {
          if (db_hdlr_->UpdateRecoveryTableAbortVersion(version_)
                            != TCOPER_RET_SUCCESS) {
            pfc_log_warn("AuditDB: Setting abort version to db failed");
          } else {
            pfc_log_info("AuditDB: Set abort version:%"PFC_PFMT_u64,
                         version_);
          }
        } else if (fail_oper_ == TC_OP_RUNNING_SAVE) {
          if (db_hdlr_->UpdateRecoveryTableSaveVersion(version_)
                            != TCOPER_RET_SUCCESS) {
            pfc_log_warn("AuditDB: Setting save version from db failed");
          } else {
            pfc_log_info("AuditDB: Set save version:%"PFC_PFMT_u64,
                         version_);
          }
        }
      }

      // SETUP message is sent only during cold startup.
      // In case of switchover/faileover, AUDITDB is sent.
      // Initialize the abort/save versions in RECOVERY_DB during cold startup
      if (*MsgIter == unc::tclib::MSG_SETUP) {
        pfc_log_info("Initializing the abort and save versions in RECOVERY DB");
        version_ = 0;
        if (db_hdlr_->UpdateRecoveryTableAbortVersion(version_)
                            != TCOPER_RET_SUCCESS) {
          pfc_log_warn("SETUP: Init of abort version to db failed");
        }
        if (db_hdlr_->UpdateRecoveryTableSaveVersion(version_)
                            != TCOPER_RET_SUCCESS) {
          pfc_log_warn("SETUP: Init of save version to db failed");
        }
      }
    }
    
    if (*MsgIter == unc::tclib::MSG_SETUP_COMPLETE) {
      tclock_->TcSetSetupComplete(PFC_TRUE);
      /* Init generation number and status of Running and Startup config*/
      TcReadStatusOperations::Init();
      pfc_log_info("MSG_SETUP_COMPLETE completed");
    }
    MsgIter++;
    delete tcmsg_;
  }  // while 
  return TC_OPER_SUCCESS;
}

/*
 * @brief Send Response
 */
TcOperStatus
  TcStartUpOperations::SendResponse(TcOperStatus ret_) {
  TcOperRet ret = TCOPER_RET_SUCCESS;

  pfc_log_info("send_oper_status value=%d", ret_);
  if (ret_ == TC_OPER_SUCCESS) {
    ret= db_hdlr_->UpdateRecoveryTable(UNC_DT_INVALID,
                                       TC_OP_INVALID,
                                       TC_CONFIG_INVALID,
                                       "", 0);
    if ( ret != TCOPER_RET_SUCCESS ) {
      pfc_log_error("%s failed to update recovery table", __FUNCTION__);
      return TC_SYSTEM_FAILURE;
    }

    pfc_bool_t global_mode_dirty = PFC_FALSE;

    if (db_hdlr_->GetRecoveryTableGlobalModeDirty(global_mode_dirty)
                                               != TCOPER_RET_SUCCESS) {
      pfc_log_error("%s failed to fetch Global Mode Dirty", __FUNCTION__);
      return TC_SYSTEM_FAILURE;
    }

    if (global_mode_dirty == PFC_TRUE) {
      if (IsCandidateDirty(USESS_ID_TC, 0) == PFC_FALSE) {
        // If Candidate is not dirty in both upll and uppl,
        // reset the global_mode_dirty
        ret = db_hdlr_->UpdateRecoveryTableGlobalModeDirty(PFC_FALSE);
        if (ret != TCOPER_RET_SUCCESS) {
          pfc_log_error("%s failed to update Global Mode Dirty", __FUNCTION__);
          return TC_SYSTEM_FAILURE;
        }
      }
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
