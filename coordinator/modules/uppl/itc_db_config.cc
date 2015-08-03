/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/**
 * @brief    Itc DbConfig
 * @file     itc_db_config.cc
 *
 */


#include "itc_db_config.hh"
#include "physicallayer.hh"
#include "itc_kt_controller.hh"

/**DBConfigurationRequest
 * @Description    : This function initializes the member data
 * @param[in]      : None
 * @Return         : None
 * */
DBConfigurationRequest::DBConfigurationRequest() {
}

/**~DBConfigurationRequest
 * @Description    : This function releases memory allocated
 *                   to pointer member data
 * @param[in]      : None
 * @Return         : None
 * */
DBConfigurationRequest::~DBConfigurationRequest() {
}

/**LoadAndCommitStartup
 * @Description    : This function copy the entries from StartUp to Candidate
 *                   DB and from Candidate to running DB and
 *                   clears State DB entries
 * @param[in]      : None
 * @Return         : UNC_RC_SUCCESS if Load and Commit Startup is successful
 *                   or UNC_UPPL_RC_ERR_* if failure occurs
 * */
UncRespCode DBConfigurationRequest::LoadAndCommitStartup(
    OdbcmConnectionHandler *db_conn) {
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  ODBCM_RC_STATUS copy_db_status = ODBCM_RC_SUCCESS;
  UncRespCode result_code = UNC_RC_SUCCESS;
  /* copy StartUp to Candidate */
  // if unc is running in coexists mode
  UncMode unc_mode = PhysicalLayer::get_instance()->\
                        get_physical_core()->getunc_mode();
  if (unc_mode != UNC_COEXISTS_MODE) {
  copy_db_status = physical_layer->get_odbc_manager()->
      CopyDatabase(UNC_DT_STARTUP, UNC_DT_CANDIDATE, db_conn);
  if (copy_db_status != ODBCM_RC_SUCCESS) {
    if (copy_db_status == ODBCM_RC_CONNECTION_ERROR) {
      // log fatal error to log daemon
      UPPL_LOG_FATAL("DB connection not available or cannot access DB");
      result_code = UNC_UPPL_RC_ERR_DB_ACCESS;
    } else {
      // log error to log daemon
      string log_msg = string("LoadAndCommitStartup:")+
          "Copying the StartUp to Candidate database failed";
      pfc_log_error(log_msg.c_str());
      result_code = UNC_UPPL_RC_ERR_COPY_STARTUP_TO_CANDID;
    }
    return result_code;
  }
  } else {
  // Clear Candidate entries
  ODBCM_RC_STATUS clear_status =
      PhysicalLayer::get_instance()->get_odbc_manager()->
      ClearDatabase(UNC_DT_CANDIDATE, db_conn);
  if (clear_status != ODBCM_RC_SUCCESS) {
    pfc_log_info("Candidate DB clearing failed");
    result_code = UNC_UPPL_RC_ERR_CLEAR_DB;
    return result_code;
  }
  }
  // Clear Running entries
  ODBCM_RC_STATUS clear_status =
      PhysicalLayer::get_instance()->get_odbc_manager()->
      ClearDatabase(UNC_DT_RUNNING, db_conn);
  if (clear_status != ODBCM_RC_SUCCESS) {
    pfc_log_info("Running DB clearing failed");
    result_code = UNC_UPPL_RC_ERR_CLEAR_DB;
    return result_code;
  }
  // Clear Import Database entries
  clear_status = PhysicalLayer::get_instance()->get_odbc_manager()->
      ClearDatabase(UNC_DT_IMPORT, db_conn);
  if (clear_status != ODBCM_RC_SUCCESS) {
    pfc_log_info("Import DB clearing failed");
    result_code = UNC_UPPL_RC_ERR_CLEAR_DB;
    return result_code;
  }
  if (unc_mode != UNC_COEXISTS_MODE) {
  /* copy Candidate to Running */
  copy_db_status = physical_layer->get_odbc_manager()->
      CopyDatabase(UNC_DT_CANDIDATE, UNC_DT_RUNNING, db_conn);
  if (copy_db_status != ODBCM_RC_SUCCESS) {
    if (copy_db_status == ODBCM_RC_CONNECTION_ERROR) {
      // log fatal error to log daemon
      string log_msg = string("LoadAndCommitStartup:")+
          "DB connection not available or cannot access DB";
      UPPL_LOG_FATAL(log_msg.c_str());
      result_code = UNC_UPPL_RC_ERR_DB_ACCESS;
    } else {
      // log error to log daemon
      string log_msg = string("LoadAndCommitStartup:")+
          "copying the Candidate to Running database failed";
      pfc_log_error(log_msg.c_str());
      result_code = UNC_UPPL_RC_ERR_COPY_CANDID_TO_RUNNING;
    }
    return result_code;
  }
  }
  // Clear State entries
  clear_status =
      PhysicalLayer::get_instance()->get_odbc_manager()->
      ClearDatabase(UNC_DT_STATE, db_conn);
  if (clear_status != ODBCM_RC_SUCCESS) {
    pfc_log_info("State DB clearing failed");
    result_code = UNC_UPPL_RC_ERR_CLEAR_DB;
  }
  return result_code;
}

/**CopyRunningtoCandidate
 * @Description    : This function copy the entries from Running to Candidate
 *                   when Startup is invalid
 * @param[in]      : None
 * @Return         : UNC_RC_SUCCESS if copy is successful
 *                   or UNC_UPPL_RC_ERR_* if failure occurs
 * */
UncRespCode DBConfigurationRequest::CopyRunningtoCandidate(
    OdbcmConnectionHandler *db_conn) {
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  ODBCM_RC_STATUS copy_db_status = ODBCM_RC_SUCCESS;
  UncRespCode result_code = UNC_RC_SUCCESS;
  /* copy StartUp to Candidate */
  copy_db_status = physical_layer->get_odbc_manager()->
      CopyDatabase(UNC_DT_RUNNING, UNC_DT_CANDIDATE, db_conn);
  if (copy_db_status != ODBCM_RC_SUCCESS) {
    if (copy_db_status == ODBCM_RC_CONNECTION_ERROR) {
      // log fatal error to log daemon
      UPPL_LOG_FATAL("DB connection not available or cannot access DB");
      result_code = UNC_UPPL_RC_ERR_DB_ACCESS;
    } else {
      // log error to log daemon
      string log_msg = string("StartupInvalidCopyRunningtoCandidate:")+
          "Copying the Running to Candidate database failed";
      pfc_log_error(log_msg.c_str());
      result_code = UNC_UPPL_RC_ERR_COPY_RUNNING_TO_CANDID;
    }
    return result_code;
  }
  return result_code;
}

/**ClearStartUpDb()
 * @Description    : This function clears the content of Startup Database
 * @param[in]      : None
 * @Return         : UNC_RC_SUCCESS if Clearing the startup DB is successful 
 *                   or UNC_UPPL_RC_ERR_* in case of failure
 * */
UncRespCode DBConfigurationRequest::ClearStartUpDb(
    OdbcmConnectionHandler *db_conn) {
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  UncRespCode result_code = UNC_RC_SUCCESS;
  ODBCM_RC_STATUS clear_db_status;
  /* clear the StartUp db */
  clear_db_status = physical_layer->get_odbc_manager()->
      ClearDatabase(UNC_DT_STARTUP, db_conn);
  if (clear_db_status != ODBCM_RC_SUCCESS) {
    if (clear_db_status == ODBCM_RC_CONNECTION_ERROR) {
      // log fatal error to log daemon
      UPPL_LOG_FATAL("DB connection not available or cannot access DB");
      result_code = UNC_UPPL_RC_ERR_DB_ACCESS;
    } else {
      // log error to log daemon
      pfc_log_error("ClearStartUpDb:Clearing the StartUp database failed");
      result_code = UNC_UPPL_RC_ERR_CLEAR_DB;
    }
  }
  return result_code;
}

/**AbortCandidateDb
 * @Description : This function modified controller to logical and rollbacks
 *                the uncommitted transaction  from the candidate database.
 * @param[in]   : None
 * @Return      : UNC_RC_SUCCESS if AbortCandidate database is successful
 *                or UNC_UPPL_RC_ERR_* in case of failure
 * */
UncRespCode DBConfigurationRequest::AbortCandidateDb(
    OdbcmConnectionHandler *db_conn,
    TcConfigMode config_mode) {
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  // If config_mode is not real-network or Global mode, return success
  if ((config_mode != TC_CONFIG_REAL) &&
                       (config_mode != TC_CONFIG_GLOBAL)) {
    return UNC_RC_SUCCESS;
  }
  UncRespCode result_code = UNC_RC_SUCCESS;
  result_code = SendDeletedControllerToLogical(db_conn);
  if (result_code != UNC_RC_SUCCESS) {
    return result_code;
  }
  result_code = SendCreatedControllerToLogical(db_conn);
  if (result_code != UNC_RC_SUCCESS) {
    return result_code;
  }
  result_code = SendUpdatedControllerToLogical(db_conn);
  if (result_code != UNC_RC_SUCCESS) {
    return result_code;
  }
  /* copy running database to candidate database */
  ODBCM_RC_STATUS copy_db_status = physical_layer->get_odbc_manager()->
      CopyDatabase(UNC_DT_RUNNING, UNC_DT_CANDIDATE, db_conn);
  if (copy_db_status != ODBCM_RC_SUCCESS) {
    if (copy_db_status == ODBCM_RC_CONNECTION_ERROR) {
      // log fatal error to log daemon
      UPPL_LOG_FATAL("DB connection not available or cannot access DB");
      result_code = UNC_UPPL_RC_ERR_DB_ACCESS;
    } else {
      /*  log error to log  daemon */
      pfc_log_error("copy running to candidate database failed");
      result_code = UNC_UPPL_RC_ERR_COPY_RUNNING_TO_CANDID;
    }
  }
  result_code = SendAppliedControllerToLogical(db_conn);
  if (result_code != UNC_RC_SUCCESS) {
    return result_code;
  }
  return result_code;
}

/**SaveRunningToStartUp
 * @Description : This function send request to ODBCManager to sync running
 *                with StartUp configuration
 * @param[in]   : None
 * @Return      : UNC_RC_SUCCESS if saving running to startup database is
 *                successful or UNC_UPPL_RC_ERR_* in case of failure
 * */
UncRespCode DBConfigurationRequest::SaveRunningToStartUp(
    OdbcmConnectionHandler *db_conn)  {
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  UncRespCode result_code = UNC_RC_SUCCESS;
  ODBCM_RC_STATUS copy_db_status;

  /* Copy running database to StartUp database */
  copy_db_status = physical_layer->get_odbc_manager()->
      CopyDatabase(UNC_DT_RUNNING, UNC_DT_STARTUP, db_conn);
  if (copy_db_status != ODBCM_RC_SUCCESS) {
    if (copy_db_status == ODBCM_RC_CONNECTION_ERROR) {
      // log fatal error to log daemon
      UPPL_LOG_FATAL("DB connection not available or cannot access DB");
      result_code = UNC_UPPL_RC_ERR_DB_ACCESS;
    } else {
      /* log  error  to log manager */
      pfc_log_error("copy running database to StartUp database failed");
      result_code = UNC_UPPL_RC_ERR_COPY_RUNNING_TO_START;
    }
  }
  return result_code;
}

/**SendDeletedControllerToLogical
 * @Description : This function send the deleted controller to logical
 *                during abort candidate db/ abort transaction
 * @param[in]   : None
 * @Return      : UNC_RC_SUCCESS if sending the controller information to
 *                logical is successful or UNC_UPPL_RC_ERR_* in case of failure 
 * */
UncRespCode DBConfigurationRequest::SendDeletedControllerToLogical(
    OdbcmConnectionHandler *db_conn) {
  Kt_Controller kt_ctr;
  vector<void*> vec_key_ctr_modified;
  // Getting the created Configuration from the database
  UncRespCode result_code = kt_ctr.GetModifiedRows(
      db_conn, vec_key_ctr_modified,
      CREATED);
  if (result_code != UNC_RC_SUCCESS) {
    // No created controller available
    return UNC_RC_SUCCESS;
  }
  for (uint32_t config_count = 0; config_count != vec_key_ctr_modified.size();
      config_count++) {
    key_ctr_t *ctr_key = reinterpret_cast<key_ctr_t *>
    (vec_key_ctr_modified[config_count]);
    // Sending the Controller Info. to Logical Layer with Delete Operation
    result_code = kt_ctr.SendUpdatedControllerInfoToUPLL(
        UNC_DT_CANDIDATE,
        UNC_OP_DELETE,
        UNC_KT_CONTROLLER,
        vec_key_ctr_modified[config_count],
        NULL);
    //  Freeing the Memory allocated in controller class
    if (ctr_key != NULL) {
      delete ctr_key;
      ctr_key = NULL;
    }
    if (result_code != UNC_RC_SUCCESS) {
      pfc_log_error("Failed to send the info to UPLL of controller");
      return UNC_UPPL_RC_ERR_LOGICAL_COMMUNICATION_FAILURE;
    }
  }
  return UNC_RC_SUCCESS;
}

/**SendCreatedControllerToLogical
 * @Description : This function send the created controller
 *                to logical during abort candidate db/ abort transaction
 * @param[in]   : None
 * @Return      : UNC_RC_SUCCESS if sending the controller information to
 *                logical is successful or UNC_UPPL_RC_ERR_* in case of failure
 * */
UncRespCode DBConfigurationRequest::SendCreatedControllerToLogical(
    OdbcmConnectionHandler *db_conn) {
  Kt_Controller kt_ctr;
  vector<void*> vec_key_ctr_modified;
  // Getting the deleted Configuration from the database
  UncRespCode result_code = kt_ctr.GetModifiedRows(
      db_conn, vec_key_ctr_modified,
      DELETED);
  if (result_code != UNC_RC_SUCCESS) {
    // No deleted controller available
    return UNC_RC_SUCCESS;
  }

  for (uint32_t config_count = 0;
      config_count != vec_key_ctr_modified.size(); config_count++) {
    vector<void *> vect_ctr_key, vect_ctr_val;
    key_ctr_t *ctr_key = reinterpret_cast<key_ctr_t*>
    (vec_key_ctr_modified[config_count]);
    pfc_log_debug("Controller Name %s", ctr_key->controller_name);
    vect_ctr_key.push_back(vec_key_ctr_modified[config_count]);
    if (kt_ctr.ReadInternal(db_conn, vect_ctr_key,
                            vect_ctr_val,
                            UNC_DT_RUNNING,
                            UNC_OP_READ) != UNC_RC_SUCCESS) {
      delete ctr_key;
      ctr_key = NULL;
      continue;
    }
    val_ctr_st_t *val_ctr_st =
        reinterpret_cast<val_ctr_st_t*>(vect_ctr_val[0]);
    if (val_ctr_st == NULL) {
      delete ctr_key;
      ctr_key = NULL;
      continue;
    }
    // Sending the Controller Info. to Logical Layer with CREATE Operation
    result_code = kt_ctr.SendUpdatedControllerInfoToUPLL(
        UNC_DT_CANDIDATE,
        UNC_OP_CREATE,
        UNC_KT_CONTROLLER,
        vec_key_ctr_modified[config_count],
        reinterpret_cast<void*>(&val_ctr_st->controller));
    //  Freeing the Memory allocated in controller class
    key_ctr_t *ctr_key_val = reinterpret_cast<key_ctr_t*>
    (vect_ctr_key[0]);
    if (ctr_key_val != NULL) {
      delete ctr_key_val;
      ctr_key_val = NULL;
    }
    delete val_ctr_st;
    val_ctr_st = NULL;
    delete ctr_key;
    ctr_key = NULL;
    if (result_code != UNC_RC_SUCCESS) {
      pfc_log_error("Failed to send the info to UPLL of controller");
      return UNC_UPPL_RC_ERR_LOGICAL_COMMUNICATION_FAILURE;
    }
  }
  return UNC_RC_SUCCESS;
}

/**SendUpdatedControllerToLogical
 * @Description : This function send the updated controller to
 *                logical during abort candidate db/ abort transaction
 * @param[in]   : None
 * @Return      : UNC_RC_SUCCESS if sending the controller information to
 *                logical is successful or UNC_UPPL_RC_ERR_* in case of failure 
 * */
UncRespCode DBConfigurationRequest::SendUpdatedControllerToLogical(
    OdbcmConnectionHandler *db_conn) {
  Kt_Controller kt_ctr;
  vector<void*> vec_key_ctr_modified;
  // Getting the Update Configuration from the database
  UncRespCode result_code = kt_ctr.GetModifiedRows(
      db_conn, vec_key_ctr_modified,
      UPDATED);
  if (result_code != UNC_RC_SUCCESS) {
    // No updated controller available
    return UNC_RC_SUCCESS;
  }
  // Iterating the updated controller vector
  for (uint32_t config_count = 0; \
  config_count != vec_key_ctr_modified.size(); config_count++) {
    // Getting the val struct from old values Running Database
    vector<void *> vect_ctr_key, vect_ctr_val;
    key_ctr_t *ctr_key = reinterpret_cast<key_ctr_t*>
    (vec_key_ctr_modified[config_count]);
    pfc_log_debug("Controller Name %s", ctr_key->controller_name);
    vect_ctr_key.push_back(vec_key_ctr_modified[config_count]);
    if (kt_ctr.ReadInternal(db_conn, vect_ctr_key,
                            vect_ctr_val,
                            UNC_DT_RUNNING,
                            UNC_OP_READ) != UNC_RC_SUCCESS) {
      delete ctr_key;
      ctr_key = NULL;
      continue;
    }
    val_ctr_st_t *val_ctr_st =
        reinterpret_cast<val_ctr_st_t*>(vect_ctr_val[0]);
    if (val_ctr_st == NULL) {
      delete ctr_key;
      ctr_key = NULL;
      continue;
    }
    // Sending the Controller Info.to Logical Layer with Update Operation
    result_code = kt_ctr.SendUpdatedControllerInfoToUPLL(
        UNC_DT_CANDIDATE,
        UNC_OP_UPDATE,
        UNC_KT_CONTROLLER,
        vec_key_ctr_modified[config_count],
        reinterpret_cast<void*>(&val_ctr_st->controller));
    //  Freeing the Memory allocated in controller class
    key_ctr_t *ctr_key_val = reinterpret_cast<key_ctr_t*>
    (vect_ctr_key[0]);
    if (ctr_key_val != NULL) {
      delete ctr_key_val;
      ctr_key_val = NULL;
    }
    delete val_ctr_st;
    val_ctr_st = NULL;
    delete ctr_key;
    ctr_key = NULL;
    if (result_code != UNC_RC_SUCCESS) {
      pfc_log_error("Failed to send the info to UPLL of controller");
      return UNC_UPPL_RC_ERR_LOGICAL_COMMUNICATION_FAILURE;
    }
  }
  return UNC_RC_SUCCESS;
}

/**SendAppliedControllerToLogical
 * @Description    : This function send the applied controller to
 *                   logical during abort candidate db/ abort transaction
 * @param[in]      : None
 * @Return         : UNC_RC_SUCCESS if sending the controller information to
 *                   logical is successful or UNC_UPPL_RC_ERR_* in case of failure
 * */
UncRespCode DBConfigurationRequest::SendAppliedControllerToLogical(
    OdbcmConnectionHandler *db_conn) {
  Kt_Controller kt_ctr;
  vector<void*> vec_key_ctr_modified;
  // Getting the deleted Configuration from the database
  UncRespCode result_code = kt_ctr.GetModifiedRows(
      db_conn, vec_key_ctr_modified,
      APPLIED);
  if (result_code != UNC_RC_SUCCESS) {
    // No deleted controller available
    return UNC_RC_SUCCESS;
  }

  for (uint32_t config_count = 0;
      config_count != vec_key_ctr_modified.size(); config_count++) {
    vector<void *> vect_ctr_key, vect_ctr_val;
    key_ctr_t *ctr_key = reinterpret_cast<key_ctr_t*>
    (vec_key_ctr_modified[config_count]);
    pfc_log_debug("Controller Name %s", ctr_key->controller_name);
    vect_ctr_key.push_back(vec_key_ctr_modified[config_count]);
    if (kt_ctr.ReadInternal(db_conn, vect_ctr_key,
                            vect_ctr_val,
                            UNC_DT_RUNNING,
                            UNC_OP_READ) != UNC_RC_SUCCESS) {
      delete ctr_key;
      ctr_key = NULL;
      continue;
    }
    val_ctr_st_t *val_ctr_st =
        reinterpret_cast<val_ctr_st_t*>(vect_ctr_val[0]);
    if (val_ctr_st == NULL) {
      delete ctr_key;
      ctr_key = NULL;
      continue;
    }
    // Sending the Controller Info. to Logical Layer with CREATE Operation
    result_code = kt_ctr.SendUpdatedControllerInfoToUPLL(
        UNC_DT_CANDIDATE,
        UNC_OP_CREATE,
        UNC_KT_CONTROLLER,
        vec_key_ctr_modified[config_count],
        reinterpret_cast<void*>(&val_ctr_st->controller));
    //  Freeing the Memory allocated in controller class
    key_ctr_t *ctr_key_val = reinterpret_cast<key_ctr_t*>
    (vect_ctr_key[0]);
    if (ctr_key_val != NULL) {
      delete ctr_key_val;
      ctr_key_val = NULL;
    }
    delete val_ctr_st;
    val_ctr_st = NULL;
    delete ctr_key;
    ctr_key = NULL;
    if (result_code != UNC_RC_SUCCESS) {
      pfc_log_error("Failed to send the info to UPLL of controller");
      return UNC_UPPL_RC_ERR_LOGICAL_COMMUNICATION_FAILURE;
    }
  }
  return UNC_RC_SUCCESS;
}
