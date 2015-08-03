/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */


#ifndef _UNC_TCLIB_DEFS_HH_
#define _UNC_TCLIB_DEFS_HH_


#include<map>
#include<list>
#include<vector>
#include<string>
#include<stdint.h>
#include <unc/keytype.h>
#include <unc/tc/external/tc_services.h>

namespace unc {
namespace tclib {

/* default argument count */
#define IPC_DEFAULT_ARG_COUNT 0
/* supported tclib services */
#define TCLIB_IPC_SERVICES    20

/*
 * All Virtual Functions of TcLibInterface returns with 
 * this type. Based on this actions will be taken care in TC
 */
typedef enum {
  TC_SUCCESS = 0,
  TC_FAILURE,
  TC_SIMPLIFIED_AUDIT,
  TC_CANCELLED_AUDIT,
  TC_LAST_DB_OPER_FAILURE
}TcCommonRet;

/*
 * Message oper types filled from tc towards tclib 
 * This oper types decides the which handle tclib interface
 * function to be invoked.
 */
/****************************************************
 *
 * NOTE: When adding new enum here, add the string in
 *       tclib_module.cc TcMsgOperTypeString too
 *
 ****************************************************/
typedef enum {
  MSG_NONE = 0,
  MSG_NOTIFY_CONFIGID,
  MSG_SETUP,
  MSG_SETUP_COMPLETE,
  MSG_COMMIT_TRANS_START,
  MSG_COMMIT_VOTE,
  MSG_COMMIT_DRIVER_VOTE,
  MSG_COMMIT_VOTE_DRIVER_RESULT,
  MSG_COMMIT_GLOBAL,
  MSG_COMMIT_DRIVER_GLOBAL,
  MSG_COMMIT_GLOBAL_DRIVER_RESULT,
  MSG_COMMIT_TRANS_END,
  MSG_AUDIT_START,
  MSG_AUDIT_TRANS_START,
  MSG_AUDIT_VOTE,
  MSG_AUDIT_DRIVER_VOTE,
  MSG_AUDIT_VOTE_DRIVER_RESULT,
  MSG_AUDIT_GLOBAL,
  MSG_AUDIT_DRIVER_GLOBAL,
  MSG_AUDIT_GLOBAL_DRIVER_RESULT,
  MSG_AUDIT_TRANS_END,
  MSG_AUDIT_END,
  MSG_AUDIT_CANCEL,
  MSG_SAVE_CONFIG,
  MSG_CLEAR_CONFIG,
  MSG_ABORT_CANDIDATE,
  MSG_AUDITDB,
  MSG_COMMIT_ABORT,
  MSG_AUDIT_ABORT,
  MSG_GET_DRIVERID,
  MSG_CONTROLLER_TYPE,
  MSG_AUTOSAVE_ENABLE,
  MSG_AUTOSAVE_DISABLE,
  MSG_MAX
}TcMsgOperType;

/*
 * services provided in tclib 
 * some services are combination of multiple operations.
 * For example:
 * TCLIB_COMMIT_TRANSACTION combination of transaction start, transaction end
 * for all channels and vote request, global commit for UPLL and UPPL.
 */
typedef enum {
  TCLIB_NOTIFY_SESSION_CONFIG = 0,
  TCLIB_COMMIT_TRANSACTION,
  TCLIB_COMMIT_DRV_VOTE_GLOBAL,
  TCLIB_COMMIT_DRV_RESULT,
  TCLIB_COMMIT_GLOBAL_ABORT,
  TCLIB_AUDIT_TRANSACTION,
  TCLIB_AUDIT_DRV_VOTE_GLOBAL,
  TCLIB_AUDIT_DRV_RESULT,
  TCLIB_AUDIT_GLOBAL_ABORT,
  TCLIB_AUDIT_CANCEL,
  TCLIB_SAVE_CONFIG,
  TCLIB_CLEAR_STARTUP,
  TCLIB_USER_ABORT,
  TCLIB_SETUP,
  TCLIB_SETUP_COMPLETE,
  TCLIB_GET_DRIVERID,
  TCLIB_AUDIT_CONFIG,
  TCLIB_CONTROLLER_TYPE,
  TCLIB_AUTOSAVE_ENABLE,
  TCLIB_AUTOSAVE_DISABLE
}TcLibServiceId;

/*
 * Enum used for handle driver result
 * phase type indicates in which phase the handle driver result
 * has invoked for UPLL and UPPL for both commit and audit operations 
 */
typedef enum {
  TC_COMMIT_VOTE_PHASE = 0,
  TC_COMMIT_GLOBAL_COMMIT_PHASE,
  TC_AUDIT_VOTE_PHASE,
  TC_AUDIT_GLOBAL_COMMIT_PHASE
}TcCommitPhaseType;

/*
 * phase in which abort operation has invoked 
 * currently, only in vote request phase abort operation
 * will be invoked.
 */
typedef enum {
  COMMIT_TRANSACTION_START = 0,
  COMMIT_VOTE_REQUEST
}TcCommitOpAbortPhase;

/*
 * phase in which abort operation has invoked 
 * currently, only in vote request phase abort operation
 * will be invoked.
 */
typedef enum {
  AUDIT_START = 0,
  AUDIT_TRANSACTION_START,
  AUDIT_VOTE_REQUEST
}TcAuditOpAbortPhase;

/*
 * audit result will be sent from tc in audit end oper type
 * This will be decided tc based on audit operations
 */
typedef enum {
  TC_AUDIT_FAILURE = 0,
  TC_AUDIT_SUCCESS,
  TC_AUDIT_CANCELLED,
  TC_SIMPLIFIED_AUDIT_SUCCESS
}TcAuditResult;

/*
 * end result will be sent from tc in transaction end oper type
 * This will be decided tc based on transaction operations 
 */
typedef enum {
  TRANS_END_FAILURE = 0,
  TRANS_END_SUCCESS
}TcTransEndResult;

// API related definitions
/*
 * used by tclib api functions
 * based on this actions will be taken care in
 * respective invoked areas
 */
typedef enum {
  TC_API_COMMON_SUCCESS = 0,
  TC_API_COMMON_FAILURE,
  TC_INVALID_PARAM,
  TC_HANDLER_ALREADY_ACTIVE,
  TC_INVALID_SESSION_ID,
  TC_INVALID_CONFIG_ID,
  TC_INVALID_MODE,
  TC_INVALID_CONTROLLER_ID,
  TC_INVALID_OPER_STATE,
  TC_INVALID_KEY_TYPE,
  TC_INVALID_KEY_DATA,
  TC_INVALID_VALUE_DATA,
  TC_INVALID_KEY_STDEF,
  TC_INVALID_VALUE_STDEF,
  TC_API_FATAL
}TcApiCommonRet;

/*
 * handle driver result function uses this structure
 * tclib constructs this structure based on the driver result
 * message request from tc 
 */
typedef struct {
  std::string controller_id;
  uint32_t  resp_code;
  uint32_t  num_of_errors;
  std::list<uint32_t> key_list;
  uint64_t commit_number;
  uint64_t commit_date;
  std::string commit_application;
}TcControllerResult;

/*
 * Driver info map is combination of controller type and its assosciated
 * controllers. Filled from UPLL and UPPL in vote request and global commit
 * phases
 */
typedef std::map<unc_keytype_ctrtype_t, std::vector<std::string> >
                 TcDriverInfoMap;

/*
 * list of controllers.
 */
typedef std::list<std::string> TcControllerList;

/*
 * vector of controller results.
 * For handle driver result to UPLL and UPPL, this vector will be filled and
 * sent in handler functions.
 */
typedef std::vector<TcControllerResult> TcCommitPhaseResult;

/*
 * keytype and its assosciated index in session
 * This map filled for handle driver result request. 
 * based on request to keytype, index will be retrieved.
 */
typedef std::map<uint32_t, uint32_t>TcKeyTypeIndexMap;

/*
 * controllerid and assosciated keyindex map 
 * based on request to controllerid, keytypeindex map will be retrieved
 */
typedef std::map<std::string, TcKeyTypeIndexMap> TcControllerKeyTypeMap;

/*
 * tc config data structure
 */

struct TcNotifyConfigData {
  uint32_t session_id_;
  uint32_t config_id_;
  TcConfigMode config_mode_;
  std::string vtn_name_;
TcNotifyConfigData()
  : session_id_(0), config_id_(0)
  {}
};

/*
 * session_id and assosciated config data map
 * based on request to session_id, config data map will be retrieved
 */
typedef std::map<uint32_t, TcNotifyConfigData> TcNotifyConfigDataMap;
}  // tclib
}  // unc

#endif /* _TCLIB_DEFS_HH */
