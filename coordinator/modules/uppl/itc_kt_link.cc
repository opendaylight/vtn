/*
 * Copyright (c) 2012-2015 NEC Corporation
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
#include "itc_read_request.hh"
#include "odbcm_db_varbind.hh"
using unc::uppl::PhysicalLayer;

/** Constructor
 * @Description : This function instantiates attr map
 * kt_link
 * @param[in] : None
 * @return    : None
 * */
Kt_Link::Kt_Link() {
  // Populate structure to be used for syntax validation
  if (attr_syntax_map_all.find(UNC_KT_LINK) ==
      attr_syntax_map_all.end()) {
    Fill_Attr_Syntax_Map();
  }
}

/** Destructor
 * @Description : Empty Destructor
 * instances for kt_link
 * @param[in] : None
 * @return    : None
 * */
Kt_Link::~Kt_Link() {
}

/**DeleteKeyInstance
 * @Description : This function deletes a row of KT_Link in
 * state link table.
 * @param[in] :
 * key_struct - the key for the new kt link instance
 * data_type - UNC_DT_* , delete only allowed in state
 * key_type - UNC_KT_LINK,value of unc_key_type_t
 * @return    : UNC_RC_SUCCESS is returned when the delete
 * is done successfully.
 * UNC_UPPL_RC_ERR_* is returned when the delete is error
 * */
UncRespCode Kt_Link::DeleteKeyInstance(OdbcmConnectionHandler *db_conn,
                                          void* key_struct,
                                          uint32_t data_type,
                                          uint32_t key_type) {
  UncRespCode delete_status = UNC_RC_SUCCESS;
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  // Check operation is allowed on the given DT, skip the check if flag is true
  if ((unc_keytype_datatype_t)data_type != UNC_DT_STATE &&
      (unc_keytype_datatype_t)data_type != UNC_DT_IMPORT) {
    pfc_log_error("Delete operation is provided on unsupported data type");
    return UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED;
  }
  // Structure used to send request to ODBC
  DBTableSchema kt_link_dbtableschema;
  vector<ODBCMOperator> operator_vector;
  // Populate DBSchema for link_table
  void* old_value;
  PopulateDBSchemaForKtTable(db_conn, kt_link_dbtableschema,
                             key_struct,
                             NULL,
                             UNC_OP_DELETE, data_type,
                             UNC_OPT1_NORMAL,
                             UNC_OPT2_NONE,
                             operator_vector,
                             old_value, NOTAPPLIED,
                             false, PFC_FALSE);
  ODBCM_RC_STATUS delete_db_status = physical_layer->get_odbc_manager()-> \
      DeleteOneRow((unc_keytype_datatype_t)data_type,
                   kt_link_dbtableschema, db_conn);
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
    pfc_log_info("Delete of a link in data_type(%d) is success",
                 data_type);
  }
  return delete_status;
}

/** ReadInternal
 * @Description : This function reads the given  instance of KT_Link
 * @param[in] : 
 * key_struct - the key for the kt link instance
 * value_struct - the value for the kt link instance
 * data_type - UNC_DT_* , read allowed in state
 * operation_type-UNC_OP_*,type of operation
 * @return    : UNC_RC_SUCCESS is returned when the response
 * is added to ipc session successfully.
 * UNC_UPPL_RC_ERR_* is returned when ipc response could not be added to sess.
 * */
UncRespCode Kt_Link::ReadInternal(OdbcmConnectionHandler *db_conn,
                                     vector<void *> &key_val,
                                     vector<void *> &val_struct,
                                     uint32_t data_type,
                                     uint32_t operation_type) {
  if (operation_type != UNC_OP_READ && operation_type != UNC_OP_READ_SIBLING &&
      operation_type != UNC_OP_READ_SIBLING_BEGIN) {
    pfc_log_trace("This function not allowed for read next/bulk/count");
    return UNC_UPPL_RC_ERR_OPERATION_NOT_SUPPORTED;
  }
  pfc_log_debug("Inside ReadInternal of KT_LINK");
  uint32_t max_rep_ct = 1;
  if (operation_type != UNC_OP_READ) {
    max_rep_ct = UPPL_MAX_REP_CT;
  }
  // Get read response from database
  val_link_st_t obj_link_val;
  memset(&obj_link_val, '\0', sizeof(val_link_st_t));
  void *key_struct = key_val[0];
  void *void_val_struct = NULL;
  if ((!val_struct.empty()) && (val_struct[0] != NULL)) {
    memcpy(&obj_link_val, (reinterpret_cast <val_link_st_t*>
                                      (val_struct[0])),
           sizeof(val_link_st_t));
    void_val_struct = reinterpret_cast<void *>(&obj_link_val);
  }
  UncRespCode read_status = UNC_RC_SUCCESS;
  bool firsttime = true;
  uint32_t option = 0;
  do {
    vector<key_link_t> vect_link_id;
    vector<val_link_st_t> vect_val_link_st;
    read_status = ReadLinkValFromDB(db_conn, key_struct,
                                     void_val_struct,
                                     data_type,
                                     operation_type,
                                     max_rep_ct,
                                     vect_val_link_st,
                                     vect_link_id, option,
                                     option);
    if (firsttime) {
      pfc_log_trace(
          "Clearing key_val and val_struct vectors for the firsttime");
      key_val.clear();
      val_struct.clear();
      firsttime = false;
    }
    if (read_status == UNC_RC_SUCCESS) {
      pfc_log_debug("Read operation is success");
      for (unsigned int iIndex = 0 ; iIndex < vect_val_link_st.size();
          ++iIndex) {
        key_link_t *key_link = new key_link_t(vect_link_id[iIndex]);
        val_link_st_t *val_link = new val_link_st_t(vect_val_link_st[iIndex]);
        key_val.push_back(reinterpret_cast<void *>(key_link));
        val_struct.push_back(reinterpret_cast<void *>(val_link));
      }
    } else if ((read_status == UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE &&
               val_struct.size() != 0)) {
      read_status = UNC_RC_SUCCESS;
    }
    if ((vect_val_link_st.size() == UPPL_MAX_REP_CT) &&
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

/**ReadBulk
 * @Description : This function reads bulk rows of KT_Link in
 *  link table of specified data type.
 *  Order of ReadBulk response
 *  val_link -> val_boundary
 * @param[in] :
 * key_struct - the key for the kt link instance
 * data_type - UNC_DT_* , read allowed in state
 * max_rep_ct - specifies number of rows to be returned
 * parent_call - indicates whether parent has called this readbulk
 * is_read_next - indicates whether this function is invoked from readnext
 * @return    : UNC_RC_SUCCESS is returned when the response
 * is added to ipc session successfully.
 * UNC_UPPL_RC_ERR_* is returned when ipc response could not be added to sess.
 * */
UncRespCode Kt_Link::ReadBulk(OdbcmConnectionHandler *db_conn,
                                 void* key_struct,
                                 uint32_t data_type,
                                 uint32_t &max_rep_ct,
                                 int child_index,
                                 pfc_bool_t parent_call,
                                 pfc_bool_t is_read_next,
                                 ReadRequest *read_req) {
  UncRespCode read_status = UNC_RC_SUCCESS;
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
    read_status = UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED;
    pfc_log_debug("read_status=%d", read_status);
    return read_status;
  }
  if (max_rep_ct == 0) {
    pfc_log_debug("max_rep_ct is 0");
    return UNC_RC_SUCCESS;
  }
  vector<val_link_st_t> vect_val_link_st;
  vector<key_link_t> vect_link_id;
  // Read the link values based on given key structure
  read_status = ReadBulkInternal(db_conn, key_struct,
                                 data_type,
                                 max_rep_ct,
                                 vect_val_link_st,
                                 vect_link_id);
  pfc_log_debug("read_status from kt_link is %d", read_status);
  if (read_status == UNC_RC_SUCCESS) {
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
      read_req->AddToBuffer(obj_key_buffer);
      val_link_st_t *val_buffer = new val_link_st_t(*vect_iter);
      BulkReadBuffer obj_value_buffer = {
          UNC_KT_LINK, IS_STATE_VALUE,
          reinterpret_cast<void*>(val_buffer)
      };
      read_req->AddToBuffer(obj_value_buffer);
      BulkReadBuffer obj_sep_buffer = {
          UNC_KT_LINK, IS_SEPARATOR, NULL
      };
      read_req->AddToBuffer(obj_sep_buffer);
      --max_rep_ct;
      if (max_rep_ct == 0) {
        return UNC_RC_SUCCESS;
      }
    }
  } else if (read_status == UNC_UPPL_RC_ERR_DB_ACCESS) {
    pfc_log_debug("Ktlink ReadBulk - Returning DB Access Error");
    return read_status;
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
        db_conn, reinterpret_cast<void *>(&nextkin_key_struct),
        data_type,
        max_rep_ct,
        2,
        false,
        is_read_next,
        read_req);
    pfc_log_debug("read_status from next kin Kt_Port is %d", read_status);
    return UNC_RC_SUCCESS;
  }
  pfc_log_debug("link reached end of table");
  pfc_log_debug("read_status=%d", read_status);
  if (read_status == UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE) {
    read_status = UNC_RC_SUCCESS;
  }
  return read_status;
}

/**ReadBulkInternal
 * @Description : This function reads bulk rows of KT_Link in
 *  link table of specified data type.
 * @param[in] :
 * key_struct - the key for the kt link instance
 * data_type-UNC_DT_*,type of database
 * max_rep_ct- specifies number of rows to be returned
 * vect_val_link - indicates the fetched values from db of val_link type
 * vect_link_id - indicates the fetched link names from db
 * @return    : UNC_RC_SUCCESS is returned when the response
 * is added to ipc session successfully.
 * UNC_UPPL_RC_ERR_* is returned when ipc response could not be added to sess.
 * */
UncRespCode Kt_Link::ReadBulkInternal(
    OdbcmConnectionHandler *db_conn,
    void* key_struct,
    uint32_t data_type,
    uint32_t max_rep_ct,
    vector<val_link_st_t> &vect_val_link_st,
    vector<key_link_t> &vect_link_id) {
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  void *val_struct = NULL;
  UncRespCode read_status = UNC_RC_SUCCESS;
  ODBCM_RC_STATUS read_db_status = ODBCM_RC_SUCCESS;
  DBTableSchema kt_link_dbtableschema;
  vector<ODBCMOperator> operator_vector;
  // Populate DBSchema for link_table
  void* old_value;
  PopulateDBSchemaForKtTable(db_conn, kt_link_dbtableschema,
                             key_struct,
                             val_struct,
                             UNC_OP_READ_BULK, data_type,
                             UNC_OPT1_NORMAL,
                             UNC_OPT2_NONE,
                             operator_vector,
                             old_value, NOTAPPLIED,
                             false, PFC_FALSE);
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
  uint32_t index = 0;
  for (; index < no_of_query ; ++index) {
    if (kt_link_dbtableschema.primary_keys_.size() <2) {
      pfc_log_debug("No record found");
      read_status = UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE;
      break;
    }
    // Read rows from DB
    read_db_status = physical_layer->get_odbc_manager()-> \
        GetBulkRows((unc_keytype_datatype_t)data_type, max_rep_ct,
                    kt_link_dbtableschema,
                    (unc_keytype_operation_t)UNC_OP_READ_BULK, db_conn);
    pfc_log_debug("GetBulkRows return: %d", read_db_status);
    if (read_db_status == ODBCM_RC_RECORD_NOT_FOUND) {
      pfc_log_debug("No record found");
      read_status = UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE;
      // Update the primary key vector
      kt_link_dbtableschema.primary_keys_.pop_back();
      pfc_log_debug(
          "Primary key vector size %"
           PFC_PFMT_SIZE_T, kt_link_dbtableschema.primary_keys_.size());
    } else if (read_db_status == ODBCM_RC_CONNECTION_ERROR) {
      read_status = UNC_UPPL_RC_ERR_DB_ACCESS;
      pfc_log_error("Read operation has failed with error %d", read_db_status);
      break;
    } else if (read_db_status == ODBCM_RC_SUCCESS) {
      read_status = UNC_RC_SUCCESS;
      pfc_log_debug("Received success response from db");
      uint32_t max_rep_ct_new = 0;
        FillLinkValueStructure(db_conn, kt_link_dbtableschema,
                       vect_val_link_st,
                       max_rep_ct_new,
                       UNC_OP_READ,
                       vect_link_id);
        pfc_log_debug("max_rep_ct_new=%d max_rep_ct=%d",
                      max_rep_ct_new, max_rep_ct);
       for (uint32_t uindex = 1; uindex < max_rep_ct_new; uindex++) {
          pfc_log_debug("Ktlink:Row list Removed");
          kt_link_dbtableschema.DeleteRowListFrontElement();
        }
        max_rep_ct -= max_rep_ct_new;

      kt_link_dbtableschema.primary_keys_.pop_back();
    } else {
      read_status = UNC_UPPL_RC_ERR_DB_GET;
      break;
    }
  }  // for loop end
  if (vect_val_link_st.empty() && index == 4) {
      pfc_log_debug("No record found");
      read_status = UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE;
    } else if (!vect_val_link_st.empty()) {
      read_status = UNC_RC_SUCCESS;
    }

  return read_status;
}

/** PerformSyntaxValidation
 * @Description : This function performs syntax validation for
 *  UNC_KT_LINK
 * @param[in]
 * key_struct - the key for the kt link instance
 * value_struct - the value for the kt link instance
 * data_type - UNC_DT_*,type of database
 * operation_type - UNC_OP*,type of operation
 * @return    : UNC_RC_SUCCESS is returned when the validation is successful
 * UNC_UPPL_RC_ERR_* is returned when validation is failure
 * */
UncRespCode Kt_Link::PerformSyntaxValidation(OdbcmConnectionHandler *db_conn,
                                                void* key_struct,
                                                void* val_struct,
                                                uint32_t operation,
                                                uint32_t data_type) {
  UncRespCode ret_code = UNC_RC_SUCCESS;
  pfc_ipcresp_t mandatory = PFC_TRUE;

  // Validate key structure
  key_link *key = reinterpret_cast<key_link_t*>(key_struct);
  string value = reinterpret_cast<char*>(key->ctr_key.controller_name);
  map<string, Kt_Class_Attr_Syntax> attr_syntax_map =
      attr_syntax_map_all[UNC_KT_LINK];
  IS_VALID_STRING_KEY(CTR_NAME_STR, value, operation, ret_code, mandatory);
  if (ret_code != UNC_RC_SUCCESS) {
    return UNC_UPPL_RC_ERR_CFG_SYNTAX;
  }

  value = reinterpret_cast<char*>(key->switch_id1);
  IS_VALID_STRING_KEY(LINK_SWITCH_ID1_STR, value, operation,
                      ret_code, mandatory);
  if (ret_code != UNC_RC_SUCCESS) {
    return UNC_UPPL_RC_ERR_CFG_SYNTAX;
  }

  value = reinterpret_cast<char*>(key->port_id1);
  IS_VALID_STRING_KEY(LINK_PORT_ID1_STR, value, operation, ret_code, mandatory);
  if (ret_code != UNC_RC_SUCCESS) {
    return UNC_UPPL_RC_ERR_CFG_SYNTAX;
  }

  value = reinterpret_cast<char*>(key->switch_id2);
  IS_VALID_STRING_KEY(LINK_SWITCH_ID2_STR, value, operation,
                      ret_code, mandatory);
  if (ret_code != UNC_RC_SUCCESS) {
    return UNC_UPPL_RC_ERR_CFG_SYNTAX;
  }

  value = reinterpret_cast<char*>(key->port_id2);
  IS_VALID_STRING_KEY(LINK_PORT_ID2_STR, value, operation, ret_code, mandatory);
  if (ret_code != UNC_RC_SUCCESS) {
    return UNC_UPPL_RC_ERR_CFG_SYNTAX;
  }

  // Validate value structure
  if (val_struct != NULL) {
    unsigned int valid_val = 0;
    // validate description
    val_link *link_value = reinterpret_cast<val_link_t*>(val_struct);
    valid_val = PhyUtil::uint8touint(link_value->valid[kIdxLinkDescription]);
    string value = reinterpret_cast<char*>(link_value->description);
    IS_VALID_STRING_VALUE(LINK_DESCRIPTION_STR, value, operation,
                          valid_val, ret_code, mandatory);
    if (ret_code != UNC_RC_SUCCESS) {
      return UNC_UPPL_RC_ERR_CFG_SYNTAX;
    }
  }
  return ret_code;
}

/** PerformSemanticValidation
 * @Description : This function performs semantic validation
 * for UNC_KT_LINK
 * @param[in] : key_struct - specifies key instance of KT_Link
 * value_struct - specifies value of KT_LINK
 * operation - UNC_OP*,type of operation
 * data_type - UNC_DT*,type of database
 * @return    : UNC_RC_SUCCESS if semantic valition is successful
 * or UNC_UPPL_RC_ERR_* if failed
 * */
UncRespCode Kt_Link::PerformSemanticValidation(
    OdbcmConnectionHandler *db_conn,
    void* key_struct,
    void* val_struct,
    uint32_t operation,
    uint32_t data_type) {
  UncRespCode status = UNC_RC_SUCCESS;
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
  UncRespCode key_status = IsKeyExists(db_conn,
                                          (unc_keytype_datatype_t)data_type,
                                          link_vect_key_value);
  pfc_log_debug("IsKeyExists status %d", key_status);
  // In case of Create operation, key should not exist
  if (operation == UNC_OP_CREATE) {
    if (key_status == UNC_RC_SUCCESS) {
      pfc_log_error("Key exists,CREATE not allowed");
      status = UNC_UPPL_RC_ERR_INSTANCE_EXISTS;
    } else if (key_status == UNC_UPPL_RC_ERR_DB_ACCESS) {
      pfc_log_error("DB Access failure");
      status = key_status;
    } else {
      pfc_log_debug("key not exist, create allowed");
    }

  } else if (operation == UNC_OP_UPDATE || operation == UNC_OP_DELETE ||
      operation == UNC_OP_READ) {
    // In case of update/delete/read operation, key should exist
    if (key_status == UNC_UPPL_RC_ERR_DB_ACCESS) {
      pfc_log_error("DB Access failure");
      status = key_status;
    } else if (key_status != UNC_RC_SUCCESS) {
      pfc_log_error("Key not found,U/D/R opern not allowed");
      status = UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE;
    } else {
      pfc_log_debug("key exist, update/del/read oper allowed");
    }
  }

  if (operation == UNC_OP_CREATE && status == UNC_RC_SUCCESS) {
    vector<string> parent_vect_key_value;
    parent_vect_key_value.push_back(controller_name);
    Kt_Controller KtObj;
    uint32_t parent_data_type = data_type;
    if (data_type == UNC_DT_IMPORT) {
      parent_data_type = UNC_DT_RUNNING;
    }
    UncRespCode parent_key_status = KtObj.IsKeyExists(
        db_conn, (unc_keytype_datatype_t)parent_data_type,
        parent_vect_key_value);
    pfc_log_debug("Parent IsKeyExists status %d", parent_key_status);
    if (parent_key_status != UNC_RC_SUCCESS) {
      status = UNC_UPPL_RC_ERR_PARENT_DOES_NOT_EXIST;
    }
  }
  pfc_log_debug("Return Code SemanticValidation: %d", status);
  return status;
}

/** HandleOperStatus
 * @Description : This function performs the required actions when oper status
 * changes
 * @param[in] : data_type-UNC_DT_*,type of database
 * key_struct-void* to kt key structure
 * value_struct-void* to kt value structure
 * @return    : UNC_RC_SUCCESS or UNC_UPPL_RC_ERR*, 
 * UNC_RC_SUCCESS is returned when the response
 * is added to ipc session successfully.
 * UNC_UPPL_RC_ERR_* is returned when ipc response could not be added to sess.
 * */
UncRespCode Kt_Link::HandleOperStatus(OdbcmConnectionHandler *db_conn,
                                         uint32_t data_type,
                                         void *key_struct,
                                         void *value_struct) {
  if (key_struct != NULL) {
    key_link_t *obj_key_link =
        reinterpret_cast<key_link_t*>(key_struct);
    string controller_name = (const char*)obj_key_link->ctr_key.controller_name;
    // Get the controller's oper status and decide on the oper_status
    key_ctr_t ctr_key;
    memcpy(ctr_key.controller_name, controller_name.c_str(),
           controller_name.length()+1);
    uint8_t ctrl_oper_status = 0;
    UpplLinkOperStatus link_oper_status = UPPL_LINK_OPER_UNKNOWN;
    Kt_Controller controller;
    UncRespCode read_status = controller.GetOperStatus(
        db_conn, data_type, reinterpret_cast<void*>(&ctr_key),
        ctrl_oper_status);
    if (read_status == UNC_RC_SUCCESS) {
      pfc_log_debug("Ctr oper_status %d", ctrl_oper_status);
      if (ctrl_oper_status ==
          (UpplControllerOperStatus) UPPL_CONTROLLER_OPER_UP) {
        pfc_log_debug("Set Link oper status as up");
        link_oper_status = UPPL_LINK_OPER_UP;
      }
    } else {
      pfc_log_info("Controller's oper_status read returned failure");
    }
    // Update oper_status in link table
    read_status = SetOperStatus(db_conn, data_type, key_struct,
                                link_oper_status);
    if (read_status != UNC_RC_SUCCESS &&
        read_status != UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE) {
      // log error
      pfc_log_error("oper_status update operation failed");
      return UNC_UPPL_RC_ERR_DB_ACCESS;
    }
  }
  return UNC_RC_SUCCESS;
}

/** SetOperStatus
 * @Description : This function updates the oper_status value
 *  of the controller
 * @param[in] : key_struct-void* to link key strcuture
 * data_type-UNC_DT_*,type of database
 * oper_status-any value of UpplLinkOperStatus
 * @return    : UNC_RC_SUCCESS/ERR*, UNC_RC_SUCCESS is returned when the
 * response is added to ipc session successfully.
 * UNC_UPPL_RC_ERR_* is returned when ipc response could not be added to sess.
 */
UncRespCode Kt_Link::SetOperStatus(OdbcmConnectionHandler *db_conn,
                                      uint32_t data_type,
                                      void* key_struct,
                                      UpplLinkOperStatus oper_status) {
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  key_link *obj_key_link=
      reinterpret_cast<key_link_t*>(key_struct);
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
    vect_prim_keys.push_back(CTR_NAME_STR);
  }

  PhyUtil::FillDbSchema(unc::uppl::CTR_NAME, controller_name,
                        controller_name.length(), DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);
  if (!switch_id1.empty()) {
    vect_prim_keys.push_back(LINK_SWITCH_ID1_STR);
    PhyUtil::FillDbSchema(unc::uppl::LINK_SWITCH_ID1, switch_id1,
                          switch_id1.length(), DATATYPE_UINT8_ARRAY_256,
                          vect_table_attr_schema);
  }
  if (!port_id1.empty()) {
    vect_prim_keys.push_back(LINK_PORT_ID1_STR);
    PhyUtil::FillDbSchema(unc::uppl::LINK_PORT_ID1, port_id1,
                          port_id1.length(), DATATYPE_UINT8_ARRAY_32,
                          vect_table_attr_schema);
  }
  if (!switch_id2.empty()) {
    vect_prim_keys.push_back(LINK_SWITCH_ID2_STR);
    PhyUtil::FillDbSchema(unc::uppl::LINK_SWITCH_ID2, switch_id2,
                          switch_id2.length(), DATATYPE_UINT8_ARRAY_256,
                          vect_table_attr_schema);
  }
  if (!port_id2.empty()) {
    vect_prim_keys.push_back(LINK_PORT_ID2_STR);
    PhyUtil::FillDbSchema(unc::uppl::LINK_PORT_ID2, port_id2,
                          port_id2.length(), DATATYPE_UINT8_ARRAY_32,
                          vect_table_attr_schema);
  }

  string oper_value = PhyUtil::uint8tostr(oper_status);
  PhyUtil::FillDbSchema(unc::uppl::LINK_OPER_STATUS, oper_value,
                        oper_value.length(), DATATYPE_UINT16,
                        vect_table_attr_schema);

  DBTableSchema kt_link_dbtableschema;
  kt_link_dbtableschema.set_table_name(unc::uppl::LINK_TABLE);
  kt_link_dbtableschema.set_primary_keys(vect_prim_keys);
  row_list.push_back(vect_table_attr_schema);
  kt_link_dbtableschema.set_row_list(row_list);

  // Call ODBCManager and update
  ODBCM_RC_STATUS update_db_status =
      physical_layer->get_odbc_manager()->UpdateOneRow(
          (unc_keytype_datatype_t)data_type,
          kt_link_dbtableschema, db_conn, true);
  if (update_db_status == ODBCM_RC_ROW_NOT_EXISTS) {
    pfc_log_info("No instance available for update");
    return UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE;
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
      val_link_st old_val_link, new_val_link;
      memset(&old_val_link, 0, sizeof(old_val_link));
      memset(&new_val_link, 0, sizeof(new_val_link));
      old_val_link.oper_status = old_oper_status;
      old_val_link.valid[kIdxLinkStOperStatus] = UNC_VF_VALID;
      new_val_link.oper_status = oper_status;
      new_val_link.valid[kIdxLinkStOperStatus] = UNC_VF_VALID;
      int err = 0;
      // Send notification to Northbound
      ServerEvent ser_evt((pfc_ipcevtype_t)UPPL_EVENTS_KT_LINK, err);
      northbound_event_header rsh = {static_cast<uint32_t>(UNC_OP_UPDATE),
          data_type,
          static_cast<uint32_t>(UNC_KT_LINK)};
      err = PhyUtil::sessOutNBEventHeader(ser_evt, rsh);
      err |= ser_evt.addOutput(*obj_key_link);
      err |= ser_evt.addOutput(new_val_link);
      err |= ser_evt.addOutput(old_val_link);
      if (err == 0) {
        PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
        // Notify operstatus modifications
        UncRespCode status = (UncRespCode) physical_layer
            ->get_ipc_connection_manager()->SendEvent(&ser_evt,
                     controller_name, UPPL_EVENTS_KT_LINK);
        pfc_log_debug("Event notification status %d", status);
      } else {
        pfc_log_error("Server Event addOutput failed");
      }
    }
  }
  return UNC_RC_SUCCESS;
}

/** IsKeyExists
 * @Description : This function checks whether the link_id exists in DB
 * @param[in] : data type - UNC_DT_*,type of database
 * key value - Contains link_id
 * @return    : UNC_RC_SUCCESS or UNC_UPPL_RC_ERR* based on operation type
 * */
UncRespCode Kt_Link::IsKeyExists(OdbcmConnectionHandler *db_conn,
                                    unc_keytype_datatype_t data_type,
                                    const vector<string> &key_values) {
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  //  void* key_struct;
  //  key_link_t *obj_key_link = reinterpret_cast<key_link_t*>(key_struct);
  UncRespCode check_status = UNC_RC_SUCCESS;
  if (key_values.empty()) {
    // No key given, return failure
    pfc_log_info("No key given. Returning error");
    return UNC_UPPL_RC_ERR_BAD_REQUEST;
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
  list< vector<TableAttrSchema> > row_list;

  // Controller_name
  PhyUtil::FillDbSchema(unc::uppl::CTR_NAME, controller_name,
                        controller_name.length(), DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);
  if (!controller_name.empty()) {
    vect_prim_keys.push_back(CTR_NAME_STR);
  }
  // switch_id1
  PhyUtil::FillDbSchema(unc::uppl::LINK_SWITCH_ID1, switch_id1,
                        switch_id1.length(), DATATYPE_UINT8_ARRAY_256,
                        vect_table_attr_schema);
  if (!switch_id1.empty()) {
    vect_prim_keys.push_back(LINK_SWITCH_ID1_STR);
  }
  // port_id1
  PhyUtil::FillDbSchema(unc::uppl::LINK_PORT_ID1, port_id1,
                        port_id1.length(), DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);
  if (!port_id1.empty()) {
    vect_prim_keys.push_back(LINK_PORT_ID1_STR);
  }
  // switch_id2
  PhyUtil::FillDbSchema(unc::uppl::LINK_SWITCH_ID2, switch_id2,
                        switch_id2.length(), DATATYPE_UINT8_ARRAY_256,
                        vect_table_attr_schema);
  if (!switch_id2.empty()) {
    vect_prim_keys.push_back(LINK_SWITCH_ID2_STR);
  }
  // port_id2
  PhyUtil::FillDbSchema(unc::uppl::LINK_PORT_ID2, port_id2,
                        port_id2.length(), DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);
  if (!port_id2.empty()) {
    vect_prim_keys.push_back(LINK_PORT_ID2_STR);
  }

  kt_link_dbtableschema.set_table_name(unc::uppl::LINK_TABLE);
  kt_link_dbtableschema.set_primary_keys(vect_prim_keys);
  row_list.push_back(vect_table_attr_schema);
  kt_link_dbtableschema.set_row_list(row_list);
  // Send request to ODBC for link_table
  ODBCM_RC_STATUS check_db_status = physical_layer->get_odbc_manager()->
      IsRowExists(data_type, kt_link_dbtableschema, db_conn);
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
 * @Description : This function populates the DBAttrSchema to be used to send
 *                  request to ODBC
 * @param[in] : 
 * kt_link_dbtableschema-DBTableSchema instance to be filled
 * key_struct-void* top link's key structure
 * val_struct-void* to link's value structure
 * operation_type-UNC_OP_*,type of operation
 * option1,option2-UNC_OPT1/OPT2_*, additional info for read operations
 * vect_prim_keys_operation- value of vector<ODBCMOperator>
 * old_value_struct-void* to link's value structure
 * row_status-CsRowStatus value
 * is_filtering-flag to indicate whether filter option is enabled
 * is_state-flag to indicate whether data type is DT_STATE
 * @return    : UNC_RC_SUCCESS or UNC_UPPL_RC_ERR*
 * */
void Kt_Link::PopulateDBSchemaForKtTable(
    OdbcmConnectionHandler *db_conn,
    DBTableSchema &kt_link_dbtableschema,
    void *key_struct,
    void *val_struct,
    uint8_t operation_type,
    uint32_t data_type,
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
  // Controller_name
  string controller_name = (const char*)obj_key_link->ctr_key.controller_name;
  PhyUtil::FillDbSchema(unc::uppl::CTR_NAME, controller_name,
                        controller_name.length(), DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);
  vect_prim_keys.push_back(CTR_NAME_STR);
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
  PhyUtil::FillDbSchema(unc::uppl::LINK_SWITCH_ID1, switch_id1,
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
  PhyUtil::FillDbSchema(unc::uppl::LINK_SWITCH_ID2, switch_id2,
                        switch_id2.length(), DATATYPE_UINT8_ARRAY_256,
                        vect_table_attr_schema);

  // port_id1
  string port_id1 = (const char*)obj_key_link->port_id1;
  if (operation_type == UNC_OP_READ_SIBLING_BEGIN ||
      operation_type == UNC_OP_READ_SIBLING_COUNT) {
    // Ignore port_id1 key value
    port_id1 = "";
  }
  PhyUtil::FillDbSchema(unc::uppl::LINK_PORT_ID1, port_id1,
                        port_id1.length(), DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);

  // port_id2
  string port_id2 = (const char*)obj_key_link->port_id2;
  if (operation_type == UNC_OP_READ_SIBLING_BEGIN ||
      operation_type == UNC_OP_READ_SIBLING_COUNT) {
    // Ignore port_id2 key value
    port_id2 = "";
  }
  PhyUtil::FillDbSchema(unc::uppl::LINK_PORT_ID2, port_id2,
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
    GetLinkValidFlag(db_conn, key_struct, *val_link_valid_st, data_type);
    old_value_struct = reinterpret_cast<void *>(val_link_valid_st);
  }
  unsigned int valid_value_struct = UNC_VF_VALID;
  if (obj_val_link != NULL) {
    valid_value_struct = PhyUtil::uint8touint(
        obj_val_link->valid[kIdxLinkStLink]);
  }
  string value = "";
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
  PhyUtil::FillDbSchema(unc::uppl::LINK_DESCRIPTION, LINK_DESCRIPTION_STR,
                        value, value.length(), DATATYPE_UINT8_ARRAY_128,
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
  PhyUtil::FillDbSchema(unc::uppl::LINK_OPER_STATUS, LINK_OPER_STATUS_STR,
                        value, value.length(), DATATYPE_UINT16,
                        operation_type, valid_val, prev_db_val,
                        vect_table_attr_schema, vect_prim_keys, valid);
  valid_val = UPPL_NO_VAL_STRUCT;
  prev_db_val = 0;
  stringstream dummy_valid;
  // valid
  PhyUtil::FillDbSchema(unc::uppl::LINK_VALID, LINK_VALID_STR, valid.str(),
                        valid.str().length(), DATATYPE_UINT8_ARRAY_2,
                        operation_type, valid_val, prev_db_val,
                        vect_table_attr_schema, vect_prim_keys, dummy_valid);
  PhyUtil::reorder_col_attrs(vect_prim_keys, vect_table_attr_schema);
  kt_link_dbtableschema.set_table_name(unc::uppl::LINK_TABLE);
  kt_link_dbtableschema.set_primary_keys(vect_prim_keys);
  row_list.push_back(vect_table_attr_schema);
  kt_link_dbtableschema.set_row_list(row_list);
  return;
}

/** FillLinkValueStructure
 * @Description : This function populates val_link_st_t by values retrieved
 * from database
 * @param[in] : kt_link_dbtableschema-DBTableSchema instance to be filled
 * vect_obj_val_link -vector of link value structure 
 * max_rep_ct-max. no. of records to be read
 * operation_type-UNC_OP_*,type of operation
 * link_id-instance of vector<key_link_t>
 * @return    : Filled val_link and controller id
 * */
void Kt_Link::FillLinkValueStructure(
    OdbcmConnectionHandler *db_conn,
    DBTableSchema &kt_link_dbtableschema,
    vector<val_link_st_t> &vect_obj_val_link,
    uint32_t &max_rep_ct,
    uint32_t operation_type,
    vector<key_link_t> &link_id) {
  // populate IPC value structure based on the response received from DB
  list < vector<TableAttrSchema> >& res_link_row_list =
      kt_link_dbtableschema.get_row_list();
  list < vector<TableAttrSchema> > :: iterator res_link_iter =
      res_link_row_list.begin();
  max_rep_ct = res_link_row_list.size();
  pfc_log_debug("res_link_row_list.size: %d", max_rep_ct);

  // populate IPC value structure based on the response received from DB
  for (; res_link_iter != res_link_row_list.end(); ++res_link_iter) {
    vector<TableAttrSchema> res_link_table_attr_schema =
        (*res_link_iter);
    vector<TableAttrSchema> :: iterator vect_link_iter =
        res_link_table_attr_schema.begin();
    uint32_t attr_size = res_link_table_attr_schema.size();
    pfc_log_debug("res_link_table_attr_schema size: %d", attr_size);
    val_link_st_t obj_val_link;
    memset(&obj_val_link, '\0', sizeof(val_link_st_t));
    memset(obj_val_link.valid, '\0', 2);
    key_link_t obj_key_link;
    memset(&obj_key_link, '\0', sizeof(key_link_t));
    // Read all attributes
    vector<int> valid_flag, cs_attr;
    for (; vect_link_iter != res_link_table_attr_schema.end();
        ++vect_link_iter) {
      // Populate values from link_table
      TableAttrSchema tab_schema = (*vect_link_iter);
      ODBCMTableColumns attr_name = tab_schema.table_attribute_name;
      string attr_value = "";
      switch (attr_name) {
        case unc::uppl::CTR_NAME:
          PhyUtil::GetValueFromDbSchemaStr(tab_schema,
                                           obj_key_link.ctr_key.controller_name,
                                           DATATYPE_UINT8_ARRAY_32);
          pfc_log_debug("controller_name: %s", reinterpret_cast<char *>
          (&obj_key_link.ctr_key.controller_name));
          break;

        case unc::uppl::LINK_SWITCH_ID1:
          PhyUtil::GetValueFromDbSchemaStr(tab_schema,
                                           obj_key_link.switch_id1,
                                           DATATYPE_UINT8_ARRAY_256);
          pfc_log_debug("switch_id1: %s", reinterpret_cast<char *>
          (&obj_key_link.switch_id1));
          break;

        case unc::uppl::LINK_PORT_ID1:
          PhyUtil::GetValueFromDbSchemaStr(tab_schema,
                                           obj_key_link.port_id1,
                                           DATATYPE_UINT8_ARRAY_32);
          pfc_log_debug("port_id1: %s", reinterpret_cast<char *>
          (&obj_key_link.port_id1));
          break;

        case unc::uppl::LINK_SWITCH_ID2:
          PhyUtil::GetValueFromDbSchemaStr(tab_schema,
                                           obj_key_link.switch_id2,
                                           DATATYPE_UINT8_ARRAY_256);
          pfc_log_debug("switch_id2: %s", reinterpret_cast<char *>
          (&obj_key_link.switch_id2));
          break;

        case unc::uppl::LINK_PORT_ID2:
          PhyUtil::GetValueFromDbSchemaStr(tab_schema,
                                           obj_key_link.port_id2,
                                           DATATYPE_UINT8_ARRAY_32);
          pfc_log_debug("port_id2: %s", reinterpret_cast<char *>
          (&obj_key_link.port_id2));
          break;

        case unc::uppl::LINK_DESCRIPTION:
          PhyUtil::GetValueFromDbSchemaStr(tab_schema,
                                           obj_val_link.link.description,
                                           DATATYPE_UINT8_ARRAY_128);
          pfc_log_debug("description: %s", obj_val_link.link.description);
          break;

        case unc::uppl::LINK_OPER_STATUS:
          PhyUtil::GetValueFromDbSchema(tab_schema, attr_value,
                                        DATATYPE_UINT16);
          obj_val_link.oper_status = atoi(attr_value.c_str());
          pfc_log_debug("oper status : %d", obj_val_link.oper_status);
          break;

        case unc::uppl::LINK_VALID:
          uint8_t link_val[ODBCM_SIZE_2];
          memset(&link_val, '\0', ODBCM_SIZE_2);
          PhyUtil::GetValueFromDbSchemaStr(tab_schema, link_val,
                                           DATATYPE_UINT8_ARRAY_2);
          memset(obj_val_link.link.valid, 0, 1);
          FrameValidValue(reinterpret_cast<const char*>(link_val),
                          obj_val_link);
          pfc_log_debug("valid: %s", link_val);
          break;

        default:
          pfc_log_info("Ignoring Link attribute %d", attr_name);
          break;
      }
    }
    vect_obj_val_link.push_back(obj_val_link);
    // populate key structure
    link_id.push_back(obj_key_link);
    pfc_log_debug("result - vect_obj_val_link size: %"
                   PFC_PFMT_SIZE_T, vect_obj_val_link.size());
  }
  return;
}

/** PerformRead
 * @Description : This function reads the instance of KT_Link based on
 *                  operation type - READ, READ_SIBLING_BEGIN, READ_SIBLING
 * @param[in] : 
 * session_id-ipc session id used for TC validation
 * configuration_id-ipc configuration id used for TC validation
 * key_struct-void* to link key structure 
 * value_struct-void * link value structure
 * data_type-UNC_DT_*,type of database
 * operation type-UNC_OP_*,type of operation requested
 * sess-object of ServerSession where the arguments present
 * option1,option2-UNC_OPT1,OPT2_*,additional info for read operations
 * max_rep_ct-max no. of records to be read
 * @return    : UNC_RC_SUCCESS or UNC_UPPL_RC_ERR*
 * */
UncRespCode Kt_Link::PerformRead(OdbcmConnectionHandler *db_conn,
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

  key_link_t *obj_key_link = reinterpret_cast<key_link_t*>(key_struct);
  if (option1 != UNC_OPT1_NORMAL) {
    pfc_log_error("PerformRead provided on unsupported option1");
    rsh.result_code = UNC_UPPL_RC_ERR_INVALID_OPTION1;
    int err = PhyUtil::sessOutRespHeader(sess, rsh);
    err |= sess.addOutput((uint32_t)UNC_KT_LINK);
    err |= sess.addOutput(*obj_key_link);
    if (err != 0) {
      pfc_log_info("addOutput failed for physical_response_header");
      return UNC_UPPL_RC_ERR_IPC_WRITE_ERROR;
    }
    return UNC_RC_SUCCESS;
  }


  UncRespCode read_status = UNC_RC_SUCCESS;
  // Read operations will return switch_st
  if ((unc_keytype_datatype_t)data_type == UNC_DT_STATE &&
      option1 == UNC_OPT1_NORMAL) {
    if (option2 != UNC_OPT2_MATCH_SWITCH1 &&
        option2 != UNC_OPT2_MATCH_SWITCH2 &&
        option2 != UNC_OPT2_MATCH_BOTH_SWITCH &&
        option2 != UNC_OPT2_NONE) {
      pfc_log_error("PerformRead provided on unsupported option2");
      rsh.result_code = UNC_UPPL_RC_ERR_INVALID_OPTION2;
      int err = PhyUtil::sessOutRespHeader(sess, rsh);
      err |= sess.addOutput((uint32_t)UNC_KT_LINK);
      err |= sess.addOutput(*obj_key_link);
      if (err != 0) {
        pfc_log_info("addOutput failed for physical_response_header");
        return UNC_UPPL_RC_ERR_IPC_WRITE_ERROR;
      }
      return UNC_RC_SUCCESS;
    }
    vector<key_link_t> vect_link_id;
    vector<val_link_t> vect_val_link;
    vector<val_link_st_t> vect_val_link_st;
    read_status = ReadLinkValFromDB(db_conn, key_struct,
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
      return UNC_UPPL_RC_ERR_IPC_WRITE_ERROR;
    }
    if (read_status != UNC_RC_SUCCESS) {
      pfc_log_error("Read operation failed with %d", read_status);
      sess.addOutput((uint32_t)UNC_KT_LINK);
      sess.addOutput(*obj_key_link);
      return UNC_RC_SUCCESS;
    }
    pfc_log_debug("From db, vect_link_id size is %"
                  PFC_PFMT_SIZE_T, vect_link_id.size());
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
    rsh.result_code = UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED;
    int err = PhyUtil::sessOutRespHeader(sess, rsh);
    err |= sess.addOutput((uint32_t)UNC_KT_LINK);
    if (err != 0) {
      pfc_log_info("addOutput failed for physical_response_header");
      return UNC_UPPL_RC_ERR_IPC_WRITE_ERROR;
    }
  }
  return UNC_RC_SUCCESS;
}

/** ReadLinkValFromDB
 * @Description : This function reads the instance of KT_Link based on
 *                  operation type - READ, READ_SIBLING_BEGIN, READ_SIBLING
 *                   from data base
 * @param[in] : key_struct-void* to kink key structure
 * value_struct-void* to kink value structure
 * data_type-UNC_DT_*,type of database
 * operation_type-UNC_OP_*,type of operation
 * max_rep_ct-max no of records to be read
 * vect_val_link_st-vector<val_link_st_t> instance
 * link_id-instance of vector<key_link_t>
 * option1,option2-additional info for read operations,UNC_OPT1/OPT2_* 
 * is_state-flag to indicate whether data type is DT_STATE
 * @return    : Success or associated error code,
 * UNC_RC_SUCCESS is returned when the response
 * is added to ipc session successfully.
 * UNC_UPPL_RC_ERR_* is returned when ipc response could not be added to sess.
 * */
UncRespCode Kt_Link::ReadLinkValFromDB(
    OdbcmConnectionHandler *db_conn,
    void* key_struct,
    void* val_struct,
    uint32_t data_type,
    uint32_t operation_type,
    uint32_t &max_rep_ct,
    vector<val_link_st_t> &vect_val_link_st,
    vector<key_link_t> &link_id,
    uint32_t option1,
    uint32_t option2) {
  if (operation_type < UNC_OP_READ) {
    // Unsupported operation type for this function
    return UNC_RC_SUCCESS;
  }
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  UncRespCode read_status = UNC_RC_SUCCESS;
  ODBCM_RC_STATUS read_db_status = ODBCM_RC_SUCCESS;

  // Construct TableAttrSchema structure
  // TableAttrSchema holds table_name, primary key, attr_name
  DBTableSchema kt_link_dbtableschema;
  vector<ODBCMOperator> vect_prim_key_operations;
  void* old_value;
  PopulateDBSchemaForKtTable(db_conn, kt_link_dbtableschema,
                             key_struct,
                             val_struct,
                             operation_type, data_type,
                             option1, option2,
                             vect_prim_key_operations,
                             old_value, NOTAPPLIED,
                             false, PFC_FALSE);

  if (operation_type == UNC_OP_READ) {
    read_db_status = physical_layer->get_odbc_manager()->
        GetOneRow((unc_keytype_datatype_t)data_type,
                  kt_link_dbtableschema, db_conn);
  } else if ((operation_type == UNC_OP_READ_SIBLING_BEGIN ||
      operation_type == UNC_OP_READ_SIBLING) &&
      (option1 == UNC_OPT1_NORMAL &&
          (option2 == UNC_OPT2_MATCH_SWITCH1 ||
              option2 == UNC_OPT2_MATCH_SWITCH2 ||
              option2 == UNC_OPT2_MATCH_BOTH_SWITCH))) {
    pfc_log_debug("calling get sibling rows with filtering");
    if (option2 == UNC_OPT2_MATCH_SWITCH2) {
      pfc_log_debug("get sibling rows with reorder based on sw2");
      kt_link_dbtableschema.frame_explicit_order_=
      " ORDER BY controller_name, switch_id2, port_id2, switch_id1, port_id1 ";
    }
    read_db_status = physical_layer->get_odbc_manager()->
        GetSiblingRows((unc_keytype_datatype_t)data_type, max_rep_ct,
                       kt_link_dbtableschema,
                       vect_prim_key_operations,
                       (unc_keytype_operation_t)operation_type, db_conn);
  } else if (operation_type == UNC_OP_READ_SIBLING) {
    pfc_log_debug("Ktlink:Primarykeysize: %"
                    PFC_PFMT_SIZE_T,
                    kt_link_dbtableschema.primary_keys_.size());
    pfc_log_debug("Inside READ SIBLING part");
    uint32_t index = 0;
    uint32_t no_of_query = 4;  // Link has 4 primary keys other than controller
    for (; index < no_of_query ; ++index) {
      if (kt_link_dbtableschema.primary_keys_.size() < 2) {
        pfc_log_debug("No record found");
        read_status = UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE;
        break;
      }
      read_db_status = physical_layer->get_odbc_manager()-> \
        GetBulkRows((unc_keytype_datatype_t)data_type, max_rep_ct,
                  kt_link_dbtableschema,
                  (unc_keytype_operation_t)UNC_OP_READ_BULK,
                  db_conn);
         kt_link_dbtableschema.primary_keys_.pop_back();
      if (read_db_status == ODBCM_RC_RECORD_NOT_FOUND) {
        read_status = UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE;
      } else if (read_db_status == ODBCM_RC_CONNECTION_ERROR) {
         read_status = UNC_UPPL_RC_ERR_DB_ACCESS;
         pfc_log_error("Read operation has failed with error %d",
                                  read_db_status);
         return read_status;
      } else if (read_db_status == ODBCM_RC_SUCCESS) {
        read_status = UNC_RC_SUCCESS;
        uint32_t max_rep_ct_new = 0;
        pfc_log_debug("Received success response from db");
        FillLinkValueStructure(db_conn, kt_link_dbtableschema,
                       vect_val_link_st,
                       max_rep_ct_new,
                       operation_type,
                       link_id);
        pfc_log_debug("max_rep_ct_new=%d max_rep_ct=%d",
                      max_rep_ct_new, max_rep_ct);
        for (uint32_t uindex = 1; uindex < max_rep_ct_new; uindex++) {
          kt_link_dbtableschema.DeleteRowListFrontElement();
        }
        max_rep_ct -= max_rep_ct_new;
      } else {
        read_status = UNC_UPPL_RC_ERR_DB_GET;
        return read_status;
      }
    }  // for end
    if (vect_val_link_st.empty() && index == 4) {
      pfc_log_debug("No record found");
      read_status = UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE;
    } else if (!vect_val_link_st.empty()) {
      max_rep_ct = vect_val_link_st.size();
      read_status = UNC_RC_SUCCESS;
    }
    return read_status;
  } else {
    pfc_log_debug("Inside READ SIBLING BEGIN part ");
    read_db_status = physical_layer->get_odbc_manager()->
    GetBulkRows((unc_keytype_datatype_t)data_type, max_rep_ct,
                 kt_link_dbtableschema,
                 (unc_keytype_operation_t)operation_type, db_conn);
  }
  // except read sibling - all other reads following block will fill the values
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
  pfc_log_debug("Read operation result: %d", read_status);
  FillLinkValueStructure(db_conn, kt_link_dbtableschema,
                         vect_val_link_st,
                         max_rep_ct,
                         operation_type,
                         link_id);
  pfc_log_debug("vect_val_link_st size: %"
                PFC_PFMT_SIZE_T, vect_val_link_st.size());
  pfc_log_debug("link_id size: %" PFC_PFMT_SIZE_T, link_id.size());
  if (vect_val_link_st.empty()) {
    // Read failed , return error
    read_status = UNC_UPPL_RC_ERR_DB_GET;
    // log error to log daemon
    pfc_log_error("Read operation has failed after reading response");
    return read_status;
  }
  return read_status;
}

/** Fill_Attr_Syntax_Map
 * @Description : This function populates the values to be used for attribute
 * validation
 * @param[in] : None
 * @return    : None
 * */
void Kt_Link::Fill_Attr_Syntax_Map() {
  map<string, Kt_Class_Attr_Syntax> attr_syntax_map;
  Kt_Class_Attr_Syntax objKeyAttrSyntax =
  { PFC_IPCTYPE_UINT8, 0, 0, 1, 32, true,  "" };
  attr_syntax_map[CTR_NAME_STR] = objKeyAttrSyntax;

  Kt_Class_Attr_Syntax objKeyAttrSyntax1 =
  { PFC_IPCTYPE_STRING, 0, 0, 1, 256, true,  "" };
  attr_syntax_map[LINK_SWITCH_ID1_STR] = objKeyAttrSyntax1;

  Kt_Class_Attr_Syntax objKeyAttrSyntax2 =
  { PFC_IPCTYPE_STRING, 0, 0, 1, 32, true,  "" };
  attr_syntax_map[LINK_PORT_ID1_STR] = objKeyAttrSyntax2;

  Kt_Class_Attr_Syntax objKeyAttrSyntax3 =
  { PFC_IPCTYPE_STRING, 0, 0, 1, 256, true,  "" };
  attr_syntax_map[LINK_SWITCH_ID2_STR] = objKeyAttrSyntax3;

  Kt_Class_Attr_Syntax objKeyAttrSyntax4 =
  { PFC_IPCTYPE_STRING, 0, 0, 1, 32, true,  "" };
  attr_syntax_map[LINK_PORT_ID2_STR] = objKeyAttrSyntax4;

  Kt_Class_Attr_Syntax objAttrDescSyntax =
  { PFC_IPCTYPE_STRING, 0, 0, 0, 128, false,  "" };
  attr_syntax_map[LINK_DESCRIPTION_STR] = objAttrDescSyntax;

  Kt_Class_Attr_Syntax objAttrValidSyntax =
  { PFC_IPCTYPE_STRING, 0, 0, 0, 2, false, "" };
  attr_syntax_map[LINK_VALID_STR] = objAttrValidSyntax;
  attr_syntax_map_all[UNC_KT_LINK] = attr_syntax_map;
}

/** GetLinkValidFlag
 * @Description : This function reads the valid flag from DB
 * @param[in] : key_struct-void* to link key structure
 * val_link_valid_st-instance of val_link_st_t
 * @return    : Success or associated error code
 * UNC_RC_SUCCESS is returned when the response
 * is added to ipc session successfully.
 * UNC_UPPL_RC_ERR_* is returned when ipc response could not be added to sess.
 * */
UncRespCode Kt_Link::GetLinkValidFlag(
    OdbcmConnectionHandler *db_conn,
    void *key_struct,
    val_link_st_t &val_link_valid_st,
    uint32_t data_type) {
  UncRespCode return_code = UNC_RC_SUCCESS;
  vector<void *> vectVal_link;
  vector<void *> vectkey_link;
  vectkey_link.push_back(key_struct);

  return_code = ReadInternal(db_conn, vectkey_link,
                             vectVal_link,
                             data_type, UNC_OP_READ);
  if (return_code == UNC_RC_SUCCESS) {
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
      pfc_log_debug("update link valid ret null val");
    }
  } else {
    pfc_log_info("read internal failure from ctr updatevalid");
  }
  return return_code;
}

/** GetOperStatus
 * @Description : This function reads the oper_status value of the link
 * @param[in] : key_struct-void* to link key structure
 * data_type-UNC_DT_*,type of database
 * param[out]:
 * oper_status-indicates the oper status of link whether up or down
 * @return    : Success or associated error code
 * UNC_RC_SUCCESS is returned when the response
 * is added to ipc session successfully.
 * UNC_UPPL_RC_ERR_* is returned when ipc response could not be added to sess.
 */
UncRespCode Kt_Link::GetOperStatus(OdbcmConnectionHandler *db_conn,
                                      uint32_t data_type,
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
  PhyUtil::FillDbSchema(unc::uppl::CTR_NAME, controller_name,
                        controller_name.length(), DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);
  if (!controller_name.empty()) {
    vect_prim_keys.push_back(CTR_NAME_STR);
  }
  // switch_id1
  PhyUtil::FillDbSchema(unc::uppl::LINK_SWITCH_ID1, switch_id1,
                        switch_id1.length(), DATATYPE_UINT8_ARRAY_256,
                        vect_table_attr_schema);
  if (!switch_id1.empty()) {
    vect_prim_keys.push_back(LINK_SWITCH_ID1_STR);
  }
  // port_id1
  PhyUtil::FillDbSchema(unc::uppl::LINK_PORT_ID1, port_id1,
                        port_id1.length(), DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);
  if (!port_id1.empty()) {
    vect_prim_keys.push_back(LINK_PORT_ID1_STR);
  }
  // switch_id2
  PhyUtil::FillDbSchema(unc::uppl::LINK_SWITCH_ID2, switch_id2,
                        switch_id2.length(), DATATYPE_UINT8_ARRAY_256,
                        vect_table_attr_schema);
  if (!switch_id2.empty()) {
    vect_prim_keys.push_back(LINK_SWITCH_ID2_STR);
  }
  // port_id2
  PhyUtil::FillDbSchema(unc::uppl::LINK_PORT_ID2, port_id2,
                        port_id2.length(), DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);
  if (!port_id2.empty()) {
    vect_prim_keys.push_back(LINK_PORT_ID2_STR);
  }

  string oper_value = "";
  PhyUtil::FillDbSchema(unc::uppl::LINK_OPER_STATUS, oper_value,
                        oper_value.length(), DATATYPE_UINT16,
                        vect_table_attr_schema);
  DBTableSchema kt_link_dbtableschema;
  PhyUtil::reorder_col_attrs(vect_prim_keys, vect_table_attr_schema);
  kt_link_dbtableschema.set_table_name(unc::uppl::LINK_TABLE);
  kt_link_dbtableschema.set_primary_keys(vect_prim_keys);
  row_list.push_back(vect_table_attr_schema);
  kt_link_dbtableschema.set_row_list(row_list);

  // Call ODBCManager and update
  ODBCM_RC_STATUS update_db_status =
      physical_layer->get_odbc_manager()->GetOneRow(
          (unc_keytype_datatype_t)data_type,
          kt_link_dbtableschema, db_conn);
  if (update_db_status != ODBCM_RC_SUCCESS) {
    pfc_log_info("oper_status read operation failed %d", update_db_status);
    return UNC_UPPL_RC_ERR_DB_GET;
  }

  // read the oper_status value
  list < vector<TableAttrSchema> >& res_row_list =
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
      ODBCMTableColumns attr_name = tab_schema.table_attribute_name;
      string attr_value = "";
      if (attr_name == unc::uppl::LINK_OPER_STATUS) {
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

/** FrameValidValue
 * @Description : This function converts the string value from db to uint8
 * @param[in] : attr_value-attribute value in string
 * obj_val_link-object of val_link_st
 * @return    : Success or associated error code
 * UNC_RC_SUCCESS is returned when the response
 * is added to ipc session successfully.
 * UNC_UPPL_RC_ERR_* is returned when ipc response could not be added to sess.
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
 * @Description : This function decides the primary key based on option
 * @param[in] : operation_type-UNC_OP_*
 * option1,option2-UNC_OPT1/OPT2_*,additional info for read operations
 * switch_id1,switch_id2-switch names that connected by the link through ports
 * port_id1,port_id2-port numbers thatare connected by the link
 * vect_prim_keys-vector<string> conatining link keys
 * vect_prim_keys_operation-instance of vector<ODBCMOperator>
 * @return    : Success or associated error code
 * UNC_RC_SUCCESS is returned when the response
 * is added to ipc session successfully.
 * UNC_UPPL_RC_ERR_* is returned when ipc response could not be added to sess.
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
    vect_prim_keys.push_back(LINK_SWITCH_ID1_STR);
    vect_prim_keys_operation.push_back(unc::uppl::EQUAL);
  }
  if (option1 == UNC_OPT1_NORMAL && option2 == UNC_OPT2_MATCH_SWITCH2) {
    vect_prim_keys.push_back(LINK_SWITCH_ID2_STR);
    vect_prim_keys_operation.push_back(unc::uppl::EQUAL);
  }
  if (option1 == UNC_OPT1_NORMAL &&
      option2 == UNC_OPT2_MATCH_BOTH_SWITCH) {
    vect_prim_keys.push_back(LINK_SWITCH_ID1_STR);
    vect_prim_keys_operation.push_back(unc::uppl::EQUAL);
    vect_prim_keys.push_back(LINK_SWITCH_ID2_STR);
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
    vect_prim_keys.push_back(LINK_SWITCH_ID1_STR);
  if (!port_id1.empty()) {
    vect_prim_keys.push_back(LINK_PORT_ID1_STR);
  }
    if (!switch_id2.empty()) {
      vect_prim_keys.push_back(LINK_SWITCH_ID2_STR);
    }
  if (!port_id2.empty()) {
    vect_prim_keys.push_back(LINK_PORT_ID2_STR);
  }
  } else {
    if (!port_id1.empty()) {
      if (option2 == UNC_OPT2_MATCH_SWITCH1 ||
          option2 == UNC_OPT2_MATCH_BOTH_SWITCH) {
        vect_prim_keys.push_back(LINK_PORT_ID1_STR);
        vect_prim_keys_operation.push_back(unc::uppl::GREATER);
      }
    }

    if (!port_id2.empty()) {
      if (option2 == UNC_OPT2_MATCH_SWITCH2) {
        vect_prim_keys.push_back(LINK_PORT_ID2_STR);
        vect_prim_keys_operation.push_back(unc::uppl::GREATER);
      }
    }
  }
}
