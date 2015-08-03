/*
 * Copyright (c) 2013-2015 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef _UNC_DATAFLOW_H_
#define _UNC_DATAFLOW_H_

/**
 * @brief   UNC Dataflow Header
 * @file    unc_dataflow.h
 * @author  HCL
 * @date    Sep-2013
 * @version SYSTEM:UNC 1.0
 *
 */

typedef enum {
  kidxDfDataFlowReason = 0,
  kidxDfDataFlowControllerCount
} unc_val_df_dataflow_index;

typedef enum {
  kidxDfDataFlowStPackets = 0,
  kidxDfDataFlowStOctets,
  kidxDfDataFlowStDuration
} unc_val_df_dataflow_st_index;

typedef enum {
  kidxDfDataFlowPathInfoSwitchId = 0,
  kidxDfDataFlowPathInfoInPort,
  kidxDfDataFlowPathInfoOutPort
} unc_val_df_dataflow_path_info_index;

typedef enum {
  kidxDfDataFlowControllerName = 0,
  kidxDfDataFlowControllerType,
  kidxDfDataFlowFlowId,
  kidxDfDataFlowStatus,
  kidxDfDataFlowFlowType,
  kidxDfDataFlowPolicyIndex,
  kidxDfDataFlowVtnId,
  kidxDfDataFlowIngressSwitchId,
  kidxDfDataFlowInPort,
  kidxDfDataFlowInStationId,
  kidxDfDataFlowInDomain,
  kidxDfDataFlowEgressSwitchId,
  kidxDfDataFlowOutPort,
  kidxDfDataFlowOutStationId,
  kidxDfDataFlowOutDomain,
  kidxDfDataFlowPathInfoCount,
  kidxDfDataFlowMatchCount,
  kidxDfDataFlowActionCount
} unc_val_df_dataflow_cmn_index;

typedef enum {
  UNC_DF_RES_SUCCESS = 0,
  /* If traversed PFC version is 5.0 */
  UNC_DF_RES_OPERATION_NOT_SUPPORTED,
  /* If flow count exceeds across a boundary by certain limit */
  UNC_DF_RES_EXCEEDS_FLOW_LIMIT,
  /* If traversed PFC is disconnected */
  UNC_DF_RES_CTRLR_DISCONNECTED,
  /* If flow traversal exceeds by certain hop limit */
  UNC_DF_RES_EXCEEDS_HOP_LIMIT,
  /* If target controller is not able to find for a VNP/BYPASS */
  UNC_DF_RES_DST_NOT_REACHED,
  /* If flow not found in target PFC */
  UNC_DF_RES_FLOW_NOT_FOUND,
  /* If any system level error at UNC */
  UNC_DF_RES_SYSTEM_ERROR,
  /* If domain type is leaf or spine */
  UNC_DF_RES_DOMAIN_NOT_SUPPORTED
}UncDataflowReason;

typedef enum {
  UNC_DF_STAT_INIT = 0,
  UNC_DF_STAT_ACTIVATING,
  UNC_DF_STAT_ACTIVE,
  UNC_DF_STAT_SWITCHING
}UncDataflowStatus;

typedef enum {
  UNC_DF_TYPE_VTN = 0
}UncDataflowFlowType;

typedef enum {
  UNC_MATCH_IN_PORT = 0,
  UNC_MATCH_DL_SRC,
  UNC_MATCH_DL_DST,
  UNC_MATCH_DL_TYPE,
  UNC_MATCH_VLAN_ID,
  UNC_MATCH_VLAN_PCP,
  UNC_MATCH_IP_TOS,
  UNC_MATCH_IP_PROTO,
  UNC_MATCH_IPV4_SRC,
  UNC_MATCH_IPV4_DST,
  UNC_MATCH_IPV6_SRC,
  UNC_MATCH_IPV6_DST,
  UNC_MATCH_TP_SRC,
  UNC_MATCH_TP_DST
}UncDataflowFlowMatchType;

typedef enum {
  UNC_MATCH_MASK_INVALID = 0,
  UNC_MATCH_MASK_VALID
}UncDataflowFlowMatchMask;

typedef enum {
  UNC_ACTION_OUTPUT = 0,
  UNC_ACTION_SET_ENQUEUE,
  UNC_ACTION_SET_DL_SRC,
  UNC_ACTION_SET_DL_DST,
  UNC_ACTION_SET_VLAN_ID,
  UNC_ACTION_SET_VLAN_PCP,
  UNC_ACTION_STRIP_VLAN,
  UNC_ACTION_SET_IPV4_SRC,
  UNC_ACTION_SET_IPV4_DST,
  UNC_ACTION_SET_IP_TOS,
  UNC_ACTION_SET_TP_SRC,
  UNC_ACTION_SET_TP_DST,
  UNC_ACTION_SET_IPV6_SRC,
  UNC_ACTION_SET_IPV6_DST
}UncDataflowFlowActionType;

#endif  //  _UNC_DATAFLOW_H_

