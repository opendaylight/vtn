/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
package org.opendaylight.vtn.javaapi.ipc.enums;

public class UncPhysicalStructIndexEnum {
	public enum UpplValDomainStIndex {
		kIdxDomainStDomain, kIdxDomainStOperStatus
	};

	public enum UpplValDomainIndex {
		kIdxDomainType, kIdxDomainDescription
	};

	public enum UpplDomainType {
		UPPL_DOMAIN_TYPE_DEFAULT("0"), UPPL_DOMAIN_TYPE_NORMAL("1"),
		UPPL_DOMAIN_TYPE_PF_LEAF("2"), UPPL_DOMAIN_TYPE_PF_SPINE("3");
		private final String value;

		private UpplDomainType(final String value) {
			this.value = value;
		}

		public String getValue() {
			return value;
		}
	};

	public enum UpplValSwitchStIndex {
		kIdxSwitch,
		kIdxSwitchOperStatus,
		kIdxSwitchManufacturer,
		kIdxSwitchHardware,
		kIdxSwitchSoftware,
		kIdxSwitchAlarmStatus
	};

	public enum UpplValSwitchIndex {
		kIdxSwitchDescription,
		kIdxSwitchModel,
		kIdxSwitchIPAddress,
		kIdxSwitchIPV6Address,
		kIdxSwitchAdminStatus,
		kIdxSwitchDomainName
	};

	public enum UpplSwitchAdminStatus {
		UPPL_SWITCH_ADMIN_UP("0"), UPPL_SWITCH_ADMIN_DOWN("1");
		private final String value;

		private UpplSwitchAdminStatus(final String value) {
			this.value = value;
		}

		public String getValue() {
			return value;
		}
	};

	public enum UpplPortAdminStatus {
		UPPL_PORT_ADMIN_UP("0"), UPPL_PORT_ADMIN_DOWN("1");

		private final String value;

		private UpplPortAdminStatus(final String value) {
			this.value = value;
		}

		public String getValue() {
			return value;
		}
	}

	public enum UpplPortOperStatus {

		UPPL_PORT_OPER_DOWN("0"),
		UPPL_PORT_OPER_UP("1"),
		UPPL_PORT_OPER_UNKNOWN("2");
		private final String value;

		private UpplPortOperStatus(final String value) {
			this.value = value;
		}

		public String getValue() {
			return value;
		}
	}

	public enum UpplValPortStIndex {
		kIdxPortSt,
		kIdxPortOperStatus,
		kIdxPortMacAddress,
		kIdxPortDirection,
		kIdxPortDuplex,
		kIdxPortSpeed,
		kIdxPortAlarmsStatus,
		kIdxPortLogicalPortId;
	}

	public enum UpplValPortStatsIndex {
		kIdxPortStatSt,
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
	};

	public enum UpplValPortNeighborIndex {
		kIdxPort, kIdxPortConnectedSwitchId, kIdxPortConnectedPortId, kIdxPortConnectedControllerId;

	}

	public enum UpplValPortIndex {
		kIdxPortNumber,
		kIdxPortDescription,
		kIdxPortAdminStatus,
		kIdxPortTrunkAllowedVlan;
	};

	public enum UpplPortDuplex {
		UPPL_PORT_DUPLEX_HALF(0), UPPL_PORT_DUPLEX_FULL(1);

		private final int value;

		private UpplPortDuplex(final int value) {
			this.value = value;
		}

		public int getValue() {
			return value;
		}
	}

	public enum UpplPortDirection {
		UPPL_PORT_DIR_INTERNEL(0),
		UPPL_PORT_DIR_EXTERNAL(1),
		UPPL_PORT_DIR_UNKNOWN(2);

		private final int value;

		private UpplPortDirection(final int value) {
			this.value = value;
		}

		public int getValue() {
			return value;
		}
	}

	public enum UpplSwitchPortAlarmsStatus {
		UPPL_ALARMS_DEFAULT_FLOW(0),
		UPPL_ALARMS_PORT_DIRECTION(1),
		UPPL_ALARMS_PORT_CONGES(2);

		private final int value;

		private UpplSwitchPortAlarmsStatus(final int value) {
			this.value = value;
		}

		public int getValue() {
			return value;
		}
	}

	public enum UpplSwitchOperStatus {
		UPPL_SWITCH_OPER_DOWN("0"),
		UPPL_SWITCH_OPER_UP("1"),
		UPPL_SWITCH_OPER_UNKNOWN("2");

		private final String value;

		private UpplSwitchOperStatus(final String value) {
			this.value = value;
		}

		public String getValue() {
			return value;
		}
	};

	public enum UpplValCtrIndex {
		kIdxType,
		kIdxVersion,
		kIdxDescription,
		kIdxIpAddress,
		kIdxUser,
		kIdxPassword,
		kIdxEnableAudit,
		kIdxPort
	};

	// Not mapped with updated header file- Poorvi

	public enum UpplTypeIndex {
		UNC_CT_UNKNOWN("0"), UNC_CT_PFC("1"), UNC_CT_VNP("2"), UNC_CT_POLC("3"),
		UNC_CT_HPVANC("4"), UNC_CT_ODC("5");

		private final String value;

		private UpplTypeIndex(final String value) {
			this.value = value;
		}

		public String getValue() {
			return value;
		}
	};

	public enum UpplValBoundaryIndex {
		kIdxBoundaryDescription,
		kIdxBoundaryControllerName1,
		kIdxBoundaryDomainName1,
		kIdxBoundaryLogicalPortId1,
		kIdxBoundaryControllerName2,
		kIdxBoundaryDomainName2,
		kIdxBoundaryLogicalPortId2
	}

	public enum UpplValBoundaryStIndex {
		kIdxBoundaryStBoundary, kIdxBoundaryStOperStatus

	};

	public enum UpplBoundaryOperStatus {
		UPPL_BOUNDARY_OPER_DOWN("0"),
		UPPL_BOUNDARY_OPER_UP("1"),
		UPPL_BOUNDARY_OPER_UNKNOWN("2");

		private final String value;

		private UpplBoundaryOperStatus(final String value) {
			this.value = value;
		}

		public String getValue() {
			return value;
		}
	};

	public enum UpplDomainOperStatus {
		UPPL_SWITCH_OPER_DOWN("0"),
		UPPL_SWITCH_OPER_UP("1"),
		UPPL_SWITCH_OPER_UNKNOWN("2");

		private final String value;

		private UpplDomainOperStatus(final String value) {
			this.value = value;
		}

		public String getValue() {
			return value;
		}
	};

	public enum UpplControllerAuditStatus {
		UPPL_AUTO_AUDIT_DISABLED("0"), UPPL_AUTO_AUDIT_ENABLED("1");

		private final String value;

		private UpplControllerAuditStatus(final String value) {
			this.value = value;
		}

		public String getValue() {
			return value;
		}
	};

	// Controller Response
	public enum UpplValCtrStIndex {
		kIdxController, kIdxActualVersion, kIdxOperStatus
	};

	public enum UpplValLinkIndex {
		kIdxLinkDescription
	};

	public enum UpplValLinkStIndex {
		kIdxLinkStLink, kIdxLinkStOperStatus,
	};

	/*
	 * public enum upplvallogicalportindex{ kIdxLogicalPortDescription ,
	 * kIdxLogicalPortType, kIdxLogicalPortSwitchId,
	 * kIdxLogicalPortPhysicalPortId, kIdxLogicalPortOperDownCriteria };
	 */

	public enum UpplValLogicalPortStIndex {
		kIdxLogicalPortSt, kIdxLogicalPortStOperStatus
	};

	public enum UpplValLogicalPortIndex {
		kIdxLogicalPortDescription,
		kIdxLogicalPortType,
		kIdxLogicalPortSwitchId,
		kIdxLogicalPortPhysicalPortId,
		kIdxLogicalPortOperDownCriteria
	};

	public enum UpplLogicalPortType {
		UPPL_LP_SWITCH("1"), UPPL_LP_PHYSICAL_PORT("2"), UPPL_LP_TRUNK_PORT(
				"11"), UPPL_LP_SUBDOMAIN("12"), UPPL_LP_TUNNEL_ENDPOINT("13"),
				UPPL_LP_PORT_GROUP("14"), UPPL_LP_MAPPING_GROUP("15");
		private final String value;

		private UpplLogicalPortType(final String value) {
			this.value = value;
		}

		public String getValue() {
			return value;
		}
	};

	public enum UpplLogicalPortOperDownCriteria {
		UPPL_OPER_DOWN_CRITERIA_ANY("0"), UPPL_OPER_DOWN_CRITERIA_ALL("1");
		private final String value;

		private UpplLogicalPortOperDownCriteria(final String value) {
			this.value = value;
		}

		public String getValue() {
			return value;
		}
	};

	public enum UpplLogicalPortOperStatus {
		UPPL_LOGICAL_PORT_OPER_DOWN("0"),
		UPPL_LOGICAL_PORT_OPER_UP("1"),
		UPPL_LOGICAL_PORT_OPER_UNKNOWN("2");
		private final String value;

		private UpplLogicalPortOperStatus(final String value) {
			this.value = value;
		}

		public String getValue() {
			return value;
		}
	};

	// All DATAFLOW Enums
	public enum UncValDfDataflowIndex {
		kidxDfDataFlowReason, kidxDfDataFlowControllerCount;
	};

	public enum UncValDfDataflowCmnIndex {
		kidxDfDataFlowControllerName,
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
	};

	public enum UncDataflowStatus {
		UNC_DF_STAT_INIT,
		UNC_DF_STAT_ACTIVATING,
		UNC_DF_STAT_ACTIVE,
		UNC_DF_STAT_SWITCHING
	};

	public enum UncDataflowFlowType {
		UNC_DF_TYPE_VTN
	};

	public enum UncValDfDataflowPathInfoIndex {
		kidxDfDataFlowPathInfoSwitchId,
		kidxDfDataFlowPathInfoInPort,
		kidxDfDataFlowPathInfoOutPort;
	}

	public enum UncValDfDataflowStIndex {
		kidxDfDataFlowStPackets,
		kidxDfDataFlowStOctets,
		kidxDfDataFlowStDuration;

	};

	public enum upplVaPortStatsIndex {
		kIdxPortStatSt,
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

	};

	public enum UpplValSwitchStDetailIndex {
		kIdxSwitchSt, kIdxSwitchStatFlowCount
	};
	
	public enum UpplValCtrPpolicyLinkWeightIndex{
		kIdxLinkWeight
	}

	public enum UpplValLmPortNeighborIndex {
		kIdxLmPort,
		kIdxLmPortConnectedSwitchId,
		kIdxLmPortConnectedPortId,
		kIdxLmPortConnectedControllerId
	};

	public enum UpplValLogicalPortBoundaryStIndex {
		kIdxLogicalPortBSt,
		kIdxLogicalPortBoundaryCandidate,
		kIdxLogicalPortBoundaryConnectedController,
		kIdxLogicalPortBoundaryConnectedDomain
	};

	public enum UpplLogicalPortBoundaryCandidate {
		UPPL_LP_BDRY_CANDIDATE_NO("0"), UPPL_LP_BDRY_CANDIDATE_YES("1");
		private final String value;

		private UpplLogicalPortBoundaryCandidate(final String value) {
			this.value = value;
		}

		public String getValue() {
			return value;
		}
	};

}
