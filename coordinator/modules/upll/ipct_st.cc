/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include "pfc/ipc.h"
#include "pfc/ipc_struct.h"
#include "ipct_st.hh"

namespace unc {
namespace upll {
namespace ipc_util {

const char *IpctSt::kIpcStrStKeyRoot = "key_root";
const char *IpctSt::kIpcStrStKeyUnifiedNw = "key_unified_nw";
const char *IpctSt::kIpcStrStValUnifiedNw = "val_unified_nw";
const char *IpctSt::kIpcStrStKeyUnwLabel = "key_unw_label";
const char *IpctSt::kIpcStrStValUnwLabel = "val_unw_label";
const char *IpctSt::kIpcStrStKeyUnwLabelRange = "key_unw_label_range";
const char *IpctSt::kIpcStrStValUnwLabelRange = "val_unw_label_range";
const char *IpctSt::kIpcStrStKeyUnwSpineDomain = "key_unw_spine_domain";
const char *IpctSt::kIpcStrStValUnwSpineDomain = "val_unw_spine_domain";
const char *IpctSt::kIpcStrStValUnwSpineDomain_Ext = "val_unw_spdom_ext";
const char *IpctSt::kIpcStrStValSpineAlarmSt = "val_spdom_st";
const char *IpctSt::kIpcStrStValUnwSpineDomainSt = "val_unw_spine_domain_st";
const char *IpctSt::kIpcStrStValUnwSpineDomainAssignedLabel =
                    "val_unw_spine_domain_assigned_label";
const char *IpctSt::kIpcStrStValUnwSpineDomainFdbentry =
                    "val_unw_spine_domain_fdbentry";
const char *IpctSt::kIpcStrStValUnwSpineDomainFdbentryVtn =
                    "val_unw_spine_domain_fdbentry_vtn";
const char *IpctSt::kIpcStrStKeyVtnUnified = "key_vtn_unified";
const char *IpctSt::kIpcStrStValVtnUnified = "val_vtn_unified";
const char *IpctSt::kIpcStrStValPing = "val_ping";
const char *IpctSt::kIpcStrStValVtnNeighbor = "val_vtn_neighbor";
const char *IpctSt::kIpcStrStKeyVtn = "key_vtn";
const char *IpctSt::kIpcStrStValVtn = "val_vtn";
const char *IpctSt::kIpcStrStValRenameVtn = "val_rename_vtn";
const char *IpctSt::kIpcStrStValVtnSt = "val_vtn_st";
const char *IpctSt::kIpcStrStKeyVtnController = "key_vtn_controller";
const char *IpctSt::kIpcStrStValVtnMappingControllerSt =
    "val_vtn_mapping_controller_st";
const char *IpctSt::kIpcStrStKeyVtnstationController =
    "key_vtnstation_controller";
const char *IpctSt::kIpcStrStValVtnstationControllerSt =
    "val_vtnstation_controller_st";
const char *IpctSt::kIpcStrStValVtnstationControllerStat =
    "val_vtnstation_controller_stat";
const char *IpctSt::kIpcStrStKeyVbr = "key_vbr";
const char *IpctSt::kIpcStrStValVbr = "val_vbr";
const char *IpctSt::kIpcStrStValRenameVbr = "val_rename_vbr";
const char *IpctSt::kIpcStrStValVbrSt = "val_vbr_st";
const char *IpctSt::kIpcStrStValVbrL2DomainSt = "val_vbr_l2_domain_st";
const char *IpctSt::kIpcStrStValVbrL2DomainMemberSt =
    "val_vbr_l2_domain_member_st";
const char *IpctSt::kIpcStrStValVbrMacEntrySt = "val_vbr_mac_entry_st";
const char *IpctSt::kIpcStrStKeyConvertVbr = "key_convert_vbr";
const char *IpctSt::kIpcStrStValConvertVbr = "val_convert_vbr";
const char *IpctSt::kIpcStrStKeyVbrIf = "key_vbr_if";
const char *IpctSt::kIpcStrStValPortMap = "val_port_map";
const char *IpctSt::kIpcStrStValVbrIf = "val_vbr_if";
const char *IpctSt::kIpcStrStValVbrIfSt = "val_vbr_if_st";
const char *IpctSt::kIpcStrStKeyVlanMap = "key_vlan_map";
const char *IpctSt::kIpcStrStValVlanMap = "val_vlan_map";
const char *IpctSt::kIpcStrStKeyVbrPortMap = "key_vbr_portmap";
const char *IpctSt::kIpcStrStValVbrPortMap = "val_vbr_portmap";
const char *IpctSt::kIpcStrStValVbrPortMapSt = "val_vbr_portmap_st";
const char *IpctSt::kIpcStrStKeyVrt = "key_vrt";
const char *IpctSt::kIpcStrStValVrt = "val_vrt";
const char *IpctSt::kIpcStrStValRenameVrt = "val_rename_vrt";
const char *IpctSt::kIpcStrStValVrtSt = "val_vrt_st";
const char *IpctSt::kIpcStrStValVrtDhcpRelaySt = "val_vrt_dhcp_relay_st";
const char *IpctSt::kIpcStrStValDhcpRelayIfSt = "val_dhcp_relay_if_st";
const char *IpctSt::kIpcStrStValVrtArpEntrySt = "val_vrt_arp_entry_st";
const char *IpctSt::kIpcStrStValVrtIpRouteSt = "val_vrt_ip_route_st";
const char *IpctSt::kIpcStrStKeyVrtIf = "key_vrt_if";
const char *IpctSt::kIpcStrStValVrtIf = "val_vrt_if";
const char *IpctSt::kIpcStrStValVrtIfSt = "val_vrt_if_st";
const char *IpctSt::kIpcStrStKeyStaticIpRoute = "key_static_ip_route";
const char *IpctSt::kIpcStrStValStaticIpRoute = "val_static_ip_route";
const char *IpctSt::kIpcStrStKeyDhcpRelayIf = "key_dhcp_relay_if";
const char *IpctSt::kIpcStrStValDhcpRelayIf = "val_dhcp_relay_if";
const char *IpctSt::kIpcStrStKeyDhcpRelayServer = "key_dhcp_relay_server";
const char *IpctSt::kIpcStrStValDhcpRelayServer = "val_dhcp_relay_server";
const char *IpctSt::kIpcStrStKeyVterminal = "key_vterm";
const char *IpctSt::kIpcStrStValVterminal = "val_vterm";
const char *IpctSt::kIpcStrStValRenameVterminal = "val_rename_vterm";
const char *IpctSt::kIpcStrStValVterminalSt = "val_vterm_st";
const char *IpctSt::kIpcStrStKeyVtermIf = "key_vterm_if";
const char *IpctSt::kIpcStrStValVtermIf = "val_vterm_if";
const char *IpctSt::kIpcStrStValVtermIfSt = "val_vterm_if_st";
const char *IpctSt::kIpcStrStKeyNwm = "key_nwm";
const char *IpctSt::kIpcStrStValNwm = "val_nwm";
const char *IpctSt::kIpcStrStValNwmSt = "val_nwm_st";
const char *IpctSt::kIpcStrStValNwmHostSt = "val_nwm_host_st";
const char *IpctSt::kIpcStrStKeyNwmHost = "key_nwm_host";
const char *IpctSt::kIpcStrStValNwmHost = "val_nwm_host";
const char *IpctSt::kIpcStrStKeyVtep = "key_vtep";
const char *IpctSt::kIpcStrStValVtep = "val_vtep";
const char *IpctSt::kIpcStrStValVtepSt = "val_vtep_st";
const char *IpctSt::kIpcStrStKeyVtepIf = "key_vtep_if";
const char *IpctSt::kIpcStrStValVtepIf = "val_vtep_if";
const char *IpctSt::kIpcStrStValVtepIfSt = "val_vtep_if_st";
const char *IpctSt::kIpcStrStKeyVtepGrp = "key_vtep_grp";
const char *IpctSt::kIpcStrStValVtepGrp = "val_vtep_grp";
const char *IpctSt::kIpcStrStKeyVtepGrpMember = "key_vtep_grp_member";
const char *IpctSt::kIpcStrStValVtepGrpMember = "val_vtep_grp_member";
const char *IpctSt::kIpcStrStKeyVtunnel = "key_vtunnel";
const char *IpctSt::kIpcStrStValVtunnel = "val_vtunnel";
const char *IpctSt::kIpcStrStValVtunnelSt = "val_vtunnel_st";
const char *IpctSt::kIpcStrStKeyVtunnelIf = "key_vtunnel_if";
const char *IpctSt::kIpcStrStValVtunnelIf = "val_vtunnel_if";
const char *IpctSt::kIpcStrStValVtunnelIfSt = "val_vtunnel_if_st";
const char *IpctSt::kIpcStrStKeyVunknown = "key_vunknown";
const char *IpctSt::kIpcStrStValVunknown = "val_vunknown";
const char *IpctSt::kIpcStrStKeyVunkIf = "key_vunk_if";
const char *IpctSt::kIpcStrStValVunkIf = "val_vunk_if";
const char *IpctSt::kIpcStrStKeyVlink = "key_vlink";
const char *IpctSt::kIpcStrStValVlink = "val_vlink";
const char *IpctSt::kIpcStrStValVlinkSt = "val_vlink_st";
const char *IpctSt::kIpcStrStValRenameVlink = "val_rename_vlink";
const char *IpctSt::kIpcStrStKeyConvertVtunnel = "key_convert_vtunnel";
const char *IpctSt::kIpcStrStValConvertVtunnel = "val_convert_vtunnel";
const char *IpctSt::kIpcStrStValVtnGatewayPort = "val_vtn_gateway_port";
const char *IpctSt::kIpcStrStKeyFlowlist = "key_flowlist";
const char *IpctSt::kIpcStrStValFlowlist = "val_flowlist";
const char *IpctSt::kIpcStrStValRenameFlowlist = "val_rename_flowlist";
const char *IpctSt::kIpcStrStKeyFlowlistEntry = "key_flowlist_entry";
const char *IpctSt::kIpcStrStValFlowlistEntry = "val_flowlist_entry";
const char *IpctSt::kIpcStrStValFlowlistEntrySt = "val_flowlist_entry_st";
const char *IpctSt::kIpcStrStPomStats = "pom_stats";
const char *IpctSt::kIpcStrStKeyVtnFlowfilter = "key_vtn_flowfilter";
const char *IpctSt::kIpcStrStValFlowfilter = "val_flowfilter";
const char *IpctSt::kIpcStrStKeyVtnFlowfilterEntry = "key_vtn_flowfilter_entry";
const char *IpctSt::kIpcStrStValVtnFlowfilterEntry = "val_vtn_flowfilter_entry";
const char *IpctSt::kIpcStrStValVtnFlowfilterControllerSt =
    "val_vtn_flowfilter_controller_st";
const char *IpctSt::kIpcStrStKeyVtnFlowfilterController =
    "key_vtn_flowfilter_controller";
const char *IpctSt::kIpcStrStValFlowfilterController =
    "val_flowfilter_controller";
const char *IpctSt::kIpcStrStKeyVbrFlowfilter = "key_vbr_flowfilter";
const char *IpctSt::kIpcStrStKeyVbrFlowfilterEntry = "key_vbr_flowfilter_entry";
const char *IpctSt::kIpcStrStValFlowfilterEntry = "val_flowfilter_entry";
const char *IpctSt::kIpcStrStValFlowfilterEntrySt = "val_flowfilter_entry_st";
const char *IpctSt::kIpcStrStKeyVbrIfFlowfilter = "key_vbr_if_flowfilter";
const char *IpctSt::kIpcStrStKeyVbrIfFlowfilterEntry =
    "key_vbr_if_flowfilter_entry";
const char *IpctSt::kIpcStrStKeyVrtIfFlowfilter = "key_vrt_if_flowfilter";
const char *IpctSt::kIpcStrStKeyVrtIfFlowfilterEntry =
    "key_vrt_if_flowfilter_entry";
const char *IpctSt::kIpcStrStKeyVtermIfFlowfilter = "key_vterm_if_flowfilter";
const char *IpctSt::kIpcStrStKeyVtermIfFlowfilterEntry =
    "key_vterm_if_flowfilter_entry";
const char *IpctSt::kIpcStrStKeyPolicingprofile = "key_policingprofile";
const char *IpctSt::kIpcStrStValPolicingprofile = "val_policingprofile";
const char *IpctSt::kIpcStrStValRenamePolicingprofile =
    "val_rename_policingprofile";
const char *IpctSt::kIpcStrStKeyPolicingprofileEntry =
    "key_policingprofile_entry";
const char *IpctSt::kIpcStrStValPolicingprofileEntry =
    "val_policingprofile_entry";
const char *IpctSt::kIpcStrStValPolicingmap = "val_policingmap";
const char *IpctSt::kIpcStrStValPolicingmapControllerSt =
    "val_policingmap_controller_st";
const char *IpctSt::kIpcStrStValPolicingmapSwitchSt =
    "val_policingmap_switch_st";
const char *IpctSt::kIpcStrStKeyVtnPolicingmapController =
    "key_vtn_policingmap_controller";
const char *IpctSt::kIpcStrStValPolicingmapController =
    "val_policingmap_controller";
const char *IpctSt::kIpcStrStKeyVbrPolicingmapEntry =
    "key_vbr_policingmap_entry";
const char *IpctSt::kIpcStrStKeyVbrifPolicingmapEntry =
    "key_vbrif_policingmap_entry";
const char *IpctSt::kIpcStrStKeyVtermIfPolicingMapEntry =
    "key_vtermif_policingmap_entry";
  // Expand structures
const char *IpctSt::kIpcStrStValVbrExpand = "val_vbr_expand";
const char *IpctSt::kIpcStrStValVbrPortMapExpand = "val_vbr_portmap_expand";
const char *IpctSt::kIpcStrStValVbrIfExpand = "val_vbr_if_expand";
const char *IpctSt::kIpcStrStValVtunnelExpand = "val_vtunnel_expand";
const char *IpctSt::kIpcStrStValVtunnelIfExpand = "val_vtunnel_if_expand";
const char *IpctSt::kIpcStrStValVlinkExpand = "val_vlink_expand";
  // Add Driver structures below
const char *IpctSt::kIpcStrStPfcdrvValVbrIf = "pfcdrv_val_vbr_if";
const char *IpctSt::kIpcStrStPfcdrvValVbrifVextif = "pfcdrv_val_vbrif_vextif";
const char *IpctSt::kIpcStrStPfcdrvValFlowfilterEntry =
    "pfcdrv_val_flowfilter_entry";
const char *IpctSt::kIpcStrStPfcdrvValVbrifPolicingmap =
    "pfcdrv_val_vbrif_policingmap";
const char *IpctSt::kIpcStrStPfcdrvValVtnController =
    "pfcdrv_val_vtn_controller";
/* VlanmapOnBoundary: Added new val struct */
const char *IpctSt::kIpcStrStPfcdrvValVlanMap =
    "pfcdrv_val_vlan_map";
const char *IpctSt::kIpcStrStPfcdrvValVbrPortMap = "pfcdrv_val_vbr_portmap";

  // Add Physical structures below
const char *IpctSt::kIpcStrStKeyCtr = "key_ctr";
const char *IpctSt::kIpcStrStValCtr = "val_ctr";
const char *IpctSt::kIpcStrStValCtrSt = "val_ctr_st";
const char *IpctSt::kIpcStrStKeyCtrDomain = "key_ctr_domain";
const char *IpctSt::kIpcStrStValCtrDomain = "val_ctr_domain";
const char *IpctSt::kIpcStrStValCtrDomainSt = "val_ctr_domain_st";
const char *IpctSt::kIpcStrStKeyLogicalPort = "key_logical_port";
const char *IpctSt::kIpcStrStValLogicalPort = "val_logical_port";
const char *IpctSt::kIpcStrStValLogicalPortSt = "val_logical_port_st";
const char *IpctSt::kIpcStrStKeyBoundary = "key_boundary";
const char *IpctSt::kIpcStrStValBoundary = "val_boundary";
const char *IpctSt::kIpcStrStValBoundarySt = "val_boundary_st";
const char *IpctSt::kIpcStrStValPathFaultAlarm = "val_path_fault_alarm";
  // Add Overlay Driver structures below
const char *IpctSt::kIpcStrStVnpdrvValVtunnel = "vnpdrv_val_vtunnel";
const char *IpctSt::kIpcStrStVnpdrvValVtunnelIf ="vnpdrv_val_vtunnel_if";
  // Add vtn dataflow structure below
const char *IpctSt::kIpcStrStKeyVtnDataflow ="key_vtn_dataflow";

std::map<std::string, const pfc_ipcstdef_t*> IpctSt::ipc_stdef_smap_;
std::map<IpctSt::IpcStructNum, const pfc_ipcstdef_t*> IpctSt::ipc_stdef_nmap_;
std::map<std::string, IpctSt::IpcStructNum> IpctSt::ipc_strname_to_stnum_map_;

#define REGISTER_STDEF(st_name_literal, st_name, st_num)   \
{                                                          \
  pfc_ipcstdef_t *stdef = new pfc_ipcstdef_t;              \
  PFC_IPC_STDEF_INIT(stdef, st_name_literal);              \
  RegisterIpcStdef(st_name, st_num, stdef);                \
}



void IpctSt::RegisterAll() {
  REGISTER_STDEF(key_root,
                 kIpcStrStKeyRoot,
                 kIpcStKeyRoot);
  REGISTER_STDEF(key_unified_nw,
                 kIpcStrStKeyUnifiedNw,
                 kIpcStKeyUnifiedNw);
  REGISTER_STDEF(val_unified_nw,
                kIpcStrStValUnifiedNw,
                kIpcStValUnifiedNw);
  REGISTER_STDEF(key_unw_label,
                 kIpcStrStKeyUnwLabel,
                 kIpcStKeyUnwLabel);
  REGISTER_STDEF(val_unw_label,
                kIpcStrStValUnwLabel,
                kIpcStValUnwLabel);
  REGISTER_STDEF(key_unw_label_range,
                 kIpcStrStKeyUnwLabelRange,
                 kIpcStKeyUnwLabelRange);
  REGISTER_STDEF(val_unw_label_range,
                kIpcStrStValUnwLabelRange,
                kIpcStValUnwLabelRange);
  REGISTER_STDEF(key_unw_spine_domain,
                kIpcStrStKeyUnwSpineDomain,
                kIpcStKeyUnwSpineDomain);
  REGISTER_STDEF(val_unw_spine_domain,
                kIpcStrStValUnwSpineDomain,
                kIpcStValUnwSpineDomain);
  REGISTER_STDEF(val_unw_spine_domain_st,
                kIpcStrStValUnwSpineDomainSt,
                kIpcStValUnwSpineDomainSt);
  REGISTER_STDEF(val_unw_spine_domain_assigned_label,
                kIpcStrStValUnwSpineDomainAssignedLabel,
                kIpcStValUnwSpineDomainAssignedLabel);
  REGISTER_STDEF(val_unw_spine_domain_fdbentry,
                kIpcStrStValUnwSpineDomainFdbentry,
                kIpcStValUnwSpineDomainFdbentry);
  REGISTER_STDEF(val_unw_spine_domain_fdbentry_vtn,
                kIpcStrStValUnwSpineDomainFdbentryVtn,
                kIpcStValUnwSpineDomainFdbentryVtn);
  REGISTER_STDEF(key_vtn_unified,
                kIpcStrStKeyVtnUnified,
                kIpcStKeyVtnUnified);
  REGISTER_STDEF(val_vtn_unified,
                kIpcStrStValVtnUnified,
                kIpcStValVtnUnified);
  REGISTER_STDEF(val_ping,
                 kIpcStrStValPing,
                 kIpcStValPing);
  REGISTER_STDEF(val_vtn_neighbor,
                 kIpcStrStValVtnNeighbor,
                 kIpcStValVtnNeighbor);
  REGISTER_STDEF(key_vtn,
                 kIpcStrStKeyVtn,
                 kIpcStKeyVtn);
  REGISTER_STDEF(val_vtn,
                 kIpcStrStValVtn,
                 kIpcStValVtn);
  REGISTER_STDEF(val_rename_vtn,
                 kIpcStrStValRenameVtn,
                 kIpcStValRenameVtn);
  REGISTER_STDEF(val_vtn_st,
                 kIpcStrStValVtnSt,
                 kIpcStValVtnSt);
  REGISTER_STDEF(key_vtn_controller,
                 kIpcStrStKeyVtnController,
                 kIpcStKeyVtnController);
  REGISTER_STDEF(val_vtn_mapping_controller_st,
                 kIpcStrStValVtnMappingControllerSt,
                 kIpcStValVtnMappingControllerSt);
  REGISTER_STDEF(key_vtnstation_controller,
                 kIpcStrStKeyVtnstationController,
                 kIpcStKeyVtnstationController);
  REGISTER_STDEF(val_vtnstation_controller_st,
                 kIpcStrStValVtnstationControllerSt,
                 kIpcStValVtnstationControllerSt);
  REGISTER_STDEF(val_vtnstation_controller_stat,
                 kIpcStrStValVtnstationControllerStat,
                 kIpcStValVtnstationControllerStat);
  REGISTER_STDEF(key_vbr,
                 kIpcStrStKeyVbr,
                 kIpcStKeyVbr);
  REGISTER_STDEF(val_vbr,
                 kIpcStrStValVbr,
                 kIpcStValVbr);
  /*REGISTER_STDEF(key_convert_vbr,
                 kIpcStrStKeyConvertVbr,
                 kIpcStKeyConvertVbr);
  REGISTER_STDEF(val_convert_vbr,
                 kIpcStrStValConvertVbr,
                 kIpcStValConvertVbr);*/
  REGISTER_STDEF(val_rename_vbr,
                 kIpcStrStValRenameVbr,
                 kIpcStValRenameVbr);
  REGISTER_STDEF(val_vbr_st,
                 kIpcStrStValVbrSt,
                 kIpcStValVbrSt);
  REGISTER_STDEF(val_vbr_l2_domain_st,
                 kIpcStrStValVbrL2DomainSt,
                 kIpcStValVbrL2DomainSt);
  REGISTER_STDEF(val_vbr_l2_domain_member_st,
                 kIpcStrStValVbrL2DomainMemberSt,
                 kIpcStValVbrL2DomainMemberSt);
  REGISTER_STDEF(val_vbr_mac_entry_st,
                 kIpcStrStValVbrMacEntrySt,
                 kIpcStValVbrMacEntrySt);
  REGISTER_STDEF(key_vbr_portmap,
                 kIpcStrStKeyVbrPortMap,
                 kIpcStKeyVbrPortMap);
  REGISTER_STDEF(val_vbr_portmap,
                 kIpcStrStValVbrPortMap,
                 kIpcStValVbrPortMap);
  REGISTER_STDEF(val_vbr_portmap_st,
                 kIpcStrStValVbrPortMapSt,
                 kIpcStValVbrPortMapSt);
  REGISTER_STDEF(key_vbr_if,
                 kIpcStrStKeyVbrIf,
                 kIpcStKeyVbrIf);
  REGISTER_STDEF(val_port_map,
                 kIpcStrStValPortMap,
                 kIpcStValPortMap);
  REGISTER_STDEF(val_vbr_if,
                 kIpcStrStValVbrIf,
                 kIpcStValVbrIf);
  REGISTER_STDEF(val_vbr_if_st,
                 kIpcStrStValVbrIfSt,
                 kIpcStValVbrIfSt);
  REGISTER_STDEF(key_vlan_map,
                 kIpcStrStKeyVlanMap,
                 kIpcStKeyVlanMap);
  REGISTER_STDEF(val_vlan_map,
                 kIpcStrStValVlanMap,
                 kIpcStValVlanMap);
  REGISTER_STDEF(key_vrt,
                 kIpcStrStKeyVrt,
                 kIpcStKeyVrt);
  REGISTER_STDEF(val_vrt,
                 kIpcStrStValVrt,
                 kIpcStValVrt);
  REGISTER_STDEF(val_rename_vrt,
                 kIpcStrStValRenameVrt,
                 kIpcStValRenameVrt);
  REGISTER_STDEF(val_vrt_st,
                 kIpcStrStValVrtSt,
                 kIpcStValVrtSt);
  REGISTER_STDEF(val_vrt_dhcp_relay_st,
                 kIpcStrStValVrtDhcpRelaySt,
                 kIpcStValVrtDhcpRelaySt);
  REGISTER_STDEF(val_dhcp_relay_if_st,
                 kIpcStrStValDhcpRelayIfSt,
                 kIpcStValDhcpRelayIfSt);
  REGISTER_STDEF(val_vrt_arp_entry_st,
                 kIpcStrStValVrtArpEntrySt,
                 kIpcStValVrtArpEntrySt);
  REGISTER_STDEF(val_vrt_ip_route_st,
                 kIpcStrStValVrtIpRouteSt,
                 kIpcStValVrtIpRouteSt);
  REGISTER_STDEF(key_vrt_if,
                 kIpcStrStKeyVrtIf,
                 kIpcStKeyVrtIf);
  REGISTER_STDEF(val_vrt_if,
                 kIpcStrStValVrtIf,
                 kIpcStValVrtIf);
  REGISTER_STDEF(val_vrt_if_st,
                 kIpcStrStValVrtIfSt,
                 kIpcStValVrtIfSt);
  REGISTER_STDEF(key_static_ip_route,
                 kIpcStrStKeyStaticIpRoute,
                 kIpcStKeyStaticIpRoute);
  REGISTER_STDEF(val_static_ip_route,
                 kIpcStrStValStaticIpRoute,
                 kIpcStValStaticIpRoute);
  REGISTER_STDEF(key_dhcp_relay_if,
                 kIpcStrStKeyDhcpRelayIf,
                 kIpcStKeyDhcpRelayIf);
  REGISTER_STDEF(val_dhcp_relay_if,
                 kIpcStrStValDhcpRelayIf,
                 kIpcStValDhcpRelayIf);
  REGISTER_STDEF(key_dhcp_relay_server,
                 kIpcStrStKeyDhcpRelayServer,
                 kIpcStKeyDhcpRelayServer);
  REGISTER_STDEF(val_dhcp_relay_server,
                 kIpcStrStValDhcpRelayServer,
                 kIpcStValDhcpRelayServer);
  REGISTER_STDEF(key_vterm,
                 kIpcStrStKeyVterminal,
                 kIpcStKeyVterminal);
  REGISTER_STDEF(val_vterm,
                 kIpcStrStValVterminal,
                 kIpcStValVterminal);
  REGISTER_STDEF(val_rename_vterm,
                 kIpcStrStValRenameVterminal,
                 kIpcStValRenameVterminal);
  REGISTER_STDEF(val_vterm_st,
                 kIpcStrStValVterminalSt,
                 kIpcStValVterminalSt);
  REGISTER_STDEF(key_vterm_if,
                 kIpcStrStKeyVtermIf,
                 kIpcStKeyVtermIf);
  REGISTER_STDEF(val_vterm_if,
                 kIpcStrStValVtermIf,
                 kIpcStValVtermIf);
  REGISTER_STDEF(val_vterm_if_st,
                 kIpcStrStValVtermIfSt,
                 kIpcStValVtermIfSt);
  REGISTER_STDEF(key_nwm,
                 kIpcStrStKeyNwm,
                 kIpcStKeyNwm);
  REGISTER_STDEF(val_nwm,
                 kIpcStrStValNwm,
                 kIpcStValNwm);
  REGISTER_STDEF(val_nwm_st,
                 kIpcStrStValNwmSt,
                 kIpcStValNwmSt);
  REGISTER_STDEF(val_nwm_host_st,
                 kIpcStrStValNwmHostSt,
                 kIpcStValNwmHostSt);
  REGISTER_STDEF(key_nwm_host,
                 kIpcStrStKeyNwmHost,
                 kIpcStKeyNwmHost);
  REGISTER_STDEF(val_nwm_host,
                 kIpcStrStValNwmHost,
                 kIpcStValNwmHost);
  REGISTER_STDEF(key_vtep,
                 kIpcStrStKeyVtep,
                 kIpcStKeyVtep);
  REGISTER_STDEF(val_vtep,
                 kIpcStrStValVtep,
                 kIpcStValVtep);
  REGISTER_STDEF(val_vtep_st,
                 kIpcStrStValVtepSt,
                 kIpcStValVtepSt);
  REGISTER_STDEF(key_vtep_if,
                 kIpcStrStKeyVtepIf,
                 kIpcStKeyVtepIf);
  REGISTER_STDEF(val_vtep_if,
                 kIpcStrStValVtepIf,
                 kIpcStValVtepIf);
  REGISTER_STDEF(val_vtep_if_st,
                 kIpcStrStValVtepIfSt,
                 kIpcStValVtepIfSt);
  REGISTER_STDEF(key_vtep_grp,
                 kIpcStrStKeyVtepGrp,
                 kIpcStKeyVtepGrp);
  REGISTER_STDEF(val_vtep_grp,
                 kIpcStrStValVtepGrp,
                 kIpcStValVtepGrp);
  REGISTER_STDEF(key_vtep_grp_member,
                 kIpcStrStKeyVtepGrpMember,
                 kIpcStKeyVtepGrpMember);
  REGISTER_STDEF(val_vtep_grp_member,
                 kIpcStrStValVtepGrpMember,
                 kIpcStValVtepGrpMember);
  REGISTER_STDEF(key_vtunnel,
                 kIpcStrStKeyVtunnel,
                 kIpcStKeyVtunnel);
  REGISTER_STDEF(val_vtunnel,
                 kIpcStrStValVtunnel,
                 kIpcStValVtunnel);
  REGISTER_STDEF(val_vtunnel_st,
                 kIpcStrStValVtunnelSt,
                 kIpcStValVtunnelSt);
  REGISTER_STDEF(key_vtunnel_if,
                 kIpcStrStKeyVtunnelIf,
                 kIpcStKeyVtunnelIf);
  REGISTER_STDEF(val_vtunnel_if,
                 kIpcStrStValVtunnelIf,
                 kIpcStValVtunnelIf);
  REGISTER_STDEF(val_vtunnel_if_st,
                 kIpcStrStValVtunnelIfSt,
                 kIpcStValVtunnelIfSt);
  REGISTER_STDEF(key_vunknown,
                 kIpcStrStKeyVunknown,
                 kIpcStKeyVunknown);
  REGISTER_STDEF(val_vunknown,
                 kIpcStrStValVunknown,
                 kIpcStValVunknown);
  REGISTER_STDEF(key_vunk_if,
                 kIpcStrStKeyVunkIf,
                 kIpcStKeyVunkIf);
  REGISTER_STDEF(val_vunk_if,
                 kIpcStrStValVunkIf,
                 kIpcStValVunkIf);
  REGISTER_STDEF(key_vlink,
                 kIpcStrStKeyVlink,
                 kIpcStKeyVlink);
  REGISTER_STDEF(val_vlink,
                 kIpcStrStValVlink,
                 kIpcStValVlink);
  REGISTER_STDEF(val_vlink_st,
                 kIpcStrStValVlinkSt,
                 kIpcStValVlinkSt);
  REGISTER_STDEF(val_rename_vlink,
                 kIpcStrStValRenameVlink,
                 kIpcStValRenameVlink);
  REGISTER_STDEF(key_flowlist,
                 kIpcStrStKeyFlowlist,
                 kIpcStKeyFlowlist);
  REGISTER_STDEF(val_flowlist,
                 kIpcStrStValFlowlist,
                 kIpcStValFlowlist);
  REGISTER_STDEF(val_rename_flowlist,
                 kIpcStrStValRenameFlowlist,
                 kIpcStValRenameFlowlist);
  REGISTER_STDEF(key_flowlist_entry,
                 kIpcStrStKeyFlowlistEntry,
                 kIpcStKeyFlowlistEntry);
  REGISTER_STDEF(val_flowlist_entry,
                 kIpcStrStValFlowlistEntry,
                 kIpcStValFlowlistEntry);
  REGISTER_STDEF(val_flowlist_entry_st,
                 kIpcStrStValFlowlistEntrySt,
                 kIpcStValFlowlistEntrySt);
  REGISTER_STDEF(pom_stats,
                 kIpcStrStPomStats,
                 kIpcStPomStats);
  REGISTER_STDEF(key_vtn_flowfilter,
                 kIpcStrStKeyVtnFlowfilter,
                 kIpcStKeyVtnFlowfilter);
  REGISTER_STDEF(val_flowfilter,
                 kIpcStrStValFlowfilter,
                 kIpcStValFlowfilter);
  REGISTER_STDEF(key_vtn_flowfilter_entry,
                 kIpcStrStKeyVtnFlowfilterEntry,
                 kIpcStKeyVtnFlowfilterEntry);
  REGISTER_STDEF(val_vtn_flowfilter_entry,
                 kIpcStrStValVtnFlowfilterEntry,
                 kIpcStValVtnFlowfilterEntry);
  REGISTER_STDEF(val_vtn_flowfilter_controller_st,
                 kIpcStrStValVtnFlowfilterControllerSt,
                 kIpcStValVtnFlowfilterControllerSt);
  REGISTER_STDEF(key_vtn_flowfilter_controller,
                 kIpcStrStKeyVtnFlowfilterController,
                 kIpcStKeyVtnFlowfilterController);
  REGISTER_STDEF(val_flowfilter_controller,
                 kIpcStrStValFlowfilterController,
                 kIpcStValFlowfilterController);
  REGISTER_STDEF(key_vbr_flowfilter,
                 kIpcStrStKeyVbrFlowfilter,
                 kIpcStKeyVbrFlowfilter);
  REGISTER_STDEF(key_vbr_flowfilter_entry,
                 kIpcStrStKeyVbrFlowfilterEntry,
                 kIpcStKeyVbrFlowfilterEntry);
  REGISTER_STDEF(val_flowfilter_entry,
                 kIpcStrStValFlowfilterEntry,
                 kIpcStValFlowfilterEntry);
  REGISTER_STDEF(val_flowfilter_entry_st,
                 kIpcStrStValFlowfilterEntrySt,
                 kIpcStValFlowfilterEntrySt);
  REGISTER_STDEF(key_vbr_if_flowfilter,
                 kIpcStrStKeyVbrIfFlowfilter,
                 kIpcStKeyVbrIfFlowfilter);
  REGISTER_STDEF(key_vbr_if_flowfilter_entry,
                 kIpcStrStKeyVbrIfFlowfilterEntry,
                 kIpcStKeyVbrIfFlowfilterEntry);
  REGISTER_STDEF(key_vrt_if_flowfilter,
                 kIpcStrStKeyVrtIfFlowfilter,
                 kIpcStKeyVrtIfFlowfilter);
  REGISTER_STDEF(key_vrt_if_flowfilter_entry,
                 kIpcStrStKeyVrtIfFlowfilterEntry,
                 kIpcStKeyVrtIfFlowfilterEntry);
  REGISTER_STDEF(key_vterm_if_flowfilter,
                 kIpcStrStKeyVtermIfFlowfilter,
                 kIpcStKeyVtermIfFlowfilter);
  REGISTER_STDEF(key_vterm_if_flowfilter_entry,
                 kIpcStrStKeyVtermIfFlowfilterEntry,
                 kIpcStKeyVtermIfFlowfilterEntry);
  REGISTER_STDEF(key_policingprofile,
                 kIpcStrStKeyPolicingprofile,
                 kIpcStKeyPolicingprofile);
  REGISTER_STDEF(val_policingprofile,
                 kIpcStrStValPolicingprofile,
                 kIpcStValPolicingprofile);
  REGISTER_STDEF(val_rename_policingprofile,
                 kIpcStrStValRenamePolicingprofile,
                 kIpcStValRenamePolicingprofile);
  REGISTER_STDEF(key_policingprofile_entry,
                 kIpcStrStKeyPolicingprofileEntry,
                 kIpcStKeyPolicingprofileEntry);
  REGISTER_STDEF(val_policingprofile_entry,
                 kIpcStrStValPolicingprofileEntry,
                 kIpcStValPolicingprofileEntry);
  REGISTER_STDEF(val_policingmap,
                 kIpcStrStValPolicingmap,
                 kIpcStValPolicingmap);
  REGISTER_STDEF(val_policingmap_controller_st,
                 kIpcStrStValPolicingmapControllerSt,
                 kIpcStValPolicingmapControllerSt);
  REGISTER_STDEF(val_policingmap_switch_st,
                 kIpcStrStValPolicingmapSwitchSt,
                 kIpcStValPolicingmapSwitchSt);
  REGISTER_STDEF(key_vtn_policingmap_controller,
                 kIpcStrStKeyVtnPolicingmapController,
                 kIpcStKeyVtnPolicingmapController);
  REGISTER_STDEF(val_policingmap_controller,
                 kIpcStrStValPolicingmapController,
                 kIpcStValPolicingmapController);
  REGISTER_STDEF(key_vbr_policingmap_entry,
                 kIpcStrStKeyVbrPolicingmapEntry,
                 kIpcStKeyVbrPolicingmapEntry);
  REGISTER_STDEF(key_vbrif_policingmap_entry,
                 kIpcStrStKeyVbrifPolicingmapEntry,
                 kIpcStKeyVbrifPolicingmapEntry);
  REGISTER_STDEF(key_vtermif_policingmap_entry,
                 kIpcStrStKeyVtermIfPolicingMapEntry,
                 kIpcStKeyVtermIfPolicingMapEntry);
  // Add Driver structures below
  REGISTER_STDEF(pfcdrv_val_vbr_if,
                 kIpcStrStPfcdrvValVbrIf,
                 kIpcStPfcdrvValVbrIf);
  REGISTER_STDEF(pfcdrv_val_vbrif_vextif,
                 kIpcStrStPfcdrvValVbrifVextif,
                 kIpcStPfcdrvValVbrifVextif);
  REGISTER_STDEF(pfcdrv_val_flowfilter_entry,
                 kIpcStrStPfcdrvValFlowfilterEntry,
                 kIpcStPfcdrvValFlowfilterEntry);
  REGISTER_STDEF(pfcdrv_val_vbrif_policingmap,
                 kIpcStrStPfcdrvValVbrifPolicingmap,
                 kIpcStPfcdrvValVbrifPolicingmap);
  /* VlanmapOnBoundary: Added new val struct */
  REGISTER_STDEF(pfcdrv_val_vlan_map,
                 kIpcStrStPfcdrvValVlanMap,
                 kIpcStPfcdrvValVlanMap);
  REGISTER_STDEF(pfcdrv_val_vbr_portmap,
                 kIpcStrStPfcdrvValVbrPortMap,
                 kIpcStPfcdrvValVbrPortMap);
  REGISTER_STDEF(pfcdrv_val_vtn_controller,
                 kIpcStrStPfcdrvValVtnController,
                 kIpcStPfcdrvValVtnController);

  // Add Physical structures below
  REGISTER_STDEF(key_ctr,
                 kIpcStrStKeyCtr,
                 kIpcStKeyCtr);
  REGISTER_STDEF(val_ctr,
                 kIpcStrStValCtr,
                 kIpcStValCtr);
  REGISTER_STDEF(val_ctr_st,
                 kIpcStrStValCtrSt,
                 kIpcStValCtrSt);
  REGISTER_STDEF(key_ctr_domain,
                 kIpcStrStKeyCtrDomain,
                 kIpcStKeyCtrDomain);
  REGISTER_STDEF(val_ctr_domain,
                 kIpcStrStValCtrDomain,
                 kIpcStValCtrDomain);
  REGISTER_STDEF(val_ctr_domain_st,
                 kIpcStrStValCtrDomainSt,
                 kIpcStValCtrDomainSt);
  REGISTER_STDEF(key_logical_port,
                 kIpcStrStKeyLogicalPort,
                 kIpcStKeyLogicalPort);
  REGISTER_STDEF(val_logical_port,
                 kIpcStrStValLogicalPort,
                 kIpcStValLogicalPort);
  REGISTER_STDEF(val_logical_port_st,
                 kIpcStrStValLogicalPortSt,
                 kIpcStValLogicalPortSt);
  REGISTER_STDEF(key_boundary,
                 kIpcStrStKeyBoundary,
                 kIpcStKeyBoundary);
  REGISTER_STDEF(val_boundary,
                 kIpcStrStValBoundary,
                 kIpcStValBoundary);
  REGISTER_STDEF(val_boundary_st,
                 kIpcStrStValBoundarySt,
                 kIpcStValBoundarySt);
  // Add Overlay Driver structures below
  REGISTER_STDEF(vnpdrv_val_vtunnel,
                 kIpcStrStVnpdrvValVtunnel,
                 kIpcStVnpdrvValVtunnel);
  REGISTER_STDEF(vnpdrv_val_vtunnel_if,
                 kIpcStrStVnpdrvValVtunnelIf,
                 kIpcStVnpdrvValVtunnelIf);
  REGISTER_STDEF(key_vtn_dataflow,
                 kIpcStrStKeyVtnDataflow,
                 kIpcStKeyVtnDataflow);
  // Expand structures
  REGISTER_STDEF(val_vbr_expand,
                 kIpcStrStValVbrExpand,
                 kIpcStValVbrExpand);
  REGISTER_STDEF(val_vbr_portmap_expand,
                 kIpcStrStValVbrPortMapExpand,
                 kIpcStValVbrPortMapExpand);
  REGISTER_STDEF(val_vbr_if_expand,
                 kIpcStrStValVbrIfExpand,
                 kIpcStValVbrIfExpand);
  REGISTER_STDEF(val_vtunnel_expand,
                 kIpcStrStValVtunnelExpand,
                 kIpcStValVtunnelExpand);
  REGISTER_STDEF(val_vtunnel_if_expand,
                 kIpcStrStValVtunnelIfExpand,
                 kIpcStValVtunnelIfExpand);
  REGISTER_STDEF(val_vlink_expand,
                 kIpcStrStValVlinkExpand,
                 kIpcStValVlinkExpand);
}

}  // namespace ipc_util
}  // namespace upll
}  // namespace unc
