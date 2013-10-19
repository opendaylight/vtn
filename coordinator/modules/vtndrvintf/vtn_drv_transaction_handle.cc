/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made
 * available under the  terms of the Eclipse Public License v1.0 which
 * accompanies this  distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 */

#include <vtn_drv_transaction_handle.hh>
#include <list>
#include <vector>

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
  pfc_log_debug("inside HandleCommitVoteRequest");
  std::string ctr_name;
  driver* drv = NULL;
  controller* ctr = NULL;
  unc::tclib::TcCommonRet ret_code = unc::tclib::TC_SUCCESS;
  ctr_iter iter;

  unc::tclib::TcLibModule* tclib_ptr =
              static_cast<unc::tclib::TcLibModule*>
             (unc::tclib::TcLibModule::getInstance("tclib"));

  if (tclib_ptr == NULL) {
     pfc_log_error("tclib getInstance returned NULL");
     return unc::tclib::TC_FAILURE;
  }

  for (iter = controllers.begin(); iter != controllers.end(); iter++) {
    pfc_bool_t vote = false;
    ctr_name = *iter;
    uint32_t err_code = crtl_inst_->GetDriverByControllerName(ctr_name,
                                                              &ctr, &drv);
    pfc_log_error("Inside controller list iteration %d", err_code);
    if (err_code == 0) {
      if (ctr != NULL) {
          pfc_log_error("gettin conn status..ctr nt NULL");
          if (ctr->get_connection_status() == CONNECTION_DOWN) {
            //  check controller connection status, if down send disconnected
            pfc_log_debug("%s Controller status is down, send disconnected", \
                         PFC_FUNCNAME);
            tclib_ptr->TcLibWriteControllerInfo(ctr_name,
                         (uint32_t)DRVAPI_RESPONSE_CONTROLLER_DISCONNECTED, 0);
            continue;
          }
      }
      vote =  drv->is_2ph_commit_support_needed();
      if (vote) {
        ret_code = drv->HandleVote(ctr);
        if (ret_code != unc::tclib::TC_SUCCESS) {
          AbortControllers(controllers);
        return unc::tclib::TC_FAILURE;
        } else {
          pfc_log_debug("HandleVote success for controller:%s",
                                                    ctr_name.c_str());
        }
      } else {
        pfc_log_debug("TcLibWriteControllerInfo for controller:%s",
                                                  ctr_name.c_str());
        tclib_ptr->TcLibWriteControllerInfo(ctr_name,
                                        (uint32_t)DRVAPI_RESPONSE_SUCCESS, 0);
      }
    } else {
      pfc_log_fatal("unable to get controller/driver connection pointer");
    }
  }

  pfc_log_debug("Exiting HandleCommitVoteRequest");
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
  pfc_log_debug("inside AbortControllers");
  std::string ctr_name;
  driver* drv = NULL;
  controller* ctr = NULL;
  uint32_t ret_code = unc::tclib::TC_SUCCESS;
  ctr_iter  iter;
  for (iter = controllers.begin(); iter != controllers.end(); iter++) {
    pfc_bool_t Abort = false;
    ctr_name = *iter;
    uint32_t err_code = crtl_inst_->GetDriverByControllerName(ctr_name,
                                                              &ctr, &drv);
    if (err_code == 0) {
      Abort =  drv->is_2ph_commit_support_needed();
      if (Abort)
        ret_code = drv->HandleAbort(ctr);
      if (ret_code == unc::tclib::TC_SUCCESS) {
        pfc_log_debug("abort controller :%s ret_code %d", ctr_name.c_str(),
                     ret_code);
      } else {
        pfc_log_debug("abort controller :%s failed ret_code %d",
                                              ctr_name.c_str(), ret_code);
      }
  }
  }  // for loop
  pfc_log_debug("Exiting AbortControllers");
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
  pfc_log_debug("inside HandleCommitGlobalCommit");
  std::string ctr_name;
  driver* drv = NULL;
  controller* ctr = NULL;
  uint32_t retc = DRVAPI_RESPONSE_FAILURE;
  ctr_iter  iter;
  unc::tclib::TcCommonRet ret_code = unc::tclib::TC_SUCCESS;
  unc::tclib::TcLibModule* tclib_ptr =
            static_cast<unc::tclib::TcLibModule*>
           (unc::tclib::TcLibModule::getInstance("tclib"));

  if (tclib_ptr == NULL) {
     pfc_log_fatal("tclib getInstance returned NULL");
     return unc::tclib::TC_FAILURE;
  }
  for (iter = controllers.begin(); iter != controllers.end(); iter++) {
    pfc_bool_t commit = false;
    ctr_name = *iter;
    uint32_t err_code = crtl_inst_->GetDriverByControllerName(ctr_name,
                                                              &ctr, &drv);
  if (ctr == NULL) {
      pfc_log_error("ctr is null : HandleCommitGlobalCommit");
      return ret_code;
    }
    if (drv == NULL) {
      pfc_log_error("drv is null : HandleCommitGlobalCommit");
      return ret_code;
    }

    if (err_code == VTN_DRV_RET_SUCCESS) {
      //  check controller connection status, if down send disconnected
      if (ctr->get_connection_status() == CONNECTION_DOWN) {
        pfc_log_debug("%s Controller status is down, send disconnected", \
                       PFC_FUNCNAME);
        tclib_ptr->TcLibWriteControllerInfo(ctr_name,
                   (uint32_t)DRVAPI_RESPONSE_CONTROLLER_DISCONNECTED, 0);
        ret_code = unc::tclib::TC_SUCCESS;
        continue;
      }
      commit =  drv->is_2ph_commit_support_needed();
      if (commit) {
        ret_code = drv->HandleCommit(ctr);
        if (ret_code != unc::tclib::TC_SUCCESS)
          pfc_log_fatal("Global Commit Failure in driver");
          return unc::tclib::TC_FAILURE;
      } else {
        if (ctr->keytree_ptr != NULL) {
          unc::vtndrvcache::CommonIterator* itr_ptr =
              ctr->keytree_ptr->get_iterator();
          if (itr_ptr == NULL) {
            pfc_log_fatal("%s: exit. itr_ptr is null", PFC_FUNCNAME);
            return unc::tclib::TC_FAILURE;
          }
          uint32_t size = ctr->keytree_ptr->cfg_list_count();
          pfc_log_debug("config node size is %d for controller %s",
                       size, ctr_name.c_str());
          unc::vtndrvcache::ConfigNode *cfgnode = NULL;

          for (cfgnode = itr_ptr->FirstItem(); itr_ptr->IsDone() == false;
               cfgnode = itr_ptr->NextItem() ) {
            unc_key_type_t keytype = cfgnode->get_type();
            pfc_log_debug("%u,keytype", keytype);

            KtHandler* hnd_ptr = kt_handler_map_.find(keytype)->second;
            PFC_ASSERT(hnd_ptr != NULL);
            retc = hnd_ptr->execute_cmd(cfgnode, ctr, drv);
            if (retc == DRVAPI_RESPONSE_SUCCESS)
              ret_code = unc::tclib::TC_SUCCESS;
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
            }
          }
          if (itr_ptr != NULL)
            delete itr_ptr;
        } else {
          pfc_log_fatal("no Cache in Controller");
          return unc::tclib::TC_FAILURE;
        }
      }
    } else {
      pfc_log_fatal("unable to get controller/driver connection pointer");
      return unc::tclib::TC_FAILURE;
    }
    if (ret_code == unc::tclib::TC_SUCCESS) {
      unc::tclib::TcLibModule* tclib_ptr =
                    static_cast<unc::tclib::TcLibModule*>
                    (unc::tclib::TcLibModule::getInstance("tclib"));
      tclib_ptr->TcLibWriteControllerInfo(ctr_name, retc, 0);
    }
    ctr->keytree_ptr->clear_commit_cache();
    pfc_log_debug("Config Node cleared for %s", ctr_name.c_str());
    if (ctr->keytree_ptr != NULL) {
      pfc_log_debug("delete keytree_ptr for:%s", ctr_name.c_str());
      delete ctr->keytree_ptr;
      ctr->keytree_ptr = NULL;
    }
  }  // for loop
  pfc_log_debug("Exiting HandleCommitGlobalCommit");
  return ret_code;
}

/**
* @Description :Method to fill the map pfc_ipcstdef_t pointer against keytype
* @param[in]   :NONE
* @return      :NONE
**/
void  DriverTxnInterface::initialize_map() {
  uint32_t loop = 0;
  unc_key_type_t KT[3] = {UNC_KT_VTN, UNC_KT_VBRIDGE, UNC_KT_VBR_IF};
  uint32_t size = sizeof KT/sizeof(unc_key_type_t);
  for (loop = 0; loop < size; loop++) {
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
      default:
        break;
    }
  }
}
}  // namespace driver
}  // namespace unc
