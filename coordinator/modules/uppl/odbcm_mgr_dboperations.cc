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
 * @file    odbcm_mgr_dboperations.cc
 *
 */

#include "odbcm_mgr.hh"
#include "odbcm_query_factory.hh"
#include "odbcm_query_processor.hh"
#include "odbcm_db_varbind.hh"
#include "odbcm_utils.hh"
#include "physicallayer.hh"
using unc::uppl::ODBCManager;
using std::string;
/**
 * @Description : clear the entries in the database tables.
 * @param[in]   : unc_keytype_datatype_t
 * @return      : ODBCM_RC_STATUS
 **/
ODBCM_RC_STATUS ODBCManager::ClearDatabase(unc_keytype_datatype_t db_name) {
  PHY_FINI_READ_LOCK();
  string            *cleardb_query = NULL;       // to store query
  SQLRETURN         odbc_rc = ODBCM_RC_SUCCESS;  //  SQL APIs rc
  ODBCM_RC_STATUS   status = ODBCM_RC_SUCCESS;   // other methods rc
  HSTMT             clear_stmt = NULL;           // db statement
  QueryFactory      *query_factory = NULL;
  QueryProcessor    *query_processor = NULL;

  /** Allocate for db statement handle */
  ODBCM_STATEMENT_CREATE(rw_conn_handle_, clear_stmt);
  /** Create query_factory and query processor objects */
  ODBCM_CREATE_OBJECT(query_factory, QueryFactory);
  ODBCM_CREATE_OBJECT(query_processor, QueryProcessor);

  /** Func. ptr for queryfactory to construct the query */
  query_factory->SetOperation(CLEARDATABASE);
  cleardb_query = (query_factory->*query_factory->GetSingleDBQuery)
                    (db_name);
  if (cleardb_query == NULL) {
    pfc_log_error("ODBCM::ODBCManager::ClearDatabase: "
        "Error in framing query");
    status = ODBCM_RC_ERROR_IN_FRAMEQUERY;
    /* Freeing all allocated memory */
    ODBCMFreeingMemory(clear_stmt, UNKNOWN_TABLE, NULL, query_factory,
                           query_processor);
    return status;
  }
  status = query_processor->ExecuteTransaction(
              CLEARDATABASE, cleardb_query, clear_stmt);
  if (status == ODBCM_RC_SUCCESS) {
    /** Commit all active transactions on this connection*/
    ODBCM_END_TRANSACTION(rw_conn_handle_, SQL_COMMIT);
    pfc_log_info("ODBCM::ODBCManager::ClearDatabase:database is cleared");
  } else {
    /** Rollback all active transactions on this connection*/
    ODBCM_END_TRANSACTION(rw_conn_handle_, SQL_ROLLBACK);
    pfc_log_info("ODBCM::ODBCManager::ClearDatabase:database is not cleared");
  }
  /* Freeing all allocated memory */
  if (cleardb_query != NULL) {
    delete [] cleardb_query;
    cleardb_query = NULL;
  }
  ODBCMFreeingMemory(clear_stmt, UNKNOWN_TABLE, NULL, query_factory,
                         query_processor);
  return status;
}
/**
 * @Description : This method will copy databases, copy all 
 *                contents from src db table to dst db table.
 * @param[in]   : unc_keytype_datatype_t , unc_keytype_datatype_t
 * @return      : ODBCM_RC_STATUS
 **/
ODBCM_RC_STATUS ODBCManager::CopyDatabase(
    unc_keytype_datatype_t src_db_name,
    unc_keytype_datatype_t dst_db_name) {
  PHY_FINI_READ_LOCK();
  /** Initialise the local variables */
  string          *p_copydb_query = NULL;
  HSTMT           copy_stmt= NULL;
  QueryFactory    *p_query_factory = NULL;
  QueryProcessor  *query_processor = NULL;
  SQLRETURN       odbc_rc = ODBCM_RC_SUCCESS;  // SQL API return value
  ODBCM_RC_STATUS status = ODBCM_RC_SUCCESS;   // other methods return value
  /** Copy db should be called only for below scenarios */
  if (!((src_db_name == UNC_DT_STARTUP && dst_db_name == UNC_DT_CANDIDATE) ||
      (src_db_name == UNC_DT_CANDIDATE && dst_db_name == UNC_DT_RUNNING) ||
      (src_db_name == UNC_DT_RUNNING && dst_db_name == UNC_DT_CANDIDATE) ||
      (src_db_name == UNC_DT_RUNNING && dst_db_name == UNC_DT_STARTUP))) {
    pfc_log_error("ODBCM::ODBCManager::CopyDatabase: Invalid copy request"
      " %s->%s", g_log_db_name[src_db_name], g_log_db_name[dst_db_name]);
    return ODBCM_RC_INVALID_DB_OPERATION;
  }
  /** Create p_query_factory and query processor objects */
  ODBCM_STATEMENT_CREATE(rw_conn_handle_, copy_stmt);
  ODBCM_CREATE_OBJECT(p_query_factory, QueryFactory);
  ODBCM_CREATE_OBJECT(query_processor, QueryProcessor);
  /** Set the FPTR */
  p_query_factory->SetOperation(COPYDATABASE);
  p_copydb_query = (p_query_factory->*p_query_factory->GetTwoDBQuery)
    (src_db_name, dst_db_name);
  if (p_copydb_query == NULL) {
    pfc_log_error("ODBCM::ODBCManager::CopyDatabase: "
                 "Error in framing copy query");
    status = ODBCM_RC_ERROR_IN_FRAMEQUERY;
    /* Freeing all allocated memory */
    ODBCMFreeingMemory(copy_stmt, UNKNOWN_TABLE, NULL, p_query_factory,
                           query_processor);
    return status;
  }
  status = query_processor->ExecuteTransaction(
    COPYDATABASE, p_copydb_query, copy_stmt);
  if (status == ODBCM_RC_SUCCESS) {
    /** Commit all active transactions on this connection*/
    ODBCM_END_TRANSACTION(rw_conn_handle_, SQL_COMMIT);
    pfc_log_info("ODBCM::ODBCManager::CopyDatabase:database is copied");
  } else {
    /** Rollback all active transactions on this connection*/
    ODBCM_END_TRANSACTION(rw_conn_handle_, SQL_ROLLBACK);
    pfc_log_info("ODBCM::ODBCManager::CopyDatabase:database is not copied");
  }
  /* Freeing all allocated memory */
  if (p_copydb_query != NULL) {
    delete [] p_copydb_query;
    p_copydb_query = NULL;
  }
  ODBCMFreeingMemory(copy_stmt, UNKNOWN_TABLE, NULL, p_query_factory,
                         query_processor);
  return status;
}


/**
 * @Description : This method will check whether the candidate 
 *                database is dirty. Dirty means, is there any 
 *                uncommited rows are present in candiate config. 
 *                other than APPLIED row status are treated as dirty.
 * @param[in]   : None
 * @return      : ODBCM_RC_STATUS
 **/
ODBCM_RC_STATUS ODBCManager::IsCandidateDirty() {
  PHY_FINI_READ_LOCK();
  string            *query = NULL;  // to receive the query from queryfactory
  SQLRETURN         odbc_rc = ODBCM_RC_SUCCESS;  // SQL APIs rc
  ODBCM_RC_STATUS   status = ODBCM_RC_SUCCESS;
  HSTMT             isdirty_stmt = NULL;  // statement for iscandidatedirty
  QueryFactory      *query_factory = NULL;
  QueryProcessor    *query_processor = NULL;

  /** Allocate for db statement handle */
  ODBCM_STATEMENT_CREATE(ro_conn_handle_, isdirty_stmt);
  /** Create query_factory and query processor objects */
  ODBCM_CREATE_OBJECT(query_factory, QueryFactory);
  ODBCM_CREATE_OBJECT(query_processor, QueryProcessor);

  /** Func. ptr for queryfactory to construct the query */
  query_factory->SetOperation(ISCANDIDATEDIRTY);
  query = (query_factory->*query_factory->GetIsDBQuery)();
  if (query == NULL) {
    pfc_log_error("ODBCM::ODBCManager::IsCandidateDirty: "
        "Error in framing query");
    status = ODBCM_RC_ERROR_IN_FRAMEQUERY;
    /* Freeing all allocated memory */
    ODBCMFreeingMemory(isdirty_stmt, UNKNOWN_TABLE, NULL, query_factory,
                           query_processor);
    return status;
  }
  /** Execute the query statements as a single transaction */
  status = query_processor->ExecuteTransaction(
            ISCANDIDATEDIRTY, query, isdirty_stmt);
  if ((status == ODBCM_RC_STMT_ERROR) ||
      (status == ODBCM_RC_DATA_ERROR)) {
    pfc_log_error("ODBCM::ODBCManager::IsCandidateDirty: "
      "Error in ExecuteTransaction: %s",
        ODBCMUtils::get_RC_Details(status).c_str());
    status = ODBCM_RC_STMT_ERROR;
  } else {
    pfc_log_debug("ODBCM::ODBCManager::IsCandidateDirty: "
        "ExecuteTransaction returns: %s",
        ODBCMUtils::get_RC_Details(status).c_str());
  }
  /* Freeing all allocated memory */
  if (query != NULL) {
    delete [] query;
    query = NULL;
  }
  ODBCMFreeingMemory(isdirty_stmt, UNKNOWN_TABLE, NULL, query_factory,
                         query_processor);
  return status;
}

/**
 * @Description : To commit all the configuration from candidate to
 *                running database. Below operations follows after
 *                invoking this method.
 *                STEP1: Change row_status in Candidate db
 *                STEP1.1: Change row_status CREATED/UPDATED --> APPLIED
 *                STEP1.2: Remove row_staus(DELETED) drom the table
 *                STEP2: TRUNCATE all the tables in Running db
 *                STEP3: Copy Candidate->Running if above steps are success
 *                copydatabase will be reused for step3.
 * @param[in]   : unc_data_type_t src_db,unc_data_type_t dst_db
 * @return      : ODBCM_RC_STATUS
 **/
ODBCM_RC_STATUS ODBCManager::CommitAllConfiguration(
    unc_keytype_datatype_t src_db,
    unc_keytype_datatype_t dst_db) {
  PHY_FINI_READ_LOCK();
  HSTMT           commit_stmt       = NULL;
  string          *p_commit_query   = NULL;
  uint8_t         query_type        = 0;
  QueryFactory    *query_factory    = NULL;
  QueryProcessor  *query_processor  = NULL;
  SQLRETURN       odbc_rc           = ODBCM_RC_SUCCESS;  // SQL API rc
  ODBCM_RC_STATUS status            = ODBCM_RC_SUCCESS;  // other methods rc
  /** Validate the db name - During commit it will be
    *always from CANDIDATE->RUNNING */
  if (src_db != UNC_DT_CANDIDATE || dst_db != UNC_DT_RUNNING) {
      pfc_log_error("ODBCM::ODBCManager::CommitAllConfiguration:"
        " Invalid commit request. %s->%s",
        g_log_db_name[src_db], g_log_db_name[dst_db]);
      return ODBCM_RC_INVALID_DB_OPERATION;
  }
  /** Create query_factory and query processor objects */
  ODBCM_STATEMENT_CREATE(rw_conn_handle_, commit_stmt);
  ODBCM_CREATE_OBJECT(query_factory, QueryFactory);
  ODBCM_CREATE_OBJECT(query_processor, QueryProcessor);
  /** Fptr for queryfactory to construct COMMITALLCONFIG query */
  for (query_type = 0; query_type < COMMIT_END; query_type++) {
    if (query_type != COPY_CANDIDATE_TO_RUNNING) {
      /* Frame the query and execute it */
      query_factory->SetOperation(COMMITALLCONFIG);
      p_commit_query = (query_factory->*query_factory->
        CommitTableQuery) (src_db, dst_db, query_type);
      if (p_commit_query == NULL) {
        pfc_log_error("ODBCM::ODBCManager::CommitAllConfiguration: "
          "Error in framing query_type:%d", query_type);
        status = ODBCM_RC_ERROR_IN_FRAMEQUERY;
        /* Freeing all allocated memory */
        ODBCMFreeingMemory(commit_stmt, UNKNOWN_TABLE, NULL, query_factory,
                               query_processor);
        return status;
      }
      status = query_processor->ExecuteTransaction(
          COMMITALLCONFIG, p_commit_query, commit_stmt);
    } else {
      /* Final step to copy config from CANDIDATE->RUNNING*/
      query_factory->SetOperation(COPYDATABASE);
      p_commit_query = (query_factory->*query_factory->
        GetTwoDBQuery) (src_db, dst_db);
      if (p_commit_query == NULL) {
        pfc_log_error("ODBCM::ODBCManager::CommitAllConfiguration: "
          "Error in framing copy query");
        status = ODBCM_RC_ERROR_IN_FRAMEQUERY;
        /* Freeing all allocated memory */
        ODBCMFreeingMemory(commit_stmt, UNKNOWN_TABLE, NULL, query_factory,
                               query_processor);
        return status;
      }
      status = query_processor->ExecuteTransaction(
          COPYDATABASE, p_commit_query, commit_stmt);
    }
    if (status != ODBCM_RC_SUCCESS &&
        status != ODBCM_RC_SUCCESS_WITH_INFO &&
        status != SQL_NO_DATA) {
      pfc_log_error("ODBCM::ODBCManager::CommitAllConfiguration:"
          " Error in ExecuteTransaction: %s",
          ODBCMUtils::get_RC_Details(status).c_str());
      status = ODBCM_RC_STMT_ERROR;
      break;
    }
    if (p_commit_query != NULL) {
      delete []p_commit_query;
      p_commit_query = NULL;
    }
  }
  if (status == ODBCM_RC_SUCCESS) {
    /** Commit all active transactions on this connection*/
    ODBCM_END_TRANSACTION(rw_conn_handle_, SQL_COMMIT);
    pfc_log_info("ODBCM::ODBCManager::CommitAllConfiguration: "
        "ODBCM level commit is completed");
  } else {
    /** Rollback all active transactions on this connection*/
    ODBCM_END_TRANSACTION(rw_conn_handle_, SQL_ROLLBACK);
    pfc_log_info("ODBCM::ODBCManager::CommitAllConfiguration: "
        "ODBCM level commit is not completed");
  }
  /* Freeing all allocated memory */
  if (p_commit_query != NULL) {
    delete [] p_commit_query;
  }
  ODBCMFreeingMemory(commit_stmt, UNKNOWN_TABLE, NULL, query_factory,
                         query_processor);
  return status;
}
/**
 * @Description : To clear controller_name instance in all the tables in DB
 * @param[in]   : unc_keytype_datatype_t db_name, string controller_name
 * @return      : ODBCM_RC_STATUS
 **/
ODBCM_RC_STATUS ODBCManager::ClearOneInstance(
    unc_keytype_datatype_t db_name,
    string controller_name) {
  PHY_FINI_READ_LOCK();
  /** Initialise the local variables */
  std::string       *QUERY = NULL;
  SQLRETURN         odbc_rc = ODBCM_RC_SUCCESS;
  ODBCM_RC_STATUS   status = ODBCM_RC_SUCCESS;
  HSTMT             stmt = NULL;
  QueryFactory      *query_factory = NULL;
  QueryProcessor    *query_processor = NULL;

  /** Do sql allocate for sql stmt */
  ODBCM_STATEMENT_CREATE(rw_conn_handle_, stmt);
  /** Create query_factory and query processor objects */
  ODBCM_CREATE_OBJECT(query_factory, QueryFactory);
  ODBCM_CREATE_OBJECT(query_processor, QueryProcessor);

  query_factory->SetOperation(CLEARONEINSTANCE);
  /** Construct query in query factory and return here */
  QUERY = (query_factory->*query_factory->GetClearInstanceQuery)
                        (db_name, controller_name);
  if (NULL == QUERY) {
    pfc_log_error("ODBCM::ODBCManager::ClearOneInstance: "
        "Error in framing query");
    status = ODBCM_RC_ERROR_IN_FRAMEQUERY;
    /* Freeing all allocated memory */
    ODBCMFreeingMemory(stmt, UNKNOWN_TABLE, NULL, query_factory,
                           query_processor);
    return status;
  }

  status = query_processor->ExecuteTransaction(
      CLEARONEINSTANCE, QUERY, stmt);
  if (status == ODBCM_RC_SUCCESS) {
    ODBCM_END_TRANSACTION(rw_conn_handle_, SQL_COMMIT);
    pfc_log_info("ODBCM::ODBCManager::ClearOneInstance: "
      "given one instance is cleared");
  } else {
    ODBCM_END_TRANSACTION(rw_conn_handle_, SQL_ROLLBACK);
    pfc_log_info("ODBCM::ODBCManager::ClearOneInstance: "
      "given one instance is not cleared");
  }
  /* Freeing all allocated memory */
  ODBCMFreeingMemory(stmt, UNKNOWN_TABLE, NULL, query_factory,
                         query_processor);
  /* Free the string pointer */
  if (QUERY != NULL) {
    delete []QUERY;
    QUERY = NULL;
  }
  return status;
}
/**EOF*/
