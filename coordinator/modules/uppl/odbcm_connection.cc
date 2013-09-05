/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/**
 * @brief   ODBC Manager Module
 * @file    odbcm_connection.cc
 *
 */

#include "odbcm_mgr.hh"
#include "odbcm_db_tableschema.hh"
#include "odbcm_db_varbind.hh"
#include "odbcm_query_factory.hh"
#include "odbcm_query_processor.hh"
#include "odbcm_utils.hh"
#include "physicallayer.hh"
#include "odbcm_connection.hh"
using unc::uppl::ODBCManager;
using unc::uppl::ODBCMTable;
using unc::uppl::QueryFactory;
using unc::uppl::QueryProcessor;
using unc::uppl::DBVarbind;
extern pfc_cfdef_t odbcm_cfdef;

/* static variable initialization */
ODBCManager *ODBCManager::ODBCManager_ = NULL;

/**
 * @Description : Constructor function which creates/initializes
 *                single instance of ODBCManager
 * @param[in]   : None
 * @return      : void
 **/
ODBCManager::ODBCManager()
:
          //  initialize the ODBCMInit flag
          IsODBCManager_initialized(0),
          /** Initialize the ODBCManager members */
          phy_conn_env_(NULL) {
  rw_nb_conn_obj_ = NULL;
  rw_sb_conn_obj_ = NULL;
}

/**
 * @Description : Destructor of odbcmanager will free all the resources
 *                which are initialized during the instantiation and 
 *                other object within the object as well
 * @param[in]   : None
 * @return      : void
 **/
ODBCManager::~ODBCManager() {
  pfc_log_info("ODBCM::~ODBCManager: Destructor to free resources");
  /** Clear all the vector inside the map */
  ((get_db_table_list_map_().find(UNC_DT_STARTUP))->second).clear();
  ((get_db_table_list_map_().find(UNC_DT_CANDIDATE))->second).clear();
  ((get_db_table_list_map_().find(UNC_DT_RUNNING))->second).clear();
  ((get_db_table_list_map_().find(UNC_DT_IMPORT))->second).clear();
  ((get_db_table_list_map_().find(UNC_DT_STATE))->second).clear();
  /** Finally, clear the db_table_list_map */
  if (!db_table_list_map_.empty())
    db_table_list_map_.clear();
  /**to clear the OdbcmSQLStateMap elements*/
  ODBCMUtils::ClearOdbcmSQLStateMap();
  /** Free the uppl db connection and environments */
  ODBCM_RC_STATUS rw_disconn_status = CloseRwConnection();
  pfc_log_debug("ODBCM::ODBCManager::CloseRwConnection Status %d",
                rw_disconn_status);
  if (NULL != phy_conn_env_) {
    SQLFreeHandle(SQL_HANDLE_ENV,  phy_conn_env_);
    pfc_log_info("ODBCM::ODBCManager:: Disconnect phy_conn_env_");
    phy_conn_env_ = NULL;
  }
}
/**
 * @Description : This function will return the ODBC read-write,
 *                read-only connection environment and it's a const method.
 * @param[in]   : None
 * @return      : SQLHENV - It will calls SQLAllocHandle and passes the address
 *                of the variable and the SQL_HANDLE_ENV option.
 **/
inline SQLHENV ODBCManager::get_phy_connection_env_() const {
  return phy_conn_env_;
}

/**
 * @Description : This static method will return the singleton
 *                ODBCManager instance if exists else create new one and return.
 * @param[in]   : None
 * @return      : ODBCManager* - the singleton instance if exists
 *                else create new one and return.
 **/
ODBCManager* ODBCManager::get_ODBCManager() {
  /*Allocate the memory for ODBCManager only if its NULL*/
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  physical_layer->ODBCManager_mutex_.lock();
  if (ODBCManager_ == NULL) {
    ODBCManager_ = new ODBCManager();
    if (NULL == ODBCManager_) {
      pfc_log_fatal("ODBCM::ODBCManager::get_ODBCManager: "
          "Error in memory allocation for ODBCManager_ !!! ");
      physical_layer->ODBCManager_mutex_.unlock();
      return NULL;
    }
  }
  physical_layer->ODBCManager_mutex_.unlock();
  return ODBCManager_;
}
/**
 * @Description : function to get table list stored in DB
 * @param[in]   : None
 * @return      : db_table_list_map_ - return table list stored in Database
 **/
std::map<int, std::vector<std::string> >&
ODBCManager::get_db_table_list_map_() {
  return db_table_list_map_;
}

/**
 * @Description : function to get table columns list in map
 * @param[in]   : None
 * @return      : odbcm_tables_column_map_
 **/
std::map<ODBCMTableColumns, std::string>&
ODBCManager::get_odbcm_tables_column_map_() {
  return odbcm_tables_column_map_;
}

/**
 * @Description : function to set read-write ODBC connection handle
 * @param[in]   : None
 * @return      : ODBCM_RC_SUCCESS - read-write ODBC connection handle 
 *                is set to success
 *                ODBCM_RC_*       - read-write ODBC connection handle failed
 **/
inline ODBCM_RC_STATUS ODBCManager::set_rw_connection_handle_(
    SQLHDBC& rw_conn_handle) {
  // 2. allocate rw connection handle
  SQLRETURN odbc_rc = SQLAllocHandle(SQL_HANDLE_DBC, phy_conn_env_,
                                     &rw_conn_handle);
  ODBCM_DBC_HANDLE_CHECK(phy_conn_env_, odbc_rc);
  if (odbc_rc != ODBCM_RC_SUCCESS && odbc_rc != ODBCM_RC_SUCCESS_WITH_INFO) {
    pfc_log_error("ODBCM::ODBCManager::set_rw_connection_handle_:"
        " Error in SQLAllocHandle %s",
        ODBCMUtils::get_RC_Details(odbc_rc).c_str());
    return (ODBCM_RC_STATUS) odbc_rc;
  }
  pfc_log_info("ODBCM::ODBCManager::set_rw_connection_handle_: Success");
  return ODBCM_RC_SUCCESS;
}
/**
 * @Description : function to set read ODBC connection handle
 * @param[in]   : None
 * @return      : ODBCM_RC_SUCCESS - read ODBC connection handle is set 
 *                to success
 *                ODBCM_RC_*       - read ODBC connection handle failed
 **/
inline ODBCM_RC_STATUS ODBCManager::set_ro_connection_handle_(
    SQLHDBC& ro_conn_handle) {
  // 2. allocate ro connection handle
  SQLRETURN odbc_rc = SQLAllocHandle(SQL_HANDLE_DBC, phy_conn_env_,
                                     &ro_conn_handle);
  ODBCM_DBC_HANDLE_CHECK(phy_conn_env_, odbc_rc);
  if (odbc_rc != ODBCM_RC_SUCCESS && odbc_rc != ODBCM_RC_SUCCESS_WITH_INFO) {
    pfc_log_error("ODBCM::ODBCManager::set_ro_connection_handle_:"
        " Error in SQLAllocHandle %s",
        ODBCMUtils::get_RC_Details(odbc_rc).c_str());
    return (ODBCM_RC_STATUS) odbc_rc;
  }
  pfc_log_debug("ODBCM::ODBCManager::set_ro_connection_handle_: Success");
  return ODBCM_RC_SUCCESS;
}
/**
 * @Description : function to set physical ODBC connection environment
 * @param[in]   : None
 * @return      : ODBCM_RC_SUCCESS - physical ODBC connection handle is set
 *                to success
 *                ODBCM_RC_*       - physicall ODBC connection handle failed
 **/
inline ODBCM_RC_STATUS ODBCManager::set_phy_connection_env_() {
  // 1. allocate Environment handle and register version
  SQLRETURN odbc_rc = ODBCM_RC_SUCCESS;
  odbc_rc = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &phy_conn_env_);
  ODBCM_ENV_HANDLE_CHECK(phy_conn_env_, odbc_rc);
  if (odbc_rc != ODBCM_RC_SUCCESS && odbc_rc != ODBCM_RC_SUCCESS_WITH_INFO) {
    pfc_log_error("ODBCM::ODBCManager::set_phy_connection_env_: "
        "Error in phy_conn_env_ %s",
        ODBCMUtils::get_RC_Details(odbc_rc).c_str());
    return (ODBCM_RC_STATUS) odbc_rc;
  }
  pfc_log_debug("ODBCM::ODBCManager::set_phy_connection_env_: "
      "phy_conn_env_ is allocated");
  odbc_rc = SQLSetEnvAttr(phy_conn_env_,  SQL_ATTR_ODBC_VERSION,
                          reinterpret_cast<void*>(SQL_OV_ODBC3), 0);
  ODBCM_ENV_HANDLE_CHECK(phy_conn_env_, odbc_rc);
  if (odbc_rc != ODBCM_RC_SUCCESS && odbc_rc != ODBCM_RC_SUCCESS_WITH_INFO) {
    pfc_log_error("ODBCM::ODBCManager::set_phy_connection_env_:"
        " Error in SQLSetEnvAttr %s",
        ODBCMUtils::get_RC_Details(odbc_rc).c_str());
    return (ODBCM_RC_STATUS) odbc_rc;
  }
  pfc_log_info("ODBCM::ODBCManager::set_phy_connection_env_: Success");
  return ODBCM_RC_SUCCESS;
}

/**
 * @Description : Function to initialize the DB table list map
 * @param[in]   : None
 * @return      : ODBCM_RC_SUCCESS - initialize database table list map is set
 *                to success
 *                ODBCM_RC_*       - initialize failed in database table map
 **/
ODBCM_RC_STATUS ODBCManager::initialize_db_table_list_map_(void) {
  uint16_t       loop1 = 0;         // To loop the tables
  vector<string> startup_vector;    // To store startup tables
  vector<string> candidate_vector;  // To store candidate tables
  vector<string> running_vector;    // To store running tables
  vector<string> state_vector;      // To store state tables
  vector<string> import_vector;     // To store state tables
  /** Below tables are present in Startup, Candidate, running */
  string uppl_tables[ODBCM_MAX_UPPL_TABLES] =
  { UPPL_CTR_TABLE,
      UPPL_CTR_DOMAIN_TABLE,
      UPPL_BOUNDARY_TABLE
  };

  /** Below tables are present only in running to support dt_state */
  string state_tables[ODBCM_MAX_STATE_TABLES] =
  { UPPL_LOGICALPORT_TABLE,
      UPPL_LOGICAL_MEMBER_PORT_TABLE,
      UPPL_SWITCH_TABLE,
      UPPL_PORT_TABLE,
      UPPL_LINK_TABLE
  };

  /** Below tables are present import tables */
  string import_tables[ODBCM_MAX_IMPORT_TABLES] =
  { UPPL_CTR_DOMAIN_TABLE,
      UPPL_LOGICALPORT_TABLE,
      UPPL_LOGICAL_MEMBER_PORT_TABLE,
      UPPL_SWITCH_TABLE,
      UPPL_PORT_TABLE,
      UPPL_LINK_TABLE
  };
  /** Push table names into startup and candidate vector */
  for (loop1 = 0; loop1 < ODBCM_MAX_UPPL_TABLES; loop1++) {
    startup_vector.push_back("s_" + uppl_tables[loop1]);
    candidate_vector.push_back("c_" + uppl_tables[loop1]);
    running_vector.push_back("r_" + uppl_tables[loop1]);
  }
  /** Push table names into running and state vector */
  for (loop1 = 0; loop1 < ODBCM_MAX_STATE_TABLES; loop1++) {
    state_vector.push_back("r_" + state_tables[loop1]);
  }
  /** Push import table names into import vector */
  for (loop1 = 0; loop1 < ODBCM_MAX_IMPORT_TABLES; loop1++) {
    import_vector.push_back("i_" + import_tables[loop1]);
  }
  /** Push vectors into table list map */
  db_table_list_map_.insert(map<int, vector<string> >::
                            value_type(UNC_DT_STARTUP, startup_vector));
  db_table_list_map_.insert(map<int, vector<string> >::
                            value_type(UNC_DT_CANDIDATE, candidate_vector));
  db_table_list_map_.insert(map<int, vector<string> >::
                            value_type(UNC_DT_RUNNING, running_vector));
  db_table_list_map_.insert(map<int, vector<string> >::
                            value_type(UNC_DT_STATE, state_vector));
  db_table_list_map_.insert(map<int, vector<string> >::
                            value_type(UNC_DT_IMPORT, import_vector));
  return ODBCM_RC_SUCCESS;
}

/**
 @Description   : This function will parse the configuration file and
 *                update the local structure db_conf_info. This structure 
 *                will be used during connection establishment.
 * @param[in]   : None
 * @return      : ODBCM_RC_SUCCESS - if the ParseConfigFile function is set to
 *                success
 *                ODBCM_RC_*       - if the ParseConfigFile function is set to
 *                failure
 **/
ODBCM_RC_STATUS ODBCManager::ParseConfigFile() {
  std::string conf_file_path = "";
  conf_file_path.append(UNC_MODULEDIR);
  conf_file_path.append(ODBCM_CONF_FILE_PATH);
  conf_file_path.append(ODBCM_CONF_FILE);
  pfc::core::ConfHandle conf_handle(conf_file_path, &odbcm_cfdef);
  int32_t err = conf_handle.getError();
  pfc_log_debug("ODBCM::ODBCManager::ParseConfigFile: "
      "path: %s ", conf_file_path.c_str());
  if (err != 0) {
    pfc_log_error("ODBCM::ODBCManager::ParseConfigFile: "
        "Err:%d, path: %s ", err, conf_file_path.c_str());
    return ODBCM_RC_FAILED;
  }
  pfc::core::ConfBlock logblock(conf_handle, "odbcm_params");

  // store the parsed values in structure
  conf_parser_.dsn     = logblock.getString("dsname", "UNC_DB_DSN");
  conf_parser_.time_out        = logblock.getUint32("time_out", 10);

  ODBCM_PRINT_DEBUG_LOG("%s", conf_parser_.dsn.c_str());
  ODBCM_PRINT_DEBUG_LOG("%d", conf_parser_.time_out);
  return ODBCM_RC_SUCCESS;
}

/**
 * @Description : This function will establish the connection between
 *                odbcmanager and RDBMS system
 * @param[in]   : None
 * @return      : ODBCM_RC_SUCCESS - if the connection between odbcmanager
 *                and RDBMS system is set to success.
 *                ODBCM_RC_*       -if the connection between odbcmanager
 *                and RDBMS system is set to failure.
 **/
ODBCM_RC_STATUS ODBCManager::InitializeConnectionEnv() {
  SQLRETURN odbc_rc = ODBCM_RC_SUCCESS;
  /* Configuring Pooling using ODBC API
    - SQL_CP_ONE_PER_HENV - Single pool for each hEnv (Environment handle)
    - SQL_CP_ONE_PER_DRIVER -
          Separate connection pool is supported for each driver
    - If using SQLDriverConnect(), need to use SQL_DRIVER_NOPROMPT
          otherwise connection will not be pooled.
   */
  odbc_rc = SQLSetEnvAttr(NULL, SQL_ATTR_CONNECTION_POOLING,
                          (SQLPOINTER) SQL_CP_ONE_PER_DRIVER, SQL_IS_INTEGER);
  if (odbc_rc != ODBCM_RC_SUCCESS && odbc_rc != ODBCM_RC_SUCCESS_WITH_INFO) {
    pfc_log_error("ODBCM::ODBCManager::InitializeConnectionEnv: Error in "
        "SQLSetEnvAttr in connection pooling activation");
    return ODBCM_RC_CONNECTION_ERROR;
  }

  if (set_phy_connection_env_() != ODBCM_RC_SUCCESS) {
    pfc_log_error("ODBCM::ODBCManager::InitializeConnectionEnv: "
        "Error in set_phy_connection_env_ ");
    return ODBCM_RC_CONNECTION_ERROR;
  }
  return ODBCM_RC_SUCCESS;
}

/**
 * @Description : This function will establish the connection between
 * odbcmanager and RDBMS system
 * @param[in]   : None
 * @return      : ODBCM_RC_STATUS
 **/
ODBCM_RC_STATUS ODBCManager::OpenDBConnection(
    OdbcmConnectionHandler *conn_obj) {
  SQLRETURN odbc_rc = ODBCM_RC_SUCCESS;
  SQLHDBC conn_handle = conn_obj->get_conn_handle();
  switch (conn_obj->get_conn_type()) {
    case kOdbcmConnReadWriteNb:
      // Check if connection already exists
      if (rw_nb_conn_obj_ != NULL &&
          rw_nb_conn_obj_->get_conn_handle() != NULL) {
        // Connection already exists
        pfc_log_debug("Using existing Nb db connection request");
        conn_obj->set_conn_handle(rw_nb_conn_obj_->get_conn_handle());
        return ODBCM_RC_SUCCESS;
      }
      if (set_rw_connection_handle_(conn_handle) != ODBCM_RC_SUCCESS) {
        pfc_log_error("ODBCM::ODBCManager::OpenDBConnection: "
            "Error in set_rw_connection_handle_ ");
        return ODBCM_RC_CONNECTION_ERROR;
      }
      /**  setting SQL_ATTR_ACCESS_MODE for RW connection is not required
       *  SQL_MODE_READ_WRITE is default */
      break;
    case kOdbcmConnReadWriteSb:
      // Check if connection already exists
      if (rw_sb_conn_obj_ != NULL &&
          rw_sb_conn_obj_->get_conn_handle() != NULL) {
        // Connection already exists
        pfc_log_debug("Using existing Sb db connection request");
        conn_obj->set_conn_handle(rw_sb_conn_obj_->get_conn_handle());
        return ODBCM_RC_SUCCESS;
      }
      if (set_rw_connection_handle_(conn_handle) != ODBCM_RC_SUCCESS) {
        pfc_log_error("ODBCM::ODBCManager::OpenDBConnection: "
            "Error in set_rw_connection_handle_ ");
        return ODBCM_RC_CONNECTION_ERROR;
      }
      /**  setting SQL_ATTR_ACCESS_MODE for RW connection is not required
       *  SQL_MODE_READ_WRITE is default */
      break;
    case kOdbcmConnReadOnly:
      if (set_ro_connection_handle_(conn_handle) != ODBCM_RC_SUCCESS) {
        pfc_log_error("ODBCM::ODBCManager::OpenDBConnection: "
            "Error in set_ro_connection_handle_ ");
        return ODBCM_RC_CONNECTION_ERROR;
      }
      odbc_rc = SQLSetConnectAttr(conn_handle, SQL_ATTR_ACCESS_MODE,
                                  (SQLPOINTER)SQL_MODE_READ_ONLY, 0);
      ODBCM_DBC_HANDLE_CHECK(conn_handle, odbc_rc);
      if (odbc_rc != ODBCM_RC_SUCCESS &&
          odbc_rc != ODBCM_RC_SUCCESS_WITH_INFO) {
        pfc_log_debug("ODBCM::ODBCManager::OpenDBConnection: "
            "Error in SQLSetConnectAttr");
        return ODBCM_RC_CONNECTION_ERROR;
      }
      break;
    default:
      pfc_log_error("ODBCM::ODBCManager::OpenDBConnection: "
          "Invalid connection type %d !! ",
          conn_obj->get_conn_type());
      break;
  }
  // *************************************************************************
  // Set the matching condition for using an existing connection in the pool
  odbc_rc = SQLSetEnvAttr(phy_conn_env_, SQL_ATTR_CP_MATCH,
                          (SQLPOINTER)SQL_CP_RELAXED_MATCH, SQL_IS_INTEGER);
  if (odbc_rc != ODBCM_RC_SUCCESS && odbc_rc != ODBCM_RC_SUCCESS_WITH_INFO) {
    pfc_log_debug("ODBCM::ODBCManager::OpenDBConnection: "
        "Error in SQLSetEnvAttr during connec. pooling activation");
    return ODBCM_RC_CONNECTION_ERROR;
  }
  //  create and pool the connections.

  std::ostringstream conn_string_stream;
  conn_string_stream << "DSN=" << conf_parser_.dsn <<
      ";TIMEOUT=" << conf_parser_.time_out;

  std::string conn_string = conn_string_stream.str();
  pfc_log_debug("ODBCM::ODBCManager::OpenDBConnection: "
      "connection string = %s", conn_string.c_str());

  // 3. Connect to the datasource
  if (NULL != conn_handle)
    odbc_rc = SQLDriverConnect(conn_handle, NULL,
                               (unsigned char*)(conn_string.c_str()),
                               SQL_NTS, NULL, 0, NULL, SQL_DRIVER_NOPROMPT);
  //  SQL_DRIVER_COMPLETE
  ODBCM_DBC_HANDLE_CHECK(conn_handle, odbc_rc);
  if (odbc_rc != ODBCM_RC_SUCCESS &&
      odbc_rc != ODBCM_RC_SUCCESS_WITH_INFO) {
    pfc_log_error("ODBCM::ODBCManager::OpenDBConnection:"
        "Could not establish connection type = %d !!",
        conn_obj->get_conn_type());
    return ODBCM_RC_CONNECTION_ERROR;
  }
  /**  setting connection attributes */
  /** Attribute - Set before or after connection
   * Either before or after
   SQL_ATTR_ACCESS_MODE
   SQL_ATTR_ASYNC_DBC_EVENT
   SQL_ATTR_ASYNC_DBC_FUNCTIONS_ENABLE
   SQL_ATTR_ASYNC_DBC_PCALLBACK
   SQL_ATTR_ASYNC_DBC_PCONTEXT
   SQL_ATTR_ASYNC_ENABLE
   SQL_ATTR_AUTO_IPD
   SQL_ATTR_AUTOCOMMIT -done
   SQL_ATTR_CONNECTION_TIMEOUT - done on connection string
   SQL_ATTR_CURRENT_CATALOG
   SQL_ATTR_METADATA_ID
   SQL_ATTR_QUIET_MODE
   SQL_ATTR_TRACE
   SQL_ATTR_TRACEFILE
   SQL_ATTR_TXN_ISOLATION
   * After
   SQL_ATTR_CONNECTION_DEAD
   SQL_ATTR_DBC_INFO_TOKEN
   SQL_ATTR_ENLIST_IN_DTC
   SQL_ATTR_TRANSLATE_LIB
   SQL_ATTR_TRANSLATE_OPTION
   */

  /**  setting SQL_ATTR_ACCESS_MODE for RW connection is not required
   *  SQL_MODE_READ_WRITE is default */
  odbc_rc = SQLSetConnectAttr(conn_handle, SQL_ATTR_TXN_ISOLATION,
                              (SQLPOINTER)SQL_TXN_READ_COMMITTED, 0);
  ODBCM_DBC_HANDLE_CHECK(conn_handle, odbc_rc);
  if (odbc_rc != ODBCM_RC_SUCCESS &&
      odbc_rc != ODBCM_RC_SUCCESS_WITH_INFO) {
    pfc_log_debug("ODBCM::ODBCManager::OpenDBConnection: "
        "Error in SQLSetConnectAttr");
    return ODBCM_RC_CONNECTION_ERROR;
  }

  odbc_rc = SQLSetConnectAttr(conn_handle, SQL_ATTR_AUTOCOMMIT,
                              (SQLPOINTER)SQL_AUTOCOMMIT_OFF, SQL_NTS);
  ODBCM_DBC_HANDLE_CHECK(conn_handle, odbc_rc);
  if (odbc_rc != ODBCM_RC_SUCCESS &&
      odbc_rc != ODBCM_RC_SUCCESS_WITH_INFO) {
    pfc_log_debug("ODBCM::ODBCManager::OpenDBConnection: "
        "Error in SQLSetConnectAttr");
    return ODBCM_RC_CONNECTION_ERROR;
  }

  /**to set the created connection handle into OdbcmConnectionHanlder 
   * object reference*/
  conn_obj->set_conn_handle(conn_handle);
  if (rw_nb_conn_obj_ == NULL &&
      conn_obj->get_conn_type() == kOdbcmConnReadWriteNb) {
    rw_nb_conn_obj_ = new OdbcmConnectionHandler(*conn_obj);
  } else if (rw_sb_conn_obj_ == NULL &&
      conn_obj->get_conn_type() == kOdbcmConnReadWriteSb) {
    rw_sb_conn_obj_ = new OdbcmConnectionHandler(*conn_obj);
  }
  pfc_log_info("ODBCM::ODBCManager::OpenDBConnection: "
      "type %d database connection creation success",
      conn_obj->get_conn_type());
  return ODBCM_RC_SUCCESS;
}

/**
 * @Description : This function will close the existing database connection
 * @param[in]   : None
 * @return      : ODBCM_RC_SUCCESS - if the connection close the existing
 *                database is set to success.
 *                ODBCM_RC_*       - if the existing connection of database
 *                is not close it set to failure.
 **/
ODBCM_RC_STATUS ODBCManager::CloseDBConnection(
    OdbcmConnectionHandler *conn_obj) {
  SQLHDBC conn_handle = conn_obj->get_conn_handle();
  if (conn_obj->get_conn_type() == kOdbcmConnReadWriteNb ||
      conn_obj->get_conn_type() == kOdbcmConnReadWriteSb) {
    if (PhysicalLayer::phyFiniFlag == 1) {
      // Connection is already released in destructor
      return ODBCM_RC_SUCCESS;
    }
    pfc_log_debug("Read Write connection will not be closed now");
    ODBCM_RC_STATUS status = ODBCM_RC_SUCCESS;
    ODBCM_END_TRANSACTION(conn_handle, SQL_ROLLBACK);
    return status;
  }
  /*  to disconnect */
  if (NULL != conn_handle) {
    SQLRETURN odbc_rc = 0;
    odbc_rc = SQLDisconnect(conn_handle);
    odbc_rc = SQLFreeHandle(SQL_HANDLE_DBC, conn_handle);
    conn_handle = NULL;
    conn_obj->set_conn_handle(conn_handle);
    if (odbc_rc != ODBCM_RC_SUCCESS) {
      pfc_log_error("ODBCM::ODBCManager::CloseDBConnection: "
          "Error on Disconnect conn_handle");
      return (ODBCM_RC_STATUS) odbc_rc;
    }
    pfc_log_debug("Read Only Connection is closed");
  }
  pfc_log_debug("ODBCM::ODBCManager::CloseDBConnection: "
      "Now, the connection is disconnected...");
  return ODBCM_RC_SUCCESS;
}

/**
 * @Description : This function will close the existing database rw connection
 * @param[in]   : None
 * @return      : ODBCM_RC_SUCCESS - if the connection close the existing
 *                database is set to success.
 *                ODBCM_RC_*       - if the existing connection of database
 *                is not close it set to failure.
 **/
ODBCM_RC_STATUS ODBCManager::CloseRwConnection() {
  if (rw_nb_conn_obj_ != NULL) {
    SQLHDBC conn_handle = rw_nb_conn_obj_->get_conn_handle();
    /*  disconnect nb conn handle*/
    if (NULL != conn_handle) {
      SQLRETURN odbc_rc = 0;
      odbc_rc = SQLDisconnect(conn_handle);
      odbc_rc = SQLFreeHandle(SQL_HANDLE_DBC, conn_handle);
      conn_handle = NULL;
      rw_nb_conn_obj_->set_conn_handle(NULL);
      delete rw_nb_conn_obj_;
      rw_nb_conn_obj_ = NULL;
      if (odbc_rc != ODBCM_RC_SUCCESS) {
        pfc_log_error("ODBCM::ODBCManager::CloseRwConnection: "
            "Error on Disconnect rw_nb_conn_handle");
        return (ODBCM_RC_STATUS) odbc_rc;
      }
    }
  }
  if (rw_sb_conn_obj_ != NULL) {
    SQLHDBC conn_handle = rw_sb_conn_obj_->get_conn_handle();
    /*  disconnect sb conn handle*/
    if (NULL != conn_handle) {
      SQLRETURN odbc_rc = 0;
      odbc_rc = SQLDisconnect(conn_handle);
      odbc_rc = SQLFreeHandle(SQL_HANDLE_DBC, conn_handle);
      conn_handle = NULL;
      rw_sb_conn_obj_->set_conn_handle(NULL);
      delete rw_sb_conn_obj_;
      rw_sb_conn_obj_ = NULL;
      if (odbc_rc != ODBCM_RC_SUCCESS) {
        pfc_log_error("ODBCM::ODBCManager::CloseRwConnection: "
            "Error on Disconnect rw_sb_conn_handle");
        return (ODBCM_RC_STATUS) odbc_rc;
      }
    }
  }
  pfc_log_debug("ODBCM::ODBCManager::CloseRwConnection: "
      "Now, the rw connections are disconnected...");
  return ODBCM_RC_SUCCESS;
}

/**
 * @Description : Get the table id from table name
 * @param[in]   : table_name - name of the tables 
 * @return      : ODBCMTable - returns the table name
 **/
ODBCMTable ODBCManager::get_table_id(string table_name) {
  if (table_name.compare(UPPL_CTR_TABLE) == 0) {
    return CTR_TABLE;
  } else if (table_name.compare(UPPL_CTR_DOMAIN_TABLE) == 0) {
    return CTR_DOMAIN_TABLE;
  } else if (table_name.compare(UPPL_LOGICALPORT_TABLE) == 0) {
    return LOGICALPORT_TABLE;
  } else if (table_name.compare(UPPL_LOGICAL_MEMBER_PORT_TABLE) == 0) {
    return LOGICAL_MEMBERPORT_TABLE;
  } else if (table_name.compare(UPPL_SWITCH_TABLE) == 0) {
    return SWITCH_TABLE;
  } else if (table_name.compare(UPPL_PORT_TABLE) == 0) {
    return PORT_TABLE;
  } else if (table_name.compare(UPPL_LINK_TABLE) == 0) {
    return LINK_TABLE;
  } else if (table_name.compare(UPPL_BOUNDARY_TABLE) == 0) {
    return BOUNDARY_TABLE;
  } else {
    pfc_log_debug("ODBCM::ODBCManager::get_table_id: "
        "Invalid table");
    return UNKNOWN_TABLE;
  }
}

/**
 * @Description : clear the allocated memory
 * @param[in]   : stmt          - which carries the SQL Query 
 *                table_id      - enum of the tables
 *                dbvarbindobj  - database varbind object
 *                qfactoryobj   - query factory object 
 *                qprocessorobj - query processor object
 * @return      : void 
 **/
void ODBCManager::ODBCMFreeingMemory(HSTMT stmt,
                                     ODBCMTable table_id,
                                     DBVarbind *dbvarbindobj,
                                     QueryFactory *qfactoryobj,
                                     QueryProcessor *qprocessorobj) {
  SQLRETURN odbc_rc = 0;
  /**  statement handler free */
  if (NULL != stmt) {
    odbc_rc = SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    stmt = NULL;
  } else {
    pfc_log_debug("ODBCM::ODBCManager::ODBCMFreeingMemory:stmt is NULL");
  }
  if (odbc_rc != ODBCM_RC_SUCCESS) {
    pfc_log_error("ODBCM::ODBCManager::ODBCMFreeingMemory: Error while "
        "Freeing resources");
  }
  /** Freeing allocated memory */
  dbvarbindobj->FreeingBindedStructure(table_id);
  ODBCM_FREE_MEMORY(dbvarbindobj);
  dbvarbindobj = NULL;
  ODBCM_FREE_MEMORY(qfactoryobj);
  qfactoryobj = NULL;
  ODBCM_FREE_MEMORY(qprocessorobj);
  qprocessorobj = NULL;
  return;
}

/**
 * @Description : To get the column name using column id (enum)
 * @param[in]   : col_id - specifies the column id enum value
 * @return      : function returns corresponding column name string for the
 *                given column id (enum)
 **/
std::string ODBCManager::GetColumnName(ODBCMTableColumns col_id) {
  std::map< ODBCMTableColumns, std::string>& ODBCMTableColumnsMap =
      get_odbcm_tables_column_map_();
  std::map< ODBCMTableColumns, std::string>::iterator column_iter =
      ODBCMTableColumnsMap.find(col_id);
  if (column_iter == ODBCMTableColumnsMap.end()) {
    pfc_log_debug("ODBCM::ODBCManager:GetColumnName"
        "column name does not exists ");
    return "";  /*return null string*/
  }
  return column_iter->second;
}

/**
 * @Description : Function to initialize the DB table list map
 * @param[in]   : None
 * @return      : ODBCM_RC_STATUS
 **/
ODBCM_RC_STATUS ODBCManager::initialize_odbcm_tables_column_map_(void) {
  OdbcmColumnName odbcm_table_columns[] = {
      {CTR_NAME, std::string(CTR_NAME_STR)},
      {CTR_TYPE, std::string(CTR_TYPE_STR)},
      {CTR_VERSION, std::string(CTR_VERSION_STR)},
      {CTR_DESCRIPTION, std::string(CTR_DESCRIPTION_STR)},
      {CTR_IP_ADDRESS, std::string(CTR_IP_ADDRESS_STR)},
      {CTR_USER_NAME, std::string(CTR_USER_NAME_STR)},
      {CTR_PASSWORD, std::string(CTR_PASSWORD_STR)},
      {CTR_ENABLE_AUDIT, std::string(CTR_ENABLE_AUDIT_STR)},
      {CTR_ACTUAL_VERSION, std::string(CTR_ACTUAL_VERSION_STR)},
      {CTR_OPER_STATUS, std::string(CTR_OPER_STATUS_STR)},
      {CTR_VALID, std::string(CTR_VALID_STR)},
      {CTR_CS_ROW_STATUS, std::string(CTR_CS_ROW_STATUS_STR)},
      {CTR_CS_ATTR, std::string(CTR_CS_ATTR_STR)},
      {DOMAIN_NAME, std::string(DOMAIN_NAME_STR)},
      {DOMAIN_TYPE, std::string(DOMAIN_TYPE_STR)},
      {DOMAIN_DESCRIPTION, std::string(DOMAIN_DESCRIPTION_STR)},
      {DOMAIN_OP_STATUS, std::string(DOMAIN_OP_STATUS_STR)},
      {DOMAIN_VALID, std::string(DOMAIN_VALID_STR)},
      {DOMAIN_CS_ROW_STATUS, std::string(DOMAIN_CS_ROW_STATUS_STR)},
      {DOMAIN_CS_ATTR, std::string(DOMAIN_CS_ATTR_STR)},
      {LP_PORT_ID, std::string(LP_PORT_ID_STR)},
      {LP_DESCRIPTION, std::string(LP_DESCRIPTION_STR)},
      {LP_PORT_TYPE, std::string(LP_PORT_TYPE_STR)},
      {LP_SWITCH_ID, std::string(LP_SWITCH_ID_STR)},
      {LP_PHYSICAL_PORT_ID, std::string(LP_PHYSICAL_PORT_ID_STR)},
      {LP_OPER_DOWN_CRITERIA, std::string(LP_OPER_DOWN_CRITERIA_STR)},
      {LP_OPER_STATUS, std::string(LP_OPER_STATUS_STR)},
      {LP_CTR_VALID, std::string(LP_CTR_VALID_STR)},
      {LMP_SWITCH_ID, std::string(LMP_SWITCH_ID_STR)},
      {LMP_PHYSICAL_PORT_ID, std::string(LMP_PHYSICAL_PORT_ID_STR)},
      {LMP_LP_PORT_ID, std::string(LMP_LP_PORT_ID_STR)},
      {SWITCH_ID, std::string(SWITCH_ID_STR)},
      {SWITCH_DESCRIPTION, std::string(SWITCH_DESCRIPTION_STR)},
      {SWITCH_MODEL, std::string(SWITCH_MODEL_STR)},
      {SWITCH_IP_ADDRESS, std::string(SWITCH_IP_ADDRESS_STR)},
      {SWITCH_IPV6_ADDRESS, std::string(SWITCH_IPV6_ADDRESS_STR)},
      {SWITCH_ADMIN_STATUS, std::string(SWITCH_ADMIN_STATUS_STR)},
      {SWITCH_DOMAIN_NAME, std::string(SWITCH_DOMAIN_NAME_STR)},
      {SWITCH_MANUFACTURER, std::string(SWITCH_MANUFACTURER_STR)},
      {SWITCH_HARDWARE, std::string(SWITCH_HARDWARE_STR)},
      {SWITCH_SOFTWARE, std::string(SWITCH_SOFTWARE_STR)},
      {SWITCH_ALARM_STATUS, std::string(SWITCH_ALARM_STATUS_STR)},
      {SWITCH_OPER_STATUS, std::string(SWITCH_OPER_STATUS_STR)},
      {SWITCH_VALID, std::string(SWITCH_VALID_STR)},
      {PORT_ID, std::string(PORT_ID_STR)},
      {PORT_NUMBER, std::string(PORT_NUMBER_STR)},
      {PORT_DESCRIPTION, std::string(PORT_DESCRIPTION_STR)},
      {PORT_ADMIN_STATUS, std::string(PORT_ADMIN_STATUS_STR)},
      {PORT_DIRECTION, std::string(PORT_DIRECTION_STR)},
      {PORT_TRUNK_ALL_VLAN, std::string(PORT_TRUNK_ALL_VLAN_STR)},
      {PORT_OPER_STATUS, std::string(PORT_OPER_STATUS_STR)},
      {PORT_MAC_ADDRESS, std::string(PORT_MAC_ADDRESS_STR)},
      {PORT_DUPLEX, std::string(PORT_DUPLEX_STR)},
      {PORT_SPEED, std::string(PORT_SPEED_STR)},
      {PORT_ALARM_STATUS, std::string(PORT_ALARM_STATUS_STR)},
      {PORT_LOGIC_PORT_ID, std::string(PORT_LOGIC_PORT_ID_STR)},
      {PORT_VALID , std::string(PORT_VALID_STR)},
      {LINK_SWITCH_ID1, std::string(LINK_SWITCH_ID1_STR)},
      {LINK_PORT_ID1, std::string(LINK_PORT_ID1_STR)},
      {LINK_SWITCH_ID2, std::string(LINK_SWITCH_ID2_STR)},
      {LINK_PORT_ID2, std::string(LINK_PORT_ID2_STR)},
      {LINK_DESCRIPTION, std::string(LINK_DESCRIPTION_STR)},
      {LINK_OPER_STATUS, std::string(LINK_OPER_STATUS_STR)},
      {LINK_VALID, std::string(LINK_VALID_STR)},
      {BDRY_ID, std::string(BDRY_ID_STR)},
      {BDRY_DESCRIPTION, std::string(BDRY_DESCRIPTION_STR)},
      {BDRY_CTR_NAME1, std::string(BDRY_CTR_NAME1_STR)},
      {BDRY_DM_NAME1, std::string(BDRY_DM_NAME1_STR)},
      {BDRY_PORT_ID1, std::string(BDRY_PORT_ID1_STR)},
      {BDRY_CTR_NAME2, std::string(BDRY_CTR_NAME2_STR)},
      {BDRY_DM_NAME2, std::string(BDRY_DM_NAME2_STR)},
      {BDRY_PORT_ID2, std::string(BDRY_PORT_ID2_STR)},
      {BDRY_OPER_STATUS, std::string(BDRY_OPER_STATUS_STR)},
      {BDRY_VALID, std::string(BDRY_VALID_STR)},
      {BDRY_ROW_STATUS, std::string(BDRY_ROW_STATUS_STR)},
      {BDRY_ATTR, std::string(BDRY_ATTR_STR)}
  };
  /** Initialise the local variables */
  uint32_t loop  = 0;
  uint32_t count = 0;
  std::pair<std::map<ODBCMTableColumns,
  std::string>::iterator, bool> table_column_names;
  /** Check map is empty or not */
  if (ODBCManager::odbcm_tables_column_map_.empty()) {
    count = sizeof(odbcm_table_columns)/sizeof(OdbcmColumnName);
    pfc_log_debug("ODBCM::ODBCManager:%s : count %d", __func__, count);
    /** Traverse and insert in the map */
    for (loop = 0; loop < count; loop++) {
      table_column_names = odbcm_tables_column_map_.insert(
          std::pair<ODBCMTableColumns, std::string>
      (odbcm_table_columns[loop].column_id,
       odbcm_table_columns[loop].column_string));
      if (table_column_names.second == false) {
        pfc_log_debug("ODBCM::ODBCManager:%s "
            "Error in inserting odbcm_tables_column_map_ %s", __func__,
            odbcm_table_columns[loop].column_string.c_str());
        return ODBCM_RC_FAILED;
      }  // if
    }  // for
  }  // if
  return ODBCM_RC_SUCCESS;
}
/**EOF*/
