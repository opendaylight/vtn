/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/**
 * @brief    Itc
 * @file     itc_db_config.cc
 *
 */


#include "itc_db_config.hh"
#include "physicallayer.hh"
#include "itc_kt_controller.hh"

/**DBConfigurationRequest()
 * @Description    : Constructor which will initializes the member data
 * @param[in]      : None
 * @Return         : void
 * **/
DBConfigurationRequest::DBConfigurationRequest() {
}

/** ~DBConfigurationRequest()
 * @Description    : Destructor to release any memory allocated to pointer member data
 * @param[in]      : None
 * @Return         : void
 * **/
DBConfigurationRequest::~DBConfigurationRequest() {
}

/**LoadAndCommitStartup()
 * @Description    : This function will call the ClearDatabase() and then send
 *                   request to ODBCManager to copy StartUp to Candidate
 *                   configuration
 * @param[in]      : None
 * @Return         : void
 * **/
UpplReturnCode DBConfigurationRequest::LoadAndCommitStartup() {
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  ODBCM_RC_STATUS copy_db_status = ODBCM_RC_SUCCESS;
  UpplReturnCode result_code = UPPL_RC_SUCCESS;
  /* copy StartUp to Candidate */
  copy_db_status = physical_layer->get_odbc_manager()->
      CopyDatabase(UNC_DT_STARTUP, UNC_DT_CANDIDATE);
  if (copy_db_status != ODBCM_RC_SUCCESS) {
    if (copy_db_status == ODBCM_RC_CONNECTION_ERROR) {
      // log fatal error to log daemon
      pfc_log_fatal("DB connection not available or cannot access DB");
      result_code = UPPL_RC_ERR_DB_ACCESS;
    } else {
      // log error to log daemon
      string log_msg = string("LoadAndCommitStartup:")+
          "Copying the StartUp to Candidate database failed";
      pfc_log_error(log_msg.c_str());
      result_code = UPPL_RC_ERR_COPY_STARTUP_TO_CANDID;
    }
    return result_code;
  }
  /* copy Candidate to Running */
  copy_db_status = physical_layer->get_odbc_manager()->
      CopyDatabase(UNC_DT_CANDIDATE, UNC_DT_RUNNING);
  if (copy_db_status != ODBCM_RC_SUCCESS) {
    if (copy_db_status == ODBCM_RC_CONNECTION_ERROR) {
      // log fatal error to log daemon
      string log_msg = string("LoadAndCommitStartup:")+
          "DB connection not available or cannot access DB";
      pfc_log_fatal(log_msg.c_str());
      result_code = UPPL_RC_ERR_DB_ACCESS;
    } else {
      // log error to log daemon
      string log_msg = string("LoadAndCommitStartup:")+
          "copying the Candidate to Running database failed";
      pfc_log_error(log_msg.c_str());
      result_code = UPPL_RC_ERR_COPY_CANDID_TO_RUNNING;
    }
    return result_code;
  }
  // Clear State entries
  ODBCM_RC_STATUS clear_status =
      PhysicalLayer::get_instance()->get_odbc_manager()->
      ClearDatabase(UNC_DT_STATE);
  if (clear_status != ODBCM_RC_SUCCESS) {
    pfc_log_info("State DB clearing failed");
    result_code = UPPL_RC_ERR_CLEAR_DB;
  }
  return result_code;
}

/** ClearStartUpDb()
 * @Description    : This function will clear the contents of clearStartup
 *                   Database
 * @param[in]      : None
 * @Return         : UpplReturnCode(enum)
 * **/
UpplReturnCode DBConfigurationRequest::ClearStartUpDb() {
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  UpplReturnCode result_code = UPPL_RC_SUCCESS;
  ODBCM_RC_STATUS clear_db_status;
  /* clear the StartUp db */
  clear_db_status = physical_layer->get_odbc_manager()->
      ClearDatabase(UNC_DT_STARTUP);
  if (clear_db_status != ODBCM_RC_SUCCESS) {
    if (clear_db_status == ODBCM_RC_CONNECTION_ERROR) {
      // log fatal error to log daemon
      pfc_log_fatal("DB connection not available or cannot access DB");
      result_code = UPPL_RC_ERR_DB_ACCESS;
    } else {
      // log error to log daemon
      pfc_log_error("ClearStartUpDb:Clearing the StartUp database failed");
      result_code = UPPL_RC_ERR_CLEAR_DB;
    }
  }
  return result_code;
}

/**ClearAllDb()
 * @Description    : This function will clear the contents of all the databases
 * @param[in]      : None
 * @Return         : UpplReturnCode(enum)
 * **/
UpplReturnCode DBConfigurationRequest::ClearAllDb() {
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  UpplReturnCode result_code = UPPL_RC_SUCCESS;
  ODBCM_RC_STATUS clear_db_status;

  /* Clear all the databases */
  clear_db_status = physical_layer->get_odbc_manager()->
      ClearDatabase(UNC_DT_CANDIDATE);
  if (clear_db_status != ODBCM_RC_SUCCESS) {
    if (clear_db_status == ODBCM_RC_CONNECTION_ERROR) {
      // log fatal error to log daemon
      pfc_log_fatal("DB connection not available or cannot access DB");
      result_code = UPPL_RC_ERR_DB_ACCESS;
    } else {
      // log error to log daemon
      pfc_log_error("ClearStartUpDb:Clearing the Candidate database failed");
      result_code = UPPL_RC_ERR_CLEAR_DB;
    }
    return result_code;
  }
  clear_db_status = physical_layer->get_odbc_manager()->
      ClearDatabase(UNC_DT_RUNNING);
  if (clear_db_status != ODBCM_RC_SUCCESS) {
    if (clear_db_status == ODBCM_RC_CONNECTION_ERROR) {
      // log fatal error to log daemon
      pfc_log_fatal("DB connection not available or cannot access DB");
      result_code = UPPL_RC_ERR_DB_ACCESS;
    } else {
      // log error to log daemon
      pfc_log_error("ClearStartUpDb:Clearing the Running database failed");
      result_code = UPPL_RC_ERR_CLEAR_DB;
    }
    return result_code;
  }

  clear_db_status = physical_layer->get_odbc_manager()->
      ClearDatabase(UNC_DT_IMPORT);
  if (clear_db_status != ODBCM_RC_SUCCESS) {
    if (clear_db_status == ODBCM_RC_CONNECTION_ERROR) {
      // log fatal error to log daemon
      pfc_log_fatal("DB connection not available or cannot access DB");
      result_code = UPPL_RC_ERR_DB_ACCESS;
    } else {
      // log error to log daemon
      pfc_log_error("ClearStartUpDb:Clearing the Import database failed");
      result_code = UPPL_RC_ERR_CLEAR_DB;
    }
    return result_code;
  }

  clear_db_status = physical_layer->get_odbc_manager()->
      ClearDatabase(UNC_DT_STARTUP);
  if (clear_db_status != ODBCM_RC_SUCCESS) {
    if (clear_db_status == ODBCM_RC_CONNECTION_ERROR) {
      // log fatal error to log daemon
      pfc_log_fatal("DB connection not available or cannot access DB");
      result_code = UPPL_RC_ERR_DB_ACCESS;
    } else {
      // log error to log daemon
      pfc_log_error("ClearStartUpDb:Clearing the StartUp database failed");
      result_code = UPPL_RC_ERR_CLEAR_DB;
    }
    return result_code;
  }
  return result_code;
}

/**AbortCandidateDb()
 * @Description    : This function rollbacks the uncommitted transaction
                     from the candidate database.
 * @param[in]      : None
 * @Return         : UpplReturnCode(enum)
 * **/
UpplReturnCode DBConfigurationRequest::AbortCandidateDb() {
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  UpplReturnCode result_code = UPPL_RC_SUCCESS;
  result_code = SendDeletedControllerToLogical();
  if (result_code != UPPL_RC_SUCCESS) {
    return result_code;
  }
  result_code = SendCreatedControllerToLogical();
  if (result_code != UPPL_RC_SUCCESS) {
    return result_code;
  }
  result_code = SendUpdatedControllerToLogical();
  if (result_code != UPPL_RC_SUCCESS) {
    return result_code;
  }
  /* copy running database to candidate database */
  ODBCM_RC_STATUS copy_db_status = physical_layer->get_odbc_manager()->
      CopyDatabase(UNC_DT_RUNNING, UNC_DT_CANDIDATE);
  if (copy_db_status != ODBCM_RC_SUCCESS) {
    if (copy_db_status == ODBCM_RC_CONNECTION_ERROR) {
      // log fatal error to log daemon
      pfc_log_fatal("DB connection not available or cannot access DB");
      result_code = UPPL_RC_ERR_DB_ACCESS;
    } else {
      /*  log error to log  daemon */
      pfc_log_error("copy running to candidate database failed");
      result_code = UPPL_RC_ERR_COPY_RUNNING_TO_CANDID;
    }
  }
  return result_code;
}

/**SaveCandidateToRunning()
 * @Description    : This function will send request to ODBCManager to sync
                     candidate with running configuration
 * @param[in]      : None
 * @Return         : UpplReturnCode(enum)
 * **/
UpplReturnCode DBConfigurationRequest::SaveCandidateToRunning() {
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  UpplReturnCode result_code = UPPL_RC_SUCCESS;
  ODBCM_RC_STATUS copy_db_status;

  /* Copy candidate database to running database */
  copy_db_status = physical_layer->get_odbc_manager()->
      CopyDatabase(UNC_DT_CANDIDATE, UNC_DT_RUNNING);
  if (copy_db_status != ODBCM_RC_SUCCESS) {
    if (copy_db_status == ODBCM_RC_CONNECTION_ERROR) {
      // log fatal error to log daemon
      pfc_log_fatal("DB connection not available or cannot access DB");
      result_code = UPPL_RC_ERR_DB_ACCESS;
    } else {
      // log error  to log daemon
      pfc_log_error("copy candidate database to running database failed");
      result_code = UPPL_RC_ERR_COPY_CANDID_TO_RUNNING;
    }
  }
  return result_code;
}

/**SaveRunningToStartUp()
 * @Description    : This function will send request to ODBCManager to sync
                     running with StartUp configuration
 * @param[in]      : None
 * @Return         : UpplReturnCode(enum)
 * **/
UpplReturnCode DBConfigurationRequest::SaveRunningToStartUp()  {
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  UpplReturnCode result_code = UPPL_RC_SUCCESS;
  ODBCM_RC_STATUS copy_db_status;

  /* Copy running database to StartUp database */
  copy_db_status = physical_layer->get_odbc_manager()->
      CopyDatabase(UNC_DT_RUNNING, UNC_DT_STARTUP);
  if (copy_db_status != ODBCM_RC_SUCCESS) {
    if (copy_db_status == ODBCM_RC_CONNECTION_ERROR) {
      // log fatal error to log daemon
      pfc_log_fatal("DB connection not available or cannot access DB");
      result_code = UPPL_RC_ERR_DB_ACCESS;
    } else {
      /* log  error  to log manager */
      pfc_log_error("copy running database to StartUp database failed");
      result_code = UPPL_RC_ERR_COPY_RUNNING_TO_START;
    }
  }
  return result_code;
}

/**AuditStartUp()
 * @Description    : This function will send request to ODBCManager to copy
                     StartUp to Candidate configuration
 * @param[in]      : None
 * @Return         : UpplReturnCode(enum)
 * **/
UpplReturnCode DBConfigurationRequest::AuditStartUp() {
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  UpplReturnCode result_code = UPPL_RC_SUCCESS;
  ODBCM_RC_STATUS copy_db_status;

  /* Copy startup database to candidate database */
  copy_db_status = physical_layer->get_odbc_manager()->
      CopyDatabase(UNC_DT_STARTUP, UNC_DT_CANDIDATE);
  if (copy_db_status != ODBCM_RC_SUCCESS) {
    if (copy_db_status == ODBCM_RC_CONNECTION_ERROR) {
      // log fatal error to log daemon
      pfc_log_fatal("DB connection not available or cannot access DB");
      result_code = UPPL_RC_ERR_DB_ACCESS;
    } else {
      // log error to log manager
      pfc_log_error("copy startup database to candidate database failed");
      result_code = UPPL_RC_ERR_COPY_STARTUP_TO_CANDID;
    }
  }
  return result_code;
}

/**AuditRunningDb()
 * @Description    : This function will send request to ODBCManager to sync
                     candidate with running configuration
 * @param[in]      : None
 * @Return         : UpplReturnCode(enum)
 * **/
UpplReturnCode DBConfigurationRequest::AuditRunningDb() {
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  UpplReturnCode result_code = UPPL_RC_SUCCESS;
  ODBCM_RC_STATUS copy_db_status;

  /* Copy candidate database to running database */
  copy_db_status = physical_layer->get_odbc_manager()->
      CopyDatabase(UNC_DT_CANDIDATE, UNC_DT_RUNNING);
  if (copy_db_status != ODBCM_RC_SUCCESS)  {
    if (copy_db_status == ODBCM_RC_CONNECTION_ERROR) {
      // log fatal error to log daemon
      pfc_log_fatal("DB connection not available or cannot access DB");
      result_code = UPPL_RC_ERR_DB_ACCESS;
    } else {
      // log error to log daemon
      pfc_log_error("copy candidate database to running database failed");
      result_code = UPPL_RC_ERR_COPY_CANDID_TO_RUNNING;
    }
  }
  return result_code;
}

/**SendDeletedControllerToLogical()
 * @Description    : This function will send the deleted controller
 * to logical during abort candidate db/ abort transaction
 * @param[in]      : None
 * @Return         : UpplReturnCode(enum)
 * **/
UpplReturnCode DBConfigurationRequest::SendDeletedControllerToLogical() {
  Kt_Controller kt_ctr;
  vector<void*> vec_key_ctr_modified;
  // Getting the created Configuration from the database
  UpplReturnCode result_code = kt_ctr.GetModifiedRows(
      vec_key_ctr_modified,
      CREATED);
  if (result_code != UPPL_RC_SUCCESS) {
    // No created controller available
    return UPPL_RC_SUCCESS;
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
    if (result_code != UPPL_RC_SUCCESS) {
      pfc_log_error("Failed to send the info to UPLL of controller");
      return UPPL_RC_ERR_LOGICAL_COMMUNICATION_FAILURE;
    }
  }
  return UPPL_RC_SUCCESS;
}

/**SendCreatedControllerToLogical()
 * @Description    : This function will send the created controller
 * to logical during abort candidate db/ abort transaction
 * @param[in]      : None
 * @Return         : UpplReturnCode(enum)
 * **/
UpplReturnCode DBConfigurationRequest::SendCreatedControllerToLogical() {
  Kt_Controller kt_ctr;
  vector<void*> vec_key_ctr_modified;
  // Getting the deleted Configuration from the database
  UpplReturnCode result_code = kt_ctr.GetModifiedRows(
      vec_key_ctr_modified,
      DELETED);
  if (result_code != UPPL_RC_SUCCESS) {
    // No deleted controller available
    return UPPL_RC_SUCCESS;
  }

  for (uint32_t config_count = 0;
      config_count != vec_key_ctr_modified.size(); config_count++) {
    vector<void *> vect_ctr_key, vect_ctr_val;
    key_ctr_t *ctr_key = reinterpret_cast<key_ctr_t*>
    (vec_key_ctr_modified[config_count]);
    pfc_log_debug("Controller Name %s", ctr_key->controller_name);
    vect_ctr_key.push_back(vec_key_ctr_modified[config_count]);
    if (kt_ctr.ReadInternal(vect_ctr_key,
                            vect_ctr_val,
                            UNC_DT_RUNNING,
                            UNC_OP_READ) != UPPL_RC_SUCCESS) {
      continue;
    }
    val_ctr_st_t *val_ctr_st =
        reinterpret_cast<val_ctr_st_t*>(vect_ctr_val[0]);
    if (val_ctr_st == NULL) {
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
    if (result_code != UPPL_RC_SUCCESS) {
      pfc_log_error("Failed to send the info to UPLL of controller");
      return UPPL_RC_ERR_LOGICAL_COMMUNICATION_FAILURE;
    }
  }
  return UPPL_RC_SUCCESS;
}

/**SendUpdatedControllerToLogical()
 * @Description    : This function will send the updated controller
 * to logical during abort candidate db/ abort transaction
 * @param[in]      : None
 * @Return         : UpplReturnCode(enum)
 * **/
UpplReturnCode DBConfigurationRequest::SendUpdatedControllerToLogical() {
  Kt_Controller kt_ctr;
  vector<void*> vec_key_ctr_modified;
  // Getting the Update Configuration from the database
  UpplReturnCode result_code = kt_ctr.GetModifiedRows(
      vec_key_ctr_modified,
      UPDATED);
  if (result_code != UPPL_RC_SUCCESS) {
    // No updated controller available
    return UPPL_RC_SUCCESS;
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
    if (kt_ctr.ReadInternal(vect_ctr_key,
                            vect_ctr_val,
                            UNC_DT_RUNNING,
                            UNC_OP_READ) != UPPL_RC_SUCCESS) {
      continue;
    }
    val_ctr_st_t *val_ctr_st =
        reinterpret_cast<val_ctr_st_t*>(vect_ctr_val[0]);
    if (val_ctr_st == NULL) {
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
    if (result_code != UPPL_RC_SUCCESS) {
      pfc_log_error("Failed to send the info to UPLL of controller");
      return UPPL_RC_ERR_LOGICAL_COMMUNICATION_FAILURE;
    }
  }
  return UPPL_RC_SUCCESS;
}
