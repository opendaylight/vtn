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

/**
 * * @Description : Constructor of Audit Request Class
 * * * @param[in] : No Parameters
 * * * @return    : No Return Type
 * */
AuditRequest::AuditRequest() {
}

/**
 * * @Description : Destructor of Audit Request Class
 * * @param[in] : No Parameters
 * * @return    : No Return Type
 * */
AuditRequest::~AuditRequest() {
}

/**
 * * @Description : This function is used to perform the Start of Audit
 * * @param[in] : driver_id,controller_id
 * * @return    : UpplReturnCode
 * */

UpplReturnCode AuditRequest::StartAudit(unc_keytype_ctrtype_t driver_id,
                                        string controller_id) {
  pfc_log_info("Inside StartAudit");
  PhysicalCore *physical_core = PhysicalCore::get_physical_core();
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
  pfc_log_info("Get the oper Status");
  val_ctr_st_t obj_val_ctr_st;
  UpplReturnCode read_status =
      KtObj.GetOperStatus(UNC_DT_RUNNING,
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
          UNC_DT_RUNNING, reinterpret_cast<void *>(&obj_key_ctr),
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
    /* Sending the Audit failure alarm to Node Manager */
    physical_core->SendAuditFailureAlarm(controller_id);
    return UPPL_RC_ERR_AUDIT_FAILURE;
  }

  return UPPL_RC_SUCCESS;
}

/**
 * * @Description : This function is used to perform the Start of Audit Transaction
 * * * @param[in] : session_id,controller type and controller_id
 * * * @return    : UpplReturnCode
 * */
UpplReturnCode AuditRequest::StartAuditTransaction(
    uint32_t session_id,
    unc_keytype_ctrtype_t driver_id,
    string controller_id)  {
  pfc_log_info("Inside StartAuditTransaction");
  return UPPL_RC_SUCCESS;
}
/**
 * * @Description : This function is used to handle the Audit Vote Request
 * * * @param[in] : session_id,driver_id,controller_id,driver_info map
 * * * @return    : UpplReturnCode
 * */
UpplReturnCode AuditRequest::HandleAuditVoteRequest(
    uint32_t session_id,
    uint32_t driver_id,
    string controller_id,
    TcDriverInfoMap &driver_info) {
  pfc_log_info("Inside HandleAuditVoteRequest");
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
      vect_ctr_key, vect_ctr_val,
      (unc_keytype_datatype_t)UNC_DT_RUNNING,
      UNC_OP_READ);
  pfc_log_info("read_status is %d", read_status);
  if (read_status != UPPL_RC_SUCCESS) {
    pfc_log_error("Could not get details for controller %s",
                  controller_id.c_str());
  } else {
    pfc_log_info("Read operation is success");
    val_ctr_st_t *obj_val_ctr =
        (reinterpret_cast <val_ctr_st_t*>(vect_ctr_val[0]));
    if (obj_val_ctr != NULL) {
      unc_keytype_ctrtype_t controller_type =
          (unc_keytype_ctrtype_t)
          (PhyUtil::uint8touint(obj_val_ctr->controller.type));
      pfc_log_info("Type is %d", controller_type);
      if (controller_type != UNC_CT_PFC &&
          controller_type != UNC_CT_VNP) {
        // return UPPL_RC_ERR_AUDIT_FAILURE;
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
  }
  return UPPL_RC_SUCCESS;
}

/* *
 * * @Description : This function is used to handle the Audit Global Commit
 * * * @param[in] : session_id,driver_id,controller_id,driver_info map,audit result
 * * * @return    : UpplReturnCode
 * */

UpplReturnCode AuditRequest::HandleAuditGlobalCommit(
    uint32_t session_id,
    uint32_t driver_id,
    string controller_id,
    TcDriverInfoMap& driver_info,
    TcAuditResult& audit_result) {
  pfc_log_info("Inside HandleAuditGlobalCommit");
  audit_result = unc::tclib::TC_AUDIT_SUCCESS;
  return UPPL_RC_SUCCESS;
}


/**
 * * @Description : This function is used to abort the Audit Process
 * * * @param[in] : session id, controller type and controller name
 * * * @return    : UpplReturnCode
 * */

UpplReturnCode AuditRequest::HandleAuditAbort(
    uint32_t session_id,
    unc_keytype_ctrtype_t driver_id,
    string controller_id,
    TcAuditOpAbortPhase operation_phase) {
  pfc_log_info("Inside HandleAuditAbort");
  return UPPL_RC_SUCCESS;
}

/**
 * * @Description : This function is used to end the Audit Transaction
 * * * @param[in] : session_id ,controller type, controller_id
 * * * @return    : UpplReturnCode
 * */
UpplReturnCode AuditRequest::EndAuditTransaction(
    uint32_t session_id,
    unc_keytype_ctrtype_t& drivertype,
    string controller_id) {
  pfc_log_info("Inside EndAuditTransaction");
  return UPPL_RC_SUCCESS;
}

/**
 * * @Description : This function is used to end the Audit
 * * * @param[in] : controller type, controller_id and audit result
 * * * @return    : UpplReturnCode
 * */
UpplReturnCode AuditRequest::EndAudit(unc_keytype_ctrtype_t driver_id,
                                      string controller_id,
                                      TcAuditResult audit_result) {
  pfc_log_info("Inside HandleEndAudit");
  uint8_t oper_status = 0;
  PhysicalCore *physical_core = PhysicalLayer::get_instance()->
      get_physical_core();
  Kt_Controller kt_controller;
  key_ctr_t key_ctr_obj;
  memcpy(key_ctr_obj.controller_name,
         controller_id.c_str(), controller_id.length()+1);
  void *key_ctr_prt = reinterpret_cast<void *>(&key_ctr_obj);
  val_ctr_st_t obj_val_ctr_st;
  pfc_log_debug("Audit result from TC %d", audit_result);
  if (audit_result == unc::tclib::TC_AUDIT_SUCCESS) {
    oper_status = UPPL_CONTROLLER_OPER_UP;
    obj_val_ctr_st.oper_status = oper_status;
    memset(obj_val_ctr_st.valid, '\0', sizeof(obj_val_ctr_st.valid));
    obj_val_ctr_st.valid[kIdxOperStatus] = 1;
    FN_START_TIME("Audit::HandleOperStatus", "Controller");
    UpplReturnCode handle_oper_status = kt_controller.HandleOperStatus(
        UNC_DT_RUNNING,
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
      uint32_t ret = ipc_mgr->StartNotificationTimer(controller_id);
      pfc_log_debug("Start Timer return code %d for controller %s",
                    ret, controller_id.c_str());
    } else {
      pfc_log_info("IPC Connection Manager Object is NULL");
    }
    // Sending  AUDIT_SUCCESS alarm to node manager
    physical_core->SendAuditSuccessAlarm(controller_id);
  } else {
    oper_status = UPPL_CONTROLLER_OPER_WAITING_AUDIT;
    obj_val_ctr_st.oper_status = oper_status;
    memset(obj_val_ctr_st.valid, '\0', sizeof(obj_val_ctr_st.valid));
    obj_val_ctr_st.valid[kIdxOperStatus] = 1;
    UpplReturnCode handle_oper_status = kt_controller.HandleOperStatus(
        UNC_DT_RUNNING,
        key_ctr_prt,
        reinterpret_cast<void *>(&obj_val_ctr_st),
        true);
    pfc_log_debug("Handle Oper Status return: %d", handle_oper_status);
    // Sending the AUDIT_FAILURE alarm to Node Manager
    physical_core->SendAuditFailureAlarm(controller_id);
  }
  return UPPL_RC_SUCCESS;
}

/**
 * * @Description : This function is used to Merge audit and running db at
 * end the Audit
 * * * @param[in] : controller name
 * * * @return    : UpplReturnCode
 * */
UpplReturnCode AuditRequest::MergeAuditDbToRunning(
    string controller_name) {
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
      continue;
    }
    vector<void*> key_running, key_import;
    key_running.push_back(key_struct);
    key_import.push_back(key_struct);
    vector<void*> val_running, val_import;
    pfc_log_debug("Reading from running db for index %d", index);
    UpplReturnCode read_running_status = class_pointer->ReadInternal(
        key_running,
        val_running,
        (uint32_t) UNC_DT_RUNNING,
        (uint32_t) UNC_OP_READ_SIBLING_BEGIN);
    // Read from Import DB
    pfc_log_debug("Reading from import db for index %d", index);
    UpplReturnCode read_import_status = class_pointer->ReadInternal(
        key_import,
        val_import,
        (uint32_t) UNC_DT_IMPORT,
        (uint32_t) UNC_OP_READ_SIBLING_BEGIN);
    if (read_running_status != UPPL_RC_SUCCESS &&
        read_import_status != UPPL_RC_SUCCESS) {
      pfc_log_debug(
          "Reading values from both import and running db failed for index %d",
          index);
      continue;
    }
    pfc_log_debug("No.of entries from running db %d",
                  static_cast<int>(key_running.size()));
    pfc_log_debug("No.of entries from import db %d",
                  static_cast<int>(key_import.size()));
    // Iterate RUNNING and compare with IMPORT
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
        void *old_val_struct;
        UpplReturnCode update_status = class_pointer->
            UpdateKeyInstance(
                reinterpret_cast<void *>(key_running[iIndex]),
                reinterpret_cast<void *>(val_import[jIndex]),
                (uint32_t)UNC_DT_STATE,
                key_type,
                old_val_struct);
        if (update_status != UPPL_RC_SUCCESS) {
          pfc_log_debug("Update failed for existing key");
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
          pfc_log_debug("Failed sending update notification");
        }
        break;
      }
      if (present_in_import == false) {
        // Delete instance from running db
        pfc_log_debug("Delete existing entries from running db");
        UpplReturnCode delete_status = class_pointer->DeleteKeyInstance(
            reinterpret_cast<void *>(key_running[iIndex]),
            (uint32_t)UNC_DT_STATE,
            key_type);
        if (delete_status != UPPL_RC_SUCCESS) {
          pfc_log_debug("Delete failed for existing key");
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
          pfc_log_debug("Failed sending delete notification");
        }
      }
    }
    // Iterate IMPORT and compare with RUNNING
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
      UpplReturnCode create_status = UPPL_RC_SUCCESS;
      // Create instance in running db
      pfc_log_debug("Create new entries in db for index: %d", index);
      if (index == Notfn_Logical_Member_Port) {
        create_status = class_pointer->CreateKeyInstance(
            reinterpret_cast<void *>(key_import[iIndex]),
            NULL,
            (uint32_t)UNC_DT_STATE,
            key_type);
      } else {
        create_status = class_pointer->CreateKeyInstance(
            reinterpret_cast<void *>(key_import[iIndex]),
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
          pfc_log_debug("Failed sending create notification");
        }
      } else {
        pfc_log_debug("Create failed for new key");
      }
    }
    // Free the readInternal key and value structures
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
    delete class_pointer;
    if (key_struct != NULL) {
      FreeKeyAndValueStruct(key_struct, NULL, index);
    }
  }
  // Clear import db entries
  ODBCM_RC_STATUS clear_status =
      PhysicalLayer::get_instance()->get_odbc_manager()->
      ClearOneInstance(UNC_DT_IMPORT, controller_name);
  if (clear_status != ODBCM_RC_SUCCESS) {
    pfc_log_info("Import DB clearing failed");
  }
  return return_code;
}

/**
 * * @Description : This function is used to get the key and class pointer
 * * * @param[in] : Notification class key type, controller name
 * * * @return    : Kt_Base* poiner and key struct
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

/** FreeKeyAndValueStruct
 * * @Description : This function clears the key struct and value struct
 * * * @param[in] : key struct, value struct and key type
 * * * @return    : void * key structure
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
}  // namespace uppl
}  // namespace unc
