/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/**
 * @brief   KT LogicalMemberPort implementation
 * @file    itc_kt_logical_member_port.cc
 *
 */

#include "itc_kt_logical_member_port.hh"
#include "itc_kt_logicalport.hh"
using unc::uppl::PhysicalLayer;
/** Constructor
 * * @Description : This function initializes member variables
 * and fills the attribute syntax map used for validation
 * * * @param[in] : None
 * * * @return    : None
 * */
Kt_LogicalMemberPort::Kt_LogicalMemberPort() {
  parent = NULL;
  Fill_Attr_Syntax_Map();
}

/** Destructor
 * * @Description : This function frees the parent and child key types
 * instances for kt_logical_member_port
 * * * @param[in] : None
 * * * @return    : None
 * */
Kt_LogicalMemberPort::~Kt_LogicalMemberPort() {
  // Delete parent object
  if (parent != NULL) {
    delete parent;
    parent = NULL;
  }
}

/**DeleteKeyInstance
 * * @Description : This function deletes a row of KT_LogicalMemberPort in
 * state logicalmemberport table.
 * * @param[in] :
 * key_struct - the key for the new kt logicalmemberport instance
 * data_type - UNC_DT_* , delete only allowed in state
 * * @return    : UPPL_RC_SUCCESS is returned when the delete
 * is done successfully.
 * UPPL_RC_ERR_* is returned when the delete is error
 * */
UpplReturnCode Kt_LogicalMemberPort::DeleteKeyInstance(void* key_struct,
                                                       uint32_t data_type,
                                                       uint32_t key_type) {
  UpplReturnCode delete_status = UPPL_RC_SUCCESS;
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  // Check whether operation is allowed on the given DT type
  if (((unc_keytype_datatype_t)data_type != UNC_DT_STATE) &&
      ((unc_keytype_datatype_t)data_type != UNC_DT_IMPORT)) {
    pfc_log_error("Delete operation is provided on unsupported data type");
    return UPPL_RC_ERR_OPERATION_NOT_ALLOWED;
  }

  key_logical_member_port_t *obj_key_logical_member_port =
      reinterpret_cast<key_logical_member_port_t*>(key_struct);
  string switch_id = reinterpret_cast<const char*>(obj_key_logical_member_port
      ->switch_id);
  string physical_port_id = reinterpret_cast<const char*>(
      obj_key_logical_member_port->physical_port_id);
  string port_id = reinterpret_cast<const char*>(obj_key_logical_member_port
      ->logical_port_key.port_id);
  string domain_name = reinterpret_cast<const char*>(
      obj_key_logical_member_port->logical_port_key.domain_key.domain_name);
  string controller_name = reinterpret_cast<const char*>(
      obj_key_logical_member_port
      ->logical_port_key.domain_key.ctr_key.controller_name);

  // Structure used to send request to ODBC
  DBTableSchema kt_logical_member_port_dbtableschema;

  // Construct Primary key list
  vector<string> vect_prim_keys;
  if (!controller_name.empty())
    vect_prim_keys.push_back(CTR_NAME);
  if (!domain_name.empty())
    vect_prim_keys.push_back(DOMAIN_NAME);
  if (!port_id.empty())
    vect_prim_keys.push_back(LMP_LP_PORT_ID);
  if (!switch_id.empty())
    vect_prim_keys.push_back(LMP_SWITCH_ID);
  if (!physical_port_id.empty())
    vect_prim_keys.push_back(LMP_PHYSICAL_PORT_ID);
  // Construct TableAttrSchema structure
  // TableAttrSchema holds table_name, primary key, attr_name
  vector<TableAttrSchema> vect_table_attr_schema;
  list< vector<TableAttrSchema> > row_list;

  // controller_name
  PhyUtil::FillDbSchema(CTR_NAME, controller_name,
                        controller_name.length(), DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);
  // domain_name
  PhyUtil::FillDbSchema(DOMAIN_NAME, domain_name,
                        domain_name.length(), DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);
  // port_id
  PhyUtil::FillDbSchema(LMP_LP_PORT_ID, port_id,
                        port_id.length(), DATATYPE_UINT8_ARRAY_320,
                        vect_table_attr_schema);
  // switch_id
  PhyUtil::FillDbSchema(LMP_SWITCH_ID, switch_id,
                        switch_id.length(), DATATYPE_UINT8_ARRAY_256,
                        vect_table_attr_schema);
  // physical_port_id
  PhyUtil::FillDbSchema(LMP_PHYSICAL_PORT_ID, physical_port_id,
                        physical_port_id.length(),
                        DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);
  // Send request to ODBC for logical_member_port_table delete
  kt_logical_member_port_dbtableschema.set_table_name(
      UPPL_LOGICAL_MEMBER_PORT_TABLE);
  kt_logical_member_port_dbtableschema.set_primary_keys(vect_prim_keys);
  row_list.push_back(vect_table_attr_schema);
  kt_logical_member_port_dbtableschema.set_row_list(row_list);

  ODBCM_RC_STATUS delete_db_status = physical_layer->get_odbc_manager()-> \
      DeleteOneRow((unc_keytype_datatype_t)data_type,
                   kt_logical_member_port_dbtableschema);
  if (delete_db_status != ODBCM_RC_SUCCESS) {
    if (delete_db_status == ODBCM_RC_CONNECTION_ERROR) {
      // log fatal error to log daemon
      pfc_log_fatal("DB connection not available or cannot access DB");
      delete_status = UPPL_RC_ERR_DB_ACCESS;
    } else if (delete_db_status == ODBCM_RC_ROW_NOT_EXISTS) {
      delete_status = UPPL_RC_ERR_NO_SUCH_INSTANCE;
    } else {
      // log error to log daemon
      pfc_log_error("DeleteOneRow error response from DB is %d",
                    delete_db_status);
      delete_status = UPPL_RC_ERR_DB_DELETE;
    }
  } else {
    pfc_log_info("Delete of a logical member port in data_type(%d) is success",
                 data_type);
  }
  return delete_status;
}

/** ReadBulk
 * * @Description : This function reads the max_rep_ct number of instances of
 *                  the KT_LogicalMemberPort
 *  Order of ReadBulk response
 *  val_ctr -> val_ctr_domain -> val_logical_port ->
 *  val_logical_member_port -> val_switch ->  val_port ->
 *  val_link -> val_boundary
 * * * @param[in] :
 * key_struct - the key for the kt logicalmemberport instance
 * data_type - UNC_DT_* , read allowed in state
 * option1/option2 - specifies any additional condition for read operation
 * max_rep_ct - specifies number of rows to be returned
 * parent_call - indicates whether parent has called this readbulk
 * is_read_next - indicates whether this function is invoked from readnext
 * * @return    : UPPL_RC_SUCCESS is returned when the response
 * is added to ipc session successfully.
 * UPPL_RC_ERR_* is returned when ipc response could not be added to sess.
 * */

UpplReturnCode Kt_LogicalMemberPort::ReadBulk(void* key_struct,
                                              uint32_t data_type,
                                              uint32_t option1,
                                              uint32_t option2,
                                              uint32_t &max_rep_ct,
                                              int child_index,
                                              pfc_bool_t parent_call,
                                              pfc_bool_t is_read_next) {
  UpplReturnCode read_status = UPPL_RC_SUCCESS;
  key_logical_member_port_t* obj_key_logical_member_port =
      reinterpret_cast<key_logical_member_port_t*>(key_struct);
  string switch_id =
      reinterpret_cast<const char*> (obj_key_logical_member_port->switch_id);
  string physical_port_id =
      reinterpret_cast<const char*>(obj_key_logical_member_port
          ->physical_port_id);
  string port_id = reinterpret_cast<const char*>(obj_key_logical_member_port
      ->logical_port_key.port_id);
  string domain_name = reinterpret_cast<const char*>(obj_key_logical_member_port
      ->logical_port_key.domain_key.domain_name);
  string controller_name = reinterpret_cast<const char*>(
      obj_key_logical_member_port
      ->logical_port_key.domain_key.ctr_key.controller_name);
  if ((unc_keytype_datatype_t)data_type != UNC_DT_STATE) {
    // Not supported
    pfc_log_debug("ReadBulk operation is not allowed in %d data type",
                  data_type);
    read_status = UPPL_RC_ERR_OPERATION_NOT_ALLOWED;
    pfc_log_debug("read_status=%d", read_status);
    return read_status;
  }
  if (max_rep_ct == 0) {
    return UPPL_RC_SUCCESS;
  }
  vector<key_logical_member_port_t> vect_logical_mem_port;
  // Read the controller values based on given key structure
  read_status = ReadBulkInternal(key_struct,
                                 data_type,
                                 max_rep_ct,
                                 vect_logical_mem_port);

  pfc_log_debug("read_status from _logical_member_port is %d", read_status);
  if (read_status == UPPL_RC_SUCCESS) {
    PhysicalCore *physical_core = PhysicalLayer::get_instance()->
        get_physical_core();
    InternalTransactionCoordinator *itc_trans  =
        physical_core->get_internal_transaction_coordinator();
    vector<key_logical_member_port_t> ::iterator logical_mem_port_iter =
        vect_logical_mem_port.begin();
    for (; logical_mem_port_iter != vect_logical_mem_port.end();
        ++logical_mem_port_iter) {
      pfc_log_debug("Iterating entries...");
      pfc_log_debug("Adding member_port with switch_id '%s' & port_id '%s'",
                    reinterpret_cast<const char*>
      ((*logical_mem_port_iter).switch_id),
      reinterpret_cast<const char*>((*logical_mem_port_iter).
          logical_port_key.port_id));
      key_logical_member_port_t *key_buffer = new key_logical_member_port_t
          (*logical_mem_port_iter);
      BulkReadBuffer obj_key_buffer = {
          UNC_KT_LOGICAL_MEMBER_PORT, IS_KEY,
          reinterpret_cast<void *>(key_buffer)
      };
      itc_trans->AddToBuffer(obj_key_buffer);
      BulkReadBuffer obj_sep_buffer = {
          UNC_KT_LOGICAL_MEMBER_PORT, IS_SEPARATOR, NULL
      };
      itc_trans->AddToBuffer(obj_sep_buffer);
      --max_rep_ct;
      if (max_rep_ct == 0) {
        pfc_log_debug("max_rep_ct reached zero, so returning");
        return read_status;
      }
    }
  }
  if (max_rep_ct > 0 && parent_call == false) {
    pfc_log_debug("max_rep_ct is %d and parent_call is %d, calling parent",
                  max_rep_ct, parent_call);
    Kt_LogicalPort nextKin;
    key_logical_port_t nextkin_key_struct;
    memcpy(reinterpret_cast<char*>(nextkin_key_struct.port_id),
           port_id.c_str(),
           port_id.length()+1);
    memcpy(
        reinterpret_cast<char*>(nextkin_key_struct.domain_key.domain_name),
        domain_name.c_str(),
        domain_name.length()+1);
    memcpy(
        reinterpret_cast<char*>
    (nextkin_key_struct.domain_key.ctr_key.controller_name),
    controller_name.c_str(),
    controller_name.length()+1);
    read_status = nextKin.ReadBulk(
        reinterpret_cast<void *>(&nextkin_key_struct),
        data_type,
        option1,
        option2,
        max_rep_ct,
        0,
        false,
        is_read_next);
    pfc_log_debug("read status from next kin Kt_LogicalPort is %d",
                  read_status);
    return UPPL_RC_SUCCESS;
  }
  pfc_log_debug("log_mem_port reached end of table");
  pfc_log_debug("read_status=%d", read_status);
  if (read_status == UPPL_RC_ERR_NO_SUCH_INSTANCE) {
    read_status = UPPL_RC_SUCCESS;
  }
  return read_status;
}

/**ReadBulkInternal
 * * @Description : This function reads bulk rows of KT_LogicalMemberPort in
 *  logicalmemberport table of specified data type.
 * * @param[in] :
 * key_struct - the key for the kt logicalmemberport instance
 * val_struct - the value struct for kt_logicalmemberport instance
 * max_rep_ct - specifies number of rows to be returned
 * vect_ctr_id - indicates the fetched contoller names from db
 * * @return    : UPPL_RC_SUCCESS is returned when the response
 * is added to ipc session successfully.
 * UPPL_RC_ERR_* is returned when ipc response could not be added to sess.
 * */
UpplReturnCode Kt_LogicalMemberPort::ReadBulkInternal(
    void* key_struct,
    uint32_t data_type,
    uint32_t max_rep_ct,
    vector<key_logical_member_port_t> &vect_logical_mem_port) {
  if (max_rep_ct <= 0) {
    return UPPL_RC_SUCCESS;
  }
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  UpplReturnCode read_status = UPPL_RC_SUCCESS;
  ODBCM_RC_STATUS read_db_status = ODBCM_RC_SUCCESS;
  DBTableSchema kt_logical_member_port_dbtableschema;

  vector<ODBCMOperator> primary_key_operation_list;
  void *old_val_struct;
  PopulateDBSchemaForKtTable(
      kt_logical_member_port_dbtableschema,
      key_struct, NULL,
      UNC_OP_READ_BULK, 0, 0,
      primary_key_operation_list, old_val_struct);
  uint32_t no_of_query = 1;
  vector<ODBCMOperator>:: iterator iter =
      find(primary_key_operation_list.begin(),
           primary_key_operation_list.end(),
           unc::uppl::MULTIPLE_QUERY);
  if (iter != primary_key_operation_list.end()) {
    // Multiple query to be sent to DB till we get a match
    pfc_log_debug("Multiple query to be sent to DB till we get a match");
    no_of_query = 2;  // LMP has 2 primary keys other than LP
  }
  for (uint32_t index = 0; index < no_of_query; ++index) {
    // Read rows from DB
    read_db_status = physical_layer->get_odbc_manager()-> \
        GetBulkRows((unc_keytype_datatype_t)data_type, max_rep_ct,
                    kt_logical_member_port_dbtableschema,
                    (unc_keytype_operation_t)UNC_OP_READ_BULK);
    if (read_db_status == ODBCM_RC_RECORD_NOT_FOUND) {
      pfc_log_debug("No record found");
      read_status = UPPL_RC_ERR_NO_SUCH_INSTANCE;
      // Update the primary key vector
      kt_logical_member_port_dbtableschema.primary_keys_.pop_back();
      pfc_log_debug(
          "Primary key vector size %d",
          static_cast<int>
      (kt_logical_member_port_dbtableschema.primary_keys_.size()));
      // return read_status;
    } else if (read_db_status == ODBCM_RC_SUCCESS) {
      read_status = UPPL_RC_SUCCESS;
      pfc_log_debug("Received success response from db");
      break;
    } else {
      read_status = UPPL_RC_ERR_DB_GET;
      // log error to log daemon
      pfc_log_error("Read operation has failed");
      // return read_status;
    }
  }
  if (read_status == UPPL_RC_SUCCESS) {
    // From the values received from DB, populate val structure
    FillLogicalMemberPortValueStructure(kt_logical_member_port_dbtableschema,
                                        max_rep_ct,
                                        UNC_OP_READ_BULK,
                                        vect_logical_mem_port);
  }
  return read_status;
}

/** PerformSyntaxValidation
 * * @Description : This function performs syntax validation for
 *  UNC_KT_LOGICAL_MEMBER_PORT
 * * * @param[in]
 * key_struct - the key for the kt logicalmemberport instance
 * value_struct - the value for the kt logicalmemberport instance
 * data_type - UNC_DT_*
 * operation_type - UNC_OP*
 * * @return    : UPPL_RC_SUCCESS is returned when the validation is successful
 * UPPL_RC_ERR_* is returned when validtion is failure
 * */

UpplReturnCode Kt_LogicalMemberPort::PerformSyntaxValidation(
    void* key_struct,
    void* val_struct,
    uint32_t operation,
    uint32_t data_type) {
  pfc_log_info("Syntax Validation of KT_LOGICAL_MEMBER_PORT");
  UpplReturnCode ret_code = UPPL_RC_SUCCESS;
  if (val_struct != NULL) {
    pfc_log_error("KT_LOGICAL_MEMBER_PORT does not have any value structure");
    return UPPL_RC_ERR_CFG_SYNTAX;
  }

  // Validate Key Structure
  pfc_bool_t mandatory = PFC_TRUE;
  key_logical_member_port *key =
      reinterpret_cast<key_logical_member_port_t*>(key_struct);

  string value = reinterpret_cast<char*>(key->switch_id);
  IS_VALID_STRING_KEY(LMP_SWITCH_ID, value, operation, ret_code, mandatory);
  if (ret_code != UPPL_RC_SUCCESS) {
    return UPPL_RC_ERR_CFG_SYNTAX;
  }


  value = reinterpret_cast<char*>(key->physical_port_id);
  IS_VALID_STRING_KEY(LMP_PHYSICAL_PORT_ID, value, operation,
                      ret_code, mandatory);
  if (ret_code != UPPL_RC_SUCCESS) {
    return UPPL_RC_ERR_CFG_SYNTAX;
  }

  value = reinterpret_cast<char*>(key->logical_port_key.port_id);
  IS_VALID_STRING_KEY(LMP_LP_PORT_ID, value, operation,
                      ret_code, mandatory);
  if (ret_code != UPPL_RC_SUCCESS) {
    return UPPL_RC_ERR_CFG_SYNTAX;
  }

  value = reinterpret_cast<char*>
  (key->logical_port_key.domain_key.domain_name);
  IS_VALID_STRING_KEY(DOMAIN_NAME, value, operation,
                      ret_code, mandatory);
  if (ret_code != UPPL_RC_SUCCESS) {
    return UPPL_RC_ERR_CFG_SYNTAX;
  }

  value = reinterpret_cast<char*>(key->
      logical_port_key.domain_key.ctr_key.controller_name);
  IS_VALID_STRING_KEY(CTR_NAME, value, operation,
                      ret_code, mandatory);
  if (ret_code != UPPL_RC_SUCCESS) {
    return UPPL_RC_ERR_CFG_SYNTAX;
  }
  return ret_code;
}

/** PerformSemanticValidation
 * * @Description : This function performs semantic validation
 * for UNC_KT_LOGICAL_MEMBER_PORT
 * * * @param[in] : key_struct - specifies key instance of KTLogicalMemberPort
 * , value_struct - specifies value of KTLOGICAL_MEMBER_PORT
 * operation - UNC_OP*
 * data_type - UNC_DT*
 * * * @return    : UPPL_RC_SUCCESS if semantic valition is successful
 * or UPPL_RC_ERR_* if failed
 * */

UpplReturnCode Kt_LogicalMemberPort::PerformSemanticValidation(
    void* key_struct,
    void* val_struct,
    uint32_t operation,
    uint32_t data_type) {
  pfc_log_debug("Inside PerformSemanticValidation of KT_LOGICAL_MEMBER_PORT");
  UpplReturnCode status = UPPL_RC_SUCCESS;

  key_logical_member_port_t *obj_key_logical_member_port =
      reinterpret_cast<key_logical_member_port_t*>(key_struct);
  string controller_name = reinterpret_cast<const char*>(
      obj_key_logical_member_port
      ->logical_port_key.domain_key.ctr_key.controller_name);
  string domain_name =
      reinterpret_cast<const char*>(obj_key_logical_member_port
          ->logical_port_key.domain_key.domain_name);
  string port_id = reinterpret_cast<const char*>(obj_key_logical_member_port
      ->logical_port_key.port_id);
  string switch_id = reinterpret_cast<const char*>(obj_key_logical_member_port
      ->switch_id);
  string physical_port_id = reinterpret_cast<const char*>(
      obj_key_logical_member_port->physical_port_id);

  vector<string> vect_key_value;
  vect_key_value.push_back(controller_name);
  vect_key_value.push_back(domain_name);
  vect_key_value.push_back(port_id);
  vect_key_value.push_back(switch_id);
  vect_key_value.push_back(physical_port_id);

  UpplReturnCode KeyStatus = IsKeyExists((unc_keytype_datatype_t)data_type,
                                         vect_key_value);
  pfc_log_info("return value of IsKeyExists:%d", KeyStatus);
  // KeyStatus = UPPL_RC_SUCCESS; //to be removed
  if (operation == UNC_OP_UPDATE || operation == UNC_OP_DELETE ||
      operation == UNC_OP_READ) {
    if (KeyStatus == UPPL_RC_ERR_DB_ACCESS) {
      pfc_log_error("DB Access failure");
      status = KeyStatus;
    } else if (KeyStatus != UPPL_RC_SUCCESS) {
      pfc_log_info("LogicalMemberPort key does not exist and"
          " hence read/delete/update operation not allowed");
      status = UPPL_RC_ERR_NO_SUCH_INSTANCE;
    } else {
      pfc_log_info("key instance exist update/del/read operation allowed");
    }
  }
  if (operation == UNC_OP_CREATE) {
    if (KeyStatus == UPPL_RC_SUCCESS) {
      pfc_log_info("LogicalMemberPort key already exist and"
          " hence create operation not allowed");
      status = UPPL_RC_ERR_INSTANCE_EXISTS;
    } else if (KeyStatus == UPPL_RC_ERR_DB_ACCESS) {
      pfc_log_error("DB Access failure");
      status = KeyStatus;
    } else {
      pfc_log_info("key instance not exist create operation allowed");
    }
  }
  if (operation == UNC_OP_CREATE && status == UPPL_RC_SUCCESS) {
    vector<string> parent_vect_key_value;
    parent_vect_key_value.push_back(controller_name);
    parent_vect_key_value.push_back(domain_name);
    parent_vect_key_value.push_back(port_id);
    pfc_log_info("calling KtLogicalPort IsKeyExists function");
    Kt_LogicalPort KtObj;
    UpplReturnCode parent_key_status = KtObj.IsKeyExists(
        (unc_keytype_datatype_t)data_type, parent_vect_key_value);
    pfc_log_debug("Parent IsKeyExists status %d", parent_key_status);
    if (parent_key_status != UPPL_RC_SUCCESS) {
      status = UPPL_RC_ERR_PARENT_DOES_NOT_EXIST;
    }
  }
  pfc_log_debug("status before returning=%d", status);
  return status;
}

/** IsKeyExists
 * * @Description : This function checks whether the
 *  logicalmemberport_id exists in DB
 * * * @param[in] : key_struct
 * * * @return    : Success or associated error code
 * */
UpplReturnCode Kt_LogicalMemberPort::IsKeyExists(
    unc_keytype_datatype_t data_type,
    vector<string> key_values) {
  PhysicalLayer* physical_layer = PhysicalLayer::get_instance();
  UpplReturnCode check_status = UPPL_RC_SUCCESS;

  if (key_values.empty()) {
    // No key given, return failure
    pfc_log_error("No key given. Returning error");
    return UPPL_RC_ERR_BAD_REQUEST;
  }

  string controller_name = key_values[0];
  string domain_name = key_values[1];
  string port_id = key_values[2];
  string switch_id = key_values[3];
  string physical_port_id = key_values[4];

  // Structure used to send request to ODBC
  DBTableSchema kt_logical_member_port_dbtableschema;

  // Construct Primary key list
  vector<string> vect_prim_keys;
  vect_prim_keys.push_back(CTR_NAME);
  vect_prim_keys.push_back(DOMAIN_NAME);
  vect_prim_keys.push_back(LP_PORT_ID);
  vect_prim_keys.push_back(LMP_SWITCH_ID);
  vect_prim_keys.push_back(LMP_PHYSICAL_PORT_ID);
  // Construct TableAttrSchema structure
  // TableAttrSchema holds table_name, primary key, attr_name
  vector<TableAttrSchema> vect_table_attr_schema;
  list< vector<TableAttrSchema> > row_list;

  // controller_name
  PhyUtil::FillDbSchema(CTR_NAME, controller_name,
                        controller_name.length(), DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);

  // domain_name
  PhyUtil::FillDbSchema(DOMAIN_NAME, domain_name,
                        domain_name.length(), DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);

  // port_id
  PhyUtil::FillDbSchema(LP_PORT_ID, port_id,
                        port_id.length(), DATATYPE_UINT8_ARRAY_320,
                        vect_table_attr_schema);
  // switch_id
  PhyUtil::FillDbSchema(LMP_SWITCH_ID, switch_id,
                        switch_id.length(), DATATYPE_UINT8_ARRAY_256,
                        vect_table_attr_schema);
  // physical_port_id
  PhyUtil::FillDbSchema(LMP_PHYSICAL_PORT_ID, physical_port_id,
                        physical_port_id.length(),
                        DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);

  kt_logical_member_port_dbtableschema.set_table_name(
      UPPL_LOGICAL_MEMBER_PORT_TABLE);
  kt_logical_member_port_dbtableschema.set_primary_keys(vect_prim_keys);
  row_list.push_back(vect_table_attr_schema);
  kt_logical_member_port_dbtableschema.set_row_list(row_list);
  //  Send request to ODBC for controlle_common_table
  ODBCM_RC_STATUS check_db_status = physical_layer->get_odbc_manager()->
      IsRowExists(data_type, kt_logical_member_port_dbtableschema);
  if (check_db_status == ODBCM_RC_CONNECTION_ERROR) {
    // log error to log daemon
    pfc_log_error("DB connection not available or cannot access DB");
    check_status = UPPL_RC_ERR_DB_ACCESS;
  } else if (check_db_status == ODBCM_RC_ROW_EXISTS) {
    pfc_log_debug("DB returned success for Row exists");
  } else {
    pfc_log_info("DB Returned failure for IsRowExists");
    check_status = UPPL_RC_ERR_NO_SUCH_INSTANCE;
  }
  pfc_log_debug("check_status = %d", check_status);
  return check_status;
}

/** PopulateDBSchemaForKtTable
 * * @Description : This function populates the DBAttrSchema to be used to
 *                  send request to ODBC
 * * * @param[in] : DBTableSchema, key_struct, operation_type
 * * * @return    : Success or associated error code
 * */
void Kt_LogicalMemberPort::PopulateDBSchemaForKtTable(
    DBTableSchema &kt_dbtableschema,
    void* key_struct,
    void* val_struct,
    uint8_t operation_type,
    uint32_t option1,
    uint32_t option2,
    vector<ODBCMOperator> &vect_key_operations,
    void* &old_value_struct,
    CsRowStatus row_status,
    pfc_bool_t is_filtering,
    pfc_bool_t is_state) {
  // Construct Primary key list
  vector<string> vect_prim_keys;
  // construct TableAttrSchema structure
  // TableAttrSchema holds table_name, primary key, attr_name
  vector<TableAttrSchema> vect_table_attr_schema;
  list < vector<TableAttrSchema> > row_list;

  key_logical_member_port_t *obj_key_logical_member_port =
      reinterpret_cast<key_logical_member_port_t*>(key_struct);

  pfc_log_info("operation: %d", operation_type);

  // controller name
  string controller_name = reinterpret_cast<const char*>(
      obj_key_logical_member_port
      ->logical_port_key.domain_key.ctr_key.controller_name);
  pfc_log_debug("controller_name: %s", controller_name.c_str());
  PhyUtil::FillDbSchema(CTR_NAME, controller_name,
                        controller_name.length(), DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);
  vect_prim_keys.push_back(CTR_NAME);
  vect_key_operations.push_back(unc::uppl::EQUAL);

  // domain name
  string domain_name = reinterpret_cast<const char*>(obj_key_logical_member_port
      ->logical_port_key.domain_key.domain_name);
  pfc_log_debug("domain_name: %s", domain_name.c_str());
  PhyUtil::FillDbSchema(DOMAIN_NAME, domain_name,
                        domain_name.length(), DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);
  if (!domain_name.empty()) {
    vect_prim_keys.push_back(DOMAIN_NAME);
    vect_key_operations.push_back(unc::uppl::EQUAL);
  }

  // port_id
  string port_id = reinterpret_cast<const char*>(obj_key_logical_member_port
      ->logical_port_key.port_id);
  pfc_log_info("port_id: %s", port_id.c_str());
  PhyUtil::FillDbSchema(LMP_LP_PORT_ID, port_id,
                        port_id.length(), DATATYPE_UINT8_ARRAY_320,
                        vect_table_attr_schema);
  if (!port_id.empty()) {
    vect_prim_keys.push_back(LMP_LP_PORT_ID);
    vect_key_operations.push_back(unc::uppl::EQUAL);
  }

  // switch_id
  string switch_id = reinterpret_cast<const char*> (obj_key_logical_member_port
      ->switch_id);
  // only parent key has to be considered for read_sibling_begin
  if (operation_type == UNC_OP_READ_SIBLING_BEGIN ||
      operation_type == UNC_OP_READ_SIBLING_COUNT) {
    vect_prim_keys.push_back(LMP_SWITCH_ID);
    vect_key_operations.push_back(unc::uppl::GREATER);
    switch_id = "";
  } else if (!switch_id.empty()) {
    vect_prim_keys.push_back(LMP_SWITCH_ID);
    vect_key_operations.push_back(unc::uppl::EQUAL);
  }
  pfc_log_info("switch_id: %s ", switch_id.c_str());
  PhyUtil::FillDbSchema(LMP_SWITCH_ID, switch_id,
                        switch_id.length(),
                        DATATYPE_UINT8_ARRAY_256,
                        vect_table_attr_schema);

  // physical_port_id
  string physical_port_id = reinterpret_cast<const char*>(
      obj_key_logical_member_port->physical_port_id);
  pfc_log_info("physical_port_id: %s ", physical_port_id.c_str());
  PhyUtil::FillDbSchema(LMP_PHYSICAL_PORT_ID, physical_port_id,
                        physical_port_id.length(),
                        DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);
  // Last key can be checked for empty - condition for ReadBulk
  // only parent key has to be considered for read_sibling_begin
  if (!physical_port_id.empty() &&
      (operation_type != UNC_OP_READ_SIBLING_BEGIN &&
          operation_type != UNC_OP_READ_SIBLING_COUNT)) {
    vect_prim_keys.push_back(LMP_PHYSICAL_PORT_ID);
    vect_key_operations.push_back(unc::uppl::GREATER);
  }
  if (physical_port_id.empty() &&
      operation_type > UNC_OP_READ &&
      vect_prim_keys.size() >= 1) {  // Key apart from controller
    vect_prim_keys.push_back(LMP_PHYSICAL_PORT_ID);
    vect_key_operations.push_back(unc::uppl::GREATER);
  }
  if ((operation_type == UNC_OP_READ_NEXT ||
      operation_type == UNC_OP_READ_BULK) &&
      !physical_port_id.empty()) {
    // Adding MULTIPLE_QUERY to primary key vector
    pfc_log_debug("Adding MULTIPLE_QUERY to primary key vector");
    vect_key_operations.push_back(unc::uppl::MULTIPLE_QUERY);
  }
  PhyUtil::reorder_col_attrs(vect_prim_keys, vect_table_attr_schema);
  kt_dbtableschema.set_table_name(UPPL_LOGICAL_MEMBER_PORT_TABLE);
  kt_dbtableschema.set_primary_keys(vect_prim_keys);
  row_list.push_back(vect_table_attr_schema);
  kt_dbtableschema.set_row_list(row_list);
  return;
}

/** FillLogicalMemberPortValueStructure
 * * @Description : This function populates val_ctr by querying database
 * * * @param[in] : switch common table dbtable schema, openflow switch
 *                  db schema, value structure and max_rep_ct, operation type
 * * * @return    : Success or associated error code
 * */
void Kt_LogicalMemberPort::FillLogicalMemberPortValueStructure(
    DBTableSchema &kt_logical_member_port_dbtableschema,
    uint32_t &max_rep_ct,
    uint32_t operation_type,
    vector<key_logical_member_port_t> &logical_mem_port) {
  // populate IPC value structure based on the response recevied from DB
  list < vector<TableAttrSchema> > res_logical_member_port_row_list =
      kt_logical_member_port_dbtableschema.get_row_list();
  list < vector<TableAttrSchema> > :: iterator res_logical_member_port_iter =
      res_logical_member_port_row_list.begin();
  max_rep_ct = res_logical_member_port_row_list.size();
  pfc_log_debug("res_logical_member_port_row_list.size: %d", max_rep_ct);

  for (; res_logical_member_port_iter != res_logical_member_port_row_list.end();
      ++res_logical_member_port_iter)  {
    vector<TableAttrSchema> res_logical_member_port_table_attr_schema =
        (*res_logical_member_port_iter);
    vector<TableAttrSchema> :: iterator vect_logical_member_port_iter =
        res_logical_member_port_table_attr_schema.begin();

    // Read all attributes
    key_logical_member_port_t obj_key_logical_mem_port;
    memset(&obj_key_logical_mem_port, '\0', sizeof(obj_key_logical_mem_port));

    for (; vect_logical_member_port_iter !=
        res_logical_member_port_table_attr_schema.end();
    ++vect_logical_member_port_iter) {
      // populate values from controller_common_table
      TableAttrSchema tab_schema = (*vect_logical_member_port_iter);
      string attr_name = tab_schema.table_attribute_name;
      string attr_value;
      if (attr_name == LMP_SWITCH_ID) {
        PhyUtil::GetValueFromDbSchema(tab_schema, attr_value,
                                      DATATYPE_UINT8_ARRAY_256);
        memcpy(obj_key_logical_mem_port.switch_id,
               attr_value.c_str(),
               attr_value.length()+1);
        pfc_log_debug("switch_id : %s", attr_value.c_str());
      }
      if (attr_name == LMP_PHYSICAL_PORT_ID) {
        PhyUtil::GetValueFromDbSchema(tab_schema, attr_value,
                                      DATATYPE_UINT8_ARRAY_32);
        memcpy(obj_key_logical_mem_port.physical_port_id,
               attr_value.c_str(),
               attr_value.length()+1);
        pfc_log_debug("physical_port_id: %s", attr_value.c_str());
      }
      if (attr_name == LMP_LP_PORT_ID) {
        PhyUtil::GetValueFromDbSchema(tab_schema, attr_value,
                                      DATATYPE_UINT8_ARRAY_320);
        memcpy(obj_key_logical_mem_port.logical_port_key.port_id,
               attr_value.c_str(),
               attr_value.length()+1);
        pfc_log_debug("port_id: %s", attr_value.c_str());
      }
      if (attr_name == DOMAIN_NAME) {
        PhyUtil::GetValueFromDbSchema(tab_schema, attr_value,
                                      DATATYPE_UINT8_ARRAY_32);
        memcpy(obj_key_logical_mem_port.logical_port_key.domain_key.domain_name,
               attr_value.c_str(),
               attr_value.length()+1);
        pfc_log_debug("domain_name: %s", attr_value.c_str());
      }
      if (attr_name == CTR_NAME) {
        PhyUtil::GetValueFromDbSchema(tab_schema, attr_value,
                                      DATATYPE_UINT8_ARRAY_32);
        memcpy(obj_key_logical_mem_port.logical_port_key.domain_key.
               ctr_key.controller_name,
               attr_value.c_str(),
               attr_value.length()+1);
        pfc_log_debug("controller_name: %s", attr_value.c_str());
      }
    }
    logical_mem_port.push_back(obj_key_logical_mem_port);
    pfc_log_debug("Vector size %zd", logical_mem_port.size());
  }
  return;
}


/** PerformRead
 * * @Description : This function reads the instance of
 *  KT_LogicalMemberPort based on operation type -
 *   READ, READ_SIBLING_BEGIN, READ_SIBLING
 * * * @param[in] : key_struct, value_struct, ipc session id,
 *                  configuration id, option1, option2, data_type,
 *                  operation type, max_rep_ct
 * * * @return    : Success or associated error code
 * */
UpplReturnCode Kt_LogicalMemberPort::PerformRead(uint32_t session_id,
                                                 uint32_t configuration_id,
                                                 void* key_struct,
                                                 void* value_struct,
                                                 uint32_t data_type,
                                                 uint32_t operation_type,
                                                 ServerSession &sess,
                                                 uint32_t option1,
                                                 uint32_t option2,
                                                 uint32_t max_rep_ct) {
  pfc_log_info("Inside PerformRead option1=%d option2=%d max_rep_ct=%d",
               option1, option2, max_rep_ct);
  pfc_log_info("Inside PerformRead operation_type=%d data_type=%d",
               operation_type, data_type);
  physical_response_header rsh = {session_id,
      configuration_id,
      operation_type,
      max_rep_ct,
      option1,
      option2,
      data_type,
      0};

  if (operation_type == UNC_OP_READ) {
    max_rep_ct = 1;
  }

  UpplReturnCode read_status = UPPL_RC_SUCCESS;
  key_logical_member_port_t *obj_key_logical_member_port =
      reinterpret_cast<key_logical_member_port_t*>(key_struct);
  if (option1 != UNC_OPT1_NORMAL) {
    pfc_log_error("Invalid option1 specified for read operation");
    rsh.result_code = UPPL_RC_ERR_INVALID_OPTION1;
    int err = PhyUtil::sessOutRespHeader(sess, rsh);
    err |= sess.addOutput((uint32_t)UNC_KT_LOGICAL_MEMBER_PORT);
    err |= sess.addOutput(*obj_key_logical_member_port);
    if (err != 0) {
      pfc_log_debug("addOutput failed for physical_response_header");
      return UPPL_RC_ERR_IPC_WRITE_ERROR;
    }
    return UPPL_RC_SUCCESS;
  }
  if (option2 != UNC_OPT2_NONE) {
    pfc_log_error("Invalid option2 specified for read operation");
    rsh.result_code = UPPL_RC_ERR_INVALID_OPTION2;
    int err = PhyUtil::sessOutRespHeader(sess, rsh);
    err |= sess.addOutput((uint32_t)UNC_KT_LOGICAL_MEMBER_PORT);
    err |= sess.addOutput(*obj_key_logical_member_port);
    if (err != 0) {
      pfc_log_debug("addOutput failed for physical_response_header");
      return UPPL_RC_ERR_IPC_WRITE_ERROR;
    }
    return UPPL_RC_SUCCESS;
  }
  // Read from DB directly for non state data types
  if ((unc_keytype_datatype_t)data_type != UNC_DT_STATE) {
    pfc_log_error("Read operation is provided on unsupported data type");
    rsh.result_code = UPPL_RC_ERR_OPERATION_NOT_ALLOWED;
    int err = PhyUtil::sessOutRespHeader(sess, rsh);
    err |= sess.addOutput((uint32_t)UNC_KT_LOGICAL_MEMBER_PORT);
    err |= sess.addOutput(*obj_key_logical_member_port);
    if (err != 0) {
      pfc_log_debug("addOutput failed for physical_response_header");
      return UPPL_RC_ERR_IPC_WRITE_ERROR;
    }
    return UPPL_RC_SUCCESS;
  }
  vector<key_logical_member_port_t> vect_logical_mem_port;
  read_status = ReadLogicalMemberPortValFromDB(key_struct,
                                               data_type,
                                               operation_type,
                                               max_rep_ct,
                                               vect_logical_mem_port);
  pfc_log_debug("size of vect_logical_mem_port=%zd",
                vect_logical_mem_port.size());
  pfc_log_debug("read_status of ReadLogicalMemberPortValFromDB = %d",
                read_status);
  rsh.result_code = read_status;
  rsh.max_rep_count = max_rep_ct;
  int err = PhyUtil::sessOutRespHeader(sess, rsh);
  if (err != 0) {
    pfc_log_error("Failure in addOutput");
    return UPPL_RC_ERR_IPC_WRITE_ERROR;
  }
  if (read_status == UPPL_RC_SUCCESS) {
    for (unsigned int index = 0;
        index < vect_logical_mem_port.size();
        ++index) {
      sess.addOutput((uint32_t)UNC_KT_LOGICAL_MEMBER_PORT);
      sess.addOutput((key_logical_member_port_t)vect_logical_mem_port[index]);
      if (index < vect_logical_mem_port.size() -1) {
        sess.addOutput();  // Seperator
      }
    }
  } else {
    pfc_log_info("Read Operation failed with %d", read_status);
    sess.addOutput((uint32_t)UNC_KT_LOGICAL_MEMBER_PORT);
    sess.addOutput(*obj_key_logical_member_port);
  }
  return UPPL_RC_SUCCESS;
}

/** ReadLogicalMemberPortValFromDB
 * * @Description : This function reads the instance of Kt_LogicalMemberPortbased on
 *                  operation type - READ, READ_SIBLING_BEGIN, READ_SIBLING
 *                  from data_base
 * * * @param[in] : key_struct, value_struct, ipc session id,
 *                  configuration id,data_type, operation type, max_rep_ct
 * * * @return    : Success or associated error code
 * */
UpplReturnCode Kt_LogicalMemberPort::ReadLogicalMemberPortValFromDB(
    void* key_struct,
    uint32_t data_type,
    uint32_t operation_type,
    uint32_t &max_rep_ct,
    vector<key_logical_member_port_t> &logical_mem_port,
    pfc_bool_t is_state) {
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  UpplReturnCode read_status = UPPL_RC_SUCCESS;
  ODBCM_RC_STATUS read_db_status = ODBCM_RC_SUCCESS;

  // Common structures that will be used to send query to ODBC
  // Structure used to send request to ODBC
  DBTableSchema kt_logical_member_port_dbtableschema;

  // construct primary key list
  vector<string> vect_prim_keys;
  vector<ODBCMOperator> prim_keys_operator;
  void *old_val_struct;
  PopulateDBSchemaForKtTable(
      kt_logical_member_port_dbtableschema,
      key_struct, NULL,
      operation_type, 0, 0,
      prim_keys_operator, old_val_struct);

  if (operation_type == UNC_OP_READ) {
    read_db_status = physical_layer->get_odbc_manager()->
        GetOneRow((unc_keytype_datatype_t)data_type,
                  kt_logical_member_port_dbtableschema);
  } else if (prim_keys_operator.empty()) {
    read_db_status = physical_layer->get_odbc_manager()->
        GetBulkRows((unc_keytype_datatype_t)data_type, max_rep_ct,
                    kt_logical_member_port_dbtableschema,
                    (unc_keytype_operation_t)operation_type);
  } else if (!prim_keys_operator.empty()) {
    read_db_status = physical_layer->get_odbc_manager()->
        GetSiblingRows((unc_keytype_datatype_t)data_type, max_rep_ct,
                       kt_logical_member_port_dbtableschema,
                       prim_keys_operator,
                       (unc_keytype_operation_t)operation_type);
  }
  if (read_db_status == ODBCM_RC_RECORD_NOT_FOUND) {
    pfc_log_debug("No record found");
    read_status = UPPL_RC_ERR_NO_SUCH_INSTANCE;
    return read_status;
  } else if (read_db_status != ODBCM_RC_SUCCESS) {
    read_status = UPPL_RC_ERR_DB_GET;
    // log error to log daemon
    pfc_log_info("Read operation has failed with error %d", read_db_status);
    return read_status;
  }
  FillLogicalMemberPortValueStructure(kt_logical_member_port_dbtableschema,
                                      max_rep_ct,
                                      operation_type,
                                      logical_mem_port);
  pfc_log_debug("logical_mem_port size: %d",
                (unsigned int)logical_mem_port.size());
  return read_status;
}

/** Fill_Attr_Syntax_Map
 * * @Description : This function fills the attributes associated
 *                  with the class
 * * * @param[in] : void
 * * * @return    : void
 * */
void Kt_LogicalMemberPort::Fill_Attr_Syntax_Map() {
  Kt_Class_Attr_Syntax objKeyAttrSyntax1 =
  { PFC_IPCTYPE_STRING, 0, 0, 1, 256, true, ""};
  attr_syntax_map[LMP_SWITCH_ID] = objKeyAttrSyntax1;

  Kt_Class_Attr_Syntax objKeyAttrSyntax2 =
  { PFC_IPCTYPE_STRING, 0, 0, 1, 32, true, ""};
  attr_syntax_map[LMP_PHYSICAL_PORT_ID] = objKeyAttrSyntax2;

  Kt_Class_Attr_Syntax objKeyAttrSyntax3 =
  { PFC_IPCTYPE_STRING, 0, 0, 1, 291, true, ""};
  attr_syntax_map[LMP_LP_PORT_ID] = objKeyAttrSyntax3;

  Kt_Class_Attr_Syntax objKeyAttr1Syntax4 =
  { PFC_IPCTYPE_UINT8, 0, 0, 1, 32, true, "" };
  attr_syntax_map[DOMAIN_NAME] = objKeyAttr1Syntax4;

  Kt_Class_Attr_Syntax objKeyAttr1Syntax5 =
  { PFC_IPCTYPE_UINT8, 0, 0, 1, 32, true, "" };
  attr_syntax_map[CTR_NAME] = objKeyAttr1Syntax5;
}

/** IsLogicalMemberPortReferred
 *  * @Description : This function returns true if given controller_name,
 *  domain and port_id are referred in logical member port table
 *  * @param[in] : controller_name, domain_name, port_id
 *  * @return    : PFC_TRUE/PFC_FALSE
 */
pfc_bool_t Kt_LogicalMemberPort::IsLogicalMemberPortReferred(
    string controller_name,
    string domain_name,
    string port_id) {
  pfc_bool_t is_ref = PFC_FALSE;
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  DBTableSchema kt_dbtableschema;
  vector<TableAttrSchema> vect_table_attr_schema;
  TableAttrSchema kt_logical_port_table_attr_schema;
  list< vector<TableAttrSchema> > row_list;
  vector<string> vect_prim_keys;

  // Check whether controller_name1 is reffered
  vect_prim_keys.push_back("controller_name");
  vect_prim_keys.push_back("domain_name");
  vect_prim_keys.push_back("port_id");
  //  controller_name
  PhyUtil::FillDbSchema("controller_name", controller_name,
                        controller_name.length(),
                        DATATYPE_UINT8_ARRAY_32, vect_table_attr_schema);
  PhyUtil::FillDbSchema("domain_name", domain_name,
                        domain_name.length(),
                        DATATYPE_UINT8_ARRAY_32, vect_table_attr_schema);
  PhyUtil::FillDbSchema("port_id", port_id,
                        port_id.length(),
                        DATATYPE_UINT8_ARRAY_320, vect_table_attr_schema);
  kt_dbtableschema.set_table_name("logical_member_port_table");
  kt_dbtableschema.set_primary_keys(vect_prim_keys);
  row_list.push_back(vect_table_attr_schema);
  kt_dbtableschema.set_row_list(row_list);

  //  Send request to ODBC for logical_member_port_table
  ODBCM_RC_STATUS read_db_status = physical_layer->
      get_odbc_manager()->IsRowExists(UNC_DT_STATE,
                                      kt_dbtableschema);
  if (read_db_status == ODBCM_RC_ROW_EXISTS) {
    //  read count
    pfc_log_debug("Given logical_port id is referred in memberport - 1");
    is_ref = PFC_TRUE;
  }
  return is_ref;
}

/** ReadInternal
 * * @Description : This function reads the given  instance of KT_Port
 ** * @param[in] : session_id - ipc session id used for TC validation
 * key_struct - the key for the kt port instance
 * data_type - UNC_DT_* , read allowed in state
 * * @return    : UPPL_RC_SUCCESS is returned when the response
 * is added to ipc session successfully.
 * UPPL_RC_ERR_* is returned when ipc response could not be added to sess.
 * */
UpplReturnCode Kt_LogicalMemberPort::ReadInternal(
    vector<void *> &key_val,
    vector<void *> &val_struct,
    uint32_t data_type,
    uint32_t operation_type) {
  pfc_log_debug("Processing Kt_LogicalMemberPort::ReadInternal");
  uint32_t max_rep_ct = 1;
  if (operation_type != UNC_OP_READ) {
    max_rep_ct = UPPL_MAX_REP_CT;
  }
  // Get read response from database
  void *key_struct = key_val[0];
  vector<key_logical_member_port_t> logical_mem_port;
  UpplReturnCode read_status = ReadLogicalMemberPortValFromDB(key_struct,
                                                              data_type,
                                                              operation_type,
                                                              max_rep_ct,
                                                              logical_mem_port);
  key_val.clear();
  if (read_status != UPPL_RC_SUCCESS) {
    pfc_log_error("ReadValFromDB returned %d with response size %d",
                  read_status,
                  static_cast<int>(logical_mem_port.size()));
  } else {
    pfc_log_debug(
        "ReadLogicalMemberPortValFromDB returned %d with response size %d",
        read_status, static_cast<int>(logical_mem_port.size()));
    for (unsigned int iIndex = 0 ; iIndex < logical_mem_port.size();
        ++iIndex) {
      key_logical_member_port_t *key_mem_port = new
          key_logical_member_port_t(logical_mem_port[iIndex]);
      key_val.push_back(reinterpret_cast<void *>(key_mem_port));
    }
  }
  return read_status;
}
