/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/**
 * @brief    KT Port implementation
 * @file     itc_kt_port.cc
 *
 */

#include "itc_kt_port.hh"
#include "itc_kt_controller.hh"
#include "itc_kt_switch.hh"
#include "itc_kt_logicalport.hh"
#include "itc_kt_link.hh"
#include "odbcm_utils.hh"
using unc::uppl::PhysicalLayer;
/** Constructor
 * * @Description : This function initializes member variables
 * and fills the attribute syntax map used for validation
 * * * @param[in] : None
 * * * @return    : None
 * */
Kt_Port::Kt_Port() {
  parent = NULL;
  // Populate structure to be used for syntax validation
  Fill_Attr_Syntax_Map();
}

/** Destructor
 * * @Description : This function frees the parent and child key types
 * instances for Kt_Port
 * * * @param[in] : None
 * * * @return    : None
 * */
Kt_Port::~Kt_Port() {
  // Delete parent object
  if (parent != NULL) {
    delete parent;
    parent = NULL;
  }
}

/** DeleteKeyInstance
 * * @Description : This function deletes the given  instance of KT_Port
 * * @param[in] : key_struct - key for the port instance
 * data_type - UNC_DT_* , delete only allowed in STATE
 * * @return    : UPPL_RC_SUCCESS is returned when the delete
 * is done successfully.
 * * UPPL_RC_ERR_* is returned when the delete is error
 * */
UpplReturnCode Kt_Port::DeleteKeyInstance(void* key_struct,
                                          uint32_t data_type,
                                          uint32_t key_type) {
  UpplReturnCode delete_status = UPPL_RC_SUCCESS;
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();

  // Check whether operation is allowed on the given DT type
  if ((unc_keytype_datatype_t)data_type != UNC_DT_STATE &&
      (unc_keytype_datatype_t)data_type != UNC_DT_IMPORT) {
    pfc_log_error("Delete operation is provided on unsupported data type");
    return UPPL_RC_ERR_OPERATION_NOT_ALLOWED;
  }

  key_port_t *obj_key_port= reinterpret_cast<key_port_t*>(key_struct);
  string controller_name = (const char*)obj_key_port->
      sw_key.ctr_key.controller_name;
  string switch_id = (const char*)obj_key_port->sw_key.switch_id;
  string port_id = (const char*)obj_key_port->port_id;
  // Driver sends kt_port delete to Physical,
  // Port delete corresponding logical port
  Kt_LogicalPort logical_port_obj;
  key_logical_port_t logical_port_key_obj;
  val_logical_port_t logical_port_val_obj;
  memset(&logical_port_val_obj, 0, sizeof(logical_port_val_obj));
  val_logical_port_st_t logical_port_st_val_obj;
  memset(&logical_port_st_val_obj, 0, sizeof(logical_port_st_val_obj));

  memcpy(logical_port_key_obj.domain_key.ctr_key.controller_name,
         controller_name.c_str(),
         controller_name.length()+1);

  memset(logical_port_key_obj.domain_key.domain_name, '\0',
         sizeof(logical_port_key_obj.domain_key.domain_name));

  memset(logical_port_key_obj.port_id, 0,
         sizeof(logical_port_key_obj.port_id));

  memset(logical_port_val_obj.valid, 0,
         sizeof(logical_port_val_obj.valid));

  memcpy(logical_port_val_obj.switch_id,
         switch_id.c_str(),
         switch_id.length()+1);
  memcpy(logical_port_val_obj.physical_port_id,
         port_id.c_str(),
         port_id.length()+1);
  memset(logical_port_st_val_obj.valid, 0, 2);
  logical_port_st_val_obj.valid[kIdxLogicalPortSt] = UNC_VF_VALID;
  logical_port_val_obj.valid[kIdxLogicalPortSwitchId] = UNC_VF_VALID;
  logical_port_val_obj.valid[kIdxLogicalPortPhysicalPortId] = UNC_VF_VALID;
  logical_port_st_val_obj.logical_port = logical_port_val_obj;
  // call read internal
  vector<void *> key_val;
  vector<void *> val_struct;
  key_val.push_back(reinterpret_cast<void *>(&logical_port_key_obj));
  val_struct.push_back(reinterpret_cast<void *>(&logical_port_st_val_obj));
  UpplReturnCode lp_read_status = logical_port_obj.ReadInternal(
      key_val, val_struct, UNC_DT_STATE, UNC_OP_READ);
  if (lp_read_status == UPPL_RC_SUCCESS) {
    // form key struct with all required primary keys and call delete
    logical_port_obj.DeleteKeyInstance(
        key_val[0],
        UNC_DT_STATE,
        UNC_KT_LOGICAL_PORT);
    // Clear key and value struct
    key_logical_port_t *key_log_port = reinterpret_cast<key_logical_port_t*>
    (key_val[0]);
    if (key_log_port != NULL) {
      delete key_log_port;
      key_log_port = NULL;
    }
    val_logical_port_t *val_log_port = reinterpret_cast<val_logical_port_t*>
    (val_struct[0]);
    if (val_log_port != NULL) {
      delete val_log_port;
      val_log_port = NULL;
    }
  }

  // Structure used to send request to ODBC
  DBTableSchema kt_port_dbtableschema;

  // Construct Primary key list
  vector<string> vect_prim_keys;
  vect_prim_keys.push_back(CTR_NAME);
  vect_prim_keys.push_back(SWITCH_ID);

  // Construct TableAttrSchema structure
  // TableAttrSchema holds table_name, primary key, attr_name
  vector<TableAttrSchema> vect_table_attr_schema;
  list< vector<TableAttrSchema> > row_list;

  // controller_name
  PhyUtil::FillDbSchema(CTR_NAME, controller_name,
                        controller_name.length(), DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);
  // switch_id
  PhyUtil::FillDbSchema(SWITCH_ID, switch_id,
                        switch_id.length(), DATATYPE_UINT8_ARRAY_256,
                        vect_table_attr_schema);

  // port_id
  if (port_id.length() > 0) {
    vect_prim_keys.push_back(PORT_ID);
    PhyUtil::FillDbSchema(PORT_ID, port_id,
                          port_id.length(), DATATYPE_UINT8_ARRAY_32,
                          vect_table_attr_schema);
  }

  // Send request to ODBC for port_table delete
  kt_port_dbtableschema.set_table_name(UPPL_PORT_TABLE);
  kt_port_dbtableschema.set_primary_keys(vect_prim_keys);
  row_list.push_back(vect_table_attr_schema);
  kt_port_dbtableschema.set_row_list(row_list);

  ODBCM_RC_STATUS delete_db_status = physical_layer->get_odbc_manager()-> \
      DeleteOneRow((unc_keytype_datatype_t)data_type,
                   kt_port_dbtableschema);
  pfc_log_info("DeleteOneRow response from DB is %d", delete_db_status);
  if (delete_db_status != ODBCM_RC_SUCCESS) {
    if (delete_db_status == ODBCM_RC_CONNECTION_ERROR) {
      // log fatal error to log daemon
      pfc_log_fatal("DB connection not available or cannot access DB");
      delete_status = UPPL_RC_ERR_DB_ACCESS;
    } else if (delete_db_status == ODBCM_RC_ROW_NOT_EXISTS) {
      pfc_log_error("given instance does not exist");
      delete_status = UPPL_RC_ERR_NO_SUCH_INSTANCE;
    } else {
      // log error to log daemon
      delete_status = UPPL_RC_ERR_DB_DELETE;
    }
  } else {
    pfc_log_info("Delete of a port in data_type(%d) is success",
                 data_type);
    delete_status = UPPL_RC_SUCCESS;
  }
  return delete_status;
}

/** ReadInternal
 * * @Description : This function reads the given  instance of KT_Port
 ** * @param[in] : session_id - ipc session id used for TC validation
 * key_struct - the key for the kt port instance
 * value_struct - the value for the kt port instance
 * data_type - UNC_DT_* , read allowed in state
 * * @return    : UPPL_RC_SUCCESS is returned when the response
 * is added to ipc session successfully.
 * UPPL_RC_ERR_* is returned when ipc response could not be added to sess.
 * */
UpplReturnCode Kt_Port::ReadInternal(vector<void *> &key_val,
                                     vector<void *> &val_struct,
                                     uint32_t data_type,
                                     uint32_t operation_type) {
  pfc_log_debug("Processing Kt_Port::ReadInternal");
  vector<key_port_t> vect_port_id;
  vector<val_port_st_t> vect_val_port_st;
  uint32_t max_rep_ct = 1;
  if (operation_type != UNC_OP_READ) {
    max_rep_ct = UPPL_MAX_REP_CT;
  }
  // Get read response from database
  void *key_struct = key_val[0];
  void *void_val_struct = NULL;
  if (!val_struct.empty()) {
    void_val_struct = val_struct[0];
  }
  UpplReturnCode read_status = ReadPortValFromDB(key_struct,
                                                 void_val_struct,
                                                 data_type,
                                                 operation_type,
                                                 max_rep_ct,
                                                 vect_val_port_st,
                                                 vect_port_id);
  key_val.clear();
  val_struct.clear();
  if (read_status != UPPL_RC_SUCCESS) {
    pfc_log_error("ReadPortValFromDB returned %d with response size %d",
                  read_status,
                  static_cast<int>(vect_val_port_st.size()));
  } else {
    pfc_log_debug("ReadPortValFromDB returned %d with response size %d",
                  read_status, static_cast<int>(vect_val_port_st.size()));
    for (unsigned int iIndex = 0 ; iIndex < vect_val_port_st.size();
        ++iIndex) {
      key_port_t *key_port = new key_port_t(vect_port_id[iIndex]);
      val_port_st_t *val_port = new val_port_st_t(vect_val_port_st[iIndex]);
      key_val.push_back(reinterpret_cast<void *>(key_port));
      val_struct.push_back(reinterpret_cast<void *>(val_port));
    }
  }
  return read_status;
}

/**ReadBulk
 * * @Description : This function reads bulk rows of KT_Port in
 *  port table of specified data type.
 *  Order of ReadBulk response
 *  val_port -> val_link -> val_boundary
 * * @param[in] :
 * key_struct - the key for the kt port instance
 * data_type - UNC_DT_* , read allowed in state
 * option1/option2 - specifies any additional condition for read operation
 * max_rep_ct - specifies number of rows to be returned
 * parent_call - indicates whether parent has called this readbulk
 * is_read_next - indicates whether this function is invoked from readnext
 * * @return    : UPPL_RC_SUCCESS is returned when the response
 * is added to ipc session successfully.
 * UPPL_RC_ERR_* is returned when ipc response could not be added to sess.
 * */
UpplReturnCode Kt_Port::ReadBulk(void* key_struct,
                                 uint32_t data_type,
                                 uint32_t option1,
                                 uint32_t option2,
                                 uint32_t &max_rep_ct,
                                 int child_index,
                                 pfc_bool_t parent_call,
                                 pfc_bool_t is_read_next) {
  pfc_log_info("Processing ReadBulk of Kt_Port");
  UpplReturnCode read_status = UPPL_RC_SUCCESS;
  key_port_t *obj_key_port= reinterpret_cast<key_port_t*>(key_struct);
  if ((unc_keytype_datatype_t)data_type != UNC_DT_STATE) {
    // Not supported data type
    read_status = UPPL_RC_ERR_OPERATION_NOT_ALLOWED;
    pfc_log_error("Operation on given data type is not allowed");
    return read_status;
  }
  string str_controller_name =
      reinterpret_cast<char *>(&obj_key_port
          ->sw_key.ctr_key.controller_name);
  string str_switch_id =
      reinterpret_cast<char *>(&obj_key_port
          ->sw_key.switch_id);
  string str_port_id =
      reinterpret_cast<char *>(&obj_key_port->port_id);
  if (max_rep_ct == 0) {
    pfc_log_debug("max_rep_ct is zero");
    return UPPL_RC_SUCCESS;
  }
  void *val_struct = NULL;
  vector<val_port_st_t> vect_val_port_st;
  vector<key_port_t> vect_port_id;

  // Read the port values based on given key structure
  read_status = ReadBulkInternal(
      key_struct,
      val_struct,
      data_type,
      max_rep_ct,
      vect_val_port_st,
      vect_port_id);

  pfc_log_debug("Read status from port is %d with result size %d",
                read_status, static_cast<int>(vect_port_id.size()));
  if (read_status == UPPL_RC_SUCCESS) {
    PhysicalCore *physical_core = PhysicalLayer::get_instance()->
        get_physical_core();
    InternalTransactionCoordinator *itc_trans  =
        physical_core->get_internal_transaction_coordinator();
    // For each switch , read the child's attributes
    vector<val_port_st_t>::iterator vect_iter =
        vect_val_port_st.begin();
    vector<key_port_t> ::iterator port_iter =
        vect_port_id.begin();
    for (; port_iter != vect_port_id.end(),
    vect_iter != vect_val_port_st.end();
    ++port_iter, ++vect_iter) {
      pfc_log_debug("Iterating entries...");
      pfc_log_debug("Adding port - '%s' to session",
                    reinterpret_cast<char *>((*port_iter).port_id));
      key_port_t *key_buffer = new key_port_t(*port_iter);
      BulkReadBuffer obj_key_buffer = {
          UNC_KT_PORT, IS_KEY,
          reinterpret_cast<void*>(key_buffer)
      };
      itc_trans->AddToBuffer(obj_key_buffer);
      val_port_st_t *val_buffer = new val_port_st_t(*vect_iter);
      BulkReadBuffer obj_value_buffer = {
          UNC_KT_PORT, IS_STATE_VALUE,
          reinterpret_cast<void*>(val_buffer)
      };
      itc_trans->AddToBuffer(obj_value_buffer);
      BulkReadBuffer obj_sep_buffer = {
          UNC_KT_PORT, IS_SEPARATOR, NULL
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
    pfc_log_debug("port is called directly, so go to parent");
    Kt_Switch nextKin;
    key_switch_t nextkin_key_struct;
    memset(nextkin_key_struct.switch_id, '\0',
           sizeof(nextkin_key_struct.switch_id));
    memset(nextkin_key_struct.ctr_key.controller_name,
           '\0', sizeof(nextkin_key_struct.ctr_key.controller_name));
    memcpy(nextkin_key_struct.switch_id,
           str_switch_id.c_str(),
           str_switch_id.length() +1);
    memcpy(nextkin_key_struct.ctr_key.controller_name,
           str_controller_name.c_str(),
           str_controller_name.length() +1);
    read_status = nextKin.ReadBulk(
        reinterpret_cast<void *>(&nextkin_key_struct),
        data_type,
        option1,
        option2,
        max_rep_ct,
        0,
        false,
        is_read_next);
    pfc_log_debug("read status from next kin Kt_Switch is %d", read_status);
    return UPPL_RC_SUCCESS;
  }
  return UPPL_RC_SUCCESS;
}

/**ReadBulkInternal
 * * @Description : This function reads bulk rows of KT_Port in
 *  port table of specified data type.
 * * @param[in] :
 * key_struct - the key for the kt port instance
 * val_struct - the value struct for kt_port instance
 * max_rep_ct - specifies number of rows to be returned
 * vect_val_port - indicates the fetched values from db of val_port type
 * vect_port_id - indicates the fetched port names from db
 * * @return    : UPPL_RC_SUCCESS is returned when the response
 * is added to ipc session successfully.
 * UPPL_RC_ERR_* is returned when ipc response could not be added to sess.
 * */
UpplReturnCode Kt_Port::ReadBulkInternal(
    void* key_struct,
    void* val_struct,
    uint32_t data_type,
    uint32_t max_rep_ct,
    vector<val_port_st_t> &vect_val_port,
    vector<key_port_t> &vect_port_id) {
  if (max_rep_ct <= 0) {
    return UPPL_RC_SUCCESS;
  }
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  UpplReturnCode read_status = UPPL_RC_SUCCESS;
  ODBCM_RC_STATUS read_db_status = ODBCM_RC_SUCCESS;
  DBTableSchema kt_port_dbtableschema;
  void *old_struct;
  // Populate DBSchema for port_table
  vector<ODBCMOperator> vect_key_operations;
  PopulateDBSchemaForKtTable(kt_port_dbtableschema,
                             key_struct,
                             val_struct,
                             UNC_OP_READ_BULK, 0, 0,
                             vect_key_operations, old_struct);
  pfc_log_debug("Calling GetBulkRows");
  // Read rows from DB
  read_db_status = physical_layer->get_odbc_manager()-> \
      GetBulkRows((unc_keytype_datatype_t)data_type, max_rep_ct,
                  kt_port_dbtableschema,
                  (unc_keytype_operation_t)UNC_OP_READ_BULK);
  pfc_log_debug("GetBulkRows return: %d", read_db_status);
  if (read_db_status == ODBCM_RC_RECORD_NOT_FOUND) {
    pfc_log_debug("No record found");
    read_status = UPPL_RC_ERR_NO_SUCH_INSTANCE;
    return read_status;
  } else if (read_db_status != ODBCM_RC_SUCCESS) {
    read_status = UPPL_RC_ERR_DB_GET;
    // log error to log daemon
    pfc_log_error("Read operation has failed");
    return read_status;
  }
  // From the values received from DB, populate val_port structure
  FillPortValueStructure(kt_port_dbtableschema,
                         vect_val_port,
                         max_rep_ct,
                         UNC_OP_READ_BULK,
                         vect_port_id);
  return read_status;
}

/** PerformSyntaxValidation
 * * @Description : This function performs syntax validation for
 *  UNC_KT_PORT
 * * * @param[in]
 * key_struct - the key for the kt port instance
 * value_struct - the value for the kt port instance
 * data_type - UNC_DT_*
 * operation_type - UNC_OP*
 * * @return    : UPPL_RC_SUCCESS is returned when the validation is successful
 * UPPL_RC_ERR_* is returned when validtion is failure
 * */
UpplReturnCode Kt_Port::PerformSyntaxValidation(void* key_struct,
                                                void* val_struct,
                                                uint32_t operation,
                                                uint32_t data_type) {
  pfc_log_info("Performing Syntax Validation of KT_PORT");
  UpplReturnCode ret_code = UPPL_RC_SUCCESS;
  pfc_bool_t mandatory = PFC_TRUE;
  // Validate key structure
  // validate controller_name
  key_port *key = reinterpret_cast<key_port_t*>(key_struct);
  string value = reinterpret_cast<char*>(key->sw_key.ctr_key.controller_name);
  IS_VALID_STRING_KEY(CTR_NAME, value, operation, ret_code, mandatory);
  if (ret_code != UPPL_RC_SUCCESS) {
    return UPPL_RC_ERR_CFG_SYNTAX;
  }

  // validate switch_id
  value = reinterpret_cast<char*>(key->sw_key.switch_id);
  IS_VALID_STRING_KEY(SWITCH_ID, value, operation, ret_code, mandatory);
  if (ret_code != UPPL_RC_SUCCESS) {
    return UPPL_RC_ERR_CFG_SYNTAX;
  }

  // validate port_id
  value = reinterpret_cast<char*>(key->port_id);
  IS_VALID_STRING_KEY(PORT_ID, value, operation, ret_code, mandatory);
  if (ret_code != UPPL_RC_SUCCESS) {
    return UPPL_RC_ERR_CFG_SYNTAX;
  }

  // Validate value structure
  if (val_struct != NULL) {
    unsigned int valid_val = 0;
    val_port *value_port = reinterpret_cast<val_port_t*>(val_struct);

    // validate port_number
    valid_val = PhyUtil::uint8touint(value_port->valid[kIdxPortNumber]);
    IS_VALID_INT_VALUE(PORT_NUMBER, value_port->port_number, operation,
                       valid_val, ret_code, mandatory);
    if (ret_code != UPPL_RC_SUCCESS) {
      return UPPL_RC_ERR_CFG_SYNTAX;
    }

    // validate description
    valid_val = PhyUtil::uint8touint(value_port->valid[kIdxPortDescription]);
    string value = reinterpret_cast<char*>(value_port->description);
    IS_VALID_STRING_VALUE(PORT_DESCRIPTION, value, operation,
                          valid_val, ret_code, mandatory);
    if (ret_code != UPPL_RC_SUCCESS) {
      return UPPL_RC_ERR_CFG_SYNTAX;
    }

    // validate admin_status
    valid_val = PhyUtil::uint8touint(value_port->valid[kIdxPortAdminStatus]);
    IS_VALID_INT_VALUE(PORT_ADMIN_STATUS, value_port->admin_status, operation,
                       valid_val, ret_code, mandatory);
    if (ret_code != UPPL_RC_SUCCESS) {
      return UPPL_RC_ERR_CFG_SYNTAX;
    }

    // validate trunk_allowed_vlan
    valid_val =
        PhyUtil::uint8touint(value_port->valid[kIdxPortTrunkAllowedVlan]);
    IS_VALID_INT_VALUE(PORT_TRUNK_ALL_VLAN, value_port->trunk_allowed_vlan,
                       operation, valid_val, ret_code, mandatory);
    if (ret_code != UPPL_RC_SUCCESS) {
      return UPPL_RC_ERR_CFG_SYNTAX;
    }
  }
  return ret_code;
}

/** PerformSemanticValidation
 * * @Description : This function performs semantic validation
 * for UNC_KT_PORT
 * * * @param[in] : key_struct - specifies key instance of KT_Port
 * , value_struct - specifies value of KT_PORT
 * operation - UNC_OP*
 * data_type - UNC_DT*
 * * * @return    : UPPL_RC_SUCCESS if semantic valition is successful
 * or UPPL_RC_ERR_* if failed
 * */
UpplReturnCode Kt_Port::PerformSemanticValidation(void* key_struct,
                                                  void* val_struct,
                                                  uint32_t operation,
                                                  uint32_t data_type) {
  UpplReturnCode status = UPPL_RC_SUCCESS;
  pfc_log_debug("Inside PerformSemanticValidation of KT_PORT");
  // Check whether the given instance of port exists in DB
  key_port *obj_key_port = reinterpret_cast<key_port_t*>(key_struct);
  string controller_name = (const char*)obj_key_port->
      sw_key.ctr_key.controller_name;
  string switch_id = (const char*)obj_key_port->sw_key.switch_id;
  string port_id = (const char*)obj_key_port->port_id;

  vector<string> key_value;
  // call Switch validation function which in turn invokes ctrl validation

  key_value.push_back(switch_id);

  pfc_log_debug("switch id: %s", switch_id.c_str());
  key_switch_t obj_key_switch;
  memset(obj_key_switch.switch_id, '\0', 32);
  memcpy(obj_key_switch.switch_id,
         switch_id.c_str(),
         switch_id.length()+1);

  key_value.push_back((const char*)obj_key_port->
                      sw_key.ctr_key.controller_name);

  pfc_log_debug("controller name in validate: %s", obj_key_port->
                sw_key.ctr_key.controller_name);
  memset(&obj_key_switch.ctr_key.controller_name, '\0', 32);
  memcpy(&obj_key_switch.ctr_key.controller_name,
         obj_key_port->sw_key.ctr_key.controller_name,
         sizeof(obj_key_port->sw_key.ctr_key.controller_name));

  pfc_log_debug("controller name in obj_key_port: %s",
                obj_key_switch.ctr_key.controller_name);


  vector<string> vect_key_value;
  vect_key_value.push_back(controller_name);
  vect_key_value.push_back(switch_id);
  vect_key_value.push_back(port_id);

  UpplReturnCode key_status = IsKeyExists(
      (unc_keytype_datatype_t)data_type, vect_key_value);
  pfc_log_debug("IsKeyExists status %d", key_status);
  // In case of create operation, key should not exist
  if (operation == UNC_OP_CREATE) {
    if (key_status == UPPL_RC_SUCCESS) {
      pfc_log_error("Key instance already exists");
      pfc_log_error("Hence create operation not allowed");
      status = UPPL_RC_ERR_INSTANCE_EXISTS;
    } else {
      pfc_log_info("key instance not exist create operation allowed");
    }
  } else if (operation == UNC_OP_UPDATE || operation == UNC_OP_DELETE ||
      operation == UNC_OP_READ) {
    // In case of update/delete/read operation, key should exist
    if (key_status != UPPL_RC_SUCCESS) {
      pfc_log_error("Key instance does not exist");
      pfc_log_error("Hence update/delete/read operation not allowed");
      status = UPPL_RC_ERR_NO_SUCH_INSTANCE;
    } else {
      pfc_log_info("key instance exist update/del/read operation allowed");
    }
  }
  if (operation == UNC_OP_CREATE && status == UPPL_RC_SUCCESS) {
    vector<string> parent_vect_key_value;
    parent_vect_key_value.push_back(controller_name);
    parent_vect_key_value.push_back(switch_id);
    Kt_Switch KtObj;
    UpplReturnCode parent_key_status = KtObj.IsKeyExists(
        (unc_keytype_datatype_t)data_type, parent_vect_key_value);
    pfc_log_debug("Parent IsKeyExists status %d", parent_key_status);
    if (parent_key_status != UPPL_RC_SUCCESS) {
      status = UPPL_RC_ERR_PARENT_DOES_NOT_EXIST;
    }
  }
  pfc_log_debug("Return Code SemanticValidation: %d", status);
  return status;
}

/** HandleDriverAlarms
 * * @Description : This function processes the alarm notification
 * received from driver
 * * * @param[in] : alarm type - contains type to indicate which alarm
 * operation - contains UNC_OP_CREATE or UNC_OP_DELETE
 * key_struct - indicates the key instance of KT_PORT
 * value_struct - indicates the alarm values structure
 * * * @return    : UPPL_RC_SUCCESS if alarm is handled successfully or
 * UPPL_RC_ERR*
 * */
UpplReturnCode Kt_Port::HandleDriverAlarms(uint32_t data_type,
                                           uint32_t alarm_type,
                                           uint32_t oper_type,
                                           void* key_struct,
                                           void* val_struct) {
  UpplReturnCode status = UPPL_RC_SUCCESS;
  key_port_t *obj_key_port =
      reinterpret_cast<key_port_t*>(key_struct);
  string controller_name =
      (const char*)obj_key_port->sw_key.ctr_key.controller_name;
  string switch_id = (const char*)obj_key_port->sw_key.switch_id;
  string port_id = (const char*)obj_key_port->port_id;
  memcpy(obj_key_port->port_id,
         port_id.c_str(),
         port_id.length()+1);
  memcpy(obj_key_port->sw_key.switch_id,
         switch_id.c_str(),
         switch_id.length()+1);
  memcpy(obj_key_port->sw_key.ctr_key.controller_name,
         controller_name.c_str(),
         controller_name.length()+1);
  pfc_log_info("alarm sent by driver is: %d", alarm_type);
  pfc_log_info("operation type: %d", oper_type);
  uint64_t alarm_status_db = 0;
  UpplReturnCode read_alarm_status = GetAlarmStatus(data_type,
                                                    key_struct,
                                                    alarm_status_db);
  // Read old_alarm_status from db
  if (read_alarm_status == UPPL_RC_SUCCESS) {
    uint64_t new_alarm_status = 0;
    uint64_t old_alarm_status = alarm_status_db;
    pfc_log_info("alarm_status received from db: %" PFC_PFMT_u64,
                 old_alarm_status);
    if (alarm_type == UNC_DEFAULT_FLOW) {
      if (oper_type == UNC_OP_CREATE) {
        // Set Default flow alarm
        new_alarm_status = old_alarm_status | ALARM_UPPL_ALARMS_DEFAULT_FLOW;
      } else if (oper_type == UNC_OP_DELETE) {
        // Clear Default flow alarm
        new_alarm_status = old_alarm_status & 0xFFFE;
      }
    }
    if (alarm_type == UNC_PORT_DIRECTION) {
      if (oper_type == UNC_OP_CREATE) {
        // Set port direction alarm
        new_alarm_status =
            old_alarm_status | ALARM_UPPL_ALARMS_PORT_DIRECTION;
      } else if (oper_type == UNC_OP_DELETE) {
        // Clear port direction alarm
        new_alarm_status = old_alarm_status & 0xFFFD;
      }
    }
    if (alarm_type == UNC_PORT_CONGES) {
      if (oper_type == UNC_OP_CREATE) {
        // Set port congestion alarm
        new_alarm_status = old_alarm_status | ALARM_UPPL_ALARMS_PORT_CONGES;
      } else if (oper_type == UNC_OP_DELETE) {
        // Clear port congestion alarm
        new_alarm_status = old_alarm_status & 0xFFFB;
      }
    }
    pfc_log_info("alarm_status to be set is: %" PFC_PFMT_u64, new_alarm_status);
    // Call UpdateKeyInstance to update the new alarm status and
    // new oper status value in DB
    val_port_st obj_val_port_st;
    memset(obj_val_port_st.valid, '\0',
           sizeof(obj_val_port_st.valid));
    memset(obj_val_port_st.port.valid, '\0',
           sizeof(obj_val_port_st.port.valid));
    obj_val_port_st.valid[kIdxPortOperStatus] = UNC_VF_INVALID;
    obj_val_port_st.valid[kIdxPortAlarmsStatus] = UNC_VF_VALID;
    obj_val_port_st.alarms_status = new_alarm_status;
    void *old_val_struct;
    // Calling UPDATE KEY INSTANCE for update in DB
    status = UpdateKeyInstance(obj_key_port,
                               reinterpret_cast<void *>(&obj_val_port_st),
                               data_type,
                               UNC_KT_PORT,
                               old_val_struct);
    if (status == UPPL_RC_SUCCESS && data_type != UNC_DT_IMPORT) {
      // Send oper_status notification to northbound
      // Form value struct
      val_port_st old_val_port, new_val_port;

      old_val_port.alarms_status = old_alarm_status;
      old_val_port.valid[kIdxPortSt] = UNC_VF_INVALID;
      old_val_port.valid[kIdxPortOperStatus] = UNC_VF_INVALID;
      old_val_port.valid[kIdxPortAlarmsStatus] = UNC_VF_VALID;

      new_val_port.alarms_status = new_alarm_status;
      new_val_port.valid[kIdxPortSt] = UNC_VF_INVALID;
      new_val_port.valid[kIdxPortOperStatus] = UNC_VF_INVALID;
      new_val_port.valid[kIdxPortAlarmsStatus] = UNC_VF_VALID;
      int err = 0;
      // Send notification to Northbound
      ServerEvent ser_evt((pfc_ipcevtype_t)UPPL_EVENTS_KT_PORT, err);
      ser_evt.addOutput((uint32_t)UNC_OP_UPDATE);
      ser_evt.addOutput(data_type);
      ser_evt.addOutput((uint32_t)UPPL_EVENTS_KT_PORT);
      ser_evt.addOutput(*obj_key_port);
      ser_evt.addOutput(new_val_port);
      ser_evt.addOutput(old_val_port);
      PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
      // Notify operstatus modifications
      status = (UpplReturnCode) physical_layer
          ->get_ipc_connection_manager()->SendEvent(&ser_evt);
    } else {
      pfc_log_info("Update alarm status in db status %d", status);
    }
    val_port_st_t *val_port =
        reinterpret_cast<val_port_st_t*>(old_val_struct);
    if (val_port != NULL) {
      delete val_port;
      val_port = NULL;
    }
  } else {
    pfc_log_error("Reading alarm status from db failed");
  }
  return status;
}

/** IsKeyExists
 * * @Description : This function checks whether the port_id exists in DB
 * * * @param[in] : data type and key value
 * * * @return    : Success or associated error code
 * */
UpplReturnCode Kt_Port::IsKeyExists(unc_keytype_datatype_t data_type,
                                    vector<string> key_values) {
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  UpplReturnCode check_status = UPPL_RC_SUCCESS;
  if (key_values.empty()) {
    // No key given, return failure
    pfc_log_error("No key given. Returning error");
    return UPPL_RC_ERR_BAD_REQUEST;
  }
  string controller_name = key_values[0];
  string switch_id = key_values[1];
  string port_id = key_values[2];

  // Structure used to send request to ODBC
  DBTableSchema kt_port_dbtableschema;

  // Construct Primary key list
  vector<string> vect_prim_keys;
  vect_prim_keys.push_back(CTR_NAME);
  vect_prim_keys.push_back(SWITCH_ID);
  vect_prim_keys.push_back(PORT_ID);

  // Construct TableAttrSchema structure
  // TableAttrSchema holds table_name, primary key, attr_name
  vector<TableAttrSchema> vect_table_attr_schema;
  list< vector<TableAttrSchema> > row_list;

  // controller_name
  PhyUtil::FillDbSchema(CTR_NAME, controller_name,
                        controller_name.length(),
                        DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);
  // switch_id
  PhyUtil::FillDbSchema(SWITCH_ID, switch_id,
                        switch_id.length(), DATATYPE_UINT8_ARRAY_256,
                        vect_table_attr_schema);
  // port_id
  PhyUtil::FillDbSchema(PORT_ID, port_id,
                        port_id.length(), DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);

  kt_port_dbtableschema.set_table_name(UPPL_PORT_TABLE);
  kt_port_dbtableschema.set_primary_keys(vect_prim_keys);
  row_list.push_back(vect_table_attr_schema);
  kt_port_dbtableschema.set_row_list(row_list);

  // Send request to ODBC for port_table
  ODBCM_RC_STATUS check_db_status = physical_layer->get_odbc_manager()->\
      IsRowExists(data_type, kt_port_dbtableschema);
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

/** HandleOperStatus
 * * @Description : This function performs the required actions when oper status
 * changes
 * * * @param[in] : Key and value struct
 * * * @return    : UPPL_RC_SUCCESS or UPPL_RC_ERR*
 * */
UpplReturnCode Kt_Port::HandleOperStatus(uint32_t data_type,
                                         void *key_struct,
                                         void *value_struct) {
  FN_START_TIME("HandleOperStatus", "Port");
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  UpplReturnCode return_code = UPPL_RC_SUCCESS;

  if (key_struct == NULL) {
    FN_END_TIME("HandleOperStatus", "Port");
    return UPPL_RC_ERR_BAD_REQUEST;
  }

  key_port_t *obj_key_port =
      reinterpret_cast<key_port_t*>(key_struct);
  string controller_name =
      (const char*) obj_key_port->sw_key.ctr_key.controller_name;
  string switch_id =
      (const char*) obj_key_port->sw_key.switch_id;
  string port_id =
      (const char*) obj_key_port->port_id;
  // Get the controller's oper status and decide on the oper_status
  key_ctr_t ctr_key;
  memcpy(ctr_key.controller_name, controller_name.c_str(),
         controller_name.length()+1);
  uint8_t ctrl_oper_status = 0;
  UpplPortOperStatus port_oper_status = UPPL_PORT_OPER_UNKNOWN;
  Kt_Controller controller;
  UpplReturnCode read_status = controller.GetOperStatus(
      data_type, reinterpret_cast<void*>(&ctr_key), ctrl_oper_status);
  if (read_status == UPPL_RC_SUCCESS) {
    pfc_log_debug("Controller's oper_status %d", ctrl_oper_status);
    if (ctrl_oper_status ==
        (UpplControllerOperStatus) UPPL_CONTROLLER_OPER_UP) {
      pfc_log_info("Set Port oper status as up");
      port_oper_status = UPPL_PORT_OPER_UP;
    }
  } else {
    pfc_log_info("Controller's oper_status read returned failure");
  }
  // Get the switch oper status and decide on the oper_status
  key_switch_t sw_key;
  memcpy(sw_key.ctr_key.controller_name, controller_name.c_str(),
         controller_name.length()+1);
  memcpy(sw_key.switch_id, switch_id.c_str(),
         switch_id.length()+1);
  uint8_t switch_oper_status = 0;
  Kt_Switch switch1;
  read_status = switch1.GetOperStatus(
      data_type, reinterpret_cast<void*>(&sw_key), switch_oper_status);
  if (read_status == UPPL_RC_SUCCESS) {
    pfc_log_debug("Switch's oper_status %d", switch_oper_status);
    if (switch_oper_status ==
        (UpplSwitchOperStatus) UPPL_SWITCH_OPER_UP) {
      pfc_log_info("Set Port oper status as up");
      port_oper_status = UPPL_PORT_OPER_UP;
    }
  } else {
    pfc_log_debug("Switch oper_status read returned failure");
  }
  // Update oper_status in port table
  return_code = SetOperStatus(data_type,
                              key_struct,
                              port_oper_status, true);
  pfc_log_debug("Set Port oper status status %d", return_code);

  // Call referred classes' notify oper_status functions
  // Get all ports associated with switch
  vector<key_port_t> vectPortKey;
  port_id.clear();
  while (true) {
    DBTableSchema kt_port_dbtableschema;
    vector<TableAttrSchema> vect_table_attr_schema;
    list< vector<TableAttrSchema> > row_list;
    vector<string> vect_prim_keys;
    vect_prim_keys.push_back(CTR_NAME);
    vect_prim_keys.push_back(SWITCH_ID);
    if (!switch_id.empty()) {
      // one more primary key is required
      vect_prim_keys.push_back(PORT_ID);
    }
    pfc_log_debug(
        "Get Bulk Rows called with controller_name %s, switch_id %s"
        "port_id %s", controller_name.c_str(), switch_id.c_str(),
        port_id.c_str());
    PhyUtil::FillDbSchema(CTR_NAME, controller_name,
                          controller_name.length(), DATATYPE_UINT8_ARRAY_32,
                          vect_table_attr_schema);

    PhyUtil::FillDbSchema(SWITCH_ID, switch_id,
                          switch_id.length(), DATATYPE_UINT8_ARRAY_256,
                          vect_table_attr_schema);

    PhyUtil::FillDbSchema(PORT_ID, port_id,
                          port_id.length(), DATATYPE_UINT8_ARRAY_32,
                          vect_table_attr_schema);

    kt_port_dbtableschema.set_table_name(UPPL_PORT_TABLE);
    kt_port_dbtableschema.set_primary_keys(vect_prim_keys);
    row_list.push_back(vect_table_attr_schema);
    kt_port_dbtableschema.set_row_list(row_list);

    ODBCM_RC_STATUS db_status = physical_layer->get_odbc_manager()->
        GetBulkRows((unc_keytype_datatype_t)UNC_DT_STATE, UPPL_MAX_REP_CT,
                    kt_port_dbtableschema,
                    (unc_keytype_operation_t)UNC_OP_READ_SIBLING_BEGIN);
    if (db_status != ODBCM_RC_SUCCESS) {
      pfc_log_info("ReadBulk failure");
      break;
    }
    list<vector<TableAttrSchema> > ::iterator iter_list;
    for (iter_list = kt_port_dbtableschema.row_list_.begin();
        iter_list != kt_port_dbtableschema.row_list_.end();
        ++iter_list) {
      vector<TableAttrSchema> attributes_vector = *iter_list;
      vector<TableAttrSchema> :: iterator iter_vector;
      key_port_t port;
      memset(port.sw_key.ctr_key.controller_name, '\0', 32);
      memset(port.sw_key.switch_id, '\0', 256);
      memset(port.port_id, '\0', 32);
      for (iter_vector = attributes_vector.begin();
          iter_vector != attributes_vector.end();
          ++iter_vector) {
        // Get attribute name of a row
        TableAttrSchema tab_att_schema = (*iter_vector);
        if (tab_att_schema.table_attribute_name == CTR_NAME) {
          PhyUtil::GetValueFromDbSchema(tab_att_schema,
                                        controller_name,
                                        DATATYPE_UINT8_ARRAY_32);
          memcpy(port.sw_key.ctr_key.controller_name,
                 controller_name.c_str(),
                 controller_name.length()+1);
        } else if (tab_att_schema.table_attribute_name == SWITCH_ID) {
          PhyUtil::GetValueFromDbSchema(tab_att_schema, switch_id,
                                        DATATYPE_UINT8_ARRAY_32);
          memcpy(port.sw_key.switch_id,
                 switch_id.c_str(),
                 switch_id.length()+1);
        } else if (tab_att_schema.table_attribute_name == PORT_ID) {
          PhyUtil::GetValueFromDbSchema(tab_att_schema, port_id,
                                        DATATYPE_UINT8_ARRAY_320);
          memcpy(port.port_id, port_id.c_str(),
                 port_id.length()+1);
        }
      }
      vectPortKey.push_back(port);
    }
  }
  vector<key_port_t>::iterator keyItr =
      vectPortKey.begin();
  for (; keyItr != vectPortKey.end(); ++keyItr) {
    // key_port_t objKeyPort;
    return_code = NotifyOperStatus(
        data_type, reinterpret_cast<void *> (&(*keyItr)), NULL);
    pfc_log_debug("Notify Oper status return %d", return_code);
  }
  FN_END_TIME("HandleOperStatus", "Port");
  return return_code;
}

/** NotifyOperStatus
 * * @Description : This function performs the notifies other associated
 * key types when oper status changes
 * * * @param[in] : Key and value struct
 * * * @return    : UPPL_RC_SUCCESS or UPPL_RC_ERR*
 * */
UpplReturnCode Kt_Port::NotifyOperStatus(uint32_t data_type,
                                         void *key_struct,
                                         void *value_struct) {
  UpplReturnCode return_code = UPPL_RC_SUCCESS;
  key_port_t *obj_key_port =
      reinterpret_cast<key_port_t*>(key_struct);

  string controller_name =
      (const char*)obj_key_port->sw_key.ctr_key.controller_name;
  string switch_id =
      (const char*)obj_key_port->sw_key.switch_id;
  string port_id =
      (const char*)obj_key_port->port_id;
  pfc_log_debug("Kt_Port::NotifyOperStatus port_id %s",
                port_id.c_str());

  // Notify Kt_Logical_Port
  key_logical_port_t logical_port_key;
  memset(&logical_port_key, 0, sizeof(key_logical_port_t));
  memcpy(logical_port_key.domain_key.ctr_key.controller_name,
         controller_name.c_str(),
         controller_name.length()+1);
  memset(logical_port_key.domain_key.domain_name, '\0', 32);
  memset(logical_port_key.port_id, '\0', 320);
  Kt_LogicalPort port_obj;
  val_logical_port_st_t port_val_obj;
  memset(&port_val_obj, 0, sizeof(val_logical_port_st_t));
  memset(port_val_obj.logical_port.valid, 0, 5);
  memcpy(port_val_obj.logical_port.switch_id,
         switch_id.c_str(), switch_id.length()+1);
  port_val_obj.logical_port.valid[kIdxLogicalPortSwitchId] = UNC_VF_VALID;
  memcpy(port_val_obj.logical_port.physical_port_id, port_id.c_str(),
         (port_id.length())+1);
  port_val_obj.logical_port.valid[kIdxLogicalPortPhysicalPortId] = UNC_VF_VALID;
  return_code = port_obj.HandleOperStatus(
      data_type, reinterpret_cast<void *> (&logical_port_key),
      reinterpret_cast<void *> (&port_val_obj));
  pfc_log_info("HandleOperStatus for port class %d",
               return_code);

  // Notify UNC_KT_LINK
  Kt_Link link;
  key_link link_key;
  memcpy(link_key.ctr_key.controller_name,
         controller_name.c_str(),
         controller_name.length()+1);
  memcpy(link_key.port_id1,
         port_id.c_str(),
         port_id.length()+1);
  memset(link_key.port_id2, 0, 32);
  memcpy(link_key.switch_id1,
         switch_id.c_str(),
         switch_id.length()+1);
  memset(link_key.switch_id2, 0, 256);
  return_code = link.HandleOperStatus(
      data_type, reinterpret_cast<void *> (&link_key),
      NULL);
  pfc_log_info("HandleOperStatus for link class %d",
               return_code);
  key_link link_key1;
  memcpy(link_key1.ctr_key.controller_name,
         controller_name.c_str(),
         controller_name.length()+1);
  memcpy(link_key1.port_id2,
         port_id.c_str(),
         port_id.length()+1);
  memset(link_key1.port_id1, 0, 32);
  memcpy(link_key1.switch_id2,
         switch_id.c_str(),
         switch_id.length()+1);
  memset(link_key1.switch_id1, 0, 256);
  return_code = link.HandleOperStatus(
      data_type, reinterpret_cast<void *> (&link_key1),
      NULL);
  pfc_log_info("HandleOperStatus for link class %d",
               return_code);
  return return_code;
}


/** GetOperStatus
 *  * @Description : This function reads the oper_status value of the port
 *  * @param[in] : key_struct
 *  * @return    : oper_status
 */
UpplReturnCode Kt_Port::GetOperStatus(uint32_t data_type,
                                      void* key_struct,
                                      uint8_t &oper_status) {
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  key_port *obj_key_port =
      reinterpret_cast<key_port_t*>(key_struct);
  TableAttrSchema kt_port_table_attr_schema;
  vector<TableAttrSchema> vect_table_attr_schema;
  list < vector<TableAttrSchema> > row_list;
  vector<string> vect_prim_keys;

  string controller_name = (const char*)obj_key_port->sw_key.
      ctr_key.controller_name;

  if (!controller_name.empty()) {
    vect_prim_keys.push_back(CTR_NAME);
  }

  PhyUtil::FillDbSchema(CTR_NAME, controller_name,
                        controller_name.length(), DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);

  string switch_id = (const char*)obj_key_port->
      sw_key.switch_id;
  pfc_log_info("switch_id: %s", switch_id.c_str());
  if (!switch_id.empty()) {
    vect_prim_keys.push_back(SWITCH_ID);
  }

  PhyUtil::FillDbSchema(SWITCH_ID, switch_id,
                        switch_id.length(), DATATYPE_UINT8_ARRAY_256,
                        vect_table_attr_schema);

  string port_id = (const char*)obj_key_port->port_id;
  if (!port_id.empty()) {
    vect_prim_keys.push_back(PORT_ID);
  }

  PhyUtil::FillDbSchema(PORT_ID, port_id,
                        port_id.length(), DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);

  string oper_value;
  PhyUtil::FillDbSchema(PORT_OPER_STATUS, oper_value,
                        oper_value.length(), DATATYPE_UINT16,
                        vect_table_attr_schema);

  DBTableSchema kt_port_dbtableschema;
  kt_port_dbtableschema.set_table_name(UPPL_PORT_TABLE);
  kt_port_dbtableschema.set_primary_keys(vect_prim_keys);
  row_list.push_back(vect_table_attr_schema);
  kt_port_dbtableschema.set_row_list(row_list);

  // Call ODBCManager and update
  ODBCM_RC_STATUS update_db_status =
      physical_layer->get_odbc_manager()->GetOneRow(
          (unc_keytype_datatype_t)data_type,
          kt_port_dbtableschema);
  if (update_db_status != ODBCM_RC_SUCCESS) {
    // log error
    pfc_log_error("oper_status read operation failed");
    return UPPL_RC_ERR_DB_GET;
  }

  // read the oper_status value
  list < vector<TableAttrSchema> > res_port_row_list =
      kt_port_dbtableschema.get_row_list();
  list < vector<TableAttrSchema> >::iterator res_port_iter =
      res_port_row_list.begin();
  // populate IPC value structure based on the response received from DB
  for (; res_port_iter!= res_port_row_list.end(); ++res_port_iter) {
    vector<TableAttrSchema> res_port_table_attr_schema = (*res_port_iter);
    vector<TableAttrSchema>:: iterator vect_port_iter =
        res_port_table_attr_schema.begin();
    for (; vect_port_iter != res_port_table_attr_schema.end();
        ++vect_port_iter) {
      // populate values from port_table
      TableAttrSchema tab_schema = (*vect_port_iter);
      string attr_name = tab_schema.table_attribute_name;
      string attr_value;
      if (attr_name == PORT_OPER_STATUS) {
        PhyUtil::GetValueFromDbSchema(tab_schema, attr_value,
                                      DATATYPE_UINT16);
        oper_status = atoi(attr_value.c_str());
        pfc_log_info("oper_status: %d", oper_status);
        break;
      }
    }
  }
  return UPPL_RC_SUCCESS;
}

/** GetAlarmStatus
 * * @Description: This function updates the alarm status in db
 * * @param[in]: key_struct - key structure of kt Port
 * alarm status - specifies the alarm status
 * * @return:  UPPL_RC_SUCCESS or UPPL_RC_ERR*
 * */

UpplReturnCode Kt_Port::GetAlarmStatus(uint32_t data_type,
                                       void* key_struct,
                                       uint64_t &alarms_status) {
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  key_port *obj_key_port =
      reinterpret_cast<key_port_t*>(key_struct);
  TableAttrSchema kt_port_table_attr_schema;
  vector<TableAttrSchema> vect_table_attr_schema;
  list < vector<TableAttrSchema> > row_list;
  vector<string> vect_prim_keys;
  vect_prim_keys.push_back(CTR_NAME);
  vect_prim_keys.push_back(SWITCH_ID);
  vect_prim_keys.push_back(PORT_ID);

  string controller_name =
      (const char*)obj_key_port->sw_key.ctr_key.controller_name;

  PhyUtil::FillDbSchema(CTR_NAME, controller_name,
                        controller_name.length(), DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);
  string switch_id = (const char*)obj_key_port->sw_key.switch_id;

  PhyUtil::FillDbSchema(SWITCH_ID, switch_id,
                        switch_id.length(), DATATYPE_UINT8_ARRAY_256,
                        vect_table_attr_schema);

  string port_id = (const char*)obj_key_port->port_id;

  PhyUtil::FillDbSchema(PORT_ID, port_id,
                        port_id.length(), DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);
  string empty_alarms_status;
  PhyUtil::FillDbSchema(PORT_ALARM_STATUS, empty_alarms_status,
                        empty_alarms_status.length(), DATATYPE_UINT64,
                        vect_table_attr_schema);
  DBTableSchema kt_port_dbtableschema;
  kt_port_dbtableschema.set_table_name(UPPL_PORT_TABLE);
  kt_port_dbtableschema.set_primary_keys(vect_prim_keys);
  row_list.push_back(vect_table_attr_schema);
  kt_port_dbtableschema.set_row_list(row_list);

  // Call ODBCManager and Update
  ODBCM_RC_STATUS update_db_status =
      physical_layer->get_odbc_manager()->GetOneRow(
          (unc_keytype_datatype_t)data_type,
          kt_port_dbtableschema);
  if (update_db_status != ODBCM_RC_SUCCESS) {
    // log error
    pfc_log_error("oper_status read operation failed");
    return UPPL_RC_ERR_DB_GET;
  }

  // read the oper status value
  list < vector<TableAttrSchema> > res_port_row_list =
      kt_port_dbtableschema.get_row_list();
  list < vector<TableAttrSchema> >::iterator res_port_iter =
      res_port_row_list.begin();

  // populate IPC value structure based on the response recevied from DB
  for (; res_port_iter!= res_port_row_list.end(); ++res_port_iter) {
    vector<TableAttrSchema> res_port_table_attr_schema = (*res_port_iter);
    vector<TableAttrSchema>:: iterator vect_port_iter =
        res_port_table_attr_schema.begin();
    for (; vect_port_iter != res_port_table_attr_schema.end();
        ++vect_port_iter) {
      // populate values from port_table
      TableAttrSchema tab_schema = (*vect_port_iter);
      string attr_name = tab_schema.table_attribute_name;
      string attr_value;
      if (attr_name == "alarms_status") {
        PhyUtil::GetValueFromDbSchema(tab_schema, attr_value,
                                      DATATYPE_UINT64);
        alarms_status = atol(attr_value.c_str());
        break;
      }
    }
  }
  return UPPL_RC_SUCCESS;
}

/** SetOperStatus
 *  * @Description : This function updates the oper_status value
 *  of the controller
 *  * @param[in] : key_struct
 *  * @return    : oper_status
 */
UpplReturnCode Kt_Port::SetOperStatus(uint32_t data_type,
                                      void* key_struct,
                                      UpplPortOperStatus oper_status,
                                      bool is_single_key) {
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  key_port_t *obj_key_port=
      reinterpret_cast<key_port_t*>(key_struct);
  TableAttrSchema kt_port_table_attr_schema;
  vector<TableAttrSchema> vect_table_attr_schema;
  list < vector<TableAttrSchema> > row_list;
  vector<string> vect_prim_keys;
  string controller_name = (const char*)obj_key_port->sw_key.
      ctr_key.controller_name;
  string switch_id = (const char*)obj_key_port->
      sw_key.switch_id;
  string port_id = (const char*)obj_key_port->port_id;

  if (!controller_name.empty()) {
    vect_prim_keys.push_back(CTR_NAME);
  }

  PhyUtil::FillDbSchema(CTR_NAME, controller_name,
                        controller_name.length(), DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);
  if (is_single_key == false) {
    if (!switch_id.empty()) {
      vect_prim_keys.push_back(SWITCH_ID);
      PhyUtil::FillDbSchema(SWITCH_ID, switch_id,
                            switch_id.length(), DATATYPE_UINT8_ARRAY_32,
                            vect_table_attr_schema);
    }

    if (!port_id.empty()) {
      vect_prim_keys.push_back(PORT_ID);
      PhyUtil::FillDbSchema(PORT_ID, port_id,
                            port_id.length(), DATATYPE_UINT8_ARRAY_32,
                            vect_table_attr_schema);
    }
  }

  string oper_value = PhyUtil::uint8tostr(oper_status);
  PhyUtil::FillDbSchema(PORT_OPER_STATUS, oper_value,
                        oper_value.length(), DATATYPE_UINT16,
                        vect_table_attr_schema);

  DBTableSchema kt_port_dbtableschema;
  kt_port_dbtableschema.set_table_name(UPPL_PORT_TABLE);
  kt_port_dbtableschema.set_primary_keys(vect_prim_keys);
  row_list.push_back(vect_table_attr_schema);
  kt_port_dbtableschema.set_row_list(row_list);

  // Call ODBCManager and update
  ODBCM_RC_STATUS update_db_status =
      physical_layer->get_odbc_manager()->UpdateOneRow(
          (unc_keytype_datatype_t)data_type,
          kt_port_dbtableschema);
  if (update_db_status != ODBCM_RC_SUCCESS) {
    // log error
    pfc_log_error("oper_status update operation failed");
    return UPPL_RC_ERR_DB_UPDATE;
  } else {
    // Notify operstatus change to northbound
    val_port_st old_val_port, new_val_port;
    uint8_t old_oper_status = 0;
    UpplReturnCode read_status = GetOperStatus(data_type,
                                               key_struct,
                                               old_oper_status);
    if (read_status == UPPL_RC_SUCCESS) {
      memset(old_val_port.valid, 0, 3);
      old_val_port.oper_status = old_oper_status;
      old_val_port.valid[kIdxPortOperStatus] = UNC_VF_VALID;
      memset(new_val_port.valid, 0, 3);
      new_val_port.oper_status = oper_status;
      new_val_port.valid[kIdxPortOperStatus] = UNC_VF_VALID;
      int err = 0;
      // Send notification to Northbound
      ServerEvent ser_evt((pfc_ipcevtype_t)UPPL_EVENTS_KT_PORT, err);
      ser_evt.addOutput((uint32_t)UNC_OP_UPDATE);
      ser_evt.addOutput(data_type);
      ser_evt.addOutput((uint32_t)UPPL_EVENTS_KT_PORT);
      ser_evt.addOutput(*obj_key_port);
      ser_evt.addOutput(new_val_port);
      ser_evt.addOutput(old_val_port);
      PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
      // Notify operstatus modifications
      UpplReturnCode status = (UpplReturnCode) physical_layer
          ->get_ipc_connection_manager()->SendEvent(&ser_evt);
      pfc_log_debug("Event notification status %d", status);
    }
  }
  return UPPL_RC_SUCCESS;
}

/** PopulateDBSchemaForKtTable
 * * @Description : This function populates the DBAttrSchema to be used to send
 *                  request to ODBC
 * * * @param[in] : DBTableSchema, key_struct, val_struct, operation_type
 * * * @return    : UPPL_RC_SUCCESS or UPPL_RC_ERR*
 * */
void Kt_Port::PopulateDBSchemaForKtTable(
    DBTableSchema &kt_port_dbtableschema,
    void* key_struct,
    void* val_struct,
    uint8_t operation_type,
    uint32_t option1,
    uint32_t option2,
    vector<ODBCMOperator> &vect_key_operations,
    void* &old_val_struct,
    CsRowStatus row_status,
    pfc_bool_t is_filtering,
    pfc_bool_t is_state) {
  // Construct Primary key list
  vector<string> vect_prim_keys;
  // construct TableAttrSchema structure
  // TableAttrSchema holds table_name, primary key, attr_name
  vector<TableAttrSchema> vect_table_attr_schema;
  list < vector<TableAttrSchema> > row_list;

  key_port_t *obj_key_port = reinterpret_cast<key_port_t*>(key_struct);
  val_port_st_t *obj_val_port = reinterpret_cast<val_port_st_t*>(val_struct);

  stringstream valid;
  pfc_log_info("operation: %d", operation_type);
  // controller_name
  string controller_name = (const char*)obj_key_port
      ->sw_key.ctr_key.controller_name;
  pfc_log_info("controller name: %s", controller_name.c_str());
  if (!controller_name.empty()) {
    vect_prim_keys.push_back(CTR_NAME);
  }
  PhyUtil::FillDbSchema(CTR_NAME, controller_name,
                        controller_name.length(), DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);
  // switch_id
  string switch_id = (const char*)obj_key_port->sw_key.switch_id;
  pfc_log_info("switch id: %s", switch_id.c_str());
  if (!switch_id.empty()) {
    vect_prim_keys.push_back(SWITCH_ID);
  }
  PhyUtil::FillDbSchema(SWITCH_ID, switch_id,
                        switch_id.length(), DATATYPE_UINT8_ARRAY_256,
                        vect_table_attr_schema);
  // port_id
  string port_id = (const char*)obj_key_port->port_id;
  if (operation_type == UNC_OP_READ_SIBLING_BEGIN ||
      operation_type == UNC_OP_READ_SIBLING_COUNT) {
    // Ignore port_id
    port_id = "";
  }

  pfc_log_info("port id: %s", port_id.c_str());
  PhyUtil::FillDbSchema(PORT_ID, port_id,
                        port_id.length(), DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);

  val_port_st_t *val_port_valid_st = NULL;
  if (operation_type == UNC_OP_UPDATE) {
    // get valid array for update req
    pfc_log_debug("Get Valid value from Update Valid Flag");
    val_port_valid_st = new val_port_st_t();
    unc_keytype_validflag_t new_valid_val = UNC_VF_VALID;
    UpdatePortValidFlag(key_struct, val_struct,
                        *val_port_valid_st,
                        new_valid_val);
    old_val_struct = reinterpret_cast<void *>(val_port_valid_st);
  }

  GetPortValStructure(
      obj_val_port,
      vect_table_attr_schema,
      vect_prim_keys,
      operation_type,
      val_port_valid_st,
      valid);
  GetPortStateValStructure(
      obj_val_port,
      vect_table_attr_schema,
      vect_prim_keys,
      operation_type,
      val_port_valid_st,
      valid);
  vect_prim_keys.push_back(PORT_ID);
  PhyUtil::reorder_col_attrs(vect_prim_keys, vect_table_attr_schema);
  kt_port_dbtableschema.set_table_name(UPPL_PORT_TABLE);
  kt_port_dbtableschema.set_primary_keys(vect_prim_keys);
  row_list.push_back(vect_table_attr_schema);
  kt_port_dbtableschema.set_row_list(row_list);
  return;
}

/** FillPortValueStructure
 * * @Description : This function populates val_port_st_t by values retrieved
 * from database
 * * * @param[in] : port table dbtable schema,
 * value structure and max_rep_ct, operation type
 * * * @return    : Filled val_port_st and port id
 * */

void Kt_Port::FillPortValueStructure(
    DBTableSchema &kt_port_dbtableschema,
    vector<val_port_st_t> &vect_obj_val_port,
    uint32_t &max_rep_ct,
    uint32_t operation_type,
    vector<key_port_t> &port_id) {
  // populate IPC value structure based on the response recevied from DB
  list < vector<TableAttrSchema> > res_port_row_list =
      kt_port_dbtableschema.get_row_list();
  list < vector<TableAttrSchema> > :: iterator res_port_iter =
      res_port_row_list.begin();
  max_rep_ct = res_port_row_list.size();
  pfc_log_debug("res_port_row_list.size: %d", max_rep_ct);

  // populate IPC value structure based on the response recevied from DB
  for (; res_port_iter != res_port_row_list.end(); ++res_port_iter) {
    vector<TableAttrSchema> res_port_table_attr_schema =
        (*res_port_iter);
    vector<TableAttrSchema> :: iterator vect_port_iter =
        res_port_table_attr_schema.begin();
    val_port_st_t obj_val_port;
    memset(&obj_val_port, 0, sizeof(val_port_st_t));
    memset(&obj_val_port.port, 0, sizeof(val_port_t));
    key_port_t obj_key_port;
    memset(&obj_key_port, '\0', sizeof(obj_key_port));
    // Read all attributes
    for (; vect_port_iter != res_port_table_attr_schema.end();
        ++vect_port_iter) {
      // Populate values from port_table
      TableAttrSchema tab_schema = (*vect_port_iter);
      string attr_name = tab_schema.table_attribute_name;
      string attr_value;
      if (attr_name == CTR_NAME) {
        PhyUtil::GetValueFromDbSchema(tab_schema, attr_value,
                                      DATATYPE_UINT8_ARRAY_32);
        memcpy(obj_key_port.sw_key.ctr_key.controller_name,
               attr_value.c_str(),
               attr_value.length()+1);
        pfc_log_debug("controller_name: %s", reinterpret_cast<char *>
        (&obj_key_port.sw_key.ctr_key.controller_name));
      }
      if (attr_name == SWITCH_ID) {
        PhyUtil::GetValueFromDbSchema(tab_schema, attr_value,
                                      DATATYPE_UINT8_ARRAY_256);
        memcpy(obj_key_port.sw_key.switch_id,
               attr_value.c_str(),
               attr_value.length()+1);
        pfc_log_debug("switch_id: %s", reinterpret_cast<char *>
        (&obj_key_port.sw_key.switch_id));
      }
      if (attr_name == PORT_ID) {
        PhyUtil::GetValueFromDbSchema(tab_schema, attr_value,
                                      DATATYPE_UINT8_ARRAY_32);
        memcpy(obj_key_port.port_id,
               attr_value.c_str(),
               attr_value.length()+1);
        pfc_log_debug("port_id: %s", reinterpret_cast<char *>
        (&obj_key_port.port_id));
      }
      if (attr_name == PORT_NUMBER) {
        PhyUtil::GetValueFromDbSchema(tab_schema, attr_value,
                                      DATATYPE_UINT32);
        obj_val_port.port.port_number = atoi(attr_value.c_str());
        pfc_log_debug("port_number: %d", obj_val_port.port.port_number);
      }
      if (attr_name == PORT_DESCRIPTION) {
        PhyUtil::GetValueFromDbSchema(tab_schema, attr_value,
                                      DATATYPE_UINT8_ARRAY_128);
        memcpy(obj_val_port.port.description,
               attr_value.c_str(),
               attr_value.length()+1);
        pfc_log_debug("description: %s", obj_val_port.port.description);
      }
      if (attr_name == PORT_ADMIN_STATUS) {
        PhyUtil::GetValueFromDbSchema(tab_schema, attr_value,
                                      DATATYPE_UINT16);
        obj_val_port.port.admin_status = atoi(attr_value.c_str());
        pfc_log_debug("admin_status: %d", obj_val_port.port.admin_status);
      }
      if (attr_name == PORT_TRUNK_ALL_VLAN) {
        PhyUtil::GetValueFromDbSchema(tab_schema, attr_value,
                                      DATATYPE_UINT16);
        obj_val_port.port.trunk_allowed_vlan =
            atoi((const char*)attr_value.c_str());
        pfc_log_debug("trunk_allowed_vlan: %s", attr_value.c_str());
      }
      if (attr_name == PORT_OPER_STATUS) {
        PhyUtil::GetValueFromDbSchema(tab_schema, attr_value,
                                      DATATYPE_UINT16);
        obj_val_port.oper_status = atoi(attr_value.c_str());
        pfc_log_debug("oper status : %d", obj_val_port.oper_status);
      }
      if (attr_name == PORT_MAC_ADDRESS) {
        PhyUtil::GetValueFromDbSchema(tab_schema, attr_value,
                                      DATATYPE_UINT8_ARRAY_6);
        memcpy(obj_val_port.mac_address,
               attr_value.c_str(),
               attr_value.length()+1);
        pfc_log_debug("Mac_address: %s", obj_val_port.mac_address);
      }
      if (attr_name == PORT_DIRECTION) {
        PhyUtil::GetValueFromDbSchema(tab_schema, attr_value,
                                      DATATYPE_UINT16);
        obj_val_port.direction = atoi(attr_value.c_str());
        pfc_log_debug("direction: %d", obj_val_port.direction);
      }
      if (attr_name == PORT_DUPLEX) {
        PhyUtil::GetValueFromDbSchema(tab_schema, attr_value,
                                      DATATYPE_UINT16);
        obj_val_port.duplex = atoi(attr_value.c_str());
        pfc_log_debug("duplex : %d", obj_val_port.duplex);
      }
      if (attr_name == PORT_SPEED) {
        PhyUtil::GetValueFromDbSchema(tab_schema, attr_value,
                                      DATATYPE_UINT64);
        pfc_log_debug("speed from DB : %s", attr_value.c_str());
        obj_val_port.speed = atol(attr_value.c_str());
        pfc_log_debug("speed : %" PFC_PFMT_u64, obj_val_port.speed);
        pfc_log_debug("speed PFC_PFMT_u64 : %" PFC_PFMT_u64 "...",
                      obj_val_port.speed);
      }
      if (attr_name == PORT_ALARM_STATUS) {
        PhyUtil::GetValueFromDbSchema(tab_schema, attr_value,
                                      DATATYPE_UINT64);
        obj_val_port.alarms_status = atol(attr_value.c_str());
        pfc_log_debug("alarms_status : %" PFC_PFMT_u64,
                      obj_val_port.alarms_status);
      }
      if (attr_name == PORT_LOGIC_PORT_ID) {
        PhyUtil::GetValueFromDbSchema(tab_schema, attr_value,
                                      DATATYPE_UINT8_ARRAY_320);
        memcpy(obj_val_port.logical_port_id,
               attr_value.c_str(),
               attr_value.length()+1);
        pfc_log_debug("Logical_port_id: %s", obj_val_port.logical_port_id);
      }
      if (attr_name == PORT_VALID) {
        memset(obj_val_port.valid, 0, 8);
        obj_val_port.valid[kIdxPortSt] = UNC_VF_VALID;
        PhyUtil::GetValueFromDbSchema(tab_schema, attr_value,
                                      DATATYPE_UINT8_ARRAY_11);
        memset(obj_val_port.port.valid, '\0', 4);
        FrameValidValue(attr_value, obj_val_port);
        pfc_log_debug("valid: %s", attr_value.c_str());
      }
    }
    vect_obj_val_port.push_back(obj_val_port);
    // populate key structure
    port_id.push_back(obj_key_port);
    pfc_log_debug("result - vect_obj_val_port size: %d",
                  (unsigned int) vect_obj_val_port.size());
    pfc_log_debug("result - vect_port_id size: %d",
                  (unsigned int) port_id.size());
  }
  return;
}

/** PerformRead
 * * @Description : This function reads the instance of KT_Port based on
 *                  operation type - READ, READ_SIBLING_BEGIN, READ_SIBLING
 * * * @param[in] : ipc session id, configuration id, key_struct, value_struct,
 *                  data_type, operation type, ServerSession, option1, option2,
 *                  max_rep_ct
 * * * @return    : UPPL_RC_SUCCESS or UPPL_RC_ERR*
 * */
UpplReturnCode Kt_Port::PerformRead(uint32_t session_id,
                                    uint32_t configuration_id,
                                    void* key_struct,
                                    void* val_struct,
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
  key_port_t *obj_key_port =
      reinterpret_cast<key_port_t*>(key_struct);
  UpplReturnCode read_status = UPPL_RC_SUCCESS;
  physical_response_header rsh = {session_id,
      configuration_id,
      operation_type,
      max_rep_ct,
      option1,
      option2,
      data_type,
      read_status};
  if (operation_type == UNC_OP_READ) {
    max_rep_ct = 1;
  }

  if ((unc_keytype_datatype_t)data_type != UNC_DT_STATE) {
    pfc_log_error("Read operation is provided on unsupported data type");
    rsh.result_code = UPPL_RC_ERR_OPERATION_NOT_ALLOWED;
    int err = PhyUtil::sessOutRespHeader(sess, rsh);
    err |= sess.addOutput((uint32_t)UNC_KT_PORT);
    err |= sess.addOutput(*obj_key_port);
    if (err != 0) {
      pfc_log_debug("addOutput failed for physical_response_header");
      return UPPL_RC_ERR_IPC_WRITE_ERROR;
    }
  }

  if (option1 != UNC_OPT1_NORMAL) {
    // Invalid operation
    pfc_log_error("PerformRead provided on unsupported option1");
    rsh.result_code = UPPL_RC_ERR_INVALID_OPTION1;
    int err = PhyUtil::sessOutRespHeader(sess, rsh);
    err |= sess.addOutput((uint32_t)UNC_KT_PORT);
    err |= sess.addOutput(*obj_key_port);
    if (err != 0) {
      pfc_log_debug("addOutput failed for physical_response_header");
      return UPPL_RC_ERR_IPC_WRITE_ERROR;
    }
    return UPPL_RC_SUCCESS;
  }

  if (option2 != UNC_OPT2_NEIGHBOR && option2 != UNC_OPT2_NONE) {
    pfc_log_error("PerformRead provided on unsupported option2");
    rsh.result_code = UPPL_RC_ERR_INVALID_OPTION2;
    int err = PhyUtil::sessOutRespHeader(sess, rsh);
    err |= sess.addOutput((uint32_t)UNC_KT_PORT);
    err |= sess.addOutput(*obj_key_port);
    if (err != 0) {
      pfc_log_debug("addOutput failed for physical_response_header");
      return UPPL_RC_ERR_IPC_WRITE_ERROR;
    }
    return UPPL_RC_SUCCESS;
  }

  if (option2 == UNC_OPT2_NONE) {
    // For DT_STATE with option2 as none, populate val_port_st
    pfc_log_debug("Populating val_port_st from DB");
    vector<key_port_t> vect_port_id;
    vector<val_port_st_t> vect_val_port_st;
    read_status = ReadPortValFromDB(key_struct,
                                    val_struct,
                                    data_type,
                                    operation_type,
                                    max_rep_ct,
                                    vect_val_port_st,
                                    vect_port_id);
    rsh.result_code = read_status;
    rsh.max_rep_count = max_rep_ct;
    pfc_log_info("read status val in performread = %d", read_status);
    if (read_status != UPPL_RC_SUCCESS) {
      rsh.max_rep_count = 0;
      int err = PhyUtil::sessOutRespHeader(sess, rsh);
      err |= sess.addOutput((uint32_t) UNC_KT_PORT);
      err |= sess.addOutput(*obj_key_port);
      if (err != 0) {
        pfc_log_debug("addOutput failed for physical_response_header");
        return UPPL_RC_ERR_IPC_WRITE_ERROR;
      }
      return UPPL_RC_SUCCESS;
    }
    int err = PhyUtil::sessOutRespHeader(sess, rsh);
    if (err != 0) {
      pfc_log_debug("addOutput failed for physical_response_header");
      return UPPL_RC_ERR_IPC_WRITE_ERROR;
    }
    for (unsigned int index = 0; index < vect_port_id.size();
        ++index) {
      sess.addOutput((uint32_t)UNC_KT_PORT);
      sess.addOutput((key_port_t)vect_port_id[index]);
      sess.addOutput(vect_val_port_st[index]);
      if (index < vect_port_id.size() - 1) {
        sess.addOutput();  //  Seperator
      }
    }

  } else if (option2 == UNC_OPT2_NEIGHBOR) {
    pfc_log_info("Read neighbor details from DB");
    val_port_st_neighbor obj_neighbor;
    read_status = ReadNeighbor(
        key_struct,
        val_struct,
        data_type,
        obj_neighbor);
    pfc_log_info("Return value for read operation %d", read_status);
    rsh.result_code = read_status;
    int err = PhyUtil::sessOutRespHeader(sess, rsh);
    err |= sess.addOutput((uint32_t)UNC_KT_PORT);
    err |= sess.addOutput(*obj_key_port);
    if (err != 0) {
      pfc_log_debug("addOutput failed for physical_response_header");
      return UPPL_RC_ERR_IPC_WRITE_ERROR;
    }
    if (read_status == UPPL_RC_SUCCESS) {
      err = sess.addOutput(obj_neighbor);
      if (err != 0) {
        pfc_log_debug("addOutput failed for physical_response_header");
        return UPPL_RC_ERR_IPC_WRITE_ERROR;
      }
    }  else {
      pfc_log_info("Read operation on neighbor failed");
    }
  }
  return UPPL_RC_SUCCESS;
}

/** ReadPortValFromDB
 * * @Description : This function reads the instance of KT_Port based on
 *                  operation type - READ, READ_SIBLING_BEGIN, READ_SIBLING
 *                   from data base
 * * * @param[in] : key_struct, value_struct, ipc session id, configuration id,
 *                  data_type, operation type, max_rep_ct
 * * * @return    : UPPL_RC_SUCCESS or UPPL_RC_ERR*
 * */
UpplReturnCode Kt_Port::ReadPortValFromDB(
    void* key_struct,
    void* val_struct,
    uint32_t data_type,
    uint32_t operation_type,
    uint32_t &max_rep_ct,
    vector<val_port_st_t> &vect_val_port_st,
    vector<key_port_t> &port_id) {
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  UpplReturnCode read_status = UPPL_RC_SUCCESS;
  ODBCM_RC_STATUS read_db_status = ODBCM_RC_SUCCESS;

  // Common structures that will be used to send query to ODBC
  // Structure used to send request to ODBC
  DBTableSchema kt_port_dbtableschema;
  void *old_struct;
  vector<ODBCMOperator> vect_key_operations;
  PopulateDBSchemaForKtTable(kt_port_dbtableschema,
                             key_struct,
                             val_struct,
                             operation_type, 0, 0,
                             vect_key_operations, old_struct);

  if (operation_type == UNC_OP_READ) {
    read_db_status = physical_layer->get_odbc_manager()->
        GetOneRow((unc_keytype_datatype_t)data_type,
                  kt_port_dbtableschema);
  } else {
    read_db_status = physical_layer->get_odbc_manager()->
        GetBulkRows((unc_keytype_datatype_t)data_type, max_rep_ct,
                    kt_port_dbtableschema,
                    (unc_keytype_operation_t)operation_type);
  }
  if (read_db_status == ODBCM_RC_RECORD_NOT_FOUND) {
    pfc_log_debug("No record found");
    read_status = UPPL_RC_ERR_NO_SUCH_INSTANCE;
    return read_status;
  } else if (read_db_status == ODBCM_RC_CONNECTION_ERROR) {
    // log error to log daemon
    pfc_log_error("DB connection not available or cannot access DB");
    read_status = UPPL_RC_ERR_DB_ACCESS;
    return read_status;
  } else if (read_db_status != ODBCM_RC_SUCCESS) {
    read_status = UPPL_RC_ERR_DB_GET;
    // log error to log daemon
    pfc_log_error("Read operation has failed: %d", read_db_status);
    return read_status;
  }

  pfc_log_debug("Read operation Success result: %d", read_status);
  FillPortValueStructure(kt_port_dbtableschema,
                         vect_val_port_st,
                         max_rep_ct,
                         UNC_OP_READ,
                         port_id);
  pfc_log_debug("vect_val_port_st size: %d",
                (unsigned int)vect_val_port_st.size());
  pfc_log_debug("port_id size: %d", (unsigned int)port_id.size());
  if (vect_val_port_st.empty()) {
    // Read failed , return error
    read_status = UPPL_RC_ERR_DB_GET;
    // log error to log daemon
    pfc_log_error("Read operation has failed after reading response");
    return read_status;
  }
  pfc_log_debug("Read operation Completed with result: %d", read_status);
  return read_status;
}

/** ReadNeighbor
 * * @Description : This function reads the Neighbor information from driver
 * * * @param[in] : Session id, configuration id, key_struct, value_struct,
 *  data_type, operation type
 * * * @return    : UPPL_RC_SUCCESS or UPPL_RC_ERR*
 * */
UpplReturnCode Kt_Port::ReadNeighbor(
    void* key_struct,
    void* val_struct,
    uint32_t data_type,
    val_port_st_neighbor &neighbor_obj) {
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  UpplReturnCode read_status = UPPL_RC_SUCCESS;
  key_port_t *obj_key_port= reinterpret_cast<key_port_t*>(key_struct);
  string controller_name = (const char*)obj_key_port->
      sw_key.ctr_key.controller_name;
  string switch_id = (const char*)obj_key_port->sw_key.switch_id;
  string port_id = (const char*)obj_key_port->port_id;
  string switch_id2, port_id2;
  // Read Port value from DB
  vector<void *> vect_key_port;
  vect_key_port.push_back(reinterpret_cast<void *>(obj_key_port));
  vector<void *> vect_val_port;
  read_status = ReadInternal(vect_key_port,
                             vect_val_port,
                             data_type,
                             UNC_OP_READ);
  if (read_status != UPPL_RC_SUCCESS) {
    // unable to read value from Db for port
    pfc_log_info("Neighbour fails - unable to read port value");
    return read_status;
  }
  val_port_st_t *obj_val_st_port =
      reinterpret_cast<val_port_st_t*>(vect_val_port[0]);
  if (obj_val_st_port == NULL) {
    // unable to read value from Db for port
    pfc_log_info("Neighbour fails - unable to read port value");
    return UPPL_RC_ERR_NO_SUCH_INSTANCE;
  }
  val_port_t obj_val_port = obj_val_st_port->port;
  DBTableSchema kt_port_dbtableschema;
  vector<string> vect_prim_keys;
  vect_prim_keys.push_back(CTR_NAME);
  vect_prim_keys.push_back(LINK_SWITCH_ID1);
  vect_prim_keys.push_back(LINK_PORT_ID1);
  vector<TableAttrSchema> vect_table_attr_schema;
  list< vector<TableAttrSchema> > row_list;
  PhyUtil::FillDbSchema(CTR_NAME, controller_name,
                        controller_name.length(), DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);
  PhyUtil::FillDbSchema(LINK_SWITCH_ID1, switch_id,
                        switch_id.length(), DATATYPE_UINT8_ARRAY_256,
                        vect_table_attr_schema);
  PhyUtil::FillDbSchema(LINK_PORT_ID1, port_id,
                        port_id.length(), DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);
  PhyUtil::FillDbSchema(LINK_SWITCH_ID2, switch_id2,
                        switch_id2.length(), DATATYPE_UINT8_ARRAY_256,
                        vect_table_attr_schema);
  PhyUtil::FillDbSchema(LINK_PORT_ID2, port_id2,
                        port_id2.length(), DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);
  kt_port_dbtableschema.set_table_name(UPPL_LINK_TABLE);
  kt_port_dbtableschema.set_primary_keys(vect_prim_keys);
  row_list.push_back(vect_table_attr_schema);
  kt_port_dbtableschema.set_row_list(row_list);

  ODBCM_RC_STATUS read_db_status = physical_layer->get_odbc_manager()->
      GetOneRow((unc_keytype_datatype_t)data_type, kt_port_dbtableschema);
  if (read_db_status == ODBCM_RC_SUCCESS) {
    read_status = UPPL_RC_SUCCESS;
    // populate IPC value structure based on the response recevied from DB
    vector<TableAttrSchema> res_table_attr_schema =
        kt_port_dbtableschema.get_row_list().front();
    vector<TableAttrSchema> ::iterator vect_iter =
        res_table_attr_schema.begin();
    memset(neighbor_obj.valid, UNC_VF_VALID, 3);
    neighbor_obj.port = obj_val_port;
    for (; vect_iter != res_table_attr_schema.end(); ++vect_iter) {
      TableAttrSchema tab_schema = (*vect_iter);
      string attr_name = tab_schema.table_attribute_name;
      if (attr_name == LINK_SWITCH_ID2) {
        string switch_id2;
        PhyUtil::GetValueFromDbSchema(tab_schema, switch_id2,
                                      DATATYPE_UINT8_ARRAY_256);
        memcpy(neighbor_obj.connected_switch_id,
               switch_id2.c_str(), switch_id2.length()+1);
        pfc_log_debug("Connected switch id: %s", switch_id2.c_str());
      } else if (attr_name == LINK_PORT_ID2) {
        string port_id2;
        PhyUtil::GetValueFromDbSchema(tab_schema, port_id2,
                                      DATATYPE_UINT8_ARRAY_32);
        memcpy(neighbor_obj.connected_port_id,
               port_id2.c_str(), port_id2.length()+1);
        pfc_log_debug("Connected port id: %s", port_id2.c_str());
      }
    }
  } else {
    if (read_db_status == ODBCM_RC_RECORD_NOT_FOUND) {
      pfc_log_info("No record to read in ReadNeighbor");
      read_status = UPPL_RC_ERR_NO_SUCH_INSTANCE;
    } else if (read_db_status == ODBCM_RC_CONNECTION_ERROR) {
      // log error to log daemon
      pfc_log_error("DB connection not available or cannot access DB");
      read_status = UPPL_RC_ERR_DB_ACCESS;
    } else if (read_db_status != ODBCM_RC_SUCCESS) {
      read_status = UPPL_RC_ERR_DB_GET;
      pfc_log_info("ReadNeighbor operation has failed with %d", read_status);
    }
  }
  delete obj_val_st_port;
  obj_val_st_port = NULL;
  key_link_t *link_key =
      reinterpret_cast<key_link_t*>(vect_key_port[0]);
  if (link_key != NULL) {
    delete link_key;
    link_key = NULL;
  }
  return read_status;
}

/** Fill_Attr_Syntax_Map
 * * @Description : This function populates the values to be used for attribute
 * validation
 * * * @param[in] : None
 * * * @return    : None
 * */
void Kt_Port::Fill_Attr_Syntax_Map() {
  Kt_Class_Attr_Syntax objKeyAttrCtrSyntax =
  { PFC_IPCTYPE_UINT8, 0, 0, 1, 32, true, "" };
  attr_syntax_map[CTR_NAME] = objKeyAttrCtrSyntax;

  Kt_Class_Attr_Syntax objKeyAttrSwitchSyntax =
  { PFC_IPCTYPE_UINT8, 0, 0, 1, 32, true, "" };
  attr_syntax_map[SWITCH_ID] = objKeyAttrSwitchSyntax;

  Kt_Class_Attr_Syntax objKeyAttrPortSyntax =
  { PFC_IPCTYPE_UINT8, 0, 0, 1, 32, true, "" };
  attr_syntax_map[PORT_ID] = objKeyAttrPortSyntax;

  Kt_Class_Attr_Syntax objAttrPortNumberSyntax =
  { PFC_IPCTYPE_UINT32, 0, 100000, 0, 0, false, "" };
  attr_syntax_map[PORT_NUMBER] = objAttrPortNumberSyntax;

  Kt_Class_Attr_Syntax objAttrDescSyntax =
  { PFC_IPCTYPE_STRING, 0, 0, 0, 128, false, "" };
  attr_syntax_map[PORT_DESCRIPTION] = objAttrDescSyntax;

  Kt_Class_Attr_Syntax objAttrAdminStatusSyntax =
  { PFC_IPCTYPE_UINT8, 0, 1, 0, 0, false, "" };
  attr_syntax_map[PORT_ADMIN_STATUS] = objAttrAdminStatusSyntax;

  Kt_Class_Attr_Syntax objAttrDirectionSyntax =
  { PFC_IPCTYPE_UINT8, 0, 2, 0, 0, false, "" };
  attr_syntax_map[PORT_DIRECTION] = objAttrDirectionSyntax;

  Kt_Class_Attr_Syntax objAttrTrunkAVlanSyntax =
  { PFC_IPCTYPE_UINT16, 0, 65536, 0, 0, false, "" };
  attr_syntax_map[PORT_TRUNK_ALL_VLAN] = objAttrTrunkAVlanSyntax;

  Kt_Class_Attr_Syntax objAttrOperStatusSyntax =
  { PFC_IPCTYPE_UINT8, 0, 2, 0, 0, false, "" };
  attr_syntax_map[PORT_OPER_STATUS] = objAttrOperStatusSyntax;

  Kt_Class_Attr_Syntax objAttrMacAddressSyntax =
  { PFC_IPCTYPE_STRING, 0, 0, 0, 6, false, "" };
  attr_syntax_map[PORT_MAC_ADDRESS] = objAttrMacAddressSyntax;

  Kt_Class_Attr_Syntax objAttrDuplexSyntax =
  { PFC_IPCTYPE_UINT8, 0, 1, 0, 0, false, "" };
  attr_syntax_map[PORT_DUPLEX] = objAttrDuplexSyntax;

  Kt_Class_Attr_Syntax objAttrSpeedSyntax =
  { PFC_IPCTYPE_UINT64, 0, 0, 0, 0, false, "" };
  attr_syntax_map[PORT_SPEED] = objAttrSpeedSyntax;

  Kt_Class_Attr_Syntax objAttrAlarmsStatusSyntax =
  { PFC_IPCTYPE_UINT64, 1, 3, 0, 0, false, "" };
  attr_syntax_map[PORT_ALARM_STATUS] = objAttrAlarmsStatusSyntax;

  Kt_Class_Attr_Syntax objAttrLogicalPortIdSyntax =
  { PFC_IPCTYPE_STRING, 0, 0, 0, 320, false, "" };
  attr_syntax_map[PORT_LOGIC_PORT_ID] = objAttrLogicalPortIdSyntax;

  Kt_Class_Attr_Syntax objAttrValidSyntax =
  { PFC_IPCTYPE_STRING, 0, 0, 0, 12, false, "" };
  attr_syntax_map[PORT_VALID] = objAttrValidSyntax;
}

/** UpdatePortValidFlag
 * * @Description : This function forms the valid flag based on update req
 * * * @param[in] : Key, value struct and newvalid val
 * * * @return    : Success or associated error code
 * */
UpplReturnCode Kt_Port::UpdatePortValidFlag(
    void *key_struct,
    void *val_struct,
    val_port_st_t &val_port_valid_st,
    unc_keytype_validflag_t new_valid_val) {
  UpplReturnCode return_code = UPPL_RC_SUCCESS;
  // read the value structure from db
  void *obj_key_port_orig = key_struct;
  vector<void *> vect_key_port;
  vect_key_port.push_back(key_struct);
  vector<void *> vect_val_port;
  return_code = ReadInternal(vect_key_port, vect_val_port,
                             UNC_DT_STATE, UNC_OP_READ);
  if (return_code != UPPL_RC_SUCCESS) {
    pfc_log_info("Read is not success");
    return return_code;
  }
  val_port_st_t *obj_val_port =
      reinterpret_cast<val_port_st_t*>(val_struct);
  val_port_st_t *obj_new_val_port =
      reinterpret_cast<val_port_st_t*>(vect_val_port[0]);
  if (obj_new_val_port == NULL) {
    pfc_log_debug("update port valid ret null val");
    return return_code;
  }
  if (new_valid_val == UNC_VF_VALID) {
    val_port_valid_st = *obj_new_val_port;

    delete obj_new_val_port;
    key_port_t *port_key =
        reinterpret_cast<key_port_t*> (vect_key_port[0]);
    if (port_key != NULL) {
      delete port_key;
    }
    return return_code;
  }
  if (obj_val_port != NULL) {
    uint32_t operation_type = UNC_OP_UPDATE;

    // Get the valid value from req received, verify the valid status
    // For all valid attribs update stream to valid, for invalid attribs
    // update stream with valid value read from db.
    // store the final valid val to stream
    stringstream ss_new;

    // port number
    unsigned int valid_val =
        PhyUtil::uint8touint(obj_val_port->port.valid[kIdxPortNumber]);
    if (PhyUtil::IsValidValue(operation_type, valid_val) == true) {
      pfc_log_debug("valid value port_number: %d", valid_val);
      ss_new << new_valid_val;
    } else {
      ss_new << obj_new_val_port->port.valid[kIdxPortNumber];
      pfc_log_debug("invalid value for port_number ignore the value");
    }

    // valid val of desc
    valid_val =
        PhyUtil::uint8touint(obj_val_port->port.valid[kIdxPortDescription]);
    if (PhyUtil::IsValidValue(operation_type, valid_val) == true) {
      pfc_log_debug("valid val desc: %d", valid_val);
      ss_new << new_valid_val;
    } else {
      ss_new << obj_new_val_port->port.valid[kIdxPortDescription];
      pfc_log_debug("invalid value for desc ignore the value");
    }

    // valid val of admin_status
    valid_val =
        PhyUtil::uint8touint(obj_val_port->port.valid[kIdxPortAdminStatus]);
    if (PhyUtil::IsValidValue(operation_type, valid_val) == true) {
      pfc_log_debug("valid value admin_status: %d", valid_val);
      ss_new << new_valid_val;
    } else {
      ss_new << obj_new_val_port->port.valid[kIdxPortAdminStatus];
      pfc_log_debug("invalid value for admin_status ignore the value");
    }

    // valid val of trunkallowed
    valid_val =
        PhyUtil::uint8touint(
            obj_val_port->port.valid[kIdxPortTrunkAllowedVlan]);
    if (PhyUtil::IsValidValue(operation_type, valid_val) == true) {
      pfc_log_debug("valid value trunkallowed: %d", valid_val);
      ss_new << new_valid_val;
    } else {
      ss_new << obj_new_val_port->port.valid[kIdxPortTrunkAllowedVlan];
      pfc_log_debug("invalid value for trunkallowed ignore the value");
    }

    // valid val of portoperstatus
    valid_val =
        PhyUtil::uint8touint(obj_val_port->valid[kIdxPortOperStatus]);
    if (PhyUtil::IsValidValue(operation_type, valid_val) == true) {
      pfc_log_debug("valid value operstatus: %d", valid_val);
      ss_new << new_valid_val;

    } else {
      ss_new << obj_new_val_port->valid[kIdxPortOperStatus];
      pfc_log_debug("invalid value for operstatus ignore the value");
    }

    // valid val of macaddress
    valid_val =
        PhyUtil::uint8touint(obj_val_port->valid[kIdxPortMacAddress]);
    if (PhyUtil::IsValidValue(operation_type, valid_val) == true) {
      pfc_log_debug("valid value macaddress: %d", valid_val);
      ss_new << new_valid_val;
    } else {
      ss_new << obj_new_val_port->valid[kIdxPortMacAddress];
      pfc_log_debug("invalid value for macaddress ignore the value");
    }

    // valid val of portdirection
    valid_val =
        PhyUtil::uint8touint(obj_val_port->valid[kIdxPortDirection]);
    if (PhyUtil::IsValidValue(operation_type, valid_val) == true) {
      pfc_log_debug("valid value portdirection: %d", valid_val);
      ss_new << new_valid_val;
    } else {
      ss_new << obj_new_val_port->valid[kIdxPortDirection];
      pfc_log_debug("invalid value for portdirection ignore the value");
    }

    // valid val of portduplex
    valid_val = PhyUtil::uint8touint(obj_val_port->valid[kIdxPortDuplex]);
    if (PhyUtil::IsValidValue(operation_type, valid_val) == true) {
      pfc_log_debug("valid value portduplex: %d", valid_val);
      ss_new << new_valid_val;
    } else {
      ss_new << obj_new_val_port->valid[kIdxPortDuplex];
      pfc_log_debug("invalid value for portduplex ignore the value");
    }

    // valid val of portspeed
    valid_val = PhyUtil::uint8touint(obj_val_port->valid[kIdxPortSpeed]);
    if (PhyUtil::IsValidValue(operation_type, valid_val) == true) {
      pfc_log_debug("valid value portspeed: %d", valid_val);
      ss_new << new_valid_val;
    } else {
      ss_new << obj_new_val_port->valid[kIdxPortSpeed];
      pfc_log_debug("invalid value for portspeed ignore the value");
    }

    // valid val of alrmstatus
    valid_val =
        PhyUtil::uint8touint(obj_val_port->valid[kIdxPortAlarmsStatus]);
    if (PhyUtil::IsValidValue(operation_type, valid_val) == true) {
      pfc_log_debug("valid value alrmstatus: %d", valid_val);
      ss_new << new_valid_val;
    } else {
      ss_new << obj_new_val_port->valid[kIdxPortAlarmsStatus];
      pfc_log_debug("invalid value for alrmstatus ignore the value");
    }

    // valid val of logicalportid
    valid_val =
        PhyUtil::uint8touint(obj_val_port->valid[kIdxPortLogicalPortId]);
    if (PhyUtil::IsValidValue(operation_type, valid_val) == true) {
      pfc_log_debug("valid value logicalportid: %d", valid_val);
      ss_new << new_valid_val;
    } else {
      ss_new << obj_new_val_port->valid[kIdxPortLogicalPortId];
      pfc_log_debug("invalid value for logicalportid ignore the value");
    }
    pfc_log_debug("updateport final valid val:%s", ss_new.str().c_str());
    // call populate schema for valid update
    return_code = PopulateSchemaForValidFlag(
        // key_struct,
        obj_key_port_orig,
        reinterpret_cast<void*> (&obj_new_val_port),
        ss_new.str().c_str());
    delete obj_new_val_port;
    obj_new_val_port = NULL;
    key_port_t *key_port = reinterpret_cast<key_port_t*>(vect_key_port[0]);
    if (key_port != NULL) {
      delete key_port;
      key_port = NULL;
    }
  }
  return return_code;
}

/** PopulateSchemaForValidFlag
 *  * @Description : This function updates the valid flag value
 *  of the port
 *  * @param[in] : key_struct, value_struct, valid val
 *  * @return    : success/failure
 */
UpplReturnCode Kt_Port::PopulateSchemaForValidFlag(void* key_struct,
                                                   void* val_struct,
                                                   string valid_new) {
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  key_port_t *obj_key_port=
      reinterpret_cast<key_port_t*>(key_struct);
  TableAttrSchema kt_port_table_attr_schema;
  vector<TableAttrSchema> vect_table_attr_schema;
  list < vector<TableAttrSchema> > row_list;

  vector<string> vect_prim_keys;
  vect_prim_keys.push_back(CTR_NAME);
  vect_prim_keys.push_back(SWITCH_ID);
  vect_prim_keys.push_back(PORT_ID);


  string controller_name = (const char*)obj_key_port->sw_key.
      ctr_key.controller_name;
  string switch_id = (const char*)obj_key_port->
      sw_key.switch_id;
  string port_id = (const char*)obj_key_port->port_id;
  if (!controller_name.empty()) {
    PhyUtil::FillDbSchema(CTR_NAME, controller_name,
                          (controller_name.length()+1), DATATYPE_UINT8_ARRAY_32,
                          vect_table_attr_schema);
  }

  if (!switch_id.empty()) {
    PhyUtil::FillDbSchema(SWITCH_ID, switch_id,
                          (switch_id.length()+1), DATATYPE_UINT8_ARRAY_256,
                          vect_table_attr_schema);
  }

  if (!port_id.empty()) {
    PhyUtil::FillDbSchema(PORT_ID, port_id,
                          (port_id.length()+1), DATATYPE_UINT8_ARRAY_32,
                          vect_table_attr_schema);
  }

  pfc_log_debug("valid new val:%s", valid_new.c_str());
  PhyUtil::FillDbSchema(PORT_VALID, valid_new.c_str(),
                        11, DATATYPE_UINT8_ARRAY_11,
                        vect_table_attr_schema);

  DBTableSchema kt_port_dbtableschema;
  kt_port_dbtableschema.set_table_name(UPPL_PORT_TABLE);
  kt_port_dbtableschema.set_primary_keys(vect_prim_keys);
  row_list.push_back(vect_table_attr_schema);
  kt_port_dbtableschema.set_row_list(row_list);

  // Call ODBCManager and update
  ODBCM_RC_STATUS update_db_status =
      physical_layer->get_odbc_manager()->UpdateOneRow(UNC_DT_STATE,
                                                       kt_port_dbtableschema);
  if (update_db_status != ODBCM_RC_SUCCESS) {
    // log error
    pfc_log_error("port valid update operation failed");
    return UPPL_RC_ERR_DB_UPDATE;
  }
  return UPPL_RC_SUCCESS;
}

/** FrameValidValue
 * * @Description : This function converts the string value from db to uint8
 * * * @param[in] : Attribute value and val_ctr_st
 * * * @return    : Success or associated error code
 * */
void Kt_Port::FrameValidValue(string attr_value,
                              val_port_st &obj_val_port) {
  for (unsigned int index = 0; index < attr_value.length(); ++index) {
    unsigned int valid = attr_value[index];
    if (attr_value[index] >= 48) {
      valid = attr_value[index] - 48;
    }
    if (index > 3) {
      obj_val_port.valid[index-3] = valid;
    } else {
      obj_val_port.port.valid[index] = valid;
    }
  }
  return;
}

/** GetPortValStructure
 * * @Description : This function reads the values given in val_port structure
 * * @param[in] : DBTableSchema - DBtableschema associated with KT_Port
 * key_struct - key instance of Kt_Switch
 * val_struct - val instance of Kt_Switch
 * operation_type - UNC_OP*
 * * @return    : UPPL_RC_SUCCESS or UPPL_RC_ERR*
 * */
void Kt_Port::GetPortValStructure(
    val_port_st_t *obj_val_port,
    vector<TableAttrSchema> &vect_table_attr_schema,
    vector<string> &vect_prim_keys,
    uint8_t operation_type,
    val_port_st_t *val_port_valid_st,
    stringstream &valid) {
  uint16_t valid_val = 0, prev_db_val = 0;
  unsigned int valid_value_struct = UNC_VF_VALID;
  if (obj_val_port != NULL) {
    valid_value_struct = PhyUtil::uint8touint(
        obj_val_port->valid[kIdxPortSt]);
  }
  string value;
  // Port_number
  if (obj_val_port != NULL && valid_value_struct == UNC_VF_VALID) {
    valid_val = PhyUtil::uint8touint(
        obj_val_port->port.valid[kIdxPortNumber]);
    value = PhyUtil::uint8tostr(obj_val_port->port.port_number);
    if (operation_type == UNC_OP_UPDATE) {
      prev_db_val =
          PhyUtil::uint8touint(
              val_port_valid_st->port.valid[kIdxPortNumber]);
    }
  } else {
    valid_val = UPPL_NO_VAL_STRUCT;
  }
  PhyUtil::FillDbSchema(PORT_NUMBER, value,
                        value.length(), DATATYPE_UINT32,
                        operation_type, valid_val, prev_db_val,
                        vect_table_attr_schema, vect_prim_keys, valid);
  value.clear();
  // Description
  if (obj_val_port != NULL && valid_value_struct == UNC_VF_VALID) {
    valid_val = PhyUtil::uint8touint(
        obj_val_port->port.valid[kIdxPortDescription]);
    value = (const char*)obj_val_port->port.description;
    if (operation_type == UNC_OP_UPDATE) {
      prev_db_val =
          PhyUtil::uint8touint(
              val_port_valid_st->port.valid[kIdxPortDescription]);
    }
  } else {
    valid_val = UPPL_NO_VAL_STRUCT;
  }
  PhyUtil::FillDbSchema(PORT_DESCRIPTION, value,
                        value.length(), DATATYPE_UINT8_ARRAY_128,
                        operation_type, valid_val, prev_db_val,
                        vect_table_attr_schema, vect_prim_keys, valid);
  value.clear();
  // Admin_status
  if (obj_val_port != NULL && valid_value_struct == UNC_VF_VALID) {
    valid_val = PhyUtil::uint8touint(
        obj_val_port->port.valid[kIdxPortAdminStatus]);
    value = PhyUtil::uint8tostr(obj_val_port->port.admin_status);
    if (operation_type == UNC_OP_UPDATE) {
      prev_db_val =
          PhyUtil::uint8touint(
              val_port_valid_st->port.valid[kIdxPortAdminStatus]);
    }
  } else {
    valid_val = UPPL_NO_VAL_STRUCT;
  }
  PhyUtil::FillDbSchema(PORT_ADMIN_STATUS, value,
                        value.length(), DATATYPE_UINT16,
                        operation_type, valid_val, prev_db_val,
                        vect_table_attr_schema, vect_prim_keys, valid);
  value.clear();
  // trunk_allowed_vlan
  if (obj_val_port != NULL && valid_value_struct == UNC_VF_VALID) {
    valid_val = PhyUtil::uint8touint(obj_val_port->port.valid\
                                     [kIdxPortTrunkAllowedVlan]);
    value = PhyUtil::uint8tostr(obj_val_port->\
                                port.trunk_allowed_vlan);
    if (operation_type == UNC_OP_UPDATE) {
      prev_db_val =
          PhyUtil::uint8touint(
              val_port_valid_st->port.valid[kIdxPortTrunkAllowedVlan]);
    }
  } else {
    valid_val = UPPL_NO_VAL_STRUCT;
  }
  PhyUtil::FillDbSchema(PORT_TRUNK_ALL_VLAN, value,
                        value.length(), DATATYPE_UINT16,
                        operation_type, valid_val, prev_db_val,
                        vect_table_attr_schema, vect_prim_keys, valid);
  return;
}

/** GetPortStateValStructure
 * * @Description : This function reads the values given in val_port_st structure
 * * @param[in] : DBTableSchema - DBtableschema associated with KT_Port
 * key_struct - key instance of Kt_Switch
 * val_struct - val instance of Kt_Switch
 * operation_type - UNC_OP*
 * * @return    : UPPL_RC_SUCCESS or UPPL_RC_ERR*
 * */
void Kt_Port::GetPortStateValStructure(
    val_port_st_t *obj_val_port,
    vector<TableAttrSchema> &vect_table_attr_schema,
    vector<string> &vect_prim_keys,
    uint8_t operation_type,
    val_port_st_t *val_port_valid_st,
    stringstream &valid) {
  string value;
  uint16_t valid_val = 0, prev_db_val = 0;
  // oper status
  if (obj_val_port != NULL) {
    valid_val = PhyUtil::uint8touint(obj_val_port->
                                     valid[kIdxPortOperStatus]);
    if (valid_val == UNC_VF_VALID) {
      value = PhyUtil::uint8tostr(obj_val_port->oper_status);
    }
    if (operation_type == UNC_OP_UPDATE) {
      prev_db_val =
          PhyUtil::uint8touint(
              val_port_valid_st->valid[kIdxPortOperStatus]);
    }
  } else {
    valid_val = UPPL_NO_VAL_STRUCT;
  }
  PhyUtil::FillDbSchema(PORT_OPER_STATUS, value,
                        value.length(), DATATYPE_UINT16,
                        operation_type, valid_val, prev_db_val,
                        vect_table_attr_schema, vect_prim_keys, valid);
  value.clear();
  // Mac_address
  if (obj_val_port != NULL) {
    valid_val = PhyUtil::uint8touint(obj_val_port->valid[kIdxPortMacAddress]);
    if (valid_val == UNC_VF_VALID) {
      value = (const char*)obj_val_port->mac_address;
    }
    if (operation_type == UNC_OP_UPDATE) {
      prev_db_val =
          PhyUtil::uint8touint(
              val_port_valid_st->valid[kIdxPortMacAddress]);
    }
  } else {
    valid_val = UPPL_NO_VAL_STRUCT;
  }
  PhyUtil::FillDbSchema(PORT_MAC_ADDRESS, value,
                        value.length(), DATATYPE_UINT8_ARRAY_6,
                        operation_type, valid_val, prev_db_val,
                        vect_table_attr_schema, vect_prim_keys, valid);
  value.clear();
  // direction
  if (obj_val_port != NULL) {
    valid_val = PhyUtil::uint8touint(obj_val_port->valid[kIdxPortDirection]);
    if (valid_val == UNC_VF_VALID) {
      value = PhyUtil::uint8tostr(obj_val_port->direction);
    }
    if (operation_type == UNC_OP_UPDATE) {
      prev_db_val =
          PhyUtil::uint8touint(
              val_port_valid_st->valid[kIdxPortDirection]);
    }
  } else {
    valid_val = UPPL_NO_VAL_STRUCT;
  }
  PhyUtil::FillDbSchema(PORT_DIRECTION, value,
                        value.length(), DATATYPE_UINT16,
                        operation_type, valid_val, prev_db_val,
                        vect_table_attr_schema, vect_prim_keys, valid);
  value.clear();
  // duplex
  if (obj_val_port != NULL) {
    valid_val = PhyUtil::uint8touint(obj_val_port->valid[kIdxPortDuplex]);
    if (valid_val == UNC_VF_VALID) {
      value = PhyUtil::uint8tostr(obj_val_port->duplex);
    }
    if (operation_type == UNC_OP_UPDATE) {
      prev_db_val =
          PhyUtil::uint8touint(
              val_port_valid_st->valid[kIdxPortDuplex]);
    }
  } else {
    valid_val = UPPL_NO_VAL_STRUCT;
  }
  PhyUtil::FillDbSchema(PORT_DUPLEX, value,
                        value.length(), DATATYPE_UINT16,
                        operation_type, valid_val, prev_db_val,
                        vect_table_attr_schema, vect_prim_keys, valid);
  value.clear();
  // speed
  if (obj_val_port != NULL) {
    valid_val = PhyUtil::uint8touint(obj_val_port->valid[kIdxPortSpeed]);
    if (valid_val == UNC_VF_VALID) {
      value = PhyUtil::uint64tostr(obj_val_port->speed);
    }
    if (operation_type == UNC_OP_UPDATE) {
      prev_db_val =
          PhyUtil::uint8touint(
              val_port_valid_st->valid[kIdxPortSpeed]);
    }
  } else {
    valid_val = UPPL_NO_VAL_STRUCT;
  }
  PhyUtil::FillDbSchema(PORT_SPEED, value,
                        value.length(), DATATYPE_UINT64,
                        operation_type, valid_val, prev_db_val,
                        vect_table_attr_schema, vect_prim_keys, valid);
  value.clear();
  // alarms_status
  if (obj_val_port != NULL) {
    valid_val = PhyUtil::uint8touint(obj_val_port->valid[kIdxPortAlarmsStatus]);
    if (valid_val == UNC_VF_VALID) {
      value = PhyUtil::uint64tostr(obj_val_port->alarms_status);
    }
    if (operation_type == UNC_OP_UPDATE) {
      prev_db_val =
          PhyUtil::uint8touint(
              val_port_valid_st->valid[kIdxPortAlarmsStatus]);
    }
  } else {
    valid_val = UPPL_NO_VAL_STRUCT;
  }
  if (operation_type == UNC_OP_CREATE) {
    valid_val = UNC_VF_VALID;
  }
  PhyUtil::FillDbSchema(PORT_ALARM_STATUS, value,
                        value.length(), DATATYPE_UINT64,
                        operation_type, valid_val, prev_db_val,
                        vect_table_attr_schema, vect_prim_keys, valid);
  value.clear();
  // Logical_port_id
  if (obj_val_port != NULL) {
    valid_val = PhyUtil::uint8touint(obj_val_port->
                                     valid[kIdxPortLogicalPortId]);
    if (valid_val == UNC_VF_VALID) {
      value = (const char*)obj_val_port->logical_port_id;
    }
    if (operation_type == UNC_OP_UPDATE) {
      prev_db_val =
          PhyUtil::uint8touint(
              val_port_valid_st->valid[kIdxPortLogicalPortId]);
    }
  } else {
    valid_val = UPPL_NO_VAL_STRUCT;
  }
  PhyUtil::FillDbSchema(PORT_LOGIC_PORT_ID, value,
                        value.length(), DATATYPE_UINT8_ARRAY_320,
                        operation_type, valid_val, prev_db_val,
                        vect_table_attr_schema, vect_prim_keys, valid);
  stringstream dummy_valid;
  valid_val = UPPL_NO_VAL_STRUCT;
  prev_db_val = 0;
  // valid
  PhyUtil::FillDbSchema(PORT_VALID, valid.str(),
                        valid.str().length(), DATATYPE_UINT8_ARRAY_11,
                        vect_table_attr_schema);
  return;
}
