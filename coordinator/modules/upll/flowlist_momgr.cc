/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include "pfc/log.h"
#include "flowlist_momgr.hh"
#include "flowlist_entry_momgr.hh"
#include "policingprofile_entry_momgr.hh"
#include "vtn_flowfilter_entry_momgr.hh"
#include "unc/upll_errno.h"
#include "upll_validation.hh"
#include "unc/upll_ipc_enum.h"
#include "ctrlr_capa_defines.hh"
#include "upll_log.hh"
namespace unc {
namespace upll {
namespace kt_momgr {

#define  NUM_KEY_MAIN_COL    3
#define  NUM_KEY_CTRL_COL    4
#define  NUM_KEY_RENAME_COL  2
#define  FLOWLIST_RENAME 0x01

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
    CFG_VAL,
    offsetof(val_flowlist_ctrl, flags),
    uud::kDalUint8, 1}
};

BindInfo FlowListMoMgr::rename_flowlist_ctrlr_tbl[] = {
  { uudst::flowlist_ctrlr::kDbiFlowListName,
    CFG_MATCH_KEY,
    offsetof(key_flowlist_t, flowlist_name),
    uud::kDalChar,
    (kMaxLenFlowListName + 1) },
  { uudst::flowlist_ctrlr::kDbiCtrlrName,
    CFG_MATCH_KEY,
    offsetof(key_user_data_t, ctrlr_id),
    uud::kDalChar,
    (kMaxLenCtrlrId + 1) },
  { uudst::flowlist_ctrlr::kDbiFlowListName,
    CFG_INPUT_KEY,
    offsetof(key_rename_vnode_info_t, new_flowlist_name),
    uud::kDalChar,
    (kMaxLenFlowListName + 1) },
  { uudst::flowlist_ctrlr::kDbiFlags,
    CFG_VAL,
    offsetof(val_flowlist_ctrl, flags),
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
  table = new Table *[ntable];

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
    if (uudst::flowlist::kDbiIpType == indx) {
      valid = &(reinterpret_cast<val_flowlist_t*>
                (val))->valid[UPLL_IDX_IP_TYPE_FL];
    } else {
       valid = NULL;
    }
  } else if (tbl == RENAMETBL) {
    valid = NULL;
  } else if (tbl == CTRLRTBL) {
    if (uudst::flowlist_ctrlr::kDbiValidIpType == indx) {
      valid = &(reinterpret_cast<val_flowlist_ctrl*>(val))->valid[0];
    } else if (uudst::flowlist_ctrlr::kDbiRefCount == indx) {
      valid = &(reinterpret_cast<val_flowlist_ctrl*>(val))->valid[1];
    } else {
      valid = NULL;
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
    free(flowlist_key);
    return UPLL_RC_ERR_GENERIC;
  }

  if (okey) {
    if (okey->get_key_type() != UNC_KT_FLOWLIST)
      return UPLL_RC_ERR_GENERIC;
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
  if (!okey)
  okey = new ConfigKeyVal(UNC_KT_FLOWLIST, IpctSt::kIpcStKeyFlowlist,
                          flowlist_key, NULL);

  SET_USER_DATA(okey, parent_key);
  return result_code;
}

upll_rc_t FlowListMoMgr::GetRenamedUncKey(ConfigKeyVal *ctrlr_key,
                                          upll_keytype_datatype_t dt_type,
                                          DalDmlIntf *dmi, uint8_t *ctrlr_id) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code;
  ConfigKeyVal *unc_key = NULL;
  key_flowlist_t *ctrlr_flowlist_key =
                  reinterpret_cast<key_flowlist_t *>(ctrlr_key->get_key());
  if (NULL == ctrlr_flowlist_key) return UPLL_RC_ERR_GENERIC;

  DbSubOp dbop = { kOpReadSingle, kOpMatchCtrlr, kOpInOutNone };


  val_rename_flowlist_t *rename_flowlist =
  reinterpret_cast<val_rename_flowlist_t*>
  (ConfigKeyVal::Malloc(sizeof(val_rename_flowlist_t)));
  uuu::upll_strncpy(rename_flowlist->flowlist_newname,
               ctrlr_flowlist_key->flowlist_name,
               (kMaxLenFlowListName + 1));
  rename_flowlist->valid[UPLL_IDX_RENAME_FLOWLIST_RFL] = UNC_VF_VALID;

  result_code = GetChildConfigKey(unc_key, NULL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetChildConfigKey returns failure result_code:%d",
      result_code);
    free(rename_flowlist);
    return result_code;
  }
  unc_key->AppendCfgVal(IpctSt::kIpcStValRenameFlowlist, rename_flowlist);

  SET_USER_DATA_CTRLR(unc_key, ctrlr_id);

  result_code = ReadConfigDB(unc_key, dt_type, UNC_OP_READ, dbop, dmi,
                             RENAMETBL);

  if (result_code == UPLL_RC_SUCCESS) {
    key_flowlist_t *flowlist_key =
    reinterpret_cast<key_flowlist_t *>(unc_key->get_key());
    uuu::upll_strncpy(ctrlr_flowlist_key->flowlist_name,
                 flowlist_key->flowlist_name,
                 (kMaxLenFlowListName + 1));
  }
  delete unc_key;
  return result_code;
}

upll_rc_t FlowListMoMgr::GetRenamedControllerKey(
    ConfigKeyVal *&ikey, upll_keytype_datatype_t dt_type, DalDmlIntf *dmi,
    controller_domain *ctrlr_dom) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_ERR_GENERIC;
  ConfigKeyVal *okey = NULL;
  uint8_t rename = 0;
  if (UPLL_RC_SUCCESS != IsRenamed(ikey, dt_type, dmi, rename)) {
    UPLL_LOG_DEBUG("Rename check for input ConfigKeyval is failed");
    return UPLL_RC_ERR_GENERIC;
  }
  if (!rename) {
    UPLL_LOG_DEBUG("Key is not Renamed");
    return UPLL_RC_SUCCESS;
  }
  /* FlowList renamed */
  key_flowlist_t *ctrlr_key = reinterpret_cast<key_flowlist_t*>
                              (ConfigKeyVal::Malloc(sizeof(key_flowlist_t)));
  if (rename & FLOWLIST_RENAME) {
    result_code = GetChildConfigKey(okey, ikey);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("GetChildConfigKey returns error, result_code:%d"
        , result_code);
      free(ctrlr_key);
      return result_code;
    }
    if (ctrlr_dom != NULL)
    SET_USER_DATA_CTRLR_DOMAIN(okey, *ctrlr_dom);
    DbSubOp dbop = { kOpReadSingle, kOpMatchCtrlr, kOpInOutFlag };
    ReadConfigDB(okey, dt_type, UNC_OP_READ, dbop, dmi, RENAMETBL);
    val_rename_flowlist_t *rename_val =
         reinterpret_cast<val_rename_flowlist_t *>(GetVal(okey));
    if (!rename_val
        || (rename_val->valid[UPLL_IDX_RENAME_FLOWLIST_RFL] != UNC_VF_VALID)) {
      if (okey) delete okey;
      if (ctrlr_key) free(ctrlr_key);
      return UPLL_RC_ERR_GENERIC;
    }
    uuu::upll_strncpy(ctrlr_key->flowlist_name,
                 rename_val->flowlist_newname,
                 (kMaxLenFlowListName + 1));
    delete okey;
  }
  free(ikey->get_key());
  ikey->SetKey(IpctSt::kIpcStKeyFlowlist,
               reinterpret_cast<void *> (ctrlr_key));

  return UPLL_RC_SUCCESS;
}

upll_rc_t FlowListMoMgr::UpdateMo(IpcReqRespHeader *req, ConfigKeyVal *ikey,
                                  DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  UPLL_LOG_DEBUG("Update operation is not allowed for KT_FLOWLIST");
  return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_KT;
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
                                       DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  // DbSubOp dbop = NULL;

  if (NULL == configkey || ctrl_id == NULL) return result_code;

  SET_USER_DATA_CTRLR(configkey, ctrl_id)
  DbSubOp dbop = { kOpReadExist, kOpMatchNone, kOpInOutNone };
  // Validate if Controller Id exists in the ImportDB or not
  result_code = UpdateConfigDB(configkey, UPLL_DT_RUNNING, UNC_OP_READ, dmi,
                               &dbop, CTRLRTBL);
  if (UPLL_RC_ERR_INSTANCE_EXISTS != result_code) {
    UPLL_LOG_DEBUG("The given Controller ID  does not exist");
    return result_code;
  }
  // Read the Configuration from the IMPORT
  DbSubOp dbop1 = { kOpReadMultiple, kOpMatchNone, kOpInOutNone };
  result_code = ReadConfigDB(configkey, UPLL_DT_IMPORT, UNC_OP_READ, dbop1, dmi,
                             MAINTBL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("Unable to read the ImportConfiguration");
    return result_code;
  }
  while (configkey != NULL) {
    // Check whether the configuration exists in the Running configuration or
    // not
    // if exists then return an error
    result_code = UpdateConfigDB(configkey, UPLL_DT_RUNNING, UNC_OP_READ, dmi,
                                 MAINTBL);
    if (UPLL_RC_ERR_INSTANCE_EXISTS == result_code)
      return UPLL_RC_ERR_MERGE_CONFLICT;
    configkey = configkey->get_next_cfg_key_val();
  }
  return result_code;
}
upll_rc_t FlowListMoMgr::IsReferenced(ConfigKeyVal *ikey,
                                      upll_keytype_datatype_t dt_type,
                                      DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  key_flowlist_t *fl_key = reinterpret_cast<key_flowlist_t *>
      (ikey->get_key());
  ConfigKeyVal *tmp_ckv = NULL;
  result_code = GetChildConfigKey(tmp_ckv, ikey);
  if (UPLL_RC_SUCCESS != result_code) {
    return result_code;
  }
  DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutNone };
  result_code = ReadConfigDB(tmp_ckv, dt_type, UNC_OP_READ,
                             dbop, dmi, CTRLRTBL);
  if (UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
    if (UPLL_RC_SUCCESS == result_code)
      result_code = UPLL_RC_ERR_INSTANCE_EXISTS;
    UPLL_LOG_DEBUG("IsRef failed %d", result_code);
    if (tmp_ckv) delete tmp_ckv;
    return result_code;
  }
  if (tmp_ckv) delete tmp_ckv;
  PolicingProfileEntryMoMgr *mgr =
    reinterpret_cast<PolicingProfileEntryMoMgr *>(const_cast<MoManager*>
            (GetMoManager(UNC_KT_POLICING_PROFILE_ENTRY)));
  if (NULL == mgr) {
    return UPLL_RC_ERR_GENERIC;
  }
  result_code = mgr->IsFlowlistConfigured(reinterpret_cast<const char *>
      (fl_key->flowlist_name), dmi);
  if (UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
    if (UPLL_RC_SUCCESS == result_code)
      result_code = UPLL_RC_ERR_INSTANCE_EXISTS;
    UPLL_LOG_DEBUG("IsRef failed %d", result_code);
    return result_code;
  }
  VtnFlowFilterEntryMoMgr *vtn_mgr =
    reinterpret_cast<VtnFlowFilterEntryMoMgr *>(const_cast<MoManager*>
            (GetMoManager(UNC_KT_VTN_FLOWFILTER_ENTRY)));
  if (NULL == vtn_mgr) {
    return UPLL_RC_ERR_GENERIC;
  }
  result_code = vtn_mgr->IsFlowListConfigured(reinterpret_cast<const char *>
      (fl_key->flowlist_name), dmi);
  if (UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
    if (UPLL_RC_SUCCESS == result_code)
      result_code = UPLL_RC_ERR_INSTANCE_EXISTS;
    UPLL_LOG_DEBUG("IsRef failed %d", result_code);
    return result_code;
  }
  key_flowlist_t *key_fle = reinterpret_cast
    <key_flowlist_t *>(ikey->get_key());
  PolicingProfileEntryMoMgr *ppe_mgr =
    reinterpret_cast<PolicingProfileEntryMoMgr *>(const_cast<MoManager *>
    (GetMoManager(UNC_KT_POLICING_PROFILE_ENTRY)));
  if (NULL == ppe_mgr) {
    return UPLL_RC_ERR_GENERIC;
  }
  result_code = ppe_mgr->IsFlowListMatched(reinterpret_cast<const char *>
    (key_fle->flowlist_name), dt_type, dmi);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("IsFlowListMatched failed from ppe %d", result_code);
    return result_code;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t FlowListMoMgr::SwapKeyVal(ConfigKeyVal *ikey, ConfigKeyVal *&okey,
                                    DalDmlIntf *dmi, uint8_t *ctrlr) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  // ConfigVal *tmp1;
  if (okey != NULL) {
    UPLL_LOG_DEBUG("Output ConfigKey should be NULL");
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
  if (cfg_val == NULL) return UPLL_RC_ERR_BAD_REQUEST;
  val_rename_flowlist_t *tval =
      reinterpret_cast<val_rename_flowlist_t *>(cfg_val->get_val());

  /* The New Name and PFC name should not be same name */
  if (!strcmp(reinterpret_cast<char *>
       (reinterpret_cast<key_flowlist_t *>(ikey->get_key())->flowlist_name),
              reinterpret_cast<char *>(tval->flowlist_newname)))
    return UPLL_RC_ERR_GENERIC;
  key_flowlist_t *key_flowlist = reinterpret_cast<key_flowlist_t*>
      (ConfigKeyVal::Malloc(sizeof(key_flowlist_t)));

  if (tval->valid[0] == UNC_VF_VALID) {
    // checking the string is empty or not
    if (!strlen(reinterpret_cast<char *>(tval->flowlist_newname))) {
      if (key_flowlist !=NULL) free(key_flowlist);
      return UPLL_RC_ERR_GENERIC;
    }
    uuu::upll_strncpy(key_flowlist->flowlist_name,
           tval->flowlist_newname,
           (kMaxLenFlowListName + 1));
  }

  okey = new ConfigKeyVal(UNC_KT_FLOWLIST, IpctSt::kIpcStKeyFlowlist,
                          key_flowlist, NULL);
  if (NULL == okey) {
    UPLL_LOG_DEBUG("Memory allocation failure for ConfigKeyVal structure");
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
      }
    }
    if (!tmp1)
      return UPLL_RC_ERR_GENERIC;
    tmp1->set_user_data(tmp->get_user_data());
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
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  unc_keytype_configstatus_t status = UNC_CS_UNKNOWN;
  unc_keytype_configstatus_t  cs_status = UNC_CS_UNKNOWN;
  cs_status = (driver_result == 0) ? UNC_CS_APPLIED : UNC_CS_NOT_APPLIED;

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
  if (op == UNC_OP_CREATE) {
    // ctrlr_val_flowlist->valid[0] = UNC_VF_INVALID;
    /* update the  status in main tbl */
    switch (val_flowlist->cs_row_status) {
      case UNC_CS_UNKNOWN:
        status = cs_status;
        break;
      case UNC_CS_PARTAILLY_APPLIED:
        if (ctrlr_val_flowlist->cs_row_status == UNC_CS_NOT_APPLIED) {
          // if this vtn has caused it then to change to applied.
        }
        break;
      case UNC_CS_APPLIED:
      case UNC_CS_NOT_APPLIED:
      case UNC_CS_INVALID:
      default:
        status =
            (cs_status == UNC_CS_APPLIED) ? UNC_CS_PARTAILLY_APPLIED : status;
        break;
    }
    val_flowlist->cs_row_status = status;
    if ( UNC_VF_NOT_SOPPORTED == val_flowlist->valid[0] ) {
        val_flowlist->cs_attr[0] = UNC_CS_NOT_SUPPORTED;
    }
    if (UNC_VF_NOT_SOPPORTED == ctrlr_val_flowlist->valid[0]) {
      ctrlr_val_flowlist->cs_attr[0] = UNC_CS_NOT_SUPPORTED;
    }
  if ((UNC_VF_VALID == val_flowlist->valid[0]) ||
        (UNC_VF_VALID_NO_VALUE == val_flowlist->valid[0]))
      if (ctrlr_val_flowlist->valid[0] != UNC_VF_NOT_SOPPORTED) {
         ctrlr_val_flowlist->cs_attr[0] = cs_status;
         val_flowlist->cs_attr[0] = (uint8_t)val_flowlist->cs_row_status;
      }
  } else if (op == UNC_OP_UPDATE) {
    if (ctrlr_val_flowlist->valid[0] != UNC_VF_NOT_SOPPORTED)
        ctrlr_val_flowlist->cs_attr[0] = cs_status;
      else
        ctrlr_val_flowlist->cs_attr[0] = UNC_CS_NOT_SUPPORTED;
      val_flowlist->cs_attr[0] = (uint8_t)val_flowlist->cs_row_status;
  }
  return result_code;
}

upll_rc_t FlowListMoMgr::AddFlowListToController(char *flowlist_name,
                                                 DalDmlIntf *dmi,
                                                 char* ctrl_id,
                                                 unc_keytype_operation_t op) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  if (UNC_OP_CREATE == op || UNC_OP_UPDATE == op) {
    result_code = CreateFlowListToController(flowlist_name, dmi, ctrl_id, op);
  } else if (UNC_OP_DELETE == op) {
    result_code = DeleteFlowListToController(flowlist_name, dmi, ctrl_id, op);
  }
  return result_code;
}

upll_rc_t FlowListMoMgr::DeleteFlowListToController(char *flowlist_name,
                                                 DalDmlIntf *dmi,
                                                 char* ctrl_id,
                                                 unc_keytype_operation_t op) {
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
  DbSubOp dbop = {kOpReadSingle, kOpMatchCtrlr, kOpInOutNone};
  result_code = ReadConfigDB(okey,
                            UPLL_DT_CANDIDATE,
                            UNC_OP_READ,
                            dbop, dmi, CTRLRTBL);
  if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
    UPLL_LOG_DEBUG("No matching record found in Ctrlrtbl");
    return UPLL_RC_SUCCESS;
  } else if (UPLL_RC_SUCCESS == result_code) {
    UPLL_LOG_DEBUG("Matching records found in ctrlrtbl");
    val_flowlist_ctrl_t *ctrlr_val = reinterpret_cast
      <val_flowlist_ctrl_t *>(GetVal(okey));
    ctrlr_val->refcount -= 1;
    if (1 > ctrlr_val->refcount) {
      ctrlr_val->valid[1] = UNC_VF_VALID;
      result_code = UpdateConfigDB(okey, UPLL_DT_CANDIDATE, UNC_OP_DELETE, dmi,
                                   CTRLRTBL);
      if (UPLL_RC_SUCCESS != result_code) {
        UPLL_LOG_DEBUG("Delete from ctrlrtbl failed");
        return result_code;
      }
     fl_entry_del  = true;
    } else {
      ctrlr_val->valid[1] = UNC_VF_VALID;
      result_code = UpdateConfigDB(okey, UPLL_DT_CANDIDATE, UNC_OP_UPDATE, dmi,
                                 CTRLRTBL);
      if (UPLL_RC_SUCCESS != result_code) {
        UPLL_LOG_DEBUG("Update in ctrlrtbl failed");
        return result_code;
      }
    }
  } else {
    UPLL_LOG_DEBUG("ReadConfigDB failed in Ctrlrtbl");
    delete okey;
    return result_code;
  }
  if (fl_entry_del) {

  FlowListEntryMoMgr *mgr = reinterpret_cast<FlowListEntryMoMgr *>
    (const_cast<MoManager *> (GetMoManager(UNC_KT_FLOWLIST_ENTRY)));
  result_code = mgr->AddFlowListToController(
                      flowlist_name, dmi, ctrl_id, op);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Unable to update the controller table for flowlistentry");
    return result_code;
  }
} 
  return UPLL_RC_SUCCESS;
}

upll_rc_t FlowListMoMgr::CreateFlowListToController(char *flowlist_name,
                                                 DalDmlIntf *dmi,
                                                 char* ctrl_id,
                                                 unc_keytype_operation_t op) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *okey = NULL;
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
  DbSubOp dbop = {kOpReadSingle, kOpMatchCtrlr, kOpInOutNone};
  result_code = ReadConfigDB(okey,
                            UPLL_DT_CANDIDATE,
                            UNC_OP_READ,
                            dbop, dmi, CTRLRTBL);
  if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
    UPLL_LOG_DEBUG("No entry in ctrlr tbl");
    ConfigKeyVal *main_ckv = NULL;
    result_code = GetChildConfigKey(main_ckv, okey);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("GetChildConfigKey failed %d", result_code);
      return result_code;
    }
    DbSubOp dbop1 = {kOpReadSingle, kOpMatchNone, kOpInOutNone};
    result_code = ReadConfigDB(main_ckv,
                            UPLL_DT_CANDIDATE,
                            UNC_OP_READ,
                            dbop1, dmi, MAINTBL);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("ReadConfigDB in maintbl failed %d", result_code);
      return result_code;
    }
    val_flowlist_t *main_val = reinterpret_cast<val_flowlist_t *>
        (GetVal(main_ckv));
    if (NULL == main_val) {
      UPLL_LOG_DEBUG(" Main Val struct is NULL");
      return UPLL_RC_ERR_GENERIC;
    }
    val_flowlist_ctrl_t *ctrlr_val = reinterpret_cast<val_flowlist_ctrl_t *>
        (GetVal(okey));
    if (NULL == ctrlr_val) {
      UPLL_LOG_DEBUG(" Ctrlr Val struct is NULL");
      return UPLL_RC_ERR_GENERIC;
    }
    ctrlr_val->refcount = 1;
    ctrlr_val->valid[1] = UNC_VF_VALID;
    ctrlr_val->valid[UPLL_IDX_IP_TYPE_FL] =
        main_val->valid[UPLL_IDX_IP_TYPE_FL];
    if (ctrlr_val->valid[UPLL_IDX_IP_TYPE_FL] == UNC_VF_NOT_SOPPORTED) {
      ctrlr_val->valid[UPLL_IDX_IP_TYPE_FL] = UNC_VF_INVALID;
    }
    result_code = UpdateConfigDB(okey, UPLL_DT_CANDIDATE,
                                 UNC_OP_CREATE, dmi,
                                 CTRLRTBL);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("UpdateConfigDB failed to create in ctrlrtbl %d",
          result_code);
      return result_code;
    }
  } else if (UPLL_RC_SUCCESS == result_code) {
    UPLL_LOG_DEBUG("Entry Already exists in ctrlrtbl");
    val_flowlist_ctrl_t *ctrlr_val = reinterpret_cast<val_flowlist_ctrl_t *>
        (GetVal(okey));
    if (NULL == ctrlr_val) {
      UPLL_LOG_DEBUG(" Ctrlr Val struct is NULL");
      return UPLL_RC_ERR_GENERIC;
    }
    ctrlr_val->refcount += 1;
    ctrlr_val->valid[1] = UNC_VF_VALID;
    result_code = UpdateConfigDB(okey, UPLL_DT_CANDIDATE,
                                 UNC_OP_UPDATE, dmi,
                                 CTRLRTBL);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("UpdateConfigDB failed to create in ctrlrtbl %d",
          result_code);
      return result_code;
    }
  } else {
    UPLL_LOG_DEBUG("ReadConfig DB in ctrlrtbl failed %d", result_code);
  }
  FlowListEntryMoMgr *mgr = reinterpret_cast<FlowListEntryMoMgr *>
    (const_cast<MoManager *> (GetMoManager(UNC_KT_FLOWLIST_ENTRY)));
  result_code = mgr->AddFlowListToController(
                      flowlist_name, dmi, ctrl_id, op);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Unable to update the controller table for flowlistentry");
    delete okey;
    return result_code;
  }
  return result_code;
}

upll_rc_t FlowListMoMgr::TxUpdateController(unc_key_type_t keytype,
                                            uint32_t session_id,
                                            uint32_t config_id,
                                            uuc::UpdateCtrlrPhase phase,
                                            set<string> *affected_ctrlr_set,
                                            DalDmlIntf *dmi,
                                            ConfigKeyVal **err_ckv) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  DalResultCode dal_result = uud::kDalRcSuccess;
  ConfigKeyVal *req = NULL, *nreq = NULL, *ck_main = NULL;
  controller_domain ctrlr_dom;
  ctrlr_dom.ctrlr = NULL;
  ctrlr_dom.domain = NULL;
  DalCursor *dal_cursor_handle = NULL;
  IpcResponse resp;
  if (phase == uuc::kUpllUcpDelete) return UPLL_RC_SUCCESS;
  unc_keytype_operation_t op = (phase == uuc::kUpllUcpCreate)?UNC_OP_CREATE:
          ((phase == uuc::kUpllUcpUpdate)?UNC_OP_UPDATE:
          ((phase == uuc::kUpllUcpDelete2)?UNC_OP_DELETE:UNC_OP_INVALID));
  switch (op) {
    case UNC_OP_CREATE:
    case UNC_OP_DELETE:
      result_code = DiffConfigDB(UPLL_DT_CANDIDATE, UPLL_DT_RUNNING,
          op, req, nreq, &dal_cursor_handle, dmi, CTRLRTBL);
      break;
    case UNC_OP_UPDATE:
      // not supported by keytype
      // return success
      UPLL_LOG_TRACE(" Not supported operation");
      return UPLL_RC_SUCCESS;
    default:
      UPLL_LOG_TRACE(" Invalid operation");
      return UPLL_RC_ERR_GENERIC;
  }
  resp.header.clnt_sess_id = session_id;
  resp.header.config_id = config_id;
  while (result_code == UPLL_RC_SUCCESS) {
    // Get Next Record
    dal_result = dmi->GetNextRecord(dal_cursor_handle);
    result_code = DalToUpllResCode(dal_result);
    if (result_code != UPLL_RC_SUCCESS) {
      break;
    }
    ck_main = NULL;
    if ((op == UNC_OP_CREATE) || (op == UNC_OP_DELETE)) {
      result_code = DupConfigKeyVal(ck_main, req, MAINTBL);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("FlowListMoMgr:DupConfigKeyVal failed during TxUpdate.");
        return result_code;
      }

      GET_USER_DATA_CTRLR_DOMAIN(ck_main, ctrlr_dom);
      UPLL_LOG_DEBUG("ctrlr : %s; domain : %s", ctrlr_dom.ctrlr,
                     ctrlr_dom.domain);
      if (NULL == ctrlr_dom.ctrlr || NULL == ctrlr_dom.domain) {
        UPLL_LOG_INFO("Invalid controller/domain");
        result_code = UPLL_RC_ERR_GENERIC;
        if (ck_main) delete ck_main;
        break;
      }
      result_code = TxUpdateProcess(ck_main, &resp, op,
          dmi, &ctrlr_dom);
      if (result_code == UPLL_RC_SUCCESS) {
        affected_ctrlr_set->insert((const char *)ctrlr_dom.ctrlr);
      } else {
        UPLL_LOG_DEBUG("TxUpdateProcess error %d", result_code);
        *err_ckv = resp.ckv_data;
        if (ck_main) delete ck_main;
        break;
      }
    }
    if (ck_main) {
      delete ck_main;
      ck_main = NULL;
    }
  }
  if (nreq)
    delete nreq;
  if (req)
    delete req;
  if (dal_cursor_handle) {
    dmi->CloseCursor(dal_cursor_handle, true);
    dal_cursor_handle = NULL;
  }
  result_code = (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) ?
      UPLL_RC_SUCCESS:result_code;
  return result_code;
}

upll_rc_t FlowListMoMgr::TxUpdateProcess(ConfigKeyVal *ck_main,
     IpcResponse *ipc_resp, unc_keytype_operation_t op,
     DalDmlIntf *dmi, controller_domain *ctrlr_dom) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code;
  /* read from main table */
  ConfigKeyVal *dup_ckmain = ck_main;
  if (op == UNC_OP_CREATE)  {
    dup_ckmain = NULL;
    result_code = GetChildConfigKey(dup_ckmain, ck_main);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Returning error %d\n", result_code);
      if (dup_ckmain) delete dup_ckmain;
      return result_code;
    }
    DbSubOp dbop = {kOpReadSingle, kOpMatchNone, kOpInOutCs};
    result_code = ReadConfigDB(dup_ckmain, UPLL_DT_CANDIDATE,
                               UNC_OP_READ, dbop, dmi, MAINTBL);
    if (UPLL_RC_SUCCESS != result_code) {
      string s(dup_ckmain->ToStrAll());
      UPLL_LOG_DEBUG("%s flowlist read failed from candidatedb (%d)",
                     s.c_str(), result_code);
      delete dup_ckmain;
      return result_code;
    }
  }
  /* Get renamed key if key is renamed */
  result_code =  GetRenamedControllerKey(dup_ckmain, UPLL_DT_CANDIDATE,
                                         dmi, ctrlr_dom);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Failed to get the Renamed ControllerKey");
    return result_code;
  }
  result_code = SendIpcReq(ipc_resp->header.clnt_sess_id,
                           ipc_resp->header.config_id, op,
                           UPLL_DT_CANDIDATE,
                           dup_ckmain, ctrlr_dom, ipc_resp);
  if (result_code == UPLL_RC_ERR_RESOURCE_DISCONNECTED) {
    result_code = UPLL_RC_SUCCESS;
    UPLL_LOG_DEBUG("controller disconnected error proceed with commit");
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

upll_rc_t FlowListMoMgr::TxCopyCandidateToRunning(
    unc_key_type_t keytype, CtrlrCommitStatusList *ctrlr_commit_status,
    DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  DalResultCode db_result;
  unc_keytype_operation_t op[] = { UNC_OP_CREATE, UNC_OP_DELETE };
  int nop = sizeof(op) / sizeof(op[0]);
  ConfigKeyVal *flowlist_key = NULL, *req = NULL, *nreq = NULL;
  DalCursor *cfg1_cursor = NULL;
  uint8_t *ctrlr_id = NULL;
  map<string, int> ctrlr_result;
  CtrlrCommitStatusList::iterator ccsListItr;
  CtrlrCommitStatus *ccStatusPtr;

  if ((ctrlr_commit_status == NULL) || (dmi == NULL)) {
    UPLL_LOG_DEBUG("Insufficient parameters");
    return UPLL_RC_ERR_GENERIC;
  }
  for (ccsListItr = ctrlr_commit_status->begin();
       ccsListItr != ctrlr_commit_status->end(); ++ccsListItr) {
    ccStatusPtr = *ccsListItr;
    ctrlr_id = reinterpret_cast<uint8_t *>
        (const_cast<char*>(ccStatusPtr->ctrlr_id.c_str()));
    ctrlr_result[ccStatusPtr->ctrlr_id] = ccStatusPtr->upll_ctrlr_result;
    if (ccStatusPtr->upll_ctrlr_result != UPLL_RC_SUCCESS) {
      for (ConfigKeyVal *ck_err = ccStatusPtr->err_ckv; ck_err != NULL; ck_err =
           ck_err->get_next_cfg_key_val()) {
        if (ck_err->get_key_type() != keytype) continue;
        result_code = GetRenamedUncKey(ck_err, UPLL_DT_CANDIDATE, dmi,
                                       ctrlr_id);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("Unable to get the Renamed UncKey %d", result_code);
          return result_code;
        }
      }
    }
  }

  for (int i = 0; i < nop; i++) {
    // Update the Main table
    if (op[i] != UNC_OP_UPDATE) {
      result_code = DiffConfigDB(UPLL_DT_CANDIDATE, UPLL_DT_RUNNING, op[i],
                                 req, nreq, &cfg1_cursor, dmi, MAINTBL);
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
    // Update the controller table
    result_code = DiffConfigDB(UPLL_DT_CANDIDATE, UPLL_DT_RUNNING, op[i], req,
                               nreq, &cfg1_cursor, dmi, CTRLRTBL);
    ConfigKeyVal *flowlist_ctrlr_key = NULL;
    while (result_code == UPLL_RC_SUCCESS) {
      db_result = dmi->GetNextRecord(cfg1_cursor);
      result_code = DalToUpllResCode(db_result);
      if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
        result_code = UPLL_RC_SUCCESS;
        break;
      }
      if (op[i] == UNC_OP_CREATE) {
        DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutFlag };
        result_code = GetChildConfigKey(flowlist_key, req);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("GetChildConfigKey is failed resultcode=%d",
                         result_code);
          return result_code;
        }
        result_code = ReadConfigDB(flowlist_key, UPLL_DT_CANDIDATE,
                                   UNC_OP_READ, dbop, dmi, MAINTBL);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("Unable to read configuration from CandidateDb");
          delete flowlist_key;
          return result_code;
        }

        result_code = DupConfigKeyVal(flowlist_ctrlr_key, req, CTRLRTBL);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("DupConfigVal function is failed %d", result_code);
          return result_code;
        }
        GET_USER_DATA_CTRLR(flowlist_ctrlr_key, ctrlr_id);
        string controller(reinterpret_cast<char *>(ctrlr_id));
        result_code = UpdateConfigStatus(flowlist_key, op[i],
                                         ctrlr_result[controller], NULL,
                                         dmi, flowlist_ctrlr_key);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG(" UpdateConfigStatus Function Failed - %d ",
                         result_code);
          delete flowlist_key;
          return result_code;
        }
      } else if (op[i] == UNC_OP_DELETE) {
        result_code = GetChildConfigKey(flowlist_ctrlr_key, req);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("GetChildConfigKey Failed  %d", result_code);
          return result_code;
        }
      }
      result_code = UpdateConfigDB(flowlist_ctrlr_key, UPLL_DT_RUNNING, op[i],
                                   dmi, CTRLRTBL);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Unable to Update Configuration at DB %d", result_code);
        delete flowlist_ctrlr_key;
        return result_code;
      }

      // update the consolidated config status in the Main Table
      if (op[i] != UNC_OP_DELETE) {
            result_code = UpdateConfigDB(flowlist_key, UPLL_DT_RUNNING,
                UNC_OP_UPDATE, dmi, MAINTBL);
            if (result_code != UPLL_RC_SUCCESS)
              return result_code;
          }

      EnqueCfgNotification(op[i], UPLL_DT_RUNNING, flowlist_ctrlr_key);
      if (flowlist_ctrlr_key) delete flowlist_ctrlr_key;
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
  }
  result_code = (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) ?
      UPLL_RC_SUCCESS : result_code;
  return result_code;
}

bool FlowListMoMgr::IsValidKey(void *key, uint64_t index) {
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
                           ConfigKeyVal *&ckv_running) {
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  val_flowlist_t *val;
  val = (ckv_running != NULL)?
                            reinterpret_cast<val_flowlist_t *>
                            (GetVal(ckv_running)):NULL;
  if (NULL == val) {
    return UPLL_RC_ERR_GENERIC;
  }
  if (uuc::kUpllUcpCreate == phase )
     val->cs_row_status = cs_status;
  for ( unsigned int loop = 0;
        loop < sizeof(val->valid)/sizeof(val->valid[0]);
        ++loop ) {
    if (cs_status == UNC_CS_INVALID &&  UNC_VF_VALID == val->valid[loop])
         val->cs_attr[loop] = cs_status;
    else if (cs_status ==  UNC_CS_APPLIED)
         val->cs_attr[loop] = cs_status;
  }
  return result_code;
}

upll_rc_t FlowListMoMgr::SetConsolidatedStatus(ConfigKeyVal *ikey,
                                               DalDmlIntf *dmi) {
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *ckv = NULL;
  DbSubOp dbop = {kOpReadMultiple, kOpMatchNone, kOpInOutNone};
  result_code = GetChildConfigKey(ckv, ikey);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("Unable to Get the configKey");
    return result_code;
  }
  result_code = ReadConfigDB(ckv,
                             UPLL_DT_RUNNING,
                             UNC_OP_READ, dbop, dmi, CTRLRTBL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("Unable to read the configuration from CTRLR Table");
    if (ckv != NULL) {
      delete ckv;
      ckv = NULL;
    }
    return result_code;
  }
  std::list< unc_keytype_configstatus_t > list_cs_row;
  std::list< unc_keytype_configstatus_t > list_cs_attr;
  val_flowlist_t *val;
  for ( ; ckv != NULL ; ckv = ckv->get_next_cfg_key_val()) {
      val = reinterpret_cast<val_flowlist_t *>(GetVal(ckv));
      list_cs_row.push_back((unc_keytype_configstatus_t)val->cs_row_status);
      list_cs_attr.push_back((unc_keytype_configstatus_t)val->cs_attr[0]);
  }
  val_flowlist_t *val_temp = reinterpret_cast<val_flowlist_t *>(GetVal(ikey));
  val_temp->cs_row_status = GetConsolidatedCsStatus(list_cs_row);
  val_temp->cs_attr[0] = GetConsolidatedCsStatus(list_cs_attr);
  result_code = UpdateConfigDB(ikey,
                               UPLL_DT_RUNNING,
                               UNC_OP_UPDATE, dmi, MAINTBL);
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
  /** check received key,val struct in configkeyval is valid
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
  upll_rc_t result_code = UPLL_RC_ERR_GENERIC;

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

  if (val_rename_flowlist->valid[UPLL_IDX_RENAME_FLOWLIST_RFL]
      != UNC_VF_VALID) {
    UPLL_LOG_DEBUG(" flowlist rename value is not set");
    return UPLL_RC_ERR_BAD_REQUEST;
  }
  result_code = ValidateKey(
      reinterpret_cast<char *>(val_rename_flowlist->flowlist_newname),
      (unsigned int)kMinLenFlowListName,
      (unsigned int)kMaxLenFlowListName);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG(" flowlist rename value validation failed %s",
                   val_rename_flowlist->flowlist_newname);
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
    // operation is read_sibiling/read_sibling_begin/read_sibling_count and
    // data type is candidate/running/state/startup
    //
    // request val structure is mandatory when
    // operation is read and
    // data type is candidate/running/state/startup
    if ((operation == UNC_OP_CREATE) || (operation == UNC_OP_READ)) {
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
  // TODO(Author) added to bypass capability check
  return UPLL_RC_SUCCESS;
  // endTODO

  upll_rc_t ret_val = UPLL_RC_ERR_GENERIC;

  if ((NULL == req) || (NULL == ikey)) {
    UPLL_LOG_DEBUG("IpcReqRespHeader/ConfigKeyval is NULL");
    return ret_val;
  }

  // ctrlr_name is not correct.
  if (!ctrlr_name) {
    ctrlr_name = static_cast<char *>(ikey->get_user_data());
  }
  const std::string version;
  /** Maximum fields in value struct */
  uint32_t dt_type = req->datatype;
  uint32_t operation = req->operation;
  uint32_t option1 = req->option1;
  uint32_t option2 = req->option2;

  UPLL_LOG_TRACE("dt_type   : (%d)"
                "operation : (%d)"
                "option1   : (%d)"
                "option2   : (%d)",
                dt_type, operation, option1, option2);

  bool result_code = false;
  uint32_t instance_count = 0;
  const uint8_t *attrs = 0;
  uint32_t max_attrs = 0;

  switch (operation) {
    case UNC_OP_CREATE: {
      result_code = GetCreateCapability(ctrlr_name, ikey->get_key_type(),
                                        &instance_count, &max_attrs, &attrs);
      break;
    }
    case UNC_OP_UPDATE: {
      result_code = GetUpdateCapability(ctrlr_name, ikey->get_key_type(),
                                        &max_attrs, &attrs);
      break;
    }
    default:
      result_code = GetReadCapability(ctrlr_name, ikey->get_key_type(),
                                      &max_attrs, &attrs);
      break;
  }

  if (!result_code) {
    UPLL_LOG_DEBUG("keytype(%d) is not supported by controller(%s)"
                   " for operation(%d)",
                   ikey->get_key_type(), ctrlr_name, operation);
    return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_KT;
  }

  val_flowlist_t *val_flowlist = NULL;
  if (ikey->get_cfg_val() && (ikey->get_cfg_val()->get_st_num() ==
      IpctSt::kIpcStValFlowlist)) {
     val_flowlist =
       reinterpret_cast<val_flowlist_t *>(ikey->get_cfg_val()->get_val());
  }

  if ((operation == UNC_OP_CREATE)) {
    if (dt_type == UPLL_DT_CANDIDATE) {
      if (val_flowlist) {
        if (max_attrs > 0) {
          return ValFlowlistAttributeSupportCheck(val_flowlist, attrs);
        } else {
          UPLL_LOG_DEBUG("Attribute list is empty for operation %d", operation);
          return UPLL_RC_ERR_CFG_SYNTAX;
        }
      } else {
        UPLL_LOG_DEBUG("Error Mandatory value structure is NULL for"
            " CREATE operation ");
        return UPLL_RC_ERR_CFG_SYNTAX;
      }
    }
  } else if ((READ_SUPPORTED_OPERATION) || (
    ((operation == UNC_OP_READ_NEXT) || (operation == UNC_OP_READ_BULK)) &&
     (dt_type != UPLL_DT_STATE))) {
    if ((READ_SUPPORTED_DATATYPE)|| ((dt_type == UPLL_DT_IMPORT) &&
       (operation != UNC_OP_READ_SIBLING_COUNT))) {
      if (option1 != UNC_OPT1_NORMAL) {
        UPLL_LOG_DEBUG(" Error: option1 is not NORMAL");
        return UPLL_RC_ERR_INVALID_OPTION1;
      }
      if (option2 != UNC_OPT2_NONE) {
        UPLL_LOG_DEBUG(" Error: option2 is not NONE");
        return UPLL_RC_ERR_INVALID_OPTION2;
      }
      /** Valid options received, validate value struct */
      if (val_flowlist) {
        if (max_attrs > 0) {
          return ValFlowlistAttributeSupportCheck(val_flowlist, attrs);
        } else {
          UPLL_LOG_DEBUG("Attribute list is empty for operation %d", operation);
          return UPLL_RC_ERR_CFG_SYNTAX;
        }
      } else if ((operation == UNC_OP_READ) && (dt_type == UPLL_DT_STATE)) {
        UPLL_LOG_DEBUG("Error- Mandatory value struct is NULL for READ");
        return UPLL_RC_ERR_CFG_SYNTAX;
      }
      /** val_flowlist is optional, return SUCCESS if it is NULL*/
      return UPLL_RC_SUCCESS;
    } else {
      UPLL_LOG_DEBUG("Error Unsupported datatype (%d)", dt_type);
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  } else if (operation == UNC_OP_DELETE) {
    UPLL_LOG_TRACE("skip val struct validation for DELETE");
    return UPLL_RC_SUCCESS;
  }
  UPLL_LOG_DEBUG("Error unsupported  operation(%d) ", operation);
  return UPLL_RC_ERR_CFG_SYNTAX;
}

upll_rc_t FlowListMoMgr::ValFlowlistAttributeSupportCheck(
val_flowlist_t *val_flowlist, const uint8_t* attrs ) {
  UPLL_FUNC_TRACE;

  if (val_flowlist != NULL) {
    if ((val_flowlist->valid[UPLL_IDX_IP_TYPE_FL] == UNC_VF_VALID)
        || (val_flowlist->valid[UPLL_IDX_IP_TYPE_FL] ==
           UNC_VF_VALID_NO_VALUE)) {
      if (attrs[unc::capa::flowlist::kCapIpType] == 0) {
        val_flowlist->valid[UPLL_IDX_IP_TYPE_FL] = UNC_VF_NOT_SOPPORTED;

        UPLL_LOG_DEBUG("IPType attr is not supported by ctrlr");
        return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
      }
    }
    UPLL_LOG_DEBUG("val_flowlist attribute validation is success");
    return UPLL_RC_SUCCESS;
  } else {
    UPLL_LOG_DEBUG("Error value struct is NULL");
    return UPLL_RC_ERR_GENERIC;
  }

  return UPLL_RC_ERR_CFG_SYNTAX;
}

upll_rc_t FlowListMoMgr::GetRenameInfo(ConfigKeyVal *ikey,
    ConfigKeyVal *okey, ConfigKeyVal *&rename_info, DalDmlIntf *dmi,
    const char *ctrlr_id, bool &renamed) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  if (!ikey || !okey || NULL == rename_info)
    return UPLL_RC_ERR_GENERIC;

  key_rename_vnode_info_t *key_rename_info =
  reinterpret_cast<key_rename_vnode_info_t*>
  (ConfigKeyVal::Malloc(sizeof(key_rename_vnode_info_t)));

  key_flowlist_t *flowlist_key = NULL;
  if (!(ikey->get_key())) {
    free(key_rename_info);
    return UPLL_RC_ERR_GENERIC;
  }
  flowlist_key = reinterpret_cast<key_flowlist_t *>
    (ikey->get_key());
  if (!strlen(reinterpret_cast<char *>
        (flowlist_key->flowlist_name))) {
    free(key_rename_info);
    return UPLL_RC_ERR_GENERIC;
  }
  uuu::upll_strncpy(key_rename_info->old_flowlist_name,
               flowlist_key->flowlist_name,
               (kMaxLenFlowListName+1));

  if (!(okey->get_key())) {
    free(key_rename_info);
    return UPLL_RC_ERR_GENERIC;
  }
  flowlist_key = reinterpret_cast<key_flowlist_t *>
    (okey->get_key());

  if (!strlen(reinterpret_cast<char *>
        (flowlist_key->flowlist_name))) {
    free(key_rename_info);
    return UPLL_RC_ERR_GENERIC;
  }
  uuu::upll_strncpy(key_rename_info->new_flowlist_name,
               flowlist_key->flowlist_name,
               (kMaxLenFlowListName+1));

  rename_info = new ConfigKeyVal(UNC_KT_FLOWLIST,
      IpctSt::kIpcInvalidStNum, key_rename_info, NULL);
  SET_USER_DATA_CTRLR(rename_info, ctrlr_id);
  if (!renamed) {
    val_rename_flowlist_t *val_rename =
    reinterpret_cast<val_rename_flowlist_t*>
    (ConfigKeyVal::Malloc(sizeof(val_rename_flowlist_t)));
    uuu::upll_strncpy(val_rename->flowlist_newname,
                 key_rename_info->old_flowlist_name,
                 (kMaxLenFlowListName+1));
    ConfigVal *cfg_val = new ConfigVal(IpctSt::kIpcStValRenameFlowlist,
        val_rename);
    okey->AppendCfgVal(cfg_val);
    SET_USER_DATA_CTRLR(okey, ctrlr_id);
    DbSubOp dbop = {kOpNotRead, kOpMatchNone, kOpInOutCtrlr};
    result_code = UpdateConfigDB(okey, UPLL_DT_IMPORT, UNC_OP_CREATE, dmi,
        &dbop, RENAMETBL);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG(" GetRenameInfo Failed. UpdateConfigDb Failed"
        " Result code - %d", result_code);
      free(key_rename_info);
      // free(val_rename);
    }
  }
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
    }
  } else {
    UPLL_LOG_DEBUG("Invalid Keytype (%d)", ikey->get_key_type());
    result_code = UPLL_RC_ERR_GENERIC;
  }
  return result_code;
}

bool FlowListMoMgr::CompareValidValue(void *&val1, void *val2,
                                      bool audit) {
  UPLL_FUNC_TRACE;
  val_flowlist_t *flowlist_val1 =
    reinterpret_cast<val_flowlist_t *>(val1);

  val_flowlist_t *flowlist_val2 =
    reinterpret_cast<val_flowlist_t *>(val2);
  if (flowlist_val1 && flowlist_val2) {
    if (audit) {
      for ( unsigned int loop = 0; loop < sizeof(flowlist_val1->valid);
          ++loop ) {
        if (UNC_VF_INVALID == flowlist_val1->valid[loop] &&
            UNC_VF_VALID == flowlist_val2->valid[loop])
          flowlist_val1->valid[loop] = UNC_VF_VALID_NO_VALUE;
      }
    }
    if (UNC_VF_VALID == flowlist_val1->valid[UPLL_IDX_IP_TYPE_FL] &&
        UNC_VF_VALID == flowlist_val2->valid[UPLL_IDX_IP_TYPE_FL]) {
      if (flowlist_val1->ip_type == flowlist_val2->ip_type) {
        flowlist_val1->valid[UPLL_IDX_IP_TYPE_FL] = UNC_VF_INVALID;
      }
    }
  }
  return false;
}

upll_rc_t FlowListMoMgr::UpdateMainTbl(ConfigKeyVal *key_fl,
      unc_keytype_operation_t op, uint32_t driver_result,
      ConfigKeyVal *nreq, DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  ConfigKeyVal *ck_fl = NULL;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  val_flowlist_t *val_fl = NULL;

  switch (op) {
    case UNC_OP_CREATE:
      result_code = DupConfigKeyVal(ck_fl, key_fl, MAINTBL);
      if (!ck_fl || (result_code != UPLL_RC_SUCCESS)) {
        UPLL_LOG_DEBUG("DupConfigKeyVal() Returning error %d\n", result_code);
        return result_code;
      }
      val_fl = reinterpret_cast<val_flowlist_t *>(GetVal(ck_fl));
      if (!val_fl) {
        UPLL_LOG_DEBUG("invalid val \n");
        return UPLL_RC_ERR_GENERIC;
      }
      val_fl->cs_row_status = UNC_CS_NOT_APPLIED;
      break;
    case UNC_OP_DELETE:

      result_code = GetChildConfigKey(ck_fl, key_fl);
      if (!ck_fl || (result_code != UPLL_RC_SUCCESS)) {
        UPLL_LOG_DEBUG("GetChildConfigKey() returning error %d", result_code);
        return UPLL_RC_ERR_GENERIC;
      }
      break;
    default:
          UPLL_LOG_DEBUG("Inalid operation\n");
      return UPLL_RC_ERR_GENERIC;
  }

  result_code = UpdateConfigDB(ck_fl, UPLL_DT_STATE, op, dmi, MAINTBL);
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
}  // namespace kt_momgr
}  // namespace upll
}  // namespace unc
