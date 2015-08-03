/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef UPLL_IPCT_ST_HH_
#define UPLL_IPCT_ST_HH_

#include <string>
#include <map>
#include "convert_vnode.hh"
#include "pfc/ipc.h"
namespace unc {
namespace upll {
namespace ipc_util {
// using namespace unc::upll::kt_momgr::convert;
/**
 * Class IpctSt contains IPC_STRUCT information for structures used by UPLL
 *
 * Instructions to add an IPC_STRUCT
 * 0. Maintain the order of the objects in the following steps
 * 1. Add a structure name constant
 * 2. Add the structure in IpcStructNum enumeration
 * 3. Register the structure
 */

class IpctSt {
 public:
  // Define literals for all ipc_struct below
  static const char *kIpcStrStKeyRoot;
  static const char *kIpcStrStKeyUnifiedNw;
  static const char *kIpcStrStValUnifiedNw;
  static const char *kIpcStrStKeyUnwLabel;
  static const char *kIpcStrStValUnwLabel;
  static const char *kIpcStrStKeyUnwLabelRange;
  static const char *kIpcStrStValUnwLabelRange;
  static const char *kIpcStrStKeyUnwSpineDomain;
  static const char *kIpcStrStValUnwSpineDomain;
  static const char *kIpcStrStValUnwSpineDomain_Ext;
  static const char *kIpcStrStValSpineAlarmSt;
  static const char *kIpcStrStValUnwSpineDomainSt;
  static const char *kIpcStrStValUnwSpineDomainAssignedLabel;
  static const char *kIpcStrStValUnwSpineDomainFdbentry;
  static const char *kIpcStrStValUnwSpineDomainFdbentryVtn;
  static const char *kIpcStrStKeyVtnUnified;
  static const char *kIpcStrStValVtnUnified;
  static const char *kIpcStrStValPing;
  static const char *kIpcStrStValVtnNeighbor;
  static const char *kIpcStrStKeyVtn;
  static const char *kIpcStrStValVtn;
  static const char *kIpcStrStValRenameVtn;
  static const char *kIpcStrStValVtnSt;
  static const char *kIpcStrStKeyVtnController;
  static const char *kIpcStrStValVtnMappingControllerSt;
  static const char *kIpcStrStKeyVtnstationController;
  static const char *kIpcStrStValVtnstationControllerSt;
  static const char *kIpcStrStValVtnstationControllerStat;
  static const char *kIpcStrStKeyVbr;
  static const char *kIpcStrStValVbr;
  static const char *kIpcStrStKeyVbrPortMap;
  static const char *kIpcStrStValVbrPortMap;
  static const char *kIpcStrStValVbrPortMapSt;
  static const char *kIpcStrStValRenameVbr;
  static const char *kIpcStrStValVbrSt;
  static const char *kIpcStrStValVbrL2DomainSt;
  static const char *kIpcStrStValVbrL2DomainMemberSt;
  static const char *kIpcStrStValVbrMacEntrySt;
  static const char *kIpcStrStKeyConvertVbr;
  static const char *kIpcStrStValConvertVbr;
  static const char *kIpcStrStKeyVbrIf;
  static const char *kIpcStrStValPortMap;
  static const char *kIpcStrStValVbrIf;
  static const char *kIpcStrStValVbrIfSt;
  static const char *kIpcStrStKeyVlanMap;
  static const char *kIpcStrStValVlanMap;
  static const char *kIpcStrStKeyVrt;
  static const char *kIpcStrStValVrt;
  static const char *kIpcStrStValRenameVrt;
  static const char *kIpcStrStValVrtSt;
  static const char *kIpcStrStValVrtDhcpRelaySt;
  static const char *kIpcStrStValDhcpRelayIfSt;
  static const char *kIpcStrStValVrtArpEntrySt;
  static const char *kIpcStrStValVrtIpRouteSt;
  static const char *kIpcStrStKeyVrtIf;
  static const char *kIpcStrStValVrtIf;
  static const char *kIpcStrStValVrtIfSt;
  static const char *kIpcStrStKeyStaticIpRoute;
  static const char *kIpcStrStValStaticIpRoute;
  static const char *kIpcStrStKeyDhcpRelayIf;
  static const char *kIpcStrStValDhcpRelayIf;
  static const char *kIpcStrStKeyDhcpRelayServer;
  static const char *kIpcStrStValDhcpRelayServer;
  static const char *kIpcStrStKeyVterminal;
  static const char *kIpcStrStValVterminal;
  static const char *kIpcStrStValRenameVterminal;
  static const char *kIpcStrStValVterminalSt;
  static const char *kIpcStrStKeyVtermIf;
  static const char *kIpcStrStValVtermIf;
  static const char *kIpcStrStValVtermIfSt;
  static const char *kIpcStrStKeyNwm;
  static const char *kIpcStrStValNwm;
  static const char *kIpcStrStValNwmSt;
  static const char *kIpcStrStValNwmHostSt;
  static const char *kIpcStrStKeyNwmHost;
  static const char *kIpcStrStValNwmHost;
  static const char *kIpcStrStKeyVtep;
  static const char *kIpcStrStValVtep;
  static const char *kIpcStrStValVtepSt;
  static const char *kIpcStrStKeyVtepIf;
  static const char *kIpcStrStValVtepIf;
  static const char *kIpcStrStValVtepIfSt;
  static const char *kIpcStrStKeyVtepGrp;
  static const char *kIpcStrStValVtepGrp;
  static const char *kIpcStrStKeyVtepGrpMember;
  static const char *kIpcStrStValVtepGrpMember;
  static const char *kIpcStrStKeyVtunnel;
  static const char *kIpcStrStValVtunnel;
  static const char *kIpcStrStValVtunnelSt;
  static const char *kIpcStrStKeyVtunnelIf;
  static const char *kIpcStrStValVtunnelIf;
  static const char *kIpcStrStValVtunnelIfSt;
  static const char *kIpcStrStKeyVunknown;
  static const char *kIpcStrStValVunknown;
  static const char *kIpcStrStKeyVunkIf;
  static const char *kIpcStrStValVunkIf;
  static const char *kIpcStrStKeyVlink;
  static const char *kIpcStrStValVlink;
  static const char *kIpcStrStValVlinkSt;
  static const char *kIpcStrStValRenameVlink;
  static const char *kIpcStrStKeyFlowlist;
  static const char *kIpcStrStValFlowlist;
  static const char *kIpcStrStValRenameFlowlist;
  static const char *kIpcStrStKeyFlowlistEntry;
  static const char *kIpcStrStValFlowlistEntry;
  static const char *kIpcStrStValFlowlistEntrySt;
  static const char *kIpcStrStPomStats;
  static const char *kIpcStrStKeyVtnFlowfilter;
  static const char *kIpcStrStValFlowfilter;
  static const char *kIpcStrStKeyVtnFlowfilterEntry;
  static const char *kIpcStrStValVtnFlowfilterEntry;
  static const char *kIpcStrStValVtnFlowfilterControllerSt;
  static const char *kIpcStrStKeyVtnFlowfilterController;
  static const char *kIpcStrStValFlowfilterController;
  static const char *kIpcStrStKeyVbrFlowfilter;
  static const char *kIpcStrStKeyVbrFlowfilterEntry;
  static const char *kIpcStrStValFlowfilterEntry;
  static const char *kIpcStrStValFlowfilterEntrySt;
  static const char *kIpcStrStKeyVbrIfFlowfilter;
  static const char *kIpcStrStKeyVbrIfFlowfilterEntry;
  static const char *kIpcStrStKeyVrtIfFlowfilter;
  static const char *kIpcStrStKeyVrtIfFlowfilterEntry;
  static const char *kIpcStrStKeyVtermIfFlowfilter;
  static const char *kIpcStrStKeyVtermIfFlowfilterEntry;
  static const char *kIpcStrStKeyPolicingprofile;
  static const char *kIpcStrStValPolicingprofile;
  static const char *kIpcStrStValRenamePolicingprofile;
  static const char *kIpcStrStKeyPolicingprofileEntry;
  static const char *kIpcStrStValPolicingprofileEntry;
  static const char *kIpcStrStValPolicingmap;
  static const char *kIpcStrStValPolicingmapControllerSt;
  static const char *kIpcStrStValPolicingmapSwitchSt;
  static const char *kIpcStrStKeyVtnPolicingmapController;
  static const char *kIpcStrStValPolicingmapController;
  static const char *kIpcStrStKeyVbrPolicingmapEntry;
  static const char *kIpcStrStKeyVbrifPolicingmapEntry;
  static const char *kIpcStrStKeyVtermIfPolicingMapEntry;
  static const char *kIpcStrStKeyVtnDataflow;
  //  Convert structures
  static const char *kIpcStrStKeyConvertVbrIf;
  static const char *kIpcStrStValConvertVbrIf;
  static const char *kIpcStrStKeyConvertVlink;
  static const char *kIpcStrStValConvertVlink;
  // Expand structires
  static const char *kIpcStrStValVbrExpand;
  static const char *kIpcStrStValVbrPortMapExpand;
  static const char *kIpcStrStValVbrIfExpand;
  static const char *kIpcStrStValVtunnelExpand;
  static const char *kIpcStrStValVtunnelIfExpand;
  static const char *kIpcStrStValVlinkExpand;
  // Driver structures
  static const char *kIpcStrStPfcdrvValVbrIf;
  static const char *kIpcStrStPfcdrvValVbrifVextif;
  static const char *kIpcStrStPfcdrvValFlowfilterEntry;
  static const char *kIpcStrStPfcdrvValVbrifPolicingmap;
  static const char *kIpcStrStPfcdrvValVtnController;
  /* VlanmapOnBoundary: New val struct */
  static const char *kIpcStrStPfcdrvValVlanMap;
  static const char *kIpcStrStPfcdrvValVbrPortMap;

      // Physical strucures
  static const char *kIpcStrStKeyCtr;
  static const char *kIpcStrStValCtr;
  static const char *kIpcStrStValCtrSt;
  static const char *kIpcStrStKeyCtrDomain;
  static const char *kIpcStrStValCtrDomain;
  static const char *kIpcStrStValCtrDomainSt;
  static const char *kIpcStrStKeyLogicalPort;
  static const char *kIpcStrStValLogicalPort;
  static const char *kIpcStrStValLogicalPortSt;
  static const char *kIpcStrStKeyBoundary;
  static const char *kIpcStrStValBoundary;
  static const char *kIpcStrStValBoundarySt;
  static const char *kIpcStrStValPathFaultAlarm;
  // Overlay Driver structures
  static const char *kIpcStrStVnpdrvValVtunnel;
  static const char *kIpcStrStVnpdrvValVtunnelIf;
  static const char *kIpcStrStKeyConvertVtunnel;
  static const char *kIpcStrStValConvertVtunnel;
  static const char *kIpcStrStKeyConvertVtunnelIf;
  static const char *kIpcStrStValConvertVtunnelIf;
  static const char *kIpcStrStValVtnGatewayPort;

  enum IpcStructNum {
    kIpcInvalidStNum = 0,
    kIpcStInt8,
    kIpcStUint8,
    kIpcStInt16,
    kIpcStUint16,
    kIpcStInt32,
    kIpcStUint32,
    kIpcStInt64,
    kIpcStUint64,
    kIpcStFloat,
    kIpcStDouble,
    kIpcStIpv4,
    kIpcStIpv6,
    kIpcStString,
    kIpcStBinary,
    // Add any standard primary IPC types above this line
    // Below this line add all other ipc_structs
    kIpcStKeyRoot,
    kIpcStValPing,
    kIpcStValVtnNeighbor,
    kIpcStKeyVtn,
    kIpcStValVtn,
    kIpcStValRenameVtn,
    kIpcStValVtnSt,
    kIpcStKeyVtnController,
    kIpcStValVtnMappingControllerSt,
    kIpcStKeyVtnstationController,
    kIpcStValVtnstationControllerSt,
    kIpcStValVtnstationControllerStat,
    kIpcStKeyVbr,
    kIpcStValVbr,
    kIpcStKeyVbrPortMap,
    kIpcStValVbrPortMap,
    kIpcStValVbrPortMapSt,
    kIpcStValRenameVbr,
    kIpcStValVbrSt,
    kIpcStValVbrL2DomainSt,
    kIpcStValVbrL2DomainMemberSt,
    kIpcStValVbrMacEntrySt,
    kIpcStKeyVbrIf,
    kIpcStValPortMap,
    kIpcStValVbrIf,
    kIpcStValVbrIfSt,
    kIpcStKeyVlanMap,
    kIpcStValVlanMap,
    kIpcStKeyVrt,
    kIpcStValVrt,
    kIpcStValRenameVrt,
    kIpcStValVrtSt,
    kIpcStValVrtDhcpRelaySt,
    kIpcStValDhcpRelayIfSt,
    kIpcStValVrtArpEntrySt,
    kIpcStValVrtIpRouteSt,
    kIpcStKeyVrtIf,
    kIpcStValVrtIf,
    kIpcStValVrtIfSt,
    kIpcStKeyStaticIpRoute,
    kIpcStValStaticIpRoute,
    kIpcStKeyDhcpRelayIf,
    kIpcStValDhcpRelayIf,
    kIpcStKeyDhcpRelayServer,
    kIpcStValDhcpRelayServer,
    kIpcStKeyVterminal,
    kIpcStValVterminal,
    kIpcStValRenameVterminal,
    kIpcStValVterminalSt,
    kIpcStKeyVtermIf,
    kIpcStValVtermIf,
    kIpcStValVtermIfSt,
    kIpcStKeyNwm,
    kIpcStValNwm,
    kIpcStValNwmSt,
    kIpcStValNwmHostSt,
    kIpcStKeyNwmHost,
    kIpcStValNwmHost,
    kIpcStKeyVtep,
    kIpcStValVtep,
    kIpcStValVtepSt,
    kIpcStKeyVtepIf,
    kIpcStValVtepIf,
    kIpcStValVtepIfSt,
    kIpcStKeyVtepGrp,
    kIpcStValVtepGrp,
    kIpcStKeyVtepGrpMember,
    kIpcStValVtepGrpMember,
    kIpcStKeyVtunnel,
    kIpcStValVtunnel,
    kIpcStValVtunnelSt,
    kIpcStKeyVtunnelIf,
    kIpcStValVtunnelIf,
    kIpcStValVtunnelIfSt,
    kIpcStKeyVunknown,
    kIpcStValVunknown,
    kIpcStKeyVunkIf,
    kIpcStValVunkIf,
    kIpcStKeyVlink,
    kIpcStValVlink,
    kIpcStValVlinkSt,
    kIpcStValRenameVlink,
    kIpcStKeyFlowlist,
    kIpcStValFlowlist,
    kIpcStValRenameFlowlist,
    kIpcStKeyFlowlistEntry,
    kIpcStValFlowlistEntry,
    kIpcStValFlowlistEntrySt,
    kIpcStPomStats,
    kIpcStKeyVtnFlowfilter,
    kIpcStValFlowfilter,
    kIpcStKeyVtnFlowfilterEntry,
    kIpcStValVtnFlowfilterEntry,
    kIpcStValVtnFlowfilterControllerSt,
    kIpcStKeyVtnFlowfilterController,
    kIpcStValFlowfilterController,
    kIpcStKeyVbrFlowfilter,
    kIpcStKeyVbrFlowfilterEntry,
    kIpcStValFlowfilterEntry,
    kIpcStValFlowfilterEntrySt,
    kIpcStKeyVbrIfFlowfilter,
    kIpcStKeyVbrIfFlowfilterEntry,
    kIpcStKeyVrtIfFlowfilter,
    kIpcStKeyVrtIfFlowfilterEntry,
    kIpcStKeyVtermIfFlowfilter,
    kIpcStKeyVtermIfFlowfilterEntry,
    kIpcStKeyPolicingprofile,
    kIpcStValPolicingprofile,
    kIpcStValRenamePolicingprofile,
    kIpcStKeyPolicingprofileEntry,
    kIpcStValPolicingprofileEntry,
    kIpcStValPolicingmap,
    kIpcStValPolicingmapControllerSt,
    kIpcStValPolicingmapSwitchSt,
    kIpcStKeyVtnPolicingmapController,
    kIpcStValPolicingmapController,
    kIpcStKeyVbrPolicingmapEntry,
    kIpcStKeyVbrifPolicingmapEntry,
    kIpcStKeyVtermIfPolicingMapEntry,
    // convert structure
    kIpcStKeyConvertVbr,
    kIpcStValConvertVbr,
    kIpcStKeyConvertVbrIf,
    kIpcStValConvertVbrIf,
    kIpcStKeyConvertVtunnel,
    kIpcStValConvertVtunnel,
    kIpcStKeyConvertVtunnelIf,
    kIpcStValConvertVtunnelIf,
    kIpcStKeyConvertVlink,
    kIpcStValConvertVlink,
    kIpcStValVtnGatewayPort,
    // Expand structure
    kIpcStValVbrExpand,
    kIpcStValVbrPortMapExpand,
    kIpcStValVbrIfExpand,
    kIpcStValVtunnelExpand,
    kIpcStValVtunnelIfExpand,
    kIpcStValVlinkExpand,
    // Driver structures
    kIpcStPfcdrvValVbrIf,
    kIpcStPfcdrvValVbrifVextif,
    kIpcStPfcdrvValFlowfilterEntry,
    kIpcStPfcdrvValVbrifPolicingmap,
    /* VlanmapOnBoundary: New val struct */
    kIpcStPfcdrvValVlanMap,
    kIpcStPfcdrvValVbrPortMap,
    kIpcStPfcdrvValVtnController,
    /* label allocation structures*/
    kIpcStKeyVbid,
    kIpcStValVbid,
    kIpcStKeyGVtnId,
    kIpcStValGVtnId,

    // Physical structures
    kIpcStKeyCtr,
    kIpcStValCtr,
    kIpcStValCtrSt,
    kIpcStKeyCtrDomain,
    kIpcStValCtrDomain,
    kIpcStValCtrDomainSt,
    kIpcStKeyLogicalPort,
    kIpcStValLogicalPort,
    kIpcStValLogicalPortSt,
    kIpcStKeyBoundary,
    kIpcStValBoundary,
    kIpcStValBoundarySt,
    kIpcStValPathFaultAlarm,
    // Overlay Driver structures
    kIpcStVnpdrvValVtunnel,
    kIpcStVnpdrvValVtunnelIf,
    // Vtn DataFlow Structures
    kIpcStKeyVtnDataflow,
    // Unified Nw structures
    kIpcStKeyUnifiedNw,
    kIpcStValUnifiedNw,
    kIpcStKeyUnwLabel,
    kIpcStValUnwLabel,
    kIpcStKeyUnwLabelRange,
    kIpcStValUnwLabelRange,
    kIpcStKeyUnwSpineDomain,
    kIpcStValUnwSpineDomain,
    kIpctStValUnwSpineDomain_Ext,
    kIpctStValSpineAlarmSt,
    kIpcStValUnwSpineDomainSt,
    kIpcStValUnwSpineDomainAssignedLabel,
    kIpcStValUnwSpineDomainFdbentry,
    kIpcStValUnwSpineDomainFdbentryVtn,
    kIpcStKeyVtnUnified,
    kIpcStValVtnUnified
  };  // enum IpcStructNum

  static uint32_t Register(const char *stname, IpcStructNum stnum);
  static void RegisterAll();

  static IpcStructNum GetIpcStNum(const char *stname) {
    std::map<std::string, IpcStructNum>::iterator it =
      ipc_strname_to_stnum_map_.find(stname);
    if (it !=  ipc_strname_to_stnum_map_.end())
      return it->second;
    else
      return kIpcInvalidStNum;
  }
  static const pfc_ipcstdef_t *GetIpcStdef(const char *stname) {
    std::map<std::string, const pfc_ipcstdef_t*>::iterator it =
      ipc_stdef_smap_.find(stname);
    if (it != ipc_stdef_smap_.end())
      return it->second;
    else
      return NULL;
  }
  static const pfc_ipcstdef_t *GetIpcStdef(IpcStructNum st_num) {
    std::map<IpcStructNum, const pfc_ipcstdef_t*>::iterator it =
      ipc_stdef_nmap_.find(st_num);
    if (it != ipc_stdef_nmap_.end())
      return it->second;
    else
      return NULL;
  }

 private:
  static void RegisterIpcStdef(const char *stname,
                               IpcStructNum st_num,
                               const pfc_ipcstdef_t *stdef) {
    if (GetIpcStNum(stname) == kIpcInvalidStNum) {
      ipc_strname_to_stnum_map_[stname] = st_num;
      ipc_stdef_smap_[stname] = stdef;
      ipc_stdef_nmap_[st_num] = stdef;
    }
  }

  static std::map<std::string, const pfc_ipcstdef_t*> ipc_stdef_smap_;
  static std::map<IpcStructNum, const pfc_ipcstdef_t*> ipc_stdef_nmap_;
  static std::map<std::string, IpcStructNum> ipc_strname_to_stnum_map_;
};

}  // namespace ipc_util
}  // namespace upll
}  // namespace unc

#endif  // UPLL_IPCT_ST_HH_
