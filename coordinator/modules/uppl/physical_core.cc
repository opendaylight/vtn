/*
 * Copyright (c) 2012-2013 NEC Corporation
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
#include <alarm.hh>
#include <clstat_api.h>
#include "physical_core.hh"
#include "physicallayer.hh"
#include "tclib_module.hh"
using unc::tclib::TcApiCommonRet;
using unc::tclib::TcLibModule;

// Configuration file handle
extern pfc_cfdef_t ctr_cap_cfdef;

// Alarm FD
static int32_t fd;

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

UpplReturnCode PhysicalCore::InitializePhysical() {
  // initialize the class member data
  // Create new internal transaction coordinator object
  internal_transaction_coordinator_ = new InternalTransactionCoordinator();

  // Read config from file
  UpplReturnCode ret = ReadConfigFile();
  pfc_log_debug("Read Configuration return %d", ret);
  if (ret != UPPL_RC_SUCCESS) {
    return UPPL_RC_ERR_CONF_FILE_READ;
  }

  // Call function to load static capability file
  ret = ReadCtrlrStaticCapability();
  pfc_log_debug("Static Capability return %d", ret);
  if (ret != UPPL_RC_SUCCESS)  {
    return UPPL_RC_ERR_CAP_FILE_READ;
  }

  // initialize alarm object
  pfc::alarm::alarm_return_code_t alarm_ret = pfc::alarm::ALM_OK;
  alarm_ret = pfc::alarm::pfc_alarm_initialize(&fd);
  pfc_log_debug("PFC Alarm intialization return %d", alarm_ret);
  if (alarm_ret != pfc::alarm::ALM_OK) {
    return UPPL_RC_ERR_ALARM_API;
  } else {
    // clear all alarms
    pfc::alarm::pfc_alarm_clear(0);
  }

  pfc_bool_t event_ret = RegisterStateHandlers();
  pfc_log_debug("State handlers registration return %d", event_ret);
  if (event_ret == PFC_TRUE) {
    ret =  UPPL_RC_SUCCESS;
  } else {
    ret = UPPL_RC_FAILURE;
  }

  TcApiCommonRet ret_code = unc::tclib::TC_API_COMMON_SUCCESS;
  TcLibModule* tclib_ptr = static_cast<TcLibModule*>
  (TcLibModule::getInstance(TCLIB_MODULE_NAME));
  if (tclib_ptr != NULL) {
    pfc_log_info("Calling TcLibRegisterHandler");
    ret_code = tclib_ptr->TcLibRegisterHandler(this);
    pfc_log_info("TcLibRegisterHandler returned %u.", ret_code);
    if (ret_code != unc::tclib::TC_API_COMMON_SUCCESS) {
      ret = UPPL_RC_FAILURE;
    }
  } else {
    ret = UPPL_RC_FAILURE;
  }
  return ret;
}

/**
 * @Description : This function finalizes physical core members
 */

UpplReturnCode PhysicalCore::FinalizePhysical() {
  // Finalize the class member data
  ctr_cap_map_.clear();
  // IPC Event handler removal is taken care by ipc library itself during fini
  /* Cancel the event Subscription from driver
  UpplReturnCode ret = CancelEventSubscripInDriver(); */
  UpplReturnCode ret = UPPL_RC_FAILURE;
  pfc_bool_t event_ret = UnRegisterStateHandlers();
  if (event_ret == PFC_TRUE) {
    ret =  UPPL_RC_SUCCESS;
  } else {
    ret = UPPL_RC_FAILURE;
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

UpplReturnCode PhysicalCore::ReadConfigFile() {
  // Config file reading
  PhysicalLayer* physical_layer = PhysicalLayer::get_instance();
  if (physical_layer == NULL) {
    return UPPL_RC_ERR_FATAL_RESOURCE_ALLOCATION;
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
  return UPPL_RC_SUCCESS;
}

/**
 * @Description : This function reads controller capability config and fills the
 *                capability map
 */

UpplReturnCode PhysicalCore::ReadCtrlrStaticCapability() {
  // Read the capability info from the static config file
  string conf_file_path = string(UNC_MODULEDIR) +
      string("/uppl_ctr_capability.conf");

  string version;
  attribute_struct attr_var;
  cap_value_struct vals;
  cap_key_struct keys;
  vector<pfc::core::ConfBlock> cObjs;

  pfc::core::ConfHandle conf_handle(conf_file_path, &ctr_cap_cfdef);

  pfc::core::ConfBlock kt_map_blk(conf_handle, "kt_cap_map_list");

  int kKtCapMapListSz = kt_map_blk.arraySize("kt_map_name");
  for (int it = 0; it < kKtCapMapListSz; ++it) {
    pfc::core::ConfBlock kt_cap_obj(conf_handle,
                                    "kt_cap",
                                    kt_map_blk.getStringAt("kt_map_name", it,
                                                           0));
    cObjs.push_back(kt_cap_obj);
  }

  for (uint32_t iter = 0; iter < cObjs.size(); ++iter) {
    int kVersArraySize = cObjs[iter].arraySize("version_supported");
    for (int idx = 0; idx < kVersArraySize; ++idx) {
      version = cObjs[iter].getStringAt("version_supported", idx, 0);
      ControllerVersion ctr_obj(version);
      int kAttribArraySize = cObjs[iter].arraySize("attribute_name");
      vals.attrs.clear();
      for (int i =0; i < kAttribArraySize; ++i) {
        attr_var.attr_name = cObjs[iter].getStringAt("attribute_name", i, 0);
        vals.attrs.push_back(attr_var);
      }
      vals.scalability_num = cObjs[iter].getUint32("scalability_num", 0);
      keys.key_type = cObjs[iter].getUint32("key_type", 0);
      ctr_cap_map_[ctr_obj].insert(std::make_pair(keys, vals));
    }
  }
  return UPPL_RC_SUCCESS;
}

/**
 * @Description : This function validates key type controller capability map
 */

UpplReturnCode PhysicalCore::ValidateKeyTypeInCtrlrCap(string version,
                                                       uint32_t key_type) {
  /* check whether the version provided in request is available in capability
   * controller map
   */
  PhysicalLayer* physical_layer = PhysicalLayer::get_instance();
  if (physical_layer == NULL) {
    return UPPL_RC_ERR_FATAL_RESOURCE_ALLOCATION;
  }
  ControllerVersion obj(version);
  cap_key_struct keystructvar;
  keystructvar.key_type = key_type;

  // map<ControllerVersion, map<cap_key_struct, cap_value_struct > >::iterator
  // iter_cap_map = ctr_cap_map_.find(obj);
  cap_iter iter_cap_map = GetVersionIterator(obj);

  if (iter_cap_map != ctr_cap_map_.end()) {
    pfc_log_debug("Controller Version = %s\n",
                  (iter_cap_map->first.version_).c_str());
    map<cap_key_struct, cap_value_struct > key_type_map;
    // Check the key type is available in the map
    key_type_map = iter_cap_map->second;
    map<cap_key_struct, cap_value_struct >::iterator
    iter_key_type_map_ = key_type_map.find(keystructvar);

    if (iter_key_type_map_ != key_type_map.end()) {
      pfc_log_debug("Found key_type is %d",
                    (iter_key_type_map_)->first.key_type);
      return UPPL_RC_SUCCESS;
    } else {
      pfc_log_info("Key Type not supported");
      return UPPL_RC_ERR_KEYTYPE_NOT_SUPPORTED;
    }
  } else {
    pfc_log_info("Version not supported");
    return UPPL_RC_ERR_VERSION_NOT_SUPPORTED;
  }
  return UPPL_RC_SUCCESS;
}

/**
 * @Description : This function checks the attribute name in associated key
 *                type is available in controller capability map
 */

UpplReturnCode PhysicalCore::ValidateAttribInCtrlrCap(string version,
                                                      uint32_t key_type,
                                                      string attribute_name) {
  UpplReturnCode ret = ValidateKeyTypeInCtrlrCap(version, key_type);
  PhysicalLayer* physical_layer = PhysicalLayer::get_instance();
  if (physical_layer == NULL) {
    return UPPL_RC_ERR_FATAL_RESOURCE_ALLOCATION;
  }
  pfc_log_debug("ValidateKeyTypeInCtrlrCap ret = %d\n", ret);
  if (ret != UPPL_RC_SUCCESS) {
    pfc_log_debug("ValidateKeyTypeInCtrlrCap ret = %d\n", ret);
    return ret;
  }
  ControllerVersion obj(version);
  cap_iter iter_cap_map = GetVersionIterator(obj);
  pfc_log_debug("Controller Version = %s\n",
                (iter_cap_map->first.version_).c_str());
  map<cap_key_struct, cap_value_struct > key_type_map;
  cap_key_struct keystructvar;
  keystructvar.key_type = key_type;
  key_type_map = (iter_cap_map)->second;
  map<cap_key_struct, cap_value_struct >::iterator iter_key_type_map_ =
      key_type_map.find(keystructvar);
  if (iter_key_type_map_ != key_type_map.end()) {
    pfc_log_debug("Found key_type is %d",
                  (iter_key_type_map_)->first.key_type);
    cap_value_struct vs = (iter_key_type_map_)->second;
    pfc_log_debug("vector size is %zd", (vs.attrs).size());
    pfc_log_debug("Attribute_name to search is %s", attribute_name.c_str());
    for (uint32_t idx = 0; idx < (vs.attrs).size(); ++idx) {
      if ((vs.attrs[idx]).attr_name == attribute_name) {
        return UPPL_RC_SUCCESS;
      }
    }
  }
  pfc_log_info("attribute not supported");
  return UPPL_RC_ERR_ATTRIBUTE_NOT_SUPPORTED;
}

/**
 * @Description : This function gets the scalability number of associated
 *                key type
 */

UpplReturnCode PhysicalCore::GetScalabilityNumber(string version,
                                                  uint32_t key_type,
                                                  uint32_t &scalability_num) {
  ControllerVersion obj(version);

  // map<ControllerVersion, map<cap_key_struct, cap_value_struct > >::iterator
  // iter_cap_map = ctr_cap_map_.find(obj);
  cap_iter iter_cap_map = GetVersionIterator(obj);
  map<cap_key_struct, cap_value_struct > key_type_map;

  if (iter_cap_map != ctr_cap_map_.end()) {
    cap_key_struct keystructvar;
    keystructvar.key_type = key_type;
    key_type_map = iter_cap_map->second;
    map<cap_key_struct, cap_value_struct >::iterator iter_key_type_map_ =
        key_type_map.find(keystructvar);

    scalability_num = ((iter_key_type_map_)->second.scalability_num);
    pfc_log_debug("Scalability number for key type %d is %d",
                  key_type, scalability_num);
    return UPPL_RC_SUCCESS;
  } else {
    pfc_log_info("PhysicalCore::GetScalabilityNumber ret = %d",
                 UPPL_RC_ERR_VERSION_NOT_SUPPORTED);
    return UPPL_RC_ERR_VERSION_NOT_SUPPORTED;
  }
}

/**
 * @Description : This function sends event subscription request to Driver
 *                through IPCConnectionManager
 */

UpplReturnCode PhysicalCore::SendEventSubscripToDriver() {
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  if (physical_layer == NULL) {
    return UPPL_RC_ERR_FATAL_RESOURCE_ALLOCATION;
  }
  UpplReturnCode ret = physical_layer->get_ipc_connection_manager()->
      SendEventSubscription();
  return ret;
}

/**
 * @Description : This function sends cancel event subscription request to
 *                Driver through IPCConnectionManager
 */

UpplReturnCode PhysicalCore::CancelEventSubscripInDriver() {
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  if (physical_layer == NULL) {
    return UPPL_RC_ERR_FATAL_RESOURCE_ALLOCATION;
  }
  UpplReturnCode ret = physical_layer->get_ipc_connection_manager()->
      CancelEventSubscription();
  return ret;
}

/**
 * @Description : This function validates the config ID by TC library API
 */

UpplReturnCode PhysicalCore::ValidateConfigId(uint32_t session_id,
                                              uint32_t config_id) {
  TcLibModule* tclib_ptr = static_cast<TcLibModule*>
  (TcLibModule::getInstance(TCLIB_MODULE_NAME));

  uint8_t resp = tclib_ptr->TcLibValidateUpdateMsg(session_id, config_id);
  UpplReturnCode return_code = UPPL_RC_FAILURE;
  pfc_log_debug("Validation of config/session id with TC returned %d", resp);
  if (resp == unc::tclib::TC_API_COMMON_SUCCESS) {
    return_code = UPPL_RC_SUCCESS;
  } else if (resp == unc::tclib::TC_INVALID_CONFIG_ID) {
    return_code = UPPL_RC_ERR_INVALID_CONFIGID;
  } else if (resp == unc::tclib::TC_INVALID_SESSION_ID) {
    return_code = UPPL_RC_ERR_INVALID_SESSIONID;
  }
  // return the response received
  return return_code;
}

/**
 * @Description : This function validates if the operation is valid
 * in UPPL_SYSTEM_ST_STANDBY state
 * @param[in] : operation_type - type of operation requested
 */

UpplReturnCode PhysicalCore::ValidateStandbyRequests(uint32_t operation_type) {
  // Throw error when non-READ* requests are received in standby
  if (get_system_state() == UPPL_SYSTEM_ST_STANDBY &&
      (operation_type < UNC_OP_READ ||
          operation_type > UNC_OP_READ_SIBLING_COUNT)) {
    pfc_log_info("operation not supported in standby");
    return UPPL_RC_ERR_NOT_SUPPORTED_BY_STANDBY;
  }
  return UPPL_RC_SUCCESS;
}

/**
 * @Description : This function gives the corresponding for controller type
 */

UpplReturnCode PhysicalCore::GetDriverName(
    unc_keytype_ctrtype_t controller_type,
    string &driver_name) {
  map<unc_keytype_ctrtype_t, string>::iterator
  iter_drv_name_map = drv_name_map_.find(controller_type);
  if (iter_drv_name_map == drv_name_map_.end()) {
    pfc_log_info("Controller type not found");
    return UPPL_RC_FAILURE;
  }
  if (iter_drv_name_map->second == "") {
    pfc_log_info("Driver name empty");
    return UPPL_RC_FAILURE;
  }
  driver_name = iter_drv_name_map->second;
  return UPPL_RC_SUCCESS;
}

/**
 * @Description : This function will be called back when TC sends user
 *                commit-transaction start request to UPPL
 *                This is an implementation of virtual function in TCLib
 */

TcCommonRet PhysicalCore::HandleCommitTransactionStart(uint32_t session_id,
                                                       uint32_t config_id) {
  PHY_FINI_IPC_LOCK(unc::tclib::TC_SUCCESS);
  //  Reject the request when system is in StandBy
  if (get_system_state() == UPPL_SYSTEM_ST_STANDBY) {
    pfc_log_error("Operation Not allowed: System is in standby\n");
    return unc::tclib::TC_FAILURE;
  }
  // Call ITC transaction handler function
  UpplReturnCode resp = internal_transaction_coordinator_->transaction_req()->
      StartTransaction(session_id, config_id);
  pfc_log_debug("HandleCommitTransactionStart return code %d", resp);
  // convert the error code returned by ITC to TC error code
  if (resp == UPPL_RC_SUCCESS) {
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
  UpplReturnCode resp = internal_transaction_coordinator_->audit_req()->
      StartAuditTransaction(session_id,
                            ctrl_type,
                            controller_id);

  // convert the error code returned by ITC to TC error code
  if (resp == UPPL_RC_SUCCESS) {
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
    TcTransEndResult trans_res) {
  PHY_FINI_IPC_LOCK(unc::tclib::TC_SUCCESS);
  //  Reject the request when system is in StandBy
  if (get_system_state() == UPPL_SYSTEM_ST_STANDBY) {
    pfc_log_error("Operation Not allowed: System is in standby\n");
    return unc::tclib::TC_FAILURE;
  }
  // Call ITC transaction handler function
  UpplReturnCode resp = internal_transaction_coordinator_->transaction_req()->
      EndTransaction(session_id,
                     config_id);
  pfc_log_debug("HandleCommitTransactionEnd response %d", resp);
  // convert the error code returned by ITC to TC error code
  if (resp == UPPL_RC_SUCCESS) {
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
  UpplReturnCode resp = internal_transaction_coordinator_->audit_req()->
      EndAuditTransaction(session_id,
                          ctrl_type,
                          controller_id);

  // convert the error code returned by ITC to TC error code
  if (resp == UPPL_RC_SUCCESS) {
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
  UpplReturnCode resp = internal_transaction_coordinator_->transaction_req()->
      HandleVoteRequest(session_id,
                        config_id,
                        driver_info);
  pfc_log_debug("HandleVoteRequest response %d", resp);
  // convert the error code returned by ITC to TC error code
  if (resp == UPPL_RC_SUCCESS) {
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
  pfc_log_debug("Received HandleAuditVoteRequest from TC");
  // Call ITC transaction handler function
  UpplReturnCode resp = internal_transaction_coordinator_->audit_req()->
      HandleAuditVoteRequest(session_id,
                             driver_id,
                             controller_id,
                             driver_info);

  // convert the error code returned by ITC to TC error code
  if (resp == UPPL_RC_SUCCESS) {
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
    TcDriverInfoMap& driver_info) {
  PHY_FINI_IPC_LOCK(unc::tclib::TC_SUCCESS);
  //  Reject the request when system is in StandBy
  if (get_system_state() == UPPL_SYSTEM_ST_STANDBY) {
    pfc_log_error("Operation Not allowed: System is in standby\n");
    return unc::tclib::TC_FAILURE;
  }
  // Call ITC transaction handler function
  // ITC to fill driver_info map while sending response
  UpplReturnCode resp = internal_transaction_coordinator_->transaction_req()->
      HandleGlobalCommitRequest(session_id,
                                config_id,
                                driver_info);

  // convert the error code returned by ITC to TC error code
  if (resp == UPPL_RC_SUCCESS) {
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
  UpplReturnCode resp = internal_transaction_coordinator_->audit_req()->
      HandleAuditGlobalCommit(session_id,
                              driver_id,
                              controller_id,
                              driver_info,
                              audit_result);

  // convert the error code returned by ITC to TC error code
  if (resp == UPPL_RC_SUCCESS) {
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
    TcCommitPhaseType commitphase,
    TcCommitPhaseResult driver_result) {
  PHY_FINI_IPC_LOCK(unc::tclib::TC_SUCCESS);
  //  Reject the request when system is in StandBy
  if (get_system_state() == UPPL_SYSTEM_ST_STANDBY) {
    pfc_log_error("Operation Not allowed: System is in standby\n");
    return unc::tclib::TC_FAILURE;
  }
  // Call ITC transaction handler function
  UpplReturnCode resp = internal_transaction_coordinator_->transaction_req()->
      HandleDriverResult(session_id,
                         config_id,
                         commitphase,
                         driver_result);
  pfc_log_debug("HandleCommitDriverResult response %d", resp);
  // convert the error code returned by ITC to TC error code
  if (resp == UPPL_RC_SUCCESS) {
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
  pfc_log_debug("Received HandleAuditDriverResult from TC");
  // Call ITC transaction handler function
  UpplReturnCode resp = internal_transaction_coordinator_->audit_req()->
      HandleAuditDriverResult(session_id,
                              controller_id,
                              commitphase,
                              driver_result,
                              audit_result);
  // convert the error code returned by ITC to TC error code
  if (resp == UPPL_RC_SUCCESS) {
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
                                           string controller_id) {
  PHY_FINI_IPC_LOCK(unc::tclib::TC_SUCCESS);
  //  Reject the request when system is in StandBy
  if (get_system_state() == UPPL_SYSTEM_ST_STANDBY) {
    pfc_log_error("Operation Not allowed: System is in standby\n");
    return unc::tclib::TC_FAILURE;
  }
  pfc_log_debug("Received HandleAuditStart from TC");
  // Call ITC transaction handler function
  UpplReturnCode resp = internal_transaction_coordinator_->audit_req()->
      StartAudit(ctrl_type,
                 controller_id);

  // convert the error code returned by ITC to TC error code
  if (resp == UPPL_RC_SUCCESS) {
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
  // Call ITC transaction handler function
  UpplReturnCode resp = internal_transaction_coordinator_->audit_req()->
      EndAudit(ctrl_type,
               controller_id,
               audit_result);

  // convert the error code returned by ITC to TC error code
  if (resp == UPPL_RC_SUCCESS) {
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

TcCommonRet PhysicalCore::HandleSaveConfiguration(uint32_t session_id) {
  PHY_FINI_IPC_LOCK(unc::tclib::TC_SUCCESS);
  //  Reject the request when system is in StandBy
  if (get_system_state() == UPPL_SYSTEM_ST_STANDBY) {
    pfc_log_error("Operation Not allowed: System is in standby\n");
    return unc::tclib::TC_FAILURE;
  }
  // Call ITC transaction handler function
  UpplReturnCode resp = internal_transaction_coordinator_->db_config_req()->
      SaveRunningToStartUp();

  // convert the error code returned by ITC to TC error code
  if (resp == UPPL_RC_SUCCESS) {
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
  // Call ITC transaction handler function
  UpplReturnCode resp = internal_transaction_coordinator_->db_config_req()->
      ClearStartUpDb();

  // convert the error code returned by ITC to TC error code
  if (resp == UPPL_RC_SUCCESS) {
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
                                               uint32_t config_id) {
  PHY_FINI_IPC_LOCK(unc::tclib::TC_SUCCESS);
  //  Reject the request when system is in StandBy
  if (get_system_state() == UPPL_SYSTEM_ST_STANDBY) {
    pfc_log_error("Operation Not allowed: System is in standby\n");
    return unc::tclib::TC_FAILURE;
  }
  // Call ITC transaction handler function
  uint8_t resp = internal_transaction_coordinator_->db_config_req()->
      AbortCandidateDb();

  // convert the error code returned by ITC to TC error code
  if (resp == UPPL_RC_SUCCESS) {
    return unc::tclib::TC_SUCCESS;
  } else {
    return unc::tclib::TC_FAILURE;
  }
}

/**
 * @Description : This function will be called back when TC sends
 *                db recovery requests during failover
 *                This is an implementation of virtual function in TCLib
 */

TcCommonRet PhysicalCore::HandleAuditConfig(unc_keytype_datatype_t db_target,
                                            TcServiceType fail_oper) {
  PHY_FINI_IPC_LOCK(unc::tclib::TC_SUCCESS);
  //  Reject the request when system is in StandBy
  if (get_system_state() == UPPL_SYSTEM_ST_STANDBY) {
    pfc_log_error("Operation Not allowed: System is in standby\n");
    return unc::tclib::TC_FAILURE;
  }
  pfc_log_debug("Recieved HandleAuditConfig from TC");
  // Call ITC transaction handler function
  UpplReturnCode resp = UPPL_RC_SUCCESS;
  if (fail_oper == TC_OP_CANDIDATE_COMMIT) {
    uint32_t session_id = 0;
    uint32_t config_id = 0;
    TcDriverInfoMap driver_info;
    TransactionRequest *txn_req =
        internal_transaction_coordinator_->transaction_req();
    resp = txn_req->StartTransaction(session_id, config_id);
    if (resp != UPPL_RC_SUCCESS) {
      pfc_log_error("HandleAuditConfig - StartTransaction failed with %d",
                    resp);
      return unc::tclib::TC_FAILURE;
    }
    resp = txn_req->HandleVoteRequest(session_id,
                                      config_id, driver_info);
    if (resp != UPPL_RC_SUCCESS) {
      pfc_log_error("HandleAuditConfig - Vote failed with %d",
                    resp);
      return unc::tclib::TC_FAILURE;
    }

    TcCommitPhaseType phase = unc::tclib::TC_COMMIT_VOTE_PHASE;
    TcCommitPhaseResult driver_result;
    resp = txn_req->HandleDriverResult(session_id,
                                       config_id, phase, driver_result);
    if (resp != UPPL_RC_SUCCESS) {
      pfc_log_error("HandleAuditConfig - DriverResult VOTE PH failed with %d",
                    resp);
      return unc::tclib::TC_FAILURE;
    }

    resp = txn_req->HandleGlobalCommitRequest(session_id,
                                              config_id, driver_info);
    if (resp != UPPL_RC_SUCCESS) {
      pfc_log_error("HandleAuditConfig - GlobalCommit failed with %d",
                    resp);
      return unc::tclib::TC_FAILURE;
    }

    phase = unc::tclib::TC_COMMIT_GLOBAL_COMMIT_PHASE;
    resp = txn_req->HandleDriverResult(session_id,
                                       config_id, phase, driver_result);
    if (resp != UPPL_RC_SUCCESS) {
      pfc_log_error("HandleAuditConfig - DriverResult COM PH failed with %d",
                    resp);
      return unc::tclib::TC_FAILURE;
    }

    resp = txn_req->EndTransaction(session_id, config_id);
    if (resp != UPPL_RC_SUCCESS) {
      pfc_log_error("HandleAuditConfig - EndTransaction COM PH failed with %d",
                    resp);
      return unc::tclib::TC_FAILURE;
    }

  } else if (fail_oper == TC_OP_CLEAR_STARTUP) {
    resp = internal_transaction_coordinator_->db_config_req()->
        ClearStartUpDb();
  } else if (fail_oper == TC_OP_RUNNING_SAVE) {
    // Copy running to startup
    resp = internal_transaction_coordinator_->db_config_req()->
        SaveRunningToStartUp();
  } else if (fail_oper == TC_OP_CANDIDATE_ABORT) {
    resp = internal_transaction_coordinator_->db_config_req()->
        AbortCandidateDb();
  } else if (fail_oper == TC_OP_USER_AUDIT ||
      fail_oper == TC_OP_DRIVER_AUDIT) {
    // Do nothing
    resp = UPPL_RC_SUCCESS;
  }

  // convert the error code returned by ITC to TC error code
  if (resp == UPPL_RC_SUCCESS) {
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
    TcCommitOpAbortPhase fail_phase) {
  PHY_FINI_IPC_LOCK(unc::tclib::TC_SUCCESS);
  //  Reject the request when system is in StandBy
  if (get_system_state() == UPPL_SYSTEM_ST_STANDBY) {
    pfc_log_error("Operation Not allowed: System is in standby\n");
    return unc::tclib::TC_FAILURE;
  }
  // Call ITC transaction handler function
  UpplReturnCode resp = internal_transaction_coordinator_->transaction_req()->
      AbortTransaction(session_id,
                       config_id,
                       fail_phase);
  // convert the error code returned by ITC to TC error code
  if (resp == UPPL_RC_SUCCESS) {
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
  UpplReturnCode resp = internal_transaction_coordinator_->audit_req()->
      HandleAuditAbort(session_id,
                       ctrl_type,
                       controller_id,
                       operation_phase);

  // convert the error code returned by ITC to TC error code
  if (resp == UPPL_RC_SUCCESS) {
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
  // Call ITC transaction handler function
  // Copy startup to candidate and running and commit
  UpplReturnCode resp = internal_transaction_coordinator_->db_config_req()->
      LoadAndCommitStartup();

  // convert the error code returned by ITC to TC error code
  if (resp == UPPL_RC_SUCCESS) {
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
  // Call ITC transaction handler function
  // Copy startup to candidate and running and commit
  UpplReturnCode resp = internal_transaction_coordinator_->
      system_state_change_req()->SystemStateChangeToActive();
  pfc_log_debug("ReturnCode of SystemStateChangeToActive %d", resp);
  // convert the error code returned by ITC to TC error code
  if (resp == UPPL_RC_SUCCESS) {
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
  unc_keytype_ctrtype_t controller_type;
  UpplReturnCode resp = PhyUtil::get_controller_type(controller_id,
                                                     controller_type,
                                                     UNC_DT_CANDIDATE);
  // convert the error code returned by ITC to TC error code
  if (resp == UPPL_RC_SUCCESS) {
    return controller_type;
  } else {
    pfc_log_warn("Error reading controller type from DB");
    return UNC_CT_UNKNOWN;
  }
}

/**
 * @Description : This function returns the list of supported controller
 *                versions
 *                The controller version will be available in static capability
 *                file
 */

list<string> PhysicalCore::GetControllerVersionList() {
  // Obtain ctrlr version list from cap map
  list<string> ctlr_version_list;
  string conf_file_path = string(UNC_MODULEDIR) +
      string("/uppl_ctr_capability.conf");

  pfc::core::ConfHandle conf_handle(conf_file_path, &ctr_cap_cfdef);

  pfc::core::ConfBlock version_list_blk(conf_handle, "version_list");

  int kVersionListSize = version_list_blk.arraySize("version_list");
  for (int i = 0; i < kVersionListSize; ++i) {
    ctlr_version_list.push_back(version_list_blk.getStringAt("version_list",
                                                             i, 0));
  }
  return ctlr_version_list;
}

/**
 * @Description : This function sends CONROLLER_DISCONNECT alarm to node manager
 *                This function will be called from Kt_Controller class when it
 *                receives CONTROLLER notification from driver with oper_status
 *                as down
 */

UpplReturnCode PhysicalCore::SendControllerDisconnectAlarm(
    string controller_id) {
  if (get_system_state() == UPPL_SYSTEM_ST_STANDBY) {
    // system is in standby
    pfc_log_info("System is in standby");
    pfc_log_info("ControllerDisconnect alarm not sent to node manager");
    return UPPL_RC_SUCCESS;
  }
  string vtn_name = "";
  const std::string& alm_msg = "Controller disconnected";
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
    return UPPL_RC_ERR_ALARM_API;
  }
  delete []data->alarm_key;
  delete data;
  return UPPL_RC_SUCCESS;
}

/**
 * @Description : This function sends CONROLLER_CONNECT alarm to node manager
 *                This is a clearance alarm for CONTROLLER_DISCONNECT alarm
 *                This function will be called from Kt_Controller class when it
 *                receives CONTROLLER notification from driver with oper_status
 *                as up
 */

UpplReturnCode PhysicalCore::SendControllerConnectAlarm(string controller_id) {
  if (get_system_state() == UPPL_SYSTEM_ST_STANDBY) {
    // system is in standby
    pfc_log_info("System is in standby");
    pfc_log_info("ControllerConnect alarm not sent to node manager");
    return UPPL_RC_SUCCESS;
  }
  string vtn_name = "";
  const std::string& alm_msg = "Controller connected";
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
    delete []data->alarm_key;
    delete data;
    return UPPL_RC_ERR_ALARM_API;
  }
  delete []data->alarm_key;
  delete data;
  return UPPL_RC_SUCCESS;
}

/**
 * @Description : This function sends AUDIT_FAILURE alarm to node manager
 *                This function will be called from audit_req class when audit
 *                operation fails
 */

UpplReturnCode PhysicalCore::SendAuditFailureAlarm(string controller_id) {
  if (get_system_state() == UPPL_SYSTEM_ST_STANDBY) {
    // system is in standby
    pfc_log_info("System is in standby");
    pfc_log_info("AuditFailure alarm not sent to node manager");
    return UPPL_RC_SUCCESS;
  }
  string vtn_name = "";
  const std::string& alm_msg =
      "Controller audit failure due to Invalid Configuration";
  const std::string& alm_msg_summary =
      "Controller audit failure";
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
    return UPPL_RC_ERR_ALARM_API;
  }
  delete []data->alarm_key;
  delete data;
  return UPPL_RC_SUCCESS;
}

/**
 * @Description : This function sends AUDIT_SUCCESS alarm to node manager
 *                This is a clearance alarm for AUDIT_FAILURE alarm
 */

UpplReturnCode PhysicalCore::SendAuditSuccessAlarm(string controller_id) {
  if (get_system_state() == UPPL_SYSTEM_ST_STANDBY) {
    // system is in standby
    pfc_log_info("System is in standby");
    pfc_log_info("AuditSuccess alarm not sent to node manager");
    return UPPL_RC_SUCCESS;
  }
  string vtn_name = "";
  const std::string& alm_msg = "Controller audit success";
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
    return UPPL_RC_ERR_ALARM_API;
  }
  delete []data->alarm_key;
  delete data;
  return UPPL_RC_SUCCESS;
}

/**
 * @Description : This function sends EVENT_HANDLING FAILURE alarm to node manager
 *                This function will be called from kt classes when event handling fails
 */

UpplReturnCode
PhysicalCore::SendEventHandlingFailureAlarm(string controller_id,
                                            string event_details) {
  if (get_system_state() == UPPL_SYSTEM_ST_STANDBY) {
    // system is in standby
    pfc_log_info("System is in standby");
    pfc_log_info("Event Handling Failure alarm not sent to node manager");
    return UPPL_RC_SUCCESS;
  }
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
    return UPPL_RC_ERR_ALARM_API;
  }
  delete []data->alarm_key;
  delete data;
  return UPPL_RC_SUCCESS;
}

/**
 * @Description : This function sends EVENT_HANDLING_SUCCESS alarm to node manager
 *                This is a clearance alarm for EVENT_HANDLING_FAILURE alarm
 */

UpplReturnCode
PhysicalCore::SendEventHandlingSuccessAlarm(string controller_id,
                                            string event_details) {
  if (get_system_state() == UPPL_SYSTEM_ST_STANDBY) {
    // system is in standby
    pfc_log_info("System is in standby");
    pfc_log_info("Event handling Success alarm not sent to node manager");
    return UPPL_RC_SUCCESS;
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
    return UPPL_RC_ERR_ALARM_API;
  }
  delete []data->alarm_key;
  delete data;
  return UPPL_RC_SUCCESS;
}

/**
 * @Description : This function returns the iterator of the matched version
 * in the capability map
 */
cap_iter PhysicalCore::GetVersionIterator(ControllerVersion version_in) {
  cap_iter iter_cap = ctr_cap_map_.begin();
  for (; iter_cap != ctr_cap_map_.end(); ++iter_cap) {
    ControllerVersion version = (*iter_cap).first;
    if (version.product_version_part1_ <= version_in.product_version_part1_ &&
        version.product_version_part2_ <= version_in.product_version_part2_ &&
        version.product_version_part3_ <= version_in.product_version_part3_) {
      // match found
      return iter_cap;
    }
  }
  // Return end of map
  iter_cap = ctr_cap_map_.end();
  return iter_cap;
}

/** RaiseEventHandlingAlarm()
 * * @Description : This function checks whether event handling failure alarm
 * is already raised for a controller
 * * * @param[in] : controller_name
 * * * @return    : UPPL_RC_SUCCESS or UPPL_RC_FAILURE
 * */
UpplReturnCode PhysicalCore::RaiseEventHandlingAlarm(string controller_name) {
  if (find(event_handling_controller_alarm_.begin(),
           event_handling_controller_alarm_.end(),
           controller_name) != event_handling_controller_alarm_.end()) {
    pfc_log_info("Alarm already raised for this controller id");
    return UPPL_RC_FAILURE;
  }
  pfc_log_debug("Adding the controller_id in the alarm_raised vector");
  event_handling_controller_alarm_.push_back(controller_name);
  return UPPL_RC_SUCCESS;
}

/** ClearEventHandlingAlarm
 * * @Description : This function checks whether event handling success alarm
 * is already raised for a controller
 * * * @param[in] : controller_name
 * * * @return    : UPPL_RC_SUCCESS or UPPL_RC_FAILURE
 * */
UpplReturnCode PhysicalCore::ClearEventHandlingAlarm(string controller_name) {
  vector<string>::iterator alarm_raised_iter =
      find(event_handling_controller_alarm_.begin(),
           event_handling_controller_alarm_.end(),
           controller_name);
  if (alarm_raised_iter != event_handling_controller_alarm_.end()) {
    pfc_log_debug(" Failure Alarm already raised for this controller id");
    event_handling_controller_alarm_.erase(alarm_raised_iter);
    pfc_log_debug("Clearing the alarm");
    return UPPL_RC_SUCCESS;
  }
  pfc_log_info("Ignoring the clearance alarm");
  return UPPL_RC_FAILURE;
}
}  // namespace uppl
}  // namespace unc

