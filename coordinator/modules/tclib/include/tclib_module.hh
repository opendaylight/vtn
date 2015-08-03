/*
 * Copyright (c) 2013-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */


#ifndef _UNC_TCLIB_MODULE_HH_
#define _UNC_TCLIB_MODULE_HH_

#include <pfcxx/module.hh>
#include <pfcxx/synch.hh>
#include <uncxx/tclib/tclib_interface.hh>
#include <tclib_struct_defs.hh>
#include <string>

namespace unc {
namespace tclib {

class TcLibInterface;

class TcLibModule : public pfc::core::Module {
 public:
  /**
   * @brief      TcLibModule constructor
   * @param[in]  *mattr Attribute of the Module
   */
  explicit TcLibModule(const pfc_modattr_t *mattr);

  /**
   * @brief      TcLibModule destructor
   */
  ~TcLibModule();

  /**
   * @brief      Init of TcLibModule
   *             initialises the member variables
   */
  pfc_bool_t init();

  /**
   * @brief      Terminate of TcLibModule
   *             setting the member variables to default values
   */
  pfc_bool_t fini();

  /**
   * @brief      To send response to TC through IPC
   * @param[in]  sess  - Session Id.
   * @param[in]  service - Represents the request sent from TC.
   * @retval     returns the response from the respective handling functions
   */
  pfc_ipcresp_t ipcService(pfc::core::ipc::ServerSession& sess,
                           pfc_ipcid_t service);

  /* API's for other modules */

  /**
   * @brief 	To provide info on whether startup-config is valid or not.
   * @param[in]    None
   * @param[out]   None
   * @retval       Boolean true if startup-config is valid; false otherwise
   */
  pfc_bool_t IsStartupConfigValid();

  /**
   * @brief      Register TclibInterface handler
   *             TcLibInterface handlers of UPLL/UPPL/driver modules are
   *             registered which will be updated in handler.
   * @retval     TC_API_COMMON_SUCCESS on register handler success
   * @retval     TC_INVALID_PARAM if handler is NULL
   * @retval     TC_HANDLER_ALREADY_ACTIVE if already register handler has done
   */
  TcApiCommonRet TcLibRegisterHandler(TcLibInterface* handler);

  /**
   * @brief      Audit controller request invoked from driver modules
   * @param[in]  controller_id controller id intended for audit
   * @retval     TC_API_COMMON_SUCCESS Audit controller request success
   * @retval     TC_API_FATAL on any api fatal failures
   * @retval     TC_API_COMMON_FAILURE on any handling failures
   */
  TcApiCommonRet TcLibAuditControllerRequest(std::string controller_id);

  /**
   * @brief      Validate update message invoked from other modules
   * @param[in]  session_id session on which commit request sent
   * @param[in]  config_id config id generated on acquire config mode for
   *             session_id
   * @param[in]  config_mode - which holds (global/real/virtual/vtn) type
   * @param[in]  vtn_name - which contains the vtn name if config_mode is vtn.
   * @retval     TC_API_COMMON_SUCCESS on validation is success
   * @retval     TC_INVALID_SESSION_ID if session id is invalid
   * @retval     TC_INVALID_CONFIG_id if config id is invalid
   */
  TcApiCommonRet TcLibValidateUpdateMsg(uint32_t sessionid, 
                                        uint32_t configid,
                                        TcConfigMode config_mode, 
                                        std::string vtn_name);

  /**
   * @brief       Gives current session_id and config_id to upll,upll,pfcdriver
   * @param[in]   current session_id
   * @param[out]  current config_id
   * @param[out]  current config_mode
   * @param[out]  current vtn_name (if config_mode is vtn)
   * @return      TC_API_COMMON_SUCCESS on success
   * @return      TC_INVALID_SESSION_ID on invalid session id
   */
  TcApiCommonRet TcLibGetSessionAttributes(uint32_t session_id, 
                                           uint32_t& config_id,
                                           TcConfigMode& config_mode,
                                           std::string& vtn_name);

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
  TcApiCommonRet TcLibReadKeyValueDataInfo(std::string controller_id,
                                           uint32_t err_pos,
                                           uint32_t key_type,
                                           pfc_ipcstdef_t key_def,
                                           pfc_ipcstdef_t value_def,
                                           void* key_data,
                                           void* value_data);
  /**
   * @brief      Write of controller id, response code and num of errors
   * @param[in]  controller_id controller id for which key type involved
   * @param[in]  response_code response code from the UPLL/UPPL
   * @param[in]  num_of_errors number of errors if any
   * @retval     TC_API_COMMON_SUCCESS on successful updation for key and value
   * @retval     TC_INVALID_OPER_STATE invalid oper state for write
   * @retval     TC_INVALID_CONTROLLER_ID invalid controller id
   * @retval     TC_API_FATAL on any api fatal failures
   */
  TcApiCommonRet TcLibWriteControllerInfo(std::string controller_id,
                                          uint32_t response_code,
                                          uint32_t num_of_errors);

  /**
   * @brief      Write of controller id, response code,num of errors,
   *             commit_number,commit_date and commit_application
   * @param[in]  controller_id controller id for which key type involved 
   * @param[in]  response_code response code from the pfcdriver 
   * @param[in]  num_of_errors number of errors if any 
   * @param[in]  commit_number Current Commit version of PFC 
   * @param[in]  commit_date Latest committed time of PFC
   * @param[in]  commit_application Application that performed commit operation
   * @retval     TC_API_COMMON_SUCCESS on successful updation for key and value
   * @retval     TC_INVALID_OPER_STATE invalid oper state for write
   * @retval     TC_INVALID_CONTROLLER_ID invalid controller id
   * @retval     TC_API_FATAL on any api fatal failures
   * note:       This API is used only by the PFC driver to fill PFC Commit Info
   */
  TcApiCommonRet TcLibWriteControllerInfo(std::string controller_id,
                                          uint32_t response_code,
                                          uint32_t num_of_errors,
                                          uint64_t commit_number,
                                          uint64_t commit_date,
                                          std::string commit_application);
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
  TcApiCommonRet TcLibWriteKeyValueDataInfo(std::string controller_id,
                                            uint32_t key_type,
                                            pfc_ipcstdef_t key_def,
                                            pfc_ipcstdef_t value_def,
                                            void* key_data,
                                            void* value_data);

  /**
   * @brief      Get config mode based on key session_id and config_id
   * @param[in]  session_id session id of notify session config data
   * @param[in]  config_id -  notify session config id
   * @param[out] config_mode - reterive from tc_notify_config_data_map
   * @param[out] vtn_name - reterive from tc_notify_config_data_map
   *                        if config_mode is VTN
   * @retval     TC_API_COMMON_SUCCESS on successful updation
   * @retval     TC_INVALID_SESSION_ID on session_id not present in map
   * @retval     TC_INVALID_CONFIG_ID on invalid config Id
   */
  TcApiCommonRet TcLibGetConfigMode(uint32_t session_id,
                                    uint32_t config_id,
                                    TcConfigMode& config_mode,
                                    std::string& vtn_name);
 private:
  /* @brief      Handles the AutoSave msgs
   * @param[in]  pfc_ipcid_t service
   */
  TcCommonRet NotifyAutoSave(pfc_ipcid_t service,
                             pfc::core::ipc::ServerSession *sess);
  /**
   * @brief      Validation of oper type sequence
   * @param[in]  oper_type operation type in commit/audit process
   * @retval     TC_SUCCESS on valid oper state sequence
   * @retval     TC_FAILURE on invalid oper state sequence
   */
  TcCommonRet ValidateOperTypeSequence(TcMsgOperType oper_type);

  /**
   * @brief      Validation of commit sequence
   * @param[in]  oper_type operation type in commit proces
   * @retval     TC_SUCCESS on valid oper state sequence
   * @retval     TC_FAILURE on invalid oper state sequence
   */
  TcCommonRet ValidateCommitOperSequence(TcMsgOperType oper_type);

  /**
   * @brief      Validation of audit sequence for driver modules
   * @param[in]  oper_type operation type in audit process
   * @retval     TC_SUCCESS on valid oper state sequence
   * @retval     TC_FAILURE on invalid oper state sequence
   */
  TcCommonRet ValidateAuditOperSequence(TcMsgOperType oper_type);

  /**
   * @brief      Validation of commit sequence for UPPL/UPLL modules
   * @param[in]  oper_type operation type in commit process
   * @retval     TC_SUCCESS on valid oper state sequence
   * @retval     TC_FAILURE on invalid oper state sequence
   */
  TcCommonRet ValidateUpllUpplCommitSequence(TcMsgOperType oper_type);

  /**
   * @brief      Validation of commit sequence for driver modules
   * @param[in]  oper_type operation type in commit process
   * @retval     TC_SUCCESS on valid oper state sequence
   * @retval     TC_FAILURE on invalid oper state sequence
   */
  TcCommonRet ValidateDriverCommitSequence(TcMsgOperType oper_type);

  /**
   * @brief      Validation of audit sequence for UPPL/UPLL modules
   * @param[in]  oper_type operation type in audit process
   * @retval     TC_SUCCESS on valid oper state sequence
   * @retval     TC_FAILURE on invalid oper state sequence
   */
  TcCommonRet ValidateUpllUpplAuditSequence(TcMsgOperType oper_type);

  /**
   * @brief      Validation of audit sequence for driver modules
   * @param[in]  oper_type operation type in audit process
   * @retval     TC_SUCCESS on valid oper state sequence
   * @retval     TC_FAILURE on invalid oper state sequence
   */
  TcCommonRet ValidateDriverAuditSequence(TcMsgOperType oper_type);

  /**
   * @brief      Is Read key value allowed in current oper state
   * @retval     TC_API_COMMON_SUCCESS on valid oper state
   * @retval     TC_API_COMMON_FAILURE on invalid oper state
   */
  TcApiCommonRet IsReadKeyValueAllowed();

  /**
   * @brief      Is write key value allowed in current oper state
   * @retval     TC_API_COMMON_SUCCESS on valid oper state
   * @retval     TC_API_COMMON_FAILURE on invalid oper state
   */
  TcApiCommonRet IsWriteKeyValueAllowed();

  uint32_t GetMatchTypeIndex(uint32_t cur_idx, uint32_t arg_count,
                                pfc_ipctype_t type);
  /**
   * @brief      Update of controller key list information
   * @retval     TC_SUCCESS on successful updation of controller key list
   * @retval     TC_FAILURE ony failures
   */
  TcCommonRet UpdateControllerKeyList();

  /**
   * @brief      Updation of session_id and config_id
   * @retval     TC_SUCCESS on successful updation
   * @retval     TC_FAILURE on any failure
   */
  TcCommonRet NotifySessionConfig(pfc::core::ipc::ServerSession *sess);

  /*
   * @brief     Updation of notify config data based on key sessionid
   * @param[in] session_id - session id of config data
   * @param[in] config_id - config id of config data
   * @param[in] config_mode - config mode of config data
   * @param[in] vtn_name - config name of config data
   * @retval    TC_SUCCESS on successful updation
   * @retval    TC_FAILURE on any failure
   */
  TcCommonRet UpdateNotifyConfigData(uint32_t session_id,
                                     uint32_t config_id,
                                     TcConfigMode config_mode, 
                                     std::string vtn_name);
  /**
   * @brief      Get notify config data based on key sessionid
   * @param[in]  session_id session id config data
   * @param[out] config_id - reterive from tc_notify_config_data_map
   * @param[out] config_mode - reterive from tc_notify_config_data_map
   * @param[out] vtn_name - reterive from tc_notify_config_data_map
   * @retval     TC_SUCCESS on successful updation
   * @retval     TC_FAILURE on any failure
   */
   TcCommonRet GetConfigData(uint32_t session_id,
                             uint32_t& config_id,
                             TcConfigMode& config_mode,
                             std::string& vtn_name);
  /**
   * @brief      commit transaction start/end operations invoking
   * @param[in]  oper_type operation type in commit trans start/end process
   * @param[in]  commit_trans_msg structure variable after reading from session
   * @retval     TC_SUCCESS on handle operation success
   * @retval     TC_FAILURE on handle operation failure
   */
  TcCommonRet CommitTransStartEnd(TcMsgOperType oper_type,
                                  TcCommitTransactionMsg commit_trans_msg);

  /**
   * @brief      commit transaction vote/global operations towards UPLL/UPPL
   * @param[in]  oper_type operation type in commit trans vote/global process
   * @param[in]  commit_trans_msg structure variable after reading from session
   * @retval     TC_SUCCESS on handle operation success
   * @retval     TC_FAILURE on handle operation failure
   */
  TcCommonRet CommitVoteGlobal(TcMsgOperType oper_type,
                                  TcCommitTransactionMsg commit_trans_msg);

  /**
   * @brief      commit transaction
   * @retval     TC_SUCCESS on handle operation success
   * @retval     TC_FAILURE on handle operation failure
   */
  TcCommonRet CommitTransaction();

  /**
   * @brief      commit vote/global transaction for driver modules
   * @retval     TC_SUCCESS on handle operation success
   * @retval     TC_FAILURE on handle operation failure
   */
  TcCommonRet CommitDriverVoteGlobal();

  /**
   * @brief      commit driver result for UPLL/UPPL modules
   * @retval     TC_SUCCESS on handle operation success
   * @retval     TC_FAILURE on handle operation failure
   */
  TcCommonRet CommitDriverResult();

  /**
   * @brief      commit global abort
   * @retval     TC_SUCCESS on handle operation success
   * @retval     TC_FAILURE on handle operation failure
   */
  TcCommonRet CommitGlobalAbort();

  /**
   * @brief      audit start/end, transaction start/end operations invoking
   * @param[in]  oper_type operation type in audit trans start/end process
   * @param[in]  audit_trans_msg structure variable after reading from session
   * @retval     TC_SUCCESS on handle operation success
   * @retval     TC_FAILURE on handle operation failure
   */
  TcCommonRet AuditTransStartEnd(TcMsgOperType oper_type,
                                 TcAuditTransactionMsg audit_trans_msg);

  /**
   * @brief      audit transaction vote/global operations towards UPLL/UPPL
   * @param[in]  oper_type operation type in audit trans vote/global process
   * @param[in]  audit_trans_msg structure variable after reading from session
   * @retval     TC_SUCCESS on handle operation success
   * @retval     TC_FAILURE on handle operation failure
   */
  TcCommonRet AuditVoteGlobal(TcMsgOperType oper_type,
                              TcAuditTransactionMsg audit_trans_msg);

  /**
   * @brief      audit transaction
   * @retval     TC_SUCCESS on handle operation success
   * @retval     TC_FAILURE on handle operation failure
   */
  TcCommonRet AuditTransaction();

  /**
   * @brief      audit vote/global transaction for driver modules
   * @retval     TC_SUCCESS on handle operation success
   * @retval     TC_FAILURE on handle operation failure
   */
  TcCommonRet AuditDriverVoteGlobal();

  /**
   * @brief      audit driver result for UPLL/UPPL modules
   * @retval     TC_SUCCESS on handle operation success
   * @retval     TC_FAILURE on handle operation failure
   */
  TcCommonRet AuditDriverResult();

  /**
   * @brief      audit cancel notification to driver, uppl, upll
   * @retval     TC_SUCCESS on handle operation success
   * @retval     TC_FAILURE on handle operation failure
   */
  TcCommonRet AuditCancel(pfc::core::ipc::ServerSession *sess);

  /**
   * @brief      audit global abort
   * @retval     TC_SUCCESS on handle operation success
   * @retval     TC_FAILURE on handle operation failure
   */
  TcCommonRet AuditGlobalAbort();

  /**
   * @brief      Release transaction resources those involved in either
   *             commit/audit operations
   */
  void ReleaseTransactionResources();

  /**
   * @brief      Save configuaration towards UPPL
   * @retval     TC_SUCCESS on handle operation success
   * @retval     TC_FAILURE on handle operation failure
   */
  TcCommonRet SaveConfiguration(pfc::core::ipc::ServerSession *sess);

  /**
   * @brief      Clear startup configuaration towards UPPL
   * @retval     TC_SUCCESS on handle operation success
   * @retval     TC_FAILURE on handle operation failure
   */
  TcCommonRet ClearStartup(pfc::core::ipc::ServerSession *sess);

  /**
   * @brief      Abort configuaration towards UPPL
   * @retval     TC_SUCCESS on handle operation success
   * @retval     TC_FAILURE on handle operation failure
   */
  TcCommonRet AbortCandidate(pfc::core::ipc::ServerSession *sess);

  /**
   * @brief      Audit config operation on fail over scenorios
   * @retval     TC_SUCCESS on handle operation success
   * @retval     TC_FAILURE on handle operation failure
   */
  TcCommonRet AuditConfig(pfc::core::ipc::ServerSession *sess);

  /**
   * @brief      Setup Configuration Message sent to UPPL at the end of startup
   *             operation to send messages to driver
   * @retval     TC_SUCCESS clear startup handling success
   * @retval     TC_FAILURE clear startup handling failed
   */
  TcCommonRet Setup(pfc::core::ipc::ServerSession *sess);

  /**
   * @brief      Setup Complete Message sent to UPPL during state changes
   * @retval     TC_SUCCESS clear startup handling success
   * @retval     TC_FAILURE clear startup handling failed
   */
  TcCommonRet SetupComplete(pfc::core::ipc::ServerSession *sess);

  /**
   * @brief      Get controller type invoked from TC to detect the controller type
   *             for a controller
   * @retval     openflow/overlay/legacy if controller id matches
   * @retval     UNC_CT_UNKNOWN if controller id does not belong to
   *             any of controller type
   */
  unc_keytype_ctrtype_t GetDriverId();

  /**
   * @brief      Get Controller Type
   *             Invoked from TC to detect the type of the controller
   *             Intended for the driver modules
   * @retval     openflow/overlay/legacy if controller id matches
   * @retval     none if requested for other than driver modules
   *             UPPL/UPLL modules should return UNC_CT_UNKNOWN
   */
  unc_keytype_ctrtype_t GetControllerType();

  /**
   * @brief      GetKeyIndex
   *             Get of key index based on controller id, err_pos from
   *             maps filled during driver result key map updates
   * @param[in]  controller_id controller id to which key type belongs
   * @param[in]  err_pos error position to which key index is required
   * @param[out] key_index index for the error position under controller id
   * @retval     TC_API_COMMON_SUCCESS if key index successfully filled
   * @retval     TC_INVALID_PARAM/TC_INVALID_KEY_TYPE/TC_INVALID_CONTROLLER_ID
   *             if failed to fill the key index
   */
  TcApiCommonRet GetKeyIndex(std::string controller_id,
                             uint32_t err_pos,
                             uint32_t &key_index);
  
  /**
   * @brief       SetHandleCancelAuditInTclib
   *              Setter function to set flag when CancelAudit is received
   *              before AuditStart notification
   * @param[in]   boolean to set/reset the flag
   * @retval      None
   */
  void SetHandleCancelAuditInTclib(pfc_bool_t);

  /**
   * @brief       IsHandleCancelAuditInTclib
   *              Getter function to return the flag to determine whether
   *              the CancelAudit notification is received before AuditStart
   * @param       None
   * @retval      boolean
   */
  pfc_bool_t IsHandleCancelAuditInTclib(); 

 private:

  /**
   * @brief      mutex control inside tclib ValidateOperTypeSequence 
   */
  pfc::core::Mutex tclib_validate_seq_mutex_;

  /**
   * @brief      pointer to update the TclibInterface handler
   */
  TcLibInterface *pTcLibInterface_;

  /**
   * @brief      pointer to update the session which requested for ipc service
   *             passed to tclib message util also to read from session
   */
  pfc::core::ipc::ServerSession *sess_;

  /**
   * @brief      mutex control inside tclib service handling
   */
  pfc::core::Mutex tclib_mutex_;

  /**
   * @brief      mutex to control ipcservices handling
   */
  pfc::core::Mutex tclib_ipc_control_mutex_;

  /*
   * @brief       holds config data based on session id
   */

  TcNotifyConfigDataMap tc_notify_config_data_map_;

  /**
   * @brief      Mutex to control tc_notify_config_data_map
   */
    
  pfc::core::Mutex tc_notify_config_data_map_lock_;

  /**
   * @brief      Updated on recieving notify session/config id
   *             used for validation of update messages
   */
  uint32_t session_id_;

  /**
   * @brief      Updated on recieving notify session/config id
   *             used for validation of update messages
   */
  uint32_t config_id_;

  /**
   * @brief      Used while handling driver result in commit/audit operations
   *             updates controller id and validates the same in write APIs
   */
  std::string controllerid_;

  /**
   * @brief      Updated on recieving the audit request for driver tclib
   *             loaded modules.
   *             used for valiadation of oper states in driver tclib modules
   */
  pfc_bool_t audit_in_progress_;

  /**
   * @brief      Holds the current running oper state in tclib
   */
  TcMsgOperType oper_state_;

  /**
   * @brief      Used while handling driver result in commit/audit operations
   */
  TcKeyTypeIndexMap key_map_;

  /**
   * @brief      Used while handling driver result in commit/audit operations
   */
  TcControllerKeyTypeMap controller_key_map_;

  /**
   * @brief      Used while handling driver result in commit/audit operations
   */
  TcCommitPhaseResult commit_phase_result_;

  /**
   * @brief Used to save autosave is on or off
   */
  pfc_bool_t auto_save_enabled_;

  /**
   * @brief Mutex for autosave_enabled_ member
   */
  pfc::core::Mutex tclib_autosave_mutex_;

  /**
   * @brief Request counter
   */
  uint32_t request_counter_;

  /**
   * @brief Mutex for request counter
   */
  pfc::core::Mutex request_counter_lock_;

  /**
   * @brief Mutex for audit-cancel received first flag
   */
  pfc::core::Mutex handle_in_tclib_mutex_;

  /**
   * @brief Flag for audit-cancel received before AuditStart
   */
  pfc_bool_t handle_in_tclib_;
};
}  // namespace tclib
}  // namespace unc

#endif /* _UNC_TCLIB_MODULE_HH_ */
