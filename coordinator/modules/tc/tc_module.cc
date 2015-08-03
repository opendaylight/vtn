/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */


#include <tc_module.hh>
#include <alarm.hh>
#include <unc/component.h>

static int32_t alarm_id;

namespace unc {
namespace tc {

/**
 * @brief      TcModule constructor
 * @param[in]  *mattr Attribute of the Module
 */
TcModule::TcModule(const pfc_modattr_t *mattr)
                   : pfc::core::Module(mattr) ,
                     read_q_(NULL),
                     audit_q_(NULL) {
  max_failover_instance_ = 0;
  secondary_wait_time_for_cancelled_audit_ = 30;
}

/**
 * @brief      TcModule destructor
 */
TcModule::~TcModule() {}


/**
 * @brief Read the paramaters from conf file
 */
void TcModule::collect_db_params() {
  pfc::core::ModuleConfBlock tc_db_block(tc_conf_block);

  act_dsn_name =
      tc_db_block.getString(tc_conf_db_dsn_name_param,
                            tc_def_db_dsn_name_value.c_str());
  dsn_name = act_dsn_name;
  sby_dsn_name =
      tc_db_block.getString(tc_conf_sby_dsn_name_param,
                            tc_def_sby_dsn_name_value.c_str());

  max_failover_instance_ = tc_db_block.getUint32(max_failover_instance_param,
                                                max_failover_instance_value);
  secondary_wait_time_for_cancelled_audit_ = tc_db_block.getUint32(
                               secondary_wait_time_for_cancelled_audit_param,
                               secondary_wait_time_for_cancelled_audit_value);

  TcAuditOperations::SetSecondaryWaitTime(
                                    secondary_wait_time_for_cancelled_audit_);
}

/**
 * @brief Create DB Handler to read/write to TC DB tables.
 */
pfc_bool_t TcModule::validate_tc_db(TcDbHandler* tc_db_) {
  if (tc_db_->InitDB() != TCOPER_RET_SUCCESS) {
    return PFC_FALSE;
  }
  return PFC_TRUE;
}


/**
 * @brief Module Init
 */
pfc_bool_t TcModule::init() {
  /*Initialize alarm*/
  pfc::alarm::alarm_return_code_t alarm_retval = pfc::alarm::ALM_OK;
  alarm_retval = pfc::alarm::pfc_alarm_initialize(&alarm_id);
  if (alarm_retval != pfc::alarm::ALM_OK) {
    pfc_log_debug("Alarm intialization failed: %d", alarm_retval);
    return PFC_FALSE;
  } else {
    pfc_log_info("Initialised audit alarm fd: %d", alarm_id);
  }

  // Assign Memory to task queues
  read_q_ = new TcTaskqUtil(TC_READ_CONCURRENCY);
  if (PFC_EXPECT_FALSE(read_q_ == NULL)) {
    pfc_log_error("ReadQ Creation Failed");
    return PFC_FALSE;
  }

  audit_q_ = new TcTaskqUtil(TC_AUDIT_CONCURRENCY);
  if (PFC_EXPECT_FALSE(audit_q_ == NULL)) {
    pfc_log_error("AuditQ Creation Failed");
    return PFC_FALSE;
  }

  // Initialite Launcher Handler to collect Daemon channels
  TcLncApiHandler lncapi_;
  if ( PFC_EXPECT_FALSE(lncapi_.collect_unc_daemon_names() != PFC_TRUE) ) {
    pfc_log_error("Unable to Collect Channel Names");
    return PFC_FALSE;
  }

  // Extract the Channel Name Entries
  tc_channels_ = lncapi_.get_tc_channels();
  if ( tc_channels_.empty() ) {
    pfc_log_error("Channel Names List Empty");
    return PFC_FALSE;
  }

  // Register the state Handler
  if (RegisterStateHandlers() != PFC_TRUE) {
    UnRegisterStateHandlers();
    pfc_log_error("Unable to Register State Handlers");
    return PFC_FALSE;
  }
  /*Read the configuration file */
  collect_db_params();
  TcDbHandler tc_db_hdlr_(dsn_name);
  if (validate_tc_db(&tc_db_hdlr_) == PFC_FALSE) {
    pfc_log_error("%s validate_tc_db failed", __FUNCTION__);
    return PFC_FALSE;
  }
  return PFC_TRUE;
}


/**
 * @brief Handle StartUp notification from UNC Core
 */
pfc_bool_t TcModule::HandleStart() {
  TcLockRet ret=
    tc_lock_.GetLock(0,
                    TC_ACQUIRE_READ_LOCK_FOR_STATE_TRANSITION,
                    TC_WRITE_NONE); 
  if (PFC_EXPECT_FALSE(ret != TC_LOCK_SUCCESS)) {
    pfc_log_error("start:Failed to Acquire State Transition Lock");
    return PFC_FALSE;
  }
  tc_lock_.TcUpdateUncState(TC_SBY);
  ret=
      tc_lock_.ReleaseLock(0,
                           0,
                           TC_RELEASE_READ_LOCK_FOR_STATE_TRANSITION,
                           TC_WRITE_NONE);
  if ( PFC_EXPECT_FALSE(ret != TC_LOCK_SUCCESS) ) {
    pfc_log_fatal("stop:Cannot Release Lock Instance");
    return PFC_FALSE;
  }
  return PFC_TRUE;
}

/**
 * @brief Handle Stop notification from UNC Core
 */
pfc_bool_t TcModule::HandleStop() {
  pfc_log_info("%s Stop TC", __FUNCTION__);
  tc_lock_.TcUpdateUncState(TC_STOP);
  // stopping running session 
  TcClientSessionUtils::tc_session_cancel_all_sessions();
  // Stop any cond_wait audit threads
  TcAuditOperations::SignalPrimaryWaitingAudit();
  TcAuditOperations::SignalSecondaryWaitingAudit();
  // Release any config or candidate requests waiting in queue
  TcConfigOperations::ClearConfigAcquisitionQueue();
  TcCandidateOperations::ClearCandidateQueue();
  tc_lock_.ResetTcGlobalDataOnStateTransition();
  pfc_log_info("%s TC Stop completed", __FUNCTION__);
  return PFC_TRUE;
}

/**
 * @brief Handle ACT notification from clstat
 * @param[in]  is_switch denotes normal startup or switch cluster
 */
pfc_bool_t TcModule::HandleAct(pfc_bool_t is_switch) {
  TcLockRet ret=
    tc_lock_.GetLock(0,
                    TC_ACQUIRE_READ_LOCK_FOR_STATE_TRANSITION,
                    TC_WRITE_NONE);

  if (PFC_EXPECT_FALSE(ret != TC_LOCK_SUCCESS))  {
    pfc_log_fatal("act: Unable to Acquire Lock");
    return PFC_FALSE;
  }

  // Set DSN name with act_dsn_name in case of ACT
  dsn_name = act_dsn_name;

  tc_lock_.ResetTcGlobalDataOnStateTransition();
  /*clear alarms in transition*/
  pfc::alarm::pfc_alarm_clear(UNCCID_TC);

  TcDbHandler* tc_db_hdlr_ = new TcDbHandler(dsn_name);
  if (tc_db_hdlr_ == NULL) {
    return PFC_FALSE;
  }
  // create operations to send setup/setup_complete
  TcStartUpOperations startup_(&tc_lock_, NULL, tc_db_hdlr_, tc_channels_,
                               is_switch);

  TcOperStatus oper_ret(startup_.Dispatch());
  if (PFC_EXPECT_FALSE(oper_ret != TC_OPER_SUCCESS)) {
    if (PFC_EXPECT_TRUE(startup_.audit_db_fail_ == PFC_TRUE)) {
      pfc_log_error("Audit DB failure in switchover");
      /*Get failover instance from DB */
      uint32_t failover_instance;
      if (PFC_EXPECT_TRUE(TCOPER_RET_SUCCESS == tc_db_hdlr_->GetRecoveryTable(
                  &startup_.database_type_,
                  &startup_.fail_oper_,
                  &startup_.config_mode_,
                  &startup_.vtn_name_,
                  &failover_instance))) {
        /*increment and update failover instance*/
        tc_db_hdlr_->UpdateRecoveryTable(startup_.database_type_,
                                         startup_.fail_oper_,
                                         startup_.config_mode_,
                                         startup_.vtn_name_,
                                         ++failover_instance);
        startup_.audit_db_fail_ = PFC_FALSE;

        if (PFC_EXPECT_TRUE(failover_instance >= max_failover_instance_)) {
          /*sending alarm to usr*/
          if (PFC_EXPECT_TRUE(TC_OPER_SUCCESS !=
                              startup_.SendAuditDBFailNotice(alarm_id))) {
            pfc_log_warn("Failed to send AuditDBFailNotification");
          }

          pfc_log_info("Transiting to ACT_FAIL state");
          tc_lock_.TcUpdateUncState(TC_ACT_FAIL);
          ret = tc_lock_.ReleaseLock(0,
                                     0,
                                     TC_RELEASE_READ_LOCK_FOR_STATE_TRANSITION,
                                     TC_WRITE_NONE);
          if (PFC_EXPECT_FALSE(ret != TC_LOCK_SUCCESS)) {
            pfc_log_fatal("Cannot Release Lock Instance");
            return PFC_FALSE;
          }
          return PFC_TRUE;
        }
      } else {
        pfc_log_error("could not fetch RecoveryTable attributes");
      }
    }
    pfc_log_fatal("StartUp Operation Failure");
    ret = tc_lock_.ReleaseLock(0,
                               0,
                               TC_RELEASE_READ_LOCK_FOR_STATE_TRANSITION,
                               TC_WRITE_NONE);
    if (PFC_EXPECT_FALSE(ret != TC_LOCK_SUCCESS)) {
      pfc_log_error("Cannot Release Lock Instance");
      return PFC_FALSE;
    }
    return PFC_FALSE;
  }

  // Set back the state to ACT
  TcOperations::SetStateChangedToSby(PFC_FALSE);

  ret = tc_lock_.ReleaseLock(0,
                             0,
                             TC_RELEASE_READ_LOCK_FOR_STATE_TRANSITION,
                             TC_WRITE_NONE);
  if (PFC_EXPECT_FALSE(ret != TC_LOCK_SUCCESS)) {
    pfc_log_fatal("Cannot Release Lock Instance");
    return PFC_FALSE;
  }

  // Startup op completed successfully. Release any waiting lock...
  if (tc_lock_.CanWriteLock()) {
    TcCandidateOperations::HandleCandidateRelease();
  }

  return PFC_TRUE;
}

/**
 * @brief HandleConfigRequests to handle incoming config requests
 * @param[in]  sess Session invoking the service.
 */
TcOperStatus TcModule::HandleConfigRequests(pfc::core::ipc::ServerSession*
                                            oper_sess) {
  TcDbHandler* tc_db_hdlr_ = new TcDbHandler(dsn_name);
  if (tc_db_hdlr_ == NULL) {
    pfc_log_fatal("%s allocating DB handler failed", __FUNCTION__);
    return TC_OPER_FAILURE;
  }
  TcConfigOperations tc_config_oper(&tc_lock_,
                                    oper_sess,
                                    tc_db_hdlr_,
                                    tc_channels_);
  return tc_config_oper.Dispatch();
}

/**
 * @brief HandleStartUpRequests to handle incoming DB operation requests
 * @param[in]  sess Session invoking the service.
 */
TcOperStatus TcModule::HandleStartUpRequests(pfc::core::ipc::ServerSession*
                                             oper_sess) {
  TcDbHandler* tc_db_hdlr_ = new TcDbHandler(dsn_name);
  if (tc_db_hdlr_ == NULL) {
    pfc_log_fatal("allocating DB handler failed");
    return TC_OPER_FAILURE;
  }
  TcDbOperations tc_db_oper(&tc_lock_,
                            oper_sess,
                            tc_db_hdlr_,
                            tc_channels_);
  return tc_db_oper.Dispatch();
}

/**
 * @brief HandleReadRequests to handle incoming read requests
 * @param[in]  sess Session invoking the service.
 */
TcOperStatus TcModule::HandleReadRequests(pfc::core::ipc::ServerSession*
                                            oper_sess) {
  TcReadOperations tc_read_oper(&tc_lock_,
                                oper_sess,
                                NULL,
                                tc_channels_,
                                read_q_);
  return tc_read_oper.Dispatch();
}
/**
 *  @brief HandleReadStatusRequests to handle incoming read requests
 *  @param[in]  sess Session invoking the service.
 */  
TcOperStatus TcModule::HandleReadStatusRequests(pfc::core::ipc::ServerSession*
                                                 oper_sess) {
  TcReadStatusOperations tc_read_status_oper(&tc_lock_,
                                             oper_sess,
                                             NULL,
                                             tc_channels_);

  return tc_read_status_oper.Dispatch();
}
/**
 * @brief HandleAutoSaveRequests to handle incoming Auto Save requests
 * @param[in]  sess Session invoking the service.
 */
TcOperStatus TcModule::HandleAutoSaveRequests(pfc::core::ipc::ServerSession*
                                              oper_sess) {
  TcDbHandler* tc_db_hdlr_ = new TcDbHandler(dsn_name);
  if (tc_db_hdlr_ == NULL) {
    pfc_log_fatal("allocating DB handler failed");
    return TC_OPER_FAILURE;
  }

  // During Autosave operations (enable / disable), configuration
  // changes are saved to/from startup-config.
  // Setting timout as Infinite
  if (oper_sess->setTimeout(NULL) != TC_OPER_SUCCESS) {
    pfc_log_warn("HandleAutoSaveRequests:: Cannot set Infinite timeout."
                 "Opertation may timeout");
  }

  TcAutoSaveOperations tc_as_oper(&tc_lock_,
                                  oper_sess,
                                  tc_db_hdlr_,
                                  tc_channels_);
  return tc_as_oper.Dispatch();
}

/**
 * @brief HandleCandidateRequests to handle incoming Candidate requests
 * @param[in]  sess Session invoking the service.
 */
TcOperStatus TcModule::HandleCandidateRequests(pfc::core::ipc::ServerSession*
                                              oper_sess) {
  TcDbHandler* tc_db_hdlr_ = new TcDbHandler(dsn_name);
  if (tc_db_hdlr_ == NULL) {
    pfc_log_fatal("allocating DB handler failed");
    return TC_OPER_FAILURE;
  }

  TcCandidateOperations tc_commit_oper(&tc_lock_,
                                       oper_sess,
                                       tc_db_hdlr_,
                                       tc_channels_);
  return tc_commit_oper.Dispatch();
}

/**
 * @brief HandleAuditRequests to handle incoming Audit requests
 * @param[in]  sess Session invoking the service.
 */
TcOperStatus TcModule::HandleAuditRequests(pfc::core::ipc::ServerSession*
                                              oper_sess) {
  TcDbHandler* tc_db_hdlr_ = new TcDbHandler(dsn_name);
  if (tc_db_hdlr_ == NULL) {
    pfc_log_fatal("allocating DB handler failed");
    return TC_OPER_FAILURE;
  }
  TcAuditOperations tc_audit_oper(&tc_lock_,
                                  oper_sess,
                                  tc_db_hdlr_,
                                  tc_channels_,
                                  audit_q_);
  return tc_audit_oper.Dispatch();
}

/**
 * @brief release all config session.
 */
TcOperStatus TcModule::ReleaseConfigSession() {
  TcOperStatus    ret = TC_OPER_SUCCESS;
  TcConfigNameMap::const_iterator cit;

  TcConfigNameMap tc_config_name_map;
  tc_config_name_map = tc_lock_.GetConfigMap();

  cit = tc_config_name_map.begin();
  for(; cit != tc_config_name_map.end(); cit++) {
    if ((cit->second).is_taken) {
      TcDbHandler* tc_db_hdlr_ = new TcDbHandler(dsn_name);
      if (tc_db_hdlr_ == NULL) {
        pfc_log_fatal("%s allocating DB handler failed", __FUNCTION__);
        return TC_OPER_FAILURE;
      }
      TcConfigOperations tc_config_oper(&tc_lock_,
                                        NULL,
                                        tc_db_hdlr_,
                                        tc_channels_);

      tc_config_oper.session_id_  = (cit->second).session_id;
      tc_config_oper.config_id_   = (cit->second).config_id;

      if (cit->first == "global-mode")  {
        tc_config_oper.tc_mode_ = TC_CONFIG_GLOBAL;
      } else if (cit->first == "real-mode") {
          tc_config_oper.tc_mode_ = TC_CONFIG_REAL;
      } else if (cit->first == "virtual-mode")  {
          tc_config_oper.tc_mode_ = TC_CONFIG_VIRTUAL;
      } else  {
          tc_config_oper.tc_mode_ = TC_CONFIG_VTN;
          tc_config_oper.vtn_name_ = cit->first;
      }

      tc_config_oper.tc_oper_ = TC_OP_CONFIG_RELEASE;
      ret = tc_config_oper.Dispatch();

      pfc_log_info("%s (All) config_id[%u] sess_id[%u], config_mode[%s],"
                   "vtn-name[%s] return %d", __FUNCTION__,
                   tc_config_oper.config_id_, tc_config_oper.session_id_,
                   (cit->first).c_str(), (cit->first).c_str(), ret);
    }
  }
  return ret;
}

/**
 * @brief Check if Config Session Exists and release the same.
 */
TcOperStatus TcModule::ReleaseConfigSession(uint32_t session_id) {
  uint32_t      config_id;
  TcConfigMode  config_mode;
  std::string   vtn_name;
  TcApiRet ret(TcGetConfigSession(session_id, config_id, 
                                  config_mode, vtn_name));
  if ( ret != TC_API_COMMON_SUCCESS ) {
  // Config Session Not Present Not needed to release
    pfc_log_warn("%s Session[%u] does not exist", __FUNCTION__, session_id);
       return TC_OPER_SUCCESS;
  }
  TcDbHandler* tc_db_hdlr_ = new TcDbHandler(dsn_name);
  if (tc_db_hdlr_ == NULL) {
    pfc_log_fatal("%s allocating DB handler failed", __FUNCTION__);
    return TC_OPER_FAILURE;
  }

  TcConfigOperations tc_config_oper(&tc_lock_,
                                    NULL,
                                    tc_db_hdlr_,
                                    tc_channels_);
  tc_config_oper.session_id_ = session_id;
  tc_config_oper.config_id_ = config_id;
  tc_config_oper.tc_mode_ = config_mode;
  if (config_mode == TC_CONFIG_VTN) {
    tc_config_oper.vtn_name_ = vtn_name;
  }
  tc_config_oper.tc_oper_ = TC_OP_CONFIG_RELEASE;
  return tc_config_oper.Dispatch();
}


/**
 * @brief ipcService to handle incoming requests
 * @param[in]  sess Session invoking the service.
 * @param[in]  service denotes the service Id of the service
 */
pfc_ipcresp_t TcModule::ipcService(pfc::core::ipc::ServerSession& sess,
                                      pfc_ipcid_t service) {
  pfc_log_info("TC Service Request");
  pfc::core::ipc::ServerSession* oper_sess=&sess;
  switch (service) {
  case TC_CONFIG_SERVICES:
  {
    return HandleConfigRequests(oper_sess);
  }
  case TC_STARTUP_DB_SERVICES:
  {
    return HandleStartUpRequests(oper_sess);
  }
  case TC_READ_ACCESS_SERVICES:
  {
    return HandleReadRequests(oper_sess);
  }
  case TC_READ_STATUS_SERVICES:
  {
    return HandleReadStatusRequests(oper_sess);
  }
  case TC_AUTO_SAVE_SERVICES:
  {
    return HandleAutoSaveRequests(oper_sess);
  }
  case TC_CANDIDATE_SERVICES:
  {
    return HandleCandidateRequests(oper_sess);
  }
  case TC_AUDIT_SERVICES:
  {
    return HandleAuditRequests(oper_sess);
  }
  default:
    pfc_log_error("Invalid Service");
    return TC_OPER_INVALID_INPUT;
  }
}

/**
 * @brief Module fini
 */
pfc_bool_t TcModule::fini() {
  pfc_log_info("TC fini");
  UnRegisterStateHandlers();
  pfc::alarm::pfc_alarm_close(alarm_id);
  delete read_q_;
  read_q_ = NULL;
  delete audit_q_;
  audit_q_ = NULL;
  pfc_log_info("TC fini completed");
  return PFC_TRUE;
}

/**
 * @brief   Get Config Session Details
 * @param[out] session_id of config session.
 * @param[out] config_id of the config session.
 */
TcApiRet TcModule::TcGetConfigSession(uint32_t session_id,
                                      uint32_t& config_id,
                                      TcConfigMode& tc_mode,
                                      std::string& vtn_name) {
  switch (tc_lock_.GetConfigData(session_id, config_id, tc_mode, vtn_name)) {
  case TC_LOCK_INVALID_PARAMS:
    return TC_INVALID_PARAM;

  case TC_LOCK_INVALID_UNC_STATE:
    return TC_INVALID_UNC_STATE;

  case TC_LOCK_SUCCESS:
    return TC_API_COMMON_SUCCESS;

  case TC_LOCK_INVALID_SESSION_ID:
  case TC_LOCK_NO_CONFIG_SESSION_EXIST:
  default:
    return TC_NO_CONFIG_SESSION;
  }
}

/**
 * @brief Notify TC to Release the Session
 * @param[in] session_id.
 */
TcApiRet TcModule::TcReleaseSession(uint32_t session_id) {
  TcState state(tc_lock_.GetUncCurrentState());

  if ( (state != TC_ACT) && (state != TC_SBY) ) {
    return TC_INVALID_UNC_STATE;
  }
  switch (tc_lock_.GetSessionOperation(session_id)) {
  // Session is a read session
  // Release the Read Access
  case TC_READ_PROGRESS:
  {
    TcReadOperations tc_read_oper(&tc_lock_,
                                  NULL,
                                  NULL,
                                  tc_channels_,
                                  read_q_);
     tc_read_oper.session_id_ = session_id;
     tc_read_oper.tc_oper_ = TC_OP_READ_RELEASE;
     tc_read_oper.Dispatch();
     return TC_API_COMMON_SUCCESS;
  }
  // Session is a config where commit in progress
  // Mark the session for release
  case TC_WRITE_ABORT_CANDIDATE_CONFIG_PROGRESS:
  case TC_CONFIG_COMMIT_PROGRESS:
  {
    TcLockRet ret(tc_lock_.TcMarkSessionId(session_id));
    if ( ret == TC_LOCK_SUCCESS )
      return TC_API_COMMON_SUCCESS;
    else
      return TC_INVALID_PARAM;
  }

  // Session is a config session
  // Release the Config Session and notify UPLL/UPPL
  case TC_CONFIG_NO_NOTIFY_PROGRESS:
  {
    if ( ReleaseConfigSession(session_id) == TC_OPER_SUCCESS ) {
      return TC_API_COMMON_SUCCESS;
    }
    return TC_INVALID_PARAM;
  }

  // Session with no Operation
  case TC_NO_OPERATION_PROGRESS:
    return TC_INVALID_PARAM;

  // Other Operations are not session specific
  default:
    return TC_API_COMMON_SUCCESS;
  }
}

}  // namespace tc
}  // namespace unc
PFC_MODULE_IPC_DECL(unc::tc::TcModule, TC_IPC_NSERVICES);
