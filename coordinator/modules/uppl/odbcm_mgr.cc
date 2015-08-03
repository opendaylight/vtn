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
 * @file    odbcm_mgr.cc
 *
 */
#include "odbcm_mgr.hh"
#include "odbcm_db_tableschema.hh"
#include "odbcm_utils.hh"
#include "physicallayer.hh"
#include "odbcm_query_factory.hh"
#include "odbcm_query_processor.hh"
#include "odbcm_db_varbind.hh"
#include "odbcm_connection.hh"
using unc::uppl::ODBCManager;

extern pfc_cfdef_t odbcm_cfdef;

/**
 * @Description : This method initialize the ODBCManager
 * @param[in]   : None
 * @return      : ODBCM_RC_STATUS
 **/
ODBCM_RC_STATUS ODBCManager::ODBCM_Initialize() {
  PHY_FINI_READ_LOCK();
  /**Flag to ensure the earlier initialize process status, 
   * since it is public method
   * */
  if (IsODBCManager_initialized == 1) {
    pfc_log_debug("ODBCM::ODBCManager::Initialize:"
      "ODBCM is Already Initialized");
    return ODBCM_RC_SUCCESS;
  }
  /** Initialize the SQLStateMap */
  ODBCM_RC_STATUS rc = ODBCMUtils::Initialize_OdbcmSQLStateMap();
  //  rc = ODBCMUtils::Intialize_RCode_String();
  pfc_log_debug("ODBCM::ODBCManager::Initialize: "
    "OdbcmSQLStateMap initialized:%s", ODBCMUtils::get_RC_Details(rc).c_str());
  if (rc != ODBCM_RC_SUCCESS) {
    pfc_log_error("ODBCM::ODBCManager::Initialize: "
        "Error in OdbcmSQLStateMap initialization");
    return ODBCM_RC_GENERAL_ERROR;
  }
  /** Initialise the table list map */
  if (ODBCM_RC_SUCCESS != initialize_db_table_list_map_()) {
    pfc_log_error("ODBCM::ODBCManager::Initialize: "
        "Error in db_table_list_map_ initialization ");
    return ODBCM_RC_GENERAL_ERROR;
  }
  pfc_log_debug("ODBCM::ODBCManager::Initialize: "
      "db_table_list_map_ is initialized ");
  /** Initialise the table columns name map */
  if (ODBCM_RC_SUCCESS != initialize_odbcm_tables_column_map_()) {
    pfc_log_error("ODBCM::ODBCManager::Initialize: "
        "Error in odbcm_tables_column_map_ initialization ");
    return ODBCM_RC_GENERAL_ERROR;
  }
  pfc_log_debug("ODBCM::ODBCManager::Initialize: "
      "odbcm_tables_column_map_ is initialized ");

  /** Parse the odbcm.conf to get odbc config information */
  if (ODBCM_RC_SUCCESS != ParseConfigFile()) {
    pfc_log_error("ODBCM::ODBCManager::Initialize: "
        "Could not parse odbcm.conf !!");
    return ODBCM_RC_GENERAL_ERROR;
  }
  pfc_log_debug("ODBCM::ODBCManager::Initialize: "
      "db_conf_info is updated");
  /* Establish the database connection */
  if (ODBCM_RC_SUCCESS != InitializeConnectionEnv()) {
    UPPL_LOG_FATAL("ODBCM::ODBCManager::Initialize: "
        "Could not allocate connection environment !!");
    return ODBCM_RC_CONNECTION_ERROR;
  }
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  PhysicalCore* physical_core = physical_layer->get_physical_core();
  conn_max_limit_ = physical_core->uppl_max_ro_db_connections_;
  pfc_log_debug("conn_max_limit__ (from conf file) = %d",
                                                conn_max_limit_);
  pfc_log_info("ODBCM::ODBCManager::Initialize: "
      "ODBCM initialized !!!");
  /**set Flag for the status of init*/
  IsODBCManager_initialized = 1;
  return ODBCM_RC_SUCCESS;
}

/**
 * @Description : This method creates single key tree
 *                instance row in the given DB table. When a row 
 *                entry is created in the database table, 
 *                cs_row_status column will be set as CREATED.
 *                During commit phase the row will be created in 
 *                running configuration and the cs_row_status 
 *                will be set accordingly.
 * @param[in]   : db_name - specifies the configuration
 *                i.e.candidate/running/startup
 *                db_table_schema - object holds the key and value struct
 *                of specified KT instance
 * @return      : ODBCM_RC_SUCCESS is returned when the row is created
 *                ODBCM_RC_* is returned when the row is not created
 *
 **/
ODBCM_RC_STATUS ODBCManager::CreateOneRow(unc_keytype_datatype_t db_name,
                                          DBTableSchema &db_table_schema,
                                        OdbcmConnectionHandler *conn_obj) {
  PHY_FINI_READ_LOCK();
  /** Initialize all local variables */
  string            insert_query = ""; /* To store the db query */
  SQLRETURN         odbc_rc           = ODBCM_RC_SUCCESS; /* SQL API rc */
  ODBCM_RC_STATUS   status            = ODBCM_RC_SUCCESS; /* other method rc */
  ODBCMTable        table_id          = UNKNOWN_TABLE;
  std::list < std::vector <TableAttrSchema> >::iterator iter_list;
  /** Check the requested row already exists in database */
  status = IsRowExists(db_name, db_table_schema, conn_obj);
  pfc_log_debug("ODBCM::ODBCManager::CreateOneRow: IsRowExists "
    "returns: %s", ODBCMUtils::get_RC_Details(status).c_str());
  if (status != ODBCM_RC_ROW_EXISTS &&
      status != ODBCM_RC_ROW_NOT_EXISTS &&
      status != ODBCM_RC_SUCCESS) {
    pfc_log_debug("ODBCM::ODBCManager::CreateOneRow:IsRowExists status "
      "%s", (ODBCMUtils::get_RC_Details(status)).c_str());
    return status;
  }
  /** Allocate memory for statement, queryfactory, 
    * query processor and db_varbind objects */
  HSTMT             create_stmt       = NULL;
  SQLHDBC rw_conn_handle = conn_obj->get_conn_handle();
  ODBCM_STATEMENT_CREATE(rw_conn_handle, create_stmt, odbc_rc);
  QueryFactory      *query_factory    = NULL;
  ODBCM_CREATE_OBJECT(query_factory, QueryFactory);
  QueryProcessor    *query_processor  = NULL;
  ODBCM_CREATE_OBJECT(query_processor, QueryProcessor);
  DBVarbind         *db_varbind       = NULL;
  ODBCM_CREATE_OBJECT(db_varbind, DBVarbind);
  /** cs_row_status value would be stored in DBTableSchmea.db_return_status_
    * if row is exists in given table */
  if (status == ODBCM_RC_ROW_EXISTS) {
    /**since other than DT_CANDIDATE tables does not cs_row_status*/
    if (db_name != UNC_DT_CANDIDATE) {
      pfc_log_debug("ODBCM::ODBCManager::CreateOneRow: "
        "This operation does not supported in %s", g_log_db_name[db_name]);
      ODBCMFreeingMemory(create_stmt, table_id, db_varbind, query_factory,
                         query_processor);
      return status;
    }
    CsRowStatus rs_value = (CsRowStatus)db_table_schema.db_return_status_;
    switch (rs_value) {
      case APPLIED:
      case NOTAPPLIED:
      case CREATED:
      case UPDATED:
      case PARTIALLY_APPLIED:
      case ROW_VALID:
        pfc_log_debug("ODBCM::ODBCManager::CreateOneRow: "
                      " primary key violation,controller already exists"
                      ": ODBCM_RC_PKEY_VIOLATION");
        /** key violation so return with error code */
        status = ODBCM_RC_PKEY_VIOLATION;
        break;
      case DELETED:
        pfc_log_info("ODBCM::ODBCManager::CreateOneRow: "
                      "create row which is already exists but deleted in"
                      "previous transaction, clear will be called and "
                      "row creation will be allowed ");
        status = ClearOneRow(db_name, db_table_schema, conn_obj);
        if (status != ODBCM_RC_SUCCESS) {
          pfc_log_error("ODBCM::ODBCManager::CreateOneRow: "
                        "Error in Clearonerow so create not continued");
          status = ODBCM_RC_INVALID_DB_OPERATION;
          /** Clearonerow is failed - createonerow is not continued */
        }
        break;
      case ROW_INVALID:
        /* worst and corner case handling*/
        status = ClearOneRow(db_name, db_table_schema, conn_obj);
        if (status != ODBCM_RC_SUCCESS) {
          pfc_log_error("ODBCM::ODBCManager::CreateOneRow: "
                        "Error in Clearonerow so create not continued");
          status = ODBCM_RC_INVALID_DB_OPERATION;
        }
        /** Clearonerow is success - createonerow will be continued */
        break;
      default:
        break;
    }
    if (status != ODBCM_RC_SUCCESS) {
      ODBCMFreeingMemory(create_stmt, table_id, db_varbind, query_factory,
                         query_processor);
      return status;
    }
  }
  query_factory->SetOperation(CREATEONEROW);
  /** Get table id enum value */
  table_id = db_table_schema.get_table_name();
  if (table_id == UNKNOWN_TABLE) {
    pfc_log_error("ODBCM::ODBCManager::CreateOneRow: Unknown table: %d",
                 db_table_schema.get_table_name());
    status = ODBCM_RC_TABLE_NOT_FOUND;
    ODBCMFreeingMemory(create_stmt, table_id, db_varbind, query_factory,
                       query_processor);
    return status;
  }
  /** function pointer set for Binding parameter, binding values 
    * on the binded struct parameter, based upon table id 
    * corresponding bind and fill value method will be called out */
  db_varbind->SetBinding(table_id, BIND_IN);
  db_varbind->SetValueStruct(table_id, BIND_IN);

  /** Construct query in query factory and return here */
  insert_query = (query_factory->*query_factory->GetQuery)
                  (db_name, db_table_schema);
  if (insert_query.empty()) {
    pfc_log_error("ODBCM::ODBCManager::CreateOneRow: "
        "Error in framing query");
    status = ODBCM_RC_ERROR_IN_FRAMEQUERY;
    ODBCMFreeingMemory(create_stmt, table_id, db_varbind, query_factory,
                       query_processor);
    return status;
  }
  /** Prepare the query statement */
  status = query_processor->PrepareQueryStatement(
            insert_query, create_stmt);
  if (status != ODBCM_RC_SUCCESS) {
    pfc_log_error("ODBCM::ODBCManager::CreateOneRow:"
        "error in prepare statement   %s",
        ODBCMUtils::get_RC_Details(status).c_str());
    status = ODBCM_RC_STMT_ERROR;
    ODBCMFreeingMemory(create_stmt, table_id, db_varbind, query_factory,
                       query_processor);
    return status;
  }
  std::list < std::vector <TableAttrSchema> >& rlist =
      db_table_schema.get_row_list();
  /**to print the entire DBTableSchema object*/
  // db_table_schema.PrintDBTableSchema();
  /** Traverse the list to bind and execute */
  if (rlist.size() > 1) {
    pfc_log_debug("ODBCM::ODBCManager::CreateOneRow: "
       "> 1 rows are received for DB operation, But 1st row only will be pushed"
       "to Database");
  }
  iter_list = rlist.begin();
  if (iter_list != rlist.end()) {
    /** Filling the values into binded structure variables */
    status = (db_varbind->*db_varbind->FillINPUTValues)((*iter_list));
    if (status != ODBCM_RC_SUCCESS) {
      pfc_log_error("ODBCM::ODBCManager::CreateOneRow: "
          "Error in filling i/p: %s",
          ODBCMUtils::get_RC_Details(status).c_str());
      status = ODBCM_RC_PARAM_BIND_ERROR;
      ODBCMFreeingMemory(create_stmt, table_id, db_varbind, query_factory,
                         query_processor);
      return status;
    }
    /** Bind table input values - the corresponding table
      * structure variables will be binded here */
    status = (db_varbind->*db_varbind->BindINParameter)
              ((*iter_list), create_stmt);
    if (status != ODBCM_RC_SUCCESS) {
      pfc_log_error("ODBCM::ODBCManager::CreateOneRow: "
        "Error in binding i/p: %s",
        ODBCMUtils::get_RC_Details(status).c_str());
      status = ODBCM_RC_PARAM_BIND_ERROR;
      ODBCMFreeingMemory(create_stmt, table_id, db_varbind, query_factory,
                         query_processor);
      return status;
    }
    PHY_SQLEXEC_LOCK();
    status = query_processor->ExecuteEditDBQuery(
              CREATEONEROW, create_stmt);
    if (status == ODBCM_RC_SUCCESS) {
      /** Commit all active transactions on this connection */
      ODBCM_END_TRANSACTION(rw_conn_handle, SQL_COMMIT, status);
      pfc_log_debug("ODBCM::ODBCManager::CreateOneRow:row is created");
    } else {
      /** Rollback all active transactions on this connection */
      ODBCM_END_TRANSACTION(rw_conn_handle, SQL_ROLLBACK, status);
      pfc_log_debug("ODBCM::ODBCManager::CreateOneRow:row is not created");
    }
  } else {
    pfc_log_info("ODBCM::ODBCManager::CreateOneRow: No Data Received !");
  }
  ODBCMFreeingMemory(create_stmt, table_id, db_varbind, query_factory,
                     query_processor);
  return status;
}

/**
 * @Description : This method deletes the attributes of single 
 *                key tree instance in a row, the row entry will 
 *                not be actually deleted in the case of 
 *                UNC_DT_CANDIDATE. only the row_status column 
 *                will be changed to DELETED. During commit phase 
 *                the row will be cleared out. In other databases, 
 *                the entry will be deleted in this method and the 
 *                transaction will be committed.
 * @param[in]   : db_name - specifies the configuration
 *                i.e.candidate/running/startup
 *                db_table_schema - object holds the key and value struct
 *                of specified KT instance
 * @return      : ODBCM_RC_SUCCESS is returned when the row is deleted
 *                ODBCM_RC_* is returned when the row is not deleted
 **/
ODBCM_RC_STATUS ODBCManager::DeleteOneRow(unc_keytype_datatype_t db_name,
                                          DBTableSchema &db_table_schema,
                                          OdbcmConnectionHandler *conn_obj ) {
  PHY_FINI_READ_LOCK();
  string              delete_query = "";  // receiving the query
                                          //  from queryfactory
  SQLRETURN           odbc_rc   = ODBCM_RC_SUCCESS;  // SQL API rc
  ODBCM_RC_STATUS     status    = ODBCM_RC_SUCCESS;  // other method rc
  ODBCMTable          table_id  = UNKNOWN_TABLE;
  std::list < std::vector <TableAttrSchema> >::iterator iter_list;
  /* To check the row exists */
  status = IsRowExists(db_name, db_table_schema, conn_obj);
  if (status == ODBCM_RC_ROW_NOT_EXISTS ||
      (status != ODBCM_RC_ROW_EXISTS &&
      status != ODBCM_RC_ROW_NOT_EXISTS &&
      status != ODBCM_RC_SUCCESS)) {
    pfc_log_debug("ODBCM::ODBCManager::DeleteOneRow: "
                  "IsRowExists status %s",
                  (ODBCMUtils::get_RC_Details(status)).c_str());
    return status;
  }
  /** Allocate memory for statement, queryfactory, 
    * query processor and db_varbind objects */
  HSTMT               delete_stmt = NULL;
  SQLHDBC rw_conn_handle = conn_obj->get_conn_handle();
  ODBCM_STATEMENT_CREATE(rw_conn_handle, delete_stmt, odbc_rc);
  QueryFactory        *query_factory = NULL;
  ODBCM_CREATE_OBJECT(query_factory, QueryFactory);
  QueryProcessor      *query_processor = NULL;
  ODBCM_CREATE_OBJECT(query_processor, QueryProcessor);
  DBVarbind           *db_varbind = NULL;
  ODBCM_CREATE_OBJECT(db_varbind, DBVarbind);

  std::list < std::vector <TableAttrSchema> > & rlist =
      db_table_schema.get_row_list();
  pfc_log_debug("ODBCM::ODBCManager::DeleteOneRow: "
      "IsRowExists status %s",
      ODBCMUtils::get_RC_Details(status).c_str());
  /** Fetch the cs_row_status value to compare further */
  if (status == ODBCM_RC_ROW_EXISTS &&
      db_name == UNC_DT_CANDIDATE) {
    CsRowStatus rs_value = (CsRowStatus)db_table_schema.db_return_status_;
    if (rs_value == DELETED) {
      pfc_log_info("ODBCM::ODBCManager::DeleteOneRow: "
                    "Row exists but already in DELETED state ");
      ODBCM_RC_STATUS status = ClearOneRow(db_name, db_table_schema, conn_obj);
      if (status != ODBCM_RC_SUCCESS) {
        pfc_log_error("ODBCM::ODBCManager::DeleteOneRow: "
                      "Error in ClearOneRow so delete not continued");
        status = ODBCM_RC_INVALID_DB_OPERATION;
      }  // if
      /** Freeing all allocated memory*/
      ODBCMFreeingMemory(delete_stmt, table_id, db_varbind, query_factory,
                         query_processor);
      return status;
    } else {
      pfc_log_info("ODBCM::ODBCManager::DeleteOneRow: "
                   "rs_value from isrowexists function is %d", rs_value);
    }
  }  // if
  query_factory->SetOperation(DELETEONEROW);
  /** Validate the table */
  table_id = db_table_schema.get_table_name();
  if (table_id == UNKNOWN_TABLE) {
    pfc_log_error("ODBCM::ODBCManager::DeleteOneRow: Unknown table: %d",
      db_table_schema.get_table_name());
    status = ODBCM_RC_TABLE_NOT_FOUND;
    /** Freeing all allocated memory*/
    ODBCMFreeingMemory(delete_stmt, table_id, db_varbind, query_factory,
                           query_processor);
    return status;
  }
  /** Function pointer set for Binding parameter, binding values 
    * on the binded struct parameter, based upon table id 
    * corresponding bind and fill value method will be called out */
  db_varbind->SetBinding(table_id, BIND_IN);
  db_varbind->SetValueStruct(table_id, BIND_IN);

  /**  Construct the query string, queryfactory call with param */
  delete_query = (query_factory->*query_factory->GetQuery)
    (db_name, db_table_schema);
  if (delete_query.empty()) {
    pfc_log_error("ODBCM::ODBCManager::DeleteOneRow: "
      "Error in framing query");
    status = ODBCM_RC_ERROR_IN_FRAMEQUERY;
    /** Freeing all allocated memory*/
    ODBCMFreeingMemory(delete_stmt, table_id, db_varbind, query_factory,
                           query_processor);
    return status;
  }
  /** Prepare the query statement using constructed query string */
  status = query_processor->PrepareQueryStatement(
            delete_query, delete_stmt);
  if (status != ODBCM_RC_SUCCESS) {
    pfc_log_error("ODBCM::ODBCManager::DeleteOneRow: "
                 "Error in preparing query");
    status = ODBCM_RC_STMT_ERROR;
    /** Freeing all allocated memory*/
    ODBCMFreeingMemory(delete_stmt, table_id, db_varbind, query_factory,
                           query_processor);
    return status;
  }
  pfc_log_debug("ODBCM::ODBCManager::DeleteOneRow: "
      "row_list size : %" PFC_PFMT_SIZE_T, rlist.size());
  // db_table_schema.PrintDBTableSchema();
  if (rlist.size() > 1) {
    pfc_log_debug("ODBCM::ODBCManager::DeleteOneRow: "
       "> 1 rows are received for DB operation,But 1st row only will be pushed"
       "to Database");
  }
  iter_list = rlist.begin();
  if (iter_list != rlist.end()) {
    /** Filling the values into binded structure variables */
    status = (db_varbind->*db_varbind->FillINPUTValues)((*iter_list));
    if (status != ODBCM_RC_SUCCESS) {
      pfc_log_error("ODBCM::ODBCManager::DeleteOneRow: "
        "Error in filling i/p: %s",
        ODBCMUtils::get_RC_Details(status).c_str());
      status = ODBCM_RC_PARAM_BIND_ERROR;
      /* Freeing all allocated memory */
      ODBCMFreeingMemory(delete_stmt, table_id, db_varbind, query_factory,
                             query_processor);
      return status;
    }
    /** To bind table input values - the corresponding table
     * structure variables will be binded here*/
    status = (db_varbind->*db_varbind->BindINParameter)
              ((*iter_list), delete_stmt);
    if (status != ODBCM_RC_SUCCESS) {
      pfc_log_error("ODBCM::ODBCManager::DeleteOneRow: "
        "Error in binding i/p: %s",
        ODBCMUtils::get_RC_Details(status).c_str());
      status = ODBCM_RC_PARAM_BIND_ERROR;
      /* Freeing all allocated memory */
      ODBCMFreeingMemory(delete_stmt, table_id, db_varbind, query_factory,
                             query_processor);
      return status;
    }
    PHY_SQLEXEC_LOCK();
    /** Execute the prepared statement to done the delete one row */
    status = query_processor->ExecuteEditDBQuery(
              DELETEONEROW, delete_stmt);
    if (status != ODBCM_RC_SUCCESS) {
      pfc_log_debug("ODBCM::ODBCManager::DeleteOneRow: "
          "ExecuteEditDBQuery status %s",
          ODBCMUtils::get_RC_Details(status).c_str());
      /* Freeing all allocated memory */
      ODBCMFreeingMemory(delete_stmt, table_id, db_varbind, query_factory,
                             query_processor);
      return status;
    }
    if (status == ODBCM_RC_SUCCESS) {
      /** Commit all active transactions on this connection*/
      ODBCM_END_TRANSACTION(rw_conn_handle, SQL_COMMIT, status);
      pfc_log_debug("ODBCM::ODBCManager::DeleteOneRow:row is deleted");
    } else {
      /** Rollback all active transactions on this connection*/
      ODBCM_END_TRANSACTION(rw_conn_handle, SQL_ROLLBACK, status);
      pfc_log_debug("ODBCM::ODBCManager::DeleteOneRow:row is not deleted");
    }
  } else {
    pfc_log_info("ODBCM::ODBCManager::DeleteOneRow: No Data Received !");
  }
  /* Freeing all allocated memory */
  ODBCMFreeingMemory(delete_stmt, table_id, db_varbind, query_factory,
                         query_processor);
  return status;
}

/**
 * @Description : This method updates one row of single keytree 
 *                instance in the database table. 
 *                The cs_row_status column will be set as updated. 
 *                During commit phase the update will be moved to 
 *                running configuration and the row_status will be 
 *                set accordingly.
 * @param[in]   : db_name - specifies the configuration
 *                i.e.candidate/running/startup
 *                db_table_schema - object holds the key and value struct
 *                of specified KT instance
 * @return      : ODBCM_RC_SUCCESS is returned when the row is updated
 *                ODBCM_RC_* is returned when the row is not updated
 **/
ODBCM_RC_STATUS ODBCManager::UpdateOneRow(unc_keytype_datatype_t db_name,
                                          DBTableSchema &db_table_schema,
                                          OdbcmConnectionHandler *conn_obj,
                                          bool IsInternal) {
  PHY_FINI_READ_LOCK();
  /** Initialize all local variables */
  string            update_query = "";  // to store the query
  SQLRETURN         odbc_rc  = ODBCM_RC_SUCCESS;  // SQL API rc
  ODBCM_RC_STATUS   status   = ODBCM_RC_SUCCESS;  // other methods rc
  ODBCMTable        table_id = UNKNOWN_TABLE;
  std::list < std::vector <TableAttrSchema> >::iterator iter;
  /** To check the row exists */
  status = IsRowExists(db_name, db_table_schema, conn_obj);
  if (status == ODBCM_RC_ROW_NOT_EXISTS ||
      (status != ODBCM_RC_ROW_EXISTS &&
      status != ODBCM_RC_ROW_NOT_EXISTS &&
      status != ODBCM_RC_SUCCESS)) {
    pfc_log_debug("ODBCM::ODBCManager::UpdateOneRow: "
      "IsRowExists status = : %s",
      (ODBCMUtils::get_RC_Details(status)).c_str());
    return status;
  }
  /** Allocation for sql stmt */
  HSTMT           update_stmt = NULL;
  SQLHDBC rw_conn_handle = conn_obj->get_conn_handle();
  ODBCM_STATEMENT_CREATE(rw_conn_handle, update_stmt, odbc_rc);
  /** Create query_factory, query processor, dbvarbind obj */
  QueryFactory    *query_factory = NULL;
  ODBCM_CREATE_OBJECT(query_factory, QueryFactory);
  QueryProcessor  *query_processor = NULL;
  ODBCM_CREATE_OBJECT(query_processor, QueryProcessor);
  DBVarbind       *db_varbind = NULL;
  ODBCM_CREATE_OBJECT(db_varbind, DBVarbind);

  std::list < std::vector <TableAttrSchema> >& rlist =
      db_table_schema.get_row_list();
  pfc_log_debug("ODBCM::ODBCManager::UpdateOneRow: "
      "IsRowExists returns: %s",
      ODBCMUtils::get_RC_Details(status).c_str());
  if (status == ODBCM_RC_ROW_EXISTS && db_name == UNC_DT_CANDIDATE) {
    CsRowStatus rs_value = (CsRowStatus)db_table_schema.db_return_status_;
    if (rs_value == DELETED && IsInternal == false) {
      pfc_log_info("ODBCM::ODBCManager::UpdateOneRow: Row exists with"
        "DELETED state so ClearOneRow will be called");
      ODBCM_RC_STATUS status = ClearOneRow(db_name, db_table_schema, conn_obj);
      if (status != ODBCM_RC_SUCCESS) {
        pfc_log_info("ODBCM::ODBCManager::UpdateOneRow: "
           "ClearOneRow is not succeeded, update not continued");
        status = ODBCM_RC_INVALID_DB_OPERATION;
        /** Clearonerow is failed - createonerow is not continued */
      } else {
        status = ODBCM_RC_ROW_NOT_EXISTS;
      }
      /* Freeing all allocated memory */
      ODBCMFreeingMemory(update_stmt, table_id, db_varbind, query_factory,
                             query_processor);
      return status;
    }
  }

  query_factory->SetOperation(UPDATEONEROW);
  /** Prepare the sql command */
  update_query = (query_factory->*query_factory->GetQueryWithBool)(
                db_name, db_table_schema, IsInternal);
  if (update_query.empty()) {
    pfc_log_error("ODBCM::ODBCManager::UpdateOneRow: "
        "Error in framing the SQL query");
    status = ODBCM_RC_ERROR_IN_FRAMEQUERY;
    /* Freeing all allocated memory */
    ODBCMFreeingMemory(update_stmt, table_id, db_varbind, query_factory,
                           query_processor);
    return status;
  }
  /** Get table id enum value*/
  table_id = db_table_schema.get_table_name();
  if (table_id == UNKNOWN_TABLE) {
    pfc_log_error("ODBCM::ODBCManager::UpdateOneRow: "
        "Unknown table:%d",
        db_table_schema.get_table_name());
    status = ODBCM_RC_TABLE_NOT_FOUND;
    /* Freeing all allocated memory */
    ODBCMFreeingMemory(update_stmt, table_id, db_varbind, query_factory,
                           query_processor);
    return status;
  }
  /** Function pointer set for Binding parameter, binding 
    * values on the binded struct parameter, based upon 
    * table id corresponding bind and fill value 
    * method will be called out */
  db_varbind->SetBinding(table_id, BIND_IN);
  db_varbind->SetValueStruct(table_id, BIND_IN);
  /** Prepare the sql statment */
  status = query_processor->PrepareQueryStatement(
            update_query, update_stmt);
  if (status != ODBCM_RC_SUCCESS) {
    pfc_log_error("ODBCM::ODBCManager::UpdateOneRow: "
        "Error in preparing query statement");
    status = ODBCM_RC_STMT_ERROR;
    /* Freeing all allocated memory */
    ODBCMFreeingMemory(update_stmt, table_id, db_varbind, query_factory,
                           query_processor);
    return status;
  }
  if (rlist.size() > 1) {
    pfc_log_debug("ODBCM::ODBCManager::UpdateOneRow: "
       "> 1 rows are received for DB operation,But 1st row only will be pushed"
       "to Database");
  }
  // db_table_schema.PrintDBTableSchema();
  iter = rlist.begin();
  if (iter != rlist.end()) {
    /** Removing the primarykey values from vector */
    std::vector <string> primarykeys = db_table_schema.get_primary_keys();
    std::vector< TableAttrSchema >::iterator iter_vector;
    uint32_t pkey_size =  primarykeys.size();
    pfc_log_debug("ODBCM::ODBCManager::UpdateOneRow: "
      "primary keys size %d, rowlist vector size %" PFC_PFMT_SIZE_T,
      pkey_size, (*iter).size());
    iter_vector = (*iter).begin();

    for (uint32_t loop = 0; loop < (*iter).size(); loop++) {
      pfc_log_debug("ODBCM::ODBCManager::UpdateOneRow: %d",
        (*iter_vector).table_attribute_name);
      iter_vector++;
    }
    if ((*iter).size() < pkey_size) {
      pfc_log_info("ODBCM::ODBCManager::UpdateOneRow: primary keys"
                   "may not be in attributes_vector");
      status = ODBCM_RC_ERROR_IN_FRAMEQUERY;
      /* Freeing all allocated memory */
      ODBCMFreeingMemory(update_stmt, table_id, db_varbind, query_factory,
                             query_processor);
      return status;
    } else {
      pfc_log_debug("ODBCM::ODBCManager::UpdateOneRow: "
        "Vector size with pkeys %" PFC_PFMT_SIZE_T, (*iter).size());
      if ((*iter).size() >= pkey_size) {
        uint32_t index = 0;
        for (iter_vector = (*iter).begin();
            (iter_vector != (*iter).end()) && (index < pkey_size);
            ++index) {
          if ((*iter_vector).p_table_attribute_value != NULL) {
            ::operator delete((*iter_vector).p_table_attribute_value);
            (*iter_vector).p_table_attribute_value = NULL;
          }
          iter_vector = (*iter).erase(iter_vector);
        }
        pfc_log_debug("ODBCM::ODBCManager::UpdateOneRow: primary keys"
            " value are removed from attributes_vector to skip from update."
            " vector size = %" PFC_PFMT_SIZE_T, (*iter).size());
      }
    }
  } else {
        pfc_log_debug("ODBCM::ODBCManager::UpdateOneRow: No Data Received !");
  }
  iter = rlist.begin();
  if (iter != rlist.end()) {
    pfc_log_debug("ODBCM::ODBCManager::UpdateOneRow: "
      "rowlist vector size: %" PFC_PFMT_SIZE_T, (*iter).size());
    /** To filling the values into binded structure variables */
    status = (db_varbind->*db_varbind->FillINPUTValues)((*iter));
    if (status != ODBCM_RC_SUCCESS) {
      pfc_log_error("ODBCM::ODBCManager::UpdateOneRow: "
        "Error in filling i/p: %s",
         ODBCMUtils::get_RC_Details(status).c_str());
      status = ODBCM_RC_PARAM_BIND_ERROR;
      /* Freeing all allocated memory */
      ODBCMFreeingMemory(update_stmt, table_id, db_varbind, query_factory,
                             query_processor);
      return status;
    }
    /** To bind table input values - the corresponding table
     * structure variables will be binded here*/
    status = (db_varbind->*db_varbind->BindINParameter)
        ((*iter), update_stmt);
    if (status != ODBCM_RC_SUCCESS) {
      pfc_log_error("ODBCM::ODBCManager::UpdateOneRow: status: %s",
        ODBCMUtils::get_RC_Details(status).c_str());
      status = ODBCM_RC_PARAM_BIND_ERROR;
      /* Freeing all allocated memory */
      ODBCMFreeingMemory(update_stmt, table_id, db_varbind, query_factory,
                             query_processor);
      return status;
    }
    PHY_SQLEXEC_LOCK();
    status = query_processor->ExecuteEditDBQuery(
              UPDATEONEROW, update_stmt);
    if (status == ODBCM_RC_SUCCESS) {
      /** Commit all active transactions on this connection*/
      ODBCM_END_TRANSACTION(rw_conn_handle, SQL_COMMIT, status);
    } else {
      /** Rollback all active transactions on this connection*/
      ODBCM_END_TRANSACTION(rw_conn_handle, SQL_ROLLBACK, status);
      pfc_log_info("ODBCM::ODBCManager::UpdateOneRow:row is not updated");
    }
  } else {
        pfc_log_debug("ODBCM::ODBCManager::UpdateOneRow: No Data Received !");
  }
  /* Freeing all allocated memory */
  ODBCMFreeingMemory(update_stmt, table_id, db_varbind, query_factory,
                         query_processor);
  return status;
}

/**
 * @Description : This method fetches single row from the
 *                database table and return the result set
 *                (fill the table rows in the DBTableSchema ref) 
 *                and return the status to caller
 * @param[in]   : db_name - specifies the configuration
 *                i.e.candidate/running/startup
 *                db_table_schema - object holds the key and value struct
 *                of specified KT instance
 * @param[out]  : DBTableSchema db_table_schema
 * @return      : ODBCM_RC_SUCCESS is returned when the GetOneRow is
 *                success otherwise DB related error code will be
 *                returned
 **/
ODBCM_RC_STATUS ODBCManager::GetOneRow(
    unc_keytype_datatype_t db_name,
    DBTableSchema &db_table_schema, OdbcmConnectionHandler *conn_obj) {
  PHY_FINI_READ_LOCK();
  /** Initialise the local variables */
  string            getone_query = "";  // to receive the
                                    // query from queryfactory
  SQLRETURN         odbc_rc  = ODBCM_RC_SUCCESS;  //  SQL API rc
  ODBCM_RC_STATUS   status   = ODBCM_RC_SUCCESS;  //  Other method rc
  ODBCMTable        table_id = UNKNOWN_TABLE;
  std::list < std::vector <TableAttrSchema> >::iterator iter_list;

  HSTMT read_stmt = NULL;  /* statement for getonerow */
  SQLHDBC ro_conn_handle = conn_obj->get_conn_handle();
  ODBCM_STATEMENT_CREATE(ro_conn_handle, read_stmt, odbc_rc);

  /** DBTableSchema row_list - get from parameter */
  std::list < std::vector <TableAttrSchema> >& rlist =
      db_table_schema.get_row_list();
  /** iterate all rows in vector */

  if (db_name == UNC_DT_STARTUP) {
    // check startup validity , if not valid, read from running
    PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
    PhysicalCore* physical_core = physical_layer->get_physical_core();
    if (!physical_core->getStartupValidStatus()) {
      db_name = UNC_DT_RUNNING;
      pfc_log_debug("ODBCM::ODBCManager::GetOneRow: modify to RUNNING");
    }
  }
  /** Create query_factory and query processor objects */
  QueryFactory    *query_factory    = NULL;
  ODBCM_CREATE_OBJECT(query_factory, QueryFactory);
  QueryProcessor  *query_processor  = NULL;
  ODBCM_CREATE_OBJECT(query_processor, QueryProcessor);
  DBVarbind       *db_varbind       = NULL;
  ODBCM_CREATE_OBJECT(db_varbind, DBVarbind);

  /** Set the operation for GETONEROW */
  query_factory->SetOperation(GETONEROW);
  /** Invoke query factory method to generate the query */
  getone_query = (query_factory->*query_factory->GetDBSpecificQuery)
                (db_name, db_table_schema);
  /** Check the query is empty or not */
  if (getone_query.empty()) {
    pfc_log_error("ODBCM::ODBCManager::GetOneRow: "
      "Error in framing query");
    status = ODBCM_RC_ERROR_IN_FRAMEQUERY;
    /* Freeing all allocated memory */
    ODBCMFreeingMemory(read_stmt, table_id, db_varbind, query_factory,
                           query_processor);
    return status;
  }
  /* prepare the sql statement using constructed sql string */
  status = query_processor->PrepareQueryStatement(
            getone_query, read_stmt);
  if (status == ODBCM_RC_CONNECTION_ERROR) {
    err_connx_list_.push_back(conn_obj->get_using_session_id());
  }

  if (status !=  ODBCM_RC_SUCCESS) {
    pfc_log_error("ODBCM::ODBCManager::GetOneRow: "
        "Error in preparing query statement: %s"
        , ODBCMUtils::get_RC_Details(status).c_str());
    status = ODBCM_RC_QUERY_FAILED;
    /* Freeing all allocated memory */
    ODBCMFreeingMemory(read_stmt, table_id, db_varbind, query_factory,
                           query_processor);
    return status;
  }
  /** Get table id */
  table_id = db_table_schema.get_table_name();
  if (table_id == UNKNOWN_TABLE) {
    pfc_log_error("ODBCM::ODBCManager::GetOneRow: "
      "Unknown table: %d",
      db_table_schema.get_table_name());
    status = ODBCM_RC_TABLE_NOT_FOUND;
    /* Freeing all allocated memory */
    ODBCMFreeingMemory(read_stmt, table_id, db_varbind, query_factory,
                           query_processor);
    return status;
  }

  /** function pointer set for Binding parameter,
    * binding values on the binded struct parameter, 
    * based upon table id corresponding bind and 
    * fill value method will be called out */
  db_varbind->SetBinding(table_id, BIND_IN);
  db_varbind->SetValueStruct(table_id, BIND_IN);
  db_varbind->SetBinding(table_id, BIND_OUT);
  db_varbind->SetValueStruct(table_id, BIND_OUT);
  iter_list = rlist.begin();
  if (iter_list != rlist.end()) {
    /* To filling the values into binded structure variables */
    status = (db_varbind->*db_varbind->FillINPUTValues)((*iter_list));
    if (status != ODBCM_RC_SUCCESS) {
      pfc_log_error("ODBCM::ODBCManager::GetOneRow: "
        "Error in filling i/p: %s",
        ODBCMUtils::get_RC_Details(status).c_str());
      status = ODBCM_RC_PARAM_BIND_ERROR;
      /* Freeing all allocated memory */
      ODBCMFreeingMemory(read_stmt, table_id, db_varbind, query_factory,
                             query_processor);
      return status;
    }

    /** To bind table input values - the corresponding table
     * structure variables will be binded here */
    status = (db_varbind->*db_varbind->BindINParameter)
        ((*iter_list), read_stmt);
    if (status != ODBCM_RC_SUCCESS) {
      pfc_log_error("ODBCM::ODBCManager::GetOneRow: "
        "Error in i/p binding: %s",
        ODBCMUtils::get_RC_Details(status).c_str());
      status = ODBCM_RC_PARAM_BIND_ERROR;
      /* Freeing all allocated memory */
      ODBCMFreeingMemory(read_stmt, table_id, db_varbind, query_factory,
                             query_processor);
      return status;
    }
    status = (db_varbind->*db_varbind->BindOUTParameter)
        ((*iter_list), read_stmt);
    if (status != ODBCM_RC_SUCCESS) {
      pfc_log_error("ODBCM::ODBCManager::GetOneRow: "
        "Error in binding o/p: %s",
        ODBCMUtils::get_RC_Details(status).c_str());
      status = ODBCM_RC_PARAM_BIND_ERROR;
      /* Freeing all allocated memory */
      ODBCMFreeingMemory(read_stmt, table_id, db_varbind, query_factory,
                             query_processor);
      return status;
    }
    PHY_SQLEXEC_LOCK();
    /** Execute the ReadDBQuery with the above statement */
    status = query_processor->ExecuteReadDBQuery(GETONEROW, read_stmt);
    ODBCM_ROLLBACK_TRANSACTION(ro_conn_handle);
    if (status == ODBCM_RC_CONNECTION_ERROR) {
      err_connx_list_.push_back(conn_obj->get_using_session_id());
    }
    if (status != ODBCM_RC_SUCCESS &&
        status != ODBCM_RC_SUCCESS_WITH_INFO) {
      pfc_log_debug("ODBCM::ODBCManager::GetOneRow: "
          "ExecuteReadDBQuery status %s",
          ODBCMUtils::get_RC_Details(status).c_str());
      /* Freeing all allocated memory */
      ODBCMFreeingMemory(read_stmt, table_id, db_varbind, query_factory,
                             query_processor);
      return status;
    }
    /** Fill the database output in dbtableschema */
    if (status == ODBCM_RC_SUCCESS) {
      /** While framing any get request, ITC allocates
       * memory for the attributes in the list.This memory
       * should be freed from db schema before calling 
       * fetch functions where, memory for all attributes 
       * allocated again */
      std::vector<TableAttrSchema>::iterator tmp_iter;
      for (tmp_iter = (*iter_list).begin();
          tmp_iter != (*iter_list).end();
          tmp_iter++) {
        if ((*tmp_iter).p_table_attribute_value)
          ::operator delete((*tmp_iter).p_table_attribute_value);
      }
      odbc_rc = SQLFetch(read_stmt);
      status = (db_varbind->*db_varbind->FetchOUTPUTValues)((*iter_list));
      if (status != ODBCM_RC_SUCCESS) {
        pfc_log_error("ODBCM::ODBCManager::GetOneRow: fetch status: %s",
          ODBCMUtils::get_RC_Details(status).c_str());
        status = ODBCM_RC_ERROR_FETCHING_ROW;
        /* Freeing all allocated memory */
        ODBCMFreeingMemory(read_stmt, table_id, db_varbind, query_factory,
                               query_processor);
        return status;
      }
    }  // if
  } else {
    pfc_log_error("ODBCM::ODBCManager::GetOneRow:No input data Received!");
  }
  db_table_schema.set_row_list(rlist);
  // db_table_schema.PrintDBTableSchema();
  /* Freeing all allocated memory */
  ODBCMFreeingMemory(read_stmt, table_id, db_varbind, query_factory,
                         query_processor);
  return status;
}

/**
 * @Description : This method clears single row in the database 
 *                table, based on the condition given in the 
 *                DBTableSchema the row will be identified 
 *                and deleted permanently.
 * @param[in]   : db_name - specifies the configuration
 *                i.e.candidate/running/startup
 *                db_table_schema - object holds the key and value struct
 *                of specified KT instance
 * @return      : ODBCM_RC_SUCCESS is returned when the ClearOneRow is
 *                success otherwise DB related error code will be
 *                returned
 **/
ODBCM_RC_STATUS ODBCManager::ClearOneRow(unc_keytype_datatype_t db_name,
                                         DBTableSchema &db_table_schema,
                                         OdbcmConnectionHandler *conn_obj) {
  PHY_FINI_READ_LOCK();
  string          clearone_query = "";              // to store the query
  SQLRETURN       odbc_rc = ODBCM_RC_SUCCESS;  // SQL API rc
  ODBCM_RC_STATUS status = ODBCM_RC_SUCCESS;   // other methods rc
  ODBCMTable      table_id = UNKNOWN_TABLE;
  std::list < std::vector <TableAttrSchema> >::iterator iter_list;
  /** statement for clearonerow */
  HSTMT clearone_stmt = NULL;
  SQLHDBC rw_conn_handle = conn_obj->get_conn_handle();
  ODBCM_STATEMENT_CREATE(rw_conn_handle, clearone_stmt, odbc_rc);
  /** Create query_factory and query processor objects */
  QueryFactory    *query_factory    = NULL;
  ODBCM_CREATE_OBJECT(query_factory, QueryFactory);
  QueryProcessor  *query_processor  = NULL;
  ODBCM_CREATE_OBJECT(query_processor, QueryProcessor);
  DBVarbind       *db_varbind       = NULL;
  ODBCM_CREATE_OBJECT(db_varbind, DBVarbind);

  /** DBTableSchema row_list - get from parameter */
  std::list < std::vector <TableAttrSchema> >& rlist =
      db_table_schema.get_row_list();
  /** To iterate entire vector */
  /** Set function ptr to construct the query in queryfactory */
  query_factory->SetOperation(CLEARONEROW);
  clearone_query = (query_factory->*query_factory->GetQuery)
                    (db_name, db_table_schema);
  if (clearone_query.empty()) {
    pfc_log_error("ODBCM::ODBCManager::ClearOneRow: "
        "Error in framing query");
    status = ODBCM_RC_ERROR_IN_FRAMEQUERY;
    /* Freeing all allocated memory */
    ODBCMFreeingMemory(clearone_stmt, table_id, db_varbind, query_factory,
                           query_processor);
    return status;
  }
  /** Get table id enum value */
  table_id = db_table_schema.get_table_name();
  if (table_id == UNKNOWN_TABLE) {
    pfc_log_error("ODBCM::ODBCManager::ClearOneRow:: "
      "Unknown table: %d", db_table_schema.get_table_name());
    status = ODBCM_RC_TABLE_NOT_FOUND;
    /* Freeing all allocated memory */
    ODBCMFreeingMemory(clearone_stmt, table_id, db_varbind, query_factory,
                           query_processor);
    return status;
  }
  /** function pointer set for Binding parameter, 
    * binding values on the binded struct parameter, 
    * based upon table id corresponding bind and fill value 
    * method will be called out */
  db_varbind->SetBinding(table_id, BIND_IN);
  db_varbind->SetValueStruct(table_id, BIND_IN);
  /** Call binding variable for input param - binding_table_input()*/
  /** prepare the query statement */
  status = query_processor->PrepareQueryStatement(clearone_query,
                                                  clearone_stmt);
  if (status != ODBCM_RC_SUCCESS) {
    pfc_log_error("ODBCM::ODBCManager::ClearOneRow"
        "Error in preparing query statement");
    status = ODBCM_RC_STMT_ERROR;
    /* Freeing all allocated memory */
    ODBCMFreeingMemory(clearone_stmt, table_id, db_varbind, query_factory,
                           query_processor);
    return status;
  }

  // db_table_schema.PrintDBTableSchema();
  iter_list = rlist.begin();
  if (iter_list != rlist.end()) {
    /** to filling the values into binded structure variables */
    status = (db_varbind->*db_varbind->FillINPUTValues)((*iter_list));
    if (status != ODBCM_RC_SUCCESS) {
      pfc_log_error("ODBCM::ODBCManager::ClearOneRow:: "
        "Error in filling i/p: %s ",
        ODBCMUtils::get_RC_Details(status).c_str());
      status = ODBCM_RC_PARAM_BIND_ERROR;
      /* Freeing all allocated memory */
      ODBCMFreeingMemory(clearone_stmt, table_id, db_varbind, query_factory,
                             query_processor);
      return status;
    }
    /** To bind table input values - the corresponding table
     * structure variables will be binded here*/
    status = (db_varbind->*db_varbind->BindINParameter)(
              (*iter_list), clearone_stmt);
    if (status != ODBCM_RC_SUCCESS) {
      pfc_log_error("ODBCM::ODBCManager::ClearOneRow: "
        "Error in binding i/p: %s",
        ODBCMUtils::get_RC_Details(status).c_str());
      status = ODBCM_RC_PARAM_BIND_ERROR;
      /* Freeing all allocated memory */
      ODBCMFreeingMemory(clearone_stmt, table_id, db_varbind, query_factory,
                             query_processor);
      return status;
    }
    PHY_SQLEXEC_LOCK();
    status = query_processor->ExecuteEditDBQuery(
            CLEARONEROW, clearone_stmt);
    if (status == ODBCM_RC_SUCCESS) {
      /** Commit all active transactions on this connection*/
      ODBCM_END_TRANSACTION(rw_conn_handle, SQL_COMMIT, status);
      pfc_log_debug("ODBCM::ODBCManager::ClearOneRow:row is cleared");
    } else {
      /** Rollback all active transactions on this connection*/
      ODBCM_END_TRANSACTION(rw_conn_handle, SQL_ROLLBACK, status);
      pfc_log_info("ODBCM::ODBCManager::ClearOneRow:row is not cleared");
    }
  } else {
      pfc_log_info("ODBCM::ODBCManager::ClearOneRow:No input data received !");
  }
  /* Freeing all allocated memory */
  ODBCMFreeingMemory(clearone_stmt, table_id, db_varbind, query_factory,
                         query_processor);
  return status;
}
/**
 * @Description : To find the row value presence in the database 
 *                table. Database name is the first parameter, 
 *                other key, primary key details are must be 
 *                provided in DBTableSchema
 * @param[in]   : db_name - specifies the configuration
 *                i.e.candidate/running/startup
 *                db_table_schema - object holds the key and value struct
 *                of specified KT instance
 * @return      : ODBCM_RC_SUCCESS is returned when the row is exits in DB
 *                success otherwise DB related error code will be
 *                returned
 **/
ODBCM_RC_STATUS ODBCManager::IsRowExists(
    unc_keytype_datatype_t db_name,
    DBTableSchema &db_table_schema, OdbcmConnectionHandler *conn_obj) {
  PHY_FINI_READ_LOCK();
  /** Initialise the local variables */
  string            query = "";                   // to store the query
  ODBCM_RC_STATUS   status = ODBCM_RC_SUCCESS;   // other methods rc
  SQLRETURN         odbc_rc = ODBCM_RC_SUCCESS;  // SQL APIs rc
  ODBCMTable        table_id = UNKNOWN_TABLE;
  std::list < std::vector <TableAttrSchema> >::iterator iter_list;
  iter_list = db_table_schema.row_list_.begin();
  if (iter_list == db_table_schema.row_list_.end()) {
    pfc_log_info("ODBCM::ODBCManager::IsRowExists:No input data received !");
    return ODBCM_RC_FAILED;
  }
  /** Statement for isrowexists */
  HSTMT           rowexists_stmt   = NULL;
  SQLHDBC ro_conn_handle = conn_obj->get_conn_handle();
  ODBCM_STATEMENT_CREATE(ro_conn_handle, rowexists_stmt, odbc_rc);
  /** Create query_factory and query processor objects */
  QueryFactory    *query_factory    = NULL;
  ODBCM_CREATE_OBJECT(query_factory, QueryFactory);
  QueryProcessor  *query_processor  = NULL;
  ODBCM_CREATE_OBJECT(query_processor, QueryProcessor);
  DBVarbind       *db_varbind       = NULL;
  ODBCM_CREATE_OBJECT(db_varbind, DBVarbind);

  if (db_name == UNC_DT_STARTUP) {
    // check startup validity , if not valid, read from running
    PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
    PhysicalCore* physical_core = physical_layer->get_physical_core();
    if (!physical_core->getStartupValidStatus()) {
      db_name = UNC_DT_RUNNING;
      pfc_log_debug("ODBCM::ODBCManager::IsRowExist:modify db name to RUNNING");
    }
  }

  /** func. ptr for ISROWEXISTS to construct query in queyfactory */
  query_factory->SetOperation(ISROWEXISTS);
  query = (query_factory->*query_factory->GetQuery)
            (db_name, db_table_schema);
  if (query.empty()) {
    pfc_log_error("ODBCM::ODBCManager::IsRowExists: "
        "Error in framing query");
    status = ODBCM_RC_ERROR_IN_FRAMEQUERY;
    /* Freeing all allocated memory */
    ODBCMFreeingMemory(rowexists_stmt, table_id, db_varbind, query_factory,
                           query_processor);
    return status;
  }
  /** Prepare the query statement */
  status = query_processor->PrepareQueryStatement(
            query, rowexists_stmt);
  if (status == ODBCM_RC_CONNECTION_ERROR) {
    err_connx_list_.push_back(conn_obj->get_using_session_id());
  }

  if (status != ODBCM_RC_SUCCESS) {
    pfc_log_error("ODBCM::ODBCManager::IsRowExists: "
        "Error in preparing query statement");
    status = ODBCM_RC_STMT_ERROR;
    /* Freeing all allocated memory */
    ODBCMFreeingMemory(rowexists_stmt, table_id, db_varbind, query_factory,
                           query_processor);
    return status;
  }
  /** Get table id enum value */
  table_id = db_table_schema.get_table_name();
  if (table_id == UNKNOWN_TABLE) {
    pfc_log_error("ODBCM::ODBCManager::IsRowExists: "
      "Unknown table: %d", db_table_schema.get_table_name());
    status = ODBCM_RC_TABLE_NOT_FOUND;
    /* Freeing all allocated memory */
    ODBCMFreeingMemory(rowexists_stmt, table_id, db_varbind, query_factory,
                           query_processor);
    return status;
  }
  /** function pointer set for Binding parameter, binding 
    * values on the binded struct parameter, based upon 
    * table id corresponding bind and fill value 
    * method will be called out */
  db_varbind->SetBinding(table_id, BIND_IN);
  db_varbind->SetValueStruct(table_id, BIND_IN);
  db_varbind->SetBinding(IS_ROW_EXISTS, BIND_OUT);
  db_varbind->SetValueStruct(IS_ROW_EXISTS, BIND_OUT);

  // db_table_schema.PrintDBTableSchema();
  /** To fill the values into binded structure variables */
  status = (db_varbind->*db_varbind->FillINPUTValues)((*iter_list));
  if (status != ODBCM_RC_SUCCESS) {
    pfc_log_error("ODBCM::ODBCManager::IsRowExists: "
                  "Error in filling i/p: %s",
                  ODBCMUtils::get_RC_Details(status).c_str());
    status = ODBCM_RC_PARAM_BIND_ERROR;
    /* Freeing all allocated memory */
    if (NULL != db_varbind->p_isrowexists) {
      delete db_varbind->p_isrowexists;
      pfc_log_debug("ODBCM::ODBCManager::IsRowExists: p_isrowexists is freed");
    }
    ODBCMFreeingMemory(rowexists_stmt, table_id, db_varbind, query_factory,
                       query_processor);
    return status;
  }

  /** Bind table input values - the corresponding table
   * structure variables will be binded here*/
  status = (db_varbind->*db_varbind->BindINParameter)
                      ((*iter_list), rowexists_stmt);
  if (status != ODBCM_RC_SUCCESS) {
    pfc_log_error("ODBCM::ODBCManager::IsRowExists: "
                  "Error in binding i/p: %s",
                  ODBCMUtils::get_RC_Details(status).c_str());
    status = ODBCM_RC_PARAM_BIND_ERROR;
    /* Freeing all allocated memory */
    if (NULL != db_varbind->p_isrowexists) {
      delete db_varbind->p_isrowexists;
      pfc_log_debug("ODBCM::ODBCManager::IsRowExists: p_isrowexists is freed");
    }
    ODBCMFreeingMemory(rowexists_stmt, table_id, db_varbind, query_factory,
                       query_processor);
    return status;
  }

  status = (db_varbind->*db_varbind->BindOUTParameter)
                      ((*iter_list), rowexists_stmt);
  if (status != ODBCM_RC_SUCCESS) {
    pfc_log_error("ODBCM::ODBCManager::IsRowExists: "
                  "Error in binding o/p: %s",
                  ODBCMUtils::get_RC_Details(status).c_str());
    status = ODBCM_RC_PARAM_BIND_ERROR;
    /* Freeing all allocated memory */
    // Output bind structure free
    if (NULL != db_varbind->p_isrowexists) {
      delete db_varbind->p_isrowexists;
      pfc_log_debug("ODBCM::ODBCManager::IsRowExists: p_isrowexists is freed");
    }
    ODBCMFreeingMemory(rowexists_stmt, table_id, db_varbind, query_factory,
                       query_processor);
    return status;
  }
  PHY_SQLEXEC_LOCK();
  /** Execute the prepared statement using query
   * string from queryfactory */
  status  = query_processor->ExecuteGroupOperationQuery(ISROWEXISTS,
                                                        rowexists_stmt);
  ODBCM_ROLLBACK_TRANSACTION(ro_conn_handle);
  if (status == ODBCM_RC_CONNECTION_ERROR) {
    err_connx_list_.push_back(conn_obj->get_using_session_id());
  }
  if ((status == ODBCM_RC_ROW_EXISTS || status == ODBCM_RC_SUCCESS)
      && db_varbind->p_isrowexists->is_exists == EXISTS &&
      db_varbind->p_isrowexists->cs_row_status != UNKNOWN) {
    pfc_log_debug("ODBCM::ODBCManager::IsRowExists: "
                  "Execute success. Start fetching. "
                  "is_exists:%d, cs_row_status:%d",
                  db_varbind->p_isrowexists->is_exists,
                  db_varbind->p_isrowexists->cs_row_status);
    switch (db_varbind->p_isrowexists->cs_row_status) {
      case 0: db_table_schema.db_return_status_ = CREATED;
      break;
      case 1: db_table_schema.db_return_status_ = UPDATED;
      break;
      case 2: db_table_schema.db_return_status_ = DELETED;
      break;
      case 3: db_table_schema.db_return_status_ = ROW_VALID;
      break;
      case 4: db_table_schema.db_return_status_ = ROW_INVALID;
      break;
      case 5: db_table_schema.db_return_status_ = APPLIED;
      break;
      case 6: db_table_schema.db_return_status_ = NOTAPPLIED;
      break;
      default:
        break;
    }
    pfc_log_debug("ODBCM::ODBCManager::IsRowExists: "
        "db_table_schema.db_return_status_ = %d ",
        db_table_schema.db_return_status_);
  }
  /* Freeing all allocated memory */
  // Output bind structure free
  if (NULL != db_varbind->p_isrowexists) {
    delete db_varbind->p_isrowexists;
    pfc_log_debug("ODBCM::ODBCManager::IsRowExists: p_isrowexists is freed");
  }
  ODBCMFreeingMemory(rowexists_stmt, table_id, db_varbind, query_factory,
                         query_processor);
  return status;
}

/**
 * @Description : This method will fetch the one or more number of 
 *                rows in the db table based upon the given 
 *                max_repetition_count & condition or filter criteria.
 * @param[in]   : db_name - specifies the configuration
 *                i.e.candidate/running/startup
 *                db_table_schema - object holds the key and value struct
 *                of specified KT instance
 *                max_repetition_count -specifies number of rows to be returned
 *                unc_keytype_operation_t - specifies any additional
 *                condition for GetBulkRows operation
 * @param[out]  : DBTableSchema             &db_table_schema,
 * @return      : ODBCM_RC_SUCCESS is returned when the rows are read from DB
 *                otherwise DB related error code will be returned
 **/
ODBCM_RC_STATUS ODBCManager::GetBulkRows(
    unc_keytype_datatype_t db_name,
    uint32_t max_repetition_count,
    DBTableSchema &db_table_schema,
    unc_keytype_operation_t op_type, OdbcmConnectionHandler *conn_obj) {
  PHY_FINI_READ_LOCK();
  /** Initialize the local variables */
  string                getbulk_query = "";               // to store query
  SQLRETURN             odbc_rc = ODBCM_RC_SUCCESS;  // SQL API rc
  ODBCM_RC_STATUS       status = ODBCM_RC_SUCCESS;   // other methods rc
  SQLLEN                row_count = 0;
  ODBCMTable            table_id = UNKNOWN_TABLE;
  std::list < std::vector <TableAttrSchema> >::iterator it_vect;

  std::list < std::vector <TableAttrSchema> > & rlist =
      db_table_schema.get_row_list();
  it_vect = rlist.begin();
  if (it_vect == rlist.end()) {
    pfc_log_info("ODBCM::ODBCManager::GetBulkRows:No input data received !");
    return ODBCM_RC_FAILED;
  }
  /** Allocate stmt handle only if the connection 
    * was successfully created */
  HSTMT           read_stmt         = NULL;
  SQLHDBC ro_conn_handle = conn_obj->get_conn_handle();
  ODBCM_STATEMENT_CREATE(ro_conn_handle, read_stmt, odbc_rc);
  /** Create query_factory and query processor objects */
  QueryFactory    *query_factory    = NULL;
  ODBCM_CREATE_OBJECT(query_factory, QueryFactory);
  QueryProcessor  *query_processor  = NULL;
  ODBCM_CREATE_OBJECT(query_processor, QueryProcessor);
  DBVarbind       *db_varbind       = NULL;
  ODBCM_CREATE_OBJECT(db_varbind, DBVarbind);

  /** Set the operation for GETBULKROWS */
  query_factory->SetOperation(GETBULKROWS);
  pfc_log_debug("ODBCM::ODBCManager: GetBulkRows: "
    "Request with max_repetition_count: %d, op_type: %d",
    max_repetition_count, op_type);
  if (db_name == UNC_DT_STARTUP) {
    // check startup validity , if not valid, read from running
    PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
    PhysicalCore* physical_core = physical_layer->get_physical_core();
    if (!physical_core->getStartupValidStatus()) {
      db_name = UNC_DT_RUNNING;
      pfc_log_debug("ODBCM::ODBCManager::GetBulkRow:modify db name to RUNNING");
    }
  }

  /** Invoke query factory method to generate the query */
  getbulk_query = (query_factory->*query_factory->GetBulkRowQuery)
        (db_name, max_repetition_count, db_table_schema, op_type);
  /** Check the query is empty or not */
  if (getbulk_query.empty()) {
    pfc_log_error("ODBCM::ODBCManager::GetBulkRows: "
      "Error in framing query");
    status = ODBCM_RC_ERROR_IN_FRAMEQUERY;
    /* Freeing all allocated memory */
    ODBCMFreeingMemory(read_stmt, table_id, db_varbind, query_factory,
                           query_processor);
    return status;
  }
  /* prepare sql statment with constructed query string  */
  status = query_processor->PrepareQueryStatement(
            getbulk_query, read_stmt);
  if (status == ODBCM_RC_CONNECTION_ERROR) {
    err_connx_list_.push_back(conn_obj->get_using_session_id());
  }

  if (status !=  ODBCM_RC_SUCCESS) {
    pfc_log_error("ODBCM::ODBCManager::GetBulkRows: "
        "Error in preparing query statement: %s",
        ODBCMUtils::get_RC_Details(status).c_str());
    status = ODBCM_RC_QUERY_FAILED;
    /* Freeing all allocated memory */
    ODBCMFreeingMemory(read_stmt, table_id, db_varbind, query_factory,
                           query_processor);
    return status;
  }
  /** Get table id */
  table_id = db_table_schema.get_table_name();
  if (table_id == UNKNOWN_TABLE) {
    pfc_log_error("ODBCM::ODBCManager::GetBulkRows: "
      "Unknown table: %d", db_table_schema.get_table_name());
    status = ODBCM_RC_TABLE_NOT_FOUND;
    /* Freeing all allocated memory */
    ODBCMFreeingMemory(read_stmt, table_id, db_varbind, query_factory,
                           query_processor);
    return status;
  }

  /** function pointer set for Binding parameter, binding 
    * values on the binded struct parameter, based upon 
    * table id corresponding bind and fill value 
    * method will be called out */
  db_varbind->SetBinding(table_id, BIND_IN);
  db_varbind->SetValueStruct(table_id, BIND_IN);
  db_varbind->SetBinding(table_id, BIND_OUT);
  db_varbind->SetValueStruct(table_id, BIND_OUT);
  /* To filling the values into binded structure variables */
  status = (db_varbind->*db_varbind->FillINPUTValues)((*it_vect));
  if (status != ODBCM_RC_SUCCESS) {
    pfc_log_error("ODBCM::ODBCManager::GetBulkRows: "
                  "Error in filling i/p: %s",
                  ODBCMUtils::get_RC_Details(status).c_str());
    status = ODBCM_RC_PARAM_BIND_ERROR;
    /* Freeing all allocated memory */
    ODBCMFreeingMemory(read_stmt, table_id, db_varbind, query_factory,
                       query_processor);
    return status;
  }
  /** To bind table input values - the corresponding table
   * structure variables will be binded here */
  status = (db_varbind->*db_varbind->BindINParameter)
                  ((*it_vect), read_stmt);
  if (status != ODBCM_RC_SUCCESS) {
    pfc_log_error("ODBCM::ODBCManager::GetBulkRows: "
                  "Error in binding i/p: %s",
                  ODBCMUtils::get_RC_Details(status).c_str());
    status = ODBCM_RC_PARAM_BIND_ERROR;
    /* Freeing all allocated memory */
    ODBCMFreeingMemory(read_stmt, table_id, db_varbind, query_factory,
                       query_processor);
    return status;
  }
  status = (db_varbind->*db_varbind->BindOUTParameter)
                  ((*it_vect), read_stmt);
  if (status != ODBCM_RC_SUCCESS) {
    pfc_log_error("ODBCM::ODBCManager::GetBulkRows: "
                  "Error in binding o/p: %s",
                  ODBCMUtils::get_RC_Details(status).c_str());
    status = ODBCM_RC_PARAM_BIND_ERROR;
    /* Freeing all allocated memory */
    ODBCMFreeingMemory(read_stmt, table_id, db_varbind, query_factory,
                       query_processor);
    return status;
  }
  PHY_SQLEXEC_LOCK();
  /** Execute the ReadDBQuery with the above statement */
  status = query_processor->ExecuteReadDBQuery(GETBULKROWS, read_stmt);
  ODBCM_ROLLBACK_TRANSACTION(ro_conn_handle);
  if (status == ODBCM_RC_CONNECTION_ERROR) {
    err_connx_list_.push_back(conn_obj->get_using_session_id());
  }
  /** Fill the database output in dbtableschema */
  if (status != ODBCM_RC_SUCCESS) {
    pfc_log_debug("ODBCM::ODBCManager::GetBulkRows: "
                  "ExecuteReadDBQuery status %s",
                  ODBCMUtils::get_RC_Details(status).c_str());
    /* Freeing all allocated memory */
    ODBCMFreeingMemory(read_stmt, table_id, db_varbind, query_factory,
                       query_processor);
    return status;
  }
  /**  To fetch all returned rows */
  /**  Fetch the no. of row return in last executed query */
  odbc_rc = SQLRowCount(read_stmt, &row_count);
  ODBCM_PROCESS_HANDLE_CHECK(read_stmt, odbc_rc);

  if (odbc_rc == ODBCM_RC_CONNECTION_ERROR) {
    err_connx_list_.push_back(conn_obj->get_using_session_id());
  }
  if (odbc_rc != ODBCM_RC_SUCCESS &&
      odbc_rc != ODBCM_RC_SUCCESS_WITH_INFO) {
    pfc_log_debug("ODBCM::ODBCManager::GetBulkRows: "
                  "Error in SQLRowCount: %s",
                  ODBCMUtils::get_RC_Details(odbc_rc).c_str());
    status = (ODBCM_RC_STATUS)odbc_rc;
    /* Freeing all allocated memory */
    ODBCMFreeingMemory(read_stmt, table_id, db_varbind, query_factory,
                       query_processor);
    return status;
  }
  pfc_log_debug("ODBCM::ODBCManager::GetBulkRows: "
      "Row count = %" UNC_PFMT_SQLLEN, row_count);
  if (row_count < 0) {
    pfc_log_debug("ODBCM::ODBCManager::GetBulkRows: "
                  "No more record found !!!");
    status = ODBCM_RC_RECORD_NO_MORE;
    /* Freeing all allocated memory */
    ODBCMFreeingMemory(read_stmt, table_id, db_varbind, query_factory,
                       query_processor);
    return status;
  }
  odbc_rc = SQLFetch(read_stmt);
  if (odbc_rc != ODBCM_RC_SUCCESS &&
      odbc_rc != ODBCM_RC_SUCCESS_WITH_INFO) {
    pfc_log_error("ODBCM::ODBCManager::GetBulkRows: "
                  "Error in SQLFetch: %s",
                  ODBCMUtils::get_RC_Details(odbc_rc).c_str());
    status = ODBCM_RC_ERROR_FETCHING_ROW;
    /* Freeing all allocated memory */
    ODBCMFreeingMemory(read_stmt, table_id, db_varbind, query_factory,
                       query_processor);
    return status;
  }

  /** While framing any get request, ITC allocates
   * memory for the attributes in the list.This memory
   * should be freed from db schema before calling
   * fetch functions where, memory for all attributes
   * allocated again */
  std::vector<TableAttrSchema>::iterator tmp_iter;
  for (tmp_iter = (*it_vect).begin();
      tmp_iter != (*it_vect).end();
      tmp_iter++) {
    if ((*tmp_iter).p_table_attribute_value)
      ::operator delete((*tmp_iter).p_table_attribute_value);
  }

  status = (db_varbind->*db_varbind->FetchOUTPUTValues)
                  ((*it_vect));
  if (row_count > 1) {
    std::vector<TableAttrSchema> new_col_attr((*it_vect));
    db_varbind->SetValueStruct(table_id, BIND_OUT);
    odbc_rc = SQLFetch(read_stmt);
    while (odbc_rc != SQL_NO_DATA) {
      if (odbc_rc != ODBCM_RC_SUCCESS &&
          odbc_rc != ODBCM_RC_SUCCESS_WITH_INFO) {
        pfc_log_error("ODBCM::ODBCManager::GetBulkRows: "
                      "Error in SQLFetch: %s",
                      ODBCMUtils::get_RC_Details(odbc_rc).c_str());
        status = ODBCM_RC_ERROR_FETCHING_ROW;
        /* Freeing all allocated memory */
        ODBCMFreeingMemory(read_stmt, table_id, db_varbind, query_factory,
                           query_processor);
        return status;
      }
      /** Fetch the recieved data */
      status = (db_varbind->*db_varbind->FetchOUTPUTValues)
                      (new_col_attr);
      if (status != ODBCM_RC_SUCCESS) {
        pfc_log_error("ODBCM::ODBCManager::GetBulkRows: "
                      "Error in Fetching o/p: %s",
                      ODBCMUtils::get_RC_Details(status).c_str());
        status = ODBCM_RC_ERROR_FETCHING_ROW;
        /* Freeing all allocated memory */
        ODBCMFreeingMemory(read_stmt, table_id, db_varbind, query_factory,
                           query_processor);
        return status;
      }
      db_varbind->SetValueStruct(table_id, BIND_OUT);
      odbc_rc = SQLFetch(read_stmt);
      /** Push the attribute vector to row_list.
       * Not required for 1st time */
      rlist.push_back(new_col_attr);
    }
  }
  db_table_schema.set_row_list(rlist);
  pfc_log_debug("ODBCM::ODBCManager::GetBulkRows:dbtableschema list size: %"
               PFC_PFMT_SIZE_T, db_table_schema.row_list_.size());
  status = ODBCM_RC_SUCCESS;
  // db_table_schema.PrintDBTableSchema();
  /* Freeing all allocated memory */
  ODBCMFreeingMemory(read_stmt, table_id, db_varbind, query_factory,
                         query_processor);
  return status;
}

/**
 * @Description : To get the sibling rows count
 *                Example: If we have Ctr1, Ctr2, Ctr3, Ctr5
 *                GetSiblingCount(Ctr2) will return number of 
 *                rows after Ctr2 i.e 2 if the given row does 
 *                not exists, the next row will be identified 
 *                and process accordingly.
 * @param[in]   : db_name - specifies the configuration
 *                i.e.candidate/running/startup
 *                db_table_schema - object holds the key and value struct
 *                of specified KT instance
 * @param[in]   : unc_keytype_datatype_t , DBTableSchema
 * @param[out]  : uint32_t count
 * @return      : ODBCM_RC_SUCCESS is returned when the siblingcount 
 *                received from DB
 *                otherwise DB related error code will be
 *                returned
 **/
ODBCM_RC_STATUS ODBCManager::GetSiblingCount(
    unc_keytype_datatype_t db_name,
    DBTableSchema& db_table_schema,
    uint32_t& count, OdbcmConnectionHandler *conn_obj) {
  PHY_FINI_READ_LOCK();
  // Initialise the local variables
  std::string QUERY("\0");  // to receive the query from queryfactory
  SQLRETURN odbc_rc = ODBCM_RC_SUCCESS;  //  SQL API rc
  ODBCM_RC_STATUS status = ODBCM_RC_SUCCESS;
  SQLLEN row_count = 0;
  count = 0;  // to store row count
  std::list <std::vector<TableAttrSchema> >::iterator i_list;
  ODBCMTable table_id = UNKNOWN_TABLE;

  HSTMT stmt = NULL;  // statement for getsiblingcount
  SQLHDBC ro_conn_handle = conn_obj->get_conn_handle();
  /* Do sql allocate for sql stmt */
  ODBCM_STATEMENT_CREATE(ro_conn_handle, stmt, odbc_rc);

  /** Create query_factory and query processor objects */
  QueryFactory *query_factory = NULL;
  ODBCM_CREATE_OBJECT(query_factory, QueryFactory);

  QueryProcessor *query_processor = NULL;
  ODBCM_CREATE_OBJECT(query_processor, QueryProcessor);

  DBVarbind *db_varbind = NULL;
  ODBCM_CREATE_OBJECT(db_varbind, DBVarbind);

  query_factory->SetOperation(GETSIBLINGCOUNT);
  if (db_name == UNC_DT_STARTUP) {
    // check startup validity , if not valid, read from running
    PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
    PhysicalCore* physical_core = physical_layer->get_physical_core();
    if (!physical_core->getStartupValidStatus()) {
      db_name = UNC_DT_RUNNING;
      pfc_log_debug("ODBCM::ODBCManager::GetSibCnt: modify db name to RUNNING");
    }
  }
  /** Construct query in query factory and return here */
  QUERY = (query_factory->*query_factory->GetQuery)(db_name, db_table_schema);
  if (QUERY.empty()) {
    pfc_log_error("ODBCM::ODBCManager::GetSiblingCount: "
        "Error in framing query");
    status = ODBCM_RC_ERROR_IN_FRAMEQUERY;
    /* Freeing all allocated memory */
    ODBCMFreeingMemory(stmt, table_id, db_varbind, query_factory,
                           query_processor);
    return status;
  }
  /** Validate the table information */
  table_id = db_table_schema.get_table_name();
  if (table_id == UNKNOWN_TABLE) {
    pfc_log_error("ODBCM::ODBCManager::GetSiblingCount: "
        "Error Unknown table: %d", db_table_schema.get_table_name());
    status = ODBCM_RC_TABLE_NOT_FOUND;
    /* Freeing all allocated memory */
    ODBCMFreeingMemory(stmt, table_id, db_varbind, query_factory,
                           query_processor);
    return status;
  }
  /* Prepare the query statement */
  status = query_processor->PrepareQueryStatement(QUERY, stmt);
  if (status == ODBCM_RC_CONNECTION_ERROR) {
    err_connx_list_.push_back(conn_obj->get_using_session_id());
  }

  if (status != ODBCM_RC_SUCCESS) {
    pfc_log_error("ODBCM::ODBCManager::GetSiblingCount: Error in "
        "preparing query statement: %s",
        ODBCMUtils::get_RC_Details(status).c_str());
    status = ODBCM_RC_STMT_ERROR;
    /* Freeing all allocated memory */
    ODBCMFreeingMemory(stmt, table_id, db_varbind, query_factory,
                           query_processor);
    return status;
  }
  /* Set the binding */
  db_varbind->SetBinding(table_id, BIND_IN);
  db_varbind->SetValueStruct(table_id, BIND_IN);

  /* No need to traverse the list */
  /* Binding the input is required */
  i_list = db_table_schema.row_list_.begin();
  if (i_list != db_table_schema.row_list_.end()) {
    /* Filling the values into binded structure variables */
    status = (db_varbind->*db_varbind->FillINPUTValues)((*i_list));
    if (status != ODBCM_RC_SUCCESS) {
      pfc_log_error("ODBCM::ODBCManager::GetSiblingCount: "
        "Error in filling i/p: %s",
        ODBCMUtils::get_RC_Details(status).c_str());
      status = ODBCM_RC_PARAM_BIND_ERROR;
      /* Freeing all allocated memory */
      ODBCMFreeingMemory(stmt, table_id, db_varbind, query_factory,
                             query_processor);
      return status;
    }
    status = (db_varbind->*db_varbind->BindINParameter)
              ((*i_list), stmt);
    if (status != ODBCM_RC_SUCCESS) {
      pfc_log_error("ODBCM::ODBCManager::GetSiblingCount: "
        "Error in binding i/p: %s",
        ODBCMUtils::get_RC_Details(status).c_str());
      status = ODBCM_RC_PARAM_BIND_ERROR;
      /* Freeing all allocated memory */
      ODBCMFreeingMemory(stmt, table_id, db_varbind, query_factory,
                             query_processor);
      return status;
    }

    PHY_SQLEXEC_LOCK();
    status = query_processor->ExecuteReadDBQuery(
              GETSIBLINGCOUNT, stmt);
    ODBCM_ROLLBACK_TRANSACTION(ro_conn_handle);
    if (status != ODBCM_RC_SUCCESS) {
      pfc_log_debug("ODBCM::ODBCManager::GetSiblingCount: "
        "ExecuteReadDBQuery: status %s",
        ODBCMUtils::get_RC_Details(status).c_str());
      /* Freeing all allocated memory */
      ODBCMFreeingMemory(stmt, table_id, db_varbind, query_factory,
                             query_processor);
      if (status == ODBCM_RC_RECORD_NOT_FOUND)
        status = ODBCM_RC_SUCCESS;
      return status;
    }

    odbc_rc = SQLRowCount(stmt, reinterpret_cast<SQLLEN*> (&row_count));
    if (odbc_rc != ODBCM_RC_SUCCESS &&
        odbc_rc != ODBCM_RC_SUCCESS_WITH_INFO) {
      pfc_log_debug("ODBCM::ODBCManager::GetSiblingCount: "
          "SQLRowCount: status %s",
          ODBCMUtils::get_RC_Details(odbc_rc).c_str());
      status = ODBCM_RC_STMT_ERROR;
      /* Freeing all allocated memory */
      ODBCMFreeingMemory(stmt, table_id, db_varbind, query_factory,
                             query_processor);
      return status;
    }
    /* Update the sibling count */
    if (row_count == 0)
      status = ODBCM_RC_SUCCESS;
    count = row_count;
    pfc_log_info("ODBCM::ODBCManager::GetSiblingCount:: "
      "row_count: %d", (uint32_t)row_count);
  } else {
    pfc_log_info("ODBCM::ODBCManager::GetSiblingCount::"
                 "No input data received !");
  }
  /* Freeing all allocated memory */
  ODBCMFreeingMemory(stmt, table_id, db_varbind, query_factory,
                         query_processor);
  return status;
}

/**
 * @Description : This method will get the row count of the given table
 * @param[in]   : db_name - specifies the configuration
 *                i.e.candidate/running/startup
 *                table_name - specifies the table name in DB
 * @param[out]  : count - specifies the row count 
 * @return      : ODBCM_RC_SUCCESS is returned when the row count
 *                received from DB otherwise DB related error
 *                code will be returned
 **/
ODBCM_RC_STATUS ODBCManager::GetRowCount(
    unc_keytype_datatype_t db_name,
    string table_name,
    uint32_t &count, OdbcmConnectionHandler *conn_obj) {
  PHY_FINI_READ_LOCK();
  /** Initialize the local variables */
  SQLLEN          row_count = 0;               // to store rowcount
  SQLRETURN       odbc_rc = ODBCM_RC_SUCCESS;  // SQL API rc
  std::string     query = "";                  // query from queryfactory
  ODBCM_RC_STATUS status = ODBCM_RC_SUCCESS;   //  other method rc
  HSTMT           stmt = NULL;                 // statement for getrowcount
  QueryFactory    *query_factory = NULL;
  QueryProcessor *query_processor = NULL;

  /** Initialise the count value */
  count = 0;
  SQLHDBC ro_conn_handle = conn_obj->get_conn_handle();
  /** Do sql allocate for sql stmt */
  ODBCM_STATEMENT_CREATE(ro_conn_handle, stmt, odbc_rc);
  /** Create query_factory and query processor objects */
  ODBCM_CREATE_OBJECT(query_factory, QueryFactory);
  ODBCM_CREATE_OBJECT(query_processor, QueryProcessor);

  query_factory->SetOperation(GETROWCOUNT);
  if (db_name == UNC_DT_STARTUP) {
    // check startup validity , if not valid, read from running
    PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
    PhysicalCore* physical_core = physical_layer->get_physical_core();
    if (!physical_core->getStartupValidStatus()) {
      db_name = UNC_DT_RUNNING;
      pfc_log_debug("ODBCM::ODBCManager::GetRowCount:"
                   " modify db name to RUNNING");
    }
  }
  /** Construct query in query factory and return here */
  query = (query_factory->*query_factory->GetCountQuery)
                    (db_name, table_name);
  if (query.empty()) {
    pfc_log_error("ODBCM::ODBCManager::GetRowCount: "
        "Error in framing query");
    status = ODBCM_RC_ERROR_IN_FRAMEQUERY;
    /* Freeing all allocated memory */
    ODBCMFreeingMemory(stmt, UNKNOWN_TABLE, NULL, query_factory,
                           query_processor);
    return status;
  }
  PHY_SQLEXEC_LOCK();
  /** Execute the query */
  status = query_processor->ExecuteQueryDirect(
            GETROWCOUNT, query, stmt);
  ODBCM_ROLLBACK_TRANSACTION(ro_conn_handle);
  if (status != ODBCM_RC_SUCCESS) {
    pfc_log_debug("ODBCM::ODBCManager::GetRowCount: "
      "ExecuteQueryDirect: status %s",
      ODBCMUtils::get_RC_Details(status).c_str());
    status = ODBCM_RC_TRANSACTION_ERROR;
    /* Freeing all allocated memory */
    ODBCMFreeingMemory(stmt, UNKNOWN_TABLE, NULL, query_factory,
                           query_processor);
    return status;
  }
  /** Get the number of rows */
  odbc_rc = SQLRowCount(stmt, reinterpret_cast<SQLLEN*> (&row_count));
  if (odbc_rc != ODBCM_RC_SUCCESS &&
      odbc_rc != ODBCM_RC_SUCCESS_WITH_INFO) {
    pfc_log_debug("ODBCM::ODBCManager::GetRowCount: "
        "SQLRowCount: status %s",
        ODBCMUtils::get_RC_Details(odbc_rc).c_str());
    status = ODBCM_RC_STMT_ERROR;
    /* Freeing all allocated memory */
    ODBCMFreeingMemory(stmt, UNKNOWN_TABLE, NULL, query_factory,
                           query_processor);
    return status;
  }
  /** Update the row count value */
  if (row_count == 0)
    status = ODBCM_RC_SUCCESS;
  count = row_count;
  pfc_log_info("ODBCM::ODBCManager::GetRowCount: Success. "
      "row_count:%d", count);
  /* Freeing all allocated memory */
  ODBCMFreeingMemory(stmt, UNKNOWN_TABLE, NULL, query_factory,
                         query_processor);
  return status;
}

/**
 * @Description : To get the modified rows
 * Example: If we have below table info
 *    ctr_name | row_status
 *    ---------+---------
 *    ctr1     | CREATED
 *    ctr2     | APPLIED
 *    -------------------
 * We get a request: GetModifiedRows(DBTableSchema(CREATED));
 * The database output will be filled in DBTableSchema
 *
 * @param[in]   : db_name - specifies the configuration
 *                i.e.candidate/running/startup
 *                db_table_schema - object holds the key and value struct
 *                of specified KT instance
 * @param[out]  : DBTableSchema db_table_schema
 * @return      : ODBCM_RC_SUCCESS is returned when the modified rows
 *                received from DB otherwise DB related error
 *                code will be returned
 **/
ODBCM_RC_STATUS ODBCManager::GetModifiedRows(
    unc_keytype_datatype_t db_name,
    DBTableSchema& db_table_schema, OdbcmConnectionHandler *conn_obj) {
  PHY_FINI_READ_LOCK();
  std::string QUERY("\0");
  SQLRETURN odbc_rc = ODBCM_RC_SUCCESS;
  ODBCM_RC_STATUS status = ODBCM_RC_SUCCESS;
  ODBCMTable table_id = UNKNOWN_TABLE;
  SQLLEN iRow_count = 0;
  std::list <std::vector<TableAttrSchema> >::iterator i_list;
  std::vector<TableAttrSchema> :: iterator iter_vector;
  if (db_name != UNC_DT_CANDIDATE) {
    pfc_log_debug("ODBCM::ODBCManager::GetModifiedRows: "
                 "This operation does not supported for %d ",
                 db_table_schema.table_name_);
    return ODBCM_RC_TABLE_NOT_FOUND;
  }
  std::list < std::vector <TableAttrSchema> >& rlist =
      db_table_schema.get_row_list();
  i_list = rlist.begin();
  if (i_list == rlist.end()) {
    pfc_log_info("ODBCM::ODBCManager::GetModifiedRows:"
                 "No input data received !");
    return ODBCM_RC_FAILED;
  }

  HSTMT get_stmt = NULL;
  SQLHDBC ro_conn_handle = conn_obj->get_conn_handle();
  /* Do sql allocate for sql stmt */
  ODBCM_STATEMENT_CREATE(ro_conn_handle, get_stmt, odbc_rc);

  QueryFactory *query_factory = NULL;
  ODBCM_CREATE_OBJECT(query_factory, QueryFactory);

  QueryProcessor *query_processor = NULL;
  ODBCM_CREATE_OBJECT(query_processor, QueryProcessor);

  DBVarbind *db_varbind = NULL;
  ODBCM_CREATE_OBJECT(db_varbind, DBVarbind);

  query_factory->SetOperation(GETMODIFIEDROWS);
  /* construct query in query factory and return here */
  QUERY = (query_factory->*query_factory->GetQuery)(db_name, db_table_schema);
  if (QUERY.empty()) {
    pfc_log_error("ODBCM::ODBCManager::GetModifiedRows: "
        "Error in framing query");
    status = ODBCM_RC_ERROR_IN_FRAMEQUERY;
    /* Freeing all allocated memory */
    ODBCMFreeingMemory(get_stmt, table_id, db_varbind, query_factory,
                           query_processor);
    return status;
  }

  /* Validate the table information */
  table_id = db_table_schema.get_table_name();
  if (table_id == UNKNOWN_TABLE) {
    pfc_log_error("ODBCM::ODBCManager::GetModifiedRows: "
      "Error Unknown table: %d",
      db_table_schema.get_table_name());
    status = ODBCM_RC_TABLE_NOT_FOUND;
    /* Freeing all allocated memory */
    ODBCMFreeingMemory(get_stmt, table_id, db_varbind, query_factory,
                           query_processor);
    return status;
  }

  db_varbind->SetBinding(table_id, BIND_OUT);
  db_varbind->SetValueStruct(table_id, BIND_OUT);

  /* Prepare the query statement */
  status = query_processor->PrepareQueryStatement(QUERY, get_stmt);
  if (status == ODBCM_RC_CONNECTION_ERROR) {
    err_connx_list_.push_back(conn_obj->get_using_session_id());
  }

  if (status != ODBCM_RC_SUCCESS) {
    pfc_log_error("ODBCM::ODBCManager::GetModifiedRows: "
      "Error in preparing query statement: %s",
      ODBCMUtils::get_RC_Details(status).c_str());
    status = ODBCM_RC_STMT_ERROR;
    /* Freeing all allocated memory */
    ODBCMFreeingMemory(get_stmt, table_id, db_varbind, query_factory,
                           query_processor);
    return status;
  }
  status = (db_varbind->*db_varbind->BindOUTParameter)
                  ((*i_list), get_stmt);
  if (status != ODBCM_RC_SUCCESS) {
    pfc_log_error("ODBCM::ODBCManager::GetModifiedRows: "
                  "Error in binding o/p: %s",
                  ODBCMUtils::get_RC_Details(status).c_str());
    status = ODBCM_RC_PARAM_BIND_ERROR;
    /* Freeing all allocated memory */
    ODBCMFreeingMemory(get_stmt, table_id, db_varbind, query_factory,
                       query_processor);
    return status;
  }
  PHY_SQLEXEC_LOCK();
  status = query_processor->ExecuteReadDBQuery(
                                               GETMODIFIEDROWS, get_stmt);
  ODBCM_ROLLBACK_TRANSACTION(ro_conn_handle);
  if (status != ODBCM_RC_SUCCESS) {
    if (status != ODBCM_RC_RECORD_NOT_FOUND)
      pfc_log_error("ODBCM::ODBCManager::GetModifiedRows: "
                    "Error in ExecuteReadDBQuery: %s",
                    ODBCMUtils::get_RC_Details(status).c_str());
    /* Freeing all allocated memory */
    ODBCMFreeingMemory(get_stmt, table_id, db_varbind, query_factory,
                       query_processor);
    return status;
  }

  /** To fetch all returned rows */
  /** Fetch the no. of row return in last executed query */
  odbc_rc = SQLRowCount(get_stmt, &iRow_count);
  ODBCM_PROCESS_HANDLE_CHECK(get_stmt, odbc_rc);
  if (odbc_rc == ODBCM_RC_CONNECTION_ERROR) {
    err_connx_list_.push_back(conn_obj->get_using_session_id());
  }
  if (odbc_rc != ODBCM_RC_SUCCESS &&
      odbc_rc != ODBCM_RC_SUCCESS_WITH_INFO) {
    pfc_log_debug("ODBCM::ODBCManager::GetModifiedRows: "
                  "SQLRowCount:status %s",
                  ODBCMUtils::get_RC_Details(odbc_rc).c_str());
    status = (ODBCM_RC_STATUS)odbc_rc;
    /* Freeing all allocated memory */
    ODBCMFreeingMemory(get_stmt, table_id, db_varbind, query_factory,
                       query_processor);
    return status;
  }
  pfc_log_info("ODBCM::ODBCManager::GetModifiedRows: "
      "Row count: %" UNC_PFMT_SQLLEN, iRow_count);
  if (iRow_count < 0) {
    pfc_log_debug("ODBCM::ODBCManager::GetModifiedRows: "
                  "No more record found");
    status = ODBCM_RC_RECORD_NO_MORE;
    /* Freeing all allocated memory */
    ODBCMFreeingMemory(get_stmt, table_id, db_varbind, query_factory,
                       query_processor);
    return status;
  }
  db_varbind->SetValueStruct(table_id, BIND_OUT);
  odbc_rc = SQLFetch(get_stmt);
  if (odbc_rc != ODBCM_RC_SUCCESS &&
      odbc_rc != ODBCM_RC_SUCCESS_WITH_INFO) {
    pfc_log_debug("ODBCM::ODBCManager::GetModifiedRows: "
                  "SQLFetch:status %s",
                  ODBCMUtils::get_RC_Details(odbc_rc).c_str());
    status = ODBCM_RC_ERROR_FETCHING_ROW;
    /* Freeing all allocated memory */
    ODBCMFreeingMemory(get_stmt, table_id, db_varbind, query_factory,
                       query_processor);
    return status;
  }
  /** While framing any get request, ITC allocates
   * memory for the attributes in the list.This memory
   * should be freed from db schema before calling
   * fetch functions where, memory for all attributes
   * allocated again */
  std::vector<TableAttrSchema>::iterator tmp_iter;
  for (tmp_iter = (*i_list).begin();
      tmp_iter != (*i_list).end();
      tmp_iter++) {
    if ((*tmp_iter).p_table_attribute_value)
      ::operator delete((*tmp_iter).p_table_attribute_value);
  }

  status = (db_varbind->*db_varbind->FetchOUTPUTValues)
                    ((*i_list));
  if (iRow_count > 1) {
    std::vector<TableAttrSchema> new_col_attr((*i_list));
    db_varbind->SetValueStruct(table_id, BIND_OUT);
    odbc_rc = SQLFetch(get_stmt);
    while (odbc_rc != SQL_NO_DATA) {
      if (odbc_rc != ODBCM_RC_SUCCESS &&
          odbc_rc != ODBCM_RC_SUCCESS_WITH_INFO) {
        pfc_log_error("ODBCM::ODBCManager::GetModifiedRows: "
                      "Error in SQLFetch: %s",
                      ODBCMUtils::get_RC_Details(odbc_rc).c_str());
        status = ODBCM_RC_ERROR_FETCHING_ROW;
        /* Freeing all allocated memory */
        ODBCMFreeingMemory(get_stmt, table_id, db_varbind, query_factory,
                           query_processor);
        return status;
      }
      status = (db_varbind->*db_varbind->FetchOUTPUTValues)
                      (new_col_attr);

      if (status != ODBCM_RC_SUCCESS) {
        pfc_log_error("ODBCM::ODBCManager::GetModifiedRows: "
                      "Error in Fetching: %s",
                      ODBCMUtils::get_RC_Details(status).c_str());
        status = ODBCM_RC_ERROR_FETCHING_ROW;
        /* Freeing all allocated memory */
        ODBCMFreeingMemory(get_stmt, table_id, db_varbind, query_factory,
                           query_processor);
        return status;
      }
      db_varbind->SetValueStruct(table_id, BIND_OUT);
      odbc_rc = SQLFetch(get_stmt);
      /** Push the attribute vector to row_list.
       * Not required for 1st time */
      rlist.push_back(new_col_attr);
    }
  }

  db_table_schema.set_row_list(rlist);
  pfc_log_debug("ODBCM::ODBCManager::GetModifiedRows:"
      "dbtableschema list size: %" PFC_PFMT_SIZE_T,
      db_table_schema.row_list_.size());
  status = ODBCM_RC_SUCCESS;
  // db_table_schema.PrintDBTableSchema();
  /** Freeing all allocated memory */
  ODBCMFreeingMemory(get_stmt, table_id, db_varbind, query_factory,
                         query_processor);
  return status;
}

/**
 * @Description : To get the sibling rows count based upon given filter
 * @param[in]   : db_name - specifies the configuration
 *                i.e.candidate/running/startup
 *                db_table_schema - object holds the key and value struct
 *                of specified KT instance
 *                filter_operators - vector to decide the filter while
 *                framing query
 * @param[out]  : count - To return the sibling row count
 * @return      : ODBCM_RC_SUCCESS is returned when the row count
 *                received from DB otherwise DB related error
 *                code will be returned
 **/
ODBCM_RC_STATUS ODBCManager::GetSiblingCount(
    unc_keytype_datatype_t db_name,
    DBTableSchema& db_table_schema,
    uint32_t& count,
    std::vector<ODBCMOperator> filter_operators,
    OdbcmConnectionHandler *conn_obj) {
  PHY_FINI_READ_LOCK();
  /** To receive the query from queryfactory*/
  std::string QUERY("\0");
  /** odbc API's retrun code */
  SQLRETURN odbc_rc = ODBCM_RC_SUCCESS;
  /** other ODBCM methods return code*/
  ODBCM_RC_STATUS status = ODBCM_RC_SUCCESS;
  /** row_count value initialize*/
  SQLLEN row_count = 0;
  count = 0;
  /**iterator for list entry in DBTableSchema*/
  std::list <std::vector<TableAttrSchema> >::iterator i_list;
  /**Initialize the table_id with unknown type*/
  ODBCMTable table_id = UNKNOWN_TABLE;
  /**Statement handler*/
  HSTMT stmt = NULL;
  SQLHDBC ro_conn_handle = conn_obj->get_conn_handle();
  /* sql handle allocate for sql stmt */
  ODBCM_STATEMENT_CREATE(ro_conn_handle, stmt, odbc_rc);
  /** Create query_factory object */
  QueryFactory *query_factory = NULL;
  ODBCM_CREATE_OBJECT(query_factory, QueryFactory);
  /** Create query_processor object */
  QueryProcessor *query_processor = NULL;
  ODBCM_CREATE_OBJECT(query_processor, QueryProcessor);
  /** Create db_varbind object */
  DBVarbind *db_varbind = NULL;
  ODBCM_CREATE_OBJECT(db_varbind, DBVarbind);
  /**set function ptr for GETSIBLINGCOUNT operation*/
  query_factory->SetOperation(GETSIBLINGCOUNT_FILTER);
  if (db_name == UNC_DT_STARTUP) {
    // check startup validity , if not valid, read from running
    PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
    PhysicalCore* physical_core = physical_layer->get_physical_core();
    if (!physical_core->getStartupValidStatus()) {
      db_name = UNC_DT_RUNNING;
      pfc_log_debug("ODBCM::ODBCManager::GetSiblingCount filter:"
                    "modify db name to RUNNING");
    }
  }
  /** Construct query in query factory and return here */
  QUERY = (query_factory->*query_factory->GetFilterCountQuery)
            (db_name, db_table_schema, filter_operators);
  if (QUERY.empty()) {
    pfc_log_error("ODBCM::ODBCManager::GetSiblingCount(with filter): "
        "Error in framing query");
    status = ODBCM_RC_ERROR_IN_FRAMEQUERY;
    /* Freeing all allocated memory */
    ODBCMFreeingMemory(stmt, table_id, db_varbind, query_factory,
                           query_processor);
    return status;
  }
  /** Validate the table information */
  table_id = db_table_schema.get_table_name();
  if (table_id == UNKNOWN_TABLE) {
    pfc_log_error("ODBCM::ODBCManager::GetSiblingCount(with filter): "
        "Error Unknown table: %d",
        db_table_schema.get_table_name());
    status = ODBCM_RC_TABLE_NOT_FOUND;
    /* Freeing all allocated memory */
    ODBCMFreeingMemory(stmt, table_id, db_varbind, query_factory,
                           query_processor);
    return status;
  }
  /** Prepare the query statement */
  status = query_processor->PrepareQueryStatement(QUERY, stmt);
  if (status == ODBCM_RC_CONNECTION_ERROR) {
    err_connx_list_.push_back(conn_obj->get_using_session_id());
  }

  if (status != ODBCM_RC_SUCCESS) {
    pfc_log_error("ODBCM::ODBCManager::GetSiblingCount(with filter): "
      "Error in preparing statement: %s",
      ODBCMUtils::get_RC_Details(status).c_str());
    status = ODBCM_RC_STMT_ERROR;
    /* Freeing all allocated memory */
    ODBCMFreeingMemory(stmt, table_id, db_varbind, query_factory,
                           query_processor);
    return status;
  }
  /* Set the binding */
  db_varbind->SetBinding(table_id, BIND_IN);
  db_varbind->SetValueStruct(table_id, BIND_IN);
  /** No need to traverse the list */
  /** Binding the input is required */
  i_list = db_table_schema.row_list_.begin();
  if (i_list != db_table_schema.row_list_.end()) {
    /** Filling the values into binded structure variables */
    status = (db_varbind->*db_varbind->FillINPUTValues)((*i_list));
    if (status != ODBCM_RC_SUCCESS) {
      pfc_log_error("ODBCM::ODBCManager::GetSiblingCount(with filter): "
          "Error in filling i/p: %s",
          ODBCMUtils::get_RC_Details(status).c_str());
      status = ODBCM_RC_PARAM_BIND_ERROR;
      /* Freeing all allocated memory */
      ODBCMFreeingMemory(stmt, table_id, db_varbind, query_factory,
                             query_processor);
      return status;
    }

    status = (db_varbind->*db_varbind->BindINParameter)
              ((*i_list), stmt);
    if (status != ODBCM_RC_SUCCESS) {
      pfc_log_error("ODBCM::ODBCManager::GetSiblingCount(with filter): "
          "Error in binding  i/p: %s",
          ODBCMUtils::get_RC_Details(status).c_str());
      status = ODBCM_RC_PARAM_BIND_ERROR;
      /* Freeing all allocated memory */
      ODBCMFreeingMemory(stmt, table_id, db_varbind, query_factory,
                             query_processor);
      return status;
    }
    PHY_SQLEXEC_LOCK();
    status = query_processor->ExecuteReadDBQuery(
              GETSIBLINGCOUNT_FILTER, stmt);
    ODBCM_ROLLBACK_TRANSACTION(ro_conn_handle);
    if (status == ODBCM_RC_CONNECTION_ERROR) {
      err_connx_list_.push_back(conn_obj->get_using_session_id());
    }
    if (status != ODBCM_RC_SUCCESS) {
      pfc_log_debug("ODBCM::ODBCManager::GetSiblingCount(with filter): "
                      "ExecuteReadDBQuery status %s",
        ODBCMUtils::get_RC_Details(status).c_str());
      /* Freeing all allocated memory */
      ODBCMFreeingMemory(stmt, table_id, db_varbind, query_factory,
                             query_processor);
      if (status == ODBCM_RC_RECORD_NOT_FOUND)
        status = ODBCM_RC_SUCCESS;
      return status;
    }

    odbc_rc = SQLRowCount(stmt, reinterpret_cast<SQLLEN*> (&row_count));
    if (odbc_rc != ODBCM_RC_SUCCESS &&
        odbc_rc != ODBCM_RC_SUCCESS_WITH_INFO) {
      pfc_log_debug("ODBCM::ODBCManager::GetSiblingCount(with filter): "
          "SQLRowCount:status %s",
          ODBCMUtils::get_RC_Details(odbc_rc).c_str());
      status = ODBCM_RC_RECORD_NO_MORE;
      /* Freeing all allocated memory */
      ODBCMFreeingMemory(stmt, table_id, db_varbind, query_factory,
                             query_processor);
      return status;
    }
    /** Update the sibling count */
    if (row_count == 0)
      status = ODBCM_RC_SUCCESS;
    count = row_count;
    pfc_log_info("ODBCM::ODBCManager::GetSiblingCount(with filter):"
        " row_count = %d", (uint32_t)row_count);
  } else {
    pfc_log_info("ODBCM::ODBCManager::GetSiblingCount(with filter):"
                 "No input data received !");
  }
  /* Freeing all allocated memory */
  ODBCMFreeingMemory(stmt, table_id, db_varbind, query_factory,
                         query_processor);
  return status;
}

/**
 * @Description : To return the sibiling rows in the given table and 
 *                based upon the given filter criteria
 * @param[in]   : db_name - specifies the configuration
 *                i.e.candidate/running/startup
 *                max_repetition_count - specifies the number of rows
 *                to return
 *                db_table_schema - object holds the key and value struct
 *                of specified KT instance
 *                filter_operators - vector to decide the filter while
 *                framing query
 *                op_type - specifies Operation type siblingbegin/sibling
 * @param[out]  : DBTableSchema& db_table_schema
 * @return      : ODBCM_RC_SUCCESS is returned when the siblingrows
 *                received from DB otherwise DB related error
 *                code will be returned
 **/
ODBCM_RC_STATUS ODBCManager::GetSiblingRows(
    unc_keytype_datatype_t db_name,
    uint32_t max_repetition_count,
    DBTableSchema& db_table_schema,
    std::vector<ODBCMOperator> filter_operators,
    unc_keytype_operation_t op_type,
    OdbcmConnectionHandler *conn_obj) {
  PHY_FINI_READ_LOCK();
  /** To receive the query from query factory*/
  std::string QUERY("\0");
  /** ODBC APIs return code*/
  SQLRETURN odbc_rc = ODBCM_RC_SUCCESS;
  /** ODBCM methods return code */
  ODBCM_RC_STATUS status = ODBCM_RC_SUCCESS;
  /** table id is initialized with unknown*/
  ODBCMTable table_id = UNKNOWN_TABLE;
  /** row count variable to store the count*/
  SQLLEN iRow_count = 0;
  /** To traverse the row list in db_table_schmea */
  std::list <std::vector<TableAttrSchema> >::iterator it_vect;
  /** To traverse the row lists vectors in db_table_schmea */
  std::list < std::vector <TableAttrSchema> >& rlist =
      db_table_schema.get_row_list();
  it_vect = rlist.begin();
  if (it_vect == rlist.end()) {
    pfc_log_info("ODBCM::ODBCManager::GetSiblingCount(with filter):"
                     "No input data received !");
    return ODBCM_RC_FAILED;
  }
  HSTMT get_stmt = NULL;
  SQLHDBC ro_conn_handle = conn_obj->get_conn_handle();
  /* Do sql allocate for sql stmt */
  ODBCM_STATEMENT_CREATE(ro_conn_handle, get_stmt, odbc_rc);

  QueryFactory *query_factory = NULL;
  ODBCM_CREATE_OBJECT(query_factory, QueryFactory);

  QueryProcessor *query_processor = NULL;
  ODBCM_CREATE_OBJECT(query_processor, QueryProcessor);

  DBVarbind *db_varbind = NULL;
  ODBCM_CREATE_OBJECT(db_varbind, DBVarbind);

  query_factory->SetOperation(GETSIBLINGROWS);
  if (db_name == UNC_DT_STARTUP) {
    // check startup validity , if not valid, read from running
    PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
    PhysicalCore* physical_core = physical_layer->get_physical_core();
    if (!physical_core->getStartupValidStatus()) {
      db_name = UNC_DT_RUNNING;
      pfc_log_debug("ODBCM::ODBCManager::GetSiblingRows: modify db to RUNNING");
    }
  }
  /** construct query in query factory and return here */
  QUERY = (query_factory->*query_factory->GetSiblingFilterQuery)
    (db_name, db_table_schema, max_repetition_count, filter_operators, op_type);
  if (QUERY.empty()) {
    pfc_log_error("ODBCM::ODBCManager::GetSiblingRows: "
        "Error in framing query");
    status = ODBCM_RC_ERROR_IN_FRAMEQUERY;
    /* Freeing all allocated memory */
    ODBCMFreeingMemory(get_stmt, table_id, db_varbind, query_factory,
                           query_processor);
    return status;
  }
  /** Validate the table information */
  table_id = db_table_schema.get_table_name();
  if (table_id == UNKNOWN_TABLE) {
    pfc_log_error("ODBCM::ODBCManager::GetSiblingRows: "
      "Unknown table: %d",
      db_table_schema.get_table_name());
    status = ODBCM_RC_TABLE_NOT_FOUND;
    /* Freeing all allocated memory */
    ODBCMFreeingMemory(get_stmt, table_id, db_varbind, query_factory,
                           query_processor);
    return status;
  }

  /** prepare sql statment with constructed query string  */
  status = query_processor->PrepareQueryStatement(QUERY, get_stmt);
  if (status == ODBCM_RC_CONNECTION_ERROR) {
    err_connx_list_.push_back(conn_obj->get_using_session_id());
  }

  if (status !=  ODBCM_RC_SUCCESS) {
    pfc_log_error("ODBCM::ODBCManager::GetSiblingRows: "
        "Error in preparing query statement: %s",
        ODBCMUtils::get_RC_Details(status).c_str());
    status = ODBCM_RC_QUERY_FAILED;
    /* Freeing all allocated memory */
    ODBCMFreeingMemory(get_stmt, table_id, db_varbind, query_factory,
                           query_processor);
    return status;
  }
  /** Get table id */
  table_id = db_table_schema.get_table_name();
  if (table_id == UNKNOWN_TABLE) {
    pfc_log_error("ODBCM::ODBCManager::GetSiblingRows: "
      "Error Unknown table: %d",
      db_table_schema.get_table_name());
    status = ODBCM_RC_TABLE_NOT_FOUND;
    /* Freeing all allocated memory */
    ODBCMFreeingMemory(get_stmt, table_id, db_varbind, query_factory,
                           query_processor);
    return status;
  }
  /** function pointer set for Binding parameter, 
    * binding values on the binded structure parameter, 
    * based upon table id corresponding bind and fill 
    * value method will be called out */
  db_varbind->SetBinding(table_id, BIND_IN);
  db_varbind->SetValueStruct(table_id, BIND_IN);
  db_varbind->SetBinding(table_id, BIND_OUT);
  db_varbind->SetValueStruct(table_id, BIND_OUT);
  /** To filling the values into binded structure variables */
  status = (db_varbind->*db_varbind->FillINPUTValues)((*it_vect));
  if (status != ODBCM_RC_SUCCESS) {
    pfc_log_error("ODBCM::ODBCManager::GetSiblingRows: "
                  "Error in filling i/p: %s",
                  ODBCMUtils::get_RC_Details(status).c_str());
    status = ODBCM_RC_PARAM_BIND_ERROR;
    /* Freeing all allocated memory */
    ODBCMFreeingMemory(get_stmt, table_id, db_varbind, query_factory,
                       query_processor);
    return status;
  }
  /** To bind table input values - the corresponding table
   * structure variables will be binded here */
  status = (db_varbind->*db_varbind->BindINParameter)
                  ((*it_vect), get_stmt);
  if (status != ODBCM_RC_SUCCESS) {
    pfc_log_error("ODBCM::ODBCManager::GetSiblingRows: "
                  "Error binding i/p: %s",
                  ODBCMUtils::get_RC_Details(status).c_str());
    status = ODBCM_RC_PARAM_BIND_ERROR;
    /* Freeing all allocated memory */
    ODBCMFreeingMemory(get_stmt, table_id, db_varbind, query_factory,
                       query_processor);
    return status;
  }
  status = (db_varbind->*db_varbind->BindOUTParameter)
                  ((*it_vect), get_stmt);
  if (status != ODBCM_RC_SUCCESS) {
    pfc_log_error("ODBCM::ODBCManager::GetSiblingRows: "
                  "Error binding o/p: %s",
                  ODBCMUtils::get_RC_Details(status).c_str());
    status = ODBCM_RC_PARAM_BIND_ERROR;
    /* Freeing all allocated memory */
    ODBCMFreeingMemory(get_stmt, table_id, db_varbind, query_factory,
                       query_processor);
    return status;
  }
  PHY_SQLEXEC_LOCK();
  /** Execute the ReadDBQuery with the above statement */
  status = query_processor->ExecuteReadDBQuery(
                                               GETSIBLINGROWS, get_stmt);
  ODBCM_ROLLBACK_TRANSACTION(ro_conn_handle);
  if (status == ODBCM_RC_CONNECTION_ERROR) {
    err_connx_list_.push_back(conn_obj->get_using_session_id());
  }
  /** Fill the database output in dbtableschema */
  if (status != ODBCM_RC_SUCCESS) {
    pfc_log_error("ODBCM::ODBCManager::GetSiblingRows: "
                  "ExecuteReadDBQuery status %s",
                  ODBCMUtils::get_RC_Details(status).c_str());
    /* Freeing all allocated memory */
    ODBCMFreeingMemory(get_stmt, table_id, db_varbind, query_factory,
                       query_processor);
    return status;
  }

  /** To fetch all returned rows Fetch the no. of row return in last
   * executed query */
  odbc_rc = SQLRowCount(get_stmt, &iRow_count);
  ODBCM_PROCESS_HANDLE_CHECK(get_stmt, odbc_rc);
  if (odbc_rc == ODBCM_RC_CONNECTION_ERROR) {
    err_connx_list_.push_back(conn_obj->get_using_session_id());
  }
  if (odbc_rc != ODBCM_RC_SUCCESS &&
      odbc_rc != ODBCM_RC_SUCCESS_WITH_INFO) {
    pfc_log_debug("ODBCM::ODBCManager::GetSiblingRows: "
                  "SQLRowCount:status %s",
                  ODBCMUtils::get_RC_Details(odbc_rc).c_str());
    status = (ODBCM_RC_STATUS)odbc_rc;
    /* Freeing all allocated memory */
    ODBCMFreeingMemory(get_stmt, table_id, db_varbind, query_factory,
                       query_processor);
    return status;
  }
  pfc_log_info("ODBCM::ODBCManager::GetSiblingRows: "
      "Row count = %" UNC_PFMT_SQLLEN, iRow_count);
  if (iRow_count <= 0) {
    pfc_log_debug("ODBCM::ODBCManager::GetSiblingRows: "
                  "No more record found ");
    status = ODBCM_RC_RECORD_NO_MORE;
    /* Freeing all allocated memory */
    ODBCMFreeingMemory(get_stmt, table_id, db_varbind, query_factory,
                       query_processor);
    return status;
  }
  db_varbind->SetValueStruct(table_id, BIND_OUT);
  odbc_rc = SQLFetch(get_stmt);
  if (odbc_rc != ODBCM_RC_SUCCESS &&
      odbc_rc != ODBCM_RC_SUCCESS_WITH_INFO) {
    pfc_log_error("ODBCM::ODBCManager::GetSiblingRows: "
                  "Error in SQLFetch: %s",
                  ODBCMUtils::get_RC_Details(odbc_rc).c_str());
    status = ODBCM_RC_ERROR_FETCHING_ROW;
    /* Freeing all allocated memory */
    ODBCMFreeingMemory(get_stmt, table_id, db_varbind, query_factory,
                       query_processor);
    return status;
  }
  /** While framing any get request, ITC allocates
   * memory for the attributes in the list.This memory
   * should be freed from db schema before calling
   * fetch functions where, memory for all attributes
   * allocated again */
  std::vector<TableAttrSchema>::iterator tmp_iter;
  for (tmp_iter = (*it_vect).begin();
      tmp_iter != (*it_vect).end();
      tmp_iter++) {
    if ((*tmp_iter).p_table_attribute_value)
      ::operator delete((*tmp_iter).p_table_attribute_value);
  }
  status = (db_varbind->*db_varbind->FetchOUTPUTValues)((*it_vect));
  if (iRow_count > 1) {
    std::vector<TableAttrSchema> new_col_attr((*it_vect));
    db_varbind->SetValueStruct(table_id, BIND_OUT);
    odbc_rc = SQLFetch(get_stmt);
    while (odbc_rc != SQL_NO_DATA) {
      if (odbc_rc != ODBCM_RC_SUCCESS &&
          odbc_rc != ODBCM_RC_SUCCESS_WITH_INFO) {
        pfc_log_error("ODBCM::ODBCManager::GetSiblingRows: "
                      "Error in fetching: %s",
                      ODBCMUtils::get_RC_Details(odbc_rc).c_str());
        status = ODBCM_RC_ERROR_FETCHING_ROW;
        /* Freeing all allocated memory */
        ODBCMFreeingMemory(get_stmt, table_id, db_varbind, query_factory,
                           query_processor);
        return status;
      }
      status = (db_varbind->*db_varbind->FetchOUTPUTValues)(new_col_attr);
      if (status != ODBCM_RC_SUCCESS) {
        pfc_log_error("ODBCM::ODBCManager::GetSiblingRows: "
                      "Error in fetching output: %s",
                      ODBCMUtils::get_RC_Details(status).c_str());
        status = ODBCM_RC_ERROR_FETCHING_ROW;
        /* Freeing all allocated memory */
        ODBCMFreeingMemory(get_stmt, table_id, db_varbind, query_factory,
                           query_processor);
        return status;
      }
      db_varbind->SetValueStruct(table_id, BIND_OUT);
      odbc_rc = SQLFetch(get_stmt);
      /** Push the attribute vector to row_list.
       * Not required for 1st time */
      rlist.push_back(new_col_attr);
    }
  }

  db_table_schema.set_row_list(rlist);
  pfc_log_debug("ODBCM::ODBCManager::GetSiblingRows: "
      "dbtableschema list size:%" PFC_PFMT_SIZE_T,
      db_table_schema.row_list_.size());
  status = ODBCM_RC_SUCCESS;
  // db_table_schema.PrintDBTableSchema();
  /* Freeing all allocated memory */
  ODBCMFreeingMemory(get_stmt, table_id, db_varbind, query_factory,
                         query_processor);
  return status;
}
/**EOF*/
