/*
 * Copyright (c) 2012-2014 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * dal_error_handler.cc
 *   contians implementation of DalErrorHandler
 */

#include <stdint.h>
#include <cstring>
#include "uncxx/upll_log.hh"
#include "dal_error_handler.hh"

namespace unc {
namespace upll {
namespace dal {

std::map<std::string, DalResultCode> DalErrorHandler::err_map_;
bool DalErrorHandler::err_map_filled_;

// Error Code Map b/w SQLSTATE and DalResultCode
// Commented out kDalRcGeneralError cases for efficiency
static const DalErrMap dal_err_map[] = {
  // 01 prefix is for warnings - SQL_SUCCESS_WITH_INFO
  // 00000 - Success with info
  {"00000", kDalRcSuccess},
  // 01000 General Warning
  {"01000", kDalRcSuccess},
  // 01001 Cursor operation conflict
  {"01001", kDalRcGeneralError},
  // 01002 Disconnect error
  {"01002", kDalRcConnNotAvailable},
  // 01003 NULL value eliminated in set function
  {"01003", kDalRcDataError},
  // 01004 String data, right-truncated
  {"01004", kDalRcDataError},
  // 01006 Privilege not revoked
  {"01006", kDalRcGeneralError},
  // 01S00 Invalid connection string attribute
  {"01S00", kDalRcConnNotEstablished},
  // 01S01 Error in row
  {"01S01", kDalRcGeneralError},
  // 01S02 Option value changed
  {"01S02", kDalRcGeneralError},
  // 01S06 Attempt to fetch before the result set returned the first rowset
  {"01S06", kDalRcGeneralError},
  // 01S07 Fractional truncation
  {"01S07", kDalRcGeneralError},
  // 01S08 Error saving File DSN
  {"01S08", kDalRcConnNotEstablished},
  // 01S09 Invalid keyword
  {"01S09", kDalRcConnNotEstablished},

  // 07 prefix is for all input errors
  // 07001 Wrong number of parameters
  {"07001", kDalRcDataError},
  // 07002 COUNT field incorrect
  {"07002", kDalRcDataError},
  // 07005 Prepared statement not a cursor
  // 07005 May not happen in this application
  // 07005 Happening for APIs which are not used in this appln
  {"07005", kDalRcDataError},
  // 07006 Restricted Datatype attribute violation
  {"07006", kDalRcDataError},
  // 07009 Invalid descriptor index
  {"07009", kDalRcDataError},
  // 07S01 Invalid use of default parameter
  {"07S01", kDalRcDataError},

  // 08 prefix is for all connection errors
  // 08001 Client unable to establish connection
  {"08001", kDalRcConnNotEstablished},
  // 08002 Connection name in use
  {"08002", kDalRcConnNotEstablished},
  // 08003 Connection not open
  {"08003", kDalRcConnNotAvailable},
  // 08004 Server rejected the connection
  {"08004", kDalRcConnNotEstablished},
  // 08007 Connection failure during transaction
  {"08007", kDalRcConnNotAvailable},
  // 08007 Communication link failure
  {"08S01", kDalRcConnNotAvailable},

  // 21S01 Insert value list does not match column list
  // DAL should have handled this, If this error happens, it is coding error
  {"21S01", kDalRcInternalError},
  // 21S02 Degree of derived table does not match column list
  {"21S02", kDalRcGeneralError},

  // 22 prefix for Input/Output Data related errors
  // 22001 String data, right-truncated
  {"22001", kDalRcDataError},
  // 22002 Indicator variable required but not supplied
  {"22002", kDalRcDataError},
  // 22003 Numeric value out of range
  {"22003", kDalRcDataError},
  // 22007 Invalid datetime format
  // Should not happen - This appln did not use Date/Time type
  {"22007", kDalRcDataError},
  // 22008 Datetime field overflow
  // Should not happen - This appln did not use Date/Time type
  {"22008", kDalRcDataError},
  // 22012 Division by zero
  {"22012", kDalRcDataError},
  // 22015 Interval field overflow
  {"22015", kDalRcDataError},
  // 22018 Invalid character value for cast specification
  {"22018", kDalRcDataError},
  // 22019 Invalid escape character
  {"22019", kDalRcDataError},
  // 22025 Invalid escape sequence
  {"22025", kDalRcDataError},
  // 22026 String data, length mismatch
  // 22026 May not happen in this application
  // 22026 Happening for APIs which are not used in this appln
  {"22026", kDalRcDataError},

  // 23000 Integrity Constraint - may not happen in this appln
  {"23000", kDalRcGeneralError},
  // 23503 Foreign Key violation
  {"23503", kDalRcParentNotFound},
  // 23505 Unique Constraint violation
  {"23505", kDalRcRecordAlreadyExists},

  // 24000 Invalid Cursor State
  {"24000", kDalRcInvalidCursor},

  // 25000 Invalid Txn state during SQLDisconnect
  {"25000", kDalRcNotDisconnected},
  // 25S prefix for Txn related errors
  // 25S01 Transaction state
  {"25S01", kDalRcTxnError},
  // 25S02 Transaction is still active
  {"25S02", kDalRcTxnError},
  // 25S03 Transaction is rolled back
  {"25S03", kDalRcTxnError},

  // 28000 Invalid authorization
  {"28000", kDalRcConnNotEstablished},

  // 34000 Invalid cursor name
  // 34000 Happen when Query contains cursor name
  // 34000 May not happen in this application
  {"34000", kDalRcInvalidCursor},

  // 3C000 Duplicate cursor name
  // 3C000 May not happen in this application
  // 3C000 Happening for APIs which are not used in this appln
  {"3C000", kDalRcInvalidCursor},

  // 3D000 Invalid catalog name
  // 3D000 Happen when Query contains catalog name
  // 3D000 May not happen in this application
  {"3D000", kDalRcGeneralError},

  // 3F000 Invalid schema name
  // 3F000 Happen when Query contains schema name
  // 3F000 May not happen in this application
  {"3F000", kDalRcGeneralError},

  // 40  Prefix for Txn related errors
  // 40001 Serialization failure
  {"40001", kDalRcTxnError},
  // 40002 Integrity constraint violation
  {"40002", kDalRcTxnError},
  // 40003 Statement completion unknown
  {"40003", kDalRcTxnError},

  // 42  Prefix for Syntax related errors
  // DAL should have handled this, If this error happens, it is coding error
  // 42000 Syntax error or access violation
  {"42000", kDalRcInternalError},
  // 42S01 Base table or view already exists
  {"42S01", kDalRcGeneralError},
  // 42S02 Base table or view not found
  {"42S02", kDalRcGeneralError},
  // 42S11 Index already exists
  {"42S11", kDalRcGeneralError},
  // 42S12 Index not found
  {"42S12", kDalRcGeneralError},
  // 42S21 Column already exists
  {"42S21", kDalRcGeneralError},
  // 42S22 Column not found
  {"42S22", kDalRcGeneralError},

  // 44000 WITH CHECK OPTION violation
  // WITH CHECK OPTION not used in this appln
  {"44000", kDalRcGeneralError},

  // HY000 General Error
  {"HY000", kDalRcGeneralError},
  // HY001 Memory Error
  {"HY001", kDalRcMemoryError},
  // HY003 Invalid Appln Buffer
  {"HY003", kDalRcDataError},
  // HY004 Invalid SQL Datatype
  {"HY004", kDalRcDataError},
  // HY007 Associated statement is not prepared
  // HY007 Happening for APIs which are not used in this appln
  {"HY007", kDalRcGeneralError},
  // HY008 Operation Cancelled - Errors during Asynchronous operation
  {"HY008", kDalRcGeneralError},
  // HY009 Invalid use of null pointer
  {"HY009", kDalRcDataError},
  // HY010 Function Sequence error
  {"HY010", kDalRcInternalError},
  // HY011 Attribute cannot be set now
  {"HY011", kDalRcInternalError},
  // HY012 Invalid transaction operation code
  {"HY012", kDalRcTxnError},
  // HY013 Memory Management error
  {"HY013", kDalRcMemoryError},
  // HY014 Limit on the number of handles
  {"HY014", kDalRcGeneralError},
  // HY015 No cursor name available - may not happen in this appln
  {"HY015", kDalRcInvalidCursor},
  // HY016 Cannot modify an implementation row descriptor
  {"HY016", kDalRcGeneralError},
  // HY017 Invalid use of an automatically allocated descriptor handle
  // Descriptor Handle not used in this appln
  {"HY017", kDalRcGeneralError},
  // HY018 Server declined cancel request
  // Happening for APIs which are not used in this appln
  {"HY018", kDalRcGeneralError},
  // HY019 Non-character and non-binary data sent in pieces
  // Happening for APIs which are not used in this appln
  {"HY019", kDalRcGeneralError},
  // HY020 Attempt to concatenate a null value
  // Happening for APIs which are not used in this appln
  {"HY020", kDalRcGeneralError},
  // HY021 Inconsistent descriptor information
  // Descriptor Handle not used in this appln
  {"HY021", kDalRcGeneralError},
  // HY024 Invalid attribute value
  {"HY024", kDalRcInternalError},
  // HY090 Invalid string or buffer length
  {"HY090", kDalRcDataError},
  // HY095 Function type out of range
  // Happening for APIs which are not used in this appln
  {"HY095", kDalRcGeneralError},
  // HY096 Invalid information type
  // Happening for APIs which are not used in this appln
  {"HY096", kDalRcGeneralError},
  // HY097 Column type out of range
  // Happening for APIs which are not used in this appln
  {"HY097", kDalRcGeneralError},
  // HY098 Scope type out of range
  // Happening for APIs which are not used in this appln
  {"HY098", kDalRcGeneralError},
  // HY099 Nullable type out of range
  // Happening for APIs which are not used in this appln
  {"HY099", kDalRcGeneralError},
  // HY100 Uniqueness option type out of range
  // Happening for APIs which are not used in this appln
  {"HY100", kDalRcGeneralError},
  // HY101 Accuracy option type out of range
  // Happening for APIs which are not used in this appln
  {"HY101", kDalRcGeneralError},
  // HY103 Invalid retrieval code
  // Happening for APIs which are not used in this appln
  {"HY103", kDalRcGeneralError},
  // HY104 Invalid precison or scale type
  {"HY104", kDalRcDataError},
  // HY105 Invalid parameter type
  {"HY105", kDalRcDataError},
  // HY106 Fetch type out of range
  {"HY106", kDalRcInvalidCursor},
  // HY107 Row value out of range
  {"HY107", kDalRcInvalidCursor},
  // HY109 Invalid cursor position
  {"HY109", kDalRcInvalidCursor},
  // HY110 Invalid driver completion
  // Happening for APIs which are not used in this appln
  {"HY110", kDalRcGeneralError},
  // HY111 Invalid bookmark value
  // Happening for APIs which are not used in this appln
  {"HY111", kDalRcGeneralError},
  // HYC00 optional feature not implemented
  {"HYC00", kDalRcGeneralError},
  // HYT00 Timeout expired
  {"HYT00", kDalRcQueryTimeOut},
  // HYT01 Connection Timeout expired
  {"HYT01", kDalRcConnTimeOut}
  // IM Prefix for Driver specific error
  // returning kDalRcGeneralError for these states
  // {"IM", kDalRcGeneralError},
};  // static const DalErrMap dal_err_map

// Public Methods of DalErrorHandler
// Process errors from ODBC Driver Manager and maps to DalResultCode
void
DalErrorHandler::ProcessOdbcErrors(const SQLSMALLINT handle_type,
                                   const SQLHANDLE handle,
                                   const SQLRETURN sql_rc,
                                   DalResultCode *dal_rc) {
  switch (sql_rc) {
    // Success
    case SQL_SUCCESS:
      *dal_rc = kDalRcSuccess;
      return;

    // Invalid Handle
    case SQL_INVALID_HANDLE:
      switch (handle_type) {
        case SQL_HANDLE_ENV:
          UPLL_LOG_INFO("Invalid environment handle");
          *dal_rc = kDalRcGeneralError;
          return;

        case SQL_HANDLE_DESC:
          UPLL_LOG_INFO("Invalid descriptor handle");
          *dal_rc = kDalRcGeneralError;
          return;

        case SQL_HANDLE_DBC:
          UPLL_LOG_INFO("Invalid connection handle");
          *dal_rc = kDalRcInvalidConnHandle;
          return;

        case SQL_HANDLE_STMT:
          UPLL_LOG_INFO("Invalid statement handle");
          *dal_rc = kDalRcGeneralError;
          return;

        default:
          UPLL_LOG_INFO("Unknown handle type");
          *dal_rc = kDalRcGeneralError;
          return;
      }  // switch (handle_type)

    case SQL_NEED_DATA:
      UPLL_LOG_INFO("Need Data");
      *dal_rc = kDalRcDataError;
      return;

    // case SQL_PARAM_DATA_AVAILABLE: available in ODBC not in unixodbc

    // Asynchronous case
    case SQL_STILL_EXECUTING:
      UPLL_LOG_INFO("Still Executing");
      *dal_rc = kDalRcGeneralError;
      return;

    // No output in the result set
    case SQL_NO_DATA:
      UPLL_LOG_DEBUG("No Data");
      *dal_rc = kDalRcRecordNotFound;
      return;

    // SQL_NULL_DATA and SQL_ERROR has same value (-1)
    case SQL_SUCCESS_WITH_INFO:
    case SQL_ERROR:
      {
        SQLRETURN rc = SQL_SUCCESS;
        uint16_t rec_no = 1;
        const uint16_t kDalSqlStateLen = 6;
        const uint16_t kDalSqlErrMsgLen = 256;
        SQLCHAR sql_state[kDalSqlStateLen];
        SQLCHAR err_msg[kDalSqlErrMsgLen];
        SQLINTEGER err_code = 0;
        SQLSMALLINT err_msg_len = 0;

        do {
          err_code = 0;
          memset(sql_state, 0, kDalSqlStateLen);
          memset(err_msg, 0, kDalSqlErrMsgLen);
          rc = SQLGetDiagRec(handle_type, handle, rec_no, sql_state, &err_code,
                             err_msg, sizeof(err_msg), &err_msg_len);
// #ifdef UNC_LP64
          UPLL_LOG_DEBUG("DB Error details[%d] - %s:%"UNC_PFMT_SQLINTEGER":%s",
                         rec_no, sql_state, err_code, err_msg);
// #endif
          rec_no++;
        } while (rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO);

        memset(sql_state, 0, kDalSqlStateLen);
        memset(err_msg, 0, kDalSqlErrMsgLen);
        rc = SQLGetDiagRec(handle_type, handle, 1, sql_state,
                  &err_code, err_msg, sizeof(err_msg), &err_msg_len);

        if (rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO) {
          UPLL_LOG_INFO("Error retrieving diag information");
          *dal_rc = kDalRcGeneralError;
          return;
        }

        *dal_rc = DalErrorHandler::FindDalResultCode(sql_state);
      }
      return;

    default:
      UPLL_LOG_INFO("Unknown return code");
      *dal_rc = kDalRcGeneralError;
      return;
  }  // switch(sql_rc)

  return;
}  // DalOdbcMgr::ProcessOdbcErrors


// One time loading of error map
bool
DalErrorHandler::FillErrorMap() {
  if (err_map_filled_ != true) {
    UPLL_LOG_TRACE("One time loading of error map");
    size_t i = 0;
    size_t count = sizeof(dal_err_map)/sizeof(DalErrMap);
    std::pair<std::map<std::string, DalResultCode>::iterator, bool> rc;

    for (i = 0; i < count; i++) {
    // fill map data
      rc = err_map_.insert(std::pair<std::string, DalResultCode>
                             (dal_err_map[i].sql_state,
                              dal_err_map[i].dal_rc));
      if (rc.second == false) {
        UPLL_LOG_DEBUG("Error inserting SQLState - %s",
                       dal_err_map[i].sql_state.c_str());
        return false;
      }
    }
    err_map_filled_ = true;
    UPLL_LOG_TRACE("Error Map for DAL loaded");
  }
  return true;
}  // DalErrorHandler::FillErrorMap

// Clearing the error map
void
DalErrorHandler::ClearErrorMap() {
  err_map_.clear();
  UPLL_LOG_TRACE("Error Map for DAL cleared");
}  // DalErrorHandler::ClearErrorMap

// Private methods of DalErrorHandler
// Find DalResultCode from SQLState
inline DalResultCode
DalErrorHandler::FindDalResultCode(const SQLCHAR *sql_state) {
  const std::string state(reinterpret_cast<const char *>(sql_state));
  std::map<std::string, DalResultCode>::iterator iter;
  iter = err_map_.find(state);
  if (iter == err_map_.end()) {
    UPLL_LOG_INFO("Warning - Error code data not available"
                  " Returning General Error");
    return kDalRcGeneralError;
  }
  return iter->second;
}  // DalErrorHandler::FindDalResultCode

}  // namespace dal
}  // namespace upll
}  // namespace unc
