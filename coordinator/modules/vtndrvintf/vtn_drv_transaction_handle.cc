/*
 * Copyright (c) 2013-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <vtn_drv_transaction_handle.hh>
#include <list>
#include <vector>
#include <memory>

namespace unc {
namespace driver {

/**
* @brief : constructor
*/
DriverTxnInterface::DriverTxnInterface(ControllerFramework* ctrl_frame,
                            kt_handler_map &map_kt)
    : crtl_inst_(ctrl_frame), kt_handler_map_(map_kt) {
      initialize_map();
    }


/**
 * @brief : Destructor
 */
DriverTxnInterface::~DriverTxnInterface() {
  std::map<unc_key_type_t, pfc_ipcstdef_t*> :: iterator map_it;
  for (map_it = key_map_.begin(); map_it != key_map_.end(); map_it++) {
    delete map_it->second;
    map_it->second = NULL;
  }
  for (map_it = val_map_.begin(); map_it != val_map_.end(); map_it++) {
    delete map_it->second;
    map_it->second = NULL;
  }

  key_map_.clear();
  val_map_.clear();
}

/**
* @Description :This functions invokes when TC sends COMMIT request to driver
* @param[in]   :session_id - ipc session id used for TC validation
                config_id - configuration id used for TC validation
                controllers -list contains controller names
* @return      :TC_SUCCESS is returned when COMMIT is success for all
*               controllers.
*               TC_FAILURE is returned when COMMIT is failure for any
*               one of the controller in the list
**/
unc::tclib::TcCommonRet DriverTxnInterface::HandleCommitGlobalCommit(
    uint32_t session_id,
    uint32_t config_id,
    unc::tclib::TcControllerList
    controllers) {
  ODC_FUNC_TRACE;
  std::string ctr_name;
  driver* drv = NULL;
  controller* ctr = NULL;
  unc::tclib::TcCommonRet ret_code = unc::tclib::TC_SUCCESS;
  ctr_iter iter;
  pfc_bool_t Abort = PFC_FALSE;
  unc::tclib::TcLibModule* tclib_ptr =
      static_cast<unc::tclib::TcLibModule*>
      (unc::tclib::TcLibModule::getInstance("tclib"));
  PFC_ASSERT(tclib_ptr != NULL);

  for (iter = controllers.begin(); iter != controllers.end(); iter++) {
    pfc_bool_t commit = PFC_FALSE;
    ctr_name = *iter;
    controller_operation util_obj(crtl_inst_, WRITE_TO_CONTROLLER, ctr_name);
    if (util_obj.get_controller_status() == PFC_FALSE) {
      pfc_log_debug("%s Controller not exist, send disconnected", \
                                               PFC_FUNCNAME);
      tclib_ptr->TcLibWriteControllerInfo(ctr_name,
                          (uint32_t)UNC_RC_CTR_DISCONNECTED, 0);
      return unc::tclib::TC_SUCCESS;
    }
    ctr = util_obj.get_controller_handle();
    drv = util_obj.get_driver_handle();
    PFC_ASSERT(ctr != NULL);
    PFC_ASSERT(drv != NULL);
    if (ctr != NULL) {
      pfc_log_error("gettin conn status..ctr nt NULL");
      if (ctr->get_connection_status() == CONNECTION_DOWN) {
        //  check controller connection status, if down send disconnected
        pfc_log_debug("%s Controller status is down, send disconnected", \
                      PFC_FUNCNAME);
        tclib_ptr->TcLibWriteControllerInfo(ctr_name,
                      (uint32_t)UNC_RC_CTR_DISCONNECTED, 0);
        continue;
      }
    }
    commit =  drv->is_2ph_commit_support_needed();
    if (commit == PFC_TRUE) {
      ret_code = drv->HandleCommit(ctr);

      if (ret_code != unc::tclib::TC_SUCCESS) {
        Abort = PFC_TRUE;
        break;
      } else {
        pfc_log_debug("HandleCommitGlobalCommit success for controller:%s",
                      ctr_name.c_str());
      }
    } else {
      pfc_log_debug("TcLibWriteControllerInfo for controller:%s",
                    ctr_name.c_str());
      tclib_ptr->TcLibWriteControllerInfo(ctr_name,
                                    (uint32_t)UNC_RC_SUCCESS, 0);
    }
  }
  if (Abort == PFC_TRUE)
    AbortControllers(controllers);
  return ret_code;
}

/**
* @brief      : This functions invokes when TC sends Audit VOTE request to drv
* @param[in]  : session_id - ipc session id used for TC validation
*               controller_id - controller_id used for TC validation
*               controllers -list contains controller names
* @return     : TC_SUCCESS is returned when VOTE is success for all
*               controllers.
*               TC_FAILURE is returned when VOTE is failure for any
*               one of the controller in the list
*/

unc::tclib::TcCommonRet
DriverTxnInterface::HandleAuditVoteRequest(uint32_t session_id,
                                     std::string controller_id,
                      unc::tclib::TcControllerList controllers) {
  ODC_FUNC_TRACE;
  uint32_t config_id = 0;
  unc::tclib::TcCommonRet ret_code = unc::tclib::TC_FAILURE;
  ret_code = HandleCommitVoteRequest(session_id, config_id, controllers);
  if (ret_code ==  unc::tclib::TC_FAILURE) {
    pfc_log_debug("Audit VOTE Failed");
    return unc::tclib::TC_FAILURE;
  }
  return unc::tclib::TC_SUCCESS;
}

/**
* @brief      : This functions invokes when TC sends AuditGlobalCommit to drv
* @param[in]  : session_id - ipc session id used for TC validation
*               controller_id - controller_id used for TC validation
*               controllers -list contains controller names
* @return     : TC_SUCCESS is returned when VOTE is success for all
*               controllers.
*               TC_FAILURE is returned when VOTE is failure for any
*               one of the controller in the list
*/
unc::tclib::TcCommonRet
DriverTxnInterface::HandleAuditGlobalCommit(uint32_t session_id,
                                      std::string controller_id,
                       unc::tclib::TcControllerList controllers) {
  ODC_FUNC_TRACE;
  uint32_t config_id = 0;
  unc::tclib::TcCommonRet ret_code = unc::tclib::TC_FAILURE;
  ret_code = HandleCommitGlobalCommit(session_id, config_id, controllers);
  if (ret_code ==  unc::tclib::TC_FAILURE) {
    pfc_log_debug("Audit Global commit Failed");
    return unc::tclib::TC_FAILURE;
  }
  return  unc::tclib::TC_SUCCESS;
}

/**
* @Description :This functions invokes when Vote is failed for controller
* @param[in]   :controllers -list contains controller names
* @return      :NONE
**/
void DriverTxnInterface::AbortControllers(unc::tclib::TcControllerList
                                          controllers) {
  ODC_FUNC_TRACE;
  std::string ctr_name;
  driver* drv = NULL;
  controller* ctr = NULL;
  uint32_t ret_code = unc::tclib::TC_SUCCESS;
  ctr_iter  iter;
  for (iter = controllers.begin(); iter != controllers.end(); iter++) {
    pfc_bool_t Abort = PFC_FALSE;
    ctr_name = *iter;
    controller_operation util_obj(crtl_inst_, WRITE_TO_CONTROLLER, ctr_name);
    ctr = util_obj.get_controller_handle();
    drv = util_obj.get_driver_handle();
    PFC_ASSERT(ctr != NULL);
    PFC_ASSERT(drv != NULL);
    Abort =  drv->is_2ph_commit_support_needed();
    if (Abort == PFC_TRUE) {
      ret_code = drv->HandleAbort(ctr);
      if (ret_code == unc::tclib::TC_SUCCESS) {
        pfc_log_debug("abort controller :%s ret_code %d", ctr_name.c_str(),
                      ret_code);
      } else {
        pfc_log_error("abort controller :%s failed ret_code %d",
                      ctr_name.c_str(), ret_code);
      }
    } else {
      if (ctr->controller_cache != NULL) {
        pfc_log_debug("AbortControllers::delete controller_cache for:%s",
                      ctr_name.c_str());
        delete ctr->controller_cache;
        ctr->controller_cache = NULL;
      }
    }
  }  // for loop
}

/**
* @brief      : This functions invokes when TC sends VOTE request to driver
* @param[in]  : session_id - ipc session id used for TC validation
                config_id - configuration id used for TC validation
                controllers -list contains controller names
* @return     : TC_SUCCESS is returned when VOTE is success for all
*               controllers.
*               TC_FAILURE is returned when VOTE is failure for any
*               one of the controller in the list
**/
unc::tclib::TcCommonRet DriverTxnInterface::HandleCommitVoteRequest(
                                        uint32_t session_id,
                                        uint32_t config_id,
                                        unc::tclib::TcControllerList
                                        controllers) {
  ODC_FUNC_TRACE;
  pfc_bool_t Abort = PFC_FALSE;
  std::string ctr_name;
  driver* drv = NULL;
  controller* ctr = NULL;
  uint32_t retc = UNC_DRV_RC_ERR_GENERIC;
  ctr_iter  iter;
  unc::tclib::TcCommonRet ret_code = unc::tclib::TC_SUCCESS;
  unc::tclib::TcLibModule* tclib_ptr =
            static_cast<unc::tclib::TcLibModule*>
           (unc::tclib::TcLibModule::getInstance("tclib"));
  PFC_ASSERT(tclib_ptr != NULL);

  for (iter = controllers.begin(); iter != controllers.end(); iter++) {
    pfc_bool_t vote = PFC_FALSE;
    ctr_name = *iter;
    controller_operation util_obj(crtl_inst_, WRITE_TO_CONTROLLER, ctr_name);
    if (util_obj.get_controller_status() == PFC_FALSE) {
      pfc_log_debug("%s Controller not exist, send disconnected", \
                                               PFC_FUNCNAME);
      tclib_ptr->TcLibWriteControllerInfo(ctr_name,
                          (uint32_t)UNC_RC_CTR_DISCONNECTED, 0);
      return unc::tclib::TC_SUCCESS;
    }

    ctr = util_obj.get_controller_handle();
    drv = util_obj.get_driver_handle();
    PFC_ASSERT(ctr != NULL);
    PFC_ASSERT(drv != NULL);
    if (ctr->get_connection_status() == CONNECTION_DOWN) {
      pfc_log_debug("%s Controller status is down, send disconnected", \
                    PFC_FUNCNAME);
      tclib_ptr->TcLibWriteControllerInfo(ctr_name,
                    (uint32_t)UNC_RC_CTR_DISCONNECTED, 0);
      ret_code = unc::tclib::TC_SUCCESS;
      continue;
    }
    vote =  drv->is_2ph_commit_support_needed();
    if (vote == PFC_TRUE) {
      ret_code = drv->HandleVote(ctr);
      if (ret_code != unc::tclib::TC_SUCCESS) {
        Abort = PFC_TRUE;
        pfc_log_error("VOTE Failure in driver");
        break;
      }
    } else {
      ret_code = HandleCommitCache(ctr_name, ctr, drv);
      if (ret_code !=unc::tclib::TC_SUCCESS) {
        pfc_log_error("VOTE Failure in driver, ret %u", ret_code);
        AbortControllers(controllers);
        pfc_log_debug("Exiting HandleCommitVoteRequest");
        return unc::tclib::TC_FAILURE;
      } else {
        retc = UNC_RC_SUCCESS;
      }
    }
    if (ret_code == unc::tclib::TC_SUCCESS) {
      tclib_ptr->TcLibWriteControllerInfo(ctr_name, retc, 0);
    }
    if (ctr->controller_cache != NULL) {
      pfc_log_debug("delete controller_cache for:%s", ctr_name.c_str());
      delete ctr->controller_cache;
      ctr->controller_cache = NULL;
    }
  }  // for loop
  if (Abort == PFC_TRUE)
    AbortControllers(controllers);
  return ret_code;
}

/**
 * @brief       - Method to Handle the controller cache
 * @param[in]   - controller name,controller*,
 *                driver*
 * @retval      - TcCommonRet enum value
 */
unc::tclib::TcCommonRet DriverTxnInterface::HandleCommitCache
                                            (std::string ctr_name,
                                             controller* ctr,
                                             driver* drv) {
  ODC_FUNC_TRACE;
  uint32_t retc = UNC_DRV_RC_ERR_GENERIC;
  unc::tclib::TcCommonRet ret_code = unc::tclib::TC_FAILURE;
  PFC_ASSERT(ctr != NULL);

  unc::tclib::TcLibModule* tclib_ptr =
      static_cast<unc::tclib::TcLibModule*>
      (unc::tclib::TcLibModule::getInstance("tclib"));
  PFC_ASSERT(tclib_ptr != NULL);
  if (ctr->controller_cache != NULL) {
    std::auto_ptr<unc::vtndrvcache::CommonIterator>
        itr_ptr(ctr->controller_cache->create_iterator());
    uint32_t size = ctr->controller_cache->cfg_list_count();
    pfc_log_debug("config node size is %d for controller %s",
                  size, ctr_name.c_str());
    unc::vtndrvcache::ConfigNode *cfgnode = NULL;
    //  get the controoler configuration from config node and execute
    for (cfgnode = itr_ptr->FirstItem(); itr_ptr->IsDone() == false;
         cfgnode = itr_ptr->NextItem() ) {
      unc_key_type_t keytype = cfgnode->get_type_name();
      pfc_log_debug("%u,keytype", keytype);
      std::map <unc_key_type_t, KtHandler*>::iterator
          iter = kt_handler_map_.begin();
      iter = kt_handler_map_.find(keytype);
      KtHandler* hnd_ptr = NULL;
      if (iter != kt_handler_map_.end()) {
        hnd_ptr = iter->second;
      }
      PFC_ASSERT(hnd_ptr != NULL);
      retc = hnd_ptr->execute_cmd(cfgnode, ctr, drv);
      if (retc == UNC_RC_SUCCESS)
        ret_code = unc::tclib::TC_SUCCESS;
      // any command execution failed for controller write the error to Tclib
      if (ret_code != unc::tclib::TC_SUCCESS) {
        pfc_log_debug("%u,execute_cmd not success", keytype);
        tclib_ptr->TcLibWriteControllerInfo(ctr_name, retc, 1);
        void* key = hnd_ptr->get_key_struct(cfgnode);
        void* val = hnd_ptr->get_val_struct(cfgnode);
        pfc_ipcstdef_t* key_sdf =key_map_.find(keytype)->second;
        pfc_ipcstdef_t* val_sdf =val_map_.find(keytype)->second;
        tclib_ptr->TcLibWriteKeyValueDataInfo(ctr_name, (uint32_t)keytype,
                                              *key_sdf, *val_sdf,
                                              key, val);
        break;
      }
    }
  } else {
    pfc_log_error("no Cache in Controller");
    return unc::tclib::TC_FAILURE;
  }
  return ret_code;
}

  /**
   * @brief       - Handles Audit end
   * @param[in]   - session id
   * @param[in]   - controller type
   * @param[in]   - controller id
   * @param[in]   - audit result
   * @retval      - returns TC_SUCCESS
   */
unc::tclib::TcCommonRet DriverTxnInterface::HandleAuditEnd(uint32_t session_id,
                                       unc_keytype_ctrtype_t ctr_type,
                                       std::string
                                       controller_id,
                                       unc::tclib::TcAuditResult
                                       audit_result) {
  controller* ctr = NULL;
  unc::tclib::TcLibModule* tclib_ptr =
      static_cast<unc::tclib::TcLibModule*>
      (unc::tclib::TcLibModule::getInstance("tclib"));
  PFC_ASSERT(tclib_ptr != NULL);
  controller_operation util_obj(crtl_inst_,
                                READ_FROM_CONTROLLER,
                                controller_id);
  if (util_obj.get_controller_status() == PFC_FALSE) {
    pfc_log_debug("%s Controller not exist, send disconnected", \
                                               PFC_FUNCNAME);
    tclib_ptr->TcLibWriteControllerInfo(controller_id,
                          (uint32_t)UNC_RC_CTR_DISCONNECTED, 0);
    return unc::tclib::TC_SUCCESS;
  }

  ctr = util_obj.get_controller_handle();
  PFC_ASSERT(ctr != NULL);
  if (audit_result == unc::tclib::TC_AUDIT_SUCCESS) {
    pfc_log_debug("AUDIT_END result:%d", audit_result);
    ctr->audit_result_ = PFC_TRUE;
  }
  return unc::tclib::TC_SUCCESS;
}

/**
* @Description :Method to fill the map pfc_ipcstdef_t pointer against keytype
* @param[in]   :NONE
* @return      :NONE
**/
void  DriverTxnInterface::initialize_map() {
  ODC_FUNC_TRACE;
  uint32_t loop = 0;
  unc_key_type_t KT[] = {UNC_KT_VTN, UNC_KT_VBRIDGE, UNC_KT_VBR_IF,
                         UNC_KT_VBR_VLANMAP};
  uint32_t kt_size = sizeof KT/sizeof(unc_key_type_t);
  for (loop = 0; loop < kt_size; loop++) {
    switch (KT[loop]) {
      case UNC_KT_VTN:
        {
        pfc_ipcstdef_t *stdef_k = new pfc_ipcstdef_t;
        PFC_IPC_STDEF_INIT(stdef_k, key_vtn);
        pfc_ipcstdef_t *stdef_v = new pfc_ipcstdef_t;
        PFC_IPC_STDEF_INIT(stdef_v, val_vtn);
        key_map_.insert(std::pair<unc_key_type_t, pfc_ipcstdef_t*>(KT[loop],
                                                                  stdef_k));
        val_map_.insert(std::pair<unc_key_type_t, pfc_ipcstdef_t*>(KT[loop],
                                                                  stdef_v));
        break;
        }
      case UNC_KT_VBRIDGE:
        {
        pfc_ipcstdef_t *stdef_kvbr = new pfc_ipcstdef_t;
        PFC_IPC_STDEF_INIT(stdef_kvbr, key_vbr);
        pfc_ipcstdef_t *stdef_vbr = new pfc_ipcstdef_t;
        PFC_IPC_STDEF_INIT(stdef_vbr, val_vbr);
        key_map_.insert(std::pair<unc_key_type_t, pfc_ipcstdef_t*>(KT[loop],
                                                                  stdef_kvbr));
        val_map_.insert(std::pair<unc_key_type_t, pfc_ipcstdef_t*>(KT[loop],
                                                                  stdef_vbr));
        break;
        }
      case UNC_KT_VBR_IF:
        {
        pfc_ipcstdef_t *stdef_kvbrif = new pfc_ipcstdef_t;
        PFC_IPC_STDEF_INIT(stdef_kvbrif, key_vbr_if);
        pfc_ipcstdef_t *stdef_vbrif = new pfc_ipcstdef_t;
        PFC_IPC_STDEF_INIT(stdef_vbrif, val_vbr_if);
        key_map_.insert(std::pair<unc_key_type_t, pfc_ipcstdef_t*>(KT[loop],
                                                                stdef_kvbrif));
        val_map_.insert(std::pair<unc_key_type_t, pfc_ipcstdef_t*>(KT[loop],
                                                                stdef_vbrif));
        break;
        }
      case UNC_KT_VBR_VLANMAP:
        {
          pfc_ipcstdef_t *stdef_kvbrvlanmap = new pfc_ipcstdef_t;
          PFC_IPC_STDEF_INIT(stdef_kvbrvlanmap, key_vlan_map);
          pfc_ipcstdef_t *stdef_vbrvlanmap = new pfc_ipcstdef_t;
          PFC_IPC_STDEF_INIT(stdef_vbrvlanmap, val_vlan_map);
          key_map_.insert(std::pair<unc_key_type_t, pfc_ipcstdef_t*>(KT[loop],
                                                          stdef_kvbrvlanmap));
          val_map_.insert(std::pair<unc_key_type_t, pfc_ipcstdef_t*>(KT[loop],
                                                           stdef_vbrvlanmap));
          break;
        }
      default:
        break;
    }
  }
}
}  // namespace driver
}  // namespace unc
