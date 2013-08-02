/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/**
 * @brief   KT Controller implementation
 * @file    itc_kt_controller.cc
 *
 */

#include "itc_kt_controller.hh"
#include "itc_kt_root.hh"
#include "itc_kt_switch.hh"
#include "itc_kt_port.hh"
#include "itc_kt_ctr_domain.hh"
#include "itc_kt_logicalport.hh"
#include "itc_kt_link.hh"
#include "itc_kt_boundary.hh"
#include "odbcm_db_varbind.hh"
#include "unc/upll_svc.h"
#include "unc/upll_errno.h"
#include "ipct_util.hh"
using unc::uppl::PhysicalLayer;
using unc::uppl::ODBCMUtils;

/** Constructor
 * * @Description : This function initializes member variables
 * and fills the attribute syntax map used for validation
 * * * @param[in] : None
 * * * @return    : None
 * */
Kt_Controller::Kt_Controller() {
  // Populate structure to be used for syntax validation
  Fill_Attr_Syntax_Map();
  /* Parent and child instances will be initialized inside
   * member functions whenever required */
  parent = NULL;
  for (int i = 0; i < KT_CONTROLLER_CHILD_COUNT; ++i) {
    child[i] = NULL;
  }
}

/** Destructor
 * * @Description : This function frees the parent and child key types
 * instances for kt_controller
 * * * @param[in] : None
 * * * @return    : None
 * */
Kt_Controller::~Kt_Controller() {
  if (parent != NULL) {
    delete parent;
    parent = NULL;
  }
  for (int i = 0; i < KT_CONTROLLER_CHILD_COUNT; ++i) {
    if (child[i] != NULL) {
      delete child[i];
      child[i] = NULL;
    }
  }
}

/** GetChildClassPointer
 *  * @Description : This function creates a new child class instance
 *   class of KtController based on index passed
 *  * @param[in] : KIndex - Child class index enum
 *  * @return    : Kt_Base* - The child class pointer
 */
Kt_Base* Kt_Controller::GetChildClassPointer(KtControllerChildClass KIndex) {
  switch (KIndex) {
    case KIdxDomain: {
      if (child[KIndex] == NULL) {
        child[KIndex] = new Kt_Ctr_Domain();
      }
      break;
    }
    case KIdxSwitch: {
      if (child[KIndex] == NULL) {
        child[KIndex] = new Kt_Switch();
      }
      break;
    }
    case KIdxLink: {
      if (child[KIndex] == NULL) {
        child[KIndex] = new Kt_Link();
      }
      break;
    }
    default: {
      pfc_log_info("Invalid index %d passed to GetChildClassPointer()",
                   KIndex);
      PFC_ASSERT(PFC_FALSE);
      return NULL;
    }
  }
  return child[KIndex];
}


/** Create
 * * @Description : This function creates a new row of KT_Controller in
 * candidate controller table.
 * * @param[in] : session_id - ipc session id used for TC validation
 * configuration_id - configuration id used for TC validation
 * key_struct - the key for the new kt controller instance
 * value_struct - the values for the new kt controller instance
 * data_type - UNC_DT_* , Create only allowed in candidate
 * sess - ipc server session where the response has to be added
 * * @return    : UPPL_RC_SUCCESS is returned when the response
 * is added to ipc session successfully.
 * UPPL_RC_ERR_* is returned when ipc response could not be added to sess.
 * */
UpplReturnCode Kt_Controller::Create(uint32_t session_id,
                                     uint32_t configuration_id,
                                     void* key_struct,
                                     void* val_struct,
                                     uint32_t data_type,
                                     ServerSession &sess) {
  UpplReturnCode create_status = UPPL_RC_SUCCESS;
  // Check whether operation is allowed on the given DT type
  if ((unc_keytype_datatype_t)data_type != UNC_DT_CANDIDATE) {
    pfc_log_error("Create operation is invoked on unsupported data type %d",
                  data_type);
    create_status = UPPL_RC_ERR_OPERATION_NOT_ALLOWED;
  } else {
    val_ctr_t obj_val_ctr = *(reinterpret_cast<val_ctr_t*>(val_struct));
    if (obj_val_ctr.type == UNC_CT_UNKNOWN) {
      create_status = ValidateUnknownCtrlrScalability(key_struct,
                                                      obj_val_ctr.type,
                                                      data_type);
    }
    if (create_status == UPPL_RC_SUCCESS) {
      create_status = CreateKeyInstance(key_struct,
                                        val_struct,
                                        data_type,
                                        UNC_KT_CONTROLLER);
      pfc_log_debug("CreateKeyInstance returned with status %d", create_status);
    }
  }
  if (create_status == UPPL_RC_SUCCESS) {
    UpplReturnCode send_status = UPPL_RC_SUCCESS;
    // Sending the Created Controller  Information to Logical Layer
    send_status = SendUpdatedControllerInfoToUPLL(UNC_DT_CANDIDATE,
                                                  UNC_OP_CREATE,
                                                  UNC_KT_CONTROLLER,
                                                  key_struct,
                                                  val_struct);
    pfc_log_info("Sending the Controller info to UPLL, status is %d",
                 send_status);
  }
  key_ctr_t *obj_key_ctr= reinterpret_cast<key_ctr_t*>(key_struct);
  // Populate the response to be sent in ServerSession
  physical_response_header rsh = {session_id,
      configuration_id,
      UNC_OP_CREATE,
      0,
      0,
      0,
      data_type,
      create_status};
  int err = PhyUtil::sessOutRespHeader(sess, rsh);
  err |= sess.addOutput((uint32_t)UNC_KT_CONTROLLER);
  err |= sess.addOutput(*obj_key_ctr);
  if (err != 0) {
    pfc_log_error("Server session addOutput failed, so return IPC_WRITE_ERROR");
    create_status = UPPL_RC_ERR_IPC_WRITE_ERROR;
  } else {
    create_status = UPPL_RC_SUCCESS;
  }
  return create_status;
}

/** CreateKeyInstance
 * * @Description : This function creates a new row of KT_Controller in
 * candidate controller table.
 * key_struct - the key for the new kt controller instance
 * value_struct - the values for the new kt controller instance
 * data_type - UNC_DT_* , Create only allowed in candidate
 * * @return    : UPPL_RC_SUCCESS is returned when the create is success
 * UPPL_RC_ERR_* is returned when ipc response could not be added to sess.
 * */
UpplReturnCode Kt_Controller::CreateKeyInstance(void* key_struct,
                                                void* val_struct,
                                                uint32_t data_type,
                                                uint32_t key_type) {
  UpplReturnCode create_status = UPPL_RC_SUCCESS;
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  // Structure used to send request to ODBC
  DBTableSchema kt_controller_dbtableschema;
  // Create DBSchema structure for controller_table
  void *old_val_struct;
  vector<ODBCMOperator> vect_key_operations;
  PopulateDBSchemaForKtTable(kt_controller_dbtableschema,
                             key_struct,
                             val_struct,
                             UNC_OP_CREATE, 0, 0,
                             vect_key_operations, old_val_struct);
  // Send request to ODBC for controller_table create
  ODBCM_RC_STATUS create_db_status = physical_layer->get_odbc_manager()->\
      CreateOneRow((unc_keytype_datatype_t)data_type,
                   kt_controller_dbtableschema);
  pfc_log_info("CreateOneRow error response from DB is %d",
               create_db_status);
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
    pfc_log_info("Create of a controller in data type(%d) is success",
                 data_type);
  }
  return create_status;
}

/** Update
 * * @Description : This function updates a row of KT_Controller in
 * candidate controller table.
 * * @param[in] : session_id - ipc session id used for TC validation
 * configuration_id - configuration id used for TC validation
 * key_struct - the key for the kt controller instance
 * value_struct - the values for the kt controller instance
 * data_type - UNC_DT_* , Update only allowed in candidate
 * sess - ipc server session where the response has to be added
 * * @return    : UPPL_RC_SUCCESS is returned when the response
 * is added to ipc session successfully.
 * UPPL_RC_ERR_* is returned when ipc response could not be added to sess.
 * */
UpplReturnCode Kt_Controller::Update(uint32_t session_id,
                                     uint32_t configuration_id,
                                     void* key_struct,
                                     void* val_struct,
                                     uint32_t data_type,
                                     ServerSession &sess) {
  UpplReturnCode update_status = UPPL_RC_SUCCESS;
  if ((unc_keytype_datatype_t)data_type != UNC_DT_CANDIDATE) {
    pfc_log_error("Update operation is invoked on unsupported data type %d",
                  data_type);
    update_status = UPPL_RC_ERR_OPERATION_NOT_ALLOWED;
  } else {
    update_status = UpdateKeyInstance(key_struct,
                                      val_struct,
                                      data_type,
                                      UNC_KT_CONTROLLER);
    pfc_log_debug("UpdateKeyInstance returned with status %d", update_status);
  }
  if (update_status == UPPL_RC_SUCCESS) {
    UpplReturnCode send_status = UPPL_RC_SUCCESS;
    // Sending the Created Controller  Information to Logical Layer
    send_status = SendUpdatedControllerInfoToUPLL(
        UNC_DT_CANDIDATE,
        UNC_OP_UPDATE,
        UNC_KT_CONTROLLER,
        key_struct,
        val_struct);
    pfc_log_info("Sending the Controller info to UPLL, status is %d",
                 send_status);
  }
  key_ctr_t *obj_key_ctr= reinterpret_cast<key_ctr_t*>(key_struct);
  // Populate the response to be sent in ServerSession
  physical_response_header rsh = {session_id,
      configuration_id,
      UNC_OP_UPDATE,
      0,
      0,
      0,
      data_type,
      update_status};
  int err = PhyUtil::sessOutRespHeader(sess, rsh);
  err |= sess.addOutput((uint32_t)UNC_KT_CONTROLLER);
  err |= sess.addOutput(*obj_key_ctr);
  if (err != 0) {
    pfc_log_error("Server session addOutput failed, so return IPC_WRITE_ERROR");
    update_status = UPPL_RC_ERR_IPC_WRITE_ERROR;
  } else {
    update_status = UPPL_RC_SUCCESS;
  }
  return update_status;
}

/** UpdateKeyInstance
 * * @Description : This function updates a row of KT_Controller in
 * candidate controller table.
 * * @param[in] :
 * key_struct - the key for the new kt controller instance
 * value_struct - the values for the new kt controller instance
 * data_type - UNC_DT_* , update only allowed in candidate
 * * @return    : UPPL_RC_SUCCESS is returned when the update
 * is done successfully.
 * UPPL_RC_ERR_* is returned when the update is error
 * */
UpplReturnCode Kt_Controller::UpdateKeyInstance(void* key_struct,
                                                void* val_struct,
                                                uint32_t data_type,
                                                uint32_t key_type) {
  UpplReturnCode update_status = UPPL_RC_SUCCESS;
  // Structure used to send request to ODBC
  DBTableSchema kt_controller_dbtableschema;
  // Create DBSchema structure for controller_table
  void *old_val_struct;
  vector<ODBCMOperator> vect_key_operations;
  PopulateDBSchemaForKtTable(kt_controller_dbtableschema,
                             key_struct,
                             val_struct,
                             UNC_OP_UPDATE, 0, 0,
                             vect_key_operations, old_val_struct);
  if (!((kt_controller_dbtableschema.get_row_list()).empty())) {
    // Send request to ODBC for controller_table update
    PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
    ODBCM_RC_STATUS update_db_status = physical_layer->get_odbc_manager()-> \
        UpdateOneRow((unc_keytype_datatype_t)data_type,
                     kt_controller_dbtableschema);
    pfc_log_info("UpdateOneRow response from DB is %d", update_db_status);
    if (update_db_status != ODBCM_RC_SUCCESS) {
      if (update_db_status == ODBCM_RC_CONNECTION_ERROR) {
        // log fatal error to log daemon
        pfc_log_fatal("DB connection not available or cannot access DB");
        update_status = UPPL_RC_ERR_DB_ACCESS;
      } else {
        // log error to log daemon
        update_status = UPPL_RC_ERR_DB_UPDATE;
      }
    } else {
      pfc_log_info("Update of a controller in data_type(%d) is success",
                   data_type);
    }
  } else {
    pfc_log_debug("Nothing to be updated, so return");
  }
  return update_status;
}

/**Delete
 * * @Description : This function deletes a row of KT_Controller in
 * candidate controller table.
 * * @param[in] : session_id - ipc session id used for TC validation
 * configuration_id - configuration id used for TC validation
 * key_struct - the key for the kt controller instance
 * data_type - UNC_DT_* , delete only allowed in candidate
 * sess - ipc server session where the response has to be added
 * * @return    : UPPL_RC_SUCCESS is returned when the response
 * is added to ipc session successfully.
 * UPPL_RC_ERR_* is returned when ipc response could not be added to sess.
 * */
UpplReturnCode Kt_Controller::Delete(uint32_t session_id,
                                     uint32_t configuration_id,
                                     void* key_struct,
                                     uint32_t data_type,
                                     ServerSession &sess) {
  UpplReturnCode delete_status = UPPL_RC_SUCCESS;
  key_ctr_t *obj_key_ctr = reinterpret_cast<key_ctr_t*>(key_struct);
  string controller_name = (const char*)obj_key_ctr->controller_name;
  Kt_Boundary boundary_class;
  // Check whether any boundary is referring controller
  pfc_bool_t is_bdry_referred = PFC_FALSE;
  is_bdry_referred = boundary_class.IsBoundaryReferred(
      UNC_KT_CONTROLLER, key_struct, data_type);
  if (is_bdry_referred == PFC_TRUE) {
    // Boundary is referring controller
    pfc_log_error(
        "Controller is referred in Boundary, "
        "so delete is not allowed");
    delete_status = UPPL_RC_ERR_CFG_SEMANTIC;
    // Populate the response to be sent in ServerSession
    physical_response_header rsh = {session_id,
        configuration_id,
        UNC_OP_DELETE,
        0,
        0,
        0,
        data_type,
        delete_status};
    int err = PhyUtil::sessOutRespHeader(sess, rsh);
    err |= sess.addOutput((uint32_t)UNC_KT_CONTROLLER);
    err |= sess.addOutput(*obj_key_ctr);
    if (err != 0) {
      pfc_log_debug(
          "Server session addOutput failed, so return IPC_WRITE_ERROR");
      delete_status = UPPL_RC_ERR_IPC_WRITE_ERROR;
    } else {
      delete_status = UPPL_RC_SUCCESS;
    }
    return delete_status;
  }
  // Check whether CONTROLLER is being referred in Logical layer
  delete_status = SendSemanticRequestToUPLL(key_struct,
                                            data_type);
  if (delete_status != UPPL_RC_SUCCESS) {
    // log error and send error response
    pfc_log_error(
        "Controller is referred in Logical, "
        "so delete is not allowed");
  }
  if (delete_status == UPPL_RC_SUCCESS) {
    // Delete child classes and then delete controller
    // In candidate db, only domain will be available
    int child_class = KIdxDomain;
    // Filling key_struct corresponding to that key type
    void *child_key_struct = getChildKeyStruct(child_class,
                                               controller_name);
    child[child_class] = GetChildClassPointer(
        (KtControllerChildClass)child_class);
    if (child[child_class] != NULL) {
      UpplReturnCode ch_delete_status =
          child[child_class]->DeleteKeyInstance(
              child_key_struct,
              data_type,
              UNC_KT_CTR_DOMAIN);
      delete child[child_class];
      child[child_class] = NULL;
      FreeChildKeyStruct(child_key_struct, child_class);
      if (ch_delete_status == UPPL_RC_ERR_NO_SUCH_INSTANCE) {
        pfc_log_debug("Child not available for controller");
      }
      if (ch_delete_status != UPPL_RC_ERR_NO_SUCH_INSTANCE &&
          ch_delete_status != UPPL_RC_SUCCESS) {
        // child delete failed, so return error
        pfc_log_error("Delete failed for child %d with error %d",
                      child_class, delete_status);
        delete_status = UPPL_RC_ERR_CFG_SEMANTIC;
      }
    } else {
      // Free key struct
      FreeChildKeyStruct(child_key_struct, child_class);
    }
    // Delete the controller now
    if (delete_status == UPPL_RC_SUCCESS) {
      delete_status = DeleteKeyInstance(key_struct, data_type,
                                        UNC_KT_CONTROLLER);
    }
  }
  if (delete_status == UPPL_RC_SUCCESS) {
    UpplReturnCode send_status = UPPL_RC_SUCCESS;
    // Sending the Created Controller  Information to Logical Layer
    send_status = SendUpdatedControllerInfoToUPLL(
        UNC_DT_CANDIDATE,
        UNC_OP_DELETE,
        UNC_KT_CONTROLLER,
        key_struct,
        0);
    pfc_log_info("Sending the Controller info to UPLL, status is %d",
                 send_status);
  }
  // Populate the response to be sent in ServerSession
  physical_response_header rsh = {session_id,
      configuration_id,
      UNC_OP_DELETE,
      0,
      0,
      0,
      data_type,
      delete_status};
  int err = PhyUtil::sessOutRespHeader(sess, rsh);
  err |= sess.addOutput((uint32_t)UNC_KT_CONTROLLER);
  err |= sess.addOutput(*obj_key_ctr);
  if (err != 0) {
    pfc_log_debug("Server session addOutput failed, so return IPC_WRITE_ERROR");
    delete_status = UPPL_RC_ERR_IPC_WRITE_ERROR;
  } else {
    delete_status = UPPL_RC_SUCCESS;
  }
  return delete_status;
}

/**DeleteKeyInstance
 * * @Description : This function deletes a row of KT_Controller in
 * candidate controller table.
 * * @param[in] :
 * key_struct - the key for the new kt controller instance
 * data_type - UNC_DT_* , delete only allowed in candidate
 * * @return    : UPPL_RC_SUCCESS is returned when the delete
 * is done successfully.
 * UPPL_RC_ERR_* is returned when the delete is error
 * */
UpplReturnCode Kt_Controller::DeleteKeyInstance(void* key_struct,
                                                uint32_t data_type,
                                                uint32_t key_type) {
  UpplReturnCode delete_status = UPPL_RC_SUCCESS;
  key_ctr_t *obj_key_ctr= reinterpret_cast<key_ctr_t*>(key_struct);
  string controller_name = (const char*)obj_key_ctr->controller_name;
  // Structure used to send request to ODBC
  DBTableSchema kt_controller_dbtableschema;

  // Construct Primary key list
  vector<string> vect_prim_keys;
  vect_prim_keys.push_back(CTR_NAME);

  // Construct TableAttrSchema structure
  // TableAttrSchema holds table_name, primary key, attr_name
  vector<TableAttrSchema> vect_table_attr_schema;
  list< vector<TableAttrSchema> > row_list;

  // Controller_name
  PhyUtil::FillDbSchema(CTR_NAME, controller_name,
                        controller_name.length(), DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);

  // Send request to ODBC for controller_table delete
  kt_controller_dbtableschema.set_table_name(UPPL_CTR_TABLE);
  kt_controller_dbtableschema.set_primary_keys(vect_prim_keys);
  row_list.push_back(vect_table_attr_schema);
  kt_controller_dbtableschema.set_row_list(row_list);

  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  ODBCM_RC_STATUS delete_db_status = physical_layer->get_odbc_manager()-> \
      DeleteOneRow((unc_keytype_datatype_t)data_type,
                   kt_controller_dbtableschema);
  pfc_log_info("DeleteOneRow error response from DB is %d",
               delete_db_status);
  if (delete_db_status != ODBCM_RC_SUCCESS) {
    if (delete_db_status == ODBCM_RC_CONNECTION_ERROR) {
      // log fatal error to log daemon
      pfc_log_fatal("DB connection not available or cannot access DB");
      delete_status = UPPL_RC_ERR_DB_ACCESS;
    } else if (delete_db_status == ODBCM_RC_ROW_NOT_EXISTS) {
      delete_status = UPPL_RC_ERR_NO_SUCH_INSTANCE;
    } else {
      // log error to log daemon
      delete_status = UPPL_RC_ERR_DB_DELETE;
    }
  } else {
    pfc_log_info("Delete of a controller in data_type(%d) is success",
                 data_type);
  }
  return delete_status;
}

/** ReadInternal
 * * @Description : This function reads the given  instance of KT_Controller
 ** * @param[in] : session_id - ipc session id used for TC validation
 * key_struct - the key for the kt controller instance
 * value_struct - the value for the kt controller instance
 * data_type - UNC_DT_* , read allowed in candidate/running/startup/state
 * * @return    : UPPL_RC_SUCCESS is returned when the response
 * is added to ipc session successfully.
 * UPPL_RC_ERR_* is returned when ipc response could not be added to sess.
 * */
UpplReturnCode Kt_Controller::ReadInternal(vector<void *> &ctr_key,
                                           vector<void *> &ctr_val,
                                           uint32_t data_type,
                                           uint32_t operation_type) {
  pfc_log_debug("Processing Kt_Controller::ReadInternal");
  vector<string> vect_controller_id;
  vector<val_ctr_st_t> vect_val_ctr_st;
  uint32_t max_rep_ct = 1;
  if (operation_type != UNC_OP_READ) {
    max_rep_ct = UPPL_MAX_REP_CT;
  }
  void *key_struct = ctr_key[0];
  void *val_struct = NULL;
  if (!ctr_val.empty()) {
    val_ctr_st_t st_ctr_val = *(reinterpret_cast<val_ctr_st_t *> (ctr_val[0]));
    val_struct = reinterpret_cast<void *>(&st_ctr_val.controller);
  }
  // Get read response from database
  UpplReturnCode read_status = ReadCtrValFromDB(key_struct,
                                                val_struct,
                                                data_type,
                                                operation_type,
                                                max_rep_ct,
                                                vect_val_ctr_st,
                                                vect_controller_id);
  ctr_key.clear();
  ctr_val.clear();
  pfc_log_info("ReadCtrValFromDB returned %d with response size %d",
               read_status,
               static_cast<int>(vect_val_ctr_st.size()));
  if (read_status == UPPL_RC_SUCCESS) {
    pfc_log_debug("ReadCtrValFromDB returned %d with response size %d",
                  read_status, static_cast<int>(vect_val_ctr_st.size()));
    for (unsigned int iIndex = 0 ; iIndex < vect_controller_id.size();
        ++iIndex) {
      key_ctr_t *key_ctr = new key_ctr_t;
      memcpy(key_ctr->controller_name,
             vect_controller_id[iIndex].c_str(),
             vect_controller_id[iIndex].length()+1);
      ctr_key.push_back(reinterpret_cast<void *>(key_ctr));
      val_ctr_st_t *val_ctr = new val_ctr_st_t(vect_val_ctr_st[iIndex]);
      ctr_val.push_back(reinterpret_cast<void *>(val_ctr));
    }
  }
  return read_status;
}

/**ReadBulk
 * * @Description : This function reads bulk rows of KT_Controller in
 *  controller table of specified data type.
 *  Order of ReadBulk response
 *  val_ctr -> val_ctr_domain -> val_logical_port ->
 *  val_logical_member_port -> val_switch ->  val_port ->
 *  val_link -> val_boundary
 * * @param[in] :
 * key_struct - the key for the kt controller instance
 * data_type - UNC_DT_* , read allowed in candidate/running/startup/state
 * option1/option2 - specifies any additional condition for read operation
 * max_rep_ct - specifies number of rows to be returned
 * parent_call - indicates whether parent has called this readbulk
 * is_read_next - indicates whether this function is invoked from readnext
 * * @return    : UPPL_RC_SUCCESS is returned when the response
 * is added to ipc session successfully.
 * UPPL_RC_ERR_* is returned when ipc response could not be added to sess.
 * */
UpplReturnCode Kt_Controller::ReadBulk(void* key_struct,
                                       uint32_t data_type,
                                       uint32_t option1,
                                       uint32_t option2,
                                       uint32_t &max_rep_ct,
                                       int child_index,
                                       pfc_bool_t parent_call,
                                       pfc_bool_t is_read_next) {
  pfc_log_info("Processing ReadBulk of Kt_Controller");
  UpplReturnCode read_status = UPPL_RC_SUCCESS;
  key_ctr_t* obj_key_ctr= reinterpret_cast<key_ctr_t*>(key_struct);

  vector<val_ctr_st_t> vect_val_ctr;
  if (data_type != UNC_DT_CANDIDATE && data_type != UNC_DT_RUNNING &&
      data_type != UNC_DT_STATE && data_type != UNC_DT_STARTUP) {
    pfc_log_debug("ReadBulk operation is not allowed in %d data type",
                  data_type);
    read_status = UPPL_RC_ERR_OPERATION_NOT_ALLOWED;
    return read_status;
  }
  pfc_bool_t ctr_exists = PFC_FALSE;
  if (max_rep_ct == 0) {
    pfc_log_debug("max_rep_ct is 0");
    return UPPL_RC_SUCCESS;
  }
  string str_controller_name =
      reinterpret_cast<char *>(&obj_key_ctr->controller_name);
  vector<string> vect_ctr_id;
  // Check for child call
  if (child_index == -1 &&
      !str_controller_name.empty()) {
    // Check for controller key existence
    vector<string> vect_ctr_key_value;
    vect_ctr_key_value.push_back(str_controller_name);
    UpplReturnCode key_exist_status = IsKeyExists(
        (unc_keytype_datatype_t)data_type,
        vect_ctr_key_value);
    if (key_exist_status == UPPL_RC_SUCCESS) {
      ctr_exists = PFC_TRUE;
    }
  }
  if (child_index > -1 && child_index < KIdxLink) {
    ctr_exists = PFC_TRUE;
  }
  void *val_struct = NULL;
  // Read the controller values based on given key structure
  read_status = ReadBulkInternal(
      key_struct,
      val_struct,
      data_type,
      max_rep_ct+1,
      vect_val_ctr,
      vect_ctr_id);
  if (ctr_exists == PFC_TRUE) {
    vect_ctr_id.insert(vect_ctr_id.begin(), str_controller_name);
    val_ctr_st_t dummy_val_ctr;
    vect_val_ctr.insert(vect_val_ctr.begin(), dummy_val_ctr);
  }
  if (read_status == UPPL_RC_SUCCESS ||
      ctr_exists == PFC_TRUE) {
    PhysicalCore *physical_core = PhysicalLayer::get_instance()->
        get_physical_core();
    InternalTransactionCoordinator *itc_trans  =
        physical_core->get_internal_transaction_coordinator();
    // For each controller, read the child's attributes
    vector<val_ctr_st_t> ::iterator vect_iter = vect_val_ctr.begin();
    vector<string> ::iterator controller_iter = vect_ctr_id.begin();
    for (; controller_iter != vect_ctr_id.end(); ++controller_iter,
    ++vect_iter) {
      pfc_log_debug("Iterating entries from controller list");
      string fKey = *controller_iter;
      pfc_log_debug("fKey %s str_controller_name %s", fKey.c_str(),
                    str_controller_name.c_str());
      if (ctr_exists == PFC_FALSE &&
          (child_index == -1 ||
              (child_index == KIdxLink && fKey != str_controller_name))) {
        val_ctr_st_t obj_ctr_st = (*vect_iter);
        if (data_type == UNC_DT_CANDIDATE &&
            obj_ctr_st.controller.cs_row_status == DELETED) {
          pfc_log_debug("Ignoring DELETED entry %s", fKey.c_str());
          continue;
        }
        key_ctr_t *ctr_key = new key_ctr_t;
        memset(ctr_key, '\0', sizeof(key_ctr_t));
        pfc_log_debug("Adding controller - '%s' to session", fKey.c_str());
        memcpy(ctr_key->controller_name,
               fKey.c_str(),
               fKey.length()+1);
        BulkReadBuffer obj_key_buffer = {
            UNC_KT_CONTROLLER, IS_KEY,
            reinterpret_cast<void *>(ctr_key)
        };
        itc_trans->AddToBuffer(obj_key_buffer);
        if ((unc_keytype_datatype_t)data_type == UNC_DT_STATE) {
          val_ctr_st_t *val_buffer = new val_ctr_st_t(*vect_iter);
          BulkReadBuffer obj_value_buffer = {
              UNC_KT_CONTROLLER, IS_STATE_VALUE,
              reinterpret_cast<void *>(val_buffer)
          };
          itc_trans->AddToBuffer(obj_value_buffer);
        } else {
          val_ctr_t *val_buffer = new val_ctr_t((*vect_iter).controller);
          BulkReadBuffer obj_value_buffer = {
              UNC_KT_CONTROLLER, IS_VALUE,
              reinterpret_cast<void *>(val_buffer)
          };
          itc_trans->AddToBuffer(obj_value_buffer);
        }
        BulkReadBuffer obj_sep_buffer = {
            UNC_KT_CONTROLLER, IS_SEPARATOR, NULL
        };
        itc_trans->AddToBuffer(obj_sep_buffer);
        --max_rep_ct;
        if (max_rep_ct == 0) {
          pfc_log_debug("Controller - max_rep_ct reached zero...");
          return UPPL_RC_SUCCESS;
        }
      }
      ctr_exists = PFC_FALSE;
      str_controller_name = fKey;
      int st_child_index = (child_index >= 0 && child_index <= KIdxLink) \
          ? child_index+1 : KIdxDomain;
      for (unsigned int kIdx = st_child_index; kIdx <= KIdxLink; ++kIdx) {
        pfc_log_debug("Controller %s - Iterating child %d",
                      fKey.c_str(), kIdx);
        if ((kIdx == KIdxSwitch || kIdx == KIdxLink) &&
            (unc_keytype_datatype_t)data_type != UNC_DT_STATE) {
          pfc_log_debug("Ignoring Switch/Link for non-dt_state");
          continue;
        }
        void *child_key_struct = getChildKeyStruct(kIdx,
                                                   fKey);
        child[kIdx] = GetChildClassPointer
            ((KtControllerChildClass)kIdx);
        if (child[kIdx] == NULL) {
          // Free Key struct
          FreeChildKeyStruct(child_key_struct, kIdx);
          continue;
        }
        pfc_log_debug("Controller Calling child %d read bulk", kIdx);
        UpplReturnCode ch_read_status = child[kIdx]->ReadBulk(
            child_key_struct,
            data_type,
            option1,
            option2,
            max_rep_ct,
            -1,
            true,
            is_read_next);
        pfc_log_debug("ReadBulk response from child %d is %d",
                      kIdx, ch_read_status);
        delete child[kIdx];
        child[kIdx] = NULL;
        FreeChildKeyStruct(child_key_struct, kIdx);
        if (max_rep_ct == 0) {
          pfc_log_debug("max_rep_ct reached zero, so returning");
          return UPPL_RC_SUCCESS;
        }
      }
      // reset child index
      child_index = -1;
    }
  }
  if (max_rep_ct > 0 && parent_call == false) {
    pfc_log_debug("max_rep_ct is %d and parent_call is %d, calling parent",
                  max_rep_ct, parent_call);
    // Filling key_struct corresponding to tht key type
    Kt_Root nextKin;
    key_root_t nextkin_key_struct;
    read_status = nextKin.ReadBulk(
        reinterpret_cast<void *>(&nextkin_key_struct),
        data_type,
        option1,
        option2,
        max_rep_ct,
        0,
        false,
        is_read_next);
    pfc_log_debug("read status from next kin Kt_Root is %d",
                  read_status);
    return UPPL_RC_SUCCESS;
  }
  pfc_log_debug("KT_Controller - Reached end of table");
  pfc_log_debug("read_status is %d", read_status);
  if (read_status == UPPL_RC_ERR_NO_SUCH_INSTANCE) {
    read_status = UPPL_RC_SUCCESS;
  }
  return UPPL_RC_SUCCESS;
}

/**ReadBulkInternal
 * * @Description : This function reads bulk rows of KT_Controller in
 *  controller table of specified data type.
 * * @param[in] :
 * key_struct - the key for the kt controller instance
 * val_struct - the value struct for kt_controller instance
 * max_rep_ct - specifies number of rows to be returned
 * vect_val_ctr - indicates the fetched values from db of val_ctr type
 * vect_ctr_id - indicates the fetched contoller names from db
 * * @return    : UPPL_RC_SUCCESS is returned when the response
 * is added to ipc session successfully.
 * UPPL_RC_ERR_* is returned when ipc response could not be added to sess.
 * */
UpplReturnCode Kt_Controller::ReadBulkInternal(
    void* key_struct,
    void* val_struct,
    uint32_t data_type,
    uint32_t max_rep_ct,
    vector<val_ctr_st_t> &vect_val_ctr,
    vector<string> &vect_ctr_id) {
  if (max_rep_ct <= 0) {
    return UPPL_RC_SUCCESS;
  }
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  UpplReturnCode read_status = UPPL_RC_SUCCESS;
  ODBCM_RC_STATUS read_db_status = ODBCM_RC_SUCCESS;
  DBTableSchema kt_controller_dbtableschema;
  // Populate DBSchema for controller_table
  void *old_val_struct;
  vector<ODBCMOperator> vect_key_operations;
  PopulateDBSchemaForKtTable(kt_controller_dbtableschema,
                             key_struct,
                             val_struct,
                             UNC_OP_READ_BULK, 0, 0,
                             vect_key_operations, old_val_struct);
  // Read rows from DB
  read_db_status = physical_layer->get_odbc_manager()-> \
      GetBulkRows((unc_keytype_datatype_t)data_type, max_rep_ct,
                  kt_controller_dbtableschema,
                  (unc_keytype_operation_t)UNC_OP_READ_BULK);
  if (read_db_status == ODBCM_RC_RECORD_NOT_FOUND) {
    pfc_log_debug("No record to read");
    read_status = UPPL_RC_ERR_NO_SUCH_INSTANCE;
    return read_status;
  } else if (read_db_status != ODBCM_RC_SUCCESS) {
    read_status = UPPL_RC_ERR_DB_GET;
    pfc_log_error("ReadBulk operation has failed with %d", read_db_status);
    return read_status;
  }

  // From the values received from DB, populate val_ctr structure
  FillControllerValueStructure(kt_controller_dbtableschema,
                               vect_val_ctr,
                               max_rep_ct,
                               UNC_OP_READ_BULK,
                               vect_ctr_id);
  return read_status;
}

/** PerformSyntaxValidation
 * * @Description : This function performs syntax validation for
 *  UNC_KT_CONTROLLER
 * * * @param[in]
 * key_struct - the key for the kt controller instance
 * value_struct - the value for the kt controller instance
 * data_type - UNC_DT_*
 * operation_type - UNC_OP*
 * * @return    : UPPL_RC_SUCCESS is returned when the validation is successful
 * UPPL_RC_ERR_* is returned when validtion is failure
 * */
UpplReturnCode Kt_Controller::PerformSyntaxValidation(void* key_struct,
                                                      void* val_struct,
                                                      uint32_t operation,
                                                      uint32_t data_type) {
  UpplReturnCode ret_code = UPPL_RC_SUCCESS;
  pfc_bool_t mandatory = PFC_TRUE;

  // Validate key structure
  key_ctr *key = reinterpret_cast<key_ctr_t*>(key_struct);
  string value = reinterpret_cast<char*>(key->controller_name);
  IS_VALID_STRING_KEY(CTR_NAME, value, operation, ret_code, mandatory);
  if (ret_code != UPPL_RC_SUCCESS) {
    return UPPL_RC_ERR_CFG_SYNTAX;
  }

  unc_keytype_ctrtype_t ctr_type = UNC_CT_UNKNOWN;
  UpplReturnCode ctr_type_code = UPPL_RC_SUCCESS;
  if (operation == UNC_OP_UPDATE) {
    ctr_type_code = PhyUtil::get_controller_type(
        value,
        ctr_type,
        (unc_keytype_datatype_t)data_type);
    pfc_log_debug("Controller type - return code %d, value %s",
                  ctr_type_code, value.c_str());
  }

  // Validate value structure
  if (val_struct != NULL) {
    val_ctr *val_ctr = reinterpret_cast<val_ctr_t*>(val_struct);
    ret_code = ValidateControllerType(
        operation,
        data_type,
        ctr_type,
        ctr_type_code,
        val_ctr);
    if (ret_code != UPPL_RC_SUCCESS) {
      return UPPL_RC_ERR_CFG_SYNTAX;
    }
    ret_code = ValidateControllerVersion(
        operation,
        data_type,
        ctr_type,
        ctr_type_code,
        val_ctr);
    if (ret_code != UPPL_RC_SUCCESS) {
      return UPPL_RC_ERR_CFG_SYNTAX;
    }
    ret_code = ValidateControllerDescription(
        operation,
        data_type,
        ctr_type,
        ctr_type_code,
        val_ctr);
    if (ret_code != UPPL_RC_SUCCESS) {
      return UPPL_RC_ERR_CFG_SYNTAX;
    }
    ret_code = ValidateControllerIpAddress(
        operation,
        data_type,
        ctr_type,
        ctr_type_code,
        key_struct,
        val_struct);
    if (ret_code != UPPL_RC_SUCCESS) {
      return UPPL_RC_ERR_CFG_SYNTAX;
    }
    ret_code = ValidateControllerUser(
        operation,
        data_type,
        ctr_type,
        ctr_type_code,
        val_ctr);
    if (ret_code != UPPL_RC_SUCCESS) {
      return UPPL_RC_ERR_CFG_SYNTAX;
    }
    ret_code = ValidateControllerPassword(
        operation,
        data_type,
        ctr_type,
        ctr_type_code,
        val_ctr);
    if (ret_code != UPPL_RC_SUCCESS) {
      return UPPL_RC_ERR_CFG_SYNTAX;
    }
    ret_code = ValidateControllerEnableAudit(
        operation,
        data_type,
        ctr_type,
        ctr_type_code,
        val_ctr);
    if (ret_code != UPPL_RC_SUCCESS) {
      return UPPL_RC_ERR_CFG_SYNTAX;
    }
  }
  return ret_code;
}

/** ValidateCtrlrValueCapability
 * * @Description : This function checks capability for UNC_KT_CONTROLLER
 * * * @param[in] : version - specifies the version to be used for validation
 * key type - specifies UNC_KT_CONTROLLER
 * * * @return    : UPPL_RC_SUCCESS if validation is successful
 * or UPPL_RC_ERR_* if validation is failed
 * */
UpplReturnCode Kt_Controller::ValidateCtrlrValueCapability(string version,
                                                           uint32_t key_type) {
  UpplReturnCode resp_code = UPPL_RC_SUCCESS;
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  map<string, Kt_Class_Attr_Syntax>::iterator itVal;
  for (itVal = attr_syntax_map.begin();
      itVal != attr_syntax_map.end();
      ++itVal) {
    string attribute_name = itVal->first;
    if (attribute_name != CTR_NAME &&
        attribute_name != CTR_VALID &&
        attribute_name != CTR_CS_ROW_STATUS &&
        attribute_name != CTR_CS_ATTR) {
      resp_code = physical_layer->get_physical_core()->
          ValidateAttribInCtrlrCap(version,
                                   key_type,
                                   attribute_name);
    }
    if (resp_code != UPPL_RC_SUCCESS) {
      pfc_log_info("Error from ValidateAttribInCtrlrCap is %d", resp_code);
      return resp_code;
    }
  }
  return resp_code;
}

/** ValidateCtrlrScalability
 * * @Description : This function checks scalability for UNC_KT_CONTROLLER
 * * * @param[in] : version - specifies the controller version
 * key type - contains UNC_KT_CONTROLLER
 * data_type - UNC_DT_*, scalability number and data_type
 * * * @return    : UPPL_RC_SUCCESS if scalability number is within range
 * or UPPL_RC_ERR_* if not
 * */
UpplReturnCode Kt_Controller::ValidateCtrlrScalability(string version,
                                                       uint32_t key_type,
                                                       uint32_t data_type) {
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  // Structure used to send request to ODBC
  // Send request to ODBC for controller_table create
  ODBCM_RC_STATUS response_status = ODBCM_RC_SUCCESS;
  uint32_t response_count = 0;
  string table_name = UPPL_CTR_TABLE;
  response_status = physical_layer->get_odbc_manager()->\
      GetRowCount((unc_keytype_datatype_t)data_type,
                  table_name, response_count);
  if (response_status == ODBCM_RC_CONNECTION_ERROR) {
    // log error to log daemon
    pfc_log_error("DB connection not available or cannot access DB");
    return UPPL_RC_ERR_DB_ACCESS;
  } else if (response_status == ODBCM_RC_SUCCESS) {
    uint32_t scalability_num = 0;
    UpplReturnCode resp_code = physical_layer->get_physical_core()->
        GetScalabilityNumber(version,
                             key_type,
                             scalability_num);
    if (resp_code != UPPL_RC_SUCCESS) {
      pfc_log_error("Unable to get scalability number from system");
      return UPPL_RC_ERR_DB_GET;
    }
    if ((uint32_t)response_count >= scalability_num) {
      pfc_log_debug("Count from DB: %d", response_count);
      pfc_log_debug("Count from Physical Core: %d", scalability_num);
      pfc_log_error("scalability range exceeded for kt_controller");
      return UPPL_RC_ERR_EXCEEDS_RESOURCE_LIMIT;
    }
  } else {
    pfc_log_error("Unable to get scalability number from DB, error is %d",
                  response_status);
    return UPPL_RC_ERR_DB_GET;
  }
  return UPPL_RC_SUCCESS;
}

/** ValidateUnKnownCtrlrScalability
 * * @Description : This function checks scalability for unknown controller
 * * * @param[in] : type - specifies the controller type
 * key type - contains UNC_KT_CONTROLLER
 * data_type - UNC_DT_*, unknown controler scalability number and data_type
 * * * @return    : UPPL_RC_SUCCESS if unknown controler scalability number
 *  is within range or UPPL_RC_ERR_* if not
 * */
UpplReturnCode Kt_Controller::ValidateUnknownCtrlrScalability(
    void *key_struct,
    uint8_t type,
    uint32_t data_type) {
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  ODBCM_RC_STATUS response_status = ODBCM_RC_SUCCESS;
  uint32_t count = 0;
  DBTableSchema kt_controller_dbtableschema;
  // Construct Primary key list
  vector<string> vect_prim_keys;
  vect_prim_keys.push_back(CTR_TYPE);
  std::vector<ODBCMOperator> vect_filter_operators;
  vect_filter_operators.push_back(unc::uppl::EQUAL);

  string value = PhyUtil::uint8tostr(type);
  pfc_log_debug("type: %s", value.c_str());

  // Construct TableAttrSchema structure
  // TableAttrSchema holds table_name, primary key, attr_name
  vector<TableAttrSchema> vect_table_attr_schema;
  list< vector<TableAttrSchema> > row_list;
  PhyUtil::FillDbSchema(CTR_TYPE, value,
                        value.length(), DATATYPE_UINT16,
                        vect_table_attr_schema);
  kt_controller_dbtableschema.set_table_name(UPPL_CTR_TABLE);
  kt_controller_dbtableschema.set_primary_keys(vect_prim_keys);
  row_list.push_back(vect_table_attr_schema);
  kt_controller_dbtableschema.set_row_list(row_list);

  // Send request to ODBC for controller_table
  response_status = physical_layer->get_odbc_manager()-> \
      GetSiblingCount((unc_keytype_datatype_t)data_type,
                      kt_controller_dbtableschema, count,
                      vect_filter_operators);

  if (response_status == ODBCM_RC_CONNECTION_ERROR) {
    // log error to log daemon
    pfc_log_error("DB connection not available or cannot access DB");
    return UPPL_RC_ERR_DB_ACCESS;
  } else if (response_status == ODBCM_RC_SUCCESS ||
      response_status == ODBCM_RC_RECORD_NOT_FOUND) {
    uint32_t unknown_ctrl_count =
        physical_layer->get_physical_core()->getUnknownControllerCount();

    if ((uint32_t)count >= unknown_ctrl_count) {
      pfc_log_debug("Count from DB: %d", count);
      pfc_log_debug("Count from Physical Core: %d", unknown_ctrl_count);
      pfc_log_error("scalability range exceeded for unknown kt_controller");
      return UPPL_RC_ERR_EXCEEDS_RESOURCE_LIMIT;
    }
  } else {
    pfc_log_error("Unable to get sibling count from DB, error is %d",
                  response_status);
    return UPPL_RC_ERR_DB_GET;
  }
  return UPPL_RC_SUCCESS;
}

/** PerformSemanticValidation
 * * @Description : This function performs semantic validation
 * for UNC_KT_CONTROLLER
 * * * @param[in] : key_struct - specifies key instance of KT_Controller
 * , value_struct - specifies value of KT_CONTROLLER
 * operation - UNC_OP*
 * data_type - UNC_DT*
 * * * @return    : UPPL_RC_SUCCESS if semantic valition is successful
 * or UPPL_RC_ERR_* if failed
 * */
UpplReturnCode Kt_Controller::PerformSemanticValidation(void* key_struct,
                                                        void* val_struct,
                                                        uint32_t operation,
                                                        uint32_t data_type) {
  UpplReturnCode status = UPPL_RC_SUCCESS;

  // Check whether the given instance of controller exists in DB
  key_ctr_t *obj_key_ctr = reinterpret_cast<key_ctr_t*>(key_struct);
  string controller_name = (const char*)obj_key_ctr->controller_name;
  vector<string> ctr_vect_key_value;
  ctr_vect_key_value.push_back(controller_name);
  UpplReturnCode key_status = IsKeyExists((unc_keytype_datatype_t)data_type,
                                          ctr_vect_key_value);
  // In case of create operation, key should not exist
  if (operation == UNC_OP_CREATE) {
    if (key_status == UPPL_RC_SUCCESS) {
      pfc_log_error("Key instance already exists, ");
      pfc_log_error("Hence create operation not allowed");
      status = UPPL_RC_ERR_INSTANCE_EXISTS;
    } else if (key_status == UPPL_RC_ERR_DB_ACCESS) {
      pfc_log_error("DB Access failure");
      status = key_status;
    } else if (key_status == UPPL_RC_ERR_DB_GET) {
      pfc_log_debug("Key does not exist. Validate Ip Address/ Type");
      // Check whether any controller with same type and ip address exists
      status = ValidateTypeIpAddress(key_struct,
                                     val_struct,
                                     data_type);
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
    }
  }
  return status;
}

/** SendSemanticRequestToUPLL
 * * @Description : This functions calls IPC to check whether UNC_KT_CONTROLLER
 *  is being referred in Logical
 * * * @param[in] : key_ctr - specifies key instance of KT_CONTROLLER
 * * * @return    : UPPL_RC_SUCCESS if controller is not referred
 * or UPPL_RC_ERR_* if controller is referred in logical
 * */
UpplReturnCode Kt_Controller::SendSemanticRequestToUPLL(void* key_struct,
                                                        uint32_t data_type) {
  // Incase for UNC_KT_CONTROLLER delete, check whether any referenced object
  // Is present in Logical Layer, If yes DELETE should not be allowed
  pfc_log_debug("Inside SendSemanticRequestToUPLL of KTController");
  UpplReturnCode status = UPPL_RC_SUCCESS;
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  status = physical_layer->get_ipc_connection_manager()->
      get_ipc_client_logical_handler()->CheckInUseInLogical(UNC_KT_CONTROLLER,
                                                            key_struct,
                                                            data_type);
  if (status != UPPL_RC_SUCCESS) {
    // log error and send error response
    pfc_log_error("Controller is being referred in Logical");
  }
  return status;
}

/** HandleDriverEvents
 * * @Description : This function processes the notification received from
 * driver
 * * * @param[in] : key_struct - specifies the key instance of KT_CONTROLLER
 * old_value_struct - old value of KT_CONTROLLER
 * new_value_struct - new value of KT_CONTROLLER
 * * * @return    : UPPL_RC_SUCCESS if events are handled successfully or
 * UPPL_RC_ERR*
 * */
UpplReturnCode Kt_Controller::HandleDriverEvents(void* key_struct,
                                                 uint32_t oper_type,
                                                 uint32_t data_type,
                                                 void* old_val_struct,
                                                 void* new_val_struct,
                                                 pfc_bool_t is_events_done) {
  key_ctr *obj_key_ctr= reinterpret_cast<key_ctr_t*>(key_struct);
  val_ctr_st *obj_old_val_ctr = reinterpret_cast<val_ctr_st*>(old_val_struct);
  val_ctr_st *obj_new_val_ctr = reinterpret_cast<val_ctr_st*>(new_val_struct);
  UpplReturnCode status = UPPL_RC_SUCCESS;
  string controller_name = reinterpret_cast<char *>
  (obj_key_ctr->controller_name);
  if (oper_type != UNC_OP_UPDATE) {
    pfc_log_info("Operation other than update is not allowed");
    return status;
  }
  if (obj_old_val_ctr == NULL || obj_new_val_ctr == NULL) {
    pfc_log_info("Either new/old value is NULL");
    return status;
  }
  unsigned int valid_val = 0;
  valid_val = PhyUtil::uint8touint(obj_new_val_ctr->valid[kIdxActualVersion]);
  if (PhyUtil::IsValidValue(oper_type, valid_val) == true) {
    string act_version = (const char*)obj_new_val_ctr->actual_version;
    status = SetActualVersion(key_struct, act_version);
    if (status != UPPL_RC_SUCCESS) {
      // log error
      pfc_log_error("act_version update operation failed");
    }
  }
  // Read old_oper_status from DB
  uint8_t oper_status_db = 0;
  UpplReturnCode read_status = GetOperStatus(data_type,
                                             key_struct,
                                             oper_status_db);
  if (read_status == UPPL_RC_SUCCESS) {
    uint8_t new_oper_status = UPPL_CONTROLLER_OPER_DOWN;
    // CONTROLLER_OPER_UP from driver
    // Its same as enum UPPL_CONTROLLER_OPER_UP
    if (obj_new_val_ctr->oper_status == UPPL_CONTROLLER_OPER_UP) {
      if (is_events_done == false) {
        new_oper_status = UPPL_CONTROLLER_OPER_WAITING_AUDIT;
      } else {
        new_oper_status = UPPL_CONTROLLER_OPER_UP;
      }
    }
    if (obj_new_val_ctr->oper_status == UPPL_CONTROLLER_OPER_DOWN) {
      // If controller is in audit list perform required action
      IPCConnectionManager *ipc_mgr = PhysicalLayer::get_instance()->
          get_ipc_connection_manager();
      pfc_bool_t is_controller_in_audit = ipc_mgr->
          IsControllerInAudit(controller_name);
      if (is_controller_in_audit == PFC_TRUE) {
        pfc_log_debug("Calling MergeAuditDbToRunning");
        // To cancel the already running timer in Audit
        uint32_t time_out_id = ipc_mgr->getTimeOutId(controller_name);
        ipc_mgr->notfn_timer_->cancel(time_out_id);
        AuditRequest audit_req;
        UpplReturnCode merge_auditdb =
            audit_req.MergeAuditDbToRunning(controller_name);
        if (merge_auditdb != UPPL_RC_SUCCESS) {
          pfc_log_info("Merge of audit and running db failed");
        }
      }
    }
    pfc_log_info("New Oper_status to be set is: %d", new_oper_status);
    if (new_oper_status != oper_status_db) {
      status = HandleOperStatus(data_type, key_struct, obj_new_val_ctr);
      pfc_log_debug("HandleOperStatus return %d", status);
      // Send notification to Northbound
      if (new_oper_status == UPPL_CONTROLLER_OPER_UP ||
          new_oper_status == UPPL_CONTROLLER_OPER_DOWN) {
        status = SendOperStatusNotification(*obj_key_ctr,
                                            oper_status_db,
                                            new_oper_status);
      }
    }
  }
  return status;
}

/** HandleDriverAlarms
 * * @Description : This function processes the alarm notification
 * received from driver
 * * * @param[in] : alarm type - contains type to indicate PATH_FAULT alarm
 * operation - contains UNC_OP_CREATE or UNC_OP_DELETE\
 * key_struct - indicates the key instance of KT_CONTROLLER
 * value_struct - indicates the alarm values structure
 * * * @return    : UPPL_RC_SUCCESS if alarm is handled successfully or
 * UPPL_RC_ERR*
 * */
UpplReturnCode Kt_Controller::HandleDriverAlarms(uint32_t data_type,
                                                 uint32_t alarm_type,
                                                 uint32_t oper_type,
                                                 void* key_struct,
                                                 void* val_struct) {
  UpplReturnCode status = UPPL_RC_SUCCESS;
  // Following alarms are sent for kt_controller
  if (alarm_type == UNC_PHYS_PATH_FAULT) {
    pfc_log_info("PHYS_PATH_FAULT alarm received from driver");
    pfc_log_info("with oper type as %d", oper_type);
    PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
    key_ctr_t *obj_key_ctr= reinterpret_cast<key_ctr_t*>(key_struct);
    string controller_name = (const char*)obj_key_ctr->controller_name;
    int err = 0;
    Kt_LogicalPort logical_port_class;
    Kt_Port port_class;
    // Populate val_path_fault_alarm_t
    val_path_fault_alarm_t obj_path_fault_alarm;
    val_phys_path_fault_alarm_t obj_val = *(reinterpret_cast
        <val_phys_path_fault_alarm_t*>(val_struct));
    string ingress_switch_id = reinterpret_cast<char *>
    (obj_val.ingress_ofs_dpid);
    string egress_switch_id = reinterpret_cast<char *>
    (obj_val.egress_ofs_dpid);
    string phy_port_id;
    vector<string> ingress_logical_port_id, egress_logical_port_id;
    // Get the associated logical_port_id for ingress switch
    logical_port_class.GetAllPortId(data_type,
                                    controller_name,
                                    ingress_switch_id,
                                    ingress_logical_port_id,
                                    true);
    memset(obj_path_fault_alarm.ingress_logical_port, 0, 320);
    if (!ingress_logical_port_id.empty()) {
      memcpy(obj_path_fault_alarm.ingress_logical_port,
             ingress_logical_port_id[0].c_str(),
             (ingress_logical_port_id[0].length())+1);
      pfc_log_debug("Ingress Logical Port: %s",
                    obj_path_fault_alarm.ingress_logical_port);
    } else {
      pfc_log_info("No logical port associated with ingress switch id");
    }
    // Get the associated logical_port_id for egress switch
    logical_port_class.GetAllPortId(data_type,
                                    controller_name,
                                    egress_switch_id,
                                    egress_logical_port_id,
                                    true);
    memset(obj_path_fault_alarm.egress_logical_port, 0, 320);
    if (!egress_logical_port_id.empty()) {
      memcpy(obj_path_fault_alarm.egress_logical_port,
             egress_logical_port_id[0].c_str(),
             egress_logical_port_id[0].length()+1);
      pfc_log_debug("Egress Logical Port: %s",
                    obj_path_fault_alarm.egress_logical_port);
    } else {
      pfc_log_info("No logical port associated with egress switch id");
    }
    ingress_logical_port_id.clear();
    egress_logical_port_id.clear();
    pfc_log_debug("Get LogPortIds for corresponding ingress switch PhyPorts");
    // Get all logical ports associated with ingress switch
    logical_port_class.GetAllPortId(data_type,
                                    controller_name,
                                    ingress_switch_id,
                                    ingress_logical_port_id,
                                    false);
    pfc_log_debug("Get LogPortIds for corresponding egress switch PhyPorts");
    // Get all logical ports associated with egress switch
    logical_port_class.GetAllPortId(data_type,
                                    controller_name,
                                    egress_switch_id,
                                    egress_logical_port_id,
                                    false);
    obj_path_fault_alarm.ingress_num_of_ports = static_cast<uint16_t>
    (ingress_logical_port_id.size());
    obj_path_fault_alarm.egress_num_of_ports =  static_cast<uint16_t>
    (egress_logical_port_id.size());
    pfc_log_debug("No.of Ingress Logical Port: %d",
                  obj_path_fault_alarm.ingress_num_of_ports);
    pfc_log_debug("No.of Egress Logical Port: %d",
                  obj_path_fault_alarm.egress_num_of_ports);
    memset(obj_path_fault_alarm.valid, UNC_VF_VALID, 4);
    // Send notification to Northbound
    ServerEvent ser_evt((pfc_ipcevtype_t)UPPL_ALARMS_PHYS_PATH_FAULT, err);
    northbound_alarm_header rsh = { oper_type,
        data_type,
        UNC_KT_CONTROLLER,
        UPPL_ALARMS_PHYS_PATH_FAULT};
    err = PhyUtil::sessOutNBAlarmHeader(ser_evt, rsh);
    err |= ser_evt.addOutput(*obj_key_ctr);
    err |= ser_evt.addOutput(obj_path_fault_alarm);
    // N number of  UINT8 logical_port[320] has to be added
    // Get all logical_port_ids associated with switch and physical port
    for (unsigned int index = 0 ; index <  ingress_logical_port_id.size();
        ++index) {
      err |= ser_evt.addOutput(ingress_logical_port_id[index]);
    }
    for (unsigned int index = 0 ; index <  egress_logical_port_id.size();
        ++index) {
      err |= ser_evt.addOutput(egress_logical_port_id[index]);
    }
    if (err != 0) {
      pfc_log_error("Server Event addOutput failed, return IPC_WRITE_ERROR");
      status = UPPL_RC_ERR_IPC_WRITE_ERROR;
    } else {
      // Call IPC server to post the event
      status = (UpplReturnCode) physical_layer->get_ipc_connection_manager()->
          SendEvent(&ser_evt);
    }
  } else {
    pfc_log_warn("%d alarm received for controller is ignored", alarm_type);
  }
  return status;
}

/** IsKeyExists
 * * @Description : This function checks whether the controller_id exists in DB
 * * * @param[in] : data type - UNC_DT_*
 * key value - Contains controller_id
 * * * @return    : UPPL_RC_SUCCESS or UPPL_RC_ERR* based on operation type
 * */
UpplReturnCode Kt_Controller::IsKeyExists(unc_keytype_datatype_t data_type,
                                          vector<string> key_values) {
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  UpplReturnCode check_status = UPPL_RC_SUCCESS;
  if (key_values.empty()) {
    // No key given, return failure
    pfc_log_error("No key given. Returning error");
    return UPPL_RC_ERR_BAD_REQUEST;
  }
  string controller_name = key_values[0];
  // Structure used to send request to ODBC
  DBTableSchema kt_controller_dbtableschema;

  // Construct Primary key list
  vector<string> vect_prim_keys;
  vect_prim_keys.push_back(CTR_NAME);

  // Construct TableAttrSchema structure
  // TableAttrSchema holds table_name, primary key, attr_name
  vector<TableAttrSchema> vect_table_attr_schema;
  list< vector<TableAttrSchema> > row_list;

  // Controller_name
  PhyUtil::FillDbSchema(CTR_NAME, controller_name,
                        controller_name.length(), DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);

  kt_controller_dbtableschema.set_table_name(UPPL_CTR_TABLE);
  kt_controller_dbtableschema.set_primary_keys(vect_prim_keys);
  row_list.push_back(vect_table_attr_schema);
  kt_controller_dbtableschema.set_row_list(row_list);

  // Send request to ODBC for controlle_common_table
  ODBCM_RC_STATUS check_db_status = physical_layer->get_odbc_manager()->\
      IsRowExists(data_type, kt_controller_dbtableschema);
  if (check_db_status == ODBCM_RC_CONNECTION_ERROR) {
    // log error to log daemon
    pfc_log_error("DB connection not available or cannot access DB");
    check_status = UPPL_RC_ERR_DB_ACCESS;
  } else if (check_db_status == ODBCM_RC_ROW_EXISTS) {
    pfc_log_debug("DB returned success for Row exists");
    pfc_log_debug("Checking .db_return_status_ %d with %d",
                  kt_controller_dbtableschema.db_return_status_,
                  static_cast<int>(DELETED));
    if (kt_controller_dbtableschema.db_return_status_ != DELETED) {
      pfc_log_debug("DB returned success for Row exists");
    } else {
      pfc_log_debug("DB Returned failure for IsRowExists");
      check_status = UPPL_RC_ERR_DB_GET;
    }
  } else {
    pfc_log_debug("DB Returned failure for IsRowExists");
    check_status = UPPL_RC_ERR_DB_GET;
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
UpplReturnCode Kt_Controller::HandleOperStatus(uint32_t data_type,
                                               void *key_struct,
                                               void *value_struct) {
  UpplReturnCode return_code = UPPL_RC_SUCCESS;
  return_code = HandleOperStatus(data_type,
                                 key_struct,
                                 value_struct,
                                 false);
  return return_code;
}

/** HandleOperStatus
 * * @Description : This function performs the required actions when oper status
 * changes
 * * * @param[in] : Key struct - identifies the controller key instance
 * value struct - identifies the controller value structure
 * * * @return    : UPPL_RC_SUCCESS or UPPL_RC_ERR*
 * */
UpplReturnCode Kt_Controller::HandleOperStatus(uint32_t data_type,
                                               void *key_struct,
                                               void *value_struct,
                                               bool bIsInternal) {
  FN_START_TIME("HandleOperStatus", "Controller");
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  UpplReturnCode return_code = UPPL_RC_SUCCESS;
  val_ctr_st *obj_val_ctr = reinterpret_cast<val_ctr_st*>(value_struct);
  key_ctr_t *obj_key_ctr = reinterpret_cast<key_ctr_t*>(key_struct);
  string controller_name = (const char*)obj_key_ctr->controller_name;
  uint8_t oper_value = obj_val_ctr->oper_status;
  uint8_t oper_status_db = 0;

  if (bIsInternal == false) {
    // If oper status is UP, set is as UPPL_CONTROLLER_OPER_AUDIT_WAITING
    if (obj_val_ctr->oper_status == UPPL_CONTROLLER_OPER_UP) {
      oper_value = UPPL_CONTROLLER_OPER_WAITING_AUDIT;
    }
  }
  UpplReturnCode read_status = GetOperStatus(data_type,
                                             key_struct,
                                             oper_status_db);
  pfc_log_debug("Get OperStatus return: %d", read_status);
  // Update oper_status in controller_table
  return_code = SetOperStatus(data_type, key_struct, oper_value);
  if (return_code != UPPL_RC_SUCCESS) {
    // log error
    pfc_log_error("oper_status update operation failed");
    return return_code;
  }

  if (bIsInternal && (obj_val_ctr->oper_status == UPPL_CONTROLLER_OPER_UP ||
      obj_val_ctr->oper_status == UPPL_CONTROLLER_OPER_DOWN)) {
    if (obj_val_ctr->oper_status != oper_status_db) {
      // Send notification to Northbound
      return_code = SendOperStatusNotification(*obj_key_ctr,
                                               oper_status_db,
                                               obj_val_ctr->oper_status);
      pfc_log_debug("Notification status %d", return_code);
    }
  }

  // Send CONTROLLER_DISCONNECT, CONTROLLER_CONNECT alarm to node manager
  // current operstatus in ipc message is DOWN
  if (obj_val_ctr->oper_status == UPPL_CONTROLLER_OPER_DOWN) {
    // Send CONTROLLER_DISCONNECT alarm
    // Call PhysicalCore's SendControllerDisconnectAlarm()
    pfc_log_info("Sending Controller Disconnect alarm");
    UpplReturnCode alarms_status=
        physical_layer->get_physical_core()->
        SendControllerDisconnectAlarm(controller_name);
    pfc_log_info("Alarm status: %d", alarms_status);
    return_code = NotifyOperStatus(UNC_DT_RUNNING,
                                   key_struct,
                                   value_struct);
  } else if (obj_val_ctr->oper_status == UPPL_CONTROLLER_OPER_UP) {
    // Send clearance for CONTROLLER_DISCONNECT alarm
    // Send CONTROLLER_CONNECT alarm
    // Call PhysicalCore's SendControllerConnectAlarm()
    pfc_log_info("Sending Controller Connect alarm");
    UpplReturnCode alarms_status=
        physical_layer->get_physical_core()->
        SendControllerConnectAlarm(controller_name);
    pfc_log_info("Alarm status: %d", alarms_status);
  }
  FN_END_TIME("HandleOperStatus", "Controller");
  return UPPL_RC_SUCCESS;
}

/** NotifyOperStatus
 * * @Description : This function performs the notifies other associated
 * key types when oper status changes
 * * * @param[in] : Key and value struct
 * * * @return    : UPPL_RC_SUCCESS or UPPL_RC_ERR*
 * */
UpplReturnCode Kt_Controller::NotifyOperStatus(uint32_t data_type,
                                               void *key_struct,
                                               void *value_struct) {
  UpplReturnCode return_code = UPPL_RC_SUCCESS;
  key_ctr *key = reinterpret_cast<key_ctr_t*>(key_struct);
  string controller_name = reinterpret_cast<char*>(key->controller_name);

  // Call the associated key types to notify controller's
  // oper_status change
  for (unsigned int index = 0 ; index < KT_CONTROLLER_OPER_STATUS_REF ;
      ++index) {
    void *ref_key_struct = NULL;
    Kt_Base *class_pointer = GetClassPointerAndKey(
        (KtControllerOperStatusRef)index, controller_name, ref_key_struct);
    return_code = class_pointer->HandleOperStatus(data_type, ref_key_struct,
                                                  NULL);
    pfc_log_info("HandleOperStatus for %d child class %d",
                 index, return_code);
    delete class_pointer;
    FreeKeyStruct(ref_key_struct, index);
  }
  /* Construct UNC_KT_BOUNDARY value struct
  Kt_Boundary boundary;
  val_boundary_t obj_val_boundary1;
  memcpy(obj_val_boundary1.controller_name1,
         controller_name.c_str(), controller_name.length()+1);
  memset(obj_val_boundary1.domain_name1, 0, 32);
  memset(obj_val_boundary1.logical_port_id1, 0, 320);
  memset(obj_val_boundary1.valid, 0, 7);
  obj_val_boundary1.valid[kIdxBoundaryControllerName1] = UNC_VF_VALID;
  return_code = boundary.HandleOperStatus(
      NULL, reinterpret_cast<void *> (&obj_val_boundary1));
  pfc_log_info("HandleOperStatus for boundary class %d", return_code);

  val_boundary_t obj_val_boundary2;
  memcpy(obj_val_boundary2.controller_name2,
         controller_name.c_str(), controller_name.length()+1);
  memset(obj_val_boundary2.domain_name2, 0, 32);
  memset(obj_val_boundary2.logical_port_id2, 0, 320);
  memset(obj_val_boundary2.valid, 0, 7);
  obj_val_boundary2.valid[kIdxBoundaryControllerName2] = UNC_VF_VALID;
  return_code = boundary.HandleOperStatus(data_type,
      NULL, reinterpret_cast<void *> (&obj_val_boundary2));
  pfc_log_info("HandleOperStatus for boundary class %d", return_code);*/
  return return_code;
}

/** PopulateDBSchemaForKtTable
 * * @Description : This function populates the DBAttrSchema to be used to send
 *                  request to ODBC
 * * * @param[in] : DBTableSchema, key_struct, val_struct, operation_type
 * * * @return    : UPPL_RC_SUCCESS or UPPL_RC_ERR*
 * */
void Kt_Controller::PopulateDBSchemaForKtTable(
    DBTableSchema &kt_controller_dbtableschema,
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

  // Construct TableAttrSchema structure
  // TableAttrSchema holds table_name, primary key, attr_name
  vector<TableAttrSchema> vect_table_attr_schema;
  list < vector<TableAttrSchema> > row_list;

  key_ctr_t *obj_key_ctr = reinterpret_cast<key_ctr_t*>(key_struct);
  val_ctr_t *obj_val_ctr = reinterpret_cast<val_ctr_t*>(val_struct);
  vect_prim_keys.push_back(CTR_NAME);
  stringstream valid;
  pfc_log_info("operation: %d", operation_type);
  // Controller_name
  string controller_name = (const char*)obj_key_ctr->controller_name;
  if (operation_type == UNC_OP_READ_SIBLING_BEGIN ||
      operation_type == UNC_OP_READ_SIBLING_COUNT) {
    // Ignore controller_name key value
    controller_name = "";
  }
  pfc_log_info("controller name: %s", controller_name.c_str());
  PhyUtil::FillDbSchema(CTR_NAME, controller_name,
                        controller_name.length(), DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);

  val_ctr_st_t val_ctr_valid_st;
  memset(&val_ctr_valid_st, 0, sizeof(val_ctr_st_t));
  if (operation_type == UNC_OP_UPDATE) {
    // get valid array for update req
    pfc_log_debug("Get Valid value from DB");
    GetCtrValidFlag(key_struct, val_ctr_valid_st);
  }

  uint16_t valid_val = 0, prev_db_val = 0;
  string value;
  unc_keytype_ctrtype_t ctr_type = UNC_CT_UNKNOWN;
  // Type
  if (obj_val_ctr != NULL) {
    valid_val = PhyUtil::uint8touint(obj_val_ctr->valid[kIdxType]);
    value = PhyUtil::uint8tostr(obj_val_ctr->type);
    ctr_type = (unc_keytype_ctrtype_t)PhyUtil::uint8touint(obj_val_ctr->type);
    prev_db_val =
        PhyUtil::uint8touint(val_ctr_valid_st.controller.valid[kIdxType]);
  } else {
    valid_val = UPPL_NO_VAL_STRUCT;
  }
  PhyUtil::FillDbSchema(CTR_TYPE, value,
                        value.length(), DATATYPE_UINT16,
                        operation_type, valid_val, prev_db_val,
                        vect_table_attr_schema, vect_prim_keys, valid);
  value.clear();
  // version
  if (obj_val_ctr != NULL) {
    valid_val = PhyUtil::uint8touint(obj_val_ctr->valid[kIdxVersion]);
    value  = (const char*)obj_val_ctr->version;
    prev_db_val =
        PhyUtil::uint8touint(val_ctr_valid_st.controller.valid[kIdxVersion]);
  } else {
    valid_val = UPPL_NO_VAL_STRUCT;
  }
  PhyUtil::FillDbSchema(CTR_VERSION, value,
                        value.length(), DATATYPE_UINT8_ARRAY_32,
                        operation_type, valid_val, prev_db_val,
                        vect_table_attr_schema, vect_prim_keys, valid);
  value.clear();
  // Description
  if (obj_val_ctr != NULL) {
    valid_val = PhyUtil::uint8touint(obj_val_ctr->valid[kIdxDescription]);
    value = (const char*)obj_val_ctr->description;
    prev_db_val =
        PhyUtil::uint8touint(
            val_ctr_valid_st.controller.valid[kIdxDescription]);
  } else {
    valid_val = UPPL_NO_VAL_STRUCT;
  }
  PhyUtil::FillDbSchema(CTR_DESCRIPTION, value,
                        value.length(), DATATYPE_UINT8_ARRAY_128,
                        operation_type, valid_val, prev_db_val,
                        vect_table_attr_schema, vect_prim_keys, valid);
  // Ip_address
  char *ip_value = new char[16];
  if (obj_val_ctr != NULL) {
    valid_val = PhyUtil::uint8touint(obj_val_ctr->valid[kIdxIpAddress]);
    inet_ntop(AF_INET, &obj_val_ctr->ip_address, ip_value, INET_ADDRSTRLEN);
    prev_db_val =
        PhyUtil::uint8touint(
            val_ctr_valid_st.controller.valid[kIdxIpAddress]);
  } else {
    valid_val = UPPL_NO_VAL_STRUCT;
  }
  PhyUtil::FillDbSchema(CTR_IP_ADDRESS, ip_value,
                        strlen(ip_value), DATATYPE_IPV4,
                        operation_type, valid_val, prev_db_val,
                        vect_table_attr_schema, vect_prim_keys, valid);
  delete []ip_value;
  value.clear();
  // user
  if (obj_val_ctr != NULL) {
    valid_val = PhyUtil::uint8touint(obj_val_ctr->valid[kIdxUser]);
    value = (const char*) obj_val_ctr->user;
    prev_db_val =
        PhyUtil::uint8touint(
            val_ctr_valid_st.controller.valid[kIdxUser]);
  } else {
    valid_val = UPPL_NO_VAL_STRUCT;
  }
  PhyUtil::FillDbSchema(CTR_USER_NAME, value,
                        value.length(), DATATYPE_UINT8_ARRAY_32,
                        operation_type, valid_val, prev_db_val,
                        vect_table_attr_schema, vect_prim_keys, valid);
  value.clear();
  // password
  if (obj_val_ctr != NULL) {
    valid_val = PhyUtil::uint8touint(obj_val_ctr->valid[kIdxPassword]);
    value = (const char*) obj_val_ctr->password;
    prev_db_val =
        PhyUtil::uint8touint(
            val_ctr_valid_st.controller.valid[kIdxPassword]);
  } else {
    valid_val = UPPL_NO_VAL_STRUCT;
  }
  PhyUtil::FillDbSchema(CTR_PASSWORD, value,
                        value.length(), DATATYPE_UINT8_ARRAY_257,
                        operation_type, valid_val, prev_db_val,
                        vect_table_attr_schema, vect_prim_keys, valid);
  value.clear();
  // enable_audit
  if (obj_val_ctr != NULL) {
    valid_val = PhyUtil::uint8touint(obj_val_ctr->valid[kIdxEnableAudit]);
    value = PhyUtil::uint8tostr(obj_val_ctr->enable_audit);
    prev_db_val =
        PhyUtil::uint8touint(
            val_ctr_valid_st.controller.valid[kIdxEnableAudit]);
  } else {
    valid_val = UPPL_NO_VAL_STRUCT;
  }
  PhyUtil::FillDbSchema(CTR_ENABLE_AUDIT, value,
                        value.length(), DATATYPE_UINT16,
                        operation_type, valid_val, prev_db_val,
                        vect_table_attr_schema, vect_prim_keys, valid);
  value.clear();
  // actual_version
  valid_val = UNC_VF_INVALID;
  prev_db_val = 0;
  value.clear();
  PhyUtil::FillDbSchema(CTR_ACTUAL_VERSION, value,
                        value.length(), DATATYPE_UINT8_ARRAY_32,
                        operation_type, valid_val, prev_db_val,
                        vect_table_attr_schema, vect_prim_keys, valid);
  valid_val = UPPL_NO_VAL_STRUCT;
  // oper_status
  if (operation_type == UNC_OP_CREATE &&
      (ctr_type == UNC_CT_UNKNOWN)) {
    // Oper_status for Unknown will be always up
    stringstream oper_value;
    oper_value << UPPL_CONTROLLER_OPER_UP;
    value = oper_value.str();
    valid_val = UNC_VF_VALID;
  }
  PhyUtil::FillDbSchema(CTR_OPER_STATUS, value,
                        value.length(), DATATYPE_UINT16,
                        operation_type, valid_val, prev_db_val,
                        vect_table_attr_schema, vect_prim_keys, valid);
  // valid
  stringstream dummy_valid;
  PhyUtil::FillDbSchema(CTR_VALID, valid.str(),
                        valid.str().length(), DATATYPE_UINT8_ARRAY_9,
                        operation_type, valid_val, prev_db_val,
                        vect_table_attr_schema, vect_prim_keys, dummy_valid);
  // cs_attr_status
  stringstream attr_status;
  for (unsigned int index = 0; index < ODBCM_SIZE_9; ++index) {
    attr_status << CREATED;
  }
  PhyUtil::FillDbSchema(CTR_CS_ATTR, attr_status.str(),
                        attr_status.str().length(), DATATYPE_UINT8_ARRAY_9,
                        operation_type, valid_val, prev_db_val,
                        vect_table_attr_schema, vect_prim_keys, dummy_valid);
  // cs_row status
  if (is_filtering == true) {
    vect_prim_keys.push_back(CTR_CS_ROW_STATUS);
  }
  value = PhyUtil::uint8tostr(row_status);
  if (operation_type >= UNC_OP_READ) {
    PhyUtil::FillDbSchema(CTR_CS_ROW_STATUS, value,
                          value.length(), DATATYPE_UINT16,
                          vect_table_attr_schema);
  }
  PhyUtil::reorder_col_attrs(vect_prim_keys, vect_table_attr_schema);
  kt_controller_dbtableschema.set_table_name(UPPL_CTR_TABLE);
  kt_controller_dbtableschema.set_primary_keys(vect_prim_keys);
  row_list.push_back(vect_table_attr_schema);
  kt_controller_dbtableschema.set_row_list(row_list);
  return;
}

/**
 * * @Description : This function populates val_ctr_t by values retrieved
 * from database
 * * * @param[in] : controller common table dbtable schema,
 * value structure and max_rep_ct, operation type
 * * * @return    : Filled val_ctr and controller id
 * */
void Kt_Controller::FillControllerValueStructure(
    DBTableSchema &kt_controller_dbtableschema,
    vector<val_ctr_st_t> &vect_obj_val_ctr,
    uint32_t &max_rep_ct,
    uint32_t operation_type,
    vector<string> &controller_id) {
  // populate IPC value structure based on the response recevied from DB
  list < vector<TableAttrSchema> > res_ctr_row_list =
      kt_controller_dbtableschema.get_row_list();
  list < vector<TableAttrSchema> > :: iterator res_ctr_iter =
      res_ctr_row_list.begin();
  max_rep_ct = res_ctr_row_list.size();
  pfc_log_debug("res_ctr_row_list.size: %d", max_rep_ct);
  // populate IPC value structure based on the response recevied from DB
  for (; res_ctr_iter != res_ctr_row_list.end(); ++res_ctr_iter) {
    vector<TableAttrSchema> res_ctr_table_attr_schema = (*res_ctr_iter);
    vector<TableAttrSchema> :: iterator vect_ctr_iter =
        res_ctr_table_attr_schema.begin();
    val_ctr_st_t obj_val_ctr_st;
    memset(obj_val_ctr_st.valid, '\0', 3);
    val_ctr_t obj_val_ctr;
    memset(&obj_val_ctr, 0, sizeof(val_ctr_t));
    memset(obj_val_ctr.valid, '\0', 7);
    memset(obj_val_ctr.cs_attr, '\0', 7);
    // Read all attributes
    vector<int> valid_flag, cs_attr;
    for (; vect_ctr_iter != res_ctr_table_attr_schema.end();
        ++vect_ctr_iter) {
      // populate values from controller_table
      TableAttrSchema tab_schema = (*vect_ctr_iter);
      string attr_name = tab_schema.table_attribute_name;
      string attr_value;
      if (attr_name == CTR_NAME) {
        PhyUtil::GetValueFromDbSchema(tab_schema, attr_value,
                                      DATATYPE_UINT8_ARRAY_32);
        controller_id.push_back(attr_value);
        pfc_log_info("controller_name: %s", attr_value.c_str());
      }
      if (attr_name == CTR_TYPE) {
        PhyUtil::GetValueFromDbSchema(tab_schema, attr_value,
                                      DATATYPE_UINT16);
        obj_val_ctr.type = atoi(attr_value.c_str());
      }
      if (attr_name == CTR_DESCRIPTION) {
        PhyUtil::GetValueFromDbSchema(tab_schema, attr_value,
                                      DATATYPE_UINT8_ARRAY_128);
        memcpy(obj_val_ctr.description,
               attr_value.c_str(),
               attr_value.length()+1);
      }
      if (attr_name == CTR_IP_ADDRESS) {
        PhyUtil::GetValueFromDbSchema(tab_schema, attr_value,
                                      DATATYPE_IPV4);
        inet_pton(AF_INET, (const char *)attr_value.c_str(),
                  &obj_val_ctr.ip_address.s_addr);
      }
      if (attr_name == CTR_USER_NAME) {
        PhyUtil::GetValueFromDbSchema(tab_schema, attr_value,
                                      DATATYPE_UINT8_ARRAY_32);
        memcpy(obj_val_ctr.user,
               attr_value.c_str(),
               attr_value.length()+1);
      }
      if (attr_name == CTR_PASSWORD) {
        PhyUtil::GetValueFromDbSchema(tab_schema, attr_value,
                                      DATATYPE_UINT8_ARRAY_257);
        memcpy(obj_val_ctr.password,
               attr_value.c_str(),
               attr_value.length()+1);
      }
      if (attr_name == CTR_VALID) {
        PhyUtil::GetValueFromDbSchema(tab_schema, attr_value,
                                      DATATYPE_UINT8_ARRAY_9);
        memset(obj_val_ctr.valid, '\0', 7);
        FrameValidValue(attr_value, obj_val_ctr_st, obj_val_ctr);
      }
      if (attr_name == CTR_CS_ATTR) {
        PhyUtil::GetValueFromDbSchema(tab_schema, attr_value,
                                      DATATYPE_UINT8_ARRAY_9);
        memset(obj_val_ctr.cs_attr, '\0', 7);
        FrameCsAttrValue(attr_value, obj_val_ctr);
      }
      if (attr_name == CTR_CS_ROW_STATUS) {
        PhyUtil::GetValueFromDbSchema(tab_schema, attr_value,
                                      DATATYPE_UINT16);
        obj_val_ctr.cs_row_status = atoi(attr_value.c_str());
      }
      if (attr_name == CTR_ENABLE_AUDIT) {
        PhyUtil::GetValueFromDbSchema(tab_schema, attr_value,
                                      DATATYPE_UINT16);
        obj_val_ctr.enable_audit = atoi(attr_value.c_str());
      }
      if (attr_name == CTR_VERSION) {
        PhyUtil::GetValueFromDbSchema(tab_schema, attr_value,
                                      DATATYPE_UINT8_ARRAY_32);
        memcpy(obj_val_ctr.version,
               attr_value.c_str(),
               attr_value.length()+1);
      }
      if (attr_name == CTR_OPER_STATUS) {
        PhyUtil::GetValueFromDbSchema(tab_schema, attr_value,
                                      DATATYPE_UINT16);
        obj_val_ctr_st.oper_status = atoi(attr_value.c_str());
      }
      if (attr_name == CTR_ACTUAL_VERSION) {
        PhyUtil::GetValueFromDbSchema(tab_schema, attr_value,
                                      DATATYPE_UINT8_ARRAY_32);
        memcpy(obj_val_ctr_st.actual_version,
               attr_value.c_str(),
               attr_value.length()+1);
      }
    }
    memcpy(&obj_val_ctr_st.controller, &obj_val_ctr, sizeof(val_ctr_t));
    vect_obj_val_ctr.push_back(obj_val_ctr_st);
  }
  return;
}

/** PerformRead
 * * @Description : This function reads the instance of KT_Controller based on
 *                  operation type - READ, READ_SIBLING_BEGIN, READ_SIBLING
 * * * @param[in] : ipc session id, configuration id, key_struct, value_struct,
 *                  data_type, operation type, ServerSession, option1, option2,
 *                  max_rep_ct
 * * * @return    : UPPL_RC_SUCCESS or UPPL_RC_ERR*
 * */
UpplReturnCode Kt_Controller::PerformRead(uint32_t session_id,
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
  key_ctr_t *obj_key_ctr= reinterpret_cast<key_ctr_t*>(key_struct);
  UpplReturnCode read_status = UPPL_RC_SUCCESS;
  if (option1 != UNC_OPT1_NORMAL) {
    pfc_log_error("Invalid option1 specified for read operation");
    physical_response_header rsh = {session_id,
        configuration_id,
        operation_type,
        max_rep_ct,
        option1,
        option2,
        data_type,
        UPPL_RC_ERR_INVALID_OPTION1};
    int err = PhyUtil::sessOutRespHeader(sess, rsh);
    err |= sess.addOutput((uint32_t)UNC_KT_CONTROLLER);
    err |= sess.addOutput(*obj_key_ctr);
    if (err != 0) {
      pfc_log_debug("addOutput failed for physical_response_header");
      return UPPL_RC_ERR_IPC_WRITE_ERROR;
    }
    return UPPL_RC_SUCCESS;
  }
  if (option2 != UNC_OPT2_NONE) {
    pfc_log_error("Invalid option2 specified for read operation");
    physical_response_header rsh = {session_id,
        configuration_id,
        operation_type,
        max_rep_ct,
        option1,
        option2,
        data_type,
        UPPL_RC_ERR_INVALID_OPTION2};
    int err = PhyUtil::sessOutRespHeader(sess, rsh);
    err |= sess.addOutput((uint32_t)UNC_KT_CONTROLLER);
    err |= sess.addOutput(*obj_key_ctr);
    if (err != 0) {
      pfc_log_debug("addOutput failed for physical_response_header");
      return UPPL_RC_ERR_IPC_WRITE_ERROR;
    }
    return UPPL_RC_SUCCESS;
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
    physical_response_header rsh = {session_id,
        configuration_id,
        operation_type,
        max_rep_ct,
        option1,
        option2,
        data_type,
        UPPL_RC_ERR_OPERATION_NOT_ALLOWED};
    int err = PhyUtil::sessOutRespHeader(sess, rsh);
    err |= sess.addOutput((uint32_t)UNC_KT_CONTROLLER);
    err |= sess.addOutput(*obj_key_ctr);
    if (err != 0) {
      pfc_log_error("addOutput failed for physical_response_header");
      return UPPL_RC_ERR_IPC_WRITE_ERROR;
    }
    return UPPL_RC_SUCCESS;
  }
  // Read from DB directly for all data types
  vector<string> vect_controller_id;
  vector<val_ctr_st_t> vect_val_ctr_st;
  read_status = ReadCtrValFromDB(key_struct,
                                 val_struct,
                                 data_type,
                                 operation_type,
                                 max_rep_ct,
                                 vect_val_ctr_st,
                                 vect_controller_id);
  physical_response_header rsh = {session_id,
      configuration_id,
      operation_type,
      max_rep_ct,
      option1,
      option2,
      data_type,
      read_status};
  if (read_status == UPPL_RC_SUCCESS) {
    for (unsigned int index = 0; index < vect_val_ctr_st.size(); ++index) {
      string controller_name = vect_controller_id[index];
      if (data_type == UNC_DT_CANDIDATE &&
          vect_val_ctr_st[index].controller.cs_row_status == DELETED) {
        pfc_log_debug("Ignoring DELETED entry %s", controller_name.c_str());
        if (rsh.max_rep_count > 0)
          rsh.max_rep_count--;
        continue;
      }
    }
    if (rsh.max_rep_count == 0) {
      rsh.result_code = UPPL_RC_ERR_NO_SUCH_INSTANCE;
      int err = PhyUtil::sessOutRespHeader(sess, rsh);
      err |= sess.addOutput((uint32_t)UNC_KT_CONTROLLER);
      err |= sess.addOutput(*obj_key_ctr);
      if (err != 0) {
        pfc_log_error("addOutput failed for physical_response_header");
        return UPPL_RC_ERR_IPC_WRITE_ERROR;
      }
      return UPPL_RC_SUCCESS;
    }
    int err = PhyUtil::sessOutRespHeader(sess, rsh);
    if (err != 0) {
      pfc_log_debug("addOutput failed for physical_response_header");
      return UPPL_RC_ERR_IPC_WRITE_ERROR;
    }
    for (unsigned int index = 0; index < vect_val_ctr_st.size(); ++index) {
      key_ctr_t obj_ctr;
      string controller_name = vect_controller_id[index];
      if (data_type == UNC_DT_CANDIDATE &&
          vect_val_ctr_st[index].controller.cs_row_status == DELETED) {
        pfc_log_debug("Ignoring DELETED entry %s", controller_name.c_str());
        continue;
      }
      pfc_log_debug("Adding controller name: %s",
                    controller_name.c_str());
      memset(&obj_ctr, 0, sizeof(obj_ctr));
      memcpy(obj_ctr.controller_name,
             controller_name.c_str(),
             controller_name.length()+1);
      sess.addOutput((uint32_t)UNC_KT_CONTROLLER);
      sess.addOutput(obj_ctr);
      if ((unc_keytype_datatype_t)data_type == UNC_DT_STATE) {
        sess.addOutput(vect_val_ctr_st[index]);
      } else {
        sess.addOutput(vect_val_ctr_st[index].controller);
      }
      if (index < vect_val_ctr_st.size()-1) {
        sess.addOutput();  // Separator
      }
    }
  } else {
    pfc_log_info("Read operation failed with %d", read_status);
    rsh.result_code = read_status;
    rsh.max_rep_count = 0;
    int err = PhyUtil::sessOutRespHeader(sess, rsh);
    err |= sess.addOutput((uint32_t)UNC_KT_CONTROLLER);
    err |= sess.addOutput(*obj_key_ctr);
    if (err != 0) {
      pfc_log_debug("addOutput failed for physical_response_header");
      return UPPL_RC_ERR_IPC_WRITE_ERROR;
    }
  }
  pfc_log_info("Return value for read operation %d", read_status);
  return read_status;
}

/** ReadCtrValFromDB
 * * @Description : This function reads the instance of KT_Controller based on
 *                  operation type - READ, READ_SIBLING_BEGIN, READ_SIBLING
 *                   from data base
 * * * @param[in] : key_struct, value_struct, ipc session id, configuration id,
 *                  data_type, operation type, max_rep_ct
 * * * @return    : UPPL_RC_SUCCESS or UPPL_RC_ERR*
 * */
UpplReturnCode Kt_Controller::ReadCtrValFromDB(
    void* key_struct,
    void* val_struct,
    uint32_t data_type,
    uint32_t operation_type,
    uint32_t &max_rep_ct,
    vector<val_ctr_st_t> &vect_val_ctr_st,
    vector<string> &controller_id) {
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  UpplReturnCode read_status = UPPL_RC_SUCCESS;
  ODBCM_RC_STATUS read_db_status = ODBCM_RC_SUCCESS;

  // Common structures that will be used to send query to ODBC
  // Structure used to send request to ODBC
  DBTableSchema kt_controller_dbtableschema;
  void *old_val_struct;
  vector<ODBCMOperator> vect_key_operations;
  PopulateDBSchemaForKtTable(kt_controller_dbtableschema,
                             key_struct,
                             val_struct,
                             operation_type, 0, 0,
                             vect_key_operations, old_val_struct);

  if (operation_type == UNC_OP_READ) {
    read_db_status = physical_layer->get_odbc_manager()->
        GetOneRow((unc_keytype_datatype_t)data_type,
                  kt_controller_dbtableschema);
  } else {
    read_db_status = physical_layer->get_odbc_manager()->
        GetBulkRows((unc_keytype_datatype_t)data_type, max_rep_ct,
                    kt_controller_dbtableschema,
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
  FillControllerValueStructure(kt_controller_dbtableschema,
                               vect_val_ctr_st,
                               max_rep_ct,
                               operation_type,
                               controller_id);
  pfc_log_debug("vect_val_ctr size: %d", (unsigned int)vect_val_ctr_st.size());
  pfc_log_debug("controller_id size: %d", (unsigned int)controller_id.size());
  if (vect_val_ctr_st.empty()) {
    // Read failed , return error
    read_status = UPPL_RC_ERR_DB_GET;
    // log error to log daemon
    pfc_log_error("Read operation has failed after reading response");
    return read_status;
  }
  return read_status;
}

/** getChildKeyStruct
 * * @Description : This function returns the void * of child key structures
 * * * @param[in] : child class index and controller name
 * * * @return    : void * key structure
 * */
void* Kt_Controller::getChildKeyStruct(unsigned int child_class,
                                       string controller_name) {
  switch (child_class) {
    case KIdxDomain: {
      key_ctr_domain_t *obj_child_key = new key_ctr_domain_t;
      memcpy(obj_child_key->ctr_key.controller_name,
             controller_name.c_str(),
             controller_name.length()+1);
      memset(obj_child_key->domain_name, 0, 32);
      void* child_key = reinterpret_cast<void *>(obj_child_key);
      return child_key;
    }
    case KIdxSwitch: {
      key_switch_t *obj_child_key = new key_switch_t;
      memcpy(obj_child_key->ctr_key.controller_name,
             controller_name.c_str(),
             controller_name.length()+1);
      memset(obj_child_key->switch_id, 0, 256);
      void* child_key = reinterpret_cast<void *>(obj_child_key);
      return child_key;
    }
    case KIdxLink: {
      key_link_t *obj_child_key = new key_link_t;
      memcpy(obj_child_key->ctr_key.controller_name,
             controller_name.c_str(),
             controller_name.length()+1);
      memset(obj_child_key->switch_id1, 0, 256);
      memset(obj_child_key->switch_id2, 0, 256);
      memset(obj_child_key->port_id1, 0, 32);
      memset(obj_child_key->port_id2, 0, 32);
      void* child_key = reinterpret_cast<void *>(obj_child_key);
      return child_key;
    }
    default: {
      // Do nothing
      pfc_log_info("getChildKeyStruct called for invalid child class %d",
                   child_class);
      PFC_ASSERT(PFC_FALSE);
      return NULL;
    }
  }
}

/** FreeChildKeyStruct
 * * @Description : This function clearsthe void * of child key structures
 * * * @param[in] : child class index and key_struct
 * * * @return    : None
 * */
void Kt_Controller::FreeChildKeyStruct(void* key_struct,
                                       unsigned int child_class) {
  switch (child_class) {
    case KIdxDomain: {
      key_ctr_domain_t *obj_child_key = reinterpret_cast<key_ctr_domain_t *>
      (key_struct);
      if (obj_child_key != NULL) {
        delete obj_child_key;
        obj_child_key = NULL;
      }
      return;
    }
    case KIdxSwitch: {
      key_switch_t *obj_child_key = reinterpret_cast<key_switch_t *>
      (key_struct);
      if (obj_child_key != NULL) {
        delete obj_child_key;
        obj_child_key = NULL;
      }
      return;
    }
    case KIdxLink: {
      key_link_t *obj_child_key = reinterpret_cast<key_link_t *>
      (key_struct);
      if (obj_child_key != NULL) {
        delete obj_child_key;
        obj_child_key = NULL;
      }
      return;
    }
    default: {
      pfc_log_info("FreeChildKeyStruct called for invalid child class %d",
                   child_class);
      PFC_ASSERT(PFC_FALSE);
      // Do nothing
      return;
    }
  }
}

/** GetModifiedRows
 * * @Description : This function reads all KT_Controller with given row_status
 * * * @param[in] : key_struct, value_struct, row_status
 * * * @return    : UPPL_RC_SUCCESS or UPPL_RC_ERR*
 * */
UpplReturnCode Kt_Controller::GetModifiedRows(vector<void *> &obj_key_struct,
                                              CsRowStatus row_status) {
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  uint32_t max_rep_ct = 0;

  vector<val_ctr_st_t> obj_value_struct;

  key_ctr_t obj_key_ctr;
  val_ctr_t val_struct;
  memset(&obj_key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_struct, 0, sizeof(val_ctr_t));

  void *ctr_key = reinterpret_cast <void *> (&obj_key_ctr);
  void *ctr_val = reinterpret_cast <void *> (&val_struct);

  UpplReturnCode read_status = UPPL_RC_SUCCESS;
  ODBCM_RC_STATUS read_db_status = ODBCM_RC_SUCCESS;

  DBTableSchema kt_controller_dbtableschema;
  void *old_val_struct;
  vector<ODBCMOperator> vect_key_operations;
  PopulateDBSchemaForKtTable(kt_controller_dbtableschema,
                             ctr_key, ctr_val,
                             UNC_OP_READ, 0, 0,
                             vect_key_operations, old_val_struct,
                             row_status,
                             true);

  read_db_status = physical_layer->get_odbc_manager()->
      GetModifiedRows(UNC_DT_CANDIDATE, kt_controller_dbtableschema);
  if (read_db_status == ODBCM_RC_RECORD_NOT_FOUND) {
    pfc_log_debug("No record to read");
    read_status = UPPL_RC_ERR_NO_SUCH_INSTANCE;
    return read_status;
  } else if (read_db_status == ODBCM_RC_CONNECTION_ERROR) {
    read_status = UPPL_RC_ERR_DB_ACCESS;
    return read_status;
  } else if (read_db_status != ODBCM_RC_SUCCESS) {
    read_status = UPPL_RC_ERR_DB_GET;
    return read_status;
  }
  vector<string> controller_id;

  FillControllerValueStructure(kt_controller_dbtableschema,
                               obj_value_struct,
                               max_rep_ct,
                               UNC_OP_READ,
                               controller_id);

  // Fill key structures
  // Value structures is available in obj_val_struct
  vector<string> ::iterator vect_iter = controller_id.begin();
  for (; vect_iter != controller_id.end(); ++vect_iter) {
    key_ctr_t *obj_key_ctr = new key_ctr_t;
    memset(obj_key_ctr->controller_name, '\0',
           sizeof(obj_key_ctr->controller_name));
    memcpy(obj_key_ctr->controller_name,
           (const char *)((*vect_iter).c_str()),
           (*vect_iter).length()+1);
    void *key_struct = reinterpret_cast<void *>(obj_key_ctr);
    pfc_log_debug("controller_name: %s",
                  obj_key_ctr->controller_name);
    obj_key_struct.push_back(key_struct);
  }
  return read_status;
}

/** Fill_Attr_Syntax_Map
 * * @Description : This function populates the values to be used for attribute
 * validation
 * * * @param[in] : None
 * * * @return    : None
 * */
void Kt_Controller::Fill_Attr_Syntax_Map() {
  Kt_Class_Attr_Syntax objKeyAttrSyntax =
  { PFC_IPCTYPE_UINT8, 0, 0, 1, 32, true,  "" };
  attr_syntax_map[CTR_NAME] = objKeyAttrSyntax;

  Kt_Class_Attr_Syntax objAttrTypeSyntax =
  { PFC_IPCTYPE_UINT8, 0, 3, 0, 0, true,  "" };
  attr_syntax_map[CTR_TYPE] = objAttrTypeSyntax;

  Kt_Class_Attr_Syntax objAttrVersionSyntax =
  { PFC_IPCTYPE_STRING, 0, 0, 1, 32, true, "" };
  attr_syntax_map[CTR_VERSION] = objAttrVersionSyntax;

  Kt_Class_Attr_Syntax objAttrDescSyntax =
  { PFC_IPCTYPE_STRING, 0, 0, 0, 128, false,  "" };
  attr_syntax_map[CTR_DESCRIPTION] = objAttrDescSyntax;

  Kt_Class_Attr_Syntax objAttrIpSyntax =
  { PFC_IPCTYPE_IPV4, 0, 4294967295LL, 0, 0, false,  "" };
  attr_syntax_map[CTR_IP_ADDRESS] = objAttrIpSyntax;

  Kt_Class_Attr_Syntax objAttrUserSyntax =
  { PFC_IPCTYPE_STRING, 0, 0, 0, 32, false, "" };
  attr_syntax_map["user"] = objAttrUserSyntax;

  Kt_Class_Attr_Syntax objAttrPswdSyntax =
  { PFC_IPCTYPE_STRING, 0, 0, 0, 257, false, "" };
  attr_syntax_map[CTR_PASSWORD] = objAttrPswdSyntax;

  Kt_Class_Attr_Syntax objAttrAuditSyntax =
  { PFC_IPCTYPE_UINT8, 0, 1, 0, 0, false, "" };
  attr_syntax_map[CTR_ENABLE_AUDIT] = objAttrAuditSyntax;

  Kt_Class_Attr_Syntax objAttrValidSyntax =
  { PFC_IPCTYPE_STRING, 0, 0, 0, 8, false, "" };
  attr_syntax_map[CTR_VALID] = objAttrValidSyntax;

  Kt_Class_Attr_Syntax objAttrCsRowSyntax =
  { PFC_IPCTYPE_STRING, 0, 3, 0, 0, false, "" };
  attr_syntax_map[CTR_CS_ROW_STATUS] = objAttrCsRowSyntax;

  Kt_Class_Attr_Syntax objAttrCsAttrSyntax =
  { PFC_IPCTYPE_STRING, 0, 0, 0, 8, false, "" };
  attr_syntax_map[CTR_CS_ATTR] = objAttrCsAttrSyntax;
}

/** GetClassPointerAndKey
 * * @Description : This function returns the void * of requested key structures
 * * and class pointer
 * * * @param[in] : key type and controller name
 * * * @return    : void * key structure
 * */
Kt_Base* Kt_Controller::GetClassPointerAndKey(
    KtControllerOperStatusRef key_type,
    string controller_name,
    void* &key) {
  Kt_Base* class_pointer = NULL;
  switch (key_type) {
    /* case KtCtrDomain: {
      class_pointer = new Kt_Ctr_Domain();
      key_ctr_domain_t *obj_key = new key_ctr_domain_t;
      memcpy(obj_key->ctr_key.controller_name,
             controller_name.c_str(),
             controller_name.length()+1);
      memset(obj_key->domain_name, 0, 32);
      key = reinterpret_cast<void *>(obj_key);
      break;
    } */
    case KtSwitch: {
      class_pointer = new Kt_Switch();
      key_switch_t *obj_key = new key_switch_t;
      memcpy(obj_key->ctr_key.controller_name,
             controller_name.c_str(),
             controller_name.length()+1);
      memset(obj_key->switch_id, 0, 256);
      key = reinterpret_cast<void *>(obj_key);
      break;
    }
    /* case KtLink: {
      class_pointer = new Kt_Link();
      key_link_t *obj_key = new key_link_t;
      memcpy(obj_key->ctr_key.controller_name,
             controller_name.c_str(),
             controller_name.length()+1);
      memset(obj_key->switch_id1, 0, 256);
      memset(obj_key->switch_id2, 0, 256);
      memset(obj_key->port_id1, 0, 32);
      memset(obj_key->port_id2, 0, 32);
      key = reinterpret_cast<void *>(obj_key);
      break;
    }
    case KtPort: {
      class_pointer = new Kt_Port();
      key_port_t *obj_key = new key_port_t;
      memcpy(obj_key->sw_key.ctr_key.controller_name,
             controller_name.c_str(),
             controller_name.length()+1);
      memset(obj_key->sw_key.switch_id, 0, 256);
      memset(obj_key->port_id, 0, 32);
      key = reinterpret_cast<void *>(obj_key);
      break;
    }
    case KtLogicalPort: {
      class_pointer = new Kt_LogicalPort();
      key_logical_port_t *obj_key = new key_logical_port_t;
      memcpy(obj_key->domain_key.ctr_key.controller_name,
             controller_name.c_str(),
             controller_name.length()+1);
      memset(obj_key->domain_key.domain_name, 0, 32);
      memset(obj_key->port_id, 0, 320);
      key = reinterpret_cast<void *>(obj_key);
      break;
    }
    case KtBoundary: {
      // Do nothing
      pfc_log_info("GetClassPointerAndKey - Boundary cannot be handled");
      PFC_ASSERT(PFC_FALSE);
      break;
    } */
  }
  return class_pointer;
}

/** FreeKeyStruct
 * * @Description : This function clears the key struct
 * * * @param[in] : key type and controller name
 * * * @return    : void * key structure
 * */
void Kt_Controller::FreeKeyStruct(void* key_struct,
                                  unsigned int key_type) {
  switch (key_type) {
    /* case KtCtrDomain: {
      key_ctr_domain_t *obj_key = reinterpret_cast<key_ctr_domain_t *>
      (key_struct);
      if (obj_key != NULL) {
        delete obj_key;
        obj_key = NULL;
      }
      break;
    } */
    case KtSwitch: {
      key_switch_t *obj_key = reinterpret_cast<key_switch_t *>
      (key_struct);
      if (obj_key != NULL) {
        delete obj_key;
        obj_key = NULL;
      }
      break;
    }
    /* case KtLink: {
      key_link_t *obj_key = reinterpret_cast<key_link_t *>
      (key_struct);
      if (obj_key != NULL) {
        delete obj_key;
        obj_key = NULL;
      }
      break;
    }
    case KtPort: {
      key_port_t *obj_key = reinterpret_cast<key_port_t *>
      (key_struct);
      if (obj_key != NULL) {
        delete obj_key;
        obj_key = NULL;
      }
      break;
    }
    case KtLogicalPort: {
      key_logical_port_t *obj_key = reinterpret_cast<key_logical_port_t *>
      (key_struct);
      if (obj_key != NULL) {
        delete obj_key;
        obj_key = NULL;
      }
      break;
    }
    case KtBoundary: {
      // Do nothing
      pfc_log_info("FreeKeyStruct - Boundary cannot be handled");
      PFC_ASSERT(PFC_FALSE);
      break;
    } */
  }
  return;
}

/** GetOperStatus
 *  * @Description : This function reads the oper_status value of the controller
 *  * @param[in] : key_struct
 *  * @return    : oper_status
 */
UpplReturnCode Kt_Controller::GetOperStatus(uint32_t data_type,
                                            void* key_struct,
                                            uint8_t &oper_status) {
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  UpplReturnCode return_code = UPPL_RC_SUCCESS;
  vector<string> vect_prim_keys;
  vect_prim_keys.push_back(CTR_NAME);

  vector<TableAttrSchema> vect_table_attr_schema;
  list < vector<TableAttrSchema> > row_list;
  DBTableSchema kt_controller_dbtableschema;

  key_ctr_t *obj_key_ctr = reinterpret_cast<key_ctr_t*>(key_struct);

  // Controller_name
  string controller_name = (const char*)obj_key_ctr->controller_name;
  pfc_log_info("controller name: %s", controller_name.c_str());
  PhyUtil::FillDbSchema(CTR_NAME, controller_name,
                        controller_name.length(), DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);

  // oper_status
  string value;
  PhyUtil::FillDbSchema(CTR_OPER_STATUS, value,
                        value.length(), DATATYPE_UINT16,
                        vect_table_attr_schema);

  kt_controller_dbtableschema.set_table_name(UPPL_CTR_TABLE);
  kt_controller_dbtableschema.set_primary_keys(vect_prim_keys);
  row_list.push_back(vect_table_attr_schema);
  kt_controller_dbtableschema.set_row_list(row_list);

  ODBCM_RC_STATUS read_db_status = physical_layer->get_odbc_manager()->
      GetOneRow(UNC_DT_RUNNING, kt_controller_dbtableschema);
  if (read_db_status == ODBCM_RC_SUCCESS) {
    // populate IPC value structure based on the response recevied from DB
    vector<TableAttrSchema> res_table_attr_schema =
        kt_controller_dbtableschema.get_row_list().front();
    vector<TableAttrSchema> ::iterator vect_iter =
        res_table_attr_schema.begin();
    for (; vect_iter != res_table_attr_schema.end(); ++vect_iter) {
      TableAttrSchema tab_schema = (*vect_iter);
      string attr_name = tab_schema.table_attribute_name;
      string attr_value;
      if (attr_name == CTR_OPER_STATUS) {
        PhyUtil::GetValueFromDbSchema(tab_schema, attr_value,
                                      DATATYPE_UINT16);
        oper_status = atoi(attr_value.c_str());
        pfc_log_info("oper_status from DB: %d", oper_status);
        break;
      }
    }
  } else {
    return_code = UPPL_RC_ERR_DB_GET;
  }
  return return_code;
}

/** SetOperStatus
 *  * @Description : This function updates the oper_status value
 *  of the controller
 *  * @param[in] : key_struct
 *  * @return    : oper_status
 */
UpplReturnCode Kt_Controller::SetOperStatus(uint32_t data_type,
                                            void* key_struct,
                                            uint8_t oper_status) {
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  UpplReturnCode return_code = UPPL_RC_SUCCESS;
  vector<string> vect_prim_keys;
  vect_prim_keys.push_back(CTR_NAME);

  vector<TableAttrSchema> vect_table_attr_schema;
  list < vector<TableAttrSchema> > row_list;
  DBTableSchema kt_controller_dbtableschema;

  key_ctr_t *obj_key_ctr = reinterpret_cast<key_ctr_t*>(key_struct);

  // Controller_name
  string controller_name = (const char*)obj_key_ctr->controller_name;
  pfc_log_info("controller name: %s", controller_name.c_str());
  PhyUtil::FillDbSchema(CTR_NAME, controller_name,
                        controller_name.length(), DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);

  // oper_status
  string value = PhyUtil::uint8tostr(oper_status);
  PhyUtil::FillDbSchema(CTR_OPER_STATUS, value,
                        value.length(), DATATYPE_UINT16,
                        vect_table_attr_schema);

  kt_controller_dbtableschema.set_table_name(UPPL_CTR_TABLE);
  kt_controller_dbtableschema.set_primary_keys(vect_prim_keys);
  row_list.push_back(vect_table_attr_schema);
  kt_controller_dbtableschema.set_row_list(row_list);

  ODBCM_RC_STATUS update_db_status = physical_layer->get_odbc_manager()->
      UpdateOneRow(UNC_DT_RUNNING, kt_controller_dbtableschema);
  if (update_db_status == ODBCM_RC_SUCCESS) {
    pfc_log_info("oper_status updated in DB successfully");
  } else {
    pfc_log_info("oper_status update failed in DB");
    return_code = UPPL_RC_ERR_DB_UPDATE;
  }
  return return_code;
}

/** SetActualVersion
 *  * @Description : This function updates the actual_version value
 *  of the controller
 *  * @param[in] : key_struct
 *  * @return    : oper_status
 */
UpplReturnCode Kt_Controller::SetActualVersion(void* key_struct,
                                               string actual_version) {
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  UpplReturnCode return_code = UPPL_RC_SUCCESS;
  vector<string> vect_prim_keys;
  vect_prim_keys.push_back(CTR_NAME);

  vector<TableAttrSchema> vect_table_attr_schema;
  list < vector<TableAttrSchema> > row_list;
  DBTableSchema kt_controller_dbtableschema;

  // Modify valid flag with VALID set for actual_version
  val_ctr_st_t val_ctr_valid_st;
  memset(&val_ctr_valid_st, 0, sizeof(val_ctr_st_t));
  pfc_log_debug("Get Valid value from DB");
  GetCtrValidFlag(key_struct, val_ctr_valid_st);
  stringstream str_valid;
  for (unsigned int index = 0; index < ODBCM_SIZE_7;
      ++index) {
    unsigned int valid = val_ctr_valid_st.controller.valid[index];
    if (valid >= 48) {
      valid -= 48;
    }
    str_valid << valid;
  }
  str_valid << UNC_VF_VALID;  // Actual Version
  str_valid << UNC_VF_VALID;  // Oper Status
  pfc_log_debug("str_valid %s", str_valid.str().c_str());

  key_ctr_t *obj_key_ctr = reinterpret_cast<key_ctr_t*>(key_struct);

  // Controller_name
  string controller_name = (const char*)obj_key_ctr->controller_name;
  pfc_log_debug("controller name: %s", controller_name.c_str());
  PhyUtil::FillDbSchema(CTR_NAME, controller_name,
                        controller_name.length(), DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);

  // actual_version_update
  PhyUtil::FillDbSchema(CTR_ACTUAL_VERSION, actual_version,
                        actual_version.length(), DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);

  PhyUtil::FillDbSchema(CTR_VALID, str_valid.str(),
                        str_valid.str().length(), DATATYPE_UINT8_ARRAY_9,
                        vect_table_attr_schema);

  kt_controller_dbtableschema.set_table_name(UPPL_CTR_TABLE);
  kt_controller_dbtableschema.set_primary_keys(vect_prim_keys);
  row_list.push_back(vect_table_attr_schema);
  kt_controller_dbtableschema.set_row_list(row_list);

  ODBCM_RC_STATUS update_db_status = physical_layer->get_odbc_manager()->
      UpdateOneRow(UNC_DT_RUNNING, kt_controller_dbtableschema);
  if (update_db_status == ODBCM_RC_SUCCESS) {
    pfc_log_info("actual version updated in DB successfully");
  } else {
    pfc_log_error("actual version update failed in DB");
    return_code = UPPL_RC_ERR_DB_UPDATE;
  }
  return return_code;
}

/**SendUpdatedControllerInfoToUPLL()
 * @Description : Sends the modified controller information to UPLL in
 *                IPC format
 * @param[in]   : operation : opeeration to update UPLL about controller
 *                data_type : DT_CANDIDATE
 *                operation_type : CREATE/UPDATE/DELETE
 *                key_type :- KT_CONTROLLER as of now
 *                key_struct:- key struct for updated controller
 *                val_struct:- val struct for updated controller
 * @return      : Success or associated error code
 * */

UpplReturnCode Kt_Controller::SendUpdatedControllerInfoToUPLL(
    uint32_t data_type,
    uint32_t operation_type,
    uint32_t key_type,
    void* key_struct,
    void* val_struct) {
  pfc_log_debug("Creating a session to logical");
  pfc_ipcconn_t connp = 0;
  int err = pfc_ipcclnt_altopen(UPLL_IPC_CHANNEL_NAME, &connp);
  if (err != 0) {
    pfc_log_error("Could not open upll ipc session");
    return UPPL_RC_ERR_LOGICAL_COMMUNICATION_FAILURE;
  }
  key_ctr_t *obj_key_ctr = reinterpret_cast<key_ctr_t*>(key_struct);
  val_ctr_t *obj_val_ctr = reinterpret_cast<val_ctr_t*>(val_struct);
  ClientSession upll_cli_session(connp, UPLL_IPC_SERVICE_NAME,
                                 UPLL_GLOBAL_CONFIG_SVC_ID, err);
  pfc_log_debug(
      "Request Sent to Logical: data_type %d, operation_type %d"
      "Key type: %d", data_type, operation_type, key_type);
  err |= upll_cli_session.addOutput((uint32_t)UPLL_UPPL_UPDATE_OP);
  err |= upll_cli_session.addOutput(data_type);
  err |= upll_cli_session.addOutput(operation_type);
  err |= upll_cli_session.addOutput(key_type);
  err |= upll_cli_session.addOutput(*obj_key_ctr);
  if (val_struct != NULL) {
    err |= upll_cli_session.addOutput(*obj_val_ctr);
  }
  if (err != UPPL_RC_SUCCESS) {
    pfc_log_error("Error in adding parameters to session");
    return UPPL_RC_ERR_LOGICAL_COMMUNICATION_FAILURE;
  }
  pfc_ipcresp_t resp = 0;
  err = upll_cli_session.invoke(resp);
  if (err != 0 || resp != UPLL_RC_SUCCESS) {
    pfc_log_error(" Request failed to UPLL with error no: %d resp=%d",
                  err, resp);
    return UPPL_RC_ERR_LOGICAL_COMMUNICATION_FAILURE;
  }
  err = pfc_ipcclnt_altclose(connp);
  if (err != 0) {
    pfc_log_info("Unable to close ipc connection");
  }
  return UPPL_RC_SUCCESS;
}

/** GetCtrValidFlag
 * * @Description : This function reads the valid flag from DB
 * * * @param[in] : Key, value struct and newvalid val
 * * * @return    : Success or associated error code
 * */
UpplReturnCode Kt_Controller::GetCtrValidFlag(
    void *key_struct,
    val_ctr_st_t &val_ctr_valid_st) {
  UpplReturnCode return_code = UPPL_RC_SUCCESS;
  vector<void *> vect_key_ctr;
  vect_key_ctr.push_back(key_struct);
  vector<void *> vect_val_port;
  return_code = ReadInternal(vect_key_ctr, vect_val_port,
                             UNC_DT_CANDIDATE, UNC_OP_READ);
  if (return_code == UPPL_RC_SUCCESS) {
    val_ctr_st_t *val_ctr_new_valid_st =
        reinterpret_cast<val_ctr_st_t*>(vect_val_port[0]);
    if (val_ctr_new_valid_st != NULL) {
      val_ctr_valid_st = *val_ctr_new_valid_st;
      delete val_ctr_new_valid_st;
      val_ctr_new_valid_st = NULL;
      key_ctr_t *ctr_key = reinterpret_cast<key_ctr_t*>
      (vect_key_ctr[0]);
      if (ctr_key != NULL) {
        delete ctr_key;
        ctr_key = NULL;
      }
    } else {
      pfc_log_info("update ctr valid return null val");
    }
  } else {
    pfc_log_info("read internal failure from ctr update valid");
  }
  return return_code;
}

/** FrameValidValue
 * * @Description : This function converts the string value from db to uint8
 * * * @param[in] : Attribute value and val_ctr_st
 * * * @return    : Success or associated error code
 * */
void Kt_Controller::FrameValidValue(string attr_value,
                                    val_ctr_st &obj_val_ctr_st,
                                    val_ctr_t &obj_val_ctr) {
  obj_val_ctr_st.valid[kIdxController] = UNC_VF_VALID;
  for (unsigned int index = 0; index < attr_value.length(); ++index) {
    unsigned int valid = attr_value[index];
    if (attr_value[index] >= 48) {
      valid = attr_value[index] - 48;
    }
    if (index > 6) {
      obj_val_ctr_st.valid[index-6] = valid;
    } else {
      obj_val_ctr.valid[index] = valid;
    }
  }
  return;
}

/** FrameCsAttrValue
 * * @Description : This function converts the string value from db to uint8
 * * * @param[in] : Attribute value and val_ctr_st
 * * * @return    : Success or associated error code
 * */
void Kt_Controller::FrameCsAttrValue(string attr_value,
                                     val_ctr_t &obj_val_ctr) {
  for (unsigned int index = 0; index < 7; ++index) {
    if (attr_value[index] >= 48) {
      obj_val_ctr.cs_attr[index] = attr_value[index] - 48;
    } else {
      obj_val_ctr.cs_attr[index] = attr_value[index];
    }
  }
  return;
}

/** ValidateTypeIpAddress
 * * @Description : This function checks whether controller exists with same
 * type and ip address
 * * * @param[in] : Key and Value Struct
 * * * @return    : Success or associated error code
 * */
UpplReturnCode Kt_Controller::ValidateTypeIpAddress(void *key_struct,
                                                    void *val_struct,
                                                    uint32_t data_type,
                                                    uint32_t ctrl_type) {
  UpplReturnCode status = UPPL_RC_SUCCESS;
  val_ctr_t *obj_val_ctr = reinterpret_cast<val_ctr_t*>(val_struct);
  if (obj_val_ctr == NULL) {
    // Not required to validate type and ip
    pfc_log_debug("Not required to validate type and ip");
    return UPPL_RC_SUCCESS;
  }
  key_ctr_t *ctr_key = reinterpret_cast<key_ctr_t*>(key_struct);
  string controller_name = (const char*)ctr_key->controller_name;
  unsigned int type_valid_val =
      PhyUtil::uint8touint(obj_val_ctr->valid[kIdxType]);
  unsigned int ip_valid_val =
      PhyUtil::uint8touint(obj_val_ctr->valid[kIdxIpAddress]);
  if (ip_valid_val == UNC_VF_INVALID) {
    // Not required to validate type and ip
    pfc_log_debug("Not required to validate type and ip");
    return UPPL_RC_SUCCESS;
  }
  uint32_t type = obj_val_ctr->type;
  if (type_valid_val == UNC_VF_INVALID) {
    type = ctrl_type;
  }
  uint32_t ip_add = obj_val_ctr->ip_address.s_addr;
  uint32_t count = UPPL_MAX_REP_CT;
  vector<string> controller_id;
  vector<val_ctr_st_t> vect_val_ctr_st;
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  DBTableSchema kt_controller_dbtableschema;
  void *old_val_struct;
  vector<ODBCMOperator> vect_key_operations;

  PopulateDBSchemaForKtTable(kt_controller_dbtableschema,
                             key_struct,
                             NULL,
                             UNC_OP_READ_SIBLING_BEGIN, 0, 0,
                             vect_key_operations, old_val_struct);
  ODBCM_RC_STATUS read_db_status = physical_layer->get_odbc_manager()->
      GetBulkRows((unc_keytype_datatype_t)data_type, count,
                  kt_controller_dbtableschema,
                  UNC_OP_READ_SIBLING_BEGIN);
  pfc_log_debug("Return of GetBulkRows:%d", read_db_status);
  if (read_db_status != ODBCM_RC_SUCCESS) {
    return status;
  }
  FillControllerValueStructure(kt_controller_dbtableschema,
                               vect_val_ctr_st,
                               count,
                               UNC_OP_READ_SIBLING_BEGIN,
                               controller_id);
  pfc_log_debug("controller_id size: %d",
                (unsigned int)controller_id.size());
  vector <val_ctr_st_t>::iterator vect_val_ctr_iter = vect_val_ctr_st.begin();
  vector <string>::iterator vect_ctr_iter = controller_id.begin();
  for (; vect_val_ctr_iter != vect_val_ctr_st.end(),
  vect_ctr_iter != controller_id.end();
  vect_val_ctr_iter++, vect_ctr_iter++) {
    val_ctr_st_t obj_ctr_st = (*vect_val_ctr_iter);
    string ctr_name_db = (*vect_ctr_iter);
    if (ctr_name_db == controller_name) {
      pfc_log_debug("Ignoring controller %s", ctr_name_db.c_str());
      continue;
    }
    if (obj_ctr_st.controller.cs_row_status != DELETED &&
        obj_ctr_st.controller.type == type &&
        obj_ctr_st.controller.ip_address.s_addr == ip_add) {
      pfc_log_error(
          "Creation of Controller with already existing type and "
          "ip address not allowed");
      status = UPPL_RC_ERR_CFG_SYNTAX;
      break;
    }
  }
  return status;
}

/** ValidateControllerType
 * * @Description : This function checks whether the given controller type
 * is valid
 * * * @param[in] : operation, data_type, ctr_type, val_ctr
 * * * @return    : Success or associated error code
 * */
UpplReturnCode Kt_Controller::ValidateControllerType(
    uint32_t operation,
    uint32_t data_type,
    unc_keytype_ctrtype_t ctr_type,
    UpplReturnCode ctr_type_code,
    val_ctr *val_ctr) {
  UpplReturnCode ret_code = UPPL_RC_SUCCESS;
  pfc_bool_t mandatory = PFC_TRUE;
  // Validate value structure
  unsigned int valid_val = 0;
  // validate type
  valid_val = PhyUtil::uint8touint(val_ctr->valid[kIdxType]);
  IS_VALID_INT_VALUE(CTR_TYPE, val_ctr->type, operation,
                     valid_val, ret_code, mandatory);
  if (ret_code != UPPL_RC_SUCCESS) {
    return UPPL_RC_ERR_CFG_SYNTAX;
  }
  if (operation == UNC_OP_UPDATE && ctr_type_code == UPPL_RC_SUCCESS &&
      valid_val == UNC_VF_VALID && val_ctr->type != ctr_type) {
    pfc_log_error("type cannot be modified");
    return UPPL_RC_ERR_CFG_SYNTAX;
  }
  return UPPL_RC_SUCCESS;
}

/** ValidateControllerVersion
 * * @Description : This function checks whether the given controller version
 * is valid
 * * * @param[in] : operation, data_type, ctr_type, val_ctr
 * * * @return    : Success or associated error code
 * */
UpplReturnCode Kt_Controller::ValidateControllerVersion(
    uint32_t operation,
    uint32_t data_type,
    unc_keytype_ctrtype_t ctr_type,
    UpplReturnCode ctr_type_code,
    val_ctr *val_ctr) {
  UpplReturnCode ret_code = UPPL_RC_SUCCESS;
  pfc_bool_t mandatory = PFC_TRUE;
  unsigned int valid_val = 0;
  // Version validation not required for Unknown Controller
  if ((operation == UNC_OP_UPDATE && ctr_type_code == UPPL_RC_SUCCESS &&
      ctr_type == UNC_CT_PFC) ||
      (operation == UNC_OP_CREATE &&  val_ctr->type == UNC_CT_PFC)) {
    // validate version
    valid_val = PhyUtil::uint8touint(val_ctr->valid[kIdxVersion]);
    string value = reinterpret_cast<char*>(val_ctr->version);
    IS_VALID_STRING_VALUE(CTR_VERSION, value, operation,
                          valid_val, ret_code, mandatory);
    if (ret_code != UPPL_RC_SUCCESS) {
      return UPPL_RC_ERR_CFG_SYNTAX;
    }
  }
  return UPPL_RC_SUCCESS;
}

/** ValidateControllerDescription
 * * @Description : This function checks whether the given controller description
 * is valid
 * * * @param[in] : operation, data_type, ctr_type, val_ctr
 * * * @return    : Success or associated error code
 * */
UpplReturnCode Kt_Controller::ValidateControllerDescription(
    uint32_t operation,
    uint32_t data_type,
    unc_keytype_ctrtype_t ctr_type,
    UpplReturnCode ctr_type_code,
    val_ctr *val_ctr) {
  UpplReturnCode ret_code = UPPL_RC_SUCCESS;
  pfc_bool_t mandatory = PFC_TRUE;
  unsigned int valid_val = 0;
  // validate description
  valid_val = PhyUtil::uint8touint(val_ctr->valid[kIdxDescription]);
  string value = reinterpret_cast<char*>(val_ctr->description);
  IS_VALID_STRING_VALUE(CTR_DESCRIPTION, value, operation,
                        valid_val, ret_code, mandatory);
  if (ret_code != UPPL_RC_SUCCESS) {
    return UPPL_RC_ERR_CFG_SYNTAX;
  }
  return UPPL_RC_SUCCESS;
}

/** ValidateControllerIpAddress
 * * @Description : This function checks whether the given controller ip address
 * is valid
 * * * @param[in] : operation, data_type, ctr_type, val_ctr
 * * * @return    : Success or associated error code
 * */
UpplReturnCode Kt_Controller::ValidateControllerIpAddress(
    uint32_t operation,
    uint32_t data_type,
    unc_keytype_ctrtype_t ctr_type,
    UpplReturnCode ctr_type_code,
    void *key_struct,
    void *val_struct) {
  UpplReturnCode ret_code = UPPL_RC_SUCCESS;
  pfc_bool_t mandatory = PFC_TRUE;
  unsigned int valid_val = 0;
  val_ctr *val_ctr = reinterpret_cast<val_ctr_t*>(val_struct);
  // validate IP address
  valid_val = PhyUtil::uint8touint(val_ctr->valid[kIdxIpAddress]);
  if (operation == UNC_OP_UPDATE && ctr_type_code == UPPL_RC_SUCCESS &&
      ctr_type == UNC_CT_UNKNOWN && valid_val == UNC_VF_VALID) {
    pfc_log_error(
        "Ip address cannot be modified for unknown controller type");
    return UPPL_RC_ERR_CFG_SYNTAX;
  }
  if (operation == UNC_OP_UPDATE && ctr_type_code == UPPL_RC_SUCCESS &&
      valid_val == UNC_VF_VALID) {
    UpplReturnCode validate_status = ValidateTypeIpAddress(key_struct,
                                                           val_struct,
                                                           data_type,
                                                           ctr_type);
    if (validate_status != UPPL_RC_SUCCESS) {
      return UPPL_RC_ERR_CFG_SYNTAX;
    }
  }
  IS_VALID_IPV4_VALUE(CTR_IP_ADDRESS, val_ctr->ip_address, operation,
                      valid_val, ret_code, mandatory);
  if (ret_code != UPPL_RC_SUCCESS) {
    return UPPL_RC_ERR_CFG_SYNTAX;
  }
  return UPPL_RC_SUCCESS;
}

/** ValidateControllerUser
 * * @Description : This function checks whether the given controller user
 * is valid
 * * * @param[in] : operation, data_type, ctr_type, val_ctr
 * * * @return    : Success or associated error code
 * */
UpplReturnCode Kt_Controller::ValidateControllerUser(
    uint32_t operation,
    uint32_t data_type,
    unc_keytype_ctrtype_t ctr_type,
    UpplReturnCode ctr_type_code,
    val_ctr *val_ctr) {
  UpplReturnCode ret_code = UPPL_RC_SUCCESS;
  pfc_bool_t mandatory = PFC_TRUE;
  unsigned int valid_val = 0;
  // validate user
  valid_val = PhyUtil::uint8touint(val_ctr->valid[kIdxUser]);
  if (operation == UNC_OP_UPDATE && ctr_type_code == UPPL_RC_SUCCESS &&
      ctr_type == UNC_CT_UNKNOWN && valid_val == UNC_VF_VALID) {
    pfc_log_error("User cannot be modified for unknown controller type");
    return UPPL_RC_ERR_CFG_SYNTAX;
  }
  string value = reinterpret_cast<char*>(val_ctr->user);
  IS_VALID_STRING_VALUE("user", value, operation,
                        valid_val, ret_code, mandatory);
  if (ret_code != UPPL_RC_SUCCESS) {
    return UPPL_RC_ERR_CFG_SYNTAX;
  }
  return UPPL_RC_SUCCESS;
}

/** ValidateControllerPassword
 * * @Description : This function checks whether the given controller password
 * is valid
 * * * @param[in] : operation, data_type, ctr_type, val_ctr
 * * * @return    : Success or associated error code
 * */
UpplReturnCode Kt_Controller::ValidateControllerPassword(
    uint32_t operation,
    uint32_t data_type,
    unc_keytype_ctrtype_t ctr_type,
    UpplReturnCode ctr_type_code,
    val_ctr *val_ctr) {
  UpplReturnCode ret_code = UPPL_RC_SUCCESS;
  pfc_bool_t mandatory = PFC_TRUE;
  unsigned int valid_val = 0;
  // validate password
  valid_val = PhyUtil::uint8touint(val_ctr->valid[kIdxPassword]);
  if (operation == UNC_OP_UPDATE && ctr_type_code == UPPL_RC_SUCCESS &&
      ctr_type == UNC_CT_UNKNOWN && valid_val == UNC_VF_VALID) {
    pfc_log_error("Password cannot be modified for unknown controller type");
    return UPPL_RC_ERR_CFG_SYNTAX;
  }
  string value = reinterpret_cast<char*>(val_ctr->password);
  IS_VALID_STRING_VALUE(CTR_PASSWORD, value, operation,
                        valid_val, ret_code, mandatory);
  if (ret_code != UPPL_RC_SUCCESS) {
    return UPPL_RC_ERR_CFG_SYNTAX;
  }
  return UPPL_RC_SUCCESS;
}

/** ValidateControllerEnableAudit
 * * @Description : This function checks whether the given controller enable audit
 * is valid
 * * * @param[in] : operation, data_type, ctr_type, val_ctr
 * * * @return    : Success or associated error code
 * */
UpplReturnCode Kt_Controller::ValidateControllerEnableAudit(
    uint32_t operation,
    uint32_t data_type,
    unc_keytype_ctrtype_t ctr_type,
    UpplReturnCode ctr_type_code,
    val_ctr *val_ctr) {
  UpplReturnCode ret_code = UPPL_RC_SUCCESS;
  pfc_bool_t mandatory = PFC_TRUE;
  unsigned int valid_val = 0;
  // validate enable_audit
  valid_val = PhyUtil::uint8touint(val_ctr->valid[kIdxEnableAudit]);
  if (operation == UNC_OP_UPDATE && ctr_type_code == UPPL_RC_SUCCESS &&
      ctr_type == UNC_CT_UNKNOWN && valid_val == UNC_VF_VALID) {
    pfc_log_error(
        "Enable audit cannot be modified for unknown controller type");
    return UPPL_RC_ERR_CFG_SYNTAX;
  }
  IS_VALID_INT_VALUE(CTR_ENABLE_AUDIT, val_ctr->enable_audit, operation,
                     valid_val, ret_code, mandatory);
  if (ret_code != UPPL_RC_SUCCESS) {
    return UPPL_RC_ERR_CFG_SYNTAX;
  }
  return UPPL_RC_SUCCESS;
}

/** SendOperStatusNotification
 * * @Description : This function sends oper status change as notification to
 * north bound
 * * * @param[in] : key_ctr, old oper status and new oper status
 * * * @return    : Success or associated error code
 * */
UpplReturnCode Kt_Controller::SendOperStatusNotification(key_ctr_t ctr_key,
                                                         uint8_t old_oper_st,
                                                         uint8_t new_oper_st) {
  UpplReturnCode status = UPPL_RC_SUCCESS;
  int err = 0;
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  val_ctr_st_t old_val_ctr, new_val_ctr;
  memset(&old_val_ctr, 0, sizeof(val_ctr_st_t));
  memset(&new_val_ctr, 0, sizeof(val_ctr_st_t));
  old_val_ctr.oper_status = old_oper_st;
  old_val_ctr.valid[kIdxOperStatus] = UNC_VF_VALID;
  new_val_ctr.oper_status = new_oper_st;
  new_val_ctr.valid[kIdxOperStatus] = UNC_VF_VALID;
  ServerEvent ser_evt((pfc_ipcevtype_t)UPPL_EVENTS_KT_CONTROLLER, err);
  northbound_event_header rsh = {UNC_OP_UPDATE,
      UNC_DT_STATE,
      UNC_KT_CONTROLLER};
  err = PhyUtil::sessOutNBEventHeader(ser_evt, rsh);
  pfc_log_debug("%s", (IpctUtil::get_string(ctr_key)).c_str());
  pfc_log_debug("%s", (IpctUtil::get_string(new_val_ctr)).c_str());
  pfc_log_debug("%s", (IpctUtil::get_string(old_val_ctr)).c_str());
  err |= ser_evt.addOutput(ctr_key);
  err |= ser_evt.addOutput(new_val_ctr);
  err |= ser_evt.addOutput(old_val_ctr);
  if (err != 0) {
    pfc_log_error(
        "Server Event addOutput failed, return IPC_WRITE_ERROR");
    status = UPPL_RC_ERR_IPC_WRITE_ERROR;
  } else {
    // Call IPC server to post the event
    status = (UpplReturnCode) physical_layer
        ->get_ipc_connection_manager()->SendEvent(&ser_evt);
  }
  return status;
}
