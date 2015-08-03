/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ctype.h>

#include <iomanip>
#include <sstream>

#include "uncxx/upll_log.hh"
#include "kt_util.hh"
#include "pfc/ipc.h"
namespace unc {
namespace upll {
namespace ipc_util {
// using namespace unc::upll::kt_momgr;
std::map<unc_key_type_t, KtUtil::KtMsgTemplate*> KtUtil::kt_msg_templates_;
KtUtil *KtUtil::singleton_instance_;

#define VALID_ARRAY_TO_STR(x) ValidArrayToStr((x), (sizeof(x)/sizeof((x)[0])))
#define CS_ARRAY_TO_STR(x) ConfigStatusToStr((x), (sizeof(x)/sizeof((x)[0])))

inline static std::string chr2int(uint8_t chr) {
  std::stringstream ss;
  ss << static_cast<int>(chr);
  if (isprint(chr)) {
    ss << " ('" << static_cast<char>(chr) << "')";
  }
  return ss.str();
}

using std::endl;

/**
 * @brief  Get the configuration message template. It includes all the optional
 *         elements too.
 *
 * @param kt[in]  Key type
 *
 * @return  configuration message template. If key type is not found, returns an
 *          empty template.
 */
const std::vector<IpctSt::IpcStructNum>& KtUtil::GetCfgMsgTemplate(
    unc_key_type_t kt) {
  UPLL_FUNC_TRACE;
  static std::vector<IpctSt::IpcStructNum> dummy;
  std::map<unc_key_type_t, KtMsgTemplate*>::iterator it
    = kt_msg_templates_.find(kt);
  if (it != kt_msg_templates_.end()) {
    return it->second->kt_cfg_msg;
  }
  return dummy;
}

/**
 * @brief Initialize KtUtil
 */
void KtUtil::Init() {
  KtMsgTemplate *tmpl;
  // UNC_KT_UNIFIED_NETWORK
  tmpl = new KtMsgTemplate();
  tmpl->kt_cfg_msg.push_back(IpctSt::kIpcStKeyUnifiedNw);
  tmpl->kt_cfg_msg.push_back(IpctSt::kIpcStValUnifiedNw);
  kt_msg_templates_[UNC_KT_UNIFIED_NETWORK] = tmpl;
  // UNC_KT_UNW_LABEL
  tmpl = new KtMsgTemplate();
  tmpl->kt_cfg_msg.push_back(IpctSt::kIpcStKeyUnwLabel);
  tmpl->kt_cfg_msg.push_back(IpctSt::kIpcStValUnwLabel);
  kt_msg_templates_[UNC_KT_UNW_LABEL] = tmpl;
  // UNC_KT_UNW_LABEL_RANGE
  tmpl = new KtMsgTemplate();
  tmpl->kt_cfg_msg.push_back(IpctSt::kIpcStKeyUnwLabelRange);
  // NO_VAL_STRUCT
  kt_msg_templates_[UNC_KT_UNW_LABEL_RANGE] = tmpl;
  // UNC_KT_UNW_SPINE_DOMAIN
  tmpl = new KtMsgTemplate();
  tmpl->kt_cfg_msg.push_back(IpctSt::kIpcStKeyUnwSpineDomain);
  tmpl->kt_cfg_msg.push_back(IpctSt::kIpcStValUnwSpineDomain);
  kt_msg_templates_[UNC_KT_UNW_SPINE_DOMAIN] = tmpl;
  // UNC_KT_VTN_UNIFIED
  tmpl = new KtMsgTemplate();
  tmpl->kt_cfg_msg.push_back(IpctSt::kIpcStKeyVtnUnified);
  tmpl->kt_cfg_msg.push_back(IpctSt::kIpcStValVtnUnified);
  kt_msg_templates_[UNC_KT_VTN_UNIFIED] = tmpl;
  // UNC_KT_FLOWLIST
  tmpl = new KtMsgTemplate();
  tmpl->kt_cfg_msg.push_back(IpctSt::kIpcStKeyFlowlist);
  tmpl->kt_cfg_msg.push_back(IpctSt::kIpcStValFlowlist);
  kt_msg_templates_[UNC_KT_FLOWLIST] = tmpl;
  // UNC_KT_FLOWLIST_ENTRY
  tmpl = new KtMsgTemplate();
  tmpl->kt_cfg_msg.push_back(IpctSt::kIpcStKeyFlowlistEntry);
  tmpl->kt_cfg_msg.push_back(IpctSt::kIpcStValFlowlistEntry);
  kt_msg_templates_[UNC_KT_FLOWLIST_ENTRY] = tmpl;
  // UNC_KT_POLICING_PROFILE
  tmpl = new KtMsgTemplate();
  tmpl->kt_cfg_msg.push_back(IpctSt::kIpcStKeyPolicingprofile);
  //                NO-VAL
  kt_msg_templates_[UNC_KT_POLICING_PROFILE] = tmpl;
  // UNC_KT_POLICING_PROFILE_ENTRY
  tmpl = new KtMsgTemplate();
  tmpl->kt_cfg_msg.push_back(IpctSt::kIpcStKeyPolicingprofileEntry);
  tmpl->kt_cfg_msg.push_back(IpctSt::kIpcStValPolicingprofileEntry);
  kt_msg_templates_[UNC_KT_POLICING_PROFILE_ENTRY] = tmpl;
  // UNC_KT_VTN
  tmpl = new KtMsgTemplate();
  tmpl->kt_cfg_msg.push_back(IpctSt::kIpcStKeyVtn);
  tmpl->kt_cfg_msg.push_back(IpctSt::kIpcStValVtn);
  kt_msg_templates_[UNC_KT_VTN] = tmpl;
  // UNC_KT_VTN_FLOWFILTER
  tmpl = new KtMsgTemplate();
  tmpl->kt_cfg_msg.push_back(IpctSt::kIpcStKeyVtnFlowfilter);
  //                NO-VAL
  kt_msg_templates_[UNC_KT_VTN_FLOWFILTER] = tmpl;
  // UNC_KT_VTN_FLOWFILTER_ENTRY
  tmpl = new KtMsgTemplate();
  tmpl->kt_cfg_msg.push_back(IpctSt::kIpcStKeyVtnFlowfilterEntry);
  tmpl->kt_cfg_msg.push_back(IpctSt::kIpcStValVtnFlowfilterEntry);
  kt_msg_templates_[UNC_KT_VTN_FLOWFILTER_ENTRY] = tmpl;
  // UNC_KT_VTN_POLICINGMAP
  tmpl = new KtMsgTemplate();
  tmpl->kt_cfg_msg.push_back(IpctSt::kIpcStKeyVtn);
  tmpl->kt_cfg_msg.push_back(IpctSt::kIpcStValPolicingmap);
  kt_msg_templates_[UNC_KT_VTN_POLICINGMAP] = tmpl;
  // UNC_KT_VBRIDGE
  tmpl = new KtMsgTemplate();
  tmpl->kt_cfg_msg.push_back(IpctSt::kIpcStKeyVbr);
  tmpl->kt_cfg_msg.push_back(IpctSt::kIpcStValVbr);
  kt_msg_templates_[UNC_KT_VBRIDGE] = tmpl;
  // UNC_KT_VBR_PORTMAP
  tmpl = new KtMsgTemplate();
  tmpl->kt_cfg_msg.push_back(IpctSt::kIpcStKeyVbrPortMap);
  tmpl->kt_cfg_msg.push_back(IpctSt::kIpcStPfcdrvValVbrPortMap);
  kt_msg_templates_[UNC_KT_VBR_PORTMAP] = tmpl;
  // UNC_KT_VBR_VLANMAP
  tmpl = new KtMsgTemplate();
  tmpl->kt_cfg_msg.push_back(IpctSt::kIpcStKeyVlanMap);
  /* VlanmapOnBoundary: Added new val struct */
  tmpl->kt_cfg_msg.push_back(IpctSt::kIpcStPfcdrvValVlanMap);
  kt_msg_templates_[UNC_KT_VBR_VLANMAP] = tmpl;
  // UNC_KT_VBR_NWMONITOR
  tmpl = new KtMsgTemplate();
  tmpl->kt_cfg_msg.push_back(IpctSt::kIpcStKeyNwm);
  tmpl->kt_cfg_msg.push_back(IpctSt::kIpcStValNwm);
  kt_msg_templates_[UNC_KT_VBR_NWMONITOR] = tmpl;
  // UNC_KT_VBR_NWMONITOR_HOST
  tmpl = new KtMsgTemplate();
  tmpl->kt_cfg_msg.push_back(IpctSt::kIpcStKeyNwmHost);
  tmpl->kt_cfg_msg.push_back(IpctSt::kIpcStValNwmHost);
  kt_msg_templates_[UNC_KT_VBR_NWMONITOR_HOST] = tmpl;
  // UNC_KT_VBR_POLICINGMAP
  tmpl = new KtMsgTemplate();
  tmpl->kt_cfg_msg.push_back(IpctSt::kIpcStKeyVbr);
  tmpl->kt_cfg_msg.push_back(IpctSt::kIpcStValPolicingmap);
  kt_msg_templates_[UNC_KT_VBR_POLICINGMAP] = tmpl;
  // UNC_KT_VBR_POLICINGMAP_ENTRY // only read
  // UNC_KT_VBR_FLOWFILTER
  tmpl = new KtMsgTemplate();
  tmpl->kt_cfg_msg.push_back(IpctSt::kIpcStKeyVbrFlowfilter);
  //                NO-VAL
  kt_msg_templates_[UNC_KT_VBR_FLOWFILTER] = tmpl;
  // UNC_KT_VBR_FLOWFILTER_ENTRY
  tmpl = new KtMsgTemplate();
  tmpl->kt_cfg_msg.push_back(IpctSt::kIpcStKeyVbrFlowfilterEntry);
  tmpl->kt_cfg_msg.push_back(IpctSt::kIpcStValFlowfilterEntry);
  kt_msg_templates_[UNC_KT_VBR_FLOWFILTER_ENTRY] = tmpl;
  // UNC_KT_VBR_IF
  tmpl = new KtMsgTemplate();
  tmpl->kt_cfg_msg.push_back(IpctSt::kIpcStKeyVbrIf);
  tmpl->kt_cfg_msg.push_back(IpctSt::kIpcStPfcdrvValVbrIf);
  kt_msg_templates_[UNC_KT_VBR_IF] = tmpl;
  // UNC_KT_VBRIF_FLOWFILTER
  tmpl = new KtMsgTemplate();
  tmpl->kt_cfg_msg.push_back(IpctSt::kIpcStKeyVbrIfFlowfilter);
  tmpl->kt_cfg_msg.push_back(IpctSt::kIpcStPfcdrvValVbrifVextif);
  kt_msg_templates_[UNC_KT_VBRIF_FLOWFILTER] = tmpl;
  // UNC_KT_VBRIF_FLOWFILTER_ENTRY
  tmpl = new KtMsgTemplate();
  tmpl->kt_cfg_msg.push_back(IpctSt::kIpcStKeyVbrIfFlowfilterEntry);
  tmpl->kt_cfg_msg.push_back(IpctSt::kIpcStPfcdrvValFlowfilterEntry);
  // can't do; TC does not support
  // tmpl->kt_cfg_msg.push_back(IpctSt::kIpcStPfcdrvValVbrifVextif);
  kt_msg_templates_[UNC_KT_VBRIF_FLOWFILTER_ENTRY] = tmpl;
  // UNC_KT_VBRIF_POLICINGMAP
  tmpl = new KtMsgTemplate();
  tmpl->kt_cfg_msg.push_back(IpctSt::kIpcStKeyVbrIf);
  tmpl->kt_cfg_msg.push_back(IpctSt::kIpcStPfcdrvValVbrifPolicingmap);
  // can't do; TC does not support
  // tmpl->kt_cfg_msg.push_back(IpctSt::kIpcStPfcdrvValVbrifVextif);
  kt_msg_templates_[UNC_KT_VBRIF_POLICINGMAP] = tmpl;
  // UNC_KT_VBRIF_POLICINGMAP_ENTRY // read only
  // UNC_KT_VROUTER
  tmpl = new KtMsgTemplate();
  tmpl->kt_cfg_msg.push_back(IpctSt::kIpcStKeyVrt);
  tmpl->kt_cfg_msg.push_back(IpctSt::kIpcStValVrt);
  kt_msg_templates_[UNC_KT_VROUTER] = tmpl;
  // UNC_KT_VRT_IPROUTE
  tmpl = new KtMsgTemplate();
  tmpl->kt_cfg_msg.push_back(IpctSt::kIpcStKeyStaticIpRoute);
  tmpl->kt_cfg_msg.push_back(IpctSt::kIpcStValStaticIpRoute);
  kt_msg_templates_[UNC_KT_VRT_IPROUTE] = tmpl;
  // UNC_KT_DHCPRELAY_SERVER
  tmpl = new KtMsgTemplate();
  tmpl->kt_cfg_msg.push_back(IpctSt::kIpcStKeyDhcpRelayServer);
  // NO - VAL
  // tmpl->kt_cfg_msg.push_back(IpctSt::kIpcStValDhcpRelayServer);
  kt_msg_templates_[UNC_KT_DHCPRELAY_SERVER] = tmpl;
  // UNC_KT_DHCPRELAY_IF
  tmpl = new KtMsgTemplate();
  tmpl->kt_cfg_msg.push_back(IpctSt::kIpcStKeyDhcpRelayIf);
  // NO - VAL
  // tmpl->kt_cfg_msg.push_back(IpctSt::kIpcStValDhcpRelayIf);
  kt_msg_templates_[UNC_KT_DHCPRELAY_IF] = tmpl;
  // UNC_KT_VRT_IF
  tmpl = new KtMsgTemplate();
  tmpl->kt_cfg_msg.push_back(IpctSt::kIpcStKeyVrtIf);
  tmpl->kt_cfg_msg.push_back(IpctSt::kIpcStValVrtIf);
  kt_msg_templates_[UNC_KT_VRT_IF] = tmpl;
  // UNC_KT_VRTIF_FLOWFILTER
  tmpl = new KtMsgTemplate();
  tmpl->kt_cfg_msg.push_back(IpctSt::kIpcStKeyVrtIfFlowfilter);
  tmpl->kt_cfg_msg.push_back(IpctSt::kIpcStPfcdrvValVbrifVextif);
  kt_msg_templates_[UNC_KT_VRTIF_FLOWFILTER] = tmpl;
  // UNC_KT_VRTIF_FLOWFILTER_ENTRY
  tmpl = new KtMsgTemplate();
  tmpl->kt_cfg_msg.push_back(IpctSt::kIpcStKeyVrtIfFlowfilterEntry);
  tmpl->kt_cfg_msg.push_back(IpctSt::kIpcStPfcdrvValFlowfilterEntry);
  kt_msg_templates_[UNC_KT_VRTIF_FLOWFILTER_ENTRY] = tmpl;
  // UNC_KT_VTERMINAL
  tmpl = new KtMsgTemplate();
  tmpl->kt_cfg_msg.push_back(IpctSt::kIpcStKeyVterminal);
  tmpl->kt_cfg_msg.push_back(IpctSt::kIpcStValVterminal);
  kt_msg_templates_[UNC_KT_VTERMINAL] = tmpl;
  // UNC_KT_VTERM_IF
  tmpl = new KtMsgTemplate();
  tmpl->kt_cfg_msg.push_back(IpctSt::kIpcStKeyVtermIf);
  tmpl->kt_cfg_msg.push_back(IpctSt::kIpcStValVtermIf);
  kt_msg_templates_[UNC_KT_VTERM_IF] = tmpl;
  // UNC_KT_VTERMIF_POLICINGMAP
  tmpl = new KtMsgTemplate();
  tmpl->kt_cfg_msg.push_back(IpctSt::kIpcStKeyVtermIf);
  tmpl->kt_cfg_msg.push_back(IpctSt::kIpcStValPolicingmap);
  // can't do; TC does not support
  // tmpl->kt_cfg_msg.push_back(IpctSt::kIpcStPfcdrvValVbrifVextif);
  kt_msg_templates_[UNC_KT_VTERMIF_POLICINGMAP] = tmpl;
  // UNC_KT_VTERMIF_POLICINGMAP_ENTRY // read only

  // UNC_KT_VTERM_IF_FLOWFILTER
  tmpl = new KtMsgTemplate();
  tmpl->kt_cfg_msg.push_back(IpctSt::kIpcStKeyVtermIfFlowfilter);
  kt_msg_templates_[UNC_KT_VTERMIF_FLOWFILTER] = tmpl;

  // UNC_KT_VTERMIF_FLOWFILTER_ENTRY
  tmpl = new KtMsgTemplate();
  tmpl->kt_cfg_msg.push_back(IpctSt::kIpcStKeyVtermIfFlowfilterEntry);
  tmpl->kt_cfg_msg.push_back(IpctSt::kIpcStValFlowfilterEntry);
  kt_msg_templates_[UNC_KT_VTERMIF_FLOWFILTER_ENTRY] = tmpl;

  // UNC_KT_VUNKNOWN
  tmpl = new KtMsgTemplate();
  tmpl->kt_cfg_msg.push_back(IpctSt::kIpcStKeyVunknown);
  tmpl->kt_cfg_msg.push_back(IpctSt::kIpcStValVunknown);
  kt_msg_templates_[UNC_KT_VUNKNOWN] = tmpl;
  // UNC_KT_VUNK_IF
  tmpl = new KtMsgTemplate();
  tmpl->kt_cfg_msg.push_back(IpctSt::kIpcStKeyVunkIf);
  tmpl->kt_cfg_msg.push_back(IpctSt::kIpcStValVunkIf);
  kt_msg_templates_[UNC_KT_VUNK_IF] = tmpl;
  // UNC_KT_VTEP
  tmpl = new KtMsgTemplate();
  tmpl->kt_cfg_msg.push_back(IpctSt::kIpcStKeyVtep);
  tmpl->kt_cfg_msg.push_back(IpctSt::kIpcStValVtep);
  kt_msg_templates_[UNC_KT_VTEP] = tmpl;
  // UNC_KT_VTEP_IF
  tmpl = new KtMsgTemplate();
  tmpl->kt_cfg_msg.push_back(IpctSt::kIpcStKeyVtepIf);
  tmpl->kt_cfg_msg.push_back(IpctSt::kIpcStValVtepIf);
  kt_msg_templates_[UNC_KT_VTEP_IF] = tmpl;
  // UNC_KT_VTEP_GRP
  tmpl = new KtMsgTemplate();
  tmpl->kt_cfg_msg.push_back(IpctSt::kIpcStKeyVtepGrp);
  tmpl->kt_cfg_msg.push_back(IpctSt::kIpcStValVtepGrp);
  kt_msg_templates_[UNC_KT_VTEP_GRP] = tmpl;
  // UNC_KT_VTEP_GRP_MEMBER
  tmpl = new KtMsgTemplate();
  tmpl->kt_cfg_msg.push_back(IpctSt::kIpcStKeyVtepGrpMember);
  tmpl->kt_cfg_msg.push_back(IpctSt::kIpcStValVtepGrpMember);
  kt_msg_templates_[UNC_KT_VTEP_GRP_MEMBER] = tmpl;
  // UNC_KT_VTUNNEL
  tmpl = new KtMsgTemplate();
  tmpl->kt_cfg_msg.push_back(IpctSt::kIpcStKeyVtunnel);
  tmpl->kt_cfg_msg.push_back(IpctSt::kIpcStValVtunnel);
  kt_msg_templates_[UNC_KT_VTUNNEL] = tmpl;
  // UNC_KT_VTUNNEL_IF
  tmpl = new KtMsgTemplate();
  tmpl->kt_cfg_msg.push_back(IpctSt::kIpcStKeyVtunnelIf);
  tmpl->kt_cfg_msg.push_back(IpctSt::kIpcStValVtunnelIf);
  kt_msg_templates_[UNC_KT_VTUNNEL_IF] = tmpl;
  // UNC_KT_VLINK
  tmpl = new KtMsgTemplate();
  tmpl->kt_cfg_msg.push_back(IpctSt::kIpcStKeyVlink);
  tmpl->kt_cfg_msg.push_back(IpctSt::kIpcStValVlink);
  kt_msg_templates_[UNC_KT_VLINK] = tmpl;
  // UNC_KT_VTN_DATAFLOW
  tmpl = new KtMsgTemplate();
  tmpl->kt_cfg_msg.push_back(IpctSt::kIpcStKeyVtnDataflow);
  kt_msg_templates_[UNC_KT_VTN_DATAFLOW] = tmpl;
}


/**
 * @brief  Given an IPC structure number and pointer to its data, get the
 *         printable string representation for the data.
 *
 * @param stnum
 * @param data
 *
 * @return
 */
std::string KtUtil::KtStructToStr(IpctSt::IpcStructNum stnum, void *data) {
  std::stringstream ss;
  if (data == NULL) {
    return "<null>";
  }
  switch (stnum) {
    case IpctSt::kIpcInvalidStNum:
      return "invalid_st_num";
    case IpctSt::kIpcStInt8:
      ss << "   -->int8 " << *(reinterpret_cast<int8_t*>(data));
      return ss.str();
    case IpctSt::kIpcStUint8:
      ss << "   -->uint8 " << *(reinterpret_cast<uint8_t*>(data));
      return ss.str();
    case IpctSt::kIpcStInt16:
      ss << "   -->int16 " << *(reinterpret_cast<int16_t*>(data));
      return ss.str();
    case IpctSt::kIpcStUint16:
      ss << "   -->uint2 " << *(reinterpret_cast<uint16_t*>(data));
      return ss.str();
    case IpctSt::kIpcStInt32:
      ss << "   -->int32 " << *(reinterpret_cast<int32_t*>(data));
      return ss.str();
    case IpctSt::kIpcStUint32:
      ss << "   -->uint32 " << *(reinterpret_cast<uint32_t*>(data));
      return ss.str();
    case IpctSt::kIpcStInt64:
      ss << "   -->int64 " << *(reinterpret_cast<int64_t*>(data));
      return ss.str();
    case IpctSt::kIpcStUint64:
      ss << "   -->uint64 " << *(reinterpret_cast<uint64_t*>(data));
      return ss.str();
    case IpctSt::kIpcStFloat:
      ss << "   -->float " << *(reinterpret_cast<float*>(data));
      return ss.str();
    case IpctSt::kIpcStDouble:
      ss << "   -->double " << *(reinterpret_cast<double*>(data));
      return ss.str();
    case IpctSt::kIpcStIpv4:
      ss << "   -->ipv4addr "
          << Ipv4AddrToStr(reinterpret_cast<struct in_addr*>(data)->s_addr);
      return ss.str();
    case IpctSt::kIpcStIpv6:
      ss << "   -->ipv6addr "
          << Ipv6AddrToStr(*reinterpret_cast<struct in6_addr*>(data));
      return ss.str();
    case IpctSt::kIpcStString:
      ss << "   -->string " << (reinterpret_cast<char*>(data));
      return ss.str();
    case IpctSt::kIpcStBinary:
      /*
         ss << *((uint8_t*)data);
         return ss.str();
         */
      return "kIpcStBinary not implemented";
      // Add any standard primary IPC types above this line
      // Below this line add all other ipc_structs
    case IpctSt::kIpcStKeyRoot:
      return "ROOT";
    case IpctSt::kIpcStKeyUnifiedNw:
      return IpcStructToStr(*reinterpret_cast<key_unified_nw*>(data));
    case IpctSt::kIpcStValUnifiedNw:
      return IpcStructToStr(*reinterpret_cast<val_unified_nw*>(data));
    case IpctSt::kIpcStKeyUnwLabel:
      return IpcStructToStr(*reinterpret_cast<key_unw_label*>(data));
    case IpctSt::kIpcStValUnwLabel:
      return IpcStructToStr(*reinterpret_cast<val_unw_label*>(data));
    case IpctSt::kIpcStKeyUnwLabelRange:
      return IpcStructToStr(*reinterpret_cast<key_unw_label_range*>(data));
    case IpctSt::kIpcStValUnwLabelRange:
      return IpcStructToStr(*reinterpret_cast<val_unw_label_range*>(data));
    case IpctSt::kIpcStKeyUnwSpineDomain:
      return IpcStructToStr(*reinterpret_cast<key_unw_spine_domain*>(data));
    case IpctSt::kIpcStValUnwSpineDomain:
      return IpcStructToStr(*reinterpret_cast<val_unw_spine_domain*>(data));
    case IpctSt::kIpctStValUnwSpineDomain_Ext:
      return IpcStructToStr(*reinterpret_cast<val_unw_spdom_ext*>(data));
    case IpctSt::kIpctStValSpineAlarmSt:
      return IpcStructToStr(*reinterpret_cast<val_spdom_st*>(data));
    case IpctSt::kIpcStValUnwSpineDomainSt:
      return IpcStructToStr(*reinterpret_cast<val_unw_spine_domain_st*>(data));
    case IpctSt::kIpcStValUnwSpineDomainAssignedLabel:
      return IpcStructToStr(
          *reinterpret_cast<val_unw_spine_domain_assigned_label*>(data));
    case IpctSt::kIpcStValUnwSpineDomainFdbentry:
      return IpcStructToStr(
          *reinterpret_cast<val_unw_spine_domain_fdbentry*>(data));
    case IpctSt::kIpcStValUnwSpineDomainFdbentryVtn:
      return IpcStructToStr(
          *reinterpret_cast<val_unw_spine_domain_fdbentry_vtn*>(data));
    case IpctSt::kIpcStKeyVtnUnified:
      return IpcStructToStr(*reinterpret_cast<key_vtn_unified*>(data));
    case IpctSt::kIpcStValVtnUnified:
      return IpcStructToStr(*reinterpret_cast<val_vtn_unified*>(data));
    case IpctSt::kIpcStValPing:
      return IpcStructToStr(*reinterpret_cast<val_ping*>(data));
    case IpctSt::kIpcStValVtnNeighbor:
      return IpcStructToStr(*reinterpret_cast<val_vtn_neighbor*>(data));
    case IpctSt::kIpcStKeyVtn:
      return IpcStructToStr(*reinterpret_cast<key_vtn*>(data));
    case IpctSt::kIpcStValVtn:
      return IpcStructToStr(*reinterpret_cast<val_vtn*>(data));
    case IpctSt::kIpcStValRenameVtn:
      return IpcStructToStr(*reinterpret_cast<val_rename_vtn*>(data));
    case IpctSt::kIpcStValVtnSt:
      return IpcStructToStr(*reinterpret_cast<val_vtn_st*>(data));
    case IpctSt::kIpcStKeyVtnController:
      return IpcStructToStr(*reinterpret_cast<key_vtn_controller*>(data));
    case IpctSt::kIpcStValVtnMappingControllerSt:
      return IpcStructToStr(
          *reinterpret_cast<val_vtn_mapping_controller_st*>(data));
    case IpctSt::kIpcStKeyVtnstationController:
      return IpcStructToStr(
          *reinterpret_cast<key_vtnstation_controller*>(data));
    case IpctSt::kIpcStValVtnstationControllerSt:
      return IpcStructToStr(
          *reinterpret_cast<val_vtnstation_controller_st*>(data));
    case IpctSt::kIpcStValVtnstationControllerStat:
      return IpcStructToStr(
          *reinterpret_cast<val_vtnstation_controller_stat*>(data));
    case IpctSt::kIpcStKeyVbr:
      return IpcStructToStr(*reinterpret_cast<key_vbr*>(data));
    case IpctSt::kIpcStValVbr:
      return IpcStructToStr(*reinterpret_cast<val_vbr*>(data));
    case IpctSt::kIpcStValRenameVbr:
      return IpcStructToStr(*reinterpret_cast<val_rename_vbr*>(data));
    case IpctSt::kIpcStValVbrSt:
      return IpcStructToStr(*reinterpret_cast<val_vbr_st*>(data));
    case IpctSt::kIpcStValVbrL2DomainSt:
      return IpcStructToStr(*reinterpret_cast<val_vbr_l2_domain_st*>(data));
    case IpctSt::kIpcStValVbrL2DomainMemberSt:
      return IpcStructToStr(
          *reinterpret_cast<val_vbr_l2_domain_member_st*>(data));
    case IpctSt::kIpcStValVbrMacEntrySt:
      return IpcStructToStr(*reinterpret_cast<val_vbr_mac_entry_st*>(data));
    case IpctSt::kIpcStKeyVbrIf:
      return IpcStructToStr(*reinterpret_cast<key_vbr_if*>(data));
    case IpctSt::kIpcStValPortMap:
      return IpcStructToStr(*reinterpret_cast<val_port_map*>(data));
    case IpctSt::kIpcStValVbrIf:
      return IpcStructToStr(*reinterpret_cast<val_vbr_if*>(data));
    case IpctSt::kIpcStValVbrIfSt:
      return IpcStructToStr(*reinterpret_cast<val_vbr_if_st*>(data));
    case IpctSt::kIpcStKeyVlanMap:
      return IpcStructToStr(*reinterpret_cast<key_vlan_map*>(data));
    case IpctSt::kIpcStValVlanMap:
      return IpcStructToStr(*reinterpret_cast<val_vlan_map*>(data));
    case IpctSt::kIpcStKeyVbrPortMap:
      return IpcStructToStr(*reinterpret_cast<key_vbr_portmap*>(data));
    case IpctSt::kIpcStValVbrPortMap:
      return IpcStructToStr(*reinterpret_cast<val_vbr_portmap*>(data));
    case IpctSt::kIpcStValVbrPortMapSt:
      return IpcStructToStr(*reinterpret_cast<val_vbr_portmap_st*>(data));
    case IpctSt::kIpcStKeyVrt:
      return IpcStructToStr(*reinterpret_cast<key_vrt*>(data));
    case IpctSt::kIpcStValVrt:
      return IpcStructToStr(*reinterpret_cast<val_vrt*>(data));
    case IpctSt::kIpcStValRenameVrt:
      return IpcStructToStr(*reinterpret_cast<val_rename_vrt*>(data));
    case IpctSt::kIpcStValVrtSt:
      return IpcStructToStr(*reinterpret_cast<val_vrt_st*>(data));
    case IpctSt::kIpcStValVrtDhcpRelaySt:
      return IpcStructToStr(*reinterpret_cast<val_vrt_dhcp_relay_st*>(data));
    case IpctSt::kIpcStValDhcpRelayIfSt:
      return IpcStructToStr(*reinterpret_cast<val_dhcp_relay_if_st*>(data));
    case IpctSt::kIpcStValVrtArpEntrySt:
      return IpcStructToStr(*reinterpret_cast<val_vrt_arp_entry_st*>(data));
    case IpctSt::kIpcStValVrtIpRouteSt:
      return IpcStructToStr(*reinterpret_cast<val_vrt_ip_route_st*>(data));
    case IpctSt::kIpcStKeyVrtIf:
      return IpcStructToStr(*reinterpret_cast<key_vrt_if*>(data));
    case IpctSt::kIpcStValVrtIf:
      return IpcStructToStr(*reinterpret_cast<val_vrt_if*>(data));
    case IpctSt::kIpcStValVrtIfSt:
      return IpcStructToStr(*reinterpret_cast<val_vrt_if_st*>(data));
    case IpctSt::kIpcStKeyStaticIpRoute:
      return IpcStructToStr(*reinterpret_cast<key_static_ip_route*>(data));
    case IpctSt::kIpcStValStaticIpRoute:
      return IpcStructToStr(*reinterpret_cast<val_static_ip_route*>(data));
    case IpctSt::kIpcStKeyDhcpRelayIf:
      return IpcStructToStr(*reinterpret_cast<key_dhcp_relay_if*>(data));
    case IpctSt::kIpcStValDhcpRelayIf:
      return IpcStructToStr(*reinterpret_cast<val_dhcp_relay_if*>(data));
    case IpctSt::kIpcStKeyDhcpRelayServer:
      return IpcStructToStr(*reinterpret_cast<key_dhcp_relay_server*>(data));
    case IpctSt::kIpcStValDhcpRelayServer:
      return IpcStructToStr(*reinterpret_cast<val_dhcp_relay_server*>(data));
    case IpctSt::kIpcStKeyVterminal:
      return IpcStructToStr(*reinterpret_cast<key_vterm*>(data));
    case IpctSt::kIpcStValVterminal:
      return IpcStructToStr(*reinterpret_cast<val_vterm*>(data));
    case IpctSt::kIpcStValRenameVterminal:
      return IpcStructToStr(*reinterpret_cast<val_rename_vterm*>(data));
    case IpctSt::kIpcStValVterminalSt:
      return IpcStructToStr(*reinterpret_cast<val_vterm_st*>(data));
    case IpctSt::kIpcStKeyVtermIf:
      return IpcStructToStr(*reinterpret_cast<key_vterm_if*>(data));
    case IpctSt::kIpcStValVtermIf:
      return IpcStructToStr(*reinterpret_cast<val_vterm_if*>(data));
    case IpctSt::kIpcStValVtermIfSt:
      return IpcStructToStr(*reinterpret_cast<val_vterm_if_st*>(data));
    case IpctSt::kIpcStKeyNwm:
      return IpcStructToStr(*reinterpret_cast<key_nwm*>(data));
    case IpctSt::kIpcStValNwm:
      return IpcStructToStr(*reinterpret_cast<val_nwm*>(data));
    case IpctSt::kIpcStValNwmSt:
      return IpcStructToStr(*reinterpret_cast<val_nwm_st*>(data));
    case IpctSt::kIpcStValNwmHostSt:
      return IpcStructToStr(*reinterpret_cast<val_nwm_host_st*>(data));
    case IpctSt::kIpcStKeyNwmHost:
      return IpcStructToStr(*reinterpret_cast<key_nwm_host*>(data));
    case IpctSt::kIpcStValNwmHost:
      return IpcStructToStr(*reinterpret_cast<val_nwm_host*>(data));
    case IpctSt::kIpcStKeyVtep:
      return IpcStructToStr(*reinterpret_cast<key_vtep*>(data));
    case IpctSt::kIpcStValVtep:
      return IpcStructToStr(*reinterpret_cast<val_vtep*>(data));
    case IpctSt::kIpcStValVtepSt:
      return IpcStructToStr(*reinterpret_cast<val_vtep_st*>(data));
    case IpctSt::kIpcStKeyVtepIf:
      return IpcStructToStr(*reinterpret_cast<key_vtep_if*>(data));
    case IpctSt::kIpcStValVtepIf:
      return IpcStructToStr(*reinterpret_cast<val_vtep_if*>(data));
    case IpctSt::kIpcStValVtepIfSt:
      return IpcStructToStr(*reinterpret_cast<val_vtep_if_st*>(data));
    case IpctSt::kIpcStKeyVtepGrp:
      return IpcStructToStr(*reinterpret_cast<key_vtep_grp*>(data));
    case IpctSt::kIpcStValVtepGrp:
      return IpcStructToStr(*reinterpret_cast<val_vtep_grp*>(data));
    case IpctSt::kIpcStKeyVtepGrpMember:
      return IpcStructToStr(*reinterpret_cast<key_vtep_grp_member*>(data));
    case IpctSt::kIpcStValVtepGrpMember:
      return IpcStructToStr(*reinterpret_cast<val_vtep_grp_member*>(data));
    case IpctSt::kIpcStKeyVtunnel:
      return IpcStructToStr(*reinterpret_cast<key_vtunnel*>(data));
    case IpctSt::kIpcStValVtunnel:
      return IpcStructToStr(*reinterpret_cast<val_vtunnel*>(data));
    case IpctSt::kIpcStValVtunnelSt:
      return IpcStructToStr(*reinterpret_cast<val_vtunnel_st*>(data));
    case IpctSt::kIpcStKeyVtunnelIf:
      return IpcStructToStr(*reinterpret_cast<key_vtunnel_if*>(data));
    case IpctSt::kIpcStValVtunnelIf:
      return IpcStructToStr(*reinterpret_cast<val_vtunnel_if*>(data));
    case IpctSt::kIpcStValVtunnelIfSt:
      return IpcStructToStr(*reinterpret_cast<val_vtunnel_if_st*>(data));
    case IpctSt::kIpcStKeyVunknown:
      return IpcStructToStr(*reinterpret_cast<key_vunknown*>(data));
    case IpctSt::kIpcStValVunknown:
      return IpcStructToStr(*reinterpret_cast<val_vunknown*>(data));
    case IpctSt::kIpcStKeyVunkIf:
      return IpcStructToStr(*reinterpret_cast<key_vunk_if*>(data));
    case IpctSt::kIpcStValVunkIf:
      return IpcStructToStr(*reinterpret_cast<val_vunk_if*>(data));
    case IpctSt::kIpcStKeyVlink:
      return IpcStructToStr(*reinterpret_cast<key_vlink*>(data));
    case IpctSt::kIpcStValVlink:
      return IpcStructToStr(*reinterpret_cast<val_vlink*>(data));
    case IpctSt::kIpcStValVlinkSt:
      return IpcStructToStr(*reinterpret_cast<val_vlink_st*>(data));
    case IpctSt::kIpcStValRenameVlink:
      return IpcStructToStr(*reinterpret_cast<val_rename_vlink*>(data));
    case IpctSt::kIpcStKeyFlowlist:
      return IpcStructToStr(*reinterpret_cast<key_flowlist*>(data));
    case IpctSt::kIpcStValFlowlist:
      return IpcStructToStr(*reinterpret_cast<val_flowlist*>(data));
    case IpctSt::kIpcStValRenameFlowlist:
      return IpcStructToStr(*reinterpret_cast<val_rename_flowlist*>(data));
    case IpctSt::kIpcStKeyFlowlistEntry:
      return IpcStructToStr(*reinterpret_cast<key_flowlist_entry*>(data));
    case IpctSt::kIpcStValFlowlistEntry:
      return IpcStructToStr(*reinterpret_cast<val_flowlist_entry*>(data));
    case IpctSt::kIpcStValFlowlistEntrySt:
      return IpcStructToStr(*reinterpret_cast<val_flowlist_entry_st*>(data));
    case IpctSt::kIpcStPomStats:
      return IpcStructToStr(*reinterpret_cast<pom_stats*>(data));
    case IpctSt::kIpcStKeyVtnFlowfilter:
      return IpcStructToStr(*reinterpret_cast<key_vtn_flowfilter*>(data));
    case IpctSt::kIpcStValFlowfilter:
      return IpcStructToStr(*reinterpret_cast<val_flowfilter*>(data));
    case IpctSt::kIpcStKeyVtnFlowfilterEntry:
      return IpcStructToStr(*reinterpret_cast<key_vtn_flowfilter_entry*>(data));
    case IpctSt::kIpcStValVtnFlowfilterEntry:
      return IpcStructToStr(*reinterpret_cast<val_vtn_flowfilter_entry*>(data));
    case IpctSt::kIpcStValVtnFlowfilterControllerSt:
      return IpcStructToStr(
          *reinterpret_cast<val_vtn_flowfilter_controller_st*>(data));
    case IpctSt::kIpcStKeyVtnFlowfilterController:
      return IpcStructToStr(
          *reinterpret_cast<key_vtn_flowfilter_controller*>(data));
    case IpctSt::kIpcStValFlowfilterController:
      return IpcStructToStr(
          *reinterpret_cast<val_flowfilter_controller*>(data));
    case IpctSt::kIpcStKeyVbrFlowfilter:
      return IpcStructToStr(*reinterpret_cast<key_vbr_flowfilter*>(data));
    case IpctSt::kIpcStKeyVbrFlowfilterEntry:
      return IpcStructToStr(*reinterpret_cast<key_vbr_flowfilter_entry*>(data));
    case IpctSt::kIpcStValFlowfilterEntry:
      return IpcStructToStr(*reinterpret_cast<val_flowfilter_entry*>(data));
    case IpctSt::kIpcStValFlowfilterEntrySt:
      return IpcStructToStr(*reinterpret_cast<val_flowfilter_entry_st*>(data));
    case IpctSt::kIpcStKeyVbrIfFlowfilter:
      return IpcStructToStr(*reinterpret_cast<key_vbr_if_flowfilter*>(data));
    case IpctSt::kIpcStKeyVbrIfFlowfilterEntry:
      return IpcStructToStr(
          *reinterpret_cast<key_vbr_if_flowfilter_entry*>(data));
    case IpctSt::kIpcStKeyVrtIfFlowfilter:
      return IpcStructToStr(*reinterpret_cast<key_vrt_if_flowfilter*>(data));
    case IpctSt::kIpcStKeyVrtIfFlowfilterEntry:
      return IpcStructToStr(
          *reinterpret_cast<key_vrt_if_flowfilter_entry*>(data));
    case IpctSt::kIpcStKeyPolicingprofile:
      return IpcStructToStr(*reinterpret_cast<key_policingprofile*>(data));
    case IpctSt::kIpcStValPolicingprofile:
      return IpcStructToStr(*reinterpret_cast<val_policingprofile*>(data));
    case IpctSt::kIpcStValRenamePolicingprofile:
      return IpcStructToStr(
          *reinterpret_cast<val_rename_policingprofile*>(data));
    case IpctSt::kIpcStKeyPolicingprofileEntry:
      return IpcStructToStr(
          *reinterpret_cast<key_policingprofile_entry*>(data));
    case IpctSt::kIpcStValPolicingprofileEntry:
      return IpcStructToStr(
          *reinterpret_cast<val_policingprofile_entry*>(data));
    case IpctSt::kIpcStValPolicingmap:
      return IpcStructToStr(*reinterpret_cast<val_policingmap*>(data));
    case IpctSt::kIpcStValPolicingmapControllerSt:
      return IpcStructToStr(
          *reinterpret_cast<val_policingmap_controller_st*>(data));
    case IpctSt::kIpcStValPolicingmapSwitchSt:
      return IpcStructToStr(
          *reinterpret_cast<val_policingmap_switch_st*>(data));
    case IpctSt::kIpcStKeyVtnPolicingmapController:
      return IpcStructToStr(
          *reinterpret_cast<key_vtn_policingmap_controller*>(data));
    case IpctSt::kIpcStValPolicingmapController:
      return IpcStructToStr(
          *reinterpret_cast<val_policingmap_controller*>(data));
    case IpctSt::kIpcStKeyVbrPolicingmapEntry:
      return IpcStructToStr(
          *reinterpret_cast<key_vbr_policingmap_entry*>(data));
    case IpctSt::kIpcStKeyVbrifPolicingmapEntry:
      return IpcStructToStr(
          *reinterpret_cast<key_vbrif_policingmap_entry*>(data));
    case IpctSt::kIpcStKeyVtermIfPolicingMapEntry:
      return IpcStructToStr(
          *reinterpret_cast<key_vtermif_policingmap_entry*>(data));
    case IpctSt::kIpcStKeyVtermIfFlowfilter:
      return IpcStructToStr(*reinterpret_cast<key_vterm_if_flowfilter*>(data));
    case IpctSt::kIpcStKeyVtermIfFlowfilterEntry:
      return IpcStructToStr(
          *reinterpret_cast<key_vterm_if_flowfilter_entry*>(data));
    // Convert structures
    case IpctSt::kIpcStKeyConvertVbr:
      return IpcStructToStr(*reinterpret_cast<key_convert_vbr*>(data));
    case IpctSt::kIpcStValConvertVbr:
      return IpcStructToStr(*reinterpret_cast<val_convert_vbr*>(data));
    case IpctSt::kIpcStKeyConvertVbrIf:
      return IpcStructToStr(*reinterpret_cast<key_convert_vbr_if*>(data));
    case IpctSt::kIpcStValConvertVbrIf:
      return IpcStructToStr(*reinterpret_cast<val_convert_vbr_if*>(data));
    case IpctSt::kIpcStKeyConvertVtunnel:
      return IpcStructToStr(*reinterpret_cast<key_convert_vtunnel*>(data));
    case IpctSt::kIpcStValConvertVtunnel:
      return IpcStructToStr(*reinterpret_cast<val_convert_vtunnel*>(data));
    case IpctSt::kIpcStKeyConvertVtunnelIf:
      return IpcStructToStr(*reinterpret_cast<key_convert_vtunnel_if*>(data));
    case IpctSt::kIpcStValConvertVtunnelIf:
      return IpcStructToStr(*reinterpret_cast<val_convert_vtunnel_if*>(data));
    case IpctSt::kIpcStKeyConvertVlink:
      return IpcStructToStr(*reinterpret_cast<key_convert_vlink*>(data));
    case IpctSt::kIpcStValConvertVlink:
      return IpcStructToStr(*reinterpret_cast<val_convert_vlink*>(data));
    case IpctSt::kIpcStValVtnGatewayPort:
      return IpcStructToStr(*reinterpret_cast<val_vtn_gateway_port*>(data));
    // Expand structures
    case IpctSt::kIpcStValVbrExpand:
      return IpcStructToStr(*reinterpret_cast<val_vbr_expand*>(data));
    case IpctSt::kIpcStValVbrIfExpand:
      return IpcStructToStr(*reinterpret_cast<val_vbr_if_expand*>(data));
    case IpctSt::kIpcStValVbrPortMapExpand:
      return IpcStructToStr(*reinterpret_cast<val_vbr_portmap_expand*>(data));
    case IpctSt::kIpcStValVtunnelExpand:
      return IpcStructToStr(*reinterpret_cast<val_vtunnel_expand*>(data));
    case IpctSt::kIpcStValVtunnelIfExpand:
      return IpcStructToStr(*reinterpret_cast<val_vtunnel_if_expand*>(data));
    case IpctSt::kIpcStValVlinkExpand:
      return IpcStructToStr(*reinterpret_cast<val_vlink_expand*>(data));
      // Driver structures
    case IpctSt::kIpcStPfcdrvValVbrIf:
      return IpcStructToStr(*reinterpret_cast<pfcdrv_val_vbr_if*>(data));
    case IpctSt::kIpcStPfcdrvValVbrifVextif:
      return IpcStructToStr(*reinterpret_cast<pfcdrv_val_vbrif_vextif*>(data));
    case IpctSt::kIpcStPfcdrvValFlowfilterEntry:
      return IpcStructToStr(
          *reinterpret_cast<pfcdrv_val_flowfilter_entry*>(data));
    case IpctSt::kIpcStPfcdrvValVbrifPolicingmap:
      return IpcStructToStr(
          *reinterpret_cast<pfcdrv_val_vbrif_policingmap*>(data));
    /* VlanmapOnBoundary: Added new val struct */
    case IpctSt::kIpcStPfcdrvValVlanMap:
      return IpcStructToStr(*reinterpret_cast<pfcdrv_val_vlan_map*>(data));
    case IpctSt::kIpcStPfcdrvValVbrPortMap:
      return IpcStructToStr(*reinterpret_cast<pfcdrv_val_vbr_portmap*>(data));
    case IpctSt::kIpcStPfcdrvValVtnController:
      return IpcStructToStr(
                        *reinterpret_cast<pfcdrv_val_vtn_controller*>(data));

      // Physical structures
    case IpctSt::kIpcStKeyCtr:
      return IpcStructToStr(*reinterpret_cast<key_ctr*>(data));
    case IpctSt::kIpcStValCtr:
      return IpcStructToStr(*reinterpret_cast<val_ctr*>(data));
    case IpctSt::kIpcStValCtrSt:
      return IpcStructToStr(*reinterpret_cast<val_ctr_st*>(data));
    case IpctSt::kIpcStKeyCtrDomain:
      return IpcStructToStr(*reinterpret_cast<key_ctr_domain*>(data));
    case IpctSt::kIpcStValCtrDomain:
      return IpcStructToStr(*reinterpret_cast<val_ctr_domain*>(data));
    case IpctSt::kIpcStValCtrDomainSt:
      return IpcStructToStr(*reinterpret_cast<val_ctr_domain_st*>(data));
    case IpctSt::kIpcStKeyLogicalPort:
      return IpcStructToStr(*reinterpret_cast<key_logical_port*>(data));
    case IpctSt::kIpcStValLogicalPort:
      return IpcStructToStr(*reinterpret_cast<val_logical_port*>(data));
    case IpctSt::kIpcStValLogicalPortSt:
      return IpcStructToStr(*reinterpret_cast<val_logical_port_st*>(data));
    case IpctSt::kIpcStKeyBoundary:
      return IpcStructToStr(*reinterpret_cast<key_boundary*>(data));
    case IpctSt::kIpcStValBoundary:
      return IpcStructToStr(*reinterpret_cast<val_boundary*>(data));
    case IpctSt::kIpcStValBoundarySt:
      return IpcStructToStr(*reinterpret_cast<val_boundary_st*>(data));
    case IpctSt::kIpcStValPathFaultAlarm:
      return IpcStructToStr(*reinterpret_cast<val_path_fault_alarm*>(data));
      // Overlay Driver structures
    case IpctSt::kIpcStVnpdrvValVtunnel:
      return IpcStructToStr(*reinterpret_cast<vnpdrv_val_vtunnel*>(data));
    case IpctSt::kIpcStVnpdrvValVtunnelIf:
      return IpcStructToStr(*reinterpret_cast<vnpdrv_val_vtunnel_if*>(data));
    case IpctSt::kIpcStKeyVtnDataflow:
      return IpcStructToStr(*reinterpret_cast<key_vtn_dataflow*>(data));
    case IpctSt::kIpcStKeyVbid:
      return IpcStructToStr(*reinterpret_cast<key_vbid_label*>(data));
    case IpctSt::kIpcStValVbid:
      return IpcStructToStr(*reinterpret_cast<val_vbid_label*>(data));
    case IpctSt::kIpcStKeyGVtnId:
      return IpcStructToStr(*reinterpret_cast<key_gvtnid_label*>(data));
    case IpctSt::kIpcStValGVtnId:
      return IpcStructToStr(*reinterpret_cast<val_gvtnid_label*>(data));
  }
  return ss.str();
}

// support functions

std::string KtUtil::Ipv4AddrToStr(in_addr ip_addr) {
  return Ipv4AddrToStr(ip_addr.s_addr);
}
std::string KtUtil::Ipv4AddrToStr(uint32_t ip_addr) {
  union IPAddressConverter {
    uint32_t ipInteger;
    uint8_t ipArray[4];
  };
  IPAddressConverter ipac;
  ipac.ipInteger = ip_addr;
  char buf[32];
  snprintf(buf, sizeof(buf), "%d.%d.%d.%d", ipac.ipArray[0], ipac.ipArray[1],
           ipac.ipArray[2], ipac.ipArray[3]);
  return buf;
}

std::string KtUtil::Ipv6AddrToStr(struct in6_addr address) {
  socklen_t ipv6_len = INET6_ADDRSTRLEN;
  char str_addr[INET6_ADDRSTRLEN];
  if (NULL == inet_ntop(AF_INET6, &address, str_addr, ipv6_len)) {
    UPLL_LOG_TRACE(" Invalid IPV6 Address");
    return " Invalid IPV6 Address..!!! ";
  }
  return str_addr;
}


std::string KtUtil::MacAddrToStr(const uint8_t *mac_addr) {
  char buf[32];
  snprintf(buf, sizeof(buf), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2],
           mac_addr[3], mac_addr[4], mac_addr[5]);
  return buf;
}


std::string KtUtil::ValidArrayToStr(const uint8_t *validarray, int size) {
  std::stringstream ss;
  ss << "[";
  for (int i = 0 ; i < size; i++) {
    switch (validarray[i]) {
      case UNC_VF_INVALID:
        ss << "I";
        break;
      case UNC_VF_VALID:
        ss << "V";
        break;
      case UNC_VF_VALID_NO_VALUE:
        ss << "N";
        break;
      case UNC_VF_NOT_SUPPORTED:
        ss << "X";
        break;
      case UNC_VF_VALUE_NOT_MODIFIED:
        ss << "M";
        break;
      default:
        ss << "!" << chr2int(validarray[i]) << "!:";
        break;
    }
  }
  ss    << "]" << endl;
  return ss.str();
}

std::string KtUtil::ConfigStatusToStr(const uint8_t *cfgstatus, int size) {
  std::stringstream ss;
  ss << "[";
  for (int i = 0 ; i < size; i++) {
    switch (cfgstatus[i]) {
      case UNC_CS_UNKNOWN:
        ss << "U";
        break;
      case UNC_CS_APPLIED:
        ss << "A";
        break;
      case UNC_CS_PARTIALLY_APPLIED:
        ss << "P";
        break;
      case UNC_CS_NOT_APPLIED:
        ss << "N";
        break;
      case UNC_CS_INVALID:
        ss << "I";
        break;
      case UNC_CS_NOT_SUPPORTED:
        ss << "X";
        break;
      default:
        ss << "!" << chr2int(cfgstatus[i]) << "!:";
        break;
    }
  }
  ss    << "]" << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const key_unified_nw& key_unified_nw) {
  std::stringstream ss;
  ss << "   -----   key_unified_nw   -----   " << endl;
  ss << "   -->unified_nw_name " << key_unified_nw.unified_nw_id << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const val_unified_nw& val_unified_nw) {
  std::stringstream ss;
  ss << "   -----   val_unified_nw   -----   " << endl;
  ss << "   -->valid " << VALID_ARRAY_TO_STR(val_unified_nw.valid);
  ss << "   -->cs_row_status " << chr2int(val_unified_nw.cs_row_status) <<
      endl;
  ss << "   -->cs_attr " << CS_ARRAY_TO_STR(val_unified_nw.cs_attr);
  ss << "   -->routing_type " <<chr2int(val_unified_nw.routing_type) << endl;
  ss << "   -->is_default " << chr2int(val_unified_nw.is_default) << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const key_unw_label& key_unw_label) {
  std::stringstream ss;
  ss << "   -----   key_unw_label   -----   " << endl;
  ss<< "   -->unified_nw_name       " <<
      key_unw_label.unified_nw_key.unified_nw_id << endl;
  ss << "   -->unw_label_name " <<key_unw_label.unw_label_id << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const val_unw_label& val_unw_label) {
  std::stringstream ss;
  ss << "   -----   val_unw_label   -----   " << endl;
  ss << "   -->valid " << VALID_ARRAY_TO_STR(val_unw_label.valid);
  ss << "   -->cs_row_status " << chr2int(val_unw_label.cs_row_status) << endl;
  ss << "   -->cs_attr " << CS_ARRAY_TO_STR(val_unw_label.cs_attr);
  ss << "   -->max_count " << val_unw_label.max_count << endl;
  ss << "   -->raising_threshold" << val_unw_label.raising_threshold << endl;
  ss << "   -->falling_threshold" << val_unw_label.falling_threshold << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const key_unw_label_range& key_unwl_range) {
  std::stringstream ss;
  ss << "   -----   key_unw_label_range   -----   " << endl;
  ss << "   -->unified_nw_name      " <<
      key_unwl_range.unw_label_key.unified_nw_key.unified_nw_id <<endl;
  ss << "   -->unw_label_name " <<
      key_unwl_range.unw_label_key.unw_label_id << endl;
  ss << "  --->min_range  " << key_unwl_range.range_min << endl;
  ss << "  --->max_range  " << key_unwl_range.range_max << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const val_unw_label_range& val_unwl_range) {
  std::stringstream ss;
  ss << "   -----   val_unw_label_range   -----   " << endl;
  ss << "   -->cs_row_status " << chr2int(val_unwl_range.cs_row_status) << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(
    const key_unw_spine_domain& key_unws_domain) {
  std::stringstream ss;
  ss << "   -----   key_unw_spine_domain  ----  " << endl;
  ss << "   --> unified_nw_name   " <<key_unws_domain.unw_key.unified_nw_id <<
      endl;
  ss << "   --> unw_spine_id  " <<key_unws_domain.unw_spine_id << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(
    const val_unw_spine_domain &val_unws_domain) {
  std::stringstream ss;
  ss << "   ----  val_unw_spine_domain  ----  " <<endl;
  ss << "   --> valid " << VALID_ARRAY_TO_STR(val_unws_domain.valid);
  ss << "   --> cs_row_status " << chr2int(val_unws_domain.cs_row_status) <<
      endl;
  ss << "   --> cs_attr " << CS_ARRAY_TO_STR(val_unws_domain.cs_attr);
  ss << "   --> spine_controller_id " << val_unws_domain.spine_controller_id <<
      endl;
  ss << "   --> spine_domain_id " << val_unws_domain.spine_domain_id << endl;
  ss << "   --> unw_label_id "    << val_unws_domain.unw_label_id << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(
    const val_unw_spdom_ext &val_unw_spdom_ext) {
  std::stringstream ss;
  ss << "   ----  val_unw_spine_domain_ext  ----  " <<endl;
  ss << "   --> valid " << VALID_ARRAY_TO_STR(val_unw_spdom_ext.valid) << endl;
  ss << IpcStructToStr(val_unw_spdom_ext.val_unw_spine_dom) << endl;
  ss << " --- Used label count " << val_unw_spdom_ext.used_label_count << endl;
  // ss << " -- Alarm Status "<<chr2int(val_unw_spdom_ext.alarm_status) << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(
    const val_spdom_st &val_spdom_st) {
  std::stringstream ss;
  ss << "   ----  val_spdom_st ----  " <<endl;
  ss << "   --> valid " << VALID_ARRAY_TO_STR(val_spdom_st.valid) << endl;
  ss << " --> Alarm Status " <<chr2int(val_spdom_st.alarm_status) << endl;
  return ss.str();
}
std::string KtUtil::IpcStructToStr(
    const val_unw_spine_domain_st &val_unwsd_st) {
  std::stringstream ss;
  ss << "   ----  val_unw_spine_domain_st ---- " << endl;
  ss << "   --> valid " << VALID_ARRAY_TO_STR(val_unwsd_st.valid);
  ss << "   --> max_count " << val_unwsd_st.max_count << endl;
  ss << "   --> used_count " << val_unwsd_st.used_count << endl;
  ss << "   --> alarm_status " <<chr2int(val_unwsd_st.alarm_status) << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(
    const val_unw_spine_domain_assigned_label &val_assigned_label) {
  std::stringstream ss;
  ss << "   ----  val_unw_spine_domain_assigned_label ----" << endl;
  ss << "   --> valid " << VALID_ARRAY_TO_STR(val_assigned_label.valid);
  ss << "   --> label " << val_assigned_label.label << endl;
  ss << "   --> vtn_id " << val_assigned_label.vtn_id << endl;
  ss << "   --> vnode_id " << val_assigned_label.vnode_id << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(
    const val_unw_spine_domain_fdbentry &val_unwsd_fdbentry) {
  std::stringstream ss;
  ss << "   ----  val_unw_spine_domain_fdbentry ----" << endl;
  ss << "   --> valid " << VALID_ARRAY_TO_STR(val_unwsd_fdbentry.valid);
  ss << "   --> max_count " <<val_unwsd_fdbentry.max_count << endl;
  ss << "   --> max_switch_id " <<val_unwsd_fdbentry.max_switch_id << endl;
  ss << "   --> min_count " <<val_unwsd_fdbentry.min_count << endl;
  ss << "   --> min_switch_id " <<val_unwsd_fdbentry.min_switch_id << endl;
  ss << "   --> avg_count " <<val_unwsd_fdbentry.avg_count << endl;
  ss << "   --> num_of_switches " <<val_unwsd_fdbentry.num_of_switches << endl;
  ss << "   --> vtn_count " <<val_unwsd_fdbentry.vtn_count << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(
    const val_unw_spine_domain_fdbentry_vtn &val_fdbentry_vtn) {
  std::stringstream ss;
  ss << "   ----  val_unw_spine_domain_fdbentry_vtn ----" << endl;
  ss << "   --> valid " << VALID_ARRAY_TO_STR(val_fdbentry_vtn.valid);
  ss << "   --> vtn_id " <<val_fdbentry_vtn.vtn_id << endl;
  ss << "   --> vlan_id " <<val_fdbentry_vtn.vlan_id << endl;
  ss << "   --> max_count " <<val_fdbentry_vtn.max_count << endl;
  ss << "   --> max_switch_id " <<val_fdbentry_vtn.max_switch_id << endl;
  ss << "   --> min_count " <<val_fdbentry_vtn.min_count << endl;
  ss << "   --> min_switch_id " <<val_fdbentry_vtn.min_switch_id << endl;
  ss << "   --> avg_count " <<val_fdbentry_vtn.avg_count << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const key_vtn_unified &key_vtn_unified) {
  std::stringstream ss;
  ss << "   ----  key_vtn_unified ----" <<endl;
  ss << "   --> vtn_name " <<key_vtn_unified.vtn_key.vtn_name <<endl;
  ss << "   --> unified_nw_id " <<key_vtn_unified.unified_nw_id << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const val_vtn_unified &val_vtn_unified) {
  std::stringstream ss;
  ss << "   ----  val_vtn_unified ----" <<endl;
  ss << "   --> valid " << VALID_ARRAY_TO_STR(val_vtn_unified.valid);
  ss << "   --> cs_row_status " << chr2int(val_vtn_unified.cs_row_status) <<
      endl;
  ss << "   --> cs_attr " << CS_ARRAY_TO_STR(val_vtn_unified.cs_attr) << endl;
  ss << "   --> spine_id " << val_vtn_unified.spine_id << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const val_ping& val_ping_t) {
  std::stringstream ss;
  ss << "   -----   val_ping   -----   " << endl;
  ss << "   -->valid " << VALID_ARRAY_TO_STR(val_ping_t.valid);
  ss << "   -->target_addr :" << Ipv4AddrToStr(val_ping_t.target_addr) << endl;
  ss << "   -->src_addr " << Ipv4AddrToStr(val_ping_t.src_addr) << endl;
  ss << "   -->dfbit " << chr2int(val_ping_t.dfbit) << endl;
  ss << "   -->packet_size " << val_ping_t.packet_size << endl;
  ss << "   -->count " << val_ping_t.count << endl;
  ss << "   -->interval " << chr2int(val_ping_t.interval) << endl;
  ss << "   -->timeout " << chr2int(val_ping_t.timeout) << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const val_vtn_neighbor& data) {
  std::stringstream ss;
  ss << "   -----   val_vtn_neighbor   -----   " << endl;
  ss << "   -->valid " << VALID_ARRAY_TO_STR(data.valid);
  ss << "   -->connected_vnode_name " << data.connected_vnode_name << endl;
  ss << "   -->connected_if_name " << data.connected_if_name << endl;
  ss << "   -->connected_vlink_name " << data.connected_vlink_name << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const key_vtn& key_vtn) {
  std::stringstream ss;
  ss << "   -----   key_vtn   -----   " << endl;
  ss << "   -->vtn_name " << key_vtn.vtn_name << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const key_vtn_dataflow& key_vtn_df) {
  std::stringstream ss;
  ss << "   -----   key_vtn_dataflow   -----   " << endl;
  ss << "   -->vtn_name " << key_vtn_df.vtn_key.vtn_name << endl;
  ss << "   -->vnode_name " << key_vtn_df.vnode_id << endl;
  ss << "   -->vlan_id " << key_vtn_df.vlanid << endl;
  ss << "   -->source_mac_addr " << MacAddrToStr(key_vtn_df.src_mac_address)
      << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const val_vtn& val_vtn) {
  std::stringstream ss;
  ss << "   -----   val_vtn   -----   " << endl;
  ss << "   -->valid " << VALID_ARRAY_TO_STR(val_vtn.valid);
  ss << "   -->cs_row_status " << chr2int(val_vtn.cs_row_status) << endl;
  ss << "   -->cs_attr " << CS_ARRAY_TO_STR(val_vtn.cs_attr);
  ss << "   -->description " << val_vtn.description << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const val_rename_vtn& val_rename_vtn) {
  std::stringstream ss;
  ss << "   -----   val_rename_vtn   -----   " << endl;
  ss << "   -->valid " << VALID_ARRAY_TO_STR(val_rename_vtn.valid);
  ss << "   -->new_name " << val_rename_vtn.new_name << endl;
  ss << "   -->Rename_type "<< chr2int(val_rename_vtn.rename_type) << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const val_vtn_st& val_vtn_st) {
  std::stringstream ss;
  ss << "   -----   val_vtn_st   -----   " << endl;
  ss << "   -->valid " << VALID_ARRAY_TO_STR(val_vtn_st.valid);
  ss << "   -->oper_status " << chr2int(val_vtn_st.oper_status) << endl;
  ss << "   -->alarm_status " << chr2int(val_vtn_st.alarm_status) << endl;
  ss << "   -->creation_time " << val_vtn_st.creation_time << endl;
  ss << "   -->last_updated_time " << val_vtn_st.last_updated_time << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const key_vtn_controller& data) {
  std::stringstream ss;
  ss << "   -----   key_vtn_controller   -----   " << endl;
  // ss << "   -->vtn_key "<< endl;
  ss << IpcStructToStr(data.vtn_key);
  ss << "   -->controller_name " << data.controller_name << endl;
  ss << "   -->domain_name " << data.domain_id << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const val_vtn_mapping_controller_st& data) {
  std::stringstream ss;
  ss << "   -----   val_vtn_mapping_controller_st   -----   " << endl;
  ss << "   -->valid " << VALID_ARRAY_TO_STR(data.valid);
  ss << "   -->switch_id " << data.switch_id << endl;
  ss << "   -->port_name " << data.port_name << endl;
  ss << "   -->logical_port_id " << data.logical_port_id << endl;
  ss << "   -->vlan_id " << data.vlan_id << endl;
  ss << "   -->tagged " << chr2int(data.tagged) << endl;
  ss << "   -->map_type " << chr2int(data.map_type) << endl;
  ss << "   -->vnode_type " << chr2int(data.vnode_type) << endl;
  ss << "   -->vnode_name " << data.vnode_name << endl;
  ss << "   -->vnode_if_name " << data.vnode_if_name << endl;
  return ss.str();
}


std::string KtUtil::IpcStructToStr(const key_vtnstation_controller& data) {
  std::stringstream ss;
  ss << "   -----   key_vtnstation_controller   -----   " << endl;
  ss << "   -->controller_name " << data.controller_name << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const val_vtnstation_controller_st& data) {
  std::stringstream ss;
  ss << "   -----   val_vtnstation_controller_st   -----   " << endl;
  ss << "   -->valid " << VALID_ARRAY_TO_STR(data.valid);
  ss << "   -->station_id " << data.station_id << endl;
  ss << "   -->created_time " << data.created_time << endl;
  ss << "   -->mac_addr " << MacAddrToStr(data.mac_addr) << endl;
  ss << "   -->ipv4_count " << data.ipv4_count << endl;
  ss << "   -->ipv6_count " << data.ipv6_count << endl;
  ss << "   -->port_name " << data.port_name << endl;
  ss << "   -->vlan_id " << data.vlan_id << endl;
  ss << "   -->map_type " << chr2int(data.map_type) << endl;
  ss << "   -->map_status " << chr2int(data.map_status) << endl;
  ss << "   -->vtn_name " << data.vtn_name << endl;
  ss << "   -->domain_id " << data.domain_id << endl;
  ss << "   -->vnode_type " << chr2int(data.vnode_type) << endl;
  ss << "   -->vnode_name " << data.vnode_name << endl;
  ss << "   -->vnode_if_name " << data.vnode_if_name << endl;
  ss << "   -->vnode_if_status " << chr2int(data.vnode_if_status) << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const val_vtnstation_controller_stat& data) {
  std::stringstream ss;
  ss << "   -----   val_vtnstation_controller_stat   -----   " << endl;
  ss << "   -->valid " << VALID_ARRAY_TO_STR(data.valid);
  ss << "   -->allTxPkt " << data.allTxPkt << endl;
  ss << "   -->allRxPkt " << data.allRxPkt << endl;
  ss << "   -->allTxBytes " << data.allTxBytes << endl;
  ss << "   -->allRxBytes " << data.allRxBytes << endl;
  ss << "   -->allNWTxPkt " << data.allNWTxPkt << endl;
  ss << "   -->allNWRxPkt " << data.allNWRxPkt << endl;
  ss << "   -->allNWTxBytes " << data.allNWTxBytes << endl;
  ss << "   -->allNWRxBytes " << data.allNWRxBytes << endl;
  ss << "   -->existingTxPkt " << data.existingTxPkt << endl;
  ss << "   -->existingRxPkt " << data.existingRxPkt << endl;
  ss << "   -->existingTxBytes " << data.existingTxBytes << endl;
  ss << "   -->existingRxBytes " << data.existingRxBytes << endl;
  ss << "   -->expiredTxPkt " << data.expiredTxPkt << endl;
  ss << "   -->expiredRxPkt " << data.expiredRxPkt << endl;
  ss << "   -->expiredTxBytes " << data.expiredTxBytes << endl;
  ss << "   -->expiredRxBytes " << data.expiredRxBytes << endl;
  ss << "   -->allDropRxPkt " << data.allDropRxPkt << endl;
  ss << "   -->allDropRxBytes " << data.allDropRxBytes << endl;
  ss << "   -->existingDropRxPkt " << data.existingDropRxPkt << endl;
  ss << "   -->existingDropRxBytes " << data.existingDropRxBytes << endl;
  ss << "   -->expiredDropRxPkt " << data.expiredDropRxPkt << endl;
  ss << "   -->expiredDropRxBytes " << data.expiredDropRxBytes << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const key_vbr& key_vbr) {
  std::stringstream ss;
  ss << "   -----   key_vbr   -----   " << endl;
  // ss << "   -->vtn_key " << endl;
  ss << IpcStructToStr(key_vbr.vtn_key);
  ss << "   -->vbridge_name " << key_vbr.vbridge_name << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const val_vbr& val_vbr) {
  std::stringstream ss;
  ss << "   -----   val_vbr   -----   " << endl;
  ss << "   -->valid " << VALID_ARRAY_TO_STR(val_vbr.valid);
  ss << "   -->cs_row_status " << chr2int(val_vbr.cs_row_status) << endl;
  ss << "   -->cs_attr " << CS_ARRAY_TO_STR(val_vbr.cs_attr);
  ss << "   -->controller_id " << val_vbr.controller_id << endl;
  ss << "   -->domain_id " << val_vbr.domain_id << endl;
  ss << "   -->vbr_description " << val_vbr.vbr_description << endl;
  ss << "   -->host_addr " << Ipv4AddrToStr(val_vbr.host_addr) << endl;
  ss << "   -->host_addr_prefixlen " << chr2int(val_vbr.host_addr_prefixlen)
      << endl;
  ss << "   -->label " << chr2int(val_vbr.label) << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const val_rename_vbr& val_rename_vbr) {
  std::stringstream ss;
  ss << "   -----   val_rename_vbr   -----   " << endl;
  ss << "   -->valid " << VALID_ARRAY_TO_STR(val_rename_vbr.valid);
  ss << "   -->new_name " << val_rename_vbr.new_name << endl;
  ss << "   -->Rename_type " <<  chr2int(val_rename_vbr.rename_type) <<endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const val_vbr_st& val_vbr_st) {
  std::stringstream ss;
  ss << "   -----   val_vbr_st   -----   " << endl;
  ss << "   -->valid " << VALID_ARRAY_TO_STR(val_vbr_st.valid);
  ss << "   -->oper_status " << chr2int(val_vbr_st.oper_status) << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const val_vbr_l2_domain_st& domain_st) {
  std::stringstream ss;
  ss << "   -----   val_vbr_l2_domain_st   -----   " << endl;
  ss << "   -->valid " << VALID_ARRAY_TO_STR(domain_st.valid);
  ss << "   -->l2domain_id " << domain_st.l2domain_id << endl;
  ss << "   -->ofs_count " << domain_st.ofs_count << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const val_vbr_l2_domain_member_st& data) {
  std::stringstream ss;
  ss << "   -----   val_vbr_l2_domain_member_st   -----   " << endl;
  ss << "   -->valid " << VALID_ARRAY_TO_STR(data.valid);
  ss << "   -->switch_id " << data.switch_id << endl;
  ss << "   -->vlan_id " << data.vlan_id << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const val_vbr_mac_entry_st& vbr_mac_st) {
  std::stringstream ss;
  ss << "   -----   val_vbr_mac_entry_st   -----   " << endl;
  ss << "   -->valid " << VALID_ARRAY_TO_STR(vbr_mac_st.valid);
  ss << "   -->macaddr " << MacAddrToStr(vbr_mac_st.macaddr) << endl;
  ss << "   -->type " << chr2int(vbr_mac_st.type) << endl;
  ss << "   -->if_name " << vbr_mac_st.if_name << endl;
  ss << "   -->if_kind " << chr2int(vbr_mac_st.if_kind) << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const key_vbr_if& key_vbr_if) {
  std::stringstream ss;
  ss << "   -----   key_vbr_if   -----   " << endl;
  // ss << "   -->vbr_key " << endl;
  ss << IpcStructToStr(key_vbr_if.vbr_key);
  ss << "   -->if_name " << key_vbr_if.if_name << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const val_port_map& val_port_map) {
  std::stringstream ss;
  ss << "   -----   val_port_map   -----   " << endl;
  ss << "   -->valid " << VALID_ARRAY_TO_STR(val_port_map.valid);
  ss << "   -->cs_attr " << CS_ARRAY_TO_STR(val_port_map.cs_attr);
  ss << "   -->logical_port_id " << val_port_map.logical_port_id << endl;
  ss << "   -->vlan_id " << val_port_map.vlan_id << endl;
  ss << "   -->tagged " << chr2int(val_port_map.tagged) << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const val_vbr_if& val_vbr_if) {
  std::stringstream ss;
  ss << "   -----   val_vbr_if   -----   " << endl;
  ss << "   -->valid " << VALID_ARRAY_TO_STR(val_vbr_if.valid);
  ss << "   -->admin_status " << chr2int(val_vbr_if.admin_status) << endl;
  ss << "   -->description " << val_vbr_if.description << endl;
  ss << "   -->cs_row_status " << chr2int(val_vbr_if.cs_row_status) << endl;
  ss << "   -->cs_attr " << CS_ARRAY_TO_STR(val_vbr_if.cs_attr);
  ss << IpcStructToStr(val_vbr_if.portmap);
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const val_vbr_if_st& val_vbr_if_st) {
  std::stringstream ss;
  ss << "   -----   val_vbr_if_st   -----   " << endl;
  ss << "   -->valid " << VALID_ARRAY_TO_STR(val_vbr_if_st.valid);
  ss << "   -->oper_status " << chr2int(val_vbr_if_st.oper_status) << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const key_vlan_map& key_vlan_map) {
  std::stringstream ss;
  ss << "   -----   key_vlan_map   -----   " << endl;
  // ss << "   -->vbr_key " << endl;
  ss << IpcStructToStr(key_vlan_map.vbr_key);
  ss << "   -->logical_port_id " << key_vlan_map.logical_port_id << endl;
  ss << "   -->logical_port_id_valid "
      << chr2int(key_vlan_map.logical_port_id_valid) << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const val_vlan_map& val_vlan_map) {
  std::stringstream ss;
  ss << "   -----   val_vlan_map   -----   " << endl;
  ss << "   -->valid " << VALID_ARRAY_TO_STR(val_vlan_map.valid);
  ss << "   -->cs_row_status " << chr2int(val_vlan_map.cs_row_status) << endl;
  ss << "   -->cs_attr " << CS_ARRAY_TO_STR(val_vlan_map.cs_attr);
  ss << "   -->vlan_id " << val_vlan_map.vlan_id << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const key_vbr_portmap& key_vbr_portmap) {
  std::stringstream ss;
  ss << "   -----   key_vbr_portmap   -----   " << endl;
  // ss << "   -->vtn_key " << endl;
  ss << IpcStructToStr(key_vbr_portmap.vbr_key);
  ss << "   -->vbr_portmap_id " << key_vbr_portmap.portmap_id << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const val_vbr_portmap& val_vbr_portmap) {
  std::stringstream ss;
  ss << "   -----   val_vbr_portmap   -----   " << endl;
  ss << "   -->valid " << VALID_ARRAY_TO_STR(val_vbr_portmap.valid);
  ss << "   -->cs_row_status " << chr2int(val_vbr_portmap.cs_row_status)
      << endl;
  ss << "   -->cs_attr " << CS_ARRAY_TO_STR(val_vbr_portmap.cs_attr);
  ss << "   -->controller_id " << val_vbr_portmap.controller_id << endl;
  ss << "   -->domain_id " << val_vbr_portmap.domain_id << endl;
  ss << "   -->logical_port_id " << val_vbr_portmap.logical_port_id << endl;
  ss << "   -->label_type " << chr2int(val_vbr_portmap.label_type) << endl;
  ss << "   -->label " << chr2int(val_vbr_portmap.label) << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(
    const val_vbr_portmap_st& val_vbr_portmap_st) {
  std::stringstream ss;
  ss << "   -----   val_vbr_portmap_st   -----   " << endl;
  ss << "   -->valid " << VALID_ARRAY_TO_STR(val_vbr_portmap_st.valid);
  ss << "   -->oper_status " << chr2int(val_vbr_portmap_st.oper_status) << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const key_vrt& key_vrt) {
  std::stringstream ss;
  ss << "   -----   key_vrt   -----   " << endl;
  // ss << "   -->vtn_key " << endl;
  ss << IpcStructToStr(key_vrt.vtn_key);
  ss << "   -->vrouter_name " << key_vrt.vrouter_name << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const val_vrt& val_vrt) {
  std::stringstream ss;
  ss << "   -----   val_vrt   -----   " << endl;
  ss << "   -->valid " << VALID_ARRAY_TO_STR(val_vrt.valid);
  ss << "   -->cs_row_status " << chr2int(val_vrt.cs_row_status) << endl;
  ss << "   -->cs_attr " << CS_ARRAY_TO_STR(val_vrt.cs_attr);
  ss << "   -->controller_id " << val_vrt.controller_id << endl;
  ss << "   -->domain_id " << val_vrt.domain_id << endl;
  ss << "   -->vrt_description " << val_vrt.vrt_description << endl;
  ss << "   -->dhcp_relay_admin_status "
      << chr2int(val_vrt.dhcp_relay_admin_status) << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const val_rename_vrt& val_rename_vrt) {
  std::stringstream ss;
  ss << "   -----   val_rename_vrt   -----   " << endl;
  ss << "   -->valid " << VALID_ARRAY_TO_STR(val_rename_vrt.valid);
  ss << "   -->new_name " << val_rename_vrt.new_name << endl;
  ss << "   -->Renmae_type " << val_rename_vrt.rename_type << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const val_vrt_st& val_vrt_st) {
  std::stringstream ss;
  ss << "   -----   val_vrt_st   -----   " << endl;
  ss << "   -->valid " << VALID_ARRAY_TO_STR(val_vrt_st.valid);
  ss << "   -->oper_status " << chr2int(val_vrt_st.oper_status) << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const val_vrt_dhcp_relay_st& val_relay_st) {
  std::stringstream ss;
  ss << "   -----   val_vrt_dhcp_relay_st   -----   " << endl;
  ss << "   -->valid " << VALID_ARRAY_TO_STR(val_relay_st.valid);
  ss << "   -->dhcprelay_adminstatus "
      << chr2int(val_relay_st.dhcprelay_adminstatus) << endl;
  ss << "   -->ip_count " << val_relay_st.ip_count << endl;
  ss << "   -->if_count " << val_relay_st.if_count << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const val_dhcp_relay_if_st& data) {
  std::stringstream ss;
  ss << "   -----   val_dhcp_relay_if_st   -----   " << endl;
  ss << "   -->valid " << VALID_ARRAY_TO_STR(data.valid);
  ss << "   -->if_name " << data.if_name << endl;
  ss << "   -->dhcprelay_status " << chr2int(data.dhcprelay_status) << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const val_vrt_arp_entry_st& val_entry_st) {
  std::stringstream ss;
  ss << "   -----   val_vrt_arp_entry_st   -----   " << endl;
  ss << "   -->valid " << VALID_ARRAY_TO_STR(val_entry_st.valid);
  ss << "   -->macaddr " << MacAddrToStr(val_entry_st.macaddr) << endl;
  ss << "   -->ip_addr " << Ipv4AddrToStr(val_entry_st.ip_addr) << endl;
  ss << "   -->type " << chr2int(val_entry_st.type) << endl;
  ss << "   -->if_name " << val_entry_st.if_name << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const val_vrt_ip_route_st& val_st) {
  std::stringstream ss;
  ss << "   -----   val_vrt_ip_route_st   -----   " << endl;
  ss << "   -->valid " << VALID_ARRAY_TO_STR(val_st.valid);
  ss << "   -->destination " << Ipv4AddrToStr(val_st.destination) << endl;
  ss << "   -->gateway " << Ipv4AddrToStr(val_st.gateway) << endl;
  ss << "   -->prefixlen " << chr2int(val_st.prefixlen) << endl;
  ss << "   -->flags " << val_st.flags << endl;
  ss << "   -->metric " << val_st.metric << endl;
  ss << "   -->use " << val_st.use << endl;
  ss << "   -->if_name " << val_st.if_name << endl;
  ss << "   -->nwmonitor_gr " << val_st.nwmonitor_gr << endl;
  ss << "   -->group_metric " << val_st.group_metric << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const key_vrt_if& key_vrt_if) {
  std::stringstream ss;
  ss << "   -----   key_vrt_if   -----   " << endl;
  // ss << "   -->vrt_key " << endl;
  ss << IpcStructToStr(key_vrt_if.vrt_key);
  ss << "   -->if_name " << key_vrt_if.if_name << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const val_vrt_if& val_vrt_if) {
  std::stringstream ss;
  ss << "   -----   val_vrt_if   -----   " << endl;
  ss << "   -->valid " << VALID_ARRAY_TO_STR(val_vrt_if.valid);
  ss << "   -->cs_row_status " << chr2int(val_vrt_if.cs_row_status) << endl;
  ss << "   -->cs_attr " << CS_ARRAY_TO_STR(val_vrt_if.cs_attr);
  ss << "   -->description " << val_vrt_if.description << endl;
  ss << "   -->ip_addr " << Ipv4AddrToStr(val_vrt_if.ip_addr) << endl;
  ss << "   -->prefixlen " << chr2int(val_vrt_if.prefixlen) << endl;
  ss << "   -->macaddr " << MacAddrToStr(val_vrt_if.macaddr) << endl;
  ss << "   -->admin_status " << chr2int(val_vrt_if.admin_status) << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const val_vrt_if_st& val_vrt_if_st) {
  std::stringstream ss;
  ss << "   -----   val_vrt_if_st   -----   " << endl;
  ss << "   -->valid " << VALID_ARRAY_TO_STR(val_vrt_if_st.valid);
  ss << "   -->oper_status " << chr2int(val_vrt_if_st.oper_status) << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const key_static_ip_route& data) {
  std::stringstream ss;
  ss << "   -----   key_static_ip_route   -----   " << endl;
  ss << IpcStructToStr(data.vrt_key);
  ss << "   -->dst_addr " << Ipv4AddrToStr(data.dst_addr) << endl;
  ss << "   -->dst_addr_prefixlen " << chr2int(data.dst_addr_prefixlen) << endl;
  ss << "   -->next_hop_addr " << Ipv4AddrToStr(data.next_hop_addr) << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const val_static_ip_route& data) {
  std::stringstream ss;
  ss << "   -----   val_static_ip_route   -----   " << endl;
  ss << "   -->valid " << VALID_ARRAY_TO_STR(data.valid);
  ss << "   -->cs_row_status " << chr2int(data.cs_row_status) << endl;
  ss << "   -->cs_attr " << CS_ARRAY_TO_STR(data.cs_attr);
  ss << "   -->nwm_name " << data.nwm_name << endl;
  ss << "   -->group_metric " << data.group_metric << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const key_dhcp_relay_if& key_dhcp_relay_if) {
  std::stringstream ss;
  ss << "   -----   key_dhcp_relay_if   -----   " << endl;
  // ss << "   -->vrt_key " << endl;
  ss << IpcStructToStr(key_dhcp_relay_if.vrt_key);
  ss << "   -->if_name " << key_dhcp_relay_if.if_name << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const val_dhcp_relay_if& val_dhcp_relay_if) {
  std::stringstream ss;
  ss << "   -----   val_dhcp_relay_if   -----   " << endl;
  ss << "   -->cs_row_status "
      << chr2int(val_dhcp_relay_if.cs_row_status) << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const key_dhcp_relay_server& key_server) {
  std::stringstream ss;
  ss << "   -----   key_dhcp_relay_server   -----   " << endl;
  // ss << "   -->vrt_key " << endl;
  ss << IpcStructToStr(key_server.vrt_key);
  ss << "   -->server_addr " << Ipv4AddrToStr(key_server.server_addr) << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const val_dhcp_relay_server& val_server) {
  std::stringstream ss;
  ss << "   -----   val_dhcp_relay_server   -----   " << endl;
  ss << "   -->cs_row_status " << chr2int(val_server.cs_row_status) << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const key_vterm& key_vterm) {
std::stringstream ss;
ss << "   -----   key_vterm   -----   " << endl;
ss << IpcStructToStr(key_vterm.vtn_key);
ss << "   -->vterminal_name " << key_vterm.vterminal_name << endl;
return ss.str();
}

std::string KtUtil::IpcStructToStr(const val_vterm& val_vterm) {
std::stringstream ss;
ss << "   -----   val_vterm   -----   " << endl;
ss << "   -->valid " << VALID_ARRAY_TO_STR(val_vterm.valid);
ss << "   -->cs_row_status " << chr2int(val_vterm.cs_row_status) << endl;
ss << "   -->cs_attr " << CS_ARRAY_TO_STR(val_vterm.cs_attr);
ss << "   -->controller_id " << val_vterm.controller_id << endl;
ss << "   -->domain_id " << val_vterm.domain_id << endl;
ss << "   -->vterm_description " << val_vterm.vterm_description << endl;
return ss.str();
}

std::string KtUtil::IpcStructToStr(const val_rename_vterm& val_rename_vterm) {
std::stringstream ss;
ss << "   -----   val_rename_vterm   -----   " << endl;
ss << "   -->valid " << VALID_ARRAY_TO_STR(val_rename_vterm.valid);
ss << "   -->new_name " << val_rename_vterm.new_name << endl;
ss << "   -->Rename_type "<< chr2int(val_rename_vterm.rename_type) << endl;
return ss.str();
}

std::string KtUtil::IpcStructToStr(const val_vterm_st& val_vterm_st) {
std::stringstream ss;
ss << "   -----   val_vterm_st   -----   " << endl;
ss << "   -->valid " << VALID_ARRAY_TO_STR(val_vterm_st.valid);
ss << "   -->oper_status " << chr2int(val_vterm_st.oper_status) << endl;
return ss.str();
}

std::string KtUtil::IpcStructToStr(const key_vterm_if& key_vterm_if) {
  std::stringstream ss;
  ss << "   -----   key_vterm_if   -----   " << endl;
  ss << IpcStructToStr(key_vterm_if.vterm_key);
  ss << "   -->if_name " << key_vterm_if.if_name << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const val_vterm_if& val_vterm_if) {
  std::stringstream ss;
  ss << "   -----   val_vterm_if   -----   " << endl;
  ss << "   -->valid " << VALID_ARRAY_TO_STR(val_vterm_if.valid);
  ss << "   -->admin_status " << chr2int(val_vterm_if.admin_status) << endl;
  ss << "   -->description " << val_vterm_if.description << endl;
  ss << "   -->cs_row_status " << chr2int(val_vterm_if.cs_row_status) << endl;
  ss << "   -->cs_attr " << CS_ARRAY_TO_STR(val_vterm_if.cs_attr);
  ss << IpcStructToStr(val_vterm_if.portmap);
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const val_vterm_if_st& val_vterm_if_st) {
  std::stringstream ss;
  ss << "   -----   val_vterm_if_st   -----   " << endl;
  ss << "   -->valid " << VALID_ARRAY_TO_STR(val_vterm_if_st.valid);
  ss << "   -->oper_status " << chr2int(val_vterm_if_st.oper_status) << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const key_nwm& key_nwm) {
  std::stringstream ss;
  ss << "   -----   key_nwm   -----   " << endl;
  // ss << "   -->vrt_key " << endl;
  ss << IpcStructToStr(key_nwm.vbr_key);
  ss << "   -->nwmonitor_name " << key_nwm.nwmonitor_name << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const val_nwm& val_nwm) {
  std::stringstream ss;
  ss << "   -----   val_nwm   -----   " << endl;
  ss << "   -->valid " << VALID_ARRAY_TO_STR(val_nwm.valid);
  ss << "   -->cs_row_status " << chr2int(val_nwm.cs_row_status) << endl;
  ss << "   -->cs_attr " << CS_ARRAY_TO_STR(val_nwm.cs_attr);
  ss << "   -->admin_status " << chr2int(val_nwm.admin_status) << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const val_nwm_st& val_nwm_st) {
  std::stringstream ss;
  ss << "   -----   val_nwm_st   -----   " << endl;
  ss << "   -->valid " << VALID_ARRAY_TO_STR(val_nwm_st.valid);
  ss << "   -->status " << chr2int(val_nwm_st.status) << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const val_nwm_host_st& val_st) {
  std::stringstream ss;
  ss << "   -----   val_nwm_host_st   -----   " << endl;
  ss << "   -->valid " << VALID_ARRAY_TO_STR(val_st.valid);
  ss << "   -->host_address " << Ipv4AddrToStr(val_st.host_address) << endl;
  ss << "   -->status " << chr2int(val_st.status) << endl;
  ss << "   -->ping_send " << val_st.ping_send << endl;
  ss << "   -->ping_recv " << val_st.ping_recv << endl;
  ss << "   -->ping_err " << val_st.ping_err << endl;
  ss << "   -->ping_trouble " << val_st.ping_trouble << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const key_nwm_host& data) {
  std::stringstream ss;
  ss << "   -----   key_nwm_host   -----   " << endl;
  // ss << "   -->nwm_key " << endl;
  ss << IpcStructToStr(data.nwm_key);
  ss << "   -->host_address " << Ipv4AddrToStr(data.host_address) << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const val_nwm_host& val_nwm_host) {
  std::stringstream ss;
  ss << "   -----   val_nwm_host   -----   " << endl;
  ss << "   -->valid " << VALID_ARRAY_TO_STR(val_nwm_host.valid);
  ss << "   -->cs_row_status " << chr2int(val_nwm_host.cs_row_status) << endl;
  ss << "   -->cs_attr " << CS_ARRAY_TO_STR(val_nwm_host.cs_attr);
  ss << "   -->health_interval " << val_nwm_host.health_interval << endl;
  ss << "   -->recovery_interval " << val_nwm_host.recovery_interval << endl;
  ss << "   -->failure_count " << chr2int(val_nwm_host.failure_count) << endl;
  ss << "   -->recovery_count " << chr2int(val_nwm_host.recovery_count) << endl;
  ss << "   -->wait_time " << chr2int(val_nwm_host.wait_time) << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const key_vtep& key_vtep) {
  std::stringstream ss;
  ss << "   -----   key_vtep   -----   " << endl;
  // ss << "   -->vtn_key " << endl;
  ss << IpcStructToStr(key_vtep.vtn_key);
  ss << "   -->vtep_name " << key_vtep.vtep_name << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const val_vtep& val_vtep) {
  std::stringstream ss;
  ss << "   -----   val_vtep   -----   " << endl;
  ss << "   -->valid " << VALID_ARRAY_TO_STR(val_vtep.valid);
  ss << "   -->cs_row_status " << chr2int(val_vtep.cs_row_status) << endl;
  ss << "   -->cs_attr " << CS_ARRAY_TO_STR(val_vtep.cs_attr);
  ss << "   -->description " << val_vtep.description << endl;
  ss << "   -->controller_id " << val_vtep.controller_id << endl;
  ss << "   -->domain_id " << val_vtep.domain_id << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const val_vtep_st& val_vtep_st) {
  std::stringstream ss;
  ss << "   -----   val_vtep_st   -----   " << endl;
  ss << "   -->valid " << VALID_ARRAY_TO_STR(val_vtep_st.valid);
  ss << "   -->oper_status " << chr2int(val_vtep_st.oper_status) << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const key_vtep_if& key_vtep_if) {
  std::stringstream ss;
  ss << "   -----   key_vtep_if   -----   " << endl;
  // ss << "   -->vtep_key " << endl;
  ss << IpcStructToStr(key_vtep_if.vtep_key);
  ss << "   -->if_name " << key_vtep_if.if_name << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const val_vtep_if& val_vtep_if) {
  std::stringstream ss;
  ss << "   -----   val_vtep_if   -----   " << endl;
  ss << "   -->valid " << VALID_ARRAY_TO_STR(val_vtep_if.valid);
  ss << "   -->cs_row_status " << chr2int(val_vtep_if.cs_row_status) << endl;
  ss << "   -->cs_attr " << CS_ARRAY_TO_STR(val_vtep_if.cs_attr);
  ss << "   -->description " << val_vtep_if.description << endl;
  ss << "   -->admin_status " << chr2int(val_vtep_if.admin_status) << endl;
  ss << IpcStructToStr(val_vtep_if.portmap);
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const val_vtep_if_st& val_vtep_if_st) {
  std::stringstream ss;
  ss << "   -----   val_vtep_if_st   -----   " << endl;
  ss << "   -->valid " << VALID_ARRAY_TO_STR(val_vtep_if_st.valid);
  ss << "   -->oper_status " << chr2int(val_vtep_if_st.oper_status) << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const key_vtep_grp& key_vtep_grp) {
  std::stringstream ss;
  ss << "   -----   key_vtep_grp   -----   " << endl;
  // ss << "   -->vtn_key " << endl;
  ss << IpcStructToStr(key_vtep_grp.vtn_key);
  ss << "   -->vtepgrp_name " << key_vtep_grp.vtepgrp_name << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const val_vtep_grp& val_vtep_grp) {
  std::stringstream ss;
  ss << "   -----   val_vtep_grp   -----   " << endl;
  ss << "   -->valid " << VALID_ARRAY_TO_STR(val_vtep_grp.valid);
  ss << "   -->cs_row_status " << chr2int(val_vtep_grp.cs_row_status) << endl;
  ss << "   -->controller_id " << val_vtep_grp.controller_id << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const key_vtep_grp_member& data) {
  std::stringstream ss;
  ss << "   -----   key_vtep_grp_member   -----   " << endl;
  // ss << "   -->vtepgrp_key" << IpcStructToStr(data.vtepgrp_key);
  ss << IpcStructToStr(data.vtepgrp_key);
  ss << "   -->vtepmember_name " << data.vtepmember_name << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const val_vtep_grp_member& data) {
  std::stringstream ss;
  ss << "   -----   val_vtep_grp_member   -----   " << endl;
  ss << "   -->cs_row_status " << chr2int(data.cs_row_status) << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const key_vtunnel& key_vtunnel) {
  std::stringstream ss;
  ss << "   -----   key_vtunnel   -----   " << endl;
  // ss << "   -->vtn_key " << endl;
  ss << IpcStructToStr(key_vtunnel.vtn_key);
  ss << "   -->vtunnel_name " << key_vtunnel.vtunnel_name << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const val_vtunnel& val_vtunnel) {
  std::stringstream ss;
  ss << "   -----   val_vtunnel   -----   " << endl;
  ss << "   -->valid " << VALID_ARRAY_TO_STR(val_vtunnel.valid);
  ss << "   -->cs_row_status " << chr2int(val_vtunnel.cs_row_status) << endl;
  ss << "   -->cs_attr " << CS_ARRAY_TO_STR(val_vtunnel.cs_attr);
  ss << "   -->description " << val_vtunnel.description << endl;
  ss << "   -->controller_id " << val_vtunnel.controller_id << endl;
  ss << "   -->domain_id " << val_vtunnel.domain_id << endl;
  ss << "   -->vtn_name " << val_vtunnel.vtn_name << endl;
  ss << "   -->vtep_grp_name " << val_vtunnel.vtep_grp_name << endl;
  ss << "   -->label " << val_vtunnel.label << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const val_vtunnel_st& val_vtunnel_st) {
  std::stringstream ss;
  ss << "   -----   val_vtunnel_st   -----   " << endl;
  ss << "   -->valid " << VALID_ARRAY_TO_STR(val_vtunnel_st.valid);
  ss << "   -->oper_status " << chr2int(val_vtunnel_st.oper_status) << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const key_vtunnel_if& key_vtunnel_if) {
  std::stringstream ss;
  ss << "   -----   key_vtunnel_if   -----   " << endl;
  // ss << "   -->vtunnel_key " << endl;
  ss << IpcStructToStr(key_vtunnel_if.vtunnel_key);
  ss << "   -->if_name " << key_vtunnel_if.if_name << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const val_vtunnel_if& val_vtunnel_if) {
  std::stringstream ss;
  ss << "   -----   val_vtunnel_if   -----   " << endl;
  ss << "   -->valid " << VALID_ARRAY_TO_STR(val_vtunnel_if.valid);
  ss << "   -->cs_row_status " << chr2int(val_vtunnel_if.cs_row_status) << endl;
  ss << "   -->cs_attr " << CS_ARRAY_TO_STR(val_vtunnel_if.cs_attr);
  ss << "   -->description " << val_vtunnel_if.description << endl;
  ss << "   -->admin_status " << chr2int(val_vtunnel_if.admin_status) << endl;
  ss << IpcStructToStr(val_vtunnel_if.portmap);
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const val_vtunnel_if_st& val_vtunnel_if_st) {
  std::stringstream ss;
  ss << "   -----   val_vtunnel_if_st   -----   " << endl;
  ss << "   -->valid " << VALID_ARRAY_TO_STR(val_vtunnel_if_st.valid);
  ss << "   -->oper_status " << chr2int(val_vtunnel_if_st.oper_status) << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const key_vunknown& key_vunknown) {
  std::stringstream ss;
  ss << "   -----   key_vunknown   -----   " << endl;
  // ss << "   -->vtn_key " << endl;
  ss << IpcStructToStr(key_vunknown.vtn_key);
  ss << "   -->vunknown_name " << key_vunknown.vunknown_name << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const val_vunknown& val_vunknown) {
  std::stringstream ss;
  ss << "   -----   val_vunknown   -----   " << endl;
  ss << "   -->valid " << VALID_ARRAY_TO_STR(val_vunknown.valid);
  ss << "   -->cs_row_status " << chr2int(val_vunknown.cs_row_status) << endl;
  ss << "   -->cs_attr " << CS_ARRAY_TO_STR(val_vunknown.cs_attr);
  ss << "   -->description " << val_vunknown.description << endl;
  ss << "   -->type " << chr2int(val_vunknown.type) << endl;
  ss << "   -->domain_id " << val_vunknown.domain_id << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const key_vunk_if& key_vunk_if) {
  std::stringstream ss;
  ss << "   -----   key_vunk_if   -----   " << endl;
  // ss << "   -->vunk_key " << endl;
  ss << IpcStructToStr(key_vunk_if.vunk_key);
  ss << "   -->if_name " << key_vunk_if.if_name << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const val_vunk_if& val_vunk_if) {
  std::stringstream ss;
  ss << "   -----   val_vunk_if   -----   " << endl;
  ss << "   -->valid " << VALID_ARRAY_TO_STR(val_vunk_if.valid);
  ss << "   -->cs_row_status " << chr2int(val_vunk_if.cs_row_status) << endl;
  ss << "   -->cs_attr " << CS_ARRAY_TO_STR(val_vunk_if.cs_attr);
  ss << "   -->description " << val_vunk_if.description << endl;
  ss << "   -->admin_status " << chr2int(val_vunk_if.admin_status) << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const key_vlink& key_vlink) {
  std::stringstream ss;
  ss << "   -----   key_vlink   -----   " << endl;
  // ss << "   -->vtn_key " << endl;
  ss << IpcStructToStr(key_vlink.vtn_key);
  ss << "   -->vlink_name " << key_vlink.vlink_name << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const val_vlink& val_vlink) {
  std::stringstream ss;
  ss << "   -----   val_vlink   -----   " << endl;
  ss << "   -->valid " << VALID_ARRAY_TO_STR(val_vlink.valid);
  ss << "   -->cs_row_status " << chr2int(val_vlink.cs_row_status) << endl;
  ss << "   -->cs_attr " << CS_ARRAY_TO_STR(val_vlink.cs_attr);
  ss << "   -->admin_status " << chr2int(val_vlink.admin_status) << endl;
  ss << "   -->vnode1_name " << val_vlink.vnode1_name << endl;
  ss << "   -->vnode1_ifname " << val_vlink.vnode1_ifname << endl;
  ss << "   -->vnode2_name " << val_vlink.vnode2_name << endl;
  ss << "   -->vnode2_ifname " << val_vlink.vnode2_ifname << endl;
  ss << "   -->boundary_name " << val_vlink.boundary_name << endl;
  ss << "   -->label_type " << chr2int(val_vlink.label_type) << endl;
  ss << "   -->label " << chr2int(val_vlink.label) << endl;
  ss << "   -->description " << val_vlink.description << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const val_vlink_st& val_vlink_st) {
  std::stringstream ss;
  ss << "   -----   val_vlink_st   -----   " << endl;
  ss << "   -->valid " << VALID_ARRAY_TO_STR(val_vlink_st.valid);
  ss << "   -->oper_status " << chr2int(val_vlink_st.oper_status) << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const val_rename_vlink& val_rename_vlink) {
  std::stringstream ss;
  ss << "   -----   val_rename_vlink   -----   " << endl;
  ss << "   -->valid " << VALID_ARRAY_TO_STR(val_rename_vlink.valid);
  ss << "   -->new_name " << val_rename_vlink.new_name << endl;
  ss << "   -->Rename_type " << chr2int(val_rename_vlink.rename_type) << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const key_flowlist& key_flowlist) {
  std::stringstream ss;
  ss << "   -----   key_flowlist   -----   " << endl;
  ss << "   -->flowlist_name " << key_flowlist.flowlist_name << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const val_flowlist& val_flowlist) {
  std::stringstream ss;
  ss << "   -----   val_flowlist   -----   " << endl;
  ss << "   -->valid " << VALID_ARRAY_TO_STR(val_flowlist.valid);
  ss << "   -->cs_row_status " << chr2int(val_flowlist.cs_row_status) << endl;
  ss << "   -->cs_attr " << CS_ARRAY_TO_STR(val_flowlist.cs_attr);
  ss << "   -->ip_type " << chr2int(val_flowlist.ip_type) << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const val_rename_flowlist& data) {
  std::stringstream ss;
  ss << "   -----   val_rename_flowlist   -----   " << endl;
  ss << "   -->valid " << VALID_ARRAY_TO_STR(data.valid);
  ss << "   -->flowlist_newname " << data.flowlist_newname << endl;
  ss << "   -->Rename_type " << chr2int(data.rename_type) << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const key_flowlist_entry& data) {
  std::stringstream ss;
  ss << "   -----   key_flowlist_entry   -----   " << endl;
  // ss << "   -->flowlist_key " << endl;
  ss << IpcStructToStr(data.flowlist_key);
  ss << "   -->sequence_num " << data.sequence_num << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const val_flowlist_entry& data) {
  std::stringstream ss;
  ss << "   -----   val_flowlist_entry   -----   " << endl;
  ss << "   -->valid " << VALID_ARRAY_TO_STR(data.valid);
  ss << "   -->cs_row_status " << chr2int(data.cs_row_status) << endl;
  ss << "   -->cs_attr " << CS_ARRAY_TO_STR(data.cs_attr);
  ss << "   -->mac_dst " << MacAddrToStr(data.mac_dst) << endl;
  ss << "   -->mac_src " << MacAddrToStr(data.mac_src) << endl;
  ss << "   -->mac_eth_type " << data.mac_eth_type << endl;
  ss << "   -->dst_ip " << Ipv4AddrToStr(data.dst_ip) << endl;
  ss << "   -->dst_ip_prefixlen " << chr2int(data.dst_ip_prefixlen) << endl;
  ss << "   -->src_ip " << Ipv4AddrToStr(data.src_ip) << endl;
  ss << "   -->src_ip_prefixlen " << chr2int(data.src_ip_prefixlen) << endl;
  ss << "   -->vlan_priority " << chr2int(data.vlan_priority) << endl;
  ss << "   -->dst_ipv6 " << Ipv6AddrToStr(data.dst_ipv6) << endl;
  ss << "   -->dst_ipv6_prefixlen " << chr2int(data.dst_ipv6_prefixlen) << endl;
  ss << "   -->src_ipv6 " << Ipv6AddrToStr(data.src_ipv6) << endl;
  ss << "   -->src_ipv6_prefixlen " << chr2int(data.src_ipv6_prefixlen) << endl;
  ss << "   -->ip_proto " << chr2int(data.ip_proto) << endl;
  ss << "   -->ip_dscp " << chr2int(data.ip_dscp) << endl;
  ss << "   -->l4_dst_port " << data.l4_dst_port << endl;
  ss << "   -->l4_dst_port_endpt " << data.l4_dst_port_endpt << endl;
  ss << "   -->l4_src_port " << data.l4_src_port << endl;
  ss << "   -->l4_src_port_endpt " << data.l4_src_port_endpt << endl;
  ss << "   -->icmp_type " << chr2int(data.icmp_type) << endl;
  ss << "   -->icmp_code " << chr2int(data.icmp_code) << endl;
  ss << "   -->icmpv6_type " << chr2int(data.icmpv6_type) << endl;
  ss << "   -->icmpv6_code " << chr2int(data.icmpv6_code) << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const pom_stats& pom_stats) {
  std::stringstream ss;
  ss << "   -----   pom_stats   -----   " << endl;
  ss << "   -->valid " << VALID_ARRAY_TO_STR(pom_stats.valid);
  ss << "   -->packets " << pom_stats.packets << endl;
  ss << "   -->bytes " << pom_stats.bytes << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const key_vtn_flowfilter& data) {
  std::stringstream ss;
  ss << "   -----   key_vtn_flowfilter   -----   " << endl;
  // ss << "   -->vtn_key " << endl;
  ss << IpcStructToStr(data.vtn_key);
  ss << "   -->input_direction " << chr2int(data.input_direction) << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const val_flowfilter& val_flowfilter) {
  std::stringstream ss;
  ss << "   -----   val_flowfilter   -----   " << endl;
  ss << "   -->cs_row_status " << chr2int(val_flowfilter.cs_row_status) << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const val_vtn_flowfilter_controller_st&
                              val_cnler_st) {
  std::stringstream ss;
  ss << "   -----   val_vtn_flowfilter_controller_st   -----   " << endl;
  ss << "   -->valid " << VALID_ARRAY_TO_STR(val_cnler_st.valid);
  ss << "   -->direction " << chr2int(val_cnler_st.direction) << endl;
  ss << "   -->sequence_num " << val_cnler_st.sequence_num << endl;
  ss << "   -->flowlist " << val_cnler_st.flowlist << endl;
  ss << "   -->ip_type " << chr2int(val_cnler_st.ip_type) << endl;
  ss << "   -->action " << chr2int(val_cnler_st.action) << endl;
  ss << "   -->dscp " << chr2int(val_cnler_st.dscp) << endl;
  ss << "   -->priority " << chr2int(val_cnler_st.priority) << endl;
  ss << "   -->nwm_status " << chr2int(val_cnler_st.nwm_status) << endl;
  ss << "   -->software " << endl;
  ss << IpcStructToStr(val_cnler_st.software);
  ss << "   -->exist " << endl;
  ss << IpcStructToStr(val_cnler_st.exist);
  ss << "   -->expire " << endl;
  ss << IpcStructToStr(val_cnler_st.expire);
  ss << "   -->total " << endl;
  ss << IpcStructToStr(val_cnler_st.total);
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const val_flowlist_entry_st& val_cnler_st) {
  std::stringstream ss;
  ss << "   -----   val_flowlist_entry_st   -----   " << endl;
  ss << "   -->valid " << VALID_ARRAY_TO_STR(val_cnler_st.valid);
  ss << "   -->sequence_num " << val_cnler_st.sequence_num << endl;
  ss << "   -->software " << endl;
  ss << IpcStructToStr(val_cnler_st.software);
  ss << "   -->exist " << endl;
  ss << IpcStructToStr(val_cnler_st.exist);
  ss << "   -->expire " << endl;
  ss << IpcStructToStr(val_cnler_st.expire);
  ss << "   -->total " << endl;
  ss << IpcStructToStr(val_cnler_st.total);
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const key_vtn_flowfilter_entry&
                              key_vtn_flowfilter_entry) {
  std::stringstream ss;
  ss << "   -----   key_vtn_flowfilter_entry   -----   " << endl;
  // ss << "   -->flowfilter_key " << endl;
  ss << IpcStructToStr(key_vtn_flowfilter_entry.flowfilter_key);
  ss << "   -->sequence_num " << key_vtn_flowfilter_entry.sequence_num << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const val_vtn_flowfilter_entry& data) {
  std::stringstream ss;
  ss << "   -----   val_vtn_flowfilter_entry   -----   " << endl;
  ss << "   -->valid " << VALID_ARRAY_TO_STR(data.valid);
  ss << "   -->cs_row_status " << chr2int(data.cs_row_status) << endl;
  ss << "   -->cs_attr " << CS_ARRAY_TO_STR(data.cs_attr);
  ss << "   -->flowlist_name " << data.flowlist_name << endl;
  ss << "   -->action " << chr2int(data.action) << endl;
  ss << "   -->nwm_name " << data.nwm_name << endl;
  ss << "   -->dscp " << chr2int(data.dscp) << endl;
  ss << "   -->priority " << chr2int(data.priority) << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const key_vtn_flowfilter_controller& data) {
  std::stringstream ss;
  ss << "   -----   key_vtn_flowfilter_controller   -----   " << endl;
  // ss << "   -->vtn_key " << endl;
  ss << IpcStructToStr(data.vtn_key);
  ss << "   -->controller_name " << data.controller_name << endl;
  ss << "   -->domain_name " << data.domain_id << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const val_flowfilter_controller& data) {
  std::stringstream ss;
  ss << "   -----   val_flowfilter_controller   -----   " << endl;
  ss << "   -->valid " << VALID_ARRAY_TO_STR(data.valid);
  ss << "   -->direction " << chr2int(data.direction) << endl;
  ss << "   -->sequence_num " << data.sequence_num << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const key_vbr_flowfilter& data) {
  std::stringstream ss;
  ss << "   -----   key_vbr_flowfilter   -----   " << endl;
  // ss << "   -->vbr_key " << endl;
  ss << IpcStructToStr(data.vbr_key);
  ss << "   -->direction " << chr2int(data.direction) << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const key_vbr_flowfilter_entry&
                              key_vbr_flowfilter_entry) {
  std::stringstream ss;
  ss << "   -----   key_vbr_flowfilter_entry   -----   " << endl;
  // ss << "   -->flowfilter_key " << endl;
  ss << IpcStructToStr(key_vbr_flowfilter_entry.flowfilter_key);
  ss << "   -->sequence_num "
      << key_vbr_flowfilter_entry.sequence_num << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const val_flowfilter_entry& data) {
  std::stringstream ss;
  ss << "   -----   val_flowfilter_entry   -----   " << endl;
  ss << "   -->valid " << VALID_ARRAY_TO_STR(data.valid);
  ss << "   -->cs_row_status " << chr2int(data.cs_row_status) << endl;
  ss << "   -->cs_attr " << CS_ARRAY_TO_STR(data.cs_attr);
  ss << "   -->flowlist_name " << data.flowlist_name << endl;
  ss << "   -->action " << chr2int(data.action) << endl;
  ss << "   -->redirect_node " << data.redirect_node << endl;
  ss << "   -->redirect_port " << data.redirect_port << endl;
  ss << "   -->redirect_direction " << chr2int(data.redirect_direction) << endl;
  ss << "   -->modify_dstmac " << MacAddrToStr(data.modify_dstmac) << endl;
  ss << "   -->modify_srcmac " << MacAddrToStr(data.modify_srcmac) << endl;
  ss << "   -->nwm_name " << data.nwm_name << endl;
  ss << "   -->dscp " << chr2int(data.dscp) << endl;
  ss << "   -->priority " << chr2int(data.priority) << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const val_flowfilter_entry_st& data) {
  std::stringstream ss;
  ss << "   -----   val_flowfilter_entry_st   -----   " << endl;
  ss << "   -->valid " << VALID_ARRAY_TO_STR(data.valid);
  ss << "   -->sequence_num " << data.sequence_num << endl;
  ss << "   -->nwm_status " << chr2int(data.nwm_status) << endl;
  ss << "   -->software " << endl;
  ss << IpcStructToStr(data.software);
  ss << "   -->exist " << endl;
  ss << IpcStructToStr(data.exist);
  ss << "   -->expire " << endl;
  ss << IpcStructToStr(data.expire);
  ss << "   -->total " << endl;
  ss << IpcStructToStr(data.total);
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const key_vbr_if_flowfilter&
                              key_vbr_if_flowfilter) {
  std::stringstream ss;
  ss << "   -----   key_vbr_if_flowfilter   -----   " << endl;
  // ss << "   -->if_key " << endl;
  ss << IpcStructToStr(key_vbr_if_flowfilter.if_key);
  ss << "   -->direction " << chr2int(key_vbr_if_flowfilter.direction) << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const key_vbr_if_flowfilter_entry&
                              key_vbr_if_flowfilter_entry) {
  std::stringstream ss;
  ss << "   -----   key_vbr_if_flowfilter_entry   -----   " << endl;
  // ss << "   -->flowfilter_key " << endl;
  ss << IpcStructToStr(key_vbr_if_flowfilter_entry.flowfilter_key);
  ss << "   -->sequence_num "
      << key_vbr_if_flowfilter_entry.sequence_num << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const key_vrt_if_flowfilter&
                              key_vrt_if_flowfilter) {
  std::stringstream ss;
  ss << "   -----   key_vrt_if_flowfilter   -----   " << endl;
  // ss << "   -->if_key " << endl;
  ss << IpcStructToStr(key_vrt_if_flowfilter.if_key);
  ss << "   -->direction " << chr2int(key_vrt_if_flowfilter.direction) << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const key_vrt_if_flowfilter_entry& data) {
  std::stringstream ss;
  ss << "   -----   key_vrt_if_flowfilter_entry   -----   " << endl;
  // ss << "   -->flowfilter_key " << endl;
  ss << IpcStructToStr(data.flowfilter_key);
  ss << "   -->sequence_num " << data.sequence_num << endl;
  return ss.str();
}
std::string KtUtil::IpcStructToStr(const key_vterm_if_flowfilter&
                              key_vterm_if_flowfilter) {
  std::stringstream ss;
  ss << "   -----   key_vterm_if_flowfilter   -----   " << endl;
  ss << IpcStructToStr(key_vterm_if_flowfilter.if_key);
  ss << "   -->direction " << chr2int(key_vterm_if_flowfilter.direction)
     << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const key_vterm_if_flowfilter_entry& data) {
  std::stringstream ss;
  ss << "   -----   key_vterm_if_flowfilter_entry   -----   " << endl;
  ss << IpcStructToStr(data.flowfilter_key);
  ss << "   -->sequence_num " << data.sequence_num << endl;
  return ss.str();
}
std::string KtUtil::IpcStructToStr(const key_policingprofile& data) {
  std::stringstream ss;
  ss << "   -----   key_policingprofile   -----   " << endl;
  ss << "   -->policingprofile_name " << data.policingprofile_name << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const val_policingprofile& data) {
  std::stringstream ss;
  ss << "   -----   val_policingprofile   -----   " << endl;
  ss << "   -->cs_row_status " << chr2int(data.cs_row_status) << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const val_rename_policingprofile&
                              val_rename_policingprofile) {
  std::stringstream ss;
  ss << "   -----   val_rename_policingprofile   -----   " << endl;
  ss << "   -->valid " << VALID_ARRAY_TO_STR(val_rename_policingprofile.valid);
  ss << "   -->policingprofile_newname "
      << val_rename_policingprofile.policingprofile_newname << endl;
  ss << "   -->Rename_type " << chr2int(val_rename_policingprofile.rename_type)
      << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const key_policingprofile_entry&
                              key_policingprofile_entry) {
  std::stringstream ss;
  ss << "   -----   key_policingprofile_entry   -----   " << endl;
  // ss << "   -->policingprofile_key " << endl;
  ss << IpcStructToStr(key_policingprofile_entry.policingprofile_key);
  ss << "   -->sequence_num "
      << chr2int(key_policingprofile_entry.sequence_num) << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const val_policingprofile_entry& data) {
  std::stringstream ss;
  ss << "   -----   val_policingprofile_entry   -----   " << endl;
  ss << "   -->valid " << VALID_ARRAY_TO_STR(data.valid);
  ss << "   -->cs_row_status " << chr2int(data.cs_row_status) << endl;
  ss << "   -->cs_attr " << CS_ARRAY_TO_STR(data.cs_attr);
  ss << "   -->flowlist " << data.flowlist << endl;
  ss << "   -->rate " << chr2int(data.rate) << endl;
  ss << "   -->cir " << data.cir << endl;
  ss << "   -->cbs " << data.cbs << endl;
  ss << "   -->pir " << data.pir << endl;
  ss << "   -->pbs " << data.pbs << endl;
  ss << "   -->green_action " << chr2int(data.green_action) << endl;
  ss << "   -->green_action_priority "
      << chr2int(data.green_action_priority) << endl;
  ss << "   -->green_action_dscp " << chr2int(data.green_action_dscp) << endl;
  ss << "   -->green_action_drop_precedence "
      << chr2int(data.green_action_drop_precedence) << endl;
  ss << "   -->yellow_action " << chr2int(data.yellow_action) << endl;
  ss << "   -->yellow_action_priority "
      << chr2int(data.yellow_action_priority) << endl;
  ss << "   -->yellow_action_dscp " << chr2int(data.yellow_action_dscp) << endl;
  ss << "   -->yellow_action_drop_precedence "
      << chr2int(data.yellow_action_drop_precedence) << endl;
  ss << "   -->red_action " << chr2int(data.red_action) << endl;
  ss << "   -->red_action_priority "
      << chr2int(data.red_action_priority) << endl;
  ss << "   -->red_action_dscp " << chr2int(data.red_action_dscp) << endl;
  ss << "   -->red_action_drop_precedence "
      << chr2int(data.red_action_drop_precedence) << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const val_policingmap& data) {
  std::stringstream ss;
  ss << "   -----   val_policingmap   -----   " << endl;
  ss << "   -->valid " << VALID_ARRAY_TO_STR(data.valid);
  ss << "   -->cs_row_status " << chr2int(data.cs_row_status) << endl;
  ss << "   -->cs_attr " << CS_ARRAY_TO_STR(data.cs_attr);
  ss << "   -->policer_name " << data.policer_name << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const val_policingmap_controller_st&
                              val_cnler_st) {
  std::stringstream ss;
  ss << "   -----   val_policingmap_controller_st   -----   " << endl;
  ss << "   -->valid " << VALID_ARRAY_TO_STR(val_cnler_st.valid);
  ss << "   -->sequence_num " << chr2int(val_cnler_st.sequence_num) << endl;
  ss << "   -->total " << endl;
  ss << IpcStructToStr(val_cnler_st.total);
  ss << "   -->green_yellow " << endl;
  ss << IpcStructToStr(val_cnler_st.green_yellow);
  ss << "   -->red " << endl;
  ss << IpcStructToStr(val_cnler_st.red);
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const val_policingmap_switch_st& data) {
  std::stringstream ss;
  ss << "   -----   val_policingmap_switch_st   -----   " << endl;
  ss << "   -->valid " << VALID_ARRAY_TO_STR(data.valid);
  ss << "   -->policer_id " << data.policer_id << endl;
  ss << "   -->switch_id " << data.switch_id << endl;
  ss << "   -->status " << chr2int(data.status) << endl;
  ss << "   -->vBridge_name " << data.vnode_name << endl;
  ss << "   -->if_name " << data.vnode_if_name << endl;
  ss << "   -->port_name " << data.port_name << endl;
  ss << "   -->vlanid " << data.vlanid << endl;
  ss << "   -->total " << endl;
  ss << IpcStructToStr(data.total);
  ss << "   -->green_yellow " << endl;
  ss << IpcStructToStr(data.green_yellow);
  ss << "   -->red " << endl;
  ss << IpcStructToStr(data.red);
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const key_vtn_policingmap_controller&
                              key_vtn_policingmap_controller) {
  std::stringstream ss;
  ss << "   -----   key_vtn_policingmap_controller   -----   " << endl;
  // ss << "   -->vtn_key " << endl;
  ss << IpcStructToStr(key_vtn_policingmap_controller.vtn_key);
  ss << "   -->controller_name "
      << key_vtn_policingmap_controller.controller_name << endl;
  ss << "   -->domain_name "
      << key_vtn_policingmap_controller.domain_id << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const key_vbr_policingmap_entry&
                                   key_vbr_policingmap_entry) {
  std::stringstream ss;
  ss << "   -----   key_vbr_policingmap_entry   -----   " << endl;
  // ss << "   -->vbr_key " << endl;
  ss << IpcStructToStr(key_vbr_policingmap_entry.vbr_key);
  ss << "   -->sequence_num "
      << chr2int(key_vbr_policingmap_entry.sequence_num) << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const val_policingmap_controller&
                                   val_policingmap_controller) {
  std::stringstream ss;
  ss << "   -----   val_policingmap_controller   -----   " << endl;
  ss << "   -->valid " << VALID_ARRAY_TO_STR(val_policingmap_controller.valid);
  ss << "   -->sequence_num "
      << chr2int(val_policingmap_controller.sequence_num) << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const key_vbrif_policingmap_entry&
                                   key_vbrif_policingmap_entry) {
  std::stringstream ss;
  ss << "   -----   key_vbrif_policingmap_entry   -----   " << endl;
  ss << IpcStructToStr(key_vbrif_policingmap_entry.vbrif_key) << endl;
  ss << "   -->sequence_num "
      << chr2int(key_vbrif_policingmap_entry.sequence_num) << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const key_vtermif_policingmap_entry&
                                   key_vtermif_policingmap_entry) {
  std::stringstream ss;
  ss << "   -----   key_vtermif_policingmap_entry   -----   " << endl;
  ss << IpcStructToStr(key_vtermif_policingmap_entry.vterm_if_key) << endl;
  ss << "   -->sequence_num "
      << chr2int(key_vtermif_policingmap_entry.sequence_num) << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const key_ctr &data) {
  std::stringstream ss;
  ss << "   -----   key_ctr   -----   " << endl;
  ss << "   -->controller_name " << data.controller_name << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const val_ctr &data) {
  std::stringstream ss;
  ss << "   -----   val_ctr   -----   " << endl;
  ss << "   -->type " << chr2int(data.type) << endl;
  ss << "   -->version " << data.version << endl;
  ss << "   -->description " << data.description << endl;
  ss << "   -->ip_address " << Ipv4AddrToStr(data.ip_address) << endl;
  ss << "   -->user " << data.user << endl;
  ss << "   -->password " << data.password << endl;
  ss << "   -->enable_audit " << data.enable_audit << endl;
  ss << "   -->valid " << VALID_ARRAY_TO_STR(data.valid);
  ss << "   -->cs_row_status " << data.cs_row_status << endl;
  ss << "   -->cs_attr " << CS_ARRAY_TO_STR(data.cs_attr);
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const val_ctr_st &data) {
  std::stringstream ss;
  ss << "   -----   val_ctr_st   -----   " << endl;
  ss << "   -->controller";
  ss << IpcStructToStr(data.controller);
  ss << "   -->actual_version " << data.actual_version << endl;
  ss << "   -->oper_status " << chr2int(data.oper_status) << endl;
  ss << "   -->valid " << VALID_ARRAY_TO_STR(data.valid);
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const key_ctr_domain &data) {
  std::stringstream ss;
  ss << "   -----   key_ctr_domain   -----   " << endl;
  // ss << "   -->ctr_key ";
  ss << IpcStructToStr(data.ctr_key);
  ss << "   -->domain_name " << data.domain_name << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const val_ctr_domain &data) {
  std::stringstream ss;
  ss << "   -----   val_ctr_domain   -----   " << endl;
  ss << "   -->Type " << chr2int(data.type) << endl;
  ss << "   -->description " << data.description << endl;
  ss << "   -->valid " << VALID_ARRAY_TO_STR(data.valid);
  ss << "   -->cs_row_status " << data.cs_row_status << endl;
  ss << "   -->cs_attr " << CS_ARRAY_TO_STR(data.cs_attr);
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const val_ctr_domain_st &data) {
  std::stringstream ss;
  ss << "   -----   val_ctr_domain_st   -----   " << endl;
  ss << "   -->valid " << VALID_ARRAY_TO_STR(data.valid);
  ss << IpcStructToStr(data.domain);
  ss << "   -->oper_status " << chr2int(data.oper_status) << endl;
  return ss.str();
}
std::string KtUtil::IpcStructToStr(const key_logical_port &data) {
  std::stringstream ss;
  ss << "   -----   key_logical_port   -----   " << endl;
  // ss << "   -->domain_key ";
  ss << IpcStructToStr(data.domain_key);
  ss << "   -->port_id " << data.port_id << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const val_logical_port &data) {
  std::stringstream ss;
  ss << "   -----   val_logical_port   -----   " << endl;
  ss << "   -->description " << data.description << endl;
  ss << "   -->port_type " << chr2int(data.port_type) << endl;
  ss << "   -->switch_id " << data.switch_id << endl;
  ss << "   -->physical_port_id " << data.physical_port_id << endl;
  ss << "   -->oper_down_criteria "
      << chr2int(data.oper_down_criteria) << endl;
  ss << "   -->valid " << VALID_ARRAY_TO_STR(data.valid);
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const val_logical_port_st &data) {
  std::stringstream ss;
  ss << "   -----   val_logical_port_st   -----   " << endl;
  ss << "   -->logical_port";
  ss << IpcStructToStr(data.logical_port);
  ss << "   -->oper_status " << chr2int(data.oper_status) << endl;
  ss << "   -->valid " << VALID_ARRAY_TO_STR(data.valid);
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const key_boundary &data) {
  std::stringstream ss;
  ss << "   -----   key_boundary   -----   " << endl;
  ss << "   -->boundary_id " << data.boundary_id << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const val_boundary &data) {
  std::stringstream ss;
  ss << "   -----   val_boundary   -----   " << endl;
  ss << "   -->description " << data.description << endl;
  ss << "   -->controller_name1 " << data.controller_name1 << endl;
  ss << "   -->domain_name1 " << data.domain_name1 << endl;
  ss << "   -->logical_port_id1 " << data.logical_port_id1 << endl;
  ss << "   -->controller_name2 " << data.controller_name2 << endl;
  ss << "   -->domain_name2 " << data.domain_name2 << endl;
  ss << "   -->logical_port_id2 " << data.logical_port_id2 << endl;
  ss << "   -->valid " << VALID_ARRAY_TO_STR(data.valid);
  ss << "   -->cs_row_status " << chr2int(data.cs_row_status) << endl;
  ss << "   -->cs_attr " << CS_ARRAY_TO_STR(data.cs_attr);
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const val_boundary_st &data) {
  std::stringstream ss;
  ss << "   -----   val_boundary_st   -----   " << endl;
  ss << "   -->oper_status " << chr2int(data.oper_status) << endl;
  ss << "   -->valid " << VALID_ARRAY_TO_STR(data.valid);
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const val_path_fault_alarm &data) {
  std::stringstream ss;
  ss << "   -----   val_path_fault_alarm   -----   " << endl;
  ss << "   -->ingress_logical_port " << data.ingress_logical_port << endl;
  ss << "   -->egress_logical_port " << data.egress_logical_port << endl;
  ss << "   -->ingress_num_of_ports " << data.ingress_num_of_ports << endl;
  ss << "   -->egress_num_of_ports " << data.egress_num_of_ports << endl;
  ss << "   -->valid " << VALID_ARRAY_TO_STR(data.valid);
  return ss.str();
}

// TODO(a) PFC-DRV
std::string KtUtil::IpcStructToStr(const pfcdrv_val_vbr_if &data) {
  std::stringstream ss;
  ss << "   -----   pfcdrv_val_vbrif   -----   " << endl;
  ss << "   -->valid " << VALID_ARRAY_TO_STR(data.valid);
  ss << "   -->val.valid " << VALID_ARRAY_TO_STR(data.val_vbrif.valid);
  ss << "   -->val.admin_status " << chr2int(data.val_vbrif.admin_status)
      << endl;
  ss << "   -->val.description " << data.val_vbrif.description << endl;
  ss << "   -->pm.valid " << VALID_ARRAY_TO_STR(data.val_vbrif.portmap.valid);
  ss << "   -->pm.logical_port_id " << data.val_vbrif.portmap.logical_port_id
      << endl;
  ss << "   -->pm.vlan_id " << data.val_vbrif.portmap.vlan_id << endl;
  ss << "   -->pm.tagged " << chr2int(data.val_vbrif.portmap.tagged) << endl;
  ss << "   -->vext_name " << data.vext_name << endl;
  ss << "   -->vext_if_name " << data.vext_if_name << endl;
  ss << "   -->vlink_name " << data.vlink_name << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const pfcdrv_val_vbrif_vextif &data) {
  std::stringstream ss;
  ss << "   -----   pfcdrv_val_vbrif_vextif   -----   " << endl;
  ss << "   -->valid: " << VALID_ARRAY_TO_STR(data.valid);
  ss << "   -->if_type: " << chr2int(data.interface_type) << endl;
  ss << "   -->vext_name: " << data.vexternal_name << endl;
  ss << "   -->vext_if_name: " << data.vext_if_name << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const pfcdrv_val_flowfilter_entry &data) {
  std::stringstream ss;
  ss << "   -----   pfcdrv_val_vbrif_policingmap   -----   " << endl;
  ss << "   -->valid: " << VALID_ARRAY_TO_STR(data.valid);
  ss << IpcStructToStr(data.val_ff_entry);
  ss << IpcStructToStr(data.val_vbrif_vextif);
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const pfcdrv_val_vbrif_policingmap &data) {
  std::stringstream ss;
  ss << "   -----   pfcdrv_val_vbrif_policingmap   -----   " << endl;
  ss << "   -->valid: " << VALID_ARRAY_TO_STR(data.valid);
  ss << IpcStructToStr(data.val_policing_map);
  ss << IpcStructToStr(data.val_vbrif_vextif);
  return ss.str();
}

/* VlanmapOnBoundary: Added new val struct */
std::string KtUtil::IpcStructToStr(const pfcdrv_val_vlan_map &data) {
  std::stringstream ss;
  ss << "   -----   pfcdrv_val_vlan_map   -----   " << endl;
  ss << "   -->valid " << VALID_ARRAY_TO_STR(data.valid);
  ss << "   -->val.vm.valid " << VALID_ARRAY_TO_STR(data.vm.valid);
  ss << "   -->vm.vlan_id " << data.vm.vlan_id << endl;
  ss << "   -->bdry_ref_count " << data.bdry_ref_count << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const pfcdrv_val_vbr_portmap &data) {
  std::stringstream ss;
  ss << "   -----   pfcdrv_val_vbr_portmap   -----   " << endl;
  ss << "   -->valid " << VALID_ARRAY_TO_STR(data.valid);
  ss << IpcStructToStr(data.vbrpm);
  ss << "   -->bdry_ref_count " << chr2int(data.bdry_ref_count) << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const pfcdrv_val_vtn_controller &data) {
  std::stringstream ss;
  ss << "   -----   pfcdrv_val_vtn_controller   -----   " << endl;
  ss << "   -->valid " << VALID_ARRAY_TO_STR(data.valid);
  ss << "   -->label_type " << chr2int(data.label_type) << endl;
  ss << "   -->label " << data.label << endl;
  return ss.str();
}

// VNP-DRV
std::string KtUtil::IpcStructToStr(const vnpdrv_val_vtunnel& data) {
  std::stringstream ss;
  ss << "   -----   vnpdrv_val_vtunnel   -----   " << endl;
  ss << "   -->valid " << VALID_ARRAY_TO_STR(data.valid);
  ss << "   -->label " << data.label << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const vnpdrv_val_vtunnel_if& data) {
  std::stringstream ss;
  ss << "   -----   vnpdrv_val_vtunnel_if   -----   " << endl;
  ss << "   -->valid " << VALID_ARRAY_TO_STR(data.valid);
  ss << "   -->label " << data.label << endl;
  ss << "   -->vlan_id " << data.vlan_id << endl;
  return ss.str();
}
// convert function definition
std::string KtUtil::IpcStructToStr(const key_convert_vbr &data) {
std::stringstream ss;
ss << "   ----- key_convert_vbr --------------" << endl;
ss << IpcStructToStr(data.vbr_key);
ss << "   -->convert_vbr_name" << data.conv_vbr_name << endl;
return ss.str();
}

std::string KtUtil::IpcStructToStr(const val_convert_vbr &data) {
std::stringstream ss;
ss << "   ----- val_convert_vbr -----   " << endl;
ss << "   -->valid " << VALID_ARRAY_TO_STR(data.valid) << endl;
ss << "   -->label " << chr2int(data.label) << endl;
return ss.str();
}

std::string KtUtil::IpcStructToStr(const key_convert_vbr_if &data) {
  std::stringstream ss;
  ss << "   ----- key_convert_vbr_if --------------" << endl;
  ss << IpcStructToStr(data.convert_vbr_key);
  ss << "   -->convert_vbr_if_name" << data.convert_if_name;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const val_convert_vbr_if&
                                   val_convert_vbrif) {
  std::stringstream ss;
  ss << "   -----   val_convert_vbr_if   -----   " << endl;
  ss << "   -->cs_row_status " <<
      chr2int(val_convert_vbrif.cs_row_status) << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const key_convert_vtunnel &data) {
std::stringstream ss;
ss << "   ----- key_convert_vtunnel --------------" << endl;
ss << IpcStructToStr(data.vtn_key);
ss << "   -->convert_vtunnel_name   " << data.convert_vtunnel_name << endl;
return ss.str();
}

std::string KtUtil::IpcStructToStr(const val_convert_vtunnel &data) {
std::stringstream ss;
ss << "   ----- val_convert_vtunnel -----   " << endl;
ss << "   -->valid " << VALID_ARRAY_TO_STR(data.valid) << endl;
ss << "   -->ref_count       " << data.ref_count << endl;
ss << "   -->label           " << data.label << endl;
return ss.str();
}

std::string KtUtil::IpcStructToStr(const key_convert_vtunnel_if &data) {
std::stringstream ss;
ss << "   ----- key_convert_vtunnel_if --------------" << endl;
ss << IpcStructToStr(data.convert_vtunnel_key);
ss << "   -->convert_vtunnel_if_name   " << data.convert_if_name << endl;
return ss.str();
}

std::string KtUtil::IpcStructToStr(const val_convert_vtunnel_if &data) {
std::stringstream ss;
ss << "   ----- val_convert_vtunnel_if -----   " << endl;
ss << "   -->valid " << VALID_ARRAY_TO_STR(data.valid) << endl;
ss << "   -->un_vbr_name     " << data.un_vbr_name << endl;
return ss.str();
}

std::string KtUtil::IpcStructToStr(const key_convert_vlink &data) {
  std::stringstream ss;
  ss << "   ----- key_convert_vlink --------------" << endl;
  ss << IpcStructToStr(data.vbr_key);
  ss << "   -->convert_vlink_name" << data.convert_vlink_name;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(
    const val_convert_vlink &convert_val_vlink) {
  std::stringstream ss;
  ss << "   ----- val_convert_vlink -----   " << endl;
  ss << "   -->valid " << VALID_ARRAY_TO_STR(convert_val_vlink.valid) << endl;
  ss << "   -->vnode1_name " << convert_val_vlink.vnode1_name << endl;
  ss << "   -->vnode1_ifname " << convert_val_vlink.vnode1_ifname << endl;
  ss << "   -->vnode2_name " << convert_val_vlink.vnode2_name << endl;
  ss << "   -->vnode2_ifname " << convert_val_vlink.vnode2_ifname << endl;
  ss << "   -->boundary_name " << convert_val_vlink.boundary_name << endl;
  ss << "   -->label_type " << chr2int(convert_val_vlink.label_type) << endl;
  ss << "   -->label " << chr2int(convert_val_vlink.label) << endl;

  return ss.str();
}

std::string KtUtil::IpcStructToStr(const val_vbr_expand &data) {
  std::stringstream ss;
  ss << "   ----------- val_vbr_expand --------------" << endl;
  ss << "   -->valid " << VALID_ARRAY_TO_STR(data.valid) << endl;
  ss << "   -->vbridge_name " << data.vbridge_name  << endl;
  ss << "   -->controller_id " << data.controller_id << endl;
  ss << "   -->domain_id " << data.domain_id << endl;
  ss << "   -->label " << chr2int(data.label) << endl;
  ss << "   -->controller_vtn_name " << data.controller_vtn_name << endl;
  ss << "   -->controller_vtn_label " << data.controller_vtn_label << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const val_vbr_portmap_expand &data) {
  std::stringstream ss;
  ss << "   ----------- val_vbr_portmap_expand --------------" << endl;
  ss << "   -->valid " << VALID_ARRAY_TO_STR(data.valid) << endl;
  ss << "   -->portmap_id " << data.portmap_id << endl;
  ss << "   -->logical_port_id " << data.logical_port_id << endl;
  ss << "   -->label_type " << chr2int(data.label_type) << endl;
  ss << "   -->label " << chr2int(data.label) << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const val_vbr_if_expand &data) {
  std::stringstream ss;
  ss << "   ----------- val_vbr_if_expand --------------" << endl;
  ss << "   -->valid " << VALID_ARRAY_TO_STR(data.valid) << endl;
  ss << "   -->if_name " << data.if_name << endl;
  ss << "   -->connected_vnode_name " << data.connected_vnode_name << endl;
  ss << "   -->connected_if_name " << data.connected_if_name << endl;
  ss << "   -->connected_vlink_name " << data.connected_vlink_name << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const val_vtunnel_expand &data) {
  std::stringstream ss;
  ss << "   ----------- val_vtunnel_expand --------------" << endl;
  ss << "   -->valid " << VALID_ARRAY_TO_STR(data.valid) << endl;
  ss << "   -->vtunnel_name " << data.vtunnel_name  << endl;
  ss << "   -->controller_id " << data.controller_id << endl;
  ss << "   -->domain_id " << data.domain_id << endl;
  ss << "   -->label " << chr2int(data.label) << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const val_vtunnel_if_expand &data) {
  std::stringstream ss;
  ss << "   ----------- val_vtunnel_if_expand --------------" << endl;
  ss << "   -->valid " << VALID_ARRAY_TO_STR(data.valid) << endl;
  ss << "   -->if_name " << data.if_name << endl;
  ss << "   -->connected_vnode_name " << data.connected_vnode_name << endl;
  ss << "   -->connected_if_name " << data.connected_if_name << endl;
  ss << "   -->connected_vlink_name " << data.connected_vlink_name << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const val_vlink_expand &data) {
  std::stringstream ss;
  ss << "   ----------- val_vlink_expand --------------" << endl;
  ss << "   -->valid " << VALID_ARRAY_TO_STR(data.valid) << endl;
  ss << "   -->vlink_name " << data.vlink_name << endl;
  ss << "   -->vnode1_name " << data.vnode1_name << endl;
  ss << "   -->vnode1_ifname " << data.vnode1_ifname << endl;
  ss << "   -->vnode2_name " << data.vnode2_name << endl;
  ss << "   -->vnode2_ifname " << data.vnode2_ifname << endl;
  ss << "   -->boundary_name " << data.boundary_name << endl;
  ss << "   -->label_type " << chr2int(data.label_type) << endl;
  ss << "   -->label " << chr2int(data.label) << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const val_vtn_gateway_port &data) {
std::stringstream ss;
  ss << "   ----- val_vtn_gateway_port -----   " << endl;
  ss << "   -->valid " << VALID_ARRAY_TO_STR(data.valid) << endl;
  ss << "   -->logical_port_id " << data.logical_port_id << endl;
  ss << "   -->label           " << data.label << endl;
  ss << "   -->ref_count       " << data.ref_count << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const key_vbid_label &data) {
  std::stringstream ss;
  ss << "   ----- key_vbid_label --------------" << endl;
  ss << IpcStructToStr(data.vtn_key);
  ss << "   -->row_no " << data.label_row << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const val_vbid_label &data) {
  std::stringstream ss;
  ss << "   ----- val_vbid_label -----   " << endl;
  ss << "   -->label " << data.label_id << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const key_gvtnid_label &data) {
  std::stringstream ss;
  ss << "   ----- key_gvtnid_label --------------" << endl;
  ss << "   -->data.ctrlr_name " << data.ctrlr_name << endl;
  ss << "   -->data.domain_name " << data.domain_name << endl;
  ss << "   -->row_no" << data.label_row << endl;
  return ss.str();
}

std::string KtUtil::IpcStructToStr(const val_gvtnid_label &data) {
  std::stringstream ss;
  ss << "   ----- val_gvtnid_label -----   " << endl;
  ss << "   -->label " << data.label_id << endl;
  return ss.str();
}
}  // namespace ipc_util
}  // namespace upll
}  // namespace unc

