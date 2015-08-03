/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include "unc/keytype.h"
#include "ctrlr_capa_defines.hh"

namespace unc {
namespace capa {

KtAttrMap kam_vtn[] = {
  { "description", vtn::kCapDesc },
  { "operstatus", vtn::kCapOperStatus },
  { "alarm_status", vtn::kCapAlarmStatus },
  { "creation_time", vtn::kCapCreationTime },
  { "last_update_time", vtn::kCapLastUpdateTime }
};

/*
KtAttrMap kam_vtn_neighbor[] = {
  { "connected_vnode_name",vtn_neighbor::kCapConnectedVnodeName },
  { "connected_if_name",vtn_neighbor::kCapConnectedIfName },
  { "connected_vlink_name",vtn_neighbor::kCapConnectedVlinkName }
};
*/

KtAttrMap kam_vbridge[] = {
  { "domain_id", vbr::kCapDomainId },
  { "description", vbr::kCapDesc },
  { "host_address", vbr::kCapHostAddr },
  { "host_address_prefixlen", vbr::kCapHostAddrPrefixlen },
  { "operstatus", vbr::kCapOperStatus }
};

KtAttrMap kam_vbr_portmap[] = {
  { "domain_id", vbr_portmap::kCapDomainId },
  { "logical_port_id", vbr_portmap::kCapLogicalPortId },
  { "label_type", vbr_portmap::kCapLabelType },
  { "label", vbr_portmap::kCapLabel },
  { "operstatus", vbr_portmap::kCapOperStatus }
};

KtAttrMap kam_vlan_map[] = {
  { "vlan_id", vlan_map::kCapVlanId }
};

KtAttrMap kam_vbr_if[] = {
  { "description", vbr_if::kCapDesc },
  { "admin_status", vbr_if::kCapAdminStatus },
  { "logical_port_id", vbr_if::kCapLogicalPortId },
  { "vlan_id", vbr_if::kCapVlanId },
  { "tagged", vbr_if::kCapTagged },
  { "operstatus", vbr_if::kCapOperStatus }
};

KtAttrMap kam_vrouter[] = {
  { "domain_id", vrt::kCapDomainId },
  { "description", vrt::kCapDesc },
  { "dhcp_relay_admin_status", vrt::kCapDhcpRelayAdminStatus },
  { "operstatus", vrt::kCapOperStatus }
};

KtAttrMap kam_dhcp_relay_server[] = {
//  {"server_address",dhcp_relay_server::kCapServerAddr}
};

KtAttrMap kam_dhcp_relay_interface[] = {

};

KtAttrMap kam_static_ip_route[] = {
  { "nwm_name", static_ip_route::kCapNwmName },
  { "group_metric", static_ip_route::kCapGroupMetric }
};

KtAttrMap kam_vrouter_interface[] = {
  { "description", vrt_if::kCapDesc },
  { "ip_address", vrt_if::kCapIpAddr },
  { "prefixlen", vrt_if::kCapPrefixlen },
  { "mac_address", vrt_if::kCapMacAddr },
  { "admin_status", vrt_if::kCapAdminStatus },
  { "operstatus", vrt_if::kCapOperStatus }
};

KtAttrMap kam_vterminal[] = {
  { "domain_id", vterminal::kCapDomainId },
  { "description", vterminal::kCapDesc },
  { "operstatus", vterminal::kCapOperStatus }
};

KtAttrMap kam_vterm_if[] = {
  { "description", vterm_if::kCapDesc },
  { "admin_status", vterm_if::kCapAdminStatus },
  { "logical_port_id", vterm_if::kCapLogicalPortId },
  { "vlan_id", vterm_if::kCapVlanId },
  { "tagged", vterm_if::kCapTagged },
  { "operstatus", vterm_if::kCapOperStatus }
};
#if 0
// Unknown does not have controller configuration.
KtAttrMap kam_vunknown[] = {
  { "description", vunknown::kCapDesc },
  { "type", vunknown::kCapType }
};

KtAttrMap kam_vunk_if[] = {
  { "description", vunk_if::kCapDesc }
};
#endif

KtAttrMap kam_vtep[] = {
  { "description", vtep::kCapDesc },
  { "operstatus", vtep::kCapOperStatus }
};

KtAttrMap kam_vtep_if[] = {
  { "description", vtep_if::kCapDesc },
  { "admin_status", vtep_if::kCapAdminStatus },
  { "logical_port_id", vtep_if::kCapLogicalPortId },
  { "vlan_id", vtep_if::kCapVlanId },
  { "tagged", vtep_if::kCapTagged },
  { "operstatus", vtep_if::kCapIfOperStatus }
};

KtAttrMap kam_vtep_grp[] = {
  { "description", vtep_grp::kCapDesc },
};

KtAttrMap kam_vtep_grp_member[] = {
};

KtAttrMap kam_vtunnel[] = {
  { "description", vtunnel::kCapDesc },
  { "vtep_name", vtunnel::kCapVtepName },
  { "vtep_group_name", vtunnel::kCapVtepGrpName },
  { "label", vtunnel::kCapLabel },
  { "operstatus", vtunnel::kCapOperStatus }
};

KtAttrMap kam_vtunnel_if[] = {
  { "description", vtunnel_if::kCapDesc },
  { "admin_status", vtunnel_if::kCapAdminStatus },
  { "logical_port_id", vtunnel_if::kCapLogicalPortId },
  { "vlan_id", vtunnel_if::kCapVlanId },
  { "tagged", vtunnel_if::kCapTagged },
  { "operstatus", vtunnel_if::kCapIfOperStatus }
};

KtAttrMap kam_vlink[] = {
  { "description", vlink::kCapDesc },
  { "admin_status", vlink::kCapAdminStatus },
  { "vnode1_name", vlink::kCapVnode1Name },
  { "vnode1_interface_name", vlink::kCapVnode1IfName },
  { "vnode2_name", vlink::kCapVnode2Name },
  { "vnode2_interface_name", vlink::kCapVnode2IfName },
  { "boundary_name", vlink::kCapBoundaryName },
  { "label_type", vlink::kCapLabelType },
  { "label", vlink::kCapLabel },
  { "operstatus", vlink::kCapOperStatus }
};

KtAttrMap kam_nwm[] = {
  { "admin_status", nwm::kCapAdminStatus },
  { "status", nwm::kCapStatus }
};

KtAttrMap kam_nwm_host[] = {
  { "health_interval", nwm_host::kCapHealthInterval },
  { "recovery_interval", nwm_host::kCapRecoveryInterval },
  { "failure_count", nwm_host::kCapFailureCount },
  { "recovery_count", nwm_host::kCapRecoveryCount },
  { "wait_time", nwm_host::kCapWaitTime }
};

KtAttrMap kam_flow_list[] = {
  { "ip_type", flowlist::kCapIpType }
};

KtAttrMap kam_flow_list_entry[] = {
  { "mac_destination", flowlist_entry::kCapMacDst },
  { "mac_source", flowlist_entry::kCapMacSrc },
  { "mac_eth_type", flowlist_entry::kCapMacEthType },
  { "destination_ip", flowlist_entry::kCapDstIp },
  { "destination_ip_prefix", flowlist_entry::kCapDstIpPrefix },
  { "source_ip", flowlist_entry::kCapSrcIp },
  { "source_ip_prefix", flowlist_entry::kCapSrcIpPrefix },
  { "vlan_priority", flowlist_entry::kCapVlanPriority },
  { "destination_ip_v6", flowlist_entry::kCapDstIpV6 },
  { "destination_ip_v6_prefix", flowlist_entry::kCapDstIpV6Prefix },
  { "source_ip_v6", flowlist_entry::kCapSrcIpV6 },
  { "source_ip_v6_prefix", flowlist_entry::kCapSrcIpV6Prefix },
  { "ip_protocol", flowlist_entry::kCapIpProtocol },
  { "ip_dscp", flowlist_entry::kCapIpDscp },
  { "l4_destination_port", flowlist_entry::kCapL4DstPort },
  { "l4_destination_port_endpt", flowlist_entry::kCapL4DstPortEndpt },
  { "l4_source_port", flowlist_entry::kCapL4SrcPort },
  { "l4_source_port_endpt", flowlist_entry::kCapL4SrcPortEndpt },
  { "icmp_type", flowlist_entry::kCapIcmpType },
  { "icmp_code", flowlist_entry::kCapIcmpCode },
  { "icmp_v6_type", flowlist_entry::kCapIcmpV6Type },
  { "icmp_v6_code", flowlist_entry::kCapIcmpV6Code }
};

KtAttrMap kam_policing_profile[] = {
  { "policer_name", policingprofile::kCapPolicerName },
};

KtAttrMap kam_policing_profile_entry[] = {
  { "flowlist", policingprofile_entry::kCapFlowlist },
  { "rate", policingprofile_entry::kCapRate },
  { "cir", policingprofile_entry::kCapCir },
  { "cbs", policingprofile_entry::kCapCbs },
  { "pir", policingprofile_entry::kCapPir },
  { "pbs", policingprofile_entry::kCapPbs },
  { "green_action", policingprofile_entry::kCapGreenAction },
  { "green_priority", policingprofile_entry::kCapGreenPriority },
  { "green_dscp", policingprofile_entry::kCapGreenDscp },
  { "green_drop", policingprofile_entry::kCapGreenDrop },
  { "yellow_action", policingprofile_entry::kCapYellowAction },
  { "yellow_priority", policingprofile_entry::kCapYellowPriority },
  { "yellow_dscp", policingprofile_entry::kCapYellowDscp },
  { "yellow_drop", policingprofile_entry::kCapYellowDrop },
  { "red_action", policingprofile_entry::kCapRedAction },
  { "red_priority", policingprofile_entry::kCapRedPriority },
  { "red_dscp", policingprofile_entry::kCapRedDscp },
  { "red_drop", policingprofile_entry::kCapRedDrop }
};

KtAttrMap kam_vtn_flow_filter[] = {
};

KtAttrMap kam_vtn_flow_filter_entry[] = {
  { "flowlist_name", vtn_flowfilter_entry::kCapFlowlistName },
  { "action", vtn_flowfilter_entry::kCapAction },
  { "network_monitor", vtn_flowfilter_entry::kCapNwnName },
  { "dscp", vtn_flowfilter_entry::kCapDscp },
  { "priority", vtn_flowfilter_entry::kCapPriority }
};

KtAttrMap kam_vtn_flow_filter_controller[] = {
  { "direction", vtn_flowfilter_controller::kCapDirection },
  { "sequence_number", vtn_flowfilter_controller::kCapSeqNum }
};

KtAttrMap kam_vtn_policing_map[] = {
  { "policer_name", vtn_policingmap::kCapPolicername }
};

KtAttrMap kam_vtn_policing_map_controller[] = {
  { "sequence_number", vtn_policingmap_controller::kCapSeqNum }
};

KtAttrMap kam_vbridge_flow_filter[] = {
};

KtAttrMap kam_vbridge_flow_filter_entry[] = {
  { "flowlist_name", vbr_flowfilter_entry::kCapFlowlistName },
  { "action", vbr_flowfilter_entry::kCapAction },
  { "redirect_node", vbr_flowfilter_entry::kCapRedirectNode },
  { "redirect_port", vbr_flowfilter_entry::kCapRedirectPort },
  { "redirect_direction", vbr_flowfilter_entry::kCapRedirectDirection },
  { "modify_destination_mac", vbr_flowfilter_entry::kCapModifyDstMac },
  { "modify_source_mac", vbr_flowfilter_entry::kCapModifySrcMac },
  { "network_monitor", vbr_flowfilter_entry::kCapNwmName },
  { "dscp", vbr_flowfilter_entry::kCapDscp },
  { "priority", vbr_flowfilter_entry::kCapPriority }
};

KtAttrMap kam_vbridge_policing_map[] = {
  { "policer_name", vbr_policingmap::kCapPolicername }
};

KtAttrMap kam_vbridge_policing_map_entry[] = {
};

KtAttrMap kam_vbridge_if_flow_filter[] = {
};

KtAttrMap kam_vbridge_if_flow_filter_entry[] = {
  { "flowlist_name", vbr_if_flowfilter_entry::kCapFlowlistName },
  { "action", vbr_if_flowfilter_entry::kCapAction },
  { "redirect_node", vbr_if_flowfilter_entry::kCapRedirectNode },
  { "redirect_port", vbr_if_flowfilter_entry::kCapRedirectPort },
  { "redirect_direction", vbr_if_flowfilter_entry::kCapRedirectDirection },
  { "modify_destination_mac", vbr_if_flowfilter_entry::kCapModifyDstMac },
  { "modify_source_mac", vbr_if_flowfilter_entry::kCapModifySrcMac },
  { "network_monitor", vbr_if_flowfilter_entry::kCapNwmName },
  { "dscp", vbr_if_flowfilter_entry::kCapDscp },
  { "priority", vbr_if_flowfilter_entry::kCapPriority }
};

KtAttrMap kam_vbridge_if_policing_map[] = {
  { "policer_name", vbr_if_policingmap::kCapPolicername }
};

KtAttrMap kam_vbridge_if_policing_map_entry[] = {
};

KtAttrMap kam_vrouter_if_flow_filter[] = {
};

KtAttrMap kam_vrouter_if_flow_filter_entry[] = {
  { "flowlist_name", vrt_if_flowfilter_entry::kCapFlowlistName },
  { "action", vrt_if_flowfilter_entry::kCapAction },
  { "redirect_node", vrt_if_flowfilter_entry::kCapRedirectNode },
  { "redirect_port", vrt_if_flowfilter_entry::kCapRedirectPort },
  { "redirect_direction", vrt_if_flowfilter_entry::kCapRedirectDirection },
  { "modify_destination_mac", vrt_if_flowfilter_entry::kCapModifyDstMac },
  { "modify_source_mac", vrt_if_flowfilter_entry::kCapModifySrcMac },
  { "network_monitor", vrt_if_flowfilter_entry::kCapNwmName },
  { "dscp", vrt_if_flowfilter_entry::kCapDscp },
  { "priority", vrt_if_flowfilter_entry::kCapPriority }
};

KtAttrMap kam_vterminal_if_flow_filter[] = {
};

KtAttrMap kam_vterminal_if_flow_filter_entry[] = {
  { "flowlist_name", vterm_if_flowfilter_entry::kCapFlowlistName },
  { "action", vterm_if_flowfilter_entry::kCapAction },
  { "redirect_node", vterm_if_flowfilter_entry::kCapRedirectNode },
  { "redirect_port", vterm_if_flowfilter_entry::kCapRedirectPort },
  { "redirect_direction", vterm_if_flowfilter_entry::kCapRedirectDirection },
  { "modify_destination_mac", vterm_if_flowfilter_entry::kCapModifyDstMac },
  { "modify_source_mac", vterm_if_flowfilter_entry::kCapModifySrcMac },
  { "network_monitor", vterm_if_flowfilter_entry::kCapNwmName },
  { "dscp", vterm_if_flowfilter_entry::kCapDscp },
  { "priority", vterm_if_flowfilter_entry::kCapPriority }
};

KtAttrMap kam_vterminal_if_policing_map[] = {
  { "policer_name", vterm_if_policingmap::kCapPolicername }
};

KtAttrMap kam_vterminal_if_policing_map_entry[] = {
};

//****************adding for physical module***********/

KtAttrMap kam_controller[] = {
  { "type", controller::kType},
  { "version", controller::kVersion},
  { "description", controller::kDescription},
  { "ip_address", controller::kIp_address},
  { "user", controller::kUser_name},
  { "password", controller::kPassword},
  { "enable_audit", controller::kEnableAudit},
  { "port", controller::kPort}
};
KtAttrMap kam_ctr_domain[] =  {

};


KtAttrMap kam_logical_port[] = {

};

KtAttrMap kam_switch[] =  {

};

KtAttrMap kam_port[] = {

};

KtAttrMap kam_link[] = {

};

KtAttrMap kam_boundary[] = {

};

KtAttrMap kam_ctr_data_flow[] = {

};
 
KtAttrMap kam_data_flow[] =  {

};

KtAttrMap kam_data_flow_v2[] =  {

};

KtAttrMap kam_vtn_dataflow[] = {
};
#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))
#endif

KtMap kt_map[] = {
  { "vtn", UNC_KT_VTN, ARRAY_SIZE(kam_vtn),
    &kam_vtn[0] },
  { "vbridge", UNC_KT_VBRIDGE, ARRAY_SIZE(kam_vbridge),
    &kam_vbridge[0] },
  { "vbr_portmap", UNC_KT_VBR_PORTMAP, ARRAY_SIZE(kam_vbr_portmap),
        &kam_vbr_portmap[0] },
  { "vlan_map", UNC_KT_VBR_VLANMAP, ARRAY_SIZE(kam_vlan_map),
    &kam_vlan_map[0] },
  { "vbridge_interface", UNC_KT_VBR_IF, ARRAY_SIZE(kam_vbr_if),
    &kam_vbr_if[0] },
  { "vrouter", UNC_KT_VROUTER, ARRAY_SIZE(kam_vrouter),
    &kam_vrouter[0] },
  { "dhcp_relay_server", UNC_KT_DHCPRELAY_SERVER, ARRAY_SIZE(kam_dhcp_relay_server),
    &kam_dhcp_relay_server[0] },
  { "dhcp_relay_interface", UNC_KT_DHCPRELAY_IF, ARRAY_SIZE(kam_dhcp_relay_interface),
    &kam_dhcp_relay_interface[0] },
  { "static_ip_route", UNC_KT_VRT_IPROUTE, ARRAY_SIZE(kam_static_ip_route),
    &kam_static_ip_route[0] },
  { "vrouter_interface", UNC_KT_VRT_IF, ARRAY_SIZE(kam_vrouter_interface),
    &kam_vrouter_interface[0] },
  { "vterminal", UNC_KT_VTERMINAL, ARRAY_SIZE(kam_vterminal),
    &kam_vterminal[0] },
  { "vterminal_interface", UNC_KT_VTERM_IF, ARRAY_SIZE(kam_vterm_if),
    &kam_vterm_if[0] },
#if 0
  { "vunknown", UNC_KT_VUNKNOWN, ARRAY_SIZE(kam_vunknown),
    &kam_vunknown[0] },
  { "unknown_if", UNC_KT_VUNK_IF, ARRAY_SIZE(kam_vunk_if),
    &kam_vunk_if[0] },
#endif
  { "vtep", UNC_KT_VTEP, ARRAY_SIZE(kam_vtep),
    &kam_vtep[0] },
  { "vtep_interface", UNC_KT_VTEP_IF, ARRAY_SIZE(kam_vtep_if),
    &kam_vtep_if[0] },
  { "vtep_group", UNC_KT_VTEP_GRP, ARRAY_SIZE(kam_vtep_grp),
    &kam_vtep_grp[0] },
  { "vtep_group_member", UNC_KT_VTEP_GRP_MEMBER,
    ARRAY_SIZE(kam_vtep_grp_member), &kam_vtep_grp_member[0] },
  { "vtunnel", UNC_KT_VTUNNEL, ARRAY_SIZE(kam_vtunnel),
    &kam_vtunnel[0] },
  { "vtunnel_interface", UNC_KT_VTUNNEL_IF, ARRAY_SIZE(kam_vtunnel_if),
    &kam_vtunnel_if[0] },
  { "vlink", UNC_KT_VLINK, ARRAY_SIZE(kam_vlink),
    &kam_vlink[0] },
  { "network_monitor_group", UNC_KT_VBR_NWMONITOR, ARRAY_SIZE(kam_nwm),
    &kam_nwm[0] },
  { "network_monitor_ip_host", UNC_KT_VBR_NWMONITOR_HOST, ARRAY_SIZE(kam_nwm_host),
    &kam_nwm_host[0] },

  { "flow_list", UNC_KT_FLOWLIST,
    ARRAY_SIZE(kam_flow_list), &kam_flow_list[0] },
  { "flow_list_entry", UNC_KT_FLOWLIST_ENTRY,
    ARRAY_SIZE(kam_flow_list_entry), &kam_flow_list_entry[0] },

  { "policing_profile", UNC_KT_POLICING_PROFILE,
    ARRAY_SIZE(kam_policing_profile), &kam_policing_profile[0] },
  { "policing_profile_entry", UNC_KT_POLICING_PROFILE_ENTRY,
    ARRAY_SIZE(kam_policing_profile_entry), &kam_policing_profile_entry[0] },

  { "vtn_flow_filter", UNC_KT_VTN_FLOWFILTER,
    ARRAY_SIZE(kam_vtn_flow_filter), &kam_vtn_flow_filter[0] },
  { "vtn_flow_filter_entry", UNC_KT_VTN_FLOWFILTER_ENTRY,
    ARRAY_SIZE(kam_vtn_flow_filter_entry), &kam_vtn_flow_filter_entry[0] },
  { "vtn_flow_filter_controller", UNC_KT_VTN_FLOWFILTER_CONTROLLER,
    ARRAY_SIZE(kam_vtn_flow_filter_controller),
    &kam_vtn_flow_filter_controller[0] },

  { "vtn_policing_map", UNC_KT_VTN_POLICINGMAP,
    ARRAY_SIZE(kam_vtn_policing_map), &kam_vtn_policing_map[0] },
  { "vtn_policing_map_controller", UNC_KT_VTN_POLICINGMAP_CONTROLLER,
    ARRAY_SIZE(kam_vtn_policing_map_controller),
    &kam_vtn_policing_map_controller[0] },


  { "vbridge_flow_filter", UNC_KT_VBR_FLOWFILTER,
    ARRAY_SIZE(kam_vbridge_flow_filter), &kam_vbridge_flow_filter[0] },
  { "vbridge_flow_filter_entry", UNC_KT_VBR_FLOWFILTER_ENTRY,
    ARRAY_SIZE(kam_vbridge_flow_filter_entry),
    &kam_vbridge_flow_filter_entry[0] },

  { "vbridge_policing_map", UNC_KT_VBR_POLICINGMAP,
    ARRAY_SIZE(kam_vbridge_policing_map), &kam_vbridge_policing_map[0] },
  { "vbridge_policing_map_entry", UNC_KT_VBR_POLICINGMAP_ENTRY,
    ARRAY_SIZE(kam_vbridge_policing_map_entry),
    &kam_vbridge_policing_map_entry[0] },

  { "vbridge_interface_flow_filter", UNC_KT_VBRIF_FLOWFILTER,
    ARRAY_SIZE(kam_vbridge_if_flow_filter), &kam_vbridge_if_flow_filter[0] },
  { "vbridge_interface_flow_filter_entry", UNC_KT_VBRIF_FLOWFILTER_ENTRY,
    ARRAY_SIZE(kam_vbridge_if_flow_filter_entry),
    &kam_vbridge_if_flow_filter_entry[0] },

  { "vbridge_interface_policing_map", UNC_KT_VBRIF_POLICINGMAP,
    ARRAY_SIZE(kam_vbridge_if_policing_map), &kam_vbridge_if_policing_map[0] },
  { "vbridge_interface_policing_map_entry", UNC_KT_VBRIF_POLICINGMAP_ENTRY,
    ARRAY_SIZE(kam_vbridge_if_policing_map_entry),
    &kam_vbridge_if_policing_map_entry[0] },

  { "vrouter_interface_flow_filter", UNC_KT_VRTIF_FLOWFILTER,
    ARRAY_SIZE(kam_vrouter_if_flow_filter), &kam_vrouter_if_flow_filter[0] },
  { "vrouter_interface_flow_filter_entry", UNC_KT_VRTIF_FLOWFILTER_ENTRY,
    ARRAY_SIZE(kam_vrouter_if_flow_filter_entry),
    &kam_vrouter_if_flow_filter_entry[0] },

  { "vterminal_interface_flow_filter", UNC_KT_VTERMIF_FLOWFILTER,
    ARRAY_SIZE(kam_vterminal_if_flow_filter), &kam_vterminal_if_flow_filter[0] },
  { "vterminal_interface_flow_filter_entry", UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
    ARRAY_SIZE(kam_vterminal_if_flow_filter_entry),
    &kam_vterminal_if_flow_filter_entry[0] },

 { "vterminal_interface_policing_map", UNC_KT_VTERMIF_POLICINGMAP,
    ARRAY_SIZE(kam_vterminal_if_policing_map), &kam_vterminal_if_policing_map[0] },
 { "vterminal_interface_policing_map_entry", UNC_KT_VTERMIF_POLICINGMAP_ENTRY,
    ARRAY_SIZE(kam_vterminal_if_policing_map_entry),
    &kam_vterminal_if_policing_map_entry[0] },

  { "controller", UNC_KT_CONTROLLER, ARRAY_SIZE(kam_controller),
    &kam_controller[0] },
  { "ctr_domain", UNC_KT_CTR_DOMAIN, ARRAY_SIZE(kam_ctr_domain),
    &kam_ctr_domain[0] },
  { "switch", UNC_KT_SWITCH, ARRAY_SIZE(kam_switch),
    &kam_switch[0] },
  { "port", UNC_KT_PORT, ARRAY_SIZE(kam_port),
    &kam_port[0] },
  { "link", UNC_KT_LINK, ARRAY_SIZE(kam_link),
    &kam_link[0] },
  { "boundary", UNC_KT_BOUNDARY, ARRAY_SIZE(kam_boundary),
    &kam_boundary[0] },
  { "dataflow", UNC_KT_DATAFLOW, ARRAY_SIZE(kam_data_flow),
    &kam_data_flow[0] },
  { "dataflow_v2", UNC_KT_DATAFLOW_V2, ARRAY_SIZE(kam_data_flow_v2),
    &kam_data_flow_v2[0] },
  { "ctr_dataflow", UNC_KT_CTR_DATAFLOW, ARRAY_SIZE(kam_ctr_data_flow),
    &kam_ctr_data_flow[0] },
  { "vtn_dataflow", UNC_KT_VTN_DATAFLOW, ARRAY_SIZE(kam_vtn_dataflow),
    &kam_vtn_dataflow[0] },
  { NULL, UNC_KT_ROOT, 0, NULL }
};



}  // namespace capa
}  // namespace unc
