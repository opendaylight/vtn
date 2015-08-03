/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/**
 * @brief   Physical Common Header
 * @file    Physical_common_def.hh
 *
 */

#ifndef _PHYSICAL_COMMON_DEF_HH_
#define _PHYSICAL_COMMON_DEF_HH_
#include <unc/base.h>
#include <unc/unc_events.h>
#include <unc/unc_base.h>
#include <string>

#define UPPL_IPC_NOTIFICATION_HANDLER_NAME  "UpplNotificationManager"
#define PFCDRIVER_IPC_CHN_NAME              "drvpfcd"
#define PFCDRIVER_IPC_SVC_NAME              "pfcdriver"
#define VNPDRIVER_IPC_CHN_NAME              "drvvnpd"
#define VNPDRIVER_IPC_SVC_NAME              "vnpdriver"
#define POLCDRIVER_IPC_CHN_NAME             "drvpolcd"
#define POLCDRIVER_IPC_SVC_NAME             "polcdriver"
#define ODCDRIVER_IPC_CHN_NAME              "drvodcd"
#define ODCDRIVER_IPC_SVC_NAME              "vtndrvintf"
#define UPLL_IPC_CHN_NAME                   "lgcnwd"
#define UPLL_IPC_SVC_NAME                   "upll"
#define TCLIB_MODULE_NAME                   "tclib"
#define CONF_FILE_PATH_SEP                  "/"

const unsigned int UPPL_MAX_REP_CT = 10000;
/*
 * @brief Operation Type
 */

typedef enum {
  CREATEONEROW = 0,
  UPDATEONEROW,
  DELETEONEROW,
  CLEARONEROW,
  GETONEROW,
  GETBULKROWS,
  GETSIBLINGCOUNT,
  GETSIBLINGCOUNT_FILTER,
  GETSIBLINGROWS,
  GETROWCOUNT,
  COPYDATABASE,
  CLEARDATABASE,
  // MERGEDATABASE,
  ISCANDIDATEDIRTY,
  ISROWEXISTS,
  GETMODIFIEDROWS,
  COMMITALLCONFIG,
  CLEARONEINSTANCE,
  CLEARALLROWS
}UpplDbOperationType;

typedef enum {
  ODBCM_RC_SQL_INVALID_HANDLE = -2,
  ODBCM_RC_QUERY_FAILED = -1,
  ODBCM_RC_SUCCESS = 0,
  ODBCM_RC_SUCCESS_WITH_INFO,
  ODBCM_RC_QUERY_STILL_EXECUTING = 2,
  ODBCM_RC_FAILED,
  ODBCM_RC_COMMON_LINK_FAILURE,
  ODBCM_RC_CONNECTION_ERROR,
  ODBCM_RC_CONNECTION_TIMEOUT,
  ODBCM_RC_CONNECTION_IN_USE,
  ODBCM_RC_SERIALIZATION_ERROR,
  ODBCM_RC_INVALID_CONN_HANDLE,
  ODBCM_RC_QUERY_TIMEOUT,
  ODBCM_RC_ERROR_IN_FRAMEQUERY,
  ODBCM_RC_INVALID_TABLE_NAME,
  ODBCM_RC_INVALID_DB_OPERATION,
  ODBCM_RC_PKEY_VIOLATION,
  ODBCM_RC_MEMORY_ERROR,
  ODBCM_RC_TABLE_NOT_FOUND,
  ODBCM_RC_RECORD_NOT_FOUND,
  ODBCM_RC_DATA_ERROR,
  ODBCM_RC_RECORD_NO_MORE,
  ODBCM_RC_NO_RECORD,
  ODBCM_RC_ERROR_FETCHING_ROW,
  ODBCM_RC_STMT_ERROR,
  ODBCM_RC_DISCONNECT_ERROR,
  ODBCM_RC_RECORD_ALREADY_EXISTS,
  ODBCM_RC_CONN_ENV_ERROR,
  ODBCM_RC_CONN_HANDLE_ERROR,
  ODBCM_RC_GENERAL_ERROR,
  ODBCM_RC_PARAM_BIND_ERROR,
  ODBCM_RC_WRONG_PARAM,
  ODBCM_RC_MORE_ROWS_FOUND,
  ODBCM_RC_ROW_EXISTS,
  ODBCM_RC_ROW_NOT_EXISTS,
  ODBCM_RC_CANDIDATE_DIRTY,
  ODBCM_RC_CANDIDATE_NO_DIRTY,
  ODBCM_RC_SQL_ERROR,
  ODBCM_RC_ROW_STATUS_NOT_FOUND,
  ODBCM_RC_COLUMN_DOES_NOT_MATCH,
  ODBCM_RC_PREPARED_STMT_ERROR,
  ODBCM_RC_TYPE_ATTR_VIOLATION,
  ODBCM_RC_INVALID_DESC,
  ODBCM_RC_UNABLE_ESTABLISH_CONN,
  ODBCM_RC_CONNECTION_REJECTED,
  ODBCM_RC_INSERT_VAL_LIST_NOT_MATCHED,
  ODBCM_RC_DATA_TRUNCATION_ERROR,
  ODBCM_RC_VARIABLE_NOT_SUPPLIED,
  ODBCM_RC_VALUE_OUT_OF_RANGE,
  ODBCM_RC_DATETIME_ERROR,
  ODBCM_RC_DIVISIBLE_ERROR,
  ODBCM_RC_FIELD_OVERFLOW,
  ODBCM_RC_INVALID_CHAR_SPEC,
  ODBCM_RC_CURSOR_STATE,
  ODBCM_RC_INVALID_CURSOR,
  ODBCM_RC_SYNTAX_ERROR,
  ODBCM_RC_INDEX_NOT_FOUND,
  ODBCM_RC_COLUMN_ALREADY_EXISTS,
  ODBCM_RC_COLUMN_NOT_FOUND,
  ODBCM_RC_NULL_POINTER_ERROR,
  ODBCM_RC_FUNC_SEQUENCE_ERROR,
  ODBCM_RC_TRANSACTION_ERROR,
  ODBCM_RC_TABLE_EXISTS,
  ODBCM_RC_COLUMN_ALREADY,
  ODBCM_RC_SQL_NEED_DATA = 99,
  ODBCM_RC_SQL_NO_DATA = 100,
  ODBCM_RC_DEFAULT
}ODBCM_RC_STATUS;

/*
 * @brief Request Attribute Datatype
 */
typedef enum {
  DATATYPE_UINT16 = 0,
  DATATYPE_UINT64,
  DATATYPE_UINT32,
  DATATYPE_IPV4,
  DATATYPE_IPV6,
  DATATYPE_UINT8_ARRAY_1,
  DATATYPE_UINT8_ARRAY_2,
  DATATYPE_UINT8_ARRAY_3,
  DATATYPE_UINT8_ARRAY_6,
  DATATYPE_UINT8_ARRAY_8,
  DATATYPE_UINT8_ARRAY_10,
  DATATYPE_UINT8_ARRAY_11,
  DATATYPE_UINT8_ARRAY_16,
  DATATYPE_UINT8_ARRAY_32,
  DATATYPE_UINT8_ARRAY_128,
  DATATYPE_UINT8_ARRAY_256,
  DATATYPE_UINT8_ARRAY_257,
  DATATYPE_UINT8_ARRAY_320
}AttributeDataType;

/*
 * @brief Transaction State
 */

typedef enum {
  TRANS_END = 0,
  TRANS_START,
  TRANS_START_SUCCESS,
  VOTE_BEGIN,
  VOTE_WAIT_DRIVER_RESULT,
  VOTE_SUCCESS,
  GLOBAL_COMMIT_BEGIN,
  GLOBAL_COMMIT_WAIT_DRIVER_RESULT,
  GLOBAL_COMMIT_DRIVER_RESULT,
  GLOBAL_COMMIT_SUCCESS
}TransState;

/*
 * @brief unc mode 
 */
typedef enum {
  UNC_SEPARATE_MODE = 0,
  UNC_COEXISTS_MODE
}UncMode;

/*
 *  @brief Driver Response Status
 */

typedef enum {
  SUCCESS = 0,
  FAILURE,
  DISCONNECT
}DriverResponseStatus;

struct physical_request_header {
  uint32_t client_sess_id;
  uint32_t config_id;
  uint32_t operation;
  uint32_t max_rep_count;
  uint32_t option1;
  uint32_t option2;
  uint32_t data_type;
  uint32_t key_type;
};

struct physical_response_header {
  uint32_t client_sess_id;
  uint32_t config_id;
  uint32_t operation;
  uint32_t max_rep_count;
  uint32_t option1;
  uint32_t option2;
  uint32_t data_type;
  uint32_t result_code;
};

struct driver_request_header {
  uint32_t client_sess_id;
  uint32_t config_id;
  std::string controller_id;
  std::string domain_id;
  uint32_t operation;
  uint32_t max_rep_count;
  uint32_t option1;
  uint32_t option2;
  uint32_t data_type;
  uint32_t key_type;
};

struct driver_response_header {
  uint32_t client_sess_id;
  uint32_t config_id;
  std::string controller_id;
  std::string domain_id;
  uint32_t operation;
  uint32_t max_rep_count;
  uint32_t option1;
  uint32_t option2;
  uint32_t data_type;
  uint32_t result_code;
};

struct driver_event_header {
  std::string controller_id;
  std::string domain_id;
  uint32_t operation;
  uint32_t data_type;
  uint32_t key_type;
};

struct lp_struct {
  uint8_t switch_id[256];
  uint8_t port_id[32];
  uint8_t port_type;
};

struct lmp_struct {
  uint8_t switch_id[256];
  uint8_t port_id[32];
};

struct port_struct {
  uint8_t port_id[32];
};

struct boundary_val {
  uint8_t controller_type;
  uint8_t controller_name[32];
  uint8_t domain_name[32];
  uint8_t logical_port_id[320];
  lp_struct lp_str;
  uint8_t boundary_id[32];
};

struct boundary_record {
  uint8_t boundary_id[32];
  uint8_t ctr_name1[32];
  uint8_t dom_name1[32];
  uint8_t lp_id1[320];
  uint8_t port_type1;
  uint8_t sw_id1[256];
  uint8_t port_id1[32];
  bool is_filled1;
  uint8_t ctr_name2[32];
  uint8_t dom_name2[32];
  uint8_t lp_id2[320];
  uint8_t port_type2;
  uint8_t sw_id2[256];
  uint8_t port_id2[32];
  bool is_filled2;
};

struct driver_alarm_header {
  std::string controller_id;
  std::string domain_id;
  uint32_t operation;
  uint32_t data_type;
  uint32_t key_type;
  uint32_t alarm_type;
};

struct northbound_event_header {
  uint32_t operation;
  uint32_t data_type;
  uint32_t key_type;
};

struct northbound_alarm_header {
  uint32_t operation;
  uint32_t data_type;
  uint32_t key_type;
  uint32_t alarm_type;
};
#endif  // _PHYSICAL_COMMON_DEF_HH_
