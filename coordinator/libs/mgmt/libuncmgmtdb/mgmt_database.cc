/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include "mgmt_database.hh"

namespace unc {
namespace mgmtdb {

#define CLASS_NAME "MgmtDatabase"

// -------------------------------------------------------------
//  Class method definitions.
// -------------------------------------------------------------
/*
 * @brief   Constructor.
 * @param   database  : [IN] database instance.
 * @return  nothing.
 * @note    
 */
MgmtDatabase::MgmtDatabase(void)
{
  // ODBC env handle.
  henv_ = NULL;
  // ODBC conection handle.
  hdbc_ = NULL;
  // transaction status.
  status_database_ = STATUS_TRANSACTION_NONE;
}


/*
 * @brief   Destructor.
 * @param   nothing.
 * @return  nothing.
 * @note    
 */
MgmtDatabase::~MgmtDatabase(void)
{
}


/*
 * @brief   Initialization.
 * @param   nothing.
 * @return  true  : success
 *          false : failure
 * @note    
 */
bool MgmtDatabase::Init(void)
{
  RETCODE db_rtn = SQL_ERROR;


  L_FUNCTION_START();

  // initial transaction status.
  status_database_ = STATUS_TRANSACTION_NONE;

  // ODBC environment alloc.
  db_rtn = SQLAllocEnv(&henv_);
  RETURN_IF_ODBC(henv_, SQL_NULL_HDBC, SQL_NULL_HSTMT,
      (db_rtn != SQL_SUCCESS), false);

  // ODBC conect alloc.
  db_rtn = SQLAllocConnect(henv_, &hdbc_);
  GOTO_IF_ODBC(henv_, SQL_NULL_HDBC, SQL_NULL_HSTMT,
      (db_rtn != SQL_SUCCESS), err_end);

  L_FUNCTION_COMPLETE();
  return true;

err_end:
  SQLFreeEnv(henv_);
  henv_ = NULL;
  hdbc_ = NULL;
  return false;
}


/*
 * @brief   Finalization.
 * @param   nothing.
 * @return  true  : success
 *          false : failure
 * @note    
 */
bool MgmtDatabase::Fini(void)
{
  L_FUNCTION_START();

  Disconnect();

  // ODBC connect free.
  SQLFreeConnect(hdbc_);
  hdbc_ = NULL;

  // ODBC environment free.
  SQLFreeEnv(henv_);
  henv_ = NULL;

  L_FUNCTION_COMPLETE();
  return true;
}


/*
 * @brief   Database connect.
 * @param   nothing.
 * @return  DB_E_OK                : Connect success.
 *          DB_E_NG                : Failure.
 * @note    
 */
db_err_e MgmtDatabase::Connect(void)
{
  RETCODE db_rtn = SQL_ERROR;
  db_err_e func_rtn = DB_E_NG;


  L_FUNCTION_START();


  RETURN_IF((status_database_ != STATUS_TRANSACTION_NONE), DB_E_CONNECT_ERR,
    "Invalid transaction status. [%d].", status_database_);

  // database connect.
  db_rtn = SQLDriverConnect(hdbc_, NULL, (SQLCHAR*)MGMTDB_CONNECT_STRING,
              SQL_NTS, NULL, 0, NULL, SQL_DRIVER_NOPROMPT);
  GOTO_IF_ODBC(henv_, hdbc_, SQL_NULL_HSTMT,
      (db_rtn != SQL_SUCCESS && db_rtn != SQL_SUCCESS_WITH_INFO), err_end);

  // auto commit off.
  db_rtn = SQLSetConnectOption(hdbc_, SQL_AUTOCOMMIT, SQL_AUTOCOMMIT_OFF);
  GOTO_IF_ODBC(henv_, hdbc_, SQL_NULL_HSTMT,
      (db_rtn != SQL_SUCCESS && db_rtn != SQL_SUCCESS_WITH_INFO), err_end);

  // begin transaction.
  func_rtn = BeginTransaction();
  if (func_rtn != DB_E_OK) {
    goto err_end;
  }

  L_FUNCTION_COMPLETE();
  return DB_E_OK;

err_end:
  SQLDisconnect(hdbc_);
  return DB_E_CONNECT_ERR;
}


/*
 * @brief   Database disconnect.
 * @param   nothing.
 * @return  DB_E_OK          : Disconnect success.
 *          DB_E_CONNECT_ERR : Not connected.
 *          DB_E_NG          : Failure.
 * @note    
 */
db_err_e MgmtDatabase::Disconnect(void)
{
  db_err_e func_rtn = DB_E_NG;
  RETCODE db_rtn = SQL_ERROR;
  db_commit_e mode;


  L_FUNCTION_START();

  // check to transaction status.
  if (status_database_ == STATUS_TRANSACTION_NONE) {
    return DB_E_TRANSACTION_ERR;
  }

  if (status_database_ == STATUS_TRANSACTION_UPDATE) {
    mode = DB_COMMIT;
  } else {
    mode = DB_ROLLBACK;
  }

  func_rtn = EndTransaction(mode);
  if (func_rtn != DB_E_OK) L_ERROR("%s", "Failure transaction commit.");

  // disconnect.
  db_rtn = SQLDisconnect(hdbc_);
  if (db_rtn == SQL_SUCCESS_WITH_INFO)
  {
    L_INFO("Info ODBC database disconnect. result=%d", db_rtn);
  }
  RETURN_IF((db_rtn != SQL_SUCCESS && db_rtn != SQL_SUCCESS_WITH_INFO), DB_E_NG,
    "Failure ODBC database disconnect. err=%d", db_rtn);

  status_database_ = STATUS_TRANSACTION_NONE;

  L_FUNCTION_COMPLETE();
  return func_rtn;
}


/*
 * @brief   SQL execution.
 * @param   sql_statement : [IN]  SQL statement.
 *          update_flag   : [IN]  update exec flag.
 *          fetch_cnt     : [IN]  fetch data count.
 *          fetch_type    : [IN]  fetch data type.
 *          exec_value    : [OUT] extraction data.
 * @return  DB_E_OK  : Disconnect success.
 *          DB_E_NG  : Failure.
 * @note    
 */
db_err_e MgmtDatabase::Exec(const std::string& sql_statement,
      const bool update_flag, const int32_t fetch_cnt,
      const int16_t fetch_type[], mgmtdb_variant_v& exec_value)
{
  db_err_e err_code = DB_E_NG;
  SQLRETURN db_rtn;
  // statement handle.
  SQLHSTMT hstmt;


  L_FUNCTION_START();

  // check to transaction status.
  RETURN_IF((status_database_ == STATUS_TRANSACTION_NONE),
    DB_E_TRANSACTION_ERR,
    "Invalid transaction status. [%d].", status_database_);

  // check parameter
  RETURN_IF((sql_statement.empty()), DB_E_PARA_ERR,
    "%s", "Invalid parameter. sql statement");

  // ODBC statement alloc.
  SQLAllocStmt(hdbc_, &hstmt);
  RETURN_IF((hstmt == NULL), DB_E_FATAL_ERR,
    "%s", "Failure ODBC statement allocation");

  // sql exec
  exec_value.clear();
  db_rtn = SQLExecDirect(hstmt, (SQLCHAR*)sql_statement.c_str(), SQL_NTS);
  GOTO_IF_ODBC(henv_, hdbc_, hstmt,
      (db_rtn != SQL_SUCCESS && db_rtn != SQL_SUCCESS_WITH_INFO), err_end);

  if (fetch_cnt != 0) {
    err_code = DbField2Value(hstmt, fetch_cnt, fetch_type, exec_value);
    if (err_code != DB_E_OK) {
      goto err_end;
    }
  }

  // update transaction mode.
  if (update_flag == true && status_database_ != STATUS_TRANSACTION_ABORT) {
    status_database_ = STATUS_TRANSACTION_UPDATE;
  }

  // ODBC statement free.
  SQLFreeStmt(hstmt, SQL_DROP);
  L_FUNCTION_COMPLETE();
  return DB_E_OK;

err_end:
  // ODBC statement free.
  SQLFreeStmt(hstmt, SQL_DROP);
  return err_code;
}


/*
 * @brief   Abort deterministic.
 * @param   nothing.
 * @return  DB_E_OK               : Success.
 *          DB_E_TRANSACTION_ERR  : Transaction error.
 *          USESS_E_NG            : Error
 * @note    
 */
db_err_e MgmtDatabase::Abort(void)
{
  L_FUNCTION_START();

  // check to transaction status.
  RETURN_IF((status_database_ == STATUS_TRANSACTION_NONE),
    DB_E_TRANSACTION_ERR,
    "Invalid transaction status. [%d].", status_database_);

  status_database_ = STATUS_TRANSACTION_ABORT;

  L_FUNCTION_COMPLETE();
  return DB_E_OK;
}


/*
 * @brief   Begin transaction.
 * @param   nothing.
 * @return  USESS_E_OK            : Success
 *          DB_E_TRANSACTION_ERR  : Transaction error.
 *          USESS_E_NG            : Error
 * @note    
 */
db_err_e MgmtDatabase::BeginTransaction(void)
{
  L_FUNCTION_START();

  // check to transaction status.
  RETURN_IF((status_database_ != STATUS_TRANSACTION_NONE),
    DB_E_TRANSACTION_ERR,
    "Invalid transaction status. [%d].", status_database_);

  status_database_ = STATUS_TRANSACTION_BEGIN;

  L_FUNCTION_COMPLETE();
  return DB_E_OK;
}


/*
 * @brief   End transaction.
 * @param   mode    : [IN] commit mode.
 * @return  USESS_E_OK            : Success
 *          DB_E_TRANSACTION_ERR  : Transaction error.
 *          USESS_E_NG            : Error
 * @note    
 */
db_err_e MgmtDatabase::EndTransaction(const db_commit_e mode)
{
  SQLSMALLINT commit_mode = SQL_ROLLBACK;
  SQLRETURN db_rtn = SQL_ERROR;


  L_FUNCTION_START();

  // check to transaction status.
  RETURN_IF((status_database_ == STATUS_TRANSACTION_NONE),
    DB_E_TRANSACTION_ERR,
    "Invalid transaction status. [%d].", status_database_);

  // check parameter.
  if (mode == DB_COMMIT) {
    commit_mode = SQL_COMMIT;
  } else if (mode == DB_ROLLBACK) {
    commit_mode = SQL_ROLLBACK;
  } else {
    L_ERROR("Invalid parameter. commit mode=[%d]", mode);
    return DB_E_PARA_ERR;
  }

  if (status_database_ == STATUS_TRANSACTION_ABORT) {
    commit_mode = SQL_ROLLBACK;
  }

  // end transaction.
  db_rtn = SQLTransact(henv_, hdbc_, commit_mode);
  RETURN_IF_ODBC(henv_, hdbc_, SQL_NULL_HSTMT,
      (db_rtn != SQL_SUCCESS), DB_E_TRANSACTION_ERR);

  // update transaction mode.
  if (status_database_ != STATUS_TRANSACTION_ABORT) {
    status_database_ = STATUS_TRANSACTION_NONE;
  }

  L_FUNCTION_COMPLETE();
  return DB_E_OK;
}


/*
 * @brief   Database data extraction.
 * @param   hstmt      : [IN]  statement handler.
 *          fetch_cnt  : [IN]  fetch data count.
 *          fetch_type : [IN]  fetch data type.
 *          exec_value : [OUT] extraction data.
 * @return  DB_E_OK   : Success
 *          DB_E_NG   : Error
 * @note    
 */
db_err_e MgmtDatabase::DbField2Value(SQLHSTMT hstmt, const int32_t fetch_cnt,
                const int16_t fetch_type[], mgmtdb_variant_v& exec_value)
{
  RETCODE db_rtn = SQL_ERROR;
  SQLLEN row_count = 0;
  variant_t get_data[fetch_cnt];


  L_FUNCTION_START();

  // /////////////////////////////////////////////////////
  // collect infomation.
  // /////////////////////////////////////////////////////
  // get line count.
  db_rtn = SQLRowCount(hstmt, &row_count);
  RETURN_IF_ODBC(henv_, hdbc_, hstmt, (db_rtn != SQL_SUCCESS), DB_E_NG);

  // out data area allocation
  exec_value.resize(row_count);
  for (int row = 0; row < row_count; ++row) {
    exec_value[row].resize(fetch_cnt);
  }

  // /////////////////////////////////////////////////////
  // data extraction.
  // /////////////////////////////////////////////////////

  // column bind.
  for (int column = 0; column < fetch_cnt; ++column) {
    db_rtn = SetBindCol(hstmt, column + 1, fetch_type[column], get_data[column]);
    RETURN_IF_ODBC(henv_, hdbc_, hstmt, (db_rtn != SQL_SUCCESS), DB_E_NG);
  }

  // line fetch.
  for (int row = 0; row < row_count; ++row) {
    db_rtn = SQLFetch(hstmt);
    RETURN_IF_ODBC(henv_, hdbc_, hstmt,
        (db_rtn != SQL_SUCCESS && db_rtn != SQL_SUCCESS_WITH_INFO), DB_E_NG);

    if(db_rtn == SQL_NO_DATA_FOUND) {
      break;
    }

    for (int column = 0; column < fetch_cnt; ++column) {
      if (get_data[column].val_type == VARIANT_STRING) {
        exec_value[row][column].val_type = get_data[column].val_type;
        exec_value[row][column].string_val(get_data[column].string_val());
      } else {
        exec_value[row][column] = get_data[column];
      }
    }
  }

  L_FUNCTION_COMPLETE();
  return DB_E_OK;
}


/*
 * @brief   Fatch data column bind.
 * @param   hstmt      : [IN]  statement handler.
 *          colum      : [IN]  column number.
 *          fetch_type : [IN]  fetch data type.
 *          get_data   : [OUT] extraction data.
 * @return  DB_E_OK   : Success
 *          DB_E_NG   : Error
 * @note    
 */
db_err_e MgmtDatabase::SetBindCol(SQLHSTMT hstmt, const int column,
        const int16_t fetch_type, variant_t &get_data)
{
  // /////////////////////////////////////////////////////
  // set data value.
  // /////////////////////////////////////////////////////
  switch (fetch_type) {
  case SQL_VARCHAR:
    get_data.new_string(USESS_DB_TEXT_SIZE);
    SQLBindCol(hstmt, column, SQL_C_CHAR, get_data.string_val(),
        USESS_DB_TEXT_SIZE, &get_data.string_len);
    get_data.val_type = VARIANT_STRING;
    break;

  case SQL_INTEGER:
    SQLBindCol(hstmt, column, SQL_C_LONG, &get_data.u_val.v_int32, 0, NULL);
    get_data.val_type = VARIANT_INT32;
    break;

  case SQL_SMALLINT:
    SQLBindCol(hstmt, column, SQL_C_SHORT, &get_data.u_val.v_int16, 0, NULL);
    get_data.val_type = VARIANT_INT16;
    break;

  case SQL_BIGINT:
    SQLBindCol(hstmt, column, SQL_C_SBIGINT, &get_data.u_val.v_int64, 0, NULL);
    get_data.val_type = VARIANT_INT64;
    break;

  case SQL_REAL:    /* FALLTHROUGH */
  case SQL_FLOAT:
    SQLBindCol(hstmt, column, SQL_C_FLOAT, &get_data.u_val.v_float, 0, NULL);
    get_data.val_type = VARIANT_FLOAT;
    break;

  case SQL_DOUBLE:
    SQLBindCol(hstmt, column, SQL_C_DOUBLE, &get_data.u_val.v_double, 0, NULL);
    get_data.val_type = VARIANT_DOUBLE;
    break;

  case SQL_TYPE_DATE:
    SQLBindCol(hstmt, column, SQL_C_DATE, &get_data.u_val.v_date, 0, NULL);
    get_data.val_type = VARIANT_DATE;
    break;

  case SQL_TYPE_TIME:
    SQLBindCol(hstmt, column, SQL_C_TIME, &get_data.u_val.v_time, 0, NULL);
    get_data.val_type = VARIANT_TIME;
    break;

  case SQL_TYPE_TIMESTAMP:
    SQLBindCol(hstmt, column, SQL_C_TIMESTAMP,
          &get_data.u_val.v_timestamp, 0, NULL);
    get_data.val_type = VARIANT_TIMESTAMP;
    break;

  default:
    return DB_E_NG;
    break;
 }

  return DB_E_OK;
}

/*
 * @brief   Database error code convert.
 * @param   err_code     : [IN] Database error code.
 * @return  mgmtdb db error code.
 * @note    
 */
db_err_e MgmtDatabase::DbErr2UsessErr(const RETCODE err_code) const
{
  db_err_e rtn_code = DB_E_NG;


  switch (err_code) {
  case SQL_SUCCESS:             /* FALLTHROUGH */
  case SQL_SUCCESS_WITH_INFO:
    rtn_code = DB_E_OK;
    break;

  case SQL_ERROR:
    rtn_code = DB_E_NG;
    break;

  case SQL_INVALID_HANDLE:
    rtn_code = DB_E_FATAL_ERR;
    break;

  case SQL_NO_DATA_FOUND:
    rtn_code = DB_E_NOTFOUND;
    break;

  default:
    rtn_code = DB_E_NG;
    break;
  }

  return rtn_code;
}

}  // namespace mgmtdb
}  // namespace unc



using namespace unc::mgmtdb;

/*
 * @brief   DB class create.
 * @param   nothing.
 * @return  class object.
 * @note    
 */
void* mgmtdb_create(void)
{
  bool ret = false;


  MgmtDatabase* cls_obj = new MgmtDatabase();
  if (cls_obj == NULL) {
    return NULL;
  }

  ret = cls_obj->Init();
  if (ret) {
    return static_cast<void*>(cls_obj);
  } else {
    cls_obj->Fini();
    delete cls_obj;
    return NULL;
  }
}


/*
 * @brief   DB class destruct.
 * @param   db_object : [IN] class object.
 * @return  nothing.
 * @note    
 */
void mgmtdb_delete(void* db_object)
{
  if (db_object == NULL) {
    return;
  }

  MgmtDatabase* cls_obj =
      static_cast<MgmtDatabase*>(db_object);

  delete cls_obj;
  return;
}


/*
 * @brief   DB connect.
 * @param   db_object : [IN] class object.
 * @return   0 : Success.
 *          -1 : Failure.
 * @note    
 */
int mgmtdb_connect(void* db_object)
{
  db_err_e ret;

  if (db_object == NULL) {
    return -1;
  }

  MgmtDatabase* cls_obj =
      static_cast<MgmtDatabase*>(db_object);

  ret = cls_obj->Connect();
  return (ret == DB_E_OK) ? 0 : -1;
}


/*
 * @brief   DB disconnect.
 * @param   db_object : [IN] class object.
 * @return   0 : Success.
 *          -1 : Failure.
 * @note    
 */
int mgmtdb_disconnect(void* db_object)
{
  if (db_object == NULL) {
    return -1;
  }


  MgmtDatabase* cls_obj =
      static_cast<MgmtDatabase*>(db_object);

  cls_obj->Disconnect();
  return 0;
}


/*
 * @brief   sql statement execute.
 * @param   db_object : [IN]  class object.
 *          exec      : [IN]  sql statement.
 *          update    : [IN]  update mode.
 *          data      : [OUT] extraction data.
 * @return   0 : Success.
 *          -1 : Failure.
 * @note    
 */
int mgmtdb_one_execute(void* db_object, char* sql_statement, bool update,
        char* data_type, void* data)
{
  int16_t fetch_type[1] = {SQL_UNKNOWN_TYPE};
  int32_t fech_cnt = 0;
  mgmtdb_variant_v exec_value;
  db_err_e ret;

  if (db_object == NULL) {
    return -1;
  }

  MgmtDatabase* cls_obj =
      static_cast<MgmtDatabase*>(db_object);

  if (strcmp(data_type, "NUMBER") == 0) {
    fetch_type[0] = SQL_INTEGER;
    fech_cnt = 1;
  } else if (strcmp(data_type, "TEXT") == 0) {
    fetch_type[0] = SQL_VARCHAR;
    fech_cnt = 1;
  } else {
    fetch_type[0] = SQL_UNKNOWN_TYPE;
    fech_cnt = 0;
  }

  ret = cls_obj->Exec(sql_statement, update, fech_cnt,
          fetch_type, exec_value);

  if (ret == DB_E_OK && fetch_type[0] != SQL_UNKNOWN_TYPE) {
    if (exec_value.size() != 1 || exec_value[0].size() != 1) {
      return -1;
    } else {
      if (fetch_type[0] == SQL_INTEGER) {
        *((int *)(data)) = exec_value[0][0].u_val.v_int32;
      } else {
        strcpy((char *)data, exec_value[0][0].string_val());
      }
    }
  }

  return (ret == DB_E_OK) ? 0 : -1;
}


/*
 * @brief   DB update abort.
 * @param   db_object : [IN]  class object.
 * @return   0 : Success.
 *          -1 : Failure.
 * @note    
 */
int mgmtdb_abort(void* db_object)
{
  db_err_e ret;

  if (db_object == NULL) {
    return -1;
  }

  MgmtDatabase* cls_obj =
      static_cast<MgmtDatabase*>(db_object);

  ret = cls_obj->Abort();
  return (ret == DB_E_OK) ? 0 : -1;
}


