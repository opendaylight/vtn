/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef IPC_UPLL_IPC_ENUM_H_
#define IPC_UPLL_IPC_ENUM_H_

#include "unc/base.h"

UNC_C_BEGIN_DECL

#define ANY_VLAN_ID 0xFFFE
#define NO_VLAN_ID 0xFFFF

/* specify admin status */
enum val_admin_status {
  UPLL_ADMIN_ENABLE = 1,
  UPLL_ADMIN_DISABLE
};

/* specify alarm status */
enum val_alarm_status {
  UPLL_ALARM_CLEAR = 0,
  UPLL_ALARM_RAISE
};

/* specify vlan tagged */
enum vlan_tagged {
  UPLL_VLAN_UNTAGGED = 0,
  UPLL_VLAN_TAGGED
};


/* controller operation status */
enum val_oper_status {
  UPLL_OPER_STATUS_UP = 1,
  UPLL_OPER_STATUS_DOWN,
  UPLL_OPER_STATUS_UNKNOWN,
  UPLL_OPER_STATUS_UNINIT
};

/* Enumeration for do not fragment flag. */
enum val_df_bit {
  UPLL_DF_BIT_DISABLE = 0,
  UPLL_DF_BIT_ENABLE
};

/* Enumeration for entry type for MAC addresses / port mapping */
enum val_mac_entry_type {
  UPLL_MAC_ENTRY_STATIC = 0,
  UPLL_MAC_ENTRY_DYNAMIC
};

/* Enumeration for vnode interface type */
enum val_vnode_if_map_type {
  UPLL_IF_OFS_MAP = 0,
  UPLL_IF_VLAN_MAP
};

/* Enumeration for vtn map status */
enum val_vtn_map_status {
  UPLL_VTN_MAP_VALID = 0,
  UPLL_VTN_MAP_INVALID
};

/* index enumeration for val_ping structure */
enum val_ping_index {
  UPLL_IDX_TARGET_ADDR_PING = 0,
  UPLL_IDX_SRC_ADDR_PING,
  UPLL_IDX_DF_BIT_PING,
  UPLL_IDX_PACKET_SIZE_PING,
  UPLL_IDX_COUNT_PING,
  UPLL_IDX_INTERVAL_PING,
  UPLL_IDX_TIMEOUT_PING
};

/* index enumeration for val_vtn_neighbor  structure */
enum val_vtn_neighbor_index {
  UPLL_IDX_CONN_VNODE_NAME_VN = 0,
  UPLL_IDX_CONN_VNODE_IF_NAME_VN,
  UPLL_IDX_CONN_VLINK_NAME_VN
};

/* index enumeration for val_vtn_station_controller_st  structure */
enum val_vtnstation_controller_st_index {
  UPLL_IDX_STATION_ID_VSCS = 0,
  UPLL_IDX_CREATED_TIME_VSCS,
  UPLL_IDX_MAC_ADDR_VSCS,
  UPLL_IDX_IPV4_COUNT_VSCS,
  UPLL_IDX_IPV6_COUNT_VSCS,
  UPLL_IDX_DATAPATH_ID_VSCS,
  UPLL_IDX_PORT_NAME_VSCS,
  UPLL_IDX_VLAN_ID_VSCS,
  UPLL_IDX_MAP_TYPE_VSCS,
  UPLL_IDX_MAP_STATUS_VSCS,
  UPLL_IDX_VTN_NAME_VSCS,
  UPLL_IDX_DOMAIN_ID_VSCS,
  UPLL_IDX_VNODE_TYPE_VSCS,
  UPLL_IDX_VNODE_NAME_VSCS,
  UPLL_IDX_VNODE_IF_NAME_VSCS,
  UPLL_IDX_VNODE_IF_STATUS_VSCS
};

/* index enumeration for val_vtn_station_controller_stat  structure */
enum val_vtnstation_controller_stat_index {
  UPLL_IDX_ALL_TX_PKT_VSCS = 0,
  UPLL_IDX_ALL_RX_PKT_VSCS,
  UPLL_IDX_ALL_TX_BYTS_VSCS,
  UPLL_IDX_ALL_RX_BYTS_VSCS,
  UPLL_IDX_ALL_NW_TX_PKT_VSCS,
  UPLL_IDX_ALL_NW_RX_PKT_VSCS,
  UPLL_IDX_ALL_NW_TX_BYTS_VSCS,
  UPLL_IDX_ALL_NW_RX_BYTS_VSCS,
  UPLL_IDX_EXST_TX_PKT_VSCS,
  UPLL_IDX_EXST_RX_PKT_VSCS,
  UPLL_IDX_EXST_TX_BYTS_VSCS,
  UPLL_IDX_EXST_RX_BYTS_VSCS,
  UPLL_IDX_EXPD_TX_PKT_VSCS,
  UPLL_IDX_EXPD_RX_PKT_VSCS,
  UPLL_IDX_EXPD_TX_BYTS_VSCS,
  UPLL_IDX_EXPD_RX_BYTS_VSCS,
  UPLL_IDX_ALL_DRP_RX_PKT_VSCS,
  UPLL_IDX_ALL_DRP_RX_BYTS_VSCS,
  UPLL_IDX_EXST_DRP_RX_PKT_VSCS,
  UPLL_IDX_EXST_DRP_RX_BYTS_VSCS,
  UPLL_IDX_EXPD_DRP_RX_PKT_VSCS,
  UPLL_IDX_EXPD_DRP_RX_BYTS_VSCS
};

/* index enumeration for val_vtn_mapping_controller_st  structure */
enum val_vtn_mapping_controller_st_index {
  UPLL_IDX_SWITCH_ID_VMCS = 0,
  UPLL_IDX_PORT_NAME_VMCS,
  UPLL_IDX_LOGICAL_PORT_ID_VMCS,
  UPLL_IDX_VLAN_ID_VMCS,
  UPLL_IDX_TAGGED_VMCS,
  UPLL_IDX_MAP_TYPE_VMCS,
  UPLL_IDX_VNODE_TYPE_VMCS,
  UPLL_IDX_VNODE_NAME_VMCS,
  UPLL_IDX_VNODE_IF_NAME_VMCS
};

/* index enumeration for val_vtn structure */
enum val_vtn_index {
  UPLL_IDX_DESC_VTN =0
};

/* index enumeration for val_rename_vtn structure */
enum val_rename_vtn_index {
  UPLL_IDX_NEW_NAME_RVTN = 0,
  UPLL_IDX_RENAME_TYPE_RVTN
};

/* index enumeration for val_vtn_st structure */
enum val_vtn_st_index {
  UPLL_IDX_OPER_STATUS_VS = 0,
  UPLL_IDX_ALARM_STATUS_VS,
  UPLL_IDX_CREATION_TIME_VS,
  UPLL_IDX_LAST_UPDATE_TIME_VS
};

/* index enumeration for val_vbr structure */
enum val_vbr_index {
  UPLL_IDX_CONTROLLER_ID_VBR = 0,
  UPLL_IDX_DOMAIN_ID_VBR,
  UPLL_IDX_DESC_VBR,
  UPLL_IDX_HOST_ADDR_VBR,
  UPLL_IDX_HOST_ADDR_PREFIXLEN_VBR,
  UPLL_IDX_LABEL_VBR
};

/* index enumeration for val_rename_vbr structure */
enum val_rename_vbr_index {
  UPLL_IDX_NEW_NAME_RVBR = 0,
  UPLL_IDX_RENAME_TYPE_RVBR
};

/* index enumeration for val_vbr_st structure */
enum val_vbr_st_index {
  UPLL_IDX_OPER_STATUS_VBRS = 0,
};

/* index enumeration for val_vbr_if_st structure */
enum val_vbr_if_st_index {
  UPLL_IDX_OPER_STATUS_VBRIS = 0,
};

/* index enumeration for val_vbr_l2_domain_st structure */
enum val_vbr_l2_domain_st_index {
  UPLL_IDX_L2_DOMAIN_ID_VL2DS  = 0,
  UPLL_IDX_OFS_COUNT_VL2DS
};

/* index enumeration for val_vbr_l2_domain_member_st structure */
enum val_vbr_l2_domain_member_st_index {
  UPLL_IDX_SWITCH_ID_VL2DMS = 0,
  UPLL_IDX_VLAN_ID_VL2DMS
};

/* index enumeration for val_vbr_mac_entry_st structure */
enum val_vbr_mac_entry_st_index {
  UPLL_IDX_MAC_ADDR_VMES = 0,
  UPLL_IDX_TYPE_VMES,
  UPLL_IDX_IF_NAME_VMES,
  UPLL_IDX_IF_KIND_VMES
};

/* index enumeration for val_vlan_map structure */
enum val_vlan_map_index {
  UPLL_IDX_VLAN_ID_VM = 0
};

/* index enumeration for val_port_map structure */
enum val_port_map_index {
  UPLL_IDX_LOGICAL_PORT_ID_PM  = 0,
  UPLL_IDX_VLAN_ID_PM,
  UPLL_IDX_TAGGED_PM
};

/* index enumeration for val_vbr_if structure */
enum val_vbr_if_index {
  UPLL_IDX_ADMIN_STATUS_VBRI = 0,
  UPLL_IDX_DESC_VBRI,
  UPLL_IDX_PM_VBRI
};

/* index enumeration for val_vrt structure */
enum val_vrt_index {
  UPLL_IDX_CONTROLLER_ID_VRT = 0,
  UPLL_IDX_DOMAIN_ID_VRT,
  UPLL_IDX_DESC_VRT,
  UPLL_IDX_DHCP_RELAY_ADMIN_STATUS_VRT
};

/* index enumeration for val_rename_vrt structure */
enum val_rename_vrt_index {
  UPLL_IDX_NEW_NAME_RVRT = 0,
  UPLL_IDX_RENAME_TYPE_RVRT
};

/* index enumeration for val_vrt_st structure */
enum val_vrt_st_index {
  UPLL_IDX_OPER_STATUS_VRTS = 0
};

/* index enumeration for val_vrt_if_st structure */
enum val_vrt_if_st_index {
  UPLL_IDX_OPER_STATUS_VRTIS = 0
};

/* index enumeration for val_vrt_dhcp_relay_st structure */
enum val_vrt_dhcp_relay_st_index {
  UPLL_IDX_DHCP_RELAY_ADMIN_STATUS_VDRS = 0,
  UPLL_IDX_IP_COUNT_VDRS,
  UPLL_IDX_IF_COUNT_VDRS
};

/* index enumeration for val_dhcp_relay_if_st structure */
enum val_dhcp_relay_if_st_index {
  UPLL_IDX_IF_NAME_DRIS = 0,
  UPLL_IDX_DHCP_RELAY_STATUS_DRIS
};

/* dhcp relay enabled status */
enum val_dhcp_relay_if_status {
  UPLL_DR_IF_INACTIVE = 0,
  UPLL_DR_IF_ACTIVE,
  UPLL_DR_IF_STARTING,
  UPLL_DR_IF_WAITING,
  UPLL_DR_IF_ERROR
};

/* index enumeration for val_vrt_arp_entry_st structure */
enum val_vrt_arp_entry_st_index {
  UPLL_IDX_MAC_ADDR_VAES = 0,
  UPLL_IDX_IP_ADDR_VAES,
  UPLL_IDX_TYPE_VAES,
  UPLL_IDX_IF_NAME_VAES
};

/* index enumeration for val_vrt_ip_route_st structure */
enum val_vrt_ip_route_st_index {
  UPLL_IDX_DESTINATION_VIRS = 0,
  UPLL_IDX_GATEWAY_VIRS,
  UPLL_IDX_PREFIXLEN_VIRS,
  UPLL_IDX_FLAGS_VIRS,
  UPLL_IDX_METRIC_VIRS,
  UPLL_IDX_USE_VIRS,
  UPLL_IDX_IF_NAME_VIRS,
  UPLL_IDX_NW_MONITOR_GR_VIRS,
  UPLL_IDX_GR_METRIC_VIRS
};

/* index enumeration for val_static_ip_route structure */
enum val_static_ip_route_index {
  UPLL_IDX_NWM_NAME_SIR = 0,
  UPLL_IDX_GROUP_METRIC_SIR
};

/* index enumeration for val_vrt_if structure */
enum val_vrt_if_index {
  UPLL_IDX_DESC_VI = 0,
  UPLL_IDX_IP_ADDR_VI,
  UPLL_IDX_PREFIXLEN_VI,
  UPLL_IDX_MAC_ADDR_VI,
  UPLL_IDX_ADMIN_ST_VI,
};

/* index enumeration for val_vunknown structure */
enum val_vunknown_index {
  UPLL_IDX_DESC_VUN = 0,
  UPLL_IDX_TYPE_VUN,
  UPLL_IDX_CONTROLLER_ID_VUN,
  UPLL_IDX_DOMAIN_ID_VUN
};

enum val_vunknown_type  {
  VUNKNOWN_TYPE_BRIDGE = 0,
  VUNKNOWN_TYPE_ROUTER
};

/* index enumeration for val_vunk_if structure */
enum val_vunk_if_index {
  UPLL_IDX_DESC_VUNI = 0,
  UPLL_IDX_ADMIN_ST_VUNI
};

/* index enumeration for val_vtep structure */
enum val_vtep_index {
  UPLL_IDX_DESC_VTEP = 0,
  UPLL_IDX_CONTROLLER_ID_VTEP,
  UPLL_IDX_DOMAIN_ID_VTEP
};

/* index enumeration for val_vtep_st structure */
enum val_vtep_st_index {
  UPLL_IDX_OPER_STATUS_VTEPS = 0,
};

/* index enumeration for val_vtep_if structure */
enum val_vtep_if_index {
  UPLL_IDX_DESC_VTEPI = 0,
  UPLL_IDX_ADMIN_ST_VTEPI,
  UPLL_IDX_PORT_MAP_VTEPI
};

/* index enumeration for val_vtep_if_st structure */
enum val_vtep_if_st_index {
  UPLL_IDX_IF_OPER_STATUS_VTEPIS = 0
};

/* index enumeration for val_vtep_grp structure */
enum val_vtep_grp_index {
  UPLL_IDX_CONTROLLER_ID_VTEPG = 0,
  UPLL_IDX_DESCRIPTION_VTEPG
};

/* index enumeration for val_vtunnel structure */
enum val_vtunnel_index {
  UPLL_IDX_DESC_VTNL = 0,
  UPLL_IDX_CONTROLLER_ID_VTNL,
  UPLL_IDX_DOMAIN_ID_VTNL,
  UPLL_IDX_VTN_NAME_VTNL,
  UPLL_IDX_VTEP_GRP_NAME_VTNL,
  UPLL_IDX_LABEL_VTNL,
};

/* index enumeration for val_vtunnel_st structure */
enum val_vtunnel_st_index {
  UPLL_IDX_OPER_STATUS_VTNLS = 0
};

/* index enumeration for val_vtunnel_if structure */
enum val_vtunnel_if_index {
  UPLL_IDX_DESC_VTNL_IF = 0,
  UPLL_IDX_ADMIN_ST_VTNL_IF,
  UPLL_IDX_PORT_MAP_VTNL_IF
};

/* index enumeration for val_vtunnel_if_st structure */
enum val_vtunnel_if_st_index {
  UPLL_IDX_IF_OPER_STATUS_VTNLI = 0
};

/* index enumeration for val_vlink structure */
enum val_vlink_index {
  UPLL_IDX_ADMIN_STATUS_VLNK = 0 ,
  UPLL_IDX_VNODE1_NAME_VLNK,
  UPLL_IDX_VNODE1_IF_NAME_VLNK,
  UPLL_IDX_VNODE2_NAME_VLNK,
  UPLL_IDX_VNODE2_IF_NAME_VLNK,
  UPLL_IDX_BOUNDARY_NAME_VLNK,
  UPLL_IDX_LABEL_TYPE_VLNK,
  UPLL_IDX_LABEL_VLNK,
  UPLL_IDX_DESCRIPTION_VLNK
};

/* index enumeration for val_vlink_st structure */
enum val_vlink_st_index {
  UPLL_IDX_OPER_STATUS_VLNKS = 0
};

/* index enumeration for val_rename_vlink structure */
enum val_rename_vlink_index {
  UPLL_IDX_NEW_NAME_RVLNK = 0,
  UPLL_IDX_RENAME_TYPE_RVLNK
};

/* index enumeration for val_nwm structure */
enum val_nwm_index {
  UPLL_IDX_ADMIN_STATUS_NWM = 0
};

/* index enumeration for val_nwm_st structure */
enum val_nwm_st_index {
  UPLL_IDX_STATUS_NWMS = 0,
};

/* index enumeration for val_nwm_host_st structure */
enum val_nwm_host_st_index {
  UPLL_IDX_HOST_ADDRESS_NWMHS = 0,
  UPLL_IDX_STATUS_NWMHMS,
  UPLL_IDX_PING_SEND_NWMHMS,
  UPLL_IDX_PING_RECV_NWMHS,
  UPLL_IDX_PING_ERR_NWMHS,
  UPLL_IDX_PING_TROUBLE_NWMHMS
};

/* index enumeration for val_nwm_host structure */
enum val_nwm_host_index {
  UPLL_IDX_HEALTH_INTERVAL_NWMH = 0,
  UPLL_IDX_RECOVERY_INTERVAL_NWMH,
  UPLL_IDX_FAILURE_COUNT_NWMH,
  UPLL_IDX_RECOVERY_COUNT_NWMH,
  UPLL_IDX_WAIT_TIME_NWMH
};

/* index enumeration for val_vterm structure */
enum val_vterm_index {
  UPLL_IDX_CONTROLLER_ID_VTERM = 0,
  UPLL_IDX_DOMAIN_ID_VTERM,
  UPLL_IDX_DESC_VTERM
};

/* index enumeration for val_rename_vterm structure */
enum val_rename_vterm_index {
  UPLL_IDX_NEW_NAME_RVTERM = 0,
  UPLL_IDX_RENAME_TYPE_RVTERM

};

/* index enumeration for val_vterm_st structure */
enum val_vterm_st_index {
  UPLL_IDX_OPER_STATUS_VTERMS = 0
};

/* index enumeration for val_vterm_if structure */
enum val_vterm_if_index {
  UPLL_IDX_ADMIN_STATUS_VTERMI = 0,
  UPLL_IDX_DESC_VTERMI,
  UPLL_IDX_PM_VTERMI
};

/* index enumeration for val_vterm_if_st structure */
enum val_vterm_if_st_index {
  UPLL_IDX_OPER_STATUS_VTERMIS = 0,
};

/* enums used for pom IPC structures */

/* IP type enum in val_flowlist */
enum flowlist_ip_type {
  UPLL_FLOWLIST_TYPE_IP = 0,    /* IPv4 */
  UPLL_FLOWLIST_TYPE_IPV6       /* IPv6 */
};

/* enum for Flowfilter action values */
enum flowfilter_action {
  UPLL_FLOWFILTER_ACT_PASS = 0,     /* pass */
  UPLL_FLOWFILTER_ACT_DROP,         /* drop */
  UPLL_FLOWFILTER_ACT_REDIRECT      /* penalty */
};

/* enum for Flowfilter direction values */
enum flowfilter_direction {
  UPLL_FLOWFILTER_DIR_IN = 0,   /* in */
  UPLL_FLOWFILTER_DIR_OUT       /* out */
};

/* action enum in val_policingprofile_entry */
enum policingprofile_action {
  UPLL_POLICINGPROFILE_ACT_PASS = 0,     /* pass */
  UPLL_POLICINGPROFILE_ACT_DROP,         /* drop */
  UPLL_POLICINGPROFILE_ACT_PENALTY       /* penalty */
};

/* rate type enum in val_policingprofile_entry */
enum policingprofile_rate_type {
  UPLL_POLICINGPROFILE_RATE_KBPS = 0,       /* Kbps */
  UPLL_POLICINGPROFILE_RATE_PPS             /* pps */
};

/* valid array enum in val_flowlist */
enum val_flowlist_index {
  UPLL_IDX_IP_TYPE_FL = 0     /* ip_type */
};

/* valid array enum in val_rename_flowlist */
enum val_rename_flowlist_index {
  UPLL_IDX_RENAME_FLOWLIST_RFL = 0,    /* flowlist_newname */
  UPLL_IDX_RENAME_TYPE_RFL
};

/* valid array enum in val_flowlist_entry index */
enum val_flowlist_entry_index {
  UPLL_IDX_MAC_DST_FLE = 0,             /* mac_dst */
  UPLL_IDX_MAC_SRC_FLE,                 /* mac_src */
  UPLL_IDX_MAC_ETH_TYPE_FLE,            /* mac_eth_type */
  UPLL_IDX_DST_IP_FLE,                  /* dst_ip */
  UPLL_IDX_DST_IP_PREFIX_FLE,           /* dst_ip_prefix */
  UPLL_IDX_SRC_IP_FLE,                  /* src_ip */
  UPLL_IDX_SRC_IP_PREFIX_FLE,           /* src_ip_prefix */
  UPLL_IDX_VLAN_PRIORITY_FLE,           /* vlan_priority */
  UPLL_IDX_DST_IP_V6_FLE,               /* dst_ipv6 */
  UPLL_IDX_DST_IP_V6_PREFIX_FLE,        /* dst_ipv6_prefix */
  UPLL_IDX_SRC_IP_V6_FLE,               /* src_ipv6 */
  UPLL_IDX_SRC_IP_V6_PREFIX_FLE,        /* src_ipv6_entry */
  UPLL_IDX_IP_PROTOCOL_FLE,             /* ip_proto */
  UPLL_IDX_IP_DSCP_FLE,                 /* ip_dscp */
  UPLL_IDX_L4_DST_PORT_FLE,             /* l4_dst_port */
  UPLL_IDX_L4_DST_PORT_ENDPT_FLE,       /* l4_dst_port_endpt */
  UPLL_IDX_L4_SRC_PORT_FLE,             /* l4_src_port */
  UPLL_IDX_L4_SRC_PORT_ENDPT_FLE,       /* l4_src_port_endpt */
  UPLL_IDX_ICMP_TYPE_FLE,               /* icmp_type */
  UPLL_IDX_ICMP_CODE_FLE,               /* icmp_code */
  UPLL_IDX_ICMP_V6_TYPE_FLE,            /* icmpv6_type */
  UPLL_IDX_ICMP_V6_CODE_FLE             /* icmpv6_code */
};

/* valid array enum in pom_stats */
enum val_pom_stats_index {
  UPLL_IDX_STATS_PACKETS= 0,    /* packets */
  UPLL_IDX_STATS_BYTES          /* bytes */
};

/* valid array enum in val_vtn_flowfilter_controller_st */
enum val_vtn_flowfilter_controller_st_index {
  UPLL_IDX_DIRECTION_VFFCS = 0,     /* direction */
  UPLL_IDX_SEQ_NUM_VFFCS,           /* sequence_num */
  UPLL_IDX_FLOW_LIST_VFFCS,         /* flowlist */
  UPLL_IDX_IP_TYPE_VFFCS,           /* ip_type */
  UPLL_IDX_ACTION_VFFCS,            /* action */
  UPLL_IDX_DSCP_VFFCS,              /* dscp */
  UPLL_IDX_PRIORITY_VFFCS,          /* priority */
  UPLL_IDX_NWM_STATUS_VFFCS,        /* n/w monitor */
  UPLL_IDX_SOFTWARE_VFFCS,          /* software */
  UPLL_IDX_EXIST_VFFCS,             /* exist */
  UPLL_IDX_EXPIRE_VFFCS,            /* expire */
  UPLL_IDX_TOTAL_VFFCS              /* total */
};

/* valid array enum in val_flowlist_entry_st */
enum val_flowlist_entry_st_index {
  UPLL_IDX_SEQ_NUM_FLES = 0,      /* sequence_num */
  UPLL_IDX_SOFTWARE_FLES,         /* software */
  UPLL_IDX_EXIST_FLES,            /* exist */
  UPLL_IDX_EXPIRE_FLES,           /* expire */
  UPLL_IDX_TOTAL_FLES             /* total */
};

/* valid array enum in val_vtn_flowfilter_entry */
enum val_vtn_flowfilter_entry_index {
  UPLL_IDX_FLOWLIST_NAME_VFFE = 0,    /* flowlist_name */
  UPLL_IDX_ACTION_VFFE,               /* action */
  UPLL_IDX_NWN_NAME_VFFE,             /* nwm_name */
  UPLL_IDX_DSCP_VFFE,                 /* dscp */
  UPLL_IDX_PRIORITY_VFFE              /* priority */
};

/* valid array enum in val_flowfilter_controller */
enum val_flowfilter_controller_index {
  UPLL_IDX_DIRECTION_FFC = 0,   /* direction */
  UPLL_IDX_SEQ_NUM_FFC          /* sequence_num */
};

/* valid array enum in val_flowfilter_entry_st */
enum val_flowfilter_entry_st_index {
  UPLL_IDX_SEQ_NUM_FFES = 0,      /* sequence_num */
  UPLL_IDX_NWM_STATUS_FFES,       /* N/W monitor */
  UPLL_IDX_SOFTWARE_FFES,         /* software */
  UPLL_IDX_EXIST_FFES,            /* exist */
  UPLL_IDX_EXPIRE_FFES,           /* expired */
  UPLL_IDX_TOTAL_FFES             /* total */
};

/* valid array enum in val_flowfilter_entry */
enum val_flowfilter_entry_index {
  UPLL_IDX_FLOWLIST_NAME_FFE = 0,  /* flowlist_name */
  UPLL_IDX_ACTION_FFE,             /* action */
  UPLL_IDX_REDIRECT_NODE_FFE,      /* redirect_node */
  UPLL_IDX_REDIRECT_PORT_FFE,      /* redirect_port */
  UPLL_IDX_REDIRECT_DIRECTION_FFE, /* redirect direction */
  UPLL_IDX_MODIFY_DST_MAC_FFE,     /* modify_dstmac */
  UPLL_IDX_MODIFY_SRC_MAC_FFE,     /* modify_srcmac */
  UPLL_IDX_NWM_NAME_FFE,           /* nwm_name */
  UPLL_IDX_DSCP_FFE,               /* dscp */
  UPLL_IDX_PRIORITY_FFE            /* priority */
};

/* valid array enum in val_rename_policingprofile */
enum val_rename_policingprofile_index {
  UPLL_IDX_RENAME_PROFILE_RPP = 0,    /* policingprofile_newname */
  UPLL_IDX_RENAME_TYPE_RPP
};

/* valid array enum in val_policingprofile_entry */
enum val_policingprofile_entry_index {
  UPLL_IDX_FLOWLIST_PPE = 0,      /* flowlist */
  UPLL_IDX_RATE_PPE,              /* rate */
  UPLL_IDX_CIR_PPE,               /* cir */
  UPLL_IDX_CBS_PPE,               /* cbs */
  UPLL_IDX_PIR_PPE,               /* pir */
  UPLL_IDX_PBS_PPE,               /* pbs */
  UPLL_IDX_GREEN_ACTION_PPE,      /* green_action */
  UPLL_IDX_GREEN_PRIORITY_PPE,    /* green_action_priority */
  UPLL_IDX_GREEN_DSCP_PPE,        /* green_action_dscp */
  UPLL_IDX_GREEN_DROP_PPE,        /* green_action_drop_precedence */
  UPLL_IDX_YELLOW_ACTION_PPE,     /* yellow_action */
  UPLL_IDX_YELLOW_PRIORITY_PPE,   /* yellow_action_priority */
  UPLL_IDX_YELLOW_DSCP_PPE,       /* yellow_action_dscp */
  UPLL_IDX_YELLOW_DROP_PPE,       /* yellow_action_drop_precedence */
  UPLL_IDX_RED_ACTION_PPE,        /* red_action */
  UPLL_IDX_RED_PRIORITY_PPE,      /* red_action_priority */
  UPLL_IDX_RED_DSCP_PPE,          /* red_action_dscp */
  UPLL_IDX_RED_DROP_PPE           /* red_action_drop_precedence */
};

/* valid array enum in val_policingmap */
enum val_policingmap_index {
  UPLL_IDX_POLICERNAME_PM = 0    /* policer_name */
};

/* valid array enum in val_policingmap_controller_st */
enum val_policingmap_controller_st_index {
  UPLL_IDX_SEQ_NUM_PMCS = 0,    /* sequence_num */
  UPLL_IDX_TOTAL_PMCS,          /* total */
  UPLL_IDX_GREEN_YELLOW_PMCS,   /* green_yellow */
  UPLL_IDX_RED_PMCS,            /* red */
};

/* valid array enum in val_policingmap_switch_st */
enum val_policingmap_switch_st_index {
  UPLL_IDX_POLICER_ID_PMSS = 0,   /* policer_id */
  UPLL_IDX_SWITCH_ID_PMSS,        /* dpid */
  UPLL_IDX_STATUS_PMSS,           /* status */
  UPLL_IDX_VNODE_NAME_PMSS,       /* vnode name*/
  UPLL_IDX_VNODE_IF_NAME_PMSS,    /* vnode if_name */
  UPLL_IDX_PORT_NAME_PMSS,        /* port_name */
  UPLL_IDX_VLAN_ID_PMSS,          /* vlanid */
  UPLL_IDX_TOTAL_PMSS,            /* total */
  UPLL_IDX_GREEN_YELLOW_PMSS,     /* green_yellow */
  UPLL_IDX_RED_PMSS               /* red */
};

/* valid array enum in val_policingmap_controller */
enum val_policingmap_controller_index {
  UPLL_IDX_SEQ_NUM_PMC = 0    /* sequence_num */
};

/* valid array enum in val_vtn_dataflow_cmn */
enum val_vtn_dataflow_cmn_index {
  UPLL_IDX_CONTROLLER_ID_VVDC = 0,           /* Controller id */
  UPLL_IDX_CONTROLLER_TYPE_VVDC,             /* Controller type */
  UPLL_IDX_FLOW_ID_VVDC,                     /* Flow id */
  UPLL_IDX_CREATED_TIME_VVDC,                /* Created time */
  UPLL_IDX_IDLE_TIMEOUT_VVDC,                /* Idle timeout */
  UPLL_IDX_HARD_TIMEOUT_VVDC,                /* Hard timeout */
  UPLL_IDX_INGRESS_VNODE_VVDC,               /* Ingress vNode */
  UPLL_IDX_INGRESS_VINTERFACE_VVDC,          /* Ingress vNode interface */
  UPLL_IDX_INGRESS_SWITCH_ID_VVDC,           /* Ingress switch id */
  UPLL_IDX_INGRESS_PORT_ID_VVDC,             /* Ingress port id */
  UPLL_IDX_INGRESS_LOGICAL_PORT_ID_VVDC,     /* Ingress logical port id */
  UPLL_IDX_INGRESS_DOMAIN_VVDC,              /* Ingress domain */
  UPLL_IDX_EGRESS_VNODE_VVDC,                /* Egress vNode */
  UPLL_IDX_EGRESS_VINTERFACE_VVDC,           /* Egress vNode interface */
  UPLL_IDX_EGRESS_SWITCH_ID_VVDC,            /* Egress switch id */
  UPLL_IDX_EGRESS_PORT_ID_VVDC,              /* Egress port id */
  UPLL_IDX_EGRESS_LOGICAL_PORT_ID_VVDC,     /* Ingress logical port id */
  UPLL_IDX_EGRESS_DOMAIN_VVDC,               /* Egress domain */
  UPLL_IDX_MATCH_COUNT_VVDC,                 /* Number of match */
  UPLL_IDX_ACTION_COUNT_VVDC,                /* Number of count */
  UPLL_IDX_PATH_INFO_COUNT_VVDC              /* Number of path */
};

/* valid array enum in val_vtn_dataflow_path_info */
enum val_vtn_dataflow_path_info_index {
  UPLL_IDX_IN_VNODE_VVDPI = 0, /* In VTN node */
  UPLL_IDX_IN_VIF_VVDPI,       /* In VTN interface */
  UPLL_IDX_OUT_VNODE_VVDPI,    /* Out VTN node */
  UPLL_IDX_OUT_VIF_VVDPI,      /* Out VTN interface */
  UPLL_IDX_VLINK_FLAG_VVDPI,   /* Vlink flag */
  UPLL_IDX_STATUS_VVDPI        /* flow status */
};

enum val_vtn_dataflow_status {
  UPLL_DATAFLOW_PATH_STATUS_NORMAL_=0,  /* Normal data flow */
  UPLL_DATAFLOW_PATH_STATUS_DROP        /* Dropped data flow */
};

enum val_vtn_dataflow_vlink_flag {
  UPLL_DATAFLOW_PATH_VLINK_NOT_EXISTS_ =0,  /* Vlink Not Exist */
  UPLL_DATAFLOW_PATH_VLINK_EXISTS        /* Vlink Exist */
};

enum val_vtn_dataflow_index {
  UPLL_IDX_REASON_VVD = 0,      /* Reason */
  UPLL_IDX_CTRLR_DOMAIN_COUNT_VVD   /* Hop count */
};

/* Import operation type enum */
enum upll_import_type {
  UPLL_IMPORT_TYPE_FULL = 0,
  UPLL_IMPORT_TYPE_PARTIAL = 1
};

/* Rename type enum */
enum upll_rename_type {
  UPLL_RENAME_TYPE_MANUAL = 0,
  UPLL_RENAME_TYPE_AUTO = 1
};

enum val_vnode_type {
  UPLL_VNODE_VBRIDGE = 0,
  UPLL_VNODE_VROUTER,
  UPLL_VNODE_VTERMINAL,
  UPLL_VNODE_VUNKNOWN,
  UPLL_VNODE_VTEP,
  UPLL_VNODE_VTUNNEL
};

/* enum for unified network routing type values */
enum val_unified_nw_routing_type {
  UPLL_ROUTING_TYPE_DIRECT_CONNECTED = 0,           /* Direct connected    */
  UPLL_ROUTING_TYPE_QINQ_TO_QINQ,                   /* QinQ to QinQ        */
  UPLL_ROUTING_TYPE_VLAN_CONNECTED                  /* VLAN connected      */
};

/* enum for label type values */
enum val_label_type {
  UPLL_LABEL_TYPE_VLAN = 1,
  UPLL_LABEL_TYPE_QIQ,
  UPLL_LABEL_TYPE_VNI,
  UPLL_LABEL_TYPE_GW_VLAN = 100
  
};

/* index enumeration for val_vbr_portmap structure */
enum val_vbr_portmap_index {
  UPLL_IDX_CONTROLLER_ID_VBRPM = 0,                /* controller identifier   */
  UPLL_IDX_DOMAIN_ID_VBRPM,                        /* domain identifier       */
  UPLL_IDX_LOGICAL_PORT_ID_VBRPM,                  /* logical port identifier */
  UPLL_IDX_LABEL_TYPE_VBRPM,                       /* label type              */
  UPLL_IDX_LABEL_VBRPM,                            /* label                   */
};

enum val_vbr_portmap_st_index {
  UPLL_IDX_OPER_STATUS_VBRPMS = 0
};


/* index enumeration for val_unified_nw structure */
enum val_unified_nw_index {
  UPLL_IDX_ROUTING_TYPE_UNW = 0,                    /* routing_type        */
  UPLL_IDX_IS_DEFAULT_UNW                           /* is_default          */
};

/* index enumeration for val_unw_spine_domain structure */
enum val_unw_spine_domain_index {
  UPLL_IDX_SPINE_CONTROLLER_ID_UNWS = 0,            /* spine_controller_id */
  UPLL_IDX_SPINE_DOMAIN_ID_UNWS,                    /* spine_domain_id     */
  UPLL_IDX_UNW_LABEL_ID_UNWS                        /* unw_label_id        */
};

/* index enumeration for val_unw_spine_domain_st structure */
enum val_unw_spine_domain_st_index {
  UPLL_IDX_MAX_COUNT_UNWS_ST = 0,                   /* max_count           */
  UPLL_IDX_USED_COUNT_UNWS_ST,                      /* used_count          */
  UPLL_IDX_ALARM_STATUS_UNWS_ST                     /* alarm_status        */
};

/* index enumeration for val_unw_spine_domain_assigned_label structure */
enum val_unw_spine_domain_assigned_label_index {
  UPLL_IDX_LABEL_UNWSAL = 0,                        /* label               */
  UPLL_IDX_VTN_ID_UNWSAL,                           /* vtn_id              */
  UPLL_IDX_VNODE_ID_UNWSAL,                         /* vnode_id            */
};

/* index enumeration for val_unw_spine_domain_fdbentry structure */
enum val_unw_spine_domain_fdbentry_index {
  UPLL_IDX_MAX_COUNT_UNWSDF = 0,                    /* max_count           */
  UPLL_IDX_MAX_SWITCH_ID_UNWSDF,                    /* max_switch_id       */
  UPLL_IDX_MIN_COUNT_UNWSDF,                        /* min_count           */
  UPLL_IDX_MIN_SWITCH_ID_UNWSDF,                    /* min_switch_id       */
  UPLL_IDX_AVG_COUNT_UNWSDF,                        /* avg_count           */
  UPLL_IDX_NO_OF_SWITCHES_UNWSDF,                   /* num_of_switches     */
  UPLL_IDX_VTN_COUNT_UNWSDF                         /* vtn_count           */
};

/* index enumeration for val_unw_spine_domain_fdbentry_vtn structure */
enum val_unw_spine_domain_fdbentry_vtn_index {
  UPLL_IDX_VTN_ID_UNWSDFV = 0,                      /* vtn_id              */
  UPLL_IDX_VLAN_ID_UNWSDFV,                         /* vlan_id             */
  UPLL_IDX_MAX_COUNT_UNWSDFV,                       /* max_count           */
  UPLL_IDX_MAX_SWITCH_ID_UNWSDFV,                   /* max_switch_id       */
  UPLL_IDX_MIN_COUNT_UNWSDFV,                       /* min_count           */
  UPLL_IDX_MIN_SWITCH_ID_UNWSDFV,                   /* min_switch_id       */
  UPLL_IDX_AVG_COUNT_UNWSDFV                        /* avg_count           */
};

/* index enumeration for val_unw_label structure */
enum val_unw_label_index {
  UPLL_IDX_MAX_COUNT_UNWL = 0,                      /* max_count           */
  UPLL_IDX_RAISING_THRESHOLD_UNWL,                  /* raising_threshold   */
  UPLL_IDX_FALLING_THRESHOLD_UNWL                   /* falling_threshold   */
};

/* index enumeration for val_vtn_unified structure */
enum val_vtn_unified_index {
  UPLL_IDX_SPINE_ID_VUNW = 0                        /* spine_id            */
};

/* index enumeration for val_vtn_vtn_flooding_path_st structure */
enum val_vtn_flooding_path_st_index {
  UPLL_IDX_CONTROLLER_ID_VFPS = 0,                  /* controller_id       */
  UPLL_IDX_DOMAIN_ID_VFPS,                          /* domain_id           */
  UPLL_IDX_FLOODING_PATH_ID_VFPS,                   /* flooding_path_id    */
  UPLL_IDX_LAST_UPDATED_VFPS,                       /* last_updated        */
  UPLL_IDX_TREE_COUNT_VFPS                          /* tree_count          */
};

/* index enumeration for val_vtn_vtn_flooding_path_tree structure */
enum val_vtn_flooding_path_tree_index {
  UPLL_IDX_ROOT_SWITCH_ID_VFPT = 0,                 /* root_switch_id      */
  UPLL_IDX_PATH_COUNT_VFPT                          /* path_count          */
};

/* index enumeration for val_vtn_vtn_flooding_path_path structure */
enum val_vtn_flooding_path_path_index {
  UPLL_IDX_MEMBER_SWITCH_ID_VFPP = 0,               /* member_switch_id    */
  UPLL_IDX_LINK_COUNT_VFPP                          /* link_count          */
};

/* index enumeration for val_vtn_vtn_flooding_path_link structure */
enum val_vtn_flooding_path_link_index {
  UPLL_IDX_SRC_SWITCH_ID_VFPL = 0,                  /* src_switch_id       */
  UPLL_IDX_SRC_PORT_VFPL,                           /* src_port            */
  UPLL_IDX_DEST_SWITCH_ID_VFPL,                     /* dest_switch_id      */
  UPLL_IDX_DEST_PORT_VFPL,                          /* dest_port           */
  UPLL_IDX_LINK_WEIGHT_VFPL                         /* link_weight         */
};

/* index enumeration for val_vbr_expand structure */
enum val_vbr_expand_index {
  UPLL_IDX_VBRIDGE_NAME_VBRE = 0,                  /* converted vBridge name    */
  UPLL_IDX_CONTROLLER_ID_VBRE,                     /* controller identifier     */
  UPLL_IDX_DOMAIN_ID_VBRE,                         /* domain identifer          */
  UPLL_IDX_LABEL_VBRE,                             /* vBID                      */
  UPLL_IDX_CONTROLLER_VTN_NAME_VBRE,               /* controller VTN identifier */
  UPLL_IDX_CONTROLLER_VTN_LABEL_VBRE               /* controller VTN label      */
};

/* index enumeration for val_vbr_portmap_expand structure */
enum val_vbr_portmap_expand_index {
  UPLL_IDX_PORTMAP_ID_VBRPME = 0,                  /* vBridge portmap Identifier */
  UPLL_IDX_LOGICAL_PORT_ID_VBRPME,                 /* logical_port_id Identifier */
  UPLL_IDX_LABEL_TYPE_VBRPME,                      /* Label Type Identifier      */
  UPLL_IDX_LABEL_VBRPME                            /* label                      */
};

/* index enumeration for val_vlink_expand structure */
enum val_vlink_expand_index {
  UPLL_IDX_VLINK_NAME_VLNKE = 0,                  /* vLink identifer           */
  UPLL_IDX_VNODE1_NAME_VLNKE,                     /* vNode1 identifer           */
  UPLL_IDX_VNODE1_IF_NAME_VLNKE,                  /* vNode1 interface identifer */
  UPLL_IDX_VNODE2_NAME_VLNKE,                     /* vNode2 indentifier         */
  UPLL_IDX_VNODE2_IF_NAME_VLNKE,                  /* vNode2 interface identifer */
  UPLL_IDX_BOUNDARY_NAME_VLNKE,                   /* boundary name              */
  UPLL_IDX_LABEL_TYPE_VLNKE,                      /* label type                 */
  UPLL_IDX_LABEL_VLNKE                           /* label                      */
};

/* index enumeration for val_vtunnel_expand structure */
enum val_vtunnel_expand_index {
  UPLL_IDX_VTUNNEL_NAME_VTNLE = 0,                /* converted vTunnel name    */
  UPLL_IDX_CONTROLLER_ID_VTNLE,                   /* controller identifier     */
  UPLL_IDX_DOMAIN_ID_VTNLE,                       /* domain identifer          */
  UPLL_IDX_LABEL_VTNLE                            /* label                     */
};

/* index enumeration for val_vtunnel_if_expand structure */
enum val_vtunnel_if_expand_index {
  UPLL_IDX_IF_NAME_VTNLIE = 0,                    /* Interface Identifier           */
  UPLL_IDX_CONN_VNODE_NAME_VTNLIE,                /* connected vNode Identifier     */
  UPLL_IDX_CONN_VNODE_IF_NAME_VTNLIE,             /* connected Interface Identifier */
  UPLL_IDX_CONN_VLINK_NAME_VTNLIE                 /* connected vLink Identifier     */
};

/* index enumeration for val_vbr_if_expand structure */
enum val_vbr_if_expand_index {
  UPLL_IDX_IF_NAME_VBRIE = 0,                     /* Interface Identifier           */
  UPLL_IDX_CONN_VNODE_NAME_VBRIE,                 /* connected vNode Identifier     */
  UPLL_IDX_CONN_VNODE_IF_NAME_VBRIE,              /* connected Interface Identifier */
  UPLL_IDX_CONN_VLINK_NAME_VBRIE                  /* connected vLink Identifier     */
};


UNC_C_END_DECL

#endif  /* IPC_UPLL_IPC_ENUM_H_*/
