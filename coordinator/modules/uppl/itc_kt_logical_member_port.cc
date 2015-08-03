/*
 * Copyright (c) 2012-2015 NEC Corporation
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
#include "itc_read_request.hh"
#include "itc_kt_port.hh"
#include "itc_kt_port_neighbor.hh"
#include "ipct_util.hh"
using unc::uppl::PhysicalLayer;
/** Constructor
 * @Description : This function initializes member variables
 * and fills the attribute syntax map used for validation
 * @param[in] : None
 * @return    : None
 * */
Kt_LogicalMemberPort::Kt_LogicalMemberPort() {
  if (attr_syntax_map_all.find(UNC_KT_LOGICAL_MEMBER_PORT) ==
        attr_syntax_map_all.end()) {
  Fill_Attr_Syntax_Map();
  }
}

/** Destructor
 * @Description : This function frees child key types
 * instances for kt_logical_member_port
 * @param[in] : None
 * @return    : None
 * */
Kt_LogicalMemberPort::~Kt_LogicalMemberPort() {
}

/**DeleteKeyInstance
 * @Description : This function deletes a row of KT_LogicalMemberPort in
 * state logicalmemberport table.
 * @param[in] :
 * key_struct - the key for the new kt logicalmemberport instance
 * data_type - UNC_DT_* , delete only allowed in state
 * key_type-UNC_KT_LOGICAL_MEMBER_PORT,value of unc_key_type_t
 * @return    : UNC_RC_SUCCESS is returned when the delete
 * is done successfully.
 * UNC_UPPL_RC_ERR_* is returned when the delete is error
 * */
UncRespCode Kt_LogicalMemberPort::DeleteKeyInstance(
    OdbcmConnectionHandler *db_conn,
    void* key_struct,
    uint32_t data_type,
    uint32_t key_type) {
  UncRespCode delete_status = UNC_RC_SUCCESS;
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  // Check whether operation is allowed on the given DT type
  if (((unc_keytype_datatype_t)data_type != UNC_DT_STATE) &&
      ((unc_keytype_datatype_t)data_type != UNC_DT_IMPORT)) {
    pfc_log_error("Delete operation is provided on unsupported data type");
    return UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED;
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
    vect_prim_keys.push_back(CTR_NAME_STR);
  if (!domain_name.empty())
    vect_prim_keys.push_back(DOMAIN_NAME_STR);
  if (!port_id.empty())
    vect_prim_keys.push_back(LMP_LP_PORT_ID_STR);
  if (!switch_id.empty())
    vect_prim_keys.push_back(LMP_SWITCH_ID_STR);
  if (!physical_port_id.empty())
    vect_prim_keys.push_back(LMP_PHYSICAL_PORT_ID_STR);
  // Construct TableAttrSchema structure
  // TableAttrSchema holds table_name, primary key, attr_name
  vector<TableAttrSchema> vect_table_attr_schema;
  list< vector<TableAttrSchema> > row_list;

  // controller_name
  PhyUtil::FillDbSchema(unc::uppl::CTR_NAME, controller_name,
                        controller_name.length(), DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);
  // domain_name
  PhyUtil::FillDbSchema(unc::uppl::DOMAIN_NAME, domain_name,
                        domain_name.length(), DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);
  // port_id
  PhyUtil::FillDbSchema(unc::uppl::LMP_LP_PORT_ID, port_id,
                        port_id.length(), DATATYPE_UINT8_ARRAY_320,
                        vect_table_attr_schema);
  // switch_id
  PhyUtil::FillDbSchema(unc::uppl::LMP_SWITCH_ID, switch_id,
                        switch_id.length(), DATATYPE_UINT8_ARRAY_256,
                        vect_table_attr_schema);
  // physical_port_id
  PhyUtil::FillDbSchema(unc::uppl::LMP_PHYSICAL_PORT_ID, physical_port_id,
                        physical_port_id.length(),
                        DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);
  // Send request to ODBC for logical_member_port_table delete
  kt_logical_member_port_dbtableschema.set_table_name(unc::uppl::
      LOGICAL_MEMBERPORT_TABLE);
  kt_logical_member_port_dbtableschema.set_primary_keys(vect_prim_keys);
  row_list.push_back(vect_table_attr_schema);
  kt_logical_member_port_dbtableschema.set_row_list(row_list);

  ODBCM_RC_STATUS delete_db_status = physical_layer->get_odbc_manager()-> \
      DeleteOneRow((unc_keytype_datatype_t)data_type,
                   kt_logical_member_port_dbtableschema, db_conn);
  if (delete_db_status != ODBCM_RC_SUCCESS) {
    if (delete_db_status == ODBCM_RC_CONNECTION_ERROR) {
      // log fatal error to log daemon
      UPPL_LOG_FATAL("DB connection not available or cannot access DB");
      delete_status = UNC_UPPL_RC_ERR_DB_ACCESS;
    } else if (delete_db_status == ODBCM_RC_ROW_NOT_EXISTS) {
      delete_status = UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE;
    } else {
      // log error to log daemon
      pfc_log_error("DeleteOneRow error response from DB is %d",
                    delete_db_status);
      delete_status = UNC_UPPL_RC_ERR_DB_DELETE;
    }
  } else {
    pfc_log_info("Delete of LMP in dt(%d) is success",
                 data_type);
  }
  return delete_status;
}

/** ReadBulk
 * @Description : This function reads the max_rep_ct number of instances of
 *                  the KT_LogicalMemberPort
 *  Order of ReadBulk response
 *  val_ctr -> val_ctr_domain -> val_logical_port ->
 *  val_logical_member_port -> val_switch ->  val_port ->
 *  val_link -> val_boundary
 * @param[in] :
 * key_struct - the key for the kt logicalmemberport instance
 * data_type - UNC_DT_* , read allowed in state
 * max_rep_ct - specifies number of rows to be returned
 * parent_call - indicates whether parent has called this readbulk
 * is_read_next - indicates whether this function is invoked from readnext
 * @return    : UNC_RC_SUCCESS is returned when the response
 * is added to ipc session successfully.
 * UNC_UPPL_RC_ERR_* is returned when ipc response could not be added to sess.
 * */
UncRespCode Kt_LogicalMemberPort::ReadBulk(
    OdbcmConnectionHandler *db_conn,
    void* key_struct,
    uint32_t data_type,
    uint32_t &max_rep_ct,
    int child_index,
    pfc_bool_t parent_call,
    pfc_bool_t is_read_next,
    ReadRequest *read_req) {
  UncRespCode read_status = UNC_RC_SUCCESS;
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
    pfc_log_debug("ReadBulk oper is not allowed in %d dt",
                  data_type);
    read_status = UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED;
    pfc_log_debug("read_status=%d", read_status);
    return read_status;
  }
  if (max_rep_ct == 0) {
    return UNC_RC_SUCCESS;
  }
  vector<key_logical_member_port_t> vect_logical_mem_port;
  // Read the controller values based on given key structure
  read_status = ReadBulkInternal(db_conn, key_struct,
                                 data_type,
                                 max_rep_ct,
                                 vect_logical_mem_port);

  pfc_log_debug("read_status from _logical_member_port is %d", read_status);
  if (read_status == UNC_RC_SUCCESS) {
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
      read_req->AddToBuffer(obj_key_buffer);
      BulkReadBuffer obj_sep_buffer = {
          UNC_KT_LOGICAL_MEMBER_PORT, IS_SEPARATOR, NULL
      };
      read_req->AddToBuffer(obj_sep_buffer);
      --max_rep_ct;
      if (max_rep_ct == 0) {
        pfc_log_debug("max_rep_ct reached zero, so returning");
        return read_status;
      }
    }
  } else if (read_status == UNC_UPPL_RC_ERR_DB_ACCESS) {
    pfc_log_debug("KtLogicalMemberPort ReadBulk - Returning DB Access Error");
    return read_status;
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
        db_conn, reinterpret_cast<void *>(&nextkin_key_struct),
        data_type,
        max_rep_ct,
        0,
        false,
        is_read_next,
        read_req);
    pfc_log_debug("read status from next kin Kt_LogicalPort is %d",
                  read_status);
    return UNC_RC_SUCCESS;
  }
  pfc_log_debug("log_mem_port reached end of table");
  pfc_log_debug("read_status=%d", read_status);
  if (read_status == UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE) {
    read_status = UNC_RC_SUCCESS;
  }
  return read_status;
}

/**ReadBulkInternal
 * @Description : This function reads bulk rows of KT_LogicalMemberPort in
 *  logicalmemberport table of specified data type.
 * @param[in] :
 * key_struct - the key for the kt logicalmemberport instance
 * max_rep_ct - specifies number of rows to be returned
 * data_type-UNC_DT_*,type of database
 * vect_logical_mem_port-instance of vector<key_logical_member_port_t> 
 * @return    : UNC_RC_SUCCESS is returned when the response
 * is added to ipc session successfully.
 * UNC_UPPL_RC_ERR_* is returned when ipc response could not be added to sess.
 * */
UncRespCode Kt_LogicalMemberPort::ReadBulkInternal(
    OdbcmConnectionHandler *db_conn,
    void* key_struct,
    uint32_t data_type,
    uint32_t max_rep_ct,
    vector<key_logical_member_port_t> &vect_logical_mem_port) {
  if (max_rep_ct <= 0) {
    return UNC_RC_SUCCESS;
  }
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  UncRespCode read_status = UNC_RC_SUCCESS;
  ODBCM_RC_STATUS read_db_status = ODBCM_RC_SUCCESS;
  DBTableSchema kt_logical_member_port_dbtableschema;

  vector<ODBCMOperator> primary_key_operation_list;
  void *old_val_struct;
  PopulateDBSchemaForKtTable(
      db_conn, kt_logical_member_port_dbtableschema,
      key_struct, NULL,
      UNC_OP_READ_BULK, data_type, 0, 0,
      primary_key_operation_list, old_val_struct,
      NOTAPPLIED, false, PFC_FALSE);
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
  uint32_t index = 0;
  for (; index < no_of_query; ++index) {
    if (kt_logical_member_port_dbtableschema.primary_keys_.size() < 3) {
      pfc_log_debug("No record found");
      read_status = UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE;
      break;
    }
    // Read rows from DB
    read_db_status = physical_layer->get_odbc_manager()-> \
        GetBulkRows((unc_keytype_datatype_t)data_type, max_rep_ct,
                    kt_logical_member_port_dbtableschema,
                    (unc_keytype_operation_t)UNC_OP_READ_BULK, db_conn);
    if (read_db_status == ODBCM_RC_RECORD_NOT_FOUND) {
      pfc_log_debug("No record found");
      read_status = UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE;
      // Update the primary key vector
      kt_logical_member_port_dbtableschema.primary_keys_.pop_back();
      pfc_log_debug(
          "Primary key vector size %"
          PFC_PFMT_SIZE_T,
      kt_logical_member_port_dbtableschema.primary_keys_.size());
    } else if (read_db_status == ODBCM_RC_CONNECTION_ERROR) {
      read_status = UNC_UPPL_RC_ERR_DB_ACCESS;
      pfc_log_error("Read operation has failed with error %d",
                     read_db_status);
      break;
    } else if (read_db_status == ODBCM_RC_SUCCESS) {
      read_status = UNC_RC_SUCCESS;
      uint32_t max_rep_ct_new = 0;
      pfc_log_debug("Received success response from db");
      // From the values received from DB, populate val structure
      FillLogicalMemberPortValueStructure(db_conn,
                                        kt_logical_member_port_dbtableschema,
                                        max_rep_ct_new,
                                        UNC_OP_READ_BULK,
                                        vect_logical_mem_port);
      pfc_log_debug("max_rep_ct_new=%d max_rep_ct=%d",
                      max_rep_ct_new, max_rep_ct);
      for (uint32_t uindex = 1; uindex < max_rep_ct_new; uindex++) {
        pfc_log_debug("Ktlink:Row list Removed");
        kt_logical_member_port_dbtableschema.DeleteRowListFrontElement();
      }
      max_rep_ct -= max_rep_ct_new;
      kt_logical_member_port_dbtableschema.primary_keys_.pop_back();
    } else {
      read_status = UNC_UPPL_RC_ERR_DB_GET;
      // log error to log daemon
      pfc_log_error("Read operation has failed");
     break;
    }
  }  // for end
  if (vect_logical_mem_port.empty() && index == 2) {
     pfc_log_debug("No record found");
     read_status = UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE;
  } else if (!vect_logical_mem_port.empty()) {
     read_status = UNC_RC_SUCCESS;
  }
  return read_status;
}

/** PerformSyntaxValidation
 * @Description : This function performs syntax validation for
 *  UNC_KT_LOGICAL_MEMBER_PORT
 * @param[in]
 * key_struct - the key for the kt logicalmemberport instance
 * value_struct - the value for the kt logicalmemberport instance
 * data_type - UNC_DT_*,type of database
 * operation_type - UNC_OP*,type of operation
 * @return    : UNC_RC_SUCCESS is returned when the validation is successful
 * UNC_UPPL_RC_ERR_* is returned when validation is failure
 * */

UncRespCode Kt_LogicalMemberPort::PerformSyntaxValidation(
    OdbcmConnectionHandler *db_conn,
    void* key_struct,
    void* val_struct,
    uint32_t operation,
    uint32_t data_type) {
  UncRespCode ret_code = UNC_RC_SUCCESS;
  if (val_struct != NULL) {
    pfc_log_error("KT_LOGICAL_MEMBER_PORT does not have any value structure");
    return UNC_UPPL_RC_ERR_CFG_SYNTAX;
  }

  // Validate Key Structure
  pfc_bool_t mandatory = PFC_TRUE;
  key_logical_member_port *key =
      reinterpret_cast<key_logical_member_port_t*>(key_struct);

  string value = reinterpret_cast<char*>(key->switch_id);
  map<string, Kt_Class_Attr_Syntax> attr_syntax_map =
        attr_syntax_map_all[UNC_KT_LOGICAL_MEMBER_PORT];
  IS_VALID_STRING_KEY(LMP_SWITCH_ID_STR, value, operation, ret_code, mandatory);
  if (ret_code != UNC_RC_SUCCESS) {
    return UNC_UPPL_RC_ERR_CFG_SYNTAX;
  }


  value = reinterpret_cast<char*>(key->physical_port_id);
  IS_VALID_STRING_KEY(LMP_PHYSICAL_PORT_ID_STR, value, operation,
                      ret_code, mandatory);
  if (ret_code != UNC_RC_SUCCESS) {
    return UNC_UPPL_RC_ERR_CFG_SYNTAX;
  }

  value = reinterpret_cast<char*>(key->logical_port_key.port_id);
  IS_VALID_STRING_KEY(LMP_LP_PORT_ID_STR, value, operation,
                      ret_code, mandatory);
  if (ret_code != UNC_RC_SUCCESS) {
    return UNC_UPPL_RC_ERR_CFG_SYNTAX;
  }

  value = reinterpret_cast<char*>
  (key->logical_port_key.domain_key.domain_name);
  IS_VALID_STRING_KEY(DOMAIN_NAME_STR, value, operation,
                      ret_code, mandatory);
  if (ret_code != UNC_RC_SUCCESS) {
    return UNC_UPPL_RC_ERR_CFG_SYNTAX;
  }

  value = reinterpret_cast<char*>(key->
      logical_port_key.domain_key.ctr_key.controller_name);
  IS_VALID_STRING_KEY(CTR_NAME_STR, value, operation,
                      ret_code, mandatory);
  if (ret_code != UNC_RC_SUCCESS) {
    return UNC_UPPL_RC_ERR_CFG_SYNTAX;
  }
  return ret_code;
}

/** PerformSemanticValidation
 * @Description : This function performs semantic validation
 * for UNC_KT_LOGICAL_MEMBER_PORT
 * @param[in] : key_struct - specifies key instance of KTLogicalMemberPort
 * value_struct - specifies value of KTLOGICAL_MEMBER_PORT
 * operation - UNC_OP*,type of operation
 * data_type - UNC_DT*,type of database
 * @return    : UNC_RC_SUCCESS if semantic valition is successful
 * or UNC_UPPL_RC_ERR_* if failed
 * */

UncRespCode Kt_LogicalMemberPort::PerformSemanticValidation(
    OdbcmConnectionHandler *db_conn,
    void* key_struct,
    void* val_struct,
    uint32_t operation,
    uint32_t data_type) {
  pfc_log_debug("Inside PerformSemanticValidation of KT_LOGICAL_MEMBER_PORT");
  UncRespCode status = UNC_RC_SUCCESS;

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

  UncRespCode KeyStatus = IsKeyExists(db_conn,
                                         (unc_keytype_datatype_t)data_type,
                                         vect_key_value);
  pfc_log_debug("return value of IsKeyExists:%d", KeyStatus);
  // KeyStatus = UNC_RC_SUCCESS; //to be removed
  if (operation == UNC_OP_UPDATE || operation == UNC_OP_DELETE ||
      operation == UNC_OP_READ) {
    if (KeyStatus == UNC_UPPL_RC_ERR_DB_ACCESS) {
      pfc_log_error("DB Access failure");
      status = KeyStatus;
    } else if (KeyStatus != UNC_RC_SUCCESS) {
      pfc_log_info("key not found,U/D/R opern not allowed");
      status = UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE;
    } else {
      pfc_log_debug("key exist, update/del/read oper allowed");
    }
  }
  if (operation == UNC_OP_CREATE) {
    if (KeyStatus == UNC_RC_SUCCESS) {
      pfc_log_info("Key exists,CREATE not allowed");
      status = UNC_UPPL_RC_ERR_INSTANCE_EXISTS;
    } else if (KeyStatus == UNC_UPPL_RC_ERR_DB_ACCESS) {
      pfc_log_error("DB Access failure");
      status = KeyStatus;
    } else {
      pfc_log_debug("key not exist, create oper allowed");
    }
  }
  if (operation == UNC_OP_CREATE && status == UNC_RC_SUCCESS) {
    vector<string> parent_vect_key_value;
    parent_vect_key_value.push_back(controller_name);
    parent_vect_key_value.push_back(domain_name);
    parent_vect_key_value.push_back(port_id);
    Kt_LogicalPort KtObj;
    UncRespCode parent_key_status = KtObj.IsKeyExists(
        db_conn, (unc_keytype_datatype_t)data_type, parent_vect_key_value);
    pfc_log_debug("Parent IsKeyExists status %d", parent_key_status);
    if (parent_key_status != UNC_RC_SUCCESS) {
      status = UNC_UPPL_RC_ERR_PARENT_DOES_NOT_EXIST;
    }
  }
  pfc_log_debug("status before returning=%d", status);
  return status;
}

/** IsKeyExists
 * @Description : This function checks whether the
 *  logicalmemberport_id exists in DB
 * @param[in] : data_type-UNC_DT_*,type of database
 * key_values-vector of strings containing key values
 * @return    : Success or associated error code,UNC_RC_SUCCESS/UNC_UPPL_RC_ERR*
 * */
UncRespCode Kt_LogicalMemberPort::IsKeyExists(
    OdbcmConnectionHandler *db_conn,
    unc_keytype_datatype_t data_type,
    const vector<string> &key_values) {
  PhysicalLayer* physical_layer = PhysicalLayer::get_instance();
  UncRespCode check_status = UNC_RC_SUCCESS;

  if (key_values.empty()) {
    // No key given, return failure
    pfc_log_error("No key given. Returning error");
    return UNC_UPPL_RC_ERR_BAD_REQUEST;
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
  vect_prim_keys.push_back(CTR_NAME_STR);
  vect_prim_keys.push_back(DOMAIN_NAME_STR);
  vect_prim_keys.push_back(LP_PORT_ID_STR);
  vect_prim_keys.push_back(LMP_SWITCH_ID_STR);
  vect_prim_keys.push_back(LMP_PHYSICAL_PORT_ID_STR);
  // Construct TableAttrSchema structure
  // TableAttrSchema holds table_name, primary key, attr_name
  vector<TableAttrSchema> vect_table_attr_schema;
  list< vector<TableAttrSchema> > row_list;

  // controller_name
  PhyUtil::FillDbSchema(unc::uppl::CTR_NAME, controller_name,
                        controller_name.length(), DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);

  // domain_name
  PhyUtil::FillDbSchema(unc::uppl::DOMAIN_NAME, domain_name,
                        domain_name.length(), DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);

  // port_id
  PhyUtil::FillDbSchema(unc::uppl::LMP_LP_PORT_ID, port_id,
                        port_id.length(), DATATYPE_UINT8_ARRAY_320,
                        vect_table_attr_schema);
  // switch_id
  PhyUtil::FillDbSchema(unc::uppl::LMP_SWITCH_ID, switch_id,
                        switch_id.length(), DATATYPE_UINT8_ARRAY_256,
                        vect_table_attr_schema);
  // physical_port_id
  PhyUtil::FillDbSchema(unc::uppl::LMP_PHYSICAL_PORT_ID, physical_port_id,
                        physical_port_id.length(),
                        DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);

  kt_logical_member_port_dbtableschema.set_table_name(unc::uppl::
      LOGICAL_MEMBERPORT_TABLE);
  kt_logical_member_port_dbtableschema.set_primary_keys(vect_prim_keys);
  row_list.push_back(vect_table_attr_schema);
  kt_logical_member_port_dbtableschema.set_row_list(row_list);
  //  Send request to ODBC for controlle_common_table
  ODBCM_RC_STATUS check_db_status = physical_layer->get_odbc_manager()->
      IsRowExists(data_type, kt_logical_member_port_dbtableschema, db_conn);
  if (check_db_status == ODBCM_RC_CONNECTION_ERROR) {
    // log error to log daemon
    pfc_log_error("DB connection not available or cannot access DB");
    check_status = UNC_UPPL_RC_ERR_DB_ACCESS;
  } else if (check_db_status == ODBCM_RC_ROW_EXISTS) {
    pfc_log_debug("DB returned success for Row exists");
  } else {
    pfc_log_debug("DB Returned failure for IsRowExists");
    check_status = UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE;
  }
  pfc_log_debug("check_status = %d", check_status);
  return check_status;
}

/** PopulateDBSchemaForKtTable
 * @Description : This function populates the DBAttrSchema to be used to
 *                  send request to ODBC
 * @param[in] : kt_dbtableschema- instance of DBTableSchema to be filled
 * key_struct-void* to LMP key structure
 * operation_type-UNC_OP_*
 * val_struct-void* to kt val structure
 * option1,option2-additional options for read operations,UNC_OPT1/OPT2_*
 * vect_key_operations- instance of vector<ODBCMOperator>
 * old_value_struct-void* to kt val structure
 * row_status- value of CsRowStatus
 * is_filtering-flag to indicate whether filter option is enabled
 * is_state-flag to indicate whether datatype is DT_STATE
 * @return    : Success or associated error code,UNC_RC_SUCCESS/ERR*
 * */
void Kt_LogicalMemberPort::PopulateDBSchemaForKtTable(
    OdbcmConnectionHandler *db_conn,
    DBTableSchema &kt_dbtableschema,
    void* key_struct,
    void* val_struct,
    uint8_t operation_type,
    uint32_t data_type,
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


  // controller name
  string controller_name = reinterpret_cast<const char*>(
      obj_key_logical_member_port
      ->logical_port_key.domain_key.ctr_key.controller_name);
  PhyUtil::FillDbSchema(unc::uppl::CTR_NAME, controller_name,
                        controller_name.length(), DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);
  vect_prim_keys.push_back(CTR_NAME_STR);
  vect_key_operations.push_back(unc::uppl::EQUAL);

  // domain name
  string domain_name = reinterpret_cast<const char*>(obj_key_logical_member_port
      ->logical_port_key.domain_key.domain_name);
  PhyUtil::FillDbSchema(unc::uppl::DOMAIN_NAME, domain_name,
                        domain_name.length(), DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);
  if (!domain_name.empty()) {
    vect_prim_keys.push_back(DOMAIN_NAME_STR);
    vect_key_operations.push_back(unc::uppl::EQUAL);
  }

  // port_id
  string port_id = reinterpret_cast<const char*>(obj_key_logical_member_port
      ->logical_port_key.port_id);
  PhyUtil::FillDbSchema(unc::uppl::LMP_LP_PORT_ID, port_id,
                        port_id.length(), DATATYPE_UINT8_ARRAY_320,
                        vect_table_attr_schema);
  if (!port_id.empty()) {
    vect_prim_keys.push_back(LMP_LP_PORT_ID_STR);
    vect_key_operations.push_back(unc::uppl::EQUAL);
  }

  // switch_id
  string switch_id = reinterpret_cast<const char*> (obj_key_logical_member_port
      ->switch_id);
  // only parent key has to be considered for read_sibling_begin
  if (operation_type == UNC_OP_READ_SIBLING_BEGIN ||
      operation_type == UNC_OP_READ_SIBLING_COUNT) {
    vect_prim_keys.push_back(LMP_SWITCH_ID_STR);
    vect_key_operations.push_back(unc::uppl::GREATER);
    switch_id = "";
  } else if (!switch_id.empty()) {
    vect_prim_keys.push_back(LMP_SWITCH_ID_STR);
    vect_key_operations.push_back(unc::uppl::EQUAL);
  }
  PhyUtil::FillDbSchema(unc::uppl::LMP_SWITCH_ID, switch_id,
                        switch_id.length(),
                        DATATYPE_UINT8_ARRAY_256,
                        vect_table_attr_schema);

  // physical_port_id
  string physical_port_id = reinterpret_cast<const char*>(
      obj_key_logical_member_port->physical_port_id);
  PhyUtil::FillDbSchema(unc::uppl::LMP_PHYSICAL_PORT_ID, physical_port_id,
                        physical_port_id.length(),
                        DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);
  // Last key can be checked for empty - condition for ReadBulk
  // only parent key has to be considered for read_sibling_begin
  if (!physical_port_id.empty() &&
      (operation_type != UNC_OP_READ_SIBLING_BEGIN &&
          operation_type != UNC_OP_READ_SIBLING_COUNT)) {
    vect_prim_keys.push_back(LMP_PHYSICAL_PORT_ID_STR);
    vect_key_operations.push_back(unc::uppl::GREATER);
  }
  if (physical_port_id.empty() &&
      operation_type > UNC_OP_READ &&
      vect_prim_keys.size() >= 1) {  // Key apart from controller
    vect_prim_keys.push_back(LMP_PHYSICAL_PORT_ID_STR);
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
  kt_dbtableschema.set_table_name(unc::uppl::LOGICAL_MEMBERPORT_TABLE);
  kt_dbtableschema.set_primary_keys(vect_prim_keys);
  row_list.push_back(vect_table_attr_schema);
  kt_dbtableschema.set_row_list(row_list);
  pfc_log_info("ctr:%s,dom:%s,port_id:%s,sw_id:%s,phy_portid:%s,oper:%d",
                  controller_name.c_str(),
                  domain_name.c_str(),
                  port_id.c_str(),
                  switch_id.c_str(),
                  physical_port_id.c_str(),
                  operation_type);
  return;
}

/** FillLogicalMemberPortValueStructure
 * @Description : This function populates val_ctr by querying database
 * @param[in] :
 * kt_logical_member_port_dbtableschema-DBTableSchema instance to be filled
 * max_rep_ct-maximum no. of records to be read
 * operation_type-UNC_OP_*,type of database
 * logical_mem_port-instance of vector<key_logical_member_port_t>
 * @return    : Success or associated error code,UNC_RC_SUCCESS/ERR*
 * */
void Kt_LogicalMemberPort::FillLogicalMemberPortValueStructure(
    OdbcmConnectionHandler *db_conn,
    DBTableSchema &kt_logical_member_port_dbtableschema,
    uint32_t &max_rep_ct,
    uint32_t operation_type,
    vector<key_logical_member_port_t> &logical_mem_port) {
  // populate IPC value structure based on the response received from DB
  list < vector<TableAttrSchema> >& res_logical_member_port_row_list =
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
      ODBCMTableColumns attr_name = tab_schema.table_attribute_name;
      switch (attr_name) {
        case unc::uppl::LMP_SWITCH_ID:
          PhyUtil::GetValueFromDbSchemaStr(tab_schema,
                                           obj_key_logical_mem_port.switch_id,
                                           DATATYPE_UINT8_ARRAY_256);
          pfc_log_debug("switch_id : %s", obj_key_logical_mem_port.switch_id);
          break;

        case unc::uppl::LMP_PHYSICAL_PORT_ID:
          PhyUtil::GetValueFromDbSchemaStr(
              tab_schema,
              obj_key_logical_mem_port.physical_port_id,
              DATATYPE_UINT8_ARRAY_32);
          pfc_log_debug("physical_port_id: %s",
                        obj_key_logical_mem_port.physical_port_id);
          break;

        case unc::uppl::LMP_LP_PORT_ID:
          PhyUtil::GetValueFromDbSchemaStr(
              tab_schema,
              obj_key_logical_mem_port.logical_port_key.port_id,
              DATATYPE_UINT8_ARRAY_320);
          pfc_log_debug("port_id: %s",
                        obj_key_logical_mem_port.logical_port_key.port_id);
          break;

        case unc::uppl::DOMAIN_NAME:
          PhyUtil::GetValueFromDbSchemaStr(
              tab_schema,
              obj_key_logical_mem_port.logical_port_key.domain_key.domain_name,
              DATATYPE_UINT8_ARRAY_32);
          pfc_log_debug(
              "domain_name: %s",
              obj_key_logical_mem_port.logical_port_key.domain_key.domain_name);
          break;

        case unc::uppl::CTR_NAME:
          PhyUtil::GetValueFromDbSchemaStr(
              tab_schema,
              obj_key_logical_mem_port.logical_port_key.domain_key.
              ctr_key.controller_name,
              DATATYPE_UINT8_ARRAY_32);
          pfc_log_debug("controller_name: %s",
                        obj_key_logical_mem_port.logical_port_key.domain_key.
                        ctr_key.controller_name);
          break;

        default:
          pfc_log_info("Ignoring LMP attrib %d", attr_name);
          break;
      }
    }
    logical_mem_port.push_back(obj_key_logical_mem_port);
    pfc_log_debug("Vector size %"PFC_PFMT_SIZE_T, logical_mem_port.size());
  }
  return;
}


/** PerformRead
 * @Description : This function reads the instance of
 *  KT_LogicalMemberPort based on operation type -
 *   READ, READ_SIBLING_BEGIN, READ_SIBLING
 * @param[in] : key_struct-void* to LMP key structure
 * value_struct-void* to kt value structure
 * session_id-ipc session id used for TC validation
 * configuration_id-ipcconfiguration id used for TC validation
 * option1,option2-UNC_OPT1/OPT2_*,additional info for read operations
 * data_type-UNC_DT_*,type of database
 * operation_type-type of operation,UNC_OP_READ*
 * max_rep_ct-max.no of records to be read
 * @param[out]: sess-ServerSession object where the arguments present
 * @return    : Success or associated error code,UNC_RC_SUCCESS/ERR*
 * */
UncRespCode Kt_LogicalMemberPort::PerformRead(
    OdbcmConnectionHandler *db_conn,
    uint32_t session_id,
    uint32_t configuration_id,
    void* key_struct,
    void* value_struct,
    uint32_t data_type,
    uint32_t operation_type,
    ServerSession &sess,
    uint32_t option1,
    uint32_t option2,
    uint32_t max_rep_ct) {
  pfc_log_debug("PerformRead oper=%d,dt=%d",
               operation_type, data_type);
  physical_response_header rsh = {session_id,
      configuration_id,
      operation_type,
      max_rep_ct,
      option1,
      option2,
      data_type,
      static_cast<uint32_t>(0)};

  if (operation_type == UNC_OP_READ) {
    max_rep_ct = 1;
  }

  UncRespCode read_status = UNC_RC_SUCCESS;
  key_logical_member_port_t *obj_key_logical_member_port =
      reinterpret_cast<key_logical_member_port_t*>(key_struct);
  if (option1 != UNC_OPT1_NORMAL) {
    pfc_log_error("Invalid option1 specified for read operation");
    rsh.result_code = UNC_UPPL_RC_ERR_INVALID_OPTION1;
    int err = PhyUtil::sessOutRespHeader(sess, rsh);
    err |= sess.addOutput((uint32_t)UNC_KT_LOGICAL_MEMBER_PORT);
    err |= sess.addOutput(*obj_key_logical_member_port);
    if (err != 0) {
      pfc_log_info("addOutput failed for physical_response_header");
      return UNC_UPPL_RC_ERR_IPC_WRITE_ERROR;
    }
    return UNC_RC_SUCCESS;
  }
  if (option2 != UNC_OPT2_NONE && option2 != UNC_OPT2_NEIGHBOR) {
    pfc_log_error("Invalid option2 specified for read operation");
    rsh.result_code = UNC_UPPL_RC_ERR_INVALID_OPTION2;
    int err = PhyUtil::sessOutRespHeader(sess, rsh);
    err |= sess.addOutput((uint32_t)UNC_KT_LOGICAL_MEMBER_PORT);
    err |= sess.addOutput(*obj_key_logical_member_port);
    if (err != 0) {
      pfc_log_info("addOutput failed for physical_response_header");
      return UNC_UPPL_RC_ERR_IPC_WRITE_ERROR;
    }
    return UNC_RC_SUCCESS;
  }
  // Read from DB directly for non state data types
  if ((unc_keytype_datatype_t)data_type != UNC_DT_STATE) {
    pfc_log_error("Read operation is provided on unsupported data type");
    rsh.result_code = UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED;
    int err = PhyUtil::sessOutRespHeader(sess, rsh);
    err |= sess.addOutput((uint32_t)UNC_KT_LOGICAL_MEMBER_PORT);
    err |= sess.addOutput(*obj_key_logical_member_port);
    if (err != 0) {
      pfc_log_info("addOutput failed for physical_response_header");
      return UNC_UPPL_RC_ERR_IPC_WRITE_ERROR;
    }
    return UNC_RC_SUCCESS;
  }
  vector<key_logical_member_port_t> vect_logical_mem_port;
  read_status = ReadLogicalMemberPortValFromDB(db_conn,
                                               key_struct,
                                               data_type,
                                               operation_type,
                                               max_rep_ct,
                                               vect_logical_mem_port);
  pfc_log_debug("size of vect_logical_mem_port=%"
                PFC_PFMT_SIZE_T, vect_logical_mem_port.size());
  pfc_log_debug("read_status of ReadLogicalMemberPortValFromDB = %d",
                 read_status);
  rsh.result_code = read_status;
  rsh.max_rep_count = max_rep_ct;
  int err = PhyUtil::sessOutRespHeader(sess, rsh);
  if (err != 0) {
    pfc_log_error("Failure in addOutput");
    return UNC_UPPL_RC_ERR_IPC_WRITE_ERROR;
  }
  if (read_status == UNC_RC_SUCCESS) {
    for (unsigned int index = 0;
        index < vect_logical_mem_port.size();
        ++index) {
      err = sess.addOutput((uint32_t)UNC_KT_LOGICAL_MEMBER_PORT);
      err |= sess.addOutput(
          (key_logical_member_port_t)vect_logical_mem_port[index]);
      if (err != 0) {
        pfc_log_error("Failure in addOutput");
        return UNC_UPPL_RC_ERR_IPC_WRITE_ERROR;
      }
      if (option1 == UNC_OPT1_NORMAL && option2 ==  UNC_OPT2_NONE) {
        // Do nothing, already added above
      } else if (option1 == UNC_OPT1_NORMAL && option2 == UNC_OPT2_NEIGHBOR) {
        // Getting the neighbor from link table
        Kt_Port port_kt;
        key_port_t port_key;
        memset(&port_key, 0, sizeof(key_port_t));
        val_port_st_neighbor obj_neighbor;
        memset(&obj_neighbor, '\0', sizeof(val_port_st_neighbor));
        memcpy(port_key.sw_key.switch_id,
               vect_logical_mem_port[index].switch_id,
               sizeof(vect_logical_mem_port[index].switch_id));
        memcpy(port_key.sw_key.ctr_key.controller_name,
                    vect_logical_mem_port[index].logical_port_key.
                    domain_key.ctr_key.controller_name,
                    sizeof(vect_logical_mem_port[index].logical_port_key.
                    domain_key.ctr_key.controller_name));
        memcpy(port_key.port_id,
               vect_logical_mem_port[index].physical_port_id,
               sizeof(vect_logical_mem_port[index].physical_port_id));
        val_lm_port_st_neighbor_t  val_lmp_neigh;
        // Setting connected neighbor valid bits as INVALID
        memset(&val_lmp_neigh , '\0', sizeof(val_lm_port_st_neighbor_t));
        val_lmp_neigh.valid[kIdxLmPortConnectedSwitchId] = UNC_VF_INVALID;
        val_lmp_neigh.valid[kIdxLmPortConnectedPortId] = UNC_VF_INVALID;
        val_lmp_neigh.valid[kIdxLmPortConnectedControllerId] = UNC_VF_INVALID;
        read_status = port_kt.ReadNeighbor(db_conn,
                                   &port_key,
                                   NULL,
                                   data_type,
                                   obj_neighbor);
        if (read_status == UNC_RC_SUCCESS) {
          memcpy(&val_lmp_neigh.port,
                 &obj_neighbor.port, sizeof(obj_neighbor.port));
          memcpy(val_lmp_neigh.connected_switch_id,
                 obj_neighbor.connected_switch_id,
                 sizeof(obj_neighbor.connected_switch_id));
          memcpy(val_lmp_neigh.connected_port_id,
                 obj_neighbor.connected_port_id,
                 sizeof(obj_neighbor.connected_port_id));
          memcpy(val_lmp_neigh.connected_controller_id,
                 obj_neighbor.connected_controller_id,
                 sizeof(obj_neighbor.connected_controller_id));
          memcpy(val_lmp_neigh.valid, obj_neighbor.valid,
                    sizeof(obj_neighbor.valid));
        } else {
          // valid bit is set as invalid
        }
        //  Fill the session output for read neighbour
        err  = sess.addOutput(val_lmp_neigh);
        if (err != 0) {
          pfc_log_error("Failure in addOutput");
          return UNC_UPPL_RC_ERR_IPC_WRITE_ERROR;
        }
        pfc_log_debug(" %s", IpctUtil::get_string(val_lmp_neigh).c_str());
      }
      if (index < vect_logical_mem_port.size() -1) {
        sess.addOutput();  // Seperator
      }
    }
  } else {
    pfc_log_info("Read Operation failed with %d", read_status);
    err = sess.addOutput((uint32_t)UNC_KT_LOGICAL_MEMBER_PORT);
    err |= sess.addOutput(*obj_key_logical_member_port);
    if (err != 0) {
      pfc_log_error("Failure in addOutput");
      return UNC_UPPL_RC_ERR_IPC_WRITE_ERROR;
    }
  }
  return UNC_RC_SUCCESS;
}

/** ReadLogicalMemberPortValFromDB
 * @Description : This function reads the instance of Kt_LogicalMemberPort
 *                based on operation type - READ, READ_SIBLING_BEGIN,
 *                READ_SIBLING from data_base
 * @param[in] : key_struct-void* to LMP key structure
 * data_type-UNC_DT_*,type of database
 * operation_type-UNC_OP_*,type of operation
 * max_rep_ct-max. no of records to be read
 * logical_mem_port-instance of vector<key_logical_member_port_t>
 * is_state-flag to indicate whether datatype is DT_STATE
 * @return    : Success or associated error code,UNC_RC_SUCCESS/ERR*            
 * */
UncRespCode Kt_LogicalMemberPort::ReadLogicalMemberPortValFromDB(
    OdbcmConnectionHandler *db_conn,
    void* key_struct,
    uint32_t data_type,
    uint32_t operation_type,
    uint32_t &max_rep_ct,
    vector<key_logical_member_port_t> &logical_mem_port) {
  if (operation_type < UNC_OP_READ) {
    // Unsupported operation type for this function
    return UNC_RC_SUCCESS;
  }
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  UncRespCode read_status = UNC_RC_SUCCESS;
  ODBCM_RC_STATUS read_db_status = ODBCM_RC_SUCCESS;

  // Common structures that will be used to send query to ODBC
  // Structure used to send request to ODBC
  DBTableSchema kt_logical_member_port_dbtableschema;

  // construct primary key list
  vector<string> vect_prim_keys;
  vector<ODBCMOperator> prim_keys_operator;
  void *old_val_struct;
  PopulateDBSchemaForKtTable(
      db_conn, kt_logical_member_port_dbtableschema,
      key_struct, NULL,
      operation_type, data_type, 0, 0,
      prim_keys_operator, old_val_struct,
      NOTAPPLIED, false, PFC_FALSE);
  pfc_log_debug("Operation type is %d", operation_type);
  if (operation_type == UNC_OP_READ) {
    read_db_status = physical_layer->get_odbc_manager()->
        GetOneRow((unc_keytype_datatype_t)data_type,
                  kt_logical_member_port_dbtableschema, db_conn);
  } else if (prim_keys_operator.empty()) {
    pfc_log_debug("LMP:Read Bulk");
    read_db_status = physical_layer->get_odbc_manager()->
        GetBulkRows((unc_keytype_datatype_t)data_type, max_rep_ct,
                    kt_logical_member_port_dbtableschema,
                    (unc_keytype_operation_t)operation_type, db_conn);
  } else if (!prim_keys_operator.empty() &&
             operation_type == UNC_OP_READ_SIBLING) {
    uint32_t no_of_query = 2;  // LMP has 2 primary keys other than LP
    uint32_t index = 0;
    for (; index < no_of_query; ++index) {
      if (kt_logical_member_port_dbtableschema.primary_keys_.size() < 3) {
        pfc_log_debug("No record found");
        read_status = UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE;
        break;
      }
      pfc_log_debug("LMP:Read Sibling");
       read_db_status = physical_layer->get_odbc_manager()-> \
        GetBulkRows((unc_keytype_datatype_t)data_type, max_rep_ct,
                    kt_logical_member_port_dbtableschema,
                    (unc_keytype_operation_t)UNC_OP_READ_BULK,
                     db_conn);
      if (read_db_status ==  ODBCM_RC_SUCCESS) {
        uint32_t max_rep_ct_new = 0;
        FillLogicalMemberPortValueStructure(db_conn,
                                      kt_logical_member_port_dbtableschema,
                                      max_rep_ct_new,
                                      operation_type,
                                      logical_mem_port);
        pfc_log_debug("max_rep_ct_new=%d max_rep_ct=%d",
                      max_rep_ct_new, max_rep_ct);
        for (uint32_t uindex = 1; uindex < max_rep_ct_new; uindex++) {
          pfc_log_debug("Ktlink:Row list Removed");
          kt_logical_member_port_dbtableschema.DeleteRowListFrontElement();
        }
        max_rep_ct -= max_rep_ct_new;
        //  Update the primary key vector
        kt_logical_member_port_dbtableschema.primary_keys_.pop_back();
        read_status = UNC_RC_SUCCESS;
      } else if (read_db_status == ODBCM_RC_CONNECTION_ERROR) {
        read_status = UNC_UPPL_RC_ERR_DB_ACCESS;
        pfc_log_error("Read operation has failed with error %d",
                       read_db_status);
        return read_status;
      } else if (read_db_status == ODBCM_RC_RECORD_NOT_FOUND) {
        pfc_log_debug("No record found");
        read_status = UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE;
        // Update the primary key vector
         kt_logical_member_port_dbtableschema.primary_keys_.pop_back();
      }
    }  // for end
    if (logical_mem_port.empty() && index == 2) {
      pfc_log_debug("No record found");
      read_status = UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE;
    } else if (!logical_mem_port.empty()) {
      max_rep_ct = logical_mem_port.size();
      read_status = UNC_RC_SUCCESS;
    }
    return read_status;
  } else if (operation_type == UNC_OP_READ_SIBLING_BEGIN) {
     pfc_log_debug("LMP:Read Sibling Begin");
      read_db_status = physical_layer->get_odbc_manager()->
        GetSiblingRows((unc_keytype_datatype_t)data_type, max_rep_ct,
                       kt_logical_member_port_dbtableschema,
                       prim_keys_operator,
                       (unc_keytype_operation_t)operation_type, db_conn);
  }
  if (read_db_status == ODBCM_RC_RECORD_NOT_FOUND) {
    pfc_log_debug("No record found");
    read_status = UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE;
    return read_status;
  } else if (read_db_status == ODBCM_RC_CONNECTION_ERROR) {
    read_status = UNC_UPPL_RC_ERR_DB_ACCESS;
    pfc_log_error("Read operation has failed with error %d", read_db_status);
    return read_status;
  } else if (read_db_status != ODBCM_RC_SUCCESS) {
    read_status = UNC_UPPL_RC_ERR_DB_GET;
    // log error to log daemon
    pfc_log_info("Read oper failed with error %d", read_db_status);
    return read_status;
  }
  FillLogicalMemberPortValueStructure(db_conn,
                                      kt_logical_member_port_dbtableschema,
                                      max_rep_ct,
                                      operation_type,
                                      logical_mem_port);
  pfc_log_debug("logical_mem_port size: %"
                PFC_PFMT_SIZE_T, logical_mem_port.size());
  return read_status;
}

/** Fill_Attr_Syntax_Map
 * @Description : This function fills the attributes associated
 *                  with the class
 * @param[in] : void
 * @return    : void
 * */
void Kt_LogicalMemberPort::Fill_Attr_Syntax_Map() {
  map<string, Kt_Class_Attr_Syntax> attr_syntax_map;
  Kt_Class_Attr_Syntax objKeyAttrSyntax1 =
  { PFC_IPCTYPE_STRING, 0, 0, 1, 256, true, ""};
  attr_syntax_map[LMP_SWITCH_ID_STR] = objKeyAttrSyntax1;

  Kt_Class_Attr_Syntax objKeyAttrSyntax2 =
  { PFC_IPCTYPE_STRING, 0, 0, 1, 32, true, ""};
  attr_syntax_map[LMP_PHYSICAL_PORT_ID_STR] = objKeyAttrSyntax2;

  Kt_Class_Attr_Syntax objKeyAttrSyntax3 =
  { PFC_IPCTYPE_STRING, 0, 0, 1, 291, true, ""};
  attr_syntax_map[LMP_LP_PORT_ID_STR] = objKeyAttrSyntax3;

  Kt_Class_Attr_Syntax objKeyAttr1Syntax4 =
  { PFC_IPCTYPE_UINT8, 0, 0, 1, 32, true, "" };
  attr_syntax_map[DOMAIN_NAME_STR] = objKeyAttr1Syntax4;

  Kt_Class_Attr_Syntax objKeyAttr1Syntax5 =
  { PFC_IPCTYPE_UINT8, 0, 0, 1, 32, true, "" };
  attr_syntax_map[CTR_NAME_STR] = objKeyAttr1Syntax5;
  attr_syntax_map_all[UNC_KT_LOGICAL_MEMBER_PORT] = attr_syntax_map;
}

/** ReadInternal
 * @Description : This function reads the given  instance of KT_Port
 * @param[in] : session_id - ipc session id used for TC validation
 * key_struct - the key for the ktLMP instance
 * val_struct - NULL
 * operation_type-UNC_OP_READ*,type of read operation
 * data_type - UNC_DT_* , read allowed in state,type of database
 * @return    : UNC_RC_SUCCESS is returned when the response
 * is added to ipc session successfully.
 * UNC_UPPL_RC_ERR_* is returned when ipc response could not be added to sess.
 * */
UncRespCode Kt_LogicalMemberPort::ReadInternal(
    OdbcmConnectionHandler *db_conn,
    vector<void *> &key_val,
    vector<void *> &val_struct,
    uint32_t data_type,
    uint32_t operation_type) {
  if (operation_type != UNC_OP_READ && operation_type != UNC_OP_READ_SIBLING &&
      operation_type != UNC_OP_READ_SIBLING_BEGIN) {
    pfc_log_trace("This function not allowed for read next/bulk/count");
    return UNC_UPPL_RC_ERR_OPERATION_NOT_SUPPORTED;
  }
  pfc_log_debug("Processing Kt_LogicalMemberPort::ReadInternal");
  uint32_t max_rep_ct = 1;
  if (operation_type != UNC_OP_READ) {
    max_rep_ct = UPPL_MAX_REP_CT;
  }
  // Get read response from database
  void *key_struct = key_val[0];
  UncRespCode read_status = UNC_RC_SUCCESS;
  bool firsttime = true;
  do {
    vector<key_logical_member_port_t> logical_mem_port;
    read_status = ReadLogicalMemberPortValFromDB(db_conn,
                                                 key_struct,
                                                 data_type,
                                                 operation_type,
                                                 max_rep_ct,
                                                 logical_mem_port);
    pfc_log_debug(
      "ReadLMP returned %d with resp size %"
      PFC_PFMT_SIZE_T, read_status, logical_mem_port.size());
    if (firsttime) {
       pfc_log_trace(
           "Clearing key_val and val_struct vectors for the first time");
       key_val.clear();
       firsttime = false;
    }
    if (read_status == UNC_RC_SUCCESS) {
      for (unsigned int iIndex = 0 ; iIndex < logical_mem_port.size();
          ++iIndex) {
        key_logical_member_port_t *key_mem_port = new
            key_logical_member_port_t(logical_mem_port[iIndex]);
        key_val.push_back(reinterpret_cast<void *>(key_mem_port));
      }
    } else if ((read_status == UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE &&
               key_val.size() != 0)) {
      read_status = UNC_RC_SUCCESS;
    }
    if ((logical_mem_port.size() == UPPL_MAX_REP_CT) &&
                     (operation_type != UNC_OP_READ)) {
      pfc_log_debug("Op:%d, key.size:%" PFC_PFMT_SIZE_T"fetch_next_set",
                    operation_type, key_val.size());
      key_struct = reinterpret_cast<void *>(key_val[key_val.size() - 1]);
      operation_type = UNC_OP_READ_SIBLING;
      continue;
    } else {
      break;
    }
  } while (true);
  return read_status;
}

/**
 * @Description: This function updates the domain name for already existing
 *                trunk port record.
 * @param[in]  : key_struct - void pointer to be type cast into logical member
 *               port key type
 *               data_type - specifies the data base type i.e UNC_DT_STATE
 *                           or UNC_DT_IMPORT
 *               key_type   - indicates the key type 
 * @return     : UNC_RC_SUCCESS - if the update of a domain name is successful
 * @return     : UNC_UPPL_RC_ERR_* if ther is any failure while updating domain
 **/
UncRespCode Kt_LogicalMemberPort::UpdateDomainNameForTP(
                             OdbcmConnectionHandler *db_conn,
                             void* key_struct,
                             void* val_struct,
                             uint32_t data_type,
                             uint32_t key_type) {
  UncRespCode status = UNC_RC_SUCCESS;
  status = CreateKeyInstance(db_conn, reinterpret_cast<void *>(key_struct),
                             reinterpret_cast<void *>(val_struct),
                             (uint32_t)UNC_DT_STATE, key_type);
  return status;
}
