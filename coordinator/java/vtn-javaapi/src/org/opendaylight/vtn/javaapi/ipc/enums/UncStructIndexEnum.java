/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.javaapi.ipc.enums;

public class UncStructIndexEnum {
	// valid
	public enum Valid {
		UNC_VF_INVALID,
		UNC_VF_VALID,
		UNC_VF_VALID_NO_VALUE,
		UNC_VF_NOT_SUPPORTED,
		UNC_VF_VALUE_NOT_MODIFIED
	};

	// vtn
	public enum ValVtnIndex {
		UPLL_IDX_DESC_VTN
	};

	// vtn response
	public enum ValVtnStIndex {
		UPLL_IDX_OPER_STATUS_VS,
		UPLL_IDX_ALARM_STATUS_VS,
		UPLL_IDX_CREATEION_TIME_VS,
		UPLL_IDX_LAST_UPDATE_TIME_VS
	};

	public enum ValVtunnelIfIndex {
		UPLL_IDX_DESC_VTNL_IF,
		UPLL_IDX_ADMIN_ST_VTNL_IF,
		UPLL_IDX_PORT_MAP_VTNL_IF

	};

	public enum ValVlinkIndex {
		UPLL_IDX_ADMIN_STATUS_VLNK,
		UPLL_IDX_VNODE1_NAME_VLNK,
		UPLL_IDX_VNODE1_IF_NAME_VLNK,
		UPLL_IDX_VNODE2_NAME_VLNK,
		UPLL_IDX_VNODE2_IF_NAME_VLNK,
		UPLL_IDX_BOUNDARY_NAME_VLNK,
		UPLL_IDX_LABEL_TYPE_VLNK,
		UPLL_IDX_LABEL_VLINK,
		UPLL_IDX_DESCRIPTION_VLNK
	}

	public enum ValVtepVndex {
		UPLL_IDX_DESC_VTEP,
		UPLL_IDX_CONTROLLER_ID_VTEP,
		UPLL_IDX_DOMAIN_ID_VTEP

	};

	public enum ValVtepIfIndex {

		UPLL_IDX_DESC_VTEPI, UPLL_IDX_ADMIN_ST_VTEPI, UPLL_IDX_PORT_MAP_VTEPI

	};

	public enum ValVtepGroupMemberIndex {
		UPLL_IDX_VTEPGRP_KEY, UPLL_IDX_VTEPMEMBER_NAME
	};

	public enum FlowlistIpType {
		UPLL_FLOWLIST_TYPE_IP("0"), UPLL_FLOWLIST_TYPE_IPV6("1");
		private final String value;

		private FlowlistIpType(final String value) {
			this.value = value;
		}

		public String getValue() {
			return value;
		}

	};

	public enum ValFlowlistIndex {
		UPLL_IDX_IP_TYPE_FL
	};

	public enum ValFlowlistEntryIndex {
		UPLL_IDX_MAC_DST_FLE,
		UPLL_IDX_MAC_SRC_FLE,
		UPLL_IDX_MAC_ETH_TYPE_FLE,
		UPLL_IDX_DST_IP_FLE,
		UPLL_IDX_DST_IP_PREFIX_FLE,
		UPLL_IDX_SRC_IP_FLE,
		UPLL_IDX_SRC_IP_PREFIX_FLE,
		UPLL_IDX_VLAN_PRIORITY_FLE,
		UPLL_IDX_DST_IP_V6_FLE,
		UPLL_IDX_DST_IP_V6_PREFIX_FLE,
		UPLL_IDX_SRC_IP_V6_FLE,
		UPLL_IDX_SRC_IP_V6_PREFIX_FLE,
		UPLL_IDX_IP_PROTOCOL_FLE,
		UPLL_IDX_IP_DSCP_FLE,
		UPLL_IDX_L4_DST_PORT_FLE,
		UPLL_IDX_L4_DST_PORT_ENDPT_FLE,
		UPLL_IDX_L4_SRC_PORT_FLE,
		UPLL_IDX_L4_SRC_PORT_ENDPT_FLE,
		UPLL_IDX_ICMP_TYPE_FLE,
		UPLL_IDX_ICMP_CODE_FLE,
		UPLL_IDX_ICMP_V6_TYPE_FLE,
		UPLL_IDX_ICMP_V6_CODE_FLE
	};

	public enum ValVtnFlowfilterEntryIndex {
		UPLL_IDX_FLOWLIST_NAME_VFFE,
		UPLL_IDX_ACTION_VFFE,
		UPLL_IDX_NWN_NAME_VFFE,
		UPLL_IDX_DSCP_VFFE,
		UPLL_IDX_PRIORITY_VFFE
	};

	// ValVbr for VBridge APIs
	public enum ValVbrIndex {
		UPLL_IDX_CONTROLLER_ID_VBR,
		UPLL_IDX_DOMAIN_ID_VBR,
		UPLL_IDX_DESC_VBR,
		UPLL_IDX_HOST_ADDR_VBR,
		UPLL_IDX_HOST_ADDR_PREFIXLEN_VBR
	};

	// ValVbrSt for response of VBridge APIs
	public enum ValVbrStIndex {
		UPLL_IDX_OPER_STATUS_VBRS,
	};

	// ValVbrIf for VBridgeInterface APIs
	public enum ValVbrIfIndex {
		UPLL_IDX_ADMIN_STATUS_VBRI, UPLL_IDX_DESC_VBRI, UPLL_IDX_PM_VBRI
	};

	// ValPortMap for VBridgeInterface APIs
	public enum ValPortMapIndex {
		UPLL_IDX_LOGICAL_PORT_ID_PM, UPLL_IDX_VLAN_ID_PM, UPLL_IDX_TAGGED_PM
	};

	// ValFlowfilterEntry
	public enum ValFlowfilterEntryIndex {
		UPLL_IDX_FLOWLIST_NAME_FFE,
		UPLL_IDX_ACTION_FFE,
		UPLL_IDX_REDIRECT_NODE_FFE,
		UPLL_IDX_REDIRECT_PORT_FFE,
		UPLL_IDX_REDIRECT_DIRECTION_FFE,
		UPLL_IDX_MODIFY_DST_MAC_FFE,
		UPLL_IDX_MODIFY_SRC_MAC_FFE,
		UPLL_IDX_NWM_NAME_FFE,
		UPLL_IDX_DSCP_FFE,
		UPLL_IDX_PRIORITY_FFE
	};

	// FFType_Direction

	public enum FlowfilterDirection {
		UPLL_FLOWFILTER_DIR_IN("0"), UPLL_FLOWFILTER_DIR_OUT("1");
		private final String value;

		private FlowfilterDirection(final String value) {
			this.value = value;
		}

		public String getValue() {
			return value;
		}
	};

	// Action
	public enum FlowfilterAction {
		UPLL_FLOWFILTER_ACT_PASS("0"),
		UPLL_FLOWFILTER_ACT_DROP("1"),
		UPLL_FLOWFILTER_ACT_REDIRECT("2");
		private final String value;

		private FlowfilterAction(final String value) {
			this.value = value;
		}

		public String getValue() {
			return value;
		}
	};

	public enum ValVrtIndex {
		UPLL_IDX_CONTROLLER_ID_VRT,
		UPLL_IDX_DOMAIN_ID_VRT,
		UPLL_IDX_DESC_VRT,
		UPLL_IDX_DHCP_RELAY_ADMIN_STATUS_VRT
	};

	// ValVrtIf
	public enum ValVrtIfIndex {
		UPLL_IDX_DESC_VI,
		UPLL_IDX_IP_ADDR_VI,
		UPLL_IDX_PREFIXLEN_VI,
		UPLL_IDX_MAC_ADDR_VI,
		UPLL_IDX_ADMIN_ST_VI,
	};

	// ValVtunnel
	public enum ValVtunnelIndex {
		UPLL_IDX_DESC_VTNL,
		UPLL_IDX_CONTROLLER_ID_VTNL,
		UPLL_IDX_DOMAIN_ID_VTNL,
		UPLL_IDX_VTN_NAME_VTNL,
		UPLL_IDX_VTEP_GRP_NAME_VTNL,
		UPLL_IDX_LABEL_VTNL,

	};

	public enum ValLabelType {
		UPLL_LBL_TYP_MPLS, UPLL_LBL_TYPE_VNI, UPLL_LBL_TYPE_VSID
	};

	// VUnkown
	public enum ValVunknownIndex {
		UPLL_IDX_DESC_VUN,
		UPLL_IDX_TYPE_VUN,
		UPLL_IDX_CONTROLLER_ID_VUN,
		UPLL_IDX_DOMAIN_ID_VUN
	};

	// VUnknown Interface
	public enum ValVunkIfIndex {
		UPLL_IDX_DESC_VUNI, UPLL_IDX_ADMIN_ST_VUNI
	};

	// flow filter Entry controller
	public enum ValFlowfilterControllerIndex {
		UPLL_IDX_DIRECTION_FFC, UPLL_IDX_SEQ_NUM_FFC
	};

	/*
	 * ARP Entry
	 */
	public enum ValVrtArpEntryStIndex {
		UPLL_IDX_MAC_ADDR_VAES,
		UPLL_IDX_IP_ADDR_VAES,
		UPLL_IDX_TYPE_VAES,
		UPLL_IDX_IF_NAME_VAES
	};

	// Policing Profile Entry rate Type
	public enum ValPolicingProfileRateType {
		UPLL_POLICINGPROFILE_RATE_KBPS, UPLL_POLICINGPROFILE_RATE_PPS
	};

	// Policing Profile Entry Action type
	public enum ValPolicingProfileAction {
		UPLL_POLICINGPROFILE_ACT_PASS,
		UPLL_POLICINGPROFILE_ACT_DROP,
		UPLL_POLICINGPROFILE_ACT_PENALTY
	};

	public enum ValAdminStatus {
		UPLL_ADMIN_ENABLE("1"), UPLL_ADMIN_DISABLE("2");
		private final String value;

		private ValAdminStatus(final String value) {
			this.value = value;
		}

		/**
		 * 
		 * @return the value
		 */
		public String getValue() {
			return value;
		}
	};

	public enum ValVbrPortType {
		UPLL_PORT, UPLL_TRUNK
	};

	/*
	 * public enum TagStatus{ UPPL_UNTAG, UPPL_TAG
	 * 
	 * };
	 */

	/*
	 * public enum TagStatus { UPPL_UNTAG("0"), UPPL_TAG("1");
	 * 
	 * private final String value;
	 * 
	 * private TagStatus(final String value) { this.value = value; }
	 *//**
	 * 
	 * @return the value
	 */
	/*
	 * public String getValue(){ return value; } };
	 */

	public enum vlan_tagged {
		UPLL_VLAN_UNTAGGED("0"), UPLL_VLAN_TAGGED("1");

		private final String value;

		private vlan_tagged(final String value) {
			this.value = value;
		}

		public String getValue() {
			return value;
		}

	};

	public enum ValStaticIpRouteIndex {
		UPLL_IDX_NWM_NAME_SIR,
		UPLL_IDX_GROUP_METRIC_SIR
	};

	// ValVlanMap
	public enum ValVlanMapIndex {
		UPLL_IDX_VLAN_ID_VM
	};

	// ValVbrvalVbrMacEntrySt
	public enum valVbrMacEntryStIndex {
		UPLL_IDX_MAC_ADDR_VMES,
		UPLL_IDX_TYPE_VMES,
		UPLL_IDX_IF_NAME_VMES,
		UPLL_IDX_IF_KIND_VMES
	};

	public enum ValVrtStIndex {
		UPLL_IDX_OPER_STATUS_VRTS
	};

	public enum ValMacEntry {
		UPLL_MAC_ENTRY_STATIC("0"), UPLL_MAC_ENTRY_DYNAMIC("1");

		private final String value;

		private ValMacEntry(final String value) {
			this.value = value;
		}

		/**
		 * 
		 * @return the value
		 */
		public String getValue() {
			return value;
		}
	};

	public enum ValVtnFlowfilterControllerStIndex {
		UPLL_IDX_DIRECTION_VFFCS,
		UPLL_IDX_SEQ_NUM_VFFCS,
		UPLL_IDX_FLOW_LIST_VFFCS,
		UPLL_IDX_IP_TYPE_VFFCS,
		UPLL_IDX_ACTION_VFFCS,
		UPLL_IDX_DSCP_VFFCS,
		UPLL_IDX_PRIORITY_VFFCS,
		UPLL_IDX_NWM_STATUS_VFFCS,
		UPLL_IDX_SOFTWARE_VFFCS,
		UPLL_IDX_EXIST_VFFCS,
		UPLL_IDX_EXPIRE_VFFCS,
		UPLL_IDX_TOTAL_VFFCS

	};

	// val_vbr_l2_domain_st
	public enum valVbrL2DomainStIndex {
		UPLL_IDX_L2_DOMAIN_ID_VL2DS, UPLL_IDX_OFS_COUNT_VL2DS
	};

	// val_vbr_l2_domain_member_st
	public enum valVbrL2DomainMemberStIndex {
		UPLL_IDX_SWITCH_ID_VL2DMS, UPLL_IDX_VLAN_ID_VL2DMS

	};

	public enum ValDhcpRelayIfStIndex {
		UPLL_IDX_IF_NAME_DRIS, UPLL_IDX_DHCP_RELAY_STATUS_DRIS
	};

	public enum ValPomStatsIndex {
		UPLL_IDX_STATS_PACKETS, UPLL_IDX_STATS_BYTES
	};

	public enum ValFlowlistEntryStIndex {
		UPLL_IDX_SEQ_NUM_FLES,
		UPLL_IDX_SOFTWARE_FLES,
		UPLL_IDX_EXIST_FLES,
		UPLL_IDX_EXPIRE_FLES,
		UPLL_IDX_TOTAL_FLES
	};

	// val_flowfilter_entry_st
	public enum ValFlowfilterEntryStIndex {
		UPLL_IDX_SEQ_NUM_FFES,
		UPLL_IDX_NWM_STATUS_FFES,
		UPLL_IDX_SOFTWARE_FFES,
		UPLL_IDX_EXIST_FFES,
		UPLL_IDX_EXPIRE_FFES,
		UPLL_IDX_TOTAL_FFES

	};

	public enum ValVtunnelStIndex {
		UPLL_IDX_OPER_STATUS_VTNLS
	};

	public enum ValVlinkStIndex {
		UPLL_IDX_OPER_STATUS_VLNKS
	};

	public enum ValVrtIfStIndex {
		UPLL_IDX_OPER_STATUS_VRTIS
	};

	public enum ValVtnNeighborIndex {
		UPLL_IDX_CONN_VNODE_NAME_VN,
		UPLL_IDX_CONN_VNODE_IF_NAME_VN,
		UPLL_IDX_CONN_VLINK_NAME_VN
	};

	public enum ValVtepIfStIndex {
		UPLL_IDX_IF_OPER_STATUS_VTEPIS
	};

	public enum PfcStatus {
		PFC_TRUE("1"), PFC_FALSE("0");
		private final String value;

		private PfcStatus(final String value) {
			this.value = value;
		}

		public String getValue() {
			return value;
		}
	};

	public enum ValOperStatus {
		UPLL_OPER_STATUS_UP("1"),
		UPLL_OPER_STATUS_DOWN("2"),
		UPLL_OPER_STATUS_UNKNOWN("3");
		private final String value;

		private ValOperStatus(final String value) {
			this.value = value;
		}

		public String getValue() {
			return value;
		}
	};

	public enum val_vrt_ip_route_st_index {
		UPLL_IDX_DESTINATION_VIRS,
		UPLL_IDX_GATEWAY_VIRS,
		UPLL_IDX_PREFIXLEN_VIRS,
		UPLL_IDX_FLAGS_VIRS,
		UPLL_IDX_METRIC_VIRS,
		UPLL_IDX_USE_VIRS,
		UPLL_IDX_IF_NAME_VIRS,
		UPLL_IDX_NW_MONITOR_GR_VIRS,
		UPLL_IDX_GR_METRIC_VIRS
	};

	public enum val_vrt_arp_entry_st_index {
		UPLL_IDX_MAC_ADDR_VAES,
		UPLL_IDX_IP_ADDR_VAES,
		UPLL_IDX_TYPE_VAES,
		UPLL_IDX_IF_NAME_VAES
	};

	public enum val_vtep_index {
		UPLL_IDX_DESC_VTEP,
		UPLL_IDX_CONTROLLER_ID_VTEP,
		UPLL_IDX_DOMAIN_ID_VTEP

	};

	public enum val_vtep_st_index {
		UPLL_IDX_OPER_STATUS_VTEPS,
	};

	public enum val_static_ip_route_index {
		UPLL_IDX_GROUP_METRIC_SIR
	};

	public enum ValVtnstationControllerStIndex {
		UPLL_IDX_STATION_ID_VSCS,
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
	
	public enum ValVnodeType  {
		   UPLL_VNODE_VBRIDGE ,
		   UPLL_VNODE_VROUTER,
		   UPLL_VNODE_VTERMINAL,
		   UPLL_VNODE_VUNKNOWN,
		   UPLL_VNODE_VTEP,
		   UPLL_VNODE_VTUNNEL
		};


	public enum val_vbr_port_type {
		UPLL_PORT, UPLL_TRUNK
	};

	public enum ValVbrIfMapType {
		UPLL_IF_OFS_MAP("0"), UPLL_IF_VLAN_MAP("1");

		private final String value;

		private ValVbrIfMapType(final String value) {
			this.value = value;
		}

		public String getValue() {
			return value;
		}
	};

	public enum ValVtnMapStatus {
		UPLL_VTN_MAP_VALID("0"), UPLL_VTN_MAP_INVALID("1");

		private final String value;

		private ValVtnMapStatus(final String value) {
			this.value = value;
		}

		public String getValue() {
			return value;
		}
	};

	public enum ValVtnStationControllerStatIndex {
		UPLL_IDX_ALL_TX_PKT_VSCS,
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

	public enum val_vtunnel_if_index {
		UPLL_IDX_DESC_VTNL_IF, UPLL_IDX_ADMIN_ST_VTNL_IF
	};

	public enum val_vtunnel_if_st_index {
		UPLL_IDX_IF_OPER_STATUS_VTNLI
	};

	public enum ValDhcpRelayIfStatus {
		UPLL_DR_IF_INACTIVE("0"), UPLL_DR_IF_ACTIVE("1"), UPLL_DR_IF_STARTING(
				"2"), UPLL_DR_IF_WAITING("3"), UPLL_DR_IF_ERROR("4");
		private final String value;

		private ValDhcpRelayIfStatus(final String value) {
			this.value = value;
		}

		public String getValue() {
			return value;
		}
	};

	public enum ValVunknowntype {
		VUNKNOWN_TYPE_BRIDGE("0"), VUNKNOWN_TYPE_ROUTER("1");
		private final String value;

		private ValVunknowntype(final String value) {
			this.value = value;
		}

		public String getValue() {
			return value;
		}
	};

	public enum val_vtep_grp_index {
		UPLL_IDX_CONTROLLER_ID_VTEPG, UPLL_IDX_DESCRIPTION_VTEPG
	};

	public enum ValVrtIpRouteStIndex {
		UPLL_IDX_DESTINATION_VIRS,
		UPLL_IDX_GATEWAY_VIRS,
		UPLL_IDX_PREFIXLEN_VIRS,
		UPLL_IDX_FLAGS_VIRS,
		UPLL_IDX_METRIC_VIRS,
		UPLL_IDX_USE_VIRS,
		UPLL_IDX_IF_NAME_VIRS,
		UPLL_IDX_NW_MONITOR_GR_VIRS,
		UPLL_IDX_GR_METRIC_VIRS
	};

	public enum ValMacEntryIfKind {
		UPLL_MAC_ENTRY_BLANK("0"), UPLL_MAC_ENTRY_TRUNK("1");

		private final String value;

		private ValMacEntryIfKind(final String value) {
			this.value = value;
		}

		/**
		 * 
		 * @return the value
		 */
		public String getValue() {
			return value;
		}
	};

	public enum ValAlarmStatus {
		UPLL_ALARM_CLEAR("0"), UPLL_ALARM_RAISE("1");

		private final String value;

		private ValAlarmStatus(final String value) {
			this.value = value;
		}

		/**
		 * 
		 * @return the value
		 */
		public String getValue() {
			return value;
		}
	};

	public enum UpplControllerOperStatus {
		UPPL_CONTROLLER_OPER_DOWN("0"),
		UPPL_CONTROLLER_OPER_UP("1"),
		UPPL_CONTROLLER_OPER_WAITING_AUDIT("2"),
		UPPL_CONTROLLER_OPER_AUDITING("3");
		private final String value;

		private UpplControllerOperStatus(final String value) {
			this.value = value;
		}

		public String getValue() {
			return value;
		}
	};

	public enum DirtyStatus {
		FALSE("0"), TRUE("1");
		private final String value;

		private DirtyStatus(final String value) {
			this.value = value;
		}

		public String getValue() {
			return value;
		}
	};

	// U12 ENUM constant
	public enum valVtnMappingControllerStIndex {
		UPLL_IDX_SWITCH_ID_VMCS,
		UPLL_IDX_PORT_NAME_VMCS,
		UPLL_IDX_LOGICAL_PORT_ID_VMCS,
		UPLL_IDX_VLAN_ID_VMCS,
		UPLL_IDX_TAGGED_VMCS,
		UPLL_IDX_MAP_TYPE_VMCS,
		UPLL_IDX_VNODE_TYPE_VMCS,
		UPLL_IDX_VNODE_NAME_VMCS,
		UPLL_IDX_VNODE_IF_NAME_VMCS

	};

	public enum valVtermIndex {
		UPLL_IDX_CONTROLLER_ID_VTERM,
		UPLL_IDX_DOMAIN_ID_VTERM,
		UPLL_IDX_DESC_VTERM

	};

	public enum valVtermStIndex {
		UPLL_IDX_OPER_STATUS_VTERMS

	};

	public enum valVtermIfIndex {
		UPLL_IDX_ADMIN_STATUS_VTERMI, UPLL_IDX_DESC_VTERMI, UPLL_IDX_PM_VTERMI
	};

	public enum valVtermIfStIndex {
		UPLL_IDX_OPER_STATUS_VTERMIS

	};

	public enum valVtnDataflowIndex {
		UPLL_IDX_REASON_VVD, UPLL_IDX_CTRLR_DOMAIN_COUNT_VVD
	};

	public enum valVtnDataflowCmnIndex {
		UPLL_IDX_CONTROLLER_ID_VVDC,
		UPLL_IDX_CONTROLLER_TYPE_VVDC,
		UPLL_IDX_FLOW_ID_VVDC,
		UPLL_IDX_CREATED_TIME_VVDC,
		UPLL_IDX_IDLE_TIMEOUT_VVDC,
		UPLL_IDX_HARD_TIMEOUT_VVDC,
		UPLL_IDX_INGRESS_VNODE_VVDC,
		UPLL_IDX_INGRESS_VINTERFACE_VVDC,
		UPLL_IDX_INGRESS_SWITCH_ID_VVDC,
		UPLL_IDX_INGRESS_PORT_ID_VVDC,
		UPLL_IDX_INGRESS_LOGICAL_PORT_ID_VVDC,
		UPLL_IDX_INGRESS_DOMAIN_VVDC,
		UPLL_IDX_EGRESS_VNODE_VVDC,
		UPLL_IDX_EGRESS_VINTERFACE_VVDC,
		UPLL_IDX_EGRESS_SWITCH_ID_VVDC,
		UPLL_IDX_EGRESS_PORT_ID_VVDC,
		UPLL_IDX_EGRESS_LOGICAL_PORT_ID_VVDC,
		UPLL_IDX_EGRESS_DOMAIN_VVDC,
		UPLL_IDX_MATCH_COUNT_VVDC,
		UPLL_IDX_ACTION_COUNT_VVDC,
		UPLL_IDX_PATH_INFO_COUNT_VVDC
	};

	public enum valVtnDataflowPathInfo {
		UPLL_IDX_IN_VNODE_VVDPI,
		UPLL_IDX_IN_VIF_VVDPI,
		UPLL_IDX_OUT_VNODE_VVDPI,
		UPLL_IDX_OUT_VIF_VVDPI,
		UPLL_IDX_VLINK_FLAG_VVDPI,
		UPLL_IDX_STATUS_VVDPI

	};

	public enum valVtnDataflowPathInfoVlinkType {
		UPLL_DATAFLOW_PATH_VLINK_NOT_EXISTS, UPLL_DATAFLOW_PATH_VLINK_EXISTS
	};

	public enum valVtnDataflowPathInfoStatusType {
		UPLL_DATAFLOW_PATH_STATUS_NORMAL, UPLL_DATAFLOW_PATH_STATUS_DROP
	};

	public enum ValDataFlowMatchVmaskvalue {
		UNC_MATCH_MASK_INVALID, UNC_MATCH_MASK_VALID;

	};

	public enum UncDataflowFlowMatchType {
		UNC_MATCH_IN_PORT,
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
		UNC_MATCH_TP_DST;
	}

	public enum UncDataflowFlowActionType {
		UNC_ACTION_OUTPUT,
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
		UNC_ACTION_SET_IPV6_DST;
	};

	public enum UncDataflowReason {
		UNC_DF_RES_SUCCESS,
		UNC_DF_RES_OPERATION_NOT_SUPPORTED,
		UNC_DF_RES_EXCEEDS_FLOW_LIMIT,
		UNC_DF_RES_CTRLR_DISCONNECTED,
		UNC_DF_RES_EXCEEDS_HOP_LIMIT,
		UNC_DF_RES_DST_NOT_REACHED,
		UNC_DF_RES_FLOW_NOT_FOUND,
		UNC_DF_RES_SYSTEM_ERROR
	};

	public enum policingProfileAction {
		UPLL_POLICINGPROFILE_ACT_PASS,
		UPLL_POLICINGPROFILE_ACT_DROP,
		UPLL_POLICINGPROFILE_ACT_PENALTY
	};

	public enum policingProfileRateType {
		UPLL_POLICINGPROFILE_RATE_KBPS, UPLL_POLICINGPROFILE_RATE_PPS
	};

	public enum valPolicingProfileEntryIndex {
		UPLL_IDX_FLOWLIST_PPE,
		UPLL_IDX_RATE_PPE,
		UPLL_IDX_CIR_PPE,
		UPLL_IDX_CBS_PPE,
		UPLL_IDX_PIR_PPE,
		UPLL_IDX_PBS_PPE,
		UPLL_IDX_GREEN_ACTION_PPE,
		UPLL_IDX_GREEN_PRIORITY_PPE,
		UPLL_IDX_GREEN_DSCP_PPE,
		UPLL_IDX_GREEN_DROP_PPE,
		UPLL_IDX_YELLOW_ACTION_PPE,
		UPLL_IDX_YELLOW_PRIORITY_PPE,
		UPLL_IDX_YELLOW_DSCP_PPE,
		UPLL_IDX_YELLOW_DROP_PPE,
		UPLL_IDX_RED_ACTION_PPE,
		UPLL_IDX_RED_PRIORITY_PPE,
		UPLL_IDX_RED_DSCP_PPE,
		UPLL_IDX_RED_DROP_PPE
	};

	public enum ValPolicingMapIndex {
		UPLL_IDX_POLICERNAME_PM
	};

	public enum ValPolicingMapControllerStIndex {
		UPLL_IDX_SEQ_NUM_PMCS,
		UPLL_IDX_TOTAL_PMCS,
		UPLL_IDX_GREEN_YELLOW_PMCS,
		UPLL_IDX_RED_PMCS,
	};

	public enum ValPolicingMapSwitchStIndex {
		UPLL_IDX_POLICER_ID_PMSS,
		UPLL_IDX_SWITCH_ID_PMSS,
		UPLL_IDX_STATUS_PMSS,
		UPLL_IDX_VBR_NAME_PMSS,
		UPLL_IDX_IF_NAME_PMSS,
		UPLL_IDX_PORT_NAME_PMSS,
		UPLL_IDX_VLAN_ID_PMSS,
		UPLL_IDX_TOTAL_PMSS,
		UPLL_IDX_GREEN_YELLOW_PMSS,
		UPLL_IDX_RED_PMSS
	};

	public enum ValPolicingMapControllerIndex {
		UPLL_IDX_SEQ_NUM_PMC
	};

	public enum ValVtnPathMapEntryIndex {
		UPLL_IDX_FLOWLIST_NAME_VPME
	};

	public enum ValVtnPathmapPpolicyEntryIndex {
		UPLL_IDX_POLICY_ID_VPMPPE, UPLL_IDX_AGING_TIME_OUT_VPMPPE
	};

	public enum ValVtnPathmapPpolicyEntryStat {
		UPLL_IDX_POLICY_TYPE_VVPMPPES, UPLL_IDX_PMAP_HWSTATS_VVPMPPES
	}

	public enum PomPmapStatsIndex {
		UPLL_IDX_SOFT_PPSI,
		UPLL_IDX_EXIST_FLOW_PPSI,
		UPLL_IDX_EXPIRED_FLOW_PPSI,
		UPLL_IDX_TOTAL_PPSI
	}

	public enum ValVtnPathmapPpolicyFlowEntryStatIndex {
		UPLL_IDX_SEQ_NUM_PMPPFES,
		UPLL_IDX_FL_STATS_PMPPFES,
		UPLL_IDX_MAC_DST_PMPPFES,
		UPLL_IDX_MAC_SRC_PMPPFES,
		UPLL_IDX_MAC_ETH_TYPE_PMPPFES,
		UPLL_IDX_DST_IP_PMPPFES,
		UPLL_IDX_DST_IP_PREFIX_PMPPFES,
		UPLL_IDX_SRC_IP_PMPPFES,
		UPLL_IDX_SRC_IP_PREFIX_PMPPFES,
		UPLL_IDX_VLAN_PRIORITY_PMPPFES,
		UPLL_IDX_DST_IP_V6_PMPPFES,
		UPLL_IDX_DST_IP_V6_PREFIX_PMPPFES,
		UPLL_IDX_SRC_IP_V6_PMPPFES,
		UPLL_IDX_SRC_IP_V6_PREFIX_PMPPFES,
		UPLL_IDX_IP_PROTOCOL_PMPPFES,
		UPLL_IDX_IP_DSCP_PMPPFES,
		UPLL_IDX_L4_DST_PORT_PMPPFES,
		UPLL_IDX_L4_DST_PORT_ENDPT_PMPPFES,
		UPLL_IDX_L4_SRC_PORT_PMPPFES,
		UPLL_IDX_L4_SRC_PORT_ENDPT_PMPPFES,
		UPLL_IDX_ICMP_TYPE_PMPPFES,
		UPLL_IDX_ICMP_CODE_PMPPFES,
		UPLL_IDX_ICMP_V6_TYPE_PMPPFES,
		UPLL_IDX_ICMP_V6_CODE_PMPPFES
	}
	
	public enum valPmapPpolicyEntryFlWeightStIndex {
		UPLL_IDX_SWITCH_ID_PMPPFLW, UPLL_IDX_PORT_ID_PMPPFLW, UPLL_IDX_LINK_WEIGHT_PMPPFLW
	}

	public enum ValVbrExpandIndex {
		UPLL_IDX_VBRIDGE_NAME_VBRE,
		UPLL_IDX_CONTROLLER_ID_VBRE,
		UPLL_IDX_DOMAIN_ID_VBRE,
		UPLL_IDX_LABEL_VBRE,
		UPLL_IDX_CONTROLLER_VTN_NAME_VBRE,
		UPLL_IDX_CONTROLLER_VTN_LABEL_VBRE
	};

	public enum ValVbrPortmapExpandIndex {
		UPLL_IDX_PORTMAP_ID_VBRPME,
		UPLL_IDX_LOGICAL_PORT_ID_VBRPME,
		UPLL_IDX_LABEL_TYPE_VBRPME,
		UPLL_IDX_LABEL_VBRPME
	};

	public enum ValVbrIfExpandIndex {
		UPLL_IDX_IF_NAME_VBRIE,
		UPLL_IDX_CONN_VNODE_NAME_VBRIE,
		UPLL_IDX_CONN_VNODE_IF_NAME_VBRIE,
		UPLL_IDX_CONN_VLINK_NAME_VBRIE
	};

	public enum ValVtunnelExpandIndex {
		UPLL_IDX_VTUNNEL_NAME_VTNLE,
		UPLL_IDX_CONTROLLER_ID_VTNLE,
		UPLL_IDX_DOMAIN_ID_VTNLE,
		UPLL_IDX_LABEL_VTNLE
	};

	public enum ValVtunnelIfExpandIndex {
		UPLL_IDX_IF_NAME_VTNLIE,
		UPLL_IDX_CONN_VNODE_NAME_VTNLIE,
		UPLL_IDX_CONN_VNODE_IF_NAME_VTNLIE,
		UPLL_IDX_CONN_VLINK_NAME_VTNLIE
	};

	public enum ValVlinkExpandIndex {
		UPLL_IDX_VLINK_NAME_VLNKE,
		UPLL_IDX_VNODE1_NAME_VLNKE,
		UPLL_IDX_VNODE1_IF_NAME_VLNKE,
		UPLL_IDX_VNODE2_NAME_VLNKE,
		UPLL_IDX_VNODE2_IF_NAME_VLNKE,
		UPLL_IDX_BOUNDARY_NAME_VLNKE,
		UPLL_IDX_LABEL_TYPE_VLINKE,
		UPLL_IDX_LABEL_VLINKE
	};

	public enum ValUnwLabelIndex {
		UPLL_IDX_MAX_COUNT_UNWL,
		UPLL_IDX_RAISING_THRESHOLD_UNWL,
		UPLL_IDX_FALLING_THRESHOLD_UNWL
	};

	public enum ValVbrPortmapIndex {
		UPLL_IDX_CONTROLLER_ID_VBRPM,
		UPLL_IDX_DOMAIN_ID_VBRPM,
		UPLL_IDX_LOGICAL_PORT_ID_VBRPM,
		UPLL_IDX_LABEL_TYPE_VBRPM,
		UPLL_IDX_LABEL_VBRPM
	};
	
	public enum ValUnifiedNwIndex {
		UPLL_IDX_ROUTING_TYPE_UNW, 
		UPLL_IDX_IS_DEFAULT_UNW
	};

	public enum ValVtnUnifiedIndex {
		  UPLL_IDX_SPINE_ID_VUNW
	};
	
	public enum ValUnwSpineDomainIndex {
		  UPLL_IDX_SPINE_CONTROLLER_ID_UNWS,
		  UPLL_IDX_SPINE_DOMAIN_ID_UNWS,
		  UPLL_IDX_UNW_LABEL_ID_UNWS
	};

	public enum ValUnwSpineDomainStIndex {
		  UPLL_IDX_MAX_COUNT_UNWS_ST,
		  UPLL_IDX_USED_COUNT_UNWS_ST,
		  UPLL_IDX_ALARM_STATUS_UNWS_ST
	};

	public enum ValUnwSpineDomainAssignedLabelIndex {
		  UPLL_IDX_LABEL_UNWSAL,
		  UPLL_IDX_VTN_ID_UNWSAL,
		  UPLL_IDX_VNODE_ID_UNWSAL
	};

	public enum ValUnwSpineDomainFdbEntryIndex {
		UPLL_IDX_MAX_COUNT_UNWSDF,
		UPLL_IDX_MAX_SWITCH_ID_UNWSDF,
		UPLL_IDX_MIN_COUNT_UNWSDF,
		UPLL_IDX_MIN_SWITCH_ID_UNWSDF,
		UPLL_IDX_AVG_COUNT_UNWSDF,
		UPLL_IDX_NO_OF_SWITCHES_UNWSDF,
		UPLL_IDX_VTN_COUNT_UNWSDF
	};

	public enum ValUnwSpineDomainFdbEntryVtnIndex {
		UPLL_IDX_VTN_ID_UNWSDFV,
		UPLL_IDX_VLAN_ID_UNWSDFV,
		UPLL_IDX_MAX_COUNT_UNWSDFV,
		UPLL_IDX_MAX_SWITCH_ID_UNWSDFV,
		UPLL_IDX_MIN_COUNT_UNWSDFV,
		UPLL_IDX_MIN_SWITCH_ID_UNWSDFV,
		UPLL_IDX_AVG_COUNT_UNWSDFV
	};


}
