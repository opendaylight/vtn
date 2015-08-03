/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include "pfc/log.h"
#include "vtn_flowfilter_entry_momgr.hh"
#include "unc/upll_errno.h"
#include "upll_validation.hh"
#include "unc/upll_ipc_enum.h"
#include "flowlist_momgr.hh"
#include "vtn_momgr.hh"
#include "uncxx/upll_log.hh"
#include "vbr_flowfilter_entry_momgr.hh"
using std::list;
using std::multimap;
using std::string;
using std::set;
using std::map;
using unc::upll::ipc_util::IpcUtil;
#define GET_VALID_MAINCTRL(tbl, l_val_ctrl_ff, l_val_ff, en) \
  (tbl == MAINTBL) ? &(l_val_ff->valid[en]) : &(l_val_ctrl_ff->valid[en])
namespace unc {
namespace upll {
namespace kt_momgr {

#define FLOWLIST_RENAME    0x02
#define NO_FLOWLIST_RENAME ~FLOWLIST_RENAME
// VTN FlowFilter Entry Main Table
BindInfo VtnFlowFilterEntryMoMgr::vtn_flowfilter_entry_bind_info[] = {
  { uudst::vtn_flowfilter_entry::kDbiVtnName, CFG_KEY,
    offsetof(key_vtn_flowfilter_entry_t, flowfilter_key.vtn_key.vtn_name),
    uud::kDalChar, (kMaxLenVtnName + 1) },
  { uudst::vtn_flowfilter_entry::kDbiInputDirection, CFG_KEY,
    offsetof(key_vtn_flowfilter_entry_t, flowfilter_key.input_direction),
    uud::kDalUint8, 1 },
  { uudst::vtn_flowfilter_entry::kDbiSequenceNum, CFG_KEY,
    offsetof(key_vtn_flowfilter_entry_t, sequence_num),
    uud::kDalUint16, 1 },
  { uudst::vtn_flowfilter_entry::kDbiFlowlistName, CFG_VAL,
    offsetof(val_vtn_flowfilter_entry_t, flowlist_name),
    uud::kDalChar, (kMaxLenFlowListName + 1) },
  { uudst::vtn_flowfilter_entry::kDbiAction, CFG_VAL,
    offsetof(val_vtn_flowfilter_entry_t, action),
    uud::kDalUint8, 1 },
  { uudst::vtn_flowfilter_entry::kDbiNwnName, CFG_VAL,
    offsetof(val_vtn_flowfilter_entry_t, nwm_name),
    uud::kDalChar, (kMaxLenNwmName + 1) },
  { uudst::vtn_flowfilter_entry::kDbiDscp, CFG_VAL,
    offsetof(val_vtn_flowfilter_entry_t, dscp),
    uud::kDalUint8, 1 },
  { uudst::vtn_flowfilter_entry::kDbiPriority, CFG_VAL,
    offsetof(val_vtn_flowfilter_entry_t, priority),
    uud::kDalUint8, 1 },
  { uudst::vtn_flowfilter_entry::kDbiFlags,
    CK_VAL, offsetof(key_user_data_t, flags),
    uud::kDalUint8, 1 },
  { uudst::vtn_flowfilter_entry::kDbiValidFlowlistName, CFG_META_VAL,
    offsetof(val_vtn_flowfilter_entry_t, valid[0]),
    uud::kDalUint8, 1 },
  { uudst::vtn_flowfilter_entry::kDbiValidAction, CFG_META_VAL,
    offsetof(val_vtn_flowfilter_entry_t, valid[1]),
    uud::kDalUint8, 1 },
  { uudst::vtn_flowfilter_entry::kDbiValidNwnName, CFG_META_VAL,
    offsetof(val_vtn_flowfilter_entry_t, valid[2]),
    uud::kDalUint8, 1 },
  { uudst::vtn_flowfilter_entry::kDbiValidDscp, CFG_META_VAL,
    offsetof(val_vtn_flowfilter_entry_t, valid[3]),
    uud::kDalUint8, 1 },
  { uudst::vtn_flowfilter_entry::kDbiValidPriority, CFG_META_VAL,
    offsetof(val_vtn_flowfilter_entry_t, valid[4]),
    uud::kDalUint8, 1 },
  { uudst::vtn_flowfilter_entry::kDbiCsRowStatus, CS_VAL,
    offsetof(val_vtn_flowfilter_entry_t, cs_row_status),
    uud::kDalUint8, 1 },
  { uudst::vtn_flowfilter_entry::kDbiCsFlowlistName, CS_VAL,
    offsetof(val_vtn_flowfilter_entry_t, cs_attr[0]),
    uud::kDalUint8, 1 },
  { uudst::vtn_flowfilter_entry::kDbiCsAction, CS_VAL,
    offsetof(val_vtn_flowfilter_entry_t, cs_attr[1]),
    uud::kDalUint8, 1 },
  { uudst::vtn_flowfilter_entry::kDbiCsNwnName, CS_VAL,
    offsetof(val_vtn_flowfilter_entry_t, cs_attr[2]),
    uud::kDalUint8, 1 },
  { uudst::vtn_flowfilter_entry::kDbiCsDscp, CS_VAL,
    offsetof(val_vtn_flowfilter_entry_t, cs_attr[3]),
    uud::kDalUint8, 1 },
  { uudst::vtn_flowfilter_entry::kDbiCsPriority, CS_VAL,
    offsetof(val_vtn_flowfilter_entry_t, cs_attr[4]),
    uud::kDalUint8, 1 } };

// VTN FlowFilter Entry Controller Table
BindInfo VtnFlowFilterEntryMoMgr::vtn_flowfilter_entry_ctrlr_bind_info[] = {
  { uudst::vtn_flowfilter_entry_ctrlr::kDbiVtnName, CFG_KEY,
    offsetof(key_vtn_flowfilter_entry_t, flowfilter_key.vtn_key.vtn_name),
    uud::kDalChar, (kMaxLenVtnName + 1) },
  { uudst::vtn_flowfilter_entry_ctrlr::kDbiInputDirection, CFG_KEY,
    offsetof(key_vtn_flowfilter_entry_t, flowfilter_key.input_direction),
    uud::kDalUint8, 1 },
  { uudst::vtn_flowfilter_entry_ctrlr::kDbiSequenceNum, CFG_KEY,
    offsetof(key_vtn_flowfilter_entry_t, sequence_num),
    uud::kDalUint16, 1 },
  { uudst::vtn_flowfilter_entry_ctrlr::kDbiCtrlrName, CK_VAL,
    offsetof(key_user_data_t, ctrlr_id),
    uud::kDalChar, (kMaxLenCtrlrId + 1) },
  { uudst::vtn_flowfilter_entry_ctrlr::kDbiDomainId, CK_VAL,
    offsetof(key_user_data_t, domain_id),
    uud::kDalChar, (kMaxLenDomainId + 1) },
  { uudst::vtn_flowfilter_entry_ctrlr::kDbiFlags, CK_VAL,
    offsetof(key_user_data_t, flags),
      uud::kDalUint8, 1 },
  { uudst::vtn_flowfilter_entry_ctrlr::kDbiValidFlowlistName, CFG_META_VAL,
    offsetof(val_vtn_flowfilter_entry_ctrlr, valid[0]),
    uud::kDalUint8, 1 },
  { uudst::vtn_flowfilter_entry_ctrlr::kDbiValidAction, CFG_META_VAL,
    offsetof(val_vtn_flowfilter_entry_ctrlr, valid[1]),
      uud::kDalUint8, 1 },
  { uudst::vtn_flowfilter_entry_ctrlr::kDbiValidNwnName, CFG_META_VAL,
    offsetof(val_vtn_flowfilter_entry_ctrlr, valid[2]),
      uud::kDalUint8, 1 },
  { uudst::vtn_flowfilter_entry_ctrlr::kDbiValidDscp, CFG_META_VAL,
    offsetof(val_vtn_flowfilter_entry_ctrlr, valid[3]),
      uud::kDalUint8, 1 },
  { uudst::vtn_flowfilter_entry_ctrlr::kDbiValidPriority, CFG_META_VAL,
    offsetof(val_vtn_flowfilter_entry_ctrlr, valid[4]),
    uud::kDalUint8, 1 },
  { uudst::vtn_flowfilter_entry_ctrlr::kDbiCsRowStatus, CS_VAL,
    offsetof(val_vtn_flowfilter_entry_ctrlr, cs_row_status),
    uud::kDalUint8, 1 },
  { uudst::vtn_flowfilter_entry_ctrlr::kDbiCsFlowlistName, CS_VAL,
    offsetof(val_vtn_flowfilter_entry_ctrlr, cs_attr[0]),
    uud::kDalUint8, 1 },
  { uudst::vtn_flowfilter_entry_ctrlr::kDbiCsAction, CS_VAL,
    offsetof(val_vtn_flowfilter_entry_ctrlr, cs_attr[1]),
    uud::kDalUint8, 1 },
  { uudst::vtn_flowfilter_entry_ctrlr::kDbiCsNwnName, CS_VAL,
    offsetof(val_vtn_flowfilter_entry_ctrlr, cs_attr[2]),
    uud::kDalUint8, 1 },
  { uudst::vtn_flowfilter_entry_ctrlr::kDbiCsDscp, CS_VAL,
    offsetof(val_vtn_flowfilter_entry_ctrlr, cs_attr[3]),
    uud::kDalUint8, 1 },
  { uudst::vtn_flowfilter_entry_ctrlr::kDbiCsPriority, CS_VAL,
    offsetof(val_vtn_flowfilter_entry_ctrlr, cs_attr[4]),
    uud::kDalUint8, 1 }
};

BindInfo VtnFlowFilterEntryMoMgr::vtnflowfilterentrymaintbl_bind_info[] = {
  {uudst::vtn_flowfilter_entry::kDbiVtnName, CFG_MATCH_KEY, offsetof(
    key_vtn_flowfilter_entry_t, flowfilter_key.vtn_key.vtn_name),
    uud::kDalChar, kMaxLenVtnName + 1},
  {uudst::vtn_flowfilter_entry::kDbiVtnName, CFG_INPUT_KEY, offsetof(
    key_rename_vnode_info_t, new_unc_vtn_name),
    uud::kDalChar, kMaxLenVtnName + 1},
  {uudst::vtn_flowfilter_entry::kDbiFlags, CK_VAL, offsetof(
    key_user_data_t, flags),
    uud::kDalUint8, 1}
};

BindInfo VtnFlowFilterEntryMoMgr::vtnflowfilterentryctrlrtbl_bind_info[] = {
  {uudst::vtn_flowfilter_entry_ctrlr::kDbiVtnName, CFG_MATCH_KEY, offsetof(
    key_vtn_flowfilter_t, vtn_key.vtn_name),
    uud::kDalChar, kMaxLenVtnName + 1},
  {uudst::vtn_flowfilter_entry_ctrlr::kDbiVtnName, CFG_INPUT_KEY, offsetof(
    key_rename_vnode_info_t, new_unc_vtn_name),
    uud::kDalChar, kMaxLenVtnName + 1},
  {uudst::vtn_flowfilter_entry_ctrlr::kDbiFlags, CK_VAL, offsetof(
    key_user_data_t, flags),
    uud::kDalUint8, 1}
};

VtnFlowFilterEntryMoMgr::VtnFlowFilterEntryMoMgr() :MoMgrImpl() {
  UPLL_FUNC_TRACE;
  // Rename operation is not support for this KT
  // setting max tables to 2
  ntable = MAX_MOMGR_TBLS;
  table = new Table *[ntable]();
  // Construct  Main Table
  table[MAINTBL] = new Table(uudst::kDbiVtnFlowFilterEntryTbl,
      UNC_KT_VTN_FLOWFILTER_ENTRY, vtn_flowfilter_entry_bind_info,
      IpctSt::kIpcStKeyVtnFlowfilterEntry, IpctSt::kIpcStValVtnFlowfilterEntry,
      uudst::vtn_flowfilter_entry::kDbiVtnFlowFilterEntryNumCols);
  table[RENAMETBL] = NULL;
  // Construct CONTROLLER Table
  table[CTRLRTBL] = new Table(uudst::kDbiVtnFlowFilterEntryCtrlrTbl,
      UNC_KT_VTN_FLOWFILTER_ENTRY, vtn_flowfilter_entry_ctrlr_bind_info,
      IpctSt::kIpcStKeyVtnFlowfilterEntry, IpctSt::kIpcInvalidStNum,
      uudst::vtn_flowfilter_entry_ctrlr::kDbiVtnFlowFilterEntryCtrlrNumCols);

  table[CONVERTTBL] = NULL;

  // VTN FlowFilter Entry Does not have any child
  nchild = 0;
  child = NULL;
}

upll_rc_t VtnFlowFilterEntryMoMgr::IsReferenced(IpcReqRespHeader *req,
                                                ConfigKeyVal *ikey,
                                                DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  if (NULL == ikey) return UPLL_RC_ERR_GENERIC;
  DbSubOp dbop = { kOpReadExist, kOpMatchNone, kOpInOutNone };
  // Check the exixtence in Maintable
  result_code = UpdateConfigDB(ikey, req->datatype, UNC_OP_READ, dmi,
                               &dbop, MAINTBL);
  return result_code;
}

upll_rc_t VtnFlowFilterEntryMoMgr::GetControllerKeyval(
    ConfigKeyVal *&ctrlckv,
    ConfigKeyVal *&ikey,
    controller_domain *ctrlr_dom) {
  UPLL_FUNC_TRACE;
  ConfigVal *ctrlcv = NULL;
  upll_rc_t result_code = UPLL_RC_ERR_GENERIC;
  key_vtn_flowfilter_entry_t *vtn_ff_ctrl_key =
           reinterpret_cast<key_vtn_flowfilter_entry_t*>(ikey->get_key());
  if (vtn_ff_ctrl_key == NULL) {
    return UPLL_RC_ERR_GENERIC;
  }
  result_code = GetChildConfigKey(ctrlckv, ikey);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetChildConfigKey failed %d", result_code);
    return result_code;
  }
  val_vtn_flowfilter_entry_t *val_vtn_ff_entry =
                     reinterpret_cast<val_vtn_flowfilter_entry_t*>
                     (ikey->get_cfg_val()->get_val());
  if (val_vtn_ff_entry == NULL) {
    return UPLL_RC_ERR_GENERIC;
  }
  val_vtn_flowfilter_entry_ctrlr *val_ff_ctrl =
      reinterpret_cast<val_vtn_flowfilter_entry_ctrlr*>
      (ConfigKeyVal::Malloc(sizeof(val_vtn_flowfilter_entry_ctrlr)));
  for (unsigned int loop = 0; loop <
      sizeof(val_vtn_ff_entry->valid) /sizeof(val_vtn_ff_entry->valid[0]);
      loop++) {
     if (val_vtn_ff_entry->valid[loop] == UNC_VF_VALID) {
        val_ff_ctrl->valid[loop] = UNC_VF_VALID;
     }
  }
  ctrlcv = new ConfigVal(IpctSt::kIpcInvalidStNum, val_ff_ctrl);
  ctrlckv->AppendCfgVal(ctrlcv);
  SET_USER_DATA_CTRLR_DOMAIN(ctrlckv, *ctrlr_dom);
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtnFlowFilterEntryMoMgr::UpdateControllerTableForVtn(
    uint8_t* vtn_name,
    controller_domain *ctrlr_dom,
    unc_keytype_operation_t op,
    upll_keytype_datatype_t dt_type,
    DalDmlIntf *dmi,
    uint8_t flag,
    TcConfigMode config_mode) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *ikey = NULL;
  std::list<controller_domain_t> list_ctrlr_dom;
  uint8_t *ctrlr_id = ctrlr_dom->ctrlr;
  if (!ctrlr_id) {
    UPLL_LOG_ERROR(" Ctrlr_id is NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  list_ctrlr_dom.push_back(*ctrlr_dom);
  result_code = GetChildConfigKey(ikey, NULL);
  if (result_code != UPLL_RC_SUCCESS) {
    return result_code;
  }
  key_vtn_flowfilter_entry_t *vtn_ff_key =
      reinterpret_cast<key_vtn_flowfilter_entry_t*>
      (ikey->get_key());
  uuu::upll_strncpy(vtn_ff_key->flowfilter_key.vtn_key.vtn_name,
                    vtn_name, (kMaxLenVtnName+1));
  // set this value so that the direction
  // can be bound for output instead of match
  vtn_ff_key->flowfilter_key.input_direction = 0xFE;

  DbSubOp dbop = {kOpReadMultiple, kOpMatchNone, kOpInOutNone};
  // Read the Configuration from the MainTable
  result_code = ReadConfigDB(ikey, dt_type,
                             UNC_OP_READ, dbop, dmi, MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      DELETE_IF_NOT_NULL(ikey);
      UPLL_LOG_DEBUG(" No Records in main table to be created in ctrlr tbl");
      return UPLL_RC_SUCCESS;
    }
    UPLL_LOG_DEBUG(" Read main table failed ");
    DELETE_IF_NOT_NULL(ikey);
    return result_code;
  }

  string vtnname(reinterpret_cast<const char *>(vtn_name));
  if (flag != 0) {
    UPLL_LOG_DEBUG("flag in UpdateControllerTableForVtn %d", flag);
    ConfigKeyVal *temp_ikey = ikey;
    ConfigKeyVal *flag_ikey = NULL;
    while (NULL != temp_ikey) {
      result_code = DupConfigKeyVal(flag_ikey, temp_ikey, MAINTBL);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("DupConfigKeyVal failed %d", result_code);
        DELETE_IF_NOT_NULL(ikey);
        return result_code;
      }
      uint8_t temp_flag = 0;
      GET_USER_DATA_FLAGS(flag_ikey, temp_flag);
      UPLL_LOG_DEBUG("temp_flag in UpdateControllerTableForVtn %d", temp_flag);
      flag = flag | temp_flag;
      SET_USER_DATA_FLAGS(flag_ikey, flag);
      UPLL_LOG_DEBUG("temp_flag in UpdateControllerTableForVtn %d", temp_flag);
      DbSubOp dbop1 = {kOpNotRead, kOpMatchNone, kOpInOutCs | kOpInOutFlag};
      result_code = UpdateConfigDB(flag_ikey, dt_type, UNC_OP_UPDATE,
          dmi, &dbop1, config_mode, vtnname, MAINTBL);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_ERROR("UpdateConfigDB failed %d", result_code);
        DELETE_IF_NOT_NULL(ikey);
        DELETE_IF_NOT_NULL(flag_ikey);
        return result_code;
      }
      DELETE_IF_NOT_NULL(flag_ikey);
      temp_ikey = temp_ikey->get_next_cfg_key_val();
    }
  }

  FlowListMoMgr *flowlist_mgr = reinterpret_cast<FlowListMoMgr *>
      (const_cast<MoManager *> (GetMoManager(UNC_KT_FLOWLIST)));
  ConfigKeyVal *new_ikey = ikey;
  while (new_ikey != NULL) {
    result_code = UpdateControllerTable(new_ikey, op,
                                        dt_type, dmi,
                                        list_ctrlr_dom,
                                        config_mode, vtnname);

    if (result_code != UPLL_RC_SUCCESS) {
      if ((result_code == UPLL_RC_ERR_EXCEEDS_RESOURCE_LIMIT) ||
          (result_code == UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR)) {
         UPLL_LOG_ERROR("ValidateCapability failed:%d", result_code);
         DELETE_IF_NOT_NULL(ikey);
         return result_code;
      }
      if (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
        UPLL_LOG_DEBUG("No instance in UpdateControllerTable");
        DELETE_IF_NOT_NULL(ikey);
        return UPLL_RC_SUCCESS;
      }
      UPLL_LOG_DEBUG("create in ctrlr tbl failed: error code (%d)",
                     result_code);
    }
    // Notify the flowlistmomgr is flowlist is configured.
    //
    val_vtn_flowfilter_entry_t *val_vtn_ffe =
        reinterpret_cast<val_vtn_flowfilter_entry_t *> (GetVal(new_ikey));
    // if Flowlist name is configured in the flowfilter
    // send controller add/delete request to flowlist momgr.
    if (UNC_VF_VALID == val_vtn_ffe->valid[UPLL_IDX_FLOWLIST_NAME_VFFE]) {
      result_code = flowlist_mgr->AddFlowListToController(
          reinterpret_cast<char *>(val_vtn_ffe->flowlist_name), dmi,
          reinterpret_cast<char *>(ctrlr_id), dt_type, op, config_mode,
          vtnname, false);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_INFO("AddFlowListToController failed err(%d)", result_code);
        DELETE_IF_NOT_NULL(ikey);
        return result_code;
      }
    }
    new_ikey = new_ikey->get_next_cfg_key_val();
  }
  DELETE_IF_NOT_NULL(ikey);
  UPLL_LOG_DEBUG("Successful completion of the controller table updation");
  return result_code;
}


upll_rc_t VtnFlowFilterEntryMoMgr::CreateCandidateMo(IpcReqRespHeader *req,
                                                     ConfigKeyVal *ikey,
                                                     DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  // ConfigKeyVal *okey = NULL;
  if (ikey == NULL || req == NULL) {
    UPLL_LOG_TRACE(
        "Cannot perform create operation due to insufficient parameters");
    return UPLL_RC_ERR_GENERIC;
  }
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  upll_rc_t vtn_ctrlr_span_rt_code = UPLL_RC_SUCCESS;
  if (req->datatype != UPLL_DT_IMPORT) {
    // validate syntax and semantics
    result_code = ValidateMessage(req, ikey);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("ValidateMessage failed result code=%d", result_code);
      return result_code;
    }
  }

  result_code = ValidateAttribute(ikey, dmi, req);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("ValidateAttribute failed: err code(%d)", result_code);
    return result_code;
  }

  std::list<controller_domain_t> list_ctrlr_dom;
  vtn_ctrlr_span_rt_code = GetVtnControllerSpan(ikey, req->datatype,
                                                dmi, list_ctrlr_dom);
  if ((vtn_ctrlr_span_rt_code != UPLL_RC_SUCCESS) &&
      (vtn_ctrlr_span_rt_code != UPLL_RC_ERR_NO_SUCH_INSTANCE)) {
    UPLL_LOG_INFO(" GetVtnControllerSpan  error code (%d)",
                   vtn_ctrlr_span_rt_code);
    return result_code;
  }

  if (vtn_ctrlr_span_rt_code == UPLL_RC_SUCCESS) {
    for (std::list<controller_domain_t>::iterator it= list_ctrlr_dom.begin();
         it != list_ctrlr_dom.end(); ++it) {
      /*result_code = ValidateCapability(
          req, ikey,
          reinterpret_cast<const char *>(it->ctrlr));
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Key not supported by controller");
        return result_code;
      }*/
    }
  } else if (UPLL_RC_ERR_NO_SUCH_INSTANCE != vtn_ctrlr_span_rt_code) {
    UPLL_LOG_ERROR(" GetVtnControllerSpan failed . Resultcode %d ",
                   vtn_ctrlr_span_rt_code);
    return vtn_ctrlr_span_rt_code;
  }
  TcConfigMode config_mode = TC_CONFIG_INVALID;
  std::string vtn_name = "";
  result_code = GetConfigModeInfo(req, config_mode, vtn_name);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetConfigMode failed");
    return result_code;
  }

  // create a record in CANDIDATE DB
  result_code = UpdateConfigDB(ikey, req->datatype, UNC_OP_CREATE,
                               dmi, config_mode, vtn_name, MAINTBL);
  if ((req->datatype == UPLL_DT_IMPORT) &&
      (result_code == UPLL_RC_ERR_INSTANCE_EXISTS)) {
      result_code = UPLL_RC_SUCCESS;
  }
  if (result_code != UPLL_RC_SUCCESS) {
    // delete okey;
    FREE_LIST_CTRLR(list_ctrlr_dom);
    UPLL_LOG_ERROR("UpdateConfigDB failed:%d", result_code);
    return result_code;
  }

// create a record in CANDIDATE DB for controller Table
  if (vtn_ctrlr_span_rt_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    result_code = UpdateControllerTable(ikey, UNC_OP_CREATE,
                                        req->datatype, dmi,
                                        list_ctrlr_dom,
                                        config_mode, vtn_name);

    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_INFO("Err while adding record in ctrlr tbl: err %d",
                     result_code);
      upll_rc_t del_result_code = UpdateConfigDB(ikey, req->datatype,
                                                 UNC_OP_DELETE,
                                                 dmi, config_mode,
                                                 vtn_name, MAINTBL);
      if (del_result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("delete in CandidateDB failed: err code(%d) ",
                       del_result_code);
      }
      FREE_LIST_CTRLR(list_ctrlr_dom);
      return result_code;
    }
  }
  val_vtn_flowfilter_entry_t *val_vtn_ffe = reinterpret_cast
    <val_vtn_flowfilter_entry_t *>(GetVal(ikey));
  if (UNC_VF_VALID == val_vtn_ffe->valid[UPLL_IDX_FLOWLIST_NAME_VFFE]) {
    result_code = UpdateFlowListInCtrl(ikey, req->datatype, UNC_OP_CREATE, dmi,
                                       config_mode, vtn_name);
    if (result_code != UPLL_RC_SUCCESS) {
      if (vtn_ctrlr_span_rt_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
        FREE_LIST_CTRLR(list_ctrlr_dom);
      }
      return result_code;
    }
  }

  if (vtn_ctrlr_span_rt_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    FREE_LIST_CTRLR(list_ctrlr_dom);
  }
  // delete okey;
  return result_code;
}

upll_rc_t VtnFlowFilterEntryMoMgr::RestorePOMInCtrlTbl(
    ConfigKeyVal *ikey,
    upll_keytype_datatype_t dt_type,
    MoMgrTables tbl,
    DalDmlIntf* dmi) {

  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  if (!ikey || !(ikey->get_key())) {
    UPLL_LOG_DEBUG("Input Key Not Valid");
    return UPLL_RC_ERR_GENERIC;
  }
  if (tbl != MAINTBL ||
       (ikey->get_key_type() != UNC_KT_VTN_FLOWFILTER_ENTRY)) {
    UPLL_LOG_DEBUG("Ignoring  ktype/Table kt=%d, tbl=%d",
                    ikey->get_key_type(), tbl);
    return result_code;
  }
  val_vtn_flowfilter_entry_t *flowfilter_val =
      reinterpret_cast<val_vtn_flowfilter_entry_t *> (GetVal(ikey));

  if (NULL == flowfilter_val) {
    UPLL_LOG_DEBUG(" Value structure is NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  /*
  if (flowfilter_val->valid[UPLL_IDX_FLOWLIST_NAME_VFFE] == UNC_VF_VALID) {
    result_code = UpdateFlowListInCtrl(ikey, dt_type, UNC_OP_CREATE, dmi);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Unable to update flowlist in ctrlr table, err %d",
                    result_code);
      return result_code;
    }
  }
  */
  return result_code;
}
upll_rc_t VtnFlowFilterEntryMoMgr::UpdateFlowListInCtrl(
                                   ConfigKeyVal *ikey,
                                   upll_keytype_datatype_t dt_type,
                                   unc_keytype_operation_t op,
                                   DalDmlIntf* dmi,
                                   TcConfigMode config_mode,
                                   string vtn_name) {
  UPLL_FUNC_TRACE;
  ConfigKeyVal *vtn_okey = NULL;
  uint8_t* ctrlr_id;
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  VtnMoMgr *vtnmgr =
      static_cast<VtnMoMgr *>((const_cast<MoManager *>
      (GetMoManager(UNC_KT_VTN))));
  FlowListMoMgr *flowlist_mgr = reinterpret_cast<FlowListMoMgr *>
  (const_cast<MoManager *> (GetMoManager(UNC_KT_FLOWLIST)));
  if (vtnmgr == NULL || flowlist_mgr == NULL) {
    UPLL_LOG_DEBUG("Invalid mgr Object for KT_VTN or KT_FLOWLIST");
    return UPLL_RC_ERR_GENERIC;
  }
  result_code = vtnmgr->GetChildConfigKey(vtn_okey, NULL);
  if (result_code != UPLL_RC_SUCCESS) {
    return result_code;
  }
  val_vtn_flowfilter_entry_t *flowfilter_val =
           reinterpret_cast<val_vtn_flowfilter_entry_t *> (GetVal(ikey));

  // For Controllers in VTN Span
  // Update the Flowlist details of the FlowlistController Table
  key_vtn_t *vtn_key = reinterpret_cast<key_vtn*>(vtn_okey->get_key());
  key_vtn_flowfilter_entry_t *ff_entry_key =
                 reinterpret_cast<key_vtn_flowfilter_entry_t*>(ikey->get_key());
  uuu::upll_strncpy(vtn_key->vtn_name,
                   ff_entry_key->flowfilter_key.vtn_key.vtn_name,
                   (kMaxLenVtnName + 1));
  result_code = vtnmgr->GetControllerDomainSpan(vtn_okey, dt_type,
                                                dmi);
  if (result_code != UPLL_RC_SUCCESS &&
      UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
    DELETE_IF_NOT_NULL(vtn_okey);
    UPLL_LOG_DEBUG("Error in getting controller span (%d)",
                   result_code);
    return result_code;
  }
  if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
    UPLL_LOG_DEBUG("No entry in ctrlr tbl (%d)",
                   result_code);
    DELETE_IF_NOT_NULL(vtn_okey);
    return UPLL_RC_SUCCESS;
  }
  ConfigKeyVal *tmp_ckv = vtn_okey;
  while (NULL != tmp_ckv) {
    // check for vnode_ref_cnt(vnodes count) in vtn_ctrlr_tbl val structure.
    // If the count is '0' then continue for next vtn_ctrlr_tbl.
    val_vtn_ctrlr *ctr_val =
        reinterpret_cast<val_vtn_ctrlr *>(GetVal(tmp_ckv));
    if (!ctr_val) {
      UPLL_LOG_ERROR("Vtn controller table val structure is NULL");
      DELETE_IF_NOT_NULL(vtn_okey);
      return UPLL_RC_ERR_GENERIC;
    }
    if (ctr_val->vnode_ref_cnt <= 0) {
      UPLL_LOG_DEBUG("skipping entry");
      tmp_ckv = tmp_ckv->get_next_cfg_key_val();
      continue;
    }
    ctrlr_id = NULL;
    GET_USER_DATA_CTRLR(tmp_ckv, ctrlr_id);
    if (NULL == ctrlr_id) {
      UPLL_LOG_DEBUG("ctrlr_id NULL");
      DELETE_IF_NOT_NULL(vtn_okey);
      return UPLL_RC_ERR_GENERIC;
    }
    UPLL_LOG_TRACE("flowlist name %s length %zu", flowfilter_val->flowlist_name,
                    strlen((const char *)flowfilter_val->flowlist_name));
    std::string temp_vtn_name;
    if (TC_CONFIG_VTN == config_mode) {
      temp_vtn_name = vtn_name;
    } else {
      temp_vtn_name = reinterpret_cast<const char*>(ff_entry_key->
                                                    flowfilter_key.vtn_key.
                                                    vtn_name);
    }
    if (UNC_OP_CREATE == op || UNC_OP_DELETE == op) {
      result_code = flowlist_mgr->AddFlowListToController(
          reinterpret_cast<char *>(flowfilter_val->flowlist_name), dmi,
          reinterpret_cast<char *> (ctrlr_id), dt_type, op,
          config_mode, temp_vtn_name, false);
      if (result_code != UPLL_RC_SUCCESS) {
        DELETE_IF_NOT_NULL(vtn_okey);
        return result_code;
      }
    } else if (UNC_OP_UPDATE == op) {
      ConfigKeyVal *tempckv = NULL;
      result_code = GetChildConfigKey(tempckv, ikey);
      if (UPLL_RC_SUCCESS != result_code) {
        UPLL_LOG_TRACE("GetChildConfigKey failed");
        DELETE_IF_NOT_NULL(vtn_okey);
        return result_code;
      }
      DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutNone };
      result_code = ReadConfigDB(tempckv, dt_type, UNC_OP_READ,
                                 dbop, dmi, MAINTBL);
      if (UPLL_RC_SUCCESS != result_code) {
        UPLL_LOG_DEBUG("ReadConfigDB failed");
        DELETE_IF_NOT_NULL(vtn_okey);
        DELETE_IF_NOT_NULL(tempckv);
        return result_code;
      }
      val_vtn_flowfilter_entry_t *vtn_ffe_val = reinterpret_cast
          <val_vtn_flowfilter_entry_t *>(GetVal(ikey));
      val_vtn_flowfilter_entry_t *temp_ffe_val = reinterpret_cast
          <val_vtn_flowfilter_entry_t *>(GetVal(tempckv));
      if (UNC_VF_VALID == vtn_ffe_val->valid[UPLL_IDX_FLOWLIST_NAME_VFFE] &&
          UNC_VF_VALID  == temp_ffe_val->
              valid[UPLL_IDX_FLOWLIST_NAME_VFFE]) {
        result_code = flowlist_mgr->AddFlowListToController(
            reinterpret_cast<char *>(temp_ffe_val->flowlist_name), dmi,
            reinterpret_cast<char *> (ctrlr_id), dt_type, UNC_OP_DELETE,
            config_mode, temp_vtn_name, false);
        if (result_code != UPLL_RC_SUCCESS) {
          DELETE_IF_NOT_NULL(vtn_okey);
          DELETE_IF_NOT_NULL(tempckv);
          return result_code;
        }
        result_code = flowlist_mgr->AddFlowListToController(
            reinterpret_cast<char *>(vtn_ffe_val->flowlist_name), dmi,
            reinterpret_cast<char *> (ctrlr_id), dt_type, UNC_OP_CREATE,
            config_mode, temp_vtn_name, false);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_INFO("flowlist-ctrlrtbl create err:%d", result_code);
          DELETE_IF_NOT_NULL(vtn_okey);
          DELETE_IF_NOT_NULL(tempckv);
          return result_code;
        }
      } else if (UNC_VF_VALID == vtn_ffe_val->
                 valid[UPLL_IDX_FLOWLIST_NAME_VFFE] &&
                 (UNC_VF_INVALID == temp_ffe_val->
              valid[UPLL_IDX_FLOWLIST_NAME_VFFE] || UNC_VF_VALID_NO_VALUE ==
              temp_ffe_val->valid[UPLL_IDX_FLOWLIST_NAME_VFFE])) {
        result_code = flowlist_mgr->AddFlowListToController(
            reinterpret_cast<char *>(vtn_ffe_val->flowlist_name), dmi,
            reinterpret_cast<char *> (ctrlr_id), dt_type, UNC_OP_CREATE,
            config_mode, temp_vtn_name, false);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_INFO("Err while updating  in flowlist-ctrlrtbl!err %d",
                        result_code);
          DELETE_IF_NOT_NULL(vtn_okey);
          DELETE_IF_NOT_NULL(tempckv);
          return result_code;
        }
      } else if (UNC_VF_VALID_NO_VALUE == vtn_ffe_val->
                 valid[UPLL_IDX_FLOWLIST_NAME_VFFE] &&
                 UNC_VF_VALID == temp_ffe_val->
              valid[UPLL_IDX_FLOWLIST_NAME_VFFE]) {
        result_code = flowlist_mgr->AddFlowListToController(
            reinterpret_cast<char *>(temp_ffe_val->flowlist_name), dmi,
            reinterpret_cast<char *> (ctrlr_id), dt_type, UNC_OP_DELETE,
            config_mode, temp_vtn_name, false);
        if (result_code != UPLL_RC_SUCCESS) {
          DELETE_IF_NOT_NULL(vtn_okey);
          DELETE_IF_NOT_NULL(tempckv);
          return result_code;
        }
      }
      DELETE_IF_NOT_NULL(tempckv);
    }
    tmp_ckv = tmp_ckv->get_next_cfg_key_val();
  }
  DELETE_IF_NOT_NULL(vtn_okey);
  return result_code;
}

upll_rc_t VtnFlowFilterEntryMoMgr::DeleteMo(IpcReqRespHeader *req,
                                            ConfigKeyVal *ikey,
                                            DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_ERR_GENERIC;

  if (NULL == ikey || NULL == req) {
  UPLL_LOG_DEBUG("Delete Operation failed:Bad request");
  return result_code;
  }
  result_code = ValidateMessage(req, ikey);
  if (result_code != UPLL_RC_SUCCESS)
     return result_code;

  result_code = UpdateConfigDB(ikey, UPLL_DT_CANDIDATE, UNC_OP_READ, dmi);

  if (UPLL_RC_ERR_INSTANCE_EXISTS != result_code) {
    UPLL_LOG_DEBUG("Delete Operation Failed: No Record found in DB");
    return result_code;
  }
  // Read the DB get the flowlist value and send the delete request to
  // flowlist momgr if flowlist is configured.

  ConfigKeyVal *okey = NULL;
  result_code = GetChildConfigKey(okey, ikey);
  if (UPLL_RC_SUCCESS != result_code) {
    return result_code;
  }
  DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutFlag };
  result_code = ReadConfigDB(okey, UPLL_DT_CANDIDATE,
                             UNC_OP_READ, dbop, dmi, MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Unable to read configuration from CandidateDb");
    DELETE_IF_NOT_NULL(okey);
    return result_code;
  }
  TcConfigMode config_mode = TC_CONFIG_INVALID;
  std::string vtn_name = "";
  result_code = GetConfigModeInfo(req, config_mode, vtn_name);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetConfigMode failed");
    return result_code;
  }

  val_vtn_flowfilter_entry_t *vtn_ffe_val = reinterpret_cast
          <val_vtn_flowfilter_entry_t *>(GetVal(okey));
  if (UNC_VF_VALID == vtn_ffe_val->valid[UPLL_IDX_FLOWLIST_NAME_VFFE]) {
    result_code = UpdateFlowListInCtrl(okey, req->datatype, UNC_OP_DELETE, dmi,
                                       config_mode, vtn_name);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Unable to update flowlist in ctrlr table");
      DELETE_IF_NOT_NULL(okey);
      return result_code;
    }
  }
  DELETE_IF_NOT_NULL(okey);

  DbSubOp dbop1 = {kOpNotRead, kOpMatchNone, kOpInOutNone};
  result_code = UpdateConfigDB(ikey, UPLL_DT_CANDIDATE, UNC_OP_DELETE, dmi,
                                &dbop1, config_mode, vtn_name, MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Delete Operation Failed: err code (%d)", result_code);
    return result_code;
  }
  result_code = UpdateConfigDB(ikey, UPLL_DT_CANDIDATE, UNC_OP_DELETE, dmi,
                               &dbop1, config_mode, vtn_name, CTRLRTBL);
  if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    UPLL_LOG_DEBUG("Delete : No matching record in ctrlrtbl:DB Error");
    return UPLL_RC_SUCCESS;
  } else if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Delete Operation Failed in ctrlrtbl:DB Error");
    return result_code;
  }
  return  UPLL_RC_SUCCESS;
}

upll_rc_t VtnFlowFilterEntryMoMgr::UpdateMo(IpcReqRespHeader *req,
                                            ConfigKeyVal *ikey,
                                            DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *ctrl_key = NULL;
  upll_rc_t vtn_ctrlr_span_rt_code = UPLL_RC_SUCCESS;
  if (NULL == ikey || NULL == req) {
    UPLL_LOG_DEBUG(" UpdateMo Failed. Insufficient input parameters.");
    return UPLL_RC_ERR_GENERIC;
  }
  result_code = ValidateMessage(req, ikey);

  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("Validation of Message failed in UpdateMo");
    return result_code;
  }

  result_code = ValidateVtnFlowfilterEntryValue(ikey, req, dmi);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG(" ValidateVtnFlowfilterEntryValue Fail");
    return result_code;
  }

  result_code = ValidateAttribute(ikey, dmi, req);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("ValidateAttribute failed: err code(%d)", result_code);
    return result_code;
  }

  val_vtn_flowfilter_entry_t *vtn_ffe_val = reinterpret_cast
      <val_vtn_flowfilter_entry_t *>(GetVal(ikey));

  DbSubOp dbop = {kOpReadExist, kOpMatchNone, kOpInOutFlag};
  result_code = UpdateConfigDB(ikey, req->datatype, UNC_OP_READ,
                               dmi, &dbop, MAINTBL);
  if (UPLL_RC_ERR_INSTANCE_EXISTS != result_code) {
    UPLL_LOG_ERROR("UpdateConfigDB failed : %d", result_code);
    return result_code;
  }
  if (IsAllAttrInvalid(vtn_ffe_val)) {
    UPLL_LOG_INFO("No attributes to be updated");
    return UPLL_RC_SUCCESS;
  }

  std::list<controller_domain_t> list_ctrlr_dom;
  vtn_ctrlr_span_rt_code = GetVtnControllerSpan(ikey, req->datatype,
                                                dmi, list_ctrlr_dom);
  if ((vtn_ctrlr_span_rt_code != UPLL_RC_SUCCESS) &&
      (vtn_ctrlr_span_rt_code != UPLL_RC_ERR_NO_SUCH_INSTANCE)) {
    UPLL_LOG_DEBUG(" GetVtnControllerSpan  error code (%d)",
                   vtn_ctrlr_span_rt_code);
    return result_code;
  }

  if (vtn_ctrlr_span_rt_code == UPLL_RC_SUCCESS) {
    for (std::list<controller_domain_t>::iterator it= list_ctrlr_dom.begin();
         it != list_ctrlr_dom.end(); ++it) {
      /*result_code = ValidateCapability(
        req, ikey,
        reinterpret_cast<const char *>(it->ctrlr));
        if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Key not supported by controller");
        return result_code;
        }*/
    }
  } else if (UPLL_RC_ERR_NO_SUCH_INSTANCE != vtn_ctrlr_span_rt_code) {
    UPLL_LOG_INFO(" GetVtnControllerSpan failed . Resultcode %d ",
                   vtn_ctrlr_span_rt_code);
    return vtn_ctrlr_span_rt_code;
  }
  /* TODO Is Reference and GetChildConfigKey APis of NWM are declared as private
   *   Commented the below code to avoid compilation error of access violation
   *   nwm_momgr.cc code must be corrected */
#if 0
  result_code = IsNWMReferenced(ikey, dmi);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Given NetworkMonitor does not exists %d", result_code);
    return result_code;
  }
#endif
  // result_code = ValidateAttribute(ikey);

  // if (UPLL_RC_SUCCESS != result_code) return result_code;

  // Check and update the flowlist reference count if the flowlist object
  // is referred

  result_code = DupConfigKeyVal(ctrl_key, ikey, MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("DupConfigKeyVal err (%d)", result_code);
    for (std::list<controller_domain_t>::iterator it= list_ctrlr_dom.begin();
         it != list_ctrlr_dom.end(); ++it) {
      free(it->ctrlr);
      free(it->domain);
    }
    DELETE_IF_NOT_NULL(ctrl_key);
    return result_code;
  }
  TcConfigMode config_mode = TC_CONFIG_INVALID;
  std::string vtn_name = "";
  result_code = GetConfigModeInfo(req, config_mode, vtn_name);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetConfigMode failed");
    for (std::list<controller_domain_t>::iterator it= list_ctrlr_dom.begin();
         it != list_ctrlr_dom.end(); ++it) {
      free(it->ctrlr);
      free(it->domain);
    }
    DELETE_IF_NOT_NULL(ctrl_key);
    return result_code;
  }

  if (UNC_VF_VALID == vtn_ffe_val->valid[UPLL_IDX_FLOWLIST_NAME_VFFE] ||
        UNC_VF_VALID_NO_VALUE ==
        vtn_ffe_val->valid[UPLL_IDX_FLOWLIST_NAME_VFFE] ) {
      result_code = UpdateFlowListInCtrl(ikey, req->datatype,
                                         UNC_OP_UPDATE, dmi,
                                         config_mode, vtn_name);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Unable to update flowlist in ctrlr table");
      if (vtn_ctrlr_span_rt_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
        for (std::list<controller_domain_t>::iterator it =
             list_ctrlr_dom.begin();
             it != list_ctrlr_dom.end(); ++it) {
          free(it->ctrlr);
          free(it->domain);
        }
      }
      DELETE_IF_NOT_NULL(ctrl_key);
      return result_code;
    }
  }
  DbSubOp dbop1 = {kOpNotRead, kOpMatchNone, kOpInOutFlag};
  uint8_t temp_flag = 0;
  GET_USER_DATA_FLAGS(ikey, temp_flag);
  UPLL_LOG_DEBUG("GET_USER_DATA_FLAGS in Update %d", temp_flag);
  result_code = UpdateConfigDB(ikey, req->datatype, UNC_OP_UPDATE, dmi,
                               &dbop1, config_mode, vtn_name, MAINTBL);

  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("UpdateMo:Failed UpdateConfigDB ");
    if (vtn_ctrlr_span_rt_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      for (std::list<controller_domain_t>::iterator it= list_ctrlr_dom.begin();
         it != list_ctrlr_dom.end(); ++it) {
        free(it->ctrlr);
        free(it->domain);
      }
    }
    DELETE_IF_NOT_NULL(ctrl_key);
    return result_code;
  }
  // Update a record in CANDIDATE DB for controller Table
  if (vtn_ctrlr_span_rt_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    result_code = UpdateControllerTable(ctrl_key, UNC_OP_UPDATE,
                                        req->datatype, dmi,
                                        list_ctrlr_dom,
                                        config_mode, vtn_name);

    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("update in ctrlr tbl failed: error code (%d)",
                     result_code);
      /*result_code = UpdateConfigDB(ikey, req->datatype, UNC_OP_DELETE,
        dmi, MAINTBL);
        if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("delete in CandidateDB failed: err code(%d) ",
        result_code);
        }*/
    }
  }

  if (vtn_ctrlr_span_rt_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    for (std::list<controller_domain_t>::iterator it= list_ctrlr_dom.begin();
         it != list_ctrlr_dom.end(); ++it) {
      free(it->ctrlr);
      free(it->domain);
    }
  }
  DELETE_IF_NOT_NULL(ctrl_key);
  ctrl_key = NULL;
  return result_code;
}

upll_rc_t VtnFlowFilterEntryMoMgr::MergeImportToCandidate(
                                            unc_key_type_t keytype,
                                            const char *ctrlr_name,
                                            DalDmlIntf *dmi,
                                            upll_import_type import_type) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *ckval = NULL;
  ConfigKeyVal *ffe_imkey = NULL, *ffe_cdkey = NULL;
  ConfigVal *ctrlcv = NULL;
  ConfigKeyVal  *ckv_import = NULL, *ckv_cand = NULL;
  uint8_t flag = 0;
  string vtn_id = "";

  if (import_type == UPLL_IMPORT_TYPE_PARTIAL) {
    UPLL_LOG_DEBUG("Partial Import");
    return (MoMgrImpl::MergeImportToCandidate(keytype, ctrlr_name,
                                               dmi, import_type));
  }

  if (NULL == ctrlr_name) {
    UPLL_LOG_DEBUG("MergeValidate ctrlr_id NULL");
    return UPLL_RC_ERR_GENERIC;
  }

  VtnMoMgr *vtnmgr =
    static_cast<VtnMoMgr *>((const_cast<MoManager *>
        (GetMoManager(UNC_KT_VTN))));
  if (vtnmgr == NULL) {
    UPLL_LOG_DEBUG("Invalid momgr object for KT_VTN");
    return UPLL_RC_ERR_GENERIC;
  }
  result_code = vtnmgr->GetChildConfigKey(ckval, NULL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetChildConfigKey ckval NULL");
    return result_code;
  }

  DbSubOp dbop = { kOpReadMultiple, kOpMatchNone, kOpInOutFlag };
  /* Read all vtn from VTN main table in import database and check with
   * Candidate database */
  result_code = vtnmgr->ReadConfigDB(ckval, UPLL_DT_IMPORT,
              UNC_OP_READ, dbop, dmi, MAINTBL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("ReadConfigDB ckval NULL (%d)", result_code);
    if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
       UPLL_LOG_DEBUG("NO record in vtn tbl (%d)", result_code);
       result_code = UPLL_RC_SUCCESS;
    }
    DELETE_IF_NOT_NULL(ckval);
    return result_code;
  }
  ConfigKeyVal *tmp_ckval = ckval;
  while (ckval != NULL) {
    /* Get the instance count from vtn ctrl table in candidate.
     * If refcount is more than 1,
     *    which means that the imported vtn is already exists in candidate
     * If refcount is zero or 1,
     *    which means that the imported vtn is not exists in candidate
     */
    uint32_t imp_instance_count, cand_instance_count;
    /* Get the instance count from vtn ctrl tbl from import db*/
    result_code = vtnmgr->GetChildConfigKey(ckv_import, ckval);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("GetChildConfigKey failed %d", result_code);
      DELETE_IF_NOT_NULL(tmp_ckval);
      return result_code;
    }
    result_code = vtnmgr->GetInstanceCount(ckv_import, NULL,
         UPLL_DT_IMPORT, &imp_instance_count, dmi, CTRLRTBL);
    DELETE_IF_NOT_NULL(ckv_import);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("GetInstanceCount failed %d", result_code);
      DELETE_IF_NOT_NULL(tmp_ckval);
      return result_code;
    }

    /* Get the instance count from vtn ctrl tbl from candidate db*/
    result_code = vtnmgr->GetChildConfigKey(ckv_cand, ckval);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("GetChildConfigKey failed %d", result_code);
      DELETE_IF_NOT_NULL(tmp_ckval);
      return result_code;
    }
    result_code = vtnmgr->GetInstanceCount(ckv_cand, NULL,
         UPLL_DT_CANDIDATE, &cand_instance_count, dmi, CTRLRTBL);
    DELETE_IF_NOT_NULL(ckv_cand);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_INFO("GetInstanceCount failed %d", result_code);
      DELETE_IF_NOT_NULL(tmp_ckval);
      return result_code;
    }
    UPLL_LOG_TRACE("Import count (%d) Candidate count (%d)",
                    imp_instance_count, cand_instance_count);
    if (imp_instance_count == cand_instance_count) {
    /* If imported ctrlr's VTN not exists in Candidate, then check the
        existence of imported ctrlr's VTN flow-filter entry
        1)If the imported ctrlr VTN does not have flow-filter entry, then
          continue with the next VTN in imported db
        2)If the imported ctrlr VTN has flow-filter entry, then create this
          flow-filter entry into candidate db
     */
      UPLL_LOG_DEBUG("VTN not exists in candidate(%d)", result_code);

      // Get the imported ctrl VTN's flow-filter entry from Import database
      key_vtn_t *vtn_ikey = reinterpret_cast<key_vtn_t *>(ckval->get_key());
      key_vtn_flowfilter_entry_t *vtn_ffe_imkey = reinterpret_cast
       <key_vtn_flowfilter_entry_t*>(ConfigKeyVal::Malloc
       (sizeof(key_vtn_flowfilter_entry_t)));
      uuu::upll_strncpy(vtn_ffe_imkey->flowfilter_key.vtn_key.vtn_name,
                        vtn_ikey->vtn_name, kMaxLenVtnName+1);
      vtn_ffe_imkey->flowfilter_key.input_direction = 0xFE;
      ffe_imkey = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_ENTRY,
                             IpctSt::kIpcStKeyVtnFlowfilterEntry,
                             vtn_ffe_imkey, NULL);

      upll_rc_t result_import = ReadConfigDB(ffe_imkey, UPLL_DT_IMPORT,
             UNC_OP_READ, dbop, dmi, MAINTBL);
      if (result_import != UPLL_RC_SUCCESS &&
         result_import != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
         DELETE_IF_NOT_NULL(ffe_imkey);
         DELETE_IF_NOT_NULL(tmp_ckval);
         return result_code;
     }

     if (result_import == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
     /* If the imported ctrlr VTN does not have flow-filter entry, then continue
      * with the next VTN in imported db */
       UPLL_LOG_DEBUG("FF_Entry not exists in import(%d)", result_code);
       DELETE_IF_NOT_NULL(ffe_imkey);
       ckval = ckval->get_next_cfg_key_val();
       continue;
     } else if (result_import == UPLL_RC_SUCCESS) {
       /* If imported ctrlr VTN has flow-filter entry, then create this
        * flow-filter entry into candidate db */

       /* Get the list of this VTN associated ctrlr and domain */
       std::list<controller_domain_t> list_ctrlr_dom;
        upll_rc_t vtn_ctrlr_span_rt_code = GetVtnControllerSpan(ffe_imkey,
                                 UPLL_DT_IMPORT, dmi, list_ctrlr_dom);
        if (vtn_ctrlr_span_rt_code != UPLL_RC_SUCCESS) {
          DELETE_IF_NOT_NULL(ffe_imkey);
          if (vtn_ctrlr_span_rt_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
            UPLL_LOG_INFO("GetVtnControllerSpan  error code (%d)",
                         vtn_ctrlr_span_rt_code);
            DELETE_IF_NOT_NULL(tmp_ckval);
            return result_code;
          }
          /* If ctrl and domain name not exist in VTN ctrl tbl, then
             continue with the next VTN in import db */
          ckval = ckval->get_next_cfg_key_val();
          continue;
        }

        ConfigKeyVal *tmp_ffe_imkey = ffe_imkey;
        while (ffe_imkey != NULL) {
        /* Create the record in flow-filter entry main tbl */
        result_code = UpdateConfigDB(ffe_imkey, UPLL_DT_CANDIDATE,
                                     UNC_OP_CREATE, dmi, TC_CONFIG_GLOBAL,
                                     vtn_id, MAINTBL);
       if (result_code != UPLL_RC_SUCCESS) {
         UPLL_LOG_INFO("create in CandidateDB failed (%d) ", result_code);
         DELETE_IF_NOT_NULL(tmp_ffe_imkey);
         DELETE_IF_NOT_NULL(tmp_ckval);
         return result_code;
       }

       std::list<controller_domain_t>::iterator it= list_ctrlr_dom.begin();
       while (it != list_ctrlr_dom.end()) {
       /* Create the record in flow-filter entry ctrlr table with ctrlr
        * and domain */
         ConfigKeyVal *ctrlckv = NULL;
         GET_USER_DATA_FLAGS(ffe_imkey, flag);
         UPLL_LOG_DEBUG("flag (%d)", flag);

         key_vtn_flowfilter_entry_t *vtn_ffe_key =
           reinterpret_cast<key_vtn_flowfilter_entry_t*>
           (ConfigKeyVal::Malloc(sizeof(key_vtn_flowfilter_entry_t)));
         memcpy(vtn_ffe_key, reinterpret_cast<key_vtn_flowfilter_entry_t*>
            (ffe_imkey->get_key()), sizeof(key_vtn_flowfilter_entry_t));

         val_vtn_flowfilter_entry_t *vtn_ffe_val = reinterpret_cast
            <val_vtn_flowfilter_entry_t *>(GetVal(ffe_imkey));

         val_vtn_flowfilter_entry_ctrlr_t *ctrlr_val = reinterpret_cast
            <val_vtn_flowfilter_entry_ctrlr_t *>(ConfigKeyVal::Malloc(sizeof
            (val_vtn_flowfilter_entry_ctrlr_t)));
         /* Get the VALID from main table record and update into ctrl tbl*/
         for (unsigned int loop = 0;
            loop < (sizeof(ctrlr_val->valid)/sizeof(ctrlr_val->valid[0]));
            loop++) {
           if (UNC_VF_NOT_SUPPORTED == vtn_ffe_val->valid[loop]) {
             ctrlr_val->valid[loop] = UNC_VF_INVALID;
           } else {
             ctrlr_val->valid[loop] = vtn_ffe_val->valid[loop];
           }
        }

        ctrlcv = new ConfigVal(IpctSt::kIpcInvalidStNum, ctrlr_val);
        ctrlckv = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_ENTRY,
                               IpctSt::kIpcStKeyVtnFlowfilterEntry,
                               vtn_ffe_key, ctrlcv);

         SET_USER_DATA_CTRLR_DOMAIN(ctrlckv, *it);
         SET_USER_DATA_FLAGS(ctrlckv, flag);
         UPLL_LOG_DEBUG("flag (%d)", flag);

         // Create a record in ctrlr tbl in candidate db
         result_code = UpdateConfigDB(ctrlckv, UPLL_DT_CANDIDATE,
                                      UNC_OP_CREATE, dmi, TC_CONFIG_GLOBAL,
                                      vtn_id, CTRLRTBL);
         if (result_code != UPLL_RC_SUCCESS) {
           UPLL_LOG_INFO("Err during insert of the record in ctrlr table (%d)",
                         result_code);
           DELETE_IF_NOT_NULL(ctrlckv);
           DELETE_IF_NOT_NULL(tmp_ffe_imkey);
           DELETE_IF_NOT_NULL(tmp_ckval);
           return result_code;
         }
         DELETE_IF_NOT_NULL(ctrlckv);
         ++it;
       }
       ffe_imkey = ffe_imkey->get_next_cfg_key_val();
     }
     DELETE_IF_NOT_NULL(ffe_imkey);
     FREE_LIST_CTRLR(list_ctrlr_dom);
     }
  } else if (imp_instance_count < cand_instance_count) {
      /* If vtn exists in both db, then check the flow-filter entry existence
         from import and candidate database */
      UPLL_LOG_DEBUG("VTN exists in candidate(%d)", result_code);

      FlowListMoMgr *flowlist_mgr = reinterpret_cast<FlowListMoMgr *>
        (const_cast<MoManager *> (GetMoManager(UNC_KT_FLOWLIST)));
      if (NULL == flowlist_mgr) {
        UPLL_LOG_DEBUG("flowlist_mgr is NULL");
        DELETE_IF_NOT_NULL(tmp_ckval);
        return UPLL_RC_ERR_GENERIC;
      }

      // Get the flow-filter entries from Import database
      ffe_imkey = NULL;
      key_vtn_t *vtn_ikey = reinterpret_cast<key_vtn_t *>(ckval->get_key());
      key_vtn_flowfilter_entry_t *vtn_ffe_imkey = reinterpret_cast
        <key_vtn_flowfilter_entry_t*>(ConfigKeyVal::Malloc
        (sizeof(key_vtn_flowfilter_entry_t)));
      uuu::upll_strncpy(vtn_ffe_imkey->flowfilter_key.vtn_key.vtn_name,
                        vtn_ikey->vtn_name, kMaxLenVtnName+1);
      vtn_ffe_imkey->flowfilter_key.input_direction = 0xFE;
      ffe_imkey = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_ENTRY,
                              IpctSt::kIpcStKeyVtnFlowfilterEntry,
                              vtn_ffe_imkey, NULL);

      upll_rc_t result_import = ReadConfigDB(ffe_imkey, UPLL_DT_IMPORT,
              UNC_OP_READ, dbop, dmi, MAINTBL);
      if (result_import != UPLL_RC_SUCCESS &&
          result_import != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
          DELETE_IF_NOT_NULL(ffe_imkey);
          DELETE_IF_NOT_NULL(tmp_ckval);
          return result_code;
      }

      /* Get the flow-filter entry from candidate database */
      ffe_cdkey = NULL;
      key_vtn_flowfilter_entry_t *vtn_ffe_cdkey = reinterpret_cast
        <key_vtn_flowfilter_entry_t*>(ConfigKeyVal::Malloc
        (sizeof(key_vtn_flowfilter_entry_t)));
      uuu::upll_strncpy(vtn_ffe_cdkey->flowfilter_key.vtn_key.vtn_name,
                        vtn_ikey->vtn_name, kMaxLenVtnName+1);
      vtn_ffe_cdkey->flowfilter_key.input_direction = 0xFE;
      ffe_cdkey = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_ENTRY,
                              IpctSt::kIpcStKeyVtnFlowfilterEntry,
                              vtn_ffe_cdkey, NULL);

      upll_rc_t result_cand = ReadConfigDB(ffe_cdkey, UPLL_DT_CANDIDATE,
              UNC_OP_READ, dbop, dmi, MAINTBL);
       if (result_cand != UPLL_RC_SUCCESS &&
           result_cand != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
          DELETE_IF_NOT_NULL(ffe_imkey);
          DELETE_IF_NOT_NULL(ffe_cdkey);
          DELETE_IF_NOT_NULL(tmp_ckval);
          return result_code;
      }

      if ((result_import == UPLL_RC_ERR_NO_SUCH_INSTANCE ||
          result_import == UPLL_RC_SUCCESS) &&
          result_cand == UPLL_RC_SUCCESS) {
        /* If the UNC has flow-filter entries, then ignore the imported ctrlr's
         * flowfilter entry if exists and apply the unc's flow-filter entries
         * to imported ctrlr */

        if (result_import == UPLL_RC_SUCCESS) {
          /* If both VTN's are same,
           * skip the imported ctrl's flow-filter entry, when UNC
           * has flow-filter entry.
           * Then get the flowlist name from incoming key and decremet the
           * refcount in FLCTRL tbl in UNC candidate ff ctrl tbl. so that the
           * imported ctrl's flowlist and entries will not be applied into
           * PFC during Audit */
          ConfigKeyVal *tmp_ffe_imkey = ffe_imkey;
          result_code = DecRefCountInFLCtrlTbl(ffe_imkey, dmi, TC_CONFIG_GLOBAL,
                                               "");
          DELETE_IF_NOT_NULL(tmp_ffe_imkey);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("Err in DecRefCountInFLCtrl (%d)", result_code);
            DELETE_IF_NOT_NULL(ffe_cdkey);
            DELETE_IF_NOT_NULL(tmp_ckval);
            return result_code;
          }
        }

        /* Get the list of this VTN associated ctrlr and domain */
        std::list<controller_domain_t> list_ctrlr_dom;
        upll_rc_t vtn_ctrlr_span_rt_code = GetVtnControllerSpan(ffe_cdkey,
                                 UPLL_DT_IMPORT, dmi, list_ctrlr_dom);
        if (vtn_ctrlr_span_rt_code != UPLL_RC_SUCCESS) {
           DELETE_IF_NOT_NULL(ffe_cdkey);
           if (vtn_ctrlr_span_rt_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
             UPLL_LOG_DEBUG("GetVtnControllerSpan  error code (%d)",
                          vtn_ctrlr_span_rt_code);
             DELETE_IF_NOT_NULL(tmp_ckval);
             return result_code;
           }
           /* If ctrl and domain name not exist in VTN ctrl tbl, then
              continue with the next VTN in import db */
           ckval = ckval->get_next_cfg_key_val();
           continue;
        }

        ConfigKeyVal *tmp_ffe_cdkey = ffe_cdkey;
        while (ffe_cdkey != NULL) {
          std::list<controller_domain_t>::iterator it= list_ctrlr_dom.begin();
          while (it != list_ctrlr_dom.end()) {
            // Create the entry in ctrlr table with as per the ctrlr and domain
            ConfigKeyVal *ctrlckv = NULL;
            GET_USER_DATA_FLAGS(ffe_cdkey, flag);
            UPLL_LOG_DEBUG("flag (%d)", flag);

            key_vtn_flowfilter_entry_t *vtn_ffe_key =
              reinterpret_cast<key_vtn_flowfilter_entry_t*>
              (ConfigKeyVal::Malloc(sizeof(key_vtn_flowfilter_entry_t)));
            memcpy(vtn_ffe_key, reinterpret_cast<key_vtn_flowfilter_entry_t*>
              (ffe_cdkey->get_key()), sizeof(key_vtn_flowfilter_entry_t));

            val_vtn_flowfilter_entry_t *vtn_ffe_val = reinterpret_cast
              <val_vtn_flowfilter_entry_t *>(GetVal(ffe_cdkey));

            val_vtn_flowfilter_entry_ctrlr_t *ctrlr_val = reinterpret_cast
              <val_vtn_flowfilter_entry_ctrlr_t *>(ConfigKeyVal::Malloc(sizeof
              (val_vtn_flowfilter_entry_ctrlr_t)));

            /* Get the VALID from main table record and update into ctrl tbl*/
            for (unsigned int loop = 0;
              loop < (sizeof(ctrlr_val->valid)/sizeof(ctrlr_val->valid[0]));
              loop++) {
              if (UNC_VF_NOT_SUPPORTED == vtn_ffe_val->valid[loop]) {
                ctrlr_val->valid[loop] = UNC_VF_INVALID;
              } else {
                ctrlr_val->valid[loop] = vtn_ffe_val->valid[loop];
              }
            }

            ctrlcv = new ConfigVal(IpctSt::kIpcInvalidStNum, ctrlr_val);
            ctrlckv = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_ENTRY,
                               IpctSt::kIpcStKeyVtnFlowfilterEntry,
                               vtn_ffe_key, ctrlcv);

            SET_USER_DATA_CTRLR_DOMAIN(ctrlckv, *it);
            SET_USER_DATA_FLAGS(ctrlckv, flag);
            UPLL_LOG_DEBUG("flag (%d)", flag);

            /* When apply the UNC's flowfilter entry to imported ctrlr,
             * create the record in flctrl tbl(with the flowlist which get
             * it from incoming value structure, increment the refcount and
             * controller name of imported ctrl). So that the flowlist will
             * be apllied into imported ctrl during Audit. Domain name is not
             * required in flctrl tbl */
              std::string temp_vtn_name;
              temp_vtn_name = reinterpret_cast<const char*>(
                  reinterpret_cast<key_vtn_flowfilter_entry_t *>
                  (ffe_cdkey->get_key())->flowfilter_key.vtn_key.vtn_name);
              result_code = flowlist_mgr->AddFlowListToController(
              reinterpret_cast<char *>(vtn_ffe_val->flowlist_name), dmi,
              reinterpret_cast<char *>(it->ctrlr), UPLL_DT_CANDIDATE,
               UNC_OP_CREATE, TC_CONFIG_GLOBAL, temp_vtn_name, false);
              if (result_code != UPLL_RC_SUCCESS) {
                 UPLL_LOG_DEBUG("AddFlowListToController failed err code(%d)",
                       result_code);
                 DELETE_IF_NOT_NULL(tmp_ckval);
                 DELETE_IF_NOT_NULL(tmp_ffe_cdkey);
                 DELETE_IF_NOT_NULL(ctrlckv);
                 FREE_LIST_CTRLR(list_ctrlr_dom);
                 return result_code;
              }

            // Create a record in vtn flow-filter entry ctrl tbl
            result_code = UpdateConfigDB(ctrlckv, UPLL_DT_CANDIDATE,
                                         UNC_OP_CREATE, dmi, TC_CONFIG_GLOBAL,
                                         vtn_id, CTRLRTBL);
            if (result_code != UPLL_RC_SUCCESS) {
              UPLL_LOG_DEBUG("Err while inserting in ctrlr table(%d)",
                           result_code);
              DELETE_IF_NOT_NULL(ctrlckv);
              DELETE_IF_NOT_NULL(tmp_ffe_cdkey);
              DELETE_IF_NOT_NULL(tmp_ckval);
              FREE_LIST_CTRLR(list_ctrlr_dom);
              return result_code;
            }
            DELETE_IF_NOT_NULL(ctrlckv);
            ++it;
          }
          // IsRecordCreatedInFLCtrlrTbl = false;
          ffe_cdkey = ffe_cdkey->get_next_cfg_key_val();
        }
        FREE_LIST_CTRLR(list_ctrlr_dom);
        DELETE_IF_NOT_NULL(tmp_ffe_cdkey);
      } else if (result_import == UPLL_RC_SUCCESS &&
                 result_cand == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
        /* If candidate does not have flow-filter entry, then skip the imported
           ctrlr's flow-filter entries */

        /* If both VTN's are same and the candidate not have the
         * flow-filter entry, skip the imported ctrl's flow-filter entry
         * Then get the flowlist name from incoming key and decremet the
         * refcount in FLCTRL tbl. so that the imported ctrl's flowlist and
         * entries will not be applied into PFC during Audit */
        ConfigKeyVal *tmp_ffe_imkey = ffe_imkey;
        result_code = DecRefCountInFLCtrlTbl(ffe_imkey, dmi, TC_CONFIG_GLOBAL,
                                             "");
        DELETE_IF_NOT_NULL(tmp_ffe_imkey);
        DELETE_IF_NOT_NULL(ffe_cdkey);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("Err in DecRefCountInFLCtrl (%d)", result_code);
          DELETE_IF_NOT_NULL(tmp_ckval);
          return result_code;
        }
        UPLL_LOG_DEBUG("DecRefCountInFLCtrlTbl success (%d)", result_code);
      } else if (result_import == UPLL_RC_ERR_NO_SUCH_INSTANCE &&
                 result_cand == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
        DELETE_IF_NOT_NULL(ffe_imkey);
        DELETE_IF_NOT_NULL(ffe_cdkey);
      }
  }
  ckval = ckval->get_next_cfg_key_val();
  }
  DELETE_IF_NOT_NULL(tmp_ckval);
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtnFlowFilterEntryMoMgr::DecRefCountInFLCtrlTbl(
                             ConfigKeyVal *ffe_imkey, DalDmlIntf *dmi,
                             TcConfigMode config_mode, string vtn_name) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *ckval_dom = NULL;
  uint8_t *ctrlr_id = NULL;

  VtnMoMgr *vtnmgr =
    static_cast<VtnMoMgr *>((const_cast<MoManager *>
        (GetMoManager(UNC_KT_VTN))));

  FlowListMoMgr *flowlist_mgr = reinterpret_cast<FlowListMoMgr *>
    (const_cast<MoManager *> (GetMoManager(UNC_KT_FLOWLIST)));
  if ((NULL == vtnmgr) || (NULL == flowlist_mgr)) {
    UPLL_LOG_DEBUG("vtnmgr/flowlist_mgr is NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  /* Get the VTN associated ctrl name */
  result_code = vtnmgr->GetChildConfigKey(ckval_dom, NULL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetChildConfigKey ckval_dom NULL");
    return result_code;
  }

  key_vtn_t *vtn_okey = reinterpret_cast<key_vtn_t *>
                                 (ckval_dom->get_key());
  key_vtn_flowfilter_entry_t *vtn_ikey = reinterpret_cast
          <key_vtn_flowfilter_entry_t *>(ffe_imkey->get_key());
  uuu::upll_strncpy(vtn_okey->vtn_name,
           vtn_ikey->flowfilter_key.vtn_key.vtn_name, kMaxLenVtnName+1);
  DbSubOp dbop2 = {kOpReadSingle, kOpMatchNone, kOpInOutCtrlr};
  result_code = vtnmgr->ReadConfigDB(ckval_dom, UPLL_DT_IMPORT,
                          UNC_OP_READ, dbop2, dmi, CTRLRTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_INFO("VTN ctrltbl read failed (%d)", result_code);
    DELETE_IF_NOT_NULL(ckval_dom);
    return result_code;
  }

  GET_USER_DATA_CTRLR(ckval_dom, ctrlr_id);
  while (ffe_imkey != NULL) {
    UPLL_LOG_DEBUG("flow-filter not exists in candidate");

    val_vtn_flowfilter_entry_t *vtn_ffe_val = reinterpret_cast
         <val_vtn_flowfilter_entry_t *>(GetVal(ffe_imkey));
    std::string temp_vtn_name;
    if (TC_CONFIG_VTN == config_mode) {
      temp_vtn_name = vtn_name;
    } else {
      temp_vtn_name = reinterpret_cast<const char*>(vtn_ikey->
                                                    flowfilter_key.vtn_key.
                                                    vtn_name);
    }
    result_code = flowlist_mgr->AddFlowListToController(
         reinterpret_cast<char *>(vtn_ffe_val->flowlist_name), dmi,
         reinterpret_cast<char *>(ctrlr_id), UPLL_DT_CANDIDATE, UNC_OP_DELETE,
         config_mode, temp_vtn_name, false);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_INFO("AddFlowListToController failed err code(%d)",
                      result_code);
      DELETE_IF_NOT_NULL(ckval_dom);
      return result_code;
    }
    ffe_imkey = ffe_imkey->get_next_cfg_key_val();
  }
  DELETE_IF_NOT_NULL(ckval_dom);
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtnFlowFilterEntryMoMgr::MergeValidate(unc_key_type_t keytype,
    const char *ctrlr_id, ConfigKeyVal *ikey, DalDmlIntf *dmi,
    upll_import_type import_type) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_ERR_GENERIC;
  if (import_type == UPLL_IMPORT_TYPE_PARTIAL) {
    // Validate within IMPORT database for normal and multidomain case
    result_code = PI_MergeValidate_for_Vtn_Flowfilter_Entry(keytype,
      ctrlr_id, ikey, dmi);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_INFO("PI_MergeValidate failed:%d", result_code);
      return result_code;
    }

    unc_keytype_operation_t op1[] = { UNC_OP_DELETE, UNC_OP_CREATE,
                                      UNC_OP_UPDATE };
    int nop1 = sizeof(op1) / sizeof(op1[0]);

    // Validate with IMPORT database with RUNNING database
    result_code = ValidateImportWithRunning(keytype, ctrlr_id,
      ikey, op1, nop1, dmi);
    if ((result_code != UPLL_RC_SUCCESS) &&
        (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE)) {
    UPLL_LOG_INFO("ValidateImportWithRunning DB err (%d)", result_code);
    return result_code;
    }
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtnFlowFilterEntryMoMgr::RenameMo(IpcReqRespHeader *req,
                                            ConfigKeyVal *ikey, DalDmlIntf *dmi,
                                            const char *ctrlr_id) {
  UPLL_FUNC_TRACE;
  return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_KT;
}

upll_rc_t VtnFlowFilterEntryMoMgr::GetChildConfigKey(ConfigKeyVal *&okey,
                                                     ConfigKeyVal *parent_key) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  key_vtn_flowfilter_entry_t *vtn_ffe_key;
  void *pkey = NULL;
  if (parent_key == NULL) {
    vtn_ffe_key = reinterpret_cast <key_vtn_flowfilter_entry_t*>
        (ConfigKeyVal::Malloc(sizeof(key_vtn_flowfilter_entry_t)));
    // If no direction is specified , 0xFE is filled to bind output direction
    vtn_ffe_key->flowfilter_key.input_direction = 0xFE;
    okey = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVtnFlowfilterEntry,
                            vtn_ffe_key, NULL);
    UPLL_LOG_DEBUG("Parent Key Filled %d", result_code);
    return UPLL_RC_SUCCESS;
  } else {
    pkey = parent_key->get_key();
  }
  if (pkey == NULL)  {
    UPLL_LOG_DEBUG("GetChildConfigKey Failed:Parent key cannot be NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  if (okey) {
    if (okey->get_key_type() != UNC_KT_VTN_FLOWFILTER_ENTRY)
      return UPLL_RC_ERR_GENERIC;
  }
  if ((okey) && (okey->get_key())) {
    vtn_ffe_key = reinterpret_cast<key_vtn_flowfilter_entry_t *>
        (okey->get_key());
  } else {
    vtn_ffe_key = reinterpret_cast<key_vtn_flowfilter_entry_t *>
        (ConfigKeyVal::Malloc(sizeof(key_vtn_flowfilter_entry_t)));
    // If no direction is specified , 0xFE is filled to bind output direction
    vtn_ffe_key->flowfilter_key.input_direction = 0xFE;
  }
  switch (parent_key->get_key_type()) {
    case UNC_KT_VTN:
      uuu::upll_strncpy(vtn_ffe_key->flowfilter_key.vtn_key.vtn_name,
                        reinterpret_cast<key_vtn_t*>
                        (pkey)->vtn_name,
                        (kMaxLenVtnName + 1));
      break;
    case UNC_KT_VTN_FLOWFILTER:
      uuu::upll_strncpy(vtn_ffe_key->flowfilter_key.vtn_key.vtn_name,
                        reinterpret_cast<key_vtn_flowfilter_t*>
                        (pkey)->vtn_key.vtn_name,
                        (kMaxLenVtnName + 1));
      vtn_ffe_key->flowfilter_key.input_direction =
          reinterpret_cast<key_vtn_flowfilter_t *>
          (pkey)->input_direction;
      break;
    case UNC_KT_VTN_FLOWFILTER_ENTRY:
      uuu::upll_strncpy(vtn_ffe_key->flowfilter_key.vtn_key.vtn_name,
                        reinterpret_cast<key_vtn_flowfilter_entry_t*>
                        (pkey)->flowfilter_key.vtn_key.vtn_name,
                        (kMaxLenVtnName + 1));
      vtn_ffe_key->flowfilter_key.input_direction =
          reinterpret_cast<key_vtn_flowfilter_entry_t *>
          (pkey)->flowfilter_key.input_direction;
      vtn_ffe_key->sequence_num =
          reinterpret_cast<key_vtn_flowfilter_entry_t *>
          (pkey)->sequence_num;
      break;
    case UNC_KT_VBR_NWMONITOR:
      uuu::upll_strncpy(vtn_ffe_key->flowfilter_key.vtn_key.vtn_name,
                        reinterpret_cast<key_vtn_flowfilter_entry_t*>
                        (pkey)->flowfilter_key.vtn_key.vtn_name,
                        (kMaxLenVtnName + 1));
      break;
    default:
      if (vtn_ffe_key) free(vtn_ffe_key);
      return UPLL_RC_ERR_GENERIC;
  }

  if ((okey) && !(okey->get_key())) {
    UPLL_LOG_DEBUG("okey not null and flow list name updated");
    okey->SetKey(IpctSt::kIpcStKeyVtnFlowfilterEntry, vtn_ffe_key);
  }

  if (!okey)
    okey = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVtnFlowfilterEntry,
                            vtn_ffe_key, NULL);

  SET_USER_DATA(okey, parent_key);
  return result_code;
}

upll_rc_t VtnFlowFilterEntryMoMgr::GetRenamedControllerKey(
    ConfigKeyVal *ikey, upll_keytype_datatype_t dt_type, DalDmlIntf *dmi,
    controller_domain *ctrlr_dom) {
  UPLL_FUNC_TRACE;
  ConfigKeyVal *okey = NULL;
  upll_rc_t result_code;
  if (ctrlr_dom)
    UPLL_LOG_DEBUG("ctrlr : %s; domain : %s", ctrlr_dom->ctrlr,
                   ctrlr_dom->domain);
  UPLL_LOG_TRACE("Start Input ConfigKeyVal %s", ikey->ToStrAll().c_str());

  if (UNC_KT_VTN_FLOWFILTER_CONTROLLER == ikey->get_key_type()) {
    MoMgrImpl *vtn_mgr = reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>
        (GetMoManager(UNC_KT_VTN)));
    if (NULL == vtn_mgr) {
      UPLL_LOG_DEBUG("mgr NULL");
      return UPLL_RC_ERR_GENERIC;
    }

    result_code = vtn_mgr->GetChildConfigKey(okey, NULL);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("GetChildConfigKey fail (%d)", result_code);
      return result_code;
    }

    DbSubOp dbop = { kOpReadSingle, kOpMatchCtrlr | kOpMatchDomain,
      kOpInOutFlag };

    if (ctrlr_dom) {
      SET_USER_DATA_CTRLR_DOMAIN(okey, *ctrlr_dom);
    } else {
      UPLL_LOG_DEBUG("ctrlr_dom null");
      DELETE_IF_NOT_NULL(okey);
      return UPLL_RC_ERR_GENERIC;
    }

    UPLL_LOG_DEBUG("ctrlr : %s; domain : %s", ctrlr_dom->ctrlr,
        ctrlr_dom->domain);
    uuu::upll_strncpy(
        reinterpret_cast<key_vtn *>(okey->get_key())->vtn_name,
        reinterpret_cast<key_vtn_flowfilter_controller_t *>
        (ikey->get_key())->vtn_key.vtn_name,
        (kMaxLenVtnName + 1));

    UPLL_LOG_DEBUG("vtn name (%s) (%s)",
        reinterpret_cast<key_vtn *>(okey->get_key())->vtn_name,
        reinterpret_cast<key_vtn_flowfilter_controller_t *>
        (ikey->get_key())->vtn_key.vtn_name);

    result_code = vtn_mgr->ReadConfigDB(okey, dt_type, UNC_OP_READ, dbop, dmi,
        RENAMETBL);
    if (UPLL_RC_SUCCESS != result_code &&
        UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
      UPLL_LOG_DEBUG("GetRenamedControllerKey ReadConfigDB error");
      DELETE_IF_NOT_NULL(okey);
      return result_code;
    }
    if (UPLL_RC_SUCCESS == result_code) {
      val_rename_vtn *rename_val =
        reinterpret_cast<val_rename_vtn *>(GetVal(okey));
      if (!rename_val) {
        UPLL_LOG_DEBUG("rename_val NULL");
        DELETE_IF_NOT_NULL(okey);
        return UPLL_RC_ERR_GENERIC;
      }

      uuu::upll_strncpy(
          reinterpret_cast<key_vtn_flowfilter_controller_t*>
          (ikey->get_key())->vtn_key.vtn_name,
          rename_val->new_name,
          kMaxLenVtnName + 1);

      UPLL_LOG_DEBUG("renamed vtn_pm_ctrl  vtn name (%s) (%s)",
          reinterpret_cast<key_vtn_flowfilter_controller_t*>
          (ikey->get_key())->vtn_key.vtn_name,
          rename_val->new_name);
    }
    DELETE_IF_NOT_NULL(okey);
    vtn_mgr = NULL;
    return UPLL_RC_SUCCESS;
  }

  // Check if VTN is renamed on the controller by getting VTN object
  MoMgrImpl *mgr =  static_cast<MoMgrImpl*>
    ((const_cast<MoManager*>
      (GetMoManager(UNC_KT_VTN))));
  if (mgr == NULL) {
    UPLL_LOG_DEBUG("Invalid momgr object for KT_VTN");
    return UPLL_RC_ERR_GENERIC;
  }
  result_code = mgr->GetChildConfigKey(okey, NULL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetRenamedControllerKey:GetChildConfigKey returns error");
    return result_code;
  }

  if (ctrlr_dom == NULL) {
    UPLL_LOG_DEBUG("ctrlr_dom null");
    DELETE_IF_NOT_NULL(okey);
    return UPLL_RC_ERR_GENERIC;
  }
  SET_USER_DATA_CTRLR_DOMAIN(okey, *ctrlr_dom);

  UPLL_LOG_DEBUG("ctrlr : %s; domain : %s", ctrlr_dom->ctrlr,
      ctrlr_dom->domain);

  // Copy the input VTN Name into the Okey and send it for rename check IN db
  uuu::upll_strncpy(
      reinterpret_cast<key_vtn *>(okey->get_key())->vtn_name,
      reinterpret_cast<key_vtn_flowfilter_entry_t *>
      (ikey->get_key())->flowfilter_key.vtn_key.vtn_name,
      (kMaxLenVtnName + 1));

  UPLL_LOG_DEBUG("vtn name (%s) (%s)",
      reinterpret_cast<key_vtn *>(okey->get_key())->vtn_name,
      reinterpret_cast<key_vtn_flowfilter_entry_t *>
      (ikey->get_key())->flowfilter_key.vtn_key.vtn_name)

  DbSubOp dbop = { kOpReadSingle, kOpMatchCtrlr | kOpMatchDomain,
    kOpInOutFlag };
  result_code = mgr->ReadConfigDB(okey, dt_type,
                                  UNC_OP_READ, dbop, dmi, RENAMETBL);
  if (UPLL_RC_SUCCESS != result_code &&
      UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
    UPLL_LOG_DEBUG("GetRenamedControllerKey:Read Configuration Error");
    DELETE_IF_NOT_NULL(okey);
    return result_code;
  }
  if (UPLL_RC_SUCCESS == result_code) {
    val_rename_vtn *rename_val =reinterpret_cast <val_rename_vtn *>
      (GetVal(okey));
    if (!rename_val) {
      UPLL_LOG_ERROR("Rename structure for VTN is not available");
      DELETE_IF_NOT_NULL(okey);
      return UPLL_RC_ERR_GENERIC;
    }

    uuu::upll_strncpy(
        reinterpret_cast<key_vtn_flowfilter_entry_t*>
        (ikey->get_key())->flowfilter_key.vtn_key.vtn_name,
        rename_val->new_name,
        (kMaxLenVtnName + 1));
  }
  DELETE_IF_NOT_NULL(okey);
  val_vtn_flowfilter_entry_t *val_vtn_flofilter_entry = reinterpret_cast
    <val_vtn_flowfilter_entry_t *>(GetVal(ikey));

  if (NULL == val_vtn_flofilter_entry) {
    return UPLL_RC_SUCCESS;
  }
  if (strlen(reinterpret_cast<char *>
        (val_vtn_flofilter_entry->flowlist_name)) == 0) {
    return UPLL_RC_SUCCESS;
  }
  MoMgrImpl *mgr_flowlist =  static_cast<MoMgrImpl*>
    ((const_cast<MoManager*> (GetMoManager(UNC_KT_FLOWLIST))));

  result_code = mgr_flowlist->GetChildConfigKey(okey, NULL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetChildConfigKey fail");
    return result_code;
  }

  SET_USER_DATA_CTRLR(okey, ctrlr_dom->ctrlr);

  uuu::upll_strncpy(
      reinterpret_cast<key_flowlist_t *>
      (okey->get_key())->flowlist_name,
      reinterpret_cast<val_vtn_flowfilter_entry_t *> (ikey->get_cfg_val()->
        get_val())->flowlist_name,
      (kMaxLenFlowListName + 1));
  UPLL_LOG_DEBUG("flowlist name (%s) (%s)",
      reinterpret_cast<key_flowlist_t *>
      (okey->get_key())->flowlist_name,
      reinterpret_cast<val_vtn_flowfilter_entry_t *> (ikey->get_cfg_val()->
        get_val())->flowlist_name);
  DbSubOp dbop1 = { kOpReadSingle, kOpMatchCtrlr, kOpInOutFlag };
  result_code = mgr_flowlist->ReadConfigDB(okey, dt_type, UNC_OP_READ,
      dbop1, dmi, RENAMETBL); /* ctrlr_name */
  if (UPLL_RC_SUCCESS != result_code &&
      UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
    UPLL_LOG_DEBUG("GetRenamedControllerKey:Read Configuration Error");
    DELETE_IF_NOT_NULL(okey);
    return result_code;
  }
  if (UPLL_RC_SUCCESS == result_code) {
    val_rename_flowlist *rename_val =reinterpret_cast <val_rename_flowlist *>
      (GetVal(okey));
    if (!rename_val) {
      UPLL_LOG_DEBUG("flowlist is not valid");
      DELETE_IF_NOT_NULL(okey);
      return UPLL_RC_ERR_GENERIC;
    }

    uuu::upll_strncpy(
        reinterpret_cast<val_vtn_flowfilter_entry_t*>
        (ikey->get_cfg_val()->get_val())->flowlist_name,
        rename_val->flowlist_newname,
        (kMaxLenFlowListName + 1));
  }
  DELETE_IF_NOT_NULL(okey);
  UPLL_LOG_TRACE("End Input ConfigKeyVal %s",
      ikey->ToStrAll().c_str());
  return UPLL_RC_SUCCESS;
}


upll_rc_t VtnFlowFilterEntryMoMgr::GetRenamedUncKey(
    ConfigKeyVal *ikey, upll_keytype_datatype_t dt_type, DalDmlIntf *dmi,
    uint8_t *ctrlr_id) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_ERR_GENERIC;
  ConfigKeyVal *unc_key = NULL;
  uint8_t rename = 0;
  DbSubOp dbop = { kOpReadSingle, kOpMatchCtrlr,
    kOpInOutCtrlr | kOpInOutDomain };
  UPLL_LOG_TRACE("%s GetRenamedUncKey vtnffentry start",
                  ikey->ToStrAll().c_str());
  if ((ikey == NULL) || (ctrlr_id == NULL) || (dmi == NULL)) {
    UPLL_LOG_DEBUG("ikey/ctrlr_id dmi NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  val_rename_vtn *rename_vtn_key  = reinterpret_cast <val_rename_vtn*>
                                (ConfigKeyVal::Malloc(sizeof(val_rename_vtn)));

  if (!rename_vtn_key) {
    UPLL_LOG_DEBUG("rename_vtn_key NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  key_vtn_flowfilter_entry_t *ctrlr_key = reinterpret_cast
      <key_vtn_flowfilter_entry_t *> (ikey->get_key());
  if (!ctrlr_key) {
    UPLL_LOG_DEBUG("ctrlr_key NULL");
    if (rename_vtn_key) free(rename_vtn_key);
    return UPLL_RC_ERR_GENERIC;
  }

  uuu::upll_strncpy(rename_vtn_key->new_name,
                    ctrlr_key->flowfilter_key.vtn_key.vtn_name,
                    (kMaxLenVtnName + 1));
  rename_vtn_key->valid[UPLL_IDX_NEW_NAME_RVTN] = UNC_VF_VALID;

  MoMgrImpl *mgr = static_cast<MoMgrImpl*>
    ((const_cast<MoManager*>
    (GetMoManager(UNC_KT_VTN))));
  if (!mgr) {
    UPLL_LOG_TRACE("mgr failed");
    if (rename_vtn_key) free(rename_vtn_key);
    return UPLL_RC_ERR_GENERIC;
  }
  result_code = mgr->GetChildConfigKey(unc_key, NULL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetRenamedUnckey:GetChildConfigKey returned error");
    free(rename_vtn_key);
    mgr = NULL;
    return result_code;
  }
  if (!unc_key) {
    UPLL_LOG_DEBUG("unc_key NULL");
    free(rename_vtn_key);
    mgr = NULL;
    return UPLL_RC_ERR_GENERIC;
  }
  SET_USER_DATA_CTRLR(unc_key, ctrlr_id);
  unc_key->AppendCfgVal(IpctSt::kIpcStValRenameVtn, rename_vtn_key);
  result_code = mgr->ReadConfigDB(unc_key, dt_type, UNC_OP_READ, dbop,
                                            dmi, RENAMETBL);
  if ((UPLL_RC_SUCCESS != result_code) &&
      (UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code)) {
    UPLL_LOG_DEBUG("ReadConfigDB failed %d", result_code);
    DELETE_IF_NOT_NULL(unc_key);
    mgr = NULL;
    return result_code;
  }

  if (result_code == UPLL_RC_SUCCESS) {
    key_vtn *vtn_key = reinterpret_cast<key_vtn *> (unc_key->get_key());
    uuu::upll_strncpy(ctrlr_key->flowfilter_key.vtn_key.vtn_name,
                      vtn_key->vtn_name,
                      (kMaxLenVtnName + 1));
    rename |= VTN_RENAME;
    SET_USER_DATA(ikey, unc_key);
    SET_USER_DATA_FLAGS(ikey, rename);
  }
  mgr = NULL;
  DELETE_IF_NOT_NULL(unc_key);
  val_rename_flowlist *rename_fl =reinterpret_cast<val_rename_flowlist*>
                 (ConfigKeyVal::Malloc(sizeof(val_rename_flowlist)));
  if (!rename_fl) {
    UPLL_LOG_DEBUG("rename_fl NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  val_vtn_flowfilter_entry_t *val_vtn_flofilter_entry =reinterpret_cast
      <val_vtn_flowfilter_entry_t *>(GetVal(ikey));
  if (!val_vtn_flofilter_entry) {
    UPLL_LOG_DEBUG("val_vtn_flofilter_entry NULL");
    free(rename_fl);
    return UPLL_RC_SUCCESS;
  }
  if (UNC_VF_VALID != val_vtn_flofilter_entry
                      ->valid[UPLL_IDX_FLOWLIST_NAME_VFFE]) {
    UPLL_LOG_DEBUG("flowlist invalid");
    free(rename_fl);
    return UPLL_RC_SUCCESS;
  }
  uuu::upll_strncpy(rename_fl->flowlist_newname,
                    val_vtn_flofilter_entry->flowlist_name,
                    (kMaxLenFlowListName + 1));
  rename_fl->valid[UPLL_IDX_RENAME_FLOWLIST_RFL] = UNC_VF_VALID;

  mgr = static_cast<MoMgrImpl*>
    ((const_cast<MoManager*>
      (GetMoManager(UNC_KT_FLOWLIST))));
  if (!mgr) {
    UPLL_LOG_TRACE("mgr failed");
    if (rename_fl) free(rename_fl);
    return UPLL_RC_ERR_GENERIC;
  }
  result_code = mgr->GetChildConfigKey(unc_key, NULL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetChildConfigKey Returned an error");
    free(rename_fl);
    mgr = NULL;
    return result_code;
  }
  if (!unc_key) {
    UPLL_LOG_DEBUG("unc_key NULL");
    free(rename_fl);
    mgr = NULL;
    return UPLL_RC_ERR_GENERIC;
  }
  SET_USER_DATA_CTRLR(unc_key, ctrlr_id);
  unc_key->AppendCfgVal(IpctSt::kIpcStValRenameFlowlist, rename_fl);
  result_code = mgr->ReadConfigDB(unc_key, dt_type, UNC_OP_READ, dbop, dmi,
                                  RENAMETBL);
  if ((UPLL_RC_SUCCESS != result_code) &&
      (UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code)) {
    UPLL_LOG_DEBUG("ReadConfigDB failed %d", result_code);
    DELETE_IF_NOT_NULL(unc_key);
    mgr = NULL;
    return result_code;
  }

  if (result_code == UPLL_RC_SUCCESS) {
  key_flowlist_t *key_flowlist = reinterpret_cast
  <key_flowlist_t *>(unc_key->get_key());
    uuu::upll_strncpy(val_vtn_flofilter_entry->flowlist_name,
                      key_flowlist->flowlist_name,
                      (kMaxLenFlowListName + 1));
    rename |= FLOWLIST_RENAME;
    uint8_t *fl_ctrlr_id = NULL;
    GET_USER_DATA_CTRLR(unc_key, fl_ctrlr_id);
    SET_USER_DATA_CTRLR(ikey, fl_ctrlr_id);
    SET_USER_DATA_FLAGS(ikey, rename);
  }
  UPLL_LOG_TRACE("%s GetRenamedUncKey vtnffentry end",
                  ikey->ToStrAll().c_str());
  DELETE_IF_NOT_NULL(unc_key);
  mgr = NULL;
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtnFlowFilterEntryMoMgr::DupConfigKeyVal(ConfigKeyVal *&okey,
                                                   ConfigKeyVal *&req,
                                                   MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  if (req == NULL) {
  UPLL_LOG_DEBUG(" DupConfigKeyVal failed. Input ConfigKeyVal is NULL");
  return UPLL_RC_ERR_GENERIC;
  }
  if (okey != NULL) {
  UPLL_LOG_DEBUG(" DupConfigKeyVal failed. Output ConfigKeyVal is NULL");
  return UPLL_RC_ERR_GENERIC;
  }
  if ((req->get_key_type() != UNC_KT_VTN_FLOWFILTER_ENTRY )&&
        (req->get_key_type() != UNC_KT_VTN_FLOWFILTER_CONTROLLER)) {
  UPLL_LOG_DEBUG("Invalid keytype");
  return UPLL_RC_ERR_GENERIC;
  }
  ConfigVal *tmp1 = NULL, *tmp = (req)->get_cfg_val();

  if (tmp) {
    if (tbl == MAINTBL) {
      val_vtn_flowfilter_entry_t *ival = reinterpret_cast
        <val_vtn_flowfilter_entry_t *>(GetVal(req));
      if (NULL != ival) {
        val_vtn_flowfilter_entry_t *flowfilter_val =reinterpret_cast
             <val_vtn_flowfilter_entry_t*>
            (ConfigKeyVal::Malloc(sizeof(val_vtn_flowfilter_entry_t)));
        memcpy(flowfilter_val, ival, sizeof(val_vtn_flowfilter_entry_t));

        tmp1 = new ConfigVal(IpctSt::kIpcStValVtnFlowfilterEntry,
                             flowfilter_val);
      }
    }
    if (tbl == CTRLRTBL) {
      val_vtn_flowfilter_entry_ctrlr_t *ival = reinterpret_cast
        <val_vtn_flowfilter_entry_ctrlr_t *>(GetVal(req));
      if (NULL != ival) {
        val_vtn_flowfilter_entry_ctrlr_t *flowfilter_val =reinterpret_cast
               <val_vtn_flowfilter_entry_ctrlr_t* >(ConfigKeyVal::Malloc(sizeof
                                         (val_vtn_flowfilter_entry_ctrlr_t)));
        memcpy(flowfilter_val, ival, sizeof(val_vtn_flowfilter_entry_ctrlr_t));

        tmp1 = new ConfigVal(IpctSt::kIpcInvalidStNum,
                             flowfilter_val);
      }
    }
  }
  if (tmp1) {
    tmp1->set_user_data(tmp->get_user_data());
    //  tmp = tmp->get_next_cfg_val();// COV UNUSED
  }
  void *tkey = (req)->get_key();
  if (!tkey) {
    DELETE_IF_NOT_NULL(tmp1);
    return UPLL_RC_ERR_GENERIC;
  }
  key_vtn_flowfilter_entry_t *ikey =reinterpret_cast
           <key_vtn_flowfilter_entry_t *>(tkey);
  key_vtn_flowfilter_entry_t *vtn_flowfilterentrykey =reinterpret_cast
      <key_vtn_flowfilter_entry_t*>
      (ConfigKeyVal::Malloc(sizeof(key_vtn_flowfilter_entry_t(*ikey))));
  memcpy(vtn_flowfilterentrykey, ikey, sizeof(key_vtn_flowfilter_entry_t));
  okey = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_ENTRY,
                          IpctSt::kIpcStKeyVtnFlowfilterEntry,
                          vtn_flowfilterentrykey, tmp1);
  SET_USER_DATA(okey, req);
  // delete okey;
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtnFlowFilterEntryMoMgr::TxCopyCandidateToRunning(
    unc_key_type_t keytype, CtrlrCommitStatusList *ctrlr_commit_status,
    DalDmlIntf *dmi, TcConfigMode config_mode, string vtn_name) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  DalResultCode db_result;
  unc_keytype_operation_t op[] = { UNC_OP_DELETE, UNC_OP_CREATE,
    UNC_OP_UPDATE };
  int nop = sizeof(op) / sizeof(op[0]);
  ConfigKeyVal *vtn_ffe_key = NULL, *req = NULL, *nreq = NULL ,
      *vtn_ffe_run_key = NULL , *vtn_ffe_run_ctrl_key = NULL, *vtn_ck_run= NULL;
  DalCursor *cfg1_cursor = NULL;
  uint8_t *ctrlr_id = NULL;

  map<string, int> ctrlr_result;
  CtrlrCommitStatusList::iterator ccsListItr;
  CtrlrCommitStatus *ccStatusPtr;

  // mode is virtual and so ignore it
  if (config_mode == TC_CONFIG_VIRTUAL) {
    return UPLL_RC_SUCCESS;
  }

  if (ctrlr_commit_status != NULL) {
    for (ccsListItr = ctrlr_commit_status->begin();
        ccsListItr != ctrlr_commit_status->end(); ++ccsListItr) {
      ccStatusPtr = *ccsListItr;
      // ctrlr_id = reinterpret_cast<uint8_t* >((const_cast<char*>
      //                                      (ccStatusPtr->ctrlr_id.c_str())));
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
                "TxCopy:GetRenamedUncKey is failed, resultcode= %d",
                result_code);

            return result_code;
          }
        }
      }
    }
  }
  for (int i = 0; i < nop; i++) {
    cfg1_cursor = NULL;
    if (op[i] != UNC_OP_UPDATE) {
      result_code = DiffConfigDB(UPLL_DT_CANDIDATE, UPLL_DT_RUNNING, op[i], req,
                                 nreq, &cfg1_cursor, dmi, NULL, config_mode,
                                 vtn_name, MAINTBL, true);
      while (result_code == UPLL_RC_SUCCESS) {
        db_result = dmi->GetNextRecord(cfg1_cursor);
        result_code = DalToUpllResCode(db_result);
        if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
          result_code = UPLL_RC_SUCCESS;
          UPLL_LOG_DEBUG("No more diff found for operation %d", op[i]);
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
      if (cfg1_cursor)
        dmi->CloseCursor(cfg1_cursor, true);
      if (req)
        delete req;
      req = NULL;
    }
    UPLL_LOG_DEBUG("Updating main table complete with op %d", op[i]);
  }

  for (int i = 0; i < nop; i++) {
    cfg1_cursor = NULL;
    MoMgrTables tbl = (op[i] == UNC_OP_UPDATE)?MAINTBL:CTRLRTBL;
    result_code = DiffConfigDB(UPLL_DT_CANDIDATE, UPLL_DT_RUNNING, op[i], req,
                               nreq, &cfg1_cursor, dmi, NULL, config_mode,
                               vtn_name, tbl, true);
    ConfigKeyVal *vtn_ffe_ctrlr_key = NULL;
    while (result_code == UPLL_RC_SUCCESS) {
      db_result = dmi->GetNextRecord(cfg1_cursor);
      result_code = DalToUpllResCode(db_result);
      if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
        result_code = UPLL_RC_SUCCESS;
        UPLL_LOG_DEBUG("No more diff found for operation %d", op[i]);
        break;
      }
      if (op[i] == UNC_OP_UPDATE) {
        DbSubOp dbop = { kOpReadMultiple, kOpMatchNone,
          kOpInOutCtrlr|kOpInOutDomain | kOpInOutCs };
        result_code = GetChildConfigKey(vtn_ffe_ctrlr_key, req);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("GetChildConfigKey is failed resultcode=%d",
                         result_code);
          if (cfg1_cursor)
               dmi->CloseCursor(cfg1_cursor, true);
          DELETE_IF_NOT_NULL(req);
          DELETE_IF_NOT_NULL(nreq);
          return result_code;
        }
        result_code = ReadConfigDB(vtn_ffe_ctrlr_key, UPLL_DT_CANDIDATE,
                                   UNC_OP_READ, dbop, dmi, CTRLRTBL);
        if (result_code != UPLL_RC_SUCCESS) {
          delete vtn_ffe_ctrlr_key;
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
            if (cfg1_cursor)
               dmi->CloseCursor(cfg1_cursor, true);
            DELETE_IF_NOT_NULL(req);
            DELETE_IF_NOT_NULL(nreq);
            return result_code;
          }
        }
        for (ConfigKeyVal *tmp = vtn_ffe_ctrlr_key; tmp != NULL;
             tmp = tmp->get_next_cfg_key_val()) {
          GET_USER_DATA_CTRLR(tmp, ctrlr_id);
          string controller(reinterpret_cast<char *> (ctrlr_id));
          UPLL_LOG_DEBUG("Controller ID =%s", controller.c_str());
          DbSubOp dbop_maintbl = { kOpReadSingle, kOpMatchNone,
                                          kOpInOutFlag |kOpInOutCs };
          DbSubOp dbop_ctrtbl = { kOpReadSingle, kOpMatchCtrlr |
                         kOpMatchDomain, kOpInOutFlag | kOpInOutCs };
          result_code = GetChildConfigKey(vtn_ffe_key, req);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("GetChildConfigKey is failed resultcode=%d",
                           result_code);
            if (cfg1_cursor)
               dmi->CloseCursor(cfg1_cursor, true);
            DELETE_IF_NOT_NULL(req);
            DELETE_IF_NOT_NULL(nreq);
            return result_code;
          }
          result_code = ReadConfigDB(vtn_ffe_key, UPLL_DT_CANDIDATE ,
                                     UNC_OP_READ, dbop_maintbl, dmi, MAINTBL);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("Unable to read configuration from CandidateDb");
            DELETE_IF_NOT_NULL(vtn_ffe_key);
            if (cfg1_cursor)
              dmi->CloseCursor(cfg1_cursor, true);
            DELETE_IF_NOT_NULL(req);
            DELETE_IF_NOT_NULL(nreq);
            return result_code;
          }
          static_cast<val_vtn_flowfilter_entry_t *>
              (GetVal(vtn_ffe_key))->cs_row_status =
              static_cast<val_vtn_flowfilter_entry_t *>
                      (GetVal(nreq))->cs_row_status;
          // For Reading The Main table for config status
          result_code = GetChildConfigKey(vtn_ffe_run_key, req);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("GetChildConfigKey is failed resultcode=%d",
                           result_code);
            if (cfg1_cursor)
               dmi->CloseCursor(cfg1_cursor, true);
            DELETE_IF_NOT_NULL(req);
            DELETE_IF_NOT_NULL(nreq);
            return result_code;
          }
          result_code = ReadConfigDB(vtn_ffe_run_key, UPLL_DT_RUNNING  ,
                                     UNC_OP_READ, dbop_maintbl, dmi, MAINTBL);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("Unable to read configuration from RunningDB");
            DELETE_IF_NOT_NULL(vtn_ffe_key);
            DELETE_IF_NOT_NULL(vtn_ffe_run_key);
            if (cfg1_cursor)
               dmi->CloseCursor(cfg1_cursor, true);
            DELETE_IF_NOT_NULL(req);
            DELETE_IF_NOT_NULL(nreq);
            return result_code;
          }
          // For Reading The controller table for config status
          result_code = GetChildConfigKey(vtn_ffe_run_ctrl_key, tmp);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("GetChildConfigKey is failed resultcode=%d",
                           result_code);
            if (cfg1_cursor)
               dmi->CloseCursor(cfg1_cursor, true);
            DELETE_IF_NOT_NULL(req);
            DELETE_IF_NOT_NULL(nreq);
            return result_code;
          }
          result_code = ReadConfigDB(vtn_ffe_run_ctrl_key, UPLL_DT_RUNNING,
                                     UNC_OP_READ, dbop_ctrtbl, dmi, CTRLRTBL);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("Unable to read configuration from RunningDb");
            DELETE_IF_NOT_NULL(vtn_ffe_key);
            DELETE_IF_NOT_NULL(vtn_ffe_run_key);
            DELETE_IF_NOT_NULL(vtn_ffe_run_ctrl_key);
            if (cfg1_cursor)
              dmi->CloseCursor(cfg1_cursor, true);
            DELETE_IF_NOT_NULL(req);
            DELETE_IF_NOT_NULL(nreq);
            return result_code;
          }
          val_vtn_flowfilter_entry_ctrlr *val_ctrlr_can = reinterpret_cast
           <val_vtn_flowfilter_entry_ctrlr *>(GetVal(tmp));
          val_vtn_flowfilter_entry_ctrlr *val_ctrlr_run = reinterpret_cast
            <val_vtn_flowfilter_entry_ctrlr *>(GetVal(vtn_ffe_run_ctrl_key));
          val_ctrlr_can->cs_row_status =val_ctrlr_run->cs_row_status;

          for (unsigned int loop = 0; loop < sizeof(val_ctrlr_run->valid)/
           sizeof(val_ctrlr_run->valid[0]); ++loop) {
           val_ctrlr_can->cs_attr[loop] = val_ctrlr_run->cs_attr[loop];
          }
          // End Reading The controller table for config status
          val_vtn_flowfilter_entry_t *val_main_can = reinterpret_cast
           <val_vtn_flowfilter_entry_t *>(GetVal(vtn_ffe_key));
          val_vtn_flowfilter_entry_t *val_main = reinterpret_cast
           <val_vtn_flowfilter_entry_t *>(GetVal(vtn_ffe_run_key));

          for (unsigned int loop = 0; loop < sizeof(val_main->valid)/
            sizeof(val_main->valid[0]); ++loop) {
            val_main_can->cs_attr[loop] = val_main->cs_attr[loop];
          }
          DELETE_IF_NOT_NULL(vtn_ffe_run_ctrl_key);
          if (ctrlr_result.empty()) {
            UPLL_LOG_TRACE("ctrlr_commit_status is NULL");
            result_code = UpdateConfigStatus(vtn_ffe_key, op[i],
                                             UPLL_RC_ERR_CTR_DISCONNECTED,
                                             nreq, dmi, tmp);
          } else {
            result_code = UpdateConfigStatus(vtn_ffe_key, op[i],
                                             ctrlr_result[controller],
                                             nreq, dmi, tmp);
          }
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("UpdateConfigStatus failed, err %d", result_code);
            break;
          }
          DELETE_IF_NOT_NULL(vtn_ffe_run_key);
          DELETE_IF_NOT_NULL(vtn_ffe_run_ctrl_key);
          void *vtnffe_ctrlval = GetVal(tmp);
          CompareValidVal(vtnffe_ctrlval, GetVal(nreq), GetVal(req), false);
          result_code = UpdateConfigDB(tmp, UPLL_DT_RUNNING,
                                       op[i], dmi, config_mode, vtn_name,
                                       CTRLRTBL);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("!!! Failed to Update the controller table, err %d",
                           result_code);
            DELETE_IF_NOT_NULL(vtn_ffe_key);
            if (cfg1_cursor)
              dmi->CloseCursor(cfg1_cursor, true);
            DELETE_IF_NOT_NULL(req);
            DELETE_IF_NOT_NULL(nreq);
            return result_code;
          }
          result_code = UpdateConfigDB(vtn_ffe_key,
                                       UPLL_DT_RUNNING, op[i], dmi, config_mode,
                                       vtn_name, MAINTBL);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("Update to Main table failed %d", result_code);
            DELETE_IF_NOT_NULL(vtn_ffe_key);
            if (cfg1_cursor)
              dmi->CloseCursor(cfg1_cursor, true);
            DELETE_IF_NOT_NULL(req);
            DELETE_IF_NOT_NULL(nreq);
            return result_code;
          }
          EnqueCfgNotification(op[i], UPLL_DT_RUNNING,
                               vtn_ffe_key);
          DELETE_IF_NOT_NULL(vtn_ffe_key);
        }
      } else {
        if (op[i] == UNC_OP_CREATE) {
          DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutFlag
                                                   | kOpInOutCs };
          result_code = GetChildConfigKey(vtn_ffe_key, req);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("GetChildConfigKey is failed resultcode=%d",
                           result_code);
            if (cfg1_cursor)
              dmi->CloseCursor(cfg1_cursor, true);
            DELETE_IF_NOT_NULL(req);
            DELETE_IF_NOT_NULL(nreq);
            return result_code;
          }
          result_code = ReadConfigDB(vtn_ffe_key, UPLL_DT_RUNNING ,
                                     UNC_OP_READ, dbop, dmi, MAINTBL);
          if (result_code != UPLL_RC_SUCCESS &&
               result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE ) {
            UPLL_LOG_DEBUG("Unable to read configuration from CandidateDb");
            if (cfg1_cursor)
              dmi->CloseCursor(cfg1_cursor, true);
            DELETE_IF_NOT_NULL(req);
            DELETE_IF_NOT_NULL(nreq);
            return result_code;
          }
          result_code = DupConfigKeyVal(vtn_ffe_ctrlr_key, req, tbl);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("Failed to create duplicate ConfigKeyVal Err (%d)",
                         result_code);
            delete vtn_ffe_key;
            if (cfg1_cursor)
              dmi->CloseCursor(cfg1_cursor, true);
            DELETE_IF_NOT_NULL(req);
            DELETE_IF_NOT_NULL(nreq);
            return result_code;
          }
          //  set consolidated config status to UNKNOWN to init vtn cs_status
          //  to the cs_status of first controller
          uint32_t cur_instance_count;
          result_code = GetInstanceCount(vtn_ffe_key, NULL,
                                   UPLL_DT_CANDIDATE, &cur_instance_count,
                                   dmi, CTRLRTBL);
          if ((result_code == UPLL_RC_SUCCESS) && (cur_instance_count == 1))
            reinterpret_cast<val_vtn_flowfilter_entry*>
                    (GetVal(vtn_ffe_key))->cs_row_status = UNC_CS_UNKNOWN;
          GET_USER_DATA_CTRLR(vtn_ffe_ctrlr_key, ctrlr_id);
          string controller(reinterpret_cast<char *> (ctrlr_id));
          if (ctrlr_result.empty()) {
            UPLL_LOG_TRACE("ctrlr_commit_status is NULL");
            result_code = UpdateConfigStatus(vtn_ffe_key, op[i],
                                             UPLL_RC_ERR_CTR_DISCONNECTED,
                                             nreq , dmi, vtn_ffe_ctrlr_key);
          } else {
            result_code = UpdateConfigStatus(vtn_ffe_key, op[i],
                                             ctrlr_result[controller], nreq ,
                                             dmi, vtn_ffe_ctrlr_key);
          }
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("Error in updating configstatus, resultcode=%d",
                           result_code);
            DELETE_IF_NOT_NULL(vtn_ffe_ctrlr_key);
            if (cfg1_cursor)
              dmi->CloseCursor(cfg1_cursor, true);
            return result_code;
          }

        } else if (op[i] == UNC_OP_DELETE) {
          // Reading Main Running DB for delete op
           DbSubOp dbop1 = { kOpReadSingle, kOpMatchNone, kOpInOutFlag
                                                        | kOpInOutCs };
           result_code = GetChildConfigKey(vtn_ck_run, req);
           if (result_code != UPLL_RC_SUCCESS) {
             UPLL_LOG_DEBUG("GetChildConfigKey is failed resultcode=%d",
                         result_code);
             if (cfg1_cursor)
               dmi->CloseCursor(cfg1_cursor, true);
             return result_code;
           }
           result_code = ReadConfigDB(vtn_ck_run, UPLL_DT_RUNNING,
                                   UNC_OP_READ, dbop1, dmi, MAINTBL);
           if (result_code != UPLL_RC_SUCCESS &&
                 result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
             UPLL_LOG_DEBUG("Unable to read configuration from CandidateDb");
             DELETE_IF_NOT_NULL(vtn_ck_run);
             if (cfg1_cursor)
               dmi->CloseCursor(cfg1_cursor, true);
             return result_code;
           }
          GET_USER_DATA_CTRLR(req, ctrlr_id);
          if (result_code == UPLL_RC_SUCCESS) {
            result_code = SetVtnFFEntryConsolidatedStatus(vtn_ck_run,
                                                          ctrlr_id, dmi);
            if (result_code != UPLL_RC_SUCCESS) {
              UPLL_LOG_DEBUG("Could not set consolidated status %d",
                                                   result_code);
              DELETE_IF_NOT_NULL(vtn_ck_run);
              if (cfg1_cursor)
                dmi->CloseCursor(cfg1_cursor, true);
              DELETE_IF_NOT_NULL(req);
              DELETE_IF_NOT_NULL(nreq);
              return result_code;
            }
           }
           DELETE_IF_NOT_NULL(vtn_ck_run);
          result_code = GetChildConfigKey(vtn_ffe_ctrlr_key, req);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("Error in getting the configkey, resultcode=%d",
                           result_code);
            if (cfg1_cursor)
              dmi->CloseCursor(cfg1_cursor, true);
            DELETE_IF_NOT_NULL(vtn_ck_run);
            DELETE_IF_NOT_NULL(req);
            DELETE_IF_NOT_NULL(nreq);
            return result_code;
          }
        }
        result_code = UpdateConfigDB(vtn_ffe_ctrlr_key,
                                     UPLL_DT_RUNNING, op[i], dmi, config_mode,
                                     vtn_name, CTRLRTBL);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("DB Error while updating controller table. err:%d",
                         result_code);
          if (cfg1_cursor)
              dmi->CloseCursor(cfg1_cursor, true);
          DELETE_IF_NOT_NULL(vtn_ck_run);
          DELETE_IF_NOT_NULL(req);
          DELETE_IF_NOT_NULL(nreq);
          DELETE_IF_NOT_NULL(vtn_ffe_ctrlr_key);
          return result_code;
        }
        if (op[i] != UNC_OP_DELETE) {
          result_code = UpdateConfigDB(vtn_ffe_key, UPLL_DT_RUNNING,
                                       UNC_OP_UPDATE, dmi, config_mode,
                                       vtn_name, MAINTBL);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("UpdateConfigDB in main tbl is failed -%d",
                           result_code);
            if (cfg1_cursor)
              dmi->CloseCursor(cfg1_cursor, true);
            DELETE_IF_NOT_NULL(vtn_ck_run);
            DELETE_IF_NOT_NULL(req);
            DELETE_IF_NOT_NULL(nreq);
            DELETE_IF_NOT_NULL(vtn_ffe_key);
            return result_code;
          }
        }
        EnqueCfgNotification(op[i], UPLL_DT_RUNNING,
                             vtn_ffe_key);
      }
      // delete vtn_flowfilter_entry_ctrlr_key;
      if (vtn_ffe_key) delete vtn_ffe_key;
      DELETE_IF_NOT_NULL(vtn_ffe_ctrlr_key);
      vtn_ffe_key = vtn_ffe_ctrlr_key = NULL;
      result_code = DalToUpllResCode(db_result);
    }
    if (cfg1_cursor)
      dmi->CloseCursor(cfg1_cursor, true);
    if (req) delete req;
    if (nreq) delete nreq;
    nreq = req = NULL;
  }
  result_code = (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE)?
      UPLL_RC_SUCCESS:result_code;
  UPLL_LOG_DEBUG("TxCopy returned with resultcode %d", result_code);
  return result_code;
}

  upll_rc_t
  VtnFlowFilterEntryMoMgr::SetVtnFFEntryConsolidatedStatus(ConfigKeyVal *ikey,
                                   uint8_t *ctrlr_id,
                                   DalDmlIntf *dmi)  {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *ctrlr_ckv = NULL;
  val_vtn_flowfilter_entry_ctrlr *ctrlr_val = NULL;
  uint8_t *vtn_ffe_exist_on_ctrlr = NULL;
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
  result_code = ReadConfigDB(ctrlr_ckv, UPLL_DT_RUNNING,
                                 UNC_OP_READ, dbop, dmi,
                                 CTRLRTBL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("ReadConfigDB from ctrltbl failed err code %d",
                   result_code);
    delete ctrlr_ckv;
    return result_code;
  }

  for (ConfigKeyVal *tmp = ctrlr_ckv; tmp != NULL;
                     tmp = tmp->get_next_cfg_key_val()) {
    ctrlr_val = reinterpret_cast<val_vtn_flowfilter_entry_ctrlr *>
                          (GetVal(tmp));
    if (!ctrlr_val) {
      UPLL_LOG_DEBUG("Controller Value is empty");
      tmp = NULL;
      DELETE_IF_NOT_NULL(ctrlr_ckv);
      return UPLL_RC_ERR_GENERIC;
    }
    GET_USER_DATA_CTRLR(tmp, vtn_ffe_exist_on_ctrlr);
    UPLL_LOG_DEBUG("Controllername from DB %s", vtn_ffe_exist_on_ctrlr);
    if (!strcmp(reinterpret_cast<char *>(vtn_ffe_exist_on_ctrlr),
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
    vtn_ffe_exist_on_ctrlr = NULL;
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
  applied = not_applied = false;
  // Set cs_status
  val_vtn_flowfilter_entry_t *vtn_ffe_val =
     static_cast<val_vtn_flowfilter_entry_t *>(GetVal(ikey));
  vtn_ffe_val->cs_row_status = c_status;
  for (unsigned int loop = 0; loop < sizeof(vtn_ffe_val->valid)/
           sizeof(vtn_ffe_val->valid[0]); ++loop) {
    for (ConfigKeyVal *tmp = ctrlr_ckv; tmp != NULL;
                     tmp = tmp->get_next_cfg_key_val()) {
      ctrlr_val =
          reinterpret_cast<val_vtn_flowfilter_entry_ctrlr *>(GetVal(tmp));

      GET_USER_DATA_CTRLR(tmp, vtn_ffe_exist_on_ctrlr);
      if (!strcmp(reinterpret_cast<char *>(vtn_ffe_exist_on_ctrlr),
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
    vtn_ffe_val->cs_attr[loop] = c_status;
    applied = not_applied =false;
  }


  DbSubOp dbop_update = {kOpNotRead, kOpMatchNone, kOpInOutCs};
  result_code = UpdateConfigDB(ikey, UPLL_DT_RUNNING, UNC_OP_UPDATE, dmi,
                               &dbop_update, TC_CONFIG_GLOBAL, vtn_name,
                               MAINTBL);
  DELETE_IF_NOT_NULL(ctrlr_ckv);
  return result_code;
}

#if 0
upll_rc_t VtnFlowFilterEntryMoMgr::UpdateConfigStatus(
    ConfigKeyVal *vtn_flow_filter_entry_key, unc_keytype_operation_t op,
    uint32_t driver_result, ConfigKeyVal *nreq, DalDmlIntf *dmi,
    ConfigKeyVal *ctrlr_key) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  unc_keytype_configstatus_t status = UNC_CS_UNKNOWN, cs_status;
  cs_status = (driver_result == 0) ? UNC_CS_APPLIED : UNC_CS_NOT_APPLIED;
  val_vtn_flowfilter_entry_t *vtn_ff_entry_val = reinterpret_cast
      <val_vtn_flowfilter_entry_t *> (GetVal(vtn_flow_filter_entry_key));
  val_vtn_flowfilter_entry_ctrlr *ctrlr_val_ff_entry =
  reinterpret_cast<val_vtn_flowfilter_entry_ctrlr *> (GetVal(ctrlr_key));
  if ((vtn_ff_entry_val == NULL)
      || (ctrlr_val_ff_entry == NULL)) {
  UPLL_LOG_DEBUG("Memory Allocation failed for Valstructure");
  return UPLL_RC_ERR_GENERIC;
  }

  if (op == UNC_OP_CREATE) {
   /* for (unsigned int ctrl_attr = 0; ctrl_attr <
      sizeof(ctrlr_val_ff_entry->valid) / sizeof(ctrlr_val_ff_entry->valid[0]);
        ++ctrl_attr) {
      ctrlr_val_ff_entry->valid[ctrl_attr] = UNC_VF_INVALID;
      }
*/
    /* update the vtn status in main tbl */
    switch (vtn_ff_entry_val->cs_row_status) {
      case UNC_CS_UNKNOWN:
        status = cs_status;
        break;
      case UNC_CS_PARTIALLY_APPLIED:
        if (ctrlr_val_ff_entry->cs_row_status
            == UNC_CS_NOT_APPLIED) {
          // if this vtn has caused it then to change to applied.
        }
        break;
      case UNC_CS_APPLIED:
      case UNC_CS_NOT_APPLIED:
      case UNC_CS_INVALID:
      default:
        status =
            (cs_status == UNC_CS_APPLIED) ? UNC_CS_PARTIALLY_APPLIED : status;
        break;
    }
    vtn_ff_entry_val->cs_row_status = status;
    for (unsigned int loop = 0; loop <
      sizeof(vtn_ff_entry_val->valid) /sizeof(vtn_ff_entry_val->valid[0]);
      ++loop) {
      if (UNC_VF_NOT_SUPPORTED == vtn_ff_entry_val->valid[loop]) {
        vtn_ff_entry_val->cs_attr[loop] = UNC_CS_NOT_SUPPORTED;
        continue;
      }
      if (UNC_VF_NOT_SUPPORTED == ctrlr_val_ff_entry->valid[loop]) {
        ctrlr_val_ff_entry->cs_attr[loop] = UNC_CS_NOT_SUPPORTED;
        continue;
      }
      if ((UNC_VF_VALID == vtn_ff_entry_val->valid[loop]) ||
         (UNC_VF_VALID_NO_VALUE == vtn_ff_entry_val->valid[loop]))
        if (ctrlr_val_ff_entry->valid[loop] != UNC_VF_NOT_SUPPORTED) {
          ctrlr_val_ff_entry->cs_attr[loop] = cs_status;
          vtn_ff_entry_val->cs_attr[loop] =
            static_cast<uint8_t>(vtn_ff_entry_val->cs_row_status);
        }
    }
  } else if (op == UNC_OP_UPDATE) {
    void *vtnffval = reinterpret_cast<void *>(vtn_ff_entry_val);
    CompareValidValue(vtnffval, GetVal(nreq), false);

    for (unsigned int loop = 0;
        loop < sizeof(ctrlr_val_ff_entry->valid) / sizeof(
                                                 ctrlr_val_ff_entry->valid[0]);
        ++loop) {
      if (ctrlr_val_ff_entry->valid[loop] != UNC_VF_NOT_SUPPORTED) {
        ctrlr_val_ff_entry->cs_attr[loop] = cs_status;
      } else {
        ctrlr_val_ff_entry->cs_attr[loop] = UNC_CS_NOT_SUPPORTED;
        vtn_ff_entry_val->cs_attr[loop] =
        static_cast<uint8_t>(vtn_ff_entry_val->cs_row_status);
     }
    }
  }
  return result_code;
}
#endif

upll_rc_t VtnFlowFilterEntryMoMgr::UpdateAuditConfigStatus(
    unc_keytype_configstatus_t cs_status,
    uuc::UpdateCtrlrPhase phase,
    ConfigKeyVal *&ckv_running,
    DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  val_vtn_flowfilter_entry_ctrlr *val;
  val = (ckv_running != NULL)?
        reinterpret_cast<val_vtn_flowfilter_entry_ctrlr *>
        (GetVal(ckv_running)):NULL;
  if (NULL == val) {
    return UPLL_RC_ERR_GENERIC;
  }
  if (uuc::kUpllUcpCreate == phase )
    val->cs_row_status = cs_status;
  if ((uuc::kUpllUcpUpdate == phase) &&
           (val->cs_row_status == UNC_CS_INVALID ||
            val->cs_row_status == UNC_CS_NOT_APPLIED))
    val->cs_row_status = cs_status;
  for (unsigned int loop = 0;
      loop < sizeof(val->valid)/sizeof(uint8_t); ++loop) {
    if ((cs_status == UNC_CS_INVALID && UNC_VF_VALID == val->valid[loop]) ||
         cs_status == UNC_CS_APPLIED)
       val->cs_attr[loop] = cs_status;
  }
  return result_code;
}

upll_rc_t VtnFlowFilterEntryMoMgr::SetConsolidatedStatus(ConfigKeyVal *ikey,
                                                         DalDmlIntf *dmi) {
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *ckv = NULL;
  string vtn_name = "";
  DbSubOp dbop = {kOpReadMultiple, kOpMatchNone, kOpInOutCs};
  result_code = GetChildConfigKey(ckv, ikey);
  if (UPLL_RC_SUCCESS != result_code) {
    return result_code;
  }
  result_code = ReadConfigDB(ckv, UPLL_DT_RUNNING, UNC_OP_READ,
                             dbop, dmi, CTRLRTBL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_ERROR("ReadConfigDB failed:%d", result_code);
    delete ckv;
    return result_code;
  }

  std::vector<list<unc_keytype_configstatus_t> > vec_attr;
  std::list< unc_keytype_configstatus_t > list_cs_row;
  std::list< unc_keytype_configstatus_t > list_cs_attr;
  val_vtn_flowfilter_entry_ctrlr *val;
  for (unsigned int loop = 0; loop < sizeof(val->valid)/sizeof(val->valid[0]);
      ++loop) {
    std::list< unc_keytype_configstatus_t > list_attr;
    vec_attr.push_back(list_attr);
  }
  ConfigKeyVal *temp_ckv = ckv;
  for (; temp_ckv != NULL; temp_ckv =  temp_ckv->get_next_cfg_key_val()) {
      val = reinterpret_cast<val_vtn_flowfilter_entry_ctrlr*>
            (GetVal(temp_ckv));
      list_cs_row.push_back((unc_keytype_configstatus_t)val->cs_row_status);
      for (unsigned int loop = 0;
            loop < sizeof(val->valid)/sizeof(val->valid[0]);
        ++loop) {
        vec_attr[loop].push_back(
                      (unc_keytype_configstatus_t)val->cs_attr[loop]);
    }
  }
  DELETE_IF_NOT_NULL(ckv);
  val_vtn_flowfilter_entry_t *val_temp =
  reinterpret_cast<val_vtn_flowfilter_entry_t *>(GetVal(ikey));
  val_temp->cs_row_status = GetConsolidatedCsStatus(list_cs_row);
  for (unsigned int loop = 0; loop < sizeof(val->valid)/sizeof(val->valid[0]);
      ++loop) {
    val_temp->cs_attr[loop] = GetConsolidatedCsStatus(vec_attr[loop]);
  }
  result_code = UpdateConfigDB(ikey, UPLL_DT_RUNNING,
                               UNC_OP_UPDATE, dmi, TC_CONFIG_GLOBAL, vtn_name,
                               MAINTBL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_ERROR("UpdateConfigDB failed:%d", result_code);
    return result_code;
  }
  return result_code;
}

upll_rc_t VtnFlowFilterEntryMoMgr::ValidateAttribute(ConfigKeyVal *ikey,
                                                     DalDmlIntf *dmi,
                                                     IpcReqRespHeader *req) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *okey = NULL;
  if (!ikey || !ikey->get_key()) {
    UPLL_LOG_DEBUG("input key is null");
    return UPLL_RC_ERR_GENERIC;
  }
  MoMgrImpl *mgr = NULL;

  // read val_vtn_flowfilter_entry from ikey
  val_vtn_flowfilter_entry_t *val_vtn_flowfilter_entry =
      static_cast<val_vtn_flowfilter_entry_t *>(
          ikey->get_cfg_val()->get_val());
  if (val_vtn_flowfilter_entry->valid[UPLL_IDX_FLOWLIST_NAME_VFFE]
      == UNC_VF_VALID) {
    // validate flowlist_name in in FLOWLIST table
    mgr = reinterpret_cast<MoMgrImpl *>
        (const_cast<MoManager *>(GetMoManager(UNC_KT_FLOWLIST)));

    if (NULL == mgr) {
      UPLL_LOG_DEBUG("Unable to get FLOWLIST object");
      return UPLL_RC_ERR_GENERIC;
    }

    /** allocate memory for FLOWLIST key_struct */
    result_code = mgr->GetChildConfigKey(okey, NULL);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Memory allocation failed for FLOWLIST key struct - %d",
                     result_code);
      return result_code;
    }

    /** fill key_flowlist_t from val_vtn_flowfilter_entry*/
    key_flowlist_t *key_flowlist = static_cast<key_flowlist_t*>(
        okey->get_key());
    uuu::upll_strncpy(key_flowlist->flowlist_name,
                      val_vtn_flowfilter_entry->flowlist_name,
                      kMaxLenFlowListName+1);

    UPLL_LOG_DEBUG("Flowlist name in val_vtn_flowfilter_entry %s",
                   key_flowlist->flowlist_name);

    /* Check flowlist_name exists in table*/
    result_code = mgr->UpdateConfigDB(okey, req->datatype,
                                      UNC_OP_READ, dmi, MAINTBL);

    if (UPLL_RC_ERR_INSTANCE_EXISTS != result_code) {
      if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE)
        result_code = UPLL_RC_ERR_CFG_SEMANTIC;
      UPLL_LOG_DEBUG("Flowlist name in val_vtn_flowfilter_entry does not exists"
                     "in FLOWLIST table");
      delete okey;
      okey = NULL;
      return result_code;
    } else {
      result_code = UPLL_RC_SUCCESS;
    }
    // Get mode id and vtn_name from tc
    if (req->datatype == UPLL_DT_CANDIDATE) {
      TcConfigMode config_mode = TC_CONFIG_INVALID;
      std::string vtn_name = "";
      result_code = GetConfigModeInfo(req, config_mode, vtn_name);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("GetConfigModeInfo failed");
        delete okey;
        okey = NULL;
        return result_code;
      }

      // mode is vtn mode, verifies flowlist existance in running tbl
      if (config_mode == TC_CONFIG_VTN) {
        result_code = mgr->UpdateConfigDB(okey, UPLL_DT_RUNNING,
                                       UNC_OP_READ, dmi, MAINTBL);

        if (UPLL_RC_ERR_INSTANCE_EXISTS != result_code) {
          UPLL_LOG_ERROR("Flowlist name in val_vtn_flowfilter_entry does not"
                         "exists in running FLOWLIST table");
          delete okey;
          okey = NULL;
          return UPLL_RC_ERR_CFG_SEMANTIC;
        } else {
          result_code = UPLL_RC_SUCCESS;
        }
      }
    }
    delete okey;
    okey = NULL;
  }

  if (req->datatype == UPLL_DT_IMPORT) {
    if (val_vtn_flowfilter_entry->valid[UPLL_IDX_NWN_NAME_VFFE]
        == UNC_VF_VALID) {
      // validate nwm_name in KT_VBR_NWMONITOR table
      mgr = reinterpret_cast<MoMgrImpl *>
        (const_cast<MoManager *>(GetMoManager(UNC_KT_VBR_NWMONITOR)));

      if (NULL == mgr) {
        UPLL_LOG_DEBUG("Unable to get KT_VBR_NWMONITOR object");
        return UPLL_RC_ERR_GENERIC;
      }

      /** allocate memory for key_nwm key_struct */
      result_code = mgr->GetChildConfigKey(okey, NULL);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Memory allocation failed for key_nwm struct - %d",
                     result_code);
        return result_code;
      }

      /* read key_vtn_vtn_flowfilter_entry from ikey*/
      key_vtn_flowfilter_entry_t *key_vtn_flowfilter_entry =
        static_cast<key_vtn_flowfilter_entry_t *>(
            ikey->get_key());

      /** fill key_nwm from key/val VTN_FLOWFILTER_ENTRY structs*/
      key_nwm_t *key_nwm = static_cast<key_nwm_t*>(
        okey->get_key());

      uuu::upll_strncpy(key_nwm->nwmonitor_name,
                      val_vtn_flowfilter_entry->nwm_name,
                      kMaxLenNwmName+1);

      uuu::upll_strncpy(key_nwm->vbr_key.vtn_key.vtn_name,
                      key_vtn_flowfilter_entry->flowfilter_key.vtn_key.vtn_name,
                      kMaxLenVtnName+1);

      /* Check nwm_name exists in table*/
      DbSubOp dbop = {kOpReadSingle, kOpMatchNone, kOpInOutNone};
      result_code = mgr->ReadConfigDB(okey, req->datatype, UNC_OP_READ,
                                    dbop, dmi, MAINTBL);

      if (UPLL_RC_SUCCESS != result_code) {
        if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
          UPLL_LOG_DEBUG("NWM name in value structure doesn't exist");
          delete okey;
          return UPLL_RC_ERR_CFG_SEMANTIC;
        }
        UPLL_LOG_DEBUG("ReadConfigDB Error, resultcode = %d", result_code);
        delete okey;
        return result_code;
      }

      delete okey;
      okey = NULL;
    }  // nwm_name is valid
  }
  UPLL_LOG_DEBUG("ValidateAttribute Successfull.");
  return result_code;
}

upll_rc_t VtnFlowFilterEntryMoMgr::AllocVal(ConfigVal *&ck_val,
                                            upll_keytype_datatype_t dt_type,
                                            MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  void *val;  //  *ck_nxtval;
  if (ck_val != NULL) {
    UPLL_LOG_DEBUG("Memory is already allocated for configval pointer");
    return UPLL_RC_ERR_GENERIC;
  }
  switch (tbl) {
    case MAINTBL:
      // TODO(UNC): Need to handle the datatype as DT_STATE case
      val = ConfigKeyVal::Malloc(sizeof(val_vtn_flowfilter_entry_t));
      ck_val = new ConfigVal(IpctSt::kIpcStValVtnFlowfilterEntry, val);
      break;
    case CTRLRTBL:
     val = ConfigKeyVal::Malloc(sizeof(val_vtn_flowfilter_entry_ctrlr_t));
     ck_val = new ConfigVal(IpctSt::kIpcInvalidStNum, val);
     break;
    default:
      val = NULL;
      return UPLL_RC_ERR_GENERIC;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtnFlowFilterEntryMoMgr::ReadMo(IpcReqRespHeader *req,
                                          ConfigKeyVal *ikey,
                                          DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  controller_domain ctrlr_dom;
  result_code = ValidateMessage(req, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG(" ValidateMessage Fail err code(%d)", result_code);
    return result_code;
  }

  memset(&ctrlr_dom, 0, sizeof(controller_domain));
  if (ikey->get_key_type() == UNC_KT_VTN_FLOWFILTER_ENTRY) {
    result_code = ReadInfoFromDB(req, ikey, dmi, &ctrlr_dom);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG(" Read request failed result(%d)", result_code);
    }
  } else if (ikey->get_key_type() == UNC_KT_VTN_FLOWFILTER_CONTROLLER) {
    result_code = ReadFlowFilterController(req, ikey, dmi);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("ReadFlowFilterController failed error (%d)",
                     result_code);
    }
  } else {
    UPLL_LOG_DEBUG(" Invalid Keytype recived");
    result_code = UPLL_RC_ERR_BAD_REQUEST;
  }
  return result_code;
}

upll_rc_t VtnFlowFilterEntryMoMgr::ReadFlowFilterController(
    IpcReqRespHeader *req,
    ConfigKeyVal *ikey,
    DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  controller_domain ctrlr_dom;
  ConfigKeyVal   *okey = NULL;
  DbSubOp dbop1 = { kOpReadExist, kOpMatchNone, kOpInOutNone};
  key_vtn_flowfilter_controller_t *key_vtn_flowfilter_ctrl =
      reinterpret_cast<key_vtn_flowfilter_controller_t*>(ikey->get_key());
  val_flowfilter_controller_t *ival = reinterpret_cast
                 <val_flowfilter_controller_t *>(GetVal(ikey));

  key_vtn_flowfilter_entry_t *key_vtn_ffe_ctrl =
      reinterpret_cast <key_vtn_flowfilter_entry_t*>
      (ConfigKeyVal::Malloc(sizeof(key_vtn_flowfilter_entry_t)));

  ConfigKeyVal *dup_ckmain = new ConfigKeyVal(
      UNC_KT_VTN_FLOWFILTER_ENTRY,
      IpctSt::kIpcStKeyVtnFlowfilterEntry,
      key_vtn_ffe_ctrl, NULL);

  uuu::upll_strncpy(
      key_vtn_ffe_ctrl->flowfilter_key.vtn_key.vtn_name,
      key_vtn_flowfilter_ctrl->vtn_key.vtn_name,
      (kMaxLenVtnName +1));
  key_vtn_ffe_ctrl->sequence_num = 0;
  key_vtn_ffe_ctrl->flowfilter_key.input_direction = 0xFE;
  if (ival->valid[UPLL_IDX_SEQ_NUM_FFC] == UNC_VF_VALID) {
    key_vtn_ffe_ctrl->sequence_num = ival->sequence_num;
  }
  if (ival->valid[UPLL_IDX_DIRECTION_FFC] == UNC_VF_VALID) {
    key_vtn_ffe_ctrl->flowfilter_key.input_direction = ival->direction;
  }
  // vtn is not configured in vtn_ff_entry main tbl
  // only vtn/vbr is configured and commited
  result_code = UpdateConfigDB(dup_ckmain, req->datatype,
                               UNC_OP_READ, dmi, &dbop1, MAINTBL);
  if (result_code != UPLL_RC_ERR_INSTANCE_EXISTS) {
    UPLL_LOG_ERROR("Requested Vtn is Not Configured in"
           "flowfilterEntryMain Table in Candidate %d", result_code);
    DELETE_IF_NOT_NULL(dup_ckmain);
    return result_code;
  }
  // vtn is not configured in vtn_ff_entry ctrlr tbl
  // vtn/ff/entry is configured but no vbr is configured

  // Setting direction again to 0XFE
  // because it will be set to 0 in updateconfigDB
  key_vtn_ffe_ctrl->flowfilter_key.input_direction = 0xFE;

  if (ival->valid[UPLL_IDX_SEQ_NUM_FFC] == UNC_VF_VALID) {
    key_vtn_ffe_ctrl->sequence_num = ival->sequence_num;
  }
  if (ival->valid[UPLL_IDX_DIRECTION_FFC] == UNC_VF_VALID) {
    key_vtn_ffe_ctrl->flowfilter_key.input_direction = ival->direction;
  }
  SET_USER_DATA_CTRLR(dup_ckmain, key_vtn_flowfilter_ctrl->controller_name);
  SET_USER_DATA_DOMAIN(dup_ckmain, key_vtn_flowfilter_ctrl->domain_id);

  dbop1.matchop = kOpMatchCtrlr | kOpMatchDomain;
  result_code = UpdateConfigDB(dup_ckmain, req->datatype,
                UNC_OP_READ, dmi, &dbop1, CTRLRTBL);
  DELETE_IF_NOT_NULL(dup_ckmain);

  if (result_code != UPLL_RC_ERR_INSTANCE_EXISTS) {
    UPLL_LOG_ERROR("Requested Configuration is not Configured in"
                   "VtnflowfilterEntry Table %d", result_code);
    return UPLL_RC_ERR_NOT_ALLOWED_AT_THIS_TIME;
  }

  switch (req->datatype) {
    case UPLL_DT_STATE:
      if ((req->option1 == UNC_OPT1_NORMAL) &&
          (req->option2 == UNC_OPT2_NONE)) {
        result_code  =  ReadControllerStateNormal(ikey, req->datatype,
                                                  req->operation, dmi);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("ReadControllerStateNormal failed err code (%d)",
                         result_code);
          return result_code;
        }
      } else if ((req->datatype == UPLL_DT_STATE)&&
                 ((req->option1 == UNC_OPT1_DETAIL) &&
                  (req->option2 == UNC_OPT2_NONE))) {
        ctrlr_dom.ctrlr = reinterpret_cast <uint8_t*>
            (ConfigKeyVal::Malloc(kMaxLenCtrlrId + 1));
        ctrlr_dom.domain = reinterpret_cast <uint8_t*>
            (ConfigKeyVal::Malloc(kMaxLenDomainId + 1));
        uuu::upll_strncpy(ctrlr_dom.ctrlr,
                          key_vtn_flowfilter_ctrl->controller_name,
                          (kMaxLenCtrlrId + 1));
        uuu::upll_strncpy(ctrlr_dom.domain,
                          key_vtn_flowfilter_ctrl->domain_id,
                          (kMaxLenDomainId + 1));
        result_code = ValidateCapability(req, ikey,
                       reinterpret_cast<char *>(ctrlr_dom.ctrlr));
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("Key not supported by controller ReadMo ");
          FREE_IF_NOT_NULL(ctrlr_dom.ctrlr);
          FREE_IF_NOT_NULL(ctrlr_dom.domain);
          return result_code;
        }

        ConfigVal *tmp1 = NULL;
        val_flowfilter_controller_t *l_val =reinterpret_cast
            <val_flowfilter_controller_t*>
            (ConfigKeyVal::Malloc(sizeof(val_flowfilter_controller_t)));

        memcpy(l_val, ival, sizeof(val_flowfilter_controller_t));
        if (ival->valid[UPLL_IDX_SEQ_NUM_FFC] != UNC_VF_VALID) {
          l_val->sequence_num = 0;
        }

        tmp1 = new ConfigVal(IpctSt::kIpcStValFlowfilterController, l_val);
        key_vtn_flowfilter_controller_t *l_key_vtn_flowfilter_ctrl =
            reinterpret_cast<key_vtn_flowfilter_controller_t*>
            (ConfigKeyVal::Malloc(sizeof(key_vtn_flowfilter_controller_t)));
        memcpy(l_key_vtn_flowfilter_ctrl, key_vtn_flowfilter_ctrl,
               sizeof(key_vtn_flowfilter_controller_t));
        ConfigKeyVal *l_key = new ConfigKeyVal(
            UNC_KT_VTN_FLOWFILTER_CONTROLLER,
            IpctSt::kIpcStKeyVtnFlowfilterController,
            l_key_vtn_flowfilter_ctrl, tmp1);
        result_code = GetRenamedControllerKey(l_key, req->datatype, dmi,
                                              &ctrlr_dom);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("GetRenamedControllerKey Failed %d", result_code);
          DELETE_IF_NOT_NULL(l_key);
          FREE_IF_NOT_NULL(ctrlr_dom.ctrlr);
          FREE_IF_NOT_NULL(ctrlr_dom.domain);
          return result_code;
        }
          // Sending the request to driver
          IpcResponse ipc_resp;
          memset(&ipc_resp, 0, sizeof(IpcResponse));
          IpcRequest ipc_req;
          memset(&ipc_req, 0, sizeof(IpcRequest));
          ipc_req.header.clnt_sess_id = req->clnt_sess_id;
          ipc_req.header.config_id = req->config_id;
          ipc_req.header.operation = UNC_OP_READ;
          ipc_req.header.option1 = req->option1;
          ipc_req.header.datatype = req->datatype;

          ipc_req.ckv_data = l_key;
          if (!IpcUtil::SendReqToDriver(
                  (const char *)ctrlr_dom.ctrlr,
                  reinterpret_cast<char *>(ctrlr_dom.domain),
                  PFCDRIVER_SERVICE_NAME, PFCDRIVER_SVID_LOGICAL,
                  &ipc_req, true, &ipc_resp)) {
            UPLL_LOG_DEBUG("SendReqToDriver failed for Key %d controller %s",
                           l_key->get_key_type(),
                           reinterpret_cast<char *>(ctrlr_dom.ctrlr));
            DELETE_IF_NOT_NULL(l_key);
            DELETE_IF_NOT_NULL(ipc_resp.ckv_data);
            FREE_IF_NOT_NULL(ctrlr_dom.ctrlr);
            FREE_IF_NOT_NULL(ctrlr_dom.domain);
            return ipc_resp.header.result_code;
          }

          if (ipc_resp.header.result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_INFO("Request for Key %d failed in %s with error %d",
                          l_key->get_key_type(), ctrlr_dom.ctrlr,
                          ipc_resp.header.result_code);
            DELETE_IF_NOT_NULL(ipc_resp.ckv_data);
            DELETE_IF_NOT_NULL(l_key);
            FREE_IF_NOT_NULL(ctrlr_dom.ctrlr);
            FREE_IF_NOT_NULL(ctrlr_dom.domain);
            return ipc_resp.header.result_code;
          }
          result_code = ReadControllerStateDetail(ikey,
                                                  ipc_resp.ckv_data,
                                                  &ctrlr_dom, &okey);
          if (result_code!= UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("ReadControllerStateDetail Fail err code (%d)",
                           result_code);
            DELETE_IF_NOT_NULL(okey);
            DELETE_IF_NOT_NULL(ipc_resp.ckv_data);
            DELETE_IF_NOT_NULL(l_key);
            FREE_IF_NOT_NULL(ctrlr_dom.ctrlr);
            FREE_IF_NOT_NULL(ctrlr_dom.domain);
            return result_code;
          }
        if (okey != NULL) {
          ikey->ResetWith(okey);
          DELETE_IF_NOT_NULL(okey);
        }
        DELETE_IF_NOT_NULL(l_key);
        DELETE_IF_NOT_NULL(ipc_resp.ckv_data);
        FREE_IF_NOT_NULL(ctrlr_dom.ctrlr);
        FREE_IF_NOT_NULL(ctrlr_dom.domain);
      }
      break;
    default:
      UPLL_LOG_DEBUG("Operation Not Allowed");
      result_code = UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT;
      break;
  }
  return result_code;
}

bool VtnFlowFilterEntryMoMgr::IsValidKey(void *key, uint64_t index,
                                         MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  key_vtn_flowfilter_entry_t *vtn_ff_key =
    reinterpret_cast<key_vtn_flowfilter_entry_t*> (key);
  upll_rc_t ret_val;
  switch (index) {
    case uudst::vtn_flowfilter_entry::kDbiVtnName:
      ret_val = ValidateKey(reinterpret_cast<char *>
                            (vtn_ff_key->flowfilter_key.vtn_key.vtn_name),
                            kMinLenVtnName,
                            kMaxLenVtnName);
      if (ret_val != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("VTN Name is not valid(%d)", ret_val);
        return false;
      }
    break;
    case uudst::vtn_flowfilter_entry::kDbiInputDirection:
      if (vtn_ff_key->flowfilter_key.input_direction == 0xFE) {
        // if operation is read sibling begin or
        // read sibling count return false
        // for output binding
        vtn_ff_key->flowfilter_key.input_direction = 0;
        return false;
      } else {
        // do normal validation.
        if (!ValidateNumericRange(vtn_ff_key->flowfilter_key.input_direction,
                                  (uint8_t) UPLL_FLOWFILTER_DIR_IN,
                                  (uint8_t) UPLL_FLOWFILTER_DIR_OUT,
                                  true, true)) {
          UPLL_LOG_DEBUG(" input direction syntax validation failed ");
          return false;
        }
      }
    break;
    case uudst::vtn_flowfilter_entry::kDbiSequenceNum:
      if (!ValidateNumericRange(vtn_ff_key->sequence_num,
                            kMinFlowFilterSeqNum, kMaxFlowFilterSeqNum, true,
                            true)) {
         UPLL_LOG_DEBUG(" Sequence Number syntax validation failed ");
         return false;
     }
     break;
  }
  return true;
}

bool VtnFlowFilterEntryMoMgr::CompareKey(void *key1, void *key2) {
  UPLL_FUNC_TRACE;
  key_vtn_flowfilter_entry_t  *key1_vtn_flowfilter, *key2_vtn_flowfilter;
  bool match = false;
  key1_vtn_flowfilter = reinterpret_cast<key_vtn_flowfilter_entry_t*>(key1);
  key2_vtn_flowfilter = reinterpret_cast<key_vtn_flowfilter_entry_t *>(key2);
  if (key1_vtn_flowfilter == key2_vtn_flowfilter) {
  match =true;
  }
  if ((!key1_vtn_flowfilter) || (!key2_vtn_flowfilter)) {
  match = false;
  } else {  // COV FORWARD NULL
  if (strncmp(reinterpret_cast<const char *>
             (key1_vtn_flowfilter->flowfilter_key.vtn_key.vtn_name),
              reinterpret_cast<const char *>
                         (key2_vtn_flowfilter->flowfilter_key.vtn_key.vtn_name),
              32) == 0) {
    match =true;
    }
  }
  return match;
}

upll_rc_t VtnFlowFilterEntryMoMgr::GetValid(void *val,
                                            uint64_t indx,
                                            uint8_t *&valid,
                                            upll_keytype_datatype_t dt_type,
                                            MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  val_vtn_flowfilter_entry_t* l_val_ff = NULL;
  val_vtn_flowfilter_entry_ctrlr *l_val_ctrl_ff = NULL;
  if (val == NULL) {
  UPLL_LOG_DEBUG("Value structure cannot be null");
  return UPLL_RC_ERR_GENERIC;
  }
  if (tbl != MAINTBL &&  tbl != CTRLRTBL) {
    valid = NULL;
    return UPLL_RC_ERR_GENERIC;
  }

    l_val_ff = reinterpret_cast
                       <val_vtn_flowfilter_entry_t*>(val);
    l_val_ctrl_ff = reinterpret_cast
                       <val_vtn_flowfilter_entry_ctrlr*>(val);
    switch (indx) {
      case uudst::vtn_flowfilter_entry::kDbiFlowlistName:
      valid  = GET_VALID_MAINCTRL(tbl, l_val_ctrl_ff, l_val_ff,
                                   UPLL_IDX_FLOWLIST_NAME_VFFE);
        break;
      case uudst::vtn_flowfilter_entry::kDbiNwnName:
      valid  = GET_VALID_MAINCTRL(tbl, l_val_ctrl_ff, l_val_ff,
                                   UPLL_IDX_NWN_NAME_VFFE);
        break;
      case uudst::vtn_flowfilter_entry::kDbiAction:
      valid  = GET_VALID_MAINCTRL(tbl, l_val_ctrl_ff, l_val_ff,
                                   UPLL_IDX_ACTION_VFFE);
        break;
      case uudst::vtn_flowfilter_entry::kDbiDscp:
      valid  = GET_VALID_MAINCTRL(tbl, l_val_ctrl_ff, l_val_ff,
                                   UPLL_IDX_DSCP_VFFE);
        break;
      case uudst::vtn_flowfilter_entry::kDbiPriority:
      valid  = GET_VALID_MAINCTRL(tbl, l_val_ctrl_ff, l_val_ff,
                                   UPLL_IDX_PRIORITY_VFFE);
        break;
      default:
        return UPLL_RC_ERR_GENERIC;
    }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtnFlowFilterEntryMoMgr::ValidateVtnFlowfilterEntryKey(
    key_vtn_flowfilter_entry_t *key_vtn_flowfilter_entry,
    unc_keytype_operation_t operation) {
  UPLL_FUNC_TRACE;

  upll_rc_t rt_code = UPLL_RC_ERR_GENERIC;
  /** Validate vtn_key structure */
  VtnMoMgr *mgrvtn = reinterpret_cast<VtnMoMgr *>(
    const_cast<MoManager*>(GetMoManager(UNC_KT_VTN)));

  if (NULL == mgrvtn) {
    UPLL_LOG_DEBUG("unable to get VtnMoMgr object to validate key_vtn");
    return UPLL_RC_ERR_GENERIC;
  }

  rt_code = mgrvtn->ValidateVtnKey(
      &(key_vtn_flowfilter_entry->flowfilter_key.vtn_key));

  if (UPLL_RC_SUCCESS != rt_code) {
    UPLL_LOG_DEBUG(" Vtn name syntax validation failed :"
                  "Err Code - %d",
                  rt_code);
    return rt_code;
  }

  /* Validate inputdirection is in the range specified in
   enum FlowFilter_Direction */
  if (!ValidateNumericRange(
      key_vtn_flowfilter_entry->flowfilter_key.input_direction,
      (uint8_t) UPLL_FLOWFILTER_DIR_IN, (uint8_t) UPLL_FLOWFILTER_DIR_OUT, true,
      true)) {
    UPLL_LOG_DEBUG(" input direction syntax validation failed ");
    return UPLL_RC_ERR_CFG_SYNTAX;
  }

  if ((operation != UNC_OP_READ_SIBLING_COUNT) &&
      (operation != UNC_OP_READ_SIBLING_BEGIN)) {
    /** validate seq_num */
    if (!ValidateNumericRange(key_vtn_flowfilter_entry->sequence_num,
                            kMinFlowFilterSeqNum, kMaxFlowFilterSeqNum, true,
                            true)) {
      UPLL_LOG_DEBUG(" Sequence Number syntax validation failed ");
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  } else {
    key_vtn_flowfilter_entry->sequence_num = 0;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtnFlowFilterEntryMoMgr::ValidateVtnFlowfilterEntryValue(
    ConfigKeyVal *key, IpcReqRespHeader *req, DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;

  /** Read val struct from ConfigKeyVal */
  val_vtn_flowfilter_entry_t *val_vtn_flowfilter_entry =
      static_cast<val_vtn_flowfilter_entry_t *>(
          key->get_cfg_val()->get_val());

  /** validate Value structure */
  if (val_vtn_flowfilter_entry != NULL) {
    upll_rc_t result_code = UPLL_RC_ERR_GENERIC;

    /**validate flowlist name */
    if (val_vtn_flowfilter_entry->valid[UPLL_IDX_FLOWLIST_NAME_VFFE]
        == UNC_VF_VALID) {
      result_code = ValidateKey(
          reinterpret_cast<char*>(val_vtn_flowfilter_entry->flowlist_name),
          kMinLenFlowListName, kMaxLenFlowListName);

      if (UPLL_RC_SUCCESS != result_code) {
        UPLL_LOG_DEBUG(" Flowlist name syntax validation failed :Err Code - %d",
                       result_code);
        return result_code;
      }
    } else if (((req->operation == UNC_OP_UPDATE) ||
                (req->operation == UNC_OP_CREATE))
               && (val_vtn_flowfilter_entry->valid[UPLL_IDX_FLOWLIST_NAME_VFFE]
                   == UNC_VF_VALID_NO_VALUE)) {
      UPLL_LOG_TRACE("Reset flowlist name");
      memset(val_vtn_flowfilter_entry->flowlist_name, 0,
             sizeof(val_vtn_flowfilter_entry->flowlist_name));
    }

    /** validate action */
    if (val_vtn_flowfilter_entry->valid[UPLL_IDX_ACTION_VFFE] == UNC_VF_VALID) {
      if (val_vtn_flowfilter_entry->action != UPLL_FLOWFILTER_ACT_PASS) {
        UPLL_LOG_DEBUG("Error Action has value(%d) other than PASS",
                       val_vtn_flowfilter_entry->action);
        return UPLL_RC_ERR_CFG_SYNTAX;
      }
    } else if ((val_vtn_flowfilter_entry->valid[UPLL_IDX_ACTION_VFFE]
                == UNC_VF_INVALID) && (req->operation == UNC_OP_UPDATE)) {
      /** Read key struct from ConfigKeyVal argument*/
      key_vtn_flowfilter_entry_t *key_vtn_flowfilter_entry =
          static_cast<key_vtn_flowfilter_entry_t*>(key->get_key());

      /** Check whether Action configured or not from DB, before updating
        DSCP/priority */
      ConfigKeyVal *okey = NULL;

      MoMgrImpl *mgr = reinterpret_cast<MoMgrImpl *>
          (const_cast<MoManager *>(GetMoManager(UNC_KT_VTN_FLOWFILTER_ENTRY)));
      if (mgr == NULL) {
        UPLL_LOG_DEBUG("Invalid momgr object for KT_VTN");
        return UPLL_RC_ERR_GENERIC;
      }
      result_code = mgr->GetChildConfigKey(okey, NULL);

      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("okey memory allocation failed- %d", result_code);
        return result_code;
      }

      key_vtn_flowfilter_entry_t *vtn_ffe_key =
          reinterpret_cast<key_vtn_flowfilter_entry_t *>(okey->get_key());

      /* copy key structure into okey key struct */
      uuu::upll_strncpy(
          vtn_ffe_key->flowfilter_key.vtn_key.vtn_name,
          key_vtn_flowfilter_entry->flowfilter_key.vtn_key.vtn_name,
          (kMaxLenVtnName+1));
      vtn_ffe_key->flowfilter_key.input_direction =
          key_vtn_flowfilter_entry->flowfilter_key.input_direction;
      vtn_ffe_key->sequence_num =
          key_vtn_flowfilter_entry->sequence_num;

      /* Check the action field configured in VTN_FLOWFILTER_ENTRY table*/
      DbSubOp dbop = {kOpReadSingle, kOpMatchNone, kOpInOutNone};
      result_code = mgr->ReadConfigDB(okey, req->datatype, UNC_OP_READ,
                                      dbop, dmi, MAINTBL);

      if (UPLL_RC_SUCCESS != result_code) {
        UPLL_LOG_DEBUG("ReadConfigDB is failed for VTN_FLOWFILTER_ENTRY");
        delete okey;
        okey = NULL;
        return result_code;
      }

      /* check the action value from the DB data */
      val_vtn_flowfilter_entry_t *val_vtn_ffe =
          reinterpret_cast<val_vtn_flowfilter_entry_t *>(
              okey->get_cfg_val()->get_val());

      if ((val_vtn_ffe->valid[UPLL_IDX_ACTION_VFFE] == UNC_VF_VALID) &&
          (val_vtn_ffe->action == UPLL_FLOWFILTER_ACT_PASS)) {
        UPLL_LOG_TRACE("Action is configured in DB with value as PASS");
      }

      delete okey;
      okey = NULL;
    } else if ((val_vtn_flowfilter_entry->valid[UPLL_IDX_ACTION_VFFE]
                == UNC_VF_VALID_NO_VALUE) &&
               ((req->operation == UNC_OP_UPDATE) ||
                (req->operation == UNC_OP_CREATE))) {
      UPLL_LOG_DEBUG("Reset Action ");
      val_vtn_flowfilter_entry->action = 0;
    }

    /** Validate nwm_name */
    if (val_vtn_flowfilter_entry->valid[UPLL_IDX_NWN_NAME_VFFE]
        == UNC_VF_VALID) {
      result_code = ValidateKey(
          reinterpret_cast<char*>(val_vtn_flowfilter_entry->nwm_name),
          kMinLenNwmName, kMaxLenNwmName);

      if (UPLL_RC_SUCCESS != result_code) {
        UPLL_LOG_DEBUG(" Network monitor name syntax validation failed :"
                       "Err Code - %d", result_code);
        return result_code;
      }
    } else if (((req->operation == UNC_OP_UPDATE) ||
                (req->operation == UNC_OP_CREATE))
               && (val_vtn_flowfilter_entry->valid[UPLL_IDX_NWN_NAME_VFFE]
                   == UNC_VF_VALID_NO_VALUE)) {
      UPLL_LOG_TRACE("Reset network monitor name");

      memset(val_vtn_flowfilter_entry->nwm_name, 0,
             sizeof(val_vtn_flowfilter_entry->nwm_name));
    }

    /** validate DSCP */
    if (val_vtn_flowfilter_entry->valid[UPLL_IDX_DSCP_VFFE] == UNC_VF_VALID) {
      if (!ValidateNumericRange(val_vtn_flowfilter_entry->dscp, kMinIPDscp,
                                kMaxIPDscp, true, true)) {
        UPLL_LOG_DEBUG(" DSCP syntax validation failed ");
        return UPLL_RC_ERR_CFG_SYNTAX;
      }
    } else if (((req->operation == UNC_OP_UPDATE) ||
                (req->operation == UNC_OP_CREATE))
               && (val_vtn_flowfilter_entry->valid[UPLL_IDX_DSCP_VFFE]
                   == UNC_VF_VALID_NO_VALUE)) {
      UPLL_LOG_TRACE("Reset DSCP");
      val_vtn_flowfilter_entry->dscp = 0;
    }

    /** validate Priority*/
    if (val_vtn_flowfilter_entry->valid[UPLL_IDX_PRIORITY_VFFE]
        == UNC_VF_VALID) {
      if (!ValidateNumericRange(val_vtn_flowfilter_entry->priority,
                                kMinVlanPriority, kMaxVlanPriority, true,
                                true)) {
        UPLL_LOG_DEBUG(" Priority syntax validation failed ");
        return UPLL_RC_ERR_CFG_SYNTAX;
      }
    } else if (((req->operation == UNC_OP_UPDATE) ||
                (req->operation == UNC_OP_CREATE))
               && (val_vtn_flowfilter_entry->valid[UPLL_IDX_PRIORITY_VFFE]
                   == UNC_VF_VALID_NO_VALUE)) {
      UPLL_LOG_TRACE("Reset Prirority");
      val_vtn_flowfilter_entry->priority = 0;
    }
  }  // end val_vtn_flowfilter_entry != NULL
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtnFlowFilterEntryMoMgr::ValVtnFlowfilterEntryAttributeSupportCheck(
    val_vtn_flowfilter_entry_t *val_vtn_flowfilter_entry,
    const uint8_t* attrs) {
  UPLL_FUNC_TRACE;

  if ((val_vtn_flowfilter_entry->valid[UPLL_IDX_FLOWLIST_NAME_VFFE]
        == UNC_VF_VALID)
      || (val_vtn_flowfilter_entry->valid[UPLL_IDX_FLOWLIST_NAME_VFFE]
        == UNC_VF_VALID_NO_VALUE)) {
    if (attrs[unc::capa::vtn_flowfilter_entry::kCapFlowlistName] == 0) {
      val_vtn_flowfilter_entry->valid[UPLL_IDX_FLOWLIST_NAME_VFFE] =
        UNC_VF_NOT_SUPPORTED;

      UPLL_LOG_DEBUG("FlowlistName attr is not supported by ctrlr");
    }
  }

  if ((val_vtn_flowfilter_entry->valid[UPLL_IDX_ACTION_VFFE] == UNC_VF_VALID)
      || (val_vtn_flowfilter_entry->valid[UPLL_IDX_ACTION_VFFE]
        == UNC_VF_VALID_NO_VALUE)) {
    if (attrs[unc::capa::vtn_flowfilter_entry::kCapAction] == 0) {
      val_vtn_flowfilter_entry->valid[UPLL_IDX_ACTION_VFFE] =
        UNC_VF_NOT_SUPPORTED;

      UPLL_LOG_DEBUG("Action attr is not supported by ctrlr");
    }
  }
  if ((val_vtn_flowfilter_entry->valid[UPLL_IDX_NWN_NAME_VFFE] ==
        UNC_VF_VALID)
      || (val_vtn_flowfilter_entry->valid[UPLL_IDX_NWN_NAME_VFFE]
        == UNC_VF_VALID_NO_VALUE)) {
    if (attrs[unc::capa::vtn_flowfilter_entry::kCapNwnName] == 0) {
      val_vtn_flowfilter_entry->valid[UPLL_IDX_NWN_NAME_VFFE] =
        UNC_VF_NOT_SUPPORTED;

      UPLL_LOG_DEBUG("Nwm name attr is not supported by ctrlr");
    }
  }
  if ((val_vtn_flowfilter_entry->valid[UPLL_IDX_DSCP_VFFE] == UNC_VF_VALID)
      || (val_vtn_flowfilter_entry->valid[UPLL_IDX_DSCP_VFFE]
        == UNC_VF_VALID_NO_VALUE)) {
    if (attrs[unc::capa::vtn_flowfilter_entry::kCapDscp] == 0) {
      val_vtn_flowfilter_entry->valid[UPLL_IDX_DSCP_VFFE] =
        UNC_VF_NOT_SUPPORTED;

      UPLL_LOG_DEBUG("Dscp attr is not supported by ctrlr");
    }
  }

  if ((val_vtn_flowfilter_entry->valid[UPLL_IDX_PRIORITY_VFFE] ==
        UNC_VF_VALID)
      || (val_vtn_flowfilter_entry->valid[UPLL_IDX_PRIORITY_VFFE]
        == UNC_VF_VALID_NO_VALUE)) {
    if (attrs[unc::capa::vtn_flowfilter_entry::kCapPriority] == 0) {
      val_vtn_flowfilter_entry->valid[UPLL_IDX_PRIORITY_VFFE] =
        UNC_VF_NOT_SUPPORTED;

      UPLL_LOG_DEBUG("Priority attr is not supported by ctrlr");
    }
  }
  return UPLL_RC_SUCCESS;
}

bool VtnFlowFilterEntryMoMgr::GetRenameKeyBindInfo(unc_key_type_t key_type,
                                                   BindInfo *&binfo,
                                                   int &nattr,
                                                   MoMgrTables tbl) {
  /* Main Table only update */
  if (MAINTBL == tbl) {
    nattr = sizeof(vtnflowfilterentrymaintbl_bind_info)/
            sizeof(vtnflowfilterentrymaintbl_bind_info[0]);
    binfo = vtnflowfilterentrymaintbl_bind_info;
  } else if (CTRLRTBL == tbl) {
    nattr = sizeof(vtnflowfilterentryctrlrtbl_bind_info)/
            sizeof(vtnflowfilterentryctrlrtbl_bind_info[0]);
    binfo = vtnflowfilterentryctrlrtbl_bind_info;
  } else {
    UPLL_LOG_DEBUG("Invalid Table, table type %d", tbl);
    return PFC_FALSE;
  }

  UPLL_LOG_DEBUG("Successful Completeion");
  return PFC_TRUE;
}

upll_rc_t  VtnFlowFilterEntryMoMgr::CopyToConfigKey(ConfigKeyVal *&okey,
                                                    ConfigKeyVal *ikey) {
  UPLL_FUNC_TRACE;
  if (!ikey || !(ikey->get_key())) {
    UPLL_LOG_DEBUG("Input Key Not Valid");
    return UPLL_RC_ERR_GENERIC;
  }

  upll_rc_t result_code = UPLL_RC_SUCCESS;

  key_rename_vnode_info *key_rename = reinterpret_cast<key_rename_vnode_info*>
      (ikey->get_key());
  key_vtn_flowfilter_entry_t *key_vtn = reinterpret_cast
      <key_vtn_flowfilter_entry_t*>
      (ConfigKeyVal::Malloc(sizeof(key_vtn_flowfilter_entry_t)));

  if (!strlen(reinterpret_cast<char *>(key_rename->old_unc_vtn_name))) {
    UPLL_LOG_DEBUG("String Length not Valid to Perform the Operation");
    free(key_vtn);
    return UPLL_RC_ERR_GENERIC;
  }

  uuu::upll_strncpy(key_vtn->flowfilter_key.vtn_key.vtn_name,
                    key_rename->old_unc_vtn_name, (kMaxLenVtnName + 1));
  key_vtn->flowfilter_key.input_direction = 0xFE;

  okey = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_ENTRY, IpctSt::
                            kIpcStKeyVtnFlowfilterEntry, key_vtn, NULL);
  if (!okey) {
    FREE_IF_NOT_NULL(key_vtn);
    return UPLL_RC_ERR_GENERIC;
  }
  return result_code;
}

upll_rc_t VtnFlowFilterEntryMoMgr::UpdateVnodeVal(ConfigKeyVal *ikey,
    DalDmlIntf *dmi,
    upll_keytype_datatype_t data_type,
    bool &no_rename) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *okey = NULL;
  ConfigKeyVal *kval = NULL;
  ConfigKeyVal *ckey = NULL;
  ConfigKeyVal *ctrlr_val = NULL;
  // key_vtn_flowfilter_entry_t *vtn_ffe_key = NULL;
  controller_domain ctrlr_dom;
  ctrlr_dom.ctrlr = NULL;
  ctrlr_dom.domain = NULL;
  uint8_t rename = 0;
  string vtn_name = "";

  key_rename_vnode_info_t *key_rename =
    reinterpret_cast<key_rename_vnode_info_t *>(ikey->get_key());

  // Copy the old flowlist name in val_vtn_flowfilter_entry
  val_vtn_flowfilter_entry_t *val =
    reinterpret_cast<val_vtn_flowfilter_entry_t *>
    (ConfigKeyVal::Malloc(sizeof(val_vtn_flowfilter_entry_t)));
  if (!strlen(reinterpret_cast<char *>(key_rename->old_flowlist_name))) {
    if (val) free(val);
    return UPLL_RC_ERR_GENERIC;
  }
  UPLL_LOG_TRACE("Rename NoRename Falg = %d", no_rename);

  result_code = GetChildConfigKey(okey, NULL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_TRACE("GetChildConfigKey Failed ");
    FREE_IF_NOT_NULL(val);
    DELETE_IF_NOT_NULL(okey);
    return UPLL_RC_ERR_GENERIC;
  }

  uuu::upll_strncpy(val->flowlist_name,
      key_rename->old_flowlist_name,
      (kMaxLenFlowListName + 1));
  val->valid[UPLL_IDX_FLOWLIST_NAME_VFFE] = UNC_VF_VALID;
  UPLL_LOG_DEBUG("FlowList name and valid (%d) (%s)",
      val->valid[UPLL_IDX_FLOWLIST_NAME_VFFE], val->flowlist_name);

  // Construct ConfigKeyVal
  //  ConfigVal *cval = new ConfigVal(IpctSt::kIpcStKeyVtnFlowfilterEntry, val);
  okey->SetCfgVal(new ConfigVal(IpctSt::kIpcStValVtnFlowfilterEntry, val));

  DbSubOp dbop = { kOpReadMultiple, kOpMatchNone, kOpInOutFlag };
#if 0
  vtn_ffe_key = reinterpret_cast<key_vtn_flowfilter_entry_t*>(okey->get_key());

  vtn_ffe_key->flowfilter_key.input_direction = 0xFE;
#endif
  // Read the record of key structure and old Flowlist name in maintbl
  result_code = ReadConfigDB(okey, data_type, UNC_OP_READ, dbop, dmi,
      MAINTBL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG(" ReadConfigDB failed ");
    DELETE_IF_NOT_NULL(okey);
    return result_code;
  }
  ConfigKeyVal *tmp_okey = okey;
  while (okey != NULL) {
    // Update the new flowlist name in MAINTBL
    result_code = GetChildConfigKey(kval, okey);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("GetChildConfigKey kval NULL");
      DELETE_IF_NOT_NULL(okey);
      return result_code;
    }
    if (!kval) return UPLL_RC_ERR_GENERIC;
    // Copy the new flowlist in val structure
    val_vtn_flowfilter_entry_t *val1 =
      reinterpret_cast<val_vtn_flowfilter_entry_t *>
      (ConfigKeyVal::Malloc
       (sizeof(val_vtn_flowfilter_entry_t)));
    // New name null check
    if (!strlen(reinterpret_cast<char *>(key_rename->new_flowlist_name))) {
      UPLL_LOG_DEBUG("new_flowlist_name NULL");
      FREE_IF_NOT_NULL(val1);
      DELETE_IF_NOT_NULL(okey);
      DELETE_IF_NOT_NULL(kval);
      return UPLL_RC_ERR_GENERIC;
    }
    // Copy the new flowlist name into val_vtn_flowfilter_entry
    uuu::upll_strncpy(val1->flowlist_name,
        key_rename->new_flowlist_name,
        (kMaxLenPolicingProfileName + 1));
    val1->valid[UPLL_IDX_FLOWLIST_NAME_VFFE] = UNC_VF_VALID;
    UPLL_LOG_DEBUG("flowlist name and valid (%d) (%s)",
        val1->valid[UPLL_IDX_FLOWLIST_NAME_VFFE],
        val1->flowlist_name);
    ConfigVal *cval1 = new ConfigVal(IpctSt::kIpcStValVtnFlowfilterEntry, val1);

    kval->SetCfgVal(cval1);

    GET_USER_DATA_FLAGS(okey, rename);

    if (!no_rename)
      rename = rename | FLOWLIST_RENAME;
    else
      rename = rename & NO_FLOWLIST_RENAME;

    SET_USER_DATA_FLAGS(kval, rename);
    // Update the new flowlist name  in MAINTBL
    result_code = UpdateConfigDB(kval, data_type, UNC_OP_UPDATE, dmi,
                                 TC_CONFIG_GLOBAL, vtn_name, MAINTBL);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("Update record Err in vtnff entry tbl CANDIDATE DB(%d)",
          result_code);
      DELETE_IF_NOT_NULL(okey);
      DELETE_IF_NOT_NULL(kval);
      return result_code;
    }

    // Get the momory alloctaed vtn key structure
    VtnMoMgr *vtnmgr =
      static_cast<VtnMoMgr *>((const_cast<MoManager *>
            (GetMoManager(UNC_KT_VTN))));
    result_code = vtnmgr->GetChildConfigKey(ckey, NULL);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("UpdateRefCountInPPCtrlr GetChildConfigKey error (%d)",
          result_code);
      DELETE_IF_NOT_NULL(okey);
      DELETE_IF_NOT_NULL(kval);
      return result_code;
    }
    if (!ckey) return UPLL_RC_ERR_GENERIC;

    key_vtn_t *vtn_okey = reinterpret_cast<key_vtn_t *>(ckey->get_key());
    key_vtn_flowfilter_entry *vtn_ikey =
      reinterpret_cast<key_vtn_flowfilter_entry *>(okey->get_key());
    uuu::upll_strncpy(vtn_okey->vtn_name,
        vtn_ikey->flowfilter_key.vtn_key.vtn_name,
        kMaxLenVtnName+1);

    UPLL_LOG_DEBUG("vtn name ckey (%s) okey (%s)",
        vtn_okey->vtn_name, vtn_ikey->flowfilter_key.vtn_key.vtn_name);
    result_code = vtnmgr->GetControllerDomainSpan(ckey, data_type, dmi);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("GetControllerSpan  no instance/error (%d)", result_code);
      if (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
        DELETE_IF_NOT_NULL(okey);
        DELETE_IF_NOT_NULL(kval);
        DELETE_IF_NOT_NULL(ckey);
        return result_code;
      }
      DELETE_IF_NOT_NULL(kval);
      DELETE_IF_NOT_NULL(ckey);
      continue;
    }

    // Update Rename flag in the controller span
    ConfigKeyVal *tmp_ckey = ckey;
    while (ckey != NULL) {
      // check for vnode_ref_cnt(vnodes count) in vtn_ctrlr_tbl val structure.
      // If the count is '0' then continue for next vtn_ctrlr_tbl.
      val_vtn_ctrlr *ctr_val =
          reinterpret_cast<val_vtn_ctrlr *>(GetVal(ckey));
      if (!ctr_val) {
        UPLL_LOG_ERROR("Vtn controller table val structure is NULL");
        DELETE_IF_NOT_NULL(okey);
        DELETE_IF_NOT_NULL(kval);
        DELETE_IF_NOT_NULL(ckey);
        return UPLL_RC_ERR_GENERIC;
      }
      if (ctr_val->vnode_ref_cnt <= 0) {
        UPLL_LOG_DEBUG("skipping entry");
        ckey = ckey->get_next_cfg_key_val();
        continue;
      }
      GET_USER_DATA_CTRLR_DOMAIN(ckey, ctrlr_dom);

      UPLL_LOG_DEBUG("ctrlr : %s; domain : %s", ctrlr_dom.ctrlr,
          ctrlr_dom.domain);

      result_code = GetChildConfigKey(ctrlr_val, okey);
      if (result_code != UPLL_RC_SUCCESS)
        return result_code;

      val_vtn_flowfilter_entry_ctrlr  *vtnffe_ctrlr_val =
        reinterpret_cast<val_vtn_flowfilter_entry_ctrlr *>
        (ConfigKeyVal::Malloc(sizeof(val_vtn_flowfilter_entry_ctrlr)));
      // Copy the new policer name into val_vtnpolicingmap_ctrl
      vtnffe_ctrlr_val->valid[UPLL_IDX_FLOWLIST_NAME_VFFE] = UNC_VF_VALID;
      ConfigVal *cval2 = new ConfigVal(IpctSt::kIpcInvalidStNum,
          vtnffe_ctrlr_val);
      ctrlr_val->SetCfgVal(cval2);
      SET_USER_DATA_CTRLR_DOMAIN(ctrlr_val, ctrlr_dom);
      SET_USER_DATA_FLAGS(ctrlr_val, rename);
      // Update the new policer name in CTRLTBL
      result_code = UpdateConfigDB(ctrlr_val, data_type, UNC_OP_UPDATE, dmi,
                                   TC_CONFIG_GLOBAL, vtn_name, CTRLRTBL);
      if (UPLL_RC_SUCCESS != result_code) {
        UPLL_LOG_DEBUG("Update record Err in vtnff entry CANDIDATE DB(%d)",
            result_code);
        DELETE_IF_NOT_NULL(okey);
        DELETE_IF_NOT_NULL(kval);
        DELETE_IF_NOT_NULL(ckey);
        DELETE_IF_NOT_NULL(ctrlr_val);
        return result_code;
      }
      DELETE_IF_NOT_NULL(ctrlr_val);
      ckey = ckey->get_next_cfg_key_val();
    }
    DELETE_IF_NOT_NULL(kval);
    DELETE_IF_NOT_NULL(tmp_ckey);
    okey = okey->get_next_cfg_key_val();
  }
  DELETE_IF_NOT_NULL(tmp_okey);
  return result_code;
}

bool  VtnFlowFilterEntryMoMgr::CompareValidValue(void *&val1,
                                                 void *val2, bool audit) {
  UPLL_FUNC_TRACE;
  bool invalid_attr = true;
  val_vtn_flowfilter_entry_t *val_ff_entry1 =
      reinterpret_cast<val_vtn_flowfilter_entry_t *>(val1);
  val_vtn_flowfilter_entry_t *val_ff_entry2 =
      reinterpret_cast<val_vtn_flowfilter_entry_t *>(val2);

  // if (audit) {
    for ( unsigned int loop = 0;
          loop < sizeof(val_ff_entry1->valid)/sizeof(val_ff_entry1->valid[0]);
          ++loop ) {
      if ( UNC_VF_INVALID == val_ff_entry1->valid[loop] &&
                  UNC_VF_VALID == val_ff_entry2->valid[loop])
        val_ff_entry1->valid[loop] = UNC_VF_VALID_NO_VALUE;
      }
  // }

  if (val_ff_entry1->valid[UPLL_IDX_FLOWLIST_NAME_VFFE] == UNC_VF_VALID &&
      val_ff_entry2->valid[UPLL_IDX_FLOWLIST_NAME_VFFE] == UNC_VF_VALID) {
    if (!strcmp(reinterpret_cast<char *>(val_ff_entry1->flowlist_name),
                reinterpret_cast<char *>(val_ff_entry2->flowlist_name)))
      val_ff_entry1->valid[UPLL_IDX_FLOWLIST_NAME_VFFE] = UNC_VF_INVALID;
  }
  if (val_ff_entry1->valid[UPLL_IDX_ACTION_VFFE] == UNC_VF_VALID &&
      val_ff_entry2->valid[UPLL_IDX_ACTION_VFFE] == UNC_VF_VALID) {
    if (val_ff_entry1->action ==
        val_ff_entry2->action)
      val_ff_entry1->valid[UPLL_IDX_ACTION_VFFE] = UNC_VF_INVALID;
  }
  if (val_ff_entry1->valid[UPLL_IDX_NWN_NAME_VFFE] == UNC_VF_VALID &&
      val_ff_entry2->valid[UPLL_IDX_NWN_NAME_VFFE] == UNC_VF_VALID) {
    if (!strcmp(reinterpret_cast<char *>(val_ff_entry1->nwm_name),
                reinterpret_cast<char *>(val_ff_entry2->nwm_name)))
      val_ff_entry1->valid[UPLL_IDX_NWN_NAME_VFFE] = UNC_VF_INVALID;
  }

  if (val_ff_entry1->valid[UPLL_IDX_DSCP_VFFE] == UNC_VF_VALID &&
      val_ff_entry2->valid[UPLL_IDX_DSCP_VFFE] == UNC_VF_VALID) {
    if (val_ff_entry1->dscp ==
        val_ff_entry2->dscp)
      val_ff_entry1->valid[UPLL_IDX_DSCP_VFFE] = UNC_VF_INVALID;
  }
  if (val_ff_entry1->valid[UPLL_IDX_PRIORITY_VFFE] == UNC_VF_VALID &&
      val_ff_entry2->valid[UPLL_IDX_PRIORITY_VFFE] == UNC_VF_VALID) {
    if (val_ff_entry1->priority ==
        val_ff_entry2->priority)
      val_ff_entry1->valid[UPLL_IDX_PRIORITY_VFFE] = UNC_VF_INVALID;
  }
  for (unsigned int loop = 0;
      loop < sizeof(val_ff_entry1->valid) / sizeof(uint8_t); ++loop) {
    if ((UNC_VF_VALID == (uint8_t) val_ff_entry1->valid[loop]) ||
       (UNC_VF_VALID_NO_VALUE == (uint8_t) val_ff_entry1->valid[loop]))
        invalid_attr = false;
  }
  return invalid_attr;
}

bool  VtnFlowFilterEntryMoMgr::CompareValidVal(void *&val1, void *val2,
                                               void *val3, bool audit) {
  UPLL_FUNC_TRACE;
  bool invalid_attr = true;
  val_vtn_flowfilter_entry_t *val_ff_entry1 =
      reinterpret_cast<val_vtn_flowfilter_entry_t *>(val1);
  val_vtn_flowfilter_entry_t *val_ff_entry2 =
      reinterpret_cast<val_vtn_flowfilter_entry_t *>(val2);
  val_vtn_flowfilter_entry_t *val_ff_entry3 =
      reinterpret_cast<val_vtn_flowfilter_entry_t *>(val3);

  // if (audit) {
    for ( unsigned int loop = 0;
          loop < sizeof(val_ff_entry1->valid)/sizeof(val_ff_entry1->valid[0]);
          ++loop ) {
      if ( UNC_VF_INVALID == val_ff_entry1->valid[loop] &&
                  UNC_VF_VALID == val_ff_entry2->valid[loop])
        val_ff_entry1->valid[loop] = UNC_VF_VALID_NO_VALUE;
      }
  // }

  if (val_ff_entry1->valid[UPLL_IDX_FLOWLIST_NAME_VFFE] == UNC_VF_VALID &&
      val_ff_entry2->valid[UPLL_IDX_FLOWLIST_NAME_VFFE] == UNC_VF_VALID) {
    if (!strcmp(reinterpret_cast<char *>(val_ff_entry3->flowlist_name),
                reinterpret_cast<char *>(val_ff_entry2->flowlist_name)))
      val_ff_entry1->valid[UPLL_IDX_FLOWLIST_NAME_VFFE] = UNC_VF_INVALID;
  }
  if (val_ff_entry1->valid[UPLL_IDX_ACTION_VFFE] == UNC_VF_VALID &&
      val_ff_entry2->valid[UPLL_IDX_ACTION_VFFE] == UNC_VF_VALID) {
    if (val_ff_entry3->action ==
        val_ff_entry2->action)
      val_ff_entry1->valid[UPLL_IDX_ACTION_VFFE] = UNC_VF_INVALID;
  }
  if (val_ff_entry1->valid[UPLL_IDX_NWN_NAME_VFFE] == UNC_VF_VALID &&
      val_ff_entry2->valid[UPLL_IDX_NWN_NAME_VFFE] == UNC_VF_VALID) {
    if (!strcmp(reinterpret_cast<char *>(val_ff_entry3->nwm_name),
                reinterpret_cast<char *>(val_ff_entry2->nwm_name)))
      val_ff_entry1->valid[UPLL_IDX_NWN_NAME_VFFE] = UNC_VF_INVALID;
  }

  if (val_ff_entry1->valid[UPLL_IDX_DSCP_VFFE] == UNC_VF_VALID &&
      val_ff_entry2->valid[UPLL_IDX_DSCP_VFFE] == UNC_VF_VALID) {
    if (val_ff_entry3->dscp ==
        val_ff_entry2->dscp)
      val_ff_entry1->valid[UPLL_IDX_DSCP_VFFE] = UNC_VF_INVALID;
  }
  if (val_ff_entry1->valid[UPLL_IDX_PRIORITY_VFFE] == UNC_VF_VALID &&
      val_ff_entry2->valid[UPLL_IDX_PRIORITY_VFFE] == UNC_VF_VALID) {
    if (val_ff_entry3->priority ==
        val_ff_entry2->priority)
      val_ff_entry1->valid[UPLL_IDX_PRIORITY_VFFE] = UNC_VF_INVALID;
  }
  for (unsigned int loop = 0;
      loop < sizeof(val_ff_entry1->valid) / sizeof(uint8_t); ++loop) {
    if ((UNC_VF_VALID == (uint8_t) val_ff_entry1->valid[loop]) ||
       (UNC_VF_VALID_NO_VALUE == (uint8_t) val_ff_entry1->valid[loop]))
        invalid_attr = false;
  }
  return invalid_attr;
}

upll_rc_t VtnFlowFilterEntryMoMgr::ReadSiblingMo(IpcReqRespHeader *req,
                                                 ConfigKeyVal *ikey,
                                                 bool begin,
                                                 DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  controller_domain ctrlr_dom;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  result_code = ValidateMessage(req, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG(" ValidateMessage Fail");
    return result_code;
  }
  memset(&ctrlr_dom, 0, sizeof(controller_domain));

  if (ikey->get_key_type() == UNC_KT_VTN_FLOWFILTER_ENTRY) {
    result_code = ReadInfoFromDB(req, ikey, dmi, &ctrlr_dom);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG(" Read DB request failed result(%d)", result_code);
    }
    return result_code;
  } else if (ikey->get_key_type() == UNC_KT_VTN_FLOWFILTER_CONTROLLER) {
    result_code = ReadSiblingFlowFilterController(req, ikey, dmi);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("ReadSiblingFlowFilterController failed error (%d)",
                     result_code);
    }
  } else {
    UPLL_LOG_DEBUG(" Invalid Keytype recived");
    result_code = UPLL_RC_ERR_BAD_REQUEST;
  }

  return result_code;
}

upll_rc_t VtnFlowFilterEntryMoMgr::ReadSiblingFlowFilterController(
    IpcReqRespHeader *req,
    ConfigKeyVal *ikey,
    DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  DbSubOp dbop1 = { kOpReadExist, kOpMatchNone, kOpInOutNone};
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  key_vtn_flowfilter_controller_t *key_vtn_ff_ctrl =
      reinterpret_cast<key_vtn_flowfilter_controller_t*>(ikey->get_key());
  key_vtn_flowfilter_entry_t *key_ctrl =
      reinterpret_cast <key_vtn_flowfilter_entry_t*>
      (ConfigKeyVal::Malloc(sizeof(key_vtn_flowfilter_entry_t)));

  ConfigKeyVal *dup_ckmain = new ConfigKeyVal(
      UNC_KT_VTN_FLOWFILTER_ENTRY,
      IpctSt::kIpcStKeyVtnFlowfilterEntry,
      key_ctrl, NULL);

  uuu::upll_strncpy(
      key_ctrl->flowfilter_key.vtn_key.vtn_name,
      key_vtn_ff_ctrl->vtn_key.vtn_name,
      (kMaxLenVtnName +1));
  key_ctrl->flowfilter_key.input_direction = 0xFE;
  // vtn is not configured in vtn_ff_entry main tbl
  // only vtn/vbr is configured and commited
  result_code = UpdateConfigDB(dup_ckmain, req->datatype,
                               UNC_OP_READ, dmi, &dbop1, MAINTBL);
  if (result_code != UPLL_RC_ERR_INSTANCE_EXISTS) {
    UPLL_LOG_DEBUG("Requested Vtn is Not Configured in"
           "flowfilterEntryMain Table in Candidate %d", result_code);
    DELETE_IF_NOT_NULL(dup_ckmain);
    return result_code;
  }
  // vtn is not configured in vtn_ff_entry ctrlr tbl
  // vtn/ff/entry is configured but no vbr is configured

  // Setting direction again to 0XFE
  // because it will be set to 0 in updateconfigDB
  key_ctrl->flowfilter_key.input_direction = 0xFE;
  result_code = UpdateConfigDB(dup_ckmain, req->datatype,
                               UNC_OP_READ, dmi, &dbop1, CTRLRTBL);
  if (result_code != UPLL_RC_ERR_INSTANCE_EXISTS) {
    UPLL_LOG_DEBUG("Requested Vtn is Not Configured in"
                   "flowfilterEntryCtrlr Table %d", result_code);
    DELETE_IF_NOT_NULL(dup_ckmain);
    return UPLL_RC_ERR_NOT_ALLOWED_AT_THIS_TIME;
  }
  DELETE_IF_NOT_NULL(dup_ckmain);
  switch (req->datatype) {
    case UPLL_DT_STATE:
      if ((req->option1 == UNC_OPT1_NORMAL) &&
          (req->option2 == UNC_OPT2_NONE)) {
        result_code = ReadSiblingControllerStateNormal(ikey, req, dmi);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("ReadSiblingControllerStateNormal failed  code(%d)",
                         result_code);
          return result_code;
        }
      } else if (req->datatype == UPLL_DT_STATE &&
                 (req->option1 == UNC_OPT1_DETAIL) &&(
                     req->option2 == UNC_OPT2_NONE)) {
        result_code = ReadSiblingControllerStateDetail(ikey, req, dmi);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("ReadSiblingControllerStateDetail failed  code(%d)",
                         result_code);
          return result_code;
        }
      } else {
        result_code = UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT;
      }
      break;
    default:
      result_code = UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT;
  }
  return result_code;
}

upll_rc_t  VtnFlowFilterEntryMoMgr::ReadSiblingControllerStateNormal(
    ConfigKeyVal *ikey,
    IpcReqRespHeader *req,
    DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  uint32_t tmp_sib_count = 0;
  upll_rc_t vtn_ctrlr_span_rt_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *tctrl_key = NULL, *okey = NULL, *tmp_key = NULL;
  ConfigKeyVal* tmp_okey = NULL;
  controller_domain ctrlr_dom, tmp_ctrlr_dom;
  // ReadSibling Operation Get The Multiple Key
  key_vtn_flowfilter_controller_t *key_vtn_ff_ctrl =
      reinterpret_cast<key_vtn_flowfilter_controller_t*>(ikey->get_key());
  // Extracting The VAl of KT_VTN_FF_Ctrl
  val_flowfilter_controller_t* val_ff_ctrl =
      reinterpret_cast<val_flowfilter_controller_t*>(GetVal(ikey));

  memset(&ctrlr_dom, 0, sizeof(controller_domain));
  memset(&tmp_ctrlr_dom, 0, sizeof(controller_domain));
  ctrlr_dom.ctrlr = reinterpret_cast <uint8_t*>
      (ConfigKeyVal::Malloc(kMaxLenCtrlrId + 1));
  ctrlr_dom.domain = reinterpret_cast <uint8_t*>
      (ConfigKeyVal::Malloc(kMaxLenDomainId + 1));

  uuu::upll_strncpy(ctrlr_dom.ctrlr, key_vtn_ff_ctrl->controller_name,
                    (kMaxLenVtnName +1));
  uuu::upll_strncpy(ctrlr_dom.domain, key_vtn_ff_ctrl->domain_id,
                    (kMaxLenVtnName +1));
  // Allocating The Key of KT_VTN_FF_Entry
  if (req->operation == UNC_OP_READ_SIBLING) {
    std::list<controller_domain_t> list_ctrlr_dom;
    vtn_ctrlr_span_rt_code = GetVtnControllerSpan(ikey, req->datatype, dmi,
                                                  list_ctrlr_dom);
    if ((vtn_ctrlr_span_rt_code != UPLL_RC_SUCCESS) &&
        (vtn_ctrlr_span_rt_code != UPLL_RC_ERR_NO_SUCH_INSTANCE)) {
      UPLL_LOG_DEBUG(" GetVtnControllerSpan  error code (%d)",
                     vtn_ctrlr_span_rt_code);
      return vtn_ctrlr_span_rt_code;
    }

    for (std::list<controller_domain_t>::iterator it= list_ctrlr_dom.begin();
         it != list_ctrlr_dom.end(); ++it) {
      int ctrl_len =  strcmp((const char*)(ctrlr_dom.ctrlr),
                             reinterpret_cast<const char *>(it->ctrlr));
      int dom_len =  strcmp((const char*)(ctrlr_dom.domain),
                            reinterpret_cast<const char *>(it->domain));
      if ((ctrl_len < 0) || ((ctrl_len == 0) && (dom_len < 0))) {
        key_vtn_flowfilter_entry_t *key_vtn_ffe_ctrl =
            reinterpret_cast <key_vtn_flowfilter_entry_t*>
            (ConfigKeyVal::Malloc(sizeof(key_vtn_flowfilter_entry_t)));
        // Allocating CKV tctrl_key
        tctrl_key = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_ENTRY,
                                     IpctSt::kIpcStKeyVtnFlowfilterEntry,
                                     key_vtn_ffe_ctrl, NULL);
        // Copying The seqno,i/p dir,Vtn_Name to The Above Key of CKV tctrl_key
        uuu::upll_strncpy(key_vtn_ffe_ctrl->flowfilter_key.vtn_key.vtn_name,
                          key_vtn_ff_ctrl->vtn_key.vtn_name,
                          (kMaxLenVtnName +1));
        if (val_ff_ctrl->valid[UPLL_IDX_SEQ_NUM_FFC] == UNC_VF_VALID) {
          key_vtn_ffe_ctrl->sequence_num = val_ff_ctrl->sequence_num;
        } else {
          key_vtn_ffe_ctrl->sequence_num = 0;
        }
        if (val_ff_ctrl->valid[UPLL_IDX_DIRECTION_FFC] == UNC_VF_VALID) {
          key_vtn_ffe_ctrl->flowfilter_key.input_direction =
              val_ff_ctrl->direction;
        }  else {
          key_vtn_ffe_ctrl->flowfilter_key.input_direction = 0xFE;
        }
        SET_USER_DATA_CTRLR(tctrl_key,
                            reinterpret_cast<const char *>(it->ctrlr));
        SET_USER_DATA_DOMAIN(tctrl_key,
                             reinterpret_cast<const char *>(it->domain));
        DbSubOp dbop = {kOpReadMultiple, kOpMatchCtrlr|kOpMatchDomain,
          kOpInOutNone};
        result_code = ReadConfigDB(tctrl_key, UPLL_DT_STATE, UNC_OP_READ,
                                   dbop, dmi, CTRLRTBL);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("Unable to read vtn configuration from CTRL DB %d",
                         result_code);
          DELETE_IF_NOT_NULL(tctrl_key);
          DELETE_IF_NOT_NULL(okey);
          return result_code;
        }
        tmp_key =  tctrl_key;
        key_vtn_flowfilter_entry_t *key_vtn_ffe =
            reinterpret_cast <key_vtn_flowfilter_entry_t*>
            (tctrl_key->get_key());

        key_vtn_flowfilter_controller_t *tmp_ff_ctrl =
            reinterpret_cast <key_vtn_flowfilter_controller_t*>
            (ConfigKeyVal::Malloc(sizeof(key_vtn_flowfilter_controller_t)));
        uuu::upll_strncpy(tmp_ff_ctrl->vtn_key.vtn_name,
                          key_vtn_ffe->flowfilter_key.vtn_key.vtn_name,
                          (kMaxLenVtnName +1));
        uuu::upll_strncpy(tmp_ff_ctrl->controller_name,
                          reinterpret_cast<const char *>(it->ctrlr),
                          (kMaxLenCtrlrId +1));

        uuu::upll_strncpy(tmp_ff_ctrl->domain_id,
                          reinterpret_cast<const char *>(it->domain),
                          (kMaxLenDomainId +1));
        tmp_okey = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_CONTROLLER,
                                    IpctSt::kIpcStKeyVtnFlowfilterController,
                                    tmp_ff_ctrl, NULL);

        while (tmp_key !=NULL) {
          result_code = ConstructReadSiblingNormalResponse(tmp_key,
                                                           req->datatype,
                                                           dmi, &tmp_okey);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("ConstructReadSiblingNormalResponse failed %d",
                           result_code);
            DELETE_IF_NOT_NULL(tmp_okey);
            DELETE_IF_NOT_NULL(tctrl_key);
            DELETE_IF_NOT_NULL(okey);
            return result_code;
          }

          tmp_key = tmp_key->get_next_cfg_key_val();
        }
        DELETE_IF_NOT_NULL(tctrl_key);
        if (okey == NULL) {
          okey = tmp_okey;
        } else {
          okey->AppendCfgKeyVal(tmp_okey);
        }
        tmp_sib_count++;
        if (tmp_sib_count == req->rep_count)
          break;
      } else {
        result_code = UPLL_RC_ERR_NO_SUCH_INSTANCE;
      }
    }

    if ((okey != NULL) && (result_code == UPLL_RC_SUCCESS)) {
      ikey->ResetWith(okey);
      req->rep_count = tmp_sib_count;
    } else {
      DELETE_IF_NOT_NULL(okey);
    }
  } else if (req->operation == UNC_OP_READ_SIBLING_BEGIN) {
    std::list<controller_domain_t> list_ctrlr_dom;
    vtn_ctrlr_span_rt_code = GetVtnControllerSpan(ikey, req->datatype,
                                                  dmi, list_ctrlr_dom);
    if ((vtn_ctrlr_span_rt_code != UPLL_RC_SUCCESS) &&
        (vtn_ctrlr_span_rt_code != UPLL_RC_ERR_NO_SUCH_INSTANCE)) {
      UPLL_LOG_DEBUG(" GetVtnControllerSpan  error code (%d)",
                     vtn_ctrlr_span_rt_code);
      return result_code;
    }

    if (vtn_ctrlr_span_rt_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG(" GetVtnControllerSpan failed . Resultcode %d ",
                     vtn_ctrlr_span_rt_code);
      return vtn_ctrlr_span_rt_code;
    }
    for (std::list<controller_domain_t>::iterator it= list_ctrlr_dom.begin();
         it != list_ctrlr_dom.end(); ++it) {
      key_vtn_flowfilter_entry_t *key_vtn_ffe_ctrl =
          reinterpret_cast <key_vtn_flowfilter_entry_t*>
          (ConfigKeyVal::Malloc(sizeof(key_vtn_flowfilter_entry_t)));
      // Allocating CKV tctrl_key
      tctrl_key = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_ENTRY,
                                   IpctSt::kIpcStKeyVtnFlowfilterEntry,
                                   key_vtn_ffe_ctrl, NULL);
      // Copying The seqno, i/p dir, Vtn_Name to The Above Key of CKV tctrl_key
      uuu::upll_strncpy(key_vtn_ffe_ctrl->flowfilter_key.vtn_key.vtn_name,
                        key_vtn_ff_ctrl->vtn_key.vtn_name, (kMaxLenVtnName +1));
      if (val_ff_ctrl->valid[UPLL_IDX_SEQ_NUM_FFC] == UNC_VF_VALID) {
        key_vtn_ffe_ctrl->sequence_num = val_ff_ctrl->sequence_num;
      } else {
        key_vtn_ffe_ctrl->sequence_num = 0;
      }
      if (val_ff_ctrl->valid[UPLL_IDX_DIRECTION_FFC] == UNC_VF_VALID) {
        key_vtn_ffe_ctrl->flowfilter_key.input_direction =
            val_ff_ctrl->direction;
      } else {
        key_vtn_ffe_ctrl->flowfilter_key.input_direction = 0xFE;
      }
      SET_USER_DATA_CTRLR(tctrl_key, reinterpret_cast<const char *>(it->ctrlr));
      SET_USER_DATA_DOMAIN(tctrl_key,
                           reinterpret_cast<const char *>(it->domain));
      DbSubOp dbop = {kOpReadMultiple, kOpMatchCtrlr|
        kOpInOutDomain, kOpInOutDomain};
      result_code = ReadConfigDB(tctrl_key, UPLL_DT_STATE, UNC_OP_READ,
                                 dbop, dmi, CTRLRTBL);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Unable to read vtn configuration from CTRL DB %d",
                       result_code);
        DELETE_IF_NOT_NULL(tctrl_key);
        DELETE_IF_NOT_NULL(okey);
        return result_code;
      }
      tmp_key = tctrl_key;
      key_vtn_flowfilter_entry_t *key_vtn_ffe =
          reinterpret_cast <key_vtn_flowfilter_entry_t*>
          (tctrl_key->get_key());
      key_vtn_flowfilter_controller_t *tmp_ff_ctrl =
          reinterpret_cast <key_vtn_flowfilter_controller_t*>
          (ConfigKeyVal::Malloc(sizeof(key_vtn_flowfilter_controller_t)));
      uuu::upll_strncpy(tmp_ff_ctrl->vtn_key.vtn_name,
                        key_vtn_ffe->flowfilter_key.vtn_key.vtn_name,
                        (kMaxLenVtnName +1));
      uuu::upll_strncpy(tmp_ff_ctrl->controller_name,
                        reinterpret_cast<const char *>(it->ctrlr),
                        (kMaxLenCtrlrId +1));
      uuu::upll_strncpy(tmp_ff_ctrl->domain_id,
                        reinterpret_cast<const char *>(it->domain),
                        (kMaxLenDomainId +1));
      tmp_okey = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_CONTROLLER,
                                  IpctSt::kIpcStKeyVtnFlowfilterController,
                                  tmp_ff_ctrl, NULL);

      while (tmp_key !=NULL) {
        result_code = ConstructReadSiblingNormalResponse(tmp_key,
                                                         req->datatype,
                                                         dmi, &tmp_okey);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("ConstructReadSiblingNormalResponse failed %d",
                         result_code);
          DELETE_IF_NOT_NULL(tctrl_key);
          DELETE_IF_NOT_NULL(tmp_okey);
          DELETE_IF_NOT_NULL(okey);
          return result_code;
        }
        tmp_key = tmp_key->get_next_cfg_key_val();
      }
      DELETE_IF_NOT_NULL(tctrl_key);
      if (okey == NULL) {
        okey = tmp_okey;
      } else {
        okey->AppendCfgKeyVal(tmp_okey);
      }
      tmp_sib_count++;
      if (tmp_sib_count == req->rep_count)
        break;
    }
    if ((okey != NULL) && (result_code == UPLL_RC_SUCCESS)) {
      ikey->ResetWith(okey);
      req->rep_count = tmp_sib_count;
    }
  } else {
    result_code = UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT;
  }
  return result_code;
}

upll_rc_t  VtnFlowFilterEntryMoMgr::ReadSiblingControllerStateDetail(
    ConfigKeyVal *ikey,
    IpcReqRespHeader *req,
    DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  DbSubOp dbop1 = { kOpReadExist, kOpMatchNone, kOpInOutNone};
  upll_rc_t vtn_ctrlr_span_rt_code = UPLL_RC_SUCCESS;
  ConfigKeyVal  *okey = NULL, *l_key = NULL , *tmp_okey =NULL;
  controller_domain ctrlr_dom, tmp_ctrlr_dom;

  key_vtn_flowfilter_controller_t *key_vtn_ff_ctrl =
      reinterpret_cast<key_vtn_flowfilter_controller_t*>(ikey->get_key());
  // Extracting The VAl of KT_VTN_FF_Ctrl
  val_flowfilter_controller_t* val_ff_ctrl =
      reinterpret_cast<val_flowfilter_controller_t*>(GetVal(ikey));

  memset(&ctrlr_dom, 0, sizeof(controller_domain));
  memset(&tmp_ctrlr_dom, 0, sizeof(controller_domain));
  ctrlr_dom.ctrlr = reinterpret_cast <uint8_t*>
      (ConfigKeyVal::Malloc(kMaxLenCtrlrId + 1));
  ctrlr_dom.domain = reinterpret_cast <uint8_t*>
      (ConfigKeyVal::Malloc(kMaxLenDomainId + 1));
  tmp_ctrlr_dom.ctrlr = reinterpret_cast <uint8_t*>
      (ConfigKeyVal::Malloc(kMaxLenCtrlrId + 1));
  tmp_ctrlr_dom.domain = reinterpret_cast <uint8_t*>
      (ConfigKeyVal::Malloc(kMaxLenDomainId + 1));

  uuu::upll_strncpy(ctrlr_dom.ctrlr,
                    key_vtn_ff_ctrl->controller_name,
                    (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(ctrlr_dom.domain,
                    key_vtn_ff_ctrl->domain_id,
                    (kMaxLenDomainId + 1));

  ConfigVal *tmp1 = NULL;
  val_flowfilter_controller_t *l_val =
      reinterpret_cast<val_flowfilter_controller_t*>
      (ConfigKeyVal::Malloc(sizeof(val_flowfilter_controller_t)));

  memcpy(l_val, val_ff_ctrl, sizeof(val_flowfilter_controller_t));
  if (val_ff_ctrl->valid[UPLL_IDX_SEQ_NUM_FFC] != UNC_VF_VALID) {
    l_val->sequence_num = 0;
  }

  tmp1 = new ConfigVal(IpctSt::kIpcStValFlowfilterController,
                       l_val);
  key_vtn_flowfilter_controller_t *l_key_vtn_ff_ctrl =
      reinterpret_cast<key_vtn_flowfilter_controller_t*>
      (ConfigKeyVal::Malloc(sizeof(key_vtn_flowfilter_controller_t)));
  l_key = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_CONTROLLER, IpctSt::
                           kIpcStKeyVtnFlowfilterController,
                           l_key_vtn_ff_ctrl, tmp1);
  // verifying given reuest is existing in vtn_ff_entry_tbl or not
  key_vtn_flowfilter_entry_t *key_ctrl =
      reinterpret_cast <key_vtn_flowfilter_entry_t*>
      (ConfigKeyVal::Malloc(sizeof(key_vtn_flowfilter_entry_t)));
  ConfigKeyVal *dup_ckmain = new ConfigKeyVal(
      UNC_KT_VTN_FLOWFILTER_ENTRY,
      IpctSt::kIpcStKeyVtnFlowfilterEntry,
      key_ctrl, NULL);

  uuu::upll_strncpy(
      key_ctrl->flowfilter_key.vtn_key.vtn_name,
      key_vtn_ff_ctrl->vtn_key.vtn_name,
      (kMaxLenVtnName +1));
  if ((val_ff_ctrl->valid[UPLL_IDX_SEQ_NUM_FFC] == UNC_VF_VALID) ||
           (val_ff_ctrl->valid[UPLL_IDX_DIRECTION_FFC] == UNC_VF_VALID)) {
    if (val_ff_ctrl->valid[UPLL_IDX_SEQ_NUM_FFC] == UNC_VF_VALID) {
      key_ctrl->sequence_num = val_ff_ctrl->sequence_num;
    }
    if (val_ff_ctrl->valid[UPLL_IDX_DIRECTION_FFC] == UNC_VF_VALID) {
      key_ctrl->flowfilter_key.input_direction = val_ff_ctrl->direction;
    }
    result_code = UpdateConfigDB(dup_ckmain, req->datatype,
                               UNC_OP_READ, dmi, &dbop1, MAINTBL);
    if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      UPLL_LOG_DEBUG("Requested Configuration is not Configured in"
                   "VtnflowfilterEntry Table %d", result_code);
      DELETE_IF_NOT_NULL(dup_ckmain);
      DELETE_IF_NOT_NULL(l_key);
      ConfigKeyVal::Free(ctrlr_dom.ctrlr);
      ConfigKeyVal::Free(ctrlr_dom.domain);
      ConfigKeyVal::Free(tmp_ctrlr_dom.ctrlr);
      ConfigKeyVal::Free(tmp_ctrlr_dom.domain);
      return result_code;
    }
  }
  DELETE_IF_NOT_NULL(dup_ckmain);
  // Extracting The Val from CKV tctrl_key
  uint32_t tmp_sib_count = 0;
  if (req->operation == UNC_OP_READ_SIBLING) {
// ====Adding VtnControllerDomainSpan
  std::list<controller_domain_t> list_ctrlr_dom;
  vtn_ctrlr_span_rt_code = GetVtnControllerSpan(ikey,
                       req->datatype, dmi, list_ctrlr_dom);
  if ((vtn_ctrlr_span_rt_code != UPLL_RC_SUCCESS) &&
      (vtn_ctrlr_span_rt_code != UPLL_RC_ERR_NO_SUCH_INSTANCE)) {
    UPLL_LOG_DEBUG(" GetVtnControllerSpan  error code (%d)",
                   vtn_ctrlr_span_rt_code);
    DELETE_IF_NOT_NULL(l_key);
    FREE_LIST_CTRLR(list_ctrlr_dom);
    ConfigKeyVal::Free(ctrlr_dom.ctrlr);
    ConfigKeyVal::Free(ctrlr_dom.domain);
    ConfigKeyVal::Free(tmp_ctrlr_dom.ctrlr);
    ConfigKeyVal::Free(tmp_ctrlr_dom.domain);
    return vtn_ctrlr_span_rt_code;
  }
  for (std::list<controller_domain_t>::iterator it= list_ctrlr_dom.begin();
       it != list_ctrlr_dom.end(); ++it) {
      ConfigKeyVal* tmp_okey = NULL;
      int ctrl_len =  strcmp((const char*)(ctrlr_dom.ctrlr),
                             reinterpret_cast<const char *>(it->ctrlr));
      int dom_len =  strcmp((const char*)(ctrlr_dom.domain),
                            reinterpret_cast<const char *>(it->domain));
      if ((ctrl_len < 0) || ((ctrl_len == 0) && (dom_len < 0))) {
        result_code = ValidateCapability(req, ikey,
                         reinterpret_cast<const char *>(it->ctrlr));
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("Key not supported by controller IN ReadSibling");
          FREE_LIST_CTRLR(list_ctrlr_dom);
          DELETE_IF_NOT_NULL(l_key);
          DELETE_IF_NOT_NULL(okey);
          ConfigKeyVal::Free(ctrlr_dom.ctrlr);
          ConfigKeyVal::Free(ctrlr_dom.domain);
          ConfigKeyVal::Free(tmp_ctrlr_dom.ctrlr);
          ConfigKeyVal::Free(tmp_ctrlr_dom.domain);
          return result_code;
        }

       key_vtn_flowfilter_controller_t *tmp_ff_ctrl =
       reinterpret_cast <key_vtn_flowfilter_controller_t*>
       (ConfigKeyVal::Malloc(sizeof(key_vtn_flowfilter_controller_t)));
       uuu::upll_strncpy(tmp_ff_ctrl->vtn_key.vtn_name,
                    key_vtn_ff_ctrl->vtn_key.vtn_name,
                    (kMaxLenVtnName +1));
       uuu::upll_strncpy(tmp_ff_ctrl->controller_name,
                   reinterpret_cast<const char *>(it->ctrlr),
                    (kMaxLenCtrlrId +1));
       uuu::upll_strncpy(tmp_ff_ctrl->domain_id,
                     reinterpret_cast<const char *>(it->domain),
                    (kMaxLenDomainId +1));
       tmp_okey = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_CONTROLLER,
                              IpctSt::kIpcStKeyVtnFlowfilterController,
                              tmp_ff_ctrl, NULL);

       val_flowfilter_controller_t* tmp_val_ff_ctrl =
        reinterpret_cast <val_flowfilter_controller_t*>
        (ConfigKeyVal::Malloc(sizeof(val_flowfilter_controller_t)));
       memcpy(tmp_val_ff_ctrl, val_ff_ctrl,
         sizeof(val_flowfilter_controller_t));
       tmp_okey->AppendCfgVal(IpctSt::kIpcStValFlowfilterController,
                         tmp_val_ff_ctrl);
       // Fill the l_key value structure with the vtn, ctrl and domain
       // names and send the request.
       uuu::upll_strncpy(l_key_vtn_ff_ctrl->vtn_key.vtn_name,
                         key_vtn_ff_ctrl->vtn_key.vtn_name,
                        (kMaxLenVtnName +1));
       uuu::upll_strncpy(tmp_ctrlr_dom.ctrlr,
                    reinterpret_cast<const char *>(it->ctrlr),
                    (kMaxLenCtrlrId +1));
       uuu::upll_strncpy(tmp_ctrlr_dom.domain,
                    reinterpret_cast<const char *>(it->domain),
                    (kMaxLenDomainId +1));
       uuu::upll_strncpy(l_key_vtn_ff_ctrl->controller_name,
                          tmp_ctrlr_dom.ctrlr, (kMaxLenCtrlrId +1));
       uuu::upll_strncpy(l_key_vtn_ff_ctrl->domain_id,
                          tmp_ctrlr_dom.domain, (kMaxLenDomainId +1));

       IpcResponse ipc_resp;
       memset(&ipc_resp, 0, sizeof(IpcResponse));
       IpcRequest ipc_req;
       memset(&ipc_req, 0, sizeof(IpcRequest));
       ipc_req.header.clnt_sess_id = req->clnt_sess_id;
       ipc_req.header.config_id = req->config_id;
       ipc_req.header.operation = UNC_OP_READ;
       ipc_req.header.option1 = req->option1;
       ipc_req.header.datatype = req->datatype;
       // Added "GetRenamedControllerKey"
       UPLL_LOG_DEBUG("Calling GetRenamedControllerKeyin ReadSibling");
       result_code = GetRenamedControllerKey(l_key, req->datatype, dmi,
                                       &tmp_ctrlr_dom);
       if (result_code != UPLL_RC_SUCCESS) {
         UPLL_LOG_DEBUG("GetRenamedControllerKey Failed %d", result_code);
         FREE_LIST_CTRLR(list_ctrlr_dom);
         DELETE_IF_NOT_NULL(l_key);
         DELETE_IF_NOT_NULL(tmp_okey);
         DELETE_IF_NOT_NULL(okey);
         ConfigKeyVal::Free(ctrlr_dom.ctrlr);
         ConfigKeyVal::Free(ctrlr_dom.domain);
         ConfigKeyVal::Free(tmp_ctrlr_dom.ctrlr);
         ConfigKeyVal::Free(tmp_ctrlr_dom.domain);
         return result_code;
       }
       ipc_req.ckv_data = l_key;
       if (!IpcUtil::SendReqToDriver(
                (const char *)tmp_ctrlr_dom.ctrlr,
                reinterpret_cast<char *>(tmp_ctrlr_dom.domain),
                PFCDRIVER_SERVICE_NAME, PFCDRIVER_SVID_LOGICAL,
                &ipc_req, true, &ipc_resp)) {
         UPLL_LOG_INFO("SendReqToDriver failed for Key %d controller %s",
                         l_key->get_key_type(),
                         reinterpret_cast<char *>(tmp_ctrlr_dom.ctrlr));
         DELETE_IF_NOT_NULL(tmp_okey);
         FREE_LIST_CTRLR(list_ctrlr_dom);
         DELETE_IF_NOT_NULL(l_key);
         DELETE_IF_NOT_NULL(okey);
         DELETE_IF_NOT_NULL(ipc_resp.ckv_data);
         ConfigKeyVal::Free(ctrlr_dom.ctrlr);
         ConfigKeyVal::Free(ctrlr_dom.domain);
         ConfigKeyVal::Free(tmp_ctrlr_dom.ctrlr);
         ConfigKeyVal::Free(tmp_ctrlr_dom.domain);
         return ipc_resp.header.result_code;
       }
       if (ipc_resp.header.result_code == UPLL_RC_ERR_CTR_DISCONNECTED) {
         result_code = UPLL_RC_ERR_NO_SUCH_INSTANCE;
         DELETE_IF_NOT_NULL(tmp_okey);
         DELETE_IF_NOT_NULL(ipc_resp.ckv_data);
         // move to next controller
         while (++it != list_ctrlr_dom.end()) {
           if (strcmp(reinterpret_cast<const char *>(tmp_ctrlr_dom.ctrlr),
                       reinterpret_cast<const char *>(it->ctrlr)) == 0) {
             UPLL_LOG_TRACE("Controller %s is disconnected", it->ctrlr);
             continue;
           } else {
             break;
           }
         }
         // continue the outer loop
         it--;
         continue;
       } else if (ipc_resp.header.result_code ==
                  UPLL_RC_ERR_NO_SUCH_INSTANCE) {
         UPLL_LOG_DEBUG("Record not found for domain %s",
                        (const char *)tmp_ctrlr_dom.domain);
         DELETE_IF_NOT_NULL(tmp_okey);
         DELETE_IF_NOT_NULL(ipc_resp.ckv_data);
         // move to next domain or next controller
         continue;
       } else if (ipc_resp.header.result_code != UPLL_RC_SUCCESS) {
         UPLL_LOG_DEBUG("Driver response for Key %d controller %s result %d",
                         l_key->get_key_type(), tmp_ctrlr_dom.ctrlr,
                         ipc_resp.header.result_code);
         DELETE_IF_NOT_NULL(tmp_okey);
         FREE_LIST_CTRLR(list_ctrlr_dom);
         DELETE_IF_NOT_NULL(l_key);
         DELETE_IF_NOT_NULL(okey);
         DELETE_IF_NOT_NULL(ipc_resp.ckv_data);
         ConfigKeyVal::Free(ctrlr_dom.ctrlr);
         ConfigKeyVal::Free(ctrlr_dom.domain);
         ConfigKeyVal::Free(tmp_ctrlr_dom.ctrlr);
         ConfigKeyVal::Free(tmp_ctrlr_dom.domain);
         return ipc_resp.header.result_code;
       }

       tmp_okey->AppendCfgVal((ipc_resp.ckv_data)->GetCfgValAndUnlink());
       DELETE_IF_NOT_NULL(ipc_resp.ckv_data);
       tmp_sib_count++;
       if (okey == NULL) {
         okey = tmp_okey;
       } else {
         okey->AppendCfgKeyVal(tmp_okey);
      }
      if (tmp_sib_count == req->rep_count)
          break;
      } else {
       result_code = UPLL_RC_ERR_NO_SUCH_INSTANCE;
     }
     if (tmp_sib_count == req->rep_count)
         break;
    }
    if ((okey != NULL) && ((result_code == UPLL_RC_SUCCESS)
        || (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE))) {
      result_code = UPLL_RC_SUCCESS;
      ikey->ResetWith(okey);
      DELETE_IF_NOT_NULL(okey);
      req->rep_count = tmp_sib_count;
    }
    FREE_LIST_CTRLR(list_ctrlr_dom);
  } else if (req->operation == UNC_OP_READ_SIBLING_BEGIN) {
    std::list<controller_domain_t> list_ctrlr_dom;
    vtn_ctrlr_span_rt_code = GetVtnControllerSpan(ikey,
                      req->datatype, dmi, list_ctrlr_dom);
    if ((vtn_ctrlr_span_rt_code != UPLL_RC_SUCCESS) &&
      (vtn_ctrlr_span_rt_code != UPLL_RC_ERR_NO_SUCH_INSTANCE)) {
      UPLL_LOG_DEBUG(" GetVtnControllerSpan  error code (%d)",
                   vtn_ctrlr_span_rt_code);
      DELETE_IF_NOT_NULL(l_key);
      FREE_LIST_CTRLR(list_ctrlr_dom);
      ConfigKeyVal::Free(ctrlr_dom.ctrlr);
      ConfigKeyVal::Free(ctrlr_dom.domain);
      ConfigKeyVal::Free(tmp_ctrlr_dom.ctrlr);
      ConfigKeyVal::Free(tmp_ctrlr_dom.domain);
      return vtn_ctrlr_span_rt_code;
    }
    for (std::list<controller_domain_t>::iterator it= list_ctrlr_dom.begin();
         it != list_ctrlr_dom.end(); ++it) {
    result_code = ValidateCapability(req, ikey,
                  reinterpret_cast<const char *>(it->ctrlr));
    if (result_code != UPLL_RC_SUCCESS) {
       UPLL_LOG_DEBUG("Key not supported by controller IN ReadSibling");
       DELETE_IF_NOT_NULL(l_key);
       FREE_LIST_CTRLR(list_ctrlr_dom);
       ConfigKeyVal::Free(ctrlr_dom.ctrlr);
       ConfigKeyVal::Free(ctrlr_dom.domain);
       ConfigKeyVal::Free(tmp_ctrlr_dom.ctrlr);
       ConfigKeyVal::Free(tmp_ctrlr_dom.domain);
       return result_code;
    }
    key_vtn_flowfilter_controller_t *tmp_ff_ctrl =
      reinterpret_cast <key_vtn_flowfilter_controller_t*>
      (ConfigKeyVal::Malloc(sizeof(key_vtn_flowfilter_controller_t)));
    uuu::upll_strncpy(tmp_ff_ctrl->vtn_key.vtn_name,
                     key_vtn_ff_ctrl->vtn_key.vtn_name,
                    (kMaxLenVtnName +1));
    uuu::upll_strncpy(tmp_ff_ctrl->controller_name,
                     reinterpret_cast<const char *>(it->ctrlr),
                    (kMaxLenCtrlrId +1));
    uuu::upll_strncpy(tmp_ff_ctrl->domain_id,
                   reinterpret_cast<const char *>(it->domain),
                    (kMaxLenDomainId +1));
    tmp_okey = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_CONTROLLER,
                              IpctSt::kIpcStKeyVtnFlowfilterController,
                              tmp_ff_ctrl, NULL);

    val_flowfilter_controller_t* tmp_val_ff_ctrl =
           reinterpret_cast <val_flowfilter_controller_t*>
           (ConfigKeyVal::Malloc(sizeof(val_flowfilter_controller_t)));
    memcpy(tmp_val_ff_ctrl, val_ff_ctrl, sizeof(val_flowfilter_controller_t));
    tmp_okey->AppendCfgVal(IpctSt::kIpcStValFlowfilterController,
                         tmp_val_ff_ctrl);
    uuu::upll_strncpy(ctrlr_dom.ctrlr,
                    reinterpret_cast<const char *>(it->ctrlr),
                    (kMaxLenCtrlrId +1));
    uuu::upll_strncpy(ctrlr_dom.domain,
                    reinterpret_cast<const char *>(it->domain),
                    (kMaxLenDomainId +1));

    // Assign/Reset the vtn_name for every iteration with the vtn_name received
    // in request, because it might be renamed in one of iteration
    uuu::upll_strncpy(l_key_vtn_ff_ctrl->vtn_key.vtn_name,
                      key_vtn_ff_ctrl->vtn_key.vtn_name,
                      (kMaxLenVtnName +1));
    uuu::upll_strncpy(l_key_vtn_ff_ctrl->controller_name,
                        ctrlr_dom.ctrlr, (kMaxLenCtrlrId +1));
    uuu::upll_strncpy(l_key_vtn_ff_ctrl->domain_id,
                        ctrlr_dom.domain, (kMaxLenDomainId +1));

    IpcResponse ipc_resp;
    memset(&ipc_resp, 0, sizeof(IpcResponse));
    IpcRequest ipc_req;
    memset(&ipc_req, 0, sizeof(IpcRequest));
    ipc_req.header.clnt_sess_id = req->clnt_sess_id;
    ipc_req.header.config_id = req->config_id;
    ipc_req.header.operation = UNC_OP_READ;
    ipc_req.header.option1 = req->option1;
    ipc_req.header.datatype = req->datatype;
    // Added "GetRenamedControllerKey"
    UPLL_LOG_TRACE("Calling GetRenamedControllerKeyin ReadSibling");
    result_code = GetRenamedControllerKey(l_key, req->datatype, dmi,
                                       &ctrlr_dom);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("GetRenamedControllerKey Failed %d", result_code);
      DELETE_IF_NOT_NULL(l_key);
      DELETE_IF_NOT_NULL(tmp_okey);
      FREE_LIST_CTRLR(list_ctrlr_dom);
      ConfigKeyVal::Free(ctrlr_dom.ctrlr);
      ConfigKeyVal::Free(ctrlr_dom.domain);
      ConfigKeyVal::Free(tmp_ctrlr_dom.ctrlr);
      ConfigKeyVal::Free(tmp_ctrlr_dom.domain);
      return result_code;
    }
    ipc_req.ckv_data = l_key;
    if (!IpcUtil::SendReqToDriver(
              (const char *)ctrlr_dom.ctrlr,
              reinterpret_cast<char *>(ctrlr_dom.domain),
              PFCDRIVER_SERVICE_NAME, PFCDRIVER_SVID_LOGICAL, &ipc_req,
              true, &ipc_resp)) {
       UPLL_LOG_INFO("SendReqToDriver failed for Key %d controller %s",
                       l_key->get_key_type(),
                       reinterpret_cast<char *>(ctrlr_dom.ctrlr));
       DELETE_IF_NOT_NULL(l_key);
       DELETE_IF_NOT_NULL(tmp_okey);
       DELETE_IF_NOT_NULL(ipc_resp.ckv_data);
       FREE_LIST_CTRLR(list_ctrlr_dom);
       ConfigKeyVal::Free(ctrlr_dom.ctrlr);
       ConfigKeyVal::Free(ctrlr_dom.domain);
       ConfigKeyVal::Free(tmp_ctrlr_dom.ctrlr);
       ConfigKeyVal::Free(tmp_ctrlr_dom.domain);
       return ipc_resp.header.result_code;
    }
    if (ipc_resp.header.result_code == UPLL_RC_ERR_CTR_DISCONNECTED) {
      result_code = UPLL_RC_ERR_NO_SUCH_INSTANCE;
      DELETE_IF_NOT_NULL(tmp_okey);
      DELETE_IF_NOT_NULL(ipc_resp.ckv_data);
      // move to next controller
      while (++it != list_ctrlr_dom.end()) {
        if (strcmp(reinterpret_cast<const char *>(ctrlr_dom.ctrlr),
                   reinterpret_cast<const char *>(it->ctrlr)) == 0) {
          UPLL_LOG_TRACE("Controller %s is disconnected", it->ctrlr);
          continue;
        } else {
          break;
        }
      }
      // continue the outer loop
      it--;
      continue;
    } else if (ipc_resp.header.result_code ==
                UPLL_RC_ERR_NO_SUCH_INSTANCE) {
       UPLL_LOG_DEBUG("Record not found for domain %s",
                      (const char *)ctrlr_dom.domain);
       DELETE_IF_NOT_NULL(tmp_okey);
       DELETE_IF_NOT_NULL(ipc_resp.ckv_data);
       // move to next domain or next controller
       continue;
    } else if (ipc_resp.header.result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Driver response for Key %d controller %s result %d",
                       l_key->get_key_type(), ctrlr_dom.ctrlr,
                       ipc_resp.header.result_code);
      DELETE_IF_NOT_NULL(l_key);
      DELETE_IF_NOT_NULL(tmp_okey);
      DELETE_IF_NOT_NULL(ipc_resp.ckv_data);
      FREE_LIST_CTRLR(list_ctrlr_dom);
      ConfigKeyVal::Free(ctrlr_dom.ctrlr);
      ConfigKeyVal::Free(ctrlr_dom.domain);
      ConfigKeyVal::Free(tmp_ctrlr_dom.ctrlr);
      ConfigKeyVal::Free(tmp_ctrlr_dom.domain);
      return ipc_resp.header.result_code;
    }
    tmp_okey->AppendCfgVal((ipc_resp.ckv_data)->GetCfgValAndUnlink());
    DELETE_IF_NOT_NULL(ipc_resp.ckv_data);
    if (okey == NULL) {
       okey = tmp_okey;
    } else {
    okey->AppendCfgKeyVal(tmp_okey);
  }

    tmp_sib_count++;
    if (tmp_sib_count == req->rep_count)
      break;
    }
    if ((okey != NULL) && ((result_code == UPLL_RC_SUCCESS)
        || (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE))) {
      result_code = UPLL_RC_SUCCESS;
      ikey->ResetWith(okey);
      req->rep_count = tmp_sib_count;
      DELETE_IF_NOT_NULL(okey);
    }
    FREE_LIST_CTRLR(list_ctrlr_dom);
  } else {
    result_code = UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT;
  }

  ConfigKeyVal::Free(ctrlr_dom.ctrlr);
  ConfigKeyVal::Free(ctrlr_dom.domain);
  ConfigKeyVal::Free(tmp_ctrlr_dom.ctrlr);
  ConfigKeyVal::Free(tmp_ctrlr_dom.domain);
  DELETE_IF_NOT_NULL(l_key);
  return result_code;
}

upll_rc_t  VtnFlowFilterEntryMoMgr::ReadControllerStateDetail(
    ConfigKeyVal *ikey ,
    ConfigKeyVal *drv_resp_ckv,
    controller_domain *ctrlr_dom,
    ConfigKeyVal **okey) {

  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  key_vtn_flowfilter_controller_t *key_vtn_ffe =
      reinterpret_cast <key_vtn_flowfilter_controller_t*>
      (ikey->get_key());
  val_flowfilter_controller_t *ival = reinterpret_cast
            <val_flowfilter_controller_t *>(GetVal(ikey));

  if (*okey == NULL) {
    key_vtn_flowfilter_controller_t *tmp_ff_ctrl =
        reinterpret_cast <key_vtn_flowfilter_controller_t*>
        (ConfigKeyVal::Malloc(sizeof(key_vtn_flowfilter_controller_t)));
    uuu::upll_strncpy(tmp_ff_ctrl->vtn_key.vtn_name,
                      key_vtn_ffe->vtn_key.vtn_name,
                      (kMaxLenVtnName +1));
    uuu::upll_strncpy(tmp_ff_ctrl->controller_name, (*ctrlr_dom).ctrlr,
                      (kMaxLenCtrlrId +1));
    uuu::upll_strncpy(tmp_ff_ctrl->domain_id, (*ctrlr_dom).domain,
                      (kMaxLenDomainId +1));
    *okey = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_CONTROLLER,
                             IpctSt::kIpcStKeyVtnFlowfilterController,
                             tmp_ff_ctrl, NULL);
    val_flowfilter_controller_t* tmp_val_ff_ctrl =
      reinterpret_cast <val_flowfilter_controller_t*>
      (ConfigKeyVal::Malloc(sizeof(val_flowfilter_controller_t)));

    memcpy(tmp_val_ff_ctrl, ival, sizeof(val_flowfilter_controller_t));
    (*okey)->AppendCfgVal(IpctSt::kIpcStValFlowfilterController,
       tmp_val_ff_ctrl);
  }
  ConfigKeyVal *tmp_okey = *okey;
  tmp_okey->AppendCfgVal(drv_resp_ckv->GetCfgValAndUnlink());
  return result_code;
}

upll_rc_t  VtnFlowFilterEntryMoMgr::ConstructReadSiblingNormalResponse(
    ConfigKeyVal *ikey,
    upll_keytype_datatype_t dt_type,
    DalDmlIntf *dmi,
    ConfigKeyVal **okey) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  controller_domain ctrlr_dom;
  memset(&ctrlr_dom, 0, sizeof(controller_domain));
  GET_USER_DATA_CTRLR_DOMAIN(ikey, ctrlr_dom);
  // Allocating The Key of KT_VTN_FF_Entry
  // Allocating CKV tctrl_key
  // Reading The  Entry_Ctrl_Table
  // Extracting The Val from CKV tctrl_key

    val_flowfilter_controller_t* l_val_ff_ctrl =
        reinterpret_cast<val_flowfilter_controller_t *>
        (ConfigKeyVal::Malloc(sizeof(val_flowfilter_controller_t)));

    val_vtn_flowfilter_entry_ctrlr_t* val_vtn_ffe_ctrlr =
        reinterpret_cast<val_vtn_flowfilter_entry_ctrlr_t*>(GetVal(ikey));

    key_vtn_flowfilter_entry_t *l_key_vtn_ffe =
        reinterpret_cast<key_vtn_flowfilter_entry_t *> (ikey->get_key());
    l_val_ff_ctrl->valid[UPLL_IDX_DIRECTION_FFC] = UNC_VF_VALID;
    l_val_ff_ctrl->direction = l_key_vtn_ffe->flowfilter_key.input_direction;
    l_val_ff_ctrl->valid[UPLL_IDX_SEQ_NUM_FFC] = UNC_VF_VALID;
    l_val_ff_ctrl->sequence_num = l_key_vtn_ffe->sequence_num;

    (*okey)->AppendCfgVal(IpctSt::kIpcStValFlowfilterController,
                           l_val_ff_ctrl);

    val_vtn_flowfilter_entry_t *op_val_vtn_ffe =
        reinterpret_cast<val_vtn_flowfilter_entry_t *>
        (ConfigKeyVal::Malloc(sizeof(val_vtn_flowfilter_entry_t)));

    result_code = GetCtrlFlowFilterEntry(l_key_vtn_ffe,
                                         val_vtn_ffe_ctrlr,
                                         dt_type,
                                         dmi,
                                         op_val_vtn_ffe,
                   reinterpret_cast<const char*>(ctrlr_dom.ctrlr));
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("GetCtrlFlowFilterEntry error code (%d)", result_code);
      free(op_val_vtn_ffe);
      return result_code;
    }
    (*okey)->AppendCfgVal(IpctSt::kIpcStValVtnFlowfilterEntry, op_val_vtn_ffe);

  return UPLL_RC_SUCCESS;
}

// Added
upll_rc_t VtnFlowFilterEntryMoMgr::ValidateMessage(IpcReqRespHeader *req,
                                                   ConfigKeyVal *key) {
  UPLL_FUNC_TRACE;

  upll_rc_t rt_code = UPLL_RC_ERR_GENERIC;

  if ((NULL == req) || (NULL == key)) {
    UPLL_LOG_DEBUG("IpcReqRespHeader/ConfigKeyval is NULL");
    return UPLL_RC_ERR_GENERIC;
  }

  if (UNC_KT_VTN_FLOWFILTER_ENTRY == key->get_key_type()) {
    rt_code  = ValidateMessageForVtnFlowFilterEntry(req, key);
    if (UPLL_RC_SUCCESS != rt_code) {
      UPLL_LOG_DEBUG(" Vtn flowfilter entry validation failed :"
                     "Err Code - %d",
                     rt_code);
      return rt_code;
    }
  } else if (UNC_KT_VTN_FLOWFILTER_CONTROLLER == key->get_key_type()) {
    rt_code  =  ValidateMessageForVtnFlowFilterController(req, key);

    if (UPLL_RC_SUCCESS != rt_code) {
      UPLL_LOG_DEBUG(" Vtn flowfilter controller validation failed :"
                     "Err Code - %d",
                     rt_code);
      return rt_code;
    }
  } else {
    UPLL_LOG_DEBUG(" Invalid keytype(%d)", key->get_key_type());
    return UPLL_RC_ERR_BAD_REQUEST;
  }
  if ((req->datatype == UPLL_DT_IMPORT) && (req->operation == UNC_OP_READ ||
       req->operation == UNC_OP_READ_SIBLING ||
       req->operation == UNC_OP_READ_SIBLING_BEGIN ||
       req->operation == UNC_OP_READ_NEXT ||
       req->operation == UNC_OP_READ_BULK ||
       req->operation == UNC_OP_READ_SIBLING_COUNT)) {
    return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT;
  }
  return rt_code;
}

upll_rc_t VtnFlowFilterEntryMoMgr::ValidateMessageForVtnFlowFilterEntry(
    IpcReqRespHeader *req,
    ConfigKeyVal *key) {
  UPLL_FUNC_TRACE;
  upll_rc_t rt_code = UPLL_RC_ERR_GENERIC;

  if (req->option1 != UNC_OPT1_NORMAL) {
    UPLL_LOG_DEBUG(" Invalid option1(%d)", req->option1);
    return UPLL_RC_ERR_INVALID_OPTION1;
  }

  if (req->option2 != UNC_OPT2_NONE) {
    UPLL_LOG_DEBUG(" Invalid option2(%d)", req->option2);
    return UPLL_RC_ERR_INVALID_OPTION2;
  }

  if (key->get_st_num() != IpctSt::kIpcStKeyVtnFlowfilterEntry) {
    UPLL_LOG_DEBUG("Invalid key structure received. received struct num - %d",
                   key->get_st_num());
    return UPLL_RC_ERR_BAD_REQUEST;
  }

  key_vtn_flowfilter_entry_t *key_vtn_flowfilter_entry =
      static_cast<key_vtn_flowfilter_entry_t*>(key->get_key());

  if (NULL == key_vtn_flowfilter_entry) {
    UPLL_LOG_DEBUG("Key structure is empty!!");
    return UPLL_RC_ERR_BAD_REQUEST;
  }

  rt_code = ValidateVtnFlowfilterEntryKey(key_vtn_flowfilter_entry,
                                          req->operation);
  if (UPLL_RC_SUCCESS != rt_code) {
    UPLL_LOG_DEBUG(" Key syntax validation failed :"
                   "Err Code - %d",
                   rt_code);
    return rt_code;
  }

  if (!key->get_cfg_val()) {
    if ((req->operation == UNC_OP_CREATE) ||
        (req->operation == UNC_OP_UPDATE)) {
      UPLL_LOG_DEBUG(" val structure is mandatory");
      return UPLL_RC_ERR_BAD_REQUEST;
    } else {
      UPLL_LOG_TRACE("val stucture is optional");
      return UPLL_RC_SUCCESS;
    }
  }

  if (key->get_cfg_val()->get_st_num() != IpctSt::kIpcStValVtnFlowfilterEntry) {
    UPLL_LOG_DEBUG("Invalid val structure received. struct num - %d",
                   (key->get_cfg_val())->get_st_num());
    return UPLL_RC_ERR_BAD_REQUEST;
  }

  val_vtn_flowfilter_entry_t *val_vtn_flowfilter_entry =
      reinterpret_cast<val_vtn_flowfilter_entry_t *>(
          key->get_cfg_val()->get_val());

  if (NULL == val_vtn_flowfilter_entry) {
    UPLL_LOG_DEBUG("KT_VTN_FLOWFILTER_ENTRY val structure is null");
    return UPLL_RC_ERR_BAD_REQUEST;
  }
  /** For update operation Value structure validation invoked from updatemo */
  if (req->operation == UNC_OP_UPDATE)
    return UPLL_RC_SUCCESS;

  return ValidateVtnFlowfilterEntryValue(key, req);
}

upll_rc_t VtnFlowFilterEntryMoMgr::ValidateMessageForVtnFlowFilterController
                                                   (IpcReqRespHeader *req,
                                                   ConfigKeyVal *key) {
  UPLL_FUNC_TRACE;
  upll_rc_t rt_code = UPLL_RC_ERR_GENERIC;

  if ((req->option1 != UNC_OPT1_NORMAL) &&
      (req->option1 != UNC_OPT1_DETAIL)) {
    UPLL_LOG_DEBUG(" invalid option1(%d)", req->option1);
    return UPLL_RC_ERR_INVALID_OPTION1;
  }

  if (req->option2 != UNC_OPT2_NONE) {
    UPLL_LOG_DEBUG(" invalid option2(%d)", req->option2);
    return UPLL_RC_ERR_INVALID_OPTION2;
  }
  if ((req->operation == UNC_OP_READ_SIBLING_COUNT) &&
     (req->option1 != UNC_OPT1_NORMAL)) {
    UPLL_LOG_DEBUG(" Invalid request(%d)", req->option1);
    return UPLL_RC_ERR_INVALID_OPTION1;
  }

  if (req->datatype != UPLL_DT_STATE) {
      UPLL_LOG_DEBUG(" Invalid Datatype(%d)", req->datatype);
      return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT;
  }

  if (key->get_st_num() != IpctSt::kIpcStKeyVtnFlowfilterController) {
      UPLL_LOG_DEBUG("Invalid key structure received. received struct num - %d",
                     key->get_st_num());
      return UPLL_RC_ERR_BAD_REQUEST;
  }

  key_vtn_flowfilter_controller_t *key_vtn_flowfilter_controller =
    reinterpret_cast<key_vtn_flowfilter_controller_t *>(key->get_key());
  if (NULL == key_vtn_flowfilter_controller) {
      UPLL_LOG_DEBUG("Key structure is empty!!");
      return UPLL_RC_ERR_BAD_REQUEST;
  }
  VtnMoMgr *mgrvtn = reinterpret_cast<VtnMoMgr *>(
    const_cast<MoManager*>(GetMoManager(UNC_KT_VTN)));
  if (NULL == mgrvtn) {
    UPLL_LOG_DEBUG("Unable to get KT_VTN object for key_vtn syntax validation");
    return UPLL_RC_ERR_GENERIC;
  }

  rt_code = mgrvtn->ValidateVtnKey((&(key_vtn_flowfilter_controller->vtn_key)));
  if (UPLL_RC_SUCCESS != rt_code) {
      UPLL_LOG_DEBUG(" Vtn name syntax validation failed :"
                  "Err Code - %d",
                  rt_code);
      return rt_code;
  }

  if ((req->operation != UNC_OP_READ_SIBLING_COUNT) &&
      (req->operation != UNC_OP_READ_SIBLING_BEGIN)) {
    rt_code = ValidateKey(reinterpret_cast<char*>(
      key_vtn_flowfilter_controller->controller_name),
        kMinLenCtrlrId, kMaxLenCtrlrId);

    if (rt_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Controllername syntax validation failed: Err code-%d",
                    rt_code);
      return rt_code;
    }
    if (!ValidateDefaultStr(key_vtn_flowfilter_controller->domain_id,
        kMinLenDomainId, kMaxLenDomainId)) {
      UPLL_LOG_DEBUG("DomainId syntax validation failed:");
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  } else {
    memset(key_vtn_flowfilter_controller->controller_name, 0, kMaxLenCtrlrId);
    memset(key_vtn_flowfilter_controller->domain_id, 0, kMaxLenDomainId);
  }

  UPLL_LOG_TRACE(" key struct validation is success");

  /** validate value structure */

  if (!key->get_cfg_val()) {
    if (req->operation == UNC_OP_READ_SIBLING_COUNT) {
       UPLL_LOG_TRACE("val stucture is optional");
       return UPLL_RC_SUCCESS;
    } else {
      UPLL_LOG_DEBUG(" val structure is mandatory");
      return UPLL_RC_ERR_BAD_REQUEST;
    }
  }

  if (key->get_cfg_val()->get_st_num() !=
      IpctSt::kIpcStValFlowfilterController) {
    UPLL_LOG_DEBUG("Invalid value structure received. struct num - %d",
                   key->get_st_num());
    return UPLL_RC_ERR_BAD_REQUEST;
  }

  val_flowfilter_controller_t *val_ff_ctrlr =
      static_cast<val_flowfilter_controller_t *>(
         key->get_cfg_val()->get_val());

  if (NULL == val_ff_ctrlr) {
    UPLL_LOG_DEBUG("KT_VTN_FLOWFILTER_CONTROLLER val structure is null");
    return UPLL_RC_ERR_BAD_REQUEST;
  }



  return ValidateVtnFlowfilterControllerValue(
                val_ff_ctrlr, req->operation);
}

upll_rc_t VtnFlowFilterEntryMoMgr::ValidateVtnFlowfilterControllerValue(
    val_flowfilter_controller_t *val_ff_ctrlr,
    uint32_t operation) {
  UPLL_FUNC_TRACE;

  /** Validate value structure*/
  if (val_ff_ctrlr != NULL) {
    /** check  direction is filled */
    if (val_ff_ctrlr->valid[UPLL_IDX_DIRECTION_FFC]
        == UNC_VF_VALID) {
      UPLL_LOG_TRACE("direction field is filled");

      /** validate direction range */
      if (!ValidateNumericRange(val_ff_ctrlr->direction,
                                (uint8_t) UPLL_FLOWFILTER_DIR_IN,
                                (uint8_t) UPLL_FLOWFILTER_DIR_OUT, true,
                                true)) {
        UPLL_LOG_DEBUG(" direction syntax validation failed ");
        return UPLL_RC_ERR_CFG_SYNTAX;
      }
    }

    /** check sequence number is configured */
    if (val_ff_ctrlr->valid[UPLL_IDX_SEQ_NUM_FFC]
        == UNC_VF_VALID) {
      UPLL_LOG_TRACE("seq_num field is filled");

      if (!ValidateNumericRange(val_ff_ctrlr->sequence_num,
                                kMinFlowFilterSeqNum, kMaxFlowFilterSeqNum,
                                true, true)) {
        UPLL_LOG_DEBUG(" Sequence Number syntax validation failed ");
        return UPLL_RC_ERR_CFG_SYNTAX;
      }
    }
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtnFlowFilterEntryMoMgr::ValidateCapability(IpcReqRespHeader *req,
                                   ConfigKeyVal *ikey,
                                   const char* ctrlr_name) {
  UPLL_FUNC_TRACE;


  upll_rc_t rt_code = UPLL_RC_ERR_GENERIC;

  if ((NULL == req) || (NULL == ikey)) {
    UPLL_LOG_DEBUG("IpcReqRespHeader/ConfigKeyval is NULL");
    return rt_code;
  }

  if (UNC_KT_VTN_FLOWFILTER_ENTRY == ikey->get_key_type()) {
    return ValidateCapabilityForVtnFlowFilterEntry(req, ikey, ctrlr_name);
  } else if (UNC_KT_VTN_FLOWFILTER_CONTROLLER == ikey->get_key_type()) {
    return ValidateCapabilityForVtnFlowFilterController(req, ikey, ctrlr_name);
  } else {
    // Invalid key_type trace
    return UPLL_RC_ERR_GENERIC;
  }
}

upll_rc_t VtnFlowFilterEntryMoMgr::
     ValidateCapabilityForVtnFlowFilterEntry(IpcReqRespHeader *req,
                                   ConfigKeyVal *ikey,
                                   const char* ctrlr_name) {
  UPLL_FUNC_TRACE;

  if (!ctrlr_name)
    ctrlr_name = static_cast<char *>(ikey->get_user_data());

  UPLL_LOG_TRACE("operation   : (%d)", req->operation);

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
      if (req->datatype == UPLL_DT_STATE) {
        UPLL_LOG_DEBUG("Calling the GetStateCapability");
        ret_code = GetStateCapability(ctrlr_name, ikey->get_key_type(),
                                      &max_attrs, &attrs);
      } else {
        UPLL_LOG_DEBUG("Calling the GetReadCapability");
        ret_code = GetReadCapability(ctrlr_name, ikey->get_key_type(),
                                      &max_attrs, &attrs);
      }
    }
  }

  if (!ret_code) {
    UPLL_LOG_DEBUG("keytype(%d) is not supported by controller(%s) "
                   "for opearion(%d)",
                   ikey->get_key_type(), ctrlr_name, req->operation);
    return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
  }

  val_vtn_flowfilter_entry_t *val_vtn_flowfilter_entry =
      reinterpret_cast<val_vtn_flowfilter_entry_t *>(GetVal(ikey));

  if (val_vtn_flowfilter_entry) {
     if (max_attrs > 0) {
        return ValVtnFlowfilterEntryAttributeSupportCheck(
           val_vtn_flowfilter_entry, attrs);
     } else {
        UPLL_LOG_DEBUG("Attribute list is empty for operation %d",
                       req->operation);
        return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
     }
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtnFlowFilterEntryMoMgr::ValidateCapabilityForVtnFlowFilterController(
    IpcReqRespHeader *req,
    ConfigKeyVal *ikey,
    const char* ctrlr_name) {
  UPLL_FUNC_TRACE;

  if (!ctrlr_name)
    ctrlr_name = static_cast<char *> (ikey->get_user_data());

  if (NULL == ctrlr_name) {
    UPLL_LOG_DEBUG("ctrlr_name is NULL");
    return UPLL_RC_ERR_GENERIC;
  }

  bool result_code = false;
  const uint8_t *attrs = 0;
  uint32_t max_attrs = 0;

  if (req->datatype == UPLL_DT_STATE) {
    UPLL_LOG_DEBUG("Calling the GetStateCapability");
     result_code = GetStateCapability(ctrlr_name, ikey->get_key_type(),
                                    &max_attrs, &attrs);
  } else {
    UPLL_LOG_DEBUG("Calling the GetReadCapability");
    result_code = GetReadCapability(ctrlr_name, ikey->get_key_type(),
                                    &max_attrs, &attrs);
  }
  if (!result_code) {
    UPLL_LOG_DEBUG("keytype(%d) is not supported by controller(%s)",
                   ikey->get_key_type(), ctrlr_name);
    return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
  }

  val_flowfilter_controller_t *val_ff_ctrlr =
        reinterpret_cast<val_flowfilter_controller_t *>(GetVal(ikey));

  if (val_ff_ctrlr) {
    if (max_attrs > 0) {
      return ValVtnFlowfilterCtrlAttributeSupportCheck(
           val_ff_ctrlr, attrs);
    } else {
        UPLL_LOG_DEBUG("Attribute list is empty for operation %d",
                       req->operation);
        return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
    }
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtnFlowFilterEntryMoMgr::ValVtnFlowfilterCtrlAttributeSupportCheck(
  val_flowfilter_controller_t *val_ff_ctrlr,
  const uint8_t* attrs) {
  UPLL_FUNC_TRACE;

  if ((val_ff_ctrlr->valid[UPLL_IDX_DIRECTION_FFC] ==
        UNC_VF_VALID) ||
      (val_ff_ctrlr->valid[UPLL_IDX_DIRECTION_FFC] ==
          UNC_VF_VALID_NO_VALUE)) {
    if (attrs[unc::capa::vtn_flowfilter_controller::kCapDirection]== 0) {
      val_ff_ctrlr->valid[UPLL_IDX_DIRECTION_FFC] =
        UNC_VF_NOT_SUPPORTED;

      UPLL_LOG_DEBUG("Direction attr is not supported by ctrlr");
    }
  }

  if ((val_ff_ctrlr->valid[UPLL_IDX_SEQ_NUM_FFC] ==
        UNC_VF_VALID) ||
      (val_ff_ctrlr->valid[UPLL_IDX_SEQ_NUM_FFC] ==
        UNC_VF_VALID_NO_VALUE)) {
    if (attrs[unc::capa::vtn_flowfilter_controller::kCapSeqNum]== 0) {
      val_ff_ctrlr->valid[UPLL_IDX_SEQ_NUM_FFC] =
        UNC_VF_NOT_SUPPORTED;

      UPLL_LOG_DEBUG("SeqNum attr is not supported by ctrlr");
    }
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t  VtnFlowFilterEntryMoMgr::ReadControllerStateNormal(
    ConfigKeyVal *ikey,
    upll_keytype_datatype_t dt_type,
    unc_keytype_operation_t op,
    DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  // Extracting The Key of KT_VTN_FF_Ctrl
  key_vtn_flowfilter_controller_t *key_vtn_ff_ctrl =
      reinterpret_cast<key_vtn_flowfilter_controller_t*>(ikey->get_key());
  // Extracting The VAl of KT_VTN_FF_Ctrl
  val_flowfilter_controller_t* val_ff_ctrl =
      reinterpret_cast<val_flowfilter_controller_t*>(GetVal(ikey));

  // Allocating The Key of KT_VTN_FF_Entry
  key_vtn_flowfilter_entry_t *key_vtn_ffe_ctrl =
      reinterpret_cast <key_vtn_flowfilter_entry_t*>
      (ConfigKeyVal::Malloc(sizeof(key_vtn_flowfilter_entry_t)));

  // Copying The seqno, i/p dir, Vtn_Name to The Above Key of CKV tctrl_key
  uuu::upll_strncpy(
      key_vtn_ffe_ctrl->flowfilter_key.vtn_key.vtn_name,
      key_vtn_ff_ctrl->vtn_key.vtn_name,
      (kMaxLenVtnName +1));
  if (val_ff_ctrl->valid[UPLL_IDX_SEQ_NUM_FFC] == UNC_VF_VALID) {
    key_vtn_ffe_ctrl->sequence_num = val_ff_ctrl->sequence_num;
  } else {
    key_vtn_ffe_ctrl->sequence_num = 0;
  }
  if (val_ff_ctrl->valid[UPLL_IDX_DIRECTION_FFC] == UNC_VF_VALID) {
    key_vtn_ffe_ctrl->flowfilter_key.input_direction = val_ff_ctrl->direction;
  } else {
    key_vtn_ffe_ctrl->flowfilter_key.input_direction = 0xFE;
  }

  ikey->DeleteCfgVal();
  val_ff_ctrl = NULL;

  // Allocating CKV tctrl_key
  ConfigKeyVal *tctrl_key = new ConfigKeyVal(
      UNC_KT_VTN_FLOWFILTER_ENTRY,
      IpctSt::kIpcStKeyVtnFlowfilterEntry,
      key_vtn_ffe_ctrl, NULL);

  // Reading The  Entry_Ctrl_Table
  SET_USER_DATA_CTRLR(tctrl_key, key_vtn_ff_ctrl->controller_name);
  SET_USER_DATA_DOMAIN(tctrl_key, key_vtn_ff_ctrl->domain_id);
  DbSubOp dbop1 = { kOpReadMultiple, kOpMatchCtrlr|kOpMatchDomain,
    kOpInOutNone };
  result_code =  ReadConfigDB(tctrl_key, dt_type, UNC_OP_READ,
                              dbop1, dmi, CTRLRTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("ReadConfigDB is Not Succes");
    delete tctrl_key;
    return result_code;
  }

  // Extracting The Val from CKV tctrl_key
  ConfigKeyVal *tmp_key = tctrl_key;
  while (tmp_key != NULL) {
    val_flowfilter_controller_t *l_val_ff_ctrl =
        reinterpret_cast<val_flowfilter_controller_t *>
        (ConfigKeyVal::Malloc(sizeof(val_flowfilter_controller_t)));

    val_vtn_flowfilter_entry_ctrlr_t* val_vtn_ffe_ctrlr =
        reinterpret_cast<val_vtn_flowfilter_entry_ctrlr_t*>(GetVal(tmp_key));

    key_vtn_flowfilter_entry_t *l_key_vtn_ffe =
        reinterpret_cast<key_vtn_flowfilter_entry_t *> (tmp_key->get_key());
    l_val_ff_ctrl->valid[UPLL_IDX_DIRECTION_FFC] = UNC_VF_VALID;
    l_val_ff_ctrl->direction = l_key_vtn_ffe->flowfilter_key.input_direction;
    l_val_ff_ctrl->valid[UPLL_IDX_SEQ_NUM_FFC] = UNC_VF_VALID;
    l_val_ff_ctrl->sequence_num = l_key_vtn_ffe->sequence_num;
    ikey->AppendCfgVal(IpctSt::kIpcStValFlowfilterController, l_val_ff_ctrl);

    val_vtn_flowfilter_entry_t *op_val_vtn_ffe =
        reinterpret_cast<val_vtn_flowfilter_entry_t *>
        (ConfigKeyVal::Malloc(sizeof(val_vtn_flowfilter_entry_t)));

    result_code = GetCtrlFlowFilterEntry(l_key_vtn_ffe,
                                         val_vtn_ffe_ctrlr,
                                         dt_type,
                                         dmi,
                                         op_val_vtn_ffe,
              reinterpret_cast<const char*>(key_vtn_ff_ctrl->controller_name));
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("GetCtrlFlowFilterEntry error code (%d)", result_code);
      return result_code;
    }
    ikey->AppendCfgVal(IpctSt::kIpcStValVtnFlowfilterEntry, op_val_vtn_ffe);
    tmp_key = tmp_key->get_next_cfg_key_val();
  }
  delete tctrl_key;
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtnFlowFilterEntryMoMgr::GetCtrlFlowFilterEntry(
    key_vtn_flowfilter_entry *l_key_vtn_ffe,
    val_vtn_flowfilter_entry_ctrlr_t *val_vtn_ffe_ctrlr,
    upll_keytype_datatype_t dt_type,
    DalDmlIntf *dmi,
    val_vtn_flowfilter_entry_t *&op_val_vtn_ffe,
    const char* ctrlr_name,
    unc_keytype_option1_t opt1) {

  UPLL_FUNC_TRACE;
  const uint8_t *attrs = NULL;
  uint32_t max_attrs = 0;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *tkey = NULL;
  key_vtn_flowfilter_entry_t *key_vtn_ffe =
      reinterpret_cast<key_vtn_flowfilter_entry_t *>
      (ConfigKeyVal::Malloc(sizeof(key_vtn_flowfilter_entry_t)));

  tkey = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_ENTRY,
                          IpctSt::kIpcStKeyVtnFlowfilterEntry,
                          key_vtn_ffe, NULL);
  // Now Matching Seq NO, Dir, VTNNAME To KEY of CKV tkey

  memcpy(key_vtn_ffe, l_key_vtn_ffe, sizeof(key_vtn_flowfilter_entry_t));

  DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutNone };
  result_code =  ReadConfigDB(tkey, dt_type, UNC_OP_READ,
                              dbop, dmi, MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("ReadConfigDB is Not Succes");
    delete tkey;
    return result_code;
  }
  // NOW Extract The Val From THe tkey Which Consist
  // Value+CS+Valid Attr.of VTN_FF_ENTRY_TBL
  val_vtn_flowfilter_entry_t *temp_val_vtn_ffe =
      reinterpret_cast<val_vtn_flowfilter_entry_t*>(GetVal(tkey));
  // Checking The Valid And Cs Attribute
  size_t tmp_size = (sizeof(val_vtn_ffe_ctrlr->valid) /
                     sizeof(val_vtn_ffe_ctrlr->valid[0]));

  memcpy(temp_val_vtn_ffe->valid, val_vtn_ffe_ctrlr->valid, tmp_size);

  tmp_size = (sizeof(val_vtn_ffe_ctrlr->cs_attr) /
              sizeof(val_vtn_ffe_ctrlr->cs_attr[0]));
  memcpy(temp_val_vtn_ffe->cs_attr, val_vtn_ffe_ctrlr->cs_attr, tmp_size);

  temp_val_vtn_ffe->cs_row_status = (uint8_t)val_vtn_ffe_ctrlr->cs_row_status;
  // Adding capaCheck for controller
  bool ret_code = false;

  if ((opt1 == UNC_OPT1_DETAIL) && (dt_type == UPLL_DT_STATE)) {
    ret_code = GetStateCapability(ctrlr_name, UNC_KT_VTN_FLOWFILTER_ENTRY,
                                      &max_attrs, &attrs);
    if (!ret_code) {
      UPLL_LOG_DEBUG(
          "GetStateCapability Is failed in GetCtrlFlowFilterEntry %d",
          ret_code);
      return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
    }

    if (max_attrs > 0) {
      result_code =  ValVtnFlowfilterEntryAttributeSupportCheck(
                                    temp_val_vtn_ffe, attrs);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("ERR result_code %d", result_code);
        return result_code;
      }
    }
  }

  if (temp_val_vtn_ffe->valid[UPLL_IDX_FLOWLIST_NAME_VFFE] != UNC_VF_VALID) {
    uuu::upll_strncpy(temp_val_vtn_ffe->flowlist_name,
                      "\0", (kMaxLenFlowListName + 1));
  }
  if (temp_val_vtn_ffe->valid[UPLL_IDX_ACTION_VFFE] != UNC_VF_VALID) {
    temp_val_vtn_ffe->action = 0;
  }
  if (temp_val_vtn_ffe->valid[UPLL_IDX_NWN_NAME_VFFE] != UNC_VF_VALID) {
    uuu::upll_strncpy(temp_val_vtn_ffe->nwm_name,
                      "\0", kMaxLenNwmName);
  }
  if (temp_val_vtn_ffe->valid[UPLL_IDX_DSCP_VFFE] != UNC_VF_VALID) {
    temp_val_vtn_ffe->dscp = 0;
  }
  if (temp_val_vtn_ffe->valid[UPLL_IDX_PRIORITY_VFFE] != UNC_VF_VALID) {
    temp_val_vtn_ffe->priority = 0;
  }

  memcpy(op_val_vtn_ffe, temp_val_vtn_ffe,
         sizeof(val_vtn_flowfilter_entry_t));
  DELETE_IF_NOT_NULL(tkey);

  return result_code;
}

upll_rc_t VtnFlowFilterEntryMoMgr::GetVtnControllerSpan(
    ConfigKeyVal *ikey,
    upll_keytype_datatype_t dt_type,
    DalDmlIntf *dmi,
    std::list<controller_domain_t> &list_ctrlr_dom) {
  UPLL_FUNC_TRACE;
  ConfigKeyVal *okey = NULL;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  VtnMoMgr *mgr = static_cast<VtnMoMgr*>(const_cast<MoManager*>
                                           (GetMoManager(UNC_KT_VTN)));
  if (mgr == NULL) {
    UPLL_LOG_DEBUG("Invalid momgr object for KT_VTN");
    return UPLL_RC_ERR_GENERIC;
  }
  result_code = mgr->GetChildConfigKey(okey, NULL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Key not supported by controller");
    return result_code;
  }
  key_vtn_t *vtn_key = reinterpret_cast<key_vtn*>(okey->get_key());

  if (ikey->get_key_type() == UNC_KT_VTN_FLOWFILTER_CONTROLLER) {
    key_vtn_flowfilter_controller_t *key_vtn_ff_ctrl =
      reinterpret_cast<key_vtn_flowfilter_controller_t*>(ikey->get_key());
    uuu::upll_strncpy(vtn_key->vtn_name,
                    key_vtn_ff_ctrl->vtn_key.vtn_name,
                    (kMaxLenVtnName+1));
  } else {
    key_vtn_flowfilter_entry_t *ffe_key =
      reinterpret_cast<key_vtn_flowfilter_entry_t*>(ikey->get_key());
    uuu::upll_strncpy(vtn_key->vtn_name,
                    ffe_key->flowfilter_key.vtn_key.vtn_name,
                    (kMaxLenVtnName+1));
  }

  result_code = mgr->GetControllerDomainSpanForPOM(
      okey, dt_type, dmi, list_ctrlr_dom);
  if ((result_code != UPLL_RC_SUCCESS) &&
      (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE)) {
    UPLL_LOG_DEBUG("Error in getting controller span (%d)",
                   result_code);
  }
  UPLL_LOG_DEBUG(" GetVtnControllerSpan Result code - %d", result_code);
  DELETE_IF_NOT_NULL(okey);
  return result_code;
}

upll_rc_t VtnFlowFilterEntryMoMgr::CreateAuditMoImpl(ConfigKeyVal *ikey,
                                   DalDmlIntf *dmi,
                                   const char *ctrlr_id) {
  UPLL_FUNC_TRACE;
  if (ikey == NULL || dmi == NULL) {
    UPLL_LOG_TRACE(
        "Cannot perform create operation due to insufficient parameters");
    return UPLL_RC_ERR_GENERIC;
  }
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  upll_rc_t vtn_ctrlr_span_rt_code = UPLL_RC_SUCCESS;
  string vtn_name = "";
  key_vtn_flowfilter_entry_t *vtn_ffe_key =
      reinterpret_cast<key_vtn_flowfilter_entry_t *>(ikey->get_key());
  if (!vtn_ffe_key) {
    UPLL_LOG_DEBUG("vtn_ffe_key NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  uint8_t old_vtn_name[kMaxLenVtnName + 1], new_vtn_name[kMaxLenVtnName + 1];
  memset(old_vtn_name, 0, sizeof(old_vtn_name));
  memset(new_vtn_name, 0, sizeof(new_vtn_name));
  uuu::upll_strncpy(old_vtn_name, vtn_ffe_key->flowfilter_key.vtn_key.vtn_name,
                    kMaxLenVtnName+1);

  UPLL_LOG_TRACE("%s Vtn_FFE CreateAuditMoImpl ikey",
                    ikey->ToStrAll().c_str());
  uuu::upll_strncpy(new_vtn_name, vtn_ffe_key->flowfilter_key.vtn_key.vtn_name,
                    kMaxLenVtnName+1);
  DbSubOp dbop1 = { kOpReadExist, kOpMatchNone, kOpInOutNone };

  result_code = UpdateConfigDB(ikey, UPLL_DT_AUDIT, UNC_OP_READ, dmi,
      &dbop1, MAINTBL);
  if (result_code != UPLL_RC_ERR_INSTANCE_EXISTS &&
      result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    UPLL_LOG_DEBUG("Update record Err in vtnffentry MainTbl(%d)",
        result_code);
    return result_code;
  }

  if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    result_code = UpdateConfigDB(ikey, UPLL_DT_AUDIT, UNC_OP_CREATE,
        dmi, TC_CONFIG_GLOBAL, vtn_name, MAINTBL);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Update record Err in vtnffentry MainTbl(%d)",
        result_code);
      return result_code;
    }
  }

  std::list<controller_domain_t> list_ctrlr_dom;
  MoMgrImpl *mgr = static_cast<MoMgrImpl*>((const_cast<MoManager*>(GetMoManager(
      UNC_KT_VTN))));
  UPLL_LOG_DEBUG(" old vtn name (%s), new vtn name (%s)", old_vtn_name,
                                             new_vtn_name);
  if (!(strcmp(reinterpret_cast<const char *>(old_vtn_name),
                reinterpret_cast<const char *>(new_vtn_name)))) {
    std::list<controller_domain_t> tmp_list_ctrlr_dom;
    vtn_ctrlr_span_rt_code = GetVtnControllerSpan(ikey, UPLL_DT_AUDIT,
                                                  dmi, tmp_list_ctrlr_dom);
    if ((vtn_ctrlr_span_rt_code != UPLL_RC_SUCCESS) &&
        (vtn_ctrlr_span_rt_code != UPLL_RC_ERR_NO_SUCH_INSTANCE)) {
      UPLL_LOG_DEBUG("GetVtnControllerSpan  error code (%d)",
                     vtn_ctrlr_span_rt_code);
      FREE_LIST_CTRLR(list_ctrlr_dom);
      return vtn_ctrlr_span_rt_code;
    }
    if (vtn_ctrlr_span_rt_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      ConfigKeyVal *unc_key = NULL;
      result_code = mgr->GetChildConfigKey(unc_key, NULL);
      if (result_code != UPLL_RC_SUCCESS) {
  UPLL_LOG_DEBUG("GetChildConfigKey fail");
  return result_code;
      }
      val_rename_vtn *rename_vtn_key = reinterpret_cast<val_rename_vtn*>
    (ConfigKeyVal::Malloc(sizeof(val_rename_vtn)));
      if (!rename_vtn_key) {
        UPLL_LOG_DEBUG("rename_vtn_key NULL");
        DELETE_IF_NOT_NULL(unc_key);
        return UPLL_RC_ERR_GENERIC;
      }
      uuu::upll_strncpy(rename_vtn_key->new_name,
      old_vtn_name,
      (kMaxLenVtnName + 1));
      rename_vtn_key->valid[UPLL_IDX_NEW_NAME_RVTN] = UNC_VF_VALID;
      unc_key->AppendCfgVal(IpctSt::kIpcStValRenameVtn, rename_vtn_key);
      DbSubOp dbop = { kOpReadSingle, kOpMatchCtrlr|kOpMatchDomain,
        kOpInOutNone };
      std::list<controller_domain_t>::iterator it;
      for (it = tmp_list_ctrlr_dom.begin();
     it != tmp_list_ctrlr_dom.end(); it++) {
  SET_USER_DATA_CTRLR_DOMAIN(unc_key, *it);
  result_code = mgr->ReadConfigDB(unc_key, UPLL_DT_RUNNING, UNC_OP_READ, dbop,
              dmi, RENAMETBL);
  if ((UPLL_RC_SUCCESS != result_code) &&
      (UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code)) {
    UPLL_LOG_DEBUG("ReadConfigDB failed %d", result_code);
    DELETE_IF_NOT_NULL(unc_key);
    mgr = NULL;
    return result_code;
  }
  if (UPLL_RC_SUCCESS == result_code) {
    continue;
  }
  list_ctrlr_dom.push_back(*it);
  tmp_list_ctrlr_dom.clear();
  break;
      }
      DELETE_IF_NOT_NULL(unc_key);
    }
  } else {
    controller_domain_t tmp_ctrlr_dom, ctrlr_dom;
    tmp_ctrlr_dom.ctrlr = NULL;
    tmp_ctrlr_dom.domain = NULL;
    ctrlr_dom.ctrlr = NULL;
    ctrlr_dom.domain = NULL;
    ConfigKeyVal *unc_key = NULL;
    DbSubOp dbop = { kOpReadSingle, kOpMatchCtrlr,
      kOpInOutDomain|kOpInOutCtrlr };
    MoMgrImpl *mgr = static_cast<MoMgrImpl*>((const_cast<MoManager*>
                                              (GetMoManager(UNC_KT_VTN))));
    result_code = mgr->GetChildConfigKey(unc_key, NULL);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("GetChildConfigKey fail");
      return result_code;
    }
    val_rename_vtn *rename_vtn_key = reinterpret_cast<val_rename_vtn*>
        (ConfigKeyVal::Malloc(sizeof(val_rename_vtn)));
    if (!rename_vtn_key) {
      UPLL_LOG_DEBUG("rename_vtn_key NULL");
      DELETE_IF_NOT_NULL(unc_key);
      return UPLL_RC_ERR_GENERIC;
    }
    uuu::upll_strncpy(rename_vtn_key->new_name,
                      old_vtn_name,
                      (kMaxLenVtnName + 1));
    rename_vtn_key->valid[UPLL_IDX_NEW_NAME_RVTN] = UNC_VF_VALID;

    SET_USER_DATA_CTRLR(unc_key, ctrlr_id);
    unc_key->AppendCfgVal(IpctSt::kIpcStValRenameVtn, rename_vtn_key);
    result_code = mgr->ReadConfigDB(unc_key, UPLL_DT_RUNNING, UNC_OP_READ, dbop,
                                              dmi, RENAMETBL);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("ReadConfigDB failed %d", result_code);
      DELETE_IF_NOT_NULL(unc_key);
      mgr = NULL;
      return result_code;
    }

    ctrlr_dom.ctrlr = reinterpret_cast<uint8_t *>
        (ConfigKeyVal::Malloc((kMaxLenCtrlrId + 1)));
    ctrlr_dom.domain = reinterpret_cast<uint8_t *>
        (ConfigKeyVal::Malloc((kMaxLenDomainId + 1)));

    GET_USER_DATA_CTRLR_DOMAIN(unc_key, tmp_ctrlr_dom);
    uuu::upll_strncpy(ctrlr_dom.ctrlr, tmp_ctrlr_dom.ctrlr,
                        (kMaxLenCtrlrId + 1));
    uuu::upll_strncpy(ctrlr_dom.domain, tmp_ctrlr_dom.domain,
                        (kMaxLenDomainId + 1));

    UPLL_LOG_TRACE(" ctrlr = %s, dom = %s ", ctrlr_dom.ctrlr,
                                              ctrlr_dom.domain);
    list_ctrlr_dom.push_back(ctrlr_dom);
    vtn_ctrlr_span_rt_code = UPLL_RC_SUCCESS;
    DELETE_IF_NOT_NULL(unc_key);
  }

  result_code = SetValidAudit(ikey);
  if (UPLL_RC_SUCCESS != result_code) {
    FREE_LIST_CTRLR(list_ctrlr_dom);
    return result_code;
  }
  // create a record in CANDIDATE DB for controller Table
  if (vtn_ctrlr_span_rt_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    result_code = UpdateControllerTable(ikey, UNC_OP_CREATE,
                                        UPLL_DT_AUDIT, dmi,
                                        list_ctrlr_dom,
                                        TC_CONFIG_GLOBAL, vtn_name);

    if (result_code != UPLL_RC_SUCCESS) {
      FREE_LIST_CTRLR(list_ctrlr_dom);
      return result_code;
    }
  }
  val_vtn_flowfilter_entry_t *val_vtn_ffe = reinterpret_cast
    <val_vtn_flowfilter_entry_t *>(GetVal(ikey));
  if (UNC_VF_VALID == val_vtn_ffe->valid[UPLL_IDX_FLOWLIST_NAME_VFFE]) {
    result_code = UpdateFlowListInCtrl(ikey, UPLL_DT_AUDIT, UNC_OP_CREATE, dmi,
                                       TC_CONFIG_GLOBAL, vtn_name);
    if (result_code != UPLL_RC_SUCCESS) {
      return result_code;
    }
  }
  FREE_LIST_CTRLR(list_ctrlr_dom);
  return result_code;
}


upll_rc_t VtnFlowFilterEntryMoMgr::UpdateControllerTable(
    ConfigKeyVal *ikey, unc_keytype_operation_t op,
    upll_keytype_datatype_t dt_type,
    DalDmlIntf *dmi,
    std::list<controller_domain_t> list_ctrlr_dom,
    TcConfigMode config_mode,
    string vtn_name) {
  UPLL_FUNC_TRACE;
  ConfigKeyVal *ctrlckv = NULL;
  ConfigVal *ctrlcv = NULL;
  unc_keytype_ctrtype_t ctrlrtype = UNC_CT_UNKNOWN;
  uuc::CtrlrMgr *ctrlr_mgr = uuc::CtrlrMgr::GetInstance();
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  std::list<controller_domain_t>::iterator it= list_ctrlr_dom.begin();
  while (it != list_ctrlr_dom.end()) {
    key_vtn_flowfilter_entry_t *vtn_ffe_key =
        reinterpret_cast<key_vtn_flowfilter_entry_t*>
        (ConfigKeyVal::Malloc(sizeof(key_vtn_flowfilter_entry_t)));
    memcpy(vtn_ffe_key, reinterpret_cast<key_vtn_flowfilter_entry_t*>
           (ikey->get_key()), sizeof(key_vtn_flowfilter_entry_t));
    val_vtn_flowfilter_entry_t *vtn_ffe_val = reinterpret_cast
        <val_vtn_flowfilter_entry_t *>(GetVal(ikey));

    if ((op == UNC_OP_CREATE) || (op == UNC_OP_UPDATE)) {
      IpcReqRespHeader *req_header = reinterpret_cast<IpcReqRespHeader*>
               (ConfigKeyVal::Malloc(sizeof(IpcReqRespHeader)));

      req_header->operation = op;
      req_header->datatype = dt_type;

      // Validate whether the attributes supported by controller or not
      result_code = ValidateCapability(
        req_header, ikey, reinterpret_cast<char*>(it->ctrlr));
      free(req_header);

      if (result_code != UPLL_RC_SUCCESS) {
        // VTN FlowfilterEntry is not supported for other than PFC Controller
        // so skip adding entry for such sontroller in ctrlr table
        free(vtn_ffe_key);
        if ((!ctrlr_mgr->GetCtrlrType(reinterpret_cast<char *>(it->ctrlr),
                       dt_type, &ctrlrtype)) || (ctrlrtype != UNC_CT_PFC)) {
            result_code = UPLL_RC_SUCCESS;
            UPLL_LOG_DEBUG("Controller type is  %d", ctrlrtype);
            ++it;
            continue;
        }
        UPLL_LOG_DEBUG("ValidateCapability Failed: result_code=%d",
              result_code);
        return result_code;
     }
    }
    val_vtn_flowfilter_entry_ctrlr_t *ctrlr_val = reinterpret_cast
        <val_vtn_flowfilter_entry_ctrlr_t *>(ConfigKeyVal::Malloc(sizeof
        (val_vtn_flowfilter_entry_ctrlr_t)));
    for (unsigned int loop = 0;
         loop < (sizeof(ctrlr_val->valid)/sizeof(ctrlr_val->valid[0]));
         loop++) {
      if (UNC_VF_NOT_SUPPORTED == vtn_ffe_val->valid[loop]) {
        ctrlr_val->valid[loop] = UNC_VF_INVALID;
      } else {
        ctrlr_val->valid[loop] = vtn_ffe_val->valid[loop];
      }
    }
    ctrlcv = new ConfigVal(IpctSt::kIpcInvalidStNum, ctrlr_val);
    ctrlckv = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_ENTRY,
                               IpctSt::kIpcStKeyVtnFlowfilterEntry,
                               vtn_ffe_key, ctrlcv);
    if (!ctrlckv) return UPLL_RC_ERR_GENERIC;
    // ctrlckv->AppendCfgVal(IpctSt::kIpcInvalidStNum, ctrlr_val);
    SET_USER_DATA_CTRLR_DOMAIN(ctrlckv, *it);
    uint8_t flag = 0;
    GET_USER_DATA_FLAGS(ikey, flag);
    SET_USER_DATA_FLAGS(ctrlckv, flag);
    if (UNC_OP_CREATE == op) {
      DbSubOp dbop1 = { kOpReadExist, kOpMatchCtrlr |
        kOpMatchDomain, kOpInOutNone };
      result_code = UpdateConfigDB(ctrlckv, dt_type, UNC_OP_READ, dmi, &dbop1,
          CTRLRTBL);
      if (result_code != UPLL_RC_ERR_INSTANCE_EXISTS &&
          result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
        UPLL_LOG_DEBUG("UpdateConfigDB Failed err_code %d", result_code);
        DELETE_IF_NOT_NULL(ctrlckv);
        return result_code;
      }
      if (UPLL_RC_ERR_INSTANCE_EXISTS == result_code) {
        ++it;
        DELETE_IF_NOT_NULL(ctrlckv);
        continue;
      }
    }
    // Create/Update/Delete a record in CANDIDATE DB
    result_code = UpdateConfigDB(ctrlckv, dt_type, op, dmi, config_mode,
                                 vtn_name, CTRLRTBL);
    DELETE_IF_NOT_NULL(ctrlckv);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Err while updating the ctrlr table for CandidateDB(%d)",
                     result_code);
      return result_code;
    }

    ++it;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtnFlowFilterEntryMoMgr::UpdateMainTbl(ConfigKeyVal *vtn_ffe_key,
      unc_keytype_operation_t op, uint32_t driver_result,
      ConfigKeyVal *nreq, DalDmlIntf *dmi) {
  ConfigKeyVal *ck_vtn_ffe = NULL;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  val_vtn_flowfilter_entry_t *vtn_ffe_val = NULL;
  void *ffeval = NULL;
  void *nffeval = NULL;
  string vtn_name = "";

  UPLL_FUNC_TRACE;
  if (op != UNC_OP_DELETE) {
    result_code = DupConfigKeyVal(ck_vtn_ffe, vtn_ffe_key, MAINTBL);
    if (!ck_vtn_ffe || result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Returning error %d", result_code);
      return UPLL_RC_ERR_GENERIC;
    }
    vtn_ffe_val = reinterpret_cast<val_vtn_flowfilter_entry_t *>
        (GetVal(ck_vtn_ffe));
    if (!vtn_ffe_val) {
      UPLL_LOG_DEBUG("invalid val");
      return UPLL_RC_ERR_GENERIC;
    }
  } else {
    result_code = GetChildConfigKey(ck_vtn_ffe, vtn_ffe_key);
    if (!ck_vtn_ffe || result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Returning error %d", result_code);
      return UPLL_RC_ERR_GENERIC;
    }
  }
  switch (op) {
    case UNC_OP_CREATE:
      vtn_ffe_val->cs_row_status = UNC_CS_APPLIED;
      break;
    case UNC_OP_UPDATE:
      ffeval = GetVal(ck_vtn_ffe);
      nffeval = GetVal(nreq);
      if (!nffeval || !ffeval) {
        UPLL_LOG_ERROR("Invalid param");
        return UPLL_RC_ERR_GENERIC;
      }
      CompareValidValue(ffeval, nffeval, false);
      vtn_ffe_val->cs_row_status =
             reinterpret_cast<val_vtn_flowfilter_entry_t *>
             (GetVal(nreq))->cs_row_status;
      break;
    case UNC_OP_DELETE:
      break;
    default:
      UPLL_LOG_DEBUG("Inalid operation");
      return UPLL_RC_ERR_GENERIC;
  }

  DbSubOp dbop = {kOpNotRead, kOpMatchNone, kOpInOutNone};
  dbop.inoutop = kOpInOutCs | kOpInOutFlag;
  result_code = UpdateConfigDB(ck_vtn_ffe, UPLL_DT_STATE, op, dmi,
                               &dbop, TC_CONFIG_GLOBAL, vtn_name, MAINTBL);
  EnqueCfgNotification(op, UPLL_DT_RUNNING, ck_vtn_ffe);
  delete ck_vtn_ffe;
  return result_code;
}

upll_rc_t VtnFlowFilterEntryMoMgr::GetControllerDomainSpan(ConfigKeyVal *ikey,
      upll_keytype_datatype_t dt_type,
      DalDmlIntf *dmi) {
    UPLL_FUNC_TRACE;
    upll_rc_t result_code;
    DbSubOp dbop = {kOpReadExist|kOpReadMultiple, kOpMatchNone,
      kOpInOutCtrlr|kOpInOutDomain};
    result_code = ReadConfigDB(ikey, dt_type, UNC_OP_READ, dbop, dmi, CTRLRTBL);
    return result_code;
  }
#if 0
upll_rc_t VtnFlowFilterEntryMoMgr::DeleteChildMo(IpcReqRespHeader *req,
                        ConfigKeyVal *ikey,
                        DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_ERR_GENERIC;
  ConfigKeyVal *tempckv = NULL;
  result_code = GetChildConfigKey(tempckv, ikey);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetChildConfigKey failed");
    return result_code;
  }
  DbSubOp dbop = {kOpReadMultiple, kOpMatchNone, kOpInOutNone};
  result_code = ReadConfigDB(tempckv, UPLL_DT_CANDIDATE, UNC_OP_READ, dbop,
                             dmi, MAINTBL);
  if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
    UPLL_LOG_DEBUG("No instance in vtn flowfilter entry table");
    DELETE_IF_NOT_NULL(tempckv);
    return UPLL_RC_SUCCESS;
  }
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Unable to read configuration from CandidateDb");
    DELETE_IF_NOT_NULL(tempckv);
    return result_code;
  }
  ConfigKeyVal *iter_ckv = tempckv;
  TcConfigMode config_mode = TC_CONFIG_INVALID;
  std::string vtn_name = "";
  result_code = GetConfigModeInfo(req, config_mode, vtn_name);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetConfigMode failed");
    return result_code;
  }

  while (NULL != iter_ckv) {
    val_vtn_flowfilter_entry_t *vtn_ffe_val = reinterpret_cast
            <val_vtn_flowfilter_entry_t *>(GetVal(iter_ckv));
    if (UNC_VF_VALID == vtn_ffe_val->valid[UPLL_IDX_FLOWLIST_NAME_VFFE]) {
      result_code = UpdateFlowListInCtrl(iter_ckv, req->datatype,
                                         UNC_OP_DELETE, dmi,
                                         config_mode, vtn_name);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Unable to update flowlist in ctrlr table");
        DELETE_IF_NOT_NULL(tempckv);
        return result_code;
      }
    }
    iter_ckv = iter_ckv->get_next_cfg_key_val();
  }
  DELETE_IF_NOT_NULL(tempckv);
  result_code = UpdateConfigDB(ikey, req->datatype, req->operation, dmi,
      config_mode, vtn_name, MAINTBL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("DeleteMo record Err in vtnpolicingmaptbl (%d)",
        result_code);
    return result_code;
  }
  result_code = UpdateConfigDB(ikey, req->datatype, req->operation, dmi,
      config_mode, vtn_name, CTRLRTBL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("DeleteMo record Err in vtnpolicingmaptbl (%d)",
        result_code);
    return result_code;
  }
  return UPLL_RC_SUCCESS;
}
#endif
upll_rc_t VtnFlowFilterEntryMoMgr::IsFlowListConfigured(
    const char* flowlist_name, upll_keytype_datatype_t dt_type,
    DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_ERR_GENERIC;
  ConfigKeyVal *ckv = NULL;
  result_code = GetChildConfigKey(ckv, NULL);
  if (UPLL_RC_SUCCESS != result_code) {
    return result_code;
  }
  if (!ckv) return UPLL_RC_ERR_GENERIC;
  val_vtn_flowfilter_entry_t *ffe_val = reinterpret_cast
    <val_vtn_flowfilter_entry_t *>(ConfigKeyVal::Malloc(sizeof
          (val_vtn_flowfilter_entry_t)));
  uuu::upll_strncpy(ffe_val->flowlist_name, flowlist_name,
      (kMaxLenFlowListName + 1));
  ffe_val->valid[UPLL_IDX_FLOWLIST_NAME_VFFE] = UNC_VF_VALID;
  ckv->AppendCfgVal(IpctSt::kIpcStValVtnFlowfilterEntry, ffe_val);
  DbSubOp dbop = {kOpReadMultiple, kOpMatchNone, kOpInOutNone};
  // Read the Configuration from the MainTable
  result_code = ReadConfigDB(ckv, dt_type,
                                UNC_OP_READ, dbop, dmi, MAINTBL);
  delete ckv;
  return result_code;
}

upll_rc_t VtnFlowFilterEntryMoMgr::GetParentConfigKey(ConfigKeyVal *&okey,
                                                      ConfigKeyVal *ikey) {
  UPLL_FUNC_TRACE;

  if (!ikey) {
    UPLL_LOG_DEBUG(" Input Key is NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  DELETE_IF_NOT_NULL(okey);
  unc_key_type_t ikey_type = ikey->get_key_type();
  if (ikey_type != UNC_KT_VTN_FLOWFILTER_ENTRY) {
    UPLL_LOG_DEBUG(" Invalid key type received. Key type - %d", ikey_type);
    return UPLL_RC_ERR_GENERIC;
  }

  key_vtn_flowfilter_entry_t *pkey =
      reinterpret_cast<key_vtn_flowfilter_entry_t*>(ikey->get_key());
  if (!pkey) {
    UPLL_LOG_DEBUG(" Input vtn flow filter entry key is NULL ");
    return UPLL_RC_ERR_GENERIC;
  }

  key_vtn_flowfilter_t *vtn_ff_key = reinterpret_cast<key_vtn_flowfilter_t*>
      (ConfigKeyVal::Malloc(sizeof(key_vtn_flowfilter_t)));

  uuu::upll_strncpy(vtn_ff_key->vtn_key.vtn_name,
                    reinterpret_cast<key_vtn_flowfilter_entry_t*>
                    (pkey)->flowfilter_key.vtn_key.vtn_name,
                    (kMaxLenVtnName + 1));
  vtn_ff_key->input_direction =reinterpret_cast<key_vtn_flowfilter_entry_t*>
                    (pkey)->flowfilter_key.input_direction;
  okey = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER,
                          IpctSt::kIpcStKeyVtnFlowfilter, vtn_ff_key, NULL);
  SET_USER_DATA(okey, ikey);
  return UPLL_RC_SUCCESS;
}
upll_rc_t  VtnFlowFilterEntryMoMgr::ReadSiblingCount(
    IpcReqRespHeader *req,
    ConfigKeyVal *ikey,
    DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  uint32_t sibling_count;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *tctrl_key = NULL, *tmp_key = NULL;
  controller_domain ctrlr_dom, tmp_ctrlr_dom;
  result_code = ValidateMessage(req, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("ValidateMessage failed result_code %d",
                   result_code);
    return result_code;
  }
  if (ikey->get_key_type() == UNC_KT_VTN_FLOWFILTER_ENTRY) {
     if (req->datatype == UPLL_DT_STATE ||
         req->datatype == UPLL_DT_STARTUP ||
           req->datatype == UPLL_DT_RUNNING ||
             req->datatype == UPLL_DT_CANDIDATE ) {
        result_code = ReadInfoFromDB(req, ikey, dmi, &ctrlr_dom);
        return result_code;
      } else {
        UPLL_LOG_DEBUG("ReadSiblingCount is not Allowed For Such datatype %d",
        req->datatype);
        return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT;
      }

  } else if (ikey->get_key_type() == UNC_KT_VTN_FLOWFILTER_CONTROLLER) {
    if ((req->datatype == UPLL_DT_STATE) && (req->option1 == UNC_OPT1_NORMAL) &&
        (req->option2 == UNC_OPT2_NONE)) {
      // ReadSibling Operation Get The Multiple Key
      key_vtn_flowfilter_controller_t *key_vtn_ff_ctrl =
          reinterpret_cast<key_vtn_flowfilter_controller_t*>(ikey->get_key());
      // Extracting The VAl of KT_VTN_FF_Ctrl
      val_flowfilter_controller_t* val_ff_ctrl =
          reinterpret_cast<val_flowfilter_controller_t*>(GetVal(ikey));

      memset(&ctrlr_dom, 0, sizeof(controller_domain));
      memset(&tmp_ctrlr_dom, 0, sizeof(controller_domain));
      ctrlr_dom.ctrlr = reinterpret_cast <uint8_t*>
          (ConfigKeyVal::Malloc(kMaxLenCtrlrId + 1));
      ctrlr_dom.domain = reinterpret_cast <uint8_t*>
          (ConfigKeyVal::Malloc(kMaxLenDomainId + 1));

      uuu::upll_strncpy(ctrlr_dom.ctrlr, key_vtn_ff_ctrl->controller_name,
                        (kMaxLenVtnName +1));
      uuu::upll_strncpy(ctrlr_dom.domain, key_vtn_ff_ctrl->domain_id,
                        (kMaxLenVtnName +1));
      // Allocating The Key of KT_VTN_FF_Entry
      key_vtn_flowfilter_entry_t *key_vtn_ffe_ctrl =
          reinterpret_cast <key_vtn_flowfilter_entry_t*>
          (ConfigKeyVal::Malloc(sizeof(key_vtn_flowfilter_entry_t)));
      // Allocating CKV tctrl_key
      tctrl_key = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_ENTRY,
                                   IpctSt::kIpcStKeyVtnFlowfilterEntry,
                                   key_vtn_ffe_ctrl, NULL);
      // Copying The seqno, i/p dir, Vtn_Name to The Above Key of CKV tctrl_key
      uuu::upll_strncpy(key_vtn_ffe_ctrl->flowfilter_key.vtn_key.vtn_name,
                        key_vtn_ff_ctrl->vtn_key.vtn_name,
                        (kMaxLenVtnName +1));
      if (NULL == val_ff_ctrl) {
        key_vtn_ffe_ctrl->sequence_num = 0;
        key_vtn_ffe_ctrl->flowfilter_key.input_direction = 0xFE;
      } else {
        if (val_ff_ctrl->valid[UPLL_IDX_SEQ_NUM_FFC] == UNC_VF_VALID) {
          key_vtn_ffe_ctrl->sequence_num = val_ff_ctrl->sequence_num;
        } else {
          key_vtn_ffe_ctrl->sequence_num = 0;
        }
        if (val_ff_ctrl->valid[UPLL_IDX_DIRECTION_FFC] == UNC_VF_VALID) {
          key_vtn_ffe_ctrl->flowfilter_key.input_direction =
              val_ff_ctrl->direction;
        } else {
          key_vtn_ffe_ctrl->flowfilter_key.input_direction = 0xFE;
        }
      }
      // Reading The  Entry_Ctrl_Table
      DbSubOp dbop = {kOpReadMultiple, kOpMatchNone,
        kOpInOutCtrlr|kOpInOutDomain};
      result_code = ReadConfigDB(tctrl_key, UPLL_DT_STATE, UNC_OP_READ,
                                 dbop, dmi, CTRLRTBL);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Unable to read vtn configuration from CTRL DB %d",
                       result_code);
        delete tctrl_key;
        return result_code;
      }

      tmp_key = tctrl_key;
      if (req->operation == UNC_OP_READ_SIBLING_COUNT) {
        sibling_count = 0;
        while (tmp_key !=NULL) {
          sibling_count++;
          tmp_key = tmp_key->get_next_cfg_key_val();
        }
        uint32_t *sib_count =
            reinterpret_cast<uint32_t*>(ConfigKeyVal::Malloc(sizeof(uint32_t)));
        *sib_count = sibling_count;
        ikey->SetCfgVal(new ConfigVal(IpctSt::kIpcStUint32, sib_count));
      }
      DELETE_IF_NOT_NULL(tctrl_key);
    } else {
      return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT;
    }
  }
  return result_code;
}

upll_rc_t VtnFlowFilterEntryMoMgr::DeleteChildrenPOM(
    ConfigKeyVal *ikey, upll_keytype_datatype_t dt_type,
    DalDmlIntf *dmi,
    TcConfigMode config_mode,
    string vtn_name) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_ERR_GENERIC;
  uint8_t *ctrlr_id = NULL;
  int nattr = 0;
  BindInfo *binfo = NULL;
  DalCursor *dal_cursor_handle = NULL;
  string query_string;
  unc_key_type_t deletedkt;

  if (NULL == ikey || NULL == dmi) {
    UPLL_LOG_DEBUG("Delete Operation failed:Bad request");
    return result_code;
  }

  key_vtn_flowfilter_entry_t *pkey =
      reinterpret_cast<key_vtn_flowfilter_entry_t*>(ikey->get_key());
  if (!pkey) {
    UPLL_LOG_DEBUG(" Input vtn flow filter entry key is NULL ");
    return UPLL_RC_ERR_GENERIC;
  }

  if (pkey->flowfilter_key.input_direction == 0xFE) {
    deletedkt = UNC_KT_VTN;
  } else {
    deletedkt = UNC_KT_VTN_FLOWFILTER;
  }

  result_code = GetFLPPCountQuery(ikey, deletedkt, query_string);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetFLPPCountQuery failed");
    return result_code;
  }

  const uudst::kDalTableIndex tbl_index = GetTable(MAINTBL, UPLL_DT_CANDIDATE);
  if (tbl_index >= uudst::kDalNumTables) {
    UPLL_LOG_DEBUG("Invalid Table Index - %d", tbl_index);
    return UPLL_RC_ERR_GENERIC;
  }
  DalBindInfo dal_bind_info(tbl_index);

  if (!GetBindInfo(MAINTBL, UPLL_DT_CANDIDATE, binfo, nattr)) {
    UPLL_LOG_DEBUG("GetBindInfo failed");
    return UPLL_RC_ERR_GENERIC;
  }

  ConfigKeyVal *vtn_ffe_ckv  = NULL;
  result_code = GetChildConfigKey(vtn_ffe_ckv, NULL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetChildConfigKey failed");
    return result_code;
  }
  ConfigVal* vtn_ffe_cv = NULL;
  result_code = AllocVal(vtn_ffe_cv, UPLL_DT_CANDIDATE, MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Error in AllocVal");
    DELETE_IF_NOT_NULL(vtn_ffe_ckv);
    return result_code;
  }
  vtn_ffe_ckv->SetCfgVal(vtn_ffe_cv);

  uint32_t count = 0;
  void *tkey = vtn_ffe_ckv->get_key();
  void *p = reinterpret_cast<void *>(reinterpret_cast<char *>(tkey) +
                                     binfo[0].offset);
  dal_bind_info.BindOutput(binfo[0].index, binfo[0].app_data_type,
                           binfo[0].array_size, p);
  tkey = vtn_ffe_ckv->get_cfg_val()->get_val();
  p = reinterpret_cast<void *>(reinterpret_cast<char *>(tkey) +
                               binfo[3].offset);
  dal_bind_info.BindOutput(binfo[3].index, binfo[3].app_data_type,
                           binfo[3].array_size, p);

  dal_bind_info.BindOutput(uud::schema::DAL_COL_STD_INTEGER,
                           uud::kDalUint32, 1, &count);

  // Get the vtn span
  ConfigKeyVal *okey = NULL;
  VtnMoMgr *vtnmgr =
    static_cast<VtnMoMgr *>((const_cast<MoManager *>
          (GetMoManager(UNC_KT_VTN))));
  result_code = vtnmgr->GetChildConfigKey(okey, NULL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("UpdateRefCountInPPCtrlr GetChildConfigKey error (%d)",
        result_code);
    DELETE_IF_NOT_NULL(vtn_ffe_ckv);
    return result_code;
  }
  if (!okey) {
    DELETE_IF_NOT_NULL(vtn_ffe_ckv);
    return UPLL_RC_ERR_GENERIC;
  }
  // To get the vtn associated controller name
  key_vtn_t *vtn_okey = reinterpret_cast<key_vtn_t *>(okey->get_key());
  key_vtn_t *vtn_ikey = reinterpret_cast<key_vtn_t *>(ikey->get_key());
  uuu::upll_strncpy(vtn_okey->vtn_name, vtn_ikey->vtn_name,
      kMaxLenVtnName+1);
  result_code = vtnmgr->GetControllerDomainSpan(okey, dt_type, dmi);
  if (result_code != UPLL_RC_SUCCESS &&
      result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    UPLL_LOG_INFO("GetControllerSpan, err %d", result_code);
    DELETE_IF_NOT_NULL(vtn_ffe_ckv);
    DELETE_IF_NOT_NULL(okey);
    return result_code;
  }
  if (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    ConfigKeyVal *temp_okey = okey;
    result_code = DalToUpllResCode(dmi->ExecuteAppQueryMultipleRecords(
                   query_string, 0, &dal_bind_info,
                   &dal_cursor_handle));
    while (result_code == UPLL_RC_SUCCESS) {
      result_code = DalToUpllResCode(dmi->GetNextRecord(dal_cursor_handle));
      if (UPLL_RC_SUCCESS == result_code) {
        // Call function to update refcount in scratch table
        key_vtn_flowfilter_entry_t *vtn_ffe_key =
            reinterpret_cast<key_vtn_flowfilter_entry_t *>
            (vtn_ffe_ckv->get_key());
        vtn_name = reinterpret_cast<const char *>
            (vtn_ffe_key->flowfilter_key.vtn_key.vtn_name);
        val_vtn_flowfilter_entry_t *vtn_ffe_val =
            reinterpret_cast<val_vtn_flowfilter_entry_t *>(GetVal(vtn_ffe_ckv));

        FlowListMoMgr *fl_mgr =
            reinterpret_cast<FlowListMoMgr *>(const_cast<MoManager *>
                                              (GetMoManager(UNC_KT_FLOWLIST)));
        if (NULL == fl_mgr) {
          UPLL_LOG_DEBUG("fl_mgr is NULL");
          DELETE_IF_NOT_NULL(vtn_ffe_ckv);
          DELETE_IF_NOT_NULL(okey);
          return UPLL_RC_ERR_GENERIC;
        }
        ConfigKeyVal *fl_ckv  = NULL;
        result_code = fl_mgr->GetChildConfigKey(fl_ckv, NULL);
        if (UPLL_RC_SUCCESS != result_code) {
          DELETE_IF_NOT_NULL(vtn_ffe_ckv);
          DELETE_IF_NOT_NULL(okey);
          UPLL_LOG_DEBUG("GetChildConfigKey failed");
          return result_code;
        }
        key_flowlist_t *fl_key = reinterpret_cast<key_flowlist_t *>
            (fl_ckv->get_key());
        uuu::upll_strncpy(fl_key->flowlist_name,
                          vtn_ffe_val->flowlist_name,
                          (kMaxLenFlowListName+1));
        // Call the update ref count function for each controller
        // in which the VTN is spanned across
        while (temp_okey != NULL) {
          // check for vnode_ref_cnt in vtn_ctrlr_tbl val structure.
          // If the count is '0' then continue for next vtn_ctrlr_tbl.
          val_vtn_ctrlr *ctr_val =
                reinterpret_cast<val_vtn_ctrlr *>(GetVal(temp_okey));
          if (!ctr_val) {
            UPLL_LOG_ERROR("Vtn controller table val structure is NULL");
            DELETE_IF_NOT_NULL(vtn_ffe_ckv);
            DELETE_IF_NOT_NULL(okey);
            DELETE_IF_NOT_NULL(fl_ckv);
            return UPLL_RC_ERR_GENERIC;
          }
          if (ctr_val->vnode_ref_cnt <= 0) {
            UPLL_LOG_DEBUG("skipping entry");
            temp_okey = temp_okey->get_next_cfg_key_val();
            continue;
          }
          // update refcount in scratch table
          unc_keytype_ctrtype_t ctrlrtype = UNC_CT_UNKNOWN;
          uuc::CtrlrMgr *ctrlr_mgr = uuc::CtrlrMgr::GetInstance();
          GET_USER_DATA_CTRLR(temp_okey, ctrlr_id);
          if ((!ctrlr_mgr->GetCtrlrType(reinterpret_cast<char *>(ctrlr_id),
                        dt_type, &ctrlrtype)) || (ctrlrtype != UNC_CT_PFC)) {
             UPLL_LOG_DEBUG("Controller type is  %d", ctrlrtype);
             continue;
          }

          SET_USER_DATA_CTRLR(fl_ckv, ctrlr_id);
          result_code = fl_mgr->UpdateRefCountInScratchTbl(fl_ckv, dmi,
                                                           dt_type,
                                                           UNC_OP_DELETE,
                                                           config_mode,
                                                           vtn_name,
                                                           count);
          if (result_code != UPLL_RC_SUCCESS &&
              result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
            UPLL_LOG_DEBUG("UpdateRefCountInScratchTbl returned %d",
                            result_code);
            DELETE_IF_NOT_NULL(vtn_ffe_ckv);
            DELETE_IF_NOT_NULL(okey);
            DELETE_IF_NOT_NULL(fl_ckv);
            return result_code;
          } else if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
            result_code = fl_mgr->InsertRecInScratchTbl(fl_ckv, dmi, dt_type,
                                                        UNC_OP_DELETE,
                                                        config_mode, vtn_name,
                                                        count);
            if (UPLL_RC_SUCCESS != result_code) {
              UPLL_LOG_DEBUG("InsertRecInScratchTbl failed %d", result_code);
              DELETE_IF_NOT_NULL(vtn_ffe_ckv);
              DELETE_IF_NOT_NULL(okey);
              DELETE_IF_NOT_NULL(fl_ckv);
              return result_code;
            }
          }
          temp_okey = temp_okey->get_next_cfg_key_val();
        }
        temp_okey = okey;
        DELETE_IF_NOT_NULL(fl_ckv);
      } else if (UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
        UPLL_LOG_DEBUG("GetNextRecord failed");
        dmi->CloseCursor(dal_cursor_handle, false);
        return result_code;
      }
    }
    if (result_code != UPLL_RC_SUCCESS &&
        result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      UPLL_LOG_DEBUG("ExecuteAppQueryMultipleRecords failed");
      dmi->CloseCursor(dal_cursor_handle, false);
      return result_code;
    }
  }
  DELETE_IF_NOT_NULL(vtn_ffe_ckv);
  DELETE_IF_NOT_NULL(okey);
  /*
  DbSubOp dbop = { kOpReadMultiple, kOpMatchNone, kOpInOutFlag };
  result_code = ReadConfigDB(temp_okey, UPLL_DT_CANDIDATE,
      UNC_OP_READ, dbop, dmi, MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
      UPLL_LOG_DEBUG("UPLL_RC_ERR_NO_SUCH_INSTANCE");
      DELETE_IF_NOT_NULL(temp_okey);
      return UPLL_RC_SUCCESS;
    }
    UPLL_LOG_DEBUG("Unable to read configuration from CandidateDb");
    DELETE_IF_NOT_NULL(temp_okey);
    return result_code;
  }
  okey = temp_okey;
  while (NULL != okey) {
    val_vtn_flowfilter_entry_t *vtn_ffe_val = reinterpret_cast
      <val_vtn_flowfilter_entry_t *>(GetVal(okey));
    if (UNC_VF_VALID == vtn_ffe_val->valid[UPLL_IDX_FLOWLIST_NAME_VFFE]) {
      result_code = UpdateFlowListInCtrl(okey, dt_type, UNC_OP_DELETE, dmi,
                                         config_mode, vtn_name);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Unable to update flowlist in ctrlr table");
        DELETE_IF_NOT_NULL(temp_okey);
        return result_code;
      }
    }
    okey = okey->get_next_cfg_key_val();
  }
  */
  ConfigKeyVal *temp_ikey = NULL;
  result_code = GetChildConfigKey(temp_ikey, ikey);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetChildConfigKey failed");
    return result_code;
  }
  result_code = UpdateConfigDB(temp_ikey, dt_type, UNC_OP_DELETE, dmi,
      config_mode, vtn_name, MAINTBL);
  if ((UPLL_RC_SUCCESS != result_code) &&
      (UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code)) {
    UPLL_LOG_DEBUG("DeleteMo record Err in vtnpolicingmaptbl (%d)",
        result_code);
    DELETE_IF_NOT_NULL(temp_ikey);
    dmi->CloseCursor(dal_cursor_handle, false);
    return result_code;
  }
  DELETE_IF_NOT_NULL(temp_ikey);
  ConfigKeyVal *ctrlr_ikey = NULL;
  result_code = GetChildConfigKey(ctrlr_ikey, ikey);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetChildConfigKey failed");
    dmi->CloseCursor(dal_cursor_handle, false);
    return result_code;
  }
  DbSubOp dbop1 = {kOpNotRead, kOpMatchNone, kOpInOutNone};
  result_code = UpdateConfigDB(ctrlr_ikey, dt_type, UNC_OP_DELETE, dmi,
      &dbop1, config_mode, vtn_name, CTRLRTBL);
  UPLL_LOG_DEBUG("UpdateConfigDB for entry ctrlr tbl %d", result_code);
  if ((UPLL_RC_SUCCESS != result_code) &&
      (UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code)) {
    UPLL_LOG_DEBUG("DeleteMo record Err in vtnpolicingmaptbl (%d)",
        result_code);
    DELETE_IF_NOT_NULL(ctrlr_ikey);
    dmi->CloseCursor(dal_cursor_handle, false);
    return result_code;
  }
  dmi->CloseCursor(dal_cursor_handle, false);
  DELETE_IF_NOT_NULL(ctrlr_ikey);
  return  UPLL_RC_SUCCESS;
}

upll_rc_t VtnFlowFilterEntryMoMgr::SetValidAudit(ConfigKeyVal *&ikey) {
  UPLL_FUNC_TRACE;
  val_vtn_flowfilter_entry_t *val = reinterpret_cast
      <val_vtn_flowfilter_entry_t *>(GetVal(ikey));
  if (NULL == val) {
    UPLL_LOG_DEBUG("val is NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  for (unsigned int loop = 0;
        loop < sizeof(val->valid) / sizeof(val->valid[0]);
        ++loop) {
    val->cs_attr[loop] = UNC_CS_APPLIED;
    val->cs_attr[loop] = UNC_CS_NOT_APPLIED;
  }
  val->cs_row_status = UNC_CS_APPLIED;
  return UPLL_RC_SUCCESS;
}

bool VtnFlowFilterEntryMoMgr::FilterAttributes(void *&val1,
                                               void *val2,
                                               bool copy_to_running,
                                               unc_keytype_operation_t op) {
  UPLL_FUNC_TRACE;
  if (op != UNC_OP_CREATE) {
    return CompareValidValue(val1, val2, copy_to_running);
  }
  return false;
}

upll_rc_t VtnFlowFilterEntryMoMgr::UpdateConfigStatus(ConfigKeyVal *main_ckv,
                                       unc_keytype_operation_t op,
                                       uint32_t driver_result,
                                       ConfigKeyVal *upd_key,
                                       DalDmlIntf *dmi,
                                       ConfigKeyVal *ctrlr_key) {
  UPLL_FUNC_TRACE;
  val_vtn_flowfilter_entry_ctrlr_t *ctrlr_val;
  val_vtn_flowfilter_entry_t *val_main = reinterpret_cast
      <val_vtn_flowfilter_entry_t *>(GetVal(main_ckv));
  unc_keytype_configstatus_t  ctrlr_status;
  uint8_t cs_status;
  ctrlr_status = (driver_result == UPLL_RC_SUCCESS) ?
                  UNC_CS_APPLIED : UNC_CS_NOT_APPLIED;
  ctrlr_val = reinterpret_cast
    <val_vtn_flowfilter_entry_ctrlr_t *>(GetVal(ctrlr_key));
  if ((val_main == NULL) || (ctrlr_val == NULL))
    return UPLL_RC_ERR_GENERIC;
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

      UPLL_LOG_TRACE("cs_status for main tbl %d\n", cs_status);
      val_main->cs_row_status = cs_status;
  }
  // Updating the Controller cs_row_status
  val_vtn_flowfilter_entry_ctrlr *run_ctrlr_val =
          reinterpret_cast<val_vtn_flowfilter_entry_ctrlr *>
                                               (GetVal(upd_key));
  if ((op == UNC_OP_UPDATE) && (run_ctrlr_val != NULL)) {
    void *valmain = reinterpret_cast<void *>(val_main);
    CompareValidValue(valmain, (GetVal(upd_key)), true);
    for (unsigned int loop = 0; loop < sizeof(val_main->valid)/
      sizeof(val_main->valid[0]); ++loop) {
      if ((val_main->valid[loop] != UNC_VF_INVALID) &&
                (val_main->valid[loop]!= UNC_VF_VALID_NO_VALUE)) {
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
          if (run_ctrlr_val->valid[loop] == UNC_VF_VALID) {
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
          if (run_ctrlr_val->valid[loop] == UNC_VF_VALID) {
            if (val_main->cs_attr[loop] == UNC_CS_PARTIALLY_APPLIED) {
              val_main->cs_attr[loop]  = UNC_CS_PARTIALLY_APPLIED;
            } else if (val_main->cs_attr[loop] == UNC_CS_NOT_APPLIED) {
               val_main->cs_attr[loop]  = UNC_CS_NOT_APPLIED;
            } else if (val_main->cs_attr[loop] == UNC_CS_INVALID) {
              val_main->cs_attr[loop] = UNC_CS_INVALID;
            } else if (val_main->cs_attr[loop] == UNC_CS_UNKNOWN) {
               val_main->cs_attr[loop]  = UNC_CS_NOT_APPLIED;
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
            cs_status = UNC_CS_PARTIALLY_APPLIED;
          } else if (val_main->cs_attr[loop] == UNC_CS_INVALID) {
            cs_status = UNC_CS_INVALID;
          } else {
            cs_status = val_main->cs_attr[loop];
          }
        } else if (ctrlr_status == UNC_CS_NOT_APPLIED) {
          if (val_main->cs_attr[loop] == UNC_CS_NOT_APPLIED) {
            cs_status =  UNC_CS_NOT_APPLIED;
          } else if (val_main->cs_attr[loop] == UNC_CS_APPLIED) {
            cs_status = UNC_CS_PARTIALLY_APPLIED;
          } else if (val_main->cs_attr[loop] == UNC_CS_INVALID) {
            cs_status = UNC_CS_INVALID;
          } else if (val_main->cs_attr[loop] == UNC_CS_UNKNOWN) {
            cs_status =  UNC_CS_NOT_APPLIED;
          }
        } else {
          cs_status =  UNC_CS_PARTIALLY_APPLIED;
        }

        val_main->cs_attr[loop]  = cs_status;
        UPLL_LOG_DEBUG("Main tbl cs_attr : %d", val_main->cs_attr[loop]);
      }
    }
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtnFlowFilterEntryMoMgr::Get_Tx_Consolidated_Status(
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

upll_rc_t VtnFlowFilterEntryMoMgr::IsRenamed(ConfigKeyVal *ikey,
                               upll_keytype_datatype_t dt_type,
                               DalDmlIntf *dmi,
                               uint8_t &rename) {
  UPLL_FUNC_TRACE;
  if (ikey == NULL) {
    UPLL_LOG_DEBUG("Input ConfigKey cannot be NULL");
    return UPLL_RC_ERR_GENERIC;
  }

  DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutFlag | kOpInOutCtrlr
                       | kOpInOutDomain };
  ConfigKeyVal *okey = NULL;
  upll_rc_t result_code;
  /* rename is set implies user wants the ikey
   * populated with val from db */
  if (!rename) {
    if (UNC_KT_VTN_FLOWFILTER_ENTRY == ikey->get_key_type()) {
      UPLL_LOG_DEBUG("UNC_KT_VTN_FLOWFILTER_ENTRY");
      result_code = GetChildConfigKey(okey, ikey);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_TRACE("Returning error %d", result_code);
        return result_code;
      }
    } else if (UNC_KT_VTN_FLOWFILTER_CONTROLLER == ikey->get_key_type()) {
      UPLL_LOG_DEBUG("UNC_KT_VTN_FLOWFILTER_CONTROLLER");

      key_vtn_flowfilter_entry *out_key =
           reinterpret_cast<key_vtn_flowfilter_entry *>
           (ConfigKeyVal::Malloc(sizeof(key_vtn_flowfilter_entry)));
      key_vtn_flowfilter_controller *in_key = reinterpret_cast
        <key_vtn_flowfilter_controller_t *>(ikey->get_key());
      val_flowfilter_controller_t *ival = reinterpret_cast
         <val_flowfilter_controller_t *>(GetVal(ikey));

      uuu::upll_strncpy(out_key->flowfilter_key.vtn_key.vtn_name,
          in_key->vtn_key.vtn_name,
          (kMaxLenVtnName + 1));

      out_key->flowfilter_key.input_direction = ival->direction;
      out_key->sequence_num = ival->sequence_num;

      okey = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_ENTRY,
                 IpctSt::kIpcStKeyVtnFlowfilterEntry,
                 out_key, NULL);
    } else {
      return UPLL_RC_ERR_GENERIC;
    }
  } else {
    okey = ikey;
  }
  result_code = ReadConfigDB(okey, dt_type, UNC_OP_READ, dbop, dmi,
                                       MAINTBL);
  if ((result_code != UPLL_RC_SUCCESS) &&
       (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE))  {
    UPLL_LOG_DEBUG("Returning error code %d", result_code);
    if (okey != ikey) delete okey;
    return UPLL_RC_ERR_GENERIC;
  }
  GET_USER_DATA_FLAGS(okey, rename);
  UPLL_LOG_DEBUG("rename flag %d", rename);
  controller_domain_t ctrlr_dom;
  ctrlr_dom.ctrlr = NULL;
  ctrlr_dom.domain = NULL;

  GET_USER_DATA_CTRLR_DOMAIN(ikey, ctrlr_dom);
    UPLL_LOG_DEBUG("Controller : %s; Domain : %s", ctrlr_dom.ctrlr,
                 ctrlr_dom.domain);
  SET_USER_DATA_CTRLR_DOMAIN(okey, ctrlr_dom);
    UPLL_LOG_DEBUG("Controller : %s; Domain : %s", ctrlr_dom.ctrlr,
                 ctrlr_dom.domain);

  SET_USER_DATA(ikey, okey);
  rename &= RENAME;
  if (okey != ikey) delete okey;
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtnFlowFilterEntryMoMgr::SetRenameFlag(ConfigKeyVal *ikey,
    DalDmlIntf *dmi,
    IpcReqRespHeader *req) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  val_vtn_flowfilter_entry_t *val_ffe = reinterpret_cast
    <val_vtn_flowfilter_entry_t *>(GetVal(ikey));
  if (!val_ffe) {
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
              UNC_KT_VTN_FLOWFILTER)));
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
    if ((UNC_VF_VALID == val_ffe->valid[UPLL_IDX_FLOWLIST_NAME_VFFE]) &&
        ((UNC_OP_CREATE == req->operation))) {
      ConfigKeyVal *fl_ckv = NULL;
      result_code = GetFlowlistConfigKey(reinterpret_cast<const char *>
          (val_ffe->flowlist_name), fl_ckv, dmi);
      if (UPLL_RC_SUCCESS != result_code) {
        UPLL_LOG_DEBUG("GetFlowlistConfigKey failed %d", result_code);
        return result_code;
      }
      MoMgrImpl *fl_mgr =
      reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>(GetMoManager(
              UNC_KT_FLOWLIST)));
      if (NULL == fl_mgr) {
        UPLL_LOG_DEBUG("fl_mgr is NULL");
        DELETE_IF_NOT_NULL(fl_ckv);
        return UPLL_RC_ERR_GENERIC;
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
        rename |= FLOWLIST_RENAME;
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
    if (UNC_VF_VALID == val_ffe->valid[UPLL_IDX_FLOWLIST_NAME_VFFE]) {
      ConfigKeyVal *fl_ckv = NULL;
      result_code = GetFlowlistConfigKey(reinterpret_cast<const char *>
          (val_ffe->flowlist_name), fl_ckv, dmi);
      if (UPLL_RC_SUCCESS != result_code) {
        UPLL_LOG_DEBUG("GetFlowlistConfigKey failed %d", result_code);
        return result_code;
      }
      MoMgrImpl *fl_mgr =
      reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>(GetMoManager(
              UNC_KT_FLOWLIST)));
      if (NULL == fl_mgr) {
        UPLL_LOG_DEBUG("fl_mgr is NULL");
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
        UPLL_LOG_DEBUG("rename flag in flowlist %d", fl_rename);
        rename |= FLOWLIST_RENAME;
      } else {
        rename &= NO_FLOWLIST_RENAME;
        /* reset flag*/
      }
      DELETE_IF_NOT_NULL(fl_ckv);
    } else if (UNC_VF_VALID_NO_VALUE == val_ffe->valid
               [UPLL_IDX_FLOWLIST_NAME_VFFE]) {
       // No rename flowlist value should be set
       rename &= NO_FLOWLIST_RENAME;
    }
    UPLL_LOG_DEBUG("Setting flag in update %d", rename);
    SET_USER_DATA_FLAGS(ikey, rename);
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtnFlowFilterEntryMoMgr::GetFlowlistConfigKey(
        const char *flowlist_name, ConfigKeyVal *&okey,
        DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_ERR_GENERIC;
  MoMgrImpl *mgr =
      reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>(GetMoManager(
              UNC_KT_FLOWLIST)));
  if (mgr == NULL) {
    UPLL_LOG_DEBUG("Invalid momgr object for KT_FLOWLIST");
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

bool VtnFlowFilterEntryMoMgr::IsAllAttrInvalid(
        val_vtn_flowfilter_entry_t *val) {
  for ( unsigned int loop = 0;
      loop < sizeof(val->valid)/sizeof(val->valid[0]); ++loop ) {
    if (UNC_VF_INVALID != val->valid[loop])
      return false;
  }
  return true;
}

upll_rc_t VtnFlowFilterEntryMoMgr::GetOperation(uuc::UpdateCtrlrPhase phase,
                                                unc_keytype_operation_t &op) {
  if (uuc::kUpllUcpDelete2 == phase) {
    UPLL_LOG_DEBUG("Delete phase 1");
    return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_KT;
  } else if (uuc::kUpllUcpUpdate == phase) {
    op = UNC_OP_UPDATE;
  } else if (uuc::kUpllUcpCreate == phase) {
    op = UNC_OP_CREATE;
  } else if (uuc::kUpllUcpDelete == phase) {
    op = UNC_OP_DELETE;
  } else if (uuc::kUpllUcpInit == phase) {
    return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_KT;
  } else {
    return UPLL_RC_ERR_GENERIC;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtnFlowFilterEntryMoMgr::AdaptValToDriver(
    ConfigKeyVal *ck_new, ConfigKeyVal *ck_old,
    unc_keytype_operation_t op, upll_keytype_datatype_t dt_type,
    unc_key_type_t keytype, DalDmlIntf *dmi,
    bool &not_send_to_drv, bool audit_update_phase) {
  UPLL_FUNC_TRACE;
  if (NULL == ck_new) {
    UPLL_LOG_DEBUG("ck_new is NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  upll_rc_t result_code = PerformSemanticCheckForNWM(ck_new, dmi , dt_type);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("PerformSemanticCheckForNWM failed");
    return result_code;
  }
  return UPLL_RC_SUCCESS;
}
upll_rc_t VtnFlowFilterEntryMoMgr::CreatePIForVtnPom(IpcReqRespHeader *req,
                                                     ConfigKeyVal *ikey,
                                                     DalDmlIntf *dmi,
                                                     const char *ctrlr_id) {
  UPLL_FUNC_TRACE;
  if (ikey == NULL || req == NULL) {
     UPLL_LOG_TRACE(
        "Cannot perform create operation due to insufficient parameters");
    return UPLL_RC_ERR_GENERIC;
  }
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  upll_rc_t vtn_ctrlr_span_rt_code = UPLL_RC_SUCCESS;
  std::list<controller_domain_t> list_ctrlr_dom;
  ConfigKeyVal *tmp_ckv = NULL;
  controller_domain ctrlr_dom;
  ctrlr_dom.ctrlr = NULL;
  ctrlr_dom.domain = NULL;
  uint8_t flag = 0;

  result_code = GetRenamedUncKey(ikey, req->datatype, dmi,
                                 reinterpret_cast<uint8_t *>(
                                 const_cast<char *>(ctrlr_id)));
  if (UPLL_RC_SUCCESS != result_code &&
      UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
      UPLL_LOG_DEBUG("GetRenamedUncKey failed %d", result_code);
      return result_code;
  }

  result_code = ValidateAttribute(ikey, dmi, req);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("ValidateAttribute failed: err code(%d)", result_code);
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
  result_code = UpdateConfigDB(ikey, req->datatype, UNC_OP_CREATE,
                               dmi, config_mode, vtn_name, MAINTBL);
  if (result_code == UPLL_RC_ERR_INSTANCE_EXISTS) {
    UPLL_LOG_DEBUG("flowfilter entry objects exists already %d", result_code);
    result_code = DupConfigKeyVal(tmp_ckv, ikey, MAINTBL);
    if (result_code != UPLL_RC_SUCCESS) {
       UPLL_LOG_DEBUG("DupConfigKeyVal failed: err code(%d)", result_code);
       return result_code;
    }
    result_code = CompareValueStructure(tmp_ckv, req->datatype, dmi);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("CompareValueStructure failed: err code(%d)", result_code);
      DELETE_IF_NOT_NULL(tmp_ckv);
      return result_code;
    }
    DELETE_IF_NOT_NULL(tmp_ckv);
  }
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("UpdateConfigDB failed: err code(%d)", result_code);
    return result_code;
  }

  GET_USER_DATA_FLAGS(ikey, flag);
  UPLL_LOG_DEBUG("flag value (%d)", flag);
  if (flag & VTN_RENAME) {
    GET_USER_DATA_CTRLR_DOMAIN(ikey, ctrlr_dom);
    list_ctrlr_dom.push_back(ctrlr_dom);
  } else {
    vtn_ctrlr_span_rt_code = GetVtnControllerSpan(ikey, req->datatype, dmi,
                                                  list_ctrlr_dom);
    if ((vtn_ctrlr_span_rt_code != UPLL_RC_SUCCESS) &&
      (vtn_ctrlr_span_rt_code != UPLL_RC_ERR_NO_SUCH_INSTANCE)) {
      UPLL_LOG_DEBUG("GetVtnControllerSpan  error code (%d)",
                      vtn_ctrlr_span_rt_code);
      FREE_LIST_CTRLR(list_ctrlr_dom);
      return vtn_ctrlr_span_rt_code;
    }
  }

  // create a record in CANDIDATE DB for controller Table
  result_code = UpdateControllerTable(ikey, UNC_OP_CREATE,
                                      req->datatype, dmi,
                                      list_ctrlr_dom,
                                      config_mode, vtn_name);

  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("create in ctrlr tbl failed: error code (%d)",
                    result_code);
    upll_rc_t del_result_code = UpdateConfigDB(ikey, req->datatype,
                                               UNC_OP_DELETE,
                                               dmi, config_mode, vtn_name,
                                               MAINTBL);
    if (del_result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("delete in CandidateDB failed: err code(%d) ",
                        del_result_code);
    }
    if (!(flag & VTN_RENAME))
      FREE_LIST_CTRLR(list_ctrlr_dom);
    return result_code;
  }

  val_vtn_flowfilter_entry_t *val_vtn_ffe = reinterpret_cast
    <val_vtn_flowfilter_entry_t *>(GetVal(ikey));
  if (UNC_VF_VALID == val_vtn_ffe->valid[UPLL_IDX_FLOWLIST_NAME_VFFE]) {
    result_code = UpdateFlowListInCtrlTbl(ikey, req->datatype, ctrlr_id, dmi,
                                          config_mode, vtn_name);
    if (result_code != UPLL_RC_SUCCESS) {
      if (!(flag & VTN_RENAME)) {
        FREE_LIST_CTRLR(list_ctrlr_dom);
      }
      return result_code;
    }
  }
  if (!(flag & VTN_RENAME))
    FREE_LIST_CTRLR(list_ctrlr_dom);

  return result_code;
}

upll_rc_t VtnFlowFilterEntryMoMgr::CompareValueStructure(ConfigKeyVal *tmp_ckv,
                                    upll_keytype_datatype_t datatype,
                                     DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_ERR_GENERIC;
  ConfigKeyVal *imp_ckv = NULL;

  result_code = GetChildConfigKey(imp_ckv, tmp_ckv);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetChildConfigKey failed %d", result_code);
    return result_code;
  }
  DbSubOp dbop = {kOpReadMultiple, kOpMatchNone,
    kOpInOutCtrlr | kOpInOutDomain};
  // Read the Configuration from the MainTable
  result_code = ReadConfigDB(imp_ckv, datatype,
                             UNC_OP_READ, dbop, dmi, MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
      DELETE_IF_NOT_NULL(imp_ckv);
      UPLL_LOG_DEBUG("ReadConfigDB err- vtn_ff_entry main tbl");
      return result_code;
  }
  result_code = CompareValStructure(GetVal(tmp_ckv), GetVal(imp_ckv));
  DELETE_IF_NOT_NULL(imp_ckv);
  return result_code;
}

upll_rc_t VtnFlowFilterEntryMoMgr::CompareValStructure(void *val1,
                                                       void *val2) {
  UPLL_FUNC_TRACE;
  val_vtn_flowfilter_entry_t *val_ff_entry1 =
      reinterpret_cast<val_vtn_flowfilter_entry_t *>(val1);
  val_vtn_flowfilter_entry_t *val_ff_entry2 =
      reinterpret_cast<val_vtn_flowfilter_entry_t *>(val2);

  UPLL_LOG_DEBUG("comparevalstruct");
  for ( unsigned int loop = 0;
        loop < sizeof(val_ff_entry1->valid)/sizeof(val_ff_entry1->valid[0]);
        ++loop ) {
    if (val_ff_entry1->valid[loop] != val_ff_entry2->valid[loop]) {
      UPLL_LOG_DEBUG("Valid flag mismatched");
      return UPLL_RC_ERR_MERGE_CONFLICT;
    }
  }

  if (val_ff_entry1->valid[UPLL_IDX_FLOWLIST_NAME_VFFE] == UNC_VF_VALID &&
      val_ff_entry2->valid[UPLL_IDX_FLOWLIST_NAME_VFFE] == UNC_VF_VALID) {
    if (strcmp(reinterpret_cast<char *>(val_ff_entry1->flowlist_name),
               reinterpret_cast<char *>(val_ff_entry2->flowlist_name))) {
       UPLL_LOG_DEBUG("Flowlist name mismatched");
       return UPLL_RC_ERR_MERGE_CONFLICT;
    }
  }
  if (val_ff_entry1->valid[UPLL_IDX_ACTION_VFFE] == UNC_VF_VALID &&
      val_ff_entry2->valid[UPLL_IDX_ACTION_VFFE] == UNC_VF_VALID) {
    if (val_ff_entry1->action !=
        val_ff_entry2->action) {
        UPLL_LOG_DEBUG("Action mismatched");
       return UPLL_RC_ERR_MERGE_CONFLICT;
    }
  }
  if (val_ff_entry1->valid[UPLL_IDX_NWN_NAME_VFFE] == UNC_VF_VALID &&
      val_ff_entry2->valid[UPLL_IDX_NWN_NAME_VFFE] == UNC_VF_VALID) {
    if (strcmp(reinterpret_cast<char *>(val_ff_entry1->nwm_name),
                reinterpret_cast<char *>(val_ff_entry2->nwm_name))) {
      UPLL_LOG_DEBUG("Network monitor name mismatched");
      return UPLL_RC_ERR_MERGE_CONFLICT;
    }
  }

  if (val_ff_entry1->valid[UPLL_IDX_DSCP_VFFE] == UNC_VF_VALID &&
      val_ff_entry2->valid[UPLL_IDX_DSCP_VFFE] == UNC_VF_VALID) {
    if (val_ff_entry1->dscp !=
        val_ff_entry2->dscp) {
      UPLL_LOG_DEBUG("dscp mismatched");
      return UPLL_RC_ERR_MERGE_CONFLICT;
    }
  }
  if (val_ff_entry1->valid[UPLL_IDX_PRIORITY_VFFE] == UNC_VF_VALID &&
      val_ff_entry2->valid[UPLL_IDX_PRIORITY_VFFE] == UNC_VF_VALID) {
    if (val_ff_entry1->priority !=
        val_ff_entry2->priority) {
      UPLL_LOG_DEBUG("Priority mismatched");
      return UPLL_RC_ERR_MERGE_CONFLICT;
    }
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtnFlowFilterEntryMoMgr::UpdateFlowListInCtrlTbl(
                                   ConfigKeyVal *ikey,
                                   upll_keytype_datatype_t dt_type,
                                   const char *ctrlr_id,
                                   DalDmlIntf* dmi,
                                   TcConfigMode config_mode,
                                   string vtn_name) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  FlowListMoMgr *flowlist_mgr = reinterpret_cast<FlowListMoMgr *>
  (const_cast<MoManager *> (GetMoManager(UNC_KT_FLOWLIST)));
  key_vtn_flowfilter_entry_t *vtn_ikey = reinterpret_cast
      <key_vtn_flowfilter_entry_t *>(ikey->get_key());
  val_vtn_flowfilter_entry_t *flowfilter_val =
           reinterpret_cast<val_vtn_flowfilter_entry_t *> (GetVal(ikey));

  UPLL_LOG_TRACE("flowlist name %s length %zu", flowfilter_val->flowlist_name,
                  strlen((const char *)flowfilter_val->flowlist_name));

  std::string temp_vtn_name;
  if (TC_CONFIG_VTN == config_mode) {
    temp_vtn_name = vtn_name;
  } else {
    temp_vtn_name = reinterpret_cast<const char*>(vtn_ikey->
                                                  flowfilter_key.vtn_key.
                                                  vtn_name);
  }
  result_code = flowlist_mgr->AddFlowListToController(
      reinterpret_cast<char *>(flowfilter_val->flowlist_name), dmi,
      reinterpret_cast<char *>(const_cast<char *>(ctrlr_id)),
      dt_type, UNC_OP_CREATE, config_mode, temp_vtn_name, false);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_TRACE("AddFlowListToController failed");
    return result_code;
  }
  return result_code;
}

upll_rc_t VtnFlowFilterEntryMoMgr::GetDomainsForController(
    ConfigKeyVal *ckv_drvr,
    ConfigKeyVal *&ctrlr_ckv,
    DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = GetChildConfigKey(ctrlr_ckv, ckv_drvr);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetChildConfigKey failed %d", result_code);
    return result_code;
  }
  DbSubOp dbop = { kOpReadMultiple, kOpMatchCtrlr,
    kOpInOutCtrlr | kOpInOutDomain};
  return ReadConfigDB(ctrlr_ckv, UPLL_DT_RUNNING, UNC_OP_READ, dbop, dmi,
                             CTRLRTBL);
}
}  // namespace kt_momgr
}  // namespace upll
}  // namespace unc
