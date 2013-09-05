/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * dal_odbc_mgr.cc
 *   Contains implementation of DalOdbcMgr class
 */

#include <time.h>
#include <sstream>

#include "pfcxx/module.hh"
#include "uncxx/upll_log.hh"
#include "dal_odbc_mgr.hh"
#include "dal_query_builder.hh"
#include "dal_error_handler.hh"

namespace unc {
namespace upll {
namespace dal {

// Constructor
DalOdbcMgr::DalOdbcMgr() {
  dal_env_handle_ = SQL_NULL_HANDLE;
  dal_conn_handle_ = SQL_NULL_HANDLE;
  conn_type_ = kDalConnReadOnly;
}

/* Desctructor */
DalOdbcMgr::~DalOdbcMgr() {
  // TODO(sankar): what if FreeHandle returns error?
  FreeHandle(SQL_HANDLE_DBC, dal_conn_handle_);
  FreeHandle(SQL_HANDLE_ENV, dal_env_handle_);
}

// Init - Initializes the DB environment
DalResultCode
DalOdbcMgr::Init() {
  SQLRETURN     sql_rc;
  DalResultCode dal_rc;

  // Enabling Conenction Pooling
  sql_rc = SQLSetEnvAttr(SQL_NULL_HANDLE,
                         SQL_ATTR_CONNECTION_POOLING,
                         reinterpret_cast<SQLPOINTER>(SQL_CP_ONE_PER_DRIVER),
                         0);
  DalErrorHandler::ProcessOdbcErrors(SQL_HANDLE_ENV,
                                     SQL_NULL_HANDLE,
                                     sql_rc, &dal_rc);
  if (dal_rc != kDalRcSuccess) {
    UPLL_LOG_TRACE("Err - %d. Failed to Enable Connection Pooling", dal_rc);
  }

  UPLL_LOG_TRACE("Connection Pooling enabled");

  // set Connection Pool Match option
  // SQL_ATTR_CP_MATCH - SQL_CP_STRICT_MATCH (default)

  // Allocate Environment handler
  sql_rc = SQLAllocHandle(SQL_HANDLE_ENV,
                          SQL_NULL_HANDLE,
                          &(dal_env_handle_));
  DalErrorHandler::ProcessOdbcErrors(SQL_HANDLE_ENV,
                                     SQL_NULL_HANDLE,
                                     sql_rc, &dal_rc);

  if (dal_rc != kDalRcSuccess) {
    UPLL_LOG_DEBUG("Err - %d. Failed to allocate Environment handle", dal_rc);
    return dal_rc;
  }

  // PFC_ASSERT(dal_env_handle_);
  if (dal_env_handle_ == SQL_NULL_HANDLE) {
    UPLL_LOG_DEBUG("NULL Environment handle");
    return kDalRcGeneralError;
  }

  UPLL_LOG_TRACE("Allocated Environment handle(%p)", dal_env_handle_);

  // Register environment for ODBC version 3
  sql_rc = SQLSetEnvAttr(dal_env_handle_,
                         SQL_ATTR_ODBC_VERSION,
                         reinterpret_cast<SQLPOINTER>(SQL_OV_ODBC3),
                         0);
  DalErrorHandler::ProcessOdbcErrors(SQL_HANDLE_ENV,
                                     dal_env_handle_,
                                     sql_rc, &dal_rc);
  if (dal_rc != kDalRcSuccess) {
    UPLL_LOG_DEBUG("Err - %d. Failed to register DB with ODBC V3", dal_rc);
    // TODO(sankar): what if FreeHandle returns error?
    FreeHandle(SQL_HANDLE_ENV, dal_env_handle_);
    return dal_rc;
  }

  UPLL_LOG_TRACE("Initialized DB with ODBC V3");

  // allocate connection handle
  sql_rc = SQLAllocHandle(SQL_HANDLE_DBC,
                          dal_env_handle_,
                          &(dal_conn_handle_));
  DalErrorHandler::ProcessOdbcErrors(SQL_HANDLE_ENV,
                                     dal_env_handle_,
                                     sql_rc, &dal_rc);
  if (dal_rc != kDalRcSuccess) {
    UPLL_LOG_DEBUG("Err - %d. Failed to allocate Connection handle", dal_rc);
    FreeHandle(SQL_HANDLE_ENV, dal_env_handle_);
    return dal_rc;
  }

  // PFC_ASSERT(dal_conn_handle_);
  if (dal_conn_handle_ == SQL_NULL_HANDLE) {
    UPLL_LOG_DEBUG("NULL Connection handle");
    FreeHandle(SQL_HANDLE_ENV, dal_env_handle_);
    return kDalRcGeneralError;
  }

  UPLL_LOG_TRACE("Allocated Connection handle(%p)", dal_conn_handle_);
  return kDalRcSuccess;
}  // DalOdbcMgr::Init

// Connects to the Datasource with the conf file data
DalResultCode
DalOdbcMgr::ConnectToDb(const DalConnType conn_type) const {
  SQLRETURN     sql_rc;
  DalResultCode dal_rc;
  // PFC_ASSERT(dal_env_handle_);

  conn_type_ = conn_type;

  if (dal_conn_handle_ == NULL) {
    UPLL_LOG_DEBUG("NULL Connection Handle");
    return kDalRcGeneralError;
  }

  // Set Connection Attributes
  dal_rc = SetConnAttributes(dal_conn_handle_, conn_type);
  if (dal_rc != kDalRcSuccess) {
    UPLL_LOG_TRACE("Err - %d. Some or All Connection Attributes not set", dal_rc);
    return dal_rc;
  }

  // Connecting to database
  std::string dal_conn_str;
  dal_conn_str.clear();
  dal_conn_str = GetConnString(conn_type);
  if (dal_conn_str.empty()) {
    UPLL_LOG_DEBUG("Error building connection string");
    return kDalRcGeneralError;
  }
  sql_rc = SQLDriverConnect(dal_conn_handle_, NULL,
                            reinterpret_cast<SQLCHAR*>
                            (const_cast<char*>(dal_conn_str.c_str())),
                            SQL_NTS, NULL, 0, NULL, SQL_DRIVER_NOPROMPT);
  DalErrorHandler::ProcessOdbcErrors(SQL_HANDLE_DBC,
                                     dal_conn_handle_,
                                     sql_rc, &dal_rc);
  if (dal_rc != kDalRcSuccess) {
    UPLL_LOG_DEBUG("Err - %d. Failed to connect Datatbase with conn string(%s)",
                   dal_rc, dal_conn_str.c_str());
    return dal_rc;
  }

  UPLL_LOG_TRACE("Successfully connected to Datatbase with conn string(%s)",
                 dal_conn_str.c_str());
  return kDalRcSuccess;
}  // DalOdbcMgr::ConnectToDb

// Disconnects from the database
DalResultCode
DalOdbcMgr::DisconnectFromDb() const {
  SQLRETURN     sql_rc;
  DalResultCode dal_rc;

  // PFC_ASSERT(dal_conn_handle_);
  if (dal_conn_handle_ == NULL) {
    UPLL_LOG_DEBUG("NULL Connection Handle");
    return kDalRcGeneralError;
  }

  sql_rc = SQLDisconnect(dal_conn_handle_);
  DalErrorHandler::ProcessOdbcErrors(SQL_HANDLE_DBC,
                                     dal_conn_handle_,
                                     sql_rc, &dal_rc);
  if (dal_rc != kDalRcSuccess) {
    UPLL_LOG_DEBUG("Err - %d. Failed to Disconnect Database for the conenction "
                  "handle(%p)",  dal_rc, dal_conn_handle_);
    return dal_rc;
  }
  UPLL_LOG_TRACE("Successfully Disconnected from Database");
  return kDalRcSuccess;
}  // DalOdbcMgr::DisconnectFromDb


// Commits all the statements executed as part of the transaction
DalResultCode
DalOdbcMgr::CommitTransaction(void) const {
  SQLRETURN     sql_rc;
  DalResultCode dal_rc;

  // PFC_ASSERT(dal_conn_handle_);
  if (dal_conn_handle_ == NULL) {
    UPLL_LOG_DEBUG("NULL Connection Handle");
    return kDalRcGeneralError;
  }

  sql_rc = SQLEndTran(SQL_HANDLE_DBC, dal_conn_handle_, SQL_COMMIT);
  DalErrorHandler::ProcessOdbcErrors(SQL_HANDLE_DBC,
                                     dal_conn_handle_,
                                     sql_rc, &dal_rc);
  if (dal_rc != kDalRcSuccess) {
    UPLL_LOG_DEBUG("Err - %d. Failed to commit transaction for the "
                  "handle(%p)",  dal_rc, dal_conn_handle_);
    return dal_rc;
  }
  UPLL_LOG_TRACE("Successfully committed transaction for the "
                 "handle(%p)",  dal_conn_handle_);
  return kDalRcSuccess;
}  // DalOdbcMgr::CommitTransaction


// Rollbacks all the statements executed as part of the transaction
DalResultCode
DalOdbcMgr::RollbackTransaction(void) const {
  SQLRETURN     sql_rc;
  DalResultCode dal_rc;

  // PFC_ASSERT(dal_conn_handle_);
  if (dal_conn_handle_ == NULL) {
    UPLL_LOG_DEBUG("NULL Connection Handle");
    return kDalRcGeneralError;
  }

  sql_rc = SQLEndTran(SQL_HANDLE_DBC, dal_conn_handle_, SQL_ROLLBACK);
  DalErrorHandler::ProcessOdbcErrors(SQL_HANDLE_DBC,
                                     dal_conn_handle_,
                                     sql_rc, &dal_rc);
  if (dal_rc != kDalRcSuccess) {
    UPLL_LOG_DEBUG("Err - %d. Failed to rollback transaction for the "
                  "handle(%p)",  dal_rc, dal_conn_handle_);
    return dal_rc;
  }
  UPLL_LOG_TRACE("Successfully rollback transaction for the "
                 "handle(%p)",  dal_conn_handle_);
  return kDalRcSuccess;
}  // DalOdbcMgr::RollbackTransaction

// Gets single matching record
DalResultCode
DalOdbcMgr::GetSingleRecord(const UpllCfgType cfg_type,
                            const DalTableIndex table_index,
                            const DalBindInfo *bind_info) const {
  SQLHANDLE     dal_stmt_handle = SQL_NULL_HANDLE;
  SQLRETURN     sql_rc;
  DalResultCode dal_rc;
  DalQueryBuilder  qbldr;
  std::string   query_stmt;

  // Validating Inputs
  if (cfg_type == UPLL_DT_INVALID) {
    UPLL_LOG_DEBUG("Invalid Config Type - %d", cfg_type);
    return kDalRcGeneralError;
  }

  if (table_index >= schema::table::kDalNumTables) {
    UPLL_LOG_DEBUG("Invalid Table Index - %d", table_index);
    return kDalRcGeneralError;
  }

  // PFC_ASSERT(bind_info)
  if (bind_info == NULL) {
    UPLL_LOG_DEBUG("NULL Bind Info for Table(%s)",
                   schema::TableName(table_index));
    return kDalRcGeneralError;
  }

  if (table_index != bind_info->get_table_index()) {
    UPLL_LOG_DEBUG("Table Index Mismatch with bind info "
                   "\n Query - %s; Bind - %s",
                   schema::TableName(table_index),
                   schema::TableName(bind_info->get_table_index()));
    return kDalRcGeneralError;
  }

  // Build Query Statement
  if (qbldr.get_sql_statement(kDalGetSingleRecQT, bind_info, query_stmt,
                        table_index, cfg_type) != true) {
    UPLL_LOG_DEBUG("Failed Building Query Stmt");
    return kDalRcGeneralError;
  } else {
    UPLL_LOG_TRACE("Query Stmt - %s", query_stmt.c_str());
  }

  // Allocate Stmt Handle, Bind and Execute the Query Statement
  dal_rc = ExecuteQuery(&dal_stmt_handle,
                        &query_stmt,
                        bind_info);

  if (dal_rc != kDalRcSuccess) {
    UPLL_LOG_DEBUG("Err - %d. Failed Executing Query Stmt - %s",
                   dal_rc, query_stmt.c_str());
    FreeHandle(SQL_HANDLE_STMT, dal_stmt_handle);
    return dal_rc;
  }
  UPLL_LOG_TRACE("Completed Executing Query Stmt - %s", query_stmt.c_str());

  // Fetching results from the resultset
  sql_rc = SQLFetch(dal_stmt_handle);
  DalErrorHandler::ProcessOdbcErrors(SQL_HANDLE_STMT,
                                     dal_stmt_handle,
                                     sql_rc, &dal_rc);
  if (dal_rc != kDalRcSuccess) {
    UPLL_LOG_DEBUG("%d - Failed to Fetch result", dal_rc);
    FreeHandle(SQL_HANDLE_STMT, dal_stmt_handle);
    return dal_rc;
  }
  UPLL_LOG_TRACE("Result fetched from DB");

  if (const_cast<DalBindInfo*>(bind_info)->CopyResultToApp() != true) {
    UPLL_LOG_DEBUG("Failed to Copy result to the DAL User Buffer");
    FreeHandle(SQL_HANDLE_STMT, dal_stmt_handle);
    return kDalRcGeneralError;
  }
  UPLL_LOG_TRACE("Copied result to the DAL User Buffer");
  UPLL_LOG_TRACE("%s",
      ((const_cast<DalBindInfo *>(bind_info))->BindListResultToStr()).c_str());

  FreeHandle(SQL_HANDLE_STMT, dal_stmt_handle);
  return kDalRcSuccess;
}  // DalOdbcMgr::GetSingleRecord

// Gets multiple matching records
DalResultCode
DalOdbcMgr::GetMultipleRecords(const UpllCfgType cfg_type,
                               const DalTableIndex table_index,
                               const size_t max_record_count,
                               const DalBindInfo *bind_info,
                               DalCursor **cursor) const {
  SQLHANDLE     dal_stmt_handle = SQL_NULL_HANDLE;
  DalResultCode dal_rc;
  DalQueryBuilder  qbldr;
  std::string query_stmt;

  // Validating Inputs
  if (cfg_type == UPLL_DT_INVALID) {
    UPLL_LOG_DEBUG("Invalid Config Type - %d", cfg_type);
    return kDalRcGeneralError;
  }

  if (table_index >= schema::table::kDalNumTables) {
    UPLL_LOG_DEBUG("Invalid Table Index - %d", table_index);
    return kDalRcGeneralError;
  }

  // PFC_ASSERT(bind_info)
  if (bind_info == NULL) {
    UPLL_LOG_DEBUG("NULL Bind Info for Table(%s)",
                   schema::TableName(table_index));
    return kDalRcGeneralError;
  }

  if (table_index != bind_info->get_table_index()) {
    UPLL_LOG_DEBUG("Table Index Mismatch with bind info "
                   "\n Query - %s; Bind - %s",
                   schema::TableName(table_index),
                   schema::TableName(bind_info->get_table_index()));
    return kDalRcGeneralError;
  }

  // Build Query Statement
  if (qbldr.get_sql_statement(kDalGetMultiRecQT, bind_info, query_stmt,
                        table_index, cfg_type) != true) {
    UPLL_LOG_DEBUG("Failed Building Query Stmt");
    return kDalRcGeneralError;
  } else {
    UPLL_LOG_TRACE("Query Stmt - %s", query_stmt.c_str());
  }

  // Allocate Stmt Handle, Bind and Execute the Query Statement
  dal_rc = ExecuteQuery(&dal_stmt_handle,
                        &query_stmt,
                        bind_info,
                        max_record_count);

  if (dal_rc != kDalRcSuccess) {
    UPLL_LOG_DEBUG("Err - %d. Failed Executing Query Stmt - %s",
                   dal_rc, query_stmt.c_str());
    FreeHandle(SQL_HANDLE_STMT, dal_stmt_handle);
    return dal_rc;
  }
  UPLL_LOG_TRACE("Completed Executing Query Stmt - %s",
                query_stmt.c_str());

  *cursor = new DalCursor(dal_stmt_handle,
                          bind_info);
  // PFC_ASSERT(*cursor)
  if (*cursor == NULL) {
    UPLL_LOG_DEBUG("Failed to allocate cursor handle for the result");
    FreeHandle(SQL_HANDLE_STMT, dal_stmt_handle);
    return kDalRcGeneralError;
  }
  UPLL_LOG_TRACE("Allocated cursor handle for the result");
  return kDalRcSuccess;
}   // DalOdbcMgr::GetMultipleRecords


// Get Next Record from the cursor
DalResultCode
DalOdbcMgr::GetNextRecord(const DalCursor *cursor) const {
  DalResultCode dal_rc;

  if (cursor == NULL) {
    UPLL_LOG_DEBUG("NULL Cursor Handle");
    return kDalRcGeneralError;
  }

  dal_rc = cursor->GetNextRecord();

  if (dal_rc != kDalRcSuccess) {
    if (dal_rc == kDalRcRecordNoMore) {
      UPLL_LOG_TRACE("Err - %d. No More Records in Cursor handle (%p)",
                     dal_rc, cursor);
    } else {
      UPLL_LOG_DEBUG("Err - %d. Error Fetching Next Record from Cursor"
                     " handle(%p)", dal_rc, cursor);
    }
    return dal_rc;
  }

  UPLL_LOG_TRACE("Successful Fetching Next Record from Cursor handle(%p)",
                 cursor);
  return kDalRcSuccess;
}  // DalOdbcMgr::GetNextRecord


// Close the cursor and destroy cursor object
DalResultCode
DalOdbcMgr::CloseCursor(DalCursor *cursor, bool delete_bind) const {
  DalResultCode dal_rc;

  if (cursor == NULL) {
    UPLL_LOG_DEBUG("NULL Cursor Handle");
    return kDalRcGeneralError;
  }

  dal_rc = cursor->CloseCursor(delete_bind);
  if (dal_rc != kDalRcSuccess) {
    UPLL_LOG_TRACE("Error closing Cursor Handle(%p)", cursor);
    delete cursor;
    return dal_rc;
  }
  delete cursor;
  UPLL_LOG_TRACE("Successful closing Cursor Handle");
  return kDalRcSuccess;
}   // DalOdbcMgr::CloseCursor

// Checks the existence of matching record
DalResultCode
DalOdbcMgr::RecordExists(const UpllCfgType cfg_type,
                         const DalTableIndex table_index,
                         const DalBindInfo *bind_info,
                         bool *existence) const {
  DalResultCode dal_rc;
  uint32_t count;

  *existence = false;
  dal_rc = GetRecordCount(cfg_type, table_index, bind_info, &count);
  if (dal_rc != kDalRcSuccess) {
    return dal_rc;
  }

  if (count > 0) {
    *existence = true;
  }

  UPLL_LOG_TRACE("Completed Executing RecordExists and result of"
                " existence is %d",  *existence);
  return kDalRcSuccess;
}

// Gets all the sibling records
DalResultCode
DalOdbcMgr::GetSiblingBegin(const UpllCfgType cfg_type,
                            const DalTableIndex table_index,
                            const size_t max_record_count,
                            const DalBindInfo *bind_info,
                            DalCursor **cursor) const {
  SQLHANDLE     dal_stmt_handle = SQL_NULL_HANDLE;
  DalResultCode dal_rc;
  DalQueryBuilder  qbldr;
  std::string query_stmt;

  // Validating Inputs
  if (cfg_type == UPLL_DT_INVALID) {
    UPLL_LOG_DEBUG("Invalid Config Type - %d", cfg_type);
    return kDalRcGeneralError;
  }

  if (table_index >= schema::table::kDalNumTables) {
    UPLL_LOG_DEBUG("Invalid Table Index - %d", table_index);
    return kDalRcGeneralError;
  }

  // PFC_ASSERT(bind_info)
  if (bind_info == NULL) {
    UPLL_LOG_DEBUG("NULL Bind Info for Table(%s)",
                   schema::TableName(table_index));
    return kDalRcGeneralError;
  }

  if (table_index != bind_info->get_table_index()) {
    UPLL_LOG_DEBUG("Table Index Mismatch with bind info "
                   "\n Query - %s; Bind - %s",
                   schema::TableName(table_index),
                   schema::TableName(bind_info->get_table_index()));
    return kDalRcGeneralError;
  }

  // Build Query Statement
  if (qbldr.get_sql_statement(kDalGetSibBegQT, bind_info, query_stmt,
                        table_index, cfg_type) != true) {
    UPLL_LOG_DEBUG("Failed Building Query Stmt");
    return kDalRcGeneralError;
  } else {
    UPLL_LOG_TRACE("Query Stmt - %s", query_stmt.c_str());
  }

  // Allocate Stmt Handle, Bind and Execute the Query Statement
  dal_rc = ExecuteQuery(&dal_stmt_handle,
                        &query_stmt,
                        bind_info,
                        max_record_count);
  if (dal_rc != kDalRcSuccess) {
    UPLL_LOG_DEBUG("Err - %d. Failed Executing Query Stmt - %s",
                   dal_rc, query_stmt.c_str());
    FreeHandle(SQL_HANDLE_STMT, dal_stmt_handle);
    return dal_rc;
  }

  *cursor = new DalCursor(dal_stmt_handle, bind_info);
  // PFC_ASSERT(*cursor)
  if (*cursor == NULL) {
    UPLL_LOG_DEBUG("Failed to allocate cursor for the result");
    FreeHandle(SQL_HANDLE_STMT, dal_stmt_handle);
    return kDalRcGeneralError;
  }
  UPLL_LOG_TRACE("Allocated cursor for the result");
  return kDalRcSuccess;
}   // DalOdbcMgr::GetSiblingBegin

// Gets the successive sibling records
DalResultCode
DalOdbcMgr::GetSiblingRecords(const UpllCfgType cfg_type,
                              const DalTableIndex table_index,
                              const size_t max_record_count,
                              const DalBindInfo *bind_info,
                              DalCursor **cursor) const {
  SQLHANDLE     dal_stmt_handle = SQL_NULL_HANDLE;
  DalResultCode dal_rc;
  DalQueryBuilder  qbldr;
  std::string query_stmt;

  // Validating Inputs
  if (cfg_type == UPLL_DT_INVALID) {
    UPLL_LOG_DEBUG("Invalid Config Type - %d", cfg_type);
    return kDalRcGeneralError;
  }

  if (table_index >= schema::table::kDalNumTables) {
    UPLL_LOG_DEBUG("Invalid Table Index - %d", table_index);
    return kDalRcGeneralError;
  }

  // PFC_ASSERT(bind_info)
  if (bind_info == NULL) {
    UPLL_LOG_DEBUG("NULL Bind Info for Table(%s)",
                   schema::TableName(table_index));
    return kDalRcGeneralError;
  }

  if (table_index != bind_info->get_table_index()) {
    UPLL_LOG_DEBUG("Table Index Mismatch with bind info "
                   "\n Query - %s; Bind - %s",
                   schema::TableName(table_index),
                   schema::TableName(bind_info->get_table_index()));
    return kDalRcGeneralError;
  }

  // Build Query Statement
  if (qbldr.get_sql_statement(kDalGetSibRecQT, bind_info, query_stmt,
                        table_index, cfg_type) != true) {
    UPLL_LOG_DEBUG("Failed Building Query Stmt");
    return kDalRcGeneralError;
  } else {
    UPLL_LOG_TRACE("Query Stmt - %s", query_stmt.c_str());
  }

  // Allocate Stmt Handle, Bind and Execute the Query Statement
  dal_rc = ExecuteQuery(&dal_stmt_handle,
                        &query_stmt,
                        bind_info,
                        max_record_count);
  if (dal_rc != kDalRcSuccess) {
    UPLL_LOG_DEBUG("Err - %d. Failed Executing Query Stmt - %s",
                   dal_rc, query_stmt.c_str());
    FreeHandle(SQL_HANDLE_STMT, dal_stmt_handle);
    return dal_rc;
  }

  *cursor = new DalCursor(dal_stmt_handle, bind_info);
  if (*cursor == NULL) {
    UPLL_LOG_DEBUG("Failed to allocate cursor for the result");
    FreeHandle(SQL_HANDLE_STMT, dal_stmt_handle);
    return kDalRcGeneralError;
  }
  UPLL_LOG_TRACE("Allocated cursor for the result");
  return kDalRcSuccess;
}   // DalOdbcMgr::GetSiblingRecords

// Gets the count of sibling records
DalResultCode
DalOdbcMgr::GetSiblingCount(const UpllCfgType cfg_type,
                            const DalTableIndex table_index,
                            const DalBindInfo *bind_info,
                            uint32_t *count) const {
  SQLHANDLE     dal_stmt_handle = SQL_NULL_HANDLE;
  SQLRETURN     sql_rc;
  DalResultCode dal_rc;
  DalQueryBuilder  qbldr;
  std::string query_stmt;

  // Validating Inputs
  if (cfg_type == UPLL_DT_INVALID) {
    UPLL_LOG_DEBUG("Invalid Config Type - %d", cfg_type);
    return kDalRcGeneralError;
  }

  if (table_index >= schema::table::kDalNumTables) {
    UPLL_LOG_DEBUG("Invalid Table Index - %d", table_index);
    return kDalRcGeneralError;
  }

  // PFC_ASSERT(bind_info)
  if (bind_info == NULL) {
    UPLL_LOG_DEBUG("NULL Bind Info for Table(%s)",
                   schema::TableName(table_index));
    return kDalRcGeneralError;
  }

  if (table_index != bind_info->get_table_index()) {
    UPLL_LOG_DEBUG("Table Index Mismatch with bind info "
                   "\n Query - %s; Bind - %s",
                   schema::TableName(table_index),
                   schema::TableName(bind_info->get_table_index()));
    return kDalRcGeneralError;
  }

  // Build Query Statement
  if (qbldr.get_sql_statement(kDalGetSibCountQT, bind_info, query_stmt,
                        table_index, cfg_type) != true) {
    UPLL_LOG_DEBUG("Failed Building Query Stmt");
    return kDalRcGeneralError;
  } else {
    UPLL_LOG_TRACE("Query Stmt - %s", query_stmt.c_str());
  }

  // Allocate Stmt Handle, Bind and Execute the Query Statement
  dal_rc = ExecuteQuery(&dal_stmt_handle,
                        &query_stmt,
                        bind_info);
  if (dal_rc != kDalRcSuccess) {
    UPLL_LOG_DEBUG("Err - %d. Failed Executing Query Stmt - %s",
                   dal_rc, query_stmt.c_str());
    FreeHandle(SQL_HANDLE_STMT, dal_stmt_handle);
    return dal_rc;
  }
  UPLL_LOG_TRACE("Completed Executing Query Stmt - %s",
                query_stmt.c_str());

  // Binding for count variable
  sql_rc = SQLBindCol(dal_stmt_handle, 1, SQL_C_ULONG, count, 0, NULL);
  DalErrorHandler::ProcessOdbcErrors(SQL_HANDLE_STMT,
                                     dal_stmt_handle,
                                     sql_rc, &dal_rc);
  if (dal_rc != kDalRcSuccess) {
    UPLL_LOG_DEBUG("Err - %d. Failed to Bind Column for count", dal_rc);
    FreeHandle(SQL_HANDLE_STMT, dal_stmt_handle);
    return dal_rc;
  }
  UPLL_LOG_DEBUG("Count variable bound to query");

  // Fetching results from the resultset
  sql_rc = SQLFetch(dal_stmt_handle);
  DalErrorHandler::ProcessOdbcErrors(SQL_HANDLE_STMT,
                                     dal_stmt_handle,
                                     sql_rc, &dal_rc);
  if (dal_rc != kDalRcSuccess) {
    UPLL_LOG_DEBUG("Err - %d. Failed to fetch result from DB", dal_rc);
    FreeHandle(SQL_HANDLE_STMT, dal_stmt_handle);
    return dal_rc;
  }

  UPLL_LOG_TRACE("Count of sibling records - %d", *count);
  FreeHandle(SQL_HANDLE_STMT, dal_stmt_handle);
  return kDalRcSuccess;
}   // DalOdbcMgr::GetSiblingCount

// Gets the count of matching records
DalResultCode
DalOdbcMgr::GetRecordCount(const UpllCfgType cfg_type,
                           const DalTableIndex table_index,
                           const DalBindInfo *bind_info,
                           uint32_t *count) const {
  SQLHANDLE     dal_stmt_handle = SQL_NULL_HANDLE;
  SQLRETURN     sql_rc;
  DalResultCode dal_rc;
  DalQueryBuilder  qbldr;
  std::string query_stmt;

  // Validating Inputs
  if (cfg_type == UPLL_DT_INVALID) {
    UPLL_LOG_DEBUG("Invalid Config Type - %d", cfg_type);
    return kDalRcGeneralError;
  }

  if (table_index >= schema::table::kDalNumTables) {
    UPLL_LOG_DEBUG("Invalid Table Index - %d", table_index);
    return kDalRcGeneralError;
  }

  // BindInput - NA; BindOutput - NA; BindMatch - Optional
  // No need to validate bind_info
  if (bind_info != NULL) {
    if (table_index != bind_info->get_table_index()) {
      UPLL_LOG_DEBUG("Table Index Mismatch with bind info "
                     "\n Query - %s; Bind - %s",
                     schema::TableName(table_index),
                     schema::TableName(bind_info->get_table_index()));
      return kDalRcGeneralError;
    }
  }

  // Build Query Statement
  if (qbldr.get_sql_statement(kDalGetRecCountQT, bind_info, query_stmt,
                        table_index, cfg_type) != true) {
    UPLL_LOG_DEBUG("Failed Building Query Stmt");
    return kDalRcGeneralError;
  } else {
    UPLL_LOG_TRACE("Query Stmt - %s", query_stmt.c_str());
  }

  // Allocate Stmt Handle, Bind and Execute the Query Statement
  dal_rc = ExecuteQuery(&dal_stmt_handle,
                        &query_stmt,
                        bind_info);
  if (dal_rc != kDalRcSuccess) {
    UPLL_LOG_DEBUG("Err - %d. Failed Executing Query Stmt - %s",
                   dal_rc, query_stmt.c_str());
    FreeHandle(SQL_HANDLE_STMT, dal_stmt_handle);
    return dal_rc;
  }
  UPLL_LOG_TRACE("Completed Executing Query Stmt - %s",
                query_stmt.c_str());

  // Binding for count variable
  sql_rc = SQLBindCol(dal_stmt_handle, 1, SQL_C_ULONG, count, 0, NULL);
  DalErrorHandler::ProcessOdbcErrors(SQL_HANDLE_STMT,
                                     dal_stmt_handle,
                                     sql_rc, &dal_rc);
  if (dal_rc != kDalRcSuccess) {
    UPLL_LOG_DEBUG("Err - %d. Failed to Bind Column for count", dal_rc);
    FreeHandle(SQL_HANDLE_STMT, dal_stmt_handle);
    return dal_rc;
  }
  UPLL_LOG_TRACE("Count variable bound to query");

  // Fetching results from the resultset
  sql_rc = SQLFetch(dal_stmt_handle);
  DalErrorHandler::ProcessOdbcErrors(SQL_HANDLE_STMT,
                                     dal_stmt_handle,
                                     sql_rc, &dal_rc);
  if (dal_rc != kDalRcSuccess) {
    UPLL_LOG_DEBUG("Err - %d. Failed to fetch result from DB", dal_rc);
    FreeHandle(SQL_HANDLE_STMT, dal_stmt_handle);
    return dal_rc;
  }

  UPLL_LOG_TRACE("Count of records - %d", *count);
  FreeHandle(SQL_HANDLE_STMT, dal_stmt_handle);
  return kDalRcSuccess;
}   // DalOdbcMgr::GetRecordCount

// Deletes the matching records
DalResultCode
DalOdbcMgr::DeleteRecords(const UpllCfgType cfg_type,
                          const DalTableIndex table_index,
                          const DalBindInfo *bind_info) const {
  SQLHANDLE     dal_stmt_handle;
  DalResultCode dal_rc;
  DalQueryBuilder  qbldr;
  std::string query_stmt;

  // Validating Inputs
  if (cfg_type == UPLL_DT_INVALID) {
    UPLL_LOG_DEBUG("Invalid Config Type - %d", cfg_type);
    return kDalRcGeneralError;
  }

  if (table_index >= schema::table::kDalNumTables) {
    UPLL_LOG_DEBUG("Invalid Table Index - %d", table_index);
    return kDalRcGeneralError;
  }

  // BindInput - NA; BindOutput - NA; BindMatch - Optional
  // No need to validate bind_info
  if (bind_info != NULL) {
    if (table_index != bind_info->get_table_index()) {
      UPLL_LOG_DEBUG("Table Index Mismatch with bind info "
                     "\n Query - %s; Bind - %s",
                     schema::TableName(table_index),
                     schema::TableName(bind_info->get_table_index()));
      return kDalRcGeneralError;
    }
  }

  // Build Query Statement
  if (qbldr.get_sql_statement(kDalDelRecQT, bind_info, query_stmt,
                        table_index, cfg_type) != true) {
    UPLL_LOG_DEBUG("Failed Building Query Stmt");
    return kDalRcGeneralError;
  } else {
    UPLL_LOG_TRACE("Query Stmt - %s", query_stmt.c_str());
  }

  // Allocate Stmt Handle, Bind and Execute the Query Statement
  dal_rc = ExecuteQuery(&dal_stmt_handle,
                        &query_stmt,
                        bind_info);

  if (dal_rc != kDalRcSuccess) {
    UPLL_LOG_DEBUG("Err - %d. Failed Executing Query Stmt - %s",
                   dal_rc, query_stmt.c_str());
    FreeHandle(SQL_HANDLE_STMT, dal_stmt_handle);
    return dal_rc;
  }
  UPLL_LOG_TRACE("Completed Executing Query Stmt - %s",
                query_stmt.c_str());
  FreeHandle(SQL_HANDLE_STMT, dal_stmt_handle);
  return kDalRcSuccess;
}   // DalOdbcMgr::DeleteRecords

// Creates a record with the given data
DalResultCode
DalOdbcMgr::CreateRecord(const UpllCfgType cfg_type,
                         const DalTableIndex table_index,
                         const DalBindInfo *bind_info) const {
  SQLHANDLE     dal_stmt_handle;
  DalResultCode dal_rc;
  DalQueryBuilder  qbldr;
  std::string query_stmt;

  // Validating Inputs
  if (cfg_type == UPLL_DT_INVALID) {
    UPLL_LOG_DEBUG("Invalid Config Type - %d", cfg_type);
    return kDalRcGeneralError;
  }

  if (table_index >= schema::table::kDalNumTables) {
    UPLL_LOG_DEBUG("Invalid Table Index - %d", table_index);
    return kDalRcGeneralError;
  }

  // PFC_ASSERT(bind_info)
  if (bind_info == NULL) {
    UPLL_LOG_DEBUG("NULL Bind Info for Table(%s)",
                   schema::TableName(table_index));
    return kDalRcGeneralError;
  }

  if (table_index != bind_info->get_table_index()) {
    UPLL_LOG_DEBUG("Table Index Mismatch with bind info "
                   "\n Query - %s; Bind - %s",
                   schema::TableName(table_index),
                   schema::TableName(bind_info->get_table_index()));
    return kDalRcGeneralError;
  }

  // Build Query Statement
  if (qbldr.get_sql_statement(kDalCreateRecQT, bind_info, query_stmt,
                        table_index, cfg_type) != true) {
    UPLL_LOG_DEBUG("Failed Building Query Stmt");
    return kDalRcGeneralError;
  } else {
    UPLL_LOG_TRACE("Query Stmt - %s", query_stmt.c_str());
  }

  // Allocate Stmt Handle, Bind and Execute the Query Statement
  dal_rc = ExecuteQuery(&dal_stmt_handle,
                        &query_stmt,
                        bind_info);
  if (dal_rc != kDalRcSuccess) {
    UPLL_LOG_DEBUG("Err - %d. Failed Executing Query Stmt - %s",
                   dal_rc, query_stmt.c_str());
    FreeHandle(SQL_HANDLE_STMT, dal_stmt_handle);
    return dal_rc;
  }
  UPLL_LOG_TRACE("Completed Executing Query Stmt - %s",
                query_stmt.c_str());
  FreeHandle(SQL_HANDLE_STMT, dal_stmt_handle);
  return kDalRcSuccess;
}   // DalOdbcMgr::CreateRecord

// Updates the matching records with the given data
DalResultCode
DalOdbcMgr::UpdateRecords(const UpllCfgType cfg_type,
                          const DalTableIndex table_index,
                          const DalBindInfo *bind_info) const {
  SQLHANDLE     dal_stmt_handle;
  DalResultCode dal_rc;
  DalQueryBuilder  qbldr;
  std::string query_stmt;

  // Validating Inputs
  if (cfg_type == UPLL_DT_INVALID) {
    UPLL_LOG_DEBUG("Invalid Config Type - %d", cfg_type);
    return kDalRcGeneralError;
  }

  if (table_index >= schema::table::kDalNumTables) {
    UPLL_LOG_DEBUG("Invalid Table Index - %d", table_index);
    return kDalRcGeneralError;
  }

  // PFC_ASSERT(bind_info)
  if (bind_info == NULL) {
    UPLL_LOG_DEBUG("NULL Bind Info for Table(%s)",
                   schema::TableName(table_index));
    return kDalRcGeneralError;
  }

  if (table_index != bind_info->get_table_index()) {
    UPLL_LOG_DEBUG("Table Index Mismatch with bind info "
                   "\n Query - %s; Bind - %s",
                   schema::TableName(table_index),
                   schema::TableName(bind_info->get_table_index()));
    return kDalRcGeneralError;
  }

  // Build Query Statement
  if (qbldr.get_sql_statement(kDalUpdateRecQT, bind_info, query_stmt,
                        table_index, cfg_type) != true) {
    UPLL_LOG_DEBUG("Failed Building Query Stmt");
    return kDalRcGeneralError;
  } else {
    UPLL_LOG_TRACE("Query Stmt - %s", query_stmt.c_str());
  }

  // Allocate Stmt Handle, Bind and Execute the Query Statement
  dal_rc = ExecuteQuery(&dal_stmt_handle,
                        &query_stmt,
                        bind_info);

  if (dal_rc != kDalRcSuccess) {
    UPLL_LOG_DEBUG("Err - %d. Failed Executing Query Stmt - %s",
                   dal_rc, query_stmt.c_str());
    FreeHandle(SQL_HANDLE_STMT, dal_stmt_handle);
    return dal_rc;
  }
  UPLL_LOG_TRACE("Completed Executing Query Stmt - %s",
                query_stmt.c_str());
  FreeHandle(SQL_HANDLE_STMT, dal_stmt_handle);
  return kDalRcSuccess;
}   // DalOdbcMgr::UpdateRecords

// Gets the records deleted in cfg_type_1
DalResultCode
DalOdbcMgr::GetDeletedRecords(const UpllCfgType cfg_type_1,
                              const UpllCfgType cfg_type_2,
                              const DalTableIndex table_index,
                              const size_t max_record_count,
                              const DalBindInfo *bind_info,
                              DalCursor **cursor) const {
  SQLHANDLE     dal_stmt_handle;
  DalResultCode dal_rc;
  DalQueryBuilder  qbldr;
  std::string query_stmt;
  *cursor = NULL;

  // Validating Inputs
  if (cfg_type_1 == UPLL_DT_INVALID || cfg_type_2 == UPLL_DT_INVALID) {
    UPLL_LOG_DEBUG("Invalid Config Type - %d %d", cfg_type_1, cfg_type_2);
    return kDalRcGeneralError;
  }

  if (cfg_type_1 == cfg_type_2) {
    UPLL_LOG_DEBUG("Same Config Type - %d %d", cfg_type_1, cfg_type_2);
    return kDalRcGeneralError;
  }

  if (table_index >= schema::table::kDalNumTables) {
    UPLL_LOG_DEBUG("Invalid Table Index - %d", table_index);
    return kDalRcGeneralError;
  }

  // PFC_ASSERT(bind_info)
  if (bind_info == NULL) {
    UPLL_LOG_DEBUG("NULL Bind Info for Table(%s)",
                   schema::TableName(table_index));
    return kDalRcGeneralError;
  }

  if (table_index != bind_info->get_table_index()) {
    UPLL_LOG_DEBUG("Table Index Mismatch with bind info "
                   "\n Query - %s; Bind - %s",
                   schema::TableName(table_index),
                   schema::TableName(bind_info->get_table_index()));
    return kDalRcGeneralError;
  }

  // Build Query Statement
  if (qbldr.get_sql_statement(kDalGetDelRecQT, bind_info, query_stmt,
                        table_index, cfg_type_1, cfg_type_2) != true) {
    UPLL_LOG_TRACE("Failed Building Query Stmt");
    return kDalRcGeneralError;
  } else {
    UPLL_LOG_DEBUG("Query Stmt - %s", query_stmt.c_str());
  }

  // Allocate Stmt Handle, Bind and Execute the Query Statement
  dal_rc = ExecuteQuery(&dal_stmt_handle,
                        &query_stmt,
                        bind_info,
                        max_record_count);

  if (dal_rc != kDalRcSuccess) {
    UPLL_LOG_DEBUG("Err - %d. Failed Executing Query Stmt - %s",
                   dal_rc, query_stmt.c_str());
    FreeHandle(SQL_HANDLE_STMT, dal_stmt_handle);
    return dal_rc;
  }
  UPLL_LOG_TRACE("Completed Executing Query Stmt - %s",
                query_stmt.c_str());

  *cursor = new DalCursor(dal_stmt_handle,
                           bind_info);
  if (*cursor == NULL) {
    UPLL_LOG_DEBUG("Failed to allocate cursor for the result");
    FreeHandle(SQL_HANDLE_STMT, dal_stmt_handle);
    return kDalRcGeneralError;
  }
  UPLL_LOG_TRACE("Allocated cursor for the result");
  return kDalRcSuccess;
}  // DalOdbcMgr::GetDeletedRecords

// Gets the records created in cfg_type_1
DalResultCode
DalOdbcMgr::GetCreatedRecords(const UpllCfgType cfg_type_1,
                              const UpllCfgType cfg_type_2,
                              const DalTableIndex table_index,
                              const size_t max_record_count,
                              const DalBindInfo *bind_info,
                              DalCursor **cursor) const {
  SQLHANDLE     dal_stmt_handle;
  DalResultCode dal_rc;
  DalQueryBuilder  qbldr;
  std::string query_stmt;

  // Validating Inputs
  if (cfg_type_1 == UPLL_DT_INVALID || cfg_type_2 == UPLL_DT_INVALID) {
    UPLL_LOG_DEBUG("Invalid Config Type - %d %d", cfg_type_1, cfg_type_2);
    return kDalRcGeneralError;
  }

  if (cfg_type_1 == cfg_type_2) {
    UPLL_LOG_DEBUG("Same Config Type - %d %d", cfg_type_1, cfg_type_2);
    return kDalRcGeneralError;
  }

  if (table_index >= schema::table::kDalNumTables) {
    UPLL_LOG_DEBUG("Invalid Table Index - %d", table_index);
    return kDalRcGeneralError;
  }

  // PFC_ASSERT(bind_info)
  if (bind_info == NULL) {
    UPLL_LOG_DEBUG("NULL Bind Info for Table(%s)",
                   schema::TableName(table_index));
    return kDalRcGeneralError;
  }

  if (table_index != bind_info->get_table_index()) {
    UPLL_LOG_DEBUG("Table Index Mismatch with bind info "
                   "\n Query - %s; Bind - %s",
                   schema::TableName(table_index),
                   schema::TableName(bind_info->get_table_index()));
    return kDalRcGeneralError;
  }

  // Build Query Statement
  if (qbldr.get_sql_statement(kDalGetCreatedRecQT, bind_info, query_stmt,
                        table_index, cfg_type_1, cfg_type_2) != true) {
    UPLL_LOG_TRACE("Failed Building Query Stmt");
    return kDalRcGeneralError;
  } else {
    UPLL_LOG_DEBUG("Query Stmt - %s", query_stmt.c_str());
  }

  // Allocate Stmt Handle, Bind and Execute the Query Statement
  dal_rc = ExecuteQuery(&dal_stmt_handle,
                        &query_stmt,
                        bind_info,
                        max_record_count);

  if (dal_rc != kDalRcSuccess) {
    UPLL_LOG_DEBUG("Err - %d. Failed Executing Query Stmt - %s",
                   dal_rc, query_stmt.c_str());
    FreeHandle(SQL_HANDLE_STMT, dal_stmt_handle);
    return dal_rc;
  }
  UPLL_LOG_TRACE("Completed Executing Query Stmt - %s",
                query_stmt.c_str());
  *cursor = new DalCursor(dal_stmt_handle,
                           bind_info);
  if (*cursor == NULL) {
    UPLL_LOG_DEBUG("Failed to allocate cursor for the result");
    FreeHandle(SQL_HANDLE_STMT, dal_stmt_handle);
    return kDalRcGeneralError;
  }

  UPLL_LOG_TRACE("Allocated cursor for the result");
  return kDalRcSuccess;
}  // DalOdbcMgr::GetCreatedRecords

// Gets the updated records from cfg_type_1 and cfg_type_2
DalResultCode
DalOdbcMgr::GetUpdatedRecords(const UpllCfgType cfg_type_1,
                              const UpllCfgType cfg_type_2,
                              const DalTableIndex table_index,
                              const size_t max_record_count,
                              const DalBindInfo *cfg_1_bind_info,
                              const DalBindInfo *cfg_2_bind_info,
                              DalCursor **cursor) const {
  SQLHANDLE     cfg_1_stmt_handle = SQL_NULL_HANDLE;
  SQLHANDLE     cfg_2_stmt_handle = SQL_NULL_HANDLE;
  DalResultCode dal_rc;
  DalQueryBuilder  qbldr;
  std::string query_stmt;

  // Validating Inputs
  if (cfg_type_1 == UPLL_DT_INVALID || cfg_type_2 == UPLL_DT_INVALID) {
    UPLL_LOG_DEBUG("Invalid Config Type - (%d, %d)", cfg_type_1, cfg_type_2);
    return kDalRcGeneralError;
  }

  if (cfg_type_1 == cfg_type_2) {
    UPLL_LOG_DEBUG("Same Config Type - (%d, %d)", cfg_type_1, cfg_type_2);
    return kDalRcGeneralError;
  }

  if (table_index >= schema::table::kDalNumTables) {
    UPLL_LOG_DEBUG("Invalid Table Index - %d", table_index);
    return kDalRcGeneralError;
  }

  // PFC_ASSERT(bind_info)
  if (cfg_1_bind_info == NULL || cfg_2_bind_info == NULL) {
    UPLL_LOG_DEBUG("NULL Bind Info for Table(%s)",
                   schema::TableName(table_index));
    return kDalRcGeneralError;
  }

  if (table_index != cfg_1_bind_info->get_table_index() ||
      table_index != cfg_2_bind_info->get_table_index()) {
    UPLL_LOG_DEBUG("Table Index Mismatch with bind info "
                   "\n Query - %s; Bind1 - %s; Bind2 - %s",
                   schema::TableName(table_index),
                   schema::TableName(cfg_1_bind_info->get_table_index()),
                   schema::TableName(cfg_2_bind_info->get_table_index()));
    return kDalRcGeneralError;
  }

  // To Fetch the updated records from cfg_type_1
  // Build Query Statement
  if (qbldr.get_sql_statement(kDalGetModRecQT,
                              cfg_1_bind_info,
                              query_stmt,
                              table_index,
                              cfg_type_1,
                              cfg_type_2) != true) {
    UPLL_LOG_TRACE("Failed Building Query Stmt");
    return kDalRcGeneralError;
  } else {
    UPLL_LOG_DEBUG("Query Stmt - %s", query_stmt.c_str());
  }

  // Allocate Stmt Handle, Bind and Execute the Query Statement
  dal_rc = ExecuteQuery(&cfg_1_stmt_handle,
                        &query_stmt,
                        cfg_1_bind_info,
                        max_record_count);

  if (dal_rc != kDalRcSuccess) {
    UPLL_LOG_DEBUG("Err - %d. Failed Executing Query Stmt - %s",
                   dal_rc, query_stmt.c_str());
    FreeHandle(SQL_HANDLE_STMT, cfg_1_stmt_handle);
    return dal_rc;
  }
  UPLL_LOG_TRACE("Completed Executing Query Stmt - %s",
                query_stmt.c_str());

  // To Fetch the updated records from cfg_type_2
  // Build Query Statement
  if (qbldr.get_sql_statement(kDalGetModRecQT,
                              cfg_2_bind_info,
                              query_stmt,
                              table_index,
                              cfg_type_2,
                              cfg_type_1) != true) {
    UPLL_LOG_TRACE("Failed Building Query Stmt");
    return kDalRcGeneralError;
  } else {
    UPLL_LOG_DEBUG("Query Stmt - %s", query_stmt.c_str());
  }

  // Allocate Stmt Handle, Bind and Execute the Query Statement
  dal_rc = ExecuteQuery(&cfg_2_stmt_handle,
                        &query_stmt,
                        cfg_2_bind_info,
                        max_record_count);

  if (dal_rc != kDalRcSuccess) {
    UPLL_LOG_DEBUG("Err - %d. Failed Executing Query Stmt - %s",
                   dal_rc, query_stmt.c_str());
    if (cfg_2_stmt_handle != SQL_NULL_HANDLE) {
      FreeHandle(SQL_HANDLE_STMT, cfg_2_stmt_handle);
    }
    return dal_rc;
  }

  *cursor = new DalCursor(cfg_1_stmt_handle,
                          cfg_1_bind_info,
                          cfg_2_stmt_handle,
                          cfg_2_bind_info);
  if (*cursor == NULL) {
    UPLL_LOG_DEBUG("Failed to allocate cursor for the result");
    FreeHandle(SQL_HANDLE_STMT, cfg_1_stmt_handle);
    FreeHandle(SQL_HANDLE_STMT, cfg_2_stmt_handle);
    return kDalRcGeneralError;
  }

  UPLL_LOG_TRACE("Allocated cursor for the result for both the config"
               "(%d, %d)",  cfg_type_1, cfg_type_2);
  return kDalRcSuccess;
}  // DalOdbcMgr::GetUpdatedRecords

// Copies the entire records of dest to src cfg_type
DalResultCode
DalOdbcMgr::CopyEntireRecords(const UpllCfgType dest_cfg_type,
                              const UpllCfgType src_cfg_type,
                              const DalTableIndex table_index,
                              const DalBindInfo *bind_info) const {
  SQLHANDLE     dal_stmt_handle = SQL_NULL_HANDLE;
  DalResultCode dal_rc;
  DalQueryBuilder  qbldr;
  std::string query_stmt;

  // Validating Inputs
  if (dest_cfg_type == UPLL_DT_INVALID || src_cfg_type == UPLL_DT_INVALID) {
    UPLL_LOG_DEBUG("Invalid Config Type - %d %d", dest_cfg_type, src_cfg_type);
    return kDalRcGeneralError;
  }

  if (dest_cfg_type == src_cfg_type) {
    UPLL_LOG_DEBUG("Same Config Type - %d %d", dest_cfg_type, src_cfg_type);
    return kDalRcGeneralError;
  }

  if (table_index >= schema::table::kDalNumTables) {
    UPLL_LOG_DEBUG("Invalid Table Index - %d", table_index);
    return kDalRcGeneralError;
  }

  // No need to validate bindinfo
  // BindInput - NA; BindOuput - O; BindMatch - NA;
  if (bind_info != NULL  && table_index != bind_info->get_table_index()) {
    UPLL_LOG_DEBUG("Table Index Mismatch with bind info "
                   "\n Query - %s; Bind - %s",
                   schema::TableName(table_index),
                   schema::TableName(bind_info->get_table_index()));
    return kDalRcGeneralError;
  }

  // Build Query Statement
  // delete all records in dst; copy records to src;
  if (qbldr.get_sql_statement(kDalCopyEntireRecQT, bind_info, query_stmt,
                        table_index, dest_cfg_type, src_cfg_type) != true) {
    UPLL_LOG_TRACE("Failed Building Query Stmt");
    return kDalRcGeneralError;
  } else {
    UPLL_LOG_DEBUG("Query Stmt - %s", query_stmt.c_str());
  }

  // Allocate Stmt Handle, Bind and Execute the Query Statement
  dal_rc = ExecuteQuery(&dal_stmt_handle,
                        &query_stmt, bind_info);

  if (dal_rc != kDalRcSuccess) {
    UPLL_LOG_DEBUG("Err - %d. Failed Executing Query Stmt - %s",
                   dal_rc, query_stmt.c_str());
    FreeHandle(SQL_HANDLE_STMT, dal_stmt_handle);
    return dal_rc;
  }
  UPLL_LOG_TRACE("Completed Executing Query Stmt - %s",
                query_stmt.c_str());
  FreeHandle(SQL_HANDLE_STMT, dal_stmt_handle);
  return kDalRcSuccess;
}   // DalOdbcMgr::CopyEntireRecords

// Copies only the modified records from dst to src cfg_type
DalResultCode
DalOdbcMgr::CopyModifiedRecords(const UpllCfgType dest_cfg_type,
                                const UpllCfgType src_cfg_type,
                                const DalTableIndex table_index,
                                const DalBindInfo *bind_info) const {
  SQLHANDLE     dal_stmt_handle = SQL_NULL_HANDLE;
  DalResultCode dal_rc;
  DalQueryBuilder  qbldr;
  std::string query_stmt;

  // Validating Inputs
  if (dest_cfg_type == UPLL_DT_INVALID || src_cfg_type == UPLL_DT_INVALID) {
    UPLL_LOG_DEBUG("Invalid Config Type - %d %d", dest_cfg_type, src_cfg_type);
    return kDalRcGeneralError;
  }

  if (dest_cfg_type == src_cfg_type) {
    UPLL_LOG_DEBUG("Same Config Type - %d %d", dest_cfg_type, src_cfg_type);
    return kDalRcGeneralError;
  }

  if (table_index >= schema::table::kDalNumTables) {
    UPLL_LOG_DEBUG("Invalid Table Index - %d", table_index);
    return kDalRcGeneralError;
  }

  // No need to validate bindinfo
  // BindInput - NA; BindOuput - O; BindMatch - O;
  if (bind_info != NULL && table_index != bind_info->get_table_index()) {
    UPLL_LOG_DEBUG("Table Index Mismatch with bind info "
                   "\n Query - %s; Bind - %s",
                   schema::TableName(table_index),
                   schema::TableName(bind_info->get_table_index()));
    return kDalRcGeneralError;
  }

  // Delete deleted records from dst
  // Build Query Statement
  if (qbldr.get_sql_statement(kDalCopyModRecDelQT, bind_info, query_stmt,
                        table_index, dest_cfg_type, src_cfg_type) != true) {
    UPLL_LOG_TRACE("Failed Building Query Stmt");
    return kDalRcGeneralError;
  } else {
    UPLL_LOG_DEBUG("Query Stmt - %s", query_stmt.c_str());
  }

  // Allocate Stmt Handle, Bind and Execute the Query Statement
  dal_rc = ExecuteQuery(&dal_stmt_handle,
                        &query_stmt, bind_info);

  if (dal_rc != kDalRcSuccess && dal_rc != kDalRcRecordNotFound) {
    UPLL_LOG_DEBUG("Err - %d. Failed Executing Query Stmt - %s",
                   dal_rc, query_stmt.c_str());
    FreeHandle(SQL_HANDLE_STMT, dal_stmt_handle);
    return dal_rc;
  }
  FreeHandle(SQL_HANDLE_STMT, dal_stmt_handle);
  UPLL_LOG_TRACE("Completed Executing Query Stmt - %s",
                query_stmt.c_str());

  // Insert created records to dst
  if (qbldr.get_sql_statement(kDalCopyModRecCreateQT, bind_info, query_stmt,
                        table_index, dest_cfg_type, src_cfg_type) != true) {
    UPLL_LOG_TRACE("Failed Building Query Stmt");
    return kDalRcGeneralError;
  } else {
    UPLL_LOG_DEBUG("Query Stmt - %s", query_stmt.c_str());
  }

  dal_rc = ExecuteQuery(&dal_stmt_handle,
                        &query_stmt, bind_info);

  if (dal_rc != kDalRcSuccess && dal_rc != kDalRcRecordNotFound) {
    UPLL_LOG_DEBUG("Err - %d. Failed Executing Query Stmt - %s",
                   dal_rc, query_stmt.c_str());
    if (dal_stmt_handle != SQL_NULL_HANDLE) {
      FreeHandle(SQL_HANDLE_STMT, dal_stmt_handle);
    }
    return dal_rc;
  }
  FreeHandle(SQL_HANDLE_STMT, dal_stmt_handle);
  UPLL_LOG_TRACE("Completed Executing Query Stmt - %s",
                query_stmt.c_str());

// Update modified records to dst
  if (qbldr.get_sql_statement(kDalCopyModRecUpdateQT, bind_info, query_stmt,
                        table_index, dest_cfg_type, src_cfg_type) != true) {
    UPLL_LOG_TRACE("Failed Building Query Stmt");
    return kDalRcGeneralError;
  } else {
    UPLL_LOG_DEBUG("Query Stmt - %s", query_stmt.c_str());
  }

  dal_rc = ExecuteQuery(&dal_stmt_handle,
                        &query_stmt, bind_info);

  if (dal_rc != kDalRcSuccess && dal_rc != kDalRcRecordNotFound) {
    UPLL_LOG_DEBUG("Err - %d. Failed Executing Query Stmt - %s",
                   dal_rc, query_stmt.c_str());
    FreeHandle(SQL_HANDLE_STMT, dal_stmt_handle);
    return dal_rc;
  }
  UPLL_LOG_TRACE("Completed Executing Query Stmt - %s",
                query_stmt.c_str());
  FreeHandle(SQL_HANDLE_STMT, dal_stmt_handle);
  return kDalRcSuccess;
}  // DalOdbcMgr::CopyModifiedRecords

// Copies only the modified records from dst to src cfg_type
DalResultCode
DalOdbcMgr::CopyModifiedInsertRecords(const UpllCfgType dest_cfg_type,
                                const UpllCfgType src_cfg_type,
                                const DalTableIndex table_index,
                                const DalBindInfo *bind_info) const {
  SQLHANDLE     dal_stmt_handle = SQL_NULL_HANDLE;
  DalResultCode dal_rc;
  DalQueryBuilder  qbldr;
  std::string query_stmt;

  // Validating Inputs
  if (dest_cfg_type == UPLL_DT_INVALID || src_cfg_type == UPLL_DT_INVALID) {
    UPLL_LOG_DEBUG("Invalid Config Type - %d %d", dest_cfg_type, src_cfg_type);
    return kDalRcGeneralError;
  }

  if (dest_cfg_type == src_cfg_type) {
    UPLL_LOG_DEBUG("Same Config Type - %d %d", dest_cfg_type, src_cfg_type);
    return kDalRcGeneralError;
  }

  if (table_index >= schema::table::kDalNumTables) {
    UPLL_LOG_DEBUG("Invalid Table Index - %d", table_index);
    return kDalRcGeneralError;
  }

  // No need to validate bindinfo
  // BindInput - NA; BindOuput - O; BindMatch - O;
  if (bind_info != NULL && table_index != bind_info->get_table_index()) {
    UPLL_LOG_DEBUG("Table Index Mismatch with bind info "
                   "\n Query - %s; Bind - %s",
                   schema::TableName(table_index),
                   schema::TableName(bind_info->get_table_index()));
    return kDalRcGeneralError;
  }

  // Insert created records to dst
  if (qbldr.get_sql_statement(kDalCopyModRecCreateQT, bind_info, query_stmt,
                        table_index, dest_cfg_type, src_cfg_type) != true) {
    UPLL_LOG_TRACE("Failed Building Query Stmt");
    return kDalRcGeneralError;
  } else {
    UPLL_LOG_DEBUG("Query Stmt - %s", query_stmt.c_str());
  }

  dal_rc = ExecuteQuery(&dal_stmt_handle,
                        &query_stmt, bind_info);

  if (dal_rc != kDalRcSuccess && dal_rc != kDalRcRecordNotFound) {
    UPLL_LOG_DEBUG("Err - %d. Failed Executing Query Stmt - %s",
                   dal_rc, query_stmt.c_str());
    if (dal_stmt_handle != SQL_NULL_HANDLE) {
      FreeHandle(SQL_HANDLE_STMT, dal_stmt_handle);
    }
    return dal_rc;
  }
  FreeHandle(SQL_HANDLE_STMT, dal_stmt_handle);
  UPLL_LOG_TRACE("Completed Executing Query Stmt - %s",
                query_stmt.c_str());
  return kDalRcSuccess;
}  // DalOdbcMgr::CopyModifiedInsertRecords

// Copies the matching records from dst to src cfg_type
DalResultCode
DalOdbcMgr::CopyMatchingRecords(const UpllCfgType dest_cfg_type,
                                const UpllCfgType src_cfg_type,
                                const DalTableIndex table_index,
                                const DalBindInfo *bind_info) const {
  SQLHANDLE dal_stmt_handle = SQL_NULL_HANDLE;
  DalResultCode dal_rc;
  DalQueryBuilder  qbldr;
  std::string query_stmt;

  // Validating Inputs
  if (dest_cfg_type == UPLL_DT_INVALID || src_cfg_type == UPLL_DT_INVALID) {
    UPLL_LOG_DEBUG("Invalid Config Type - %d %d", dest_cfg_type, src_cfg_type);
    return kDalRcGeneralError;
  }

  if (dest_cfg_type == src_cfg_type) {
    UPLL_LOG_DEBUG("Same Config Type - %d %d", dest_cfg_type, src_cfg_type);
    return kDalRcGeneralError;
  }

  if (table_index >= schema::table::kDalNumTables) {
    UPLL_LOG_DEBUG("Invalid Table Index - %d", table_index);
    return kDalRcGeneralError;
  }

  // BindInput - NA; BindOuput - O; BindMatch - M;
  if (bind_info == NULL) {
    UPLL_LOG_DEBUG("NULL Bind Info for Table(%s)",
                   schema::TableName(table_index));
    return kDalRcGeneralError;
  }

  if (table_index != bind_info->get_table_index()) {
    UPLL_LOG_DEBUG("Table Index Mismatch with bind info "
                   "\n Query - %s; Bind - %s",
                   schema::TableName(table_index),
                   schema::TableName(bind_info->get_table_index()));
    return kDalRcGeneralError;
  }


  // Build Query Statement
  if (qbldr.get_sql_statement(kDalCopyMatchingRecQT, bind_info, query_stmt,
                        table_index, dest_cfg_type, src_cfg_type) != true) {
    UPLL_LOG_TRACE("Failed Building Query Stmt");
    return kDalRcGeneralError;
  } else {
    UPLL_LOG_DEBUG("Query Stmt - %s", query_stmt.c_str());
  }

  // Allocate Stmt Handle, Bind and Execute the Query Statement
  dal_rc = ExecuteQuery(&dal_stmt_handle,
                        &query_stmt,
                        bind_info);

  if (dal_rc != kDalRcSuccess) {
    UPLL_LOG_DEBUG("Err - %d. Failed Executing Query Stmt - %s",
                   dal_rc, query_stmt.c_str());
    FreeHandle(SQL_HANDLE_STMT, dal_stmt_handle);
    return dal_rc;
  }

  UPLL_LOG_TRACE("Completed Executing Query Stmt - %s",
                query_stmt.c_str());
  return kDalRcSuccess;
}  // DalOdbcMgr::CopyMatchingRecords

// Checks the matching records are identical in both the cfg_types
DalResultCode
DalOdbcMgr::CheckRecordsIdentical(const UpllCfgType cfg_type_1,
                                  const UpllCfgType cfg_type_2,
                                  const DalTableIndex table_index,
                                  const DalBindInfo *bind_info,
                                  bool *identical) const {
  SQLHANDLE     dal_stmt_handle;
  SQLRETURN     sql_rc;
  DalResultCode dal_rc;
  DalQueryBuilder  qbldr;
  std::string query_stmt;
  uint32_t count = 0;

  // Validating Inputs
  if (cfg_type_1 == UPLL_DT_INVALID || cfg_type_2 == UPLL_DT_INVALID) {
    UPLL_LOG_DEBUG("Invalid Config Type - %d %d", cfg_type_1, cfg_type_2);
    return kDalRcGeneralError;
  }

  if (cfg_type_1 == cfg_type_2) {
    UPLL_LOG_DEBUG("Same Config Type - %d %d", cfg_type_1, cfg_type_2);
    return kDalRcGeneralError;
  }

  if (table_index >= schema::table::kDalNumTables) {
    UPLL_LOG_DEBUG("Invalid Table Index - %d", table_index);
    return kDalRcGeneralError;
  }

  // No need to validate bindinfo
  // BindInput - NA; BindOuput - NA; BindMatch - O;
  if (bind_info != NULL && table_index != bind_info->get_table_index()) {
    UPLL_LOG_DEBUG("Table Index Mismatch with bind info "
                   "\n Query - %s; Bind - %s",
                   schema::TableName(table_index),
                   schema::TableName(bind_info->get_table_index()));
    return kDalRcGeneralError;
  }

  // Build Query Statement
  if (qbldr.get_sql_statement(kDalCheckRecIdenticalQT, bind_info, query_stmt,
                        table_index, cfg_type_1, cfg_type_2) != true) {
    UPLL_LOG_TRACE("Failed Building Query Stmt");
    return kDalRcGeneralError;
  } else {
    UPLL_LOG_DEBUG("Query Stmt - %s", query_stmt.c_str());
  }

  // Allocate Stmt Handle, Bind and Execute the Query Statement
  dal_rc = ExecuteQuery(&dal_stmt_handle,
                        &query_stmt,
                        bind_info);

  if (dal_rc != kDalRcSuccess) {
    UPLL_LOG_DEBUG("Err - %d. Failed Executing Query Stmt - %s",
                   dal_rc, query_stmt.c_str());
    FreeHandle(SQL_HANDLE_STMT, dal_stmt_handle);
    return dal_rc;
  }

  // Binding for count variable
  // count of same records in both the configurations
  sql_rc = SQLBindCol(dal_stmt_handle, 1, SQL_C_ULONG, &count, 0, NULL);
  DalErrorHandler::ProcessOdbcErrors(SQL_HANDLE_STMT,
                                     dal_stmt_handle,
                                     sql_rc, &dal_rc);
  if (dal_rc != kDalRcSuccess) {
    UPLL_LOG_DEBUG("Err - %d. Failed Binding count for Query Stmt", dal_rc);
    FreeHandle(SQL_HANDLE_STMT, dal_stmt_handle);
    return dal_rc;
  }
  UPLL_LOG_DEBUG("Count variable bound for Query Stmt");

  // Fetching results from the resultset
  sql_rc = SQLFetch(dal_stmt_handle);
  DalErrorHandler::ProcessOdbcErrors(SQL_HANDLE_STMT,
                                     dal_stmt_handle,
                                     sql_rc, &dal_rc);
  if (dal_rc != kDalRcSuccess) {
    UPLL_LOG_DEBUG("Err - %d. Failed to fetch result from DB", dal_rc);
    FreeHandle(SQL_HANDLE_STMT, dal_stmt_handle);
    return dal_rc;
  }
  UPLL_LOG_DEBUG("Count of Mismatch records - %d", count);

  if (count == 0) {
    *identical = true;   // set to TRUE
    UPLL_LOG_TRACE("Records are Identical");
  } else {
    *identical = false;   // set to FALSE
    UPLL_LOG_TRACE("Records are not Identical");
  }

  FreeHandle(SQL_HANDLE_STMT, dal_stmt_handle);
  return kDalRcSuccess;
}  // DalOdbcMgr::CheckRecordsIdentical

// Executes User supplied query and returns single record in the result set
DalResultCode
DalOdbcMgr::ExecuteAppQuerySingleRecord(
                         const std::string query_stmt,
                         const DalBindInfo *bind_info) const {
  SQLHANDLE     dal_stmt_handle = SQL_NULL_HANDLE;
  SQLRETURN     sql_rc;
  DalResultCode dal_rc;

  // Validating Inputs
  if (query_stmt.size() == 0) {
    UPLL_LOG_DEBUG("Empty Query Stmt string");
    return kDalRcGeneralError;
  }

  // PFC_ASSERT(bind_info)
  if (bind_info == NULL) {
    UPLL_LOG_DEBUG("NULL Bind Info");
    return kDalRcGeneralError;
  }

  // Allocate Stmt Handle, Bind and Execute the Query Statement
  dal_rc = ExecuteQuery(&dal_stmt_handle, &query_stmt, bind_info);
  if (dal_rc != kDalRcSuccess) {
    UPLL_LOG_DEBUG("Err - %d. Failed Executing Query Stmt - %s",
                   dal_rc, query_stmt.c_str());
    FreeHandle(SQL_HANDLE_STMT, dal_stmt_handle);
    return dal_rc;
  }
  UPLL_LOG_TRACE("Completed Executing Query Stmt - %s", query_stmt.c_str());

  // Fetching results from the resultset
  sql_rc = SQLFetch(dal_stmt_handle);
  DalErrorHandler::ProcessOdbcErrors(SQL_HANDLE_STMT,
                                     dal_stmt_handle,
                                     sql_rc, &dal_rc);
  if (dal_rc != kDalRcSuccess) {
    UPLL_LOG_DEBUG("%d - Failed to Fetch result", dal_rc);
    FreeHandle(SQL_HANDLE_STMT, dal_stmt_handle);
    return dal_rc;
  }
  UPLL_LOG_TRACE("Result fetched from DB");

  if (const_cast<DalBindInfo*>(bind_info)->CopyResultToApp() != true) {
    UPLL_LOG_DEBUG("Failed to Copy result to the DAL User Buffer");
    FreeHandle(SQL_HANDLE_STMT, dal_stmt_handle);
    return kDalRcGeneralError;
  }
  UPLL_LOG_TRACE("Copied result to the DAL User Buffer");
  UPLL_LOG_TRACE("%s",
      ((const_cast<DalBindInfo *>(bind_info))->BindListResultToStr()).c_str());

  FreeHandle(SQL_HANDLE_STMT, dal_stmt_handle);
  return kDalRcSuccess;
}  // DalOdbcMgr::ExecuteAppQuerySingleRecord

// Executes user given query resulting multiple records in the result set
DalResultCode
DalOdbcMgr::ExecuteAppQueryMultipleRecords(
                         const std::string query_stmt,
                         const size_t max_record_count,
                         const DalBindInfo *bind_info,
                         DalCursor **cursor) const {
  SQLHANDLE     dal_stmt_handle = SQL_NULL_HANDLE;
  DalResultCode dal_rc;

  // Validating Inputs
  if (query_stmt.size() == 0) {
    UPLL_LOG_DEBUG("Empty Query Stmt string");
    return kDalRcGeneralError;
  }

  // PFC_ASSERT(bind_info)
  if (bind_info == NULL) {
    UPLL_LOG_DEBUG("NULL Bind Info");
    return kDalRcGeneralError;
  }

  // Allocate Stmt Handle, Bind and Execute the Query Statement
  dal_rc = ExecuteQuery(&dal_stmt_handle,
                        &query_stmt,
                        bind_info,
                        max_record_count);

  if (dal_rc != kDalRcSuccess) {
    UPLL_LOG_DEBUG("Err - %d. Failed Executing Query Stmt - %s",
                   dal_rc, query_stmt.c_str());
    FreeHandle(SQL_HANDLE_STMT, dal_stmt_handle);
    return dal_rc;
  }
  UPLL_LOG_TRACE("Completed Executing Query Stmt - %s",
                query_stmt.c_str());

  *cursor = new DalCursor(dal_stmt_handle,
                          bind_info);
  // PFC_ASSERT(*cursor)
  if (*cursor == NULL) {
    UPLL_LOG_DEBUG("Failed to allocate cursor handle for the result");
    FreeHandle(SQL_HANDLE_STMT, dal_stmt_handle);
    return kDalRcGeneralError;
  }
  UPLL_LOG_TRACE("Allocated cursor handle for the result");
  return kDalRcSuccess;
}   // DalOdbcMgr::ExecuteAppQueryMultipleRecords

// Private Methods of DalOdbcMgr
// Set Connection Attributes
DalResultCode
DalOdbcMgr::SetConnAttributes(const SQLHANDLE conn_handle,
                              const DalConnType conn_type) const {
  SQLRETURN     sql_rc;
  DalResultCode dal_rc;

  if (conn_handle == NULL) {
    UPLL_LOG_DEBUG("NULL Connection Handle");
    return kDalRcGeneralError;
  }

  // Connection settings based on conn_type
  // set connection mode
  // SQL_ATTR_ACCESS_MODE - SQL_MODE_READ_WRITE (default)
  if (conn_type == kDalConnReadOnly) {
    // set connection mode
    // SQL_ATTR_ACCESS_MODE - SQL_MODE_READ_ONLY (default: SQL_MODE_READ_WRITE)
    sql_rc = SQLSetConnectAttr(
                 conn_handle,
                 SQL_ATTR_ACCESS_MODE,
                 reinterpret_cast<SQLPOINTER>(SQL_MODE_READ_ONLY),
                 0);
    if (sql_rc == SQL_SUCCESS) {
      UPLL_LOG_TRACE("Connection mode set to Read-Only");
    } else if (sql_rc == SQL_SUCCESS_WITH_INFO) {
      UPLL_LOG_TRACE(" Connection mode set to default");
    }
    DalErrorHandler::ProcessOdbcErrors(SQL_HANDLE_DBC,
                                       conn_handle,
                                       sql_rc, &dal_rc);
    if (dal_rc != kDalRcSuccess) {
      UPLL_LOG_TRACE(" Failed to set Connection mode. Default value set");
    }
  }

  // set auto commit off
  // SQL_ATTR_AUTOCOMMIT SQL_AUTOCOMMIT_OFF (default: SQL_AUTOCOMMIT_ON)
  sql_rc = SQLSetConnectAttr(conn_handle,
                             SQL_ATTR_AUTOCOMMIT,
                             reinterpret_cast<SQLPOINTER>(SQL_AUTOCOMMIT_OFF),
                             0);
  if (sql_rc == SQL_SUCCESS) {
    UPLL_LOG_TRACE("Auto commit option set to off");
  } else if (sql_rc == SQL_SUCCESS_WITH_INFO) {
    UPLL_LOG_TRACE(" Auto commit option set to default");
  }
  DalErrorHandler::ProcessOdbcErrors(SQL_HANDLE_DBC,
                                     conn_handle,
                                     sql_rc, &dal_rc);
  // TODO(sankar): Do we need to return from here?
  if (dal_rc != kDalRcSuccess) {
    UPLL_LOG_TRACE(" Failed to Set AutoCommit option. Default value set");
    return kDalRcGeneralError;
  }

  // set Transaction Isolation Level
  // default value - default value as in driver/datasource
  // 0, if not supported in both driver and datasource
  sql_rc = SQLSetConnectAttr(
               conn_handle,
               SQL_ATTR_TXN_ISOLATION,
               reinterpret_cast<SQLPOINTER>(SQL_TXN_READ_COMMITTED),
               0);
  if (sql_rc == SQL_SUCCESS) {
    UPLL_LOG_TRACE("Transaction Isolation level set to Read-Committed");
  } else if (sql_rc == SQL_SUCCESS_WITH_INFO) {
    UPLL_LOG_TRACE(" Transaction Isolation level set to default");
  }
  DalErrorHandler::ProcessOdbcErrors(SQL_HANDLE_DBC,
                                     conn_handle,
                                     sql_rc, &dal_rc);
  // TODO(sankar): Do we need to return error from here?
  if (dal_rc != kDalRcSuccess) {
    UPLL_LOG_TRACE(" Failed to set Transaction Isolation level."
                 " Default value set");
  }
  return kDalRcSuccess;
}  // DalOdbcMgr::SetConnAttributes

// Set Statement Attributes
void
DalOdbcMgr::SetStmtAttributes(const SQLHANDLE stmt_handle) const {
  SQLRETURN     sql_rc;
  DalResultCode dal_rc;
  uint32_t query_timeout;

  // SQL_ATTR_NOSCAN - SQL_NOSCAN_ON (default: SQL_NOSCAN_OFF)
  //   Scan for escape sequence to edit query stmts acc to datasource
  sql_rc = SQLSetStmtAttr(stmt_handle,
                          SQL_ATTR_NOSCAN,
                          (SQLPOINTER)SQL_NOSCAN_ON,
                          0);
  if (sql_rc == SQL_SUCCESS) {
    UPLL_LOG_TRACE("Noscan option set to on");
  } else if (sql_rc == SQL_SUCCESS_WITH_INFO) {
    UPLL_LOG_TRACE(" Noscan option set to default");
  }
  DalErrorHandler::ProcessOdbcErrors(SQL_HANDLE_STMT,
                                     stmt_handle,
                                     sql_rc, &dal_rc);
  if (dal_rc != kDalRcSuccess) {
    UPLL_LOG_TRACE(" Failed to Set NoScan On. Default value set");
  }

  // SQL_ATTR_QUERY_TIMEOUT - conf (default: 0 no timeout)
  // TODO(sankar): should come from conf file
  query_timeout = 0;
  sql_rc = SQLSetStmtAttr(stmt_handle,
                          SQL_ATTR_QUERY_TIMEOUT,
                          reinterpret_cast<SQLPOINTER>(query_timeout),
                          SQL_IS_UINTEGER);
  if (sql_rc == SQL_SUCCESS) {
    UPLL_LOG_TRACE("Query Timeout set - %d", query_timeout);
  } else if (sql_rc == SQL_SUCCESS_WITH_INFO) {
    UPLL_LOG_TRACE(" Query Timeout set to default");
  }
  DalErrorHandler::ProcessOdbcErrors(SQL_HANDLE_STMT,
                                     stmt_handle,
                                     sql_rc, &dal_rc);
  if (dal_rc != kDalRcSuccess) {
    UPLL_LOG_TRACE(" Failed to set Query Timeout. Default value set");
  }

  // SQL_ATTR_ROW_ARRAY_SIZE - 1 (default)
  //   Number of records to be fetched using SQLFetch

  // SQL_ATTR_PARAM_BIND_TYPE - SQL_PARAM_BIND_BY_COLUMN (default)
  //   Binding column by column

  // SQL_ATTR_ROW_BIND_TYPE - SQL_BIND_BY_COLUMN (default)
  //   Useful only for block cursors where SQLFetch returns more than 1 row

  // SQL_ATTR_MAX_ROWS - Max Number of rows in Select result
  //                     (default: 0 all rows)
  //   Max number of rows can be returned from SELECT query
  return;
}  // DalOdbcMgr::SetStmtAttributes

// Set Cursor Attributes
void
DalOdbcMgr::SetCursorAttributes(const SQLHANDLE stmt_handle,
                                const uint32_t max_count) const {
  SQLRETURN     sql_rc;
  DalResultCode dal_rc;

  // SQL_ATTR_CONCURRENCY - SQL_CONCUR_READ_ONLY (default)
  //   Concurrent behavior - read-only/lock etc

  // SQL_ATTR_CURSOR_SCROLLABLE - SQL_NONSCROLLABLE (default)
  //   Cursor can move up/down or not

  // SQL_ATTR_CURSOR_SENSITIVITY - SQL_INSENSITIVE (default: SQL_UNSPECIFIED)
  //   Cursor sensitivity - Sensitivity towards data modified by other cursor

  // SQL_ATTR_CURSOR_TYPE - SQL_CURSOR_STATIC (default: SQL_CURSOR_FORWARD_ONLY)
  //   Type of cursor - static/forward-only/dynamic etc

  // SQL_ATTR_MAX_ROWS - Max Number of rows in Select result
  //                     (default: 0 all rows)
  //   Max number of rows can be returned from SELECT query
  sql_rc = SQLSetStmtAttr(stmt_handle,
                          SQL_ATTR_MAX_ROWS,
                          reinterpret_cast<SQLPOINTER>(max_count),
                          SQL_IS_UINTEGER);
  if (sql_rc == SQL_SUCCESS) {
    UPLL_LOG_TRACE("Max Row Size set to %d", max_count);
  } else if (sql_rc == SQL_SUCCESS_WITH_INFO) {
    UPLL_LOG_TRACE(" Default Max Row Size set");
  }
  DalErrorHandler::ProcessOdbcErrors(SQL_HANDLE_STMT,
                                     stmt_handle,
                                     sql_rc, &dal_rc);
  if (dal_rc != kDalRcSuccess) {
    UPLL_LOG_TRACE(" Failed to set Max Row Size set. Default value set");
  }
  return;
}  // DalOdbcMgr::SetCursorAttributes

// Binds dal user data to the Query statement
DalResultCode
DalOdbcMgr::BindToQuery(const SQLHANDLE *stmt_handle,
                        const DalBindInfo *bind_info) const {
  DalResultCode dal_rc;
  uint16_t in_param_idx = 1;
  uint16_t out_param_idx = 1;

  // Input Validation
  // PFC_ASSERT(stmt_handle);
  // PFC_ASSERT(*stmt_handle);
  if (stmt_handle == NULL || *stmt_handle == NULL) {
    UPLL_LOG_DEBUG("Invalid Statement Handle");
    return kDalRcGeneralError;
  }

  // PFC_ASSERT(bind_info);
  if (bind_info == NULL) {
    UPLL_LOG_DEBUG("NULL Bind Info");
    return kDalRcGeneralError;
  }

  // Printing Bind List for debugging
  UPLL_LOG_VERBOSE("%s",
      ((const_cast<DalBindInfo *>(bind_info))->BindListToStr()).c_str());
  UPLL_LOG_TRACE("%s",
      ((const_cast<DalBindInfo *>(bind_info))->BindListInputToStr()).c_str());

  // Dont change the order of binding(input, output and match)
  // Parameter Indexing is dependant on this ordering.
  // Binding Input Parameters
  if (bind_info->get_input_bind_count() != 0) {
    dal_rc = BindInputToQuery(stmt_handle, bind_info, &in_param_idx);
    if (dal_rc != kDalRcSuccess) {
      UPLL_LOG_DEBUG("Err - %d. Failure while Binding Input parameters",
                     dal_rc);
      return dal_rc;
    }
  }

  // Binding Output Parameters
  if (bind_info->get_output_bind_count() != 0) {
    dal_rc = BindOutputToQuery(stmt_handle, bind_info, &out_param_idx);
    if (dal_rc != kDalRcSuccess) {
      UPLL_LOG_DEBUG("Err - %d. Failure while Binding Output parameters",
                     dal_rc);
      return dal_rc;
    }
  }

  // Binding Match Parameters
  if (bind_info->get_match_bind_count() != 0) {
    dal_rc = BindMatchToQuery(stmt_handle, bind_info, &in_param_idx);
    if (dal_rc != kDalRcSuccess) {
      UPLL_LOG_DEBUG("Err - %d. Failure while Binding Match parameters",
                     dal_rc);
      return dal_rc;
    }
  }

  UPLL_LOG_TRACE("Parameters bound sucessfully to Query");
  return kDalRcSuccess;
}  // DalBindInfo::BindToQuery

// Wrapper for the APIs
// Binds Input Parameters to the Query statement
DalResultCode
DalOdbcMgr::BindInputToQuery(const SQLHANDLE *stmt_handle,
                             const DalBindInfo *bind_info,
                             uint16_t *param_idx) const {
  DalTableIndex table_index;
  DalColumnIndex column_index;
  DalIoType io_type;
  DalBindList bind_list;
  DalBindList::iterator iter;
  DalBindColumnInfo *col_info = NULL;
  DalResultCode dal_rc;
  SQLRETURN sql_rc;

  // Input Validation
  // PFC_ASSERT(stmt_handle);
  // PFC_ASSERT(*stmt_handle);
  if (stmt_handle == NULL || *stmt_handle == NULL) {
    UPLL_LOG_DEBUG("Invalid Statement Handle");
    return kDalRcGeneralError;
  }

  // PFC_ASSERT(bind_info);
  if (bind_info == NULL) {
    UPLL_LOG_DEBUG("Invalid Binding Informartion");
    return kDalRcGeneralError;
  }

  table_index = bind_info->get_table_index();
  bind_list = bind_info->get_bind_list();
  for (iter = bind_list.begin();
       iter != bind_list.end();
       ++iter)   {
    col_info = reinterpret_cast<DalBindColumnInfo *>(*iter);

  // PFC_ASSERT(col_info);
    if (col_info == NULL) {
      UPLL_LOG_DEBUG("Invalid column Info");
      return kDalRcGeneralError;
    }

    column_index = col_info->get_column_index();
    io_type = col_info->get_io_type();
    if (io_type == kDalIoInputOnly ||
        io_type == kDalIoInputAndMatch) {
      sql_rc = SQLBindParameter(*stmt_handle, *param_idx, SQL_PARAM_INPUT,
                 schema::ColumnDalDataTypeId(table_index, column_index),
                 schema::ColumnDbDataTypeId(table_index, column_index),
                 *(col_info->get_buff_len_ptr()), 0,
                 col_info->get_db_in_out_addr(),
                 *(col_info->get_buff_len_ptr()),
                 col_info->get_buff_len_ptr());

      DalErrorHandler::ProcessOdbcErrors(SQL_HANDLE_STMT,
                                         *stmt_handle,
                                         sql_rc, &dal_rc);
      if (dal_rc != kDalRcSuccess) {
        UPLL_LOG_DEBUG("Err - %d. Error while binding Column(%s) for Input",
                       dal_rc, schema::ColumnName(table_index, column_index));
        return dal_rc;
      }
      (*param_idx)++;
    }
  }  // for iter
  UPLL_LOG_VERBOSE("Input Parameters bound sucessfully to Query");
  return dal_rc;
}  // DalBindInfo::BindInputToQuery

// Binds Match parameters to the Query statement
DalResultCode
DalOdbcMgr::BindOutputToQuery(const SQLHANDLE *stmt_handle,
                              const DalBindInfo *bind_info,
                              uint16_t *param_idx) const {
  DalTableIndex table_index;
  DalColumnIndex column_index;
  DalIoType io_type;
  DalBindList bind_list;
  DalBindList::iterator iter;
  DalBindColumnInfo *col_info = NULL;
  DalResultCode dal_rc;
  SQLRETURN sql_rc;

  // Input Validation
  // PFC_ASSERT(stmt_handle);
  // PFC_ASSERT(*stmt_handle);
  if (stmt_handle == NULL || *stmt_handle == NULL) {
    UPLL_LOG_DEBUG("Invalid Statement Handle");
    return kDalRcGeneralError;
  }

  // PFC_ASSERT(bind_info);
  if (bind_info == NULL) {
    UPLL_LOG_DEBUG("Invalid Binding Informartion");
    return kDalRcGeneralError;
  }
  table_index = bind_info->get_table_index();
  bind_list = bind_info->get_bind_list();
  for (iter = bind_list.begin();
       iter != bind_list.end();
       ++iter)   {
    col_info = reinterpret_cast<DalBindColumnInfo *>(*iter);

  // PFC_ASSERT(col_info);
    if (col_info == NULL) {
      UPLL_LOG_DEBUG("Invalid column Info");
      return kDalRcGeneralError;
    }

    column_index = col_info->get_column_index();
    io_type = col_info->get_io_type();
    if (io_type == kDalIoOutputOnly ||
        io_type == kDalIoOutputAndMatch) {
      sql_rc = SQLBindCol(*stmt_handle, *param_idx,
                schema::ColumnDalDataTypeId(table_index, column_index),
                col_info->get_db_in_out_addr(),
                schema::ColumnDbArraySize(table_index, column_index),
                col_info->get_buff_len_ptr());

      DalErrorHandler::ProcessOdbcErrors(SQL_HANDLE_STMT,
                                         *stmt_handle,
                                         sql_rc, &dal_rc);
      if (dal_rc != kDalRcSuccess) {
        UPLL_LOG_DEBUG("Err - %d. Error while binding Column(%s) for Output",
                       dal_rc, schema::ColumnName(table_index, column_index));
        return dal_rc;
      }
      (*param_idx)++;
    }
  }  // for iter
  UPLL_LOG_VERBOSE("Output Parameters bound sucessfully to Query");
  return dal_rc;
}  // DalBindInfo::BindOutputToQuery

// Binds Output parameters to the Query statement
DalResultCode
DalOdbcMgr::BindMatchToQuery(const SQLHANDLE *stmt_handle,
                             const DalBindInfo *bind_info,
                             uint16_t *param_idx) const {
  DalTableIndex table_index;
  DalColumnIndex column_index;
  DalIoType io_type;
  DalBindList bind_list;
  DalBindList::iterator iter;
  DalBindColumnInfo *col_info = NULL;
  DalResultCode dal_rc;
  SQLRETURN sql_rc;

  // Input Validation
  // PFC_ASSERT(stmt_handle);
  // PFC_ASSERT(*stmt_handle);
  if (stmt_handle == NULL || *stmt_handle == NULL) {
    UPLL_LOG_DEBUG("Invalid Statement Handle");
    return kDalRcGeneralError;
  }

  // PFC_ASSERT(bind_info);
  if (bind_info == NULL) {
    UPLL_LOG_DEBUG("Invalid Binding Informartion");
    return kDalRcGeneralError;
  }
  table_index = bind_info->get_table_index();
  bind_list = bind_info->get_bind_list();
  for (iter = bind_list.begin();
       iter != bind_list.end();
       ++iter)   {
    col_info = reinterpret_cast<DalBindColumnInfo *>(*iter);

  // PFC_ASSERT(col_info);
    if (col_info == NULL) {
      UPLL_LOG_DEBUG("Invalid column Info");
      return kDalRcGeneralError;
    }

    column_index = col_info->get_column_index();
    io_type = col_info->get_io_type();
    if (io_type == kDalIoMatchOnly ||
        io_type == kDalIoInputAndMatch ||
        io_type == kDalIoOutputAndMatch) {
      sql_rc = SQLBindParameter(*stmt_handle, *param_idx, SQL_PARAM_INPUT,
                 schema::ColumnDalDataTypeId(table_index, column_index),
                 schema::ColumnDbDataTypeId(table_index, column_index),
                 *(col_info->get_buff_len_ptr()), 0,
                 col_info->get_db_match_addr(),
                 *(col_info->get_buff_len_ptr()),
                 col_info->get_buff_len_ptr());

      DalErrorHandler::ProcessOdbcErrors(SQL_HANDLE_STMT,
                                         *stmt_handle,
                                         sql_rc, &dal_rc);
      if (dal_rc != kDalRcSuccess) {
        UPLL_LOG_DEBUG("Err - %d. Error while binding Column(%s) for Match",
                       dal_rc, schema::ColumnName(table_index, column_index));
        return dal_rc;
      }
      (*param_idx)++;
    }
  }  // for iter
  UPLL_LOG_VERBOSE("Match Parameters bound sucessfully to Query");
  return dal_rc;
}  // DalBindInfo::BindMatchToQuery

// Allocates Stmt Handle, Bind Parameters, Build and Execute the Query
DalResultCode
DalOdbcMgr::ExecuteQuery(SQLHANDLE *dal_stmt_handle,
                         const std::string *query_stmt,
                         const DalBindInfo *bind_info,
                         const uint32_t max_count) const {
  SQLRETURN     sql_rc;
  DalResultCode dal_rc;

  // Input Validation
  // PFC_ASSERT(stmt_handle);
  // PFC_ASSERT(*stmt_handle);
  // PFC_ASSERT(query_stmt)
  if (dal_stmt_handle == SQL_NULL_HANDLE) {
    UPLL_LOG_DEBUG("NULL Statement Handle Reference");
    return kDalRcGeneralError;
  }

  if (query_stmt == NULL) {
    UPLL_LOG_DEBUG("NULL Query Stmt");
    return kDalRcGeneralError;
  }

  // Allocate Statement Handle
  sql_rc = SQLAllocHandle(SQL_HANDLE_STMT,
                          dal_conn_handle_,
                          dal_stmt_handle);
  DalErrorHandler::ProcessOdbcErrors(SQL_HANDLE_STMT,
                                     *dal_stmt_handle,
                                     sql_rc, &dal_rc);
  if (dal_rc != kDalRcSuccess) {
    UPLL_LOG_DEBUG("Err - %d. Failed to Allocate Statement handle",
                   dal_rc);
    return dal_rc;
  }
  if (*dal_stmt_handle == SQL_NULL_HANDLE) {
    UPLL_LOG_DEBUG("Err - %d. Failed to Allocate Statement handle",
                   dal_rc);
    return kDalRcGeneralError;
  } else {
    UPLL_LOG_TRACE("Statement Handle(%p) Created", dal_stmt_handle);
  }

  // Setting Statement Attributes
  SetStmtAttributes(*dal_stmt_handle);

  // Setting Cursor Attributes
  SetCursorAttributes(*dal_stmt_handle, max_count);

  // Bind attributes to query statement only if available
  if (bind_info != NULL) {
    dal_rc = BindToQuery(dal_stmt_handle, bind_info);
    if (dal_rc != kDalRcSuccess) {
      UPLL_LOG_DEBUG("Err - %d. Failed to Bind parameters to Query",
                     dal_rc);
      FreeHandle(SQL_HANDLE_STMT, *dal_stmt_handle);
      return dal_rc;
    }
    UPLL_LOG_VERBOSE("Success Binding parameters to Query");
  }

  // Executing the Query Statement
  sql_rc = SQLExecDirect(*dal_stmt_handle,
                         (unsigned char*)(query_stmt->c_str()),
                         SQL_NTS);
  DalErrorHandler::ProcessOdbcErrors(SQL_HANDLE_STMT,
                                     *dal_stmt_handle,
                                     sql_rc, &dal_rc);
  if (dal_rc != kDalRcSuccess) {
    UPLL_LOG_DEBUG("Err - %d. Failed to Execute Query %s",
                   dal_rc, query_stmt->c_str());
    FreeHandle(SQL_HANDLE_STMT, *dal_stmt_handle);
    return dal_rc;
  }

  UPLL_LOG_VERBOSE("Query Successfully Executed %s",
                query_stmt->c_str());
  return kDalRcSuccess;
}   // DalOdbcMgr::ExecuteQuery

// Frees the handle passed
inline DalResultCode
DalOdbcMgr::FreeHandle(const SQLSMALLINT handle_type,
                       SQLHANDLE handle) const {
  SQLRETURN sql_rc = SQL_SUCCESS;
  DalResultCode dal_rc = kDalRcSuccess;

  if (handle == SQL_NULL_HANDLE) {
    UPLL_LOG_TRACE("Null Handle");
    return kDalRcSuccess;
  }
  sql_rc = SQLFreeHandle(handle_type, handle);
  if (sql_rc != SQL_SUCCESS && sql_rc != SQL_SUCCESS_WITH_INFO) {
    DalErrorHandler::ProcessOdbcErrors(handle_type, handle,
                                       sql_rc, &dal_rc);
    if (dal_rc != kDalRcSuccess) {
      UPLL_LOG_DEBUG("Err - %d. Failed to Free Handle %p",
                     dal_rc, handle);
      return dal_rc;
    }
  }
  handle = SQL_NULL_HANDLE;
  return kDalRcSuccess;
}

// Derives the Database Connection String from Conf file - dal.conf
std::string
DalOdbcMgr::GetConnString(const DalConnType conn_type) const {
  std::string dal_conn_str;
  std::string dal_cf_str = std::string(DAL_CONF_FILE);
  std::string conn_param_str;

  pfc::core::ConfHandle dal_cf_handle(dal_cf_str, &dal_cfdef);
  int32_t cf_err = dal_cf_handle.getError();

  if (cf_err != 0) {
    UPLL_LOG_DEBUG("Err - %d. Error while reading conf file = %s ",
                   cf_err, dal_cf_str.c_str());
    return "";
  }

  if (conn_type == kDalConnReadWrite) {
    conn_param_str = "rw_conn";
  } else {
    conn_param_str = "ro_conn";
  }
  pfc::core::ConfBlock dal_cfb(dal_cf_handle, "dal_db_params",
                               conn_param_str.c_str());

  // store the parsed values in structure
  dal_conn_str.clear();
  dal_conn_str += "DSN=";
  dal_conn_str += dal_cfb.getString("dsn_name", "PostgreSQL");
  dal_conn_str += ";";
  UPLL_LOG_TRACE("Connection String(%s) formed from conf file",
                 dal_conn_str.c_str());

  return dal_conn_str;
}  // DalOdbcMgr::GetConnString
}  // namespace dal
}  // namespace upll
}  // namespace unc
