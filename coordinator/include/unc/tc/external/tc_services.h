/*
 * Copyright (c) 2012-2014 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef SRC_INCLUDE_UNC_TC_EXTERNAL_TC_SERVICES_H_
#define SRC_INCLUDE_UNC_TC_EXTERNAL_TC_SERVICES_H_

#include <unc/unc_base.h>

/* Values used to establish Client Session with TC Module */
#define UNC_CHANNEL_NAME "uncd"
#define TC_SERVICE_NAME  "tc"

/* Number of Services supported by TC */
#define TC_IPC_NSERVICES 6

/* List of Service ID's Suopported */
typedef enum {
  TC_CONFIG_SERVICES = 0,
  TC_CANDIDATE_SERVICES,
  TC_STARTUP_DB_SERVICES,
  TC_READ_ACCESS_SERVICES,
  TC_AUTO_SAVE_SERVICES,
  TC_AUDIT_SERVICES
} TcServiceId;

/* List of Operations in different Services */
typedef enum {
  /* TC_CONFIG_SERVICES      */
  TC_OP_CONFIG_ACQUIRE = 0,
  TC_OP_CONFIG_RELEASE,
  TC_OP_CONFIG_ACQUIRE_TIMED,
  TC_OP_CONFIG_ACQUIRE_FORCE,
  /* TC_CANDIDATE_OPERATIONS */
  TC_OP_CANDIDATE_COMMIT,
  TC_OP_CANDIDATE_ABORT,
  /* TC_STARTUP_DB_SERVICES  */
  TC_OP_RUNNING_SAVE,
  TC_OP_CLEAR_STARTUP,
  /* TC_READ_ACCESS_SERVICES */
  TC_OP_READ_ACQUIRE,
  TC_OP_READ_RELEASE,
  /* TC_AUTO_SAVE_SERVICES   */
  TC_OP_AUTOSAVE_GET,
  TC_OP_AUTOSAVE_ENABLE,
  TC_OP_AUTOSAVE_DISABLE,
  /* TC_AUDIT_SERVICES       */
  TC_OP_USER_AUDIT,
  TC_OP_DRIVER_AUDIT,
  /* INVALID_OPERATION      */
  TC_OP_INVALID
} TcServiceType;

/* List of Return Values and Operation Status Values */
typedef enum {
  /* System Failover */
  TC_OPER_FAILURE       = UNC_TC_OPER_FAILURE,
  /* Invalid Input Values Passed*/
  TC_OPER_INVALID_INPUT = UNC_TC_OPER_INVALID_INPUT,
  TC_OPER_SUCCESS       = UNC_RC_SUCCESS,
  TC_CONFIG_NOT_PRESENT = UNC_TC_CONFIG_NOT_PRESENT,
  TC_CONFIG_PRESENT,
  TC_STATE_CHANGED      = UNC_TC_STATE_CHANGED,
  TC_INVALID_CONFIG_ID, 
  TC_INVALID_OPERATION_TYPE,
  TC_INVALID_SESSION_ID,
  TC_INVALID_STATE,
  TC_OPER_ABORT,
  TC_SESSION_ALREADY_ACTIVE,
  TC_SESSION_NOT_ACTIVE,
  TC_SYSTEM_BUSY,
  TC_SYSTEM_FAILURE,
  TC_OPER_FORBIDDEN          = UNC_TC_FORBIDDEN_OPERATION,
  TC_OPER_DRIVER_NOT_PRESENT = UNC_RC_ERR_DRIVER_NOT_PRESENT,
  TC_OPER_SIMPLIFIED_AUDIT
} TcOperStatus;

/* The Indices of data in the Request Message */
typedef enum {
  TC_REQ_OP_TYPE_INDEX = 0,
  TC_REQ_SESSION_ID_INDEX,
  TC_REQ_ARG_INDEX
} TcCommonReqIndex;

/* The Order of Data in Response Mesage */
typedef enum {
  TC_RES_OP_TYPE_INDEX = 0,
  TC_RES_SESSION_ID_INDEX,
  TC_RES_OP_STATUS_INDEX,
  TC_RES_VALUE_INDEX
} TcCommonRespIndex;

/*
 * The Order of Data in Response to
 * TC_CANDIDATE_SERVICES operations
 */
typedef enum {
  TC_CAND_RES_OP_TYPE_INDEX = 0,
  TC_CAND_RES_SESSION_ID_INDEX,
  TC_CAND_RES_CONFIG_ID_INDEX,
  TC_CAND_RES_OP_STATUS_INDEX
} TcCandidateOperRespIndex;


/* AutoSave Values */
typedef enum {
  TC_AUTOSAVE_DISABLED = 0,
  TC_AUTOSAVE_ENABLED
} TcAutoSaveValue;

/* Audit operation Status */
typedef enum {
    TC_AUDIT_OPER_FAILURE = 0,
    TC_AUDIT_OPER_SUCCESS 
} TcAuditStatus;



#endif  // SRC_INCLUDE_UNC_TC_EXTERNAL_TC_SERVICES_H_
