/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef _UNC_TCLIB_STRUCT_DEFS_HH_
#define _UNC_TCLIB_STRUCT_DEFS_HH_

#include <unc/keytype.h>
#include <unc/tc/external/tc_services.h>
#include <list>
#include <string>

namespace unc {
namespace tclib {

#define TC_DEFAULT_VALUE 0U
#define DRV_RESULT_START_POS 3U

/*
 * Filled for transaction start, vote request, global commit
 * of UPLL/UPPPL and transaction end oper types in
 * commit process
 */
typedef struct {
  TcMsgOperType oper_type;
  uint32_t session_id;
  uint32_t config_id;
  TcTransEndResult end_result;
}TcCommitTransactionMsg;

/*
 * Filled for driver vote request and global commit
 * oper types in commit process
 */
typedef struct {
  TcMsgOperType oper_type;
  uint32_t session_id;
  uint32_t config_id;
  uint8_t cntrl_count; /* number of controllers in list */
  TcControllerList cntrl_list;
}TcCommitDrvVoteGlobalMsg;

/*
 * Filled for GlobalAbort in commit process
 */
typedef struct {
  TcMsgOperType oper_type;
  uint32_t session_id;
  uint32_t config_id;
  TcCommitOpAbortPhase commit_oper_phase;
}TcCommitGlobalAbortMsg;

/*
 * Filled for driver result handling in commit process
 */
typedef struct {
  TcMsgOperType oper_type;
  uint32_t session_id;
  uint32_t config_id;
  TcCommitPhaseType phase; /* phase of driver result vote/global */
} TcCommitDrvResultMsg;

/*
 * Filled for audit start, transaction start, vote request,
 * global commit of UPLL/UPPPL and transaction end oper types in
 * audit process
 */
typedef struct {
  TcMsgOperType oper_type;
  uint32_t session_id;
  unc_keytype_ctrtype_t driver_id;
  std::string controller_id;
  uint8_t reconnect_controller; /*filled only for audit start*/
  TcAuditResult audit_result; /* filled only for audit end */
  TcTransEndResult end_result; /* filled for transaction end */
  TcAuditType audit_type; /*filled only for audit start*/
  uint64_t commit_number; /*Filled only for audit start */
  uint64_t commit_date; /*Filled only for audit start */
  std::string commit_application; /*Filled only for audit start */
}TcAuditTransactionMsg;

/*
 * Filled for driver vote request and global commit
 * oper types in audit process
 */
typedef struct {
  TcMsgOperType oper_type;
  uint32_t session_id;
  std::string controller_id;
  uint8_t cntrl_count; /* number of controllers in list */
  TcControllerList cntrl_list;
}TcAuditDrvVoteGlobalMsg;

/*
 * Filled for driver result handling in audit process
 */
typedef struct {
  TcMsgOperType oper_type;
  uint32_t session_id;
  std::string controller_id;
  TcCommitPhaseType phase; /* phase of driver result vote/global */
}TcAuditDrvResultMsg;

/*
 * Filled for GlobalAbort in audit process
 */
typedef struct {
  TcMsgOperType oper_type;
  uint32_t session_id;
  unc_keytype_ctrtype_t driver_id;
  std::string controller_id;
  TcAuditOpAbortPhase audit_oper_phase;
}TcAuditGlobalAbortMsg;

/*
 * Filled for start up message
 */
typedef struct {
  TcMsgOperType oper_type;
  uint32_t session_id;
}TcStartupMsg;

/*
 * Filled for user abort candidate message
 */
typedef struct {
  uint32_t session_id;
  uint32_t config_id;
}TcUserAbortMsg;

/*
 * Filled for user audit config message
 */
typedef struct {
  unc_keytype_datatype_t db_type;
  TcServiceType service_type;
  TcConfigMode config_mode;
  std::string vtn_name;
  uint64_t version;
}TcAuditConfigMsg;
}  // tclib
}  // unc

#endif /* _UNC_TCLIB_STRUCT_DEFS_HH_ */
