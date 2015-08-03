/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <sstream>
#include "flowlist_entry_momgr.hh"
#include "flowlist_momgr.hh"
#include "policingprofile_entry_momgr.hh"
#include "ctrlr_capa_defines.hh"
#include "upll_validation.hh"
#include "uncxx/upll_log.hh"
#include "dal/dal_odbc_mgr.hh"
#include "upll_db_query.hh"
using std::ostringstream;

#define NUM_FL_KEY_MAIN_COL 3
#define NUM_FL_KEY_CTRLR_COL 3
#define RENAME_FLOWLIST 0x01

#define GET_VALID_MAINCTRL(tbl, l_val_ctrl_ff, l_val_ff, en) \
  (tbl == MAINTBL) ? &(l_val_ff->valid[en]) : &(l_val_ctrl_ff->valid[en])
#define FLOWLIST_COL_INDEX_MAINTBL 0
#define FLOWLIST_COL_INDEX_CTRLTBL 0
#define FLE_CTRL_COL_INDEX_CTRLTBL 2
using unc::upll::ipc_util::IpcUtil;
namespace unc {
namespace upll {
namespace kt_momgr {

// FlowListEntry Table(Main Table)
BindInfo FlowListEntryMoMgr::flowlistentry_bind_info[] = {
  { uudst::flowlist_entry::kDbiFlowListName,
    CFG_KEY,
    offsetof(key_flowlist_entry_t, flowlist_key.flowlist_name),
    uud::kDalChar, kMaxLenFlowListName+1},
  { uudst::flowlist_entry::kDbiSequenceNum,
    CFG_KEY,
    offsetof(key_flowlist_entry_t, sequence_num),
    uud::kDalUint16, 1 },
  { uudst::flowlist_entry::kDbiMacDst,
    CFG_VAL,
    offsetof(val_flowlist_entry_t, mac_dst),
    uud::kDalChar, 6 },
  { uudst::flowlist_entry::kDbiMacSrc,
    CFG_VAL,
    offsetof(val_flowlist_entry_t, mac_src),
    uud::kDalChar, 6 },
  { uudst::flowlist_entry::kDbiMacEthType,
    CFG_VAL,
    offsetof(val_flowlist_entry_t, mac_eth_type),
    uud::kDalUint16, 1 },
  { uudst::flowlist_entry::kDbiDstIp,
    CFG_VAL,
    offsetof(val_flowlist_entry_t, dst_ip),
    uud::kDalUint32, 1 },
  { uudst::flowlist_entry::kDbiDstIpPrefix,
    CFG_VAL,
    offsetof(val_flowlist_entry_t, dst_ip_prefixlen),
    uud::kDalUint8, 1 },
  { uudst::flowlist_entry::kDbiSrcIp,
    CFG_VAL,
    offsetof(val_flowlist_entry_t, src_ip),
    uud::kDalUint32, 1 },
  { uudst::flowlist_entry::kDbiSrcIpPrefix,
    CFG_VAL,
    offsetof(val_flowlist_entry_t, src_ip_prefixlen),
    uud::kDalUint8, 1 },
  { uudst::flowlist_entry::kDbiVlanPriority,
    CFG_VAL,
    offsetof(val_flowlist_entry_t, vlan_priority),
    uud::kDalUint8, 1 },
  { uudst::flowlist_entry::kDbiDstIpV6,
    CFG_VAL,
    offsetof(val_flowlist_entry_t, dst_ipv6),
    uud::kDalChar, 16 },
  { uudst::flowlist_entry::kDbiDstIpV6Prefix,
    CFG_VAL,
    offsetof(val_flowlist_entry_t, dst_ipv6_prefixlen),
    uud::kDalUint8, 1 },
  { uudst::flowlist_entry::kDbiSrcIpV6,
    CFG_VAL,
    offsetof(val_flowlist_entry_t, src_ipv6),
    uud::kDalChar, 16 },
  { uudst::flowlist_entry::kDbiSrcIpV6Prefix,
    CFG_VAL,
    offsetof(val_flowlist_entry_t, src_ipv6_prefixlen),
    uud::kDalUint8, 1 },
  { uudst::flowlist_entry::kDbiIpProtocol,
    CFG_VAL,
    offsetof(val_flowlist_entry_t, ip_proto),
    uud::kDalUint8, 1 },
  { uudst::flowlist_entry::kDbiIpDscp,
    CFG_VAL,
    offsetof(val_flowlist_entry_t, ip_dscp),
    uud::kDalUint8, 1 },
  { uudst::flowlist_entry::kDbiL4DstPort,
    CFG_VAL,
    offsetof(val_flowlist_entry_t, l4_dst_port),
    uud::kDalUint16, 1 },
  { uudst::flowlist_entry::kDbiL4DstPortEndpt,
    CFG_VAL,
    offsetof(val_flowlist_entry_t, l4_dst_port_endpt),
    uud::kDalUint16, 1 },
  { uudst::flowlist_entry::kDbiL4SrcPort,
    CFG_VAL,
    offsetof(val_flowlist_entry_t, l4_src_port),
    uud::kDalUint16, 1 },
  { uudst::flowlist_entry::kDbiL4SrcPortEndpt,
    CFG_VAL,
    offsetof(val_flowlist_entry_t, l4_src_port_endpt),
    uud::kDalUint16, 1 },
  { uudst::flowlist_entry::kDbiIcmpType,
    CFG_VAL,
    offsetof(val_flowlist_entry_t, icmp_type),
    uud::kDalUint8, 1 },
  { uudst::flowlist_entry::kDbiIcmpCode,
    CFG_VAL,
    offsetof(val_flowlist_entry_t, icmp_code),
    uud::kDalUint8, 1 },
  { uudst::flowlist_entry::kDbiIcmpV6Type,
    CFG_VAL,
    offsetof(val_flowlist_entry_t, icmpv6_type),
    uud::kDalUint8, 1 },
  { uudst::flowlist_entry::kDbiIcmpV6Code,
    CFG_VAL,
    offsetof(val_flowlist_entry_t, icmpv6_code),
    uud::kDalUint8, 1 },
  { uudst::flowlist_entry::kDbiFlags,
    CK_VAL,
    offsetof(key_user_data_t, flags),
    uud::kDalUint8, 1 },
  { uudst::flowlist_entry::kDbiValidMacDst,
    CFG_META_VAL,
    offsetof(val_flowlist_entry_t, valid[0]),
    uud::kDalUint8, 1 },
  { uudst::flowlist_entry::kDbiValidMacSrc,
    CFG_META_VAL,
    offsetof(val_flowlist_entry_t, valid[1]),
    uud::kDalUint8, 1 },
  { uudst::flowlist_entry::kDbiValidMacEthType,
    CFG_META_VAL,
    offsetof(val_flowlist_entry_t, valid[2]),
    uud::kDalUint8, 1 },
  { uudst::flowlist_entry::kDbiValidDstIp,
    CFG_META_VAL,
    offsetof(val_flowlist_entry_t, valid[3]),
    uud::kDalUint8, 1 },
  { uudst::flowlist_entry::kDbiValidDstIpPrefix,
    CFG_META_VAL,
    offsetof(val_flowlist_entry_t, valid[4]),
    uud::kDalUint8, 1 },
  { uudst::flowlist_entry::kDbiValidSrcIp,
    CFG_META_VAL,
    offsetof(val_flowlist_entry_t, valid[5]),
    uud::kDalUint8, 1 },
  { uudst::flowlist_entry::kDbiValidSrcIpPrefix,
    CFG_META_VAL,
    offsetof(val_flowlist_entry_t, valid[6]),
    uud::kDalUint8, 1 },
  { uudst::flowlist_entry::kDbiValidVlanPriority,
    CFG_META_VAL,
    offsetof(val_flowlist_entry_t, valid[7]),
    uud::kDalUint8, 1 },
  { uudst::flowlist_entry::kDbiValidDstIpV6,
    CFG_META_VAL,
    offsetof(val_flowlist_entry_t, valid[8]),
    uud::kDalUint8, 1 },
  { uudst::flowlist_entry::kDbiValidDstIpV6Prefix,
    CFG_META_VAL,
    offsetof(val_flowlist_entry_t, valid[9]),
    uud::kDalUint8, 1 },
  { uudst::flowlist_entry::kDbiValidSrcIpV6,
    CFG_META_VAL,
    offsetof(val_flowlist_entry_t, valid[10]),
    uud::kDalUint8, 1 },
  { uudst::flowlist_entry::kDbiValidSrcIpV6Prefix,
    CFG_META_VAL,
    offsetof(val_flowlist_entry_t, valid[11]),
    uud::kDalUint8, 1 },
  { uudst::flowlist_entry::kDbiValidIpProtocol,
    CFG_META_VAL,
    offsetof(val_flowlist_entry_t, valid[12]),
    uud::kDalUint8, 1 },
  { uudst::flowlist_entry::kDbiValidIpDscp,
    CFG_META_VAL,
    offsetof(val_flowlist_entry_t, valid[13]),
    uud::kDalUint8, 1 },
  { uudst::flowlist_entry::kDbiValidL4DstPort,
    CFG_META_VAL,
    offsetof(val_flowlist_entry_t, valid[14]),
    uud::kDalUint8, 1 },
  { uudst::flowlist_entry::kDbiValidL4DstPortEndpt,
    CFG_META_VAL,
    offsetof(val_flowlist_entry_t, valid[15]),
    uud::kDalUint8, 1 },
  { uudst::flowlist_entry::kDbiValidL4SrcPort,
    CFG_META_VAL,
    offsetof(val_flowlist_entry_t, valid[16]),
    uud::kDalUint8, 1 },
  { uudst::flowlist_entry::kDbiValidL4SrcPortEndpt,
    CFG_META_VAL,
    offsetof(val_flowlist_entry_t, valid[17]),
    uud::kDalUint8, 1 },
  { uudst::flowlist_entry::kDbiValidIcmpType,
    CFG_META_VAL,
    offsetof(val_flowlist_entry_t, valid[18]),
    uud::kDalUint8, 1 },
  { uudst::flowlist_entry::kDbiValidIcmpCode,
    CFG_META_VAL,
    offsetof(val_flowlist_entry_t, valid[19]),
    uud::kDalUint8, 1 },
  { uudst::flowlist_entry::kDbiValidIcmpV6Type,
    CFG_META_VAL,
    offsetof(val_flowlist_entry_t, valid[20]),
    uud::kDalUint8, 1 },
  { uudst::flowlist_entry::kDbiValidIcmpV6Code,
    CFG_META_VAL,
    offsetof(val_flowlist_entry_t, valid[21]),
    uud::kDalUint8, 1 },
  { uudst::flowlist_entry::kDbiCsRowStatus,
    CS_VAL,
    offsetof(val_flowlist_entry_t, cs_row_status),
    uud::kDalUint8, 1 },
  { uudst::flowlist_entry::kDbiCsMacDst,
    CS_VAL,
    offsetof(val_flowlist_entry_t, cs_attr[0]),
    uud::kDalUint8, 1 },
  { uudst::flowlist_entry::kDbiCsMacSrc,
    CS_VAL,
    offsetof(val_flowlist_entry_t, cs_attr[1]),
    uud::kDalUint8, 1 },
  { uudst::flowlist_entry::kDbiCsMacEthType,
    CS_VAL,
    offsetof(val_flowlist_entry_t, cs_attr[2]),
    uud::kDalUint8, 1 },
  { uudst::flowlist_entry::kDbiCsDstIp,
    CS_VAL,
    offsetof(val_flowlist_entry_t, cs_attr[3]),
    uud::kDalUint8, 1 },
  { uudst::flowlist_entry::kDbiCsDstIpPrefix,
    CS_VAL,
    offsetof(val_flowlist_entry_t, cs_attr[4]),
    uud::kDalUint8, 1 },
  { uudst::flowlist_entry::kDbiCsSrcIp,
    CS_VAL,
    offsetof(val_flowlist_entry_t, cs_attr[5]),
    uud::kDalUint8, 1 },
  { uudst::flowlist_entry::kDbiCsSrcIpPrefix,
    CS_VAL,
    offsetof(val_flowlist_entry_t, cs_attr[6]),
    uud::kDalUint8, 1 },
  { uudst::flowlist_entry::kDbiCsVlanPriority,
    CS_VAL,
    offsetof(val_flowlist_entry_t, cs_attr[7]),
    uud::kDalUint8, 1 },
  { uudst::flowlist_entry::kDbiCsDstIpV6,
    CS_VAL,
    offsetof(val_flowlist_entry_t, cs_attr[8]),
    uud::kDalUint8, 1 },
  { uudst::flowlist_entry::kDbiCsDstIpV6Prefix,
    CS_VAL,
    offsetof(val_flowlist_entry_t, cs_attr[9]),
    uud::kDalUint8, 1 },
  { uudst::flowlist_entry::kDbiCsSrcIpV6,
    CS_VAL,
    offsetof(val_flowlist_entry_t, cs_attr[10]),
    uud::kDalUint8, 1 },
  { uudst::flowlist_entry::kDbiCsSrcIpV6Prefix,
    CS_VAL,
    offsetof(val_flowlist_entry_t, cs_attr[11]),
    uud::kDalUint8, 1 },
  { uudst::flowlist_entry::kDbiCsIpProtocol,
    CS_VAL,
    offsetof(val_flowlist_entry_t, cs_attr[12]),
    uud::kDalUint8, 1 },
  { uudst::flowlist_entry::kDbiCsIpDscp,
    CS_VAL,
    offsetof(val_flowlist_entry_t, cs_attr[13]),
    uud::kDalUint8, 1 },
  { uudst::flowlist_entry::kDbiCsL4DstPort,
    CS_VAL,
    offsetof(val_flowlist_entry_t, cs_attr[14]),
    uud::kDalUint8, 1 },
  { uudst::flowlist_entry::kDbiCsL4DstPortEndpt,
    CS_VAL,
    offsetof(val_flowlist_entry_t, cs_attr[15]),
    uud::kDalUint8, 1 },
  { uudst::flowlist_entry::kDbiCsL4SrcPort,
    CS_VAL,
    offsetof(val_flowlist_entry_t, cs_attr[16]),
    uud::kDalUint8, 1 },
  { uudst::flowlist_entry::kDbiCsL4SrcPortEndpt,
    CS_VAL,
    offsetof(val_flowlist_entry_t, cs_attr[17]),
    uud::kDalUint8, 1 },
  { uudst::flowlist_entry::kDbiCsIcmpType,
    CS_VAL,
    offsetof(val_flowlist_entry_t, cs_attr[18]),
    uud::kDalUint8, 1 },
  { uudst::flowlist_entry::kDbiCsIcmpCode,
    CS_VAL,
    offsetof(val_flowlist_entry_t, cs_attr[19]),
    uud::kDalUint8, 1 },
  { uudst::flowlist_entry::kDbiCsIcmpV6Type,
    CS_VAL,
    offsetof(val_flowlist_entry_t, cs_attr[20]),
    uud::kDalUint8, 1 },
  { uudst::flowlist_entry::kDbiCsIcmpV6Code,
    CS_VAL,
    offsetof(val_flowlist_entry_t, cs_attr[21]),
    uud::kDalUint8, 1 } };

// FlowListEntry Controller Table
BindInfo FlowListEntryMoMgr::flowlistentry_controller_bind_info[] = {
  { uudst::flowlist_entry_ctrlr::kDbiFlowListName,
    CFG_KEY,
    offsetof(key_flowlist_entry_t, flowlist_key.flowlist_name),
    uud::kDalChar, (kMaxLenFlowListName + 1) },
  { uudst::flowlist_entry_ctrlr::kDbiSequenceNum,
    CFG_KEY,
    offsetof(key_flowlist_entry_t, sequence_num),
    uud::kDalUint16, 1 },
  { uudst::flowlist_entry_ctrlr::kDbiCtrlrName,
    CK_VAL,
    offsetof(key_user_data_t, ctrlr_id),
    uud::kDalChar, (kMaxLenCtrlrId + 1)},
  { uudst::flowlist_entry_ctrlr::kDbiFlags,
    CK_VAL,
    offsetof(key_user_data_t, flags),
    uud::kDalUint8, 1 },
  { uudst::flowlist_entry_ctrlr::kDbiValidMacDst,
    CFG_META_VAL,
    offsetof(val_flowlist_entry_ctrl_t, valid[0]),
    uud::kDalUint8, 1 },
  { uudst::flowlist_entry_ctrlr::kDbiValidMacSrc,
    CFG_META_VAL,
    offsetof(val_flowlist_entry_ctrl_t, valid[1]),
    uud::kDalUint8, 1 },
  { uudst::flowlist_entry_ctrlr::kDbiValidMacEthType,
    CFG_META_VAL,
    offsetof(val_flowlist_entry_ctrl_t, valid[2]),
    uud::kDalUint8, 1 },
  { uudst::flowlist_entry_ctrlr::kDbiValidDstIp,
    CFG_META_VAL,
    offsetof(val_flowlist_entry_ctrl_t, valid[3]),
    uud::kDalUint8, 1 },
  { uudst::flowlist_entry_ctrlr::kDbiValidDstIpPrefix,
    CFG_META_VAL,
    offsetof(val_flowlist_entry_ctrl_t, valid[4]),
    uud::kDalUint8, 1 },
  { uudst::flowlist_entry_ctrlr::kDbiValidSrcIp,
    CFG_META_VAL,
    offsetof(val_flowlist_entry_ctrl_t, valid[5]),
    uud::kDalUint8, 1 },
  { uudst::flowlist_entry_ctrlr::kDbiValidSrcIpPrefix,
    CFG_META_VAL,
    offsetof(val_flowlist_entry_ctrl_t, valid[6]),
    uud::kDalUint8, 1 },
  { uudst::flowlist_entry_ctrlr::kDbiValidVlanPriority,
    CFG_META_VAL,
    offsetof(val_flowlist_entry_ctrl_t, valid[7]),
    uud::kDalUint8, 1 },
  { uudst::flowlist_entry_ctrlr::kDbiValidDstIpV6,
    CFG_META_VAL,
    offsetof(val_flowlist_entry_ctrl_t, valid[8]),
    uud::kDalUint8, 1 },
  { uudst::flowlist_entry_ctrlr::kDbiValidDstIpV6Prefix,
    CFG_META_VAL,
    offsetof(val_flowlist_entry_ctrl_t, valid[9]),
    uud::kDalUint8, 1 },
  { uudst::flowlist_entry_ctrlr::kDbiValidSrcIpV6,
    CFG_META_VAL,
    offsetof(val_flowlist_entry_ctrl_t, valid[10]),
    uud::kDalUint8, 1 },
  { uudst::flowlist_entry_ctrlr::kDbiValidSrcIpV6Prefix,
    CFG_META_VAL,
    offsetof(val_flowlist_entry_ctrl_t, valid[11]),
    uud::kDalUint8, 1 },
  { uudst::flowlist_entry_ctrlr::kDbiValidIpProtocol,
    CFG_META_VAL,
    offsetof(val_flowlist_entry_ctrl_t, valid[12]),
    uud::kDalUint8, 1 },
  { uudst::flowlist_entry_ctrlr::kDbiValidIpDscp,
    CFG_META_VAL,
    offsetof(val_flowlist_entry_ctrl_t, valid[13]),
    uud::kDalUint8, 1 },
  { uudst::flowlist_entry_ctrlr::kDbiValidL4DstPort,
    CFG_META_VAL,
    offsetof(val_flowlist_entry_ctrl_t, valid[14]),
    uud::kDalUint8, 1 },
  { uudst::flowlist_entry_ctrlr::kDbiValidL4DstPortEndpt,
    CFG_META_VAL,
    offsetof(val_flowlist_entry_ctrl_t, valid[15]),
    uud::kDalUint8, 1 },
  { uudst::flowlist_entry_ctrlr::kDbiValidL4SrcPort,
    CFG_META_VAL,
    offsetof(val_flowlist_entry_ctrl_t, valid[16]),
    uud::kDalUint8, 1 },
  { uudst::flowlist_entry_ctrlr::kDbiValidL4SrcPortEndpt,
    CFG_META_VAL,
    offsetof(val_flowlist_entry_ctrl_t, valid[17]),
    uud::kDalUint8, 1 },
  { uudst::flowlist_entry_ctrlr::kDbiValidIcmpType,
    CFG_META_VAL,
    offsetof(val_flowlist_entry_ctrl_t, valid[18]),
    uud::kDalUint8, 1 },
  { uudst::flowlist_entry_ctrlr::kDbiValidIcmpCode,
    CFG_META_VAL,
    offsetof(val_flowlist_entry_ctrl_t, valid[19]),
    uud::kDalUint8, 1 },
  { uudst::flowlist_entry_ctrlr::kDbiValidIcmpV6Type,
    CFG_META_VAL,
    offsetof(val_flowlist_entry_ctrl_t, valid[20]),
    uud::kDalUint8, 1 },
  { uudst::flowlist_entry_ctrlr::kDbiValidIcmpV6Code,
    CFG_META_VAL,
    offsetof(val_flowlist_entry_ctrl_t, valid[21]),
    uud::kDalUint8, 1 },
  { uudst::flowlist_entry_ctrlr::kDbiCsRowStatus,
    CS_VAL,
    offsetof(val_flowlist_entry_ctrl_t, cs_row_status),
    uud::kDalUint8, 1 },
  { uudst::flowlist_entry_ctrlr::kDbiCsMacDst,
    CS_VAL,
    offsetof(val_flowlist_entry_ctrl_t, cs_attr[0]),
    uud::kDalUint8, 1 },
  { uudst::flowlist_entry_ctrlr::kDbiCsMacSrc,
    CS_VAL,
    offsetof(val_flowlist_entry_ctrl_t, cs_attr[1]),
    uud::kDalUint8, 1 },
  { uudst::flowlist_entry_ctrlr::kDbiCsMacEthType,
    CS_VAL,
    offsetof(val_flowlist_entry_ctrl_t, cs_attr[2]),
    uud::kDalUint8, 1 },
  { uudst::flowlist_entry_ctrlr::kDbiCsDstIp,
    CS_VAL,
    offsetof(val_flowlist_entry_ctrl_t, cs_attr[3]),
    uud::kDalUint8, 1 },
  { uudst::flowlist_entry_ctrlr::kDbiCsDstIpPrefix,
    CS_VAL,
    offsetof(val_flowlist_entry_ctrl_t, cs_attr[4]),
    uud::kDalUint8, 1 },
  { uudst::flowlist_entry_ctrlr::kDbiCsSrcIp,
    CS_VAL,
    offsetof(val_flowlist_entry_ctrl_t, cs_attr[5]),
    uud::kDalUint8, 1 },
  { uudst::flowlist_entry_ctrlr::kDbiCsSrcIpPrefix,
    CS_VAL,
    offsetof(val_flowlist_entry_ctrl_t, cs_attr[6]),
    uud::kDalUint8, 1 },
  { uudst::flowlist_entry_ctrlr::kDbiCsVlanPriority,
    CS_VAL,
    offsetof(val_flowlist_entry_ctrl_t, cs_attr[7]),
    uud::kDalUint8, 1 },
  { uudst::flowlist_entry_ctrlr::kDbiCsDstIpV6,
    CS_VAL,
    offsetof(val_flowlist_entry_ctrl_t, cs_attr[8]),
    uud::kDalUint8, 1 },
  { uudst::flowlist_entry_ctrlr::kDbiCsDstIpV6Prefix,
    CS_VAL,
    offsetof(val_flowlist_entry_ctrl_t, cs_attr[9]),
    uud::kDalUint8, 1 },
  { uudst::flowlist_entry_ctrlr::kDbiCsSrcIpV6,
    CS_VAL,
    offsetof(val_flowlist_entry_ctrl_t, cs_attr[10]),
    uud::kDalUint8, 1 },
  { uudst::flowlist_entry_ctrlr::kDbiCsSrcIpV6Prefix,
    CS_VAL,
    offsetof(val_flowlist_entry_ctrl_t, cs_attr[11]),
    uud::kDalUint8, 1 },
  { uudst::flowlist_entry_ctrlr::kDbiCsIpProtocol,
    CS_VAL,
    offsetof(val_flowlist_entry_ctrl_t, cs_attr[12]),
    uud::kDalUint8, 1 },
  { uudst::flowlist_entry_ctrlr::kDbiCsIpDscp,
    CS_VAL,
    offsetof(val_flowlist_entry_ctrl_t, cs_attr[13]),
    uud::kDalUint8, 1 },
  { uudst::flowlist_entry_ctrlr::kDbiCsL4DstPort,
    CS_VAL,
    offsetof(val_flowlist_entry_ctrl_t, cs_attr[14]),
    uud::kDalUint8, 1 },
  { uudst::flowlist_entry_ctrlr::kDbiCsL4DstPortEndpt,
    CS_VAL,
    offsetof(val_flowlist_entry_ctrl_t, cs_attr[15]),
    uud::kDalUint8, 1 },
  { uudst::flowlist_entry_ctrlr::kDbiCsL4SrcPort,
    CS_VAL,
    offsetof(val_flowlist_entry_ctrl_t, cs_attr[16]),
    uud::kDalUint8, 1 },
  { uudst::flowlist_entry_ctrlr::kDbiCsL4SrcPortEndpt,
    CS_VAL,
    offsetof(val_flowlist_entry_ctrl_t, cs_attr[17]),
    uud::kDalUint8, 1 },
  { uudst::flowlist_entry_ctrlr::kDbiCsIcmpType,
    CS_VAL,
    offsetof(val_flowlist_entry_ctrl_t, cs_attr[18]),
    uud::kDalUint8, 1 },
  { uudst::flowlist_entry_ctrlr::kDbiCsIcmpCode,
    CS_VAL,
    offsetof(val_flowlist_entry_ctrl_t, cs_attr[19]),
    uud::kDalUint8, 1 },
  { uudst::flowlist_entry_ctrlr::kDbiCsIcmpV6Type,
    CS_VAL,
    offsetof(val_flowlist_entry_ctrl_t, cs_attr[20]),
    uud::kDalUint8, 1 },
  { uudst::flowlist_entry_ctrlr::kDbiCsIcmpV6Code,
    CS_VAL,
    offsetof(val_flowlist_entry_ctrl_t, cs_attr[21]),
    uud::kDalUint8, 1 } };

BindInfo FlowListEntryMoMgr::rename_flowlist_entry_main_tbl[] = {
  {uudst::flowlist_entry::kDbiFlowListName,
    CFG_MATCH_KEY,
    offsetof(key_flowlist_entry_t, flowlist_key.flowlist_name),
    uud::kDalChar, kMaxLenFlowListName+1 },
  {uudst::flowlist_entry::kDbiFlowListName,
    CFG_INPUT_KEY,
    offsetof(key_rename_vnode_info_t, new_flowlist_name),
    uud::kDalChar, kMaxLenFlowListName+1},
  { uudst::flowlist_entry::kDbiFlags,
    CK_VAL,
    offsetof(key_user_data_t, flags),
    uud::kDalUint8, 1 }
};

BindInfo FlowListEntryMoMgr::rename_flowlist_entry_ctrlr_tbl[] = {
  {uudst::flowlist_entry_ctrlr::kDbiFlowListName,
    CFG_MATCH_KEY,
    offsetof(key_flowlist_entry_t, flowlist_key.flowlist_name),
    uud::kDalChar, (kMaxLenFlowListName + 1) },
  {uudst::flowlist_entry_ctrlr::kDbiFlowListName,
    CFG_INPUT_KEY,
    offsetof(key_rename_vnode_info_t, new_flowlist_name),
    uud::kDalChar, kMaxLenFlowListName+1},
  { uudst::flowlist_entry_ctrlr::kDbiFlags,
    CK_VAL,
    offsetof(key_user_data_t, flags),
    uud::kDalUint8, 1}
};

bool FlowListEntryMoMgr::GetRenameKeyBindInfo(unc_key_type_t key_type,
    BindInfo *&binfo, int &nattr, MoMgrTables tbl) {
  if (MAINTBL == tbl) {
     nattr = NUM_FL_KEY_MAIN_COL;
     binfo = rename_flowlist_entry_main_tbl;
  } else if (CTRLRTBL == tbl) {
     nattr = NUM_FL_KEY_CTRLR_COL;
     binfo = rename_flowlist_entry_ctrlr_tbl;
  }
  return PFC_TRUE;
}

FlowListEntryMoMgr::FlowListEntryMoMgr() : MoMgrImpl() {
  UPLL_FUNC_TRACE;
  // Rename operation is not support for this KT
  // setting table indexed for rename table to NULL
  ntable = MAX_MOMGR_TBLS;
  table = new Table *[ntable]();

  // For Main Table
  table[MAINTBL] = new Table(uudst::kDbiFlowListEntryTbl,
      UNC_KT_FLOWLIST_ENTRY, flowlistentry_bind_info,
      IpctSt::kIpcStKeyFlowlistEntry, IpctSt::kIpcStValFlowlistEntry,
      uudst::flowlist_entry::kDbiFlowListEntryNumCols);

  // rename table is not used
  table[RENAMETBL] = NULL;

  // For Controller Table
  table[CTRLRTBL] = new Table(uudst::kDbiFlowListEntryCtrlrTbl,
      UNC_KT_FLOWLIST_ENTRY, flowlistentry_controller_bind_info,
      IpctSt::kIpcStKeyFlowlistEntry, IpctSt::kIpcInvalidStNum,
      uudst::flowlist_entry_ctrlr::kDbiFlowListEntryCtrlrNumCols);
  table[CONVERTTBL] = NULL;
  nchild = 0;
  child = NULL;
}



upll_rc_t FlowListEntryMoMgr::GetValid(void *val,
                                       uint64_t indx,
                                       uint8_t *&valid,
                                       upll_keytype_datatype_t dt_type,
                                       MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  val_flowlist_entry_t *val_fle = NULL;
  val_flowlist_entry_ctrl_t *val_fle_ctrl = NULL;
  if (val == NULL) {
    UPLL_LOG_DEBUG("Memory is not Allocated");
    return UPLL_RC_ERR_GENERIC;
  }
  if (tbl == MAINTBL) {
    val_fle = reinterpret_cast<val_flowlist_entry_t*>(val);
  } else if (tbl == CTRLRTBL) {
    val_fle_ctrl = reinterpret_cast<val_flowlist_entry_ctrl_t*>(val);
  } else {
    valid = NULL;
    return UPLL_RC_ERR_GENERIC;
  }

  switch (indx) {
    case uudst::flowlist_entry::kDbiMacDst:
      valid  = GET_VALID_MAINCTRL(tbl, val_fle_ctrl, val_fle,
                                  UPLL_IDX_MAC_DST_FLE);
      break;
    case uudst::flowlist_entry::kDbiMacSrc:
      valid  = GET_VALID_MAINCTRL(tbl, val_fle_ctrl, val_fle,
                                  UPLL_IDX_MAC_SRC_FLE);
      break;
    case uudst::flowlist_entry::kDbiMacEthType:
      valid  = GET_VALID_MAINCTRL(tbl, val_fle_ctrl, val_fle,
                                  UPLL_IDX_MAC_ETH_TYPE_FLE);
      break;
    case uudst::flowlist_entry::kDbiDstIp:
      valid  = GET_VALID_MAINCTRL(tbl, val_fle_ctrl, val_fle,
                                  UPLL_IDX_DST_IP_FLE);
      break;
    case uudst::flowlist_entry::kDbiDstIpPrefix:
      valid  = GET_VALID_MAINCTRL(tbl, val_fle_ctrl, val_fle,
                                  UPLL_IDX_DST_IP_PREFIX_FLE);
      break;
    case uudst::flowlist_entry::kDbiSrcIp:
      valid  = GET_VALID_MAINCTRL(tbl, val_fle_ctrl, val_fle,
                                  UPLL_IDX_SRC_IP_FLE);
      break;
    case uudst::flowlist_entry::kDbiSrcIpPrefix:
      valid  = GET_VALID_MAINCTRL(tbl, val_fle_ctrl, val_fle,
                                  UPLL_IDX_SRC_IP_PREFIX_FLE);
      break;
    case uudst::flowlist_entry::kDbiVlanPriority:
      valid  = GET_VALID_MAINCTRL(tbl, val_fle_ctrl, val_fle,
                                  UPLL_IDX_VLAN_PRIORITY_FLE);
      break;
    case uudst::flowlist_entry::kDbiDstIpV6:
      valid  = GET_VALID_MAINCTRL(tbl, val_fle_ctrl, val_fle,
                                  UPLL_IDX_DST_IP_V6_FLE);
      break;
    case uudst::flowlist_entry::kDbiDstIpV6Prefix:
      valid  = GET_VALID_MAINCTRL(tbl, val_fle_ctrl, val_fle,
                                  UPLL_IDX_DST_IP_V6_PREFIX_FLE);
      break;
    case uudst::flowlist_entry::kDbiSrcIpV6:
      valid  = GET_VALID_MAINCTRL(tbl, val_fle_ctrl, val_fle,
                                  UPLL_IDX_SRC_IP_V6_FLE);
      break;
    case uudst::flowlist_entry::kDbiSrcIpV6Prefix:
      valid  = GET_VALID_MAINCTRL(tbl, val_fle_ctrl, val_fle,
                                  UPLL_IDX_SRC_IP_V6_PREFIX_FLE);
      break;
    case uudst::flowlist_entry::kDbiIpProtocol:
      valid  = GET_VALID_MAINCTRL(tbl, val_fle_ctrl, val_fle,
                                  UPLL_IDX_IP_PROTOCOL_FLE);
      break;
    case uudst::flowlist_entry::kDbiIpDscp:
      valid  = GET_VALID_MAINCTRL(tbl, val_fle_ctrl, val_fle,
                                  UPLL_IDX_IP_DSCP_FLE);
      break;
    case uudst::flowlist_entry::kDbiL4DstPort:
      valid  = GET_VALID_MAINCTRL(tbl, val_fle_ctrl, val_fle,
                                  UPLL_IDX_L4_DST_PORT_FLE);
      break;
    case uudst::flowlist_entry::kDbiL4DstPortEndpt:
      valid  = GET_VALID_MAINCTRL(tbl, val_fle_ctrl, val_fle,
                                  UPLL_IDX_L4_DST_PORT_ENDPT_FLE);
      break;
    case uudst::flowlist_entry::kDbiL4SrcPort:
      valid  = GET_VALID_MAINCTRL(tbl, val_fle_ctrl, val_fle,
                                  UPLL_IDX_L4_SRC_PORT_FLE);
      break;
    case uudst::flowlist_entry::kDbiL4SrcPortEndpt:
      valid  = GET_VALID_MAINCTRL(tbl, val_fle_ctrl, val_fle,
                                  UPLL_IDX_L4_SRC_PORT_ENDPT_FLE);
      break;
    case uudst::flowlist_entry::kDbiIcmpType:
      valid  = GET_VALID_MAINCTRL(tbl, val_fle_ctrl, val_fle,
                                  UPLL_IDX_ICMP_TYPE_FLE);
      break;
    case uudst::flowlist_entry::kDbiIcmpCode:
      valid  = GET_VALID_MAINCTRL(tbl, val_fle_ctrl, val_fle,
                                  UPLL_IDX_ICMP_CODE_FLE);
      break;
    case uudst::flowlist_entry::kDbiIcmpV6Type:
      valid  = GET_VALID_MAINCTRL(tbl, val_fle_ctrl, val_fle,
                                  UPLL_IDX_ICMP_V6_TYPE_FLE);
      break;
    case uudst::flowlist_entry::kDbiIcmpV6Code:
      valid  = GET_VALID_MAINCTRL(tbl, val_fle_ctrl, val_fle,
                                  UPLL_IDX_ICMP_V6_CODE_FLE);
      break;
    default :
      valid = NULL;
      return UPLL_RC_ERR_GENERIC;
  }
  UPLL_LOG_DEBUG("GetValidAttributte is Succesfull");
  return UPLL_RC_SUCCESS;
}

/**
 * @brief     Allocates Memory for the Incoming Pointer to the Class.
 * @param[out] ck_val     This Contains the pointer to the Class for
 *                        which memory has to be allocated.
 * @param[in]  dt_type    Describes Configiration Information.
 * @param[in]  tbl        Describes the Destination table Information.
 * @retval     RT_SUCCESS Successfull completion.
 */

upll_rc_t FlowListEntryMoMgr::AllocVal(ConfigVal *&ck_val,
    upll_keytype_datatype_t dt_type, MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  void *val = NULL;
  uint32_t index;
  if (ck_val != NULL) {
    UPLL_LOG_DEBUG("Already COntains some Data .AllocVal Fails");
    return UPLL_RC_ERR_GENERIC;
  }

  switch (tbl) {
  case MAINTBL:
    val = ConfigKeyVal::Malloc(sizeof(val_flowlist_entry_t));
      ck_val = new ConfigVal(IpctSt::kIpcStValFlowlistEntry, val);
    break;
  case CTRLRTBL:
    val = ConfigKeyVal::Malloc(sizeof(val_flowlist_entry_ctrl));
      for (index = UPLL_IDX_MAC_DST_FLE; index <= UPLL_IDX_ICMP_V6_CODE_FLE;
            index++) {
          reinterpret_cast<val_flowlist_entry_ctrl *>(val)->valid[index] =
              UNC_VF_INVALID;
        }
      ck_val = new ConfigVal(IpctSt::kIpcInvalidStNum, val);
    break;
  default:
    val = NULL;
    break;
  }
  if (val == NULL) {
    UPLL_LOG_DEBUG("Failed to allocate memory for val structure");
    return UPLL_RC_ERR_GENERIC;
  }
  UPLL_LOG_DEBUG(" AllocVal Successfull.");
  return UPLL_RC_SUCCESS;
}

bool FlowListEntryMoMgr::IsValidKey(void *key, uint64_t index,
                                    MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  key_flowlist_entry_t *flowlist_entry_key =
    reinterpret_cast<key_flowlist_entry_t*> (key);
  upll_rc_t ret_val;
  if (uudst::flowlist_entry::kDbiFlowListName == index) {
    ret_val = ValidateKey(reinterpret_cast<char *>
        (flowlist_entry_key->flowlist_key.flowlist_name),
        kMinLenFlowListName,
        kMaxLenFlowListName);
    if (ret_val != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("FlowList Name is not valid(%d)", ret_val);
      return false;
    }
  }
  if (uudst::flowlist_entry::kDbiSequenceNum == index) {
    if (!ValidateNumericRange(flowlist_entry_key->sequence_num,
          kMinFlowFilterSeqNum, kMaxFlowFilterSeqNum, true,
          true)) {
      UPLL_LOG_DEBUG(" Sequence Number syntax validation failed ");
      return false;
    }
  }
  return true;
}

/**
 * @brief Method used to fill the CongigKeyVal with the Parent Class Information
 * @param[out] okey        This Contains the pointer to the ConfigKeyVal
 *                         Class for which fields have to be updated with
 *                         values from the parent Class.
 * @param[in]  parent_key  This Contains the pointer to the ConfigKeyVal
 *                         Class which is the Parent Class used to
 *                         fill the details.
 * @retval     RT_SUCCESS  Successfull completion.
 */
upll_rc_t FlowListEntryMoMgr::GetChildConfigKey(ConfigKeyVal *&okey,
                                                ConfigKeyVal *parent_key) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  key_flowlist_entry_t *key_fle = NULL;
  void *pkey = NULL;

  if (parent_key == NULL) {
    key_fle = reinterpret_cast<key_flowlist_entry_t*>
        (ConfigKeyVal::Malloc(sizeof(key_flowlist_entry_t)));
    okey = new ConfigKeyVal(UNC_KT_FLOWLIST_ENTRY,
                            IpctSt::kIpcStKeyFlowlistEntry, key_fle, NULL);
    UPLL_LOG_DEBUG("Parent Key Filled %d", result_code);
    return UPLL_RC_SUCCESS;
  } else {
    pkey = parent_key->get_key();
  }

  if (!pkey) {
//    if (key_fle) free(key_fle);
    return UPLL_RC_ERR_GENERIC;
  }
  if (okey) {
    if (okey->get_key_type() != UNC_KT_FLOWLIST_ENTRY)
      return UPLL_RC_ERR_GENERIC;
  }
  if ((okey) && (okey->get_key())) {
    key_fle = reinterpret_cast<key_flowlist_entry_t *>(okey->get_key());
  } else {
    key_fle = reinterpret_cast<key_flowlist_entry_t *>
        (ConfigKeyVal::Malloc(sizeof(key_flowlist_entry_t)));
  }
  switch (parent_key->get_key_type()) {
    case UNC_KT_FLOWLIST:
      uuu::upll_strncpy(
          key_fle->flowlist_key.flowlist_name,
          reinterpret_cast<key_flowlist_t *>
          (pkey)->flowlist_name, (kMaxLenFlowListName+1));
      break;
    case UNC_KT_FLOWLIST_ENTRY:
      uuu::upll_strncpy(
          key_fle->flowlist_key.flowlist_name,
          reinterpret_cast<key_flowlist_entry_t *>
          (pkey)->flowlist_key.flowlist_name, (kMaxLenFlowListName+1));
      key_fle->sequence_num = reinterpret_cast<key_flowlist_entry_t *>
          (pkey)->sequence_num;
      break;
    default:
      if ((!okey) || (!okey->get_key())) FREE_IF_NOT_NULL(key_fle);
      return UPLL_RC_ERR_GENERIC;
      break;
  }

  if ((okey) && !(okey->get_key())) {
    UPLL_LOG_DEBUG("okey not null and flow list name updated");
    okey->SetKey(IpctSt::kIpcStKeyFlowlistEntry, key_fle);
  }

  if (!okey)
    okey = new ConfigKeyVal(UNC_KT_FLOWLIST_ENTRY,
                            IpctSt::kIpcStKeyFlowlistEntry,
                            key_fle, NULL);

  SET_USER_DATA(okey, parent_key);
  UPLL_LOG_DEBUG("okey filled Succesfully %d", result_code);
  return result_code;
}

upll_rc_t FlowListEntryMoMgr::GetRenamedUncKey(ConfigKeyVal *ctrlr_key,
    upll_keytype_datatype_t dt_type, DalDmlIntf *dmi, uint8_t *ctrlr_id) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code;
  ConfigKeyVal *unc_key = NULL;
  UPLL_LOG_TRACE("%s GetRenamedUncKey fl_entry start",
                  ctrlr_key->ToStrAll().c_str());
  key_flowlist_entry_t *ctrlr_flowlist_entry_key =
      reinterpret_cast<key_flowlist_entry_t *> (ctrlr_key->get_key());
  if (NULL == ctrlr_flowlist_entry_key) {
    UPLL_LOG_DEBUG("ctrlr_flowlist_entry_key NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  DbSubOp dbop = { kOpReadSingle, kOpMatchCtrlr, kOpInOutNone };

  val_rename_flowlist_t *rename_flowlist =
  reinterpret_cast<val_rename_flowlist_t *>
  (ConfigKeyVal::Malloc(sizeof(val_rename_flowlist_t)));
  uuu::upll_strncpy(rename_flowlist->flowlist_newname,
                    ctrlr_flowlist_entry_key->flowlist_key.flowlist_name,
           (kMaxLenFlowListName+1));
  rename_flowlist->valid[UPLL_IDX_RENAME_FLOWLIST_RFL] = UNC_VF_VALID;

  FlowListMoMgr *mgr =
    reinterpret_cast<FlowListMoMgr *>(const_cast<MoManager *>(GetMoManager(
            UNC_KT_FLOWLIST)));
  if (!mgr) {
    UPLL_LOG_TRACE("mgr failed");
    free(rename_flowlist);
    return UPLL_RC_ERR_GENERIC;
  }

  result_code = mgr->GetChildConfigKey(unc_key, NULL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetChildConfigKey failed %d", result_code);
    free(rename_flowlist);
    mgr = NULL;
    return result_code;
  }
  if (!unc_key) {
    UPLL_LOG_DEBUG("unc_key NULL");
    free(rename_flowlist);
    mgr = NULL;
    return result_code;
  }
  unc_key->AppendCfgVal(IpctSt::kIpcStValRenameFlowlist,
      rename_flowlist);

  UPLL_LOG_DEBUG("ctrlr_id fle (%s)", ctrlr_id);
  SET_USER_DATA_CTRLR(unc_key, ctrlr_id);

  result_code = mgr->ReadConfigDB(unc_key, dt_type, UNC_OP_READ, dbop, dmi,
      RENAMETBL);
  if (result_code == UPLL_RC_SUCCESS) {
    key_flowlist_entry_t *flowlist_entry =
        reinterpret_cast<key_flowlist_entry_t *> (unc_key->get_key());
    uuu::upll_strncpy(ctrlr_flowlist_entry_key->flowlist_key.flowlist_name,
                      flowlist_entry->flowlist_key.flowlist_name,
                      (kMaxLenFlowListName+1));
    SET_USER_DATA(ctrlr_key, unc_key);
    SET_USER_DATA_FLAGS(ctrlr_key, FL_RENAME);
  }
  UPLL_LOG_TRACE("%s GetRenamedUncKey fl_entry end",
                  ctrlr_key->ToStrAll().c_str());
  DELETE_IF_NOT_NULL(unc_key);
  mgr = NULL;
  if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE)
     result_code = UPLL_RC_SUCCESS;
  return result_code;
}

upll_rc_t FlowListEntryMoMgr::ChkFlowlistNameInRenameTbl(
    ConfigKeyVal *ctrlr_key, upll_keytype_datatype_t dt_type, DalDmlIntf *dmi,
    const char *ctrlr_id) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code;
  ConfigKeyVal *unc_key = NULL;
  UPLL_LOG_TRACE("%s GetRenamedUncKey fl_entry start",
                  ctrlr_key->ToStrAll().c_str());
  key_flowlist_entry_t *ctrlr_flowlist_entry_key =
      reinterpret_cast<key_flowlist_entry_t *> (ctrlr_key->get_key());
  if (NULL == ctrlr_flowlist_entry_key) {
    UPLL_LOG_DEBUG("ctrlr_flowlist_entry_key NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  DbSubOp dbop = { kOpReadSingle, kOpMatchCtrlr, kOpInOutNone };

  val_rename_flowlist_t *rename_flowlist =
  reinterpret_cast<val_rename_flowlist_t *>
  (ConfigKeyVal::Malloc(sizeof(val_rename_flowlist_t)));
  uuu::upll_strncpy(rename_flowlist->flowlist_newname,
                    ctrlr_flowlist_entry_key->flowlist_key.flowlist_name,
           (kMaxLenFlowListName+1));
  rename_flowlist->valid[UPLL_IDX_RENAME_FLOWLIST_RFL] = UNC_VF_VALID;

  FlowListMoMgr *mgr =
    reinterpret_cast<FlowListMoMgr *>(const_cast<MoManager *>(GetMoManager(
            UNC_KT_FLOWLIST)));
  if (!mgr) {
    UPLL_LOG_TRACE("mgr failed");
    free(rename_flowlist);
    return UPLL_RC_ERR_GENERIC;
  }

  result_code = mgr->GetChildConfigKey(unc_key, NULL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetChildConfigKey failed %d", result_code);
    free(rename_flowlist);
    mgr = NULL;
    return result_code;
  }
  if (!unc_key) {
    UPLL_LOG_DEBUG("unc_key NULL");
    free(rename_flowlist);
    mgr = NULL;
    return result_code;
  }
  unc_key->AppendCfgVal(IpctSt::kIpcStValRenameFlowlist,
      rename_flowlist);

  UPLL_LOG_DEBUG("ctrlr_id fle (%s)", ctrlr_id);
  SET_USER_DATA_CTRLR(unc_key, ctrlr_id);

  result_code = mgr->ReadConfigDB(unc_key, dt_type, UNC_OP_READ, dbop, dmi,
      RENAMETBL);
  DELETE_IF_NOT_NULL(unc_key);
  return result_code;
}

upll_rc_t FlowListEntryMoMgr::ReadRecord(IpcReqRespHeader *req,
    ConfigKeyVal *ikey, DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code;

  // 1.Validating the read request
  result_code = ValidateMessage(req, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Validate Message Failed %d", result_code);
    return result_code;
  }

  DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutNone };
  switch (req->datatype) {
  // Retrieving config information
  case UPLL_DT_CANDIDATE:
  case UPLL_DT_RUNNING:
  case UPLL_DT_STARTUP:
  case UPLL_DT_STATE:
    if (req->operation == UNC_OP_READ) {
      result_code = ReadConfigDB(ikey, req->datatype, req->operation,
                                 dbop, dmi, MAINTBL);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG(" ReadConfigDB failed:-%d", result_code);
        return result_code;
      }
    } else {
      if ((req->operation == UNC_OP_READ_SIBLING_BEGIN) ||
          (req->operation == UNC_OP_READ_SIBLING)) {
        dbop.readop = kOpReadMultiple;
      }
      result_code = ReadConfigDB(ikey, req->datatype, req->operation,
                                 dbop, dmi, MAINTBL);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG(" ReadConfigDB failed:-%d", result_code);
        return result_code;
      }
    }
    break;

  case UPLL_DT_IMPORT:
    // Retrieving state information
    if (req->operation != UNC_OP_READ_SIBLING_COUNT) {
      result_code = GetRenamedControllerKey(ikey, req->datatype, dmi, NULL);
      result_code = ReadConfigDB(ikey, req->datatype, req->operation, dbop, dmi,
          RENAMETBL);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG(" ReadConfigDB failed:-%d", result_code);
        return result_code;
      }
    }
    break;
  default:
    return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT;
  }  // end of switch

  UPLL_LOG_DEBUG("Read Record Successfull");
  return UPLL_RC_SUCCESS;
}

upll_rc_t FlowListEntryMoMgr::ReadMo(IpcReqRespHeader *req, ConfigKeyVal *ikey,
    DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  controller_domain ctrlr_dom;

  // validate syntax
  result_code = ValidateMessage(req, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("ValidateMessage failed, Error - %d", result_code);
    return result_code;
  }
  result_code = ReadInfoFromDB(req, ikey, dmi, &ctrlr_dom);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG(" Read request failed result(%d)", result_code);
  }
  return result_code;
}

upll_rc_t FlowListEntryMoMgr:: ReadSiblingMo(IpcReqRespHeader *req,
    ConfigKeyVal *ikey, bool begin, DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  controller_domain ctrlr_dom;

  // validate syntax
  result_code = ValidateMessage(req, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("ValidateMessage failed, Error - %d", result_code);
    return result_code;
  }
  result_code = ReadInfoFromDB(req, ikey, dmi, &ctrlr_dom);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG(" Read request failed result(%d)", result_code);
  }
  return result_code;
}

upll_rc_t FlowListEntryMoMgr::DupConfigKeyVal(ConfigKeyVal *&okey,
    ConfigKeyVal *&req, MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  if (req == NULL) {
    UPLL_LOG_DEBUG("Request is null");
    return UPLL_RC_ERR_GENERIC;
  }

  if (okey != NULL) {
    UPLL_LOG_DEBUG("oKey already Contains Data");
    return UPLL_RC_ERR_GENERIC;
  }

  if (req->get_key_type() != UNC_KT_FLOWLIST_ENTRY) {
    UPLL_LOG_DEBUG(" DupConfigKeyval Failed.");
    return UPLL_RC_ERR_GENERIC;
  }

  ConfigVal *tmp1 = NULL, *tmp = (req)->get_cfg_val();

  if (tmp) {
    if (tbl == MAINTBL) {
      val_flowlist_entry_t *ival = NULL;
      ival = reinterpret_cast<val_flowlist_entry_t *> (GetVal(req));
      if (NULL != ival) {
        val_flowlist_entry_t *flowlist_entry_val =
                      reinterpret_cast<val_flowlist_entry_t*>
                     (ConfigKeyVal::Malloc(sizeof(val_flowlist_entry_t)));
        memcpy(flowlist_entry_val, ival, sizeof(val_flowlist_entry_t));
        tmp1 = new ConfigVal(IpctSt::kIpcStValFlowlistEntry,
            flowlist_entry_val);
      }
    } else if (tbl == CTRLRTBL) {
      val_flowlist_entry_ctrl_t *ival =
          reinterpret_cast<val_flowlist_entry_ctrl_t *>(GetVal(req));
      if (NULL != ival) {
        val_flowlist_entry_ctrl_t *flowlist_ctrlr_val =
            reinterpret_cast<val_flowlist_entry_ctrl_t *>
            (ConfigKeyVal::Malloc(sizeof(val_flowlist_entry_ctrl_t)));
        memcpy(flowlist_ctrlr_val, ival, sizeof(val_flowlist_entry_ctrl_t));
        tmp1 = new ConfigVal(IpctSt::kIpcInvalidStNum,
                             flowlist_ctrlr_val);
      }
    }
    if (tmp1) tmp1->set_user_data(tmp->get_user_data());
    // tmp = tmp->get_next_cfg_val();//COVERITY UN UDSED VALUE
  }
  void *tkey = (req)->get_key();
  key_flowlist_entry_t *ikey = NULL;
  if (tkey != NULL) {
    ikey = reinterpret_cast<key_flowlist_entry_t *> (tkey);
    key_flowlist_entry_t *flowlist_entry =
                        reinterpret_cast<key_flowlist_entry_t*>
                        (ConfigKeyVal::Malloc(sizeof(key_flowlist_entry_t)));
    memcpy(flowlist_entry, ikey, sizeof(key_flowlist_entry_t));
    okey = new ConfigKeyVal(UNC_KT_FLOWLIST_ENTRY,
        IpctSt::kIpcStKeyFlowlistEntry, flowlist_entry, tmp1);
    SET_USER_DATA(okey, req)

    UPLL_LOG_DEBUG("DupConfigkeyVal Succesfull.");
    return UPLL_RC_SUCCESS;
  }
  delete tmp1;
  return UPLL_RC_ERR_GENERIC;
}

bool FlowListEntryMoMgr::CompareKey(ConfigKeyVal *key1,
                                    ConfigKeyVal *key2) {
  UPLL_FUNC_TRACE;
  key_flowlist_entry_t *flowlist_entry_key1,
      *flowlist_entry_key2;
  bool match = false;
  if (key1 == key2) {
    return true;
  }
  if (!key1 || !key2) return false;
  flowlist_entry_key1 =
    reinterpret_cast<key_flowlist_entry_t *>(key1->get_key());
  flowlist_entry_key2 =
      reinterpret_cast<key_flowlist_entry_t *>(key2->get_key());
  if (flowlist_entry_key1 == flowlist_entry_key2) {
    return true;
  }
  if ((!flowlist_entry_key1) || (!flowlist_entry_key2)) {
    UPLL_LOG_DEBUG("FlowListEntryMoMgr::CompareKey failed");
    return false;
  }
  if (strcmp(reinterpret_cast<const char *>
       (flowlist_entry_key1->
            flowlist_key.flowlist_name),
       reinterpret_cast<const char *>
       (flowlist_entry_key2->
         flowlist_key.flowlist_name)) == 0) {
    if (flowlist_entry_key1->sequence_num ==
        flowlist_entry_key2->sequence_num)
    match = true;
    UPLL_LOG_DEBUG(" FlowListEntryMoMgr::CompareKey, Both Keys are same");
  }
  return match;
}

bool FlowListEntryMoMgr::FilterAttributes(void *&val1,
                                          void *val2,
                                          bool copy_to_running,
                                          unc_keytype_operation_t op) {
  UPLL_FUNC_TRACE;
  if (op != UNC_OP_CREATE)
    return CompareValidValue(val1, val2, copy_to_running);
  return false;
}

bool FlowListEntryMoMgr::CompareValidValue(void *&val1, void *val2,
                                           bool copy_to_running) {
  UPLL_FUNC_TRACE;
  bool invalid_attr = true;
  val_flowlist_entry_t *flowlist_entry_val1 =
    reinterpret_cast<val_flowlist_entry_t *>(val1);

  val_flowlist_entry_t *flowlist_entry_val2 =
    reinterpret_cast<val_flowlist_entry_t *>(val2);
  if (!flowlist_entry_val1 || !flowlist_entry_val2) {
    return false;
  }
  for ( unsigned int loop = 0; loop < (sizeof(flowlist_entry_val1->valid)
                         /(sizeof(flowlist_entry_val1->valid[0])));
          ++loop ) {
      if (UNC_VF_INVALID == flowlist_entry_val1->valid[loop] &&
                  UNC_VF_VALID == flowlist_entry_val2->valid[loop])
       flowlist_entry_val1->valid[loop] = UNC_VF_VALID_NO_VALUE;
  }
  if (UNC_VF_VALID == flowlist_entry_val1->valid[UPLL_IDX_MAC_DST_FLE] &&
      UNC_VF_VALID == flowlist_entry_val2->valid[UPLL_IDX_MAC_DST_FLE])
    if (!memcmp(flowlist_entry_val1->mac_dst, flowlist_entry_val2->mac_dst,
        sizeof(flowlist_entry_val2->mac_dst)))
     flowlist_entry_val1->valid[UPLL_IDX_MAC_DST_FLE] = UNC_VF_INVALID;

  if (UNC_VF_VALID == flowlist_entry_val1->valid[UPLL_IDX_MAC_SRC_FLE] &&
      UNC_VF_VALID == flowlist_entry_val2->valid[UPLL_IDX_MAC_SRC_FLE] )
    if (!memcmp(flowlist_entry_val1->mac_src, flowlist_entry_val2->mac_src,
       sizeof(flowlist_entry_val2->mac_src)))
     flowlist_entry_val1->valid[UPLL_IDX_MAC_SRC_FLE] = UNC_VF_INVALID;

  if (UNC_VF_VALID == flowlist_entry_val1->valid[UPLL_IDX_MAC_ETH_TYPE_FLE] &&
      UNC_VF_VALID == flowlist_entry_val2->valid[UPLL_IDX_MAC_ETH_TYPE_FLE] )
    if (flowlist_entry_val1->mac_eth_type == flowlist_entry_val2->mac_eth_type)
     flowlist_entry_val1->valid[UPLL_IDX_MAC_ETH_TYPE_FLE] = UNC_VF_INVALID;

  if (UNC_VF_VALID == flowlist_entry_val1->valid[UPLL_IDX_DST_IP_FLE] &&
      UNC_VF_VALID == flowlist_entry_val2->valid[UPLL_IDX_DST_IP_FLE] )
    if (!memcmp(&flowlist_entry_val1->dst_ip,
               &flowlist_entry_val2->dst_ip,
               sizeof(flowlist_entry_val2->dst_ip)))
    flowlist_entry_val1->valid[UPLL_IDX_DST_IP_FLE] =
        (copy_to_running)?UNC_VF_INVALID:UNC_VF_VALUE_NOT_MODIFIED;

  if (UNC_VF_VALID == flowlist_entry_val1->valid[UPLL_IDX_DST_IP_PREFIX_FLE] &&
      UNC_VF_VALID == flowlist_entry_val2->valid[UPLL_IDX_DST_IP_PREFIX_FLE] )
    if (flowlist_entry_val1->dst_ip_prefixlen  ==
                            flowlist_entry_val2->dst_ip_prefixlen)
     flowlist_entry_val1->valid[UPLL_IDX_DST_IP_PREFIX_FLE] =
         (copy_to_running)?UNC_VF_INVALID:UNC_VF_VALUE_NOT_MODIFIED;


  if (UNC_VF_VALID == flowlist_entry_val1->valid[UPLL_IDX_SRC_IP_FLE] &&
      UNC_VF_VALID == flowlist_entry_val2->valid[UPLL_IDX_SRC_IP_FLE] )
    if (!memcmp(&flowlist_entry_val1->src_ip,
               &flowlist_entry_val2->src_ip,
               sizeof(flowlist_entry_val2->src_ip)))
    flowlist_entry_val1->valid[UPLL_IDX_SRC_IP_FLE] =
        (copy_to_running)?UNC_VF_INVALID:UNC_VF_VALUE_NOT_MODIFIED;

  if (UNC_VF_VALID == flowlist_entry_val1->valid[UPLL_IDX_SRC_IP_PREFIX_FLE] &&
      UNC_VF_VALID == flowlist_entry_val2->valid[UPLL_IDX_SRC_IP_PREFIX_FLE] )
    if (flowlist_entry_val1->src_ip_prefixlen  ==
                            flowlist_entry_val2->src_ip_prefixlen)
    flowlist_entry_val1->valid[UPLL_IDX_SRC_IP_PREFIX_FLE] =
        (copy_to_running)?UNC_VF_INVALID:UNC_VF_VALUE_NOT_MODIFIED;

  if (UNC_VF_VALID == flowlist_entry_val1->valid[UPLL_IDX_VLAN_PRIORITY_FLE] &&
      UNC_VF_VALID == flowlist_entry_val2->valid[UPLL_IDX_VLAN_PRIORITY_FLE] )
    if (flowlist_entry_val1->vlan_priority  ==
                                          flowlist_entry_val2->vlan_priority)
     flowlist_entry_val1->valid[UPLL_IDX_VLAN_PRIORITY_FLE] = UNC_VF_INVALID;

  if (UNC_VF_VALID == flowlist_entry_val1->valid[UPLL_IDX_DST_IP_V6_FLE] &&
      UNC_VF_VALID == flowlist_entry_val2->valid[UPLL_IDX_DST_IP_V6_FLE] )
    if (!memcmp(&flowlist_entry_val1->dst_ipv6,
               &flowlist_entry_val2->dst_ipv6,
               sizeof(flowlist_entry_val2->dst_ipv6)))
     flowlist_entry_val1->valid[UPLL_IDX_DST_IP_V6_FLE] =
         (copy_to_running)?UNC_VF_INVALID:UNC_VF_VALUE_NOT_MODIFIED;

  if (UNC_VF_VALID ==
      flowlist_entry_val1->valid[UPLL_IDX_DST_IP_V6_PREFIX_FLE] &&
      UNC_VF_VALID == flowlist_entry_val2->valid[UPLL_IDX_DST_IP_V6_PREFIX_FLE])
    if (flowlist_entry_val1->dst_ipv6_prefixlen  ==
                            flowlist_entry_val2->dst_ipv6_prefixlen)
     flowlist_entry_val1->valid[UPLL_IDX_DST_IP_V6_PREFIX_FLE] =
         (copy_to_running)?UNC_VF_INVALID:UNC_VF_VALUE_NOT_MODIFIED;

  if (UNC_VF_VALID == flowlist_entry_val1->valid[UPLL_IDX_SRC_IP_V6_FLE] &&
      UNC_VF_VALID == flowlist_entry_val2->valid[UPLL_IDX_SRC_IP_V6_FLE] )
    if (!memcmp(&flowlist_entry_val1->src_ipv6,
               &flowlist_entry_val2->src_ipv6,
               sizeof(flowlist_entry_val2->src_ipv6)))
      flowlist_entry_val1->valid[UPLL_IDX_SRC_IP_V6_FLE] =
          (copy_to_running)?UNC_VF_INVALID:UNC_VF_VALUE_NOT_MODIFIED;

  if (UNC_VF_VALID ==
      flowlist_entry_val1->valid[UPLL_IDX_SRC_IP_V6_PREFIX_FLE] &&
      UNC_VF_VALID == flowlist_entry_val2->valid[UPLL_IDX_SRC_IP_V6_PREFIX_FLE])
    if (flowlist_entry_val1->src_ipv6_prefixlen  ==
                            flowlist_entry_val2->src_ipv6_prefixlen)
     flowlist_entry_val1->valid[UPLL_IDX_SRC_IP_V6_PREFIX_FLE] =
         (copy_to_running)?UNC_VF_INVALID:UNC_VF_VALUE_NOT_MODIFIED;

  if (UNC_VF_VALID == flowlist_entry_val1->valid[UPLL_IDX_IP_PROTOCOL_FLE] &&
      UNC_VF_VALID == flowlist_entry_val2->valid[UPLL_IDX_IP_PROTOCOL_FLE] )
    if (flowlist_entry_val1->ip_proto == flowlist_entry_val2->ip_proto)
     flowlist_entry_val1->valid[UPLL_IDX_IP_PROTOCOL_FLE] = UNC_VF_INVALID;

  if (UNC_VF_VALID == flowlist_entry_val1->valid[UPLL_IDX_IP_DSCP_FLE] &&
      UNC_VF_VALID == flowlist_entry_val2->valid[UPLL_IDX_IP_DSCP_FLE] )
    if (flowlist_entry_val1->ip_dscp == flowlist_entry_val2->ip_dscp)
    flowlist_entry_val1->valid[UPLL_IDX_IP_DSCP_FLE] = UNC_VF_INVALID;


  if (UNC_VF_VALID == flowlist_entry_val1->valid[UPLL_IDX_L4_DST_PORT_FLE] &&
      UNC_VF_VALID == flowlist_entry_val2->valid[UPLL_IDX_L4_DST_PORT_FLE] )
    if (flowlist_entry_val1->l4_dst_port == flowlist_entry_val2->l4_dst_port)
    flowlist_entry_val1->valid[UPLL_IDX_L4_DST_PORT_FLE] =
    (copy_to_running)?UNC_VF_INVALID:UNC_VF_VALUE_NOT_MODIFIED;

  if (UNC_VF_VALID == flowlist_entry_val1->valid[UPLL_IDX_L4_DST_PORT_ENDPT_FLE]
     && UNC_VF_VALID ==
        flowlist_entry_val2->valid[UPLL_IDX_L4_DST_PORT_ENDPT_FLE])
    if (flowlist_entry_val1->l4_dst_port_endpt ==
                           flowlist_entry_val2->l4_dst_port_endpt)
      flowlist_entry_val1->valid[UPLL_IDX_L4_DST_PORT_ENDPT_FLE] =
      (copy_to_running)?UNC_VF_INVALID:UNC_VF_VALUE_NOT_MODIFIED;

  if (UNC_VF_VALID == flowlist_entry_val1->valid[UPLL_IDX_L4_SRC_PORT_FLE] &&
      UNC_VF_VALID == flowlist_entry_val2->valid[UPLL_IDX_L4_SRC_PORT_FLE] )
    if (flowlist_entry_val1->l4_src_port == flowlist_entry_val2->l4_src_port)
      flowlist_entry_val1->valid[UPLL_IDX_L4_SRC_PORT_FLE] =
      (copy_to_running)?UNC_VF_INVALID:UNC_VF_VALUE_NOT_MODIFIED;


  if (UNC_VF_VALID ==
      flowlist_entry_val1->valid[UPLL_IDX_L4_SRC_PORT_ENDPT_FLE] &&
      UNC_VF_VALID ==
      flowlist_entry_val2->valid[UPLL_IDX_L4_SRC_PORT_ENDPT_FLE] )
    if (flowlist_entry_val1->l4_src_port_endpt ==
                           flowlist_entry_val2->l4_src_port_endpt)
      flowlist_entry_val1->valid[UPLL_IDX_L4_SRC_PORT_ENDPT_FLE] =
      (copy_to_running)?UNC_VF_INVALID:UNC_VF_VALUE_NOT_MODIFIED;

  if (UNC_VF_VALID == flowlist_entry_val1->valid[UPLL_IDX_ICMP_TYPE_FLE] &&
      UNC_VF_VALID == flowlist_entry_val2->valid[UPLL_IDX_ICMP_TYPE_FLE])
    if (flowlist_entry_val1->icmp_type == flowlist_entry_val2->icmp_type)
      flowlist_entry_val1->valid[UPLL_IDX_ICMP_TYPE_FLE] = UNC_VF_INVALID;

  if (UNC_VF_VALID == flowlist_entry_val1->valid[UPLL_IDX_ICMP_CODE_FLE] &&
      UNC_VF_VALID == flowlist_entry_val2->valid[UPLL_IDX_ICMP_CODE_FLE])
    if (flowlist_entry_val1->icmp_code == flowlist_entry_val2->icmp_code)
      flowlist_entry_val1->valid[UPLL_IDX_ICMP_CODE_FLE] = UNC_VF_INVALID;

  if (UNC_VF_VALID == flowlist_entry_val1->valid[UPLL_IDX_ICMP_V6_TYPE_FLE] &&
      UNC_VF_VALID == flowlist_entry_val2->valid[UPLL_IDX_ICMP_V6_TYPE_FLE])
    if (flowlist_entry_val1->icmpv6_type == flowlist_entry_val2->icmpv6_type)
      flowlist_entry_val1->valid[UPLL_IDX_ICMP_V6_TYPE_FLE] = UNC_VF_INVALID;

  if (UNC_VF_VALID == flowlist_entry_val1->valid[UPLL_IDX_ICMP_V6_CODE_FLE] &&
      UNC_VF_VALID == flowlist_entry_val2->valid[UPLL_IDX_ICMP_V6_CODE_FLE])
    if (flowlist_entry_val1->icmpv6_code == flowlist_entry_val2->icmpv6_code)
     flowlist_entry_val1->valid[UPLL_IDX_ICMP_V6_CODE_FLE] = UNC_VF_INVALID;

  for (unsigned int loop = 0;
    loop < sizeof(flowlist_entry_val1->valid) / sizeof(uint8_t); ++loop) {
    if ((UNC_VF_VALID == (uint8_t) flowlist_entry_val1->valid[loop]) ||
       (UNC_VF_VALID_NO_VALUE == (uint8_t) flowlist_entry_val1->valid[loop]))
        invalid_attr = false;
  }
  return invalid_attr;
}

bool FlowListEntryMoMgr::CompareValidVal(void *&val1, void *val2, void *val3,
                                         bool copy_to_running) {
  UPLL_FUNC_TRACE;
  bool invalid_attr = true;
  val_flowlist_entry_t *flowlist_entry_val1 =
    reinterpret_cast<val_flowlist_entry_t *>(val1);

  val_flowlist_entry_t *flowlist_entry_val2 =
    reinterpret_cast<val_flowlist_entry_t *>(val2);

  val_flowlist_entry_t *flowlist_entry_val3 =
    reinterpret_cast<val_flowlist_entry_t *>(val2);
  // Addressed Coverity DEADCODE
  if (!flowlist_entry_val1 || !flowlist_entry_val2 || !flowlist_entry_val3) {
    return false;
  }
  for ( unsigned int loop = 0; loop < (sizeof(flowlist_entry_val1->valid)
                         /(sizeof(flowlist_entry_val1->valid[0])));
          ++loop ) {
      if (UNC_VF_INVALID == flowlist_entry_val1->valid[loop] &&
                  UNC_VF_VALID == flowlist_entry_val2->valid[loop])
       flowlist_entry_val1->valid[loop] = UNC_VF_VALID_NO_VALUE;
  }
  if (UNC_VF_VALID == flowlist_entry_val1->valid[UPLL_IDX_MAC_DST_FLE] &&
      UNC_VF_VALID == flowlist_entry_val2->valid[UPLL_IDX_MAC_DST_FLE])
    if (!memcmp(flowlist_entry_val3->mac_dst, flowlist_entry_val2->mac_dst,
        sizeof(flowlist_entry_val2->mac_dst)))
     flowlist_entry_val1->valid[UPLL_IDX_MAC_DST_FLE] = UNC_VF_INVALID;

  if (UNC_VF_VALID == flowlist_entry_val1->valid[UPLL_IDX_MAC_SRC_FLE] &&
      UNC_VF_VALID == flowlist_entry_val2->valid[UPLL_IDX_MAC_SRC_FLE] )
    if (!memcmp(flowlist_entry_val3->mac_src, flowlist_entry_val2->mac_src,
       sizeof(flowlist_entry_val2->mac_src)))
     flowlist_entry_val1->valid[UPLL_IDX_MAC_SRC_FLE] = UNC_VF_INVALID;

  if (UNC_VF_VALID == flowlist_entry_val1->valid[UPLL_IDX_MAC_ETH_TYPE_FLE] &&
      UNC_VF_VALID == flowlist_entry_val2->valid[UPLL_IDX_MAC_ETH_TYPE_FLE] )
    if (flowlist_entry_val3->mac_eth_type == flowlist_entry_val2->mac_eth_type)
     flowlist_entry_val1->valid[UPLL_IDX_MAC_ETH_TYPE_FLE] = UNC_VF_INVALID;

  if (UNC_VF_VALID == flowlist_entry_val1->valid[UPLL_IDX_DST_IP_FLE] &&
      UNC_VF_VALID == flowlist_entry_val2->valid[UPLL_IDX_DST_IP_FLE] )
    if (!memcmp(&flowlist_entry_val3->dst_ip,
               &flowlist_entry_val2->dst_ip,
               sizeof(flowlist_entry_val2->dst_ip)))
    flowlist_entry_val1->valid[UPLL_IDX_DST_IP_FLE] = UNC_VF_INVALID;

  if (UNC_VF_VALID == flowlist_entry_val1->valid[UPLL_IDX_DST_IP_PREFIX_FLE] &&
      UNC_VF_VALID == flowlist_entry_val2->valid[UPLL_IDX_DST_IP_PREFIX_FLE] )
    if (flowlist_entry_val3->dst_ip_prefixlen  ==
                            flowlist_entry_val2->dst_ip_prefixlen)
     flowlist_entry_val1->valid[UPLL_IDX_DST_IP_PREFIX_FLE] = UNC_VF_INVALID;


  if (UNC_VF_VALID == flowlist_entry_val1->valid[UPLL_IDX_SRC_IP_FLE] &&
      UNC_VF_VALID == flowlist_entry_val2->valid[UPLL_IDX_SRC_IP_FLE] )
    if (!memcmp(&flowlist_entry_val3->src_ip,
               &flowlist_entry_val2->src_ip,
               sizeof(flowlist_entry_val2->src_ip)))
    flowlist_entry_val1->valid[UPLL_IDX_SRC_IP_FLE] = UNC_VF_INVALID;

  if (UNC_VF_VALID == flowlist_entry_val1->valid[UPLL_IDX_SRC_IP_PREFIX_FLE] &&
      UNC_VF_VALID == flowlist_entry_val2->valid[UPLL_IDX_SRC_IP_PREFIX_FLE] )
    if (flowlist_entry_val3->src_ip_prefixlen  ==
                            flowlist_entry_val2->src_ip_prefixlen)
    flowlist_entry_val1->valid[UPLL_IDX_SRC_IP_PREFIX_FLE] = UNC_VF_INVALID;

  if (UNC_VF_VALID == flowlist_entry_val1->valid[UPLL_IDX_VLAN_PRIORITY_FLE] &&
      UNC_VF_VALID == flowlist_entry_val2->valid[UPLL_IDX_VLAN_PRIORITY_FLE] )
    if (flowlist_entry_val3->vlan_priority  ==
                                          flowlist_entry_val2->vlan_priority)
     flowlist_entry_val1->valid[UPLL_IDX_VLAN_PRIORITY_FLE] = UNC_VF_INVALID;

  if (UNC_VF_VALID == flowlist_entry_val1->valid[UPLL_IDX_DST_IP_V6_FLE] &&
      UNC_VF_VALID == flowlist_entry_val2->valid[UPLL_IDX_DST_IP_V6_FLE] )
    if (!memcmp(&flowlist_entry_val3->dst_ipv6,
               &flowlist_entry_val2->dst_ipv6,
               sizeof(flowlist_entry_val2->dst_ipv6)))
     flowlist_entry_val1->valid[UPLL_IDX_DST_IP_V6_FLE] = UNC_VF_INVALID;

  if (UNC_VF_VALID ==
      flowlist_entry_val1->valid[UPLL_IDX_DST_IP_V6_PREFIX_FLE] &&
      UNC_VF_VALID == flowlist_entry_val2->valid[UPLL_IDX_DST_IP_V6_PREFIX_FLE])
    if (flowlist_entry_val3->dst_ipv6_prefixlen  ==
                            flowlist_entry_val2->dst_ipv6_prefixlen)
     flowlist_entry_val1->valid[UPLL_IDX_DST_IP_V6_PREFIX_FLE] = UNC_VF_INVALID;

  if (UNC_VF_VALID == flowlist_entry_val1->valid[UPLL_IDX_SRC_IP_V6_FLE] &&
      UNC_VF_VALID == flowlist_entry_val2->valid[UPLL_IDX_SRC_IP_V6_FLE] )
    if (!memcmp(&flowlist_entry_val3->src_ipv6,
               &flowlist_entry_val2->src_ipv6,
               sizeof(flowlist_entry_val2->src_ipv6)))
      flowlist_entry_val1->valid[UPLL_IDX_SRC_IP_V6_FLE] = UNC_VF_INVALID;

  if (UNC_VF_VALID ==
      flowlist_entry_val1->valid[UPLL_IDX_SRC_IP_V6_PREFIX_FLE] &&
      UNC_VF_VALID == flowlist_entry_val2->valid[UPLL_IDX_SRC_IP_V6_PREFIX_FLE])
    if (flowlist_entry_val3->src_ipv6_prefixlen  ==
                            flowlist_entry_val2->src_ipv6_prefixlen)
     flowlist_entry_val1->valid[UPLL_IDX_SRC_IP_V6_PREFIX_FLE] = UNC_VF_INVALID;

  if (UNC_VF_VALID == flowlist_entry_val1->valid[UPLL_IDX_IP_PROTOCOL_FLE] &&
      UNC_VF_VALID == flowlist_entry_val2->valid[UPLL_IDX_IP_PROTOCOL_FLE] )
    if (flowlist_entry_val3->ip_proto == flowlist_entry_val2->ip_proto)
     flowlist_entry_val1->valid[UPLL_IDX_IP_PROTOCOL_FLE] = UNC_VF_INVALID;

  if (UNC_VF_VALID == flowlist_entry_val1->valid[UPLL_IDX_IP_DSCP_FLE] &&
      UNC_VF_VALID == flowlist_entry_val2->valid[UPLL_IDX_IP_DSCP_FLE] )
    if (flowlist_entry_val3->ip_dscp == flowlist_entry_val2->ip_dscp)
    flowlist_entry_val1->valid[UPLL_IDX_IP_DSCP_FLE] = UNC_VF_INVALID;


  if (UNC_VF_VALID == flowlist_entry_val1->valid[UPLL_IDX_L4_DST_PORT_FLE] &&
      UNC_VF_VALID == flowlist_entry_val2->valid[UPLL_IDX_L4_DST_PORT_FLE] )
    if (flowlist_entry_val3->l4_dst_port == flowlist_entry_val2->l4_dst_port)
    flowlist_entry_val1->valid[UPLL_IDX_L4_DST_PORT_FLE] =
    (copy_to_running)?UNC_VF_INVALID:UNC_VF_VALUE_NOT_MODIFIED;

  if (UNC_VF_VALID == flowlist_entry_val1->valid[UPLL_IDX_L4_DST_PORT_ENDPT_FLE]
     && UNC_VF_VALID ==
        flowlist_entry_val2->valid[UPLL_IDX_L4_DST_PORT_ENDPT_FLE])
    if (flowlist_entry_val3->l4_dst_port_endpt ==
                           flowlist_entry_val2->l4_dst_port_endpt)
      flowlist_entry_val1->valid[UPLL_IDX_L4_DST_PORT_ENDPT_FLE] =
      (copy_to_running)?UNC_VF_INVALID:UNC_VF_VALUE_NOT_MODIFIED;

  if (UNC_VF_VALID == flowlist_entry_val1->valid[UPLL_IDX_L4_SRC_PORT_FLE] &&
      UNC_VF_VALID == flowlist_entry_val2->valid[UPLL_IDX_L4_SRC_PORT_FLE] )
    if (flowlist_entry_val3->l4_src_port == flowlist_entry_val2->l4_src_port)
      flowlist_entry_val1->valid[UPLL_IDX_L4_SRC_PORT_FLE] =
      (copy_to_running)?UNC_VF_INVALID:UNC_VF_VALUE_NOT_MODIFIED;


  if (UNC_VF_VALID ==
      flowlist_entry_val1->valid[UPLL_IDX_L4_SRC_PORT_ENDPT_FLE] &&
      UNC_VF_VALID ==
      flowlist_entry_val2->valid[UPLL_IDX_L4_SRC_PORT_ENDPT_FLE] )
    if (flowlist_entry_val3->l4_src_port_endpt ==
                           flowlist_entry_val2->l4_src_port_endpt)
      flowlist_entry_val1->valid[UPLL_IDX_L4_SRC_PORT_ENDPT_FLE] =
      (copy_to_running)?UNC_VF_INVALID:UNC_VF_VALUE_NOT_MODIFIED;

  if (UNC_VF_VALID == flowlist_entry_val1->valid[UPLL_IDX_ICMP_TYPE_FLE] &&
      UNC_VF_VALID == flowlist_entry_val2->valid[UPLL_IDX_ICMP_TYPE_FLE])
    if (flowlist_entry_val3->icmp_type == flowlist_entry_val2->icmp_type)
      flowlist_entry_val1->valid[UPLL_IDX_ICMP_TYPE_FLE] = UNC_VF_INVALID;

  if (UNC_VF_VALID == flowlist_entry_val1->valid[UPLL_IDX_ICMP_CODE_FLE] &&
      UNC_VF_VALID == flowlist_entry_val2->valid[UPLL_IDX_ICMP_CODE_FLE])
    if (flowlist_entry_val3->icmp_code == flowlist_entry_val2->icmp_code)
      flowlist_entry_val1->valid[UPLL_IDX_ICMP_CODE_FLE] = UNC_VF_INVALID;

  if (UNC_VF_VALID == flowlist_entry_val1->valid[UPLL_IDX_ICMP_V6_TYPE_FLE] &&
      UNC_VF_VALID == flowlist_entry_val2->valid[UPLL_IDX_ICMP_V6_TYPE_FLE])
    if (flowlist_entry_val3->icmpv6_type == flowlist_entry_val2->icmpv6_type)
      flowlist_entry_val1->valid[UPLL_IDX_ICMP_V6_TYPE_FLE] = UNC_VF_INVALID;

  if (UNC_VF_VALID == flowlist_entry_val1->valid[UPLL_IDX_ICMP_V6_CODE_FLE] &&
      UNC_VF_VALID == flowlist_entry_val2->valid[UPLL_IDX_ICMP_V6_CODE_FLE])
    if (flowlist_entry_val3->icmpv6_code == flowlist_entry_val2->icmpv6_code)
     flowlist_entry_val1->valid[UPLL_IDX_ICMP_V6_CODE_FLE] = UNC_VF_INVALID;

  for (unsigned int loop = 0;
    loop < sizeof(flowlist_entry_val1->valid) / sizeof(uint8_t); ++loop) {
    if ((UNC_VF_VALID == (uint8_t) flowlist_entry_val1->valid[loop]) ||
       (UNC_VF_VALID_NO_VALUE == (uint8_t) flowlist_entry_val1->valid[loop]))
        invalid_attr = false;
  }
  return invalid_attr;
}

upll_rc_t FlowListEntryMoMgr::MergeValidate(unc_key_type_t keytype,
    const char *ctrlr_id, ConfigKeyVal *ikey,
    DalDmlIntf *dmi, upll_import_type import_type) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_ERR_GENERIC;
  unc_keytype_operation_t op[] = {UNC_OP_DELETE, UNC_OP_CREATE, UNC_OP_UPDATE};
  int nop = sizeof(op) / sizeof(op[0]);

  result_code = ValidateImportWithRunning(keytype, ctrlr_id,
                                            ikey, op, nop, dmi);
  if ((result_code != UPLL_RC_SUCCESS) &&
      (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE)) {
     UPLL_LOG_DEBUG("ValidateImportWithRunning DB err (%d)", result_code);
     return result_code;
  }

  UPLL_LOG_DEBUG("MergeValidate result code (%d)", result_code);
  return UPLL_RC_SUCCESS;
}

upll_rc_t FlowListEntryMoMgr::RenameMo(IpcReqRespHeader *req,
    ConfigKeyVal *ikey, DalDmlIntf *dmi, const char *ctrlr_id) {
  UPLL_FUNC_TRACE;
  UPLL_LOG_DEBUG(" FlowListEntryMoMgr::Rename Not required:: successful ");
  return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_KT;
}

/**
 * @brief     Method used for IsReferenced.
 * @param[in] req        Pointer to IpcResResHeader
 * @param[in] ikey       Contains the Pointer to ConfigkeyVal Class.
 * @param[in] dmi        Pointer to DalDmlIntf Class.
 * @retval    RT_SUCCESS Successfull completion.
 * upll_rc_t IsReferenced(ConfigKeyVal *ikey, upll_keytype_datatype_t dt_type,
 *                            DalDmlIntf *dmi);
 */
upll_rc_t FlowListEntryMoMgr::IsReferenced(IpcReqRespHeader *req,
    ConfigKeyVal *ikey, DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  // int refcount = 0;
  // uint8_t *ctrlr_id = { 0 };
  // ConfigKeyVal *okey = NULL;
  if (NULL == ikey) {
    UPLL_LOG_DEBUG("ikey is NULL");
    return UPLL_RC_ERR_CFG_SEMANTIC;
  }
  key_flowlist_entry_t *key_fle = reinterpret_cast
    <key_flowlist_entry_t *>(ikey->get_key());
  PolicingProfileEntryMoMgr *mgr =
    reinterpret_cast<PolicingProfileEntryMoMgr *>(const_cast<MoManager *>
    (GetMoManager(UNC_KT_POLICING_PROFILE_ENTRY)));
  if (NULL == mgr) {
    return UPLL_RC_ERR_GENERIC;
  }

  TcConfigMode config_mode = TC_CONFIG_INVALID;
  std::string vtn_name = "";
  result_code = GetConfigModeInfo(req, config_mode, vtn_name);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetConfigMode failed");
    return result_code;
  }

  if (UPLL_DT_CANDIDATE == req->datatype) {
    result_code = mgr->IsFlowListMatched(reinterpret_cast<const char *>
                                         (key_fle->flowlist_key.flowlist_name),
                                         dmi, config_mode, vtn_name);
    if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
      UPLL_LOG_DEBUG("IsFlowListMatched failed %d", result_code);
      return result_code;
    }
  } else {
    result_code = mgr->IsFlowListMatched(reinterpret_cast<const char *>
                                        (key_fle->flowlist_key.flowlist_name),
                                         req->datatype, dmi);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("IsFlowListMatched failed from ppe %d", result_code);
    }
  }
  return result_code;
}

upll_rc_t FlowListEntryMoMgr::GetFlowListKeyVal(
    ConfigKeyVal *&okey, ConfigKeyVal *&ikey) {
  UPLL_FUNC_TRACE;

  key_flowlist_entry_t *key_flowlist_entry =
    reinterpret_cast<key_flowlist_entry_t *>(ikey->get_key());

  if (NULL == key_flowlist_entry) {
    UPLL_LOG_DEBUG("Invalid key");
    return UPLL_RC_ERR_CFG_SEMANTIC;
  }

  key_flowlist_t *key_flowlist =
    reinterpret_cast<key_flowlist_t *>(ConfigKeyVal::Malloc(
          sizeof(key_flowlist_t)));
  uuu::upll_strncpy(
      key_flowlist->flowlist_name, key_flowlist_entry->
      flowlist_key.flowlist_name, kMaxLenFlowListName + 1);

  okey = new ConfigKeyVal(UNC_KT_FLOWLIST,
      IpctSt::kIpcStKeyFlowlist, key_flowlist,
      NULL);
  UPLL_LOG_DEBUG("GetObjectConfigVal Successfull");
  return UPLL_RC_SUCCESS;
}


upll_rc_t FlowListEntryMoMgr::GetRenamedControllerKey(ConfigKeyVal *ikey,
    upll_keytype_datatype_t dt_type, DalDmlIntf *dmi,
    controller_domain *ctrlr_dom) {
  UPLL_FUNC_TRACE;
  ConfigKeyVal *okey = NULL;
  upll_rc_t result_code = UPLL_RC_ERR_GENERIC;

  /* Flowlist is  renamed */
  UPLL_LOG_DEBUG("flow list renamed");
  MoMgrImpl *mgr =
    reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>(GetMoManager(
            UNC_KT_FLOWLIST)));

  if (NULL == mgr) {
    UPLL_LOG_DEBUG("Unable to get kt_flowlist instance");
    return UPLL_RC_ERR_GENERIC;
  }
  result_code = mgr->GetChildConfigKey(okey, NULL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetChildConfigKey fail (%d)", result_code);
    return UPLL_RC_ERR_GENERIC;
  }

  if (ctrlr_dom != NULL) {
    SET_USER_DATA_CTRLR_DOMAIN(okey, *ctrlr_dom);
  } else {
    UPLL_LOG_DEBUG("Controller id is null");
    DELETE_IF_NOT_NULL(okey);
    return UPLL_RC_ERR_GENERIC;
  }

  UPLL_LOG_DEBUG("ctrlr : %s; domain : %s", ctrlr_dom->ctrlr,
      ctrlr_dom->domain);

  uuu::upll_strncpy(
      reinterpret_cast<key_flowlist_t *>(okey->get_key())->flowlist_name,
      reinterpret_cast <key_flowlist_entry *>
      (ikey->get_key())->flowlist_key.flowlist_name,
      (kMaxLenFlowListName + 1));

  UPLL_LOG_DEBUG("flowlist name (%s) (%s)",
      reinterpret_cast<key_flowlist_t *>(okey->get_key())->flowlist_name,
      reinterpret_cast <key_flowlist_entry *>
      (ikey->get_key())->flowlist_key.flowlist_name);

  DbSubOp dbop = { kOpReadSingle, kOpMatchCtrlr, kOpInOutFlag };

  result_code = mgr->ReadConfigDB(okey, dt_type,
                                  UNC_OP_READ, dbop, dmi, RENAMETBL);
  if (result_code != UPLL_RC_SUCCESS) {
    if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
      UPLL_LOG_DEBUG("ReadConfigDB no instance");
      DELETE_IF_NOT_NULL(okey);
      return UPLL_RC_SUCCESS;
    }
    UPLL_LOG_DEBUG("ReadConfigDB failed (%d)", result_code);
    DELETE_IF_NOT_NULL(okey);
    return result_code;
  }

  val_rename_flowlist_t *rename_val = NULL;
  rename_val = reinterpret_cast<val_rename_flowlist_t *> (GetVal(okey));
  if (!rename_val) {
    UPLL_LOG_DEBUG("FlowList Name is not valid");
    DELETE_IF_NOT_NULL(okey);
    return UPLL_RC_ERR_GENERIC;
  }

  key_flowlist_entry *key = reinterpret_cast<key_flowlist_entry *>(
      ikey->get_key());
  uuu::upll_strncpy(key->flowlist_key.flowlist_name,
      rename_val->flowlist_newname,
      (kMaxLenFlowListName+1));
  UPLL_LOG_DEBUG("flowlist re name (%s) (%s)",
      key->flowlist_key.flowlist_name,
      rename_val->flowlist_newname);

  DELETE_IF_NOT_NULL(okey);

  UPLL_LOG_DEBUG("GetRenamedControllerKey is Successfull");
  return UPLL_RC_SUCCESS;
}

upll_rc_t FlowListEntryMoMgr::GetControllerDomainSpan(ConfigKeyVal *ikey,
                                        upll_keytype_datatype_t dt_type,
                                        DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code;
  DbSubOp dbop = {kOpReadExist|kOpReadMultiple, kOpMatchNone,
                  kOpInOutCtrlr};

  result_code = ReadConfigDB(ikey, dt_type, UNC_OP_READ, dbop, dmi, CTRLRTBL);
  return result_code;
}

upll_rc_t FlowListEntryMoMgr::AddFlowListToController(char *flowlist_name,
                                                DalDmlIntf *dmi,
                                                char* ctrl_id,
                                                upll_keytype_datatype_t dt_type,
                                                unc_keytype_operation_t op,
                                                TcConfigMode config_mode,
                                                string vtn_name) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *okey = NULL;
  result_code = GetChildConfigKey(okey, NULL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetChildConfigKey failed %d", result_code);
    return result_code;
  }
  key_flowlist_entry_t *flowlist_key = reinterpret_cast<key_flowlist_entry_t*>
                                 (okey->get_key());
  uuu::upll_strncpy(flowlist_key->flowlist_key.flowlist_name,
                    flowlist_name, (kMaxLenFlowListName+1));
  DbSubOp dbop = {kOpReadMultiple, kOpMatchNone, kOpInOutFlag | kOpInOutCs};

  result_code = ReadConfigDB(okey,
                            dt_type,
                            UNC_OP_READ,
                            dbop, dmi, MAINTBL);
  if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
    UPLL_LOG_DEBUG("No such instance in entry table");
    DELETE_IF_NOT_NULL(okey);
    return UPLL_RC_SUCCESS;
  }
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Unable to read flowlist configuration from DB %d",
                    result_code);
    DELETE_IF_NOT_NULL(okey);
    return result_code;
  }
  // Update the flowlist entry controller table based on the operation
  result_code = UpdateControllerTable(okey, op, dt_type, dmi, ctrl_id,
                                      config_mode, vtn_name);
  DELETE_IF_NOT_NULL(okey);
  return result_code;
}

void FlowListEntryMoMgr::SetValidAttributesForController(
    val_flowlist_entry_t *val) {
  UPLL_FUNC_TRACE;
  for ( unsigned int loop = 0;
      loop < sizeof(val->valid)/sizeof(val->valid[0]); ++loop ) {
    if ((val->valid[loop] == UNC_VF_NOT_SUPPORTED)
        || (val->valid[loop] ==
           UNC_VF_VALID_NO_VALUE)) {
        val->valid[loop] = UNC_VF_INVALID;
    }
  }
}

string FlowListEntryMoMgr::GetQueryStringForCtrlrTable(
                           upll_keytype_datatype_t dt_type) {
  UPLL_FUNC_TRACE;
  string sql_query = "INSERT INTO ";
  if (dt_type == UPLL_DT_CANDIDATE) {
    sql_query += "ca_flowlist_entry_ctrlr_tbl";
  } else if (dt_type == UPLL_DT_AUDIT) {
    sql_query += "au_flowlist_entry_ctrlr_tbl";
  } else if (dt_type == UPLL_DT_IMPORT) {
    sql_query += "im_flowlist_entry_ctrlr_tbl";
  } else {
    return "";
  }
  sql_query +=
      " (ctrlr_name, flowlist_name, sequence_num, flags, valid_mac_dst, "\
      "valid_mac_src, valid_mac_eth_type, valid_dst_ip,"\
      "valid_dst_ip_prefix, valid_src_ip, valid_src_ip_prefix,"\
      "valid_vlan_priority, valid_dst_ipv6, valid_dst_ipv6_prefix,"\
      "valid_src_ipv6, valid_src_ipv6_prefix, valid_ip_protocol,"\
      "valid_ip_dscp, valid_l4_dst_port, valid_l4_dst_port_endpt,"\
      "valid_l4_src_port, valid_l4_src_port_endpt, valid_icmp_type,"\
      "valid_icmp_code, valid_icmpv6_type, valid_icmpv6_code";
  // For AuditDB Copy the cs attributes from the main table
  if (dt_type == UPLL_DT_AUDIT) {
     sql_query +=
                ", cs_rowstatus, cs_mac_dst, cs_mac_src, cs_mac_eth_type,"\
                "cs_dst_ip, cs_dst_ip_prefix, cs_src_ip, cs_src_ip_prefix,"\
                "cs_vlan_priority, cs_dst_ipv6, cs_dst_ipv6_prefix,"\
                "cs_src_ipv6, cs_src_ipv6_prefix, cs_ip_protocol,"\
                "cs_ip_dscp, cs_l4_dst_port, cs_l4_dst_port_endpt,"\
                "cs_l4_src_port, cs_l4_src_port_endpt,"\
                "cs_icmp_type, cs_icmp_code, cs_icmpv6_type, cs_icmpv6_code";
  }
  // c_flag, u_flag available only for candidate DB
  if (dt_type == UPLL_DT_CANDIDATE)
    sql_query += ", c_flag, u_flag";

  sql_query+=") ";
  return sql_query;
}

// This Function should be called only if any of the  attribute is not
// supported by the controller
void  FlowListEntryMoMgr::GetControllerNotSupportedAttrCol(
                                   set<string> *ctrlr_notsupported_attr_set,
                                   uint8_t valid_index) {
  UPLL_FUNC_TRACE;
    switch (valid_index) {
      case UPLL_IDX_MAC_DST_FLE:
         ctrlr_notsupported_attr_set->insert("valid_mac_dst");
      break;
      case UPLL_IDX_MAC_SRC_FLE:
         ctrlr_notsupported_attr_set->insert("valid_mac_src");
      break;
      case UPLL_IDX_MAC_ETH_TYPE_FLE:
         ctrlr_notsupported_attr_set->insert("valid_mac_eth_type");
      break;
      case UPLL_IDX_DST_IP_FLE:
         ctrlr_notsupported_attr_set->insert("valid_dst_ip");
      break;
      case UPLL_IDX_DST_IP_PREFIX_FLE:
         ctrlr_notsupported_attr_set->insert("valid_dst_ip_prefix");
      break;
      case UPLL_IDX_SRC_IP_FLE:
         ctrlr_notsupported_attr_set->insert("valid_src_ip");
      break;
      case UPLL_IDX_SRC_IP_PREFIX_FLE:
         ctrlr_notsupported_attr_set->insert("valid_src_ip_prefix");
      break;
      case UPLL_IDX_VLAN_PRIORITY_FLE:
         ctrlr_notsupported_attr_set->insert("valid_vlan_priority");
      break;
      case UPLL_IDX_DST_IP_V6_FLE:
         ctrlr_notsupported_attr_set->insert("valid_dst_ipv6");
      break;
      case UPLL_IDX_DST_IP_V6_PREFIX_FLE:
         ctrlr_notsupported_attr_set->insert("valid_dst_ipv6_prefix");
      break;
      case UPLL_IDX_SRC_IP_V6_FLE:
         ctrlr_notsupported_attr_set->insert("valid_src_ipv6");
      break;
      case UPLL_IDX_SRC_IP_V6_PREFIX_FLE:
         ctrlr_notsupported_attr_set->insert("valid_src_ipv6_prefix");
      break;
      case UPLL_IDX_IP_PROTOCOL_FLE:
         ctrlr_notsupported_attr_set->insert("valid_ip_protocol");
      break;
      case UPLL_IDX_IP_DSCP_FLE:
         ctrlr_notsupported_attr_set->insert("valid_ip_dscp");
      break;
      case UPLL_IDX_L4_DST_PORT_FLE:
         ctrlr_notsupported_attr_set->insert("valid_l4_dst_port");
      break;
      case UPLL_IDX_L4_DST_PORT_ENDPT_FLE:
         ctrlr_notsupported_attr_set->insert("valid_l4_dst_port_endpt");
      break;
      case UPLL_IDX_L4_SRC_PORT_FLE:
         ctrlr_notsupported_attr_set->insert("valid_l4_src_port");
      break;
      case  UPLL_IDX_L4_SRC_PORT_ENDPT_FLE:
         ctrlr_notsupported_attr_set->insert("valid_l4_src_port_endpt");
      break;
      case UPLL_IDX_ICMP_TYPE_FLE:
         ctrlr_notsupported_attr_set->insert("valid_icmp_type");
      break;
      case UPLL_IDX_ICMP_CODE_FLE:
         ctrlr_notsupported_attr_set->insert("valid_icmp_code");
      break;
      case UPLL_IDX_ICMP_V6_TYPE_FLE:
         ctrlr_notsupported_attr_set->insert("valid_icmpv6_type");
      break;
      case UPLL_IDX_ICMP_V6_CODE_FLE:
         ctrlr_notsupported_attr_set->insert("valid_icmpv6_code");
      break;
    }
}
string FlowListEntryMoMgr::SelectColumnsDynamically(char * ctrl_id,
                                upll_keytype_datatype_t dt_type,
                                set<string> *ctrlr_notsupported_attr_set
                                ) {
  UPLL_FUNC_TRACE;
  string  append_col_query = " SELECT ";
  append_col_query += "'";
  append_col_query += ctrl_id;
  append_col_query += "'";
  append_col_query += ",";
  append_col_query += "flowlist_name, sequence_num, flags, valid_mac_dst,"\
                "valid_mac_src, valid_mac_eth_type, valid_dst_ip,"\
                "valid_dst_ip_prefix, valid_src_ip, valid_src_ip_prefix,"\
                "valid_vlan_priority, valid_dst_ipv6, valid_dst_ipv6_prefix,"\
                "valid_src_ipv6, valid_src_ipv6_prefix, valid_ip_protocol,"\
                "valid_ip_dscp, valid_l4_dst_port, valid_l4_dst_port_endpt,"\
                "valid_l4_src_port, valid_l4_src_port_endpt, valid_icmp_type,"\
                "valid_icmp_code, valid_icmpv6_type, valid_icmpv6_code";
  // For Audit DB , Copy the cs attr from the main table
  if (dt_type == UPLL_DT_AUDIT) {
    append_col_query +=
                 ", cs_rowstatus, cs_mac_dst, cs_mac_src, cs_mac_eth_type,"\
                "cs_dst_ip, cs_dst_ip_prefix, cs_src_ip, cs_src_ip_prefix,"\
                "cs_vlan_priority, cs_dst_ipv6, cs_dst_ipv6_prefix,"\
                "cs_src_ipv6, cs_src_ipv6_prefix, cs_ip_protocol,"\
                "cs_ip_dscp, cs_l4_dst_port, cs_l4_dst_port_endpt,"\
                "cs_l4_src_port, cs_l4_src_port_endpt,"\
                "cs_icmp_type, cs_icmp_code, cs_icmpv6_type, cs_icmpv6_code";
  }
  if (dt_type == UPLL_DT_CANDIDATE) {
    append_col_query += ",";
    append_col_query += create_time_c_flag;
    append_col_query += ",";
    append_col_query += create_time_u_flag;
  }
  append_col_query+= " FROM ";
  if (dt_type == UPLL_DT_CANDIDATE) {
    append_col_query += "ca_flowlist_entry_tbl";
  } else if (dt_type == UPLL_DT_AUDIT) {
    append_col_query += "au_flowlist_entry_tbl";
  } else if (dt_type == UPLL_DT_IMPORT) {
    append_col_query += "im_flowlist_entry_tbl";
  }
  if (!ctrlr_notsupported_attr_set->empty()) {
     for (std::set<string>::const_iterator it =
          ctrlr_notsupported_attr_set->begin();
          it != ctrlr_notsupported_attr_set->end(); it++) {
        size_t col_position =append_col_query.find(*it);
        // replace the columns which are not supported as UNC_VF_INVALID
        //  the value UNC_VF_INVALID should be stored as the attribute which
        //  is not supported by the controller

        if (col_position != std::string::npos)
          append_col_query.replace(col_position, (*it).length(),
             static_cast<ostringstream*>(
               &(ostringstream() << UNC_VF_INVALID))->str());
       }
  }
  append_col_query += " WHERE flowlist_name = ?";
  return append_col_query;
}
upll_rc_t FlowListEntryMoMgr::UpdateControllerTable(ConfigKeyVal *ikey,
                                               unc_keytype_operation_t op,
                                               upll_keytype_datatype_t dt_type,
                                               DalDmlIntf *dmi,
                                               char* ctrl_id,
                                               TcConfigMode config_mode,
                                               string vtn_name) {
  UPLL_FUNC_TRACE;
  // If controller type is other than PFC/ODC, return SUCCESS
  unc_keytype_ctrtype_t ctrlrtype = UNC_CT_UNKNOWN;
  uuc::CtrlrMgr *ctrlr_mgr = uuc::CtrlrMgr::GetInstance();
  if ((!ctrlr_mgr->GetCtrlrType(reinterpret_cast<char *>(ctrl_id),
     dt_type, &ctrlrtype)) || ((ctrlrtype != UNC_CT_PFC) &&
                               (ctrlrtype != UNC_CT_ODC))) {
    UPLL_LOG_DEBUG("Controller type is  %d", ctrlrtype);
    return UPLL_RC_SUCCESS;
  }
  uint8_t *vtnname = NULL;
  if (!vtn_name.empty()) {
    vtnname = reinterpret_cast<uint8_t *>(const_cast<char *>
                  (vtn_name.c_str()));
  }
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  if (op == UNC_OP_DELETE) {
    // Delete the entry from CTRLTBL based on flowlist name and ctrlr name
    SET_USER_DATA_CTRLR(ikey, ctrl_id);
    key_user_data *tuser_data  = reinterpret_cast<key_user_data_t *>
                               (ikey->get_user_data());

    void *tkey = ikey->get_key();
    void *p = NULL;
    const uudst::kDalTableIndex tbl_index = GetTable(CTRLRTBL, dt_type);
    DalBindInfo *db_info = new DalBindInfo(tbl_index);
    BindInfo *binfo = flowlistentry_controller_bind_info;

    //  Bind match flowlist_name
    uint8_t array_index = FLOWLIST_COL_INDEX_MAINTBL;
    uint64_t indx = binfo[array_index].index;
    p = reinterpret_cast<void *>(reinterpret_cast<char *>(tkey)
        + binfo[array_index].offset);
    db_info->BindMatch(indx, binfo[array_index].app_data_type,
                       binfo[array_index].array_size, p);

    //  Bind match ctrlr_name
    array_index += FLE_CTRL_COL_INDEX_CTRLTBL;
    indx = binfo[array_index].index;
    p = reinterpret_cast<void *>(reinterpret_cast<char *>(tuser_data)
        + binfo[array_index].offset);
    db_info->BindMatch(indx, binfo[array_index].app_data_type,
                       binfo[array_index].array_size, p);
    // Delete the entries from CTRLTBL for the matched
    // flowlist_name and ctrlr_name
    result_code = DalToUpllResCode(
         dmi->ExecuteAppQuery(QUERY_CA_DELETE_FL_IN_FLECTRLTBL, dt_type,
                              tbl_index, db_info, UNC_OP_DELETE, config_mode,
                              vtnname));
    if (result_code != UPLL_RC_SUCCESS) {
      if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
        UPLL_LOG_DEBUG("No such instance in ctrlr table");
        result_code = UPLL_RC_SUCCESS;
      } else {
        UPLL_LOG_INFO("Unable to delete flowlist entry from CTRLRTBL %d",
                        result_code);
      }
    } else {
      // Copy the deleted entry from ru_fe_ctr_tbl to ca_del_fle_ctr_tbl
      result_code = DalToUpllResCode(dmi->CopyMatchingRecords(
          UPLL_DT_CANDIDATE_DEL, UPLL_DT_RUNNING, tbl_index, db_info,
          config_mode, vtnname));
      if (result_code != UPLL_RC_SUCCESS) {
        if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
          UPLL_LOG_DEBUG("No such instance in running table");
          result_code = UPLL_RC_SUCCESS;
        } else {
          UPLL_LOG_INFO("Unable to CopyMatchingRecords from RUNNING DB %d",
                          result_code);
        }
      }
    }
    DELETE_IF_NOT_NULL(db_info);
    return result_code;
  }

  ConfigKeyVal *temp_ikey = ikey;
  val_flowlist_entry_ctrl_t *val_flowlist_entry_ctrl = NULL;
  std::set<std::string> ctrlr_not_supported_attr;
  upll_rc_t result_code_ctrlrtbl =  UPLL_RC_SUCCESS;
  string query_string;
  // check for UPDATE/CREATE operation

  // Create dummy configval and perform the capability check
  ConfigKeyVal *capa_ckv = NULL;
  result_code = GetChildConfigKey(capa_ckv, NULL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetChildConfigKectrlr_ckvy is failed resultcode=%d",
                   result_code);
    return result_code;
  }

  val_flowlist_entry_t *dummy_val =
    reinterpret_cast<val_flowlist_entry_t*>
     (ConfigKeyVal::Malloc(sizeof(val_flowlist_entry_t)));
  // set all valid flags as VALID to identify the NOT_SUPPORTED fields
  for ( unsigned int loop = 0;
      loop < sizeof(dummy_val->valid)/sizeof(dummy_val->valid[0]);
      ++loop ) {
    dummy_val->valid[loop] = UNC_VF_VALID;
  }

  capa_ckv->AppendCfgVal(IpctSt::kIpcStValFlowlistEntry, dummy_val);

  IpcReqRespHeader *req_header = reinterpret_cast<IpcReqRespHeader*>
    (ConfigKeyVal::Malloc(sizeof(IpcReqRespHeader)));

  req_header->operation = op;
  req_header->datatype = UPLL_DT_CANDIDATE;

  // Validate whether the attributes supported by controller or not
  result_code = ValidateCapability(req_header, capa_ckv, ctrl_id);
  free(req_header);

  if (result_code != UPLL_RC_SUCCESS) {
    // Error should be returned if the failure code is other then ctrlr not
    // supported
    DELETE_IF_NOT_NULL(capa_ckv);
    UPLL_LOG_DEBUG("ValidateCapability Failed: result_code=%d",
                   result_code);
    return result_code;
  }  // capability check ends

  while (temp_ikey != NULL) {
    // check entry existance in CTRLR table
    DbSubOp dbop = { kOpReadSingle, kOpMatchCtrlr, kOpInOutNone };
    ConfigKeyVal *ctrlr_ckv = NULL;
    result_code = GetChildConfigKey(ctrlr_ckv, temp_ikey);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("GetChildConfigKey is failed resultcode=%d",
                     result_code);
      DELETE_IF_NOT_NULL(capa_ckv);
      return result_code;
    }
    SET_USER_DATA_CTRLR(ctrlr_ckv, ctrl_id);
    result_code_ctrlrtbl = ReadConfigDB(ctrlr_ckv, dt_type, UNC_OP_READ,
        dbop, dmi, CTRLRTBL);
    if (result_code_ctrlrtbl == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      if (op == UNC_OP_CREATE || op == UNC_OP_UPDATE) {
        for ( unsigned int loop = 0;
          loop < sizeof(dummy_val->valid)/sizeof(dummy_val->valid[0]);
          ++loop ) {
          if (dummy_val->valid[loop] == UNC_VF_NOT_SUPPORTED) {
            // Update the not_supported fields in set
            GetControllerNotSupportedAttrCol(&ctrlr_not_supported_attr, loop);
          }
        }
        DELETE_IF_NOT_NULL(capa_ckv);
        query_string = GetQueryStringForCtrlrTable(dt_type);

        // If any of the attribute is not supported by the controller then
        // the stores the value as INVALID for that column attribute
        query_string += SelectColumnsDynamically(ctrl_id,
                                                dt_type,
                                                &ctrlr_not_supported_attr);
        // UPLL_LOG_DEBUG("SQLQUERY =%s", query_string.c_str());

        if (!ctrlr_not_supported_attr.empty())
          ctrlr_not_supported_attr.clear();

        ConfigKeyVal *fle_ckv = NULL;
        result_code = GetChildConfigKey(fle_ckv, ikey);
        void *tkey = fle_ckv->get_key();
        void *p = NULL;
        const uudst::kDalTableIndex tbl_index = GetTable(CTRLRTBL, dt_type);
        DalBindInfo *db_info = new DalBindInfo(tbl_index);
        BindInfo *bindinfo_maintbl = flowlistentry_bind_info;
        //  Bind match flowlist
        uint8_t array_index = FLOWLIST_COL_INDEX_MAINTBL;
        uint64_t indx = bindinfo_maintbl[array_index].index;
        p = reinterpret_cast<void *>(reinterpret_cast<char *>(tkey)
          + bindinfo_maintbl[array_index].offset);
        db_info->BindMatch(indx, bindinfo_maintbl[array_index].app_data_type,
                       bindinfo_maintbl[array_index].array_size, p);
        unc_keytype_operation_t ctrl_tbl_op = UNC_OP_CREATE;
        bool is_delete_dirty =  false;
        if (dt_type == UPLL_DT_CANDIDATE) {
          is_delete_dirty =  dmi->IsTableDirtyShallowForOp(tbl_index,
                                                           UNC_OP_DELETE,
                                                           config_mode,
                                                           vtnname);
          if (is_delete_dirty) {
            // Check the records exists in the flowlist entry ctrlr table at
            // running Configuration  or not for the given flowlist and the
            // controller id.
            // If record exists then set the UPDATE operation code while calling
            // DB access layer API so as to UPDATE the running DB(ctrlr tbl)
            key_flowlist_entry_t *key_fle =
               reinterpret_cast<key_flowlist_entry_t *>(ctrlr_ckv->get_key());
            key_fle->sequence_num = 0;
            DbSubOp runningdb_dbop = { kOpReadExist, kOpMatchCtrlr,
              kOpInOutNone };
            result_code = UpdateConfigDB(ctrlr_ckv, UPLL_DT_RUNNING,
                                         UNC_OP_READ, dmi, &runningdb_dbop,
                                         CTRLRTBL);
            if (UPLL_RC_ERR_INSTANCE_EXISTS == result_code) {
              ctrl_tbl_op = UNC_OP_UPDATE;
            } else if (UPLL_RC_ERR_NO_SUCH_INSTANCE !=result_code) {
              DELETE_IF_NOT_NULL(fle_ckv);
              DELETE_IF_NOT_NULL(db_info);
              DELETE_IF_NOT_NULL(ctrlr_ckv);
              UPLL_LOG_INFO("Failed to get flowlistentry from runningdb, rc %d",
                          result_code);
              return result_code;
            }
          }
        }
        // Invoke the query_string
        uint8_t *vtnname = NULL;
        if (!vtn_name.empty()) {
          vtnname = reinterpret_cast<uint8_t *>(const_cast<char *>
                        (vtn_name.c_str()));
        }
        result_code = DalToUpllResCode(
                      dmi->ExecuteAppQuery(query_string, dt_type, tbl_index,
                      db_info, ctrl_tbl_op, config_mode, vtnname));
        DELETE_IF_NOT_NULL(db_info);
        if (UPLL_RC_SUCCESS != result_code) {
          UPLL_LOG_DEBUG("SQLQUERY =%s", query_string.c_str());
          UPLL_LOG_INFO("Failed to update the flowlistentry in ctrlrtbl, rc %d",
                        result_code);
          DELETE_IF_NOT_NULL(ctrlr_ckv);
          DELETE_IF_NOT_NULL(fle_ckv);
          return result_code;
        }
        if (is_delete_dirty && dt_type == UPLL_DT_CANDIDATE &&
            ctrl_tbl_op == UNC_OP_UPDATE) {
          SET_USER_DATA_CTRLR(fle_ckv, ctrl_id);
          tkey = fle_ckv->get_key();
          db_info = new DalBindInfo(tbl_index);
           // Bind the ctrlr table bindinfo array
          BindInfo *bindinfo_ctrlrtbl = flowlistentry_controller_bind_info;
          // Bind FlowList
          array_index = FLOWLIST_COL_INDEX_CTRLTBL;
          indx = bindinfo_ctrlrtbl[array_index].index;
          void *bindptr =
              reinterpret_cast<void *>(reinterpret_cast<char *>(tkey)
              + bindinfo_ctrlrtbl[array_index].offset);
          db_info->BindMatch(indx, bindinfo_ctrlrtbl[array_index].app_data_type,
                       bindinfo_ctrlrtbl[array_index].array_size, bindptr);
          // Bind CtrlrName
          array_index += FLE_CTRL_COL_INDEX_CTRLTBL;
          indx = bindinfo_ctrlrtbl[array_index].index;
          key_user_data *tuser_data  = reinterpret_cast<key_user_data_t *>
                                                 (fle_ckv->get_user_data());
          bindptr =
              reinterpret_cast<void *>(reinterpret_cast<char *>(tuser_data)
              + bindinfo_ctrlrtbl[array_index].offset);
          db_info->BindMatch(indx, bindinfo_ctrlrtbl[array_index].app_data_type,
                       bindinfo_ctrlrtbl[array_index].array_size, bindptr);
          uint8_t *vtnname = NULL;
          if (!vtn_name.empty()) {
            vtnname = reinterpret_cast<uint8_t *>(const_cast<char *>
                          (vtn_name.c_str()));
          }
          result_code = DalToUpllResCode(
                dmi->ExecuteAppQuery(QUERY_CA_UPDATE_C_U_FLAGS_IN_FLE_CTRLRTBL,
                                     dt_type, tbl_index, db_info, ctrl_tbl_op,
                                      config_mode, vtnname));
          DELETE_IF_NOT_NULL(db_info);
          if (UPLL_RC_SUCCESS != result_code) {
            if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
              UPLL_LOG_DEBUG("No Records found for update");
              result_code  = UPLL_RC_SUCCESS;
            } else {
              UPLL_LOG_INFO("Failed to update flowlistentry in ctrlrtbl, rc%d",
                          result_code);
            }
            DELETE_IF_NOT_NULL(fle_ckv);
            DELETE_IF_NOT_NULL(ctrlr_ckv);
            return result_code;
          }
        }
        // Delete the ca_del record if the entry is created/updated in ctrlr tbl
        if (ctrl_tbl_op == UNC_OP_UPDATE && dt_type == UPLL_DT_CANDIDATE
            && is_delete_dirty) {
          key_flowlist_entry_t *key_fle =
             reinterpret_cast<key_flowlist_entry_t *>(fle_ckv->get_key());
          key_fle->sequence_num = 0;
          result_code = UpdateConfigDB(fle_ckv, UPLL_DT_CANDIDATE_DEL,
                                       UNC_OP_DELETE, dmi, config_mode,
                                       vtn_name, CTRLRTBL);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_INFO("UpdateConfigDB failed for delete records in"
                          " ca_del_fle_ctrlr_tbl. result:%d ", result_code);
            DELETE_IF_NOT_NULL(fle_ckv);
            DELETE_IF_NOT_NULL(ctrlr_ckv);
            return result_code;
          }
        }
        DELETE_IF_NOT_NULL(fle_ckv);
      }
      DELETE_IF_NOT_NULL(ctrlr_ckv);
      DELETE_IF_NOT_NULL(capa_ckv);
      return result_code;
    } else if (UPLL_RC_SUCCESS == result_code_ctrlrtbl) {
      if (op == UNC_OP_UPDATE) {
          val_flowlist_entry_t *val_flowlist_entry =
            reinterpret_cast<val_flowlist_entry_t *>
            (GetVal(temp_ikey));

          val_flowlist_entry_ctrl = reinterpret_cast<val_flowlist_entry_ctrl_t*>
            (GetVal(ctrlr_ckv));
          // copy the valid flags from MAINTbl to CTRLRTbl and assign INVALID
          // for controller not_supported fields
          for ( unsigned int loop = 0;
              loop < sizeof
              (val_flowlist_entry->valid)/sizeof(val_flowlist_entry->valid[0]);
              ++loop ) {
            if (dummy_val->valid[loop] == UNC_VF_NOT_SUPPORTED) {
               val_flowlist_entry_ctrl->valid[loop] = UNC_VF_INVALID;
            } else {
              val_flowlist_entry_ctrl->valid[loop] =
                 val_flowlist_entry->valid[loop];
            }
          }
        // copy the cs_attr and cs_row_status from MAINTbl to CTRLRTbl
        if (UPLL_DT_AUDIT == dt_type) {
          val_flowlist_entry_t *val_flowlist_entry =
            reinterpret_cast<val_flowlist_entry_t *>
            (GetVal(temp_ikey));
          val_flowlist_entry_ctrl = reinterpret_cast<val_flowlist_entry_ctrl_t*>
            (GetVal(ctrlr_ckv));
          for ( unsigned int loop = 0;
              loop < sizeof
              (val_flowlist_entry->valid)/sizeof(val_flowlist_entry->valid[0]);
              ++loop ) {
            val_flowlist_entry_ctrl->cs_attr[loop] =
                (unc_keytype_configstatus_t)
                val_flowlist_entry->cs_attr[loop];
          }
          val_flowlist_entry_ctrl->cs_row_status = (unc_keytype_configstatus_t)
              val_flowlist_entry->cs_row_status;
        }
        SET_USER_DATA_CTRLR(ctrlr_ckv, ctrl_id);
        result_code = UpdateConfigDB(ctrlr_ckv, dt_type,
                                     op, dmi, config_mode,
                                     vtn_name, CTRLRTBL);

        if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
          DELETE_IF_NOT_NULL(ctrlr_ckv);
          temp_ikey = temp_ikey->get_next_cfg_key_val();
          continue;
        }
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("Unable to update a FlowListEntry in Ctrlr table=%d",
              result_code);
        }
      }
      DELETE_IF_NOT_NULL(ctrlr_ckv);
    } else {
      UPLL_LOG_DEBUG("ReadConfigDB failed %d ", result_code);
      DELETE_IF_NOT_NULL(ctrlr_ckv);
      DELETE_IF_NOT_NULL(capa_ckv);
      return result_code;
    }
    temp_ikey = temp_ikey->get_next_cfg_key_val();
  }
  DELETE_IF_NOT_NULL(capa_ckv);
  return result_code;
}

upll_rc_t FlowListEntryMoMgr::TxCopyCandidateToRunning(
                                     unc_key_type_t keytype,
                                     CtrlrCommitStatusList *ctrlr_commit_status,
                                     DalDmlIntf *dmi,
                                     TcConfigMode config_mode,
                                     std::string vtn_name) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  DalResultCode db_result;
  unc_keytype_operation_t op[]= { UNC_OP_DELETE, UNC_OP_CREATE, UNC_OP_UPDATE};
  int nop = sizeof(op) / sizeof(op[0]);
  ConfigKeyVal *fle_key = NULL, *req = NULL;
  ConfigKeyVal *nreq = NULL , *fl_run_key = NULL;
  ConfigKeyVal *fl_run_ctrl_key = NULL , *fl_ck_run = NULL;
  DalCursor *cfg1_cursor = NULL;
  uint8_t *ctrlr_id = NULL;
#if 0
  IpcReqRespHeader *req_header = reinterpret_cast<IpcReqRespHeader *>
      (ConfigKeyVal::Malloc(sizeof(IpcReqRespHeader)));
#endif
  map<string, int> ctrlr_result;
  CtrlrCommitStatusList::iterator ccsListItr;
  CtrlrCommitStatus *ccStatusPtr;

  if (ctrlr_commit_status != NULL) {
    for (ccsListItr = ctrlr_commit_status->begin();
        ccsListItr != ctrlr_commit_status->end(); ++ccsListItr) {
      ccStatusPtr = *ccsListItr;
      ctrlr_id = reinterpret_cast<uint8_t *>(&ccStatusPtr->ctrlr_id);
      ctrlr_result[ccStatusPtr->ctrlr_id] = ccStatusPtr->upll_ctrlr_result;
      if (ccStatusPtr->upll_ctrlr_result != UPLL_RC_SUCCESS) {
        for (ConfigKeyVal *ck_err = ccStatusPtr->err_ckv; ck_err != NULL;
             ck_err = ck_err->get_next_cfg_key_val()) {
          if (ck_err->get_key_type() != keytype) continue;
          result_code = GetRenamedUncKey(ck_err, UPLL_DT_CANDIDATE, dmi,
              ctrlr_id);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_ERROR(
                "FlowListEntryMoMgr::GetRenamedUncKey is failed, "
                "resultcode= %d", result_code);
            return result_code;
          }
        }
      }
    }
  }

  if (config_mode != TC_CONFIG_VTN) {
    for (int i = 0; i < nop; i++) {
      if (op[i] != UNC_OP_UPDATE) {
        result_code = DiffConfigDB(UPLL_DT_CANDIDATE, UPLL_DT_RUNNING, op[i],
                                 req, nreq, &cfg1_cursor, dmi, NULL,
                                 config_mode, vtn_name, MAINTBL, true);
        while (result_code == UPLL_RC_SUCCESS) {
          db_result = dmi->GetNextRecord(cfg1_cursor);
          result_code = DalToUpllResCode(db_result);
          if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
            result_code = UPLL_RC_SUCCESS;
            break;
          }
          result_code = UpdateMainTbl(req, op[i], UPLL_RC_SUCCESS,
                                    nreq, dmi);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("Updating Main table Error %d", result_code);
            if (cfg1_cursor)
              dmi->CloseCursor(cfg1_cursor, true);
            DELETE_IF_NOT_NULL(req);
            DELETE_IF_NOT_NULL(nreq);
            return result_code;
          }
        }
        if (cfg1_cursor) {
          dmi->CloseCursor(cfg1_cursor, true);
          cfg1_cursor = NULL;
        }
        DELETE_IF_NOT_NULL(req);
        DELETE_IF_NOT_NULL(nreq);
      }
      UPLL_LOG_DEBUG("Updating main table complete with op %d", op[i]);
    }   // for loop
  }  //  if loop

  // if (config_mode != TC_CONFIG_VIRTUAL) {
    for (int i = 0; i < nop; i++) {
      MoMgrTables tbl = (op[i] == UNC_OP_UPDATE)?MAINTBL:CTRLRTBL;
      if ((config_mode == TC_CONFIG_VIRTUAL) &&
           (op[i] != UNC_OP_UPDATE)) {
        continue;
      } else if ((config_mode == TC_CONFIG_VTN) &&
                 (op[i] == UNC_OP_UPDATE)) {
        continue;
      }
      result_code = DiffConfigDB(UPLL_DT_CANDIDATE, UPLL_DT_RUNNING, op[i], req,
                               nreq, &cfg1_cursor, dmi, NULL, config_mode,
                               vtn_name, tbl, true);

      ConfigKeyVal *fle_ctrlr_key = NULL;
      while (result_code == UPLL_RC_SUCCESS) {
        db_result = dmi->GetNextRecord(cfg1_cursor);
        result_code = DalToUpllResCode(db_result);
        if (result_code != UPLL_RC_SUCCESS)
          break;
        if ((op[i] == UNC_OP_UPDATE) && (config_mode == TC_CONFIG_GLOBAL ||
             config_mode == TC_CONFIG_VIRTUAL)) {
          DbSubOp dbop = { kOpReadMultiple, kOpMatchNone,
          kOpInOutCtrlr | kOpInOutCs };
          result_code = GetChildConfigKey(fle_ctrlr_key, req);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("GetChildConfigKey is failed resultcode=%d",
                           result_code);
            if (cfg1_cursor)
             dmi->CloseCursor(cfg1_cursor, true);
            DELETE_IF_NOT_NULL(req);
            DELETE_IF_NOT_NULL(nreq);
            return result_code;
          }
          /* Capability check
           * req_header->operation = op[i];
           * strcpy((char*)req_header->datatype, (char*)UNC_DT_CANDIDATE);
           * result_code = ValidateCapability(req_header, vtn_ctrlr_key);
           *                                         */
          result_code = ReadConfigDB(fle_ctrlr_key, UPLL_DT_CANDIDATE,
                                   UNC_OP_READ, dbop, dmi, CTRLRTBL);
          if (result_code != UPLL_RC_SUCCESS) {
            DELETE_IF_NOT_NULL(fle_ctrlr_key);
            if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
              UPLL_LOG_DEBUG("No record found in FlowlistEntry Ctrlr Tbl");
              result_code = UpdateMainTbl(req, op[i], UPLL_RC_SUCCESS,
                                        nreq, dmi);
              if (result_code != UPLL_RC_SUCCESS) {
                UPLL_LOG_DEBUG("Error updating main table%d", result_code);
                if (cfg1_cursor)
                  dmi->CloseCursor(cfg1_cursor, true);
                DELETE_IF_NOT_NULL(req);
                DELETE_IF_NOT_NULL(nreq);
                return result_code;
              } else {
                continue;
              }
            } else  {
              UPLL_LOG_DEBUG("DB err while reading records from ctrlrtbl,"
                            "err %d", result_code);

              if (cfg1_cursor)
                dmi->CloseCursor(cfg1_cursor, true);
              DELETE_IF_NOT_NULL(req);
              DELETE_IF_NOT_NULL(nreq);
              return result_code;
            }
          }
          for (ConfigKeyVal *tmp = fle_ctrlr_key; tmp != NULL; tmp =
              tmp->get_next_cfg_key_val()) {
            GET_USER_DATA_CTRLR(tmp, ctrlr_id);
            string controller(reinterpret_cast<char *>(ctrlr_id));

            UPLL_LOG_DEBUG("Controller ID =%s", controller.c_str());
            DbSubOp dbop_maintbl = { kOpReadSingle, kOpMatchNone,
                  kOpInOutFlag | kOpInOutCs };
            result_code = GetChildConfigKey(fle_key, req);
            if (result_code != UPLL_RC_SUCCESS) {
              UPLL_LOG_DEBUG("GetChildConfigKey is failed resultcode=%d",
                             result_code);
              DELETE_IF_NOT_NULL(fle_ctrlr_key);
              if (cfg1_cursor)
                dmi->CloseCursor(cfg1_cursor, true);
              DELETE_IF_NOT_NULL(req);
              DELETE_IF_NOT_NULL(nreq);
              return result_code;
            }
            result_code = ReadConfigDB(fle_key, UPLL_DT_CANDIDATE,
                                     UNC_OP_READ, dbop_maintbl, dmi, MAINTBL);
            if (result_code != UPLL_RC_SUCCESS) {
              UPLL_LOG_DEBUG("Unable to read configuration from RunningDb");
              DELETE_IF_NOT_NULL(fle_key);
              DELETE_IF_NOT_NULL(fle_ctrlr_key);
              if (cfg1_cursor)
                dmi->CloseCursor(cfg1_cursor, true);
            DELETE_IF_NOT_NULL(req);
            DELETE_IF_NOT_NULL(nreq);
            return result_code;
          }
          static_cast<val_flowlist_entry_t *>
              (GetVal(fle_key))->cs_row_status =
              static_cast<val_flowlist_entry_t *>
                      (GetVal(nreq))->cs_row_status;

         //  ReadingConfigstatus for main tbl
          result_code = GetChildConfigKey(fl_run_key, req);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("GetChildConfigKey is failed resultcode=%d",
                           result_code);
            DELETE_IF_NOT_NULL(fle_key);
            DELETE_IF_NOT_NULL(fle_ctrlr_key);
            if (cfg1_cursor)
               dmi->CloseCursor(cfg1_cursor, true);
            DELETE_IF_NOT_NULL(req);
            DELETE_IF_NOT_NULL(nreq);

            return result_code;
          }

          result_code = ReadConfigDB(fl_run_key, UPLL_DT_RUNNING  ,
                                     UNC_OP_READ, dbop_maintbl, dmi, MAINTBL);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("Unable to read configuration from RunningDb");
            DELETE_IF_NOT_NULL(fle_key);
            DELETE_IF_NOT_NULL(fle_ctrlr_key);
            DELETE_IF_NOT_NULL(fl_run_key);
            if (cfg1_cursor)
              dmi->CloseCursor(cfg1_cursor, true);
            DELETE_IF_NOT_NULL(req);
            DELETE_IF_NOT_NULL(nreq);
            return result_code;
          }

          val_flowlist_entry_t *val_main_can = reinterpret_cast
           <val_flowlist_entry_t *>(GetVal(fle_key));
          val_flowlist_entry_t *val_main = reinterpret_cast
           <val_flowlist_entry_t *>(GetVal(fl_run_key));
          for (unsigned int loop = 0; loop < sizeof(val_main->valid)/
            sizeof(val_main->valid[0]); ++loop) {
            val_main_can->cs_attr[loop] = val_main->cs_attr[loop];
          }

          // For Reading The controller table for config status
          result_code = GetChildConfigKey(fl_run_ctrl_key, tmp);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("GetChildConfigKey is failed resultcode=%d",
                           result_code);
            DELETE_IF_NOT_NULL(fle_key);
            DELETE_IF_NOT_NULL(fle_ctrlr_key);
            DELETE_IF_NOT_NULL(fl_run_key);
            if (cfg1_cursor)
               dmi->CloseCursor(cfg1_cursor, true);
            DELETE_IF_NOT_NULL(req);
            DELETE_IF_NOT_NULL(nreq);
            return result_code;
          }
          result_code = ReadConfigDB(fl_run_ctrl_key, UPLL_DT_RUNNING  ,
                                     UNC_OP_READ, dbop_maintbl, dmi, CTRLRTBL);
          if ((result_code != UPLL_RC_SUCCESS) &&
              (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE)) {
            UPLL_LOG_DEBUG("Unable to read configuration from CandidateDb");
            DELETE_IF_NOT_NULL(fle_key);
            DELETE_IF_NOT_NULL(fle_ctrlr_key);
            DELETE_IF_NOT_NULL(fl_run_key);
            DELETE_IF_NOT_NULL(fl_run_ctrl_key);
            if (cfg1_cursor)
               dmi->CloseCursor(cfg1_cursor, true);
            DELETE_IF_NOT_NULL(req);
            DELETE_IF_NOT_NULL(nreq);
            return result_code;
          }
          if (result_code == UPLL_RC_SUCCESS) {
            val_flowlist_entry_ctrl *val_ctrlr_can = reinterpret_cast
             <val_flowlist_entry_ctrl_t *>(GetVal(tmp));
            val_flowlist_entry_ctrl *val_ctrlr_run = reinterpret_cast
              <val_flowlist_entry_ctrl_t *>(GetVal(fl_run_ctrl_key));
            val_ctrlr_can->cs_row_status = val_ctrlr_run->cs_row_status;

            for (unsigned int loop = 0; loop < sizeof(val_ctrlr_run->valid)/
                 sizeof(val_ctrlr_run->valid[0]); ++loop) {
              val_ctrlr_can->cs_attr[loop] = val_ctrlr_run->cs_attr[loop];
            }
            // End Reading The controller table for config status
            if (ctrlr_result.empty()) {
              UPLL_LOG_TRACE("ctrlr_commit_status is NULL.");
              result_code = UpdateConfigStatus(fle_key, op[i],
                                               UPLL_RC_ERR_CTR_DISCONNECTED,
                                               nreq, dmi, tmp);
            } else {
              result_code = UpdateConfigStatus(fle_key, op[i],
                                               ctrlr_result[controller], nreq,
                                               dmi, tmp);
            }
            if (result_code != UPLL_RC_SUCCESS) {
               UPLL_LOG_DEBUG("UpdateConfigStatus Failed , err %d",
                              result_code);
               break;
            }
            DELETE_IF_NOT_NULL(fl_run_key);
            DELETE_IF_NOT_NULL(fl_run_ctrl_key);
            void *fle_val1 = GetVal(tmp);
            void *fle_val2 = GetVal(nreq);
            void *fle_val3 = GetVal(req);
            CompareValidVal(fle_val1, fle_val2, fle_val3, false);
            result_code = UpdateConfigDB(tmp,
                                         UPLL_DT_RUNNING, op[i], dmi,
                                         config_mode, vtn_name, CTRLRTBL);
            if (result_code != UPLL_RC_SUCCESS) {
              UPLL_LOG_DEBUG("UpdateConfigDB for ctrlr tbl is failed ");
              DELETE_IF_NOT_NULL(fle_ctrlr_key);
              DELETE_IF_NOT_NULL(fle_key);
              if (cfg1_cursor)
                dmi->CloseCursor(cfg1_cursor, true);
              DELETE_IF_NOT_NULL(req);
              DELETE_IF_NOT_NULL(nreq);
              return result_code;
            }
          } else {
            DELETE_IF_NOT_NULL(fl_run_ctrl_key);
          }
          result_code = UpdateConfigDB(fle_key, UPLL_DT_RUNNING,
                                       op[i], dmi, config_mode, vtn_name,
                                       MAINTBL);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("UpdateConfigDB for main tbl is failed");
            DELETE_IF_NOT_NULL(fle_ctrlr_key);
            DELETE_IF_NOT_NULL(fle_key);
            if (cfg1_cursor)
              dmi->CloseCursor(cfg1_cursor, true);
            DELETE_IF_NOT_NULL(req);
            DELETE_IF_NOT_NULL(nreq);
            return result_code;
          }  // COV UNREACHABLE
          EnqueCfgNotification(op[i], UPLL_DT_RUNNING,
                               fle_key);
          DELETE_IF_NOT_NULL(fle_key);
        }
      } else {
        if ((op[i] == UNC_OP_CREATE) && (config_mode != TC_CONFIG_VIRTUAL)) {
          DbSubOp dbop = { kOpReadSingle, kOpMatchNone,
            kOpInOutFlag |kOpInOutCs };
          result_code = GetChildConfigKey(fle_key, req);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("GetChildConfigKey is failed resultcode=%d",
                           result_code);
            if (cfg1_cursor)
              dmi->CloseCursor(cfg1_cursor, true);
            DELETE_IF_NOT_NULL(req);
            DELETE_IF_NOT_NULL(nreq);
            return result_code;
          }

          result_code = ReadConfigDB(fle_key, UPLL_DT_RUNNING,
                                     UNC_OP_READ, dbop, dmi, MAINTBL);
          if ((result_code != UPLL_RC_SUCCESS) &&
             (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE)) {
             UPLL_LOG_DEBUG("ReadConfigDB is failed -%d", result_code);
             DELETE_IF_NOT_NULL(fle_key);
             if ((config_mode == TC_CONFIG_VTN) &&
                 (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE)) {
               continue;
             }
             if (cfg1_cursor)
               dmi->CloseCursor(cfg1_cursor, true);
             DELETE_IF_NOT_NULL(req);
             DELETE_IF_NOT_NULL(nreq);
             return result_code;
          }
          /* Capability check
           * req_header->operation = op[i];
           * strcpy((char*)req_header->datatype, (char*)UNC_DT_CANDIDATE);
           * result_code = ValidateCapability(req_header, vtn_ctrlr_key);
           *                                                 */

          // set consolidated config status to UNKNOWN to init vtn cs_status
          // to the cs_status of first controller
          uint32_t cur_instance_count;
          result_code = GetInstanceCount(fle_key, NULL,
                                   UPLL_DT_CANDIDATE, &cur_instance_count,
                                   dmi, CTRLRTBL);
          if ((result_code == UPLL_RC_SUCCESS) && (cur_instance_count == 1))
            reinterpret_cast<val_flowlist_entry *>(
                GetVal(fle_key))->cs_row_status = UNC_CS_UNKNOWN;
          result_code = DupConfigKeyVal(fle_ctrlr_key, req, tbl);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("DupConfigKeyVal is failed -%d", result_code);
            DELETE_IF_NOT_NULL(fle_key);
            if (cfg1_cursor)
              dmi->CloseCursor(cfg1_cursor, true);
            DELETE_IF_NOT_NULL(req);
            DELETE_IF_NOT_NULL(nreq);
            return result_code;
          }

          GET_USER_DATA_CTRLR(fle_ctrlr_key, ctrlr_id);
          string controller(reinterpret_cast<char *>(ctrlr_id));
          if (ctrlr_result.empty()) {
            UPLL_LOG_TRACE("ctrlr_commit_status is NULL.");
            result_code = UpdateConfigStatus(fle_key, op[i],
                                             UPLL_RC_ERR_CTR_DISCONNECTED, nreq,
                                             dmi, fle_ctrlr_key);
          } else {
            result_code = UpdateConfigStatus(fle_key, op[i],
                                             ctrlr_result[controller], nreq,
                                             dmi, fle_ctrlr_key);
          }
        } else if ((op[i] == UNC_OP_DELETE) &&
                   (config_mode != TC_CONFIG_VIRTUAL)) {
           // Reading Main Running DB for delete op
           DbSubOp dbop1 = { kOpReadSingle, kOpMatchNone,
                             kOpInOutFlag | kOpInOutCs };
          GET_USER_DATA_CTRLR(req, ctrlr_id);
          result_code = GetChildConfigKey(fl_ck_run, req);
           if (result_code != UPLL_RC_SUCCESS) {
             UPLL_LOG_DEBUG("GetChildConfigKey is failed resultcode=%d",
                         result_code);
            if (cfg1_cursor)
              dmi->CloseCursor(cfg1_cursor, true);
            DELETE_IF_NOT_NULL(req);
            DELETE_IF_NOT_NULL(nreq);
            return result_code;
           }

           GET_USER_DATA_CTRLR(req, ctrlr_id);
           result_code = ReadConfigDB(fl_ck_run, UPLL_DT_RUNNING,
                                   UNC_OP_READ, dbop1, dmi, MAINTBL);

           if (result_code != UPLL_RC_SUCCESS &&
                 result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
             UPLL_LOG_DEBUG("Unable to read configuration from RunningDB");
             DELETE_IF_NOT_NULL(fl_ck_run);
             if (cfg1_cursor)
              dmi->CloseCursor(cfg1_cursor, true);
             DELETE_IF_NOT_NULL(req);
             DELETE_IF_NOT_NULL(nreq);
             return result_code;
           }
           if (result_code == UPLL_RC_SUCCESS) {
             // If Record exists in MainTBL then perform consolidation
             result_code = SetFlowlistEntryConsolidatedStatus(fl_ck_run,
                                                              ctrlr_id, dmi);
             if (result_code != UPLL_RC_SUCCESS) {
               UPLL_LOG_DEBUG("Could not set consolidated status %d",
                               result_code);
               DELETE_IF_NOT_NULL(fl_ck_run);
               if (cfg1_cursor)
                dmi->CloseCursor(cfg1_cursor, true);
               DELETE_IF_NOT_NULL(req);
               DELETE_IF_NOT_NULL(nreq);
               return result_code;
             }
           }
           DELETE_IF_NOT_NULL(fl_ck_run);
           result_code = GetChildConfigKey(fle_ctrlr_key, req);
           if (result_code != UPLL_RC_SUCCESS) {
             UPLL_LOG_DEBUG("Error in getting the GetChildConfigKey, err=%d",
                             result_code);
             if (cfg1_cursor)
               dmi->CloseCursor(cfg1_cursor, true);
             DELETE_IF_NOT_NULL(req);
             DELETE_IF_NOT_NULL(nreq);
             return result_code;
           }
         }
         result_code = UpdateConfigDB(fle_ctrlr_key, UPLL_DT_RUNNING,
                                    op[i], dmi, config_mode, vtn_name,
                                    CTRLRTBL);
           if (result_code != UPLL_RC_SUCCESS) {
             UPLL_LOG_DEBUG("UpdateConfigDB in ctrlr tbl is failed -%d",
                            result_code);
             DELETE_IF_NOT_NULL(fle_ctrlr_key);
             if (cfg1_cursor)
               dmi->CloseCursor(cfg1_cursor, true);
             DELETE_IF_NOT_NULL(req);
             DELETE_IF_NOT_NULL(nreq);
             DELETE_IF_NOT_NULL(fle_key);
             return result_code;
           }
           if (op[i] != UNC_OP_DELETE) {
             result_code = UpdateConfigDB(fle_key, UPLL_DT_RUNNING,
                                       UNC_OP_UPDATE, dmi, config_mode,
                                       vtn_name, MAINTBL);
            if (result_code != UPLL_RC_SUCCESS) {
              UPLL_LOG_DEBUG("UpdateConfigDB in main tbl is failed -%d",
                             result_code);
              DELETE_IF_NOT_NULL(fle_key);
              DELETE_IF_NOT_NULL(fle_ctrlr_key);
              if (cfg1_cursor)
                dmi->CloseCursor(cfg1_cursor, true);
              DELETE_IF_NOT_NULL(req);
              DELETE_IF_NOT_NULL(nreq);
              return result_code;
            }
          }
          EnqueCfgNotification(op[i], UPLL_DT_RUNNING,
                              fle_key);
        }
        if (fle_key) delete fle_key;
          DELETE_IF_NOT_NULL(fle_ctrlr_key);
          fle_key = fle_ctrlr_key = NULL;
          result_code = DalToUpllResCode(db_result);
        }
        if (cfg1_cursor) {
          dmi->CloseCursor(cfg1_cursor, true);
          cfg1_cursor = NULL;
        }
        if (nreq) delete nreq;
        if (req) delete req;
        nreq = req = NULL;
     }  // for loop
  // }  // if loop
  result_code = (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) ?
      UPLL_RC_SUCCESS : result_code;
  UPLL_LOG_DEBUG("TxcopyCandidateto Running is successful -%d", result_code);
  return result_code;
}

upll_rc_t
FlowListEntryMoMgr::SetFlowlistEntryConsolidatedStatus(ConfigKeyVal *ikey,
                                                       uint8_t *ctrlr_id,
                                                       DalDmlIntf *dmi)  {
  UPLL_FUNC_TRACE;
  UPLL_LOG_DEBUG("Input controller id  %s", ctrlr_id);
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *ctrlr_ckv = NULL;
  val_flowlist_entry_ctrl_t *ctrlr_val = NULL;
  uint8_t *fle_exist_on_ctrlr = NULL;
  bool applied = false, not_applied = false, invalid = false;
  unc_keytype_configstatus_t c_status = UNC_CS_NOT_APPLIED;
  string vtn_name = "";

  DbSubOp dbop = { kOpReadMultiple, kOpMatchNone,
                   kOpInOutCtrlr | kOpInOutDomain | kOpInOutCs };
  if (!ikey || !dmi) {
    UPLL_LOG_DEBUG("Invalid Input");
    return UPLL_RC_ERR_GENERIC;
  }
  result_code = GetChildConfigKey(ctrlr_ckv, ikey);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetChildConfigKey failed err code %d", result_code);
    return result_code;
  }
  result_code = ReadConfigDB(ctrlr_ckv, UPLL_DT_RUNNING, UNC_OP_READ, dbop, dmi,
                             CTRLRTBL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("ReadConfigDB from ctrltbl failed err code %d",
                   result_code);
    DELETE_IF_NOT_NULL(ctrlr_ckv);
    return result_code;
  }

  for (ConfigKeyVal *tmp = ctrlr_ckv; tmp != NULL;
                     tmp = tmp->get_next_cfg_key_val()) {
    ctrlr_val = reinterpret_cast<val_flowlist_entry_ctrl_t *>(GetVal(tmp));
    if (!ctrlr_val) {
      UPLL_LOG_ERROR("Controller Value is empty");
      tmp = NULL;
      delete ctrlr_ckv;
      return UPLL_RC_ERR_GENERIC;
    }
    GET_USER_DATA_CTRLR(tmp, fle_exist_on_ctrlr);
    UPLL_LOG_DEBUG("internally read ctrollername %s", fle_exist_on_ctrlr);
    if (!strcmp(reinterpret_cast<char *>(fle_exist_on_ctrlr),
                reinterpret_cast<char *>(ctrlr_id)))
      continue;  // skipping entry of deleted controller

    switch (ctrlr_val->cs_row_status) {
      case UNC_CS_APPLIED:
        applied = true;
      break;
      case UNC_CS_NOT_APPLIED:
        not_applied = true;
      break;
      case UNC_CS_INVALID:
        invalid = true;
      break;
      default:
        UPLL_LOG_DEBUG("Invalid status");
        DELETE_IF_NOT_NULL(ctrlr_ckv);
        // return UPLL_RC_ERR_GENERIC;
    }
    fle_exist_on_ctrlr = NULL;
  }
  if (invalid) {
    c_status = UNC_CS_INVALID;
  } else if (applied && !not_applied) {
    c_status = UNC_CS_APPLIED;
  } else if (!applied && not_applied) {
    c_status = UNC_CS_NOT_APPLIED;
  } else if (applied && not_applied) {
    c_status = UNC_CS_PARTIALLY_APPLIED;
  } else {
    c_status = UNC_CS_APPLIED;
  }
  applied = not_applied =false;
  // Set cs_status
  val_flowlist_entry_t *fleval =
     static_cast<val_flowlist_entry_t *>(GetVal(ikey));
  fleval->cs_row_status = c_status;

  for (unsigned int loop = 0; loop < sizeof(fleval->valid)/
           sizeof(fleval->valid[0]); ++loop) {
    for (ConfigKeyVal *tmp = ctrlr_ckv; tmp != NULL;
                     tmp = tmp->get_next_cfg_key_val()) {
      ctrlr_val = reinterpret_cast<val_flowlist_entry_ctrl_t *>(GetVal(tmp));

      GET_USER_DATA_CTRLR(tmp, fle_exist_on_ctrlr);
      UPLL_LOG_DEBUG("internally read ctrollername %s", fle_exist_on_ctrlr);
      if (!strcmp(reinterpret_cast<char *>(fle_exist_on_ctrlr),
                reinterpret_cast<char *>(ctrlr_id)))
        continue;  // skipping entry of deleted controller
       if (ctrlr_val->valid[loop] == UNC_VF_VALID) {
        switch (ctrlr_val->cs_attr[loop]) {
          case UNC_CS_APPLIED:
            applied = true;
        break;
        case UNC_CS_NOT_APPLIED:
          not_applied = true;
        break;
        case UNC_CS_INVALID:
          invalid = true;
        break;
        default:
          UPLL_LOG_DEBUG("Invalid status %d", ctrlr_val->cs_attr[loop]);
        }
      }
    }
    if (invalid) {
      c_status = UNC_CS_INVALID;
    } else if (applied && !not_applied) {
        c_status = UNC_CS_APPLIED;
    } else if (!applied && not_applied) {
        c_status = UNC_CS_NOT_APPLIED;
    } else if (applied && not_applied) {
        c_status = UNC_CS_PARTIALLY_APPLIED;
    } else {
        c_status = UNC_CS_APPLIED;
    }
    fleval->cs_attr[loop] = c_status;
    applied = not_applied =false;
  }

  DbSubOp dbop_update = {kOpNotRead, kOpMatchNone, kOpInOutCs};
  result_code = UpdateConfigDB(ikey, UPLL_DT_RUNNING, UNC_OP_UPDATE, dmi,
                               &dbop_update, TC_CONFIG_GLOBAL, vtn_name,
                               MAINTBL);

  DELETE_IF_NOT_NULL(ctrlr_ckv);
  return result_code;
}

upll_rc_t FlowListEntryMoMgr::UpdateAuditConfigStatus(
                           unc_keytype_configstatus_t cs_status,
                           uuc::UpdateCtrlrPhase phase,
                           ConfigKeyVal *&ckv_running,
                           DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  val_flowlist_entry_ctrl_t *val;
  val = (ckv_running != NULL)?
                            reinterpret_cast<val_flowlist_entry_ctrl_t *>
                            (GetVal(ckv_running)):NULL;
  if (NULL == val) {
    return UPLL_RC_ERR_GENERIC;
  }
  UPLL_LOG_DEBUG("CS Status= %u phase =%u", cs_status, phase);
  if (uuc::kUpllUcpCreate == phase )
    val->cs_row_status = cs_status;
  if ((uuc::kUpllUcpUpdate == phase) &&
           (val->cs_row_status == UNC_CS_INVALID ||
            val->cs_row_status == UNC_CS_NOT_APPLIED))
    val->cs_row_status = cs_status;
  for ( unsigned int loop = 0;
        loop < sizeof(val->valid)/sizeof(val->valid[0]);
        ++loop ) {
    if ((cs_status == UNC_CS_INVALID && UNC_VF_VALID == val->valid[loop]) ||
         cs_status == UNC_CS_APPLIED)
       val->cs_attr[loop] = cs_status;
  }
  return result_code;
}

upll_rc_t FlowListEntryMoMgr::SetConsolidatedStatus(ConfigKeyVal *ikey,
                                                    DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *ckv = NULL;
  string vtn_name = "";

  DbSubOp dbop = {kOpReadMultiple, kOpMatchNone, kOpInOutCs};
  result_code = GetChildConfigKey(ckv, ikey);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("Unable to create ConfigKeyVal");
    return result_code;
  }
  result_code = ReadConfigDB(ckv, UPLL_DT_RUNNING, UNC_OP_READ, dbop , dmi,
                CTRLRTBL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_INFO("Unable to read configuration from RunningDB, ResultCode=%d",
                  result_code);
    delete ckv;
    return result_code;
  }
  std::vector<list<unc_keytype_configstatus_t> > vec_attr;
  std::list< unc_keytype_configstatus_t > list_cs_row;
  val_flowlist_entry_ctrl_t *val;
  for (unsigned int loop = 0; loop < sizeof(val->valid)/sizeof(val->valid[0]);
      ++loop) {
    std::list< unc_keytype_configstatus_t > list_attr;
    vec_attr.push_back(list_attr);
  }
  ConfigKeyVal *temp_ckv = ckv;
  for ( ; temp_ckv != NULL ; temp_ckv = temp_ckv->get_next_cfg_key_val()) {
    val = reinterpret_cast<val_flowlist_entry_ctrl_t *>(GetVal(temp_ckv));
    list_cs_row.push_back((unc_keytype_configstatus_t)val->cs_row_status);
    for (unsigned int loop = 0; loop < sizeof(val->valid)/sizeof(val->valid[0]);
        ++loop) {
      vec_attr[loop].push_back((unc_keytype_configstatus_t)val->cs_attr[loop]);
    }
  }
  DELETE_IF_NOT_NULL(ckv);
  val_flowlist_entry_t *val_temp =
    reinterpret_cast<val_flowlist_entry_t *>(GetVal(ikey));
  val_temp->cs_row_status = GetConsolidatedCsStatus(list_cs_row);
  for (unsigned int loop = 0; loop < sizeof(val->valid)/sizeof(val->valid[0]);
      ++loop) {
    val_temp->cs_attr[loop] = GetConsolidatedCsStatus(vec_attr[loop]);
  }
  result_code = UpdateConfigDB(ikey, UPLL_DT_RUNNING,
                               UNC_OP_UPDATE, dmi, TC_CONFIG_GLOBAL, vtn_name,
                               MAINTBL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_INFO("Unable to Update the Running DB, result_code=%d",
                  result_code);
    return result_code;
  }
  return result_code;
}

upll_rc_t FlowListEntryMoMgr::ValidateCapability(IpcReqRespHeader *req,
                                                 ConfigKeyVal *key,
                                                 const char *ctrlr_name) {
  UPLL_FUNC_TRACE;

  upll_rc_t rt_code = UPLL_RC_ERR_GENERIC;

  if ((NULL == req) || (NULL == key)) {
    UPLL_LOG_DEBUG("IpcReqRespHeader/ConfigKeyval is NULL");
    return rt_code;
  }

  if (!ctrlr_name) {
    ctrlr_name = static_cast<char *>(key->get_user_data());
  }

  if (NULL == ctrlr_name) {
    UPLL_LOG_DEBUG("ctrlr_name is NULL");
    return UPLL_RC_ERR_GENERIC;
  }

  UPLL_LOG_DEBUG("ctrlr_name(%s), datatype :(%d)",
               ctrlr_name, req->datatype);

  bool result_code = false;
  uint32_t max_instance_count = 0;
  const uint8_t *attrs = NULL;
  uint32_t max_attrs = 0;

  switch (req->operation) {
    case UNC_OP_CREATE: {
      result_code = GetCreateCapability(ctrlr_name, key->get_key_type(),
                                      &max_instance_count, &max_attrs, &attrs);
      break;
    }
    case UNC_OP_UPDATE: {
      result_code = GetUpdateCapability(ctrlr_name, key->get_key_type(),
                                        &max_attrs, &attrs);
      break;
    }
    default: {
      if (req->datatype == UPLL_DT_STATE) {
        result_code = GetStateCapability(ctrlr_name, key->get_key_type(),
                                      &max_attrs, &attrs);
      } else {
        result_code = GetReadCapability(ctrlr_name, key->get_key_type(),
                                      &max_attrs, &attrs);
      }
    }
  }

  if (!result_code) {
    UPLL_LOG_DEBUG("keytype(%d) is not supported by controller(%s) "
                   "for opeartion(%d)",
                   key->get_key_type(), ctrlr_name, req->operation);
    return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
  }
  val_flowlist_entry_t *val_flowlist_entry =
     static_cast<val_flowlist_entry_t *>(GetVal(key));

  if (val_flowlist_entry) {
    if (max_attrs > 0) {
      return ValFlowlistEntryAttributeSupportCheck(val_flowlist_entry,
            attrs);
    } else {
      UPLL_LOG_DEBUG("Attribute list is empty for operation %d",
                     req->operation);
      return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
    }
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t FlowListEntryMoMgr::ValFlowlistEntryAttributeSupportCheck(
    val_flowlist_entry_t *val_flowlist_entry, const uint8_t* attrs ) {
  UPLL_FUNC_TRACE;

  /** validate dst_mac, src_mac attributes*/
  ValidateMacAttribute(val_flowlist_entry, attrs);

  /** validate dst_ip, dst_ip_prefixlen, src_ip, src_ipprefixlen attributes */
  ValidateIPAttribute(val_flowlist_entry, attrs);

  /** validate mac_eth_type attribyte */
  ValidateMacEthTypeAttribute(val_flowlist_entry, attrs);

  /** validate vlan_priority attribute */
  ValidateVlanPriorityAttribute(val_flowlist_entry, attrs);

  /** validates dst_ipv6 and src_ipv6 attributes and its length*/
  ValidateIPV6Attribute(val_flowlist_entry, attrs);

  /** validates ip_proto attribute */
  ValidateIpProtoAttribute(val_flowlist_entry, attrs);

  ValidateIpDscpAttribute(val_flowlist_entry, attrs);

  /** validates l4 dstport, srxport and endpt attributes */
  ValidateL4PortAttribute(val_flowlist_entry, attrs);

  /** validates icmp_type icmp_code, icmpv6_type, icmpv6_code attributes */
  ValidateICMPAttribute(val_flowlist_entry, attrs);

  return UPLL_RC_SUCCESS;
}

void FlowListEntryMoMgr::ValidateIpProtoAttribute(
    val_flowlist_entry_t *val_flowlist_entry, const uint8_t *attrs) {
  UPLL_FUNC_TRACE;

  if ((val_flowlist_entry->valid[UPLL_IDX_IP_PROTOCOL_FLE] == UNC_VF_VALID)
      || (val_flowlist_entry->valid[UPLL_IDX_IP_PROTOCOL_FLE]
          == UNC_VF_VALID_NO_VALUE)) {
    if (attrs[unc::capa::flowlist_entry::kCapIpProtocol] == 0) {
      UPLL_LOG_DEBUG("ip_proto attr is not supported by ctrlr");
      val_flowlist_entry->valid[UPLL_IDX_IP_PROTOCOL_FLE] =
          UNC_VF_NOT_SUPPORTED;
    }
  }
}

void FlowListEntryMoMgr::ValidateVlanPriorityAttribute(
    val_flowlist_entry_t *val_flowlist_entry, const uint8_t *attrs) {
  UPLL_FUNC_TRACE;

  /** validate vlan_priority attribute */
  if ((val_flowlist_entry->valid[UPLL_IDX_VLAN_PRIORITY_FLE] == UNC_VF_VALID)
      || (val_flowlist_entry->valid[UPLL_IDX_VLAN_PRIORITY_FLE]
          == UNC_VF_VALID_NO_VALUE)) {
    if (attrs[unc::capa::flowlist_entry::kCapVlanPriority] == 0) {
      UPLL_LOG_DEBUG("VlanPriority attr is not supported by ctrlr");
      val_flowlist_entry->valid[UPLL_IDX_VLAN_PRIORITY_FLE] =
          UNC_VF_NOT_SUPPORTED;
    }
  }
}

void FlowListEntryMoMgr::ValidateMacEthTypeAttribute(
    val_flowlist_entry_t *val_flowlist_entry, const uint8_t *attrs) {
  UPLL_FUNC_TRACE;

  if ((val_flowlist_entry->valid[UPLL_IDX_MAC_ETH_TYPE_FLE] == UNC_VF_VALID)
      || (val_flowlist_entry->valid[UPLL_IDX_MAC_ETH_TYPE_FLE]
          == UNC_VF_VALID_NO_VALUE)) {
    if (attrs[unc::capa::flowlist_entry::kCapMacEthType] == 0) {
      UPLL_LOG_DEBUG(" Eth type attr is not supported by ctrlr");
      val_flowlist_entry->valid[UPLL_IDX_MAC_ETH_TYPE_FLE] =
          UNC_VF_NOT_SUPPORTED;
    }
  }
}

void FlowListEntryMoMgr::ValidateIpDscpAttribute(
    val_flowlist_entry_t *val_flowlist_entry, const uint8_t *attrs) {
  UPLL_FUNC_TRACE;

  /** validates DSCP attribute */
  if ((val_flowlist_entry->valid[UPLL_IDX_IP_DSCP_FLE] == UNC_VF_VALID)
      || (val_flowlist_entry->valid[UPLL_IDX_IP_DSCP_FLE]
          == UNC_VF_VALID_NO_VALUE)) {
    if (attrs[unc::capa::flowlist_entry::kCapIpDscp] == 0) {
      UPLL_LOG_DEBUG(" IP_DSCP attr is not supported by ctrlr");
      val_flowlist_entry->valid[UPLL_IDX_IP_DSCP_FLE] = UNC_VF_NOT_SUPPORTED;
    }
  }
}

void FlowListEntryMoMgr::ValidateMacAttribute(
    val_flowlist_entry_t *val_flowlist_entry, const uint8_t *attrs) {
  UPLL_FUNC_TRACE;

  if ((val_flowlist_entry->valid[UPLL_IDX_MAC_DST_FLE] == UNC_VF_VALID)
      || (val_flowlist_entry->valid[UPLL_IDX_MAC_DST_FLE]
          == UNC_VF_VALID_NO_VALUE)) {
    if (attrs[unc::capa::flowlist_entry::kCapMacDst] == 0) {
      UPLL_LOG_DEBUG(" DST_MAC attr is not supported by ctrlr");
      val_flowlist_entry->valid[UPLL_IDX_MAC_DST_FLE] = UNC_VF_NOT_SUPPORTED;
    }
  }

  if ((val_flowlist_entry->valid[UPLL_IDX_MAC_SRC_FLE] == UNC_VF_VALID)
      || (val_flowlist_entry->valid[UPLL_IDX_MAC_SRC_FLE]
          == UNC_VF_VALID_NO_VALUE)) {
    if (attrs[unc::capa::flowlist_entry::kCapMacSrc] == 0) {
      UPLL_LOG_DEBUG(" SRC_MAC attr is not supported by ctrlr");
      val_flowlist_entry->valid[UPLL_IDX_MAC_SRC_FLE] = UNC_VF_NOT_SUPPORTED;
    }
  }
}

void FlowListEntryMoMgr::ValidateIPAttribute(
    val_flowlist_entry_t *val_flowlist_entry, const uint8_t *attrs) {
  UPLL_FUNC_TRACE;

  if ((val_flowlist_entry->valid[UPLL_IDX_DST_IP_FLE] == UNC_VF_VALID)
      || (val_flowlist_entry->valid[UPLL_IDX_DST_IP_FLE]
          == UNC_VF_VALID_NO_VALUE)) {
    if (attrs[unc::capa::flowlist_entry::kCapDstIp] == 0) {
      UPLL_LOG_DEBUG(" DST_IP attr is not supported by ctrlr");
      val_flowlist_entry->valid[UPLL_IDX_DST_IP_FLE] = UNC_VF_NOT_SUPPORTED;
    }
  }

  if ((val_flowlist_entry->valid[UPLL_IDX_DST_IP_PREFIX_FLE] == UNC_VF_VALID)
      || (val_flowlist_entry->valid[UPLL_IDX_DST_IP_PREFIX_FLE]
          == UNC_VF_VALID_NO_VALUE)) {
    if (attrs[unc::capa::flowlist_entry::kCapDstIpPrefix] == 0) {
      UPLL_LOG_DEBUG(" DST_IP_PREFIX attr is not supported by ctrlr");
      val_flowlist_entry->valid[UPLL_IDX_DST_IP_PREFIX_FLE] =
          UNC_VF_NOT_SUPPORTED;
    }
  }

  if ((val_flowlist_entry->valid[UPLL_IDX_SRC_IP_FLE] == UNC_VF_VALID)
      || (val_flowlist_entry->valid[UPLL_IDX_SRC_IP_FLE]
          == UNC_VF_VALID_NO_VALUE)) {
    if (attrs[unc::capa::flowlist_entry::kCapSrcIp] == 0) {
      UPLL_LOG_DEBUG(" SRC_IP attr is not supported by ctrlr");
      val_flowlist_entry->valid[UPLL_IDX_SRC_IP_FLE] = UNC_VF_NOT_SUPPORTED;
    }
  }

  if ((val_flowlist_entry->valid[UPLL_IDX_SRC_IP_PREFIX_FLE] == UNC_VF_VALID)
      || (val_flowlist_entry->valid[UPLL_IDX_SRC_IP_PREFIX_FLE]
          == UNC_VF_VALID_NO_VALUE)) {
    if (attrs[unc::capa::flowlist_entry::kCapSrcIpPrefix] == 0) {
      UPLL_LOG_DEBUG(" SRC_IP_PREFIX attr is not supported by ctrlr");
      val_flowlist_entry->valid[UPLL_IDX_SRC_IP_PREFIX_FLE] =
          UNC_VF_NOT_SUPPORTED;
    }
  }
}

void FlowListEntryMoMgr::ValidateIPV6Attribute(
    val_flowlist_entry_t *val_flowlist_entry, const uint8_t *attrs) {
  UPLL_FUNC_TRACE;

  if ((val_flowlist_entry->valid[UPLL_IDX_DST_IP_V6_FLE] == UNC_VF_VALID)
      || (val_flowlist_entry->valid[UPLL_IDX_DST_IP_V6_FLE]
          == UNC_VF_VALID_NO_VALUE)) {
    if (attrs[unc::capa::flowlist_entry::kCapDstIpV6] == 0) {
      UPLL_LOG_DEBUG(" DST_IP_V6 attr is not supported by ctrlr");
      val_flowlist_entry->valid[UPLL_IDX_DST_IP_V6_FLE] = UNC_VF_NOT_SUPPORTED;
    }
  }

  if ((val_flowlist_entry->valid[UPLL_IDX_DST_IP_V6_PREFIX_FLE] == UNC_VF_VALID)
      || (val_flowlist_entry->valid[UPLL_IDX_DST_IP_V6_PREFIX_FLE]
          == UNC_VF_VALID_NO_VALUE)) {
    if (attrs[unc::capa::flowlist_entry::kCapDstIpV6Prefix] == 0) {
      UPLL_LOG_DEBUG(" DST_IP_V6_PREFIX attr is not supported by ctrlr");
      val_flowlist_entry->valid[UPLL_IDX_DST_IP_V6_PREFIX_FLE] =
          UNC_VF_NOT_SUPPORTED;
    }
  }

  if ((val_flowlist_entry->valid[UPLL_IDX_SRC_IP_V6_FLE] == UNC_VF_VALID)
      || (val_flowlist_entry->valid[UPLL_IDX_SRC_IP_V6_FLE]
          == UNC_VF_VALID_NO_VALUE)) {
    if (attrs[unc::capa::flowlist_entry::kCapSrcIpV6] == 0) {
      UPLL_LOG_DEBUG(" SRC_IP_V6 attr is not supported by ctrlr");
      val_flowlist_entry->valid[UPLL_IDX_SRC_IP_V6_FLE] = UNC_VF_NOT_SUPPORTED;
    }
  }

  if ((val_flowlist_entry->valid[UPLL_IDX_SRC_IP_V6_PREFIX_FLE] == UNC_VF_VALID)
      || (val_flowlist_entry->valid[UPLL_IDX_SRC_IP_V6_PREFIX_FLE]
          == UNC_VF_VALID_NO_VALUE)) {
    if (attrs[unc::capa::flowlist_entry::kCapSrcIpV6Prefix] == 0) {
      UPLL_LOG_DEBUG(" SRC_IP_V6_PREFIX attr is not supported by ctrlr");
      val_flowlist_entry->valid[UPLL_IDX_SRC_IP_V6_PREFIX_FLE] =
          UNC_VF_NOT_SUPPORTED;
    }
  }
}

void FlowListEntryMoMgr::ValidateL4PortAttribute(
    val_flowlist_entry_t *val_flowlist_entry, const uint8_t *attrs) {
  UPLL_FUNC_TRACE;

  if ((val_flowlist_entry->valid[UPLL_IDX_L4_DST_PORT_FLE] == UNC_VF_VALID)
      || (val_flowlist_entry->valid[UPLL_IDX_L4_DST_PORT_FLE]
          == UNC_VF_VALID_NO_VALUE)) {
    if (attrs[unc::capa::flowlist_entry::kCapL4DstPort] == 0) {
      UPLL_LOG_DEBUG(" L4_DST_PORT attr is not supported by ctrlr");
      val_flowlist_entry->valid[UPLL_IDX_L4_DST_PORT_FLE] =
          UNC_VF_NOT_SUPPORTED;
    }
  }

  if ((val_flowlist_entry->valid[UPLL_IDX_L4_DST_PORT_ENDPT_FLE] ==
      UNC_VF_VALID)
      || (val_flowlist_entry->valid[UPLL_IDX_L4_DST_PORT_ENDPT_FLE]
          == UNC_VF_VALID_NO_VALUE)) {
    if (attrs[unc::capa::flowlist_entry::kCapL4DstPortEndpt] == 0) {
      UPLL_LOG_DEBUG(" L4_DST_PORT_ENDPT attr is not supported by ctrlr");
      val_flowlist_entry->valid[UPLL_IDX_L4_DST_PORT_ENDPT_FLE] =
          UNC_VF_NOT_SUPPORTED;
    }
  }

  if ((val_flowlist_entry->valid[UPLL_IDX_L4_SRC_PORT_FLE] == UNC_VF_VALID)
      || (val_flowlist_entry->valid[UPLL_IDX_L4_SRC_PORT_FLE]
          == UNC_VF_VALID_NO_VALUE)) {
    if (attrs[unc::capa::flowlist_entry::kCapL4SrcPort] == 0) {
      UPLL_LOG_DEBUG(" L4_SRC_PORT attr is not supported by ctrlr");
      val_flowlist_entry->valid[UPLL_IDX_L4_SRC_PORT_FLE] =
          UNC_VF_NOT_SUPPORTED;
    }
  }

  if ((val_flowlist_entry->valid[UPLL_IDX_L4_SRC_PORT_ENDPT_FLE] ==
       UNC_VF_VALID)
      || (val_flowlist_entry->valid[UPLL_IDX_L4_SRC_PORT_ENDPT_FLE]
          == UNC_VF_VALID_NO_VALUE)) {
    if (attrs[unc::capa::flowlist_entry::kCapL4SrcPortEndpt] == 0) {
      UPLL_LOG_DEBUG(" L4_SRC_PORT_ENDPT attr is not supported by ctrlr");
      val_flowlist_entry->valid[UPLL_IDX_L4_SRC_PORT_ENDPT_FLE] =
          UNC_VF_NOT_SUPPORTED;
    }
  }
}

void FlowListEntryMoMgr::ValidateICMPAttribute(
    val_flowlist_entry_t *val_flowlist_entry, const uint8_t *attrs) {
  UPLL_FUNC_TRACE;

  if ((val_flowlist_entry->valid[UPLL_IDX_ICMP_TYPE_FLE] == UNC_VF_VALID)
      || (val_flowlist_entry->valid[UPLL_IDX_ICMP_TYPE_FLE]
          == UNC_VF_VALID_NO_VALUE)) {
    if (attrs[unc::capa::flowlist_entry::kCapIcmpType] == 0) {
      UPLL_LOG_DEBUG(" ICMP_TYPE attr is not supported by ctrlr");
      val_flowlist_entry->valid[UPLL_IDX_ICMP_TYPE_FLE] = UNC_VF_NOT_SUPPORTED;
    }
  }

  if ((val_flowlist_entry->valid[UPLL_IDX_ICMP_CODE_FLE] == UNC_VF_VALID)
      || (val_flowlist_entry->valid[UPLL_IDX_ICMP_CODE_FLE]
          == UNC_VF_VALID_NO_VALUE)) {
    if (attrs[unc::capa::flowlist_entry::kCapIcmpCode] == 0) {
      UPLL_LOG_DEBUG(" ICMP CODE attr is not supported by ctrlr");
      val_flowlist_entry->valid[UPLL_IDX_ICMP_CODE_FLE] = UNC_VF_NOT_SUPPORTED;
    }
  }

  if ((val_flowlist_entry->valid[UPLL_IDX_ICMP_V6_TYPE_FLE] == UNC_VF_VALID)
      || (val_flowlist_entry->valid[UPLL_IDX_ICMP_V6_TYPE_FLE]
          == UNC_VF_VALID_NO_VALUE)) {
    if (attrs[unc::capa::flowlist_entry::kCapIcmpV6Type] == 0) {
      UPLL_LOG_DEBUG(" ICMP_V6_TYPE attr is not supported by ctrlr");
      val_flowlist_entry->valid[UPLL_IDX_ICMP_V6_TYPE_FLE] =
          UNC_VF_NOT_SUPPORTED;
    }
  }

  if ((val_flowlist_entry->valid[UPLL_IDX_ICMP_V6_CODE_FLE] == UNC_VF_VALID)
      || (val_flowlist_entry->valid[UPLL_IDX_ICMP_V6_CODE_FLE]
          == UNC_VF_VALID_NO_VALUE)) {
    if (attrs[unc::capa::flowlist_entry::kCapIcmpV6Code] == 0) {
      UPLL_LOG_DEBUG(" ICMP_V6 attr is not supported by ctrlr");
      val_flowlist_entry->valid[UPLL_IDX_ICMP_V6_CODE_FLE] =
          UNC_VF_NOT_SUPPORTED;
    }
  }
}

upll_rc_t FlowListEntryMoMgr::ValidateMessage(IpcReqRespHeader *req,
                                              ConfigKeyVal *key) {
  UPLL_FUNC_TRACE;

  if ((NULL == req) || (NULL == key)) {
    UPLL_LOG_DEBUG("IpcReqRespHeader/ConfigKeyval is NULL");
    return UPLL_RC_ERR_BAD_REQUEST;
  }

  upll_rc_t rt_code = UPLL_RC_SUCCESS;

  if (UNC_KT_FLOWLIST_ENTRY != key->get_key_type()) {
    UPLL_LOG_DEBUG(" Invalid keytype(%d)", key->get_key_type());
    return UPLL_RC_ERR_BAD_REQUEST;
  }

  if (req->option1 != UNC_OPT1_NORMAL) {
    UPLL_LOG_DEBUG(" invalid option1(%d)", req->option1);
    return UPLL_RC_ERR_INVALID_OPTION1;
  }
  if (req->option2 != UNC_OPT2_NONE) {
    UPLL_LOG_DEBUG(" invalid option2(%d)", req->option2);
    return UPLL_RC_ERR_INVALID_OPTION2;
  }

  if (UPLL_RC_SUCCESS !=
      (rt_code = ValidateFlowlistEntryKey(key, req->operation))) {
    UPLL_LOG_DEBUG("KT_FLOWLIST_ENTRY key structure syntax "
                   "validation failed: Err code-%d",
                   rt_code);
    return rt_code;
  }

  if (UPLL_RC_SUCCESS != (rt_code = ValidateFlowlistEntryVal(
              key, req->operation, req->datatype))) {
    UPLL_LOG_DEBUG("KT_FLOWLIST_ENTRY val structure syntax "
                   "validation failed: Err code-%d",
                   rt_code);
  }
  return rt_code;
}


upll_rc_t FlowListEntryMoMgr::ValidateFlowlistEntryKey(
    ConfigKeyVal *key,
    unc_keytype_operation_t op) {
  UPLL_FUNC_TRACE;
  upll_rc_t rt_code = UPLL_RC_SUCCESS;

  /** Validate keyStruct fields*/
  if (key->get_st_num() != IpctSt::kIpcStKeyFlowlistEntry) {
    UPLL_LOG_DEBUG("Invalid key structure received. struct num - %d",
                   key->get_st_num());
    return UPLL_RC_ERR_BAD_REQUEST;
  }

  /** Read key structure */
  key_flowlist_entry_t *key_flowlist_entry =
      reinterpret_cast<key_flowlist_entry_t *>(key->get_key());
  if (NULL == key_flowlist_entry) {
    UPLL_LOG_DEBUG("KT_FLOWLIST_ENTRY Key structure is empty!!");
    return UPLL_RC_ERR_BAD_REQUEST;
  }

  rt_code = ValidateKey(
      reinterpret_cast<char *>(key_flowlist_entry->flowlist_key.flowlist_name),
      kMinLenFlowListName,
      kMaxLenFlowListName);

  if (UPLL_RC_SUCCESS != rt_code) {
    UPLL_LOG_DEBUG("FlowlistName(%s) syntax validation failed: err code - %d",
                   key_flowlist_entry->flowlist_key.flowlist_name, rt_code);
    return rt_code;
  }

  if ((op != UNC_OP_READ_SIBLING_COUNT) &&
      (op != UNC_OP_READ_SIBLING_BEGIN)) {
    /** Validate sequence Number*/
    if (!ValidateNumericRange(key_flowlist_entry->sequence_num,
                              kMinFlowFilterSeqNum,
                              kMaxFlowFilterSeqNum, true, true)) {
      UPLL_LOG_DEBUG("SeqNum(%d) syntax validation failed: err code - %d",
                     key_flowlist_entry->sequence_num, rt_code);
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  } else {
    if (key_flowlist_entry->sequence_num) {
      // reset the sequence number
      // for sibling read and count
      // operation
      key_flowlist_entry->sequence_num = 0;
    }
  }

  UPLL_LOG_DEBUG("ValidateFlowlistEntryKey validation is success");
  return UPLL_RC_SUCCESS;
}

upll_rc_t FlowListEntryMoMgr::ValidateFlowlistEntryVal(ConfigKeyVal *key,
                                                       uint32_t operation,
                                                       uint32_t datatype) {
  UPLL_FUNC_TRACE;
  upll_rc_t rt_code = UPLL_RC_ERR_GENERIC;

  if (!key->get_cfg_val()) {
    if ((operation == UNC_OP_DELETE) || (operation == UNC_OP_READ) ||
        (operation == UNC_OP_READ_SIBLING) ||
        (operation == UNC_OP_READ_SIBLING_BEGIN) ||
        (operation == UNC_OP_READ_SIBLING_COUNT)) {
      UPLL_LOG_DEBUG("val stucture is optional");
      return UPLL_RC_SUCCESS;
    } else {
      UPLL_LOG_DEBUG(" val structure is mandatory");
      return UPLL_RC_ERR_BAD_REQUEST;
    }
  }
  if ((key->get_cfg_val()->get_st_num() != IpctSt::kIpcStValFlowlistEntry))  {
    UPLL_LOG_DEBUG("Invalid val structure received. struct num - %d",
                   (key->get_cfg_val())->get_st_num());
    return UPLL_RC_ERR_BAD_REQUEST;
  }

  val_flowlist_entry_t *val_flowlist_entry =
      reinterpret_cast<val_flowlist_entry_t *>(key->get_cfg_val()->get_val());

  if (val_flowlist_entry == NULL) {
    UPLL_LOG_DEBUG("KT_FLOWLIST_ENTRY val structure is null");
    return UPLL_RC_ERR_BAD_REQUEST;
  }
  /** Validate Value struct fields */
  /** Validate mac address */
  if ((rt_code = ValidateFlowlistMacAddr(val_flowlist_entry, operation))
      != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("syntax validation failed for mac address");
    return rt_code;
  }

  if ((rt_code = ValidateEthType(val_flowlist_entry, operation))
      != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("syntax validation failed for eth type : err code (%d)",
                   rt_code);
    return rt_code;
  }

  if ((rt_code = ValidateVlanPriority(val_flowlist_entry, operation))
      != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Vlan Priority syntax validation failed: err code (%d)",
                   rt_code);
    return rt_code;
  }

  if ((rt_code = ValidateIPProto(val_flowlist_entry, operation))
      != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("IP protocol syntax validation failed: err code (%d)",
                   rt_code);
    return rt_code;
  }

  if ((rt_code = ValidateDscp(val_flowlist_entry, operation))
      != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("DSCP syntax validation failed: err code (%d)",
                   rt_code);
    return rt_code;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t FlowListEntryMoMgr::ValidateFlowlistEntryVal(IpcReqRespHeader *req,
                                                 ConfigKeyVal *key,
                                                 DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t rt_code = UPLL_RC_SUCCESS;
  key_flowlist_entry_t *key_flowlist_entry =
      reinterpret_cast<key_flowlist_entry_t *>(key->get_key());

  val_flowlist_entry_t *val_flowlist_entry =
      reinterpret_cast<val_flowlist_entry_t *>(key->get_cfg_val()->get_val());

  /** Read val_flowlist struct from DB to check ip_type */

  key_flowlist_t *key_flowlist =
      reinterpret_cast<key_flowlist_t *>
      (ConfigKeyVal::Malloc(sizeof(key_flowlist_t)));

  /* copy flowlistname to KT_FLOWLIST key strcuture */
  uuu::upll_strncpy(key_flowlist->flowlist_name,
                    key_flowlist_entry->flowlist_key.flowlist_name,
                    (kMaxLenFlowListName+1));

  ConfigKeyVal *okey = new ConfigKeyVal(UNC_KT_FLOWLIST,
                                        IpctSt::kIpcStKeyFlowlist,
                                        key_flowlist);

  DbSubOp readop = { kOpReadSingle, kOpMatchNone, kOpInOutNone };

  MoMgrImpl *mgrflowlist = reinterpret_cast<MoMgrImpl *>(
      const_cast<MoManager *>(GetMoManager(UNC_KT_FLOWLIST)));

  if (mgrflowlist) {
    if (UPLL_RC_SUCCESS != (rt_code = mgrflowlist->ReadConfigDB(okey,
                           (upll_keytype_datatype_t) req->datatype,
                           UNC_OP_READ, readop, dmi, MAINTBL))) {
      UPLL_LOG_DEBUG("Error in read :Err code=%d", rt_code);
      delete okey;
      return ((UPLL_RC_ERR_NO_SUCH_INSTANCE == rt_code) ?
              UPLL_RC_ERR_PARENT_DOES_NOT_EXIST : rt_code);
    }
  }

  /** Get val_flowlist structure to read ip_type*/
  val_flowlist_t *val_flowlist =
      reinterpret_cast<val_flowlist_t *>(GetVal(okey));

  if (NULL == val_flowlist) {
    UPLL_LOG_DEBUG("val_flowlist is NULL, can't access flowlist IP type");
    delete okey;
    return UPLL_RC_ERR_CFG_SYNTAX;
  }

  if ((rt_code = ValidateIPAddress(val_flowlist_entry, val_flowlist->ip_type,
                                   req->operation, false)) != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("syntax check failed for dst IP address. err code (%d)",
                   rt_code);
    DELETE_IF_NOT_NULL(okey);
    return rt_code;
  }

  if ((rt_code = ValidateIPAddress(val_flowlist_entry, val_flowlist->ip_type,
                                   req->operation, true)) != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG(" syntax check failed for src IPaddress. err code (%d)",
                   rt_code);

    DELETE_IF_NOT_NULL(okey);
    return rt_code;
  }

  /** TCP/UDP port number is filled for the flowlist entry which does not
    contain icmp_type, icmp_code, icmpv6_type, and/or icmpv6_code */

  /** Read val_flowlist struct from DB to check ip_type */
  key_flowlist_entry_t *tmp_key_fle = NULL;
  ConfigKeyVal *fle_okey = NULL;
  val_flowlist_entry_t *tmp_val_fle = NULL;

  if (req->operation == UNC_OP_UPDATE) {
    tmp_key_fle = reinterpret_cast<key_flowlist_entry_t *>
        (ConfigKeyVal::Malloc(sizeof(key_flowlist_entry_t)));

    /* copy flowlistname to KT_FLOWLIST key strcuture */
    uuu::upll_strncpy(tmp_key_fle->flowlist_key.flowlist_name,
                      key_flowlist_entry->flowlist_key.flowlist_name,
                      (kMaxLenFlowListName+1));
    tmp_key_fle->sequence_num = key_flowlist_entry->sequence_num;

    fle_okey = new ConfigKeyVal(UNC_KT_FLOWLIST_ENTRY,
                                IpctSt::kIpcStKeyFlowlistEntry,
                                tmp_key_fle);

    DbSubOp readop = { kOpReadSingle, kOpMatchNone, kOpInOutNone };

    if (UPLL_RC_SUCCESS != (rt_code = ReadConfigDB(
                fle_okey, req->datatype,
                UNC_OP_READ, readop, dmi, MAINTBL))) {
      UPLL_LOG_DEBUG("Error in read :Err code=%d", rt_code);
      delete fle_okey;
      delete okey;
      return rt_code;
    }

    /** Get val_flowlist structure to read ip_type*/
    tmp_val_fle = reinterpret_cast<val_flowlist_entry_t *>(GetVal(fle_okey));
    if (NULL == tmp_val_fle) {
      UPLL_LOG_DEBUG("val_flowlist_entry is NULL");
      delete fle_okey;
      delete okey;
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  }

  /** validate l4dst_port and l4_dst_portendpt */
  if ((rt_code = ValidateL4Port(val_flowlist_entry, tmp_val_fle,
                 req->operation, false)) != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("dst L4 Port syntax validation failed: err code (%d)",
                   rt_code);
    delete fle_okey;
    delete okey;
    return rt_code;
  }

  if ((rt_code = ValidateL4Port(val_flowlist_entry, tmp_val_fle,
                 req->operation, true)) != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("src L4 Port syntax validation failed: err code (%d)",
                   rt_code);
    delete fle_okey;
    delete okey;
    return rt_code;
  }

  /** validate ICMP code and type
  */

  if ((rt_code = ValidateIcmp(val_flowlist_entry, tmp_val_fle,
                 val_flowlist->ip_type, req->operation)) != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("ICMP syntax validation failed: err code (%d)",
                   rt_code);
    delete fle_okey;
    delete okey;
    return rt_code;
  }
  delete fle_okey;
  delete okey;
  return rt_code;
}

upll_rc_t FlowListEntryMoMgr::ValidateFlowlistMacAddr(
    val_flowlist_entry_t *val_flowlist_entry, uint32_t operation) {
  UPLL_FUNC_TRACE;
  /** no specific check for mac_addr since the acceptable range is
    0x000000000000 to 0xffffffffffff */

  if ((operation == UNC_OP_UPDATE) || (operation == UNC_OP_CREATE)) {
    if (val_flowlist_entry->valid[UPLL_IDX_MAC_DST_FLE] ==
        UNC_VF_VALID_NO_VALUE) {
      UPLL_LOG_DEBUG("Reset mac_dst");
      memset(val_flowlist_entry->mac_dst, 0, 6);
    }

    if (val_flowlist_entry->valid[UPLL_IDX_MAC_SRC_FLE]
        == UNC_VF_VALID_NO_VALUE) {
      UPLL_LOG_DEBUG("Reset mac_src");
      memset(val_flowlist_entry->mac_src, 0, 6);
    }
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t FlowListEntryMoMgr::ValidateEthType(
    val_flowlist_entry_t *val_flowlist_entry, uint32_t operation) {
  UPLL_FUNC_TRACE;

  /** Validate ether type of the ethernet frame */
  if ((operation == UNC_OP_UPDATE) || (operation == UNC_OP_CREATE)) {
    if (val_flowlist_entry->valid[UPLL_IDX_MAC_ETH_TYPE_FLE] ==
       UNC_VF_VALID_NO_VALUE) {
      UPLL_LOG_DEBUG("Reset mac_eth_type");
      val_flowlist_entry->mac_eth_type = 0;
    }
  }
  if (val_flowlist_entry->valid[UPLL_IDX_MAC_ETH_TYPE_FLE] == UNC_VF_VALID) {
    if (!ValidateNumericRange(val_flowlist_entry->mac_eth_type, kMinEthType,
                              kMaxEthType, true, true)) {
      UPLL_LOG_DEBUG("Ether type(%d) syntax validation failed",
                     val_flowlist_entry->mac_eth_type);
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t FlowListEntryMoMgr::ValidateIPAddress(
    val_flowlist_entry_t *val_flowlist_entry,
    uint8_t ip_type,
    unc_keytype_operation_t operation, bool is_src_ip) {
  UPLL_FUNC_TRACE;
  enum val_flowlist_entry_index  fle_idx_ip, fle_idx_ip_prefix;
  uint8_t prefix_len = 0;
  uint8_t min_prefix_len = ((ip_type == UPLL_FLOWLIST_TYPE_IP) ?
                            kMinIpv4Prefix : kMinIpv6Prefix);
  uint8_t max_prefix_len = ((ip_type == UPLL_FLOWLIST_TYPE_IP) ?
                            kMaxIpv4Prefix : kMaxIpv6Prefix);

  /* check if the flowlist ip type and the ip address configured
   * is the same else return syntax error
   */
  if (ip_type == UPLL_FLOWLIST_TYPE_IP) {
    if ((val_flowlist_entry->valid[UPLL_IDX_SRC_IP_V6_FLE] == UNC_VF_VALID) ||
        (val_flowlist_entry->valid[UPLL_IDX_SRC_IP_V6_FLE] ==
         UNC_VF_VALID_NO_VALUE) ||
        (val_flowlist_entry->valid[UPLL_IDX_DST_IP_V6_FLE] == UNC_VF_VALID) ||
        (val_flowlist_entry->valid[UPLL_IDX_DST_IP_V6_FLE] ==
         UNC_VF_VALID_NO_VALUE)) {
      UPLL_LOG_DEBUG("Invalid IP adress type set");
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  } else {
    if ((val_flowlist_entry->valid[UPLL_IDX_SRC_IP_FLE] == UNC_VF_VALID) ||
        (val_flowlist_entry->valid[UPLL_IDX_SRC_IP_FLE] ==
         UNC_VF_VALID_NO_VALUE) ||
        (val_flowlist_entry->valid[UPLL_IDX_DST_IP_FLE] == UNC_VF_VALID) ||
        (val_flowlist_entry->valid[UPLL_IDX_DST_IP_FLE] ==
         UNC_VF_VALID_NO_VALUE)) {
      UPLL_LOG_DEBUG("Invalid IP adress type set");
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  }
  /** ip and ip prefix should be configured simultaneously,
   *       so validate both */
  if (is_src_ip) {
    if (ip_type == UPLL_FLOWLIST_TYPE_IP) {
      fle_idx_ip = UPLL_IDX_SRC_IP_FLE;
      fle_idx_ip_prefix = UPLL_IDX_SRC_IP_PREFIX_FLE;
    } else {
      fle_idx_ip = UPLL_IDX_SRC_IP_V6_FLE;
      fle_idx_ip_prefix = UPLL_IDX_SRC_IP_V6_PREFIX_FLE;
    }
  } else {
    if (ip_type == UPLL_FLOWLIST_TYPE_IP) {
      fle_idx_ip = UPLL_IDX_DST_IP_FLE;
      fle_idx_ip_prefix = UPLL_IDX_DST_IP_PREFIX_FLE;
    } else {
      fle_idx_ip = UPLL_IDX_DST_IP_V6_FLE;
      fle_idx_ip_prefix = UPLL_IDX_DST_IP_V6_PREFIX_FLE;
    }
  }

  if ((val_flowlist_entry->valid[fle_idx_ip] == UNC_VF_INVALID) &&
      (val_flowlist_entry->valid[fle_idx_ip_prefix] == UNC_VF_INVALID)) {
    UPLL_LOG_DEBUG(" Not configured. no validation required");
    return UPLL_RC_SUCCESS;
  }

  if ((operation == UNC_OP_UPDATE) || (operation == UNC_OP_CREATE)) {
    if (val_flowlist_entry->valid[fle_idx_ip] == UNC_VF_VALID_NO_VALUE) {
      if (val_flowlist_entry->valid[fle_idx_ip_prefix] ==
          UNC_VF_VALID_NO_VALUE) {
        if (is_src_ip) {
          if (ip_type == UPLL_FLOWLIST_TYPE_IP) {
            val_flowlist_entry->src_ip.s_addr = 0;
            val_flowlist_entry->src_ip_prefixlen = 0;
          } else {
            memset(val_flowlist_entry->src_ipv6.s6_addr, 0, 16);
            val_flowlist_entry->src_ipv6_prefixlen = 0;
          }
        } else {
          if (ip_type == UPLL_FLOWLIST_TYPE_IP) {
            val_flowlist_entry->dst_ip.s_addr = 0;
            val_flowlist_entry->dst_ip_prefixlen = 0;
          } else {
            memset(val_flowlist_entry->dst_ipv6.s6_addr, 0, 16);
            val_flowlist_entry->dst_ipv6_prefixlen = 0;
          }
        }
        return UPLL_RC_SUCCESS;
      } else {
        UPLL_LOG_DEBUG("ip address and prefix not set simultaneously");
        return UPLL_RC_ERR_CFG_SYNTAX;
      }
    }
  }

  if ((val_flowlist_entry->valid[fle_idx_ip] == UNC_VF_VALID)  &&
      (val_flowlist_entry->valid[fle_idx_ip_prefix] == UNC_VF_VALID)) {
    if (is_src_ip) {
      prefix_len = ((ip_type == UPLL_FLOWLIST_TYPE_IP) ?
                    val_flowlist_entry->src_ip_prefixlen :
                    val_flowlist_entry->src_ipv6_prefixlen);
    } else {
      prefix_len = ((ip_type == UPLL_FLOWLIST_TYPE_IP) ?
                    val_flowlist_entry->dst_ip_prefixlen :
                    val_flowlist_entry->dst_ipv6_prefixlen);
    }
    // No specific Ip address validation is required
    // validate IP prefix validation
    if (!ValidateNumericRange(prefix_len, min_prefix_len, max_prefix_len,
                              true, true)) {
      UPLL_LOG_DEBUG("Ip Prefix(%d) validation failed", prefix_len);
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  } else {
    UPLL_LOG_DEBUG("ip address and prefix not set simultaneously");
    return UPLL_RC_ERR_CFG_SYNTAX;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t FlowListEntryMoMgr::ValidateVlanPriority(
    val_flowlist_entry_t *val_flowlist_entry, uint32_t operation) {
  UPLL_FUNC_TRACE;

  if (((operation == UNC_OP_UPDATE) || (operation == UNC_OP_CREATE)) &&
      (val_flowlist_entry->valid[UPLL_IDX_VLAN_PRIORITY_FLE]
       == UNC_VF_VALID_NO_VALUE)) {
    UPLL_LOG_DEBUG("Reset vlan_priority ");
    val_flowlist_entry->vlan_priority = 0;
    return UPLL_RC_SUCCESS;
  }

  /* check vlan priority field is filled */
  if (val_flowlist_entry->valid[UPLL_IDX_VLAN_PRIORITY_FLE] == UNC_VF_VALID) {
    if (!ValidateNumericRange(val_flowlist_entry->vlan_priority,
                              kMinVlanPriority, kMaxVlanPriority, true, true)) {
      UPLL_LOG_DEBUG("Vlan Priority(%d) syntax validation failed",
                     val_flowlist_entry->vlan_priority);
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t FlowListEntryMoMgr::ValidateIPProto(
    val_flowlist_entry_t *val_flowlist_entry, uint32_t operation) {
  UPLL_FUNC_TRACE;

  if (((operation == UNC_OP_UPDATE) || (operation == UNC_OP_CREATE)) &&
      (val_flowlist_entry->valid[UPLL_IDX_IP_PROTOCOL_FLE]
          == UNC_VF_VALID_NO_VALUE)) {
    UPLL_LOG_DEBUG("Reset ip_proto");
    val_flowlist_entry->ip_proto = 0;
    return UPLL_RC_SUCCESS;
  }
  /** check IP Protocol Number is filled */
  if (val_flowlist_entry->valid[UPLL_IDX_IP_PROTOCOL_FLE] == UNC_VF_VALID) {
    if (!ValidateNumericRange(val_flowlist_entry->ip_proto, kMinIPProto,
                              kMaxIPProto, true, true)) {
      UPLL_LOG_DEBUG("IP protocol number(%d) syntax validation failed",
                     val_flowlist_entry->ip_proto);
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t FlowListEntryMoMgr::ValidateDscp(
    val_flowlist_entry_t *val_flowlist_entry, uint32_t operation) {
  UPLL_FUNC_TRACE;

  if (((operation == UNC_OP_UPDATE) || (operation == UNC_OP_CREATE)) &&
      (val_flowlist_entry->valid[UPLL_IDX_IP_DSCP_FLE]
          == UNC_VF_VALID_NO_VALUE)) {
    UPLL_LOG_DEBUG("Reset ip_dscp");
    val_flowlist_entry->ip_dscp = 0;
    return UPLL_RC_SUCCESS;
  }
  /** check DSCP value is filled */
  if (val_flowlist_entry->valid[UPLL_IDX_IP_DSCP_FLE] == UNC_VF_VALID) {
    if (!ValidateNumericRange(val_flowlist_entry->ip_dscp, kMinIPDscp,
                              kMaxIPDscp, true, true)) {
      UPLL_LOG_DEBUG(" DSCP number(%d) syntax validation failed",
                     val_flowlist_entry->ip_dscp);
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t FlowListEntryMoMgr::ValidateL4Port(
    val_flowlist_entry_t *val_flowlist_entry,
    val_flowlist_entry_t *db_val_fle,
    uint32_t operation, bool is_src_port) {
  UPLL_FUNC_TRACE;
  enum val_flowlist_entry_index  fle_idx_port, fle_idx_end_port;
  uint16_t *l4_port;
  uint16_t *l4_end_port;

  if ((val_flowlist_entry->valid[UPLL_IDX_L4_DST_PORT_FLE] == UNC_VF_VALID) ||
      (val_flowlist_entry->valid[UPLL_IDX_L4_SRC_PORT_FLE] == UNC_VF_VALID) ||
      (val_flowlist_entry->valid[UPLL_IDX_L4_DST_PORT_ENDPT_FLE]
       == UNC_VF_VALID) ||
      (val_flowlist_entry->valid[UPLL_IDX_L4_SRC_PORT_ENDPT_FLE]
       == UNC_VF_VALID) ||
       ((operation == UNC_OP_UPDATE) &&
        ((db_val_fle->valid[UPLL_IDX_L4_DST_PORT_FLE] == UNC_VF_VALID) ||
        (db_val_fle->valid[UPLL_IDX_L4_SRC_PORT_FLE] == UNC_VF_VALID) ||
        (db_val_fle->valid[UPLL_IDX_L4_DST_PORT_ENDPT_FLE] ==
         UNC_VF_VALID) ||
        (db_val_fle->valid[UPLL_IDX_L4_SRC_PORT_ENDPT_FLE] ==
          UNC_VF_VALID)))) {
    if ((val_flowlist_entry->valid[UPLL_IDX_ICMP_TYPE_FLE] == UNC_VF_VALID) ||
        (val_flowlist_entry->valid[UPLL_IDX_ICMP_CODE_FLE] == UNC_VF_VALID) ||
        (val_flowlist_entry->valid[UPLL_IDX_ICMP_V6_TYPE_FLE] ==
         UNC_VF_VALID) ||
        (val_flowlist_entry->valid[UPLL_IDX_ICMP_V6_CODE_FLE] ==
         UNC_VF_VALID)) {
      UPLL_LOG_DEBUG("ICMP value and L4 ports configured simultaneously");
      return UPLL_RC_ERR_CFG_SYNTAX;
    } else if (operation == UNC_OP_UPDATE) {
      if ((db_val_fle->valid[UPLL_IDX_ICMP_TYPE_FLE] == UNC_VF_VALID) ||
          (db_val_fle->valid[UPLL_IDX_ICMP_CODE_FLE] == UNC_VF_VALID) ||
          (db_val_fle->valid[UPLL_IDX_ICMP_V6_TYPE_FLE] == UNC_VF_VALID) ||
          (db_val_fle->valid[UPLL_IDX_ICMP_V6_CODE_FLE] == UNC_VF_VALID)) {
        UPLL_LOG_DEBUG("ICMP value and L4 ports configured simultaneously");
        return UPLL_RC_ERR_CFG_SYNTAX;
      }
    }
  }
  if (is_src_port) {
    fle_idx_port = UPLL_IDX_L4_SRC_PORT_FLE;
    fle_idx_end_port = UPLL_IDX_L4_SRC_PORT_ENDPT_FLE;
    l4_port = &val_flowlist_entry->l4_src_port;
    l4_end_port = &val_flowlist_entry->l4_src_port_endpt;
  } else {
    fle_idx_port = UPLL_IDX_L4_DST_PORT_FLE;
    fle_idx_end_port = UPLL_IDX_L4_DST_PORT_ENDPT_FLE;
    l4_port = &val_flowlist_entry->l4_dst_port;
    l4_end_port = &val_flowlist_entry->l4_dst_port_endpt;
  }

  if ((operation == UNC_OP_UPDATE) || (operation == UNC_OP_CREATE)) {
    if (val_flowlist_entry->valid[fle_idx_port]
        == UNC_VF_VALID_NO_VALUE) {
      UPLL_LOG_DEBUG(" Reset l4_port");
      // when dst_port/src_port is set as no value then
      // reset dst_port_endpt/src_port_endpt also
      *l4_port = 0;
      *l4_end_port = 0;
      val_flowlist_entry->valid[fle_idx_end_port] = UNC_VF_VALID_NO_VALUE;
      return UPLL_RC_SUCCESS;
    }
    if (val_flowlist_entry->valid[fle_idx_end_port]
        == UNC_VF_VALID_NO_VALUE) {
      UPLL_LOG_DEBUG(" Reset l4_port_endpt");
      *l4_end_port = 0;
      return UPLL_RC_SUCCESS;
    }
  }
  /** validate End point TCP/UDP destination port number */
  if (val_flowlist_entry->valid[fle_idx_port] == UNC_VF_VALID) {
    if (!ValidateNumericRange(*l4_port, kMinL4Port,
                              kMaxL4Port, true, true)) {
      UPLL_LOG_DEBUG("TCP/UDP port (%d) syntax validation failed ", *l4_port);
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  }
  if (val_flowlist_entry->valid[fle_idx_end_port] ==
      UNC_VF_VALID) {
    /*EndPoint Port Must be larger than the l4_dst_port value*/
    if (!ValidateNumericRange(*l4_end_port,
                              *l4_port,
                              kMaxL4Port, false, true)) {
      UPLL_LOG_DEBUG("Endpoint TCP/UDP port(%d) syntax validation failed",
                     *l4_end_port);
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t FlowListEntryMoMgr::ValidateIcmp(
    val_flowlist_entry_t *val_flowlist_entry,
    val_flowlist_entry_t *db_val_fle,
    uint8_t ip_type, uint32_t operation) {
  UPLL_FUNC_TRACE;
  enum val_flowlist_entry_index  fle_idx_code, fle_idx_type;
  uint8_t *icmp_code, *icmp_type;
  /* check if the flowlist ip type and the ICMP configured
   * is the same else return syntax error
   */
  if (ip_type == UPLL_FLOWLIST_TYPE_IP) {
    if ((val_flowlist_entry->valid[UPLL_IDX_ICMP_V6_TYPE_FLE] == UNC_VF_VALID)
        || (val_flowlist_entry->valid[UPLL_IDX_ICMP_V6_TYPE_FLE] ==
            UNC_VF_VALID_NO_VALUE) ||
        (val_flowlist_entry->valid[UPLL_IDX_ICMP_V6_CODE_FLE] == UNC_VF_VALID)
        || (val_flowlist_entry->valid[UPLL_IDX_ICMP_V6_CODE_FLE] ==
            UNC_VF_VALID_NO_VALUE)) {
      UPLL_LOG_DEBUG("Invalid ICMP set for flowlist type");
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  } else {
    if ((val_flowlist_entry->valid[UPLL_IDX_ICMP_TYPE_FLE] == UNC_VF_VALID) ||
        (val_flowlist_entry->valid[UPLL_IDX_ICMP_TYPE_FLE] ==
         UNC_VF_VALID_NO_VALUE) ||
        (val_flowlist_entry->valid[UPLL_IDX_ICMP_CODE_FLE] == UNC_VF_VALID) ||
        (val_flowlist_entry->valid[UPLL_IDX_ICMP_CODE_FLE] ==
         UNC_VF_VALID_NO_VALUE)) {
      UPLL_LOG_DEBUG("Invalid IP adress type set");
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  }
  if (ip_type == UPLL_FLOWLIST_TYPE_IP) {
    fle_idx_type = UPLL_IDX_ICMP_TYPE_FLE;
    fle_idx_code = UPLL_IDX_ICMP_CODE_FLE;
    icmp_type = &val_flowlist_entry->icmp_type;
    icmp_code = &val_flowlist_entry->icmp_code;
  } else {
    fle_idx_type = UPLL_IDX_ICMP_V6_TYPE_FLE;
    fle_idx_code = UPLL_IDX_ICMP_V6_CODE_FLE;
    icmp_type = &val_flowlist_entry->icmpv6_type;
    icmp_code = &val_flowlist_entry->icmpv6_code;
  }

  /** Validate ICMP type */
  if (val_flowlist_entry->valid[fle_idx_type] == UNC_VF_VALID) {
    /** validate ICMP Type */
    if (!ValidateNumericRange(*icmp_type, kMinIcmpValue,
                              kMaxIcmpValue, true, true)) {
      UPLL_LOG_DEBUG("ICMP Type syntax validation failed ");
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  }
  /** validate ICMP Code*/
  if (val_flowlist_entry->valid[fle_idx_code] == UNC_VF_VALID) {
    /** validate ICMP Code */
    if (!ValidateNumericRange(*icmp_code, kMinIcmpValue,
                              kMaxIcmpValue, true, true)) {
      UPLL_LOG_DEBUG("ICMP Code syntax validation failed");
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  }
  if ((operation == UNC_OP_UPDATE) || (operation == UNC_OP_CREATE)) {
    if (val_flowlist_entry->valid[fle_idx_type]
        == UNC_VF_VALID_NO_VALUE) {
      *icmp_type = 0;
    }
    if (val_flowlist_entry->valid[fle_idx_code]
        == UNC_VF_VALID_NO_VALUE) {
      *icmp_code = 0;
    }
  }

  return UPLL_RC_SUCCESS;
}

upll_rc_t FlowListEntryMoMgr::ValidateAttribute(ConfigKeyVal *ikey,
                                                DalDmlIntf *dmi,
                                                IpcReqRespHeader *req) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  if (ikey->get_key_type() != UNC_KT_FLOWLIST_ENTRY) {
    UPLL_LOG_DEBUG(" ValidationAttribute Failed.");
    return UPLL_RC_ERR_GENERIC;
  }
  result_code = IsFlowListMatched(ikey, dmi, req);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("IsFlowListMatched failed in fle %d", result_code);
    return result_code;
  }
  UPLL_LOG_DEBUG("ValidateAttribute Successfull.");
  return result_code;
}

upll_rc_t FlowListEntryMoMgr::IsFlowListMatched(ConfigKeyVal *ikey,
                                                DalDmlIntf *dmi,
                                                IpcReqRespHeader *req) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  if (UNC_OP_UPDATE == req->operation ||
      UNC_OP_CREATE == req->operation) {
    key_flowlist_entry_t *key_fle = reinterpret_cast
      <key_flowlist_entry_t *>(ikey->get_key());
    PolicingProfileEntryMoMgr *mgr =
      reinterpret_cast<PolicingProfileEntryMoMgr *>(const_cast<MoManager *>
      (GetMoManager(UNC_KT_POLICING_PROFILE_ENTRY)));
    if (NULL == mgr) {
      return UPLL_RC_ERR_GENERIC;
    }
    result_code = mgr->IsFlowListMatched(reinterpret_cast<const char *>
      (key_fle->flowlist_key.flowlist_name), req->datatype, dmi);
    if (UPLL_RC_SUCCESS != result_code) {
      if (result_code == UPLL_RC_ERR_INSTANCE_EXISTS)
        result_code = UPLL_RC_ERR_CFG_SEMANTIC;
      UPLL_LOG_DEBUG("IsFlowListMatched failed from ppe %d", result_code);
      return result_code;
    }
    if (req->datatype == UPLL_DT_CANDIDATE) {
      TcConfigMode config_mode = TC_CONFIG_INVALID;
      std::string vtn_name = "";
      result_code = GetConfigModeInfo(req, config_mode, vtn_name);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("GetConfigModeInfo failed");
        return result_code;
      }
      // verify the flowlist reference in running DB also
      if (config_mode == TC_CONFIG_VIRTUAL) {
        result_code = mgr->IsFlowListMatched(
           reinterpret_cast<const char *>(key_fle->flowlist_key.flowlist_name),
           UPLL_DT_RUNNING, dmi);
        if (UPLL_RC_SUCCESS != result_code) {
          UPLL_LOG_DEBUG("IsFlowListMatched failed from ppe %d", result_code);
          return UPLL_RC_ERR_CFG_SEMANTIC;
        }
      }
    }
  }
  return result_code;
}

upll_rc_t FlowListEntryMoMgr::CopyToConfigKey(ConfigKeyVal *&okey,
    ConfigKeyVal *ikey) {
  if ( !ikey || !(ikey->get_key()) )
    return UPLL_RC_ERR_GENERIC;

  key_flowlist_entry_t *key_flowlist = NULL;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  key_rename_vnode_info_t *key_rename =
    reinterpret_cast<key_rename_vnode_info_t *>(ikey->get_key());
  key_flowlist =
    reinterpret_cast<key_flowlist_entry_t *>
    (ConfigKeyVal::Malloc(sizeof(key_flowlist_entry_t)));
  uuu::upll_strncpy(key_flowlist->flowlist_key.flowlist_name,
                    key_rename->old_flowlist_name,
                    (kMaxLenFlowListName+1));
  okey = new ConfigKeyVal(UNC_KT_FLOWLIST_ENTRY,
      IpctSt::kIpcStKeyFlowlistEntry, key_flowlist, NULL);
  if (!okey) {
    free(key_flowlist);
    UPLL_LOG_DEBUG("Copy to ConfigKey Failed");
    result_code = UPLL_RC_ERR_GENERIC;
  }

  return result_code;
}

upll_rc_t FlowListEntryMoMgr::CreateCandidateMo(IpcReqRespHeader *req,
                                       ConfigKeyVal *ikey,
                                       DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  if (ikey == NULL || req == NULL) {
        return UPLL_RC_ERR_GENERIC;
  }
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  if (req->datatype != UPLL_DT_IMPORT) {
    // validate syntax
    result_code = ValidateMessage(req, ikey);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("ValidateMessage failed, Error - %d", result_code);
      return result_code;
    }
    // Check whether instance is already exists or not
    DbSubOp dbop = {kOpReadExist, kOpMatchNone, kOpInOutFlag};
    result_code = UpdateConfigDB(ikey, req->datatype, UNC_OP_READ,
                               dmi, &dbop, MAINTBL);
    if (result_code == UPLL_RC_ERR_INSTANCE_EXISTS) {
      UPLL_LOG_DEBUG("Given sequence number is already configured");
      return result_code;
    } else if (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      UPLL_LOG_INFO("Error while fetching data from DB, err-%d",
                    result_code);
      return result_code;
    }
  }
  result_code = ValidateAttribute(ikey, dmi, req);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("ValidateAttribute failed %d", result_code);
    return result_code;
  }
  // validate syntax for parameters that has
  // database dependency
  result_code = ValidateFlowlistEntryVal(req, ikey, dmi);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("ValidateMessageVal failed, Error - %d", result_code);
    return result_code;
  }
  TcConfigMode config_mode = TC_CONFIG_INVALID;
  std::string vtn_name = "";
  result_code = GetConfigModeInfo(req, config_mode, vtn_name);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetConfigMode failed");
    return result_code;
  }

  // create a record in CANDIDATE DB
  result_code = UpdateConfigDB(ikey, req->datatype, UNC_OP_CREATE, dmi,
                               config_mode, vtn_name);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("UpdateConfigDB failed for maintbl %d", result_code);
    return result_code;
  }
  result_code = CreateEntryCtrlrTbl(req, ikey, dmi);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("CreateEntryCtrlrTbl failed %d", result_code);
    return result_code;
  }
  return result_code;
}

upll_rc_t FlowListEntryMoMgr::CreateEntryCtrlrTbl(IpcReqRespHeader *req,
                              ConfigKeyVal *ikey,
                              DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  ConfigKeyVal *pkey = NULL, *tmp_ikey = NULL;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  result_code = GetParentConfigKey(pkey, ikey);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_INFO("GetParentConfigKey Error:%d", result_code);
    return result_code;
  }

  DbSubOp dbop = {kOpReadMultiple, kOpMatchNone, kOpInOutCtrlr};
  FlowListMoMgr *mgr =
    reinterpret_cast<FlowListMoMgr *>(const_cast<MoManager *>(GetMoManager(
            UNC_KT_FLOWLIST)));
  if (NULL == mgr) {
    UPLL_LOG_DEBUG("mgr is NULL");
    DELETE_IF_NOT_NULL(pkey);
    return UPLL_RC_ERR_GENERIC;
  }
  result_code = mgr->ReadConfigDB(pkey, req->datatype, UNC_OP_READ,
                             dbop, dmi, CTRLRTBL);
  if (UPLL_RC_SUCCESS != result_code) {
    if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
      UPLL_LOG_DEBUG("No record in flowlist ctrlrtbl");
      DELETE_IF_NOT_NULL(pkey);
      return UPLL_RC_SUCCESS;
    }
    UPLL_LOG_DEBUG("ReadConfigDB failed %d", result_code);
    DELETE_IF_NOT_NULL(pkey);
    return result_code;
  }
  TcConfigMode config_mode = TC_CONFIG_INVALID;
  std::string vtn_name = "";
  result_code = GetConfigModeInfo(req, config_mode, vtn_name);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetConfigMode failed");
    return result_code;
  }

  ConfigKeyVal *temp_pkey = pkey;
  while (NULL != temp_pkey) {
    ConfigKeyVal *ctrlr_ckv = NULL;
    result_code = GetChildConfigKey(ctrlr_ckv, ikey);
    if (UPLL_RC_SUCCESS != result_code) {
      DELETE_IF_NOT_NULL(pkey);
      return result_code;
    }
    uint8_t *ctrlr_id = NULL;
    uuc::CtrlrMgr *ctrlr_mgr = uuc::CtrlrMgr::GetInstance();
    unc_keytype_ctrtype_t ctrlrtype = UNC_CT_UNKNOWN;
    GET_USER_DATA_CTRLR(temp_pkey, ctrlr_id);
    SET_USER_DATA_CTRLR(ctrlr_ckv, ctrlr_id);

    // Validate whether the attributes supported by controller or not
    result_code = ValidateCapability(req, ikey, reinterpret_cast<char *>(
                                     ctrlr_id));
    if (result_code != UPLL_RC_SUCCESS) {
    // Error should be returned if the failure code is other then ctrlr not
    // supported
      DELETE_IF_NOT_NULL(ctrlr_ckv);
      if ((!ctrlr_mgr->GetCtrlrType(reinterpret_cast<char *>(ctrlr_id),
          req->datatype, &ctrlrtype)) || ((ctrlrtype != UNC_CT_PFC) &&
                                          (ctrlrtype != UNC_CT_ODC))) {
          result_code = UPLL_RC_SUCCESS;
          UPLL_LOG_DEBUG("Controller type is  %d", ctrlrtype);
          temp_pkey = temp_pkey->get_next_cfg_key_val();
          continue;
      }
      DELETE_IF_NOT_NULL(pkey);
      UPLL_LOG_DEBUG("ValidateCapability Failed: result_code=%d",
      result_code);
      return result_code;
    }
    result_code = DupConfigKeyVal(tmp_ikey, ikey, MAINTBL);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("DupConfigKeyVal Error");
      DELETE_IF_NOT_NULL(pkey);
      return result_code;
    }
    val_flowlist_entry_t *val_fle = reinterpret_cast<val_flowlist_entry_t *>
                                    (GetVal(tmp_ikey));
    /** change NOT_SOPPRTED attribute to INVALID to store in ctrlr table */
    SetValidAttributesForController(val_fle);

    val_flowlist_entry_ctrl_t *val_ctrlr = reinterpret_cast
        <val_flowlist_entry_ctrl_t *>
        (ConfigKeyVal::Malloc(sizeof(val_flowlist_entry_ctrl_t)));
    for ( unsigned int loop = 0;
        loop < sizeof(val_fle->valid)/sizeof(val_fle->valid[0]);
        ++loop ) {
      val_ctrlr->valid[loop] = val_fle->valid[loop];
    }
    ctrlr_ckv->AppendCfgVal(IpctSt::kIpcInvalidStNum, val_ctrlr);
    DbSubOp dbop1 = {kOpNotRead, kOpMatchNone, kOpInOutCtrlr};
    result_code = UpdateConfigDB(ctrlr_ckv, req->datatype, UNC_OP_CREATE,
                               dmi, &dbop1, config_mode, vtn_name, CTRLRTBL);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_INFO("UpdateConfigDB failed %d", result_code);
      DELETE_IF_NOT_NULL(pkey);
      DELETE_IF_NOT_NULL(tmp_ikey);
      DELETE_IF_NOT_NULL(ctrlr_ckv);
      return result_code;
    }
    DELETE_IF_NOT_NULL(tmp_ikey);
    DELETE_IF_NOT_NULL(ctrlr_ckv);
    temp_pkey = temp_pkey->get_next_cfg_key_val();
  }
  DELETE_IF_NOT_NULL(pkey);
  return UPLL_RC_SUCCESS;
}

upll_rc_t FlowListEntryMoMgr::UpdateMo(IpcReqRespHeader *req,
                              ConfigKeyVal *ikey,
                              DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;

  uint8_t *ctrlr_id = NULL;
  upll_rc_t result_code = UPLL_RC_ERR_GENERIC;
  if (NULL == ikey || NULL == req || !(ikey->get_key())) {
     UPLL_LOG_DEBUG("Given Input is Empty");
     return UPLL_RC_ERR_GENERIC;
  }

  TcConfigMode config_mode = TC_CONFIG_INVALID;
  std::string vtn_name = "";
  result_code = GetConfigModeInfo(req, config_mode, vtn_name);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetConfigMode failed");
    return result_code;
  }

  if (config_mode == TC_CONFIG_VIRTUAL) {
    UPLL_LOG_INFO("VIRTUAL Mode flowlist_entry UPDATE");
    return UPLL_RC_ERR_NOT_ALLOWED_AT_THIS_TIME;
  }

  result_code = ValidateMessage(req, ikey);
  if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("Validation Message is Failed ");
      return result_code;
  }
  //  Check whether it is a update request or replay of configuration
  ConfigKeyVal *db_ckv = NULL;
  result_code = GetChildConfigKey(db_ckv, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_INFO("GetChildConfigKey is failed result_code = %d",
                    result_code);
    return result_code;
  }
  //  Read the record from DB and compare it with the incoming request
  DbSubOp dbop = {kOpReadSingle, kOpMatchNone, kOpInOutNone};
  result_code = ReadConfigDB(db_ckv, UPLL_DT_CANDIDATE, UNC_OP_READ,
                             dbop, dmi, MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_INFO("ReadConfigDB failed %d",
                  result_code);
    DELETE_IF_NOT_NULL(db_ckv);
    return result_code;
  }
  ConfigKeyVal *dup_ikey = NULL;
  result_code = DupConfigKeyVal(dup_ikey, ikey, MAINTBL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_INFO("DupConfigKeyVal failed %d", result_code);
    DELETE_IF_NOT_NULL(db_ckv);
    return result_code;
  }
  bool attr_modified = IsAttributeUpdated(GetVal(dup_ikey), GetVal(db_ckv));
  DELETE_IF_NOT_NULL(dup_ikey);
  DELETE_IF_NOT_NULL(db_ckv);
  if (!attr_modified) {
    UPLL_LOG_DEBUG("No attribute is changed");
    return UPLL_RC_SUCCESS;
  }
  //  Continue usual update validation and process
  result_code = ValidateFlowlistEntryVal(req, ikey, dmi);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("ValidateMessageVal failed, Error - %d", result_code);
    return result_code;
  }
  if (IsAllAttrInvalid(
    reinterpret_cast<val_flowlist_entry_t *>(GetVal(ikey)))) {
    UPLL_LOG_INFO("No attributes to be updated");
    return UPLL_RC_SUCCESS;
  }
  result_code = ValidateAttribute(ikey, dmi, req);
  if (UPLL_RC_SUCCESS  != result_code) {
      UPLL_LOG_DEBUG("Validate Attribute is Failed");
      return result_code;
  }

  ConfigKeyVal *ctrl_key = NULL;
  // Construct the DUP key to update in the controller table
  result_code = DupConfigKeyVal(ctrl_key, ikey, MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("DupConfigKeyVal is failed result_code = %d",
                    result_code);
      return result_code;
  }

  result_code = UpdateConfigDB(ikey, req->datatype, UNC_OP_UPDATE,
                               dmi, config_mode, vtn_name, MAINTBL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("Updation Failure in DB : %d", result_code);
    DELETE_IF_NOT_NULL(ctrl_key);
    return result_code;
  }

  DbSubOp dbop_ctrlr = {kOpReadMultiple, kOpMatchNone,
    kOpInOutCtrlr | kOpInOutDomain | kOpInOutFlag};
  ConfigKeyVal *okey = NULL;
  result_code = GetChildConfigKey(okey, ikey);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetChildConfigKey Failed %d", result_code);
    return result_code;
  }
  result_code = ReadConfigDB(okey, req->datatype, UNC_OP_READ,
                             dbop_ctrlr, dmi, CTRLRTBL);

  if (UPLL_RC_SUCCESS == result_code ||
      result_code == UPLL_RC_ERR_INSTANCE_EXISTS) {
      UPLL_LOG_DEBUG("Record  Exists in flowlist controller table");

      ConfigKeyVal *tmp_key = okey;
      while (tmp_key != NULL) {
        GET_USER_DATA_CTRLR(tmp_key, ctrlr_id);
        result_code = UpdateControllerTable(ctrl_key,
                                          UNC_OP_UPDATE,
                                          req->datatype,
                                          dmi,
                                          reinterpret_cast<char*>(ctrlr_id),
                                          config_mode, vtn_name);

        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_INFO("Failed to Update the controller Table err(%d)",
                       result_code);
          DELETE_IF_NOT_NULL(okey);
          DELETE_IF_NOT_NULL(ctrl_key);
          okey = NULL;
          ctrl_key = NULL;
          return result_code;
        }
        ctrlr_id = NULL;
        tmp_key = tmp_key->get_next_cfg_key_val();
    }
  }

  DELETE_IF_NOT_NULL(okey);
  DELETE_IF_NOT_NULL(ctrl_key);
  UPLL_LOG_TRACE("Updated Done Successfully %d", result_code);
  return UPLL_RC_SUCCESS;
}

upll_rc_t FlowListEntryMoMgr::UpdateMainTbl(ConfigKeyVal *fle_key,
      unc_keytype_operation_t op, uint32_t driver_result,
      ConfigKeyVal *nreq, DalDmlIntf *dmi) {
  ConfigKeyVal *ck_fle = NULL;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  val_flowlist_entry_t *fle_val = NULL;
  void *fleval = NULL;
  void *nfleval = NULL;

  UPLL_FUNC_TRACE;
  if (op != UNC_OP_DELETE) {
    result_code = DupConfigKeyVal(ck_fle, fle_key, MAINTBL);
    if (!ck_fle || result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Returning error %d", result_code);
      return UPLL_RC_ERR_GENERIC;
    }
    fle_val = reinterpret_cast<val_flowlist_entry_t *>(GetVal(ck_fle));
    if (!fle_val) {
      UPLL_LOG_DEBUG("invalid val");
      DELETE_IF_NOT_NULL(ck_fle);
      return UPLL_RC_ERR_GENERIC;
    }
  } else {
    result_code = GetChildConfigKey(ck_fle, fle_key);
    if (!ck_fle || result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Returning error %d", result_code);
      DELETE_IF_NOT_NULL(ck_fle);
      return UPLL_RC_ERR_GENERIC;
    }
  }
  switch (op) {
    case UNC_OP_CREATE:
      fle_val->cs_row_status = UNC_CS_APPLIED;
      break;
    case UNC_OP_UPDATE:
      fleval = GetVal(ck_fle);
      nfleval = (nreq)?GetVal(nreq):NULL;
      if (!nfleval) {
        UPLL_LOG_ERROR("Invalid param");
        DELETE_IF_NOT_NULL(ck_fle);
        return UPLL_RC_ERR_GENERIC;
      }
      CompareValidValue(fleval, nfleval, true);
      fle_val->cs_row_status =
             reinterpret_cast<val_flowlist_entry_t*>(
                 GetVal(nreq))->cs_row_status;

      break;
    case UNC_OP_DELETE:
      break;
    default:
      UPLL_LOG_DEBUG("Inalid operation");
      return UPLL_RC_ERR_GENERIC;
  }

  DbSubOp dbop = {kOpNotRead, kOpMatchNone, kOpInOutNone};
  dbop.inoutop = kOpInOutCs;
  string vtn_name = "";
  result_code = UpdateConfigDB(ck_fle, UPLL_DT_STATE, op, dmi, &dbop,
                               TC_CONFIG_GLOBAL, vtn_name, MAINTBL);
  EnqueCfgNotification(op, UPLL_DT_RUNNING, ck_fle);
  delete ck_fle;
  return result_code;
}

upll_rc_t FlowListEntryMoMgr::GetParentConfigKey(ConfigKeyVal *&okey,
                                                 ConfigKeyVal *ikey) {
  UPLL_FUNC_TRACE;

  if (!ikey) {
    UPLL_LOG_DEBUG(" Input Key is NULL");
    return UPLL_RC_ERR_GENERIC;
  }

  unc_key_type_t ikey_type = ikey->get_key_type();
  if (ikey_type != UNC_KT_FLOWLIST_ENTRY) {
    UPLL_LOG_DEBUG(" Invalid key type received. Key type - %d", ikey_type);
    return UPLL_RC_ERR_GENERIC;
  }

  key_flowlist_entry_t *pkey = reinterpret_cast<key_flowlist_entry_t*>
      (ikey->get_key());
  if (!pkey) {
    UPLL_LOG_DEBUG(" Input flowlist entry key is NULL ");
    return UPLL_RC_ERR_GENERIC;
  }

  key_flowlist_t *fl_key = reinterpret_cast<key_flowlist_t*>
      (ConfigKeyVal::Malloc(sizeof(key_flowlist_t)));

  uuu::upll_strncpy(fl_key->flowlist_name,
                    reinterpret_cast<key_flowlist_entry_t *>
                    (pkey)->flowlist_key.flowlist_name,
                    (kMaxLenFlowListName + 1));
  okey = new ConfigKeyVal(UNC_KT_FLOWLIST, IpctSt::kIpcStKeyFlowlist,
                          fl_key, NULL);
  SET_USER_DATA(okey, ikey);
  return UPLL_RC_SUCCESS;
}

upll_rc_t FlowListEntryMoMgr::SetValidAudit(ConfigKeyVal *&ikey) {
  UPLL_FUNC_TRACE;
  val_flowlist_entry_t *val = reinterpret_cast
      <val_flowlist_entry_t *>(GetVal(ikey));
  if (NULL == val) {
    UPLL_LOG_DEBUG("val is NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  for ( unsigned int loop = 0;
        loop < sizeof(val->valid)/sizeof(val->valid[0]);
        ++loop ) {
    val->cs_attr[loop] = UNC_CS_APPLIED;
  }
  val->cs_row_status = UNC_CS_APPLIED;
  return UPLL_RC_SUCCESS;
}

upll_rc_t FlowListEntryMoMgr::UpdateConfigStatus(ConfigKeyVal *main_ckv,
                                       unc_keytype_operation_t op,
                                       uint32_t driver_result,
                                       ConfigKeyVal *upd_key,
                                       DalDmlIntf *dmi,
                                       ConfigKeyVal *ctrlr_key) {
  UPLL_FUNC_TRACE;
  // upll_rc_t result_code = UPLL_RC_SUCCESS;
  val_flowlist_entry_ctrl_t *ctrlr_val;

  val_flowlist_entry_t *val_main = reinterpret_cast
      <val_flowlist_entry_t *>(GetVal(main_ckv));
  uint8_t cs_status;
  unc_keytype_configstatus_t ctrlr_status =
      (driver_result == UPLL_RC_SUCCESS) ? UNC_CS_APPLIED : UNC_CS_NOT_APPLIED;
  ctrlr_val = reinterpret_cast<val_flowlist_entry_ctrl_t *>(GetVal(ctrlr_key));
  if (ctrlr_val == NULL) return UPLL_RC_ERR_GENERIC;
  cs_status = (val_main->cs_row_status);
  UPLL_LOG_TRACE("cs_status %d ctrlr_status %d\n", cs_status, ctrlr_status);
  if (op == UNC_OP_CREATE) {
    ctrlr_val->cs_row_status = ctrlr_status;
    if (val_main->cs_row_status == UNC_CS_UNKNOWN) {
      /* first entry in ctrlr table */
      cs_status = ctrlr_status;
    } else if (val_main->cs_row_status == UNC_CS_INVALID) {
      cs_status = UNC_CS_INVALID;
    } else if (val_main->cs_row_status == UNC_CS_APPLIED) {
      if (ctrlr_status == UNC_CS_NOT_APPLIED) {
        cs_status = UNC_CS_PARTIALLY_APPLIED;
      }
    } else if (val_main->cs_row_status == UNC_CS_NOT_APPLIED) {
      if (ctrlr_status == UNC_CS_APPLIED) {
        cs_status =  UNC_CS_PARTIALLY_APPLIED;
      }
    } else {
      cs_status = UNC_CS_PARTIALLY_APPLIED;
    }
    val_main->cs_row_status = cs_status;
  }

  val_flowlist_entry_ctrl_t *run_ctrlr_val =
      reinterpret_cast<val_flowlist_entry_ctrl_t *>
      (GetVal(upd_key));
  // Updating the Controller cs_row_status
  if ((op == UNC_OP_UPDATE) && (upd_key != NULL)) {
    void *valmain = reinterpret_cast<void *>(val_main);
    CompareValidValue(valmain, (GetVal(upd_key)), true);
    for (unsigned int loop = 0; loop < sizeof(val_main->valid)/
         sizeof(val_main->valid[0]); ++loop) {
      if ((val_main->valid[loop] != UNC_VF_INVALID) &&
          (val_main->valid[loop] != UNC_VF_VALID_NO_VALUE)) {
        if (ctrlr_status == UNC_CS_APPLIED) {
          if (ctrlr_val->valid[loop] == UNC_VF_VALID) {
            ctrlr_val->cs_attr[loop] = UNC_CS_APPLIED;
          }
          if (val_main->cs_attr[loop] == UNC_CS_UNKNOWN) {
            cs_status = UNC_CS_APPLIED;
          } else if (val_main->cs_attr[loop] == UNC_CS_NOT_APPLIED) {
            cs_status = UNC_CS_PARTIALLY_APPLIED;
          } else if (val_main->cs_attr[loop] == UNC_CS_INVALID) {
            cs_status = UNC_CS_INVALID;
          }
        } else if (ctrlr_status == UNC_CS_NOT_APPLIED) {
          ctrlr_val->cs_attr[loop] = UNC_CS_NOT_APPLIED;
          if (val_main->cs_attr[loop] == UNC_CS_UNKNOWN) {
            cs_status = UNC_CS_NOT_APPLIED;
          } else if (val_main->cs_attr[loop] == UNC_CS_APPLIED) {
            cs_status = UNC_CS_PARTIALLY_APPLIED;
          } else if (val_main->cs_attr[loop] == UNC_CS_INVALID) {
            cs_status = UNC_CS_INVALID;
          }
        }
        val_main->cs_attr[loop]  = cs_status;
        UPLL_LOG_DEBUG("UpdatePath tbl cs_attr : %d", val_main->cs_attr[loop]);
      }
      if (val_main->valid[loop] == UNC_VF_INVALID) {
        if (ctrlr_status == UNC_CS_APPLIED) {
          if ((run_ctrlr_val != NULL) &&
              (run_ctrlr_val->valid[loop] == UNC_VF_VALID)) {
            if (val_main->cs_attr[loop] == UNC_CS_PARTIALLY_APPLIED) {
              val_main->cs_attr[loop] = UNC_CS_PARTIALLY_APPLIED;
            } else if (val_main->cs_attr[loop] == UNC_CS_NOT_APPLIED) {
              val_main->cs_attr[loop] = UNC_CS_NOT_APPLIED;
            } else if (val_main->cs_attr[loop] == UNC_CS_INVALID) {
              val_main->cs_attr[loop] = UNC_CS_INVALID;
            } else {
              val_main->cs_attr[loop]  = ctrlr_status;
            }
          }
          if (ctrlr_val->cs_attr[loop] == UNC_CS_APPLIED) {
            ctrlr_val->cs_attr[loop] = ctrlr_status;
          }
        } else if (ctrlr_status == UNC_CS_NOT_APPLIED) {
          if ((run_ctrlr_val != NULL) &&
              (run_ctrlr_val->valid[loop] == UNC_VF_VALID)) {
            if (val_main->cs_attr[loop] == UNC_CS_PARTIALLY_APPLIED) {
              val_main->cs_attr[loop]  = UNC_CS_PARTIALLY_APPLIED;
            } else if (val_main->cs_attr[loop] == UNC_CS_NOT_APPLIED) {
              val_main->cs_attr[loop]  = UNC_CS_NOT_APPLIED;
            } else if (val_main->cs_attr[loop] == UNC_CS_INVALID) {
              val_main->cs_attr[loop]  = UNC_CS_INVALID;
            } else {
              val_main->cs_attr[loop]  = UNC_CS_PARTIALLY_APPLIED;
            }
          }
        }
      }
      if (val_main->valid[loop] == UNC_VF_VALID_NO_VALUE) {
        ctrlr_val->cs_attr[loop] = UNC_CS_UNKNOWN;
        val_main->cs_attr[loop]  = UNC_CS_UNKNOWN;
      }
    }
  }

  if (op == UNC_OP_CREATE) {
    for (unsigned int loop = 0; loop < sizeof(val_main->valid)/
         sizeof(val_main->valid[0]); ++loop) {
      if (val_main->valid[loop] != UNC_VF_INVALID) {
        if (ctrlr_val->cs_attr[loop] != UNC_CS_NOT_SUPPORTED)
          ctrlr_val->cs_attr[loop] = ctrlr_status;
        else
          ctrlr_val->cs_attr[loop] = UNC_CS_NOT_SUPPORTED;

        if (val_main->cs_attr[loop] == ctrlr_status) {
          cs_status = ctrlr_status;
        } else if (ctrlr_status == UNC_CS_APPLIED) {
          if (val_main->cs_attr[loop] == UNC_CS_UNKNOWN) {
            cs_status = ctrlr_status;
          } else if (val_main->cs_attr[loop] == UNC_CS_NOT_APPLIED) {
            val_main->cs_attr[loop] = UNC_CS_PARTIALLY_APPLIED;
          } else if (val_main->cs_attr[loop] == UNC_CS_INVALID) {
            val_main->cs_attr[loop] = UNC_CS_INVALID;
          } else {
            cs_status = val_main->cs_attr[loop];
          }
        } else if (ctrlr_status == UNC_CS_NOT_APPLIED) {
          if (val_main->cs_attr[loop] == UNC_CS_UNKNOWN) {
            cs_status =  UNC_CS_NOT_APPLIED;
          } else if (val_main->cs_attr[loop] == UNC_CS_NOT_APPLIED) {
            cs_status =  UNC_CS_NOT_APPLIED;
          } else if (val_main->cs_attr[loop] == UNC_CS_APPLIED) {
            cs_status =  UNC_CS_PARTIALLY_APPLIED;
          } else if (val_main->cs_attr[loop] == UNC_CS_INVALID) {
            val_main->cs_attr[loop] = UNC_CS_INVALID;
          } else {
            cs_status =  UNC_CS_PARTIALLY_APPLIED;
          }
        }
        val_main->cs_attr[loop]  = cs_status;
        UPLL_LOG_DEBUG("Main tbl cs_attr : %d", val_main->cs_attr[loop]);
      }
    }
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t FlowListEntryMoMgr::Get_Tx_Consolidated_Status(
    unc_keytype_configstatus_t &status,
    unc_keytype_configstatus_t  drv_result_status,
    unc_keytype_configstatus_t current_cs,
    unc_keytype_configstatus_t current_ctrlr_cs) {

  switch (current_cs) {
    case UNC_CS_UNKNOWN:
      status = drv_result_status;
      break;
    case UNC_CS_PARTIALLY_APPLIED:
      if (current_ctrlr_cs == UNC_CS_NOT_APPLIED) {
        // Todo: if this vtn has caused it then to change to applied.
        status = (drv_result_status != UNC_CS_APPLIED) ?
          UNC_CS_PARTIALLY_APPLIED : drv_result_status;
      }
      break;
    case UNC_CS_APPLIED:
    case UNC_CS_NOT_APPLIED:
    case UNC_CS_INVALID:
    default:
      status = (drv_result_status == UNC_CS_NOT_APPLIED)?
        UNC_CS_PARTIALLY_APPLIED:
        (status == UNC_CS_UNKNOWN)?drv_result_status:status;
      break;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t FlowListEntryMoMgr::SetRenameFlag(ConfigKeyVal *ikey,
                                            DalDmlIntf *dmi,
                                            IpcReqRespHeader *req) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *pkey = NULL;
  result_code = GetParentConfigKey(pkey, ikey);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetParentConfigKey failed %d", result_code);
    return result_code;
  }
  MoMgrImpl *mgr =
    reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>(GetMoManager(
            UNC_KT_FLOWLIST)));
  if (!mgr) {
    UPLL_LOG_DEBUG("mgr is NULL");
    DELETE_IF_NOT_NULL(pkey);
    return UPLL_RC_ERR_GENERIC;
  }
  uint8_t rename = 0;
  result_code = mgr->IsRenamed(pkey, req->datatype, dmi, rename);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("IsRenamed failed %d", result_code);
    DELETE_IF_NOT_NULL(pkey);
    return result_code;
  }
  UPLL_LOG_DEBUG("Flag from parent : %d", rename);
  SET_USER_DATA_FLAGS(ikey, rename);
  DELETE_IF_NOT_NULL(pkey);
  return UPLL_RC_SUCCESS;
}

bool FlowListEntryMoMgr::IsAllAttrInvalid(
        val_flowlist_entry_t *val) {
  for ( unsigned int loop = 0;
      loop < sizeof(val->valid)/sizeof(val->valid[0]); ++loop ) {
    if (UNC_VF_INVALID != val->valid[loop])
      return false;
  }
  return true;
}

upll_rc_t FlowListEntryMoMgr::GetOperation(uuc::UpdateCtrlrPhase phase,
                                           unc_keytype_operation_t &op) {
  if (uuc::kUpllUcpDelete == phase) {
    UPLL_LOG_DEBUG("Delete phase 1");
    return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_KT;
  } else if (uuc::kUpllUcpUpdate == phase) {
    op = UNC_OP_UPDATE;
  } else if (uuc::kUpllUcpCreate == phase) {
    op = UNC_OP_CREATE;
  } else if (uuc::kUpllUcpDelete2 == phase) {
    op = UNC_OP_DELETE;
  } else if (uuc::kUpllUcpInit == phase) {
    return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_KT;
  } else {
    return UPLL_RC_ERR_GENERIC;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t FlowListEntryMoMgr::GetDomainsForController(
    ConfigKeyVal *ckv_drvr,
    ConfigKeyVal *&ctrlr_ckv,
    DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = GetChildConfigKey(ctrlr_ckv, ckv_drvr);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetChildConfigKey failed %d", result_code);
    return result_code;
  }
  DbSubOp dbop = { kOpReadSingle, kOpMatchCtrlr, kOpInOutCtrlr };
  return ReadConfigDB(ctrlr_ckv, UPLL_DT_RUNNING, UNC_OP_READ, dbop, dmi,
                             CTRLRTBL);
}

bool FlowListEntryMoMgr::IsAttributeUpdated(void *val1, void *val2) {
  UPLL_FUNC_TRACE;
  val_flowlist_entry_t *flowlist_entry_val1 =
    reinterpret_cast<val_flowlist_entry_t *>(val1);

  val_flowlist_entry_t *flowlist_entry_val2 =
    reinterpret_cast<val_flowlist_entry_t *>(val2);
  for ( unsigned int loop = 0; loop < (sizeof(flowlist_entry_val1->valid)
                         /(sizeof(flowlist_entry_val1->valid[0])));
          ++loop ) {
      if ((UNC_VF_VALID_NO_VALUE == flowlist_entry_val1->valid[loop] &&
                  UNC_VF_VALID == flowlist_entry_val2->valid[loop]) ||
          (UNC_VF_VALID == flowlist_entry_val1->valid[loop] &&
                  UNC_VF_INVALID == flowlist_entry_val2->valid[loop]))
        return true;
  }
  if (UNC_VF_VALID == flowlist_entry_val1->valid[UPLL_IDX_MAC_DST_FLE] &&
      UNC_VF_VALID == flowlist_entry_val2->valid[UPLL_IDX_MAC_DST_FLE])
    if (memcmp(flowlist_entry_val1->mac_dst, flowlist_entry_val2->mac_dst,
        sizeof(flowlist_entry_val2->mac_dst)))
      return true;

  if (UNC_VF_VALID == flowlist_entry_val1->valid[UPLL_IDX_MAC_SRC_FLE] &&
      UNC_VF_VALID == flowlist_entry_val2->valid[UPLL_IDX_MAC_SRC_FLE] )
    if (memcmp(flowlist_entry_val1->mac_src, flowlist_entry_val2->mac_src,
       sizeof(flowlist_entry_val2->mac_src)))
      return true;

  if (UNC_VF_VALID == flowlist_entry_val1->valid[UPLL_IDX_MAC_ETH_TYPE_FLE] &&
      UNC_VF_VALID == flowlist_entry_val2->valid[UPLL_IDX_MAC_ETH_TYPE_FLE] )
    if (flowlist_entry_val1->mac_eth_type != flowlist_entry_val2->mac_eth_type)
      return true;

  if (UNC_VF_VALID == flowlist_entry_val1->valid[UPLL_IDX_DST_IP_FLE] &&
      UNC_VF_VALID == flowlist_entry_val2->valid[UPLL_IDX_DST_IP_FLE] )
    if (memcmp(&flowlist_entry_val1->dst_ip,
               &flowlist_entry_val2->dst_ip,
               sizeof(flowlist_entry_val2->dst_ip)))
      return true;

  if (UNC_VF_VALID == flowlist_entry_val1->valid[UPLL_IDX_DST_IP_PREFIX_FLE] &&
      UNC_VF_VALID == flowlist_entry_val2->valid[UPLL_IDX_DST_IP_PREFIX_FLE] )
    if (flowlist_entry_val1->dst_ip_prefixlen  !=
                            flowlist_entry_val2->dst_ip_prefixlen)
      return true;


  if (UNC_VF_VALID == flowlist_entry_val1->valid[UPLL_IDX_SRC_IP_FLE] &&
      UNC_VF_VALID == flowlist_entry_val2->valid[UPLL_IDX_SRC_IP_FLE] )
    if (memcmp(&flowlist_entry_val1->src_ip,
               &flowlist_entry_val2->src_ip,
               sizeof(flowlist_entry_val2->src_ip)))
      return true;

  if (UNC_VF_VALID == flowlist_entry_val1->valid[UPLL_IDX_SRC_IP_PREFIX_FLE] &&
      UNC_VF_VALID == flowlist_entry_val2->valid[UPLL_IDX_SRC_IP_PREFIX_FLE] )
    if (flowlist_entry_val1->src_ip_prefixlen  !=
                            flowlist_entry_val2->src_ip_prefixlen)
      return true;

  if (UNC_VF_VALID == flowlist_entry_val1->valid[UPLL_IDX_VLAN_PRIORITY_FLE] &&
      UNC_VF_VALID == flowlist_entry_val2->valid[UPLL_IDX_VLAN_PRIORITY_FLE] )
    if (flowlist_entry_val1->vlan_priority  !=
                                          flowlist_entry_val2->vlan_priority)
      return true;

  if (UNC_VF_VALID == flowlist_entry_val1->valid[UPLL_IDX_DST_IP_V6_FLE] &&
      UNC_VF_VALID == flowlist_entry_val2->valid[UPLL_IDX_DST_IP_V6_FLE] )
    if (memcmp(&flowlist_entry_val1->dst_ipv6,
               &flowlist_entry_val2->dst_ipv6,
               sizeof(flowlist_entry_val2->dst_ipv6)))
      return true;

  if (UNC_VF_VALID ==
      flowlist_entry_val1->valid[UPLL_IDX_DST_IP_V6_PREFIX_FLE] &&
      UNC_VF_VALID == flowlist_entry_val2->valid[UPLL_IDX_DST_IP_V6_PREFIX_FLE])
    if (flowlist_entry_val1->dst_ipv6_prefixlen  !=
                            flowlist_entry_val2->dst_ipv6_prefixlen)
      return true;

  if (UNC_VF_VALID == flowlist_entry_val1->valid[UPLL_IDX_SRC_IP_V6_FLE] &&
      UNC_VF_VALID == flowlist_entry_val2->valid[UPLL_IDX_SRC_IP_V6_FLE] )
    if (memcmp(&flowlist_entry_val1->src_ipv6,
               &flowlist_entry_val2->src_ipv6,
               sizeof(flowlist_entry_val2->src_ipv6)))
      return true;

  if (UNC_VF_VALID ==
      flowlist_entry_val1->valid[UPLL_IDX_SRC_IP_V6_PREFIX_FLE] &&
      UNC_VF_VALID == flowlist_entry_val2->valid[UPLL_IDX_SRC_IP_V6_PREFIX_FLE])
    if (flowlist_entry_val1->src_ipv6_prefixlen  !=
                            flowlist_entry_val2->src_ipv6_prefixlen)
      return true;

  if (UNC_VF_VALID == flowlist_entry_val1->valid[UPLL_IDX_IP_PROTOCOL_FLE] &&
      UNC_VF_VALID == flowlist_entry_val2->valid[UPLL_IDX_IP_PROTOCOL_FLE] )
    if (flowlist_entry_val1->ip_proto != flowlist_entry_val2->ip_proto)
      return true;

  if (UNC_VF_VALID == flowlist_entry_val1->valid[UPLL_IDX_IP_DSCP_FLE] &&
      UNC_VF_VALID == flowlist_entry_val2->valid[UPLL_IDX_IP_DSCP_FLE] )
    if (flowlist_entry_val1->ip_dscp != flowlist_entry_val2->ip_dscp)
      return true;


  if (UNC_VF_VALID == flowlist_entry_val1->valid[UPLL_IDX_L4_DST_PORT_FLE] &&
      UNC_VF_VALID == flowlist_entry_val2->valid[UPLL_IDX_L4_DST_PORT_FLE] )
    if (flowlist_entry_val1->l4_dst_port != flowlist_entry_val2->l4_dst_port)
      return true;

  if (UNC_VF_VALID == flowlist_entry_val1->valid[UPLL_IDX_L4_DST_PORT_ENDPT_FLE]
     && UNC_VF_VALID ==
        flowlist_entry_val2->valid[UPLL_IDX_L4_DST_PORT_ENDPT_FLE])
    if (flowlist_entry_val1->l4_dst_port_endpt !=
                           flowlist_entry_val2->l4_dst_port_endpt)
      return true;

  if (UNC_VF_VALID == flowlist_entry_val1->valid[UPLL_IDX_L4_SRC_PORT_FLE] &&
      UNC_VF_VALID == flowlist_entry_val2->valid[UPLL_IDX_L4_SRC_PORT_FLE] )
    if (flowlist_entry_val1->l4_src_port != flowlist_entry_val2->l4_src_port)
      return true;


  if (UNC_VF_VALID ==
      flowlist_entry_val1->valid[UPLL_IDX_L4_SRC_PORT_ENDPT_FLE] &&
      UNC_VF_VALID ==
      flowlist_entry_val2->valid[UPLL_IDX_L4_SRC_PORT_ENDPT_FLE] )
    if (flowlist_entry_val1->l4_src_port_endpt !=
                           flowlist_entry_val2->l4_src_port_endpt)
      return true;

  if (UNC_VF_VALID == flowlist_entry_val1->valid[UPLL_IDX_ICMP_TYPE_FLE] &&
      UNC_VF_VALID == flowlist_entry_val2->valid[UPLL_IDX_ICMP_TYPE_FLE])
    if (flowlist_entry_val1->icmp_type != flowlist_entry_val2->icmp_type)
      return true;

  if (UNC_VF_VALID == flowlist_entry_val1->valid[UPLL_IDX_ICMP_CODE_FLE] &&
      UNC_VF_VALID == flowlist_entry_val2->valid[UPLL_IDX_ICMP_CODE_FLE])
    if (flowlist_entry_val1->icmp_code != flowlist_entry_val2->icmp_code)
      return true;

  if (UNC_VF_VALID == flowlist_entry_val1->valid[UPLL_IDX_ICMP_V6_TYPE_FLE] &&
      UNC_VF_VALID == flowlist_entry_val2->valid[UPLL_IDX_ICMP_V6_TYPE_FLE])
    if (flowlist_entry_val1->icmpv6_type != flowlist_entry_val2->icmpv6_type)
      return true;

  if (UNC_VF_VALID == flowlist_entry_val1->valid[UPLL_IDX_ICMP_V6_CODE_FLE] &&
      UNC_VF_VALID == flowlist_entry_val2->valid[UPLL_IDX_ICMP_V6_CODE_FLE])
    if (flowlist_entry_val1->icmpv6_code != flowlist_entry_val2->icmpv6_code)
      return true;

  return false;
}

upll_rc_t FlowListEntryMoMgr::ClearVirtualKtDirtyInGlobal(DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code;

  unc_keytype_operation_t op_arr[] = { UNC_OP_CREATE,
                                       UNC_OP_DELETE,
                                       UNC_OP_UPDATE};
  uint32_t nop = 3;

  uudst::kDalTableIndex tbl_idx = GetTable(MAINTBL, UPLL_DT_CANDIDATE);
  for (uint32_t i = 0; i < nop; i++) {
    result_code = DalToUpllResCode(dmi->ClearGlobalDirtyTblCacheAndDB(
                                            tbl_idx, op_arr[i]));
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("ClearGlobalDirtyTblCacheAndDB failed %d", result_code);
      return result_code;
    }
  }

  GetTable(CTRLRTBL, UPLL_DT_CANDIDATE);
  result_code = DalToUpllResCode(dmi->ClearGlobalDirtyTblCacheAndDB(
                                          tbl_idx, UNC_OP_UPDATE));
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("ClearGlobalDirtyTblCacheAndDB failed %d", result_code);
    return result_code;
  }
  return UPLL_RC_SUCCESS;
}
}  // namespace kt_momgr
}  // namespace upll
}  // namespace unc
