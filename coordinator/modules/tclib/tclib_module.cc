/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <tclib_module.hh>
#include <tclib_msg_util.hh>
#include <uncxx/tc/libtc_common.hh>

namespace unc {
namespace tclib {

const char* TcMsgOperTypeString[] = {
  "MSG_NONE",
  "MSG_NOTIFY_CONFIGID",
  "MSG_SETUP",
  "MSG_SETUP_COMPLETE",
  "MSG_COMMIT_TRANS_START",
  "MSG_COMMIT_VOTE",
  "MSG_COMMIT_DRIVER_VOTE",
  "MSG_COMMIT_VOTE_DRIVER_RESULT",
  "MSG_COMMIT_GLOBAL",
  "MSG_COMMIT_DRIVER_GLOBAL",
  "MSG_COMMIT_GLOBAL_DRIVER_RESULT",
  "MSG_COMMIT_TRANS_END",
  "MSG_AUDIT_START",
  "MSG_AUDIT_TRANS_START",
  "MSG_AUDIT_VOTE",
  "MSG_AUDIT_DRIVER_VOTE",
  "MSG_AUDIT_VOTE_DRIVER_RESULT",
  "MSG_AUDIT_GLOBAL",
  "MSG_AUDIT_DRIVER_GLOBAL",
  "MSG_AUDIT_GLOBAL_DRIVER_RESULT",
  "MSG_AUDIT_TRANS_END",
  "MSG_AUDIT_END",
  "MSG_AUDIT_CANCEL",
  "MSG_SAVE_CONFIG",
  "MSG_CLEAR_CONFIG",
  "MSG_ABORT_CANDIDATE",
  "MSG_AUDITDB",
  "MSG_COMMIT_ABORT",
  "MSG_AUDIT_ABORT",
  "MSG_GET_DRIVERID",
  "MSG_CONTROLLER_TYPE",
  "MSG_AUTOSAVE_ENABLE",
  "MSG_AUTOSAVE_DISABLE",
  "MSG_MAX"
};

/**
 * @brief      TcLibModule constructor
 * @param[in]  *mattr Attribute of the Module
 */
TcLibModule::TcLibModule(const pfc_modattr_t *mattr)
                   : pfc::core::Module(mattr) {
  /* Initialization of TcLibModule data */
  pTcLibInterface_ = NULL;
  sess_ = NULL;
  tc_notify_config_data_map_.clear();
  controllerid_.clear();
  audit_in_progress_ = PFC_FALSE;
  oper_state_ = MSG_NONE;
  key_map_.clear();
  controller_key_map_.clear();
  commit_phase_result_.clear();
  audit_in_progress_ = PFC_FALSE;
  auto_save_enabled_ = PFC_FALSE;
  request_counter_ = 0;
  handle_in_tclib_ = PFC_FALSE;

  pfc_log_info("%s %d constructor", __FUNCTION__, __LINE__);
}

/**
 * @brief      TcLibModule destructor
 */
TcLibModule::~TcLibModule() {
  pfc_log_info("%s %d destructor", __FUNCTION__, __LINE__);
}

/**
 * @brief      Release transaction resources those involved in either
 *             commit/audit operations
 */
void TcLibModule::ReleaseTransactionResources() {
  // Freeing the resources used in transaction processing
  oper_state_ = MSG_NONE;
  // clearing the containers
  key_map_.clear();
  controller_key_map_.clear();
  commit_phase_result_.clear();
  SetHandleCancelAuditInTclib(PFC_FALSE);
  pfc_log_info("%s %d Released resources used for transaction",
               __FUNCTION__, __LINE__);
}

/**
 * @brief      Register TclibInterface handler
 *             TcLibInterface handlers of UPLL/UPPL/driver modules are
 *             registered which will be updated in handler.
 * @retval     TC_API_COMMON_SUCCESS on register handler success
 * @retval     TC_INVALID_PARAM if handler is NULL
 * @retval     TC_HANDLER_ALREADY_ACTIVE if already register handler has done
 */
TcApiCommonRet TcLibModule::TcLibRegisterHandler(TcLibInterface* handler) {
  TcApiCommonRet ret = TC_API_COMMON_SUCCESS;

  if (handler == NULL)
    return TC_INVALID_PARAM;

  pfc::core::ScopedMutex m(tclib_mutex_);

  if (pTcLibInterface_ == NULL) {
    pfc_log_info("%s %d Registering the handler", __FUNCTION__, __LINE__);
    pTcLibInterface_ = handler;
    ret = TC_API_COMMON_SUCCESS;
  } else {
    pfc_log_error("%s %d Handler Already Registered", __FUNCTION__, __LINE__);
    ret = TC_HANDLER_ALREADY_ACTIVE;
  }
  return ret;
}

/**
 * @brief      Init of TcLibModule
 *             initialises the member variables
 */
pfc_bool_t TcLibModule::init() {
  pfc_log_info("%s %d tclib initialisation", __FUNCTION__, __LINE__);
  return PFC_TRUE;
}

/**
 * @brief      Terminate of TcLibModule
 *             setting the member variables to default values
 */
pfc_bool_t TcLibModule::fini() {
  pTcLibInterface_ = NULL;
  sess_ = NULL;
  tc_notify_config_data_map_.clear();
  oper_state_ = MSG_NONE;
  audit_in_progress_ = PFC_FALSE;
  key_map_.clear();
  controller_key_map_.clear();
  commit_phase_result_.clear();

  pfc_log_info("%s %d tclib shutdown", __FUNCTION__, __LINE__);
  return PFC_TRUE;
}

/**
 * @brief      Audit controller request invoked from driver modules
 * @param[in]  controller_id controller id intended for audit
 * @retval     TC_API_COMMON_SUCCESS Audit controller request success
 * @retval     TC_API_FATAL on any api fatal failures
 * @retval     TC_API_COMMON_FAILURE on any handling failures
 */
TcApiCommonRet TcLibModule::TcLibAuditControllerRequest
                            (std::string controller_id) {
  TcApiCommonRet ret = TC_API_COMMON_SUCCESS;
  pfc_ipcconn_t conn = 0;
  pfc_ipcresp_t resp = 0;
  uint32_t idx = 0;
  int err = 0;
  unc_keytype_ctrtype_t ctr_type = UNC_CT_UNKNOWN;
  tc::TcUtilRet util_ret = tc::TCUTIL_RET_SUCCESS;
  uint8_t force_reconnect = 0;

  if (pTcLibInterface_ != NULL) {
    ctr_type = pTcLibInterface_->HandleGetControllerType();
    if (ctr_type == UNC_CT_UNKNOWN) {
      pfc_log_error("%s %d Invalid controller type cannot proceed for audit",
                    __FUNCTION__, __LINE__);
      return TC_API_COMMON_FAILURE;
    }
  } else {
    pfc_log_error("%s %d TcLibInterface not registered",
                  __FUNCTION__, __LINE__);
    return TC_API_COMMON_FAILURE;
  }

  /* Construct Message and send controller id to TC to start Audit
   * Create a client session towards the TC
   * Fill the controller id also mention the service id
   */
  err = pfc_ipcclnt_altopen("uncd", &conn);
  if (err != 0 || conn == 0) {
    pfc_log_fatal("%s %d client altopen failed", __FUNCTION__, __LINE__);
    return TC_API_FATAL;
  }

  pfc::core::ipc::ClientSession sess(conn, "tc", TC_AUDIT_SERVICES,
                                     err);
  if (err != 0) {
    pfc_log_fatal("%s %d client session create failed towards TC",
                  __FUNCTION__, __LINE__);
    pfc_ipcclnt_altclose(conn);
    return TC_API_FATAL;
  }

  /* Writing service type into the session */
  util_ret = tc::TcClientSessionUtils::set_uint32(&sess,
                                                  TC_OP_DRIVER_AUDIT);
  if (util_ret != tc::TCUTIL_RET_SUCCESS) {
    pfc_log_error("%s %d TcServerSessionUtils failed with %d",
                  __FUNCTION__, __LINE__, util_ret);
    pfc_ipcclnt_altclose(conn);
    return TC_API_COMMON_FAILURE;
  }

  /* Writing controller id into the session */
  idx++;
  util_ret = tc::TcClientSessionUtils::set_string(&sess, controller_id);
  if (util_ret != tc::TCUTIL_RET_SUCCESS) {
    pfc_log_error("%s %d TcServerSessionUtils failed with %d",
                  __FUNCTION__, __LINE__, util_ret);
    pfc_ipcclnt_altclose(conn);
    return TC_API_COMMON_FAILURE;
  }

  /* Writing controller type into the session */
  idx++;
  util_ret = tc::TcClientSessionUtils::set_uint8(&sess,
                                                 (uint8_t)ctr_type);
  if (util_ret != tc::TCUTIL_RET_SUCCESS) {
    pfc_log_error("%s %d TcServerSessionUtils failed with %d",
                  __FUNCTION__, __LINE__, util_ret);
    pfc_ipcclnt_altclose(conn);
    return TC_API_COMMON_FAILURE;
  }
  /* Writing force_reconnect into the session */
  idx++;
  util_ret = tc::TcClientSessionUtils::set_uint8(&sess, force_reconnect);
  if (util_ret != tc::TCUTIL_RET_SUCCESS) {
    pfc_log_error("%s %d TcServerSessionUtils failed with %d",
                  __FUNCTION__, __LINE__, util_ret);
    pfc_ipcclnt_altclose(conn);
    return TC_API_COMMON_FAILURE;
  }

  /*Invoke the session using the handle*/
  err = sess.invoke(resp);
  if (err != 0) {
    pfc_log_error("%s %d PFC IPC Invoke Failed", __FUNCTION__, __LINE__);
    pfc_ipcclnt_altclose(conn);
    return TC_API_FATAL;
  }

  /* Get the response and return the same towards Driver module */
  if (resp == TC_OPER_SUCCESS) {
    pfc_log_info("%s %d Audit Message to TC success", __FUNCTION__, __LINE__);
    ret = TC_API_COMMON_SUCCESS;
  } else {
    pfc_log_error("%s %d Audit Message to TC failed", __FUNCTION__, __LINE__);
    ret = TC_API_COMMON_FAILURE;
  }

  pfc_ipcclnt_altclose(conn);
  return ret;
}

/**
 * @brief      Validate update message invoked from other modules
 * @param[in]  session_id session on which commit request sent
 * @param[in]  config_id config id generated on acquire config mode for
 *             session_id
 * @param[in]  config_mode (global/real/virtual/VTN) types
 * @param[in]  vtn_name - name of VTN type.
 * @retval     TC_API_COMMON_SUCCESS on validation is success
 * @retval     TC_INVALID_SESSION_ID if session id is invalid
 * @retval     TC_INVALID_CONFIG_id if config id is invalid
 */
TcApiCommonRet TcLibModule::TcLibValidateUpdateMsg(uint32_t session_id,
                                                   uint32_t config_id,
                                                   TcConfigMode config_mode, 
                                                   std::string vtn_name) {
  pfc::core::ScopedMutex m(tclib_mutex_);

  TcApiCommonRet ret = TC_API_COMMON_SUCCESS;

  uint32_t validate_config_id = 0;
  TcConfigMode validate_config_mode = TC_CONFIG_INVALID;
  std::string validate_vtn_name;
  
  TcCommonRet common_ret = GetConfigData(session_id, validate_config_id, 
                              validate_config_mode, validate_vtn_name);
  if (common_ret != TC_SUCCESS) {
    pfc_log_error("%s %d Invalid session-id[%u]",
                  __FUNCTION__, __LINE__, session_id);
    ret = TC_INVALID_SESSION_ID;
  } else if ((validate_config_id == TC_DEFAULT_VALUE) || 
             (config_id == TC_DEFAULT_VALUE) ||
             (validate_config_id != config_id)) {
    pfc_log_error("%s %d Invalid config id %d",
                  __FUNCTION__, __LINE__, config_id);
    ret = TC_INVALID_CONFIG_ID;
  } else if (validate_config_mode == TC_CONFIG_GLOBAL) {
    ret = TC_API_COMMON_SUCCESS; 
  } else if (config_mode != validate_config_mode) { 
    pfc_log_error("%s %d Invalid config mode %d",
                  __FUNCTION__, __LINE__, config_mode);
    ret = TC_INVALID_MODE;
  } else if (config_mode == TC_CONFIG_VTN && vtn_name != validate_vtn_name) {
    pfc_log_error("%s %d Invalid vtn %s",
                    __FUNCTION__, __LINE__, vtn_name.c_str());
    return TC_INVALID_MODE;
  }
  return ret;
}


/**
 * @brief       Gives current session_id and config_id to upll, uppl, pfcdriver
 * @param[in]   current session_id
 * @param[out]  current config_id
 * @param[out]  current config_mode
 * @param[out]  current vtn_name (if config_mode is vtn)
 * @return      TC_API_COMMON_SUCCESS on success
 * @return      TC_INVALID_SESSION_ID on invalid session id
 */
TcApiCommonRet TcLibModule::TcLibGetSessionAttributes(uint32_t session_id,
                                                      uint32_t& config_id,
                                                      TcConfigMode& config_mode,
                                                      std::string& vtn_name) {
  config_id = 0;
  config_mode = TC_CONFIG_INVALID;
  vtn_name.clear();

  pfc::core::ScopedMutex m(tc_notify_config_data_map_lock_);
  if (tc_notify_config_data_map_.find(session_id) == 
                               tc_notify_config_data_map_.end())  {
     return TC_INVALID_SESSION_ID;
  }
  config_id = tc_notify_config_data_map_[session_id].config_id_;
  config_mode = tc_notify_config_data_map_[session_id].config_mode_;
  if (config_mode == TC_CONFIG_VTN) {
    vtn_name = tc_notify_config_data_map_[session_id].vtn_name_;
  }
  return TC_API_COMMON_SUCCESS;
}

/**
   * @brief      GetKeyIndex
   *             Get of key index based on controller id, err_pos from
   *             maps filled during driver result key map updates
   * @param[in]  controller_id controller id to which key type belongs
   * @param[in]  err_pos error position to which key index is required
   * @param[out] key_index index for the error position under controller id
   * @retval     TC_API_COMMON_SUCCESS if key index successfully filled
   * @retval     TC_INVALID_PARAM if the err_pos is invalid
   * @retval     TC_INVALID_KEY_TYPE if key type is invalid
   * @retval     TC_INVALID_CONTROLLER_ID if controller id is invalid
   */
TcApiCommonRet TcLibModule::GetKeyIndex(std::string controller_id,
                                        uint32_t err_pos,
                                        uint32_t &key_index) {
  TcApiCommonRet ret = TC_API_COMMON_SUCCESS;
  /* Read of Key Value definitions based on controller_id and keytype*/
  std::map<std::string, TcKeyTypeIndexMap>::iterator it;
  std::map<uint32_t, uint32_t>::iterator key_it;
  TcKeyTypeIndexMap key_type_map;

  pfc_log_debug("%s %d Cntrl ID:%s err_pos %d",
               __FUNCTION__, __LINE__, controller_id.c_str(), err_pos);

  if (!controller_key_map_.empty()) {
    it = controller_key_map_.find(controller_id);
    if (it == controller_key_map_.end()) {
      pfc_log_error("Missing contrl-id:%s", controller_id.c_str());
      return TC_INVALID_PARAM;
    }
    key_type_map = it->second;
    if (!key_type_map.empty()) {
      key_it = key_type_map.find(err_pos);
      if (key_it != key_type_map.end()) {
        key_index = key_it->second;
        pfc_log_debug("%s %d updated key_index %d",
                     __FUNCTION__, __LINE__, key_index);
      } else {
        pfc_log_error("%s %d invalid err pos %d",
                      __FUNCTION__, __LINE__, err_pos);
        return TC_INVALID_PARAM;
      }
    } else {
      pfc_log_error("%s %d key type index map is empty",
                    __FUNCTION__, __LINE__);
      return TC_INVALID_KEY_TYPE;
    }
  } else {
    pfc_log_error("%s %d controller key type index map is empty",
                  __FUNCTION__, __LINE__);
    return TC_INVALID_CONTROLLER_ID;
  }

  return ret;
}


/**
 * @brief      Write of controller id, response code, num of errors,
 *             commit_number, commit_version, commit_application.
 * @param[in]  controller_id controller id for which key type involved
 * @param[in]  response_code response code from the UPLL/UPPL
 * @param[in]  num_of_errors number of errors if any
 * @retval     TC_API_COMMON_SUCCESS on successful updation for key and value
 * @retval     TC_INVALID_OPER_STATE invalid oper state for write
 * @retval     TC_INVALID_CONTROLLER_ID invalid controller id
 * @retval     TC_API_FATAL on any api fatal failures
 */

TcApiCommonRet TcLibModule::TcLibWriteControllerInfo(std::string controller_id,
                            uint32_t response_code, uint32_t num_of_errors) {
  TcApiCommonRet ret= TC_API_COMMON_SUCCESS;
  tc::TcUtilRet util_ret = tc::TCUTIL_RET_SUCCESS;

  if (oper_state_ == MSG_NONE) {
    pfc_log_error("%s %d Invalid operation to write key value info",
                  __FUNCTION__, __LINE__);
    return TC_INVALID_OPER_STATE;
  }

  /* write of controller_id, response_code, num_of_errors, commit_number,
   * commit_date and commit_application */
  util_ret = tc::TcServerSessionUtils::set_string(sess_, controller_id);
  if (util_ret != tc::TCUTIL_RET_SUCCESS) {
    pfc_log_error("%s %d TcServerSessionUtils failed with %d",
                  __FUNCTION__, __LINE__, util_ret);
    return TC_API_COMMON_FAILURE;
  }

  util_ret = tc::TcServerSessionUtils::set_uint32(sess_, response_code);
  if (util_ret != tc::TCUTIL_RET_SUCCESS) {
    pfc_log_error("%s %d TcServerSessionUtils failed with %d",
                  __FUNCTION__, __LINE__, util_ret);
    return TC_API_COMMON_FAILURE;
  }

  util_ret = tc::TcServerSessionUtils::set_uint32(sess_, num_of_errors);
  if (util_ret != tc::TCUTIL_RET_SUCCESS) {
    pfc_log_error("%s %d TcServerSessionUtils failed with %d",
                  __FUNCTION__, __LINE__, util_ret);
    return TC_API_COMMON_FAILURE;
  }

  pfc_log_debug("%s %d Write controller info involved for controller_id %s",
               __FUNCTION__, __LINE__, controller_id.c_str());
  controllerid_ = controller_id;

  return ret;
}

/**
 * @brief      Read of key types key and value data information from session
 * @param[in]  controller_id controller id for which key type involved
 * @param[in]  err_pos position of the key type in the key_list
 * @param[in]  key_type intended key type for which key and value to be read
 * @param[in]  key_def ipcstdef structure name for key
 * @param[in]  value_def ipcstdef structure name for value
 * @param[out] key_data key data to be filled for requested key type
 * @param[out] value_data value data to be filled for requested key type
 * @retval     TC_API_COMMON_SUCCESS on successful updation for key and value
 * @retval     TC_INVALID_KEY_TYPE invalid key type with respect to err_pos
 * @retval     TC_INVALID_CONTROLLER_ID invalid controller id
 * @retval     TC_API_FATAL on any api fatal failures
 */
TcApiCommonRet TcLibModule::TcLibReadKeyValueDataInfo(std::string controller_id,
                                                      uint32_t err_pos,
                                                      uint32_t key_type,
                                                      pfc_ipcstdef_t key_def,
                                                      pfc_ipcstdef_t value_def,
                                                      void* key_data,
                                                      void* value_data) {
  TcApiCommonRet ret = TC_API_COMMON_SUCCESS;
  uint32_t idx = 0;

  ret = IsReadKeyValueAllowed();
  if (ret != TC_API_COMMON_SUCCESS) {
    pfc_log_error("%s %d Invalid operation to read key value info",
                  __FUNCTION__, __LINE__);
    return TC_INVALID_OPER_STATE;
  }

  uint32_t key_index, local_key_type;
  ret = GetKeyIndex(controller_id, err_pos, key_index);
  if (ret != TC_API_COMMON_SUCCESS) {
    pfc_log_error("%s %d Invalid key type/controllerid/err_pos %d",
                  __FUNCTION__, __LINE__, ret);
    return ret;
  }

  tc::TcUtilRet util_ret = tc::TCUTIL_RET_SUCCESS;

  // Read from the session based on key_index.
  idx = key_index;
  util_ret = tc::TcServerSessionUtils::get_uint32(sess_, idx,
                                                  &local_key_type);
  if (util_ret != tc::TCUTIL_RET_SUCCESS) {
    pfc_log_error("%s %d TcServerSessionUtils failed with %d",
                  __FUNCTION__, __LINE__, util_ret);
    return TC_API_COMMON_FAILURE;
  }

  if (key_type == local_key_type) {
    // Read the next two indices for key and value of key Type
    idx++;
    util_ret = tc::TcServerSessionUtils::get_struct(sess_, idx,
                                                         key_def, key_data);
    if (util_ret != tc::TCUTIL_RET_SUCCESS) {
      pfc_log_error("%s %d TcServerSessionUtils failed with %d",
                    __FUNCTION__, __LINE__, util_ret);
      return TC_API_COMMON_FAILURE;
    }

    if (value_data != NULL) {
      idx++;
      util_ret = tc::TcServerSessionUtils::get_struct(sess_, idx, value_def,
                                                value_data);
      if (util_ret != tc::TCUTIL_RET_SUCCESS) {
        pfc_log_error("%s %d TcServerSessionUtils failed with %d",
                      __FUNCTION__, __LINE__, util_ret);
        return TC_API_COMMON_FAILURE;
      }
    } else {
      pfc_log_debug("Value structure is empty");
    }
  } else {
    pfc_log_error("%s %d key type mismatch key_type %d local_key_type %d",
                  __FUNCTION__, __LINE__, key_type, local_key_type);
    return TC_INVALID_KEY_TYPE;
  }

  return ret;
}

/**
 * @brief      Write of controller id, response code, num of errors,
 *             commit_number, commit_version, commit_application.
 * @param[in]  controller_id controller id for which key type involved
 * @param[in]  response_code response code from the pfcdriver
 * @param[in]  num_of_errors number of errors if any
 * @param[in]  commit_number current Commit version of PFC
 * @param[in]  commit_date latest committed time of PFC
 * @param[in]  commit_application application that performed commit operation
 * @retval     TC_API_COMMON_SUCCESS on successful updation for key and value
 * @retval     TC_INVALID_OPER_STATE invalid oper state for write
 * @retval     TC_INVALID_CONTROLLER_ID invalid controller id
 * @retval     TC_API_FATAL on any api fatal failures
 * @note:     This API is used only by PFCdriver to fill PFC commit informations
 */

TcApiCommonRet TcLibModule::TcLibWriteControllerInfo(std::string controller_id,
                            uint32_t response_code, uint32_t num_of_errors,
                            uint64_t commit_number, uint64_t commit_date,
                            std::string commit_application) {
  TcApiCommonRet ret= TC_API_COMMON_SUCCESS;
  tc::TcUtilRet util_ret = tc::TCUTIL_RET_SUCCESS;

  if (oper_state_ == MSG_NONE) {
    pfc_log_error("%s %d Invalid operation to write key value info",
                  __FUNCTION__, __LINE__);
    return TC_INVALID_OPER_STATE;
  }

  /* write of controller_id, response_code, num_of_errors, commit_number,
   * commit_date and commit_application */
  util_ret = tc::TcServerSessionUtils::set_string(sess_, controller_id);
  if (util_ret != tc::TCUTIL_RET_SUCCESS) {
    pfc_log_error("%s %d TcServerSessionUtils failed with %d",
                  __FUNCTION__, __LINE__, util_ret);
    return TC_API_COMMON_FAILURE;
  }

  util_ret = tc::TcServerSessionUtils::set_uint32(sess_, response_code);
  if (util_ret != tc::TCUTIL_RET_SUCCESS) {
    pfc_log_error("%s %d TcServerSessionUtils failed with %d",
                  __FUNCTION__, __LINE__, util_ret);
    return TC_API_COMMON_FAILURE;
  }

  util_ret = tc::TcServerSessionUtils::set_uint32(sess_, num_of_errors);
  if (util_ret != tc::TCUTIL_RET_SUCCESS) {
    pfc_log_error("%s %d TcServerSessionUtils failed with %d",
                  __FUNCTION__, __LINE__, util_ret);
    return TC_API_COMMON_FAILURE;
  }

  util_ret = tc::TcServerSessionUtils::set_uint64(sess_, commit_number);
  if (util_ret != tc::TCUTIL_RET_SUCCESS) {
    pfc_log_error("%s %d TcServerSessionUtils failed with %d",
                  __FUNCTION__, __LINE__, util_ret);
    return TC_API_COMMON_FAILURE;
  }

  util_ret = tc::TcServerSessionUtils::set_uint64(sess_, commit_date);
  if (util_ret != tc::TCUTIL_RET_SUCCESS) {
    pfc_log_error("%s %d TcServerSessionUtils failed with %d",
                  __FUNCTION__, __LINE__, util_ret);
    return TC_API_COMMON_FAILURE;
  }

  util_ret = tc::TcServerSessionUtils::set_string(sess_, commit_application);
  if (util_ret != tc::TCUTIL_RET_SUCCESS) {
    pfc_log_error("%s %d TcServerSessionUtils failed with %d",
                  __FUNCTION__, __LINE__, util_ret);
    return TC_API_COMMON_FAILURE;
  }


  pfc_log_debug("%s %d Write controller info involved for controller_id %s",
               __FUNCTION__, __LINE__, controller_id.c_str());
  controllerid_ = controller_id;

  return ret;
}

/**
 * @brief      Write of key type, key and data
 * @param[in]  controller_id controller id for which key type involved
 * @param[in]  key_type intended key type for which key and value to write
 * @param[in]  key_def ipcstdef structure name for key
 * @param[in]  value_def ipcstdef structure name for value
 * @param[in]  key_data key data to write into session
 * @param[in]  value_data value data to write into session
 * @retval     TC_API_COMMON_SUCCESS on successful updation for key and value
 * @retval     TC_INVALID_OPER_STATE invalid oper state for write
 * @retval     TC_INVALID_CONTROLLER_ID invalid controller id
 * @retval     TC_API_FATAL on any api fatal failures
 */
TcApiCommonRet TcLibModule::TcLibWriteKeyValueDataInfo(
                                         std::string controller_id,
                                         uint32_t key_type,
                                         pfc_ipcstdef_t key_def,
                                         pfc_ipcstdef_t value_def,
                                         void* key_data,
                                         void* value_data) {
  TcApiCommonRet ret= TC_API_COMMON_SUCCESS;
  tc::TcUtilRet util_ret = tc::TCUTIL_RET_SUCCESS;

  if (oper_state_ == MSG_NONE) {
    pfc_log_error("%s %d Invalid operation to write key value info",
                  __FUNCTION__, __LINE__);
    return TC_INVALID_OPER_STATE;
  }
  /* Validate the controller id whether for same the earlier write
   * API is involved or not */
  if (controllerid_ != controller_id) {
    pfc_log_error("%s %d Invalid Controller id", __FUNCTION__, __LINE__);
    return TC_INVALID_CONTROLLER_ID;
  }

  /* Write of Key Value information to the session*/
  util_ret = tc::TcServerSessionUtils::set_uint32(sess_, key_type);
  if (util_ret != tc::TCUTIL_RET_SUCCESS) {
    pfc_log_error("%s %d TcServerSessionUtils failed with %d",
                  __FUNCTION__, __LINE__, util_ret);
    return TC_API_COMMON_FAILURE;
  }

  util_ret = tc::TcServerSessionUtils::set_struct(sess_, key_def,
                                                       key_data);
  if (util_ret != tc::TCUTIL_RET_SUCCESS) {
    pfc_log_error("%s %d TcServerSessionUtils failed with %d",
                  __FUNCTION__, __LINE__, util_ret);
    return TC_API_COMMON_FAILURE;
  }

  if (value_data != NULL) {
    util_ret = tc::TcServerSessionUtils::set_struct(sess_, value_def,
                                                         value_data);
    if (util_ret != tc::TCUTIL_RET_SUCCESS) {
      pfc_log_error("%s %d TcServerSessionUtils failed with %d",
                    __FUNCTION__, __LINE__, util_ret);
      return TC_API_COMMON_FAILURE;
    }
  } else {
    pfc_log_debug("Value structure is not filled");
  }
  pfc_log_info("%s %d write data succcess keytype %d ctrlid %s",
               __FUNCTION__, __LINE__, key_type, controller_id.c_str());
  return ret;
}

uint32_t TcLibModule::GetMatchTypeIndex(uint32_t cur_idx, uint32_t arg_count,
                                         pfc_ipctype_t type) {
  pfc_ipctype_t response;
  uint8_t err = 0;
  while (cur_idx < arg_count) {
    cur_idx++;
    if (cur_idx < arg_count) {
      /*validate response type*/
      err = sess_->getArgType(cur_idx, response);
      if ( err != 0 ) {
        pfc_log_error("Could not retrieve ArgType %d", __LINE__);
        return 0;
      }
      if (response != type) {
        pfc_log_debug("%s %d index %d does not has matched type %d",
                      __FUNCTION__, __LINE__, cur_idx, response);
        continue;
      }
      break;
    } else {
        pfc_log_debug("%s %d index %d exceeded arg count %d",
                      __FUNCTION__, __LINE__, cur_idx, arg_count);
        return 0;
    }
  }

  pfc_log_debug("%s %d returning index %d ", __FUNCTION__, __LINE__, cur_idx);
  return cur_idx;
}

/**
 * @brief      Update of controller key list information
 * @retval     TC_SUCCESS on successful updation of controller key list
 * @retval     TC_FAILURE ony failures
 */
TcCommonRet TcLibModule::UpdateControllerKeyList() {
  TcCommonRet ret = TC_SUCCESS;
  uint32_t argcount = 0, idx = 0;
  TcCommitPhaseResult comm_phase_res;
  tc::TcUtilRet util_ret = tc::TCUTIL_RET_SUCCESS;

  if (sess_ != NULL) {
    argcount = sess_->getArgCount();
    pfc_log_debug("%s %d session arg count %d",
                 __FUNCTION__, __LINE__, argcount);
  } else {
    pfc_log_error("%s %d session pointer is NULL", __FUNCTION__, __LINE__);
    return TC_FAILURE;
  }
  if (argcount == IPC_DEFAULT_ARG_COUNT) {
    pfc_log_error("%s %d Argument count is NULL", __FUNCTION__, __LINE__);
    return TC_FAILURE;
  }

  // clearing the containers between vote and global
  key_map_.clear();
  controller_key_map_.clear();
  commit_phase_result_.clear();

  idx = DRV_RESULT_START_POS;
  while (idx < argcount) {
    idx = GetMatchTypeIndex(idx, argcount, PFC_IPCTYPE_STRING);
    if (idx == 0) {
      pfc_log_debug("%s %d completed arguments read",
                    __FUNCTION__, __LINE__);
      return ret;
    }


    TcControllerResult ctrl_res;
    util_ret = tc::TcServerSessionUtils::get_string(sess_, idx,
                                                    ctrl_res.controller_id);
    if (util_ret != tc::TCUTIL_RET_SUCCESS) {
      pfc_log_error("%s %d TcServerSessionUtils failed with %d",
                    __FUNCTION__, __LINE__, util_ret);
      return TC_FAILURE;
    }

    idx++;
    util_ret = tc::TcServerSessionUtils::get_uint32(sess_, idx,
                                                    &ctrl_res.resp_code);
    if (util_ret != tc::TCUTIL_RET_SUCCESS) {
      pfc_log_error("%s %d TcServerSessionUtils failed with %d",
                    __FUNCTION__, __LINE__, util_ret);
      return TC_FAILURE;
    }

    idx++;
    util_ret = tc::TcServerSessionUtils::get_uint32(sess_, idx,
                                                    &ctrl_res.num_of_errors);
    if (util_ret != tc::TCUTIL_RET_SUCCESS) {
      pfc_log_error("%s %d TcServerSessionUtils failed with %d",
                    __FUNCTION__, __LINE__, util_ret);
      return TC_FAILURE;
    }
    uint32_t temp_idx = 0;
    temp_idx = idx;
    idx = GetMatchTypeIndex(idx, argcount, PFC_IPCTYPE_UINT64);
    if (idx != 0) {
 
      util_ret = tc::TcServerSessionUtils::get_uint64(sess_, idx,
                                                      &ctrl_res.commit_number);
      if (util_ret != tc::TCUTIL_RET_SUCCESS) {
        pfc_log_error("%s %d TcServerSessionUtils failed with %d",
                      __FUNCTION__, __LINE__, util_ret);
        return TC_FAILURE;
      }
      idx++;
      util_ret = tc::TcServerSessionUtils::get_uint64(sess_, idx,
                                                      &ctrl_res.commit_date);
      if (util_ret != tc::TCUTIL_RET_SUCCESS) {
        pfc_log_error("%s %d TcServerSessionUtils failed with %d",
                      __FUNCTION__, __LINE__, util_ret);
        return TC_FAILURE;
      }
      idx++;
      util_ret = tc::TcServerSessionUtils::get_string(sess_, idx,
                                              ctrl_res.commit_application);
      if (util_ret != tc::TCUTIL_RET_SUCCESS) {
        pfc_log_error("%s %d TcServerSessionUtils failed with %d",
                      __FUNCTION__, __LINE__, util_ret);
        return TC_FAILURE;
      }
    } else {
      pfc_log_info("No Commit Info Receieved");
      idx = temp_idx;
      ctrl_res.commit_number = 0;
      ctrl_res.commit_date = 0;
      ctrl_res.commit_application = "";
    }
    pfc_log_info("Cntrl ID:%s Resp Code:%d Num_Errs:%d Commit Number:%" PFC_PFMT_u64
                 "Commit Date:%" PFC_PFMT_u64 " Commit App:%s",
                 ctrl_res.controller_id.c_str(), ctrl_res.resp_code,
                 ctrl_res.num_of_errors, ctrl_res.commit_number,
                 ctrl_res.commit_date, ctrl_res.commit_application.c_str());

    if (ctrl_res.num_of_errors == 0) {
      pfc_log_debug("%s %d no errors ", __FUNCTION__, __LINE__);
      commit_phase_result_.push_back(ctrl_res);
      /* if no errors for the controller id, then the next index will
       * have next controller id so incrementing only by 1
       */
      continue;
    }

    key_map_.clear();
    pfc_log_info("%s %d num of errors %d",
                 __FUNCTION__, __LINE__, ctrl_res.num_of_errors);

    uint32_t local_kt = 0, err_count = 0;

    for (err_count = 0; err_count < ctrl_res.num_of_errors; err_count++) {
      idx = GetMatchTypeIndex(idx, argcount, PFC_IPCTYPE_UINT32);
      if (idx == 0) {
        pfc_log_debug("%s %d completed arguments read",
                    __FUNCTION__, __LINE__);
        return ret;
      }

      util_ret = tc::TcServerSessionUtils::get_uint32(sess_, idx,
                                                           &local_kt);
      if (util_ret != tc::TCUTIL_RET_SUCCESS) {
        pfc_log_error("%s %d TcServerSessionUtils failed with %d",
                      __FUNCTION__, __LINE__, util_ret);
        return TC_FAILURE;
      }

      // Push the local_kt into list
      ctrl_res.key_list.push_back(local_kt);

      // fill the map with keytree and index
      key_map_.insert(std::pair<uint32_t, uint32_t>(err_count, idx));
      pfc_log_info("err:%d key_index:%d ", err_count, idx);
    }
    // fill the map with controller id and key_map
    controller_key_map_.insert(std::pair<std::string, TcKeyTypeIndexMap>
                               (ctrl_res.controller_id, key_map_));

    // push the ctrl_res to commit_phase_result vector
    commit_phase_result_.push_back(ctrl_res);
  }

  return ret;
}

/**
 * @brief      Validation of commit sequence for UPPL/UPLL modules
 * @param[in]  oper_type operation type in commit process
 * @retval     TC_SUCCESS on valid oper state sequence
 * @retval     TC_FAILURE on invalid oper state sequence
 */
TcCommonRet TcLibModule::ValidateUpllUpplCommitSequence
                         (TcMsgOperType oper_type) {
  switch (oper_type) {
    case MSG_COMMIT_TRANS_START :
      if (oper_state_ == MSG_NONE)
        return TC_SUCCESS;
      else
        return TC_FAILURE;
    case MSG_COMMIT_VOTE :
      if (oper_state_ == MSG_COMMIT_TRANS_START)
        return TC_SUCCESS;
      else
        return TC_FAILURE;
    case MSG_COMMIT_VOTE_DRIVER_RESULT :
      if (oper_state_ == MSG_COMMIT_VOTE)
        return TC_SUCCESS;
      else
        return TC_FAILURE;
    case MSG_COMMIT_GLOBAL :
      if ((oper_state_ == MSG_COMMIT_VOTE_DRIVER_RESULT)||
          (oper_state_ == MSG_COMMIT_VOTE))
        return TC_SUCCESS;
      else
        return TC_FAILURE;
    case MSG_COMMIT_GLOBAL_DRIVER_RESULT :
      if (oper_state_ == MSG_COMMIT_GLOBAL)
        return TC_SUCCESS;
      else
        return TC_FAILURE;
    case MSG_COMMIT_TRANS_END :
      if ((oper_state_ == MSG_COMMIT_GLOBAL_DRIVER_RESULT)||
          (oper_state_ == MSG_COMMIT_TRANS_START)||
          (oper_state_ == MSG_COMMIT_VOTE)||
          (oper_state_ == MSG_COMMIT_GLOBAL)||
          (oper_state_ == MSG_COMMIT_ABORT))
        return TC_SUCCESS;
      else
        return TC_FAILURE;
    case MSG_COMMIT_ABORT :
      if ((oper_state_ == MSG_COMMIT_VOTE)||
          (oper_state_ == MSG_COMMIT_VOTE_DRIVER_RESULT))
        return TC_SUCCESS;
      else
        return TC_FAILURE;
    default:
      return TC_FAILURE;
  }
}

/**
 * @brief      Validation of commit sequence for driver modules
 * @param[in]  oper_type operation type in commit process
 * @retval     TC_SUCCESS on valid oper state sequence
 * @retval     TC_FAILURE on invalid oper state sequence
 */
TcCommonRet TcLibModule::ValidateDriverCommitSequence(TcMsgOperType oper_type) {
  switch (oper_type) {
    case MSG_COMMIT_TRANS_START :
      if (oper_state_ == MSG_NONE)
        return TC_SUCCESS;
      else
        return TC_FAILURE;
    case MSG_COMMIT_DRIVER_VOTE :
      if (oper_state_ == MSG_COMMIT_TRANS_START)
        return TC_SUCCESS;
      else
        return TC_FAILURE;
    case MSG_COMMIT_DRIVER_GLOBAL :
      if (oper_state_ == MSG_COMMIT_DRIVER_VOTE)
        return TC_SUCCESS;
      else
        return TC_FAILURE;
    case MSG_COMMIT_TRANS_END :
      if ((oper_state_ == MSG_COMMIT_DRIVER_GLOBAL)||
          (oper_state_ == MSG_COMMIT_DRIVER_VOTE)||
          (oper_state_ == MSG_COMMIT_TRANS_START)||
          (oper_state_ == MSG_COMMIT_ABORT))
        return TC_SUCCESS;
      else
        return TC_FAILURE;
    case MSG_COMMIT_ABORT :
      if (oper_state_ == MSG_COMMIT_DRIVER_VOTE)
        return TC_SUCCESS;
      else
        return TC_FAILURE;
    default:
      return TC_FAILURE;
  }
}

/**
 * @brief      Validation of commit sequence
 * @param[in]  oper_type operation type in commit proces
 * @retval     TC_SUCCESS on valid oper state sequence
 * @retval     TC_FAILURE on invalid oper state sequence
 */
TcCommonRet TcLibModule::ValidateCommitOperSequence(TcMsgOperType oper_type) {
  TcCommonRet ret = TC_SUCCESS;
  unc_keytype_ctrtype_t ctr_type = UNC_CT_UNKNOWN;

  ctr_type = GetControllerType();
  if (ctr_type == UNC_CT_UNKNOWN) {
    ret = ValidateUpllUpplCommitSequence(oper_type);
    pfc_log_debug("%s %d UPLL/UPPL validate commit sequence returned with %d",
                 __FUNCTION__, __LINE__, ret);
  } else {
    ret = ValidateDriverCommitSequence(oper_type);
    pfc_log_debug("%s %d driver validate commit sequence returned with %d",
                 __FUNCTION__, __LINE__, ret);
  }

  return ret;
}

/**
 * @brief      Validation of audit sequence for UPPL/UPLL modules
 * @param[in]  oper_type operation type in audit process
 * @retval     TC_SUCCESS on valid oper state sequence
 * @retval     TC_FAILURE on invalid oper state sequence
 */
TcCommonRet TcLibModule::ValidateUpllUpplAuditSequence
                         (TcMsgOperType oper_type) {
  switch (oper_type) {
    case MSG_AUDIT_START :
      if (oper_state_ == MSG_NONE || oper_state_ == MSG_AUDIT_CANCEL)
        return TC_SUCCESS;
      else
        return TC_FAILURE;
    case MSG_AUDIT_TRANS_START :
      if (oper_state_ == MSG_AUDIT_START || oper_state_ == MSG_AUDIT_CANCEL)
        return TC_SUCCESS;
      else
        return TC_FAILURE;
    case MSG_AUDIT_VOTE :
      if (oper_state_ == MSG_AUDIT_TRANS_START||oper_state_==MSG_AUDIT_CANCEL)
        return TC_SUCCESS;
      else
        return TC_FAILURE;
    case MSG_AUDIT_VOTE_DRIVER_RESULT :
      if (oper_state_ == MSG_AUDIT_VOTE || oper_state_ == MSG_AUDIT_CANCEL)
        return TC_SUCCESS;
      else
        return TC_FAILURE;
    case MSG_AUDIT_GLOBAL :
      if (oper_state_ == MSG_AUDIT_VOTE_DRIVER_RESULT)
        return TC_SUCCESS;
      else
        return TC_FAILURE;
    case MSG_AUDIT_GLOBAL_DRIVER_RESULT :
      if (oper_state_ == MSG_AUDIT_GLOBAL)
        return TC_SUCCESS;
      else
        return TC_FAILURE;
    case MSG_AUDIT_CANCEL:
      if (oper_state_ == MSG_AUDIT_START ||
          oper_state_ == MSG_AUDIT_TRANS_START ||
          oper_state_ == MSG_AUDIT_VOTE ||
          oper_state_ == MSG_AUDIT_VOTE_DRIVER_RESULT) {
        return TC_SUCCESS;
      } else {
        return TC_FAILURE;
      }
    case MSG_AUDIT_TRANS_END :
      if ((oper_state_ == MSG_AUDIT_GLOBAL_DRIVER_RESULT)||
          (oper_state_ == MSG_AUDIT_TRANS_START) ||
          (oper_state_ == MSG_AUDIT_VOTE) ||
          (oper_state_ == MSG_AUDIT_ABORT) ||
          (oper_state_ == MSG_AUDIT_CANCEL) )
        return TC_SUCCESS;
      else
        return TC_FAILURE;
    case MSG_AUDIT_END:
      if ((oper_state_ == MSG_AUDIT_START) ||
          (oper_state_ == MSG_AUDIT_TRANS_START) ||
          (oper_state_ == MSG_AUDIT_VOTE) ||
          (oper_state_ == MSG_AUDIT_VOTE_DRIVER_RESULT) ||
          (oper_state_ == MSG_AUDIT_TRANS_END) ||
          (oper_state_ == MSG_AUDIT_CANCEL))
        return TC_SUCCESS;
      else
        return TC_FAILURE;
    case MSG_AUDIT_ABORT :
      if ((oper_state_ == MSG_AUDIT_VOTE)||
          (oper_state_ == MSG_AUDIT_VOTE_DRIVER_RESULT) ||
          (oper_state_ == MSG_AUDIT_CANCEL))
        return TC_SUCCESS;
      else
        return TC_FAILURE;
    default:
      return TC_FAILURE;
  }
}

/**
 * @brief      Validation of audit sequence for driver modules
 * @param[in]  oper_type operation type in audit process
 * @retval     TC_SUCCESS on valid oper state sequence
 * @retval     TC_FAILURE on invalid oper state sequence
 */
TcCommonRet TcLibModule::ValidateDriverAuditSequence(TcMsgOperType oper_type) {
  switch (oper_type) {
    case MSG_AUDIT_START :
      /* If audit of controller does not belongs to that driver
       * audit start should not be recieved
       */
      if (audit_in_progress_ == PFC_TRUE) {
        if (oper_state_ == MSG_NONE || oper_state_ == MSG_AUDIT_CANCEL)
          return TC_SUCCESS;
        else
          return TC_FAILURE;
      } else {
        return TC_FAILURE;
      }

    case MSG_AUDIT_TRANS_START :
      /* if audit of controller does not belongs to that driver
       * audit start will not be recieved, directly audit transaction
       * start will be sent. below condition is for that validation
       */
      if ((audit_in_progress_ == PFC_TRUE) &&
          (oper_state_ == MSG_AUDIT_START || oper_state_ == MSG_AUDIT_CANCEL))
        return TC_SUCCESS;
      else if ((audit_in_progress_ == PFC_FALSE) &&
               (oper_state_ == MSG_NONE))
        return TC_SUCCESS;
      else
        return TC_FAILURE;
    case MSG_AUDIT_DRIVER_VOTE :
      if (oper_state_ == MSG_AUDIT_TRANS_START ||
          oper_state_ == MSG_AUDIT_CANCEL)
        return TC_SUCCESS;
      else
        return TC_FAILURE;
    case MSG_AUDIT_CANCEL:
      if (oper_state_ == MSG_AUDIT_START ||
          oper_state_ == MSG_AUDIT_TRANS_START ||
          oper_state_ == MSG_AUDIT_DRIVER_VOTE) {
        return TC_SUCCESS;
      } else {
        return TC_FAILURE;
      }
    case MSG_AUDIT_DRIVER_GLOBAL :
      if (oper_state_ == MSG_AUDIT_DRIVER_VOTE)
        return TC_SUCCESS;
      else
        return TC_FAILURE;
    case MSG_AUDIT_TRANS_END :
      if ((oper_state_ == MSG_AUDIT_DRIVER_GLOBAL)||
          (oper_state_ == MSG_AUDIT_TRANS_START)||
          (oper_state_ == MSG_AUDIT_DRIVER_VOTE)||
          (oper_state_ == MSG_AUDIT_ABORT) ||
          (oper_state_ == MSG_AUDIT_CANCEL))
        return TC_SUCCESS;
      else
        return TC_FAILURE;
    case MSG_AUDIT_END:
      /* If audit of controller does not belongs to that driver
       * audit end should not be recieved
       */
      if (audit_in_progress_ == PFC_TRUE) {
        if ((oper_state_ == MSG_AUDIT_START) ||
            (oper_state_ == MSG_AUDIT_TRANS_START) ||
            (oper_state_ == MSG_AUDIT_VOTE) ||
            (oper_state_ == MSG_AUDIT_DRIVER_VOTE) ||
            (oper_state_ == MSG_AUDIT_VOTE_DRIVER_RESULT) ||
            (oper_state_ == MSG_AUDIT_TRANS_END) ||
            (oper_state_ == MSG_AUDIT_CANCEL))
          return TC_SUCCESS;
        else
          return TC_FAILURE;
      } else {
        return TC_FAILURE;
      }

    case MSG_AUDIT_ABORT :
      if (oper_state_ == MSG_AUDIT_DRIVER_VOTE ||
          oper_state_ == MSG_AUDIT_CANCEL)
        return TC_SUCCESS;
      else
        return TC_FAILURE;
    default:
      return TC_FAILURE;
  }
}

/**
 * @brief      Validation of audit sequence for driver modules
 * @param[in]  oper_type operation type in audit process
 * @retval     TC_SUCCESS on valid oper state sequence
 * @retval     TC_FAILURE on invalid oper state sequence
 */
TcCommonRet TcLibModule::ValidateAuditOperSequence(TcMsgOperType oper_type) {
  TcCommonRet ret = TC_SUCCESS;
  unc_keytype_ctrtype_t ctr_type = UNC_CT_UNKNOWN;

  if ( oper_type == MSG_AUDIT_CANCEL && oper_state_ == MSG_NONE ) {
    pfc_log_info("%s AUDIT_CANCEL received before AUDIT_START",
                 __FUNCTION__);
    SetHandleCancelAuditInTclib(PFC_TRUE);
  }

  ctr_type = GetControllerType();
  if (ctr_type == UNC_CT_UNKNOWN) {
    ret = ValidateUpllUpplAuditSequence(oper_type);
    pfc_log_debug("%s %d UPLL/UPPL validate audit sequence returned with %d;"
                 "opertype = %d operstate = %d",
                 __FUNCTION__, __LINE__, ret, oper_type, oper_state_);
  } else {
    ret = ValidateDriverAuditSequence(oper_type);
    pfc_log_debug("%s %d driver validate audit sequence returned with %d;"
                 "opertype = %d operstate = %d",
                 __FUNCTION__, __LINE__, ret, oper_type, oper_state_);
  }

  return ret;
}

/**
 * @brief      Validation of oper type sequence
 * @param[in]  oper_type operation type in commit/audit process
 * @retval     TC_SUCCESS on valid oper state sequence
 * @retval     TC_FAILURE on invalid oper state sequence
 */
TcCommonRet TcLibModule::ValidateOperTypeSequence(TcMsgOperType oper_type) {
  TcCommonRet ret = TC_SUCCESS;

  pfc::core::ScopedMutex m(tclib_validate_seq_mutex_);
  /* Validation of oper_type */
  switch (oper_type) {
    case MSG_COMMIT_TRANS_START :
    case MSG_COMMIT_VOTE :
    case MSG_COMMIT_DRIVER_VOTE :
    case MSG_COMMIT_VOTE_DRIVER_RESULT :
    case MSG_COMMIT_GLOBAL :
    case MSG_COMMIT_DRIVER_GLOBAL :
    case MSG_COMMIT_GLOBAL_DRIVER_RESULT :
    case MSG_COMMIT_TRANS_END :
    case MSG_COMMIT_ABORT :
      ret = ValidateCommitOperSequence(oper_type);
      break;
    case MSG_AUDIT_START :
    case MSG_AUDIT_TRANS_START :
    case MSG_AUDIT_VOTE :
    case MSG_AUDIT_DRIVER_VOTE :
    case MSG_AUDIT_VOTE_DRIVER_RESULT :
    case MSG_AUDIT_GLOBAL :
    case MSG_AUDIT_DRIVER_GLOBAL :
    case MSG_AUDIT_GLOBAL_DRIVER_RESULT :
    case MSG_AUDIT_TRANS_END :
    case MSG_AUDIT_END :
    case MSG_AUDIT_ABORT :
    case MSG_AUDIT_CANCEL:
      ret = ValidateAuditOperSequence(oper_type);
      break;
    default:
      return TC_FAILURE;
  }

  pfc_log_debug("%s %d old oper state %s new oper state %s",
                __FUNCTION__, __LINE__, TcMsgOperTypeString[oper_state_],
                TcMsgOperTypeString[oper_type]);
  //PFC_ASSERT(ret == TC_SUCCESS);
  if (ret != TC_SUCCESS) {
    if (oper_type == MSG_AUDIT_CANCEL) {
      pfc_log_warn("%s Received op %s is not expected in %s state."
                   "Skipping processing of MSG_AUDIT_CANCEL",
                   __FUNCTION__, TcMsgOperTypeString[oper_type],
                   TcMsgOperTypeString[oper_state_]);
      return TC_SUCCESS;
    } else {
      pfc_log_fatal("%s Received op %s is not expected in %s state.",
                    __FUNCTION__, TcMsgOperTypeString[oper_type],
                    TcMsgOperTypeString[oper_state_]);
    }
  }

  oper_state_ = oper_type;
  pfc_log_info("%s %d Updated the oper type with %s",
               __FUNCTION__, __LINE__, TcMsgOperTypeString[oper_state_]);
  return ret;
}

/**
 * @brief      Is Read key value allowed in current oper state
 * @retval     TC_API_COMMON_SUCCESS on valid oper state
 * @retval     TC_API_COMMON_FAILURE on invalid oper state
 */
TcApiCommonRet TcLibModule::IsReadKeyValueAllowed() {
  /* Validation of oper state for reading key value towards the TC */
  switch (oper_state_) {
    case MSG_COMMIT_VOTE_DRIVER_RESULT:
    case MSG_COMMIT_GLOBAL_DRIVER_RESULT:
    case MSG_AUDIT_VOTE_DRIVER_RESULT:
    case MSG_AUDIT_GLOBAL_DRIVER_RESULT:
      return TC_API_COMMON_SUCCESS;
    default:
      return TC_API_COMMON_FAILURE;
  }
}

/**
 * @brief      Is write key value allowed in current oper state
 * @retval     TC_API_COMMON_SUCCESS on valid oper state
 * @retval     TC_API_COMMON_FAILURE on invalid oper state
 */
TcApiCommonRet TcLibModule::IsWriteKeyValueAllowed() {
  /* Validation of oper state for writing key value towards the TC */
  switch (oper_state_) {
    case MSG_COMMIT_TRANS_START:
    case MSG_COMMIT_DRIVER_VOTE:
    case MSG_COMMIT_DRIVER_GLOBAL:
    case MSG_COMMIT_VOTE_DRIVER_RESULT:
    case MSG_COMMIT_GLOBAL_DRIVER_RESULT:
    case MSG_AUDIT_TRANS_START:
    case MSG_AUDIT_DRIVER_VOTE:
    case MSG_AUDIT_DRIVER_GLOBAL:
    case MSG_AUDIT_VOTE_DRIVER_RESULT:
    case MSG_AUDIT_GLOBAL_DRIVER_RESULT:
      return TC_API_COMMON_SUCCESS;
    default:
      return TC_API_COMMON_FAILURE;
  }
}


/**
 * @brief      Get config mode based on key session_id and config_id
 * @param[in]  session_id session id of notify session config data
 * @param[in]  config_id -  notify session config id
 * @param[out] config_mode - reterive from tc_notify_config_data_map
 * @param[out] vtn_name - reterive from tc_notify_config_data_map
 *                        if config_mode is VTN
 * @retval     TC_API_COMMON_SUCCESS on successful updation
 * @retval     TC_INVALID_SESSION_ID on session_id not present in map
 * @retval     TC_INVALID_CONFIG_ID on invalid Id
 */
TcApiCommonRet TcLibModule::TcLibGetConfigMode(uint32_t session_id,
                                          uint32_t config_id,
                                          TcConfigMode& config_mode,
                                          std::string& vtn_name) {

  config_mode = TC_CONFIG_INVALID;
  vtn_name.clear();

  pfc::core::ScopedMutex m(tc_notify_config_data_map_lock_);

  if (tc_notify_config_data_map_.find(session_id) == 
                                   tc_notify_config_data_map_.end())  {
    return TC_INVALID_SESSION_ID;
  }
  if (config_id != tc_notify_config_data_map_[session_id].config_id_)  {
    return TC_INVALID_CONFIG_ID;
  }
  config_mode = tc_notify_config_data_map_[session_id].config_mode_;
  vtn_name = tc_notify_config_data_map_[session_id].vtn_name_;

  return TC_API_COMMON_SUCCESS; 
}

/**
 * @brief      Get notify config data based on key session_id
 * @param[in]  session_id session id of notify session config data
 * @param[out] config_id - reterive from tc_notify_config_data_map
 * @param[out] config_mode - reterive from tc_notify_config_data_map
 * @param[out] vtn_name - reterive from tc_notify_config_data_map
 *                        if config_mode is VTN
 * @retval     TC_SUCCESS on successful updation
 * @retval     TC_FAILURE on any failure
 */
TcCommonRet TcLibModule::GetConfigData(uint32_t session_id,
                                       uint32_t& config_id,
                                       TcConfigMode& config_mode,
                                       std::string& vtn_name) {
  pfc::core::ScopedMutex m(tc_notify_config_data_map_lock_);
  config_id = 0;
  config_mode = TC_CONFIG_INVALID;
  vtn_name.clear();

  // Check only for platform modules. No need to check for drivers
  if (GetControllerType() == UNC_CT_UNKNOWN) {
    TcNotifyConfigDataMap::iterator it =
            tc_notify_config_data_map_.find(session_id);

    if (it == tc_notify_config_data_map_.end()) {
      pfc_log_error("%s Session-id[%u] not existing in config-map",
                  __FUNCTION__, session_id);
      return TC_FAILURE;
    }
    config_id = it->second.config_id_;
    config_mode = it->second.config_mode_;
    vtn_name = it->second.vtn_name_;
  }
  return TC_SUCCESS; 
}


/**
 * @brief     Updation of notify config data based on key session_id
 * @param[in] session_id - session id of notify session config data
 * @param[in] config_id - config id of notify session config data
 * @param[in] config_mode - config mode of notify session config data
 * @param[in] vtn_name - vtn name of notify session config data
 * @retval    TC_SUCCESS on successful updation
 * @retval    TC_FAILURE on any failure
 */

TcCommonRet TcLibModule::UpdateNotifyConfigData(uint32_t session_id,
                                                uint32_t config_id,
                                                TcConfigMode config_mode,
                                                std::string vtn_name) {
  pfc::core::ScopedMutex m(tc_notify_config_data_map_lock_);

  if (config_mode == TC_CONFIG_GLOBAL && config_id != 0) {
    pfc_log_info("%s Got global config mode acquire. Clear all other entries",
                 __FUNCTION__);
    tc_notify_config_data_map_.clear();
  }
  tc_notify_config_data_map_[session_id].session_id_ = session_id;
  tc_notify_config_data_map_[session_id].config_id_ = config_id;
  tc_notify_config_data_map_[session_id].config_mode_ = config_mode;
  tc_notify_config_data_map_[session_id].vtn_name_ = vtn_name;
  pfc_log_info("%s Sess-id[%u] conf-id[%u] conf-mode[%d], vtn-name[%s] added",
               __FUNCTION__,
               session_id, config_id, config_mode, vtn_name.c_str());
  return TC_SUCCESS; 
}

/**
 * @brief      Updation of session_id and config_id
 * @retval     TC_SUCCESS on successful updation
 * @retval     TC_FAILURE on any failure
 */
TcCommonRet TcLibModule::
NotifySessionConfig(pfc::core::ipc::ServerSession *sess) {
  uint32_t argcount = 0, idx = 0;
  tc::TcUtilRet util_ret = tc::TCUTIL_RET_SUCCESS;

  pfc::core::ScopedMutex m(tclib_mutex_);

  /*read from the session and update the session and config id*/
  if (sess != NULL) {
    argcount = sess->getArgCount();
    pfc_log_debug("%s %d session arg count %d",
                 __FUNCTION__, __LINE__, argcount);
  } else {
    pfc_log_error("%s %d session pointer is NULL", __FUNCTION__, __LINE__);
    return TC_FAILURE;
  }
  if (argcount == IPC_DEFAULT_ARG_COUNT) {
    pfc_log_error("%s %d Argument count is NULL", __FUNCTION__, __LINE__);
    return TC_FAILURE;
  }

  uint32_t session_id = 0;
  util_ret = tc::TcServerSessionUtils::get_uint32(sess, idx,
                                                       &session_id);
  if (util_ret != tc::TCUTIL_RET_SUCCESS) {
    pfc_log_error("%s %d TcServerSessionUtils get session_id failed with %d",
                  __FUNCTION__, __LINE__, util_ret);
    return TC_FAILURE;
  }

  idx++;
  uint32_t config_id = 0;
  util_ret = tc::TcServerSessionUtils::get_uint32(sess, idx, &config_id);
  if (util_ret != tc::TCUTIL_RET_SUCCESS) {
    pfc_log_error("%s %d TcServerSessionUtils get config_id failed with %d",
                  __FUNCTION__, __LINE__, util_ret);
    return TC_FAILURE;
  }
  
  idx++;
  uint8_t tc_mode = 0;
  util_ret = tc::TcServerSessionUtils::get_uint8(sess, idx, &tc_mode);
  TcConfigMode config_mode = (TcConfigMode)tc_mode;
  if (util_ret != tc::TCUTIL_RET_SUCCESS) {
    pfc_log_error("%s %d TcServerSessionUtils get config_mode failed with %d",
                  __FUNCTION__, __LINE__, util_ret);
    return TC_FAILURE;
  }

  idx++;
  std::string vtn_name;
  util_ret = tc::TcServerSessionUtils::get_string(sess, idx, vtn_name);
  if (util_ret != tc::TCUTIL_RET_SUCCESS) {
    pfc_log_error("%s %d TcServerSessionUtils get vtn-name failed with %d",
                  __FUNCTION__, __LINE__, util_ret);
    return TC_FAILURE;
  }

  if (!config_id)  {
    pfc::core::ScopedMutex m(tc_notify_config_data_map_lock_);
    tc_notify_config_data_map_.erase(session_id);
    pfc_log_info("%s Sess-id[%u] removed from config-map",
                 __FUNCTION__, session_id);
  } else {
    TcCommonRet ret = UpdateNotifyConfigData(session_id, config_id, 
                                                config_mode, vtn_name);
    if (ret != TC_SUCCESS) {
      pfc_log_error("%s %d UpdateNotifyConfigData failed with %d",
                    __FUNCTION__, __LINE__, ret);
      return TC_FAILURE;
    }
  }

  pfc_log_debug("%s %d Updated session_id %u config_id %u "
                "config_mode %d vtn_name %s",
                __FUNCTION__, __LINE__, session_id, config_id,
                config_mode, vtn_name.c_str());

  return TC_SUCCESS;
}

/**
 * @brief      commit transaction start/end operations invoking
 * @param[in]  oper_type operation type in commit trans start/end process
 * @param[in]  commit_trans_msg structure variable after reading from session
 * @retval     TC_SUCCESS on handle operation success
 * @retval     TC_FAILURE on handle operation failure
 */
TcCommonRet TcLibModule::CommitTransStartEnd(TcMsgOperType oper_type,
                                             TcCommitTransactionMsg
                                             commit_trans_msg) {
  TcCommonRet ret = TC_SUCCESS;

  uint32_t config_id = 0;
  TcConfigMode config_mode = TC_CONFIG_INVALID;
  std::string vtn_name;

  if (pTcLibInterface_ == NULL) {
    pfc_log_error("%s %d TcLibInterface not registered",
                  __FUNCTION__, __LINE__);
    return TC_FAILURE;
  }

  // Config data not available for driver modules (tclib)
  // Send config_mode as TC_CONFIG_INVALID and empty vtn-name for drivers
  ret = GetConfigData(commit_trans_msg.session_id, config_id, 
                                       config_mode, vtn_name);

  if (oper_type == MSG_COMMIT_TRANS_START) {
    /*
     * On TC_SUCCESS, no key type information will be written in a response.
     * On TC_FAILURE, key type information will be filled from the
     * HandleCommitTransactionStart using the provided TcLibWrite APIs.
     */
    ret = pTcLibInterface_->HandleCommitTransactionStart(
                                          commit_trans_msg.session_id,
                                          commit_trans_msg.config_id,
                                                         config_mode,
                                                            vtn_name);

    if (ret != TC_SUCCESS) {
      pfc_log_error("%s %d oper_type %d Handler returned with %d",
                    __FUNCTION__, __LINE__, oper_type, ret);
      ReleaseTransactionResources();
    }
  } else if (oper_type == MSG_COMMIT_TRANS_END) {
    ret = pTcLibInterface_->HandleCommitTransactionEnd(
                                       commit_trans_msg.session_id,
                                       commit_trans_msg.config_id,
                                       config_mode, vtn_name,
                                       commit_trans_msg.end_result);
    /* Release the transaction resources used*/
    ReleaseTransactionResources();
  } else {
    pfc_log_error("%s %d Invalid oper type for this interface",
                  __FUNCTION__, __LINE__);
    ret = TC_FAILURE;
  }
  pfc_log_info("%s %d oper_type %d Handler returned with %d",
               __FUNCTION__, __LINE__, oper_type, ret);
  return ret;
}

/**
 * @brief      commit transaction vote/global operations towards UPLL/UPPL
 * @param[in]  oper_type operation type in commit trans vote/global process
 * @param[in]  commit_trans_msg structure variable after reading from session
 * @retval     TC_SUCCESS on handle operation success
 * @retval     TC_FAILURE on handle operation failure
 */
TcCommonRet TcLibModule::CommitVoteGlobal(TcMsgOperType oper_type,
                                          TcCommitTransactionMsg
                                          commit_trans_msg) {
  TcCommonRet ret = TC_SUCCESS;
  tc::TcUtilRet util_ret = tc::TCUTIL_RET_SUCCESS;
  TcDriverInfoMap driver_info;
  uint32_t config_id = 0;
  TcConfigMode config_mode = TC_CONFIG_INVALID;
  std::string vtn_name;

  if (pTcLibInterface_ == NULL) {
    pfc_log_error("%s %d TcLibInterface not registered",
                  __FUNCTION__, __LINE__);
    return TC_FAILURE;
  }

  // Get config-mode and vtn-name from store and pass to platform
  ret = GetConfigData(commit_trans_msg.session_id, config_id, 
                                       config_mode, vtn_name);

  if (oper_type == MSG_COMMIT_VOTE) {
    ret = pTcLibInterface_->HandleCommitVoteRequest(
                                       commit_trans_msg.session_id,
                                       commit_trans_msg.config_id,
                                       config_mode, vtn_name,
                                       driver_info);
  } else if (oper_type == MSG_COMMIT_GLOBAL) {
    ret = pTcLibInterface_->HandleCommitGlobalCommit(
                                       commit_trans_msg.session_id,
                                       commit_trans_msg.config_id,
                                       config_mode, vtn_name,
                                       driver_info);
  } else {
    pfc_log_error("%s %d Invalid oper type for this interface",
                  __FUNCTION__, __LINE__);
    return TC_FAILURE;
  }

  if (ret != TC_SUCCESS) {
    pfc_log_error("%s %d HandleCommitvoteRequest Failed with %d",
                  __FUNCTION__, __LINE__, ret);
    return ret;
  }

  pfc_log_info("%s %d oper_type %d Handler returned with %d",
               __FUNCTION__, __LINE__, oper_type, ret);

  // write the driver info map into session as response
  if (driver_info.empty()) {
    pfc_log_info("%s %d Driver info is empty", __FUNCTION__, __LINE__);
    return ret;
  }

  std::map<unc_keytype_ctrtype_t, std::vector<std::string> >::iterator m_it;
  std::vector<std::string>::iterator v_it;
  std::vector<std::string> ctrl_vector;
  unc_keytype_ctrtype_t ctr_type;
  uint8_t ctrl_count = 0;

  // setting the driver count
  util_ret = tc::TcServerSessionUtils::set_uint8(sess_,
                                                 driver_info.size());
  if (util_ret != tc::TCUTIL_RET_SUCCESS) {
    pfc_log_error("%s %d TcServerSessionUtils failed with %d",
                  __FUNCTION__, __LINE__, util_ret);
    return TC_FAILURE;
  }

  for (m_it = driver_info.begin(); m_it != driver_info.end(); m_it++) {
    ctr_type = m_it->first;
    util_ret = tc::TcServerSessionUtils::set_uint8(sess_, ctr_type);
    if (util_ret != tc::TCUTIL_RET_SUCCESS) {
      pfc_log_error("%s %d TcServerSessionUtils failed with %d",
                    __FUNCTION__, __LINE__, util_ret);
      return TC_FAILURE;
    }

    ctrl_vector = m_it->second;
    ctrl_count = ctrl_vector.size();  // controller count
    util_ret = tc::TcServerSessionUtils::set_uint8(sess_, ctrl_count);
    if (util_ret != tc::TCUTIL_RET_SUCCESS) {
      pfc_log_error("%s %d TcServerSessionUtils failed with %d",
                    __FUNCTION__, __LINE__, util_ret);
      return TC_FAILURE;
    }

    for (v_it = ctrl_vector.begin(); v_it != ctrl_vector.end(); v_it++) {
      util_ret = tc::TcServerSessionUtils::set_string(sess_, *v_it);
      if (util_ret != tc::TCUTIL_RET_SUCCESS) {
        pfc_log_error("%s %d TcServerSessionUtils failed with %d",
                      __FUNCTION__, __LINE__, util_ret);
        return TC_FAILURE;
      }
    }
  }

  pfc_log_debug("%s %d Driver info filled driver_count %"PFC_PFMT_SIZE_T
               " ctrl_count %d", __FUNCTION__, __LINE__,
               driver_info.size(), ctrl_count);
  return ret;
}

/**
 * @brief      commit transaction
 * @retval     TC_SUCCESS on handle operation success
 * @retval     TC_FAILURE on handle operation failure
 */
TcCommonRet TcLibModule::CommitTransaction() {
  pfc::core::ScopedMutex m(tclib_ipc_control_mutex_);

  TcCommonRet ret = TC_SUCCESS;
  TcApiCommonRet val_ret = TC_API_COMMON_SUCCESS;
  TcCommitTransactionMsg commit_trans_msg;
  uint32_t config_id = 0;
  std::string vtn_name;
  TcConfigMode config_mode = TC_CONFIG_INVALID;

  ret = TcLibMsgUtil::GetCommitTransactionMsg(sess_, commit_trans_msg);
  if (ret != TC_SUCCESS) {
    pfc_log_error("%s %d Get Msg Util error %d", __FUNCTION__, __LINE__, ret);
    return ret;
  }

  pfc_log_info("%s OperType:%s, session[%u], config[%u], end-result[%s]",
               __FUNCTION__, TcMsgOperTypeString[commit_trans_msg.oper_type],
               commit_trans_msg.session_id, commit_trans_msg.config_id,
               (commit_trans_msg.oper_type == MSG_COMMIT_TRANS_END?
                 (commit_trans_msg.end_result == TRANS_END_SUCCESS?
                  "SUCCESS":"FAILURE"):""));

  if (ValidateOperTypeSequence(commit_trans_msg.oper_type)!= TC_SUCCESS) {
    pfc_log_error("%s %d Operation type sequence invalid",
                  __FUNCTION__, __LINE__);
    return TC_FAILURE;
  }

  if (GetControllerType() == UNC_CT_UNKNOWN) {
    ret = GetConfigData(commit_trans_msg.session_id, config_id, 
                        config_mode, vtn_name);

    val_ret = TcLibValidateUpdateMsg(commit_trans_msg.session_id,
                                     commit_trans_msg.config_id, 
                                     config_mode, vtn_name);
    if (val_ret != TC_API_COMMON_SUCCESS) {
      pfc_log_error("%s %d Invalid session or config id %d",
                    __FUNCTION__, __LINE__, val_ret);
      return TC_FAILURE;
    }
  }
  switch (commit_trans_msg.oper_type) {
    case MSG_COMMIT_TRANS_START:
    case MSG_COMMIT_TRANS_END:
      ret = CommitTransStartEnd(commit_trans_msg.oper_type, commit_trans_msg);
      break;
    case MSG_COMMIT_VOTE:
    case MSG_COMMIT_GLOBAL:
      ret = CommitVoteGlobal(commit_trans_msg.oper_type, commit_trans_msg);
      break;
    default:
      pfc_log_error("%s %d Invalid Operation", __FUNCTION__, __LINE__);
      ret = TC_FAILURE;
  }

  return ret;
}

/**
 * @brief      commit vote/global transaction for driver modules
 * @retval     TC_SUCCESS on handle operation success
 * @retval     TC_FAILURE on handle operation failure
 */
TcCommonRet TcLibModule::CommitDriverVoteGlobal() {
  pfc::core::ScopedMutex m(tclib_ipc_control_mutex_);

  TcCommonRet ret = TC_SUCCESS;
  TcCommitDrvVoteGlobalMsg drv_vote_global_msg;

  if (pTcLibInterface_ == NULL) {
    pfc_log_error("%s %d TcLibInterface not registered",
                  __FUNCTION__, __LINE__);
    return TC_FAILURE;
  }
  ret = TcLibMsgUtil::GetCommitDrvVoteGlobalMsg(sess_, drv_vote_global_msg);
  if (ret != TC_SUCCESS) {
    pfc_log_error("%s %d Get Msg Util error %d", __FUNCTION__, __LINE__, ret);
    return ret;
  }

  if (ValidateOperTypeSequence(drv_vote_global_msg.oper_type) != TC_SUCCESS) {
    pfc_log_error("%s %d Operation type sequence invalid",
                  __FUNCTION__, __LINE__);
    return TC_FAILURE;
  }

  if (drv_vote_global_msg.oper_type == MSG_COMMIT_DRIVER_VOTE) {
    ret = pTcLibInterface_->HandleCommitVoteRequest(
                                       drv_vote_global_msg.session_id,
                                       drv_vote_global_msg.config_id,
                                       drv_vote_global_msg.cntrl_list);
  } else if (drv_vote_global_msg.oper_type == MSG_COMMIT_DRIVER_GLOBAL) {
    ret = pTcLibInterface_->HandleCommitGlobalCommit(
                                       drv_vote_global_msg.session_id,
                                       drv_vote_global_msg.config_id,
                                       drv_vote_global_msg.cntrl_list);
  }

  /* Response for the above service requests are handled inside
   * the respective functions. TcLibWrite APIs are used to write
   * the response messages to the session
   */

  pfc_log_info("%s %d oper_type %d Handler returned with %d",
               __FUNCTION__, __LINE__, drv_vote_global_msg.oper_type, ret);
  return ret;
}

/**
 * @brief      commit driver result for UPLL/UPPL modules
 * @retval     TC_SUCCESS on handle operation success
 * @retval     TC_FAILURE on handle operation failure
 */
TcCommonRet TcLibModule::CommitDriverResult() {
  pfc::core::ScopedMutex m(tclib_ipc_control_mutex_);

  TcCommonRet ret = TC_SUCCESS;
  TcApiCommonRet val_ret = TC_API_COMMON_SUCCESS;
  TcCommitDrvResultMsg drvresult_msg;
  TcConfigMode config_mode = TC_CONFIG_INVALID;
  std::string vtn_name;
  uint32_t config_id = 0;

  if (pTcLibInterface_ == NULL) {
    pfc_log_error("%s %d TcLibInterface not registered",
                  __FUNCTION__, __LINE__);
    return TC_FAILURE;
  }

  ret = TcLibMsgUtil::GetCommitDrvResultMsg(sess_, drvresult_msg);
  if (ret != TC_SUCCESS) {
    pfc_log_error("%s %d Get Msg Util error %d", __FUNCTION__, __LINE__, ret);
    return ret;
  }

  if (ValidateOperTypeSequence(drvresult_msg.oper_type) != TC_SUCCESS) {
    pfc_log_error("%s %d Operation type sequence invalid",
                  __FUNCTION__, __LINE__);
    return TC_FAILURE;
  }

  ret = GetConfigData(drvresult_msg.session_id, config_id, 
                                       config_mode, vtn_name);

  val_ret = TcLibValidateUpdateMsg(drvresult_msg.session_id,
                                   drvresult_msg.config_id,
                                   config_mode, vtn_name);
  if (val_ret != TC_API_COMMON_SUCCESS) {
    pfc_log_error("%s %d Invalid session or config id %d",
                  __FUNCTION__, __LINE__, val_ret);
    return TC_FAILURE;
  }

  ret = UpdateControllerKeyList();
  if (ret != TC_SUCCESS) {
    pfc_log_error("%s %d UpdateControllerKeyList failed %d",
                  __FUNCTION__, __LINE__, ret);
    return ret;
  }
  ret =  pTcLibInterface_->HandleCommitDriverResult(
                                      drvresult_msg.session_id,
                                      drvresult_msg.config_id,
                                      config_mode, vtn_name,
                                      drvresult_msg.phase,
                                      commit_phase_result_);
  if (ret != TC_SUCCESS) {
    pfc_log_error("%s %d HandleCommitDriverResult failed %d",
                  __FUNCTION__, __LINE__, ret);
    return ret;
  }
  /* Response for the above service request are handled inside
   * the handle function itself. TcLibWrite APIs are used to
   * write the response messages to the session
   */

  pfc_log_info("%s %d oper_type %d Handler returned with %d",
               __FUNCTION__, __LINE__, drvresult_msg.oper_type, ret);
  return ret;
}

/**
 * @brief      commit global abort
 * @retval     TC_SUCCESS on handle operation success
 * @retval     TC_FAILURE on handle operation failure
 */
TcCommonRet TcLibModule::CommitGlobalAbort() {
  pfc::core::ScopedMutex m(tclib_ipc_control_mutex_);

  TcCommonRet ret = TC_SUCCESS;
  TcApiCommonRet val_ret = TC_API_COMMON_SUCCESS;
  TcCommitGlobalAbortMsg abort_msg;
  uint32_t config_id = 0;
  std::string vtn_name;
  TcConfigMode config_mode = TC_CONFIG_INVALID;

  if (pTcLibInterface_ == NULL) {
    pfc_log_error("%s %d TcLibInterface not registered",
                  __FUNCTION__, __LINE__);
    return TC_FAILURE;
  }

  ret = TcLibMsgUtil::GetCommitGlobalAbortMsg(sess_, abort_msg);
  if (ret != TC_SUCCESS) {
    pfc_log_error("%s %d Get Msg Util error %d", __FUNCTION__, __LINE__, ret);
    return ret;
  }

  if (ValidateOperTypeSequence(abort_msg.oper_type) != TC_SUCCESS) {
    pfc_log_error("%s %d Operation type sequence invalid",
                  __FUNCTION__, __LINE__);
    return TC_FAILURE;
  }

  if (GetControllerType() == UNC_CT_UNKNOWN) {
    ret = GetConfigData(abort_msg.session_id, config_id, config_mode, vtn_name);

    val_ret = TcLibValidateUpdateMsg(abort_msg.session_id,
                                     abort_msg.config_id,
                                     config_mode, vtn_name);
    if (val_ret != TC_API_COMMON_SUCCESS) {
      pfc_log_error("%s %d Invalid session or config id %d",
                    __FUNCTION__, __LINE__, val_ret);
      return TC_FAILURE;
    }
  }
  ret = pTcLibInterface_->HandleCommitGlobalAbort(
                                     abort_msg.session_id,
                                     abort_msg.config_id,
                                     config_mode, vtn_name,
                                     abort_msg.commit_oper_phase);
  if (ret != TC_SUCCESS) {
    pfc_log_error("%s %d HandleCommitGlobalAbort failed %d",
                  __FUNCTION__, __LINE__, ret);
    return ret;
  }
  pfc_log_info("%s %d oper_type %d Handler returned with %d",
               __FUNCTION__, __LINE__, abort_msg.oper_type, ret);
  return ret;
}

// Audit Operations
/**
 * @brief      audit start/end, transaction start/end operations invoking
 * @param[in]  oper_type operation type in audit trans start/end process
 * @param[in]  audit_trans_msg structure variable after reading from session
 * @retval     TC_SUCCESS on handle operation success
 * @retval     TC_FAILURE on handle operation failure
 */
TcCommonRet TcLibModule::AuditTransStartEnd(TcMsgOperType oper_type,
                                            TcAuditTransactionMsg
                                            audit_trans_msg) {
  TcCommonRet ret = TC_SUCCESS;

  if (pTcLibInterface_ == NULL) {
    pfc_log_error("%s %d TcLibInterface not registered",
                  __FUNCTION__, __LINE__);
    return TC_FAILURE;
  }

  switch (oper_type) {
    case MSG_AUDIT_START :
    {
      pfc::core::ScopedMutex m(tclib_validate_seq_mutex_);
      if (IsHandleCancelAuditInTclib() == PFC_TRUE || 
          oper_state_ == MSG_AUDIT_CANCEL) {
        pfc_log_info("%s %d Handle in TCLIB. Not calling HandleAuditStart",
                     __FUNCTION__, __LINE__);
        m.unlock();
        return TC_CANCELLED_AUDIT;
      }
      m.unlock();

      if (GetControllerType() != UNC_CT_UNKNOWN) {  //For Driver
          ret = pTcLibInterface_->HandleAuditStart(
                                      audit_trans_msg.session_id,
                                      audit_trans_msg.driver_id,
                                      audit_trans_msg.controller_id,
                                      audit_trans_msg.reconnect_controller,
                                      audit_trans_msg.audit_type);
      } else { // For UPLL, UPPL
        ret = pTcLibInterface_->HandleAuditStart(
                                    audit_trans_msg.session_id,
                                    audit_trans_msg.driver_id,
                                    audit_trans_msg.controller_id,
                                    audit_trans_msg.audit_type,
                                    audit_trans_msg.commit_number,
                                    audit_trans_msg.commit_date,
                                    audit_trans_msg.commit_application);
      }
      pfc_log_info("%s %d HandleAuditStart returned with %d",
                   __FUNCTION__, __LINE__, ret);
      if (ret != TC_SUCCESS && ret != TC_SIMPLIFIED_AUDIT &&
          ret != TC_CANCELLED_AUDIT) {
        pfc_log_error("%s %d oper_type %d Handler returned with %d",
                      __FUNCTION__, __LINE__, oper_type, ret);
        /* reset of audit in progess flag */
        audit_in_progress_ = PFC_FALSE;
        ReleaseTransactionResources();
      }
      break;
    }
    case MSG_AUDIT_END :
      if (IsHandleCancelAuditInTclib() == PFC_TRUE) {
        pfc_log_info("%s %d Handle in TCLIB. Not calling HandleAuditEnd",
                     __FUNCTION__, __LINE__);
        ret = TC_SUCCESS;
      } else {
        ret = pTcLibInterface_->HandleAuditEnd(audit_trans_msg.session_id,
                                              audit_trans_msg.driver_id,
                                              audit_trans_msg.controller_id,
                                              audit_trans_msg.audit_result);
        pfc_log_info("%s %d HandleAuditEnd returned with %d",
                   __FUNCTION__, __LINE__, ret);
      }
      /* reset of audit in progess flag */
      audit_in_progress_ = PFC_FALSE;
      ReleaseTransactionResources();
      break;
    case MSG_AUDIT_TRANS_START :
      /*
       * On TC_SUCCESS, no key type information will be written in a response.
       * On TC_FAILURE, key type information will be filled from the
       * HandleAuditTransactionStart using the provided TcLibWrite APIs.
       */
      ret = pTcLibInterface_->HandleAuditTransactionStart(
                                        audit_trans_msg.session_id,
                                        audit_trans_msg.driver_id,
                                        audit_trans_msg.controller_id);
      pfc_log_info("%s %d HandleTransactionStart returned with %d",
                   __FUNCTION__, __LINE__, ret);
      break;
    case MSG_AUDIT_TRANS_END :
      ret = pTcLibInterface_->HandleAuditTransactionEnd(
                                        audit_trans_msg.session_id,
                                        audit_trans_msg.driver_id,
                                        audit_trans_msg.controller_id,
                                        audit_trans_msg.end_result);
      pfc_log_info("%s %d HandleTransactionEnd returned with %d",
                   __FUNCTION__, __LINE__, ret);
      if ((GetControllerType() != UNC_CT_UNKNOWN) &&
          (audit_in_progress_ == PFC_FALSE)) {
        /* Audit end will not be recieved for drivers not intended for audit
           so release the transaction resources
         */
        ReleaseTransactionResources();
      }
      break;
    default :
      ret = TC_FAILURE;
      pfc_log_error("%s %d Invalid oper type for this interface %d",
                   __FUNCTION__, __LINE__, oper_type);
  }
  return ret;
}

/**
 * @brief      audit transaction vote/global operations towards UPLL/UPPL
 * @param[in]  oper_type operation type in audit trans vote/global process
 * @param[in]  audit_trans_msg structure variable after reading from session
 * @retval     TC_SUCCESS on handle operation success
 * @retval     TC_FAILURE on handle operation failure
 */
TcCommonRet TcLibModule::AuditVoteGlobal(TcMsgOperType oper_type,
                                         TcAuditTransactionMsg
                                         audit_trans_msg) {
  TcCommonRet ret = TC_SUCCESS;
  tc::TcUtilRet util_ret = tc::TCUTIL_RET_SUCCESS;
  TcDriverInfoMap driver_info;
  TcAuditResult audit_result = TC_AUDIT_SUCCESS;

  if (pTcLibInterface_ == NULL) {
    pfc_log_error("%s %d TcLibInterface not registered",
                  __FUNCTION__, __LINE__);
    return TC_FAILURE;
  }

  if (oper_type == MSG_AUDIT_VOTE) {
    ret = pTcLibInterface_->HandleAuditVoteRequest(
                                       audit_trans_msg.session_id,
                                       audit_trans_msg.driver_id,
                                       audit_trans_msg.controller_id,
                                       driver_info);
  } else if (oper_type == MSG_AUDIT_GLOBAL) {
    ret = pTcLibInterface_->HandleAuditGlobalCommit(
                                       audit_trans_msg.session_id,
                                       audit_trans_msg.driver_id,
                                       audit_trans_msg.controller_id,
                                       driver_info,
                                       audit_result);
  } else {
    pfc_log_error("%s %d Invalid oper type for this interface",
                  __FUNCTION__, __LINE__);
    return TC_FAILURE;
  }

  if (ret != TC_SUCCESS) {
    pfc_log_error("%s %d HandleAuditvote/HandleAuditGlobal failed",
                  __FUNCTION__, __LINE__);
    return ret;
  }

  pfc_log_info("%s %d oper_type %d Handler returned with %d",
               __FUNCTION__, __LINE__, oper_type, ret);

  // write the driver info map into session as response
  if (driver_info.empty()) {
    pfc_log_info("%s %d Driver info is empty", __FUNCTION__, __LINE__);
    if (oper_type == MSG_AUDIT_GLOBAL) {
      /* adding audit_result to the session which is filled by
       * HandleAuditGlobalCommit function
       */
      util_ret = tc::TcServerSessionUtils::set_uint8(sess_, audit_result);
      if (util_ret != tc::TCUTIL_RET_SUCCESS) {
        pfc_log_error("%s %d TcServerSessionUtils failed with %d",
                      __FUNCTION__, __LINE__, util_ret);
        return TC_FAILURE;
      }
    }
    return ret;
  }

  std::map<unc_keytype_ctrtype_t, std::vector<std::string> >::iterator m_it;
  std::vector<std::string>::iterator v_it;
  std::vector<std::string> ctrl_vector;
  unc_keytype_ctrtype_t ctr_type;
  uint8_t ctrl_count = 0;

  // setting the driver count
  util_ret = tc::TcServerSessionUtils::set_uint8(sess_,
                                                 driver_info.size());
  if (util_ret != tc::TCUTIL_RET_SUCCESS) {
    pfc_log_error("%s %d TcServerSessionUtils failed with %d",
                  __FUNCTION__, __LINE__, util_ret);
    return TC_FAILURE;
  }

  for (m_it = driver_info.begin(); m_it != driver_info.end(); m_it++) {
    ctr_type = m_it->first;
    util_ret = tc::TcServerSessionUtils::set_uint8(sess_, ctr_type);
    if (util_ret != tc::TCUTIL_RET_SUCCESS) {
      pfc_log_error("%s %d TcServerSessionUtils failed with %d",
                    __FUNCTION__, __LINE__, util_ret);
      return TC_FAILURE;
    }

    ctrl_vector = m_it->second;
    ctrl_count = ctrl_vector.size();  // controller count
    util_ret = tc::TcServerSessionUtils::set_uint8(sess_, ctrl_count);
    if (util_ret != tc::TCUTIL_RET_SUCCESS) {
      pfc_log_error("%s %d TcServerSessionUtils failed with %d",
                    __FUNCTION__, __LINE__, util_ret);
      return TC_FAILURE;
    }

    for (v_it = ctrl_vector.begin(); v_it != ctrl_vector.end(); v_it++) {
      util_ret = tc::TcServerSessionUtils::set_string(sess_, *v_it);
      if (util_ret != tc::TCUTIL_RET_SUCCESS) {
        pfc_log_error("%s %d TcServerSessionUtils failed with %d",
                      __FUNCTION__, __LINE__, util_ret);
        return TC_FAILURE;
      }
    }
  }

  if (oper_type == MSG_AUDIT_GLOBAL) {
    /* adding audit_result to the session which is filled by
     * HandleAuditGlobalCommit function
     */
    util_ret = tc::TcServerSessionUtils::set_uint8(sess_, audit_result);
    if (util_ret != tc::TCUTIL_RET_SUCCESS) {
      pfc_log_error("%s %d TcServerSessionUtils failed with %d",
                    __FUNCTION__, __LINE__, util_ret);
      return TC_FAILURE;
    }
  }

  pfc_log_info("%s %d Driver info filled driver_count %"PFC_PFMT_SIZE_T
               " ctrl_count %d", __FUNCTION__, __LINE__,
               driver_info.size(), ctrl_count);
  return ret;
}

/**
 * @brief      audit transaction
 * @retval     TC_SUCCESS on handle operation success
 * @retval     TC_FAILURE on handle operation failure
 */
TcCommonRet TcLibModule::AuditTransaction() {
  pfc::core::ScopedMutex m(tclib_ipc_control_mutex_);

  TcCommonRet ret = TC_SUCCESS;
  TcAuditTransactionMsg audit_trans_msg;

  ret = TcLibMsgUtil::GetAuditTransactionMsg(sess_, audit_trans_msg);
  if (ret != TC_SUCCESS) {
    pfc_log_error("%s %d Get Msg Util error %d", __FUNCTION__, __LINE__, ret);
    return ret;
  }

  pfc_log_info("%s oper=%s, sessId[%u], drv_id[%d], controller-id[%s]",
               __FUNCTION__, TcMsgOperTypeString[audit_trans_msg.oper_type],
               audit_trans_msg.session_id, audit_trans_msg.driver_id,
               audit_trans_msg.controller_id.c_str());

  if (GetControllerType() != UNC_CT_UNKNOWN) {
    if (audit_trans_msg.driver_id == GetControllerType()) {
      audit_in_progress_ = PFC_TRUE;
      pfc_log_info("%s %d audit flag enabled %d",
                   __FUNCTION__, __LINE__, audit_in_progress_);
    } else {
      audit_in_progress_ = PFC_FALSE;
      pfc_log_info("%s %d audit flag disabled %d",
                   __FUNCTION__, __LINE__, audit_in_progress_);
    }
  }

  if (ValidateOperTypeSequence(audit_trans_msg.oper_type)!= TC_SUCCESS) {
    pfc_log_error("%s %d Operation type sequence invalid",
                  __FUNCTION__, __LINE__);
    return TC_FAILURE;
  }

  switch (audit_trans_msg.oper_type) {
    case MSG_AUDIT_START :
    case MSG_AUDIT_END :
    case MSG_AUDIT_TRANS_START:
    case MSG_AUDIT_TRANS_END:
      ret = AuditTransStartEnd(audit_trans_msg.oper_type, audit_trans_msg);
      break;
    case MSG_AUDIT_VOTE:
    case MSG_AUDIT_GLOBAL:
      ret = AuditVoteGlobal(audit_trans_msg.oper_type, audit_trans_msg);
      break;
    default:
      pfc_log_error("%s %d Invalid Operation", __FUNCTION__, __LINE__);
      ret = TC_FAILURE;
  }

  return ret;
}

/**
 * @brief      audit cancel notification to driver, uppl, upll
 * @retval     TC_SUCCESS on handle operation success
 * @retval     TC_FAILURE on handle operation failure
 */
TcCommonRet TcLibModule::AuditCancel(pfc::core::ipc::ServerSession *sess) {
  // No lock acquired here with tclib_ipc_control_mutex_
  // The CancelAudit request to be processed simultaneously
  // No member variables are accessed / updated - so thread safe without lock

  TcCommonRet ret = TC_SUCCESS;

  pfc_log_info("AUDIT_CANCEL notification");

  tc::TcUtilRet util_ret = tc::TCUTIL_RET_SUCCESS;
  uint32_t argcount = 0, session_id = 0, idx = 0;
  uint8_t opertype = 0, driverid = 0;

  TcMsgOperType oper_type;
  unc_keytype_ctrtype_t driver_id;
  std::string controller_id;

  if (sess == NULL) {
    pfc_log_error("%s Invalid session null", __FUNCTION__);
    return TC_FAILURE;
  }

  argcount = sess->getArgCount();
  pfc_log_debug("%s session arg count %d",
               __FUNCTION__, argcount);

  // argcount empty check
  if (argcount == IPC_DEFAULT_ARG_COUNT) {
    pfc_log_error("%s Argument count is %d",
                   __FUNCTION__, argcount);
    return TC_FAILURE;
  }

  // oper_type
  util_ret = tc::TcServerSessionUtils::get_uint8(sess, idx, &opertype);
  if (util_ret != tc::TCUTIL_RET_SUCCESS) {
    pfc_log_error("%s TcServerSessionUtils opertype reading failed with %d",
                  __FUNCTION__, util_ret);
    return TC_FAILURE;
  }
  oper_type = (TcMsgOperType)opertype;

  // session_id
  idx++;
  util_ret = tc::TcServerSessionUtils::get_uint32(sess, idx,
                                            &session_id);
  if (util_ret != tc::TCUTIL_RET_SUCCESS) {
    pfc_log_error("%s TcServerSessionUtils failed reading sessid with %d",
                  __FUNCTION__, util_ret);
    return TC_FAILURE;
  }

  // driver_id
  idx++;
  util_ret = tc::TcServerSessionUtils::get_uint8(sess, idx, &driverid);
  if (util_ret != tc::TCUTIL_RET_SUCCESS) {
    pfc_log_error("%s TcServerSessionUtils failed reading drv-id with %d",
                  __FUNCTION__, util_ret);
    return TC_FAILURE;
  }
  driver_id = (unc_keytype_ctrtype_t) driverid;

  // controller_id
  idx++;
  util_ret = tc::TcServerSessionUtils::get_string(sess, idx,
                                            controller_id);
  if (util_ret != tc::TCUTIL_RET_SUCCESS) {
    pfc_log_error("%s TcServerSessionUtils failed reading ctr_id with %d",
                  __FUNCTION__, util_ret);
    return TC_FAILURE;
  }

  if (oper_type != MSG_AUDIT_CANCEL) {
    pfc_log_error("%s Received opertype[%d] but expected MSG_AUDIT_CANCEL",
                  __FUNCTION__, oper_type);
    return TC_FAILURE;
  }

  if (ValidateOperTypeSequence(oper_type)!= TC_SUCCESS) {
    pfc_log_error("%s Operation type sequence invalid",
                  __FUNCTION__);
    return TC_FAILURE;
  }

  if (IsHandleCancelAuditInTclib() == PFC_FALSE) {
    ret =  pTcLibInterface_->HandleAuditCancel(session_id,
                                             driver_id,
                                             controller_id);
  } else {
    pfc_log_info("%s AUDIT_CANCEL received before AUDIT_START."
                 "Not calling HandleAuditCancel()", __FUNCTION__);
    return TC_SUCCESS;
  }

  if (ret != TC_SUCCESS) {
    pfc_log_error("%s HandleAuditCancel return error %d",
                  __FUNCTION__, ret);
  }

  return ret;
}

/**
 * @brief      audit vote/global transaction for driver modules
 * @retval     TC_SUCCESS on handle operation success
 * @retval     TC_FAILURE on handle operation failure
 */
TcCommonRet TcLibModule::AuditDriverVoteGlobal() {
  pfc::core::ScopedMutex m(tclib_ipc_control_mutex_);

  TcCommonRet ret = TC_SUCCESS;
  TcAuditDrvVoteGlobalMsg drv_vote_global_msg;

  if (pTcLibInterface_ == NULL) {
    pfc_log_error("%s %d TcLibInterface not registered",
                  __FUNCTION__, __LINE__);
    return TC_FAILURE;
  }

  ret = TcLibMsgUtil::GetAuditDrvVoteGlobalMsg(sess_, drv_vote_global_msg);
  if (ret != TC_SUCCESS) {
    pfc_log_error("%s %d Get Msg Util error %d", __FUNCTION__, __LINE__, ret);
    return ret;
  }

  if (ValidateOperTypeSequence(drv_vote_global_msg.oper_type) != TC_SUCCESS) {
    pfc_log_error("%s %d Operation type sequence invalid",
                  __FUNCTION__, __LINE__);
    return TC_FAILURE;
  }
  if (drv_vote_global_msg.oper_type == MSG_AUDIT_DRIVER_VOTE) {
    ret = pTcLibInterface_->HandleAuditVoteRequest(
                                       drv_vote_global_msg.session_id,
                                       drv_vote_global_msg.controller_id,
                                       drv_vote_global_msg.cntrl_list);
  } else if (drv_vote_global_msg.oper_type == MSG_AUDIT_DRIVER_GLOBAL) {
    ret = pTcLibInterface_->HandleAuditGlobalCommit(
                                       drv_vote_global_msg.session_id,
                                       drv_vote_global_msg.controller_id,
                                       drv_vote_global_msg.cntrl_list);
  }

  /* Response for the above service requests are handled inside
   * the respective functions. TcLibWrite APIs are used to write
   * the response messages to the session
   */

  pfc_log_info("%s %d oper_type %d Handler returned with %d",
               __FUNCTION__, __LINE__, drv_vote_global_msg.oper_type, ret);
  return ret;
}

/**
 * @brief      audit driver result for UPLL/UPPL modules
 * @retval     TC_SUCCESS on handle operation success
 * @retval     TC_FAILURE on handle operation failure
 */
TcCommonRet TcLibModule::AuditDriverResult() {
  pfc::core::ScopedMutex m(tclib_ipc_control_mutex_);

  TcCommonRet ret = TC_SUCCESS;
  TcAuditDrvResultMsg drvresult_msg;

  if (pTcLibInterface_ == NULL) {
    pfc_log_error("%s %d TcLibInterface not registered",
                  __FUNCTION__, __LINE__);
    return TC_FAILURE;
  }

  ret = TcLibMsgUtil::GetAuditDrvResultMsg(sess_, drvresult_msg);
  if (ret != TC_SUCCESS) {
    pfc_log_error("%s %d Get Msg Util error %d", __FUNCTION__, __LINE__, ret);
    return ret;
  }

  if (ValidateOperTypeSequence(drvresult_msg.oper_type) != TC_SUCCESS) {
    pfc_log_error("%s %d Operation type sequence invalid",
                  __FUNCTION__, __LINE__);
    return TC_FAILURE;
  }

  ret = UpdateControllerKeyList();
  if (ret != TC_SUCCESS) {
    pfc_log_error("%s %d UpdateControllerKeyList failed %d",
                  __FUNCTION__, __LINE__, ret);
    return ret;
  }

  TcAuditResult audit_result = TC_AUDIT_SUCCESS;

  ret =  pTcLibInterface_->HandleAuditDriverResult(
                                      drvresult_msg.session_id,
                                      drvresult_msg.controller_id,
                                      drvresult_msg.phase,
                                      commit_phase_result_,
                                      audit_result);
  if (ret != TC_SUCCESS) {
    pfc_log_error("%s %d HandleAuditDriverResult failed %d",
                  __FUNCTION__, __LINE__, ret);
    return ret;
  }
  /* Response for the above service request are handled inside
   * the handle function itself. TcLibWrite APIs are used to
   * write the response messages to the session
   */

  if (drvresult_msg.phase == TC_AUDIT_GLOBAL_COMMIT_PHASE) {
    /* adding audit_result to the session which is filled by
     * HandleAuditDriverResult function (only for global commit phase
     * audit_result will be filled)
     */
    tc::TcUtilRet util_ret = tc::TCUTIL_RET_SUCCESS;

    util_ret = tc::TcServerSessionUtils::set_uint8(sess_, audit_result);
    if (util_ret != tc::TCUTIL_RET_SUCCESS) {
      pfc_log_error("%s %d TcServerSessionUtils failed with %d",
                    __FUNCTION__, __LINE__, util_ret);
      return TC_FAILURE;
    }
  }

  pfc_log_info("%s %d oper_type %d Handler returned with %d",
               __FUNCTION__, __LINE__, drvresult_msg.oper_type, ret);
  return ret;
}

/**
 * @brief      audit global abort
 * @retval     TC_SUCCESS on handle operation success
 * @retval     TC_FAILURE on handle operation failure
 */
TcCommonRet TcLibModule::AuditGlobalAbort() {
  pfc::core::ScopedMutex m(tclib_ipc_control_mutex_);

  TcCommonRet ret = TC_SUCCESS;
  TcAuditGlobalAbortMsg abort_msg;

  if (pTcLibInterface_ == NULL) {
    pfc_log_error("%s %d TcLibInterface not registered",
                  __FUNCTION__, __LINE__);
    return TC_FAILURE;
  }

  ret = TcLibMsgUtil::GetAuditGlobalAbortMsg(sess_, abort_msg);
  if (ret != TC_SUCCESS) {
    pfc_log_error("%s %d Get Msg Util error %d", __FUNCTION__, __LINE__, ret);
    return ret;
  }

  if (ValidateOperTypeSequence(abort_msg.oper_type) != TC_SUCCESS) {
    pfc_log_error("%s %d Operation type sequence invalid",
                  __FUNCTION__, __LINE__);
    return TC_FAILURE;
  }

  ret = pTcLibInterface_->HandleAuditGlobalAbort(
                                     abort_msg.session_id,
                                     abort_msg.driver_id,
                                     abort_msg.controller_id,
                                     abort_msg.audit_oper_phase);
  if (ret != TC_SUCCESS) {
    pfc_log_error("%s %d HandleAuditGlobalAbort failed %d",
                  __FUNCTION__, __LINE__, ret);
    return ret;
  }
  pfc_log_info("%s %d oper_type %d Handler returned with %d",
               __FUNCTION__, __LINE__, abort_msg.oper_type, ret);
  return ret;
}

/**
 * @brief      Save configuaration towards UPPL
 * @retval     TC_SUCCESS on handle operation success
 * @retval     TC_FAILURE on handle operation failure
 */
TcCommonRet TcLibModule::SaveConfiguration(
                                pfc::core::ipc::ServerSession *sess) {
  pfc::core::ScopedMutex m(tclib_ipc_control_mutex_);

  TcCommonRet ret = TC_SUCCESS;
  uint32_t argcount = 0, idx = 0, sess_id = 0;
  tc::TcUtilRet util_ret = tc::TCUTIL_RET_SUCCESS;

  if (pTcLibInterface_ == NULL) {
    pfc_log_error("%s %d TcLibInterface not registered",
                  __FUNCTION__, __LINE__);
    return TC_FAILURE;
  }

  if (sess != NULL) {
    argcount = sess->getArgCount();
    pfc_log_info("%s %d session arg count %d",
                 __FUNCTION__, __LINE__, argcount);
  } else {
    pfc_log_error("%s %d session pointer is NULL", __FUNCTION__, __LINE__);
    return TC_FAILURE;
  }
  if (argcount == IPC_DEFAULT_ARG_COUNT) {
    pfc_log_error("%s %d Argument count is NULL", __FUNCTION__, __LINE__);
    return TC_FAILURE;
  }

  util_ret = tc::TcServerSessionUtils::get_uint32(sess, idx, &sess_id);
  if (util_ret != tc::TCUTIL_RET_SUCCESS) {
    pfc_log_error("%s %d TcServerSessionUtils failed with %d",
                  __FUNCTION__, __LINE__, util_ret);
    return TC_FAILURE;
  }
 
  idx++;
  uint64_t save_version = 0;
  util_ret = tc::TcServerSessionUtils::get_uint64(sess, idx, &save_version);
  if (util_ret != tc::TCUTIL_RET_SUCCESS) {
    pfc_log_error("%s %d TcServerSessionUtils failed with %d",
                  __FUNCTION__, __LINE__, util_ret);
    return TC_FAILURE;
  }

  ret = pTcLibInterface_->HandleSaveConfiguration(sess_id, save_version); 
  if (ret != TC_SUCCESS) {
    pfc_log_error("%s %d HandleSaveConfiguration failed %d",
                  __FUNCTION__, __LINE__, ret);
    return ret;
  }

  pfc_log_info("%s %d Handler returned with %d",
               __FUNCTION__, __LINE__, ret);
  return ret;
}

/**
 * @brief      Clear startup configuaration towards UPPL
 * @retval     TC_SUCCESS on handle operation success
 * @retval     TC_FAILURE on handle operation failure
 */
TcCommonRet TcLibModule::ClearStartup(pfc::core::ipc::ServerSession *sess) {
  pfc::core::ScopedMutex m(tclib_ipc_control_mutex_);

  TcCommonRet ret = TC_SUCCESS;
  uint32_t argcount = 0, idx = 0, sess_id = 0;
  tc::TcUtilRet util_ret = tc::TCUTIL_RET_SUCCESS;

  if (pTcLibInterface_ == NULL) {
    pfc_log_error("%s %d TcLibInterface not registered",
                  __FUNCTION__, __LINE__);
    return TC_FAILURE;
  }

  if (sess != NULL) {
    argcount = sess->getArgCount();
    pfc_log_info("%s %d session arg count %d",
                 __FUNCTION__, __LINE__, argcount);
  } else {
    pfc_log_error("%s %d session pointer is NULL", __FUNCTION__, __LINE__);
    return TC_FAILURE;
  }
  if (argcount == IPC_DEFAULT_ARG_COUNT) {
    pfc_log_error("%s %d Argument count is NULL", __FUNCTION__, __LINE__);
    return TC_FAILURE;
  }

  util_ret = tc::TcServerSessionUtils::get_uint32(sess, idx, &sess_id);
  if (util_ret != tc::TCUTIL_RET_SUCCESS) {
    pfc_log_error("%s %d TcServerSessionUtils failed with %d",
                  __FUNCTION__, __LINE__, util_ret);
    return TC_FAILURE;
  }

  ret = pTcLibInterface_->HandleClearStartup(sess_id);
  if (ret != TC_SUCCESS) {
    pfc_log_error("%s %d HandleClearStartup failed %d",
                  __FUNCTION__, __LINE__, ret);
    return ret;
  }

  pfc_log_info("%s %d Handler returned with %d",
               __FUNCTION__, __LINE__, ret);
  return ret;
}

/**
 * @brief      Abort configuaration towards UPPL
 * @retval     TC_SUCCESS on handle operation success
 * @retval     TC_FAILURE on handle operation failure
 */
TcCommonRet TcLibModule::AbortCandidate(pfc::core::ipc::ServerSession *sess) {
  pfc::core::ScopedMutex m(tclib_ipc_control_mutex_);

  TcCommonRet ret = TC_SUCCESS;
  uint32_t argcount = 0, idx = 0, sess_id = 0, conf_id = 0;
  tc::TcUtilRet util_ret = tc::TCUTIL_RET_SUCCESS;
  TcConfigMode config_mode = TC_CONFIG_INVALID;
  uint32_t config_id =0;
  std::string vtn_name;

  if (pTcLibInterface_ == NULL) {
    pfc_log_error("%s %d TcLibInterface not registered",
                  __FUNCTION__, __LINE__);
    return TC_FAILURE;
  }

  if (sess != NULL) {
    argcount = sess->getArgCount();
    pfc_log_debug("%s %d session arg count %d",
                 __FUNCTION__, __LINE__, argcount);
  } else {
    pfc_log_error("%s %d session pointer is NULL", __FUNCTION__, __LINE__);
    return TC_FAILURE;
  }
  if (argcount == IPC_DEFAULT_ARG_COUNT) {
    pfc_log_error("%s %d Argument count is NULL", __FUNCTION__, __LINE__);
    return TC_FAILURE;
  }

  util_ret = tc::TcServerSessionUtils::get_uint32(sess, idx, &sess_id);
  if (util_ret != tc::TCUTIL_RET_SUCCESS) {
    pfc_log_error("%s %d TcServerSessionUtils failed with %d",
                  __FUNCTION__, __LINE__, util_ret);
    return TC_FAILURE;
  }

  idx++;
  util_ret = tc::TcServerSessionUtils::get_uint32(sess, idx, &conf_id);
  if (util_ret != tc::TCUTIL_RET_SUCCESS) {
    pfc_log_error("%s %d TcServerSessionUtils failed with %d",
                  __FUNCTION__, __LINE__, util_ret);
    return TC_FAILURE;
  }

  idx++;
  uint64_t abort_version = 0;
  util_ret = tc::TcServerSessionUtils::get_uint64(sess, idx, &abort_version);
  if (util_ret != tc::TCUTIL_RET_SUCCESS) {
    pfc_log_error("%s %d TcServerSessionUtils failed with %d",
                  __FUNCTION__, __LINE__, util_ret);
    return TC_FAILURE;
  }

  ret = GetConfigData(sess_id, config_id, config_mode, vtn_name);

  ret = pTcLibInterface_->HandleAbortCandidate(sess_id, conf_id, 
                                               config_mode, vtn_name,
                                               abort_version);
  if (ret != TC_SUCCESS) {
    pfc_log_error("%s %d HandleAbortCandidate failed %d",
                  __FUNCTION__, __LINE__, ret);
    return ret;
  }

  pfc_log_info("%s %d Handler returned with %d",
               __FUNCTION__, __LINE__, ret);
  return ret;
}

/**
 * @brief      Audit config operation on fail over scenorios
 * @retval     TC_SUCCESS on handle operation success
 * @retval     TC_FAILURE on handle operation failure
 */
TcCommonRet TcLibModule::AuditConfig(pfc::core::ipc::ServerSession *sess) {
  pfc::core::ScopedMutex m(tclib_ipc_control_mutex_);

  TcCommonRet ret = TC_SUCCESS;
  TcAuditConfigMsg audit_config_msg;

  if (pTcLibInterface_ == NULL) {
    pfc_log_error("%s %d TcLibInterface not registered",
                  __FUNCTION__, __LINE__);
    return TC_FAILURE;
  }

  ret = TcLibMsgUtil::GetAuditConfigMsg(sess, audit_config_msg);
  if (ret != TC_SUCCESS) {
    pfc_log_error("%s %d Get Msg Util error %d", __FUNCTION__, __LINE__, ret);
    return ret;
  }
 
  ret = pTcLibInterface_->HandleAuditConfig(audit_config_msg.db_type,
                                            audit_config_msg.service_type,
                                            audit_config_msg.config_mode,
                                            audit_config_msg.vtn_name,
                                            audit_config_msg.version); 
  if (ret != TC_SUCCESS) {
    pfc_log_error("%s %d HandleAuditConfig failed", __FUNCTION__, __LINE__);
    return ret;
  }

  pfc_log_info("%s %d Handler returned with %d",
               __FUNCTION__, __LINE__, ret);
  return ret;
}

/**
 * @brief      Setup Configuration Message sent to UPPL at the end of startup
 *             operation to send messages to driver
 * @retval     TC_SUCCESS clear startup handling success
 * @retval     TC_FAILURE clear startup handling failed
 */
TcCommonRet TcLibModule::Setup(pfc::core::ipc::ServerSession *sess) {
  pfc::core::ScopedMutex m(tclib_ipc_control_mutex_);
  TcCommonRet ret = TC_SUCCESS;

  if (pTcLibInterface_ == NULL) {
    pfc_log_error("%s %d TcLibInterface not registered",
                  __FUNCTION__, __LINE__);
    return TC_FAILURE;
  }
  // Read from session and get the startup config validity
  if (sess == NULL) {
    pfc_log_error("%s %d session pointer is NULL", __FUNCTION__, __LINE__);
    return TC_FAILURE;
  }

  uint32_t argcount = sess->getArgCount();
  pfc_log_info("%s %d session arg count %d",
               __FUNCTION__, __LINE__, argcount);
  if (argcount == IPC_DEFAULT_ARG_COUNT) {
    pfc_log_error("%s %d Argument count is NULL", __FUNCTION__, __LINE__);
    return TC_FAILURE;
  }

  tc::TcUtilRet util_ret = tc::TCUTIL_RET_SUCCESS;
  uint32_t arg_index = 0;
  pfc::core::ScopedMutex as_lock(tclib_autosave_mutex_);
  util_ret = tc::TcServerSessionUtils::get_uint8(sess,
                                                 arg_index,
                                                 &auto_save_enabled_);
  if (util_ret != tc::TCUTIL_RET_SUCCESS) {
    pfc_log_error("%s %d Tc read sess failed with %d",
                  __FUNCTION__, __LINE__, util_ret);
    return TC_FAILURE;
  }

  pfc_log_info("%s Received Autosave information; enabled? %d",
               __FUNCTION__, auto_save_enabled_);

  as_lock.unlock(); 

  ret = pTcLibInterface_->HandleSetup();
  if (ret != TC_SUCCESS) {
    pfc_log_error("%s %d HandleSetup failed %d",
                  __FUNCTION__, __LINE__, ret);
    return ret;
  }

  pfc_log_info("%s %d Handler returned with %d",
               __FUNCTION__, __LINE__, ret);
  return ret;
}

/**
 * @brief      Setup Complete Message sent to UPPL during state changes
 * @retval     TC_SUCCESS clear startup handling success
 * @retval     TC_FAILURE clear startup handling failed
 */
TcCommonRet TcLibModule::SetupComplete(pfc::core::ipc::ServerSession *sess) {
  pfc::core::ScopedMutex m(tclib_ipc_control_mutex_);
  TcCommonRet ret = TC_SUCCESS;

  if (pTcLibInterface_ == NULL) {
    pfc_log_error("%s %d TcLibInterface not registered",
                  __FUNCTION__, __LINE__);
    return TC_FAILURE;
  }

  // Read from session and get the startup config validity
  if (sess == NULL) {
    pfc_log_error("%s %d session pointer is NULL", __FUNCTION__, __LINE__);
    return TC_FAILURE;
  }

  uint32_t argcount = sess->getArgCount();
  pfc_log_info("%s %d session arg count %d",
               __FUNCTION__, __LINE__, argcount);
  if (argcount == IPC_DEFAULT_ARG_COUNT) {
    pfc_log_error("%s %d Argument count is NULL", __FUNCTION__, __LINE__);
    return TC_FAILURE;
  }

  tc::TcUtilRet util_ret = tc::TCUTIL_RET_SUCCESS;
  uint32_t arg_index = 0;
  pfc::core::ScopedMutex as_lock(tclib_autosave_mutex_);
  util_ret = tc::TcServerSessionUtils::get_uint8(sess,
                                                 arg_index,
                                                 &auto_save_enabled_);
  if (util_ret != tc::TCUTIL_RET_SUCCESS) {
    pfc_log_error("%s %d Tc read sess failed with %d",
                  __FUNCTION__, __LINE__, util_ret);
    return TC_FAILURE;
  }

  pfc_log_info("%s Received Autosave information; enabled? %d",
               __FUNCTION__, auto_save_enabled_);

  as_lock.unlock(); 

  ret = pTcLibInterface_->HandleSetupComplete();
  if (ret != TC_SUCCESS) {
    pfc_log_error("%s %d HandleSetupComplete failed %d",
                  __FUNCTION__, __LINE__, ret);
    return ret;
  }

  pfc_log_info("%s %d Handler returned with %d",
               __FUNCTION__, __LINE__, ret);
  return ret;
}

/**
 * @brief      Get controller type invoked from TC to detect the controller type
 *             for a controller
 * @retval     openflow/overlay/legacy if controller id matches
 * @retval     UNC_CT_UNKNOWN if controller id does not belong to
 *             any of controller type
 */
unc_keytype_ctrtype_t TcLibModule::GetDriverId() {

  pfc::core::ScopedMutex m(tclib_ipc_control_mutex_);

  unc_keytype_ctrtype_t ctr_type = UNC_CT_UNKNOWN;
  uint32_t argcount = 0;

  if (pTcLibInterface_ == NULL) {
    pfc_log_error("%s %d TcLibInterface not registered",
                  __FUNCTION__, __LINE__);
    return UNC_CT_UNKNOWN;
  }

  if (sess_ != NULL) {
    argcount = sess_->getArgCount();
    pfc_log_info("%s %d session arg count %d",
                 __FUNCTION__, __LINE__, argcount);
  } else {
    pfc_log_error("%s %d session pointer is NULL", __FUNCTION__, __LINE__);
    return UNC_CT_UNKNOWN;
  }
  if (argcount == IPC_DEFAULT_ARG_COUNT) {
    pfc_log_error("%s %d Argument count is NULL", __FUNCTION__, __LINE__);
    return UNC_CT_UNKNOWN;
  }

  uint32_t idx = 0;
  std::string controller_id;
  tc::TcUtilRet util_ret = tc::TCUTIL_RET_SUCCESS;

  util_ret = tc::TcServerSessionUtils::get_string(sess_, idx, controller_id);
  if (util_ret != tc::TCUTIL_RET_SUCCESS) {
    pfc_log_error("%s %d TcServerSessionUtils failed with %d",
                  __FUNCTION__, __LINE__, util_ret);
    return UNC_CT_UNKNOWN;
  }
  oper_state_ = MSG_GET_DRIVERID;
  ctr_type = pTcLibInterface_->HandleGetControllerType(controller_id);

  pfc_log_info("%s %d Handler returned with %d",
               __FUNCTION__, __LINE__, ctr_type);

  oper_state_ = MSG_NONE;
  return ctr_type;
}

/**
 * @brief      Get Controller Type
 *             Invoked from TC to detect the type of the controller
 *             Intended for the driver modules
 * @retval     openflow/overlay/legacy if controller id matches
 * @retval     none if requested for other than driver modules
 *             UPPL/UPLL modules should return UNC_CT_UNKNOWN
 */
unc_keytype_ctrtype_t TcLibModule::GetControllerType() {
  unc_keytype_ctrtype_t ctr_type = UNC_CT_UNKNOWN;

  if (pTcLibInterface_ == NULL) {
    pfc_log_error("%s %d TcLibInterface not registered",
                  __FUNCTION__, __LINE__);
    return UNC_CT_UNKNOWN;
  }

  ctr_type = pTcLibInterface_->HandleGetControllerType();

  pfc_log_debug("%s %d Handler returned with %d",
               __FUNCTION__, __LINE__, ctr_type);
  return ctr_type;
}

/**
 * @brief      To send response to TC through IPC
 * @param[in]  sess  - Session Id.
 * @param[in]  service - Represents the request sent from TC.
 * @retval     returns the response from the respective handling functions
 */
pfc_ipcresp_t TcLibModule::ipcService(pfc::core::ipc::ServerSession& sess,
                                      pfc_ipcid_t service) {
  pfc_ipcresp_t resp_ret = TC_SUCCESS;

  if (pTcLibInterface_ == NULL) {
    pfc_log_error("%s %d Handler is not registered: NULL",
                  __FUNCTION__, __LINE__);
    return PFC_IPCRESP_FATAL;
  }
  /*set IPC timeout to infinity for commit/audit operations*/
  if ((service >= TCLIB_COMMIT_TRANSACTION &&
       service <= TCLIB_SETUP_COMPLETE) ||
      (service == TCLIB_AUDIT_CONFIG ||
       service == TCLIB_AUTOSAVE_ENABLE ||
       service == TCLIB_AUTOSAVE_DISABLE)) {
    if (TC_SUCCESS != sess.setTimeout(NULL)) {
      pfc_log_warn("setting IPC timeout to infinity failed");
    }
  }

  request_counter_lock_.lock();
  request_counter_++;
  request_counter_lock_.unlock();
  

  /*configure mode can be released while internal driver audit is in progress
   * to avoid session corruption, current session is passed directly to
   * NotifySessionConfig API*/
  if ((service >= TCLIB_COMMIT_TRANSACTION &&
      service <= TCLIB_AUDIT_GLOBAL_ABORT) ||
      service == TCLIB_GET_DRIVERID ||
      service == TCLIB_CONTROLLER_TYPE) {
    /*assign current ipc session to class member*/
    sess_ = &sess;
  }

  switch (service) {
    case TCLIB_NOTIFY_SESSION_CONFIG:
      return (pfc_ipcresp_t)NotifySessionConfig(&sess);
    case TCLIB_COMMIT_TRANSACTION:
      resp_ret = (pfc_ipcresp_t)CommitTransaction();
      break;
    case TCLIB_COMMIT_DRV_VOTE_GLOBAL :
      resp_ret = (pfc_ipcresp_t)CommitDriverVoteGlobal();
      break;
    case TCLIB_COMMIT_DRV_RESULT :
      resp_ret = (pfc_ipcresp_t)CommitDriverResult();
      break;
    case TCLIB_COMMIT_GLOBAL_ABORT :
      resp_ret = (pfc_ipcresp_t)CommitGlobalAbort();
      break;
    case TCLIB_AUDIT_TRANSACTION:
      resp_ret = (pfc_ipcresp_t)AuditTransaction();
      break;
    case TCLIB_AUDIT_DRV_VOTE_GLOBAL :
      resp_ret = (pfc_ipcresp_t)AuditDriverVoteGlobal();
      break;
    case TCLIB_AUDIT_DRV_RESULT :
      resp_ret = (pfc_ipcresp_t)AuditDriverResult();
      break;
    case TCLIB_AUDIT_GLOBAL_ABORT :
      resp_ret = (pfc_ipcresp_t)AuditGlobalAbort();
      break;
    case TCLIB_AUDIT_CANCEL:
      resp_ret = (pfc_ipcresp_t)AuditCancel(&sess);
      break;
    case TCLIB_SAVE_CONFIG :
      resp_ret = (pfc_ipcresp_t)SaveConfiguration(&sess);
      break;
    case TCLIB_CLEAR_STARTUP :
      resp_ret = (pfc_ipcresp_t)ClearStartup(&sess);
      break;
    case TCLIB_USER_ABORT:
      resp_ret = (pfc_ipcresp_t)AbortCandidate(&sess);
      break;
    case TCLIB_SETUP :
      resp_ret = (pfc_ipcresp_t)Setup(&sess);
      break;
    case TCLIB_SETUP_COMPLETE :
      resp_ret = (pfc_ipcresp_t)SetupComplete(&sess);
      break;
    case TCLIB_GET_DRIVERID:
      resp_ret = (pfc_ipcresp_t)GetDriverId();
      break;
    case TCLIB_AUDIT_CONFIG :
      resp_ret = (pfc_ipcresp_t)AuditConfig(&sess);
      break;
    case TCLIB_CONTROLLER_TYPE:
      resp_ret = (pfc_ipcresp_t)GetControllerType();
      break;
    case TCLIB_AUTOSAVE_ENABLE:
    case TCLIB_AUTOSAVE_DISABLE:
      resp_ret = (pfc_ipcresp_t) NotifyAutoSave(service, &sess);
      break;
    default:
      pfc_log_error("%s %d Invalid service", __FUNCTION__, __LINE__);
      resp_ret = PFC_IPCRESP_FATAL; /*invalid*/
  }

  request_counter_lock_.lock();
  request_counter_--;
  if (request_counter_ == 0) {
    sess_ = NULL;
  }
  request_counter_lock_.unlock();

  return resp_ret;
}

TcCommonRet TcLibModule::NotifyAutoSave(pfc_ipcid_t service,
                                        pfc::core::ipc::ServerSession *sess) {
  pfc::core::ScopedMutex m(tclib_autosave_mutex_);
  TcCommonRet ret = TC_SUCCESS;
  switch(service) {
    case TCLIB_AUTOSAVE_ENABLE:
      pfc_log_info("NotifyAutoSave:: AUTOSAVE_ENABLE");
      auto_save_enabled_ = PFC_TRUE;
      ret = ClearStartup(sess);
      if (ret != TC_SUCCESS) {
        pfc_log_error("%s %d ClearStartup failed %d",
                      __FUNCTION__, __LINE__, ret);
        return ret;
      }
      pfc_log_info("NotifyAutoSave:: ClearStartup success");
      break;
    case TCLIB_AUTOSAVE_DISABLE:
      pfc_log_info("NotifyAutoSave:: AUTOSAVE_DISABLE");
      auto_save_enabled_ = PFC_FALSE;
      ret = SaveConfiguration(sess);
      if (ret != TC_SUCCESS) {
        pfc_log_error("%s %d SaveConfiguration failed %d",
                      __FUNCTION__, __LINE__, ret);
        return ret;
      }
      pfc_log_info("NotifyAutoSave:: SaveConfiguration success");
      break;
    default:
      return TC_FAILURE;
  }
  return TC_SUCCESS;
}

/**
 * @brief      Provide info if startup-config is valid or not
 * @param[in]  None
 * @retval     PFC_TRUE if autosave is disabled
 * @retval     PFC_FALSE if autosave is enabled
 */
pfc_bool_t TcLibModule::IsStartupConfigValid() {
  pfc::core::ScopedMutex m(tclib_autosave_mutex_);
  return !auto_save_enabled_;
}

void TcLibModule::SetHandleCancelAuditInTclib(pfc_bool_t handle_in_tclib) {
    pfc::core::ScopedMutex m(handle_in_tclib_mutex_);
    handle_in_tclib_ = handle_in_tclib;
}

pfc_bool_t TcLibModule::IsHandleCancelAuditInTclib() {
    pfc::core::ScopedMutex m(handle_in_tclib_mutex_);
    return handle_in_tclib_;
}

} // namespace tclib
} // namespace unc
// Declare C++ module
PFC_MODULE_IPC_DECL(unc::tclib::TcLibModule, TCLIB_IPC_SERVICES);
