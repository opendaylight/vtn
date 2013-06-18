/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/**
 * @brief   Kt Base implementation
 * @file    itc_kt_state_base.cc
 *
 */

#include "itc_kt_state_base.hh"
#include "physicallayer.hh"
#include "itc_kt_logicalport.hh"

using unc::uppl::PhysicalLayer;

/** Constructor
 * * @Description : This function instantiates parent and child key types for
 * kt
 * * * @param[in] : None
 * * * @return    : None
 * */
Kt_State_Base::Kt_State_Base() {
}

/** Create
 * * @Description : This function returns error when CREATE request is sent for State
 * Objects from north bound
 * * @param[in] : session_id - ipc session id used for TC validation
 * configuration_id - configuration id used for TC validation
 * key_struct - the key for the new kt instance
 * value_struct - the values for the new kt instance
 * data_type - UNC_DT_* , Create only allowed in running/state/import
 * sess - ipc server session where the response has to be added
 * * @return    : UPPL_RC_ERR_* is returned.
 * */
UpplReturnCode Kt_State_Base::Create(uint32_t session_id,
                                     uint32_t configuration_id,
                                     void* key_struct,
                                     void* val_struct,
                                     uint32_t data_type,
                                     uint32_t key_type,
                                     ServerSession &sess) {
  pfc_log_error("Create not allowed from VTN");
  UpplReturnCode create_status = UPPL_RC_SUCCESS;

  // Populate the response to be sent in ServerSession
  physical_response_header rsh = {session_id,
      configuration_id,
      UNC_OP_CREATE,
      0,
      0,
      0,
      data_type,
      UPPL_RC_ERR_OPERATION_NOT_ALLOWED};

  int err = PhyUtil::sessOutRespHeader(sess, rsh);
  err |= sess.addOutput(key_type);
  switch (key_type) {
    case UNC_KT_LOGICAL_PORT: {
      key_logical_port_t *obj_key =
          reinterpret_cast<key_logical_port_t*>(key_struct);
      err |= sess.addOutput(*obj_key);
      break;
    }
    case UNC_KT_LOGICAL_MEMBER_PORT: {
      key_logical_member_port_t *obj_key =
          reinterpret_cast<key_logical_member_port_t*>(key_struct);
      err |= sess.addOutput(*obj_key);
      break;
    }
    case UNC_KT_SWITCH: {
      key_switch_t *obj_key = reinterpret_cast<key_switch_t*>(key_struct);
      err |= sess.addOutput(*obj_key);
      break;
    }
    case UNC_KT_PORT: {
      key_port_t *obj_key = reinterpret_cast<key_port_t*>(key_struct);
      err |= sess.addOutput(*obj_key);
      break;
    }
    case UNC_KT_LINK: {
      key_link_t *obj_key = reinterpret_cast<key_link_t*>(key_struct);
      err |= sess.addOutput(*obj_key);
      break;
    }
  }

  if (err != 0) {
    pfc_log_error("Server session addOutput failed, so return IPC_WRITE_ERROR");
    create_status = UPPL_RC_ERR_IPC_WRITE_ERROR;
  } else {
    create_status = UPPL_RC_SUCCESS;
  }
  return create_status;
}

/** CreateKeyInstance
 * * @Description : This function creates a new row of KT in
 * candidate  table.
 * key_struct - the key for the new kt  instance
 * value_struct - the values for the new kt  instance
 * data_type - UNC_DT_* , Create only allowed in running/state/import
 * * @return    : UPPL_RC_SUCCESS is returned when the create is success
 * UPPL_RC_ERR_* is returned when ipc response could not be added to sess.
 * */
UpplReturnCode Kt_State_Base::CreateKeyInstance(void* key_struct,
                                                void* val_struct,
                                                uint32_t data_type,
                                                uint32_t key_type) {
  UpplReturnCode create_status = UPPL_RC_SUCCESS;
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();

  pfc_log_debug("Create instance of Kt: %d", key_type);
  // Check whether operation is allowed on the given DT_TYPE
  if (((unc_keytype_datatype_t)data_type != UNC_DT_STATE) &&
      ((unc_keytype_datatype_t)data_type != UNC_DT_IMPORT)) {
    pfc_log_error("Create operation is provided on unsupported data type %d",
                  data_type);
    return UPPL_RC_ERR_OPERATION_NOT_ALLOWED;
  }

  // Structure used to send request to ODBC
  DBTableSchema kt_dbtableschema;
  void* old_value_struct;
  vector<ODBCMOperator> vect_key_operations;
  // Create DBSchema structure for kt_table
  PopulateDBSchemaForKtTable(kt_dbtableschema,
                             key_struct,
                             val_struct,
                             UNC_OP_CREATE, 0, 0,
                             vect_key_operations, old_value_struct);

  // Send request to ODBC for kt_table create
  ODBCM_RC_STATUS create_db_status = physical_layer->get_odbc_manager()->
      CreateOneRow((unc_keytype_datatype_t)data_type, kt_dbtableschema);

  pfc_log_info("Response from DB: %d", create_db_status);

  if (create_db_status != ODBCM_RC_SUCCESS) {
    if (create_db_status == ODBCM_RC_CONNECTION_ERROR) {
      // log fatal error to log daemon
      pfc_log_fatal("DB connection not available or cannot access DB");
      create_status = UPPL_RC_ERR_DB_ACCESS;
    } else {
      // log error to log daemon
      pfc_log_error("Create operation has failed");
      create_status = UPPL_RC_ERR_DB_CREATE;
    }
  } else {
    pfc_log_info("Create of a kt %d in data_type %d is success", key_type,
                 data_type);
  }
  return create_status;
}

/** Update
 * * @Description : This function updates a row of KT in
 * candidate  table.
 * * @param[in] : session_id - ipc session id used for TC validation
 * configuration_id - configuration id used for TC validation
 * key_struct - the key for the kt  instance
 * value_struct - the values for the kt  instance
 * data_type - UNC_DT_* , Update only allowed in running/state/import
 * sess - ipc server session where the response has to be added
 * * @return    : UPPL_RC_SUCCESS is returned when the response
 * is added to ipc session successfully.
 * UPPL_RC_ERR_* is returned when ipc response could not be added to sess.
 * */
UpplReturnCode Kt_State_Base::Update(uint32_t session_id,
                                     uint32_t configuration_id,
                                     void* key_struct,
                                     void* val_struct,
                                     uint32_t data_type,
                                     uint32_t key_type,
                                     ServerSession &sess) {
  pfc_log_error("Update not allowed from VTN");
  UpplReturnCode update_status = UPPL_RC_SUCCESS;

  // Populate the response to be sent in ServerSession
  physical_response_header rsh = {session_id,
      configuration_id,
      UNC_OP_UPDATE,
      0,
      0,
      0,
      data_type,
      UPPL_RC_ERR_OPERATION_NOT_ALLOWED};

  int err = PhyUtil::sessOutRespHeader(sess, rsh);
  err |= sess.addOutput(key_type);
  switch (key_type) {
    case UNC_KT_LOGICAL_PORT: {
      key_logical_port_t *obj_key =
          reinterpret_cast<key_logical_port_t*>(key_struct);
      err |= sess.addOutput(*obj_key);
      break;
    }
    case UNC_KT_LOGICAL_MEMBER_PORT: {
      key_logical_member_port_t *obj_key =
          reinterpret_cast<key_logical_member_port_t*>(key_struct);
      err |= sess.addOutput(*obj_key);
      break;
    }
    case UNC_KT_SWITCH: {
      key_switch_t *obj_key = reinterpret_cast<key_switch_t*>(key_struct);
      err |= sess.addOutput(*obj_key);
      break;
    }
    case UNC_KT_PORT: {
      key_port_t *obj_key = reinterpret_cast<key_port_t*>(key_struct);
      err |= sess.addOutput(*obj_key);
      break;
    }
    case UNC_KT_LINK: {
      key_link_t *obj_key = reinterpret_cast<key_link_t*>(key_struct);
      err |= sess.addOutput(*obj_key);
      break;
    }
  }

  if (err != 0) {
    pfc_log_error("Server session addOutput failed, so return IPC_WRITE_ERROR");
    update_status = UPPL_RC_ERR_IPC_WRITE_ERROR;
  } else {
    update_status = UPPL_RC_SUCCESS;
  }
  return update_status;
}

/** UpdateKeyInstance
 * * @Description : This function updates a row of KT in
 * candidate  table.
 * * @param[in] :
 * key_struct - the key for the new kt instance
 * value_struct - the values for the new kt instance
 * data_type - UNC_DT_* , update only allowed in running/state/import
 * * @return    : UPPL_RC_SUCCESS is returned when the update
 * is done successfully.
 * UPPL_RC_ERR_* is returned when the update is error
 * */
UpplReturnCode Kt_State_Base::UpdateKeyInstance(void* key_struct,
                                                void* val_struct,
                                                uint32_t data_type,
                                                uint32_t key_type,
                                                void* &old_val_struct) {
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  UpplReturnCode update_status = UPPL_RC_SUCCESS;
  if (key_type == UNC_KT_LOGICAL_PORT ||
      key_type == UNC_KT_LOGICAL_MEMBER_PORT) {
    pfc_log_error("Update operation is provided on unsupported key type %d",
                  key_type);
    update_status = UPPL_RC_ERR_OPERATION_NOT_ALLOWED;
    return update_status;
  }
  // Check whether operation is allowed on the given DT type
  if (((unc_keytype_datatype_t)data_type != UNC_DT_STATE) &&
      ((unc_keytype_datatype_t)data_type != UNC_DT_IMPORT)) {
    pfc_log_error("Update operation is provided on unsupported data type %d",
                  data_type);
    update_status = UPPL_RC_ERR_OPERATION_NOT_ALLOWED;
    return update_status;
  }

  // Structure used to send request to ODBC
  DBTableSchema kt_dbtableschema;
  vector<ODBCMOperator> vect_key_operations;
  // Create DBSchema structure for table
  PopulateDBSchemaForKtTable(kt_dbtableschema,
                             key_struct,
                             val_struct,
                             UNC_OP_UPDATE, 0, 0,
                             vect_key_operations, old_val_struct);
  if (!((kt_dbtableschema.get_row_list()).empty())) {
    // Send request to ODBC for kt_table update
    ODBCM_RC_STATUS update_db_status = physical_layer->get_odbc_manager()-> \
        UpdateOneRow((unc_keytype_datatype_t)data_type,
                     kt_dbtableschema);
    pfc_log_info("UpdateOneRow response from DB is %d", update_db_status);
    if (update_db_status == ODBCM_RC_CONNECTION_ERROR) {
      // log fatal error to log daemon
      pfc_log_fatal("DB connection not available or cannot access DB");
      update_status = UPPL_RC_ERR_DB_ACCESS;
    } else if (update_db_status != ODBCM_RC_SUCCESS) {
      // log error to log daemon
      pfc_log_error("UpdateOneRow error response from DB is %d",
                    update_db_status);
      update_status = UPPL_RC_ERR_DB_UPDATE;
    } else {
      pfc_log_info("Update of kt in data type %d is success",
                   data_type);
    }
  }
  return update_status;
}

/**Delete
 * * @Description : This function deletes a row of KT in
 * candidate  table.
 * * @param[in] : session_id - ipc session id used for TC validation
 * configuration_id - configuration id used for TC validation
 * key_struct - the key for the kt instance
 * data_type - UNC_DT_* , delete only allowed in running/state/import
 * sess - ipc server session where the response has to be added
 * * @return    : UPPL_RC_SUCCESS is returned when the response
 * is added to ipc session successfully.
 * UPPL_RC_ERR_* is returned when ipc response could not be added to sess.
 * */
UpplReturnCode Kt_State_Base::Delete(uint32_t session_id,
                                     uint32_t configuration_id,
                                     void* key_struct,
                                     uint32_t data_type,
                                     uint32_t key_type,
                                     ServerSession &sess) {
  pfc_log_error("Delete not allowed from VTN");
  UpplReturnCode delete_status = UPPL_RC_SUCCESS;

  // Populate the response to be sent in ServerSession
  physical_response_header rsh = {session_id,
      configuration_id,
      UNC_OP_DELETE,
      0,
      0,
      0,
      data_type,
      UPPL_RC_ERR_OPERATION_NOT_ALLOWED};

  int err = PhyUtil::sessOutRespHeader(sess, rsh);
  err |= sess.addOutput(key_type);
  switch (key_type) {
    case UNC_KT_LOGICAL_PORT: {
      key_logical_port_t *obj_key =
          reinterpret_cast<key_logical_port_t*>(key_struct);
      err |= sess.addOutput(*obj_key);
      break;
    }
    case UNC_KT_LOGICAL_MEMBER_PORT: {
      key_logical_member_port_t *obj_key =
          reinterpret_cast<key_logical_member_port_t*>(key_struct);
      err |= sess.addOutput(*obj_key);
      break;
    }
    case UNC_KT_SWITCH: {
      key_switch_t *obj_key = reinterpret_cast<key_switch_t*>(key_struct);
      err |= sess.addOutput(*obj_key);
      break;
    }
    case UNC_KT_PORT: {
      key_port_t *obj_key = reinterpret_cast<key_port_t*>(key_struct);
      err |= sess.addOutput(*obj_key);
      break;
    }
    case UNC_KT_LINK: {
      key_link_t *obj_key = reinterpret_cast<key_link_t*>(key_struct);
      err |= sess.addOutput(*obj_key);
      break;
    }
  }

  if (err != 0) {
    pfc_log_error("Server session addOutput failed, so return IPC_WRITE_ERROR");
    delete_status = UPPL_RC_ERR_IPC_WRITE_ERROR;
  } else {
    delete_status = UPPL_RC_SUCCESS;
  }
  return delete_status;
}

/** HandleDriverEvents
 * * @Description : This function processes the notification received from
 * driver
 * * * @param[in] : key_struct - specifies the key instance of KT
 * old_value_struct - old value of KT
 * new_value_struct - new value of KT
 * * * @return    : UPPL_RC_SUCCESS if events are handled successfully or
 * UPPL_RC_ERR*
 * */
UpplReturnCode Kt_State_Base::HandleDriverEvents(void* key_struct,
                                                 uint32_t oper_type,
                                                 uint32_t data_type,
                                                 uint32_t key_type,
                                                 void* old_val_struct,
                                                 void* new_val_struct) {
  UpplReturnCode status = UPPL_RC_SUCCESS;
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  string controller_name;
  string event_details;  // for sending event handling alarm
  // Validate the given request
  switch (key_type) {
    case UNC_KT_CTR_DOMAIN: {
      key_ctr_domain_t *obj_key =
          reinterpret_cast<key_ctr_domain_t*>(key_struct);
      controller_name = reinterpret_cast<const char*>(
          obj_key->ctr_key.controller_name);
      uint32_t parent_data_type = data_type;
      if (data_type == UNC_DT_IMPORT) {
        parent_data_type = UNC_DT_RUNNING;
      }
      unc_keytype_ctrtype_t controller_type;
      UpplReturnCode retcode = PhyUtil::get_controller_type(
          controller_name, controller_type,
          (unc_keytype_datatype_t)parent_data_type);
      // Check whether operation is allowed on the given DT type
      if (retcode != UPPL_RC_SUCCESS) {
        pfc_log_error("Error getting the controller type");
        return UPPL_RC_ERR_OPERATION_NOT_ALLOWED;
      } else if (retcode == UPPL_RC_SUCCESS &&
          controller_type == UNC_CT_UNKNOWN) {
        pfc_log_error("Unknown domain cannot be operated from driver");
        return UPPL_RC_ERR_OPERATION_NOT_ALLOWED;
      }
      event_details = "KT_CONTROLLER";
      break;
    }
    case UNC_KT_LOGICAL_PORT: {
      key_logical_port_t *obj_key =
          reinterpret_cast<key_logical_port_t*>(key_struct);
      controller_name = reinterpret_cast<const char*>(
          obj_key->domain_key.ctr_key.controller_name);
      event_details = "KT_LOGICAL_PORT";
      break;
    }
    case UNC_KT_LOGICAL_MEMBER_PORT: {
      key_logical_member_port_t *obj_key =
          reinterpret_cast<key_logical_member_port_t*>(key_struct);
      controller_name = reinterpret_cast<const char*>(
          obj_key->logical_port_key.domain_key.ctr_key.controller_name);
      event_details = "KT_LOGICAL_MEMBER_PORT";
      break;
    }
    case UNC_KT_SWITCH: {
      key_switch_t *obj_key = reinterpret_cast<key_switch_t*>(key_struct);
      controller_name = reinterpret_cast<const char*>(
          obj_key->ctr_key.controller_name);
      event_details = "KT_SWITCH";
      break;
    }
    case UNC_KT_PORT: {
      key_port_t *obj_key = reinterpret_cast<key_port_t*>(key_struct);
      controller_name = reinterpret_cast<const char*>(
          obj_key->sw_key.ctr_key.controller_name);
      event_details = "KT_PORT";
      break;
    }
    case UNC_KT_LINK: {
      key_link_t *obj_key = reinterpret_cast<key_link_t*>(key_struct);
      controller_name = reinterpret_cast<const char*>(
          obj_key->ctr_key.controller_name);
      event_details = "KT_LINK";
      break;
    }
  }
  status = ValidateRequest(key_struct, new_val_struct,
                           oper_type, data_type, key_type);
  if (status != UPPL_RC_SUCCESS) {
    pfc_log_error(
        "HandleDriverEvents failed with %d "
        "for operation %d with data type %d", status, oper_type, data_type);
    UpplReturnCode alarm_status = physical_layer->get_physical_core()->
        RaiseEventHandlingAlarm(controller_name);
    if (alarm_status == UPPL_RC_SUCCESS) {
      alarm_status = physical_layer->get_physical_core()->
          SendEventHandlingFailureAlarm(controller_name,
                                        event_details);
      pfc_log_debug("Failure alarm sent to nodemanager - status %d",
                    alarm_status);
    }
    return status;
  }

  // Call CreateKeyInstance, UpdateKeyInstance and DeleteKeyInstance based
  // on operation type
  // Create the kt instance in DB
  void* old_value_struct;
  if (oper_type == UNC_OP_CREATE) {
    pfc_log_info("Call CreateKeyInstance to create kt in DB");
    status = CreateKeyInstance(key_struct, new_val_struct,
                               data_type, key_type);
    pfc_log_info("CreateKeyInstance status: %d" , status);
  } else if (oper_type == UNC_OP_UPDATE) {
    // Call UpdateKeyInstance to update kt value in DB
    pfc_log_info("Call UpdateKeyInstance to update kt in DB");
    status = UpdateKeyInstance(key_struct, new_val_struct,
                               data_type, key_type,
                               old_value_struct);
    pfc_log_info("UpdateKeyInstance status: %d" , status);
  } else if (oper_type == UNC_OP_DELETE) {
    // Call DeleteKeyInstance to Delete kt from DB
    pfc_log_info("Call DeleteKeyInstance to delete kt in DB");
    status = DeleteKeyInstance(key_struct, data_type, key_type);
    pfc_log_info("DeleteKeyInstance status: %d" , status);
  }
  // logical port oper status handling
  if (key_type == UNC_KT_LOGICAL_MEMBER_PORT &&
      (oper_type == UNC_OP_CREATE || oper_type == UNC_OP_DELETE) &&
       status == UPPL_RC_SUCCESS) {
    key_logical_member_port_t *obj_key =
          reinterpret_cast<key_logical_member_port_t*>(key_struct);
    string domain_name = reinterpret_cast<const char*>(
          obj_key->logical_port_key.domain_key.domain_name);
    string port_id = reinterpret_cast<const char*>(obj_key
      ->logical_port_key.port_id);

    key_logical_port logical_port_key;
    memcpy(logical_port_key.domain_key.ctr_key.controller_name,
         controller_name.c_str(),
         controller_name.length()+1);
    memcpy(logical_port_key.domain_key.domain_name,
         domain_name.c_str(),
         domain_name.length()+1);
    memcpy(logical_port_key.port_id,
           port_id.c_str(),
           port_id.length()+1);

    Kt_LogicalPort logical_port;
    UpplReturnCode return_code = logical_port.HandleOperStatus(
      data_type, reinterpret_cast<void *> (&logical_port_key),
      NULL);
    pfc_log_info("HandleOperStatus for logical_port class %d",
               return_code);
  }

  if (status != UPPL_RC_SUCCESS) {
    UpplReturnCode alarm_status = physical_layer->get_physical_core()->
        RaiseEventHandlingAlarm(controller_name);
    if (alarm_status == UPPL_RC_SUCCESS) {
      alarm_status = physical_layer->get_physical_core()->
          SendEventHandlingFailureAlarm(controller_name,
                                        event_details);
      pfc_log_debug("Failure alarm sent to node manager - status %d",
                    alarm_status);
    }
  } else {
    UpplReturnCode alarm_status = physical_layer->get_physical_core()->
        ClearEventHandlingAlarm(controller_name);
    if (alarm_status == UPPL_RC_SUCCESS) {
      alarm_status = physical_layer->get_physical_core()->
          SendEventHandlingSuccessAlarm(controller_name,
                                        event_details);
      pfc_log_debug("Success alarm sent to node manager - status %d",
                    alarm_status);
    }
    if (data_type != UNC_DT_IMPORT) {
      pfc_ipcevtype_t event_type = GetEventType(key_type);
      int err = 0;
      ServerEvent ser_evt(event_type, err);
      northbound_event_header rsh = {oper_type,
          data_type,
          key_type};
      err = PhyUtil::sessOutNBEventHeader(ser_evt, rsh);
      err |= AddKeyStructuretoSession(key_type,
                                      ser_evt,
                                      key_struct);
      err |= AddValueStructuretoSession(key_type,
                                        oper_type,
                                        data_type,
                                        ser_evt,
                                        new_val_struct,
                                        old_value_struct);
      if (err != 0) {
        pfc_log_error(
            "Server Event addOutput failed, return IPC_WRITE_ERROR");
        status = UPPL_RC_ERR_IPC_WRITE_ERROR;
      } else {
        // Call IPC server to post the event
        status = (UpplReturnCode) physical_layer
            ->get_ipc_connection_manager()->SendEvent(&ser_evt);
      }
    }
    // Old value structure memory clean up
    if (oper_type == UNC_OP_UPDATE) {
      ClearValueStructure(key_type,
                          old_value_struct);
    }
  }
  return status;
}
