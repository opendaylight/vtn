/*
 * Copyright (c) 2012-2014 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include "tc_db_handler.hh"

namespace unc {
namespace tc {


/*
 * @brief : Constructor
 */
TcDbHandler::TcDbHandler(std::string dsn_name)
        :dsn_name_(dsn_name),
         db_env_(NULL),
         db_conn_handle_(NULL) {
  if (TCOPER_RET_SUCCESS != OpenDBConnection()) {
      pfc_log_fatal("Creating Database Handler Failed!!");
  }
}


/*
 * @brief : Copy Constructor
 */
TcDbHandler::TcDbHandler(const TcDbHandler& db_hdlr)
  : dsn_name_(db_hdlr.dsn_name_),
    db_env_(NULL),  db_conn_handle_(NULL) {
      if (TCOPER_RET_SUCCESS != OpenDBConnection()) {
        pfc_log_fatal("Creating Database Handler Failed!!");
      }
}

/*!\brief method to get the error reason */
void
TcDbHandler::GetErrorReason(SQLRETURN sqlret,
                            SQLSMALLINT handletype,
                            SQLHANDLE handle) {
  char Db_SQL_status[10], Db_msg[200];
  SQLSMALLINT Db_msglen;
  SQLINTEGER Db_err;

  pfc_log_error("errno: %d", sqlret);

  if (SQL_SUCCESS == SQLGetDiagRec(handletype, handle, STATUS_RECORD,
                reinterpret_cast<SQLCHAR*> (Db_SQL_status), &Db_err,
                reinterpret_cast<SQLCHAR*> (Db_msg), BUFF_LEN, &Db_msglen)) {
    pfc_log_error("%s", Db_msg);
  } else {
    pfc_log_error("SQLGetDiagRec failed");
  }
}

/*!\brief Initializes the ODBC environment variables.
 * @result TCOPER_RET_FAILURE - ODBC connection failure.
 *         TCOPER_RET_SUCCESS - initialization success.
 * */
TcOperRet TcDbHandler::SetConnectionEnv() {
  pfc_log_debug("%s entry", __FUNCTION__);
  SQLRETURN SQL_ret;

  /*allocate environment handle*/
  SQL_ret = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &db_env_);
  if ((SQL_ret != SQL_SUCCESS) && (SQL_ret != SQL_SUCCESS_WITH_INFO)) {
    pfc_log_error("Allocating Env Handle failed: %d", SQL_ret);
    return TCOPER_RET_FAILURE;
  }
  pfc_log_debug("Env Handle allocated successfully");

  /*Set env attributes*/
  SQL_ret = SQLSetEnvAttr(db_env_, SQL_ATTR_ODBC_VERSION,
                          reinterpret_cast<void*> (SQL_OV_ODBC3), 0);
  if ((SQL_ret != SQL_SUCCESS) && (SQL_ret != SQL_SUCCESS_WITH_INFO)) {
    GetErrorReason(SQL_ret, SQL_HANDLE_ENV, db_env_);
    SQLFreeHandle(SQL_HANDLE_ENV, db_env_);
    return TCOPER_RET_FAILURE;
  }
  pfc_log_debug("Env Attributes set successfully");

  /*allocate ODBC handle*/
  SQL_ret = SQLAllocHandle(SQL_HANDLE_DBC, db_env_, &db_conn_handle_);
  if ((SQL_ret != SQL_SUCCESS) && (SQL_ret != SQL_SUCCESS_WITH_INFO)) {
    pfc_log_error("Allocating DBC Handle failed: %d", SQL_ret);
    SQLFreeHandle(SQL_HANDLE_ENV, db_env_);
    return TCOPER_RET_FAILURE;
  }
  pfc_log_debug("DBC Handle allocated successfully -- %s exit", __FUNCTION__);

  return TCOPER_RET_SUCCESS;
}
/*sets the connection string for SQLDriverConnect API*/
std::string TcDbHandler::GetDBConnectString() {
  std::string conn_str;
  /*append DSN name*/
  conn_str.append("DSN=");
  conn_str.append(dsn_name_);

  return conn_str;
}

/*for readable log*/
std::string TcDbHandler::ConvertAutoSaveString(pfc_bool_t autosave) {
  std::string ret_string;

  switch (autosave) {
    case PFC_TRUE:
      {
        ret_string="AUTOSAVE ENABLED";
        break;
      }
    case PFC_FALSE:
      {
        ret_string="AUTOSAVE DISABLED";
        break;
      }
    default:
      {
        ret_string="AUTOSAVE INVALID";
        break;
      }
  }
  return ret_string;
}

/*for readable log*/
std::string TcDbHandler::ConvertDbasetoString(SQLINTEGER dbase) {
  std::string ret_string;

  switch (dbase) {
    case UNC_DT_CANDIDATE:
      {
        ret_string="CANDIDATE DB";
        break;
      }
    case UNC_DT_RUNNING:
      {
        ret_string="RUNNING DB";
        break;
      }
    case UNC_DT_STARTUP:
      {
        ret_string="STARTUP DB";
        break;
      }
    default:
      {
        ret_string="INVALID DB";
        break;
      }
  }
  return ret_string;
}

/*for readable log*/
std::string TcDbHandler::ConvertOptoString(SQLINTEGER op) {
  std::string ret_string;

  switch (op) {
    case TC_OP_CANDIDATE_COMMIT:
      {
        ret_string="COMMIT OPERATION";
        break;
      }
    case TC_OP_CANDIDATE_ABORT:
      {
        ret_string="CANDIDATE ABORT OPERATION";
        break;
      }
    case TC_OP_RUNNING_SAVE:
      {
        ret_string="SAVE STARTUP";
        break;
      }
    case TC_OP_CLEAR_STARTUP:
      {
        ret_string="CLEAR STARTUP";
        break;
      }
    case TC_OP_USER_AUDIT:
      {
        ret_string="USER AUDIT";
        break;
      }
    case TC_OP_DRIVER_AUDIT:
      {
        ret_string="DRIVER AUDIT";
        break;
      }
    default:
      {
        ret_string="INVALID OP";
        break;
      }
  }
  return ret_string;
}


/*!\brief Initializes the ODBC Connection.
 * @result TCOPER_RET_FAILURE - ODBC connection failure.
 *         TCOPER_RET_SUCCESS - initialization success.
 * */
TcOperRet TcDbHandler::OpenDBConnection() {
  pfc_log_debug("%s entry", __FUNCTION__);
  SQLRETURN SQL_ret;

  /*set connection environment attributes*/
  if (TCOPER_RET_FAILURE == SetConnectionEnv()) {
    return TCOPER_RET_FAILURE;
  }
  /*connect to the datastore*/
  std::string conn_str = GetDBConnectString();

  SQL_ret = SQLDriverConnect(db_conn_handle_, NULL,
                             reinterpret_cast<SQLCHAR*>
                             (const_cast<char*> (conn_str.c_str())),
                             SQL_NTS, NULL, 0, NULL, SQL_DRIVER_NOPROMPT);
  if ((SQL_ret != SQL_SUCCESS) && (SQL_ret != SQL_SUCCESS_WITH_INFO)) {
    GetErrorReason(SQL_ret, SQL_HANDLE_DBC, db_conn_handle_);
    SQLFreeHandle(SQL_HANDLE_DBC, db_conn_handle_);
    SQLFreeHandle(SQL_HANDLE_ENV, db_env_);
    return TCOPER_RET_FAILURE;
  }
  pfc_log_info("DB connected successfully -- %s exit", __FUNCTION__);
  return TCOPER_RET_SUCCESS;
}

/*!\brief Close DB Connection.
 * @result TCOPER_RET_SUCCESS/TCOPER_RET_FAILURE
 * */
TcOperRet TcDbHandler::CloseDBConnection() {
  pfc_log_debug("%s entry", __FUNCTION__);
  SQLRETURN SQL_ret;

  if (db_conn_handle_) {
    SQL_ret = SQLDisconnect(db_conn_handle_);
    if (SQL_ret != SQL_SUCCESS) {
      GetErrorReason(SQL_ret, SQL_HANDLE_DBC, db_conn_handle_);
      return TCOPER_RET_FAILURE;
    }
    /*free DB handle*/
    SQL_ret = SQLFreeHandle(SQL_HANDLE_DBC, db_conn_handle_);
    if (SQL_ret != SQL_SUCCESS) {
      GetErrorReason(SQL_ret, SQL_HANDLE_DBC, db_conn_handle_);
      return TCOPER_RET_FAILURE;
    }
  }
  /*free environment handle*/
  if (db_env_) {
    SQL_ret = SQLFreeHandle(SQL_HANDLE_ENV, db_env_);
    if (SQL_ret != SQL_SUCCESS) {
      GetErrorReason(SQL_ret, SQL_HANDLE_ENV, db_env_);
      return TCOPER_RET_FAILURE;
    }
  }
  pfc_log_info("Disconnected DB connection");
  return TCOPER_RET_SUCCESS;
}

/*!\brief check for row existence
 * @param [in] */
TcOperRet TcDbHandler::IsRowExists(std::string table_name,
                                   std::string attribute) {
  pfc_log_debug("%s entry", __FUNCTION__);
  SQLRETURN SQL_ret;
  HSTMT hstmt_get;
  std::string get_query;
  SQLLEN iRow;
  SQLINTEGER query_length;

  /*create query to find if row is set with a value*/
  get_query.append("SELECT ");
  get_query.append(attribute);
  get_query.append(" FROM ");
  get_query.append(table_name);

  SQL_ret = SQLAllocHandle(SQL_HANDLE_STMT, db_conn_handle_, &hstmt_get);
  if ((SQL_ret != SQL_SUCCESS) && (SQL_ret != SQL_SUCCESS_WITH_INFO)) {
    pfc_log_error("Allocating statement handle failed: %d", SQL_ret);
    return TCOPER_RET_FATAL;
  }

  query_length = (SQLINTEGER)get_query.length();
  SQL_ret = SQLExecDirect(hstmt_get,
                          (unsigned char*) get_query.c_str(), query_length);
  if ((SQL_ret != SQL_SUCCESS) && (SQL_ret != SQL_SUCCESS_WITH_INFO)) {
    GetErrorReason(SQL_ret, SQL_HANDLE_STMT, hstmt_get);
    SQLFreeHandle(SQL_HANDLE_STMT, hstmt_get);
    return TCOPER_RET_FATAL;
  }
  /*get row count*/
  SQL_ret = SQLRowCount(hstmt_get, &iRow);
  if ((SQL_ret != SQL_SUCCESS) && (SQL_ret != SQL_SUCCESS_WITH_INFO)) {
    GetErrorReason(SQL_ret, SQL_HANDLE_STMT, hstmt_get);
    SQLFreeHandle(SQL_HANDLE_STMT, hstmt_get);
    return TCOPER_RET_FATAL;
  } else {
    if (iRow == 0) {
      pfc_log_info("Row does not exist");
      SQLFreeHandle(SQL_HANDLE_STMT, hstmt_get);
      return TCOPER_RET_FAILURE;
    }
  }
  /*clear all variables*/
  SQLFreeHandle(SQL_HANDLE_STMT, hstmt_get);
  get_query.clear();
  return TCOPER_RET_SUCCESS;
}

/*!\brief  Sets Default value to TC_UNC_CONF_TABLE
 * @result TCOPER_RET_FAILURE/TCOPER_RET_SUCCESS.
 * */
TcOperRet TcDbHandler::SetDefaultConfTable() {
  pfc_log_debug("%s entry", __FUNCTION__);
  SQLRETURN SQL_ret;
  std::string set_query;
  HSTMT hstmt_set;
  TcOperRet retval = TCOPER_RET_SUCCESS;
  SQLINTEGER query_length;

  retval = IsRowExists(SAVE_CONF_TABLE, "auto_save");
  /*returns FAILURE if row exists without a value*/
  if (retval != TCOPER_RET_FAILURE) {
    return retval;
  }
  /*set query*/
  set_query.append("INSERT INTO ");
  set_query.append(SAVE_CONF_TABLE);
  set_query.append("(auto_save,date_time)");
  set_query.append(" VALUES(false,CURRENT_TIMESTAMP);");
  /*allocate statement handle*/
  SQL_ret = SQLAllocHandle(SQL_HANDLE_STMT, db_conn_handle_, &hstmt_set);
  if ((SQL_ret != SQL_SUCCESS) && (SQL_ret != SQL_SUCCESS_WITH_INFO)) {
    pfc_log_error("Allocating statement handle  failed: %d", SQL_ret);
    return TCOPER_RET_FAILURE;
  }
  /*execute query*/
  query_length = set_query.length();
  SQL_ret = SQLExecDirect(hstmt_set,
                          (unsigned char*) set_query.c_str(), query_length);
  if ((SQL_ret != SQL_SUCCESS) && (SQL_ret != SQL_SUCCESS_WITH_INFO)) {
    GetErrorReason(SQL_ret, SQL_HANDLE_STMT, hstmt_set);
    SQLFreeHandle(SQL_HANDLE_STMT, hstmt_set);
    return TCOPER_RET_FAILURE;
  }
  /*clear all variables*/
  SQLFreeHandle(SQL_HANDLE_STMT, hstmt_set);
  set_query.clear();
  return TCOPER_RET_SUCCESS;
}

/*!\brief  Updates auto_save of TC_UNC_CONF_TABLE
 * @result TCOPER_RET_FAILURE/TCOPER_RET_SUCCESS.
 * */
TcOperRet TcDbHandler::UpdateConfTable(pfc_bool_t auto_save) {
  pfc_log_debug("%s entry", __FUNCTION__);
  SQLRETURN SQL_ret;
  TcOperRet retval = TCOPER_RET_SUCCESS;
  std::string upd_query;
  HSTMT hstmt_upd;
  SQLINTEGER query_length;
  /*validate the row*/
  retval = IsRowExists(SAVE_CONF_TABLE, "auto_save");
  if (retval != TCOPER_RET_SUCCESS) {
    return retval;
  }
  /*set query*/
  upd_query.append("UPDATE ");
  upd_query.append(SAVE_CONF_TABLE);
  upd_query.append(" SET auto_save='");
  if (auto_save == true) {
    upd_query.append("true");
  } else {
    upd_query.append("false");
  }
  upd_query.append("',date_time=CURRENT_TIMESTAMP WHERE auto_save >= '0';");
  /*allocate stmt handle*/
  SQL_ret = SQLAllocHandle(SQL_HANDLE_STMT, db_conn_handle_, &hstmt_upd);
  if ((SQL_ret != SQL_SUCCESS) && (SQL_ret != SQL_SUCCESS_WITH_INFO)) {
    pfc_log_error("Allocating statement handle failed: %d", SQL_ret);
    return TCOPER_RET_FAILURE;
  }
  /*execute query*/
  query_length = upd_query.length();
  SQL_ret = SQLExecDirect(hstmt_upd,
                          (unsigned char*) upd_query.c_str(), query_length);
  if ((SQL_ret != SQL_SUCCESS) && (SQL_ret != SQL_SUCCESS_WITH_INFO)) {
    GetErrorReason(SQL_ret, SQL_HANDLE_STMT, hstmt_upd);
    SQLFreeHandle(SQL_HANDLE_STMT, hstmt_upd);
    return TCOPER_RET_FAILURE;
  }
  /*clear all variables*/
  SQLFreeHandle(SQL_HANDLE_STMT, hstmt_upd);
  upd_query.clear();
  pfc_log_info("Updated auto-save in DB");
  return TCOPER_RET_SUCCESS;
}

/*!\brief Retrieves data forn save conf table
 * */
TcOperRet TcDbHandler::GetConfTable(pfc_bool_t* auto_save) {
  pfc_log_debug("%s entry", __FUNCTION__);
  SQLRETURN SQL_ret;
  HSTMT hstmt_get;
  std::string get_query;
  char ret_data[16];
  SQLINTEGER query_length;
  SQLLEN Db_err;
  TcOperRet retval = TCOPER_RET_SUCCESS;
  /*validate the row*/
  retval = IsRowExists(SAVE_CONF_TABLE, "auto_save");
  if (retval != TCOPER_RET_SUCCESS) {
    return retval;
  }
  /*set query*/
  get_query.append("SELECT auto_save FROM TC_UNC_CONF_TABLE");

  SQL_ret = SQLAllocHandle(SQL_HANDLE_STMT, db_conn_handle_, &hstmt_get);
  if ((SQL_ret != SQL_SUCCESS) && (SQL_ret != SQL_SUCCESS_WITH_INFO)) {
    pfc_log_error("Allocating statement handle failed: %d", SQL_ret);
    return TCOPER_RET_FAILURE;
  }

  /*
   * Bind "auto_save" column to retrieve data
   * Although PosqgreSQL ODBC driver can handle bool as SQL_C_BIT,
   * we treat it as SQL_C_CHAR to keep compatibility with other DBMS.
   */
  SQL_ret = SQLBindCol(hstmt_get, 1, SQL_C_CHAR, ret_data, sizeof(ret_data),
                       &Db_err);
  if ((SQL_ret != SQL_SUCCESS) && (SQL_ret != SQL_SUCCESS_WITH_INFO)) {
    GetErrorReason(SQL_ret, SQL_HANDLE_STMT, hstmt_get);
    return TCOPER_RET_FAILURE;
  }

  query_length = (SQLINTEGER)get_query.length();
  SQL_ret = SQLExecDirect(hstmt_get,
                          (unsigned char*) get_query.c_str(), query_length);
  if ((SQL_ret != SQL_SUCCESS) && (SQL_ret != SQL_SUCCESS_WITH_INFO)) {
    GetErrorReason(SQL_ret, SQL_HANDLE_STMT, hstmt_get);
    SQLFreeHandle(SQL_HANDLE_STMT, hstmt_get);
    return TCOPER_RET_FAILURE;
  }
  /*fetch data*/
  SQL_ret = SQLFetch(hstmt_get);
  if (SQL_ret == SQL_SUCCESS) {
    // We expect false is converted into "0" or "false".
    // NULL is treated as false.
    if (Db_err == SQL_NULL_DATA || ret_data[0] == '0' ||
        ret_data[0] == 'f' || ret_data[0] == 'F') {
      *auto_save = PFC_FALSE;
    } else {
      *auto_save = PFC_TRUE;
    }
  } else {
    GetErrorReason(SQL_ret, SQL_HANDLE_STMT, hstmt_get);
    SQLFreeHandle(SQL_HANDLE_STMT, hstmt_get);
    return TCOPER_RET_SUCCESS;
  }
  std::string autosave_state = ConvertAutoSaveString(*auto_save);
  pfc_log_info("auto-save = %s" , autosave_state.c_str());
  /*clear all variables*/
  SQLFreeHandle(SQL_HANDLE_STMT, hstmt_get);
  get_query.clear();
  return TCOPER_RET_SUCCESS;
}

/*!\brief set default values for recovery table
 * */
TcOperRet TcDbHandler::SetDefaultRecoveryTable() {
  pfc_log_debug("%s entry", __FUNCTION__);
  std::string set_query;
  HSTMT hstmt_set;
  SQLRETURN SQL_ret;
  unc_keytype_datatype_t database = UNC_DT_INVALID;
  TcServiceType oper = TC_OP_INVALID;
  int failover_instance = 0;
  char inttostr[4];
  TcOperRet retval = TCOPER_RET_SUCCESS;
  SQLINTEGER query_length;
  /*validate the row*/
  retval = IsRowExists(RECOVERY_TABLE, "database");
  /*returns FAILURE if row exists without a value*/
  if (retval != TCOPER_RET_FAILURE) {
    return retval;
  }
  /*set query*/
  set_query.append("INSERT INTO ");
  set_query.append(RECOVERY_TABLE);
  set_query.append("(database,operation,failover_instance,date_time)");
  set_query.append(" VALUES(");

  snprintf(inttostr, sizeof(database), "%d", database);
  set_query.append(inttostr);
  set_query.append(",");
  snprintf(inttostr, sizeof(oper), "%d", oper);
  set_query.append(inttostr);
  set_query.append(",");
  snprintf(inttostr, sizeof(oper), "%d", failover_instance);
  set_query.append(inttostr);
  set_query.append(",CURRENT_TIMESTAMP);");

  SQL_ret = SQLAllocHandle(SQL_HANDLE_STMT, db_conn_handle_, &hstmt_set);
  if ((SQL_ret != SQL_SUCCESS) && (SQL_ret != SQL_SUCCESS_WITH_INFO)) {
    pfc_log_error("Allocating statement handle failed: %d", SQL_ret);
    return TCOPER_RET_FAILURE;
  }

  query_length = (SQLINTEGER)set_query.length();
  SQL_ret = SQLExecDirect(hstmt_set,
                          (unsigned char*) set_query.c_str(), query_length);
  if ((SQL_ret != SQL_SUCCESS) && (SQL_ret != SQL_SUCCESS_WITH_INFO)) {
    GetErrorReason(SQL_ret, SQL_HANDLE_STMT, hstmt_set);
    SQLFreeHandle(SQL_HANDLE_STMT, hstmt_set);
    return TCOPER_RET_SUCCESS;
  }
  /*clear all variables*/
  SQLFreeHandle(SQL_HANDLE_STMT, hstmt_set);
  set_query.clear();
  return TCOPER_RET_SUCCESS;
}

/*!\brief updates recovery table
 * */
TcOperRet TcDbHandler::UpdateRecoveryTable(unc_keytype_datatype_t data_base,
                                           TcServiceType operation,
                                           uint32_t failover_instance) {
  pfc_log_debug("%s entry", __FUNCTION__);
  SQLRETURN SQL_ret;
  std::string upd_query;
  HSTMT hstmt_upd;
  char inttostr[4];
  TcOperRet retval = TCOPER_RET_SUCCESS;
  SQLINTEGER query_length;
  /*validate the row*/
  retval = IsRowExists(RECOVERY_TABLE, "database");
  if (retval != TCOPER_RET_SUCCESS) {
    return retval;
  }
  /* check if the inputs are valid */
  if (TCOPER_RET_SUCCESS !=
      TcMsg::ValidateAuditDBAttributes(data_base, operation)) {
    pfc_log_error("Audit DB attributes mismatch");
    return TCOPER_RET_FAILURE;
  }

  /* create query */
  upd_query.append("UPDATE TC_RECOVERY_TABLE SET database=");
  snprintf(inttostr, sizeof(data_base), "%d", data_base);
  upd_query.append(inttostr);
  snprintf(inttostr, sizeof(operation), "%d", operation);
  upd_query.append(",operation=");
  upd_query.append(inttostr);
  snprintf(inttostr, sizeof(operation), "%d", failover_instance);
  upd_query.append(",failover_instance=");
  upd_query.append(inttostr);
  upd_query.append(",date_time=CURRENT_TIMESTAMP WHERE database >= 0;");

  SQL_ret = SQLAllocHandle(SQL_HANDLE_STMT, db_conn_handle_, &hstmt_upd);
  if ((SQL_ret != SQL_SUCCESS) && (SQL_ret != SQL_SUCCESS_WITH_INFO)) {
    pfc_log_error("Allocating statement handle failed: %d", SQL_ret);
    return TCOPER_RET_FAILURE;
  }

  query_length = upd_query.length();
  SQL_ret = SQLExecDirect(hstmt_upd,
                          (unsigned char*) upd_query.c_str(), query_length);
  if ((SQL_ret != SQL_SUCCESS) && (SQL_ret != SQL_SUCCESS_WITH_INFO)) {
    GetErrorReason(SQL_ret, SQL_HANDLE_STMT, hstmt_upd);
    SQLFreeHandle(SQL_HANDLE_STMT, hstmt_upd);
    return TCOPER_RET_FAILURE;
  }
  /* clear all variables */
  pfc_log_info("updated db:%d fail_oper:%d in DB", data_base, operation);
  SQLFreeHandle(SQL_HANDLE_STMT, hstmt_upd);
  upd_query.clear();
  return TCOPER_RET_SUCCESS;
}

/*!\brief retrieves data from recovery table
 * */
TcOperRet TcDbHandler::GetRecoveryTable(unc_keytype_datatype_t* db,
                                        TcServiceType* oper,
                                        uint32_t* failover_instance) {
  pfc_log_debug("%s entry", __FUNCTION__);
  SQLRETURN SQL_ret;
  HSTMT hstmt_get;
  std::string get_query;
  SQLINTEGER dbase, op, fail_inst;
  SQLINTEGER query_length;
  SQLLEN Db_err[3];
  TcOperRet retval = TCOPER_RET_SUCCESS;
  /*validate the row*/
  retval = IsRowExists(RECOVERY_TABLE, "database");
  if (retval != TCOPER_RET_SUCCESS) {
    return retval;
  }
  /*set query*/
  get_query.append("SELECT database,operation,failover_instance ");
  get_query.append("FROM TC_RECOVERY_TABLE");

  SQL_ret = SQLAllocHandle(SQL_HANDLE_STMT, db_conn_handle_, &hstmt_get);
  if ((SQL_ret != SQL_SUCCESS) && (SQL_ret != SQL_SUCCESS_WITH_INFO)) {
    pfc_log_error("Allocating statement handle failed: %d", SQL_ret);
    return TCOPER_RET_FAILURE;
  }
  /*bind variables to fetch data from DB*/
  SQL_ret = SQLBindCol(hstmt_get, 1, SQL_C_LONG, &dbase, 5, &Db_err[0]);
  if ((SQL_ret != SQL_SUCCESS) && (SQL_ret != SQL_SUCCESS_WITH_INFO)) {
    GetErrorReason(SQL_ret, SQL_HANDLE_STMT, hstmt_get);
    return TCOPER_RET_FAILURE;
  }

  SQL_ret = SQLBindCol(hstmt_get, 2, SQL_C_LONG, &op, 5, &Db_err[1]);
  if ((SQL_ret != SQL_SUCCESS) && (SQL_ret != SQL_SUCCESS_WITH_INFO)) {
    GetErrorReason(SQL_ret, SQL_HANDLE_STMT, hstmt_get);
    return TCOPER_RET_FAILURE;
  }
  SQL_ret = SQLBindCol(hstmt_get, 3, SQL_C_LONG, &fail_inst, 5, &Db_err[2]);
  if ((SQL_ret != SQL_SUCCESS) && (SQL_ret != SQL_SUCCESS_WITH_INFO)) {
    GetErrorReason(SQL_ret, SQL_HANDLE_STMT, hstmt_get);
    return TCOPER_RET_FAILURE;
  }

  query_length = (SQLINTEGER)get_query.length();
  SQL_ret = SQLExecDirect(hstmt_get,
                          (unsigned char*) get_query.c_str(), query_length);
  if ((SQL_ret != SQL_SUCCESS) && (SQL_ret != SQL_SUCCESS_WITH_INFO)) {
    GetErrorReason(SQL_ret, SQL_HANDLE_STMT, hstmt_get);
    SQLFreeHandle(SQL_HANDLE_STMT, hstmt_get);
    return TCOPER_RET_FAILURE;
  }
  /*fetch data*/
  SQL_ret = SQLFetch(hstmt_get);
  if (SQL_ret != SQL_NO_DATA) {
    *db = (Db_err[0] == SQL_NULL_DATA)
      ? UNC_DT_INVALID : (unc_keytype_datatype_t)dbase;
    *oper = (Db_err[1] == SQL_NULL_DATA) ? TC_OP_INVALID : (TcServiceType)op;
    *failover_instance = (Db_err[2] == SQL_NULL_DATA)
      ? 1U : (uint32_t)fail_inst;
  }
  std::string op_string = ConvertOptoString(*oper);
  std::string dbase_string = ConvertDbasetoString(*db);
  pfc_log_info("db = %s; fail_oper = %s, fail_instance = %d",
               dbase_string.c_str(), op_string.c_str(), *failover_instance);
  /* clear all variables */
  SQLFreeHandle(SQL_HANDLE_STMT, hstmt_get);
  get_query.clear();
  return TCOPER_RET_SUCCESS;
}

/*!\brief Initializes Tc Db tables
 * */
TcOperRet TcDbHandler::InitTcDbTables() {
  pfc_log_debug("%s entry", __FUNCTION__);

  /*set default values to TC Conf table*/
  if (TCOPER_RET_SUCCESS == SetDefaultConfTable()) {
    pfc_log_info("TC_UNC_CONF_TABLE is set");
  } else {
    pfc_log_error("Initializing TC_UNC_SAVE_CONF_TABLE failed");
    return TCOPER_RET_FAILURE;
  }
  /*set default values to TC Recovery table*/
  if (TCOPER_RET_SUCCESS == SetDefaultRecoveryTable()) {
    pfc_log_info("TC_RECOVERY_TABLE is set");
  } else {
    pfc_log_error("Initializing RECOVERY_TABLE failed");
    return TCOPER_RET_FAILURE;
  }
  return TCOPER_RET_SUCCESS;
}

/*!\brief Initializes TC DB*/
TcOperRet TcDbHandler::InitDB() {
  /*set default values to TC DB tables*/
  return (InitTcDbTables());
}

}  // namespace tc
}  // namespace unc
