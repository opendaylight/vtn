/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <set>
#include "policingprofile_entry_momgr.hh"
#include "policingprofile_momgr.hh"
#include "flowlist_momgr.hh"
#include "upll_log.hh"
#include "upll_validation.hh"
#include "unc/upll_ipc_enum.h"
#include "unc/upll_errno.h"

#define GET_VALID_MAINCTRL(tbl, l_val_ctrl_ff, l_val_ff, en) \
  (tbl == MAINTBL) ? &(l_val_ff->valid[en]) : &(l_val_ctrl_ff->valid[en])

namespace unc {
namespace upll {
namespace kt_momgr {

#define PP_RENAME_FLAG 0x01
#define FL_RENAME_FLAG 0x02
#define FLT_RENAME_FLAG 0X04

#define NUM_PP_KEY_MAIN_COL 3
#define NUM_PP_KEY_CTRLR_COL 5
#define NUM_FL_KEY_MAIN_COL 4
#define NUM_FL_KEY_CTRLR_COL 4

BindInfo PolicingProfileEntryMoMgr::policingprofileentry_bind_info[] = {
  { uudst::policingprofile_entry::kDbiPolicingProfileName,
    CFG_KEY,
    offsetof(key_policingprofile_entry_t,
             policingprofile_key.policingprofile_name),
    uud::kDalChar,
    (kMaxLenPolicingProfileName + 1) },
  { uudst::policingprofile_entry::kDbiSequenceNum,
    CFG_KEY,
    offsetof(key_policingprofile_entry_t, sequence_num),
    uud::kDalUint8, 1 },
  { uudst::policingprofile_entry::kDbiFlowlist,
    CFG_VAL,
    offsetof(val_policingprofile_entry_t, flowlist),
    uud::kDalChar,
    (kMaxLenFlowListName + 1) },
  { uudst::policingprofile_entry::kDbiRate,
    CFG_VAL,
    offsetof(val_policingprofile_entry_t, rate),
    uud::kDalUint8, 1 },
  { uudst::policingprofile_entry::kDbiCir,
    CFG_VAL,
    offsetof(val_policingprofile_entry_t, cir),
    uud::kDalUint32, 1 },
  { uudst::policingprofile_entry::kDbiCbs,
    CFG_VAL,
    offsetof(val_policingprofile_entry_t, cbs),
    uud::kDalUint32, 1 },
  { uudst::policingprofile_entry::kDbiPir,
    CFG_VAL,
    offsetof(val_policingprofile_entry_t, pir),
    uud::kDalUint32, 1 },
  { uudst::policingprofile_entry::kDbiPbs,
    CFG_VAL,
    offsetof(val_policingprofile_entry_t, pbs),
    uud::kDalUint32, 1 },
  { uudst::policingprofile_entry::kDbiGreenAction,
    CFG_VAL,
    offsetof(val_policingprofile_entry_t, green_action),
    uud::kDalUint8, 1 },
  { uudst::policingprofile_entry::kDbiGreenPriority,
    CFG_VAL,
    offsetof(val_policingprofile_entry_t, green_action_priority),
    uud::kDalUint8, 1 },
  { uudst::policingprofile_entry::kDbiGreenDscp,
    CFG_VAL,
    offsetof(val_policingprofile_entry_t, green_action_dscp),
    uud::kDalUint8, 1 },
  { uudst::policingprofile_entry::kDbiGreenDrop,
    CFG_VAL,
    offsetof(val_policingprofile_entry_t, green_action_drop_precedence),
    uud::kDalUint8, 1 },
  { uudst::policingprofile_entry::kDbiYellowAction,
    CFG_VAL,
    offsetof(val_policingprofile_entry_t, yellow_action),
    uud::kDalUint8, 1 },
  { uudst::policingprofile_entry::kDbiYellowPriority,
    CFG_VAL,
    offsetof(val_policingprofile_entry_t, yellow_action_priority),
    uud::kDalUint8, 1 },
  { uudst::policingprofile_entry::kDbiYellowDscp,
    CFG_VAL,
    offsetof(val_policingprofile_entry_t, yellow_action_dscp),
    uud::kDalUint8, 1 },
  { uudst::policingprofile_entry::kDbiYellowDrop,
    CFG_VAL,
    offsetof(val_policingprofile_entry_t, yellow_action_drop_precedence),
    uud::kDalUint8, 1 },
  { uudst::policingprofile_entry::kDbiRedAction,
    CFG_VAL,
    offsetof(val_policingprofile_entry_t, red_action),
    uud::kDalUint8, 1 },
  { uudst::policingprofile_entry::kDbiRedPriority,
    CFG_VAL,
    offsetof(val_policingprofile_entry_t, red_action_priority),
    uud::kDalUint8, 1 },
  { uudst::policingprofile_entry::kDbiRedDscp,
    CFG_VAL,
    offsetof(val_policingprofile_entry_t, red_action_dscp),
    uud::kDalUint8, 1 },
  { uudst::policingprofile_entry::kDbiRedDrop,
    CFG_VAL,
    offsetof(val_policingprofile_entry_t, red_action_drop_precedence),
    uud::kDalUint8, 1 },
  { uudst::policingprofile_entry::kDbiFlags,
    CK_VAL,
    offsetof(key_user_data_t, flags),
    uud::kDalUint8, 1 },
  { uudst::policingprofile_entry::kDbiValidFlowlist,
    CFG_META_VAL,
    offsetof(val_policingprofile_entry_t, valid[0]),
    uud::kDalUint8, 1 },
  { uudst::policingprofile_entry::kDbiValidRate,
    CFG_META_VAL,
    offsetof(val_policingprofile_entry_t, valid[1]),
    uud::kDalUint8, 1 },
  { uudst::policingprofile_entry::kDbiValidCir,
    CFG_META_VAL,
    offsetof(val_policingprofile_entry_t, valid[2]),
    uud::kDalUint8, 1 },
  { uudst::policingprofile_entry::kDbiValidCbs,
    CFG_META_VAL,
    offsetof(val_policingprofile_entry_t, valid[3]),
    uud::kDalUint8, 1 },
  { uudst::policingprofile_entry::kDbiValidPir,
    CFG_META_VAL,
    offsetof(val_policingprofile_entry_t, valid[4]),
    uud::kDalUint8, 1 },
  { uudst::policingprofile_entry::kDbiValidPbs,
    CFG_META_VAL,
    offsetof(val_policingprofile_entry_t, valid[5]),
    uud::kDalUint8, 1 },
  { uudst::policingprofile_entry::kDbiValidGreenAction,
    CFG_META_VAL,
    offsetof(val_policingprofile_entry_t, valid[6]),
    uud::kDalUint8, 1 },
  { uudst::policingprofile_entry::kDbiValidGreenPriority,
    CFG_META_VAL,
    offsetof(val_policingprofile_entry_t, valid[7]),
    uud::kDalUint8, 1 },
  { uudst::policingprofile_entry::kDbiValidGreenDscp,
    CFG_META_VAL, offsetof(val_policingprofile_entry_t, valid[8]),
    uud::kDalUint8, 1 },
  { uudst::policingprofile_entry::kDbiValidGreenDrop,
    CFG_META_VAL,
    offsetof(val_policingprofile_entry_t, valid[9]),
    uud::kDalUint8, 1 },
  { uudst::policingprofile_entry::kDbiValidYellowAction,
    CFG_META_VAL,
    offsetof(val_policingprofile_entry_t, valid[10]),
    uud::kDalUint8, 1 },
  { uudst::policingprofile_entry::kDbiValidYellowPriority,
    CFG_META_VAL,
    offsetof(val_policingprofile_entry_t, valid[11]),
    uud::kDalUint8, 1 },
  { uudst::policingprofile_entry::kDbiValidYellowDscp,
    CFG_META_VAL,
    offsetof(val_policingprofile_entry_t, valid[12]),
    uud::kDalUint8, 1 },
  { uudst::policingprofile_entry::kDbiValidYellowDrop,
    CFG_META_VAL,
    offsetof(val_policingprofile_entry_t, valid[13]),
    uud::kDalUint8, 1 },
  { uudst::policingprofile_entry::kDbiValidRedAction,
    CFG_META_VAL,
    offsetof(val_policingprofile_entry_t, valid[14]),
    uud::kDalUint8, 1 },
  { uudst::policingprofile_entry::kDbiValidRedPriority,
    CFG_META_VAL,
    offsetof(val_policingprofile_entry_t, valid[15]),
    uud::kDalUint8, 1 },
  { uudst::policingprofile_entry::kDbiValidRedDscp,
    CFG_META_VAL,
    offsetof(val_policingprofile_entry_t, valid[16]),
    uud::kDalUint8, 1 },
  { uudst::policingprofile_entry::kDbiValidRedDrop,
    CFG_META_VAL,
    offsetof(val_policingprofile_entry_t, valid[17]),
    uud::kDalUint8, 1 },
  { uudst::policingprofile_entry::kDbiCsRowStatus,
    CS_VAL,
    offsetof(val_policingprofile_entry_t, cs_row_status),
    uud::kDalUint8, 1 },
  { uudst::policingprofile_entry::kDbiCsFlowlist,
    CS_VAL,
    offsetof(val_policingprofile_entry_t, cs_attr[0]),
    uud::kDalUint8, 1 },
  { uudst::policingprofile_entry::kDbiCsRate,
    CS_VAL,
    offsetof(val_policingprofile_entry_t, cs_attr[1]),
    uud::kDalUint8, 1 },
  { uudst::policingprofile_entry::kDbiCsCir,
    CS_VAL,
    offsetof(val_policingprofile_entry_t, cs_attr[2]),
    uud::kDalUint8, 1 },
  { uudst::policingprofile_entry::kDbiCsCbs, CS_VAL, offsetof(
      val_policingprofile_entry_t, cs_attr[3]),
  uud::kDalUint8, 1 },
  { uudst::policingprofile_entry::kDbiCsPir, CS_VAL, offsetof(
      val_policingprofile_entry_t, cs_attr[4]),
  uud::kDalUint8, 1 },
  { uudst::policingprofile_entry::kDbiCsPbs, CS_VAL, offsetof(
      val_policingprofile_entry_t, cs_attr[5]),
  uud::kDalUint8, 1 },
  { uudst::policingprofile_entry::kDbiCsGreenAction, CS_VAL, offsetof(
      val_policingprofile_entry_t, cs_attr[6]),
  uud::kDalUint8, 1 },
  { uudst::policingprofile_entry::kDbiCsGreenPriority, CS_VAL, offsetof(
      val_policingprofile_entry_t, cs_attr[7]),
  uud::kDalUint8, 1 },
  { uudst::policingprofile_entry::kDbiCsGreenDscp, CS_VAL, offsetof(
      val_policingprofile_entry_t, cs_attr[8]),
  uud::kDalUint8, 1 },
  { uudst::policingprofile_entry::kDbiCsGreenDrop, CS_VAL, offsetof(
      val_policingprofile_entry_t, cs_attr[9]),
  uud::kDalUint8, 1 },
  { uudst::policingprofile_entry::kDbiCsYellowAction, CS_VAL, offsetof(
      val_policingprofile_entry_t, cs_attr[10]),
  uud::kDalUint8, 1 },
  { uudst::policingprofile_entry::kDbiCsYellowPriority, CS_VAL, offsetof(
      val_policingprofile_entry_t, cs_attr[11]),
  uud::kDalUint8, 1 },
  { uudst::policingprofile_entry::kDbiCsYellowDscp, CS_VAL, offsetof(
      val_policingprofile_entry_t, cs_attr[12]),
  uud::kDalUint8, 1 },
  { uudst::policingprofile_entry::kDbiCsYellowDrop, CS_VAL, offsetof(
      val_policingprofile_entry_t, cs_attr[13]),
  uud::kDalUint8, 1 },
  { uudst::policingprofile_entry::kDbiCsRedAction, CS_VAL, offsetof(
      val_policingprofile_entry_t, cs_attr[14]),
  uud::kDalUint8, 1 },
  { uudst::policingprofile_entry::kDbiCsRedPriority, CS_VAL, offsetof(
      val_policingprofile_entry_t, cs_attr[15]),
  uud::kDalUint8, 1 },
  { uudst::policingprofile_entry::kDbiCsRedDscp, CS_VAL, offsetof(
      val_policingprofile_entry_t, cs_attr[16]),
  uud::kDalUint8, 1 },
  { uudst::policingprofile_entry::kDbiCsRedDrop, CS_VAL, offsetof(
      val_policingprofile_entry_t, cs_attr[17]),
  uud::kDalUint8, 1 }
};

BindInfo PolicingProfileEntryMoMgr::
  policingprofileentry_controller_bind_info[] = {
  { uudst::policingprofile_entry_ctrlr::kDbiPolicingProfileName,
    CFG_KEY,
    offsetof(key_policingprofile_entry_t,
             policingprofile_key.policingprofile_name),
  uud::kDalChar,
  (kMaxLenPolicingProfileName + 1) },
  { uudst::policingprofile_entry_ctrlr::kDbiSequenceNum, CFG_KEY, offsetof(
      key_policingprofile_entry_t, sequence_num),
  uud::kDalUint8, 1 },
  { uudst::policingprofile_entry_ctrlr::kDbiCtrlrName, CK_VAL, offsetof(
      key_user_data_t, ctrlr_id),
  uud::kDalChar, kMaxLenCtrlrId + 1 },
  { uudst::policingprofile_entry_ctrlr::kDbiFlags, CK_VAL, offsetof(
      val_policingprofile_entry_ctrl_t, flags),
  uud::kDalUint8, 1 },
  { uudst::policingprofile_entry_ctrlr::kDbiValidFlowlist, CFG_META_VAL,
      offsetof(val_policingprofile_entry_ctrl_t, valid[0]),
  uud::kDalUint8, 1 },
  { uudst::policingprofile_entry_ctrlr::kDbiValidRate, CFG_META_VAL, offsetof(
      val_policingprofile_entry_ctrl_t, valid[1]),
  uud::kDalUint8, 1 },
  { uudst::policingprofile_entry_ctrlr::kDbiValidCir, CFG_META_VAL, offsetof(
      val_policingprofile_entry_ctrl_t, valid[2]),
  uud::kDalUint8, 1 },
  { uudst::policingprofile_entry_ctrlr::kDbiValidCbs, CFG_META_VAL, offsetof(
      val_policingprofile_entry_ctrl_t, valid[3]),
  uud::kDalUint8, 1 },
  { uudst::policingprofile_entry_ctrlr::kDbiValidPir, CFG_META_VAL, offsetof(
      val_policingprofile_entry_ctrl_t, valid[4]),
  uud::kDalUint8, 1 },
  { uudst::policingprofile_entry_ctrlr::kDbiValidPbs, CFG_META_VAL, offsetof(
      val_policingprofile_entry_ctrl_t, valid[5]),
  uud::kDalUint8, 1 },
  { uudst::policingprofile_entry_ctrlr::kDbiValidGreenAction, CFG_META_VAL,
      offsetof(val_policingprofile_entry_ctrl_t, valid[6]),
  uud::kDalUint8, 1 },
  { uudst::policingprofile_entry_ctrlr::kDbiValidGreenPriority, CFG_META_VAL,
    offsetof(val_policingprofile_entry_ctrl_t, valid[7]),
  uud::kDalUint8, 1 },
  { uudst::policingprofile_entry_ctrlr::kDbiValidGreenDscp, CFG_META_VAL,
      offsetof(val_policingprofile_entry_ctrl_t, valid[8]),
  uud::kDalUint8, 1 },
  { uudst::policingprofile_entry_ctrlr::kDbiValidGreenDrop, CFG_META_VAL,
      offsetof(val_policingprofile_entry_ctrl_t, valid[9]),
  uud::kDalUint8, 1 },
  { uudst::policingprofile_entry_ctrlr::kDbiValidYellowAction, CFG_META_VAL,
      offsetof(val_policingprofile_entry_ctrl_t, valid[10]),
  uud::kDalUint8, 1 },
  { uudst::policingprofile_entry_ctrlr::kDbiValidYellowPriority, CFG_META_VAL,
      offsetof(val_policingprofile_entry_ctrl_t, valid[11]),
  uud::kDalUint8, 1 },
  { uudst::policingprofile_entry_ctrlr::kDbiValidYellowDscp, CFG_META_VAL,
      offsetof(val_policingprofile_entry_ctrl_t, valid[12]),
  uud::kDalUint8, 1 },
  { uudst::policingprofile_entry_ctrlr::kDbiValidYellowDrop, CFG_META_VAL,
      offsetof(val_policingprofile_entry_ctrl_t, valid[13]),
  uud::kDalUint8, 1 },
  { uudst::policingprofile_entry_ctrlr::kDbiValidRedAction, CFG_META_VAL,
      offsetof(val_policingprofile_entry_ctrl_t, valid[14]),
  uud::kDalUint8, 1 },
  { uudst::policingprofile_entry_ctrlr::kDbiValidRedPriority, CFG_META_VAL,
      offsetof(val_policingprofile_entry_ctrl_t, valid[15]),
  uud::kDalUint8, 1 },
  { uudst::policingprofile_entry_ctrlr::kDbiValidRedDscp, CFG_META_VAL,
      offsetof(val_policingprofile_entry_ctrl_t, valid[16]),
  uud::kDalUint8, 1 },
  { uudst::policingprofile_entry_ctrlr::kDbiValidRedDrop, CFG_META_VAL,
      offsetof(val_policingprofile_entry_ctrl_t, valid[17]),
  uud::kDalUint8, 1 },
  { uudst::policingprofile_entry_ctrlr::kDbiCsRowStatus, CS_VAL, offsetof(
      val_policingprofile_entry_ctrl_t, cs_row_status),
  uud::kDalUint8, 1 },
  { uudst::policingprofile_entry_ctrlr::kDbiCsFlowlist, CS_VAL, offsetof(
      val_policingprofile_entry_ctrl_t, cs_attr[0]),
  uud::kDalUint8, 1 },
  { uudst::policingprofile_entry_ctrlr::kDbiCsRate, CS_VAL, offsetof(
      val_policingprofile_entry_ctrl_t, cs_attr[1]),
  uud::kDalUint8, 1 },
  { uudst::policingprofile_entry_ctrlr::kDbiCsCir, CS_VAL, offsetof(
      val_policingprofile_entry_ctrl_t, cs_attr[2]),
  uud::kDalUint8, 1 },
  { uudst::policingprofile_entry_ctrlr::kDbiCsCbs, CS_VAL, offsetof(
      val_policingprofile_entry_ctrl_t, cs_attr[3]),
  uud::kDalUint8, 1 },
  { uudst::policingprofile_entry_ctrlr::kDbiCsPir, CS_VAL,
    offsetof(val_policingprofile_entry_ctrl_t, cs_attr[4]),
    uud::kDalUint8, 1 },
  { uudst::policingprofile_entry_ctrlr::kDbiCsPbs, CS_VAL,
    offsetof(val_policingprofile_entry_ctrl_t, cs_attr[5]),
    uud::kDalUint8, 1 },
  { uudst::policingprofile_entry_ctrlr::kDbiCsGreenAction, CS_VAL,
    offsetof(val_policingprofile_entry_ctrl_t, cs_attr[6]),
    uud::kDalUint8, 1 },
  { uudst::policingprofile_entry_ctrlr::kDbiCsGreenPriority, CS_VAL,
    offsetof(val_policingprofile_entry_ctrl_t, cs_attr[7]),
    uud::kDalUint8, 1 },
  { uudst::policingprofile_entry_ctrlr::kDbiCsGreenDscp, CS_VAL,
    offsetof(val_policingprofile_entry_ctrl_t, cs_attr[8]),
  uud::kDalUint8, 1 },
  { uudst::policingprofile_entry_ctrlr::kDbiCsGreenDrop, CS_VAL,
    offsetof(val_policingprofile_entry_ctrl_t, cs_attr[9]),
  uud::kDalUint8, 1 },
  { uudst::policingprofile_entry_ctrlr::kDbiCsYellowAction, CS_VAL,
    offsetof(val_policingprofile_entry_ctrl_t, cs_attr[10]),
  uud::kDalUint8, 1 },
  { uudst::policingprofile_entry_ctrlr::kDbiCsYellowPriority, CS_VAL,
    offsetof(val_policingprofile_entry_ctrl_t, cs_attr[11]),
    uud::kDalUint8, 1 },
  { uudst::policingprofile_entry_ctrlr::kDbiCsYellowDscp, CS_VAL,
    offsetof(val_policingprofile_entry_ctrl_t, cs_attr[12]),
  uud::kDalUint8, 1 },
  { uudst::policingprofile_entry_ctrlr::kDbiCsYellowDrop, CS_VAL,
    offsetof(val_policingprofile_entry_ctrl_t, cs_attr[13]),
  uud::kDalUint8, 1 },
  { uudst::policingprofile_entry_ctrlr::kDbiCsRedAction, CS_VAL,
    offsetof(val_policingprofile_entry_ctrl_t, cs_attr[14]),
  uud::kDalUint8, 1 },
  { uudst::policingprofile_entry_ctrlr::kDbiCsRedPriority, CS_VAL,
    offsetof(val_policingprofile_entry_ctrl_t, cs_attr[15]),
    uud::kDalUint8, 1 },
  { uudst::policingprofile_entry_ctrlr::kDbiCsRedDscp, CS_VAL,
    offsetof(val_policingprofile_entry_ctrl_t, cs_attr[16]),
  uud::kDalUint8, 1 },
  { uudst::policingprofile_entry_ctrlr::kDbiCsRedDrop, CS_VAL,
    offsetof(val_policingprofile_entry_ctrl_t, cs_attr[17]),
  uud::kDalUint8, 1 } };

BindInfo PolicingProfileEntryMoMgr::rename_policingprofile_entry_main_tbl[] = {
  { uudst::policingprofile_entry::kDbiPolicingProfileName, CFG_MATCH_KEY,
    offsetof(key_policingprofile_entry_t,
             policingprofile_key.policingprofile_name),
  uud::kDalChar, kMaxLenPolicingProfileName + 1 },
  { uudst::policingprofile_entry::kDbiSequenceNum, CFG_MATCH_KEY, offsetof(
      key_policingprofile_entry_t, sequence_num),
  uud::kDalUint8, 1 },
  { uudst::policingprofile_entry::kDbiPolicingProfileName, CFG_INPUT_KEY,
    offsetof(key_rename_vnode_info_t, new_policingprofile_name),
  uud::kDalChar, kMaxLenPolicingProfileName + 1 },
  { uudst::policingprofile_entry::kDbiFlags, CFG_INPUT_KEY,
    offsetof(key_user_data_t, flags), uud::kDalUint8, 1 } };

BindInfo PolicingProfileEntryMoMgr::rename_policingprofile_entry_ctrl_tbl[] = {
  { uudst::policingprofile_entry_ctrlr::kDbiPolicingProfileName, CFG_MATCH_KEY,
    offsetof(key_policingprofile_entry_t,
        policingprofile_key.policingprofile_name),
    uud::kDalChar, kMaxLenPolicingProfileName + 1 },
  { uudst::policingprofile_entry_ctrlr::kDbiSequenceNum, CFG_MATCH_KEY,
    offsetof(key_policingprofile_entry_t, sequence_num),
  uud::kDalUint8, 1 },
  { uudst::policingprofile_entry_ctrlr::kDbiCtrlrName, CFG_MATCH_KEY, offsetof(
      key_user_data_t, ctrlr_id),
  uud::kDalChar, kMaxLenCtrlrId + 1 },
  { uudst::policingprofile_entry_ctrlr::kDbiPolicingProfileName, CFG_INPUT_KEY,
    offsetof(key_rename_vnode_info_t, new_policingprofile_name),
    uud::kDalChar, kMaxLenPolicingProfileName + 1 },
  { uudst::policingprofile_entry_ctrlr::kDbiFlags, CFG_INPUT_KEY, offsetof(
      key_user_data_t, flags),
  uud::kDalUint8, 1 } };

BindInfo PolicingProfileEntryMoMgr::rename_flowlist_pp_entry_main_tbl[] = {
  { uudst::policingprofile_entry::kDbiPolicingProfileName, CFG_MATCH_KEY,
    offsetof(key_policingprofile_entry_t,
             policingprofile_key.policingprofile_name),
  uud::kDalChar, kMaxLenFlowListName + 1 },
  { uudst::policingprofile_entry::kDbiSequenceNum, CFG_MATCH_KEY, offsetof(
      key_policingprofile_entry_t, sequence_num),
  uud::kDalUint8, 1 },
  { uudst::policingprofile_entry::kDbiFlowlist, CFG_MATCH_KEY, offsetof(
      key_rename_vnode_info_t, new_flowlist_name),
  uud::kDalChar, kMaxLenFlowListName + 1 },
  { uudst::policingprofile_entry::kDbiFlags, CFG_INPUT_KEY,
    offsetof(key_user_data_t, flags), uud::kDalUint8, 1 } };

BindInfo PolicingProfileEntryMoMgr::rename_flowlist_pp_entry_ctrl_tbl[] = {
  { uudst::policingprofile_entry_ctrlr::kDbiPolicingProfileName, CFG_MATCH_KEY,
    offsetof(key_policingprofile_entry_t,
        policingprofile_key.policingprofile_name),
    uud::kDalChar, kMaxLenFlowListName + 1 },
  { uudst::policingprofile_entry_ctrlr::kDbiSequenceNum, CFG_MATCH_KEY,
    offsetof(key_policingprofile_entry_t, sequence_num), uud::kDalUint8, 1 },
  { uudst::policingprofile_entry_ctrlr::kDbiCtrlrName, CFG_MATCH_KEY, offsetof(
      key_user_data_t, ctrlr_id),
  uud::kDalChar, kMaxLenCtrlrId + 1 },
  { uudst::policingprofile_entry_ctrlr::kDbiFlags, CFG_INPUT_KEY, offsetof(
      key_user_data_t, flags),
  uud::kDalUint8, 1 } };

bool PolicingProfileEntryMoMgr::GetRenameKeyBindInfo(unc_key_type_t key_type,
    BindInfo *&binfo,
    int &nattr,
    MoMgrTables tbl) {
  switch (key_type) {
    case UNC_KT_POLICING_PROFILE_ENTRY:
      if (MAINTBL == tbl) {
        nattr = NUM_PP_KEY_MAIN_COL;
        binfo = rename_policingprofile_entry_main_tbl;
      } else if (CTRLRTBL == tbl) {
        nattr = NUM_PP_KEY_CTRLR_COL;
        binfo = rename_policingprofile_entry_ctrl_tbl;
      } else {
        return PFC_FALSE;
      }
      break;
    case UNC_KT_FLOWLIST:
      if (MAINTBL == tbl) {
        nattr = NUM_FL_KEY_MAIN_COL;
        binfo = rename_flowlist_pp_entry_main_tbl;
      } else if (CTRLRTBL == tbl) {
        nattr = NUM_FL_KEY_CTRLR_COL;
        binfo = rename_flowlist_pp_entry_ctrl_tbl;
      } else {
        return PFC_FALSE;
      }
      break;
    default:
      return PFC_FALSE;
  }
  return PFC_TRUE;
}

PolicingProfileEntryMoMgr::PolicingProfileEntryMoMgr() :  MoMgrImpl() {
  UPLL_FUNC_TRACE;
  // Rename operation is not support for this KT
  ntable = MAX_MOMGR_TBLS;
  table = new Table *[ntable];

  /* For Main Table */
  table[MAINTBL]= new Table(uudst::kDbiPolicingProfileEntryTbl,
      UNC_KT_POLICING_PROFILE_ENTRY, policingprofileentry_bind_info,
      IpctSt::kIpcStKeyPolicingprofileEntry,
      IpctSt::kIpcStValPolicingprofileEntry,
      uudst::policingprofile_entry::kDbiPolicingProfileEntryNumCols);

  /* For Rename Table */
  table[RENAMETBL] = NULL;

  /* For Controller Table */
  table[CTRLRTBL] = new Table(uudst::kDbiPolicingProfileEntryCtrlrTbl,
      UNC_KT_POLICING_PROFILE_ENTRY, policingprofileentry_controller_bind_info,
      IpctSt::kIpcStKeyPolicingprofileEntry, IpctSt::kIpcInvalidStNum,
      uudst::policingprofile_entry_ctrlr::kDbiPolicingProfileEntryCtrlrNumCols);

  nchild = 0;
  child = NULL;
};


upll_rc_t PolicingProfileEntryMoMgr::AllocVal(ConfigVal *&ck_val,
    upll_keytype_datatype_t dt_type,
    MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  void *val; /* *ck_nxtval;*/
  int array_size = 0;
  if (ck_val != NULL) {
    UPLL_LOG_DEBUG("No Need to allocate memory :AllocVal Failed-");
    return UPLL_RC_ERR_GENERIC;
  }
  switch (tbl) {
    case MAINTBL:
      val = reinterpret_cast<void *>
          (ConfigKeyVal::Malloc(sizeof(val_policingprofile_entry_t)));
      array_size =
          (sizeof(reinterpret_cast
                 <val_policingprofile_entry_ctrl*>(val)->valid)/
           sizeof(reinterpret_cast
                  <val_policingprofile_entry_ctrl *>(val)->valid[0]));
      for (int index = 0; index < array_size; index++) {
        reinterpret_cast<val_policingprofile_entry_ctrl *>(val)->valid[index] =
          UNC_VF_INVALID;
      }
      ck_val = new ConfigVal(IpctSt::kIpcStValPolicingprofileEntry, val);
      break;
    case CTRLRTBL:
      val = reinterpret_cast<void *>(ConfigKeyVal::Malloc(
            sizeof(val_policingprofile_entry_ctrl_t)));
      array_size =
          (sizeof(reinterpret_cast
                 <val_policingprofile_entry_ctrl*>(val)->valid)/
          sizeof(reinterpret_cast
                 <val_policingprofile_entry_ctrl*>(val)->valid[0]));

      for (int index = 0; index < array_size; index++) {
        reinterpret_cast<val_policingprofile_entry_ctrl *>(val)->valid[index] =
          UNC_VF_INVALID;
      }
      ck_val = new ConfigVal(IpctSt::kIpcInvalidStNum, val);
      break;
    default:
      return UPLL_RC_ERR_GENERIC;
  }
  UPLL_LOG_DEBUG(
      " PolicingProfileEntryMoMgr::Allocation of Memory is successful ");
  return UPLL_RC_SUCCESS;
}

upll_rc_t PolicingProfileEntryMoMgr::GetChildConfigKey(
    ConfigKeyVal *&okey, ConfigKeyVal *parent_key) {
  UPLL_FUNC_TRACE;
  key_policingprofile_entry_t *key_ppe = NULL;
  void *pkey = NULL;

  if (parent_key == NULL) {
    UPLL_LOG_DEBUG("parent key is not NULL");
    key_ppe = reinterpret_cast<key_policingprofile_entry_t *>
        (ConfigKeyVal::Malloc(sizeof(key_policingprofile_entry_t)));

    okey = new ConfigKeyVal(UNC_KT_POLICING_PROFILE_ENTRY,
                            IpctSt::kIpcStKeyPolicingprofileEntry,
                            key_ppe, NULL);
    UPLL_LOG_DEBUG(
        " PolicingProfileEntryMoMgr::Alolocation of Memory is successful ");
    return UPLL_RC_SUCCESS;
  } else {
    UPLL_LOG_DEBUG("parent ckey is not NULL");
    pkey = parent_key->get_key();
  }
  if (NULL == pkey) {
    UPLL_LOG_DEBUG("error Generated::Key type not supported :-");
//    if(key_ppe) free(key_ppe);
    return UPLL_RC_ERR_GENERIC;
  }
  if (okey) {
    UPLL_LOG_DEBUG("Okey not NULL");
    if (okey->get_key_type() != UNC_KT_POLICING_PROFILE_ENTRY)
      return UPLL_RC_ERR_GENERIC;
    key_ppe = reinterpret_cast<key_policingprofile_entry_t *>(okey->get_key());
  } else {
    UPLL_LOG_DEBUG("Okey NULL");
    key_ppe = reinterpret_cast<key_policingprofile_entry_t *>
        (ConfigKeyVal::Malloc(sizeof(key_policingprofile_entry_t)));
  }
  switch (parent_key->get_key_type()) {
    case UNC_KT_POLICING_PROFILE:
      uuu::upll_strncpy(key_ppe->policingprofile_key.policingprofile_name,
                        reinterpret_cast<key_policingprofile_t *>
                        (pkey)->policingprofile_name,
                        (kMaxLenPolicingProfileName + 1));
      break;
    case UNC_KT_POLICING_PROFILE_ENTRY:
      UPLL_LOG_DEBUG("Inside ppe case");
      uuu::upll_strncpy(key_ppe->policingprofile_key.policingprofile_name,
                        reinterpret_cast<key_policingprofile_entry_t *>
                        (pkey)->policingprofile_key.policingprofile_name,
                        (kMaxLenPolicingProfileName + 1));
      key_ppe->sequence_num = reinterpret_cast<key_policingprofile_entry_t *>
          (pkey)->sequence_num;
      break;
    default:
      UPLL_LOG_DEBUG("Inside default case");
      if (key_ppe != NULL) free(key_ppe);
      return UPLL_RC_ERR_GENERIC;
      break;
  }

  UPLL_LOG_DEBUG("GetChildConfigKey  %s",
                 key_ppe->policingprofile_key.policingprofile_name);
  UPLL_LOG_DEBUG("GetChildConfigKey  %d",
                 key_ppe->sequence_num);

  if (!okey)
    okey = new ConfigKeyVal(UNC_KT_POLICING_PROFILE_ENTRY,
                            IpctSt::kIpcStKeyPolicingprofileEntry,
                            key_ppe, NULL);

  SET_USER_DATA(okey, parent_key);
  UPLL_LOG_DEBUG(
      " PolicingProfileEntryMoMgr:: Memory allocation Successful ");
  return UPLL_RC_SUCCESS;
}

upll_rc_t PolicingProfileEntryMoMgr::GetRenamedUncKey(
    ConfigKeyVal *ctrlr_key, upll_keytype_datatype_t dt_type, DalDmlIntf *dmi,
    uint8_t *ctrlr_id) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_ERR_GENERIC;
  ConfigKeyVal *unc_key = NULL;
  if (NULL == ctrlr_key) {
    UPLL_LOG_DEBUG(" PolicingProfileEntryMoMgr::GetRenamedUncKey Failed.");
    return result_code;
  }
  key_policingprofile_entry_t *ctrlr_policingprofile_entry_key =
    reinterpret_cast<key_policingprofile_entry_t *>(ctrlr_key->get_key());
  if (NULL == ctrlr_policingprofile_entry_key) {
    UPLL_LOG_DEBUG(" PolicingProfileEntryMoMgr::GetRenamedUncKey Failed.");
    return result_code;
  }
  DbSubOp dbop = { kOpReadSingle, kOpMatchCtrlr, kOpInOutNone };
  val_rename_policingprofile_t *rename_policingprofile =
    reinterpret_cast<val_rename_policingprofile_t *>(ConfigKeyVal::Malloc(
          sizeof(val_rename_policingprofile_t)));
  uuu::upll_strncpy(
      rename_policingprofile->policingprofile_newname,
      ctrlr_policingprofile_entry_key->policingprofile_key.policingprofile_name,
      (kMaxLenPolicingProfileName + 1));
  rename_policingprofile->valid[UPLL_IDX_RENAME_PROFILE_RPP] = UNC_VF_VALID;
  MoMgrImpl *mgr =
    reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>(GetMoManager(
            UNC_KT_POLICING_PROFILE)));
  result_code = mgr->GetChildConfigKey(unc_key, NULL);
  if (UPLL_RC_SUCCESS != result_code) {
    free(rename_policingprofile);
    return result_code;
  }
  unc_key->AppendCfgVal(IpctSt::kIpcStValRenamePolicingprofile,
      rename_policingprofile);
  if (ctrlr_id)
    SET_USER_DATA_CTRLR(unc_key, ctrlr_id);

  result_code = mgr->ReadConfigDB(unc_key, dt_type, UNC_OP_READ, dbop, dmi,
      RENAMETBL);
  if (UPLL_RC_SUCCESS == result_code) {
    key_policingprofile_entry_t *policingprofile_entry =
      reinterpret_cast<key_policingprofile_entry_t *>(unc_key->get_key());
    uuu::upll_strncpy(
      ctrlr_policingprofile_entry_key->policingprofile_key.policingprofile_name,
      policingprofile_entry->policingprofile_key.policingprofile_name,
      (kMaxLenPolicingProfileName + 1));
  } else if (UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
    UPLL_LOG_DEBUG("ReadConfigDB failed %d", result_code);
    return result_code;
  }
  val_policingprofile_entry_t *val_ppe =
    reinterpret_cast<val_policingprofile_entry_t *>(GetVal(ctrlr_key));
  if (UNC_VF_VALID != val_ppe->valid[UPLL_IDX_FLOWLIST_PPE ]) {
    return UPLL_RC_SUCCESS;
  }
  ConfigKeyVal *fl_ckv = NULL;
  val_rename_flowlist_t *rename_flowlist =
    reinterpret_cast<val_rename_flowlist_t *>(ConfigKeyVal::Malloc(
          sizeof(val_rename_flowlist_t)));

  uuu::upll_strncpy(rename_flowlist->flowlist_newname,
                    val_ppe->flowlist,
                    (kMaxLenFlowListName + 1));
  rename_flowlist->valid[UPLL_IDX_RENAME_FLOWLIST_RFL] = UNC_VF_VALID;
  MoMgrImpl *mgr_fl =
    reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>(GetMoManager(
            UNC_KT_FLOWLIST)));
  result_code = mgr_fl->GetChildConfigKey(fl_ckv, NULL);
  if (UPLL_RC_SUCCESS != result_code) {
    return result_code;
  }
  unc_key->AppendCfgVal(IpctSt::kIpcStValRenameFlowlist,
      rename_flowlist);
  if (ctrlr_id)
    SET_USER_DATA_CTRLR(fl_ckv, ctrlr_id);
  result_code = mgr->ReadConfigDB(fl_ckv, dt_type, UNC_OP_READ, dbop, dmi,
      RENAMETBL);
  if (UPLL_RC_SUCCESS == result_code) {
    key_flowlist_t *key_flowlist =
      reinterpret_cast<key_flowlist_t *>(fl_ckv->get_key());
    uuu::upll_strncpy(val_ppe->flowlist,
                      key_flowlist->flowlist_name,
                      (kMaxLenFlowListName + 1));
  } else if (UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
    UPLL_LOG_DEBUG("ReadConfigDB failed %d", result_code);
    return result_code;
  }
  delete unc_key;
  delete fl_ckv;
  unc_key = fl_ckv = NULL;
  UPLL_LOG_DEBUG(" PolicingProfileEntryMoMgr::GetRenamedUncKey Failed : %d",
      result_code);
  return result_code;
}

upll_rc_t PolicingProfileEntryMoMgr::DupConfigKeyVal(ConfigKeyVal *&okey,
    ConfigKeyVal *&req,
    MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  if (NULL == req) {
    return UPLL_RC_ERR_GENERIC;
  }
  if (NULL != okey) {
    return UPLL_RC_ERR_GENERIC;
  }
  if (req->get_key_type() != UNC_KT_POLICING_PROFILE_ENTRY) {
    UPLL_LOG_DEBUG("error Generated::Key type not supported :-");
    return UPLL_RC_ERR_GENERIC;
  }
  if (NULL == (req->get_key())) {
    return UPLL_RC_ERR_GENERIC;
  }
  ConfigVal *tmp1 = NULL, *tmp = (req)->get_cfg_val();
  if (tmp) {
    if (tbl == MAINTBL) {
      val_policingprofile_entry_t *ival =
        reinterpret_cast<val_policingprofile_entry_t *>(GetVal(req));
      if (NULL != ival) {
        val_policingprofile_entry_t *policingprofile_entry_val =
          reinterpret_cast<val_policingprofile_entry_t *>
          (ConfigKeyVal::Malloc(sizeof(val_policingprofile_entry_t)));
        memcpy(policingprofile_entry_val, ival,
            sizeof(val_policingprofile_entry_t));
        tmp1 = new ConfigVal(IpctSt::kIpcStValPolicingprofileEntry,
            policingprofile_entry_val);
       // free(policingprofile_entry_val);  //  TODO(check whether req)
      }
    } else if (CTRLRTBL == tbl) {
      val_policingprofile_entry_ctrl_t *ival =
        reinterpret_cast<val_policingprofile_entry_ctrl_t *>(GetVal(req));
      if (NULL != ival) {
        val_policingprofile_entry_ctrl_t *entry_val =
          reinterpret_cast<val_policingprofile_entry_ctrl_t *>
          (ConfigKeyVal::Malloc(sizeof(val_policingprofile_entry_ctrl_t)));
        memcpy(entry_val, ival, sizeof(val_policingprofile_entry_ctrl_t));
        tmp1 = new ConfigVal(IpctSt::kIpcInvalidStNum, entry_val);
      //  free(entry_val);  //  TODO(check whether req)
      }
    } else {
      return UPLL_RC_ERR_GENERIC;
    }

    if (NULL == tmp1) {
      UPLL_LOG_DEBUG("Null Pointer:");
      return UPLL_RC_ERR_GENERIC;
    }
    tmp1->set_user_data(tmp->get_user_data());
  }
  key_policingprofile_entry_t *tkey =
      reinterpret_cast<key_policingprofile_entry_t *>(req->get_key());
  if (NULL == tkey) {
    delete tmp1;  // COV RESOURCE LEAK
    return UPLL_RC_ERR_GENERIC;
  }

  key_policingprofile_entry_t *policingprofile_entry =
    reinterpret_cast<key_policingprofile_entry_t *>
    (ConfigKeyVal::Malloc(sizeof(key_policingprofile_entry_t)));
  memcpy(policingprofile_entry, tkey,
         sizeof(key_policingprofile_entry_t));

  okey = new ConfigKeyVal(UNC_KT_POLICING_PROFILE_ENTRY,
      IpctSt::kIpcStKeyPolicingprofileEntry,
      policingprofile_entry, tmp1);
  SET_USER_DATA(okey, req);
  //  free(policingprofile_entry);  //TODO (check whether req)
  UPLL_LOG_DEBUG(" PolicingProfileEntryMoMgr::Successful Compilation ");
  return UPLL_RC_SUCCESS;
}

upll_rc_t PolicingProfileEntryMoMgr::UpdateAuditConfigStatus(
    unc_keytype_configstatus_t cs_status,
    uuc::UpdateCtrlrPhase phase,
    ConfigKeyVal *&ckv_running) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  val_policingprofile_entry_t *val;
  val = (ckv_running != NULL) ? reinterpret_cast
      <val_policingprofile_entry_t *>(GetVal(
          ckv_running)) :  NULL;
  if (NULL == val) {
    return UPLL_RC_ERR_GENERIC;
  }
  if (uuc::kUpllUcpCreate == phase) val->cs_row_status = cs_status;
  for (unsigned int loop = 0; loop < sizeof(val->valid)/sizeof(val->valid[0]);
      ++loop) {
    if (cs_status == UNC_CS_INVALID && UNC_VF_VALID ==
        val->valid[loop]) val->cs_attr[loop] = cs_status;
    else
      val->cs_attr[loop] = cs_status;
  }
  return result_code;
}

bool PolicingProfileEntryMoMgr::CompareKey(ConfigKeyVal *key1,
    ConfigKeyVal *key2) {
  UPLL_FUNC_TRACE;
  bool match = false;
  if (NULL == key1 || NULL == key2) {
    return match;
  }
  if (key1->get_key_type() != UNC_KT_POLICING_PROFILE_ENTRY ||
      key2->get_key_type() != UNC_KT_POLICING_PROFILE_ENTRY) {
    UPLL_LOG_DEBUG("PolicingProfileEntryMoMgr::Invalid Key type");
    return match;
  }
  key_policingprofile_entry_t *policingprofile_entry_key1,
                              *policingprofile_entry_key2;
  policingprofile_entry_key1 =
    reinterpret_cast<key_policingprofile_entry_t *>(key1->get_key());
  policingprofile_entry_key2 =
    reinterpret_cast<key_policingprofile_entry_t *>(key2->get_key());

  if (NULL == policingprofile_entry_key1 ||
     NULL == policingprofile_entry_key2) {
    return match;
  }
  if (strcmp(
        reinterpret_cast<const char *>(policingprofile_entry_key1->
            policingprofile_key.policingprofile_name),
        reinterpret_cast<const char *>(policingprofile_entry_key2->
            policingprofile_key.policingprofile_name)) == 0 &&
      policingprofile_entry_key1->sequence_num ==
      policingprofile_entry_key2->sequence_num) {
    match = true;
    UPLL_LOG_DEBUG(" PolicingProfileEntryMoMgr::CompareKey,"
                   " Both Keys are same");
  }
  return match;
}

upll_rc_t PolicingProfileEntryMoMgr::MergeValidate(unc_key_type_t keytype,
    const char *ctrlr_id,
    ConfigKeyVal *ikey,
    DalDmlIntf *dmi) {
  // No Implementation for Merge Validate.
  UPLL_FUNC_TRACE;
  UPLL_LOG_DEBUG(
      " PolicingProfileEntryMoMgr::Rename Not required:: successful ");
  return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_KT;
}

upll_rc_t PolicingProfileEntryMoMgr::RenameMo(IpcReqRespHeader *req,
    ConfigKeyVal *ikey,
    DalDmlIntf *dmi,
    const char *ctrlr_id) {
  // No Implementation for RenameMo.
  UPLL_FUNC_TRACE;
  UPLL_LOG_DEBUG(
      " PolicingProfileEntryMoMgr::Alolocation of Memory is successful ");
  return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_KT;
}

upll_rc_t PolicingProfileEntryMoMgr::IsReferenced(
    ConfigKeyVal *ikey, upll_keytype_datatype_t dt_type, DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *okey = NULL;
  ConfigKeyVal *ppe_okey = NULL;
  result_code = GetChildConfigKey(ppe_okey, NULL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetChildConfigKey failed ");
    return result_code;
  }
  result_code = UpdateConfigDB(ikey,
                  dt_type, UNC_OP_READ, dmi, CTRLRTBL);
  if (UPLL_RC_ERR_INSTANCE_EXISTS == result_code) {
    return UPLL_RC_ERR_CFG_SEMANTIC;
  } else if (UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
    delete ppe_okey;
    return result_code;
  }
  // Change to GetParentKey
  result_code = GetPolicingprofileKeyVal(okey, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Memory is not allocated for okey %d", result_code);
    return result_code;
  }
  MoMgrImpl *mgr =
    reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>(GetMoManager(
            UNC_KT_POLICING_PROFILE)));
  if (!mgr) {
    UPLL_LOG_DEBUG("Invalid mgr param\n");
    delete okey;  // RESOURCE LEAK
    return UPLL_RC_ERR_GENERIC;
  }

  result_code = mgr->IsReferenced(okey, UPLL_DT_CANDIDATE, dmi);

  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("IsReferenced id Faliled %d", result_code);
    delete okey;
    return result_code;
  }
  delete okey;
  return result_code;
}

upll_rc_t PolicingProfileEntryMoMgr::GetPolicingprofileKeyVal(
    ConfigKeyVal *&okey, ConfigKeyVal *&ikey) {
  UPLL_FUNC_TRACE;

  key_policingprofile_entry_t *key_policingprofile_entry =
    reinterpret_cast<key_policingprofile_entry_t *>(ikey->get_key());

  if (NULL == key_policingprofile_entry) {
    UPLL_LOG_DEBUG("PolicingProfileEntryMoMgr:: Invalid key");
    return UPLL_RC_ERR_GENERIC;
  }

  key_policingprofile_t *key_policingprofile =
    reinterpret_cast<key_policingprofile_t *>(ConfigKeyVal::Malloc(
          sizeof(key_policingprofile_t)));
  uuu::upll_strncpy(
      key_policingprofile->policingprofile_name,
      key_policingprofile_entry->policingprofile_key.policingprofile_name,
      (kMaxLenPolicingProfileName + 1));

  okey = new ConfigKeyVal(UNC_KT_POLICING_PROFILE,
      IpctSt::kIpcStKeyPolicingprofile, key_policingprofile,
      NULL);

  if (okey) {
    SET_USER_DATA(okey, ikey);
  } else {
    free(key_policingprofile);
    return UPLL_RC_ERR_GENERIC;
  }
  UPLL_LOG_DEBUG("GetObjectConfigVal Successfull");
  return UPLL_RC_SUCCESS;
}

upll_rc_t PolicingProfileEntryMoMgr::GetRenamedControllerKey(
    ConfigKeyVal *&ikey, upll_keytype_datatype_t dt_type, DalDmlIntf *dmi,
    controller_domain *ctrlr_dom) {
  UPLL_FUNC_TRACE;
  if (!ikey) {
    return UPLL_RC_ERR_GENERIC;
  }
  upll_rc_t result_code = UPLL_RC_ERR_GENERIC;
  ConfigKeyVal *okey = NULL;
  uint8_t rename = 0, rename_fl = 0;
  DbSubOp dbop = { kOpReadSingle, kOpMatchCtrlr, kOpInOutFlag };
  uint8_t *ctrlr_id = ctrlr_dom->ctrlr;
  IsRenamed(ikey, dt_type, dmi, rename);
  if (!rename) {
    return UPLL_RC_SUCCESS;
  }
  /* PolicingProfile_name is  renamed */
  key_policingprofile_entry_t *ctrlr_key =
    reinterpret_cast<key_policingprofile_entry_t *>(ConfigKeyVal::Malloc(
          sizeof(key_policingprofile_entry_t)));

  if (rename & PP_RENAME_FLAG) {
    result_code = GetChildConfigKey(okey, ikey);
    if (UPLL_RC_SUCCESS != result_code) {
      return UPLL_RC_ERR_GENERIC;
    }
    if (NULL != ctrlr_id) {
      SET_USER_DATA_CTRLR(okey, ctrlr_id);
    }
    /* ctrlr_name */
    result_code = ReadConfigDB(okey, dt_type, UNC_OP_READ, dbop, dmi,
                  RENAMETBL);  // COVERITY CHECKED RETURN
    if (result_code != UPLL_RC_SUCCESS) {
      free(ctrlr_key);  // COV RESOURCE LEAK FIX
      return result_code;
    }
    /* Null check missing */
    val_rename_policingprofile_t *rename_val =
      reinterpret_cast<val_rename_policingprofile_t *>(GetVal(okey));

    if (!rename_val
        || (rename_val->valid[UPLL_IDX_RENAME_PROFILE_RPP] != UNC_VF_VALID)) {
      UPLL_LOG_DEBUG("rename failed :-");
      return UPLL_RC_ERR_GENERIC;
    }
    uuu::upll_strncpy(
        ctrlr_key->policingprofile_key.policingprofile_name,
        rename_val->policingprofile_newname,
        (kMaxLenPolicingProfileName + 1));
    delete okey;
  }
  free(ikey->get_key());
  ikey->SetKey(IpctSt::kIpcStKeyPolicingprofileEntry,
      reinterpret_cast<void *>(ctrlr_key));
  /* rename flowlist */
  val_policingprofile_entry_t *val_policingprofile_entry =
    reinterpret_cast<val_policingprofile_entry_t *>(GetVal(ikey));
  key_flowlist_t *key_flowlist =
    reinterpret_cast<key_flowlist_t *>(GetVal(okey));

  uuu::upll_strncpy(key_flowlist->flowlist_name,
                    val_policingprofile_entry->flowlist,
                    (kMaxLenFlowListName + 1));

  MoMgrImpl *mgr =
    reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>(GetMoManager(
            UNC_KT_FLOWLIST)));

  IsRenamed(okey, dt_type, dmi, rename_fl);
  if (rename_fl == 0) {
    delete okey;
    return UPLL_RC_SUCCESS;
  }
  SET_USER_DATA_CTRLR(okey, ctrlr_id);
  result_code = mgr->ReadConfigDB(okey, dt_type, UNC_OP_READ, dbop, dmi,
      RENAMETBL);
  if (result_code != UPLL_RC_SUCCESS) {
    return result_code;
  }
  val_rename_flowlist_t *rename_flowlist =
    reinterpret_cast<val_rename_flowlist_t *>(GetVal(okey));
  if (rename_fl & FL_RENAME_FLAG) { /* flowlist renamed */
    uuu::upll_strncpy(val_policingprofile_entry->flowlist,
                      rename_flowlist->flowlist_newname,
                      (kMaxLenFlowListName + 1));
  }
  // TODO(Author) has to fi th eissue DEAD CODE
  // rename = rename_fl || FLT_RENAME_FLAG;
  SET_USER_DATA_FLAGS(okey, rename);
  /* GetControlerKey(*ikey); */
  free(ctrlr_key);
  UPLL_LOG_DEBUG(
      " PolicingProfileEntryMoMgr::GetRenamedControllerKey is successful ");
  return UPLL_RC_SUCCESS;
}

upll_rc_t PolicingProfileEntryMoMgr::TxUpdateController(
    unc_key_type_t keytype, uint32_t session_id, uint32_t config_id,
    uuc::UpdateCtrlrPhase phase,
    set<string> *affected_ctrlr_set,
    DalDmlIntf *dmi,
    ConfigKeyVal **err_ckv) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  DalResultCode dal_result = uud::kDalRcSuccess;
  ConfigKeyVal *req = NULL, *nreq = NULL, *ck_main = NULL;
  controller_domain ctrlr_dom;  // UNINIT
  ctrlr_dom.ctrlr = NULL;
  ctrlr_dom.domain = NULL;
  DalCursor *dal_cursor_handle;
  IpcResponse resp;
  memset(&resp, 0, sizeof(resp));
  if (uuc::kUpllUcpDelete == phase) {
    UPLL_LOG_DEBUG("Delete phase 1");
    return UPLL_RC_SUCCESS;
  }
  if (uuc::kUpllUcpDelete2 == phase) UPLL_LOG_DEBUG("Delete phase 2");
  unc_keytype_operation_t op = (phase == uuc::kUpllUcpCreate)?UNC_OP_CREATE:
      ((phase == uuc::kUpllUcpUpdate)?UNC_OP_UPDATE:
       ((phase == uuc::kUpllUcpDelete2)?UNC_OP_DELETE:UNC_OP_INVALID));
  switch (op) {
    case UNC_OP_CREATE:
    case UNC_OP_DELETE:
      result_code = DiffConfigDB(UPLL_DT_CANDIDATE, UPLL_DT_RUNNING, op,
                                 req, nreq, &dal_cursor_handle, dmi, CTRLRTBL);
      break;
    case UNC_OP_UPDATE:
      result_code = DiffConfigDB(UPLL_DT_CANDIDATE, UPLL_DT_RUNNING, op,
                                 req, nreq, &dal_cursor_handle, dmi, MAINTBL);
      break;
    default:
      UPLL_LOG_TRACE("Invalid operation \n");
      return UPLL_RC_ERR_GENERIC;
  }
  resp.header.clnt_sess_id = session_id;
  resp.header.config_id = config_id;
  while (result_code == UPLL_RC_SUCCESS) {
    /* Get Next Record */
    dal_result = dmi->GetNextRecord(dal_cursor_handle);
    result_code = DalToUpllResCode(dal_result);
    if (result_code != UPLL_RC_SUCCESS)
      break;
    ck_main = NULL;
    if ((op == UNC_OP_CREATE) || (op == UNC_OP_DELETE)) {
      result_code = DupConfigKeyVal(ck_main, req, MAINTBL);
      if (result_code != UPLL_RC_SUCCESS)
        return result_code;

      GET_USER_DATA_CTRLR_DOMAIN(req, ctrlr_dom);
      UPLL_LOG_DEBUG("ctrlr : %s; domain : %s", ctrlr_dom.ctrlr,
                     ctrlr_dom.domain);
      if (ctrlr_dom.ctrlr == NULL) {
        UPLL_LOG_DEBUG("Invalid controller");
        DELETE_IF_NOT_NULL(ck_main);
        ck_main = NULL;
        return UPLL_RC_ERR_GENERIC;
      }
      result_code = TxUpdateProcess(ck_main, &resp,
                                    op, dmi, &ctrlr_dom);
      affected_ctrlr_set->insert((const char *)ctrlr_dom.ctrlr);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("TxUpdateProcess Returns error %d", result_code);
        *err_ckv = resp.ckv_data;
        DELETE_IF_NOT_NULL(ck_main);
        ck_main = NULL;
        break;
      }
    } else if (op == UNC_OP_UPDATE) {
      UPLL_LOG_TRACE(" Not supported operation \n");
      return UPLL_RC_SUCCESS;
      #if 0
      ConfigKeyVal *ck_ctrlr = NULL;

      result_code = DupConfigKeyVal(ck_main, req, MAINTBL);
      if (result_code != UPLL_RC_SUCCESS)
        return result_code;
      /*
         result_code = ValidateCapability(&(ipc_req.header), ck_main);
         if (result_code != UPLL_RC_SUCCESS) {
         DELETE_IF_NOT_NULL(ck_main);
         return result_code;
         }
         */
      result_code = GetChildConfigKey(ck_ctrlr, req);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("GetChildConfigKey failed %d", result_code)
        delete ck_main;
        return result_code;
      }
      result_code = GetControllerSpan(ck_ctrlr, UPLL_DT_CANDIDATE, dmi);
      if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
        UPLL_LOG_DEBUG("GetControllerSpan no instance");
        delete ck_ctrlr;
        ck_ctrlr = NULL;
        continue;
      } else if (UPLL_RC_SUCCESS != result_code) {
        delete ck_ctrlr;
        ck_ctrlr = NULL;
        UPLL_LOG_DEBUG("GetControllerSpan failed %d", result_code);
        return result_code;
      }

      for (ConfigKeyVal *tmp = ck_ctrlr; tmp != NULL;
           tmp = tmp->get_next_cfg_key_val()) {
        ctrlr_dom.ctrlr = NULL;
        GET_USER_DATA_CTRLR_DOMAIN(tmp, ctrlr_dom);
        UPLL_LOG_DEBUG("ctrlr id - %s ", ctrlr_dom.ctrlr);
        if (ctrlr_dom.ctrlr == NULL) {
          UPLL_LOG_DEBUG("Invalid controller");
          return UPLL_RC_ERR_GENERIC;
        }
        UPLL_LOG_DEBUG("ctrlr id - %s ", ctrlr_dom.ctrlr);
        result_code = TxUpdateProcess(ck_main, &resp, op, dmi, &ctrlr_dom);
        affected_ctrlr_set->insert(reinterpret_cast<const char *>
                                   (ctrlr_dom.ctrlr));
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("TxUpdate Process returns with %d\n", result_code);
          *err_ckv = resp.ckv_data;
          break;
        }
      }
      DELETE_IF_NOT_NULL(ck_ctrlr);
      ck_ctrlr = NULL;
      #endif
    }
    DELETE_IF_NOT_NULL(ck_main);
    ck_main = NULL;
  }
  DELETE_IF_NOT_NULL(nreq);
  nreq = NULL;
  DELETE_IF_NOT_NULL(req);
  req = NULL;
  if (dal_cursor_handle) {
    dmi->CloseCursor(dal_cursor_handle, true);
    dal_cursor_handle = NULL;
  }
  result_code = (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE)?
      UPLL_RC_SUCCESS:result_code;
  return result_code;
}

upll_rc_t PolicingProfileEntryMoMgr::GetControllerSpan(
    ConfigKeyVal *ikey, upll_keytype_datatype_t dt_type, DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code;
  DbSubOp dbop = { kOpReadMultiple, kOpMatchNone, kOpInOutCtrlr };

  result_code = ReadConfigDB(ikey, dt_type, UNC_OP_READ, dbop, dmi, CTRLRTBL);
  UPLL_LOG_DEBUG("GetControllerSpan successful:- %d", result_code);
  return result_code;
}

upll_rc_t PolicingProfileEntryMoMgr::TxUpdateProcess(ConfigKeyVal *ck_main,
                                              IpcResponse *ipc_resp,
                                              unc_keytype_operation_t op,
                                              DalDmlIntf *dmi,
                                              controller_domain *ctrlr_dom) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code;
  /* read from main table */
  ConfigKeyVal *dup_ckmain = ck_main;
  if (op == UNC_OP_CREATE) {
    dup_ckmain = NULL;
    result_code = GetChildConfigKey(dup_ckmain, ck_main);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Returning error %d\n", result_code);
      if (dup_ckmain) delete dup_ckmain;
      return result_code;
    }
    DbSubOp dbop = {kOpReadSingle, kOpMatchNone, kOpInOutNone};
    result_code = ReadConfigDB(dup_ckmain, UPLL_DT_CANDIDATE,
                               UNC_OP_READ, dbop, dmi, MAINTBL);
    if (result_code != UPLL_RC_SUCCESS) {
      string s(dup_ckmain->ToStrAll());
      UPLL_LOG_INFO("%s Policingprofile Entry read failed %d",
          s.c_str(), result_code);
      delete dup_ckmain;
      return result_code;
    }
  }
  /* Get renamed key if key is renamed */
  result_code =  GetRenamedControllerKey(dup_ckmain, UPLL_DT_CANDIDATE,
                                         dmi, ctrlr_dom);
  if (result_code != UPLL_RC_SUCCESS) {
    return result_code;
  }
  result_code = SendIpcReq(ipc_resp->header.clnt_sess_id,
                           ipc_resp->header.config_id, op,
                           UPLL_DT_CANDIDATE, dup_ckmain, ctrlr_dom, ipc_resp);
  if (result_code == UPLL_RC_ERR_RESOURCE_DISCONNECTED) {
    UPLL_LOG_DEBUG("Controller disconnected\n");
    result_code = UPLL_RC_SUCCESS;
  }
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("IpcSend failed %d\n", result_code);
  }
  if ((op == UNC_OP_CREATE) && dup_ckmain) {
    delete dup_ckmain;
    dup_ckmain = NULL;
  }
  return result_code;
}

upll_rc_t PolicingProfileEntryMoMgr::FilterAttributes(
        ConfigKeyVal *ckv_main, ConfigKeyVal *ckv_ctrlr) {
  UPLL_FUNC_TRACE;
  val_policingprofile_entry_t *val_main = reinterpret_cast
      <val_policingprofile_entry_t *>(GetVal(ckv_main));
  val_policingprofile_entry_ctrl_t *val_ctrlr = reinterpret_cast
      <val_policingprofile_entry_ctrl_t *>(GetVal(ckv_main));
  if (NULL == val_main || NULL == val_ctrlr) {
    return UPLL_RC_ERR_GENERIC;
  }
  for (unsigned int loop = 0;
        loop < sizeof(val_ctrlr->valid) /
        sizeof(val_ctrlr->valid[0]); ++loop) {
    val_main->valid[loop] = val_ctrlr->valid[loop];
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t PolicingProfileEntryMoMgr::TxCopyCandidateToRunning(
    unc_key_type_t keytype, CtrlrCommitStatusList *ctrlr_commit_status,
    DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  DalResultCode db_result;
  unc_keytype_operation_t op[] = {  UNC_OP_DELETE,UNC_OP_CREATE,
                                   UNC_OP_UPDATE };
  int nop = sizeof(op) / sizeof(op[0]);
  ConfigKeyVal *ppe_key = NULL, *req = NULL, *nreq = NULL;
  DalCursor *cfg1_cursor = NULL;
  uint8_t *ctrlr_id = NULL;
#if 0
  IpcReqRespHeader *req_header = reinterpret_cast<IpcReqRespHeader *>
      (ConfigKeyVal::Malloc(sizeof(IpcReqRespHeader)));
#endif
  map<string, int> ctrlr_result;
  CtrlrCommitStatusList::iterator ccsListItr;
  CtrlrCommitStatus *ccStatusPtr;

  if ((ctrlr_commit_status == NULL) || (dmi == NULL)) {
    UPLL_LOG_DEBUG(
        " PolicingProfileEntryMoMgr::ctrlr_commit_statusm, dmi is not valid ");
    return UPLL_RC_ERR_GENERIC;
  }
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
          UPLL_LOG_DEBUG(
              " PolicingProfileEntryMoMgr::GetRenamedUncKey is successful -%d",
              result_code);
          return result_code;
        }
      }
    }
  }
  for (int i = 0; i < nop; i++) {
    if (op[i] != UNC_OP_UPDATE) {
      result_code = DiffConfigDB(UPLL_DT_CANDIDATE, UPLL_DT_RUNNING, op[i], req,
                                 nreq, &cfg1_cursor, dmi, MAINTBL);
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
          return result_code;
        }
      }
      if (cfg1_cursor) {
        dmi->CloseCursor(cfg1_cursor, true);
        cfg1_cursor = NULL;
      }
      if (req)
        delete req;
      req = NULL;
    }
    UPLL_LOG_DEBUG("Updating main table complete with op %d", op[i]);
  }

  for (int i = 0; i < nop; i++) {
    MoMgrTables tbl = (op[i] == UNC_OP_UPDATE)?MAINTBL:CTRLRTBL;
    result_code = DiffConfigDB(UPLL_DT_CANDIDATE, UPLL_DT_RUNNING, op[i], req,
                               nreq, &cfg1_cursor, dmi, tbl);

    ConfigKeyVal *ppe_ctrlr_key = NULL;
    while (result_code == UPLL_RC_SUCCESS) {
      db_result = dmi->GetNextRecord(cfg1_cursor);
      result_code = DalToUpllResCode(db_result);
      if (result_code != UPLL_RC_SUCCESS)
        break;

      if (op[i] == UNC_OP_UPDATE) {
        DbSubOp dbop = { kOpReadMultiple, kOpMatchNone, kOpInOutCtrlr };
        result_code = GetChildConfigKey(ppe_ctrlr_key, req);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("GetChildConfigKey is failed resultcode=%d",
                         result_code);
          return result_code;
        }
        /* Capability check
         * req_header->operation = op[i];
         * strcpy((char*)req_header->datatype, (char*)UNC_DT_CANDIDATE);
         * result_code = ValidateCapability(req_header, vtn_ctrlr_key);
         *                                         */
        result_code = ReadConfigDB(ppe_ctrlr_key, UPLL_DT_CANDIDATE,
                                   UNC_OP_READ, dbop, dmi, CTRLRTBL);
        if (result_code != UPLL_RC_SUCCESS) {
          delete ppe_ctrlr_key;
          if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
            result_code = UpdateMainTbl(req, op[i], UPLL_RC_SUCCESS,
                                        nreq, dmi);
            if (result_code != UPLL_RC_SUCCESS) {
              UPLL_LOG_DEBUG("Error updating main table%d", result_code);
              return result_code;
            } else {
              continue;
            }
          } else  {
            return result_code;
          }
        }

        result_code = DupConfigKeyVal(ppe_key, req, MAINTBL);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("DupConfigKeyVal is failed result_code = %d",
                         result_code);
          delete(ppe_ctrlr_key);  // COVERITY RESOURCE LEAk
          return result_code;
        }
        GET_USER_DATA_CTRLR(ppe_ctrlr_key, ctrlr_id);
        string controller(reinterpret_cast<char *>(ctrlr_id));
        void *fle_val1 = GetVal(ppe_ctrlr_key);
        void *fle_val2 = GetVal(nreq);
        for (ConfigKeyVal *tmp = ppe_ctrlr_key; tmp != NULL; tmp =
             tmp->get_next_cfg_key_val()) {
          result_code = UpdateConfigStatus(ppe_key, op[i],
                                           ctrlr_result[controller], nreq,
                                           dmi, tmp);
          if (result_code != UPLL_RC_SUCCESS) break;
          CompareValidValue(fle_val1, fle_val2, false);
          result_code = UpdateConfigDB(ppe_ctrlr_key,
                                       UPLL_DT_RUNNING, op[i], dmi, CTRLRTBL);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("UpdateConfigDB for ctrlr tbl is failed ");
            delete ppe_ctrlr_key;  // COVERITY RESOURCE LEAk
            delete ppe_key;
            return result_code;
          }
          result_code = UpdateConfigDB(ppe_key, UPLL_DT_RUNNING,
                                       op[i], dmi, MAINTBL);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("UpdateConfigDB for main tbl is failed");
            delete ppe_ctrlr_key;  // COVERITY RESOURCE LEAk
            return result_code;
          }  // COV UNREACHABLE
          EnqueCfgNotification(op[i], UPLL_DT_RUNNING,
                               ppe_ctrlr_key);
        }
      } else {
        if (op[i] == UNC_OP_CREATE) {
          DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutFlag };
          result_code = GetChildConfigKey(ppe_key, req);
          result_code = ReadConfigDB(ppe_key, UPLL_DT_CANDIDATE,
                                     UNC_OP_READ, dbop, dmi, MAINTBL);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("ReadConfigDB is failed -%d", result_code);
            return result_code;
          }
          /* Capability check
           * req_header->operation = op[i];
           * strcpy((char*)req_header->datatype, (char*)UNC_DT_CANDIDATE);
           * result_code = ValidateCapability(req_header, vtn_ctrlr_key);
           *                                                 */
          result_code = DupConfigKeyVal(ppe_ctrlr_key, req, tbl);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("DupConfigKeyVal is failed -%d", result_code);
            return result_code;
          }

          GET_USER_DATA_CTRLR(ppe_ctrlr_key, ctrlr_id);
          string controller(reinterpret_cast<char *>(ctrlr_id));
          result_code = UpdateConfigStatus(ppe_key, op[i],
                                           ctrlr_result[controller], NULL,
                                           dmi, ppe_ctrlr_key);
        } else if (op[i] == UNC_OP_DELETE) {
          result_code = GetChildConfigKey(ppe_ctrlr_key, req);
        }
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("GetChildConfigKey is failed -%d",
                         result_code);
          return result_code;
        }
        result_code = UpdateConfigDB(ppe_ctrlr_key, UPLL_DT_RUNNING,
                                     op[i], dmi, CTRLRTBL);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("UpdateConfigDB in ctrlr tbl is failed -%d",
                         result_code);
          return result_code;
        }
        if (op[i] != UNC_OP_DELETE) {
          result_code = UpdateConfigDB(ppe_key, UPLL_DT_RUNNING,
                                       UNC_OP_UPDATE, dmi, MAINTBL);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("UpdateConfigDB in main tbl is failed -%d",
                           result_code);
            return result_code;
          }
        }
        EnqueCfgNotification(op[i], UPLL_DT_RUNNING,
                             ppe_key);
      }
      if (ppe_key) delete ppe_key;
      ppe_key = ppe_ctrlr_key = NULL;
      result_code = DalToUpllResCode(db_result);
    }
    if (cfg1_cursor) {
      dmi->CloseCursor(cfg1_cursor, true);
      cfg1_cursor = NULL;
    }
    if (nreq) delete nreq;
    if (req) delete req;
    nreq = req = NULL;
  }
  result_code = (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) ?
      UPLL_RC_SUCCESS : result_code;
  UPLL_LOG_DEBUG("TxcopyCandidatetoRunning is successful -%d", result_code);
  return result_code;
}

upll_rc_t PolicingProfileEntryMoMgr::UpdateMainTbl(ConfigKeyVal *ppe_key,
      unc_keytype_operation_t op, uint32_t driver_result,
      ConfigKeyVal *nreq, DalDmlIntf *dmi) {
  ConfigKeyVal *ck_ppe = NULL;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  val_policingprofile_entry_t *ppe_val = NULL;
  void *ppeval = NULL;
  void *nppeval = NULL;

  UPLL_FUNC_TRACE;
  if (op != UNC_OP_DELETE) {
    result_code = DupConfigKeyVal(ck_ppe, ppe_key, MAINTBL);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Returning error %d\n", result_code);
      return UPLL_RC_ERR_GENERIC;
    }
    ppe_val = reinterpret_cast<val_policingprofile_entry_t *>(GetVal(ck_ppe));
    if (!ppe_val) {
      UPLL_LOG_DEBUG("invalid val \n");
      return UPLL_RC_ERR_GENERIC;
    }
  } else {
    result_code = GetChildConfigKey(ck_ppe, ppe_key);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Returning error %d\n", result_code);
      return UPLL_RC_ERR_GENERIC;
    }
  }
  switch (op) {
    case UNC_OP_CREATE:
      ppe_val->cs_row_status = UNC_CS_NOT_APPLIED;
      break;
    case UNC_OP_UPDATE:

      ppeval = GetVal(ck_ppe);
      nppeval = (nreq)?GetVal(nreq):NULL;
      if (!nppeval) {
        UPLL_LOG_DEBUG("Invalid param\n");
        return UPLL_RC_ERR_GENERIC;
      }
      CompareValidValue(ppeval, nppeval, false);
      break;
    case UNC_OP_DELETE:
      break;
    default:
      UPLL_LOG_DEBUG("Inalid operation\n");
      return UPLL_RC_ERR_GENERIC;
  }
  result_code = UpdateConfigDB(ck_ppe, UPLL_DT_RUNNING, op, dmi, MAINTBL);
  EnqueCfgNotification(op, UPLL_DT_RUNNING, ck_ppe);
  delete ck_ppe;
  return result_code;
}

bool PolicingProfileEntryMoMgr::CompareValidValue(void *&val1, void *val2,
    bool audit) {
  UPLL_FUNC_TRACE;
  val_policingprofile_entry_t *val_pp_entry1 =
    reinterpret_cast<val_policingprofile_entry_t *>(val1);
  val_policingprofile_entry_t *val_pp_entry2 =
    reinterpret_cast<val_policingprofile_entry_t *>(val2);

//  if (audit) {
    for (unsigned int loop = 0;
        loop < sizeof(val_pp_entry1->valid) /
        sizeof(val_pp_entry1->valid[0]); ++loop) {
      if (val_pp_entry1->valid[loop] == UNC_VF_INVALID
          && val_pp_entry2->valid[loop] == UNC_VF_VALID) {
        val_pp_entry1->valid[loop] = UNC_VF_VALID_NO_VALUE;
      }
    }
//  }

  if (val_pp_entry1->valid[UPLL_IDX_FLOWLIST_PPE] == (UNC_VF_VALID)
      && val_pp_entry2->valid[UPLL_IDX_FLOWLIST_PPE] == UNC_VF_VALID) {
    if (strcmp(reinterpret_cast<char *>(val_pp_entry1->flowlist),
          reinterpret_cast<const char *>(val_pp_entry2->flowlist)) != 0)
      val_pp_entry1->valid[UPLL_IDX_FLOWLIST_PPE] = UNC_VF_VALID;
  }
  if (val_pp_entry1->valid[UPLL_IDX_RATE_PPE] == UNC_VF_VALID
      && val_pp_entry2->valid[UPLL_IDX_RATE_PPE] == UNC_VF_VALID) {
    if (val_pp_entry1->rate == val_pp_entry2->rate) {
      val_pp_entry1->valid[UPLL_IDX_RATE_PPE] = UNC_VF_INVALID;
    }
  }
  if (val_pp_entry1->valid[UPLL_IDX_CIR_PPE] == UNC_VF_VALID
      && val_pp_entry2->valid[UPLL_IDX_CIR_PPE] == UNC_VF_VALID) {
    if (val_pp_entry1->cir == val_pp_entry2->cir)
      val_pp_entry1->valid[UPLL_IDX_CIR_PPE] = UNC_VF_INVALID;
  }
  if (val_pp_entry1->valid[UPLL_IDX_CBS_PPE] == UNC_VF_VALID
      && val_pp_entry2->valid[UPLL_IDX_CBS_PPE] == UNC_VF_VALID) {
    if (val_pp_entry1->cbs == val_pp_entry2->cbs)
      val_pp_entry1->valid[UPLL_IDX_CBS_PPE] = UNC_VF_INVALID;
  }
  if (val_pp_entry1->valid[UPLL_IDX_PIR_PPE] == UNC_VF_VALID
      && val_pp_entry2->valid[UPLL_IDX_PIR_PPE] == UNC_VF_VALID) {
    if (val_pp_entry1->pir == val_pp_entry2->pir)
      val_pp_entry1->valid[UPLL_IDX_PIR_PPE] = UNC_VF_INVALID;
  }
  if (val_pp_entry1->valid[UPLL_IDX_PBS_PPE] == UNC_VF_VALID
      && val_pp_entry2->valid[UPLL_IDX_PBS_PPE] == UNC_VF_VALID) {
    if (val_pp_entry1->pbs == val_pp_entry2->pbs)
      val_pp_entry1->valid[UPLL_IDX_PBS_PPE] = UNC_VF_INVALID;
  }
  if (val_pp_entry1->valid[UPLL_IDX_GREEN_ACTION_PPE] == UNC_VF_VALID
      && val_pp_entry2->valid[UPLL_IDX_GREEN_ACTION_PPE] == UNC_VF_VALID) {
    if (val_pp_entry1->green_action == val_pp_entry2->green_action)
      val_pp_entry1->valid[UPLL_IDX_GREEN_ACTION_PPE] = UNC_VF_INVALID;
  }
  if (val_pp_entry1->valid[UPLL_IDX_GREEN_PRIORITY_PPE] == UNC_VF_VALID
      && val_pp_entry2->valid[UPLL_IDX_GREEN_PRIORITY_PPE] == UNC_VF_VALID) {
    if (val_pp_entry1->green_action_priority !=
        val_pp_entry2->green_action_priority)
      val_pp_entry1->valid[UPLL_IDX_GREEN_PRIORITY_PPE] = UNC_VF_INVALID;
  }
  if (val_pp_entry1->valid[UPLL_IDX_GREEN_DSCP_PPE] == UNC_VF_VALID
      && val_pp_entry2->valid[UPLL_IDX_GREEN_DSCP_PPE] == UNC_VF_VALID) {
    if (val_pp_entry1->green_action_dscp == val_pp_entry2->green_action_dscp)
      val_pp_entry1->valid[UPLL_IDX_GREEN_DSCP_PPE] = UNC_VF_INVALID;
  }
  if (val_pp_entry1->valid[UPLL_IDX_GREEN_DROP_PPE] == UNC_VF_VALID
      && val_pp_entry2->valid[UPLL_IDX_GREEN_DROP_PPE] == UNC_VF_VALID) {
    if (val_pp_entry1->green_action_drop_precedence ==
        val_pp_entry2->green_action_drop_precedence)
      val_pp_entry1->valid[UPLL_IDX_GREEN_DROP_PPE] = UNC_VF_INVALID;
  }
  if (val_pp_entry1->valid[UPLL_IDX_YELLOW_ACTION_PPE] == UNC_VF_VALID
      && val_pp_entry2->valid[UPLL_IDX_YELLOW_ACTION_PPE] == UNC_VF_VALID) {
    if (val_pp_entry1->yellow_action == val_pp_entry2->yellow_action)
      val_pp_entry1->valid[UPLL_IDX_YELLOW_ACTION_PPE] = UNC_VF_INVALID;
  }
  if (val_pp_entry1->valid[UPLL_IDX_YELLOW_PRIORITY_PPE] == UNC_VF_VALID
      && val_pp_entry2->valid[UPLL_IDX_YELLOW_PRIORITY_PPE] == UNC_VF_VALID) {
    if (val_pp_entry1->yellow_action_priority ==
        val_pp_entry2->yellow_action_priority)
      val_pp_entry1->valid[UPLL_IDX_YELLOW_PRIORITY_PPE] = UNC_VF_INVALID;
  }
  if (val_pp_entry1->valid[UPLL_IDX_YELLOW_DSCP_PPE] == UNC_VF_VALID
      && val_pp_entry2->valid[UPLL_IDX_YELLOW_DSCP_PPE] == UNC_VF_VALID) {
    if (val_pp_entry1->yellow_action_dscp == val_pp_entry2->yellow_action_dscp)
      val_pp_entry1->valid[UPLL_IDX_YELLOW_DSCP_PPE] = UNC_VF_INVALID;
  }
  if (val_pp_entry1->valid[UPLL_IDX_YELLOW_DROP_PPE] == UNC_VF_VALID
      && val_pp_entry2->valid[UPLL_IDX_YELLOW_DROP_PPE] == UNC_VF_VALID) {
    if (val_pp_entry1->yellow_action_drop_precedence ==
        val_pp_entry2->yellow_action_drop_precedence)
      val_pp_entry1->valid[UPLL_IDX_YELLOW_DROP_PPE] = UNC_VF_INVALID;
  }
  if (val_pp_entry1->valid[UPLL_IDX_RED_ACTION_PPE] == UNC_VF_VALID
      && val_pp_entry2->valid[UPLL_IDX_RED_ACTION_PPE] == UNC_VF_VALID) {
    if (val_pp_entry1->red_action == val_pp_entry2->red_action)
      val_pp_entry1->valid[UPLL_IDX_RED_ACTION_PPE] = UNC_VF_INVALID;
  }
  if (val_pp_entry1->valid[UPLL_IDX_RED_PRIORITY_PPE] == UNC_VF_VALID
      && val_pp_entry2->valid[UPLL_IDX_RED_PRIORITY_PPE] == UNC_VF_VALID) {
    if (val_pp_entry1->red_action_priority ==
        val_pp_entry2->red_action_priority)
      val_pp_entry1->valid[UPLL_IDX_RED_PRIORITY_PPE] = UNC_VF_INVALID;
  }
  if (val_pp_entry1->valid[UPLL_IDX_RED_DSCP_PPE] == UNC_VF_VALID
      && val_pp_entry2->valid[UPLL_IDX_RED_DSCP_PPE] == UNC_VF_VALID) {
    if (val_pp_entry1->red_action_dscp ==
        val_pp_entry2->red_action_dscp)
      val_pp_entry1->valid[UPLL_IDX_RED_DSCP_PPE] = UNC_VF_INVALID;
  }
  if (val_pp_entry1->valid[UPLL_IDX_RED_DROP_PPE] == UNC_VF_VALID
      && val_pp_entry2->valid[UPLL_IDX_RED_DROP_PPE] == UNC_VF_VALID) {
    if (val_pp_entry1->red_action_drop_precedence ==
        val_pp_entry2->red_action_drop_precedence)
      val_pp_entry1->valid[UPLL_IDX_RED_DROP_PPE] = UNC_VF_INVALID;
  }
  return true;
}

upll_rc_t PolicingProfileEntryMoMgr::ReadRecord(IpcReqRespHeader *req,
    ConfigKeyVal *ikey,
    DalDmlIntf *dmi,
    unc_keytype_operation_t op) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code;

  DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutNone };

  result_code = ValidateMessage(req, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG(
        "ValidateMessage is successful:-%d",
        result_code);
    return result_code;
  }
  if (req->datatype == UPLL_DT_STATE) {
    result_code = UpdateConfigDB(ikey, UPLL_DT_RUNNING, UNC_OP_READ, dmi);
  } else {
    result_code = UpdateConfigDB(ikey, req->datatype, UNC_OP_READ, dmi);
  }

  if (result_code != UPLL_RC_ERR_INSTANCE_EXISTS) {
    UPLL_LOG_DEBUG(" PolicingProfileEntryMoMgr::UpdateConfigDB is failed :-%d",
        result_code);
    return UPLL_RC_ERR_NO_SUCH_INSTANCE;
  }
  switch (req->datatype) {
    // Retrieving config information
    case UPLL_DT_CANDIDATE:  // MIXED ENUMS
    case UPLL_DT_RUNNING:
    case UPLL_DT_STARTUP:
    case UPLL_DT_STATE:
      if (req->option1 == UNC_OPT1_NORMAL) {
        result_code = ReadConfigDB(ikey, req->datatype, req->operation, dbop,
            dmi, MAINTBL);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG(
              " PolicingProfileEntryMoMgr::ReadConfigDB is Successful :-%d",
              result_code);
          return result_code;
        }
      } else {
        UPLL_LOG_DEBUG(" PolicingProfileEntryMoMgr::Operation not allowed:-");
        return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT;
      }

      break;

    case UPLL_DT_IMPORT:  // MIXED ENUMS
      if (req->option1 == UNC_OPT1_NORMAL) {
        result_code = GetRenamedUncKey(ikey, req->datatype, dmi, NULL);
        result_code = ReadConfigDB(ikey, req->datatype, req->operation, dbop,
            dmi, MAINTBL);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG(
              " PolicingProfileEntryMoMgr::ReadConfigDB is Successful :-%d",
              result_code);
          return result_code;
        }

      } else {
        UPLL_LOG_DEBUG(" PolicingProfileEntryMoMgr::Operation not allowed:-");
        return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT;
      }
      break;

    default:
      UPLL_LOG_DEBUG(" PolicingProfileEntryMoMgr::Operation not allowed:-");
      return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT;
  }  // end of switch
  UPLL_LOG_DEBUG(" PolicingProfileEntryMoMgr::ReadMo is Successful :-%d",
      result_code);
  return UPLL_RC_SUCCESS;
}

upll_rc_t PolicingProfileEntryMoMgr::ReadMo(IpcReqRespHeader *req,
    ConfigKeyVal *ikey,
    DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  controller_domain ctrlr_dom;
  result_code = ValidateMessage(req, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("ValidateMessage failed result_code %d",
                    result_code);
      return result_code;
  }
  result_code = ReadInfoFromDB(req, ikey, dmi, &ctrlr_dom);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG(" Read request failed result(%d)", result_code);
  }
  return result_code;
}

upll_rc_t PolicingProfileEntryMoMgr::ReadSiblingMo(IpcReqRespHeader *req,
    ConfigKeyVal *ikey,
    DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  controller_domain ctrlr_dom;
  result_code = ValidateMessage(req, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("ValidateMessage failed result_code %d",
                   result_code);
    return result_code;
  }
  result_code = ReadInfoFromDB(req, ikey, dmi, &ctrlr_dom);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG(" Read request failed result(%d)", result_code);
  }
  return result_code;

  // return ReadRecord(req, ikey, dmi, UNC_OP_READ_SIBLING);
}

upll_rc_t PolicingProfileEntryMoMgr::GetParentConfigKey(ConfigKeyVal *&okey,
                                                        ConfigKeyVal *ikey) {
  UPLL_FUNC_TRACE;

  if (!ikey) {
    UPLL_LOG_DEBUG(" Input Key is NULL");
    return UPLL_RC_ERR_GENERIC;
  }

  unc_key_type_t ikey_type = ikey->get_key_type();
  if (ikey_type != UNC_KT_POLICING_PROFILE_ENTRY) {
    UPLL_LOG_DEBUG(" Invalid key type received. Key type - %d", ikey_type);
    return UPLL_RC_ERR_GENERIC;
  }

  key_policingprofile_entry_t *pkey =
      reinterpret_cast<key_policingprofile_entry_t*>(ikey->get_key());
  if (!pkey) {
    UPLL_LOG_DEBUG(" Input policing profile entry key is NULL ");
    return UPLL_RC_ERR_GENERIC;
  }

  key_policingprofile_t *pp_key = reinterpret_cast<key_policingprofile_t*>
      (ConfigKeyVal::Malloc(sizeof(key_policingprofile_t)));

  uuu::upll_strncpy(pp_key->policingprofile_name,
                    reinterpret_cast<key_policingprofile_entry_t *>
                    (pkey)->policingprofile_key.policingprofile_name,
                    (kMaxLenPolicingProfileName + 1));
  okey = new ConfigKeyVal(UNC_KT_POLICING_PROFILE,
                          IpctSt::kIpcStKeyPolicingprofile, pp_key, NULL);
  SET_USER_DATA(okey, ikey);
  return UPLL_RC_SUCCESS;
}

upll_rc_t PolicingProfileEntryMoMgr::ValidateRate(
    val_policingprofile_entry_t *val_ppe,
    uint32_t operation) {
  UPLL_FUNC_TRACE;

  /* validate Rate */
  if (ValidateNumericRange(val_ppe->rate,
        (uint8_t) UPLL_POLICINGPROFILE_RATE_KBPS,
        (uint8_t) UPLL_POLICINGPROFILE_RATE_PPS, true,
        true)) {
    UPLL_LOG_DEBUG(" Rate validation is success");

    /** validate cir if filled */
    if (val_ppe->valid[UPLL_IDX_CIR_PPE] == UNC_VF_VALID) {
      UPLL_LOG_DEBUG("CIR value is filled");

      /** check cir range */
      if (!(ValidateNumericRange(val_ppe->cir, kMinRateType,
              kMaxRateType, true, true))) {
        UPLL_LOG_DEBUG(" CIR validation failed");
        return UPLL_RC_ERR_CFG_SYNTAX;
      }
    } else if ((operation == UNC_OP_UPDATE || operation == UNC_OP_CREATE)
        && (val_ppe->valid[UPLL_IDX_CIR_PPE]
          == UNC_VF_VALID_NO_VALUE)) {
      UPLL_LOG_DEBUG("Reset CIR");
      val_ppe->cir = 0;
    }

    /** validate cbs if filled */
    if (val_ppe->valid[UPLL_IDX_CBS_PPE] == UNC_VF_VALID) {
      UPLL_LOG_DEBUG("CBS value is filled");

      /** check cbs range */
      if (!(ValidateNumericRange(val_ppe->cbs, kMinBurstSize,
              kMaxBurstSize, true, true))) {
        UPLL_LOG_DEBUG(" CBS validation failed");
        return UPLL_RC_ERR_CFG_SYNTAX;
      }
    } else if ((operation == UNC_OP_UPDATE || operation == UNC_OP_CREATE)
        && (val_ppe->valid[UPLL_IDX_CBS_PPE]
          == UNC_VF_VALID_NO_VALUE)) {
      UPLL_LOG_DEBUG("Reset CBS");
      val_ppe->cbs = 0;
    }

    /**validate pir if filled */
    if (val_ppe->valid[UPLL_IDX_PIR_PPE] == UNC_VF_VALID) {
      UPLL_LOG_DEBUG("PIR value is filled");

      /** check pir range */
      if (!(ValidateNumericRange(val_ppe->pir, kMinRateType,
              kMaxRateType, true, true))) {
        UPLL_LOG_DEBUG(" PIR validation failed");
        return UPLL_RC_ERR_CFG_SYNTAX;
      }
    } else if ((operation == UNC_OP_UPDATE || operation == UNC_OP_CREATE)
        && (val_ppe->valid[UPLL_IDX_PIR_PPE]
          == UNC_VF_VALID_NO_VALUE)) {
      UPLL_LOG_DEBUG("Reset PIR");
      val_ppe->pir = 0;
    }

    /** validate pbs if filled */
    if (val_ppe->valid[UPLL_IDX_PBS_PPE] == UNC_VF_VALID) {
      UPLL_LOG_DEBUG("PBS value is filled");

      /** check pbs range */
      if (!(ValidateNumericRange(val_ppe->pbs, kMinBurstSize,
              kMaxBurstSize, true, true))) {
        UPLL_LOG_DEBUG(" PBS validation failed");
        return UPLL_RC_ERR_CFG_SYNTAX;
      }
    } else if ((operation == UNC_OP_UPDATE || operation == UNC_OP_CREATE)
        && (val_ppe->valid[UPLL_IDX_PBS_PPE]
          == UNC_VF_VALID_NO_VALUE)) {
      UPLL_LOG_DEBUG("Reset PBS");
      val_ppe->pbs = 0;
    }
    if (UNC_OP_CREATE == operation) {
      if ((UNC_VF_VALID == val_ppe->valid[UPLL_IDX_PIR_PPE])
        && (UNC_VF_VALID == val_ppe->valid[UPLL_IDX_CIR_PPE])) {
        if (val_ppe->pir < val_ppe->cir) {
          UPLL_LOG_DEBUG("cir is greated");
          return UPLL_RC_ERR_CFG_SYNTAX;
        }
      } else {
        UPLL_LOG_DEBUG("cir and pir are amndatory");
        return UPLL_RC_ERR_CFG_SYNTAX;
      }
    }
    return UPLL_RC_SUCCESS;
  }
  /** rate validation failed */
  return UPLL_RC_ERR_CFG_SYNTAX;
}

upll_rc_t PolicingProfileEntryMoMgr::ValidateColorAction(
    val_policingprofile_entry_t *val_ppe,
    uint32_t operation) {
  UPLL_FUNC_TRACE;

  /** validate green_action */
  if (val_ppe->valid[UPLL_IDX_GREEN_ACTION_PPE]
      == UNC_VF_VALID) {
    if (!(ValidateNumericRange(val_ppe->green_action,
            (uint8_t) UPLL_POLICINGPROFILE_ACT_PASS,
            (uint8_t) UPLL_POLICINGPROFILE_ACT_PENALTY, true,
            true))) {
      UPLL_LOG_DEBUG("green_action syntax validation failed");
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  } else if ((operation == UNC_OP_UPDATE || operation == UNC_OP_CREATE)
      && (val_ppe->valid[UPLL_IDX_GREEN_ACTION_PPE]
        == UNC_VF_VALID_NO_VALUE)) {
    UPLL_LOG_DEBUG("Reset green_action");
    val_ppe->green_action = 0;
  }

  /** validate red_action */
  if (val_ppe->valid[UPLL_IDX_RED_ACTION_PPE]
      == UNC_VF_VALID) {
    if (!(ValidateNumericRange(val_ppe->red_action,
            (uint8_t) UPLL_POLICINGPROFILE_ACT_PASS,
            (uint8_t) UPLL_POLICINGPROFILE_ACT_PENALTY, true,
            true))) {
      UPLL_LOG_DEBUG("red_action syntax validation failed");
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  } else if ((operation == UNC_OP_UPDATE || operation == UNC_OP_CREATE)
      && (val_ppe->valid[UPLL_IDX_RED_ACTION_PPE]
        == UNC_VF_VALID_NO_VALUE)) {
    UPLL_LOG_DEBUG("Reset red_action");
    val_ppe->red_action = 0;
  }

  /** validate yellow_action */
  if (val_ppe->valid[UPLL_IDX_YELLOW_ACTION_PPE]
      == UNC_VF_VALID) {
    if (!(ValidateNumericRange(val_ppe->yellow_action,
            (uint8_t) UPLL_POLICINGPROFILE_ACT_PASS,
            (uint8_t) UPLL_POLICINGPROFILE_ACT_PENALTY, true,
            true))) {
      UPLL_LOG_DEBUG("yellow_action syntax validation failed");
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  } else if ((operation == UNC_OP_UPDATE || operation == UNC_OP_CREATE)
      && (val_ppe->valid[UPLL_IDX_YELLOW_ACTION_PPE]
        == UNC_VF_VALID_NO_VALUE)) {
    UPLL_LOG_DEBUG("Reset yellow_action");
    val_ppe->yellow_action = 0;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t PolicingProfileEntryMoMgr::ValidateColorPriority(
    val_policingprofile_entry_t *val_ppe,
    uint32_t operation) {
  UPLL_FUNC_TRACE;

  /** validate green_action_priority */
  if (val_ppe->valid[UPLL_IDX_GREEN_PRIORITY_PPE]
      == UNC_VF_VALID) {
    if (val_ppe->green_action
        == UPLL_POLICINGPROFILE_ACT_PENALTY) {
      /** check green_action_priority range */
      if (!ValidateNumericRange(
            val_ppe->green_action_priority, kMinVlanPriority,
            kMaxVlanPriority, true, true)) {
        UPLL_LOG_DEBUG("green_action_priority syntax validation failed");
        return UPLL_RC_ERR_CFG_SYNTAX;
      }
    } else {
      UPLL_LOG_DEBUG("Error Green_action_priority configured but green_action"
          "is not UPLL_POLICINGPROFILE_ACT_PENALTY");

      return UPLL_RC_ERR_CFG_SYNTAX;
    }

  } else if ((operation == UNC_OP_UPDATE || operation == UNC_OP_CREATE)
      && (val_ppe->valid[UPLL_IDX_GREEN_PRIORITY_PPE]
        == UNC_VF_VALID_NO_VALUE)) {
    UPLL_LOG_DEBUG("Reset green_priority");
    val_ppe->green_action_priority = 0;
  }

  /** validate red_action_priority */
  if (val_ppe->valid[UPLL_IDX_RED_PRIORITY_PPE]
      == UNC_VF_VALID) {
    if (val_ppe->red_action
        == UPLL_POLICINGPROFILE_ACT_PENALTY) {
      /** check green_action_priority range */
      if (!ValidateNumericRange(val_ppe->red_action_priority,
            kMinVlanPriority, kMaxVlanPriority, true,
            true)) {
        UPLL_LOG_DEBUG("red_action_priority syntax validation failed");
        return UPLL_RC_ERR_CFG_SYNTAX;
      }
    } else {
      UPLL_LOG_DEBUG("Error red_action_priority configured but red_action"
          "is not UPLL_POLICINGPROFILE_ACT_PENALTY");

      return UPLL_RC_ERR_CFG_SYNTAX;
    }

  } else if ((operation == UNC_OP_UPDATE || operation == UNC_OP_CREATE)
      && (val_ppe->valid[UPLL_IDX_RED_PRIORITY_PPE]
        == UNC_VF_VALID_NO_VALUE)) {
    UPLL_LOG_DEBUG("Reset red_priority");
    val_ppe->red_action_priority = 0;
  }

  /** validate yellow_action_priority */
  if (val_ppe->valid[UPLL_IDX_YELLOW_PRIORITY_PPE]
      == UNC_VF_VALID) {
    if (val_ppe->yellow_action
        == UPLL_POLICINGPROFILE_ACT_PENALTY) {
      /** check yellow_action_priority range */
      if (!ValidateNumericRange(
            val_ppe->yellow_action_priority, kMinVlanPriority,
            kMaxVlanPriority, true, true)) {
        UPLL_LOG_DEBUG("yellow_action_priority syntax validation failed");
        return UPLL_RC_ERR_CFG_SYNTAX;
      }
    } else {
      UPLL_LOG_DEBUG("Error yellow_action_priority configured but yellow_action"
          "is not UPLL_POLICINGPROFILE_ACT_PENALTY");

      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  } else if ((operation == UNC_OP_UPDATE || operation == UNC_OP_CREATE)
      && (val_ppe->valid[UPLL_IDX_YELLOW_PRIORITY_PPE]
        == UNC_VF_VALID_NO_VALUE)) {
    UPLL_LOG_DEBUG("Reset yellow priority");
    val_ppe->yellow_action_priority = 0;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t PolicingProfileEntryMoMgr::ValidateColorPrecedence(
    val_policingprofile_entry_t *val_ppe,
    uint32_t operation) {
  UPLL_FUNC_TRACE;

  /** validate green_action_drop_precedence */
  if (val_ppe->valid[UPLL_IDX_GREEN_DROP_PPE]
      == UNC_VF_VALID) {
    if (val_ppe->green_action
        == UPLL_POLICINGPROFILE_ACT_PENALTY) {
      /** check green_action_drop_precedence range */
      if (!(ValidateNumericRange(
          val_ppe->green_action_drop_precedence,
          kMinPrecedence, kMaxPrecedence, true, true))) {
        UPLL_LOG_DEBUG("green_action_drop_precedence syntax validation failed");
        return UPLL_RC_ERR_CFG_SYNTAX;
      }
    } else {
      UPLL_LOG_DEBUG("Error Green_action_drop_precedence configured but"
                    " green_action is not UPLL_POLICINGPROFILE_ACT_PENALTY");

      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  } else if ((operation == UNC_OP_UPDATE || operation == UNC_OP_CREATE)
      && (val_ppe->valid[UPLL_IDX_GREEN_DROP_PPE]
          == UNC_VF_VALID_NO_VALUE)) {
    UPLL_LOG_DEBUG("Reset Green_action_drop_precedence");
    val_ppe->green_action_drop_precedence = 0;
  }

  /** validate red_action_drop_precedence */
  if (val_ppe->valid[UPLL_IDX_RED_DROP_PPE] == UNC_VF_VALID) {
    if (val_ppe->red_action
        == UPLL_POLICINGPROFILE_ACT_PENALTY) {
      /** check green_action_drop_precedence range */
      if (!(ValidateNumericRange(
          val_ppe->red_action_drop_precedence, kMinPrecedence,
          kMaxPrecedence, true, true))) {
        UPLL_LOG_DEBUG("red_action_drop_precedence syntax validation failed");
        return UPLL_RC_ERR_CFG_SYNTAX;
      }
    } else {
      UPLL_LOG_DEBUG("Error red_action_drop_precedence configured but"
                     " red_action is not UPLL_POLICINGPROFILE_ACT_PENALTY");

      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  } else if ((operation == UNC_OP_UPDATE || operation == UNC_OP_CREATE)
      && (val_ppe->valid[UPLL_IDX_RED_DROP_PPE]
          == UNC_VF_VALID_NO_VALUE)) {
    UPLL_LOG_DEBUG("Reset red_action_drop_precedence");
    val_ppe->red_action_drop_precedence = 0;
  }

  /** validate yellow_action_drop_precedence */
  if (val_ppe->valid[UPLL_IDX_YELLOW_DROP_PPE]
      == UNC_VF_VALID) {
    if (val_ppe->yellow_action
        == UPLL_POLICINGPROFILE_ACT_PENALTY) {
      /** check yellow_action_drop_precedence range */
      if (!(ValidateNumericRange(
          val_ppe->yellow_action_drop_precedence,
          kMinPrecedence, kMaxPrecedence, true, true))) {
        UPLL_LOG_DEBUG("yellow_action_drop_precedence syntax check failed");
        return UPLL_RC_ERR_CFG_SYNTAX;
      }
    } else {
      UPLL_LOG_DEBUG("Error yellow_action_drop_precedence configured but"
                    " yellow_action is not UPLL_POLICINGPROFILE_ACT_PENALTY");

      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  } else if ((operation == UNC_OP_UPDATE || operation == UNC_OP_CREATE)
      && (val_ppe->valid[UPLL_IDX_YELLOW_DROP_PPE]
          == UNC_VF_VALID_NO_VALUE)) {
    UPLL_LOG_DEBUG("Reset yellow_action_drop_precedence");
    val_ppe->yellow_action_drop_precedence = 0;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t PolicingProfileEntryMoMgr::ValidateColorDscp(
    val_policingprofile_entry_t *val_ppe,
    uint32_t operation) {
  UPLL_FUNC_TRACE;

  /** validate green_action_dscp */
  if (val_ppe->valid[UPLL_IDX_GREEN_DSCP_PPE]
      == UNC_VF_VALID) {
    if (val_ppe->green_action
        == UPLL_POLICINGPROFILE_ACT_PENALTY) {
      /** Use common function to check green_action_dscp range */
      if (!ValidateNumericRange(val_ppe->green_action_dscp,
            kMinIPDscp, kMaxIPDscp, true, true)) {
        UPLL_LOG_DEBUG("green_action_dscp syntax validation failed");
        return UPLL_RC_ERR_CFG_SYNTAX;
      }
    } else {
      UPLL_LOG_DEBUG("Error Green_action_dscp configured but green_action"
          "is not UPLL_POLICINGPROFILE_ACT_PENALTY");

      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  } else if ((operation == UNC_OP_UPDATE || operation == UNC_OP_CREATE)
      && (val_ppe->valid[UPLL_IDX_GREEN_DSCP_PPE]
        == UNC_VF_VALID_NO_VALUE)) {
    UPLL_LOG_DEBUG("Reset green_action_dscp");
    val_ppe->green_action_dscp = 0;
  }

  /** validate red_action_dscp */
  if (val_ppe->valid[UPLL_IDX_RED_DSCP_PPE] == UNC_VF_VALID) {
    if (val_ppe->red_action
        == UPLL_POLICINGPROFILE_ACT_PENALTY) {
      /** check green_action_dscp range */
      if (!ValidateNumericRange(val_ppe->red_action_dscp,
            kMinIPDscp, kMaxIPDscp, true, true)) {
        UPLL_LOG_DEBUG("red_action_dscp syntax validation failed");
        return UPLL_RC_ERR_CFG_SYNTAX;
      }
    } else {
      UPLL_LOG_DEBUG("Error red_action_dscp configured but red_action"
          "is not UPLL_POLICINGPROFILE_ACT_PENALTY");

      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  } else if ((operation == UNC_OP_UPDATE || operation == UNC_OP_CREATE)
      && (val_ppe->valid[UPLL_IDX_RED_DSCP_PPE]
        == UNC_VF_VALID_NO_VALUE)) {
    UPLL_LOG_DEBUG("Reset Red action dscp");
    val_ppe->red_action_dscp = 0;
  }

  /** validate yellow_action_dscp */
  if (val_ppe->valid[UPLL_IDX_YELLOW_DSCP_PPE]
      == UNC_VF_VALID) {
    if (val_ppe->yellow_action
        == UPLL_POLICINGPROFILE_ACT_PENALTY) {
      /** check yellow_action_dscp range */
      if (!ValidateNumericRange(val_ppe->yellow_action_dscp,
            kMinIPDscp, kMaxIPDscp, true, true)) {
        UPLL_LOG_DEBUG("yellow_action_dscp syntax validation failed");
        return UPLL_RC_ERR_CFG_SYNTAX;
      }
    } else {
      UPLL_LOG_DEBUG("Error yellow_action_dscp configured but yellow_action"
          "is not UPLL_POLICINGPROFILE_ACT_PENALTY");

      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  } else if ((operation == UNC_OP_UPDATE || operation == UNC_OP_CREATE)
      && (val_ppe->valid[UPLL_IDX_YELLOW_DSCP_PPE]
        == UNC_VF_VALID_NO_VALUE)) {
    UPLL_LOG_DEBUG("Reset yellow action dscp");
    val_ppe->yellow_action_dscp = 0;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t PolicingProfileEntryMoMgr::ValidateCapability(IpcReqRespHeader *req,
                                          ConfigKeyVal *ikey,
                                          const char* ctrlr_name) {
  UPLL_FUNC_TRACE;

  upll_rc_t result_code = UPLL_RC_ERR_GENERIC;
  if ((NULL == req) || (NULL == ikey)) {
    UPLL_LOG_DEBUG("IpcReqRespHeader/ConfigKeyval is NULL");
    return UPLL_RC_ERR_GENERIC;
  }

  if (!ctrlr_name)
    ctrlr_name = static_cast<char *>(ikey->get_user_data());

  uint32_t dt_type = req->datatype;
  uint32_t operation = req->operation;
  uint32_t option1 = req->option1;
  uint32_t option2 = req->option2;

  UPLL_LOG_DEBUG("dt_type   : (%d)"
               "operation : (%d)"
               "option1   : (%d)"
               "option2   : (%d)",
               dt_type, operation, option1, option2);

  bool ret_code = false;
  uint32_t instance_count;
  const uint8_t *attrs = NULL;
  uint32_t max_attrs = 0;

  switch (operation) {
    case UNC_OP_CREATE: {
      ret_code = GetCreateCapability(ctrlr_name, ikey->get_key_type(),
                                        &instance_count, &max_attrs, &attrs);
      break;
    }
    case UNC_OP_UPDATE: {
      ret_code = GetUpdateCapability(ctrlr_name, ikey->get_key_type(),
                                        &max_attrs, &attrs);
      break;
    }
    default: {
      ret_code = GetReadCapability(ctrlr_name, ikey->get_key_type(),
                                      &max_attrs, &attrs);
    }
  }
  if (!ret_code) {
    UPLL_LOG_DEBUG("keytype(%d) is not supported by controller(%s) "
                   "for operation(%d)",
                   ikey->get_key_type(), ctrlr_name, operation);
    return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_KT;
  }

  val_policingprofile_entry_t *val_policingprofile_entry = NULL;
  /** check valid key, val struct received in ConfigKeyVal */
  if (ikey->get_cfg_val() && (ikey->get_cfg_val()->get_st_num() ==
     IpctSt::kIpcStValPolicingprofileEntry)) {
      val_policingprofile_entry =
        reinterpret_cast<val_policingprofile_entry_t *>(
           ikey->get_cfg_val()->get_val());
  }

  if ((operation == UNC_OP_CREATE) || (operation == UNC_OP_UPDATE)) {
    if (dt_type == UPLL_DT_CANDIDATE) {
      if (val_policingprofile_entry) {
        if (max_attrs > 0) {
          return ValPolicingProfileEntryAttributeSupportCheck(
             val_policingprofile_entry, attrs);
        } else {
          UPLL_LOG_DEBUG("Attribute list is empty for operation %d", operation);
        }
      } else {
        UPLL_LOG_DEBUG("Error value struct is mandatory for CREATE/UPDATE");
        return result_code;
      }
    } else {
      UPLL_LOG_DEBUG("Unsupported datatype for CREATE/UPDATE");
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  } else if (READ_SUPPORTED_OPERATION) {
    if (READ_SUPPORTED_DATATYPE) {
      if (option1 != UNC_OPT1_NORMAL) {
         UPLL_LOG_DEBUG(" Error: option1 is not NORMAL");
         return UPLL_RC_ERR_INVALID_OPTION1;
      }
      if (option2 == UNC_OPT2_NONE) {
        UPLL_LOG_DEBUG(" Error: option2 is not NONE");
        return UPLL_RC_ERR_INVALID_OPTION2;
      }
      /** Valid options received, validate value struct */
      if (val_policingprofile_entry) {
        if (max_attrs > 0) {
          return ValPolicingProfileEntryAttributeSupportCheck(
             val_policingprofile_entry, attrs);
        } else {
          UPLL_LOG_DEBUG("Attribute list is empty for operation %d", operation);
        }
      }
        return UPLL_RC_SUCCESS;
    } else {
      UPLL_LOG_DEBUG("Error Unsupported datatype (%d)", dt_type);
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  } else if (OPEARTION_WITH_VAL_STRUCT_NONE) {
    /** Value struct is NONE for this operations */
    UPLL_LOG_DEBUG("Skip capability check, Operation type is %d", operation);
    return UPLL_RC_SUCCESS;
  }

  UPLL_LOG_DEBUG("Error Unsupported operation (%d)", operation);
  return UPLL_RC_ERR_CFG_SYNTAX;
}

upll_rc_t PolicingProfileEntryMoMgr::
ValPolicingProfileEntryAttributeSupportCheck(
  val_policingprofile_entry_t *val_policingprofile_entry,
  const uint8_t *attrs) {
  UPLL_FUNC_TRACE;

  upll_rc_t ret_code = UPLL_RC_ERR_GENERIC;

  if (val_policingprofile_entry != NULL) {
    if ((val_policingprofile_entry->valid[UPLL_IDX_FLOWLIST_PPE] ==
         UNC_VF_VALID)
        || (val_policingprofile_entry->valid[UPLL_IDX_FLOWLIST_PPE]
            == UNC_VF_VALID_NO_VALUE)) {
      if (attrs[unc::capa::policingprofile_entry::kCapFlowlist] == 0) {
        UPLL_LOG_DEBUG("FLOWLIST  attr is not supported by ctrlr");
        val_policingprofile_entry->valid[UPLL_IDX_FLOWLIST_PPE] =
            UNC_VF_NOT_SOPPORTED;
        return UPLL_RC_ERR_CFG_SYNTAX;
      }
    }

    if ((val_policingprofile_entry->valid[UPLL_IDX_RATE_PPE] ==
         UNC_VF_VALID)
        || (val_policingprofile_entry->valid[UPLL_IDX_RATE_PPE]
            == UNC_VF_VALID_NO_VALUE)) {
      if (attrs[unc::capa::policingprofile_entry::kCapRate] == 0) {
        UPLL_LOG_DEBUG("FLOWLIST attr is not supported by ctrlr");
        val_policingprofile_entry->valid[UPLL_IDX_RATE_PPE] =
            UNC_VF_NOT_SOPPORTED;
        return UPLL_RC_ERR_CFG_SYNTAX;
      }
    }

    if ((val_policingprofile_entry->valid[UPLL_IDX_CIR_PPE] == UNC_VF_VALID)
        || (val_policingprofile_entry->valid[UPLL_IDX_CIR_PPE]
            == UNC_VF_VALID_NO_VALUE)) {
      if (attrs[unc::capa::policingprofile_entry::kCapCir] == 0) {
        UPLL_LOG_DEBUG("CIR attr is not supported by ctrlr");
        val_policingprofile_entry->valid[UPLL_IDX_CIR_PPE] =
            UNC_VF_NOT_SOPPORTED;
        return UPLL_RC_ERR_CFG_SYNTAX;
      }
    }

    if ((val_policingprofile_entry->valid[UPLL_IDX_CBS_PPE] == UNC_VF_VALID)
        || (val_policingprofile_entry->valid[UPLL_IDX_CBS_PPE]
            == UNC_VF_VALID_NO_VALUE)) {
      if (attrs[unc::capa::policingprofile_entry::kCapCbs] == 0) {
        UPLL_LOG_DEBUG("CBS attr is not supported by ctrlr");
        val_policingprofile_entry->valid[UPLL_IDX_CBS_PPE] =
            UNC_VF_NOT_SOPPORTED;
        return UPLL_RC_ERR_CFG_SYNTAX;
      }
    }

    if ((val_policingprofile_entry->valid[UPLL_IDX_PIR_PPE] == UNC_VF_VALID)
        || (val_policingprofile_entry->valid[UPLL_IDX_PIR_PPE]
            == UNC_VF_VALID_NO_VALUE)) {
      if (attrs[unc::capa::policingprofile_entry::kCapPir] == 0) {
        UPLL_LOG_DEBUG("PIR attr is not supported by ctrlr");
        val_policingprofile_entry->valid[UPLL_IDX_PIR_PPE] =
            UNC_VF_NOT_SOPPORTED;
        return UPLL_RC_ERR_CFG_SYNTAX;
      }
    }

    if ((val_policingprofile_entry->valid[UPLL_IDX_PBS_PPE] == UNC_VF_VALID)
        || (val_policingprofile_entry->valid[UPLL_IDX_PBS_PPE]
            == UNC_VF_VALID_NO_VALUE)) {
      if (attrs[unc::capa::policingprofile_entry::kCapPbs] == 0) {
        UPLL_LOG_DEBUG("PBS attr is not supported by ctrlr");
        val_policingprofile_entry->valid[UPLL_IDX_PBS_PPE] =
            UNC_VF_NOT_SOPPORTED;
        return UPLL_RC_ERR_CFG_SYNTAX;
      }
    }

    if ((ret_code
        == ValidateGreenFieldAttribute(val_policingprofile_entry, attrs))
        != UPLL_RC_SUCCESS) {
      return ret_code;
    }

    if ((ret_code == ValidateRedFieldAttribute(val_policingprofile_entry,
         attrs))!= UPLL_RC_SUCCESS) {
      return ret_code;
    }

    if ((ret_code
        == ValidateYellowFieldAttribute(val_policingprofile_entry, attrs))
        != UPLL_RC_SUCCESS) {
      return ret_code;
    }
    return UPLL_RC_SUCCESS;
  }
  UPLL_LOG_DEBUG(
      "Error, Unable to validate the attribute since val struct is NULL");
  return UPLL_RC_ERR_CFG_SYNTAX;
}

upll_rc_t PolicingProfileEntryMoMgr::ValidateGreenFieldAttribute(
    val_policingprofile_entry_t * val_policingprofile_entry,
    const uint8_t *attrs) {
  UPLL_FUNC_TRACE;

  if ((val_policingprofile_entry->valid[UPLL_IDX_GREEN_ACTION_PPE]
      == UNC_VF_VALID)
      || (val_policingprofile_entry->valid[UPLL_IDX_GREEN_ACTION_PPE]
          == UNC_VF_VALID_NO_VALUE)) {
    if (attrs[unc::capa::policingprofile_entry::kCapGreenAction] == 0) {
      UPLL_LOG_DEBUG("GREEN_ACTION attr is not supported by ctrlr");
      val_policingprofile_entry->valid[UPLL_IDX_GREEN_ACTION_PPE] =
          UNC_VF_NOT_SOPPORTED;
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  }

  if ((val_policingprofile_entry->valid[UPLL_IDX_GREEN_PRIORITY_PPE]
      == UNC_VF_VALID)
      || (val_policingprofile_entry->valid[UPLL_IDX_GREEN_PRIORITY_PPE]
          == UNC_VF_VALID_NO_VALUE)) {
    if (attrs[unc::capa::policingprofile_entry::kCapGreenPriority] == 0) {
      UPLL_LOG_DEBUG("GREEN_PRIORITY attr is not supported by ctrlr");
      val_policingprofile_entry->valid[UPLL_IDX_GREEN_PRIORITY_PPE] =
          UNC_VF_NOT_SOPPORTED;
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  }

  if ((val_policingprofile_entry->valid[UPLL_IDX_GREEN_DSCP_PPE] ==
        UNC_VF_VALID)
      || (val_policingprofile_entry->valid[UPLL_IDX_GREEN_DSCP_PPE]
          == UNC_VF_VALID_NO_VALUE)) {
    if (attrs[unc::capa::policingprofile_entry::kCapGreenDscp] == 0) {
      UPLL_LOG_DEBUG("GREEN_DSCP attr is not supported by ctrlr");
      val_policingprofile_entry->valid[UPLL_IDX_GREEN_DSCP_PPE] =
          UNC_VF_NOT_SOPPORTED;
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  }

  if ((val_policingprofile_entry->valid[UPLL_IDX_GREEN_DROP_PPE] ==
      UNC_VF_VALID)
      || (val_policingprofile_entry->valid[UPLL_IDX_GREEN_DROP_PPE]
          == UNC_VF_VALID_NO_VALUE)) {
    if (attrs[unc::capa::policingprofile_entry::kCapGreenDrop] == 0) {
      UPLL_LOG_DEBUG("GREEN_DROP attr is not supported by ctrlr");
      val_policingprofile_entry->valid[UPLL_IDX_GREEN_DROP_PPE] =
          UNC_VF_NOT_SOPPORTED;
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t PolicingProfileEntryMoMgr::ValidateYellowFieldAttribute(
    val_policingprofile_entry_t * val_policingprofile_entry,
    const uint8_t *attrs) {
  UPLL_FUNC_TRACE;

  if ((val_policingprofile_entry->valid[UPLL_IDX_YELLOW_ACTION_PPE]
      == UNC_VF_VALID)
      || (val_policingprofile_entry->valid[UPLL_IDX_YELLOW_ACTION_PPE]
          == UNC_VF_VALID_NO_VALUE)) {
    if (attrs[unc::capa::policingprofile_entry::kCapYellowAction] == 0) {
      UPLL_LOG_DEBUG("YELLOW_ACTION attr is not supported by ctrlr");
      val_policingprofile_entry->valid[UPLL_IDX_YELLOW_ACTION_PPE] =
          UNC_VF_NOT_SOPPORTED;
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  }

  if ((val_policingprofile_entry->valid[UPLL_IDX_YELLOW_PRIORITY_PPE]
      == UNC_VF_VALID)
      || (val_policingprofile_entry->valid[UPLL_IDX_YELLOW_PRIORITY_PPE]
          == UNC_VF_VALID_NO_VALUE)) {
    if (attrs[unc::capa::policingprofile_entry::kCapYellowPriority] == 0) {
      UPLL_LOG_DEBUG("YELLOW_PRIORITY attr is not supported by ctrlr");
      val_policingprofile_entry->valid[UPLL_IDX_YELLOW_PRIORITY_PPE] =
          UNC_VF_NOT_SOPPORTED;
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  }

  if ((val_policingprofile_entry->valid[UPLL_IDX_YELLOW_DSCP_PPE]
      == UNC_VF_VALID)
      || (val_policingprofile_entry->valid[UPLL_IDX_YELLOW_DSCP_PPE]
          == UNC_VF_VALID_NO_VALUE)) {
    if (attrs[unc::capa::policingprofile_entry::kCapYellowDscp] == 0) {
      UPLL_LOG_DEBUG("YELLOW_DSCP attr is not supported by ctrlr");
      val_policingprofile_entry->valid[UPLL_IDX_YELLOW_DSCP_PPE] =
          UNC_VF_NOT_SOPPORTED;
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  }

  if ((val_policingprofile_entry->valid[UPLL_IDX_YELLOW_DROP_PPE]
      == UNC_VF_VALID)
      || (val_policingprofile_entry->valid[UPLL_IDX_YELLOW_DROP_PPE]
          == UNC_VF_VALID_NO_VALUE)) {
    if (attrs[unc::capa::policingprofile_entry::kCapYellowDrop] == 0) {
      UPLL_LOG_DEBUG("YELLOW_DROP attr is not supported by ctrlr");
      val_policingprofile_entry->valid[UPLL_IDX_YELLOW_DROP_PPE] =
          UNC_VF_NOT_SOPPORTED;
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t PolicingProfileEntryMoMgr::ValidateRedFieldAttribute(
    val_policingprofile_entry_t * val_policingprofile_entry,
    const uint8_t *attrs) {
  UPLL_FUNC_TRACE;

  if ((val_policingprofile_entry->valid[UPLL_IDX_RED_ACTION_PPE]
      == UNC_VF_VALID)
      || (val_policingprofile_entry->valid[UPLL_IDX_RED_ACTION_PPE]
          == UNC_VF_VALID_NO_VALUE)) {
    if (attrs[unc::capa::policingprofile_entry::kCapRedAction] == 0) {
      UPLL_LOG_DEBUG("RED_ACTION attr is not supported by ctrlr");
      val_policingprofile_entry->valid[UPLL_IDX_RED_ACTION_PPE] =
          UNC_VF_NOT_SOPPORTED;
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  }

  if ((val_policingprofile_entry->valid[UPLL_IDX_RED_PRIORITY_PPE]
      == UNC_VF_VALID)
      || (val_policingprofile_entry->valid[UPLL_IDX_RED_PRIORITY_PPE]
          == UNC_VF_VALID_NO_VALUE)) {
    if (attrs[unc::capa::policingprofile_entry::kCapRedPriority] == 0) {
      UPLL_LOG_DEBUG("RED_PRIORITY attr is not supported by ctrlr");
      val_policingprofile_entry->valid[UPLL_IDX_RED_PRIORITY_PPE] =
          UNC_VF_NOT_SOPPORTED;
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  }

  if ((val_policingprofile_entry->valid[UPLL_IDX_RED_DSCP_PPE] == UNC_VF_VALID)
      || (val_policingprofile_entry->valid[UPLL_IDX_RED_DSCP_PPE]
          == UNC_VF_VALID_NO_VALUE)) {
    if (attrs[unc::capa::policingprofile_entry::kCapRedDscp] == 0) {
      UPLL_LOG_DEBUG("RED_DSCP attr is not supported by ctrlr");
      val_policingprofile_entry->valid[UPLL_IDX_RED_DSCP_PPE] =
          UNC_VF_NOT_SOPPORTED;
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  }

  if ((val_policingprofile_entry->valid[UPLL_IDX_RED_DROP_PPE] == UNC_VF_VALID)
      || (val_policingprofile_entry->valid[UPLL_IDX_RED_DROP_PPE]
          == UNC_VF_VALID_NO_VALUE)) {
    if (attrs[unc::capa::policingprofile_entry::kCapRedDrop] == 0) {
      UPLL_LOG_DEBUG("RED_DROP attr is not supported by ctrlr");
      val_policingprofile_entry->valid[UPLL_IDX_RED_DROP_PPE] =
          UNC_VF_NOT_SOPPORTED;
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t PolicingProfileEntryMoMgr::CopyToConfigKey(ConfigKeyVal *&okey,
    ConfigKeyVal *ikey) {
  UPLL_FUNC_TRACE;
  if (NULL == ikey || NULL ==(ikey->get_key()) ||
      NULL != okey) {
    return UPLL_RC_ERR_GENERIC;
  }

  upll_rc_t result_code = UPLL_RC_SUCCESS;
  if (UNC_KT_POLICING_PROFILE == ikey->get_key_type()) {
    key_rename_vnode_info_t *key_rename =
      reinterpret_cast<key_rename_vnode_info_t *>(ikey->get_key());

    key_policingprofile_entry_t *key_policingprofile =
      reinterpret_cast<key_policingprofile_entry_t *>(ConfigKeyVal::Malloc(
            sizeof(key_policingprofile_entry_t)));
    if (!strlen(
          reinterpret_cast<char *>(key_rename->old_policingprofile_name))) {
      free(key_policingprofile);
      return UPLL_RC_ERR_GENERIC;
    }
    uuu::upll_strncpy(
        key_policingprofile->policingprofile_key.policingprofile_name,
        key_rename->old_policingprofile_name,
        (kMaxLenPolicingProfileName + 1));

    okey = new ConfigKeyVal(UNC_KT_POLICING_PROFILE,
        IpctSt::kIpcStKeyPolicingprofileEntry,
        key_policingprofile, NULL);
    // free(key_policingprofile);
  } else if (UNC_KT_FLOWLIST == ikey->get_key_type()) {
    key_rename_vnode_info_t *key_rename =
      reinterpret_cast<key_rename_vnode_info_t *>(ikey->get_key());

    if (!strlen(
          reinterpret_cast<char *>(key_rename->old_flowlist_name))) {
      return UPLL_RC_ERR_GENERIC;
    }

    val_policingprofile_entry_t *val =
      reinterpret_cast<val_policingprofile_entry_t *>(ConfigKeyVal::Malloc(
            sizeof(val_policingprofile_entry_t)));

    uuu::upll_strncpy(val->flowlist,
                      key_rename->old_flowlist_name,
                      (kMaxLenPolicingProfileName + 1));

    val->valid[UPLL_IDX_FLOWLIST_PPE] = UNC_VF_VALID;

    ConfigVal *ckv = new ConfigVal(IpctSt::kIpcStValPolicingprofileEntry, val);
    okey = new ConfigKeyVal(UNC_KT_POLICING_PROFILE,
        IpctSt::kIpcStKeyPolicingprofileEntry, NULL, ckv);
    // free(val);
  }
  if (!okey) {
    return UPLL_RC_ERR_GENERIC;
  }
  return result_code;
}

upll_rc_t PolicingProfileEntryMoMgr::UpdatePolicingProfileEntryRenamed(
    ConfigKeyVal *rename_info, DalDmlIntf *dmi,
    upll_keytype_datatype_t data_type) {
  UPLL_FUNC_TRACE;
  uint8_t rename = 0;  // UNINIT
  uint8_t *ctrlr_id = NULL;
  if (!rename_info || !rename_info->get_key()) {  // COVERITY FORWARD NULL
    return UPLL_RC_ERR_GENERIC;
  }
  GET_USER_DATA_CTRLR(rename_info, ctrlr_id);
  key_rename_vnode_info *key_rename = reinterpret_cast<key_rename_vnode_info *>
      (ConfigKeyVal::Malloc(sizeof(key_rename_vnode_info)));
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *okey = NULL;
  ConfigKeyVal *tmp_key = NULL;
  ConfigKeyVal *ctrlr_tmp_key = NULL;

  result_code = CopyToConfigKey(okey, rename_info);
  if (UPLL_RC_SUCCESS != result_code) {
    free(key_rename);
    return result_code;
  }
  if (!okey)  {
    free(key_rename);
    return UPLL_RC_ERR_GENERIC;
  }
  if (!(okey->get_key())) {
    delete okey;
    free(key_rename);
    return UPLL_RC_ERR_GENERIC;
  }
  uuu::upll_strncpy(
                    key_rename->new_flowlist_name,
                    reinterpret_cast<key_rename_vnode_info *>
                    (rename_info->get_key())->new_flowlist_name,
                    (kMaxLenFlowListName + 1));
  DbSubOp dbop1 = { kOpReadMultiple, kOpMatchNone, kOpInOutFlag };
  result_code = ReadConfigDB(okey, data_type, UNC_OP_READ, dbop1, dmi, MAINTBL);
  if (UPLL_RC_SUCCESS != result_code) {
    delete key_rename;
    return result_code;
  }
  while (okey) {
    result_code = GetChildConfigKey(tmp_key, okey);
    if (result_code != UPLL_RC_SUCCESS) {
      delete key_rename;  // COV RESOURCE LEAK FIC
      return result_code;
    }
    result_code = GetChildConfigKey(ctrlr_tmp_key, okey);
    if (result_code != UPLL_RC_SUCCESS) {
      // COV RESOURCE LEAK FIX
      delete tmp_key;
      delete key_rename;
      return result_code;
    }
    if (!tmp_key || !(tmp_key->get_key())) {  // COVERITY FORWARD NULL
      delete ctrlr_tmp_key;  // COV RESOURCE LEAK FIX
      delete key_rename;
      delete tmp_key;
      return UPLL_RC_ERR_GENERIC;
    }
    // COVERITY FORWARD NULL
    if (!ctrlr_tmp_key || !(ctrlr_tmp_key->get_key())) {
      // COV RESOURCE LEAK FIX
      delete key_rename;
      delete ctrlr_tmp_key;
      delete tmp_key;
      return UPLL_RC_ERR_GENERIC;
    }
    GET_USER_DATA_FLAGS(okey, rename);
    rename = rename || FL_RENAME_FLAG;
    SET_USER_DATA_FLAGS(tmp_key, rename);
    SET_USER_DATA_FLAGS(ctrlr_tmp_key, rename);
    tmp_key->AppendCfgKeyVal(table[MAINTBL]->get_key_type(),
        IpctSt::kIpcInvalidStNum, key_rename, NULL);
    ctrlr_tmp_key->AppendCfgKeyVal(table[MAINTBL]->get_key_type(),
        IpctSt::kIpcInvalidStNum, key_rename, NULL);
    DbSubOp dbop = { kOpNotRead, kOpMatchNone, kOpInOutFlag };
    result_code = UpdateRenameKey(tmp_key, data_type, UNC_OP_UPDATE, dmi, &dbop,
        MAINTBL);
    if (UPLL_RC_SUCCESS != result_code) {
      return result_code;
    }
    SET_USER_DATA_CTRLR(ctrlr_tmp_key, ctrlr_id);
    DbSubOp dbop1 = { kOpNotRead, kOpMatchCtrlr, kOpInOutFlag };
    result_code = UpdateRenameKey(tmp_key, data_type, UNC_OP_UPDATE, dmi,
        &dbop1, CTRLRTBL);
    if (UPLL_RC_SUCCESS != result_code) {
      return result_code;
    }
    if (tmp_key) {
      free(tmp_key->get_key());
      delete tmp_key;
    }
    if (ctrlr_tmp_key) {
      free(ctrlr_tmp_key->get_key());
      delete ctrlr_tmp_key;
    }
    okey = okey->get_next_cfg_key_val();
  }
  if (okey) {  // TODO(Author) has to fix the issue
    free(okey->get_key());
    delete okey;
  }
  delete key_rename;
  return result_code;
}

bool PolicingProfileEntryMoMgr::IsValidKey(void *ikey, uint64_t index) {
  UPLL_FUNC_TRACE;
  key_policingprofile_entry *key =
    reinterpret_cast<key_policingprofile_entry *>(ikey);
  UPLL_LOG_TRACE("Entering IsValidKey");
  bool ret_val = UPLL_RC_SUCCESS;

  if (index == uudst::policingprofile_entry::kDbiPolicingProfileName) {
    ret_val = ValidateKey(
        reinterpret_cast<char *>(key->policingprofile_key.policingprofile_name),
        kMinLenPolicingProfileName, kMaxLenPolicingProfileName);
    if (ret_val != UPLL_RC_SUCCESS) {
      UPLL_LOG_TRACE("Policingprofile Name is not valid(%d)", ret_val);
      return false;
    }
  }
  if (index == uudst::policingprofile_entry::kDbiSequenceNum) {
    if (!ValidateNumericRange(key->sequence_num,
          kMinPolicingProfileSeqNum,
          kMaxPolicingProfileSeqNum, true, true)) {
      UPLL_LOG_DEBUG("Sequence number Syntax validation failed ");
      return false;
    }
  }
  return true;
}

upll_rc_t PolicingProfileEntryMoMgr::GetPolicingProfileEntryCtrlrKeyval(
    ConfigKeyVal *&ppe_keyval, const char *policingprofile_name,
    const char *ctrlr_id) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  if (NULL == policingprofile_name || NULL == ctrlr_id ||
     NULL != ppe_keyval) {
    return UPLL_RC_ERR_GENERIC;
  }

  result_code = GetChildConfigKey(ppe_keyval, NULL);
  if (result_code != UPLL_RC_SUCCESS) {
    return result_code;
  }

  key_policingprofile_entry_t *key_policingprofie =
    reinterpret_cast<key_policingprofile_entry_t *>(ppe_keyval->get_key());
  uuu::upll_strncpy(
      key_policingprofie->policingprofile_key.policingprofile_name,
      policingprofile_name,
      (kMaxLenPolicingProfileName + 1));

  SET_USER_DATA_CTRLR(ppe_keyval, ctrlr_id);

  return UPLL_RC_SUCCESS;
}

upll_rc_t PolicingProfileEntryMoMgr::PolicingProfileEntryCtrlrTblOper(
    const char *policingprofile_name, const char *ctrlr_id, DalDmlIntf *dmi,
    unc_keytype_operation_t oper, upll_keytype_datatype_t dt_type) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *ppe_ckv = NULL;
  result_code = GetPolicingProfileEntryCtrlrKeyval(ppe_ckv,
      policingprofile_name,
      ctrlr_id);
  if (UPLL_RC_SUCCESS != result_code) {
    return result_code;
  }

  if (UNC_OP_CREATE == oper) {
    result_code = CtrlrTblCreate(ppe_ckv, dmi, dt_type);
  } else if (UNC_OP_DELETE == oper) {
    ConfigKeyVal *okey = NULL;
    result_code = GetPolicingprofileKeyVal(okey, ppe_ckv);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("GetPolicingprofileKeyVal failed %d", result_code);
      return result_code;
    }
    PolicingProfileMoMgr *mgr = reinterpret_cast<PolicingProfileMoMgr *>
        (const_cast<MoManager *>(GetMoManager(UNC_KT_POLICING_PROFILE)));
    DbSubOp dbop = { kOpReadExist, kOpMatchCtrlr, kOpInOutNone };
    result_code = mgr->UpdateConfigDB(okey, dt_type, UNC_OP_READ, dmi, &dbop,
      CTRLRTBL);
    if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
      UPLL_LOG_DEBUG("Delete in entry table");
      result_code = CtrlrTblDelete(ppe_ckv, dmi, dt_type);
    } else if (UPLL_RC_ERR_INSTANCE_EXISTS == result_code) {
      UPLL_LOG_DEBUG("Dont delete return success");
      return UPLL_RC_SUCCESS;
    } else {
      UPLL_LOG_DEBUG("Update configdb failed %d", result_code);
      delete ppe_ckv;
      delete okey;
      return result_code;
    }
  } else {
    result_code = UPLL_RC_ERR_NO_SUCH_OPERATION;
  }
  delete ppe_ckv;  // COV RESOURCE LEAK FIX
  return result_code;
}

upll_rc_t PolicingProfileEntryMoMgr::CtrlrTblDelete(
    ConfigKeyVal *ppe_ckv, DalDmlIntf *dmi, upll_keytype_datatype_t dt_type) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *temp_ckv = NULL;
  uint8_t *ctrlr_id = NULL;
  result_code = GetChildConfigKey(temp_ckv, ppe_ckv);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetChildConfigKey failed %d", result_code);
    return result_code;
  }
  DbSubOp dbop1 = { kOpReadMultiple, kOpMatchNone, kOpInOutNone };
  result_code = ReadConfigDB(temp_ckv, dt_type, UNC_OP_READ, dbop1,
                             dmi, MAINTBL);
  if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
    UPLL_LOG_DEBUG("No entry in policingprofile entry maintbl");
    return UPLL_RC_SUCCESS;
  }

  while (temp_ckv) {
    GET_USER_DATA_CTRLR(temp_ckv, ctrlr_id);
    val_policingprofile_entry_t *temp_ppe_val = reinterpret_cast
        <val_policingprofile_entry_t *>(GetVal(temp_ckv));
    if (UNC_VF_VALID == temp_ppe_val->valid[UPLL_IDX_FLOWLIST_PPE ]) {
     FlowListMoMgr *flowlist_mgr = reinterpret_cast<FlowListMoMgr *>
      (const_cast<MoManager *> (GetMoManager(UNC_KT_FLOWLIST)));
     result_code = flowlist_mgr->AddFlowListToController(
          reinterpret_cast<char *>(temp_ppe_val->flowlist), dmi,
          reinterpret_cast<char *>(ctrlr_id) , UNC_OP_DELETE);
      if (result_code != UPLL_RC_SUCCESS) {
        if (UPLL_RC_ERR_INSTANCE_EXISTS == result_code) {
          continue;
        }
        UPLL_LOG_DEBUG("AddFlowListToController failed err code(%d)",
                       result_code)
        delete temp_ckv;
        return result_code;
      }
    }
    temp_ckv = temp_ckv->get_next_cfg_key_val();
  }
  DbSubOp dbop = { kOpNotRead, kOpMatchCtrlr, kOpInOutNone };
  result_code = UpdateConfigDB(ppe_ckv, dt_type, UNC_OP_DELETE, dmi, &dbop,
      CTRLRTBL);
  if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
    return UPLL_RC_SUCCESS;
  } else if (UPLL_RC_SUCCESS != result_code) {
    return result_code;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t PolicingProfileEntryMoMgr::CtrlrTblCreate(
    ConfigKeyVal *ppe_ckv, DalDmlIntf *dmi, upll_keytype_datatype_t dt_type) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  uint8_t *ctrlr_id = NULL;

  DbSubOp dbop = { kOpReadMultiple, kOpMatchNone, kOpInOutNone };

  result_code = ReadConfigDB(ppe_ckv, dt_type, UNC_OP_READ, dbop, dmi, MAINTBL);
  if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
    UPLL_LOG_DEBUG("No entry in policingprofile entry maintbl");
    return UPLL_RC_SUCCESS;
  }
  GET_USER_DATA_CTRLR(ppe_ckv, ctrlr_id);

  IpcReqRespHeader *req_header = NULL;
  req_header = reinterpret_cast<IpcReqRespHeader*>
      (ConfigKeyVal::Malloc(sizeof(IpcReqRespHeader)));

  req_header->operation = UNC_OP_CREATE;
  req_header->datatype = UPLL_DT_CANDIDATE;


  while (ppe_ckv) {
    GET_USER_DATA_CTRLR(ppe_ckv, ctrlr_id);
    val_policingprofile_entry_t *temp_ppe_val = reinterpret_cast
        <val_policingprofile_entry_t *>(GetVal(ppe_ckv));
    if (UNC_VF_VALID == temp_ppe_val->valid[UPLL_IDX_FLOWLIST_PPE ]) {
     FlowListMoMgr *flowlist_mgr = reinterpret_cast<FlowListMoMgr *>
      (const_cast<MoManager *> (GetMoManager(UNC_KT_FLOWLIST)));
     result_code = flowlist_mgr->AddFlowListToController(
          reinterpret_cast<char *>(temp_ppe_val->flowlist), dmi,
          reinterpret_cast<char *>(ctrlr_id) , UNC_OP_CREATE);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("AddFlowListToController failed err code(%d)",
                       result_code)
        free(req_header);
        return result_code;
      }
    }
    ConfigKeyVal *new_ppe = NULL;
    result_code = GetChildConfigKey(new_ppe, ppe_ckv);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("GetChildConfigKey failed %d", result_code);
      free(req_header);
      return result_code;
    }
    SET_USER_DATA_CTRLR(new_ppe, ctrlr_id);
    DbSubOp dbop1 = { kOpReadSingle, kOpMatchCtrlr, kOpInOutNone };
    result_code = ReadConfigDB(new_ppe, dt_type, UNC_OP_READ, dbop1, dmi,
                                 CTRLRTBL);
    if (UPLL_RC_SUCCESS == result_code) {
      ppe_ckv = ppe_ckv->get_next_cfg_key_val();
      continue;
    } else if (UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
      UPLL_LOG_DEBUG("CtrlrTbl create failed %d", result_code);
      free(req_header);
      return result_code;
    }
    val_policingprofile_entry_ctrl_t *val_ctrlr =
      reinterpret_cast<val_policingprofile_entry_ctrl_t*>(GetVal(new_ppe));
    if (NULL == val_ctrlr) {
      free(req_header);
      return UPLL_RC_ERR_GENERIC;
    }
    val_policingprofile_entry_t *val =
      reinterpret_cast<val_policingprofile_entry_t *>(GetVal(ppe_ckv));
    val_ctrlr->cs_row_status = (unc_keytype_configstatus_t)val->cs_row_status;
    /*result_code = ValidateCapability(req_header, ppe_ckv,
        reinterpret_cast<const char *>(ctrlr_id));
    if (result_code != UPLL_RC_SUCCESS
        || result_code != UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR) {
      free(val_ctrlr);
      delete ppe_ckv;
      free(req_header);
      return result_code;
    }*/
    // TODO(Author) has to fix th eissue DEAD CODE
    for (unsigned int loop = 0;
         loop < (sizeof(val_ctrlr->valid)/sizeof(val_ctrlr->valid[0]));
         loop++) {
      val_ctrlr->valid[loop] = val->valid[loop];
      UPLL_LOG_DEBUG("valid of %d - %d  ctrlrtbl - %d", loop,  val->valid[loop],
                       val_ctrlr->valid[loop]);
    }
    // ppe_ckv(Author) has to fic the issue UNUSED_VALUE
    string s(new_ppe->ToStrAll());
    UPLL_LOG_INFO("%s new_ppe Policingprofile", s.c_str());
    string s1(ppe_ckv->ToStrAll());
    UPLL_LOG_INFO("%s ppe_ckv Policingprofile", s1.c_str());
    result_code = UpdateConfigDB(new_ppe, dt_type, UNC_OP_CREATE, dmi,
        CTRLRTBL);
    if (UPLL_RC_SUCCESS != result_code) {
      delete new_ppe;
      free(req_header);
      return result_code;
    }
    ppe_ckv = ppe_ckv->get_next_cfg_key_val();
    delete new_ppe;
  }
  free(req_header);
  return UPLL_RC_SUCCESS;
}

upll_rc_t PolicingProfileEntryMoMgr::SetConsolidatedStatus(ConfigKeyVal *ikey,
    DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *ckv = NULL;
  DbSubOp dbop = { kOpReadMultiple, kOpMatchNone, kOpInOutNone };
  result_code = GetChildConfigKey(ckv, ikey);
  if (UPLL_RC_SUCCESS != result_code) {
    return result_code;
  }
  result_code = ReadConfigDB(ckv, UPLL_DT_RUNNING, UNC_OP_READ, dbop, dmi,
      CTRLRTBL);
  if (UPLL_RC_SUCCESS != result_code) {
    delete ckv;
    return result_code;
  }
  std::vector < list<unc_keytype_configstatus_t> > vec_attr;
  std::list < unc_keytype_configstatus_t > list_cs_row;
  val_policingprofile_entry_t *val;
  for (unsigned int loop = 0; loop < sizeof(val->valid)/sizeof(val->valid[0]);
      ++loop) {
    std::list < unc_keytype_configstatus_t > list_attr;
    vec_attr.push_back(list_attr);
  }
  for (; ckv != NULL; ckv = ckv->get_next_cfg_key_val()) {
    val = reinterpret_cast<val_policingprofile_entry_t *>(GetVal(ckv));
    list_cs_row.push_back((unc_keytype_configstatus_t) val->cs_row_status);
    for (unsigned int loop = 0; loop < sizeof(val->valid)/
        sizeof(val->valid[0]); ++loop) {
      vec_attr[loop].push_back((unc_keytype_configstatus_t)
        val->cs_attr[loop]);
    }
  }
  val_policingprofile_entry_t *val_temp =
    reinterpret_cast<val_policingprofile_entry_t *>(GetVal(ikey));
  val_temp->cs_row_status = GetConsolidatedCsStatus(list_cs_row);
  for (unsigned int loop = 0; loop < sizeof(val->valid)/
      sizeof(val->valid[0]); ++loop) {
    val_temp->cs_attr[loop] = GetConsolidatedCsStatus(vec_attr[loop]);
  }
  result_code = UpdateConfigDB(ikey, UPLL_DT_RUNNING, UNC_OP_UPDATE, dmi,
      MAINTBL);
  if (UPLL_RC_SUCCESS != result_code) {
    return result_code;
  }
  return result_code;
}

upll_rc_t PolicingProfileEntryMoMgr::ReadPolicingProfileEntry(
    const char *policingprofile_name, uint8_t seq_num, const char *ctrlr_id,
    DalDmlIntf *dmi, upll_keytype_datatype_t dt_type, ConfigKeyVal *&ppe_ckv) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code;
  if (NULL == policingprofile_name || (!ctrlr_id) || (!dmi)) {  // FORWARD NULL
    return UPLL_RC_ERR_GENERIC;
  }
  if (NULL != ppe_ckv) {
    return UPLL_RC_ERR_GENERIC;
  }
  UPLL_LOG_DEBUG("Sequence number - %d", seq_num);
  result_code = GetChildConfigKey(ppe_ckv, NULL);
  if (UPLL_RC_SUCCESS != result_code) {
    return result_code;
  }
  key_policingprofile_entry_t *key =
    reinterpret_cast<key_policingprofile_entry_t *>(ppe_ckv->get_key());
  if (!key) {
    delete ppe_ckv;
    return UPLL_RC_ERR_GENERIC;
  }
  uuu::upll_strncpy(
      key->policingprofile_key.policingprofile_name,
      policingprofile_name,
      (kMaxLenPolicingProfileName + 1));
  key->sequence_num = seq_num;
  DbSubOp dbop = { kOpReadMultiple, kOpMatchNone, kOpInOutCs };
  result_code = ReadConfigDB(ppe_ckv, dt_type, UNC_OP_READ, dbop, dmi, MAINTBL);
  if (UPLL_RC_SUCCESS != result_code) {
    delete ppe_ckv;
    return result_code;
  }
  ConfigKeyVal *temp_ckv = ppe_ckv;
  while (NULL != temp_ckv) {
    ConfigKeyVal *ctrlr_ckv = NULL;
    result_code = GetChildConfigKey(ctrlr_ckv, temp_ckv);
    if (UPLL_RC_SUCCESS != result_code) {
      delete ppe_ckv;
      return result_code;
    }
    SET_USER_DATA_CTRLR(ctrlr_ckv, ctrlr_id);
    DbSubOp dbop1 = { kOpReadSingle, kOpMatchCtrlr, kOpInOutCs };
    result_code = ReadConfigDB(ctrlr_ckv, dt_type, UNC_OP_READ, dbop1, dmi,
        CTRLRTBL);
    if (UPLL_RC_SUCCESS != result_code) {
      delete ppe_ckv;
      delete ctrlr_ckv;
      return result_code;
    }
    val_policingprofile_entry_t *val =
      reinterpret_cast<val_policingprofile_entry_t *>(GetVal(temp_ckv));
    val_policingprofile_entry_ctrl_t *ctrl_val =
      reinterpret_cast<val_policingprofile_entry_ctrl_t *>(GetVal(ctrlr_ckv));
    val->cs_row_status = ctrl_val->cs_row_status;
    for (unsigned int loop = 0; loop <
        sizeof(val->valid[loop]/sizeof(val->valid[0])); loop++) {
      val->valid[loop] = ctrl_val->valid[loop];
      val->cs_attr[loop] = ctrl_val->cs_attr[loop];
    }
    temp_ckv = temp_ckv->get_next_cfg_key_val();
    delete ctrlr_ckv;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t PolicingProfileEntryMoMgr::GetValid(void *val, uint64_t index,
    uint8_t *&valid,
    upll_keytype_datatype_t dt_type,
    MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  val_policingprofile_entry_t *val_ppe = NULL;
  val_policingprofile_entry_ctrl_t *val_ppe_ctrl = NULL;

  if (tbl == MAINTBL) {
    val_ppe = reinterpret_cast<val_policingprofile_entry_t *>(val);
  } else if (tbl == CTRLRTBL) {
    val_ppe_ctrl = reinterpret_cast<val_policingprofile_entry_ctrl_t *>(val);
  } else {
    valid = NULL;
    UPLL_LOG_DEBUG("Inavlid Table");
    return UPLL_RC_ERR_GENERIC;
  }
  switch (index) {
    case uudst::policingprofile_entry::kDbiFlowlist:
      valid  = GET_VALID_MAINCTRL(tbl, val_ppe_ctrl, val_ppe,
                                   UPLL_IDX_FLOWLIST_PPE);
        break;

    case uudst::policingprofile_entry::kDbiRate:
      valid  = GET_VALID_MAINCTRL(tbl, val_ppe_ctrl, val_ppe,
                                   UPLL_IDX_RATE_PPE);
        break;

    case uudst::policingprofile_entry::kDbiCir:
      valid  = GET_VALID_MAINCTRL(tbl, val_ppe_ctrl, val_ppe,
                                   UPLL_IDX_CIR_PPE);
        break;

    case uudst::policingprofile_entry::kDbiCbs:
      valid  = GET_VALID_MAINCTRL(tbl, val_ppe_ctrl, val_ppe,
                                   UPLL_IDX_CBS_PPE);
        break;

    case uudst::policingprofile_entry::kDbiPir:
      valid  = GET_VALID_MAINCTRL(tbl, val_ppe_ctrl, val_ppe,
                                   UPLL_IDX_PIR_PPE);
        break;

    case uudst::policingprofile_entry::kDbiPbs:
      valid  = GET_VALID_MAINCTRL(tbl, val_ppe_ctrl, val_ppe,
                                   UPLL_IDX_PBS_PPE);
        break;

    case uudst::policingprofile_entry::kDbiGreenAction:
      valid  = GET_VALID_MAINCTRL(tbl, val_ppe_ctrl, val_ppe,
                                   UPLL_IDX_GREEN_ACTION_PPE);
        break;

    case uudst::policingprofile_entry::kDbiGreenPriority:
      valid  = GET_VALID_MAINCTRL(tbl, val_ppe_ctrl, val_ppe,
                                   UPLL_IDX_GREEN_PRIORITY_PPE);
        break;

    case uudst::policingprofile_entry::kDbiGreenDscp:
      valid  = GET_VALID_MAINCTRL(tbl, val_ppe_ctrl, val_ppe,
                                   UPLL_IDX_GREEN_DSCP_PPE);
        break;

    case uudst::policingprofile_entry::kDbiGreenDrop:
      valid  = GET_VALID_MAINCTRL(tbl, val_ppe_ctrl, val_ppe,
                                   UPLL_IDX_GREEN_DROP_PPE);
        break;

    case uudst::policingprofile_entry::kDbiYellowAction:
      valid  = GET_VALID_MAINCTRL(tbl, val_ppe_ctrl, val_ppe,
                                   UPLL_IDX_YELLOW_ACTION_PPE);
        break;

    case uudst::policingprofile_entry::kDbiYellowPriority:
      valid  = GET_VALID_MAINCTRL(tbl, val_ppe_ctrl, val_ppe,
                                   UPLL_IDX_YELLOW_PRIORITY_PPE);
        break;

    case uudst::policingprofile_entry::kDbiYellowDscp:
      valid  = GET_VALID_MAINCTRL(tbl, val_ppe_ctrl, val_ppe,
                                   UPLL_IDX_YELLOW_DSCP_PPE);
        break;

    case uudst::policingprofile_entry::kDbiYellowDrop:
      valid  = GET_VALID_MAINCTRL(tbl, val_ppe_ctrl, val_ppe,
                                   UPLL_IDX_YELLOW_DROP_PPE);
        break;

    case uudst::policingprofile_entry::kDbiRedAction:
      valid  = GET_VALID_MAINCTRL(tbl, val_ppe_ctrl, val_ppe,
                                   UPLL_IDX_RED_ACTION_PPE);
        break;

    case uudst::policingprofile_entry::kDbiRedPriority:
      valid  = GET_VALID_MAINCTRL(tbl, val_ppe_ctrl, val_ppe,
                                   UPLL_IDX_RED_PRIORITY_PPE);
        break;

    case uudst::policingprofile_entry::kDbiRedDscp:
      valid  = GET_VALID_MAINCTRL(tbl, val_ppe_ctrl, val_ppe,
                                   UPLL_IDX_RED_DSCP_PPE);
        break;

    case uudst::policingprofile_entry::kDbiRedDrop:
      valid  = GET_VALID_MAINCTRL(tbl, val_ppe_ctrl, val_ppe,
                                   UPLL_IDX_RED_DROP_PPE);
        break;

    default :
      valid = NULL;
      break;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t PolicingProfileEntryMoMgr::UpdateConfigStatus(ConfigKeyVal *key,
    unc_keytype_operation_t op, uint32_t driver_result, ConfigKeyVal *nreq,
    DalDmlIntf *dmi, ConfigKeyVal *ctrlr_key) {
  UPLL_FUNC_TRACE;
  // char obj;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  unc_keytype_configstatus_t status = UNC_CS_UNKNOWN,
      cs_status = UNC_CS_UNKNOWN;
  cs_status = (driver_result == 0) ? UNC_CS_APPLIED : UNC_CS_NOT_APPLIED;
  if ((NULL == ctrlr_key) || (NULL == key)) return UPLL_RC_ERR_GENERIC;
  val_policingprofile_entry_t *ppe_val =
      reinterpret_cast<val_policingprofile_entry_t *>(GetVal(key));
  val_policingprofile_entry_ctrl *ppe_ctrlr_val =
      reinterpret_cast<val_policingprofile_entry_ctrl*>(GetVal(ctrlr_key));
  if ((ppe_val == NULL) || (NULL == ppe_ctrlr_val)) {
    UPLL_LOG_DEBUG("ppe_val not supported :-");
    return UPLL_RC_ERR_GENERIC;
  }
  if (op == UNC_OP_CREATE) {
    switch (ppe_val->cs_row_status) {
    case UNC_CS_UNKNOWN:
      status = cs_status;
      break;
    case UNC_CS_PARTAILLY_APPLIED:
      if (ppe_ctrlr_val->cs_row_status == UNC_CS_NOT_APPLIED) {
        /* changes need to do */
      }
    case UNC_CS_APPLIED:
    case UNC_CS_NOT_APPLIED:
    case UNC_CS_INVALID:
    default:
      status =
          (cs_status == UNC_CS_APPLIED) ? UNC_CS_PARTAILLY_APPLIED : status;
      break;
    }
    ppe_val->cs_row_status = status;
    for (unsigned int loop = 0;
         loop < sizeof(ppe_val->valid)/sizeof(ppe_val->valid[0]);
         ++loop) {
        // Setting CS to the not supported attributes
        if (UNC_VF_NOT_SOPPORTED == ppe_val->valid[loop]) {
          ppe_val->cs_attr[loop] = UNC_CS_NOT_SUPPORTED;
          continue;
        }
        if (UNC_VF_NOT_SOPPORTED == ppe_ctrlr_val->valid[loop]) {
          ppe_ctrlr_val->cs_attr[loop] = UNC_CS_NOT_SUPPORTED;
          continue;
        }
        if ((UNC_VF_VALID == ppe_val->valid[loop]) ||
           (UNC_VF_VALID_NO_VALUE == ppe_val->valid[loop]))
          if (ppe_ctrlr_val->valid[loop] != UNC_VF_NOT_SOPPORTED) {
            ppe_ctrlr_val->cs_attr[loop] = cs_status;
            ppe_val->cs_attr[loop] = (uint8_t)ppe_val->cs_row_status;
          }
    }

  } else if (op == UNC_OP_UPDATE) {
    // void *flowlistentryval = NULL;
    void* ppeval_1 = GetVal(key);
    void* ppeval_2 = GetVal(nreq);
    CompareValidValue(ppeval_1, ppeval_2, false);
    for (unsigned int loop = 0;
        loop < sizeof(ppe_val->valid) / sizeof(ppe_val->valid[0]);
        ++loop) {
        if (ppe_ctrlr_val->valid[loop] != UNC_VF_NOT_SOPPORTED) {
          ppe_ctrlr_val->cs_attr[loop] = cs_status;
        } else {
          ppe_ctrlr_val->cs_attr[loop] = UNC_CS_NOT_SUPPORTED;
        }
          ppe_val->cs_attr[loop] = (uint8_t)ppe_val->cs_row_status;
    }
  }
  return result_code;
}


#if 0
upll_rc_t PolicingProfileEntryMoMgr::UpdateConfigStatus(ConfigKeyVal *ppe_key,
    unc_keytype_operation_t op, uint32_t driver_result, ConfigKeyVal *upd_key,
    DalDmlIntf *dmi, ConfigKeyVal *ctrlr_key) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  val_policingprofile_entry_t *ppe_val = NULL;

  unc_keytype_configstatus_t cs_status =
      (driver_result == UPLL_RC_SUCCESS) ? UNC_CS_APPLIED : UNC_CS_NOT_APPLIED;
  ppe_val = reinterpret_cast<val_policingprofile_entry_t *>(GetVal(ppe_key));
  if (ppe_val == NULL) {
    return UPLL_RC_ERR_GENERIC;
  }
  ppe_val->cs_row_status = cs_status;
  if (op == UNC_OP_CREATE) {
    for (unsigned int loop = 0; loop < sizeof(ppe_val->valid) /
        sizeof(ppe_val->valid[0]); ++loop) {
      /* Setting CS to the not supported attributes */
      if (UNC_VF_NOT_SOPPORTED == ppe_val->valid[loop]) {
        ppe_val->cs_attr[loop] = UNC_CS_NOT_SUPPORTED;
        continue;
      }
      if ((UNC_VF_VALID == ppe_val->valid[loop])
          || (UNC_VF_VALID_NO_VALUE == ppe_val->valid[loop]))
        ppe_val->cs_attr[loop] = ppe_val->cs_row_status;
    }
  } else if (op == UNC_OP_UPDATE) {
    void *ppeval = reinterpret_cast<void *>(&ppe_val);
    CompareValidValue(ppeval, GetVal(upd_key), false);
    for (unsigned int loop = 0; loop < sizeof(ppe_val->valid) /
        sizeof(ppe_val->valid[0]); ++loop) {
      if (ppe_val->valid[loop] != UNC_VF_NOT_SOPPORTED) {
        if ((UNC_VF_VALID == ppe_val->valid[loop])
            || (UNC_VF_VALID_NO_VALUE == ppe_val->valid[loop]))
          ppe_val->cs_attr[loop] = ppe_val->cs_row_status;
      } else {
        ppe_val->cs_attr[loop] = UNC_CS_NOT_SUPPORTED;
      }
    }
  } else {
    return UPLL_RC_ERR_GENERIC;
  }
  return result_code;
}
#endif
upll_rc_t PolicingProfileEntryMoMgr::IsFlowlistConfigured(
  const char* flowlist_name, DalDmlIntf *dmi) {
  upll_rc_t result_code = UPLL_RC_ERR_GENERIC;
  ConfigKeyVal *ckv = NULL;
  result_code = GetChildConfigKey(ckv, NULL);
  if (UPLL_RC_SUCCESS != result_code) {
    return result_code;
  }
  val_policingprofile_entry_t *ppe_val =
      reinterpret_cast<val_policingprofile_entry_t*>
      (ConfigKeyVal::Malloc(sizeof(val_policingprofile_entry_t)));
  uuu::upll_strncpy(ppe_val->flowlist, flowlist_name,
      (kMaxLenFlowListName + 1));
  ppe_val->valid[UPLL_IDX_POLICERNAME_PM] = UNC_VF_VALID;
  ckv->AppendCfgVal(IpctSt::kIpcStValPolicingprofileEntry, ppe_val);
  DbSubOp dbop = {kOpReadMultiple, kOpMatchNone, kOpInOutNone};
  result_code = ReadConfigDB(ckv, UPLL_DT_CANDIDATE, UNC_OP_READ, dbop,
                             dmi, MAINTBL);
  delete ckv;
  return result_code;
}

upll_rc_t PolicingProfileEntryMoMgr::ReadDetailEntry(
    ConfigKeyVal *ff_ckv, upll_keytype_datatype_t dt_type,
    DbSubOp dbop, DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_ERR_GENERIC;
  if (NULL == ff_ckv) {
    return UPLL_RC_ERR_GENERIC;
  }

  result_code = ReadConfigDB(ff_ckv, dt_type, UNC_OP_READ, dbop, dmi, MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    return result_code;
  }
  return result_code;
}

upll_rc_t PolicingProfileEntryMoMgr::ValidateMessage(IpcReqRespHeader *req,
                                                     ConfigKeyVal *key) {
  UPLL_FUNC_TRACE;

  upll_rc_t rt_code = UPLL_RC_ERR_GENERIC;

  if ((NULL == req) || (NULL == key)) {
    UPLL_LOG_DEBUG("IpcReqRespHeader/ConfigKeyval is NULL");
    return UPLL_RC_ERR_BAD_REQUEST;
  }

  if (UNC_KT_POLICING_PROFILE_ENTRY != key->get_key_type()) {
    UPLL_LOG_DEBUG("Received keytype (%d) is not KT_POLICINGPROFILE_ENTRY!!",
        key->get_key_type());
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
      (rt_code = ValidatePolicingProfileEntryKey(key, req->operation))) {
    UPLL_LOG_DEBUG("KT_POLICING_PROFILE_ENTRY key structure syntax "
                    "validation failed: Err code-%d",
                     rt_code);
    return rt_code;
  }

  if (UPLL_RC_SUCCESS != (rt_code = ValidatePolicingprofileEntryVal(
              key, req->operation, req->datatype))) {
    UPLL_LOG_DEBUG("KT_POLICING_PROFILE_ENTRY val structure syntax "
                   "validation failed: Err code-%d",
                   rt_code);
    return rt_code;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t PolicingProfileEntryMoMgr::ValidatePolicingProfileEntryKey(
    ConfigKeyVal *key, uint32_t op) {
  UPLL_FUNC_TRACE;
  upll_rc_t rt_code = UPLL_RC_ERR_GENERIC;

  if (key->get_st_num() != IpctSt::kIpcStKeyPolicingprofileEntry) {
    UPLL_LOG_DEBUG("Invalid key structure received. struct num - %d",
                   key->get_st_num());
    return UPLL_RC_ERR_BAD_REQUEST;
  }

  /** Read key structure */
  key_policingprofile_entry_t *key_policingprofile_entry =
      reinterpret_cast<key_policingprofile_entry_t *>(key->get_key());
  if (NULL == key_policingprofile_entry) {
    UPLL_LOG_DEBUG("KT_POLICING_PROFILE_ENTRY Key structure is empty!!");
    return UPLL_RC_ERR_BAD_REQUEST;
  }

  rt_code = ValidateKey(
      reinterpret_cast<char *>(key_policingprofile_entry->
        policingprofile_key.policingprofile_name),
        kMinLenPolicingProfileName, kMaxLenPolicingProfileName);

  if (UPLL_RC_SUCCESS != rt_code) {
    UPLL_LOG_DEBUG("PolicingProfile Name(%s) syntax validation failed: "
        "err code - %d", key_policingprofile_entry->policingprofile_key.
        policingprofile_name , rt_code);
    return rt_code;
  }

  if ((op != UNC_OP_READ_SIBLING_COUNT) &&
      (op != UNC_OP_READ_SIBLING_BEGIN)) {
   /** when policingprofile name is set to "flood",
    *  seq_num should be set as 1 */
    if (strcmp(reinterpret_cast<char *>(key_policingprofile_entry->
               policingprofile_key.policingprofile_name), "flood") == 0) {
      if (key_policingprofile_entry->sequence_num == 1) {
        UPLL_LOG_DEBUG("valid sequence_num filled for profile_name-flood");
        return UPLL_RC_SUCCESS;
      } else {
        UPLL_LOG_DEBUG("Error Invalid sequence_num for policing name-flood(%d)",
            key_policingprofile_entry->sequence_num);
        return UPLL_RC_ERR_CFG_SEMANTIC;
      }
    }
    /** Validate sequence Number*/
    if (!ValidateNumericRange(key_policingprofile_entry->sequence_num,
                              kMinPolicingProfileSeqNum,
                              kMaxPolicingProfileSeqNum, true, true)) {
      UPLL_LOG_DEBUG("SeqNum(%d) syntax validation failed: err code - %d",
                      key_policingprofile_entry->sequence_num, rt_code);
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  } else {
    if (key_policingprofile_entry->sequence_num) {
      // reset the sequence number
      // for sibling read and count
      // operation
      key_policingprofile_entry->sequence_num = 0;
    }
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t PolicingProfileEntryMoMgr::ValidatePolicingprofileEntryVal(
    ConfigKeyVal *key,
    uint32_t operation,
    uint32_t datatype) {
  UPLL_FUNC_TRACE;

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
  if ((key->get_cfg_val()->get_st_num() !=
       IpctSt::kIpcStValPolicingprofileEntry))  {
    UPLL_LOG_DEBUG("Invalid val structure received. struct num - %d",
                   (key->get_cfg_val())->get_st_num());
    return UPLL_RC_ERR_BAD_REQUEST;
  }

  val_policingprofile_entry_t *val_ppe =
      reinterpret_cast<val_policingprofile_entry_t *>
      (key->get_cfg_val()->get_val());

  if (val_ppe == NULL) {
    UPLL_LOG_DEBUG("KT_POLICING_PROFILE_ENTRY val structure is null");
    return UPLL_RC_ERR_BAD_REQUEST;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t PolicingProfileEntryMoMgr::UpdateMo(IpcReqRespHeader *req,
                              ConfigKeyVal *ikey,
                              DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;

  upll_rc_t result_code = UPLL_RC_ERR_GENERIC;
  if (NULL == ikey || NULL == req || !(ikey->get_key())) {
     UPLL_LOG_ERROR("Given Input is Empty");
     return UPLL_RC_ERR_GENERIC;
  }
  UPLL_LOG_ERROR("UpdateMo for %d", ikey->get_key_type());
  result_code = ValidateMessage(req, ikey);
  if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_ERROR("Validation Message is Failed ");
      return result_code;
  }
  ConfigKeyVal *okey = NULL;
  result_code = GetChildConfigKey(okey, ikey);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetChildConfigKey Failed %d", result_code);
    return result_code;
  }
  DbSubOp dbop = {kOpReadSingle, kOpMatchNone,
    kOpInOutCtrlr | kOpInOutDomain | kOpInOutFlag};
  result_code = ReadConfigDB(okey, req->datatype, UNC_OP_READ,
                             dbop, dmi, MAINTBL);
  if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_ERROR("Record does Not Exists");
      delete okey;
      return result_code;
  }
  result_code = ValidateAttribute(ikey, dmi, req);
  if (UPLL_RC_SUCCESS  != result_code) {
      UPLL_LOG_ERROR("Validate Attribute is Failed");
      return result_code;
  }
  result_code = UpdateConfigDB(ikey, req->datatype, UNC_OP_UPDATE,
                               dmi, MAINTBL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_ERROR("Updation Failure in DB : %d", result_code);
    return result_code;
  }
  UPLL_LOG_TRACE("Updated Done Successfully %d", result_code);
  return result_code;
}

upll_rc_t PolicingProfileEntryMoMgr::ValidateAttribute(ConfigKeyVal *ikey,
                                                       DalDmlIntf *dmi,
                                                       IpcReqRespHeader *req) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  result_code = ValidatePolicingProfileName(ikey, dmi, req);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("ValidatePolicingProfileName failed %d", result_code);
    return result_code;
  }

  result_code = ValidatePolicingProfileEntryValue(ikey, dmi, req);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("ValidatePolicingProfileEntryValue failed %d", result_code);
    return result_code;
  }

  return result_code;
}

upll_rc_t PolicingProfileEntryMoMgr::ValidatePolicingProfileName(
    ConfigKeyVal *ikey, DalDmlIntf *dmi,
    IpcReqRespHeader *req) {
  UPLL_FUNC_TRACE;
  upll_rc_t rt_val = UPLL_RC_ERR_GENERIC;

  /* validate policingprofile name */
  key_policingprofile_entry_t *ppe_key =reinterpret_cast
    <key_policingprofile_entry_t *>(ikey->get_key());

  /** check policingprofile_name exists in KT_POLICINGPROFILE table */
  MoMgrImpl *mgr =
    reinterpret_cast<MoMgrImpl *>(const_cast<MoManager*>(GetMoManager(
            UNC_KT_POLICING_PROFILE)));
  if (NULL == mgr) {
    return UPLL_RC_ERR_GENERIC;
  }
  ConfigKeyVal *pp_ckv = NULL;
  rt_val = mgr->GetChildConfigKey(pp_ckv, NULL);
  if (UPLL_RC_SUCCESS != rt_val) {
    UPLL_LOG_DEBUG("GetChildConfigKey fails %d", rt_val);
    return rt_val;
  }
  key_policingprofile_t *pp_key = reinterpret_cast<key_policingprofile_t *>
      (pp_ckv->get_key());
  if (NULL == pp_key) {
    UPLL_LOG_DEBUG("PolicingProfile key is NULL");
    delete pp_ckv;
    return UPLL_RC_ERR_GENERIC;
  }
  uuu::upll_strncpy(pp_key->policingprofile_name,
    ppe_key->policingprofile_key.policingprofile_name,
    kMaxLenPolicingProfileName+1);

  rt_val = mgr->UpdateConfigDB(pp_ckv, req->datatype,
                UNC_OP_READ, dmi);

  if (rt_val != UPLL_RC_ERR_INSTANCE_EXISTS) {
    UPLL_LOG_DEBUG("policingprofile name does not exists in "
        "KT_POLICINGPROFILE table");
    delete pp_ckv;
    return UPLL_RC_ERR_PARENT_DOES_NOT_EXIST;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t PolicingProfileEntryMoMgr::ValidatePolicingProfileEntryValue(
    ConfigKeyVal *ikey, DalDmlIntf *dmi,
    IpcReqRespHeader *req) {
  UPLL_FUNC_TRACE;
  upll_rc_t rt_code = UPLL_RC_ERR_GENERIC;
  bool check = false;
  val_policingprofile_entry_t *val_ppe = reinterpret_cast
    <val_policingprofile_entry_t *>(GetVal(ikey));

  if (val_ppe != NULL) {
    rt_code = ValidateFlowList(ikey, dmi, req);
    if (UPLL_RC_SUCCESS != rt_code) {
      UPLL_LOG_DEBUG("Validate flowlist failed %d", rt_code);
      return rt_code;
    }
    uint32_t operation = req->operation;
    if (UNC_OP_CREATE == operation) {
      if ((val_ppe->valid[UPLL_IDX_RATE_PPE] == UNC_VF_VALID)
      || (val_ppe->valid[UPLL_IDX_CIR_PPE] == UNC_VF_VALID)
      || (val_ppe->valid[UPLL_IDX_CBS_PPE] == UNC_VF_VALID)
      || (val_ppe->valid[UPLL_IDX_PIR_PPE] == UNC_VF_VALID)
      || (val_ppe->valid[UPLL_IDX_PBS_PPE] == UNC_VF_VALID)
      || (val_ppe->valid[UPLL_IDX_GREEN_ACTION_PPE] == UNC_VF_VALID)
      || (val_ppe->valid[UPLL_IDX_GREEN_PRIORITY_PPE] == UNC_VF_VALID)
      || (val_ppe->valid[UPLL_IDX_GREEN_DSCP_PPE] == UNC_VF_VALID)
      || (val_ppe->valid[UPLL_IDX_GREEN_DROP_PPE] == UNC_VF_VALID)
      || (val_ppe->valid[UPLL_IDX_YELLOW_ACTION_PPE] == UNC_VF_VALID)
      || (val_ppe->valid[UPLL_IDX_YELLOW_PRIORITY_PPE] == UNC_VF_VALID)
      || (val_ppe->valid[UPLL_IDX_YELLOW_DSCP_PPE] == UNC_VF_VALID)
      || (val_ppe->valid[UPLL_IDX_YELLOW_DROP_PPE] == UNC_VF_VALID)
      || (val_ppe->valid[UPLL_IDX_RED_ACTION_PPE] == UNC_VF_VALID)
      || (val_ppe->valid[UPLL_IDX_RED_PRIORITY_PPE] == UNC_VF_VALID)
      || (val_ppe->valid[UPLL_IDX_RED_DSCP_PPE] == UNC_VF_VALID)
      || (val_ppe->valid[UPLL_IDX_RED_DROP_PPE] == UNC_VF_VALID)) {
        check = true;
      }
    } else if (UNC_OP_UPDATE == operation) {
      if ((val_ppe->valid[UPLL_IDX_RATE_PPE] == UNC_VF_VALID)
      || (val_ppe->valid[UPLL_IDX_CIR_PPE] == UNC_VF_VALID)
      || (val_ppe->valid[UPLL_IDX_CBS_PPE] == UNC_VF_VALID)
      || (val_ppe->valid[UPLL_IDX_PIR_PPE] == UNC_VF_VALID)
      || (val_ppe->valid[UPLL_IDX_PBS_PPE] == UNC_VF_VALID)
      || (val_ppe->valid[UPLL_IDX_GREEN_ACTION_PPE] == UNC_VF_VALID)
      || (val_ppe->valid[UPLL_IDX_GREEN_PRIORITY_PPE] == UNC_VF_VALID)
      || (val_ppe->valid[UPLL_IDX_GREEN_DSCP_PPE] == UNC_VF_VALID)
      || (val_ppe->valid[UPLL_IDX_GREEN_DROP_PPE] == UNC_VF_VALID)
      || (val_ppe->valid[UPLL_IDX_YELLOW_ACTION_PPE] == UNC_VF_VALID)
      || (val_ppe->valid[UPLL_IDX_YELLOW_PRIORITY_PPE] == UNC_VF_VALID)
      || (val_ppe->valid[UPLL_IDX_YELLOW_DSCP_PPE] == UNC_VF_VALID)
      || (val_ppe->valid[UPLL_IDX_YELLOW_DROP_PPE] == UNC_VF_VALID)
      || (val_ppe->valid[UPLL_IDX_RED_ACTION_PPE] == UNC_VF_VALID)
      || (val_ppe->valid[UPLL_IDX_RED_PRIORITY_PPE] == UNC_VF_VALID)
      || (val_ppe->valid[UPLL_IDX_RED_DSCP_PPE] == UNC_VF_VALID)
      || (val_ppe->valid[UPLL_IDX_RED_DROP_PPE] == UNC_VF_VALID)
      || (val_ppe->valid[UPLL_IDX_RATE_PPE] == UNC_VF_VALID_NO_VALUE)
      || (val_ppe->valid[UPLL_IDX_CIR_PPE] == UNC_VF_VALID_NO_VALUE)
      || (val_ppe->valid[UPLL_IDX_CBS_PPE] == UNC_VF_VALID_NO_VALUE)
      || (val_ppe->valid[UPLL_IDX_PIR_PPE] == UNC_VF_VALID_NO_VALUE)
      || (val_ppe->valid[UPLL_IDX_PBS_PPE] == UNC_VF_VALID_NO_VALUE)
      || (val_ppe->valid[UPLL_IDX_GREEN_ACTION_PPE] == UNC_VF_VALID_NO_VALUE)
      || (val_ppe->valid[UPLL_IDX_GREEN_PRIORITY_PPE] == UNC_VF_VALID_NO_VALUE)
      || (val_ppe->valid[UPLL_IDX_GREEN_DSCP_PPE] == UNC_VF_VALID_NO_VALUE)
      || (val_ppe->valid[UPLL_IDX_GREEN_DROP_PPE] == UNC_VF_VALID_NO_VALUE)
      || (val_ppe->valid[UPLL_IDX_YELLOW_ACTION_PPE] == UNC_VF_VALID_NO_VALUE)
      || (val_ppe->valid[UPLL_IDX_YELLOW_PRIORITY_PPE] == UNC_VF_VALID_NO_VALUE)
      || (val_ppe->valid[UPLL_IDX_YELLOW_DSCP_PPE] == UNC_VF_VALID_NO_VALUE)
      || (val_ppe->valid[UPLL_IDX_YELLOW_DROP_PPE] == UNC_VF_VALID_NO_VALUE)
      || (val_ppe->valid[UPLL_IDX_RED_ACTION_PPE] == UNC_VF_VALID_NO_VALUE)
      || (val_ppe->valid[UPLL_IDX_RED_PRIORITY_PPE] == UNC_VF_VALID_NO_VALUE)
      || (val_ppe->valid[UPLL_IDX_RED_DSCP_PPE] == UNC_VF_VALID_NO_VALUE)
      || (val_ppe->valid[UPLL_IDX_RED_DROP_PPE] == UNC_VF_VALID_NO_VALUE)) {
        check = true;
      }
    }
    if (check) {
      rt_code =
          ValidatePolicingProfileNameInVtnVbrVbrIfPolicingMapTbl(
            ikey, dmi, req);
      if (rt_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Update not allowed when policingprofile is in use");
        return rt_code;
      }
    }

    /** Validate rate, cir, cbs, pir, pbs */
    if (val_ppe->valid[UPLL_IDX_RATE_PPE] == UNC_VF_VALID) {
      rt_code = ValidateRate(val_ppe, operation);

      if (UPLL_RC_SUCCESS != rt_code) {
        UPLL_LOG_DEBUG(
            " PolicingProfileEntryname Syntax validation failed :Err Code - %d",
            rt_code);
        return rt_code;
      }
    } else if ((val_ppe->valid[UPLL_IDX_CIR_PPE]
        == UNC_VF_VALID)
        || (val_ppe->valid[UPLL_IDX_CBS_PPE] == UNC_VF_VALID)
        || (val_ppe->valid[UPLL_IDX_PIR_PPE] == UNC_VF_VALID)
        || (val_ppe->valid[UPLL_IDX_PBS_PPE] == UNC_VF_VALID)) {
      UPLL_LOG_DEBUG("Error Rate is not filled but its subsequent field "
                     "filled");
      return UPLL_RC_ERR_CFG_SYNTAX;
    } else if ((operation == UNC_OP_UPDATE)
        && (val_ppe->valid[UPLL_IDX_RATE_PPE] == UNC_VF_VALID_NO_VALUE)) {
      UPLL_LOG_DEBUG("Reset Rate");
      val_ppe->rate = 0;
    }

    /** Validate red_action, green_action, yellow_action */
    rt_code = ValidateColorAction(val_ppe, operation);

    if (UPLL_RC_SUCCESS != rt_code) {
      UPLL_LOG_DEBUG(" Color action syntax validation failed :Err Code - %d",
                    rt_code);
      return rt_code;
    }

    /** Validate red_action_priority, green_action_priority,
     yellow_action_priority */
    rt_code = ValidateColorPriority(val_ppe, operation);

    if (UPLL_RC_SUCCESS != rt_code) {
      UPLL_LOG_DEBUG(" Color priority syntax validation failed :Err Code - %d",
                    rt_code);
      return rt_code;
    }

    /** Validate red_precedence, green_precedence, yellow_precedence */
    rt_code = ValidateColorPrecedence(val_ppe, operation);

    if (UPLL_RC_SUCCESS != rt_code) {
      UPLL_LOG_DEBUG(" Color precedence syntax validation failed:Err Code- %d",
                    rt_code);
      return rt_code;
    }

    /** Validate dscp for red, green, yellow */
    rt_code = ValidateColorDscp(val_ppe, operation);
    if (UPLL_RC_SUCCESS != rt_code) {
      UPLL_LOG_DEBUG("DSCP syntax validation failed :Err Code - %d", rt_code);
      return rt_code;
    }
  }
  return UPLL_RC_SUCCESS;
}


upll_rc_t PolicingProfileEntryMoMgr::ValidateFlowList(
    ConfigKeyVal *ikey, DalDmlIntf *dmi,
    IpcReqRespHeader *req) {
  UPLL_FUNC_TRACE;
  upll_rc_t rt_code = UPLL_RC_ERR_GENERIC;

  val_policingprofile_entry_t *ppe_val = reinterpret_cast
    <val_policingprofile_entry_t *>(GetVal(ikey));
  if (ppe_val->valid[UPLL_IDX_FLOWLIST_PPE] == UNC_VF_VALID) {
    rt_code = ValidateKey(reinterpret_cast<char*>(ppe_val->flowlist),
                          kMinLenFlowListName,
                          kMaxLenFlowListName);

    if (UPLL_RC_SUCCESS != rt_code) {
      UPLL_LOG_DEBUG(" flowlist key validation failed %s, err code - %d",
                     ppe_val->flowlist, rt_code);
    }

    MoMgrImpl *mgr = reinterpret_cast<MoMgrImpl *>
        (const_cast<MoManager*>(GetMoManager(UNC_KT_FLOWLIST)));
    if (NULL == mgr) {
      return UPLL_RC_ERR_GENERIC;
    }
    ConfigKeyVal *fl_ckv = NULL;
    rt_code = mgr->GetChildConfigKey(fl_ckv, NULL);
    if (UPLL_RC_SUCCESS != rt_code) {
      UPLL_LOG_DEBUG("GetChildConfigKey fails %d", rt_code);
      return rt_code;
    }
    key_flowlist_t *fl_key = reinterpret_cast
      <key_flowlist_t *>(fl_ckv->get_key());
    uuu::upll_strncpy(fl_key->flowlist_name,
      ppe_val->flowlist,
      kMaxLenFlowListName+1);

    rt_code = mgr->UpdateConfigDB(fl_ckv, req->datatype,
                UNC_OP_READ, dmi);

    if (rt_code != UPLL_RC_ERR_INSTANCE_EXISTS) {
      UPLL_LOG_DEBUG("flowlist name does not exists in "
          "KT_FLOWLIST table");
      delete fl_ckv;
      return UPLL_RC_ERR_CFG_SEMANTIC;
    }
  } else if ((req->operation == UNC_OP_UPDATE ||
              req->operation == UNC_OP_CREATE)
        && (ppe_val->valid[UPLL_IDX_FLOWLIST_PPE] == UNC_VF_VALID_NO_VALUE)) {
      UPLL_LOG_DEBUG("Reset Flowlist name ");
      memset(ppe_val->flowlist, '\0', sizeof(ppe_val->flowlist));
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t PolicingProfileEntryMoMgr::
  ValidatePolicingProfileNameInVtnVbrVbrIfPolicingMapTbl(
    ConfigKeyVal *ikey, DalDmlIntf *dmi,
    IpcReqRespHeader *req) {
  UPLL_FUNC_TRACE;

  upll_rc_t result_code = UPLL_RC_ERR_GENERIC;
  val_policingmap_t *val_policingmap = reinterpret_cast<val_policingmap_t *>
    (ConfigKeyVal::Malloc(sizeof(val_policingmap_t)));
  key_policingprofile_entry_t *key_ppe = reinterpret_cast
    <key_policingprofile_entry_t *>(ikey->get_key());
  uuu::upll_strncpy(val_policingmap->policer_name,
    key_ppe->policingprofile_key.policingprofile_name,
    (kMaxLenPolicingProfileName + 1));
  val_policingmap->valid[UPLL_IDX_POLICERNAME_PM] = UNC_VF_VALID;

  ConfigVal *tempval = new ConfigVal(IpctSt::kIpcStValPolicingmap,
      val_policingmap);
  MoMgrImpl *mgr = reinterpret_cast<MoMgrImpl *>
      (const_cast<MoManager*>(GetMoManager(UNC_KT_VTN_POLICINGMAP)));
  if (!mgr) {
    delete tempval;
    return UPLL_RC_ERR_GENERIC;
  }
  ConfigKeyVal *vtn_ckv = NULL;
  result_code = mgr->GetChildConfigKey(vtn_ckv, NULL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetChildConfigKey Failed %d", result_code);
    delete tempval;
    return result_code;
  }
  vtn_ckv->AppendCfgVal(tempval);

  DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutNone };
  result_code = mgr->ReadConfigDB(vtn_ckv, req->datatype, UNC_OP_READ, dbop,
      dmi, MAINTBL);
  if (result_code == UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Policing profile name used in VTN_POLICINGMAP table");
    return UPLL_RC_ERR_CFG_SEMANTIC;
  } else if (UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
    UPLL_LOG_DEBUG("Policing profile name check in pm failed %d",
          result_code);
    delete vtn_ckv;
    return result_code;
  }
  delete vtn_ckv;

  val_policingmap_t *val_policingmap1 =
    reinterpret_cast<val_policingmap_t *>
    (ConfigKeyVal::Malloc(sizeof(val_policingmap_t)));
  uuu::upll_strncpy(val_policingmap1->policer_name,
    key_ppe->policingprofile_key.policingprofile_name,
    (kMaxLenPolicingProfileName + 1));
  val_policingmap1->valid[UPLL_IDX_POLICERNAME_PM ] = UNC_VF_VALID;
  ConfigVal *tempval1 = new ConfigVal(IpctSt::kIpcStValPolicingmap,
      val_policingmap1);
  mgr =
    reinterpret_cast<MoMgrImpl *>(const_cast<MoManager*>(GetMoManager(
            UNC_KT_VBR_POLICINGMAP)));
  if (!mgr) {
    return UPLL_RC_ERR_GENERIC;
  }
  ConfigKeyVal *vbr_ckv = NULL;
  result_code = mgr->GetChildConfigKey(vbr_ckv, NULL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetChildConfigKey Failed %d", result_code);
    return result_code;
  }
  vbr_ckv->AppendCfgVal(tempval1);

  result_code = mgr->ReadConfigDB(vbr_ckv, req->datatype, UNC_OP_READ, dbop,
      dmi, MAINTBL);
  if (result_code == UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Policing profile name used in VTN_POLICINGMAP table");
    return UPLL_RC_ERR_CFG_SEMANTIC;
  } else if (UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
    UPLL_LOG_DEBUG("Policing profile name check in pm failed %d",
          result_code);
    return result_code;
  }
  delete vbr_ckv;

  val_policingmap_t *val_policingmap2 =
    reinterpret_cast<val_policingmap_t *>
    (ConfigKeyVal::Malloc(sizeof(val_policingmap_t)));
  uuu::upll_strncpy(val_policingmap2->policer_name,
    key_ppe->policingprofile_key.policingprofile_name,
    (kMaxLenPolicingProfileName + 1));
  val_policingmap2->valid[UPLL_IDX_POLICERNAME_PM ] = UNC_VF_VALID;
  ConfigVal *tempval2 = new ConfigVal(IpctSt::kIpcStValPolicingmap,
      val_policingmap2);
  mgr =
    reinterpret_cast<MoMgrImpl *>(const_cast<MoManager*>(GetMoManager(
            UNC_KT_VBRIF_POLICINGMAP)));
  if (!mgr) {
    return UPLL_RC_ERR_GENERIC;
  }
  ConfigKeyVal *vbrif_ckv = NULL;
  result_code = mgr->GetChildConfigKey(vbrif_ckv, NULL);
  if (UPLL_RC_SUCCESS != result_code) {
    delete tempval;
    UPLL_LOG_DEBUG("GetChildConfigKey Failed %d", result_code);
    return result_code;
  }
  vbrif_ckv->AppendCfgVal(tempval2);

  result_code = mgr->ReadConfigDB(vbrif_ckv, req->datatype, UNC_OP_READ, dbop,
      dmi, MAINTBL);
  if (result_code == UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Policing profile name used in VTN_POLICINGMAP table");
    return UPLL_RC_ERR_CFG_SEMANTIC;
  } else if (UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
    UPLL_LOG_DEBUG("Policing profile name check in pm failed %d",
          result_code);
    return result_code;
  }
  delete vbrif_ckv;

  return UPLL_RC_SUCCESS;
}

upll_rc_t PolicingProfileEntryMoMgr::ValidateValidElements(
    const char *policingprofile_name, DalDmlIntf *dmi,
    upll_keytype_datatype_t dt_type) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *ppe_ckv = NULL;
  result_code = GetChildConfigKey(ppe_ckv, NULL);
  if (UPLL_RC_SUCCESS != result_code) {
    return result_code;
  }
  key_policingprofile_entry_t *ppe_key = reinterpret_cast
      <key_policingprofile_entry_t *>(ppe_ckv->get_key());
  uuu::upll_strncpy(ppe_key->policingprofile_key.policingprofile_name,
    policingprofile_name, kMaxLenPolicingProfileName);

  DbSubOp dbop = { kOpReadMultiple, kOpMatchNone, kOpInOutNone };
  result_code = ReadConfigDB(ppe_ckv, dt_type, UNC_OP_READ, dbop, dmi, MAINTBL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("No entry in policingprofile entry maintbl");
    delete ppe_ckv;
    return result_code;
  }
  while (NULL != ppe_ckv) {
    val_policingprofile_entry_t *val_ppe = reinterpret_cast
      <val_policingprofile_entry_t *>(GetVal(ppe_ckv));
    if (UNC_VF_VALID != val_ppe->valid[UPLL_IDX_FLOWLIST_PPE ]) {
      UPLL_LOG_DEBUG("Attribute flowlist is not valid");
      return UPLL_RC_ERR_CFG_SEMANTIC;
    }
    if (UNC_VF_VALID != val_ppe->valid[UPLL_IDX_RATE_PPE]) {
      UPLL_LOG_DEBUG("Attribute rate is not valid");
      return UPLL_RC_ERR_CFG_SEMANTIC;
    }
    if (UNC_VF_VALID != val_ppe->valid[UPLL_IDX_CIR_PPE]) {
      UPLL_LOG_DEBUG("Attribute cir is not valid");
      return UPLL_RC_ERR_CFG_SEMANTIC;
    }
    if (UNC_VF_VALID != val_ppe->valid[UPLL_IDX_PIR_PPE]) {
      UPLL_LOG_DEBUG("Attribute pir is not valid");
      return UPLL_RC_ERR_CFG_SEMANTIC;
    }
    if (UNC_VF_VALID == val_ppe->valid[UPLL_IDX_GREEN_ACTION_PPE]) {
      if (UPLL_POLICINGPROFILE_ACT_PASS != val_ppe->green_action &&
          UPLL_POLICINGPROFILE_ACT_DROP != val_ppe->green_action) {
        if (UNC_VF_VALID == val_ppe->valid[UPLL_IDX_GREEN_PRIORITY_PPE] ||
            UNC_VF_VALID == val_ppe->valid[UPLL_IDX_GREEN_DSCP_PPE] ||
            UNC_VF_VALID == val_ppe->valid[UPLL_IDX_GREEN_DROP_PPE]) {
          UPLL_LOG_DEBUG("Atleast one is valid");
        } else {
          UPLL_LOG_DEBUG("Attribute UPLL_IDX_GREEN_ACTION_PPE failed");
          return UPLL_RC_ERR_CFG_SEMANTIC;
        }
      }
    } else {
      return UPLL_RC_ERR_CFG_SEMANTIC;
    }
    if (UNC_VF_VALID == val_ppe->valid[UPLL_IDX_RED_ACTION_PPE]) {
      if (UPLL_POLICINGPROFILE_ACT_PASS != val_ppe->red_action &&
          UPLL_POLICINGPROFILE_ACT_DROP != val_ppe->red_action) {
        if (UNC_VF_VALID == val_ppe->valid[UPLL_IDX_RED_PRIORITY_PPE] ||
            UNC_VF_VALID == val_ppe->valid[UPLL_IDX_RED_DSCP_PPE] ||
            UNC_VF_VALID == val_ppe->valid[UPLL_IDX_RED_DROP_PPE]) {
          UPLL_LOG_DEBUG("Atleast one is valid");
        } else {
          UPLL_LOG_DEBUG("Attribute UPLL_IDX_RED_ACTION_PPE failed");
          return UPLL_RC_ERR_CFG_SEMANTIC;
        }
      }
    } else {
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
    if (UNC_VF_VALID == val_ppe->valid[UPLL_IDX_YELLOW_ACTION_PPE]) {
      if (UPLL_POLICINGPROFILE_ACT_PASS != val_ppe->yellow_action &&
          UPLL_POLICINGPROFILE_ACT_DROP != val_ppe->yellow_action) {
        if (UNC_VF_VALID == val_ppe->valid[UPLL_IDX_YELLOW_PRIORITY_PPE] ||
            UNC_VF_VALID == val_ppe->valid[UPLL_IDX_YELLOW_DSCP_PPE] ||
            UNC_VF_VALID == val_ppe->valid[UPLL_IDX_YELLOW_DROP_PPE]) {
          UPLL_LOG_DEBUG("Atleast one is valid ");

        } else {
          UPLL_LOG_DEBUG("Attribute UPLL_IDX_YELLOW_ACTION_PPE failed");
          return UPLL_RC_ERR_CFG_SEMANTIC;
        }
      }
    } else {
      return UPLL_RC_ERR_CFG_SEMANTIC;
    }
    ppe_ckv = ppe_ckv->get_next_cfg_key_val();
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t PolicingProfileEntryMoMgr::IsFlowListMatched(
    const char *flowlist_name, upll_keytype_datatype_t dt_type,
    DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *ppe_ckv = NULL;
  result_code = GetChildConfigKey(ppe_ckv, NULL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetChildConfigKey failed");
    return result_code;
  }
  val_policingprofile_entry_t *val_ppe = reinterpret_cast
    <val_policingprofile_entry_t *>(ConfigKeyVal::
     Malloc(sizeof(val_policingprofile_entry_t)));
  val_ppe->valid[UPLL_IDX_FLOWLIST_PPE ] = UNC_VF_VALID;
  uuu::upll_strncpy(val_ppe->flowlist, flowlist_name,
    kMaxLenFlowListName + 1);
  ppe_ckv->AppendCfgVal(IpctSt::kIpcStValPolicingprofileEntry, val_ppe);
  DbSubOp dbop = {kOpReadMultiple, kOpMatchNone, kOpInOutNone};
  result_code = ReadConfigDB(ppe_ckv, dt_type, UNC_OP_READ, dbop, dmi,
                  MAINTBL);
  if (UPLL_RC_SUCCESS != result_code) {
    if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
      UPLL_LOG_DEBUG("Flowlist is not referred by any policingprofile");
      return UPLL_RC_SUCCESS;
    }
    UPLL_LOG_DEBUG("ReadConfigDB failed %d", result_code);
    return result_code;
  }
  ConfigKeyVal *temp_ppe_ckv = ppe_ckv;
  while (NULL != temp_ppe_ckv) {
    ConfigKeyVal *ctrlr_ppe_ckv = NULL;
    result_code = GetChildConfigKey(ctrlr_ppe_ckv, temp_ppe_ckv);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("GetChildConfigKey failed");
      return result_code;
    }
    DbSubOp dbop1 = {kOpReadMultiple, kOpMatchNone, kOpInOutNone};
    result_code = ReadConfigDB(ctrlr_ppe_ckv, dt_type, UNC_OP_READ, dbop1, dmi,
                    CTRLRTBL);
    if (UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
      if (UPLL_RC_SUCCESS == result_code) {
        UPLL_LOG_DEBUG("Flowlist is referred by policingprofile which"
                       "is referred by a policingmap");
        return UPLL_RC_ERR_INSTANCE_EXISTS;
      }
      UPLL_LOG_DEBUG("ReadConfigDB failed %d", result_code);
      return result_code;
    }
    temp_ppe_ckv = temp_ppe_ckv->get_next_cfg_key_val();
  }
  delete ppe_ckv;
  ppe_ckv = NULL;
  return UPLL_RC_SUCCESS;
}
}  // namespace kt_momgr
}  // namespace upll
}  // namespace unc
