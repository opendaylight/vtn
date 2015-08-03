/*
 * Copyright (c) 2012-2015 NEC Corporation
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
#define TC_IPC_NSERVICES 7

/* List of Service ID's Suopported */
typedef enum {
  TC_CONFIG_SERVICES = 0,
  TC_CANDIDATE_SERVICES,
  TC_STARTUP_DB_SERVICES,
  TC_READ_ACCESS_SERVICES,
  TC_READ_STATUS_SERVICES,
  TC_AUTO_SAVE_SERVICES,
  TC_AUDIT_SERVICES
} TcServiceId;

/* List of Operations in different Services */
/* ****************************************************
 *
 * NOTE: When adding new svc enum here, add handling in
 *       TcOpertions::OperTypeToStr() too
 *
 ******************************************************/
typedef enum {
  /* TC_CONFIG_SERVICES      */
  TC_OP_CONFIG_ACQUIRE = 0,
  TC_OP_CONFIG_RELEASE,
  TC_OP_CONFIG_ACQUIRE_TIMED,
  TC_OP_CONFIG_ACQUIRE_PARTIAL,
  TC_OP_CONFIG_ACQUIRE_FORCE,
  /* TC_CANDIDATE_OPERATIONS */
  TC_OP_CANDIDATE_COMMIT,
  TC_OP_CANDIDATE_COMMIT_TIMED,
  TC_OP_CANDIDATE_ABORT,
  TC_OP_CANDIDATE_ABORT_TIMED,
  /* TC_STARTUP_DB_SERVICES  */
  TC_OP_RUNNING_SAVE,
  TC_OP_CLEAR_STARTUP,
  /* TC_READ_ACCESS_SERVICES */
  TC_OP_READ_ACQUIRE,
  TC_OP_READ_RELEASE,
  /* TC_READ_STATUS_SERVICES */
  TC_OP_READ_RUNNING_STATUS,
  TC_OP_READ_STARTUP_STATUS,
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
  TC_OPER_FAILURE            = UNC_TC_OPER_FAILURE,            //-11
  /* Invalid Input Values Passed*/
  TC_OPER_INVALID_INPUT      = UNC_TC_OPER_INVALID_INPUT,      //-10
  TC_OPER_SUCCESS            = UNC_RC_SUCCESS,                 // 0
  TC_CONFIG_NOT_PRESENT      = UNC_TC_CONFIG_NOT_PRESENT,      //100
  TC_CONFIG_PRESENT          = UNC_TC_CONFIG_PRESENT,          //101
  TC_STATE_CHANGED           = UNC_TC_STATE_CHANGED,           //102
  TC_INVALID_CONFIG_ID       = UNC_TC_INVALID_CONFIG_ID,       //103
  TC_INVALID_OPERATION_TYPE  = UNC_TC_INVALID_OPERATION_TYPE,  //104  
  TC_INVALID_SESSION_ID      = UNC_TC_INVALID_SESSION_ID,      //105
  TC_INVALID_STATE           = UNC_TC_INVALID_STATE,           //106
  TC_OPER_ABORT              = UNC_TC_OPER_ABORT,              //107
  TC_SESSION_ALREADY_ACTIVE  = UNC_TC_SESSION_ALREADY_ACTIVE,  //108
  TC_SESSION_NOT_ACTIVE      = UNC_TC_SESSION_NOT_ACTIVE,      //109
  TC_SYSTEM_BUSY             = UNC_TC_SYSTEM_BUSY,             //110
  TC_SYSTEM_FAILURE          = UNC_TC_SYSTEM_FAILURE,          //111
  TC_OPER_FORBIDDEN          = UNC_TC_FORBIDDEN_OPERATION,     //112
  TC_OPER_AUDIT_CANCELLED    = UNC_TC_AUDIT_CANCELLED,         //113
  TC_OPER_SIMPLIFIED_AUDIT,                                    //114
  TC_OPER_DRIVER_NOT_PRESENT = UNC_RC_ERR_DRIVER_NOT_PRESENT,  //200
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

/* Audit Type */
typedef enum  {
  TC_AUDIT_INVALID = 0,
  TC_AUDIT_NORMAL,
  TC_AUDIT_SIMPLIFIED,
  TC_AUDIT_REALNETWORK
} TcAuditType;

/* Partial Config Mode */
typedef enum {
  TC_CONFIG_INVALID = 0,
  TC_CONFIG_GLOBAL,
  TC_CONFIG_REAL,
  TC_CONFIG_VIRTUAL,
  TC_CONFIG_VTN
} TcConfigMode;

/* Read DB Status */
typedef enum  {
  TC_DB_STATUS_CONFIRMED = 0,
  TC_DB_STATUS_UPDATING
}TcReadStatusType;

#endif  // SRC_INCLUDE_UNC_TC_EXTERNAL_TC_SERVICES_H_
