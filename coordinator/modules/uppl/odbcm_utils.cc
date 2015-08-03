/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 *  @brief   ODBC Manager
 *  @file    odbcm_utils.cc
 */
#include  <sys/socket.h>
#include  <arpa/inet.h>
#include "odbcm_common.hh"
#include "odbcm_utils.hh"
#include "physicallayer.hh"
#include "physical_common_def.hh"
#include "odbcm_db_varbind.hh"

namespace unc {
namespace uppl {

/*
 * Mapping between SQL State string and odbcm return code
 */
std::map <std::string, ODBCM_RC_STATUS> ODBCMUtils::OdbcmSQLStateMap;

/**
 * @Description : ODBCMUtils constructor
 * @param[in]   : None
 * @return      : None
 **/
ODBCMUtils::ODBCMUtils() {
  /** Empty constructor */
}

/**
 * @Description : ODBCMUtils destructor
 * @param[in]   : None
 * @return      : None
 **/
ODBCMUtils::~ODBCMUtils() {
  /*
   * Clear the structure and map 
   */
}
/**
 * Return code string map structure array
 **/
ReturnCodes ODBCMUtils::rcode_string[] = {
  {ODBCM_RC_SUCCESS, "ODBCM_RC_SUCCESS"},
  {ODBCM_RC_SUCCESS_WITH_INFO, "ODBCM_RC_SUCCESS_WITH_INFO"},
  {ODBCM_RC_QUERY_STILL_EXECUTING, "ODBCM_RC_QUERY_STILL_EXECUTING"},
  {ODBCM_RC_FAILED, "ODBCM_RC_FAILED"},
  {ODBCM_RC_COMMON_LINK_FAILURE, "ODBCM_RC_COMMON_LINK_FAILURE"},
  {ODBCM_RC_CONNECTION_ERROR, "ODBCM_RC_CONNECTION_ERROR"},
  {ODBCM_RC_CONNECTION_TIMEOUT, "ODBCM_RC_CONNECTION_TIMEOUT"},
  {ODBCM_RC_CONNECTION_IN_USE, "ODBCM_RC_CONNECTION_IN_USE"},
  {ODBCM_RC_SERIALIZATION_ERROR, "ODBCM_RC_SERIALIZATION_ERROR"},
  {ODBCM_RC_INVALID_CONN_HANDLE, "ODBCM_RC_INVALID_CONN_HANDLE"},
  {ODBCM_RC_QUERY_TIMEOUT, "ODBCM_RC_QUERY_TIMEOUT"},
  {ODBCM_RC_ERROR_IN_FRAMEQUERY, "ODBCM_RC_ERROR_IN_FRAMEQUERY"},
  {ODBCM_RC_INVALID_TABLE_NAME, "ODBCM_RC_INVALID_TABLE_NAME"},
  {ODBCM_RC_INVALID_DB_OPERATION, "ODBCM_RC_INVALID_DB_OPERATION"},
  {ODBCM_RC_PKEY_VIOLATION, "ODBCM_RC_PKEY_VIOLATION"},
  {ODBCM_RC_MEMORY_ERROR, "ODBCM_RC_MEMORY_ERROR"},
  {ODBCM_RC_TABLE_NOT_FOUND, "ODBCM_RC_TABLE_NOT_FOUND"},
  {ODBCM_RC_RECORD_NOT_FOUND, "ODBCM_RC_RECORD_NOT_FOUND"},
  {ODBCM_RC_DATA_ERROR, "ODBCM_RC_DATA_ERROR"},
  {ODBCM_RC_RECORD_NO_MORE, "ODBCM_RC_RECORD_NO_MORE"},
  {ODBCM_RC_NO_RECORD, "ODBCM_RC_NO_RECORD"},
  {ODBCM_RC_ERROR_FETCHING_ROW, "ODBCM_RC_ERROR_FETCHING_ROW"},
  {ODBCM_RC_STMT_ERROR, "ODBCM_RC_STMT_ERROR"},
  {ODBCM_RC_DISCONNECT_ERROR, "ODBCM_RC_DISCONNECT_ERROR"},
  {ODBCM_RC_RECORD_ALREADY_EXISTS, "ODBCM_RC_RECORD_ALREADY_EXISTS"},
  {ODBCM_RC_CONN_ENV_ERROR, "ODBCM_RC_CONN_ENV_ERROR"},
  {ODBCM_RC_CONN_HANDLE_ERROR, "ODBCM_RC_CONN_HANDLE_ERROR"},
  {ODBCM_RC_GENERAL_ERROR, "ODBCM_RC_GENERAL_ERROR"},
  {ODBCM_RC_PARAM_BIND_ERROR, "ODBCM_RC_PARAM_BIND_ERROR"},
  {ODBCM_RC_WRONG_PARAM, "ODBCM_RC_WRONG_PARAM"},
  {ODBCM_RC_MORE_ROWS_FOUND, "ODBCM_RC_MORE_ROWS_FOUND"},
  {ODBCM_RC_ROW_EXISTS, "ODBCM_RC_ROW_EXISTS"},
  {ODBCM_RC_ROW_NOT_EXISTS, "ODBCM_RC_ROW_NOT_EXISTS"},
  {ODBCM_RC_CANDIDATE_DIRTY, "ODBCM_RC_CANDIDATE_DIRTY"},
  {ODBCM_RC_CANDIDATE_NO_DIRTY, "ODBCM_RC_CANDIDATE_NO_DIRTY"},
  {ODBCM_RC_SQL_ERROR, "ODBCM_RC_SQL_ERROR"},
  {ODBCM_RC_SQL_NEED_DATA, "ODBCM_RC_SQL_NEED_DATA"},
  {ODBCM_RC_ROW_STATUS_NOT_FOUND, "ODBCM_RC_ROW_STATUS_NOT_FOUND"},
  {ODBCM_RC_COLUMN_DOES_NOT_MATCH, "ODBCM_RC_COLUMN_DOES_NOT_MATCH"},
  {ODBCM_RC_PREPARED_STMT_ERROR, "ODBCM_RC_PREPARED_STMT_ERROR"},
  {ODBCM_RC_TYPE_ATTR_VIOLATION, "ODBCM_RC_TYPE_ATTR_VIOLATION"},
  {ODBCM_RC_INVALID_DESC, "ODBCM_RC_INVALID_DESC"},
  {ODBCM_RC_UNABLE_ESTABLISH_CONN, "ODBCM_RC_UNABLE_ESTABLISH_CONN"},
  {ODBCM_RC_CONNECTION_REJECTED, "ODBCM_RC_CONNECTION_REJECTED"},
  {ODBCM_RC_INSERT_VAL_LIST_NOT_MATCHED,
  "ODBCM_RC_INSERT_VAL_LIST_NOT_MATCHED"},
  {ODBCM_RC_DATA_TRUNCATION_ERROR, "ODBCM_RC_DATA_TRUNCATION_ERROR"},
  {ODBCM_RC_VARIABLE_NOT_SUPPLIED, "ODBCM_RC_VARIABLE_NOT_SUPPLIED"},
  {ODBCM_RC_VALUE_OUT_OF_RANGE, "ODBCM_RC_VALUE_OUT_OF_RANGE"},
  {ODBCM_RC_DATETIME_ERROR, "ODBCM_RC_DATETIME_ERROR"},
  {ODBCM_RC_DIVISIBLE_ERROR, "ODBCM_RC_DIVISIBLE_ERROR"},
  {ODBCM_RC_FIELD_OVERFLOW, "ODBCM_RC_FIELD_OVERFLOW"},
  {ODBCM_RC_INVALID_CHAR_SPEC, "ODBCM_RC_INVALID_CHAR_SPEC"},
  {ODBCM_RC_CURSOR_STATE, "ODBCM_RC_CURSOR_STATE"},
  {ODBCM_RC_INVALID_CURSOR, "ODBCM_RC_INVALID_CURSOR"},
  {ODBCM_RC_SYNTAX_ERROR, "ODBCM_RC_SYNTAX_ERROR"},
  {ODBCM_RC_INDEX_NOT_FOUND, "ODBCM_RC_INDEX_NOT_FOUND"},
  {ODBCM_RC_COLUMN_ALREADY_EXISTS, "ODBCM_RC_COLUMN_ALREADY_EXISTS"},
  {ODBCM_RC_COLUMN_NOT_FOUND, "ODBCM_RC_COLUMN_NOT_FOUND"},
  {ODBCM_RC_NULL_POINTER_ERROR, "ODBCM_RC_NULL_POINTER_ERROR"},
  {ODBCM_RC_FUNC_SEQUENCE_ERROR, "ODBCM_RC_FUNC_SEQUENCE_ERROR"},
  {ODBCM_RC_TRANSACTION_ERROR, "ODBCM_RC_TRANSACTION_ERROR"},
  {ODBCM_RC_TABLE_EXISTS, "ODBCM_RC_TABLE_EXISTS"},
  {ODBCM_RC_COLUMN_ALREADY, "ODBCM_RC_COLUMN_ALREADY"},
  {ODBCM_RC_SQL_NO_DATA, "ODBCM_RC_SQL_NO_DATA"}
};

/*ODBCM_RC_STATUS ODBCMUtils::Intialize_RCode_String() {
  return ODBCM_RC_SUCCESS;
}*/
/**
 * @Description : This method will Initialize OdbcmSQLStateMap, 
 *                with the values from OdbcmReturnCode structure array
 * @param[in]   : void 
 * @return      : ODBCM_RC_SUCCESS - is returned when the initialize
 *                is done successfully.
 *                ODBCM_RC_*       - is returned when the initialize is error
 **/
ODBCM_RC_STATUS ODBCMUtils::Initialize_OdbcmSQLStateMap(void) {
  /*
   * ODBC SQLSTATE Code Map with ODBCM return code
   */
  OdbcmReturnCode odbcm_rc_map[] = {
    /** Start with 07 are input errors */
    {"00000", ODBCM_RC_SUCCESS},
    {"07001", ODBCM_RC_WRONG_PARAM},
    {"07002", ODBCM_RC_COLUMN_DOES_NOT_MATCH},
    {"07005", ODBCM_RC_PREPARED_STMT_ERROR},
    {"07006", ODBCM_RC_TYPE_ATTR_VIOLATION},
    {"07009", ODBCM_RC_INVALID_DESC},
    {"07S01", ODBCM_RC_WRONG_PARAM},
    /** start with 08 are connection errors */
    {"08001", ODBCM_RC_UNABLE_ESTABLISH_CONN},
    {"08002", ODBCM_RC_CONNECTION_IN_USE},
    {"08003", ODBCM_RC_CONNECTION_ERROR},
    {"08004", ODBCM_RC_CONNECTION_REJECTED},
    {"08007", ODBCM_RC_TRANSACTION_ERROR},
    {"08S01", ODBCM_RC_COMMON_LINK_FAILURE},
    /** Like 21S01 are Insert value list does not match column list */
    {"21S01", ODBCM_RC_INSERT_VAL_LIST_NOT_MATCHED},
    /** Like 21S02 are Degree of derived table does not match column list */
    {"21S02", ODBCM_RC_COLUMN_DOES_NOT_MATCH},
    /** Start with 22 are Input/Output Data related errors */
    {"22001", ODBCM_RC_DATA_TRUNCATION_ERROR},
    {"22002", ODBCM_RC_VARIABLE_NOT_SUPPLIED},
    {"22003", ODBCM_RC_VALUE_OUT_OF_RANGE},
    {"22007", ODBCM_RC_DATETIME_ERROR},
    {"22008", ODBCM_RC_DATETIME_ERROR},
    {"22012", ODBCM_RC_DIVISIBLE_ERROR},
    {"22015", ODBCM_RC_FIELD_OVERFLOW},
    {"22018", ODBCM_RC_INVALID_CHAR_SPEC},
    {"22019", ODBCM_RC_INVALID_CHAR_SPEC},
    {"22025", ODBCM_RC_INVALID_CHAR_SPEC},
    {"22026", ODBCM_RC_INVALID_CHAR_SPEC},
    {"23000", ODBCM_RC_GENERAL_ERROR},
    {"24000", ODBCM_RC_CURSOR_STATE},
    {"25000", ODBCM_RC_DISCONNECT_ERROR},
    /** Start with 25S are Txn related errors */
    {"25S01", ODBCM_RC_TRANSACTION_ERROR},
    {"25S02", ODBCM_RC_TRANSACTION_ERROR},
    {"25S03", ODBCM_RC_TRANSACTION_ERROR},
    {"28000", ODBCM_RC_CONNECTION_ERROR},
    {"34000", ODBCM_RC_INVALID_CURSOR},
    {"3C000", ODBCM_RC_INVALID_CURSOR},
    {"3D000", ODBCM_RC_GENERAL_ERROR},
    {"3F000", ODBCM_RC_GENERAL_ERROR},
    /** Start with 40  are Txn related errors */
    {"40001", ODBCM_RC_SERIALIZATION_ERROR},
    {"40002", ODBCM_RC_TRANSACTION_ERROR},
    {"40003", ODBCM_RC_TRANSACTION_ERROR},
    /** Start with 42 are Syntax related errors */
    /** 42000 Syntax error or access violation */
    {"42000", ODBCM_RC_SYNTAX_ERROR},
    /** 42S01 Base table or view already exists */
    {"42S01", ODBCM_RC_TABLE_EXISTS},
    /** 42S02 Base table or view not found */
    {"42S02", ODBCM_RC_TABLE_NOT_FOUND},
    /** 42S11 Index already exists */
    {"42S11", ODBCM_RC_PKEY_VIOLATION},
    /** 42S12 Index not found */
    {"42S12", ODBCM_RC_INDEX_NOT_FOUND},
    /** 42S21 Column already exists */
    {"42S21", ODBCM_RC_COLUMN_ALREADY_EXISTS},
    /** 42S22 Column not found */
    {"42S22", ODBCM_RC_COLUMN_NOT_FOUND},
    {"44000", ODBCM_RC_GENERAL_ERROR},
    {"HY000", ODBCM_RC_GENERAL_ERROR},
    {"HY001", ODBCM_RC_MEMORY_ERROR},
    /** HY003 Invalid Appln Buffer */
    {"HY003", ODBCM_RC_DATA_ERROR},
    /** HY004 Invalid SQL Datatype */
    {"HY004", ODBCM_RC_DATA_ERROR},
    /** HY007 Associated statement is not prepared */
    /** HY007 Happening for APIs which are not used in this appln */
    {"HY007", ODBCM_RC_GENERAL_ERROR},
    /** HY008 Operation Cancelled - Errors during Asynchronous operation */
    {"HY008", ODBCM_RC_GENERAL_ERROR},
    /** HY009 Invalid use of null pointer */
    {"HY009", ODBCM_RC_NULL_POINTER_ERROR},
    /** HY010 Function Sequence error */
    {"HY010", ODBCM_RC_FUNC_SEQUENCE_ERROR},
    /** HY011 Attribute cannot be set now */
    {"HY011", ODBCM_RC_DATA_ERROR},
    /** HY012 Invalid transaction operation code */
    {"HY012", ODBCM_RC_TRANSACTION_ERROR},
    /** HY013 Memory Management error */
    {"HY013", ODBCM_RC_MEMORY_ERROR},
    /** HY014 Limit on the number of handles */
    {"HY014", ODBCM_RC_GENERAL_ERROR},
    /** HY015 No cursor name available - may not happen in this appln */
    {"HY015", ODBCM_RC_INVALID_CURSOR},
    /** HY016 Cannot modify an implementation row descriptor */
    {"HY016", ODBCM_RC_GENERAL_ERROR},
    /** HY017 Invalid use of an automatically allocated descriptor handle */
    /** Descriptor Handle not used in this appln */
    {"HY017", ODBCM_RC_GENERAL_ERROR},
    /** HY018 Server declined cancel request */
    /** Happening for APIs which are not used in this appln */
    {"HY018", ODBCM_RC_GENERAL_ERROR},
    /** HY019 Non-character and non-binary data sent in pieces */
    /** Happening for APIs which are not used in this appln */
    {"HY019", ODBCM_RC_GENERAL_ERROR},
    /** HY020 Attempt to concatenate a null value */
    /** Happening for APIs which are not used in this appln */
    {"HY020", ODBCM_RC_GENERAL_ERROR},
    /** HY021 Inconsistent descriptor information */
    /** Descriptor Handle not used in this appln */
    {"HY021", ODBCM_RC_GENERAL_ERROR},
    /** HY024 Invalid attribute value */
    {"HY024", ODBCM_RC_DATA_ERROR},
    /** HY090 Invalid string or buffer length */
    {"HY090", ODBCM_RC_DATA_ERROR},
    /** HY095 Function type out of range */
    /** Happening for APIs which are not used in this appln */
    {"HY095", ODBCM_RC_GENERAL_ERROR},
    /** HY096 Invalid information type */
    /** Happening for APIs which are not used in this appln */
    {"HY096", ODBCM_RC_GENERAL_ERROR},
    /** HY097 Column type out of range */
    /** Happening for APIs which are not used in this appln */
    {"HY097", ODBCM_RC_GENERAL_ERROR},
    /** Happening for APIs which are not used in this appln */
    {"HY098", ODBCM_RC_GENERAL_ERROR},
    /** Happening for APIs which are not used in this appln */
    {"HY099", ODBCM_RC_GENERAL_ERROR},
    /** Happening for APIs which are not used in this appln */
    {"HY100", ODBCM_RC_GENERAL_ERROR},
    /** Happening for APIs which are not used in this appln */
    {"HY101", ODBCM_RC_GENERAL_ERROR},
    /** Happening for APIs which are not used in this appln */
    {"HY103", ODBCM_RC_GENERAL_ERROR},
    /** HY104 Invalid precison or scale type */
    {"HY104", ODBCM_RC_DATA_ERROR},
    {"HY105", ODBCM_RC_DATA_ERROR},
    {"HY106", ODBCM_RC_INVALID_CURSOR},
    {"HY107", ODBCM_RC_INVALID_CURSOR},
    {"HY109", ODBCM_RC_INVALID_CURSOR},
    {"HY110", ODBCM_RC_GENERAL_ERROR},
    {"HY111", ODBCM_RC_GENERAL_ERROR},
    {"HYC00", ODBCM_RC_GENERAL_ERROR},
    {"HYT00", ODBCM_RC_QUERY_TIMEOUT},
    {"HYT01", ODBCM_RC_CONNECTION_TIMEOUT},
    /**database access problem due to connection dead*/
    {"57P01", ODBCM_RC_CONNECTION_ERROR}
  };
  /** Initialise the local variables */
  uint32_t loop  = 0;
  uint32_t count = 0;
  std::pair<std::map<std::string, ODBCM_RC_STATUS>::iterator, bool> odbc_rc;
  /** Check map is empty or not */
  if (ODBCMUtils::OdbcmSQLStateMap.empty()) {
    count = sizeof(odbcm_rc_map)/sizeof(OdbcmReturnCode);
    pfc_log_debug("ODBCM::ODBCMUtils:%s : count %d", __func__, count);
    /** Traverse and insert in the map */
    for (loop = 0; loop < count; loop++) {
      odbc_rc = OdbcmSQLStateMap.insert(
          std::pair<std::string, ODBCM_RC_STATUS>
          (odbcm_rc_map[loop].sql_state,
           odbcm_rc_map[loop].odbcm_rc));
      if (odbc_rc.second == false) {
        pfc_log_debug("ODBCM::ODBCMUtils:%s "
            "Error in inserting OdbcmSQLStateMap %s", __func__,
            odbcm_rc_map[loop].sql_state.c_str());
        return ODBCM_RC_FAILED;
      }  // if
    }  // for
  }  // if
  return ODBCM_RC_SUCCESS;
}

/**
 * @Description : Clear the OdbcmSQLStateMap if it is not empty
 * @param[in]   : void
 * @return      : ODBCM_RC_SUCCESS - is returned when the Clearing of   
 *                OdbcmSQLStateMap is done successfully. 
 **/
ODBCM_RC_STATUS ODBCMUtils::ClearOdbcmSQLStateMap(void) {
  /** Check map is empty or not */
  if (!OdbcmSQLStateMap.empty()) {
    /** Clear the map if its not empty */
    OdbcmSQLStateMap.clear();
  }
  return ODBCM_RC_SUCCESS;
}

/**
 * @Description : This method returns odbcm rc with respect to the
 *                sql_state string value
 * @param[in]   : odbcm_sqlstate - sql statement 
 * @return      : ODBCM_RC_SUCCESS - is returned when the iterator
 *                is done successfully.
 *                ODBCM_RC_FAILED  - is returned when the iterator is fails
 **/
ODBCM_RC_STATUS ODBCMUtils::get_odbc_rc(const std::string odbcm_sqlstate) {
  std::map<std::string, ODBCM_RC_STATUS>::iterator iter;
    /** Find the return code mapped to SQL State string */
  if (OdbcmSQLStateMap.empty() != 0) {
    pfc_log_debug("ODBCM::ODBCMUtils:%s "
                    " Error code map is empty ", __func__);
    return ODBCM_RC_FAILED;
  }
  OdbcmSQLStateMap.begin();
  pfc_log_debug("ODBCM::ODBCMUtils:SQLSTATE= %s ", odbcm_sqlstate.c_str());
  iter = OdbcmSQLStateMap.find(odbcm_sqlstate);
  if (iter == OdbcmSQLStateMap.end()) {
    pfc_log_debug("ODBCM::ODBCMUtils:%s "
                " Error code does not exists ", __func__);
    return ODBCM_RC_FAILED;
  }
  return iter->second;
}

/**
 * @Description : Print odbc info which are collected by SQLGetInfo() 
 * @param[in]   : conn_handle - ODBC connection handle   
 * @return      : None
 **/
void ODBCMUtils::print_odbc_details(SQLHDBC conn_handle) {
  /** Initialise the local variables */
  ODBCDetails   odbc_info;
  SQLUSMALLINT  MaxAConCount    = 0;
  SQLUSMALLINT  LevelOfODBCCon  = 0;
  SQLUSMALLINT  max_concur_act  = 0;
  SQLUINTEGER   LevelOfSQL      = 0;
  SQLUINTEGER   getdata_support = 0;
  SQLRETURN     odbc_rc = SQL_SUCCESS;
  /* 
   * Get odbc information using SQLGetInfo
   */
  odbc_rc = SQLGetInfo(conn_handle, SQL_GETDATA_EXTENSIONS,
             (SQLPOINTER)&getdata_support, 0, 0);
  if (odbc_rc != SQL_SUCCESS && odbc_rc != SQL_SUCCESS_WITH_INFO) {
    pfc_log_debug("ODBCM::ODBCMUtils::odbc_info:"
                 " Error SQL_GETDATA_EXTENSIONS");
  }
  /** Get data source name */
  odbc_rc = SQLGetInfo(conn_handle, SQL_DATA_SOURCE_NAME,
             (SQLPOINTER)odbc_info.datasource_name,
             sizeof(odbc_info.datasource_name), NULL);
  if (odbc_rc != SQL_SUCCESS && odbc_rc != SQL_SUCCESS_WITH_INFO) {
    pfc_log_debug("ODBCM::ODBCMUtils::odbc_info:"
                 " Error SQL_DATA_SOURCE_NAME");
  }
  /** Get database name */
  odbc_rc = SQLGetInfo(conn_handle, SQL_DATABASE_NAME,
             (SQLPOINTER)odbc_info.database_name,
             sizeof(odbc_info.database_name), NULL);
  if (odbc_rc != SQL_SUCCESS && odbc_rc != SQL_SUCCESS_WITH_INFO) {
    pfc_log_debug("ODBCM::ODBCMUtils::odbc_info:"
                 " Error SQL_DATABASE_NAME");
  }
  /** Get server name */
  odbc_rc = SQLGetInfo(conn_handle, SQL_SERVER_NAME,
             (SQLPOINTER)odbc_info.server_name,
             sizeof(odbc_info.server_name), NULL);
  if (odbc_rc != SQL_SUCCESS && odbc_rc != SQL_SUCCESS_WITH_INFO) {
    pfc_log_debug("ODBCM::ODBCMUtils::odbc_info:"
                 " Error SQL_SERVER_NAME");
  }
  /** Get database user name */
  odbc_rc = SQLGetInfo(conn_handle, SQL_USER_NAME,
               (SQLPOINTER)odbc_info.user_name,
               sizeof(odbc_info.user_name), NULL);
  if (odbc_rc != SQL_SUCCESS && odbc_rc != SQL_SUCCESS_WITH_INFO) {
    pfc_log_debug("ODBCM::ODBCMUtils::odbc_info:"
                 " Error SQL_USER_NAME");
  }
  /** Get DBMS name */
  odbc_rc = SQLGetInfo(conn_handle, SQL_DBMS_NAME,
             (SQLPOINTER)odbc_info.dbms_name,
             sizeof(odbc_info.dbms_name), NULL);
  if (odbc_rc != SQL_SUCCESS && odbc_rc != SQL_SUCCESS_WITH_INFO) {
    pfc_log_debug("ODBCM::ODBCMUtils::odbc_info:"
                 " Error SQL_DBMS_NAME");
  }
  /** Get DBMS version */
  odbc_rc = SQLGetInfo(conn_handle, SQL_DBMS_VER,
             (SQLPOINTER)odbc_info.dbms_ver,
             sizeof(odbc_info.dbms_ver), NULL);
  if (odbc_rc != SQL_SUCCESS && odbc_rc != SQL_SUCCESS_WITH_INFO) {
    pfc_log_debug("ODBCM::ODBCMUtils::odbc_info:"
                 " Error SQL_DBMS_VER");
  }
  /** Get odbc driver name  */
  odbc_rc = SQLGetInfo(conn_handle, SQL_DRIVER_NAME,
             (SQLPOINTER)odbc_info.odbcdriver_name,
             sizeof(odbc_info.odbcdriver_name), NULL);
  if (odbc_rc != SQL_SUCCESS && odbc_rc != SQL_SUCCESS_WITH_INFO) {
    pfc_log_debug("ODBCM::ODBCMUtils::odbc_info:"
                 " Error SQL_DRIVER_NAME");
  }
  /** Get odbc version */
  odbc_rc = SQLGetInfo(conn_handle, SQL_DRIVER_ODBC_VER,
             (SQLPOINTER)odbc_info.odbcdriver_ver,
             sizeof(odbc_info.odbcdriver_ver), NULL);
  if (odbc_rc != SQL_SUCCESS && odbc_rc != SQL_SUCCESS_WITH_INFO) {
    pfc_log_debug("ODBCM::ODBCMUtils::odbc_info:"
                 " Error SQL_DRIVER_ODBC_VER");
  }
  /** Get odbc driver version */
  odbc_rc = SQLGetInfo(conn_handle, SQL_DRIVER_VER,
             (SQLPOINTER)odbc_info.driver_ver,
             sizeof(odbc_info.driver_ver), NULL);
  if (odbc_rc != SQL_SUCCESS && odbc_rc != SQL_SUCCESS_WITH_INFO) {
    pfc_log_debug("ODBCM::ODBCMUtils::odbc_info:"
                 " Error SQL_DRIVER_VER");
  }
  /** Get odbc version */
  odbc_rc = SQLGetInfo(conn_handle, SQL_ODBC_VER,
             (SQLPOINTER)odbc_info.odbc_ver,
             sizeof(odbc_info.odbc_ver), NULL);
  if (odbc_rc != SQL_SUCCESS && odbc_rc != SQL_SUCCESS_WITH_INFO) {
    pfc_log_debug("ODBCM::ODBCMUtils::odbc_info:"
                 " Error SQL_ODBC_VER");
  }
  /** Get whether data source is read only */
  odbc_rc = SQLGetInfo(conn_handle, SQL_DATA_SOURCE_READ_ONLY,
             (SQLPOINTER)odbc_info.IsdatabaseRO,
             sizeof(odbc_info.IsdatabaseRO), NULL);
  if (odbc_rc != SQL_SUCCESS && odbc_rc != SQL_SUCCESS_WITH_INFO) {
    pfc_log_debug("ODBCM::ODBCMUtils::odbc_info:"
                 " Error SQL_DATA_SOURCE_READ_ONLY");
  }
  /** Get the maximum allowed driver connections */
  odbc_rc = SQLGetInfo(conn_handle, SQL_MAX_DRIVER_CONNECTIONS,
                       &MaxAConCount, 0, 0);
  if (odbc_rc != SQL_SUCCESS && odbc_rc != SQL_SUCCESS_WITH_INFO) {
    pfc_log_debug("ODBCM::ODBCMUtils::odbc_info:"
                 " Error SQL_MAX_DRIVER_CONNECTIONS");
  }
  /** Get the odbc api conformance parameter */
  odbc_rc = SQLGetInfo(conn_handle, SQL_ODBC_API_CONFORMANCE,
                       &LevelOfODBCCon, 0, 0);
  if (odbc_rc != SQL_SUCCESS && odbc_rc != SQL_SUCCESS_WITH_INFO) {
    pfc_log_debug("ODBCM::ODBCMUtils::odbc_info:"
                 " Error SQL_ODBC_API_CONFORMANCE");
  }
  /** Get the sql conformance  */
  odbc_rc = SQLGetInfo(conn_handle, SQL_SQL_CONFORMANCE,
             (SQLPOINTER)&LevelOfSQL, sizeof(LevelOfSQL), NULL);
  if (odbc_rc != SQL_SUCCESS && odbc_rc != SQL_SUCCESS_WITH_INFO) {
    pfc_log_debug("ODBCM::ODBCMUtils::odbc_info:"
                 " Error SQL_SQL_CONFORMANCE");
  }
  /** Get the maximum allowed concurrent activities */
  odbc_rc = SQLGetInfo(conn_handle, SQL_MAX_CONCURRENT_ACTIVITIES,
                       &max_concur_act, 0, 0);
  if (odbc_rc != SQL_SUCCESS && odbc_rc != SQL_SUCCESS_WITH_INFO) {
    pfc_log_debug("ODBCM::ODBCMUtils::odbc_info:"
                 " Error SQL_MAX_CONCURRENT_ACTIVITIES");
  }

  if (max_concur_act == 0) {
    pfc_log_debug("ODBCM::ODBCMUtils::odbc_info: "
        "MAX_CONCURRENT_ACTIVITIES: No limit or undefined");
  } else {
    pfc_log_debug("ODBCM::ODBCMUtils::odbc_info: "
        "MAX_CONCURRENT_ACTIVITIES: %u\n", max_concur_act);
  }
  if (getdata_support & SQL_GD_ANY_ORDER) {
    pfc_log_debug("ODBCM::ODBCMUtils::odbc_info: "
        "Columns can be retrieved in any order");
  } else {
    pfc_log_debug("ODBCM::ODBCMUtils::odbc_info: "
        "Columns must be retrieved in order");
  }
  if (getdata_support & SQL_GD_ANY_COLUMN) {
    pfc_log_debug("ODBCM::ODBCMUtils::odbc_info: "
        "Columns can be retrieved before last bound one");
  } else {
    pfc_log_debug("ODBCM::ODBCMUtils::odbc_info: "
        "Columns must be retrieved after last bound one");
  }
  /*
   * Print the complete odbc info
   */
  ODBCM_PRINT_DEBUG_LOG("%s", odbc_info.dbms_name);
  ODBCM_PRINT_DEBUG_LOG("%s", odbc_info.dbms_ver);
  ODBCM_PRINT_DEBUG_LOG("%s", odbc_info.datasource_name);
  ODBCM_PRINT_DEBUG_LOG("%s", odbc_info.database_name);
  ODBCM_PRINT_DEBUG_LOG("%s", odbc_info.server_name);
  ODBCM_PRINT_DEBUG_LOG("%s", odbc_info.user_name);
  ODBCM_PRINT_DEBUG_LOG("%s", odbc_info.odbcdriver_name);
  ODBCM_PRINT_DEBUG_LOG("%s", odbc_info.odbcdriver_ver);
  ODBCM_PRINT_DEBUG_LOG("%s", odbc_info.driver_ver);
  ODBCM_PRINT_DEBUG_LOG("%s", odbc_info.odbc_ver);
  ODBCM_PRINT_DEBUG_LOG("%s", odbc_info.IsdatabaseRO);
  ODBCM_PRINT_DEBUG_LOG("%d", MaxAConCount);
  ODBCM_PRINT_DEBUG_LOG("%d", LevelOfODBCCon);
  ODBCM_PRINT_DEBUG_LOG("%d", max_concur_act);
  ODBCM_PRINT_DEBUG_LOG("%d", static_cast<int> (LevelOfSQL));
  ODBCM_PRINT_DEBUG_LOG("%d", static_cast<int> (getdata_support));
}

/**
 * @Description : return the ip address version 4
 * @param[in]   : ip_address - ip address type i.e. IPV4
 * @return      : string - The conversion of ip adress to asci format
 **/
std::string ODBCMUtils::get_ip_string(uint32_t ip_address) {
  struct sockaddr_in ip;
  ip.sin_addr.s_addr = ip_address;
  /** Conver uint32_t ip adress to asci readable format */
  return inet_ntoa(ip.sin_addr);
}

/**
 * @Description : return the ipv6 address string
 * @param[in]   : ipv6_address - ip address type i.e. IPV6
 * @return      : string  - The conversion of ip adress to asci format
 **/
std::string ODBCMUtils::get_ipv6_string(uint8_t *ipv6_address) {
  sockaddr_in6 addr;
  memset(&addr, 0, sizeof(sockaddr_in6));
  char str[INET6_ADDRSTRLEN];
  ODBCM_MEMSET(&str, '\0', INET6_ADDRSTRLEN);
  ODBCM_MEMSET(&addr.sin6_addr, 0, sizeof addr.sin6_addr);
  //  store this IP address in addr
  ODBCM_MEMCPY(&addr.sin6_addr.s6_addr, ipv6_address,
               sizeof addr.sin6_addr.s6_addr);
  inet_ntop(AF_INET6, &(addr.sin6_addr), str, INET6_ADDRSTRLEN);
  return std::string(str);
}

/**
 * @Description : return the return code string if return code enum is given
 * @param[in]   : error - error code
 * @return      : string - corresponding error code string
 **/
std::string ODBCMUtils::get_RC_Details(int32_t error) {
  int length;
  if (error == ODBCM_RC_SQL_INVALID_HANDLE)
    return std::string("ODBCM_RC_SQL_INVALID_HANDLE");
  else if (error == ODBCM_RC_QUERY_FAILED)
    return std::string("ODBCM_RC_QUERY_FAILED");

  length = sizeof(rcode_string)/sizeof(ReturnCodes);
  if (error < length)
    return std::string(rcode_string[error].rc_string);
  else
    return std::string("");
}

/**
 * @Description : To print the error location
 * @param[in]   : odbcm_rc - return code of ODBCM api 
                  line     - specify the line number
                  file     - specify the file name
 * @return      : None
 **/
void ODBCMUtils::OdbcmHandleLocationPrint(SQLRETURN odbcm_rc,
    int32_t line, std::string file) {
  /** Print the error location */
  pfc_log_debug("ODBCM::ODBCMUtils::OdbcmHandleLocationPrint: "
      "Error in file: %s, line_no: %d, odbcm_rc:%d",
      file.c_str(), line, odbcm_rc);
}

/**
* @Description : To print the return values
* @param[in]   : htype - handle type
                 hndl  - handle
* @return      : ODBCM_RC_SUCCESS - if the OdbcmHandleDiagnosticsPrint is
*                success
*                ODBCM_RC_*       - if the OdbcmHandleDiagnosticsPrint is
*                failed
**/
ODBCM_RC_STATUS ODBCMUtils::OdbcmHandleDiagnosticsPrint(
              SQLSMALLINT htype,
              SQLHANDLE hndl) {
  /** Initialise the local variables */
  SQLCHAR     message[SQL_MAX_MESSAGE_LENGTH + 1];
  SQLCHAR     sqlstate[SQL_SQLSTATE_SIZE + 1];
  SQLINTEGER  sqlcode = 0;
  SQLSMALLINT length = 0;
  SQLSMALLINT col_no = 1;
  ODBCM_RC_STATUS rc = ODBCM_RC_SUCCESS;
  /* Get multiple field settings of diagnostic record */
  while (SQLGetDiagRec(htype, hndl, col_no, sqlstate, &sqlcode, message,
                       SQL_MAX_MESSAGE_LENGTH + 1, &length) == SQL_SUCCESS) {
    pfc_log_debug("ODBCM::ODBCMUtils::SQLGetDiagRec: "
      "SQLState: %s, Native Error Code: %d, Message: %s",
      sqlstate, static_cast<int> (sqlcode), message);
    col_no++;
  /** Get the odbcm error code */
    rc = ODBCMUtils::get_odbc_rc(std::string((const char*)sqlstate));
    if (rc != ODBCM_RC_FAILED) {
      if (rc == ODBCM_RC_COMMON_LINK_FAILURE) {
        pfc_log_error("ODBCM::ODBCMUtills:: ODBCM_RC_COMMON_LINK_FAILURE");
        PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
        PhysicalCore* physical_core = physical_layer->get_physical_core();
        physical_layer->get_odbc_manager()->FreeingConnections(false);
        if (physical_core->system_transit_state_ == true) {
          return ODBCM_RC_FAILED;
        }
        return ODBCM_RC_CONNECTION_ERROR;
      }
      pfc_log_debug("ODBCM::ODBCMUtils::SQLGetDiagRec: SQLState: %s,"
      "OdbcmReturnCode: %d(%s)", sqlstate, rc,
      ODBCMUtils::get_RC_Details(rc).c_str());
      return (ODBCM_RC_STATUS) rc;
    }
  }
  return ODBCM_RC_FAILED;
} /* OdbcmHandleDiagnosticsPrint */

/**
* @Description : To print the debug information - outputs to screen 
*                unexpected occurrences with ODBCM functions
* @param[in]   : htype    - handle type
*                hndl     - handle
*                odbcm_rc - return code of ODBCM api
*                line     - specify the line number 
*                file     - specify the file name
* @return      : ODBCM_RC_SUCCESS - if the OdbcmHandleInfoPrint is success
*                ODBCM_RC_*       - if the OdbcmHandleInfoPrint is failed
**/
ODBCM_RC_STATUS ODBCMUtils::OdbcmHandleInfoPrint(
                              SQLSMALLINT htype,
                              SQLHANDLE hndl,
                              SQLRETURN odbcm_rc,
                              int32_t line,
                              std::string file) {
  /** Initialize the local variables */
  ODBCM_RC_STATUS rc = ODBCM_RC_SUCCESS;
  /** Switch based on the return code */
  switch (odbcm_rc) {
    case SQL_SUCCESS:
      rc = ODBCM_RC_SUCCESS;
      break;
    case SQL_INVALID_HANDLE:
      pfc_log_debug("ODBCM::ODBCMUtils::OdbcmHandleInfoPrint: "
        "ODBCM INVALID HANDLE !!!!!");
      OdbcmHandleLocationPrint(odbcm_rc, line, file);
      rc = ODBCM_RC_INVALID_CONN_HANDLE;
      break;
    case SQL_ERROR:
      pfc_log_debug("ODBCM::ODBCMUtils::OdbcmHandleInfoPrint: "
        "ODBCM ERROR !!!!!");
      OdbcmHandleLocationPrint(odbcm_rc, line, file);
      rc = OdbcmHandleDiagnosticsPrint(htype, hndl);
      break;
    case SQL_SUCCESS_WITH_INFO:
      rc = ODBCM_RC_SUCCESS_WITH_INFO;
      break;
    case SQL_STILL_EXECUTING:
      pfc_log_debug("ODBCM::ODBCMUtils::OdbcmHandleInfoPrint: "
        "ODBCM STILL EXECUTING !!!!!");
      rc = ODBCM_RC_SUCCESS;
      break;
    case SQL_NEED_DATA:
      pfc_log_debug("ODBCM::ODBCMUtils::OdbcmHandleInfoPrint: "
        "ODBCM NEED DATA !!!!!");
      OdbcmHandleLocationPrint(odbcm_rc, line, file);
      OdbcmHandleSqlCancel(hndl);
      rc = ODBCM_RC_SQL_NEED_DATA;
      break;
    case SQL_NO_DATA_FOUND:
      pfc_log_debug("ODBCM::ODBCMUtils::OdbcmHandleInfoPrint: "
        "ODBCM NO DATA FOUND !!!!!");
      rc = ODBCM_RC_NO_RECORD;
      break;
    default:
      pfc_log_debug("ODBCM::ODBCMUtils::Unknown return code !!!!!");
      OdbcmHandleLocationPrint(odbcm_rc, line, file);
      rc = ODBCM_RC_DEFAULT;
      break;
  }
  return rc;
} /* OdbcmHandleInfoPrint */

/**
* @Description : To handle the sql cancel statment
* @param[in]   : htype - handle type
* @return      : int   - returns a numeric value based on the
*                OdbcmHandleSqlCancel function
**/
int ODBCMUtils::OdbcmHandleSqlCancel(SQLHANDLE htype) {
  /** Initialise the local variables */
  SQLRETURN odbc_rc = ODBCM_RC_SUCCESS;
  pfc_log_debug("ODBCM::ODBCMUtils::OdbcmHandleSqlCancel:"
    " SQLCancel Statement !!!!!");
  /** Cancel teh sql statement */
  odbc_rc = SQLCancel(htype);
  return odbc_rc;
} /*OdbcmHandleSqlCancel(htype) */

/**
* @Description : To free stmt handles & print unexpected occurrences
* @param[in]   : hstmt - handle statement 
* @return      : int   - returns a numeric value based on the
*                OdbcmStmtResourcesFree function
**/
int ODBCMUtils::OdbcmStmtResourcesFree(SQLHANDLE hstmt) {
  /** Initialise the local variables */
  int       rc = 0;
  SQLRETURN odbcm_rc = SQL_SUCCESS;
  /** Free the statement handle */
  odbcm_rc = SQLFreeStmt(hstmt, SQL_UNBIND);
  rc = OdbcmHandleInfoPrint(
    SQL_HANDLE_STMT, hstmt, odbcm_rc, __LINE__, __FILE__);

  /** Free the statement handle */
  odbcm_rc = SQLFreeStmt(hstmt, SQL_RESET_PARAMS);
  rc = OdbcmHandleInfoPrint(
    SQL_HANDLE_STMT, hstmt, odbcm_rc, __LINE__, __FILE__);

  /** Free the statement handle */
  odbcm_rc = SQLFreeStmt(hstmt, SQL_CLOSE);
  rc = OdbcmHandleInfoPrint(
    SQL_HANDLE_STMT, hstmt, odbcm_rc, __LINE__, __FILE__);
  if (NULL != hstmt)
    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
  if (rc != 0)
    return 1;

  return 0;
} /* OdbcmStmtResourcesFree */

/**
*@Description : To rollback transactions on a single connection
*@param[in]   : hdbc - handler database connection
*@return      : void
**/
void ODBCMUtils::OdbcmTransRollback(SQLHANDLE hdbc) {
  /** Initialise the local variables */
  int       rc = 0;
  SQLRETURN odbcm_rc = SQL_SUCCESS;
  /** End transactions on the connection */
  odbcm_rc = SQLEndTran(SQL_HANDLE_DBC, hdbc, SQL_ROLLBACK);
  rc = OdbcmHandleInfoPrint(
    SQL_HANDLE_DBC, hdbc, odbcm_rc, __LINE__, __FILE__);
  if (rc == 0) {
    pfc_log_debug("ODBCM::ODBCMUtils::OdbcmTransRollback:"
      " Transaction rolled back successfully.\n");
  }
}

/**
*@Description : db connection pool - semaphore access 
                initializes semaphore using SETVAL
*@param[in]   : int val 
*@return      : int
*/
int ODBCMUtils::set_semvalue(int val) {
  union semun sem_union;  // sem_union;

  sem_union.val = val;
  if ( semctl ( sem_id, 0, SETVAL, sem_union ) == -1 ) return ( 0 );
  return 1;
}

/**
*@Description : delete semaphore
*@param[in]   : None
*@return      : int 
*/
int ODBCMUtils::del_semvalue() {
  union semun sem_union;  // sem_union;

  sem_union.val = 1;
  if ( semctl ( sem_id, 0, IPC_RMID, sem_union ) == -1 ) return ( 0 );
  return 1;
}

/**
*@Description : set semaphore down
*@param[in]   : None
*@return      : int 
*/
int ODBCMUtils::SEM_DOWN() {
  struct sembuf b;

  b.sem_num = 0;
  b.sem_op = -1;  // P(), i.e. down()
  b.sem_flg = SEM_UNDO;
  if (semop(sem_id, &b, 1)== -1) {
    pfc_log_debug("Semaphore DOWN() failed!");
    return 0;
  }

  return 1;
}

/**
*@Description : set semaphore up 
*@param[in]   : None
*@return      : int 
*/
int ODBCMUtils::SEM_UP() {
  struct sembuf b;

  b.sem_num = 0;  /* Operate on semaphore 0 */
  /* Wait for value to equal 0 */
  b.sem_op =  1;  // V(), i.e. UP()
  b.sem_flg = SEM_UNDO;
  if (semop(sem_id, &b, 1)== -1) {
    pfc_log_debug("Semaphore UP() failed!");
    return 0;
  }
  return 1;
}

}  // namespace uppl
}  // namespace unc
