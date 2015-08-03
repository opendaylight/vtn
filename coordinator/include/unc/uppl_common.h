/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef UNC_UPPL_COMMON_H_
#define UNC_UPPL_COMMON_H_

/**
 * @brief   UPPL Common Header
 * @file    uppl_common.h
 * @author  HCL
 * @date    Dec-2014
 * @version SYSTEM:UNC v6.2.0.0, MODULE:Physical
 *
 */

/*
 * @brief System State
 */

#define UPPL_IPC_CHN_NAME                   "phynwd"
#define UPPL_IPC_SVC_NAME                   "uppl"

#define UPPL_LP_STANDARD_SD_DESC "PF Standard Subdomain"
#define UPPL_LP_EXTENSION_SD_DESC "PF Extension Subdomain"
#define UPPL_LP_MAC_FORWARDING_DESC "PF MAC Forwarding Subdomain"

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
  UPPL_CONTROLLER_OPER_AUDITING,
  UPPL_CONTROLLER_OPER_EVENTS_START,
  UPPL_CONTROLLER_OPER_EVENTS_MERGED
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
  kIdxEnableAudit,
  kIdxcPort
}uppl_val_ctr_index;

//  Enum for the structure val_ctr_st
typedef enum {
  kIdxController = 0,
  kIdxActualVersion,
  kIdxOperStatus,
  kIdxActualId
}uppl_val_ctr_st_index;

typedef enum {
  kIdxCtrVal = 0,
  kIdxCtrCommitNumber,
  kIdxCtrCommitDate,
  kIdxCtrCommitApplication
}uppl_val_ctr_commit_version_index;

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
  UPPL_DOMAIN_TYPE_NORMAL,
  UPPL_DOMAIN_TYPE_PF_LEAF,
  UPPL_DOMAIN_TYPE_PF_SPINE
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

//  Enum for the structure val_domain_fdbentry
typedef enum {
  kIdxDomainFdbStDomain = 0,
  kIdxDomainFdbMaxCount,
  kIdxDomainFdbMaxSwitchId,
  kIdxDomainFdbMinCount,
  kIdxDomainFdbMinSwitchId,
  kIdxDomainFdbAverageCount,
  kIdxDomainFdbNoOfSwitches
}uppl_val_domain_fdbentry_index;

//  Enum for the structure val_domain_fdbentry_vid
typedef enum {
  kIdxDomainFdbVlanId = 0,
  kIdxDomainFdbVidMaxCount,
  kIdxDomainFdbVidMaxSwitchId,
  kIdxDomainFdbVidMinCount,
  kIdxDomainFdbVidMinSwitchId,
  kIdxDomainFdbVidAverageCount
}uppl_val_domain_fdbentry_vid_index;

/*
 * KT_LOGICAL_PORT specific
 */
typedef enum {
  UPPL_LP_SWITCH = 1,
  UPPL_LP_PHYSICAL_PORT = 2,
  UPPL_LP_TRUNK_PORT = 11,
  UPPL_LP_SUBDOMAIN = 12,
  UPPL_LP_TUNNEL_ENDPOINT = 13,
  UPPL_LP_PORT_GROUP = 14,
  UPPL_LP_MAPPING_GROUP = 15
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
  UPPL_LP_BDRY_CANDIDATE_NO = 0,
  UPPL_LP_BDRY_CANDIDATE_YES
}UpplLogicalPortBoundaryCandidate;

typedef enum {
  kIdxLogicalPortBSt=0,
  kIdxLogicalPortBoundaryCandidate,
  kIdxLogicalPortBoundaryConnectedController,
  kIdxLogicalPortBoundaryConnectedDomain,
}uppl_val_logical_port_boundary_st_index;

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
 * KT_LOGICAL_MEMBER_PORT specific
 */
typedef enum {
  kIdxLmPort = 0,
  kIdxLmPortConnectedSwitchId,
  kIdxLmPortConnectedPortId,
  kIdxLmPortConnectedControllerId
}uppl_val_lm_port_neighbor_index;

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

typedef enum {
  kIdxSwitchSt = 0,
  kIdxSwitchStatFlowCount
}uppl_val_switch_st_detail_index;

typedef enum {
  kIdxSwitchFdbSt = 0,
  kIdxSwitchFdbCount
}uppl_val_switch_st_fdbentry_index;

typedef enum {
  kIdxSwitchFdbVlanId = 0,
  kIdxSwitchFdbVidCount
}uppl_val_switch_st_fdbentry_vid_index;

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
  kIdxPortStatSt = 0,
  kIdxPortStatRxPackets,
  kIdxPortStatTxPackets,
  kIdxPortStatRxBytes,
  kIdxPortStatTxBytes,
  kIdxPortStatRxDropped,
  kIdxPortStatTxDropped,
  kIdxPortStatRxErrors,
  kIdxPortStatTxErrors,
  kIdxPortStatRxFrameErr,
  kIdxPortStatRxOverErr,
  kIdxPortStatRxCrcErr,
  kIdxPortStatCollisions
}uppl_val_port_stats_index;

typedef enum {
  kIdxPort = 0,
  kIdxPortConnectedSwitchId,
  kIdxPortConnectedPortId,
  kIdxPortConnectedControllerId
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
  UPPL_ALARMS_OFS_BC_CORE_DOMAIN_SPLIT, /* BC Core Domain Split/Recovery*/
  UPPL_ALARMS_OFS_BC_SUB_DOMAIN_SPLIT, /* BC Sub Domain Split/Recovery*/
  UPPL_ALARMS_FLOW_ENT_FULL,    /*FlowEntry Full Occurred/Recovery*/
  UPPL_ALARMS_DEFAULT_FLOW,     /*Default Flow Failure/Success*/
  UPPL_ALARMS_PORT_DIRECTION,   /*Port direction is inconsistent/consistent*/
  UPPL_ALARMS_PORT_CONGES,      /*OFS Port Congestion Occurred/Recovered*/
  UPPL_ALARMS_OFS_LACK_FEATURES,/*OFS has lack of features/recovered it*/
  UPPL_ALARMS_OFS_DISABLED,/*OFS is disabled/enabled*/
  UPPL_ALARMS_OFS_BC_DOMAIN_SPLIT /* BC Leaf/Spine Domain Split/Recovery*/
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

/*
 * @KT_DATAFLOW_V2
 */
typedef enum {
  kIdxflowId = 0
}uppl_val_data_flow_v2_index;
#endif  // UNC_UPPL_COMMON_H_
