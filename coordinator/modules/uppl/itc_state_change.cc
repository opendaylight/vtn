/*
 * Copyright (c) 2012-2014 NEC Corporation
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
    return err;
  }
  err = UNC_RC_SUCCESS;
  IPCClientDriverHandler vnp_drv_handler(UNC_CT_VNP, err);
  if (err != UNC_RC_SUCCESS) {
    pfc_log_error("Cannot open session to VNP driver");
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
    string domain_id = "";
    driver_request_header rqh = {0, 0, controller_name, domain_id,
        UNC_OP_DELETE, 0, (uint32_t)0, (uint32_t)0,
        UNC_DT_RUNNING, UNC_KT_CONTROLLER};
    int err = PhyUtil::sessOutDriverReqHeader(*cli_session, rqh);
    err |= cli_session->addOutput(*ctr_key);
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
    }
    if (controller_type == UNC_CT_VNP) {
      driver_response = vnp_drv_handler.SendReqAndGetResp(rsp);
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
  }
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  PhysicalCore* physical_core = physical_layer->get_physical_core();
  physical_core->set_system_state(UPPL_SYSTEM_ST_STANDBY);
  // To clear the internal alarm details holding map
  physical_core->alarm_status_map_.clear();

  pfc_log_info("SystemStateChangeToStandby returned SUCCESS");
  return UNC_RC_SUCCESS;
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
    pfc_log_error("Cannot open session to PFC driver");
    return err;
  }
  err = UNC_RC_SUCCESS;
  IPCClientDriverHandler vnp_drv_handler(UNC_CT_VNP, err);
  if (err != UNC_RC_SUCCESS) {
    pfc_log_error("Cannot open session to VNP driver");
    return err;
  }
  pfc_log_debug("Setting system_transit_state_ to be true:%d",
                physical_core->system_transit_state_);
  physical_core->system_transit_state_ = true;  // for sending recovery
                                          //  alarm to node manager.
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
    std::string alarm_category = "1";
    std::string map_key = "";
    map_key.append(controller_name).append("#").append(alarm_category);
    physical_core->alarm_status_map_.insert(
      std::pair<std::string, bool> (map_key, true));
    alarm_category = "3";
    map_key = "";
    map_key.append(controller_name).append("#").append(alarm_category);
    physical_core->alarm_status_map_.insert(
      std::pair<std::string, bool> (map_key, true));
    if (controller_type != UNC_CT_UNKNOWN) {
      pfc_log_info("Set the oper Status of controller as down %s",
                   controller_name.c_str());
      uint8_t oper_status = UPPL_CONTROLLER_OPER_DOWN;
      UncRespCode operation_status = kt_ctr.SetOperStatus(
          db_conn,
          UNC_DT_RUNNING,
          vect_ctr_key[ctrIndex],
          oper_status);
      if (operation_status != UNC_RC_SUCCESS) {
        pfc_log_error("Unable to set the oper status of controller as down");
      }
    }
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
    upll_result = kt_ctr.SendUpdatedControllerInfoToUPLL(
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
    string domain_id = "";
    driver_request_header rqh = {0, 0, controller_name, domain_id,
        UNC_OP_CREATE, 0, (uint32_t)0, (uint32_t)0,
        UNC_DT_RUNNING, UNC_KT_CONTROLLER};
    int err = PhyUtil::sessOutDriverReqHeader(*cli_session, rqh);
    err |= cli_session->addOutput(*ctr_key);
    err |= cli_session->addOutput(obj_val_ctr->controller);
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
    pfc_log_info("Sending connect request to driver");
    // Send the request to driver
    UncRespCode driver_response = UNC_RC_SUCCESS;
    driver_response_header rsp;
    if (controller_type == UNC_CT_PFC) {
      driver_response = pfc_drv_handler.SendReqAndGetResp(rsp);
    }
    if (controller_type == UNC_CT_VNP) {
      driver_response = vnp_drv_handler.SendReqAndGetResp(rsp);
    }

    pfc_log_debug("driver_response is  %d", driver_response);
    if (err !=0 || driver_response != UNC_RC_SUCCESS) {
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
  pfc_log_info("SystemStateChangeToActive returned SUCCESS");
  physical_core->system_transit_state_ = false;
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
