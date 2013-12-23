/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */


/*
 *  @brief   ODBC Manager
 *  @file    odbcm_utils.hh
 */

#ifndef _ODBCM_UTILS_HH_
#define _ODBCM_UTILS_HH_

#include <map>
#include <string>
#include <odbcm_common.hh>
#include <odbcm_db_tableschema.hh>

namespace unc {
namespace uppl {


#if 0
typedef struct {
  const ODBCM_RC_STATUS rcode;
  const std::string rc_string;
}ReturnCodes;

/*
 * To print the database info in pfc log
 */
const char g_log_db_name[7][20] = {
  "UNC_DT_INVALID",   /* String for invalid db */
  "UNC_DT_STATE",     /* String for state db */
  "UNC_DT_CANDIDATE", /* String for candidate db */
  "UNC_DT_RUNNING",   /* String for running db */
  "UNC_DT_STARTUP",   /* String for startup db */
  "UNC_DT_IMPORT",    /* String for import db */
  "UNC_DT_AUDIT"      /* String for audit db */
};

/*
 * To disable binding log information ODBCM_DEBUG
 * flag should be 0 and to enable make it 1
 *
 */
#define ODBCM_DEBUG 1
/*
 * Check ODBCM_DEBUG flag is enabled or not
 */
#if ODBCM_DEBUG
#define odbcm_debug_info(fmt, ...) \
  pfc_log_debug(fmt, ##__VA_ARGS__);
#else
#define odbcm_debug_info(fmt, ...) (void)0
#endif

/*
 * Max length of characters used in ODBC details
 */
#define ODBCM_NAME_LEN      256

/*
 * ODBCM debug log
 */
#define ODBCM_PRINT_DEBUG_LOG(A, ...) \
    pfc_log_debug(#__VA_ARGS__ "=" A, ##__VA_ARGS__);

/*
 * Macro for database environment handle checking
 */
#define ODBCM_ENV_HANDLE_CHECK(henv, odbcm_rc)                \
  if (odbcm_rc != SQL_SUCCESS) {                              \
    ODBCM_RC_STATUS rc = ODBCMUtils::OdbcmHandleInfoPrint(    \
        SQL_HANDLE_ENV, henv, odbcm_rc, __LINE__, __FILE__);  \
    if (rc != SQL_SUCCESS)                                    \
    return (ODBCM_RC_STATUS)rc;                               \
  }

/*
 * Macro for database connection handle checking
 */
#define ODBCM_DBC_HANDLE_CHECK(hdbc, odbcm_rc)                \
  if (odbcm_rc != SQL_SUCCESS) {                              \
    ODBCM_RC_STATUS rc = ODBCMUtils::OdbcmHandleInfoPrint(    \
        SQL_HANDLE_DBC, hdbc, odbcm_rc, __LINE__, __FILE__);  \
    if (rc != SQL_SUCCESS)                                    \
      odbcm_rc = rc;                                          \
  }

/*
 * Macro for query statement handle checking
 */
#define ODBCM_STMT_HANDLE_CHECK(hstmt, hdbc, odbcm_rc)        \
  if (NULL != hstmt) {                                        \
    if (odbcm_rc != SQL_SUCCESS) {                              \
      ODBCM_RC_STATUS rc = ODBCMUtils::OdbcmHandleInfoPrint(    \
          SQL_HANDLE_STMT, hstmt, odbcm_rc, __LINE__, __FILE__);\
      if (rc == SQL_ERROR)                                      \
      ODBCMUtils::OdbcmStmtResourcesFree(hstmt);                \
      if (rc != SQL_SUCCESS)                                    \
      ODBCMUtils::OdbcmTransRollback(hdbc);                     \
      if (rc != SQL_SUCCESS) return (ODBCM_RC_STATUS)rc;        \
    }                                                           \
  }

/*
 * Macro for database parameters handle checking
 */
#define ODBCM_PARAM_HANDLE_CHECK(hstmt, odbcm_rc)             \
    if (odbcm_rc != SQL_SUCCESS ||                            \
      odbcm_rc != SQL_SUCCESS_WITH_INFO) {                    \
      ODBCM_RC_STATUS rc = ODBCMUtils::OdbcmHandleInfoPrint(  \
        SQL_HANDLE_STMT, hstmt, odbcm_rc, __LINE__, __FILE__);\
        if (rc == SQL_ERROR)                                  \
          ODBCMUtils::OdbcmStmtResourcesFree(hstmt);          \
        return (ODBCM_RC_STATUS)rc;                           \
    }

/*
 * Macro for database process handle checking
 */
#define ODBCM_PROCESS_HANDLE_CHECK(hstmt, odbcm_rc)           \
    if (odbcm_rc != SQL_SUCCESS) {                            \
      ODBCM_RC_STATUS rc = ODBCMUtils::OdbcmHandleInfoPrint(  \
        SQL_HANDLE_STMT, hstmt, odbcm_rc, __LINE__, __FILE__);\
        if (rc == SQL_ERROR ||                                \
            rc == SQL_STILL_EXECUTING ||                      \
            rc == SQL_NEED_DATA ) {                           \
          ODBCMUtils::OdbcmStmtResourcesFree(hstmt);          \
         }                                                    \
        odbcm_rc = (ODBCM_RC_STATUS)rc;                     \
    }

/*
 * Odbc return code
 */
typedef struct {
  const std::string       sql_state;  /* SQL state */
  const ODBCM_RC_STATUS   odbcm_rc;   /* SQL api return value */
}OdbcmReturnCode;

/*
 * Structure to hold complete odbc information
 */
typedef struct {
  SQLCHAR dbms_name[ODBCM_NAME_LEN];        /* DBMS name */
  SQLCHAR dbms_ver[ODBCM_NAME_LEN];         /* DBMS Version */
  SQLCHAR datasource_name[ODBCM_NAME_LEN];  /* Datasource name */
  SQLCHAR database_name[ODBCM_NAME_LEN];    /* Database name */
  SQLCHAR server_name[ODBCM_NAME_LEN];      /* Server name */
  SQLCHAR user_name[ODBCM_NAME_LEN];        /* Database user name */
  SQLCHAR odbcdriver_name[ODBCM_NAME_LEN];  /* Odbc driver name */
  SQLCHAR odbcdriver_ver[ODBCM_NAME_LEN];   /* Odbc driver version */
  SQLCHAR driver_ver[ODBCM_NAME_LEN];       /* driver version */
  SQLCHAR odbc_ver[ODBCM_NAME_LEN];         /* Odbc version */
  SQLCHAR IsdatabaseRO[ODBCM_NAME_LEN];     /* To check db is read-only */
}ODBCDetails;

/*
 * Class containing utility functions
 * required in ODBCM
 */
#endif
class ODBCMUtils {
  public:
    /*
     * Constructor of ODBCMUtils
     */
    ODBCMUtils();

    /*
     * Destructor of ODBCMUtils
     */
    ~ODBCMUtils();

    /*
     * Method to get IP address in string readable format
     * For example: 10.10.10.10
     */
    static std::string get_ip_string(uint32_t);

    static std::string get_ipv6_string(uint8_t *ipv6_address);

    //  static ODBCM_RC_STATUS Intialize_RCode_String();

  private:
    /** No private members */
};
}  // namespace uppl
}  // namespace unc
#endif /* _ODBCM_UTILS_HH_ */
