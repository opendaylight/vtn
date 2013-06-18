/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef _USESS_DATABASE_HH_
#define _USESS_DATABASE_HH_


#include <string.h>
#include <string>
#include <map>
#include <vector>
#include <stdint.h>

#include <sql.h>
#include <sqlext.h>

#include <unc/base.h>
#include <pfc/log.h>


namespace unc {
namespace mgmtdb {

// database text column max size.
#define USESS_DB_TEXT_SIZE  (32 * 1024)
#define MGMTDB_CONNECT_STRING "DSN=UNC_DB_DSN"

// ---------------------------------------------------------------------
// definition of macro.
// ---------------------------------------------------------------------

// log output
// Note: "CLASS_NAME" the declaration at the top of the class for each source.
#define L_FATAL(format, ...) \
  pfc_log_fatal("%s.%s:" format, CLASS_NAME, __FUNCTION__, __VA_ARGS__)
#define L_ERROR(format, ...) \
  pfc_log_error("%s.%s:" format, CLASS_NAME, __FUNCTION__, __VA_ARGS__)
#define L_WARN(format, ...) \
  pfc_log_warn("%s.%s:" format, CLASS_NAME, __FUNCTION__, __VA_ARGS__)
#define L_INFO(format, ...) \
  pfc_log_info("%s.%s:" format, CLASS_NAME, __FUNCTION__, __VA_ARGS__)
#define L_DEBUG(format, ...) \
  pfc_log_debug("%s.%s:" format, CLASS_NAME, __FUNCTION__, __VA_ARGS__)
#define L_TRACE(format, ...) \
  pfc_log_trace("%s.%s:" format, CLASS_NAME, __FUNCTION__, __VA_ARGS__)
#define L_VERBOSE(format, ...) \
  pfc_log_verbose("%s.%s:" format, CLASS_NAME, __FUNCTION__, __VA_ARGS__)

// function start/completed log.
#define L_FUNCTION_START() \
  pfc_log_verbose("%s.%s:start", CLASS_NAME, __FUNCTION__)
#define L_FUNCTION_COMPLETE() \
  pfc_log_verbose("%s.%s:completed", CLASS_NAME, __FUNCTION__)

// ///////////////////////////////////////////////////
// error check & error message output.
// ///////////////////////////////////////////////////
#define GOTO_IF(CONDITIONS, LABEL, FMT, ...) \
  if (CONDITIONS) { \
    L_ERROR(FMT, __VA_ARGS__); \
    goto LABEL; \
  }

#define RETURN_IF(CONDITIONS, RET, FMT, ...) \
  if (CONDITIONS) { \
    L_ERROR(FMT, __VA_ARGS__); \
    return RET; \
  }

// ODBC error log output.
#define L_ODBC_ERROR(H_ENV, H_DBC, H_STMT) \
{ \
    SQLCHAR state[256] = {'\0'}; \
    SQLCHAR msg[256] = {'\0'}; \
    SDWORD code = 0; \
    SWORD sz = 0; \
    SQLError(H_ENV, H_DBC, H_STMT, state, &code, msg, sizeof(msg), &sz); \
    L_ERROR("ODBC error. %s(%" UNC_PFMT_SDWORD ")%*s\n", (char*)state, \
            code, (int)sz, (char*)msg);                                \
}

#define RETURN_IF_ODBC(H_ENV, H_DBC, H_STMT, CONDITIONS, RET) \
  if (CONDITIONS) { \
    L_ODBC_ERROR(H_ENV, H_DBC, H_STMT); \
    return RET; \
  }

#define GOTO_IF_ODBC(H_ENV, H_DBC, H_STMT, CONDITIONS, LABEL) \
  if (CONDITIONS) { \
    L_ODBC_ERROR(H_ENV, H_DBC, H_STMT); \
    goto LABEL; \
  }


// ---------------------------------------------------------------------
// definition of enumerate.
// ---------------------------------------------------------------------
// Database error code.
typedef enum {
  DB_E_OK,                        // Normal.
  DB_E_NG,                        // Other error.
  DB_E_PARA_ERR,                  // Parameter error.
  DB_E_NOTFOUND,                  // Record Not found.
  DB_E_UPDATE_ERR,                // Database update error.
  DB_E_TRANSACTION_ERR,           // Transaction error.
  DB_E_CONNECT_ERR,               // Connected error.
  DB_E_ALREADY_CONNECTED,         // Already connected.
  DB_E_FATAL_ERR                  // Database system error.
} db_err_e;

// transaction commit mode.
typedef enum {
  DB_COMMIT,          // Commit.
  DB_ROLLBACK         // Rollback.
} db_commit_e;

// Database transaction status.
typedef enum {
  STATUS_TRANSACTION_NONE,    // Non Transaction.
  STATUS_TRANSACTION_BEGIN,   // Transaction started, no update.
  STATUS_TRANSACTION_UPDATE,  // Transaction started, is updated.
  STATUS_TRANSACTION_ABORT    // Abort confirm.
} transaction_status_e;

// variant type struct data type.
typedef enum {
  VARIANT_UNKNOWN,
  VARIANT_STRING,
  VARIANT_INT8,
  VARIANT_INT16,
  VARIANT_INT32,
  VARIANT_INT64,
  VARIANT_FLOAT,
  VARIANT_DOUBLE,
  VARIANT_TIME,
  VARIANT_DATE,
  VARIANT_TIMESTAMP
} variant_data_type_e;


// ---------------------------------------------------------------------
// Definition of type.
// ---------------------------------------------------------------------
// variant type.
typedef struct variant_data_type {
  // ------------------------------
  // data type.
  // ------------------------------
  variant_data_type_e val_type;
  SQLLEN string_len;

  // ------------------------------
  // data element.
  // ------------------------------
  char* tmp_char;
  union union_variant {
    int8_t v_int8;
    int16_t v_int16;
    int32_t v_int32;
    int64_t v_int64;
    float v_float;
    double v_double;
    SQL_TIME_STRUCT v_time;
    SQL_DATE_STRUCT v_date;
    SQL_TIMESTAMP_STRUCT v_timestamp;
  } u_val;

  // ------------------------------
  //  constructor.
  // ------------------------------
  variant_data_type(void)
  {
    string_len = 0;
    val_type = VARIANT_UNKNOWN;
    tmp_char = NULL;
  }

  // ------------------------------
  //  destructor.
  // ------------------------------
  ~variant_data_type(void)
  {
    if (tmp_char != NULL) {
      delete [] tmp_char;
      tmp_char = NULL;
    }
  }

  // ------------------------------
  // char member area allocation.
  // ------------------------------
  void new_string(int size)
  {
    if (tmp_char != NULL) {
      delete [] tmp_char;
      tmp_char = NULL;
    }
    tmp_char = new char[size + 1];
  }

  // ------------------------------
  //  char member getter.
  // ------------------------------
  char* string_val() const
  {
    return tmp_char;
  }

  // ------------------------------
  //  char member setter.
  // ------------------------------
  void string_val(const char *input)
  {
    if (input == NULL) {
      if (tmp_char != NULL) {
        delete [] tmp_char;
        tmp_char = NULL;
      }
      return;
    }

    int len = strlen(input);

    if (tmp_char == NULL ||
       (tmp_char != NULL && strlen(tmp_char) != strlen(input))) {
      new_string(len);
    }
    strncpy(tmp_char, input, len);
    tmp_char[len] = '\0';
  }
} variant_t;

// mgmtdb database access data type.
typedef std::vector< std::vector<variant_t> > mgmtdb_variant_v;


// -------------------------------------------------------------
// Class declaration.
// -------------------------------------------------------------
class MgmtDatabase
{
 public:
  MgmtDatabase(void);
  ~MgmtDatabase(void);

  bool Init(void);
  bool Fini(void);

  db_err_e Connect(void);
  db_err_e Disconnect(void);
  db_err_e Exec(const std::string& sql_statement, const bool update_flag,
                const int32_t fetch_cnt, const int16_t fetch_type[],
                mgmtdb_variant_v& exec_value);
  db_err_e Abort(void);

 private:
  db_err_e BeginTransaction(void);
  db_err_e EndTransaction(const db_commit_e mode);
  db_err_e DbField2Value(SQLHSTMT hstmt, const int32_t fetch_cnt,
                const int16_t fetch_type[], mgmtdb_variant_v& exec_value);
  db_err_e SetBindCol(SQLHSTMT hstmt, const int column,
                const int16_t fetch_type, variant_t &exec_value);
  db_err_e DbErr2UsessErr(const RETCODE err_code) const;

  // -----------------------------
  //  data member.
  // -----------------------------
  SQLHENV henv_;   // ODBC env handle.
  SQLHDBC hdbc_;   // ODBC conection handle.

  transaction_status_e status_database_;
};

}  // namespace mgmtdb
}  // namespace unc



#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus
  void* mgmtdb_create(void);
  void mgmtdb_delete(void* db_object);
  int mgmtdb_connect(void* db_object);
  int mgmtdb_disconnect(void* db_object);
  int mgmtdb_one_execute(void* db_object, char* sql_statement,
        bool update, char* data_type, void* data);
  int mgmtdb_abort(void* db_object);
#ifdef __cplusplus
}
#endif  // __cplusplus
#endif      // _USESS_DATABASE_HH_
