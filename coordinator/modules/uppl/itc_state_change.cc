/*
 * Copyright (c) 2012-2013 NEC Corporation
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

namespace unc {
namespace uppl {

/*SystemStateChangeRequest()
 * @Description : SystemStateChangeRequest constructor
 * @param[in]   : None
 * @return      : void
 */
SystemStateChangeRequest::SystemStateChangeRequest() {
}

/*~SystemStateChangeRequest()
 * @Description : SystemStateChangeRequest destructor
 * @param[in]   : None
 * @return      : void
 */
SystemStateChangeRequest::~SystemStateChangeRequest() {
}

/** SystemStateChangeToStandBy()
 * @Description : Active state to Standby
 * @param[in]   : none
 * @return      : Success or associated error code
 * */
UpplReturnCode SystemStateChangeRequest::SystemStateChangeToStandBy() {
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  PhysicalCore* physical_core_ = physical_layer->get_physical_core();
  Kt_Controller kt_ctr;
  UpplReturnCode read_status;
  UpplReturnCode result_code = UPPL_RC_SUCCESS;
  unc_keytype_ctrtype_t controller_type = UNC_CT_PFC;

  vector<string> vec_controller_name;
  // Calling the function GetControllerListFromDB
  result_code = GetControllerListFromDb(UNC_DT_RUNNING, vec_controller_name);

  if (result_code != UPPL_RC_SUCCESS) {
    pfc_log_info("Getting controller list failed with %d", result_code);
    return UPPL_RC_SUCCESS;
  }

  key_ctr_t obj_key_ctr;
  int err = 0;
  IPCClientDriverHandler pfc_drv_handler(UNC_CT_PFC, err);
  IPCClientDriverHandler vnp_drv_handler(UNC_CT_VNP, err);
  memset(obj_key_ctr.controller_name,
         '\0',
         sizeof(obj_key_ctr.controller_name));
  // Filling the key structure of KT_CONTROLLER
  for (uint32_t ctrIndex = 0; ctrIndex < vec_controller_name.size();
      ctrIndex ++) {
    memcpy(obj_key_ctr.controller_name, vec_controller_name[ctrIndex].c_str(),
           vec_controller_name[ctrIndex].length()+1);
    string controller_name = vec_controller_name[ctrIndex].c_str();
    pfc_log_info("vec_controller_name  %s",
                 controller_name.c_str());
    /* Calling the ReadInternal function of KT_controller to get the
       val structure of related controller */
    vector<void *> vect_ctr_key, vect_ctr_val;
    vect_ctr_key.push_back(reinterpret_cast<void*>(&obj_key_ctr));
    read_status = kt_ctr.ReadInternal(vect_ctr_key, vect_ctr_val,
                                      (unc_keytype_datatype_t)UNC_DT_RUNNING,
                                      UNC_OP_READ);
    if (read_status != UPPL_RC_SUCCESS) {
      pfc_log_debug("read_status is %d", read_status);
      pfc_log_error("Could not get details for controller %s",
                    controller_name.c_str());
    } else {
      val_ctr_st_t obj_val_ctr =
          *(reinterpret_cast<val_ctr_st_t*>(&vect_ctr_val[0]));
      controller_type =
          (unc_keytype_ctrtype_t)
          (PhyUtil::uint8touint(obj_val_ctr.controller.type));
      // Sending the Delete Request to Driver through IPC Framework
      //  construct IPC structure with DELETE  operation
      ClientSession *cli_session = NULL;
      if (controller_type == UNC_CT_PFC) {
        pfc_log_info("PFC Controller Type");
        cli_session = pfc_drv_handler.ResetAndGetSession();
      } else if (controller_type == UNC_CT_VNP) {
        pfc_log_info("VNP Controller Type");
        cli_session = vnp_drv_handler.ResetAndGetSession();
      } else {
        pfc_log_info("DRIVER SUPPORT NOT ADDED YET FOR"
            " UNKNOWN type");
        continue;
      }
      string domain_id;
      driver_request_header rqh = {0, 0, controller_name, domain_id,
          UNC_OP_DELETE, 0, (uint32_t)0, (uint32_t)0,
          UNC_DT_RUNNING, UNC_KT_CONTROLLER};
      err = PhyUtil::sessOutDriverReqHeader(*cli_session, rqh);
      err |= cli_session->addOutput(obj_key_ctr);
      if (err != UPPL_RC_SUCCESS) {
        pfc_log_info("Could not open driver ipc session");
        continue;
      }
      // Send the request to driver
      UpplReturnCode driver_response = UPPL_RC_SUCCESS;
      driver_response_header rsp;
      if (controller_type == UNC_CT_PFC) {
        driver_response = pfc_drv_handler.SendReqAndGetResp(rsp);
      }
      if (controller_type == UNC_CT_VNP) {
        driver_response = vnp_drv_handler.SendReqAndGetResp(rsp);
      }

      pfc_log_info("driver_response is  %d", driver_response);
      if (driver_response != UPPL_RC_SUCCESS) {
        pfc_log_info(
            "Controller disconnect request failed at "
            "driver with error %d", driver_response);
      }
      // Release memory allocated for key struct
      key_ctr_t *ctr_key = reinterpret_cast<key_ctr_t*>(vect_ctr_key[0]);
      val_ctr_st_t *ctr_val = reinterpret_cast<val_ctr_st_t*>(vect_ctr_val[0]);
      if (ctr_key != NULL) {
        delete ctr_key;
        ctr_key = NULL;
      }
      // delete the val memory
      if (ctr_val != NULL) {
        delete ctr_val;
        ctr_val = NULL;
      }
    }
  }
  physical_core_->set_system_state(UPPL_SYSTEM_ST_STANDBY);
  pfc_log_info("SystemStateChangeToStandby returned");
  return result_code;
}


/** GetControllerListFromDb()
 * @Description : Gets the controller list from the database
 * @param[in]   : none
 * @return      : Success or associated error code
 * */
UpplReturnCode SystemStateChangeRequest::GetControllerListFromDb
(uint32_t data_type, vector<string> &vec_controller_name) {
  UpplReturnCode result_code = UPPL_RC_SUCCESS;
  /* Structure used to send request to ODBC */
  DBTableSchema dbtableschema_obj;
  /* Construct Primary key list */
  vector<string> vect_prim_keys;
  vect_prim_keys.push_back("controller_name");
  /* construct TableAttrSchema structure
     TableAttrSchema holds table_name, primary key, attr_name */
  vector<TableAttrSchema> vect_table_attr_schema;
  TableAttrSchema  table_attr_schema_obj;
  list< vector<TableAttrSchema> > row_list;
  dbtableschema_obj.set_table_name("controller_table");
  dbtableschema_obj.set_primary_keys(vect_prim_keys);

  string controller_name;
  /* controller_name */
  PhyUtil::FillDbSchema("controller_name", controller_name,
                        controller_name.length(), DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);

  dbtableschema_obj.PushBackToRowList(vect_table_attr_schema);
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  string table_name = "controller_table";

  ODBCM_RC_STATUS db_status = physical_layer->get_odbc_manager()->
      GetBulkRows((unc_keytype_datatype_t)data_type, UPPL_MAX_REP_CT,
                  dbtableschema_obj, UNC_OP_READ_SIBLING);
  if (db_status != ODBCM_RC_SUCCESS) {
    if (db_status == ODBCM_RC_CONNECTION_ERROR) {
      /* log fatal error to log daemon */
      pfc_log_fatal(
          "DB connection not available or cannot access DB, error=%d",
          db_status);
      result_code = UPPL_RC_ERR_DB_ACCESS;
    } else {
      /* log error to log daemon */
      pfc_log_info("GetControllerListFromDb :DB error %d", db_status);
      result_code = UPPL_RC_ERR_DB_GET;
    }
    return result_code;
  }
  pfc_log_debug("Traversing the list");
  // To traverse the list
  list<vector<TableAttrSchema> > ::iterator iter_list;
  vector<TableAttrSchema>  :: iterator iter_vector;  // To traverse the vector
  unsigned int list_of_row, attr_vector;
  for (list_of_row = 0, iter_list = dbtableschema_obj.row_list_.begin();
      iter_list != dbtableschema_obj.row_list_.end();
      iter_list++, list_of_row++) {
    pfc_log_debug("Traversing the  dbtableschema_obj.row_list");
    /*  This vector contains all attributes of a row in a table */
    vector<TableAttrSchema> attributes_vector = *iter_list;
    TableAttrSchema attribute;
    /* Get the column names  and values */
    for (attr_vector = 0, iter_vector = attributes_vector.begin();
        iter_vector != attributes_vector.end();
        attr_vector++, iter_vector++) {
      /* Get attribute name of a row */
      pfc_log_debug("Traversing the  Attributes vector");
      attribute = (*iter_vector);
      pfc_log_debug("Getting the attribute:  %s",
                    attribute.table_attribute_name.c_str());
      if (attribute.table_attribute_name == "controller_name") {
        string *pattr_value =
            reinterpret_cast<string*>(&attribute.p_table_attribute_value);
        string controller_name = *pattr_value;
        pfc_log_info("controller_name:  %s", controller_name.c_str());
        vec_controller_name.push_back(controller_name.c_str());
      }
    }
  }
  pfc_log_info("GetContrllerListFrom db result code is %d",  result_code);
  return result_code;
}

/** SystemStateChangeToStandBy()
 * @Description : Standby state to Active
 * @param[in]   : none
 * @return      : Success or associated error code
 * */
UpplReturnCode SystemStateChangeRequest::SystemStateChangeToActive() {
  Kt_Controller kt_ctr;
  UpplReturnCode read_status, operation_status;
  UpplReturnCode controller_status;
  UpplReturnCode result_code = UPPL_RC_SUCCESS;
  uint8_t oper_status;
  unc_keytype_ctrtype_t controller_type;

  pfc_log_info("Start SystemStateChangeToActive");

  vector<string> vec_controller_name;
  /* Calling the function GetControllerListFromDB */
  controller_status = GetControllerListFromDb(UNC_DT_RUNNING,
                                              vec_controller_name);
  if (controller_status != UPPL_RC_SUCCESS) {
    pfc_log_info("Getting controller list failed with %d", controller_status);
    return UPPL_RC_SUCCESS;
  }

  key_ctr_t obj_key_ctr;
  int err = 0;
  IPCClientDriverHandler pfc_drv_handler(UNC_CT_PFC, err);
  IPCClientDriverHandler vnp_drv_handler(UNC_CT_VNP, err);

  /* Filling the key structure of KT_CONTROLLER */
  for (uint32_t ctrIndex = 0; ctrIndex < vec_controller_name.size();
      ctrIndex ++) {
    memset(obj_key_ctr.controller_name, '\0',
           sizeof(obj_key_ctr.controller_name));
    memcpy(obj_key_ctr.controller_name, vec_controller_name[ctrIndex].c_str(),
           vec_controller_name[ctrIndex].length()+1);
    pfc_log_info("obj_key_ctr.controller_name  %s",
                 obj_key_ctr.controller_name);
    pfc_log_info("vec_controller_name  %s",
                 vec_controller_name[ctrIndex].c_str());

    /* Getting the Oper Status of the controller*/
    pfc_log_info("Set the oper Status of controller as DOWN");
    oper_status = UPPL_CONTROLLER_OPER_DOWN;
    operation_status = kt_ctr.SetOperStatus(UNC_DT_RUNNING,
                                            &obj_key_ctr, oper_status);
    if (operation_status != UPPL_RC_SUCCESS) {
      pfc_log_info("Unable to set the oper status of controller as DOWN");
    }
    /* Calling the ReadInternal function of KT_controller to get the
       val structure of related controller */
    vector<void *> vect_ctr_key, vect_ctr_val;
    vect_ctr_key.push_back(reinterpret_cast<void*>(&obj_key_ctr));
    read_status = kt_ctr.ReadInternal(vect_ctr_key, vect_ctr_val,
                                      UNC_DT_RUNNING,
                                      UNC_OP_READ);
    string controller_name = vec_controller_name[ctrIndex].c_str();
    if (read_status != UPPL_RC_SUCCESS) {
      pfc_log_info("read_status is %d", read_status);
      pfc_log_error("Read operation has failed");
      read_status = UPPL_RC_ERR_DB_ACCESS;
      return read_status;
    } else {
      val_ctr_st_t obj_val_ctr =
          *(reinterpret_cast<val_ctr_st_t*>(vect_ctr_val[0]));
      controller_type =
          (unc_keytype_ctrtype_t)
          (PhyUtil::uint8touint(obj_val_ctr.controller.type));

      // Sending the Controller Update Information to Logical Layer
      UpplReturnCode upll_result = kt_ctr.SendUpdatedControllerInfoToUPLL(
          UNC_DT_CANDIDATE,
          UNC_OP_CREATE,
          UNC_KT_CONTROLLER,
          &obj_key_ctr,
          &obj_val_ctr.controller);
      if (upll_result != UPPL_RC_SUCCESS) {
        pfc_log_info("Failed to send the info to UPLL of controller  %s",
                     controller_name.c_str());
      }
      upll_result = kt_ctr.SendUpdatedControllerInfoToUPLL(
          UNC_DT_RUNNING,
          UNC_OP_CREATE,
          UNC_KT_CONTROLLER,
          &obj_key_ctr,
          &obj_val_ctr.controller);
      if (upll_result != UPPL_RC_SUCCESS) {
        pfc_log_info("Failed to send the info to UPLL of controller  %s",
                     controller_name.c_str());
      }
      /* Sending the Create Request to Driver through IPC Framework
       construct IPC structure with CREATE  operation */
      ClientSession *cli_session = NULL;
      if (controller_type == UNC_CT_PFC) {
        pfc_log_info("PFC Controller Type");
        cli_session = pfc_drv_handler.ResetAndGetSession();
      } else if (controller_type == UNC_CT_VNP) {
        pfc_log_info("VNP Controller Type");
        cli_session = vnp_drv_handler.ResetAndGetSession();
      } else {
        pfc_log_info("DRIVER SUPPORT NOT ADDED YET FOR UNKNOWN type");
        continue;
      }
      string domain_id;
      driver_request_header rqh = {0, 0, controller_name, domain_id,
          UNC_OP_CREATE, 0, (uint32_t)0, (uint32_t)0,
          UNC_DT_RUNNING, UNC_KT_CONTROLLER};
      int err = PhyUtil::sessOutDriverReqHeader(*cli_session, rqh);
      err |= cli_session->addOutput(obj_key_ctr);
      err |= cli_session->addOutput(obj_val_ctr.controller);
      if (err != UPPL_RC_SUCCESS) {
        pfc_log_error("Could not add objects to driver ipc session");
        continue;
      }
      pfc_log_info("Sending connect request to driver");
      // Send the request to driver
      UpplReturnCode driver_response = UPPL_RC_SUCCESS;
      driver_response_header rsp;
      if (controller_type == UNC_CT_PFC) {
        driver_response = pfc_drv_handler.SendReqAndGetResp(rsp);
      }
      if (controller_type == UNC_CT_VNP) {
        driver_response = vnp_drv_handler.SendReqAndGetResp(rsp);
      }

      pfc_log_info("driver_response is  %d", driver_response);
      if (err !=0 || driver_response != UPPL_RC_SUCCESS) {
        pfc_log_error(
            "Could not connect to controller %s, driver returned error %d",
            controller_name.c_str(), driver_response);
      }
      // Release memory allocated for key struct
      key_ctr_t *ctr_key = reinterpret_cast<key_ctr_t*>(vect_ctr_key[0]);
      val_ctr_st_t *ctr_val = reinterpret_cast<val_ctr_st_t*>(vect_ctr_val[0]);
      if (ctr_key != NULL) {
        delete ctr_key;
        ctr_key = NULL;
      }
      // delete the val memory
      if (ctr_val != NULL) {
        delete ctr_val;
        ctr_val = NULL;
      }
    }
  }
  pfc_log_info("SystemStateChangeToActive returned %d", result_code);
  return result_code;
}
}  // namespace uppl
}  // namespace unc
