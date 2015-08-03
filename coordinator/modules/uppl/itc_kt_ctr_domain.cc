/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/**
 * @brief   KT Domain implementation
 * @file    itc_kt_ctr_domain.cc
 *
 */

#include "itc_kt_ctr_domain.hh"
#include "itc_kt_controller.hh"
#include "itc_kt_boundary.hh"
#include "itc_kt_switch.hh"
#include "itc_kt_logicalport.hh"
#include "odbcm_db_varbind.hh"
#include "itc_read_request.hh"

using unc::uppl::PhysicalLayer;
#define DEFAULT_DOMAIN "(DEFAULT)"

/** 
 * @Description : This function initializes member variables
 *                and fills the attribute syntax map used for validation
 * @param[in]   : None
 * @return      : None
 **/
Kt_Ctr_Domain::Kt_Ctr_Domain() {
  for (int i = 0; i < KT_CTR_DOMAIN_CHILD_COUNT; ++i) {
    child[i] = NULL;
  }
  // Populate structure to be used for syntax validation
  if (attr_syntax_map_all.find(UNC_KT_CTR_DOMAIN) ==
      attr_syntax_map_all.end()) {
    Fill_Attr_Syntax_Map();
  }
}

/** 
 * @Description : This function frees child key instances
 *                instances for kt_ctr_domain
 * @param[in]   : None
 * @return      : None
 **/
Kt_Ctr_Domain::~Kt_Ctr_Domain() {
  // Delete all child objects
  for (int i = 0; i < KT_CTR_DOMAIN_CHILD_COUNT; ++i) {
    if (child[i] != NULL) {
      delete child[i];
      child[i] = NULL;
    }
  }
}


/** GetChildClassPointer
 * @Description : This function creates a new child class instance
 *                class of KtDomain based on index passed
 * @param[in]   : KtDomainChildClass
 * @return      : Kt_Base*
 **/
Kt_Base* Kt_Ctr_Domain::GetChildClassPointer(KtDomainChildClass KIndex) {
  switch (KIndex) {
    case KIdxLogicalPort: {
      if (child[KIndex] == NULL) {
        child[KIndex] = new Kt_LogicalPort();
      }
      break;
    }
    default: {
      pfc_log_error("Invalid index %d passed to GetChildClassPointer()",
                    KIndex);
      PFC_ASSERT(PFC_FALSE);
      return NULL;
    }
  }
  return child[KIndex];
}

/** 
 * @Description : This function creates a new row of KT_Domain in
 *                candidate domain table.
 * @param[in]   : session_id - ipc session id used for TC validation
 *                configuration_id - configuration id used for TC validation
 *                key_struct - the key for the new kt domain instance
 *                value_struct - the values for the new kt domain instance
 *                data_type - UNC_DT_* , Create only allowed in candidate from
 *                VTN
 *                sess - ipc server session where the response has to be added
 * @return      : UNC_RC_SUCCESS is returned when the response is added to ipc
 *                session successfully.
 *                UNC_UPPL_RC_ERR_* is returned when ipc response could not be
 *                added to sess.
 **/

UncRespCode Kt_Ctr_Domain::Create(OdbcmConnectionHandler *db_conn,
                                     uint32_t session_id,
                                     uint32_t configuration_id,
                                     void* key_struct,
                                     void* val_struct,
                                     uint32_t data_type,
                                     ServerSession &sess) {
  UncRespCode create_status = UNC_RC_SUCCESS;
  key_ctr_domain *obj_key_ctr_domain=
      reinterpret_cast<key_ctr_domain*>(key_struct);
  if ((unc_keytype_datatype_t)data_type != UNC_DT_CANDIDATE) {
    pfc_log_error("Create operation is provided on unsupported data type");
    create_status = UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED;
  } else {
    string controller_name = (const char*)obj_key_ctr_domain->
        ctr_key.controller_name;
    unc_keytype_ctrtype_t controller_type;
    UncRespCode retcode = PhyUtil::get_controller_type(
        db_conn, controller_name,
        controller_type,
        (unc_keytype_datatype_t)data_type);
    // Check whether operation is allowed on the given DT type
    if (retcode != UNC_RC_SUCCESS) {
      pfc_log_error("Error getting the controller type");
      create_status = UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED;
    } else if (retcode == UNC_RC_SUCCESS) {
      if (controller_type != UNC_CT_UNKNOWN) {
        pfc_log_error("Create Operation of KNOWN DOMAIN");
        pfc_log_error("is not supported from VTN");
        create_status = UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED;
      } else {
        create_status = CreateKeyInstance(db_conn, key_struct,
                                          val_struct,
                                          data_type,
                                          UNC_KT_CTR_DOMAIN);
        pfc_log_debug("Calling CreateKeyInstance returned: %d", create_status);
      }
    }
  }
  // Populate the response to be sent in ServerSession
  physical_response_header rsh = {session_id,
      configuration_id,
      UNC_OP_CREATE,
      0,
      0,
      0,
      data_type,
      static_cast<uint32_t>(create_status)};

  int err = PhyUtil::sessOutRespHeader(sess, rsh);
  err |= sess.addOutput((uint32_t)UNC_KT_CTR_DOMAIN);
  err |= sess.addOutput(*obj_key_ctr_domain);
  if (err != 0) {
    pfc_log_error("Server session addOutput failed, so return IPC_WRITE_ERROR");
    create_status = UNC_UPPL_RC_ERR_IPC_WRITE_ERROR;
  } else {
    create_status = UNC_RC_SUCCESS;
  }
  return create_status;
}

/** CreateKeyInstance
 * @Description : This function creates a new row of KT_Domain in
 *                candidate Domain table.
 * param[in]    : key_struct - the key for the new kt Domain instance
 *                value_struct - the values for the new kt Domain instance
 *                data_type - UNC_DT_* , Create only allowed in candidate if
 *                request is from VTN and allowed in state if request is from
 *                south bound
 *                key_type - indicates the key type class 
 * * @return    : UNC_RC_SUCCESS is returned when the create is success
 *                UNC_UPPL_RC_ERR_* is returned when ipc response could not be
 *                added to sess.
 * */
UncRespCode Kt_Ctr_Domain::CreateKeyInstance(OdbcmConnectionHandler *db_conn,
                                                void* key_struct,
                                                void* val_struct,
                                                uint32_t data_type,
                                                uint32_t key_type) {
  UncRespCode create_status = UNC_RC_SUCCESS;
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  // Check whether operation is allowed on the given DT type
  if ((unc_keytype_datatype_t)data_type != UNC_DT_CANDIDATE &&
      (unc_keytype_datatype_t)data_type != UNC_DT_STATE &&
      (unc_keytype_datatype_t)data_type != UNC_DT_IMPORT) {
    pfc_log_error("Create operation is provided on unsupported data type");
    return UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED;
  }

  // Structure used to send request to ODBC
  DBTableSchema kt_ctr_domain_dbtableschema;
  // Create DBSchema structure for ctr_domain_table
  void *old_val_struct;
  vector<ODBCMOperator> vect_key_operations;
  if ((unc_keytype_datatype_t)data_type == UNC_DT_CANDIDATE) {
    PopulateDBSchemaForKtTable(db_conn, kt_ctr_domain_dbtableschema,
                               key_struct,
                               val_struct,
                               UNC_OP_CREATE, data_type, 0, 0,
                               vect_key_operations, old_val_struct,
                               NOTAPPLIED, false, PFC_FALSE);
  } else if ((unc_keytype_datatype_t)data_type == UNC_DT_STATE ||
      (unc_keytype_datatype_t)data_type == UNC_DT_IMPORT) {
    PopulateDBSchemaForKtTable(db_conn, kt_ctr_domain_dbtableschema,
                               key_struct,
                               val_struct,
                               UNC_OP_CREATE, data_type, 0, 0,
                               vect_key_operations, old_val_struct,
                               NOTAPPLIED,
                               false,
                               PFC_TRUE);
  }
  // Send request to ODBC for ctr_domain_table create
  ODBCM_RC_STATUS create_db_status = physical_layer->get_odbc_manager()->\
      CreateOneRow((unc_keytype_datatype_t)data_type,
                   kt_ctr_domain_dbtableschema, db_conn);
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
    pfc_log_info("Create of Domain is success");
  }
  return create_status;
}

/** 
 * @Description : This function updates a row of KT_Domain in
 *                candidate Domain table.
 * @param[in]   : session_id - ipc session id used for TC validation
 *                configuration_id - configuration id used for TC validation
 *                key_struct - the key for the kt Domain instance
 *                value_struct - the values for the kt Domain instance
 *                data_type - UNC_DT_* , Update only allowed in candidate from
 *                VTN
 *                sess - ipc server session where the response has to be added
 * @return      : UNC_RC_SUCCESS is returned when the response is added to ipc
 *                session successfully.
 *                UNC_UPPL_RC_ERR_* is returned when ipc response could not be
 *                added to sess.
 **/
UncRespCode Kt_Ctr_Domain::Update(OdbcmConnectionHandler *db_conn,
                                     uint32_t session_id,
                                     uint32_t configuration_id,
                                     void* key_struct,
                                     void* val_struct,
                                     uint32_t data_type,
                                     ServerSession &sess) {
  UncRespCode update_status = UNC_RC_SUCCESS;
  key_ctr_domain *obj_key_ctr_domain=
      reinterpret_cast<key_ctr_domain*>(key_struct);
  if ((unc_keytype_datatype_t)data_type != UNC_DT_CANDIDATE) {
    pfc_log_error("Update operation is provided on unsupported data type");
    update_status = UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED;
  } else {
    string controller_name = (const char*)obj_key_ctr_domain->
        ctr_key.controller_name;
    unc_keytype_ctrtype_t controller_type;
    UncRespCode retcode = PhyUtil::get_controller_type(
        db_conn, controller_name,
        controller_type,
        (unc_keytype_datatype_t)data_type);
    // Check whether operation is allowed on the given DT type
    if (retcode != UNC_RC_SUCCESS) {
      pfc_log_error("Error getting the controller type");
      update_status = UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED;
    } else if (retcode == UNC_RC_SUCCESS) {
      if (controller_type != UNC_CT_UNKNOWN) {
        pfc_log_error("Update Operation of KNOWN DOMAIN");
        pfc_log_error("is not supported from VTN");
        update_status = UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED;
      } else {
        update_status = UpdateKeyInstance(db_conn, key_struct,
                                          val_struct,
                                          data_type,
                                          UNC_KT_CTR_DOMAIN);
        pfc_log_debug("Calling UpdateKeyInstance returned: %d", update_status);
      }
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
  err |= sess.addOutput((uint32_t)UNC_KT_CTR_DOMAIN);
  err |= sess.addOutput(*obj_key_ctr_domain);
  if (err != 0) {
    pfc_log_error("Server session addOutput failed, so return IPC_WRITE_ERROR");
    update_status = UNC_UPPL_RC_ERR_IPC_WRITE_ERROR;
  } else {
    update_status = UNC_RC_SUCCESS;
  }
  return update_status;
}

/** UpdateKeyInstance
 * @Description : This function updates a row of KT_Domain in Domain table.
 * @param[in]   : key_struct - the key for the new kt Domain instance
 *                value_struct - the values for the new kt Domain instance
 *                data_type - UNC_DT_* , update only allowed in candidate if
 *                request is from VTN and allowed in state if request is from
 *                southbound
 *                key_type - indicates the key type class
 * * @return    : UNC_RC_SUCCESS is returned when the update
 * is done successfully.
 * UNC_UPPL_RC_ERR_* is returned when the update is error
 * */
UncRespCode Kt_Ctr_Domain::UpdateKeyInstance(OdbcmConnectionHandler *db_conn,
                                                void* key_struct,
                                                void* val_struct,
                                                uint32_t data_type,
                                                uint32_t key_type) {
  UncRespCode update_status = UNC_RC_SUCCESS;
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  // Check whether operation is allowed on the given DT type
  if ((unc_keytype_datatype_t)data_type != UNC_DT_CANDIDATE &&
      (unc_keytype_datatype_t)data_type != UNC_DT_STATE &&
      (unc_keytype_datatype_t)data_type != UNC_DT_IMPORT) {
    pfc_log_error("Update operation is provided on unsupported data type");
    return UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED;
  }
  // Structure used to send request to ODBC
  DBTableSchema kt_ctr_domain_dbtableschema;
  void *old_val_struct;
  vector<ODBCMOperator> vect_key_operations;
  // Create DBSchema structure for ctr_domain_table
  if ((unc_keytype_datatype_t)data_type == UNC_DT_CANDIDATE) {
    PopulateDBSchemaForKtTable(db_conn, kt_ctr_domain_dbtableschema,
                               key_struct,
                               val_struct,
                               UNC_OP_UPDATE, data_type, 0, 0,
                               vect_key_operations, old_val_struct,
                               NOTAPPLIED, false, PFC_FALSE);
  } else if ((unc_keytype_datatype_t)data_type == UNC_DT_STATE ||
      (unc_keytype_datatype_t)data_type == UNC_DT_IMPORT) {
    PopulateDBSchemaForKtTable(db_conn, kt_ctr_domain_dbtableschema,
                               key_struct,
                               val_struct,
                               UNC_OP_UPDATE, data_type, 0, 0,
                               vect_key_operations, old_val_struct,
                               NOTAPPLIED,
                               false,
                               PFC_TRUE);
  }
  if (!((kt_ctr_domain_dbtableschema.get_row_list()).empty())) {
    // Send request to ODBC for ctr_domain_table update
    ODBCM_RC_STATUS update_db_status = physical_layer->get_odbc_manager()-> \
        UpdateOneRow((unc_keytype_datatype_t)data_type,
                     kt_ctr_domain_dbtableschema, db_conn, false);
    if (update_db_status != ODBCM_RC_SUCCESS) {
      if (update_db_status == ODBCM_RC_CONNECTION_ERROR) {
        // log fatal error to log daemon
        UPPL_LOG_FATAL("DB connection not available or cannot access DB");
        update_status = UNC_UPPL_RC_ERR_DB_ACCESS;
      } else {
        // log error to log daemon
        update_status = UNC_UPPL_RC_ERR_DB_UPDATE;
      }
    } else {
      pfc_log_info("Update of a ctr domain in data_type(%d) is success",
                   data_type);
    }
  } else {
    pfc_log_debug("Nothing to be updated, so return");
  }
  return update_status;
}

/**Delete
 * * @Description : This function deletes a row of KT_Domain in
 *                  candidate Domain table.
 * * @param[in]   : session_id - ipc session id used for TC validation
 *                  configuration_id - configuration id used for TC validation
 *                  key_struct - the key for the kt Domain instance
 *                  data_type - UNC_DT_* , delete only allowed in candidate if
 *                  request is from VTN and allowed in state if request is from
 *                  southbound
 *                  sess - ipc server session where the response has to be added
 * * @return      : UNC_RC_SUCCESS is returned when the response is added to
 *                  ipc session successfully.
 *                  UNC_UPPL_RC_ERR_* is returned when ipc response could not be
 *                  added to sess.
 **/
UncRespCode Kt_Ctr_Domain::Delete(OdbcmConnectionHandler *db_conn,
                                     uint32_t session_id,
                                     uint32_t configuration_id,
                                     void* key_struct,
                                     uint32_t data_type,
                                     ServerSession &sess) {
  UncRespCode delete_status = UNC_RC_SUCCESS;
  key_ctr_domain *obj_key_ctr_domain=
      reinterpret_cast<key_ctr_domain*>(key_struct);
  if ((unc_keytype_datatype_t)data_type != UNC_DT_CANDIDATE) {
    pfc_log_error("Delete operation is provided on unsupported data type");
    delete_status = UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED;
  } else {
    string domain_name = (const char*)obj_key_ctr_domain->domain_name;
    string controller_name = (const char*)obj_key_ctr_domain->
        ctr_key.controller_name;
    unc_keytype_ctrtype_t controller_type;
    UncRespCode retcode = PhyUtil::get_controller_type(
        db_conn, controller_name, controller_type,
        (unc_keytype_datatype_t)data_type);
    // Check whether operation is allowed on the given DT type
    if (retcode != UNC_RC_SUCCESS) {
      pfc_log_error("Error getting the controller type");
      delete_status = UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED;
    }
    if (retcode == UNC_RC_SUCCESS && controller_type != UNC_CT_UNKNOWN) {
      pfc_log_error("Delete Operation of KNOWN DOMAIN");
      pfc_log_error("is not supported from VTN");
      delete_status = UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED;
    } else if (retcode == UNC_RC_SUCCESS) {
      // Check whether any boundary is referring domain
      Kt_Boundary boundary_class;
      if (boundary_class.IsBoundaryReferred(db_conn, UNC_KT_CTR_DOMAIN,
                                            key_struct,
                                            data_type) == PFC_TRUE) {
        // Boundary is referring Domain
        pfc_log_error("Domain is referred in Boundary, "
            "so delete is not allowed");
        delete_status = UNC_UPPL_RC_ERR_CFG_SEMANTIC;
      } else {
        // Delete domain
        delete_status = DeleteKeyInstance(db_conn, key_struct, data_type,
                                          UNC_KT_CTR_DOMAIN);
      }
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
  err |= sess.addOutput((uint32_t)UNC_KT_CTR_DOMAIN);
  err |= sess.addOutput(*obj_key_ctr_domain);
  if (err != 0) {
    pfc_log_error("Server session addOutput failed, so return IPC_WRITE_ERROR");
    delete_status = UNC_UPPL_RC_ERR_IPC_WRITE_ERROR;
  } else {
    delete_status = UNC_RC_SUCCESS;
  }
  return delete_status;
}

/**DeleteKeyInstance
 * @Description : This function deletes a row of KT_Domain in
 *                  candidate domain table.
 * @param[in]   : key_struct - the key for the new kt domain instance
 *                data_type - UNC_DT_* , delete only allowed in candidate if
 *                request is from VTN and allowed in state if request is from
 *                southbound
 *                key_type - indicates the key type class
 * @return      : UNC_RC_SUCCESS is returned when the delete is done
 *                successfully.
 *                UNC_UPPL_RC_ERR_* is returned when the delete is error
 **/
UncRespCode Kt_Ctr_Domain::DeleteKeyInstance(OdbcmConnectionHandler *db_conn,
                                                void* key_struct,
                                                uint32_t data_type,
                                                uint32_t key_type) {
  // Check whether operation is allowed on the given DT type
  if ((unc_keytype_datatype_t)data_type != UNC_DT_CANDIDATE &&
      (unc_keytype_datatype_t)data_type != UNC_DT_STATE &&
      (unc_keytype_datatype_t)data_type != UNC_DT_IMPORT) {
    pfc_log_error("Delete operation is provided on unsupported data type");
    return UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED;
  }
  UncRespCode delete_status = UNC_RC_SUCCESS;
  key_ctr_domain *obj_key_ctr_domain=
      reinterpret_cast<key_ctr_domain*>(key_struct);
  string domain_name = (const char*)obj_key_ctr_domain->domain_name;
  string controller_name = (const char*)obj_key_ctr_domain->
      ctr_key.controller_name;

  vector<val_ctr_domain_st_t> vect_val_ctr_domain;
  vector<key_ctr_domain_t> vect_ctr_domain_id;

  if (domain_name.length() == 0) {
    delete_status = ReadBulkInternal(db_conn, key_struct, data_type,
                                     UPPL_MAX_REP_CT,
                                     vect_val_ctr_domain, vect_ctr_domain_id);
  } else {
    vect_ctr_domain_id.push_back(*obj_key_ctr_domain);
  }

  // Delete child classes and then delete domain
  vector<key_ctr_domain_t> ::iterator ctr_domain_iter =
      vect_ctr_domain_id.begin();
  for (; ctr_domain_iter != vect_ctr_domain_id.end(); ++ctr_domain_iter) {
    domain_name = (const char*)(*ctr_domain_iter).domain_name;
    pfc_log_debug("Iterating port entries for domain %s", domain_name.c_str());
    // Check logical port for corresponding logical port reference and delete it
    Kt_LogicalPort logicalport_obj;
    if ((unc_keytype_datatype_t)data_type == UNC_DT_STATE ||
        (unc_keytype_datatype_t)data_type == UNC_DT_IMPORT) {
      // Check logical port reference
      // Filling key_struct corresponding to tht key type
      void *child_key_struct = getChildKeyStruct(KIdxLogicalPort,
                                                 controller_name,
                                                 domain_name);
      Kt_LogicalPort logical_port;
      UncRespCode ch_delete_status = logical_port.DeleteKeyInstance(
          db_conn,
          child_key_struct,
          data_type,
          UNC_KT_LOGICAL_PORT);
      FreeChildKeyStruct(KIdxLogicalPort, child_key_struct);
      if (ch_delete_status == UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE) {
        pfc_log_debug("Domain has no child");
      }
      if (ch_delete_status != UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE &&
          ch_delete_status != UNC_RC_SUCCESS) {
        // child delete , failed so return error
        pfc_log_error("Delete failed for child with %d with error %d",
                      0, ch_delete_status);
        delete_status = UNC_UPPL_RC_ERR_CFG_SEMANTIC;
        break;
      }
    }
    if (delete_status != UNC_RC_SUCCESS) {
      continue;
    }
    // Structure used to send request to ODBC
    DBTableSchema kt_ctr_domain_dbtableschema;
    // Construct Primary key list
    // construct TableAttrSchema structure
    // TableAttrSchema holds table_name, primary key, attr_name
    vector<TableAttrSchema> vect_table_attr_schema;
    list< vector<TableAttrSchema> > row_list;
    vector<string> vect_prim_keys;
    vect_prim_keys.push_back(CTR_NAME_STR);
    // controller_name
    PhyUtil::FillDbSchema(unc::uppl::CTR_NAME, controller_name,
                          controller_name.length(), DATATYPE_UINT8_ARRAY_32,
                          vect_table_attr_schema);
    if (domain_name.length() != 0) {
      vect_prim_keys.push_back(DOMAIN_NAME_STR);

      // domain_name
      PhyUtil::FillDbSchema(unc::uppl::DOMAIN_NAME, domain_name,
                            domain_name.length(), DATATYPE_UINT8_ARRAY_32,
                            vect_table_attr_schema);
    }

    // Send request to ODBC for ctr_domain_table delete
    kt_ctr_domain_dbtableschema.set_table_name(unc::uppl::CTR_DOMAIN_TABLE);
    kt_ctr_domain_dbtableschema.set_primary_keys(vect_prim_keys);
    row_list.push_back(vect_table_attr_schema);
    kt_ctr_domain_dbtableschema.set_row_list(row_list);

    PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
    // Send request to ODBC for ctr_domain_table delete
    ODBCM_RC_STATUS delete_db_status = physical_layer->get_odbc_manager()->
        DeleteOneRow((unc_keytype_datatype_t)data_type,
                     kt_ctr_domain_dbtableschema, db_conn);
    pfc_log_debug("DeleteOneRow status is %d", delete_db_status);
    if (delete_db_status != ODBCM_RC_SUCCESS) {
      if (delete_db_status == ODBCM_RC_CONNECTION_ERROR) {
        // log fatal error to log daemon
        UPPL_LOG_FATAL("DB connection not available or cannot access DB");
        delete_status = UNC_UPPL_RC_ERR_DB_ACCESS;
      } else if (delete_db_status == ODBCM_RC_ROW_NOT_EXISTS ||
          delete_db_status == ODBCM_RC_RECORD_NOT_FOUND) {
        pfc_log_error("No matching record found");
        delete_status = UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE;
      } else {
        // log error to log daemon
        pfc_log_error("Delete operation has failed in domain common table");
        delete_status = UNC_UPPL_RC_ERR_DB_DELETE;
      }
    } else if (data_type != UNC_DT_CANDIDATE) {
      // Boundary operstatus to be changed only after running db sync
      // Notify switch to update domainid as invalid
      delete_status = UNC_RC_SUCCESS;
      Kt_Switch switch_obj;
      key_switch_t obj_key_switch;
      memset(&obj_key_switch, '\0', sizeof(key_switch_t));
      string switch_controller_name = (const char*)obj_key_ctr_domain->
          ctr_key.controller_name;
      memcpy(obj_key_switch.ctr_key.controller_name,
             switch_controller_name.c_str(),
             switch_controller_name.length()+1);

      // call switch function to set domain id valid status as invalid
      val_switch_st_t obj_val_switch;
      memset(&obj_val_switch, 0, sizeof(obj_val_switch));
      obj_val_switch.valid[kIdxSwitch] = UNC_VF_VALID;
      memcpy(obj_val_switch.switch_val.domain_name,
             domain_name.c_str(),
             domain_name.length()+1);
      obj_val_switch.switch_val.valid[kIdxSwitchDomainName] = UNC_VF_VALID;
      UncRespCode read_status = switch_obj.UpdateSwitchValidFlag(
          db_conn, reinterpret_cast<void*>(&obj_key_switch),
          reinterpret_cast<void*>(&obj_val_switch),
          obj_val_switch,
          UNC_VF_INVALID, data_type);
      pfc_log_debug("UpdateSwitchValidFlag returned %d", read_status);
    } else {
      pfc_log_info("Delete of a ctr_domain in data_type(%d) is success",
                   data_type);
      delete_status = UNC_RC_SUCCESS;
    }
  }
  return delete_status;
}

/** 
 * @Description : This function reads the given  instance of KT_Domain
 * @param[in]   : session_id - ipc session id used for TC validation
 *                key_struct - the key for the kt Domain instance
 *                val_struct - the value for the kt Domain instance
 *                data_type - UNC_DT_* , read allowed in
 *                candidate/running/startup/state
 *                operation_type - indicates the operation type - UNC_OP_*.
 * @return      : UNC_RC_SUCCESS is returned when the response
 *                is added to ipc session successfully.
 *                UNC_UPPL_RC_ERR_* is returned when ipc response could not be
 *                added to sess.
 **/
UncRespCode Kt_Ctr_Domain::ReadInternal(OdbcmConnectionHandler *db_conn,
                                           vector<void *> &key_val,
                                           vector<void *> &val_struct,
                                           uint32_t data_type,
                                           uint32_t operation_type) {
  if (operation_type != UNC_OP_READ && operation_type != UNC_OP_READ_SIBLING &&
      operation_type != UNC_OP_READ_SIBLING_BEGIN) {
    pfc_log_trace("This function not allowed for read next/bulk/count");
    return UNC_UPPL_RC_ERR_OPERATION_NOT_SUPPORTED;
  }
  pfc_log_debug("Inside ReadInternal of KT_CTR_DOMAIN");
  uint32_t max_rep_ct = 1;
  if (operation_type != UNC_OP_READ) {
    max_rep_ct = UPPL_MAX_REP_CT;
  }
  val_ctr_domain_st_t obj_dom_val;
  memset(&obj_dom_val, '\0', sizeof(val_ctr_domain_st_t));
  void *key_struct = key_val[0];
  void *void_val_struct = NULL;
  if ((!val_struct.empty()) && (val_struct[0] != NULL)) {
    memcpy(&obj_dom_val, (reinterpret_cast <val_ctr_domain_st_t*>
                                      (val_struct[0])),
           sizeof(val_ctr_domain_st_t));
    void_val_struct = reinterpret_cast<void *>(&obj_dom_val);
  }
  UncRespCode read_status = UNC_RC_SUCCESS;
  bool firsttime = true;
  do {
  // Get read response from database
    vector<key_ctr_domain> vect_domain_id;
    vector<val_ctr_domain> vect_val_ctr_domain;
    vector<val_ctr_domain_st> vect_val_ctr_domain_st;
    read_status = ReadDomainValFromDB(db_conn, key_struct,
                                        void_val_struct,
                                        data_type,
                                        operation_type,
                                        max_rep_ct,
                                        vect_val_ctr_domain_st,
                                        vect_domain_id);
    if (firsttime) {
       pfc_log_trace(
           "Clearing key_val and val_struct vectors for the first time");
      key_val.clear();
      val_struct.clear();
       firsttime = false;
    }
    pfc_log_debug("ReadDomainValFromDB returned %d with response size %"
               PFC_PFMT_SIZE_T,
               read_status,
               vect_val_ctr_domain_st.size());
    if (read_status == UNC_RC_SUCCESS) {
      pfc_log_debug("Read operation is success");
      for (unsigned int iIndex = 0 ; iIndex < vect_val_ctr_domain_st.size();
          ++iIndex) {
        key_ctr_domain_t *key_domain =
            new key_ctr_domain(vect_domain_id[iIndex]);
        key_val.push_back(reinterpret_cast<void *>(key_domain));
        val_ctr_domain_st *val_domain =
            new val_ctr_domain_st(vect_val_ctr_domain_st[iIndex]);
        val_struct.push_back(reinterpret_cast<void *>(val_domain));
      }
    } else if ((read_status == UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE &&
               val_struct.size() != 0)) {
      read_status = UNC_RC_SUCCESS;
    }
    if ((vect_val_ctr_domain_st.size() == UPPL_MAX_REP_CT) &&
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
  pfc_log_debug("Returned Key vector size: %" PFC_PFMT_SIZE_T,
                key_val.size());
  return read_status;
}

/**
 * @Description : This function reads bulk rows of KT_Domain in
 *                Domain table of specified data type.
 * @param[in]   : key_struct - the key for the kt Domain instance
 *                data_type - UNC_DT_* , read allowed in
 *                candidate/running/startup/state
 *                max_rep_ct - specifies number of rows to be returned
 *                parent_call - indicates whether parent has called this
 *                readbulk
 *                is_read_next - indicates whether this function is invoked
 *                from readnext
 * @return      : UNC_RC_SUCCESS is returned when the response
 *                is added to ipc session successfully.
 *                UNC_UPPL_RC_ERR_* is returned when ipc response could not be
 *                added to sess.
 **/
UncRespCode Kt_Ctr_Domain::ReadBulk(OdbcmConnectionHandler *db_conn,
                                       void* key_struct,
                                       uint32_t data_type,
                                       uint32_t &max_rep_ct,
                                       int child_index,
                                       pfc_bool_t parent_call,
                                       pfc_bool_t is_read_next,
                                       ReadRequest *read_req) {
  key_ctr_domain obj_key_ctr_domain=
      *(reinterpret_cast<key_ctr_domain*>(key_struct));
  UncRespCode read_status = UNC_RC_SUCCESS;

  vector<val_ctr_domain_st> vect_val_ctr_domain;
  if (data_type != UNC_DT_CANDIDATE && data_type != UNC_DT_RUNNING &&
      data_type != UNC_DT_STATE && data_type != UNC_DT_STARTUP) {
    pfc_log_debug("ReadBulk operation is not allowed in %d data type",
                  data_type);
    read_status = UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED;
    return read_status;
  }
  string str_domain_name =
      reinterpret_cast<char *>(obj_key_ctr_domain.domain_name);
  string str_controller_name =
      reinterpret_cast<char *>(obj_key_ctr_domain.ctr_key.controller_name);
  unc_keytype_ctrtype_t controller_type = UNC_CT_UNKNOWN;
  pfc_bool_t invalid_parent = false;
  UncRespCode retcode = PhyUtil::get_controller_type(
      db_conn, str_controller_name, controller_type,
      (unc_keytype_datatype_t)data_type);
  if (retcode != UNC_RC_SUCCESS) {
    pfc_log_error("Error getting the controller type");
    invalid_parent = true;
  }
  pfc_log_debug("Controller_type: %d", controller_type);
  pfc_bool_t iterate_domain = PFC_TRUE;
  if (((unc_keytype_datatype_t)data_type == UNC_DT_CANDIDATE ||
      (unc_keytype_datatype_t)data_type == UNC_DT_RUNNING ||
      (unc_keytype_datatype_t)data_type == UNC_DT_STARTUP) &&
      controller_type != UNC_CT_UNKNOWN) {
    // ignore the read_bulk of domain since for Running/Candidate/Startup only
    // UNKNOWN DOMAIN is expected in response
    pfc_log_debug(
        "ignore the domain value since for Running/Candidate, only "
        "UNKNOWN DOMAIN is expected in response");
    iterate_domain = PFC_FALSE;
  }
  pfc_bool_t domain_exists = PFC_FALSE;
  vector<val_ctr_domain_st> vect_val_ctr_domain_st;
  vector<key_ctr_domain> vect_domain_id;
  if (iterate_domain == PFC_TRUE && max_rep_ct > 0) {
    // Check for child call
    if (child_index == -1 &&
        !str_domain_name.empty()) {
      // Check for controller key existence
      vector<string> vect_key_value;
      vect_key_value.push_back(str_controller_name);
      vect_key_value.push_back(str_domain_name);
      UncRespCode key_exist_status = IsKeyExists(
          db_conn, (unc_keytype_datatype_t)data_type,
          vect_key_value);
      if (key_exist_status == UNC_RC_SUCCESS) {
        domain_exists = PFC_TRUE;
      }
    }
    // Read the domain values based on given key structure
    read_status = ReadBulkInternal(db_conn, key_struct,
                                   data_type,
                                   max_rep_ct,
                                   vect_val_ctr_domain_st,
                                   vect_domain_id);
    if (domain_exists == PFC_TRUE) {
      vect_domain_id.insert(vect_domain_id.begin(), obj_key_ctr_domain);
      val_ctr_domain_st_t dummy_val_domain;
      vect_val_ctr_domain_st.insert(vect_val_ctr_domain_st.begin(),
                                    dummy_val_domain);
    }
    pfc_log_debug("read status from Kt_Ctr_Domain is %d", read_status);
    if (invalid_parent == false && (read_status == UNC_RC_SUCCESS ||
        domain_exists == PFC_TRUE)) {
      // For each domain, read the child's attributes
      vector<val_ctr_domain_st> ::iterator vect_iter =
          vect_val_ctr_domain_st.begin();
      vector<key_ctr_domain> ::iterator domain_iter =
          vect_domain_id.begin();
      for (; domain_iter != vect_domain_id.end(); ++domain_iter,
      ++vect_iter) {
        pfc_log_debug("Iterating domain entries");
        if (domain_exists == PFC_FALSE) {
          val_ctr_domain_st_t obj_val_st = (*vect_iter);
          if (data_type == UNC_DT_CANDIDATE &&
              obj_val_st.domain.cs_row_status == DELETED) {
            pfc_log_debug("Ignoring DELETED entry %s",
                          reinterpret_cast<char *>((*domain_iter).domain_name));
            continue;
          }
          pfc_log_debug(
              "Adding domain - '%s' to session",
              reinterpret_cast<char *>((*domain_iter).domain_name));
          key_ctr_domain_t *key_buffer = new key_ctr_domain_t(*domain_iter);
          BulkReadBuffer obj_key_buffer = {
              UNC_KT_CTR_DOMAIN, IS_KEY,
              reinterpret_cast<void *>(key_buffer)
          };
          read_req->AddToBuffer(obj_key_buffer);
          if ((unc_keytype_datatype_t)data_type == UNC_DT_STATE) {
            val_ctr_domain_st_t *val_buffer =
                new val_ctr_domain_st_t(*vect_iter);
            BulkReadBuffer obj_value_buffer = {
                UNC_KT_CTR_DOMAIN, IS_STATE_VALUE,
                reinterpret_cast<void *>(val_buffer)
            };
            read_req->AddToBuffer(obj_value_buffer);
          } else {
            val_ctr_domain_t *val_buffer =
                new val_ctr_domain_t((*vect_iter).domain);
            BulkReadBuffer obj_value_buffer = {
                UNC_KT_CTR_DOMAIN, IS_VALUE,
                reinterpret_cast<void *>(val_buffer)
            };
            read_req->AddToBuffer(obj_value_buffer);
          }
          BulkReadBuffer obj_sep_buffer = {
              UNC_KT_CTR_DOMAIN, IS_SEPARATOR, NULL
          };
          read_req->AddToBuffer(obj_sep_buffer);
          --max_rep_ct;
          if (max_rep_ct == 0) {
            pfc_log_debug("CtrDomain - max_rep_ct reached zero...");
            return UNC_RC_SUCCESS;
          }
        }
        domain_exists = PFC_FALSE;
        pfc_log_debug("child_index %d", child_index);
        str_domain_name =
            reinterpret_cast<char *>((*domain_iter).domain_name);
        int st_child_index = KIdxLogicalPort;
/*
            (child_index >= 0 && child_index <= KIdxLogicalPort) \
            ? child_index+1 : KIdxLogicalPort;
*/
        pfc_log_debug("st_child_index %d", st_child_index);
        for (int kIdx = st_child_index; kIdx <= KIdxLogicalPort; ++kIdx) {
          pfc_log_debug("Domain Iterating child %d", kIdx);
          void *child_key_struct = getChildKeyStruct(kIdx,
                                                     str_controller_name,
                                                     str_domain_name);
          child[kIdx] = GetChildClassPointer((KtDomainChildClass)kIdx);
          if (child[kIdx] == NULL) {
            // Free key struct
            FreeChildKeyStruct(kIdx, child_key_struct);
            continue;
          }
          pfc_log_debug("Domain = Calling child %d read bulk", kIdx);
          UncRespCode ch_read_status = child[kIdx]->ReadBulk(
              db_conn, child_key_struct,
              data_type,
              max_rep_ct,
              -1,
              true,
              is_read_next,
              read_req);
          pfc_log_debug("read status from child %d is %d",
                        kIdx, ch_read_status);
          delete child[kIdx];
          child[kIdx] = NULL;
          FreeChildKeyStruct(kIdx, child_key_struct);
          if (max_rep_ct == 0) {
            pfc_log_debug("max_rep_ct reached zero, so returning");
            return UNC_RC_SUCCESS;
          }
        }
        // reset child index
        child_index = -1;
      }
    } else if (read_status == UNC_UPPL_RC_ERR_DB_ACCESS) {
      pfc_log_debug("KtCtrDomain ReadBulk - Returning DB Access Error");
      return read_status;
    }
  } else if (max_rep_ct == 0) {
    return UNC_RC_SUCCESS;
  }
  if (max_rep_ct > 0 && parent_call == false) {
    pfc_log_debug("max_rep_ct is %d and parent_call is %d, calling parent",
                  max_rep_ct, parent_call);
    // Filling key_struct corresponding to the key type
    Kt_Controller nextKin;
    key_ctr_t nextkin_key_struct;
    memcpy(nextkin_key_struct.controller_name,
           str_controller_name.c_str(),
           str_controller_name.length()+1);
    read_status = nextKin.ReadBulk(
        db_conn, reinterpret_cast<void *>(&nextkin_key_struct),
        data_type,
        max_rep_ct,
        0,
        false,
        is_read_next,
        read_req);
    pfc_log_debug("read status from Kt_Controller kin is %d", read_status);
    return UNC_RC_SUCCESS;
  }
  pfc_log_debug("reached end of key tree, read_status=%d",
                read_status);
  if (read_status == UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE) {
    read_status = UNC_RC_SUCCESS;
  }
  return UNC_RC_SUCCESS;
}

/**
 * @Description : This function reads bulk rows of KT_Domain in
 *                Domain table of specified data type.
 * @param[in]   : key_struct - the key for the kt Domain instance
 *                val_struct - the value struct for kt_Domain instance
 *                max_rep_ct - specifies number of rows to be returned
 *                vect_val_ctr_domain - indicates the fetched values from db of
 *                val_ctr_domain type
 *                vect_domain_id - indicates the fetched domain names from db
 * @return      : UNC_RC_SUCCESS is returned when the response
 *                is added to ipc session successfully.
 *                UNC_UPPL_RC_ERR_* is returned when ipc response could not be
 *                added to sess.
 **/
UncRespCode Kt_Ctr_Domain::ReadBulkInternal(
    OdbcmConnectionHandler *db_conn,
    void* key_struct,
    uint32_t data_type,
    uint32_t max_rep_ct,
    vector<val_ctr_domain_st> &vect_val_ctr_domain_st,
    vector<key_ctr_domain> &vect_domain_id) {
  if (max_rep_ct <= 0) {
    return UNC_RC_SUCCESS;
  }
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  void *val_struct = NULL;
  UncRespCode read_status = UNC_RC_SUCCESS;
  ODBCM_RC_STATUS read_db_status = ODBCM_RC_SUCCESS;
  DBTableSchema kt_ctr_domain_dbtableschema;
  // Populate DBSchema for ctr_domain_table
  void *old_val_struct;
  vector<ODBCMOperator> vect_key_operations;
  PopulateDBSchemaForKtTable(db_conn, kt_ctr_domain_dbtableschema,
                             key_struct,
                             val_struct,
                             UNC_OP_READ_BULK, data_type, 0, 0,
                             vect_key_operations, old_val_struct,
                             NOTAPPLIED, false, PFC_FALSE);
  // Read rows from DB
  read_db_status = physical_layer->get_odbc_manager()-> \
      GetBulkRows((unc_keytype_datatype_t)data_type, max_rep_ct,
                  kt_ctr_domain_dbtableschema,
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
    read_status = UNC_UPPL_RC_ERR_DB_GET;
    // log error to log daemon
    pfc_log_error("Read operation has failed");
    return read_status;
  }
  // From the values received from DB, populate val_ctr_domain structure
  FillDomainValueStructure(db_conn, kt_ctr_domain_dbtableschema,
                           vect_val_ctr_domain_st,
                           max_rep_ct,
                           UNC_OP_READ_BULK,
                           vect_domain_id);
  return read_status;
}

/** 
 * @Description : This function performs syntax validation for UNC_KT_DOMAIN
 * @param[in]   : key_struct - the key for the kt Domain instance
 *                value_struct - the value for the kt Domain instance
 *                data_type - UNC_DT_* - indicates the data base type
 *                operation_type - UNC_OP* - indicates the operation type
 * @return      : UNC_RC_SUCCESS is returned when the validation is successful
 *                UNC_UPPL_RC_ERR_* is returned when validation is failure
 **/
UncRespCode Kt_Ctr_Domain::PerformSyntaxValidation(
    OdbcmConnectionHandler *db_conn,
    void* key_struct,
    void* val_struct,
    uint32_t operation,
    uint32_t data_type) {
  UncRespCode ret_code = UNC_RC_SUCCESS;
  pfc_bool_t mandatory = PFC_TRUE;

  // Validate key structure
  key_ctr_domain *key = reinterpret_cast<key_ctr_domain_t*>(key_struct);
  string domain_name = reinterpret_cast<char*>(key->domain_name);
  map<string, Kt_Class_Attr_Syntax> attr_syntax_map =
      attr_syntax_map_all[UNC_KT_CTR_DOMAIN];
  IS_VALID_STRING_KEY(DOMAIN_NAME_STR, domain_name, operation, ret_code,
                      mandatory);
  if (ret_code != UNC_RC_SUCCESS) {
    return UNC_UPPL_RC_ERR_CFG_SYNTAX;
  }

  string value = reinterpret_cast<char*>(key->ctr_key.controller_name);
  IS_VALID_STRING_KEY(CTR_NAME_STR, value, operation, ret_code, mandatory);
  if (ret_code != UNC_RC_SUCCESS) {
    return UNC_UPPL_RC_ERR_CFG_SYNTAX;
  }

  // Validate value structure
  if (val_struct == NULL) {
    return ret_code;
  }
  unsigned int valid_val = 0;

  // validate type
  val_ctr_domain *domain_value =
      reinterpret_cast<val_ctr_domain_t*>(val_struct);
  valid_val = PhyUtil::uint8touint(domain_value->valid[kIdxDomainType]);
  IS_VALID_INT_VALUE(DOMAIN_TYPE_STR, domain_value->type, operation,
                     valid_val, ret_code, mandatory);
  if (ret_code != UNC_RC_SUCCESS) {
    return UNC_UPPL_RC_ERR_CFG_SYNTAX;
  }
  if (data_type == UNC_DT_CANDIDATE) {
    if (operation == UNC_OP_CREATE) {
      if (domain_value->type != UPPL_DOMAIN_TYPE_NORMAL) {
        pfc_log_info(
            "Only normal controller domain can be created from northbound");
        return UNC_UPPL_RC_ERR_CFG_SYNTAX;
      }
      if (domain_name == DEFAULT_DOMAIN) {
        pfc_log_info("Default Domain name cannot be used");
        return UNC_UPPL_RC_ERR_CFG_SYNTAX;
      }
    }
    if (operation == UNC_OP_UPDATE) {
      if (valid_val == UNC_VF_VALID) {
        pfc_log_info("Domain type cannot be modified from northbound");
        return UNC_UPPL_RC_ERR_CFG_SYNTAX;
      }
    }
  }
  // validate description
  valid_val =
      PhyUtil::uint8touint(domain_value->valid[kIdxDomainDescription]);
  value = reinterpret_cast<char*>(domain_value->description);
  IS_VALID_STRING_VALUE(DOMAIN_DESCRIPTION_STR, value, operation,
                        valid_val, ret_code, mandatory);
  if (ret_code != UNC_RC_SUCCESS) {
    return UNC_UPPL_RC_ERR_CFG_SYNTAX;
  }
  return ret_code;
}

/** 
 * @Description : This function performs semantic validation for UNC_KT_DOMAIN 
 * @param[in]   : key_struct - specifies key instance of KT_DOMAIN
 *                val_struct - specifies value of KT_DOMAIN
 *                operation - UNC_OP* - indicates the operation type
 *                data_type - UNC_DT* - indicates the data base type
 * @return      : UNC_RC_SUCCESS if semantic valition is successful
 *                or UNC_UPPL_RC_ERR_* if failed
 **/
UncRespCode Kt_Ctr_Domain::PerformSemanticValidation(
    OdbcmConnectionHandler *db_conn,
    void* key_struct,
    void* val_struct,
    uint32_t operation,
    uint32_t data_type) {
  UncRespCode status = UNC_RC_SUCCESS;
  pfc_log_debug("PerformSemanticValidation:KT_CTR_DOMAIN");
  // Check whether the given instance of domain exists in DB
  key_ctr_domain *obj_key_ctr_domain =
      reinterpret_cast<key_ctr_domain*>(key_struct);
  string domain_name = (const char*)obj_key_ctr_domain->domain_name;
  string controller_name = (const char*)obj_key_ctr_domain->
      ctr_key.controller_name;
  vector<string> domain_vect_key_value;
  domain_vect_key_value.push_back(controller_name);
  domain_vect_key_value.push_back(domain_name);
  UncRespCode key_status = IsKeyExists(db_conn,
                                          (unc_keytype_datatype_t)data_type,
                                          domain_vect_key_value);
  pfc_log_debug("IsKeyExists status %d", key_status);
  // In case of create operation, key should not exist
  if (operation == UNC_OP_CREATE) {
    if (key_status == UNC_RC_SUCCESS) {
      pfc_log_error("Key exists,CREATE not allowed");
      status = UNC_UPPL_RC_ERR_INSTANCE_EXISTS;
    } else if (key_status == UNC_UPPL_RC_ERR_DB_ACCESS) {
      pfc_log_error("DB Access failure");
      status = key_status;
    } else {
      pfc_log_debug("key instance not exist create operation allowed");
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
      pfc_log_debug("key instance exist update/del/read operation allowed");
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
        db_conn,
        (unc_keytype_datatype_t)parent_data_type, parent_vect_key_value);
    pfc_log_debug("Parent IsKeyExists status %d", parent_key_status);
    if (parent_key_status != UNC_RC_SUCCESS) {
      status = UNC_UPPL_RC_ERR_PARENT_DOES_NOT_EXIST;
    }
  }
  pfc_log_debug("Return Code SemanticValidation: %d", status);
  return status;
}

/** 
 * @Description : This function processes the alarm notification received
 *                from driver
 * @param[in]   : data_type - indicates the data base type 
 *                alarm type - contains type to indicate PATH_FAULT alarm
 *                oper_type - operation - contains UNC_OP_CREATE or
 *                UNC_OP_DELETE
 *                key_struct - indicates the key instance of KT_DOMAIN
 *                value_struct - indicates the alarm values structure
 * @return      : UNC_RC_SUCCESS if alarm is handled successfully or
 *                UNC_UPPL_RC_ERR* if incase of failure
 **/
UncRespCode Kt_Ctr_Domain::HandleDriverAlarms(
    OdbcmConnectionHandler *db_conn,
    uint32_t data_type,
    uint32_t alarm_type,
    uint32_t oper_type,
    void* key_struct,
    void* val_struct) {
  UncRespCode status = UNC_RC_SUCCESS;
  if (alarm_type != UNC_COREDOMAIN_SPLIT && alarm_type != UNC_DOMAIN_SPLIT) {
    pfc_log_warn("%d alarm received for domain is ignored", alarm_type);
    return status;
  }
  uint8_t oper_status_db = 0;
  UncRespCode read_status = GetOperStatus(db_conn, data_type,
                                             key_struct,
                                             oper_status_db);
  if (read_status != UNC_RC_SUCCESS) {
    pfc_log_error("Unable to get current oper_status from db");
    return read_status;
  }
  UpplDomainOperStatus new_oper_status = UPPL_CTR_DOMAIN_OPER_DOWN;
  // Read old_oper_status from DB
  if (oper_type == UNC_OP_CREATE) {
    new_oper_status = UPPL_CTR_DOMAIN_OPER_DOWN;
  } else if (oper_type == UNC_OP_DELETE) {
    new_oper_status = UPPL_CTR_DOMAIN_OPER_UP;
  }
  UpplDomainOperStatus old_oper_status =
      (UpplDomainOperStatus)oper_status_db;
  pfc_log_debug("Oper_status received from db: %d "
                "New Oper_status to be set is: %d"
                , old_oper_status, new_oper_status);
  if (new_oper_status != old_oper_status) {
    // Set oper_status update in DB
    status = SetOperStatus(db_conn, data_type, key_struct, new_oper_status);
    if (status != 0) {
      pfc_log_info("Update oper_status return: %d", status);
    }
  }
  return status;
}

/** HandleOperStatus
 * @Description : This function performs the required actions when
 *                oper status changes
 * @param[in]   : data_type - indicates the data base type
 *                key_struct - indicates the key instance of KT_DOMAIN
 *                value_struct - indicates the value structure of KT_DOMAIN
 * @return      : UNC_RC_SUCCESS - if oper status update in DB is successful
 *                else UNC_UPPL_RC_ERR_* is returned
 **/
UncRespCode Kt_Ctr_Domain::HandleOperStatus(
    OdbcmConnectionHandler *db_conn,
    uint32_t data_type,
    void *key_struct,
    void *value_struct) {
  UncRespCode return_code = UNC_RC_SUCCESS;

  if (key_struct != NULL) {
    key_ctr_domain_t *obj_key_ctr_domain =
        reinterpret_cast<key_ctr_domain_t*>(key_struct);
    string controller_name =
        (const char*) obj_key_ctr_domain->ctr_key.controller_name;
    // Get the controller's oper status and decide on the oper_status
    key_ctr_t ctr_key;
    memcpy(ctr_key.controller_name, controller_name.c_str(),
           controller_name.length()+1);
    uint8_t ctrl_oper_status = 0;
    UpplDomainOperStatus domain_oper_status = UPPL_CTR_DOMAIN_OPER_UNKNOWN;
    Kt_Controller controller;
    UncRespCode read_status = controller.GetOperStatus(
        db_conn, data_type, reinterpret_cast<void*>(&ctr_key),
        ctrl_oper_status);
    if (read_status == UNC_RC_SUCCESS) {
      if (ctrl_oper_status ==
          (UpplControllerOperStatus) UPPL_CONTROLLER_OPER_UP) {
        pfc_log_debug("Set Domain oper status as up");
        domain_oper_status = UPPL_CTR_DOMAIN_OPER_UP;
      }
    } else {
      pfc_log_info("Controller's oper_status read returned failure");
    }
    // Update oper_status in domain table
    return_code = SetOperStatus(db_conn, data_type, key_struct,
                                domain_oper_status);
    pfc_log_debug("Set Domain oper status status %d", return_code);

  } else {
    return_code = UNC_UPPL_RC_ERR_BAD_REQUEST;
  }
  return return_code;
}

/** 
 * @Description : This function invokes the notifyoperstatus of boundary
 * @param[in]   : data_type - indicates the data base type
 *                key_struct - indicates the key instance of kt domain
 * @return      : UNC_RC_SUCCESS -if oper status is handled properly in
 *                boundary
 *                or UNC_UPPL_RC_ERR* in case of failure
 **/
UncRespCode Kt_Ctr_Domain::InvokeBoundaryNotifyOperStatus(
    OdbcmConnectionHandler *db_conn,
    uint32_t data_type,
    void *key_struct) {
  // Get controller and domain oper status
  UncRespCode return_code = UNC_RC_SUCCESS;
  key_ctr_domain_t *obj_key_ctr_domain =
      reinterpret_cast<key_ctr_domain_t*>(key_struct);

  string controller_name =
      (const char*)obj_key_ctr_domain->ctr_key.controller_name;
  string domain_name =
      (const char*)obj_key_ctr_domain->domain_name;
  vector<OperStatusHolder> ref_oper_status;
  GET_ADD_CTRL_OPER_STATUS(controller_name, ref_oper_status,
                          data_type, db_conn);
  Kt_Boundary boundary;
  val_boundary_t obj_val_boundary1;
  memset(&obj_val_boundary1, 0, sizeof(obj_val_boundary1));
  memcpy(obj_val_boundary1.controller_name1,
         controller_name.c_str(), controller_name.length()+1);
  obj_val_boundary1.valid[kIdxBoundaryControllerName1] = UNC_VF_VALID;
  memcpy(obj_val_boundary1.domain_name1,
         domain_name.c_str(), domain_name.length()+1);
  obj_val_boundary1.valid[kIdxBoundaryDomainName1] = UNC_VF_VALID;
  return_code = boundary.HandleOperStatus(
      db_conn,
      data_type, NULL,
      reinterpret_cast<void *> (&obj_val_boundary1),
      ref_oper_status);
  pfc_log_debug("HandleOperStatus for boundary C1D1 class %d", return_code);
  val_boundary_t obj_val_boundary2;
  memset(&obj_val_boundary2, 0, sizeof(obj_val_boundary2));
  memcpy(obj_val_boundary2.controller_name2,
         controller_name.c_str(), controller_name.length()+1);
  obj_val_boundary2.valid[kIdxBoundaryControllerName2] = UNC_VF_VALID;
  memcpy(obj_val_boundary2.domain_name2,
         domain_name.c_str(), domain_name.length()+1);
  obj_val_boundary2.valid[kIdxBoundaryDomainName2] = UNC_VF_VALID;
  return_code = boundary.HandleOperStatus(
      db_conn,
      data_type, NULL,
      reinterpret_cast<void *> (&obj_val_boundary2),
      ref_oper_status);
  pfc_log_debug("HandleOperStatus for boundary C2D2 class %d", return_code);
  ClearOperStatusHolder(ref_oper_status);
  return return_code;
}

/** 
 * @Description : This function updates the oper status in db
 * @param[in]   : data_type - indicates the data base type
 *                key_struct - key instance of KT_DOMAIN
 *                oper_status - reference variable to store the oper status
 *                from DB
 * @return      : UNC_RC_SUCCESS - if oper status is read successfully else
 *                UNC_UPPL_RC_ERR* is returned
 **/
UncRespCode Kt_Ctr_Domain::GetOperStatus(OdbcmConnectionHandler *db_conn,
                                            uint32_t data_type,
                                            void* key_struct,
                                            uint8_t &oper_status) {
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  key_ctr_domain *obj_key_ctr_domain =
      reinterpret_cast<key_ctr_domain_t*>(key_struct);
  vector<TableAttrSchema> vect_table_attr_schema;
  list < vector<TableAttrSchema> > row_list;
  vector<string> vect_prim_keys;
  string controller_name = (const char*)obj_key_ctr_domain->
      ctr_key.controller_name;
  string domain_name = (const char*)obj_key_ctr_domain->domain_name;
  if (!controller_name.empty()) {
    vect_prim_keys.push_back(CTR_NAME_STR);
  }

  if (!domain_name.empty()) {
    vect_prim_keys.push_back(DOMAIN_NAME_STR);
  }

  PhyUtil::FillDbSchema(unc::uppl::CTR_NAME, controller_name,
                        controller_name.length(), DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);

  PhyUtil::FillDbSchema(unc::uppl::DOMAIN_NAME, domain_name,
                        domain_name.length(), DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);

  string oper_value;
  PhyUtil::FillDbSchema(unc::uppl::DOMAIN_OP_STATUS, oper_value,
                        oper_value.length(), DATATYPE_UINT16,
                        vect_table_attr_schema);

  DBTableSchema kt_ctr_domain_dbtableschema;
  kt_ctr_domain_dbtableschema.set_table_name(unc::uppl::CTR_DOMAIN_TABLE);
  kt_ctr_domain_dbtableschema.set_primary_keys(vect_prim_keys);
  row_list.push_back(vect_table_attr_schema);
  kt_ctr_domain_dbtableschema.set_row_list(row_list);

  // Call ODBCManager and read
  ODBCM_RC_STATUS read_db_status =
      physical_layer->get_odbc_manager()->GetOneRow(
          (unc_keytype_datatype_t)data_type,
          kt_ctr_domain_dbtableschema, db_conn);
  if (read_db_status != ODBCM_RC_SUCCESS &&
      read_db_status != ODBCM_RC_RECORD_NOT_FOUND) {
    // log error
    pfc_log_error("oper_status read operation failed");
    return UNC_UPPL_RC_ERR_DB_GET;
  }

  // read the oper_status value
  list < vector<TableAttrSchema> >& res_domain_row_list =
      kt_ctr_domain_dbtableschema.get_row_list();
  list < vector<TableAttrSchema> > :: iterator res_domain_iter =
      res_domain_row_list.begin();
  // populate IPC value structure based on the response received from DB
  for (; res_domain_iter!= res_domain_row_list.end(); ++res_domain_iter) {
    vector<TableAttrSchema> res_ctr_domain_table_attr_schema =
        (*res_domain_iter);
    vector<TableAttrSchema> :: iterator vect_domain_iter =
        res_ctr_domain_table_attr_schema.begin();
    for (; vect_domain_iter != res_ctr_domain_table_attr_schema.end();
        ++vect_domain_iter) {
      // populate values from ctr_domain_table
      TableAttrSchema tab_schema = (*vect_domain_iter);
      ODBCMTableColumns attr_name = tab_schema.table_attribute_name;
      string attr_value;
      if (attr_name == unc::uppl::DOMAIN_OP_STATUS) {
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
 * @Description : This function updates the oper status in db
 * @param[in]   : data_type - indicates the data base type
 *                key_struct - key instance of kt domain
 *                oper_status - oper status value to be updated in DB
 * @return      : UNC_RC_SUCCESS - if the oper status is updation in db is
 *                success or UNC_UPPL_RC_ERR*
 **/
UncRespCode Kt_Ctr_Domain::SetOperStatus(OdbcmConnectionHandler *db_conn,
                                            uint32_t data_type,
                                            void* key_struct,
                                            UpplDomainOperStatus oper_status) {
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  key_ctr_domain *obj_key_ctr_domain=
      reinterpret_cast<key_ctr_domain_t*>(key_struct);
  vector<TableAttrSchema> vect_table_attr_schema;
  list < vector<TableAttrSchema> > row_list;
  vector<string> vect_prim_keys;
  vect_prim_keys.push_back(CTR_NAME_STR);
  string controller_name = (const char*)obj_key_ctr_domain->
      ctr_key.controller_name;

  PhyUtil::FillDbSchema(unc::uppl::CTR_NAME, controller_name,
                        controller_name.length(), DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);

  string domain_name = (const char*)obj_key_ctr_domain->domain_name;
  if (!domain_name.empty()) {
    vect_prim_keys.push_back(DOMAIN_NAME_STR);
    PhyUtil::FillDbSchema(unc::uppl::DOMAIN_NAME, domain_name,
                          domain_name.length(), DATATYPE_UINT8_ARRAY_32,
                          vect_table_attr_schema);
  }

  string oper_value = PhyUtil::uint8tostr(oper_status);
  PhyUtil::FillDbSchema(unc::uppl::DOMAIN_OP_STATUS, oper_value,
                        oper_value.length(), DATATYPE_UINT16,
                        vect_table_attr_schema);

  DBTableSchema kt_ctr_domain_dbtableschema;
  kt_ctr_domain_dbtableschema.set_table_name(unc::uppl::CTR_DOMAIN_TABLE);
  kt_ctr_domain_dbtableschema.set_primary_keys(vect_prim_keys);
  row_list.push_back(vect_table_attr_schema);
  kt_ctr_domain_dbtableschema.set_row_list(row_list);

  // Call ODBCManager and update
  ODBCM_RC_STATUS update_db_status =
      physical_layer->get_odbc_manager()->UpdateOneRow(
          (unc_keytype_datatype_t)data_type,
          kt_ctr_domain_dbtableschema, db_conn, true);
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
    if (read_status == UNC_RC_SUCCESS && data_type != UNC_DT_IMPORT) {
      val_ctr_domain_st old_val_ctr_domain, new_val_ctr_domain;
      memset(&old_val_ctr_domain, 0, sizeof(old_val_ctr_domain));
      memset(&new_val_ctr_domain, 0, sizeof(new_val_ctr_domain));
      old_val_ctr_domain.oper_status = old_oper_status;
      old_val_ctr_domain.valid[kIdxDomainStOperStatus] = UNC_VF_VALID;
      new_val_ctr_domain.oper_status = oper_status;
      new_val_ctr_domain.valid[kIdxDomainStOperStatus] = UNC_VF_VALID;
      int err = 0;
      // Send notification to Northbound
      ServerEvent ser_evt((pfc_ipcevtype_t)UPPL_EVENTS_KT_CTR_DOMAIN, err);
      northbound_event_header rsh = {static_cast<uint32_t>(UNC_OP_UPDATE),
          data_type,
          static_cast<uint32_t>(UNC_KT_CTR_DOMAIN)};
      err = PhyUtil::sessOutNBEventHeader(ser_evt, rsh);
      err |= ser_evt.addOutput(*obj_key_ctr_domain);
      err |= ser_evt.addOutput(new_val_ctr_domain);
      err |= ser_evt.addOutput(old_val_ctr_domain);
      if (err == 0) {
        PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
        // Notify operstatus modifications
        UncRespCode status = (UncRespCode) physical_layer
            ->get_ipc_connection_manager()->SendEvent(&ser_evt, controller_name,
                                                    UPPL_EVENTS_KT_CTR_DOMAIN);
        pfc_log_debug("Event notification status %d", status);
      } else {
        pfc_log_error("Server Event addOutput failed");
      }
    }
  }
  return UNC_RC_SUCCESS;
}

/** 
 * @Description : This function checks whether the DOMAIN_id exists in DB
 * @param[in]   : data type - UNC_DT_* - indicates the data base type
 *                key values - vector of type string to store the primary
 *                key values
 * @return      : UNC_RC_SUCCESS - incase the row exists in domain table or
 *                UNC_UPPL_RC_ERR* is returned
 **/
UncRespCode Kt_Ctr_Domain::IsKeyExists(OdbcmConnectionHandler *db_conn,
                                          unc_keytype_datatype_t data_type,
                                          const vector<string> &key_values) {
  pfc_log_debug("Inside IsKeyExists");
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  UncRespCode check_status = UNC_RC_SUCCESS;
  if (key_values.empty()) {
    // No key given, return failure
    pfc_log_error("No key given. Returning error");
    return UNC_UPPL_RC_ERR_BAD_REQUEST;
  }

  string controller_name = key_values[0];
  string domain_name = key_values[1];

  // Structure used to send request to ODBC
  DBTableSchema kt_ctr_domain_dbtableschema;

  // Construct Primary key list
  vector<string> vect_prim_keys;
  vect_prim_keys.push_back(CTR_NAME_STR);
  vect_prim_keys.push_back(DOMAIN_NAME_STR);

  // Construct TableAttrSchema structure
  // TableAttrSchema holds table_name, primary key, attr_name
  vector<TableAttrSchema> vect_table_attr_schema;
  list< vector<TableAttrSchema> > row_list;
  // controller_name
  PhyUtil::FillDbSchema(unc::uppl::CTR_NAME, controller_name,
                        controller_name.length(),
                        DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);

  // Domain_name
  PhyUtil::FillDbSchema(unc::uppl::DOMAIN_NAME, domain_name,
                        domain_name.length(), DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);

  kt_ctr_domain_dbtableschema.set_table_name(unc::uppl::CTR_DOMAIN_TABLE);
  kt_ctr_domain_dbtableschema.set_primary_keys(vect_prim_keys);
  row_list.push_back(vect_table_attr_schema);
  kt_ctr_domain_dbtableschema.set_row_list(row_list);

  // Send request to ODBC for ctr_domain_table
  ODBCM_RC_STATUS check_db_status = physical_layer->get_odbc_manager()->\
      IsRowExists(data_type, kt_ctr_domain_dbtableschema, db_conn);
  if (check_db_status == ODBCM_RC_CONNECTION_ERROR) {
    // log error to log daemon
    pfc_log_error("DB connection not available or cannot access DB");
    check_status = UNC_UPPL_RC_ERR_DB_ACCESS;
  } else if (check_db_status == ODBCM_RC_ROW_EXISTS) {
    pfc_log_debug("DB returned success for Row exists");
    pfc_log_debug("Checking .db_return_status_ %d with %d",
                  kt_ctr_domain_dbtableschema.db_return_status_,
                  static_cast<int>(DELETED));
    if (kt_ctr_domain_dbtableschema.db_return_status_ != DELETED) {
      pfc_log_debug("DB returned success for Row exists");
    } else {
      pfc_log_debug("DB Returned failure for IsRowExists");
      check_status = UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE;
    }
  } else {
    pfc_log_info("DB Returned failure for IsRowExists");
    check_status = UNC_UPPL_RC_ERR_DB_GET;
  }
  pfc_log_debug("check_status = %d", check_status);
  return check_status;
}


/**
 * @Description : This function is used to populate the db schema using the
 *                given key struct, value struct, data_type, operation,
 *                option1 and option
 * @param[in]   : kt_ctr_domain_dbtableschema - object of type DBTableSchema
 *                key_struct - key instance of ctr domain type
 *                val_struct - value structure of ctr domain type
 *                operation_type - specifies the operation type i.e
 *                                 UNC_OP_READ or UNC_OP_READ_SIBLING_BEGIN etc
 *                option1/option2 - specifies any additional option for
 *                populating in DB
 *                vect_key_operations - vector of type ODBCMOperator
 *                old_value_struct - holds the old value structure of the
 *                ctr domain key type
 *                row_status - enum indicating the row status of controller
 *                domain type entries in db
 *                is_filtering/is_state - bool variables
 * @return      : None
 **/

void Kt_Ctr_Domain::PopulateDBSchemaForKtTable(
    OdbcmConnectionHandler *db_conn,
    DBTableSchema &kt_ctr_domain_dbtableschema,
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
  pfc_log_debug("Inside PopulateDBSchemaForKtTable for Kt_ctr_domain");
  // Construct Primary key list
  vector<string> vect_prim_keys;
  vect_prim_keys.push_back(CTR_NAME_STR);

  // Construct TableAttrSchema structuree
  // TableAttrSchema holds table_name, primary key, attr_name
  vector<TableAttrSchema> vect_table_attr_schema;
  list < vector<TableAttrSchema> > row_list;

  key_ctr_domain *obj_key_ctr_domain=
      reinterpret_cast<key_ctr_domain*>(key_struct);
  val_ctr_domain_t *obj_val_ctr_domain = NULL;
  val_ctr_domain_st_t *obj_val_ctr_domain_st = NULL;
  unsigned int valid_value_struct = UNC_VF_VALID;

  if (is_state == PFC_TRUE) {
    obj_val_ctr_domain_st = reinterpret_cast<val_ctr_domain_st*>(val_struct);
    obj_val_ctr_domain = &obj_val_ctr_domain_st->domain;
    valid_value_struct = PhyUtil::uint8touint(
        obj_val_ctr_domain_st->valid[kIdxDomainStDomain]);
    pfc_log_debug("Valid Value: %d", valid_value_struct);
  } else {
    obj_val_ctr_domain = reinterpret_cast<val_ctr_domain*>(val_struct);
  }
  stringstream valid;

  // controller_name
  string controller_name = (const char*)obj_key_ctr_domain->
      ctr_key.controller_name;
  PhyUtil::FillDbSchema(unc::uppl::CTR_NAME, controller_name,
                        controller_name.length(),
                        DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);

  // Domain_name
  string domain_name = (const char*)obj_key_ctr_domain->domain_name;
  if (operation_type == UNC_OP_READ_SIBLING_BEGIN ||
      operation_type == UNC_OP_READ_SIBLING_COUNT) {
    domain_name = "";
  }

  PhyUtil::FillDbSchema(unc::uppl::DOMAIN_NAME, domain_name,
                        domain_name.length(), DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);
  val_ctr_domain_st_t val_ctr_domain_valid_st;
  memset(&val_ctr_domain_valid_st, 0, sizeof(val_ctr_domain_st_t));
  if (operation_type == UNC_OP_UPDATE) {
    // get valid array for update req
    pfc_log_debug("Get Valid value from Update Valid Flag");
    GetCtrDomainValidFlag(db_conn, key_struct,
                          val_ctr_domain_valid_st, data_type);
  }
  uint16_t valid_val = 0, prev_db_val = 0;
  string value;
  // Type
  if (obj_val_ctr_domain != NULL && valid_value_struct == UNC_VF_VALID) {
    valid_val = PhyUtil::uint8touint(obj_val_ctr_domain->valid[kIdxDomainType]);
    value = PhyUtil::uint8tostr(obj_val_ctr_domain->type);
    prev_db_val = PhyUtil::uint8touint(
        val_ctr_domain_valid_st.domain.valid[kIdxDomainType]);
  } else {
    valid_val = UPPL_NO_VAL_STRUCT;
  }
  PhyUtil::FillDbSchema(unc::uppl::DOMAIN_TYPE, DOMAIN_TYPE_STR, value,
                        value.length(), DATATYPE_UINT16,
                        operation_type, valid_val, prev_db_val,
                        vect_table_attr_schema, vect_prim_keys, valid);
  value.clear();
  // Description
  if (obj_val_ctr_domain != NULL && valid_value_struct == UNC_VF_VALID) {
    valid_val = PhyUtil::uint8touint(obj_val_ctr_domain->
                                     valid[kIdxDomainDescription]);
    value = (const char*)obj_val_ctr_domain->description;
    prev_db_val = PhyUtil::uint8touint(
        val_ctr_domain_valid_st.domain.valid[kIdxDomainDescription]);
  } else {
    valid_val = UPPL_NO_VAL_STRUCT;
  }
  PhyUtil::FillDbSchema(unc::uppl::DOMAIN_DESCRIPTION, DOMAIN_DESCRIPTION_STR,
                        value, value.length(), DATATYPE_UINT8_ARRAY_128,
                        operation_type, valid_val, prev_db_val,
                        vect_table_attr_schema, vect_prim_keys, valid);
  prev_db_val = 0;
  value.clear();
  // oper_status will be set as 1 by default in DB during create
  if (operation_type == UNC_OP_CREATE) {
    if ((unc_keytype_datatype_t)data_type == UNC_DT_CANDIDATE) {
      value = "1";
    }
  }
  valid_val = UPPL_NO_VAL_STRUCT;
  if (operation_type == UNC_OP_UPDATE && is_state == PFC_TRUE &&
      obj_val_ctr_domain_st != NULL) {
    valid_val = PhyUtil::uint8touint(obj_val_ctr_domain_st->
                                     valid[kIdxDomainStOperStatus]);
    value = PhyUtil::uint8tostr(obj_val_ctr_domain_st->oper_status);
    PhyUtil::FillDbSchema(unc::uppl::DOMAIN_OP_STATUS, DOMAIN_OP_STATUS_STR,
                          value, value.length(), DATATYPE_UINT16,
                          operation_type, valid_val, prev_db_val,
                          vect_table_attr_schema, vect_prim_keys, valid);
  } else if (is_state == PFC_TRUE) {
    // Add oper_status as well if state value structure is passed
    valid_val = UNC_VF_VALID;
    value = PhyUtil::uint8tostr(obj_val_ctr_domain_st->oper_status);
    PhyUtil::FillDbSchema(unc::uppl::DOMAIN_OP_STATUS, DOMAIN_OP_STATUS_STR,
                          value, value.length(), DATATYPE_UINT16,
                          operation_type, valid_val, prev_db_val,
                          vect_table_attr_schema, vect_prim_keys, valid);
  } else if (operation_type >= UNC_OP_READ) {
    PhyUtil::FillDbSchema(unc::uppl::DOMAIN_OP_STATUS, DOMAIN_OP_STATUS_STR,
                          value, value.length(), DATATYPE_UINT16,
                          operation_type, valid_val, prev_db_val,
                          vect_table_attr_schema, vect_prim_keys, valid);
  } else {
    valid << UNC_VF_VALID;
  }
  stringstream dummy_valid;
  valid_val = UPPL_NO_VAL_STRUCT;
  // valid
  PhyUtil::FillDbSchema(unc::uppl::DOMAIN_VALID, DOMAIN_VALID_STR, valid.str(),
                        ODBCM_SIZE_3, DATATYPE_UINT8_ARRAY_3,
                        operation_type, valid_val, prev_db_val,
                        vect_table_attr_schema, vect_prim_keys, dummy_valid);
  // cs_attr_status
  stringstream attr_status;
  for (unsigned int index = 0; index < ODBCM_SIZE_3; ++index) {
    attr_status << CREATED;
  }
  PhyUtil::FillDbSchema(unc::uppl::DOMAIN_CS_ATTR, attr_status.str(),
                        ODBCM_SIZE_3, DATATYPE_UINT8_ARRAY_3,
                        vect_table_attr_schema);
  // cs_row status
  if (is_filtering == true) {
    vect_prim_keys.push_back(DOMAIN_CS_ROW_STATUS_STR);
  }

  value = PhyUtil::uint8tostr(row_status);
  if (operation_type >= UNC_OP_READ) {
    PhyUtil::FillDbSchema(unc::uppl::DOMAIN_CS_ROW_STATUS, value,
                          value.length(), DATATYPE_UINT16,
                          vect_table_attr_schema);
  }
  vect_prim_keys.push_back(DOMAIN_NAME_STR);
  PhyUtil::reorder_col_attrs(vect_prim_keys, vect_table_attr_schema);
  kt_ctr_domain_dbtableschema.set_table_name(unc::uppl::CTR_DOMAIN_TABLE);
  kt_ctr_domain_dbtableschema.set_primary_keys(vect_prim_keys);
  row_list.push_back(vect_table_attr_schema);
  kt_ctr_domain_dbtableschema.set_row_list(row_list);
  return;
}

/**
 * @Description : This function is used to read KT_CTR_DOMAIN instance in
 *                database table using key_ctr provided in IPC request
 *                The IPC response would be filled in IPC session
 * @param[in]   : ipc session id - ipc session id used for TC validation
 *                configuration id - configuration id used for TC validation
 *                key_struct - key instance of ctr domain
 *                value_struct - value structure instance of ctr domain type
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
UncRespCode Kt_Ctr_Domain::PerformRead(OdbcmConnectionHandler *db_conn,
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
  key_ctr_domain *obj_key_ctr_domain=
      reinterpret_cast<key_ctr_domain*>(key_struct);
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
    err |= sess.addOutput((uint32_t)UNC_KT_CTR_DOMAIN);
    err |= sess.addOutput(*obj_key_ctr_domain);
    if (err != 0) {
      pfc_log_error("addOutput failed for physical_response_header");
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
    err |= sess.addOutput((uint32_t)UNC_KT_CTR_DOMAIN);
    err |= sess.addOutput(*obj_key_ctr_domain);
    if (err != 0) {
      pfc_log_error("addOutput failed for physical_response_header");
      return UNC_UPPL_RC_ERR_IPC_WRITE_ERROR;
    }
    return UNC_RC_SUCCESS;
  }
  UncRespCode read_status = UNC_RC_SUCCESS;
  // Read from DB directly for all data types
  vector<key_ctr_domain> vect_domain_id;
  vector<val_ctr_domain_st> vect_val_ctr_domain_st;
  read_status = ReadDomainValFromDB(db_conn, key_struct,
                                    val_struct,
                                    data_type,
                                    operation_type,
                                    max_rep_ct,
                                    vect_val_ctr_domain_st,
                                    vect_domain_id);
  rsh.result_code = read_status;
  rsh.max_rep_count = max_rep_ct;
  if (read_status == UNC_RC_SUCCESS) {
    for (unsigned int index = 0; index < vect_domain_id.size();
        ++index) {
      string domain_name = (const char *)vect_domain_id[index].domain_name;
      if (data_type == UNC_DT_CANDIDATE &&
          vect_val_ctr_domain_st[index].domain.cs_row_status == DELETED) {
        pfc_log_debug("Ignoring DELETED entry %s", domain_name.c_str());
        if (rsh.max_rep_count > 0)
          rsh.max_rep_count--;
        continue;
      }
    }
    if (rsh.max_rep_count == 0) {
      rsh.result_code = UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE;
      int err = PhyUtil::sessOutRespHeader(sess, rsh);
      err |= sess.addOutput((uint32_t)UNC_KT_CONTROLLER);
      err |= sess.addOutput(*obj_key_ctr_domain);
      if (err != 0) {
        pfc_log_error("addOutput failed for physical_response_header");
        return UNC_UPPL_RC_ERR_IPC_WRITE_ERROR;
      }
      return UNC_RC_SUCCESS;
    }
    int err = PhyUtil::sessOutRespHeader(sess, rsh);
    if (err != 0) {
      pfc_log_error("addOutput failed for physical_response_header");
      return UNC_UPPL_RC_ERR_IPC_WRITE_ERROR;
    }
    pfc_log_debug("Vector size of Domain Id: %"
                  PFC_PFMT_SIZE_T, vect_domain_id.size());
    for (unsigned int index = 0; index < vect_domain_id.size();
        ++index) {
      string domain_name = (const char *)vect_domain_id[index].domain_name;
      if (data_type == UNC_DT_CANDIDATE &&
          vect_val_ctr_domain_st[index].domain.cs_row_status == DELETED) {
        pfc_log_debug("Ignoring DELETED entry %s", domain_name.c_str());
        continue;
      }
      pfc_log_debug("Adding domain name: %s", domain_name.c_str());
      err |= sess.addOutput((uint32_t)UNC_KT_CTR_DOMAIN);
      err |= sess.addOutput(vect_domain_id[index]);
      if ((unc_keytype_datatype_t)data_type == UNC_DT_STATE) {
        err |= sess.addOutput(vect_val_ctr_domain_st[index]);
      } else {
        err |= sess.addOutput(vect_val_ctr_domain_st[index].domain);
      }
      if (index < vect_domain_id.size() -1) {
        err |= sess.addOutput();
      }
      if (err !=0) {
        pfc_log_error("addOutput failed for physical_response_header");
        return UNC_UPPL_RC_ERR_IPC_WRITE_ERROR;
      }
    }
  } else {
    pfc_log_error("Read operation failed with %d", read_status);
    rsh.result_code = read_status;
    int err = PhyUtil::sessOutRespHeader(sess, rsh);
    err |= sess.addOutput((uint32_t)UNC_KT_CTR_DOMAIN);
    err |= sess.addOutput(*obj_key_ctr_domain);
    if (err != 0) {
      pfc_log_error("addOutput failed for physical_response_header");
      return UNC_UPPL_RC_ERR_IPC_WRITE_ERROR;
    }
  }
  pfc_log_debug("Return value for read operation %d", read_status);
  return read_status;
}

/** 
 * @Description : This function is used to read KT_CTR_DOMAIN instance in
 *                database table using key_ctr provided in IPC request and
 *                operation_type
 * @param[in]   : key_struct - key instance of ctr domain 
 *                value_struct - value structure of ctr domain type
 *                data_type - indicates the data base type
 *                operation_type - indicates the operation type UNC_OP*
 *                max_rep_ct - indicates the maximum repetition count
 *                vect_val_ctr_domain_st- vector to store the
 *                val_ctr_domain_st_t structure
 *                domain_id - vector of type key_ctr_domain to store the
 *                domain id
 * @return      : UNC_RC_SUCCESS - read operation is success
 *                UNC_UPPL_RC_ERR_DB_GET - read operation is failure
 **/
UncRespCode Kt_Ctr_Domain::ReadDomainValFromDB(
    OdbcmConnectionHandler *db_conn,
    void* key_struct,
    void* val_struct,
    uint32_t data_type,
    uint32_t operation_type,
    uint32_t &max_rep_ct,
    vector<val_ctr_domain_st> &vect_val_ctr_domain_st,
    vector<key_ctr_domain> &domain_id) {
  if (operation_type < UNC_OP_READ) {
    // Unsupported operation type for this function
    return UNC_RC_SUCCESS;
  }
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  UncRespCode read_status = UNC_RC_SUCCESS;
  ODBCM_RC_STATUS read_db_status = ODBCM_RC_SUCCESS;

  // Common structures that will be used to send query to ODBC
  // Structure used to send request to ODBC
  DBTableSchema kt_ctr_domain_dbtableschema;
  void *old_val_struct;
  vector<ODBCMOperator> vect_key_operations;
  PopulateDBSchemaForKtTable(db_conn, kt_ctr_domain_dbtableschema,
                             key_struct,
                             val_struct,
                             operation_type, data_type, 0, 0,
                             vect_key_operations, old_val_struct,
                             NOTAPPLIED, false, PFC_FALSE);

  if (operation_type == UNC_OP_READ) {
    read_db_status = physical_layer->get_odbc_manager()->
        GetOneRow((unc_keytype_datatype_t)data_type,
                  kt_ctr_domain_dbtableschema, db_conn);
  } else {
    read_db_status = physical_layer->get_odbc_manager()->
        GetBulkRows((unc_keytype_datatype_t)data_type, max_rep_ct,
                    kt_ctr_domain_dbtableschema,
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
    pfc_log_error("Read operation has failed with error %d", read_db_status);
    return read_status;
  }
  pfc_log_debug("Read operation result: %d", read_status);
  FillDomainValueStructure(db_conn, kt_ctr_domain_dbtableschema,
                           vect_val_ctr_domain_st,
                           max_rep_ct,
                           operation_type,
                           domain_id);
  pfc_log_debug("vect_val_ctr_domain size: %"
                PFC_PFMT_SIZE_T, vect_val_ctr_domain_st.size());
  pfc_log_debug("domain_id size: %" PFC_PFMT_SIZE_T, domain_id.size());
  if (vect_val_ctr_domain_st.empty()) {
    // Read failed , return error
    read_status = UNC_UPPL_RC_ERR_DB_GET;
    // log error to log daemon
    pfc_log_error("Read operation has failed after reading response");
    return read_status;
  }
  return read_status;
}

/** 
 * @Description : This function returns the void * of child key structures
 * @param[in]   : child class index - stores the child class index enum
 *                controller_name - string type to store the controller name
 *                domain name - string type to store the domain name
 * @return      : void * key structure
 */
void* Kt_Ctr_Domain::getChildKeyStruct(int child_class,
                                       string controller_name,
                                       string domain_name) {
  switch (child_class) {
    case KIdxLogicalPort: {
      key_logical_port_t *child_key = new key_logical_port_t;
      memset(child_key->domain_key.domain_name, '\0',
             sizeof(child_key->domain_key.domain_name));
      memcpy(child_key->domain_key.domain_name,
             domain_name.c_str(),
             domain_name.length()+1);
      memset(child_key->domain_key.ctr_key.controller_name, '\0',
             sizeof(child_key->domain_key.ctr_key.controller_name));
      memcpy(child_key->domain_key.ctr_key.controller_name,
             controller_name.c_str(),
             controller_name.length() +1);
      memset(child_key->port_id, 0, 320);
      void* obj_child_key = reinterpret_cast<void *>(child_key);
      return obj_child_key;
    }
    default: {
      pfc_log_error("Invalid index %d passed to getChildKeyStruct()",
                    child_class);
      PFC_ASSERT(PFC_FALSE);
      // Do nothing
      return NULL;
    }
  }
}

/** 
 * @Description : This function frees the void * of child key structures
 * @param[in]   : child class index - stores the child class index enum
 *                key_struct - key instance of the child class Logical port
 * @return      : None
 **/
void Kt_Ctr_Domain::FreeChildKeyStruct(int child_class,
                                       void *key_struct) {
  switch (child_class) {
    case KIdxLogicalPort: {
      key_logical_port_t *child_key =
          reinterpret_cast<key_logical_port_t *>(key_struct);
      if (child_key != NULL) {
        delete child_key;
        child_key = NULL;
      }
      return;
    }
    default: {
      pfc_log_error("Invalid index %d passed to FreeChildKeyStruct()",
                    child_class);
      PFC_ASSERT(PFC_FALSE);
      // Do nothing
      return;
    }
  }
}

/**  
 * @Description : This function populates val_ctr_domain by values retrieved
 *                from database
 * @param[in]   : kt_ctr_domain_dbtableschema - object of type DBTableSchema
 *                vect_obj_val_ctr_domain - vector of  type val_ctr_domain_st
 *                max_rep_ct - indicates the maximum repetition count
 *                operation_type - indicates the operation type
 *                vect_domain_id - vectpor of type vect_domain_id to store the
 *                domain id
 * @return      : None
 **/
void Kt_Ctr_Domain::FillDomainValueStructure(
    OdbcmConnectionHandler *db_conn,
    DBTableSchema &kt_ctr_domain_dbtableschema,
    vector<val_ctr_domain_st> &vect_obj_val_ctr_domain,
    uint32_t &max_rep_ct,
    uint32_t operation_type,
    vector<key_ctr_domain> &vect_domain_id) {
  // populate IPC value structure based on the response received from DB
  list < vector<TableAttrSchema> >& res_domain_row_list =
      kt_ctr_domain_dbtableschema.get_row_list();
  list < vector<TableAttrSchema> > :: iterator res_domain_iter =
      res_domain_row_list.begin();
  max_rep_ct = res_domain_row_list.size();
  pfc_log_debug("res_domain_row_list.size: %d", max_rep_ct);
  key_ctr_domain obj_key_ctr_domain;

  // populate IPC value structure based on the response received from DB
  for (; res_domain_iter != res_domain_row_list.end(); ++res_domain_iter) {
    vector<TableAttrSchema> res_ctr_domain_table_attr_schema =
        (*res_domain_iter);
    vector<TableAttrSchema> :: iterator vect_domain_iter =
        res_ctr_domain_table_attr_schema.begin();
    uint32_t attr_size = res_ctr_domain_table_attr_schema.size();
    pfc_log_debug("res_ctr_domain_table_attr_schema size: %d", attr_size);
    memset(&obj_key_ctr_domain, '\0', sizeof(obj_key_ctr_domain));
    val_ctr_domain_st obj_val_ctr_domain_st;
    memset(&obj_val_ctr_domain_st, 0, sizeof(obj_val_ctr_domain_st));
    val_ctr_domain obj_val_ctr_domain;
    memset(&obj_val_ctr_domain, 0, sizeof(obj_val_ctr_domain));
    // Read all attributes
    vector<int> valid_flag, cs_attr;
    for (; vect_domain_iter != res_ctr_domain_table_attr_schema.end();
        ++vect_domain_iter) {
      // populate values from ctr_domain_table
      TableAttrSchema tab_schema = (*vect_domain_iter);
      ODBCMTableColumns attr_name = tab_schema.table_attribute_name;
      string attr_value;
      switch (attr_name) {
        case unc::uppl::DOMAIN_NAME:
          PhyUtil::GetValueFromDbSchemaStr(tab_schema,
                                           obj_key_ctr_domain.domain_name,
                                           DATATYPE_UINT8_ARRAY_32);
          pfc_log_debug("domain_name: %s", reinterpret_cast<char *>
          (&obj_key_ctr_domain.domain_name));
          break;

        case unc::uppl::CTR_NAME:
          PhyUtil::GetValueFromDbSchemaStr(
              tab_schema,
              obj_key_ctr_domain.ctr_key.controller_name,
              DATATYPE_UINT8_ARRAY_32);
          pfc_log_debug("controller_name: %s", reinterpret_cast<char *>
          (&obj_key_ctr_domain.ctr_key.controller_name));
          break;

        case unc::uppl::DOMAIN_TYPE:
          PhyUtil::GetValueFromDbSchema(tab_schema, attr_value,
                                        DATATYPE_UINT16);
          obj_val_ctr_domain.type = atoi(attr_value.c_str());
          pfc_log_debug("type: %d", obj_val_ctr_domain.type);
          break;

        case unc::uppl::DOMAIN_DESCRIPTION:
          PhyUtil::GetValueFromDbSchemaStr(tab_schema,
                                           obj_val_ctr_domain.description,
                                           DATATYPE_UINT8_ARRAY_128);
          pfc_log_debug("description: %s", attr_value.c_str());
          break;

        case unc::uppl::DOMAIN_OP_STATUS:
          PhyUtil::GetValueFromDbSchema(tab_schema, attr_value,
                                        DATATYPE_UINT16);
          obj_val_ctr_domain_st.oper_status = atoi(attr_value.c_str());
          pfc_log_debug("oper_status: %s", attr_value.c_str());
          break;

        case unc::uppl::DOMAIN_VALID:
          uint8_t dmn_valid[ODBCM_SIZE_3];
          PhyUtil::GetValueFromDbSchemaStr(tab_schema, dmn_valid,
                                           DATATYPE_UINT8_ARRAY_3);
          memset(obj_val_ctr_domain.valid, 0, 2);
          FrameValidValue(reinterpret_cast<const char*> (dmn_valid),
                          obj_val_ctr_domain_st,
                          obj_val_ctr_domain);
          break;

        case unc::uppl::DOMAIN_CS_ATTR:
          uint8_t dmn_cs_attr[ODBCM_SIZE_3];
          PhyUtil::GetValueFromDbSchemaStr(tab_schema, dmn_cs_attr,
                                           DATATYPE_UINT8_ARRAY_3);
          memset(obj_val_ctr_domain.cs_attr, 0, 2);
          FrameCsAttrValue(reinterpret_cast<const char*> (dmn_cs_attr),
                           obj_val_ctr_domain);
          break;

        case unc::uppl::DOMAIN_CS_ROW_STATUS:
          PhyUtil::GetValueFromDbSchema(tab_schema, attr_value,
                                        DATATYPE_UINT16);
          obj_val_ctr_domain.cs_row_status = atoi(attr_value.c_str());
          pfc_log_debug("row_status: %s", attr_value.c_str());
          break;

        default:
          pfc_log_info("Ignoring Domain Attribute %d", attr_name);
          break;
      }
    }
    obj_val_ctr_domain_st.domain = obj_val_ctr_domain;
    vect_domain_id.push_back(obj_key_ctr_domain);
    vect_obj_val_ctr_domain.push_back(obj_val_ctr_domain_st);
    pfc_log_debug("result - vect_obj_val_ctr_domain size: %"
                  PFC_PFMT_SIZE_T, vect_obj_val_ctr_domain.size());
  }
  return;
}

/** 
 * @Description : This function reads all kt_ctr_domain with given row_status
 * @param[in]   : obj_key_struct - vector to hold the key structure of ctr
 *                domain key type
 *                row_status - indicates the row status to be queried in DB with
 * @return      : UNC_RC_SUCCESS - if the row exists in DB or UNC_UPPL_RC_ERR*
 *                if the row doesn't exists in DB
 **/
UncRespCode Kt_Ctr_Domain::GetModifiedRows(OdbcmConnectionHandler *db_conn,
                                              vector<void *> &obj_key_struct,
                                              CsRowStatus row_status) {
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  uint32_t max_rep_ct = 0;

  vector<val_ctr_domain_st> obj_value_struct;

  key_ctr_domain obj_key_ctr_domain;
  val_ctr_domain val_struct;
  memset(&obj_key_ctr_domain, 0, sizeof(key_ctr_domain));
  memset(&val_struct, 0, sizeof(val_ctr_domain));

  void *domain_key = reinterpret_cast <void *> (&obj_key_ctr_domain);
  void *domain_val = reinterpret_cast <void *> (&val_struct);

  UncRespCode read_status = UNC_RC_SUCCESS;
  ODBCM_RC_STATUS read_db_status = ODBCM_RC_SUCCESS;

  DBTableSchema kt_ctr_domain_dbtableschema;
  void *old_val_struct;
  vector<ODBCMOperator> vect_key_operations;
  PopulateDBSchemaForKtTable(db_conn, kt_ctr_domain_dbtableschema,
                             domain_key,
                             domain_val,
                             UNC_OP_READ, UNC_DT_CANDIDATE, 0, 0,
                             vect_key_operations, old_val_struct,
                             row_status,
                             true, PFC_FALSE);

  read_db_status = physical_layer->get_odbc_manager()->
      GetModifiedRows(UNC_DT_CANDIDATE, kt_ctr_domain_dbtableschema, db_conn);

  if (read_db_status == ODBCM_RC_RECORD_NOT_FOUND) {
    pfc_log_debug("No record to read");
    read_status = UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE;
    return read_status;
  } else if (read_db_status == ODBCM_RC_CONNECTION_ERROR) {
    read_status = UNC_UPPL_RC_ERR_DB_ACCESS;
    return read_status;
  } else if (read_db_status != ODBCM_RC_SUCCESS) {
    read_status = UNC_UPPL_RC_ERR_DB_GET;
    return read_status;
  }
  vector<key_ctr_domain> vect_domain_id;

  FillDomainValueStructure(db_conn, kt_ctr_domain_dbtableschema,
                           obj_value_struct,
                           max_rep_ct,
                           UNC_OP_READ_BULK,
                           vect_domain_id);

  // Fill key structures
  // Value structures is available in obj_val_struct
  vector<key_ctr_domain> ::iterator vect_iter = vect_domain_id.begin();
  for (; vect_iter != vect_domain_id.end(); ++vect_iter) {
    key_ctr_domain_t *obj_key_ctr_domain = new key_ctr_domain_t;
    memset(obj_key_ctr_domain->ctr_key.controller_name, '\0', 32);
    memset(obj_key_ctr_domain->domain_name, '\0', 32);
    memcpy(obj_key_ctr_domain->ctr_key.controller_name,
           (const char *)((*vect_iter).ctr_key.controller_name),
           sizeof((*vect_iter).ctr_key.controller_name));
    memcpy(obj_key_ctr_domain->domain_name,
           (const char *)((*vect_iter).domain_name),
           sizeof((*vect_iter).domain_name));
    pfc_log_debug("KT_DOMAIN: controller name:  %s\n",
                  obj_key_ctr_domain->ctr_key.controller_name);
    pfc_log_debug("KT_DOMAIN: domain name is %s\n",
                  obj_key_ctr_domain->domain_name);
    void *key_struct = reinterpret_cast<void *>(obj_key_ctr_domain);
    obj_key_struct.push_back(key_struct);
  }
  return read_status;
}

/** 
 * @Description : This function populates the values to be used for attribute
 *                validation
 * @param[in] : None
 * @return    : None
 **/
void Kt_Ctr_Domain::Fill_Attr_Syntax_Map() {
  map<string, Kt_Class_Attr_Syntax> attr_syntax_map;
  Kt_Class_Attr_Syntax objKeyAttrSyntax =
  { PFC_IPCTYPE_UINT8, 0, 0, 1, 32, true, "" };
  attr_syntax_map[DOMAIN_NAME_STR] = objKeyAttrSyntax;

  Kt_Class_Attr_Syntax objKeyAttrSyntax1 =
  { PFC_IPCTYPE_UINT8, 0, 0, 1, 32, true, "" };
  attr_syntax_map[CTR_NAME_STR] = objKeyAttrSyntax1;

  Kt_Class_Attr_Syntax objAttrTypeSyntax =
  { PFC_IPCTYPE_UINT8, 0, 3, 0, 0, true, "" };
  attr_syntax_map[DOMAIN_TYPE_STR] = objAttrTypeSyntax;

  Kt_Class_Attr_Syntax objAttrDescSyntax =
  { PFC_IPCTYPE_STRING, 0, 0, 0, 128, false, "" };
  attr_syntax_map[DOMAIN_DESCRIPTION_STR] = objAttrDescSyntax;

  Kt_Class_Attr_Syntax objAttrValidSyntax =
  { PFC_IPCTYPE_STRING, 0, 0, 0, 2, false, "" };
  attr_syntax_map[DOMAIN_VALID_STR] = objAttrValidSyntax;

  Kt_Class_Attr_Syntax objAttrCsRowSyntax =
  { PFC_IPCTYPE_STRING, 0, 3, 0, 0, false, "" };
  attr_syntax_map[DOMAIN_CS_ROW_STATUS_STR] = objAttrCsRowSyntax;

  Kt_Class_Attr_Syntax objAttrCsAttrSyntax =
  { PFC_IPCTYPE_STRING, 0, 0, 1, 2, false, "" };
  attr_syntax_map[DOMAIN_CS_ATTR_STR] = objAttrCsAttrSyntax;
  attr_syntax_map_all[UNC_KT_CTR_DOMAIN] = attr_syntax_map;
}

/** 
 * @Description : This function reads the valid flag from DB
 * @param[in]   : key_struct - key instanc of ctr domain
 *                val_ctr_domain_valid_st - structure variable of type
 *                val_ctr_domain_st_t
 * @return      : UNC_RC_SUCCESS - if the ctr domain valid flag read is
 *                success else UNC_UPPL_RC_ERR_*
 **/
UncRespCode Kt_Ctr_Domain::GetCtrDomainValidFlag(
    OdbcmConnectionHandler *db_conn,
    void *key_struct,
    val_ctr_domain_st_t &val_ctr_domain_valid_st,
    uint32_t data_type) {
  UncRespCode return_code = UNC_RC_SUCCESS;
  vector<void *> vect_key_ctr_domain;
  vect_key_ctr_domain.push_back(key_struct);
  vector<void *> vect_val_ctr_domain;
  return_code = ReadInternal(db_conn, vect_key_ctr_domain, vect_val_ctr_domain,
                             data_type, UNC_OP_READ);
  if (return_code == UNC_RC_SUCCESS) {
    val_ctr_domain_st_t *obj_new_ctr_domain_val =
        reinterpret_cast<val_ctr_domain_st_t*>(vect_val_ctr_domain[0]);
    if (obj_new_ctr_domain_val != NULL) {
      val_ctr_domain_valid_st = *obj_new_ctr_domain_val;

      delete obj_new_ctr_domain_val;
      obj_new_ctr_domain_val = NULL;
      key_ctr_domain_t *domain_key = reinterpret_cast<key_ctr_domain_t*>
      (vect_key_ctr_domain[0]);
      if (domain_key != NULL) {
        delete domain_key;
        domain_key = NULL;
      }
    } else {
      pfc_log_debug("update domain valid ret null val");
    }
  }
  return return_code;
}

/** 
 * @Description : This function converts the valid value from db to uint8
 * @param[in]   : attr_value - string to store attribute value
 *                obj_val_ctr_domain_st/obj_val_ctr_domain - struct variables
 *                of the ctr domain val structure
 * @return      : None
 **/
void Kt_Ctr_Domain::FrameValidValue(string attr_value,
                                    val_ctr_domain_st &obj_val_ctr_domain_st,
                                    val_ctr_domain_t &obj_val_ctr_domain) {
  for (unsigned int i = 0; i < 2 ; ++i) {
    if (attr_value[i] < 48) {
      obj_val_ctr_domain.valid[i] = attr_value[i];
    } else {
      obj_val_ctr_domain.valid[i] = attr_value[i] - 48;
    }
  }
  memset(obj_val_ctr_domain_st.valid, 0, 2);
  obj_val_ctr_domain_st.valid[kIdxDomainStDomain] = UNC_VF_VALID;
  if (attr_value[2] < 48) {
    obj_val_ctr_domain_st.valid[kIdxDomainStOperStatus] = attr_value[2];
  } else {
    obj_val_ctr_domain_st.valid[kIdxDomainStOperStatus] =
        attr_value[2] - 48;
  }
  return;
}

/** 
 * @Description : This function converts the CsAttr value value from db to uint8
 * @param[in]   : attr_value - string to store attribute value
 *                obj_val_ctr_domain - struct variables of the
 *                ctr domain val structure
 * @return      : None
 **/
void Kt_Ctr_Domain::FrameCsAttrValue(string attr_value,
                                     val_ctr_domain_t &obj_val_ctr_domain) {
  for (unsigned int i = 0; i < 2 ; ++i) {
    if (attr_value[i] < 48) {
      obj_val_ctr_domain.cs_attr[i] = attr_value[i];
    } else {
      obj_val_ctr_domain.cs_attr[i] = attr_value[i] - 48;
    }
  }
  return;
}
