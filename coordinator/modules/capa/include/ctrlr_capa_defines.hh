/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef CTRLR_CAPA_DEFINES_HH_
#define CTRLR_CAPA_DEFINES_HH_

#include "unc/keytype.h"

namespace unc {
namespace capa {

struct KtAttrMap {
  const char *attr_name;
  uint32_t attr_index;
};

struct KtMap {
  const char *kt_name;
  unc_key_type_t keytype;
  uint32_t attr_count;
  struct KtAttrMap *attr_map;
};

extern KtMap kt_map[];

namespace vtn {
enum vtn {
  kCapDesc = 0,
  kCapOperStatus,
  kCapAlarmStatus,
  kCapCreationTime,
  kCapLastUpdateTime
};
}  // namespace vtn

namespace vtn_neighbor {
enum vtn_neighbor {
  kCapConnectedVnodeName,
  kCapConnectedIfName,
  kCapConnectedVlinkName
};
} // namespace vtn_neighbor

namespace vbr {
enum vbr {
  kCapDesc = 0,
  kCapHostAddr,
  kCapHostAddrPrefixlen,
  kCapOperStatus
};
}  // namespace vbr

namespace vlan_map {
enum vlan_map {
  kCapVlanId = 0
};
}  // namespace vlan_map

namespace vbr_if {
enum vbr_if {
  kCapDesc = 0,
  kCapAdminStatus,
  kCapLogicalPortId,
  kCapVlanId,
  kCapTagged,
  kCapOperStatus
};
}  // namespace vbr_if


namespace vrt {
enum vrt {
  kCapDesc = 0,
  kCapDhcpRelayAdminStatus,
  kCapOperStatus
};
}  // namespace vrt

namespace dhcp_relay_server {
enum dhcp_relay_server {
 //   kCapServerAddr = 0
};
} //namespace dhcp_relay_server

namespace dhcp_relay_if {
enum dhcp_relay_if {
};
}//namespace dhcp_relay_if

namespace static_ip_route {
enum static_ip_route {
  kCapGroupMetric = 0
};
}  // namespace static_ip_route

namespace vrt_if {
enum vrt_if {
  kCapDesc = 0,
  kCapIpAddr,
  kCapPrefixlen,
  kCapMacAddr,
  kCapAdminStatus,
  kCapOperStatus
};
}  // namespace vrt_if

namespace vunknown {
enum vunknown {
  kCapDesc = 0,
  kCapType
};
}  // namespace vunknown

namespace vunk_if {
enum vunk_if {
  kCapDesc = 0,
  kCapAdminStatus
};
}  // namespace vunk_if

namespace vtep {
enum vtep {
  kCapDesc = 0,
  kCapOperStatus
};
}  // namespace vtep

namespace vtep_if {
enum vtep_if {
  kCapDesc = 0,
  kCapAdminStatus,
  kCapLogicalPortId,
  kCapVlanId,
  kCapTagged,
  kCapIfOperStatus
};
}  // namespace vtep_if

namespace vtep_grp {
enum vtep_grp {
  kCapDesc = 0
};
}  // namespace vtep_grp

namespace vtunnel {
enum vtunnel {
  kCapDesc = 0,
  kCapVtepName,
  kCapVtepGrpName,
  kCapLabel,
  kCapOperStatus
};
}  // namespace vtunnel

namespace vtunnel_if {
enum vtunnel_if {
  kCapDesc = 0,
  kCapAdminStatus,
  kCapLogicalPortId,
  kCapVlanId,
  kCapTagged,
  kCapIfOperStatus
};
}  // namespace vtunnel_if

namespace vlink {
enum vlink {
  kCapDesc = 0,
  kCapAdminStatus,
  kCapVnode1Name,
  kCapVnode1IfName,
  kCapVnode2Name,
  kCapVnode2IfName,
  kCapBoundaryName,
  kCapVlanId,
  kCapOperStatus
};
}  // namespace vlink

namespace nwm {
enum nwm {
  kCapAdminStatus = 0,
  kCapStatus
};
}  // namespace nwm

namespace nwm_host {
enum nwm_host {
  kCapHealthInterval = 0,
  kCapRecoveryInterval,
  kCapFailureCount,
  kCapRecoveryCount,
  kCapWaitTime
};
}  // namespace nwm_host

// UNC_KT_FLOWLIST
namespace flowlist {
enum flowlist {
  kCapIpType = 0
};
}  // namespace flowlist

// UNC_KT_FLOWLIST_ENTRY,
namespace flowlist_entry {
enum flowlist_entry {
  kCapMacDst = 0,
  kCapMacSrc,
  kCapMacEthType,
  kCapDstIp,
  kCapDstIpPrefix,
  kCapSrcIp,
  kCapSrcIpPrefix,
  kCapVlanPriority,
  kCapDstIpV6,
  kCapDstIpV6Prefix,
  kCapSrcIpV6,
  kCapSrcIpV6Prefix,
  kCapIpProtocol,
  kCapIpDscp,
  kCapL4DstPort,
  kCapL4DstPortEndpt,
  kCapL4SrcPort,
  kCapL4SrcPortEndpt,
  kCapIcmpType,
  kCapIcmpCode,
  kCapIcmpV6Type,
  kCapIcmpV6Code
};
}  // namespace flowlist_entry

// UNC_KT_POLICING_PROFILE
namespace policingprofile {
enum policingprofile {
  kCapPolicerName = 0
};
}  // namespace policingprofile

// UNC_KT_POLICING_PROFILE_ENTRY
namespace policingprofile_entry {
enum policingprofile_entry {
  kCapFlowlist = 0,
  kCapRate,
  kCapCir,
  kCapCbs,
  kCapPir,
  kCapPbs,
  kCapGreenAction,
  kCapGreenPriority,
  kCapGreenDscp,
  kCapGreenDrop,
  kCapYellowAction,
  kCapYellowPriority,
  kCapYellowDscp,
  kCapYellowDrop,
  kCapRedAction,
  kCapRedPriority,
  kCapRedDscp,
  kCapRedDrop
};
}  // namespace policingprofile_entry

// UNC_KT_VTN_FLOWFILTER
namespace vtn_flowfilter {
enum vtn_flowfilter {
};
}  // namespace vtn_flowfilter

// UNC_KT_VTN_FLOWFILTER_ENTRY
namespace vtn_flowfilter_entry {
enum vtn_flowfilter_entry {
  kCapFlowlistName = 0,
  kCapAction,
  kCapNwnName,
  kCapDscp,
  kCapPriority
};
}  // namespace vtn_flowfilter_entry

namespace vtn_flowfilter_controller {
enum  vtn_flowfilter_controller {
  kCapDirection=0,
  kCapSeqNum
};
}  // namesapce vtn_flowfilter_controller

// UNC_KT_VBR_POLICINGMAP
namespace vtn_policingmap {
enum vtn_policingmap {
  kCapPolicername = 0
};
}  // namespace vbr_policingmap

// UNC_KT_VTN_POLICINGMAP_CONTROLLER
namespace vtn_policingmap_controller {
enum vtn_policingmap_controller {
  kCapSeqNum = 0
};
}  // namespace vtn_policingmap_controller

// UNC_KT_VBR_FLOWFILTER
namespace vbr_flowfilter {
enum vbr_flowfilter {
};
}  // namespace vbr_flowfilter


// UNC_KT_VBR_FLOWFILTER_ENTRY
namespace vbr_flowfilter_entry {
enum vbr_flowfilter_entry {
  kCapFlowlistName = 0,
  kCapAction,
  kCapRedirectNode,
  kCapRedirectPort,
  kCapModifyDstMac,
  kCapModifySrcMac,
  kCapNwmName,
  kCapDscp,
  kCapPriority
};
}  // namespace vbridge_flowfilter_entry

// UNC_KT_VBR_POLICINGMAP
namespace vbr_policingmap {
enum vbr_policingmap {
  kCapPolicername = 0
};
}  // namespace vbr_policingmap

// UNC_KT_VBR_POLICINGMAP_ENTRY
namespace vbr_policingmap_entry {
enum vbr_policingmap_entry {
};
}  // namespace vbr_policingmap_entry


// UNC_KT_VBRIF_FLOWFILTER
namespace vbr_if_flowfilter {
enum vbr_if_flowfilter {
};
}  // namespace vbrif_flowfilter


// UNC_KT_VBRIF_FLOWFILTER_ENTRY
namespace vbr_if_flowfilter_entry {
enum vbr_if_flowfilter_entry {
  kCapFlowlistName = 0,
  kCapAction,
  kCapRedirectNode,
  kCapRedirectPort,
  kCapModifyDstMac,
  kCapModifySrcMac,
  kCapNwmName,
  kCapDscp,
  kCapPriority
};
}  // namespace vbridge_if_flowfilter_entry

// UNC_KT_VBRIF_POLICINGMAP
namespace vbr_if_policingmap {
enum vbr_if_policingmap {
  kCapPolicername = 0
};
}  // namespace vbrif_policingmap


// UNC_KT_VBRIF_POLICINGMAP_ENTRY
namespace vbr_if_policingmap_entry {
enum vbr_if_policingmap_entry {
};
}  // namespace vbrif_policingmap_entry


// UNC_KT_VRTIF_FLOWFILTER
namespace vrt_if_flowfilter {
enum vrt_if_flowfilter {
};
}  // namespace vrt_if_flowfilter

// UNC_KT_VRTIF_FLOWFILTER_ENTRY
namespace vrt_if_flowfilter_entry {
enum vrt_if_flowfilter_entry {
  kCapFlowlistName = 0,
  kCapAction,
  kCapRedirectNode,
  kCapRedirectPort,
  kCapModifyDstMac,
  kCapModifySrcMac,
  kCapNwmName,
  kCapDscp,
  kCapPriority
};
}  // namespace vrouter_if_flowfilter_entry

}  // namespace capa
}  // namespace unc

#endif  // CTRLR_CAPA_DEFINES_HH_
