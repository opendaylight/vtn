/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/**
 * @brief   KT Link implementation
 * @file    itc_kt_link.cc
 *
 */

#include "itc_kt_link.hh"
#include "itc_kt_controller.hh"
using unc::uppl::PhysicalLayer;

/** Constructor
 * * @Description : This function instantiates parent and child key types for
 * kt_link
 * * * @param[in] : None
 * * * @return    : None
 * */
Kt_Link::Kt_Link() {
  // Populate structure to be used for syntax validation
  parent = NULL;
  Fill_Attr_Syntax_Map();
}

/** Destructor
 * * @Description : This function clears the parent and child key types
 * instances for kt_link
 * * * @param[in] : None
 * * * @return    : None
 * */
Kt_Link::~Kt_Link() {
}

/**DeleteKeyInstance
 * * @Description : This function deletes a row of KT_Link in
 * state link table.
 * * @param[in] :
 * key_struct - the key for the new kt link instance
 * data_type - UNC_DT_* , delete only allowed in state
 * * @return    : UPPL_RC_SUCCESS is returned when the delete
 * is done successfully.
 * UPPL_RC_ERR_* is returned when the delete is error
 * */
UpplReturnCode Kt_Link::DeleteKeyInstance(void* key_struct,
                                          uint32_t data_type,
                                          uint32_t key_type) {
  UpplReturnCode delete_status = UPPL_RC_SUCCESS;
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  // Check operation is allowed on the given DT, skip the check if flag is true
  if ((unc_keytype_datatype_t)data_type != UNC_DT_STATE &&
      (unc_keytype_datatype_t)data_type != UNC_DT_IMPORT) {
    pfc_log_error("Delete operation is provided on unsupported data type");
    return UPPL_RC_ERR_OPERATION_NOT_ALLOWED;
  }
  // Structure used to send request to ODBC
  DBTableSchema kt_link_dbtableschema;
  vector<ODBCMOperator> operator_vector;
  // Populate DBSchema for link_table
  void* old_value;
  PopulateDBSchemaForKtTable(kt_link_dbtableschema,
                             key_struct,
                             NULL,
                             UNC_OP_DELETE,
                             UNC_OPT1_NORMAL,
                             UNC_OPT2_NONE,
                             operator_vector,
                             old_value);
  ODBCM_RC_STATUS delete_db_status = physical_layer->get_odbc_manager()-> \
      DeleteOneRow((unc_keytype_datatype_t)data_type,
                   kt_link_dbtableschema);
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
    pfc_log_info("Delete of a link in data_type(%d) is success",
                 data_type);
  }
  return delete_status;
}

/** ReadInternal
 * * @Description : This function reads the given  instance of KT_Link
 ** * @param[in] : session_id - ipc session id used for TC validation
 * key_struct - the key for the kt link instance
 * value_struct - the value for the kt link instance
 * data_type - UNC_DT_* , read allowed in state
 * * @return    : UPPL_RC_SUCCESS is returned when the response
 * is added to ipc session successfully.
 * UPPL_RC_ERR_* is returned when ipc response could not be added to sess.
 * */
UpplReturnCode Kt_Link::ReadInternal(vector<void *> &key_val,
                                     vector<void *> &val_struct,
                                     uint32_t data_type,
                                     uint32_t operation_type) {
  pfc_log_debug("Inside ReadInternal of KT_LINK");
  vector<key_link_t> vect_link_id;
  vector<val_link_st_t> vect_val_link_st;
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
  uint32_t option = 0;
  UpplReturnCode read_status = ReadLinkValFromDB(key_struct,
                                                 void_val_struct,
                                                 data_type,
                                                 operation_type,
                                                 max_rep_ct,
                                                 vect_val_link_st,
                                                 vect_link_id, option,
                                                 option);
  key_val.clear();
  val_struct.clear();
  if (read_status != UPPL_RC_SUCCESS) {
    pfc_log_error("Read operation has failed");
  } else {
    pfc_log_debug("Read operation is success");
    for (unsigned int iIndex = 0 ; iIndex < vect_val_link_st.size();
        ++iIndex) {
      key_link_t *key_link = new key_link_t(vect_link_id[iIndex]);
      val_link_st_t *val_link = new val_link_st_t(vect_val_link_st[iIndex]);
      key_val.push_back(reinterpret_cast<void *>(key_link));
      val_struct.push_back(reinterpret_cast<void *>(val_link));
    }
  }
  return read_status;
}

/**ReadBulk
 * * @Description : This function reads bulk rows of KT_Link in
 *  link table of specified data type.
 *  Order of ReadBulk response
 *  val_link -> val_boundary
 * * @param[in] :
 * key_struct - the key for the kt link instance
 * data_type - UNC_DT_* , read allowed in state
 * option1/option2 - specifies any additional condition for read operation
 * max_rep_ct - specifies number of rows to be returned
 * parent_call - indicates whether parent has called this readbulk
 * is_read_next - indicates whether this function is invoked from readnext
 * * @return    : UPPL_RC_SUCCESS is returned when the response
 * is added to ipc session successfully.
 * UPPL_RC_ERR_* is returned when ipc response could not be added to sess.
 * */
UpplReturnCode Kt_Link::ReadBulk(void* key_struct,
                                 uint32_t data_type,
                                 uint32_t option1,
                                 uint32_t option2,
                                 uint32_t &max_rep_ct,
                                 int child_index,
                                 pfc_bool_t parent_call,
                                 pfc_bool_t is_read_next) {
  UpplReturnCode read_status = UPPL_RC_SUCCESS;
  key_link_t* obj_key_link=
      reinterpret_cast<key_link_t*>(key_struct);
  string str_controller_name =
      reinterpret_cast<char *>(&obj_key_link->ctr_key.controller_name);
  string str_switch_id1 =
      reinterpret_cast<char *>(&obj_key_link->switch_id1);
  string str_port_id1 =
      reinterpret_cast<char *>(&obj_key_link->port_id1);
  string str_switch_id2 =
      reinterpret_cast<char *>(&obj_key_link->switch_id2);
  string str_port_id2 =
      reinterpret_cast<char *>(&obj_key_link->port_id2);
  if ((unc_keytype_datatype_t)data_type != UNC_DT_STATE) {
    // Not supported
    pfc_log_debug("ReadBulk operation is not allowed in %d data type",
                  data_type);
    read_status = UPPL_RC_ERR_OPERATION_NOT_ALLOWED;
    pfc_log_debug("read_status=%d", read_status);
    return read_status;
  }
  if (max_rep_ct == 0) {
    pfc_log_debug("max_rep_ct is 0");
    return UPPL_RC_SUCCESS;
  }
  vector<val_link_st_t> vect_val_link_st;
  vector<key_link_t> vect_link_id;
  // Read the link values based on given key structure
  read_status = ReadBulkInternal(key_struct,
                                 data_type,
                                 max_rep_ct,
                                 vect_val_link_st,
                                 vect_link_id);
  pfc_log_debug("read_status from kt_link is %d", read_status);
  if (read_status == UPPL_RC_SUCCESS) {
    PhysicalCore *physical_core = PhysicalLayer::get_instance()->
        get_physical_core();
    InternalTransactionCoordinator *itc_trans  =
        physical_core->get_internal_transaction_coordinator();
    // For each link , read the values
    vector<val_link_st_t> ::iterator vect_iter = vect_val_link_st.begin();
    vector<key_link_t> ::iterator link_iter = vect_link_id.begin();
    for (; link_iter != vect_link_id.end(); ++link_iter, ++vect_iter) {
      pfc_log_debug("Iterating link entries...");
      pfc_log_debug(
          "Adding link - switch_id1 '%s', switch_id2 '%s' "
          "port_id1 - '%s', port_id2 - '%s' to session",
          reinterpret_cast<char *>((*link_iter).switch_id1),
          reinterpret_cast<char *>((*link_iter).switch_id2),
          reinterpret_cast<char *>((*link_iter).port_id1),
          reinterpret_cast<char *>((*link_iter).port_id2));
      key_link_t *key_buffer = new key_link_t(*link_iter);
      BulkReadBuffer obj_key_buffer = {
          UNC_KT_LINK, IS_KEY,
          reinterpret_cast<void*>(key_buffer)
      };
      itc_trans->AddToBuffer(obj_key_buffer);
      val_link_st_t *val_buffer = new val_link_st_t(*vect_iter);
      BulkReadBuffer obj_value_buffer = {
          UNC_KT_LINK, IS_STATE_VALUE,
          reinterpret_cast<void*>(val_buffer)
      };
      itc_trans->AddToBuffer(obj_value_buffer);
      BulkReadBuffer obj_sep_buffer = {
          UNC_KT_LINK, IS_SEPARATOR, NULL
      };
      itc_trans->AddToBuffer(obj_sep_buffer);
      --max_rep_ct;
      if (max_rep_ct == 0) {
        return UPPL_RC_SUCCESS;
      }
    }
  }
  if (max_rep_ct >0 && parent_call == false) {
    // Filling key_struct corresponding to the key type
    pfc_log_debug("Calling parent Kt_Controller read bulk");
    Kt_Controller nextKin;
    key_ctr_t nextkin_key_struct;
    memcpy(nextkin_key_struct.controller_name,
           str_controller_name.c_str(),
           str_controller_name.length() +1);
    read_status = nextKin.ReadBulk(
        reinterpret_cast<void *>(&nextkin_key_struct),
        data_type,
        option1,
        option2,
        max_rep_ct,
        2,
        false,
        is_read_next);
    pfc_log_debug("read_status from next kin Kt_Port is %d", read_status);
    return UPPL_RC_SUCCESS;
  }
  pfc_log_debug("link reached end of table");
  pfc_log_debug("read_status=%d", read_status);
  if (read_status == UPPL_RC_ERR_NO_SUCH_INSTANCE) {
    read_status = UPPL_RC_SUCCESS;
  }
  return read_status;
}

/**ReadBulkInternal
 * * @Description : This function reads bulk rows of KT_Link in
 *  link table of specified data type.
 * * @param[in] :
 * key_struct - the key for the kt link instance
 * val_struct - the value struct for kt_link instance
 * max_rep_ct - specifies number of rows to be returned
 * vect_val_link - indicates the fetched values from db of val_link type
 * vect_link_id - indicates the fetched link names from db
 * * @return    : UPPL_RC_SUCCESS is returned when the response
 * is added to ipc session successfully.
 * UPPL_RC_ERR_* is returned when ipc response could not be added to sess.
 * */
UpplReturnCode Kt_Link::ReadBulkInternal(
    void* key_struct,
    uint32_t data_type,
    uint32_t max_rep_ct,
    vector<val_link_st_t> &vect_val_link_st,
    vector<key_link_t> &vect_link_id) {
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  void *val_struct = NULL;
  UpplReturnCode read_status = UPPL_RC_SUCCESS;
  ODBCM_RC_STATUS read_db_status = ODBCM_RC_SUCCESS;
  DBTableSchema kt_link_dbtableschema;
  vector<ODBCMOperator> operator_vector;
  // Populate DBSchema for link_table
  void* old_value;
  PopulateDBSchemaForKtTable(kt_link_dbtableschema,
                             key_struct,
                             val_struct,
                             UNC_OP_READ_BULK,
                             UNC_OPT1_NORMAL,
                             UNC_OPT2_NONE,
                             operator_vector,
                             old_value);
  uint32_t no_of_query = 1;
  vector<ODBCMOperator>:: iterator iter =
      find(operator_vector.begin(),
           operator_vector.end(),
           unc::uppl::MULTIPLE_QUERY);
  if (iter != operator_vector.end()) {
    // Multiple query to be sent to DB till we get a match
    pfc_log_debug("Multiple query to be sent to DB till we get a match");
    no_of_query = 4;  // Link has 4 primary keys other than controller
  }
  for (uint32_t index = 0; index < no_of_query; ++index) {
    // Read rows from DB
    read_db_status = physical_layer->get_odbc_manager()-> \
        GetBulkRows((unc_keytype_datatype_t)data_type, max_rep_ct,
                    kt_link_dbtableschema,
                    (unc_keytype_operation_t)UNC_OP_READ_BULK);
    pfc_log_debug("GetBulkRows return: %d", read_db_status);
    if (read_db_status == ODBCM_RC_RECORD_NOT_FOUND) {
      pfc_log_debug("No record found");
      read_status = UPPL_RC_ERR_NO_SUCH_INSTANCE;
      // Update the primary key vector
      kt_link_dbtableschema.primary_keys_.pop_back();
      pfc_log_debug(
          "Primary key vector size %d",
          static_cast<int>(kt_link_dbtableschema.primary_keys_.size()));
      // return read_status;
    } else if (read_db_status == ODBCM_RC_SUCCESS) {
      read_status = UPPL_RC_SUCCESS;
      pfc_log_debug("Received success response from db");
      break;
    } else {
      read_status = UPPL_RC_ERR_DB_GET;
    }
  }
  if (read_status == UPPL_RC_SUCCESS) {
    // From the values received from DB, populate val_link structure
    FillLinkValueStructure(kt_link_dbtableschema,
                           vect_val_link_st,
                           max_rep_ct,
                           UNC_OP_READ,
                           vect_link_id);
  }
  return read_status;
}

/** PerformSyntaxValidation
 * * @Description : This function performs syntax validation for
 *  UNC_KT_LINK
 * * * @param[in]
 * key_struct - the key for the kt link instance
 * value_struct - the value for the kt link instance
 * data_type - UNC_DT_*
 * operation_type - UNC_OP*
 * * @return    : UPPL_RC_SUCCESS is returned when the validation is successful
 * UPPL_RC_ERR_* is returned when validtion is failure
 * */
UpplReturnCode Kt_Link::PerformSyntaxValidation(void* key_struct,
                                                void* val_struct,
                                                uint32_t operation,
                                                uint32_t data_type) {
  pfc_log_info("Performing Syntax Validation of KT_LINK");
  UpplReturnCode ret_code = UPPL_RC_SUCCESS;
  pfc_ipcresp_t mandatory = PFC_TRUE;

  // Validate key structure
  key_link *key = reinterpret_cast<key_link_t*>(key_struct);
  string value = reinterpret_cast<char*>(key->ctr_key.controller_name);
  IS_VALID_STRING_KEY(CTR_NAME, value, operation, ret_code, mandatory);
  if (ret_code != UPPL_RC_SUCCESS) {
    return UPPL_RC_ERR_CFG_SYNTAX;
  }

  value = reinterpret_cast<char*>(key->switch_id1);
  IS_VALID_STRING_KEY(LINK_SWITCH_ID1, value, operation, ret_code, mandatory);
  if (ret_code != UPPL_RC_SUCCESS) {
    return UPPL_RC_ERR_CFG_SYNTAX;
  }

  value = reinterpret_cast<char*>(key->port_id1);
  IS_VALID_STRING_KEY(LINK_PORT_ID1, value, operation, ret_code, mandatory);
  if (ret_code != UPPL_RC_SUCCESS) {
    return UPPL_RC_ERR_CFG_SYNTAX;
  }

  value = reinterpret_cast<char*>(key->switch_id2);
  IS_VALID_STRING_KEY(LINK_SWITCH_ID2, value, operation, ret_code, mandatory);
  if (ret_code != UPPL_RC_SUCCESS) {
    return UPPL_RC_ERR_CFG_SYNTAX;
  }

  value = reinterpret_cast<char*>(key->port_id2);
  IS_VALID_STRING_KEY(LINK_PORT_ID2, value, operation, ret_code, mandatory);
  if (ret_code != UPPL_RC_SUCCESS) {
    return UPPL_RC_ERR_CFG_SYNTAX;
  }

  // Validate value structure
  if (val_struct != NULL) {
    unsigned int valid_val = 0;
    // validate description
    val_link *link_value = reinterpret_cast<val_link_t*>(val_struct);
    valid_val = PhyUtil::uint8touint(link_value->valid[kIdxLinkDescription]);
    string value = reinterpret_cast<char*>(link_value->description);
    IS_VALID_STRING_VALUE(LINK_DESCRIPTION, value, operation,
                          valid_val, ret_code, mandatory);
    if (ret_code != UPPL_RC_SUCCESS) {
      return UPPL_RC_ERR_CFG_SYNTAX;
    }
  }
  return ret_code;
}

/** PerformSemanticValidation
 * * @Description : This function performs semantic validation
 * for UNC_KT_LINK
 * * * @param[in] : key_struct - specifies key instance of KT_Link
 * , value_struct - specifies value of KT_LINK
 * operation - UNC_OP*
 * data_type - UNC_DT*
 * * * @return    : UPPL_RC_SUCCESS if semantic valition is successful
 * or UPPL_RC_ERR_* if failed
 * */
UpplReturnCode Kt_Link::PerformSemanticValidation(void* key_struct,
                                                  void* val_struct,
                                                  uint32_t operation,
                                                  uint32_t data_type) {
  UpplReturnCode status = UPPL_RC_SUCCESS;
  pfc_log_debug("Inside PerformSemanticValidation of KT_LINK");
  // Check whether the given instance of link exists in DB
  key_link_t *obj_key_link = reinterpret_cast<key_link_t*>(key_struct);
  string controller_name = (const char*)obj_key_link->ctr_key.controller_name;
  string switch_id1 = (const char*)obj_key_link->switch_id1;
  string port_id1 = (const char*)obj_key_link->port_id1;
  string switch_id2 = (const char*)obj_key_link->switch_id2;
  string port_id2 = (const char*)obj_key_link->port_id2;
  vector<string> link_vect_key_value;
  link_vect_key_value.push_back(controller_name);
  link_vect_key_value.push_back(switch_id1);
  link_vect_key_value.push_back(port_id1);
  link_vect_key_value.push_back(switch_id2);
  link_vect_key_value.push_back(port_id2);
  UpplReturnCode key_status = IsKeyExists((unc_keytype_datatype_t)data_type,
                                          link_vect_key_value);
  pfc_log_debug("IsKeyExists status %d", key_status);
  // In case of Create operation, key should not exist
  if (operation == UNC_OP_CREATE) {
    if (key_status == UPPL_RC_SUCCESS) {
      pfc_log_error("Key instance already exists");
      pfc_log_error("Hence Create operation not allowed");
      status = UPPL_RC_ERR_INSTANCE_EXISTS;
    } else if (key_status == UPPL_RC_ERR_DB_ACCESS) {
      pfc_log_error("DB Access failure");
      status = key_status;
    } else {
      pfc_log_info("key instance not exist Create operation allowed");
    }

  } else if (operation == UNC_OP_UPDATE || operation == UNC_OP_DELETE ||
      operation == UNC_OP_READ) {
    // In case of update/delete/read operation, key should exist
    if (key_status == UPPL_RC_ERR_DB_ACCESS) {
      pfc_log_error("DB Access failure");
      status = key_status;
    } else if (key_status != UPPL_RC_SUCCESS) {
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
    Kt_Controller KtObj;
    uint32_t parent_data_type = data_type;
    if (data_type == UNC_DT_IMPORT) {
      parent_data_type = UNC_DT_RUNNING;
    }
    UpplReturnCode parent_key_status = KtObj.IsKeyExists(
        (unc_keytype_datatype_t)parent_data_type, parent_vect_key_value);
    pfc_log_debug("Parent IsKeyExists status %d", parent_key_status);
    if (parent_key_status != UPPL_RC_SUCCESS) {
      status = UPPL_RC_ERR_PARENT_DOES_NOT_EXIST;
    }
  }
  pfc_log_debug("Return Code SemanticValidation: %d", status);
  return status;
}

/** HandleOperStatus
 * * @Description : This function performs the required actions when oper status
 * changes
 * * * @param[in] : Key and value struct
 * * * @return    : UPPL_RC_SUCCESS or UPPL_RC_ERR*
 * */
UpplReturnCode Kt_Link::HandleOperStatus(uint32_t data_type,
                                         void *key_struct,
                                         void *value_struct) {
  FN_START_TIME("HandleOperStatus", "Link");
  // PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  UpplReturnCode return_code = UPPL_RC_SUCCESS;

  if (key_struct != NULL) {
    key_link_t *obj_key_link =
        reinterpret_cast<key_link_t*>(key_struct);
    string controller_name = (const char*)obj_key_link->ctr_key.controller_name;
    string switch_id1 = (const char*)obj_key_link->switch_id1;
    string port_id1 = (const char*)obj_key_link->port_id1;
    string switch_id2 = (const char*)obj_key_link->switch_id2;
    string port_id2 = (const char*)obj_key_link->port_id2;
    // Get the controller's oper status and decide on the oper_status
    key_ctr_t ctr_key;
    memcpy(ctr_key.controller_name, controller_name.c_str(),
           controller_name.length()+1);
    uint8_t ctrl_oper_status = 0;
    UpplLinkOperStatus link_oper_status = UPPL_LINK_OPER_UNKNOWN;
    Kt_Controller controller;
    UpplReturnCode read_status = controller.GetOperStatus(
        data_type, reinterpret_cast<void*>(&ctr_key), ctrl_oper_status);
    if (read_status == UPPL_RC_SUCCESS) {
      pfc_log_info("Controller's oper_status %d", ctrl_oper_status);
      if (ctrl_oper_status ==
          (UpplControllerOperStatus) UPPL_CONTROLLER_OPER_UP) {
        pfc_log_info("Set Link oper status as up");
        link_oper_status = UPPL_LINK_OPER_UP;
      }
    } else {
      pfc_log_info("Controller's oper_status read returned failure");
    }
    // Update oper_status in link table
    return_code = SetOperStatus(data_type, key_struct,
                                link_oper_status, true);
    if (return_code != UPPL_RC_SUCCESS) {
      // log error
      pfc_log_error("oper_status update operation failed");
      FN_END_TIME("HandleOperStatus", "Link");
      return UPPL_RC_ERR_DB_ACCESS;
    }
  }
  FN_END_TIME("HandleOperStatus", "Link");
  return UPPL_RC_SUCCESS;
}

/** SetOperStatus
 *  * @Description : This function updates the oper_status value
 *  of the controller
 *  * @param[in] : key_struct
 *  * @return    : oper_status
 */
UpplReturnCode Kt_Link::SetOperStatus(uint32_t data_type,
                                      void* key_struct,
                                      UpplLinkOperStatus oper_status,
                                      bool is_single_key) {
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  key_link *obj_key_link=
      reinterpret_cast<key_link_t*>(key_struct);
  TableAttrSchema kt_link_table_attr_schema;
  vector<TableAttrSchema> vect_table_attr_schema;
  list < vector<TableAttrSchema> > row_list;
  vector<string> vect_prim_keys;
  string controller_name = (const char*)obj_key_link->
      ctr_key.controller_name;
  string switch_id1 = (const char*)obj_key_link->switch_id1;
  string port_id1 = (const char*)obj_key_link->port_id1;
  string switch_id2 = (const char*)obj_key_link->switch_id2;
  string port_id2 = (const char*)obj_key_link->port_id2;
  if (!controller_name.empty()) {
    vect_prim_keys.push_back(CTR_NAME);
  }

  PhyUtil::FillDbSchema(CTR_NAME, controller_name,
                        controller_name.length(), DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);
  if (is_single_key == false) {
    if (!switch_id1.empty()) {
      vect_prim_keys.push_back(LINK_SWITCH_ID1);
      PhyUtil::FillDbSchema(LINK_SWITCH_ID1, switch_id1,
                            switch_id1.length(), DATATYPE_UINT8_ARRAY_256,
                            vect_table_attr_schema);
    }
    if (!port_id1.empty()) {
      vect_prim_keys.push_back(LINK_PORT_ID1);
      PhyUtil::FillDbSchema(LINK_PORT_ID1, port_id1,
                            port_id1.length(), DATATYPE_UINT8_ARRAY_32,
                            vect_table_attr_schema);
    }
    if (!switch_id2.empty()) {
      vect_prim_keys.push_back(LINK_SWITCH_ID2);
      PhyUtil::FillDbSchema(LINK_SWITCH_ID2, switch_id2,
                            switch_id2.length(), DATATYPE_UINT8_ARRAY_256,
                            vect_table_attr_schema);
    }
    if (!port_id2.empty()) {
      vect_prim_keys.push_back(LINK_PORT_ID2);
      PhyUtil::FillDbSchema(LINK_PORT_ID2, port_id2,
                            port_id2.length(), DATATYPE_UINT8_ARRAY_32,
                            vect_table_attr_schema);
    }
  }

  string oper_value = PhyUtil::uint8tostr(oper_status);
  PhyUtil::FillDbSchema(LINK_OPER_STATUS, oper_value,
                        oper_value.length(), DATATYPE_UINT16,
                        vect_table_attr_schema);

  DBTableSchema kt_link_dbtableschema;
  kt_link_dbtableschema.set_table_name(UPPL_LINK_TABLE);
  kt_link_dbtableschema.set_primary_keys(vect_prim_keys);
  row_list.push_back(vect_table_attr_schema);
  kt_link_dbtableschema.set_row_list(row_list);

  // Call ODBCManager and update
  ODBCM_RC_STATUS update_db_status =
      physical_layer->get_odbc_manager()->UpdateOneRow(
          (unc_keytype_datatype_t)data_type,
          kt_link_dbtableschema);
  if (update_db_status != ODBCM_RC_SUCCESS) {
    // log error
    pfc_log_error("oper_status update operation failed");
    return UPPL_RC_ERR_DB_UPDATE;
  } else {
    // Notify operstatus change to northbound
    val_link_st old_val_link, new_val_link;
    uint8_t old_oper_status = 0;
    UpplReturnCode read_status = GetOperStatus(data_type,
                                               key_struct,
                                               old_oper_status);
    if (read_status == UPPL_RC_SUCCESS) {
      memset(old_val_link.valid, 0, 2);
      old_val_link.oper_status = old_oper_status;
      old_val_link.valid[kIdxLinkStOperStatus] = UNC_VF_VALID;
      memset(new_val_link.valid, 0, 2);
      new_val_link.oper_status = oper_status;
      new_val_link.valid[kIdxLinkStOperStatus] = UNC_VF_VALID;
      int err = 0;
      // Send notification to Northbound
      ServerEvent ser_evt((pfc_ipcevtype_t)UPPL_EVENTS_KT_LINK, err);
      ser_evt.addOutput((uint32_t)UNC_OP_UPDATE);
      ser_evt.addOutput(data_type);
      ser_evt.addOutput((uint32_t)UPPL_EVENTS_KT_LINK);
      ser_evt.addOutput(*obj_key_link);
      ser_evt.addOutput(new_val_link);
      ser_evt.addOutput(old_val_link);
      PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
      // Notify operstatus modifications
      UpplReturnCode status = (UpplReturnCode) physical_layer
          ->get_ipc_connection_manager()->SendEvent(&ser_evt);
      pfc_log_debug("Event notification status %d", status);
    }
  }
  return UPPL_RC_SUCCESS;
}

/** IsKeyExists
 * * @Description : This function checks whether the link_id exists in DB
 * * * @param[in] : data type - UNC_DT_*
 * key value - Contains link_id
 * * * @return    : UPPL_RC_SUCCESS or UPPL_RC_ERR* based on operation type
 * */
UpplReturnCode Kt_Link::IsKeyExists(unc_keytype_datatype_t data_type,
                                    vector<string> key_values) {
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  //  void* key_struct;
  //  key_link_t *obj_key_link = reinterpret_cast<key_link_t*>(key_struct);
  UpplReturnCode check_status = UPPL_RC_SUCCESS;
  if (key_values.empty()) {
    // No key given, return failure
    pfc_log_info("No key given. Returning error");
    return UPPL_RC_ERR_BAD_REQUEST;
  }

  string controller_name = key_values[0];
  string switch_id1 = key_values[1];
  string port_id1 = key_values[2];
  string switch_id2 = key_values[3];
  string port_id2 = key_values[4];
  // Structure used to send request to ODBC
  DBTableSchema kt_link_dbtableschema;
  // Construct Primary key list
  vector<string> vect_prim_keys;

  // construct TableAttrSchema structure
  // TableAttrSchema holds table_name, primary key, attr_name
  vector<TableAttrSchema> vect_table_attr_schema;
  TableAttrSchema kt_link_table_attr_schema;
  list< vector<TableAttrSchema> > row_list;

  // Controller_name
  PhyUtil::FillDbSchema(CTR_NAME, controller_name,
                        controller_name.length(), DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);
  if (!controller_name.empty()) {
    vect_prim_keys.push_back(CTR_NAME);
  }
  // switch_id1
  PhyUtil::FillDbSchema(LINK_SWITCH_ID1, switch_id1,
                        switch_id1.length(), DATATYPE_UINT8_ARRAY_256,
                        vect_table_attr_schema);
  if (!switch_id1.empty()) {
    vect_prim_keys.push_back(LINK_SWITCH_ID1);
  }
  // port_id1
  PhyUtil::FillDbSchema(LINK_PORT_ID1, port_id1,
                        port_id1.length(), DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);
  if (!port_id1.empty()) {
    vect_prim_keys.push_back(LINK_PORT_ID1);
  }
  // switch_id2
  PhyUtil::FillDbSchema(LINK_SWITCH_ID2, switch_id2,
                        switch_id2.length(), DATATYPE_UINT8_ARRAY_256,
                        vect_table_attr_schema);
  if (!switch_id2.empty()) {
    vect_prim_keys.push_back(LINK_SWITCH_ID2);
  }
  // port_id2
  PhyUtil::FillDbSchema(LINK_PORT_ID2, port_id2,
                        port_id2.length(), DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);
  if (!port_id2.empty()) {
    vect_prim_keys.push_back(LINK_PORT_ID2);
  }

  kt_link_dbtableschema.set_table_name(UPPL_LINK_TABLE);
  kt_link_dbtableschema.set_primary_keys(vect_prim_keys);
  row_list.push_back(vect_table_attr_schema);
  kt_link_dbtableschema.set_row_list(row_list);
  // Send request to ODBC for link_table
  ODBCM_RC_STATUS check_db_status = physical_layer->get_odbc_manager()->
      IsRowExists(data_type, kt_link_dbtableschema);
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
 * * @Description : This function populates the DBAttrSchema to be used to send
 *                  request to ODBC
 * * * @param[in] : DBTableSchema, key_struct, val_struct, operation_type
 * * * @return    : UPPL_RC_SUCCESS or UPPL_RC_ERR*
 * */
void Kt_Link::PopulateDBSchemaForKtTable(
    DBTableSchema &kt_link_dbtableschema,
    void *key_struct,
    void *val_struct,
    uint8_t operation_type,
    uint32_t option1,
    uint32_t option2,
    vector<ODBCMOperator> &vect_prim_keys_operation,
    void* &old_value_struct,
    CsRowStatus row_status,
    pfc_bool_t is_filtering,
    pfc_bool_t is_state) {
  // Construct Primary key list
  vector<string> vect_prim_keys;
  // Construct TableAttrSchema structuree
  // TableAttrSchema holds table_name, primary key, attr_name
  vector<TableAttrSchema> vect_table_attr_schema;
  list < vector<TableAttrSchema> > row_list;

  key_link_t *obj_key_link = reinterpret_cast<key_link_t*>(key_struct);
  val_link_st_t *obj_val_link = reinterpret_cast<val_link_st_t*>(val_struct);

  stringstream valid;
  pfc_log_info("operation: %d", operation_type);
  // Controller_name
  string controller_name = (const char*)obj_key_link->ctr_key.controller_name;
  pfc_log_info("controller name: %s", controller_name.c_str());
  PhyUtil::FillDbSchema(CTR_NAME, controller_name,
                        controller_name.length(), DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);
  vect_prim_keys.push_back(CTR_NAME);
  vect_prim_keys_operation.push_back(unc::uppl::EQUAL);

  // switch_id1
  string switch_id1 = (const char*)obj_key_link->switch_id1;
  if ((operation_type == UNC_OP_READ_SIBLING_BEGIN ||
      operation_type == UNC_OP_READ_SIBLING_COUNT) &&
      (option2 != UNC_OPT2_MATCH_SWITCH1 &&
          option2 != UNC_OPT2_MATCH_BOTH_SWITCH)) {
    // Ignore switch_id1 key value
    switch_id1 = "";
  }
  pfc_log_info("switch_id1: %s", switch_id1.c_str());
  PhyUtil::FillDbSchema(LINK_SWITCH_ID1, switch_id1,
                        switch_id1.length(), DATATYPE_UINT8_ARRAY_256,
                        vect_table_attr_schema);
  // switch_id2
  string switch_id2 = (const char*)obj_key_link->switch_id2;
  if ((operation_type == UNC_OP_READ_SIBLING_BEGIN ||
      operation_type == UNC_OP_READ_SIBLING_COUNT) &&
      (option2 != UNC_OPT2_MATCH_SWITCH2 &&
          option2 != UNC_OPT2_MATCH_BOTH_SWITCH)) {
    // Ignore switch_id2 key value
    switch_id2 = "";
  }
  pfc_log_info("switch_id2: %s", switch_id2.c_str());
  PhyUtil::FillDbSchema(LINK_SWITCH_ID2, switch_id2,
                        switch_id2.length(), DATATYPE_UINT8_ARRAY_256,
                        vect_table_attr_schema);

  // port_id1
  string port_id1 = (const char*)obj_key_link->port_id1;
  if (operation_type == UNC_OP_READ_SIBLING_BEGIN ||
      operation_type == UNC_OP_READ_SIBLING_COUNT) {
    // Ignore port_id1 key value
    port_id1 = "";
  }
  pfc_log_info("port_id1: %s", port_id1.c_str());
  PhyUtil::FillDbSchema(LINK_PORT_ID1, port_id1,
                        port_id1.length(), DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);

  // port_id2
  string port_id2 = (const char*)obj_key_link->port_id2;
  if (operation_type == UNC_OP_READ_SIBLING_BEGIN ||
      operation_type == UNC_OP_READ_SIBLING_COUNT) {
    // Ignore port_id2 key value
    port_id2 = "";
  }
  pfc_log_info("port_id2: %s", port_id2.c_str());
  PhyUtil::FillDbSchema(LINK_PORT_ID2, port_id2,
                        port_id2.length(), DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);
  PopulatePrimaryKeys(
      operation_type,
      option1,
      option2,
      switch_id1,
      switch_id2,
      port_id1,
      port_id2,
      vect_prim_keys,
      vect_prim_keys_operation);
  val_link_st_t *val_link_valid_st = NULL;
  if (operation_type == UNC_OP_UPDATE) {
    // get valid array for update req
    pfc_log_debug("Get link valid value from update valid flag");
    val_link_valid_st = new val_link_st_t();
    GetLinkValidFlag(key_struct, *val_link_valid_st);
    old_value_struct = reinterpret_cast<void *>(val_link_valid_st);
  }
  unsigned int valid_value_struct = UNC_VF_VALID;
  if (obj_val_link != NULL) {
    valid_value_struct = PhyUtil::uint8touint(
        obj_val_link->valid[kIdxLinkStLink]);
  }
  string value;
  uint32_t valid_val = 0, prev_db_val = 0;
  // Description
  if (obj_val_link != NULL &&
      valid_value_struct == UNC_VF_VALID) {
    valid_val = PhyUtil::uint8touint(
        obj_val_link->link.valid[kIdxLinkDescription]);
    value = (const char*)obj_val_link->link.description;
    if (operation_type == UNC_OP_UPDATE) {
      prev_db_val =
          PhyUtil::uint8touint(val_link_valid_st->link.
                               valid[kIdxLinkDescription]);
    }
  } else {
    valid_val = UPPL_NO_VAL_STRUCT;
  }
  PhyUtil::FillDbSchema(LINK_DESCRIPTION, value,
                        value.length(), DATATYPE_UINT8_ARRAY_128,
                        operation_type, valid_val, prev_db_val,
                        vect_table_attr_schema, vect_prim_keys, valid);
  value.clear();
  // oper_status
  if (obj_val_link != NULL) {
    valid_val = PhyUtil::uint8touint(obj_val_link->
                                     valid[kIdxLinkStOperStatus]);
    if (valid_val == UNC_VF_VALID) {
      value = PhyUtil::uint8tostr(obj_val_link->oper_status);
    }
    if (operation_type == UNC_OP_UPDATE) {
      prev_db_val =
          PhyUtil::uint8touint(val_link_valid_st->
                               valid[kIdxLinkStOperStatus]);
    }
  } else {
    valid_val = UPPL_NO_VAL_STRUCT;
  }
  PhyUtil::FillDbSchema(LINK_OPER_STATUS, value,
                        value.length(), DATATYPE_UINT16,
                        operation_type, valid_val, prev_db_val,
                        vect_table_attr_schema, vect_prim_keys, valid);
  valid_val = UPPL_NO_VAL_STRUCT;
  prev_db_val = 0;
  stringstream dummy_valid;
  // valid
  PhyUtil::FillDbSchema(LINK_VALID, valid.str(),
                        valid.str().length(), DATATYPE_UINT8_ARRAY_2,
                        operation_type, valid_val, prev_db_val,
                        vect_table_attr_schema, vect_prim_keys, dummy_valid);
  PhyUtil::reorder_col_attrs(vect_prim_keys, vect_table_attr_schema);
  kt_link_dbtableschema.set_table_name(UPPL_LINK_TABLE);
  kt_link_dbtableschema.set_primary_keys(vect_prim_keys);
  row_list.push_back(vect_table_attr_schema);
  kt_link_dbtableschema.set_row_list(row_list);
  return;
}

/** FillLinkValueStructure
 * * @Description : This function populates val_link_st_t by values retrieved
 * from database
 * * * @param[in] : controller common table dbtable schema,
 * value structure and max_rep_ct, operation type
 * * * @return    : Filled val_link and controller id
 * */
void Kt_Link::FillLinkValueStructure(
    DBTableSchema &kt_link_dbtableschema,
    vector<val_link_st_t> &vect_obj_val_link,
    uint32_t &max_rep_ct,
    uint32_t operation_type,
    vector<key_link_t> &link_id) {
  // populate IPC value structure based on the response recevied from DB
  list < vector<TableAttrSchema> > res_link_row_list =
      kt_link_dbtableschema.get_row_list();
  list < vector<TableAttrSchema> > :: iterator res_link_iter =
      res_link_row_list.begin();
  max_rep_ct = res_link_row_list.size();
  pfc_log_debug("res_link_row_list.size: %d", max_rep_ct);

  // populate IPC value structure based on the response recevied from DB
  for (; res_link_iter != res_link_row_list.end(); ++res_link_iter) {
    vector<TableAttrSchema> res_link_table_attr_schema =
        (*res_link_iter);
    vector<TableAttrSchema> :: iterator vect_link_iter =
        res_link_table_attr_schema.begin();
    uint32_t attr_size = res_link_table_attr_schema.size();
    pfc_log_debug("res_link_table_attr_schema size: %d", attr_size);
    val_link_st_t obj_val_link;
    memset(obj_val_link.valid, '\0', 2);
    key_link_t obj_key_link;
    // Read all attributes
    vector<int> valid_flag, cs_attr;
    for (; vect_link_iter != res_link_table_attr_schema.end();
        ++vect_link_iter) {
      // Populate values from link_table
      TableAttrSchema tab_schema = (*vect_link_iter);
      string attr_name = tab_schema.table_attribute_name;
      string attr_value;
      if (attr_name == CTR_NAME) {
        PhyUtil::GetValueFromDbSchema(tab_schema, attr_value,
                                      DATATYPE_UINT8_ARRAY_32);
        memcpy(obj_key_link.ctr_key.controller_name,
               attr_value.c_str(),
               attr_value.length()+1);
        pfc_log_debug("controller_name: %s", reinterpret_cast<char *>
        (&obj_key_link.ctr_key.controller_name));
      }
      if (attr_name == LINK_SWITCH_ID1) {
        PhyUtil::GetValueFromDbSchema(tab_schema, attr_value,
                                      DATATYPE_UINT8_ARRAY_256);
        memcpy(obj_key_link.switch_id1,
               attr_value.c_str(),
               attr_value.length()+1);
        pfc_log_debug("switch_id1: %s", reinterpret_cast<char *>
        (&obj_key_link.switch_id1));
      }
      if (attr_name == LINK_PORT_ID1) {
        PhyUtil::GetValueFromDbSchema(tab_schema, attr_value,
                                      DATATYPE_UINT8_ARRAY_32);
        memcpy(obj_key_link.port_id1,
               attr_value.c_str(),
               attr_value.length()+1);
        pfc_log_debug("port_id1: %s", reinterpret_cast<char *>
        (&obj_key_link.port_id1));
      }
      if (attr_name == LINK_SWITCH_ID2) {
        PhyUtil::GetValueFromDbSchema(tab_schema, attr_value,
                                      DATATYPE_UINT8_ARRAY_256);
        memcpy(obj_key_link.switch_id2,
               attr_value.c_str(),
               attr_value.length()+1);
        pfc_log_debug("switch_id2: %s", reinterpret_cast<char *>
        (&obj_key_link.switch_id2));
      }
      if (attr_name == LINK_PORT_ID2) {
        PhyUtil::GetValueFromDbSchema(tab_schema, attr_value,
                                      DATATYPE_UINT8_ARRAY_32);
        memcpy(obj_key_link.port_id2,
               attr_value.c_str(),
               attr_value.length()+1);
        pfc_log_debug("port_id2: %s", reinterpret_cast<char *>
        (&obj_key_link.port_id2));
      }
      if (attr_name == LINK_DESCRIPTION) {
        PhyUtil::GetValueFromDbSchema(tab_schema, attr_value,
                                      DATATYPE_UINT8_ARRAY_128);
        memcpy(obj_val_link.link.description,
               attr_value.c_str(),
               attr_value.length()+1);
        pfc_log_debug("description: %s", obj_val_link.link.description);
      }
      if (attr_name == LINK_OPER_STATUS) {
        PhyUtil::GetValueFromDbSchema(tab_schema, attr_value,
                                      DATATYPE_UINT16);
        obj_val_link.oper_status = atoi(attr_value.c_str());
        pfc_log_debug("oper status : %d", obj_val_link.oper_status);
      }
      if (attr_name == LINK_VALID) {
        PhyUtil::GetValueFromDbSchema(tab_schema, attr_value,
                                      DATATYPE_UINT8_ARRAY_2);
        memset(obj_val_link.link.valid, 0, 1);
        FrameValidValue(attr_value, obj_val_link);
        pfc_log_debug("valid: %s", attr_value.c_str());
      }
    }
    vect_obj_val_link.push_back(obj_val_link);
    // populate key structure
    link_id.push_back(obj_key_link);
    pfc_log_debug("result - vect_obj_val_link size: %d",
                  (unsigned int) vect_obj_val_link.size());
  }
  return;
}

/** PerformRead
 * * @Description : This function reads the instance of KT_Link based on
 *                  operation type - READ, READ_SIBLING_BEGIN, READ_SIBLING
 * * * @param[in] : ipc session id, configuration id, key_struct, value_struct,
 *                  data_type, operation type, ServerSession, option1, option2,
 *                  max_rep_ct
 * * * @return    : UPPL_RC_SUCCESS or UPPL_RC_ERR*
 * */
UpplReturnCode Kt_Link::PerformRead(uint32_t session_id,
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

  key_link_t *obj_key_link = reinterpret_cast<key_link_t*>(key_struct);
  if (option1 != UNC_OPT1_NORMAL) {
    pfc_log_error("PerformRead provided on unsupported option1");
    rsh.result_code = UPPL_RC_ERR_INVALID_OPTION1;
    int err = PhyUtil::sessOutRespHeader(sess, rsh);
    err |= sess.addOutput((uint32_t)UNC_KT_LINK);
    err |= sess.addOutput(*obj_key_link);
    if (err != 0) {
      pfc_log_debug("addOutput failed for physical_response_header");
      return UPPL_RC_ERR_IPC_WRITE_ERROR;
    }
    return UPPL_RC_SUCCESS;
  }


  UpplReturnCode read_status = UPPL_RC_SUCCESS;
  // Read operations will return switch_st
  if ((unc_keytype_datatype_t)data_type == UNC_DT_STATE &&
      option1 == UNC_OPT1_NORMAL) {
    if (option2 != UNC_OPT2_MATCH_SWITCH1 &&
        option2 != UNC_OPT2_MATCH_SWITCH2 &&
        option2 != UNC_OPT2_MATCH_BOTH_SWITCH &&
        option2 != UNC_OPT2_NONE) {
      pfc_log_error("PerformRead provided on unsupported option2");
      rsh.result_code = UPPL_RC_ERR_INVALID_OPTION2;
      int err = PhyUtil::sessOutRespHeader(sess, rsh);
      err |= sess.addOutput((uint32_t)UNC_KT_LINK);
      err |= sess.addOutput(*obj_key_link);
      if (err != 0) {
        pfc_log_debug("addOutput failed for physical_response_header");
        return UPPL_RC_ERR_IPC_WRITE_ERROR;
      }
      return UPPL_RC_SUCCESS;
    }
    vector<key_link_t> vect_link_id;
    vector<val_link_t> vect_val_link;
    vector<val_link_st_t> vect_val_link_st;
    read_status = ReadLinkValFromDB(key_struct,
                                    val_struct,
                                    data_type,
                                    operation_type,
                                    max_rep_ct,
                                    vect_val_link_st,
                                    vect_link_id,
                                    option1,
                                    option2);
    rsh.result_code = read_status;
    rsh.max_rep_count = max_rep_ct;
    int err = PhyUtil::sessOutRespHeader(sess, rsh);
    if (err != 0) {
      pfc_log_error("Failure in addOutput");
      return UPPL_RC_ERR_IPC_WRITE_ERROR;
    }
    if (read_status != UPPL_RC_SUCCESS) {
      pfc_log_error("Read operation failed with %d", read_status);
      sess.addOutput((uint32_t)UNC_KT_LINK);
      sess.addOutput(*obj_key_link);
      return UPPL_RC_SUCCESS;
    }
    pfc_log_debug("From db, vect_link_id size is %d",
                  static_cast<int>(vect_link_id.size()));
    for (unsigned int index = 0;
        index < vect_link_id.size();
        ++index) {
      sess.addOutput((uint32_t)UNC_KT_LINK);
      sess.addOutput((key_link_t)vect_link_id[index]);
      sess.addOutput((val_link_st_t)vect_val_link_st[index]);
      if (index < vect_link_id.size() -1) {
        sess.addOutput();  // separator
      }
    }

  } else {
    // Invalid data type
    pfc_log_error("Read operation is provided on unsupported data type");
    rsh.result_code = UPPL_RC_ERR_OPERATION_NOT_ALLOWED;
    int err = PhyUtil::sessOutRespHeader(sess, rsh);
    err |= sess.addOutput((uint32_t)UNC_KT_LINK);
    if (err != 0) {
      pfc_log_debug("addOutput failed for physical_response_header");
      return UPPL_RC_ERR_IPC_WRITE_ERROR;
    }
  }
  return UPPL_RC_SUCCESS;
}

/** ReadLinkValFromDB
 * * @Description : This function reads the instance of KT_Link based on
 *                  operation type - READ, READ_SIBLING_BEGIN, READ_SIBLING
 *                   from data base
 * * * @param[in] : key_struct, value_struct, ipc session id, configuration id,
 *                  data_type, operation type, max_rep_ct
 * * * @return    : Success or associated error code
 * */
UpplReturnCode Kt_Link::ReadLinkValFromDB(
    void* key_struct,
    void* val_struct,
    uint32_t data_type,
    uint32_t operation_type,
    uint32_t &max_rep_ct,
    vector<val_link_st_t> &vect_val_link_st,
    vector<key_link_t> &link_id,
    uint32_t option1,
    uint32_t option2,
    pfc_bool_t is_state) {
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  UpplReturnCode read_status = UPPL_RC_SUCCESS;
  ODBCM_RC_STATUS read_db_status = ODBCM_RC_SUCCESS;

  // Construct TableAttrSchema structure
  // TableAttrSchema holds table_name, primary key, attr_name
  DBTableSchema kt_link_dbtableschema;
  vector<ODBCMOperator> vect_prim_key_operations;
  void* old_value;
  PopulateDBSchemaForKtTable(kt_link_dbtableschema,
                             key_struct,
                             val_struct,
                             operation_type,
                             option1, option2,
                             vect_prim_key_operations,
                             old_value);

  if (operation_type == UNC_OP_READ) {
    read_db_status = physical_layer->get_odbc_manager()->
        GetOneRow((unc_keytype_datatype_t)data_type,
                  kt_link_dbtableschema);
  } else if ((operation_type == UNC_OP_READ_SIBLING_BEGIN ||
      operation_type == UNC_OP_READ_SIBLING) &&
      (option1 == UNC_OPT1_NORMAL &&
          (option2 == UNC_OPT2_MATCH_SWITCH1 ||
              option2 == UNC_OPT2_MATCH_SWITCH2 ||
              option2 == UNC_OPT2_MATCH_BOTH_SWITCH))) {
    pfc_log_debug("calling get sibling rows with filtering");
    read_db_status = physical_layer->get_odbc_manager()->
        GetSiblingRows((unc_keytype_datatype_t)data_type, max_rep_ct,
                       kt_link_dbtableschema,
                       vect_prim_key_operations,
                       (unc_keytype_operation_t)operation_type);
  } else {
    read_db_status = physical_layer->get_odbc_manager()->
        GetBulkRows((unc_keytype_datatype_t)data_type, max_rep_ct,
                    kt_link_dbtableschema,
                    (unc_keytype_operation_t)operation_type);
  }
  if (read_db_status == ODBCM_RC_RECORD_NOT_FOUND) {
    pfc_log_debug("No record found");
    read_status = UPPL_RC_ERR_NO_SUCH_INSTANCE;
    return read_status;
  } else if (read_db_status != ODBCM_RC_SUCCESS) {
    read_status = UPPL_RC_ERR_DB_GET;
    // log error to log daemon
    pfc_log_error("Read operation has failed with error %d", read_db_status);
    return read_status;
  }
  pfc_log_debug("Read operation result: %d", read_status);
  FillLinkValueStructure(kt_link_dbtableschema,
                         vect_val_link_st,
                         max_rep_ct,
                         operation_type,
                         link_id);
  pfc_log_debug("vect_val_link_st size: %d",
                (unsigned int)vect_val_link_st.size());
  pfc_log_debug("link_id size: %d", (unsigned int)link_id.size());
  if (vect_val_link_st.empty()) {
    // Read failed , return error
    read_status = UPPL_RC_ERR_DB_GET;
    // log error to log daemon
    pfc_log_error("Read operation has failed after reading response");
    return read_status;
  }
  return read_status;
}

/** Fill_Attr_Syntax_Map
 * * @Description : This function populates the values to be used for attribute
 * validation
 * * * @param[in] : None
 * * * @return    : None
 * */
void Kt_Link::Fill_Attr_Syntax_Map() {
  Kt_Class_Attr_Syntax objKeyAttrSyntax =
  { PFC_IPCTYPE_UINT8, 0, 0, 1, 32, true,  "" };
  attr_syntax_map[CTR_NAME] = objKeyAttrSyntax;

  Kt_Class_Attr_Syntax objKeyAttrSyntax1 =
  { PFC_IPCTYPE_STRING, 0, 0, 1, 256, true,  "" };
  attr_syntax_map[LINK_SWITCH_ID1] = objKeyAttrSyntax1;

  Kt_Class_Attr_Syntax objKeyAttrSyntax2 =
  { PFC_IPCTYPE_STRING, 0, 0, 1, 32, true,  "" };
  attr_syntax_map[LINK_PORT_ID1] = objKeyAttrSyntax2;

  Kt_Class_Attr_Syntax objKeyAttrSyntax3 =
  { PFC_IPCTYPE_STRING, 0, 0, 1, 256, true,  "" };
  attr_syntax_map[LINK_SWITCH_ID2] = objKeyAttrSyntax3;

  Kt_Class_Attr_Syntax objKeyAttrSyntax4 =
  { PFC_IPCTYPE_STRING, 0, 0, 1, 32, true,  "" };
  attr_syntax_map[LINK_PORT_ID2] = objKeyAttrSyntax4;

  Kt_Class_Attr_Syntax objAttrDescSyntax =
  { PFC_IPCTYPE_STRING, 0, 0, 0, 128, false,  "" };
  attr_syntax_map[LINK_DESCRIPTION] = objAttrDescSyntax;

  Kt_Class_Attr_Syntax objAttrValidSyntax =
  { PFC_IPCTYPE_STRING, 0, 0, 0, 2, false, "" };
  attr_syntax_map[LINK_VALID] = objAttrValidSyntax;
}

/** GetLinkValidFlag
 * * @Description : This function reads the valid flag from DB
 * * * @param[in] : Key, value struct and newvalid val
 * * * @return    : Success or associated error code
 * */
UpplReturnCode Kt_Link::GetLinkValidFlag(
    void *key_struct,
    val_link_st_t &val_link_valid_st) {
  UpplReturnCode return_code = UPPL_RC_SUCCESS;
  vector<void *> vectVal_link;
  vector<void *> vectkey_link;
  vectkey_link.push_back(key_struct);

  return_code = ReadInternal(vectkey_link,
                             vectVal_link,
                             UNC_DT_STATE, UNC_OP_READ);
  if (return_code == UPPL_RC_SUCCESS) {
    val_link_st_t *obj_new_link_val_vect =
        reinterpret_cast<val_link_st_t*> (vectVal_link[0]);
    if (obj_new_link_val_vect != NULL) {
      val_link_valid_st = *obj_new_link_val_vect;

      delete obj_new_link_val_vect;
      obj_new_link_val_vect = NULL;
      key_link_t *link_key = reinterpret_cast<key_link_t*>
      (vectkey_link[0]);
      if (link_key != NULL) {
        delete link_key;
        link_key = NULL;
      }
    } else {
      pfc_log_info("update link valid ret null val");
    }
  } else {
    pfc_log_info("read internal failure from ctr updatevalid");
  }
  return return_code;
}

/** GetOperStatus
 *  * @Description : This function reads the oper_status value of the link
 *  * @param[in] : key_struct
 *  * @return    : oper_status
 */
UpplReturnCode Kt_Link::GetOperStatus(uint32_t data_type,
                                      void* key_struct,
                                      uint8_t &oper_status) {
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  key_link *obj_key_link =
      reinterpret_cast<key_link_t*>(key_struct);
  vector<TableAttrSchema> vect_table_attr_schema;
  list < vector<TableAttrSchema> > row_list;
  vector<string> vect_prim_keys;

  string controller_name = (const char*)obj_key_link->ctr_key.controller_name;
  string switch_id1 = (const char*)obj_key_link->switch_id1;
  string port_id1 = (const char*)obj_key_link->port_id1;
  string switch_id2 = (const char*)obj_key_link->switch_id2;
  string port_id2 = (const char*)obj_key_link->port_id2;
  // Controller_name
  PhyUtil::FillDbSchema(CTR_NAME, controller_name,
                        controller_name.length(), DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);
  if (!controller_name.empty()) {
    vect_prim_keys.push_back(CTR_NAME);
  }
  // switch_id1
  PhyUtil::FillDbSchema(LINK_SWITCH_ID1, switch_id1,
                        switch_id1.length(), DATATYPE_UINT8_ARRAY_256,
                        vect_table_attr_schema);
  if (!switch_id1.empty()) {
    vect_prim_keys.push_back(LINK_SWITCH_ID1);
  }
  // port_id1
  PhyUtil::FillDbSchema(LINK_PORT_ID1, port_id1,
                        port_id1.length(), DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);
  if (!port_id1.empty()) {
    vect_prim_keys.push_back(LINK_PORT_ID1);
  }
  // switch_id2
  PhyUtil::FillDbSchema(LINK_SWITCH_ID2, switch_id2,
                        switch_id2.length(), DATATYPE_UINT8_ARRAY_256,
                        vect_table_attr_schema);
  if (!switch_id2.empty()) {
    vect_prim_keys.push_back(LINK_SWITCH_ID2);
  }
  // port_id2
  PhyUtil::FillDbSchema(LINK_PORT_ID2, port_id2,
                        port_id2.length(), DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);
  if (!port_id2.empty()) {
    vect_prim_keys.push_back(LINK_PORT_ID2);
  }

  string oper_value;
  PhyUtil::FillDbSchema(LINK_OPER_STATUS, oper_value,
                        oper_value.length(), DATATYPE_UINT16,
                        vect_table_attr_schema);
  DBTableSchema kt_link_dbtableschema;
  PhyUtil::reorder_col_attrs(vect_prim_keys, vect_table_attr_schema);
  kt_link_dbtableschema.set_table_name(UPPL_LINK_TABLE);
  kt_link_dbtableschema.set_primary_keys(vect_prim_keys);
  row_list.push_back(vect_table_attr_schema);
  kt_link_dbtableschema.set_row_list(row_list);

  // Call ODBCManager and update
  ODBCM_RC_STATUS update_db_status =
      physical_layer->get_odbc_manager()->GetOneRow(
          (unc_keytype_datatype_t)data_type,
          kt_link_dbtableschema);
  if (update_db_status != ODBCM_RC_SUCCESS) {
    // log error
    pfc_log_info("oper_status read operation failed");
    return UPPL_RC_ERR_DB_GET;
  }

  // read the oper_status value
  list < vector<TableAttrSchema> > res_row_list =
      kt_link_dbtableschema.get_row_list();
  list < vector<TableAttrSchema> >::iterator res_iter =
      res_row_list.begin();
  // populate IPC value structure based on the response received from DB
  for (; res_iter!= res_row_list.end(); ++res_iter) {
    vector<TableAttrSchema> res_table_attr_schema = (*res_iter);
    vector<TableAttrSchema>:: iterator vect_iter =
        res_table_attr_schema.begin();
    for (; vect_iter != res_table_attr_schema.end();
        ++vect_iter) {
      // populate values from port_table
      TableAttrSchema tab_schema = (*vect_iter);
      string attr_name = tab_schema.table_attribute_name;
      string attr_value;
      if (attr_name == LINK_OPER_STATUS) {
        PhyUtil::GetValueFromDbSchema(tab_schema, attr_value,
                                      DATATYPE_UINT16);
        oper_status = atoi(attr_value.c_str());
        pfc_log_debug("oper_status: %d", oper_status);
        break;
      }
    }
  }
  return UPPL_RC_SUCCESS;
}

/** FrameValidValue
 * * @Description : This function converts the string value from db to uint8
 * * * @param[in] : Attribute value and val_ctr_st
 * * * @return    : Success or associated error code
 * */
void Kt_Link::FrameValidValue(string attr_value,
                              val_link_st &obj_val_link) {
  if (!attr_value.empty()) {
    if (attr_value[0] < 48) {
      obj_val_link.link.valid[kIdxLinkDescription] = attr_value[0];
    } else {
      obj_val_link.link.valid[kIdxLinkDescription] = attr_value[0] - 48;
    }
    if (attr_value[1] < 48) {
      obj_val_link.valid[kIdxLinkStOperStatus] = attr_value[1];
    } else {
      obj_val_link.valid[kIdxLinkStOperStatus] = attr_value[1] - 48;
    }
  }
  obj_val_link.valid[kIdxLinkStLink] = UNC_VF_VALID;
  return;
}

/** PopulatePrimaryKeys
 * * @Description : This function decides the primary key based on option
 * * * @param[in] : Attribute value and val_ctr_st
 * * * @return    : Success or associated error code
 * */
void Kt_Link::PopulatePrimaryKeys(
    uint32_t operation_type,
    uint32_t option1,
    uint32_t option2,
    string switch_id1,
    string switch_id2,
    string port_id1,
    string port_id2,
    vector<string> &vect_prim_keys,
    vector<ODBCMOperator> &vect_prim_keys_operation) {
  if (option1 == UNC_OPT1_NORMAL && option2 == UNC_OPT2_MATCH_SWITCH1) {
    vect_prim_keys.push_back(LINK_SWITCH_ID1);
    vect_prim_keys_operation.push_back(unc::uppl::EQUAL);
  }
  if (option1 == UNC_OPT1_NORMAL && option2 == UNC_OPT2_MATCH_SWITCH2) {
    vect_prim_keys.push_back(LINK_SWITCH_ID2);
    vect_prim_keys_operation.push_back(unc::uppl::EQUAL);
  }
  if (option1 == UNC_OPT1_NORMAL &&
      option2 == UNC_OPT2_MATCH_BOTH_SWITCH) {
    vect_prim_keys.push_back(LINK_SWITCH_ID1);
    vect_prim_keys_operation.push_back(unc::uppl::EQUAL);
    vect_prim_keys.push_back(LINK_SWITCH_ID2);
    vect_prim_keys_operation.push_back(unc::uppl::EQUAL);
  }
  if (option1 == UNC_OPT1_NORMAL && (option2 != UNC_OPT2_MATCH_SWITCH1 &&
      option2 != UNC_OPT2_MATCH_SWITCH2 &&
      option2 != UNC_OPT2_MATCH_BOTH_SWITCH)) {
    vect_prim_keys_operation.clear();
    if ((operation_type == UNC_OP_READ_NEXT ||
        operation_type == UNC_OP_READ_BULK) &&
        !switch_id1.empty() && !switch_id2.empty() &&
        !port_id1.empty() && !port_id2.empty()) {
      // Adding MULTIPLE_QUERY to primary key vector
      pfc_log_debug("Adding MULTIPLE_QUERY to primary key vector");
      vect_prim_keys_operation.push_back(unc::uppl::MULTIPLE_QUERY);
    }
    // Need not check empty condition
    // Another key required for getbulk operation
    vect_prim_keys.push_back(LINK_SWITCH_ID1);
    if (!switch_id2.empty()) {
      vect_prim_keys.push_back(LINK_SWITCH_ID2);
    }
  }
  if (!port_id1.empty()) {
    vect_prim_keys.push_back(LINK_PORT_ID1);
    if (option2 == UNC_OPT2_MATCH_SWITCH1 ||
        option2 == UNC_OPT2_MATCH_SWITCH2 ||
        option2 == UNC_OPT2_MATCH_BOTH_SWITCH) {
      vect_prim_keys_operation.push_back(unc::uppl::GREATER);
    }
  }
  if (!port_id2.empty()) {
    vect_prim_keys.push_back(LINK_PORT_ID2);
    if (option2 == UNC_OPT2_MATCH_SWITCH1 ||
        option2 == UNC_OPT2_MATCH_SWITCH2 ||
        option2 == UNC_OPT2_MATCH_BOTH_SWITCH) {
      vect_prim_keys_operation.push_back(unc::uppl::GREATER);
    }
  }
}
