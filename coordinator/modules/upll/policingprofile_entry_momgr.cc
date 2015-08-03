/*
 * Copyright (c) 2012-2015 NEC Corporation
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
#include "uncxx/upll_log.hh"
#include "upll_validation.hh"
#include "unc/upll_ipc_enum.h"
#include "unc/upll_errno.h"
#include "config_mgr.hh"
#include "upll_db_query.hh"

#define GET_VALID_MAINCTRL(tbl, l_val_ctrl_ff, l_val_ff, en) \
  (tbl == MAINTBL) ? &(l_val_ff->valid[en]) : &(l_val_ctrl_ff->valid[en])

namespace unc {
namespace upll {
namespace kt_momgr {

#define PP_RENAME_FLAG 0x01
#define FL_RENAME_FLAG 0x02
#define FLT_RENAME_FLAG 0X04

#define FLOWLIST_RENAME 0x02
#define NO_FLOWLIST_RENAME 0xFD

#define NUM_PP_KEY_MAIN_COL 4
#define NUM_PP_KEY_CTRLR_COL 4

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
      key_user_data_t, flags),
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
  { uudst::policingprofile_entry::kDbiFlags, CK_VAL,
    offsetof(key_user_data_t, flags), uud::kDalUint8, 1 } };

BindInfo PolicingProfileEntryMoMgr::rename_policingprofile_entry_ctrl_tbl[] = {
  { uudst::policingprofile_entry_ctrlr::kDbiPolicingProfileName, CFG_MATCH_KEY,
    offsetof(key_policingprofile_entry_t,
        policingprofile_key.policingprofile_name),
    uud::kDalChar, kMaxLenPolicingProfileName + 1 },
  { uudst::policingprofile_entry_ctrlr::kDbiSequenceNum, CFG_MATCH_KEY,
    offsetof(key_policingprofile_entry_t, sequence_num),
  uud::kDalUint8, 1 },
  { uudst::policingprofile_entry_ctrlr::kDbiPolicingProfileName, CFG_INPUT_KEY,
    offsetof(key_rename_vnode_info_t, new_policingprofile_name),
    uud::kDalChar, kMaxLenPolicingProfileName + 1 },
  { uudst::policingprofile_entry_ctrlr::kDbiFlags, CK_VAL, offsetof(
    key_user_data_t, flags),
     uud::kDalUint8, 1 } };

bool PolicingProfileEntryMoMgr::GetRenameKeyBindInfo(unc_key_type_t key_type,
    BindInfo *&binfo,
    int &nattr,
  MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  if (MAINTBL == tbl) {
    nattr = NUM_PP_KEY_MAIN_COL;
    binfo = rename_policingprofile_entry_main_tbl;
  } else if (CTRLRTBL == tbl) {
    nattr = NUM_PP_KEY_CTRLR_COL;
    binfo = rename_policingprofile_entry_ctrl_tbl;
  } else {
    return PFC_FALSE;
  }
  return PFC_TRUE;
}

PolicingProfileEntryMoMgr::PolicingProfileEntryMoMgr() :  MoMgrImpl() {
  UPLL_FUNC_TRACE;
  // Rename operation is not support for this KT
  ntable = MAX_MOMGR_TBLS;
  table = new Table *[ntable]();

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

  table[CONVERTTBL] = NULL;
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
//    if (key_ppe) free(key_ppe);
    return UPLL_RC_ERR_GENERIC;
  }
  if (okey) {
    UPLL_LOG_DEBUG("Okey not NULL");
    if (okey->get_key_type() != UNC_KT_POLICING_PROFILE_ENTRY)
      return UPLL_RC_ERR_GENERIC;
  }
  if ((okey) && (okey->get_key())) {
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

  if ((okey) && !(okey->get_key())) {
    UPLL_LOG_TRACE("okey not NULL profile name updated");
    okey->SetKey(IpctSt::kIpcStKeyPolicingprofileEntry, key_ppe);
  }

  if (!okey) {
    okey = new ConfigKeyVal(UNC_KT_POLICING_PROFILE_ENTRY,
                            IpctSt::kIpcStKeyPolicingprofileEntry,
                            key_ppe, NULL);
  }

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
  UPLL_LOG_TRACE("%s GetRenamedUncKey pp_entry start",
                  ctrlr_key->ToStrAll().c_str());
  if ((NULL == ctrlr_key) || (NULL == ctrlr_id) || (NULL == dmi)) {
    UPLL_LOG_DEBUG(" PolicingProfileEntryMoMgr::GetRenamedUncKey Failed.");
    return result_code;
  }
  uint8_t rename = 0;
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
  if (!rename_policingprofile) {
    UPLL_LOG_DEBUG("rename_policingprofile NULL");
    return result_code;
  }
  uuu::upll_strncpy(
      rename_policingprofile->policingprofile_newname,
      ctrlr_policingprofile_entry_key->policingprofile_key.policingprofile_name,
      (kMaxLenPolicingProfileName + 1));
  rename_policingprofile->valid[UPLL_IDX_RENAME_PROFILE_RPP] = UNC_VF_VALID;
  MoMgrImpl *mgr =
    reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>(GetMoManager(
            UNC_KT_POLICING_PROFILE)));
  if (!mgr) {
    UPLL_LOG_TRACE("mgr failed");
    if (rename_policingprofile) free(rename_policingprofile);
    return UPLL_RC_ERR_GENERIC;
  }
  result_code = mgr->GetChildConfigKey(unc_key, NULL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetChildConfigKey NULL");
    free(rename_policingprofile);
    return result_code;
  }
  if (!unc_key) {
    UPLL_LOG_DEBUG("unc_key NULL");
    free(rename_policingprofile);
    return UPLL_RC_ERR_GENERIC;
  }
  unc_key->AppendCfgVal(IpctSt::kIpcStValRenamePolicingprofile,
      rename_policingprofile);
  if (ctrlr_id)
    SET_USER_DATA_CTRLR(unc_key, ctrlr_id);

  UPLL_LOG_DEBUG("ctrlr_id (%s)", ctrlr_id);
  result_code = mgr->ReadConfigDB(unc_key, dt_type, UNC_OP_READ, dbop, dmi,
      RENAMETBL);
  if (UPLL_RC_SUCCESS == result_code) {
    key_policingprofile_entry_t *policingprofile_entry =
      reinterpret_cast<key_policingprofile_entry_t *>(unc_key->get_key());
    uuu::upll_strncpy(
      ctrlr_policingprofile_entry_key->policingprofile_key.policingprofile_name,
      policingprofile_entry->policingprofile_key.policingprofile_name,
      (kMaxLenPolicingProfileName + 1));
      rename |= PP_RENAME_FLAG;
      SET_USER_DATA(ctrlr_key, unc_key);
      SET_USER_DATA_FLAGS(ctrlr_key, rename);

  } else if (UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
    UPLL_LOG_DEBUG("ReadConfigDB failed %d", result_code);
    DELETE_IF_NOT_NULL(unc_key);
    mgr = NULL;
    return result_code;
  }
  mgr = NULL;
  DELETE_IF_NOT_NULL(unc_key);

  val_policingprofile_entry_t *val_ppe =
    reinterpret_cast<val_policingprofile_entry_t *>(GetVal(ctrlr_key));
  if (!val_ppe) {
    UPLL_LOG_DEBUG("val_ppe NULL");
    return UPLL_RC_SUCCESS;
  }
  if (UNC_VF_VALID != val_ppe->valid[UPLL_IDX_FLOWLIST_PPE ]) {
    UPLL_LOG_DEBUG("flowlist invalid");
    return UPLL_RC_SUCCESS;
  }
  ConfigKeyVal *fl_ckv = NULL;
  val_rename_flowlist_t *rename_flowlist =
    reinterpret_cast<val_rename_flowlist_t *>(ConfigKeyVal::Malloc(
          sizeof(val_rename_flowlist_t)));

  if (!rename_flowlist) {
    UPLL_LOG_DEBUG("rename_flowlist NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  uuu::upll_strncpy(rename_flowlist->flowlist_newname,
                    val_ppe->flowlist,
                    (kMaxLenFlowListName + 1));
  rename_flowlist->valid[UPLL_IDX_RENAME_FLOWLIST_RFL] = UNC_VF_VALID;
  MoMgrImpl *mgr_fl =
    reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>(GetMoManager(
            UNC_KT_FLOWLIST)));
  if (!mgr_fl) {
    UPLL_LOG_TRACE("mgr failed");
    if (rename_flowlist) free(rename_flowlist);
    return UPLL_RC_ERR_GENERIC;
  }

  result_code = mgr_fl->GetChildConfigKey(fl_ckv, NULL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetChildConfigKey NULL");
    free(rename_flowlist);
    return result_code;
  }
  if (!fl_ckv) {
    UPLL_LOG_DEBUG("fl_ckv NULL");
    free(rename_flowlist);
    return UPLL_RC_ERR_GENERIC;
  }
  fl_ckv->AppendCfgVal(IpctSt::kIpcStValRenameFlowlist,
      rename_flowlist);

  UPLL_LOG_DEBUG("ctrlr_id ppe (%s)", ctrlr_id);

  if (ctrlr_id)
    SET_USER_DATA_CTRLR(fl_ckv, ctrlr_id);

  result_code = mgr_fl->ReadConfigDB(fl_ckv, dt_type, UNC_OP_READ, dbop, dmi,
      RENAMETBL);
  if (UPLL_RC_SUCCESS == result_code) {
    key_flowlist_t *key_flowlist =
      reinterpret_cast<key_flowlist_t *>(fl_ckv->get_key());
    uuu::upll_strncpy(val_ppe->flowlist,
                      key_flowlist->flowlist_name,
                      (kMaxLenFlowListName + 1));
    rename |= FL_RENAME_FLAG;
    SET_USER_DATA(ctrlr_key, unc_key);
    SET_USER_DATA_FLAGS(ctrlr_key, rename);
  } else if (UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
    UPLL_LOG_DEBUG("ReadConfigDB failed %d", result_code);
    mgr_fl = NULL;
    DELETE_IF_NOT_NULL(fl_ckv);
    return result_code;
  }
  UPLL_LOG_TRACE("%s GetRenamedUncKey pp_entry end",
                  ctrlr_key->ToStrAll().c_str());
  DELETE_IF_NOT_NULL(fl_ckv);
  mgr_fl = NULL;
  return UPLL_RC_SUCCESS;
}


upll_rc_t PolicingProfileEntryMoMgr::ChkProfileNameInRenameTbl(
    ConfigKeyVal *ctrlr_key, upll_keytype_datatype_t dt_type, DalDmlIntf *dmi,
    const char *ctrlr_id) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_ERR_GENERIC;
  ConfigKeyVal *unc_key = NULL;
  UPLL_LOG_TRACE("%s GetRenamedUncKey pp_entry start",
                  ctrlr_key->ToStrAll().c_str());
  if ((NULL == ctrlr_key) || (NULL == ctrlr_id) || (NULL == dmi)) {
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
  if (!rename_policingprofile) {
    UPLL_LOG_DEBUG("rename_policingprofile NULL");
    return result_code;
  }
  uuu::upll_strncpy(
      rename_policingprofile->policingprofile_newname,
      ctrlr_policingprofile_entry_key->policingprofile_key.policingprofile_name,
      (kMaxLenPolicingProfileName + 1));
  rename_policingprofile->valid[UPLL_IDX_RENAME_PROFILE_RPP] = UNC_VF_VALID;
  MoMgrImpl *mgr =
    reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>(GetMoManager(
            UNC_KT_POLICING_PROFILE)));
  if (!mgr) {
    UPLL_LOG_TRACE("mgr failed");
    if (rename_policingprofile) free(rename_policingprofile);
    return UPLL_RC_ERR_GENERIC;
  }
  result_code = mgr->GetChildConfigKey(unc_key, NULL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetChildConfigKey NULL");
    free(rename_policingprofile);
    return result_code;
  }
  if (!unc_key) {
    UPLL_LOG_DEBUG("unc_key NULL");
    free(rename_policingprofile);
    return UPLL_RC_ERR_GENERIC;
  }
  unc_key->AppendCfgVal(IpctSt::kIpcStValRenamePolicingprofile,
      rename_policingprofile);
  if (ctrlr_id)
    SET_USER_DATA_CTRLR(unc_key, ctrlr_id);

  UPLL_LOG_DEBUG("ctrlr_id (%s)", ctrlr_id);
  result_code = mgr->ReadConfigDB(unc_key, dt_type, UNC_OP_READ, dbop, dmi,
      RENAMETBL);
  DELETE_IF_NOT_NULL(unc_key);
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
        if (NULL == tmp1) {
          UPLL_LOG_DEBUG("Null Pointer:");
          FREE_IF_NOT_NULL(policingprofile_entry_val);
          return UPLL_RC_ERR_GENERIC;
        }

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
        if (NULL == tmp1) {
          UPLL_LOG_DEBUG("Null Pointer:");
         FREE_IF_NOT_NULL(entry_val);
         return UPLL_RC_ERR_GENERIC;
        }
      //  free(entry_val);  //  TODO(check whether req)
      }
    } else {
      return UPLL_RC_ERR_GENERIC;
    }
    if (tmp1 != NULL) {
    tmp1->set_user_data(tmp->get_user_data());
    }
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
    ConfigKeyVal *&ckv_running,
    DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  val_policingprofile_entry_ctrl_t *val;
/*  val = (ckv_running != NULL) ? reinterpret_cast
      <val_policingprofile_entry_ctrl_t *>(GetVal(
          ckv_running)) :  NULL;*/
  val = reinterpret_cast
      <val_policingprofile_entry_ctrl_t *>(GetVal(ckv_running));
  if (NULL == val) {
    UPLL_LOG_DEBUG("Val struct is NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  if (uuc::kUpllUcpCreate == phase )
    val->cs_row_status = cs_status;
  if ((uuc::kUpllUcpUpdate == phase) &&
           (val->cs_row_status == UNC_CS_INVALID ||
            val->cs_row_status == UNC_CS_NOT_APPLIED))
    val->cs_row_status = cs_status;
  for (unsigned int loop = 0; loop < sizeof(val->valid)/sizeof(val->valid[0]);
      ++loop) {
    if ((cs_status == UNC_CS_INVALID && UNC_VF_VALID == val->valid[loop]) ||
         cs_status == UNC_CS_APPLIED)
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
    const char *ctrlr_id, ConfigKeyVal *ikey,
    DalDmlIntf *dmi, upll_import_type import_type) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_ERR_GENERIC;
  unc_keytype_operation_t op[] = { UNC_OP_DELETE,
    UNC_OP_CREATE,
    UNC_OP_UPDATE };
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

upll_rc_t PolicingProfileEntryMoMgr::IsReferenced(IpcReqRespHeader *req,
    ConfigKeyVal *ikey, DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *okey = NULL;
  result_code = UpdateConfigDB(ikey,
                  req->datatype, UNC_OP_READ, dmi, CTRLRTBL);
  if (UPLL_RC_ERR_INSTANCE_EXISTS == result_code) {
    return UPLL_RC_ERR_CFG_SEMANTIC;
  } else if (UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
    return result_code;
  }
  // Change to GetParentKey
  result_code = GetPolicingprofileKeyVal(okey, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Memory is not allocated for okey %d", result_code);
    DELETE_IF_NOT_NULL(okey);
    return result_code;
  }
  MoMgrImpl *mgr =
    reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>(GetMoManager(
            UNC_KT_POLICING_PROFILE)));
  if (!mgr) {
    UPLL_LOG_DEBUG("Invalid mgr param");
    DELETE_IF_NOT_NULL(okey);  // RESOURCE LEAK
    return UPLL_RC_ERR_GENERIC;
  }

  result_code = mgr->IsReferenced(req, okey, dmi);

  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("IsReferenced id Faliled %d", result_code);
    DELETE_IF_NOT_NULL(okey);
    return result_code;
  }
  DELETE_IF_NOT_NULL(okey);
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
    ConfigKeyVal *ikey, upll_keytype_datatype_t dt_type, DalDmlIntf *dmi,
    controller_domain *ctrlr_dom) {
  UPLL_FUNC_TRACE;
  if (!ikey) {
    return UPLL_RC_ERR_GENERIC;
  }
  upll_rc_t result_code = UPLL_RC_ERR_GENERIC;
  ConfigKeyVal *okey = NULL;

  DbSubOp dbop = { kOpReadSingle, kOpMatchCtrlr, kOpInOutFlag };

  /* PolicingProfile_name is  renamed */
  MoMgrImpl *ppmgr = static_cast<MoMgrImpl*>
    ((const_cast<MoManager*>(GetMoManager(UNC_KT_POLICING_PROFILE))));
  if (NULL == ppmgr) {
    UPLL_LOG_DEBUG("Policingprofile instance is NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  result_code = ppmgr->GetChildConfigKey(okey, NULL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetChildConfigKey fail (%d)", result_code);
    return UPLL_RC_ERR_GENERIC;
  }

  if (NULL != ctrlr_dom) {
    SET_USER_DATA_CTRLR(okey, ctrlr_dom->ctrlr);
  } else {
    UPLL_LOG_DEBUG("ctrlr null");
    DELETE_IF_NOT_NULL(okey);
    return UPLL_RC_ERR_GENERIC;
  }

  UPLL_LOG_DEBUG("ctrlr : %s; domain : %s", ctrlr_dom->ctrlr,
      ctrlr_dom->domain);

  uuu::upll_strncpy(
      reinterpret_cast<key_policingprofile_t *>
      (okey->get_key())->policingprofile_name,
      reinterpret_cast <key_policingprofile_entry *>
      (ikey->get_key())->policingprofile_key.policingprofile_name,
      (kMaxLenPolicingProfileName + 1));

  UPLL_LOG_DEBUG("profile name (%s) (%s)",
      reinterpret_cast<key_policingprofile_t *>
      (okey->get_key())->policingprofile_name,
      reinterpret_cast <key_policingprofile_entry *>
      (ikey->get_key())->policingprofile_key.policingprofile_name);

  /* ctrlr_name */
  result_code = ppmgr->ReadConfigDB(okey, dt_type, UNC_OP_READ, dbop, dmi,
      RENAMETBL);  // COVERITY CHECKED RETURN
  if ((result_code != UPLL_RC_SUCCESS) &&
      (UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code)) {
    DELETE_IF_NOT_NULL(okey);
    UPLL_LOG_DEBUG("ReadConfigDB failed (%d)", result_code);
    return result_code;
  }
  /* Null check missing */
  if (UPLL_RC_SUCCESS == result_code) {
    val_rename_policingprofile_t *rename_val =
      reinterpret_cast<val_rename_policingprofile_t *>(GetVal(okey));
    if (!rename_val) {
      UPLL_LOG_DEBUG("rename_val null");
      DELETE_IF_NOT_NULL(okey);
      return UPLL_RC_ERR_GENERIC;
    }

    uuu::upll_strncpy(
        reinterpret_cast<key_policingprofile_entry_t *>
        (ikey->get_key())->policingprofile_key.policingprofile_name,
        rename_val->policingprofile_newname,
        (kMaxLenPolicingProfileName + 1));
  }
  DELETE_IF_NOT_NULL(okey);
  /* rename flowlist */
  UPLL_LOG_DEBUG("flowlist name renamed");
  // Since during delete there wont be val structure
  val_policingprofile_entry_t *val_ppe =
      reinterpret_cast<val_policingprofile_entry_t*>(GetVal(ikey));
  if (NULL == val_ppe) {
    return UPLL_RC_SUCCESS;
  }
  if (strlen(reinterpret_cast<char *>
        (val_ppe->flowlist))) {
    MoMgrImpl *mgrflist = static_cast<MoMgrImpl*>
      ((const_cast<MoManager*> (GetMoManager(UNC_KT_FLOWLIST))));
    result_code = mgrflist->GetChildConfigKey(okey, NULL);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("GetChildConfigKey fail");
      return result_code;
    }
    SET_USER_DATA_CTRLR(okey, ctrlr_dom->ctrlr);

    UPLL_LOG_DEBUG("ctrlr : %s;", ctrlr_dom->ctrlr);

    uuu::upll_strncpy(
        reinterpret_cast<key_flowlist_t *>(okey->get_key())->flowlist_name,
        reinterpret_cast<val_policingprofile_entry_t*>
        (ikey->get_cfg_val()->get_val())->flowlist,
        kMaxLenFlowListName + 1);

    UPLL_LOG_DEBUG("flowlist name (%s) (%s)",
        reinterpret_cast<key_flowlist_t *>(okey->get_key())->flowlist_name,
        reinterpret_cast<val_policingprofile_entry_t *>
        (ikey->get_cfg_val()->get_val())->flowlist);

    DbSubOp dbop = { kOpReadSingle, kOpMatchCtrlr, kOpInOutFlag };
    /* ctrlr_name */
    result_code =  mgrflist->ReadConfigDB(okey, dt_type,
        UNC_OP_READ, dbop, dmi, RENAMETBL);
    if ((result_code != UPLL_RC_SUCCESS ) &&
        (UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code)) {
      UPLL_LOG_DEBUG("ReadConfigDB fail");
      DELETE_IF_NOT_NULL(okey);
      return UPLL_RC_ERR_GENERIC;
    }
    if (UPLL_RC_SUCCESS == result_code) {
      val_rename_flowlist_t *rename_val =
        reinterpret_cast <val_rename_flowlist_t*>(GetVal(okey));

      if (!rename_val) {
        UPLL_LOG_DEBUG("flowlist is not valid");
        DELETE_IF_NOT_NULL(okey);
        return UPLL_RC_ERR_GENERIC;
      }
      uuu::upll_strncpy(
          reinterpret_cast<val_policingprofile_entry_t *>
          (ikey->get_cfg_val()->get_val())->flowlist,
          rename_val->flowlist_newname,
          (kMaxLenFlowListName + 1));
    }
    DELETE_IF_NOT_NULL(okey);
  }

  UPLL_LOG_DEBUG("%s GetRenamedCtrl pp_entry end",
      (ikey->ToStrAll()).c_str());
  UPLL_LOG_DEBUG("GetRenamedControllerKey is successful ");
  return UPLL_RC_SUCCESS;
}


upll_rc_t PolicingProfileEntryMoMgr::GetControllerDomainSpan(
    ConfigKeyVal *ikey, upll_keytype_datatype_t dt_type, DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code;
  DbSubOp dbop = { kOpReadMultiple, kOpMatchNone, kOpInOutCtrlr };

  result_code = ReadConfigDB(ikey, dt_type, UNC_OP_READ, dbop, dmi, CTRLRTBL);
  UPLL_LOG_DEBUG("GetControllerDomainSpan successful:- %d", result_code);
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
    DalDmlIntf *dmi, TcConfigMode config_mode, std::string vtn_name) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  DalResultCode db_result;
  unc_keytype_operation_t op[] = {  UNC_OP_DELETE, UNC_OP_CREATE,
                                   UNC_OP_UPDATE };
  int nop = sizeof(op) / sizeof(op[0]);
  ConfigKeyVal *ppe_key = NULL, *req = NULL, *nreq = NULL,
               *pp_ck_run = NULL;
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
            UPLL_LOG_INFO(
                "PolicingProfileEntryMoMgr::GetRenamedUncKey is successful -%d",
                result_code);
            return result_code;
          }
        }
      }
    }
  }

  if (config_mode != TC_CONFIG_VTN) {
    for (int i = 0; i < nop; i++) {
      cfg1_cursor = NULL;
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
            UPLL_LOG_INFO("Updating Main table Error %d", result_code);
            DELETE_IF_NOT_NULL(req);
            dmi->CloseCursor(cfg1_cursor, true);
            return result_code;
          }
        }
        if (cfg1_cursor) {
          dmi->CloseCursor(cfg1_cursor, true);
          cfg1_cursor = NULL;
        }
        DELETE_IF_NOT_NULL(req);
      }
      UPLL_LOG_DEBUG("Updating main table complete with op %d", op[i]);
    }   // for loop
  }  // if loop

    for (int i = 0; i < nop; i++) {
      cfg1_cursor = NULL;
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

      ConfigKeyVal *ppe_ctrlr_key = NULL;
      while (result_code == UPLL_RC_SUCCESS) {
        db_result = dmi->GetNextRecord(cfg1_cursor);
        result_code = DalToUpllResCode(db_result);
        if (result_code != UPLL_RC_SUCCESS)
         break;

        if ((op[i] == UNC_OP_UPDATE) && (config_mode == TC_CONFIG_GLOBAL ||
             config_mode == TC_CONFIG_VIRTUAL)) {
          DbSubOp dbop = { kOpReadMultiple, kOpMatchNone,
            kOpInOutCtrlr |kOpInOutCs };
          result_code = GetChildConfigKey(ppe_ctrlr_key, req);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("GetChildConfigKey is failed resultcode=%d",
                result_code);
            DELETE_IF_NOT_NULL(req);
            DELETE_IF_NOT_NULL(nreq);
            dmi->CloseCursor(cfg1_cursor, true);
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
            DELETE_IF_NOT_NULL(ppe_ctrlr_key);
            if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
              result_code = UpdateMainTbl(req, op[i], UPLL_RC_SUCCESS,
                                        nreq, dmi);
              if (result_code != UPLL_RC_SUCCESS) {
                UPLL_LOG_INFO("Error updating main table %d", result_code);
                DELETE_IF_NOT_NULL(req);
                DELETE_IF_NOT_NULL(nreq);
                DELETE_IF_NOT_NULL(ppe_ctrlr_key);
                dmi->CloseCursor(cfg1_cursor, true);
                return result_code;
              } else {
                DELETE_IF_NOT_NULL(ppe_ctrlr_key);
                continue;
              }
            } else  {
              UPLL_LOG_INFO("DB err while reading records from ctrlrtbl,"
                            "err %d", result_code);
              DELETE_IF_NOT_NULL(req);
              DELETE_IF_NOT_NULL(nreq);
              DELETE_IF_NOT_NULL(ppe_ctrlr_key);
              dmi->CloseCursor(cfg1_cursor, true);
              return result_code;
            }
          }

          for (ConfigKeyVal *tmp = ppe_ctrlr_key; tmp != NULL; tmp =
              tmp->get_next_cfg_key_val()) {
            GET_USER_DATA_CTRLR(tmp, ctrlr_id);
            string controller(reinterpret_cast<char *>(ctrlr_id));

            UPLL_LOG_DEBUG("Controller ID =%s", controller.c_str());
            DbSubOp dbop_maintbl = { kOpReadSingle, kOpMatchNone,
                                     kOpInOutFlag };
            result_code = GetChildConfigKey(ppe_key, req);
            if (result_code != UPLL_RC_SUCCESS) {
              UPLL_LOG_DEBUG("GetChildConfigKey is failed resultcode=%d",
                  result_code);
              DELETE_IF_NOT_NULL(req);
              DELETE_IF_NOT_NULL(nreq);
              DELETE_IF_NOT_NULL(ppe_ctrlr_key);
              dmi->CloseCursor(cfg1_cursor, true);
              return result_code;
            }
            result_code = ReadConfigDB(ppe_key, UPLL_DT_CANDIDATE,
                                     UNC_OP_READ, dbop_maintbl, dmi, MAINTBL);
            if (result_code != UPLL_RC_SUCCESS) {
              UPLL_LOG_DEBUG("Unable to read configuration from RunningDb");
              DELETE_IF_NOT_NULL(ppe_key);
              DELETE_IF_NOT_NULL(ppe_ctrlr_key);
              DELETE_IF_NOT_NULL(req);
              DELETE_IF_NOT_NULL(nreq);
              dmi->CloseCursor(cfg1_cursor, true);
              return result_code;
            }
            static_cast<val_policingprofile_entry_t *>
              (GetVal(ppe_key))->cs_row_status =
            static_cast<val_policingprofile_entry_t *>
              (GetVal(nreq))->cs_row_status;

            if (ctrlr_result.empty()) {
              UPLL_LOG_TRACE("ctrlr_commit_status is NULL.");
              result_code = UpdateConfigStatus(ppe_key, op[i],
                                             UPLL_RC_ERR_CTR_DISCONNECTED, nreq,
                                             dmi, tmp);
            } else {
              result_code = UpdateConfigStatus(ppe_key, op[i],
                                             ctrlr_result[controller], nreq,
                                             dmi, tmp);
            }
            if (result_code != UPLL_RC_SUCCESS) break;

            void *fle_val1 = GetVal(tmp);
            void *fle_val2 = GetVal(nreq);
            CompareValidValue(fle_val1, fle_val2, true);
            result_code = UpdateConfigDB(tmp,
                                       UPLL_DT_RUNNING, op[i], dmi,
                                       config_mode, vtn_name, CTRLRTBL);
            if (result_code != UPLL_RC_SUCCESS) {
              UPLL_LOG_INFO("UpdateConfigDB for ctrlr tbl is failed:%d",
               result_code);
              DELETE_IF_NOT_NULL(ppe_ctrlr_key);
              DELETE_IF_NOT_NULL(ppe_key);
              DELETE_IF_NOT_NULL(req);
              DELETE_IF_NOT_NULL(nreq);
              dmi->CloseCursor(cfg1_cursor, true);
              return result_code;
            }
            result_code = UpdateConfigDB(ppe_key, UPLL_DT_RUNNING,
                                       op[i], dmi, config_mode, vtn_name,
                                       MAINTBL);
            if (result_code != UPLL_RC_SUCCESS) {
              UPLL_LOG_INFO("UpdateConfigDB for main tbl is failed:%d",
                result_code);
              DELETE_IF_NOT_NULL(ppe_ctrlr_key);
              DELETE_IF_NOT_NULL(ppe_key);
              DELETE_IF_NOT_NULL(req);
              DELETE_IF_NOT_NULL(nreq);
              dmi->CloseCursor(cfg1_cursor, true);
              return result_code;
            }  // COV UNREACHABLE
            EnqueCfgNotification(op[i], UPLL_DT_RUNNING,
                                tmp);
            DELETE_IF_NOT_NULL(ppe_key);
          }
          DELETE_IF_NOT_NULL(ppe_ctrlr_key);
        } else {
          if ((op[i] == UNC_OP_CREATE) && (config_mode != TC_CONFIG_VIRTUAL)) {
            DbSubOp dbop = { kOpReadSingle, kOpMatchNone,
              kOpInOutFlag | kOpInOutCs };
            result_code = GetChildConfigKey(ppe_key, req);
            if (UPLL_RC_SUCCESS != result_code) {
              UPLL_LOG_DEBUG("GetChildConfigKey failed %d", result_code);
              DELETE_IF_NOT_NULL(req);
              DELETE_IF_NOT_NULL(nreq);
              dmi->CloseCursor(cfg1_cursor, true);
              return result_code;
            }
            result_code = ReadConfigDB(ppe_key,
                                      UPLL_DT_RUNNING /*UPLL_DT_CANDIDATE*/,
                                      UNC_OP_READ, dbop, dmi, MAINTBL);
            if ((result_code != UPLL_RC_SUCCESS) &&
               (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE)) {
              UPLL_LOG_DEBUG("ReadConfigDB is failed -%d", result_code);
              DELETE_IF_NOT_NULL(ppe_key);
              if ((config_mode == TC_CONFIG_VTN) &&
                  (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE)) {
                continue;
              }
              DELETE_IF_NOT_NULL(req);
              DELETE_IF_NOT_NULL(nreq);
              dmi->CloseCursor(cfg1_cursor, true);
              return result_code;
            }
           /* set consolidated config status to UNKNOWN to init vtn cs_status
           * to the cs_status of first controller
           */
            uint32_t cur_instance_count;
            result_code = GetInstanceCount(ppe_key, NULL,
                UPLL_DT_CANDIDATE, &cur_instance_count,
                dmi, CTRLRTBL);
            if (UPLL_RC_SUCCESS != result_code) {
              UPLL_LOG_INFO("GetInstanceCount failed %d", result_code);
              DELETE_IF_NOT_NULL(ppe_key);
              DELETE_IF_NOT_NULL(req);
              DELETE_IF_NOT_NULL(nreq);
              dmi->CloseCursor(cfg1_cursor, true);
              return result_code;
            }
            if (cur_instance_count == 1)
              reinterpret_cast<val_policingprofile_entry*>(GetVal(ppe_key))->
                cs_row_status = UNC_CS_UNKNOWN;
          /* Capability check
           * req_header->operation = op[i];
           * strcpy((char*)req_header->datatype, (char*)UNC_DT_CANDIDATE);
           * result_code = ValidateCapability(req_header, vtn_ctrlr_key);
           *                                                 */
            result_code = DupConfigKeyVal(ppe_ctrlr_key, req, tbl);
            if (result_code != UPLL_RC_SUCCESS) {
              UPLL_LOG_DEBUG("DupConfigKeyVal is failed -%d", result_code);
              DELETE_IF_NOT_NULL(ppe_key);
              DELETE_IF_NOT_NULL(req);
              DELETE_IF_NOT_NULL(nreq);
              dmi->CloseCursor(cfg1_cursor, true);
              return result_code;
            }

            GET_USER_DATA_CTRLR(ppe_ctrlr_key, ctrlr_id);
            string controller(reinterpret_cast<char *>(ctrlr_id));
            if (ctrlr_result.empty()) {
              UPLL_LOG_TRACE("ctrlr_commit_status is NULL.");
              result_code = UpdateConfigStatus(ppe_key, op[i],
                 UPLL_RC_ERR_CTR_DISCONNECTED, nreq,
                 dmi, ppe_ctrlr_key);
            } else {
              result_code = UpdateConfigStatus(ppe_key, op[i],
                  ctrlr_result[controller], nreq,
                  dmi, ppe_ctrlr_key);
            }
          } else if ((op[i] == UNC_OP_DELETE) &&
                     (config_mode != TC_CONFIG_VIRTUAL)) {
            // Reading Main Running DB for delete op
            DbSubOp dbop1 = { kOpReadSingle, kOpMatchNone,
              kOpInOutFlag | kOpInOutCs };
            result_code = GetChildConfigKey(pp_ck_run, req);
            if (result_code != UPLL_RC_SUCCESS) {
              UPLL_LOG_DEBUG("GetChildConfigKey is failed resultcode=%d",
                  result_code);
              DELETE_IF_NOT_NULL(req);
              dmi->CloseCursor(cfg1_cursor, true);
              return result_code;
            }
            result_code = ReadConfigDB(pp_ck_run, UPLL_DT_RUNNING,
               UNC_OP_READ, dbop1, dmi, MAINTBL);
            if (result_code != UPLL_RC_SUCCESS &&
                result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
              UPLL_LOG_DEBUG("Unable to read configuration from RunningDB");
              DELETE_IF_NOT_NULL(pp_ck_run);
              DELETE_IF_NOT_NULL(req);
              DELETE_IF_NOT_NULL(nreq);
              dmi->CloseCursor(cfg1_cursor, true);
              return result_code;
            }
            if (result_code == UPLL_RC_SUCCESS) {
              GET_USER_DATA_CTRLR(req, ctrlr_id);
              string controller(reinterpret_cast<char *>(ctrlr_id));
              result_code = SetPPEntryConsolidatedStatus(pp_ck_run,
                                                       ctrlr_id, dmi);
              if (result_code != UPLL_RC_SUCCESS) {
                UPLL_LOG_DEBUG("Could not set consolidated status %d",
                               result_code);
                DELETE_IF_NOT_NULL(pp_ck_run);
                DELETE_IF_NOT_NULL(req);
                DELETE_IF_NOT_NULL(nreq);
                dmi->CloseCursor(cfg1_cursor, true);
                return result_code;
              }
            }
            DELETE_IF_NOT_NULL(pp_ck_run);
            result_code = GetChildConfigKey(ppe_ctrlr_key, req);
            if (UPLL_RC_SUCCESS != result_code) {
              UPLL_LOG_DEBUG("GetChildConfigKey failed %d", result_code);
              DELETE_IF_NOT_NULL(req);
              DELETE_IF_NOT_NULL(nreq);
              dmi->CloseCursor(cfg1_cursor, true);
              return result_code;
            }
          }
          result_code = UpdateConfigDB(ppe_ctrlr_key, UPLL_DT_RUNNING,
                                     op[i], dmi, config_mode, vtn_name,
                                     CTRLRTBL);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("UpdateConfigDB in ctrlr tbl is failed -%d",
                           result_code);
            DELETE_IF_NOT_NULL(ppe_key);
            DELETE_IF_NOT_NULL(ppe_ctrlr_key);
            DELETE_IF_NOT_NULL(req);
            DELETE_IF_NOT_NULL(nreq);
            dmi->CloseCursor(cfg1_cursor, true);
            return result_code;
          }
          if (op[i] != UNC_OP_DELETE) {
            result_code = UpdateConfigDB(ppe_key, UPLL_DT_RUNNING,
                                       UNC_OP_UPDATE, dmi, config_mode,
                                       vtn_name, MAINTBL);
            if (result_code != UPLL_RC_SUCCESS) {
              UPLL_LOG_DEBUG("UpdateConfigDB in main tbl is failed -%d",
                            result_code);
              DELETE_IF_NOT_NULL(ppe_key);
              DELETE_IF_NOT_NULL(ppe_ctrlr_key);
              DELETE_IF_NOT_NULL(req);
              DELETE_IF_NOT_NULL(nreq);
              dmi->CloseCursor(cfg1_cursor, true);
              return result_code;
            }
          }
          EnqueCfgNotification(op[i], UPLL_DT_RUNNING,
                              ppe_key);
        }
        DELETE_IF_NOT_NULL(ppe_key);
        DELETE_IF_NOT_NULL(ppe_ctrlr_key);
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
  string vtn_name = "";

  UPLL_FUNC_TRACE;
  if (op != UNC_OP_DELETE) {
    result_code = DupConfigKeyVal(ck_ppe, ppe_key, MAINTBL);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Returning error %d", result_code);
      return UPLL_RC_ERR_GENERIC;
    }
    ppe_val = reinterpret_cast<val_policingprofile_entry_t *>(GetVal(ck_ppe));
    if (!ppe_val) {
      UPLL_LOG_DEBUG("invalid val");
      return UPLL_RC_ERR_GENERIC;
    }
  } else {
    result_code = GetChildConfigKey(ck_ppe, ppe_key);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Returning error %d", result_code);
      return UPLL_RC_ERR_GENERIC;
    }
  }
  switch (op) {
    case UNC_OP_CREATE:
      ppe_val->cs_row_status = UNC_CS_APPLIED;
      break;
    case UNC_OP_UPDATE:
      ppeval = GetVal(ck_ppe);
      nppeval = (nreq)?GetVal(nreq):NULL;
      if (!nppeval) {
        UPLL_LOG_DEBUG("Invalid param");
        return UPLL_RC_ERR_GENERIC;
      }
      CompareValidValue(ppeval, nppeval, true);
      ppe_val->cs_row_status =
             reinterpret_cast<val_policingprofile_entry_t *>(
                 GetVal(nreq))->cs_row_status;

      break;
    case UNC_OP_DELETE:
      break;
    default:
      UPLL_LOG_DEBUG("Inalid operation");
      return UPLL_RC_ERR_GENERIC;
  }

  DbSubOp dbop = {kOpNotRead, kOpMatchNone, kOpInOutNone};
  dbop.inoutop = kOpInOutCs | kOpInOutFlag;
  result_code = UpdateConfigDB(ck_ppe, UPLL_DT_RUNNING, op, dmi,
                               &dbop, TC_CONFIG_GLOBAL, vtn_name, MAINTBL);
  EnqueCfgNotification(op, UPLL_DT_RUNNING, ck_ppe);
  delete ck_ppe;
  return result_code;
}

bool PolicingProfileEntryMoMgr::CompareValidValue(void *&val1, void *val2,
    bool copy_to_running) {
  UPLL_FUNC_TRACE;
  bool invalid_attr = true;
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
    if (!strncmp(reinterpret_cast<char *>(val_pp_entry1->flowlist),
               reinterpret_cast<char *>(val_pp_entry2->flowlist),
               (kMaxLenFlowListName + 1)))
      val_pp_entry1->valid[UPLL_IDX_FLOWLIST_PPE] = UNC_VF_INVALID;
  }
  if (val_pp_entry1->valid[UPLL_IDX_RATE_PPE] == UNC_VF_VALID
      && val_pp_entry2->valid[UPLL_IDX_RATE_PPE] == UNC_VF_VALID) {
    if (val_pp_entry1->rate == val_pp_entry2->rate) {
      val_pp_entry1->valid[UPLL_IDX_RATE_PPE] =
      (copy_to_running)?UNC_VF_INVALID:UNC_VF_VALUE_NOT_MODIFIED;
    }
  }
  if (val_pp_entry1->valid[UPLL_IDX_CIR_PPE] == UNC_VF_VALID
      && val_pp_entry2->valid[UPLL_IDX_CIR_PPE] == UNC_VF_VALID) {
    if (val_pp_entry1->cir == val_pp_entry2->cir)
      val_pp_entry1->valid[UPLL_IDX_CIR_PPE] =
      (copy_to_running)?UNC_VF_INVALID:UNC_VF_VALUE_NOT_MODIFIED;
  }
  if (val_pp_entry1->valid[UPLL_IDX_CBS_PPE] == UNC_VF_VALID
      && val_pp_entry2->valid[UPLL_IDX_CBS_PPE] == UNC_VF_VALID) {
    if (val_pp_entry1->cbs == val_pp_entry2->cbs)
      val_pp_entry1->valid[UPLL_IDX_CBS_PPE] =
      (copy_to_running)?UNC_VF_INVALID:UNC_VF_VALUE_NOT_MODIFIED;
  }
  if (val_pp_entry1->valid[UPLL_IDX_PIR_PPE] == UNC_VF_VALID
      && val_pp_entry2->valid[UPLL_IDX_PIR_PPE] == UNC_VF_VALID) {
    if (val_pp_entry1->pir == val_pp_entry2->pir)
      val_pp_entry1->valid[UPLL_IDX_PIR_PPE] =
      (copy_to_running)?UNC_VF_INVALID:UNC_VF_VALUE_NOT_MODIFIED;
  }
  if (val_pp_entry1->valid[UPLL_IDX_PBS_PPE] == UNC_VF_VALID
      && val_pp_entry2->valid[UPLL_IDX_PBS_PPE] == UNC_VF_VALID) {
    if (val_pp_entry1->pbs == val_pp_entry2->pbs)
      val_pp_entry1->valid[UPLL_IDX_PBS_PPE] =
      (copy_to_running)?UNC_VF_INVALID:UNC_VF_VALUE_NOT_MODIFIED;
  }
  if (val_pp_entry1->valid[UPLL_IDX_GREEN_ACTION_PPE] == UNC_VF_VALID
      && val_pp_entry2->valid[UPLL_IDX_GREEN_ACTION_PPE] == UNC_VF_VALID) {
    if (val_pp_entry1->green_action == val_pp_entry2->green_action)
      val_pp_entry1->valid[UPLL_IDX_GREEN_ACTION_PPE] =
      (copy_to_running)?UNC_VF_INVALID:UNC_VF_VALUE_NOT_MODIFIED;
  }
  if (val_pp_entry1->valid[UPLL_IDX_GREEN_PRIORITY_PPE] == UNC_VF_VALID
      && val_pp_entry2->valid[UPLL_IDX_GREEN_PRIORITY_PPE] == UNC_VF_VALID) {
    if (val_pp_entry1->green_action_priority ==
        val_pp_entry2->green_action_priority)
      val_pp_entry1->valid[UPLL_IDX_GREEN_PRIORITY_PPE] =
      (copy_to_running)?UNC_VF_INVALID:UNC_VF_VALUE_NOT_MODIFIED;
  }
  if (val_pp_entry1->valid[UPLL_IDX_GREEN_DSCP_PPE] == UNC_VF_VALID
      && val_pp_entry2->valid[UPLL_IDX_GREEN_DSCP_PPE] == UNC_VF_VALID) {
    if (val_pp_entry1->green_action_dscp == val_pp_entry2->green_action_dscp)
      val_pp_entry1->valid[UPLL_IDX_GREEN_DSCP_PPE] =
      (copy_to_running)?UNC_VF_INVALID:UNC_VF_VALUE_NOT_MODIFIED;
  }
  if (val_pp_entry1->valid[UPLL_IDX_GREEN_DROP_PPE] == UNC_VF_VALID
      && val_pp_entry2->valid[UPLL_IDX_GREEN_DROP_PPE] == UNC_VF_VALID) {
    if (val_pp_entry1->green_action_drop_precedence ==
        val_pp_entry2->green_action_drop_precedence)
      val_pp_entry1->valid[UPLL_IDX_GREEN_DROP_PPE] =
      (copy_to_running)?UNC_VF_INVALID:UNC_VF_VALUE_NOT_MODIFIED;
  }
  if (val_pp_entry1->valid[UPLL_IDX_YELLOW_ACTION_PPE] == UNC_VF_VALID
      && val_pp_entry2->valid[UPLL_IDX_YELLOW_ACTION_PPE] == UNC_VF_VALID) {
    if (val_pp_entry1->yellow_action == val_pp_entry2->yellow_action)
      val_pp_entry1->valid[UPLL_IDX_YELLOW_ACTION_PPE] =
      (copy_to_running)?UNC_VF_INVALID:UNC_VF_VALUE_NOT_MODIFIED;
  }
  if (val_pp_entry1->valid[UPLL_IDX_YELLOW_PRIORITY_PPE] == UNC_VF_VALID
      && val_pp_entry2->valid[UPLL_IDX_YELLOW_PRIORITY_PPE] == UNC_VF_VALID) {
    if (val_pp_entry1->yellow_action_priority ==
        val_pp_entry2->yellow_action_priority)
      val_pp_entry1->valid[UPLL_IDX_YELLOW_PRIORITY_PPE] =
      (copy_to_running)?UNC_VF_INVALID:UNC_VF_VALUE_NOT_MODIFIED;
  }
  if (val_pp_entry1->valid[UPLL_IDX_YELLOW_DSCP_PPE] == UNC_VF_VALID
      && val_pp_entry2->valid[UPLL_IDX_YELLOW_DSCP_PPE] == UNC_VF_VALID) {
    if (val_pp_entry1->yellow_action_dscp == val_pp_entry2->yellow_action_dscp)
      val_pp_entry1->valid[UPLL_IDX_YELLOW_DSCP_PPE] =
      (copy_to_running)?UNC_VF_INVALID:UNC_VF_VALUE_NOT_MODIFIED;
  }
  if (val_pp_entry1->valid[UPLL_IDX_YELLOW_DROP_PPE] == UNC_VF_VALID
      && val_pp_entry2->valid[UPLL_IDX_YELLOW_DROP_PPE] == UNC_VF_VALID) {
    if (val_pp_entry1->yellow_action_drop_precedence ==
        val_pp_entry2->yellow_action_drop_precedence)
      val_pp_entry1->valid[UPLL_IDX_YELLOW_DROP_PPE] =
      (copy_to_running)?UNC_VF_INVALID:UNC_VF_VALUE_NOT_MODIFIED;
  }
  if (val_pp_entry1->valid[UPLL_IDX_RED_ACTION_PPE] == UNC_VF_VALID
      && val_pp_entry2->valid[UPLL_IDX_RED_ACTION_PPE] == UNC_VF_VALID) {
    if (val_pp_entry1->red_action == val_pp_entry2->red_action)
      val_pp_entry1->valid[UPLL_IDX_RED_ACTION_PPE] =
      (copy_to_running)?UNC_VF_INVALID:UNC_VF_VALUE_NOT_MODIFIED;
  }
  if (val_pp_entry1->valid[UPLL_IDX_RED_PRIORITY_PPE] == UNC_VF_VALID
      && val_pp_entry2->valid[UPLL_IDX_RED_PRIORITY_PPE] == UNC_VF_VALID) {
    if (val_pp_entry1->red_action_priority ==
        val_pp_entry2->red_action_priority)
      val_pp_entry1->valid[UPLL_IDX_RED_PRIORITY_PPE] =
      (copy_to_running)?UNC_VF_INVALID:UNC_VF_VALUE_NOT_MODIFIED;
  }
  if (val_pp_entry1->valid[UPLL_IDX_RED_DSCP_PPE] == UNC_VF_VALID
      && val_pp_entry2->valid[UPLL_IDX_RED_DSCP_PPE] == UNC_VF_VALID) {
    if (val_pp_entry1->red_action_dscp ==
        val_pp_entry2->red_action_dscp)
      val_pp_entry1->valid[UPLL_IDX_RED_DSCP_PPE] =
      (copy_to_running)?UNC_VF_INVALID:UNC_VF_VALUE_NOT_MODIFIED;
  }
  if (val_pp_entry1->valid[UPLL_IDX_RED_DROP_PPE] == UNC_VF_VALID
      && val_pp_entry2->valid[UPLL_IDX_RED_DROP_PPE] == UNC_VF_VALID) {
    if (val_pp_entry1->red_action_drop_precedence ==
        val_pp_entry2->red_action_drop_precedence)
      val_pp_entry1->valid[UPLL_IDX_RED_DROP_PPE] =
      (copy_to_running)?UNC_VF_INVALID:UNC_VF_VALUE_NOT_MODIFIED;
  }
  for (unsigned int loop = 0;
      loop < sizeof(val_pp_entry1->valid)/ sizeof(uint8_t); ++loop) {
    if ((UNC_VF_VALID == (uint8_t) val_pp_entry1->valid[loop]) ||
        (UNC_VF_VALID_NO_VALUE == (uint8_t) val_pp_entry1->valid[loop]))
      invalid_attr = false;
  }
  return invalid_attr;
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
  DELETE_IF_NOT_NULL(okey);
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
      val_ppe->cbs = 0;
      val_ppe->valid[UPLL_IDX_CBS_PPE] = UNC_VF_VALID_NO_VALUE;
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
      val_ppe->pbs = 0;
      val_ppe->valid[UPLL_IDX_PBS_PPE] = UNC_VF_VALID_NO_VALUE;
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
        UPLL_LOG_DEBUG("cir and pir are mandatory");
        return UPLL_RC_ERR_CFG_SYNTAX;
      }
    } else if (UNC_OP_UPDATE == operation) {
      if ((UNC_VF_VALID_NO_VALUE == val_ppe->valid[UPLL_IDX_PIR_PPE])
        || (UNC_VF_VALID_NO_VALUE == val_ppe->valid[UPLL_IDX_CIR_PPE])) {
        UPLL_LOG_DEBUG("cir and pir are mandatory");
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
   /** If Green Action attribute is removed (valid_no_value),
     * then subsequent attributes should be reset.
     */
    val_ppe->green_action_priority = 0;
    val_ppe->green_action_drop_precedence = 0;
    val_ppe->green_action_drop_precedence = 0;
    val_ppe->valid[UPLL_IDX_GREEN_PRIORITY_PPE] = UNC_VF_VALID_NO_VALUE;
    val_ppe->valid[UPLL_IDX_GREEN_DSCP_PPE] = UNC_VF_VALID_NO_VALUE;
    val_ppe->valid[UPLL_IDX_GREEN_DROP_PPE] = UNC_VF_VALID_NO_VALUE;
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
   /** If Red Action attribute is removed (valid_no_value),
     * then subsequent attributes should be reset.
     */
    val_ppe->red_action_priority = 0;
    val_ppe->red_action_drop_precedence = 0;
    val_ppe->red_action_drop_precedence = 0;
    val_ppe->valid[UPLL_IDX_RED_PRIORITY_PPE] = UNC_VF_VALID_NO_VALUE;
    val_ppe->valid[UPLL_IDX_RED_DSCP_PPE] = UNC_VF_VALID_NO_VALUE;
    val_ppe->valid[UPLL_IDX_RED_DROP_PPE] = UNC_VF_VALID_NO_VALUE;
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
   /** If Yellow Action attribute is removed (valid_no_value),
     * then subsequent attributes should be reset.
     */
    val_ppe->yellow_action_priority = 0;
    val_ppe->yellow_action_drop_precedence = 0;
    val_ppe->yellow_action_drop_precedence = 0;
    val_ppe->valid[UPLL_IDX_YELLOW_PRIORITY_PPE] = UNC_VF_VALID_NO_VALUE;
    val_ppe->valid[UPLL_IDX_YELLOW_DSCP_PPE] = UNC_VF_VALID_NO_VALUE;
    val_ppe->valid[UPLL_IDX_YELLOW_DROP_PPE] = UNC_VF_VALID_NO_VALUE;
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

  if ((NULL == req) || (NULL == ikey)) {
    UPLL_LOG_DEBUG("IpcReqRespHeader/ConfigKeyval is NULL");
    return UPLL_RC_ERR_GENERIC;
  }

  if (!ctrlr_name)
    ctrlr_name = static_cast<char *>(ikey->get_user_data());

  if (NULL == ctrlr_name) {
    UPLL_LOG_DEBUG("ctrlr_name is NULL");
    return UPLL_RC_ERR_GENERIC;
  }

  UPLL_LOG_TRACE("ctrlr_name(%s), operation : (%d)",
               ctrlr_name, req->operation);

  bool ret_code = false;
  uint32_t max_instance_count = 0;
  const uint8_t *attrs = NULL;
  uint32_t max_attrs = 0;

  switch (req->operation) {
    case UNC_OP_CREATE: {
      ret_code = GetCreateCapability(ctrlr_name, ikey->get_key_type(),
                                        &max_instance_count,
                                        &max_attrs, &attrs);
      break;
    }
    case UNC_OP_UPDATE: {
      ret_code = GetUpdateCapability(ctrlr_name, ikey->get_key_type(),
                                        &max_attrs, &attrs);
      break;
    }
    default: {
      if (req->datatype == UPLL_DT_STATE)
        ret_code = GetStateCapability(ctrlr_name, ikey->get_key_type(),
                                      &max_attrs, &attrs);
      else
        ret_code = GetReadCapability(ctrlr_name, ikey->get_key_type(),
                                      &max_attrs, &attrs);
    }
  }

  if (!ret_code) {
    UPLL_LOG_DEBUG("keytype(%d) is not supported by controller(%s) "
                   "for operation(%d)",
                   ikey->get_key_type(), ctrlr_name, req->operation);
    return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
  }

  val_policingprofile_entry_t *val_policingprofile_entry =
        reinterpret_cast<val_policingprofile_entry_t *>(
           GetVal(ikey));

  if (val_policingprofile_entry) {
    if (max_attrs > 0) {
      return ValPolicingProfileEntryAttributeSupportCheck(
            val_policingprofile_entry, attrs);
    } else {
      UPLL_LOG_DEBUG("Attribute list is empty for operation %d",
                     req->operation);
      return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
    }
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t PolicingProfileEntryMoMgr::
ValPolicingProfileEntryAttributeSupportCheck(
  val_policingprofile_entry_t *val_policingprofile_entry,
  const uint8_t *attrs) {
  UPLL_FUNC_TRACE;

  if ((val_policingprofile_entry->valid[UPLL_IDX_FLOWLIST_PPE] ==
       UNC_VF_VALID)
      || (val_policingprofile_entry->valid[UPLL_IDX_FLOWLIST_PPE]
          == UNC_VF_VALID_NO_VALUE)) {
    if (attrs[unc::capa::policingprofile_entry::kCapFlowlist] == 0) {
      UPLL_LOG_DEBUG("FLOWLIST  attr is not supported by ctrlr");
      val_policingprofile_entry->valid[UPLL_IDX_FLOWLIST_PPE] =
          UNC_VF_NOT_SUPPORTED;
    }
  }

  if ((val_policingprofile_entry->valid[UPLL_IDX_RATE_PPE] ==
       UNC_VF_VALID)
      || (val_policingprofile_entry->valid[UPLL_IDX_RATE_PPE]
          == UNC_VF_VALID_NO_VALUE)) {
    if (attrs[unc::capa::policingprofile_entry::kCapRate] == 0) {
      UPLL_LOG_DEBUG("FLOWLIST attr is not supported by ctrlr");
      val_policingprofile_entry->valid[UPLL_IDX_RATE_PPE] =
          UNC_VF_NOT_SUPPORTED;
    }
  }

  if ((val_policingprofile_entry->valid[UPLL_IDX_CIR_PPE] == UNC_VF_VALID)
      || (val_policingprofile_entry->valid[UPLL_IDX_CIR_PPE]
          == UNC_VF_VALID_NO_VALUE)) {
    if (attrs[unc::capa::policingprofile_entry::kCapCir] == 0) {
      UPLL_LOG_DEBUG("CIR attr is not supported by ctrlr");
      val_policingprofile_entry->valid[UPLL_IDX_CIR_PPE] =
          UNC_VF_NOT_SUPPORTED;
    }
  }

  if ((val_policingprofile_entry->valid[UPLL_IDX_CBS_PPE] == UNC_VF_VALID)
      || (val_policingprofile_entry->valid[UPLL_IDX_CBS_PPE]
          == UNC_VF_VALID_NO_VALUE)) {
    if (attrs[unc::capa::policingprofile_entry::kCapCbs] == 0) {
      UPLL_LOG_DEBUG("CBS attr is not supported by ctrlr");
      val_policingprofile_entry->valid[UPLL_IDX_CBS_PPE] =
          UNC_VF_NOT_SUPPORTED;
    }
  }

  if ((val_policingprofile_entry->valid[UPLL_IDX_PIR_PPE] == UNC_VF_VALID)
      || (val_policingprofile_entry->valid[UPLL_IDX_PIR_PPE]
          == UNC_VF_VALID_NO_VALUE)) {
    if (attrs[unc::capa::policingprofile_entry::kCapPir] == 0) {
      UPLL_LOG_DEBUG("PIR attr is not supported by ctrlr");
      val_policingprofile_entry->valid[UPLL_IDX_PIR_PPE] =
          UNC_VF_NOT_SUPPORTED;
    }
  }

  if ((val_policingprofile_entry->valid[UPLL_IDX_PBS_PPE] == UNC_VF_VALID)
      || (val_policingprofile_entry->valid[UPLL_IDX_PBS_PPE]
          == UNC_VF_VALID_NO_VALUE)) {
    if (attrs[unc::capa::policingprofile_entry::kCapPbs] == 0) {
      UPLL_LOG_DEBUG("PBS attr is not supported by ctrlr");
      val_policingprofile_entry->valid[UPLL_IDX_PBS_PPE] =
          UNC_VF_NOT_SUPPORTED;
    }
  }

  ValidateGreenFieldAttribute(val_policingprofile_entry, attrs);

  ValidateRedFieldAttribute(val_policingprofile_entry, attrs);

  ValidateYellowFieldAttribute(val_policingprofile_entry, attrs);

  return UPLL_RC_SUCCESS;
}

void PolicingProfileEntryMoMgr::ValidateGreenFieldAttribute(
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
          UNC_VF_NOT_SUPPORTED;
    }
  }

  if ((val_policingprofile_entry->valid[UPLL_IDX_GREEN_PRIORITY_PPE]
      == UNC_VF_VALID)
      || (val_policingprofile_entry->valid[UPLL_IDX_GREEN_PRIORITY_PPE]
          == UNC_VF_VALID_NO_VALUE)) {
    if (attrs[unc::capa::policingprofile_entry::kCapGreenPriority] == 0) {
      UPLL_LOG_DEBUG("GREEN_PRIORITY attr is not supported by ctrlr");
      val_policingprofile_entry->valid[UPLL_IDX_GREEN_PRIORITY_PPE] =
          UNC_VF_NOT_SUPPORTED;
    }
  }

  if ((val_policingprofile_entry->valid[UPLL_IDX_GREEN_DSCP_PPE] ==
        UNC_VF_VALID)
      || (val_policingprofile_entry->valid[UPLL_IDX_GREEN_DSCP_PPE]
          == UNC_VF_VALID_NO_VALUE)) {
    if (attrs[unc::capa::policingprofile_entry::kCapGreenDscp] == 0) {
      UPLL_LOG_DEBUG("GREEN_DSCP attr is not supported by ctrlr");
      val_policingprofile_entry->valid[UPLL_IDX_GREEN_DSCP_PPE] =
          UNC_VF_NOT_SUPPORTED;
    }
  }

  if ((val_policingprofile_entry->valid[UPLL_IDX_GREEN_DROP_PPE] ==
      UNC_VF_VALID)
      || (val_policingprofile_entry->valid[UPLL_IDX_GREEN_DROP_PPE]
          == UNC_VF_VALID_NO_VALUE)) {
    if (attrs[unc::capa::policingprofile_entry::kCapGreenDrop] == 0) {
      UPLL_LOG_DEBUG("GREEN_DROP attr is not supported by ctrlr");
      val_policingprofile_entry->valid[UPLL_IDX_GREEN_DROP_PPE] =
          UNC_VF_NOT_SUPPORTED;
    }
  }
}

void PolicingProfileEntryMoMgr::ValidateYellowFieldAttribute(
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
          UNC_VF_NOT_SUPPORTED;
    }
  }

  if ((val_policingprofile_entry->valid[UPLL_IDX_YELLOW_PRIORITY_PPE]
      == UNC_VF_VALID)
      || (val_policingprofile_entry->valid[UPLL_IDX_YELLOW_PRIORITY_PPE]
          == UNC_VF_VALID_NO_VALUE)) {
    if (attrs[unc::capa::policingprofile_entry::kCapYellowPriority] == 0) {
      UPLL_LOG_DEBUG("YELLOW_PRIORITY attr is not supported by ctrlr");
      val_policingprofile_entry->valid[UPLL_IDX_YELLOW_PRIORITY_PPE] =
          UNC_VF_NOT_SUPPORTED;
    }
  }

  if ((val_policingprofile_entry->valid[UPLL_IDX_YELLOW_DSCP_PPE]
      == UNC_VF_VALID)
      || (val_policingprofile_entry->valid[UPLL_IDX_YELLOW_DSCP_PPE]
          == UNC_VF_VALID_NO_VALUE)) {
    if (attrs[unc::capa::policingprofile_entry::kCapYellowDscp] == 0) {
      UPLL_LOG_DEBUG("YELLOW_DSCP attr is not supported by ctrlr");
      val_policingprofile_entry->valid[UPLL_IDX_YELLOW_DSCP_PPE] =
          UNC_VF_NOT_SUPPORTED;
    }
  }

  if ((val_policingprofile_entry->valid[UPLL_IDX_YELLOW_DROP_PPE]
      == UNC_VF_VALID)
      || (val_policingprofile_entry->valid[UPLL_IDX_YELLOW_DROP_PPE]
          == UNC_VF_VALID_NO_VALUE)) {
    if (attrs[unc::capa::policingprofile_entry::kCapYellowDrop] == 0) {
      UPLL_LOG_DEBUG("YELLOW_DROP attr is not supported by ctrlr");
      val_policingprofile_entry->valid[UPLL_IDX_YELLOW_DROP_PPE] =
          UNC_VF_NOT_SUPPORTED;
    }
  }
}

void PolicingProfileEntryMoMgr::ValidateRedFieldAttribute(
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
          UNC_VF_NOT_SUPPORTED;
    }
  }

  if ((val_policingprofile_entry->valid[UPLL_IDX_RED_PRIORITY_PPE]
      == UNC_VF_VALID)
      || (val_policingprofile_entry->valid[UPLL_IDX_RED_PRIORITY_PPE]
          == UNC_VF_VALID_NO_VALUE)) {
    if (attrs[unc::capa::policingprofile_entry::kCapRedPriority] == 0) {
      UPLL_LOG_DEBUG("RED_PRIORITY attr is not supported by ctrlr");
      val_policingprofile_entry->valid[UPLL_IDX_RED_PRIORITY_PPE] =
          UNC_VF_NOT_SUPPORTED;
    }
  }

  if ((val_policingprofile_entry->valid[UPLL_IDX_RED_DSCP_PPE] == UNC_VF_VALID)
      || (val_policingprofile_entry->valid[UPLL_IDX_RED_DSCP_PPE]
          == UNC_VF_VALID_NO_VALUE)) {
    if (attrs[unc::capa::policingprofile_entry::kCapRedDscp] == 0) {
      UPLL_LOG_DEBUG("RED_DSCP attr is not supported by ctrlr");
      val_policingprofile_entry->valid[UPLL_IDX_RED_DSCP_PPE] =
          UNC_VF_NOT_SUPPORTED;
    }
  }

  if ((val_policingprofile_entry->valid[UPLL_IDX_RED_DROP_PPE] == UNC_VF_VALID)
      || (val_policingprofile_entry->valid[UPLL_IDX_RED_DROP_PPE]
          == UNC_VF_VALID_NO_VALUE)) {
    if (attrs[unc::capa::policingprofile_entry::kCapRedDrop] == 0) {
      UPLL_LOG_DEBUG("RED_DROP attr is not supported by ctrlr");
      val_policingprofile_entry->valid[UPLL_IDX_RED_DROP_PPE] =
          UNC_VF_NOT_SUPPORTED;
    }
  }
}

upll_rc_t PolicingProfileEntryMoMgr::CopyToConfigKey(ConfigKeyVal *&okey,
    ConfigKeyVal *ikey) {
  UPLL_FUNC_TRACE;
  if (NULL == ikey || NULL ==(ikey->get_key()) ||
      NULL != okey) {
    UPLL_LOG_DEBUG("Invalid Input");
    return UPLL_RC_ERR_GENERIC;
  }

  upll_rc_t result_code = UPLL_RC_SUCCESS;
  key_rename_vnode_info_t *key_rename =
    reinterpret_cast<key_rename_vnode_info_t *>(ikey->get_key());

  key_policingprofile_entry_t *key_policingprofile =
    reinterpret_cast<key_policingprofile_entry_t *>(ConfigKeyVal::Malloc(
          sizeof(key_policingprofile_entry_t)));
  if (!strlen(
        reinterpret_cast<char *>(key_rename->old_policingprofile_name))) {
    UPLL_LOG_ERROR("key_rename->old_policingprofile_name NULL");
    if (key_policingprofile) free(key_policingprofile);
    return UPLL_RC_ERR_GENERIC;
  }
  uuu::upll_strncpy(
      key_policingprofile->policingprofile_key.policingprofile_name,
      key_rename->old_policingprofile_name,
      (kMaxLenPolicingProfileName + 1));

  okey = new ConfigKeyVal(UNC_KT_POLICING_PROFILE_ENTRY,
      IpctSt::kIpcStKeyPolicingprofileEntry,
      key_policingprofile, NULL);
  if (!okey) {
    UPLL_LOG_DEBUG("okey NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  return result_code;
}

upll_rc_t PolicingProfileEntryMoMgr::UpdateVnodeVal(ConfigKeyVal *ikey,
                                             DalDmlIntf *dmi,
                                             upll_keytype_datatype_t data_type,
                                             bool &no_rename) {
  UPLL_FUNC_TRACE;
  ConfigKeyVal *okey = NULL;
  ConfigKeyVal *kval = NULL;

  uint8_t rename = 0;
  string vtn_name = "";
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  key_rename_vnode_info_t *key_rename =
  reinterpret_cast<key_rename_vnode_info_t *>(ikey->get_key());
  UPLL_LOG_TRACE("old name (%s) (%s)", key_rename->old_flowlist_name,
              key_rename->new_flowlist_name);
  val_policingprofile_entry_t *val_ppe =
         reinterpret_cast<val_policingprofile_entry_t *>
         (ConfigKeyVal::Malloc(sizeof(val_policingprofile_entry_t)));
  if (!val_ppe) {
    UPLL_LOG_TRACE("val_ppe Failed");
    return UPLL_RC_ERR_GENERIC;
  }

  memset(val_ppe, 0, sizeof(val_policingprofile_entry_t));
  if (!strlen(reinterpret_cast<char *>(key_rename->old_flowlist_name))) {
    UPLL_LOG_ERROR("old_flowlist_name null");
    if (val_ppe) free(val_ppe);
    return UPLL_RC_ERR_GENERIC;
  }

  result_code = GetChildConfigKey(okey, NULL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_TRACE("GetChildConfigKey Failed ");
    FREE_IF_NOT_NULL(val_ppe);
    return UPLL_RC_ERR_GENERIC;
  }
  uuu::upll_strncpy(val_ppe->flowlist, key_rename->old_flowlist_name,
      (kMaxLenFlowListName + 1));
  val_ppe->valid[UPLL_IDX_FLOWLIST_PPE] = UNC_VF_VALID;
  UPLL_LOG_DEBUG("policer name and valid (%d) (%s)",
                  val_ppe->valid[UPLL_IDX_FLOWLIST_PPE], val_ppe->flowlist);
  okey->SetCfgVal(new ConfigVal
                  (IpctSt::kIpcStValPolicingprofileEntry, val_ppe));

  DbSubOp dbop = { kOpReadMultiple, kOpMatchNone, kOpInOutFlag };

  // Read the record of key structure and old policer name in maintbl
  result_code = ReadConfigDB(okey, data_type, UNC_OP_READ, dbop, dmi,
    MAINTBL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG(" ReadConfigDB failed ");
    DELETE_IF_NOT_NULL(okey);
    return result_code;
  }
  ConfigKeyVal *first_ckv = okey;
  while (okey != NULL) {
    // Update the new flowlist name in MAINTBL
    result_code = GetChildConfigKey(kval, okey);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("GetChildConfigKey kval NULL");
      DELETE_IF_NOT_NULL(first_ckv);
      return result_code;
    }

    // Copy the new flowlist name in val_policingprofile_entry
    val_policingprofile_entry_t *val1 =
         reinterpret_cast<val_policingprofile_entry_t *>
         (ConfigKeyVal::Malloc(sizeof(val_policingprofile_entry_t)));
    if (!val1) return UPLL_RC_ERR_GENERIC;
    memset(val1, 0, sizeof(val_policingprofile_entry_t));

    // New name null check
    if (!strlen(reinterpret_cast<char *>(key_rename->new_flowlist_name))) {
      if (val1) free(val1);
      UPLL_LOG_DEBUG("new_policingprofile_name NULL");
      DELETE_IF_NOT_NULL(kval);
      DELETE_IF_NOT_NULL(first_ckv);
      return UPLL_RC_ERR_GENERIC;
    }
    // Copy the new flowlist name into val_policingprofile_entry_t
     uuu::upll_strncpy(val1->flowlist, key_rename->new_flowlist_name,
      (kMaxLenFlowListName + 1));
    val1->valid[UPLL_IDX_FLOWLIST_PPE] = UNC_VF_VALID;
    UPLL_LOG_DEBUG("policer name and valid (%d) (%s)",
                    val1->valid[UPLL_IDX_FLOWLIST_PPE], val1->flowlist);

    ConfigVal *cval1 =
        new ConfigVal(IpctSt::kIpcStValPolicingprofileEntry, val1);

    kval->SetCfgVal(cval1);

    GET_USER_DATA_FLAGS(okey, rename);

    if (!no_rename)
      rename = rename | FLOWLIST_RENAME;
    else
      rename = rename & NO_FLOWLIST_RENAME;

    SET_USER_DATA_FLAGS(kval, rename);

    // Update the new flowlist name in MAINTBL
    result_code = UpdateConfigDB(kval, data_type, UNC_OP_UPDATE, dmi,
                  TC_CONFIG_GLOBAL, vtn_name, MAINTBL);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("Create record Err in vtnpolicingmaptbl CANDIDATE DB(%d)",
        result_code);
      DELETE_IF_NOT_NULL(kval);
      DELETE_IF_NOT_NULL(first_ckv);
      return result_code;
    }
    DELETE_IF_NOT_NULL(kval);
    okey = okey->get_next_cfg_key_val();
  }
  DELETE_IF_NOT_NULL(first_ckv);
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
    free(key_rename);
    return result_code;
  }
  while (okey) {
    result_code = GetChildConfigKey(tmp_key, okey);
    if (result_code != UPLL_RC_SUCCESS) {
      free(key_rename);  // COV RESOURCE LEAK FIC
      return result_code;
    }
    result_code = GetChildConfigKey(ctrlr_tmp_key, okey);
    if (result_code != UPLL_RC_SUCCESS) {
      // COV RESOURCE LEAK FIX
      delete tmp_key;
      free(key_rename);
      return result_code;
    }
    if (!tmp_key || !(tmp_key->get_key())) {  // COVERITY FORWARD NULL
      delete ctrlr_tmp_key;  // COV RESOURCE LEAK FIX
      free(key_rename);
      delete tmp_key;
      return UPLL_RC_ERR_GENERIC;
    }
    // COVERITY FORWARD NULL
    if (!ctrlr_tmp_key || !(ctrlr_tmp_key->get_key())) {
      // COV RESOURCE LEAK FIX
      free(key_rename);
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
      DELETE_IF_NOT_NULL(tmp_key);
      DELETE_IF_NOT_NULL(ctrlr_tmp_key);
      DELETE_IF_NOT_NULL(okey);
      return result_code;
    }
    SET_USER_DATA_CTRLR(ctrlr_tmp_key, ctrlr_id);
    DbSubOp dbop1 = { kOpNotRead, kOpMatchCtrlr, kOpInOutFlag };
    result_code = UpdateRenameKey(tmp_key, data_type, UNC_OP_UPDATE, dmi,
        &dbop1, CTRLRTBL);
    if (UPLL_RC_SUCCESS != result_code) {
      DELETE_IF_NOT_NULL(tmp_key);
      DELETE_IF_NOT_NULL(ctrlr_tmp_key);
      DELETE_IF_NOT_NULL(okey);
      return result_code;
    }
    // Check with Sarath - Start
    DELETE_IF_NOT_NULL(tmp_key);
    DELETE_IF_NOT_NULL(ctrlr_tmp_key);
    okey = okey->get_next_cfg_key_val();
  }
  DELETE_IF_NOT_NULL(okey);  // Logically dead code
  FREE_IF_NOT_NULL(key_rename);
  return result_code;
}

bool PolicingProfileEntryMoMgr::IsValidKey(void *ikey, uint64_t index,
                                           MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  key_policingprofile_entry *key =
    reinterpret_cast<key_policingprofile_entry *>(ikey);
  upll_rc_t ret_val = UPLL_RC_SUCCESS;

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
    unc_keytype_operation_t oper, upll_keytype_datatype_t dt_type,
    TcConfigMode config_mode, string vtn_name, bool commit) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *ppe_ckv = NULL;
  result_code = GetPolicingProfileEntryCtrlrKeyval(ppe_ckv,
      policingprofile_name,
      ctrlr_id);
  if (UPLL_RC_SUCCESS != result_code) {
    DELETE_IF_NOT_NULL(ppe_ckv);
    return result_code;
  }

  if (UNC_OP_CREATE == oper) {
    result_code = CtrlrTblCreate(ppe_ckv, dmi, dt_type, config_mode,
                                 vtn_name, commit);
  } else if (UNC_OP_DELETE == oper) {
    ConfigKeyVal *okey = NULL;
    result_code = GetPolicingprofileKeyVal(okey, ppe_ckv);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("GetPolicingprofileKeyVal failed %d", result_code);
      DELETE_IF_NOT_NULL(ppe_ckv);
      return result_code;
    }
    PolicingProfileMoMgr *mgr = reinterpret_cast<PolicingProfileMoMgr *>
        (const_cast<MoManager *>(GetMoManager(UNC_KT_POLICING_PROFILE)));
    if (NULL == mgr) {
      UPLL_LOG_DEBUG("mgr is NULL");
      DELETE_IF_NOT_NULL(ppe_ckv);
      DELETE_IF_NOT_NULL(okey);
      return UPLL_RC_ERR_GENERIC;
    }
    DbSubOp dbop = { kOpReadExist, kOpMatchCtrlr, kOpInOutNone };
    result_code = mgr->UpdateConfigDB(okey, dt_type, UNC_OP_READ, dmi, &dbop,
      CTRLRTBL);
    if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
      UPLL_LOG_DEBUG("Delete in entry table");
      result_code = CtrlrTblDelete(ppe_ckv, dmi, dt_type, config_mode,
                                   vtn_name, commit);
      DELETE_IF_NOT_NULL(okey);
    } else if (UPLL_RC_ERR_INSTANCE_EXISTS == result_code) {
      UPLL_LOG_DEBUG("Dont delete . Drecrement refcount in flowlist");
      result_code = DecrementRefCount(okey, dmi, dt_type, config_mode,
                                      vtn_name);
      if (UPLL_RC_SUCCESS != result_code) {
        UPLL_LOG_DEBUG("DecrementRefCount failed %d", result_code);
        DELETE_IF_NOT_NULL(ppe_ckv);
        DELETE_IF_NOT_NULL(okey);
        return result_code;
      }
      DELETE_IF_NOT_NULL(ppe_ckv);
      DELETE_IF_NOT_NULL(okey);
    } else {
      UPLL_LOG_DEBUG("Update configdb failed %d", result_code);
      DELETE_IF_NOT_NULL(ppe_ckv);
      DELETE_IF_NOT_NULL(okey);
      return result_code;
    }
  } else {
    result_code = UPLL_RC_ERR_NO_SUCH_OPERATION;
  }
  DELETE_IF_NOT_NULL(ppe_ckv);  // COV RESOURCE LEAK FIX
  return result_code;
}

upll_rc_t PolicingProfileEntryMoMgr::DecrementRefCount(
    ConfigKeyVal *ppe_ckv, DalDmlIntf *dmi, upll_keytype_datatype_t dt_type,
    TcConfigMode config_mode, string vtn_name) {
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
    DELETE_IF_NOT_NULL(temp_ckv);
    return UPLL_RC_SUCCESS;
  }
  ConfigKeyVal *loop_ckv = temp_ckv;
  while (loop_ckv) {
    GET_USER_DATA_CTRLR(loop_ckv, ctrlr_id);
    val_policingprofile_entry_t *temp_ppe_val = reinterpret_cast
        <val_policingprofile_entry_t *>(GetVal(loop_ckv));
    if (NULL == temp_ppe_val) {
      UPLL_LOG_DEBUG("val_policingprofile_entry is NULL");
      DELETE_IF_NOT_NULL(temp_ckv);
      return UPLL_RC_ERR_GENERIC;
    }
    if (UNC_VF_VALID == temp_ppe_val->valid[UPLL_IDX_FLOWLIST_PPE ]) {
     FlowListMoMgr *flowlist_mgr = reinterpret_cast<FlowListMoMgr *>
      (const_cast<MoManager *> (GetMoManager(UNC_KT_FLOWLIST)));
     if (NULL == flowlist_mgr) {
       UPLL_LOG_DEBUG("flowlist_mgr is NULL");
       DELETE_IF_NOT_NULL(temp_ckv);
       return UPLL_RC_ERR_GENERIC;
     }
     result_code = flowlist_mgr->AddFlowListToController(
          reinterpret_cast<char *>(temp_ppe_val->flowlist), dmi,
          reinterpret_cast<char *>(ctrlr_id), dt_type, UNC_OP_DELETE,
          config_mode, vtn_name);
      if (result_code != UPLL_RC_SUCCESS) {
        if (UPLL_RC_ERR_INSTANCE_EXISTS == result_code) {
          continue;
        }
        UPLL_LOG_DEBUG("AddFlowListToController failed err code(%d)",
                       result_code)
        DELETE_IF_NOT_NULL(temp_ckv);
        return result_code;
      }
    }
    loop_ckv = loop_ckv->get_next_cfg_key_val();
  }
  DELETE_IF_NOT_NULL(temp_ckv);
  return UPLL_RC_SUCCESS;
}

upll_rc_t PolicingProfileEntryMoMgr::CtrlrTblDelete(
    ConfigKeyVal *ppe_ckv, DalDmlIntf *dmi, upll_keytype_datatype_t dt_type,
    TcConfigMode config_mode, string vtn_name, bool commit) {
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
    DELETE_IF_NOT_NULL(temp_ckv);
    return UPLL_RC_SUCCESS;
  }
  ConfigKeyVal *loop_ckv = temp_ckv;
  while (loop_ckv) {
    GET_USER_DATA_CTRLR(loop_ckv, ctrlr_id);
    val_policingprofile_entry_t *temp_ppe_val = reinterpret_cast
        <val_policingprofile_entry_t *>(GetVal(loop_ckv));
    if (NULL == temp_ppe_val) {
      UPLL_LOG_DEBUG("val_policingprofile_entry is NULL");
      DELETE_IF_NOT_NULL(temp_ckv);
      return UPLL_RC_ERR_GENERIC;
    }
    if (!commit) {
      if (UNC_VF_VALID == temp_ppe_val->valid[UPLL_IDX_FLOWLIST_PPE ]) {
       FlowListMoMgr *flowlist_mgr = reinterpret_cast<FlowListMoMgr *>
        (const_cast<MoManager *> (GetMoManager(UNC_KT_FLOWLIST)));
       result_code = flowlist_mgr->AddFlowListToController(
            reinterpret_cast<char *>(temp_ppe_val->flowlist), dmi,
            reinterpret_cast<char *>(ctrlr_id), dt_type, UNC_OP_DELETE,
            config_mode, vtn_name, false);
        if (result_code != UPLL_RC_SUCCESS) {
          if (UPLL_RC_ERR_INSTANCE_EXISTS == result_code) {
            loop_ckv = loop_ckv->get_next_cfg_key_val();
            continue;
          }
          UPLL_LOG_DEBUG("AddFlowListToController failed err code(%d)",
                         result_code)
          DELETE_IF_NOT_NULL(temp_ckv);
          return result_code;
        }
      }
    }
    loop_ckv = loop_ckv->get_next_cfg_key_val();
  }
  DELETE_IF_NOT_NULL(temp_ckv);
  DbSubOp dbop = { kOpNotRead, kOpMatchCtrlr, kOpInOutNone };
  result_code = UpdateConfigDB(ppe_ckv, dt_type, UNC_OP_DELETE, dmi, &dbop,
      config_mode, vtn_name, CTRLRTBL);
  if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
    return UPLL_RC_SUCCESS;
  } else if (UPLL_RC_SUCCESS != result_code) {
    return result_code;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t PolicingProfileEntryMoMgr::CtrlrTblCreate(
    ConfigKeyVal *ppe_ckv, DalDmlIntf *dmi, upll_keytype_datatype_t dt_type,
    TcConfigMode config_mode, string vtn_name, bool commit) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  uint8_t *ctrlr_id = NULL;

  DbSubOp dbop = { kOpReadMultiple, kOpMatchNone, kOpInOutFlag | kOpInOutCs };

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
    uint8_t flag = 0;
    GET_USER_DATA_CTRLR(ppe_ckv, ctrlr_id);
    val_policingprofile_entry_t *temp_ppe_val = reinterpret_cast
        <val_policingprofile_entry_t *>(GetVal(ppe_ckv));
    if (NULL == temp_ppe_val) {
      UPLL_LOG_DEBUG("PolicingprofileEntry val is Null");
      free(req_header);
      return UPLL_RC_ERR_GENERIC;
    }
    if (UNC_VF_VALID == temp_ppe_val->valid[UPLL_IDX_FLOWLIST_PPE ]) {
     FlowListMoMgr *flowlist_mgr = reinterpret_cast<FlowListMoMgr *>
      (const_cast<MoManager *> (GetMoManager(UNC_KT_FLOWLIST)));
     if (NULL == flowlist_mgr) {
       UPLL_LOG_DEBUG("flowlist_mgr is NULL");
       free(req_header);
       return UPLL_RC_ERR_GENERIC;
     }
     result_code = flowlist_mgr->AddFlowListToController(
          reinterpret_cast<char *>(temp_ppe_val->flowlist), dmi,
          reinterpret_cast<char *>(ctrlr_id), dt_type, UNC_OP_CREATE,
          config_mode, vtn_name, false);
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
    GET_USER_DATA_FLAGS(ppe_ckv, flag);
    SET_USER_DATA_FLAGS(new_ppe, flag);
    val_policingprofile_entry_ctrl_t *val_ctrlr =
            reinterpret_cast<val_policingprofile_entry_ctrl_t *>
            (ConfigKeyVal::Malloc(sizeof(val_policingprofile_entry_ctrl_t)));
    if (NULL == val_ctrlr) {
      free(req_header);
      DELETE_IF_NOT_NULL(new_ppe);
      return UPLL_RC_ERR_GENERIC;
    }
    new_ppe->AppendCfgVal(IpctSt::kIpcInvalidStNum, val_ctrlr);
    val_policingprofile_entry_t *val =
      reinterpret_cast<val_policingprofile_entry_t *>(GetVal(ppe_ckv));
    val_ctrlr->cs_row_status = (unc_keytype_configstatus_t)val->cs_row_status;

    // capability check
    result_code = ValidateCapability(
      req_header, ppe_ckv, reinterpret_cast<char *>(ctrlr_id));
    unc_keytype_ctrtype_t ctrlrtype = UNC_CT_UNKNOWN;
    uuc::CtrlrMgr *ctrlr_mgr = uuc::CtrlrMgr::GetInstance();
    if (result_code != UPLL_RC_SUCCESS) {
      // Policingprofile is not supported for other than PFC Controller
      // so skip adding entry for such sontroller in ctrlr table
      DELETE_IF_NOT_NULL(new_ppe);
      if ((!ctrlr_mgr->GetCtrlrType(reinterpret_cast<char *>(ctrlr_id),
                     dt_type, &ctrlrtype)) || (ctrlrtype != UNC_CT_PFC)) {
          result_code = UPLL_RC_SUCCESS;
          UPLL_LOG_DEBUG("Controller type is  %d", ctrlrtype);
          ppe_ckv = ppe_ckv->get_next_cfg_key_val();
          continue;
       }
       free(req_header);
       UPLL_LOG_DEBUG("Key not supported by controller");
       return result_code;
    }

    for (unsigned int loop = 0;
         loop < (sizeof(val_ctrlr->valid)/sizeof(val_ctrlr->valid[0]));
         loop++) {
      if (val->valid[loop] == UNC_VF_NOT_SUPPORTED)
         val_ctrlr->valid[loop] = UNC_VF_INVALID;
      else
        val_ctrlr->valid[loop] = val->valid[loop];
      UPLL_LOG_DEBUG("valid of %d - %d  ctrlrtbl - %d", loop,  val->valid[loop],
                       val_ctrlr->valid[loop]);
    }
    // Audit
    if (UPLL_DT_AUDIT == dt_type) {
      UPLL_LOG_DEBUG("Audit db setting cs");
      for (unsigned int loop = 0;
         loop < (sizeof(val_ctrlr->valid)/sizeof(val_ctrlr->valid[0]));
         loop++) {
        val_ctrlr->cs_attr[loop] =
            (unc_keytype_configstatus_t)val->cs_attr[loop];
      }
      val_ctrlr->cs_row_status = (unc_keytype_configstatus_t)val->cs_row_status;
    }
    result_code = UpdateConfigDB(new_ppe, dt_type, UNC_OP_CREATE, dmi,
        config_mode, vtn_name, CTRLRTBL);
    if (UPLL_RC_SUCCESS != result_code) {
      if (UPLL_RC_ERR_INSTANCE_EXISTS == result_code) {
        ppe_ckv = ppe_ckv->get_next_cfg_key_val();
        DELETE_IF_NOT_NULL(new_ppe);
        continue;
      }
      DELETE_IF_NOT_NULL(new_ppe);
      free(req_header);
      return result_code;
    }
    ppe_ckv = ppe_ckv->get_next_cfg_key_val();
    DELETE_IF_NOT_NULL(new_ppe);
  }
  free(req_header);
  return UPLL_RC_SUCCESS;
}

upll_rc_t PolicingProfileEntryMoMgr::SetConsolidatedStatus(ConfigKeyVal *ikey,
    DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *ckv = NULL;
  string vtn_name = "";

  DbSubOp dbop = { kOpReadMultiple, kOpMatchNone, kOpInOutCs};
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
  val_policingprofile_entry_ctrl_t *val;
  for (unsigned int loop = 0; loop < sizeof(val->valid)/sizeof(val->valid[0]);
      ++loop) {
    std::list < unc_keytype_configstatus_t > list_attr;
    vec_attr.push_back(list_attr);
  }
  ConfigKeyVal *temp_ckv = ckv;
  for (; temp_ckv != NULL; temp_ckv = temp_ckv->get_next_cfg_key_val()) {
    val = reinterpret_cast<val_policingprofile_entry_ctrl_t *>
                          (GetVal(temp_ckv));
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
      TC_CONFIG_GLOBAL, vtn_name, MAINTBL);
  if (UPLL_RC_SUCCESS != result_code) {
    return result_code;
  }
  DELETE_IF_NOT_NULL(ckv);
  return result_code;
}

upll_rc_t PolicingProfileEntryMoMgr::ReadPolicingProfileEntry(
    const char *policingprofile_name, uint8_t seq_num, const char *ctrlr_id,
    DalDmlIntf *dmi, upll_keytype_datatype_t dt_type, ConfigKeyVal *&ppe_ckv,
    unc_keytype_option1_t opt1) {
  UPLL_FUNC_TRACE;

  const uint8_t *attrs = NULL;
  uint32_t max_attrs = 0;
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
    bool ret_code = false;

    if ((opt1 == UNC_OPT1_DETAIL) && (dt_type == UPLL_DT_STATE)) {
      ret_code = GetStateCapability(ctrlr_id, UNC_KT_POLICING_PROFILE_ENTRY,
                                      &max_attrs, &attrs);
      if (!ret_code) {
        UPLL_LOG_DEBUG(
            "GetStateCapability Is failed in PolicingProfileEntry %d",
            ret_code);
        return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
      }
      if (max_attrs > 0) {
        result_code =  ValPolicingProfileEntryAttributeSupportCheck(
                                     val, attrs);
      }
      for (unsigned int loop = 0; loop <
          sizeof(val->valid[loop]/sizeof(val->valid[0])); loop++) {
        val->cs_attr[loop] = ctrl_val->cs_attr[loop];
      }
    } else {
      for (unsigned int loop = 0; loop <
          sizeof(val->valid[loop]/sizeof(val->valid[0])); loop++) {
        val->valid[loop] = ctrl_val->valid[loop];
        val->cs_attr[loop] = ctrl_val->cs_attr[loop];
      }
    }

  if (val->valid[UPLL_IDX_FLOWLIST_PPE] != UNC_VF_VALID) {
      uuu::upll_strncpy(val->flowlist,
                      "\0", (kMaxLenFlowListName + 1));
  }
  if (val->valid[UPLL_IDX_CBS_PPE] != UNC_VF_VALID) {
    val->cbs = 0;
  }
  if (val->valid[UPLL_IDX_PIR_PPE] != UNC_VF_VALID) {
    val->pir = 0;
  }
  if (val->valid[UPLL_IDX_PBS_PPE] != UNC_VF_VALID) {
    val->pbs = 0;
  }
  if (val->valid[UPLL_IDX_GREEN_ACTION_PPE] != UNC_VF_VALID) {
    val->green_action = 0;
  }
  if (val->valid[UPLL_IDX_GREEN_PRIORITY_PPE] != UNC_VF_VALID) {
    val->green_action_priority = 0;
  }
  if (val->valid[UPLL_IDX_GREEN_DSCP_PPE] != UNC_VF_VALID) {
    val->green_action_dscp = 0;
  }
  if (val->valid[UPLL_IDX_GREEN_DROP_PPE] != UNC_VF_VALID) {
    val->green_action_drop_precedence = 0;
  }

  if (val->valid[UPLL_IDX_YELLOW_ACTION_PPE] != UNC_VF_VALID) {
    val->yellow_action = 0;
  }

  if (val->valid[UPLL_IDX_YELLOW_PRIORITY_PPE] != UNC_VF_VALID) {
    val->yellow_action_priority = 0;
  }

  if (val->valid[UPLL_IDX_YELLOW_DSCP_PPE] != UNC_VF_VALID) {
    val->yellow_action_dscp = 0;
  }

  if (val->valid[UPLL_IDX_YELLOW_DROP_PPE] != UNC_VF_VALID) {
    val->yellow_action_drop_precedence = 0;
  }

  if (val->valid[UPLL_IDX_RED_ACTION_PPE] != UNC_VF_VALID) {
    val->red_action = 0;
  }

  if (val->valid[UPLL_IDX_RED_PRIORITY_PPE] != UNC_VF_VALID) {
    val->red_action_priority = 0;
  }

  if (val->valid[UPLL_IDX_RED_DSCP_PPE] != UNC_VF_VALID) {
    val->red_action_dscp = 0;
  }

  if (val->valid[UPLL_IDX_RED_DROP_PPE] != UNC_VF_VALID) {
    val->red_action_drop_precedence = 0;
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
      return UPLL_RC_ERR_GENERIC;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t PolicingProfileEntryMoMgr::IsFlowlistConfigured(
  const char* flowlist_name, upll_keytype_datatype_t dt_type,
  DalDmlIntf *dmi) {
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
  ppe_val->valid[UPLL_IDX_FLOWLIST_PPE] = UNC_VF_VALID;
  ckv->AppendCfgVal(IpctSt::kIpcStValPolicingprofileEntry, ppe_val);
  DbSubOp dbop = {kOpReadMultiple, kOpMatchNone, kOpInOutNone};
  result_code = ReadConfigDB(ckv, dt_type, UNC_OP_READ, dbop,
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
  UPLL_LOG_TRACE("UpdateMo for %d", ikey->get_key_type());
  result_code = ValidateMessage(req, ikey);
  if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_ERROR("Validation Message is Failed ");
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
  result_code = ValidateAttribute(ikey, dmi, req);
  if (UPLL_RC_SUCCESS  != result_code) {
      UPLL_LOG_ERROR("Validate Attribute is Failed");
      return result_code;
  }
  if (IsAllAttrInvalid(
    reinterpret_cast<val_policingprofile_entry_t *>(GetVal(ikey)))) {
    UPLL_LOG_INFO("No attributes to be updated");
    return UPLL_RC_SUCCESS;
  }
  TcConfigMode config_mode = TC_CONFIG_INVALID;
  std::string vtn_name = "";
  result_code = GetConfigModeInfo(req, config_mode, vtn_name);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetConfigMode failed");
    return result_code;
  }

  DbSubOp dbop1 = {kOpNotRead, kOpMatchNone, kOpInOutFlag};
  result_code = UpdateConfigDB(ikey, req->datatype, UNC_OP_UPDATE,
                               dmi, &dbop1, config_mode, vtn_name, MAINTBL);
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
  if (UNC_OP_CREATE == req->operation) {
    DbSubOp dbop = {kOpReadExist, kOpMatchNone, kOpInOutNone};
    result_code = UpdateConfigDB(ikey, req->datatype, UNC_OP_READ,
                               dmi, &dbop, MAINTBL);
    if (UPLL_RC_ERR_INSTANCE_EXISTS == result_code) {
      UPLL_LOG_DEBUG("Instance already exists, result_code %d",
                     result_code);
      return result_code;
    } else if (UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
      UPLL_LOG_INFO("err while checking instance existance in DB, err(%d)",
                  result_code);
      return result_code;
    }
  }
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

upll_rc_t PolicingProfileEntryMoMgr::SetRenameFlag(ConfigKeyVal *ikey,
    DalDmlIntf *dmi,
    IpcReqRespHeader *req) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  val_policingprofile_entry_t *val_ppe = reinterpret_cast
    <val_policingprofile_entry_t *>(GetVal(ikey));
  if (!val_ppe) {
    UPLL_LOG_DEBUG("Val is NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  ConfigKeyVal *pkey = NULL;
  if (UNC_OP_CREATE == req->operation) {
    result_code = GetParentConfigKey(pkey, ikey);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("GetParentConfigKey failed %d", result_code);
      return result_code;
    }
    MoMgrImpl *mgr =
      reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>(GetMoManager(
              UNC_KT_POLICING_PROFILE)));
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
    DELETE_IF_NOT_NULL(pkey);
    // Check flowlist is renamed
    if ((UNC_VF_VALID == val_ppe->valid[UPLL_IDX_FLOWLIST_PPE]) &&
        ((UNC_OP_CREATE == req->operation))) {
      ConfigKeyVal *fl_ckv = NULL;
      result_code = GetFlowlistConfigKey(reinterpret_cast<const char *>
          (val_ppe->flowlist), fl_ckv, dmi);
      if (UPLL_RC_SUCCESS != result_code) {
        UPLL_LOG_DEBUG("GetFlowlistConfigKey failed %d", result_code);
        return result_code;
      }
      MoMgrImpl *fl_mgr =
      reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>(GetMoManager(
              UNC_KT_FLOWLIST)));
      if (NULL == fl_mgr) {
        UPLL_LOG_DEBUG("flow list KT reference is NULL");
        DELETE_IF_NOT_NULL(fl_ckv);
        return UPLL_RC_ERR_GENERIC;
      }
      uint8_t fl_rename = 0;
      result_code = fl_mgr->IsRenamed(fl_ckv, req->datatype, dmi, fl_rename);
      if (UPLL_RC_SUCCESS != result_code) {
        UPLL_LOG_DEBUG("IsRenamed failed %d", result_code);
        DELETE_IF_NOT_NULL(fl_ckv);
        return result_code;
      }
      if (fl_rename & 0x01) {
        rename |= FLOWLIST_RENAME;  // TODO(rename) Check for correct flag value
      }
      DELETE_IF_NOT_NULL(fl_ckv);
    }
    SET_USER_DATA_FLAGS(ikey, rename);
  } else if (UNC_OP_UPDATE == req->operation) {
    uint8_t rename = 0;
    result_code = IsRenamed(ikey, req->datatype, dmi, rename);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("IsRenamed failed %d", result_code);
      return result_code;
    }
    if (UNC_VF_VALID == val_ppe->valid[UPLL_IDX_FLOWLIST_PPE]) {
      ConfigKeyVal *fl_ckv = NULL;
      result_code = GetFlowlistConfigKey(reinterpret_cast<const char *>
          (val_ppe->flowlist), fl_ckv, dmi);
      if (UPLL_RC_SUCCESS != result_code) {
        UPLL_LOG_DEBUG("GetFlowlistConfigKey failed %d", result_code);
        return result_code;
      }
      MoMgrImpl *fl_mgr =
      reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>(GetMoManager(
              UNC_KT_FLOWLIST)));
      if (NULL == fl_mgr) {
        UPLL_LOG_DEBUG("flow list KT reference is NULL");
        DELETE_IF_NOT_NULL(fl_ckv);
        return UPLL_RC_ERR_GENERIC;
      }
      uint8_t fl_rename = 0;
      result_code = fl_mgr->IsRenamed(fl_ckv, req->datatype, dmi, fl_rename);
      if (UPLL_RC_SUCCESS != result_code) {
        UPLL_LOG_DEBUG("IsRenamed failed %d", result_code);
        DELETE_IF_NOT_NULL(fl_ckv);
        return result_code;
      }
      if (fl_rename & 0x01) {
        rename |= FLOWLIST_RENAME;  // TODO(rename) Check for correct flag value
      } else {
        rename = rename & NO_FLOWLIST_RENAME;
      }
      DELETE_IF_NOT_NULL(fl_ckv);
    } else if (UNC_VF_VALID_NO_VALUE == val_ppe->valid
               [UPLL_IDX_FLOWLIST_PPE]) {
       // TODO(rename) Check for correct flag value.
       // No rename flowlist value should be set
       rename |= ~FLOWLIST_RENAME;
    }
    SET_USER_DATA_FLAGS(ikey, rename);
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t PolicingProfileEntryMoMgr::GetFlowlistConfigKey(
        const char *flowlist_name, ConfigKeyVal *&okey,
        DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_ERR_GENERIC;
  MoMgrImpl *mgr =
      reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>(GetMoManager(
              UNC_KT_FLOWLIST)));
  if (NULL == mgr) {
    UPLL_LOG_DEBUG("Flowlist instance is NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  result_code = mgr->GetChildConfigKey(okey, NULL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetChildConfigKey failed %d", result_code);
    return result_code;
  }
  key_flowlist_t *okey_key = reinterpret_cast<key_flowlist_t *>
      (okey->get_key());
  uuu::upll_strncpy(okey_key->flowlist_name,
        flowlist_name,
        (kMaxLenFlowListName+1));
  return UPLL_RC_SUCCESS;
}

upll_rc_t PolicingProfileEntryMoMgr::ValidatePolicingProfileName(
    ConfigKeyVal *ikey, DalDmlIntf *dmi,
    IpcReqRespHeader *req) {
  UPLL_FUNC_TRACE;
  upll_rc_t rt_code = UPLL_RC_ERR_GENERIC;

  // if policing profile is applied in policing map return error
  if (req->operation == UNC_OP_CREATE) {
    rt_code = ValidatePolicingProfileEntryInPolicingMap(ikey, dmi, req);
    if (rt_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Update not allowed when policingprofile is in use");
      return rt_code;
    }
    if (req->datatype == UPLL_DT_CANDIDATE) {
      TcConfigMode config_mode = TC_CONFIG_INVALID;
      std::string vtn_name = "";
      rt_code = GetConfigModeInfo(req, config_mode, vtn_name);
      if (rt_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("GetConfigModeInfo failed");
        return rt_code;
      }

      if (config_mode == TC_CONFIG_VIRTUAL) {
        req->datatype = UPLL_DT_RUNNING;
        rt_code = ValidatePolicingProfileEntryInPolicingMap(ikey, dmi, req);
        req->datatype = UPLL_DT_CANDIDATE;
        if (rt_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("Update not allowed when policingprofile is in use");
          return rt_code;
        }
      }
    }
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
      if ((val_ppe->valid[UPLL_IDX_FLOWLIST_PPE] == UNC_VF_VALID)
      || (val_ppe->valid[UPLL_IDX_RATE_PPE] == UNC_VF_VALID)
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
      if ((val_ppe->valid[UPLL_IDX_FLOWLIST_PPE] == UNC_VF_VALID)
      || (val_ppe->valid[UPLL_IDX_RATE_PPE] == UNC_VF_VALID)
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
      || (val_ppe->valid[UPLL_IDX_FLOWLIST_PPE] == UNC_VF_VALID_NO_VALUE)
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
      rt_code = ValidatePolicingProfileEntryInPolicingMap(ikey, dmi, req);
      if (rt_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Update not allowed when policingprofile is in use");
        return rt_code;
      }
      if (req->datatype == UPLL_DT_CANDIDATE) {
        TcConfigMode config_mode = TC_CONFIG_INVALID;
        std::string vtn_name = "";
        rt_code = GetConfigModeInfo(req, config_mode, vtn_name);
        if (rt_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("GetConfigMode failed");
          return rt_code;
        }

        if (config_mode == TC_CONFIG_VIRTUAL) {
          req->datatype = UPLL_DT_RUNNING;
          rt_code = ValidatePolicingProfileEntryInPolicingMap(ikey, dmi, req);
          req->datatype = UPLL_DT_CANDIDATE;
          if (rt_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("Update not allowed when policingprofile is in use");
            return rt_code;
          }
        }
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
      /** If rate attribute is removed then the subsequent fields
        * should also be reset
        **/
      val_ppe->cir = 0;
      val_ppe->cbs = 0;
      val_ppe->pir = 0;
      val_ppe->pbs = 0;
      val_ppe->valid[UPLL_IDX_CIR_PPE] = UNC_VF_VALID_NO_VALUE;
      val_ppe->valid[UPLL_IDX_CBS_PPE] = UNC_VF_VALID_NO_VALUE;
      val_ppe->valid[UPLL_IDX_PIR_PPE] = UNC_VF_VALID_NO_VALUE;
      val_ppe->valid[UPLL_IDX_PBS_PPE] = UNC_VF_VALID_NO_VALUE;
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
      return rt_code;
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

    delete fl_ckv;
    if (rt_code != UPLL_RC_ERR_INSTANCE_EXISTS) {
      UPLL_LOG_DEBUG("flowlist name does not exists in "
          "KT_FLOWLIST table");
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

upll_rc_t PolicingProfileEntryMoMgr::ValidatePolicingProfileEntryInPolicingMap(
    ConfigKeyVal *ikey,
    DalDmlIntf *dmi,
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
    DELETE_IF_NOT_NULL(tempval);
    return UPLL_RC_ERR_GENERIC;
  }
  ConfigKeyVal *vtn_ckv = NULL;
  result_code = mgr->GetChildConfigKey(vtn_ckv, NULL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetChildConfigKey Failed %d", result_code);
    DELETE_IF_NOT_NULL(tempval);
    return result_code;
  }
  vtn_ckv->AppendCfgVal(tempval);

  DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutNone };
  result_code = mgr->ReadConfigDB(vtn_ckv, req->datatype, UNC_OP_READ, dbop,
      dmi, MAINTBL);
  if (result_code == UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Policing profile name used in VTN_POLICINGMAP table");
    DELETE_IF_NOT_NULL(vtn_ckv);
    return UPLL_RC_ERR_CFG_SEMANTIC;
  } else if (UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
    UPLL_LOG_DEBUG("Policing profile name check in pm failed %d",
          result_code);
    DELETE_IF_NOT_NULL(vtn_ckv);
    return result_code;
  }
  DELETE_IF_NOT_NULL(vtn_ckv);

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
    DELETE_IF_NOT_NULL(tempval1);
    return UPLL_RC_ERR_GENERIC;
  }
  ConfigKeyVal *vbr_ckv = NULL;
  result_code = mgr->GetChildConfigKey(vbr_ckv, NULL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetChildConfigKey Failed %d", result_code);
    DELETE_IF_NOT_NULL(tempval1);
    return result_code;
  }
  vbr_ckv->AppendCfgVal(tempval1);

  result_code = mgr->ReadConfigDB(vbr_ckv, req->datatype, UNC_OP_READ, dbop,
      dmi, MAINTBL);
  if (result_code == UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Policing profile name used in VTN_POLICINGMAP table");
    DELETE_IF_NOT_NULL(vbr_ckv);
    return UPLL_RC_ERR_CFG_SEMANTIC;
  } else if (UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
    UPLL_LOG_DEBUG("Policing profile name check in pm failed %d",
          result_code);
    DELETE_IF_NOT_NULL(vbr_ckv);
    return result_code;
  }
  DELETE_IF_NOT_NULL(vbr_ckv);

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
    DELETE_IF_NOT_NULL(tempval2);
    return UPLL_RC_ERR_GENERIC;
  }
  ConfigKeyVal *vbrif_ckv = NULL;
  result_code = mgr->GetChildConfigKey(vbrif_ckv, NULL);
  if (UPLL_RC_SUCCESS != result_code) {
    DELETE_IF_NOT_NULL(tempval2);
    UPLL_LOG_DEBUG("GetChildConfigKey Failed %d", result_code);
    return result_code;
  }
  vbrif_ckv->AppendCfgVal(tempval2);

  result_code = mgr->ReadConfigDB(vbrif_ckv, req->datatype, UNC_OP_READ, dbop,
      dmi, MAINTBL);
  if (result_code == UPLL_RC_SUCCESS) {
    DELETE_IF_NOT_NULL(vbrif_ckv);
    UPLL_LOG_DEBUG("Policing profile name used in VTN_POLICINGMAP table");
    return UPLL_RC_ERR_CFG_SEMANTIC;
  } else if (UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
    UPLL_LOG_DEBUG("Policing profile name check in pm failed %d",
          result_code);
    DELETE_IF_NOT_NULL(vbrif_ckv);
    return result_code;
  }
  DELETE_IF_NOT_NULL(vbrif_ckv);

  val_policingmap_t *val_policingmap3 =
    reinterpret_cast<val_policingmap_t *>
    (ConfigKeyVal::Malloc(sizeof(val_policingmap_t)));
  uuu::upll_strncpy(val_policingmap3->policer_name,
    key_ppe->policingprofile_key.policingprofile_name,
    (kMaxLenPolicingProfileName + 1));
  val_policingmap3->valid[UPLL_IDX_POLICERNAME_PM ] = UNC_VF_VALID;
  ConfigVal *tempval3 = new ConfigVal(IpctSt::kIpcStValPolicingmap,
      val_policingmap3);
  mgr =
    reinterpret_cast<MoMgrImpl *>(const_cast<MoManager*>(GetMoManager(
            UNC_KT_VTERMIF_POLICINGMAP)));
  if (!mgr) {
    DELETE_IF_NOT_NULL(tempval3);
    return UPLL_RC_ERR_GENERIC;
  }
  ConfigKeyVal *vtermifpm_ckv = NULL;
  result_code = mgr->GetChildConfigKey(vtermifpm_ckv, NULL);
  if (UPLL_RC_SUCCESS != result_code) {
    DELETE_IF_NOT_NULL(tempval3);
    UPLL_LOG_DEBUG("GetChildConfigKey Failed %d", result_code);
    return result_code;
  }
  vtermifpm_ckv->AppendCfgVal(tempval3);

  result_code = mgr->ReadConfigDB(vtermifpm_ckv, req->datatype,
                                  UNC_OP_READ, dbop,
                                  dmi, MAINTBL);
  if (result_code == UPLL_RC_SUCCESS) {
    DELETE_IF_NOT_NULL(vtermifpm_ckv);
    UPLL_LOG_DEBUG("Policing profile name used in VTN_POLICINGMAP table");
    return UPLL_RC_ERR_CFG_SEMANTIC;
  } else if (UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
    UPLL_LOG_DEBUG("Policing profile name check in pm failed %d",
          result_code);
    DELETE_IF_NOT_NULL(vtermifpm_ckv);
    return result_code;
  }
  DELETE_IF_NOT_NULL(vtermifpm_ckv);

  return UPLL_RC_SUCCESS;
}

upll_rc_t PolicingProfileEntryMoMgr::GetFlowListEntryConfigKey(
        ConfigKeyVal *&okey, ConfigKeyVal *ikey) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  val_policingprofile_entry_t *val = reinterpret_cast
      <val_policingprofile_entry_t *>(GetVal(ikey));
  key_flowlist_entry_t *out_key = reinterpret_cast
      <key_flowlist_entry_t *>(ConfigKeyVal::
     Malloc(sizeof(key_flowlist_entry_t)));
  uuu::upll_strncpy(out_key->flowlist_key.flowlist_name,
      val->flowlist, kMaxLenFlowListName);
  okey = new ConfigKeyVal(UNC_KT_FLOWLIST_ENTRY,
             IpctSt::kIpcStKeyFlowlistEntry, out_key, NULL);
  if (!okey)
    return UPLL_RC_ERR_GENERIC;
  return result_code;
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
    UPLL_LOG_ERROR("ReadConfigDB failed:%d", result_code);
    delete ppe_ckv;
    return result_code;
  }
  ConfigKeyVal *temp_ppe_ckv = ppe_ckv;
  while (NULL != temp_ppe_ckv) {
    val_policingprofile_entry_t *val_ppe = reinterpret_cast
      <val_policingprofile_entry_t *>(GetVal(temp_ppe_ckv));
    if (UNC_VF_VALID != val_ppe->valid[UPLL_IDX_FLOWLIST_PPE ]) {
      UPLL_LOG_DEBUG("Attribute flowlist is not valid");
      DELETE_IF_NOT_NULL(ppe_ckv);
      return UPLL_RC_ERR_CFG_SEMANTIC;
    }
    if (UNC_VF_VALID == val_ppe->valid[UPLL_IDX_FLOWLIST_PPE ]) {
      ConfigKeyVal *okey = NULL;
      result_code = GetFlowListEntryConfigKey(okey, temp_ppe_ckv);
      if (UPLL_RC_SUCCESS != result_code) {
        DELETE_IF_NOT_NULL(ppe_ckv);
        return result_code;
      }
      MoMgrImpl *fle_mgr =
          reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>(GetMoManager(
            UNC_KT_FLOWLIST_ENTRY)));
      if (NULL == fle_mgr) {
        UPLL_LOG_DEBUG("fle_mgr is NULL");
        DELETE_IF_NOT_NULL(ppe_ckv);
        DELETE_IF_NOT_NULL(okey);
        return UPLL_RC_ERR_GENERIC;
      }
      result_code = fle_mgr->UpdateConfigDB(okey,
                  dt_type, UNC_OP_READ, dmi, MAINTBL);
      if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
        UPLL_LOG_DEBUG("No seq number for given flowlist");
        DELETE_IF_NOT_NULL(ppe_ckv);
        DELETE_IF_NOT_NULL(okey);
        return UPLL_RC_ERR_CFG_SEMANTIC;
      } else if (UPLL_RC_ERR_INSTANCE_EXISTS != result_code) {
        UPLL_LOG_DEBUG("UpdateConfigDB failed %d", result_code);
        DELETE_IF_NOT_NULL(ppe_ckv);
        DELETE_IF_NOT_NULL(okey);
        return result_code;
      }
      DELETE_IF_NOT_NULL(okey);
    }
    if (UNC_VF_VALID != val_ppe->valid[UPLL_IDX_RATE_PPE]) {
      UPLL_LOG_DEBUG("Attribute rate is not valid");
      DELETE_IF_NOT_NULL(ppe_ckv);
      return UPLL_RC_ERR_CFG_SEMANTIC;
    }
    if (UNC_VF_VALID != val_ppe->valid[UPLL_IDX_CIR_PPE]) {
      UPLL_LOG_DEBUG("Attribute cir is not valid");
      DELETE_IF_NOT_NULL(ppe_ckv);
      return UPLL_RC_ERR_CFG_SEMANTIC;
    }
    if (UNC_VF_VALID != val_ppe->valid[UPLL_IDX_PIR_PPE]) {
      UPLL_LOG_DEBUG("Attribute pir is not valid");
      DELETE_IF_NOT_NULL(ppe_ckv);
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
          DELETE_IF_NOT_NULL(ppe_ckv);
          return UPLL_RC_ERR_CFG_SEMANTIC;
        }
      }
    } else {
      DELETE_IF_NOT_NULL(ppe_ckv);
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
          DELETE_IF_NOT_NULL(ppe_ckv);
          return UPLL_RC_ERR_CFG_SEMANTIC;
        }
      }
    } else {
      DELETE_IF_NOT_NULL(ppe_ckv);
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
          DELETE_IF_NOT_NULL(ppe_ckv);
          return UPLL_RC_ERR_CFG_SEMANTIC;
        }
      }
    } else {
      DELETE_IF_NOT_NULL(ppe_ckv);
      return UPLL_RC_ERR_CFG_SEMANTIC;
    }
    temp_ppe_ckv = temp_ppe_ckv->get_next_cfg_key_val();
  }
  DELETE_IF_NOT_NULL(ppe_ckv);
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
    DELETE_IF_NOT_NULL(ppe_ckv);
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
      DELETE_IF_NOT_NULL(ppe_ckv);
      return result_code;
    }
    DbSubOp dbop1 = {kOpReadMultiple, kOpMatchNone, kOpInOutNone};
    result_code = ReadConfigDB(ctrlr_ppe_ckv, dt_type, UNC_OP_READ, dbop1, dmi,
                    CTRLRTBL);
    if (UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
      DELETE_IF_NOT_NULL(ppe_ckv);
      DELETE_IF_NOT_NULL(ctrlr_ppe_ckv);
      if (UPLL_RC_SUCCESS == result_code) {
        UPLL_LOG_DEBUG("Flowlist is referred by policingprofile which"
                       "is referred by a policingmap");
        return UPLL_RC_ERR_INSTANCE_EXISTS;
      }
      UPLL_LOG_DEBUG("ReadConfigDB failed %d", result_code);
      return result_code;
    }
    temp_ppe_ckv = temp_ppe_ckv->get_next_cfg_key_val();
    DELETE_IF_NOT_NULL(ctrlr_ppe_ckv);
  }
  DELETE_IF_NOT_NULL(ppe_ckv);
  return UPLL_RC_SUCCESS;
}

upll_rc_t PolicingProfileEntryMoMgr::SetValidAudit(ConfigKeyVal *&ikey) {
  UPLL_FUNC_TRACE;
  val_policingprofile_entry_t *val = reinterpret_cast
      <val_policingprofile_entry_t *>(GetVal(ikey));
  if (NULL == val) {
    UPLL_LOG_DEBUG("val is NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  for (unsigned int loop = 0;
        loop < sizeof(val->valid) / sizeof(val->valid[0]);
        ++loop) {
    val->cs_attr[loop] = UNC_CS_APPLIED;
  }
  val->cs_row_status = UNC_CS_APPLIED;
  return UPLL_RC_SUCCESS;
}

bool PolicingProfileEntryMoMgr::FilterAttributes(void *&val1,
                                          void *val2,
                                          bool copy_to_running,
                                          unc_keytype_operation_t op) {
  UPLL_FUNC_TRACE;
  if (op != UNC_OP_CREATE)
    return CompareValidValue(val1, val2, copy_to_running);
  return false;
}

upll_rc_t PolicingProfileEntryMoMgr::UpdateConfigStatus(ConfigKeyVal *main_ckv,
    unc_keytype_operation_t op,
    uint32_t driver_result,
    ConfigKeyVal *upd_key,
    DalDmlIntf *dmi,
    ConfigKeyVal *ctrlr_key) {
  UPLL_FUNC_TRACE;
  val_policingprofile_entry_ctrl_t *ppe_val;
  unc_keytype_configstatus_t  ctrlr_status;
  uint8_t cs_status;
  val_policingprofile_entry_t *val_ppe_main = reinterpret_cast
    <val_policingprofile_entry_t *>(GetVal(main_ckv));
  ctrlr_status =
    (driver_result == UPLL_RC_SUCCESS) ? UNC_CS_APPLIED : UNC_CS_NOT_APPLIED;
  ppe_val = reinterpret_cast<val_policingprofile_entry_ctrl_t *>
    (GetVal(ctrlr_key));
  if (ppe_val == NULL) return UPLL_RC_ERR_GENERIC;
  cs_status = (val_ppe_main->cs_row_status);

  UPLL_LOG_TRACE("cs_status %d ctrlr_status %d\n", cs_status, ctrlr_status);
  if (op == UNC_OP_CREATE) {
    ppe_val->cs_row_status = ctrlr_status;
    if (val_ppe_main->cs_row_status == UNC_CS_UNKNOWN) {
      /* first entry in ctrlr table */
      cs_status = ctrlr_status;
    } else if (val_ppe_main->cs_row_status == UNC_CS_APPLIED) {
      if (ctrlr_status == UNC_CS_NOT_APPLIED) {
        cs_status = UNC_CS_PARTIALLY_APPLIED;
      }

    } else if (val_ppe_main->cs_row_status == UNC_CS_NOT_APPLIED) {
      if (ctrlr_status == UNC_CS_APPLIED) {
        cs_status =  UNC_CS_PARTIALLY_APPLIED;
      }
    } else if (val_ppe_main->cs_row_status == UNC_CS_INVALID) {
      cs_status = UNC_CS_INVALID;
    } else {
      cs_status = UNC_CS_PARTIALLY_APPLIED;
    }
    val_ppe_main->cs_row_status = cs_status;
  }
  // Updating the Controller cs_row_status
  val_policingprofile_entry_ctrl *run_ctrlr_val =
    reinterpret_cast<val_policingprofile_entry_ctrl_t *>
    (GetVal(upd_key));
  if ((op == UNC_OP_UPDATE) && (upd_key != NULL)) {
    if (run_ctrlr_val != NULL)
      ppe_val->cs_row_status = run_ctrlr_val->cs_row_status;
  }

  for (unsigned int loop = 0; loop < sizeof(val_ppe_main->valid)/
      sizeof(val_ppe_main->valid[0]); ++loop) {
    if (val_ppe_main->valid[loop] != UNC_VF_INVALID) {
      if (ppe_val->cs_attr[loop] != UNC_CS_NOT_SUPPORTED)
        ppe_val->cs_attr[loop] = ctrlr_status;
      else
        ppe_val->cs_attr[loop] = UNC_CS_NOT_SUPPORTED;

      if (op == UNC_OP_CREATE) {
        if (val_ppe_main->cs_attr[loop] == UNC_CS_INVALID) {
          cs_status = UNC_CS_INVALID;
        }
        if (val_ppe_main->cs_attr[loop] == ctrlr_status) {
          cs_status = ctrlr_status;
        } else if (ctrlr_status == UNC_CS_APPLIED) {
          if (val_ppe_main->cs_attr[loop] == UNC_CS_UNKNOWN) {
            cs_status = ctrlr_status;
          } else if (val_ppe_main->cs_attr[loop] == UNC_CS_NOT_APPLIED) {
            val_ppe_main->cs_attr[loop] = UNC_CS_PARTIALLY_APPLIED;
          } else {
            cs_status = val_ppe_main->cs_attr[loop];
          }
        } else if (ctrlr_status == UNC_CS_NOT_APPLIED) {
          if (val_ppe_main->cs_attr[loop] == UNC_CS_NOT_APPLIED) {
            cs_status =  UNC_CS_NOT_APPLIED;
          }
        } else {
          cs_status =  UNC_CS_PARTIALLY_APPLIED;
        }
        val_ppe_main->cs_attr[loop]  = cs_status;
        UPLL_LOG_DEBUG("Main tbl cs_attr : %d", val_ppe_main->cs_attr[loop]);
      }
    }
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t PolicingProfileEntryMoMgr::Get_Tx_Consolidated_Status(
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

upll_rc_t PolicingProfileEntryMoMgr::SetPPEntryConsolidatedStatus(
    ConfigKeyVal *ikey,
    uint8_t *ctrlr_id,
    DalDmlIntf *dmi)  {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *ctrlr_ckv = NULL;
  val_policingprofile_entry_ctrl_t *ctrlr_val = NULL;
  uint8_t *pp_exist_on_ctrlr = NULL;
  bool applied = false, not_applied = false, invalid = false;
  unc_keytype_configstatus_t c_status = UNC_CS_NOT_APPLIED;
  string vtn_name = "";

  DbSubOp dbop = { kOpReadMultiple, kOpMatchNone,
                   kOpInOutCtrlr | kOpInOutCs };
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
    ctrlr_val = reinterpret_cast<val_policingprofile_entry_ctrl_t *>
                (GetVal(tmp));
    if (!ctrlr_val) {
      UPLL_LOG_DEBUG("Controller Value is empty");
      tmp = NULL;
      DELETE_IF_NOT_NULL(ctrlr_ckv);
      return UPLL_RC_ERR_GENERIC;
    }
    GET_USER_DATA_CTRLR(tmp, pp_exist_on_ctrlr);
    if (!strcmp(reinterpret_cast<char *>(pp_exist_on_ctrlr),
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
        break;
        // return UPLL_RC_ERR_GENERIC;
    }
    pp_exist_on_ctrlr = NULL;
  }
  UPLL_LOG_DEBUG("PPE - applied %d not_applied %d", applied, not_applied);
  if (invalid) {
    c_status = UNC_CS_INVALID;
  }
  if (applied && !not_applied) {
    c_status = UNC_CS_APPLIED;
  } else if (!applied && not_applied) {
    c_status = UNC_CS_NOT_APPLIED;
  } else if (applied && not_applied) {
    c_status = UNC_CS_PARTIALLY_APPLIED;
  } else {
    c_status = UNC_CS_APPLIED;
  }
  // Set cs_status
  val_policingprofile_entry_t *val = static_cast
      <val_policingprofile_entry_t *>(GetVal(ikey));
  val->cs_row_status = c_status;
  for (unsigned int loop = 0; loop <
       sizeof(val->valid)/sizeof(val->valid[0]);
       ++loop) {
    for (ConfigKeyVal *tmp = ctrlr_ckv; tmp != NULL;
                     tmp = tmp->get_next_cfg_key_val()) {
      ctrlr_val =
          reinterpret_cast<val_policingprofile_entry_ctrl_t *>(GetVal(tmp));

      GET_USER_DATA_CTRLR(tmp, pp_exist_on_ctrlr);
      UPLL_LOG_DEBUG("Controller name in DB %s", pp_exist_on_ctrlr);
      if (!strcmp(reinterpret_cast<char *>(pp_exist_on_ctrlr),
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
    val->cs_attr[loop] = c_status;
    applied = not_applied =false;
  }

  DbSubOp dbop_update = {kOpNotRead, kOpMatchNone, kOpInOutCs};
  result_code = UpdateConfigDB(ikey, UPLL_DT_RUNNING, UNC_OP_UPDATE, dmi,
                               &dbop_update, TC_CONFIG_GLOBAL, vtn_name,
                               MAINTBL);
  DELETE_IF_NOT_NULL(ctrlr_ckv);
  return result_code;
}

bool PolicingProfileEntryMoMgr::IsAllAttrInvalid(
        val_policingprofile_entry_t *val) {
  for ( unsigned int loop = 0;
      loop < sizeof(val->valid)/sizeof(val->valid[0]); ++loop ) {
    if (UNC_VF_INVALID != val->valid[loop])
      return false;
  }
  return true;
}

upll_rc_t PolicingProfileEntryMoMgr::GetOperation(uuc::UpdateCtrlrPhase phase,
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

upll_rc_t PolicingProfileEntryMoMgr::GetDomainsForController(
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

bool PolicingProfileEntryMoMgr::IsAttributeUpdated(void *val1, void *val2) {
  UPLL_FUNC_TRACE;
  val_policingprofile_entry_t *val_pp_entry1 =
    reinterpret_cast<val_policingprofile_entry_t *>(val1);
  val_policingprofile_entry_t *val_pp_entry2 =
    reinterpret_cast<val_policingprofile_entry_t *>(val2);

  for ( unsigned int loop = 0; loop < (sizeof(val_pp_entry1->valid)
                         /(sizeof(val_pp_entry1->valid[0])));
          ++loop ) {
      if ((UNC_VF_VALID_NO_VALUE == val_pp_entry1->valid[loop] &&
                  UNC_VF_VALID == val_pp_entry2->valid[loop]) ||
          (UNC_VF_VALID == val_pp_entry1->valid[loop] &&
                  UNC_VF_INVALID == val_pp_entry2->valid[loop]))
        return true;
  }

  if (val_pp_entry1->valid[UPLL_IDX_FLOWLIST_PPE] == (UNC_VF_VALID)
      && val_pp_entry2->valid[UPLL_IDX_FLOWLIST_PPE] == UNC_VF_VALID) {
    if (strncmp(reinterpret_cast<char *>(val_pp_entry1->flowlist),
               reinterpret_cast<char *>(val_pp_entry2->flowlist),
               (kMaxLenFlowListName + 1)))
      return true;
  }
  if (val_pp_entry1->valid[UPLL_IDX_RATE_PPE] == UNC_VF_VALID
      && val_pp_entry2->valid[UPLL_IDX_RATE_PPE] == UNC_VF_VALID) {
    if (val_pp_entry1->rate != val_pp_entry2->rate) {
      return true;
    }
  }
  if (val_pp_entry1->valid[UPLL_IDX_CIR_PPE] == UNC_VF_VALID
      && val_pp_entry2->valid[UPLL_IDX_CIR_PPE] == UNC_VF_VALID) {
    if (val_pp_entry1->cir != val_pp_entry2->cir)
      return true;
  }
  if (val_pp_entry1->valid[UPLL_IDX_CBS_PPE] == UNC_VF_VALID
      && val_pp_entry2->valid[UPLL_IDX_CBS_PPE] == UNC_VF_VALID) {
    if (val_pp_entry1->cbs != val_pp_entry2->cbs)
      return true;
  }
  if (val_pp_entry1->valid[UPLL_IDX_PIR_PPE] == UNC_VF_VALID
      && val_pp_entry2->valid[UPLL_IDX_PIR_PPE] == UNC_VF_VALID) {
    if (val_pp_entry1->pir != val_pp_entry2->pir)
      return true;
  }
  if (val_pp_entry1->valid[UPLL_IDX_PBS_PPE] == UNC_VF_VALID
      && val_pp_entry2->valid[UPLL_IDX_PBS_PPE] == UNC_VF_VALID) {
    if (val_pp_entry1->pbs != val_pp_entry2->pbs)
      return true;
  }
  if (val_pp_entry1->valid[UPLL_IDX_GREEN_ACTION_PPE] == UNC_VF_VALID
      && val_pp_entry2->valid[UPLL_IDX_GREEN_ACTION_PPE] == UNC_VF_VALID) {
    if (val_pp_entry1->green_action != val_pp_entry2->green_action)
      return true;
  }
  if (val_pp_entry1->valid[UPLL_IDX_GREEN_PRIORITY_PPE] == UNC_VF_VALID
      && val_pp_entry2->valid[UPLL_IDX_GREEN_PRIORITY_PPE] == UNC_VF_VALID) {
    if (val_pp_entry1->green_action_priority !=
        val_pp_entry2->green_action_priority)
      return true;
  }
  if (val_pp_entry1->valid[UPLL_IDX_GREEN_DSCP_PPE] == UNC_VF_VALID
      && val_pp_entry2->valid[UPLL_IDX_GREEN_DSCP_PPE] == UNC_VF_VALID) {
    if (val_pp_entry1->green_action_dscp != val_pp_entry2->green_action_dscp)
      return true;
  }
  if (val_pp_entry1->valid[UPLL_IDX_GREEN_DROP_PPE] == UNC_VF_VALID
      && val_pp_entry2->valid[UPLL_IDX_GREEN_DROP_PPE] == UNC_VF_VALID) {
    if (val_pp_entry1->green_action_drop_precedence !=
        val_pp_entry2->green_action_drop_precedence)
      return true;
  }
  if (val_pp_entry1->valid[UPLL_IDX_YELLOW_ACTION_PPE] == UNC_VF_VALID
      && val_pp_entry2->valid[UPLL_IDX_YELLOW_ACTION_PPE] == UNC_VF_VALID) {
    if (val_pp_entry1->yellow_action != val_pp_entry2->yellow_action)
      return true;
  }
  if (val_pp_entry1->valid[UPLL_IDX_YELLOW_PRIORITY_PPE] == UNC_VF_VALID
      && val_pp_entry2->valid[UPLL_IDX_YELLOW_PRIORITY_PPE] == UNC_VF_VALID) {
    if (val_pp_entry1->yellow_action_priority !=
        val_pp_entry2->yellow_action_priority)
      return true;
  }
  if (val_pp_entry1->valid[UPLL_IDX_YELLOW_DSCP_PPE] == UNC_VF_VALID
      && val_pp_entry2->valid[UPLL_IDX_YELLOW_DSCP_PPE] == UNC_VF_VALID) {
    if (val_pp_entry1->yellow_action_dscp != val_pp_entry2->yellow_action_dscp)
      return true;
  }
  if (val_pp_entry1->valid[UPLL_IDX_YELLOW_DROP_PPE] == UNC_VF_VALID
      && val_pp_entry2->valid[UPLL_IDX_YELLOW_DROP_PPE] == UNC_VF_VALID) {
    if (val_pp_entry1->yellow_action_drop_precedence !=
        val_pp_entry2->yellow_action_drop_precedence)
      return true;
  }
  if (val_pp_entry1->valid[UPLL_IDX_RED_ACTION_PPE] == UNC_VF_VALID
      && val_pp_entry2->valid[UPLL_IDX_RED_ACTION_PPE] == UNC_VF_VALID) {
    if (val_pp_entry1->red_action != val_pp_entry2->red_action)
      return true;
  }
  if (val_pp_entry1->valid[UPLL_IDX_RED_PRIORITY_PPE] == UNC_VF_VALID
      && val_pp_entry2->valid[UPLL_IDX_RED_PRIORITY_PPE] == UNC_VF_VALID) {
    if (val_pp_entry1->red_action_priority !=
        val_pp_entry2->red_action_priority)
      return true;
  }
  if (val_pp_entry1->valid[UPLL_IDX_RED_DSCP_PPE] == UNC_VF_VALID
      && val_pp_entry2->valid[UPLL_IDX_RED_DSCP_PPE] == UNC_VF_VALID) {
    if (val_pp_entry1->red_action_dscp !=
        val_pp_entry2->red_action_dscp)
      return true;
  }
  if (val_pp_entry1->valid[UPLL_IDX_RED_DROP_PPE] == UNC_VF_VALID
      && val_pp_entry2->valid[UPLL_IDX_RED_DROP_PPE] == UNC_VF_VALID) {
    if (val_pp_entry1->red_action_drop_precedence !=
        val_pp_entry2->red_action_drop_precedence)
      return true;
  }
  return false;
}

upll_rc_t PolicingProfileEntryMoMgr::IsFlowListMatched(
    const char* flowlist_name, DalDmlIntf *dmi,
    TcConfigMode config_mode, string vtn_name) {

  UPLL_FUNC_TRACE;

  upll_rc_t result_code = UPLL_RC_ERR_GENERIC;

  DalBindInfo *db_info = new DalBindInfo(uudst::kDbiPolicingProfileEntryTbl);

  ConfigKeyVal *ckv = NULL;
  result_code = GetChildConfigKey(ckv, NULL);
  if (UPLL_RC_SUCCESS != result_code) {
    DELETE_IF_NOT_NULL(db_info);
    return result_code;
  }
  val_policingprofile_entry_t *ppe_val =
      reinterpret_cast<val_policingprofile_entry_t*>
      (ConfigKeyVal::Malloc(sizeof(val_policingprofile_entry_t)));
  uuu::upll_strncpy(ppe_val->flowlist, flowlist_name,
      (kMaxLenFlowListName + 1));
  ckv->AppendCfgVal(IpctSt::kIpcStValPolicingprofileEntry, ppe_val);


  void *tkey = ckv->get_key();

  //  Bind Output policingprofile_name
  db_info->BindOutput(uudst::policingprofile_entry::kDbiPolicingProfileName,
                      uud::kDalChar,
                      (kMaxLenPolicingProfileName + 1),
                      reinterpret_cast<void *>(
                          reinterpret_cast<key_policingprofile_entry_t*>
                          (tkey)->policingprofile_key.policingprofile_name));

  db_info->BindMatch(uudst::policingprofile_entry::kDbiFlowlist,
                     uud::kDalChar,
                     (kMaxLenFlowListName + 1),
                     reinterpret_cast<void *>(
                         (ppe_val)->flowlist));

  std::string query_string = QUERY_READ_DISTINCT_PPE_MAIN_TBL;

  DalCursor *dal_cursor_handle = NULL;
  result_code = DalToUpllResCode(
      dmi->ExecuteAppQueryMultipleRecords(query_string, 0,
                                          db_info, &dal_cursor_handle));
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Reading multiple records failed %d", result_code);
    DELETE_IF_NOT_NULL(ckv);
    DELETE_IF_NOT_NULL(db_info);
    dmi->CloseCursor(dal_cursor_handle, false);
    return result_code;
  }
  while (UPLL_RC_SUCCESS == DalToUpllResCode(dmi->GetNextRecord
                                             (dal_cursor_handle))) {
    PolicingProfileMoMgr *ppmgr = static_cast<PolicingProfileMoMgr*>
        ((const_cast<MoManager*>(GetMoManager(UNC_KT_POLICING_PROFILE))));
    if (NULL == ppmgr) {
      UPLL_LOG_DEBUG("Policingprofile instance is NULL");
      DELETE_IF_NOT_NULL(db_info);
      DELETE_IF_NOT_NULL(ckv);
      dmi->CloseCursor(dal_cursor_handle, false);
      return UPLL_RC_ERR_GENERIC;
    }
    result_code = ppmgr->RefCountSemanticCheck(reinterpret_cast<const char *>
                                               (reinterpret_cast
                                               <key_policingprofile_entry_t*>
                                               (tkey)->policingprofile_key.
                                               policingprofile_name),
                                               dmi, config_mode, vtn_name);
    if (UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
      UPLL_LOG_DEBUG("RefCountSemanticCheck returned %d", result_code);
      DELETE_IF_NOT_NULL(db_info);
      DELETE_IF_NOT_NULL(ckv);
      dmi->CloseCursor(dal_cursor_handle, false);
      return result_code;
    }
  }
  if (dal_cursor_handle)
    dmi->CloseCursor(dal_cursor_handle, false);
  DELETE_IF_NOT_NULL(db_info);
  DELETE_IF_NOT_NULL(ckv);
  return result_code;
}

upll_rc_t PolicingProfileEntryMoMgr::ClearVirtualKtDirtyInGlobal(
                                         DalDmlIntf *dmi) {
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

upll_rc_t PolicingProfileEntryMoMgr::UpdateRefCountInFl(
    const char *policingprofile_name, DalDmlIntf *dmi,
    upll_keytype_datatype_t dt_type, unc_keytype_operation_t op,
    TcConfigMode config_mode, string vtn_name) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  ConfigKeyVal *ppe_ckv = NULL;
  result_code = GetChildConfigKey(ppe_ckv, NULL);
  if (result_code != UPLL_RC_SUCCESS) {
    return result_code;
  }

  key_policingprofile_entry_t *key_policingprofie =
    reinterpret_cast<key_policingprofile_entry_t *>(ppe_ckv->get_key());
  uuu::upll_strncpy(
      key_policingprofie->policingprofile_key.policingprofile_name,
      policingprofile_name,
      (kMaxLenPolicingProfileName + 1));

  DbSubOp dbop = { kOpReadMultiple, kOpMatchNone, kOpInOutFlag | kOpInOutCs };

  result_code = ReadConfigDB(ppe_ckv, dt_type, UNC_OP_READ, dbop, dmi, MAINTBL);
  if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
    UPLL_LOG_DEBUG("No entry in policingprofile entry maintbl");
    DELETE_IF_NOT_NULL(ppe_ckv);
    return UPLL_RC_SUCCESS;
  }
  uint8_t *c_id = NULL;
  ConfigKeyVal *tmp_ppe_ckv = ppe_ckv;
  while (tmp_ppe_ckv) {
    val_policingprofile_entry_t *temp_ppe_val = reinterpret_cast
        <val_policingprofile_entry_t *>(GetVal(tmp_ppe_ckv));
    if (NULL == temp_ppe_val) {
      UPLL_LOG_DEBUG("PolicingprofileEntry val is Null");
      DELETE_IF_NOT_NULL(ppe_ckv);
      return UPLL_RC_ERR_GENERIC;
    }
    if (UNC_VF_VALID == temp_ppe_val->valid[UPLL_IDX_FLOWLIST_PPE ]) {
      FlowListMoMgr *flowlist_mgr = reinterpret_cast<FlowListMoMgr *>
        (const_cast<MoManager *> (GetMoManager(UNC_KT_FLOWLIST)));
      if (NULL == flowlist_mgr) {
        UPLL_LOG_DEBUG("flowlist_mgr is NULL");
        DELETE_IF_NOT_NULL(ppe_ckv);
        return UPLL_RC_ERR_GENERIC;
      }
      ConfigKeyVal *ctrlr_ppe_ckv = NULL;
      result_code = GetChildConfigKey(ctrlr_ppe_ckv, tmp_ppe_ckv);
      if (UPLL_RC_SUCCESS != result_code) {
        UPLL_LOG_DEBUG("GetChildConfigKey failed %d", result_code);
        DELETE_IF_NOT_NULL(ppe_ckv);
        return result_code;
      }
      DbSubOp dbop = { kOpReadMultiple, kOpMatchNone, kOpInOutCtrlr };
      result_code = ReadConfigDB(ctrlr_ppe_ckv, dt_type, UNC_OP_READ, dbop,
                                 dmi, CTRLRTBL);
      if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
        UPLL_LOG_DEBUG("No entry in policingprofile entry maintbl");
        DELETE_IF_NOT_NULL(ctrlr_ppe_ckv);
        tmp_ppe_ckv = tmp_ppe_ckv->get_next_cfg_key_val();
        continue;
      } else if (UPLL_RC_SUCCESS != result_code) {
        UPLL_LOG_DEBUG("No entry in policingprofile entry maintbl");
        DELETE_IF_NOT_NULL(ppe_ckv);
        DELETE_IF_NOT_NULL(ctrlr_ppe_ckv);
        return result_code;
      }
      ConfigKeyVal *ctrlr_temp_ckv = ctrlr_ppe_ckv;
      while (ctrlr_temp_ckv) {
        GET_USER_DATA_CTRLR(ctrlr_temp_ckv, c_id);
        result_code = flowlist_mgr->DeleteFlowListToController(
              reinterpret_cast<char *>(temp_ppe_val->flowlist), dmi,
              reinterpret_cast<char *>(c_id), dt_type, UNC_OP_DELETE,
              config_mode, vtn_name, true, 1);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("AddFlowListToController failed err code(%d)",
                         result_code)
          DELETE_IF_NOT_NULL(ppe_ckv);
          DELETE_IF_NOT_NULL(ctrlr_ppe_ckv);
          return result_code;
        }
        ctrlr_temp_ckv = ctrlr_temp_ckv->get_next_cfg_key_val();
      }
      DELETE_IF_NOT_NULL(ctrlr_ppe_ckv);
    }
    tmp_ppe_ckv = tmp_ppe_ckv->get_next_cfg_key_val();
  }
  DELETE_IF_NOT_NULL(ppe_ckv);
  return UPLL_RC_SUCCESS;
}

upll_rc_t PolicingProfileEntryMoMgr::DeleteChildrenPOM(ConfigKeyVal *ikey,
                      upll_keytype_datatype_t dt_type, DalDmlIntf *dmi,
                      TcConfigMode config_mode, string vtn_name) {
  UPLL_FUNC_TRACE;
  ConfigKeyVal *ckv = NULL;
  upll_rc_t result_code = GetPolicingprofileKeyVal(ckv, ikey);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetPolicingprofileKeyVal failed %d", result_code);
    return result_code;
  }
  void *tkey = ckv->get_key();
  DalBindInfo *db_info = new DalBindInfo(uudst::kDbiPpScratchTbl);
  db_info->BindMatch(uudst::pp_scratch::kDbiPolicingProfileName,
                     uud::kDalChar, (kMaxLenPolicingProfileName + 1),
                     reinterpret_cast<void *>(reinterpret_cast
                     <key_policingprofile_entry_t*>(tkey)->
                     policingprofile_key.policingprofile_name));
  int st_ref_count = 0;
  db_info->BindOutput(uudst::pp_scratch::kDbiRefCount,
                      uud::kDalUint32,
                      1,
                      &st_ref_count);

  std::string query_string = QUERY_SUM_PP_SCRATCH_REF_COUNT_WITH_PP;
  result_code = DalToUpllResCode(
      dmi->ExecuteAppQuerySingleRecord(query_string, db_info));
  if ((result_code != UPLL_RC_SUCCESS) &&
      (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE)) {
    UPLL_LOG_DEBUG("Reading scratch tbl failed %d", result_code);
    DELETE_IF_NOT_NULL(db_info);
    DELETE_IF_NOT_NULL(ckv);
    return result_code;
  }
  DELETE_IF_NOT_NULL(db_info);
  if (UPLL_RC_SUCCESS == result_code) {
    key_policingprofile_entry_t *ppe_key = reinterpret_cast<
        key_policingprofile_entry_t *>(ikey->get_key());
    result_code = UpdateRefCountInFl(reinterpret_cast<char *>(
        ppe_key->policingprofile_key.policingprofile_name),
        dmi, UPLL_DT_CANDIDATE, UNC_OP_DELETE, config_mode, vtn_name);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("UpdateRefCountInFl failed %d", result_code);
      DELETE_IF_NOT_NULL(ckv);
      return result_code;
    }
  }

  for (int i = 0; i < ntable; i++) {
    if (GetTable((MoMgrTables)i, UPLL_DT_CANDIDATE) >= uudst::kDalNumTables) {
      continue;
    }
    // skip the deletion for convert table, it is deleted as part of vbr_portmap
    // delete
    DbSubOp dbop = {kOpNotRead, kOpMatchNone, kOpInOutNone};
    result_code = UpdateConfigDB(ikey, UPLL_DT_CANDIDATE,
             UNC_OP_DELETE, dmi, &dbop, config_mode,
             vtn_name, (MoMgrTables)i);
    result_code = (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE)?
                  UPLL_RC_SUCCESS:result_code;
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_ERROR("Delete Operation fails with %d", result_code);
      DELETE_IF_NOT_NULL(ckv);
      return result_code;
    }
  }
  DELETE_IF_NOT_NULL(ckv);
  return UPLL_RC_SUCCESS;
}
}  // namespace kt_momgr
}  // namespace upll
}  // namespace unc
