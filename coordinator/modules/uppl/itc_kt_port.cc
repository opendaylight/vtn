/*
 * Copyright (c) 2012-2015 NEC Corporation
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
#include "ipct_util.hh"
#include "itc_read_request.hh"
#include "odbcm_db_varbind.hh"
#include "itc_kt_port_neighbor.hh"
using unc::uppl::PhysicalLayer;
using unc::uppl::IPCClientDriverHandler;

/** 
 * @Description : This function initializes member variables
 *                and fills the attribute syntax map used for validation
 * @param[in]   : None
 * @return      : None
 **/
Kt_Port::Kt_Port() {
  // Populate structure to be used for syntax validation
  if (attr_syntax_map_all.find(UNC_KT_PORT) == attr_syntax_map_all.end()) {
    Fill_Attr_Syntax_Map();
  }
}

/** 
 * @Description : This function does nothing
 * @param[in]   : None
 * @return      : None
 **/
Kt_Port::~Kt_Port() {
}

/** 
 * @Description : This function deletes the given  instance of KT_Port from
 *                running port table
 * @param[in]   : key_struct - void pointer to be type cast into port key type
 *                data_type - indicates the data base type,UNC_DT_* delete only
 *                allowed in STATE
 *                key_type - indicates the Key Type Class 
 * @return      : UNC_RC_SUCCESS is returned when the delete of a port in
 *                running db is successful
 *                UNC_UPPL_RC_ERR_* is returned when the delete is failure
 * */
UncRespCode Kt_Port::DeleteKeyInstance(OdbcmConnectionHandler *db_conn,
                                          void* key_struct,
                                          uint32_t data_type,
                                          uint32_t key_type) {
  UncRespCode delete_status = UNC_RC_SUCCESS;
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();

  // Check whether operation is allowed on the given DT type
  if ((unc_keytype_datatype_t)data_type != UNC_DT_STATE &&
      (unc_keytype_datatype_t)data_type != UNC_DT_IMPORT) {
    pfc_log_error("Delete operation is provided on unsupported data type");
    return UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED;
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
  memset(&logical_port_key_obj, '\0',
         sizeof(logical_port_key_obj));
  val_logical_port_t logical_port_val_obj;
  memset(&logical_port_val_obj, 0, sizeof(logical_port_val_obj));
  val_logical_port_st_t logical_port_st_val_obj;
  memset(&logical_port_st_val_obj, 0, sizeof(logical_port_st_val_obj));

  memcpy(logical_port_key_obj.domain_key.ctr_key.controller_name,
         controller_name.c_str(),
         controller_name.length()+1);

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
  UncRespCode lp_read_status = logical_port_obj.ReadInternal(
      db_conn, key_val, val_struct, data_type, UNC_OP_READ);
  if (lp_read_status == UNC_RC_SUCCESS) {
    // form key struct with all required primary keys and call delete
    logical_port_obj.DeleteKeyInstance(
        db_conn, key_val[0],
        data_type,
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
  vect_prim_keys.push_back(CTR_NAME_STR);
  vect_prim_keys.push_back(SWITCH_ID_STR);

  // Construct TableAttrSchema structure
  // TableAttrSchema holds table_name, primary key, attr_name
  vector<TableAttrSchema> vect_table_attr_schema;
  list< vector<TableAttrSchema> > row_list;

  // controller_name
  PhyUtil::FillDbSchema(unc::uppl::CTR_NAME, controller_name,
                        controller_name.length(), DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);
  // switch_id
  PhyUtil::FillDbSchema(unc::uppl::SWITCH_ID, switch_id,
                        switch_id.length(), DATATYPE_UINT8_ARRAY_256,
                        vect_table_attr_schema);

  // port_id
  if (port_id.length() > 0) {
    vect_prim_keys.push_back(PORT_ID_STR);
    PhyUtil::FillDbSchema(unc::uppl::PORT_ID, port_id,
                          port_id.length(), DATATYPE_UINT8_ARRAY_32,
                          vect_table_attr_schema);
  }

  // Send request to ODBC for port_table delete
  kt_port_dbtableschema.set_table_name(unc::uppl::PORT_TABLE);
  kt_port_dbtableschema.set_primary_keys(vect_prim_keys);
  row_list.push_back(vect_table_attr_schema);
  kt_port_dbtableschema.set_row_list(row_list);

  ODBCM_RC_STATUS delete_db_status = physical_layer->get_odbc_manager()-> \
      DeleteOneRow((unc_keytype_datatype_t)data_type,
                   kt_port_dbtableschema, db_conn);
  if (delete_db_status != ODBCM_RC_SUCCESS) {
    if (delete_db_status == ODBCM_RC_CONNECTION_ERROR) {
      // log fatal error to log daemon
      UPPL_LOG_FATAL("DB connection not available or cannot access DB");
      delete_status = UNC_UPPL_RC_ERR_DB_ACCESS;
    } else if (delete_db_status == ODBCM_RC_ROW_NOT_EXISTS) {
      pfc_log_error("given instance does not exist");
      delete_status = UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE;
    } else {
      // log error to log daemon
      delete_status = UNC_UPPL_RC_ERR_DB_DELETE;
    }
  } else {
    pfc_log_info("Delete of a port in dt(%d) is success",
                 data_type);
    delete_status = UNC_RC_SUCCESS;
  }
  return delete_status;
}

/** 
 * @Description : This function reads the given  instance of KT_Port
 * @param[in]   : key_val - vector to hold the primary key values 
 *                val_struct - vector to hold the value structure
 *                data_type - indicates the data base type, UNC_DT_* ,
 *                read allowed in DT_STATE
 *                operation_type - indicates the operation type for
 *                read operation
 * @return      : UNC_RC_SUCCESS is returned when read operation is success on
 *                DT_STATE db
 *                UNC_UPPL_RC_ERR_* is returned when read operation is failure
 **/
UncRespCode Kt_Port::ReadInternal(OdbcmConnectionHandler *db_conn,
                                     vector<void *> &key_val,
                                     vector<void *> &val_struct,
                                     uint32_t data_type,
                                     uint32_t operation_type) {
  if (operation_type != UNC_OP_READ && operation_type != UNC_OP_READ_SIBLING &&
      operation_type != UNC_OP_READ_SIBLING_BEGIN) {
    pfc_log_trace("This function not allowed for read next/bulk/count");
    return UNC_UPPL_RC_ERR_OPERATION_NOT_SUPPORTED;
  }
  pfc_log_debug("Processing Kt_Port::ReadInternal");
  vector<key_port_t> vect_port_id;
  vector<val_port_st_t> vect_val_port_st;
  uint32_t max_rep_ct = 1;
  if (operation_type != UNC_OP_READ) {
    max_rep_ct = UPPL_MAX_REP_CT;
  }
  // Get read response from database
  val_port_st_t obj_port_val;
  memset(&obj_port_val, '\0', sizeof(val_port_st_t));
  void *key_struct = key_val[0];
  void *void_val_struct = NULL;
  if ((!val_struct.empty()) && (val_struct[0] != NULL)) {
    memcpy(&obj_port_val, (reinterpret_cast <val_port_st_t*>
                                      (val_struct[0])),
           sizeof(val_port_st_t));
    void_val_struct = reinterpret_cast<void *>(&obj_port_val);
  }
  UncRespCode read_status = UNC_RC_SUCCESS;
  bool firsttime = true;
  do {
    read_status = ReadPortValFromDB(db_conn,
                                     key_struct,
                                     void_val_struct,
                                     data_type,
                                     operation_type,
                                     max_rep_ct,
                                     vect_val_port_st,
                                     vect_port_id);
    if (firsttime) {
      pfc_log_trace(
          "Clearing key_val and val_struct vectors for the first time");
      key_val.clear();
      val_struct.clear();
      firsttime = false;
    }
    if (read_status == UNC_RC_SUCCESS) {
      pfc_log_debug("ReadPortValFromDB returned %d with response size %"
                  PFC_PFMT_SIZE_T, read_status, vect_val_port_st.size());
      for (unsigned int iIndex = 0 ; iIndex < vect_val_port_st.size();
          ++iIndex) {
        key_port_t *key_port = new key_port_t(vect_port_id[iIndex]);
        val_port_st_t *val_port = new val_port_st_t(vect_val_port_st[iIndex]);
        key_val.push_back(reinterpret_cast<void *>(key_port));
        val_struct.push_back(reinterpret_cast<void *>(val_port));
      }
    } else if ((read_status == UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE &&
               val_struct.size() != 0)) {
      read_status = UNC_RC_SUCCESS;
    }
    if ((vect_val_port_st.size() == UPPL_MAX_REP_CT) &&
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
 * @Description : This function reads bulk rows of KT_Port in running port table 
 *                of specified data type as well as its parent class.
 * Order of ReadBulk response
 * val_port -> val_link -> val_boundary
 * @param[in]   : key_struct - void pointer to be type cast into port key type
 *                data_type - UNC_DT_* , read allowed in state
 *                max_rep_ct - specifies number of rows to be returned
 *                parent_call - indicates whether parent has called this
 *                readbulk
 *                is_read_next - indicates whether this function is invoked
 *                from readnext
 * @return      : UNC_RC_SUCCESS is returned when the read bulk operation is
 *                success.
 *                UNC_UPPL_RC_ERR_* is returned when read bulk operation is failure.
 **/
UncRespCode Kt_Port::ReadBulk(OdbcmConnectionHandler *db_conn,
                                 void* key_struct,
                                 uint32_t data_type,
                                 uint32_t &max_rep_ct,
                                 int child_index,
                                 pfc_bool_t parent_call,
                                 pfc_bool_t is_read_next,
                                 ReadRequest *read_req) {
  UncRespCode read_status = UNC_RC_SUCCESS;
  key_port_t *obj_key_port= reinterpret_cast<key_port_t*>(key_struct);
  if ((unc_keytype_datatype_t)data_type != UNC_DT_STATE) {
    // Not supported data type
    read_status = UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED;
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
    return UNC_RC_SUCCESS;
  }
  void *val_struct = NULL;
  vector<val_port_st_t> vect_val_port_st;
  vector<key_port_t> vect_port_id;

  // Read the port values based on given key structure
  read_status = ReadBulkInternal(
      db_conn, key_struct,
      val_struct,
      data_type,
      max_rep_ct,
      vect_val_port_st,
      vect_port_id);

  pfc_log_debug("Read status from port is %d with result size %"
                PFC_PFMT_SIZE_T, read_status, vect_port_id.size());
  if (read_status == UNC_RC_SUCCESS) {
    vector<val_port_st_t>::iterator vect_iter =
        vect_val_port_st.begin();
    vector<key_port_t> ::iterator port_iter =
        vect_port_id.begin();
    for (; (port_iter != vect_port_id.end()) &&
    (vect_iter != vect_val_port_st.end());
    ++port_iter, ++vect_iter) {
      pfc_log_debug("Iterating entries...");
      pfc_log_debug("Adding port - '%s' to session",
                    reinterpret_cast<char *>((*port_iter).port_id));
      key_port_t *key_buffer = new key_port_t(*port_iter);
      BulkReadBuffer obj_key_buffer = {
          UNC_KT_PORT, IS_KEY,
          reinterpret_cast<void*>(key_buffer)
      };
      read_req->AddToBuffer(obj_key_buffer);
      val_port_st_t *val_buffer = new val_port_st_t(*vect_iter);
      BulkReadBuffer obj_value_buffer = {
          UNC_KT_PORT, IS_STATE_VALUE,
          reinterpret_cast<void*>(val_buffer)
      };
      read_req->AddToBuffer(obj_value_buffer);
      BulkReadBuffer obj_sep_buffer = {
          UNC_KT_PORT, IS_SEPARATOR, NULL
      };
      read_req->AddToBuffer(obj_sep_buffer);
      --max_rep_ct;
      if (max_rep_ct == 0) {
        pfc_log_debug("max_rep_ct reached zero, so returning");
        return read_status;
      }
    }
  } else if (read_status == UNC_UPPL_RC_ERR_DB_ACCESS) {
    pfc_log_debug("KtPort ReadBulk - Returning DB Access Error");
    return read_status;
  }

  if (max_rep_ct > 0 && parent_call == false) {
    pfc_log_debug("port is called directly, so go to parent");
    Kt_Switch nextKin;
    key_switch_t nextkin_key_struct;
    memset(&nextkin_key_struct, '\0',
           sizeof(nextkin_key_struct));
    memcpy(nextkin_key_struct.switch_id,
           str_switch_id.c_str(),
           str_switch_id.length() +1);
    memcpy(nextkin_key_struct.ctr_key.controller_name,
           str_controller_name.c_str(),
           str_controller_name.length() +1);
    read_status = nextKin.ReadBulk(
        db_conn, reinterpret_cast<void *>(&nextkin_key_struct),
        data_type,
        max_rep_ct,
        0,
        false,
        is_read_next,
        read_req);
    pfc_log_debug("read status from next kin Kt_Switch is %d", read_status);
    return UNC_RC_SUCCESS;
  }
  return UNC_RC_SUCCESS;
}

/**ReadBulkInternal
 * @Description : This function reads bulk rows of KT_Port in
 *                port table of specified data type.
 * @param[in]   : key_struct - void pointer to be type cast into port key type
 *                val_struct - void pointer to be type cast into port value
 *                structure type
 *                max_rep_ct - specifies number of rows to be returned
 *                vect_val_port - store the structure elements of type
 *                val_port_st_t from db
 *                vect_port_id - stores the port_id values from the key
 *                structure of kt port
 * @return      : UNC_RC_SUCCESS is returned when the read bulk operation is
 *                successful.
 *                UNC_UPPL_RC_ERR_* is returned when the read bulk operation is
 *                failure
 **/
UncRespCode Kt_Port::ReadBulkInternal(
    OdbcmConnectionHandler *db_conn,
    void* key_struct,
    void* val_struct,
    uint32_t data_type,
    uint32_t max_rep_ct,
    vector<val_port_st_t> &vect_val_port,
    vector<key_port_t> &vect_port_id) {
  if (max_rep_ct <= 0) {
    return UNC_RC_SUCCESS;
  }
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  UncRespCode read_status = UNC_RC_SUCCESS;
  ODBCM_RC_STATUS read_db_status = ODBCM_RC_SUCCESS;
  DBTableSchema kt_port_dbtableschema;
  void *old_struct;
  // Populate DBSchema for port_table
  vector<ODBCMOperator> vect_key_operations;
  PopulateDBSchemaForKtTable(db_conn, kt_port_dbtableschema,
                             key_struct,
                             val_struct,
                             UNC_OP_READ_BULK, data_type, 0, 0,
                             vect_key_operations, old_struct,
                             NOTAPPLIED, false, PFC_FALSE);
  pfc_log_debug("Calling GetBulkRows");
  // Read rows from DB
  read_db_status = physical_layer->get_odbc_manager()-> \
      GetBulkRows((unc_keytype_datatype_t)data_type, max_rep_ct,
                  kt_port_dbtableschema,
                  (unc_keytype_operation_t)UNC_OP_READ_BULK, db_conn);
  pfc_log_debug("GetBulkRows return: %d", read_db_status);
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
    pfc_log_error("Read operation has failed with error %d", read_db_status);
    return read_status;
  }
  // From the values received from DB, populate val_port structure
  FillPortValueStructure(db_conn, kt_port_dbtableschema,
                         vect_val_port,
                         max_rep_ct,
                         UNC_OP_READ_BULK,
                         vect_port_id);
  return read_status;
}

/** 
 * @Description : This function performs syntax validation for key type kt_port
 * @param[in]   : key_struct - void pointer to be type cast into port key type
 *                val_struct - void pointer to be type cast into va structure of
 *                key type port
 *                operation_type - UNC_OP* - indicates the operation type
 *                data_type - UNC_DT_* - indicates the data base type
 * @return      : UNC_RC_SUCCESS is returned when the validation is successful
 *                UNC_UPPL_RC_ERR_* is returned when validation is failure
 **/
UncRespCode Kt_Port::PerformSyntaxValidation(OdbcmConnectionHandler *db_conn,
                                                void* key_struct,
                                                void* val_struct,
                                                uint32_t operation,
                                                uint32_t data_type) {
  UncRespCode ret_code = UNC_RC_SUCCESS;
  pfc_bool_t mandatory = PFC_TRUE;
  map<string, Kt_Class_Attr_Syntax> attr_syntax_map =
      attr_syntax_map_all[UNC_KT_PORT];
  // Validate key structure
  // validate controller_name
  key_port *key = reinterpret_cast<key_port_t*>(key_struct);
  string value = reinterpret_cast<char*>(key->sw_key.ctr_key.controller_name);
  IS_VALID_STRING_KEY(CTR_NAME_STR, value, operation, ret_code, mandatory);
  if (ret_code != UNC_RC_SUCCESS) {
    return UNC_UPPL_RC_ERR_CFG_SYNTAX;
  }

  // validate switch_id
  value = reinterpret_cast<char*>(key->sw_key.switch_id);
  IS_VALID_STRING_KEY(SWITCH_ID_STR, value, operation, ret_code, mandatory);
  if (ret_code != UNC_RC_SUCCESS) {
    return UNC_UPPL_RC_ERR_CFG_SYNTAX;
  }

  // validate port_id
  value = reinterpret_cast<char*>(key->port_id);
  IS_VALID_STRING_KEY(PORT_ID_STR, value, operation, ret_code, mandatory);
  if (ret_code != UNC_RC_SUCCESS) {
    return UNC_UPPL_RC_ERR_CFG_SYNTAX;
  }

  // Validate value structure
  if (val_struct != NULL) {
    unsigned int valid_val = 0;
    val_port *value_port = reinterpret_cast<val_port_t*>(val_struct);

    // validate port_number
    valid_val = PhyUtil::uint8touint(value_port->valid[kIdxPortNumber]);
    IS_VALID_INT_VALUE(PORT_NUMBER_STR, value_port->port_number, operation,
                       valid_val, ret_code, mandatory);
    if (ret_code != UNC_RC_SUCCESS) {
      return UNC_UPPL_RC_ERR_CFG_SYNTAX;
    }

    // validate description
    valid_val = PhyUtil::uint8touint(value_port->valid[kIdxPortDescription]);
    string value = reinterpret_cast<char*>(value_port->description);
    IS_VALID_STRING_VALUE(PORT_DESCRIPTION_STR, value, operation,
                          valid_val, ret_code, mandatory);
    if (ret_code != UNC_RC_SUCCESS) {
      return UNC_UPPL_RC_ERR_CFG_SYNTAX;
    }

    // validate admin_status
    valid_val = PhyUtil::uint8touint(value_port->valid[kIdxPortAdminStatus]);
    IS_VALID_INT_VALUE(PORT_ADMIN_STATUS_STR, value_port->admin_status,
                       operation, valid_val, ret_code, mandatory);
    if (ret_code != UNC_RC_SUCCESS) {
      return UNC_UPPL_RC_ERR_CFG_SYNTAX;
    }

    // validate trunk_allowed_vlan
    valid_val =
        PhyUtil::uint8touint(value_port->valid[kIdxPortTrunkAllowedVlan]);
    IS_VALID_INT_VALUE(PORT_TRUNK_ALL_VLAN_STR, value_port->trunk_allowed_vlan,
                       operation, valid_val, ret_code, mandatory);
    if (ret_code != UNC_RC_SUCCESS) {
      return UNC_UPPL_RC_ERR_CFG_SYNTAX;
    }
  }
  return ret_code;
}

/** 
 * @Description : This function performs semantic validation
 *                for UNC_KT_PORT
 * @param[in]   : key_struct - void pointer to be type cast into port key type
 *                val_struct - void pointer to be type cast into val structure
 *                of key type port
 *                operation_type - UNC_OP* - indicates the operation type
 *                data_type - UNC_DT_* - indicates the data base type
 * @return      : UNC_RC_SUCCESS is returned when the semantic validation is
 *                successful
 *                UNC_UPPL_RC_ERR_* is returned when semantic validation is failure
 * */
UncRespCode Kt_Port::PerformSemanticValidation(
    OdbcmConnectionHandler *db_conn,
    void* key_struct,
    void* val_struct,
    uint32_t operation,
    uint32_t data_type) {
  UncRespCode status = UNC_RC_SUCCESS;
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
  memset(&obj_key_switch, '\0', sizeof(obj_key_switch));
  memcpy(obj_key_switch.switch_id,
         switch_id.c_str(),
         switch_id.length()+1);

  key_value.push_back((const char*)obj_key_port->
                      sw_key.ctr_key.controller_name);

  pfc_log_debug("controller name in validate: %s", obj_key_port->
                sw_key.ctr_key.controller_name);
  memcpy(&obj_key_switch.ctr_key.controller_name,
         obj_key_port->sw_key.ctr_key.controller_name,
         sizeof(obj_key_port->sw_key.ctr_key.controller_name));

  pfc_log_debug("controller name in obj_key_port: %s",
                obj_key_switch.ctr_key.controller_name);


  vector<string> vect_key_value;
  vect_key_value.push_back(controller_name);
  vect_key_value.push_back(switch_id);
  vect_key_value.push_back(port_id);

  UncRespCode key_status = IsKeyExists(
      db_conn, (unc_keytype_datatype_t)data_type, vect_key_value);
  pfc_log_debug("IsKeyExists status %d", key_status);
  // In case of create operation, key should not exist
  if (operation == UNC_OP_CREATE) {
    if (key_status == UNC_RC_SUCCESS) {
      pfc_log_error("Key exists,CREATE not allowed");
      status = UNC_UPPL_RC_ERR_INSTANCE_EXISTS;
    } else {
      pfc_log_debug("key not exist, create oper allowed");
    }
  } else if (operation == UNC_OP_UPDATE || operation == UNC_OP_DELETE ||
      operation == UNC_OP_READ) {
    // In case of update/delete/read operation, key should exist
    if (key_status != UNC_RC_SUCCESS) {
      pfc_log_error("Key not found,U/D/R opern not allowed");
      status = UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE;
    } else {
      pfc_log_debug("key exist, update/del/read operation allowed");
    }
  }
  if (operation == UNC_OP_CREATE && status == UNC_RC_SUCCESS) {
    vector<string> parent_vect_key_value;
    parent_vect_key_value.push_back(controller_name);
    parent_vect_key_value.push_back(switch_id);
    Kt_Switch KtObj;
    UncRespCode parent_key_status = KtObj.IsKeyExists(
        db_conn, (unc_keytype_datatype_t)data_type, parent_vect_key_value);
    pfc_log_debug("Parent IsKeyExists status %d", parent_key_status);
    if (parent_key_status != UNC_RC_SUCCESS) {
      status = UNC_UPPL_RC_ERR_PARENT_DOES_NOT_EXIST;
    }
  }
  pfc_log_debug("Return Code SemanticValidation: %d", status);
  return status;
}

/**
 * @Description : This function processes the alarm notification
 *                sent by driver for port key type and sends the notification
 *                to northbound
 * @param[in]   : data_type - indicates the data base type i.e UNC_DT_STATE
 *                            or UNC_DT_IMPORT
 *                alarm type - indicates the alarm type sent by the driver
 *                oper_type - indicates the operation type i.e. UNC_OP_CREATE
 *                key_struct - void pointer type to be type cast to port
 *                key type
 *                value_struct - void pointer to be type cast to port value
 *                structure type
 * @return      : the oper status result from db will be returned
 *                UNC_RC_SUCCESS - in case oper status is received from DB
 *                UNC_UPPL_RC_ERR_* - in case unable to receive oper status from DB
 **/

UncRespCode Kt_Port::HandleDriverAlarms(OdbcmConnectionHandler *db_conn,
                                           uint32_t data_type,
                                           uint32_t alarm_type,
                                           uint32_t oper_type,
                                           void* key_struct,
                                           void* val_struct) {
  UncRespCode status = UNC_RC_SUCCESS;
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
  uint64_t alarm_status_db = 0;
  UncRespCode read_alarm_status = GetAlarmStatus(db_conn, data_type,
                                                    key_struct,
                                                    alarm_status_db);
  // Read old_alarm_status from db
  if (read_alarm_status == UNC_RC_SUCCESS) {
    uint64_t new_alarm_status = 0;
    uint64_t old_alarm_status = alarm_status_db;
    pfc_log_debug("alarm_status received from db: %" PFC_PFMT_u64,
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
    status = UpdateKeyInstance(db_conn, obj_key_port,
                               reinterpret_cast<void *>(&obj_val_port_st),
                               data_type,
                               UNC_KT_PORT,
                               old_val_struct);
    if (status == UNC_RC_SUCCESS && data_type != UNC_DT_IMPORT) {
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
      northbound_event_header rsh = {static_cast<uint32_t>(UNC_OP_UPDATE),
          data_type,
          static_cast<uint32_t>(UNC_KT_PORT)};
      err = PhyUtil::sessOutNBEventHeader(ser_evt, rsh);
      err |= ser_evt.addOutput(*obj_key_port);
      err |= ser_evt.addOutput(new_val_port);
      err |= ser_evt.addOutput(old_val_port);
      if (err == 0) {
        PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
        // Notify operstatus modifications
        status = (UncRespCode) physical_layer
            ->get_ipc_connection_manager()->SendEvent(&ser_evt,
                controller_name, UPPL_EVENTS_KT_PORT);
      } else {
        pfc_log_error("Server Event addOutput failed");
        status = UNC_UPPL_RC_ERR_IPC_WRITE_ERROR;
      }
    } else {
      pfc_log_debug("Update alarm status in db status %d", status);
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

/** 
 * @Description : This function checks whether the port_id exists in specified
 *                data base
 * @param[in]   : data type - indicates the database type
 *                key values - vector to store the primary key values of port
 *                key type
 * @return      : UNC_RC_SUCCESS - if the row exists in db with the key value
 *                specified
 *                UNC_UPPL_RC_ERR_* - if the row doesn't exists in the specified DB   
 **/
UncRespCode Kt_Port::IsKeyExists(OdbcmConnectionHandler *db_conn,
                                    unc_keytype_datatype_t data_type,
                                    const vector<string> &key_values) {
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  UncRespCode check_status = UNC_RC_SUCCESS;
  if (key_values.empty()) {
    // No key given, return failure
    pfc_log_error("No key given. Returning error");
    return UNC_UPPL_RC_ERR_BAD_REQUEST;
  }
  string controller_name = key_values[0];
  string switch_id = key_values[1];
  string port_id = key_values[2];

  // Structure used to send request to ODBC
  DBTableSchema kt_port_dbtableschema;

  // Construct Primary key list
  vector<string> vect_prim_keys;
  vect_prim_keys.push_back(CTR_NAME_STR);
  vect_prim_keys.push_back(SWITCH_ID_STR);
  vect_prim_keys.push_back(PORT_ID_STR);

  // Construct TableAttrSchema structure
  // TableAttrSchema holds table_name, primary key, attr_name
  vector<TableAttrSchema> vect_table_attr_schema;
  list< vector<TableAttrSchema> > row_list;

  // controller_name
  PhyUtil::FillDbSchema(unc::uppl::CTR_NAME, controller_name,
                        controller_name.length(),
                        DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);
  // switch_id
  PhyUtil::FillDbSchema(unc::uppl::SWITCH_ID, switch_id,
                        switch_id.length(), DATATYPE_UINT8_ARRAY_256,
                        vect_table_attr_schema);
  // port_id
  PhyUtil::FillDbSchema(unc::uppl::PORT_ID, port_id,
                        port_id.length(), DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);

  kt_port_dbtableschema.set_table_name(unc::uppl::PORT_TABLE);
  kt_port_dbtableschema.set_primary_keys(vect_prim_keys);
  row_list.push_back(vect_table_attr_schema);
  kt_port_dbtableschema.set_row_list(row_list);

  // Send request to ODBC for port_table
  ODBCM_RC_STATUS check_db_status = physical_layer->get_odbc_manager()->\
      IsRowExists(data_type, kt_port_dbtableschema, db_conn);
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

/** 
 * @Description : This function performs the required actions when oper status
 *                of its parent class
 *                changes i.e. when controller/switch oper status is down
 * @param[in]   : data_type - indicates the data base type
 *                key_struct - void pointer to be type cast to port key type
 *                value_struct - void pointer to be type cast to port value
 *                structure type
 * @return      : UNC_RC_SUCCESS - if the update operation of oper status
 *                change in db is successful
 *                UNC_UPPL_RC_ERR* - if the update operation of oper status in db
 *                is failure
 **/
UncRespCode Kt_Port::HandleOperStatus(OdbcmConnectionHandler *db_conn,
                                         uint32_t data_type,
                                         void *key_struct,
                                         void *value_struct) {
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  UncRespCode return_code = UNC_RC_SUCCESS;
  if (key_struct == NULL) {
    return UNC_UPPL_RC_ERR_BAD_REQUEST;
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
  UncRespCode read_status = controller.GetOperStatus(
      db_conn, data_type, reinterpret_cast<void*>(&ctr_key), ctrl_oper_status);
  if (read_status != UNC_RC_SUCCESS) {
    pfc_log_info("Controller's oper_status read returned failure");
    return read_status;
  }
  pfc_log_debug("Controller's oper_status %d", ctrl_oper_status);
  vector<OperStatusHolder> ref_oper_status;
  ADD_CTRL_OPER_STATUS(controller_name, ctrl_oper_status, ref_oper_status);
  if (ctrl_oper_status ==
      (UpplControllerOperStatus) UPPL_CONTROLLER_OPER_UP) {
    pfc_log_debug("Set Port oper status as up");
    port_oper_status = UPPL_PORT_OPER_UP;
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
      db_conn, data_type, reinterpret_cast<void*>(&sw_key), switch_oper_status);
  if (read_status == UNC_RC_SUCCESS) {
    pfc_log_debug("Switch's oper_status %d", switch_oper_status);
    ADD_SWITCH_OPER_STATUS(sw_key, switch_oper_status, ref_oper_status);
    if (switch_oper_status ==
        (UpplSwitchOperStatus) UPPL_SWITCH_OPER_UP) {
      pfc_log_debug("Set Port oper status as up");
      port_oper_status = UPPL_PORT_OPER_UP;
    }
  } else {
    pfc_log_debug("Switch oper_status read returned failure");
  }
  // Update oper_status in port table
  return_code = SetOperStatus(db_conn, data_type,
                              key_struct,
                              port_oper_status);
  pfc_log_debug("SetOperStatus ret_code: %d", return_code);

  // Call referred classes' notify oper_status functions
  // Get all ports associated with switch
  vector<key_port_t> vectPortKey;
  port_id.clear();
  while (true) {
    DBTableSchema kt_port_dbtableschema;
    vector<TableAttrSchema> vect_table_attr_schema;
    list< vector<TableAttrSchema> > row_list;
    vector<string> vect_prim_keys;
    vect_prim_keys.push_back(CTR_NAME_STR);
    vect_prim_keys.push_back(SWITCH_ID_STR);
    if (!switch_id.empty()) {
      // one more primary key is required
      vect_prim_keys.push_back(PORT_ID_STR);
    }
    pfc_log_debug(
        "Get Bulk Rows called with controller_name %s, switch_id %s"
        "port_id %s", controller_name.c_str(), switch_id.c_str(),
        port_id.c_str());
    PhyUtil::FillDbSchema(unc::uppl::CTR_NAME, controller_name,
                          controller_name.length(), DATATYPE_UINT8_ARRAY_32,
                          vect_table_attr_schema);

    PhyUtil::FillDbSchema(unc::uppl::SWITCH_ID, switch_id,
                          switch_id.length(), DATATYPE_UINT8_ARRAY_256,
                          vect_table_attr_schema);

    PhyUtil::FillDbSchema(unc::uppl::PORT_ID, port_id,
                          port_id.length(), DATATYPE_UINT8_ARRAY_32,
                          vect_table_attr_schema);

    kt_port_dbtableschema.set_table_name(unc::uppl::PORT_TABLE);
    kt_port_dbtableschema.set_primary_keys(vect_prim_keys);
    row_list.push_back(vect_table_attr_schema);
    kt_port_dbtableschema.set_row_list(row_list);

    ODBCM_RC_STATUS db_status = physical_layer->get_odbc_manager()->
        GetBulkRows((unc_keytype_datatype_t)data_type, UPPL_MAX_REP_CT,
                    kt_port_dbtableschema,
                    (unc_keytype_operation_t)UNC_OP_READ_SIBLING_BEGIN,
                    db_conn);
    if (db_status != ODBCM_RC_SUCCESS) {
      pfc_log_debug("error, No other port available");
      break;
    }
    list<vector<TableAttrSchema> > ::iterator iter_list;
    for (iter_list = kt_port_dbtableschema.row_list_.begin();
        iter_list != kt_port_dbtableschema.row_list_.end();
        ++iter_list) {
      vector<TableAttrSchema> attributes_vector = *iter_list;
      vector<TableAttrSchema> :: iterator iter_vector;
      key_port_t port;
      memset(&port, '\0', sizeof(port));
      for (iter_vector = attributes_vector.begin();
          iter_vector != attributes_vector.end();
          ++iter_vector) {
        // Get attribute name of a row
        TableAttrSchema tab_att_schema = (*iter_vector);
        switch (tab_att_schema.table_attribute_name) {
          case unc::uppl::CTR_NAME:
            PhyUtil::GetValueFromDbSchemaStr(
                tab_att_schema,
                port.sw_key.ctr_key.controller_name,
                DATATYPE_UINT8_ARRAY_32);
            controller_name = reinterpret_cast<const char*>
            (port.sw_key.ctr_key.controller_name);
            break;
          case unc::uppl::SWITCH_ID:
            PhyUtil::GetValueFromDbSchemaStr(tab_att_schema,
                                             port.sw_key.switch_id,
                                             DATATYPE_UINT8_ARRAY_256);
            switch_id = reinterpret_cast<const char*>
            (port.sw_key.switch_id);
            break;
          case unc::uppl::PORT_ID:
            PhyUtil::GetValueFromDbSchemaStr(tab_att_schema, port.port_id,
                                             DATATYPE_UINT8_ARRAY_320);
            port_id = reinterpret_cast<const char*>(port.port_id);
            break;
          default:
            break;
        }
      }
      vectPortKey.push_back(port);
    }
    if (kt_port_dbtableschema.row_list_.size() < UPPL_MAX_REP_CT) {
      pfc_log_debug("No other port available");
      break;
    }
  }
  vector<key_port_t>::iterator keyItr =
      vectPortKey.begin();
  for (; keyItr != vectPortKey.end(); ++keyItr) {
    key_port_t key_port = (*keyItr);
    ADD_PORT_OPER_STATUS(
        key_port,
        port_oper_status, ref_oper_status);
    return_code = NotifyOperStatus(
        db_conn, data_type, reinterpret_cast<void *> (&(*keyItr)), NULL,
        ref_oper_status);
    pfc_log_debug("Notify Oper status return %d", return_code);
  }
  ClearOperStatusHolder(ref_oper_status);
  return return_code;
}

/** 
 * @Description : This function performs the notifies other associated
 *                key types when port oper status changes
 * @param[in]   : data_type - indicates the data base type
 *                key_struct - void pointer to be type cast into respective
 *                key type's key structure
 *                value_struct - void pointer to be type cast into respective
 *                key type's value structure
 * @return      : UNC_RC_SUCCESS  - if the updation of oper status change in
 *                other key type is success
 *                UNC_UPPL_RC_ERR* - if the updation of oper status change in db
 *                is failure
 **/
UncRespCode Kt_Port::NotifyOperStatus(
    OdbcmConnectionHandler *db_conn,
    uint32_t data_type,
    void *key_struct,
    void *value_struct,
    vector<OperStatusHolder> &ref_oper_status) {
  UncRespCode return_code = UNC_RC_SUCCESS;
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
  Kt_LogicalPort log_port_obj;
  val_logical_port_st_t port_val_obj;
  memset(&port_val_obj, 0, sizeof(val_logical_port_st_t));
  memset(port_val_obj.logical_port.valid, 0, 5);
  memcpy(port_val_obj.logical_port.switch_id,
         switch_id.c_str(), switch_id.length()+1);
  port_val_obj.logical_port.valid[kIdxLogicalPortSwitchId] = UNC_VF_VALID;
  memcpy(port_val_obj.logical_port.physical_port_id, port_id.c_str(),
         (port_id.length())+1);
  port_val_obj.logical_port.valid[kIdxLogicalPortPhysicalPortId] =
      UNC_VF_VALID;
  return_code = log_port_obj.HandleOperStatus(
      db_conn, data_type, reinterpret_cast<void *> (&logical_port_key),
      reinterpret_cast<void *> (&port_val_obj), ref_oper_status,
      UNC_KT_PORT);
  pfc_log_debug("HandleOperStatus for port class %d",
                return_code);

  // Perform SubDomain operstatus handling as well
  return_code = SubDomainOperStatusHandling(db_conn, data_type,
                                            controller_name,
                                            switch_id, port_id);

  // Notify UNC_KT_LINK
  Kt_Link link;
  key_link link_key;
  memset(&link_key, 0, sizeof(link_key));
  memcpy(link_key.ctr_key.controller_name,
         controller_name.c_str(),
         controller_name.length()+1);
  memcpy(link_key.port_id1,
         port_id.c_str(),
         port_id.length()+1);
  memcpy(link_key.switch_id1,
         switch_id.c_str(),
         switch_id.length()+1);
  return_code = link.HandleOperStatus(
      db_conn, data_type, reinterpret_cast<void *> (&link_key),
      NULL);
  pfc_log_debug("HandleOperStatus for link class %d",
                return_code);
  key_link link_key1;
  memset(&link_key1, 0, sizeof(link_key1));
  memcpy(link_key1.ctr_key.controller_name,
         controller_name.c_str(),
         controller_name.length()+1);
  memcpy(link_key1.port_id2,
         port_id.c_str(),
         port_id.length()+1);
  memcpy(link_key1.switch_id2,
         switch_id.c_str(),
         switch_id.length()+1);
  return_code = link.HandleOperStatus(
      db_conn, data_type, reinterpret_cast<void *> (&link_key1),
      NULL);
  pfc_log_debug("HandleOperStatus for link class %d",
                return_code);
  return return_code;
}


/** 
 * @Description : This function reads the oper_status value of the port from
 *                the specified data type
 * @param[in]   : data_type - indicates the data base type i.e. DT_STATE
 *                key_struct - void pointer to be type cast to port key type
 *                structure
 * @return      : oper_status - it stores the oper status returned from DB
 **/
UncRespCode Kt_Port::GetOperStatus(OdbcmConnectionHandler *db_conn,
                                      uint32_t data_type,
                                      void* key_struct,
                                      uint8_t &oper_status) {
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  key_port *obj_key_port =
      reinterpret_cast<key_port_t*>(key_struct);
  vector<TableAttrSchema> vect_table_attr_schema;
  list < vector<TableAttrSchema> > row_list;
  vector<string> vect_prim_keys;

  string controller_name = (const char*)obj_key_port->sw_key.
      ctr_key.controller_name;

  if (!controller_name.empty()) {
    vect_prim_keys.push_back(CTR_NAME_STR);
  }

  PhyUtil::FillDbSchema(unc::uppl::CTR_NAME, controller_name,
                        controller_name.length(), DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);

  string switch_id = (const char*)obj_key_port->
      sw_key.switch_id;
  if (!switch_id.empty()) {
    vect_prim_keys.push_back(SWITCH_ID_STR);
  }

  PhyUtil::FillDbSchema(unc::uppl::SWITCH_ID, switch_id,
                        switch_id.length(), DATATYPE_UINT8_ARRAY_256,
                        vect_table_attr_schema);

  string port_id = (const char*)obj_key_port->port_id;
  if (!port_id.empty()) {
    vect_prim_keys.push_back(PORT_ID_STR);
  }

  PhyUtil::FillDbSchema(unc::uppl::PORT_ID, port_id,
                        port_id.length(), DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);

  string oper_value = "";
  PhyUtil::FillDbSchema(unc::uppl::PORT_OPER_STATUS, oper_value,
                        oper_value.length(), DATATYPE_UINT16,
                        vect_table_attr_schema);

  DBTableSchema kt_port_dbtableschema;
  kt_port_dbtableschema.set_table_name(unc::uppl::PORT_TABLE);
  kt_port_dbtableschema.set_primary_keys(vect_prim_keys);
  row_list.push_back(vect_table_attr_schema);
  kt_port_dbtableschema.set_row_list(row_list);

  // Call ODBCManager and update
  ODBCM_RC_STATUS update_db_status =
      physical_layer->get_odbc_manager()->GetOneRow(
          (unc_keytype_datatype_t)data_type,
          kt_port_dbtableschema, db_conn);
  if (update_db_status != ODBCM_RC_SUCCESS) {
    pfc_log_info("oper_status read operation failed %d", update_db_status);
    return UNC_UPPL_RC_ERR_DB_GET;
  }

  // read the oper_status value
  list < vector<TableAttrSchema> >& res_port_row_list =
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
      ODBCMTableColumns attr_name = tab_schema.table_attribute_name;
      string attr_value = "";
      if (attr_name == unc::uppl::PORT_OPER_STATUS) {
        PhyUtil::GetValueFromDbSchema(tab_schema, attr_value,
                                      DATATYPE_UINT16);
        oper_status = atoi(attr_value.c_str());
        pfc_log_debug("oper_status: %d", oper_status);
        break;
      }
    }
  }
  return UNC_RC_SUCCESS;
}

/** 
 * @Description : This function updates the alarm status in db
 * @param[in]   : data_type - indicates the data base type
 *                key_struct - void pointer type to be type cast to port key
 *                type structure
 *                alarm status - stores the alarm status received from DB
 * @return      : UNC_RC_SUCCESS - if the alarm status is received
 *                successfully from db
 *                UNC_UPPL_RC_ERR* - if there is failure in reading the value
 *                from DB
 **/

UncRespCode Kt_Port::GetAlarmStatus(OdbcmConnectionHandler *db_conn,
                                       uint32_t data_type,
                                       void* key_struct,
                                       uint64_t &alarms_status) {
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  key_port *obj_key_port =
      reinterpret_cast<key_port_t*>(key_struct);
  vector<TableAttrSchema> vect_table_attr_schema;
  list < vector<TableAttrSchema> > row_list;
  vector<string> vect_prim_keys;
  vect_prim_keys.push_back(CTR_NAME_STR);
  vect_prim_keys.push_back(SWITCH_ID_STR);
  vect_prim_keys.push_back(PORT_ID_STR);

  string controller_name =
      (const char*)obj_key_port->sw_key.ctr_key.controller_name;

  PhyUtil::FillDbSchema(unc::uppl::CTR_NAME, controller_name,
                        controller_name.length(), DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);
  string switch_id = (const char*)obj_key_port->sw_key.switch_id;

  PhyUtil::FillDbSchema(unc::uppl::SWITCH_ID, switch_id,
                        switch_id.length(), DATATYPE_UINT8_ARRAY_256,
                        vect_table_attr_schema);

  string port_id = (const char*)obj_key_port->port_id;

  PhyUtil::FillDbSchema(unc::uppl::PORT_ID, port_id,
                        port_id.length(), DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);
  string empty_alarms_status = "";
  PhyUtil::FillDbSchema(unc::uppl::PORT_ALARM_STATUS, empty_alarms_status,
                        empty_alarms_status.length(), DATATYPE_UINT64,
                        vect_table_attr_schema);
  DBTableSchema kt_port_dbtableschema;
  kt_port_dbtableschema.set_table_name(unc::uppl::PORT_TABLE);
  kt_port_dbtableschema.set_primary_keys(vect_prim_keys);
  row_list.push_back(vect_table_attr_schema);
  kt_port_dbtableschema.set_row_list(row_list);

  // Call ODBCManager and Update
  ODBCM_RC_STATUS update_db_status =
      physical_layer->get_odbc_manager()->GetOneRow(
          (unc_keytype_datatype_t)data_type,
          kt_port_dbtableschema, db_conn);
  if (update_db_status != ODBCM_RC_SUCCESS) {
    pfc_log_info("oper_status read operation failed %d", update_db_status);
    return UNC_UPPL_RC_ERR_DB_GET;
  }

  // read the oper status value
  list < vector<TableAttrSchema> >& res_port_row_list =
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
      ODBCMTableColumns attr_name = tab_schema.table_attribute_name;
      string attr_value = "";
      if (attr_name == unc::uppl::PORT_ALARM_STATUS) {
        PhyUtil::GetValueFromDbSchema(tab_schema, attr_value,
                                      DATATYPE_UINT64);
        alarms_status = atol(attr_value.c_str());
        break;
      }
    }
  }
  return UNC_RC_SUCCESS;
}

/**
 * @Description : This function updates the oper_status value of the port in DB
 * @param[in]   : data_type - indicates the data base type
 *                key_struct - void pointer to be type cast into port key
 *                type structure
 *                oper_status - stores the oper status value to be updated in db
 * @return    : oper_status
 **/
UncRespCode Kt_Port::SetOperStatus(OdbcmConnectionHandler *db_conn,
                                      uint32_t data_type,
                                      void* key_struct,
                                      UpplPortOperStatus oper_status) {
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  key_port_t *obj_key_port=
      reinterpret_cast<key_port_t*>(key_struct);
  vector<TableAttrSchema> vect_table_attr_schema;
  list < vector<TableAttrSchema> > row_list;
  vector<string> vect_prim_keys;
  string controller_name = (const char*)obj_key_port->sw_key.
      ctr_key.controller_name;
  string switch_id = (const char*)obj_key_port->
      sw_key.switch_id;
  string port_id = (const char*)obj_key_port->port_id;

  if (!controller_name.empty()) {
    vect_prim_keys.push_back(CTR_NAME_STR);
  }

  PhyUtil::FillDbSchema(unc::uppl::CTR_NAME, controller_name,
                        controller_name.length(), DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);
  if (!switch_id.empty()) {
    vect_prim_keys.push_back(SWITCH_ID_STR);
    PhyUtil::FillDbSchema(unc::uppl::SWITCH_ID, switch_id,
                          switch_id.length(), DATATYPE_UINT8_ARRAY_256,
                          vect_table_attr_schema);
  }

  if (!port_id.empty()) {
    vect_prim_keys.push_back(PORT_ID_STR);
    PhyUtil::FillDbSchema(unc::uppl::PORT_ID, port_id,
                          port_id.length(), DATATYPE_UINT8_ARRAY_32,
                          vect_table_attr_schema);
  }

  string oper_value = PhyUtil::uint8tostr(oper_status);
  PhyUtil::FillDbSchema(unc::uppl::PORT_OPER_STATUS, oper_value,
                        oper_value.length(), DATATYPE_UINT16,
                        vect_table_attr_schema);

  DBTableSchema kt_port_dbtableschema;
  kt_port_dbtableschema.set_table_name(unc::uppl::PORT_TABLE);
  kt_port_dbtableschema.set_primary_keys(vect_prim_keys);
  row_list.push_back(vect_table_attr_schema);
  kt_port_dbtableschema.set_row_list(row_list);

  // Call ODBCManager and update
  ODBCM_RC_STATUS update_db_status =
      physical_layer->get_odbc_manager()->UpdateOneRow(
          (unc_keytype_datatype_t)data_type,
          kt_port_dbtableschema, db_conn, true);
  if (update_db_status == ODBCM_RC_ROW_NOT_EXISTS) {
    pfc_log_debug("No instance available for update");
  } else if (update_db_status != ODBCM_RC_SUCCESS) {
    // log error
    pfc_log_error("oper_status update operation failed");
    return UNC_UPPL_RC_ERR_DB_UPDATE;
  } else {
    // Notify operstatus change to northbound
    uint8_t old_oper_status = 0;
    UncRespCode read_status = GetOperStatus(db_conn, data_type,
                                               key_struct,
                                               old_oper_status);
    if (read_status == UNC_RC_SUCCESS) {
      val_port_st old_val_port, new_val_port;
      memset(&old_val_port, 0, sizeof(old_val_port));
      memset(&new_val_port, 0, sizeof(new_val_port));
      old_val_port.oper_status = old_oper_status;
      old_val_port.valid[kIdxPortOperStatus] = UNC_VF_VALID;
      new_val_port.oper_status = oper_status;
      new_val_port.valid[kIdxPortOperStatus] = UNC_VF_VALID;
      int err = 0;
      // Send notification to Northbound
      ServerEvent ser_evt((pfc_ipcevtype_t)UPPL_EVENTS_KT_PORT, err);
      northbound_event_header rsh = {static_cast<uint32_t>(UNC_OP_UPDATE),
          data_type,
          static_cast<uint32_t>(UNC_KT_PORT)};
      err = PhyUtil::sessOutNBEventHeader(ser_evt, rsh);
      err |= ser_evt.addOutput(*obj_key_port);
      err |= ser_evt.addOutput(new_val_port);
      err |= ser_evt.addOutput(old_val_port);
      if (err == 0) {
        PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
        // Notify operstatus modifications
        UncRespCode status = (UncRespCode) physical_layer
            ->get_ipc_connection_manager()->SendEvent(&ser_evt,
                controller_name, UPPL_EVENTS_KT_PORT);
        pfc_log_debug("Event notification status %d", status);
      } else {
        pfc_log_error("Server Event addOutput failed");
      }
    }
  }
  return UNC_RC_SUCCESS;
}

/**
 * @Description : This function is used to populate the db schema using the
 *                given key struct, value struct, data_type, operation,
 *                option1 and option
 * @param[in]   : kt_port_dbtableschema - object of type DBTableSchema
 *                key_struct - void pointer to be type cast into port key type
 *                val_struct - void pointer to be type cast into port value type
 *                operation_type - specifies the operation type i.e
 *                                 UNC_OP_READ or UNC_OP_READ_SIBLING_BEGIN etc
 *                option1/option2 - specifies any additional option for
 *                populating in DB
 *                vect_key_operations - vector of type ODBCMOperator
 *                old_value_struct - holds the old value structure of the
 *                port key type
 *                row_status - enum indicating the row status of port type
 *                entries in db
 *                is_filtering/is_state - bool variables
 * @return      : None
 **/
void Kt_Port::PopulateDBSchemaForKtTable(
    OdbcmConnectionHandler *db_conn,
    DBTableSchema &kt_port_dbtableschema,
    void* key_struct,
    void* val_struct,
    uint8_t operation_type,
    uint32_t data_type,
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
  // controller_name
  string controller_name = (const char*)obj_key_port
      ->sw_key.ctr_key.controller_name;
  if (!controller_name.empty()) {
    vect_prim_keys.push_back(CTR_NAME_STR);
  }
  PhyUtil::FillDbSchema(unc::uppl::CTR_NAME, controller_name,
                        controller_name.length(), DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);
  // switch_id
  string switch_id = (const char*)obj_key_port->sw_key.switch_id;
  if (!switch_id.empty()) {
    vect_prim_keys.push_back(SWITCH_ID_STR);
  }
  PhyUtil::FillDbSchema(unc::uppl::SWITCH_ID, switch_id,
                        switch_id.length(), DATATYPE_UINT8_ARRAY_256,
                        vect_table_attr_schema);
  // port_id
  string port_id = (const char*)obj_key_port->port_id;
  if (operation_type == UNC_OP_READ_SIBLING_BEGIN ||
      operation_type == UNC_OP_READ_SIBLING_COUNT) {
    // Ignore port_id
    port_id = "";
  }

  pfc_log_info("ctr name:%s,switch id:%s,port id:%s,oper_type:%d",
                 controller_name.c_str(),
                 switch_id.c_str(),
                 port_id.c_str(),
                 operation_type);
  PhyUtil::FillDbSchema(unc::uppl::PORT_ID, port_id,
                        port_id.length(), DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);

  val_port_st_t *val_port_valid_st = NULL;
  if (operation_type == UNC_OP_UPDATE) {
    // get valid array for update req
    pfc_log_debug("Get Valid value from Update Valid Flag");
    val_port_valid_st = new val_port_st_t();
    unc_keytype_validflag_t new_valid_val = UNC_VF_VALID;
    UpdatePortValidFlag(db_conn, key_struct, val_struct,
                        *val_port_valid_st,
                        new_valid_val, data_type);
    old_val_struct = reinterpret_cast<void *>(val_port_valid_st);
  }

  GetPortValStructure(
      db_conn, obj_val_port,
      vect_table_attr_schema,
      vect_prim_keys,
      operation_type,
      val_port_valid_st,
      valid);
  GetPortStateValStructure(
      db_conn, obj_val_port,
      vect_table_attr_schema,
      vect_prim_keys,
      operation_type,
      val_port_valid_st,
      valid);
  vect_prim_keys.push_back(PORT_ID_STR);
  PhyUtil::reorder_col_attrs(vect_prim_keys, vect_table_attr_schema);
  kt_port_dbtableschema.set_table_name(unc::uppl::PORT_TABLE);
  kt_port_dbtableschema.set_primary_keys(vect_prim_keys);
  row_list.push_back(vect_table_attr_schema);
  kt_port_dbtableschema.set_row_list(row_list);
  return;
}

/** 
 * @Description : This function populates val_port_st_t by values retrieved
 *                from database
 * @param[in]   : kt_port table dbtable schema - object of type DBTableSchema
 *                vect_obj_val_port - vector to store the val_port_st structure
 *                values
 *                max_rep_ct - specifies number of rows to be returned
 *                operation_type - indicates the operation type  
 *                port_id - vector of type key_port_t to store the port id
 * @return      : None
 **/

void Kt_Port::FillPortValueStructure(
    OdbcmConnectionHandler *db_conn,
    DBTableSchema &kt_port_dbtableschema,
    vector<val_port_st_t> &vect_obj_val_port,
    uint32_t &max_rep_ct,
    uint32_t operation_type,
    vector<key_port_t> &port_id) {
  // populate IPC value structure based on the response received from DB
  list < vector<TableAttrSchema> >& res_port_row_list =
      kt_port_dbtableschema.get_row_list();
  list < vector<TableAttrSchema> > :: iterator res_port_iter =
      res_port_row_list.begin();
  max_rep_ct = res_port_row_list.size();
  pfc_log_debug("res_port_row_list.size: %d", max_rep_ct);

  // populate IPC value structure based on the response received from DB
  for (; res_port_iter != res_port_row_list.end(); ++res_port_iter) {
    vector<TableAttrSchema> res_port_table_attr_schema =
        (*res_port_iter);
    vector<TableAttrSchema> :: iterator vect_port_iter =
        res_port_table_attr_schema.begin();
    val_port_st_t obj_val_port;
    memset(&obj_val_port, 0, sizeof(val_port_st_t));
    key_port_t obj_key_port;
    memset(&obj_key_port, '\0', sizeof(obj_key_port));
    // Read all attributes
    for (; vect_port_iter != res_port_table_attr_schema.end();
        ++vect_port_iter) {
      // Populate values from port_table
      TableAttrSchema tab_schema = (*vect_port_iter);
      ODBCMTableColumns attr_name = tab_schema.table_attribute_name;
      string attr_value = "";
      switch (attr_name) {
        case unc::uppl::CTR_NAME:
          PhyUtil::GetValueFromDbSchemaStr(
              tab_schema,
              obj_key_port.sw_key.ctr_key.controller_name,
              DATATYPE_UINT8_ARRAY_32);
          pfc_log_debug("controller_name: %s", reinterpret_cast<char *>
          (&obj_key_port.sw_key.ctr_key.controller_name));
          break;

        case unc::uppl::SWITCH_ID:
          PhyUtil::GetValueFromDbSchemaStr(tab_schema,
                                           obj_key_port.sw_key.switch_id,
                                           DATATYPE_UINT8_ARRAY_256);
          pfc_log_debug("switch_id: %s", reinterpret_cast<char *>
          (&obj_key_port.sw_key.switch_id));
          break;

        case unc::uppl::PORT_ID:
          PhyUtil::GetValueFromDbSchemaStr(tab_schema,
                                           obj_key_port.port_id,
                                           DATATYPE_UINT8_ARRAY_32);
          pfc_log_debug("port_id: %s", reinterpret_cast<char *>
          (&obj_key_port.port_id));
          break;

        case unc::uppl::PORT_NUMBER:
          PhyUtil::GetValueFromDbSchema(tab_schema, attr_value,
                                        DATATYPE_UINT32);
          obj_val_port.port.port_number = atoi(attr_value.c_str());
          pfc_log_debug("port_number: %d", obj_val_port.port.port_number);
          break;

        case unc::uppl::PORT_DESCRIPTION:
          PhyUtil::GetValueFromDbSchemaStr(tab_schema,
                                           obj_val_port.port.description,
                                           DATATYPE_UINT8_ARRAY_128);
          pfc_log_debug("description: %s", obj_val_port.port.description);
          break;

        case unc::uppl::PORT_ADMIN_STATUS:
          PhyUtil::GetValueFromDbSchema(tab_schema, attr_value,
                                        DATATYPE_UINT16);
          obj_val_port.port.admin_status = atoi(attr_value.c_str());
          pfc_log_debug("admin_status: %d", obj_val_port.port.admin_status);
          break;

        case unc::uppl::PORT_TRUNK_ALL_VLAN:
          PhyUtil::GetValueFromDbSchema(tab_schema, attr_value,
                                        DATATYPE_UINT16);
          obj_val_port.port.trunk_allowed_vlan =
              atoi((const char*)attr_value.c_str());
          pfc_log_debug("trunk_allowed_vlan: %s", attr_value.c_str());
          break;

        case unc::uppl::PORT_OPER_STATUS:
          PhyUtil::GetValueFromDbSchema(tab_schema, attr_value,
                                        DATATYPE_UINT16);
          obj_val_port.oper_status = atoi(attr_value.c_str());
          pfc_log_debug("oper status : %d", obj_val_port.oper_status);
          break;

        case unc::uppl::PORT_MAC_ADDRESS:
          PhyUtil::GetValueFromDbSchemaStr(tab_schema,
                                           obj_val_port.mac_address,
                                           DATATYPE_UINT8_ARRAY_6);
          break;

        case unc::uppl::PORT_DIRECTION:
          PhyUtil::GetValueFromDbSchema(tab_schema, attr_value,
                                        DATATYPE_UINT16);
          obj_val_port.direction = atoi(attr_value.c_str());
          pfc_log_debug("direction: %d", obj_val_port.direction);
          break;

        case unc::uppl::PORT_DUPLEX:
          PhyUtil::GetValueFromDbSchema(tab_schema, attr_value,
                                        DATATYPE_UINT16);
          obj_val_port.duplex = atoi(attr_value.c_str());
          pfc_log_debug("duplex : %d", obj_val_port.duplex);
          break;

        case unc::uppl::PORT_SPEED:
          PhyUtil::GetValueFromDbSchema(tab_schema, attr_value,
                                        DATATYPE_UINT64);
          pfc_log_debug("speed from DB : %s", attr_value.c_str());
          obj_val_port.speed = atol(attr_value.c_str());
          pfc_log_debug("speed : %" PFC_PFMT_u64, obj_val_port.speed);
          pfc_log_debug("speed PFC_PFMT_u64 : %" PFC_PFMT_u64 "...",
                        obj_val_port.speed);
          break;

        case unc::uppl::PORT_ALARM_STATUS:
          PhyUtil::GetValueFromDbSchema(tab_schema, attr_value,
                                        DATATYPE_UINT64);
          obj_val_port.alarms_status = atol(attr_value.c_str());
          pfc_log_debug("alarms_status : %" PFC_PFMT_u64,
                        obj_val_port.alarms_status);
          break;

        case unc::uppl::PORT_LOGIC_PORT_ID:
          PhyUtil::GetValueFromDbSchemaStr(tab_schema,
                                           obj_val_port.logical_port_id,
                                           DATATYPE_UINT8_ARRAY_320);
          pfc_log_debug("Logical_port_id: %s", obj_val_port.logical_port_id);
          break;

        case unc::uppl::PORT_VALID:
          uint8_t port_valid[ODBCM_SIZE_11];
          memset(obj_val_port.valid, 0, 8);
          obj_val_port.valid[kIdxPortSt] = UNC_VF_VALID;
          PhyUtil::GetValueFromDbSchemaStr(tab_schema, port_valid,
                                           DATATYPE_UINT8_ARRAY_11);
          memset(obj_val_port.port.valid, '\0', 4);
          FrameValidValue(reinterpret_cast<const char*> (port_valid),
                          obj_val_port);
          pfc_log_debug("valid: %s", port_valid);
          break;

        default:
          pfc_log_info("Ignoring Port attribute %d", attr_name);
          break;
      }
    }
    vect_obj_val_port.push_back(obj_val_port);
    // populate key structure
    port_id.push_back(obj_key_port);
    pfc_log_debug("result - vect_obj_val_port size: %"
                  PFC_PFMT_SIZE_T, vect_obj_val_port.size());
    pfc_log_debug("result - vect_port_id size: %"
                  PFC_PFMT_SIZE_T, port_id.size());
  }
  return;
}

/**
 * @Description : This function is used to read KT_PORT instance in database
 *                table using key_ctr provided in IPC request
 *                The IPC response would be filled in IPC session
 * @param[in]   : ipc session id - ipc session id used for TC validation
 *                configuration id - configuration id used for TC validation
 *                key_struct - void pointer to be type cast into port key type
 *                value_struct - void pointer to be type cast into port value
 *                structure
 *                data_type - specifies the data base type i.e UNC_DT_STATE
 *                            or UNC_DT_IMPORT
 *                operation type - indicates the operation type
 *                sess - ipc server session where the response has to be added
 *                option1,option2 - specifies any additional condition for
 *                read operation
 *                max_rep_ct - indicates the maximum repetition count
 * @return      : UNC_RC_SUCCESS - if the read operation is successful
 *                UNC_UPPL_RC_ERR_* - read operation failed
 **/
UncRespCode Kt_Port::PerformRead(OdbcmConnectionHandler *db_conn,
                                    uint32_t session_id,
                                    uint32_t configuration_id,
                                    void* key_struct,
                                    void* val_struct,
                                    uint32_t data_type,
                                    uint32_t operation_type,
                                    ServerSession &sess,
                                    uint32_t option1,
                                    uint32_t option2,
                                    uint32_t max_rep_ct) {
  pfc_log_debug("PerformRead oper=%d,dt=%d",
               operation_type, data_type);
  key_port_t *obj_key_port =
      reinterpret_cast<key_port_t*>(key_struct);
  UncRespCode read_status = UNC_RC_SUCCESS;
  physical_response_header rsh = {session_id,
      configuration_id,
      operation_type,
      max_rep_ct,
      option1,
      option2,
      data_type,
      static_cast<uint32_t>(read_status)};
  if (operation_type == UNC_OP_READ) {
    max_rep_ct = 1;
  }
  if ((unc_keytype_datatype_t)data_type != UNC_DT_STATE) {
    pfc_log_error("Read operation is provided on unsupported data type");
    rsh.result_code = UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED;
    int err = PhyUtil::sessOutRespHeader(sess, rsh);
    err |= sess.addOutput((uint32_t)UNC_KT_PORT);
    err |= sess.addOutput(*obj_key_port);
    if (err != 0) {
      pfc_log_info("addOutput failed for physical_response_header");
      return UNC_UPPL_RC_ERR_IPC_WRITE_ERROR;
    }
    return UNC_RC_SUCCESS;
  }
  // U12 - Allowing UNC_OPT1_DETAIL for Port Statistics
  if (option1 != UNC_OPT1_NORMAL && option1 != UNC_OPT1_DETAIL) {
    // Invalid operation
    pfc_log_error("PerformRead provided on unsupported option1");
    rsh.result_code = UNC_UPPL_RC_ERR_INVALID_OPTION1;
    int err = PhyUtil::sessOutRespHeader(sess, rsh);
    err |= sess.addOutput((uint32_t)UNC_KT_PORT);
    err |= sess.addOutput(*obj_key_port);
    if (err != 0) {
      pfc_log_info("addOutput failed for physical_response_header");
      return UNC_UPPL_RC_ERR_IPC_WRITE_ERROR;
    }
    return UNC_RC_SUCCESS;
  }

  if (option1 == UNC_OPT1_DETAIL && option2 != UNC_OPT2_NONE) {
    pfc_log_error("PerformRead provided on unsupported option2");
    rsh.result_code = UNC_UPPL_RC_ERR_INVALID_OPTION2;
    int err = PhyUtil::sessOutRespHeader(sess, rsh);
    err |= sess.addOutput((uint32_t)UNC_KT_PORT);
    err |= sess.addOutput(*obj_key_port);
    if (err != 0) {
      pfc_log_info("addOutput failed for physical_response_header");
      return UNC_UPPL_RC_ERR_IPC_WRITE_ERROR;
    }
    return UNC_RC_SUCCESS;
  }
  // U12 End

  if (option2 != UNC_OPT2_NEIGHBOR && option2 != UNC_OPT2_NONE) {
    pfc_log_error("PerformRead provided on unsupported option2");
    rsh.result_code = UNC_UPPL_RC_ERR_INVALID_OPTION2;
    int err = PhyUtil::sessOutRespHeader(sess, rsh);
    err |= sess.addOutput((uint32_t)UNC_KT_PORT);
    err |= sess.addOutput(*obj_key_port);
    if (err != 0) {
      pfc_log_info("addOutput failed for physical_response_header");
      return UNC_UPPL_RC_ERR_IPC_WRITE_ERROR;
    }
    return UNC_RC_SUCCESS;
  }


  // U12 START
  string controller_name = (const char*)obj_key_port->sw_key.
      ctr_key.controller_name;
  ClientSession *cli_session = NULL;
  if (option1 == UNC_OPT1_DETAIL && option2 == UNC_OPT2_NONE) {
    pfc_log_trace("Inside Detail:KT_PORT");
    // Not allowing the STANDBY request
     PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
     if (physical_layer->get_physical_core()->get_system_state() == \
         UPPL_SYSTEM_ST_STANDBY) {
       pfc_log_error("System is in Standby mode");
        rsh.result_code = UNC_UPPL_RC_ERR_NOT_SUPPORTED_BY_STANDBY;
        rsh.max_rep_count = 0;
        int err = PhyUtil::sessOutRespHeader(sess, rsh);
        err |= sess.addOutput((uint32_t) UNC_KT_PORT);
        err |= sess.addOutput(*obj_key_port);
        if (err != 0) {
          pfc_log_info("addOutput failed for physical_response_header");
          return UNC_UPPL_RC_ERR_IPC_WRITE_ERROR;
        }
        return UNC_RC_SUCCESS;
      }

      // Getting the Response from Driver
      vector<val_port_st_t> vect_val_port_st;
      vector<key_port_t> vect_port_id;
      read_status = ReadPortValFromDB(db_conn,
                                        obj_key_port,
                                        val_struct,
                                        data_type,
                                        operation_type,
                                        max_rep_ct,
                                        vect_val_port_st,
                                        vect_port_id);
      rsh.max_rep_count = max_rep_ct;
      if (vect_port_id.size() == 0 || vect_val_port_st.size() == 0) {
        pfc_log_debug("U12:read vectors size is zero");
        read_status = UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE;
      }
      pfc_log_debug("U12:read status val in PerformRead = %d", read_status);
      if (read_status != UNC_RC_SUCCESS) {
        rsh.result_code = read_status;
        rsh.max_rep_count = 0;
        int err = PhyUtil::sessOutRespHeader(sess, rsh);
        err |= sess.addOutput((uint32_t) UNC_KT_PORT);
        err |= sess.addOutput(*obj_key_port);
        if (err != 0) {
          pfc_log_info("addOutput failed for physical_response_header");
          return UNC_UPPL_RC_ERR_IPC_WRITE_ERROR;
        }
        return UNC_RC_SUCCESS;
      }
      // Filling the key structure and value strucute to the map from DB
      map<string, val_port_stats_t> nb_port_stats_map;

      for (unsigned int index = 0; index < vect_port_id.size();
          ++index) {
        key_port_t obj_key_port = vect_port_id[index];
        string port_name =  reinterpret_cast<char*>(obj_key_port.port_id);
        pfc_log_debug("Inside the map %s", obj_key_port.port_id);
        val_port_stats_t ptr;
        memset(&ptr, 0, sizeof(val_port_stats_t));
        memcpy(&(ptr.port_st_val), &vect_val_port_st[index]
            , sizeof(val_port_st_t));
        ptr.valid[0] = 1;
        nb_port_stats_map.insert(std::pair<string, val_port_stats_t>
                            (port_name, ptr));
        pfc_log_debug("Map size=%" PFC_PFMT_SIZE_T, nb_port_stats_map.size());
      }

    unc_keytype_ctrtype_t controller_type = UNC_CT_UNKNOWN;
    // call GetControllerType controller type func
    (unc_keytype_ctrtype_t)PhyUtil::get_controller_type(
                                                 db_conn,
                                                 controller_name,
                                                 controller_type,
                                                 UNC_DT_STATE);
    pfc_log_debug("Port:controller type is %d ", controller_type);
    if (controller_type ==  UNC_CT_PFC ||
        controller_type ==  UNC_CT_ODC) {
      UncRespCode driver_response = UNC_RC_SUCCESS;

     UncRespCode err_resp = UNC_RC_SUCCESS;
     IPCClientDriverHandler drv_handler(controller_type, err_resp);
     if (err_resp == 0) {
      cli_session = drv_handler.ResetAndGetSession();
      // Creating a session to driver
      string domain_id = "";
      int err = 0;
      if (operation_type == UNC_OP_READ) {
        driver_request_header rqh = {uint32_t(0), uint32_t(0), controller_name,
        domain_id, static_cast<uint32_t>(UNC_OP_READ), uint32_t(0),
        static_cast<uint32_t>(UNC_OPT1_DETAIL),
        static_cast<uint32_t>(UNC_OPT2_NONE),
        static_cast<uint32_t>(UNC_DT_STATE),
        static_cast<uint32_t>(UNC_KT_PORT)};
        err = PhyUtil::sessOutDriverReqHeader(*cli_session, rqh);
      } else if (operation_type == UNC_OP_READ_SIBLING ||
               operation_type ==  UNC_OP_READ_SIBLING_BEGIN) {
        driver_request_header rqh = {uint32_t(0), uint32_t(0), controller_name,
        domain_id, static_cast<uint32_t>(UNC_OP_READ), uint32_t(0),
        static_cast<uint32_t>(UNC_OPT1_DETAIL),
        static_cast<uint32_t>(UNC_OPT2_SIBLING_ALL),
        static_cast<uint32_t>(UNC_DT_STATE),
        static_cast<uint32_t>(UNC_KT_PORT)};
        err = PhyUtil::sessOutDriverReqHeader(*cli_session, rqh);
      }
      err |= cli_session->addOutput(*obj_key_port);
      if (err == 0) {
      driver_response_header rsp;
      driver_response = drv_handler.SendReqAndGetResp(rsp);
      if (driver_response == UNC_RC_SUCCESS) {
      uint8_t response_count = cli_session->getResponseCount();
      pfc_log_info("drv resp cnt=%d", response_count);
      if (response_count > 10) {
      for (uint32_t idx = 11;
               idx < response_count ; ) {
         key_port_t key_port_drv_resp;
         int err1 = cli_session->getResponse(idx++, key_port_drv_resp);
         if (err1 != UNC_RC_SUCCESS) {
           driver_response = UNC_UPPL_RC_ERR_DRIVER_COMMUNICATION_FAILURE;
           break;
         }
        string port_name = (const char*)key_port_drv_resp.port_id;
        pfc_log_debug("Portname from drv resp %s", port_name.c_str());
        val_port_stats_t port_stat_val;
        memset(&port_stat_val, '\0', sizeof(val_port_stats_t));
        err1 = cli_session->getResponse(idx++, port_stat_val);
        if (err1 != UNC_RC_SUCCESS) {
          driver_response = UNC_UPPL_RC_ERR_DRIVER_COMMUNICATION_FAILURE;
          break;
        }
        map<string, val_port_stats_t> :: iterator it_port_key_val =
                                   nb_port_stats_map.find(port_name);
        if (it_port_key_val != nb_port_stats_map.end()) {
          val_port_stats_t val_port_stat = it_port_key_val->second;
          memcpy(&port_stat_val.port_st_val, &val_port_stat.port_st_val
              , sizeof(port_stat_val.port_st_val));
          port_stat_val.valid[0] = 1;
          nb_port_stats_map[port_name] = port_stat_val;
        }
      }
    } else {
       driver_response = UNC_UPPL_RC_ERR_DRIVER_COMMUNICATION_FAILURE;
    }
  } else {
     // do nothing
     }
    } else {
      driver_response = UNC_UPPL_RC_ERR_DRIVER_COMMUNICATION_FAILURE;
    }
    } else {
        driver_response = UNC_UPPL_RC_ERR_DRIVER_COMMUNICATION_FAILURE;
      }
      if (driver_response != UNC_RC_SUCCESS) {
        rsh.result_code = driver_response;
        rsh.max_rep_count = 0;
        int err = PhyUtil::sessOutRespHeader(sess, rsh);
        err |= sess.addOutput((uint32_t) UNC_KT_PORT);
        err |= sess.addOutput(*obj_key_port);
        if (err != 0) {
          pfc_log_info("addOutput failed for physical_response_header");
          return UNC_UPPL_RC_ERR_IPC_WRITE_ERROR;
        }
        return UNC_RC_SUCCESS;
      }
    }

      int err = PhyUtil::sessOutRespHeader(sess, rsh);
      if (err != 0) {
        pfc_log_info("addOutput failed for physical_response_header");
        return UNC_UPPL_RC_ERR_IPC_WRITE_ERROR;
      }

      for (unsigned int index = 0; index < vect_port_id.size();
          ++index) {
        key_port_t obj_key_port = vect_port_id[index];
        string port_name =  reinterpret_cast<char*>(obj_key_port.port_id);
        pfc_log_debug("Inside the map %s", port_name.c_str());
        map<string, val_port_stats_t> :: iterator it_port_key_val =
                                  nb_port_stats_map.find(port_name);
        if (it_port_key_val == nb_port_stats_map.end()) {
          pfc_log_debug("Skipping %s", port_name.c_str());
          continue;
        }
          val_port_stats_t val_stats_obj = it_port_key_val->second;
          err |= sess.addOutput((uint32_t)UNC_KT_PORT);
          pfc_log_debug("%s", IpctUtil::get_string(obj_key_port).c_str());
          err |= sess.addOutput(obj_key_port);
          pfc_log_debug("%s", IpctUtil::get_string(val_stats_obj).c_str());
          err |= sess.addOutput(val_stats_obj);
          if (err != 0) {
            pfc_log_info("addOutput failed for physical_response_header");
            return UNC_UPPL_RC_ERR_IPC_WRITE_ERROR;
          }
          if (index < vect_port_id.size() - 1) {
            sess.addOutput();  //  Seperator
          }
      }
      return UNC_RC_SUCCESS;
    }
    // U12 END

    if (option1 == UNC_OPT1_NORMAL && option2 == UNC_OPT2_NONE) {
      // For DT_STATE with option2 as none, populate val_port_st
      pfc_log_debug("Populating val_port_st from DB");
      vector<key_port_t> vect_port_id;
      vector<val_port_st_t> vect_val_port_st;
      read_status = ReadPortValFromDB(db_conn, key_struct,
                                      val_struct,
                                      data_type,
                                      operation_type,
                                      max_rep_ct,
                                      vect_val_port_st,
                                      vect_port_id);
      rsh.result_code = read_status;
      rsh.max_rep_count = max_rep_ct;
      pfc_log_debug("read status val in performread = %d", read_status);
      if (read_status != UNC_RC_SUCCESS) {
        rsh.max_rep_count = 0;
        int err = PhyUtil::sessOutRespHeader(sess, rsh);
        err |= sess.addOutput((uint32_t) UNC_KT_PORT);
        err |= sess.addOutput(*obj_key_port);
        if (err != 0) {
          pfc_log_info("addOutput failed for physical_response_header");
          return UNC_UPPL_RC_ERR_IPC_WRITE_ERROR;
        }
        return UNC_RC_SUCCESS;
      }
      int err = PhyUtil::sessOutRespHeader(sess, rsh);
      if (err != 0) {
        pfc_log_info("addOutput failed for physical_response_header");
        return UNC_UPPL_RC_ERR_IPC_WRITE_ERROR;
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
    } else if (option1 == UNC_OPT1_NORMAL && option2 == UNC_OPT2_NEIGHBOR) {
      pfc_log_debug("Read neighbor details from DB");
      val_port_st_neighbor obj_neighbor;
      // Setting the connected neighbor valid bits to INVALID
      memset(&obj_neighbor, '\0', sizeof(val_port_st_neighbor));
      obj_neighbor.valid[kIdxPortConnectedSwitchId] = UNC_VF_INVALID;
      obj_neighbor.valid[kIdxPortConnectedPortId] = UNC_VF_INVALID;
      obj_neighbor.valid[kIdxPortConnectedControllerId] = UNC_VF_INVALID;
      read_status = ReadNeighbor(
        db_conn, key_struct,
        val_struct,
        data_type,
        obj_neighbor);
      pfc_log_debug("Return value for read operation %d", read_status);
      rsh.result_code = read_status;
      int err = PhyUtil::sessOutRespHeader(sess, rsh);
      err |= sess.addOutput((uint32_t)UNC_KT_PORT);
      err |= sess.addOutput(*obj_key_port);
      if (err != 0) {
        pfc_log_info("addOutput failed for physical_response_header");
        return UNC_UPPL_RC_ERR_IPC_WRITE_ERROR;
      }
      if (read_status == UNC_RC_SUCCESS) {
        err = sess.addOutput(obj_neighbor);
        if (err != 0) {
          pfc_log_info("addOutput failed for physical_response_header");
          return UNC_UPPL_RC_ERR_IPC_WRITE_ERROR;
        }
      } else {
        pfc_log_info("Read operation on neighbor failed");
      }
      pfc_log_debug(" %s", IpctUtil::get_string(obj_neighbor).c_str());
    }
    return UNC_RC_SUCCESS;
}

/** 
 * @Description : This function is used to read KT_PORT instance in database
 *                table using key_ctr provided in IPC request and operation_type
 * @param[in]   : key_struct - void pointer to be type cast into port key
 *                structure
 *                value_struct - void pointer to be type cast into port value
 *                structure
 *                data_type - indicates the data base type
 *                operation_type - indicates the operation type UNC_OP*
 *                max_rep_ct - indicates the maximum repetition count
 *                vect_val_port - vector to store the val_port_st_t structure
 *                port_id - vector of type key_port_t to store the
 *                logical port id
 * @return      : UNC_RC_SUCCESS - read operation is success
 *                UNC_UPPL_RC_ERR_DB_GET - read operation is failure
 **/
UncRespCode Kt_Port::ReadPortValFromDB(
    OdbcmConnectionHandler *db_conn,
    void* key_struct,
    void* val_struct,
    uint32_t data_type,
    uint32_t operation_type,
    uint32_t &max_rep_ct,
    vector<val_port_st_t> &vect_val_port_st,
    vector<key_port_t> &port_id) {
  if (operation_type < UNC_OP_READ) {
    // Unsupported operation type for this function
    return UNC_RC_SUCCESS;
  }
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  UncRespCode read_status = UNC_RC_SUCCESS;
  ODBCM_RC_STATUS read_db_status = ODBCM_RC_SUCCESS;
  // Common structures that will be used to send query to ODBC
  // Structure used to send request to ODBC
  DBTableSchema kt_port_dbtableschema;
  void *old_struct;
  vector<ODBCMOperator> vect_key_operations;
  PopulateDBSchemaForKtTable(db_conn, kt_port_dbtableschema,
                             key_struct,
                             val_struct,
                             operation_type, data_type, 0, 0,
                             vect_key_operations, old_struct,
                             NOTAPPLIED, false, PFC_FALSE);

  if (operation_type == UNC_OP_READ) {
    read_db_status = physical_layer->get_odbc_manager()->
        GetOneRow((unc_keytype_datatype_t)data_type,
                  kt_port_dbtableschema, db_conn);
  } else {
    read_db_status = physical_layer->get_odbc_manager()->
        GetBulkRows((unc_keytype_datatype_t)data_type, max_rep_ct,
                    kt_port_dbtableschema,
                    (unc_keytype_operation_t)operation_type, db_conn);
  }
  if (read_db_status == ODBCM_RC_RECORD_NOT_FOUND) {
    pfc_log_debug("No record found");
    read_status = UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE;
    return read_status;
  } else if (read_db_status == ODBCM_RC_CONNECTION_ERROR) {
    // log error to log daemon
    pfc_log_error("DB connection not available or cannot access DB");
    read_status = UNC_UPPL_RC_ERR_DB_ACCESS;
    return read_status;
  } else if (read_db_status != ODBCM_RC_SUCCESS) {
    read_status = UNC_UPPL_RC_ERR_DB_GET;
    // log error to log daemon
    pfc_log_error("Read operation has failed: %d", read_db_status);
    return read_status;
  }

  pfc_log_debug("Read operation Success result: %d", read_status);
  FillPortValueStructure(db_conn, kt_port_dbtableschema,
                         vect_val_port_st,
                         max_rep_ct,
                         UNC_OP_READ,
                         port_id);
  pfc_log_debug("vect_val_port_st size: %"
                PFC_PFMT_SIZE_T, vect_val_port_st.size());
  pfc_log_debug("port_id size: %" PFC_PFMT_SIZE_T, port_id.size());
  if (vect_val_port_st.empty()) {
    // Read failed , return error
    read_status = UNC_UPPL_RC_ERR_DB_GET;
    // log error to log daemon
    pfc_log_error("Read operation has failed after reading response");
    return read_status;
  }
  pfc_log_debug("Read operation Completed with result: %d", read_status);
  return read_status;
}

/** ReadNeighbor
 * @Description : This function reads the Neighbor information from driver
 * @param[in]   : key_struct - void pointer to be type cast to port key type
 *                val_struct - void pointer to be type cast to port val
 *                structure type
 *                data_type - indicates the data base type
 *                neighbor_obj - structure variable of type val_port_st_neighbor
 * @return      : UNC_RC_SUCCESS - is returned if the read neighbor operation
 *                is success
 *                UNC_UPPL_RC_ERR* - is returned if the read neighbor operation is
 *                failure
 **/
UncRespCode Kt_Port::ReadNeighbor(
    OdbcmConnectionHandler *db_conn,
    void* key_struct,
    void* val_struct,
    uint32_t data_type,
    val_port_st_neighbor &neighbor_obj) {
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  UncRespCode read_status = UNC_RC_SUCCESS;
  key_port_t *obj_key_port= reinterpret_cast<key_port_t*>(key_struct);
  string controller_name = (const char*)obj_key_port->
      sw_key.ctr_key.controller_name;
  string switch_id = (const char*)obj_key_port->sw_key.switch_id;
  string port_id = (const char*)obj_key_port->port_id;
  string switch_id2 = "";
  string port_id2 = "";
  // Read Port value from DB
  vector<void *> vect_key_port;
  vect_key_port.push_back(reinterpret_cast<void *>(obj_key_port));
  vector<void *> vect_val_port;
  read_status = ReadInternal(db_conn, vect_key_port,
                             vect_val_port,
                             data_type,
                             UNC_OP_READ);
  if (read_status != UNC_RC_SUCCESS) {
    // unable to read value from Db for port
    pfc_log_info("Neighbour fails - unable to read port value");
    return read_status;
  }
  val_port_st_t *obj_val_st_port =
      reinterpret_cast<val_port_st_t*>(vect_val_port[0]);

  if (obj_val_st_port == NULL) {
    // unable to read value from Db for port
    pfc_log_info("Neighbour fails - unable to read port value");
    return UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE;
  }
  val_port_t obj_val_port = obj_val_st_port->port;
  neighbor_obj.port = obj_val_port;
  neighbor_obj.valid[kIdxPort] = UNC_VF_VALID;
  DBTableSchema kt_port_dbtableschema;
  vector<string> vect_prim_keys;
  vect_prim_keys.push_back(CTR_NAME_STR);
  vect_prim_keys.push_back(LINK_SWITCH_ID1_STR);
  vect_prim_keys.push_back(LINK_PORT_ID1_STR);
  vector<TableAttrSchema> vect_table_attr_schema;
  list< vector<TableAttrSchema> > row_list;
  PhyUtil::FillDbSchema(unc::uppl::CTR_NAME, controller_name,
                        controller_name.length(), DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);
  PhyUtil::FillDbSchema(unc::uppl::LINK_SWITCH_ID1, switch_id,
                        switch_id.length(), DATATYPE_UINT8_ARRAY_256,
                        vect_table_attr_schema);
  PhyUtil::FillDbSchema(unc::uppl::LINK_PORT_ID1, port_id,
                        port_id.length(), DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);
  PhyUtil::FillDbSchema(unc::uppl::LINK_SWITCH_ID2, switch_id2,
                        switch_id2.length(), DATATYPE_UINT8_ARRAY_256,
                        vect_table_attr_schema);
  PhyUtil::FillDbSchema(unc::uppl::LINK_PORT_ID2, port_id2,
                        port_id2.length(), DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);
  kt_port_dbtableschema.set_table_name(unc::uppl::LINK_TABLE);
  kt_port_dbtableschema.set_primary_keys(vect_prim_keys);
  row_list.push_back(vect_table_attr_schema);
  kt_port_dbtableschema.set_row_list(row_list);
  ODBCM_RC_STATUS read_db_status = physical_layer->get_odbc_manager()->
      GetOneRow((unc_keytype_datatype_t)data_type, kt_port_dbtableschema,
                db_conn);
  if (read_db_status == ODBCM_RC_SUCCESS) {
    read_status = UNC_RC_SUCCESS;
    // populate IPC value structure based on the response received from DB
    vector<TableAttrSchema> res_table_attr_schema =
        kt_port_dbtableschema.get_row_list().front();
    vector<TableAttrSchema> ::iterator vect_iter =
        res_table_attr_schema.begin();
    neighbor_obj.valid[kIdxPortConnectedSwitchId] = UNC_VF_VALID;
    neighbor_obj.valid[kIdxPortConnectedPortId] = UNC_VF_VALID;
    for (; vect_iter != res_table_attr_schema.end(); ++vect_iter) {
      TableAttrSchema tab_schema = (*vect_iter);
      ODBCMTableColumns attr_name = tab_schema.table_attribute_name;
      if (attr_name == unc::uppl::LINK_SWITCH_ID2) {
        PhyUtil::GetValueFromDbSchemaStr(tab_schema,
                                         neighbor_obj.connected_switch_id,
                                         DATATYPE_UINT8_ARRAY_256);
        pfc_log_debug("Connected switch id: %s",
                      neighbor_obj.connected_switch_id);
      } else if (attr_name == unc::uppl::LINK_PORT_ID2) {
        PhyUtil::GetValueFromDbSchemaStr(tab_schema,
                                         neighbor_obj.connected_port_id,
                                         DATATYPE_UINT8_ARRAY_32);
        pfc_log_debug("Connected port id: %s", neighbor_obj.connected_port_id);
      }
    }
  } else {
    if (read_db_status == ODBCM_RC_RECORD_NOT_FOUND) {
      pfc_log_debug("No record to read in ReadNeighbor");
      read_status = UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE;
    } else if (read_db_status == ODBCM_RC_CONNECTION_ERROR) {
      // log error to log daemon
      pfc_log_error("DB connection not available or cannot access DB");
      read_status = UNC_UPPL_RC_ERR_DB_ACCESS;
    } else if (read_db_status != ODBCM_RC_SUCCESS) {
      read_status = UNC_UPPL_RC_ERR_DB_GET;
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
  if (read_status == UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE) {
    // unable to read value from Db(link table) for port
    // Read the value from port table
      vector<key_port_t> vect_key_port;
      vector<val_port_st_neighbor_t> vect_val_nbr;
      Kt_Port_Neighbor port_neigh_obj;
      uint32_t max_rep_ct = 1;
      read_status = port_neigh_obj.ReadPortNeighbor(db_conn,
                    data_type, key_struct, UNC_OP_READ, max_rep_ct,
                    vect_val_nbr, vect_key_port);
     if (read_status != UNC_RC_SUCCESS) {
      pfc_log_error("ReadPortNeighor failed with read_status:%d", read_status);
      return read_status;
     }
    memcpy(neighbor_obj.connected_controller_id, vect_val_nbr[0].connected_controller_id,
                     sizeof(neighbor_obj.connected_controller_id));
    memcpy(neighbor_obj.connected_switch_id, vect_val_nbr[0].connected_switch_id,
                     sizeof(neighbor_obj.connected_switch_id));
    memcpy(neighbor_obj.connected_port_id, vect_val_nbr[0].connected_port_id,
                     sizeof(neighbor_obj.connected_port_id));
    memcpy(neighbor_obj.valid, vect_val_nbr[0].valid,
                     sizeof(neighbor_obj.valid));
    //Resetting neighbor_obj.valid[kIdxPort] as it will be overwritten 
    neighbor_obj.valid[kIdxPort] = UNC_VF_VALID;
    bool SetNeighAttrInvalid =  false;
    if ((neighbor_obj.valid[kIdxPortConnectedControllerId] == UNC_VF_VALID) &&
     (neighbor_obj.valid[kIdxPortConnectedPortId] == UNC_VF_VALID) &&
     (neighbor_obj.valid[kIdxPortConnectedSwitchId] == UNC_VF_VALID) &&
     (strlen((const char*)neighbor_obj.connected_controller_id) != 0) &&
     (strlen((const char*)neighbor_obj.connected_switch_id) != 0) &&
     (strlen((const char*)neighbor_obj.connected_port_id) != 0)) {
       string ctr_name = "";
       string actual_controller_id = reinterpret_cast<char*>(
                                       neighbor_obj.connected_controller_id);
       read_status  = PhyUtil::ConvertToControllerName(db_conn,
                        actual_controller_id, ctr_name);
       if (read_status == UNC_RC_SUCCESS) {
         if (ctr_name != "") {
            memcpy(neighbor_obj.connected_controller_id, ctr_name.c_str(),
            ctr_name.length()+1);
         } else {
           SetNeighAttrInvalid = true;
         }
       } else {
         SetNeighAttrInvalid = true;
       }
    } else {
      SetNeighAttrInvalid = true;
    }

    if (SetNeighAttrInvalid == true) {
      pfc_log_debug("Setting the NeighAttr as INALID");
      neighbor_obj.valid[kIdxPortConnectedControllerId] = UNC_VF_INVALID;
      neighbor_obj.valid[kIdxPortConnectedSwitchId] = UNC_VF_INVALID;
      neighbor_obj.valid[kIdxPortConnectedPortId] = UNC_VF_INVALID;
      memset(neighbor_obj.connected_controller_id, '\0',
             sizeof(neighbor_obj.connected_controller_id));
      memset(neighbor_obj.connected_switch_id, '\0',
             sizeof(neighbor_obj.connected_switch_id));
      memset(neighbor_obj.connected_port_id, '\0',
             sizeof(neighbor_obj.connected_port_id));
      read_status = UNC_RC_SUCCESS;
    }
  }
  return read_status;
}

/** 
 * @Description : This function populates the values to be used for attribute
 *                validation
 * @param[in] : None
 * @return    : None
 **/
void Kt_Port::Fill_Attr_Syntax_Map() {
  map<string, Kt_Class_Attr_Syntax> attr_syntax_map;

  Kt_Class_Attr_Syntax objKeyAttrCtrSyntax =
  { PFC_IPCTYPE_UINT8, 0, 0, 1, 32, true, "" };
  attr_syntax_map[CTR_NAME_STR] = objKeyAttrCtrSyntax;

  Kt_Class_Attr_Syntax objKeyAttrSwitchSyntax =
  { PFC_IPCTYPE_UINT8, 0, 0, 1, 32, true, "" };
  attr_syntax_map[SWITCH_ID_STR] = objKeyAttrSwitchSyntax;

  Kt_Class_Attr_Syntax objKeyAttrPortSyntax =
  { PFC_IPCTYPE_UINT8, 0, 0, 1, 32, true, "" };
  attr_syntax_map[PORT_ID_STR] = objKeyAttrPortSyntax;

  Kt_Class_Attr_Syntax objAttrPortNumberSyntax =
  { PFC_IPCTYPE_UINT32, 1, 4294967295LL, 0, 0, false, "" };
  attr_syntax_map[PORT_NUMBER_STR] = objAttrPortNumberSyntax;

  Kt_Class_Attr_Syntax objAttrDescSyntax =
  { PFC_IPCTYPE_STRING, 0, 0, 0, 128, false, "" };
  attr_syntax_map[PORT_DESCRIPTION_STR] = objAttrDescSyntax;

  Kt_Class_Attr_Syntax objAttrAdminStatusSyntax =
  { PFC_IPCTYPE_UINT8, 0, 1, 0, 0, false, "" };
  attr_syntax_map[PORT_ADMIN_STATUS_STR] = objAttrAdminStatusSyntax;

  Kt_Class_Attr_Syntax objAttrDirectionSyntax =
  { PFC_IPCTYPE_UINT8, 0, 2, 0, 0, false, "" };
  attr_syntax_map[PORT_DIRECTION_STR] = objAttrDirectionSyntax;

  Kt_Class_Attr_Syntax objAttrTrunkAVlanSyntax =
  { PFC_IPCTYPE_UINT16, 0, 65535, 0, 0, false, "" };
  attr_syntax_map[PORT_TRUNK_ALL_VLAN_STR] = objAttrTrunkAVlanSyntax;

  Kt_Class_Attr_Syntax objAttrOperStatusSyntax =
  { PFC_IPCTYPE_UINT8, 0, 2, 0, 0, false, "" };
  attr_syntax_map[PORT_OPER_STATUS_STR] = objAttrOperStatusSyntax;

  Kt_Class_Attr_Syntax objAttrMacAddressSyntax =
  { PFC_IPCTYPE_STRING, 0, 0, 0, 6, false, "" };
  attr_syntax_map[PORT_MAC_ADDRESS_STR] = objAttrMacAddressSyntax;

  Kt_Class_Attr_Syntax objAttrDuplexSyntax =
  { PFC_IPCTYPE_UINT8, 0, 1, 0, 0, false, "" };
  attr_syntax_map[PORT_DUPLEX_STR] = objAttrDuplexSyntax;

  Kt_Class_Attr_Syntax objAttrSpeedSyntax =
  { PFC_IPCTYPE_UINT64, 0, 0, 0, 0, false, "" };
  attr_syntax_map[PORT_SPEED_STR] = objAttrSpeedSyntax;

  Kt_Class_Attr_Syntax objAttrAlarmsStatusSyntax =
  { PFC_IPCTYPE_UINT64, 1, 3, 0, 0, false, "" };
  attr_syntax_map[PORT_ALARM_STATUS_STR] = objAttrAlarmsStatusSyntax;

  Kt_Class_Attr_Syntax objAttrLogicalPortIdSyntax =
  { PFC_IPCTYPE_STRING, 0, 0, 0, 320, false, "" };
  attr_syntax_map[PORT_LOGIC_PORT_ID_STR] = objAttrLogicalPortIdSyntax;

  Kt_Class_Attr_Syntax objAttrValidSyntax =
  { PFC_IPCTYPE_STRING, 0, 0, 0, 12, false, "" };
  attr_syntax_map[PORT_VALID_STR] = objAttrValidSyntax;

  attr_syntax_map_all[UNC_KT_PORT] = attr_syntax_map;
}

/** 
 * @Description : This function forms the valid flag based on update req
 * @param[in]   : key_struct - void pointer to be type cast to port key type
 *                val_struct - void pointer to be type cast to port value
 *                structure type
 *                val_port_valid_st - structure variable of type
 *                val_port_valid_st
 *                new_valid_val - enum for valid flag
 * @return      : UNC_RC_SUCCESS - if valid flag update is success 
 *                UPL_RC_ERR_* - if valid flag update is failure
 **/
UncRespCode Kt_Port::UpdatePortValidFlag(
    OdbcmConnectionHandler *db_conn,
    void *key_struct,
    void *val_struct,
    val_port_st_t &val_port_valid_st,
    unc_keytype_validflag_t new_valid_val,
    uint32_t data_type) {
  UncRespCode return_code = UNC_RC_SUCCESS;
  // read the value structure from db
  void *obj_key_port_orig = key_struct;
  vector<void *> vect_key_port;
  vect_key_port.push_back(key_struct);
  vector<void *> vect_val_port;
  return_code = ReadInternal(db_conn, vect_key_port, vect_val_port,
                             data_type, UNC_OP_READ);
  if (return_code != UNC_RC_SUCCESS) {
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
    // For all valid attributes update stream to valid, for invalid attributes
    // update stream with valid value read from db.
    // store the final valid val to stream
    stringstream ss_new;

    // port number
    unsigned int valid_val =
        PhyUtil::uint8touint(obj_val_port->port.valid[kIdxPortNumber]);
    if (PhyUtil::IsValidValue(operation_type, valid_val) == true) {
      ss_new << new_valid_val;
    } else {
      ss_new << PhyUtil::uint8touint(
          obj_new_val_port->port.valid[kIdxPortNumber]);
      pfc_log_debug("invalid value for port_number ignore the value");
    }

    // valid val of desc
    valid_val =
        PhyUtil::uint8touint(obj_val_port->port.valid[kIdxPortDescription]);
    if (PhyUtil::IsValidValue(operation_type, valid_val) == true) {
      ss_new << new_valid_val;
    } else {
      ss_new << PhyUtil::uint8touint(
          obj_new_val_port->port.valid[kIdxPortDescription]);
      pfc_log_debug("invalid value for desc ignore the value");
    }

    // valid val of admin_status
    valid_val =
        PhyUtil::uint8touint(obj_val_port->port.valid[kIdxPortAdminStatus]);
    if (PhyUtil::IsValidValue(operation_type, valid_val) == true) {
      ss_new << new_valid_val;
    } else {
      ss_new << PhyUtil::uint8touint(
          obj_new_val_port->port.valid[kIdxPortAdminStatus]);
      pfc_log_debug("invalid value for admin_status ignore the value");
    }

    // valid val of trunkallowed
    valid_val =
        PhyUtil::uint8touint(
            obj_val_port->port.valid[kIdxPortTrunkAllowedVlan]);
    if (PhyUtil::IsValidValue(operation_type, valid_val) == true) {
      ss_new << new_valid_val;
    } else {
      ss_new << PhyUtil::uint8touint(
          obj_new_val_port->port.valid[kIdxPortTrunkAllowedVlan]);
      pfc_log_debug("invalid value for trunkallowed ignore the value");
    }

    // valid val of portoperstatus
    valid_val =
        PhyUtil::uint8touint(obj_val_port->valid[kIdxPortOperStatus]);
    if (PhyUtil::IsValidValue(operation_type, valid_val) == true) {
      ss_new << new_valid_val;

    } else {
      ss_new << PhyUtil::uint8touint(
          obj_new_val_port->valid[kIdxPortOperStatus]);
      pfc_log_debug("invalid value for operstatus ignore the value");
    }

    // valid val of macaddress
    valid_val =
        PhyUtil::uint8touint(obj_val_port->valid[kIdxPortMacAddress]);
    if (PhyUtil::IsValidValue(operation_type, valid_val) == true) {
      ss_new << new_valid_val;
    } else {
      ss_new << PhyUtil::uint8touint(
          obj_new_val_port->valid[kIdxPortMacAddress]);
      pfc_log_debug("invalid value for macaddress ignore the value");
    }

    // valid val of portdirection
    valid_val =
        PhyUtil::uint8touint(obj_val_port->valid[kIdxPortDirection]);
    if (PhyUtil::IsValidValue(operation_type, valid_val) == true) {
      ss_new << new_valid_val;
    } else {
      ss_new << PhyUtil::uint8touint(
          obj_new_val_port->valid[kIdxPortDirection]);
      pfc_log_debug("invalid value for portdirection ignore the value");
    }

    // valid val of portduplex
    valid_val = PhyUtil::uint8touint(obj_val_port->valid[kIdxPortDuplex]);
    if (PhyUtil::IsValidValue(operation_type, valid_val) == true) {
      ss_new << new_valid_val;
    } else {
      ss_new << PhyUtil::uint8touint(obj_new_val_port->valid[kIdxPortDuplex]);
      pfc_log_debug("invalid value for portduplex ignore the value");
    }

    // valid val of portspeed
    valid_val = PhyUtil::uint8touint(obj_val_port->valid[kIdxPortSpeed]);
    if (PhyUtil::IsValidValue(operation_type, valid_val) == true) {
      ss_new << new_valid_val;
    } else {
      ss_new << PhyUtil::uint8touint(obj_new_val_port->valid[kIdxPortSpeed]);
      pfc_log_debug("invalid value for portspeed ignore the value");
    }

    // valid val of alarmstatus
    valid_val =
        PhyUtil::uint8touint(obj_val_port->valid[kIdxPortAlarmsStatus]);
    if (PhyUtil::IsValidValue(operation_type, valid_val) == true) {
      ss_new << new_valid_val;
    } else {
      ss_new << PhyUtil::uint8touint(
          obj_new_val_port->valid[kIdxPortAlarmsStatus]);
      pfc_log_debug("invalid value for alrmstatus ignore the value");
    }

    // valid val of logicalportid
    valid_val =
        PhyUtil::uint8touint(obj_val_port->valid[kIdxPortLogicalPortId]);
    if (PhyUtil::IsValidValue(operation_type, valid_val) == true) {
      ss_new << new_valid_val;
    } else {
      ss_new << PhyUtil::uint8touint(
          obj_new_val_port->valid[kIdxPortLogicalPortId]);
      pfc_log_debug("invalid value for logicalportid ignore the value");
    }
    pfc_log_debug("update port final valid val:%s", ss_new.str().c_str());
    // call populate schema for valid update
    return_code = PopulateSchemaForValidFlag(
        db_conn,
        obj_key_port_orig,
        reinterpret_cast<void*> (&obj_new_val_port),
        ss_new.str().c_str(), data_type);
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

/** 
 * @Description : This function updates the valid flag value
 *                of the port
 * @param[in]   : key_struct - void pointer to be type cast to port key type
 *                structure
 *                val_struct - void pointer to be type cast to port value type
 *                structure
 *                valid_new - stores the valid flag to be updated in Db
 * @return      : UNC_RC_SUCCESS - if port valid update operation is success
 *                UNC_UPPL_RC_ERR_* - if port valid update operation is failure
 **/
UncRespCode Kt_Port::PopulateSchemaForValidFlag(
    OdbcmConnectionHandler *db_conn,
    void* key_struct,
    void* val_struct,
    string valid_new,
    uint32_t data_type) {
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  key_port_t *obj_key_port=
      reinterpret_cast<key_port_t*>(key_struct);
  vector<TableAttrSchema> vect_table_attr_schema;
  list < vector<TableAttrSchema> > row_list;

  vector<string> vect_prim_keys;
  vect_prim_keys.push_back(CTR_NAME_STR);
  vect_prim_keys.push_back(SWITCH_ID_STR);
  vect_prim_keys.push_back(PORT_ID_STR);


  string controller_name = (const char*)obj_key_port->sw_key.
      ctr_key.controller_name;
  string switch_id = (const char*)obj_key_port->
      sw_key.switch_id;
  string port_id = (const char*)obj_key_port->port_id;
  if (!controller_name.empty()) {
    PhyUtil::FillDbSchema(unc::uppl::CTR_NAME, controller_name,
                          (controller_name.length()+1), DATATYPE_UINT8_ARRAY_32,
                          vect_table_attr_schema);
  }

  if (!switch_id.empty()) {
    PhyUtil::FillDbSchema(unc::uppl::SWITCH_ID, switch_id,
                          (switch_id.length()+1), DATATYPE_UINT8_ARRAY_256,
                          vect_table_attr_schema);
  }

  if (!port_id.empty()) {
    PhyUtil::FillDbSchema(unc::uppl::PORT_ID, port_id,
                          (port_id.length()+1), DATATYPE_UINT8_ARRAY_32,
                          vect_table_attr_schema);
  }

  pfc_log_debug("valid new val:%s", valid_new.c_str());
  PhyUtil::FillDbSchema(unc::uppl::PORT_VALID, valid_new.c_str(),
                        11, DATATYPE_UINT8_ARRAY_11,
                        vect_table_attr_schema);

  DBTableSchema kt_port_dbtableschema;
  kt_port_dbtableschema.set_table_name(unc::uppl::PORT_TABLE);
  kt_port_dbtableschema.set_primary_keys(vect_prim_keys);
  row_list.push_back(vect_table_attr_schema);
  kt_port_dbtableschema.set_row_list(row_list);

  // Call ODBCManager and update
  ODBCM_RC_STATUS update_db_status =
      physical_layer->get_odbc_manager()->UpdateOneRow(
          unc_keytype_datatype_t(data_type),
          kt_port_dbtableschema,
          db_conn, true);
  if (update_db_status != ODBCM_RC_SUCCESS) {
    // log error
    pfc_log_error("port valid update operation failed");
    return UNC_UPPL_RC_ERR_DB_UPDATE;
  }
  return UNC_RC_SUCCESS;
}

/**
 * @Description : This function converts the string value from db to uint8
 * @param[in]   : attr_value - string to store attribute value
 *                obj_val_port - struct variables of val_port_st structure
 * @return      : None
 **/

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

/** 
 * @Description : This function reads the values given in val_port structure
 * @param[in]   : obj_val_port - pointer to structure val_port_st_t
 *                vect_table_attr_schema - vector to store the db table
 *                attribute schema
 *                vect_prim_keys - vector to store the primary keys of port key
 *                type
 *                operation_type - indicates the operation type
 *                val_port_valid_st - pointer to the structure val_port_st_t
 *                valid - reference variable to store the valid flags of port
 *                value structure
 * @return      : None 
 **/
void Kt_Port::GetPortValStructure(
    OdbcmConnectionHandler *db_conn,
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

  string value = "";
  // Port_number
  if (obj_val_port != NULL) {
    if (valid_value_struct == UNC_VF_VALID) {
      valid_val = PhyUtil::uint8touint(
        obj_val_port->port.valid[kIdxPortNumber]);
      value = PhyUtil::uint32tostr(obj_val_port->port.port_number);
    } else if (valid_value_struct == UNC_VF_VALID_NO_VALUE) {
      valid_val = UNC_VF_VALID_NO_VALUE;
    }
    if (operation_type == UNC_OP_UPDATE) {
      prev_db_val =
          PhyUtil::uint8touint(
              val_port_valid_st->port.valid[kIdxPortNumber]);
    }
  } else {
    valid_val = UPPL_NO_VAL_STRUCT;
  }
  PhyUtil::FillDbSchema(unc::uppl::PORT_NUMBER, PORT_NUMBER_STR, value,
                        value.length(), DATATYPE_UINT32,
                        operation_type, valid_val, prev_db_val,
                        vect_table_attr_schema, vect_prim_keys, valid);
  value.clear();
  // Description
  if (obj_val_port != NULL) {
    if (valid_value_struct == UNC_VF_VALID) {
      valid_val = PhyUtil::uint8touint(
          obj_val_port->port.valid[kIdxPortDescription]);
      value = (const char*)obj_val_port->port.description;
    } else if (valid_value_struct == UNC_VF_VALID_NO_VALUE) {
      valid_val = UNC_VF_VALID_NO_VALUE;
    }
    if (operation_type == UNC_OP_UPDATE) {
      prev_db_val =
          PhyUtil::uint8touint(
              val_port_valid_st->port.valid[kIdxPortDescription]);
    }
  } else {
    valid_val = UPPL_NO_VAL_STRUCT;
  }
  PhyUtil::FillDbSchema(unc::uppl::PORT_DESCRIPTION, PORT_DESCRIPTION_STR,
                        value, value.length(), DATATYPE_UINT8_ARRAY_128,
                        operation_type, valid_val, prev_db_val,
                        vect_table_attr_schema, vect_prim_keys, valid);
  value.clear();
  // Admin_status
  if (obj_val_port != NULL) {
    if (valid_value_struct == UNC_VF_VALID) {
      valid_val = PhyUtil::uint8touint(
          obj_val_port->port.valid[kIdxPortAdminStatus]);
      value = PhyUtil::uint8tostr(obj_val_port->port.admin_status);
    } else if (valid_value_struct == UNC_VF_VALID_NO_VALUE) {
      valid_val = UNC_VF_VALID_NO_VALUE;
    }
    if (operation_type == UNC_OP_UPDATE) {
      prev_db_val =
          PhyUtil::uint8touint(
              val_port_valid_st->port.valid[kIdxPortAdminStatus]);
    }
  } else {
    valid_val = UPPL_NO_VAL_STRUCT;
  }
  PhyUtil::FillDbSchema(unc::uppl::PORT_ADMIN_STATUS, PORT_ADMIN_STATUS_STR,
                        value, value.length(), DATATYPE_UINT16,
                        operation_type, valid_val, prev_db_val,
                        vect_table_attr_schema, vect_prim_keys, valid);
  value.clear();
  // trunk_allowed_vlan
  if (obj_val_port != NULL) {
    if (valid_value_struct == UNC_VF_VALID) {
      valid_val = PhyUtil::uint8touint(obj_val_port->port.valid\
                                     [kIdxPortTrunkAllowedVlan]);
      value = PhyUtil::uint16tostr(obj_val_port->\
                                port.trunk_allowed_vlan);
    } else if (valid_value_struct == UNC_VF_VALID_NO_VALUE) {
      valid_val = UNC_VF_VALID_NO_VALUE;
    }
    if (operation_type == UNC_OP_UPDATE) {
      prev_db_val =
          PhyUtil::uint8touint(
              val_port_valid_st->port.valid[kIdxPortTrunkAllowedVlan]);
    }
  } else {
    valid_val = UPPL_NO_VAL_STRUCT;
  }
  PhyUtil::FillDbSchema(unc::uppl::PORT_TRUNK_ALL_VLAN, PORT_TRUNK_ALL_VLAN_STR,
                        value, value.length(), DATATYPE_UINT16,
                        operation_type, valid_val, prev_db_val,
                        vect_table_attr_schema, vect_prim_keys, valid);
  return;
}

/** 
 * @Description : This function reads the values given in val_port_st structure
 * @param[in]   : obj_val_port - pointer to structure val_port_st_t
 *                vect_table_attr_schema - vector to store the table attribute
 *                schema of key type port
 *                vect_prim_keys - vector to store the primary keys of port key
 *                type
 *                operation_type - indicates the operation type
 *                val_port_valid_st - pointer to the structure val_port_st_t
 *                valid - reference variable to store the valid flags of port
 *                value structure
 * @return      : None
 **/
void Kt_Port::GetPortStateValStructure(
    OdbcmConnectionHandler *db_conn,
    val_port_st_t *obj_val_port,
    vector<TableAttrSchema> &vect_table_attr_schema,
    vector<string> &vect_prim_keys,
    uint8_t operation_type,
    val_port_st_t *val_port_valid_st,
    stringstream &valid) {
  string value = "";
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
  PhyUtil::FillDbSchema(unc::uppl::PORT_OPER_STATUS, PORT_OPER_STATUS_STR,
                        value, value.length(), DATATYPE_UINT16,
                        operation_type, valid_val, prev_db_val,
                        vect_table_attr_schema, vect_prim_keys, valid);
  value.clear();
  // Mac_address
  if (obj_val_port != NULL) {
    valid_val = PhyUtil::uint8touint(obj_val_port->valid[kIdxPortMacAddress]);
    if (operation_type == UNC_OP_UPDATE) {
      prev_db_val =
          PhyUtil::uint8touint(
              val_port_valid_st->valid[kIdxPortMacAddress]);
    }
  } else {
    valid_val = UPPL_NO_VAL_STRUCT;
  }
  if (valid_val == UNC_VF_VALID) {
    PhyUtil::FillDbSchema(unc::uppl::PORT_MAC_ADDRESS, PORT_MAC_ADDRESS_STR,
                          obj_val_port->mac_address, ODBCM_SIZE_6,
                          DATATYPE_UINT8_ARRAY_6,
                          operation_type, valid_val, prev_db_val,
                          vect_table_attr_schema, vect_prim_keys, valid);
  } else {
    PhyUtil::FillDbSchema(unc::uppl::PORT_MAC_ADDRESS, PORT_MAC_ADDRESS_STR,
                          value, value.length(), DATATYPE_UINT8_ARRAY_6,
                          operation_type, valid_val, prev_db_val,
                          vect_table_attr_schema, vect_prim_keys, valid);
  }

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
  PhyUtil::FillDbSchema(unc::uppl::PORT_DIRECTION, PORT_DIRECTION_STR, value,
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
  PhyUtil::FillDbSchema(unc::uppl::PORT_DUPLEX, PORT_DUPLEX_STR, value,
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
  PhyUtil::FillDbSchema(unc::uppl::PORT_SPEED, PORT_SPEED_STR, value,
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
  PhyUtil::FillDbSchema(unc::uppl::PORT_ALARM_STATUS, PORT_ALARM_STATUS_STR,
                        value, value.length(), DATATYPE_UINT64,
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
  PhyUtil::FillDbSchema(unc::uppl::PORT_LOGIC_PORT_ID, PORT_LOGIC_PORT_ID_STR,
                        value, value.length(), DATATYPE_UINT8_ARRAY_320,
                        operation_type, valid_val, prev_db_val,
                        vect_table_attr_schema, vect_prim_keys, valid);
  stringstream dummy_valid;
  valid_val = UPPL_NO_VAL_STRUCT;
  prev_db_val = 0;
  // valid
  PhyUtil::FillDbSchema(unc::uppl::PORT_VALID, valid.str(),
                        valid.str().length(), DATATYPE_UINT8_ARRAY_11,
                        vect_table_attr_schema);
  return;
}

/**
 * @Description : This function performs oper status handling for sub domain
 * @param[in]   : data_type - indicates the data base type
 *                controller_name, switch_name and physical_port id
 * @return      : UNC_RC_SUCCESS
 **/
UncRespCode Kt_Port::SubDomainOperStatusHandling(
    OdbcmConnectionHandler *db_conn,
    uint32_t data_type,
    string controller_name,
    string switch_id,
    string physical_port_id) {
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  DBTableSchema kt_logicalport_dbtableschema;
  vector<string> vect_prim_keys;
  vector<TableAttrSchema> vect_table_attr_schema;
  list< vector<TableAttrSchema> > row_list;
  vect_prim_keys.push_back(CTR_NAME_STR);
  PhyUtil::FillDbSchema(unc::uppl::CTR_NAME, controller_name,
                        controller_name.length(), DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);
  vect_prim_keys.push_back(LP_SWITCH_ID_STR);
  PhyUtil::FillDbSchema(unc::uppl::LMP_SWITCH_ID, switch_id,
                        switch_id.length(), DATATYPE_UINT8_ARRAY_256,
                        vect_table_attr_schema);
  vect_prim_keys.push_back(LMP_PHYSICAL_PORT_ID_STR);
  PhyUtil::FillDbSchema(unc::uppl::LMP_PHYSICAL_PORT_ID, physical_port_id,
                        physical_port_id.length(), DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);
  string log_port_id = "";
  vect_prim_keys.push_back(LMP_LP_PORT_ID_STR);
  PhyUtil::FillDbSchema(unc::uppl::LMP_LP_PORT_ID, log_port_id,
                        log_port_id.length(), DATATYPE_UINT8_ARRAY_320,
                        vect_table_attr_schema);
  string domain_name = "";
  PhyUtil::FillDbSchema(unc::uppl::DOMAIN_NAME, domain_name,
                        domain_name.length(), DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);
  kt_logicalport_dbtableschema.set_table_name(
      unc::uppl::LOGICAL_MEMBERPORT_TABLE);
  kt_logicalport_dbtableschema.set_primary_keys(vect_prim_keys);

  row_list.push_back(vect_table_attr_schema);
  kt_logicalport_dbtableschema.set_row_list(row_list);

  ODBCM_RC_STATUS db_status = physical_layer->get_odbc_manager()->
      GetBulkRows((unc_keytype_datatype_t)data_type, UPPL_MAX_REP_CT,
                  kt_logicalport_dbtableschema,
                  (unc_keytype_operation_t)UNC_OP_READ_SIBLING_BEGIN,
                  db_conn);
  if (db_status != ODBCM_RC_SUCCESS) {
    pfc_log_debug("No associated logical member port");
    return UNC_RC_SUCCESS;
  }
  // To traverse the list
  list<vector<TableAttrSchema> > ::iterator iter_list;
  for (iter_list = kt_logicalport_dbtableschema.row_list_.begin();
      iter_list != kt_logicalport_dbtableschema.row_list_.end();
      ++iter_list) {
    vector<TableAttrSchema> &attributes_vector = *iter_list;
    vector<TableAttrSchema> :: iterator iter_vector;
    for (iter_vector = attributes_vector.begin();
        iter_vector != attributes_vector.end();
        ++iter_vector) {
      /* Get attribute name of a row */
      TableAttrSchema tab_att_schema = (*iter_vector);
      switch (tab_att_schema.table_attribute_name) {
        case unc::uppl::LMP_LP_PORT_ID:
          uint8_t lp_id[ODBCM_SIZE_320];
          PhyUtil::GetValueFromDbSchemaStr(tab_att_schema, lp_id,
                                           DATATYPE_UINT8_ARRAY_320);
          log_port_id = reinterpret_cast<const char*> (lp_id);
          break;
        case unc::uppl::DOMAIN_NAME:
          uint8_t lp_domain_name[ODBCM_SIZE_32];
          PhyUtil::GetValueFromDbSchemaStr(tab_att_schema, lp_domain_name,
                                           DATATYPE_UINT8_ARRAY_32);
          domain_name = reinterpret_cast<const char*> (lp_domain_name);
          break;
        default:
          break;
      }
    }
    // Call LogicalPort Handle Oper status
    // Notify Kt_Logical_Port
    key_logical_port_t logical_port_key;
    memset(&logical_port_key, 0, sizeof(key_logical_port_t));
    memcpy(logical_port_key.domain_key.ctr_key.controller_name,
           controller_name.c_str(),
           controller_name.length()+1);
    memcpy(logical_port_key.domain_key.domain_name,
           domain_name.c_str(),
           domain_name.length()+1);
    memcpy(logical_port_key.port_id,
           log_port_id.c_str(),
           log_port_id.length()+1);
    vector<OperStatusHolder> ref_oper_status;
    GET_ADD_CTRL_OPER_STATUS(controller_name, ref_oper_status,
                             data_type, db_conn);
    Kt_LogicalPort log_port_obj;
    UncRespCode return_code = log_port_obj.HandleOperStatus(
        db_conn, data_type, reinterpret_cast<void *> (&logical_port_key),
        NULL, ref_oper_status, UNC_KT_PORT);
    pfc_log_debug("HandleOperStatus for logical port class %d",
                  return_code);
    ClearOperStatusHolder(ref_oper_status);
  }
  return UNC_RC_SUCCESS;
}
