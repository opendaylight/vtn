/*
 * Copyright (c) 2012-2014 NEC Corporation
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

#include <sys/ipc.h>
#include <sys/sem.h>
#include <map>
#include <string>
#include "odbcm_common.hh"
#include "odbcm_db_tableschema.hh"

namespace unc {
namespace uppl {
#if defined(__GNU_LIBRARY__) && !defined(_SEM_SEMUN_UNDEFINED)
  /* union semun is defined by including <sys/sem.h> */
#else
  /* according to X/OPEN we have to define it ourselves */
  union semun {
    int val;                  /* value for SETVAL */
    struct semid_ds *buf;     /* buffer for IPC_STAT, IPC_SET */
    unsigned short *array;    /* array for GETALL, SETALL */
                              /* Linux specific part: */
    struct seminfo *__buf;    /* buffer for IPC_INFO */
  };
#endif
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
  pfc_log_debug((fmt), ##__VA_ARGS__);
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
  if ((odbcm_rc) != SQL_SUCCESS) {                              \
    ODBCM_RC_STATUS rc = ODBCMUtils::OdbcmHandleInfoPrint(    \
        SQL_HANDLE_ENV, (henv), (odbcm_rc), __LINE__, __FILE__);  \
    if (rc != SQL_SUCCESS)                                    \
    return (ODBCM_RC_STATUS)rc;                               \
  }

/*
 * Macro for database connection handle checking
 */
#define ODBCM_DBC_HANDLE_CHECK(hdbc, odbcm_rc)                \
  if ((odbcm_rc) != SQL_SUCCESS) {                              \
    ODBCM_RC_STATUS rc = ODBCMUtils::OdbcmHandleInfoPrint(    \
        SQL_HANDLE_DBC, (hdbc), (odbcm_rc), __LINE__, __FILE__);  \
    if (rc != SQL_SUCCESS)                                    \
      (odbcm_rc) = rc;                                          \
  }

/* 
 * Macro for query statement handle checking
 */
#define ODBCM_STMT_HANDLE_CHECK(hstmt, hdbc, odbcm_rc)        \
  if (NULL != (hstmt)) {                                        \
    if ((odbcm_rc) != SQL_SUCCESS) {                              \
      ODBCM_RC_STATUS rc = ODBCMUtils::OdbcmHandleInfoPrint(    \
          SQL_HANDLE_STMT, (hstmt), (odbcm_rc), __LINE__, __FILE__);\
      if (rc == SQL_ERROR)                                      \
      ODBCMUtils::OdbcmStmtResourcesFree((hstmt));                \
      if (rc != SQL_SUCCESS)                                    \
      ODBCMUtils::OdbcmTransRollback((hdbc));                     \
      if (rc != SQL_SUCCESS) return (ODBCM_RC_STATUS)rc;        \
    }                                                           \
  }

/*
 * Macro for database parameters handle checking
 */
#define ODBCM_PARAM_HANDLE_CHECK(hstmt, odbcm_rc)             \
    if ((odbcm_rc) != SQL_SUCCESS ||                            \
      (odbcm_rc) != SQL_SUCCESS_WITH_INFO) {                    \
      ODBCM_RC_STATUS rc = ODBCMUtils::OdbcmHandleInfoPrint(  \
        SQL_HANDLE_STMT, (hstmt), (odbcm_rc), __LINE__, __FILE__);\
        if (rc == SQL_ERROR)                                  \
          ODBCMUtils::OdbcmStmtResourcesFree((hstmt));          \
        return (ODBCM_RC_STATUS)rc;                           \
    }

/* 
 * Macro for database process handle checking
 */
#define ODBCM_PROCESS_HANDLE_CHECK(hstmt, odbcm_rc)           \
    if ((odbcm_rc) != SQL_SUCCESS) {                            \
      ODBCM_RC_STATUS rc = ODBCMUtils::OdbcmHandleInfoPrint(  \
        SQL_HANDLE_STMT, (hstmt), (odbcm_rc), __LINE__, __FILE__);\
        if (rc == SQL_ERROR ||                                \
            rc == SQL_STILL_EXECUTING ||                      \
            rc == SQL_NEED_DATA ) {                           \
          ODBCMUtils::OdbcmStmtResourcesFree((hstmt));          \
         }                                                    \
        (odbcm_rc) = (ODBCM_RC_STATUS)rc;                     \
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

     static int sem_id;

    /*
     * Info about the SQL error
     */
    static std::map<std::string, ODBCM_RC_STATUS> OdbcmSQLStateMap;
    static ReturnCodes rcode_string[];

    /*
     * Method to print ODBC connection info such as
     * DBMS name, DBMS version, ODBC version etc.
     */ 
    static void print_odbc_details(SQLHDBC);

    /*
     * Method to print the return code info in pfc log
     */
    static std::string get_RC_Details(int32_t error);

    /*
     * Method to get IP address in string readable format
     * For example: 10.10.10.10
     */ 
    static std::string get_ip_string(uint32_t);

    /*
     * Method to print file name, line number and error code
     * when a function returns failure i.e location of error
     */
    static void OdbcmHandleLocationPrint(SQLRETURN,
        int32_t, std::string);

    /*
     * Method to get the diagnostic info from SQL return value
     */
    static ODBCM_RC_STATUS OdbcmHandleDiagnosticsPrint(SQLSMALLINT,
        SQLHANDLE);

    /*
     * Method to print the debug information during failure
     */
    static ODBCM_RC_STATUS OdbcmHandleInfoPrint(SQLSMALLINT,
        SQLHANDLE, SQLRETURN, int32_t, std::string);

    /*
     * Method to free the memory resources when an error occurs
     */
    static int OdbcmStmtResourcesFree(SQLHANDLE);

    /*
     * Method to rollback a transaction
     */
    static void OdbcmTransRollback(SQLHANDLE);

    /*
     * Method to handle SQL cancel statement
     */
    static int OdbcmHandleSqlCancel(SQLHANDLE);

    /*
     * Mapping between SQL State string and odbcm return code
     */
    static ODBCM_RC_STATUS Initialize_OdbcmSQLStateMap(void);

    /*
     * Method to clear the OdbcmSQLStateMap
     */
    static ODBCM_RC_STATUS ClearOdbcmSQLStateMap(void);

    /*
     * Method to return the odbcm rc with respect to the
     * sql_state string value
     */
    static inline ODBCM_RC_STATUS get_odbc_rc(const std::string);
    /*
     * Method to convert ipv6 array to string
     */
    static std::string get_ipv6_string(uint8_t *ipv6_address);

    //  static ODBCM_RC_STATUS Intialize_RCode_String();

    // initializes semaphore using SETVAL
    static int set_semvalue(int val);

    // delete semaphore
    static int del_semvalue();

    static int SEM_DOWN();
    static int SEM_UP();

  private:
    /** No private members */
};
}  // namespace uppl
}  // namespace unc
#endif /* _ODBCM_UTILS_HH_ */
