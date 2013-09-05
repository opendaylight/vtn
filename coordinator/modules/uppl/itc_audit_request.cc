/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/**
 *  @brief    Audit Request handling
 *  @file     itc_audit_request.cc
 */

#include <vector>
#include "physicallayer.hh"
#include "physical_itc.hh"
#include "itc_audit_request.hh"
#include "itc_kt_controller.hh"
#include "itc_kt_ctr_domain.hh"
#include "itc_kt_logicalport.hh"
#include "itc_kt_logical_member_port.hh"
#include "itc_kt_switch.hh"
#include "itc_kt_port.hh"
#include "itc_kt_link.hh"
#include "itc_kt_boundary.hh"
#include "ipc_client_configuration_handler.hh"

namespace unc {
namespace uppl {

/**AuditRequest
 * @Description : This function initializes the member data
 * @param[in]   : None
 * @return      : None
 * */
AuditRequest::AuditRequest() {
}

/**~AuditRequest
 * @Description : This function releases the  memory allocated to
 *                pointer member data
 * @param[in]   : None
 * @return      : None
 * */
AuditRequest::~AuditRequest() {
}

/**StartAudit
 * @Description : This function performs the required pre condition validations
 *                such as checking of the existence of the controller and its
 *                operation status and set its value again.
 * @param[in]   : driver_id - Specifies one of the following controller type,
 *                PFC,VNP,Bypass    
 *                controller_id - controller name to which the audit to start
 * @return      : UPPL_RC_SUCCESS if the Audit is success for the controller 
 *                or UPPL_RC_ERR_*
 * */

UpplReturnCode AuditRequest::StartAudit(OdbcmConnectionHandler *db_conn,
                                        unc_keytype_ctrtype_t driver_id,
                                        string controller_id) {
  pfc_log_info("Processing StartAudit");
  Kt_Controller KtObj;
  uint8_t oper_status = 0;
  key_ctr_t obj_key_ctr;
  memset(obj_key_ctr.controller_name,
         '\0',
         sizeof(obj_key_ctr.controller_name));
  memcpy(obj_key_ctr.controller_name,
         controller_id.c_str(),
         controller_id.length()+1);

  /* Checks controller existence and its oper status */
  pfc_log_debug("Get controller oper Status");
  val_ctr_st_t obj_val_ctr_st;
  UpplReturnCode read_status =
      KtObj.GetOperStatus(db_conn, UNC_DT_RUNNING,
                          reinterpret_cast<void *>(&obj_key_ctr), oper_status);
  if (read_status == UPPL_RC_SUCCESS) {
    // Checking the audit precondition
    if (oper_status == UPPL_CONTROLLER_OPER_DOWN ||
        oper_status == UPPL_CONTROLLER_OPER_UP ||
        oper_status == UPPL_CONTROLLER_OPER_WAITING_AUDIT) {
      obj_val_ctr_st.oper_status = UPPL_CONTROLLER_OPER_AUDITING;
      memset(obj_val_ctr_st.valid, '\0', sizeof(obj_val_ctr_st.valid));
      obj_val_ctr_st.valid[kIdxOperStatus] = 1;
      FN_START_TIME("Audit::HandleOperStatus", "Controller");
      UpplReturnCode handle_oper_status = KtObj.HandleOperStatus(
          db_conn, UNC_DT_RUNNING, reinterpret_cast<void *>(&obj_key_ctr),
          reinterpret_cast<void *>(&obj_val_ctr_st),
          true);
      pfc_log_debug("Handle Oper Status return: %d", handle_oper_status);
      FN_END_TIME("Audit::HandleOperStatus", "Controller");
      return UPPL_RC_SUCCESS;
    } else {
      // return UPPL_RC_ERR_AUDIT_FAILURE;
    }
  } else {
    pfc_log_error("Unable to get controller oper status");
    return UPPL_RC_ERR_AUDIT_FAILURE;
  }

  return UPPL_RC_SUCCESS;
}

/**StartAuditTransaction
 * @Description : This function is invoked when TC sends AuditTransactionStart
 *                to Physical Core
 * @param[in]   : session_id - ipc session id used for TC validation
 *                driver_id - Specifies one of the following controller type-
 *                PFC,VNP,Bypass
 *                controller_id - controller name in which the audit to start
 * @return      : UPPL_RC_SUCCESS if the AuditTransaction is success for the
 *                controller or UPPL_RC_ERR_* for audit transaction failure
 * */
UpplReturnCode AuditRequest::StartAuditTransaction(
    uint32_t session_id,
    unc_keytype_ctrtype_t driver_id,
    string controller_id)  {
  pfc_log_info("Returning success for StartAuditTransaction");
  return UPPL_RC_SUCCESS;
}

/**HandleAuditVoteRequest
 * @Description : This function is invoked when TC sends AuditVoteRequest to
 *                Physical Core and here,get the controller details for the
 *                controller where the audit occurs
 * @param[in]   : session_id - ipc session id used for TC validation
 *                driver_id - Specifies one of the following controller type-
 *                PFC,VNP,Bypass
 *                controller_id - controller name in which the audit occurs
 * @param[out]  : driver_info - contains the controller list
 * @return      : UPPL_RC_SUCCESS if the AuditVoteRequest is success or 
 *                UPPL_RC_ERR_* if AuditVoteRequest is failed.
 * */
UpplReturnCode AuditRequest::HandleAuditVoteRequest(
    OdbcmConnectionHandler *db_conn,
    uint32_t session_id,
    uint32_t driver_id,
    string controller_id,
    TcDriverInfoMap &driver_info) {
  pfc_log_info("Processing HandleAuditVoteRequest");
  Kt_Controller KtObj;
  std::vector<std::string> controllers;
  controllers.push_back(controller_id);
  key_ctr_t obj_key_ctr;
  memset(obj_key_ctr.controller_name,
         '\0',
         sizeof(obj_key_ctr.controller_name));
  memcpy(obj_key_ctr.controller_name,
         controller_id.c_str(),
         controller_id.length()+1);
  vector<void *> vect_ctr_key, vect_ctr_val;
  vect_ctr_key.push_back(reinterpret_cast<void *>(&obj_key_ctr));
  UpplReturnCode read_status = KtObj.ReadInternal(
      db_conn, vect_ctr_key, vect_ctr_val,
      (unc_keytype_datatype_t)UNC_DT_RUNNING,
      UNC_OP_READ);
  if (read_status != UPPL_RC_SUCCESS) {
    pfc_log_error("Could not get details for controller %s",
                  controller_id.c_str());
    return UPPL_RC_ERR_AUDIT_FAILURE;
  }
  pfc_log_debug("Read operation is success");
  val_ctr_st_t *obj_val_ctr =
      (reinterpret_cast <val_ctr_st_t*>(vect_ctr_val[0]));
  if (obj_val_ctr != NULL) {
    unc_keytype_ctrtype_t controller_type =
        (unc_keytype_ctrtype_t)
        (PhyUtil::uint8touint(obj_val_ctr->controller.type));
    pfc_log_debug("Controller Type is %d", controller_type);
    if (controller_type != UNC_CT_PFC &&
        controller_type != UNC_CT_VNP) {
      pfc_log_debug("Unsupported controller type - ignoring ");
    } else {
      /* PHYSICAL SHOULD NOT SEND UPDATED CONTROLLER LIST
        driver_info.insert(std::pair<unc_keytype_ctrtype_t,
                           std::vector<std::string > >
        (controller_type, controllers));
       */
    }
    // Release memory allocated for key struct
    key_ctr_t *ctr_key = reinterpret_cast<key_ctr_t*>(vect_ctr_key[0]);
    if (ctr_key != NULL) {
      delete ctr_key;
      ctr_key = NULL;
    }
    // delete the val memory
    delete obj_val_ctr;
    obj_val_ctr = NULL;
  }
  return UPPL_RC_SUCCESS;
}

/**HandleAuditGlobalCommit
 * @Description : This function is invoked when TC sends AuditGlobalCommit
 *                to Physical Core
 * @param[in]   : session_id - ipc session id used for TC validation
 *                driver_id - Specifies one of the following controller type-
 *                PFC,VNP,Bypass
 *                controller_id - controller name in which the audit occurs
 *                driver_info - contains the controller list
 * @param[out]  : audit result - specifies the TC audit result 
 * @return      : UPPL_RC_SUCCESS if the AuditGlobalCommit is success for the
 *                controller or UPPL_RC_ERR_* for the failure
 * */

UpplReturnCode AuditRequest::HandleAuditGlobalCommit(
    uint32_t session_id,
    uint32_t driver_id,
    string controller_id,
    TcDriverInfoMap& driver_info,
    TcAuditResult& audit_result) {
  pfc_log_info("Returing Success for HandleAuditGlobalCommit");
  audit_result = unc::tclib::TC_AUDIT_SUCCESS;
  return UPPL_RC_SUCCESS;
}


/**HandleAuditAbort
 * @Description : This function is invoked when TC sends AuditGlobalAbort
 *                to Physical Core
 * @param[in]   : session_id - ipc session id used for TC validation
 *                driver_id - Specifies one of the following controller type-
 *                PFC,VNP,Bypass
 *                controller_id - controller name in which the audit occurs
 *                operation_phase - 
 * @return      : UPPL_RC_SUCCESS if the AuditGlobalAbort is success for the
 *                controller or UPPL_RC_ERR_* for the failure 
 * */

UpplReturnCode AuditRequest::HandleAuditAbort(
    uint32_t session_id,
    unc_keytype_ctrtype_t driver_id,
    string controller_id,
    TcAuditOpAbortPhase operation_phase) {
  pfc_log_info("Returing success for HandleAuditAbort");
  return UPPL_RC_SUCCESS;
}

/**EndAuditTransaction
 * @Description : This function is used to end the Audit Transaction
 * @param[in]   : session_id - ipc session id used for TC validation
 *                driver_id - Specifies one of the following controller type-
 *                PFC,VNP,Bypass
 *                controller_id - controller name in which the audit occurs 
 * @return      : UPPL_RC_SUCCESS if the EndAuditTransaction is success for the
 *                controller or UPPL_RC_ERR_* for the failure
 * */
UpplReturnCode AuditRequest::EndAuditTransaction(
    uint32_t session_id,
    unc_keytype_ctrtype_t& drivertype,
    string controller_id) {
  pfc_log_info("Returning success for EndAuditTransaction");
  return UPPL_RC_SUCCESS;
}

/**EndAudit
 * @Description : This function is used to end the Audit and adds the
 *                controller to audit list
 * @param[in]   : driver_id - Specifies one of the following controller type-
 *                PFC,VNP,Bypass
 *                controller_id - controller name in which the audit occurs
 *                audit result - specifies the TC audit result
 * @return      : UPPL_RC_SUCCESS if the End Audit is success for the controller
 *                or returns UPPL_RC_ERR_ if the Audit is failed
 * */
UpplReturnCode AuditRequest::EndAudit(OdbcmConnectionHandler *db_conn,
                                      unc_keytype_ctrtype_t driver_id,
                                      string controller_id,
                                      TcAuditResult audit_result) {
  pfc_log_info("Processing HandleEndAudit");
  uint8_t oper_status = 0;
  Kt_Controller kt_controller;
  key_ctr_t key_ctr_obj;
  memset(&key_ctr_obj, '\0', sizeof(key_ctr_t));
  memcpy(key_ctr_obj.controller_name,
         controller_id.c_str(), controller_id.length()+1);
  void *key_ctr_prt = reinterpret_cast<void *>(&key_ctr_obj);
  val_ctr_st_t obj_val_ctr_st;
  memset(&obj_val_ctr_st, '\0', sizeof(val_ctr_st_t));
  pfc_log_debug("Audit result from TC %d", audit_result);
  if (audit_result == unc::tclib::TC_AUDIT_SUCCESS) {
    oper_status = UPPL_CONTROLLER_OPER_UP;
    obj_val_ctr_st.oper_status = oper_status;
    memset(obj_val_ctr_st.valid, '\0', sizeof(obj_val_ctr_st.valid));
    obj_val_ctr_st.valid[kIdxOperStatus] = 1;
    FN_START_TIME("Audit::HandleOperStatus", "Controller");
    UpplReturnCode handle_oper_status = kt_controller.HandleOperStatus(
        db_conn, UNC_DT_RUNNING,
        key_ctr_prt,
        reinterpret_cast<void *>(&obj_val_ctr_st),
        true);
    pfc_log_debug("Handle Oper Status return: %d", handle_oper_status);
    FN_END_TIME("Audit::HandleOperStatus", "Controller");
    // Add the controller to controller_in_audit vector
    IPCConnectionManager *ipc_mgr = PhysicalLayer::get_instance()->
        get_ipc_connection_manager();
    if (ipc_mgr != NULL) {
      pfc_log_debug("Adding controller to audit list %s",
                    controller_id.c_str());
      ipc_mgr->addControllerToAuditList(controller_id);
      // Start timer for processing audit notification from driver
      uint32_t ret = ipc_mgr->StartNotificationTimer(db_conn, controller_id);
      pfc_log_debug("Start Timer return code %d for controller %s",
                    ret, controller_id.c_str());
    } else {
      pfc_log_debug("IPC Connection Manager Object is NULL");
    }
  } else {
    oper_status = UPPL_CONTROLLER_OPER_WAITING_AUDIT;
    obj_val_ctr_st.oper_status = oper_status;
    memset(obj_val_ctr_st.valid, '\0', sizeof(obj_val_ctr_st.valid));
    obj_val_ctr_st.valid[kIdxOperStatus] = 1;
    UpplReturnCode handle_oper_status = kt_controller.HandleOperStatus(
        db_conn, UNC_DT_RUNNING,
        key_ctr_prt,
        reinterpret_cast<void *>(&obj_val_ctr_st),
        true);
    pfc_log_debug("Handle Oper Status return: %d", handle_oper_status);
  }
  return UPPL_RC_SUCCESS;
}

/**MergeAuditDbToRunning
 * @Description : This function is used to Merge audit and running db at
 *                end Audit and clear the import db.
 * @param[in]   : controller name - controller name in which the audit occurs
 * @return      : UPPL_RC_SUCCESS if the updation of running DB with the
 *                imported value is success or UPPL_RC_ERR_* for failure of 
 *                merging
 * */
UpplReturnCode AuditRequest::MergeAuditDbToRunning(
    OdbcmConnectionHandler *db_conn,
    string controller_name) {
  pfc_log_info("MergeAuditDbToRunning-controller_name: %s",
               controller_name.c_str());
  // Check for MergeImportRunning Lock
  ScopedReadWriteLock eventDoneLock(PhysicalLayer::get_events_done_lock_(),
                                    PFC_TRUE);  // write lock
  UpplReturnCode return_code = UPPL_RC_SUCCESS;
  // Remove the controller_name from controller_in_audit list
  return_code = PhysicalLayer::get_instance()->get_ipc_connection_manager()->
      removeControllerFromAuditList(controller_name);
  if (return_code != UPPL_RC_SUCCESS) {
    // Not required to do merge, return SUCCESS
    return UPPL_RC_SUCCESS;
  }
  for (unsigned int index = 0;
      index < STATE_OBJECTS; ++index) {
    void *key_struct = NULL;
    uint32_t key_type;
    Kt_Base *class_pointer = GetClassPointerAndKey(
        (AuditStateObjects)index, controller_name, key_struct, key_type);
    if (class_pointer == NULL) {
      if (key_struct != NULL) {
        FreeKeyAndValueStruct(key_struct, NULL, index);
      }
      continue;
    }
    vector<void*> key_running, key_import;
    key_running.push_back(key_struct);
    key_import.push_back(key_struct);
    vector<void*> val_running, val_import;
    pfc_log_debug("Reading from running db for index %d", index);
    UpplReturnCode read_running_status = class_pointer->ReadInternal(
        db_conn, key_running,
        val_running,
        (uint32_t) UNC_DT_RUNNING,
        (uint32_t) UNC_OP_READ_SIBLING_BEGIN);
    // Read from Import DB
    pfc_log_debug("Reading from import db for index %d", index);
    UpplReturnCode read_import_status = class_pointer->ReadInternal(
        db_conn, key_import,
        val_import,
        (uint32_t) UNC_DT_IMPORT,
        (uint32_t) UNC_OP_READ_SIBLING_BEGIN);
    if (read_running_status != UPPL_RC_SUCCESS &&
        read_import_status != UPPL_RC_SUCCESS) {
      pfc_log_debug(
          "Reading values from both import and running db failed for index %d",
          index);
      delete class_pointer;
      if (key_struct != NULL) {
        FreeKeyAndValueStruct(key_struct, NULL, index);
      }
      continue;
    }
    pfc_log_debug("No.of entries from running db %d",
                  static_cast<int>(key_running.size()));
    pfc_log_debug("No.of entries from import db %d",
                  static_cast<int>(key_import.size()));
    // Iterate RUNNING and compare with IMPORT
    // Update Running if values mismatch with Import
    // Delete entries from Running if not found in Import
    UpdateRunningDbWithImportDb(key_import, val_import,
                                key_running, val_running,
                                index, class_pointer,
                                db_conn, key_type);
    // Iterate IMPORT and compare with RUNNING
    // And create entries in running db if not present
    AddToRunningDbFromImportDb(key_import, val_import,
                               key_running, val_running,
                               index, class_pointer,
                               db_conn, key_type);
    // Free the readInternal key and value structures
    FreeKeyAndValueStruct(key_import, val_import,
                          key_running, val_running, index);
    delete class_pointer;
    if (key_struct != NULL) {
      FreeKeyAndValueStruct(key_struct, NULL, index);
    }
  }
  // Clear import db entries
  ODBCM_RC_STATUS clear_status =
      PhysicalLayer::get_instance()->get_odbc_manager()->
      ClearOneInstance(UNC_DT_IMPORT, controller_name, db_conn);
  if (clear_status != ODBCM_RC_SUCCESS) {
    pfc_log_info("Import DB clearing failed");
  }
  pfc_log_info("MergeAuditDbToRunning return code: %d", return_code);
  return return_code;
}

/**GetClassPointerAndKey
 * @Description : This function is used to get the key and class pointer
 *                of the appropriate kt_classes
 * @param[in]   : audit_key_type -
 *                controller name - controller name in which the audit occurs
 *                key - Specifies the key of the appropriate key types
 *                key_type - specifies the appropriate key types
 * @return      : Returns the class pointer to the appropriate kt classes.
 * */
Kt_Base* AuditRequest::GetClassPointerAndKey(
    AuditStateObjects audit_key_type,
    string controller_name,
    void* &key,
    uint32_t &key_type) {
  Kt_Base* class_pointer = NULL;
  switch (audit_key_type) {
    case Notfn_Ctr_Domain: {
      class_pointer = new Kt_Ctr_Domain();
      key_ctr_domain_t *obj_key = new key_ctr_domain_t;
      memcpy(obj_key->ctr_key.controller_name,
             controller_name.c_str(),
             controller_name.length()+1);
      memset(obj_key->domain_name, 0, 32);
      key = reinterpret_cast<void *>(obj_key);
      key_type = UNC_KT_CTR_DOMAIN;
      break;
    }
    case Notfn_Logical_Port: {
      class_pointer = new Kt_LogicalPort();
      key_logical_port_t *obj_key = new key_logical_port_t;
      memcpy(obj_key->domain_key.ctr_key.controller_name,
             controller_name.c_str(),
             controller_name.length()+1);
      memset(obj_key->domain_key.domain_name, 0, 32);
      memset(obj_key->port_id, 0, 320);
      key = reinterpret_cast<void *>(obj_key);
      key_type = UNC_KT_LOGICAL_PORT;
      break;
    }
    case Notfn_Logical_Member_Port: {
      class_pointer = new Kt_LogicalMemberPort();
      key_logical_member_port_t * obj_key = new key_logical_member_port_t;
      memcpy(obj_key->logical_port_key.domain_key.
             ctr_key.controller_name,
             controller_name.c_str(),
             controller_name.length()+1);
      memset(obj_key->logical_port_key.domain_key.domain_name,
             0, 32);
      memset(obj_key->logical_port_key.port_id,
             0, 320);
      memset(obj_key->physical_port_id, 0, 32);
      memset(obj_key->switch_id, 0, 256);
      key = reinterpret_cast<void *>(obj_key);
      key_type = UNC_KT_LOGICAL_MEMBER_PORT;
      break;
    }
    case Notfn_Switch: {
      class_pointer = new Kt_Switch();
      key_switch_t *obj_key = new key_switch_t;
      memcpy(obj_key->ctr_key.controller_name,
             controller_name.c_str(),
             controller_name.length()+1);
      memset(obj_key->switch_id, 0, 256);
      key = reinterpret_cast<void *>(obj_key);
      key_type = UNC_KT_SWITCH;
      break;
    }
    case Notfn_Port: {
      class_pointer = new Kt_Port();
      key_port_t *obj_key = new key_port_t;
      memcpy(obj_key->sw_key.ctr_key.controller_name,
             controller_name.c_str(),
             controller_name.length()+1);
      memset(obj_key->sw_key.switch_id, 0, 256);
      memset(obj_key->port_id, 0, 32);
      key = reinterpret_cast<void *>(obj_key);
      key_type = UNC_KT_PORT;
      break;
    }
    case Notfn_Link: {
      class_pointer = new Kt_Link();
      key_link_t *obj_key = new key_link_t;
      memcpy(obj_key->ctr_key.controller_name,
             controller_name.c_str(),
             controller_name.length()+1);
      memset(obj_key->switch_id1, 0, 256);
      memset(obj_key->switch_id2, 0, 256);
      memset(obj_key->port_id1, 0, 32);
      memset(obj_key->port_id2, 0, 32);
      key = reinterpret_cast<void *>(obj_key);
      key_type = UNC_KT_LINK;
      break;
    }
  }
  return class_pointer;
}

/**FreeKeyAndValueStruct
 * @Description : This function clears the key struct and value struct
 *                of the appropriate key types
 * @param[in]   : key struct - the key for the appropriate key types
 *                value struct - the value structure for the appropriate
 *                key types
 *                key type - UNC_KT_* specifies the appropriate key types
 * @return      : returns void * key structure for  the successfull deletion
 *                of key and value structures of the key types
 * */
void AuditRequest::FreeKeyAndValueStruct(void* key_struct,
                                         void* value_struct,
                                         unsigned int key_type) {
  switch (key_type) {
    case Notfn_Ctr_Domain: {
      key_ctr_domain_t *obj_key = reinterpret_cast<key_ctr_domain_t *>
      (key_struct);
      if (obj_key != NULL) {
        delete obj_key;
        obj_key = NULL;
      }
      val_ctr_domain_st_t *obj_val =
          reinterpret_cast<val_ctr_domain_st_t*> (value_struct);
      if (obj_val != NULL) {
        delete obj_val;
        obj_val = NULL;
      }
      break;
    }
    case Notfn_Logical_Port: {
      key_logical_port_t *obj_key = reinterpret_cast<key_logical_port_t *>
      (key_struct);
      if (obj_key != NULL) {
        delete obj_key;
        obj_key = NULL;
      }
      val_logical_port_st_t *obj_val =
          reinterpret_cast<val_logical_port_st_t*> (value_struct);
      if (obj_val != NULL) {
        delete obj_val;
        obj_val = NULL;
      }
      break;
    }
    case Notfn_Logical_Member_Port: {
      key_logical_member_port_t *obj_key =
          reinterpret_cast<key_logical_member_port_t *>
      (key_struct);
      if (obj_key != NULL) {
        delete obj_key;
        obj_key = NULL;
      }
      break;
    }
    case Notfn_Switch: {
      key_switch_t *obj_key = reinterpret_cast<key_switch_t *>
      (key_struct);
      if (obj_key != NULL) {
        delete obj_key;
        obj_key = NULL;
      }
      val_switch_st_t *obj_val =
          reinterpret_cast<val_switch_st_t*> (value_struct);
      if (obj_val != NULL) {
        delete obj_val;
        obj_val = NULL;
      }
      break;
    }
    case Notfn_Port: {
      key_port_t *obj_key = reinterpret_cast<key_port_t *>
      (key_struct);
      if (obj_key != NULL) {
        delete obj_key;
        obj_key = NULL;
      }
      val_port_st_t *obj_val =
          reinterpret_cast<val_port_st_t*> (value_struct);
      if (obj_val != NULL) {
        delete obj_val;
        obj_val = NULL;
      }
      break;
    }
    case Notfn_Link: {
      key_link_t *obj_key = reinterpret_cast<key_link_t *>
      (key_struct);
      if (obj_key != NULL) {
        delete obj_key;
        obj_key = NULL;
      }
      val_port_st_t *obj_val =
          reinterpret_cast<val_port_st_t*> (value_struct);
      if (obj_val != NULL) {
        delete obj_val;
        obj_val = NULL;
      }
      break;
    }
  }
  return;
}

/**
 * @Description : This overloaded function is called when audit notification
 * time out expires
 * @param[in]   : None
 * @return      : None
 * */
void AuditNotification::operator() ()  {
  pfc_log_debug("Received Audit Notification timer out controller %s",
                controller_name_.c_str());
  AuditRequest audit_req;
  UpplReturnCode db_ret = UPPL_RC_SUCCESS;
  OPEN_DB_CONNECTION(unc::uppl::kOdbcmConnReadWriteSb, db_ret);
  if (db_ret != UPPL_RC_SUCCESS) {
    pfc_log_error("Error in opening DB connection");
  } else {
    audit_req.MergeAuditDbToRunning(&db_conn, controller_name_);
  }
}

/**
 * @Description : This function compares running db and import db
 *                If values mismatch in import db and running db, running db
 *                will be updated using values from import db
 *                If key instances available in running db is not available in
 *                import db, it will be deleted from running db
 * @param[in]   : Vector of Key, Value available in running and import db
 * @return      : None
 * */
void AuditRequest::UpdateRunningDbWithImportDb(
    const vector<void *> &key_import,
    const vector<void *> &val_import,
    const vector<void *> &key_running,
    const vector<void *> &val_running,
    unsigned int index,
    Kt_Base *class_pointer,
    OdbcmConnectionHandler *db_conn,
    uint32_t key_type) {
  for (unsigned int iIndex = 0;
      iIndex < (uint32_t)key_running.size(); ++iIndex) {
    bool present_in_import = false;
    for (unsigned int jIndex = 0;
        jIndex < (uint32_t)key_import.size(); ++jIndex) {
      // check whether key is same
      pfc_bool_t is_same_key = class_pointer->CompareKeyStruct(
          key_running[iIndex],
          key_import[jIndex]);
      if (is_same_key == PFC_FALSE) {
        continue;
      }
      present_in_import = true;
      // check whether value is same
      if (index == Notfn_Logical_Member_Port) {
        continue;
      }
      pfc_bool_t is_same_value = class_pointer->CompareValueStruct(
          val_running[iIndex],
          val_import[jIndex]);
      if (is_same_value == PFC_TRUE) {
        pfc_log_debug("Values are same");
        continue;
      }
      // Update running db with import value
      pfc_log_debug("Update running db with import db value");
      void *old_val_struct = NULL;
      UpplReturnCode update_status = class_pointer->
          UpdateKeyInstance(
              db_conn, reinterpret_cast<void *>(key_running[iIndex]),
              reinterpret_cast<void *>(val_import[jIndex]),
              (uint32_t)UNC_DT_STATE,
              key_type,
              old_val_struct);
      if (update_status != UPPL_RC_SUCCESS) {
        pfc_log_error("Update failed for existing key");
        continue;
      }
      // Send value change notification to north bound - val_st
      UpplReturnCode notfn_status =
          class_pointer->ConfigurationChangeNotification(
              (uint32_t)UNC_DT_STATE,
              key_type,
              (uint32_t)UNC_OP_UPDATE,
              reinterpret_cast<void *>(key_running[iIndex]),
              reinterpret_cast<void *>(val_running[iIndex]),
              reinterpret_cast<void *>(val_import[jIndex]));
      if (notfn_status != UPPL_RC_SUCCESS) {
        pfc_log_error("Failed sending update notification");
      }
      // Old value structure memory clean up
      if(old_val_struct != NULL) {
        class_pointer->ClearValueStructure(key_type,
                        old_val_struct);
        old_val_struct = NULL;
      }
      break;
    }
    if (present_in_import == false) {
      // Delete instance from running db
      pfc_log_debug("Delete existing entries from running db");
      UpplReturnCode delete_status = class_pointer->DeleteKeyInstance(
          db_conn, reinterpret_cast<void *>(key_running[iIndex]),
          (uint32_t)UNC_DT_STATE,
          key_type);
      if (delete_status != UPPL_RC_SUCCESS) {
        pfc_log_error("Delete failed for existing key");
      }
      // Send value change notification to north bound - val_st
      UpplReturnCode notfn_status =
          class_pointer->ConfigurationChangeNotification(
              (uint32_t)UNC_DT_STATE,
              key_type,
              (uint32_t)UNC_OP_DELETE,
              reinterpret_cast<void *>(key_running[iIndex]),
              NULL,
              NULL);
      if (notfn_status != UPPL_RC_SUCCESS) {
        pfc_log_error("Failed sending delete notification");
      }
    }
  }
}

/**
 * @Description : This function compares running db and import db *
 *                If key instances available in import db is not available in
 *                running db, it will be added to running db
 * @param[in]   : Vector of Key, Value available in running and import db
 * @return      : None
 * */
void AuditRequest::AddToRunningDbFromImportDb(
    const vector<void *> &key_import,
    const vector<void *> &val_import,
    const vector<void *> &key_running,
    const vector<void *> &val_running,
    unsigned int index,
    Kt_Base *class_pointer,
    OdbcmConnectionHandler *db_conn,
    uint32_t key_type) {
  for (unsigned int iIndex = 0;
      iIndex < (uint32_t)key_import.size(); ++iIndex) {
    bool present_in_running = false;
    for (unsigned int jIndex = 0;
        jIndex < (uint32_t)key_running.size(); ++jIndex) {
      pfc_bool_t is_same_key = class_pointer->CompareKeyStruct(
          key_import[iIndex],
          key_running[jIndex]);
      // Check if key already present in running db
      if (is_same_key == PFC_TRUE) {
        present_in_running = true;
        break;
      }
    }
    if (present_in_running == true) {
      continue;
    }
    // Create instance in running db
    UpplReturnCode create_status = UPPL_RC_SUCCESS;
    // Validate key
    UpplReturnCode validate_status = UPPL_RC_SUCCESS;
    if (index == Notfn_Logical_Member_Port) {
      validate_status = class_pointer->ValidateRequest(
          db_conn,
          reinterpret_cast<void *>(key_import[iIndex]),
          NULL,
          UNC_OP_CREATE, UNC_DT_STATE, key_type);
    } else {
      validate_status = class_pointer->ValidateRequest(
          db_conn,
          reinterpret_cast<void *>(key_import[iIndex]),
          reinterpret_cast<void *>(val_import[iIndex]),
          UNC_OP_CREATE, UNC_DT_STATE, key_type);
    }
    if (validate_status != UPPL_RC_SUCCESS) {
      pfc_log_info("Validation failed for index %d", index);
      continue;
    }
    pfc_log_debug("Create new entries in db for index: %d", index);
    if (index == Notfn_Logical_Member_Port) {
      create_status = class_pointer->CreateKeyInstance(
          db_conn, reinterpret_cast<void *>(key_import[iIndex]),
          NULL,
          (uint32_t)UNC_DT_STATE,
          key_type);
    } else {
      create_status = class_pointer->CreateKeyInstance(
          db_conn, reinterpret_cast<void *>(key_import[iIndex]),
          reinterpret_cast<void *>(val_import[iIndex]),
          (uint32_t)UNC_DT_STATE,
          key_type);
    }
    if (create_status == UPPL_RC_SUCCESS) {
      // Send value change notification to north bound - val_st
      UpplReturnCode notfn_status = UPPL_RC_SUCCESS;
      if (index == Notfn_Logical_Member_Port) {
        notfn_status =
            class_pointer->ConfigurationChangeNotification(
                (uint32_t)UNC_DT_STATE,
                key_type,
                (uint32_t)UNC_OP_CREATE,
                reinterpret_cast<void *>(key_import[iIndex]),
                NULL,
                NULL);
      } else {
        notfn_status =
            class_pointer->ConfigurationChangeNotification(
                (uint32_t)UNC_DT_STATE,
                key_type,
                (uint32_t)UNC_OP_CREATE,
                reinterpret_cast<void *>(key_import[iIndex]),
                NULL,
                reinterpret_cast<void *>(val_import[iIndex]));
      }
      if (notfn_status != UPPL_RC_SUCCESS) {
        pfc_log_error("Failed sending create notification");
      }
    } else {
      pfc_log_error("Create failed for new key");
    }
  }
  return;
}

/**
 * @Description : This function clears the internal structure contains key
 *                and values of import and running db
 * @param[in]   : Vector of Key, Value available in running and import db
 * @return      : None
 * */
void AuditRequest::FreeKeyAndValueStruct(
    const vector<void *> &key_import,
    const vector<void *> &val_import,
    const vector<void *> &key_running,
    const vector<void *> &val_running,
    unsigned int index) {
  for (unsigned int iIndex = 0;
      iIndex < (uint32_t)key_import.size(); ++iIndex) {
    if (index == Notfn_Logical_Member_Port) {
      FreeKeyAndValueStruct(key_import[iIndex],
                            NULL,
                            index);
    } else {
      FreeKeyAndValueStruct(key_import[iIndex],
                            val_import[iIndex],
                            index);
    }
  }
  for (unsigned int iIndex = 0;
      iIndex < (uint32_t)key_running.size(); ++iIndex) {
    if (index == Notfn_Logical_Member_Port) {
      FreeKeyAndValueStruct(key_running[iIndex],
                            NULL,
                            index);
    } else {
      FreeKeyAndValueStruct(key_running[iIndex],
                            val_running[iIndex],
                            index);
    }
  }
}

}  // namespace uppl
}  // namespace unc
