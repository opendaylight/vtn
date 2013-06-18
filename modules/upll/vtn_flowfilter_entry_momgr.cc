/*
 * Copyright (c) 2012-2013 NEC Corporation
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
#include "upll_log.hh"
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

#define NUM_KEY_MAIN_TBL_        5
#define NUM_KEY_RENAME_MAIN_TBL  5
#define NUM_KEY_CTRLR_TBL        5

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
  {uudst::vtn_flowfilter_entry::kDbiInputDirection, CFG_MATCH_KEY, offsetof(
    key_vtn_flowfilter_entry_t, flowfilter_key.input_direction),
    uud::kDalUint8, 1},
  {uudst::vtn_flowfilter_entry::kDbiSequenceNum, CFG_MATCH_KEY, offsetof(
    key_vtn_flowfilter_entry_t, sequence_num),
    uud::kDalUint8, 1 },
  {uudst::vtn_flowfilter_entry::kDbiVtnName, CFG_INPUT_KEY, offsetof(
    key_rename_vnode_info_t, new_unc_vtn_name),
    uud::kDalChar, kMaxLenVtnName + 1},
  {uudst::vtn_flowfilter_entry::kDbiFlags, CFG_INPUT_KEY, offsetof(
    key_user_data_t, flags),
    uud::kDalUint8, 1}
};

BindInfo VtnFlowFilterEntryMoMgr::vtnflowfilterentryctrlrtbl_bind_info[] = {
  {uudst::vtn_flowfilter_entry_ctrlr::kDbiVtnName, CFG_MATCH_KEY, offsetof(
    key_vtn_flowfilter_t, vtn_key.vtn_name),
    uud::kDalChar, kMaxLenVtnName + 1},
  { uudst::vtn_flowfilter_entry_ctrlr::kDbiInputDirection, CFG_MATCH_KEY,
    offsetof(key_vtn_flowfilter_t, input_direction),
    uud::kDalUint8, 1},
  {uudst::vtn_flowfilter_entry_ctrlr::kDbiVtnName, CFG_INPUT_KEY, offsetof(
    key_rename_vnode_info_t, new_unc_vtn_name),
    uud::kDalChar, kMaxLenVtnName + 1},
  { uudst::vtn_flowfilter_entry_ctrlr::kDbiCtrlrName, CK_VAL,
    offsetof(key_user_data_t, ctrlr_id),
    uud::kDalChar, kMaxLenCtrlrId + 1},
  {uudst::vtn_flowfilter_entry_ctrlr::kDbiDomainId, CK_VAL,
    offsetof(key_user_data_t, domain_id),
    uud::kDalChar, kMaxLenDomainId + 1},
  {uudst::vtn_flowfilter_entry_ctrlr::kDbiFlags, CFG_INPUT_KEY, offsetof(
    key_user_data_t, flags),
    uud::kDalUint8, 1}
};

BindInfo VtnFlowFilterEntryMoMgr::vtn_flowlist_rename_bind_info[] = {
  {uudst::vtn_flowfilter_entry::kDbiVtnName, CFG_MATCH_KEY, offsetof(
     key_vtn_flowfilter_entry_t, flowfilter_key.vtn_key.vtn_name),
     uud::kDalChar, kMaxLenVtnName + 1},
  {uudst::vtn_flowfilter_entry::kDbiInputDirection, CFG_MATCH_KEY, offsetof(
    key_vtn_flowfilter_entry_t, flowfilter_key.input_direction),
    uud::kDalUint8, 1},
  {uudst::vtn_flowfilter_entry::kDbiSequenceNum, CFG_MATCH_KEY, offsetof(
     key_vtn_flowfilter_entry_t, sequence_num),
     uud::kDalUint16, 1},
  {uudst::vtn_flowfilter_entry::kDbiFlowlistName, CFG_INPUT_KEY, offsetof(
     key_rename_vnode_info_t, new_flowlist_name),
     uud::kDalChar, kMaxLenVtnName + 1},
  {uudst::vtn_flowfilter_entry::kDbiFlags, CFG_INPUT_KEY, offsetof(
     key_user_data_t, flags),
     uud::kDalUint8, 1}
};

VtnFlowFilterEntryMoMgr::VtnFlowFilterEntryMoMgr() :MoMgrImpl() {
  UPLL_FUNC_TRACE;
  // Rename operation is not support for this KT
  // setting max tables to 2
  ntable = MAX_MOMGR_TBLS;
  table = new Table *[ntable];
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

  // VTN FlowFilter Entry Does not have any child
  nchild = 0;
  child = NULL;
}

upll_rc_t VtnFlowFilterEntryMoMgr::IsReferenced(
                                               ConfigKeyVal *ikey,
                                               upll_keytype_datatype_t dt_type,
                                               DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  if (NULL == ikey) return UPLL_RC_ERR_GENERIC;
  DbSubOp dbop = { kOpReadExist, kOpMatchNone, kOpInOutNone };
  // Check the exixtence in Maintable
  result_code = UpdateConfigDB(ikey, dt_type, UNC_OP_READ, dmi, &dbop, MAINTBL);
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
                                                 DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *ikey = NULL;
  std::list<controller_domain_t> list_ctrlr_dom;
  uint8_t *ctrlr_id = ctrlr_dom->ctrlr;
  if (!ctrlr_id) {
    UPLL_LOG_TRACE(" Ctrlr_id is NULL");
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
  result_code = ReadConfigDB(ikey, UPLL_DT_CANDIDATE,
                             UNC_OP_READ, dbop, dmi, MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      UPLL_LOG_DEBUG(" No Records in main table to be created in ctrlr tbl");
      return UPLL_RC_SUCCESS;
    }
    UPLL_LOG_DEBUG(" Read main table failed ");
    delete ikey;
    return result_code;
  }

  FlowListMoMgr *flowlist_mgr = reinterpret_cast<FlowListMoMgr *>
      (const_cast<MoManager *> (GetMoManager(UNC_KT_FLOWLIST)));

  while (ikey != NULL) {
    result_code = UpdateControllerTable(ikey, UNC_OP_CREATE,
                                        UPLL_DT_CANDIDATE, dmi,
                                        list_ctrlr_dom);

    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("create in ctrlr tbl failed: error code (%d)",
                     result_code);
    }
    // Notify the flowlistmomgr is flowlist is configured.
    //
    val_vtn_flowfilter_entry_t *val_vtn_ffe =
        reinterpret_cast<val_vtn_flowfilter_entry_t *> (GetVal(ikey));
    // if Flowlist name is configured in the flowfilter
    // send controller add/delete request to flowlist momgr.
    if (UNC_VF_VALID == val_vtn_ffe->valid[UPLL_IDX_FLOWLIST_NAME_VFFE]) {
      result_code = flowlist_mgr->AddFlowListToController(
          reinterpret_cast<char *>(val_vtn_ffe->flowlist_name), dmi,
          reinterpret_cast<char *>(ctrlr_id) , op);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("AddFlowListToController failed err(%d)", result_code)
            return result_code;
      }
    }
    ikey = ikey->get_next_cfg_key_val();
  }
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
  // validate syntax and semantics
  result_code = ValidateMessage(req, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("ValidateMessage failed result code=%d", result_code);
    return result_code;
  }

  result_code = ValidateAttribute(ikey, dmi, req);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("ValidateAttribute failed: err code(%d)", result_code);
    return result_code;
  }
  /*
  result_code = ValidateCapability(req, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("ValidateCapability failed result code=%d", result_code);
    return result_code;
  }
  */
  // Check if Object already exists in CANDIDATE DB
  result_code = UpdateConfigDB(ikey, req->datatype, UNC_OP_READ, dmi);
  if ((result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) &&
      (result_code != UPLL_RC_ERR_INSTANCE_EXISTS)) {
    UPLL_LOG_DEBUG(" UpdateConfigDB() error (%d)", result_code);
    return result_code;
  }
  if (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    UPLL_LOG_DEBUG("Record already exists in Candidate DB: err code(%d)",
                   result_code);
    return result_code;
  }
  // Check if Object exists in RUNNING DB and move it to CANDIDATE DB
  result_code = UpdateConfigDB(ikey, UPLL_DT_RUNNING, UNC_OP_READ, dmi,
                               MAINTBL);
  if ((result_code != UPLL_RC_ERR_INSTANCE_EXISTS) &&
      (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE)) {
    UPLL_LOG_DEBUG(" Is Exist check operation failed: err code(%d)",
                   result_code);
    return result_code;
  }
  if (result_code == UPLL_RC_ERR_INSTANCE_EXISTS) {
    result_code = RestoreChildren(ikey, req->datatype, UPLL_DT_RUNNING, dmi);
    if (result_code != UPLL_RC_SUCCESS) {
      // delete okey;
      return UPLL_RC_ERR_GENERIC;
    }
  }

  std::list<controller_domain_t> list_ctrlr_dom;
  vtn_ctrlr_span_rt_code = GetVtnControllerSpan(ikey, dmi, list_ctrlr_dom);
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
    UPLL_LOG_DEBUG(" GetVtnControllerSpan failed . Resultcode %d ",
                   vtn_ctrlr_span_rt_code);
    return vtn_ctrlr_span_rt_code;
  }

  // create a record in CANDIDATE DB
  result_code = UpdateConfigDB(ikey, req->datatype, UNC_OP_CREATE,
                               dmi, MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    // delete okey;
    return result_code;
  }

// create a record in CANDIDATE DB for controller Table
  if (vtn_ctrlr_span_rt_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    result_code = UpdateControllerTable(ikey, UNC_OP_CREATE,
                                        req->datatype, dmi,
                                        list_ctrlr_dom);

    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("create in ctrlr tbl failed: error code (%d)",
                     result_code);
      result_code = UpdateConfigDB(ikey, req->datatype, UNC_OP_DELETE,
                                   dmi, MAINTBL);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("delete in CandidateDB failed: err code(%d) ",
                       result_code);
      }
    }
  }
  val_vtn_flowfilter_entry_t *val_vtn_ffe = reinterpret_cast
    <val_vtn_flowfilter_entry_t *>(GetVal(ikey));
  if (UNC_VF_VALID == val_vtn_ffe->valid[UPLL_IDX_FLOWLIST_NAME_VFFE]) {
    result_code = UpdateFlowListInCtrl(ikey, UNC_OP_CREATE, dmi);
    if (result_code != UPLL_RC_SUCCESS) {
      // delete okey;
      return result_code;
    }
  }
  // delete okey;
  return result_code;
}

upll_rc_t VtnFlowFilterEntryMoMgr::UpdateFlowListInCtrl(
                                   ConfigKeyVal *ikey,
                                   unc_keytype_operation_t op,
                                   DalDmlIntf* dmi) {
  UPLL_FUNC_TRACE;
  ConfigKeyVal *vtn_okey = NULL;
  uint8_t* ctrlr_id;
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  VtnMoMgr *vtnmgr =
      static_cast<VtnMoMgr *>((const_cast<MoManager *>
      (GetMoManager(UNC_KT_VTN))));
  result_code = vtnmgr->GetChildConfigKey(vtn_okey, NULL);
  if (result_code != UPLL_RC_SUCCESS) {
    return result_code;
  }
  FlowListMoMgr *flowlist_mgr = reinterpret_cast<FlowListMoMgr *>
  (const_cast<MoManager *> (GetMoManager(UNC_KT_FLOWLIST)));
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
  result_code = vtnmgr->GetControllerDomainSpan(vtn_okey, UPLL_DT_CANDIDATE,
                                                dmi);
  if (result_code != UPLL_RC_SUCCESS &&
      UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
    delete vtn_okey;
    UPLL_LOG_DEBUG("Error in getting controller span (%d)",
                   result_code);
    return result_code;
  }
  if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
    UPLL_LOG_DEBUG("No entry in ctrlr tbl (%d)",
                   result_code);
    return UPLL_RC_SUCCESS;
  }
  ConfigKeyVal *tmp_ckv = vtn_okey;
  while (NULL != tmp_ckv) {
    ctrlr_id = NULL;
    GET_USER_DATA_CTRLR(tmp_ckv, ctrlr_id);
    if (NULL == ctrlr_id) {
      UPLL_LOG_DEBUG("ctrlr_id NULL");
      delete vtn_okey;
      return UPLL_RC_ERR_GENERIC;
    }
    UPLL_LOG_TRACE("flowlist name %s length %zu", flowfilter_val->flowlist_name,
                    strlen((const char *)flowfilter_val->flowlist_name));
    if (UNC_OP_CREATE == op || UNC_OP_DELETE == op) {
      result_code = flowlist_mgr->AddFlowListToController(
          reinterpret_cast<char *>(flowfilter_val->flowlist_name), dmi,
          reinterpret_cast<char *> (ctrlr_id) , op);
      if (result_code != UPLL_RC_SUCCESS) {
        delete vtn_okey;
        return result_code;
      }
    } else if (UNC_OP_UPDATE == op) {
      ConfigKeyVal *tempckv = NULL;
      result_code = GetChildConfigKey(tempckv, ikey);
      if (UPLL_RC_SUCCESS != result_code) {
        UPLL_LOG_TRACE("GetChildConfigKey failed");
        return result_code;
      }
      DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutNone };
      result_code = ReadConfigDB(tempckv, UPLL_DT_CANDIDATE, UNC_OP_READ,
                                 dbop, dmi, MAINTBL);
      if (UPLL_RC_SUCCESS != result_code) {
        UPLL_LOG_DEBUG("ReadConfigDB failed");
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
            reinterpret_cast<char *> (ctrlr_id) , UNC_OP_DELETE);
        if (result_code != UPLL_RC_SUCCESS) {
          delete tempckv;
          return result_code;
        }
        result_code = flowlist_mgr->AddFlowListToController(
            reinterpret_cast<char *>(vtn_ffe_val->flowlist_name), dmi,
            reinterpret_cast<char *> (ctrlr_id) , UNC_OP_CREATE);
        if (result_code != UPLL_RC_SUCCESS) {
          delete tempckv;
          return result_code;
        }
      } else if (UNC_VF_VALID == vtn_ffe_val->
                 valid[UPLL_IDX_FLOWLIST_NAME_VFFE] &&
                 (UNC_VF_INVALID == temp_ffe_val->
              valid[UPLL_IDX_FLOWLIST_NAME_VFFE] || UNC_VF_VALID_NO_VALUE ==
              temp_ffe_val->valid[UPLL_IDX_FLOWLIST_NAME_VFFE])) {
        result_code = flowlist_mgr->AddFlowListToController(
            reinterpret_cast<char *>(vtn_ffe_val->flowlist_name), dmi,
            reinterpret_cast<char *> (ctrlr_id) , UNC_OP_CREATE);
        if (result_code != UPLL_RC_SUCCESS) {
          delete tempckv;
          return result_code;
        }
      } else if (UNC_VF_VALID_NO_VALUE == vtn_ffe_val->
                 valid[UPLL_IDX_FLOWLIST_NAME_VFFE] &&
                 UNC_VF_VALID == temp_ffe_val->
              valid[UPLL_IDX_FLOWLIST_NAME_VFFE]) {
        result_code = flowlist_mgr->AddFlowListToController(
            reinterpret_cast<char *>(temp_ffe_val->flowlist_name), dmi,
            reinterpret_cast<char *> (ctrlr_id) , UNC_OP_DELETE);
        if (result_code != UPLL_RC_SUCCESS) {
          delete tempckv;
          return result_code;
        }
      }
    }
    tmp_ckv = tmp_ckv->get_next_cfg_key_val();
  }
  delete vtn_okey;
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

#if 0
  result_code = ValidateCapability(req, ikey);

  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("ValidateCapability failed resultcode=%d",
                      result_code);
    return result_code;
  }
#endif
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
    delete okey;
    return result_code;
  }
  val_vtn_flowfilter_entry_t *vtn_ffe_val = reinterpret_cast
          <val_vtn_flowfilter_entry_t *>(GetVal(okey));
  if (UNC_VF_VALID == vtn_ffe_val->valid[UPLL_IDX_FLOWLIST_NAME_VFFE]) {
    result_code = UpdateFlowListInCtrl(okey, UNC_OP_DELETE, dmi);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Unable to update flowlist in ctrlr table");
      delete okey;
      return result_code;
    }
  }
  delete okey;
  result_code = DeleteCandidateMo(req, ikey, dmi);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Delete Operation Failed DB Error (%d)", result_code);
    return result_code;
  }
/*
  delete okey;
  okey = NULL;

  DbSubOp dbop = {kOpNotRead, kOpMatchNone, kOpInOutNone};
  result_code = UpdateConfigDB(ikey, UPLL_DT_CANDIDATE, UNC_OP_DELETE, dmi,
                                &dbop, MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Delete Operation Failed: err code (%d)", result_code);
    return result_code;
  }
  result_code = UpdateConfigDB(ikey, UPLL_DT_CANDIDATE, UNC_OP_DELETE, dmi,
                               &dbop, CTRLRTBL);
  if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    UPLL_LOG_DEBUG("Delete : No matching record in ctrlrtbl:DB Error");
    return UPLL_RC_SUCCESS;
  }else if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Delete Operation Failed in ctrlrtbl:DB Error");
    return result_code;
  }
  */
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
  ConfigKeyVal *okey = NULL;
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

  /*
     result_code = ValidateCapability(req, ikey);
     if (UPLL_RC_SUCCESS != result_code) {
     UPLL_LOG_DEBUG("Validation of Message failed in UpdateMo");
     return result_code;
     }
     */
  result_code = UpdateConfigDB(ikey, req->datatype, UNC_OP_READ, dmi, MAINTBL);

  if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    UPLL_LOG_DEBUG("UpdateMo  record not available (%d)",
                   result_code);
    return result_code;
  } else if (result_code == UPLL_RC_ERR_INSTANCE_EXISTS) {
    UPLL_LOG_DEBUG("UpdateMo  record available");
  } else if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("UpdateMo  error accessing DB (%d)",
                   result_code);
    return result_code;
  }

  val_vtn_flowfilter_entry_t *vtn_ffe_val = reinterpret_cast
      <val_vtn_flowfilter_entry_t *>(GetVal(ikey));
  if (UNC_VF_VALID == vtn_ffe_val->valid[UPLL_IDX_FLOWLIST_NAME_VFFE]) {
    FlowListMoMgr *flowlist_mgr = reinterpret_cast<FlowListMoMgr *>
        (const_cast<MoManager *> (GetMoManager(UNC_KT_FLOWLIST)));
    result_code = flowlist_mgr->GetChildConfigKey(okey, NULL);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("GetChildConfigKey failed for FlowList %d", result_code);
      return result_code;
    }
    key_flowlist_t *key_flowlist = reinterpret_cast<key_flowlist_t*>
        (okey->get_key());
    uuu::upll_strncpy(key_flowlist->flowlist_name,
                      vtn_ffe_val->flowlist_name,
                      (kMaxLenFlowListName +1));
    result_code = flowlist_mgr->UpdateConfigDB(okey, req->datatype,
                                               UNC_OP_READ, dmi, MAINTBL);
    if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      UPLL_LOG_DEBUG("Given FlowList does not exists %d", result_code);
      delete okey;
      return result_code;
    } else if (result_code == UPLL_RC_ERR_INSTANCE_EXISTS) {
      UPLL_LOG_DEBUG("Instance  Available");
    } else if (result_code != UPLL_RC_SUCCESS) {
      delete okey;
      UPLL_LOG_DEBUG("Error Accesing CANDIDATE DB (%d)", result_code);
      return result_code;
    }
  }
  std::list<controller_domain_t> list_ctrlr_dom;
  vtn_ctrlr_span_rt_code = GetVtnControllerSpan(ikey, dmi, list_ctrlr_dom);
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
    UPLL_LOG_DEBUG(" GetVtnControllerSpan failed . Resultcode %d ",
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
    DELETE_IF_NOT_NULL(okey);
    return result_code;
  }
  if (UNC_VF_VALID == vtn_ffe_val->valid[UPLL_IDX_FLOWLIST_NAME_VFFE] ||
        UNC_VF_VALID_NO_VALUE == vtn_ffe_val->valid[UPLL_IDX_FLOWLIST_NAME_VFFE] ) {
      result_code = UpdateFlowListInCtrl(ikey, UNC_OP_UPDATE, dmi);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Unable to update flowlist in ctrlr table");
      delete okey;
      return result_code;
    }
  }

  result_code = UpdateConfigDB(ikey, req->datatype, UNC_OP_UPDATE, dmi,
                               MAINTBL);

  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("UpdateMo:Failed UpdateConfigDB ");
    delete okey;
    return result_code;
  }
  // Update a record in CANDIDATE DB for controller Table
  if (vtn_ctrlr_span_rt_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    result_code = UpdateControllerTable(ctrl_key, UNC_OP_UPDATE,
                                        req->datatype, dmi,
                                        list_ctrlr_dom);

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
  delete okey;

  DELETE_IF_NOT_NULL(ctrl_key);
  ctrl_key = NULL;
  return result_code;
}

 /* TODO Is Reference and GetChildConfigKey APis of NWM are declared as private
 *   Commented the below code to avoid compilation error of access violation
 *   nwm_momgr.cc code must be corrected */
#if 0
upll_rc_t VtnFlowFilterEntryMoMgr::IsNWMReferenced(ConfigKeyVal *ikey,
                                                   DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  ConfigKeyVal *okey = NULL;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  NwMonitorMoMgr *nmgr = reinterpret_cast<NwMonitorMoMgr *>
      (const_cast<MoManager *> (GetMoManager(UNC_KT_VBR_NWMONITOR)));
  result_code = nmgr->GetChildConfigKey(okey, NULL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Recored %d", result_code);
    return result_code;
  }
  val_flowfilter_entry_t *flowfilter_val =
      reinterpret_cast<val_flowfilter_entry_t *> (GetVal(ikey));

  key_nwm_t *key_nwm = reinterpret_cast<key_nwm_t*>(okey->get_key());
  strncpy(reinterpret_cast<char*>(key_nwm->nwmonitor_name),
          reinterpret_cast<const char*>(flowfilter_val->nwm_name),
          kMaxLenNwmName +1);
  result_code = nmgr->IsReferenced(okey, UPLL_DT_CANDIDATE, dmi);
  if (result_code != UPLL_RC_SUCCESS) {
    if (UPLL_RC_ERR_INSTANCE_EXISTS != result_code) {
      delete okey;
      UPLL_LOG_DEBUG("Unable to get the NWM Reference %d", result_code);
      return result_code;
    }
  }
}
#endif
upll_rc_t VtnFlowFilterEntryMoMgr::MergeValidate(unc_key_type_t keytype,
                                                 const char *ctrlr_id,
                                                 ConfigKeyVal *ikey,
                                                 DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  DbSubOp dbop = { kOpReadMultiple, kOpMatchNone, kOpInOutNone };
  ConfigKeyVal *tkey;
  ConfigKeyVal *flowlist_keyval = NULL;

  if (NULL == ikey) return result_code;
  result_code = ReadConfigDB(ikey, UPLL_DT_IMPORT, UNC_OP_READ, dbop, dmi,
                             MAINTBL);
  if (UPLL_RC_SUCCESS != result_code) return result_code;
  MoMgrImpl *mgr = static_cast<MoMgrImpl*>
                  ((const_cast<MoManager*>
               (GetMoManager(UNC_KT_FLOWLIST))));
  tkey = ikey;
  while (ikey != NULL) {
    result_code = mgr->GetChildConfigKey(flowlist_keyval, NULL);
    if (result_code != UPLL_RC_SUCCESS) {
      return result_code;
    }

    val_vtn_flowfilter_entry_t* val_flowfilter_entry = reinterpret_cast
        <val_vtn_flowfilter_entry_t*>(GetVal(ikey));
    key_flowlist_t *key_flowlist = reinterpret_cast
        <key_flowlist_t *> (flowlist_keyval->get_key());
    uuu::upll_strncpy(key_flowlist->flowlist_name,
                      val_flowfilter_entry->flowlist_name,
                      (kMaxLenFlowListName + 1));
    result_code = mgr->UpdateConfigDB(flowlist_keyval, UPLL_DT_CANDIDATE,
                                      UNC_OP_READ, dmi, MAINTBL);
    if (result_code != UPLL_RC_ERR_INSTANCE_EXISTS) {
      delete flowlist_keyval;
      UPLL_LOG_DEBUG("MergeValidate Failed:Conflict occured");
      return UPLL_RC_ERR_MERGE_CONFLICT;
    }
    ikey = tkey->get_next_cfg_key_val();
  }
  delete flowlist_keyval;
  return result_code;
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
    vtn_ffe_key = reinterpret_cast<key_vtn_flowfilter_entry_t *>
        (okey->get_key());
  } else {
    vtn_ffe_key = reinterpret_cast<key_vtn_flowfilter_entry_t *>
        (ConfigKeyVal::Malloc(sizeof(key_vtn_flowfilter_entry_t)));
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
    default:
      if (vtn_ffe_key) free(vtn_ffe_key);
      return UPLL_RC_ERR_GENERIC;
  }
  if (!okey)
    okey = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVtnFlowfilterEntry,
                            vtn_ffe_key, NULL);

  SET_USER_DATA(okey, parent_key);
  return result_code;
}

upll_rc_t VtnFlowFilterEntryMoMgr::GetRenamedControllerKey(
    ConfigKeyVal *&ikey, upll_keytype_datatype_t dt_type, DalDmlIntf *dmi,
    controller_domain *ctrlr_dom) {
  UPLL_FUNC_TRACE;
  ConfigKeyVal *okey = NULL;
  upll_rc_t result_code;
  uint8_t rename = 0, rename_flowlist = 0;
  // TODO(UNC) :changed to fix compilation issue
  DbSubOp dbop = { kOpReadSingle, kOpMatchCtrlr | kOpMatchDomain,
                   kOpInOutCtrlr };
  // Check if VTN is renamed on the controller by getting VTN object
  MoMgrImpl *mgr =  static_cast<MoMgrImpl*>
    ((const_cast<MoManager*>
    (GetMoManager(UNC_KT_VTN))));
  result_code = mgr->GetChildConfigKey(okey, NULL);
  if (UPLL_RC_SUCCESS != result_code) {
     UPLL_LOG_DEBUG("GetRenamedControllerKey:GetChildConfigKey returns error");
     return result_code;
    }
  // mgr->IsRenamed(ikey, dt_type, dmi, rename);
  result_code = IsRenamed(ikey, dt_type, dmi, rename);
  if (result_code != UPLL_RC_SUCCESS) {
     UPLL_LOG_DEBUG("IsRenamed returns error");
     delete okey;
     return result_code;
  }
  if (!rename) {
    delete okey;
    UPLL_LOG_DEBUG(" Not Renamed ");
    return UPLL_RC_SUCCESS;
  }
  /* Vtn renamed */
  if (rename & 0x01) {
    if (ctrlr_dom == NULL) {
      UPLL_LOG_DEBUG("GetRenamedControllerKey:Controller Name cannot be null");
      delete okey;
      return UPLL_RC_ERR_GENERIC;
    }
    SET_USER_DATA_CTRLR_DOMAIN(okey, *ctrlr_dom);
    // Copy the input VTN Name into the Okey and send it for rename check IN db
    uuu::upll_strncpy(
                      reinterpret_cast<key_vtn *>(okey->get_key())->vtn_name,
                      reinterpret_cast<key_vtn_flowfilter_entry_t *>
                      (ikey->get_key())->flowfilter_key.vtn_key.vtn_name,
                      (kMaxLenVtnName + 1));
     result_code = mgr->ReadConfigDB(okey,
                                    dt_type,
                                    UNC_OP_READ, dbop, dmi, RENAMETBL);
     if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("GetRenamedControllerKey:Read Configuration Error");
      return result_code;
     }
     val_rename_vtn *rename_val =reinterpret_cast <val_rename_vtn *>
     (GetVal(okey));
    if (!rename_val
        || (rename_val->valid[UPLL_IDX_NEW_NAME_RVTN] != UNC_VF_VALID)) {
      UPLL_LOG_DEBUG("Rename structure for VTN is not available");
      return UPLL_RC_ERR_GENERIC;
    }
    uuu::upll_strncpy(
                      reinterpret_cast<key_vtn_flowfilter_entry_t*>
                      (ikey->get_key())->flowfilter_key.vtn_key.vtn_name,
                      rename_val->new_name,
                      (kMaxLenVtnName + 1));
    SET_USER_DATA_FLAGS(ikey, VTN_RENAME);
  }
  okey = NULL;
  mgr = NULL;
  mgr =  static_cast<MoMgrImpl*>
    ((const_cast<MoManager*> (GetMoManager(UNC_KT_FLOWLIST))));
  result_code = mgr->GetChildConfigKey(okey, NULL);
  if (result_code != UPLL_RC_SUCCESS) {
    return result_code;
  }

  // Copy the input VTN Name into the Okey and send it for rename check IN db
  uuu::upll_strncpy(
      reinterpret_cast<key_flowlist_t *>
     (okey->get_key())->flowlist_name,
     reinterpret_cast<val_vtn_flowfilter_entry_t *> (ikey->get_cfg_val()->
     get_val())->flowlist_name,
     (kMaxLenFlowListName + 1));
  result_code = IsRenamed(ikey, dt_type, dmi, rename_flowlist);
  if (result_code != UPLL_RC_SUCCESS) {
     UPLL_LOG_DEBUG("IsRenamed returns error");
     return result_code;
  }
  if (rename_flowlist == 0) {
    delete okey;
    return UPLL_RC_SUCCESS;
  }
  if (ctrlr_dom)
  SET_USER_DATA_CTRLR(okey, ctrlr_dom->ctrlr);
  result_code = mgr->ReadConfigDB(okey, dt_type, UNC_OP_READ, dbop, dmi,
                                  RENAMETBL); /* ctrlr_name */
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetRenamedControllerKey:Read Configuration Error");
    return result_code;
  }
  val_rename_flowlist *rename_val =reinterpret_cast <val_rename_flowlist *>
  (GetVal(okey));
  if (!rename_val || (rename_val->valid[UPLL_IDX_IP_TYPE_FL] != UNC_VF_VALID))
    return UPLL_RC_ERR_GENERIC;
  uuu::upll_strncpy(
                    reinterpret_cast<val_vtn_flowfilter_entry_t*>
                    (ikey->get_cfg_val()->get_val())->flowlist_name,
                    rename_val->flowlist_newname,
                    (kMaxLenFlowListName + 1));
  SET_USER_DATA_FLAGS(ikey, rename_flowlist);
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtnFlowFilterEntryMoMgr::GetRenamedUncKey(
    ConfigKeyVal *ikey, upll_keytype_datatype_t dt_type, DalDmlIntf *dmi,
    uint8_t *ctrlr_id) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_ERR_GENERIC;
  ConfigKeyVal *unc_key = NULL;
  DbSubOp dbop = { kOpReadSingle, kOpMatchCtrlr, kOpInOutNone };

  key_vtn *rename_vtn_key  = reinterpret_cast <key_vtn*>
                                  (ConfigKeyVal::Malloc(sizeof(key_vtn)));

  key_vtn_flowfilter_entry_t *ctrlr_key = reinterpret_cast
      <key_vtn_flowfilter_entry_t *> (ikey->get_key());

  uuu::upll_strncpy(rename_vtn_key->vtn_name,
                    ctrlr_key->flowfilter_key.vtn_key.vtn_name,
                    (kMaxLenVtnName + 1));
  MoMgrImpl *mgr = static_cast<MoMgrImpl*>
    ((const_cast<MoManager*>
    (GetMoManager(UNC_KT_VTN))));
  result_code = mgr->GetChildConfigKey(unc_key, NULL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetRenamedUnckey:GetChildConfigKey returned error");
    free(rename_vtn_key);
    return result_code;
  }
  if (ctrlr_id == NULL) {
    UPLL_LOG_DEBUG("GetRenamedUncKey:Controller Name cannot be be null");
    free(rename_vtn_key);
    delete unc_key;
    return UPLL_RC_ERR_GENERIC;
  }
  SET_USER_DATA_CTRLR(unc_key, ctrlr_id);
  unc_key->AppendCfgVal(IpctSt::kIpcStValRenameVtn, rename_vtn_key);
  result_code = mgr->ReadConfigDB(unc_key, dt_type, UNC_OP_READ, dbop,
                                            dmi, RENAMETBL);
  if (result_code == UPLL_RC_SUCCESS) {
    key_vtn *vtn_key = reinterpret_cast<key_vtn *> (unc_key->get_key());
    uuu::upll_strncpy(ctrlr_key->flowfilter_key.vtn_key.vtn_name,
                      vtn_key->vtn_name,
                      (kMaxLenVtnName + 1));
  }
  unc_key = NULL;
  val_rename_flowlist *rename_fl =reinterpret_cast<val_rename_flowlist*>
                 (ConfigKeyVal::Malloc(sizeof(val_rename_flowlist)));

  val_vtn_flowfilter_entry_t *val_vtn_flofilter_entry =reinterpret_cast
      <val_vtn_flowfilter_entry_t *>(GetVal(ikey));
  uuu::upll_strncpy(rename_fl->flowlist_newname,
                    val_vtn_flofilter_entry->flowlist_name,
                    (kMaxLenFlowListName + 1));
  mgr = static_cast<MoMgrImpl*>
    ((const_cast<MoManager*>
      (GetMoManager(UNC_KT_FLOWLIST))));
  result_code = mgr->GetChildConfigKey(unc_key, NULL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetChildConfigKey Returned an error");
    free(rename_fl);
    delete unc_key;
    return result_code;
  }
  SET_USER_DATA_CTRLR(unc_key, ctrlr_id);
  unc_key->AppendCfgVal(IpctSt::kIpcStValRenameFlowlist, rename_fl);
  result_code = mgr->ReadConfigDB(unc_key, dt_type, UNC_OP_READ, dbop, dmi,
                                  RENAMETBL);
  if (result_code == UPLL_RC_SUCCESS) {
  key_flowlist_t *key_flowlist = reinterpret_cast
  <key_flowlist_t *>(unc_key->get_key());
    uuu::upll_strncpy(val_vtn_flofilter_entry->flowlist_name,
                      key_flowlist->flowlist_name,
                      (kMaxLenFlowListName + 1));
  }
  return result_code;
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
    delete tmp1;
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
    DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  DalResultCode db_result;
  unc_keytype_operation_t op[] = { UNC_OP_DELETE, UNC_OP_CREATE,
    UNC_OP_UPDATE };
  int nop = sizeof(op) / sizeof(op[0]);
  ConfigKeyVal *vtn_ffe_key = NULL, *req = NULL, *nreq = NULL;
  DalCursor *cfg1_cursor = NULL;
  uint8_t *ctrlr_id = NULL;

  map<string, int> ctrlr_result;
  CtrlrCommitStatusList::iterator ccsListItr;
  CtrlrCommitStatus *ccStatusPtr;

  if ((ctrlr_commit_status == NULL) || (dmi == NULL)) {
    UPLL_LOG_DEBUG(
        "TxCopy:ctrlr_commit_status is NULL");

    return UPLL_RC_ERR_GENERIC;
  }
  for (ccsListItr = ctrlr_commit_status->begin();
       ccsListItr != ctrlr_commit_status->end(); ++ccsListItr) {
    ccStatusPtr = *ccsListItr;
    // ctrlr_id = reinterpret_cast<uint8_t* >((const_cast<char*>
      //                                      (ccStatusPtr->ctrlr_id.c_str())));
    ctrlr_id = reinterpret_cast<uint8_t *>(&ccStatusPtr->ctrlr_id);
    ctrlr_result[ccStatusPtr->ctrlr_id] = ccStatusPtr->upll_ctrlr_result;
    if (ccStatusPtr->upll_ctrlr_result != UPLL_RC_SUCCESS) {
      for (ConfigKeyVal *ck_err = ccStatusPtr->err_ckv; ck_err != NULL; ck_err =
           ck_err->get_next_cfg_key_val()) {
        if (ck_err->get_key_type() != keytype) continue;
        result_code = GetRenamedUncKey(ck_err, UPLL_DT_CANDIDATE, dmi,
                                       ctrlr_id);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG(
              "TxCopy:GetRenamedUncKey is failed, resultcode= %d",
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
          UPLL_LOG_DEBUG("No diff found for op %d", op[i]);
          break;
        }
        result_code = UpdateMainTbl(req, op[i], UPLL_RC_SUCCESS,
                                    nreq, dmi);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("Updating Main table Error %d", result_code);
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
    MoMgrTables tbl = (op[i] == UNC_OP_UPDATE)?MAINTBL:CTRLRTBL;
    result_code = DiffConfigDB(UPLL_DT_CANDIDATE, UPLL_DT_RUNNING, op[i], req,
                               nreq, &cfg1_cursor, dmi, tbl);
    ConfigKeyVal *vtn_ffe_ctrlr_key = NULL;
    while (result_code == UPLL_RC_SUCCESS) {
      db_result = dmi->GetNextRecord(cfg1_cursor);
      result_code = DalToUpllResCode(db_result);
      if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
        result_code = UPLL_RC_SUCCESS;
        UPLL_LOG_DEBUG("!!! No diff found for op %d", op[i]);
        break;
      }
      if (op[i] == UNC_OP_UPDATE) {
        DbSubOp dbop = { kOpReadMultiple, kOpMatchNone,
          kOpInOutCtrlr|kOpInOutDomain };
        result_code = GetChildConfigKey(vtn_ffe_ctrlr_key, req);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("GetChildConfigKey is failed resultcode=%d",
                         result_code);
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
            return result_code;
          }
        }

        result_code = DupConfigKeyVal(vtn_ffe_key, req, tbl);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("DupConfigKeyVal is failed result_code = %d",
                         result_code);
          return result_code;
        }

        GET_USER_DATA_CTRLR(vtn_ffe_ctrlr_key, ctrlr_id);
        string controller(reinterpret_cast<char *> (ctrlr_id));
        for (ConfigKeyVal *tmp = vtn_ffe_ctrlr_key; tmp != NULL;
             tmp = tmp->get_next_cfg_key_val()) {
          result_code = UpdateConfigStatus(vtn_ffe_key, op[i],
                                           ctrlr_result[controller],
                                           nreq, dmi, tmp);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("UpdateConfigStatus failed, err %d", result_code);
            break;
          }

          void *vtnffe_ctrlval = GetVal(vtn_ffe_ctrlr_key);
          CompareValidValue(vtnffe_ctrlval, GetVal(nreq), false);
          result_code = UpdateConfigDB(vtn_ffe_ctrlr_key,
                                       UPLL_DT_RUNNING, op[i], dmi, CTRLRTBL);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("!!! Failed to Update the controller table, err %d",
                           result_code);
            return result_code;
          }
          result_code = UpdateConfigDB(vtn_ffe_key,
                                       UPLL_DT_RUNNING, op[i], dmi, MAINTBL);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("Update to Main table failed %d\n", result_code);
            return result_code;
          }
          EnqueCfgNotification(op[i], UPLL_DT_RUNNING,
                               vtn_ffe_key);
        }
      } else {
        if (op[i] == UNC_OP_CREATE) {
          DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutFlag };
          result_code = GetChildConfigKey(vtn_ffe_key, req);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("GetChildConfigKey is failed resultcode=%d",
                           result_code);
            return result_code;
          }
          result_code = ReadConfigDB(vtn_ffe_key, UPLL_DT_CANDIDATE,
                                     UNC_OP_READ, dbop, dmi, MAINTBL);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("Unable to read configuration from CandidateDb");
            return result_code;
          }

          result_code = DupConfigKeyVal(vtn_ffe_ctrlr_key, req, tbl);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("Failed to create duplicate ConfigKeyVal Err (%d)",
                         result_code);
            delete vtn_ffe_key;
            return result_code;
          }
          GET_USER_DATA_CTRLR(vtn_ffe_ctrlr_key, ctrlr_id);
          string controller(reinterpret_cast<char *> (ctrlr_id));
          result_code = UpdateConfigStatus(vtn_ffe_key, op[i],
                                           ctrlr_result[controller], NULL,
                                           dmi, vtn_ffe_ctrlr_key);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("Error in updating configstatus, resultcode=%d",
                           result_code);
            delete vtn_ffe_ctrlr_key;
            return result_code;
          }

        } else if (op[i] == UNC_OP_DELETE) {
          result_code = GetChildConfigKey(vtn_ffe_ctrlr_key, req);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("Error in getting the configkey, resultcode=%d",
                           result_code);
            return result_code;
          }
        }
        result_code = UpdateConfigDB(vtn_ffe_ctrlr_key,
                                     UPLL_DT_RUNNING, op[i], dmi, CTRLRTBL);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("DB Error while updating controller table. err:%d",
                         result_code);
          delete vtn_ffe_ctrlr_key;
          return result_code;
        }
        if (op[i] != UNC_OP_DELETE) {
          result_code = UpdateConfigDB(vtn_ffe_key, UPLL_DT_RUNNING,
                                       UNC_OP_UPDATE, dmi, MAINTBL);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("UpdateConfigDB in main tbl is failed -%d",
                           result_code);
            return result_code;
          }
        }
        EnqueCfgNotification(op[i], UPLL_DT_RUNNING,
                             vtn_ffe_key);
      }
      // delete vtn_flowfilter_entry_ctrlr_key;
      if (vtn_ffe_key) delete vtn_ffe_key;
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
      case UNC_CS_PARTAILLY_APPLIED:
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
            (cs_status == UNC_CS_APPLIED) ? UNC_CS_PARTAILLY_APPLIED : status;
        break;
    }
    vtn_ff_entry_val->cs_row_status = status;
    for (unsigned int loop = 0; loop <
      sizeof(vtn_ff_entry_val->valid) /sizeof(vtn_ff_entry_val->valid[0]);
      ++loop) {
      if (UNC_VF_NOT_SOPPORTED == vtn_ff_entry_val->valid[loop]) {
        vtn_ff_entry_val->cs_attr[loop] = UNC_CS_NOT_SUPPORTED;
        continue;
      }
      if (UNC_VF_NOT_SOPPORTED == ctrlr_val_ff_entry->valid[loop]) {
        ctrlr_val_ff_entry->cs_attr[loop] = UNC_CS_NOT_SUPPORTED;
        continue;
      }
      if ((UNC_VF_VALID == vtn_ff_entry_val->valid[loop]) ||
         (UNC_VF_VALID_NO_VALUE == vtn_ff_entry_val->valid[loop]))
        if (ctrlr_val_ff_entry->valid[loop] != UNC_VF_NOT_SOPPORTED) {
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
      if (ctrlr_val_ff_entry->valid[loop] != UNC_VF_NOT_SOPPORTED) {
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

upll_rc_t VtnFlowFilterEntryMoMgr::UpdateAuditConfigStatus(
                                         unc_keytype_configstatus_t cs_status,
                                         uuc::UpdateCtrlrPhase phase,
                                         ConfigKeyVal *&ckv_running) {
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  val_vtn_flowfilter_entry_t *val;
  val = (ckv_running != NULL)?
        reinterpret_cast<val_vtn_flowfilter_entry_t *>(GetVal(ckv_running)):
                                                                     NULL;
  if (NULL == val) {
    return UPLL_RC_ERR_GENERIC;
  }
  if (uuc::kUpllUcpCreate == phase )
     val->cs_row_status = cs_status;
  for (unsigned int loop = 0;
      loop < sizeof(val->valid)/sizeof(uint8_t); ++loop) {
    if (cs_status == UNC_CS_INVALID &&  UNC_VF_VALID == val->valid[loop])
         val->cs_attr[loop] = cs_status;
    else if (cs_status == UNC_CS_APPLIED)
         val->cs_attr[loop] = cs_status;
  }
  return result_code;
}

upll_rc_t VtnFlowFilterEntryMoMgr::SetConsolidatedStatus(ConfigKeyVal *ikey,
                                                         DalDmlIntf *dmi) {
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *ckv = NULL;
  DbSubOp dbop = {kOpReadMultiple, kOpMatchNone, kOpInOutNone};
  result_code = GetChildConfigKey(ckv, ikey);
  if (UPLL_RC_SUCCESS != result_code) {
    return result_code;
  }
  result_code = ReadConfigDB(ckv, UPLL_DT_RUNNING, UNC_OP_READ,
                             dbop, dmi, CTRLRTBL);
  if (UPLL_RC_SUCCESS != result_code) {
    delete ckv;
    return result_code;
  }

  std::vector<list<unc_keytype_configstatus_t> > vec_attr;
  std::list< unc_keytype_configstatus_t > list_cs_row;
  std::list< unc_keytype_configstatus_t > list_cs_attr;
  val_vtn_flowfilter_entry_t *val;
  for (unsigned int loop = 0; loop < sizeof(val->valid)/sizeof(val->valid[0]);
      ++loop) {
    std::list< unc_keytype_configstatus_t > list_attr;
    vec_attr.push_back(list_attr);
  }

  for (; ckv != NULL; ckv =  ckv->get_next_cfg_key_val()) {
      val = reinterpret_cast<val_vtn_flowfilter_entry_t*>(GetVal(ckv));
      list_cs_row.push_back((unc_keytype_configstatus_t)val->cs_row_status);
      for (unsigned int loop = 0;
            loop < sizeof(val->valid)/sizeof(val->valid[0]);
        ++loop) {
        vec_attr[loop].push_back(
                      (unc_keytype_configstatus_t)val->cs_attr[loop]);
    }
  }
  val_vtn_flowfilter_entry_t *val_temp =
  reinterpret_cast<val_vtn_flowfilter_entry_t *>(GetVal(ikey));
  val_temp->cs_row_status = GetConsolidatedCsStatus(list_cs_row);
  for (unsigned int loop = 0; loop < sizeof(val->valid)/sizeof(val->valid[0]);
      ++loop) {
    val_temp->cs_attr[loop] = GetConsolidatedCsStatus(vec_attr[loop]);
  }
  result_code = UpdateConfigDB(ikey, UPLL_DT_RUNNING, UNC_OP_UPDATE,
                               dmi, MAINTBL);
  if (UPLL_RC_SUCCESS != result_code) {
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
  key_vtn_flowfilter_entry_t *key_vtn_ffe =
      reinterpret_cast<key_vtn_flowfilter_entry_t *>(ikey->get_key());
  MoMgrImpl *mgr = reinterpret_cast<MoMgrImpl *>
      (const_cast<MoManager *>(GetMoManager(UNC_KT_VTN_FLOWFILTER)));

  result_code = mgr->GetChildConfigKey(okey, NULL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Key not supported - %d", result_code);
    return result_code;
  }

  key_vtn_flowfilter_t *vtn_ff_key =
      reinterpret_cast<key_vtn_flowfilter_t *>(okey->get_key());
  uuu::upll_strncpy(vtn_ff_key->vtn_key.vtn_name,
                    key_vtn_ffe->flowfilter_key.vtn_key.vtn_name,
                    (kMaxLenVtnName+1));
  vtn_ff_key->input_direction = key_vtn_ffe->flowfilter_key.input_direction;

  /* Checks the given vtn exists or not */
  result_code = mgr->UpdateConfigDB(okey, req->datatype, UNC_OP_READ,
                                    dmi, MAINTBL);
  if (UPLL_RC_ERR_INSTANCE_EXISTS != result_code) {
    UPLL_LOG_DEBUG(" Parent VTN flowfilter key does not exists");
    delete okey;
    okey = NULL;
    return UPLL_RC_ERR_PARENT_DOES_NOT_EXIST;
  } else {
    result_code = UPLL_RC_SUCCESS;
  }

  delete okey;
  okey = NULL;

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
      UPLL_LOG_DEBUG("Flowlist name in val_vtn_flowfilter_entry does not exists"
                     "in FLOWLIST table");
      delete okey;
      okey = NULL;
      return UPLL_RC_ERR_CFG_SEMANTIC;
    } else {
      result_code = UPLL_RC_SUCCESS;
    }

    delete okey;
    okey = NULL;
  }

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
                      kMaxLenVnodeName+1);

    uuu::upll_strncpy(key_nwm->vbr_key.vtn_key.vtn_name,
                      key_vtn_flowfilter_entry->flowfilter_key.vtn_key.vtn_name,
                      kMaxLenVtnName+1);

    /* Check nwm_name exists in table*/
    DbSubOp dbop = {kOpReadSingle, kOpMatchNone, kOpInOutNone};
    result_code = mgr->ReadConfigDB(okey, req->datatype, UNC_OP_READ,
                                    dbop, dmi, MAINTBL);

    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("NWM name in val_vtn_flowfilter_entry does not exists"
                     "in KT_VBR_NWMONITOR table");
      delete okey;
      okey = NULL;
      return UPLL_RC_ERR_CFG_SEMANTIC;
    } else {
      result_code = UPLL_RC_SUCCESS;
    }

    delete okey;
    okey = NULL;
  }  // nwm_name is valid
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

  DbSubOp dbop1 = { kOpReadExist, kOpMatchNone, kOpInOutNone};
  key_vtn_flowfilter_controller_t *key_vtn_flowfilter_ctrl =
      reinterpret_cast<key_vtn_flowfilter_controller_t*>(ikey->get_key());
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

  key_vtn_ffe_ctrl->flowfilter_key.input_direction = 0xFE;
  result_code = UpdateConfigDB(dup_ckmain, req->datatype,
                               UNC_OP_READ, dmi, &dbop1, CTRLRTBL);
  if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    UPLL_LOG_DEBUG("Requested Vtn is Not Configured in"
                   "flowfilterEntryCtrlr Table %d", result_code);
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

        ConfigVal *tmp1 = NULL;
        val_flowfilter_controller_t *l_val =reinterpret_cast
            <val_flowfilter_controller_t*>
            (ConfigKeyVal::Malloc(sizeof(val_flowfilter_controller_t)));
        val_flowfilter_controller_t *ival = reinterpret_cast
            <val_flowfilter_controller_t *>(GetVal(ikey));

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
#if 0
        result_code = GetRenamedControllerKey(l_key, req->datatype, dmi,
                                              &ctrlr_dom);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("GetRenamedControllerKey Failed %d", result_code);
          return result_code;
        }
#endif
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
          return UPLL_RC_ERR_GENERIC;
        }

        if (ipc_resp.header.result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_INFO("Request for Key %d failed in %s with error %d\n",
                        l_key->get_key_type(), ctrlr_dom.ctrlr,
                        ipc_resp.header.result_code);
          delete dup_ckmain;
          return ipc_resp.header.result_code;
        }
        ConfigKeyVal *okey = NULL;
        result_code = ReadControllerStateDetail(ikey,
                                                ipc_resp.ckv_data,
                                                req->datatype,
                                                req->operation,
                                                dmi, &ctrlr_dom, &okey);
        if (result_code!= UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("ReadControllerStateDetail Fail err code (%d)",
                         result_code);
          return result_code;
        } else {
          if (okey != NULL) {
            ikey->ResetWith(okey);
          }
        }
      }
      break;
    default:
      UPLL_LOG_DEBUG("Operation Not Allowed");
      result_code = UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT;
      break;
  }
  return result_code;
}

bool VtnFlowFilterEntryMoMgr::IsValidKey(void *key,
                                         uint64_t index) {
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

upll_rc_t VtnFlowFilterEntryMoMgr::TxUpdateProcess(ConfigKeyVal *ck_main,
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
      UPLL_LOG_DEBUG("%s Flowfilter entry read failed %d",
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
                           ipc_resp->header.config_id, op, UPLL_DT_CANDIDATE,
                           dup_ckmain, ctrlr_dom, ipc_resp);
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

upll_rc_t VtnFlowFilterEntryMoMgr::TxUpdateController(
    unc_key_type_t keytype, uint32_t session_id, uint32_t config_id,
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
  DalCursor *dal_cursor_handle;
  IpcResponse resp;
  memset(&resp, 0, sizeof(IpcResponse));

  unc_keytype_operation_t op = (phase == uuc::kUpllUcpCreate) ? UNC_OP_CREATE :
      ((phase == uuc::kUpllUcpUpdate) ? UNC_OP_UPDATE :
       ((phase == uuc::kUpllUcpDelete) ?UNC_OP_DELETE:UNC_OP_INVALID));
  switch (op) {
    case UNC_OP_CREATE:
    case UNC_OP_DELETE:
      result_code = DiffConfigDB(UPLL_DT_CANDIDATE, UPLL_DT_RUNNING, op, req,
                                 nreq, &dal_cursor_handle, dmi, CTRLRTBL);
      break;
    case UNC_OP_UPDATE:
      result_code = DiffConfigDB(UPLL_DT_CANDIDATE, UPLL_DT_RUNNING, op, req,
                                 nreq, &dal_cursor_handle, dmi, MAINTBL);
      break;
    default:
      UPLL_LOG_DEBUG("TxUpdateController Invalid operation \n");
      return UPLL_RC_ERR_GENERIC;
  }
  resp.header.clnt_sess_id = session_id;
  resp.header.config_id = config_id;
  while (result_code == UPLL_RC_SUCCESS) {
    dal_result = dmi->GetNextRecord(dal_cursor_handle);
    result_code = DalToUpllResCode(dal_result);
    if (result_code != UPLL_RC_SUCCESS)
      break;
    ck_main = NULL;
    if ((op == UNC_OP_CREATE) || (op == UNC_OP_DELETE)) {
      result_code = GetChildConfigKey(ck_main, req);
      if (result_code != UPLL_RC_SUCCESS) {
         UPLL_LOG_DEBUG("GetChildConfiKey Failed, err %d", result_code);
         return result_code;
      }
      // GET_USER_DATA_CTRLR_DOMAIN(ck_main, ctrlr_dom);
      GET_USER_DATA_CTRLR_DOMAIN(req, ctrlr_dom);
       /*
      result_code = DupConfigKeyVal(ck_main, req, MAINTBL);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("DupConfigKeyVal err (%d)", result_code);
        return result_code;
      }*/
      // GET_USER_DATA_CTRLR_DOMAIN(ck_main, ctrlr_dom);
      UPLL_LOG_DEBUG("ctrlr : %s; domain : %s", ctrlr_dom.ctrlr,
                     ctrlr_dom.domain);
      if (ctrlr_dom.ctrlr == NULL || NULL == ctrlr_dom.domain) {
        UPLL_LOG_DEBUG("Invalid controller/Domain");
        if (ck_main) delete ck_main;
        return UPLL_RC_ERR_GENERIC;
      }
      result_code = TxUpdateProcess(ck_main, &resp,
                                    op, dmi, &ctrlr_dom);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("TxUpdateProcess Returns error %d", result_code);
        *err_ckv = resp.ckv_data;
        if (ck_main) delete ck_main;
        break;
      }
      affected_ctrlr_set->insert((const char *)ctrlr_dom.ctrlr);
    } else if (op == UNC_OP_UPDATE) {
      ConfigKeyVal *ck_ctrlr = NULL;
      result_code = DupConfigKeyVal(ck_main, req, MAINTBL);
      if (result_code != UPLL_RC_SUCCESS)
        return result_code;
      /*
         result_code = ValidateCapability(&(ipc_req.header), ck_main);

         if (result_code != UPLL_RC_SUCCESS) {
         UPLL_LOG_DEBUG("TxUpdate  ValidateCapability Err (%d) (%d) (%d)",
         result_code, ipc_req.header.operation,
         ipc_req.header.datatype);
         return result_code;
         } */
      result_code = GetChildConfigKey(ck_ctrlr, ck_main);
      if (result_code != UPLL_RC_SUCCESS)
        return result_code;
      if (GetControllerDomainSpan(ck_ctrlr, UPLL_DT_CANDIDATE, dmi) ==
          UPLL_RC_ERR_NO_SUCH_INSTANCE) {
        delete ck_ctrlr;
        ck_ctrlr = NULL;
        continue;
      }
      void *main = GetVal(ck_main);
      void *val_nrec = (nreq) ? GetVal(nreq) : NULL;
      CompareValidValue(main, val_nrec, false);
      for (ConfigKeyVal *tmp = ck_ctrlr; tmp != NULL;
           tmp = tmp->get_next_cfg_key_val()) {
        GET_USER_DATA_CTRLR_DOMAIN(tmp, ctrlr_dom);
        if (ctrlr_dom.ctrlr == NULL || (ctrlr_dom.domain == NULL)) {
          UPLL_LOG_DEBUG("Invalid controller");
          delete ck_ctrlr;
          return UPLL_RC_ERR_GENERIC;
        }

        result_code = TxUpdateProcess(ck_main, &resp, op, dmi, &ctrlr_dom);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("TxUpdate Process returns with %d\n", result_code);
          *err_ckv = resp.ckv_data;
          if (ck_main) delete ck_main;
          break;
        }
        affected_ctrlr_set->insert(reinterpret_cast<const char *>
                                   (ctrlr_dom.ctrlr));
      }
      delete ck_ctrlr;
    }
    delete ck_main;
  }
  if (nreq)
    delete nreq;
  if (req)
    delete req;
  if (dal_cursor_handle)
    dmi->CloseCursor(dal_cursor_handle, true);
  result_code = (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE)?
      UPLL_RC_SUCCESS:result_code;
  return result_code;
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

    bool action_field_notconfigured = true;

    /** validate action */
    if (val_vtn_flowfilter_entry->valid[UPLL_IDX_ACTION_VFFE] == UNC_VF_VALID) {
      if (val_vtn_flowfilter_entry->action != UPLL_FLOWFILTER_ACT_PASS) {
        UPLL_LOG_DEBUG("Error Action has value(%d) other than PASS",
                       val_vtn_flowfilter_entry->action);
        return UPLL_RC_ERR_CFG_SYNTAX;
      }
      action_field_notconfigured = false;
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
        return UPLL_RC_ERR_NO_SUCH_INSTANCE;
      }

      /* check the action value from the DB data */
      val_vtn_flowfilter_entry_t *val_vtn_ffe =
          reinterpret_cast<val_vtn_flowfilter_entry_t *>(
              okey->get_cfg_val()->get_val());

      if ((val_vtn_ffe->valid[UPLL_IDX_ACTION_VFFE] == UNC_VF_VALID) &&
          (val_vtn_ffe->action == UPLL_FLOWFILTER_ACT_PASS)) {
        UPLL_LOG_TRACE("Action is configured in DB with value as PASS");
        action_field_notconfigured = false;
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
      if (action_field_notconfigured) {
        UPLL_LOG_DEBUG("Error DSCP configured when Action is not filled");
        return UPLL_RC_ERR_CFG_SYNTAX;
      }

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
      if (action_field_notconfigured) {
        UPLL_LOG_DEBUG("PRIORITY is configured when Action is not filled");
        return UPLL_RC_ERR_CFG_SYNTAX;
      }

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

  if (val_vtn_flowfilter_entry != NULL) {
    if ((val_vtn_flowfilter_entry->valid[UPLL_IDX_FLOWLIST_NAME_VFFE]
         == UNC_VF_VALID)
        || (val_vtn_flowfilter_entry->valid[UPLL_IDX_FLOWLIST_NAME_VFFE]
            == UNC_VF_VALID_NO_VALUE)) {
      if (attrs[unc::capa::vtn_flowfilter_entry::kCapFlowlistName] == 0) {
        val_vtn_flowfilter_entry->valid[UPLL_IDX_FLOWLIST_NAME_VFFE] =
            UNC_VF_NOT_SOPPORTED;

        UPLL_LOG_DEBUG("FlowlistName attr is not supported by ctrlr");
        return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
      }
    }

    if ((val_vtn_flowfilter_entry->valid[UPLL_IDX_ACTION_VFFE] == UNC_VF_VALID)
        || (val_vtn_flowfilter_entry->valid[UPLL_IDX_ACTION_VFFE]
            == UNC_VF_VALID_NO_VALUE)) {
      if (attrs[unc::capa::vtn_flowfilter_entry::kCapAction] == 0) {
        val_vtn_flowfilter_entry->valid[UPLL_IDX_ACTION_VFFE] =
            UNC_VF_NOT_SOPPORTED;

        UPLL_LOG_DEBUG("Action attr is not supported by ctrlr");
        return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
      }
    }
    if ((val_vtn_flowfilter_entry->valid[UPLL_IDX_NWN_NAME_VFFE] ==
         UNC_VF_VALID)
        || (val_vtn_flowfilter_entry->valid[UPLL_IDX_NWN_NAME_VFFE]
            == UNC_VF_VALID_NO_VALUE)) {
      if (attrs[unc::capa::vtn_flowfilter_entry::kCapNwnName] == 0) {
        val_vtn_flowfilter_entry->valid[UPLL_IDX_NWN_NAME_VFFE] =
            UNC_VF_NOT_SOPPORTED;

        UPLL_LOG_DEBUG("Nwm name attr is not supported by ctrlr");
        return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
      }
    }
    if ((val_vtn_flowfilter_entry->valid[UPLL_IDX_DSCP_VFFE] == UNC_VF_VALID)
        || (val_vtn_flowfilter_entry->valid[UPLL_IDX_DSCP_VFFE]
            == UNC_VF_VALID_NO_VALUE)) {
      if (attrs[unc::capa::vtn_flowfilter_entry::kCapDscp] == 0) {
        val_vtn_flowfilter_entry->valid[UPLL_IDX_DSCP_VFFE] =
            UNC_VF_NOT_SOPPORTED;

        UPLL_LOG_DEBUG("Dscp attr is not supported by ctrlr");
        return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
      }
    }

    if ((val_vtn_flowfilter_entry->valid[UPLL_IDX_PRIORITY_VFFE] ==
         UNC_VF_VALID)
        || (val_vtn_flowfilter_entry->valid[UPLL_IDX_PRIORITY_VFFE]
            == UNC_VF_VALID_NO_VALUE)) {
      if (attrs[unc::capa::vtn_flowfilter_entry::kCapPriority] == 0) {
        val_vtn_flowfilter_entry->valid[UPLL_IDX_PRIORITY_VFFE] =
            UNC_VF_NOT_SOPPORTED;

        UPLL_LOG_DEBUG("Priority attr is not supported by ctrlr");
        return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
      }
    }

  } else {
    UPLL_LOG_DEBUG("Error value struct is NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  return UPLL_RC_SUCCESS;
}

bool VtnFlowFilterEntryMoMgr::GetRenameKeyBindInfo(unc_key_type_t key_type,
                                                   BindInfo *&binfo,
                                                   int &nattr,
                                                   MoMgrTables tbl) {
  /* Main Table only update */
  if (UNC_KT_VTN_FLOWFILTER_ENTRY == key_type) {
    if (MAINTBL == tbl) {
      nattr = NUM_KEY_MAIN_TBL_;
      binfo = vtnflowfilterentrymaintbl_bind_info;
    } else if (CTRLRTBL == tbl) {
      nattr = NUM_KEY_CTRLR_TBL;
      binfo = vtnflowfilterentryctrlrtbl_bind_info;
    }
  }

  /* Check for Flowlist Rename*/
  if (UNC_KT_FLOWLIST == key_type) {
    if (MAINTBL == tbl) {
      nattr = NUM_KEY_RENAME_MAIN_TBL;
      binfo = vtn_flowlist_rename_bind_info;
    }
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

  if (UNC_KT_VTN_FLOWFILTER_ENTRY == ikey->get_key_type()) {
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
#if 0
    if (ikey->get_key_type() == table[MAINTBL]->get_key_type()) {
      if (!strlen(reinterpret_cast<char *> (key_rename->old_unc_vnode_name)))
        return UPLL_RC_ERR_GENERIC;
      strncpy(reinterpret_cast<char*>
              (key_vrt_if->flowfilter_key.if_key.vrt_key.vrouter_name),
              reinterpret_cast<char *> (key_rename->old_unc_vnode_name), 32);
    }
#endif
    okey = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_ENTRY, IpctSt::
                            kIpcStKeyVtnFlowfilterEntry, key_vtn, NULL);
  }
  if (UNC_KT_FLOWLIST == ikey->get_key_type()) {
    key_rename_vnode_info_t *key_rename =
        reinterpret_cast<key_rename_vnode_info_t *>(ikey->get_key());
    val_vtn_flowfilter_entry_t *val =
        reinterpret_cast <val_vtn_flowfilter_entry_t*>
        (ConfigKeyVal::Malloc(sizeof(val_vtn_flowfilter_entry_t)));
    uuu::upll_strncpy(val->flowlist_name,
                      key_rename->old_flowlist_name,
                      (kMaxLenFlowListName+1));

    val->valid[UPLL_IDX_FLOWLIST_PPE] = UNC_VF_VALID;

    ConfigVal *ckv = new ConfigVal(IpctSt::kIpcStValVtnFlowfilterEntry, val);
    okey = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVtnFlowfilterEntry, NULL, ckv);
  }

  if (!okey)
    return UPLL_RC_ERR_GENERIC;
  return result_code;
}

bool  VtnFlowFilterEntryMoMgr::CompareValidValue(void *&val1,
                                                 void *val2, bool audit) {
  UPLL_FUNC_TRACE;
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
    if (val_ff_entry1->action !=
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
    if (val_ff_entry1->dscp!=
        val_ff_entry2->dscp)
      val_ff_entry1->valid[UPLL_IDX_DSCP_VFFE] = UNC_VF_INVALID;
  }
  if (val_ff_entry1->valid[UPLL_IDX_PRIORITY_VFFE] == UNC_VF_VALID &&
      val_ff_entry2->valid[UPLL_IDX_PRIORITY_VFFE] == UNC_VF_VALID) {
    if (val_ff_entry1->priority !=
        val_ff_entry2->priority)
      val_ff_entry1->valid[UPLL_IDX_PRIORITY_VFFE] = UNC_VF_INVALID;
  }
  return false;
}

upll_rc_t VtnFlowFilterEntryMoMgr::ReadSiblingMo(IpcReqRespHeader *req,
                                                 ConfigKeyVal *ikey, bool begin,
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
  result_code = UpdateConfigDB(dup_ckmain, req->datatype,
                               UNC_OP_READ, dmi, &dbop1, CTRLRTBL);
  if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    UPLL_LOG_DEBUG("Requested Vtn is Not Configured in"
                          "flowfilterEntryCtrlr Table %d", result_code);
    return UPLL_RC_ERR_NOT_ALLOWED_AT_THIS_TIME;
  }

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
          delete dup_ckmain;
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
  ConfigKeyVal *tctrl_key = NULL, *okey = NULL, *tmp_key = NULL;
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
  if (val_ff_ctrl->valid[UPLL_IDX_SEQ_NUM_FFC] == UNC_VF_VALID) {
    key_vtn_ffe_ctrl->sequence_num = val_ff_ctrl->sequence_num;
  } else {
    key_vtn_ffe_ctrl->sequence_num = 0;
  }
  key_vtn_ffe_ctrl->flowfilter_key.input_direction = val_ff_ctrl->direction;
  // Reading The  Entry_Ctrl_Table
  DbSubOp dbop = {kOpReadMultiple, kOpMatchNone,
    kOpInOutCtrlr|kOpInOutDomain};
  result_code = ReadConfigDB(tctrl_key, UPLL_DT_STATE, UNC_OP_READ,
                             dbop, req->rep_count, dmi, CTRLRTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Unable to read vtn configuration from CTRL DB %d",
                   result_code);
    delete tctrl_key;
    return result_code;
  }

  tmp_key = tctrl_key;
  if (req->operation == UNC_OP_READ_SIBLING) {
    while (tmp_key !=NULL) {
      GET_USER_DATA_CTRLR_DOMAIN(tmp_key, tmp_ctrlr_dom);
      int ctrl_len =  strcmp((const char*)(ctrlr_dom.ctrlr),
                         (const char*)(tmp_ctrlr_dom.ctrlr));
      int dom_len =  strcmp((const char*)(ctrlr_dom.domain),
                        (const char*)(tmp_ctrlr_dom.domain));
      if ((ctrl_len < 0) || ((ctrl_len == 0) && (dom_len < 0))) {
        result_code = ConstructReadSiblingNormalResponse(tmp_key,
                                                         req->datatype,
                                                         dmi, &okey);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("ConstructReadSiblingNormalResponse failed %d",
                         result_code);
          return result_code;
        }
      }
      tmp_key = tmp_key->get_next_cfg_key_val();
    }
    if ((okey != NULL) && (result_code == UPLL_RC_SUCCESS)) {
      ikey->ResetWith(okey);
    }
  } else if (req->operation == UNC_OP_READ_SIBLING_BEGIN) {
    while (tmp_key !=NULL) {
      GET_USER_DATA_CTRLR_DOMAIN(tmp_key, ctrlr_dom);
      result_code = ConstructReadSiblingNormalResponse(tmp_key,
                                                       req->datatype,
                                                       dmi, &okey);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("ConstructReadSiblingNormalResponse failed %d",
                       result_code);
        return result_code;
      }
      tmp_key = tmp_key->get_next_cfg_key_val();
    }
    if ((okey != NULL) && (result_code == UPLL_RC_SUCCESS)) {
      ikey->ResetWith(okey);
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
  ConfigKeyVal *tctrl_key = NULL, *okey = NULL, *tmp_key = NULL, *l_key = NULL;
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
  uuu::upll_strncpy(l_key_vtn_ff_ctrl->vtn_key.vtn_name,
                    key_vtn_ff_ctrl->vtn_key.vtn_name,
                    (kMaxLenVtnName +1));
  uuu::upll_strncpy(l_key_vtn_ff_ctrl->controller_name,
                    key_vtn_ff_ctrl->controller_name,
                    (kMaxLenVtnName +1));
  uuu::upll_strncpy(l_key_vtn_ff_ctrl->domain_id,
                    key_vtn_ff_ctrl->domain_id,
                    (kMaxLenVtnName +1));
  l_key = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_CONTROLLER, IpctSt::
                           kIpcStKeyVtnFlowfilterController,
                           l_key_vtn_ff_ctrl, tmp1);
#if 0
  UPLL_LOG_DEBUG("GetRenamedControllerKey");
  result_code = GetRenamedControllerKey(l_key, req->datatype, dmi,
                                        &l_ctrlr_dom);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetRenamedControllerKey Failed %d", result_code);
    return result_code;
  }

#endif
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
                    key_vtn_ff_ctrl->vtn_key.vtn_name, (kMaxLenVtnName +1));
  key_vtn_ffe_ctrl->flowfilter_key.input_direction = val_ff_ctrl->direction;
  if (val_ff_ctrl->valid[UPLL_IDX_SEQ_NUM_FFC] == UNC_VF_VALID) {
    key_vtn_ffe_ctrl->sequence_num = val_ff_ctrl->sequence_num;
  } else {
    key_vtn_ffe_ctrl->sequence_num = 0;
  }
  // Reading The  Entry_Ctrl_Table
  DbSubOp dbop = {kOpReadMultiple, kOpMatchNone, kOpInOutCtrlr|kOpInOutDomain};
  result_code = ReadConfigDB(tctrl_key, UPLL_DT_STATE, UNC_OP_READ,
                             dbop, req->rep_count, dmi, CTRLRTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Unable to read vtn configuration from CTRL DB %d",
                   result_code);
    delete l_key;
    delete tctrl_key;
    return result_code;
  }

  // Extracting The Val from CKV tctrl_key
  tmp_key = tctrl_key;
  if (req->operation == UNC_OP_READ_SIBLING) {
    while (tmp_key != NULL) {
      GET_USER_DATA_CTRLR_DOMAIN(tmp_key, tmp_ctrlr_dom);
      int ctrl_len =  strcmp((const char*)(ctrlr_dom.ctrlr),
                             (const char*)(tmp_ctrlr_dom.ctrlr));
      int dom_len =  strcmp((const char*)(ctrlr_dom.domain),
                            (const char*)(tmp_ctrlr_dom.domain));
      if ((ctrl_len < 0) || ((ctrl_len == 0) && (dom_len < 0))) {
        uuu::upll_strncpy(l_key_vtn_ff_ctrl->controller_name,
                          tmp_ctrlr_dom.ctrlr, (kMaxLenCtrlrId +1));
        uuu::upll_strncpy(l_key_vtn_ff_ctrl->domain_id,
                          tmp_ctrlr_dom.domain, (kMaxLenDomainId +1));
        l_val->sequence_num = key_vtn_ffe_ctrl->sequence_num;
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
          return UPLL_RC_ERR_GENERIC;
        }

        if (ipc_resp.header.result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("Driver response for Key %d controller %s result %d",
                         l_key->get_key_type(), ctrlr_dom.ctrlr,
                         ipc_resp.header.result_code);
          return ipc_resp.header.result_code;
        }
        result_code = ConstructReadSiblingDetailResponse(
            tmp_key,
            ipc_resp.ckv_data,
            req->datatype,
            dmi, &okey);
        if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("ConstructReadDetailResponse error code (%d)",
                           result_code);
          return result_code;
        }
      } else {
        result_code = UPLL_RC_ERR_NO_SUCH_INSTANCE;
      }
      tmp_key = tmp_key->get_next_cfg_key_val();
    }
    if ((okey != NULL) && (result_code == UPLL_RC_SUCCESS)) {
      ikey->ResetWith(okey);
    }
  } else if (req->operation == UNC_OP_READ_SIBLING_BEGIN) {
    while (tmp_key != NULL) {
      GET_USER_DATA_CTRLR_DOMAIN(tmp_key, ctrlr_dom);
      uuu::upll_strncpy(l_key_vtn_ff_ctrl->controller_name,
                        ctrlr_dom.ctrlr, (kMaxLenCtrlrId +1));
      uuu::upll_strncpy(l_key_vtn_ff_ctrl->domain_id,
                        ctrlr_dom.domain, (kMaxLenDomainId +1));
      l_val->sequence_num = key_vtn_ffe_ctrl->sequence_num;
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
                  PFCDRIVER_SERVICE_NAME, PFCDRIVER_SVID_LOGICAL, &ipc_req,
                  true, &ipc_resp)) {
        UPLL_LOG_DEBUG("SendReqToDriver failed for Key %d controller %s",
                       l_key->get_key_type(),
                       reinterpret_cast<char *>(ctrlr_dom.ctrlr));
        return UPLL_RC_ERR_GENERIC;
      }

      if (ipc_resp.header.result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Driver response for Key %d controller %s result %d",
                       l_key->get_key_type(), ctrlr_dom.ctrlr,
                       ipc_resp.header.result_code);
        return ipc_resp.header.result_code;
      }

      result_code = ConstructReadSiblingDetailResponse(
          tmp_key,
          ipc_resp.ckv_data,
          req->datatype,
          dmi, &okey);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("ConstructReadDetailResponse error code (%d)",
                       result_code);
        return result_code;
      }
      tmp_key = tmp_key->get_next_cfg_key_val();
    }
    if ((okey != NULL) && (result_code == UPLL_RC_SUCCESS)) {
      ikey->ResetWith(okey); 
    }
  } else {
    result_code = UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT;
  }
  return result_code;
}

upll_rc_t  VtnFlowFilterEntryMoMgr::ConstructReadSiblingDetailResponse(
    ConfigKeyVal *ikey ,
    ConfigKeyVal *drv_resp_ckv,
    upll_keytype_datatype_t dt_type,
    DalDmlIntf *dmi,
    ConfigKeyVal **okey) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *tmp_okey = NULL;
  ConfigVal *drv_resp_val = NULL;
  controller_domain ctrlr_dom;
  drv_resp_val =  drv_resp_ckv->get_cfg_val();
  // Extracting The Key of KT_VTN_FF_Ctrl

  key_vtn_flowfilter_entry_t *key_vtn_ffe =
      reinterpret_cast <key_vtn_flowfilter_entry_t*>
      (ikey->get_key());
  memset(&ctrlr_dom, 0, sizeof(controller_domain));
  GET_USER_DATA_CTRLR_DOMAIN(ikey, ctrlr_dom);

  key_vtn_flowfilter_controller_t *tmp_ff_ctrl =
      reinterpret_cast <key_vtn_flowfilter_controller_t*>
      (ConfigKeyVal::Malloc(sizeof(key_vtn_flowfilter_controller_t)));
  uuu::upll_strncpy(tmp_ff_ctrl->vtn_key.vtn_name,
                    key_vtn_ffe->flowfilter_key.vtn_key.vtn_name,
                    (kMaxLenVtnName +1));
  uuu::upll_strncpy(tmp_ff_ctrl->controller_name, ctrlr_dom.ctrlr,
                    (kMaxLenCtrlrId +1));
  uuu::upll_strncpy(tmp_ff_ctrl->domain_id, ctrlr_dom.domain,
                    (kMaxLenDomainId +1));
  tmp_okey = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_CONTROLLER,
                              IpctSt::kIpcStKeyVtnFlowfilterController,
                              tmp_ff_ctrl, NULL);

  val_flowfilter_controller_t* tmp_val_ff_ctrl =
      reinterpret_cast <val_flowfilter_controller_t*>
      (ConfigKeyVal::Malloc(sizeof(val_flowfilter_controller_t)));

  tmp_val_ff_ctrl->valid[UPLL_IDX_DIRECTION_FFC] = UNC_VF_VALID;
  tmp_val_ff_ctrl->direction = key_vtn_ffe->flowfilter_key.input_direction;
  tmp_val_ff_ctrl->valid[UPLL_IDX_SEQ_NUM_FFC] = UNC_VF_VALID;
  tmp_val_ff_ctrl->sequence_num = key_vtn_ffe->sequence_num;

  tmp_okey->AppendCfgVal(IpctSt::kIpcStValFlowfilterController,
                         tmp_val_ff_ctrl);

  while (drv_resp_val !=NULL) {
    if (IpctSt::kIpcStValVtnFlowfilterControllerSt !=
        drv_resp_val->get_st_num()) {
      UPLL_LOG_DEBUG("Incorrect structure received from driver, struct num %d",
                     drv_resp_val->get_st_num());
      return  UPLL_RC_ERR_GENERIC;
    }
    // Now Cheking  The 1st Structure of Driver Rersponse
    //  is val_vtn_flowfilter_controller_st or Not
    val_vtn_flowfilter_controller_st_t *tmp_vtn_ffe_st =
        reinterpret_cast <val_vtn_flowfilter_controller_st_t *>
        (drv_resp_val->get_val());
    // Check Seq No is Valid Or not
    if ((tmp_vtn_ffe_st)->valid[UPLL_IDX_SEQ_NUM_VFFCS] == UNC_VF_VALID) {
      /******* StartReading From The Vtn_FlowFilter_Entry_CtrlTable*******/
      // Copying The seqno, i/p dir, Vtn_Name to The Above Key of CKV tctrl_key
      ConfigKeyVal *tctrl_key = NULL;
      // Allocating The Key of KT_VTN_FF_Entry
      key_vtn_flowfilter_entry_t *key_vtn_ffe_ctrl =
          reinterpret_cast <key_vtn_flowfilter_entry_t*>
          (ConfigKeyVal::Malloc(sizeof(key_vtn_flowfilter_entry_t)));
      tctrl_key = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_ENTRY,
                                   IpctSt::kIpcStKeyVtnFlowfilterEntry,
                                   key_vtn_ffe_ctrl, NULL);

      key_vtn_ffe_ctrl->sequence_num = tmp_vtn_ffe_st->sequence_num;
      uuu::upll_strncpy(
          key_vtn_ffe_ctrl->flowfilter_key.vtn_key.vtn_name,
          key_vtn_ffe->flowfilter_key.vtn_key.vtn_name,
          (kMaxLenVtnName +1));
      key_vtn_ffe_ctrl->flowfilter_key.input_direction =
          key_vtn_ffe->flowfilter_key.input_direction;
      SET_USER_DATA_CTRLR(tctrl_key, ctrlr_dom.ctrlr);
      SET_USER_DATA_DOMAIN(tctrl_key, ctrlr_dom.domain);
      // Reading The  Entry_Ctrl_Table

      DbSubOp dbop1 = { kOpReadSingle, kOpMatchCtrlr|kOpMatchDomain,
        kOpInOutNone };
      result_code =  ReadConfigDB(tctrl_key, dt_type, UNC_OP_READ,
                                  dbop1, dmi, CTRLRTBL);
      // Extracting The Val from CKV tctrl_key
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Exiting :ReadConfigDB  Ctrltable Failed");
        delete tmp_okey;
        delete tctrl_key;
        return result_code;
      }

      val_vtn_flowfilter_entry_ctrlr_t* val_vtn_ffe_ctrlr =
          reinterpret_cast<val_vtn_flowfilter_entry_ctrlr_t*>
          (GetVal(tctrl_key));
      if (val_vtn_ffe_ctrlr == NULL) {
        UPLL_LOG_DEBUG("Exiting val_vtn_flowfilter_entry_ctrlr is Null \n");
        return result_code;
      }

      val_vtn_flowfilter_entry_t *op_val_vtn_ffe =
          reinterpret_cast<val_vtn_flowfilter_entry_t *>
          (ConfigKeyVal::Malloc(sizeof(val_vtn_flowfilter_entry_t)));

      result_code = GetCtrlFlowFilterEntry(key_vtn_ffe_ctrl,
                                           val_vtn_ffe_ctrlr,
                                           dt_type,
                                           dmi,
                                           op_val_vtn_ffe);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("GetCtrlFlowFilterEntry error code (%d)", result_code);
        return result_code;
      }

      /*Reading From The Vtn_FlowFilter_Entry_CtrlTable Finished */
      //  CKV tkey Consist The key_vtn_flowfilter_entry And
      //  Will Be used To Read The VTN_FF_ENTRY_TBL
      /*Reading From VtnFlowfilterEntryTbl Start*/
      // Appending The Ctrl St
      val_vtn_flowfilter_controller_st_t *l_val_ff_st =reinterpret_cast
          <val_vtn_flowfilter_controller_st_t* >
          (ConfigKeyVal::Malloc(sizeof(val_vtn_flowfilter_controller_st_t)));
      memcpy(l_val_ff_st, tmp_vtn_ffe_st,
             sizeof(val_vtn_flowfilter_controller_st_t));
      tmp_okey->AppendCfgVal(IpctSt::kIpcStValVtnFlowfilterControllerSt,
                             l_val_ff_st);
      tmp_okey->AppendCfgVal(IpctSt::kIpcStValVtnFlowfilterEntry,
                             op_val_vtn_ffe);

      if ((drv_resp_val = drv_resp_val->get_next_cfg_val()) == NULL) {
        UPLL_LOG_DEBUG("No More entries in driver response\n");
        break;
      }
      // Appending The FlowlistEntry St
      if (IpctSt::kIpcStValFlowlistEntrySt != drv_resp_val->get_st_num()) {
        UPLL_LOG_DEBUG("No flowflist entries returned by driver");
        continue;
      }
      while (IpctSt::kIpcStValFlowlistEntrySt == (
              drv_resp_val)->get_st_num()) {
        val_flowlist_entry_st_t* tmp_val_fl_st =
            reinterpret_cast<val_flowlist_entry_st_t*>
            (ConfigKeyVal::Malloc(sizeof(val_flowlist_entry_st_t)));
        memcpy(tmp_val_fl_st,
               reinterpret_cast<val_flowlist_entry_st_t*>
               (drv_resp_val->get_val()),
               sizeof(val_flowlist_entry_st_t));
        tmp_okey->AppendCfgVal(IpctSt::kIpcStValFlowlistEntrySt,
                               tmp_val_fl_st);
        drv_resp_val = drv_resp_val->get_next_cfg_val();
        if (!drv_resp_val) {
          break;
        }
      }
    }
  }
  if (*okey == NULL) {
    *okey = tmp_okey;
  } else {
    (*okey)->AppendCfgKeyVal(tmp_okey);
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t  VtnFlowFilterEntryMoMgr::ReadControllerStateDetail(
    ConfigKeyVal *ikey ,
    ConfigKeyVal *drv_resp_ckv,
    upll_keytype_datatype_t dt_type,
    unc_keytype_operation_t op,
    DalDmlIntf *dmi,
    controller_domain *ctrlr_dom,
    ConfigKeyVal **okey) {

  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *tmp_okey = NULL;
  key_vtn_flowfilter_controller_t *key_vtn_ffe =
      reinterpret_cast <key_vtn_flowfilter_controller_t*>
      (ikey->get_key());
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
  tmp_okey = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_CONTROLLER,
                              IpctSt::kIpcStKeyVtnFlowfilterController,
                              tmp_ff_ctrl, NULL);

  val_flowfilter_controller_t *val_ff_ctrl =
      reinterpret_cast<val_flowfilter_controller_t*>
      (ConfigKeyVal::Malloc(sizeof(val_flowfilter_controller_t)));
  val_flowfilter_controller_t *tmp_val_ff_ctrl =
      reinterpret_cast<val_flowfilter_controller_t*>(GetVal(ikey));
  if (!tmp_val_ff_ctrl) {
    UPLL_LOG_DEBUG(" Invalid value read from DB");
    free(val_ff_ctrl);
    return UPLL_RC_ERR_GENERIC;
  }
  memcpy(val_ff_ctrl, tmp_val_ff_ctrl, sizeof(val_flowfilter_controller_t));
  tmp_okey->AppendCfgVal(IpctSt::kIpcStValFlowfilter, val_ff_ctrl);


  ConfigVal *drv_resp_val = drv_resp_ckv->get_cfg_val();
  // Extracting The Key of KT_VTN_FF_Ctrl
  key_vtn_flowfilter_controller_t *key_vtn_ff_ctrl =
      reinterpret_cast<key_vtn_flowfilter_controller_t*> (ikey->get_key());

  while (drv_resp_val != NULL) {
    // Now Cheking  The 1st Structure of Driver Rersponse
    //  is val_vtn_flowfilter_controller_st or Not
    if (IpctSt::kIpcStValVtnFlowfilterControllerSt !=
        drv_resp_val->get_st_num()) {
      return  UPLL_RC_ERR_GENERIC;
    }
    val_vtn_flowfilter_controller_st_t * val_vtn_ff_ctrl_st =
        reinterpret_cast <val_vtn_flowfilter_controller_st_t *>
        (drv_resp_val->get_val());
    // Check Seq No is Valid Or not
    if ((val_vtn_ff_ctrl_st)->valid[UPLL_IDX_SEQ_NUM_VFFCS] == UNC_VF_VALID) {
      /******* StartReading From The Vtn_FlowFilter_Entry_CtrlTable*******/
      // Copying The seqno, i/p dir, Vtn_Name to The Above Key of CKV tctrl_key
      // Allocating The Key of KT_VTN_FF_Entry
      key_vtn_flowfilter_entry_t *key_vtn_ffe_ctrl =
          reinterpret_cast <key_vtn_flowfilter_entry_t*>
          (ConfigKeyVal::Malloc(sizeof(key_vtn_flowfilter_entry_t)));

      ConfigKeyVal *tctrl_key = new ConfigKeyVal(
          UNC_KT_VTN_FLOWFILTER_ENTRY,
          IpctSt::kIpcStKeyVtnFlowfilterEntry,
          key_vtn_ffe_ctrl, NULL);

      key_vtn_ffe_ctrl->sequence_num = val_vtn_ff_ctrl_st->sequence_num;
      uuu::upll_strncpy(
          key_vtn_ffe_ctrl->flowfilter_key.vtn_key.vtn_name,
          key_vtn_ff_ctrl->vtn_key.vtn_name,
          (kMaxLenVtnName +1));
      key_vtn_ffe_ctrl->flowfilter_key.input_direction =
          val_vtn_ff_ctrl_st->direction;

      SET_USER_DATA_CTRLR(tctrl_key, ctrlr_dom->ctrlr);
      SET_USER_DATA_DOMAIN(tctrl_key, ctrlr_dom->domain);
      // Reading The  Entry_Ctrl_Table

      DbSubOp dbop1 = { kOpReadSingle, kOpMatchCtrlr|kOpMatchDomain,
        kOpInOutNone };
      result_code =  ReadConfigDB(tctrl_key, dt_type, UNC_OP_READ,
                                  dbop1, dmi, CTRLRTBL);
      // Extracting The Val from CKV tctrl_key
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Exiting :ReadConfigDB  Ctrltable Failed");
        delete tmp_okey;
        delete tctrl_key;
        return result_code;
      }

      val_vtn_flowfilter_entry_ctrlr_t *val_vtn_ffe_ctrlr =
          reinterpret_cast<val_vtn_flowfilter_entry_ctrlr_t*>
          (GetVal(tctrl_key));
      if (val_vtn_ffe_ctrlr == NULL) {
        UPLL_LOG_DEBUG("Exiting val_vtn_ffe_ctrlr is Null \n");
        return result_code;
      }
      /*Reading From The Vtn_FlowFilter_Entry_CtrlTable Finished */
      //  CKV tkey Consist The key_vtn_flowfilter_entry And
      //  Will Be used To Read The VTN_FF_ENTRY_TBL
      /*Reading From VtnFlowfilterEntryTbl Start*/

      key_vtn_flowfilter_entry_t *key_vtn_ffe =
          reinterpret_cast<key_vtn_flowfilter_entry_t *>
          (ConfigKeyVal::Malloc(sizeof(key_vtn_flowfilter_entry_t)));

      // Now Matching Seq NO, Dir, VTNNAME To KEY of CKV tkey
      uuu::upll_strncpy(
          key_vtn_ffe->flowfilter_key.vtn_key.vtn_name,
          key_vtn_ff_ctrl->vtn_key.vtn_name, (kMaxLenVtnName +1));
      key_vtn_ffe->flowfilter_key.input_direction =
          val_vtn_ff_ctrl_st->direction;
      key_vtn_ffe->sequence_num =
          val_vtn_ff_ctrl_st->sequence_num;
      val_vtn_flowfilter_entry_t *op_val_vtn_ffe =
          reinterpret_cast<val_vtn_flowfilter_entry_t *>
          (ConfigKeyVal::Malloc(sizeof(val_vtn_flowfilter_entry_t)));

      result_code = GetCtrlFlowFilterEntry(key_vtn_ffe,
                                           val_vtn_ffe_ctrlr,
                                           dt_type,
                                           dmi,
                                           op_val_vtn_ffe);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("GetCtrlFlowFilterEntry error code (%d)", result_code);
        return result_code;
      }

      // Appending The Ctrl St
      tmp_okey->AppendCfgVal(IpctSt::kIpcStValVtnFlowfilterControllerSt,
                         val_vtn_ff_ctrl_st);
      // Appending The VAl_FF_Entry
      tmp_okey->AppendCfgVal(IpctSt::kIpcStValVtnFlowfilterEntry,
                         op_val_vtn_ffe);

      if ((drv_resp_val = drv_resp_val->get_next_cfg_val()) == NULL) {
        UPLL_LOG_DEBUG("Next Vlaue structure is null\n");
        break;
      }

      if (IpctSt::kIpcStValFlowlistEntrySt != (drv_resp_val)->get_st_num()) {
        UPLL_LOG_DEBUG("No flowflist entries returned by driver");
        continue;
      }
      // Appending The FlowlistEntry st
      while (IpctSt::kIpcStValFlowlistEntrySt == drv_resp_val->get_st_num()) {
        val_flowlist_entry_st_t* tmp_val_fl_st =
            reinterpret_cast<val_flowlist_entry_st_t*>
            (ConfigKeyVal::Malloc(sizeof(val_flowlist_entry_st_t)));
        memcpy(tmp_val_fl_st,
               reinterpret_cast<val_flowlist_entry_st_t*>
               (drv_resp_val->get_val()),
               sizeof(val_flowlist_entry_st_t));
        tmp_okey->AppendCfgVal(IpctSt::kIpcStValFlowlistEntrySt,
                           tmp_val_fl_st);
        drv_resp_val = drv_resp_val->get_next_cfg_val();
        if (!drv_resp_val) {
          break;
        }
      }
    }
  }
  if (*okey == NULL) {
    *okey = tmp_okey;
  } else {
    (*okey)->AppendCfgKeyVal(tmp_okey);
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t  VtnFlowFilterEntryMoMgr::ConstructReadSiblingNormalResponse(
    ConfigKeyVal *ikey,
    upll_keytype_datatype_t dt_type,
    DalDmlIntf *dmi,
    ConfigKeyVal **okey) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *tmp_okey = NULL;
  controller_domain ctrlr_dom;

  key_vtn_flowfilter_entry_t *key_vtn_ffe =
      reinterpret_cast <key_vtn_flowfilter_entry_t*>
      (ikey->get_key());
  memset(&ctrlr_dom, 0, sizeof(controller_domain));
  GET_USER_DATA_CTRLR_DOMAIN(ikey, ctrlr_dom);

  key_vtn_flowfilter_controller_t *tmp_ff_ctrl =
      reinterpret_cast <key_vtn_flowfilter_controller_t*>
      (ConfigKeyVal::Malloc(sizeof(key_vtn_flowfilter_controller_t)));
  uuu::upll_strncpy(tmp_ff_ctrl->vtn_key.vtn_name,
                    key_vtn_ffe->flowfilter_key.vtn_key.vtn_name,
                    (kMaxLenVtnName +1));
  uuu::upll_strncpy(tmp_ff_ctrl->controller_name, ctrlr_dom.ctrlr,
                    (kMaxLenCtrlrId +1));
  uuu::upll_strncpy(tmp_ff_ctrl->domain_id, ctrlr_dom.domain,
                    (kMaxLenDomainId +1));
  tmp_okey = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_CONTROLLER,
                              IpctSt::kIpcStKeyVtnFlowfilterController,
                              tmp_ff_ctrl, NULL);

  ConfigKeyVal *tctrl_key = NULL;
  // Allocating The Key of KT_VTN_FF_Entry
  key_vtn_flowfilter_entry_t *key_vtn_ffe_ctrl =
      reinterpret_cast <key_vtn_flowfilter_entry_t*>
      (ConfigKeyVal::Malloc(sizeof(key_vtn_flowfilter_entry_t)));

  memcpy(key_vtn_ffe_ctrl, key_vtn_ffe, sizeof(key_vtn_flowfilter_entry_t));

  // Allocating CKV tctrl_key
  tctrl_key = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_ENTRY,
                               IpctSt::kIpcStKeyVtnFlowfilterEntry,
                               key_vtn_ffe_ctrl, NULL);

  // Reading The  Entry_Ctrl_Table
  SET_USER_DATA_CTRLR(tctrl_key, ctrlr_dom.ctrlr);
  SET_USER_DATA_DOMAIN(tctrl_key, ctrlr_dom.domain);

  DbSubOp dbop1 = { kOpReadMultiple, kOpMatchCtrlr|kOpMatchDomain,
    kOpInOutNone };
  result_code =  ReadConfigDB(tctrl_key, dt_type, UNC_OP_READ,
                              dbop1, dmi, CTRLRTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("ReadConfigDB is Not Succes");
  }
  // Extracting The Val from CKV tctrl_key
  ConfigKeyVal *tmp_key = tctrl_key;
  while (tmp_key != NULL) {
    val_flowfilter_controller_t* l_val_ff_ctrl =
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

    tmp_okey->AppendCfgVal(IpctSt::kIpcStValFlowfilterController,
                           l_val_ff_ctrl);

    val_vtn_flowfilter_entry_t *op_val_vtn_ffe =
        reinterpret_cast<val_vtn_flowfilter_entry_t *>
        (ConfigKeyVal::Malloc(sizeof(val_vtn_flowfilter_entry_t)));

    result_code = GetCtrlFlowFilterEntry(l_key_vtn_ffe,
                                         val_vtn_ffe_ctrlr,
                                         dt_type,
                                         dmi,
                                         op_val_vtn_ffe);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("GetCtrlFlowFilterEntry error code (%d)", result_code);
      free(op_val_vtn_ffe);
      delete tmp_okey;
      delete tctrl_key;
      return result_code;
    }
    tmp_okey->AppendCfgVal(IpctSt::kIpcStValVtnFlowfilterEntry, op_val_vtn_ffe);
    tmp_key = tmp_key->get_next_cfg_key_val();
  }

  if (*okey == NULL) {
    *okey = tmp_okey;
  } else {
    (*okey)->AppendCfgKeyVal(tmp_okey);
  }
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
      UPLL_LOG_DEBUG(" Vtn name syntax validation failed :"
                     "Err Code - %d",
                     rt_code);
      return rt_code;
    }
  } else if (UNC_KT_VTN_FLOWFILTER_CONTROLLER == key->get_key_type()) {
    rt_code  =  ValidateMessageForVtnFlowFilterController(req, key);

    if (UPLL_RC_SUCCESS != rt_code) {
      UPLL_LOG_DEBUG(" Vtn name syntax validation failed :"
                     "Err Code - %d",
                     rt_code);
      return rt_code;
    }
  } else {
    UPLL_LOG_DEBUG(" Invalid keytype(%d)", key->get_key_type());
    return UPLL_RC_ERR_BAD_REQUEST;
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
  if((req->option1 != UNC_OPT1_NORMAL) 
              &&(req->operation == UNC_OP_READ_SIBLING_COUNT)) {
     UPLL_LOG_DEBUG(" Error: option1 is not NORMAL for ReadSiblingCount");
     return UPLL_RC_ERR_INVALID_OPTION1;
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
    rt_code = ValidateDefaultStr(reinterpret_cast<char*>(
      key_vtn_flowfilter_controller->domain_id),
        kMinLenDomainId, kMaxLenDomainId);

    if (rt_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("DomainId syntax validation failed: Err code-%d",
                    rt_code);
      return rt_code;
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
      /*TODO(Author): confirm the mandatory param list*/
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

  val_flowfilter_controller_t *val_flowfilter_controller =
      static_cast<val_flowfilter_controller_t *>(
         key->get_cfg_val()->get_val());

  if (NULL == val_flowfilter_controller) {
    UPLL_LOG_DEBUG("KT_VTN_FLOWFILTER_CONTROLLER val structure is null");
    return UPLL_RC_ERR_BAD_REQUEST;
  }



  return ValidateVtnFlowfilterControllerValue(
                val_flowfilter_controller, req->operation);
}

upll_rc_t VtnFlowFilterEntryMoMgr::ValidateVtnFlowfilterControllerValue(
    val_flowfilter_controller_t *val_flowfilter_controller,
    uint32_t operation) {
  UPLL_FUNC_TRACE;

  /** Validate value structure*/
  if (val_flowfilter_controller != NULL) {
    /** check  direction is filled */
    if (val_flowfilter_controller->valid[UPLL_IDX_DIRECTION_FFC]
        == UNC_VF_VALID) {
      UPLL_LOG_TRACE("direction field is filled");

      /** validate direction range */
      if (!ValidateNumericRange(val_flowfilter_controller->direction,
                                (uint8_t) UPLL_FLOWFILTER_DIR_IN,
                                (uint8_t) UPLL_FLOWFILTER_DIR_OUT, true,
                                true)) {
        UPLL_LOG_DEBUG(" direction syntax validation failed ");
        return UPLL_RC_ERR_CFG_SYNTAX;
      }
    } else if (((operation == UNC_OP_UPDATE) || (operation == UNC_OP_CREATE))

        && (val_flowfilter_controller->valid[UPLL_IDX_DIRECTION_FFC]
            == UNC_VF_VALID_NO_VALUE)) {
      UPLL_LOG_TRACE("Reset direction");
      val_flowfilter_controller->direction = 0;
    }

    /** check sequence number is configured */
    if (val_flowfilter_controller->valid[UPLL_IDX_SEQ_NUM_FFC]
        == UNC_VF_VALID) {
      UPLL_LOG_TRACE("seq_num field is filled");

      if (!ValidateNumericRange(val_flowfilter_controller->sequence_num,
                                kMinFlowFilterSeqNum, kMaxFlowFilterSeqNum,
                                true, true)) {
        UPLL_LOG_DEBUG(" Sequence Number syntax validation failed ");
        return UPLL_RC_ERR_CFG_SYNTAX;
      }
    } else if (((operation == UNC_OP_UPDATE) || (operation == UNC_OP_CREATE))
        && (val_flowfilter_controller->valid[UPLL_IDX_SEQ_NUM_FFC]
            == UNC_VF_VALID_NO_VALUE)) {
      UPLL_LOG_TRACE("Reset seq_num");
      val_flowfilter_controller->sequence_num = 0;
    }
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtnFlowFilterEntryMoMgr::ValidateCapability(IpcReqRespHeader *req,
                                   ConfigKeyVal *ikey,
                                   const char* ctrlr_name) {
  UPLL_FUNC_TRACE;

  // TODO(Author) added to bypass capability check
  return UPLL_RC_SUCCESS;
  // endTODO
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

  upll_rc_t result_code = UPLL_RC_ERR_GENERIC;

  if (!ctrlr_name)
    ctrlr_name = static_cast<char *>(ikey->get_user_data());

  uint32_t dt_type = req->datatype;
  uint32_t operation = req->operation;
  uint32_t option1 = req->option1;
  uint32_t option2 = req->option2;

  UPLL_LOG_TRACE("dt_type   : (%d)"
                "operation : (%d)"
                "option1   : (%d)"
                "option2   : (%d)",
                dt_type, operation, option1, option2);

  bool ret_code = false;
  uint32_t instance_count = 0;
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
                   "for opearion(%d)",
                   ikey->get_key_type(), ctrlr_name, operation);
    return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_KT;
  }

  val_vtn_flowfilter_entry_t *val_vtn_flowfilter_entry = NULL;

  if (ikey->get_cfg_val() && (ikey->get_cfg_val()->get_st_num()
      == IpctSt::kIpcStValVtnFlowfilterEntry)) {
    val_vtn_flowfilter_entry =
      reinterpret_cast<val_vtn_flowfilter_entry_t *>(
        ikey->get_cfg_val()->get_val());
  }

  if ((operation == UNC_OP_CREATE) || (operation == UNC_OP_UPDATE)) {
    if (dt_type == UPLL_DT_CANDIDATE) {
      if (val_vtn_flowfilter_entry) {
        if (max_attrs > 0) {
          return ValVtnFlowfilterEntryAttributeSupportCheck(
             val_vtn_flowfilter_entry, attrs);
        } else {
          UPLL_LOG_DEBUG("Attribute list is empty for operation %d", operation);
        }
      } else {
        UPLL_LOG_TRACE("Error value struct is mandatory for CREATE/UPDATE");
        return result_code;
      }
    } else {
      UPLL_LOG_DEBUG("Unsupported datatype for CREATE/UPDATE");
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  } else if (READ_SUPPORTED_OPERATION) {
    if (READ_SUPPORTED_DATATYPE) {
      if ((option1 != UNC_OPT1_NORMAL)
          && (option1 == UNC_OPT1_DETAIL
            && operation == UNC_OP_READ_SIBLING_COUNT)) {
        UPLL_LOG_DEBUG(" Error: option1 is not NORMAL");
        return UPLL_RC_ERR_INVALID_OPTION1;
      }
      if (option2 == UNC_OPT2_NONE) {
        UPLL_LOG_DEBUG(" Error: option2 is not NONE");
        return UPLL_RC_ERR_INVALID_OPTION2;
      }
      if (val_vtn_flowfilter_entry) {
        if (max_attrs > 0) {
          return ValVtnFlowfilterEntryAttributeSupportCheck(
             val_vtn_flowfilter_entry, attrs);
        } else {
          UPLL_LOG_DEBUG("Attribute list is empty for operation %d", operation);
        }
      }
      return UPLL_RC_SUCCESS;
    } else {
      UPLL_LOG_DEBUG("Error Unsupported datatype(%d)", dt_type);
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  } else if (OPEARTION_WITH_VAL_STRUCT_NONE) {
    /** Value struct is NONE for this operations */
    UPLL_LOG_TRACE("Skip Attribute check, Operation type is %d", operation);
    return UPLL_RC_SUCCESS;
  }

  UPLL_LOG_DEBUG("Error Unsupported operation(%d)", operation);
  return UPLL_RC_ERR_CFG_SYNTAX;
}
upll_rc_t VtnFlowFilterEntryMoMgr::ValidateCapabilityForVtnFlowFilterController(
    IpcReqRespHeader *req,
    ConfigKeyVal *ikey,
    const char* ctrlr_name) {
  UPLL_FUNC_TRACE;

  if (!ctrlr_name)
    ctrlr_name = static_cast<char *> (ikey->get_user_data());

  bool result_code = false;
  const uint8_t *attrs = 0;
  uint32_t max_attrs = 0;

  result_code = GetReadCapability(ctrlr_name, ikey->get_key_type(),
                                    &max_attrs, &attrs);
  if (!result_code) {
    UPLL_LOG_DEBUG("keytype(%d) is not supported by controller(%s)",
                   ikey->get_key_type(), ctrlr_name);
    return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_KT;
  }

  val_flowfilter_controller_t *val_flowfilter_controller = NULL;

  if (ikey->get_cfg_val() && (ikey->get_cfg_val()->get_st_num() ==
      IpctSt::kIpcStKeyVtnFlowfilterController)) {
      val_flowfilter_controller =
        reinterpret_cast<val_flowfilter_controller_t *>(
          ikey->get_cfg_val()->get_val());
  }

  uint32_t dt_type = req->datatype;
  uint32_t operation = req->operation;
  uint32_t option1 = req->option1;
  uint32_t option2 = req->option2;

  UPLL_LOG_TRACE("dt_type   : (%d)"
                "operation : (%d)"
                "option1   : (%d)"
                "option2   : (%d)",
                dt_type, operation, option1, option2);

  if (READ_SUPPORTED_OPERATION) {
    if (dt_type == UPLL_DT_STATE) {
      if ((option1 != UNC_OPT1_NORMAL)
          && (option1 != UNC_OPT1_DETAIL
              && operation == UNC_OP_READ_SIBLING_COUNT)) {
          UPLL_LOG_DEBUG(" Error: option1 is not NORMAL");
         return UPLL_RC_ERR_INVALID_OPTION1;
       }
       if (option2 != UNC_OPT2_NONE) {
         UPLL_LOG_DEBUG(" Error: option2 is not NONE");
         return UPLL_RC_ERR_INVALID_OPTION2;
       }
       if (val_flowfilter_controller) {
        if (max_attrs > 0) {
          return ValVtnFlowfilterCtrlAttributeSupportCheck(
             val_flowfilter_controller, attrs);
        } else {
          UPLL_LOG_DEBUG("Attribute list is empty for operation %d", operation);
        }
       } else if (operation != UNC_OP_READ_SIBLING_COUNT) {
        /** value structure is mandatory for all other operation */
         UPLL_LOG_DEBUG("Mandatory value struct is missing");
         return UPLL_RC_ERR_CFG_SYNTAX;
       }
      /** return SUCCESS, value struct is optional for
         UNC_OP_READ_SIBLING_COUNT */
      return UPLL_RC_SUCCESS;
    } else {
      UPLL_LOG_DEBUG("Error Unsupported datatype (%d)", dt_type);
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  }
  UPLL_LOG_DEBUG("Error Unsupported operation(%d)", operation);
  return UPLL_RC_ERR_CFG_SYNTAX;
}
upll_rc_t VtnFlowFilterEntryMoMgr::ValVtnFlowfilterCtrlAttributeSupportCheck(
  val_flowfilter_controller_t *val_flowfilter_controller,
  const uint8_t* attrs) {
  UPLL_FUNC_TRACE;

  if (val_flowfilter_controller != NULL) {
    if ((val_flowfilter_controller->valid[UPLL_IDX_DIRECTION_FFC] ==
          UNC_VF_VALID) ||
        (val_flowfilter_controller->valid[UPLL_IDX_DIRECTION_FFC] ==
            UNC_VF_VALID_NO_VALUE)) {
      if (attrs[unc::capa::vtn_flowfilter_controller::kCapDirection]== 0) {
        val_flowfilter_controller->valid[UPLL_IDX_DIRECTION_FFC] =
          UNC_VF_NOT_SOPPORTED;

        UPLL_LOG_DEBUG("Direction attr is not supported by ctrlr");
        return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
      }
    }

    if ((val_flowfilter_controller->valid[UPLL_IDX_SEQ_NUM_FFC] ==
          UNC_VF_VALID) ||
        (val_flowfilter_controller->valid[UPLL_IDX_SEQ_NUM_FFC] ==
          UNC_VF_VALID_NO_VALUE)) {
      if (attrs[unc::capa::vtn_flowfilter_controller::kCapSeqNum]== 0) {
        val_flowfilter_controller->valid[UPLL_IDX_SEQ_NUM_FFC] =
          UNC_VF_NOT_SOPPORTED;

        UPLL_LOG_DEBUG("SeqNum attr is not supported by ctrlr");
        return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
      }
    }
  } else {
    UPLL_LOG_DEBUG("Error value struct is NULL");
    return UPLL_RC_ERR_GENERIC;
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
  key_vtn_ffe_ctrl->flowfilter_key.input_direction = val_ff_ctrl->direction;
  if (val_ff_ctrl->valid[UPLL_IDX_SEQ_NUM_FFC] == UNC_VF_VALID) {
    key_vtn_ffe_ctrl->sequence_num = val_ff_ctrl->sequence_num;
  } else {
    key_vtn_ffe_ctrl->sequence_num = 0;
  }

  ikey->DeleteCfgVal();
  val_ff_ctrl = NULL;

  // Allocating CKV tctrl_key
  ConfigKeyVal *tctrl_key = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_ENTRY,
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
                                         op_val_vtn_ffe);
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
    val_vtn_flowfilter_entry_t *&op_val_vtn_ffe) {

  UPLL_FUNC_TRACE;
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

  if (temp_val_vtn_ffe->valid[UPLL_IDX_FLOWLIST_NAME_VFFE] == UNC_VF_INVALID) {
    uuu::upll_strncpy(temp_val_vtn_ffe->flowlist_name,
                      "\0", (kMaxLenFlowListName + 1));
  }
  if (temp_val_vtn_ffe->valid[UPLL_IDX_ACTION_VFFE] == UNC_VF_INVALID) {
    temp_val_vtn_ffe->action = 0;
  }
  if (temp_val_vtn_ffe->valid[UPLL_IDX_NWN_NAME_VFFE] == UNC_VF_INVALID) {
    uuu::upll_strncpy(temp_val_vtn_ffe->nwm_name,
                      "\0", kMaxLenNwmName);
  }
  if (temp_val_vtn_ffe->valid[UPLL_IDX_DSCP_VFFE] == UNC_VF_INVALID) {
    temp_val_vtn_ffe->dscp = 0;
  }
  if (temp_val_vtn_ffe->valid[UPLL_IDX_PRIORITY_VFFE] == UNC_VF_INVALID) {
    temp_val_vtn_ffe->priority = 0;
  }

  memcpy(op_val_vtn_ffe, temp_val_vtn_ffe,
         sizeof(val_vtn_flowfilter_entry_t));
  DELETE_IF_NOT_NULL(tkey);

  return result_code;
}

upll_rc_t VtnFlowFilterEntryMoMgr::GetVtnControllerSpan(
    ConfigKeyVal *ikey,
    DalDmlIntf *dmi,
    std::list<controller_domain_t> &list_ctrlr_dom) {
  UPLL_FUNC_TRACE;
  ConfigKeyVal *okey = NULL;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  VtnMoMgr *mgr = static_cast<VtnMoMgr*>(const_cast<MoManager*>
                                           (GetMoManager(UNC_KT_VTN)));
  result_code = mgr->GetChildConfigKey(okey, NULL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Key not supported by controller");
    return result_code;
  }
  key_vtn_t *vtn_key = reinterpret_cast<key_vtn*>(okey->get_key());
  key_vtn_flowfilter_entry_t *ffe_key =
      reinterpret_cast<key_vtn_flowfilter_entry_t*>(ikey->get_key());
  uuu::upll_strncpy(vtn_key->vtn_name,
                    ffe_key->flowfilter_key.vtn_key.vtn_name,
                    (kMaxLenVtnName+1));
  result_code = mgr->GetControllerDomainSpan(okey, UPLL_DT_CANDIDATE,
                                             dmi,
                                             list_ctrlr_dom);
  if ((result_code != UPLL_RC_SUCCESS) &&
      (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE)) {
    UPLL_LOG_DEBUG("Error in getting controller span (%d)",
                   result_code);
  }
  UPLL_LOG_DEBUG(" GetVtnControllerSpan Result code - %d", result_code);
  delete okey;
  return result_code;
}

upll_rc_t VtnFlowFilterEntryMoMgr::UpdateControllerTable(
    ConfigKeyVal *ikey, unc_keytype_operation_t op,
    upll_keytype_datatype_t dt_type,
    DalDmlIntf *dmi,
    std::list<controller_domain_t> list_ctrlr_dom) {
  UPLL_FUNC_TRACE;
  ConfigKeyVal *ctrlckv = NULL;
  ConfigVal *ctrlcv = NULL;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  for (std::list<controller_domain_t>::iterator it= list_ctrlr_dom.begin();
       it != list_ctrlr_dom.end(); ++it) {
    key_vtn_flowfilter_entry_t *vtn_ffe_key =
        reinterpret_cast<key_vtn_flowfilter_entry_t*>
        (ConfigKeyVal::Malloc(sizeof(key_vtn_flowfilter_entry_t)));
    memcpy(vtn_ffe_key, reinterpret_cast<key_vtn_flowfilter_entry_t*>
           (ikey->get_key()), sizeof(key_vtn_flowfilter_entry_t));
    val_vtn_flowfilter_entry_t *vtn_ffe_val = reinterpret_cast
        <val_vtn_flowfilter_entry_t *>(GetVal(ikey));
    val_vtn_flowfilter_entry_ctrlr_t *ctrlr_val = reinterpret_cast
        <val_vtn_flowfilter_entry_ctrlr_t *>(ConfigKeyVal::Malloc(sizeof
        (val_vtn_flowfilter_entry_ctrlr_t)));
    for (unsigned int loop = 0;
         loop < (sizeof(ctrlr_val->valid)/sizeof(ctrlr_val->valid[0]));
         loop++) {
      if (UNC_VF_NOT_SOPPORTED == vtn_ffe_val->valid[loop]) {
        ctrlr_val->valid[loop] = UNC_VF_INVALID;
      } else {
        ctrlr_val->valid[loop] = vtn_ffe_val->valid[loop];
      }
    }
    ctrlcv = new ConfigVal(IpctSt::kIpcInvalidStNum, ctrlr_val);
    ctrlckv = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER_ENTRY,
                               IpctSt::kIpcStKeyVtnFlowfilterEntry,
                               vtn_ffe_key, ctrlcv);
    // ctrlckv->AppendCfgVal(IpctSt::kIpcInvalidStNum, ctrlr_val);
    SET_USER_DATA_CTRLR_DOMAIN(ctrlckv, *it);

    // Create/Update/Delete a record in CANDIDATE DB
    result_code = UpdateConfigDB(ctrlckv, dt_type, op, dmi, CTRLRTBL);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Err while inserting in ctrlr table for candidateDb(%d)",
                     result_code);
    }

    if (ctrlckv) {
      delete ctrlckv;
      ctrlckv = NULL;
    }
    if (result_code != UPLL_RC_SUCCESS) {
      return result_code;
    }
  }
  return result_code;
}

upll_rc_t VtnFlowFilterEntryMoMgr::UpdateMainTbl(ConfigKeyVal *vtn_ffe_key,
      unc_keytype_operation_t op, uint32_t driver_result,
      ConfigKeyVal *nreq, DalDmlIntf *dmi) {
  ConfigKeyVal *ck_vtn_ffe = NULL;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  val_vtn_flowfilter_entry_t *vtn_ffe_val = NULL;
  void *ffeval = NULL;
  void *nffeval = NULL;

  UPLL_FUNC_TRACE;
  if (op != UNC_OP_DELETE) {
    result_code = DupConfigKeyVal(ck_vtn_ffe, vtn_ffe_key, MAINTBL);
    if (!ck_vtn_ffe || result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Returning error %d\n", result_code);
      return UPLL_RC_ERR_GENERIC;
    }
    vtn_ffe_val = reinterpret_cast<val_vtn_flowfilter_entry_t *>
        (GetVal(ck_vtn_ffe));
    if (!vtn_ffe_val) {
      UPLL_LOG_DEBUG("invalid val \n");
      return UPLL_RC_ERR_GENERIC;
    }
  } else {
    result_code = GetChildConfigKey(ck_vtn_ffe, vtn_ffe_key);
    if (!ck_vtn_ffe || result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Returning error %d\n", result_code);
      return UPLL_RC_ERR_GENERIC;
    }
  }
  switch (op) {
    case UNC_OP_CREATE:
      vtn_ffe_val->cs_row_status = UNC_CS_NOT_APPLIED;
      break;
    case UNC_OP_UPDATE:
      ffeval = reinterpret_cast<void *>(&vtn_ffe_val);
      nffeval = (nreq)?GetVal(nreq):NULL;
      if (!nffeval) {
        UPLL_LOG_DEBUG("Invalid param\n");
        return UPLL_RC_ERR_GENERIC;
      }
      CompareValidValue(ffeval, nffeval, false);
      break;
    case UNC_OP_DELETE:
      break;
    default:
      UPLL_LOG_DEBUG("Inalid operation\n");
      return UPLL_RC_ERR_GENERIC;
  }
  result_code = UpdateConfigDB(ck_vtn_ffe, UPLL_DT_STATE, op, dmi, MAINTBL);
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
    return UPLL_RC_SUCCESS;
  }
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Unable to read configuration from CandidateDb");
    delete tempckv;
    return result_code;
  }
  ConfigKeyVal *iter_ckv = tempckv;
  while (NULL != iter_ckv) {
    val_vtn_flowfilter_entry_t *vtn_ffe_val = reinterpret_cast
            <val_vtn_flowfilter_entry_t *>(GetVal(iter_ckv));
    if (UNC_VF_VALID == vtn_ffe_val->valid[UPLL_IDX_FLOWLIST_NAME_VFFE]) {
      result_code = UpdateFlowListInCtrl(iter_ckv, UNC_OP_DELETE, dmi);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Unable to update flowlist in ctrlr table");
        delete tempckv;
        return result_code;
      }
    }
    iter_ckv = iter_ckv->get_next_cfg_key_val();
  }
  delete tempckv;
  result_code = UpdateConfigDB(ikey, req->datatype, req->operation, dmi,
      MAINTBL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("DeleteMo record Err in vtnpolicingmaptbl (%d)",
        result_code);
    return result_code;
  }
  result_code = UpdateConfigDB(ikey, req->datatype, req->operation, dmi,
      CTRLRTBL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("DeleteMo record Err in vtnpolicingmaptbl (%d)",
        result_code);
    return result_code;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtnFlowFilterEntryMoMgr::IsFlowListConfigured(
    const char* flowlist_name, DalDmlIntf *dmi) {
  upll_rc_t result_code = UPLL_RC_ERR_GENERIC;
  ConfigKeyVal *ckv = NULL;
  result_code = GetChildConfigKey(ckv, NULL);
  if (UPLL_RC_SUCCESS != result_code) {
    return result_code;
  }
  if (!ckv) return UPLL_RC_ERR_GENERIC;
  val_flowfilter_entry_t *ffe_val = reinterpret_cast
    <val_flowfilter_entry_t *>(ConfigKeyVal::Malloc(sizeof
          (val_flowfilter_entry_t)));
  uuu::upll_strncpy(ffe_val->flowlist_name, flowlist_name,
      (kMaxLenFlowListName + 1));
  ckv->AppendCfgVal(IpctSt::kIpcStValFlowfilterEntry, ffe_val);
  DbSubOp dbop = {kOpReadMultiple, kOpMatchNone, kOpInOutNone};
  // Read the Configuration from the MainTable
  result_code = ReadConfigDB(ckv, UPLL_DT_CANDIDATE,
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
    result_code = ReadInfoFromDB(req, ikey, dmi, &ctrlr_dom);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG(" Read DB request failed result(%d)", result_code);
      return result_code;
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
      if (val_ff_ctrl->valid[UPLL_IDX_SEQ_NUM_FFC] == UNC_VF_VALID) {
        key_vtn_ffe_ctrl->sequence_num = val_ff_ctrl->sequence_num;
      } else {
        key_vtn_ffe_ctrl->sequence_num = 0;
      }
      key_vtn_ffe_ctrl->flowfilter_key.input_direction = val_ff_ctrl->direction;
      // Reading The  Entry_Ctrl_Table
      DbSubOp dbop = {kOpReadMultiple, kOpMatchNone,
        kOpInOutCtrlr|kOpInOutDomain};
      result_code = ReadConfigDB(tctrl_key, UPLL_DT_STATE, UNC_OP_READ,
                                 dbop, req->rep_count, dmi, CTRLRTBL);
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
    } else {
      return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT;
    }
  }
  return result_code;
}

}  // namespace kt_momgr
}  // namespace upll
}  // namespace unc
