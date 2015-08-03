/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <sstream>
#include "pfc/log.h"
#include "flowlist_momgr.hh"
#include "flowlist_entry_momgr.hh"
#include "policingprofile_momgr.hh"
#include "policingprofile_entry_momgr.hh"
#include "vtn_flowfilter_entry_momgr.hh"
#include "unc/upll_errno.h"
#include "upll_validation.hh"
#include "unc/upll_ipc_enum.h"
#include "ctrlr_capa_defines.hh"
#include "uncxx/upll_log.hh"
#include "upll_db_query.hh"

namespace unc {
namespace upll {
namespace kt_momgr {

#define  NUM_KEY_MAIN_COL    3
#define  NUM_KEY_CTRL_COL    3
#define  NUM_KEY_RENAME_COL  2
#define  FLOWLIST_RENAME 0x01
#define  NO_FLOWLIST_RENAME 0xFE

// FlowList Table(Main Table)
BindInfo FlowListMoMgr::flowlist_bind_info[] = {
  {uudst::flowlist::kDbiFlowListName,
    CFG_KEY,
    offsetof(key_flowlist_t, flowlist_name),
    uud::kDalChar, (kMaxLenFlowListName + 1)},
  {uudst::flowlist::kDbiIpType,
    CFG_VAL,
    offsetof(val_flowlist_t, ip_type),
    uud::kDalUint8, 1},
  {uudst::flowlist::kDbiFlags,
    CK_VAL,
    offsetof(key_user_data_t, flags),
    uud::kDalUint8, 1},
  {uudst::flowlist::kDbiValidIpType,
    CFG_META_VAL,
    offsetof(val_flowlist_t, valid[0]),
    uud::kDalUint8, 1},
  {uudst::flowlist::kDbiCsRowStatus,
    CS_VAL,
    offsetof(val_flowlist_t, cs_row_status),
    uud::kDalUint8, 1},
  {uudst::flowlist::kDbiCsIpType,
    CS_VAL,
    offsetof(val_flowlist_t, cs_attr[0]),
    uud::kDalUint8, 1}};

// FlowList Rename Table
BindInfo FlowListMoMgr::flowlist_rename_bind_info[] = {
  {uudst::flowlist_rename::kDbiFlowListName,
    CFG_KEY,
    offsetof(key_flowlist_t, flowlist_name),
    uud::kDalChar, (kMaxLenFlowListName+1)},
  {uudst::flowlist_rename::kDbiCtrlrName,
    CK_VAL,
    offsetof(key_user_data_t, ctrlr_id),
    uud::kDalChar, (kMaxLenCtrlrId + 1)},
  {uudst::flowlist_rename::kDbiFlowListNameCtrlr,
    CFG_VAL,
    offsetof(val_rename_flowlist_t, flowlist_newname),
    uud::kDalChar, (kMaxLenFlowListName + 1)} };

// FLowListController Table
BindInfo FlowListMoMgr::flowlist_controller_bind_info[] = {
  {uudst::flowlist_ctrlr::kDbiFlowListName,
    CFG_KEY,
    offsetof(key_flowlist_t, flowlist_name),
    uud::kDalChar, (kMaxLenFlowListName + 1)},
  {uudst::flowlist_ctrlr::kDbiCtrlrName,
    CK_VAL,
    offsetof(key_user_data_t, ctrlr_id),
    uud::kDalChar, (kMaxLenCtrlrId + 1)},
  {uudst::flowlist_ctrlr::kDbiRefCount,
    CFG_VAL,
    offsetof(val_flowlist_ctrl, refcount),
    uud::kDalUint32, 1},
  {uudst::flowlist_ctrlr::kDbiFlags,
    CK_VAL,
    offsetof(key_user_data_t, flags),
    uud::kDalUint8, 1},
  {uudst::flowlist_ctrlr::kDbiValidIpType,
    CFG_META_VAL,
    offsetof(val_flowlist_ctrl, valid[0]),
    uud::kDalUint8, 1},
  {uudst::flowlist_ctrlr::kDbiCsRowStatus,
    CS_VAL,
    offsetof(val_flowlist_ctrl, cs_row_status),
    uud::kDalUint8, 1},
  {uudst::flowlist_ctrlr::kDbiCsIpType,
    CS_VAL,
    offsetof(val_flowlist_ctrl, cs_attr[0]),
    uud::kDalUint8, 1} };

BindInfo FlowListMoMgr::rename_flowlist_main_tbl[] = {
  { uudst::flowlist::kDbiFlowListName,
    CFG_MATCH_KEY,
    offsetof(key_flowlist_t, flowlist_name),
    uud::kDalChar,
    (kMaxLenFlowListName + 1) },
  { uudst::flowlist::kDbiFlowListName,
    CFG_INPUT_KEY,
    offsetof(key_rename_vnode_info_t, new_flowlist_name),
    uud::kDalChar,
    (kMaxLenFlowListName + 1) },
  { uudst::flowlist::kDbiFlags,
    CK_VAL,
    offsetof(key_user_data_t, flags),
    uud::kDalUint8, 1}
};

BindInfo FlowListMoMgr::rename_flowlist_ctrlr_tbl[] = {
  { uudst::flowlist_ctrlr::kDbiFlowListName,
    CFG_MATCH_KEY,
    offsetof(key_flowlist_t, flowlist_name),
    uud::kDalChar,
    (kMaxLenFlowListName + 1) },
  { uudst::flowlist_ctrlr::kDbiFlowListName,
    CFG_INPUT_KEY,
    offsetof(key_rename_vnode_info_t, new_flowlist_name),
    uud::kDalChar,
    (kMaxLenFlowListName + 1) },
  { uudst::flowlist_ctrlr::kDbiFlags,
    CK_VAL,
    offsetof(key_user_data_t, flags),
    uud::kDalUint8, 1}
};

BindInfo FlowListMoMgr::rename_flowlist_rename_tbl[] = {
  {uudst::flowlist_rename::kDbiFlowListName, CFG_MATCH_KEY,
    offsetof(key_flowlist_t, flowlist_name),
    uud::kDalChar, (kMaxLenFlowListName + 1)},
  {uudst::flowlist_rename::kDbiFlowListName, CFG_INPUT_KEY,
    offsetof(key_rename_vnode_info_t, new_flowlist_name),
    uud::kDalChar, (kMaxLenFlowListName + 1)}
};

unc_key_type_t FlowListMoMgr::flowlist_child[] = { UNC_KT_FLOWLIST_ENTRY };

/**
 *  @brief constructor
 */
FlowListMoMgr::FlowListMoMgr():MoMgrImpl() {
  UPLL_FUNC_TRACE;
  ntable = MAX_MOMGR_TBLS;
  table = new Table *[ntable]();

  /*Construct the Flowlist  Main Table*/
  table[MAINTBL] = new Table(uudst::kDbiFlowListTbl,
      UNC_KT_FLOWLIST, flowlist_bind_info,
      IpctSt::kIpcStKeyFlowlist, IpctSt::kIpcStValFlowlist,
      uudst::flowlist::kDbiFlowListNumCols);

  /*Construct Flowlist RenameTable*/
  table[RENAMETBL] = new Table(uudst::kDbiFlowListRenameTbl, UNC_KT_FLOWLIST,
      flowlist_rename_bind_info, IpctSt::kIpcStKeyFlowlist,
      IpctSt::kIpcStValRenameFlowlist,
      uudst::flowlist_rename::kDbiFlowListRenameNumCols);

  /*Construct FlowList Controller Table*/
  table[CTRLRTBL] = new Table(uudst::kDbiFlowListCtrlrTbl, UNC_KT_FLOWLIST,
      flowlist_controller_bind_info, IpctSt::kIpcStKeyFlowlist,
      IpctSt::kIpcInvalidStNum,
      uudst::flowlist_ctrlr::kDbiFlowListCtrlrNumCols);

  table[CONVERTTBL] = NULL;

  nchild = sizeof(flowlist_child) / sizeof(flowlist_child[0]);
  child = flowlist_child;
}

bool FlowListMoMgr::GetRenameKeyBindInfo(unc_key_type_t key_type,
    BindInfo *&binfo, int &nattr, MoMgrTables tbl) {
  switch (key_type) {
    case UNC_KT_FLOWLIST:
      if (MAINTBL == tbl) {
        nattr = NUM_KEY_MAIN_COL;
        binfo = rename_flowlist_main_tbl;
      } else if (CTRLRTBL == tbl) {
        nattr = NUM_KEY_CTRL_COL;
        binfo = rename_flowlist_ctrlr_tbl;
      } else {
        nattr = NUM_KEY_RENAME_COL;
        binfo = rename_flowlist_rename_tbl;
      }
    default:
      break;
  }
  return PFC_TRUE;
}

upll_rc_t FlowListMoMgr::GetValid(void*val,
                                  uint64_t indx,
                                  uint8_t *&valid,
                                  upll_keytype_datatype_t dt_type,
                                  MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  if (val == NULL) return UPLL_RC_ERR_GENERIC;

  if (tbl == MAINTBL) {
    valid = NULL;
    switch (indx) {
      case uudst::flowlist::kDbiIpType:
        valid = &(reinterpret_cast<val_flowlist_t*>
                (val))->valid[UPLL_IDX_IP_TYPE_FL];
        break;
      default:
         return UPLL_RC_ERR_GENERIC;
    }
    #if 0
    if (indx == uudst::flowlist::kDbiIpType) {
      valid = &(reinterpret_cast<val_flowlist_t*>
                (val))->valid[UPLL_IDX_IP_TYPE_FL];
    } else {
       valid = NULL;
    }
    #endif
  } else if (tbl == RENAMETBL) {
    valid = NULL;
    switch (indx) {
      case uudst::flowlist_rename::kDbiFlowListNameCtrlr:
         valid = &(reinterpret_cast<val_rename_flowlist *>
                 (val))->valid[UPLL_IDX_RENAME_FLOWLIST_RFL];
         break;
      default:
         return UPLL_RC_ERR_GENERIC;
    }
    #if 0
    if (indx == uudst::flowlist_rename::kDbiFlowListNameCtrlr) {
      valid = &(reinterpret_cast<val_rename_flowlist *>
                 (val))->valid[UPLL_IDX_RENAME_FLOWLIST_RFL];
    } else {
      valid = NULL;
    }
    #endif
  } else if (tbl == CTRLRTBL) {
    #if 0
    if (indx == uudst::flowlist_ctrlr::kDbiValidIpType) {
      valid = &(reinterpret_cast<val_flowlist_ctrl*>(val))->valid[0];
    } else if (indx ==uudst::flowlist_ctrlr::kDbiRefCount) {
      valid = &(reinterpret_cast<val_flowlist_ctrl*>(val))->valid[1];
    } else {
      valid = NULL;
    }
    #endif
    valid = NULL;
    switch (indx) {
      case uudst::flowlist_ctrlr::kDbiValidIpType:
        valid = &(reinterpret_cast<val_flowlist_ctrl*>(val))->valid[0];
        break;
      case uudst::flowlist_ctrlr::kDbiRefCount:
        valid = &(reinterpret_cast<val_flowlist_ctrl*>(val))->valid[1];
      default:
         break;
    }
  } else {
    valid = NULL;
    return UPLL_RC_ERR_GENERIC;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t FlowListMoMgr::AllocVal(ConfigVal *&ck_val,
                                  upll_keytype_datatype_t dt_type,
                                  MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  void *val = NULL;
  if (ck_val != NULL) return UPLL_RC_ERR_GENERIC;
  switch (tbl) {
    case MAINTBL:
      val = ConfigKeyVal::Malloc(sizeof(val_flowlist_t));
      ck_val = new ConfigVal(IpctSt::kIpcStValFlowlist, val);
      break;
    case RENAMETBL:
      val = ConfigKeyVal::Malloc(sizeof(val_rename_flowlist_t));
      ck_val = new ConfigVal(IpctSt::kIpcStValRenameFlowlist, val);
      break;
    case CTRLRTBL:
      val = ConfigKeyVal::Malloc(sizeof(val_flowlist_ctrl_t));
      ck_val = new ConfigVal(IpctSt::kIpcInvalidStNum, val);
      break;
    default:
      val = NULL;
      break;
  }
  if (val == NULL) {
    UPLL_LOG_DEBUG("Memory Allocation Failure");
    return UPLL_RC_ERR_GENERIC;
  }
  return UPLL_RC_SUCCESS;
}


upll_rc_t FlowListMoMgr::ValidateAttribute(ConfigKeyVal *ikey,
                                           DalDmlIntf *dmi,
                                           IpcReqRespHeader *req) {
  UPLL_FUNC_TRACE;
  // No specific semantic check required for Create/Update operations
  if (ikey->get_key_type() != UNC_KT_FLOWLIST)
    return UPLL_RC_ERR_GENERIC;
  else
    return UPLL_RC_SUCCESS;
}

upll_rc_t FlowListMoMgr::GetChildConfigKey(ConfigKeyVal *&okey,
                                           ConfigKeyVal *parent_key) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  void *pkey = NULL;
  key_flowlist_t *flowlist_key = NULL;

  // If parent key is null then allocate the new configkey val and return
  if (parent_key == NULL) {
    flowlist_key = reinterpret_cast<key_flowlist_t *>
      (ConfigKeyVal::Malloc(sizeof(key_flowlist_t)));

    okey = new ConfigKeyVal(UNC_KT_FLOWLIST, IpctSt::kIpcStKeyFlowlist,
                            flowlist_key, NULL);
    return UPLL_RC_SUCCESS;
  } else {
    pkey = parent_key->get_key();
  }

  if (!pkey) {
    UPLL_LOG_DEBUG("pkey NULL");
    return UPLL_RC_ERR_GENERIC;
  }

  if (okey) {
    if (okey->get_key_type() != UNC_KT_FLOWLIST)
      return UPLL_RC_ERR_GENERIC;
  }
  if ((okey) && (okey->get_key())) {
    flowlist_key = reinterpret_cast<key_flowlist_t *>(okey->get_key());
  } else {
    flowlist_key = reinterpret_cast<key_flowlist_t *>
        (ConfigKeyVal::Malloc(sizeof(key_flowlist_t)));
  }
  switch (parent_key->get_key_type()) {
    case UNC_KT_ROOT:
      break;
    case UNC_KT_FLOWLIST:
      uuu::upll_strncpy(flowlist_key->flowlist_name,
                        (reinterpret_cast<key_flowlist_t *>
                         (pkey)->flowlist_name),
                        (kMaxLenFlowListName + 1));
      break;
    default:
      if (flowlist_key) free(flowlist_key);
      return UPLL_RC_ERR_GENERIC;
  }
  // Allocate and assign the out configkeyval
  if ((okey) && !(okey->get_key())) {
    UPLL_LOG_DEBUG("okey not null and flow list name updated");
    okey->SetKey(IpctSt::kIpcStKeyFlowlist, flowlist_key);
  }
  if (!okey) {
    okey = new ConfigKeyVal(UNC_KT_FLOWLIST, IpctSt::kIpcStKeyFlowlist,
                            flowlist_key, NULL);
  }
  SET_USER_DATA(okey, parent_key);
  UPLL_LOG_TRACE("%s GetChildConfigKey fl start",
                  okey->ToStrAll().c_str());
  return result_code;
}

upll_rc_t FlowListMoMgr::GetRenamedUncKey(ConfigKeyVal *ctrlr_key,
                                          upll_keytype_datatype_t dt_type,
                                          DalDmlIntf *dmi, uint8_t *ctrlr_id) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code;
  ConfigKeyVal *unc_key = NULL;
  UPLL_LOG_TRACE("%s GetRenamedUncKey fl start",
                  ctrlr_key->ToStrAll().c_str());
  key_flowlist_t *ctrlr_flowlist_key =
                  reinterpret_cast<key_flowlist_t *>(ctrlr_key->get_key());
  if (NULL == ctrlr_flowlist_key) return UPLL_RC_ERR_GENERIC;

  DbSubOp dbop = { kOpReadSingle, kOpMatchCtrlr, kOpInOutNone };

  val_rename_flowlist_t *rename_flowlist =
  reinterpret_cast<val_rename_flowlist_t*>
  (ConfigKeyVal::Malloc(sizeof(val_rename_flowlist_t)));
  if (!rename_flowlist) {
    UPLL_LOG_DEBUG("rename_flowlist NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  uuu::upll_strncpy(rename_flowlist->flowlist_newname,
               ctrlr_flowlist_key->flowlist_name,
               (kMaxLenFlowListName + 1));
  rename_flowlist->valid[UPLL_IDX_RENAME_FLOWLIST_RFL] = UNC_VF_VALID;

  result_code = GetChildConfigKey(unc_key, NULL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetChildConfigKey failure %d", result_code);
    free(rename_flowlist);
    return result_code;
  }

  if (!unc_key) {
    UPLL_LOG_DEBUG("unc_key NULL");
    free(rename_flowlist);
    return UPLL_RC_ERR_GENERIC;
  }

  unc_key->AppendCfgVal(IpctSt::kIpcStValRenameFlowlist, rename_flowlist);

  UPLL_LOG_DEBUG("ctrlr_id fl (%s)", ctrlr_id);
  if (ctrlr_id) {
    SET_USER_DATA_CTRLR(unc_key, ctrlr_id);
  } else {
    dbop.matchop = kOpMatchNone;
  }
  dbop.inoutop = kOpInOutCtrlr;
  result_code = ReadConfigDB(unc_key, dt_type, UNC_OP_READ, dbop, dmi,
                             RENAMETBL);

  if (result_code == UPLL_RC_SUCCESS) {
    key_flowlist_t *flowlist_key =
    reinterpret_cast<key_flowlist_t *>(unc_key->get_key());
    uuu::upll_strncpy(ctrlr_flowlist_key->flowlist_name,
                 flowlist_key->flowlist_name,
                 (kMaxLenFlowListName + 1));
    SET_USER_DATA(ctrlr_key, unc_key);
    SET_USER_DATA_FLAGS(ctrlr_key, FL_RENAME);
  }
  UPLL_LOG_TRACE("%s GetRenamedUncKey fl end",
                  ctrlr_key->ToStrAll().c_str());
  DELETE_IF_NOT_NULL(unc_key);
  return result_code;
}

upll_rc_t FlowListMoMgr::GetRenamedControllerKey(
    ConfigKeyVal *ikey, upll_keytype_datatype_t dt_type, DalDmlIntf *dmi,
    controller_domain *ctrlr_dom) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_ERR_GENERIC;
  ConfigKeyVal *okey = NULL;

  result_code = GetChildConfigKey(okey, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetChildConfigKey fail (%d)", result_code);
    return result_code;
  }

  if (ctrlr_dom != NULL) {
    SET_USER_DATA_CTRLR_DOMAIN(okey, *ctrlr_dom);
  } else {
    UPLL_LOG_ERROR("Controller id is null");
    DELETE_IF_NOT_NULL(okey);
    return UPLL_RC_ERR_GENERIC;
  }

  UPLL_LOG_DEBUG("ctrlr : %s; domain : %s", ctrlr_dom->ctrlr,
      ctrlr_dom->domain);

  DbSubOp dbop = { kOpReadSingle, kOpMatchCtrlr, kOpInOutFlag };

  result_code = ReadConfigDB(okey, dt_type, UNC_OP_READ, dbop, dmi, RENAMETBL);
  if (result_code != UPLL_RC_SUCCESS) {
    if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
      UPLL_LOG_DEBUG("ReadConfigDB no instance");
      DELETE_IF_NOT_NULL(okey);
      return UPLL_RC_SUCCESS;
    }
    DELETE_IF_NOT_NULL(okey);
    return result_code;
  }

  val_rename_flowlist_t *rename_val =
    reinterpret_cast<val_rename_flowlist_t *>(GetVal(okey));
  if (!rename_val) {
    DELETE_IF_NOT_NULL(okey);
    return UPLL_RC_ERR_GENERIC;
  }

  key_flowlist *key = reinterpret_cast<key_flowlist *>(ikey->get_key());
  uuu::upll_strncpy(key->flowlist_name,
      rename_val->flowlist_newname,
      (kMaxLenFlowListName + 1));
  DELETE_IF_NOT_NULL(okey);

  return UPLL_RC_SUCCESS;
}


upll_rc_t FlowListMoMgr::UpdateMo(IpcReqRespHeader *req, ConfigKeyVal *ikey,
                                  DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  TcConfigMode config_mode = TC_CONFIG_INVALID;
  std::string vtn_name = "";
  upll_rc_t result_code = GetConfigModeInfo(req, config_mode, vtn_name);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetConfigMode failed");
    return result_code;
  }

  if (config_mode == TC_CONFIG_VIRTUAL) {
    UPLL_LOG_INFO("VIRTUAL Mode flowlist_entry UPDATE");
    return UPLL_RC_ERR_NOT_ALLOWED_AT_THIS_TIME;
  }
  UPLL_LOG_DEBUG("Update operation is not allowed for KT_FLOWLIST");
  return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_KT;
}

upll_rc_t FlowListMoMgr::SetFlowListConsolidatedStatus(ConfigKeyVal *ikey,
                                                       uint8_t *ctrlr_id,
                                                       DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *ctrlr_ckv = NULL;
  val_flowlist_ctrl *ctrlr_val = NULL;
  uint8_t *flowlist_exist_on_ctrlr = NULL;
  bool applied = false, not_applied = false, invalid = false;
  unc_keytype_configstatus_t c_status = UNC_CS_NOT_APPLIED;
  DbSubOp dbop = { kOpReadMultiple, kOpMatchNone,
                   kOpInOutCtrlr | kOpInOutDomain | kOpInOutCs };
  if (!ikey || !dmi || !ctrlr_id) {
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
    ctrlr_val = reinterpret_cast<val_flowlist_ctrl *>(GetVal(tmp));
    if (!ctrlr_val) {
      UPLL_LOG_DEBUG("Controller Value is empty");
      tmp = NULL;
      DELETE_IF_NOT_NULL(ctrlr_ckv);
      return UPLL_RC_ERR_GENERIC;
    }
    GET_USER_DATA_CTRLR(tmp, flowlist_exist_on_ctrlr);
    if (!strcmp(reinterpret_cast<char *>(flowlist_exist_on_ctrlr),
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
      break;  // Addressed Coverity MISSING_BREAK
      default:
        UPLL_LOG_DEBUG("Invalid status");
        DELETE_IF_NOT_NULL(ctrlr_ckv);
        break;
        // return UPLL_RC_ERR_GENERIC;
    }
    flowlist_exist_on_ctrlr = NULL;
  }
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
  val_flowlist_t *flowlist_val = static_cast<val_flowlist_t *>(GetVal(ikey));
  flowlist_val->cs_row_status = c_status;
  flowlist_val->cs_attr[0] = c_status;
  std::string vtn_name = "";
  DbSubOp dbop_update = {kOpNotRead, kOpMatchNone, kOpInOutCs};
  result_code = UpdateConfigDB(ikey, UPLL_DT_RUNNING, UNC_OP_UPDATE, dmi,
                               &dbop_update, TC_CONFIG_GLOBAL, vtn_name,
                               MAINTBL);
  DELETE_IF_NOT_NULL(ctrlr_ckv);
  return result_code;
}
upll_rc_t FlowListMoMgr::ReadRecord(IpcReqRespHeader *req, ConfigKeyVal *ikey,
                                    DalDmlIntf *dmi) {
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
      // TODO(author) The below check is not required
      // as its done in ValidateMessage()
      if (req->operation != UNC_OP_READ_SIBLING_COUNT) {
        result_code = GetRenamedControllerKey(ikey, req->datatype, dmi, NULL);
        result_code = ReadConfigDB(ikey, req->datatype, req->operation,
                                   dbop, dmi, RENAMETBL);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG(" ReadConfigDB failed:-%d", result_code);
          return result_code;
        }
      }
      break;
    default:
      // TODO(author) Log a message
      return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT;
      break;
  }  // end of switch
  UPLL_LOG_DEBUG("Read Record Successfull");
  return UPLL_RC_SUCCESS;
}

upll_rc_t FlowListMoMgr::ReadMo(IpcReqRespHeader *req, ConfigKeyVal *ikey,
                                DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  controller_domain ctrlr_dom;

  result_code = ValidateMessage(req, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Validate Message Failed %d", result_code);
    return result_code;
  }
  result_code = ReadInfoFromDB(req, ikey, dmi, &ctrlr_dom);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG(" Read request failed result(%d)", result_code);
  }
  return result_code;
}

upll_rc_t FlowListMoMgr::ReadSiblingMo(IpcReqRespHeader *req,
                                       ConfigKeyVal *ikey,
                                       bool begin, DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  controller_domain ctrlr_dom;

  result_code = ValidateMessage(req, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Validate Message Failed %d", result_code);
    return result_code;
  }
  result_code = ReadInfoFromDB(req, ikey, dmi, &ctrlr_dom);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG(" Read sibling request failed result(%d)", result_code);
  }
  return result_code;
}

upll_rc_t FlowListMoMgr::MergeValidate(unc_key_type_t keytype,
                                       const char *ctrl_id,
                                       ConfigKeyVal *configkey,
                                       DalDmlIntf *dmi,
                                       upll_import_type import_type) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_ERR_GENERIC;
  // DbSubOp dbop = NULL;

  ConfigKeyVal *ckey = NULL;
  ConfigKeyVal *temp_ckey = NULL;

  unc_keytype_operation_t op[] = { UNC_OP_UPDATE };
  int nop = sizeof(op)/ sizeof(op[0]);

  if (ctrl_id == NULL) return result_code;
  // Read the Configuration from the IMPORT
  result_code = GetChildConfigKey(temp_ckey, NULL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetChildConfigKey ckval NULL");
    return result_code;
  }

  DbSubOp dbop1 = { kOpReadMultiple, kOpMatchNone, kOpInOutNone };
  result_code = ReadConfigDB(temp_ckey, UPLL_DT_IMPORT, UNC_OP_READ, dbop1, dmi,
                             MAINTBL);
  if (UPLL_RC_SUCCESS != result_code &&
      UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
    UPLL_LOG_DEBUG("Unable to read the ImportConfiguration");
    DELETE_IF_NOT_NULL(temp_ckey);
    return result_code;
  }
  if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
     DELETE_IF_NOT_NULL(temp_ckey);
     return UPLL_RC_SUCCESS;
  }
  ckey = temp_ckey;
  while (ckey != NULL) {
    UPLL_LOG_DEBUG("ckey not null");

    // Check the flow list as stand alone
    DbSubOp dbop = { kOpReadExist, kOpMatchNone, kOpInOutNone };
    // ckey has rename val set, so removing  that to read from ctrlr tbl
    ckey->SetCfgVal(NULL);
    result_code = UpdateConfigDB(ckey, UPLL_DT_IMPORT, UNC_OP_READ, dmi, &dbop,
                                 CTRLRTBL);
    if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      // if import-mode is "ignore"
      if (UNC_IMPORT_IGNORE_MODE == GetImportErrorBehaviour()) {
        DbSubOp dbDeleteOp = { kOpNotRead, kOpMatchNone, kOpInOutNone };
        string temp_vtn_name = "";

        // Its necessary to delete flowlist's childnode first before
        // deleting Flowlist parent node.

        // Get Flowlist entry MoMgr
        FlowListEntryMoMgr *flEntry_mgr =
          reinterpret_cast<FlowListEntryMoMgr*>(const_cast<MoManager *>
                                       (GetMoManager(UNC_KT_FLOWLIST_ENTRY)));
        if (NULL == flEntry_mgr) {
            DELETE_IF_NOT_NULL(temp_ckey);
            UPLL_LOG_ERROR("mgr failed");
            return UPLL_RC_ERR_GENERIC;
        }

        // Get FLE ConfigKey
        ConfigKeyVal *temp_flEtryckey = NULL;
        result_code = flEntry_mgr->GetChildConfigKey(temp_flEtryckey, ckey);
        if (UPLL_RC_SUCCESS != result_code) {
          DELETE_IF_NOT_NULL(temp_ckey);
          UPLL_LOG_ERROR("FLE GetChildConfigKey ckval NULL");
          return result_code;
        }

        // Delete FlowlistEntry
        result_code = flEntry_mgr->UpdateConfigDB(temp_flEtryckey,
                         UPLL_DT_IMPORT, UNC_OP_DELETE, dmi, &dbDeleteOp,
                         TC_CONFIG_GLOBAL, temp_vtn_name, MAINTBL);
        if ((UPLL_RC_SUCCESS != result_code) &&
           (UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code)) {
            DELETE_IF_NOT_NULL(temp_ckey);
            DELETE_IF_NOT_NULL(temp_flEtryckey);
            UPLL_LOG_ERROR("Failed to delete stand-alone flowlistEntry");
            return result_code;
        }

        // Delete FlowListEntry ConfigKey
        DELETE_IF_NOT_NULL(temp_flEtryckey);

        // Delete standalone flow list
        result_code = UpdateConfigDB(ckey, UPLL_DT_IMPORT, UNC_OP_DELETE, dmi,
                         &dbDeleteOp, TC_CONFIG_GLOBAL, temp_vtn_name, MAINTBL);
        if (UPLL_RC_SUCCESS != result_code) {
          DELETE_IF_NOT_NULL(temp_ckey);
          UPLL_LOG_ERROR("Failed to delete stand-alone flowlist");
          return result_code;
        }
      } else {
        UPLL_LOG_ERROR("flow list is stand alone");
        result_code = GetChildConfigKey(configkey, ckey);
        DELETE_IF_NOT_NULL(temp_ckey);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_ERROR("DupConfigKeyVal fail");
          return result_code;
        }
        return UPLL_RC_ERR_MERGE_CONFLICT;
      }
    } else if (result_code != UPLL_RC_ERR_INSTANCE_EXISTS) {
      UPLL_LOG_DEBUG("Database Error");
      DELETE_IF_NOT_NULL(temp_ckey);
      return result_code;
    }
    ckey = ckey->get_next_cfg_key_val();
  }
  DELETE_IF_NOT_NULL(temp_ckey);

  result_code = ValidateImportWithRunning(keytype, ctrl_id, configkey,
                                          op, nop, dmi);
  if ((result_code != UPLL_RC_SUCCESS) &&
      (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE)) {
     UPLL_LOG_INFO("ValidateImportWithRunning db err (%d)", result_code);
     return result_code;
  }
  return UPLL_RC_SUCCESS;
}
upll_rc_t FlowListMoMgr::IsReferenced(IpcReqRespHeader *req,
                                      ConfigKeyVal *ikey,
                                      DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  key_flowlist_t *fl_key = reinterpret_cast<key_flowlist_t *>
      (ikey->get_key());
  if (UPLL_DT_CANDIDATE == req->datatype) {
    TcConfigMode config_mode = TC_CONFIG_INVALID;
    std::string vtn_name = "";
    result_code = GetConfigModeInfo(req, config_mode, vtn_name);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("GetConfigMode failed");
      return result_code;
    }

    result_code = RefCountSemanticCheck(reinterpret_cast<const char *>
                                        (fl_key->flowlist_name), dmi,
                                        config_mode, vtn_name);
    if (UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
      UPLL_LOG_DEBUG("RefCountSemanticCheck failed %d", result_code);
      return result_code;
    }
    PolicingProfileEntryMoMgr *mgr =
      reinterpret_cast<PolicingProfileEntryMoMgr *>(const_cast<MoManager*>
              (GetMoManager(UNC_KT_POLICING_PROFILE_ENTRY)));
    if (NULL == mgr) {
      return UPLL_RC_ERR_GENERIC;
    }
    result_code = mgr->IsFlowlistConfigured(reinterpret_cast<const char *>
        (fl_key->flowlist_name), req->datatype, dmi);
    if (UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
      if (UPLL_RC_SUCCESS == result_code)
        result_code = UPLL_RC_ERR_CFG_SEMANTIC;
      UPLL_LOG_INFO("IsFlowlistConfigured failed %d", result_code);
      return result_code;
    }
  } else {
    ConfigKeyVal *tmp_ckv = NULL;
    result_code = GetChildConfigKey(tmp_ckv, ikey);
    if (UPLL_RC_SUCCESS != result_code) {
      return result_code;
    }
    DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutNone };
    result_code = ReadConfigDB(tmp_ckv, req->datatype, UNC_OP_READ,
                               dbop, dmi, CTRLRTBL);
    if (UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
      if (UPLL_RC_SUCCESS == result_code)
        result_code = UPLL_RC_ERR_CFG_SEMANTIC;
      UPLL_LOG_DEBUG("IsRef failed %d", result_code);
      if (tmp_ckv) delete tmp_ckv;
      return result_code;
    }
    if (tmp_ckv) delete tmp_ckv;
  }
  VtnFlowFilterEntryMoMgr *vtn_mgr =
    reinterpret_cast<VtnFlowFilterEntryMoMgr *>(const_cast<MoManager*>
            (GetMoManager(UNC_KT_VTN_FLOWFILTER_ENTRY)));
  if (NULL == vtn_mgr) {
    return UPLL_RC_ERR_GENERIC;
  }
  result_code = vtn_mgr->IsFlowListConfigured(reinterpret_cast<const char *>
      (fl_key->flowlist_name), req->datatype, dmi);
  if (UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
    if (UPLL_RC_SUCCESS == result_code)
      result_code = UPLL_RC_ERR_CFG_SEMANTIC;
    UPLL_LOG_INFO("IsFlowListConfigured failed %d", result_code);
    return result_code;
  }

  return UPLL_RC_SUCCESS;
}

upll_rc_t FlowListMoMgr::SwapKeyVal(ConfigKeyVal *ikey, ConfigKeyVal *&okey,
                                    DalDmlIntf *dmi, uint8_t *ctrlr,
                                    bool &no_rename) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  // ConfigVal *tmp1;
  if (okey != NULL) {
    UPLL_LOG_DEBUG("Output ConfigKey should be NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  if (!strlen(reinterpret_cast<const char*>(ctrlr))) {
    UPLL_LOG_DEBUG("Controller ID is NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  if (ikey == NULL) {
    UPLL_LOG_DEBUG("Input ConfigKey Cannot be null");
    return UPLL_RC_ERR_BAD_REQUEST;
  }
  if (ikey->get_key_type() != UNC_KT_FLOWLIST) {
    UPLL_LOG_DEBUG("INVALID Key Type");
    return UPLL_RC_ERR_BAD_REQUEST;
  }
  ConfigVal *cfg_val = ikey->get_cfg_val();
  if (cfg_val == NULL) {
    UPLL_LOG_DEBUG("Rename Val configkey is NULL");
    return UPLL_RC_ERR_BAD_REQUEST;
  }
  val_rename_flowlist_t *tval =
      reinterpret_cast<val_rename_flowlist_t *>(cfg_val->get_val());
  if (!tval) {
      UPLL_LOG_DEBUG("Val is NULL");
      return UPLL_RC_ERR_GENERIC;
  }
  key_flowlist_t *key_flowlist = reinterpret_cast<key_flowlist_t*>
      (ConfigKeyVal::Malloc(sizeof(key_flowlist_t)));
  /* no rename*/
  if (UNC_VF_VALID_NO_VALUE == tval->valid[UPLL_IDX_RENAME_FLOWLIST_RFL]) {
    no_rename = true;
    uuu::upll_strncpy(key_flowlist->flowlist_name,
           reinterpret_cast<key_flowlist_t*>(ikey->get_key())->flowlist_name,
           (kMaxLenFlowListName + 1));

  } else if (tval->valid[UPLL_IDX_RENAME_FLOWLIST_RFL] == UNC_VF_VALID) {
    // checking the string is empty or not
    if (!strlen(reinterpret_cast<char *>(tval->flowlist_newname))) {
      free(key_flowlist);
      return UPLL_RC_ERR_GENERIC;
    }
    uuu::upll_strncpy(key_flowlist->flowlist_name,
           tval->flowlist_newname,
           (kMaxLenFlowListName + 1));
    /* The New Name and PFC name should not be same name */
    if (!strcmp(reinterpret_cast<char *>
       (reinterpret_cast<key_flowlist_t *>(ikey->get_key())->flowlist_name),
              reinterpret_cast<char *>(tval->flowlist_newname))) {
      free(key_flowlist);
      return UPLL_RC_ERR_GENERIC;
    }
  }

  okey = new ConfigKeyVal(UNC_KT_FLOWLIST, IpctSt::kIpcStKeyFlowlist,
                          key_flowlist, NULL);
  if (NULL == okey) {
    UPLL_LOG_DEBUG("Memory allocation failure for ConfigKeyVal structure");
    FREE_IF_NOT_NULL(key_flowlist);
    return UPLL_RC_ERR_GENERIC;
  }
  return result_code;
}

upll_rc_t FlowListMoMgr::DupConfigKeyVal(ConfigKeyVal *&okey,
                                         ConfigKeyVal *&req, MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  if (req == NULL) return UPLL_RC_ERR_GENERIC;
  if (okey != NULL) return UPLL_RC_ERR_GENERIC;
  if (req->get_key_type() != UNC_KT_FLOWLIST) return UPLL_RC_ERR_GENERIC;

  ConfigVal *tmp1 = NULL, *tmp = (req)->get_cfg_val();

  if (tmp) {
    if (tbl == MAINTBL) {
      val_flowlist_t *ival = reinterpret_cast<val_flowlist_t *>(GetVal(req));
      if (NULL != ival) {
        val_flowlist_t *flowlist_val = reinterpret_cast<val_flowlist_t*>
          (ConfigKeyVal::Malloc(sizeof(val_flowlist_t)));
        memcpy(flowlist_val, ival, sizeof(val_flowlist_t));
        tmp1 = new ConfigVal(IpctSt::kIpcStValFlowlist, flowlist_val);
        if (!tmp1) {
          FREE_IF_NOT_NULL(flowlist_val);
          return UPLL_RC_ERR_GENERIC;
        }
      }
    } else if (tbl == RENAMETBL) {
      val_rename_flowlist_t *ival =
          reinterpret_cast<val_rename_flowlist_t *>(GetVal(req));
      if (NULL != ival) {
        val_rename_flowlist_t *rename_val =
          reinterpret_cast<val_rename_flowlist_t*>
          (ConfigKeyVal::Malloc(sizeof(val_rename_flowlist_t)));
        memcpy(rename_val, ival, sizeof(val_rename_flowlist_t));
        tmp1 = new ConfigVal(IpctSt::kIpcStValRenameFlowlist, rename_val);
        if (!tmp1) {
          FREE_IF_NOT_NULL(rename_val);
          return UPLL_RC_ERR_GENERIC;
        }
      }
    } else if (tbl == CTRLRTBL) {
      val_flowlist_ctrl *ival =
          reinterpret_cast<val_flowlist_ctrl *>(GetVal(req));
      if (NULL != ival) {
        val_flowlist_ctrl *flowlist_ctrlr_val =
            reinterpret_cast<val_flowlist_ctrl *>(ConfigKeyVal::Malloc
                                                  (sizeof(val_flowlist_ctrl)));
        memcpy(flowlist_ctrlr_val, ival, sizeof(val_flowlist_ctrl));
        tmp1 = new ConfigVal(IpctSt::kIpcStValFlowlist, flowlist_ctrlr_val);
        if (!tmp1) {
          FREE_IF_NOT_NULL(flowlist_ctrlr_val);
          return UPLL_RC_ERR_GENERIC;
        }
      }
    }
    if (tmp1) tmp1->set_user_data(tmp->get_user_data());
    // tmp = tmp->get_next_cfg_val();
  }

  void *tkey = (req != NULL) ? (req)->get_key() : NULL;
  if (tkey) {
    key_flowlist_t *ikey = reinterpret_cast<key_flowlist_t *>(tkey);
    key_flowlist_t *flowlist_key = reinterpret_cast<key_flowlist_t*>
      (ConfigKeyVal::Malloc(sizeof(key_flowlist_t)));
    memcpy(flowlist_key, ikey, sizeof(key_flowlist_t));
    okey = new ConfigKeyVal(UNC_KT_FLOWLIST, IpctSt::kIpcStKeyFlowlist,
                          flowlist_key, tmp1);
  }
  if (okey) {
    SET_USER_DATA(okey, req)
  } else {
    delete tmp1;
    return UPLL_RC_ERR_GENERIC;
  }

  return UPLL_RC_SUCCESS;
}

upll_rc_t FlowListMoMgr::UpdateConfigStatus(ConfigKeyVal *flowlist_key,
                                            unc_keytype_operation_t op,
                                            uint32_t driver_result,
                                            ConfigKeyVal *nreq,
                                            DalDmlIntf *dmi,
                                            ConfigKeyVal *ctrlr_key) {
  UPLL_FUNC_TRACE;
  unc_keytype_configstatus_t  ctrlr_status;
  uint8_t cs_status;
  ctrlr_status = (driver_result == 0) ? UNC_CS_APPLIED : UNC_CS_NOT_APPLIED;

  if ((NULL == flowlist_key) || (NULL == ctrlr_key)) {
    UPLL_LOG_DEBUG("input struct is NULL");
    return UPLL_RC_ERR_GENERIC;
  }

  val_flowlist_t *val_flowlist =
                 reinterpret_cast<val_flowlist_t *>(GetVal(flowlist_key));
  val_flowlist_ctrl *ctrlr_val_flowlist =
      reinterpret_cast<val_flowlist_ctrl *>(GetVal(ctrlr_key));
  if ((val_flowlist == NULL) || (ctrlr_val_flowlist == NULL)) {
    UPLL_LOG_DEBUG("Value structure is empty!!");
    return UPLL_RC_ERR_GENERIC;
  }
  cs_status = val_flowlist->cs_row_status;
  UPLL_LOG_TRACE("cs_status %d ctrlr_status %d\n", cs_status, ctrlr_status);

  if (op == UNC_OP_CREATE) {
    ctrlr_val_flowlist->cs_row_status = ctrlr_status;
    if (val_flowlist->cs_row_status == UNC_CS_INVALID) {
      cs_status = UNC_CS_INVALID;
    } else if (val_flowlist->cs_row_status == UNC_CS_UNKNOWN) {
       /* first entry in ctrlr table */
       cs_status = ctrlr_status;
    } else if (val_flowlist->cs_row_status == UNC_CS_APPLIED) {
      if (ctrlr_status == UNC_CS_NOT_APPLIED) {
        cs_status = UNC_CS_PARTIALLY_APPLIED;
      }
    } else if (val_flowlist->cs_row_status == UNC_CS_NOT_APPLIED) {
      if (ctrlr_status == UNC_CS_APPLIED) {
        cs_status =  UNC_CS_PARTIALLY_APPLIED;
      }
    } else {
       cs_status = UNC_CS_PARTIALLY_APPLIED;
    }
    val_flowlist->cs_row_status = cs_status;
    val_flowlist->cs_attr[0]  = cs_status;
  }
  // Updating the Controller cs_row_status
  if ((op == UNC_OP_UPDATE) && (nreq != NULL)) {
      val_flowlist_ctrl *run_ctrlr_val = reinterpret_cast<val_flowlist_ctrl *>
                                                     (GetVal(nreq));
      if (run_ctrlr_val != NULL)
       ctrlr_val_flowlist->cs_row_status = run_ctrlr_val->cs_row_status;
  }
  if (val_flowlist->valid[0] != UNC_VF_INVALID) {
    if (ctrlr_val_flowlist->cs_attr[0] != UNC_CS_NOT_SUPPORTED)
      ctrlr_val_flowlist->cs_attr[0] = ctrlr_status;
    else
      ctrlr_val_flowlist->cs_attr[0] = UNC_CS_NOT_SUPPORTED;

    if (val_flowlist->cs_attr[0] == ctrlr_status) {
      cs_status = ctrlr_status;
    } else if (val_flowlist->cs_attr[0] == UNC_CS_INVALID) {
      cs_status = UNC_CS_INVALID;
    } else if (ctrlr_status == UNC_CS_APPLIED) {
      if (val_flowlist->cs_attr[0] == UNC_CS_UNKNOWN) {
        cs_status = ctrlr_status;
      }
      if (val_flowlist->cs_attr[0] == UNC_CS_NOT_APPLIED) {
        cs_status = UNC_CS_PARTIALLY_APPLIED;
      } else {
        cs_status = val_flowlist->cs_attr[0];
      }
    } else if (ctrlr_status == UNC_CS_NOT_APPLIED) {
       if (val_flowlist->cs_attr[0] == UNC_CS_NOT_APPLIED) {
         cs_status =  UNC_CS_NOT_APPLIED;
       } else {
         cs_status =  UNC_CS_PARTIALLY_APPLIED;
       }
    }
    val_flowlist->cs_attr[0]  = cs_status;
    UPLL_LOG_DEBUG("Main tbl cs_attr : %d", val_flowlist->cs_attr[0]);
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t FlowListMoMgr::AddFlowListToController(
    char *flowlist_name,
    DalDmlIntf *dmi,
    char* ctrl_id,
    upll_keytype_datatype_t dt_type,
    unc_keytype_operation_t op,
    TcConfigMode config_mode,
    string vtn_name, bool is_commit) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  if (UNC_OP_CREATE == op || UNC_OP_UPDATE == op) {
    result_code = CreateFlowListToController(flowlist_name, dmi,
                                             ctrl_id, dt_type, op,
                                             config_mode, vtn_name, is_commit,
                                             1);
  } else if (UNC_OP_DELETE == op) {
    result_code = DeleteFlowListToController(flowlist_name, dmi,
                                             ctrl_id, dt_type, op,
                                             config_mode, vtn_name, is_commit,
                                             1);
  }
  return result_code;
}

upll_rc_t FlowListMoMgr::DeleteFlowListToController(char *flowlist_name,
    DalDmlIntf *dmi,
    char* ctrl_id,
    upll_keytype_datatype_t dt_type,
    unc_keytype_operation_t op,
    TcConfigMode config_mode,
    string vtn_name, bool is_commit, int count) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *okey = NULL;
  bool fl_entry_del = false;
  result_code = GetChildConfigKey(okey, NULL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetChildConfigKey failed %d", result_code);
    return result_code;
  }
  key_flowlist_t *okey_key = reinterpret_cast<key_flowlist_t *>
    (okey->get_key());
  uuu::upll_strncpy(okey_key->flowlist_name,
      flowlist_name,
      (kMaxLenFlowListName+1));
  SET_USER_DATA_CTRLR(okey, ctrl_id);

  if ((UPLL_DT_CANDIDATE == dt_type) &&
      (false == is_commit)) {
    result_code = UpdateRefCountInScratchTbl(okey, dmi, dt_type, UNC_OP_DELETE,
                                             config_mode, vtn_name, count);
    if (UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
      UPLL_LOG_DEBUG("UpdateRefCountInScratchTbl returned %d", result_code);
      DELETE_IF_NOT_NULL(okey);
      return result_code;
    }
    result_code = InsertRecInScratchTbl(okey, dmi, dt_type, UNC_OP_DELETE,
                                        config_mode, vtn_name, count);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("InsertRecInScratchTbl failed %d", result_code);
      DELETE_IF_NOT_NULL(okey);
      return result_code;
    }
  } else {
    DbSubOp dbop = {kOpReadSingle, kOpMatchCtrlr, kOpInOutNone};
    result_code = ReadConfigDB(okey,
                               dt_type,
                               UNC_OP_READ,
                               dbop, dmi, CTRLRTBL);
    if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
      UPLL_LOG_DEBUG("No matching record found in Ctrlrtbl");
      DELETE_IF_NOT_NULL(okey);
      return UPLL_RC_SUCCESS;
    } else if (UPLL_RC_SUCCESS == result_code) {
      UPLL_LOG_DEBUG("Matching records found in ctrlrtbl");
      val_flowlist_ctrl_t *ctrlr_val = reinterpret_cast
          <val_flowlist_ctrl_t *>(GetVal(okey));
      ctrlr_val->refcount -= 1;
      UPLL_LOG_DEBUG("Ref_count in flowlist %d", ctrlr_val->refcount);
      if (1 > ctrlr_val->refcount) {
        ctrlr_val->valid[1] = UNC_VF_VALID;
        result_code = UpdateConfigDB(okey, dt_type, UNC_OP_DELETE, dmi,
                                     config_mode, vtn_name, CTRLRTBL);
        if (UPLL_RC_SUCCESS != result_code) {
          UPLL_LOG_DEBUG("Delete from ctrlrtbl failed");
          DELETE_IF_NOT_NULL(okey);
          return result_code;
        }
        fl_entry_del  = true;
        // Renametbl entry should be deleted when no entry in flowlist ctrlr tbl
        result_code = UpdateConfigDB(okey, dt_type, UNC_OP_DELETE,
                                     dmi, &dbop, config_mode,
                                     vtn_name, RENAMETBL);
        if (UPLL_RC_SUCCESS != result_code &&
            UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
          UPLL_LOG_TRACE("UpdateConfigDB Failed %d", result_code);
          DELETE_IF_NOT_NULL(okey);
          return result_code;
        }
        result_code = (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code)?
            UPLL_RC_SUCCESS:result_code;
      } else {
        ctrlr_val->valid[1] = UNC_VF_VALID;
        result_code = UpdateConfigDB(okey, dt_type, UNC_OP_UPDATE, dmi,
                                     config_mode, vtn_name, CTRLRTBL);
        if (UPLL_RC_SUCCESS != result_code) {
          UPLL_LOG_DEBUG("Update in ctrlrtbl failed");
          DELETE_IF_NOT_NULL(okey);
          return result_code;
        }
      }
    } else {
      UPLL_LOG_DEBUG("ReadConfigDB failed in Ctrlrtbl");
      DELETE_IF_NOT_NULL(okey);
      return result_code;
    }
    DELETE_IF_NOT_NULL(okey);
    if (fl_entry_del) {
      FlowListEntryMoMgr *mgr = reinterpret_cast<FlowListEntryMoMgr *>
          (const_cast<MoManager *> (GetMoManager(UNC_KT_FLOWLIST_ENTRY)));
      result_code = mgr->AddFlowListToController(
          flowlist_name, dmi, ctrl_id, dt_type, op, config_mode, vtn_name);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_INFO("FLE controller table update failed:%d", result_code);
        return result_code;
      }
    }
  }
  DELETE_IF_NOT_NULL(okey);
  return UPLL_RC_SUCCESS;
}

upll_rc_t FlowListMoMgr::CreateFlowListToController(
    char *flowlist_name,
    DalDmlIntf *dmi,
    char* ctrl_id,
    upll_keytype_datatype_t dt_type,
    unc_keytype_operation_t op,
    TcConfigMode config_mode,
    string vtn_name, bool is_commit,
    int count) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *okey = NULL;
  ConfigKeyVal *rename_key = NULL;
  bool flag_chk = false;
  result_code = GetChildConfigKey(okey, NULL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetChildConfigKey failed %d", result_code);
    return result_code;
  }
  unc_keytype_ctrtype_t ctrlrtype = UNC_CT_UNKNOWN;
  uuc::CtrlrMgr *ctrlr_mgr = uuc::CtrlrMgr::GetInstance();

  key_flowlist_t *okey_key = reinterpret_cast<key_flowlist_t *>
      (okey->get_key());
  uuu::upll_strncpy(okey_key->flowlist_name,
        flowlist_name,
        (kMaxLenFlowListName+1));
  SET_USER_DATA_CTRLR(okey, ctrl_id);
  if ((UPLL_DT_CANDIDATE == dt_type) &&
      (false == is_commit)) {
    result_code = UpdateRefCountInScratchTbl(okey, dmi, dt_type, UNC_OP_CREATE,
                                             config_mode, vtn_name, count);
    if (UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
      UPLL_LOG_DEBUG("UpdateRefCountInScratchTbl restuned %d", result_code);
      DELETE_IF_NOT_NULL(okey);
      return result_code;
    }
    if (UPLL_DT_CANDIDATE == dt_type) {
      result_code = InsertRecInScratchTbl(okey, dmi, dt_type, UNC_OP_CREATE,
                                          config_mode, vtn_name, count);
      if (UPLL_RC_SUCCESS != result_code) {
        UPLL_LOG_DEBUG("InsertRecInScratchTbl failed %d", result_code);
        DELETE_IF_NOT_NULL(okey);
        return result_code;
      }
    }
    DELETE_IF_NOT_NULL(okey);
    return UPLL_RC_SUCCESS;
  } else if ((UPLL_DT_CANDIDATE == dt_type) &&
             (true == is_commit)) {
    result_code = UPLL_RC_ERR_NO_SUCH_INSTANCE;
  } else {
    //  Update Ref count in flowlist ctrlr tbl directly.
    result_code = UpdateRefCountInCtrlrTbl(okey, dmi, dt_type, config_mode,
                                           vtn_name);
    UPLL_LOG_DEBUG("UpdateRefCountInCtrlrTbl returns %d", result_code);
    //  If result_code is not UPLL_RC_ERR_NO_SUCH_INSTANCE
    //  then return result_code
    if (UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
      DELETE_IF_NOT_NULL(okey);
      return result_code;
    }
  }
  //  If UPLL_RC_ERR_NO_SUCH_INSTANCE there is no record in flowlist_ctrlr_tbl
  //  for ref_count to be updated. So Create a record in DB
  if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
    UPLL_LOG_DEBUG("No entry in ctrlr tbl");
    // scenario: flowlist1 is renamed into flowlist2
    // and it stored into candidate
    // now user created flowlist1 in candidate. UNC accept this configuration.
    // But UNC should return error,
    // when this flowlist (flowlist1) is referred by any
    if (dt_type == UPLL_DT_CANDIDATE) {
     uint8_t *ctrlrid = NULL;
     result_code = GetChildConfigKey(rename_key, okey);
     if (result_code != UPLL_RC_SUCCESS) {
       UPLL_LOG_DEBUG("GetChildConfigKey failed (%d)", result_code);
       DELETE_IF_NOT_NULL(okey);
       return result_code;
     }
     if (!rename_key) {
       UPLL_LOG_DEBUG("rename_key NULL");
       DELETE_IF_NOT_NULL(okey);
       return UPLL_RC_ERR_GENERIC;
     }
     GET_USER_DATA_CTRLR(rename_key, ctrlrid);

     result_code = GetRenamedUncKey(rename_key, UPLL_DT_CANDIDATE,
                                    dmi, ctrlrid);
     if (result_code == UPLL_RC_SUCCESS) {
       UPLL_LOG_DEBUG("flowlist name already renamed & exists %d", result_code);
       DELETE_IF_NOT_NULL(rename_key);
       DELETE_IF_NOT_NULL(okey);
       return UPLL_RC_ERR_CFG_SEMANTIC;
     } else if (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
       UPLL_LOG_DEBUG("GetRenamedUncKey Failed err_code %d", result_code);
       DELETE_IF_NOT_NULL(rename_key);
       DELETE_IF_NOT_NULL(okey);
       return result_code;
     } else {  // If NO_SUCH_INSTANCE check in RUNNING
       result_code = GetRenamedUncKey(rename_key, UPLL_DT_RUNNING,
                                      dmi, ctrlrid);
       DELETE_IF_NOT_NULL(rename_key);
       if (result_code == UPLL_RC_SUCCESS) {
         UPLL_LOG_DEBUG("flowlist name already renamed & exists %d",
                        result_code);
         DELETE_IF_NOT_NULL(okey);
         return UPLL_RC_ERR_CFG_SEMANTIC;
       } else if (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
         UPLL_LOG_DEBUG("GetRenamedUncKey Failed err_code %d", result_code);
         DELETE_IF_NOT_NULL(okey);
         return result_code;
       }
     }
#if 1
     // scenario: During import/partial import the flowlist name got renamed.
     // merge, commit, audit done. rename tbl info will be removed, when delete
     // the flowlist from candidate and create it again and commit.
     // Fix: copy the running renametbl configuration and placed it in
     // candidate configuration
     ConfigKeyVal *ckv_running_rename = NULL, *ckv_main = NULL;
     DbSubOp dbop = {kOpReadSingle, kOpMatchCtrlr, kOpInOutNone};

     result_code = GetChildConfigKey(ckv_running_rename, okey);
     if (result_code != UPLL_RC_SUCCESS) {
       UPLL_LOG_DEBUG("GetChildConfigKey failed (%d)", result_code);
       DELETE_IF_NOT_NULL(okey);
       return result_code;
     }
     if (!ckv_running_rename) {
       UPLL_LOG_DEBUG("rename_key NULL");
       DELETE_IF_NOT_NULL(okey);
       return UPLL_RC_ERR_GENERIC;
     }
     GET_USER_DATA_CTRLR(ckv_running_rename, ctrlrid);

     result_code = ReadConfigDB(ckv_running_rename, UPLL_DT_RUNNING,
            UNC_OP_READ, dbop, dmi, RENAMETBL);
     if (UPLL_RC_SUCCESS != result_code &&
            UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
          UPLL_LOG_DEBUG("ReadConfigDB failed %d", result_code)
          DELETE_IF_NOT_NULL(okey);
          DELETE_IF_NOT_NULL(ckv_running_rename);
          return result_code;
     }
     if (UPLL_RC_SUCCESS == result_code) {
        val_rename_flowlist_t* rename_flowlist = reinterpret_cast
           <val_rename_flowlist_t *>(GetVal(ckv_running_rename));
        rename_flowlist->valid[UPLL_IDX_RENAME_FLOWLIST_RFL] = UNC_VF_VALID;
        result_code = UpdateConfigDB(ckv_running_rename, UPLL_DT_CANDIDATE,
              UNC_OP_CREATE, dmi, config_mode, vtn_name, RENAMETBL);
        if (UPLL_RC_SUCCESS != result_code &&
            UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
            UPLL_LOG_DEBUG("UpdateConfigDB failed %d", result_code)
            DELETE_IF_NOT_NULL(okey);
            DELETE_IF_NOT_NULL(ckv_running_rename);
            return result_code;
        }

        result_code = GetChildConfigKey(ckv_main, NULL);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("GetChildConfigKey failed (%d)", result_code);
          DELETE_IF_NOT_NULL(okey);
          DELETE_IF_NOT_NULL(ckv_running_rename);
          return result_code;
        }
        key_flowlist_t *okey_key1 = reinterpret_cast<key_flowlist_t *>
          (ckv_main->get_key());
        uuu::upll_strncpy(okey_key1->flowlist_name, flowlist_name,
        (kMaxLenFlowListName+1));

        DbSubOp dbop_1 = {kOpNotRead, kOpMatchNone, kOpInOutFlag};

        SET_USER_DATA_FLAGS(ckv_main, 0x01);
        flag_chk = true;
        result_code = UpdateConfigDB(ckv_main, UPLL_DT_CANDIDATE,
              UNC_OP_UPDATE, dmi, &dbop_1, config_mode, vtn_name, MAINTBL);
        DELETE_IF_NOT_NULL(ckv_main);
        DELETE_IF_NOT_NULL(ckv_running_rename);
        if (UPLL_RC_SUCCESS != result_code &&
            UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
            UPLL_LOG_DEBUG("UpdateConfigDB failed %d", result_code)
            DELETE_IF_NOT_NULL(okey);
            return result_code;
        }
     }
     DELETE_IF_NOT_NULL(ckv_running_rename);
#endif
    }
    ConfigKeyVal *main_ckv = NULL;
    result_code = GetChildConfigKey(main_ckv, okey);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("GetChildConfigKey failed %d", result_code);
      DELETE_IF_NOT_NULL(okey);
      return result_code;
    }
    DbSubOp dbop1 = {kOpReadSingle, kOpMatchNone, kOpInOutFlag | kOpInOutCs};
    result_code = ReadConfigDB(main_ckv,
                            dt_type,
                            UNC_OP_READ,
                            dbop1, dmi, MAINTBL);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("ReadConfigDB in maintbl failed %d", result_code);
      if ((UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code)
          && ((UPLL_DT_AUDIT == dt_type) ||
              (UPLL_DT_CANDIDATE == dt_type))) {
        // For AUDIT, If flowlist name which is referred in policing profile
        // or flowfilter is not configured then
        // ignore the error  and return success
        UPLL_LOG_DEBUG("Skipping the controller table insertion for AUDIT");
        DELETE_IF_NOT_NULL(okey);
        DELETE_IF_NOT_NULL(main_ckv);
        return UPLL_RC_SUCCESS;
      }
      UPLL_LOG_ERROR("ReadConfigDB in maintbl failed %d", result_code);
      DELETE_IF_NOT_NULL(okey);
      DELETE_IF_NOT_NULL(main_ckv);
      return result_code;
    }
    if (UPLL_DT_IMPORT == dt_type || flag_chk == true) {
      // Copy the flag from maintbl and update into ctrlrtbl
      uint8_t fl_flag = 0x00;
      GET_USER_DATA_FLAGS(main_ckv, fl_flag);
      SET_USER_DATA_FLAGS(okey, fl_flag);
    }
    val_flowlist_t *main_val = reinterpret_cast<val_flowlist_t *>
        (GetVal(main_ckv));
    if (NULL == main_val) {
      UPLL_LOG_DEBUG(" Main Val struct is NULL");
      DELETE_IF_NOT_NULL(okey);
      DELETE_IF_NOT_NULL(main_ckv);
      return UPLL_RC_ERR_GENERIC;
    }
    val_flowlist_ctrl_t *ctrlr_val = reinterpret_cast<val_flowlist_ctrl_t *>
        (ConfigKeyVal::Malloc(sizeof(val_flowlist_ctrl_t)));
    okey->AppendCfgVal(IpctSt::kIpcInvalidStNum, ctrlr_val);
     // capability check
    IpcReqRespHeader *temp_req = reinterpret_cast<IpcReqRespHeader *>
      (ConfigKeyVal::Malloc(sizeof(IpcReqRespHeader)));

    if (NULL == temp_req) {
      UPLL_LOG_DEBUG(" Memory allocation for IpcReqRespHeader failed");
      DELETE_IF_NOT_NULL(main_ckv);
      DELETE_IF_NOT_NULL(okey);
      return UPLL_RC_ERR_GENERIC;
    }
    temp_req->operation = op;
    temp_req->datatype = UPLL_DT_CANDIDATE;

    result_code = ValidateCapability(temp_req, main_ckv, ctrl_id);
    free(temp_req);

    if (result_code != UPLL_RC_SUCCESS) {
       // FlowList is not supported for other than PFC/ODC Controller
       // so SKIP the adding entry for such sontroller
       DELETE_IF_NOT_NULL(main_ckv);
       DELETE_IF_NOT_NULL(okey);
       if ((!ctrlr_mgr->GetCtrlrType(reinterpret_cast<char *>(ctrl_id),
                       dt_type, &ctrlrtype)) || ((ctrlrtype != UNC_CT_PFC) &&
                                                 (ctrlrtype != UNC_CT_ODC))) {
          UPLL_LOG_DEBUG("Controller type is  %d", ctrlrtype);
          return UPLL_RC_SUCCESS;
       }
       UPLL_LOG_DEBUG("Key not supported by controller");
       return result_code;
    }
    if (UPLL_DT_CANDIDATE == dt_type) {
      ctrlr_val->refcount = count;
    } else {
      ctrlr_val->refcount = 1;
    }
    ctrlr_val->valid[1] = UNC_VF_VALID;
    ctrlr_val->valid[UPLL_IDX_IP_TYPE_FL] =
        main_val->valid[UPLL_IDX_IP_TYPE_FL];
    if (ctrlr_val->valid[UPLL_IDX_IP_TYPE_FL] == UNC_VF_NOT_SUPPORTED) {
      ctrlr_val->valid[UPLL_IDX_IP_TYPE_FL] = UNC_VF_INVALID;
    }
    if (UPLL_DT_AUDIT == dt_type) {
      UPLL_LOG_DEBUG("Setting cs cs_attr %d cs_row_status %d",
                     main_val->cs_attr[0], main_val->cs_row_status);
      ctrlr_val->cs_row_status =
          (unc_keytype_configstatus_t)main_val->cs_row_status;
      ctrlr_val->cs_attr[0] = (unc_keytype_configstatus_t)main_val->cs_attr[0];
      UPLL_LOG_DEBUG("Sutting ctrlr cs cs_attr %d cs_row_status %d",
                     ctrlr_val->cs_attr[0], ctrlr_val->cs_row_status);
    }
    result_code = UpdateConfigDB(okey, dt_type,
                                 UNC_OP_CREATE, dmi,
                                 config_mode, vtn_name,
                                 CTRLRTBL);
    if ((UPLL_RC_SUCCESS != result_code) &&
        (UPLL_RC_ERR_INSTANCE_EXISTS != result_code)) {
      UPLL_LOG_DEBUG("UpdateConfigDB failed to create in ctrlrtbl %d",
          result_code);
      DELETE_IF_NOT_NULL(main_ckv);
      DELETE_IF_NOT_NULL(okey);
      return result_code;
    }
    DELETE_IF_NOT_NULL(main_ckv);
  }
  FlowListEntryMoMgr *mgr = reinterpret_cast<FlowListEntryMoMgr *>
    (const_cast<MoManager *> (GetMoManager(UNC_KT_FLOWLIST_ENTRY)));
  result_code = mgr->AddFlowListToController(
                      flowlist_name, dmi, ctrl_id, dt_type, op,
                      config_mode, vtn_name);
  if (result_code != UPLL_RC_SUCCESS) {
    DELETE_IF_NOT_NULL(okey);
    UPLL_LOG_INFO("flowlistentry controller table update failed:%d",
         result_code);
    return result_code;
  }
  DELETE_IF_NOT_NULL(okey);
  return result_code;
}

upll_rc_t FlowListMoMgr::TxCopyCandidateToRunning(
    unc_key_type_t keytype, CtrlrCommitStatusList *ctrlr_commit_status,
    DalDmlIntf *dmi, TcConfigMode config_mode, std::string vtn_name) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  DalResultCode db_result;
  unc_keytype_operation_t op[] = { UNC_OP_DELETE, UNC_OP_CREATE,
                                   UNC_OP_UPDATE};
  int nop = sizeof(op) / sizeof(op[0]);
  ConfigKeyVal *flowlist_key = NULL, *req = NULL, *nreq = NULL ,
               *instance_key = NULL;
  DalCursor *cfg1_cursor = NULL;
  uint8_t *ctrlr_id = NULL;
  map<string, int> ctrlr_result;
  CtrlrCommitStatusList::iterator ccsListItr_begin;
  CtrlrCommitStatusList::iterator ccsListItr_end;
  CtrlrCommitStatus *ccStatusPtr;

  // To compute scratch tbl
  // ca_pp_scratch_tbl is computed first as an entry will be put in
  // fl_scratch tbl from ppe.
  if (config_mode != TC_CONFIG_VIRTUAL) {
    PolicingProfileMoMgr *mgr = reinterpret_cast<PolicingProfileMoMgr *>
          (const_cast<MoManager *> (GetMoManager(UNC_KT_POLICING_PROFILE)));
    result_code = mgr->ComputeCtrlrTblRefCountFromScratchTbl(
                      NULL, dmi, UPLL_DT_CANDIDATE,
                      config_mode, vtn_name);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("ComputeCtrlrTblRefCountFromScratchTbl returned %d",
                      result_code);
      return result_code;
    }
    result_code = ComputeCtrlrTblRefCountFromScratchTbl(
                      NULL, dmi, UPLL_DT_CANDIDATE,
                      config_mode, vtn_name);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("ComputeCtrlrTblRefCountFromScratchTbl returned %d",
                      result_code);
      return result_code;
    }
  }
  if (ctrlr_commit_status != NULL) {
    ccsListItr_begin = ctrlr_commit_status->begin();
    ccsListItr_end = ctrlr_commit_status->end();
    for (; ccsListItr_begin != ccsListItr_end; ++ccsListItr_begin) {
      ccStatusPtr = *ccsListItr_begin;
      ctrlr_id = reinterpret_cast<uint8_t *>
        (const_cast<char*>(ccStatusPtr->ctrlr_id.c_str()));
      ctrlr_result[ccStatusPtr->ctrlr_id] = ccStatusPtr->upll_ctrlr_result;
      if (ccStatusPtr->upll_ctrlr_result != UPLL_RC_SUCCESS) {
        for (ConfigKeyVal *ck_err = ccStatusPtr->err_ckv; ck_err != NULL;
             ck_err = ck_err->get_next_cfg_key_val()) {
          if (ck_err->get_key_type() != keytype) continue;
          result_code = GetRenamedUncKey(ck_err, UPLL_DT_CANDIDATE, dmi,
              ctrlr_id);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_INFO("Unable to get the Renamed UncKey %d", result_code);
            return result_code;
          }
        }
      }
    }
  }

  if (config_mode != TC_CONFIG_VTN) {
    for (int i = 0; i < nop; i++) {
      cfg1_cursor = NULL;
      // Update the Main table
      //    if (op[i] != UNC_OP_UPDATE) {
      result_code = DiffConfigDB(UPLL_DT_CANDIDATE, UPLL_DT_RUNNING, op[i],
                                 req, nreq, &cfg1_cursor, dmi, NULL,
                                 config_mode, vtn_name, MAINTBL , true);
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
      if (req)
        DELETE_IF_NOT_NULL(req);
      DELETE_IF_NOT_NULL(nreq);
      req = NULL;
//    }
      UPLL_LOG_DEBUG("Updating main table complete with op %d", op[i]);
    }  // for loop
  }  // if loop

  if (config_mode != TC_CONFIG_VIRTUAL) {
    for (int i = 0; i < nop; i++) {
      cfg1_cursor = NULL;
      // Update the controller table
      result_code = DiffConfigDB(UPLL_DT_CANDIDATE, UPLL_DT_RUNNING, op[i],
                              req, nreq, &cfg1_cursor, dmi, NULL, config_mode,
                              vtn_name, CTRLRTBL, true);
      ConfigKeyVal *flowlist_ctrlr_key = NULL;
      while (result_code == UPLL_RC_SUCCESS) {
        db_result = dmi->GetNextRecord(cfg1_cursor);
        result_code = DalToUpllResCode(db_result);
        if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
          result_code = UPLL_RC_SUCCESS;
          break;
        }
        if (op[i] == UNC_OP_CREATE) {
          DbSubOp dbop = { kOpReadSingle, kOpMatchNone,
            kOpInOutFlag | kOpInOutCs };
          result_code = GetChildConfigKey(flowlist_key, req);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("GetChildConfigKey is failed resultcode=%d",
                           result_code);
            if (cfg1_cursor)
              dmi->CloseCursor(cfg1_cursor, true);
            DELETE_IF_NOT_NULL(req);
            DELETE_IF_NOT_NULL(nreq);
            return result_code;
          }
          result_code = ReadConfigDB(flowlist_key, UPLL_DT_RUNNING,
                                   UNC_OP_READ, dbop, dmi, MAINTBL);
          if ((result_code != UPLL_RC_SUCCESS) &&
             (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE)) {
            UPLL_LOG_INFO("CandidateDB read failed:%d", result_code);
            DELETE_IF_NOT_NULL(flowlist_key);
            if (cfg1_cursor)
              dmi->CloseCursor(cfg1_cursor, true);
            DELETE_IF_NOT_NULL(req);
            DELETE_IF_NOT_NULL(nreq);
            return result_code;
          }

          result_code = DupConfigKeyVal(flowlist_ctrlr_key, req, CTRLRTBL);
          if (result_code != UPLL_RC_SUCCESS) {
            DELETE_IF_NOT_NULL(flowlist_key);
            UPLL_LOG_DEBUG("DupConfigVal function is failed %d", result_code);
            if (cfg1_cursor)
              dmi->CloseCursor(cfg1_cursor, true);
            DELETE_IF_NOT_NULL(req);
            DELETE_IF_NOT_NULL(nreq);
            return result_code;
          }
          result_code = GetChildConfigKey(instance_key, flowlist_key);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("GetChildConfigKey for instance_key is failed: %d",
                          result_code);
            DELETE_IF_NOT_NULL(flowlist_ctrlr_key);
            DELETE_IF_NOT_NULL(flowlist_key);
            if (cfg1_cursor)
              dmi->CloseCursor(cfg1_cursor, true);
            DELETE_IF_NOT_NULL(req);
            DELETE_IF_NOT_NULL(nreq);
            return result_code;
          }

         /* set consolidated config status to UNKNOWN to init  cs_status
         * to the cs_status of first controller
         */
          uint32_t cur_instance_count;
          result_code = GetInstanceCount(instance_key, NULL,
                                   UPLL_DT_CANDIDATE, &cur_instance_count,
                                   dmi, CTRLRTBL);
          UPLL_LOG_TRACE("in iiiflowlist cs_status %d \n",
                         cur_instance_count);
          if ((result_code == UPLL_RC_SUCCESS) && (cur_instance_count == 1))
            reinterpret_cast<val_flowlist *>(GetVal(flowlist_key))->
            cs_row_status = UNC_CS_UNKNOWN;

            DELETE_IF_NOT_NULL(instance_key);

            GET_USER_DATA_CTRLR(flowlist_ctrlr_key, ctrlr_id);
            string controller(reinterpret_cast<char *>(ctrlr_id));
            if (ctrlr_result.empty()) {
              result_code = UpdateConfigStatus(flowlist_key, op[i],
                                           UPLL_RC_ERR_CTR_DISCONNECTED, nreq,
                                           dmi, flowlist_ctrlr_key);
            } else {
              result_code = UpdateConfigStatus(flowlist_key, op[i],
                                           ctrlr_result[controller], nreq,
                                           dmi, flowlist_ctrlr_key);
           }
           if (result_code != UPLL_RC_SUCCESS) {
             UPLL_LOG_INFO("UpdateConfigStatus function failed - %d ",
                           result_code);
             DELETE_IF_NOT_NULL(flowlist_key);
             DELETE_IF_NOT_NULL(flowlist_ctrlr_key);
             if (cfg1_cursor)
               dmi->CloseCursor(cfg1_cursor, true);
             DELETE_IF_NOT_NULL(req);
             DELETE_IF_NOT_NULL(nreq);
             return result_code;
           }
        } else if (op[i] == UNC_OP_DELETE) {
            DbSubOp dbop = { kOpReadSingle, kOpMatchNone,
                             kOpInOutFlag | kOpInOutCs };
            result_code = GetChildConfigKey(flowlist_key, req);
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
            result_code = ReadConfigDB(flowlist_key, UPLL_DT_RUNNING,
                                   UNC_OP_READ, dbop, dmi, MAINTBL);

            if (result_code != UPLL_RC_SUCCESS &&
                result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
              UPLL_LOG_DEBUG("Unable to read configuration from RunningDB");
              DELETE_IF_NOT_NULL(flowlist_key);
              if (cfg1_cursor)
                dmi->CloseCursor(cfg1_cursor, true);
              DELETE_IF_NOT_NULL(req);
              DELETE_IF_NOT_NULL(nreq);
              return result_code;
            }
            if (result_code == UPLL_RC_SUCCESS) {
              result_code = SetFlowListConsolidatedStatus(flowlist_key,
                                                        ctrlr_id, dmi);
              if (result_code != UPLL_RC_SUCCESS) {
                UPLL_LOG_INFO("Could not set consolidated status %d",
                               result_code);
                DELETE_IF_NOT_NULL(flowlist_key);
                if (cfg1_cursor)
                  dmi->CloseCursor(cfg1_cursor, true);
                DELETE_IF_NOT_NULL(req);
                DELETE_IF_NOT_NULL(nreq);
                return result_code;
              }
            }
            result_code = GetChildConfigKey(flowlist_ctrlr_key, req);
            if (result_code != UPLL_RC_SUCCESS) {
              UPLL_LOG_DEBUG("GetChildConfigKey Failed  %d", result_code);
              DELETE_IF_NOT_NULL(flowlist_key);
              if (cfg1_cursor)
                dmi->CloseCursor(cfg1_cursor, true);
              DELETE_IF_NOT_NULL(req);
              DELETE_IF_NOT_NULL(nreq);
              return result_code;
            }
          }
          if (UNC_OP_CREATE == op[i]) {
            val_flowlist_ctrl_t *val_ctrlr_temp = reinterpret_cast
               <val_flowlist_ctrl_t *>(GetVal(flowlist_ctrlr_key));
            val_flowlist_ctrl_t *val_ctrlr_temp1 = reinterpret_cast
               <val_flowlist_ctrl_t *>(GetVal(req));
            val_ctrlr_temp->valid[1] = UNC_VF_VALID;
            val_ctrlr_temp1->refcount = val_ctrlr_temp->refcount;
          } else if (UNC_OP_UPDATE == op[i]) {
            result_code = DupConfigKeyVal(flowlist_ctrlr_key, nreq, CTRLRTBL);
            if (result_code != UPLL_RC_SUCCESS) {
              UPLL_LOG_DEBUG("DupConfigVal function is failed %d",
                           result_code);
              if (cfg1_cursor)
                dmi->CloseCursor(cfg1_cursor, true);
              DELETE_IF_NOT_NULL(req);
              DELETE_IF_NOT_NULL(nreq);
              return result_code;
            }
            val_flowlist_ctrl_t *val_ctrlr_temp = reinterpret_cast
              <val_flowlist_ctrl_t *>(GetVal(flowlist_ctrlr_key));
            val_ctrlr_temp->valid[1] = UNC_VF_VALID;
            val_flowlist_ctrl_t *val_ctrlr_temp1 = reinterpret_cast
                <val_flowlist_ctrl_t *>(GetVal(req));
            val_ctrlr_temp->refcount = val_ctrlr_temp1->refcount;
            UPLL_LOG_DEBUG("Ref count in dupckv%d", val_ctrlr_temp->refcount);
            UPLL_LOG_DEBUG("Ref count in req%d", val_ctrlr_temp1->refcount);
          }
          result_code = UpdateConfigDB(flowlist_ctrlr_key, UPLL_DT_RUNNING,
                          op[i], dmi, config_mode, vtn_name, CTRLRTBL);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("Unable to Update Configuration at DB %d",
                           result_code);
            DELETE_IF_NOT_NULL(flowlist_ctrlr_key);
            DELETE_IF_NOT_NULL(flowlist_key);
            if (cfg1_cursor)
              dmi->CloseCursor(cfg1_cursor, true);
            DELETE_IF_NOT_NULL(req);
            DELETE_IF_NOT_NULL(nreq);
            return result_code;
          }

          // update the consolidated config status in the Main Table
         if (op[i] == UNC_OP_CREATE) {
           result_code = UpdateConfigDB(flowlist_key, UPLL_DT_RUNNING,
                                    UNC_OP_UPDATE, dmi, config_mode,
                                    vtn_name, MAINTBL);
           if (result_code != UPLL_RC_SUCCESS) {
             DELETE_IF_NOT_NULL(flowlist_ctrlr_key);
             DELETE_IF_NOT_NULL(flowlist_key);
             if (cfg1_cursor)
              dmi->CloseCursor(cfg1_cursor, true);
             DELETE_IF_NOT_NULL(req);
             DELETE_IF_NOT_NULL(nreq);
             return result_code;
           }
         }

         EnqueCfgNotification(op[i], UPLL_DT_RUNNING, flowlist_ctrlr_key);
         if (flowlist_ctrlr_key) delete flowlist_ctrlr_key;
           DELETE_IF_NOT_NULL(flowlist_key);
           flowlist_key = flowlist_ctrlr_key = NULL;
           result_code = DalToUpllResCode(db_result);
         }
         if (cfg1_cursor) {
           dmi->CloseCursor(cfg1_cursor, true);
           cfg1_cursor = NULL;
         }
         if (req) delete req;
         if (nreq) delete nreq;
         nreq = req = NULL;
         result_code = TxCopyRenameTableFromCandidateToRunning(keytype,
                                                          op[i], dmi,
                                                          config_mode,
                                                          vtn_name);
         UPLL_LOG_DEBUG("TxCopyRenameTableFromCandidateToRunning returned %d",
                                                            result_code);
    }  // for loop
  }  //  if loop
  result_code = (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) ?
      UPLL_RC_SUCCESS : result_code;

  if ((UPLL_RC_SUCCESS == result_code) &&
      (config_mode != TC_CONFIG_VIRTUAL)) {
    ClearScratchTbl(config_mode, vtn_name, dmi);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("ClearScratchTbl failed %d", result_code);
    }
  }
  return result_code;
}

bool FlowListMoMgr::IsValidKey(void *key, uint64_t index,
                               MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  key_flowlist_t *flowlist_key =
    reinterpret_cast<key_flowlist_t*> (key);
  upll_rc_t ret_val;
  ret_val = ValidateKey(reinterpret_cast<char *>
                        (flowlist_key->flowlist_name),
                        kMinLenFlowListName,
                        kMaxLenFlowListName);
  if (ret_val != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("FlowList Name is not valid(%d)", ret_val);
      return false;
  }
  return true;
}

upll_rc_t FlowListMoMgr::UpdateAuditConfigStatus(
                           unc_keytype_configstatus_t cs_status,
                           uuc::UpdateCtrlrPhase phase,
                           ConfigKeyVal *&ckv_running,
                           DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  val_flowlist_ctrl_t *val;
  val = (ckv_running != NULL)?
                            reinterpret_cast<val_flowlist_ctrl_t *>
                            (GetVal(ckv_running)):NULL;
  if (NULL == val) {
    UPLL_LOG_DEBUG("Value Structure is Empty");
    return UPLL_RC_ERR_GENERIC;
  }
  UPLL_LOG_DEBUG("The Value of Cs Status at start is %d", cs_status);
  if (uuc::kUpllUcpCreate == phase )
    val->cs_row_status = cs_status;
  if ((uuc::kUpllUcpUpdate == phase) &&
           (val->cs_row_status == UNC_CS_INVALID ||
            val->cs_row_status == UNC_CS_NOT_APPLIED))
    val->cs_row_status = cs_status;
  if ((cs_status == UNC_CS_INVALID && UNC_VF_VALID == val->valid[0]) ||
       cs_status == UNC_CS_APPLIED)
    val->cs_attr[0] = cs_status;

  return result_code;
}

upll_rc_t FlowListMoMgr::SetConsolidatedStatus(ConfigKeyVal *ikey,
                                               DalDmlIntf *dmi) {
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *ckv = NULL;
  DbSubOp dbop = {kOpReadMultiple, kOpMatchNone, kOpInOutCs};
  result_code = GetChildConfigKey(ckv, ikey);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("Unable to Get the configKey");
    return result_code;
  }
  result_code = ReadConfigDB(ckv,
                             UPLL_DT_RUNNING,
                             UNC_OP_READ, dbop, dmi, CTRLRTBL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_ERROR("CTRLR table read failed:%d", result_code);
    if (ckv != NULL) {
      delete ckv;
      ckv = NULL;
    }
    return result_code;
  }
  std::list< unc_keytype_configstatus_t > list_cs_row;
  std::list< unc_keytype_configstatus_t > list_cs_attr;
  val_flowlist_ctrl_t *val;
  ConfigKeyVal *temp_ckv = ckv;
  for ( ; temp_ckv != NULL ; temp_ckv = temp_ckv->get_next_cfg_key_val()) {
      val = reinterpret_cast<val_flowlist_ctrl_t *>(GetVal(temp_ckv));
      list_cs_row.push_back((unc_keytype_configstatus_t)val->cs_row_status);
      list_cs_attr.push_back((unc_keytype_configstatus_t)val->cs_attr[0]);
  }
  DELETE_IF_NOT_NULL(ckv);
  val_flowlist_t *val_temp = reinterpret_cast<val_flowlist_t *>(GetVal(ikey));
  val_temp->cs_row_status = GetConsolidatedCsStatus(list_cs_row);
  val_temp->cs_attr[0] = GetConsolidatedCsStatus(list_cs_attr);
  std::string temp_vtn_name = "";
  result_code = UpdateConfigDB(ikey,
                               UPLL_DT_RUNNING,
                               UNC_OP_UPDATE, dmi, TC_CONFIG_GLOBAL,
                               temp_vtn_name, MAINTBL);
  if (UPLL_RC_SUCCESS != result_code) {
    return result_code;
  }
  return result_code;
}

bool FlowListMoMgr::CompareKey(void *key1, void *key2) {
  key_flowlist_t *flowlist_key1, *flowlist_key2;
  bool match = false;
  flowlist_key1 = reinterpret_cast<key_flowlist_t *>(key1);
  flowlist_key2 = reinterpret_cast<key_flowlist_t *>(key2);
  if (flowlist_key1 == flowlist_key2)
      return true;
  if ((!flowlist_key1) || (!flowlist_key2)) {
    UPLL_LOG_DEBUG(" CompareKey failed");
    return false;
  }
  if (strcmp(reinterpret_cast<char *>(flowlist_key1->flowlist_name),
             reinterpret_cast< char *>(flowlist_key2->flowlist_name)) == 0) {
    match = true;
     UPLL_LOG_DEBUG(" CompareKey . Both Keys are same");
  }
  return match;
}

upll_rc_t FlowListMoMgr::ValidateMessage(IpcReqRespHeader *req,
                                         ConfigKeyVal *key) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_ERR_GENERIC;

  if ((NULL == req) || (NULL == key)) {
    UPLL_LOG_DEBUG("IpcReqRespHeader/ConfigKeyval is NULL");
    return UPLL_RC_ERR_BAD_REQUEST;
  }

  if (UNC_KT_FLOWLIST != key->get_key_type()) {
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
        (result_code = ValidateFlowListKey(key, req->operation))) {
      UPLL_LOG_DEBUG(" flow-list key validation failed result(%d)",
                     result_code);
      return result_code;
    }

  if ((req->operation == UNC_OP_RENAME) ||
      ((req->datatype == UPLL_DT_IMPORT) &&
       ((req->operation == UNC_OP_READ) ||
        (req->operation == UNC_OP_READ_SIBLING) ||
        (req->operation == UNC_OP_READ_SIBLING_BEGIN)))) {
    if (UPLL_RC_SUCCESS != (result_code = ValidateFlowListValRename(
                key, req->operation, req->datatype))) {
      UPLL_LOG_DEBUG(" flow-list val rename validation failed ");
    }
  } else {
    /* validate val struct based on datatype, operation, options */
    if (UPLL_RC_SUCCESS != (result_code = ValidateFlowListVal(
                key, req->operation, req->datatype))) {
      UPLL_LOG_DEBUG(" flow-list val validation failed ");
    }
  }


  // TODO(author) Capability check should be done for Read operations
  return result_code;
}

upll_rc_t FlowListMoMgr::ValidateFlowListKey(ConfigKeyVal *key,
                                             unc_keytype_operation_t op) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_ERR_GENERIC;
  /** check received key, val struct in configkeyval is valid
   * KT_FLOWLIST structs */
  if (key->get_st_num() != IpctSt::kIpcStKeyFlowlist) {
    UPLL_LOG_DEBUG("Invalid key structure received. struct num - %d",
                   key->get_st_num());
    return UPLL_RC_ERR_BAD_REQUEST;
  }

  /** Read key, val struct from ConfigKeyVal */
  key_flowlist_t *key_flowlist =
      reinterpret_cast<key_flowlist_t *>(key->get_key());

  if (NULL == key_flowlist) {
    UPLL_LOG_DEBUG("KT_FLOWLIST Key structure is null");
    return UPLL_RC_ERR_BAD_REQUEST;
  }

  if ((op != UNC_OP_READ_SIBLING_COUNT) &&
      (op != UNC_OP_READ_SIBLING_BEGIN)) {
    /** validate key struct */
    result_code = ValidateKey(
        reinterpret_cast<char*>(key_flowlist->flowlist_name),
        kMinLenFlowListName,
        kMaxLenFlowListName);

    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG(" flowlist key validation failed %s, err code - %d",
                     key_flowlist->flowlist_name, result_code);
    }
  } else {
    UPLL_LOG_DEBUG(" operation is %d", op);
    if (strlen(reinterpret_cast<const char*>(key_flowlist->flowlist_name))) {
      memset(key_flowlist->flowlist_name, 0, (kMaxLenFlowListName + 1));
    }
    result_code = UPLL_RC_SUCCESS;
  }
  return result_code;
}

upll_rc_t FlowListMoMgr::ValidateFlowListValRename(ConfigKeyVal *key,
                                                   uint32_t operation,
                                                   uint32_t datatype) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  if (!key->get_cfg_val()) {
    // operation is read/read_sibling/read_sibling_begin and data type is import
    if ((datatype == UPLL_DT_IMPORT) &&
        ((operation == UNC_OP_READ) || (operation == UNC_OP_READ_SIBLING) ||
         (operation == UNC_OP_READ_SIBLING_BEGIN))) {
      UPLL_LOG_DEBUG("val stucture is optional");
      return UPLL_RC_SUCCESS;
    } else {
      UPLL_LOG_DEBUG(" val structure is mandatory");
      return UPLL_RC_ERR_BAD_REQUEST;
    }
  }
  if ((key->get_cfg_val())->get_st_num() != IpctSt::kIpcStValRenameFlowlist) {
    UPLL_LOG_DEBUG("Invalid val structure received. struct num - %d",
                   (key->get_cfg_val())->get_st_num());
    return UPLL_RC_ERR_BAD_REQUEST;
      }

  if (NULL == key->get_cfg_val()->get_val()) {
    UPLL_LOG_DEBUG("KT_FLOWLIST val rename structure is null");
    return UPLL_RC_ERR_BAD_REQUEST;
  }
  val_rename_flowlist_t *val_rename_flowlist =
      static_cast<val_rename_flowlist_t *>(key->get_cfg_val()->get_val());

  if (UPLL_DT_IMPORT != datatype) {
  if (val_rename_flowlist->valid[UPLL_IDX_RENAME_FLOWLIST_RFL]
      != UNC_VF_VALID) {
    UPLL_LOG_DEBUG(" flowlist rename value is not set");
    return UPLL_RC_ERR_BAD_REQUEST;
    }
  }
  if (val_rename_flowlist->valid[UPLL_IDX_RENAME_FLOWLIST_RFL]
        == UNC_VF_VALID) {
    result_code = ValidateKey(
      reinterpret_cast<char *>(val_rename_flowlist->flowlist_newname),
      (unsigned int)kMinLenFlowListName,
      (unsigned int)kMaxLenFlowListName);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG(" flowlist rename value validation failed %s",
                   val_rename_flowlist->flowlist_newname);
      return result_code;
    }
  }
  return result_code;
}

upll_rc_t FlowListMoMgr::ValidateFlowListVal(ConfigKeyVal *key,
                                             uint32_t operation,
                                             uint32_t datatype) {
  UPLL_FUNC_TRACE;
  if (!key->get_cfg_val()) {
    // request val structure can be null in following condition
    // operation is delete
    // operation is read/read_sibiling/read_sibling_begin/read_sibling_count
    // and data type is candidate/running/state/startup
    //
    // request val structure is mandatory when
    // operation is create and
    // data type is candidate/running/state/startup
    if ((operation == UNC_OP_CREATE)/* || (operation == UNC_OP_READ)*/) {
      UPLL_LOG_DEBUG(" val structure is mandatory");
      return UPLL_RC_ERR_BAD_REQUEST;
    } else {
      UPLL_LOG_DEBUG("val stucture is optional");
      return UPLL_RC_SUCCESS;
    }
  }
  if ((key->get_cfg_val()->get_st_num() != IpctSt::kIpcStValFlowlist)) {
    UPLL_LOG_DEBUG("Invalid val structure received. struct num - %d",
                   (key->get_cfg_val())->get_st_num());
    return UPLL_RC_ERR_BAD_REQUEST;
  }

  val_flowlist_t *val_flowlist = reinterpret_cast<val_flowlist_t *>
      (key->get_cfg_val()->get_val());
  if (NULL == val_flowlist) {
    UPLL_LOG_DEBUG("KT_FLOWLIST val structure is null");
    return UPLL_RC_ERR_BAD_REQUEST;
  }

  // validate if ip_type valid flag is set to valid
  if (val_flowlist->valid[UPLL_IDX_IP_TYPE_FL] == UNC_VF_VALID) {
    // ip_type is either ip or Ipv6
    if (!ValidateNumericRange(val_flowlist->ip_type,
                              (uint8_t) UPLL_FLOWLIST_TYPE_IP,
                              (uint8_t) UPLL_FLOWLIST_TYPE_IPV6, true, true)) {
      UPLL_LOG_DEBUG(" Syntax check failed invalid Ip type - %d",
                     val_flowlist->ip_type);
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  } else if ((operation == UNC_OP_CREATE) &&
      (val_flowlist->valid[UPLL_IDX_IP_TYPE_FL] == UNC_VF_VALID_NO_VALUE)) {
    UPLL_LOG_TRACE("Reset ip_type");
    val_flowlist->ip_type = UPLL_FLOWLIST_TYPE_IP;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t FlowListMoMgr::ValidateCapability(IpcReqRespHeader *req,
                                            ConfigKeyVal *ikey,
                                            const char* ctrlr_name) {
  UPLL_FUNC_TRACE;

  upll_rc_t ret_val = UPLL_RC_ERR_GENERIC;

  if ((NULL == req) || (NULL == ikey)) {
    UPLL_LOG_DEBUG("IpcReqRespHeader/ConfigKeyval is NULL");
    return ret_val;
  }

  if (!ctrlr_name) {
    ctrlr_name = static_cast<char *>(ikey->get_user_data());
  }

  if (NULL == ctrlr_name) {
    UPLL_LOG_DEBUG("ctrlr_name is NULL");
    return UPLL_RC_ERR_GENERIC;
  }

  UPLL_LOG_TRACE("ctrlr_name(%s), datatype : (%d)",
                ctrlr_name, req->datatype);

  bool result_code = false;
  uint32_t max_instance_count = 0;
  const uint8_t *attrs = 0;
  uint32_t max_attrs = 0;

  switch (req->operation) {
    case UNC_OP_CREATE: {
      result_code = GetCreateCapability(ctrlr_name, ikey->get_key_type(),
                                        &max_instance_count,
                                        &max_attrs, &attrs);
      break;
    }
    case UNC_OP_UPDATE: {
      result_code = GetUpdateCapability(ctrlr_name, ikey->get_key_type(),
                                        &max_attrs, &attrs);
      break;
    }
    default:
      if (req->datatype == UPLL_DT_STATE) {
        result_code = GetStateCapability(ctrlr_name, ikey->get_key_type(),
                                      &max_attrs, &attrs);
      } else {
        result_code = GetReadCapability(ctrlr_name, ikey->get_key_type(),
                                      &max_attrs, &attrs);
      }
      break;
  }

  if (!result_code) {
    UPLL_LOG_DEBUG("keytype(%d) is not supported by controller(%s)"
                   " for operation(%d)",
                   ikey->get_key_type(), ctrlr_name, req->operation);
    return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
  }

  val_flowlist_t *val_flowlist =
       reinterpret_cast<val_flowlist_t *>(GetVal(ikey));

  if (val_flowlist) {
     if (max_attrs > 0) {
        return ValFlowlistAttributeSupportCheck(val_flowlist, attrs);
     } else {
       UPLL_LOG_DEBUG("Attribute list is empty for operation %d",
                      req->operation);
       return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
     }
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t FlowListMoMgr::ValFlowlistAttributeSupportCheck(
val_flowlist_t *val_flowlist, const uint8_t* attrs ) {
  UPLL_FUNC_TRACE;

  if ((val_flowlist->valid[UPLL_IDX_IP_TYPE_FL] == UNC_VF_VALID)
      || (val_flowlist->valid[UPLL_IDX_IP_TYPE_FL] ==
         UNC_VF_VALID_NO_VALUE)) {
    if (attrs[unc::capa::flowlist::kCapIpType] == 0) {
      val_flowlist->valid[UPLL_IDX_IP_TYPE_FL] = UNC_VF_NOT_SUPPORTED;

      UPLL_LOG_DEBUG("IPType attr is not supported by ctrlr");
    }
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t FlowListMoMgr::GetRenameInfo(ConfigKeyVal *ikey,
    ConfigKeyVal *okey, ConfigKeyVal *&rename_info, DalDmlIntf *dmi,
    const char *ctrlr_id, bool &renamed) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  if (!ikey || !okey || NULL != rename_info
      || !(ikey->get_key()) || !(okey->get_key())) {
    UPLL_LOG_DEBUG("Insufficient parameters");
    return UPLL_RC_ERR_GENERIC;
  }
  key_rename_vnode_info_t *key_rename_info =
  reinterpret_cast<key_rename_vnode_info_t*>
  (ConfigKeyVal::Malloc(sizeof(key_rename_vnode_info_t)));

  key_flowlist_t *flowlist_key = NULL;
  flowlist_key = reinterpret_cast<key_flowlist_t *>
    (ikey->get_key());
  if (renamed) {
    uuu::upll_strncpy(key_rename_info->ctrlr_flowlist_name,
               reinterpret_cast<val_rename_flowlist_t*>
               (GetVal(ikey))->flowlist_newname, (kMaxLenFlowListName+1));

  } else {
    /* if not renamed the ikey contains the controller name */
    uuu::upll_strncpy(key_rename_info->ctrlr_flowlist_name,
        flowlist_key->flowlist_name,
        (kMaxLenFlowListName + 1));
    UPLL_LOG_DEBUG("key_rename_info->ctrlr_flowlist_name ::: (%s)",
                   key_rename_info->ctrlr_flowlist_name);
  }

  uuu::upll_strncpy(key_rename_info->old_flowlist_name,
               flowlist_key->flowlist_name,
               (kMaxLenFlowListName+1));

  flowlist_key = reinterpret_cast<key_flowlist_t *>
    (okey->get_key());

  uuu::upll_strncpy(key_rename_info->new_flowlist_name,
               flowlist_key->flowlist_name,
               (kMaxLenFlowListName+1));

  rename_info = new ConfigKeyVal(UNC_KT_FLOWLIST,
      IpctSt::kIpcInvalidStNum, key_rename_info, NULL);
  if (!rename_info) {
    free(key_rename_info);
    key_rename_info = NULL;
    UPLL_LOG_DEBUG("Failed to allocate memory for ConfigKeyVal");
    return UPLL_RC_ERR_GENERIC;
  }

  DbSubOp dbop = {kOpReadExist, kOpMatchNone, kOpInOutNone};
  // ikey has rename val set, so removing  that to read from ctrlr tbl
  ikey->SetCfgVal(NULL);
  result_code = UpdateConfigDB(ikey, UPLL_DT_IMPORT,
                             UNC_OP_READ, dmi, &dbop, CTRLRTBL);
  if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
    result_code = UPLL_RC_ERR_NOT_ALLOWED_AT_THIS_TIME;
    DELETE_IF_NOT_NULL(rename_info);
    UPLL_LOG_DEBUG("StandAlone configuration found %d", result_code);
    return result_code;
  } else if (UPLL_RC_ERR_INSTANCE_EXISTS != result_code)  {
    UPLL_LOG_DEBUG("ReadConfigDB failed - %d", result_code);
    DELETE_IF_NOT_NULL(rename_info);
    return result_code;
  }
  SET_USER_DATA_CTRLR(rename_info, ctrlr_id);
  if (!renamed) {
    val_rename_flowlist_t *val_rename =
    reinterpret_cast<val_rename_flowlist_t*>
    (ConfigKeyVal::Malloc(sizeof(val_rename_flowlist_t)));
    uuu::upll_strncpy(val_rename->flowlist_newname,
                 key_rename_info->old_flowlist_name,
                 (kMaxLenFlowListName+1));
    val_rename->valid[UPLL_IDX_RENAME_FLOWLIST_RFL] = UNC_VF_VALID;
    ConfigVal *cfg_val = new ConfigVal(IpctSt::kIpcStValRenameFlowlist,
        val_rename);
    okey->SetCfgVal(cfg_val);
    SET_USER_DATA_CTRLR(okey, ctrlr_id);
    DbSubOp dbop1 = {kOpNotRead, kOpMatchNone, kOpInOutCtrlr};
    string temp_vtn_name = "";
    result_code = UpdateConfigDB(okey, UPLL_DT_IMPORT, UNC_OP_CREATE, dmi,
        &dbop1, TC_CONFIG_GLOBAL, temp_vtn_name, RENAMETBL);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG(" GetRenameInfo Failed. UpdateConfigDb Failed"
        " Result code - %d", result_code);
    }
  }
  if (UPLL_RC_ERR_INSTANCE_EXISTS == result_code)
    result_code = UPLL_RC_SUCCESS;
  UPLL_LOG_DEBUG("Exiting GetRenameInfo result_code - %d", result_code);
  return result_code;
}

upll_rc_t FlowListMoMgr::CopyToConfigKey(ConfigKeyVal *&okey,
    ConfigKeyVal *ikey) {

  if ( !ikey || !(ikey->get_key()) )
    return UPLL_RC_ERR_GENERIC;

  upll_rc_t result_code = UPLL_RC_SUCCESS;
  key_flowlist_t *key_flowlist = NULL;

  if (UNC_KT_FLOWLIST == ikey->get_key_type()) {
    key_rename_vnode_info_t *key_rename =
      reinterpret_cast<key_rename_vnode_info_t *>(ikey->get_key());
    key_flowlist =
      reinterpret_cast<key_flowlist_t*>
      (ConfigKeyVal::Malloc(sizeof(key_flowlist_t)));
    if (!strlen(reinterpret_cast<char *>
          (key_rename->old_flowlist_name))) {
      free(key_flowlist);
      return UPLL_RC_ERR_GENERIC;
    }
    uuu::upll_strncpy(key_flowlist->flowlist_name,
                      key_rename->old_flowlist_name,
                      (kMaxLenFlowListName+1));

    okey = new ConfigKeyVal(UNC_KT_FLOWLIST,
        IpctSt::kIpcStKeyFlowlist, key_flowlist, NULL);
    if (!okey) {
      UPLL_LOG_DEBUG("Memory allocation failed for ConfigKeyVal");
      free(key_flowlist);
      result_code = UPLL_RC_ERR_GENERIC;
      return result_code;
    }
    SET_USER_DATA(okey, ikey);
  } else {
    UPLL_LOG_DEBUG("Invalid Keytype (%d)", ikey->get_key_type());
    result_code = UPLL_RC_ERR_GENERIC;
  }
  return result_code;
}

bool FlowListMoMgr::FilterAttributes(void *&val1,
                                     void *val2,
                                     bool copy_to_running,
                                     unc_keytype_operation_t op) {
  UPLL_FUNC_TRACE;
  if (op != UNC_OP_CREATE)
    return CompareValidValue(val1, val2, copy_to_running);
  return false;
}

bool FlowListMoMgr::CompareValidValue(void *&val1, void *val2,
                                      bool audit) {
  UPLL_FUNC_TRACE;
  bool invalid_attr = true;
  val_flowlist_t *flowlist_val1 =
    reinterpret_cast<val_flowlist_t *>(val1);

  val_flowlist_t *flowlist_val2 =
    reinterpret_cast<val_flowlist_t *>(val2);
//  if (flowlist_val1 && flowlist_val2) {
  //  if (audit) {
  for (unsigned int loop = 0; loop < sizeof(flowlist_val1->valid);
       ++loop ) {
    if (UNC_VF_INVALID == flowlist_val1->valid[loop] &&
        UNC_VF_VALID == flowlist_val2->valid[loop]) {
        flowlist_val1->valid[loop] = UNC_VF_VALID_NO_VALUE;
    }
  }
  // }
  if (UNC_VF_VALID == flowlist_val1->valid[UPLL_IDX_IP_TYPE_FL] &&
      UNC_VF_VALID == flowlist_val2->valid[UPLL_IDX_IP_TYPE_FL]) {
    if (flowlist_val1->ip_type == flowlist_val2->ip_type) {
        flowlist_val1->valid[UPLL_IDX_IP_TYPE_FL] = UNC_VF_INVALID;
    }
  }
  for (unsigned int loop = 0;
      loop < sizeof(flowlist_val1->valid) / sizeof(uint8_t); ++loop) {
    if ((UNC_VF_VALID == (uint8_t) flowlist_val1->valid[loop]) ||
       (UNC_VF_VALID_NO_VALUE == (uint8_t) flowlist_val1->valid[loop]))
        invalid_attr = false;
  }
  return invalid_attr;
}

upll_rc_t FlowListMoMgr::UpdateMainTbl(ConfigKeyVal *key_fl,
      unc_keytype_operation_t op, uint32_t driver_result,
      ConfigKeyVal *nreq, DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  ConfigKeyVal *ck_fl = NULL;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  val_flowlist_t *val_fl = NULL;
  std::string vtn_name = "";

  switch (op) {
    case UNC_OP_CREATE:
    case UNC_OP_UPDATE:
      result_code = DupConfigKeyVal(ck_fl, key_fl, MAINTBL);
      if (!ck_fl || (result_code != UPLL_RC_SUCCESS)) {
        UPLL_LOG_DEBUG("DupConfigKeyVal() Returning error %d", result_code);
        return result_code;
      }
      val_fl = reinterpret_cast<val_flowlist_t *>(GetVal(ck_fl));
      if (!val_fl) {
        UPLL_LOG_DEBUG("invalid val");
        DELETE_IF_NOT_NULL(ck_fl);
        return UPLL_RC_ERR_GENERIC;
      }
      val_fl->cs_row_status = UNC_CS_APPLIED;
      break;
    case UNC_OP_DELETE:

      result_code = GetChildConfigKey(ck_fl, key_fl);
      if (!ck_fl || (result_code != UPLL_RC_SUCCESS)) {
        UPLL_LOG_DEBUG("GetChildConfigKey() returning error %d", result_code);
        return UPLL_RC_ERR_GENERIC;
      }
      break;
    default:
          UPLL_LOG_DEBUG("Inalid operation");
      return UPLL_RC_ERR_GENERIC;
  }

  result_code = UpdateConfigDB(ck_fl, UPLL_DT_STATE, op, dmi, TC_CONFIG_GLOBAL,
                               vtn_name, MAINTBL);
  EnqueCfgNotification(op, UPLL_DT_RUNNING, key_fl);
  delete ck_fl;
  return result_code;
}

upll_rc_t FlowListMoMgr::GetParentConfigKey(ConfigKeyVal *&okey,
                                            ConfigKeyVal *ikey) {
  UPLL_FUNC_TRACE;

  if (!ikey) {
    UPLL_LOG_DEBUG(" Input Key is NULL");
    return UPLL_RC_ERR_GENERIC;
  }

  unc_key_type_t ikey_type = ikey->get_key_type();
  if (ikey_type != UNC_KT_FLOWLIST) {
    UPLL_LOG_DEBUG(" Invalid key type received. Key type - %d", ikey_type);
    return UPLL_RC_ERR_GENERIC;
  }

  key_root_t *root_key = reinterpret_cast<key_root_t*>
      (ConfigKeyVal::Malloc(sizeof(key_root_t)));

  okey = new ConfigKeyVal(UNC_KT_ROOT, IpctSt::kIpcStKeyRoot,
                          root_key, NULL);
  SET_USER_DATA(okey, ikey);
  return UPLL_RC_SUCCESS;
}

upll_rc_t FlowListMoMgr::SetValidAudit(ConfigKeyVal *&ikey) {
  UPLL_FUNC_TRACE;
  val_flowlist_t *val = reinterpret_cast
      <val_flowlist_t *>(GetVal(ikey));
  if (NULL == val) {
    UPLL_LOG_DEBUG("val is NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  val->cs_attr[0] = UNC_CS_APPLIED;
  val->cs_row_status = UNC_CS_APPLIED;
  return UPLL_RC_SUCCESS;
}

//  Increments the ref_count by 1 for given flowlist_name and
//  ctrlr_name in the given datatype.
upll_rc_t FlowListMoMgr::UpdateRefCountInCtrlrTbl(ConfigKeyVal *ikey,
     DalDmlIntf *dmi, upll_keytype_datatype_t dt_type,
     TcConfigMode config_mode, string vtn_name) {
  UPLL_FUNC_TRACE;

  if (!ikey || !(ikey->get_key())) {
    UPLL_LOG_DEBUG("Input key is Empty");
    return UPLL_RC_ERR_GENERIC;
  }

  key_user_data *tuser_data  = reinterpret_cast<key_user_data_t *>
                               (ikey->get_user_data());
  if (!tuser_data) {
    UPLL_LOG_DEBUG("UserData is NULL");
    return UPLL_RC_ERR_GENERIC;
  }

  void *tkey = ikey->get_key();
  void *p = NULL;
  const uudst::kDalTableIndex tbl_index = GetTable(CTRLRTBL, dt_type);
  DalBindInfo *db_info = new DalBindInfo(tbl_index);
  BindInfo *binfo = flowlist_controller_bind_info;

  //  Bind match flowlist_name
  uint8_t array_index = 0;
  uint64_t indx = binfo[array_index].index;
  p = reinterpret_cast<void *>(reinterpret_cast<char *>(tkey)
      + binfo[array_index].offset);
  db_info->BindMatch(indx, binfo[array_index].app_data_type,
                     binfo[array_index].array_size, p);

  //  Bind match ctrlr_name
  ++array_index;
  indx = binfo[array_index].index;
  p = reinterpret_cast<void *>(reinterpret_cast<char *>(tuser_data)
      + binfo[array_index].offset);
  db_info->BindMatch(indx, binfo[array_index].app_data_type,
                     binfo[array_index].array_size, p);

  //  Incrment the ref_count by 1 for the matched flowlist_name and ctrlr_name
  //  for the given datatype
  std::string query_string;
  if (dt_type == UPLL_DT_CANDIDATE) {
    query_string = QUERY_FF_CAND_REF_COUNT_UPDATE;
  } else if (dt_type == UPLL_DT_AUDIT) {
    query_string = QUERY_FF_AUD_REF_COUNT_UPDATE;
  } else if (dt_type == UPLL_DT_IMPORT) {
    query_string = QUERY_FF_IMP_REF_COUNT_UPDATE;
  }

  uint8_t *vtnname = NULL;
  if (!vtn_name.empty()) {
    vtnname = reinterpret_cast<uint8_t *>(
      const_cast<char *>(vtn_name.c_str()));
  }
  upll_rc_t result_code = DalToUpllResCode(
         dmi->ExecuteAppQuery(query_string, dt_type, tbl_index,
                              db_info, UNC_OP_UPDATE, config_mode,
                              vtnname));
  DELETE_IF_NOT_NULL(db_info);
  return result_code;
}

upll_rc_t FlowListMoMgr::GetOperation(uuc::UpdateCtrlrPhase phase,
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
    // Return success
  } else {
    return UPLL_RC_ERR_GENERIC;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t FlowListMoMgr::CopyKeyToVal(ConfigKeyVal *ikey,
                                 ConfigKeyVal *&okey) {
  UPLL_FUNC_TRACE;
  if (!ikey)
    return UPLL_RC_ERR_GENERIC;
  upll_rc_t result_code = GetChildConfigKey(okey, NULL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetChildConfigKey failed");
    return result_code;
  }
  val_rename_flowlist *val = reinterpret_cast<val_rename_flowlist_t *>(
                          ConfigKeyVal::Malloc(sizeof(val_rename_flowlist)));
  // Note: Validate message is take care of validate the key part
  key_flowlist_t *key = reinterpret_cast<key_flowlist_t *>(ikey->get_key());
  uuu::upll_strncpy(val->flowlist_newname, key->flowlist_name,
                    (kMaxLenFlowListName+1));
  val->valid[UPLL_CTRLR_VTN_NAME_VALID] = UNC_VF_VALID;
  okey->SetCfgVal(new ConfigVal(IpctSt::kIpcStValRenameFlowlist, val));
  return UPLL_RC_SUCCESS;
}

upll_rc_t FlowListMoMgr::GetControllerDomainSpan(
    ConfigKeyVal *ikey, upll_keytype_datatype_t dt_type,
    DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code;
  DbSubOp dbop = {kOpReadExist|kOpReadMultiple, kOpMatchNone,
                  kOpInOutCtrlr};

  result_code = ReadConfigDB(ikey, dt_type, UNC_OP_READ, dbop, dmi, CTRLRTBL);
  return result_code;
}

upll_rc_t FlowListMoMgr::GetDomainsForController(
    ConfigKeyVal *ckv_drvr,
    ConfigKeyVal *&ctrlr_ckv,
    DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = GetChildConfigKey(ctrlr_ckv, ckv_drvr);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_INFO("GetChildConfigKey failed %d", result_code);
    return result_code;
  }
  DbSubOp dbop = { kOpReadSingle, kOpMatchCtrlr, kOpInOutCtrlr };
  return ReadConfigDB(ctrlr_ckv, UPLL_DT_RUNNING, UNC_OP_READ, dbop, dmi,
                             CTRLRTBL);
}

upll_rc_t FlowListMoMgr::UpdateRefCountInScratchTbl(
    ConfigKeyVal *ikey,
    DalDmlIntf *dmi, upll_keytype_datatype_t dt_type,
    unc_keytype_operation_t op,
    TcConfigMode config_mode, string vtn_name,
    uint32_t count) {
  UPLL_FUNC_TRACE;

  if (!ikey || !(ikey->get_key())) {
    UPLL_LOG_DEBUG("Input key is Empty");
    return UPLL_RC_ERR_GENERIC;
  }

  key_user_data *tuser_data  = reinterpret_cast<key_user_data_t *>
                               (ikey->get_user_data());
  if (!tuser_data) {
    UPLL_LOG_DEBUG("UserData is NULL");
    return UPLL_RC_ERR_GENERIC;
  }

  void *tkey = ikey->get_key();
  DalBindInfo *db_info = new DalBindInfo(uudst::kDbiFlScratchTbl);

  //  Bind match flowlist_name
  db_info->BindMatch(uudst::fl_scratch::kDbiFlowListName,
                     uud::kDalChar,
                     (kMaxLenFlowListName + 1),
                     reinterpret_cast<void *>(reinterpret_cast<key_flowlist_t*>
                                              (tkey)->flowlist_name));

  //  Bind match ctrlr_name
  db_info->BindMatch(uudst::fl_scratch::kDbiCtrlrName,
                     uud::kDalChar,
                     (kMaxLenCtrlrId + 1),
                     &(tuser_data->ctrlr_id));
  UPLL_LOG_DEBUG("vtn name from ff %s", vtn_name.c_str());
  uint8_t *vtnname = NULL;
  if (!vtn_name.empty()) {
    vtnname = reinterpret_cast<uint8_t *>(
      const_cast<char *>(vtn_name.c_str()));
  } else {
    UPLL_LOG_DEBUG("Invalid vtn name");
    DELETE_IF_NOT_NULL(db_info);
    return UPLL_RC_ERR_GENERIC;
  }

  UPLL_LOG_DEBUG("vtn name from ff %s", vtnname);

  // Bind match vtn_name
  db_info->BindMatch(uudst::fl_scratch::kDbiVtnName,
                     uud::kDalChar,
                     (kMaxLenVtnName + 1),
                     vtnname);

  //  Incrment the ref_count by 1 for the matched policingprofile_name
  //  and ctrlr_name for the given datatype
  std::stringstream ss;
  ss << count;
  std::string query_string;
  if (op == UNC_OP_CREATE) {
    query_string += "UPDATE ca_fl_scratch_tbl SET ref_count = ref_count + ";
    query_string += (ss.str());
    query_string += " WHERE flowlist_name = ? AND ctrlr_name = ?"\
                     " AND vtn_name = ?";
  } else if (op == UNC_OP_DELETE) {
    query_string += "UPDATE ca_fl_scratch_tbl SET ref_count = ref_count - ";
    query_string += (ss.str());
    query_string += " WHERE flowlist_name = ? AND ctrlr_name = ?"\
                     " AND vtn_name = ?";
  } else {
    UPLL_LOG_DEBUG("Invalid operation");
    DELETE_IF_NOT_NULL(db_info);
    return UPLL_RC_ERR_GENERIC;
  }
  upll_rc_t result_code = DalToUpllResCode(
         dmi->ExecuteAppQuery(query_string, dt_type, uudst::kDbiFlScratchTbl,
                              db_info, UNC_OP_UPDATE, config_mode,
                              vtnname));
  DELETE_IF_NOT_NULL(db_info);
  return result_code;
}

upll_rc_t FlowListMoMgr::InsertRecInScratchTbl(ConfigKeyVal *ikey,
     DalDmlIntf *dmi, upll_keytype_datatype_t dt_type,
     unc_keytype_operation_t op,
     TcConfigMode config_mode, string vtn_name,
     uint32_t count) {
  UPLL_FUNC_TRACE;

  if (!ikey || !(ikey->get_key())) {
    UPLL_LOG_DEBUG("Input key is Empty");
    return UPLL_RC_ERR_GENERIC;
  }

  key_user_data *tuser_data  = reinterpret_cast<key_user_data_t *>
                               (ikey->get_user_data());
  if (!tuser_data) {
    UPLL_LOG_DEBUG("UserData is NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  void *tkey = ikey->get_key();
  DalBindInfo *db_info = new DalBindInfo(uudst::kDbiFlScratchTbl);

  //  Bind Input flowlist_name
  db_info->BindInput(uudst::fl_scratch::kDbiFlowListName,
                     uud::kDalChar,
                     (kMaxLenFlowListName + 1),
                     reinterpret_cast<void *>(reinterpret_cast<key_flowlist_t*>
                                              (tkey)->flowlist_name));

  //  Bind Input ctrlr_name
  db_info->BindInput(uudst::fl_scratch::kDbiCtrlrName,
                     uud::kDalChar,
                     (kMaxLenCtrlrId + 1),
                     &(tuser_data->ctrlr_id));

  uint8_t *vtnname = NULL;
  if (!vtn_name.empty()) {
    vtnname = reinterpret_cast<uint8_t *>(
      const_cast<char *>(vtn_name.c_str()));
  } else {
    UPLL_LOG_DEBUG("Invalid vtn name");
    DELETE_IF_NOT_NULL(db_info);
    return UPLL_RC_ERR_GENERIC;
  }


  // Bind Input vtn_name
  db_info->BindInput(uudst::fl_scratch::kDbiVtnName,
                     uud::kDalChar,
                     (kMaxLenVtnName + 1),
                     vtnname);

  //  Incrment the ref_count by 1 for the matched policingprofile_name
  //  and ctrlr_name for the given datatype
  std::stringstream ss;
  ss << count;
  std::string query_string;
  if (op == UNC_OP_CREATE) {
    query_string += "INSERT INTO ca_fl_scratch_tbl "\
                     "(flowlist_name, ctrlr_name, vtn_name, ref_count) "\
                     "VALUES (?, ?, ?, ";
    query_string += (ss.str());
    query_string += ")";
  } else {
    query_string += "INSERT INTO ca_fl_scratch_tbl "\
                     "(flowlist_name, ctrlr_name, vtn_name, ref_count) "\
                     "VALUES (?, ?, ?, -";
    query_string += (ss.str());
    query_string += ")";
  }

  upll_rc_t result_code = DalToUpllResCode(
         dmi->ExecuteAppQuery(query_string, dt_type, uudst::kDbiFlScratchTbl,
                              db_info, UNC_OP_CREATE, config_mode,
                              vtnname));
  DELETE_IF_NOT_NULL(db_info);
  return result_code;
}

upll_rc_t FlowListMoMgr::ComputeRefCountInScratchTbl(
    ConfigKeyVal *ikey,
    DalDmlIntf *dmi, upll_keytype_datatype_t dt_type,
    TcConfigMode config_mode, string vtn_name,
    int &ref_count) {
  UPLL_FUNC_TRACE;

  if (!ikey || !(ikey->get_key())) {
    UPLL_LOG_DEBUG("Input key is Empty");
    return UPLL_RC_ERR_GENERIC;
  }
  key_user_data *tuser_data  = reinterpret_cast<key_user_data_t *>
      (ikey->get_user_data());
  if (!tuser_data) {
    UPLL_LOG_DEBUG("UserData is NULL");
    return UPLL_RC_ERR_GENERIC;
  }

  void *tkey = ikey->get_key();
  DalBindInfo *db_info = new DalBindInfo(uudst::kDbiFlScratchTbl);
  //  Bind Input flowlist_name
  db_info->BindMatch(uudst::fl_scratch::kDbiFlowListName,
                     uud::kDalChar,
                     (kMaxLenFlowListName + 1),
                     reinterpret_cast<void *>(reinterpret_cast<key_flowlist_t*>
                                              (tkey)->flowlist_name));

  //  Bind Input ctrlr_name
  db_info->BindMatch(uudst::fl_scratch::kDbiCtrlrName,
                     uud::kDalChar,
                     (kMaxLenCtrlrId + 1),
                     &(tuser_data->ctrlr_id));

  std::string query_string;
  query_string = QUERY_SUM_FL_SCRATCH_REF_COUNT_WITH_CTRLR;
  uint8_t *vtnname = NULL;

  if (config_mode == TC_CONFIG_VTN) {
    if (!vtn_name.empty()) {
      vtnname = reinterpret_cast<uint8_t *>(
          const_cast<char *>(vtn_name.c_str()));
    } else {
      UPLL_LOG_DEBUG("vtn name is NULL");
      DELETE_IF_NOT_NULL(db_info);
      return UPLL_RC_ERR_GENERIC;
    }


    //  Bind Input vtn_name
    db_info->BindMatch(uudst::fl_scratch::kDbiVtnName,
                       uud::kDalChar,
                       (kMaxLenVtnName + 1),
                       vtnname);

    query_string = QUERY_READ_REF_COUNT_FL_SCRATCH_TBL;
  }

  int db_ref_count = 0;
  db_info->BindOutput(uudst::fl_scratch::kDbiRefCount,
                      uud::kDalUint32,
                      1,
                      &db_ref_count);
  //  Incrment the ref_count by 1 for the matched policingprofile_name
  //  and ctrlr_name for the given datatype
  upll_rc_t result_code = DalToUpllResCode(dmi->
                                 ExecuteAppQuerySingleRecord(query_string,
                                                             db_info));
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Reading single records failed %d", result_code);
    DELETE_IF_NOT_NULL(db_info);
    return result_code;
  }
  ref_count = db_ref_count;
  UPLL_LOG_DEBUG(" ref_count in scratch tbl : %d", ref_count);
  DELETE_IF_NOT_NULL(db_info);
  return result_code;
}

upll_rc_t FlowListMoMgr::ReadCtrlrTbl(
    ConfigKeyVal *&okey,
    DalDmlIntf *dmi, upll_keytype_datatype_t dt_type) {

  UPLL_FUNC_TRACE;

  DbSubOp dbop = { kOpReadMultiple, kOpMatchNone, kOpInOutCtrlr };
  upll_rc_t result_code = ReadConfigDB(okey, dt_type, UNC_OP_READ, dbop, dmi,
      CTRLRTBL);
  UPLL_LOG_DEBUG("ReadConfigDB returned %d", result_code);
  return result_code;
}

upll_rc_t FlowListMoMgr::ClearScratchTbl(
    TcConfigMode config_mode, string vtn_name,
    DalDmlIntf *dmi, bool is_abort) {

  UPLL_FUNC_TRACE;

  DalBindInfo *db_info = new DalBindInfo(uudst::kDbiFlScratchTbl);

  uint8_t *vtnname = NULL;
  std::string query_string = QUERY_DELETE_ALL_FL_SCRATCH_TBL;
  if (config_mode == TC_CONFIG_VTN) {
    if (!vtn_name.empty()) {
      vtnname = reinterpret_cast<uint8_t *>(
          const_cast<char *>(vtn_name.c_str()));
    } else {
      UPLL_LOG_DEBUG("Invalid vtn name");
      DELETE_IF_NOT_NULL(db_info);
      return UPLL_RC_ERR_GENERIC;
    }
    //  Bind Match vtn_name
    db_info->BindMatch(uudst::fl_scratch::kDbiVtnName,
                       uud::kDalChar,
                       (kMaxLenVtnName + 1),
                       vtnname);
    query_string = QUERY_DELETE_VTN_FL_SCRATCH_TBL;
  }
  upll_rc_t result_code = DalToUpllResCode(
      dmi->ExecuteAppQuery(query_string, UPLL_DT_CANDIDATE,
                           uudst::kDbiFlScratchTbl, db_info, UNC_OP_DELETE,
                           config_mode, vtnname));

  DELETE_IF_NOT_NULL(db_info);
  if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
    result_code = UPLL_RC_SUCCESS;
  }
  return result_code;
}

upll_rc_t FlowListMoMgr::RefCountSemanticCheck(
    const char* flowlist_name, DalDmlIntf *dmi,
    TcConfigMode config_mode, string vtn_name) {

  UPLL_FUNC_TRACE;

  upll_rc_t result_code = UPLL_RC_ERR_GENERIC;

  DalBindInfo *db_info = new DalBindInfo(uudst::kDbiFlowListCtrlrTbl);

  db_info->BindMatch(uudst::flowlist_ctrlr::kDbiFlowListName,
                     uud::kDalChar,
                     (kMaxLenFlowListName + 1),
                     flowlist_name);


  uint32_t ct_ref_count = 0;
  db_info->BindOutput(uudst::flowlist_ctrlr::kDbiRefCount,
                      uud::kDalUint32,
                      1,
                      &ct_ref_count);

  std::string query_string = QUERY_SUM_FL_CTRLR_REF_COUNT;

  result_code = DalToUpllResCode(
         dmi->ExecuteAppQuerySingleRecord(query_string, db_info));
  if ((UPLL_RC_SUCCESS != result_code) &&
      (UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code)) {
    UPLL_LOG_DEBUG("ExecuteAppQuery returned %d", result_code);
    DELETE_IF_NOT_NULL(db_info);
    return result_code;
  }

  DalBindInfo *st_db_info = new DalBindInfo(uudst::kDbiFlScratchTbl);
  st_db_info->BindMatch(uudst::fl_scratch::kDbiFlowListName,
                       uud::kDalChar,
                       (kMaxLenFlowListName + 1),
                       flowlist_name);

  uint32_t st_ref_count = 0;
  st_db_info->BindOutput(uudst::fl_scratch::kDbiRefCount,
                        uud::kDalUint32,
                        1,
                        &st_ref_count);

  query_string = QUERY_SUM_FL_SCRATCH_REF_COUNT;
  result_code = DalToUpllResCode(
         dmi->ExecuteAppQuerySingleRecord(query_string, st_db_info));
  if ((UPLL_RC_SUCCESS != result_code) &&
      (UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code)) {
    UPLL_LOG_DEBUG("ExecuteAppQuery returned %d", result_code);
    DELETE_IF_NOT_NULL(db_info);
    DELETE_IF_NOT_NULL(st_db_info);
    return result_code;
  }

  if (0 != (ct_ref_count + st_ref_count)) {
    UPLL_LOG_DEBUG("FlowList is matched");
    DELETE_IF_NOT_NULL(st_db_info);
    DELETE_IF_NOT_NULL(db_info);
    return UPLL_RC_ERR_CFG_SEMANTIC;
  }

  DELETE_IF_NOT_NULL(st_db_info);
  DELETE_IF_NOT_NULL(db_info);
  return UPLL_RC_ERR_NO_SUCH_INSTANCE;
}

upll_rc_t FlowListMoMgr::InstanceExistsInScratchTbl(
    ConfigKeyVal *ikey, TcConfigMode config_mode, string vtn_name,
    DalDmlIntf *dmi) {

  UPLL_FUNC_TRACE;


  ConfigKeyVal *ckv = NULL;
  upll_rc_t result_code = GetChildConfigKey(ckv, ikey);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetChildConfigKey failed %d", result_code);
    return result_code;
  }
  key_user_data *tuser_data  = reinterpret_cast<key_user_data_t *>
      (ckv->get_user_data());
  if (!tuser_data) {
    UPLL_LOG_DEBUG("UserData is NULL");
    DELETE_IF_NOT_NULL(ckv);
    return UPLL_RC_ERR_GENERIC;
  }

  void *tkey = ckv->get_key();

  DalBindInfo *db_info = new DalBindInfo(uudst::kDbiFlScratchTbl);
  //  Bind Match policingprofile_name
  db_info->BindMatch(uudst::fl_scratch::kDbiFlowListName,
                     uud::kDalChar,
                     (kMaxLenFlowListName + 1),
                     reinterpret_cast<void *>(reinterpret_cast
                                              <key_flowlist_t*>
                                              (tkey)->flowlist_name));

  //  Bind Match ctrlr_name
  db_info->BindMatch(uudst::fl_scratch::kDbiCtrlrName,
                     uud::kDalChar,
                     (kMaxLenCtrlrId + 1),
                     &(tuser_data->ctrlr_id));
  std::string query_string;
  query_string = QUERY_READ_NO_VTN_REF_COUNT_FL_SCRATCH_TBL;
  uint8_t *vtnname = NULL;
  if (config_mode == TC_CONFIG_VTN) {
    if (!vtn_name.empty()) {
      vtnname = reinterpret_cast<uint8_t *>(
          const_cast<char *>(vtn_name.c_str()));
    } else {
      UPLL_LOG_DEBUG("Invalid vtn name");
      DELETE_IF_NOT_NULL(db_info);
      DELETE_IF_NOT_NULL(ckv);
    }


    //  Bind Match vtn_name
    db_info->BindMatch(uudst::fl_scratch::kDbiVtnName,
                       uud::kDalChar,
                       (kMaxLenVtnName + 1),
                       vtnname);
    query_string = QUERY_READ_REF_COUNT_FL_SCRATCH_TBL;
  }

  result_code = DalToUpllResCode(
      dmi->ExecuteAppQuerySingleRecord(query_string, db_info));
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Reading multiple records failed %d", result_code);
    DELETE_IF_NOT_NULL(ckv);
    DELETE_IF_NOT_NULL(db_info);
    return result_code;
  }
  DELETE_IF_NOT_NULL(ckv);
  DELETE_IF_NOT_NULL(db_info);
  return UPLL_RC_SUCCESS;
}

upll_rc_t FlowListMoMgr::ClearVirtualKtDirtyInGlobal(DalDmlIntf *dmi) {
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

upll_rc_t FlowListMoMgr::ComputeCtrlrTblRefCountFromScratchTbl(
    ConfigKeyVal *ikey,
    DalDmlIntf *dmi, upll_keytype_datatype_t dt_type,
    TcConfigMode config_mode, string vtn_name) {

  UPLL_FUNC_TRACE;

  uint8_t *vtnname = NULL;
  if (config_mode == TC_CONFIG_VTN) {
    if (!vtn_name.empty()) {
      vtnname = reinterpret_cast<uint8_t *>(
          const_cast<char *>(vtn_name.c_str()));
    } else {
      UPLL_LOG_DEBUG("vtn name is NULL");
      return UPLL_RC_ERR_GENERIC;
    }
  }
  // If scratch tbl is not dirty then skip the scratch tbl computation
  if (!dmi->IsTableDirtyShallow(uudst::kDbiFlScratchTbl,
                                config_mode, vtnname)) {
    UPLL_LOG_DEBUG("No entries in scratch tbl");
    return UPLL_RC_SUCCESS;
  }
  ConfigKeyVal *ckv = NULL;
  upll_rc_t result_code = GetChildConfigKey(ckv, ikey);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetChildConfigKey failed %d", result_code);
    return result_code;
  }
  GET_USER_DATA(ckv);
// Read records from scratch tbl
  key_user_data *tuser_data  = reinterpret_cast<key_user_data_t *>
      (ckv->get_user_data());

  void *tkey = ckv->get_key();
  DalBindInfo *db_info = new DalBindInfo(uudst::kDbiFlScratchTbl);

  //  Bind Match policingprofile_name
  db_info->BindOutput(uudst::fl_scratch::kDbiFlowListName,
                      uud::kDalChar,
                      (kMaxLenFlowListName + 1),
                      reinterpret_cast<void *>(reinterpret_cast
                                               <key_flowlist_t*>
                                               (tkey)->flowlist_name));

  //  Bind Match ctrlr_name
  db_info->BindOutput(uudst::fl_scratch::kDbiCtrlrName,
                      uud::kDalChar,
                      (kMaxLenCtrlrId + 1),
                      &(tuser_data->ctrlr_id));

  std::string query_string;
  query_string = QUERY_READ_AND_SUM_REF_COUNT_FL_SCRATCH_TBL;
  if (config_mode == TC_CONFIG_VTN) {
    //  Bind Match vtn_name
    db_info->BindMatch(uudst::fl_scratch::kDbiVtnName,
                       uud::kDalChar,
                       (kMaxLenVtnName + 1),
                       vtnname);
    query_string = QUERY_READ_FL_SCRATCH_TBL_VTN_MODE;
  }
  int st_ref_count = 0;
  db_info->BindOutput(uudst::fl_scratch::kDbiRefCount,
                      uud::kDalUint32,
                      1,
                      &st_ref_count);
  DalCursor *dal_cursor_handle = NULL;
  result_code = DalToUpllResCode(dmi->ExecuteAppQueryMultipleRecords(
                 query_string, 0, db_info, &dal_cursor_handle));
  while (result_code == UPLL_RC_SUCCESS) {
    result_code = DalToUpllResCode(dmi->GetNextRecord(dal_cursor_handle));
    UPLL_LOG_DEBUG("GetNextRecord returned %d", result_code);
    if (UPLL_RC_SUCCESS == result_code) {
      if (st_ref_count == 0) {
        continue;
      }
      ConfigKeyVal *ctrlr_ckv = NULL;
      result_code = GetChildConfigKey(ctrlr_ckv, ckv);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("GetChildConfigkey failed %d", result_code);
        DELETE_IF_NOT_NULL(db_info);
        DELETE_IF_NOT_NULL(ckv);
        dmi->CloseCursor(dal_cursor_handle, false);
        return result_code;
      }

      DbSubOp dbop = { kOpReadSingle, kOpMatchCtrlr, kOpInOutNone };
      result_code = ReadConfigDB(ctrlr_ckv, dt_type, UNC_OP_READ, dbop, dmi,
                                 CTRLRTBL);
      if ((UPLL_RC_SUCCESS != result_code) &&
          (UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code)) {
        UPLL_LOG_DEBUG("ReadConfigDB failed %d", result_code);
        DELETE_IF_NOT_NULL(db_info);
        DELETE_IF_NOT_NULL(ckv);
        DELETE_IF_NOT_NULL(ctrlr_ckv);
        dmi->CloseCursor(dal_cursor_handle, false);
        return result_code;
      }
      if (UPLL_RC_SUCCESS == result_code) {
        val_flowlist_ctrl_t *val_ctrlr = reinterpret_cast
            <val_flowlist_ctrl_t *>(GetVal(ctrlr_ckv));
        unc_keytype_operation_t op;
        if (0 == st_ref_count + val_ctrlr->refcount) {
          op = UNC_OP_DELETE;
        } else {
          val_ctrlr->refcount += st_ref_count;
          val_ctrlr->valid[1] = UNC_VF_VALID;
          op = UNC_OP_UPDATE;
        }
        DbSubOp dbop1 = {kOpNotRead, kOpMatchCtrlr, kOpInOutNone};
        result_code = UpdateConfigDB(ctrlr_ckv, dt_type, op, dmi, &dbop1,
                                     config_mode, vtn_name, CTRLRTBL);
        if (UPLL_RC_SUCCESS != result_code) {
          UPLL_LOG_DEBUG("UpdateConfigDB failed %d", result_code);
          DELETE_IF_NOT_NULL(db_info);
          DELETE_IF_NOT_NULL(ckv);
          DELETE_IF_NOT_NULL(ctrlr_ckv);
          dmi->CloseCursor(dal_cursor_handle, false);
          return result_code;
        }
        if (op == UNC_OP_DELETE) {
          FlowListEntryMoMgr *fle_mgr =
              reinterpret_cast<FlowListEntryMoMgr *>
              (const_cast<MoManager *>(GetMoManager(
                          UNC_KT_FLOWLIST_ENTRY)));
          key_flowlist_t *key_fl = reinterpret_cast<key_flowlist_t *>
                                  (ctrlr_ckv->get_key());
          result_code = fle_mgr->AddFlowListToController(
              reinterpret_cast<char*>(key_fl->flowlist_name), dmi,
              reinterpret_cast<char*>(tuser_data->ctrlr_id), dt_type,
              UNC_OP_DELETE, config_mode, vtn_name);
          if (UPLL_RC_SUCCESS != result_code) {
            UPLL_LOG_DEBUG("AddFlowListToController Delete failed %d",
                            result_code);
            DELETE_IF_NOT_NULL(db_info);
            DELETE_IF_NOT_NULL(ckv);
            DELETE_IF_NOT_NULL(ctrlr_ckv);
            dmi->CloseCursor(dal_cursor_handle, false);
            return result_code;
          }
          result_code = UpdateConfigDB(ctrlr_ckv, dt_type, UNC_OP_DELETE,
                                       dmi, &dbop1, config_mode, vtn_name,
                                       RENAMETBL);
          if ((UPLL_RC_SUCCESS != result_code) &&
              (UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code)) {
            UPLL_LOG_INFO("UpdateConfigDB Failed %d", result_code);
            DELETE_IF_NOT_NULL(db_info);
            DELETE_IF_NOT_NULL(ckv);
            DELETE_IF_NOT_NULL(ctrlr_ckv);
            dmi->CloseCursor(dal_cursor_handle, false);
            return result_code;
          }
          if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code)
            result_code = UPLL_RC_SUCCESS;
        }
        DELETE_IF_NOT_NULL(ctrlr_ckv);
      } else {
        key_flowlist_t *key_fl = reinterpret_cast<key_flowlist_t *>
                                (ctrlr_ckv->get_key());
        result_code = CreateFlowListToController(
              reinterpret_cast<char*>(key_fl->flowlist_name), dmi,
              reinterpret_cast<char*>(tuser_data->ctrlr_id), dt_type,
              UNC_OP_CREATE, config_mode, vtn_name, true, st_ref_count);
        if ((UPLL_RC_SUCCESS != result_code) &&
            (UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code)) {
          UPLL_LOG_DEBUG("CreateFlowListToController failed %d", result_code);
          DELETE_IF_NOT_NULL(db_info);
          DELETE_IF_NOT_NULL(ckv);
          DELETE_IF_NOT_NULL(ctrlr_ckv);
          dmi->CloseCursor(dal_cursor_handle, false);
          return result_code;
        }
        if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE)
          result_code = UPLL_RC_SUCCESS;
        DELETE_IF_NOT_NULL(ctrlr_ckv);
      }
    } else if (UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
      UPLL_LOG_DEBUG("GetNextRecord failed");
      dmi->CloseCursor(dal_cursor_handle, false);
      DELETE_IF_NOT_NULL(db_info);
      DELETE_IF_NOT_NULL(ckv);
      return result_code;
    }
  }
  if (dal_cursor_handle) {
    dmi->CloseCursor(dal_cursor_handle, false);
  }
  DELETE_IF_NOT_NULL(db_info);
  DELETE_IF_NOT_NULL(ckv);
  return UPLL_RC_SUCCESS;
}

upll_rc_t FlowListMoMgr::DeleteChildrenPOM(ConfigKeyVal *ikey,
                      upll_keytype_datatype_t dt_type, DalDmlIntf *dmi,
                      TcConfigMode config_mode, string vtn_name) {
  UPLL_FUNC_TRACE;

  upll_rc_t result_code = UPLL_RC_ERR_GENERIC;
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
      return result_code;
    }
  }

  void *tkey = ikey->get_key();
  DalBindInfo *db_info = new DalBindInfo(uudst::kDbiFlScratchTbl);
  db_info->BindMatch(uudst::fl_scratch::kDbiFlowListName,
                     uud::kDalChar, (kMaxLenFlowListName + 1),
                     reinterpret_cast<void *>(reinterpret_cast
                     <key_flowlist_t*>(tkey)->flowlist_name));

  std::string query_string = QUERY_DELETE_FL_SCRATCH_TBL_MATCH_FL;
  uint8_t *vtnname = NULL;
  if (!vtn_name.empty()) {
    vtnname = reinterpret_cast<uint8_t *>(
        const_cast<char *>(vtn_name.c_str()));
  }
  result_code = DalToUpllResCode(
         dmi->ExecuteAppQuery(query_string, dt_type, uudst::kDbiFlScratchTbl,
                              db_info, UNC_OP_DELETE, config_mode,
                              vtnname));
  DELETE_IF_NOT_NULL(db_info);
  if ((UPLL_RC_SUCCESS != result_code) &&
      (UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code)) {
    return result_code;
  }
  return UPLL_RC_SUCCESS;
}
}  // namespace kt_momgr
}  // namespace upll
}  // namespace unc
