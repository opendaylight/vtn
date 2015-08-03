/*
 * Copyright (c) 2012-2015 NEC Corporation
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
#include "itc_kt_port.hh"
#include "itc_kt_port_neighbor.hh"
#include "itc_kt_switch.hh"
#include "itc_kt_link.hh"
#include "itc_kt_controller.hh"

using unc::uppl::PhysicalLayer;

/** Constructor
 * @Description : Empty Constructor
 * @param[in] : None
 * @return    : None
 * */
Kt_State_Base::Kt_State_Base() {
}

/** Create
 * @Description : This function returns error when CREATE request is sent
 * for State Objects from north bound
 * @param[in] : session_id - ipc session id used for TC validation
 * configuration_id - configuration id used for TC validation
 * key_struct - the key for the new kt instance
 * value_struct - the values for the new kt instance
 * data_type - UNC_DT_* , Create only allowed in running/state/import
 * key_type-UNC_KT_*,any value of unc_key_type_t
 * sess - ipc server session where the response has to be added
 * @return    : UNC_RC_SUCCESS or UNC_UPPL_RC_ERR_* is returned.
 * */
UncRespCode Kt_State_Base::Create(OdbcmConnectionHandler *db_conn,
                                     uint32_t session_id,
                                     uint32_t configuration_id,
                                     void* key_struct,
                                     void* val_struct,
                                     uint32_t data_type,
                                     uint32_t key_type,
                                     ServerSession &sess) {
  pfc_log_error("Create not allowed from VTN");
  UncRespCode create_status = UNC_RC_SUCCESS;

  // Populate the response to be sent in ServerSession
  physical_response_header rsh = {session_id,
      configuration_id,
      UNC_OP_CREATE,
      0,
      0,
      0,
      data_type,
      UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED};

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
    case UNC_KT_PORT:
    case UNC_KT_PORT_NEIGHBOR: {
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
    create_status = UNC_UPPL_RC_ERR_IPC_WRITE_ERROR;
  } else {
    create_status = UNC_RC_SUCCESS;
  }
  return create_status;
}

/** CreateKeyInstance
 * @Description : This function creates a new row of KT in
 * candidate  table.
 * @param[in]:key_struct - the key for the new kt  instance
 * value_struct - the values for the new kt  instance
 * data_type - UNC_DT_* , Create only allowed in running/state/import
 * key_type-UNC_KT_*,any value of unc_key_type_t
 * @return    : UNC_RC_SUCCESS is returned when the create is success
 * UNC_UPPL_RC_ERR_* is returned when ipc response could not be added to sess.
 * */
UncRespCode Kt_State_Base::CreateKeyInstance(OdbcmConnectionHandler *db_conn,
                                                void* key_struct,
                                                void* val_struct,
                                                uint32_t data_type,
                                                uint32_t key_type) {
  UncRespCode create_status = UNC_RC_SUCCESS;
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();

  pfc_log_debug("Create instance of Kt: %d", key_type);
  // Check whether operation is allowed on the given DT_TYPE
  if (((unc_keytype_datatype_t)data_type != UNC_DT_STATE) &&
      ((unc_keytype_datatype_t)data_type != UNC_DT_IMPORT)) {
    pfc_log_error("Create operation is provided on unsupported data type %d",
                  data_type);
    return UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED;
  }

  // Structure used to send request to ODBC
  DBTableSchema kt_dbtableschema;
  void* old_value_struct = NULL;
  vector<ODBCMOperator> vect_key_operations;
  // Create DBSchema structure for kt_table
  PopulateDBSchemaForKtTable(db_conn, kt_dbtableschema,
                             key_struct,
                             val_struct,
                             UNC_OP_CREATE, data_type, 0, 0,
                             vect_key_operations, old_value_struct,
                             NOTAPPLIED, false, PFC_FALSE);

  // Send request to ODBC for kt_table create
  ODBCM_RC_STATUS create_db_status = ODBCM_RC_SUCCESS;
  if (key_type == UNC_KT_PORT_NEIGHBOR) {
    create_db_status = physical_layer->get_odbc_manager()->
         UpdateOneRow((unc_keytype_datatype_t)data_type,
                                kt_dbtableschema, db_conn, true);
  } else {
    create_db_status = physical_layer->get_odbc_manager()->
      CreateOneRow((unc_keytype_datatype_t)data_type,
                   kt_dbtableschema, db_conn);
  }
  if (create_db_status != ODBCM_RC_SUCCESS) {
    if (create_db_status == ODBCM_RC_CONNECTION_ERROR) {
      // log fatal error to log daemon
      UPPL_LOG_FATAL("DB connection not available or cannot access DB");
      create_status = UNC_UPPL_RC_ERR_DB_ACCESS;
    } else {
      // log error to log daemon
      pfc_log_error("Create operation has failed");
      create_status = UNC_UPPL_RC_ERR_DB_CREATE;
    }
  } else {
    pfc_log_info("Create of a kt %d in data_type %d is success", key_type,
                 data_type);
  }
  // Checking CREATE operation of LOGICAL_PORT if port_type is UPPL_LP_SWITCH
  if (key_type == UNC_KT_LOGICAL_PORT) {
      Kt_LogicalPort logical_port;
      UncRespCode status = UNC_RC_SUCCESS;
      status = logical_port.UpdateDomainNameForTP(db_conn, key_struct,
                                     val_struct, data_type, key_type);
      if (status != UNC_RC_SUCCESS) {  //  to avoid inconsistency in DB
        pfc_log_error("update domain for TP is FAILED, INVESTIGATE");
        return  status;
      }
  }
  return create_status;
}

/** Update
 * @Description : This function updates a row of KT in
 * candidate  table.
 * @param[in] : session_id - ipc session id used for TC validation
 * configuration_id - configuration id used for TC validation
 * key_struct - the key for the kt  instance
 * value_struct - the values for the kt  instance
 * data_type - UNC_DT_* , Update only allowed in running/state/import
 * key_type-UNC_KT_*,any value of unc_key_type_t
 * sess - ipc server session where the response has to be added
 * @return    : UNC_RC_SUCCESS is returned when the response
 * is added to ipc session successfully.
 * UNC_UPPL_RC_ERR_* is returned when ipc response could not be added to sess.
 * */
UncRespCode Kt_State_Base::Update(OdbcmConnectionHandler *db_conn,
                                     uint32_t session_id,
                                     uint32_t configuration_id,
                                     void* key_struct,
                                     void* val_struct,
                                     uint32_t data_type,
                                     uint32_t key_type,
                                     ServerSession &sess) {
  pfc_log_error("Update not allowed from VTN");
  UncRespCode update_status = UNC_RC_SUCCESS;

  // Populate the response to be sent in ServerSession
  physical_response_header rsh = {session_id,
      configuration_id,
      UNC_OP_UPDATE,
      0,
      0,
      0,
      data_type,
      UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED};

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
    case UNC_KT_PORT:
    case UNC_KT_PORT_NEIGHBOR: {
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
    update_status = UNC_UPPL_RC_ERR_IPC_WRITE_ERROR;
  } else {
    update_status = UNC_RC_SUCCESS;
  }
  return update_status;
}

/** UpdateKeyInstance
 * @Description : This function updates a row of KT in
 * candidate  table.
 * @param[in] :
 * key_struct - the key for the new kt instance
 * value_struct - the values for the new kt instance
 * data_type - UNC_DT_* , update only allowed in running/state/import
 * key_type-UNC_KT_*,any value of unc_key_type_t
 * old_val_struct-void * to switch value structure
 * @return    : UNC_RC_SUCCESS is returned when the update
 * is done successfully.
 * UNC_UPPL_RC_ERR_* is returned when the update is error
 * */
UncRespCode Kt_State_Base::UpdateKeyInstance(OdbcmConnectionHandler *db_conn,
                                                void* key_struct,
                                                void* val_struct,
                                                uint32_t data_type,
                                                uint32_t key_type,
                                                void* &old_val_struct) {
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  UncRespCode update_status = UNC_RC_SUCCESS;
  // Check whether operation is allowed on the given DT type
  if (((unc_keytype_datatype_t)data_type != UNC_DT_STATE) &&
      ((unc_keytype_datatype_t)data_type != UNC_DT_IMPORT)) {
    pfc_log_error("Update operation is provided on unsupported data type %d",
                  data_type);
    update_status = UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED;
    return update_status;
  }
  pfc_bool_t is_state = PFC_FALSE;
  if (key_type == UNC_KT_CTR_DOMAIN) {
    is_state = PFC_TRUE;
  }
  // Structure used to send request to ODBC
  DBTableSchema kt_dbtableschema;
  vector<ODBCMOperator> vect_key_operations;
  // Create DBSchema structure for table
  PopulateDBSchemaForKtTable(db_conn, kt_dbtableschema,
                             key_struct,
                             val_struct,
                             UNC_OP_UPDATE, data_type, 0, 0,
                             vect_key_operations, old_val_struct,
                             NOTAPPLIED, false, is_state);
  if (!((kt_dbtableschema.get_row_list()).empty())) {
    // Send request to ODBC for kt_table update
    ODBCM_RC_STATUS update_db_status = physical_layer->get_odbc_manager()-> \
        UpdateOneRow((unc_keytype_datatype_t)data_type,
                     kt_dbtableschema, db_conn, false);
    if (update_db_status == ODBCM_RC_CONNECTION_ERROR) {
      // log fatal error to log daemon
      UPPL_LOG_FATAL("DB connection not available or cannot access DB");
      update_status = UNC_UPPL_RC_ERR_DB_ACCESS;
    } else if (update_db_status != ODBCM_RC_SUCCESS) {
      // log error to log daemon
      pfc_log_error("UpdateOneRow error response from DB is %d",
                    update_db_status);
      update_status = UNC_UPPL_RC_ERR_DB_UPDATE;
    } else {
      pfc_log_info("Update of kt in data type %d is success",
                   data_type);
    }
  }
  return update_status;
}

/**Delete
 * @Description : This function deletes a row of KT in
 * candidate  table.
 * @param[in] : session_id - ipc session id used for TC validation
 * configuration_id - configuration id used for TC validation
 * key_struct - the key for the kt instance
 * data_type - UNC_DT_* , delete only allowed in running/state/import,
 * key_type-UNC_KT_*,any value of unc_key_type_t
 * sess - ipc server session where the response has to be added
 * @return    : UNC_RC_SUCCESS is returned when the response
 * is added to ipc session successfully.
 * UNC_UPPL_RC_ERR_* is returned when ipc response could not be added to sess.
 * */
UncRespCode Kt_State_Base::Delete(OdbcmConnectionHandler *db_conn,
                                     uint32_t session_id,
                                     uint32_t configuration_id,
                                     void* key_struct,
                                     uint32_t data_type,
                                     uint32_t key_type,
                                     ServerSession &sess) {
  pfc_log_error("Delete not allowed from VTN");
  UncRespCode delete_status = UNC_RC_SUCCESS;

  // Populate the response to be sent in ServerSession
  physical_response_header rsh = {session_id,
      configuration_id,
      UNC_OP_DELETE,
      0,
      0,
      0,
      data_type,
      UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED};

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
    case UNC_KT_PORT:
    case UNC_KT_PORT_NEIGHBOR: {
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
    delete_status = UNC_UPPL_RC_ERR_IPC_WRITE_ERROR;
  } else {
    delete_status = UNC_RC_SUCCESS;
  }
  return delete_status;
}

/** HandleDriverEvents
 * @Description : This function processes the notification received from
 * driver
 * @param[in] : key_struct - specifies the key instance of KT
 * old_value_struct - old value of KT
 * new_value_struct - new value of KT
 * oper_type-UNC_OP_*,type of operation
 * data_type-UNC_DT_*,type of database
 * key_type-UNC_KT_*,any value of unc_key_type_t
 * @return    : UNC_RC_SUCCESS if events are handled successfully or
 * UNC_UPPL_RC_ERR*
 * */
UncRespCode Kt_State_Base::HandleDriverEvents(
    OdbcmConnectionHandler *db_conn,
    void* key_struct,
    uint32_t oper_type,
    uint32_t data_type,
    uint32_t key_type,
    void* old_val_struct,
    void* new_val_struct) {
  UncRespCode status = UNC_RC_SUCCESS;
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  string controller_name = "";
  string event_details = "";  // for sending event handling alarm
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
      UncRespCode retcode = PhyUtil::get_controller_type(
          db_conn, controller_name, controller_type,
          (unc_keytype_datatype_t)parent_data_type);
      // Check whether operation is allowed on the given DT type
      if (retcode != UNC_RC_SUCCESS) {
        pfc_log_error("Error getting the controller type");
        return UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED;
      } else if (retcode == UNC_RC_SUCCESS &&
          controller_type == UNC_CT_UNKNOWN) {
        pfc_log_error("Unknown domain cannot be operated from driver");
        return UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED;
      }
      event_details = "KT_CTR_DOMAIN";
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
    case UNC_KT_PORT_NEIGHBOR: {
      key_port_t *obj_key = reinterpret_cast<key_port_t*>(key_struct);
      controller_name = reinterpret_cast<const char*>(
          obj_key->sw_key.ctr_key.controller_name);
      event_details = "KT_PORT(neighbor)";
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
  status = ValidateRequest(db_conn, key_struct, new_val_struct,
                           oper_type, data_type, key_type);
  if (status != UNC_RC_SUCCESS) {
    pfc_log_debug(
        "HandleDriverEvents validation failed with %d "
        "for operation %d with data type %d", status, oper_type, data_type);
    if ((oper_type == UNC_OP_CREATE && status !=
         UNC_UPPL_RC_ERR_INSTANCE_EXISTS) ||
         ((oper_type == UNC_OP_UPDATE ||
           oper_type == UNC_OP_DELETE) &&
           status != UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE)) {
      PhysicalLayer::ctr_oprn_mutex_.lock();
      map<string, CtrOprnStatus> :: iterator it;
      it = PhysicalLayer::ctr_oprn_status_.find(controller_name);
      if (it != PhysicalLayer::ctr_oprn_status_.end()) {
        if (it->second.EventsStartReceived == false ||
                       it->second.IsIPChanged == true) {
          pfc_log_debug("Alarm ignored,due to EventsStart/IPchanged flag");
          PhysicalLayer::ctr_oprn_mutex_.unlock();
          return UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED;
        }
      }
      PhysicalLayer::ctr_oprn_mutex_.unlock();
      pfc_log_info(
        "HandleDriverEvents validation failed with %d "
        "for operation %d with data type %d", status, oper_type, data_type);
      // Raise validation failure alarm
      UncRespCode alarm_status = physical_layer->get_physical_core()->
          RaiseEventHandlingAlarm(controller_name);

      if (alarm_status == UNC_RC_SUCCESS) {
        // Checking the presence of the controller in running database
        Kt_Controller kt_ctr;
        vector<string> vect_ctr_key_value;
        vect_ctr_key_value.push_back(controller_name);
        UncRespCode key_exist_running = kt_ctr.IsKeyExists(
               db_conn, UNC_DT_RUNNING,
               vect_ctr_key_value);

        if (key_exist_running == UNC_RC_SUCCESS) {
        // Sending event handling failure alarm
        alarm_status = physical_layer->get_physical_core()->
            SendEventHandlingFailureAlarm(controller_name,
                                          event_details);
        pfc_log_debug(
            "Event Handling Validation Failure alarm sent - status %d",
             alarm_status);
        } else {
        pfc_log_info(
            "Event Handling Validation Failure alarm not sent - status %d",
            key_exist_running);
        }
      }
    }
    return status;
  }

  // Call CreateKeyInstance, UpdateKeyInstance and DeleteKeyInstance based
  // on operation type
  // Create the kt instance in DB
  void* old_value_struct = NULL;
  if (oper_type == UNC_OP_CREATE) {
    pfc_log_debug("Call CreateKeyInstance to create kt in DB");
    status = CreateKeyInstance(db_conn, key_struct, new_val_struct,
                               data_type, key_type);
  } else if (oper_type == UNC_OP_UPDATE) {
    if (key_type == UNC_KT_LOGICAL_PORT ||
        key_type == UNC_KT_LOGICAL_MEMBER_PORT) {
      pfc_log_error("Update operation is provided on unsupported key type %d",
                    key_type);
      status = UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED;
      return status;
    }
    // Call UpdateKeyInstance to update kt value in DB
    pfc_log_debug("Call UpdateKeyInstance to update kt in DB");
    status = UpdateKeyInstance(db_conn, key_struct, new_val_struct,
                               data_type, key_type,
                               old_value_struct);
  } else if (oper_type == UNC_OP_DELETE) {
    // Call DeleteKeyInstance to Delete kt from DB
    pfc_log_debug("Call DeleteKeyInstance to delete kt in DB");
    status = DeleteKeyInstance(db_conn, key_struct, data_type, key_type);
  }
  if (status != UNC_RC_SUCCESS) {
    UncRespCode alarm_status = physical_layer->get_physical_core()->
        RaiseEventHandlingAlarm(controller_name);
    if (alarm_status == UNC_RC_SUCCESS) {
      // Checking the presence of the controller in running database
      Kt_Controller kt_ctr;
      vector<string> vect_ctr_key_value;
      vect_ctr_key_value.push_back(controller_name);
      UncRespCode key_exist_running = kt_ctr.IsKeyExists(
             db_conn, UNC_DT_RUNNING,
             vect_ctr_key_value);
      if (key_exist_running == UNC_RC_SUCCESS) {
        alarm_status = physical_layer->get_physical_core()->
          SendEventHandlingFailureAlarm(controller_name,
                                        event_details);
        pfc_log_debug("Failure alarm sent to node manager - status %d",
                   alarm_status);
      } else {
        pfc_log_info("Failure alarm not sent to node manager - status %d",
                   key_exist_running);
      }
    }
  } else {
    UncRespCode alarm_status = physical_layer->get_physical_core()->
      ClearEventHandlingAlarm(controller_name);
    if (alarm_status == UNC_RC_SUCCESS) {
      alarm_status = physical_layer->get_physical_core()->
          SendEventHandlingSuccessAlarm(controller_name,
                                    event_details);
    }
    if (key_type != UNC_KT_PORT_NEIGHBOR) {
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
          status = UNC_UPPL_RC_ERR_IPC_WRITE_ERROR;
        } else {
        // Call IPC server to post the event
          status = (UncRespCode) physical_layer
              ->get_ipc_connection_manager()->SendEvent(&ser_evt,
                                 controller_name, event_type);
        }
      }
      // Perform for oper_status handling
      UncRespCode oper_status_handle = HandleOperStatus(db_conn, key_struct,
                                                       oper_type,
                                                       data_type,
                                                       key_type,
                                                       new_val_struct,
                                                       controller_name);
      pfc_log_debug("Oper Status Handling Status %d", oper_status_handle);
    }
  }
  // Old value structure memory clean up
  if (oper_type == UNC_OP_UPDATE) {
    if (old_value_struct != NULL) {
      ClearValueStructure(key_type,
                          old_value_struct);
      old_value_struct = NULL;
    }
  }
  return status;
}

/** HandleOperStatus
 * @Description : This function calls oper status handling of associated
 * key types
 * @param[in] : key_struct - specifies the key instance of KT
 * new_value_struct - new value of KT
 * oper_type-UNC_OP_*,type of operation
 * data_type-UNC_DT_*,type of database
 * key_type-UNC_KT_*,any value of unc_key_type_t
 * controller_name-controller id
 * @return    : UNC_RC_SUCCESS if oper status changes are handled
 *  successfully or UNC_UPPL_RC_ERR*
 * */
UncRespCode Kt_State_Base::HandleOperStatus(OdbcmConnectionHandler *db_conn,
                                               void* key_struct,
                                               uint32_t oper_type,
                                               uint32_t data_type,
                                               uint32_t key_type,
                                               void* new_val_struct,
                                               string controller_name) {
  UncRespCode oper_return = UNC_RC_SUCCESS;
  // logical port oper status handling
  if (key_type == UNC_KT_LOGICAL_MEMBER_PORT &&
      (oper_type == UNC_OP_CREATE || oper_type == UNC_OP_DELETE)) {
    key_logical_member_port_t *obj_key =
        reinterpret_cast<key_logical_member_port_t*>(key_struct);
    string domain_name = reinterpret_cast<const char*>(
        obj_key->logical_port_key.domain_key.domain_name);
    string port_id = reinterpret_cast<const char*>(obj_key
        ->logical_port_key.port_id);

    key_logical_port logical_port_key;
    memset(&logical_port_key, '\0', sizeof(key_logical_port));
    memcpy(logical_port_key.domain_key.ctr_key.controller_name,
           controller_name.c_str(),
           controller_name.length()+1);
    memcpy(logical_port_key.domain_key.domain_name,
           domain_name.c_str(),
           domain_name.length()+1);
    memcpy(logical_port_key.port_id,
           port_id.c_str(),
           port_id.length()+1);
    vector<OperStatusHolder> ref_oper_status;
    GET_ADD_CTRL_OPER_STATUS(controller_name, ref_oper_status,
                             data_type, db_conn);
    Kt_LogicalPort logical_port;
    oper_return = logical_port.HandleOperStatus(
        db_conn,
        data_type,
        reinterpret_cast<void *> (&logical_port_key),
        NULL,
        ref_oper_status, UNC_KT_LOGICAL_MEMBER_PORT);
    pfc_log_debug("HandleOperStatus for logical_port class %d",
                  oper_return);
    ClearOperStatusHolder(ref_oper_status);
  } else if (key_type == UNC_KT_PORT && oper_type == UNC_OP_UPDATE) {
    // Port oper status changes
    Kt_Port kt_port;
    val_port_st_t port_st = *(reinterpret_cast<val_port_st_t*>(new_val_struct));
    unsigned int valid_val =
        PhyUtil::uint8touint(port_st.valid[kIdxPortOperStatus]);
    if (valid_val == UNC_VF_VALID) {
      key_port_t port_key = *(reinterpret_cast<key_port_t*>(key_struct));
      vector<OperStatusHolder> ref_oper_status;
      GET_ADD_CTRL_OPER_STATUS(controller_name, ref_oper_status,
                              data_type, db_conn);
      // Get Switch oper status
      key_switch_t switch_key;
      Kt_Switch kt_switch;
      memcpy(switch_key.ctr_key.controller_name, controller_name.c_str(),
             (controller_name.length())+1);
      memcpy(switch_key.switch_id, port_key.sw_key.switch_id,
             sizeof(port_key.sw_key.switch_id));
      uint8_t switch_oper_status = 0;
      read_status = kt_switch.GetOperStatus(
          db_conn, data_type,
          reinterpret_cast<void*>(&switch_key), switch_oper_status);
      pfc_log_debug("Switch read_status %d, oper_status %d",
                    read_status, switch_oper_status);
      ADD_SWITCH_OPER_STATUS(switch_key, switch_oper_status, ref_oper_status);
      // Get port oper status
      uint8_t port_oper_status = 0;
      read_status = kt_port.GetOperStatus(
          db_conn, data_type,
          key_struct, port_oper_status);
      pfc_log_debug("Port read_status %d, oper_status %d",
                    read_status, port_oper_status);
      ADD_PORT_OPER_STATUS(
          port_key, port_oper_status, ref_oper_status);
      oper_return = kt_port.NotifyOperStatus(db_conn, data_type,
                                             key_struct,
                                             new_val_struct,
                                             ref_oper_status);
      pfc_log_debug("Port notify oper status return %d", oper_return);
      ClearOperStatusHolder(ref_oper_status);
    }
  } else if (key_type == UNC_KT_SWITCH && oper_type == UNC_OP_UPDATE) {
    // Switch oper status changes
    Kt_Switch kt_switch;
    val_switch_st_t switch_st =
        *(reinterpret_cast<val_switch_st_t*>(new_val_struct));
    unsigned int valid_val =
        PhyUtil::uint8touint(switch_st.valid[kIdxSwitchOperStatus]);
    if (valid_val == UNC_VF_VALID) {
      key_switch_t switch_key =
          *(reinterpret_cast<key_switch_t*>(key_struct));
      vector<OperStatusHolder> ref_oper_status;
      GET_ADD_CTRL_OPER_STATUS(controller_name, ref_oper_status,
                              data_type, db_conn);
      // Get Switch oper status
      uint8_t switch_oper_status = 0;
      read_status = kt_switch.GetOperStatus(
          db_conn, data_type,
          key_struct, switch_oper_status);
      pfc_log_debug("Switch read_status %d, oper_status %d",
                    read_status, switch_oper_status);
      ADD_SWITCH_OPER_STATUS(switch_key,
                             switch_oper_status, ref_oper_status);
      oper_return = kt_switch.NotifyOperStatus(db_conn, data_type,
                                               key_struct,
                                               new_val_struct,
                                               ref_oper_status);
      pfc_log_debug("Switch notify oper status return %d", oper_return);
      ClearOperStatusHolder(ref_oper_status);
    }
  } else if (key_type == UNC_KT_LINK && oper_type == UNC_OP_UPDATE) {
  // Link oper status changes
    Kt_Link kt_link;
    val_link_st_t link_st =
        *(reinterpret_cast<val_link_st_t*>(new_val_struct));
    unsigned int valid_val =
        PhyUtil::uint8touint(link_st.valid[kIdxLinkStOperStatus]);
    if (valid_val == UNC_VF_VALID) {
      oper_return = kt_link.NotifyOperStatus(db_conn, data_type,
                                             key_struct,
                                             new_val_struct);
      pfc_log_debug("Link notify oper status return %d", oper_return);
    }
  }
  return oper_return;
}
