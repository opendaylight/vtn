/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/**
 * @brief    Physical Core
 * @file     physicalcore.cc
 *
 */

#include <unc/config.h>
#include <unc/component.h>
#include <unc/unc_base.h>
#include <alarm.hh>
#include <clstat_api.h>
#include <uncxx/lib_unc.hh>
#include "physical_core.hh"
#include "physicallayer.hh"
#include "tclib_module.hh"
#include "itc_kt_controller.hh"
#include "ipc_client_configuration_handler.hh"

using unc::tclib::TcApiCommonRet;
using unc::tclib::TcLibModule;

// Alarm FD
static int32_t fd;
using unc::unclib::UncModeUtil;

namespace unc {
namespace uppl {

// Static Variable initialization
PhysicalCore* PhysicalCore::physical_core_ = NULL;

/**
 * @Description : The function returns singleton instance of PhysicalCore class
 */

PhysicalCore* PhysicalCore::get_physical_core() {
  PhysicalLayer* physical_layer = PhysicalLayer::get_instance();

  physical_layer->physical_core_mutex_.lock();
  if (physical_core_ == NULL) {
    physical_core_ = new PhysicalCore();
  }
  physical_layer->physical_core_mutex_.unlock();
  return physical_core_;
}

/**
 * @Description : This function initializes physical core members and reads
 *                config, sends event subscription to driver
 */

UncRespCode PhysicalCore::InitializePhysical() {
  // initialize the class member data
  // Create new internal transaction coordinator object
  pfc_log_debug("system_transit_state_ :%d", system_transit_state_);
  internal_transaction_coordinator_ = InternalTransactionCoordinator::
                get_internaltransactioncoordinator();
  if (internal_transaction_coordinator_ == NULL) {
    pfc_log_error("Memory not allocated for internal_transaction_coordinator_");
    return UNC_UPPL_RC_ERR_FATAL_RESOURCE_ALLOCATION;
  }

  // Read config from file
  UncRespCode ret = ReadConfigFile();
  pfc_log_debug("Read Configuration return %d", ret);
  if (ret != UNC_RC_SUCCESS) {
    return UNC_UPPL_RC_ERR_CONF_FILE_READ;
  }

  // initialize alarm object
  pfc::alarm::alarm_return_code_t alarm_ret = pfc::alarm::ALM_OK;
  alarm_ret = pfc::alarm::pfc_alarm_initialize(&fd);
  pfc_log_debug("PFC Alarm intialization return %d", alarm_ret);
  if (alarm_ret != pfc::alarm::ALM_OK) {
    return UNC_UPPL_RC_ERR_ALARM_API;
  } else {
    // clear all alarms
    pfc::alarm::pfc_alarm_clear(UNCCID_PHYSICAL);
  }

  pfc_bool_t event_ret = RegisterStateHandlers();
  pfc_log_debug("State handlers registration return %d", event_ret);
  if (event_ret == PFC_TRUE) {
    ret =  UNC_RC_SUCCESS;
  } else {
    ret = UNC_UPPL_RC_FAILURE;
  }

  TcApiCommonRet ret_code = unc::tclib::TC_API_COMMON_SUCCESS;
  TcLibModule* tclib_ptr = static_cast<TcLibModule*>
  (TcLibModule::getInstance(TCLIB_MODULE_NAME));
  if (tclib_ptr != NULL) {
    ret_code = tclib_ptr->TcLibRegisterHandler(this);
    pfc_log_info("TcLibRegisterHandler returned %u.", ret_code);
    if (ret_code != unc::tclib::TC_API_COMMON_SUCCESS) {
      ret = UNC_UPPL_RC_FAILURE;
    }
  } else {
    ret = UNC_UPPL_RC_FAILURE;
  }
  return ret;
}

/**
 * @Description : This function finalizes physical core members
 */

UncRespCode PhysicalCore::FinalizePhysical() {
  // Finalize the class member data
  // IPC Event handler removal is taken care by ipc library itself during fini
  /* Cancel the event Subscription from driver
  UncRespCode ret = CancelEventSubscripInDriver(); */

  NotificationManager::delete_taskq_util();

  UncRespCode ret = UNC_UPPL_RC_FAILURE;
  pfc_bool_t event_ret = UnRegisterStateHandlers();
  if (event_ret == PFC_TRUE) {
    ret =  UNC_RC_SUCCESS;
  } else {
    ret = UNC_UPPL_RC_FAILURE;
  }

  if (internal_transaction_coordinator_ != NULL)
    delete internal_transaction_coordinator_;

  pfc::alarm::pfc_alarm_close(fd);

  if (physical_core_ != NULL)
    delete physical_core_;

  return ret;
}

/**
 * @Description : This function reads config from file
 */

UncRespCode PhysicalCore::ReadConfigFile() {
  // Config file reading
  PhysicalLayer* physical_layer = PhysicalLayer::get_instance();
  if (physical_layer == NULL) {
    return UNC_UPPL_RC_ERR_FATAL_RESOURCE_ALLOCATION;
  }
  pfc::core::ModuleConfBlock ipcblock(IPC_PARAMS_BLK);

  uppl_ipc_channel_name_ = ipcblock.getString("uppl_channel_name",
                                              UPPL_IPC_CHN_NAME);
  uppl_ipc_service_name_ = ipcblock.getString("uppl_service_name",
                                              UPPL_IPC_SVC_NAME);

  physical_layer->get_ipc_connection_manager()->InitIpcConnectionManager(
      uppl_ipc_channel_name_,
      uppl_ipc_service_name_);

  // Fill the driver name map
  drv_name_map_[UNC_CT_PFC] = ipcblock.getString("pfcdrv_service_name", "");
  drv_name_map_[UNC_CT_VNP] = ipcblock.getString("ovrlaydrv_service_name", "");
  drv_name_map_[UNC_CT_POLC] = ipcblock.getString("polcdrv_service_name", "");
  drv_name_map_[UNC_CT_ODC] = ipcblock.getString("odcdrv_service_name", "");

  int kSize = ipcblock.arraySize("sb_ipc_service_ids");
  uint8_t sb_ipc_service_ids[kSize];
  for (int i = 0; i < kSize; ++i) {
    sb_ipc_service_ids[i] = ipcblock.getByteAt("sb_ipc_service_ids", i, 0);
    sb_ipc_service_ids_.push_back(sb_ipc_service_ids[i]);
  }

  kSize = ipcblock.arraySize("logical_ipc_service_ids");
  uint8_t logical_ipc_service_ids[kSize];
  for (int i = 0; i < kSize; ++i) {
    logical_ipc_service_ids[i] = ipcblock.getByteAt("logical_ipc_service_ids",
                                                    i, 0);
    logical_ipc_service_ids_.push_back(logical_ipc_service_ids[i]);
  }

  kSize = ipcblock.arraySize("uppl_ipc_service_ids");
  uint8_t uppl_ipc_service_ids[kSize];
  for (int i = 0; i < kSize; ++i) {
    uppl_ipc_service_ids[i] = ipcblock.getByteAt("uppl_ipc_service_ids", i, 0);
    uppl_ipc_service_ids_.push_back(uppl_ipc_service_ids[i]);
  }

  audit_notfn_timeout_ = ipcblock.getUint32("audit_notfn_timeout", 30);
  unknown_controller_count_ = ipcblock.getUint32("unknown_controller_count", 1);
  uppl_max_dataflowtraversal_ = ipcblock.getUint32(
                                    "uppl_max_dataflowtraversal", 1000);
  pfc_log_debug("uppl_max_dataflowtraversal_ - red from uppl.conf = %d",
                  uppl_max_dataflowtraversal_);
  uppl_max_ro_db_connections_ = ipcblock.getUint32(
                                    "uppl_max_ro_db_connections", 100);
  pfc_log_debug("uppl_max_ro_db_connections_ - red from uppl.conf = %d",
                  uppl_max_ro_db_connections_);
  // accessing the unclib UncModeUtil api to get unc_mode
  int ret_code = 0;
  UncModeUtil ouncmodeutil(ret_code);
  if (ret_code != 0)
    pfc_log_debug("Failed to access unclib.conf. default unc_mode is taken");
  unc_mode_ = (UncMode)ouncmodeutil.libunc_get_unc_mode();
  pfc_log_debug("unc_mode from unclib is %d", unc_mode_);
  return UNC_RC_SUCCESS;
}


/**
 * @Description : This function sends event subscription request to Driver
 *                through IPCConnectionManager
 */

UncRespCode PhysicalCore::SendEventSubscripToDriver() {
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  if (physical_layer == NULL) {
    return UNC_UPPL_RC_ERR_FATAL_RESOURCE_ALLOCATION;
  }
  UncRespCode ret = physical_layer->get_ipc_connection_manager()->
      SendEventSubscription();
  return ret;
}

/**
 * @Description : This function sends cancel event subscription request to
 *                Driver through IPCConnectionManager
 */

UncRespCode PhysicalCore::CancelEventSubscripInDriver() {
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  if (physical_layer == NULL) {
    return UNC_UPPL_RC_ERR_FATAL_RESOURCE_ALLOCATION;
  }
  UncRespCode ret = physical_layer->get_ipc_connection_manager()->
      CancelEventSubscription();
  return ret;
}

/**
 * @Description : This function validates the config ID by TC library API
 */

UncRespCode PhysicalCore::ValidateConfigId(uint32_t session_id,
                                           uint32_t config_id) {
  TcLibModule* tclib_ptr = static_cast<TcLibModule*>
  (TcLibModule::getInstance(TCLIB_MODULE_NAME));
  if (tclib_ptr == NULL) {  // cpptest issue fix
    return UNC_UPPL_RC_ERR_INVALID_STATE;
  }
  //  as part of paritial configure mode support, config_mode, vtn_name
  //  are passed in this API. vtn_name is always null.
  TcConfigMode config_mode = TC_CONFIG_REAL;
  std::string vtn_name = "";
  uint8_t resp = tclib_ptr->TcLibValidateUpdateMsg(session_id, config_id,
                                                   config_mode,
                                                   vtn_name);
  UncRespCode return_code = UNC_UPPL_RC_FAILURE;
  pfc_log_debug("Validation of config/session id with TC returned %d"
                                " config mode %d", resp, config_mode);
  if (resp == unc::tclib::TC_API_COMMON_SUCCESS) {
    return_code = UNC_RC_SUCCESS;
  } else if (resp == unc::tclib::TC_INVALID_CONFIG_ID) {
    return_code = UNC_UPPL_RC_ERR_INVALID_CONFIGID;
  } else if (resp == unc::tclib::TC_INVALID_SESSION_ID) {
    return_code = UNC_UPPL_RC_ERR_INVALID_SESSIONID;
  } else if (resp == unc::tclib::TC_INVALID_MODE) {
    return_code = UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED;
  }
  // return the response received
  return return_code;
}

/**
 * @Description : This function getss the config Mode by TC library API
 */

UncRespCode PhysicalCore::GetConfigMode(uint32_t session_id,
                                        uint32_t config_id,
                                        TcConfigMode &config_mode,
                                        std::string vtn_name) {
  TcLibModule* tclib_ptr = static_cast<TcLibModule*>
  (TcLibModule::getInstance(TCLIB_MODULE_NAME));
  if (tclib_ptr == NULL) {  // cpptest issue fix
    return UNC_UPPL_RC_ERR_INVALID_STATE;
  }
  //  as part of paritial configure mode support, config_mode, vtn_name
  //  are passed in this API. vtn_name is always null.
  uint8_t resp = tclib_ptr->TcLibGetConfigMode(session_id, config_id,
                                                   config_mode,
                                                   vtn_name);
  UncRespCode return_code = UNC_UPPL_RC_FAILURE;
  pfc_log_debug("GetConfigMode returned %d config mode %d", resp, config_mode);
  if (resp == unc::tclib::TC_API_COMMON_SUCCESS) {
    return_code = UNC_RC_SUCCESS;
  } else if (resp == unc::tclib::TC_INVALID_CONFIG_ID) {
    return_code = UNC_UPPL_RC_ERR_INVALID_CONFIGID;
  } else if (resp == unc::tclib::TC_INVALID_SESSION_ID) {
    return_code = UNC_UPPL_RC_ERR_INVALID_SESSIONID;
  }
  // return the response received
  return return_code;
}

/**
 * @Description : This function validates if the operation is valid
 * in UPPL_SYSTEM_ST_STANDBY state
 * @param[in] : operation_type - type of operation requested
 */

UncRespCode PhysicalCore::ValidateStandbyRequests(uint32_t operation_type) {
  // Throw error when non-READ* requests are received in standby
  if (get_system_state() == UPPL_SYSTEM_ST_STANDBY &&
      (operation_type < UNC_OP_READ ||
          operation_type > UNC_OP_READ_SIBLING_COUNT)) {
    pfc_log_info("operation not supported in standby");
    return UNC_UPPL_RC_ERR_NOT_SUPPORTED_BY_STANDBY;
  }
  return UNC_RC_SUCCESS;
}

/**
 * @Description : This function gives the corresponding for controller type
 */

UncRespCode PhysicalCore::GetDriverName(
    unc_keytype_ctrtype_t controller_type,
    string &driver_name) {
  map<unc_keytype_ctrtype_t, string>::iterator
  iter_drv_name_map = drv_name_map_.find(controller_type);
  if (iter_drv_name_map == drv_name_map_.end()) {
    pfc_log_info("Controller type not found");
    return UNC_UPPL_RC_FAILURE;
  }
  if (iter_drv_name_map->second == "") {
    pfc_log_info("Driver name empty");
    return UNC_UPPL_RC_FAILURE;
  }
  driver_name = iter_drv_name_map->second;
  return UNC_RC_SUCCESS;
}

/**
 * @Description : This function will be called back when TC sends user
 *                commit-transaction start request to UPPL
 *                This is an implementation of virtual function in TCLib
 */

TcCommonRet PhysicalCore::HandleCommitTransactionStart(uint32_t session_id,
                                                       uint32_t config_id,
                                                       TcConfigMode config_mode,
                                                       std::string vtn_name) {
  PHY_FINI_IPC_LOCK(unc::tclib::TC_SUCCESS);
  //  Reject the request when system is in StandBy
  if (get_system_state() == UPPL_SYSTEM_ST_STANDBY) {
    pfc_log_error("Operation Not allowed: System is in standby\n");
    return unc::tclib::TC_FAILURE;
  }
  OPEN_DB_CONNECTION_TC_REQUEST(unc::uppl::kOdbcmConnReadWriteNb);
  // Call ITC transaction handler function
  UncRespCode resp = internal_transaction_coordinator_->transaction_req()->
      StartTransaction(&db_conn, session_id, config_id, config_mode);
  pfc_log_debug("HandleCommitTransactionStart return code %d", resp);
  // convert the error code returned by ITC to TC error code
  if (resp == UNC_RC_SUCCESS) {
    return unc::tclib::TC_SUCCESS;
  } else {
    return unc::tclib::TC_FAILURE;
  }
}

/**
 * @Description : This function will be called back when TC sends user
 *                Audit-transaction start request to UPPL
 *                This is an implementation of virtual function in TCLib
 */

TcCommonRet PhysicalCore::HandleAuditTransactionStart(
    uint32_t session_id,
    unc_keytype_ctrtype_t ctrl_type,
    string controller_id) {
  PHY_FINI_IPC_LOCK(unc::tclib::TC_SUCCESS);
  //  Reject the request when system is in StandBy
  if (get_system_state() == UPPL_SYSTEM_ST_STANDBY) {
    pfc_log_error("Operation Not allowed: System is in standby\n");
    return unc::tclib::TC_FAILURE;
  }
  pfc_log_debug("Received HandleAuditTransactionStart from TC");
  // Call ITC transaction handler function
  UncRespCode resp = internal_transaction_coordinator_->audit_req()->
      StartAuditTransaction(session_id,
                            ctrl_type,
                            controller_id);

  // convert the error code returned by ITC to TC error code
  if (resp == UNC_RC_SUCCESS) {
    return unc::tclib::TC_SUCCESS;
  } else {
    return unc::tclib::TC_FAILURE;
  }
}

/**
 * @Description : This function will be called back when TC sends user
 *                commit-transaction end request to UPPL
 *                This is an implementation of virtual function in TCLib
 */

TcCommonRet PhysicalCore::HandleCommitTransactionEnd(
    uint32_t session_id,
    uint32_t config_id,
    TcConfigMode config_mode,
    std::string vtn_name,
    TcTransEndResult trans_res) {
  PHY_FINI_IPC_LOCK(unc::tclib::TC_SUCCESS);
  //  Reject the request when system is in StandBy
  if (get_system_state() == UPPL_SYSTEM_ST_STANDBY) {
    pfc_log_error("Operation Not allowed: System is in standby\n");
    return unc::tclib::TC_FAILURE;
  }
  OPEN_DB_CONNECTION_TC_REQUEST(unc::uppl::kOdbcmConnReadWriteNb);
  // Call ITC transaction handler function
  UncRespCode resp = internal_transaction_coordinator_->transaction_req()->
      EndTransaction(&db_conn, session_id,
                     config_id, config_mode,
                     trans_res,
                     true);
  pfc_log_debug("HandleCommitTransactionEnd response %d", resp);
  // convert the error code returned by ITC to TC error code
  if (resp == UNC_RC_SUCCESS) {
    return unc::tclib::TC_SUCCESS;
  } else {
    return unc::tclib::TC_FAILURE;
  }
}

/**
 * @Description : This function will be called back when TC sends user
 *                Audit-transaction End request to UPPL
 *                This is an implementation of virtual function in TCLib
 */

TcCommonRet PhysicalCore::HandleAuditTransactionEnd(
    uint32_t session_id,
    unc_keytype_ctrtype_t ctrl_type,
    string controller_id,
    TcTransEndResult end_res) {
  PHY_FINI_IPC_LOCK(unc::tclib::TC_SUCCESS);
  //  Reject the request when system is in StandBy
  if (get_system_state() == UPPL_SYSTEM_ST_STANDBY) {
    pfc_log_error("Operation Not allowed: System is in standby\n");
    return unc::tclib::TC_FAILURE;
  }
  pfc_log_debug("Received HandleAuditTransactionEnd from TC");
  // Call ITC transaction handler function
  UncRespCode resp = internal_transaction_coordinator_->audit_req()->
      EndAuditTransaction(session_id,
                          ctrl_type,
                          controller_id);

  // convert the error code returned by ITC to TC error code
  if (resp == UNC_RC_SUCCESS) {
    return unc::tclib::TC_SUCCESS;
  } else {
    return unc::tclib::TC_FAILURE;
  }
}

/**
 * @Description : This function will be called back when TC sends user
 *                commit vote request to UPPL
 *                This is an implementation of virtual function in TCLib
 */

TcCommonRet PhysicalCore::HandleCommitVoteRequest(
    uint32_t session_id,
    uint32_t config_id,
    TcConfigMode config_mode,
    std::string vtn_name,
    TcDriverInfoMap& driver_info) {
  PHY_FINI_IPC_LOCK(unc::tclib::TC_SUCCESS);
  //  Reject the request when system is in StandBy
  if (get_system_state() == UPPL_SYSTEM_ST_STANDBY) {
    pfc_log_error("Operation Not allowed: System is in standby\n");
    return unc::tclib::TC_FAILURE;
  }
  // Call ITC transaction handler function
  // ITC to fill driver_info map while sending response
  pfc_log_debug("Received vote request. Calling HandleVoteRequest");
  UncRespCode resp = internal_transaction_coordinator_->transaction_req()->
      HandleVoteRequest(session_id,
                        config_id,
                        config_mode,
                        driver_info);
  pfc_log_debug("HandleVoteRequest response %d", resp);
  // convert the error code returned by ITC to TC error code
  if (resp == UNC_RC_SUCCESS) {
    return unc::tclib::TC_SUCCESS;
  } else {
    return unc::tclib::TC_FAILURE;
  }
}

/**
 * @Description : This function will be called back when TC sends user
 *                Audit vote request to UPPL
 *                This is an implementation of virtual function in TCLib
 */

TcCommonRet PhysicalCore::HandleAuditVoteRequest(
    uint32_t session_id,
    uint32_t driver_id,
    string controller_id,
    TcDriverInfoMap &driver_info) {
  PHY_FINI_IPC_LOCK(unc::tclib::TC_SUCCESS);
  //  Reject the request when system is in StandBy
  if (get_system_state() == UPPL_SYSTEM_ST_STANDBY) {
    pfc_log_error("Operation Not allowed: System is in standby\n");
    return unc::tclib::TC_FAILURE;
  }
  OPEN_DB_CONNECTION_TC_REQUEST(unc::uppl::kOdbcmConnReadWriteNb);
  pfc_log_debug("Received HandleAuditVoteRequest from TC");
  // Call ITC transaction handler function
  UncRespCode resp = internal_transaction_coordinator_->audit_req()->
      HandleAuditVoteRequest(&db_conn, session_id,
                             driver_id,
                             controller_id,
                             driver_info);

  // convert the error code returned by ITC to TC error code
  if (resp == UNC_RC_SUCCESS) {
    return unc::tclib::TC_SUCCESS;
  } else {
    return unc::tclib::TC_FAILURE;
  }
}

/**
 * @Description : This function will be called back when TC sends user
 *                Global commit request to UPPL
 *                This is an implementation of virtual function in TCLib
 */

TcCommonRet PhysicalCore::HandleCommitGlobalCommit(
    uint32_t session_id,
    uint32_t config_id,
    TcConfigMode config_mode,
    std::string vtn_name,
    TcDriverInfoMap& driver_info) {
  PHY_FINI_IPC_LOCK(unc::tclib::TC_SUCCESS);
  //  Reject the request when system is in StandBy
  if (get_system_state() == UPPL_SYSTEM_ST_STANDBY) {
    pfc_log_error("Operation Not allowed: System is in standby\n");
    return unc::tclib::TC_FAILURE;
  }

  // Call ITC transaction handler function
  // ITC to fill driver_info map while sending response
  UncRespCode resp = internal_transaction_coordinator_->transaction_req()->
      HandleGlobalCommitRequest(session_id,
                                config_id,
                                config_mode,
                                driver_info);

  // convert the error code returned by ITC to TC error code
  if (resp == UNC_RC_SUCCESS) {
    return unc::tclib::TC_SUCCESS;
  } else {
    return unc::tclib::TC_FAILURE;
  }
}

/**
 * @Description : This function will be called back when TC sends user
 *                Audit Global Commit request to UPPL
 *                This is an implementation of virtual function in TCLib
 */

TcCommonRet PhysicalCore::HandleAuditGlobalCommit(
    uint32_t session_id,
    uint32_t driver_id,
    string controller_id,
    TcDriverInfoMap& driver_info,
    TcAuditResult& audit_result) {
  PHY_FINI_IPC_LOCK(unc::tclib::TC_SUCCESS);
  //  Reject the request when system is in StandBy
  if (get_system_state() == UPPL_SYSTEM_ST_STANDBY) {
    pfc_log_error("Operation Not allowed: System is in standby\n");
    return unc::tclib::TC_FAILURE;
  }
  pfc_log_debug("Received HandleAuditGlobalCommit from TC");
  // Call ITC transaction handler function
  UncRespCode resp = internal_transaction_coordinator_->audit_req()->
      HandleAuditGlobalCommit(session_id,
                              driver_id,
                              controller_id,
                              driver_info,
                              audit_result);

  // convert the error code returned by ITC to TC error code
  if (resp == UNC_RC_SUCCESS) {
    return unc::tclib::TC_SUCCESS;
  } else {
    return unc::tclib::TC_FAILURE;
  }
}

/**
 * @Description : This function will be called back when TC sends user
 *                Driver Result to UPPL
 *                This is an implementation of virtual function in TCLib
 */

TcCommonRet PhysicalCore::HandleCommitDriverResult(
    uint32_t session_id,
    uint32_t config_id,
    TcConfigMode config_mode,
    std::string vtn_name,
    TcCommitPhaseType commitphase,
    TcCommitPhaseResult driver_result) {
  PHY_FINI_IPC_LOCK(unc::tclib::TC_SUCCESS);
  //  Reject the request when system is in StandBy
  if (get_system_state() == UPPL_SYSTEM_ST_STANDBY) {
    pfc_log_error("Operation Not allowed: System is in standby\n");
    return unc::tclib::TC_FAILURE;
  }
  OPEN_DB_CONNECTION_TC_REQUEST(unc::uppl::kOdbcmConnReadWriteNb);
  // Call ITC transaction handler function
  UncRespCode resp = internal_transaction_coordinator_->transaction_req()->
      HandleDriverResult(&db_conn, session_id,
                         config_id,
                         config_mode,
                         commitphase,
                         driver_result);
  pfc_log_debug("HandleCommitDriverResult response %d", resp);
  // convert the error code returned by ITC to TC error code
  if (resp == UNC_RC_SUCCESS) {
    return unc::tclib::TC_SUCCESS;
  } else {
    return unc::tclib::TC_FAILURE;
  }
}

/**
 * @Description : This function will be called back when TC sends user
 *                Driver Result to UPPL
 *                This is an implementation of virtual function in TCLib
 */

TcCommonRet PhysicalCore::HandleAuditDriverResult(
    uint32_t session_id,
    string controller_id,
    TcCommitPhaseType commitphase,
    TcCommitPhaseResult driver_result,
    TcAuditResult& audit_result) {
  PHY_FINI_IPC_LOCK(unc::tclib::TC_SUCCESS);
  //  Reject the request when system is in StandBy
  if (get_system_state() == UPPL_SYSTEM_ST_STANDBY) {
    pfc_log_error("Operation Not allowed: System is in standby\n");
    return unc::tclib::TC_FAILURE;
  }
  OPEN_DB_CONNECTION_TC_REQUEST(unc::uppl::kOdbcmConnReadWriteNb);
  pfc_log_debug("Received HandleAuditDriverResult from TC");
  // Call ITC transaction handler function
  UncRespCode resp = internal_transaction_coordinator_->audit_req()->
      HandleAuditDriverResult(&db_conn, session_id,
                              controller_id,
                              commitphase,
                              driver_result,
                              audit_result);
  // convert the error code returned by ITC to TC error code
  if (resp == UNC_RC_SUCCESS) {
    return unc::tclib::TC_SUCCESS;
  } else {
    return unc::tclib::TC_FAILURE;
  }
}

/**
 * @Description : This function will be called back when TC sends user
 *                Audit Start request to UPPL
 *                This is an implementation of virtual function in TCLib
 */

TcCommonRet PhysicalCore::HandleAuditStart(uint32_t session_id,
                  unc_keytype_ctrtype_t ctrl_type,
                  string controller_id,
                  TcAuditType audit_type,
                  uint64_t  commit_number,  /*Latest Commit version PFC*/
                  uint64_t  commit_date,  /*Latest committed time of PFC*/
                  std::string commit_application
                  /*Application that performed commit operation*/) {
  //  If set, simplified audit will be executed
  PHY_FINI_IPC_LOCK(unc::tclib::TC_SUCCESS);
  //  Reject the request when system is in StandBy
  if (get_system_state() == UPPL_SYSTEM_ST_STANDBY) {
    pfc_log_error("Operation Not allowed: System is in standby\n");
    return unc::tclib::TC_FAILURE;
  }
  OPEN_DB_CONNECTION_TC_REQUEST(unc::uppl::kOdbcmConnReadWriteNb);
  pfc_log_debug("Received HandleAuditStart from TC");
  // Call ITC transaction handler function
  UncRespCode resp = internal_transaction_coordinator_->audit_req()->
      StartAudit(&db_conn, ctrl_type, controller_id, audit_type,
                 commit_number, commit_date, commit_application);

  // convert the error code returned by ITC to TC error code
  if (resp == UNC_RC_SUCCESS) {
    return unc::tclib::TC_SUCCESS;
  } else {
    return unc::tclib::TC_FAILURE;
  }
}

/**
 * @Description : This function will be called back when TC sends user
 *                Audit End request to UPPL
 *                This is an implementation of virtual function in TCLib
 */

TcCommonRet PhysicalCore::HandleAuditEnd(uint32_t session_id,
                                         unc_keytype_ctrtype_t ctrl_type,
                                         string controller_id,
                                         TcAuditResult audit_result) {
  PHY_FINI_IPC_LOCK(unc::tclib::TC_SUCCESS);
  //  Reject the request when system is in StandBy
  if (get_system_state() == UPPL_SYSTEM_ST_STANDBY) {
    pfc_log_error("Operation Not allowed: System is in standby\n");
    return unc::tclib::TC_FAILURE;
  }
  pfc_log_debug("Received HandleAuditEnd from TC");
  OPEN_DB_CONNECTION_TC_REQUEST(unc::uppl::kOdbcmConnReadWriteNb);
  // Call ITC transaction handler function
  UncRespCode resp = internal_transaction_coordinator_->audit_req()->
      EndAudit(&db_conn, ctrl_type,
               controller_id,
               audit_result);

  // convert the error code returned by ITC to TC error code
  if (resp == UNC_RC_SUCCESS) {
    return unc::tclib::TC_SUCCESS;
  } else {
    return unc::tclib::TC_FAILURE;
  }
}

/**
 * @Description : This function will be called back when TC sends user
 *                save configuration request to UPPL
 *                This is an implementation of virtual function in TCLib
 */

TcCommonRet PhysicalCore::HandleSaveConfiguration(uint32_t session_id,
                                                  uint64_t save_version) {
  PHY_FINI_IPC_LOCK(unc::tclib::TC_SUCCESS);
  //  Reject the request when system is in StandBy
  if (get_system_state() == UPPL_SYSTEM_ST_STANDBY) {
    pfc_log_error("Operation Not allowed: System is in standby\n");
    return unc::tclib::TC_FAILURE;
  }
  OPEN_DB_CONNECTION_TC_REQUEST(unc::uppl::kOdbcmConnReadWriteNb);
  // Call ITC transaction handler function
  UncRespCode resp = internal_transaction_coordinator_->db_config_req()->
      SaveRunningToStartUp(&db_conn);

  // convert the error code returned by ITC to TC error code
  if (resp == UNC_RC_SUCCESS) {
    return unc::tclib::TC_SUCCESS;
  } else {
    return unc::tclib::TC_FAILURE;
  }
}

/**
 * @Description : This function will be called back when TC sends user
 *                clear startup request to UPPL
 *                This is an implementation of virtual function in TCLib
 */

TcCommonRet PhysicalCore::HandleClearStartup(uint32_t session_id) {
  PHY_FINI_IPC_LOCK(unc::tclib::TC_SUCCESS);
  //  Reject the request when system is in StandBy
  if (get_system_state() == UPPL_SYSTEM_ST_STANDBY) {
    pfc_log_error("Operation Not allowed: System is in standby\n");
    return unc::tclib::TC_FAILURE;
  }
  OPEN_DB_CONNECTION_TC_REQUEST(unc::uppl::kOdbcmConnReadWriteNb);
  // Call ITC transaction handler function
  UncRespCode resp = internal_transaction_coordinator_->db_config_req()->
      ClearStartUpDb(&db_conn);

  // convert the error code returned by ITC to TC error code
  if (resp == UNC_RC_SUCCESS) {
    return unc::tclib::TC_SUCCESS;
  } else {
    return unc::tclib::TC_FAILURE;
  }
}

/**
 * @Description : This function will be called back when TC sends user
 *                Abort Candidate request to UPPL
 *                This is an implementation of virtual function in TCLib
 */

TcCommonRet PhysicalCore::HandleAbortCandidate(uint32_t session_id,
                                               uint32_t config_id,
                                               TcConfigMode config_mode,
                                               std::string vtn_name,
                                               uint64_t abort_version) {
  PHY_FINI_IPC_LOCK(unc::tclib::TC_SUCCESS);
  //  Reject the request when system is in StandBy
  if (get_system_state() == UPPL_SYSTEM_ST_STANDBY) {
    pfc_log_error("Operation Not allowed: System is in standby\n");
    return unc::tclib::TC_FAILURE;
  }
  OPEN_DB_CONNECTION_TC_REQUEST(unc::uppl::kOdbcmConnReadWriteNb);
  // Call ITC transaction handler function
  uint8_t resp = internal_transaction_coordinator_->db_config_req()->
      AbortCandidateDb(&db_conn, config_mode);

  // convert the error code returned by ITC to TC error code
  if (resp == UNC_RC_SUCCESS) {
    return unc::tclib::TC_SUCCESS;
  } else {
    return unc::tclib::TC_FAILURE;
  }
}

TcCommonRet PhysicalCore::SendControllerInfoToUPLL(
                          OdbcmConnectionHandler *db_conn
    , uint32_t dt_type) {
  Kt_Controller kt_ctr;
  vector<void *> vect_ctr_key, vect_ctr_val;
  key_ctr_t key_ctr_obj;
  memset(&key_ctr_obj, '\0', sizeof(key_ctr_t));
  vect_ctr_key.push_back(reinterpret_cast<void *>(&key_ctr_obj));

  // Getting the controller list from dt_type Database
  UncRespCode read_status = kt_ctr.ReadInternal(db_conn, vect_ctr_key,
                                                   vect_ctr_val,
                                                   dt_type,
                                                   UNC_OP_READ_SIBLING_BEGIN);
  if (read_status != UNC_RC_SUCCESS &&
                      read_status != UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE) {
    pfc_log_info("read from %d db is %d", dt_type, read_status);
    return unc::tclib::TC_FAILURE;
  }
  read_status = UNC_RC_SUCCESS;
  for (uint32_t ctrIndex = 0; ctrIndex < vect_ctr_key.size();
      ctrIndex ++) {
    key_ctr_t *ctr_key =
        reinterpret_cast<key_ctr_t*>(vect_ctr_key[ctrIndex]);
    string controller_name = (const char*)ctr_key->controller_name;
    pfc_log_debug("controller_name: %s", controller_name.c_str());
    val_ctr_st_t *obj_val_ctr =
        reinterpret_cast<val_ctr_st_t*>(vect_ctr_val[ctrIndex]);

    // Sending the Controller Update Information to Logical Layer
    UncRespCode upll_result = kt_ctr.SendUpdatedControllerInfoToUPLL(
        dt_type,
        UNC_OP_CREATE,
        UNC_KT_CONTROLLER,
        vect_ctr_key[ctrIndex],
        reinterpret_cast<void*>(&obj_val_ctr->controller));
    if (upll_result != UNC_RC_SUCCESS) {
      pfc_log_info("Failed to send the controller %s in %d DB to UPLL",
                   controller_name.c_str(), dt_type);
      read_status = upll_result;
    }
    // Release memory allocated for key struct
    delete ctr_key;
    ctr_key = NULL;
    // delete the val memory
    delete obj_val_ctr;
    obj_val_ctr = NULL;
  }
  if (read_status != UNC_RC_SUCCESS) {
    pfc_log_info("SendUpdatedControllerInfoToUPLL failed.. %d", read_status);
    return unc::tclib::TC_FAILURE;
  }
  return unc::tclib::TC_SUCCESS;
}

/**
 * @Description : This function will be called back when TC sends
 *                db recovery requests during failover
 *                This is an implementation of virtual function in TCLib
 */

TcCommonRet PhysicalCore::HandleAuditConfig(unc_keytype_datatype_t db_target,
                                            TcServiceType fail_oper,
                                            TcConfigMode config_mode,
                                            std::string vtn_name,
                                            uint64_t version) {
  PHY_FINI_IPC_LOCK(unc::tclib::TC_SUCCESS);
  //  Reject the request when system is in StandBy
  if (get_system_state() == UPPL_SYSTEM_ST_STANDBY) {
    pfc_log_error("Operation Not allowed: System is in standby\n");
    return unc::tclib::TC_FAILURE;
  }
  OPEN_DB_CONNECTION_TC_REQUEST(unc::uppl::kOdbcmConnReadWriteNb);
  pfc_log_info("AuditDB from TC dt:%d failop:%d", db_target, fail_oper);
  // Call ITC transaction handler function
  UncRespCode resp = UNC_RC_SUCCESS;
  if (fail_oper == TC_OP_CANDIDATE_COMMIT) {
    uint32_t session_id = 0;
    uint32_t config_id = 0;
    TcDriverInfoMap driver_info;

    TcCommonRet send_status =
               SendControllerInfoToUPLL(&db_conn, UNC_DT_CANDIDATE);
    if (send_status != unc::tclib::TC_SUCCESS) {
      pfc_log_debug("Send to UPLL failed with %d", send_status);
      return unc::tclib::TC_FAILURE;
    }
    send_status = SendControllerInfoToUPLL(&db_conn, UNC_DT_RUNNING);
    if (send_status != unc::tclib::TC_SUCCESS) {
      pfc_log_debug("Send to UPLL failed with %d", send_status);
      return unc::tclib::TC_FAILURE;
    }

    TransactionRequest *txn_req =
        internal_transaction_coordinator_->transaction_req();
    resp = txn_req->StartTransaction(&db_conn, session_id,
                                     config_id, config_mode);
    if (resp != UNC_RC_SUCCESS) {
      pfc_log_error("HandleAuditConfig - StartTransaction failed with %d",
                    resp);
      return unc::tclib::TC_FAILURE;
    }
    resp = txn_req->HandleVoteRequest(session_id, config_id,
                                      config_mode, driver_info);
    if (resp != UNC_RC_SUCCESS) {
      pfc_log_error("HandleAuditConfig - Vote failed with %d",
                    resp);
      return unc::tclib::TC_FAILURE;
    }

    TcCommitPhaseType phase = unc::tclib::TC_COMMIT_VOTE_PHASE;
    TcCommitPhaseResult driver_result;
    resp = txn_req->HandleDriverResult(&db_conn, session_id, config_id,
                                       config_mode, phase, driver_result);
    if (resp != UNC_RC_SUCCESS) {
      pfc_log_error("HandleAuditConfig - DriverResult VOTE PH failed with %d",
                    resp);
      return unc::tclib::TC_FAILURE;
    }

    resp = txn_req->HandleGlobalCommitRequest(session_id, config_id,
                                              config_mode, driver_info);
    if (resp != UNC_RC_SUCCESS) {
      pfc_log_error("HandleAuditConfig - GlobalCommit failed with %d",
                    resp);
      return unc::tclib::TC_FAILURE;
    }

    phase = unc::tclib::TC_COMMIT_GLOBAL_COMMIT_PHASE;
    resp = txn_req->HandleDriverResult(&db_conn, session_id, config_id,
                                       config_mode, phase, driver_result);
    if (resp != UNC_RC_SUCCESS) {
      pfc_log_error("HandleAuditConfig - DriverResult COM PH failed with %d",
                    resp);
      return unc::tclib::TC_FAILURE;
    }
    TcTransEndResult trans_res = unc::tclib::TRANS_END_SUCCESS;
    bool audit_flag = false;
    resp = txn_req->EndTransaction(
        &db_conn, session_id, config_id, config_mode, trans_res, audit_flag);
    if (resp != UNC_RC_SUCCESS) {
      pfc_log_error("HandleAuditConfig - EndTransaction COM PH failed with %d",
                    resp);
      return unc::tclib::TC_FAILURE;
    }

  } else if (fail_oper == TC_OP_CLEAR_STARTUP) {
    resp = internal_transaction_coordinator_->db_config_req()->
        ClearStartUpDb(&db_conn);
  } else if (fail_oper == TC_OP_RUNNING_SAVE) {
    // Copy running to startup
    resp = internal_transaction_coordinator_->db_config_req()->
        SaveRunningToStartUp(&db_conn);
  } else if (fail_oper == TC_OP_CANDIDATE_ABORT) {
    pfc_log_debug("Handling Failover op - CANDIDATE_ABORT");

    TcCommonRet send_status = SendControllerInfoToUPLL(
         &db_conn, UNC_DT_CANDIDATE);
    if (send_status != unc::tclib::TC_SUCCESS) {
      pfc_log_debug("Send to UPLL failed with %d", send_status);
      return unc::tclib::TC_FAILURE;
    }
    send_status = SendControllerInfoToUPLL(&db_conn, UNC_DT_RUNNING);
    if (send_status != unc::tclib::TC_SUCCESS) {
      pfc_log_debug("Send to UPLL failed with %d", send_status);
      return unc::tclib::TC_FAILURE;
    }

    resp = internal_transaction_coordinator_->db_config_req()->
        AbortCandidateDb(&db_conn, config_mode);
  } else if (fail_oper == TC_OP_USER_AUDIT ||
      fail_oper == TC_OP_DRIVER_AUDIT) {
    // Do nothing
    resp = UNC_RC_SUCCESS;
  }

  // convert the error code returned by ITC to TC error code
  if (resp == UNC_RC_SUCCESS) {
    return unc::tclib::TC_SUCCESS;
  } else {
    return unc::tclib::TC_FAILURE;
  }
}

/**
 * @Description : This function will be called back when TC sends
 *                abort during user commit transaction phase
 *                This is an implementation of virtual function in TCLib
 */

TcCommonRet PhysicalCore::HandleCommitGlobalAbort(
    uint32_t session_id,
    uint32_t config_id,
    TcConfigMode config_mode,
    std::string vtn_name,
    TcCommitOpAbortPhase fail_phase) {
  PHY_FINI_IPC_LOCK(unc::tclib::TC_SUCCESS);
  //  Reject the request when system is in StandBy
  if (get_system_state() == UPPL_SYSTEM_ST_STANDBY) {
    pfc_log_error("Operation Not allowed: System is in standby\n");
    return unc::tclib::TC_FAILURE;
  }
  // Call ITC transaction handler function
  UncRespCode resp = internal_transaction_coordinator_->transaction_req()->
      AbortTransaction(session_id,
                       config_id,
                       config_mode,
                       fail_phase);
  // convert the error code returned by ITC to TC error code
  if (resp == UNC_RC_SUCCESS) {
    return unc::tclib::TC_SUCCESS;
  } else {
    return unc::tclib::TC_FAILURE;
  }
}

/**
 * @Description : This function will be called back when TC sends
 *                abort during audit transaction phase
 *                This is an implementation of virtual function in TCLib
 */

TcCommonRet PhysicalCore::HandleAuditGlobalAbort(
    uint32_t session_id,
    unc_keytype_ctrtype_t ctrl_type,
    string controller_id,
    TcAuditOpAbortPhase operation_phase) {
  PHY_FINI_IPC_LOCK(unc::tclib::TC_SUCCESS);
  //  Reject the request when system is in StandBy
  if (get_system_state() == UPPL_SYSTEM_ST_STANDBY) {
    pfc_log_error("Operation Not allowed: System is in standby\n");
    return unc::tclib::TC_FAILURE;
  }
  // Call ITC transaction handler function
  UncRespCode resp = internal_transaction_coordinator_->audit_req()->
      HandleAuditAbort(session_id,
                       ctrl_type,
                       controller_id,
                       operation_phase);

  // convert the error code returned by ITC to TC error code
  if (resp == UNC_RC_SUCCESS) {
    return unc::tclib::TC_SUCCESS;
  } else {
    return unc::tclib::TC_FAILURE;
  }
}

/**
 * @Description : This function will be called back when TC sends
 *                cancel audit when commit/abort request arrives 
 *                with cancel flag enable
 *                This is an implementation of virtual function in TCLib
 */

TcCommonRet PhysicalCore::HandleAuditCancel(
    uint32_t session_id,
    unc_keytype_ctrtype_t driver_id,
    std::string controller_id) {
  PHY_FINI_IPC_LOCK(unc::tclib::TC_SUCCESS);
  //  Reject the request when system is in StandBy
  if (get_system_state() == UPPL_SYSTEM_ST_STANDBY) {
    pfc_log_error("Operation Not allowed: System is in standby\n");
    return unc::tclib::TC_FAILURE;
  }
  // Call ITC transaction handler function
  UncRespCode resp = internal_transaction_coordinator_->audit_req()->
      HandleAuditCancel(session_id,
                       driver_id,
                       controller_id);

  // convert the error code returned by ITC to TC error code
  if (resp == UNC_RC_SUCCESS) {
    return unc::tclib::TC_SUCCESS;
  } else {
    return unc::tclib::TC_FAILURE;
  }
}

/**
 * @Description : This function will be called back when TC sends
 *                load startup configuration request to UPPL
 *                This is an implementation of virtual function in TCLib
 */

TcCommonRet PhysicalCore::HandleSetup() {
  PHY_FINI_IPC_LOCK(unc::tclib::TC_SUCCESS);
  //  Reject the request when system is in StandBy
  if (get_system_state() == UPPL_SYSTEM_ST_STANDBY) {
    pfc_log_error("Operation Not allowed: System is in standby\n");
    return unc::tclib::TC_FAILURE;
  }
  UncRespCode resp = UNC_UPPL_RC_FAILURE;
  OPEN_DB_CONNECTION_TC_REQUEST(unc::uppl::kOdbcmConnReadWriteNb);
  // Call ITC transaction handler function
  // Copy startup to candidate and running and commit
  if (getStartupValidStatus()) {
    resp = internal_transaction_coordinator_->db_config_req()->
      LoadAndCommitStartup(&db_conn);
  } else {
    resp = internal_transaction_coordinator_->db_config_req()->
        CopyRunningtoCandidate(&db_conn);
    pfc_log_debug("Startup Config is not valid copy running"
                   " to candidate resp:%d\n", resp);
    // send success response by default
    resp = UNC_RC_SUCCESS;
  }

  // convert the error code returned by ITC to TC error code
  if (resp == UNC_RC_SUCCESS) {
    return unc::tclib::TC_SUCCESS;
  } else {
    return unc::tclib::TC_FAILURE;
  }
}

/**
 * @Description : This function will be called back when TC sends
 *                load startup configuration request to UPPL
 *                This is an implementation of virtual function in TCLib
 */

TcCommonRet PhysicalCore::HandleSetupComplete() {
  PHY_FINI_IPC_LOCK(unc::tclib::TC_SUCCESS);
  pfc_log_debug("Received SetupComplete request from TC");
  //  Reject the request when system is in StandBy
  if (get_system_state() == UPPL_SYSTEM_ST_STANDBY) {
    pfc_log_error("Operation Not allowed: System is in standby\n");
    return unc::tclib::TC_FAILURE;
  }
  OPEN_DB_CONNECTION_TC_REQUEST(unc::uppl::kOdbcmConnReadWriteNb);
  // Call ITC transaction handler function
  // Copy startup to candidate and running and commit
  UncRespCode resp = internal_transaction_coordinator_->
      system_state_change_req()->SystemStateChangeToActive(&db_conn);
  pfc_log_debug("ReturnCode of SystemStateChangeToActive %d", resp);
  // convert the error code returned by ITC to TC error code
  if (resp == UNC_RC_SUCCESS) {
    pfc_log_debug("Returning Success to Tclib");
    return unc::tclib::TC_SUCCESS;
  } else {
    pfc_log_debug("Returning failure to Tclib");
    return unc::tclib::TC_FAILURE;
  }
}

/**
 * @Description : This function returns the type of controller
 *                This is an implementation of virtual function in TCLib
 */

unc_keytype_ctrtype_t PhysicalCore::HandleGetControllerType(
    string controller_id) {
  // call util function to get controller type
  unc_keytype_ctrtype_t controller_type = UNC_CT_UNKNOWN;
  UncRespCode db_ret = UNC_RC_SUCCESS;
  OPEN_DB_CONNECTION(unc::uppl::kOdbcmConnReadWriteNb, db_ret);
  if (db_ret != UNC_RC_SUCCESS) {
    pfc_log_error("db cxn open error");
    TcLibModule* tclib_ptr = static_cast<TcLibModule*>
        (TcLibModule::getInstance(TCLIB_MODULE_NAME));
    if (tclib_ptr != NULL) {
      tclib_ptr->TcLibWriteControllerInfo(controller_id.c_str(),
                                        UNC_RC_INTERNAL_ERR,
                                        0);
    }
    return UNC_CT_UNKNOWN;
  }
  UncRespCode resp = PhyUtil::get_controller_type(&db_conn,
                                                     controller_id,
                                                     controller_type,
                                                     UNC_DT_RUNNING);
  // convert the error code returned by ITC to TC error code
  if (resp == UNC_RC_SUCCESS) {
    return controller_type;
  } else if (resp == UNC_UPPL_RC_ERR_DB_GET) {
    pfc_log_error("Unknown controller-id");
    TcLibModule* tclib_ptr = static_cast<TcLibModule*>
        (TcLibModule::getInstance(TCLIB_MODULE_NAME));
    if (tclib_ptr != NULL) {
    tclib_ptr->TcLibWriteControllerInfo(controller_id.c_str(),
                                        UNC_RC_NO_SUCH_INSTANCE,
                                        0);
    }
    return UNC_CT_UNKNOWN;
  } else {
    pfc_log_error("DB access error");
    TcLibModule* tclib_ptr = static_cast<TcLibModule*>
        (TcLibModule::getInstance(TCLIB_MODULE_NAME));
    if (tclib_ptr != NULL) {
      tclib_ptr->TcLibWriteControllerInfo(controller_id.c_str(),
                                        UNC_RC_INTERNAL_ERR,
                                        0);
    }
    return UNC_CT_UNKNOWN;
  }
}

/**
 * @Description : This function sends CONROLLER_DISCONNECT alarm to node
 *                manager
 *                This function will be called from Kt_Controller class when it
 *                receives CONTROLLER notification from driver with oper_status
 *                as down
 */

UncRespCode PhysicalCore::SendControllerDisconnectAlarm(
                                    string controller_id) {
  if (system_transit_state_ == true &&
     get_system_state() == UPPL_SYSTEM_ST_ACTIVE) {
    pfc_log_info("System is in active->standby transition,"
                 " so don't send disconnect alarm");
    return UNC_RC_SUCCESS;
  }
  if (get_system_state() == UPPL_SYSTEM_ST_STANDBY) {
    pfc_log_info("System is in standby");
    pfc_log_info("ControllerDisconnect alarm not sent to node manager");
    return UNC_RC_SUCCESS;
  }
  // By alarm definition,alarm category =1 for connect/disconnect alarm
  std::string alarm_category = "1";
  std::string map_key = "";
  map_key.append(controller_id).append("#").append(alarm_category);
  std::map<std::string, bool>::iterator alarm_map_iter;
  alarm_map_iter = alarm_status_map_.find(map_key);
  if (alarm_map_iter != alarm_status_map_.end()) { /*there is an entry found*/
    // Do nothing in this case, just return
    pfc_log_debug("controller entry is available in alarm_status_map_"
                                 " - NOT send alarm again");
    return UNC_UPPL_RC_ERR_INSTANCE_EXISTS;
  }
  alarm_status_map_.insert(std::pair<std::string, bool> (map_key, true));
  pfc_log_debug("Controller inserted into alarm_status_map_:%s",
                map_key.c_str());
  string vtn_name = "";
  const std::string& alm_msg = "Controller disconnected - " + controller_id;
  const std::string& alm_msg_summary = "Controller disconnected";
  pfc::alarm::alarm_info_with_key_t* data =
      new pfc::alarm::alarm_info_with_key_t;
  data->alarm_class = pfc::alarm::ALM_WARNING;
  data->apl_No = UNCCID_PHYSICAL;
  data->alarm_category = 1;
  data->alarm_key_size = controller_id.length();
  data->alarm_key = new uint8_t[controller_id.length()+1];
  memcpy(data->alarm_key,
         controller_id.c_str(),
         controller_id.length()+1);
  data->alarm_kind = 1;

  pfc::alarm::alarm_return_code_t ret = pfc::alarm::pfc_alarm_send_with_key(
      vtn_name,
      alm_msg,
      alm_msg_summary,
      data, fd);
  if (ret != pfc::alarm::ALM_OK) {
    delete []data->alarm_key;
    delete data;
    return UNC_UPPL_RC_ERR_ALARM_API;
  }
  delete []data->alarm_key;
  delete data;
  pfc_log_info("Sent Controller Disconnected alarm - %s",
               controller_id.c_str());
  return UNC_RC_SUCCESS;
}

/**
 * @Description : This function sends CONROLLER_CONNECT alarm to node manager
 *                This is a clearance alarm for CONTROLLER_DISCONNECT alarm
 *                This function will be called from Kt_Controller class when it
 *                receives CONTROLLER notification from driver with oper_status
 *                as up
 */

UncRespCode PhysicalCore::SendControllerConnectAlarm(string controller_id) {
  pfc_log_debug("controller_name:%s", controller_id.c_str());
  if (get_system_state() == UPPL_SYSTEM_ST_STANDBY) {
    // system is in standby
    pfc_log_info("System is in standby");
    pfc_log_info("ControllerConnect alarm not sent to node manager");
    return UNC_RC_SUCCESS;
  }
  std::string alarm_category = "1";  // as per alarm definition,
                        //  alarm category is 1 for connect/disconnect alarm
  std::map<std::string, bool>::iterator alarm_map_iter;
  std::string map_key = "";
  map_key.append(controller_id).append("#").append(alarm_category);
  alarm_map_iter = alarm_status_map_.find(map_key);
  if (alarm_map_iter == alarm_status_map_.end()) { /*there is no entry*/
    // Do nothing, return
    pfc_log_debug("no controller entry found in alarm_status_map_ "
                               "- NOT send alarm");
    return UNC_RC_SUCCESS;
  }
  string vtn_name = "";
  const std::string& alm_msg = "Controller connected - " + controller_id;
  const std::string& alm_msg_summary = "Controller connected";
  pfc::alarm::alarm_info_with_key_t* data =
      new pfc::alarm::alarm_info_with_key_t;
  data->alarm_class = pfc::alarm::ALM_NOTICE;
  data->apl_No = UNCCID_PHYSICAL;
  data->alarm_category = 1;
  data->alarm_key_size = controller_id.length();
  data->alarm_key = new uint8_t[controller_id.length()+1];
  memcpy(data->alarm_key,
         controller_id.c_str(),
         controller_id.length()+1);
  data->alarm_kind = 0;

  pfc::alarm::alarm_return_code_t ret = pfc::alarm::pfc_alarm_send_with_key(
      vtn_name,
      alm_msg,
      alm_msg_summary,
      data, fd);
  if (ret != pfc::alarm::ALM_OK) {
    pfc_log_debug("return UNC_UPPL_RC_ERR_ALARM_API");
    delete []data->alarm_key;
    delete data;
    return UNC_UPPL_RC_ERR_ALARM_API;
  }
  pfc_log_debug("Sending alarm successfully");
  delete []data->alarm_key;
  delete data;
  if (!alarm_status_map_.empty()) {
    if (alarm_map_iter != alarm_status_map_.end()) { /*there is entry*/
      pfc_log_debug("Clearing an element from map:%s",
                    alarm_map_iter->first.c_str());
      alarm_status_map_.erase(alarm_map_iter);
    }
  }
  pfc_log_info("Sent Controller Connected alarm - %s",
               controller_id.c_str());
  return UNC_RC_SUCCESS;
}


/**
 * @Description : This function sends EVENT_HANDLING_FAILURE notice to
 *                node manager
 *                This function will be called from ipc_connection_manager 
 *                when send event is failed.
 */
UncRespCode
PhysicalCore::SendEventPostFailureNotice(string controller_id,
                                            string event_details) {
  if (system_transit_state_ == true &&
     get_system_state() == UPPL_SYSTEM_ST_ACTIVE) {
    pfc_log_info("System is in active->standby transition,"
                 " so don't send event post failure notice");
    return UNC_RC_SUCCESS;
  }
  if (get_system_state() == UPPL_SYSTEM_ST_STANDBY) {
    pfc_log_info("SBY node, post event failure notice"
                 " not sent to node manager");
    return UNC_RC_SUCCESS;
  }
  std::string alarm_category = "3";
  string vtn_name = "";
  const std::string& alm_msg =
      "Event Post failure - " + event_details;
  const std::string& alm_msg_summary =
      "Event Post failure notice";
  pfc::alarm::alarm_info_with_key_t* data =
      new pfc::alarm::alarm_info_with_key_t;
  data->alarm_class = pfc::alarm::ALM_WARNING;
  data->apl_No = UNCCID_PHYSICAL;
  data->alarm_category = 3;
  data->alarm_key_size = controller_id.length();
  data->alarm_key = new uint8_t[controller_id.length()+1];
  memcpy(data->alarm_key, controller_id.c_str(), controller_id.length()+1);
  data->alarm_kind = 2;  //  in case of North Bound Event handling failure

  pfc::alarm::alarm_return_code_t ret = pfc::alarm::pfc_alarm_send_with_key(
      vtn_name,
      alm_msg,
      alm_msg_summary,
      data, fd);
  if (ret != pfc::alarm::ALM_OK) {
    delete []data->alarm_key;
    delete data;
    return UNC_UPPL_RC_ERR_ALARM_API;
  }
  delete []data->alarm_key;
  delete data;
  pfc_log_debug("Sent post event operation Failure notice - %s , %s",
               controller_id.c_str(), event_details.c_str());
  return UNC_RC_SUCCESS;
}

/**
 * @Description : This function sends duplicate controller id notice to
 *                node manager,if similar actual id is found for different
 *                controllers.
 *                This function will be called from Kt_Controller 
 *                when duplicate controller id is received. 
 */
UncRespCode
PhysicalCore::SendDuplicateControllerIdAlarm(string dup_ctr_id,
                           string actual_id, string orig_ctr_id) {
  if (system_transit_state_ == true &&
     get_system_state() == UPPL_SYSTEM_ST_ACTIVE) {
    pfc_log_info("System is in active->standby transition,"
                 " so don't send duplicate controller id notice");
    return UNC_RC_SUCCESS;
  }
  if (get_system_state() == UPPL_SYSTEM_ST_STANDBY) {
    pfc_log_info("SBY node, duplicate controller id notice"
                 " not sent to node manager");
    return UNC_RC_SUCCESS;
  }
  std::string alarm_category = "4";
  string vtn_name = "";
  const std::string& alm_msg =
      "Duplicate Actual ID exists: Duplicate controller Id-" + dup_ctr_id +
      ", Actual Id-" + actual_id + ", Original controller Id-" + orig_ctr_id;
  const std::string& alm_msg_summary =
      "Duplicate Actual ID exists";
  pfc::alarm::alarm_info_with_key_t* data =
      new pfc::alarm::alarm_info_with_key_t;
  data->alarm_class = pfc::alarm::ALM_NOTICE;
  data->apl_No = UNCCID_PHYSICAL;
  data->alarm_category = 4;
  data->alarm_key_size = dup_ctr_id.length();
  data->alarm_key = new uint8_t[dup_ctr_id.length()+1];
  memcpy(data->alarm_key, dup_ctr_id.c_str(), dup_ctr_id.length()+1);
  data->alarm_kind = 2;  //  in case of receiving duplicate controller id

  pfc::alarm::alarm_return_code_t ret = pfc::alarm::pfc_alarm_send_with_key(
      vtn_name,
      alm_msg,
      alm_msg_summary,
      data, fd);
  if (ret != pfc::alarm::ALM_OK) {
    delete []data->alarm_key;
    delete data;
    return UNC_UPPL_RC_ERR_ALARM_API;
  }
  delete []data->alarm_key;
  delete data;
  pfc_log_debug("Sent duplicate controller id received alarm - %s",
               dup_ctr_id.c_str());
  return UNC_RC_SUCCESS;
}

/**
 * @Description : This function sends EVENT_HANDLING FAILURE alarm to
 *                node manager
 *                This function will be called from kt classes when event
 *                handling fails
 */
UncRespCode
PhysicalCore::SendEventHandlingFailureAlarm(string controller_id,
                                            string event_details) {
  if (system_transit_state_ == true &&
     get_system_state() == UPPL_SYSTEM_ST_ACTIVE) {
    pfc_log_info("System is in active->standby transition,"
                 " so don't send failure alarm");
    return UNC_RC_SUCCESS;
  }
  if (get_system_state() == UPPL_SYSTEM_ST_STANDBY) {
    pfc_log_info("System is in standby");
    pfc_log_info("Event Handling Failure alarm not sent to node manager");
    return UNC_RC_SUCCESS;
  }

  std::string alarm_category = "3";  // as per alarm definition,alarm category
                           // is 3 for event handling success/failure alarm
  std::string map_key = "";
  map_key.append(controller_id).append("#").append(alarm_category);
  std::map<std::string, bool>::iterator alarm_map_iter;
  alarm_map_iter = alarm_status_map_.find(map_key);
  if (alarm_map_iter != alarm_status_map_.end()) { /*there is an entry found*/
    // Do nothing in this case, just return
    pfc_log_debug("controller entry is available in alarm_status_map_"
                                 " - NOT send alarm again");
    return UNC_UPPL_RC_ERR_INSTANCE_EXISTS;
  }
  alarm_status_map_.insert(std::pair<std::string, bool> (map_key, true));

  string vtn_name = "";
  const std::string& alm_msg =
      "Event Handling failure - " + event_details;
  const std::string& alm_msg_summary =
      "Event Handling failure";
  pfc::alarm::alarm_info_with_key_t* data =
      new pfc::alarm::alarm_info_with_key_t;
  data->alarm_class = pfc::alarm::ALM_WARNING;
  data->apl_No = UNCCID_PHYSICAL;
  data->alarm_category = 3;
  data->alarm_key_size = controller_id.length();
  data->alarm_key = new uint8_t[controller_id.length()+1];
  memcpy(data->alarm_key,
         controller_id.c_str(),
         controller_id.length()+1);
  data->alarm_kind = 1;

  pfc::alarm::alarm_return_code_t ret = pfc::alarm::pfc_alarm_send_with_key(
      vtn_name,
      alm_msg,
      alm_msg_summary,
      data, fd);
  if (ret != pfc::alarm::ALM_OK) {
    delete []data->alarm_key;
    delete data;
    return UNC_UPPL_RC_ERR_ALARM_API;
  }
  delete []data->alarm_key;
  delete data;
  pfc_log_debug("Sent Event Handling Failure alarm - %s , %s",
               controller_id.c_str(), event_details.c_str());
  return UNC_RC_SUCCESS;
}

/**
 * @Description : This function sends EVENT_HANDLING_SUCCESS alarm to
 *                node manager
 *                This is a clearance alarm for EVENT_HANDLING_FAILURE alarm
 */

UncRespCode
PhysicalCore::SendEventHandlingSuccessAlarm(string controller_id,
                                            string event_details) {
  if (get_system_state() == UPPL_SYSTEM_ST_STANDBY) {
    // system is in standby
    pfc_log_info("System is in standby");
    pfc_log_info("Event handling Success alarm not sent to node manager");
    return UNC_RC_SUCCESS;
  }

  std::string alarm_category = "3";  // as per alarm definition,
           //  alarm category is 3 for event handling success/failure alarm
  std::map<std::string, bool>::iterator alarm_map_iter;
  std::string map_key = "";
  map_key.append(controller_id).append("#").append(alarm_category);
  alarm_map_iter = alarm_status_map_.find(map_key);
  if (alarm_map_iter == alarm_status_map_.end()) { /*there is no entry*/
    // Do nothing, return
    pfc_log_debug("no controller entry found in alarm_status_map_ "
                               "- NOT send alarm");
    return UNC_RC_SUCCESS;
  }

  string vtn_name = "";
  const std::string& alm_msg = "Event handling success - " + event_details;
  const std::string& alm_msg_summary =
      "Event handling success";
  pfc::alarm::alarm_info_with_key_t* data =
      new pfc::alarm::alarm_info_with_key_t;
  data->alarm_class = pfc::alarm::ALM_NOTICE;
  data->apl_No = UNCCID_PHYSICAL;
  data->alarm_category = 3;
  data->alarm_key_size = controller_id.length();
  data->alarm_key = new uint8_t[controller_id.length()+1];
  memcpy(data->alarm_key,
         controller_id.c_str(),
         controller_id.length()+1);
  data->alarm_kind = 0;
  pfc::alarm::alarm_return_code_t ret = pfc::alarm::pfc_alarm_send_with_key(
      vtn_name,
      alm_msg,
      alm_msg_summary,
      data, fd);
  if (ret != pfc::alarm::ALM_OK) {
    delete []data->alarm_key;
    delete data;
    return UNC_UPPL_RC_ERR_ALARM_API;
  }
  delete []data->alarm_key;
  delete data;
  if (!alarm_status_map_.empty()) {
    if (alarm_map_iter != alarm_status_map_.end()) { /*there is no entry*/
      pfc_log_debug("Clearing an element from map:%s",
                    alarm_map_iter->first.c_str());
      alarm_status_map_.erase(alarm_map_iter);
    }
  }
  pfc_log_info("Sent Event Handling Success alarm - %s , %s",
               controller_id.c_str(), event_details.c_str());
  return UNC_RC_SUCCESS;
}

/**
 * @Description : This function sends DRIVER AUDIT SUCCESS alarm to node manager
 *                This is a recovery alarm for AUDIT FAILURE alarm
 *                This function will be called from itc_audit_request when it
 *                receives EndAudit notification from TC with audit status success
 */
UncRespCode PhysicalCore::SendControllerAuditSuccessAlarm(
                                              string controller_id) {
  pfc_log_debug("controller_name:%s", controller_id.c_str());
  if (get_system_state() == UPPL_SYSTEM_ST_STANDBY) {
    // system is in standby
    pfc_log_info("System is in standby - audit is restricted");
    return UNC_RC_SUCCESS;
  }
  std::string alarm_category = "2";  // as per alarm definition,
                        //  alarm category is 2 for audit fail/success alarm
  std::map<std::string, bool>::iterator alarm_map_iter;
  std::string map_key = "";
  map_key.append(controller_id).append("#").append(alarm_category);
  alarm_map_iter = alarm_status_map_.find(map_key);
  if (alarm_map_iter == alarm_status_map_.end()) { /*there is no entry*/
    // Do nothing, return
    pfc_log_debug("no occurence alarm entry found in alarm_status_map_ "
                               "- NOT send recovery alarm");
    return UNC_RC_SUCCESS;
  }

  string vtn_name = "";
  const std::string& alm_msg = "Controller audit success.Controller Id - "+\
                                controller_id;
  const std::string& alm_msg_summary = "Controller audit success";
  pfc::alarm::alarm_info_with_key_t* data =
      new pfc::alarm::alarm_info_with_key_t;
  data->alarm_class = pfc::alarm::ALM_NOTICE;
  data->apl_No = UNCCID_PHYSICAL;
  data->alarm_category = 2;
  data->alarm_key_size = controller_id.length();
  data->alarm_key = new uint8_t[controller_id.length()+1];
  memcpy(data->alarm_key,
         controller_id.c_str(),
         controller_id.length()+1);
  data->alarm_kind = 0;
  pfc::alarm::alarm_return_code_t ret = pfc::alarm::pfc_alarm_send_with_key(
      vtn_name,
      alm_msg,
      alm_msg_summary,
      data, fd);
  if (ret != pfc::alarm::ALM_OK) {
    delete []data->alarm_key;
    delete data;
    return UNC_UPPL_RC_ERR_ALARM_API;
  }
  delete []data->alarm_key;
  delete data;
  if (!alarm_status_map_.empty()) {
    if (alarm_map_iter != alarm_status_map_.end()) { /*there is entry*/
      pfc_log_debug("Clearing an element from map:%s",
                    alarm_map_iter->first.c_str());
      alarm_status_map_.erase(alarm_map_iter);
    }
  }
  pfc_log_info("Sent Controller Audit Success alarm - %s",
                   controller_id.c_str());
  return UNC_RC_SUCCESS;
}
/**
 * @Description: This function sends DRIVER AUDIT FAILURE alarm to node manager
 *             This is a occurence alarm for AUDIT FAILURE
 *             This function will be called from itc_audit_request when it
 *             receives EndAudit notification from TC with audit status failure
 */
UncRespCode PhysicalCore::SendControllerAuditFailureAlarm(
                                                string controller_id) {
  // Do we need the same here? Pls test and confirm
  pfc_log_debug("controller_name:%s", controller_id.c_str());
  if (get_system_state() == UPPL_SYSTEM_ST_STANDBY) {
    // system is in standby
    pfc_log_info("System is in standby - audit is restricted");
    return UNC_RC_SUCCESS;
  }
  std::string alarm_category = "2";  // as per alarm definition,
                        //  alarm category is 2 for audit fail/success alarm
  std::map<std::string, bool>::iterator alarm_map_iter;
  std::string map_key = "";
  map_key.append(controller_id).append("#").append(alarm_category);
  alarm_map_iter = alarm_status_map_.find(map_key);
  if (alarm_map_iter != alarm_status_map_.end()) { /*there is an entry found*/
    //  Do nothing in this case, just return
    pfc_log_debug("occurence alarm entry is available in alarm_status_map_"
                  " - NOT send occurence alarm again");
    return UNC_UPPL_RC_ERR_INSTANCE_EXISTS;
  }
  alarm_status_map_.insert(std::pair<std::string, bool> (map_key, true));
  pfc_log_debug("Controller inserted into alarm_status_map_:%s",
                  map_key.c_str());
  string vtn_name = "";
  const std::string& alm_msg = "Controller audit failure.Controller Id - " +\
                              controller_id;
  const std::string& alm_msg_summary = "Controller audit failure";
  pfc::alarm::alarm_info_with_key_t* data =
        new pfc::alarm::alarm_info_with_key_t;
  data->alarm_class = pfc::alarm::ALM_WARNING;
  data->apl_No = UNCCID_PHYSICAL;
  data->alarm_category = 2;
  data->alarm_key_size = controller_id.length();
  data->alarm_key = new uint8_t[controller_id.length()+1];
  memcpy(data->alarm_key,
             controller_id.c_str(),
                    controller_id.length()+1);
  data->alarm_kind = 1;
  pfc::alarm::alarm_return_code_t ret = pfc::alarm::pfc_alarm_send_with_key(
          vtn_name,
          alm_msg,
          alm_msg_summary,
          data, fd);
  if (ret != pfc::alarm::ALM_OK) {
    delete []data->alarm_key;
    delete data;
    return UNC_UPPL_RC_ERR_ALARM_API;
  }
  delete []data->alarm_key;
  delete data;
  pfc_log_info("Sent Controller Audit Failure alarm - %s",
                   controller_id.c_str());
  return UNC_RC_SUCCESS;
}

/** RaiseEventHandlingAlarm()
 * * @Description : This function checks whether event handling failure alarm
 * is already raised for a controller
 * * * @param[in] : controller_name
 * * * @return    : UNC_RC_SUCCESS or UNC_UPPL_RC_FAILURE
 * */
UncRespCode PhysicalCore::RaiseEventHandlingAlarm(string controller_name) {
  if (find(event_handling_controller_alarm_.begin(),
           event_handling_controller_alarm_.end(),
           controller_name) != event_handling_controller_alarm_.end()) {
    pfc_log_info("Alarm already raised for this controller id");
    return UNC_UPPL_RC_FAILURE;
  }
  pfc_log_debug("Adding the controller_id in the alarm_raised vector");
  event_handling_controller_alarm_.push_back(controller_name);
  return UNC_RC_SUCCESS;
}

/** ClearEventHandlingAlarm
 * * @Description : This function checks whether event handling success alarm
 * is already raised for a controller
 * * * @param[in] : controller_name
 * * * @return    : UNC_RC_SUCCESS or UNC_UPPL_RC_FAILURE
 * */
UncRespCode PhysicalCore::ClearEventHandlingAlarm(string controller_name) {
  vector<string>::iterator alarm_raised_iter =
      find(event_handling_controller_alarm_.begin(),
           event_handling_controller_alarm_.end(),
           controller_name);
  if (alarm_raised_iter != event_handling_controller_alarm_.end()) {
    pfc_log_debug(" Failure Alarm already raised for this controller id");
    alarm_raised_iter =
               event_handling_controller_alarm_.erase(alarm_raised_iter);
    pfc_log_debug("Clearing the alarm");
    return UNC_RC_SUCCESS;
  }
  pfc_log_debug("Ignoring the clearance alarm");
  return UNC_UPPL_RC_FAILURE;
}

/** 
 * @Description : This function checks whether controller_name key
 * is exists in the alarm_status_map_ and remove that.
 * (it checks alarm_category 1 and 3)
 * @param[in] : controller_name
 * @return    : UNC_RC_SUCCESS or UNC_UPPL_RC_FAILURE
 */

UncRespCode PhysicalCore::remove_ctr_from_alarm_status_map(
         std::string controller_name, std::string alarm_category) {
  std::string map_key = "";
  map_key.append(controller_name).append("#").append(alarm_category);
  std::map<std::string, bool>::iterator alarm_map_iter;
  alarm_map_iter = alarm_status_map_.find(map_key);
  if (alarm_map_iter != alarm_status_map_.end()) {
    // send recovery alarm and delete the entry
    if (alarm_category.compare("1") == 0)
      SendControllerConnectAlarm(controller_name);
    else if (alarm_category.compare("2") == 0)
      SendControllerAuditSuccessAlarm(controller_name);
    else if (alarm_category.compare("3") == 0)
      SendEventHandlingSuccessAlarm(controller_name,
                                        "KT_CONTROLLER - DELETE/AUDIT");
    else
      pfc_log_error("alarm category is not matched in alarm_status_map");

    pfc_log_debug("controller key is deleted in alarm_status_map_");
    return UNC_RC_SUCCESS;
  }
  pfc_log_debug("controller key is not found alarm_status_map_");
  return UNC_UPPL_RC_FAILURE;
}

/** 
 * @Description : This function get and return the capa module instance
 * @param[in] : None
 * @return    : CapaIntf
 */

unc::capa::CapaIntf *PhysicalCore::GetCapaInterface() {
  unc::capa::CapaIntf *capa = reinterpret_cast<unc::capa::CapaIntf *>(
      pfc::core::Module::getInstance("capa"));
  if (capa == NULL) {
    pfc_log_error("CapaModule is not found in UPPL daemon");
  }
  return capa;
}
/** 
 * @Description : This function get and return the startup config validity
 * @param[in] : None
 * @return    : StartupValidity status
 */
uint16_t PhysicalCore::getStartupValidStatus() {
  TcLibModule* tclib_ptr = static_cast<TcLibModule*>
    (TcLibModule::getInstance(TCLIB_MODULE_NAME));
  return tclib_ptr->IsStartupConfigValid();
}

}  // namespace uppl
}  // namespace unc

