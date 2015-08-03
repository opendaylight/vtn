/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/**
 * @brief    KT Boundary implementation
 * @file     itc_kt_boundary.cc
 *
 */

#include <uncxx/tclib/tclib_defs.hh>
#include "itc_kt_boundary.hh"
#include "itc_kt_controller.hh"
#include "itc_kt_logicalport.hh"
#include "odbcm_db_varbind.hh"
#include "ipct_util.hh"
#include "itc_read_request.hh"

using unc::uppl::PhysicalLayer;
#define DEFAULT_DOMAIN "(DEFAULT)"

/** Constructor
 * * @Description : This function fills the syntax map used for validation
 * * @param[in]   : None
 * * @return      : None
 **/
Kt_Boundary::Kt_Boundary() {
  if (attr_syntax_map_all.find(UNC_KT_BOUNDARY) ==
      attr_syntax_map_all.end()) {
    // Populate structure to be used for syntax validation
    Fill_Attr_Syntax_Map();
  }
}

/** Destructor
 * * @Description : Empty Destructor
 * * @param[in]   : None
 * * @return        : None
 **/
Kt_Boundary::~Kt_Boundary() {
}

/** Create
 * * @Description : This function creates a new row of KT_Boundary in
 * candidate boundary table.
 * * @param[in] : session_id - ipc session id used for TC validation
 * configuration_id - configuration id used for TC validation
 * key_struct - the key for the new kt boundary instance
 * value_struct - the values for the new kt boundary instance
 * data_type - UNC_DT_* , Create only allowed in candidate
 * sess - ipc server session where the response has to be added
 * * @return    : UNC_RC_SUCCESS is returned when the response
 * is added to ipc session successfully.
 * UNC_UPPL_RC_ERR_* is returned when ipc response could not be added to sess.
 * */
UncRespCode Kt_Boundary::Create(OdbcmConnectionHandler *db_conn,
                                   uint32_t session_id,
                                   uint32_t configuration_id,
                                   void* key_struct,
                                   void* val_struct,
                                   uint32_t data_type,
                                   ServerSession &sess) {
  UncRespCode create_status = UNC_RC_SUCCESS;
  // Check whether operation is allowed on the given DT type
  if ((unc_keytype_datatype_t)data_type != UNC_DT_CANDIDATE) {
    pfc_log_error("Create operation is invoked on unsupported data type %d",
                  data_type);
    create_status = UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED;
  } else {
    create_status = CreateKeyInstance(db_conn, key_struct, val_struct,
                                      data_type, UNC_KT_BOUNDARY);

    pfc_log_debug("Calling CreateKeyInstance, returned %d", create_status);
  }
  key_boundary_t *key_obj= reinterpret_cast<key_boundary_t*>(key_struct);
  // Populate the response to be sent in ServerSession
  physical_response_header rsh = {session_id,
      configuration_id,
      UNC_OP_CREATE,
      0,
      0,
      0,
      data_type,
      static_cast<uint32_t>(create_status)};
  rsh.result_code = create_status;
  int err = PhyUtil::sessOutRespHeader(sess, rsh);
  err |= sess.addOutput((uint32_t)UNC_KT_BOUNDARY);
  err |= sess.addOutput(*key_obj);
  if (err != 0) {
    pfc_log_error("Server session addOutput failed, so return IPC_WRITE_ERROR");
    create_status = UNC_UPPL_RC_ERR_IPC_WRITE_ERROR;
  } else {
    create_status = UNC_RC_SUCCESS;
  }
  return create_status;
}

/** CreateKeyInstance
 * * @Description : This function creates a new row of KT_Boundary in
 * candidate boundary table.
 * param[in]:key_struct - the key for the new kt boundary instance
 * value_struct - the values for the new kt boundary instance
 * data_type - UNC_DT_* , Create only allowed in candidate
 * key_type-UNC_KT_BOUNDARY,value of unc_key_type_t
 * @return    : UNC_RC_SUCCESS is returned when the create is success
 * UNC_UPPL_RC_ERR_* is returned when ipc response could not be added to sess.
 * */
UncRespCode Kt_Boundary::CreateKeyInstance(OdbcmConnectionHandler *db_conn,
                                              void* key_struct,
                                              void* val_struct,
                                              uint32_t data_type,
                                              uint32_t key_type) {
  UncRespCode create_status = UNC_RC_SUCCESS;
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();

  // Structure used to send request to ODBC
  DBTableSchema kt_boundary_dbtableschema;
  vector<ODBCMOperator> vect_key_operations;
  void* old_val_struct;
  // Create DBSchema structure for boundary table
  PopulateDBSchemaForKtTable(db_conn, kt_boundary_dbtableschema,
                             key_struct,
                             val_struct,
                             UNC_OP_CREATE, data_type, 0, 0,
                             vect_key_operations, old_val_struct,
                             NOTAPPLIED, false, PFC_FALSE);

  // Send request to ODBC for boundary_table create
  ODBCM_RC_STATUS create_db_status = physical_layer->get_odbc_manager()->\
      CreateOneRow((unc_keytype_datatype_t)data_type,
                   kt_boundary_dbtableschema, db_conn);
  if (create_db_status != ODBCM_RC_SUCCESS) {
    if (create_db_status == ODBCM_RC_CONNECTION_ERROR) {
      // log fatal error to log daemon
      UPPL_LOG_FATAL("DB connection not available or cannot access DB");
      create_status = UNC_UPPL_RC_ERR_DB_ACCESS;
    } else if (create_db_status == ODBCM_RC_PKEY_VIOLATION) {
      // log fatal error to log daemon
      pfc_log_error("Boundary instance already exists");
      create_status = UNC_UPPL_RC_ERR_INSTANCE_EXISTS;
    } else {
      // log error to log daemon
      pfc_log_error("Create operation has failed");
      create_status = UNC_UPPL_RC_ERR_DB_CREATE;
    }
  } else {
    pfc_log_info("Create of a boundary in datatype(%d) is success", data_type);
  }
  return create_status;
}

/** Update
 * @Description : This function updates a row of KT_Boundary in
 * candidate boundary table.
 * @param[in] : session_id - ipc session id used for TC validation
 * configuration_id - configuration id used for TC validation
 * key_struct - the key for the kt boundary instance
 * value_struct - the values for the kt boundary instance
 * data_type - UNC_DT_* , Update only allowed in candidate
 * sess - ipc server session where the response has to be added
 * @return    : UNC_RC_SUCCESS is returned when the response
 * is added to ipc session successfully.
 * UNC_UPPL_RC_ERR_* is returned when ipc response could not be added to sess.
 * */
UncRespCode Kt_Boundary::Update(OdbcmConnectionHandler *db_conn,
                                   uint32_t session_id,
                                   uint32_t configuration_id,
                                   void* key_struct,
                                   void* val_struct,
                                   uint32_t data_type,
                                   ServerSession & sess) {
  UncRespCode update_status = UNC_RC_SUCCESS;
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  key_boundary_t *key_obj = reinterpret_cast<key_boundary_t*>(key_struct);
  if ((unc_keytype_datatype_t)data_type != UNC_DT_CANDIDATE) {
    pfc_log_error("Update operation is provided on unsupported data type");
    update_status = UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED;
  } else {
    // Structure used to send request to ODBC
    DBTableSchema kt_boundary_dbtableschema;
    // Create DBSchema structure for boundary_table
    vector<ODBCMOperator> vect_key_operations;
    void* old_val_struct;
    PopulateDBSchemaForKtTable(db_conn, kt_boundary_dbtableschema,
                               key_struct,
                               val_struct,
                               UNC_OP_UPDATE, data_type, 0, 0,
                               vect_key_operations, old_val_struct,
                               NOTAPPLIED, false, PFC_FALSE);

    if (!((kt_boundary_dbtableschema.get_row_list()).empty())) {
      // Send request to ODBC for Boundary_common_table update
      ODBCM_RC_STATUS update_db_status = physical_layer->get_odbc_manager()->
          UpdateOneRow((unc_keytype_datatype_t)data_type,
                       kt_boundary_dbtableschema, db_conn, false);
      if (update_db_status != ODBCM_RC_SUCCESS &&
          update_db_status == ODBCM_RC_CONNECTION_ERROR) {
        // log fatal error to log daemon
        UPPL_LOG_FATAL("DB connection not available or cannot access DB");
        update_status = UNC_UPPL_RC_ERR_DB_ACCESS;
      } else if (update_db_status != ODBCM_RC_SUCCESS) {
        // log error to log daemon
        pfc_log_error("Update operation has failed");
        update_status = UNC_UPPL_RC_ERR_DB_UPDATE;
      }
      if (update_db_status != ODBCM_RC_SUCCESS) {
        pfc_log_info("Update of a boundary in data_type(%d) is success",
                     data_type);
      }
    } else {
      pfc_log_info("Nothing to be updated, so return");
    }
  }
  // Populate the response to be sent in ServerSession
  physical_response_header rsh = {session_id,
      configuration_id,
      UNC_OP_UPDATE,
      0,
      0,
      0,
      data_type,
      static_cast<uint32_t>(update_status)};
  int err = PhyUtil::sessOutRespHeader(sess, rsh);
  err |= sess.addOutput((uint32_t)UNC_KT_BOUNDARY);
  err |= sess.addOutput(*key_obj);
  if (err != 0) {
    pfc_log_error("Server session addOutput failed, so return IPC_WRITE_ERROR");
    update_status = UNC_UPPL_RC_ERR_IPC_WRITE_ERROR;
  } else {
    update_status = UNC_RC_SUCCESS;
  }
  return update_status;
}

/** Delete
 * * @Description : This function deletes a row of KT_Boundary in
 * candidate boundary table.
 * * @param[in] : session_id - ipc session id used for TC validation
 * configuration_id - configuration id used for TC validation
 * key_struct - the key for the kt boundary instance
 * data_type - UNC_DT_* , delete only allowed in candidate
 * sess - ipc server session where the response has to be added
 * * @return    : UNC_RC_SUCCESS is returned when the response
 * is added to ipc session successfully.
 * UNC_UPPL_RC_ERR_* is returned when ipc response could not be added to sess.
 * */
UncRespCode Kt_Boundary::Delete(OdbcmConnectionHandler *db_conn,
                                   uint32_t session_id,
                                   uint32_t configuration_id,
                                   void* key_struct,
                                   uint32_t data_type,
                                   ServerSession & sess) {
  UncRespCode delete_status = UNC_RC_SUCCESS;
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  key_boundary_t *key_obj= reinterpret_cast<key_boundary_t*>(key_struct);
  // Check whether operation is allowed on the given DT type
  if ((unc_keytype_datatype_t)data_type != UNC_DT_CANDIDATE) {
    pfc_log_error("Delete operation is invoked on unsupported data type %d",
                  data_type);
    delete_status = UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED;
    // Populate the response to be sent in ServerSession
    physical_response_header rsh = {session_id,
        configuration_id,
        UNC_OP_DELETE,
        0,
        0,
        0,
        data_type,
        static_cast<uint32_t>(delete_status)};
    int err = PhyUtil::sessOutRespHeader(sess, rsh);
    err |= sess.addOutput((uint32_t)UNC_KT_BOUNDARY);
    err |= sess.addOutput(*key_obj);
    if (err != 0) {
      pfc_log_info(
          "Server session addOutput failed, so return IPC_WRITE_ERROR");
      return UNC_UPPL_RC_ERR_IPC_WRITE_ERROR;
    }
    return UNC_RC_SUCCESS;
  }
  string boundary_id = (const char*)key_obj->boundary_id;

  // Check whether BOUNDARY is being referred in Logical layer
  TcConfigMode config_mode;
  std::string vtn_name = "";
  UncRespCode validate_status = PhysicalLayer::get_instance() \
      ->get_physical_core() \
      ->GetConfigMode(session_id, configuration_id, config_mode, vtn_name);
  if (validate_status != UNC_RC_SUCCESS) {
    if (validate_status == UNC_UPPL_RC_ERR_INVALID_CONFIGID) {
      pfc_log_error("Physical_core::GetConfigMode::Configid validation failed");
    }
    if (validate_status == UNC_UPPL_RC_ERR_INVALID_SESSIONID) {
      pfc_log_error("Physical_core::GetConfigMode::Sessonid validation failed");
    }
    // Populate the response to be sent in ServerSession
    physical_response_header rsh = {session_id,
        configuration_id,
        UNC_OP_DELETE,
        0,
        0,
        0,
        data_type,
        static_cast<uint32_t>(validate_status)};
    int err = PhyUtil::sessOutRespHeader(sess, rsh);
    err |= sess.addOutput((uint32_t)UNC_KT_BOUNDARY);
    err |= sess.addOutput(*key_obj);
    if (err != 0) {
      pfc_log_info(
          "Server session addOutput failed, so return IPC_WRITE_ERROR");
      return UNC_UPPL_RC_ERR_IPC_WRITE_ERROR;
    }
    return UNC_RC_SUCCESS;
  }
  // Check whether BOUNDARY is being referred in Logical layer
  delete_status = SendSemanticRequestToUPLL(key_struct,
                                            data_type);

  if (delete_status != UNC_RC_SUCCESS) {
    // log error and send error response
    pfc_log_error("Boundary is referred in Logical,"
        "so delete is not allowed");
  } else if (delete_status == UNC_RC_SUCCESS &&
                          config_mode == TC_CONFIG_REAL) {
    // Check whether BOUNDARY is referred in Running DB in Logical layer
    delete_status = SendSemanticRequestToUPLL(key_struct,
                                              UNC_DT_RUNNING);
    if (delete_status != UNC_RC_SUCCESS) {
      pfc_log_error("Boundary is referred in Running DB in Logical, "
         "so delete is not allowed");
    }
  } else {
    pfc_log_debug("Boundary is not referred in CANDIDATE DB in logical");
  }
  if (delete_status == UNC_RC_SUCCESS) {
    // Structure used to send request to ODBC
    DBTableSchema kt_boundary_dbtableschema;

    // Construct Primary key list
    vector<string> vect_prim_keys;
    vect_prim_keys.push_back(BDRY_ID_STR);

    // construct TableAttrSchema structure
    // TableAttrSchema holds table_name, primary key, attr_name
    vector<TableAttrSchema> vect_table_attr_schema;
    list < vector<TableAttrSchema> > row_list;

    // boundary_id
    PhyUtil::FillDbSchema(unc::uppl::BDRY_ID, boundary_id, boundary_id.length(),
                          DATATYPE_UINT8_ARRAY_32, vect_table_attr_schema);

    // Send request to ODBC for Boundary_common_table delete
    kt_boundary_dbtableschema.set_table_name(unc::uppl::BOUNDARY_TABLE);
    kt_boundary_dbtableschema.set_primary_keys(vect_prim_keys);
    row_list.push_back(vect_table_attr_schema);
    kt_boundary_dbtableschema.set_row_list(row_list);

    ODBCM_RC_STATUS delete_db_status = physical_layer->get_odbc_manager()->
        DeleteOneRow((unc_keytype_datatype_t)data_type,
                     kt_boundary_dbtableschema, db_conn);
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
      pfc_log_info("Delete of a boundary in data_type(%d) is success",
                   data_type);
      delete_status = UNC_RC_SUCCESS;
    }
  }
  // Populate the response to be sent in ServerSession
  physical_response_header rsh = {session_id,
      configuration_id,
      UNC_OP_DELETE,
      0,
      0,
      0,
      data_type,
      static_cast<uint32_t>(delete_status)};
  int err = PhyUtil::sessOutRespHeader(sess, rsh);
  err |= sess.addOutput((uint32_t)UNC_KT_BOUNDARY);
  err |= sess.addOutput(*key_obj);
  if (err != 0) {
    pfc_log_info("Server session addOutput failed, so return IPC_WRITE_ERROR");
    return UNC_UPPL_RC_ERR_IPC_WRITE_ERROR;
  }
  return UNC_RC_SUCCESS;
}

/** ReadInternal
 * @Description : This function reads the given  instance of KT_Boundary
 * @param[in] : 
 * boundary_key - vector of void* to the key for the kt boundary instance
 * boundary_val - vector of void* to the value for the kt boundary instance
 * data_type - UNC_DT_* , read allowed in candidate/running/startup/state
 * operation_type-UNC_OP_*,type of operation
 * @return    : UNC_RC_SUCCESS is returned when the response
 * is added to ipc session successfully.
 * UNC_UPPL_RC_ERR_* is returned when ipc response could not be added to sess.
 * */
UncRespCode Kt_Boundary::ReadInternal(OdbcmConnectionHandler *db_conn,
                                         vector<void *> &boundary_key,
                                         vector<void *> &boundary_val,
                                         uint32_t data_type,
                                         uint32_t operation_type) {
  if (operation_type != UNC_OP_READ && operation_type != UNC_OP_READ_SIBLING &&
      operation_type != UNC_OP_READ_SIBLING_BEGIN) {
    pfc_log_trace("This function not allowed for read next/bulk/count");
    return UNC_UPPL_RC_ERR_OPERATION_NOT_SUPPORTED;
  }
  pfc_log_debug("Processing Kt_Boundary::ReadInternal");
  uint32_t max_rep_ct = 1;
  if (operation_type != UNC_OP_READ) {
    // Get read response from database
    max_rep_ct = UPPL_MAX_REP_CT;
  }
  void *key_struct = boundary_key[0];
  void *val_struct = NULL;
  val_boundary_st_t st_boundary_val;
  memset(&st_boundary_val, '\0', sizeof(val_boundary_st_t));
  if ((!boundary_val.empty()) && (boundary_val[0] != NULL)) {
    memcpy(&st_boundary_val,
           (reinterpret_cast<val_boundary_st_t *> (boundary_val[0])),
           sizeof(val_boundary_st_t));
    val_struct = reinterpret_cast<void *>(&st_boundary_val.boundary);
  }
  // Get read response from database
  UncRespCode read_status = UNC_RC_SUCCESS;
  bool firsttime = true;
  do {
    vector<key_boundary_t> vect_boundary_id;
    vector<val_boundary_st_t> vect_val_boundary_st;
    read_status = ReadBoundaryValFromDB(db_conn,
                                        key_struct,
                                        val_struct,
                                        data_type,
                                        operation_type,
                                        max_rep_ct,
                                        vect_boundary_id,
                                        vect_val_boundary_st);
    if (firsttime) {
      pfc_log_trace(
          "Clearing key_val and val_struct vectors for the first time");
      boundary_key.clear();
      boundary_val.clear();
      firsttime = false;
    }
    pfc_log_debug("ReadBoundaryValFromDB returned %d with response size %"
                PFC_PFMT_SIZE_T, read_status, vect_val_boundary_st.size());
    if (read_status == UNC_RC_SUCCESS) {
      for (unsigned int iIndex = 0 ; iIndex < vect_boundary_id.size();
           ++iIndex) {
        key_boundary_t *key_boundary =
            new key_boundary_t(vect_boundary_id[iIndex]);
        boundary_key.push_back(reinterpret_cast<void *>(key_boundary));
        val_boundary_st_t *val_boundary =
            new val_boundary_st_t(vect_val_boundary_st[iIndex]);
        boundary_val.push_back(reinterpret_cast<void *>(val_boundary));
      }
    } else if ((read_status == UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE &&
               boundary_val.size() != 0)) {
      read_status = UNC_RC_SUCCESS;
    }
    if ((vect_val_boundary_st.size() == UPPL_MAX_REP_CT) &&
                     (operation_type != UNC_OP_READ)) {
      pfc_log_debug("Op:%d, key.size:%" PFC_PFMT_SIZE_T"fetch_next_set",
                    operation_type, boundary_key.size());
      key_struct = reinterpret_cast<void *>(
                     boundary_key[boundary_key.size() - 1]);
      operation_type = UNC_OP_READ_SIBLING;
      continue;
    } else {
      break;
    }
  } while (true);
  return read_status;
}

/** ReadBulk
 * @Description : This function reads bulk rows of KT_Boundary in
 *  boundary table of specified data type.
 * @param[in] :
 * key_struct - the key for the kt boundary instance
 * data_type - UNC_DT_* , read allowed in candidate/running/startup/state
 * max_rep_ct - specifies number of rows to be returned
 * parent_call - indicates whether parent has called this readbulk
 * is_read_next - indicates whether this function is invoked from readnext
 * @return    : UNC_RC_SUCCESS is returned when the response
 * is added to ipc session successfully.
 * UNC_UPPL_RC_ERR_* is returned when ipc response could not be added to sess.
 * */
UncRespCode Kt_Boundary::ReadBulk(OdbcmConnectionHandler *db_conn,
                                     void* key_struct,
                                     uint32_t data_type,
                                     uint32_t &max_rep_ct,
                                     int child_index,
                                     pfc_bool_t parent_call,
                                     pfc_bool_t is_read_next,
                                     ReadRequest *read_req) {
  UncRespCode read_status = UNC_RC_SUCCESS;
  if (data_type != UNC_DT_CANDIDATE && data_type != UNC_DT_RUNNING &&
      data_type != UNC_DT_STATE && data_type != UNC_DT_STARTUP) {
    read_status = UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED;
    pfc_log_debug("ReadBulk operation is not allowed in %d data type",
                  data_type);
    return read_status;
  }

  if (max_rep_ct == 0) {
    pfc_log_info("max_rep_ct is 0");
    return UNC_RC_SUCCESS;
  }
  // check whether UNC_KT_BOUNDARY key instance is available in the request
  vector<val_boundary_st_t> vect_val_boundary;
  void *val_struct = NULL;
  vector<key_boundary_t> vect_key_boundary;
  read_status = ReadBulkInternal(
      db_conn, key_struct,
      val_struct,
      data_type,
      max_rep_ct,
      vect_key_boundary,
      vect_val_boundary);
  pfc_log_debug("read_status of ReadBulkInternal is %d with vector size %"
                PFC_PFMT_SIZE_T, read_status, vect_val_boundary.size());
  pfc_log_debug("read_status from ReadBulkInternal is %d with key size %"
                PFC_PFMT_SIZE_T, read_status, vect_key_boundary.size());
  if (read_status != UNC_RC_SUCCESS) {
    pfc_log_debug("Boundary read bulk internal is not success");
    if (read_status == UNC_UPPL_RC_ERR_DB_ACCESS) {
      pfc_log_debug("KtBoundary ReadBulk - Returning DB Access Error");
      return read_status;
    }
    return UNC_RC_SUCCESS;
  }
  vector<key_boundary_t> ::iterator boundary_iter =
      vect_key_boundary.begin();
  vector<val_boundary_st_t> ::iterator vect_iter =
      vect_val_boundary.begin();
  for (; boundary_iter != vect_key_boundary.end(); ++boundary_iter,
  ++vect_iter) {
    val_boundary_st_t obj_val_st = (*vect_iter);
    if (data_type == UNC_DT_CANDIDATE &&
        obj_val_st.boundary.cs_row_status == DELETED) {
      pfc_log_debug("Ignoring DELETED entry %s",
                    reinterpret_cast<char *>((*boundary_iter).boundary_id));
      continue;
    }
    pfc_log_debug("Adding boundary - %s to session",
                  reinterpret_cast<char *>((*boundary_iter).boundary_id));
    key_boundary_t *key_buffer = new key_boundary_t(*boundary_iter);
    BulkReadBuffer obj_key_buffer = {
        UNC_KT_BOUNDARY, IS_KEY,
        reinterpret_cast<void *>(key_buffer)
    };
    read_req->AddToBuffer(obj_key_buffer);
    if ((unc_keytype_datatype_t)data_type == UNC_DT_STATE) {
      val_boundary_st_t *val_buffer =
          new val_boundary_st_t(*vect_iter);
      BulkReadBuffer obj_value_buffer = {
          UNC_KT_BOUNDARY, IS_STATE_VALUE,
          reinterpret_cast<void *>(val_buffer)
      };
      read_req->AddToBuffer(obj_value_buffer);
    } else {
      val_boundary_t *val_buffer =
          new val_boundary_t((*vect_iter).boundary);
      BulkReadBuffer obj_value_buffer = {
          UNC_KT_BOUNDARY, IS_VALUE,
          reinterpret_cast<void *>(val_buffer)
      };
      read_req->AddToBuffer(obj_value_buffer);
    }
    BulkReadBuffer obj_sep_buffer = {
        UNC_KT_BOUNDARY, IS_SEPARATOR, NULL
    };
    read_req->AddToBuffer(obj_sep_buffer);
    --max_rep_ct;
    if (max_rep_ct == 0) {
      return UNC_RC_SUCCESS;
    }
  }
  pfc_log_debug("reached end of key tree, read_status=%d",
                read_status);
  return UNC_RC_SUCCESS;
}

/** ReadBulkInternal
 * @Description : This function reads bulk rows of KT_Boundary in
 *  bounndary table of specified data type.
 * @param[in] :
 * key_struct - the key for the kt boundary instance
 * val_struct - the value struct for kt_boundary instance
 * max_rep_ct - specifies number of rows to be returned
 * vect_key_boundary-indicates the fetched keys from db of val_boundary type
 * vect_val_boundary - indicates the fetched values
 * from db of val_boundary type
 * @return    : UNC_RC_SUCCESS is returned when the response
 * is added to ipc session successfully.
 * UNC_UPPL_RC_ERR_* is returned when ipc response could not be added to sess.
 * */
UncRespCode Kt_Boundary::ReadBulkInternal(
    OdbcmConnectionHandler *db_conn,
    void* key_struct,
    void *val_struct,
    uint32_t data_type,
    uint32_t max_rep_ct,
    vector<key_boundary_t> &vect_key_boundary,
    vector<val_boundary_st_t> &vect_val_boundary) {
  pfc_log_debug("ReadBulkInternal:Kt-Boundary");
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  UncRespCode read_status = UNC_RC_SUCCESS;
  ODBCM_RC_STATUS read_db_status = ODBCM_RC_SUCCESS;
  DBTableSchema kt_boundary_dbtableschema;
  vector<ODBCMOperator> vect_key_operations;
  void* old_val_struct;
  // Populate DBSchema for boundary_table
  PopulateDBSchemaForKtTable(db_conn, kt_boundary_dbtableschema,
                             key_struct,
                             val_struct,
                             UNC_OP_READ_BULK, data_type, 0, 0,
                             vect_key_operations, old_val_struct,
                             NOTAPPLIED, false, PFC_FALSE);
  // Read rows from DB
  read_db_status = physical_layer->get_odbc_manager()-> \
      GetBulkRows((unc_keytype_datatype_t)data_type, max_rep_ct,
                  kt_boundary_dbtableschema,
                  (unc_keytype_operation_t)UNC_OP_READ_BULK, db_conn);
  if (read_db_status == ODBCM_RC_RECORD_NOT_FOUND) {
    pfc_log_debug("No record found");
    read_status = UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE;
    return read_status;
  } else if (read_db_status == ODBCM_RC_CONNECTION_ERROR) {
    read_status = UNC_UPPL_RC_ERR_DB_ACCESS;
    pfc_log_error("Read operation has failed with error %d", read_db_status);
    return read_status;
  } else if (read_db_status != ODBCM_RC_SUCCESS) {
    // log error to log daemon
    pfc_log_error("Read operation has failed with error %d", read_db_status);
    read_status = UNC_UPPL_RC_ERR_DB_GET;
    return read_status;
  }
  // From the values received from DB, populate val_boundary structure
  FillBoundaryValueStructure(db_conn, kt_boundary_dbtableschema,
                             vect_key_boundary,
                             vect_val_boundary,
                             max_rep_ct);
  return read_status;
}

/** PerformSyntaxValidation
 * @Description : This function performs syntax validation for
 *  UNC_KT_BOUNDARY
 * @param[in]
 * key_struct - the key for the kt boundary instance
 * value_struct - the value for the kt boundary instance
 * data_type - UNC_DT_*,type of database
 * operation_type - UNC_OP*,type of operation
 * @return    : UNC_RC_SUCCESS is returned when the validation is successful
 * UNC_UPPL_RC_ERR_* is returned when validation is failure
 * */
UncRespCode Kt_Boundary::PerformSyntaxValidation(
    OdbcmConnectionHandler *db_conn,
    void* key_struct,
    void* val_struct,
    uint32_t operation,
    uint32_t data_type) {

  UncRespCode ret_code = UNC_RC_SUCCESS;
  pfc_ipcresp_t mandatory = PFC_TRUE;

  // Validate key structure
  key_boundary_t *key = reinterpret_cast<key_boundary_t*>(key_struct);
  string value = reinterpret_cast<char*>(key->boundary_id);
  map<string, Kt_Class_Attr_Syntax> attr_syntax_map =
      attr_syntax_map_all[UNC_KT_BOUNDARY];
  IS_VALID_STRING_KEY(BDRY_ID_STR, value, operation, ret_code, mandatory);
  if (ret_code != UNC_RC_SUCCESS) {
    return UNC_UPPL_RC_ERR_CFG_SYNTAX;
  }

  // Validate value structure
  if (val_struct != NULL) {
    unsigned int valid_val = 0;

    // validate description
    val_boundary_t *val_bdry = reinterpret_cast<val_boundary_t*>(val_struct);
    valid_val =
        PhyUtil::uint8touint(val_bdry->valid[kIdxBoundaryDescription]);
    string value = reinterpret_cast<char*>(val_bdry->description);
    IS_VALID_STRING_VALUE(BDRY_DESCRIPTION_STR, value, operation,
                          valid_val, ret_code, mandatory);
    if (ret_code != UNC_RC_SUCCESS) {
      return UNC_UPPL_RC_ERR_CFG_SYNTAX;
    }

    // validate controller_name1
    unsigned int ctr1_valid_val =
        PhyUtil::uint8touint(val_bdry->valid[kIdxBoundaryControllerName1]);
    value = reinterpret_cast<char*>(val_bdry->controller_name1);
    IS_VALID_STRING_VALUE(BDRY_CTR_NAME1_STR, value, operation,
                          ctr1_valid_val, ret_code, mandatory);
    if (ret_code != UNC_RC_SUCCESS) {
      return UNC_UPPL_RC_ERR_CFG_SYNTAX;
    }
    if (operation == UNC_OP_UPDATE && ctr1_valid_val == UNC_VF_VALID) {
      pfc_log_error("Controllername1 cannot be modified");
      return UNC_UPPL_RC_ERR_CFG_SYNTAX;
    }

    // validate domain_name1
    unsigned int dmn1_valid_val =
        PhyUtil::uint8touint(val_bdry->valid[kIdxBoundaryDomainName1]);
    value = reinterpret_cast<char*>(val_bdry->domain_name1);
    IS_VALID_STRING_VALUE(BDRY_DM_NAME1_STR, value, operation,
                          dmn1_valid_val, ret_code, mandatory);
    if (ret_code != UNC_RC_SUCCESS) {
      return UNC_UPPL_RC_ERR_CFG_SYNTAX;
    }
    if (operation == UNC_OP_UPDATE && dmn1_valid_val == UNC_VF_VALID) {
      pfc_log_error("Domainname1 cannot be modified");
      return UNC_UPPL_RC_ERR_CFG_SYNTAX;
    }

    // validate logical_port_id1
    valid_val =
        PhyUtil::uint8touint(val_bdry->valid[kIdxBoundaryLogicalPortId1]);
    value = reinterpret_cast<char*>(val_bdry->logical_port_id1);
    IS_VALID_STRING_VALUE(BDRY_PORT_ID1_STR, value, operation,
                          valid_val, ret_code, mandatory);
    if (ret_code != UNC_RC_SUCCESS) {
      return UNC_UPPL_RC_ERR_CFG_SYNTAX;
    }
    if (operation == UNC_OP_UPDATE && valid_val == UNC_VF_VALID) {
      pfc_log_error("LogicalPortId1 cannot be modified");
      return UNC_UPPL_RC_ERR_CFG_SYNTAX;
    }

    // validate controller_name2
    unsigned int ctr2_valid_val =
        PhyUtil::uint8touint(val_bdry->valid[kIdxBoundaryControllerName2]);
    value = reinterpret_cast<char*>(val_bdry->controller_name2);
    IS_VALID_STRING_VALUE(BDRY_CTR_NAME2_STR, value, operation,
                          ctr2_valid_val, ret_code, mandatory);
    if (ret_code != UNC_RC_SUCCESS) {
      return UNC_UPPL_RC_ERR_CFG_SYNTAX;
    }
    if (operation == UNC_OP_UPDATE && ctr2_valid_val == UNC_VF_VALID) {
      pfc_log_error("Controllername2 cannot be modified");
      return UNC_UPPL_RC_ERR_CFG_SYNTAX;
    }

    // validate domain_name2
    unsigned int dmn2_valid_val =
        PhyUtil::uint8touint(val_bdry->valid[kIdxBoundaryDomainName2]);
    value = reinterpret_cast<char*>(val_bdry->domain_name2);
    IS_VALID_STRING_VALUE(BDRY_DM_NAME2_STR, value, operation,
                          dmn2_valid_val, ret_code, mandatory);
    if (ret_code != UNC_RC_SUCCESS) {
      return UNC_UPPL_RC_ERR_CFG_SYNTAX;
    }
    if (operation == UNC_OP_UPDATE && dmn2_valid_val == UNC_VF_VALID) {
      pfc_log_error("Domainname2 cannot be modified");
      return UNC_UPPL_RC_ERR_CFG_SYNTAX;
    }

    // validate logical_port_id2
    valid_val =
        PhyUtil::uint8touint(val_bdry->valid[kIdxBoundaryLogicalPortId2]);
    value = reinterpret_cast<char*>(val_bdry->logical_port_id2);
    IS_VALID_STRING_VALUE(BDRY_PORT_ID2_STR, value, operation,
                          valid_val, ret_code, mandatory);
    if (ret_code != UNC_RC_SUCCESS) {
      return UNC_UPPL_RC_ERR_CFG_SYNTAX;
    }
    if (operation == UNC_OP_UPDATE && valid_val == UNC_VF_VALID) {
      pfc_log_error("LogicalPortId2 cannot be modified");
      return UNC_UPPL_RC_ERR_CFG_SYNTAX;
    }

    if (operation == UNC_OP_READ_SIBLING_BEGIN ||
        operation == UNC_OP_READ_SIBLING) {
      UncRespCode validate_ret = ValidateSiblingFiltering(ctr1_valid_val,
                                                             ctr2_valid_val,
                                                             dmn1_valid_val,
                                                             dmn2_valid_val);
      if (validate_ret != UNC_RC_SUCCESS) {
        return validate_ret;
      }
    }
  }
  return ret_code;
}

/** PerformSemanticValidation
 * @Description : This function performs semantic validation
 * for UNC_KT_BOUNDARY
 * @param[in] : key_struct - specifies key instance of KT_Boundary
 * value_struct - specifies value of KT_BOUNDARY
 * operation - UNC_OP*,type of operation
 * data_type - UNC_DT*,type of database
 * @return    : UNC_RC_SUCCESS if semantic valition is successful
 * or UNC_UPPL_RC_ERR_* if failed
 * */
UncRespCode Kt_Boundary::PerformSemanticValidation(
    OdbcmConnectionHandler *db_conn,
    void* key_struct,
    void* val_struct,
    uint32_t operation,
    uint32_t data_type) {
  UncRespCode status = UNC_RC_SUCCESS;
  pfc_log_debug("PerformSemanticValidation:KT_BOUNDARY");
  key_boundary_t *obj_key_boundary
  = reinterpret_cast<key_boundary_t*>(key_struct);
  string boundary_id = (const char*)obj_key_boundary->boundary_id;
  vector<string> boundary_vect_key_value;
  boundary_vect_key_value.push_back(boundary_id);
  UncRespCode key_status = IsKeyExists(db_conn,
                                          (unc_keytype_datatype_t)data_type,
                                          boundary_vect_key_value);
  pfc_log_debug("IsKeyExists status %d", key_status);
  // In case of create operation, key should not exist
  if (operation == UNC_OP_CREATE) {
    if (key_status == UNC_RC_SUCCESS) {
      pfc_log_error("Key exist,CREATE not allowed");
      status = UNC_UPPL_RC_ERR_INSTANCE_EXISTS;
    } else if (key_status == UNC_UPPL_RC_ERR_DB_ACCESS) {
      pfc_log_error("DB Access failure");
      status = key_status;
    }
  } else if (operation == UNC_OP_UPDATE || operation == UNC_OP_DELETE ||
      operation == UNC_OP_READ) {
    // In case of update/delete/read operation, key should exist
    if (key_status == UNC_UPPL_RC_ERR_DB_ACCESS) {
      pfc_log_error("DB Access failure");
      status = key_status;
    } else if (key_status != UNC_RC_SUCCESS) {
      pfc_log_error("Key doesn't exist,U/D/R oprn not allowed");
      status = UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE;
    } else {
      pfc_log_debug("key instance exist update/del/read operation allowed");
    }
  }
  if (status != UNC_RC_SUCCESS) {
    pfc_log_debug("Return Code SemanticValidation: %d", status);
    return status;
  }
  if (operation != UNC_OP_CREATE && operation != UNC_OP_UPDATE) {
    pfc_log_debug("Return Code SemanticValidation: %d", status);
    return status;
  }
  // verification of controller type and boundary logical port value
  val_boundary_t *obj_boundary_val =
      reinterpret_cast<val_boundary_t*>(val_struct);
  if (obj_boundary_val == NULL) {
    pfc_log_debug("Return Code SemanticValidation: %d", status);
    return status;
  }
  pfc_log_debug("Validate Controller Existence and Logical port validity");
  unsigned int ctr1_valid_val =
      PhyUtil::uint8touint(obj_boundary_val->
                           valid[kIdxBoundaryControllerName1]);
  unsigned int lp1_valid_val =
      PhyUtil::uint8touint(obj_boundary_val->
                           valid[kIdxBoundaryLogicalPortId1]);
  unsigned int dmn1_valid_val =
      PhyUtil::uint8touint(obj_boundary_val->
                           valid[kIdxBoundaryDomainName1]);
  string controller_name1 = "";
  if (ctr1_valid_val == UNC_VF_VALID) {
    controller_name1 = (const char*)obj_boundary_val->controller_name1;
    unc_keytype_ctrtype_t type = UNC_CT_UNKNOWN;
    UncRespCode ctr_type_status =
        PhyUtil::get_controller_type(db_conn, controller_name1,
                                     type,
                                     (unc_keytype_datatype_t) data_type);
    if (ctr_type_status !=  UNC_RC_SUCCESS) {
      pfc_log_error(
          "Operation %d is not allowed as controller1 instance %s not exists",
          operation, controller_name1.c_str());
      return UNC_UPPL_RC_ERR_CFG_SEMANTIC;
    }
    pfc_log_debug("Controller1 type: %d", type);
    if (type != (unc_keytype_ctrtype_t) UNC_CT_UNKNOWN) {
      pfc_log_debug("logical port valid %d", lp1_valid_val);
      string logical_port_id1 =
          (const char*)obj_boundary_val->logical_port_id1;
      if (lp1_valid_val == UNC_VF_INVALID || logical_port_id1.empty()) {
        pfc_log_info("logical port not valid/null for controller %s",
                     controller_name1.c_str());
        return UNC_UPPL_RC_ERR_CFG_SEMANTIC;
      }
    } else {
      // validate domain name
      pfc_log_debug("Domain1 valid %d", dmn1_valid_val);
      string domain_name1 =
          (const char*)obj_boundary_val->domain_name1;
      if (dmn1_valid_val == UNC_VF_VALID && domain_name1 == DEFAULT_DOMAIN) {
        pfc_log_info("Default domain cannot be given for bypass controller %s",
                     controller_name1.c_str());
        return UNC_UPPL_RC_ERR_CFG_SEMANTIC;
      }
    }
  }
  unsigned int ctr2_valid_val =
      PhyUtil::uint8touint(obj_boundary_val->
                           valid[kIdxBoundaryControllerName2]);
  unsigned int lp2_valid_val =
      PhyUtil::uint8touint(obj_boundary_val->
                           valid[kIdxBoundaryLogicalPortId2]);
  unsigned int dmn2_valid_val =
      PhyUtil::uint8touint(obj_boundary_val->
                           valid[kIdxBoundaryDomainName2]);
  string controller_name2;
  if (ctr2_valid_val == UNC_VF_VALID) {
    controller_name2 = (const char*)obj_boundary_val->controller_name2;
    unc_keytype_ctrtype_t type = UNC_CT_UNKNOWN;
    UncRespCode ctr_type_status =
        PhyUtil::get_controller_type(db_conn, controller_name2,
                                     type,
                                     (unc_keytype_datatype_t) data_type);
    if (ctr_type_status != UNC_RC_SUCCESS) {
      pfc_log_error(
          "Operation %d is not allowed as controller instance %s not exists",
          operation, controller_name2.c_str());
      return UNC_UPPL_RC_ERR_CFG_SEMANTIC;
    }
    pfc_log_debug("Controller2 type :%d ", type);
    if (type != (unc_keytype_ctrtype_t) UNC_CT_UNKNOWN) {
      pfc_log_debug("logical port valid %d", lp2_valid_val);
      string logical_port_id2 =
          (const char*)obj_boundary_val->logical_port_id2;
      if (lp2_valid_val == UNC_VF_INVALID || logical_port_id2.empty()) {
        pfc_log_info("logicalport2 not valid/null for controller %s",
                     controller_name2.c_str());
        return UNC_UPPL_RC_ERR_CFG_SEMANTIC;
      }
    } else {
      // validate domain name
      pfc_log_debug("Domain2 valid %d", dmn2_valid_val);
      string domain_name2 =
          (const char*)obj_boundary_val->domain_name2;
      if (dmn2_valid_val == UNC_VF_VALID && domain_name2 == DEFAULT_DOMAIN) {
        pfc_log_info("Default domain cannot be given for bypass controller %s",
                     controller_name2.c_str());
        return UNC_UPPL_RC_ERR_CFG_SEMANTIC;
      }
    }
  }

  string domain_name1;
  if (dmn1_valid_val == UNC_VF_VALID) {
    domain_name1 = (const char*)obj_boundary_val->domain_name1;
  }
  string domain_name2;
  if (dmn2_valid_val == UNC_VF_VALID) {
    domain_name2 = (const char*)obj_boundary_val->domain_name2;
  }
  if (ctr1_valid_val != UNC_VF_VALID ||
      ctr2_valid_val != UNC_VF_VALID ||
      dmn1_valid_val != UNC_VF_VALID ||
      dmn2_valid_val != UNC_VF_VALID) {
    pfc_log_debug("Return Code SemanticValidation: %d", status);
    return status;
  }
  if (controller_name1 == controller_name2 &&
      domain_name1 == domain_name2) {
    pfc_log_error(
        "Controller name1, Controller name2 "
        "and domain name1, domain name2 are same");
    status = UNC_UPPL_RC_ERR_CFG_SEMANTIC;
    pfc_log_debug("Return Code SemanticValidation: %d", status);
    return status;
  }
  status = CheckBoundaryExistence(db_conn, key_struct, val_struct, data_type);
  pfc_log_debug("Return Code SemanticValidation: %d", status);
  return status;
}


/** PopulateDBSchemaForKtTable
 * @Description : This function populates the DBAttrSchema to be used to
 * send request to ODBC
 * @param[in] : 
 * kt_boundary_dbtableschema-DBTableSchema instance of Boundary to be filled
 * key_struct-void* to bdry key structure
 * val_struct-void* to bdry val structure
 * operation_type-UNC_OP_*,type of operation
 * option1,option2-UNC_OPT1/OPT2_*,additional options for read operation
 * vect_key_operations-vector<ODBCMOperator> instance
 * row_status-CsRowStatus value
 * is_filtering-flag to indicate whether filter option enabled
 * is_state-flag to indicate whether datatype is ST_STATE
 * @return : void
 **/
void Kt_Boundary::PopulateDBSchemaForKtTable(
    OdbcmConnectionHandler *db_conn,
    DBTableSchema &kt_boundary_dbtableschema,
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
  pfc_log_debug("Populate Kt_boundary");
  // Construct Primary key list
  vector<string> vect_prim_keys;

  // construct TableAttrSchema structure
  // TableAttrSchema holds table_name, primary key, attr_name
  vector<TableAttrSchema> vect_table_attr_schema;
  list < vector<TableAttrSchema> > row_list;

  key_boundary_t* obj_key_boundary =
      reinterpret_cast<key_boundary_t*>(key_struct);
  val_boundary_t* obj_val_boundary =
      reinterpret_cast<val_boundary_t*>(val_struct);

  stringstream valid;
  uint16_t valid_val = 0, prev_db_val = 0;

  string boundary_id = "";
  if (obj_key_boundary != NULL) {
    boundary_id = (const char*)obj_key_boundary->boundary_id;
  }
  if (operation_type == UNC_OP_READ_SIBLING_BEGIN ||
      operation_type == UNC_OP_READ_SIBLING_COUNT) {
    // Ignore boundary_id
    boundary_id = "";
  }
  val_boundary_st_t val_boundary_valid_st;
  memset(&val_boundary_valid_st, 0, sizeof(val_boundary_st_t));
  if (operation_type == UNC_OP_UPDATE) {
    // get valid array for update request
    pfc_log_debug("Get Valid value from DB");
    GetBoundaryValidFlag(db_conn, key_struct, val_boundary_valid_st, data_type);
  }
  string value;
  // description
  if (obj_val_boundary != NULL) {
    valid_val = PhyUtil::uint8touint(obj_val_boundary->
                                     valid[kIdxBoundaryDescription]);
    value = (const char*)obj_val_boundary->description;
    prev_db_val =
        PhyUtil::uint8touint(
            val_boundary_valid_st.boundary.valid[kIdxBoundaryDescription]);
  } else {
    valid_val = UPPL_NO_VAL_STRUCT;
  }
  PhyUtil::FillDbSchema(unc::uppl::BDRY_DESCRIPTION, BDRY_DESCRIPTION_STR,
                        value, value.length(), DATATYPE_UINT8_ARRAY_128,
                        operation_type, valid_val, prev_db_val,
                        vect_table_attr_schema, vect_prim_keys, valid);
  value.clear();
  // controller_name1
  if (obj_val_boundary != NULL) {
    valid_val = PhyUtil::uint8touint(obj_val_boundary
                                     ->valid[kIdxBoundaryControllerName1]);
    value = (const char*)obj_val_boundary->controller_name1;
    prev_db_val =
        PhyUtil::uint8touint(
            val_boundary_valid_st.boundary.valid[kIdxBoundaryControllerName1]);
  } else {
    valid_val = UPPL_NO_VAL_STRUCT;
  }
  PhyUtil::FillDbSchema(unc::uppl::BDRY_CTR_NAME1, BDRY_CTR_NAME1_STR, value,
                        value.length(), DATATYPE_UINT8_ARRAY_32,
                        operation_type, valid_val, prev_db_val,
                        vect_table_attr_schema, vect_prim_keys, valid);
  value.clear();
  // domain_name1
  if (obj_val_boundary != NULL) {
    valid_val = PhyUtil::uint8touint(obj_val_boundary
                                     ->valid[kIdxBoundaryDomainName1]);
    value = (const char*)obj_val_boundary->domain_name1;
    prev_db_val =
        PhyUtil::uint8touint(
            val_boundary_valid_st.boundary.valid[kIdxBoundaryDomainName1]);
  } else {
    valid_val = UPPL_NO_VAL_STRUCT;
  }
  PhyUtil::FillDbSchema(unc::uppl::BDRY_DM_NAME1, BDRY_DM_NAME1_STR, value,
                        value.length(), DATATYPE_UINT8_ARRAY_32,
                        operation_type, valid_val, prev_db_val,
                        vect_table_attr_schema, vect_prim_keys, valid);
  value.clear();
  // logical_port_id1
  if (obj_val_boundary != NULL) {
    valid_val = PhyUtil::uint8touint(obj_val_boundary
                                     ->valid[kIdxBoundaryLogicalPortId1]);
    value = (const char*)obj_val_boundary->logical_port_id1;
    prev_db_val =
        PhyUtil::uint8touint(
            val_boundary_valid_st.boundary.valid[kIdxBoundaryLogicalPortId1]);
  } else {
    valid_val = UPPL_NO_VAL_STRUCT;
  }
  PhyUtil::FillDbSchema(unc::uppl::BDRY_PORT_ID1, BDRY_PORT_ID1_STR, value,
                        value.length(), DATATYPE_UINT8_ARRAY_320,
                        operation_type, valid_val, prev_db_val,
                        vect_table_attr_schema, vect_prim_keys, valid);
  value.clear();
  // controller_name2
  if (obj_val_boundary != NULL) {
    valid_val = PhyUtil::uint8touint(obj_val_boundary
                                     ->valid[kIdxBoundaryControllerName2]);
    value = (const char*)obj_val_boundary->controller_name2;
    prev_db_val =
        PhyUtil::uint8touint(
            val_boundary_valid_st.boundary.valid[kIdxBoundaryControllerName2]);
  } else {
    valid_val = UPPL_NO_VAL_STRUCT;
  }
  PhyUtil::FillDbSchema(unc::uppl::BDRY_CTR_NAME2, BDRY_CTR_NAME2_STR, value,
                        value.length(), DATATYPE_UINT8_ARRAY_32,
                        operation_type, valid_val, prev_db_val,
                        vect_table_attr_schema, vect_prim_keys, valid);
  value.clear();
  // domain_name2
  if (obj_val_boundary != NULL) {
    valid_val = PhyUtil::uint8touint(obj_val_boundary
                                     ->valid[kIdxBoundaryDomainName2]);
    value = (const char*)obj_val_boundary->domain_name2;
    prev_db_val =
        PhyUtil::uint8touint(
            val_boundary_valid_st.boundary.valid[kIdxBoundaryDomainName2]);
  } else {
    valid_val = UPPL_NO_VAL_STRUCT;
  }
  PhyUtil::FillDbSchema(unc::uppl::BDRY_DM_NAME2, BDRY_DM_NAME2_STR, value,
                        value .length(), DATATYPE_UINT8_ARRAY_32,
                        operation_type, valid_val, prev_db_val,
                        vect_table_attr_schema, vect_prim_keys, valid);
  value.clear();
  // logical_port_id2
  if (obj_val_boundary != NULL) {
    valid_val = PhyUtil::uint8touint(obj_val_boundary
                                     ->valid[kIdxBoundaryLogicalPortId2]);
    value = (const char*)obj_val_boundary->logical_port_id2;
    prev_db_val =
        PhyUtil::uint8touint(
            val_boundary_valid_st.boundary.valid[kIdxBoundaryLogicalPortId2]);
  } else {
    valid_val = UPPL_NO_VAL_STRUCT;
  }
  PhyUtil::FillDbSchema(unc::uppl::BDRY_PORT_ID2, BDRY_PORT_ID2_STR, value,
                        value.length(), DATATYPE_UINT8_ARRAY_320,
                        operation_type, valid_val, prev_db_val,
                        vect_table_attr_schema, vect_prim_keys, valid);

  vect_prim_keys.push_back(BDRY_ID_STR);
  PhyUtil::FillDbSchema(unc::uppl::BDRY_ID, boundary_id, boundary_id.length(),
                        DATATYPE_UINT8_ARRAY_32, vect_table_attr_schema);

  // oper status
  if (obj_val_boundary != NULL) {
    valid_val = UPPL_NO_VAL_STRUCT;
    value = "";
    prev_db_val = 0;
  }
  PhyUtil::FillDbSchema(unc::uppl::BDRY_OPER_STATUS, BDRY_OPER_STATUS_STR,
                        value, value.length(), DATATYPE_UINT16,
                        operation_type, valid_val, prev_db_val,
                        vect_table_attr_schema, vect_prim_keys, valid);

  // valid
  valid_val = UPPL_NO_VAL_STRUCT;
  prev_db_val = 0;
  stringstream dummy_valid;
  PhyUtil::FillDbSchema(unc::uppl::BDRY_VALID, BDRY_VALID_STR, valid.str(),
                        valid.str().length(), DATATYPE_UINT8_ARRAY_8,
                        operation_type, valid_val, prev_db_val,
                        vect_table_attr_schema, vect_prim_keys, dummy_valid);

  stringstream attr_status;
  for (unsigned int index = 0; index < ODBCM_SIZE_8; ++index) {
    attr_status << CREATED;
  }
  PhyUtil::FillDbSchema(unc::uppl::BDRY_ATTR, BDRY_ATTR_STR, attr_status.str(),
                        attr_status.str().length(), DATATYPE_UINT8_ARRAY_8,
                        operation_type, valid_val, prev_db_val,
                        vect_table_attr_schema, vect_prim_keys, dummy_valid);
  // cs_row status
  if (is_filtering == true) {
    vect_prim_keys.push_back(BDRY_ROW_STATUS_STR);
  }
  value = PhyUtil::uint8tostr(row_status);
  if (operation_type >= UNC_OP_READ) {
    PhyUtil::FillDbSchema(unc::uppl::BDRY_ROW_STATUS, value,
                          value.length(), DATATYPE_UINT16,
                          vect_table_attr_schema);
  }
  PhyUtil::reorder_col_attrs(vect_prim_keys, vect_table_attr_schema);

  kt_boundary_dbtableschema.set_table_name(unc::uppl::BOUNDARY_TABLE);
  kt_boundary_dbtableschema.set_primary_keys(vect_prim_keys);
  row_list.push_back(vect_table_attr_schema);
  kt_boundary_dbtableschema.set_row_list(row_list);
  return;
}

/** FillBoundaryValueStructure
 * @Description : This function populates val_boundary_t by values retrieved
 * from database
 * @param[in]: kt_boundary_dbtableschema-boundary table dbtable schema 
 * to be filled
 * vect_obj_key_boundary-vector<key_boundary_t> instance
 * vect_obj_val_boundary-vector<val_boundary_st_t> instance
 * max_rep_ct-max no of records to be read
 * @return : void
 **/

void Kt_Boundary::FillBoundaryValueStructure(
    OdbcmConnectionHandler *db_conn,
    DBTableSchema &kt_boundary_dbtableschema,
    vector<key_boundary_t> &vect_obj_key_boundary,
    vector<val_boundary_st_t> &vect_obj_val_boundary,
    uint32_t &max_rep_ct) {
  //  populate IPC value structure based on the response received from DB
  list < vector<TableAttrSchema> >& res_boundary_row_list =
      kt_boundary_dbtableschema.get_row_list();
  list < vector<TableAttrSchema> > :: iterator res_boundary_iter =
      res_boundary_row_list.begin();

  max_rep_ct = res_boundary_row_list.size();
  pfc_log_debug("res_boundary_row_list.size: %d", max_rep_ct);
  // populate IPC value structure based on the response received from DB
  for (; res_boundary_iter != res_boundary_row_list.end();
      ++res_boundary_iter) {
    vector<TableAttrSchema> res_boundary_table_attr_schema =
        (*res_boundary_iter);
    vector<TableAttrSchema> :: iterator vect_boundary_iter =
        res_boundary_table_attr_schema.begin();
    key_boundary_t obj_key_boundary;
    memset(&obj_key_boundary, 0, sizeof(obj_key_boundary));

    val_boundary_t obj_val_boundary;
    memset(&obj_val_boundary, 0, sizeof(val_boundary_t));
    memset(obj_val_boundary.valid, '\0', 7);
    memset(obj_val_boundary.cs_attr, '\0', 7);
    val_boundary_st_t obj_val_boundary_st;
    memset(obj_val_boundary_st.valid, '\0', 2);
    // Read all attributes
    for (; vect_boundary_iter != res_boundary_table_attr_schema.end();
        ++vect_boundary_iter) {
      //  Populate values from boundary_table
      TableAttrSchema tab_schema = (*vect_boundary_iter);
      ODBCMTableColumns attr_name = tab_schema.table_attribute_name;
      string attr_value;
      switch (attr_name) {
        case unc::uppl::BDRY_ID:
          PhyUtil::GetValueFromDbSchemaStr(tab_schema,
                                           obj_key_boundary.boundary_id,
                                           DATATYPE_UINT8_ARRAY_32);
          vect_obj_key_boundary.push_back(obj_key_boundary);
          pfc_log_debug("boundary_id: %s",
                        obj_key_boundary.boundary_id);
          break;

        case unc::uppl::BDRY_DESCRIPTION:
          PhyUtil::GetValueFromDbSchemaStr(tab_schema,
                                           obj_val_boundary.description,
                                           DATATYPE_UINT8_ARRAY_128);
          pfc_log_debug("description: %s", obj_val_boundary.description);
          break;

        case unc::uppl::BDRY_CTR_NAME1:
          PhyUtil::GetValueFromDbSchemaStr(tab_schema,
                                           obj_val_boundary.controller_name1,
                                           DATATYPE_UINT8_ARRAY_32);
          pfc_log_debug("controller_name1: %s",
                        obj_val_boundary.controller_name1);
          break;

        case unc::uppl::BDRY_DM_NAME1:
          PhyUtil::GetValueFromDbSchemaStr(tab_schema,
                                           obj_val_boundary.domain_name1,
                                           DATATYPE_UINT8_ARRAY_32);
          pfc_log_debug("domain_name1: %s", obj_val_boundary.domain_name1);
          break;

        case unc::uppl::BDRY_PORT_ID1:
          PhyUtil::GetValueFromDbSchemaStr(tab_schema,
                                           obj_val_boundary.logical_port_id1,
                                           DATATYPE_UINT8_ARRAY_320);
          pfc_log_debug("logical_port_id1: %s",
                        obj_val_boundary.logical_port_id1);
          break;

        case unc::uppl::BDRY_CTR_NAME2:
          PhyUtil::GetValueFromDbSchemaStr(tab_schema,
                                           obj_val_boundary.controller_name2,
                                           DATATYPE_UINT8_ARRAY_32);
          pfc_log_debug("controller_name2: %s",
                        obj_val_boundary.controller_name2);
          break;

        case unc::uppl::BDRY_DM_NAME2:
          PhyUtil::GetValueFromDbSchemaStr(tab_schema,
                                           obj_val_boundary.domain_name2,
                                           DATATYPE_UINT8_ARRAY_32);
          pfc_log_debug("domain_name2: %s", obj_val_boundary.domain_name2);
          break;

        case unc::uppl::BDRY_PORT_ID2:
          PhyUtil::GetValueFromDbSchemaStr(tab_schema,
                                           obj_val_boundary.logical_port_id2,
                                           DATATYPE_UINT8_ARRAY_320);
          pfc_log_debug("logical_port_id2: %s"
              , obj_val_boundary.logical_port_id2);
          break;

        case unc::uppl::BDRY_VALID:
          uint8_t bdr_valid[ODBCM_SIZE_8];
          PhyUtil::GetValueFromDbSchemaStr(tab_schema, bdr_valid,
                                           DATATYPE_UINT8_ARRAY_8);
          memset(obj_val_boundary.valid, '\0', 7);
          FrameValidValue(reinterpret_cast<const char*> (bdr_valid),
                          obj_val_boundary_st, obj_val_boundary);
          pfc_log_debug("valid: %s", bdr_valid);
          break;

        case unc::uppl::BDRY_ATTR:
          uint8_t bdr_attr[ODBCM_SIZE_8];
          PhyUtil::GetValueFromDbSchemaStr(tab_schema, bdr_attr,
                                           DATATYPE_UINT8_ARRAY_8);
          memset(obj_val_boundary.cs_attr, '\0', 7);
          FrameCsAttrValue(reinterpret_cast<const char*> (bdr_attr),
                           obj_val_boundary);
          pfc_log_debug("cs_attr: %s", bdr_attr);
          break;

        case unc::uppl::BDRY_ROW_STATUS:
          PhyUtil::GetValueFromDbSchema(tab_schema, attr_value,
                                        DATATYPE_UINT16);
          obj_val_boundary.cs_row_status = atoi(attr_value.c_str());
          pfc_log_debug("row_status: %s", attr_value.c_str());
          break;

        case unc::uppl::BDRY_OPER_STATUS:
          PhyUtil::GetValueFromDbSchema(tab_schema, attr_value,
                                        DATATYPE_UINT16);
          obj_val_boundary_st.oper_status = atoi(attr_value.c_str());
          pfc_log_debug("oper_status: %s", attr_value.c_str());
          break;

        default:
          pfc_log_debug("Ignoring Boundary attribute %d", attr_name);
          break;
      }
    }
    obj_val_boundary_st.boundary = obj_val_boundary;
    vect_obj_val_boundary.push_back(obj_val_boundary_st);
    pfc_log_debug("result - vect_obj_val_boundary size: %"
                   PFC_PFMT_SIZE_T, vect_obj_val_boundary.size());
  }
  return;
}

/** PerformRead
 * @Description : This function reads the instance of KT_Boundary based on
 * operation type - UNC_OP_READ, UNC_OP_READ_SIBLING_BEGIN,
 * UNC_OP_READ_SIBLING
 * @param[in] : key_struct-void* to bdry key structure
 * value_struct-void* to bdry value structure
 * session_id-ipc session id used for TC validation
 * configuration_id -ipc configuration id used for TC validation
 * operation_type-UNC_OP_*
 * sess-object of ServerSession
 * option1,option2-additional info for read opeartions,UNC_OPT1/OPT2_*
 * max_rep_ct-max no of records to be read
 * @return   : Success or associated error code
 * UNC_RC_SUCCESS is returned when the response
 * is added to ipc session successfully.
 * UNC_UPPL_RC_ERR_* is returned when ipc response could not be added to sess.
 **/
UncRespCode Kt_Boundary::PerformRead(OdbcmConnectionHandler *db_conn,
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
  pfc_log_debug("PerformRead oper=%d dt=%d",
               operation_type, data_type);
  key_boundary_t *obj_boundary = reinterpret_cast<key_boundary_t*>(key_struct);
  UncRespCode read_status = UNC_RC_SUCCESS;
  physical_response_header rsh = {session_id,
      configuration_id,
      operation_type,
      max_rep_ct,
      option1,
      option2,
      data_type,
      static_cast<uint32_t>(0)};
  if (option1 != UNC_OPT1_NORMAL) {
    pfc_log_error("Invalid option1 specified for read operation");
    rsh.result_code = UNC_UPPL_RC_ERR_INVALID_OPTION1;
    int err = PhyUtil::sessOutRespHeader(sess, rsh);
    err |= sess.addOutput((uint32_t)UNC_KT_BOUNDARY);
    err |= sess.addOutput(*obj_boundary);
    if (err != 0) {
      pfc_log_info("addOutput failed for physical_response_header");
      return UNC_UPPL_RC_ERR_IPC_WRITE_ERROR;
    }
    return UNC_RC_SUCCESS;
  }
  if (operation_type == UNC_OP_READ) {
    max_rep_ct = 1;
  }
  if ((unc_keytype_datatype_t)data_type != UNC_DT_CANDIDATE &&
      (unc_keytype_datatype_t)data_type != UNC_DT_RUNNING &&
      (unc_keytype_datatype_t)data_type != UNC_DT_STARTUP &&
      (unc_keytype_datatype_t)data_type != UNC_DT_STATE &&
      (unc_keytype_datatype_t)data_type != UNC_DT_IMPORT) {
    pfc_log_error("Data type is not allowed");
    rsh.result_code = UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED;
    int err = PhyUtil::sessOutRespHeader(sess, rsh);
    err |= sess.addOutput((uint32_t)UNC_KT_BOUNDARY);
    err |= sess.addOutput(*obj_boundary);
    if (err != 0) {
      pfc_log_error("addOutput failed for physical_response_header");
      return UNC_UPPL_RC_ERR_IPC_WRITE_ERROR;
    }
    return UNC_RC_SUCCESS;
  }
  vector<key_boundary_t> vect_key_boundary;
  vector<val_boundary_st_t> vect_val_boundary_st;
  read_status = ReadBoundaryValFromDB(db_conn,
                                      key_struct,
                                      val_struct,
                                      data_type,
                                      operation_type,
                                      max_rep_ct,
                                      vect_key_boundary,
                                      vect_val_boundary_st);
  rsh.result_code = read_status;
  rsh.max_rep_count = max_rep_ct;
  if (read_status == UNC_RC_SUCCESS) {
    for (unsigned int index = 0;
        index < vect_key_boundary.size();
        ++index) {
      string bId = (const char *)vect_key_boundary[index].boundary_id;
      if (data_type == UNC_DT_CANDIDATE &&
          vect_val_boundary_st[index].boundary.cs_row_status == DELETED) {
        pfc_log_debug("Ignoring DELETED entry %s", bId.c_str());
        if (rsh.max_rep_count > 0)
          rsh.max_rep_count--;
        continue;
      }
    }
    if (rsh.max_rep_count == 0) {
      rsh.result_code = UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE;
      int err = PhyUtil::sessOutRespHeader(sess, rsh);
      err |= sess.addOutput((uint32_t)UNC_KT_BOUNDARY);
      err |= sess.addOutput(*obj_boundary);
      if (err != 0) {
        pfc_log_error("addOutput failed for physical_response_header");
        return UNC_UPPL_RC_ERR_IPC_WRITE_ERROR;
      }
      return UNC_RC_SUCCESS;
    }
    int err = PhyUtil::sessOutRespHeader(sess, rsh);
    if (err != 0) {
      pfc_log_error("Failure in addOutput");
      return UNC_UPPL_RC_ERR_IPC_WRITE_ERROR;
    }
    pfc_log_debug("From db, vect_boundary size is %"
                  PFC_PFMT_SIZE_T, vect_key_boundary.size());
    for (unsigned int index = 0;
        index < vect_key_boundary.size();
        ++index) {
      string bId = (const char *)vect_key_boundary[index].boundary_id;
      if (data_type == UNC_DT_CANDIDATE &&
          vect_val_boundary_st[index].boundary.cs_row_status == DELETED) {
        pfc_log_debug("Ignoring DELETED entry %s", bId.c_str());
        continue;
      }
      pfc_log_debug("Adding entry %s", bId.c_str());
      sess.addOutput((uint32_t)UNC_KT_BOUNDARY);
      sess.addOutput((key_boundary_t)vect_key_boundary[index]);
      if ((unc_keytype_datatype_t)data_type == UNC_DT_STATE) {
        sess.addOutput(vect_val_boundary_st[index]);
      } else {
        sess.addOutput(vect_val_boundary_st[index].boundary);
      }
      if (index < vect_key_boundary.size()-1) {
        sess.addOutput();  // Separator
      }
    }
  } else {
    rsh.max_rep_count = 0;
    int err = PhyUtil::sessOutRespHeader(sess, rsh);
    pfc_log_error("Read operation failed with %d", read_status);
    err |= sess.addOutput((uint32_t)UNC_KT_BOUNDARY);
    err |= sess.addOutput(*obj_boundary);
    if (err != 0) {
      pfc_log_info("addOutput failure for physical_reponse_header");
      return UNC_UPPL_RC_ERR_IPC_WRITE_ERROR;
    }
  }
  pfc_log_debug("Return value for read operation %d", read_status);
  return read_status;
}

/** ReadBoundaryValFromDB
 * @Description : This function reads the instance of KT_Boundary based on
 * operation type - UNC_OP_READ, UNC_OP_READ_SIBLING_BEGIN,
 * UNC_OP_READ_SIBLING
 * @param[in] : key_struct-void* to bdry key structure
 * value_struct-void* to bdry val structure
 * option1,option2-UNC_OPT1/OPT2_*,additional info for read operation
 * data_type-UNC_DT_*,type of database
 * operation type-UNC_OP_*,type of operation 
 * max_rep_ct-max no of records to be read
 * vect_key_boundary-instance of vector<key_boundary_t>
 * vect_val_boundary-instance of vector<val_boundary_t>
 * vect_val_boundary_st-instance of vector<val_boundary_st_t> 
 * is_state-flag to indicate whether data type is DT_STATE
 * @return    : Success or associated error code
 * UNC_RC_SUCCESS is returned when the response
 * is added to ipc session successfully.
 * UNC_UPPL_RC_ERR_* is returned when ipc response could not be added to sess.
 **/
UncRespCode Kt_Boundary::ReadBoundaryValFromDB(
    OdbcmConnectionHandler *db_conn,
    void* key_struct,
    void* val_struct,
    uint32_t data_type,
    uint32_t operation_type,
    uint32_t &max_rep_ct,
    vector<key_boundary_t> &vect_key_boundary,
    vector<val_boundary_st_t> &vect_val_boundary_st) {
  if (operation_type < UNC_OP_READ) {
    // Unsupported operation type for this function
    return UNC_RC_SUCCESS;
  }
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  UncRespCode read_status = UNC_RC_SUCCESS;
  ODBCM_RC_STATUS read_db_status = ODBCM_RC_SUCCESS;

  // Common structures that will be used to send query to ODBC
  // Structure used to send request to ODBC
  DBTableSchema kt_boundary_dbtableschema;
  // construct TableAttrSchema structure
  // TableAttrSchema holds table_name, primary key, attr_name
  vector<TableAttrSchema> vect_table_attr_schema;
  list < vector<TableAttrSchema> > row_list;
  vector<ODBCMOperator> vect_key_operations;
  void* old_val_struct;
  PopulateDBSchemaForKtTable(db_conn, kt_boundary_dbtableschema,
                             key_struct,
                             val_struct,
                             operation_type, data_type, 0, 0,
                             vect_key_operations, old_val_struct,
                             NOTAPPLIED, false, PFC_FALSE);

  if (operation_type == UNC_OP_READ) {
    read_db_status = physical_layer->get_odbc_manager()->
        GetOneRow((unc_keytype_datatype_t)data_type,
                  kt_boundary_dbtableschema, db_conn);
  } else {
    read_db_status = physical_layer->get_odbc_manager()->
        GetBulkRows((unc_keytype_datatype_t)data_type, max_rep_ct,
                    kt_boundary_dbtableschema,
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
    pfc_log_info("Read operation has failed with error %d", read_db_status);
    return read_status;
  }
  FillBoundaryValueStructure(db_conn, kt_boundary_dbtableschema,
                             vect_key_boundary,
                             vect_val_boundary_st,
                             max_rep_ct);

  if (vect_val_boundary_st.empty()) {
    // Read failed, return error
    read_status = UNC_UPPL_RC_ERR_DB_GET;
    // log error to log daemon
    pfc_log_info("Read operation has failed, after reading response");
  }
  return read_status;
}

/** GetModifiedRows
 * @Description : This function reads all KT_Boundary with given row_status
 * @param[in] : obj_key_struct-vector of void* to bdry key structures
 * row_status-CsRowStatus value
 * @return    : Success or associated error code
 * UNC_RC_SUCCESS is returned when the response
 * is added to ipc session successfully.
 * UNC_UPPL_RC_ERR_* is returned when ipc response could not be added to sess.
 * */
UncRespCode Kt_Boundary::GetModifiedRows(OdbcmConnectionHandler *db_conn,
                                            vector<void *> &obj_key_struct,
                                            CsRowStatus row_status) {
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  uint32_t max_rep_ct = 0;
  vector<key_boundary_t> vect_obj_key_struct;
  vector<val_boundary_st_t> vect_obj_value_struct;

  key_boundary_t obj_key_boundary;
  val_boundary_t val_struct;
  memset(&obj_key_boundary, 0, sizeof(key_boundary_t));
  memset(&val_struct, 0, sizeof(val_boundary_t));

  void *boundary_key = reinterpret_cast<void *>(&obj_key_boundary);
  void *boundary_val = reinterpret_cast<void *>(&val_struct);

  UncRespCode read_status = UNC_RC_SUCCESS;
  ODBCM_RC_STATUS read_db_status = ODBCM_RC_SUCCESS;

  DBTableSchema kt_boundary_dbtableschema;
  vector<ODBCMOperator> vect_key_operations;
  void* old_val_struct;
  PopulateDBSchemaForKtTable(db_conn, kt_boundary_dbtableschema,
                             boundary_key,
                             boundary_val,
                             UNC_OP_READ, UNC_DT_CANDIDATE, 0, 0,
                             vect_key_operations, old_val_struct,
                             row_status,
                             true, PFC_FALSE);

  read_db_status = physical_layer->get_odbc_manager()->
      GetModifiedRows(UNC_DT_CANDIDATE, kt_boundary_dbtableschema, db_conn);

  if (read_db_status == ODBCM_RC_RECORD_NOT_FOUND) {
    pfc_log_debug("No record to read");
    read_status = UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE;
    return read_status;
  } else if (read_db_status == ODBCM_RC_CONNECTION_ERROR) {
    read_status = UNC_UPPL_RC_ERR_DB_ACCESS;
    return read_status;
  } else if (read_db_status != ODBCM_RC_SUCCESS) {
    pfc_log_error("Read operation has failed");
    read_status = UNC_UPPL_RC_ERR_DB_GET;
    return read_status;
  }

  FillBoundaryValueStructure(db_conn, kt_boundary_dbtableschema,
                             vect_obj_key_struct,
                             vect_obj_value_struct,
                             max_rep_ct);

  // Fill key structures as well
  vector<key_boundary_t> :: iterator vect_iter = vect_obj_key_struct.begin();
  for (; vect_iter != vect_obj_key_struct.end(); ++vect_iter) {
    key_boundary_t *obj_key_boundary = new key_boundary_t;
    memset(obj_key_boundary->boundary_id, '\0',
           sizeof(obj_key_boundary->boundary_id));
    memcpy(obj_key_boundary->boundary_id,
           (const char *)(*vect_iter).boundary_id,
           strlen((const char *)(*vect_iter).boundary_id)+1);
    pfc_log_debug("KT_BOUNDARY name is %s\n", obj_key_boundary->boundary_id);
    void *key_struct = reinterpret_cast<void *>(obj_key_boundary);
    obj_key_struct.push_back(key_struct);
  }
  return read_status;
}

/** IsKeyExists
 * @Description : This function checks whether the boundary_id exists in DB
 * @param[in] : data_type-UNC_DT_*,type of database
 * key_values-vector of keys ins string
 * @return    : UNC_RC_SUCCESS or UNC_UPPL_RC_ERR* based on operation type
 * * */
UncRespCode Kt_Boundary::IsKeyExists(OdbcmConnectionHandler *db_conn,
                                        unc_keytype_datatype_t data_type,
                                        const vector<string> &key_values) {
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();

  UncRespCode check_status = UNC_RC_SUCCESS;

  if (key_values.empty()) {
    // No key given, return failure
    pfc_log_info("No key given. Returning error");
    return UNC_UPPL_RC_ERR_BAD_REQUEST;
  }

  string boundary_id = key_values[0];

  // Structure used to send request to ODBC
  DBTableSchema kt_boundary_dbtableschema;

  // Construct Primary key list
  vector<string> vect_prim_keys;
  vect_prim_keys.push_back(BDRY_ID_STR);

  // Construct TableAttrSchema structure
  // TableAttrSchema holds table_name, primary key, attr_name
  vector<TableAttrSchema> vect_table_attr_schema;
  list< vector<TableAttrSchema> > row_list;

  // boundary_id
  PhyUtil::FillDbSchema(unc::uppl::BDRY_ID, boundary_id, boundary_id.length(),
                        DATATYPE_UINT8_ARRAY_32, vect_table_attr_schema);

  kt_boundary_dbtableschema.set_table_name(unc::uppl::BOUNDARY_TABLE);
  kt_boundary_dbtableschema.set_primary_keys(vect_prim_keys);
  row_list.push_back(vect_table_attr_schema);
  kt_boundary_dbtableschema.set_row_list(row_list);

  // Send request to ODBC for Boundary_common_table
  ODBCM_RC_STATUS check_db_status = physical_layer->get_odbc_manager()->\
      IsRowExists(data_type, kt_boundary_dbtableschema, db_conn);

  if (check_db_status == ODBCM_RC_CONNECTION_ERROR) {
    pfc_log_error("DB connection not available or cannot access DB");
    check_status = UNC_UPPL_RC_ERR_DB_ACCESS;
  } else if (check_db_status == ODBCM_RC_ROW_EXISTS) {
    pfc_log_debug("DB returned success for Row exists");
    pfc_log_debug("Checking .db_return_status_ %d with %d",
                  kt_boundary_dbtableschema.db_return_status_,
                  static_cast<int>(DELETED));
    if (kt_boundary_dbtableschema.db_return_status_ != DELETED) {
      pfc_log_debug("DB returned success for Row exists");
    } else {
      pfc_log_debug("DB Returned failure for IsRowExists");
      check_status = UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE;
    }
  } else {
    pfc_log_info("DB Returned failure for IsRowExists");
    check_status = UNC_UPPL_RC_ERR_DB_GET;
  }
  return check_status;
}


/** Fill_Attr_Syntax_Map
 * @Description : This function populates the values to be used for attribute
 *  * validation
 * * @param[in] : none
 * * @return    : void
 *****/
void Kt_Boundary::Fill_Attr_Syntax_Map() {
  map<string, Kt_Class_Attr_Syntax> attr_syntax_map;
  Kt_Class_Attr_Syntax a0 = { PFC_IPCTYPE_STRING, 0, 0, 1, 32, true, "" };
  attr_syntax_map[BDRY_ID_STR] = a0;

  Kt_Class_Attr_Syntax a1 = { PFC_IPCTYPE_STRING, 0, 0, 1, 128, false, "" };
  attr_syntax_map[BDRY_DESCRIPTION_STR] = a1;

  Kt_Class_Attr_Syntax a2 = { PFC_IPCTYPE_STRING, 0, 0, 1, 32, true, "" };
  attr_syntax_map[BDRY_CTR_NAME1_STR] = a2;

  Kt_Class_Attr_Syntax a3 = { PFC_IPCTYPE_STRING, 0, 0, 1, 32, true, "" };
  attr_syntax_map[BDRY_DM_NAME1_STR] = a3;

  Kt_Class_Attr_Syntax a5 = { PFC_IPCTYPE_STRING, 0, 0, 4, 320, false, "" };
  attr_syntax_map[BDRY_PORT_ID1_STR] = a5;

  Kt_Class_Attr_Syntax a6 = { PFC_IPCTYPE_STRING, 0, 0, 1, 32, true, "" };
  attr_syntax_map[BDRY_CTR_NAME2_STR] = a6;

  Kt_Class_Attr_Syntax a7 = { PFC_IPCTYPE_STRING, 0, 0, 1, 32, true, "" };
  attr_syntax_map[BDRY_DM_NAME2_STR] = a7;

  Kt_Class_Attr_Syntax a9 = { PFC_IPCTYPE_STRING, 0, 0, 4, 320, false, "" };
  attr_syntax_map[BDRY_PORT_ID2_STR] = a9;

  Kt_Class_Attr_Syntax a10 = { PFC_IPCTYPE_STRING, 0, 0, 0, 7, false, "" };
  attr_syntax_map[BDRY_VALID_STR] = a10;

  Kt_Class_Attr_Syntax a11 = { PFC_IPCTYPE_STRING, 0, 3, 0, 0, false, "" };
  attr_syntax_map[BDRY_ROW_STATUS_STR] = a11;

  Kt_Class_Attr_Syntax a12 = { PFC_IPCTYPE_STRING, 0, 0, 0, 7, false, "" };
  attr_syntax_map[BDRY_ATTR_STR] = a12;
  attr_syntax_map_all[UNC_KT_BOUNDARY] = attr_syntax_map;
}

/** SendSemanticRequestToUPLL
 * @Description : This functions calls IPC to check whether UNC_KT_BOUNDARY
 * is being referred in Logical
 * @param[in] : key_boundary - specifies key instance of KT_BOUNDARY
 * data_type-UNC_DT_*,type of database
 * @return    : UNC_RC_SUCCESS if boundary is not referred
 * or UNC_UPPL_RC_ERR_* if boundary is referred in logical
 **/
UncRespCode Kt_Boundary::SendSemanticRequestToUPLL(void* key_struct,
                                                      uint32_t data_type) {
  // Incase for UNC_KT_BOUNDARY delete,
  // check whether any referenced object
  // Is present in Logical Layer,
  // If yes DELETE should not be allowed
  UncRespCode status = UNC_RC_SUCCESS;
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();

  status = physical_layer->get_ipc_connection_manager()->
      get_ipc_client_logical_handler()->CheckInUseInLogical(UNC_KT_BOUNDARY,
                                                            key_struct,
                                                            data_type);
  pfc_log_debug("In SendSemanticRequest status=%d", status);
  // Boundary is being referred
  if (status != UNC_RC_SUCCESS) {
    // log error and send error response
    pfc_log_error("Boundary is being referred in Logical");
  }
  return status;
}

/** getBoundaryInputOperStatus
 * @Description : This function gets the oper status
 * @param[in] : data_type-UNC_DT_*,type of database
 * controller_name-Controller Name referred by this boundary
 * domain_name-Domain Name referred by this boundary
 * logical_port_id-Logical port id referred by this boundary
 * @return    : oper status of boundary
 */
UpplBoundaryOperStatus Kt_Boundary::getBoundaryInputOperStatus(
    OdbcmConnectionHandler *db_conn,
    uint32_t data_type,
    string controller_name,
    string domain_name,
    string logical_port_id,
    vector<OperStatusHolder> &ref_oper_status) {
  pfc_log_debug("getBoundaryInputOperStatus() - controller name: %s",
                controller_name.c_str());
  UpplBoundaryOperStatus boundary_oper_status = UPPL_BOUNDARY_OPER_UNKNOWN;
  if (controller_name.empty())
    return boundary_oper_status;
  key_ctr_t ctr_key;
  memset(&ctr_key, 0, sizeof(key_ctr_t));
  memcpy(ctr_key.controller_name, controller_name.c_str(),
         (controller_name.length())+1);
  void* key_type_struct = reinterpret_cast<void*>(&ctr_key);
  Kt_Controller controller;
  uint8_t ctr_oper_status =
      (UpplControllerOperStatus) UPPL_CONTROLLER_OPER_UP;
  uint8_t logical_port_oper_status =
      (UpplLogicalPortOperStatus) UPPL_LOGICAL_PORT_OPER_UP;
  uint32_t ctrl_data_type = data_type;
  if (data_type == UNC_DT_IMPORT) {
    ctrl_data_type = UNC_DT_RUNNING;
  }
  // Get the controller's oper status and decide on the oper_status
  UncRespCode read_oper_status = get_oper_status(
      ref_oper_status,
      UNC_KT_CONTROLLER,
      reinterpret_cast<void*>(&ctr_key),
      ctr_oper_status);
  UncRespCode read_status = UNC_RC_SUCCESS;
  if (read_oper_status != UNC_RC_SUCCESS) {
    read_status = controller.GetOperStatus(
        db_conn, ctrl_data_type, key_type_struct, ctr_oper_status);
    pfc_log_debug("Controller's oper_status %d", ctr_oper_status);
    ADD_CTRL_OPER_STATUS(controller_name, ctr_oper_status, ref_oper_status);
  }

  if (read_status == UNC_RC_SUCCESS &&
      ctr_oper_status != UPPL_CONTROLLER_OPER_DOWN) {
    // Boundary oper status does not depends on domain
    if (domain_name.empty()) {
      boundary_oper_status = UPPL_BOUNDARY_OPER_UP;
      pfc_log_debug("Returning boundary_oper_status %d", boundary_oper_status);
      return boundary_oper_status;
    }
    if (logical_port_id.empty()) {
      boundary_oper_status = UPPL_BOUNDARY_OPER_UP;
      pfc_log_debug("Returning boundary_oper_status %d", boundary_oper_status);
      return boundary_oper_status;
    }
    unc_keytype_ctrtype_t ctrl_type = UNC_CT_UNKNOWN;
    uint32_t ctr_data_type = data_type;
    if (data_type == UNC_DT_IMPORT) {
      ctr_data_type = UNC_DT_RUNNING;
    }
    UncRespCode ctr_type_status =
        PhyUtil::get_controller_type(db_conn, controller_name,
                                     ctrl_type,
                                     (unc_keytype_datatype_t) ctr_data_type);
    if (ctr_type_status != UNC_RC_SUCCESS) {
      pfc_log_debug("Error in get_controller_type :%d", ctr_type_status);
      return UPPL_BOUNDARY_OPER_UNKNOWN;
    }
    if (ctrl_type == (unc_keytype_ctrtype_t) UNC_CT_UNKNOWN) {
      boundary_oper_status = UPPL_BOUNDARY_OPER_UP;
      pfc_log_info(
          "LP need not be considered for bypass controller."
          " Returning boundary_oper_status %d", boundary_oper_status);
      return boundary_oper_status;
    }
    key_logical_port_t logical_port_key;
    memset(&logical_port_key, '\0', sizeof(logical_port_key));
    memcpy(logical_port_key.domain_key.ctr_key.controller_name,
           controller_name.c_str(),
           controller_name.length()+1);
    memcpy(logical_port_key.domain_key.domain_name,
           domain_name.c_str(),
           domain_name.length()+1);
    memcpy(logical_port_key.port_id,
           logical_port_id.c_str(),
           logical_port_id.length()+1);
    UncRespCode read_oper_status = get_oper_status(
        ref_oper_status,
        UNC_KT_LOGICAL_PORT,
        reinterpret_cast<void*>(&logical_port_key),
        logical_port_oper_status);
    if (read_oper_status != UNC_RC_SUCCESS) {
      key_type_struct = reinterpret_cast<void*>(&logical_port_key);
      Kt_LogicalPort logical_port;
      // Get the logical_port's oper status and decide on the oper_status
      read_status = logical_port.GetOperStatus(db_conn, data_type,
                                               key_type_struct,
                                               logical_port_oper_status);
      if (read_status != UNC_RC_SUCCESS) {
        if (data_type == UNC_DT_IMPORT) {
          // Check in running db as well
          read_status = logical_port.GetOperStatus(db_conn, UNC_DT_RUNNING,
                                                   key_type_struct,
                                                   logical_port_oper_status);
          pfc_log_debug("Logical_Port's oper_status %d",
                       logical_port_oper_status);
        }
        if (read_status != UNC_RC_SUCCESS) {
          pfc_log_debug("Returning boundary_oper_status %d",
                       boundary_oper_status);
          return boundary_oper_status;
        } else {
          pfc_log_debug("Logical_Port's oper_status %d",
                       logical_port_oper_status);
          ADD_LP_PORT_OPER_STATUS(logical_port_key,
                                  logical_port_oper_status,
                                  ref_oper_status);
        }
      } else {
        pfc_log_debug(
            "Logical_Port's oper_status %d", logical_port_oper_status);
        ADD_LP_PORT_OPER_STATUS(logical_port_key,
                                logical_port_oper_status,
                                ref_oper_status);
      }
    }
    if (logical_port_oper_status == UPPL_LOGICAL_PORT_OPER_UP) {
      boundary_oper_status = UPPL_BOUNDARY_OPER_UP;
    } else if (logical_port_oper_status == UPPL_LOGICAL_PORT_OPER_UNKNOWN) {
      boundary_oper_status = UPPL_BOUNDARY_OPER_UNKNOWN;
    } else {
      boundary_oper_status = UPPL_BOUNDARY_OPER_DOWN;
    }
  }
  pfc_log_info("Returning boundary_oper_status %d", boundary_oper_status);
  return boundary_oper_status;
}

/** HandleOperStatus
 * @Description : This function performs the required actions
 *  when oper status changes in controller/domain/logical_port
 * @param[in] : data_type-UNC_DT_*,type of database
 * key_struct-void* to bdry key structure
 * value_struct-void* to bdry val structure
 * @return    : Success or associated error code
 */
UncRespCode Kt_Boundary::HandleOperStatus(
    OdbcmConnectionHandler *db_conn,
    uint32_t data_type,
    void *key_struct,
    void *value_struct,
    vector<OperStatusHolder> &ref_oper_status) {
  UncRespCode return_code = UNC_RC_SUCCESS;
  UpplBoundaryOperStatus boundary_oper_status = UPPL_BOUNDARY_OPER_UNKNOWN;
  if (value_struct == NULL) {
    pfc_log_info("Value struct is NULL- Returning SUCCESS");
    return return_code;
  }
  string controller_name = "";
  string domain_name = "";
  string port_id = "";
  val_boundary_t *obj_val_boundary =
      reinterpret_cast<val_boundary_t*>(value_struct);
  if (obj_val_boundary->valid[kIdxBoundaryControllerName1] != UNC_VF_VALID &&
      obj_val_boundary->valid[kIdxBoundaryControllerName2] != UNC_VF_VALID) {
    pfc_log_info("Controller name not valid- Returning SUCCESS");
    return return_code;
  }
  bool ctr1_valid = false;
  if (obj_val_boundary->valid[kIdxBoundaryControllerName1] == UNC_VF_VALID) {
    ctr1_valid = true;
    controller_name = (const char*) obj_val_boundary->controller_name1;
    pfc_log_debug("Kt_Boundary::HandleOperStatus() - controller1 name: %s",
                  controller_name.c_str());
    domain_name = (const char*) obj_val_boundary->domain_name1;
    port_id = (const char*) obj_val_boundary->logical_port_id1;
  } else {
    controller_name = (const char*) obj_val_boundary->controller_name2;
    pfc_log_debug("Kt_Boundary::HandleOperStatus() - controller2 name: %s",
                  controller_name.c_str());
    domain_name = (const char*) obj_val_boundary->domain_name2;
    port_id = (const char*) obj_val_boundary->logical_port_id2;
  }
  boundary_oper_status = getBoundaryInputOperStatus(db_conn, data_type,
                                                    controller_name,
                                                    domain_name, port_id,
                                                    ref_oper_status);

  if (boundary_oper_status == UPPL_BOUNDARY_OPER_UNKNOWN) {
    pfc_log_info("Set Boundary oper status as down/unknown");
    // Update oper_status in boundary table
    return_code = SetOperStatus(db_conn, UNC_DT_STATE, NULL, value_struct,
                                boundary_oper_status);
    pfc_log_debug("Set Boundary oper status status %d", return_code);
    return return_code;
  }
  UpplBoundaryOperStatus first_oper_status = boundary_oper_status;
  pfc_log_debug("Bdry first_oper_status = %d", boundary_oper_status);

  vector<key_boundary_t> vect_key_boundary;
  vector<val_boundary_st_t> vect_val_boundary_st;
  uint32_t max_rep_cnt = UPPL_MAX_REP_CT;
  UncRespCode read_status = ReadBoundaryValFromDB(
      db_conn, NULL,
      value_struct,
      UNC_DT_STATE,
      UNC_OP_READ_SIBLING_BEGIN,
      max_rep_cnt,
      vect_key_boundary,
      vect_val_boundary_st);
  if (read_status != UNC_RC_SUCCESS) {
    pfc_log_debug("No boundaries associated with given controller name");
    return return_code;
  }
  pfc_log_debug("From db, vect_boundary size is %"
                PFC_PFMT_SIZE_T, vect_key_boundary.size());
  for (unsigned int index = 0;
      index < vect_key_boundary.size();
      ++index) {
    if (ctr1_valid == true) {
      controller_name = (const char*) vect_val_boundary_st[index].\
          boundary.controller_name2;
      pfc_log_debug(
          "Kt_Boundary::HandleOperStatus() controller2 name: %s",
          controller_name.c_str());
      domain_name = (const char*) vect_val_boundary_st[index].\
          boundary.domain_name2;
      port_id = (const char*) vect_val_boundary_st[index].\
          boundary.logical_port_id2;
    } else {
      controller_name = (const char*) vect_val_boundary_st[index].\
          boundary.controller_name1;
      pfc_log_debug(
          "Kt_Boundary::HandleOperStatus() controller1 name: %s",
          controller_name.c_str());
      domain_name = (const char*) vect_val_boundary_st[index].\
          boundary.domain_name1;
      port_id = (const char*) vect_val_boundary_st[index].\
          boundary.logical_port_id1;
    }
    UpplBoundaryOperStatus second_oper_status =
        getBoundaryInputOperStatus(db_conn, data_type, controller_name,
                                   domain_name, port_id,
                                   ref_oper_status);
    pfc_log_debug("oper status for set2 %d", second_oper_status);
    UpplBoundaryOperStatus final_oper_status = UPPL_BOUNDARY_OPER_UNKNOWN;
    if (second_oper_status == UPPL_BOUNDARY_OPER_UNKNOWN) {
      final_oper_status = second_oper_status;
    } else {
      if (first_oper_status == UPPL_BOUNDARY_OPER_UP) {
        final_oper_status = second_oper_status;
      } else {
        final_oper_status = first_oper_status;
      }
    }
    pfc_log_debug("oper status for final %d", final_oper_status);
    SetOperStatus(db_conn, UNC_DT_STATE,
                  reinterpret_cast<void*>(&vect_key_boundary[index]),
                  reinterpret_cast<void*>
    (&vect_val_boundary_st[index].boundary),
    final_oper_status);
  }
  return return_code;
}

/** SetOperStatus
 * @Description : This function updates the oper status in db
 * @param[in] :data_type
 * key_struct-void* to bdry key structure
 * value_struct-void* to bdry value structure
 * oper_status-oper status of boundary 
 * @return    : Success or associated error code,
 *              UNC_RC_SUCCESS/ERR*
 */
UncRespCode Kt_Boundary::SetOperStatus(OdbcmConnectionHandler *db_conn,
                                          uint32_t data_type,
                                          void* key_struct,
                                          void* value_struct,
                                          UpplBoundaryOperStatus oper_status) {
  UncRespCode set_status_1 = UNC_RC_SUCCESS;
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  vector<TableAttrSchema> vect_table_attr_schema;
  list < vector<TableAttrSchema> > row_list;
  vector<string> vect_prim_keys;
  string empty = "";
  string controller_name = "";
  string domain_name = "";
  string logical_port_id = "";
  map<string, uint8_t> bdry_notfn;
  bool ctr1_valid = false;
  if (value_struct != NULL) {
    val_boundary *obj_val_boundary=
        reinterpret_cast<val_boundary_t*>(value_struct);
    if (obj_val_boundary->valid[kIdxBoundaryControllerName1] == UNC_VF_VALID) {
      controller_name = (const char*)obj_val_boundary->controller_name1;
      ctr1_valid = true;
      if (obj_val_boundary->valid[kIdxBoundaryDomainName1] == UNC_VF_VALID) {
        domain_name = (const char*)obj_val_boundary->domain_name1;
      }
      if (obj_val_boundary->valid[kIdxBoundaryLogicalPortId1] == UNC_VF_VALID) {
        logical_port_id = (const char*)obj_val_boundary->logical_port_id1;
      }
    } else if (obj_val_boundary->valid[kIdxBoundaryControllerName2]
                                       == UNC_VF_VALID) {
      controller_name = (const char*)obj_val_boundary->controller_name2;
      if (obj_val_boundary->valid[kIdxBoundaryDomainName2] == UNC_VF_VALID) {
        domain_name = (const char*)obj_val_boundary->domain_name2;
      }
      if (obj_val_boundary->valid[kIdxBoundaryLogicalPortId2] == UNC_VF_VALID) {
        logical_port_id = (const char*)obj_val_boundary->logical_port_id2;
      }
    } else {
      pfc_log_info("controller_name1/2 flag is not valid");
      return UNC_UPPL_RC_ERR_BAD_REQUEST;
    }
  }
  if (key_struct != NULL) {
    key_boundary_t *obj_key_boundary=
        reinterpret_cast<key_boundary_t*>(key_struct);
    string boun_id = (const char *)obj_key_boundary->boundary_id;
    // boundary_id
    vect_prim_keys.push_back(BDRY_ID_STR);
    PhyUtil::FillDbSchema(unc::uppl::BDRY_ID, boun_id.c_str(),
                          boun_id.length(), DATATYPE_UINT8_ARRAY_32,
                          vect_table_attr_schema);
    uint8_t oper_status_in_db = 0;
    UncRespCode get_status = GetOperStatus(db_conn, data_type,
                                              key_struct,
                                              oper_status_in_db);
    if (get_status == UNC_RC_SUCCESS) {
      bdry_notfn[boun_id] = oper_status_in_db;
    }
  } else if (value_struct != NULL) {
    if (ctr1_valid == true) {
      // controller_name1
      vect_prim_keys.push_back(BDRY_CTR_NAME1_STR);
      PhyUtil::FillDbSchema(unc::uppl::BDRY_CTR_NAME1, controller_name,
                            controller_name.length(), DATATYPE_UINT8_ARRAY_32,
                            vect_table_attr_schema);

      if (!domain_name.empty()) {
        // domain_name1
        vect_prim_keys.push_back(BDRY_DM_NAME1_STR);
        PhyUtil::FillDbSchema(unc::uppl::BDRY_DM_NAME1, domain_name,
                              domain_name.length(), DATATYPE_UINT8_ARRAY_32,
                              vect_table_attr_schema);
      }

      if (!logical_port_id.empty()) {
        // logical_port_id1
        vect_prim_keys.push_back(BDRY_PORT_ID1_STR);
        PhyUtil::FillDbSchema(unc::uppl::BDRY_PORT_ID1, logical_port_id,
                              logical_port_id.length(),
                              DATATYPE_UINT8_ARRAY_320,
                              vect_table_attr_schema);
      }
    } else {
      // controller_name2
      vect_prim_keys.push_back(BDRY_CTR_NAME2_STR);
      PhyUtil::FillDbSchema(unc::uppl::BDRY_CTR_NAME2, controller_name,
                            controller_name.length(), DATATYPE_UINT8_ARRAY_32,
                            vect_table_attr_schema);

      if (!domain_name.empty()) {
        // domain_name2
        vect_prim_keys.push_back(BDRY_DM_NAME2_STR);
        PhyUtil::FillDbSchema(unc::uppl::BDRY_DM_NAME2, domain_name,
                              domain_name.length(), DATATYPE_UINT8_ARRAY_32,
                              vect_table_attr_schema);
      }

      if (!logical_port_id.empty()) {
        // logical_port_id2
        vect_prim_keys.push_back(BDRY_PORT_ID2_STR);
        PhyUtil::FillDbSchema(unc::uppl::BDRY_PORT_ID2, logical_port_id,
                              logical_port_id.length(),
                              DATATYPE_UINT8_ARRAY_320,
                              vect_table_attr_schema);
      }
    }
    UncRespCode get_status = GetAllBoundaryOperStatus(db_conn,
                                                         controller_name,
                                                         domain_name,
                                                         logical_port_id,
                                                         bdry_notfn,
                                                         data_type);
    pfc_log_debug("Read existing oper status return %d", get_status);
  } else {
    return UNC_RC_SUCCESS;
  }

  // oper_status
  string oper_value = PhyUtil::uint8tostr(oper_status);
  PhyUtil::FillDbSchema(unc::uppl::BDRY_OPER_STATUS, oper_value,
                        oper_value.length(), DATATYPE_UINT16,
                        vect_table_attr_schema);

  DBTableSchema kt_boundary_dbtableschema;
  kt_boundary_dbtableschema.set_table_name(unc::uppl::BOUNDARY_TABLE);
  kt_boundary_dbtableschema.set_primary_keys(vect_prim_keys);
  row_list.push_back(vect_table_attr_schema);
  kt_boundary_dbtableschema.set_row_list(row_list);

  // Call ODBCManager and update
  // Send request to ODBC for boundary_table
  ODBCM_RC_STATUS update_db_status = physical_layer->get_odbc_manager()->
      UpdateOneRow((unc_keytype_datatype_t)data_type,
                   kt_boundary_dbtableschema, db_conn, true);
  if (update_db_status != ODBCM_RC_SUCCESS) {
    pfc_log_info(
        "oper_status update operation not success"
        " for controller/domain/logical_port set 1");
    set_status_1 = UNC_UPPL_RC_ERR_DB_UPDATE;
  }

  if (set_status_1 == UNC_RC_SUCCESS) {
    pfc_log_debug("Sending boundary oper status notification");
    // Send notification
    map<string, uint8_t> :: iterator iter = bdry_notfn.begin();
    for (; iter != bdry_notfn.end(); ++iter) {
      key_boundary_t bdry_key;
      string bdry_id = (*iter).first;
      if (bdry_id.empty()) {
        continue;
      }
      uint8_t oper_status_db = (*iter).second;
      memcpy(bdry_key.boundary_id, bdry_id.c_str(),
             bdry_id.length()+1);
      pfc_log_debug("Oper_status in db %d, new oper_status %d",
                    oper_status_db, oper_status);
      if (oper_status_db != oper_status) {
        UncRespCode oper_notfn = SendOperStatusNotification(bdry_key,
                                                               oper_status_db,
                                                               oper_status);
        pfc_log_debug("Notification Status - %d for boundary_id %s",
                      oper_notfn, bdry_id.c_str());
      }
    }
  }
  return UNC_RC_SUCCESS;
}

/** IsBoundaryReferred
 * @Description : This function returns true if given key_struct
 *  value is referred in boundary table
 * @param[in] : keytype-UNC_KT_BOUNDARY,value od unc_key_type_t
 * key_struct-void* to bdry key structure
 * data_type-UNC_DT_*,type of database
 * @return    : PFC_TRUE/PFC_FALSE
 */
pfc_bool_t Kt_Boundary::IsBoundaryReferred(OdbcmConnectionHandler *db_conn,
                                           unc_key_type_t keytype,
                                           void *key_struct,
                                           uint32_t data_type) {
  pfc_bool_t is_ref = PFC_FALSE;
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  DBTableSchema kt_dbtableschema;
  vector<TableAttrSchema> vect_table_attr_schema;
  list< vector<TableAttrSchema> > row_list;
  vector<string> vect_prim_keys;
  string controller_name = "";
  string domain_name = "";
  if (keytype == UNC_KT_CONTROLLER) {
    // Check whether controller_name1 is referred
    key_ctr_t *ctr_key = reinterpret_cast<key_ctr_t*>(key_struct);
    controller_name = (const char*)ctr_key->controller_name;

    vect_prim_keys.push_back(BDRY_CTR_NAME1_STR);
    // controller_name1
    PhyUtil::FillDbSchema(unc::uppl::BDRY_CTR_NAME1, controller_name,
                          controller_name.length(),
                          DATATYPE_UINT8_ARRAY_32, vect_table_attr_schema);
  } else if (keytype == UNC_KT_CTR_DOMAIN) {
    // Check whether domain_name1 is referred
    key_ctr_domain_t *ctr_domain_key =
        reinterpret_cast<key_ctr_domain_t*>(key_struct);
    controller_name = (const char*)ctr_domain_key->ctr_key.controller_name;
    domain_name = (const char*)ctr_domain_key->domain_name;

    vect_prim_keys.push_back(BDRY_CTR_NAME1_STR);
    // controller_name1
    PhyUtil::FillDbSchema(unc::uppl::BDRY_CTR_NAME1, controller_name,
                          controller_name.length(),
                          DATATYPE_UINT8_ARRAY_32, vect_table_attr_schema);

    vect_prim_keys.push_back(BDRY_DM_NAME1_STR);
    // domain_name1
    PhyUtil::FillDbSchema(unc::uppl::BDRY_DM_NAME1, domain_name,
                          domain_name.length(),
                          DATATYPE_UINT8_ARRAY_32, vect_table_attr_schema);
  } else {
    pfc_log_info("unknown key type to check in boundary table");
    return is_ref;
  }
  kt_dbtableschema.set_table_name(unc::uppl::BOUNDARY_TABLE);
  kt_dbtableschema.set_primary_keys(vect_prim_keys);
  row_list.push_back(vect_table_attr_schema);
  kt_dbtableschema.set_row_list(row_list);

  // Send request to ODBC for boundary_table
  ODBCM_RC_STATUS read_db_status = physical_layer->
      get_odbc_manager()->IsRowExists((unc_keytype_datatype_t)data_type,
                                      kt_dbtableschema, db_conn);
  if (read_db_status == ODBCM_RC_ROW_EXISTS &&
      kt_dbtableschema.db_return_status_ != DELETED) {
    pfc_log_info("Given controller/domain is referred in boundary");
    is_ref = PFC_TRUE;
    return is_ref;
  }

  vect_prim_keys.clear();
  vect_table_attr_schema.clear();

  if (keytype == UNC_KT_CONTROLLER) {
    // Check whether controller_name2 is referred
    key_ctr_t *ctr_key = reinterpret_cast<key_ctr_t*>(key_struct);
    controller_name = (const char*)ctr_key->controller_name;

    vect_prim_keys.push_back(BDRY_CTR_NAME2_STR);
    // controller_name2
    PhyUtil::FillDbSchema(unc::uppl::BDRY_CTR_NAME2, controller_name,
                          controller_name.length(),
                          DATATYPE_UINT8_ARRAY_32, vect_table_attr_schema);
  } else if (keytype == UNC_KT_CTR_DOMAIN) {
    // Check whether domain_name2 is referred
    key_ctr_domain_t *ctr_domain_key =
        reinterpret_cast<key_ctr_domain_t*>(key_struct);
    controller_name = (const char*)ctr_domain_key->ctr_key.controller_name;
    domain_name = (const char*)ctr_domain_key->domain_name;

    vect_prim_keys.push_back(BDRY_CTR_NAME2_STR);
    // controller_name2
    PhyUtil::FillDbSchema(unc::uppl::BDRY_CTR_NAME2, controller_name,
                          controller_name.length(),
                          DATATYPE_UINT8_ARRAY_32, vect_table_attr_schema);

    vect_prim_keys.push_back(BDRY_DM_NAME2_STR);
    // domain_name2
    PhyUtil::FillDbSchema(unc::uppl::BDRY_DM_NAME2, domain_name,
                          domain_name.length(),
                          DATATYPE_UINT8_ARRAY_32, vect_table_attr_schema);
  }
  DBTableSchema kt_dbtableschemaboundary;
  kt_dbtableschemaboundary.set_table_name(unc::uppl::BOUNDARY_TABLE);
  kt_dbtableschemaboundary.set_primary_keys(vect_prim_keys);
  row_list.clear();
  row_list.push_back(vect_table_attr_schema);
  kt_dbtableschemaboundary.set_row_list(row_list);

  // Send request to ODBC for boundary_table
  read_db_status = physical_layer->
      get_odbc_manager()->IsRowExists((unc_keytype_datatype_t)data_type,
                                      kt_dbtableschemaboundary, db_conn);
  if (read_db_status == ODBCM_RC_ROW_EXISTS &&
      kt_dbtableschemaboundary.db_return_status_ != DELETED) {
    pfc_log_info("Given controller/domain is referred in boundary");
    is_ref = PFC_TRUE;
    return is_ref;
  }
  return is_ref;
}

/** GetBoundaryValidFlag
 * @Description : This function gets the valid flag from db
 * @param[in] : key_struct-void* to bdry key structure
 * val_boundary_valid_st-instance of val_boundary_st_t
 * @return    : Success or associated error code,UNC_RC_SUCCESS/ERR*
 * */
UncRespCode Kt_Boundary::GetBoundaryValidFlag(
    OdbcmConnectionHandler *db_conn,
    void *key_struct,
    val_boundary_st_t &val_boundary_valid_st,
    uint32_t data_type) {
  UncRespCode return_code = UNC_RC_SUCCESS;
  vector<void *> vect_key_boundary;
  vect_key_boundary.push_back(key_struct);
  vector<void *> vect_val_boundary;
  return_code = ReadInternal(db_conn, vect_key_boundary, vect_val_boundary,
                             data_type, UNC_OP_READ);
  if (return_code == UNC_RC_SUCCESS) {
    val_boundary_st_t *val_boundary_new_valid_st =
        reinterpret_cast<val_boundary_st_t*>(vect_val_boundary[0]);
    if (val_boundary_new_valid_st != NULL) {
      val_boundary_valid_st = *val_boundary_new_valid_st;

      delete val_boundary_new_valid_st;
      val_boundary_new_valid_st = NULL;
      key_boundary_t *boundary_key = reinterpret_cast<key_boundary_t*>
      (vect_key_boundary[0]);
      if (boundary_key != NULL) {
        delete boundary_key;
        boundary_key = NULL;
      }
    } else {
      pfc_log_info("update boundary valid return null value");
    }
  } else {
    pfc_log_info("read internal failure from boundary update valid");
  }
  return return_code;
}

/** FrameValidValue
 * @Description : This function converts the string value from db to uint8
 * @param[in] : attr_value-Attribute value in string
 * obj_val_boundary_st-object of val_boundary_st
 * obj_val_boundary-object of val_boundary_t
 * @return    : void
 * */
void Kt_Boundary::FrameValidValue(string attr_value,
                                  val_boundary_st &obj_val_boundary_st,
                                  val_boundary_t &obj_val_boundary) {
  obj_val_boundary_st.valid[kIdxBoundaryStBoundary] = UNC_VF_VALID;
  for (unsigned int index = 0; index < attr_value.length(); ++index) {
    unsigned int valid = attr_value[index];
    if (attr_value[index] >= 48) {
      valid = attr_value[index] - 48;
    }
    if (index <= 6) {
      obj_val_boundary.valid[index] = valid;
    } else if (index < 8) {
      obj_val_boundary_st.valid[index-6] = valid;
    }
  }
  return;
}

/** FrameCsAttrValue
 * @Description : This function converts the string value from db to uint8
 * @param[in] : attr_value-Attribute value in string
 * obj_val_boundary-instance of val_boundary_t
 * @return    :undo 
 * */
void Kt_Boundary::FrameCsAttrValue(string attr_value,
                                   val_boundary_t &obj_val_boundary) {
  for (unsigned int index = 0; index < 7; ++index) {
    if (attr_value[index] >= 48) {
      obj_val_boundary.cs_attr[index] = attr_value[index] - 48;
    } else {
      obj_val_boundary.cs_attr[index] = attr_value[index];
    }
  }
  return;
}

/** ValidateSiblingFiltering
 * @Description : This function validates the read sibling filtering conditions
 * @param[in] : ctr1_valid_val,ctr2_valid_val-valid_value of controller_name
 * dmn1_valid_val,dmn2_valid_val-valid_value of domain_name
 * @return    : Success or associated error code,UNC_RC_SUCCESS/ERR*
 * */
UncRespCode Kt_Boundary::ValidateSiblingFiltering(
    unsigned int ctr1_valid_val,
    unsigned int ctr2_valid_val,
    unsigned int dmn1_valid_val,
    unsigned int dmn2_valid_val) {
  if (dmn1_valid_val == UNC_VF_VALID && ctr1_valid_val != UNC_VF_VALID) {
    pfc_log_error(
        "domain_name1 is valid and controller_name1 is not valid");
    return UNC_UPPL_RC_ERR_CFG_SYNTAX;
  }
  if (dmn2_valid_val == UNC_VF_VALID && ctr2_valid_val != UNC_VF_VALID) {
    pfc_log_error(
        "domain_name2 is valid and controller_name2 is not valid");
    return UNC_UPPL_RC_ERR_CFG_SYNTAX;
  }
  return UNC_RC_SUCCESS;
}


/** GetOperStatus
 * @Description : This function reads the oper_status value of the boundary
 * @param[in] : key_struct-void* to bdry key structure
 * data type-UNC_DT_*,type of database
 * param[out]:oper_status-oper status of boundary
 * @return    : Success or associated error code,UNC_RC_SUCCESS/ERR*
 */
UncRespCode Kt_Boundary::GetOperStatus(OdbcmConnectionHandler *db_conn,
                                          uint32_t data_type,
                                          void* key_struct,
                                          uint8_t &oper_status) {
  pfc_log_debug("Begin GetOperStatus");
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  UncRespCode return_code = UNC_RC_SUCCESS;
  vector<string> vect_prim_keys;
  vector<TableAttrSchema> vect_table_attr_schema;
  list < vector<TableAttrSchema> > row_list;

  key_boundary_t* obj_key_boundary =
      reinterpret_cast<key_boundary_t*>(key_struct);

  string boundary_id = (const char*)obj_key_boundary->boundary_id;
  vect_prim_keys.push_back(BDRY_ID_STR);
  PhyUtil::FillDbSchema(unc::uppl::BDRY_ID, boundary_id, boundary_id.length(),
                        DATATYPE_UINT8_ARRAY_32, vect_table_attr_schema);
  string value = "";
  unsigned int valid_val = 0, prev_db_val = 0;
  stringstream valid;
  PhyUtil::FillDbSchema(unc::uppl::BDRY_OPER_STATUS, BDRY_OPER_STATUS_STR,
                        value, value.length(), DATATYPE_UINT16,
                        (uint32_t) UNC_OP_READ, valid_val, prev_db_val,
                        vect_table_attr_schema, vect_prim_keys, valid);
  DBTableSchema kt_boundary_dbtableschema;
  kt_boundary_dbtableschema.set_table_name(unc::uppl::BOUNDARY_TABLE);
  kt_boundary_dbtableschema.set_primary_keys(vect_prim_keys);
  row_list.push_back(vect_table_attr_schema);
  kt_boundary_dbtableschema.set_row_list(row_list);
  ODBCM_RC_STATUS read_db_status = physical_layer->get_odbc_manager()->
      GetOneRow((unc_keytype_datatype_t)data_type, kt_boundary_dbtableschema,
                db_conn);
  if (read_db_status == ODBCM_RC_SUCCESS) {
    vector<TableAttrSchema> res_table_attr_schema =
        kt_boundary_dbtableschema.get_row_list().front();
    vector<TableAttrSchema> ::iterator vect_iter =
        res_table_attr_schema.begin();
    for (; vect_iter != res_table_attr_schema.end(); ++vect_iter) {
      TableAttrSchema tab_schema = (*vect_iter);
      ODBCMTableColumns attr_name = tab_schema.table_attribute_name;
      string attr_value = "";
      if (attr_name == unc::uppl::BDRY_OPER_STATUS) {
        PhyUtil::GetValueFromDbSchema(tab_schema, attr_value,
                                      DATATYPE_UINT16);
        oper_status = atoi(attr_value.c_str());
        pfc_log_info("oper_status from DB: %d", oper_status);
        break;
      }
    }
  } else {
    return_code = UNC_UPPL_RC_ERR_DB_GET;
  }
  return return_code;
}

/** SendOperStatusNotification
 * @Description : This function sends oper status change as notification to
 * north bound
 * @param[in] : bdry_key-object of bdry key structure
 * old_oper_st-old oper status 
 * new_oper_st-new oper status
 * @return    : Success or associated error code,UNC_RC_SUCCESS/ERR*
 * */
UncRespCode Kt_Boundary::SendOperStatusNotification(
    key_boundary_t bdry_key,
    uint8_t old_oper_st,
    uint8_t new_oper_st) {
  UncRespCode status = UNC_RC_SUCCESS;
  int err = 0;
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  val_boundary_st_t old_val_bdry, new_val_bdry;
  memset(&old_val_bdry, 0, sizeof(val_boundary_st_t));
  memset(&new_val_bdry, 0, sizeof(val_boundary_st_t));
  old_val_bdry.oper_status = old_oper_st;
  old_val_bdry.valid[kIdxBoundaryStOperStatus] = UNC_VF_VALID;
  // Getting the val structre from the database
  vector<void *> vect_key_boundary;
  vect_key_boundary.push_back(&bdry_key);
  vector<void *> vect_val_boundary;
  UncRespCode db_ret = UNC_RC_SUCCESS;
  OPEN_DB_CONNECTION(unc::uppl::kOdbcmConnReadOnly, db_ret);
  if (db_ret != UNC_RC_SUCCESS) {
    pfc_log_error("Error in opening DB connection");
    return db_ret;
  }
  UncRespCode return_code = ReadInternal(&db_conn,
                                         vect_key_boundary,
                                         vect_val_boundary,
                                         UNC_DT_RUNNING,
                                         UNC_OP_READ);
  if (return_code == UNC_RC_SUCCESS) {
    val_boundary_st_t *val_boundary_new_valid_st =
        reinterpret_cast<val_boundary_st_t*>(vect_val_boundary[0]);
    if (val_boundary_new_valid_st != NULL) {
      new_val_bdry = *val_boundary_new_valid_st;
      delete val_boundary_new_valid_st;
      val_boundary_new_valid_st = NULL;
      key_boundary_t *boundary_key = reinterpret_cast<key_boundary_t*>
      (vect_key_boundary[0]);
      if (boundary_key != NULL) {
        delete boundary_key;
        boundary_key = NULL;
      }
      new_val_bdry.oper_status = new_oper_st;
      new_val_bdry.valid[kIdxBoundaryStOperStatus] = UNC_VF_VALID;
      ServerEvent ser_evt((pfc_ipcevtype_t)UPPL_EVENTS_KT_BOUNDARY, err);
      northbound_event_header rsh = {static_cast<uint32_t>(UNC_OP_UPDATE),
          static_cast<uint32_t>(UNC_DT_STATE),
          static_cast<uint32_t>(UNC_KT_BOUNDARY)};
      err = PhyUtil::sessOutNBEventHeader(ser_evt, rsh);
      err |= ser_evt.addOutput(bdry_key);
      err |= ser_evt.addOutput(new_val_bdry);
      err |= ser_evt.addOutput(old_val_bdry);
      if (err != 0) {
        pfc_log_error(
            "Server Event addOutput failed, return IPC_WRITE_ERROR");
        status = UNC_UPPL_RC_ERR_IPC_WRITE_ERROR;
      } else {
        pfc_log_debug("%s", (IpctUtil::get_string(bdry_key)).c_str());
        pfc_log_debug("%s", (IpctUtil::get_string(new_val_bdry)).c_str());
        pfc_log_debug("%s", (IpctUtil::get_string(old_val_bdry)).c_str());
        // Call IPC server to post the event
        status = (UncRespCode) physical_layer
            ->get_ipc_connection_manager()->SendEvent(&ser_evt,
                                             "", UPPL_EVENTS_KT_BOUNDARY);
      }
    }
  } else {
    pfc_log_error("DB Error while getting Boundary details");
  }
  return status;
}
/** GetAllBoundaryOperStatus
 * @Description : This function reads the oper_status value all the boundaries
 *  provided in the request
 * @param[in] : controller_name-controller id in string
 * domain_name-domain id in string
 * logical_port_id-logical port id in string
 * bdry_notfn-instance of map<string, uint8_t>
 * data_type-UNC_DT_*,type of database
 * @return    :Success or associated error code 
 */
UncRespCode Kt_Boundary::GetAllBoundaryOperStatus(
    OdbcmConnectionHandler *db_conn,
    string controller_name,
    string domain_name,
    string logical_port_id,
    map<string, uint8_t> &bdry_notfn,
    uint32_t data_type) {
  pfc_log_debug("Begin GetAllBoundaryOperStatus");
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  UncRespCode return_code = UNC_RC_SUCCESS;
  uint32_t max_rep_ct = UPPL_MAX_REP_CT;
  vector<TableAttrSchema> vect_table_attr_schema;
  list < vector<TableAttrSchema> > row_list;
  vector<string> vect_prim_keys;

  // controller_name1
  vect_prim_keys.push_back(BDRY_CTR_NAME1_STR);
  PhyUtil::FillDbSchema(unc::uppl::BDRY_CTR_NAME1, controller_name,
                        controller_name.length(), DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);

  if (!domain_name.empty()) {
    // domain_name1
    vect_prim_keys.push_back(BDRY_DM_NAME1_STR);
    PhyUtil::FillDbSchema(unc::uppl::BDRY_DM_NAME1, domain_name,
                          domain_name.length(), DATATYPE_UINT8_ARRAY_32,
                          vect_table_attr_schema);
  }

  if (!logical_port_id.empty()) {
    // logical_port_id1
    vect_prim_keys.push_back(BDRY_PORT_ID1_STR);
    PhyUtil::FillDbSchema(unc::uppl::BDRY_PORT_ID1, logical_port_id,
                          logical_port_id.length(), DATATYPE_UINT8_ARRAY_320,
                          vect_table_attr_schema);
  }

  // boundary_id
  string boun_id = "";
  PhyUtil::FillDbSchema(unc::uppl::BDRY_ID, boun_id.c_str(),
                        boun_id.length(), DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);
  vect_prim_keys.push_back(BDRY_ID_STR);

  // oper_status
  string oper_value = "";
  PhyUtil::FillDbSchema(unc::uppl::BDRY_OPER_STATUS, oper_value,
                        oper_value.length(), DATATYPE_UINT16,
                        vect_table_attr_schema);

  DBTableSchema kt_boundary_dbtableschema;
  kt_boundary_dbtableschema.set_table_name(unc::uppl::BOUNDARY_TABLE);
  kt_boundary_dbtableschema.set_primary_keys(vect_prim_keys);
  row_list.push_back(vect_table_attr_schema);
  kt_boundary_dbtableschema.set_row_list(row_list);

  ODBCM_RC_STATUS read_db_status = physical_layer->get_odbc_manager()->
      GetBulkRows((unc_keytype_datatype_t)data_type, max_rep_ct,
                  kt_boundary_dbtableschema,
                  (unc_keytype_operation_t)UNC_OP_READ_BULK, db_conn);
  pfc_log_debug("GetBulk return %d", read_db_status);
  if (read_db_status != ODBCM_RC_SUCCESS) {
    return_code = UNC_UPPL_RC_ERR_DB_GET;
  }
  list < vector<TableAttrSchema> >& res_boundary_row_list =
      kt_boundary_dbtableschema.get_row_list();
  list < std::vector <TableAttrSchema> > :: iterator iter =
      res_boundary_row_list.begin();
  for (; iter != res_boundary_row_list.end(); ++iter) {
    vector<TableAttrSchema> res_table_attr_schema = (*iter);
    vector<TableAttrSchema> ::iterator vect_iter =
        res_table_attr_schema.begin();
    uint8_t oper_status = 0;
    string bdry_id = "";
    for (; vect_iter != res_table_attr_schema.end(); ++vect_iter) {
      TableAttrSchema tab_schema = (*vect_iter);
      ODBCMTableColumns attr_name = tab_schema.table_attribute_name;
      string attr_value = "";
      switch (attr_name) {
        case unc::uppl::BDRY_OPER_STATUS:
          PhyUtil::GetValueFromDbSchema(tab_schema, attr_value,
                                        DATATYPE_UINT16);
          oper_status = atoi(attr_value.c_str());
          pfc_log_info("oper_status from DB: %d", oper_status);
          break;
        case unc::uppl::BDRY_ID:
          uint8_t bdr_id[ODBCM_SIZE_32];
          PhyUtil::GetValueFromDbSchemaStr(tab_schema, bdr_id,
                                           DATATYPE_UINT8_ARRAY_32);
          bdry_id = reinterpret_cast<const char*> (bdr_id);
          pfc_log_info("bdry_id from DB: %s", bdr_id);
          break;
        default :
          break;
      }
    }
    bdry_notfn[bdry_id] = oper_status;
  }
  return_code = UNC_RC_SUCCESS;
  vector<TableAttrSchema> vect_table_attr_schema_1;
  list < vector<TableAttrSchema> > row_list_1;
  vector<string> vect_prim_keys_1;
  // controller_name2
  vect_prim_keys_1.push_back(BDRY_CTR_NAME2_STR);
  PhyUtil::FillDbSchema(unc::uppl::BDRY_CTR_NAME2, controller_name,
                        controller_name.length(), DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema_1);
  if (!domain_name.empty()) {
    // domain_name2
    vect_prim_keys_1.push_back(BDRY_DM_NAME2_STR);
    PhyUtil::FillDbSchema(unc::uppl::BDRY_DM_NAME2, domain_name,
                          domain_name.length(), DATATYPE_UINT8_ARRAY_32,
                          vect_table_attr_schema_1);
  }

  if (!logical_port_id.empty()) {
    // logical_port_id2
    vect_prim_keys_1.push_back(BDRY_PORT_ID2_STR);
    PhyUtil::FillDbSchema(unc::uppl::BDRY_PORT_ID2, logical_port_id,
                          logical_port_id.length(), DATATYPE_UINT8_ARRAY_320,
                          vect_table_attr_schema_1);
  }

  // boundary_id
  boun_id.clear();
  PhyUtil::FillDbSchema(unc::uppl::BDRY_ID, boun_id.c_str(),
                        boun_id.length(), DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema_1);
  vect_prim_keys_1.push_back(BDRY_ID_STR);

  // oper_status
  oper_value.clear();
  PhyUtil::FillDbSchema(unc::uppl::BDRY_OPER_STATUS, oper_value,
                        oper_value.length(), DATATYPE_UINT16,
                        vect_table_attr_schema_1);
  DBTableSchema kt_dbtableschema;
  kt_dbtableschema.set_table_name(unc::uppl::BOUNDARY_TABLE);
  kt_dbtableschema.set_primary_keys(vect_prim_keys_1);
  row_list_1.push_back(vect_table_attr_schema_1);
  kt_dbtableschema.set_row_list(row_list_1);

  read_db_status = physical_layer->get_odbc_manager()->
      GetBulkRows((unc_keytype_datatype_t)data_type, max_rep_ct,
                  kt_dbtableschema,
                  (unc_keytype_operation_t)UNC_OP_READ_BULK, db_conn);
  pfc_log_debug("GetBulk return %d", read_db_status);
  if (read_db_status != ODBCM_RC_SUCCESS) {
    return_code = UNC_UPPL_RC_ERR_DB_GET;
  }
  list < vector<TableAttrSchema> >& res_boundary_row_list_1 =
      kt_dbtableschema.get_row_list();
  iter = res_boundary_row_list_1.begin();
  for (; iter != res_boundary_row_list_1.end(); ++iter) {
    vector<TableAttrSchema> res_table_attr_schema = (*iter);
    vector<TableAttrSchema> ::iterator vect_iter =
        res_table_attr_schema.begin();
    uint8_t oper_status = 0;
    string bdry_id = "";
    for (; vect_iter != res_table_attr_schema.end(); ++vect_iter) {
      TableAttrSchema tab_schema = (*vect_iter);
      ODBCMTableColumns attr_name = tab_schema.table_attribute_name;
      string attr_value = "";
      switch (attr_name) {
        case unc::uppl::BDRY_OPER_STATUS:
          PhyUtil::GetValueFromDbSchema(tab_schema, attr_value,
                                        DATATYPE_UINT16);
          oper_status = atoi(attr_value.c_str());
          pfc_log_info("oper_status from DB: %d", oper_status);
          break;
        case unc::uppl::BDRY_ID:
          uint8_t bdr_id[ODBCM_SIZE_32];
          PhyUtil::GetValueFromDbSchemaStr(tab_schema, bdr_id,
                                           DATATYPE_UINT8_ARRAY_32);
          bdry_id = reinterpret_cast<const char*> (bdr_id);
          pfc_log_info("bdry_id from DB: %s", bdr_id);
          break;
        default:
          break;
      }
    }
    bdry_notfn[bdry_id] = oper_status;
  }
  return return_code;
}

/** CheckBoundaryExistence
 * @Description : This function checks whether boundary already exists with
 * given value structure
 * @param[in] : key struct - boundary key structure
 * value_struct - boundary value structure
 * data_type-UNC_DT_*,type of database
 * @return    :Success or associated error code
 */
UncRespCode Kt_Boundary::CheckBoundaryExistence(
    OdbcmConnectionHandler *db_conn,
    void *key_struct,
    void *val_struct,
    uint32_t data_type) {
  UncRespCode status = UNC_RC_SUCCESS;
  // Check whether same boundary exists already
  vector<void *> vect_key, vect_val;
  vect_key.push_back(key_struct);
  vect_val.push_back(val_struct);
  UncRespCode read_status = ReadInternal(db_conn, vect_key,
                                            vect_val,
                                            data_type,
                                            UNC_OP_READ_SIBLING_BEGIN);
  if (read_status == UNC_RC_SUCCESS) {
    for (unsigned int index = 0; index < vect_key.size(); ++index) {
      // Clear the memory
      val_boundary_st_t *boundary_val_st =
          reinterpret_cast<val_boundary_st_t*>(vect_val[index]);
      if (boundary_val_st != NULL) {
        if (boundary_val_st->boundary.cs_row_status != DELETED) {
          pfc_log_info(
              "Already a boundary exists with same domain/logicalport");
          status = UNC_UPPL_RC_ERR_CFG_SEMANTIC;
        }
        delete boundary_val_st;
        boundary_val_st = NULL;
      }
      key_boundary_t *bdry_key = reinterpret_cast<key_boundary_t*>
      (vect_key[index]);
      if (bdry_key != NULL) {
        delete bdry_key;
        bdry_key = NULL;
      }
    }
  }
  if (status != UNC_RC_SUCCESS) {
    return status;
  }
  // Check with reverse combination as well
  val_boundary_t val_new_bdry;
  memset(&val_new_bdry, 0, sizeof(val_new_bdry));
  val_boundary_t *obj_boundary_val =
      reinterpret_cast<val_boundary_t*>(val_struct);
  memcpy(val_new_bdry.description, obj_boundary_val->description,
         sizeof(val_new_bdry.description));
  memcpy(val_new_bdry.controller_name1, obj_boundary_val->controller_name2,
         sizeof(val_new_bdry.controller_name1));
  memcpy(val_new_bdry.domain_name1, obj_boundary_val->domain_name2,
         sizeof(val_new_bdry.domain_name1));
  memcpy(val_new_bdry.logical_port_id1, obj_boundary_val->logical_port_id2,
         sizeof(val_new_bdry.logical_port_id1));
  memcpy(val_new_bdry.controller_name2, obj_boundary_val->controller_name1,
         sizeof(val_new_bdry.controller_name2));
  memcpy(val_new_bdry.domain_name2, obj_boundary_val->domain_name1,
         sizeof(val_new_bdry.domain_name2));
  memcpy(val_new_bdry.logical_port_id2, obj_boundary_val->logical_port_id1,
         sizeof(val_new_bdry.logical_port_id2));
  stringstream new_valid;
  new_valid << obj_boundary_val->valid[kIdxBoundaryDescription] <<
      obj_boundary_val->valid[kIdxBoundaryControllerName2] <<
      obj_boundary_val->valid[kIdxBoundaryDomainName2] <<
      obj_boundary_val->valid[kIdxBoundaryLogicalPortId2] <<
      obj_boundary_val->valid[kIdxBoundaryControllerName1] <<
      obj_boundary_val->valid[kIdxBoundaryDomainName1] <<
      obj_boundary_val->valid[kIdxBoundaryLogicalPortId1];
  memcpy(val_new_bdry.valid, (new_valid.str()).c_str(),
         sizeof(val_new_bdry.valid));
  val_new_bdry.cs_row_status = obj_boundary_val->cs_row_status;
  memcpy(val_new_bdry.cs_attr, obj_boundary_val->cs_attr,
         sizeof(val_new_bdry.cs_attr));
  vect_key.clear();
  vect_val.clear();
  vect_key.push_back(key_struct);
  vect_val.push_back(reinterpret_cast<void*>(&val_new_bdry));
  read_status = ReadInternal(db_conn, vect_key,
                             vect_val,
                             data_type,
                             UNC_OP_READ_SIBLING_BEGIN);
  if (read_status == UNC_RC_SUCCESS) {
    for (unsigned int index = 0; index < vect_key.size(); ++index) {
      // Clear the memory
      val_boundary_st_t *boundary_val_st =
          reinterpret_cast<val_boundary_st_t*>(vect_val[index]);
      if (boundary_val_st != NULL) {
        if (boundary_val_st->boundary.cs_row_status != DELETED) {
          pfc_log_info(
              "Already a boundary exists with same domain/logicalport");
          status = UNC_UPPL_RC_ERR_CFG_SEMANTIC;
        }
        delete boundary_val_st;
        boundary_val_st = NULL;
      }
      key_boundary_t *bdry_key = reinterpret_cast<key_boundary_t*>
      (vect_key[index]);
      if (bdry_key != NULL) {
        delete bdry_key;
        bdry_key = NULL;
      }
    }
  }
  return status;
}
