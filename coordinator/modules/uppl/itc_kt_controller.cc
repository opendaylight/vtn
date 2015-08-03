/*
 * Copyright (c) 2012-2015 NEC Corporation
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

#include <uncxx/tclib/tclib_defs.hh>
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
#include "itc_read_request.hh"
#include "phy_util.hh"
#include "capa_module.hh"
#include "ctrlr_capa_defines.hh"

using unc::uppl::PhysicalLayer;
using unc::uppl::ScopedReadWriteLock;
using unc::uppl::ODBCMUtils;
using unc::uppl::ODBCMTableColumns;

#define IPCT_USER "user"  // attribute name in val_ctr is "user"
// and field name in database is "user_name"

/** Constructor
 * @Description : This function initializes member variables
 * and fills the attribute syntax map used for validation
 * @param[in] : None
 * @return    : None
 * */
Kt_Controller::Kt_Controller() {
  // Populate structure to be used for syntax validation
  if (attr_syntax_map_all.find(UNC_KT_CONTROLLER) ==
      attr_syntax_map_all.end()) {
    Fill_Attr_Syntax_Map();
  }
  /* Child instances will be initialized inside
   * member functions whenever required */
  for (int i = 0; i < KT_CONTROLLER_CHILD_COUNT; ++i) {
    child[i] = NULL;
  }
}

/** Destructor
 * @Description : This function frees the child key instances
 * instances for kt_controller
 * @param[in] : None
 * @return    : None
 * */
Kt_Controller::~Kt_Controller() {
  for (int i = 0; i < KT_CONTROLLER_CHILD_COUNT; ++i) {
    if (child[i] != NULL) {
      delete child[i];
      child[i] = NULL;
    }
  }
}

/** GetChildClassPointer
 * @Description : This function creates a new child class instance
 *   class of KtController based on index passed
 * @param[in] : KIndex - Child class index enum
 * @return    : Kt_Base* - The child class pointer
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
 * @Description : This function creates a new row of KT_Controller in
 * candidate controller table.
 * @param[in] : session_id - ipc session id used for TC validation
 * configuration_id - configuration id used for TC validation
 * key_struct - the key for the new kt controller instance
 * value_struct - the values for the new kt controller instance
 * data_type - UNC_DT_* , Create only allowed in candidate
 * sess - ipc server session where the response has to be added
 * @return    : UNC_RC_SUCCESS is returned when the response
 * is added to ipc session successfully.
 * UNC_UPPL_RC_ERR_* is returned when ipc response could not be added to sess.
 * */
UncRespCode Kt_Controller::Create(OdbcmConnectionHandler *db_conn,
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
    val_ctr_t obj_val_ctr = *(reinterpret_cast<val_ctr_t*>(val_struct));
    unc_keytype_ctrtype_t ctr_type = (unc_keytype_ctrtype_t)obj_val_ctr.type;
    if (ctr_type == UNC_CT_UNKNOWN) {
      create_status = ValidateUnknownCtrlrScalability(db_conn, key_struct,
                                    obj_val_ctr.type, data_type);
    } else {
      uint32_t instance_count = 0;
      uint32_t nums = 0;
      const uint8_t *attrs = NULL;
      string version = (const char*)obj_val_ctr.version;
      unc::capa::CapaModule *capa = reinterpret_cast<unc::capa::CapaModule *>(
      pfc::core::Module::getInstance("capa"));
      if (capa == NULL) {
        UPPL_LOG_FATAL("%s:%d: CapaModule is not found", __FUNCTION__,
                                                   __LINE__);
        create_status = UNC_UPPL_RC_ERR_CFG_SEMANTIC;
      } else if (!capa->GetCreateCapability(ctr_type,
                 version, UNC_KT_CONTROLLER, &instance_count,
                         &nums, &attrs)) {
        pfc_log_info("UNC_KT_CONTROLLER is NOT supported for version : %s",
                       obj_val_ctr.version);
        create_status = UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED;
      } else {
        // Check the port attribute
        if (create_status == UNC_RC_SUCCESS)
          create_status = ValKtCtrAttributeSupportCheck(&obj_val_ctr, NULL,
                       attrs, (unc_keytype_operation_t)UNC_OP_CREATE);
      }
      pfc_log_info("instance_count after ctr version capa check %d",
                                                  instance_count);
    }
    if (create_status == UNC_RC_SUCCESS) {
      // Verify the respective driver presence and if driver returns
      // ECONNREFUSED then return driver not present error
      PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
      create_status = physical_layer->get_ipc_connection_manager()->
                                                  GetDriverPresence(ctr_type);
      if (create_status == UNC_RC_ERR_DRIVER_NOT_PRESENT) {
        pfc_log_info("Driver is not present skip Create: %d", create_status);
      } else {
        // Sending the Created Controller  Information to Logical Layer
          create_status = SendUpdatedControllerInfoToUPLL(UNC_DT_CANDIDATE,
                                                  UNC_OP_CREATE,
                                                  UNC_KT_CONTROLLER,
                                                  key_struct,
                                                  val_struct);
          pfc_log_info("Sending the Controller info to UPLL, status is %d",
                 create_status);
      }
    }
  }
  if (create_status == UNC_RC_SUCCESS) {
    create_status = CreateKeyInstance(db_conn, key_struct,
                                      val_struct,
                                      data_type,
                                      UNC_KT_CONTROLLER);
    pfc_log_debug("CreateKeyInstance returned with status %d",
                           create_status);
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
      static_cast<uint32_t>(create_status)};
  int err = PhyUtil::sessOutRespHeader(sess, rsh);
  err |= sess.addOutput((uint32_t)UNC_KT_CONTROLLER);
  err |= sess.addOutput(*obj_key_ctr);
  if (err != 0) {
    pfc_log_error("Server session addOutput failed, so return IPC_WRITE_ERROR");
    create_status = UNC_UPPL_RC_ERR_IPC_WRITE_ERROR;
  } else {
    create_status = UNC_RC_SUCCESS;
  }
  return create_status;
}

/** CreateKeyInstance
 * @Description : This function creates a new row of KT_Controller in
 * candidate controller table.
 * key_struct - the key for the new kt controller instance
 * value_struct - the values for the new kt controller instance
 * data_type - UNC_DT_* , Create only allowed in candidate
 * key_type-UNC_KT_CONTROLLER,value of unc_key_type_t
 * @return    : UNC_RC_SUCCESS is returned when the create is success
 * UNC_UPPL_RC_ERR_* is returned when ipc response could not be added to sess.
 * */
UncRespCode Kt_Controller::CreateKeyInstance(OdbcmConnectionHandler *db_conn,
                                                void* key_struct,
                                                void* val_struct,
                                                uint32_t data_type,
                                                uint32_t key_type) {
  UncRespCode create_status = UNC_RC_SUCCESS;
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  // Structure used to send request to ODBC
  DBTableSchema kt_controller_dbtableschema;
  // Create DBSchema structure for controller_table
  void *old_val_struct;
  vector<ODBCMOperator> vect_key_operations;
  CsRowStatus cs_row_status = NOTAPPLIED;
  PopulateDBSchemaForKtTable(db_conn, kt_controller_dbtableschema,
                             key_struct,
                             val_struct,
                             UNC_OP_CREATE, data_type, 0, 0,
                             vect_key_operations, old_val_struct,
                             cs_row_status, false, PFC_FALSE);
  // Send request to ODBC for controller_table create
  ODBCM_RC_STATUS create_db_status = physical_layer->get_odbc_manager()->\
      CreateOneRow((unc_keytype_datatype_t)data_type,
                   kt_controller_dbtableschema, db_conn);
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
  }
  return create_status;
}

/** Update
 * @Description : This function updates a row of KT_Controller in
 * candidate controller table.
 * @param[in] : session_id - ipc session id used for TC validation
 * configuration_id - configuration id used for TC validation
 * key_struct - the key for the kt controller instance
 * value_struct - the values for the kt controller instance
 * data_type - UNC_DT_* , Update only allowed in candidate
 * sess - ipc server session where the response has to be added
 * @return    : UNC_RC_SUCCESS is returned when the response
 * is added to ipc session successfully.
 * UNC_UPPL_RC_ERR_* is returned when ipc response could not be added to sess.
 * */
UncRespCode Kt_Controller::Update(OdbcmConnectionHandler *db_conn,
                                     uint32_t session_id,
                                     uint32_t configuration_id,
                                     void* key_struct,
                                     void* val_struct,
                                     uint32_t data_type,
                                     ServerSession &sess) {
  UncRespCode update_status = UNC_RC_SUCCESS;
  if ((unc_keytype_datatype_t)data_type != UNC_DT_CANDIDATE) {
    pfc_log_error("Update operation is invoked on unsupported data type %d",
                  data_type);
    update_status = UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED;
  } else {
    // Verify the respective driver presence and if driver returns
    // ECONNREFUSED then return driver not present error
    val_ctr_t obj_val_ctr = *(reinterpret_cast<val_ctr_t*>(val_struct));
    unc_keytype_ctrtype_t ctr_type = UNC_CT_UNKNOWN;
    string version = "";
    UncRespCode read_code = UNC_RC_SUCCESS;

     // Call the ReadInternal fucntion and get the controller type and version
      key_ctr *key = reinterpret_cast<key_ctr_t*>(key_struct);
      string ctr_name = reinterpret_cast<char*>
                                        (key->controller_name);
      vector<void *> vect_key_ctr;
      vect_key_ctr.push_back(key_struct);
      vector<void *> vect_val_ctr;
      read_code = ReadInternal(db_conn, vect_key_ctr, vect_val_ctr,
                             data_type, UNC_OP_READ);
      val_ctr_st_t *db_ctr_val_st = NULL;
      if (read_code == UNC_RC_SUCCESS) {
        db_ctr_val_st = reinterpret_cast<val_ctr_st_t*>(vect_val_ctr[0]);
        if (db_ctr_val_st != NULL) {
          ctr_type = (unc_keytype_ctrtype_t)db_ctr_val_st->controller.type;
          version = reinterpret_cast<const char *>(
                      db_ctr_val_st->controller.version);
        }
        key_ctr_t *ctr_key = reinterpret_cast<key_ctr_t*>
         (vect_key_ctr[0]);
        if (ctr_key != NULL) {
          delete ctr_key;
          ctr_key = NULL;
        }
      // Capa validation
      if (obj_val_ctr.valid[kIdxVersion] == UNC_VF_VALID) {
        version =  reinterpret_cast<const char*>(obj_val_ctr.version);
      }  // else case already store from DB
      pfc_log_debug("version is %s, type = %d", version.c_str(), ctr_type);
      if (ctr_type != UNC_CT_UNKNOWN) {
      uint32_t nums = 0;
      const uint8_t *attrs = NULL;
      unc::capa::CapaModule *capa = reinterpret_cast<unc::capa::CapaModule *>(
          pfc::core::Module::getInstance("capa"));
      if (capa == NULL) {
        UPPL_LOG_FATAL("%s:%d: CapaModule is not found", __FUNCTION__,
                                         __LINE__);
        update_status = UNC_UPPL_RC_ERR_CFG_SEMANTIC;
      } else if (capa->GetUpdateCapability((unc_keytype_ctrtype_t)ctr_type,
                version, UNC_KT_CONTROLLER, &nums, &attrs) == 1) {
        pfc_log_info("Callling ValKtCtrAttributeSupportCheck");
        update_status = ValKtCtrAttributeSupportCheck(&obj_val_ctr,
                       &db_ctr_val_st->controller, attrs,
                       (unc_keytype_operation_t)UNC_OP_UPDATE);
        } else {
          pfc_log_info("UNC_KT_CONTROLLER is NOT supported for version: %s",
                 obj_val_ctr.version);
          update_status = UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED;
       }
       }
    } else {
     update_status =  read_code;
    }
    if (db_ctr_val_st != NULL) {
      delete db_ctr_val_st;
      db_ctr_val_st = NULL;
    }
    if (update_status == UNC_RC_SUCCESS) {
      PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
      update_status = physical_layer->get_ipc_connection_manager()->
                                         GetDriverPresence(ctr_type);
      if (update_status == UNC_RC_ERR_DRIVER_NOT_PRESENT) {
        pfc_log_info("Driver is not present skip update op: %d", update_status);
      }
    }
  }
  if (update_status == UNC_RC_SUCCESS) {
    // Sending the Created Controller  Information to Logical Layer
    update_status = SendUpdatedControllerInfoToUPLL(
        UNC_DT_CANDIDATE,
        UNC_OP_UPDATE,
        UNC_KT_CONTROLLER,
        key_struct,
        val_struct);
    pfc_log_info("Sending the Controller info to UPLL, status is %d",
                 update_status);
  }
  if (update_status == UNC_RC_SUCCESS) {
    update_status = UpdateKeyInstance(db_conn, key_struct,
                                  val_struct, data_type,
                                  UNC_KT_CONTROLLER, false);
    pfc_log_debug("UpdateKeyInstance returned with status %d",
                                                     update_status);
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
      static_cast<uint32_t>(update_status)};
  int err = PhyUtil::sessOutRespHeader(sess, rsh);
  err |= sess.addOutput((uint32_t)UNC_KT_CONTROLLER);
  err |= sess.addOutput(*obj_key_ctr);
  if (err != 0) {
    pfc_log_error("Server session addOutput failed, so return IPC_WRITE_ERROR");
    update_status = UNC_UPPL_RC_ERR_IPC_WRITE_ERROR;
  } else {
    update_status = UNC_RC_SUCCESS;
  }
  return update_status;
}

/** UpdateKeyInstance
 * @Description : This function updates a row of KT_Controller in
 * candidate controller table.
 * @param[in] :
 * key_struct - the key for the new kt controller instance
 * value_struct - the values for the new kt controller instance
 * data_type - UNC_DT_* , update only allowed in candidate
 * key_type-UNC_KT_CONTROLLER,value of unc_key_type_t
 * @return    : UNC_RC_SUCCESS is returned when the update
 * is done successfully.
 * UNC_UPPL_RC_ERR_* is returned when the update is error
 * */
UncRespCode Kt_Controller::UpdateKeyInstance(OdbcmConnectionHandler *db_conn,
                                                void* key_struct,
                                                void* val_struct,
                                                uint32_t data_type,
                                                uint32_t key_type,
                                                pfc_bool_t commit_ver_flag) {
  UncRespCode update_status = UNC_RC_SUCCESS;
  // Structure used to send request to ODBC
  DBTableSchema kt_controller_dbtableschema;
  vector<ODBCMOperator> vect_key_operations;
  pfc_bool_t IsInternal = false;
  void *old_val_struct;
  // Create DBSchema structure for controller_table
  if (commit_ver_flag != true) {
    CsRowStatus cs_row_status = NOTAPPLIED;
    PopulateDBSchemaForKtTable(db_conn, kt_controller_dbtableschema,
                             key_struct,
                             val_struct,
                             UNC_OP_UPDATE, data_type, 0, 0,
                             vect_key_operations, old_val_struct,
                             cs_row_status, false, PFC_FALSE);
  } else {
    // commit version update
    PopulateDBSchemaForCommitVersion(db_conn, kt_controller_dbtableschema,
                             key_struct,
                             val_struct,
                             UNC_OP_UPDATE);
    IsInternal = true;
  }
  if (!((kt_controller_dbtableschema.get_row_list()).empty())) {
    // Send request to ODBC for controller_table update
    PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
    ODBCM_RC_STATUS update_db_status = physical_layer->get_odbc_manager()-> \
        UpdateOneRow((unc_keytype_datatype_t)data_type,
                     kt_controller_dbtableschema, db_conn, IsInternal);
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
      pfc_log_info("Update of a controller in data_type(%d) is success",
                   data_type);
    }
  } else {
    pfc_log_debug("Nothing to be updated, so return");
  }
  return update_status;
}

/**Delete
 * @Description : This function deletes a row of KT_Controller in
 * candidate controller table.
 * @param[in] : session_id - ipc session id used for TC validation
 * configuration_id - configuration id used for TC validation
 * key_struct - the key for the kt controller instance
 * data_type - UNC_DT_* , delete only allowed in candidate
 * sess - ipc server session where the response has to be added
 * @return    : UNC_RC_SUCCESS is returned when the response
 * is added to ipc session successfully.
 * UNC_UPPL_RC_ERR_* is returned when ipc response could not be added to sess.
 * */
UncRespCode Kt_Controller::Delete(OdbcmConnectionHandler *db_conn,
                                     uint32_t session_id,
                                     uint32_t configuration_id,
                                     void* key_struct,
                                     uint32_t data_type,
                                     ServerSession &sess) {
  UncRespCode delete_status = UNC_RC_SUCCESS;
  key_ctr_t *obj_key_ctr = reinterpret_cast<key_ctr_t*>(key_struct);
  string controller_name = (const char*)obj_key_ctr->controller_name;
  // 1. Check whether the controller is being imported
  PhysicalCore *physical_core = PhysicalLayer::get_instance()->
      get_physical_core();
  InternalTransactionCoordinator *itc_trans  =
      physical_core->get_internal_transaction_coordinator();
  if (itc_trans->IsControllerInImport(controller_name) == PFC_TRUE) {
    pfc_log_info("Import is in progress for controller %s, Delete not allowed",
                 controller_name.c_str());
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
    err |= sess.addOutput((uint32_t)UNC_KT_CONTROLLER);
    err |= sess.addOutput(*obj_key_ctr);
    if (err != 0) {
      pfc_log_info(
          "Server session addOutput failed, so return IPC_WRITE_ERROR");
      delete_status = UNC_UPPL_RC_ERR_IPC_WRITE_ERROR;
    } else {
      delete_status = UNC_RC_SUCCESS;
    }
    return delete_status;
  }
  Kt_Boundary boundary_class;
  // 2. Check whether any boundary is referring controller
  pfc_bool_t is_bdry_referred = PFC_FALSE;
  is_bdry_referred = boundary_class.IsBoundaryReferred(
      db_conn, UNC_KT_CONTROLLER, key_struct, data_type);
  if (is_bdry_referred == PFC_TRUE) {
    // Boundary is referring controller
    pfc_log_error(
        "Controller is referred in Boundary, "
        "so delete is not allowed");
    delete_status = UNC_UPPL_RC_ERR_CFG_SEMANTIC;
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
    err |= sess.addOutput((uint32_t)UNC_KT_CONTROLLER);
    err |= sess.addOutput(*obj_key_ctr);
    if (err != 0) {
      pfc_log_info(
          "Server session addOutput failed, so return IPC_WRITE_ERROR");
      delete_status = UNC_UPPL_RC_ERR_IPC_WRITE_ERROR;
    } else {
      delete_status = UNC_RC_SUCCESS;  // the response is sent successfully
    }
    return delete_status;
  }
  // 3. Check whether CONTROLLER is being referred in Logical layer
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
    physical_response_header rsh = {session_id,
            configuration_id,
            UNC_OP_DELETE,
            0,
            0,
            0,
            data_type,
            static_cast<uint32_t>(validate_status)};
    int err = PhyUtil::sessOutRespHeader(sess, rsh);
    err |= sess.addOutput((uint32_t)UNC_KT_CONTROLLER);
    err |= sess.addOutput(*obj_key_ctr);
    if (err != 0) {
      pfc_log_info(
          "Server session addOutput failed, so return IPC_WRITE_ERROR");
      validate_status = UNC_UPPL_RC_ERR_IPC_WRITE_ERROR;
    } else {
      validate_status = UNC_RC_SUCCESS;
    }
    return validate_status;
  }
  // 3.1 reference check in candidate configuration
  delete_status = SendSemanticRequestToUPLL(key_struct,
                                            data_type);
  if (delete_status != UNC_RC_SUCCESS) {
    // log error and send error response
    pfc_log_error("Controller is referred in Logical, "
        "so delete is not allowed");
  } else if (delete_status == UNC_RC_SUCCESS &&
                       config_mode == TC_CONFIG_REAL) {
    // 3.2 Check whether CONTROLLER is referred in Running DB in Logical layer
    delete_status = SendSemanticRequestToUPLL(key_struct,
                                              UNC_DT_RUNNING);
    if (delete_status != UNC_RC_SUCCESS) {
      pfc_log_error("Controller is referred in Running DB in Logical, "
         "so delete is not allowed");
    }
  } else {
    pfc_log_debug("Controller is not referred in CANDIDATE DB in logical");
  }
  if (delete_status == UNC_RC_SUCCESS) {
    // Sending the Deleted Controller  Information to Logical Layer
    delete_status = SendUpdatedControllerInfoToUPLL(
        UNC_DT_CANDIDATE,
        UNC_OP_DELETE,
        UNC_KT_CONTROLLER,
        key_struct,
        0);
    pfc_log_info("Sending the Controller info to UPLL, status is %d",
                 delete_status);
  }
  //  4. Delete child classes and then delete controller
  //  In candidate db, only domain will be available
  if (delete_status == UNC_RC_SUCCESS) {
    int child_class = KIdxDomain;
    // Filling key_struct corresponding to that key type
    void *child_key_struct = getChildKeyStruct(child_class,
                                               controller_name);
    child[child_class] = GetChildClassPointer(
        (KtControllerChildClass)child_class);
    if (child[child_class] != NULL) {
      UncRespCode ch_delete_status =
          child[child_class]->DeleteKeyInstance(db_conn,
                                                child_key_struct,
                                                data_type,
                                                UNC_KT_CTR_DOMAIN);
      delete child[child_class];
      child[child_class] = NULL;
      FreeChildKeyStruct(child_key_struct, child_class);
      if (ch_delete_status == UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE) {
        pfc_log_debug("Child not available for controller");
      }
      if (ch_delete_status != UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE &&
          ch_delete_status != UNC_RC_SUCCESS) {
        // child delete failed, so return error
        pfc_log_error("Delete failed for child %d with error %d",
                      child_class, delete_status);
        delete_status = UNC_UPPL_RC_ERR_CFG_SEMANTIC;
      }
    } else {
      // Free key struct
      FreeChildKeyStruct(child_key_struct, child_class);
    }
    // Delete the controller now
    if (delete_status == UNC_RC_SUCCESS) {
      delete_status = DeleteKeyInstance(db_conn, key_struct, data_type,
                                        UNC_KT_CONTROLLER);
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
  err |= sess.addOutput((uint32_t)UNC_KT_CONTROLLER);
  err |= sess.addOutput(*obj_key_ctr);
  if (err != 0) {
    pfc_log_info("Server session addOutput failed, so return IPC_WRITE_ERROR");
    delete_status = UNC_UPPL_RC_ERR_IPC_WRITE_ERROR;
  } else {
    delete_status = UNC_RC_SUCCESS;
  }
  return delete_status;
}

/**DeleteKeyInstance
 * @Description : This function deletes a row of KT_Controller in
 * candidate controller table.
 * @param[in] :
 * key_struct - the key for the new kt controller instance
 * data_type - UNC_DT_* , delete only allowed in candidate
 * key_type-UNC_DT_CONTROLLER,value of unc_keytype_t
 * @return    : UNC_RC_SUCCESS is returned when the delete
 * is done successfully.
 * UNC_UPPL_RC_ERR_* is returned when the delete is error
 * */
UncRespCode Kt_Controller::DeleteKeyInstance(OdbcmConnectionHandler *db_conn,
                                                void* key_struct,
                                                uint32_t data_type,
                                                uint32_t key_type) {
  UncRespCode delete_status = UNC_RC_SUCCESS;
  key_ctr_t *obj_key_ctr= reinterpret_cast<key_ctr_t*>(key_struct);
  string controller_name = (const char*)obj_key_ctr->controller_name;
  // Structure used to send request to ODBC
  DBTableSchema kt_controller_dbtableschema;

  // Construct Primary key list
  vector<string> vect_prim_keys;
  vect_prim_keys.push_back(CTR_NAME_STR);

  // Construct TableAttrSchema structure
  // TableAttrSchema holds table_name, primary key, attr_name
  vector<TableAttrSchema> vect_table_attr_schema;
  list< vector<TableAttrSchema> > row_list;

  // Controller_name
  PhyUtil::FillDbSchema(unc::uppl::CTR_NAME, controller_name,
                        controller_name.length(), DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);

  // Send request to ODBC for controller_table delete
  kt_controller_dbtableschema.set_table_name(unc::uppl::CTR_TABLE);
  kt_controller_dbtableschema.set_primary_keys(vect_prim_keys);
  row_list.push_back(vect_table_attr_schema);
  kt_controller_dbtableschema.set_row_list(row_list);

  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  ODBCM_RC_STATUS delete_db_status = physical_layer->get_odbc_manager()-> \
      DeleteOneRow((unc_keytype_datatype_t)data_type,
                   kt_controller_dbtableschema, db_conn);
  if (delete_db_status != ODBCM_RC_SUCCESS) {
    if (delete_db_status == ODBCM_RC_CONNECTION_ERROR) {
      // log fatal error to log daemon
      UPPL_LOG_FATAL("DB connection not available or cannot access DB");
      delete_status = UNC_UPPL_RC_ERR_DB_ACCESS;
    } else if (delete_db_status == ODBCM_RC_ROW_NOT_EXISTS) {
      delete_status = UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE;
    } else {
      // log error to log daemon
      delete_status = UNC_UPPL_RC_ERR_DB_DELETE;
    }
  } else {
    pfc_log_info("Delete of a controller in data_type(%d) is success",
                 data_type);
  }
  return delete_status;
}

/** ReadInternal
 * @Description : This function reads the given  instance of KT_Controller
 * @param[in] : session_id - ipc session id used for TC validation
 * key_struct - the key for the kt controller instance
 * value_struct - the value for the kt controller instance
 * data_type - UNC_DT_* , read allowed in candidate/running/startup/state
 * operation_type-UNC_OP_*,type of operation
 * @return    : UNC_RC_SUCCESS is returned when the response
 * is added to ipc session successfully.
 * UNC_UPPL_RC_ERR_* is returned when ipc response could not be added to sess.
 * */
UncRespCode Kt_Controller::ReadInternal(OdbcmConnectionHandler *db_conn,
                                           vector<void *> &ctr_key,
                                           vector<void *> &ctr_val,
                                           uint32_t data_type,
                                           uint32_t operation_type) {
  if (operation_type != UNC_OP_READ && operation_type != UNC_OP_READ_SIBLING &&
      operation_type != UNC_OP_READ_SIBLING_BEGIN) {
    pfc_log_trace("This function not allowed for read next/bulk/count");
    return UNC_UPPL_RC_ERR_OPERATION_NOT_SUPPORTED;
  }
  pfc_log_debug("Processing Kt_Controller::ReadInternal");
  uint32_t max_rep_ct = 1;
  if (operation_type != UNC_OP_READ) {
    max_rep_ct = UPPL_MAX_REP_CT;
  }
  void *key_struct = ctr_key[0];
  void *val_struct = NULL;
  val_ctr_st_t st_ctr_val;
  if ((!ctr_val.empty()) && (ctr_val[0] != NULL)) {
    memcpy(&st_ctr_val, (reinterpret_cast<val_ctr_st_t *> (ctr_val[0])),
           sizeof(val_ctr_st_t));
    val_struct = reinterpret_cast<void *>(&st_ctr_val.controller);
  }
  UncRespCode read_status = UNC_RC_SUCCESS;
  bool firsttime = true;
  do {
  // Get read response from database
    vector<string> vect_controller_id;
    vector<val_ctr_st_t> vect_val_ctr_st;
    read_status = ReadCtrValFromDB(db_conn, key_struct,
                                                val_struct,
                                                data_type,
                                                operation_type,
                                                max_rep_ct,
                                                vect_val_ctr_st,
                                                vect_controller_id);
    if (firsttime) {
      pfc_log_trace(
          "Clearing key_val and val_struct vectors for the first time");
      ctr_key.clear();
      ctr_val.clear();
      firsttime = false;
    }
    pfc_log_debug("ReadCtrValFromDB returned %d with response size %"
               PFC_PFMT_SIZE_T,
               read_status,
               vect_val_ctr_st.size());
    if (read_status == UNC_RC_SUCCESS) {
      pfc_log_debug("ReadCtrValFromDB returned %d with response size %"
                   PFC_PFMT_SIZE_T, read_status, vect_val_ctr_st.size());
      for (unsigned int iIndex = 0 ; iIndex < vect_controller_id.size();
          ++iIndex) {
        key_ctr_t *key_ctr = new key_ctr_t;
        memset(key_ctr->controller_name, 0, sizeof(key_ctr->controller_name));
        memcpy(key_ctr->controller_name,
             vect_controller_id[iIndex].c_str(),
             vect_controller_id[iIndex].length()+1);
        ctr_key.push_back(reinterpret_cast<void *>(key_ctr));
        val_ctr_st_t *val_ctr = new val_ctr_st_t(vect_val_ctr_st[iIndex]);
        ctr_val.push_back(reinterpret_cast<void *>(val_ctr));
      }
    } else if ((read_status == UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE &&
               ctr_val.size() != 0)) {
      read_status = UNC_RC_SUCCESS;
    }
    if ((vect_val_ctr_st.size() == UPPL_MAX_REP_CT) &&
                     (operation_type != UNC_OP_READ)) {
      pfc_log_debug("Op:%d, key.size:%" PFC_PFMT_SIZE_T"fetch_next_set",
                    operation_type, ctr_key.size());
      key_struct = reinterpret_cast<void *>(ctr_key[ctr_key.size() - 1]);
      operation_type = UNC_OP_READ_SIBLING;
      continue;
    } else {
      break;
    }
  } while (true);
  return read_status;
}

/**ReadBulk
 * @Description : This function reads bulk rows of KT_Controller in
 *  controller table of specified data type.
 *  Order of ReadBulk response
 *  val_ctr -> val_ctr_domain -> val_logical_port ->
 *  val_logical_member_port -> val_switch ->  val_port ->
 *  val_link -> val_boundary
 * @param[in] :
 * key_struct - the key for the kt controller instance
 * data_type - UNC_DT_* , read allowed in candidate/running/startup/state
 * max_rep_ct - specifies number of rows to be returned
 * child_index-index indicating the children of Controller
 * parent_call - indicates whether parent has called this readbulk
 * is_read_next - indicates whether this function is invoked from readnext
 * @return    : UNC_RC_SUCCESS is returned when the response
 * is added to ipc session successfully.
 * UNC_UPPL_RC_ERR_* is returned when ipc response could not be added to sess.
 * */
UncRespCode Kt_Controller::ReadBulk(OdbcmConnectionHandler *db_conn,
                                       void* key_struct,
                                       uint32_t data_type,
                                       uint32_t &max_rep_ct,
                                       int child_index,
                                       pfc_bool_t parent_call,
                                       pfc_bool_t is_read_next,
                                       ReadRequest *read_req) {
  pfc_log_info("Processing ReadBulk of Kt_Controller");
  UncRespCode read_status = UNC_RC_SUCCESS;
  key_ctr_t* obj_key_ctr= reinterpret_cast<key_ctr_t*>(key_struct);

  vector<val_ctr_st_t> vect_val_ctr;
  if (data_type != UNC_DT_CANDIDATE && data_type != UNC_DT_RUNNING &&
      data_type != UNC_DT_STATE && data_type != UNC_DT_STARTUP) {
    pfc_log_info("ReadBulk operation is not allowed in %d data type",
                  data_type);
    read_status = UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED;
    return read_status;
  }
  pfc_bool_t ctr_exists = PFC_FALSE;
  if (max_rep_ct == 0) {
    pfc_log_debug("max_rep_ct is 0");
    return UNC_RC_SUCCESS;
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
    UncRespCode key_exist_status = IsKeyExists(
        db_conn,
        (unc_keytype_datatype_t)data_type,
        vect_ctr_key_value);
    if (key_exist_status == UNC_RC_SUCCESS) {
      ctr_exists = PFC_TRUE;
    }
  }
  if (child_index > -1 && child_index < KIdxLink) {
    ctr_exists = PFC_TRUE;
  }
  void *val_struct = NULL;
  // Read the controller values based on given key structure
  read_status = ReadBulkInternal(db_conn,
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
  if (read_status == UNC_RC_SUCCESS ||
      ctr_exists == PFC_TRUE) {
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
        read_req->AddToBuffer(obj_key_buffer);
        if ((unc_keytype_datatype_t)data_type == UNC_DT_STATE) {
          val_ctr_st_t *val_buffer = new val_ctr_st_t(*vect_iter);
          BulkReadBuffer obj_value_buffer = {
              UNC_KT_CONTROLLER, IS_STATE_VALUE,
              reinterpret_cast<void *>(val_buffer)
          };
          read_req->AddToBuffer(obj_value_buffer);
        } else {
          val_ctr_t *val_buffer = new val_ctr_t((*vect_iter).controller);
          BulkReadBuffer obj_value_buffer = {
              UNC_KT_CONTROLLER, IS_VALUE,
              reinterpret_cast<void *>(val_buffer)
          };
          read_req->AddToBuffer(obj_value_buffer);
        }
        BulkReadBuffer obj_sep_buffer = {
            UNC_KT_CONTROLLER, IS_SEPARATOR, NULL
        };
        read_req->AddToBuffer(obj_sep_buffer);
        --max_rep_ct;
        if (max_rep_ct == 0) {
          pfc_log_debug("Controller - max_rep_ct reached zero...");
          return UNC_RC_SUCCESS;
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
        UncRespCode ch_read_status = child[kIdx]->ReadBulk(
            db_conn, child_key_struct,
            data_type,
            max_rep_ct,
            -1,
            true,
            is_read_next,
            read_req);
        pfc_log_debug("ReadBulk response from child %d is %d",
                      kIdx, ch_read_status);
        delete child[kIdx];
        child[kIdx] = NULL;
        FreeChildKeyStruct(child_key_struct, kIdx);
        if (max_rep_ct == 0) {
          pfc_log_debug("max_rep_ct reached zero, so returning");
          return UNC_RC_SUCCESS;
        }
      }
      // reset child index
      child_index = -1;
    }
  } else if (read_status == UNC_UPPL_RC_ERR_DB_ACCESS) {
    pfc_log_debug("KtController ReadBulk - Returning DB Access Error");
    return read_status;
  }
  if (max_rep_ct > 0 && parent_call == false) {
    pfc_log_debug("max_rep_ct is %d and parent_call is %d, calling parent",
                  max_rep_ct, parent_call);
    // Filling key_struct corresponding to tht key type
    Kt_Root nextKin;
    key_root_t nextkin_key_struct;
    read_status = nextKin.ReadBulk(
        db_conn, reinterpret_cast<void *>(&nextkin_key_struct),
        data_type,
        max_rep_ct,
        0,
        false,
        is_read_next,
        read_req);
    pfc_log_debug("read status from next kin Kt_Root is %d",
                  read_status);
    return UNC_RC_SUCCESS;
  }
  pfc_log_debug("KT_Controller - Reached end of table");
  pfc_log_debug("read_status is %d", read_status);
  if (read_status == UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE) {
    read_status = UNC_RC_SUCCESS;
  }
  return UNC_RC_SUCCESS;
}

/**ReadBulkInternal
 * @Description : This function reads bulk rows of KT_Controller in
 *  controller table of specified data type.
 * @param[in] :
 * key_struct - the key for the kt controller instance
 * data_type-UNC_DT_*
 * val_struct - the value struct for kt_controller instance
 * max_rep_ct - specifies number of rows to be returned
 * vect_val_ctr - indicates the fetched values from db of val_ctr type
 * vect_ctr_id - indicates the fetched contoller names from db
 * @return    : UNC_RC_SUCCESS is returned when the response
 * is added to ipc session successfully.
 * UNC_UPPL_RC_ERR_* is returned when ipc response could not be added to sess.
 * */
UncRespCode Kt_Controller::ReadBulkInternal(
    OdbcmConnectionHandler *db_conn,
    void* key_struct,
    void* val_struct,
    uint32_t data_type,
    uint32_t max_rep_ct,
    vector<val_ctr_st_t> &vect_val_ctr,
    vector<string> &vect_ctr_id) {
  if (max_rep_ct <= 0) {
    return UNC_RC_SUCCESS;
  }
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  UncRespCode read_status = UNC_RC_SUCCESS;
  ODBCM_RC_STATUS read_db_status = ODBCM_RC_SUCCESS;
  DBTableSchema kt_controller_dbtableschema;
  // Populate DBSchema for controller_table
  void *old_val_struct;
  vector<ODBCMOperator> vect_key_operations;
  CsRowStatus cs_row_status = NOTAPPLIED;
  PopulateDBSchemaForKtTable(db_conn, kt_controller_dbtableschema,
                             key_struct,
                             val_struct,
                             UNC_OP_READ_BULK, data_type, 0, 0,
                             vect_key_operations, old_val_struct,
                             cs_row_status, false, PFC_FALSE);
  // Read rows from DB
  read_db_status = physical_layer->get_odbc_manager()-> \
      GetBulkRows((unc_keytype_datatype_t)data_type, max_rep_ct,
                  kt_controller_dbtableschema,
                  (unc_keytype_operation_t)UNC_OP_READ_BULK, db_conn);
  if (read_db_status == ODBCM_RC_RECORD_NOT_FOUND) {
    pfc_log_debug("No record to read");
    read_status = UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE;
    return read_status;
  } else if (read_db_status == ODBCM_RC_CONNECTION_ERROR) {
    read_status = UNC_UPPL_RC_ERR_DB_ACCESS;
    pfc_log_error("ReadBulk operation has failed with %d", read_db_status);
    return read_status;
  } else if (read_db_status != ODBCM_RC_SUCCESS) {
    read_status = UNC_UPPL_RC_ERR_DB_GET;
    pfc_log_error("ReadBulk operation has failed with %d", read_db_status);
    return read_status;
  }

  // From the values received from DB, populate val_ctr structure
  FillControllerValueStructure(db_conn, kt_controller_dbtableschema,
                               vect_val_ctr,
                               max_rep_ct,
                               UNC_OP_READ_BULK,
                               vect_ctr_id);
  return read_status;
}

/** PerformSyntaxValidation
 * @Description : This function performs syntax validation for
 *  UNC_KT_CONTROLLER
 * @param[in]
 * key_struct - the key for the kt controller instance
 * value_struct - the value for the kt controller instance
 * data_type - UNC_DT_*,type of database
 * operation_type - UNC_OP*,type of operation
 * @return    : UNC_RC_SUCCESS is returned when the validation is successful
 * UNC_UPPL_RC_ERR_* is returned when validation is failure
 * */
UncRespCode Kt_Controller::PerformSyntaxValidation(
    OdbcmConnectionHandler *db_conn,
    void* key_struct,
    void* val_struct,
    uint32_t operation,
    uint32_t data_type) {
  UncRespCode ret_code = UNC_RC_SUCCESS;
  pfc_bool_t mandatory = PFC_TRUE;
  map<string, Kt_Class_Attr_Syntax> attr_syntax_map =
      attr_syntax_map_all[UNC_KT_CONTROLLER];
  // Validate key structure
  key_ctr *key = reinterpret_cast<key_ctr_t*>(key_struct);
  string value = reinterpret_cast<char*>(key->controller_name);
  IS_VALID_STRING_KEY(CTR_NAME_STR, value, operation, ret_code, mandatory);
  if (ret_code != UNC_RC_SUCCESS) {
    return UNC_UPPL_RC_ERR_CFG_SYNTAX;
  }

  unc_keytype_ctrtype_t ctr_type = UNC_CT_UNKNOWN;
  UncRespCode ctr_type_code = UNC_RC_SUCCESS;
  if (operation == UNC_OP_UPDATE) {
    ctr_type_code = PhyUtil::get_controller_type(
        db_conn, value,
        ctr_type,
        (unc_keytype_datatype_t)data_type);
    pfc_log_debug("Controller type - return code %d, value %s",
                  ctr_type_code, value.c_str());
  } else if (operation == UNC_OP_CREATE) {
    val_ctr *val_ctr = reinterpret_cast<val_ctr_t*>(val_struct);
    ctr_type = (unc_keytype_ctrtype_t)val_ctr->type;
  }

  // Validate value structure
  if (val_struct != NULL) {
    val_ctr *val_ctr = reinterpret_cast<val_ctr_t*>(val_struct);
    ret_code = ValidateControllerType(
        db_conn, operation,
        data_type,
        ctr_type,
        ctr_type_code,
        val_ctr);
    if (ret_code != UNC_RC_SUCCESS) {
      return UNC_UPPL_RC_ERR_CFG_SYNTAX;
    }
    ret_code = ValidateControllerVersion(
        db_conn, operation,
        data_type,
        ctr_type,
        ctr_type_code,
        val_ctr);
    if (ret_code != UNC_RC_SUCCESS) {
      return UNC_UPPL_RC_ERR_CFG_SYNTAX;
    }
    ret_code = ValidateControllerDescription(
        db_conn, operation,
        data_type,
        ctr_type,
        ctr_type_code,
        val_ctr);
    if (ret_code != UNC_RC_SUCCESS) {
      return UNC_UPPL_RC_ERR_CFG_SYNTAX;
    }
    ret_code = ValidateControllerIpAddress(
        db_conn, operation,
        data_type,
        ctr_type,
        ctr_type_code,
        key_struct,
        val_struct);
    if (ret_code != UNC_RC_SUCCESS) {
      return UNC_UPPL_RC_ERR_CFG_SYNTAX;
    }
    ret_code = ValidateControllerUser(
        db_conn, operation,
        data_type,
        ctr_type,
        ctr_type_code,
        val_ctr);
    if (ret_code != UNC_RC_SUCCESS) {
      return UNC_UPPL_RC_ERR_CFG_SYNTAX;
    }
    ret_code = ValidateControllerPassword(
        db_conn, operation,
        data_type,
        ctr_type,
        ctr_type_code,
        val_ctr);
    if (ret_code != UNC_RC_SUCCESS) {
      return UNC_UPPL_RC_ERR_CFG_SYNTAX;
    }
    ret_code = ValidateControllerEnableAudit(
        db_conn, operation,
        data_type,
        ctr_type,
        ctr_type_code,
        val_ctr);
    if (ret_code != UNC_RC_SUCCESS) {
      return UNC_UPPL_RC_ERR_CFG_SYNTAX;
    }
    ret_code = ValidateControllerPort(
        db_conn, operation,
        data_type,
        ctr_type,
        ctr_type_code,
        val_ctr);
    if (ret_code != UNC_RC_SUCCESS) {
      return UNC_UPPL_RC_ERR_CFG_SYNTAX;
    }
  }
  return ret_code;
}

/** ValidateUnKnownCtrlrScalability
 * @Description : This function checks scalability for unknown controller
 * @param[in] : type - specifies the controller type
 * key_struct - contains UNC_KT_CONTROLLER key structure
 * data_type - UNC_DT_*, unknown controler scalability number and data_type
 * @return    : UNC_RC_SUCCESS if unknown controler scalability number
 *  is within range or UNC_UPPL_RC_ERR_* if not
 * */
UncRespCode Kt_Controller::ValidateUnknownCtrlrScalability(
    OdbcmConnectionHandler *db_conn,
    void *key_struct,
    uint8_t type,
    uint32_t data_type) {
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  ODBCM_RC_STATUS response_status = ODBCM_RC_SUCCESS;
  uint32_t count = 0;
  DBTableSchema kt_controller_dbtableschema;
  // Construct Primary key list
  vector<string> vect_prim_keys;
  vect_prim_keys.push_back(CTR_TYPE_STR);
  std::vector<ODBCMOperator> vect_filter_operators;
  vect_filter_operators.push_back(unc::uppl::EQUAL);

  string value = PhyUtil::uint8tostr(type);
  pfc_log_debug("type: %s", value.c_str());

  // Construct TableAttrSchema structure
  // TableAttrSchema holds table_name, primary key, attr_name
  vector<TableAttrSchema> vect_table_attr_schema;
  list< vector<TableAttrSchema> > row_list;
  PhyUtil::FillDbSchema(unc::uppl::CTR_TYPE, value,
                        value.length(), DATATYPE_UINT16,
                        vect_table_attr_schema);
  kt_controller_dbtableschema.set_table_name(unc::uppl::CTR_TABLE);
  kt_controller_dbtableschema.set_primary_keys(vect_prim_keys);
  row_list.push_back(vect_table_attr_schema);
  kt_controller_dbtableschema.set_row_list(row_list);

  // Send request to ODBC for controller_table
  response_status = physical_layer->get_odbc_manager()-> \
      GetSiblingCount((unc_keytype_datatype_t)data_type,
                      kt_controller_dbtableschema, count,
                      vect_filter_operators, db_conn);

  if (response_status == ODBCM_RC_CONNECTION_ERROR) {
    // log error to log daemon
    pfc_log_error("DB connection not available or cannot access DB");
    return UNC_UPPL_RC_ERR_DB_ACCESS;
  } else if (response_status == ODBCM_RC_SUCCESS ||
      response_status == ODBCM_RC_RECORD_NOT_FOUND) {
    uint32_t unknown_ctrl_count =
        physical_layer->get_physical_core()->getUnknownControllerCount();

    if ((uint32_t)count >= unknown_ctrl_count) {
      pfc_log_debug("Count from DB: %d", count);
      pfc_log_debug("Count from Physical Core: %d", unknown_ctrl_count);
      pfc_log_error("scalability range exceeded for unknown kt_controller");
      return UNC_UPPL_RC_ERR_EXCEEDS_RESOURCE_LIMIT;
    }
  } else {
    pfc_log_error("Unable to get sibling count from DB, error is %d",
                  response_status);
    return UNC_UPPL_RC_ERR_DB_GET;
  }
  return UNC_RC_SUCCESS;
}

/** PerformSemanticValidation
 * @Description : This function performs semantic validation
 * for UNC_KT_CONTROLLER
 * @param[in] : key_struct - specifies key instance of KT_Controller
 * value_struct - specifies value of KT_CONTROLLER,value of unc_key_type_t
 * operation - UNC_OP*,type of operation
 * data_type - UNC_DT*,type of database
 * @return    : UNC_RC_SUCCESS if semantic valition is successful
 * or UNC_UPPL_RC_ERR_* if failed
 * */
UncRespCode Kt_Controller::PerformSemanticValidation(
    OdbcmConnectionHandler *db_conn,
    void* key_struct,
    void* val_struct,
    uint32_t operation,
    uint32_t data_type) {
  UncRespCode status = UNC_RC_SUCCESS;

  // Check whether the given instance of controller exists in DB
  key_ctr_t *obj_key_ctr = reinterpret_cast<key_ctr_t*>(key_struct);
  if (val_struct == NULL && operation == UNC_OP_CREATE) {
    pfc_log_debug("Val struct is mandatory for controller create request");
    return UNC_UPPL_RC_ERR_CFG_SYNTAX;
  }
  val_ctr_t *obj_val_ctr = reinterpret_cast<val_ctr_t*>(val_struct);
  string controller_name = (const char*)obj_key_ctr->controller_name;
  vector<string> ctr_vect_key_value;
  ctr_vect_key_value.push_back(controller_name);
  UncRespCode key_status = IsKeyExists(db_conn,
                                          (unc_keytype_datatype_t)data_type,
                                          ctr_vect_key_value);
  // In case of create operation, key should not exist
  if (operation == UNC_OP_CREATE) {
    if (key_status == UNC_RC_SUCCESS) {
      pfc_log_error("Key exists,CREATE not allowed");
      status = UNC_UPPL_RC_ERR_INSTANCE_EXISTS;
    } else if (key_status == UNC_UPPL_RC_ERR_DB_ACCESS) {
      pfc_log_error("DB Access failure");
      status = key_status;
    } else if (key_status == UNC_UPPL_RC_ERR_DB_GET) {
      if (obj_val_ctr->valid[kIdxType] != UNC_VF_VALID) {
        pfc_log_debug("Type must be valid for controller create request");
        return UNC_UPPL_RC_ERR_CFG_SYNTAX;
      }
      pfc_log_debug("Key does not exist. Validate Ip Address/ Type");
      // Check whether any controller with same type and ip address exists
      unc_keytype_ctrtype_t ctr_type = UNC_CT_UNKNOWN;
      status = ValidateTypeIpAddress(db_conn, key_struct,
                                     val_struct,
                                     data_type, ctr_type);
      if (status == UNC_RC_SUCCESS) {
        pfc_log_debug("Validating Type and Ip Address in Running Db");
        unc_keytype_ctrtype_t ctr_type = UNC_CT_UNKNOWN;
        status = ValidateTypeIpAddress(db_conn, key_struct,
                                       val_struct,
                                       UNC_DT_RUNNING, ctr_type);
      }
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
    }
  }
  return status;
}

/** SendSemanticRequestToUPLL
 * @Description : This functions calls IPC to check whether UNC_KT_CONTROLLER
 *  is being referred in Logical
 * @param[in] : key_ctr - specifies key instance of KT_CONTROLLER
 * data_type-UNC_DT_*,type of database
 * @return    : UNC_RC_SUCCESS if controller is not referred
 * or UNC_UPPL_RC_ERR_* if controller is referred in logical
 * */
UncRespCode Kt_Controller::SendSemanticRequestToUPLL(void* key_struct,
                                                        uint32_t data_type) {
  // Incase for UNC_KT_CONTROLLER delete, check whether any referenced object
  // Is present in Logical Layer, If yes DELETE should not be allowed
  pfc_log_debug("SendSemanticRequestToUPLL of KTController");
  UncRespCode status = UNC_RC_SUCCESS;
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  status = physical_layer->get_ipc_connection_manager()->
      get_ipc_client_logical_handler()->CheckInUseInLogical(UNC_KT_CONTROLLER,
                                                            key_struct,
                                                            data_type);
  if (status != UNC_RC_SUCCESS) {
    // log error and send error response
    pfc_log_error("Controller is being referred in Logical");
  }
  return status;
}

/** HandleDriverEvents
 * @Description : This function processes the notification received from
 * driver
 * @param[in] : key_struct - specifies the key instance of KT_CONTROLLER
 * oper_type-UNC_OP_*,type of operation
 * data_type-UNC_DT_*,type ,of database
 * is_events_done-flag to indicate whether event is processed
 * old_value_struct - old value of KT_CONTROLLER
 * new_value_struct - new value of KT_CONTROLLER
 * @return    : UNC_RC_SUCCESS if events are handled successfully or
 * UNC_UPPL_RC_ERR*
 * */
UncRespCode Kt_Controller::HandleDriverEvents(
    OdbcmConnectionHandler *db_conn,
    void* key_struct,
    uint32_t oper_type,
    uint32_t data_type,
    void* old_val_struct,
    void* new_val_struct,
    pfc_bool_t is_events_done) {
  key_ctr *obj_key_ctr= reinterpret_cast<key_ctr_t*>(key_struct);
  val_ctr_st *obj_old_val_ctr = reinterpret_cast<val_ctr_st*>(old_val_struct);
  val_ctr_st *obj_new_val_ctr = reinterpret_cast<val_ctr_st*>(new_val_struct);
  UncRespCode status = UNC_RC_SUCCESS;
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
  string act_version = (const char*)obj_new_val_ctr->actual_version;
  if (valid_val == UNC_VF_VALID || valid_val == UNC_VF_VALID_NO_VALUE) {
    unc_keytype_validflag_t fvalid = UNC_VF_VALID;
    if (valid_val == UNC_VF_VALID_NO_VALUE) {
      fvalid = UNC_VF_INVALID;
      act_version = "";
    }
    status = SetActualVersion(db_conn, key_struct, act_version,
                              UNC_DT_RUNNING, fvalid);
    if (status != UNC_RC_SUCCESS) {
      // log error
      pfc_log_error("act_version update operation failed for running");
    }
    status = SetActualVersion(db_conn, key_struct,
                              act_version, UNC_DT_CANDIDATE, fvalid);
    if (status != UNC_RC_SUCCESS) {
      // log error
      pfc_log_error("act_version update operation failed for candidate");
    }
  }
  valid_val = PhyUtil::uint8touint(obj_new_val_ctr->valid[kIdxActualId]);
  string actual_id = (const char*)obj_new_val_ctr->actual_id;
  if (valid_val == UNC_VF_VALID || valid_val == UNC_VF_VALID_NO_VALUE) {
    unc_keytype_validflag_t fvalid = UNC_VF_VALID;
    if (valid_val == UNC_VF_VALID_NO_VALUE) {
      fvalid = UNC_VF_INVALID;
      actual_id = "";
    }
    status = SetActualControllerId(db_conn, key_struct,
                           actual_id, UNC_DT_RUNNING, fvalid);
    if (status != UNC_RC_SUCCESS) {
      // log error
      pfc_log_error("actual_id update operation failed for running");
    }
    status = SetActualControllerId(db_conn, key_struct,
                             actual_id, UNC_DT_CANDIDATE, fvalid);
    if (status != UNC_RC_SUCCESS) {
      // log error
      pfc_log_error("actual_id update operation failed for candidate");
    }
  }
    // Read old_oper_status from DB
  UncRespCode read_status =  UNC_RC_SUCCESS;
  ReadWriteLock *rwlock = NULL;
  PhysicalLayer::ctr_oprn_mutex_.lock();
  PHY_OPERSTATUS_LOCK(controller_name, read_status, rwlock, true);
  if (read_status != UNC_RC_SUCCESS) {
    PhysicalLayer::ctr_oprn_mutex_.unlock();
    return read_status;
  }
  // Check for duplicate controller id and raise an alarm if duplicate exists
  status = CheckDuplicateControllerId(actual_id, controller_name, db_conn);
  if (status != UNC_RC_SUCCESS) {
    // log error
    pfc_log_error("checking duplicate controller id operation failed");
  }
  uint8_t oper_status_db = 0;
  read_status = GetOperStatus(db_conn, data_type,
                                             key_struct,
                                             oper_status_db);
  if (read_status != UNC_RC_SUCCESS) {
    PHY_OPERSTATUS_LOCK(controller_name, read_status, rwlock, false);
    PhysicalLayer::ctr_oprn_mutex_.unlock();
    // No need to check read_status since return value doesn't matter
    return read_status;
  }
  uint8_t new_oper_status = UPPL_CONTROLLER_OPER_DOWN;
  // CONTROLLER_OPER_UP from driver
  // Its same as enum UPPL_CONTROLLER_OPER_UP
  if (obj_new_val_ctr->oper_status == UPPL_CONTROLLER_OPER_UP &&
      obj_new_val_ctr->valid[kIdxOperStatus] == UNC_VF_VALID) {
    if (is_events_done == false) {
      new_oper_status = UPPL_CONTROLLER_OPER_WAITING_AUDIT;
    } else {
      new_oper_status = UPPL_CONTROLLER_OPER_UP;
    }
    map<string, CtrOprnStatus> :: iterator it =
            PhysicalLayer::ctr_oprn_status_.find(controller_name);
    if (it != PhysicalLayer::ctr_oprn_status_.end()) {
      CtrOprnStatus ctr_oprn =  it->second;
      if (ctr_oprn.IsIPChanged != false) {
        ctr_oprn.IsIPChanged =  false;
        ClearImportAndStateEntries(db_conn, controller_name);
      }
      ctr_oprn.ActualOperStatus =  new_oper_status;
      PhysicalLayer::ctr_oprn_status_[controller_name] = ctr_oprn;
    }
  }
  if (obj_new_val_ctr->oper_status == UPPL_CONTROLLER_OPER_DOWN &&
      obj_new_val_ctr->valid[kIdxOperStatus] == UNC_VF_VALID) {
    // Set the EventStartReceived flag as false to block events
    map<string, CtrOprnStatus> :: iterator it =
    PhysicalLayer::ctr_oprn_status_.find(controller_name);
    if (it != PhysicalLayer::ctr_oprn_status_.end()) {
      CtrOprnStatus ctr_oprn =  it->second;
      ctr_oprn.EventsStartReceived =  false;
      ctr_oprn.ActualOperStatus =  UPPL_CONTROLLER_OPER_DOWN;
      if (ctr_oprn.IsIPChanged == true)
        ClearImportAndStateEntries(db_conn, controller_name);
      PhysicalLayer::ctr_oprn_status_[controller_name] = ctr_oprn;
    }
    pfc_log_info("Sending Controller Disconnect alarm");
    UncRespCode alarms_status=
        PhysicalLayer::get_instance()->get_physical_core()->
        SendControllerDisconnectAlarm(controller_name);
    pfc_log_info("Alarm status: %d", alarms_status);
    // If controller is in audit list perform required action
    IPCConnectionManager *ipc_mgr = PhysicalLayer::get_instance()->
        get_ipc_connection_manager();
    pfc_bool_t is_controller_in_audit = ipc_mgr->
        IsControllerInAudit(controller_name);
    if (is_controller_in_audit == PFC_TRUE) {
      PHY_OPERSTATUS_LOCK(controller_name, read_status, rwlock, false);
      PhysicalLayer::ctr_oprn_mutex_.unlock();
      pfc_log_debug("Calling MergeAuditDbToRunning");
      {
      // To cancel the already running timer in Audit
      PHY_TIMER_LOCK();
      UncRespCode cancel_ret = ipc_mgr->CancelTimer(controller_name);
      if (cancel_ret != UNC_RC_SUCCESS) {
        pfc_log_info("Failure in cancelling timer for controller %s",
                     controller_name.c_str());
      }
      }
      AuditRequest audit_req;
      UncRespCode merge_auditdb =
          audit_req.MergeAuditDbToRunning(db_conn, controller_name);
      if (merge_auditdb != UNC_RC_SUCCESS) {
        pfc_log_info("Merge of audit and running db failed");
      }
      PhysicalLayer::ctr_oprn_mutex_.lock();
      PHY_OPERSTATUS_LOCK(controller_name, read_status, rwlock, true);
      if (read_status != UNC_RC_SUCCESS) {
        PhysicalLayer::ctr_oprn_mutex_.unlock();
        return read_status;
      }
    }
    // Check for Ip Address and remove state db if no ip address is found
    UncRespCode state_status = CheckIpAndClearStateDB(db_conn, key_struct);
    pfc_log_debug("Ip and state db processing status : %d", state_status);
  }
  pfc_log_info("New Oper_status to be set is: %d", new_oper_status);
  if (new_oper_status == UPPL_CONTROLLER_OPER_WAITING_AUDIT &&
      (oper_status_db == UPPL_CONTROLLER_OPER_AUDITING ||
      oper_status_db == UPPL_CONTROLLER_OPER_UP)) {
    pfc_log_info("Audit is going on:so keep orignal state of controller");
    PHY_OPERSTATUS_LOCK(controller_name, read_status, rwlock, false);
    PhysicalLayer::ctr_oprn_mutex_.unlock();
    // No need to check read_status since return value doesn't matter
    return UNC_RC_SUCCESS;
  }
  if (new_oper_status != oper_status_db &&
      obj_new_val_ctr->valid[kIdxOperStatus] == UNC_VF_VALID) {
    if (is_events_done == true) {
      // To avoid changing back to audit_waiting
      status = HandleOperStatus(db_conn, data_type,
                                key_struct, obj_new_val_ctr, true);
    } else {
      status = HandleOperStatus(db_conn, data_type,
                                key_struct, obj_new_val_ctr, false);
    }
    pfc_log_debug("HandleOperStatus return %d", status);
    // Send notification to Northbound
    if (new_oper_status == UPPL_CONTROLLER_OPER_UP ||
        new_oper_status == UPPL_CONTROLLER_OPER_DOWN) {
      status = SendOperStatusNotification(*obj_key_ctr,
                                          oper_status_db,
                                          new_oper_status);
    }
  }
  PHY_OPERSTATUS_LOCK(controller_name, read_status, rwlock, false);
  PhysicalLayer::ctr_oprn_mutex_.unlock();
  // No need to check read_status since return value doesn't matter
  return status;
}

/** ClearImportAndStateEntries
 * @Description : This function clears import and state entries for
 * given controller
 * @param[in] : OdbcmConnectionHandler, controller_name
 * @return    : UNC_RC_SUCCESS if successfully cleared or
 * UNC_UPPL_RC_ERR*
 */
UncRespCode Kt_Controller::ClearImportAndStateEntries(
           OdbcmConnectionHandler *db_conn, string controller_name) {
  UncRespCode status = UNC_RC_SUCCESS;
  pfc_log_info("Removing State entries for controller %s",
             controller_name.c_str());
  ODBCM_RC_STATUS clear_status =
    PhysicalLayer::get_instance()->get_odbc_manager()->
    ClearOneInstance(UNC_DT_STATE, controller_name, db_conn);
  if (clear_status != ODBCM_RC_SUCCESS && clear_status
                            != ODBCM_RC_SUCCESS_WITH_INFO) {
    pfc_log_info("State DB clearing failed");
    status = UNC_UPPL_RC_ERR_CLEAR_DB;
  }
  clear_status = PhysicalLayer::get_instance()->get_odbc_manager()->
    ClearOneInstance(UNC_DT_IMPORT, controller_name, db_conn);
  if (clear_status != ODBCM_RC_SUCCESS && clear_status
                            != ODBCM_RC_SUCCESS_WITH_INFO) {
    pfc_log_info("Import DB clearing failed");
    status = UNC_UPPL_RC_ERR_CLEAR_DB;
  }
  return status; 
}
/** HandleDriverAlarms
 * @Description : This function processes the alarm notification
 * received from driver
 * @param[in] : alarm type - contains type to indicate PATH_FAULT alarm
 * operation - contains UNC_OP_CREATE or UNC_OP_DELETE\
 * key_struct - indicates the key instance of KT_CONTROLLER
 * value_struct - indicates the alarm values structure
 * data_type-UNC_DT_*,type of database
 * alarm_type-type of alarm raised
 * oper_type-UNC_OP_*,type of operation
 * @return    : UNC_RC_SUCCESS if alarm is handled successfully or
 * UNC_UPPL_RC_ERR*
 * */
UncRespCode Kt_Controller::HandleDriverAlarms(
    OdbcmConnectionHandler *db_conn,
    uint32_t data_type,
    uint32_t alarm_type,
    uint32_t oper_type,
    void* key_struct,
    void* val_struct) {
  UncRespCode status = UNC_RC_SUCCESS;
  // Following alarms are sent for kt_controller
  if (alarm_type == UNC_PHYS_PATH_FAULT) {
    pfc_log_info("PHYS_PATH_FAULT alarm received from driver");
    pfc_log_info("with oper type as %d", oper_type);
    PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
    key_ctr_t *obj_key_ctr= reinterpret_cast<key_ctr_t*>(key_struct);
    string controller_name = (const char*)obj_key_ctr->controller_name;
    string temp_domain_name = "";
    string domain_name = "";
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
    string phy_port_id = "";
    vector<string> ingress_logical_port_id, egress_logical_port_id;
    // Get the associated logical_port_id for ingress switch
    logical_port_class.GetAllPortId(db_conn, data_type,
                                    controller_name,
                                    ingress_switch_id,
                                    temp_domain_name,
                                    ingress_logical_port_id,
                                    true);
    memset(&obj_path_fault_alarm, 0, sizeof(obj_path_fault_alarm));
    if (!ingress_logical_port_id.empty()) {
      memcpy(obj_path_fault_alarm.ingress_logical_port,
             ingress_logical_port_id[0].c_str(),
             (ingress_logical_port_id[0].length())+1);
      pfc_log_debug("Ingress Logical Port: %s",
                    obj_path_fault_alarm.ingress_logical_port);
      domain_name = temp_domain_name;
    } else {
      pfc_log_info("No logical port associated with ingress switch id");
    }
    // Get the associated logical_port_id for egress switch
    logical_port_class.GetAllPortId(db_conn, data_type,
                                    controller_name,
                                    egress_switch_id,
                                    temp_domain_name,
                                    egress_logical_port_id,
                                    true);
    if (!egress_logical_port_id.empty()) {
      memcpy(obj_path_fault_alarm.egress_logical_port,
             egress_logical_port_id[0].c_str(),
             egress_logical_port_id[0].length()+1);
      pfc_log_debug("Egress Logical Port: %s",
                    obj_path_fault_alarm.egress_logical_port);
      domain_name = temp_domain_name;
    } else {
      pfc_log_info("No logical port associated with egress switch id");
    }
    ingress_logical_port_id.clear();
    egress_logical_port_id.clear();
    pfc_log_debug("Get LogPortIds for corresponding ingress switch PhyPorts");
    // Get all logical ports associated with ingress switch
    logical_port_class.GetAllPortId(db_conn, data_type,
                                    controller_name,
                                    ingress_switch_id,
                                    temp_domain_name,
                                    ingress_logical_port_id,
                                    false);
    pfc_log_debug("Get LogPortIds for corresponding egress switch PhyPorts");
    // Get all logical ports associated with egress switch
    logical_port_class.GetAllPortId(db_conn, data_type,
                                    controller_name,
                                    egress_switch_id,
                                    temp_domain_name,
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
        UNC_KT_CTR_DOMAIN,
        UPPL_ALARMS_PHYS_PATH_FAULT};
    err = PhyUtil::sessOutNBAlarmHeader(ser_evt, rsh);
    key_ctr_domain_t ctr_dmn_key;
    memset(&ctr_dmn_key, '\0', sizeof(ctr_dmn_key));
    memcpy(ctr_dmn_key.ctr_key.controller_name,
           (const char *)controller_name.c_str(), controller_name.length()+1);
    if (!domain_name.empty()) {
      memcpy(ctr_dmn_key.domain_name,
             (const char *)domain_name.c_str(), domain_name.length()+1);
    }
    err |= ser_evt.addOutput(ctr_dmn_key);
    pfc_log_info("%s", (IpctUtil::get_string(ctr_dmn_key)).c_str());
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
      status = UNC_UPPL_RC_ERR_IPC_WRITE_ERROR;
    } else {
      // Call IPC server to post the event
      status = (UncRespCode) physical_layer->get_ipc_connection_manager()->
          SendEvent(&ser_evt, controller_name, UPPL_ALARMS_PHYS_PATH_FAULT);
    }
  } else {
    pfc_log_warn("%d alarm received for controller is ignored", alarm_type);
  }
  return status;
}

/** IsKeyExists
 * @Description : This function checks whether the controller_id exists in DB
 * @param[in] : data type - UNC_DT_*,type of database
 * key value - Contains controller_id
 * @return    : UNC_RC_SUCCESS or UNC_UPPL_RC_ERR* based on operation type
 * */
UncRespCode Kt_Controller::IsKeyExists(
    OdbcmConnectionHandler *db_conn,
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
  // Structure used to send request to ODBC
  DBTableSchema kt_controller_dbtableschema;

  // Construct Primary key list
  vector<string> vect_prim_keys;
  vect_prim_keys.push_back(CTR_NAME_STR);

  // Construct TableAttrSchema structure
  // TableAttrSchema holds table_name, primary key, attr_name
  vector<TableAttrSchema> vect_table_attr_schema;
  list< vector<TableAttrSchema> > row_list;

  // Controller_name
  PhyUtil::FillDbSchema(unc::uppl::CTR_NAME, controller_name,
                        controller_name.length(), DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);

  kt_controller_dbtableschema.set_table_name(unc::uppl::CTR_TABLE);
  kt_controller_dbtableschema.set_primary_keys(vect_prim_keys);
  row_list.push_back(vect_table_attr_schema);
  kt_controller_dbtableschema.set_row_list(row_list);

  // Send request to ODBC for controlle_common_table
  ODBCM_RC_STATUS check_db_status = physical_layer->get_odbc_manager()->\
      IsRowExists(data_type, kt_controller_dbtableschema, db_conn);
  if (check_db_status == ODBCM_RC_CONNECTION_ERROR) {
    // log error to log daemon
    pfc_log_error("DB connection not available or cannot access DB");
    check_status = UNC_UPPL_RC_ERR_DB_ACCESS;
  } else if (check_db_status == ODBCM_RC_ROW_EXISTS) {
    pfc_log_debug("DB returned success for Row exists");
    pfc_log_debug("Checking .db_return_status_ %d with %d",
                  kt_controller_dbtableschema.db_return_status_,
                  static_cast<int>(DELETED));
    if (kt_controller_dbtableschema.db_return_status_ != DELETED) {
      pfc_log_debug("DB schema returned success for Row exists");
    } else {
      pfc_log_debug("DB schema Returned failure for IsRowExists");
      check_status = UNC_UPPL_RC_ERR_DB_GET;
    }
  } else {
    pfc_log_debug("DB Returned failure for IsRowExists");
    check_status = UNC_UPPL_RC_ERR_DB_GET;
  }
  pfc_log_debug("check_status = %d", check_status);
  return check_status;
}

/** HandleOperStatus
 * @Description : This function performs the required actions when oper status
 * changes
 * @param[in] : key_struct-void* to CTR key structure
 * value_struct-void* to CTR val structure
 * data_type-UNC_DT_*,type of database
 * @return    : UNC_RC_SUCCESS or UNC_UPPL_RC_ERR*
 * */
UncRespCode Kt_Controller::HandleOperStatus(
    OdbcmConnectionHandler *db_conn,
    uint32_t data_type,
    void *key_struct,
    void *value_struct) {
  UncRespCode return_code = UNC_RC_SUCCESS;
  return_code = HandleOperStatus(db_conn, data_type,
                                 key_struct,
                                 value_struct,
                                 false);
  return return_code;
}

/** HandleOperStatus
 * @Description : This function performs the required actions when oper status
 * changes
 * @param[in] : key_struct - identifies the controller key instance
 * value_struct - identifies the controller value structure
 * data_type-UNC_DT_*,type of database
 * bIsInternal-based on this flag,oper status is updated 
 * @return    : UNC_RC_SUCCESS or UNC_UPPL_RC_ERR*
 * */
UncRespCode Kt_Controller::HandleOperStatus(
    OdbcmConnectionHandler *db_conn,
    uint32_t data_type,
    void *key_struct,
    void *value_struct,
    bool bIsInternal) {
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  UncRespCode return_code = UNC_RC_SUCCESS;
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
  UncRespCode read_status = GetOperStatus(db_conn, data_type,
                                             key_struct,
                                             oper_status_db);
  pfc_log_debug("Get OperStatus return: %d", read_status);
  // Update oper_status in controller_table
  return_code = SetOperStatus(db_conn, data_type, key_struct, oper_value);
  if (return_code != UNC_RC_SUCCESS) {
    pfc_log_info("oper_status update operation failed %d", return_code);
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
    /*pfc_log_info("Sending Controller Disconnect alarm");
    UncRespCode alarms_status=
        physical_layer->get_physical_core()->
        SendControllerDisconnectAlarm(controller_name);
    pfc_log_info("Alarm status: %d", alarms_status);*/
    vector<OperStatusHolder> ref_oper_status;
    ADD_CTRL_OPER_STATUS(controller_name,
                         obj_val_ctr->oper_status,
                         ref_oper_status);
    return_code = NotifyOperStatus(db_conn, UNC_DT_RUNNING,
                                   key_struct,
                                   value_struct,
                                   ref_oper_status);
    ClearOperStatusHolder(ref_oper_status);
  } else if (obj_val_ctr->oper_status == UPPL_CONTROLLER_OPER_UP) {
    // Send clearance for CONTROLLER_DISCONNECT alarm
    // Send CONTROLLER_CONNECT alarm
    // Call PhysicalCore's SendControllerConnectAlarm()
    UncRespCode alarms_status=
        physical_layer->get_physical_core()->
        SendControllerConnectAlarm(controller_name);
    pfc_log_info("Alarm status for controller %s is %d",
                  controller_name.c_str(), alarms_status);
  }
  return UNC_RC_SUCCESS;
}

/** NotifyOperStatus
 * @Description : This function performs the notifies other associated
 * key types when oper status changes
 * @param[in] : data_type-UNC_DT_*,type of database
 * key_struct-void* to CTR key structure
 * value_struct-void* to CTR val structure
 * @return    : UNC_RC_SUCCESS or UNC_UPPL_RC_ERR*
 * */
UncRespCode Kt_Controller::NotifyOperStatus(
    OdbcmConnectionHandler *db_conn,
    uint32_t data_type,
    void *key_struct,
    void *value_struct,
    vector<OperStatusHolder> &ref_oper_status) {
  UncRespCode return_code = UNC_RC_SUCCESS;
  key_ctr *key = reinterpret_cast<key_ctr_t*>(key_struct);
  string controller_name = reinterpret_cast<char*>(key->controller_name);

  // Call the associated key types to notify controller's
  // oper_status change
  for (unsigned int index = 0 ; index < KT_CONTROLLER_OPER_STATUS_REF ;
      ++index) {
    void *ref_key_struct = NULL;
    Kt_Base *class_pointer = GetClassPointerAndKey(
        (KtControllerOperStatusRef)index, controller_name, ref_key_struct);
    return_code = class_pointer->HandleOperStatus(db_conn, data_type,
                                                  ref_key_struct,
                                                  NULL);
    pfc_log_info("HandleOperStatus for %d child class %d",
                 index, return_code);
    delete class_pointer;
    FreeKeyStruct(ref_key_struct, index);
  }
  // Notify Boundary to change oper status
  Kt_Boundary boundary;
  val_boundary_t obj_val_boundary1;
  memset(&obj_val_boundary1, 0, sizeof(obj_val_boundary1));
  memcpy(obj_val_boundary1.controller_name1,
         controller_name.c_str(), controller_name.length()+1);
  obj_val_boundary1.valid[kIdxBoundaryControllerName1] = UNC_VF_VALID;
  return_code = boundary.HandleOperStatus(
      db_conn,
      UNC_DT_RUNNING, NULL,
      reinterpret_cast<void *> (&obj_val_boundary1),
      ref_oper_status);
  pfc_log_info("HandleOperStatus for boundary C1 class %d", return_code);

  val_boundary_t obj_val_boundary2;
  memset(&obj_val_boundary2, 0, sizeof(obj_val_boundary2));
  memcpy(obj_val_boundary2.controller_name2,
         controller_name.c_str(), controller_name.length()+1);
  obj_val_boundary2.valid[kIdxBoundaryControllerName2] = UNC_VF_VALID;
  return_code = boundary.HandleOperStatus(
      db_conn,
      UNC_DT_RUNNING, NULL,
      reinterpret_cast<void *> (&obj_val_boundary2),
      ref_oper_status);
  pfc_log_info("HandleOperStatus for boundary C2 class %d", return_code);

  return return_code;
}

/** PopulateDBSchemaForKtTable
 * @Description : This function populates the DBAttrSchema to be used to send
 *                  request to ODBC
 * @param[in] : kt_controller_dbtableschema-DBTableSchema instance associated
 *              with Controller
 * key_struct-void* to CTR key structure
 * val_struct-void* to CTR value structure
 * operation_type-UNC_OP_*,type of operation
 * option1,option2-UNC_OPT1/OPT2_*,additional info for read operations
 * vect_key_operations-instance of vector<ODBCMOperator>
 * old_value_struct-void* to old value strcuture of CTR
 * row_status-CsRowStatus value
 * is_filtering-flag to indicate whether filter  option is enabled or not
 * is_state-flag to indicate whether data type is DT_STATE
 * @return    : void
 * */
void Kt_Controller::PopulateDBSchemaForKtTable(
    OdbcmConnectionHandler *db_conn,
    DBTableSchema &kt_controller_dbtableschema,
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

  // Construct TableAttrSchema structure
  // TableAttrSchema holds table_name, primary key, attr_name
  vector<TableAttrSchema> vect_table_attr_schema;
  list < vector<TableAttrSchema> > row_list;

  key_ctr_t *obj_key_ctr = reinterpret_cast<key_ctr_t*>(key_struct);
  val_ctr_t *obj_val_ctr = reinterpret_cast<val_ctr_t*>(val_struct);

  stringstream valid;
  // Controller_name
  string controller_name = (const char*)obj_key_ctr->controller_name;
  if (operation_type == UNC_OP_READ_SIBLING_BEGIN ||
      operation_type == UNC_OP_READ_SIBLING_COUNT) {
    // Ignore controller_name key value
    controller_name = "";
  }
  pfc_log_debug("ctr_name:%s,oper:%d", controller_name.c_str(),
                                                  operation_type);
  PhyUtil::FillDbSchema(unc::uppl::CTR_NAME, controller_name,
                        controller_name.length(), DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);

  val_ctr_st_t val_ctr_valid_st;
  memset(&val_ctr_valid_st, 0, sizeof(val_ctr_st_t));
  if (operation_type == UNC_OP_UPDATE) {
    // get valid array for update req
    pfc_log_debug("Get Valid value from DB");
    GetCtrValidFlag(db_conn, key_struct, val_ctr_valid_st, data_type);
  }

  uint16_t valid_val = 0, prev_db_val = 0;
  string value = "";
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
  PhyUtil::FillDbSchema(unc::uppl::CTR_TYPE, CTR_TYPE_STR, value,
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
  PhyUtil::FillDbSchema(unc::uppl::CTR_VERSION, CTR_VERSION_STR, value,
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
  PhyUtil::FillDbSchema(unc::uppl::CTR_DESCRIPTION, CTR_DESCRIPTION_STR, value,
                        value.length(), DATATYPE_UINT8_ARRAY_128,
                        operation_type, valid_val, prev_db_val,
                        vect_table_attr_schema, vect_prim_keys, valid);
  // Ip_address
  char *ip_value = new char[16];
  memset(ip_value, '\0', 16);
  if (obj_val_ctr != NULL) {
    valid_val = PhyUtil::uint8touint(obj_val_ctr->valid[kIdxIpAddress]);
    inet_ntop(AF_INET, &obj_val_ctr->ip_address, ip_value, INET_ADDRSTRLEN);
    prev_db_val =
        PhyUtil::uint8touint(
            val_ctr_valid_st.controller.valid[kIdxIpAddress]);
  } else {
    valid_val = UPPL_NO_VAL_STRUCT;
  }
  PhyUtil::FillDbSchema(unc::uppl::CTR_IP_ADDRESS, CTR_IP_ADDRESS_STR, ip_value,
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
  PhyUtil::FillDbSchema(unc::uppl::CTR_USER_NAME, CTR_USER_NAME_STR, value,
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
  PhyUtil::FillDbSchema(unc::uppl::CTR_PASSWORD, CTR_PASSWORD_STR, value,
                        value.length(), DATATYPE_UINT8_ARRAY_257,
                        operation_type, valid_val, prev_db_val,
                        vect_table_attr_schema, vect_prim_keys, valid);
  value.clear();
  // enable_audit
  if (obj_val_ctr != NULL) {
    if (operation_type == UNC_OP_CREATE) {
      valid_val = UNC_VF_VALID;
    } else {
      valid_val = PhyUtil::uint8touint(obj_val_ctr->valid[kIdxEnableAudit]);
    }
    value = PhyUtil::uint8tostr(obj_val_ctr->enable_audit);
    prev_db_val =
        PhyUtil::uint8touint(
            val_ctr_valid_st.controller.valid[kIdxEnableAudit]);
  } else {
    valid_val = UPPL_NO_VAL_STRUCT;
  }
  PhyUtil::FillDbSchema(unc::uppl::CTR_ENABLE_AUDIT, CTR_ENABLE_AUDIT_STR,
                        value, value.length(), DATATYPE_UINT16,
                        operation_type, valid_val, prev_db_val,
                        vect_table_attr_schema, vect_prim_keys, valid);
  value.clear();

  // actual_version
  valid_val = UNC_VF_INVALID;
  if (operation_type == UNC_OP_UPDATE) {
    prev_db_val =
        PhyUtil::uint8touint(
            val_ctr_valid_st.valid[kIdxActualVersion]);
  } else {
    prev_db_val = 0;
  }
  PhyUtil::FillDbSchema(unc::uppl::CTR_ACTUAL_VERSION, CTR_ACTUAL_VERSION_STR,
                        value, value.length(), DATATYPE_UINT8_ARRAY_32,
                        operation_type, valid_val, prev_db_val,
                        vect_table_attr_schema, vect_prim_keys, valid);
  value.clear();
  // oper_status
  if (operation_type == UNC_OP_UPDATE) {
    valid_val = UNC_VF_INVALID;
    prev_db_val =
        PhyUtil::uint8touint(
            val_ctr_valid_st.valid[kIdxOperStatus]);
  } else {
    valid_val = UPPL_NO_VAL_STRUCT;
    prev_db_val = 0;
  }
  if (operation_type == UNC_OP_CREATE &&
      (ctr_type == UNC_CT_UNKNOWN)) {
    // Oper_status for Unknown will be always up
    stringstream oper_value;
    oper_value << UPPL_CONTROLLER_OPER_UP;
    value = oper_value.str();
    valid_val = UNC_VF_VALID;
  }
  PhyUtil::FillDbSchema(unc::uppl::CTR_OPER_STATUS, CTR_OPER_STATUS_STR, value,
                        value.length(), DATATYPE_UINT16,
                        operation_type, valid_val, prev_db_val,
                        vect_table_attr_schema, vect_prim_keys, valid);
  value.clear();
  if (operation_type >= UNC_OP_READ) {
    // actual_id
    PhyUtil::FillDbSchema(unc::uppl::CTR_ACTUAL_CONTROLLERID, value,
                        value.length(), DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);

    value.clear();
    // valid_actual_id
    PhyUtil::FillDbSchema(unc::uppl::CTR_VALID_ACTUAL_CONTROLLERID, value,
                        value.length(), DATATYPE_UINT8_ARRAY_1,
                        vect_table_attr_schema);
    value.clear();
  }
  // Port
  if (obj_val_ctr != NULL) {
    valid_val = PhyUtil::uint8touint(obj_val_ctr->valid[kIdxcPort]);
    value = PhyUtil::uint16tostr(obj_val_ctr->port);
    prev_db_val =
        PhyUtil::uint8touint(val_ctr_valid_st.controller.valid[kIdxcPort]);
  } else {
    valid_val = UPPL_NO_VAL_STRUCT;
  }
  PhyUtil::FillDbSchema(unc::uppl::CTR_PORT, CTR_PORT_STR, value,
                        value.length(), DATATYPE_UINT16,
                        operation_type, valid_val, prev_db_val,
                        vect_table_attr_schema, vect_prim_keys, valid);
  value.clear();
  // valid
  valid_val = UPPL_NO_VAL_STRUCT;
  stringstream dummy_valid;
  PhyUtil::FillDbSchema(unc::uppl::CTR_VALID, CTR_VALID_STR, valid.str(),
                        valid.str().length(), DATATYPE_UINT8_ARRAY_10,
                        operation_type, valid_val, prev_db_val,
                        vect_table_attr_schema, vect_prim_keys, dummy_valid);
  // cs_attr_status
  if (operation_type == UNC_OP_UPDATE) {
    valid_val = UNC_VF_INVALID;
  }
  stringstream attr_status;
  for (unsigned int index = 0; index < ODBCM_SIZE_10; ++index) {
    attr_status << CREATED;
  }
  PhyUtil::FillDbSchema(unc::uppl::CTR_CS_ATTR, CTR_CS_ATTR_STR,
                        attr_status.str(),
                        attr_status.str().length(), DATATYPE_UINT8_ARRAY_10,
                        operation_type, valid_val, prev_db_val,
                        vect_table_attr_schema, vect_prim_keys, dummy_valid);
  // cs_row status
  if (is_filtering == true) {
    vect_prim_keys.push_back(CTR_CS_ROW_STATUS_STR);
  }
  value = PhyUtil::uint8tostr(row_status);
  if (operation_type >= UNC_OP_READ) {
    PhyUtil::FillDbSchema(unc::uppl::CTR_CS_ROW_STATUS, value,
                          value.length(), DATATYPE_UINT16,
                          vect_table_attr_schema);
  }
  vect_prim_keys.push_back(CTR_NAME_STR);
  PhyUtil::reorder_col_attrs(vect_prim_keys, vect_table_attr_schema);
  kt_controller_dbtableschema.set_table_name(unc::uppl::CTR_TABLE);
  kt_controller_dbtableschema.set_primary_keys(vect_prim_keys);
  row_list.push_back(vect_table_attr_schema);
  kt_controller_dbtableschema.set_row_list(row_list);
  return;
}

/** PopulateDBSchemaForCommitVersion
 * @Description : This function populates the DBAttrSchema to be used to send
 *                request to ODBC, here we introduced valid flag column for
 *                each attribute column.
 * @param[in] : kt_controller_dbtableschema-DBTableSchema instance associated
 *              with Controller
 * key_struct-void* to CTR key structure
 * val_struct-void* to CTR Commit version value structure
 * operation_type-UNC_OP_*,type of operation
 * option1,option2-not used
 * vect_key_operations-not used
 * old_value_struct-not used
 * @return    : void
 * */
void Kt_Controller::PopulateDBSchemaForCommitVersion(
    OdbcmConnectionHandler *db_conn,
    DBTableSchema &kt_controller_dbtableschema,
    void* key_struct,
    void* val_struct,
    uint8_t operation_type) {
  // Construct Primary key list
  vector<string> vect_prim_keys;

  // Construct TableAttrSchema structure
  // TableAttrSchema holds table_name, primary key, attr_name
  vector<TableAttrSchema> vect_table_attr_schema;
  list < vector<TableAttrSchema> > row_list;

  key_ctr_t *obj_key_ctr = reinterpret_cast<key_ctr_t*>(key_struct);
  val_ctr_commit_ver_t *obj_val_ctr =
              reinterpret_cast<val_ctr_commit_ver_t*>(val_struct);

  pfc_log_debug("operation: %d", operation_type);
  stringstream valid;
  // Controller_name
  string controller_name = (const char*)obj_key_ctr->controller_name;
  pfc_log_info("controller name: %s", controller_name.c_str());
  PhyUtil::FillDbSchema(unc::uppl::CTR_NAME, controller_name,
                        controller_name.length(), DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);
  uint16_t valid_val = 0;
  string value = "";
  // Commit number
  if (obj_val_ctr != NULL) {
    valid_val = PhyUtil::uint8touint(obj_val_ctr->valid[kIdxCtrCommitNumber]);
    value = PhyUtil::uint64tostr(obj_val_ctr->commit_number);
  } else {
    valid_val = UPPL_NO_VAL_STRUCT;
  }
  PhyUtil::FillDbSchema(unc::uppl::CTR_COMMIT_NUMBER, CTR_COMMIT_NUMBER_STR,
                        value, value.length(), DATATYPE_UINT64,
                        operation_type, valid_val, 0,
                        vect_table_attr_schema, vect_prim_keys, valid);
  value.clear();
  // commit date
  if (obj_val_ctr != NULL) {
    valid_val = PhyUtil::uint8touint(obj_val_ctr->valid[kIdxCtrCommitDate]);
    value  =  PhyUtil::uint64tostr(obj_val_ctr->commit_date);
  } else {
    valid_val = UPPL_NO_VAL_STRUCT;
  }
  PhyUtil::FillDbSchema(unc::uppl::CTR_COMMIT_DATE, CTR_COMMIT_DATE_STR, value,
                       value.length(), DATATYPE_UINT64,
                       operation_type, valid_val, 0,
                       vect_table_attr_schema, vect_prim_keys, valid);
  value.clear();
  // commit application
  if (obj_val_ctr != NULL) {
    valid_val = PhyUtil::uint8touint
                               (obj_val_ctr->valid[kIdxCtrCommitApplication]);
    value = (const char*)obj_val_ctr->commit_application;
  } else {
    valid_val = UPPL_NO_VAL_STRUCT;
  }
  PhyUtil::FillDbSchema(unc::uppl::CTR_COMMIT_APPLICATION,
                       CTR_COMMIT_APPLICATION_STR, value, value.length(),
                       DATATYPE_UINT8_ARRAY_256,
                       operation_type, valid_val, 0,
                       vect_table_attr_schema, vect_prim_keys, valid);
  value.clear();
  // valid
  valid_val = UPPL_NO_VAL_STRUCT;
  stringstream dummy_valid;
  PhyUtil::FillDbSchema(unc::uppl::CTR_VALID_COMMIT_VERSION,
                        CTR_VALID_COMMIT_VERSION_STR, valid.str(),
                        valid.str().length(), DATATYPE_UINT8_ARRAY_3,
                        operation_type, valid_val, 0,
                        vect_table_attr_schema, vect_prim_keys, dummy_valid);

  vect_prim_keys.push_back(CTR_NAME_STR);
  PhyUtil::reorder_col_attrs(vect_prim_keys, vect_table_attr_schema);
  kt_controller_dbtableschema.set_table_name(unc::uppl::CTR_TABLE);
  kt_controller_dbtableschema.set_primary_keys(vect_prim_keys);
  row_list.push_back(vect_table_attr_schema);
  kt_controller_dbtableschema.set_row_list(row_list);
  return;
}

/**
 * @Description : This function populates val_ctr_t by values retrieved
 * from database
 * @param[in] : kt_controller_dbtableschema-DBTableSchema instance to be filled
 * CTR commit ver value structures
 * max_rep_ct-max. no. of records to be read
 * operation_type-type of operation,UNC_OP_*
 * controller_id-vector of strings containg the controller names
 * @return    : void
 * */
void Kt_Controller::FillControllerCommitVerStructure(
    OdbcmConnectionHandler *db_conn,
    DBTableSchema &kt_controller_dbtableschema,
    vector<val_ctr_commit_ver_t> &vect_obj_val_ctr,
    uint32_t &max_rep_ct,
    uint32_t operation_type,
    vector<string> &controller_id) {
  // populate IPC value structure based on the response received from DB
  list < vector<TableAttrSchema> >& res_ctr_row_list =
      kt_controller_dbtableschema.get_row_list();
  list < vector<TableAttrSchema> > :: iterator res_ctr_iter =
      res_ctr_row_list.begin();
  max_rep_ct = res_ctr_row_list.size();
  pfc_log_debug("res_ctr_row_list.size: %d", max_rep_ct);

  // populate IPC value structure based on the response received from DB
  for (; res_ctr_iter != res_ctr_row_list.end(); ++res_ctr_iter) {
    vector<TableAttrSchema> res_ctr_table_attr_schema = (*res_ctr_iter);
    vector<TableAttrSchema> :: iterator vect_ctr_iter =
        res_ctr_table_attr_schema.begin();
    val_ctr_commit_ver_t obj_val_ctr_cv;
    memset(&obj_val_ctr_cv, '\0', sizeof(val_ctr_commit_ver_t));
    for (; vect_ctr_iter != res_ctr_table_attr_schema.end();
        ++vect_ctr_iter) {
      // populate values from controller_table
      TableAttrSchema tab_schema = (*vect_ctr_iter);
      ODBCMTableColumns attr_name = tab_schema.table_attribute_name;
      string attr_value = "";
      switch (attr_name) {
        case unc::uppl::CTR_NAME :
          uint8_t ctr_name[ODBCM_SIZE_32];
          PhyUtil::GetValueFromDbSchemaStr(tab_schema, ctr_name,
                                           DATATYPE_UINT8_ARRAY_32);
          controller_id.push_back(reinterpret_cast<const char*>(ctr_name));
          pfc_log_debug("controller_name: %s", ctr_name);
          break;

        case unc::uppl::CTR_COMMIT_NUMBER:
          PhyUtil::GetValueFromDbSchema(tab_schema, attr_value,
                                        DATATYPE_UINT64);
          obj_val_ctr_cv.commit_number = atol(attr_value.c_str());
          break;

        case unc::uppl::CTR_COMMIT_DATE:
          PhyUtil::GetValueFromDbSchema(tab_schema, attr_value,
                                        DATATYPE_UINT64);
          obj_val_ctr_cv.commit_date = atol(attr_value.c_str());
          break;

        case unc::uppl::CTR_COMMIT_APPLICATION:
          PhyUtil::GetValueFromDbSchemaStr(tab_schema,
                                  obj_val_ctr_cv.commit_application,
                                           DATATYPE_UINT8_ARRAY_256);
          break;

        case unc::uppl::CTR_VALID_COMMIT_VERSION:
          uint8_t ctr_valid[ODBCM_SIZE_3];
          PhyUtil::GetValueFromDbSchemaStr(tab_schema, ctr_valid,
                                           DATATYPE_UINT8_ARRAY_3);
          FrameCVValidValue(reinterpret_cast<const char*>(ctr_valid),
                          obj_val_ctr_cv);
          break;

        default:
          pfc_log_info("Ignoring Controller attribute %d", attr_name);
          break;
      }
    }
    vect_obj_val_ctr.push_back(obj_val_ctr_cv);
  }
  return;
}

/**
 * @Description : This function populates val_ctr_t by values retrieved
 * from database
 * @param[in] : kt_controller_dbtableschema-DBTableSchema instance to be filled
 * vect_obj_val_ctr-vector of CTR value structures
 * max_rep_ct-max. no. of records to be read
 * operation_type-type of operation,UNC_OP_*
 * controller_id-vector of strings containg the controller names
 * @return    : void
 * */
void Kt_Controller::FillControllerValueStructure(
    OdbcmConnectionHandler *db_conn,
    DBTableSchema &kt_controller_dbtableschema,
    vector<val_ctr_st_t> &vect_obj_val_ctr,
    uint32_t &max_rep_ct,
    uint32_t operation_type,
    vector<string> &controller_id) {
  // populate IPC value structure based on the response received from DB
  list < vector<TableAttrSchema> >& res_ctr_row_list =
      kt_controller_dbtableschema.get_row_list();
  list < vector<TableAttrSchema> > :: iterator res_ctr_iter =
      res_ctr_row_list.begin();
  max_rep_ct = res_ctr_row_list.size();
  uint8_t actual_id_valid[1];
  pfc_log_debug("res_ctr_row_list.size: %d", max_rep_ct);
  // populate IPC value structure based on the response received from DB
  for (; res_ctr_iter != res_ctr_row_list.end(); ++res_ctr_iter) {
    vector<TableAttrSchema> res_ctr_table_attr_schema = (*res_ctr_iter);
    vector<TableAttrSchema> :: iterator vect_ctr_iter =
        res_ctr_table_attr_schema.begin();
    val_ctr_st_t obj_val_ctr_st;
    memset(actual_id_valid, 0, sizeof(actual_id_valid));
    memset(&obj_val_ctr_st, '\0', sizeof(val_ctr_st_t));
    val_ctr_t obj_val_ctr;
    memset(&obj_val_ctr, 0, sizeof(val_ctr_t));
    // Read all attributes
    vector<int> valid_flag, cs_attr;
    for (; vect_ctr_iter != res_ctr_table_attr_schema.end();
        ++vect_ctr_iter) {
      // populate values from controller_table
      TableAttrSchema tab_schema = (*vect_ctr_iter);
      ODBCMTableColumns attr_name = tab_schema.table_attribute_name;
      string attr_value = "";
      switch (attr_name) {
        case unc::uppl::CTR_NAME :
          uint8_t ctr_name[ODBCM_SIZE_32];
          PhyUtil::GetValueFromDbSchemaStr(tab_schema, ctr_name,
                                           DATATYPE_UINT8_ARRAY_32);
          controller_id.push_back(reinterpret_cast<const char*>(ctr_name));
          pfc_log_debug("controller_name: %s", ctr_name);
          break;

        case unc::uppl::CTR_TYPE :
          PhyUtil::GetValueFromDbSchema(tab_schema, attr_value,
                                        DATATYPE_UINT16);
          obj_val_ctr.type = atoi(attr_value.c_str());
          break;

        case unc::uppl::CTR_DESCRIPTION :
          PhyUtil::GetValueFromDbSchemaStr(tab_schema, obj_val_ctr.description,
                                           DATATYPE_UINT8_ARRAY_128);
          break;

        case unc::uppl::CTR_IP_ADDRESS:
          PhyUtil::GetValueFromDbSchema(tab_schema, attr_value,
                                        DATATYPE_IPV4);
          inet_pton(AF_INET, (const char *)attr_value.c_str(),
                    &obj_val_ctr.ip_address.s_addr);
          break;

        case unc::uppl::CTR_USER_NAME:
          PhyUtil::GetValueFromDbSchemaStr(tab_schema, obj_val_ctr.user,
                                           DATATYPE_UINT8_ARRAY_32);
          break;

        case unc::uppl::CTR_PASSWORD:
          PhyUtil::GetValueFromDbSchemaStr(tab_schema, obj_val_ctr.password,
                                           DATATYPE_UINT8_ARRAY_257);
          break;

        case unc::uppl::CTR_VALID:
          uint8_t ctr_valid[ODBCM_SIZE_10];
          PhyUtil::GetValueFromDbSchemaStr(tab_schema, ctr_valid,
                                           DATATYPE_UINT8_ARRAY_10);
          memset(obj_val_ctr.valid, '\0', 8);
          FrameValidValue(reinterpret_cast<const char*>(ctr_valid),
                          obj_val_ctr_st, obj_val_ctr);
          break;

        case unc::uppl::CTR_CS_ATTR:
          uint8_t ctr_cs_attr[ODBCM_SIZE_10];
          PhyUtil::GetValueFromDbSchemaStr(tab_schema, ctr_cs_attr,
                                           DATATYPE_UINT8_ARRAY_10);
          memset(obj_val_ctr.cs_attr, '\0', 8);
          FrameCsAttrValue(reinterpret_cast<const char*>(ctr_cs_attr),
                           obj_val_ctr);
          break;

        case unc::uppl::CTR_CS_ROW_STATUS:
          PhyUtil::GetValueFromDbSchema(tab_schema, attr_value,
                                        DATATYPE_UINT16);
          obj_val_ctr.cs_row_status = atoi(attr_value.c_str());
          break;

        case unc::uppl::CTR_ENABLE_AUDIT:
          PhyUtil::GetValueFromDbSchema(tab_schema, attr_value,
                                        DATATYPE_UINT16);
          obj_val_ctr.enable_audit = atoi(attr_value.c_str());
          break;

        case unc::uppl::CTR_PORT:
          PhyUtil::GetValueFromDbSchema(tab_schema, attr_value,
                                        DATATYPE_UINT16);
          obj_val_ctr.port = atoi(attr_value.c_str());
          break;

        case unc::uppl::CTR_VERSION:
          PhyUtil::GetValueFromDbSchemaStr(tab_schema, obj_val_ctr.version,
                                           DATATYPE_UINT8_ARRAY_32);
          break;

        case unc::uppl::CTR_OPER_STATUS:
          PhyUtil::GetValueFromDbSchema(tab_schema, attr_value,
                                        DATATYPE_UINT16);
          obj_val_ctr_st.oper_status = atoi(attr_value.c_str());
          break;

        case unc::uppl::CTR_ACTUAL_VERSION:
          PhyUtil::GetValueFromDbSchemaStr(tab_schema,
                                           obj_val_ctr_st.actual_version,
                                           DATATYPE_UINT8_ARRAY_32);
          break;
        case unc::uppl::CTR_ACTUAL_CONTROLLERID:
          PhyUtil::GetValueFromDbSchemaStr(tab_schema,
                                           obj_val_ctr_st.actual_id,
                                           DATATYPE_UINT8_ARRAY_32);
          break;
        case unc::uppl::CTR_VALID_ACTUAL_CONTROLLERID:
          PhyUtil::GetValueFromDbSchemaStr(tab_schema,
                                           actual_id_valid,
                                           DATATYPE_UINT8_ARRAY_1);
          obj_val_ctr_st.valid[kIdxActualId] =
                    static_cast<int>(actual_id_valid[0] - 48);
          break;
        default:
          pfc_log_info("Ignoring Controller attribute %d", attr_name);
          break;
      }
    }
    memcpy(&obj_val_ctr_st.controller, &obj_val_ctr, sizeof(val_ctr_t));
    vect_obj_val_ctr.push_back(obj_val_ctr_st);
  }
  return;
}

/** PerformRead
 * @Description : This function reads the instance of KT_Controller based on
 *                  operation type - READ, READ_SIBLING_BEGIN, READ_SIBLING
 * @param[in] : session_id-ipc session id used for TC validation
 * configuration_id-ipc configuration id used for TC validation
 * key_struct-void* to ctr key structure
 * value_struct-void* to ctr value structure
 * data_type-UNC_DT_*,type of database
 * operation _type-UNC_OP_*,type of database
 * sess- object of ServerSession
 * option1,option2-UNC_OPT1/OPT2_*,additional infor read operations
 * max_rep_ct-max. no of records to be read
 * @return    : UNC_RC_SUCCESS or UNC_UPPL_RC_ERR*,
 * UNC_RC_SUCCESS is returned when the response
 * is added to ipc session successfully.
 * UNC_UPPL_RC_ERR_* is returned when ipc response could not be added to sess.
 * */
UncRespCode Kt_Controller::PerformRead(OdbcmConnectionHandler *db_conn,
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
  pfc_log_debug("PerformRead oper=%d dt=%d"
              " max_rep_ct=%d", operation_type, data_type, max_rep_ct);
  key_ctr_t *obj_key_ctr= reinterpret_cast<key_ctr_t*>(key_struct);
  UncRespCode read_status = UNC_RC_SUCCESS;
  if (option1 != UNC_OPT1_NORMAL) {
    pfc_log_error("Invalid option1 specified for read operation");
    physical_response_header rsh = {session_id,
        configuration_id,
        operation_type,
        max_rep_ct,
        option1,
        option2,
        data_type,
        static_cast<uint32_t>(UNC_UPPL_RC_ERR_INVALID_OPTION1)};
    int err = PhyUtil::sessOutRespHeader(sess, rsh);
    err |= sess.addOutput((uint32_t)UNC_KT_CONTROLLER);
    err |= sess.addOutput(*obj_key_ctr);
    if (err != 0) {
      pfc_log_info("addOutput failed for physical_response_header");
      return UNC_UPPL_RC_ERR_IPC_WRITE_ERROR;
    }
    return UNC_RC_SUCCESS;
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
        static_cast<uint32_t>(UNC_UPPL_RC_ERR_INVALID_OPTION2)};
    int err = PhyUtil::sessOutRespHeader(sess, rsh);
    err |= sess.addOutput((uint32_t)UNC_KT_CONTROLLER);
    err |= sess.addOutput(*obj_key_ctr);
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
    physical_response_header rsh = {session_id,
        configuration_id,
        operation_type,
        max_rep_ct,
        option1,
        option2,
        data_type,
        static_cast<uint32_t>(UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED)};
    int err = PhyUtil::sessOutRespHeader(sess, rsh);
    err |= sess.addOutput((uint32_t)UNC_KT_CONTROLLER);
    err |= sess.addOutput(*obj_key_ctr);
    if (err != 0) {
      pfc_log_error("addOutput failed for physical_response_header");
      return UNC_UPPL_RC_ERR_IPC_WRITE_ERROR;
    }
    return UNC_RC_SUCCESS;
  }
  // Read from DB directly for all data types
  vector<string> vect_controller_id;
  vector<val_ctr_st_t> vect_val_ctr_st;
  read_status = ReadCtrValFromDB(db_conn, key_struct,
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
      static_cast<uint32_t>(read_status)};

  if (read_status == UNC_RC_SUCCESS) {
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
      rsh.result_code = UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE;
      int err = PhyUtil::sessOutRespHeader(sess, rsh);
      err |= sess.addOutput((uint32_t)UNC_KT_CONTROLLER);
      err |= sess.addOutput(*obj_key_ctr);
      if (err != 0) {
        pfc_log_error("addOutput failed for physical_response_header");
        return UNC_UPPL_RC_ERR_IPC_WRITE_ERROR;
      }
      return UNC_RC_SUCCESS;
    }
    int err = PhyUtil::sessOutRespHeader(sess, rsh);
    if (err != 0) {
      pfc_log_info("addOutput failed for physical_response_header");
      return UNC_UPPL_RC_ERR_IPC_WRITE_ERROR;
    }
    for (unsigned int index = 0; index < vect_val_ctr_st.size(); ++index) {
      key_ctr_t obj_ctr;
      memset(&obj_ctr, '\0', sizeof(key_ctr_t));
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
      pfc_log_info("addOutput failed for physical_response_header");
      return UNC_UPPL_RC_ERR_IPC_WRITE_ERROR;
    }
  }
  pfc_log_debug("Return value for read operation %d, max_rep_ct %d",
                                              read_status, max_rep_ct);
  return read_status;
}

/** ReadCtrCommitVerFromDB
 * @Description : This function reads the instance of KT_Controller based on
 *                  operation type - READ from data base
 * @param[in] : key_struct-void* to CTR key structure
 * value_struct-void* to CTR commit ver val structure
 * data_type-UNC_DT_*,type of database
 * operation type-UNC_OP_*,type of operation
 * max_rep_ct-max no of records to be read
 * vect_val_ctr_st-instance of vector<val_ctr_commit_ver_t>
 * controller_id-instance of vector<string> containing ctr keys
 * @return    : UNC_RC_SUCCESS or UNC_UPPL_RC_ERR*
 * UNC_RC_SUCCESS is returned when the response
 * is added to ipc session successfully.
 * UNC_UPPL_RC_ERR_* is returned when ipc response could not be added to sess.
 * */
UncRespCode Kt_Controller::ReadCtrCommitVerFromDB(
    OdbcmConnectionHandler *db_conn,
    void* key_struct,
    void* val_struct,
    uint32_t data_type,
    uint32_t operation_type,
    uint32_t &max_rep_ct,
    vector<val_ctr_commit_ver_t> &vect_val_ctr_cv,
    vector<string> &controller_id) {
  pfc_log_debug("ReadCtrCommitVerFromDB()");
  if (operation_type < UNC_OP_READ) {
    // Unsupported operation type for this function
    return UNC_RC_SUCCESS;
  }
  pfc_log_debug("ReadCtrCommitVerFromDB1()");
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  UncRespCode read_status = UNC_RC_SUCCESS;
  ODBCM_RC_STATUS read_db_status = ODBCM_RC_SUCCESS;

  // Common structures that will be used to send query to ODBC
  // Structure used to send request to ODBC
  DBTableSchema kt_controller_dbtableschema;
  PopulateDBSchemaForCommitVersion(db_conn, kt_controller_dbtableschema,
                             key_struct,
                             val_struct,
                             operation_type);
  read_db_status = physical_layer->get_odbc_manager()->
        GetOneRow((unc_keytype_datatype_t)data_type,
                  kt_controller_dbtableschema, db_conn);
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
  FillControllerCommitVerStructure(db_conn, kt_controller_dbtableschema,
                               vect_val_ctr_cv,
                               max_rep_ct,
                               operation_type,
                               controller_id);
  pfc_log_debug("CmtVer:vect_val_ctr size: %" PFC_PFMT_SIZE_T,
                 vect_val_ctr_cv.size());
  pfc_log_debug("CmtVer:controller_id size: %" PFC_PFMT_SIZE_T,
                 controller_id.size());
  if (vect_val_ctr_cv.empty()) {
    // Read failed , return error
    read_status = UNC_UPPL_RC_ERR_DB_GET;
    // log error to log daemon
    pfc_log_error("Read operation has failed after reading response");
    return read_status;
  }
  return read_status;
}

/** ReadCtrValFromDB
 * @Description : This function reads the instance of KT_Controller based on
 *                  operation type - READ, READ_SIBLING_BEGIN, READ_SIBLING
 *                   from data base
 * @param[in] : key_struct-void* to CTR key structure
 * value_struct-void* to CTR val structure
 * data_type-UNC_DT_*,type of database
 * operation type-UNC_OP_*,type of operation
 * max_rep_ct-max no of records to be read
 * vect_val_ctr_st-instance of vector<val_ctr_st_t>
 * controller_id-instance of vector<string> containing ctr keys
 * @return    : UNC_RC_SUCCESS or UNC_UPPL_RC_ERR*
 * UNC_RC_SUCCESS is returned when the response
 * is added to ipc session successfully.
 * UNC_UPPL_RC_ERR_* is returned when ipc response could not be added to sess.
 * */
UncRespCode Kt_Controller::ReadCtrValFromDB(
    OdbcmConnectionHandler *db_conn,
    void* key_struct,
    void* val_struct,
    uint32_t data_type,
    uint32_t operation_type,
    uint32_t &max_rep_ct,
    vector<val_ctr_st_t> &vect_val_ctr_st,
    vector<string> &controller_id) {
  if (operation_type < UNC_OP_READ) {
    // Unsupported operation type for this function
    return UNC_RC_SUCCESS;
  }
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  UncRespCode read_status = UNC_RC_SUCCESS;
  ODBCM_RC_STATUS read_db_status = ODBCM_RC_SUCCESS;

  // Common structures that will be used to send query to ODBC
  // Structure used to send request to ODBC
  DBTableSchema kt_controller_dbtableschema;
  void *old_val_struct = NULL;
  vector<ODBCMOperator> vect_key_operations;
  CsRowStatus cs_row_status = NOTAPPLIED;
  PopulateDBSchemaForKtTable(db_conn, kt_controller_dbtableschema,
                             key_struct,
                             val_struct,
                             operation_type, data_type, 0, 0,
                             vect_key_operations, old_val_struct,
                             cs_row_status, false, PFC_FALSE);

  if (operation_type == UNC_OP_READ) {
    read_db_status = physical_layer->get_odbc_manager()->
        GetOneRow((unc_keytype_datatype_t)data_type,
                  kt_controller_dbtableschema, db_conn);
  } else {
    read_db_status = physical_layer->get_odbc_manager()->
        GetBulkRows((unc_keytype_datatype_t)data_type, max_rep_ct,
                    kt_controller_dbtableschema,
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
  FillControllerValueStructure(db_conn, kt_controller_dbtableschema,
                               vect_val_ctr_st,
                               max_rep_ct,
                               operation_type,
                               controller_id);
  pfc_log_debug("vect_val_ctr size: %" PFC_PFMT_SIZE_T, vect_val_ctr_st.size());
  pfc_log_debug("controller_id size: %" PFC_PFMT_SIZE_T, controller_id.size());
  if (vect_val_ctr_st.empty()) {
    // Read failed , return error
    read_status = UNC_UPPL_RC_ERR_DB_GET;
    // log error to log daemon
    pfc_log_error("Read operation has failed after reading response");
    return read_status;
  }
  return read_status;
}

/** getChildKeyStruct
 * @Description : This function returns the void * of child key structures
 * @param[in] : child_class-child class index indicating children of Controller
 * controller_name-controller id
 * @return    : void * key structure
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
 * @Description : This function clearsthe void * of child key structures
 * @param[in] : child_class-child class index indicating children of Controller
 * key_struct-void* to ctr key structure
 * @return    : void
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
 * @Description : This function reads all KT_Controller with given row_status
 * @param[in] : obj_key_struct-vector of void* to ctr key structure
 * row_status-CsRowStatus value
 * @return    : UNC_RC_SUCCESS or UNC_UPPL_RC_ERR*
 * UNC_RC_SUCCESS is returned when the response
 * is added to ipc session successfully.
 * UNC_UPPL_RC_ERR_* is returned when ipc response could not be added to sess.
 * */
UncRespCode Kt_Controller::GetModifiedRows(OdbcmConnectionHandler *db_conn,
                                              vector<void *> &obj_key_struct,
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

  UncRespCode read_status = UNC_RC_SUCCESS;
  ODBCM_RC_STATUS read_db_status = ODBCM_RC_SUCCESS;

  DBTableSchema kt_controller_dbtableschema;
  void *old_val_struct;
  vector<ODBCMOperator> vect_key_operations;
  PopulateDBSchemaForKtTable(db_conn, kt_controller_dbtableschema,
                             ctr_key, ctr_val,
                             UNC_OP_READ, UNC_DT_CANDIDATE, 0, 0,
                             vect_key_operations, old_val_struct,
                             row_status,
                             true, PFC_FALSE);

  read_db_status = physical_layer->get_odbc_manager()->
      GetModifiedRows(UNC_DT_CANDIDATE, kt_controller_dbtableschema, db_conn);
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
  vector<string> controller_id;

  FillControllerValueStructure(db_conn, kt_controller_dbtableschema,
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
 * @Description : This function populates the values to be used for attribute
 * validation
 * @param[in] : None
 * @return    : void
 * */
void Kt_Controller::Fill_Attr_Syntax_Map() {
  map<string, Kt_Class_Attr_Syntax> attr_syntax_map;
  Kt_Class_Attr_Syntax objKeyAttrSyntax =
  { PFC_IPCTYPE_UINT8, 0, 0, 1, 32, true,  "" };
  attr_syntax_map[CTR_NAME_STR] = objKeyAttrSyntax;

  Kt_Class_Attr_Syntax objAttrTypeSyntax =
  { PFC_IPCTYPE_UINT8, 0, 5, 0, 0, true,  "" };
  attr_syntax_map[CTR_TYPE_STR] = objAttrTypeSyntax;

  Kt_Class_Attr_Syntax objAttrVersionSyntax =
  { PFC_IPCTYPE_STRING, 0, 0, 1, 32, true, "" };
  attr_syntax_map[CTR_VERSION_STR] = objAttrVersionSyntax;

  Kt_Class_Attr_Syntax objAttrDescSyntax =
  { PFC_IPCTYPE_STRING, 0, 0, 1, 128, false,  "" };
  attr_syntax_map[CTR_DESCRIPTION_STR] = objAttrDescSyntax;

  Kt_Class_Attr_Syntax objAttrIpSyntax =
  { PFC_IPCTYPE_IPV4, 0, 4294967295LL, 0, 0, false,  "" };
  attr_syntax_map[CTR_IP_ADDRESS_STR] = objAttrIpSyntax;

  Kt_Class_Attr_Syntax objAttrUserSyntax =
  { PFC_IPCTYPE_STRING, 0, 0, 1, 32, false, "" };
  attr_syntax_map[IPCT_USER] = objAttrUserSyntax;

  Kt_Class_Attr_Syntax objAttrPswdSyntax =
  { PFC_IPCTYPE_STRING, 0, 0, 1, 257, false, "" };
  attr_syntax_map[CTR_PASSWORD_STR] = objAttrPswdSyntax;

  Kt_Class_Attr_Syntax objAttrAuditSyntax =
  { PFC_IPCTYPE_UINT8, 0, 1, 0, 0, false, "" };
  attr_syntax_map[CTR_ENABLE_AUDIT_STR] = objAttrAuditSyntax;

  Kt_Class_Attr_Syntax objAttrPortSyntax =
  { PFC_IPCTYPE_UINT16, 0, 65535, 0, 0, false, "" };
  attr_syntax_map[CTR_PORT_STR] = objAttrPortSyntax;

  Kt_Class_Attr_Syntax objAttrValidSyntax =
  { PFC_IPCTYPE_STRING, 0, 0, 0, 7, false, "" };
  attr_syntax_map[CTR_VALID_STR] = objAttrValidSyntax;

  Kt_Class_Attr_Syntax objAttrCsRowSyntax =
  { PFC_IPCTYPE_STRING, 0, 3, 0, 0, false, "" };
  attr_syntax_map[CTR_CS_ROW_STATUS_STR] = objAttrCsRowSyntax;

  Kt_Class_Attr_Syntax objAttrCsAttrSyntax =
  { PFC_IPCTYPE_STRING, 0, 0, 0, 7, false, "" };
  attr_syntax_map[CTR_CS_ATTR_STR] = objAttrCsAttrSyntax;
  attr_syntax_map_all[UNC_KT_CONTROLLER] = attr_syntax_map;
}

/** GetClassPointerAndKey
 * @Description : This function returns the void * of requested key structures
 * and class pointer
 * @param[in] : key_type-any value of KtControllerOperStatusRef 
 * controller name-controller id
 * key-void* to ctr key structure
 * @return    : KtBase* key structure
 * */
Kt_Base* Kt_Controller::GetClassPointerAndKey(
    KtControllerOperStatusRef key_type,
    string controller_name,
    void* &key) {
  Kt_Base* class_pointer = NULL;
  switch (key_type) {
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
  }
  return class_pointer;
}

/** FreeKeyStruct
 * @Description : This function clears the key struct
 * @param[in] : key type-any child kt of Controller
 * key_struct-void* to any controller's child key structure
 * @return    : void
 * */
void Kt_Controller::FreeKeyStruct(void* key_struct,
                                  unsigned int key_type) {
  switch (key_type) {
    case KtSwitch: {
      key_switch_t *obj_key = reinterpret_cast<key_switch_t *>
      (key_struct);
      if (obj_key != NULL) {
        delete obj_key;
        obj_key = NULL;
      }
      break;
    }
  }
  return;
}

/** GetOperStatus
 * @Description : This function reads the oper_status value of the controller
 * @param[in] : key_struct
 * data_type-UNC_DT_*,type of database
 * param[out]:oper_status-oper status of Controller whether up/down/auditing
 * @return    : Success or associated error code
 * UNC_RC_SUCCESS is returned when the response
 * is added to ipc session successfully.
 * UNC_UPPL_RC_ERR_* is returned when ipc response could not be added to sess.
 */
UncRespCode Kt_Controller::GetOperStatus(OdbcmConnectionHandler *db_conn,
                                            uint32_t data_type,
                                            void* key_struct,
                                            uint8_t &oper_status) {
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  UncRespCode return_code = UNC_RC_SUCCESS;
  vector<string> vect_prim_keys;
  vect_prim_keys.push_back(CTR_NAME_STR);

  vector<TableAttrSchema> vect_table_attr_schema;
  list < vector<TableAttrSchema> > row_list;
  DBTableSchema kt_controller_dbtableschema;

  key_ctr_t *obj_key_ctr = reinterpret_cast<key_ctr_t*>(key_struct);

  // Controller_name
  string controller_name = (const char*)obj_key_ctr->controller_name;
  pfc_log_info("controller name: %s", controller_name.c_str());
  PhyUtil::FillDbSchema(unc::uppl::CTR_NAME, controller_name,
                        controller_name.length(), DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);

  // oper_status
  string value = "";
  PhyUtil::FillDbSchema(unc::uppl::CTR_OPER_STATUS, value,
                        value.length(), DATATYPE_UINT16,
                        vect_table_attr_schema);

  kt_controller_dbtableschema.set_table_name(unc::uppl::CTR_TABLE);
  kt_controller_dbtableschema.set_primary_keys(vect_prim_keys);
  row_list.push_back(vect_table_attr_schema);
  kt_controller_dbtableschema.set_row_list(row_list);

  ODBCM_RC_STATUS read_db_status = physical_layer->get_odbc_manager()->
      GetOneRow(UNC_DT_RUNNING, kt_controller_dbtableschema, db_conn);
  if (read_db_status == ODBCM_RC_SUCCESS) {
    // populate IPC value structure based on the response received from DB
    vector<TableAttrSchema> res_table_attr_schema =
        kt_controller_dbtableschema.get_row_list().front();
    vector<TableAttrSchema> ::iterator vect_iter =
        res_table_attr_schema.begin();
    for (; vect_iter != res_table_attr_schema.end(); ++vect_iter) {
      TableAttrSchema tab_schema = (*vect_iter);
      ODBCMTableColumns attr_name = tab_schema.table_attribute_name;
      string attr_value = "";
      if (attr_name == unc::uppl::CTR_OPER_STATUS) {
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

/** SetOperStatus
 * @Description : This function updates the oper_status value
 *  of the controller
 * @param[in] : key_struct-void* to ctr key structure
 * data_type-UNC_DT_*,type of database
 * oper_status-oper status of Controller
 * @return    : Success or associated error code 
 * UNC_RC_SUCCESS is returned when the response
 * is added to ipc session successfully.
 * UNC_UPPL_RC_ERR_* is returned when ipc response could not be added to sess.
 */
UncRespCode Kt_Controller::SetOperStatus(OdbcmConnectionHandler *db_conn,
                                            uint32_t data_type,
                                            void* key_struct,
                                            uint8_t oper_status) {
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  UncRespCode return_code = UNC_RC_SUCCESS;
  vector<string> vect_prim_keys;
  vect_prim_keys.push_back(CTR_NAME_STR);

  vector<TableAttrSchema> vect_table_attr_schema;
  list < vector<TableAttrSchema> > row_list;
  DBTableSchema kt_controller_dbtableschema;

  key_ctr_t *obj_key_ctr = reinterpret_cast<key_ctr_t*>(key_struct);

  // Controller_name
  string controller_name = (const char*)obj_key_ctr->controller_name;
  pfc_log_info("controller name: %s", controller_name.c_str());
  PhyUtil::FillDbSchema(unc::uppl::CTR_NAME, controller_name,
                        controller_name.length(), DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);

  // oper_status
  string value = PhyUtil::uint8tostr(oper_status);
  PhyUtil::FillDbSchema(unc::uppl::CTR_OPER_STATUS, value,
                        value.length(), DATATYPE_UINT16,
                        vect_table_attr_schema);

  kt_controller_dbtableschema.set_table_name(unc::uppl::CTR_TABLE);
  kt_controller_dbtableschema.set_primary_keys(vect_prim_keys);
  row_list.push_back(vect_table_attr_schema);
  kt_controller_dbtableschema.set_row_list(row_list);

  ODBCM_RC_STATUS update_db_status = physical_layer->get_odbc_manager()->
      UpdateOneRow(UNC_DT_RUNNING, kt_controller_dbtableschema, db_conn, true);
  if (update_db_status == ODBCM_RC_SUCCESS) {
    pfc_log_info("oper_status updated in DB successfully");
  } else if (update_db_status == ODBCM_RC_CONNECTION_ERROR) {
    UPPL_LOG_FATAL("DB connection issue during set oper status");
    return_code = UNC_UPPL_RC_ERR_DB_ACCESS;
  } else {
    pfc_log_info("oper_status update failed in DB");
    return_code = UNC_UPPL_RC_ERR_DB_UPDATE;
  }
  return return_code;
}

/** SetActualVersion
 * @Description : This function updates the actual_version value
 *  of the controller
 * @param[in] : key_struct-void* to ctr key structure
 * actual_version-version of controller
 * data_type-UNC_DT_*,type of database
 * @return    : Success or associated error code
 * UNC_RC_SUCCESS is returned when the response
 * is added to ipc session successfully.
 * UNC_UPPL_RC_ERR_* is returned when ipc response could not be added to sess.
 */
UncRespCode Kt_Controller::SetActualVersion(OdbcmConnectionHandler *db_conn,
                                               void* key_struct,
                                               string actual_version,
                                               uint32_t data_type,
                                               uint32_t valid_flag) {
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  UncRespCode return_code = UNC_RC_SUCCESS;
  vector<string> vect_prim_keys;
  vect_prim_keys.push_back(CTR_NAME_STR);

  vector<TableAttrSchema> vect_table_attr_schema;
  list < vector<TableAttrSchema> > row_list;
  DBTableSchema kt_controller_dbtableschema;

  // Modify valid flag with VALID set for actual_version
  val_ctr_st_t val_ctr_valid_st;
  memset(&val_ctr_valid_st, 0, sizeof(val_ctr_st_t));
  pfc_log_debug("Get Valid value from DB");
  GetCtrValidFlag(db_conn, key_struct, val_ctr_valid_st, data_type);
  stringstream str_valid;
  unsigned int valid = UNC_VF_INVALID;
  /*it is not ODBCM_SIZE_8 since port is at last position*/
  for (unsigned int index = 0; index < ODBCM_SIZE_7;
      ++index) {
  //  type, version, description, ip_address, user, password, enable_audit
    valid = val_ctr_valid_st.controller.valid[index];
    if (valid >= 48) {
      valid -= 48;
    }
    str_valid << valid;
  }
  str_valid << valid_flag;  // Actual Version
  str_valid << UNC_VF_VALID;  // Oper Status
  // Port
  valid = val_ctr_valid_st.controller.valid[kIdxcPort];
  if (valid >= 48) {
    valid -= 48;
  }
  str_valid << valid;

  pfc_log_debug("str_valid %s", str_valid.str().c_str());
  key_ctr_t *obj_key_ctr = reinterpret_cast<key_ctr_t*>(key_struct);

  // Controller_name
  string controller_name = (const char*)obj_key_ctr->controller_name;
  pfc_log_debug("controller name: %s", controller_name.c_str());
  PhyUtil::FillDbSchema(unc::uppl::CTR_NAME, controller_name,
                        controller_name.length(), DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);

  // actual_version_update
  PhyUtil::FillDbSchema(unc::uppl::CTR_ACTUAL_VERSION, actual_version,
                        actual_version.length(), DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);

  PhyUtil::FillDbSchema(unc::uppl::CTR_VALID, str_valid.str(),
                        str_valid.str().length(), DATATYPE_UINT8_ARRAY_10,
                        vect_table_attr_schema);

  kt_controller_dbtableschema.set_table_name(unc::uppl::CTR_TABLE);
  kt_controller_dbtableschema.set_primary_keys(vect_prim_keys);
  row_list.push_back(vect_table_attr_schema);
  kt_controller_dbtableschema.set_row_list(row_list);
  ODBCM_RC_STATUS update_db_status = ODBCM_RC_SUCCESS;
  update_db_status = physical_layer->get_odbc_manager()->
      UpdateOneRow((unc_keytype_datatype_t)data_type,
                   kt_controller_dbtableschema, db_conn, true);
  if (update_db_status == ODBCM_RC_SUCCESS) {
    pfc_log_info("actual version updated in DB successfully");
  } else {
    pfc_log_error("actual version update failed in DB");
    return_code = UNC_UPPL_RC_ERR_DB_UPDATE;
  }
  return return_code;
}

/** SetActualControllerId
 * @Description : This function updates the actual_id value
 *                of the controller
 * @param[in] : key_struct-void* to ctr key structure
 *              actual_id of controller
 *              data_type-UNC_DT_*,type of database
 * @return    : Success or associated error code
 *              UNC_RC_SUCCESS is returned when the response
 *              is added to ipc session successfully.
 *              UNC_UPPL_RC_ERR_* is returned when ipc response 
 *              could not be added to sess.
 */
UncRespCode Kt_Controller::SetActualControllerId(
                                OdbcmConnectionHandler *db_conn,
                                void* key_struct,
                                string actual_id,
                                uint32_t data_type,
                                uint32_t valid_flag) {
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  UncRespCode return_code = UNC_RC_SUCCESS;
  vector<string> vect_prim_keys;
  vect_prim_keys.push_back(CTR_NAME_STR);
  vector<TableAttrSchema> vect_table_attr_schema;
  list < vector<TableAttrSchema> > row_list;
  DBTableSchema kt_controller_dbtableschema;

  key_ctr_t *obj_key_ctr = reinterpret_cast<key_ctr_t*>(key_struct);

  // Controller_name
  string controller_name = (const char*)obj_key_ctr->controller_name;
  pfc_log_debug("controller name: %s", controller_name.c_str());
  PhyUtil::FillDbSchema(unc::uppl::CTR_NAME, controller_name,
                        controller_name.length(), DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);

  // actual_Id
  PhyUtil::FillDbSchema(unc::uppl::CTR_ACTUAL_CONTROLLERID, actual_id,
                        actual_id.length(), DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);
  stringstream str_valid;
  str_valid << valid_flag;
  // valid_actual_Id
  PhyUtil::FillDbSchema(unc::uppl::CTR_VALID_ACTUAL_CONTROLLERID,
                        str_valid.str(),
                        str_valid.str().length(), DATATYPE_UINT8_ARRAY_1,
                        vect_table_attr_schema);

  kt_controller_dbtableschema.set_table_name(unc::uppl::CTR_TABLE);
  kt_controller_dbtableschema.set_primary_keys(vect_prim_keys);
  row_list.push_back(vect_table_attr_schema);
  kt_controller_dbtableschema.set_row_list(row_list);
  ODBCM_RC_STATUS update_db_status = ODBCM_RC_SUCCESS;
  update_db_status = physical_layer->get_odbc_manager()->
        UpdateOneRow((unc_keytype_datatype_t)data_type,
                  kt_controller_dbtableschema, db_conn, true);
  if (update_db_status == ODBCM_RC_SUCCESS) {
    pfc_log_info("actual id updated in DB successfully");
  } else {
    pfc_log_error("actual id update failed in DB");
    return_code = UNC_UPPL_RC_ERR_DB_UPDATE;
  }
  return return_code;
}

/**CheckDuplicateControllerId
 * @Description : This function checks for duplicate controller id in the
 *                 controller table and sends an alarm in such case.
 * @param[in]   : controller_name, actr_id, db_conn
 * @return      : UNC_RC_SUCCESS if the logical is updated
 *                or UNC_UPPL_RC_ERR_* if the update fails
 * */
UncRespCode Kt_Controller::CheckDuplicateControllerId(
    string actr_id, string ctr_name, OdbcmConnectionHandler *db_conn) {
  /* Get all the controller entry from running db */
  Kt_Controller kt_ctr;
  vector<void *> vect_ctr_key, vect_ctr_val;
  key_ctr_t key_ctr_obj;
  memset(&key_ctr_obj, 0, sizeof(key_ctr_t));
  memcpy(key_ctr_obj.controller_name, ctr_name.c_str(),
                          ctr_name.length()+1);
  // The key structure can be empty for UNC_OP_READ_SIBLING_BEGIN
  vect_ctr_key.push_back(reinterpret_cast<void *>(&key_ctr_obj));
  UncRespCode read_status = kt_ctr.ReadInternal(db_conn, vect_ctr_key,
                                                   vect_ctr_val,
                                                   UNC_DT_RUNNING,
                                                   UNC_OP_READ_SIBLING_BEGIN);
  if (read_status != UNC_RC_SUCCESS) {
    pfc_log_info("read from running db is %d", read_status);
    return read_status;
  }
  int dupid_flag = 0;  // To avoid memory leak, run thru complete for loop
  for (uint32_t ctrIndex = 0; ctrIndex < vect_ctr_key.size();
      ctrIndex ++) {
    uint32_t valid_flag = 0;
    key_ctr_t *ctr_key =
        reinterpret_cast<key_ctr_t*>(vect_ctr_key[ctrIndex]);
    string orig_controller_name = (const char*)ctr_key->controller_name;
    pfc_log_debug("controller_name: %s", orig_controller_name.c_str());
    val_ctr_st_t *obj_val_ctr =
        reinterpret_cast<val_ctr_st_t*>(vect_ctr_val[ctrIndex]);
    valid_flag = obj_val_ctr->valid[kIdxActualId];
    if (valid_flag == UNC_VF_VALID) {
      if ((actr_id == (const char*)obj_val_ctr->actual_id) &&
                     (ctr_name != orig_controller_name) && (dupid_flag == 0)) {
        dupid_flag = 1;  // Alarm send for first duplicate id is spotted
        // raise alarm for duplicate controller id to node manager
        PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
        UncRespCode alarm_status = physical_layer->get_physical_core()->
             SendDuplicateControllerIdAlarm(ctr_name, actr_id,
                                            orig_controller_name);
        if (alarm_status != UNC_RC_SUCCESS) {
          pfc_log_error("Sending duplicate controller id alarm failed.");
        }
      }
    }
    // Release memory allocated for key struct

    if (ctr_key != NULL) {
      delete ctr_key;
      ctr_key = NULL;
    }
    // delete the val memory
    if (obj_val_ctr != NULL) {
      delete obj_val_ctr;
      obj_val_ctr = NULL;
    }
  }
  return UNC_RC_SUCCESS;
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
 *                UNC_RC_SUCCESS is returned when the response
 *                is added to ipc session successfully.
 *                UNC_UPPL_RC_ERR_* is returned when ipc response 
 *                could not be added to sess.
 * */

UncRespCode Kt_Controller::SendUpdatedControllerInfoToUPLL(
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
    return UNC_UPPL_RC_ERR_LOGICAL_COMMUNICATION_FAILURE;
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
  if (err != UNC_RC_SUCCESS) {
    pfc_log_error("Error in adding parameters to session");
    return UNC_UPPL_RC_ERR_LOGICAL_COMMUNICATION_FAILURE;
  }
  pfc_ipcresp_t resp = 0;
  err = upll_cli_session.invoke(resp);
  if (err != 0 || resp != UPLL_RC_SUCCESS) {
    pfc_log_error(" Request failed to UPLL with error no: %d resp=%d",
                  err, resp);
    return UNC_UPPL_RC_ERR_LOGICAL_COMMUNICATION_FAILURE;
  }
  err = pfc_ipcclnt_altclose(connp);
  if (err != 0) {
    pfc_log_info("Unable to close ipc connection");
  }
  return UNC_RC_SUCCESS;
}

/** GetCtrValidFlag
 * @Description : This function reads the valid flag from DB
 * @param[in] : key_struct-void* to ctr key structure
 * val_ctr_valid_st-instance of val_ctr_st_t
 * @return    : Success or associated error code
 * UNC_RC_SUCCESS is returned when the response
 * is added to ipc session successfully.
 * UNC_UPPL_RC_ERR_* is returned when ipc response could not be added to sess.
 * */
UncRespCode Kt_Controller::GetCtrValidFlag(OdbcmConnectionHandler *db_conn,
                                              void *key_struct,
                                              val_ctr_st_t &val_ctr_valid_st,
                                              uint32_t data_type) {
  UncRespCode return_code = UNC_RC_SUCCESS;
  vector<void *> vect_key_ctr;
  vect_key_ctr.push_back(key_struct);
  vector<void *> vect_val_ctr;
  return_code = ReadInternal(db_conn, vect_key_ctr, vect_val_ctr,
                             data_type, UNC_OP_READ);
  if (return_code == UNC_RC_SUCCESS) {
    val_ctr_st_t *val_ctr_new_valid_st =
        reinterpret_cast<val_ctr_st_t*>(vect_val_ctr[0]);
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
 * @Description : This function converts the string value from db to uint8
 * @param[in] : attr_value-attribute value in string
 * obj_val_ctr_st-instance of val_ctr_st
 * obj_val_ctr-instance of val_ctr_t
 * @return    : void 
 * */
void Kt_Controller::FrameValidValue(string attr_value,
                                    val_ctr_st &obj_val_ctr_st,
                                    val_ctr_t &obj_val_ctr) {
  obj_val_ctr_st.valid[kIdxController] = UNC_VF_VALID;
  pfc_log_debug("valid value %d", obj_val_ctr_st.valid[kIdxController]);
  // attr_value's ascii key  value is coverted to integer by (-48)
  // attr_value is a "valid" column's value
  // type
  obj_val_ctr.valid[kIdxType] = static_cast<int>(attr_value[0] - 48);
  // version
  obj_val_ctr.valid[kIdxVersion] = static_cast<int>(attr_value[1] - 48);
  // description
  obj_val_ctr.valid[kIdxDescription] = static_cast<int>(attr_value[2] - 48);
  // ip_address
  obj_val_ctr.valid[kIdxIpAddress] = static_cast<int>(attr_value[3] - 48);
  // user
  obj_val_ctr.valid[kIdxUser] = static_cast<int>(attr_value[4] - 48);
  // password
  obj_val_ctr.valid[kIdxPassword] = static_cast<int>(attr_value[5] - 48);
  // enableAudit
  obj_val_ctr.valid[kIdxEnableAudit] = static_cast<int>(attr_value[6] - 48);
  // ActualVersion
  obj_val_ctr_st.valid[kIdxActualVersion] =
                                          static_cast<int>(attr_value[7] - 48);
  // OperStatus
  obj_val_ctr_st.valid[kIdxOperStatus] = static_cast<int>(attr_value[8] - 48);
  // port
  obj_val_ctr.valid[kIdxcPort] = static_cast<int>(attr_value[9] - 48);
  return;
}

/** FrameCVValidValue 
 * @Description : This function converts the string value from db to uint8
 * @param[in] : attr_value-attribute value in string
 * obj_cv_ctr-instance of val_ctr_commit_ver_t
 * @return    : void 
 * */
void Kt_Controller::FrameCVValidValue(string attr_value,
                    val_ctr_commit_ver_t &obj_cv_ctr) {
  if (attr_value.length() == 0) {
    obj_cv_ctr.valid[kIdxCtrCommitNumber] = UNC_VF_VALID;
    obj_cv_ctr.valid[kIdxCtrCommitDate] = UNC_VF_VALID;
    obj_cv_ctr.valid[kIdxCtrCommitApplication] = UNC_VF_VALID;
    return;
  }
  for (unsigned int index = 0; index < attr_value.length(); ++index) {
    if (attr_value[index] >= 48) {
      obj_cv_ctr.valid[index+1] = attr_value[index] - 48;
      pfc_log_debug("FrameCVValidValue: valid flag %d is %d", index,
                                      obj_cv_ctr.valid[index+1]);
    }
  }
}

/** FrameCsAttrValue
 * @Description : This function converts the string value from db to uint8
 * @param[in] : attr_value-Attribute value in string
 * obj_val_ctr-instance of val_ctr_st
 * @return    : void
 * */
void Kt_Controller::FrameCsAttrValue(string attr_value,
                                     val_ctr_t &obj_val_ctr) {
  for (unsigned int index = 0; index < 8; ++index) {
    if (attr_value[index] >= 48) {
      obj_val_ctr.cs_attr[index] = attr_value[index] - 48;
    } else {
      obj_val_ctr.cs_attr[index] = attr_value[index];
    }
  }
  return;
}

/** ValidateTypeIpAddress
 * @Description : This function checks whether controller exists with same
 * type and ip address
 * @param[in] : key_struct-void* to ctr key structure
 * val_struct-void* to ctr val structure
 * data_type-UNC_DT_*,type of datatype
 * ctrl_type-type of controller,value of unc_keytype_ctrtype_t
 * @return    : Success or associated error code
 * UNC_RC_SUCCESS is returned when the response
 * is added to ipc session successfully.
 * UNC_UPPL_RC_ERR_* is returned when ipc response could not be added to sess.
 * */
UncRespCode Kt_Controller::ValidateTypeIpAddress(
    OdbcmConnectionHandler *db_conn,
    void *key_struct,
    void *val_struct,
    uint32_t data_type,
    uint32_t ctrl_type) {
  UncRespCode status = UNC_RC_SUCCESS;
  val_ctr_t *obj_val_ctr = reinterpret_cast<val_ctr_t*>(val_struct);
  if (obj_val_ctr == NULL) {
    // Not required to validate type and ip
    pfc_log_debug("Not required to validate type and ip");
    return UNC_RC_SUCCESS;
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
    return UNC_RC_SUCCESS;
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
  CsRowStatus cs_row_status = NOTAPPLIED;
  PopulateDBSchemaForKtTable(db_conn, kt_controller_dbtableschema,
                             key_struct,
                             NULL,
                             UNC_OP_READ_SIBLING_BEGIN, data_type, 0, 0,
                             vect_key_operations, old_val_struct,
                             cs_row_status, false, PFC_FALSE);
  ODBCM_RC_STATUS read_db_status = physical_layer->get_odbc_manager()->
      GetBulkRows((unc_keytype_datatype_t)data_type, count,
                  kt_controller_dbtableschema,
                  UNC_OP_READ_SIBLING_BEGIN, db_conn);
  pfc_log_debug("Return of GetBulkRows:%d", read_db_status);
  if (read_db_status != ODBCM_RC_SUCCESS) {
    return status;
  }
  FillControllerValueStructure(db_conn, kt_controller_dbtableschema,
                               vect_val_ctr_st,
                               count,
                               UNC_OP_READ_SIBLING_BEGIN,
                               controller_id);
  pfc_log_debug("controller_id size: %"
                PFC_PFMT_SIZE_T, controller_id.size());
  vector <val_ctr_st_t>::iterator vect_val_ctr_iter = vect_val_ctr_st.begin();
  vector <string>::iterator vect_ctr_iter = controller_id.begin();
  for (; (vect_val_ctr_iter != vect_val_ctr_st.end()) &&
  (vect_ctr_iter != controller_id.end());
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
      status = UNC_UPPL_RC_ERR_CFG_SYNTAX;
      break;
    }
  }
  return status;
}

/** ValidateControllerType
 * @Description : This function checks whether the given controller type
 * is valid
 * @param[in] : operation-UNC_OP_*,type of operation
 * data_type-UNC_DT_*,type of database
 * ctr_type-type of controller(unc_keytype_ctrtype_t)
 * val_ctr-pointer to val_ctr_t
 * param[out]:ctr_type_code-Success or associated error code
 * @return    : Success or associated error code
 * UNC_RC_SUCCESS is returned when the response
 * is added to ipc session successfully.
 * UNC_UPPL_RC_ERR_* is returned when ipc response could not be added to sess.
 * */
UncRespCode Kt_Controller::ValidateControllerType(
    OdbcmConnectionHandler *db_conn,
    uint32_t operation,
    uint32_t data_type,
    unc_keytype_ctrtype_t ctr_type,
    UncRespCode ctr_type_code,
    val_ctr *val_ctr) {
  UncRespCode ret_code = UNC_RC_SUCCESS;
  pfc_bool_t mandatory = PFC_TRUE;
  // Validate value structure
  unsigned int valid_val = 0;
  // validate type
  valid_val = PhyUtil::uint8touint(val_ctr->valid[kIdxType]);
  map<string, Kt_Class_Attr_Syntax> attr_syntax_map =
      attr_syntax_map_all[UNC_KT_CONTROLLER];
  IS_VALID_INT_VALUE(CTR_TYPE_STR, val_ctr->type, operation,
                     valid_val, ret_code, mandatory);
  if (ret_code != UNC_RC_SUCCESS) {
    return UNC_UPPL_RC_ERR_CFG_SYNTAX;
  }
  if (operation == UNC_OP_UPDATE && ctr_type_code == UNC_RC_SUCCESS &&
      valid_val == UNC_VF_VALID && val_ctr->type != ctr_type) {
    pfc_log_error("type cannot be modified");
    return UNC_UPPL_RC_ERR_CFG_SYNTAX;
  }
  return UNC_RC_SUCCESS;
}

/** ValidateControllerVersion
 * @Description : This function checks whether the given controller version
 * is valid
 * @param[in] : operation-UNC_OP_*,type of operation
 * data_type-UNC_DT_*,type of database
 * ctr_type-type of controller,value of unc_keytype_ctrtype_t
 * val_ctr-pointer ctr value structure
 * param[out]:ctr_type_code-Success or associated error code
 * @return    : Success or associated error code
 * UNC_RC_SUCCESS is returned when the response
 * is added to ipc session successfully.
 * UNC_UPPL_RC_ERR_* is returned when ipc response could not be added to sess.
 * */
UncRespCode Kt_Controller::ValidateControllerVersion(
    OdbcmConnectionHandler *db_conn,
    uint32_t operation,
    uint32_t data_type,
    unc_keytype_ctrtype_t ctr_type,
    UncRespCode ctr_type_code,
    val_ctr *val_ctr) {
  UncRespCode ret_code = UNC_RC_SUCCESS;
  pfc_bool_t mandatory = PFC_TRUE;
  unsigned int valid_val = 0;
  // Version validation not required for Unknown Controller
  if ((operation == UNC_OP_UPDATE && ctr_type_code == UNC_RC_SUCCESS &&
      ctr_type != UNC_CT_UNKNOWN) ||
      (operation == UNC_OP_CREATE &&  val_ctr->type != UNC_CT_UNKNOWN)) {
    // validate version
    map<string, Kt_Class_Attr_Syntax> attr_syntax_map =
        attr_syntax_map_all[UNC_KT_CONTROLLER];
    valid_val = PhyUtil::uint8touint(val_ctr->valid[kIdxVersion]);
    string value = reinterpret_cast<char*>(val_ctr->version);
    IS_VALID_STRING_VALUE(CTR_VERSION_STR, value, operation,
                          valid_val, ret_code, mandatory);
    if (ret_code != UNC_RC_SUCCESS) {
      return UNC_UPPL_RC_ERR_CFG_SYNTAX;
    }
  }
  return UNC_RC_SUCCESS;
}

/** ValidateControllerDescription
 * @Description : This function checks whether the given controller description
 * is valid
 * @param[in] : operation-UNC_OP_*,type of operation
 * data_type-UNC_DT_*,type of database
 * ctr_type-type of controller
 * val_ctr-pointer to ctr val structure
 * param[out]:ctr_type_code-Success or associated error code
 * @return    : Success or associated error code
 * UNC_RC_SUCCESS is returned when the response
 * is added to ipc session successfully.
 * UNC_UPPL_RC_ERR_* is returned when ipc response could not be added to sess.
 * */
UncRespCode Kt_Controller::ValidateControllerDescription(
    OdbcmConnectionHandler *db_conn,
    uint32_t operation,
    uint32_t data_type,
    unc_keytype_ctrtype_t ctr_type,
    UncRespCode ctr_type_code,
    val_ctr *val_ctr) {
  UncRespCode ret_code = UNC_RC_SUCCESS;
  pfc_bool_t mandatory = PFC_TRUE;
  unsigned int valid_val = 0;
  // validate description
  valid_val = PhyUtil::uint8touint(val_ctr->valid[kIdxDescription]);
  string value = reinterpret_cast<char*>(val_ctr->description);
  map<string, Kt_Class_Attr_Syntax> attr_syntax_map =
      attr_syntax_map_all[UNC_KT_CONTROLLER];
  IS_VALID_STRING_VALUE(CTR_DESCRIPTION_STR, value, operation,
                        valid_val, ret_code, mandatory);
  if (ret_code != UNC_RC_SUCCESS) {
    return UNC_UPPL_RC_ERR_CFG_SYNTAX;
  }
  return UNC_RC_SUCCESS;
}

/** ValidateControllerIpAddress
 * @Description : This function checks whether the given controller ip address
 * is valid
 * @param[in] : operation-UNC_OP_*,type of operation
 * data_type-UNC_DT_*,type of database
 * ctr_type-type of controller,value of unc_keytype_ctrtype_t
 * val_ctr-pointer to ctr val structure
 * param[out]:ctr_type_code-Success or associated error code
 * @return    : Success or associated error code
 * UNC_RC_SUCCESS is returned when the response
 * is added to ipc session successfully.
 * UNC_UPPL_RC_ERR_* is returned when ipc response could not be added to sess.
 * */
UncRespCode Kt_Controller::ValidateControllerIpAddress(
    OdbcmConnectionHandler *db_conn,
    uint32_t operation,
    uint32_t data_type,
    unc_keytype_ctrtype_t ctr_type,
    UncRespCode ctr_type_code,
    void *key_struct,
    void *val_struct) {
  UncRespCode ret_code = UNC_RC_SUCCESS;
  pfc_bool_t mandatory = PFC_TRUE;
  unsigned int valid_val = 0;
  val_ctr *val_ctr = reinterpret_cast<val_ctr_t*>(val_struct);
  // validate IP address
  valid_val = PhyUtil::uint8touint(val_ctr->valid[kIdxIpAddress]);
  if ((operation == UNC_OP_UPDATE || UNC_OP_CREATE) &&
                              ctr_type_code == UNC_RC_SUCCESS &&
      ctr_type == UNC_CT_UNKNOWN && valid_val == UNC_VF_VALID) {
    pfc_log_error(
        "Ip address cannot be modified for unknown controller type");
    return UNC_UPPL_RC_ERR_CFG_SYNTAX;
  }
  if (operation == UNC_OP_UPDATE && ctr_type_code == UNC_RC_SUCCESS &&
      valid_val == UNC_VF_VALID) {
    UncRespCode validate_status = ValidateTypeIpAddress(db_conn, key_struct,
                                                           val_struct,
                                                           data_type,
                                                           ctr_type);
    if (validate_status != UNC_RC_SUCCESS) {
      return UNC_UPPL_RC_ERR_CFG_SYNTAX;
    }
    pfc_log_debug("Validating Type and Ip Address in Running Db");
    validate_status = ValidateTypeIpAddress(db_conn, key_struct,
                                            val_struct,
                                            UNC_DT_RUNNING,
                                            ctr_type);
    if (validate_status != UNC_RC_SUCCESS) {
      return UNC_UPPL_RC_ERR_CFG_SYNTAX;
    }
  }
  map<string, Kt_Class_Attr_Syntax> attr_syntax_map =
      attr_syntax_map_all[UNC_KT_CONTROLLER];
  IS_VALID_IPV4_VALUE(CTR_IP_ADDRESS_STR, val_ctr->ip_address, operation,
                      valid_val, ret_code, mandatory);
  if (ret_code != UNC_RC_SUCCESS) {
    return UNC_UPPL_RC_ERR_CFG_SYNTAX;
  }
  return UNC_RC_SUCCESS;
}

/** ValidateControllerUser
 * @Description : This function checks whether the given controller user
 * is valid
 * @param[in] : operation-UNC_OP_*,type of operation
 * data_type-UNC_DT_*,type of database
 * ctr_type-type of controller,value of unc_keytype_ctrtype_t
 * val_ctr-pointer to ctr val structure
 * param[out]:ctr_type_code-Success or associated error code
 * @return    : Success or associated error code
 * UNC_RC_SUCCESS is returned when the response
 * is added to ipc session successfully.
 * UNC_UPPL_RC_ERR_* is returned when ipc response could not be added to sess. 
 * */
UncRespCode Kt_Controller::ValidateControllerUser(
    OdbcmConnectionHandler *db_conn,
    uint32_t operation,
    uint32_t data_type,
    unc_keytype_ctrtype_t ctr_type,
    UncRespCode ctr_type_code,
    val_ctr *val_ctr) {
  UncRespCode ret_code = UNC_RC_SUCCESS;
  pfc_bool_t mandatory = PFC_TRUE;
  unsigned int valid_val = 0;
  // validate user
  valid_val = PhyUtil::uint8touint(val_ctr->valid[kIdxUser]);
  if ((operation == UNC_OP_UPDATE || UNC_OP_CREATE) &&
                             ctr_type_code == UNC_RC_SUCCESS &&
      ctr_type == UNC_CT_UNKNOWN && valid_val == UNC_VF_VALID) {
    pfc_log_error("User cannot be modified for unknown controller type");
    return UNC_UPPL_RC_ERR_CFG_SYNTAX;
  }
  string value = reinterpret_cast<char*>(val_ctr->user);
  map<string, Kt_Class_Attr_Syntax> attr_syntax_map =
      attr_syntax_map_all[UNC_KT_CONTROLLER];
  IS_VALID_STRING_VALUE(IPCT_USER, value, operation,
                        valid_val, ret_code, mandatory);
  if (ret_code != UNC_RC_SUCCESS) {
    return UNC_UPPL_RC_ERR_CFG_SYNTAX;
  }
  return UNC_RC_SUCCESS;
}

/** ValidateControllerPassword
 * @Description : This function checks whether the given controller password
 * is valid
 * @param[in] :operation-UNC_OP_*,type of operation
 * data_type-UNC_DT_*,type of database
 * ctr_type-type of controller,value of unc_keytype_ctrtype_t
 * val_ctr-pointer to ctr val structure
 * param[out]:ctr_type_code-Success or associated error code
 * @return    : Success or associated error code,UNC_RC_SUCCESS/ERR*
 * */
UncRespCode Kt_Controller::ValidateControllerPassword(
    OdbcmConnectionHandler *db_conn,
    uint32_t operation,
    uint32_t data_type,
    unc_keytype_ctrtype_t ctr_type,
    UncRespCode ctr_type_code,
    val_ctr *val_ctr) {
  UncRespCode ret_code = UNC_RC_SUCCESS;
  pfc_bool_t mandatory = PFC_TRUE;
  unsigned int valid_val = 0;
  // validate password
  valid_val = PhyUtil::uint8touint(val_ctr->valid[kIdxPassword]);
  if ((operation == UNC_OP_UPDATE || UNC_OP_CREATE) &&
                              ctr_type_code == UNC_RC_SUCCESS &&
      ctr_type == UNC_CT_UNKNOWN && valid_val == UNC_VF_VALID) {
    pfc_log_error("Password cannot be modified for unknown controller type");
    return UNC_UPPL_RC_ERR_CFG_SYNTAX;
  }
  string value = reinterpret_cast<char*>(val_ctr->password);
  map<string, Kt_Class_Attr_Syntax> attr_syntax_map =
      attr_syntax_map_all[UNC_KT_CONTROLLER];
  IS_VALID_STRING_VALUE(CTR_PASSWORD_STR, value, operation,
                        valid_val, ret_code, mandatory);
  if (ret_code != UNC_RC_SUCCESS) {
    return UNC_UPPL_RC_ERR_CFG_SYNTAX;
  }
  return UNC_RC_SUCCESS;
}

/** ValidateControllerEnableAudit
 * @Description : This function checks whether the given controller enable audit
 * is valid
 * @param[in] : operation-UNC_OP_*,type of operation
 * data_type-UNC_DT_*,type of database
 * ctr_type-type of controller, value of unc_keytype_ctrtype_t
 * val_ctr-pointer to ctr val structure
 * param[out]:ctr_type_code-Success or associated error code 
 * @return    : Success or associated error code,UNC_RC_SUCCESS/ERR*
 * */
UncRespCode Kt_Controller::ValidateControllerEnableAudit(
    OdbcmConnectionHandler *db_conn,
    uint32_t operation,
    uint32_t data_type,
    unc_keytype_ctrtype_t ctr_type,
    UncRespCode ctr_type_code,
    val_ctr *val_ctr) {
  UncRespCode ret_code = UNC_RC_SUCCESS;
  pfc_bool_t mandatory = PFC_TRUE;
  unsigned int valid_val = 0;
  // validate enable_audit
  valid_val = PhyUtil::uint8touint(val_ctr->valid[kIdxEnableAudit]);
  if ((operation == UNC_OP_UPDATE || UNC_OP_CREATE) &&
                             ctr_type_code == UNC_RC_SUCCESS &&
      ctr_type == UNC_CT_UNKNOWN && valid_val == UNC_VF_VALID) {
    pfc_log_error(
        "Enable audit cannot be modified for unknown controller type");
    return UNC_UPPL_RC_ERR_CFG_SYNTAX;
  }
  map<string, Kt_Class_Attr_Syntax> attr_syntax_map =
      attr_syntax_map_all[UNC_KT_CONTROLLER];
  IS_VALID_INT_VALUE(CTR_ENABLE_AUDIT_STR, val_ctr->enable_audit, operation,
                     valid_val, ret_code, mandatory);
  if (ret_code != UNC_RC_SUCCESS) {
    return UNC_UPPL_RC_ERR_CFG_SYNTAX;
  }
  return UNC_RC_SUCCESS;
}

/** ValidateControllerPort
 * @Description : This function checks whether the given controller port
 * is valid
 * @param[in] : operation-UNC_OP_*,type of operation
 * data_type-UNC_DT_*,type of database
 * ctr_type-type of controller, value of unc_keytype_ctrtype_t
 * val_ctr-pointer to ctr val structure
 * param[out]:ctr_type_code-Success or associated error code
 * @return    : Success or associated error code,UNC_RC_SUCCESS/ERR*
 * **/
UncRespCode Kt_Controller::ValidateControllerPort(
    OdbcmConnectionHandler *db_conn,
    uint32_t operation,
    uint32_t data_type,
    unc_keytype_ctrtype_t ctr_type,
    UncRespCode ctr_type_code,
    val_ctr *val_ctr) {
  UncRespCode ret_code = UNC_RC_SUCCESS;
  pfc_bool_t mandatory = PFC_TRUE;
  unsigned int valid_val = 0;
  // validate port
  valid_val = PhyUtil::uint8touint(val_ctr->valid[kIdxcPort]);
  if ((operation == UNC_OP_UPDATE || UNC_OP_CREATE) &&
                            ctr_type_code == UNC_RC_SUCCESS &&
      ctr_type == UNC_CT_UNKNOWN && valid_val == UNC_VF_VALID) {
    pfc_log_error(
        "Port cannot be modified for unknown controller type");
    return UNC_UPPL_RC_ERR_CFG_SYNTAX;
  }
  map<string, Kt_Class_Attr_Syntax> attr_syntax_map =
      attr_syntax_map_all[UNC_KT_CONTROLLER];
  IS_VALID_INT_VALUE(CTR_PORT_STR, val_ctr->port, operation,
                     valid_val, ret_code, mandatory);
  if (ret_code != UNC_RC_SUCCESS) {
    return UNC_UPPL_RC_ERR_CFG_SYNTAX;
  }
  return UNC_RC_SUCCESS;
}

/** SendOperStatusNotification
 * @Description : This function sends oper status change as notification to
 * north bound
 * @param[in] : ctr_key pointer to ctr key structure
 * old_oper_st-old oper status of controller 
 * new_oper_st-new oper status of controller
 * @return    : Success or associated error code,UNC_UPPL_RC_SUCESS/ERR*
 * */
UncRespCode Kt_Controller::SendOperStatusNotification(key_ctr_t ctr_key,
                                                         uint8_t old_oper_st,
                                                         uint8_t new_oper_st) {
  UncRespCode status = UNC_RC_SUCCESS;
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
  err |= ser_evt.addOutput(ctr_key);
  err |= ser_evt.addOutput(new_val_ctr);
  err |= ser_evt.addOutput(old_val_ctr);
  if (err != 0) {
    pfc_log_error(
        "Server Event addOutput failed, return IPC_WRITE_ERROR");
    status = UNC_UPPL_RC_ERR_IPC_WRITE_ERROR;
  } else {
    string controller_name = IpctUtil::get_string(ctr_key);
    pfc_log_debug("%s", controller_name.c_str());
    pfc_log_debug("NEW %s", (IpctUtil::get_string(new_val_ctr)).c_str());
    pfc_log_debug("OLD %s", (IpctUtil::get_string(old_val_ctr)).c_str());
    // Call IPC server to post the event
    status = (UncRespCode) physical_layer
        ->get_ipc_connection_manager()->SendEvent(&ser_evt, controller_name,
                                                 UPPL_EVENTS_KT_CONTROLLER);
  }
  return status;
}

/** CheckIpAndClearStateDB
 * @Description : This function checks the ip address and clears state db
 *                if ip address is empty
 * @param[in] : key_struct of controller
 * @return    : Success or associated error code,UNC_UPPL_RC_SUCESS/ERR*
 * */
UncRespCode Kt_Controller::CheckIpAndClearStateDB(
    OdbcmConnectionHandler *db_conn,
    void *key_struct) {
  UncRespCode state_status = UNC_RC_SUCCESS;
  vector<void *> vect_ctr_key, vect_ctr_val;
  vect_ctr_key.push_back(key_struct);
  state_status = ReadInternal(db_conn,
                              vect_ctr_key,
                              vect_ctr_val,
                              UNC_DT_RUNNING,
                              UNC_OP_READ);
  if (state_status != UNC_RC_SUCCESS) {
    pfc_log_debug("Read Internal Failed");
    return state_status;
  }
  val_ctr_st_t *ctr_val =
      reinterpret_cast<val_ctr_st_t*>(vect_ctr_val[0]);
  key_ctr_t *ctr_key = reinterpret_cast<key_ctr_t*>(vect_ctr_key[0]);
  string controller_name = "";
  if (ctr_key != NULL) {
    controller_name = reinterpret_cast<char*>(ctr_key->controller_name);
    delete ctr_key;
    ctr_key = NULL;
  }
  if (ctr_val != NULL) {
    if (ctr_val->controller.ip_address.s_addr == 0) {
      // Release all associated entries in state db
      pfc_log_info("Removing State entries for controller %s",
                   controller_name.c_str());
      ODBCM_RC_STATUS clear_status =
          PhysicalLayer::get_instance()->get_odbc_manager()->
          ClearOneInstance(UNC_DT_STATE, controller_name, db_conn);
      if (clear_status != ODBCM_RC_SUCCESS) {
        pfc_log_info("State DB clearing failed");
      }
      clear_status =
          PhysicalLayer::get_instance()->get_odbc_manager()->
          ClearOneInstance(UNC_DT_IMPORT, controller_name, db_conn);
      if (clear_status != ODBCM_RC_SUCCESS) {
        pfc_log_info("Import DB clearing failed");
      }
      // Reset actual version as empty
      string act_version = "";
      UncRespCode status = SetActualVersion(db_conn, key_struct, act_version,
                                               UNC_DT_RUNNING, UNC_VF_INVALID);
      if (status != UNC_RC_SUCCESS) {
        // log error
        pfc_log_error("act_version reset operation failed for running");
      }
      status = SetActualVersion(db_conn, key_struct,
                                act_version, UNC_DT_CANDIDATE, UNC_VF_INVALID);
      if (status != UNC_RC_SUCCESS) {
        // log error
        pfc_log_error("act_version reset operation failed for candidate");
      }

      // Reset actual id as empty
      string actual_id = "";
      status = SetActualControllerId(db_conn, key_struct, actual_id,
                        UNC_DT_RUNNING, UNC_VF_INVALID);
      if (status != UNC_RC_SUCCESS) {
        // log error
        pfc_log_error("actual_id reset operation failed for running");
      }
      status = SetActualControllerId(db_conn, key_struct, actual_id,
                        UNC_DT_CANDIDATE, UNC_VF_INVALID);
      if (status != UNC_RC_SUCCESS) {
        // log error
        pfc_log_error("actual_id reset operation failed for candidate");
      }
    }
    delete ctr_val;
    ctr_val = NULL;
  }
  return state_status;
}

/** CheckSameIp
 * @Description : This function checks whether same ip is given
 * @param[in] : key_struct-void* to ctr key structure
 * val_ctr_valid_st-instance of val_ctr_st_t
 * @return    : Success or associated error code
 * */
UncRespCode Kt_Controller::CheckSameIp(OdbcmConnectionHandler *db_conn,
                                          void *key_struct,
                                          void *val_struct,
                                          uint32_t data_type) {
  UncRespCode return_code = UNC_RC_SUCCESS;
  vector<void *> vect_key_ctr;
  vect_key_ctr.push_back(key_struct);
  vector<void *> vect_val_ctr;
  val_ctr_t ctr_val = *reinterpret_cast<val_ctr_t*>(val_struct);
  uint32_t existing_ip = ctr_val.ip_address.s_addr;
  return_code = ReadInternal(db_conn, vect_key_ctr, vect_val_ctr,
                             data_type, UNC_OP_READ);
  if (return_code == UNC_RC_SUCCESS) {
    val_ctr_st_t *val_ctr_new_valid_st =
        reinterpret_cast<val_ctr_st_t*>(vect_val_ctr[0]);
    if (val_ctr_new_valid_st != NULL) {
      uint32_t in_ip = val_ctr_new_valid_st->controller.ip_address.s_addr;
      if (existing_ip == in_ip) {
        pfc_log_info("Same Ip given - Ignore");
      } else {
        pfc_log_info("Different Ip given - Throw Error");
        return_code = UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED;
      }
      delete val_ctr_new_valid_st;
      val_ctr_new_valid_st = NULL;
      key_ctr_t *ctr_key = reinterpret_cast<key_ctr_t*>
      (vect_key_ctr[0]);
      if (ctr_key != NULL) {
        delete ctr_key;
        ctr_key = NULL;
      }
    }
  }
  return return_code;
}

/** ValidateControllerCount 
 * @Description : This function fetech and return the rowcount of 
 * c_controller_table.
 * @param[in] : key_struct-void* to ctr key structure
 * val_ctr_valid_st-instance of val_ctr_st_ta, db_conn and data_type
 * @param[out]: count
 * @return    : Success or associated error code
 * */
UncRespCode Kt_Controller::ValidateControllerCount(
                     OdbcmConnectionHandler *db_conn,
                     void* key_struct, void* val_struct, uint32_t data_type) {
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  ODBCM_RC_STATUS response_status = ODBCM_RC_SUCCESS;
  uint32_t count = 0;

  string table_name(physical_layer->get_odbc_manager()-> \
      GetTableName(unc::uppl::CTR_TABLE));
  pfc_log_debug("ValidateControllerCount - table_name is %s",
                         table_name.c_str());

  // Send request to ODBC for controller_table
  response_status = physical_layer->get_odbc_manager()-> \
      GetRowCount((unc_keytype_datatype_t)data_type,
                      table_name, count,
                      db_conn);

  if (response_status == ODBCM_RC_CONNECTION_ERROR) {
    // log error to log daemon
    pfc_log_error("DB connection not available or cannot access DB");
    return UNC_UPPL_RC_ERR_DB_ACCESS;
  } else if (response_status == ODBCM_RC_SUCCESS) {
    if ((uint32_t)count >= 1) {
      pfc_log_debug("ValidateControllerCount - Count from DB: %d", count);
      pfc_log_error("ctr count is exceeded for kt_controller in coexists mode");
      return UNC_UPPL_RC_ERR_EXCEEDS_RESOURCE_LIMIT;
    }
  } else {
    pfc_log_error("Unable to get row count from DB, error is %d",
                  response_status);
    return UNC_UPPL_RC_ERR_DB_GET;
  }
  return UNC_RC_SUCCESS;
}
/** CheckAuditFlag 
 * @Description : This function checks the audit enable value
 * and put it in the out parameter.
 * @param[in] : db_conn - points to Db Connection Handler
 *            : kt_ctr - object of key_ctr
 * @param[out]: audit_flag: value of attribute 'audit_enable' of the controller
 * @return    : Success or associated error code
 * */
UncRespCode Kt_Controller::CheckAuditFlag(OdbcmConnectionHandler *db_conn,
                                          key_ctr_t kt_ctr,
                                          uint8_t &audit_flag) {
  vector<void *> vect_key_ctr;
  vect_key_ctr.push_back(&kt_ctr);
  vector<void *> vect_val_ctr;
  UncRespCode return_code = ReadInternal(db_conn, vect_key_ctr,
                                         vect_val_ctr,
                                         UNC_DT_RUNNING,
                                         UNC_OP_READ);
  if (return_code == UNC_RC_SUCCESS) {
    val_ctr_st_t *ctr_val_st = reinterpret_cast<val_ctr_st_t*>(vect_val_ctr[0]);
    if (ctr_val_st != NULL) {
      audit_flag = ctr_val_st->controller.enable_audit;
      delete ctr_val_st;
      ctr_val_st = NULL;
    }
    key_ctr_t *ctr_key = reinterpret_cast<key_ctr_t*>
    (vect_key_ctr[0]);
    if (ctr_key != NULL) {
      delete ctr_key;
      ctr_key = NULL;
    }
  }
  pfc_log_debug("audit flag parameter %d :", audit_flag);
  return return_code;
}

/** ValKtCtrAttributeSupportCheck
 * @Description : This function performs port Attribute validation for
 * UNC_KT_CONTROLLER
 * value_struct - the value structure of kt_controller instance
 * @param - Port attribute (*attr)
 * operation_type - UNC_OP*, type of operation
 * @return    : UNC_RC_SUCCESS is returned when the validation is failure
 * UNC_UPPL_RC_ERR_* is returned when validation is success
 * */
UncRespCode Kt_Controller::ValKtCtrAttributeSupportCheck(
                                        val_ctr_t *obj_val_ctr,
                                        const val_ctr_t *db_ctr_val,
                                        const uint8_t *attrs,
                                        unc_keytype_operation_t op_type) {
  if (op_type == UNC_OP_UPDATE) {
    if (db_ctr_val == NULL) return UNC_UPPL_RC_ERR_CFG_SEMANTIC;
    if (db_ctr_val->valid[kIdxcPort] == UNC_VF_INVALID &&
          obj_val_ctr->valid[kIdxcPort] == UNC_VF_VALID_NO_VALUE) {
      obj_val_ctr->valid[kIdxcPort] = UNC_VF_INVALID;
      return UNC_RC_SUCCESS;
    }
  }
  if (obj_val_ctr != NULL && obj_val_ctr->valid[kIdxcPort] == UNC_VF_VALID) {
      // || (obj_val_ctr->valid[kIdxcPort] == UNC_VF_VALID_NO_VALUE))) {
    pfc_log_debug("inside the port check incoming valid flag=%d"
        , obj_val_ctr->valid[kIdxcPort]);
    if (attrs[unc::capa::controller::kPort] == PFC_FALSE) {
      obj_val_ctr->valid[kIdxcPort] = UNC_VF_INVALID;
      if (op_type == UNC_OP_CREATE || op_type == UNC_OP_UPDATE) {
        pfc_log_info("Port attr is not supported by ctrlr ");
        return UNC_UPPL_RC_ERR_CFG_SEMANTIC;
      }
    }
  }
  return UNC_RC_SUCCESS;
}
