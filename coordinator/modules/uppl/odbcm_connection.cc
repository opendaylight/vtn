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

using unc::uppl::ODBCManager;
using unc::uppl::ODBCMTable;
using unc::uppl::QueryFactory;
using unc::uppl::QueryProcessor;
using unc::uppl::DBVarbind;
extern pfc_cfdef_t odbcm_cfdef;

/* ODBCManager instance */
static ODBCManager *ODBCManager_ = NULL;

/**
 * @Description : Constructor function which creates/initializes
 *                single instance of ODBCManager
 * @param[in]   : None
 * @return      : void
 **/
ODBCManager::ODBCManager() {
  pfc_log_info("ODBCM::ODBCManager:: Constructor to initialize the members");
  /** Initialize the ODBCManager members */
  rw_conn_handle_ = NULL, ro_conn_handle_ = NULL, phy_conn_env_ = NULL;
  //  initialize the ODBCMInit flag
  IsODBCManager_initialized = 0;
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
  /** Close the db connection */
  if (ODBCM_RC_SUCCESS != CloseDBConnection())
    pfc_log_error("ODBCM::~ODBCManager:: Error in CloseDBConnection");
  else
    pfc_log_info("ODBCM::~ODBCManager:: DB connection closed successfully");

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
}
/**
 * @Description : This function will return the ODBC read-write,
 * read-only connection environment and it's a const method.
 * @param[in]   : None
 * @return      : SQLHENV
 **/
inline SQLHENV ODBCManager::get_phy_connenction_env_() const {
  return phy_conn_env_;
}
/**
 * @Description : This function will return the ODBC read-write connection
 * handle and it's a const method.
 * @param[in]   : None
 * @return      : SQLHDBC
 **/
inline SQLHDBC ODBCManager::get_rw_connenction_handle_() const {
  return rw_conn_handle_;
}
/**
 * @Description : This function will return the ODBC read-only
 * connection handle and it's a const method.
 * @param[in]   : None
 * @return      : SQLHENV
 **/
inline SQLHDBC ODBCManager::get_ro_connenction_handle_() const {
  return ro_conn_handle_;
}
/**
 * @Description : This static method will return the singleton
 * ODBCManager instance if exists else create new one and return.
 * @param[in]   : None
 * @return      : ODBCManager*
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
 * @return      : db_table_list_map_
 **/
std::map<int,  std::vector<std::string> >
                        ODBCManager::get_db_table_list_map_() const {
  return db_table_list_map_;
}
/**
 * @Description : function to set read-write ODBC connection handle
 * @param[in]   : None
 * @return      : ODBCM_RC_STATUS
 **/
inline ODBCM_RC_STATUS ODBCManager::set_rw_connenction_handle_() {
  // 2. allocate rw connection handle
  SQLRETURN odbc_rc = SQLAllocHandle(SQL_HANDLE_DBC, phy_conn_env_,
      &rw_conn_handle_);
  ODBCM_DBC_HANDLE_CHECK(phy_conn_env_, odbc_rc);
  if (odbc_rc != ODBCM_RC_SUCCESS && odbc_rc != ODBCM_RC_SUCCESS_WITH_INFO) {
    pfc_log_fatal("ODBCM::ODBCManager::set_rw_connenction_handle_:"
        " Error in SQLAllocHandle %s",
        ODBCMUtils::get_RC_Details(odbc_rc).c_str());
    return (ODBCM_RC_STATUS) odbc_rc;
  }
  pfc_log_info("ODBCM::ODBCManager::set_rw_connenction_handle_: Success");
  return ODBCM_RC_SUCCESS;
}
/**
 * @Description : function to set read ODBC connection handle
 * @param[in]   : None
 * @return      : ODBCM_RC_STATUS
 **/
inline ODBCM_RC_STATUS ODBCManager::set_ro_connenction_handle_() {
  // 2. allocate ro connection handle
  SQLRETURN odbc_rc = SQLAllocHandle(SQL_HANDLE_DBC, phy_conn_env_,
                                   &ro_conn_handle_);
  ODBCM_DBC_HANDLE_CHECK(phy_conn_env_, odbc_rc);
  if (odbc_rc != ODBCM_RC_SUCCESS && odbc_rc != ODBCM_RC_SUCCESS_WITH_INFO) {
    pfc_log_fatal("ODBCM::ODBCManager::set_ro_connenction_handle_:"
        " Error in SQLAllocHandle %s",
        ODBCMUtils::get_RC_Details(odbc_rc).c_str());
    return (ODBCM_RC_STATUS) odbc_rc;
  }
  pfc_log_info("ODBCM::ODBCManager::set_ro_connenction_handle_: Success");
  return ODBCM_RC_SUCCESS;
}
/**
 * @Description : function to set read-write ODBC connection environment
 * @param[in]   : None
 * @return      : ODBCM_RC_STATUS
 **/
inline ODBCM_RC_STATUS ODBCManager::set_phy_connenction_env_() {
  // 1. allocate Environment handle and register version
  SQLRETURN odbc_rc = ODBCM_RC_SUCCESS;
  odbc_rc = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &phy_conn_env_);
  ODBCM_ENV_HANDLE_CHECK(phy_conn_env_, odbc_rc);
  if (odbc_rc != ODBCM_RC_SUCCESS && odbc_rc != ODBCM_RC_SUCCESS_WITH_INFO) {
    pfc_log_fatal("ODBCM::ODBCManager::set_phy_connenction_env_: "
        "Error in phy_conn_env_ %s",
        ODBCMUtils::get_RC_Details(odbc_rc).c_str());
    return (ODBCM_RC_STATUS) odbc_rc;
  }
  pfc_log_info("ODBCM::ODBCManager::set_phy_connenction_env_: "
    "phy_conn_env_ is allocated");
  odbc_rc = SQLSetEnvAttr(phy_conn_env_,  SQL_ATTR_ODBC_VERSION,
                          reinterpret_cast<void*>(SQL_OV_ODBC3), 0);
  ODBCM_ENV_HANDLE_CHECK(phy_conn_env_, odbc_rc);
  if (odbc_rc != ODBCM_RC_SUCCESS && odbc_rc != ODBCM_RC_SUCCESS_WITH_INFO) {
    pfc_log_fatal("ODBCM::ODBCManager::set_phy_connenction_env_:"
        " Error in SQLSetEnvAttr %s",
        ODBCMUtils::get_RC_Details(odbc_rc).c_str());
    return (ODBCM_RC_STATUS) odbc_rc;
  }
  pfc_log_info("ODBCM::ODBCManager::set_phy_connenction_env_: Success");
  return ODBCM_RC_SUCCESS;
}

/**
 * @Description : Function to initialize the DB table list map
 * @param[in]   : None
 * @return      : ODBCM_RC_STATUS
 **/
ODBCM_RC_STATUS ODBCManager::initialize_db_table_list_map_() {
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
 @Description : This function will parse the configuration file and
 * update the local structure db_conf_info. this structure will be used
 * during connection establishment.
 * @param[in]   : None
 * @return      : ODBCM_RC_STATUS
 **/
ODBCM_RC_STATUS ODBCManager::ParseConfigFile() {
  std::string conf_file_path;
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
 * odbcmanager and RDBMS system
 * @param[in]   : None
 * @return      : ODBCM_RC_STATUS
 **/
ODBCM_RC_STATUS ODBCManager::OpenDBConnection() {
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
    pfc_log_debug("ODBCM::ODBCManager::OpenDBConnection: Error in "
                  "SQLSetEnvAttr in connection pooling activation");
    return ODBCM_RC_CONNECTION_ERROR;
  }

  if (set_phy_connenction_env_() != ODBCM_RC_SUCCESS) {
    pfc_log_fatal("ODBCM::ODBCManager::OpenDBConnection: "
                  "Error in set_phy_connenction_env_ ");
    return ODBCM_RC_CONN_ENV_ERROR;
  }

  if (set_rw_connenction_handle_() != ODBCM_RC_SUCCESS) {
    pfc_log_fatal("ODBCM::ODBCManager::OpenDBConnection: "
                  "Error in set_rw_connenction_handle_ ");
    return ODBCM_RC_CONNECTION_ERROR;
  }
  if (set_ro_connenction_handle_() != ODBCM_RC_SUCCESS) {
    pfc_log_fatal("ODBCM::ODBCManager::OpenDBConnection: "
                  "Error in set_ro_connenction_handle_ ");
    return ODBCM_RC_CONNECTION_ERROR;
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

  /*********************RW Connection **************************************/
  std::ostringstream conn_string_stream;
  conn_string_stream << "DSN=" << conf_parser_.dsn <<
  ";TIMEOUT=" << conf_parser_.time_out;

  std::string conn_string = conn_string_stream.str();
  pfc_log_info("ODBCM::ODBCManager::OpenDBConnection: "
      "rw connection string = %s", conn_string.c_str());

  // 3. Connect to the datasource
  if (NULL != rw_conn_handle_)
    odbc_rc = SQLDriverConnect(rw_conn_handle_, NULL,
                (unsigned char*)(conn_string.c_str()),
                SQL_NTS, NULL, 0, NULL, SQL_DRIVER_NOPROMPT);
  //  SQL_DRIVER_COMPLETE
  ODBCM_DBC_HANDLE_CHECK(rw_conn_handle_, odbc_rc);
  if (odbc_rc != ODBCM_RC_SUCCESS &&
      odbc_rc != ODBCM_RC_SUCCESS_WITH_INFO) {
    pfc_log_fatal("ODBCM::ODBCManager::OpenDBConnection:"
                  "Could not establish RW connection !!");
    return ODBCM_RC_CONNECTION_ERROR;
  }
  //  print connection information
  ODBCMUtils::print_odbc_details(rw_conn_handle_);
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
  odbc_rc = SQLSetConnectAttr(rw_conn_handle_, SQL_ATTR_TXN_ISOLATION,
              (SQLPOINTER)SQL_TXN_READ_COMMITTED, 0);
  ODBCM_DBC_HANDLE_CHECK(rw_conn_handle_, odbc_rc);
  if (odbc_rc != ODBCM_RC_SUCCESS &&
      odbc_rc != ODBCM_RC_SUCCESS_WITH_INFO) {
    pfc_log_debug("ODBCM::ODBCManager::OpenDBConnection: "
      "Error in SQLSetConnectAttr");
    return ODBCM_RC_CONNECTION_ERROR;
  }

  odbc_rc = SQLSetConnectAttr(rw_conn_handle_, SQL_ATTR_AUTOCOMMIT,
              (SQLPOINTER)SQL_AUTOCOMMIT_OFF, SQL_NTS);
  ODBCM_DBC_HANDLE_CHECK(rw_conn_handle_, odbc_rc);
  if (odbc_rc != ODBCM_RC_SUCCESS &&
      odbc_rc != ODBCM_RC_SUCCESS_WITH_INFO) {
    pfc_log_debug("ODBCM::ODBCManager::OpenDBConnection: "
      "Error in SQLSetConnectAttr");
    return ODBCM_RC_CONNECTION_ERROR;
  }

  odbc_rc = SQLSetConnectAttr(rw_conn_handle_, SQL_ATTR_TRACE,
              (SQLPOINTER)SQL_OPT_TRACE_ON, SQL_NTS);
  ODBCM_DBC_HANDLE_CHECK(rw_conn_handle_, odbc_rc);
  if (odbc_rc != ODBCM_RC_SUCCESS &&
      odbc_rc != ODBCM_RC_SUCCESS_WITH_INFO) {
    pfc_log_debug("ODBCM::ODBCManager::OpenDBConnection: "
      "Error in SQLSetConnectAttr");
    return ODBCM_RC_CONNECTION_ERROR;
  }
  pfc_log_info("ODBCM::ODBCManager::OpenDBConnection: "
    "rw database connection creation success");
  /*********************RW Connection End************************************/

  /*********************RO Connection**************************************/
  conn_string_stream.str("");
  conn_string_stream << "DSN=" << conf_parser_.dsn <<
  ";TIMEOUT=" << conf_parser_.time_out;
  conn_string = "";
  conn_string = conn_string_stream.str();
  pfc_log_info("ODBCM::ODBCManager::OpenDBConnection: "
      "ro connection string = %s", conn_string.c_str());

  if (NULL != ro_conn_handle_)
    odbc_rc = SQLDriverConnect(ro_conn_handle_, NULL,
              (unsigned char*)(conn_string.c_str()),
              SQL_NTS, NULL, 0, NULL, SQL_DRIVER_NOPROMPT);
  ODBCM_DBC_HANDLE_CHECK(ro_conn_handle_, odbc_rc);
  if (odbc_rc != ODBCM_RC_SUCCESS &&
      odbc_rc != ODBCM_RC_SUCCESS_WITH_INFO) {
    pfc_log_fatal("ODBCM::ODBCManager::OpenDBConnection: "
      "Could not establish RW connection !!");
    return ODBCM_RC_CONNECTION_ERROR;
  }
  /**  print connection information */
  ODBCMUtils::print_odbc_details(ro_conn_handle_);

  odbc_rc = SQLSetConnectAttr(ro_conn_handle_, SQL_ATTR_TXN_ISOLATION,
              (SQLPOINTER)SQL_TXN_READ_COMMITTED, 0);
  ODBCM_DBC_HANDLE_CHECK(ro_conn_handle_, odbc_rc);
  if (odbc_rc != ODBCM_RC_SUCCESS &&
      odbc_rc != ODBCM_RC_SUCCESS_WITH_INFO) {
    pfc_log_debug("ODBCM::ODBCManager::OpenDBConnection: "
      "Error in ro SQLSetConnectAttr");
    return ODBCM_RC_CONNECTION_ERROR;
  }

  odbc_rc = SQLSetConnectAttr(ro_conn_handle_, SQL_ATTR_TRACE,
              (SQLPOINTER)SQL_OPT_TRACE_ON, SQL_NTS);
  ODBCM_DBC_HANDLE_CHECK(ro_conn_handle_, odbc_rc);
  if (odbc_rc != ODBCM_RC_SUCCESS &&
      odbc_rc != ODBCM_RC_SUCCESS_WITH_INFO) {
    pfc_log_debug("ODBCM::ODBCManager::OpenDBConnection: "
      "Error in SQLSetConnectAttr");
    return ODBCM_RC_CONNECTION_ERROR;
  }
  /**********************RO Connection End***********************************/
  pfc_log_info("ODBCM::ODBCManager::OpenDBConnection: "
    "ro database connection creation success");
  return ODBCM_RC_SUCCESS;
}
/**
 * @Description : This function will close the existing database connection
 * and reconnect the db connection
 * @param[in]   : None
 * @return      : ODBCM_RC_STATUS
 **/
ODBCM_RC_STATUS ODBCManager::ReconnectDB() {
  if (CloseDBConnection() != ODBCM_RC_SUCCESS) {
    pfc_log_info("ODBCM::ODBCManager::ReconnectDB:"
       "Error on CloseDBConnection");
  }
  /* Establish the database connection */
  if (ODBCM_RC_SUCCESS != OpenDBConnection()) {
    pfc_log_fatal("ODBCM::ODBCManager::ReconnectDB: "
        "Could not establish Database Connection !!");
    return ODBCM_RC_CONNECTION_ERROR;
  } else {
    pfc_log_info("ODBCM::ODBCManager::ReconnectDB:"
     "Database is reconnected successfully");
  }
  return ODBCM_RC_SUCCESS;
}

/**
 * @Description : This function will close the existing database connection
 * @param[in]   : None
 * @return      : ODBCM_RC_STATUS
 **/
ODBCM_RC_STATUS ODBCManager::CloseDBConnection() {
  SQLRETURN odbc_rc = 0;
  /*  free handler */
  pfc_log_info("ODBCM::ODBCManager::CloseDBConnection: "
    "Freeing handler");
  /*  to disconnect */
  if (NULL != rw_conn_handle_) {
    odbc_rc = SQLDisconnect(rw_conn_handle_);
    odbc_rc = SQLFreeHandle(SQL_HANDLE_DBC, rw_conn_handle_);
    pfc_log_info("ODBCM::ODBCManager::CloseDBConnection: "
      "Disconnect rw_conn_handle_");
    rw_conn_handle_ = NULL;
    if (odbc_rc != ODBCM_RC_SUCCESS) {
      pfc_log_error("ODBCM::ODBCManager::CloseDBConnection: "
        "Error on Disconnect rw_conn_handle_");
      return (ODBCM_RC_STATUS) odbc_rc;
    }
  }
  if (NULL != ro_conn_handle_) {
    odbc_rc = SQLDisconnect(ro_conn_handle_);
    odbc_rc = SQLFreeHandle(SQL_HANDLE_DBC, ro_conn_handle_);
    pfc_log_info("ODBCM::ODBCManager::CloseDBConnection: "
      "Disconnect ro_conn_handle_");
    ro_conn_handle_ = NULL;
    if (odbc_rc != ODBCM_RC_SUCCESS) {
      pfc_log_error("ODBCM::ODBCManager::CloseDBConnection: "
        "Error on Disconnect ro_conn_handle_");
      return (ODBCM_RC_STATUS) odbc_rc;
    }
  }
  if (NULL != phy_conn_env_) {
    odbc_rc = SQLFreeHandle(SQL_HANDLE_ENV,  phy_conn_env_);
    pfc_log_info("ODBCM::ODBCManager:: Disconnect phy_conn_env_");
    phy_conn_env_ = NULL;
  }
  pfc_log_info("ODBCM::ODBCManager::CloseDBConnection: "
    "Now, database disconnected...");
  return ODBCM_RC_SUCCESS;
}

/**
 * @Description : Get the table id from table name
 * @param[in]   : string table_name
 * @return      : ODBCMTable
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
 * @param[in]   : STMT , ODBCMTable ,DBVarbind ,QueryFactory ,QueryProcessor
 * @return      : ODBCM_RC_STATUS
 **/
void ODBCManager::ODBCMFreeingMemory(HSTMT stmt,
                        ODBCMTable table_id,
                        DBVarbind *dbvarbindobj,
                        QueryFactory *qfactoryobj,
                        QueryProcessor *qprocessorobj) {
  SQLRETURN odbc_rc = 0;
  /**  statement handler free */
  //  ODBCM_FREE_STMT(stmt);
  if (NULL != stmt) {
    odbc_rc = SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    stmt = NULL;
  } else {
    pfc_log_debug("ODBCM::ODBCManager::ODBCMFreeingMemory:stmt is NULL");
  }
  if (odbc_rc != ODBCM_RC_SUCCESS) {
    pfc_log_error("ODBCM::ODBCManager::ODBCMFreeingMemory: Error while "
        "Freeing resources");
    return;
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
/**EOF*/
