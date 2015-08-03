/*
 * Copyright (c) 2012-2015 NEC Corporation
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
using unc::uppl::ODBCMUtils;
extern pfc_cfdef_t odbcm_cfdef;

/* static variable initialization */
ODBCManager *ODBCManager::ODBCManager_ = NULL;
int ODBCMUtils::sem_id = 0;

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
          phy_conn_env_(NULL),
          conn_max_limit_(0) {
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
  FreeingConnections(true);
  ODBCMUtils::del_semvalue();

  /** Free the uppl db connection and environments */
  ODBCM_RC_STATUS rw_disconn_status = CloseRwConnection();
  pfc_log_debug("ODBCM::ODBCManager::CloseRwConnection Status %d",
                rw_disconn_status);
  if (NULL != phy_conn_env_) {
    SQLFreeHandle(SQL_HANDLE_ENV,  phy_conn_env_);
    pfc_log_info("ODBCM::ODBCManager:: Disconnect phy_conn_env_");
    phy_conn_env_ = NULL;
  }
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
  pfc_log_info("ODBCM::~ODBCManager: Destructor work is completed");
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
      UPPL_LOG_FATAL("ODBCM::ODBCManager::get_ODBCManager: "
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
  conf_file_path.append(CONF_FILE_PATH_SEP);
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
  conf_parser_.fip_dsn     = logblock.getString("db_fip_dsn", "UNC_DB_DSN");
  conf_parser_.local_dsn     = logblock.getString("db_local_dsn",
                                                  "UNC_DB_LC1_DSN");
  conf_parser_.time_out        = logblock.getUint32("time_out", 10);

  ODBCM_PRINT_DEBUG_LOG("%s", conf_parser_.fip_dsn.c_str());
  ODBCM_PRINT_DEBUG_LOG("%s", conf_parser_.local_dsn.c_str());
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
  std::ostringstream conn_string_stream;
  std::string conn_string;
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
        UPPL_LOG_FATAL("ODBCM::ODBCManager::OpenDBConnection: "
            "Error in set_rw_connection_handle_ ");
        return ODBCM_RC_CONNECTION_ERROR;
      }
      conn_string_stream << "DSN=" << conf_parser_.fip_dsn <<
      ";TIMEOUT=" << conf_parser_.time_out;
      conn_string = conn_string_stream.str();
       pfc_log_debug("ODBCM::ODBCManager::OpenDBConnection: "
             "connection string = %s", conn_string.c_str());
  // 3. Connect to the datasource
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
        UPPL_LOG_FATAL("ODBCM::ODBCManager::OpenDBConnection: "
            "Error in set_rw_connection_handle_ ");
        return ODBCM_RC_CONNECTION_ERROR;
      }
      conn_string_stream << "DSN=" << conf_parser_.fip_dsn <<
          ";TIMEOUT=" << conf_parser_.time_out;
      conn_string = conn_string_stream.str();
      pfc_log_debug("ODBCM::ODBCManager::OpenDBConnection: "
           "connection string = %s", conn_string.c_str());
  // 3. Connect to the datasource
      /**  setting SQL_ATTR_ACCESS_MODE for RW connection is not required
       *  SQL_MODE_READ_WRITE is default */
      break;
    case kOdbcmConnReadOnly:
      if (set_ro_connection_handle_(conn_handle) != ODBCM_RC_SUCCESS) {
        pfc_log_error("ODBCM::ODBCManager::OpenDBConnection: "
            "Error in set_ro_connection_handle_ ");
        return ODBCM_RC_CONNECTION_ERROR;
      }
      conn_string_stream << "DSN=" << conf_parser_.local_dsn <<
         ";TIMEOUT=" << conf_parser_.time_out;
      conn_string = conn_string_stream.str();
      pfc_log_debug("ODBCM::ODBCManager::OpenDBConnection: "
        "connection string = %s", conn_string.c_str());
  // 3. Connect to the datasource
      odbc_rc = SQLSetConnectAttr(conn_handle, SQL_ATTR_ACCESS_MODE,
                                  (SQLPOINTER)SQL_MODE_READ_ONLY, 0);
      ODBCM_DBC_HANDLE_CHECK(conn_handle, odbc_rc);
      if (odbc_rc != ODBCM_RC_SUCCESS &&
          odbc_rc != ODBCM_RC_SUCCESS_WITH_INFO) {
        pfc_log_info("ODBCM::ODBCManager::OpenDBConnection: "
            "Error in SQLSetConnectAttr");
        return ODBCM_RC_CONNECTION_ERROR;
      }
      break;
    default:
      pfc_log_error("ODBCM::ODBCManager::OpenDBConnection: "
          "Invalid connection type %d !! ",
          conn_obj->get_conn_type());
      return ODBCM_RC_CONNECTION_ERROR;
  }
  // *************************************************************************
  // Set the matching condition for using an existing connection in the pool
  odbc_rc = SQLSetEnvAttr(phy_conn_env_, SQL_ATTR_CP_MATCH,
                          (SQLPOINTER)SQL_CP_RELAXED_MATCH, SQL_IS_INTEGER);
  if (odbc_rc != ODBCM_RC_SUCCESS && odbc_rc != ODBCM_RC_SUCCESS_WITH_INFO) {
    pfc_log_info("ODBCM::ODBCManager::OpenDBConnection: "
        "Error in SQLSetEnvAttr during connec. pooling activation");
    return ODBCM_RC_CONNECTION_ERROR;
  }
  //  create and pool the connections.

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
    conn_handle = NULL;
    conn_obj->set_conn_handle(conn_handle);
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
    pfc_log_info("ODBCM::ODBCManager::OpenDBConnection: "
        "Error in SQLSetConnectAttr");
    conn_handle = NULL;
    conn_obj->set_conn_handle(conn_handle);
    return ODBCM_RC_CONNECTION_ERROR;
  }

  odbc_rc = SQLSetConnectAttr(conn_handle, SQL_ATTR_AUTOCOMMIT,
                              (SQLPOINTER)SQL_AUTOCOMMIT_OFF, SQL_NTS);
  ODBCM_DBC_HANDLE_CHECK(conn_handle, odbc_rc);
  if (odbc_rc != ODBCM_RC_SUCCESS &&
      odbc_rc != ODBCM_RC_SUCCESS_WITH_INFO) {
    pfc_log_info("ODBCM::ODBCManager::OpenDBConnection: "
        "Error in SQLSetConnectAttr");
    conn_handle = NULL;
    conn_obj->set_conn_handle(conn_handle);
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
 @Description : When NB request comes, this function assigns db conn.
 *              to service the request. If the free pool has connection,
 *              that shall be assigned, if free pool does not have conn.
 *              it will create and assign new connection, if the exisitng
 *              no. of conns. not reached to conn.max.limit.if max conn.limit
 *              reached, subsequent requests shall wait (using semaphore) for
 *              db connection.
 *              If read request from configure mode shall be serviced by
 *              RW conn. only.
 * @param[in]  : uint32_t session_id, uint32_t config_id
 * @param[out] : OdbcmConnectionHandler *&db_conn
 * @return     : ODBCM_RC_STATUS
 **/
ODBCM_RC_STATUS ODBCManager::AssignDBConnection(
                             OdbcmConnectionHandler *&db_conn,
                             uint32_t session_id, uint32_t config_id) {
  pfc_log_trace("Entered into AssignDBConnection"
                " session_id = %d, config_id = %d", session_id, config_id);
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  UncRespCode db_ret = UNC_RC_SUCCESS;
  /**RW conn shall be reused for READ which is op belongs to same session*/
  //  if config_id is > 0, it is config mode read request.
  PhysicalCore* physical_core = physical_layer->get_physical_core();
  if (config_id != 0 && 
      physical_core->get_system_state() != UPPL_SYSTEM_ST_ACTIVE) {
    pfc_log_trace("Invalid config_id RW conn is not available in SBY node");
    return ODBCM_RC_CONNECTION_ERROR;
  }
  physical_layer->db_conpool_mutex_.lock();
  //  if session_id = USESS_ID_UPLL then read request should use RO conn only
  if (config_id > 0 && session_id != USESS_ID_UPLL) {
    pfc_log_trace("RW conn req. for config mode READ operation");
    if (rw_nb_conn_obj_ != NULL) {
      db_conn = rw_nb_conn_obj_;
      pfc_log_trace("RW conn is assigned for READ operation");
    } else {  /*if RW connection not available allocate conn handle and return*/
      pfc_log_trace("RW conn is NULL !!, Open RW connection and store/"
                                      "assign for read req.");
      // retry RW connection creation
      rw_nb_conn_obj_ = new OdbcmConnectionHandler(
                            unc::uppl::kOdbcmConnReadWriteNb,
                            db_ret,
                            PhysicalLayer::get_instance()->get_odbc_manager());
      if (db_ret == UNC_RC_SUCCESS && rw_nb_conn_obj_ != NULL) {
        db_conn = rw_nb_conn_obj_;
      } else {
        pfc_log_error("RW connection assignation failed!! conn. not available");
        physical_layer->db_conpool_mutex_.unlock();
        return ODBCM_RC_CONNECTION_ERROR;
      }
      pfc_log_trace("RW conn is allocated and assigned for READ operation");
    }
    physical_layer->db_conpool_mutex_.unlock();
    return ODBCM_RC_SUCCESS;
  }

  pfc::core::Thread *th_self = pfc::core::Thread::self();
  uint32_t th_id = th_self->getId();
  delete th_self;
  th_self = NULL;
  // finding conn with same session id exists  - debug purpose.
  std::map<uint64_t, OdbcmConnectionHandler*>::iterator cpool_iter =
                               conpool_inuse_map_.begin();
  uint64_t use_session_id_ = session_id;
  use_session_id_ = (use_session_id_ << 32) + th_id;
  pfc_log_debug("CXN - use_session_id_ (session_id + thread_id) = %"
                       PFC_PFMT_u64, use_session_id_);

  cpool_iter = conpool_inuse_map_.find(use_session_id_);
  if (cpool_iter != conpool_inuse_map_.end()) {
    pfc_log_error("conn. for session_id %d th_id %d,"
        "is already exists SHOULDNOT", session_id, th_id);
  }

  //  if config id < 0 - READ operation
  if (!conpool_free_list_.empty()) {
    //  take connection from list assign to request.
    db_conn = conpool_free_list_.front();
    pfc_log_trace("Free %" PFC_PFMT_SIZE_T " RO conn(s) available in conn pool "
             "and 1 is assigned for READ operation", conpool_free_list_.size());
    db_conn->set_using_session_id(session_id, th_id);
    conpool_inuse_map_[db_conn->get_using_session_id()] = db_conn;
    conpool_free_list_.pop_front();  // free pool is poped
    pfc_log_trace("Existing RO conn is assgn. for req sess_id.thread_id = %"
                  PFC_PFMT_u64, db_conn->get_using_session_id());
  } else {
    pfc_log_trace("Free RO conn is NOT available in conn pool");
    //  create new connection if no.of conn does not reach conn_max_limit
    pfc_log_debug("In use conn pool size = %" PFC_PFMT_SIZE_T,
                                           conpool_inuse_map_.size());
    pfc_log_debug("max allowed no. of conn  = %d", conn_max_limit_);
    if (conpool_inuse_map_.size() < conn_max_limit_) {
      db_conn = new OdbcmConnectionHandler(unc::uppl::kOdbcmConnReadOnly,
            db_ret, PhysicalLayer::get_instance()->get_odbc_manager());
      if (db_ret != UNC_RC_SUCCESS) {
        pfc_log_error("db RO connection creation is failed.!!");
        //  In case of error in Read connection, all unused Read connections
        physical_layer->db_conpool_mutex_.unlock();
        return ODBCM_RC_CONNECTION_ERROR;
      }
      db_conn->set_using_session_id(session_id, th_id);
      conpool_inuse_map_[db_conn->get_using_session_id()] = db_conn;
      pfc_log_trace("RO conn is created and assgined for request sess_id = %"
                    PFC_PFMT_u64, db_conn->get_using_session_id());
    } else {
      /*put request on WAIT state using semaphore. WAIT state shall be 
      * release !SEM_UP() after any one of the read connection is freed */

      ODBCMUtils::sem_id  = semget((key_t)1234, 1, 0666 | IPC_CREAT);
      pfc_log_trace("READ will be blocked by Semaphore SEM_ID = %d",
                                              ODBCMUtils::sem_id);
      do {
        if (!ODBCMUtils::set_semvalue(0)) {  //  request will be blocked
          pfc_log_info("Semaphore initialized failed!");
          physical_layer->db_conpool_mutex_.unlock();
          return ODBCM_RC_FAILED;
        }
        pfc_log_trace("READ Entered into WAIT state"
                " session_id = %d, config_id = %d", session_id, config_id);
        physical_layer->db_conpool_mutex_.unlock();
        if (!ODBCMUtils::SEM_DOWN()) {
          pfc_log_info("entering critical section failed!");
          return ODBCM_RC_FAILED;
        }
        pfc_log_trace("SEM UP is done!! DB Connection will be assigned... ");
        physical_layer->db_conpool_mutex_.lock();
        //  take connection from list assign to request.
        if (conpool_free_list_.empty()) {
          pfc_log_info("After SEM release, no conn available in free pool");
        }
      } while (conpool_free_list_.empty());
      db_conn = conpool_free_list_.front();
      pfc_log_trace("Free %" PFC_PFMT_SIZE_T " RO conn(s) available"
                    " in conn pool and 1 is assigned for READ operation",
                    conpool_free_list_.size());
      db_conn->set_using_session_id(session_id, th_id);
      conpool_inuse_map_[db_conn->get_using_session_id()] = db_conn;
      conpool_free_list_.pop_front();  // free pool is poped
      pfc_log_trace("Freed RO conn is assgined for request sess_id = %"
                    PFC_PFMT_u64,
                    db_conn->get_using_session_id());
    }
  }
  physical_layer->db_conpool_mutex_.unlock();
  // Commented as fix for REVERSE_INULL coverity error
  /*if (db_conn == NULL) {
    pfc_log_info("After SEM release, db_conn is NULL");
    return ODBCM_RC_FAILED;
  }*/
  pfc_log_debug("db_conn type is %d", db_conn->get_conn_type());
  return ODBCM_RC_SUCCESS;
}


/**
* @Description : when the read request completes the operation, the db conn
*                will be pooled instead of disconnect and free them. if any
*                read request waiting for db conn. signal will be passed to
*                that request (by semaphore)
* @param[in]   : OdbcmConnectionHandler *&conn_obj, uint32_t session_id,
*                uint32_t config_id
* @return      : ODBCM_RC_STATUS
**/
ODBCM_RC_STATUS ODBCManager::PoolDBConnection(OdbcmConnectionHandler *&conn_obj,
                             uint32_t session_id, uint32_t config_id) {
  pfc_log_trace("PoolDBConnection - session_id = %d, config_id = %d",
                                            session_id, config_id);
  if (config_id > 0) {  // RW conn is used for configure sess. READ, NO-POOL.
    pfc_log_debug("Read uses RWconn,NO-POOL required (ENDTRANS-ROLLBACK DONE)");
    SQLHDBC conn_handle = conn_obj->get_conn_handle();
    PHY_SQLEXEC_LOCK();
    ODBCM_ROLLBACK_TRANSACTION(conn_handle);
    return ODBCM_RC_SUCCESS;
  }

  //  Freeing erroneous db connections
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  if (!err_connx_list_.empty()) {
    physical_layer->db_conpool_mutex_.lock();
    pfc_log_debug("Erroneous %" PFC_PFMT_SIZE_T
                  " RO Connection is present and about to free",
                  err_connx_list_.size());
    std::list<uint64_t>::iterator err_iter;
    err_iter = err_connx_list_.begin();
    std::map<uint64_t, OdbcmConnectionHandler*>::iterator cpool_iter =
                           conpool_inuse_map_.begin();
    for ( ; err_iter != err_connx_list_.end(); err_iter++) {
      if (!conpool_inuse_map_.empty()) {
        cpool_iter = conpool_inuse_map_.find(*err_iter);
        pfc_log_info("error conn found in pool map is %" PFC_PFMT_u64,
             (*err_iter));
        if (cpool_iter != conpool_inuse_map_.end()) {
          SQLHDBC conn_handle = conn_obj->get_conn_handle();
          ODBCM_ROLLBACK_TRANSACTION(conn_handle);
          delete (*cpool_iter).second;
          conpool_inuse_map_.erase(cpool_iter);
          pfc_log_debug("Err DB Conn is freed now..(ENDTRANS-ROLLBACK DONE)!!");
        }
      }
    }
    err_connx_list_.clear();
    physical_layer->db_conpool_mutex_.unlock();
  }

  PhysicalCore* physical_core = physical_layer->get_physical_core();
  if (physical_core->system_transit_state_ == true) {
    pfc_log_info("UNC is in transit state");
    FreeingConnections(false);
    return ODBCM_RC_FAILED;
  }
  physical_layer->db_conpool_mutex_.lock();

  std::map<uint64_t, OdbcmConnectionHandler*>::iterator cpool_iter =
                           conpool_inuse_map_.begin();
  bool process_waiting = false;

  if (!conpool_inuse_map_.empty()) {
    cpool_iter = conpool_inuse_map_.find(conn_obj->get_using_session_id());
    pfc_log_debug("session id to find conn in pool map is %" PFC_PFMT_u64,
        conn_obj->get_using_session_id());
  } else {
    pfc_log_error("conpool_inuse_map_ is empty !!");
    cpool_iter = conpool_inuse_map_.end();
  }

  if (conpool_inuse_map_.size() >= conn_max_limit_) {
    process_waiting = true;
    pfc_log_trace("Another READ shall be waiting for free DB conn !!");
  }

  if (cpool_iter != conpool_inuse_map_.end()) {
    SQLHDBC conn_handle = conn_obj->get_conn_handle();
    ODBCM_ROLLBACK_TRANSACTION(conn_handle);
    conpool_free_list_.push_back(conn_obj);
    conpool_inuse_map_.erase(cpool_iter);
    pfc_log_debug("DB Conn is pooled now..(ENDTRANS-ROLLBACK DONE)!!");
  } else { /*could be error case*/
    // free all unused db connections
    while (!conpool_free_list_.empty()) {
      delete conpool_free_list_.back();
      conpool_free_list_.pop_back();
    }
    ODBCMUtils::del_semvalue();
    pfc_log_error("DB Conn RECALL error!! all read connection shall be freed");
  }

  if (process_waiting == true) {
    pfc_log_info("release the critical section ! SEM_UP - session_id");
    if (!ODBCMUtils::SEM_UP()) {
      pfc_log_info("leaving from critical section failed!");
      physical_layer->db_conpool_mutex_.unlock();
      return ODBCM_RC_FAILED;
    }
    pfc_log_trace("semaphore UP is happened to signal the waiting req");
  }
  physical_layer->db_conpool_mutex_.unlock();
  return ODBCM_RC_SUCCESS;
}

/**
* @Description : this function will free the existing connections
*                if the IsAllOrUnused is true, used, unused conn.
*                will be freed and closed permanentely.
*                if the IsAllOrUnused is false, only unused conn.
*                will be freed and closed permanentely.
* @param[in]   : bool IsAllOrUnused
* @return      : ODBCM_RC_STATUS
**/
ODBCM_RC_STATUS ODBCManager::FreeingConnections(bool IsAllOrUnused) {
  pfc_log_trace("Freeing unused Connections ... ");
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  physical_layer->db_conpool_mutex_.lock();

  if (!conpool_free_list_.empty()) {
    pfc_log_debug("Unused %" PFC_PFMT_SIZE_T
                  " RO Connection is present and about to free",
                  conpool_free_list_.size());
    std::list<OdbcmConnectionHandler*>::iterator cpoolfree_iter;
    cpoolfree_iter = conpool_free_list_.begin();
    for ( ; cpoolfree_iter != conpool_free_list_.end(); cpoolfree_iter++) {
      OdbcmConnectionHandler *db_conn = *cpoolfree_iter;
      delete db_conn;  // destructor intern calls the closeDBconnection
      db_conn = NULL;
      pfc_log_debug("Unused RO Connection is freed");
    }
    conpool_free_list_.clear();
  }

  if (IsAllOrUnused != false) { /*All conn. freed including used one */
  pfc_log_trace("Freeing used Connections ... ");
  std::map<uint64_t, OdbcmConnectionHandler*>::iterator cpoolinuse_iter;
  if (!conpool_inuse_map_.empty()) {
    pfc_log_debug("used %"PFC_PFMT_SIZE_T
                  " RO Connection is present and about to free",
                  conpool_inuse_map_.size());
    cpoolinuse_iter = conpool_inuse_map_.begin();
    for ( ; cpoolinuse_iter != conpool_inuse_map_.end(); cpoolinuse_iter++) {
      OdbcmConnectionHandler *db_conn = (*cpoolinuse_iter).second;
                                         // the closeDBconnection
      delete db_conn;
      db_conn = NULL;
      pfc_log_debug("used RO Connection is freed");
    }
    conpool_inuse_map_.clear();
  }
  }
  physical_layer->db_conpool_mutex_.unlock();
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
    PHY_SQLEXEC_LOCK();
    ODBCM_ROLLBACK_TRANSACTION(conn_handle);
    return ODBCM_RC_SUCCESS;
  }
  /*  to disconnect */
  if (NULL != conn_handle) {
    SQLRETURN odbc_rc = 0;
    ODBCM_ROLLBACK_TRANSACTION(conn_handle);
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
  ODBCM_RC_STATUS ret_status = ODBCM_RC_SUCCESS;
  if (rw_nb_conn_obj_ != NULL) {
    SQLHDBC conn_handle = rw_nb_conn_obj_->get_conn_handle();
    /*  disconnect nb conn handle*/
    if (NULL != conn_handle) {
      SQLRETURN odbc_rc = 0;
      odbc_rc = SQLDisconnect(conn_handle);
      odbc_rc = SQLFreeHandle(SQL_HANDLE_DBC, conn_handle);
      conn_handle = NULL;
      rw_nb_conn_obj_->set_conn_handle(NULL);
      if (odbc_rc != ODBCM_RC_SUCCESS) {
        pfc_log_error("ODBCM::ODBCManager::CloseRwConnection: "
            "Error on Disconnect rw_nb_conn_handle");
        ret_status = (ODBCM_RC_STATUS) odbc_rc;
      }
    }
    delete rw_nb_conn_obj_;
    rw_nb_conn_obj_ = NULL;
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
      if (odbc_rc != ODBCM_RC_SUCCESS) {
        pfc_log_error("ODBCM::ODBCManager::CloseRwConnection: "
            "Error on Disconnect rw_sb_conn_handle");
        ret_status = (ODBCM_RC_STATUS) odbc_rc;
      }
    }
    delete rw_sb_conn_obj_;
    rw_sb_conn_obj_ = NULL;
  }
  if (ret_status == ODBCM_RC_SUCCESS)
    pfc_log_debug("ODBCM::ODBCManager::CloseRwConnection: "
      "Now, the rw connections are disconnected...");
  return ret_status;
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
      {CTR_PORT, std::string(CTR_PORT_STR)},
      {CTR_PORT_READ, std::string(CTR_PORT_STR_COALESCE)},
      /* above is for special handling of port column, 
       * if it is null return fixed value CTR_PORT_INVALID_VALUE
       * */
      {CTR_ACTUAL_VERSION, std::string(CTR_ACTUAL_VERSION_STR)},
      {CTR_ACTUAL_CONTROLLERID, std::string(CTR_ACTUAL_CONTROLLERID_STR)},
      {CTR_VALID_ACTUAL_CONTROLLERID,
         std::string(CTR_VALID_ACTUAL_CONTROLLERID_STR)},
      {CTR_OPER_STATUS, std::string(CTR_OPER_STATUS_STR)},
      {CTR_VALID, std::string(CTR_VALID_STR)},
      {CTR_CS_ROW_STATUS, std::string(CTR_CS_ROW_STATUS_STR)},
      {CTR_CS_ATTR, std::string(CTR_CS_ATTR_STR)},
      {CTR_COMMIT_NUMBER, std::string(CTR_COMMIT_NUMBER_STR)},
      {CTR_COMMIT_DATE, std::string(CTR_COMMIT_DATE_STR)},
      {CTR_COMMIT_APPLICATION, std::string(CTR_COMMIT_APPLICATION_STR)},
      {CTR_VALID_COMMIT_VERSION, std::string(CTR_VALID_COMMIT_VERSION_STR)},
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
      {PORT_CONNECTED_SWITCH_ID , std::string(PORT_CONNECTED_SWITCH_ID_STR)},
      {PORT_CONNECTED_PORT_ID , std::string(PORT_CONNECTED_PORT_ID_STR)},
      {PORT_CONNECTED_CONTROLLER_ID ,
        std::string(PORT_CONNECTED_CONTROLLER_ID_STR)},
      {PORT_CONNECTEDNEIGHBOR_VALID ,
        std::string(PORT_CONNECTEDNEIGHBOR_VALID_STR)},
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
