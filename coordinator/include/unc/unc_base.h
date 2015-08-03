/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/**
* @file   unc_base.h
* @brief  Driver common definition.
* @par History
*  - 2013/01/01 Create.
*  - 2013/03/11 Fix for ticket #35.
*/

#ifndef _UNC_BASE_H_
#define _UNC_BASE_H_

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

/**
 * @brief  UNC API response code returned
*/
typedef enum {
  UNC_RC_SUCCESS = 0,

  /*TC response codes*/
  UNC_TC_OPER_FAILURE = -11,           /* System Failover */
  UNC_TC_OPER_INVALID_INPUT = -10,     /* Invalid Input Values Passed */
  UNC_TC_CONFIG_NOT_PRESENT = 100,     /* Config mode is not acquired */
  UNC_TC_CONFIG_PRESENT,               /* Config mode already present */
  UNC_TC_STATE_CHANGED,
  UNC_TC_INVALID_CONFIG_ID,
  UNC_TC_INVALID_OPERATION_TYPE,
  UNC_TC_INVALID_SESSION_ID,
  UNC_TC_INVALID_STATE,
  UNC_TC_OPER_ABORT,                    /* Operation aborted */
  UNC_TC_SESSION_ALREADY_ACTIVE,
  UNC_TC_SESSION_NOT_ACTIVE,
  UNC_TC_SYSTEM_BUSY,                   /* Acquiring lock failed as another
                                           operation is in progress */
  UNC_TC_SYSTEM_FAILURE,                /* Internal errors in any of the modules */
  UNC_TC_FORBIDDEN_OPERATION,          /* Operation forbidden in current setup*/
  UNC_TC_AUDIT_CANCELLED,              /* User Audit cancelled due to CancellableAudit */
  
  /*Common error codes*/
  UNC_RC_ERR_DRIVER_NOT_PRESENT = 200,

  /* UPLL Service */
  UNC_UPLL_RC_ERR_GENERIC = 1000,                   /* Generic error */
  UNC_UPLL_RC_ERR_BAD_REQUEST,               /* The request message format is bad */
  UNC_UPLL_RC_ERR_BAD_CONFIG_OR_SESSION_ID,  /* The given session does not have the
                                            config lock */
  UNC_UPLL_RC_ERR_NO_SUCH_OPERATION,         /* Not a valid operation */
  UNC_UPLL_RC_ERR_INVALID_OPTION1,           /* Not a valid option1 */
  UNC_UPLL_RC_ERR_INVALID_OPTION2,           /* Not a valid option2 */
  UNC_UPLL_RC_ERR_CFG_SYNTAX,                /* Syntax check failed */
  UNC_UPLL_RC_ERR_CFG_SEMANTIC,              /* Semantic check failed */
  UNC_UPLL_RC_ERR_RESOURCE_DISCONNECTED,     /* Resource (DBMS, Physical, Driver) is
                                            diconnected */
  UNC_UPLL_RC_ERR_DB_ACCESS,                 /* DBMS access (read / write /
                                            transacation) failure */
  UNC_UPLL_RC_ERR_NO_SUCH_INSTANCE,          /* Instance specified by key does not
                                            exist */
  UNC_UPLL_RC_ERR_NO_SUCH_NAME,              /* The specified keytype is unknown */
  UNC_UPLL_RC_ERR_NO_SUCH_DATATYPE,          /* The specified datatype is unknown */
  UNC_UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR,    /* The operation not supported by
                                            controller */
  UNC_UPLL_RC_ERR_NOT_SUPPORTED_BY_STANDBY,  /* The operation not supported by
                                            standby UPLL */
  UNC_UPLL_RC_ERR_PARENT_DOES_NOT_EXIST,     /* For creating the given keytype
                                            instance, its parent does not
                                            exist */
  UNC_UPLL_RC_ERR_INSTANCE_EXISTS,           /* The given keytype instance cannot be
                                            created because it already exists */
  UNC_UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT,   /* Not allowed for this datatype */
  UNC_UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_KT,   /* Not allowed for this KT */
  UNC_UPLL_RC_ERR_NOT_ALLOWED_AT_THIS_TIME,  /* Not allowed for at this time */
  UNC_UPLL_RC_ERR_EXCEEDS_RESOURCE_LIMIT,    /* The given operation exceeds the
                                            resource limit */
  UNC_UPLL_RC_ERR_MERGE_CONFLICT,            /* Merge failed as there is a merge
                                            conflict */
  UNC_UPLL_RC_ERR_CANDIDATE_IS_DIRTY,        /* The operation could not be performed
                                            because there are uncommited changes
                                            in the candidate configuration */
  UNC_UPLL_RC_ERR_SHUTTING_DOWN,              /* UPLL daemon is shutting down and
                                            cannot process the request */
  UNC_UPLL_RC_ERR_EXPAND,                    /* Unified-network/boundary
                                                configuration is incomplete */
  UNC_UPLL_RC_ERR_VLAN_IN_USE,               /* Specified logical port and VLAN
                                                ID is already in use */
  UNC_UPLL_RC_ERR_VLAN_TYPE_MISMATCH,        /* Mismatch in VLAN type */
  UNC_UPLL_RC_ERR_NAME_CONFLICT,             /* User configured name conflicts
                                                with internally created name */
  UNC_UPLL_RC_ERR_INVALID_DOMAIN,            /* Specified domain is invalid */
  
  UNC_UPLL_RC_ERR_INVALID_DEST_ADDR,        /* Specified destination address is invalid */
  
  UNC_UPLL_RC_ERR_INVALID_NEXTHOP_ADDR,     /* Specified next-hop address is invalid */
  
  UNC_UPLL_RC_ERR_INVALID_NWMONGRP,         /* Specified network-group name does not exist */
  /*UPPL Service*/
  UNC_UPPL_RC_ERR_BAD_REQUEST = 2000,       /* The request message format is bad */
  UNC_UPPL_RC_FAILURE,                  /* REMOVE THIS */
  UNC_UPPL_RC_ERR_INVALID_CONFIGID,         /* Invalid config-id*/
  UNC_UPPL_RC_ERR_INVALID_SESSIONID,        /* Invalid session-id */
  UNC_UPPL_RC_ERR_VERSION_NOT_SUPPORTED,    /* unsupported version*/
  UNC_UPPL_RC_ERR_KEYTYPE_NOT_SUPPORTED,    /* unsupported keytype*/
  UNC_UPPL_RC_ERR_DATATYPE_NOT_SUPPORTED,   /* Unsupported datatype*/
  UNC_UPPL_RC_ERR_ATTRIBUTE_NOT_SUPPORTED,  /* Unsupported attribute*/
  UNC_UPPL_RC_ERR_OPERATION_NOT_SUPPORTED,  /* Unsupported operation*/
  UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED,    /* Operation is not allowed at this state*/
  UNC_UPPL_RC_ERR_INVALID_OPTION1,          /* Not a valid option1 */
  UNC_UPPL_RC_ERR_INVALID_OPTION2,          /* Not a valid option2 */
  UNC_UPPL_RC_ERR_CFG_SYNTAX,               /* Syntax check failed */
  UNC_UPPL_RC_ERR_CFG_SEMANTIC,             /* Semantic check failed */
  UNC_UPPL_RC_ERR_PARENT_DOES_NOT_EXIST,    /* parent does not exist */
  UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE,         /* required instance does not exist */
  UNC_UPPL_RC_ERR_INSTANCE_EXISTS,          /* instance already exists */
  UNC_UPPL_RC_ERR_EXCEEDS_RESOURCE_LIMIT,   /* Exceeds resource limit */
  UNC_UPPL_RC_ERR_DB_ACCESS,                /* Error while accessing Database */
  UNC_UPPL_RC_ERR_NOT_SUPPORTED_BY_STANDBY, /* operation not allowed in SBY */
  UNC_UPPL_RC_ERR_RESOURCE_DISCONNECTED,    /* resource disconnected*/
  UNC_UPPL_RC_ERR_INVALID_STATE,            /* Invalid state */
  UNC_UPPL_RC_ERR_MERGE_CONFLICT,           /* Merge conflict */
  UNC_UPPL_RC_ERR_FATAL_RESOURCE_ALLOCATION,    /* Fatal resource allocation */
  UNC_UPPL_RC_ERR_DRIVER_COMMUNICATION_FAILURE, /* communication with driver failed*/
  UNC_UPPL_RC_ERR_LOGICAL_COMMUNICATION_FAILURE,/* communication with upll failed */
  UNC_UPPL_RC_ERR_SHUTTING_DOWN,            /* process is shutting down */
  UNC_UPPL_RC_ERR_IPC_WRITE_ERROR,          /* IPC write error */
  UNC_UPPL_RC_ERR_DB_UPDATE,                /* DB update error */
  UNC_UPPL_RC_ERR_DB_GET,                   /* DB Get error */
  UNC_UPPL_RC_ERR_DB_DELETE,                /* DB delete error */
  UNC_UPPL_RC_ERR_DB_CREATE,                /* DB create error */
  UNC_UPPL_RC_ERR_CANDIDATE_IS_DIRTY,       /* candidate is dirty */
  UNC_UPPL_RC_ERR_ATTRIBUTE_NOT_FOUND,      /* could not find the required attribute */
  UNC_UPPL_RC_ERR_NOTIFICATION_NOT_SUPPORTED,   /* Unsupported notification */
  UNC_UPPL_RC_ERR_NOTIFICATION_HANDLING_FAILED, /* Notification handling failed */
  UNC_UPPL_RC_ERR_ALARM_API,                    /* Alarm API failure*/
  UNC_UPPL_RC_ERR_CONF_FILE_READ,               /* Error reading configuration file */
  UNC_UPPL_RC_ERR_CAP_FILE_READ,                /* Error in reading capability file */
  UNC_UPPL_RC_ERR_DB_OPER_STATUS,               /* DB status error */
  UNC_UPPL_RC_ERR_CTRLR_DISCONNECTED,           /* Controller is disconnected for state read */
  UNC_UPPL_RC_ERR_AUDIT_NOT_IN_PROGRESS,        /* audit not in progress */
  UNC_UPPL_RC_ERR_FATAL_COPY_CONFIG,            /* copying running DB to startup DB failed */
  UNC_UPPL_RC_ERR_FATAL_COPYDB_CANDID_RUNNING,  /* DB connection error while copying
                                               candidate DB to running DB */
  UNC_UPPL_RC_ERR_COPY_CANDID_TO_RUNNING,       /* Error in copying candidate to running DB
                                               while startup*/
  UNC_UPPL_RC_ERR_COPY_RUNNING_TO_CANDID,       /* copy running to candidate database failed in
                                                abort candidate */
  UNC_UPPL_RC_ERR_INVALID_CANDID_CONFIG,
  UNC_UPPL_RC_ERR_WRITE_ENTITY_DB,
  UNC_UPPL_RC_ERR_COPY_RUNNING_TO_START,
  UNC_UPPL_RC_ERR_COPY_STARTUP_TO_CANDID,
  UNC_UPPL_RC_ERR_CLEAR_DB,
  UNC_UPPL_RC_ERR_IMPORT_START_INVALID_DRIVER_RESPONSE,
  UNC_UPPL_RC_ERR_IMPORT_FAILURE,
  UNC_UPPL_RC_ERR_IMPORT_MERGE_FAILURE,
  UNC_UPPL_RC_ERR_COMMIT_OPERATION_NOT_ALLOWED,
  UNC_UPPL_RC_ERR_COMMIT_UPDATE_DRIVER_FAILURE,
  UNC_UPPL_RC_ERR_TRANSACTION_START,
  UNC_UPPL_RC_ERR_INVALID_TRANSACT_START_REQ,
  UNC_UPPL_RC_ERR_TRANSACTION_START_INVALID_DRIVER_RESPONSE,
  UNC_UPPL_RC_ERR_VOTE_DB_INVALID,
  UNC_UPPL_RC_ERR_VOTE_INVALID_REQ,
  UNC_UPPL_RC_ERR_AUDIT_FAILURE,
  UNC_UPPL_RC_ERR_IMPORT_IN,
  UNC_UPPL_RC_ERR_ABORT_AUDIT,
  UNC_UPPL_RC_ERR_ABORT_TRANSACTION,
  UNC_UPPL_RC_ERR_MANDATORY_ATTRIB_NULL_VALUE,
  UNC_UPPL_RC_ERR_MANDATORY_ATTRIB_INVALID,

  /*Driver response codes*/
  UNC_DRV_RC_ERR_GENERIC = 3000,       /* internal/generic errors  */
  UNC_DRV_RC_DAEMON_INACTIVE,          /* driver daemon is not up  */
  UNC_DRV_RC_INVALID_REQUEST_FORMAT,   /* invalid request format   */
  UNC_DRV_RC_INVALID_SESSION_ID,       /* invalid session id       */
  UNC_DRV_RC_INVALID_CONFIG_ID,        /* invalid config id        */
  UNC_DRV_RC_INVALID_OPERATION,        /* invalid operation        */
  UNC_DRV_RC_INVALID_OPTION1,          /* invalid option1          */
  UNC_DRV_RC_INVALID_OPTION2,          /* invalid option2          */
  UNC_DRV_RC_INVALID_DATATYPE,         /* invalid data type        */
  UNC_DRV_RC_INVALID_KEYTYPE,          /* invalid key type         */
  UNC_DRV_RC_MISSING_KEY_STRUCT,       /* missing key structure    */
  UNC_DRV_RC_MISSING_VAL_STRUCT,       /* missing value structure  */
  UNC_DRV_RC_ERR_ATTRIBUTE_SYNTAX,     /* attribute syntax error */
  UNC_DRV_RC_ERR_ATTRIBUTE_SEMANTIC,   /* attribute semantic error */
  UNC_DRV_RC_ERR_NOT_SUPPORTED_BY_CTRLR, /* capa check error */

  /*Transaction errors*/
  UNC_RC_INTERNAL_ERR = 4000,      /* Internal error in any of the modules*/
  UNC_RC_CONFIG_INVAL,             /* invalid configuiration   */
  UNC_RC_CTRLAPI_FAILURE,          /* controller api failed    */
  UNC_RC_CTR_CONFIG_STATUS_ERR,    /* controller configuration status is not confirmed */
  UNC_RC_CTR_BUSY,                 /* acquiring config mode failed in controller */
  UNC_RC_CTR_DISCONNECTED,         /* controller disconnected/down  */
  UNC_RC_REQ_NOT_SENT_TO_CTR,      /* request not sent to controller   */
  UNC_RC_NO_SUCH_INSTANCE,         /* request for unknown attribute */
  UNC_RC_UNSUPPORTED_CTRL_CONFIG,  /* unsupported controller configuration */
  UNC_RC_REQUEST_CANCELLED
}UncRespCode;

#ifdef __cplusplus
}  /* End of extern "C" */
#endif  /* __cplusplus */

#endif
