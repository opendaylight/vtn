#ifndef _UPPL_COMMON_H_
#define _UPPL_COMMON_H_

/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/**
 * @brief   UPPL Common Header
 * @file    uppl_common.h
 * @author  HCL
 * @date    Nov-2012
 * @version SYSTEM:UNC 1.0, MODULE:Physical
 *
 */


/*
 * @brief System State
 */

#define UPPL_IPC_CHN_NAME                   "phynwd"
#define UPPL_IPC_SVC_NAME                   "uppl"

typedef enum {
  UPPL_SYSTEM_ST_ACTIVE = 0,
  UPPL_SYSTEM_ST_STANDBY
}UpplSystemState;

/*
 * KT_CONTROLLER specific
 */
typedef enum {
  UPPL_CONTROLLER_OPER_DOWN = 0,
  UPPL_CONTROLLER_OPER_UP,
  UPPL_CONTROLLER_OPER_WAITING_AUDIT,
  UPPL_CONTROLLER_OPER_AUDITING
}UpplControllerOperStatus;

typedef enum {
  UPPL_AUTO_AUDIT_DISABLED = 0,
  UPPL_AUTO_AUDIT_ENABLED
}UpplControllerAuditStatus;

//  Enum for the structure val_ctr
typedef enum {
  kIdxType = 0,
  kIdxVersion,
  kIdxDescription,
  kIdxIpAddress,
  kIdxUser,
  kIdxPassword,
  kIdxEnableAudit
}uppl_val_ctr_index;

//  Enum for the structure val_ctr_st
typedef enum {
  kIdxController = 0,
  kIdxActualVersion,
  kIdxOperStatus
}uppl_val_ctr_st_index;


//  Enum for the structure val_path_fault_alarm
typedef enum {
   kIdxIngressLogicalPort = 0,
   kIdxEgressLogicalPort,
   kIdxIngressNoOfPorts,
   kIdxEgressNoOfPorts
}uppl_val_path_fault_alarm_index;

/*
 * KT_CTR_DOMAIN specific
 */
typedef enum {
  UPPL_DOMAIN_TYPE_DEFAULT = 0,
  UPPL_DOMAIN_TYPE_NORMAL
}UpplDomainType;

typedef enum {
  UPPL_CTR_DOMAIN_OPER_DOWN = 0,
  UPPL_CTR_DOMAIN_OPER_UP,
  UPPL_CTR_DOMAIN_OPER_UNKNOWN
}UpplDomainOperStatus;

//  Enum for the structure val_domain
typedef enum {
  kIdxDomainType = 0,
  kIdxDomainDescription
}uppl_val_domain_index;

//  Enum for the structure val_domain_st
typedef enum {
  kIdxDomainStDomain = 0,
  kIdxDomainStOperStatus
}uppl_val_domain_st_index;

/*
 * KT_LOGICAL_PORT specific
 */
typedef enum {
  UPPL_LP_SWITCH = 0,
  UPPL_LP_PHYSICAL_PORT,
  UPPL_LP_TRUNK_PORT,
  UPPL_LP_SUBDOMAIN,
  UPPL_LP_TUNNEL_ENDPOINT
}UpplLogicalPortType;

typedef enum {
  UPPL_OPER_DOWN_CRITERIA_ANY = 0,
  UPPL_OPER_DOWN_CRITERIA_ALL
}UpplLogicalPortOperDownCriteria;

typedef enum {
  UPPL_LOGICAL_PORT_OPER_DOWN = 0,
  UPPL_LOGICAL_PORT_OPER_UP,
  UPPL_LOGICAL_PORT_OPER_UNKNOWN
}UpplLogicalPortOperStatus;

typedef enum {
  kIdxLogicalPortDescription = 0,
  kIdxLogicalPortType,
  kIdxLogicalPortSwitchId,
  kIdxLogicalPortPhysicalPortId,
  kIdxLogicalPortOperDownCriteria
}uppl_val_logical_port_index;

typedef enum {
  kIdxLogicalPortSt = 0,
  kIdxLogicalPortStOperStatus
}uppl_val_logical_port_st_index;

/*
 * KT_SWITCH specific
 */
typedef enum {
  UPPL_SWITCH_ADMIN_UP = 0,
  UPPL_SWITCH_ADMIN_DOWN
}UpplSwitchAdminStatus;

typedef enum {
  UPPL_SWITCH_OPER_DOWN = 0,
  UPPL_SWITCH_OPER_UP,
  UPPL_SWITCH_OPER_UNKNOWN
}UpplSwitchOperStatus;

typedef enum {
  kIdxSwitchDescription = 0,
  kIdxSwitchModel,
  kIdxSwitchIPAddress,
  kIdxSwitchIPV6Address,
  kIdxSwitchAdminStatus,
  kIdxSwitchDomainName
}uppl_val_switch_index;

typedef enum {
  kIdxSwitch = 0,
  kIdxSwitchOperStatus,
  kIdxSwitchManufacturer,
  kIdxSwitchHardware,
  kIdxSwitchSoftware,
  kIdxSwitchAlarmStatus
}uppl_val_switch_st_index;

/*
 * KT_PORT specific
 */
typedef enum {
  UPPL_PORT_ADMIN_UP = 0,
  UPPL_PORT_ADMIN_DOWN
}UpplPortAdminStatus;

typedef enum {
  UPPL_PORT_OPER_DOWN = 0,
  UPPL_PORT_OPER_UP,
  UPPL_PORT_OPER_UNKNOWN
}UpplPortOperStatus;

typedef enum {
  UPPL_PORT_DIR_INTERNEL = 0,
  UPPL_PORT_DIR_EXTERNAL,
  UPPL_PORT_DIR_UNKNOWN,
}UpplPortDirection;

typedef enum {
  UPPL_PORT_DUPLEX_HALF = 0,
  UPPL_PORT_DUPLEX_FULL
}UpplPortDuplex;

typedef enum {
  kIdxPortNumber = 0,
  kIdxPortDescription,
  kIdxPortAdminStatus,
  kIdxPortTrunkAllowedVlan
}uppl_val_port_index;

typedef enum {
  kIdxPortSt = 0,
  kIdxPortOperStatus,
  kIdxPortMacAddress,
  kIdxPortDirection,
  kIdxPortDuplex,
  kIdxPortSpeed,
  kIdxPortAlarmsStatus,
  kIdxPortLogicalPortId
}uppl_val_port_st_index;

typedef enum {
  kIdxPort = 0,
  kIdxPortConnectedSwitchId,
  kIdxPortConnectedPortId
}uppl_val_port_neighbor_index;

/*
 * KT_LINK specific
 */
typedef enum {
  UPPL_LINK_OPER_DOWN = 0,
  UPPL_LINK_OPER_UP,
  UPPL_LINK_OPER_UNKNOWN
}UpplLinkOperStatus;

typedef enum {
  kIdxLinkDescription = 0
}uppl_val_link_index;

typedef enum {
  kIdxLinkStLink = 0,
  kIdxLinkStOperStatus,
}uppl_val_link_st_index;

/*
 * KT_BOUNDARY specific
 */
typedef enum {
  UPPL_BOUNDARY_OPER_DOWN = 0,
  UPPL_BOUNDARY_OPER_UP,
  UPPL_BOUNDARY_OPER_UNKNOWN
}UpplBoundaryOperStatus;

typedef enum {
  kIdxBoundaryDescription = 0,
  kIdxBoundaryControllerName1,
  kIdxBoundaryDomainName1,
  kIdxBoundaryLogicalPortId1,
  kIdxBoundaryControllerName2,
  kIdxBoundaryDomainName2,
  kIdxBoundaryLogicalPortId2
}uppl_val_boundary_index;

typedef enum {
  kIdxBoundaryStBoundary = 0,
  kIdxBoundaryStOperStatus
}uppl_val_boundary_st_index;

/*
 * @brief DB Return Status and (Error Code)
 */

typedef enum {
  UPPL_RC_SUCCESS = 0,
  UPPL_RC_FAILURE,
  UPPL_RC_ERR_BAD_REQUEST,
  UPPL_RC_ERR_INVALID_CONFIGID,
  UPPL_RC_ERR_INVALID_SESSIONID,
  UPPL_RC_ERR_VERSION_NOT_SUPPORTED,
  UPPL_RC_ERR_KEYTYPE_NOT_SUPPORTED,
  UPPL_RC_ERR_DATATYPE_NOT_SUPPORTED,
  UPPL_RC_ERR_ATTRIBUTE_NOT_SUPPORTED,
  UPPL_RC_ERR_OPERATION_NOT_SUPPORTED,
  UPPL_RC_ERR_OPERATION_NOT_ALLOWED,
  UPPL_RC_ERR_INVALID_OPTION1,
  UPPL_RC_ERR_INVALID_OPTION2,
  UPPL_RC_ERR_CFG_SYNTAX,
  UPPL_RC_ERR_CFG_SEMANTIC,
  UPPL_RC_ERR_PARENT_DOES_NOT_EXIST,
  UPPL_RC_ERR_NO_SUCH_INSTANCE,
  UPPL_RC_ERR_INSTANCE_EXISTS,
  UPPL_RC_ERR_EXCEEDS_RESOURCE_LIMIT,
  UPPL_RC_ERR_DB_ACCESS,
  UPPL_RC_ERR_NOT_SUPPORTED_BY_STANDBY,
  UPPL_RC_ERR_RESOURCE_DISCONNECTED,
  UPPL_RC_ERR_INVALID_STATE,
  UPPL_RC_ERR_MERGE_CONFLICT,
  UPPL_RC_ERR_FATAL_RESOURCE_ALLOCATION,
  UPPL_RC_ERR_DRIVER_COMMUNICATION_FAILURE,
  UPPL_RC_ERR_LOGICAL_COMMUNICATION_FAILURE,
  UPPL_RC_ERR_SHUTTING_DOWN,
  UPPL_RC_ERR_IPC_WRITE_ERROR,
  UPPL_RC_ERR_DB_UPDATE,
  UPPL_RC_ERR_DB_GET,
  UPPL_RC_ERR_DB_DELETE,
  UPPL_RC_ERR_DB_CREATE,
  UPPL_RC_ERR_CANDIDATE_IS_DIRTY,
  UPPL_RC_ERR_ATTRIBUTE_NOT_FOUND,

  UPPL_RC_ERR_NOTIFICATION_NOT_SUPPORTED,
  UPPL_RC_ERR_NOTIFICATION_HANDLING_FAILED,
  UPPL_RC_ERR_ALARM_API,

  UPPL_RC_ERR_CONF_FILE_READ,
  UPPL_RC_ERR_CAP_FILE_READ,

  UPPL_RC_ERR_DB_OPER_STATUS,

  UPPL_RC_ERR_AUDIT_NOT_IN_PROGRESS,
  UPPL_RC_ERR_FATAL_COPY_CONFIG,
  UPPL_RC_ERR_FATAL_COPYDB_CANDID_RUNNING,
  UPPL_RC_ERR_COPY_CANDID_TO_RUNNING,
  UPPL_RC_ERR_COPY_RUNNING_TO_CANDID,
  UPPL_RC_ERR_INVALID_CANDID_CONFIG,
  UPPL_RC_ERR_WRITE_ENTITY_DB,
  UPPL_RC_ERR_COPY_RUNNING_TO_START,
  UPPL_RC_ERR_COPY_STARTUP_TO_CANDID,
  UPPL_RC_ERR_CLEAR_DB,
  UPPL_RC_ERR_IMPORT_START_INVALID_DRIVER_RESPONSE,
  UPPL_RC_ERR_IMPORT_FAILURE,
  UPPL_RC_ERR_IMPORT_MERGE_FAILURE,
  UPPL_RC_ERR_COMMIT_OPERATION_NOT_ALLOWED,
  UPPL_RC_ERR_COMMIT_UPDATE_DRIVER_FAILURE,
  UPPL_RC_ERR_TRANSACTION_START,
  UPPL_RC_ERR_INVALID_TRANSACT_START_REQ,
  UPPL_RC_ERR_TRANSACTION_START_INVALID_DRIVER_RESPONSE,
  UPPL_RC_ERR_VOTE_DB_INVALID,
  UPPL_RC_ERR_VOTE_INVALID_REQ,
  UPPL_RC_ERR_AUDIT_FAILURE,
  UPPL_RC_ERR_IMPORT_IN,
  UPPL_RC_ERR_ABORT_AUDIT,
  UPPL_RC_ERR_ABORT_TRANSACTION,
  UPPL_RC_ERR_MANDATORY_ATTRIB_NULL_VALUE,
  UPPL_RC_ERR_MANDATORY_ATTRIB_INVALID,
}UpplReturnCode;

typedef enum {
  UPPL_SVC_CONFIGREQ = 0,
  UPPL_SVC_READREQ,
  UPPL_SVC_GLOBAL_CONFIG
}uppl_service_id;

typedef enum {
  UPPL_EVENTS_KT_CONTROLLER = 0,
  UPPL_EVENTS_KT_CTR_DOMAIN,
  UPPL_EVENTS_KT_LOGICAL_PORT,
  UPPL_EVENTS_KT_LOGICAL_MEMBER_PORT,
  UPPL_EVENTS_KT_SWITCH,
  UPPL_EVENTS_KT_PORT,
  UPPL_EVENTS_KT_LINK,
  UPPL_EVENTS_KT_BOUNDARY,
  UPPL_ALARMS_PHYS_PATH_FAULT
} uppl_event_type_t;

/*Alarm type enumeration body*/
typedef enum {
  UPPL_ALARMS_PATH_FAULT = 0,   /*Physical Path Fault/Recovery*/
  UPPL_ALARMS_OFS_BC_CORE_DOMAIN_SPLIT, /* Broadcast Core Domain Split/Recovery*/
  UPPL_ALARMS_OFS_BC_SUB_DOMAIN_SPLIT, /* Broadcast Sub Domain Split/Recovery*/
  UPPL_ALARMS_FLOW_ENT_FULL,    /*FlowEntry Full Occurred/Recovery*/
  UPPL_ALARMS_DEFAULT_FLOW,     /*Default Flow Failure/Success*/
  UPPL_ALARMS_PORT_DIRECTION,   /*Port direction is inconsistent/consistent*/
  UPPL_ALARMS_PORT_CONGES,      /*OFS Port Congestion Occurred/Recovered*/
  UPPL_ALARMS_OFS_LACK_FEATURES,/*OFS is lack of features/OFS has recovered lack of features*/
} uppl_alarm_type_t;

/*
 * @brief Row Status
 */
typedef enum {
  CREATED = 0,
  UPDATED,
  DELETED,
  ROW_VALID,
  ROW_INVALID,
  APPLIED,
  NOTAPPLIED,
  PARTIALLY_APPLIED
}CsRowStatus;

/*
 *  @brief Valid
 */

typedef enum {
  INVALID = 0,
  VALID,
  VALID_WITH_NO_VALUE,
  NOT_SUPPORTED,
  NOT_SET
}Valid;

typedef enum {
  UNC_OP_IS_CANDIDATE_DIRTY = 0,
  UNC_OP_IMPORT_CONTROLLER_CONFIG,
  UNC_OP_MERGE_CONTROLLER_CONFIG,
  UNC_OP_CLEAR_IMPORT_CONFIG
} unc_addl_operation_t;


#endif  //  _UPPL_COMMON_H_

