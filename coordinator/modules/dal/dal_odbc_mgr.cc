/*
 * Copyright (c) 2012-2014 NEC Corporation
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

#define SET_DB_STATE_DISCONNECT(dal_rc, conn_state)       \
  if ((dal_rc) == kDalRcConnNotEstablished ||             \
      (dal_rc) == kDalRcConnNotAvailable ||               \
      (dal_rc) == kDalRcConnTimeOut ||                    \
      (dal_rc) == kDalRcQueryTimeOut) {                   \
    (conn_state) = kDalDbDisconnected;                    \
  }

namespace unc {
namespace upll {
namespace dal {

std::map<std::string, DalTableIndex> DalOdbcMgr::tbl_name_to_idx_map_;

// Constructor
DalOdbcMgr::DalOdbcMgr() {
  dal_env_handle_ = SQL_NULL_HANDLE;
  dal_conn_handle_ = SQL_NULL_HANDLE;
  conn_type_ = kDalConnReadOnly;
  conn_state_ = kDalDbDisconnected;
  write_count_ = 0;
}

/* Desctructor */
DalOdbcMgr::~DalOdbcMgr() {
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
    UPLL_LOG_INFO("Err - %d. Failed to enable connection Pooling", dal_rc);
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
    UPLL_LOG_INFO("Err - %d. Failed to allocate environment handle", dal_rc);
    return dal_rc;
  }

  // PFC_ASSERT(dal_env_handle_);
  if (dal_env_handle_ == SQL_NULL_HANDLE) {
    UPLL_LOG_INFO("NULL Environment handle");
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
    UPLL_LOG_INFO("Err - %d. Failed to register DB with ODBC V3", dal_rc);
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
    UPLL_LOG_INFO("Err - %d. Failed to allocate connection handle", dal_rc);
    FreeHandle(SQL_HANDLE_ENV, dal_env_handle_);
    return dal_rc;
  }

  // PFC_ASSERT(dal_conn_handle_);
  if (dal_conn_handle_ == SQL_NULL_HANDLE) {
    UPLL_LOG_INFO("NULL Connection handle");
    FreeHandle(SQL_HANDLE_ENV, dal_env_handle_);
    return kDalRcGeneralError;
  }

  UPLL_LOG_TRACE("Allocated Connection handle(%p)", dal_conn_handle_);
  return kDalRcSuccess;
}  // DalOdbcMgr::Init

bool DalOdbcMgr::FillTableName2IndexMap() {
  using namespace schema;
  for (uint16_t table_index = 0; (table_index < table::kDalNumTables);
       table_index++) {
     if (((int)strlen(table::table_schema[table_index].table_name))
         > table::cfg_tbl_dirty::max_cfg_tbl_name_len) {
       UPLL_LOG_INFO("DAL Schema is bad. Table: %s",
                      table::table_schema[table_index].table_name);
       return false;
     }
     tbl_name_to_idx_map_[table::table_schema[table_index].table_name] =
         table_index;
  }
  return true;
}

// Connects to the Datasource with the conf file data
DalResultCode
DalOdbcMgr::ConnectToDb(const DalConnType conn_type) const {
  SQLRETURN     sql_rc;
  DalResultCode dal_rc;
  conn_type_ = conn_type;

  if (dal_conn_handle_ == NULL) {
    UPLL_LOG_INFO("NULL Connection handle");
    return kDalRcGeneralError;
  }

  // Set Connection Attributes
  dal_rc = SetConnAttributes(dal_conn_handle_, conn_type);
  if (dal_rc != kDalRcSuccess) {
    UPLL_LOG_INFO("Err - %d. Some/all connection attributes not set", dal_rc);
    return dal_rc;
  }

  // Connecting to database
  std::string dal_conn_str;
  dal_conn_str.clear();
  dal_conn_str = GetConnString(conn_type);
  if (dal_conn_str.empty()) {
    UPLL_LOG_INFO("Error building connection string");
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
    UPLL_LOG_DEBUG("Err - %d. Failed to connect datatbase with conn string(%s)",
                   dal_rc, dal_conn_str.c_str());
    return dal_rc;
  }
  conn_state_ = kDalDbConnected;

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
    UPLL_LOG_INFO("NULL Connection handle");
    return kDalRcGeneralError;
  }

  sql_rc = SQLDisconnect(dal_conn_handle_);
  DalErrorHandler::ProcessOdbcErrors(SQL_HANDLE_DBC,
                                     dal_conn_handle_,
                                     sql_rc, &dal_rc);
  if (dal_rc != kDalRcSuccess) {
    UPLL_LOG_INFO("Err - %d. Failed to disconnect database for the connection "
                  "handle(%p)",  dal_rc, dal_conn_handle_);
    return dal_rc;
  }
  conn_state_ = kDalDbDisconnected;
  UPLL_LOG_TRACE("Successfully Disconnected from Database");
  return kDalRcSuccess;
}  // DalOdbcMgr::DisconnectFromDb


// Commits all the statements executed as part of the transaction
DalResultCode
DalOdbcMgr::CommitTransaction(void) const {
  SQLRETURN     sql_rc;
  DalResultCode dal_rc;

  if (dal_conn_handle_ == NULL) {
    UPLL_LOG_INFO("NULL Connection handle");
    return kDalRcGeneralError;
  }

  sql_rc = SQLEndTran(SQL_HANDLE_DBC, dal_conn_handle_, SQL_COMMIT);
  DalErrorHandler::ProcessOdbcErrors(SQL_HANDLE_DBC,
                                     dal_conn_handle_,
                                     sql_rc, &dal_rc);
  SET_DB_STATE_DISCONNECT(dal_rc, conn_state_);
  if (dal_rc != kDalRcSuccess) {
    UPLL_LOG_INFO("Err - %d. Failed to commit transaction for the "
                  "handle(%p)",  dal_rc, dal_conn_handle_);
    return dal_rc;
  }
  UPLL_LOG_TRACE("Successfully committed transaction for the "
                 "handle(%p)",  dal_conn_handle_);
  write_count_ = 0; // TODO(s): Is this good for configmgr ?
  return kDalRcSuccess;
}  // DalOdbcMgr::CommitTransaction


// Rollbacks all the statements executed as part of the transaction
DalResultCode
DalOdbcMgr::RollbackTransaction(void) const {
  SQLRETURN     sql_rc;
  DalResultCode dal_rc;

  // PFC_ASSERT(dal_conn_handle_);
  if (dal_conn_handle_ == NULL) {
    UPLL_LOG_INFO("NULL Connection handle");
    return kDalRcGeneralError;
  }

  sql_rc = SQLEndTran(SQL_HANDLE_DBC, dal_conn_handle_, SQL_ROLLBACK);
  DalErrorHandler::ProcessOdbcErrors(SQL_HANDLE_DBC,
                                     dal_conn_handle_,
                                     sql_rc, &dal_rc);
  SET_DB_STATE_DISCONNECT(dal_rc, conn_state_);
  if (dal_rc != kDalRcSuccess) {
    UPLL_LOG_INFO("Err - %d. Failed to rollback transaction for the "
                  "handle(%p)",  dal_rc, dal_conn_handle_);
    return dal_rc;
  }
  UPLL_LOG_TRACE("Successfully rollback transaction for the "
                 "handle(%p)",  dal_conn_handle_);
  write_count_ = 0; // TODO(s): Is this good for configmgr ?
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
    UPLL_LOG_INFO("Invalid config type - %d", cfg_type);
    return kDalRcGeneralError;
  }

  if (table_index >= schema::table::kDalNumTables) {
    UPLL_LOG_INFO("Invalid table index - %d", table_index);
    return kDalRcGeneralError;
  }

  // PFC_ASSERT(bind_info)
  if (bind_info == NULL) {
    UPLL_LOG_INFO("NULL bind info for table(%s)",
                  schema::TableName(table_index));
    return kDalRcGeneralError;
  }

  if (table_index != bind_info->get_table_index()) {
    UPLL_LOG_INFO("Table index mismatch with bind info "
                  "Query - %s; Bind - %s",
                  schema::TableName(table_index),
                  schema::TableName(bind_info->get_table_index()));
    return kDalRcGeneralError;
  }

  // Build Query Statement
  if (qbldr.get_sql_statement(kDalGetSingleRecQT, bind_info, query_stmt,
                        table_index, cfg_type) != true) {
    UPLL_LOG_INFO("Failed building query stmt");
    return kDalRcGeneralError;
  } else {
    UPLL_LOG_TRACE("Query stmt - %s", query_stmt.c_str());
  }

  // Allocate Stmt Handle, Bind and Execute the Query Statement
  dal_rc = ExecuteQuery(&dal_stmt_handle,
                        &query_stmt,
                        bind_info);

  if (dal_rc != kDalRcSuccess) {
    UPLL_LOG_INFO("Err - %d. Failed executing query stmt - %s",
                   dal_rc, query_stmt.c_str());
    FreeHandle(SQL_HANDLE_STMT, dal_stmt_handle);
    return dal_rc;
  }
  UPLL_LOG_TRACE("Completed executing query stmt - %s", query_stmt.c_str());

  // Fetching results from the resultset
  sql_rc = SQLFetch(dal_stmt_handle);
  DalErrorHandler::ProcessOdbcErrors(SQL_HANDLE_STMT,
                                     dal_stmt_handle,
                                     sql_rc, &dal_rc);
  SET_DB_STATE_DISCONNECT(dal_rc, conn_state_);
  if (dal_rc != kDalRcSuccess) {
    if (dal_rc != kDalRcRecordNotFound) {
      UPLL_LOG_INFO("%d - Failed to Fetch result, query stmt - %s",
                    dal_rc, query_stmt.c_str());
    } else {
      UPLL_LOG_TRACE("%d - Failed to Fetch result, query stmt - %s",
                     dal_rc, query_stmt.c_str());
    }
    FreeHandle(SQL_HANDLE_STMT, dal_stmt_handle);
    return dal_rc;
  }
  UPLL_LOG_TRACE("Result fetched from DB");

  if (const_cast<DalBindInfo*>(bind_info)->CopyResultToApp() != true) {
    UPLL_LOG_INFO("Failed to Copy result to the DAL User Buffer");
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
    UPLL_LOG_INFO("Invalid config type - %d", cfg_type);
    return kDalRcGeneralError;
  }

  if (table_index >= schema::table::kDalNumTables) {
    UPLL_LOG_INFO("Invalid table index - %d", table_index);
    return kDalRcGeneralError;
  }

  // PFC_ASSERT(bind_info)
  if (bind_info == NULL) {
    UPLL_LOG_INFO("NULL Bind Info for Table(%s)",
                   schema::TableName(table_index));
    return kDalRcGeneralError;
  }

  if (table_index != bind_info->get_table_index()) {
    UPLL_LOG_INFO("Table index mismatch with bind info "
                   "Query - %s; Bind - %s",
                   schema::TableName(table_index),
                   schema::TableName(bind_info->get_table_index()));
    return kDalRcGeneralError;
  }

  // Build Query Statement
  if (qbldr.get_sql_statement(kDalGetMultiRecQT, bind_info, query_stmt,
                        table_index, cfg_type) != true) {
    UPLL_LOG_INFO("Failed building query stmt");
    return kDalRcGeneralError;
  } else {
    UPLL_LOG_TRACE("Query stmt - %s", query_stmt.c_str());
  }

  // Allocate Stmt Handle, Bind and Execute the Query Statement
  dal_rc = ExecuteQuery(&dal_stmt_handle,
                        &query_stmt,
                        bind_info,
                        max_record_count);

  if (dal_rc != kDalRcSuccess) {
    UPLL_LOG_DEBUG("Err - %d. Failed executing query stmt - %s",
                   dal_rc, query_stmt.c_str());
    FreeHandle(SQL_HANDLE_STMT, dal_stmt_handle);
    return dal_rc;
  }
  UPLL_LOG_TRACE("Completed executing query stmt - %s",
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
  SET_DB_STATE_DISCONNECT(dal_rc, conn_state_);

  if (dal_rc != kDalRcSuccess) {
    if (dal_rc == kDalRcRecordNoMore) {
      UPLL_LOG_TRACE("Err - %d. No More Records in Cursor handle (%p)",
                     dal_rc, cursor);
    } else {
      UPLL_LOG_INFO("Err - %d. Error Fetching Next Record from Cursor"
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
  SET_DB_STATE_DISCONNECT(dal_rc, conn_state_);

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
  SQLHANDLE     dal_stmt_handle = SQL_NULL_HANDLE;
  SQLRETURN     sql_rc;
  DalResultCode dal_rc;
  DalQueryBuilder  qbldr;
  std::string query_stmt;

  // Validating Inputs
  if (cfg_type == UPLL_DT_INVALID) {
    UPLL_LOG_DEBUG("Invalid config type - %d", cfg_type);
    return kDalRcGeneralError;
  }

  if (table_index >= schema::table::kDalNumTables) {
    UPLL_LOG_DEBUG("Invalid table index - %d", table_index);
    return kDalRcGeneralError;
  }

  // BindInput - NA; BindOutput - NA; BindMatch - Optional
  // No need to validate bind_info
  if (bind_info != NULL) {
    if (table_index != bind_info->get_table_index()) {
      UPLL_LOG_DEBUG("Table index mismatch with bind info "
                     "Query - %s; Bind - %s",
                     schema::TableName(table_index),
                     schema::TableName(bind_info->get_table_index()));
      return kDalRcGeneralError;
    }
  }

  // Build Query Statement
  if (qbldr.get_sql_statement(kDalRecExistsQT, bind_info, query_stmt,
                        table_index, cfg_type) != true) {
    UPLL_LOG_DEBUG("Failed building query stmt");
    return kDalRcGeneralError;
  } else {
    UPLL_LOG_TRACE("Query stmt - %s", query_stmt.c_str());
  }

  // Allocate Stmt Handle, Bind and Execute the Query Statement
  dal_rc = ExecuteQuery(&dal_stmt_handle,
                        &query_stmt,
                        bind_info);
  if (dal_rc != kDalRcSuccess) {
    UPLL_LOG_DEBUG("Err - %d. XXXXX - Failed executing query stmt - %s",
                   dal_rc, query_stmt.c_str());
    FreeHandle(SQL_HANDLE_STMT, dal_stmt_handle);
    return dal_rc;
  }
  UPLL_LOG_TRACE("Completed executing query stmt - %s",
                query_stmt.c_str());

  // Fetching results from the resultset
  SQLLEN row_count = 0;
  sql_rc = SQLRowCount(dal_stmt_handle, &row_count);
  DalErrorHandler::ProcessOdbcErrors(SQL_HANDLE_STMT,
                                     dal_stmt_handle,
                                     sql_rc, &dal_rc);
  SET_DB_STATE_DISCONNECT(dal_rc, conn_state_);

  if (dal_rc != kDalRcSuccess) {
    UPLL_LOG_DEBUG("Err - %d. XXXXX - Failed to fetch result from DB", dal_rc);
    FreeHandle(SQL_HANDLE_STMT, dal_stmt_handle);
    return dal_rc;
  }

  *existence = (row_count > 0) ? true : false;
  UPLL_LOG_TRACE("Completed executing RecordExists and result of"
                " existence is %d",  *existence);
  FreeHandle(SQL_HANDLE_STMT, dal_stmt_handle);
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
    UPLL_LOG_DEBUG("Invalid config type - %d", cfg_type);
    return kDalRcGeneralError;
  }

  if (table_index >= schema::table::kDalNumTables) {
    UPLL_LOG_DEBUG("Invalid table index - %d", table_index);
    return kDalRcGeneralError;
  }

  // PFC_ASSERT(bind_info)
  if (bind_info == NULL) {
    UPLL_LOG_DEBUG("NULL Bind Info for Table(%s)",
                   schema::TableName(table_index));
    return kDalRcGeneralError;
  }

  if (table_index != bind_info->get_table_index()) {
    UPLL_LOG_DEBUG("Table index mismatch with bind info "
                   "Query - %s; Bind - %s",
                   schema::TableName(table_index),
                   schema::TableName(bind_info->get_table_index()));
    return kDalRcGeneralError;
  }

  // Build Query Statement
  if (qbldr.get_sql_statement(kDalGetSibBegQT, bind_info, query_stmt,
                        table_index, cfg_type) != true) {
    UPLL_LOG_DEBUG("Failed building query stmt");
    return kDalRcGeneralError;
  } else {
    UPLL_LOG_TRACE("Query stmt - %s", query_stmt.c_str());
  }

  // Allocate Stmt Handle, Bind and Execute the Query Statement
  dal_rc = ExecuteQuery(&dal_stmt_handle,
                        &query_stmt,
                        bind_info,
                        max_record_count);
  if (dal_rc != kDalRcSuccess) {
    UPLL_LOG_DEBUG("Err - %d. Failed executing query stmt - %s",
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
    UPLL_LOG_DEBUG("Invalid config type - %d", cfg_type);
    return kDalRcGeneralError;
  }

  if (table_index >= schema::table::kDalNumTables) {
    UPLL_LOG_DEBUG("Invalid table index - %d", table_index);
    return kDalRcGeneralError;
  }

  // PFC_ASSERT(bind_info)
  if (bind_info == NULL) {
    UPLL_LOG_DEBUG("NULL Bind Info for Table(%s)",
                   schema::TableName(table_index));
    return kDalRcGeneralError;
  }

  if (table_index != bind_info->get_table_index()) {
    UPLL_LOG_DEBUG("Table index mismatch with bind info "
                   "Query - %s; Bind - %s",
                   schema::TableName(table_index),
                   schema::TableName(bind_info->get_table_index()));
    return kDalRcGeneralError;
  }

  // Build Query Statement
  if (qbldr.get_sql_statement(kDalGetSibRecQT, bind_info, query_stmt,
                        table_index, cfg_type) != true) {
    UPLL_LOG_DEBUG("Failed building query stmt");
    return kDalRcGeneralError;
  } else {
    UPLL_LOG_TRACE("Query stmt - %s", query_stmt.c_str());
  }

  // Allocate Stmt Handle, Bind and Execute the Query Statement
  dal_rc = ExecuteQuery(&dal_stmt_handle,
                        &query_stmt,
                        bind_info,
                        max_record_count);
  if (dal_rc != kDalRcSuccess) {
    UPLL_LOG_DEBUG("Err - %d. Failed executing query stmt - %s",
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
    UPLL_LOG_DEBUG("Invalid config type - %d", cfg_type);
    return kDalRcGeneralError;
  }

  if (table_index >= schema::table::kDalNumTables) {
    UPLL_LOG_DEBUG("Invalid table index - %d", table_index);
    return kDalRcGeneralError;
  }

  // PFC_ASSERT(bind_info)
  if (bind_info == NULL) {
    UPLL_LOG_DEBUG("NULL Bind Info for Table(%s)",
                   schema::TableName(table_index));
    return kDalRcGeneralError;
  }

  if (table_index != bind_info->get_table_index()) {
    UPLL_LOG_DEBUG("Table index mismatch with bind info "
                   "Query - %s; Bind - %s",
                   schema::TableName(table_index),
                   schema::TableName(bind_info->get_table_index()));
    return kDalRcGeneralError;
  }

  // Build Query Statement
  if (qbldr.get_sql_statement(kDalGetSibCountQT, bind_info, query_stmt,
                        table_index, cfg_type) != true) {
    UPLL_LOG_DEBUG("Failed building query stmt");
    return kDalRcGeneralError;
  } else {
    UPLL_LOG_TRACE("Query stmt - %s", query_stmt.c_str());
  }

  // Allocate Stmt Handle, Bind and Execute the Query Statement
  dal_rc = ExecuteQuery(&dal_stmt_handle,
                        &query_stmt,
                        bind_info);
  if (dal_rc != kDalRcSuccess) {
    UPLL_LOG_DEBUG("Err - %d. Failed executing query stmt - %s",
                   dal_rc, query_stmt.c_str());
    FreeHandle(SQL_HANDLE_STMT, dal_stmt_handle);
    return dal_rc;
  }
  UPLL_LOG_TRACE("Completed executing query stmt - %s",
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
    UPLL_LOG_INFO("Err - %d. Failed to fetch result from DB, query stmt - %s",
                  dal_rc, query_stmt.c_str());
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
    UPLL_LOG_DEBUG("Invalid config type - %d", cfg_type);
    return kDalRcGeneralError;
  }

  if (table_index >= schema::table::kDalNumTables) {
    UPLL_LOG_DEBUG("Invalid table index - %d", table_index);
    return kDalRcGeneralError;
  }

  // BindInput - NA; BindOutput - NA; BindMatch - Optional
  // No need to validate bind_info
  if (bind_info != NULL) {
    if (table_index != bind_info->get_table_index()) {
      UPLL_LOG_DEBUG("Table index mismatch with bind info "
                     "Query - %s; Bind - %s",
                     schema::TableName(table_index),
                     schema::TableName(bind_info->get_table_index()));
      return kDalRcGeneralError;
    }
  }

  // Build Query Statement
  if (qbldr.get_sql_statement(kDalGetRecCountQT, bind_info, query_stmt,
                        table_index, cfg_type) != true) {
    UPLL_LOG_DEBUG("Failed building query stmt");
    return kDalRcGeneralError;
  } else {
    UPLL_LOG_TRACE("Query stmt - %s", query_stmt.c_str());
  }

  // Allocate Stmt Handle, Bind and Execute the Query Statement
  dal_rc = ExecuteQuery(&dal_stmt_handle,
                        &query_stmt,
                        bind_info);
  if (dal_rc != kDalRcSuccess) {
    UPLL_LOG_DEBUG("Err - %d. Failed executing query stmt - %s",
                   dal_rc, query_stmt.c_str());
    FreeHandle(SQL_HANDLE_STMT, dal_stmt_handle);
    return dal_rc;
  }
  UPLL_LOG_TRACE("Completed executing query stmt - %s",
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
  SET_DB_STATE_DISCONNECT(dal_rc, conn_state_);
  if (dal_rc != kDalRcSuccess) {
    UPLL_LOG_INFO("Err - %d. Failed to fetch result from DB, query stmt - %s",
                  dal_rc, query_stmt.c_str());
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
                          const DalBindInfo *bind_info,
                          const bool truncate) const {
  SQLHANDLE     dal_stmt_handle = SQL_NULL_HANDLE;
  DalResultCode dal_rc;
  DalQueryBuilder  qbldr;
  std::string query_stmt;

  // Validating Inputs
  if (cfg_type == UPLL_DT_INVALID) {
    UPLL_LOG_DEBUG("Invalid config type - %d", cfg_type);
    return kDalRcGeneralError;
  }

  if (table_index >= schema::table::kDalNumTables) {
    UPLL_LOG_DEBUG("Invalid table index - %d", table_index);
    return kDalRcGeneralError;
  }

  // BindInput - NA; BindOutput - NA; BindMatch - Optional
  // No need to validate bind_info
  if (bind_info != NULL) {
    if (table_index != bind_info->get_table_index()) {
      UPLL_LOG_DEBUG("Table index mismatch with bind info "
                     "Query - %s; Bind - %s",
                     schema::TableName(table_index),
                     schema::TableName(bind_info->get_table_index()));
      return kDalRcGeneralError;
    }
  }

  DalApiNum query_template;

  if (truncate == true) {
    query_template = kDalTruncTableQT;
    // Note: TRUNCATE query does has WHERE clause, so bind_info is not used.
  } else {
    query_template = kDalDelRecQT;
  }

  // Build Query Statement
  if (qbldr.get_sql_statement(query_template, bind_info, query_stmt,
                              table_index, cfg_type) != true) {
    UPLL_LOG_ERROR("Failed building query stmt");
    return kDalRcGeneralError;
  } else {
    UPLL_LOG_TRACE("Query stmt - %s", query_stmt.c_str());
  }

  // Allocate Stmt Handle, Bind and Execute the Query Statement
  dal_rc = ExecuteQuery(&dal_stmt_handle,
                        &query_stmt,
                        bind_info);

  if (dal_rc != kDalRcSuccess) {
    UPLL_LOG_DEBUG("Err - %d. Failed executing query stmt - %s",
                   dal_rc, query_stmt.c_str());
    FreeHandle(SQL_HANDLE_STMT, dal_stmt_handle);
    if (dal_rc == kDalRcParentNotFound) {
      UPLL_LOG_DEBUG("Foreign Key Violation error. Returning kDalRcGeneralError");
      dal_rc = kDalRcGeneralError;
    }
    return dal_rc;
  }
  UPLL_LOG_TRACE("Completed executing query stmt - %s",
                query_stmt.c_str());
  FreeHandle(SQL_HANDLE_STMT, dal_stmt_handle);

  // Storing dirty table list for skip unmodified tables during commit
  if (cfg_type == UPLL_DT_CANDIDATE) {
    if ((dal_rc = SetTableDirty(cfg_type, table_index, UNC_OP_DELETE))
        != kDalRcSuccess) {
      UPLL_LOG_ERROR("SetTableDirty failed with rc=%d", dal_rc);
      return kDalRcGeneralError;
    }
  }
  write_count_++;
  return kDalRcSuccess;
}   // DalOdbcMgr::DeleteRecords

DalResultCode
DalOdbcMgr::CheckParentInstance(const UpllCfgType cfg_type,
                                const DalTableIndex table_index,
                                const DalBindInfo *bind_info) const {
  UPLL_FUNC_TRACE;
  DalResultCode dal_rc;

  if (bind_info->get_input_bind_count() == 0) {
    UPLL_LOG_DEBUG("No Input bind data for ParentCheck");
    return kDalRcGeneralError;
  }

  // Create Bind Info for Instance Check
  DalBindInfo ci_binfo(schema::TableParentIndex(table_index));
  DalBindList bind_list = bind_info->get_bind_list();
  for (DalBindList::iterator iter = bind_list.begin();
       iter != bind_list.end(); ++iter) {
    // Break for loop when all Foreign Keys bound
    if (ci_binfo.get_input_bind_count() ==
        schema::TableNumFkCols(table_index)) {
      break;
    }

    DalBindColumnInfo *col_info = NULL;
    col_info = reinterpret_cast<DalBindColumnInfo *>(*iter);
    if (col_info == NULL) {
      UPLL_LOG_DEBUG("Invalid column Info");
      return kDalRcGeneralError;
    }

    // Bind only for Input and Foreign Key indices
    if ((col_info->get_io_type() != kDalIoInputOnly &&
         col_info->get_io_type() != kDalIoInputAndMatch) ||
        col_info->get_column_index() >= schema::TableNumFkCols(table_index)) {
      continue;
    }

    if (!ci_binfo.BindMatch(col_info->get_column_index(),
                            col_info->get_app_data_type(),
                            col_info->get_app_array_size(),
                            col_info->get_db_in_out_addr())) {
      UPLL_LOG_DEBUG("Error Binding Match for Parent Check");
      return kDalRcGeneralError;
    }
  }
  UPLL_LOG_VERBOSE("Constructed Bind Info for Parent Check : %s",
                   ci_binfo.BindListToStr().c_str());

  // Call RecordExists with the newly created BindInfo
  bool existence;
  dal_rc = RecordExists(cfg_type, schema::TableParentIndex(table_index),
                        &ci_binfo, &existence);
  if (dal_rc == kDalRcSuccess) {
    return ((existence == true) ? kDalRcRecordAlreadyExists
                                : kDalRcRecordNotFound);
  }
  return dal_rc;
}

DalResultCode
DalOdbcMgr::CheckInstance(const UpllCfgType cfg_type,
                          const DalTableIndex table_index,
                          const DalBindInfo *bind_info) const {
  UPLL_FUNC_TRACE;
  DalResultCode dal_rc;

  if (bind_info->get_input_bind_count() == 0) {
    UPLL_LOG_DEBUG("No Input bind data for CheckInstance");
    return kDalRcGeneralError;
  }

  // Create Bind Info for Instance Check
  DalBindInfo ci_binfo(table_index);
  DalBindList bind_list = bind_info->get_bind_list();
  for (DalBindList::iterator iter = bind_list.begin();
       iter != bind_list.end(); ++iter) {

    DalBindColumnInfo *col_info = NULL;
    col_info = reinterpret_cast<DalBindColumnInfo *>(*iter);
    if (col_info == NULL) {
      UPLL_LOG_DEBUG("Invalid column Info");
      return kDalRcGeneralError;
    }

    // Bind only for Input and Foreign Key indices
    if (col_info->get_io_type() != kDalIoInputOnly &&
        col_info->get_io_type() != kDalIoInputAndMatch) {
      continue;
    }
    if (col_info->get_column_index() >= schema::TableNumPkCols(table_index) &&
        !schema::AddtlBindForInstanceExistsCheck(
            table_index, col_info->get_column_index())) {
      continue;
    }

    if (!ci_binfo.BindMatch(col_info->get_column_index(),
                            col_info->get_app_data_type(),
                            col_info->get_app_array_size(),
                            col_info->get_db_in_out_addr())) {
      UPLL_LOG_DEBUG("Error Binding Match for Instance Check");
      return kDalRcGeneralError;
    }
  }
  UPLL_LOG_VERBOSE("Constructed Bind Info for Instance Check : %s",
                   ci_binfo.BindListToStr().c_str());

  // Call RecordExists with the existing BindInfo
  bool existence;
  dal_rc = RecordExists(cfg_type, table_index, &ci_binfo, &existence);
  if (dal_rc == kDalRcSuccess) {
    return ((existence == true) ? kDalRcRecordAlreadyExists
                                : kDalRcRecordNotFound);
  }
  return dal_rc;
}

// Creates a record with the given data
DalResultCode
DalOdbcMgr::CreateRecord(const UpllCfgType cfg_type,
                         const DalTableIndex table_index,
                         const DalBindInfo *bind_info) const {
  SQLHANDLE     dal_stmt_handle = SQL_NULL_HANDLE;
  DalResultCode dal_rc;
  DalQueryBuilder  qbldr;
  std::string query_stmt;

  // Validating Inputs
  if (cfg_type == UPLL_DT_INVALID) {
    UPLL_LOG_DEBUG("Invalid config type - %d", cfg_type);
    return kDalRcGeneralError;
  }

  if (table_index >= schema::table::kDalNumTables) {
    UPLL_LOG_DEBUG("Invalid table index - %d", table_index);
    return kDalRcGeneralError;
  }

  if (bind_info == NULL) {
    UPLL_LOG_DEBUG("NULL Bind Info for Table(%s)",
                   schema::TableName(table_index));
    return kDalRcGeneralError;
  }

  if (table_index != bind_info->get_table_index()) {
    UPLL_LOG_DEBUG("Table index mismatch with bind info "
                   "Query - %s; Bind - %s",
                   schema::TableName(table_index),
                   schema::TableName(bind_info->get_table_index()));
    return kDalRcGeneralError;
  }

  // Parent Existence Check for Import Datatype
  if (cfg_type == UPLL_DT_IMPORT && schema::TableNumFkCols(table_index) > 0) {
    dal_rc = CheckParentInstance(cfg_type, table_index, bind_info);
    if (dal_rc == kDalRcRecordNotFound) {
       UPLL_LOG_DEBUG("Parent Does not Exist");
       return kDalRcParentNotFound;
    } else if (dal_rc != kDalRcRecordAlreadyExists) {
       UPLL_LOG_DEBUG("Error during Parent Existence Check");
       return dal_rc;
    }
  }

  DalApiNum query_template;
  // Perf: Build query with c_flag/u_flag when cfg_type is candidate
  if (cfg_type == UPLL_DT_CANDIDATE) {
    // By default, create record in CAND with c_flag=1
    dal_rc = kDalRcRecordNotFound;
    // Perf: Incase of delete and create:- If delete dirty is 
    // set for table_index then check whether entry exists in RUNNING 
    if (delete_dirty.find(table_index) != delete_dirty.end())
        dal_rc = CheckInstance(UPLL_DT_RUNNING, table_index, bind_info);
    // If entry exists, create record in CAND with u_flag=1
    if (dal_rc == kDalRcRecordAlreadyExists)
      query_template = kDalCreateCandRecUpdateQT;
    else if (dal_rc == kDalRcRecordNotFound)
      query_template = kDalCreateCandRecQT;
    else
      return dal_rc;
  } else {
    query_template = kDalCreateRecQT;
  }

  // Build Query Statement
  if (qbldr.get_sql_statement(query_template, bind_info, query_stmt,
                        table_index, cfg_type) != true) {
    UPLL_LOG_DEBUG("Failed building query stmt");
    return kDalRcGeneralError;
  } else {
    UPLL_LOG_TRACE("Query stmt - %s", query_stmt.c_str());
  }

  // Allocate Stmt Handle, Bind and Execute the Query Statement
  dal_rc = ExecuteQuery(&dal_stmt_handle,
                        &query_stmt,
                        bind_info);
  FreeHandle(SQL_HANDLE_STMT, dal_stmt_handle);
  // Diagnose GeneralError for ParentCheck and InstanceCheck
  if (dal_rc == kDalRcGeneralError) {
    UPLL_LOG_DEBUG("Err - %d. Failed executing query stmt - %s",
                   dal_rc, query_stmt.c_str());
    dal_rc = CheckInstance(cfg_type, table_index, bind_info);
    if (dal_rc == kDalRcRecordAlreadyExists) {
      UPLL_LOG_DEBUG("Instance Already Exists");
      return kDalRcRecordAlreadyExists;
    } else if (dal_rc != kDalRcRecordNotFound) {
      UPLL_LOG_DEBUG("Error during Instance Check");
      return dal_rc;
    }
    if (schema::TableNumFkCols(table_index) > 0) {
      dal_rc = CheckParentInstance(cfg_type, table_index, bind_info);
      if (dal_rc == kDalRcRecordNotFound) {
        UPLL_LOG_DEBUG("Parent Does not Exist");
        return kDalRcParentNotFound;
      } else if (dal_rc != kDalRcRecordAlreadyExists) {
        UPLL_LOG_DEBUG("Error during Parent Check");
        return dal_rc;
      }
    }
  }
  if (dal_rc != kDalRcSuccess) {
    UPLL_LOG_DEBUG("Err - %d. Failed executing query stmt - %s",
                   dal_rc, query_stmt.c_str());
    return dal_rc;
  }
  UPLL_LOG_TRACE("Completed executing query stmt - %s",
                query_stmt.c_str());

  // Storing dirty table list for skip unmodified tables during commit
  if (cfg_type == UPLL_DT_CANDIDATE) {
    UPLL_LOG_TRACE("Insert for table %d", table_index);
    if ((dal_rc = SetTableDirty(cfg_type, table_index, UNC_OP_CREATE))
         != kDalRcSuccess) {
      UPLL_LOG_ERROR("SetTableDirty failed with rc=%d", dal_rc);
      return kDalRcGeneralError;
    }  
  }
  
  write_count_++;
  return kDalRcSuccess;
}   // DalOdbcMgr::CreateRecord

// Updates the matching records with the given data
DalResultCode
DalOdbcMgr::UpdateRecords(const UpllCfgType cfg_type,
                          const DalTableIndex table_index,
                          const DalBindInfo *bind_info) const {
  SQLHANDLE     dal_stmt_handle = SQL_NULL_HANDLE;
  DalResultCode dal_rc;
  DalQueryBuilder  qbldr;
  std::string query_stmt;

  // Validating Inputs
  if (cfg_type == UPLL_DT_INVALID) {
    UPLL_LOG_DEBUG("Invalid config type - %d", cfg_type);
    return kDalRcGeneralError;
  }

  if (table_index >= schema::table::kDalNumTables) {
    UPLL_LOG_DEBUG("Invalid table index - %d", table_index);
    return kDalRcGeneralError;
  }

  // PFC_ASSERT(bind_info)
  if (bind_info == NULL) {
    UPLL_LOG_DEBUG("NULL Bind Info for Table(%s)",
                   schema::TableName(table_index));
    return kDalRcGeneralError;
  }

  if (table_index != bind_info->get_table_index()) {
    UPLL_LOG_DEBUG("Table index mismatch with bind info "
                   "Query - %s; Bind - %s",
                   schema::TableName(table_index),
                   schema::TableName(bind_info->get_table_index()));
    return kDalRcGeneralError;
  }

  DalApiNum query_template;
  /* Perf: Build query with u_flag when cfg_type is candidate */
  if (cfg_type == UPLL_DT_CANDIDATE) {
    query_template = kDalUpdateCandRecQT; 
  } else {
    query_template = kDalUpdateRecQT;
  }

  // Build Query Statement
  if (qbldr.get_sql_statement(query_template, bind_info, query_stmt,
                        table_index, cfg_type) != true) {
    UPLL_LOG_DEBUG("Failed building query stmt");
    return kDalRcGeneralError;
  } else {
    UPLL_LOG_TRACE("Query stmt - %s", query_stmt.c_str());
  }

  // Allocate Stmt Handle, Bind and Execute the Query Statement
  dal_rc = ExecuteQuery(&dal_stmt_handle,
                        &query_stmt,
                        bind_info);

  if (dal_rc != kDalRcSuccess) {
    UPLL_LOG_DEBUG("Err - %d. Failed executing query stmt - %s",
                   dal_rc, query_stmt.c_str());
    if (dal_rc == kDalRcParentNotFound) {
      UPLL_LOG_DEBUG("Foreign Key Violation error. Returning kDalRcGeneralError");
      dal_rc = kDalRcGeneralError;
    }
    FreeHandle(SQL_HANDLE_STMT, dal_stmt_handle);
    return dal_rc;
  }
  UPLL_LOG_TRACE("Completed executing query stmt - %s",
                query_stmt.c_str());
  FreeHandle(SQL_HANDLE_STMT, dal_stmt_handle);

  // Storing dirty table list for skip unmodified tables during commit
  if (cfg_type == UPLL_DT_CANDIDATE) {
    if ((dal_rc = SetTableDirty(cfg_type, table_index, UNC_OP_UPDATE))
        != kDalRcSuccess) {
      UPLL_LOG_ERROR("SetTableDirty failed with rc=%d", dal_rc);
      return kDalRcGeneralError;
    }
  }
  write_count_++;
  return kDalRcSuccess;
}   // DalOdbcMgr::UpdateRecords

// Executes the user given query 
DalResultCode
DalOdbcMgr::ExecuteAppQuery(std::string query_stmt,
                            const UpllCfgType cfg_type,
                            const DalTableIndex table_index,
                            const DalBindInfo *bind_info,
                            const unc_keytype_operation_t dirty_op) const {
  SQLHANDLE     dal_stmt_handle = SQL_NULL_HANDLE;
  DalResultCode dal_rc = kDalRcGeneralError;
  DalQueryBuilder  qbldr;

  // Validating Inputs
  if (cfg_type == UPLL_DT_INVALID) {
    UPLL_LOG_DEBUG("Invalid config type - %d", cfg_type);
    return kDalRcGeneralError;
  }

  if (table_index >= schema::table::kDalNumTables) {
    UPLL_LOG_DEBUG("Invalid table index - %d", table_index);
    return kDalRcGeneralError;
  }

  if (bind_info == NULL) {
    UPLL_LOG_DEBUG("NULL Bind Info for Table(%s)",
                   schema::TableName(table_index));
    return kDalRcGeneralError;
  }

  if (table_index != bind_info->get_table_index()) {
    UPLL_LOG_DEBUG("Table index mismatch with bind info "
                   "Query - %s; Bind - %s",
                   schema::TableName(table_index),
                   schema::TableName(bind_info->get_table_index()));
    return kDalRcGeneralError;
  }
  if (query_stmt.empty()) {
    UPLL_LOG_DEBUG("Query statement is NULL");
    return kDalRcGeneralError;
  }
  // Allocate Stmt Handle, Bind and Execute the Query Statement
  dal_rc = ExecuteQuery(&dal_stmt_handle,
                        &query_stmt,
                        bind_info);

  if (dal_rc != kDalRcSuccess) {
    UPLL_LOG_DEBUG("Err - %d. Failed executing query stmt - %s",
                   dal_rc, query_stmt.c_str());
    if (dal_rc == kDalRcParentNotFound) {
      UPLL_LOG_DEBUG("Foreign Key Violation error. Returning kDalRcGeneralError");
      dal_rc = kDalRcGeneralError;
    }
    FreeHandle(SQL_HANDLE_STMT, dal_stmt_handle);
    return dal_rc;
  }
  UPLL_LOG_TRACE("Completed executing query stmt - %s",
                query_stmt.c_str());
  FreeHandle(SQL_HANDLE_STMT, dal_stmt_handle);

  // Storing dirty table list for skip unmodified tables during commit
  if (cfg_type == UPLL_DT_CANDIDATE) {
    if ((dal_rc = SetTableDirty(cfg_type, table_index, dirty_op))
        != kDalRcSuccess) {
      UPLL_LOG_ERROR("SetTableDirty failed with rc=%d", dal_rc);
      return kDalRcGeneralError;
    }
  }
  write_count_++;
  return kDalRcSuccess;
}   // DalOdbcMgr::ExecuteAppQuery


// Gets the records deleted in cfg_type_1
DalResultCode
DalOdbcMgr::GetDeletedRecords(const UpllCfgType cfg_type_1,
                              const UpllCfgType cfg_type_2,
                              const DalTableIndex table_index,
                              const size_t max_record_count,
                              const DalBindInfo *bind_info,
                              DalCursor **cursor) const {
  SQLHANDLE     dal_stmt_handle = SQL_NULL_HANDLE;
  DalResultCode dal_rc;
  DalQueryBuilder  qbldr;
  std::string query_stmt;
  *cursor = NULL;

  // Validating Inputs
  if (cfg_type_1 == UPLL_DT_INVALID || cfg_type_2 == UPLL_DT_INVALID) {
    UPLL_LOG_DEBUG("Invalid config type - %d %d", cfg_type_1, cfg_type_2);
    return kDalRcGeneralError;
  }

  if (cfg_type_1 == cfg_type_2) {
    UPLL_LOG_DEBUG("Same config type - %d %d", cfg_type_1, cfg_type_2);
    return kDalRcGeneralError;
  }

  if (table_index >= schema::table::kDalNumTables) {
    UPLL_LOG_DEBUG("Invalid table index - %d", table_index);
    return kDalRcGeneralError;
  }

  // Skip if table is non dirty for commit
  if (cfg_type_1 == UPLL_DT_CANDIDATE && cfg_type_2 == UPLL_DT_RUNNING &&
      delete_dirty.find(table_index) == delete_dirty.end()) {
    UPLL_LOG_DEBUG("Skipping GetDeletedRecords for %s",
      schema::TableName(table_index));
    return kDalRcRecordNotFound;
  }
  UPLL_LOG_TRACE("Performing GetDeletedRecords for %s",
    schema::TableName(table_index));

  // PFC_ASSERT(bind_info)
  if (bind_info == NULL) {
    UPLL_LOG_DEBUG("NULL Bind Info for Table(%s)",
                   schema::TableName(table_index));
    return kDalRcGeneralError;
  }

  if (table_index != bind_info->get_table_index()) {
    UPLL_LOG_DEBUG("Table index mismatch with bind info "
                   "Query - %s; Bind - %s",
                   schema::TableName(table_index),
                   schema::TableName(bind_info->get_table_index()));
    return kDalRcGeneralError;
  }

  // Build Query Statement
  if (qbldr.get_sql_statement(kDalGetDelRecQT, bind_info, query_stmt,
                        table_index, cfg_type_1, cfg_type_2) != true) {
    UPLL_LOG_TRACE("Failed building query stmt");
    return kDalRcGeneralError;
  } else {
    UPLL_LOG_DEBUG("Query stmt - %s", query_stmt.c_str());
  }

  // Allocate Stmt Handle, Bind and Execute the Query Statement
  dal_rc = ExecuteQuery(&dal_stmt_handle,
                        &query_stmt,
                        bind_info,
                        max_record_count);

  if (dal_rc != kDalRcSuccess) {
    UPLL_LOG_DEBUG("Err - %d. Failed executing query stmt - %s",
                   dal_rc, query_stmt.c_str());
    FreeHandle(SQL_HANDLE_STMT, dal_stmt_handle);
    return dal_rc;
  }
  UPLL_LOG_TRACE("Completed executing query stmt - %s",
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

DalResultCode
DalOdbcMgr::ClearCreateUpdateFlags(const DalTableIndex table_index,
                                    const UpllCfgType cfg_type) const {
  SQLHANDLE     dal_stmt_handle = SQL_NULL_HANDLE;
  DalResultCode dal_rc;
  DalQueryBuilder  qbldr;
  std::string query_stmt;

  if (cfg_type == UPLL_DT_INVALID) {
    UPLL_LOG_DEBUG("Invalid config type - %d", cfg_type);
    return kDalRcGeneralError;
  }

  if (table_index >= schema::table::kDalNumTables) {
    UPLL_LOG_DEBUG("Invalid table index - %d", table_index);
    return kDalRcGeneralError;
  }

  // Skip if table is not dirty
  if (cfg_type == UPLL_DT_CANDIDATE) {
    if (create_dirty.find(table_index) == create_dirty.end() &&
       update_dirty.find(table_index) == update_dirty.end()) {
      UPLL_LOG_DEBUG("Skipping ClearCreateUpdateFlags for %s",
          schema::TableName(table_index));
      return kDalRcRecordNotFound;
    }
  }

  // Build Query Statement
  if (qbldr.get_sql_statement(kDalClearCandFlagsQT, NULL, query_stmt,
                        table_index, cfg_type) != true) {
    UPLL_LOG_TRACE("Failed building query stmt");
    return kDalRcGeneralError;
  } else {
    UPLL_LOG_DEBUG("Query stmt - %s", query_stmt.c_str());
  }

  // Allocate Stmt Handle, Bind and Execute the Query Statement
  dal_rc = ExecuteQuery(&dal_stmt_handle,
                        &query_stmt,
                        NULL);

  FreeHandle(SQL_HANDLE_STMT, dal_stmt_handle);
  if (dal_rc != kDalRcSuccess) {
    UPLL_LOG_DEBUG("Err - %d. Failed executing query stmt - %s",
                   dal_rc, query_stmt.c_str());
    return dal_rc;
  }
  UPLL_LOG_TRACE("Completed executing query stmt - %s",
                query_stmt.c_str());

  return kDalRcSuccess;
} // DalOdbcMgr::ClearCreateUpdateFlags

// Gets the records created in cfg_type_1
DalResultCode
DalOdbcMgr::GetCreatedRecords(const UpllCfgType cfg_type_1,
                              const UpllCfgType cfg_type_2,
                              const DalTableIndex table_index,
                              const size_t max_record_count,
                              const DalBindInfo *bind_info,
                              DalCursor **cursor) const {
  SQLHANDLE     dal_stmt_handle = SQL_NULL_HANDLE;
  DalResultCode dal_rc;
  DalQueryBuilder  qbldr;
  std::string query_stmt;

  // Validating Inputs
  if (cfg_type_1 == UPLL_DT_INVALID || cfg_type_2 == UPLL_DT_INVALID) {
    UPLL_LOG_DEBUG("Invalid config type - %d %d", cfg_type_1, cfg_type_2);
    return kDalRcGeneralError;
  }

  if (cfg_type_1 == cfg_type_2) {
    UPLL_LOG_DEBUG("Same config type - %d %d", cfg_type_1, cfg_type_2);
    return kDalRcGeneralError;
  }

  if (table_index >= schema::table::kDalNumTables) {
    UPLL_LOG_DEBUG("Invalid table index - %d", table_index);
    return kDalRcGeneralError;
  }

  // Skip if table is not dirty
  if (cfg_type_1 == UPLL_DT_CANDIDATE && cfg_type_2 == UPLL_DT_RUNNING &&
      create_dirty.find(table_index) == create_dirty.end()) {
    UPLL_LOG_DEBUG("Skipping GetCreatedRecords for %s",
      schema::TableName(table_index));
    return kDalRcRecordNotFound;
  }
  UPLL_LOG_TRACE("Performing GetCreatedRecords for %s",
    schema::TableName(table_index));

  // PFC_ASSERT(bind_info)
  if (bind_info == NULL) {
    UPLL_LOG_DEBUG("NULL Bind Info for Table(%s)",
                   schema::TableName(table_index));
    return kDalRcGeneralError;
  }

  if (table_index != bind_info->get_table_index()) {
    UPLL_LOG_DEBUG("Table index mismatch with bind info "
                   "Query - %s; Bind - %s",
                   schema::TableName(table_index),
                   schema::TableName(bind_info->get_table_index()));
    return kDalRcGeneralError;
  }

  DalApiNum query_template;
  if (cfg_type_1 == UPLL_DT_CANDIDATE && cfg_type_2 == UPLL_DT_RUNNING)
    query_template = kDalGetCreatedRecInCandQT;
  else
    query_template = kDalGetCreatedRecQT;

  // Build Query Statement
  if (qbldr.get_sql_statement(query_template, bind_info, query_stmt,
                        table_index, cfg_type_1, cfg_type_2) != true) {
    UPLL_LOG_TRACE("Failed building query stmt");
    return kDalRcGeneralError;
  } else {
    UPLL_LOG_DEBUG("Query stmt - %s", query_stmt.c_str());
  }

  // Allocate Stmt Handle, Bind and Execute the Query Statement
  dal_rc = ExecuteQuery(&dal_stmt_handle,
                        &query_stmt,
                        bind_info,
                        max_record_count);

  if (dal_rc != kDalRcSuccess) {
    UPLL_LOG_DEBUG("Err - %d. Failed executing query stmt - %s",
                   dal_rc, query_stmt.c_str());
    FreeHandle(SQL_HANDLE_STMT, dal_stmt_handle);
    return dal_rc;
  }
  UPLL_LOG_TRACE("Completed executing query stmt - %s",
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
    UPLL_LOG_DEBUG("Invalid config type - (%d, %d)", cfg_type_1, cfg_type_2);
    return kDalRcGeneralError;
  }

  if (cfg_type_1 == cfg_type_2) {
    UPLL_LOG_DEBUG("Same config type - (%d, %d)", cfg_type_1, cfg_type_2);
    return kDalRcGeneralError;
  }

  if (table_index >= schema::table::kDalNumTables) {
    UPLL_LOG_DEBUG("Invalid table index - %d", table_index);
    return kDalRcGeneralError;
  }

  // Skip if table is not dirty
  if (cfg_type_1 == UPLL_DT_CANDIDATE && cfg_type_2 == UPLL_DT_RUNNING &&
      update_dirty.find(table_index) == update_dirty.end()) {
    UPLL_LOG_DEBUG("Skipping GetUpdatedRecords for %s",
      schema::TableName(table_index));
    return kDalRcRecordNotFound;
  }
  UPLL_LOG_TRACE("Performing GetUpdatedRecords for %s",
    schema::TableName(table_index));

  // PFC_ASSERT(bind_info)
  if (cfg_1_bind_info == NULL || cfg_2_bind_info == NULL) {
    UPLL_LOG_DEBUG("NULL Bind Info for Table(%s)",
                   schema::TableName(table_index));
    return kDalRcGeneralError;
  }

  if (table_index != cfg_1_bind_info->get_table_index() ||
      table_index != cfg_2_bind_info->get_table_index()) {
    UPLL_LOG_DEBUG("Table index mismatch with bind info "
                   "Query - %s; Bind1 - %s; Bind2 - %s",
                   schema::TableName(table_index),
                   schema::TableName(cfg_1_bind_info->get_table_index()),
                   schema::TableName(cfg_2_bind_info->get_table_index()));
    return kDalRcGeneralError;
  }

  DalApiNum query_template;

  if (cfg_type_1 == UPLL_DT_CANDIDATE && cfg_type_2 == UPLL_DT_RUNNING)
    query_template = kDalGetModRecConfig1QT;
  else
    query_template = kDalGetModRecQT;

  // To Fetch the updated records from cfg_type_1
  // Build Query Statement
  if (qbldr.get_sql_statement(query_template,
                              cfg_1_bind_info,
                              query_stmt,
                              table_index,
                              cfg_type_1,
                              cfg_type_2) != true) {
    UPLL_LOG_TRACE("Failed building query stmt");
    return kDalRcGeneralError;
  } else {
    UPLL_LOG_DEBUG("Query stmt - %s", query_stmt.c_str());
  }

  // Allocate Stmt Handle, Bind and Execute the Query Statement
  dal_rc = ExecuteQuery(&cfg_1_stmt_handle,
                        &query_stmt,
                        cfg_1_bind_info,
                        max_record_count);

  if (dal_rc != kDalRcSuccess) {
    UPLL_LOG_DEBUG("Err - %d. Failed executing query stmt - %s",
                   dal_rc, query_stmt.c_str());
    FreeHandle(SQL_HANDLE_STMT, cfg_1_stmt_handle);
    return dal_rc;
  }
  UPLL_LOG_TRACE("Completed executing query stmt - %s",
                query_stmt.c_str());

  if (cfg_type_1 == UPLL_DT_CANDIDATE && cfg_type_2 == UPLL_DT_RUNNING)
    query_template = kDalGetModRecConfig2QT;
  else
    query_template = kDalGetModRecQT;

  // To Fetch the updated records from cfg_type_2
  // Build Query Statement
  if (qbldr.get_sql_statement(query_template,
                              cfg_2_bind_info,
                              query_stmt,
                              table_index,
    ((query_template == kDalGetModRecQT) ? cfg_type_2 : cfg_type_1),
    ((query_template == kDalGetModRecQT) ? cfg_type_1 : cfg_type_2)
                              ) != true) {
    UPLL_LOG_TRACE("Failed building query stmt");
    return kDalRcGeneralError;
  } else {
    UPLL_LOG_DEBUG("Query stmt - %s", query_stmt.c_str());
  }

  // Allocate Stmt Handle, Bind and Execute the Query Statement
  dal_rc = ExecuteQuery(&cfg_2_stmt_handle,
                        &query_stmt,
                        cfg_2_bind_info,
                        max_record_count);

  if (dal_rc != kDalRcSuccess) {
    UPLL_LOG_DEBUG("Err - %d. Failed executing query stmt - %s",
                   dal_rc, query_stmt.c_str());
    if (cfg_1_stmt_handle != SQL_NULL_HANDLE) {
      FreeHandle(SQL_HANDLE_STMT, cfg_1_stmt_handle);
    }
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
    UPLL_LOG_DEBUG("Invalid config type - %d %d", dest_cfg_type, src_cfg_type);
    return kDalRcGeneralError;
  }

  if (dest_cfg_type == src_cfg_type) {
    UPLL_LOG_DEBUG("Same config type - %d %d", dest_cfg_type, src_cfg_type);
    return kDalRcGeneralError;
  }

  if (table_index >= schema::table::kDalNumTables) {
    UPLL_LOG_DEBUG("Invalid table index - %d", table_index);
    return kDalRcGeneralError;
  }

  // No need to validate bindinfo
  // BindInput - NA; BindOuput - O; BindMatch - NA;
  if (bind_info != NULL  && table_index != bind_info->get_table_index()) {
    UPLL_LOG_DEBUG("Table index mismatch with bind info "
                   "Query - %s; Bind - %s",
                   schema::TableName(table_index),
                   schema::TableName(bind_info->get_table_index()));
    return kDalRcGeneralError;
  }

  // Build Query Statement
  // copy records from src to dest cfg_type
  if (qbldr.get_sql_statement(kDalCopyEntireRecQT, bind_info, query_stmt,
                        table_index, dest_cfg_type, src_cfg_type) != true) {
    UPLL_LOG_TRACE("Failed building query stmt");
    return kDalRcGeneralError;
  } else {
    UPLL_LOG_DEBUG("Query stmt - %s", query_stmt.c_str());
  }

  // Allocate Stmt Handle, Bind and Execute the Query Statement
  dal_rc = ExecuteQuery(&dal_stmt_handle,
                        &query_stmt, bind_info);

  if (dal_rc == kDalRcParentNotFound) {
    UPLL_LOG_DEBUG("Foreign Key Violation error. Returning kDalRcGeneralError");
    dal_rc = kDalRcGeneralError;
  }
  if (dal_rc != kDalRcSuccess) {
    UPLL_LOG_DEBUG("Err - %d. Failed executing query stmt - %s",
                   dal_rc, query_stmt.c_str());
    FreeHandle(SQL_HANDLE_STMT, dal_stmt_handle);
    return dal_rc;
  }
  UPLL_LOG_TRACE("Completed executing query stmt - %s",
                query_stmt.c_str());
  FreeHandle(SQL_HANDLE_STMT, dal_stmt_handle);
  if (dest_cfg_type == UPLL_DT_CANDIDATE) {
    // Note: Precondition for CopyEntireRecords is DeleteRecords(all rows)
    // So, it needs to be treated that new entries are created and old
    // entries are updated or deleted.
//TODO(rev): Should we write to DB also?
    create_dirty.insert(table_index);
    update_dirty.insert(table_index);
//TODO(rev): DeleteRecords() would set delete_dirty. Why is it done again?
    delete_dirty.insert(table_index);
  }
  return kDalRcSuccess;
}   // DalOdbcMgr::CopyEntireRecords

// Copies only the modified records from src to dst cfg_type
DalResultCode
DalOdbcMgr::CopyModifiedRecords(const UpllCfgType dest_cfg_type,
                                const UpllCfgType src_cfg_type,
                                const DalTableIndex table_index,
                                const DalBindInfo *bind_info,
                                const unc_keytype_operation_t op) const {
  SQLHANDLE     dal_stmt_handle = SQL_NULL_HANDLE;
  DalResultCode dal_rc;
  DalQueryBuilder  qbldr;
  std::string query_stmt;

  // Validating Inputs
  if (dest_cfg_type == UPLL_DT_INVALID || src_cfg_type == UPLL_DT_INVALID) {
    UPLL_LOG_DEBUG("Invalid config type - %d %d", dest_cfg_type, src_cfg_type);
    return kDalRcGeneralError;
  }

  if (dest_cfg_type == src_cfg_type) {
    UPLL_LOG_DEBUG("Same config type - %d %d", dest_cfg_type, src_cfg_type);
    return kDalRcGeneralError;
  }

  if (table_index >= schema::table::kDalNumTables) {
    UPLL_LOG_DEBUG("Invalid table index - %d", table_index);
    return kDalRcGeneralError;
  }

  // No need to validate bindinfo
  // BindInput - NA; BindOuput - O; BindMatch - O;
  if (bind_info != NULL && table_index != bind_info->get_table_index()) {
    UPLL_LOG_DEBUG("Table index mismatch with bind info "
                   "Query - %s; Bind - %s",
                   schema::TableName(table_index),
                   schema::TableName(bind_info->get_table_index()));
    return kDalRcGeneralError;
  }

  DalApiNum query_template;
  if (op == UNC_OP_DELETE) {
    if (dest_cfg_type == UPLL_DT_CANDIDATE &&
        src_cfg_type == UPLL_DT_RUNNING &&
        (create_dirty.find(table_index) == create_dirty.end())) {
       UPLL_LOG_DEBUG("No entries modified for the operation %d in table %d",
                      op, table_index);
       return kDalRcRecordNotFound;
    }
    if (dest_cfg_type == UPLL_DT_CANDIDATE &&
        src_cfg_type == UPLL_DT_IMPORT) {
      query_template = kDalCopyModRecDelImportQT;
    } else {
      query_template = kDalCopyModRecDelQT;
    }
  } else if (op == UNC_OP_CREATE) {
    if (dest_cfg_type == UPLL_DT_CANDIDATE &&
        src_cfg_type == UPLL_DT_RUNNING &&
        delete_dirty.find(table_index) == delete_dirty.end()) {
       UPLL_LOG_DEBUG("No entries modified for the operation %d in table %d",
                      op, table_index); 
       return kDalRcRecordNotFound;
    }
    if (dest_cfg_type == UPLL_DT_CANDIDATE &&
        src_cfg_type == UPLL_DT_IMPORT) {
      query_template = kDalCopyModRecCreateImportQT;
    } else {
      query_template = kDalCopyModRecCreateQT;
    }
  } else if (op == UNC_OP_UPDATE) {
    if (dest_cfg_type == UPLL_DT_CANDIDATE &&
        src_cfg_type == UPLL_DT_RUNNING &&
        update_dirty.find(table_index) == update_dirty.end()) {
       UPLL_LOG_DEBUG("No entries modified for the operation %d in table %d",
                      op, table_index); 
       return kDalRcRecordNotFound;
    }
    if (dest_cfg_type == UPLL_DT_CANDIDATE &&
        src_cfg_type == UPLL_DT_RUNNING) {
      // Copy RUNNING to CANDIDATE during abort
      query_template = kDalCopyModRecUpdateAbortQT;
    } else if (dest_cfg_type == UPLL_DT_CANDIDATE &&
                src_cfg_type == UPLL_DT_IMPORT){
      // Copy IMPORT to CANDIDATE during merge
      query_template = kDalCopyModRecUpdateImportQT;
    } else {
      query_template = kDalCopyModRecUpdateQT;
    }
  } else {
    UPLL_LOG_DEBUG("Invalid operation %d", op);
    return kDalRcGeneralError;
  }

  // Build Query Statement
  if (qbldr.get_sql_statement(query_template, bind_info, query_stmt,
                        table_index, dest_cfg_type, src_cfg_type) != true) {
    UPLL_LOG_TRACE("Failed building query stmt");
    return kDalRcGeneralError;
  } else {
    UPLL_LOG_DEBUG("Query stmt - %s", query_stmt.c_str());
  }

  // Allocate Stmt Handle, Bind and Execute the Query Statement
  dal_rc = ExecuteQuery(&dal_stmt_handle,
                        &query_stmt, bind_info);

  if (dal_rc == kDalRcParentNotFound) {
    UPLL_LOG_DEBUG("Foreign Key Violation error. Returning kDalRcGeneralError");
    dal_rc = kDalRcGeneralError;
  }
  if (dal_rc != kDalRcSuccess && dal_rc != kDalRcRecordNotFound) {
    UPLL_LOG_DEBUG("Err - %d. Failed executing query stmt - %s",
                   dal_rc, query_stmt.c_str());
    FreeHandle(SQL_HANDLE_STMT, dal_stmt_handle);
    return dal_rc;
  }

  if (dal_rc == kDalRcSuccess && dest_cfg_type == UPLL_DT_CANDIDATE) {
    if ((dal_rc = SetTableDirty(dest_cfg_type, table_index, op))
        != kDalRcSuccess) {
      UPLL_LOG_ERROR("SetTableDirty failed with rc=%d", dal_rc);
      dal_rc = kDalRcGeneralError;
    }
  }

  FreeHandle(SQL_HANDLE_STMT, dal_stmt_handle);
  UPLL_LOG_TRACE("Completed executing query stmt - %s",
                query_stmt.c_str());

  return dal_rc;
}  // DalOdbcMgr::CopyModifiedRecords

// Copies the matching records from src to dest cfg_type
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
    UPLL_LOG_DEBUG("Invalid config type - %d %d", dest_cfg_type, src_cfg_type);
    return kDalRcGeneralError;
  }

  if (dest_cfg_type == src_cfg_type) {
    UPLL_LOG_DEBUG("Same config type - %d %d", dest_cfg_type, src_cfg_type);
    return kDalRcGeneralError;
  }

  if (table_index >= schema::table::kDalNumTables) {
    UPLL_LOG_DEBUG("Invalid table index - %d", table_index);
    return kDalRcGeneralError;
  }

  // BindInput - NA; BindOuput - O; BindMatch - M;
  if (bind_info == NULL) {
    UPLL_LOG_DEBUG("NULL Bind Info for Table(%s)",
                   schema::TableName(table_index));
    return kDalRcGeneralError;
  }

  if (table_index != bind_info->get_table_index()) {
    UPLL_LOG_DEBUG("Table index mismatch with bind info "
                   "Query - %s; Bind - %s",
                   schema::TableName(table_index),
                   schema::TableName(bind_info->get_table_index()));
    return kDalRcGeneralError;
  }


  // Build Query Statement
  if (qbldr.get_sql_statement(kDalCopyMatchingRecQT, bind_info, query_stmt,
                        table_index, dest_cfg_type, src_cfg_type) != true) {
    UPLL_LOG_TRACE("Failed building query stmt");
    return kDalRcGeneralError;
  } else {
    UPLL_LOG_DEBUG("Query stmt - %s", query_stmt.c_str());
  }

  // Allocate Stmt Handle, Bind and Execute the Query Statement
  dal_rc = ExecuteQuery(&dal_stmt_handle,
                        &query_stmt,
                        bind_info);

  if (dal_rc == kDalRcParentNotFound) {
    UPLL_LOG_DEBUG("Foreign Key Violation error. Returning kDalRcGeneralError");
    dal_rc = kDalRcGeneralError;
  }
  FreeHandle(SQL_HANDLE_STMT, dal_stmt_handle);
  if (dal_rc != kDalRcSuccess) {
    UPLL_LOG_DEBUG("Err - %d. Failed executing query stmt - %s",
                   dal_rc, query_stmt.c_str());
    return dal_rc;
  }

  UPLL_LOG_TRACE("Completed executing query stmt - %s",
                query_stmt.c_str());
  if (dest_cfg_type == UPLL_DT_CANDIDATE) {
    // additionally treat that table is updated and deleted, just in case
    if (((dal_rc == SetTableDirty(dest_cfg_type, table_index, UNC_OP_CREATE))
         == kDalRcSuccess) &&
        ((dal_rc == SetTableDirty(dest_cfg_type, table_index, UNC_OP_UPDATE))
         == kDalRcSuccess)) { 
    } else {
        UPLL_LOG_ERROR("SetTableDirty failed with rc=%d", dal_rc);
        dal_rc = kDalRcGeneralError;
    }
  }
  return dal_rc;
}  // DalOdbcMgr::CopyMatchingRecords

// Checks the matching records are identical in both the cfg_types
DalResultCode
DalOdbcMgr::CheckRecordsIdentical(const UpllCfgType cfg_type_1,
                                  const UpllCfgType cfg_type_2,
                                  const DalTableIndex table_index,
                                  const DalBindInfo *bind_info,
                                  bool *identical) const {
  UPLL_FUNC_TRACE;
  SQLHANDLE     dal_stmt_handle = SQL_NULL_HANDLE;
  SQLRETURN     sql_rc;
  DalResultCode dal_rc;
  DalQueryBuilder  qbldr;
  std::string query_stmt;
  uint32_t count = 0;

  // Validating Inputs
  if (cfg_type_1 == UPLL_DT_INVALID || cfg_type_2 == UPLL_DT_INVALID) {
    UPLL_LOG_DEBUG("Invalid config type - %d %d", cfg_type_1, cfg_type_2);
    return kDalRcGeneralError;
  }

  if (cfg_type_1 == cfg_type_2) {
    UPLL_LOG_DEBUG("Same config type - %d %d", cfg_type_1, cfg_type_2);
    return kDalRcGeneralError;
  }

  if (table_index >= schema::table::kDalNumTables) {
    UPLL_LOG_DEBUG("Invalid table index - %d", table_index);
    return kDalRcGeneralError;
  }

  // No need to validate bindinfo
  // BindInput - NA; BindOuput - NA; BindMatch - O;
  if (bind_info != NULL && table_index != bind_info->get_table_index()) {
    UPLL_LOG_DEBUG("Table index mismatch with bind info "
                   "Query - %s; Bind - %s",
                   schema::TableName(table_index),
                   schema::TableName(bind_info->get_table_index()));
    return kDalRcGeneralError;
  }

  // Build Query Statement
  if (qbldr.get_sql_statement(kDalCheckRecIdenticalQT, bind_info, query_stmt,
                        table_index, cfg_type_1, cfg_type_2) != true) {
    UPLL_LOG_TRACE("Failed building query stmt");
    return kDalRcGeneralError;
  } else {
    UPLL_LOG_DEBUG("Query stmt - %s", query_stmt.c_str());
  }

  // Allocate Stmt Handle, Bind and Execute the Query Statement
  dal_rc = ExecuteQuery(&dal_stmt_handle,
                        &query_stmt,
                        bind_info);

  if (dal_rc != kDalRcSuccess) {
    UPLL_LOG_DEBUG("Err - %d. Failed executing query stmt - %s",
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
    UPLL_LOG_DEBUG("Err - %d. Failed Binding count for query stmt", dal_rc);
    FreeHandle(SQL_HANDLE_STMT, dal_stmt_handle);
    return dal_rc;
  }
  UPLL_LOG_DEBUG("Count variable bound for query stmt");

  // Fetching results from the resultset
  sql_rc = SQLFetch(dal_stmt_handle);
  DalErrorHandler::ProcessOdbcErrors(SQL_HANDLE_STMT,
                                     dal_stmt_handle,
                                     sql_rc, &dal_rc);
  SET_DB_STATE_DISCONNECT(dal_rc, conn_state_);
  if (dal_rc != kDalRcSuccess) {
    UPLL_LOG_INFO("Err - %d. Failed to fetch result from DB, query stmt - %s",
                  dal_rc, query_stmt.c_str());
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
    UPLL_LOG_DEBUG("Empty query stmt string");
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
    UPLL_LOG_DEBUG("Err - %d. Failed executing query stmt - %s",
                   dal_rc, query_stmt.c_str());
    FreeHandle(SQL_HANDLE_STMT, dal_stmt_handle);
    return dal_rc;
  }
  UPLL_LOG_TRACE("Completed executing query stmt - %s", query_stmt.c_str());

  // Fetching results from the resultset
  sql_rc = SQLFetch(dal_stmt_handle);
  DalErrorHandler::ProcessOdbcErrors(SQL_HANDLE_STMT,
                                     dal_stmt_handle,
                                     sql_rc, &dal_rc);
  SET_DB_STATE_DISCONNECT(dal_rc, conn_state_);
  if (dal_rc != kDalRcSuccess) {
    if (dal_rc != kDalRcRecordNoMore) {
      UPLL_LOG_INFO("%d - Failed to Fetch result, query stmt - %s",
                    dal_rc, query_stmt.c_str());
    } else {
      UPLL_LOG_DEBUG("%d - Failed to Fetch result, query stmt - %s",
                     dal_rc, query_stmt.c_str());
    }
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
    UPLL_LOG_DEBUG("Empty query stmt string");
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
    UPLL_LOG_DEBUG("Err - %d. Failed executing query stmt - %s",
                   dal_rc, query_stmt.c_str());
    FreeHandle(SQL_HANDLE_STMT, dal_stmt_handle);
    return dal_rc;
  }
  UPLL_LOG_TRACE("Completed executing query stmt - %s",
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
  SET_DB_STATE_DISCONNECT(dal_rc, conn_state_);
  if (dal_rc != kDalRcSuccess) {
    UPLL_LOG_TRACE(" Failed to Set NoScan On. Default value set");
  }

  // SQL_ATTR_QUERY_TIMEOUT - conf (default: 0 no timeout)
#if 0
  // TODO(sankar): should come from conf file
  uint32_t query_timeout = 0;
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
  SET_DB_STATE_DISCONNECT(dal_rc, conn_state_);
  if (dal_rc != kDalRcSuccess) {
    UPLL_LOG_TRACE(" Failed to set Query Timeout. Default value set");
  }
#endif
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
  SET_DB_STATE_DISCONNECT(dal_rc, conn_state_);
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
  DalResultCode dal_rc = kDalRcGeneralError;
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
  DalResultCode dal_rc = kDalRcGeneralError;
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
  DalResultCode dal_rc = kDalRcGeneralError;
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
    UPLL_LOG_ERROR("NULL Statement Handle Reference");
    return kDalRcGeneralError;
  }

  if (query_stmt == NULL) {
    UPLL_LOG_ERROR("NULL Query stmt");
    return kDalRcGeneralError;
  }

  // Allocate Statement Handle
  sql_rc = SQLAllocHandle(SQL_HANDLE_STMT,
                          dal_conn_handle_,
                          dal_stmt_handle);
  DalErrorHandler::ProcessOdbcErrors(SQL_HANDLE_STMT,
                                     *dal_stmt_handle,
                                     sql_rc, &dal_rc);
  SET_DB_STATE_DISCONNECT(dal_rc, conn_state_);
  if (dal_rc != kDalRcSuccess) {
    UPLL_LOG_DEBUG("Err - %d. Failed to Allocate Statement handle",
                   dal_rc);
    return dal_rc;
  }
  if (*dal_stmt_handle == SQL_NULL_HANDLE) {
    UPLL_LOG_ERROR("Err - %d. Failed to Allocate Statement handle",
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
    SET_DB_STATE_DISCONNECT(dal_rc, conn_state_);
    if (dal_rc != kDalRcSuccess) {
      UPLL_LOG_INFO("Err - %d. Failed to Bind parameters to Query", dal_rc);
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
  SET_DB_STATE_DISCONNECT(dal_rc, conn_state_);
  if (dal_rc != kDalRcSuccess) {
    if (dal_rc != kDalRcRecordNotFound &&
        dal_rc != kDalRcRecordAlreadyExists) {
      UPLL_LOG_INFO("Err - %d. Failed to Execute Query %s",
                    dal_rc, query_stmt->c_str());
    } else {
      UPLL_LOG_DEBUG("Err - %d. Failed to Execute Query %s",
                     dal_rc, query_stmt->c_str());
    }
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
  std::string dal_cf_str = DAL_CONF_FILE;
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


// Creates a record with the given data
DalResultCode
DalOdbcMgr::ExecuteAppQueryModifyRecord(
                         const UpllCfgType cfg_type,
                         const DalTableIndex table_index,
                         const std::string query_stmt,
                         const DalBindInfo *bind_info,
                         const unc_keytype_operation_t op) const {
  SQLHANDLE     dal_stmt_handle = SQL_NULL_HANDLE;
  DalResultCode dal_rc;

  // Validating Inputs
  if (bind_info == NULL) {
    UPLL_LOG_DEBUG("NULL Bind Info for Table(%s)",
                   schema::TableName(table_index));
    if (op == UNC_OP_CREATE || op == UNC_OP_UPDATE) {
      return kDalRcGeneralError;
    }
  }

  // Parent Existence Check for Import Datatype
  if (cfg_type == UPLL_DT_IMPORT && op == UNC_OP_CREATE &&
      schema::TableNumFkCols(table_index) > 0) {
    dal_rc = CheckParentInstance(cfg_type, table_index, bind_info);
    if (dal_rc == kDalRcRecordNotFound) {
       UPLL_LOG_DEBUG("Parent Does not Exist");
       return kDalRcParentNotFound;
    } else if (dal_rc != kDalRcRecordAlreadyExists) {
       UPLL_LOG_DEBUG("Error during Parent Existence Check");
       return dal_rc;
    }
  }

  // Allocate Stmt Handle, Bind and Execute the Query Statement
  dal_rc = ExecuteQuery(&dal_stmt_handle, &query_stmt, bind_info);
  FreeHandle(SQL_HANDLE_STMT, dal_stmt_handle);

  // Diagnose GeneralError for ParentCheck and InstanceCheck
  if (dal_rc == kDalRcGeneralError &&
      cfg_type == UPLL_DT_CANDIDATE &&
      op == UNC_OP_CREATE) {
    UPLL_LOG_DEBUG("Err - %d. Failed executing query stmt - %s",
                   dal_rc, query_stmt.c_str());
    dal_rc = CheckInstance(cfg_type, table_index, bind_info);
    if (dal_rc == kDalRcRecordAlreadyExists) {
      UPLL_LOG_DEBUG("Instance Already Exists");
      return kDalRcRecordAlreadyExists;
    } else if (dal_rc != kDalRcRecordNotFound) {
      UPLL_LOG_DEBUG("Error during Instance Check");
      return dal_rc;
    }
    if (schema::TableNumFkCols(table_index) > 0) {
      dal_rc = CheckParentInstance(cfg_type, table_index, bind_info);
      if (dal_rc == kDalRcRecordNotFound) {
        UPLL_LOG_DEBUG("Parent Does not Exist");
        return kDalRcParentNotFound;
      } else if (dal_rc != kDalRcRecordAlreadyExists) {
        UPLL_LOG_DEBUG("Error during Parent Check");
        return dal_rc;
      }
    }
  }

  if (dal_rc != kDalRcSuccess) {
    UPLL_LOG_DEBUG("Err - %d. Failed executing query stmt - %s",
                   dal_rc, query_stmt.c_str());
    return dal_rc;
  }
  UPLL_LOG_TRACE("Completed executing query stmt - %s",
                query_stmt.c_str());

  // Storing dirty table list for skip unmodified tables during commit
  if (cfg_type == UPLL_DT_CANDIDATE) {
    UPLL_LOG_TRACE("Insert for table %d operation %d", table_index, op);
    if ((dal_rc = SetTableDirty(cfg_type, table_index, op)) != kDalRcSuccess) {
      return dal_rc;
    }
  }
  write_count_++;
  return kDalRcSuccess;
}  // DalOdbcMgr::ExecuteAppQueryModifyRecord

DalResultCode
DalOdbcMgr::SetTableDirty(const UpllCfgType cfg_type,
                          const DalTableIndex table_index,
                          const unc_keytype_operation_t op) const {
  DalResultCode dal_rc;
  if (op == UNC_OP_DELETE) {
    if (delete_dirty.end() == delete_dirty.find(table_index)) {
      if ((dal_rc = SetCfgTblDirtyInDB(cfg_type, op, table_index))
          != kDalRcSuccess) {
        UPLL_LOG_ERROR("SetCfgTblDirtyInDB failed with rc=%d", dal_rc);
        return kDalRcGeneralError;
      }
    }
    delete_dirty.insert(table_index);
  } else if (op == UNC_OP_CREATE) {
    if (create_dirty.end() == create_dirty.find(table_index)) {
      if ((dal_rc = SetCfgTblDirtyInDB(cfg_type, op, table_index))
          != kDalRcSuccess) {
        UPLL_LOG_ERROR("SetCfgTblDirtyInDB failed with rc=%d", dal_rc);
        return kDalRcGeneralError;
      }
    }
    create_dirty.insert(table_index);
    // In case of restore operation, update dirty should be set
    if (delete_dirty.find(table_index) != delete_dirty.end()) {
      if (update_dirty.end() == update_dirty.find(table_index)) {
        if ((dal_rc = SetCfgTblDirtyInDB(cfg_type, op, table_index))
            != kDalRcSuccess) {
          UPLL_LOG_ERROR("SetCfgTblDirtyInDB failed with rc=%d", dal_rc);
          return kDalRcGeneralError;
        }
      }
      update_dirty.insert(table_index);
    }
  } else if (op == UNC_OP_UPDATE) {
    if (update_dirty.end() == update_dirty.find(table_index)) {
      if ((dal_rc = SetCfgTblDirtyInDB(cfg_type, op, table_index))
          != kDalRcSuccess) {
        UPLL_LOG_ERROR("SetCfgTblDirtyInDB failed with rc=%d", dal_rc);
        return kDalRcGeneralError;
      }
      update_dirty.insert(table_index);
    }
  } else {
    // Not an issue
  }
  return kDalRcSuccess;
}

// update the dirty flag for given table name for create operation.
DalResultCode
DalOdbcMgr::SetCfgTblDirtyInDB(const UpllCfgType cfg_type,
                               const unc_keytype_operation_t op,
                               const DalTableIndex table_index) const {
  UPLL_FUNC_TRACE;
  SQLHANDLE     dal_stmt_handle = SQL_NULL_HANDLE;
  DalResultCode dal_rc;

  DalQueryBuilder  qbldr;
  std::string query_stmt;

  // Validating Inputs
  if (cfg_type != UPLL_DT_CANDIDATE) {
    UPLL_LOG_ERROR("Invalid config type - %d", cfg_type);
    return kDalRcGeneralError;
  }

  if (table_index >= schema::table::kDalNumTables) {
    UPLL_LOG_DEBUG("Invalid table index - %d", table_index);
    return kDalRcGeneralError;
  }

  std::string tbl_name = schema::TableName(table_index);
  using namespace unc::upll::dal::schema;

  DalBindInfo bind_info(table::kDbiCfgTblDirtyTbl);
  bind_info.BindMatch(table::cfg_tbl_dirty::kDbiTblName, kDalChar, 32, 
                      tbl_name.c_str());
  uint8_t op8 = op;  // bind 1 byte int
  bind_info.BindMatch(table::cfg_tbl_dirty::kDbiOperation, kDalUint8, 1, &op8);

  // Build Query Statement
  if (qbldr.get_sql_statement(kDalDirtyTblUpdateRecQT, &bind_info, query_stmt,
                              table::kDbiCfgTblDirtyTbl, cfg_type) != true) {
    UPLL_LOG_ERROR("Failed building query stmt");
    return kDalRcGeneralError;
  } else {
    UPLL_LOG_TRACE("Query stmt - %s", query_stmt.c_str());
  }

  // Allocate Stmt Handle, Bind and Execute the Query Statement
  dal_rc = ExecuteQuery(&dal_stmt_handle, &query_stmt, &bind_info);
  FreeHandle(SQL_HANDLE_STMT, dal_stmt_handle);
  if (dal_rc != kDalRcSuccess) {
    UPLL_LOG_ERROR("Err - %d. Failed executing query stmt - %s",
                   dal_rc, query_stmt.c_str());
    // Overwrite the error code to kDalRcGeneralError as it is DAL level error
    return kDalRcGeneralError;
  }

  UPLL_LOG_TRACE("Completed executing query stmt - %s",
                query_stmt.c_str());
  return kDalRcSuccess;
}  // DalOdbcMgr::SetCfgTblDirtyInDB

// Get all the dirty tables info
DalResultCode
DalOdbcMgr::UpdateDirtyTblCacheFromDB() const {
  UPLL_FUNC_TRACE;

  DalResultCode dal_rc;
  uint8_t dirty = 1;
  uint8_t op;

  // Remove current cache entries
  ClearDirtyTblCache();

  // Bind type, tbl_name, flag for output 
  using namespace unc::upll::dal::schema;
  //char tbl_name[table::cfg_tbl_dirty::max_cfg_tbl_name_len];
  char tbl_name[32];
  memset(tbl_name, 0, sizeof(tbl_name));
  DalBindInfo bind_info(table::kDbiCfgTblDirtyTbl);
  bind_info.BindOutput(table::cfg_tbl_dirty::kDbiTblName, kDalChar, 32,
                       tbl_name);
  bind_info.BindOutput(table::cfg_tbl_dirty::kDbiOperation, kDalUint8, 1, &op);
  bind_info.BindMatch(table::cfg_tbl_dirty::kDbiDirty, kDalUint8, 1, &dirty );

  // Get all the tables being dirty
  DalCursor *cursor = NULL;
  dal_rc = GetMultipleRecords(UPLL_DT_CANDIDATE, table::kDbiCfgTblDirtyTbl,
                              0, &bind_info, &cursor);  
  if (kDalRcSuccess != dal_rc) {
    if (kDalRcRecordNotFound == dal_rc) {
      UPLL_LOG_TRACE("No dirty records found in DB for op=%d", op);
      if (cursor != NULL) {
        delete cursor;
      }
      return kDalRcSuccess;
    } else {
      UPLL_LOG_ERROR("GetMultipleRecords failed with rc=%d", dal_rc);
      if (cursor != NULL) {
        delete cursor;
      }
      return dal_rc;
    }
  }
  
  // Traverse through each record and get type, tbl_name, op.
  while (dal_rc == kDalRcSuccess) {
    dal_rc = GetNextRecord(cursor);
    if (kDalRcSuccess != dal_rc) {
      if (kDalRcRecordNoMore == dal_rc) {
        dal_rc = kDalRcSuccess;
      } else {
        UPLL_LOG_ERROR("Reading DB failed with rc=%d", dal_rc);
      } 
      break;
    }
    UPLL_LOG_TRACE("Dirty table - tbl_name:%s, op:%d", tbl_name, op);
    dal_rc = UpdateDirtyTblCache(UPLL_DT_CANDIDATE, tbl_name,
                                 static_cast<unc_keytype_operation_t>(op));
    if (kDalRcSuccess != dal_rc) {
      break;
    }
  }
  if (cursor != NULL) {
    delete cursor;
  }
  return dal_rc;
}  // DalOdbcMgr::UpdateDirtyTblCacheFromDB
 
DalResultCode
DalOdbcMgr::UpdateDirtyTblCache(const UpllCfgType cfg_type,
                                const char *tbl_name,
                                const unc_keytype_operation_t op) const {
  UPLL_FUNC_TRACE;

  DalTableIndex tbl_idx;
  if (kDalRcSuccess != GetTableIndex(tbl_name, &tbl_idx)) {
    UPLL_LOG_ERROR("Unknown table %s", tbl_name);
    return kDalRcGeneralError;
  }
  if (cfg_type == UPLL_DT_CANDIDATE) {
    switch (op) {
      case UNC_OP_DELETE :
        delete_dirty.insert(tbl_idx);
        break;
      case UNC_OP_CREATE :
        create_dirty.insert(tbl_idx);
        break;
      case UNC_OP_UPDATE :
        update_dirty.insert(tbl_idx);
        break;
      default:
        UPLL_LOG_ERROR("Unexpected Operation  %d", op);
      return kDalRcGeneralError;
    }
  }
  return kDalRcSuccess;
}  // DalOdbcMgr::UpdateDirtyTblCache

// Caller should provide valid pointer for tbl_idx
DalResultCode
DalOdbcMgr::GetTableIndex(const char* tbl_name, DalTableIndex *tbl_idx) const {
  UPLL_FUNC_TRACE;
  std::map<std::string, DalTableIndex>::iterator it =
    tbl_name_to_idx_map_.find(tbl_name);
  if (tbl_name_to_idx_map_.end() == it) {
    UPLL_LOG_ERROR("Table %s does not exist", tbl_name);
    return kDalRcGeneralError;
  }
  *tbl_idx = it->second;
  UPLL_LOG_TRACE("table_name: %s, table_idex: %d", tbl_name, *tbl_idx);
  return kDalRcSuccess;
}  // DalOdbcMgr::GetTableIndex

DalResultCode
DalOdbcMgr::ClearAllDirtyTblInDB(UpllCfgType cfg_type) const {
  UPLL_FUNC_TRACE;
  SQLHANDLE     dal_stmt_handle = SQL_NULL_HANDLE;
  DalResultCode dal_rc;

  DalQueryBuilder  qbldr;
  std::string query_stmt;

  if (cfg_type != UPLL_DT_CANDIDATE) {
    return kDalRcGeneralError;
  }

  // Build Query Statement
  if (qbldr.get_sql_statement(kDalDirtyTblClearAllQT, NULL, query_stmt,
                              schema::table::kDbiCfgTblDirtyTbl,
                              cfg_type) != true) {
    UPLL_LOG_DEBUG("Failed building query stmt");
    return kDalRcGeneralError;
  } else {
    UPLL_LOG_TRACE("Query stmt - %s", query_stmt.c_str());
  }

  // Allocate Stmt Handle, Bind and Execute the Query Statement
  dal_rc = ExecuteQuery(&dal_stmt_handle, &query_stmt, NULL);

  FreeHandle(SQL_HANDLE_STMT, dal_stmt_handle);

  if (dal_rc != kDalRcSuccess) {
    if (dal_rc == kDalRcRecordNotFound) {
      UPLL_LOG_DEBUG("Record not found");
      dal_rc = kDalRcSuccess;
    } else {
      UPLL_LOG_ERROR("Err - %d. Failed executing query stmt - %s",
                     dal_rc, query_stmt.c_str());
      return dal_rc;
    }
  }
  UPLL_LOG_TRACE("Completed executing query stmt - %s",
                 query_stmt.c_str());
  return kDalRcSuccess;
}  // DalOdbcMgr::ClearAllDirtyTblInDB

}  // namespace dal
}  // namespace upll
}  // namespace unc
