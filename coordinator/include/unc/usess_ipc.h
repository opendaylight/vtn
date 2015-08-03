/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef _USESS_IPC_H_
#define _USESS_IPC_H_

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

//////////////////////////////
// IPC
//////////////////////////////

// Name of IPC services provided by the UNC daemon.
#define UNCD_IPC_SERVICE "usess"

// Invalid session ID.
#define USESS_ID_INVALID 0

// Fixed session ID.
#define USESS_ID_DB_MGMT  128  // Database Management
#define USESS_ID_LAUNCHER 129  // Launcher
#define USESS_ID_STARTUPPROXY 130  // Startup Proxy
#define USESS_ID_UPLL 131  // UPLL
#define USESS_ID_UPPL 132  // UPPL
#define USESS_ID_TC 133  // TC

// Default user name.
#define USESS_USER_CLI_ADMIN "UNC_CLI_ADMIN"
#define USESS_USER_WEB_ADMIN "UNC_WEB_ADMIN"
#define USESS_USER_WEB_OPER "UNC_WEB_OPER"


// IPC service ID
typedef enum {
  kUsessSessAdd,          // add user authentication and session.
  kUsessSessDel,          // delete session.
  kUsessSessTypeDel,      // delete session of specified session type.
  kUsessEnable,           // authenticate enable.
  kUsessDisable,          // to cancel the state of the enable.
  kUsessSessCount,        // gets number of sessions.
  kUsessSessList,         // gets list of session information.
  kUsessSessDetail,       // gets detailed Session information.
  kUserUserPasswd,        // To change the user password.
  kUserEnablePasswd,      // To change the enable password.
  kUsessIpcNipcs          // number of IPC service ID.
} usess_ipc_service_id_e;


// error code.
typedef enum {
  USESS_E_OK,                     // success.
  USESS_E_NG,                     // error.
  USESS_E_INVALID_SESSID,         // Invalid current session ID.
  USESS_E_NO_SUCH_SESSID,         // Invalid target session ID.
  USESS_E_INVALID_PRIVILEGE,      // Invalid privileges
  USESS_E_INVALID_MODE,           // Invalid mode.
  USESS_E_INVALID_SESSTYPE,       // Invalid session type.
  USESS_E_INVALID_USER,           // Invalid user name.
  USESS_E_INVALID_PASSWD,         // Invalid password.
  USESS_E_SESS_OVER,              // Over the number of user sessions
} usess_ipc_err_e;

// -------------------------------------
// IPC parameter definition.
// -------------------------------------

// Type of user session.
typedef enum {
  USESS_TYPE_UNKNOWN = 0,     // Unknown session type.
  USESS_TYPE_CLI,             // CLI session type.
  USESS_TYPE_CLI_DAEMON,      // Resident CLI session type.
  USESS_TYPE_WEB_API,         // WEB API session type.
  USESS_TYPE_WEB_UI,          // WEB UI session type.
} usess_type_e;


// Type of user.
typedef enum {
  USER_TYPE_UNKNOWN = 0,      // Unknown user.
  USER_TYPE_OPER,             // operator user.
  USER_TYPE_ADMIN             // administrator user.
} user_type_e;


// Mode of user session.
typedef enum {
  USESS_MODE_UNKNOWN = 0,     // Unknown mode.
  USESS_MODE_OPER,            // operator mode.
  USESS_MODE_ENABLE,          // enable mode.
  USESS_MODE_DEL              // Delete session.
} usess_mode_e;


// config mode status.
typedef enum {
  CONFIG_STATUS_NONE = 0,       // Not Configuration mode.
  CONFIG_STATUS_TCLOCK,         // Global Configuration mode.
  CONFIG_STATUS_TCLOCK_PART     // Partial Configuration mode.
} usess_config_mode_e;


// hash type
typedef enum {
  HASH_TYPE_UNKNOWN = 0,
  HASH_TYPE_MD5 = 1,
  HASH_TYPE_SHA256 = 5,
  HASH_TYPE_SHA512 = 6,
} hash_type_e;


#ifdef __cplusplus
}  /* extern "C" */
#endif  /* __cplusplus */

#endif  // _USESS_IPC_H_
