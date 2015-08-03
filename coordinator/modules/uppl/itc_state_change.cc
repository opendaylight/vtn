/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * @brief  :  ITC SystemStateChange Request
 * @file   :  itc_state_change.cc
 *
 */

#include "physicallayer.hh"
#include "itc_state_change.hh"
#include "phy_util.hh"
#include "odbcm_mgr.hh"
#include "itc_kt_base.hh"
#include "itc_kt_controller.hh"
#include "unc/upll_svc.h"
#include "unc/upll_errno.h"
#include "ipc_client_configuration_handler.hh"
#include "odbcm_db_varbind.hh"
#include "ipct_util.hh"
namespace unc {
namespace uppl {

/**SystemStateChangeRequest
 * @Description : This function initializes the member data
 * @param[in]   : None
 * @return      : None
 */
SystemStateChangeRequest::SystemStateChangeRequest() {
}

/**~SystemStateChangeRequest
 * @Description : This function releases memory allocated to
 *                pointer member data
 * @param[in]   : None
 * @return      : None
 */
SystemStateChangeRequest::~SystemStateChangeRequest() {
}

/**SystemStateChangeToStandBy
 * @Description : This function change the system state from Active to Standby
 *                Gets the controller list from database and send the DELETE
 *                request to driver
 * @param[in]   : None
 * @return      : UNC_RC_SUCCESS if the system state is changed to standby
 *                or UNC_UPPL_RC_ERR_* if the switchover fails
 * */
UncRespCode SystemStateChangeRequest::SystemStateChangeToStandBy(
    OdbcmConnectionHandler *db_conn) {
  pfc_log_info("Start SystemStateChangeToStandBy");
  Kt_Controller kt_ctr;
  vector<void *> vect_ctr_key, vect_ctr_val;
  key_ctr_t key_ctr_obj;
  memset(&key_ctr_obj, 0, sizeof(key_ctr_obj));
  vect_ctr_key.push_back(reinterpret_cast<void *>(&key_ctr_obj));
  UncRespCode read_status = kt_ctr.ReadInternal(db_conn, vect_ctr_key,
                                                   vect_ctr_val,
                                                   UNC_DT_RUNNING,
                                                   UNC_OP_READ_SIBLING_BEGIN);
  if (read_status != UNC_RC_SUCCESS) {
    pfc_log_info("read from running db is %d", read_status);
    // Check for entries in candidate db
    return read_status;
  }
  UncRespCode err = UNC_RC_SUCCESS;
  IPCClientDriverHandler pfc_drv_handler(UNC_CT_PFC, err);
  if (err != UNC_RC_SUCCESS) {
    pfc_log_error("Cannot open session to PFC driver");
    ClearVector(vect_ctr_key, vect_ctr_val);
    return err;
  }
  IPCClientDriverHandler vnp_drv_handler(UNC_CT_VNP, err);
  if (err != UNC_RC_SUCCESS) {
    pfc_log_error("Cannot open session to VNP driver");
    ClearVector(vect_ctr_key, vect_ctr_val);
    return err;
  }
  IPCClientDriverHandler polc_drv_handler(UNC_CT_POLC, err);
  if (err != UNC_RC_SUCCESS) {
    pfc_log_error("Cannot open session to POLC driver");
    ClearVector(vect_ctr_key, vect_ctr_val);
    return err;
  }
  IPCClientDriverHandler odc_drv_handler(UNC_CT_ODC, err);
  if (err != UNC_RC_SUCCESS) {
    pfc_log_error("Cannot open session to ODC driver");
    ClearVector(vect_ctr_key, vect_ctr_val);
    return err;
  }
  for (uint32_t ctrIndex = 0; ctrIndex < vect_ctr_key.size();
      ctrIndex ++) {
    key_ctr_t *ctr_key =
        reinterpret_cast<key_ctr_t*>(vect_ctr_key[ctrIndex]);
    string controller_name = (const char*)ctr_key->controller_name;
    pfc_log_debug("controller_name: %s", controller_name.c_str());
    val_ctr_st_t *obj_val_ctr =
        reinterpret_cast<val_ctr_st_t*>(vect_ctr_val[ctrIndex]);
    unc_keytype_ctrtype_t controller_type =
        (unc_keytype_ctrtype_t)
        (PhyUtil::uint8touint(obj_val_ctr->controller.type));
    // Sending the Delete Request to Driver through IPC Framework
    //  construct IPC structure with DELETE  operation
    ClientSession *cli_session = NULL;
    if (controller_type == UNC_CT_PFC) {
      pfc_log_debug("Send controller info to PFC driver");
      cli_session = pfc_drv_handler.ResetAndGetSession();
    } else if (controller_type == UNC_CT_VNP) {
      pfc_log_debug("Send controller info to VNP driver");
      cli_session = vnp_drv_handler.ResetAndGetSession();
    } else if (controller_type == UNC_CT_POLC) {
      pfc_log_debug("Send controller info to POLC driver");
      cli_session = polc_drv_handler.ResetAndGetSession();
    } else if (controller_type == UNC_CT_ODC) {
      pfc_log_debug("Send controller info to ODC driver");
      cli_session = odc_drv_handler.ResetAndGetSession();
    } else {
      pfc_log_info("Driver support not yet added for unknown controller");
      // Release memory allocated for key struct
      delete ctr_key;
      ctr_key = NULL;
      // delete the val memory
      delete obj_val_ctr;
      obj_val_ctr = NULL;
      continue;
    }
    //  getting commit version values from DT_RUNNING
    vector<string> vect_controller_id;
    vector<val_ctr_commit_ver_t> vect_val_ctr_cv;
    uint32_t max_rep_ct = 1;
    val_ctr_commit_ver_t ctr_val_struct;
    memset(&ctr_val_struct, 0, sizeof(val_ctr_commit_ver_t));
    UncRespCode read_status = kt_ctr.ReadCtrCommitVerFromDB(db_conn,
                          reinterpret_cast<void*>(ctr_key),
                          NULL,
                          UNC_DT_RUNNING, UNC_OP_READ, max_rep_ct,
                          vect_val_ctr_cv, vect_controller_id);
    if (read_status != UNC_RC_SUCCESS) {
      pfc_log_error("Error in read commit version from DB");
      ctr_val_struct.valid[kIdxCtrCommitNumber] = UNC_VF_VALID;
      ctr_val_struct.valid[kIdxCtrCommitDate] = UNC_VF_VALID;
      ctr_val_struct.valid[kIdxCtrCommitApplication] = UNC_VF_VALID;
    } else {
      ctr_val_struct =  vect_val_ctr_cv[0];
    }
    memcpy(&ctr_val_struct.controller, &obj_val_ctr->controller,
            sizeof(obj_val_ctr->controller));
    ctr_val_struct.valid[kIdxCtrVal] = UNC_VF_VALID;
    string domain_id = "";
    driver_request_header rqh = {0, 0, controller_name, domain_id,
        UNC_OP_DELETE, 0, (uint32_t)0, (uint32_t)0,
        UNC_DT_RUNNING, UNC_KT_CONTROLLER};
    int err = PhyUtil::sessOutDriverReqHeader(*cli_session, rqh);
    err |= cli_session->addOutput(*ctr_key);
    err |= cli_session->addOutput(ctr_val_struct);
    pfc_log_info("KEY:%s",
                        IpctUtil::get_string(*ctr_key).c_str());
    pfc_log_info("OLD VAL:%s",
                        IpctUtil::get_string(*obj_val_ctr).c_str());
    if (err != UNC_RC_SUCCESS) {
      pfc_log_info("Could not open driver ipc session");
      // Release memory allocated for key struct
      delete ctr_key;
      ctr_key = NULL;
      // delete the val memory
      delete obj_val_ctr;
      obj_val_ctr = NULL;
      continue;
    }
    // Send the request to driver
    UncRespCode driver_response = UNC_RC_SUCCESS;
    driver_response_header rsp;
    if (controller_type == UNC_CT_PFC) {
      driver_response = pfc_drv_handler.SendReqAndGetResp(rsp);
    } else if (controller_type == UNC_CT_VNP) {
      driver_response = vnp_drv_handler.SendReqAndGetResp(rsp);
    } else if (controller_type == UNC_CT_POLC) {
      driver_response = polc_drv_handler.SendReqAndGetResp(rsp);
    } else if (controller_type == UNC_CT_ODC) {
      driver_response = odc_drv_handler.SendReqAndGetResp(rsp);
    }
    pfc_log_debug("driver_response is  %d", driver_response);
    if (driver_response != UNC_RC_SUCCESS) {
      pfc_log_info(
          "Controller disconnect request failed at "
          "driver with error %d", driver_response);
    }
    // Release memory allocated for key struct
    delete ctr_key;
    ctr_key = NULL;
    // delete the val memory
    delete obj_val_ctr;
    obj_val_ctr = NULL;
    // Create a task queue to process particular controller's events
    PhyEventTaskqUtil *taskq_util = NotificationManager::get_taskq_util();
    taskq_util->del_task_queue(controller_name);
    // Delete the readwrite lock of this controller from CtrlUtil map.
    map<string, CtrOprnStatus>::iterator it;
    PhysicalLayer::ctr_oprn_mutex_.lock();
    it = PhysicalLayer::ctr_oprn_status_.find(controller_name);
    if (it != PhysicalLayer::ctr_oprn_status_.end()) {
      if (it->second.rwlock_oper_st != NULL)
        delete it->second.rwlock_oper_st;
      PhysicalLayer::ctr_oprn_status_.erase(it);
    }
    PhysicalLayer::ctr_oprn_mutex_.unlock();
  }
  PhysicalLayer::ctr_oprn_mutex_.lock();
  PhysicalLayer::ctr_oprn_status_.clear();  // Clear the utility holder map
  PhysicalLayer::ctr_oprn_mutex_.unlock();
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  PhysicalCore* physical_core = physical_layer->get_physical_core();
  physical_core->set_system_state(UPPL_SYSTEM_ST_STANDBY);
  // To clear the internal alarm details holding map
  physical_core->alarm_status_map_.clear();

  pfc_log_info("SystemStateChangeToStandby returned SUCCESS");
  return UNC_RC_SUCCESS;
}

/* ClearVector
 * @Description : This function deletes the memory stored in the vectors
 * @param[in]   : vector<void *>, vector<void * 
 * @return      : None
 *
 **/
void SystemStateChangeRequest::ClearVector(
    vector<void *> vect_ctr_key, vector<void *> vect_ctr_val) {
  for (uint32_t ctrIndex = 0; ctrIndex < vect_ctr_key.size();
      ctrIndex ++) {
    ::operator delete(vect_ctr_key[ctrIndex]);
    ::operator delete(vect_ctr_val[ctrIndex]);
  }
}


/**SystemStateChangeToActive
 * @Description : This function change the system state from Standby to active.
 *                Gets the controller list from database and send the CREATE
 *                request to driver
 * @param[in]   : None
 * @return      : UNC_RC_SUCCESS if the system state is changed to active
 *                or UNC_UPPL_RC_ERR_* if the switchover fails
 * */
UncRespCode SystemStateChangeRequest::SystemStateChangeToActive(
    OdbcmConnectionHandler *db_conn) {
  pfc_log_info("Start SystemStateChangeToActive");
  /* Get all the controller entry from running db */
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  PhysicalCore* physical_core = physical_layer->get_physical_core();
  // To clear the internal alarm details holding map
  physical_core->alarm_status_map_.clear();
  Kt_Controller kt_ctr;
  vector<void *> vect_ctr_key, vect_ctr_val;
  key_ctr_t key_ctr_obj;
  memset(&key_ctr_obj, '\0', sizeof(key_ctr_t));
  vect_ctr_key.push_back(reinterpret_cast<void *>(&key_ctr_obj));
  UncRespCode read_status = kt_ctr.ReadInternal(db_conn, vect_ctr_key,
                                                   vect_ctr_val,
                                                   UNC_DT_RUNNING,
                                                   UNC_OP_READ_SIBLING_BEGIN);
  if (read_status != UNC_RC_SUCCESS) {
    pfc_log_info("read from running db is %d", read_status);
    // Check for entries in candidate db
    return SendCandidateInfoToLogical(db_conn);
  }

  UncRespCode err = UNC_RC_SUCCESS;
  IPCClientDriverHandler pfc_drv_handler(UNC_CT_PFC, err);
  if (err != UNC_RC_SUCCESS) {
    ClearVector(vect_ctr_key, vect_ctr_val);
    pfc_log_error("Cannot open session to PFC driver");
    return err;
  }
  IPCClientDriverHandler vnp_drv_handler(UNC_CT_VNP, err);
  if (err != UNC_RC_SUCCESS) {
    ClearVector(vect_ctr_key, vect_ctr_val);
    pfc_log_error("Cannot open session to VNP driver");
    return err;
  }
  IPCClientDriverHandler polc_drv_handler(UNC_CT_POLC, err);
  if (err != UNC_RC_SUCCESS) {
    ClearVector(vect_ctr_key, vect_ctr_val);
    pfc_log_error("Cannot open session to POLC driver");
    return err;
  }
  IPCClientDriverHandler odc_drv_handler(UNC_CT_ODC, err);
  if (err != UNC_RC_SUCCESS) {
    ClearVector(vect_ctr_key, vect_ctr_val);
    pfc_log_error("Cannot open session to ODC driver");
    return err;
  }
  for (uint32_t ctrIndex = 0; ctrIndex < vect_ctr_key.size();
      ctrIndex ++) {
    key_ctr_t *ctr_key =
        reinterpret_cast<key_ctr_t*>(vect_ctr_key[ctrIndex]);
    string controller_name = (const char*)ctr_key->controller_name;
    pfc_log_debug("controller_name: %s", controller_name.c_str());
    val_ctr_st_t *obj_val_ctr =
        reinterpret_cast<val_ctr_st_t*>(vect_ctr_val[ctrIndex]);
    unc_keytype_ctrtype_t controller_type =
        (unc_keytype_ctrtype_t)
        (PhyUtil::uint8touint(obj_val_ctr->controller.type));
    if (controller_type != UNC_CT_UNKNOWN) {
      // Clearing the state entries
      pfc_log_info("Removing State entries for controller %s",
                     controller_name.c_str());
      ODBCM_RC_STATUS clear_status =
          PhysicalLayer::get_instance()->get_odbc_manager()->
          ClearOneInstance(UNC_DT_STATE, controller_name, db_conn);
      if (clear_status != ODBCM_RC_SUCCESS) {
        pfc_log_error("State DB clearing failed");
      }
      clear_status =
          PhysicalLayer::get_instance()->get_odbc_manager()->
          ClearOneInstance(UNC_DT_IMPORT, controller_name, db_conn);
      if (clear_status != ODBCM_RC_SUCCESS) {
        pfc_log_error("Import DB clearing failed");
      }
      pfc_log_info("Set the oper Status of controller as down %s",
                   controller_name.c_str());
      val_ctr_st_t *ctr_val =
        reinterpret_cast<val_ctr_st_t*>(vect_ctr_val[ctrIndex]);
      memset(ctr_val->valid, '\0', sizeof(ctr_val->valid));
      ctr_val->valid[kIdxOperStatus] = 1;
      ctr_val->oper_status = UPPL_CONTROLLER_OPER_DOWN;
      UncRespCode operation_status = kt_ctr.HandleOperStatus(
          db_conn,
          UNC_DT_RUNNING,
          vect_ctr_key[ctrIndex],
          ctr_val,
          false);
      if (operation_status != UNC_RC_SUCCESS) {
        pfc_log_error("Unable to set the oper status of controller as down");
      }
      // Reset actual version as empty in running and candidate
      string act_version = "";
      UncRespCode status = kt_ctr.SetActualVersion(
                                   db_conn, vect_ctr_key[ctrIndex], act_version,
                                   UNC_DT_RUNNING, UNC_VF_INVALID);
      if (status != UNC_RC_SUCCESS) {
        pfc_log_error("act_version reset operation failed for running");
      }
      status = kt_ctr.SetActualVersion(db_conn, vect_ctr_key[ctrIndex],
          act_version, UNC_DT_CANDIDATE, UNC_VF_INVALID);
      if (status != UNC_RC_SUCCESS) {
        pfc_log_error("act_version reset operation failed for candidate");
      }
      // Reset actual id as empty in running and candidate
      string actual_id = "";
      status = kt_ctr.SetActualControllerId(db_conn, vect_ctr_key[ctrIndex],
                                 actual_id, UNC_DT_RUNNING, UNC_VF_INVALID);
      if (status != UNC_RC_SUCCESS) {
        // log error
        pfc_log_error("actual_id reset operation failed for running");
      }
      status = kt_ctr.SetActualControllerId(db_conn, vect_ctr_key[ctrIndex],
                               actual_id, UNC_DT_CANDIDATE, UNC_VF_INVALID);
      if (status != UNC_RC_SUCCESS) {
        // log error
        pfc_log_error("actual_id reset operation failed for candidate");
      }
    }
    UncRespCode upll_result = kt_ctr.SendUpdatedControllerInfoToUPLL(
        UNC_DT_RUNNING,
        UNC_OP_CREATE,
        UNC_KT_CONTROLLER,
        vect_ctr_key[ctrIndex],
        reinterpret_cast<void*>(&obj_val_ctr->controller));
    if (upll_result != UNC_RC_SUCCESS) {
      pfc_log_info("Failed to send the controller %s in running to UPLL",
                   controller_name.c_str());
    }
    /* Sending the Create Request to Driver through IPC Framework
       construct IPC structure with CREATE  operation */
    ClientSession *cli_session = NULL;
    if (controller_type == UNC_CT_PFC) {
      pfc_log_debug("Send controller info to PFC driver");
      cli_session = pfc_drv_handler.ResetAndGetSession();
    } else if (controller_type == UNC_CT_VNP) {
      pfc_log_debug("Send controller info to VNP driver");
      cli_session = vnp_drv_handler.ResetAndGetSession();
    } else if (controller_type == UNC_CT_POLC) {
      pfc_log_debug("Send controller info to POLC driver");
      cli_session = polc_drv_handler.ResetAndGetSession();
    } else if (controller_type == UNC_CT_ODC) {
      pfc_log_debug("Send controller info to ODC driver");
      cli_session = odc_drv_handler.ResetAndGetSession();
    } else {
      pfc_log_info("Driver support not yet added for unknown controller");
      // Release memory allocated for key struct
      delete ctr_key;
      ctr_key = NULL;
      // delete the val memory
      delete obj_val_ctr;
      obj_val_ctr = NULL;
      continue;
    }
    //  getting commit version values from DT_RUNNING
    vector<string> vect_controller_id;
    vector<val_ctr_commit_ver_t> vect_val_ctr_cv;
    uint32_t max_rep_ct = 1;
    val_ctr_commit_ver_t ctr_val_struct;
    memset(&ctr_val_struct, 0, sizeof(val_ctr_commit_ver_t));
    UncRespCode read_status = kt_ctr.ReadCtrCommitVerFromDB(db_conn,
                          reinterpret_cast<void*>(ctr_key),
                          NULL,
                          UNC_DT_RUNNING, UNC_OP_READ, max_rep_ct,
                          vect_val_ctr_cv, vect_controller_id);
    if (read_status != UNC_RC_SUCCESS) {
      pfc_log_error("Error in read commit version from DB");
      ctr_val_struct.valid[kIdxCtrCommitNumber] = UNC_VF_VALID;
      ctr_val_struct.valid[kIdxCtrCommitDate] = UNC_VF_VALID;
      ctr_val_struct.valid[kIdxCtrCommitApplication] = UNC_VF_VALID;
    } else {
      ctr_val_struct =  vect_val_ctr_cv[0];
    }
    memcpy(&ctr_val_struct.controller, &obj_val_ctr->controller,
            sizeof(obj_val_ctr->controller));
    ctr_val_struct.valid[kIdxCtrVal] = UNC_VF_VALID;
    string domain_id = "";
    driver_request_header rqh = {0, 0, controller_name, domain_id,
        UNC_OP_CREATE, 0, (uint32_t)0, (uint32_t)0,
        UNC_DT_RUNNING, UNC_KT_CONTROLLER};
    int err = PhyUtil::sessOutDriverReqHeader(*cli_session, rqh);
    err |= cli_session->addOutput(*ctr_key);
    err |= cli_session->addOutput(ctr_val_struct);
    pfc_log_info("KEY:%s",
                   IpctUtil::get_string(*ctr_key).c_str());
    pfc_log_info("VAL:%s",
                  IpctUtil::get_string(ctr_val_struct).c_str());
    if (err != UNC_RC_SUCCESS) {
      pfc_log_error("Could not add objects to driver ipc session");
      // Release memory allocated for key struct
      delete ctr_key;
      ctr_key = NULL;
      // delete the val memory
      delete obj_val_ctr;
      obj_val_ctr = NULL;
      continue;
    }
    // Create a task queue to process particular controller's events
    PhyEventTaskqUtil *taskq_util = NotificationManager::get_taskq_util();
    UncRespCode status = taskq_util->create_task_queue(controller_name);
    if (status != UNC_RC_SUCCESS)
      UPPL_LOG_FATAL("Event Handling taskq creation error !!");
    // Inserting the lock object in the map for the particular controller
    CtrOprnStatus oCtrOprnStatus;
    oCtrOprnStatus.rwlock_oper_st = new ReadWriteLock();
    PhysicalLayer::ctr_oprn_mutex_.lock();
    PhysicalLayer::ctr_oprn_status_.insert(std::pair<string, CtrOprnStatus>
                                         (controller_name, oCtrOprnStatus));
    PhysicalLayer::ctr_oprn_mutex_.unlock();
    pfc_log_info("Sending connect request to driver");
    // Send the request to driver
    UncRespCode driver_response = UNC_RC_SUCCESS;
    driver_response_header rsp;
    if (controller_type == UNC_CT_PFC) {
      driver_response = pfc_drv_handler.SendReqAndGetResp(rsp);
    } else if (controller_type == UNC_CT_VNP) {
      driver_response = vnp_drv_handler.SendReqAndGetResp(rsp);
    } else if (controller_type == UNC_CT_POLC) {
      driver_response = polc_drv_handler.SendReqAndGetResp(rsp);
    } else if (controller_type == UNC_CT_ODC) {
      driver_response = odc_drv_handler.SendReqAndGetResp(rsp);
    }  // else part will not occur. It is already eliminated above...
    pfc_log_debug("driver_response is  %d", driver_response);
    if (driver_response != UNC_RC_SUCCESS) {
      pfc_log_error(
          "Could not connect to controller %s, driver returned error %d",
          controller_name.c_str(), driver_response);
    }
    // Release memory allocated for key struct
    delete ctr_key;
    ctr_key = NULL;
    // delete the val memory
    delete obj_val_ctr;
    obj_val_ctr = NULL;
  }
  // Sending the Controller Candidate Info to Logical Layer
  SendCandidateInfoToLogical(db_conn);
  pfc_log_info("SystemStateChangeToActive returned SUCCESS");
  return UNC_RC_SUCCESS;
}

/**SendCandidateInfoToLogical
 * @Description : This function gets the controller list from candidate database
 *                and send the update request to logical
 * @param[in]   : None
 * @return      : UNC_RC_SUCCESS if the logical is updated
 *                or UNC_UPPL_RC_ERR_* if the update fails
 * */
UncRespCode SystemStateChangeRequest::SendCandidateInfoToLogical(
    OdbcmConnectionHandler *db_conn) {
  /* Get all the controller entry from candidate db */
  Kt_Controller kt_ctr;
  vector<void *> vect_ctr_key, vect_ctr_val;
  key_ctr_t key_ctr_obj;
  memset(&key_ctr_obj, '\0', sizeof(key_ctr_t));
  vect_ctr_key.push_back(reinterpret_cast<void *>(&key_ctr_obj));
  UncRespCode read_status = kt_ctr.ReadInternal(db_conn, vect_ctr_key,
                                                   vect_ctr_val,
                                                   UNC_DT_CANDIDATE,
                                                   UNC_OP_READ_SIBLING_BEGIN);
  if (read_status != UNC_RC_SUCCESS) {
    pfc_log_info("read from candidate db is %d", read_status);
    return UNC_RC_SUCCESS;
  }

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
        UNC_DT_CANDIDATE,
        UNC_OP_CREATE,
        UNC_KT_CONTROLLER,
        vect_ctr_key[ctrIndex],
        reinterpret_cast<void*>(&obj_val_ctr->controller));
    if (upll_result != UNC_RC_SUCCESS) {
      pfc_log_info("Failed to send the controller %s in candidate to UPLL",
                   controller_name.c_str());
    }
    // Release memory allocated for key struct
    delete ctr_key;
    ctr_key = NULL;
    // delete the val memory
    delete obj_val_ctr;
    obj_val_ctr = NULL;
  }
  return UNC_RC_SUCCESS;
}
}  // namespace uppl
}  // namespace unc
