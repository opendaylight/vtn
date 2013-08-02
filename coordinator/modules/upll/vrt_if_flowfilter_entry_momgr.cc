/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include "pfc/log.h"
#include "vrt_if_flowfilter_entry_momgr.hh"
#include "unc/upll_errno.h"
#include "upll_validation.hh"
#include "unc/upll_ipc_enum.h"
#include "flowlist_momgr.hh"
#include "vrt_if_momgr.hh"
#include "upll_log.hh"
#include "vbr_flowfilter_entry_momgr.hh"

namespace unc {
namespace upll {
namespace kt_momgr {

using unc::upll::ipc_util::IpcUtil;
#define NUM_KEY_MAIN_TBL_  5
#define NUM_KEY_RENAME_MAIN_TBL 7

#define FLOWLIST_RENAME_FLAG    0x04  // For 3rd Bit
#define VTN_RENAME_FLAG         0x01  // For first Bit
#define VBR_RENAME_FLAG         0x10  // For 2nd Bit
#define VRT_RENAME_FLAG         0x10  // For 2nd Bit
#define VLINK_CONFIGURED 0x01
#define PORTMAP_CONFIGURED 0x02
#define VLINK_PORTMAP_CONFIGURED 0x03
#define SET_FLAG_VLINK 0x08
#define SET_FLAG_PORTMAP 0x0A
#define SET_FLAG_VLINK_PORTMAP 0x018

BindInfo VrtIfFlowFilterEntryMoMgr::vrt_if_flowfilter_entry_bind_info[] = {
  { uudst::vrt_if_flowfilter_entry::kDbiVtnName, CFG_KEY,
    offsetof(key_vrt_if_flowfilter_entry_t,
             flowfilter_key.if_key.vrt_key.vtn_key.vtn_name),
    uud::kDalChar, (kMaxLenVtnName + 1) },
  { uudst::vrt_if_flowfilter_entry::kDbiVrtName, CFG_KEY,
    offsetof(key_vrt_if_flowfilter_entry_t,
             flowfilter_key.if_key.vrt_key.vrouter_name),
    uud::kDalChar, (kMaxLenVnodeName + 1) },
  { uudst::vrt_if_flowfilter_entry::kDbiVrtIfName, CFG_KEY,
    offsetof(key_vrt_if_flowfilter_entry_t, flowfilter_key.if_key.if_name),
    uud::kDalChar, (kMaxLenInterfaceName + 1) },
  { uudst::vrt_if_flowfilter_entry::kDbiInputDirection, CFG_KEY,
    offsetof(key_vrt_if_flowfilter_entry_t, flowfilter_key.direction),
    uud::kDalUint8, 1 },
  { uudst::vrt_if_flowfilter_entry::kDbiSequenceNum, CFG_KEY,
    offsetof(key_vrt_if_flowfilter_entry_t, sequence_num),
    uud::kDalUint16, 1 },
  { uudst::vrt_if_flowfilter_entry::kDbiCtrlrName, CK_VAL,
    offsetof(key_user_data_t, ctrlr_id),
    uud::kDalChar, (kMaxLenCtrlrId + 1) },
  { uudst::vrt_if_flowfilter_entry::kDbiDomainId, CK_VAL,
      offsetof(key_user_data, domain_id),
      uud::kDalChar, (kMaxLenDomainId + 1) },
  { uudst::vrt_if_flowfilter_entry::kDbiFlowlistName, CFG_VAL,
    offsetof(val_flowfilter_entry_t, flowlist_name),
      uud::kDalChar, (kMaxLenFlowListName + 1) },
  { uudst::vrt_if_flowfilter_entry::kDbiAction, CFG_VAL,
    offsetof(val_flowfilter_entry_t, action),
    uud::kDalUint8, 1 },
  { uudst::vrt_if_flowfilter_entry::kDbiRedirectNode, CFG_VAL,
    offsetof(val_flowfilter_entry_t, redirect_node),
    uud::kDalChar, (kMaxLenVnodeName + 1) },
  { uudst::vrt_if_flowfilter_entry::kDbiRedirectPort, CFG_VAL,
    offsetof(val_flowfilter_entry_t, redirect_port),
    uud::kDalChar, (kMaxLenInterfaceName + 1) },
  { uudst::vrt_if_flowfilter_entry::kDbiModifyDstMac, CFG_VAL,
    offsetof(val_flowfilter_entry_t, modify_dstmac),
    uud::kDalChar, 6},
  { uudst::vrt_if_flowfilter_entry::kDbiModifySrcMac, CFG_VAL,
    offsetof(val_flowfilter_entry_t, modify_srcmac),
    uud::kDalChar, 6},
  { uudst::vrt_if_flowfilter_entry::kDbiNwmName, CFG_VAL,
    offsetof(val_flowfilter_entry_t, nwm_name),
    uud::kDalChar, (kMaxLenNwmName + 1) },
  { uudst::vrt_if_flowfilter_entry::kDbiDscp, CFG_VAL,
    offsetof(val_flowfilter_entry_t, dscp),
    uud::kDalUint8, 1 },
  { uudst::vrt_if_flowfilter_entry::kDbiPriority, CFG_VAL,
    offsetof(val_flowfilter_entry_t, priority),
    uud::kDalUint8, 1 },
  { uudst::vrt_if_flowfilter_entry::kDbiFlags, CK_VAL,
    offsetof(key_user_data_t, flags),
    uud::kDalUint8, 1 },
  { uudst::vrt_if_flowfilter_entry::kDbiValidFlowlistName, CFG_META_VAL,
    offsetof(val_flowfilter_entry_t, valid[0]),
    uud::kDalUint8, 1 },
  { uudst::vrt_if_flowfilter_entry::kDbiValidAction, CFG_META_VAL,
    offsetof(val_flowfilter_entry_t, valid[1]),
    uud::kDalUint8, 1 },
  { uudst::vrt_if_flowfilter_entry::kDbiValidRedirectNode, CFG_META_VAL,
    offsetof(val_flowfilter_entry_t, valid[2]),
    uud::kDalUint8, 1 },
  { uudst::vrt_if_flowfilter_entry::kDbiValidRedirectPort, CFG_META_VAL,
    offsetof(val_flowfilter_entry_t, valid[3]),
    uud::kDalUint8, 1 },
  { uudst::vrt_if_flowfilter_entry::kDbiValidModifyDstMac, CFG_META_VAL,
    offsetof(val_flowfilter_entry_t, valid[4]),
    uud::kDalUint8, 1 },
  { uudst::vrt_if_flowfilter_entry::kDbiValidModifySrcMac, CFG_META_VAL,
    offsetof(val_flowfilter_entry_t, valid[5]),
    uud::kDalUint8, 1 },
  { uudst::vrt_if_flowfilter_entry::kDbiValidNwmName, CFG_META_VAL,
    offsetof(val_flowfilter_entry_t, valid[6]),
    uud::kDalUint8, 1 },
  { uudst::vrt_if_flowfilter_entry::kDbiValidDscp, CFG_META_VAL,
    offsetof(val_flowfilter_entry_t, valid[7]),
    uud::kDalUint8, 1 },
  { uudst::vrt_if_flowfilter_entry::kDbiValidPriority, CFG_META_VAL,
    offsetof(val_flowfilter_entry_t, valid[8]),
    uud::kDalUint8, 1 },
  { uudst::vrt_if_flowfilter_entry::kDbiCsRowStatus, CS_VAL,
    offsetof(val_flowfilter_entry_t, cs_row_status),
    uud::kDalUint8, 1 },
  { uudst::vrt_if_flowfilter_entry::kDbiCsFlowlistName, CS_VAL,
    offsetof(val_flowfilter_entry_t, cs_attr[0]),
    uud::kDalUint8, 1 },
  { uudst::vrt_if_flowfilter_entry::kDbiCsAction, CS_VAL,
    offsetof(val_flowfilter_entry_t, cs_attr[1]),
    uud::kDalUint8, 1 },
  { uudst::vrt_if_flowfilter_entry::kDbiCsRedirectNode, CS_VAL,
    offsetof(val_flowfilter_entry_t, cs_attr[2]),
    uud::kDalUint8, 1 },
  { uudst::vrt_if_flowfilter_entry::kDbiCsRedirectPort, CS_VAL,
    offsetof(val_flowfilter_entry_t, cs_attr[3]),
    uud::kDalUint8, 1 },
  { uudst::vrt_if_flowfilter_entry::kDbiCsModifyDstMac, CS_VAL,
    offsetof(val_flowfilter_entry_t, cs_attr[4]),
    uud::kDalUint8, 1 },
  { uudst::vrt_if_flowfilter_entry::kDbiCsModifySrcMac, CS_VAL,
    offsetof(val_flowfilter_entry_t, cs_attr[5]),
    uud::kDalUint8, 1 },
  { uudst::vrt_if_flowfilter_entry::kDbiCsNwmName, CS_VAL,
    offsetof(val_flowlist_entry_t, cs_attr[6]),
    uud::kDalUint8, 1 },
  { uudst::vrt_if_flowfilter_entry::kDbiCsDscp, CS_VAL,
    offsetof(val_flowlist_entry_t, cs_attr[7]),
    uud::kDalUint8, 1 },
  { uudst::vrt_if_flowfilter_entry::kDbiCsPriority, CS_VAL,
    offsetof(val_flowlist_entry_t, cs_attr[8]),
    uud::kDalUint8, 1 }
};

BindInfo VrtIfFlowFilterEntryMoMgr::vrt_if_flowfilter_entry_maintbl_bind_info[]
= {
  { uudst::vrt_if_flowfilter_entry::kDbiVtnName, CFG_MATCH_KEY,
    offsetof(key_vrt_if_flowfilter_entry_t,
             flowfilter_key.if_key.vrt_key.vtn_key.vtn_name),
    uud::kDalChar, (kMaxLenVtnName + 1) },
  { uudst::vrt_if_flowfilter_entry::kDbiVrtName, CFG_MATCH_KEY,
    offsetof(key_vrt_if_flowfilter_entry_t,
             flowfilter_key.if_key.vrt_key.vrouter_name),
    uud::kDalChar, (kMaxLenVnodeName + 1) },
  { uudst::vrt_if_flowfilter_entry::kDbiVrtIfName, CFG_MATCH_KEY,
    offsetof(key_vrt_if_flowfilter_entry_t, flowfilter_key.if_key.if_name),
    uud::kDalChar, (kMaxLenInterfaceName + 1) },
  { uudst::vrt_if_flowfilter_entry::kDbiInputDirection, CFG_MATCH_KEY,
    offsetof(key_vrt_if_flowfilter_entry_t, flowfilter_key.direction),
    uud::kDalUint8, 1 },
  { uudst::vrt_if_flowfilter_entry::kDbiSequenceNum, CFG_MATCH_KEY,
    offsetof(key_vrt_if_flowfilter_entry_t, sequence_num),
    uud::kDalUint16, 1 },
  { uudst::vrt_if_flowfilter_entry::kDbiVtnName, CFG_INPUT_KEY,
    offsetof(key_rename_vnode_info_t, new_unc_vtn_name),
    uud::kDalChar, (kMaxLenVtnName + 1) },
  { uudst::vrt_if_flowfilter_entry::kDbiVrtName, CFG_INPUT_KEY,
    offsetof(key_rename_vnode_info_t, new_unc_vnode_name),
    uud::kDalChar, (kMaxLenVnodeName + 1) },
  { uudst::vrt_if_flowfilter_entry::kDbiFlags, CFG_INPUT_KEY,
    offsetof(key_user_data_t, flags),
    uud::kDalUint8, 1 }
};

BindInfo VrtIfFlowFilterEntryMoMgr::vrt_if_flowlist_rename_bind_info[] = {
  { uudst::vrt_if_flowfilter_entry::kDbiVtnName, CFG_MATCH_KEY,
    offsetof(key_vrt_if_flowfilter_entry_t,
             flowfilter_key.if_key.vrt_key.vtn_key.vtn_name),
    uud::kDalChar, (kMaxLenVtnName + 1) },
  { uudst::vrt_if_flowfilter_entry::kDbiVrtName, CFG_MATCH_KEY,
    offsetof(key_vrt_if_flowfilter_entry_t,
             flowfilter_key.if_key.vrt_key.vrouter_name),
    uud::kDalChar, (kMaxLenVnodeName + 1) },
  { uudst::vrt_if_flowfilter_entry::kDbiVrtIfName, CFG_MATCH_KEY,
    offsetof(key_vrt_if_flowfilter_entry_t, flowfilter_key.if_key.if_name),
    uud::kDalChar, (kMaxLenInterfaceName + 1) },
  { uudst::vrt_if_flowfilter_entry::kDbiInputDirection, CFG_MATCH_KEY,
    offsetof(key_vrt_if_flowfilter_entry_t, flowfilter_key.direction),
    uud::kDalUint8, 1 },
  { uudst::vrt_if_flowfilter_entry::kDbiSequenceNum, CFG_MATCH_KEY,
    offsetof(key_vrt_if_flowfilter_entry_t, sequence_num),
    uud::kDalUint16, 1 },
  { uudst::vrt_if_flowfilter_entry::kDbiFlowlistName, CFG_INPUT_KEY,
    offsetof(key_rename_vnode_info_t, new_flowlist_name),
    uud::kDalChar, (kMaxLenFlowListName + 1) },
  { uudst::vrt_if_flowfilter_entry::kDbiFlags, CFG_INPUT_KEY,
    offsetof(key_user_data_t, flags),
    uud::kDalUint8, 1 }
};

VrtIfFlowFilterEntryMoMgr::VrtIfFlowFilterEntryMoMgr() : MoMgrImpl() {
  UPLL_FUNC_TRACE;
  // Rename and ctrlr tables not required for this KT
  // setting  table indexed for ctrl table and rename table to NULL
  ntable = (MAX_MOMGR_TBLS);
  table = new Table *[ntable];

  // For Main Table
  table[MAINTBL] = new Table(uudst::kDbiVrtIfFlowFilterEntryTbl,
      UNC_KT_VRTIF_FLOWFILTER_ENTRY, vrt_if_flowfilter_entry_bind_info,
      IpctSt::kIpcStKeyVrtIfFlowfilterEntry, IpctSt::kIpcStValFlowfilterEntry,
      uudst::vrt_if_flowfilter_entry::kDbiVrtIfFlowFilterEntryNumCols);
  table[RENAMETBL] = NULL;
  table[CTRLRTBL] = NULL;
  nchild = 0;
  child = NULL;
}

upll_rc_t VrtIfFlowFilterEntryMoMgr::DeleteMo(IpcReqRespHeader *req,
                                              ConfigKeyVal *ikey,
                                              DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  uint8_t *ctrlr_id = NULL;
  upll_rc_t result_code = UPLL_RC_ERR_GENERIC;
  if (NULL == ikey && NULL == req) return result_code;
  result_code = ValidateMessage(req, ikey);
  if (result_code != UPLL_RC_SUCCESS)
    return result_code;

  result_code = UpdateConfigDB(ikey, UPLL_DT_CANDIDATE, UNC_OP_READ, dmi);
  if (UPLL_RC_ERR_INSTANCE_EXISTS != result_code) {
    UPLL_LOG_TRACE("Instance Already Exists");
    return result_code;
  }
  ConfigKeyVal *okey = NULL;
  result_code = GetChildConfigKey(okey, ikey);
  if (UPLL_RC_SUCCESS != result_code) {
    return result_code;
  }
  DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutCtrlr };
  result_code = ReadConfigDB(okey, UPLL_DT_CANDIDATE,
                             UNC_OP_READ, dbop, dmi, MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Unable to read configuration from CandidateDb");
    delete okey;
    return result_code;
  }
  GET_USER_DATA_CTRLR(ikey, ctrlr_id);
  val_flowfilter_entry_t *flowfilter_val =
      reinterpret_cast<val_flowfilter_entry_t *> (GetVal(okey));
  if (flowfilter_val->valid[UPLL_IDX_FLOWLIST_NAME_FFE] == UNC_VF_VALID) {
    FlowListMoMgr *mgr = reinterpret_cast<FlowListMoMgr *>
        (const_cast<MoManager *> (GetMoManager(UNC_KT_FLOWLIST)));
    result_code = mgr->AddFlowListToController(
        reinterpret_cast<char *>(flowfilter_val->flowlist_name), dmi,
        reinterpret_cast<char *>(ctrlr_id) , UNC_OP_DELETE);
    if (result_code != UPLL_RC_SUCCESS) return result_code;
  }

  result_code = UpdateConfigDB(ikey, UPLL_DT_CANDIDATE, UNC_OP_DELETE, dmi,
                               MAINTBL);

  return result_code;
}

upll_rc_t VrtIfFlowFilterEntryMoMgr::UpdateAuditConfigStatus(
                      unc_keytype_configstatus_t cs_status,
                      uuc::UpdateCtrlrPhase phase,
                      ConfigKeyVal *&ckv_running) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  val_flowfilter_entry_t *vrt_if_flowfilter_entry_val = NULL;

  vrt_if_flowfilter_entry_val = (ckv_running != NULL)?
      reinterpret_cast<val_flowfilter_entry_t *> (GetVal(ckv_running)):NULL;

  if (NULL == vrt_if_flowfilter_entry_val) {
    UPLL_LOG_DEBUG("UpdateAuditConfigStatus::vrt_if_flowfilter_entry_val Null");
    return UPLL_RC_ERR_GENERIC;
  }

  if (uuc::kUpllUcpCreate == phase )
    vrt_if_flowfilter_entry_val->cs_row_status = cs_status;
  for ( unsigned int loop = 0;
       loop < sizeof(vrt_if_flowfilter_entry_val->valid)/
       sizeof(vrt_if_flowfilter_entry_val->valid[0]); ++loop ) {
    if ((cs_status == UNC_CS_INVALID) &&
        (UNC_VF_VALID == vrt_if_flowfilter_entry_val->valid[loop]))
      vrt_if_flowfilter_entry_val->cs_attr[loop] = cs_status;
    else
      vrt_if_flowfilter_entry_val->cs_attr[loop] = cs_status;
  }
  UPLL_LOG_DEBUG("UpdateAuditConfigStatus::Success");
  return result_code;
}

upll_rc_t VrtIfFlowFilterEntryMoMgr::UpdateMo(IpcReqRespHeader *req,
                                              ConfigKeyVal *ikey,
                                              DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  ConfigKeyVal *okey = NULL;
  uint8_t *ctrlr_id = NULL;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  if (NULL == ikey && NULL == req) {
    UPLL_LOG_DEBUG("Both Request and Input Key are Null");
    return UPLL_RC_ERR_GENERIC;
  }
  UPLL_LOG_DEBUG("In updateMo");
  result_code = ValidateMessage(req, ikey);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("Vallidation is Failed %d", result_code);
    return result_code;
  }

  result_code = ValidateVrtIfValStruct(req, ikey, dmi);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("ValidateVrtIfValStruct Failed %d", result_code);
    return result_code;
  }

  // Capability Check
  /*
  result_code = ValidateCapability(req, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Key/Attribute not supported by controller");
    // return result_code;
  }*/
  // Check if Object exists in Candidate Configuration or not
  result_code = UpdateConfigDB(ikey, req->datatype, UNC_OP_READ, dmi, MAINTBL);

  if (UPLL_RC_ERR_INSTANCE_EXISTS != result_code) {
    UPLL_LOG_DEBUG("Recored %d", result_code);
    return result_code;
  }
  result_code = ValidateAttribute(ikey, dmi, req);
  if (UPLL_RC_SUCCESS != result_code)
    return result_code;
  //  Check and update the flowlist reference count
  // if the flowlist object is referred
  FlowListMoMgr *flowlist_mgr = reinterpret_cast<FlowListMoMgr *>
      (const_cast<MoManager *> (GetMoManager(UNC_KT_FLOWLIST)));
  val_flowfilter_entry_t *flowfilter_val =
    reinterpret_cast<val_flowfilter_entry_t *> (GetVal(ikey));
  if (UNC_VF_VALID == flowfilter_val->valid[UPLL_IDX_FLOWLIST_NAME_FFE]) {
    // Check Flowlist object exist or not
    result_code = flowlist_mgr->GetChildConfigKey(okey, NULL);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Recored %d", result_code);
      return result_code;
    }
    key_flowlist_t *key_flowlist = reinterpret_cast<key_flowlist_t*>
      (okey->get_key());
    uuu::upll_strncpy(key_flowlist->flowlist_name,
        flowfilter_val->flowlist_name,
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
    if (okey != NULL) {
      delete okey;
      okey = NULL;
    }
  }
  if (UNC_VF_VALID == flowfilter_val->valid[UPLL_IDX_FLOWLIST_NAME_FFE] ||
      UNC_VF_VALID_NO_VALUE == flowfilter_val->
      valid[UPLL_IDX_FLOWLIST_NAME_FFE]) {
    result_code = GetChildConfigKey(okey, ikey);
      if (UPLL_RC_SUCCESS != result_code) {
        UPLL_LOG_DEBUG("GetChildConfigKey failed");
        return result_code;
      }
    DbSubOp dbop = { kOpReadSingle, kOpMatchNone,
                     kOpInOutCtrlr|kOpInOutDomain };
    result_code = ReadConfigDB(okey, UPLL_DT_CANDIDATE, UNC_OP_READ, dbop, dmi,
        MAINTBL);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("ReadConfigDB failed %d", result_code);
      return result_code;
    }
    GET_USER_DATA_CTRLR(okey, ctrlr_id);
    val_flowfilter_entry_t *temp_ffe_val = reinterpret_cast
      <val_flowfilter_entry_t *>(GetVal(okey));
    UPLL_LOG_DEBUG("flowlist name %s", flowfilter_val->flowlist_name);
    if (UNC_VF_VALID == flowfilter_val->valid[UPLL_IDX_FLOWLIST_NAME_VFFE] &&
        UNC_VF_VALID  == temp_ffe_val->
        valid[UPLL_IDX_FLOWLIST_NAME_VFFE]) {
      UPLL_LOG_DEBUG("Update option 1");
      result_code = flowlist_mgr->AddFlowListToController(
          reinterpret_cast<char *>(temp_ffe_val->flowlist_name), dmi,
          reinterpret_cast<char *>(ctrlr_id) , UNC_OP_DELETE);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("AddFlowListToController failed %d", result_code);
        delete okey;
        return result_code;
      }
      result_code = flowlist_mgr->AddFlowListToController(
          reinterpret_cast<char *>(flowfilter_val->flowlist_name), dmi,
          reinterpret_cast<char *>(ctrlr_id) , UNC_OP_CREATE);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("AddFlowListToController failed %d", result_code);
        delete okey;
        return result_code;
      }
    } else if (UNC_VF_VALID == flowfilter_val->
        valid[UPLL_IDX_FLOWLIST_NAME_VFFE] &&
        (UNC_VF_INVALID == temp_ffe_val->
         valid[UPLL_IDX_FLOWLIST_NAME_VFFE] || UNC_VF_VALID_NO_VALUE ==
         temp_ffe_val->valid[UPLL_IDX_FLOWLIST_NAME_VFFE])) {
      UPLL_LOG_DEBUG("Update option 2");
      result_code = flowlist_mgr->AddFlowListToController(
          reinterpret_cast<char *>(flowfilter_val->flowlist_name), dmi,
          reinterpret_cast<char *> (ctrlr_id) , UNC_OP_CREATE);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("AddFlowListToController failed %d", result_code);
        delete okey;
        return result_code;
      }
    } else if (UNC_VF_VALID_NO_VALUE == flowfilter_val->
        valid[UPLL_IDX_FLOWLIST_NAME_VFFE] &&
        UNC_VF_VALID == temp_ffe_val->
        valid[UPLL_IDX_FLOWLIST_NAME_VFFE]) {
      UPLL_LOG_DEBUG("Update option 3");
      result_code = flowlist_mgr->AddFlowListToController(
          reinterpret_cast<char *>(temp_ffe_val->flowlist_name), dmi,
          reinterpret_cast<char *>(ctrlr_id) , UNC_OP_DELETE);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("AddFlowListToController failed %d", result_code);
        delete okey;
        return result_code;
      }
    }
  }

  // N/w monitor
#if 0
  NwMonitorMoMgr *nmgr = reinterpret_cast<NwMonitorMoMgr *>
  (const_cast<MoManager *> (GetMoManager(UNC_KT_VBR_NWMONITOR)));
  //  result_code = nmgr->GetChildConfigKey(okey, NULL); //TODO
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Recored %d", result_code);
    return result_code;
  }
  key_nwm_t *key_nwm = reinterpret_cast<key_nwm_t*>(okey->get_key());
  uuu::upll_strncpy(reinterpret_cast<char*>(key_nwm->nwmonitor_name),
     reinterpret_cast<const char*>(flowfilter_val->nwm_name),
               kMaxLenNwmName +1);
  //  result_code = nmgr->IsReferenced(okey, req->datatype, dmi); //TODO
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Recored %d", result_code);
    return result_code;
  }
#endif
  result_code = UpdateConfigDB(ikey, req->datatype, UNC_OP_UPDATE,
  dmi, MAINTBL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("UpdateConfigDB is Failed %d", result_code);
    return result_code;
  }
  delete okey;
  return result_code;
}

upll_rc_t VrtIfFlowFilterEntryMoMgr::GetRenamedControllerKey(
  ConfigKeyVal *&ikey, upll_keytype_datatype_t dt_type, DalDmlIntf *dmi,
  controller_domain *ctrlr_dom) {
  UPLL_FUNC_TRACE
  ConfigKeyVal *okey = NULL;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  // ConfigKeyVal *temp_key=ikey;
  uint8_t rename = 0;
  IsRenamed(ikey, dt_type, dmi, rename);
  if (!rename) return UPLL_RC_SUCCESS;
  /* vtn renamed */
  if (rename & VTN_RENAME_FLAG) {
    MoMgrImpl *mgrvtn =  static_cast<MoMgrImpl*>
    ((const_cast<MoManager*> (GetMoManager(UNC_KT_VTN))));
    mgrvtn->GetChildConfigKey(okey, NULL);
    if (ctrlr_dom != NULL)
      SET_USER_DATA_CTRLR_DOMAIN(okey, *ctrlr_dom);
    uuu::upll_strncpy(
             reinterpret_cast<key_vtn *>(okey->get_key())->vtn_name,
             reinterpret_cast<key_vrt_if_flowfilter_entry_t *>
             (ikey->get_key())->flowfilter_key.if_key.vrt_key.vtn_key.vtn_name,
             (kMaxLenVtnName + 1));
    DbSubOp dbop = { kOpReadSingle, kOpMatchCtrlr, kOpInOutFlag };
    /* ctrlr_name */
    result_code =  mgrvtn->ReadConfigDB(okey, dt_type, UNC_OP_READ,
                               dbop, dmi, RENAMETBL);
    if ( result_code != UPLL_RC_SUCCESS ) {
       return UPLL_RC_ERR_GENERIC;
    }
    val_rename_vtn *rename_val = reinterpret_cast <val_rename_vtn *>
    (GetVal(okey));  // NULL Checks Missing
    if (!rename_val
      || (rename_val->valid[UPLL_IDX_NEW_NAME_RVTN] != UNC_VF_VALID)) {
      UPLL_LOG_DEBUG("Vtn Name is not Valid.");
      return UPLL_RC_ERR_GENERIC;
    }
    uuu::upll_strncpy(
         reinterpret_cast<key_vrt_if_flowfilter_entry_t*>
         (ikey->get_key())->flowfilter_key.if_key.vrt_key.vtn_key.vtn_name,
         rename_val->new_name,
         (kMaxLenVtnName + 1));
    SET_USER_DATA_FLAGS(ikey, rename);
    delete okey;
  }
/*Vrouter_name*/
  if (rename & VRT_RENAME_FLAG) {
     okey = NULL;
     MoMgrImpl *VrtMoMgr = static_cast<MoMgrImpl*>
    ((const_cast<MoManager*> (GetMoManager(UNC_KT_VROUTER))));
    if (VrtMoMgr == NULL) {
      UPLL_LOG_DEBUG("InValid Reference of VRTIF");
      return UPLL_RC_ERR_BAD_REQUEST;
    }
    result_code = VrtMoMgr->GetChildConfigKey(okey, ikey);
    if ( result_code != UPLL_RC_SUCCESS ) {  // COV USE AFTER FREE
       // delete okey;
       return UPLL_RC_ERR_GENERIC;
    }
    if (ctrlr_dom)
      SET_USER_DATA_CTRLR_DOMAIN(okey, *ctrlr_dom);
    DbSubOp dbop = { kOpReadSingle, kOpMatchCtrlr, kOpInOutFlag };
    /* ctrlr_name */
    result_code =  VrtMoMgr->ReadConfigDB(okey, dt_type, UNC_OP_READ,
                                         dbop, dmi, RENAMETBL);
    if ( result_code != UPLL_RC_SUCCESS ) {
       return UPLL_RC_ERR_GENERIC;
    }
    // NULL Checks Missing
    val_rename_vrt_t *rename_val =
    reinterpret_cast <val_rename_vrt_t *> (GetVal(okey));
    if (!rename_val
       || (rename_val->valid[UPLL_IDX_NEW_NAME_RVTN] != UNC_VF_VALID)) {
      UPLL_LOG_DEBUG("VRT Name is not Valid.");
      return UPLL_RC_ERR_GENERIC;
    }
    uuu::upll_strncpy(
    reinterpret_cast<key_vrt_if_flowfilter_entry_t*>
    (ikey->get_key())->flowfilter_key.if_key.vrt_key.vrouter_name,
    rename_val->new_name,
    (kMaxLenVnodeName + 1));
    SET_USER_DATA_FLAGS(ikey, rename);
    delete okey;
  }
  //  flowlist_name
  if (rename & FLOWLIST_RENAME_FLAG) {
    MoMgrImpl *mgrflist = static_cast<MoMgrImpl*>
    ((const_cast<MoManager*> (GetMoManager(UNC_KT_FLOWLIST))));
    mgrflist->GetChildConfigKey(okey, NULL);
    if (ctrlr_dom)
      SET_USER_DATA_CTRLR(okey, ctrlr_dom->ctrlr);
    DbSubOp dbop = { kOpReadSingle, kOpMatchCtrlr, kOpInOutFlag };
    /* ctrlr_name */
    result_code =  mgrflist->ReadConfigDB(okey, dt_type,
                                UNC_OP_READ, dbop, dmi, RENAMETBL);
    if ( result_code != UPLL_RC_SUCCESS ) {
       return UPLL_RC_ERR_GENERIC;
    }
    val_rename_flowlist_t *rename_val =
        reinterpret_cast <val_rename_flowlist_t*>(GetVal(okey));

    if (!rename_val
       || (rename_val->valid[UPLL_IDX_NEW_NAME_RVTN] != UNC_VF_VALID))
      return UPLL_RC_ERR_GENERIC;
    uuu::upll_strncpy(
    reinterpret_cast<val_flowfilter_entry_t*>
    (ikey->get_cfg_val()->get_val())->flowlist_name,
    rename_val->flowlist_newname,
    (kMaxLenFlowListName + 1));
    SET_USER_DATA_FLAGS(ikey, rename);
    delete okey;
  }
  UPLL_LOG_DEBUG("Renamed Controller key is sucessfull.");
  return UPLL_RC_SUCCESS;
}

upll_rc_t VrtIfFlowFilterEntryMoMgr::GetRenamedUncKey(
  ConfigKeyVal *ikey, upll_keytype_datatype_t dt_type, DalDmlIntf *dmi,
  uint8_t *ctrlr_id) {
  UPLL_FUNC_TRACE;

  if (ctrlr_id == NULL) {
    UPLL_LOG_DEBUG("ctrlr_id is NULL");
    return UPLL_RC_ERR_GENERIC;
  }

  ConfigKeyVal *unc_key = NULL;
  DbSubOp dbop = { kOpReadSingle, kOpMatchCtrlr, kOpInOutNone };
  MoMgrImpl *VrtIfMoMgr = static_cast<MoMgrImpl*>
    ((const_cast<MoManager*> (GetMoManager(UNC_KT_VROUTER))));

  val_rename_vnode *rename_val = reinterpret_cast<val_rename_vnode*>
                    (ConfigKeyVal::Malloc(sizeof(val_rename_vnode)));

  key_vrt_if_flowfilter_entry_t *ctrlr_key = reinterpret_cast
  <key_vrt_if_flowfilter_entry_t *> (ikey->get_key());

  if (NULL == ctrlr_key) {
    UPLL_LOG_DEBUG(" ctrlr_key is NULL");
    free(rename_val);
    return UPLL_RC_ERR_GENERIC;
  }

  uuu::upll_strncpy(
  rename_val->ctrlr_vtn_name,
  ctrlr_key->flowfilter_key.if_key.vrt_key.vtn_key.vtn_name,
  (kMaxLenVtnName + 1));
  //  rename_val->valid[0] = UNC_VF_VALID; //  TODO(UNC):for vrouter_name
  uuu::upll_strncpy(rename_val->ctrlr_vnode_name,
                    ctrlr_key->flowfilter_key.if_key.vrt_key.vrouter_name,
                    (kMaxLenVnodeName + 1));
  // rename_flowlist->valid[0] = UNC_VF_VALID;
  VrtIfMoMgr->GetChildConfigKey(unc_key, NULL);
  if (NULL == unc_key) {
    UPLL_LOG_DEBUG(" unc_key is NULL");
    free(rename_val);
    return UPLL_RC_ERR_GENERIC;
  }

  unc_key->set_user_data(ctrlr_id);
  unc_key->AppendCfgVal(IpctSt::kIpcStValRenameVtn, rename_val);
  upll_rc_t result_code = VrtIfMoMgr->ReadConfigDB(unc_key, dt_type,
  UNC_OP_READ, dbop, dmi, RENAMETBL);
  if (result_code == UPLL_RC_SUCCESS) {
    key_vrt_if_flowfilter_entry_t *key_vrt_if_flowfilter_entry =reinterpret_cast
    <key_vrt_if_flowfilter_entry_t *> (unc_key->get_key());
    uuu::upll_strncpy(
    ctrlr_key->flowfilter_key.if_key.vrt_key.vtn_key.vtn_name,
    key_vrt_if_flowfilter_entry->flowfilter_key.if_key.vrt_key.vtn_key.vtn_name,
    (kMaxLenVtnName + 1));
    uuu::upll_strncpy(
    ctrlr_key->flowfilter_key.if_key.vrt_key.vrouter_name,
    key_vrt_if_flowfilter_entry->flowfilter_key.if_key.vrt_key.vrouter_name,
    (kMaxLenVnodeName + 1));
  }
  delete unc_key;
  unc_key = NULL;
  val_rename_flowlist_t *rename_flowlist =
      reinterpret_cast<val_rename_flowlist_t*>
                   (ConfigKeyVal::Malloc(sizeof(val_rename_flowlist_t)));
  val_flowfilter_entry_t *val_flowfilter_entry = reinterpret_cast
                            <val_flowfilter_entry_t *> (GetVal(ikey));

  uuu::upll_strncpy(rename_flowlist->flowlist_newname,
                    val_flowfilter_entry->flowlist_name,
                    (kMaxLenFlowListName + 1));

  MoMgrImpl* mgr =static_cast<MoMgrImpl*>
  ((const_cast<MoManager*> (GetMoManager(UNC_KT_FLOWLIST))));
  result_code = mgr->GetChildConfigKey(unc_key, NULL);

  if (UPLL_RC_SUCCESS != result_code) {
    free(rename_val);
    free(rename_flowlist);  // RESOURCE LEAK FIX
    return result_code;
  }

  SET_USER_DATA_CTRLR(unc_key, ctrlr_id);
  unc_key->AppendCfgVal(IpctSt::kIpcStValRenameFlowlist, rename_flowlist);
  result_code = mgr->ReadConfigDB(unc_key, dt_type, UNC_OP_READ, dbop, dmi,
                                RENAMETBL);

  if (result_code == UPLL_RC_SUCCESS) {
    val_flowfilter_entry_t *temp_val_flowfilter_entry = reinterpret_cast
    <val_flowfilter_entry_t *>(unc_key->get_cfg_val()->get_val());

    uuu::upll_strncpy(val_flowfilter_entry->flowlist_name,
    temp_val_flowfilter_entry->flowlist_name,
    (kMaxLenFlowListName + 1));
  }
  UPLL_LOG_DEBUG("Key is filled with UncKey Successfully %d", result_code);
  free(rename_val);
  delete unc_key;
  return result_code;
}

upll_rc_t VrtIfFlowFilterEntryMoMgr::GetChildConfigKey(
    ConfigKeyVal *&okey, ConfigKeyVal *parent_key) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  key_vrt_if_flowfilter_entry_t *vrt_if_ffe_key;
  void *pkey = NULL;

  if (parent_key == NULL) {
    vrt_if_ffe_key = reinterpret_cast<key_vrt_if_flowfilter_entry_t*>
        (ConfigKeyVal::Malloc(sizeof(key_vrt_if_flowfilter_entry_t)));
    okey = new ConfigKeyVal(UNC_KT_VRTIF_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVrtIfFlowfilterEntry,
                            vrt_if_ffe_key, NULL);
    UPLL_LOG_DEBUG("Parent Key Filled ");
    return UPLL_RC_SUCCESS;
  } else {
    pkey = parent_key->get_key();
  }
  if (!pkey) return UPLL_RC_ERR_GENERIC;

  if (okey) {
    if (okey->get_key_type() != UNC_KT_VRTIF_FLOWFILTER_ENTRY)
      return UPLL_RC_ERR_GENERIC;
    vrt_if_ffe_key = reinterpret_cast<key_vrt_if_flowfilter_entry_t *>
        (okey->get_key());
  } else {
    vrt_if_ffe_key = reinterpret_cast<key_vrt_if_flowfilter_entry_t *>
        (ConfigKeyVal::Malloc(sizeof(key_vrt_if_flowfilter_entry_t)));
  }
  switch (parent_key->get_key_type()) {
    case UNC_KT_VTN:
      uuu::upll_strncpy(
          vrt_if_ffe_key->flowfilter_key.if_key.vrt_key.vtn_key.vtn_name,
          reinterpret_cast<key_vtn_t *>
          (pkey)->vtn_name,
          kMaxLenVtnName + 1);
      break;
    case UNC_KT_VROUTER:
      uuu::upll_strncpy(
          vrt_if_ffe_key->flowfilter_key.if_key.vrt_key.vtn_key.vtn_name,
          reinterpret_cast<key_vrt_t *>
          (pkey)->vtn_key.vtn_name,
          kMaxLenVtnName + 1);
      uuu::upll_strncpy(
          vrt_if_ffe_key->flowfilter_key.if_key.vrt_key.vrouter_name,
          reinterpret_cast<key_vrt_t *>
          (pkey)->vrouter_name,
          kMaxLenVnodeName + 1);
      break;
    case UNC_KT_VRT_IF:
      uuu::upll_strncpy(
          vrt_if_ffe_key->flowfilter_key.if_key.vrt_key.vtn_key.vtn_name,
          reinterpret_cast<key_vrt_if_t *>
          (pkey)->vrt_key.vtn_key.vtn_name,
          kMaxLenVtnName + 1);
      uuu::upll_strncpy(
          vrt_if_ffe_key->flowfilter_key.if_key.vrt_key.vrouter_name,
          reinterpret_cast<key_vrt_if_t *>
          (pkey)->vrt_key.vrouter_name,
          kMaxLenVnodeName + 1);
      uuu::upll_strncpy(vrt_if_ffe_key->flowfilter_key.if_key.if_name,
                        reinterpret_cast<key_vrt_if_t *>
                        (pkey)->if_name,
                        kMaxLenInterfaceName + 1);
      break;
    case UNC_KT_VRTIF_FLOWFILTER:
      uuu::upll_strncpy(
          vrt_if_ffe_key->flowfilter_key.if_key.vrt_key.vtn_key.vtn_name,
          reinterpret_cast<key_vrt_if_flowfilter_t *>
          (pkey)->if_key.vrt_key.vtn_key.vtn_name,
          kMaxLenVtnName + 1);
      uuu::upll_strncpy(
          vrt_if_ffe_key->flowfilter_key.if_key.vrt_key.vrouter_name,
          reinterpret_cast<key_vrt_if_flowfilter_t *>
          (pkey)->if_key.vrt_key.vrouter_name,
          kMaxLenVnodeName + 1);
      uuu::upll_strncpy(vrt_if_ffe_key->flowfilter_key.if_key.if_name,
                        reinterpret_cast<key_vrt_if_flowfilter_t *>
                        (pkey)->if_key.if_name,
                        kMaxLenInterfaceName + 1);
      vrt_if_ffe_key->flowfilter_key.direction =
          reinterpret_cast<key_vrt_if_flowfilter_t *>
          (pkey)->direction;
      break;
    case UNC_KT_VRTIF_FLOWFILTER_ENTRY:
      uuu::upll_strncpy(
          vrt_if_ffe_key->flowfilter_key.if_key.vrt_key.vtn_key.vtn_name,
          reinterpret_cast<key_vrt_if_flowfilter_entry_t *>
          (pkey)->flowfilter_key.if_key.vrt_key.vtn_key.vtn_name,
          kMaxLenVtnName + 1);
      uuu::upll_strncpy(
          vrt_if_ffe_key->flowfilter_key.if_key.vrt_key.vrouter_name,
          reinterpret_cast<key_vrt_if_flowfilter_entry_t *>
          (pkey)->flowfilter_key.if_key.vrt_key.vrouter_name,
          kMaxLenVnodeName + 1);
      uuu::upll_strncpy(vrt_if_ffe_key->flowfilter_key.if_key.if_name,
                        reinterpret_cast<key_vrt_if_flowfilter_entry_t *>
                        (pkey)->flowfilter_key.if_key.if_name,
                        kMaxLenInterfaceName + 1);
      vrt_if_ffe_key->flowfilter_key.direction =
          reinterpret_cast<key_vrt_if_flowfilter_entry_t *>
          (pkey)->flowfilter_key.direction;
      vrt_if_ffe_key->sequence_num =
          reinterpret_cast<key_vrt_if_flowfilter_entry_t *>
          (pkey)->sequence_num;
      break;
    default:
      if (vrt_if_ffe_key) free(vrt_if_ffe_key);
      return UPLL_RC_ERR_GENERIC;
  }

  if (!okey) {
    okey = new ConfigKeyVal(UNC_KT_VRTIF_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVrtIfFlowfilterEntry,
                            vrt_if_ffe_key, NULL);
  }
  SET_USER_DATA(okey, parent_key);
  UPLL_LOG_DEBUG("GetChildConfigKey :: okey filled Succesfully ");
  return result_code;
}

upll_rc_t VrtIfFlowFilterEntryMoMgr::MergeValidate(unc_key_type_t keytype,
                                                   const char *ctrlr_id,
                                                   ConfigKeyVal *ikey,
                                                   DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  DbSubOp dbop = { kOpReadMultiple, kOpMatchNone, kOpInOutNone };
  ConfigKeyVal *tkey;
  ConfigKeyVal *keyval = NULL;

  if (NULL == ikey) {
    UPLL_LOG_DEBUG("GetChildConfigKey::ikey is Null");
    return UPLL_RC_ERR_BAD_REQUEST;
  }
  result_code = ReadConfigDB(ikey, UPLL_DT_IMPORT, UNC_OP_READ, dbop, dmi,
                           MAINTBL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("ReadConfigDB is not Success");
    return result_code;
  }
  MoMgrImpl *mgr = static_cast<MoMgrImpl*>
       ((const_cast<MoManager*> (GetMoManager(UNC_KT_FLOWLIST))));

  tkey = ikey;
  while (ikey != NULL) {
  result_code = mgr->GetChildConfigKey(keyval, NULL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetChildConfigKey is not Success");
    return result_code;
  }

  val_flowfilter_entry_t *flowfilter_val = reinterpret_cast
  <val_flowfilter_entry_t *>((GetVal(ikey)));
  key_flowlist_t *key_flowlist = reinterpret_cast <key_flowlist_t *>
  (keyval->get_key());

  uuu::upll_strncpy(key_flowlist->flowlist_name,
                    flowfilter_val->flowlist_name,
                    (kMaxLenFlowListName + 1));

  result_code = mgr->UpdateConfigDB(keyval, UPLL_DT_CANDIDATE, UNC_OP_READ, dmi,
                                    MAINTBL);
  if (result_code != UPLL_RC_ERR_INSTANCE_EXISTS) {
    return UPLL_RC_ERR_MERGE_CONFLICT;
  }
  ikey = tkey->get_next_cfg_key_val();
  }
  if (tkey)
    delete tkey;

  UPLL_LOG_DEBUG("MergeValidate  is Success.");
  return result_code;
}

upll_rc_t VrtIfFlowFilterEntryMoMgr::DupConfigKeyVal(ConfigKeyVal *&okey,
                                                     ConfigKeyVal *&req,
                                                     MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  if (req == NULL) {
    UPLL_LOG_DEBUG("Request is null");
    return UPLL_RC_ERR_GENERIC;
  }
  if (okey != NULL) {
    UPLL_LOG_DEBUG("oKey already Contains Data");
    return UPLL_RC_ERR_GENERIC;
  }
  if (req->get_key_type() != UNC_KT_VRTIF_FLOWFILTER_ENTRY) {
     UPLL_LOG_DEBUG(" DupConfigKeyval Failed.");
     return UPLL_RC_ERR_GENERIC;
  }
  ConfigVal *tmp1 = NULL, *tmp = (req)->get_cfg_val();
  if (tmp) {
    if (tbl == MAINTBL) {
      val_flowfilter_entry_t *ival =
          reinterpret_cast <val_flowfilter_entry_t *>(GetVal(req));
      if (ival != NULL) {
        val_flowfilter_entry_t *val_ffe =
            reinterpret_cast<val_flowfilter_entry_t *>
            (ConfigKeyVal::Malloc(sizeof(val_flowfilter_entry_t)));

        memcpy(val_ffe, ival, sizeof(val_flowfilter_entry_t));
        tmp1 = new ConfigVal(IpctSt::kIpcStValFlowfilterEntry,
                             val_ffe);
      }
    }
  }

  void *tkey = (req != NULL) ? (req)->get_key() : NULL;
  key_vrt_if_flowfilter_entry_t *ikey =
      reinterpret_cast<key_vrt_if_flowfilter_entry_t *>(tkey);
  key_vrt_if_flowfilter_entry_t *key_vrt_if_ffe =
     reinterpret_cast<key_vrt_if_flowfilter_entry_t*>
        (ConfigKeyVal::Malloc(sizeof(key_vrt_if_flowfilter_entry_t)));

  memcpy(key_vrt_if_ffe, ikey, sizeof(key_vrt_if_flowfilter_entry_t));
  okey = new ConfigKeyVal(UNC_KT_VRTIF_FLOWFILTER_ENTRY,
                        IpctSt::kIpcStKeyVrtIfFlowfilterEntry,
                        key_vrt_if_ffe, tmp1);
  if (okey) {
    SET_USER_DATA(okey, req);
  }
  UPLL_LOG_DEBUG("DupConfigkeyVal Succesfull.");
  return UPLL_RC_SUCCESS;
}

upll_rc_t VrtIfFlowFilterEntryMoMgr::ReadMo(IpcReqRespHeader *req,
                                            ConfigKeyVal *ikey,
                                            DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  controller_domain ctrlr_dom;
  ConfigKeyVal* l_key = NULL, *dup_key = NULL;
  DbSubOp dbop1 = { kOpReadSingle, kOpMatchNone,
    kOpInOutCtrlr|kOpInOutDomain|kOpInOutFlag };
  result_code = ValidateMessage(req, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("ValidateMessage failed result_code %d", result_code);
    return result_code;
  }

  memset(&ctrlr_dom, 0, sizeof(controller_domain));
  switch (req->datatype) {
    // Retrieving config information
    case UPLL_DT_CANDIDATE:
    case UPLL_DT_RUNNING:
    case UPLL_DT_STARTUP:
    case UPLL_DT_STATE:
      if (req->option1 == UNC_OPT1_NORMAL) {
        result_code = ReadInfoFromDB(req, ikey, dmi, &ctrlr_dom);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG(" Read request failed result(%d)", result_code);
          return result_code;
        }
        // Retrieving state information
      } else if ((req->datatype == UPLL_DT_STATE) &&
                 (req->option1 == UNC_OPT1_DETAIL)&&
                 (req->option2 == UNC_OPT2_NONE)) {
        result_code =  DupConfigKeyVal(dup_key, ikey, MAINTBL);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("DupConfigKeyVal Faill in ReadMo for dup_key");
          return result_code;
        }

        result_code = ReadConfigDB(dup_key, req->datatype,
                                   UNC_OP_READ, dbop1, dmi, MAINTBL);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("ReadConfigDB  Faill in ReadMo for dup_key");
          delete dup_key;
          return result_code;
        }
        result_code =  DupConfigKeyVal(l_key, ikey, MAINTBL);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("DupConfigKeyVal Faill in ReadMo for l_key");
          return result_code;
        }
        GET_USER_DATA_CTRLR_DOMAIN(dup_key, ctrlr_dom);
        SET_USER_DATA_CTRLR_DOMAIN(l_key, ctrlr_dom);
        // 1.Getting renamed name if renamed
        result_code = GetRenamedControllerKey(l_key, req->datatype,
                                              dmi, &ctrlr_dom);
        if (result_code != UPLL_RC_SUCCESS) {
          return result_code;
        }

        uint8_t vlink_flag = 0;
        GET_USER_DATA_FLAGS(dup_key, vlink_flag);
        if (!(SET_FLAG_VLINK & vlink_flag)) {
          UPLL_LOG_DEBUG("Vlink Not Configured");
          return UPLL_RC_ERR_NOT_ALLOWED_AT_THIS_TIME;
        }

        pfcdrv_val_flowfilter_entry_t *pfc_val =
            reinterpret_cast<pfcdrv_val_flowfilter_entry_t *>
            (ConfigKeyVal::Malloc(sizeof(pfcdrv_val_flowfilter_entry_t)));

        pfc_val->valid[PFCDRV_IDX_FLOWFILTER_ENTRY_FFE] = UNC_VF_INVALID;
        pfc_val->valid[PFCDRV_IDX_VAL_VBRIF_VEXTIF_FFE] = UNC_VF_VALID;
        pfc_val->val_vbrif_vextif.valid[PFCDRV_IDX_INTERFACE_TYPE] =
            UNC_VF_VALID;
        pfc_val->val_vbrif_vextif.valid[PFCDRV_IDX_VEXTERNAL_NAME_VBRIF] =
            UNC_VF_INVALID;
        pfc_val->val_vbrif_vextif.valid[PFCDRV_IDX_VEXT_IF_NAME_VBRIF] =
            UNC_VF_INVALID;
        pfc_val->val_vbrif_vextif.interface_type = PFCDRV_IF_TYPE_VBRIF;

        l_key->SetCfgVal(new ConfigVal(IpctSt::kIpcStPfcdrvValFlowfilterEntry,
                                       pfc_val));

        // 2.send request to driver
        IpcResponse ipc_resp;
        memset(&ipc_resp, 0, sizeof(IpcResponse));
        IpcRequest ipc_req;
        memset(&ipc_req, 0, sizeof(IpcRequest));
        ipc_req.header.clnt_sess_id = req->clnt_sess_id;
        ipc_req.header.config_id = req->config_id;
        ipc_req.header.operation = req->operation;
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
        ConfigKeyVal *okey = NULL;
        result_code = ConstructReadDetailResponse(dup_key,
                                                  ipc_resp.ckv_data,
                                                  dmi, &okey);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("ConstructReadDetailResponse error code (%d)",
                         result_code);
          return result_code;
        } else {
          if (okey != NULL) {
            ikey->ResetWith(okey);
          }
        }
        DELETE_IF_NOT_NULL(dup_key);
        DELETE_IF_NOT_NULL(l_key);
      }
      break;
    default:
      result_code = UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT;
  }
  return result_code;
}

upll_rc_t VrtIfFlowFilterEntryMoMgr::ReadSiblingMo(IpcReqRespHeader *req,
                                                   ConfigKeyVal *ikey,
                                                   bool begin,
                                                   DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  controller_domain ctrlr_dom;
  ConfigKeyVal *dup_key = NULL, *l_key = NULL, *tctrl_key = NULL;
  ConfigKeyVal *okey = NULL, *tmp_key = NULL, *flag_key = NULL;
  DbSubOp dbop = { kOpReadSingle, kOpMatchNone,
    kOpInOutCtrlr|kOpInOutDomain|kOpInOutFlag };

  result_code = ValidateMessage(req, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("ValidateMessage failed result_code %d",
                   result_code);
    return result_code;
  }


  memset(&ctrlr_dom, 0, sizeof(controller_domain));
  switch (req->datatype) {
    // Retrieving config information
    case UPLL_DT_CANDIDATE:
    case UPLL_DT_RUNNING:
    case UPLL_DT_STARTUP:
    case UPLL_DT_STATE:
      if (req->option1 == UNC_OPT1_NORMAL) {
        result_code = ReadInfoFromDB(req, ikey, dmi, &ctrlr_dom);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG(" Read request failed result(%d)", result_code);
        }
      } else if ((req->datatype == UPLL_DT_STATE) &&
                 (req->option1 == UNC_OPT1_DETAIL) &&
                 (req->option2 == UNC_OPT2_NONE)) {
        result_code =  DupConfigKeyVal(tctrl_key, ikey, MAINTBL);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("DupConfigKeyVal Failed for tctrl_key");
          return result_code;
        }
        result_code = ReadInfoFromDB(req, tctrl_key, dmi, &ctrlr_dom);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("ReadInfoFromDB Fail in ReadSiblingMo for tctrl_key");
          return result_code;
        }

        result_code =  DupConfigKeyVal(dup_key, tctrl_key, MAINTBL);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG(" DupConfigKeyVal failed for dup_key%d ", result_code);
          return result_code;
        }

        result_code = ReadConfigDB(dup_key, req->datatype, UNC_OP_READ,
                                   dbop, dmi, MAINTBL);
        if (UPLL_RC_SUCCESS != result_code) {
          UPLL_LOG_DEBUG("ReadConfigDb failed for tctrl_key err code(%d)",
                         result_code);
          return result_code;
        }

        result_code =  DupConfigKeyVal(l_key, ikey, MAINTBL);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("DupConfigKeyVal Faill in ReadSiblingMo for l_key");
          return result_code;
        }
        GET_USER_DATA_CTRLR_DOMAIN(dup_key, ctrlr_dom);
        SET_USER_DATA_CTRLR_DOMAIN(l_key, ctrlr_dom);
        // 1.Getting renamed name if renamed
        result_code = GetRenamedControllerKey(l_key, req->datatype,
                                              dmi, &ctrlr_dom);
        if (result_code != UPLL_RC_SUCCESS) {
          return result_code;
        }

        result_code =  DupConfigKeyVal(flag_key, tctrl_key, MAINTBL);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG(" DupConfigKeyVal failed for flag_key %d ",
                         result_code);
          return result_code;
        }

        DbSubOp dbop2 = { kOpReadSingle, kOpMatchNone, kOpInOutFlag };
        result_code = ReadConfigDB(flag_key, req->datatype ,
                                   UNC_OP_READ, dbop2, dmi, MAINTBL);
        if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
          UPLL_LOG_DEBUG("No Recrods in the Vrt_If_FlowFilter_Entry Table");
          return UPLL_RC_SUCCESS;
        }
        uint8_t vlink_flag = 0;
        GET_USER_DATA_FLAGS(flag_key, vlink_flag);
        if (!(SET_FLAG_VLINK & vlink_flag)) {
          UPLL_LOG_DEBUG("Vlink Not Configured");
          return UPLL_RC_ERR_NOT_ALLOWED_AT_THIS_TIME;
        }
        pfcdrv_val_flowfilter_entry_t *pfc_val =
            reinterpret_cast<pfcdrv_val_flowfilter_entry_t *>
            (ConfigKeyVal::Malloc(sizeof(pfcdrv_val_flowfilter_entry_t)));

        pfc_val->valid[PFCDRV_IDX_FLOWFILTER_ENTRY_FFE] = UNC_VF_INVALID;
        pfc_val->valid[PFCDRV_IDX_VAL_VBRIF_VEXTIF_FFE] = UNC_VF_VALID;
        pfc_val->val_vbrif_vextif.valid[PFCDRV_IDX_INTERFACE_TYPE] =
            UNC_VF_VALID;
        pfc_val->val_vbrif_vextif.valid[PFCDRV_IDX_VEXTERNAL_NAME_VBRIF] =
            UNC_VF_INVALID;
        pfc_val->val_vbrif_vextif.valid[PFCDRV_IDX_VEXT_IF_NAME_VBRIF] =
            UNC_VF_INVALID;
        pfc_val->val_vbrif_vextif.interface_type = PFCDRV_IF_TYPE_VBRIF;

        l_key->SetCfgVal(new ConfigVal(IpctSt::kIpcStPfcdrvValFlowfilterEntry,
                                       pfc_val));
        // 2.send request to driver
        IpcResponse ipc_resp;
        memset(&ipc_resp, 0, sizeof(IpcResponse));
        IpcRequest ipc_req;
        memset(&ipc_req, 0, sizeof(IpcRequest));
        ipc_req.header.clnt_sess_id = req->clnt_sess_id;
        ipc_req.header.config_id = req->config_id;
        ipc_req.header.operation = UNC_OP_READ;
        ipc_req.header.option1 = req->option1;
        ipc_req.header.datatype = req->datatype;
        tmp_key = tctrl_key;
        while (tmp_key !=NULL) {
          reinterpret_cast<key_vrt_if_flowfilter_entry_t*>
              (l_key->get_key())->flowfilter_key.direction =
              reinterpret_cast<key_vrt_if_flowfilter_entry_t*>
              (tmp_key->get_key())->flowfilter_key.direction;
          reinterpret_cast<key_vrt_if_flowfilter_entry_t*>
              (l_key->get_key())->sequence_num =
              reinterpret_cast<key_vrt_if_flowfilter_entry_t*>
              (tmp_key->get_key())->sequence_num;
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

          result_code = ConstructReadDetailResponse(tmp_key,
                                                    ipc_resp.ckv_data,
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
        DELETE_IF_NOT_NULL(l_key);
        DELETE_IF_NOT_NULL(tctrl_key);
      }
      break;
    default:
      result_code = UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT;
  }
  return result_code;
}

upll_rc_t VrtIfFlowFilterEntryMoMgr::UpdateConfigStatus(
  ConfigKeyVal *key, unc_keytype_operation_t op, uint32_t driver_result,
  ConfigKeyVal *upd_key, DalDmlIntf *dmi, ConfigKeyVal *ctrlr_key) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  val_flowfilter_entry_t *vrtif_ff_entry_val = NULL;
  unc_keytype_configstatus_t cs_status =
  (driver_result == 0) ? UNC_CS_APPLIED : UNC_CS_NOT_APPLIED;

  vrtif_ff_entry_val = reinterpret_cast<val_flowfilter_entry_t *>
                               (GetVal(key));

  if (vrtif_ff_entry_val == NULL) {
    UPLL_LOG_DEBUG("vrtif_ff_entry_val is Null");
    return UPLL_RC_ERR_GENERIC;
  }

  if (op == UNC_OP_CREATE) {
    if (vrtif_ff_entry_val->cs_row_status != UNC_CS_NOT_SUPPORTED)
        vrtif_ff_entry_val->cs_row_status = cs_status;
       for (unsigned int loop = 0;
        loop < (sizeof(vrtif_ff_entry_val->valid)/
               sizeof(vrtif_ff_entry_val->valid[0])); ++loop) {
        if ((UNC_VF_VALID == vrtif_ff_entry_val->valid[loop])
        || (UNC_VF_VALID_NO_VALUE == vrtif_ff_entry_val->valid[loop]))
          if (vrtif_ff_entry_val->cs_attr[loop] != UNC_CS_NOT_SUPPORTED)
            vrtif_ff_entry_val->cs_attr[loop] =
                            vrtif_ff_entry_val->cs_row_status;
     }
  } else if (op == UNC_OP_UPDATE) {
    void *fle_val1 = GetVal(key);
    void *fle_val2 = GetVal(upd_key);
    CompareValidValue(fle_val1, fle_val2, false);
    for (unsigned int loop = 0;
         loop < sizeof(vrtif_ff_entry_val->valid)/
                sizeof(vrtif_ff_entry_val->valid[0]); ++loop) {
      if (vrtif_ff_entry_val->cs_attr[loop] != UNC_CS_NOT_SUPPORTED)
      if ((UNC_VF_VALID == vrtif_ff_entry_val->valid[loop])
        ||(UNC_VF_VALID_NO_VALUE == vrtif_ff_entry_val->valid[loop]))
         //  if (CompareVal(vrtif_ff_entry_val, upd_key->GetVal()))
        vrtif_ff_entry_val->cs_attr[loop] =
        vrtif_ff_entry_val->cs_row_status;
    }
    } else {
      UPLL_LOG_DEBUG("Operation Not Supported.");
      return UPLL_RC_ERR_GENERIC;
    }
  UPLL_LOG_DEBUG("UpdateConfigStatus Success");
  return result_code;
}

bool VrtIfFlowFilterEntryMoMgr::IsValidKey(void *key,
                                           uint64_t index) {
  UPLL_FUNC_TRACE;
  bool ret_val = UPLL_RC_SUCCESS;
  key_vrt_if_flowfilter_entry_t  *ff_key =
      reinterpret_cast<key_vrt_if_flowfilter_entry_t *>(key);
  if (ff_key == NULL)
    return false;

  switch (index) {
    case uudst::vrt_if_flowfilter_entry::kDbiVtnName:
      ret_val = ValidateKey(
          reinterpret_cast<char *>
          (ff_key->flowfilter_key.if_key.vrt_key.vtn_key.vtn_name),
          kMinLenVtnName, kMaxLenVtnName);
      if (ret_val != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("VTN Name is not valid(%d)", ret_val);
        return false;
      }
      break;
    case uudst::vrt_if_flowfilter_entry::kDbiVrtName:
      ret_val = ValidateKey(reinterpret_cast<char *>
                        (ff_key->flowfilter_key.if_key.vrt_key.vrouter_name),
                        kMinLenVnodeName, kMaxLenVnodeName);
      if (ret_val != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("VRT Name is not valid(%d)", ret_val);
        return false;
      }
      break;
    case uudst::vrt_if_flowfilter_entry::kDbiVrtIfName:
      ret_val = ValidateKey(reinterpret_cast<char *>
                            (ff_key->flowfilter_key.if_key.if_name),
                            kMinLenInterfaceName, kMaxLenInterfaceName);
      if (ret_val != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("VRTIF  Name is not valid(%d)", ret_val);
        return false;
      }
      break;
    case uudst::vrt_if_flowfilter_entry::kDbiInputDirection:
      if (ff_key->flowfilter_key.direction == 0xFE) {
        // if operation is read sibling begin or
        // read sibling count return false
        // for output binding
        ff_key->flowfilter_key.direction = 0;
        return false;
       }
      if (!ValidateNumericRange(ff_key->flowfilter_key.direction,
                                (uint8_t) UPLL_FLOWFILTER_DIR_IN,
                                (uint8_t) UPLL_FLOWFILTER_DIR_OUT,
                                true, true)) {
        UPLL_LOG_DEBUG("direction syntax validation failed :");
        return false;
      }
      break;
     case uudst::vrt_if_flowfilter_entry::kDbiSequenceNum:
      if (!ValidateNumericRange(ff_key->sequence_num,
                                kMinFlowFilterSeqNum,
                                kMaxFlowFilterSeqNum,
                                true, true)) {
        UPLL_LOG_DEBUG("sequence number syntax validation failed");
        return false;
      }
      break;
    default:
      UPLL_LOG_DEBUG("Invalid Key Index");
      return false;
      break;
  }
  return true;
}


upll_rc_t VrtIfFlowFilterEntryMoMgr::IsReferenced(ConfigKeyVal *ikey,
                                           upll_keytype_datatype_t dt_type,
                                           DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_ERR_GENERIC;
  if (NULL == ikey) return UPLL_RC_ERR_BAD_REQUEST;
  DbSubOp dbop = { kOpReadExist, kOpMatchNone, kOpInOutNone };
  // Check the object existence
  result_code = UpdateConfigDB(ikey, dt_type, UNC_OP_READ, dmi, &dbop, MAINTBL);
  return result_code;
}


upll_rc_t VrtIfFlowFilterEntryMoMgr::ValidateAttribute(ConfigKeyVal *ikey,
                                                       DalDmlIntf *dmi,
                                                       IpcReqRespHeader *req) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  ConfigKeyVal *okey = NULL;
  if (!ikey || !ikey->get_key()) {
    UPLL_LOG_DEBUG("input key is null");
    return UPLL_RC_ERR_GENERIC;
  }

  key_vrt_if_flowfilter_entry_t *key_vrtif_ffe =
      reinterpret_cast<key_vrt_if_flowfilter_entry_t *>(ikey->get_key());
  MoMgrImpl *mgr = reinterpret_cast<MoMgrImpl *>
      (const_cast<MoManager *>(GetMoManager(UNC_KT_VRTIF_FLOWFILTER)));

  result_code = mgr->GetChildConfigKey(okey, NULL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetChildConfigKey failed - %d", result_code);
    return result_code;
  }

  key_vrt_if_flowfilter_t *vrtif_ff_key =
      reinterpret_cast<key_vrt_if_flowfilter_t *>(okey->get_key());

  uuu::upll_strncpy(vrtif_ff_key->if_key.vrt_key.vtn_key.vtn_name,
        key_vrtif_ffe->flowfilter_key.if_key.vrt_key.vtn_key.vtn_name,
        kMaxLenVtnName + 1);

  uuu::upll_strncpy(vrtif_ff_key->if_key.vrt_key.vrouter_name,
        key_vrtif_ffe->flowfilter_key.if_key.vrt_key.vrouter_name,
        kMaxLenVnodeName + 1);

  uuu::upll_strncpy(vrtif_ff_key->if_key.if_name,
        key_vrtif_ffe->flowfilter_key.if_key.if_name,
        kMaxLenInterfaceName + 1);

  vrtif_ff_key->direction = key_vrtif_ffe->flowfilter_key.direction;

  /* Checks the given vrt_if_flowfilter exists in DB or not */
  result_code = mgr->UpdateConfigDB(okey, req->datatype, UNC_OP_READ,
                                    dmi, MAINTBL);

  if (UPLL_RC_ERR_INSTANCE_EXISTS != result_code) {
    UPLL_LOG_DEBUG(" Parent VRT_IF_FLOWFILTER key does not exists");
    delete okey;
    okey = NULL;
    return UPLL_RC_ERR_PARENT_DOES_NOT_EXIST;
  } else {
    result_code = UPLL_RC_SUCCESS;
  }

  delete okey;
  okey = NULL;

  /* read val_flowfilter_entry from ikey*/
  val_flowfilter_entry_t *val_flowfilter_entry =
    static_cast<val_flowfilter_entry_t *>(
        ikey->get_cfg_val()->get_val());

  if (val_flowfilter_entry->valid[UPLL_IDX_FLOWLIST_NAME_FFE]
        == UNC_VF_VALID) {
  /* validate flowlist_name in val_flowfilter_entry exists in FLOWLIST table*/
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

  /** fill key_flowlist_t from val_flowfilter_entry*/
  key_flowlist_t *key_flowlist = static_cast<key_flowlist_t*>(
    okey->get_key());
  uuu::upll_strncpy(key_flowlist->flowlist_name,
    val_flowfilter_entry->flowlist_name,
    kMaxLenFlowListName+1);

  UPLL_LOG_TRACE("Flowlist name in val_flowfilter_entry %s",
                 key_flowlist->flowlist_name);

  /* Check flowlist_name exists in table*/
  result_code = mgr->UpdateConfigDB(okey, req->datatype,
                                     UNC_OP_READ, dmi, MAINTBL);

  if (UPLL_RC_ERR_INSTANCE_EXISTS != result_code) {
    UPLL_LOG_DEBUG("Flowlist name in val_flowfilter_entry does not exists"
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

  if (val_flowfilter_entry->valid[UPLL_IDX_NWM_NAME_FFE]
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

    /** fill key_nwm from key/val VRTIF_FLOWFILTER_ENTRY structs*/
    key_nwm_t *key_nwm = static_cast<key_nwm_t*>(
      okey->get_key());

    uuu::upll_strncpy(key_nwm->nwmonitor_name,
      val_flowfilter_entry->nwm_name,
      kMaxLenVnodeName+1);

    uuu::upll_strncpy(key_nwm->vbr_key.vtn_key.vtn_name,
      key_vrtif_ffe->flowfilter_key.if_key.vrt_key.vtn_key.vtn_name,
      kMaxLenVtnName+1);

    /* Check nwm_name exists in table*/
    DbSubOp dbop = {kOpReadSingle, kOpMatchNone, kOpInOutNone};
    result_code = mgr->ReadConfigDB(okey, req->datatype, UNC_OP_READ,
                                     dbop, dmi, MAINTBL);

    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("NWM name in val_flowfilter_entry does not exists"
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

upll_rc_t VrtIfFlowFilterEntryMoMgr::AllocVal(ConfigVal *&ck_val,
                                              upll_keytype_datatype_t dt_type,
                                              MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  void *val;  //   *ck_nxtval;
  if (ck_val != NULL) {
    UPLL_LOG_DEBUG("ck_val Consist the Value");
    return UPLL_RC_ERR_GENERIC;
  }
  if (tbl == MAINTBL) {
    val  = reinterpret_cast <void *>(
        ConfigKeyVal::Malloc(sizeof(val_flowfilter_entry_t)));
    ck_val = new ConfigVal(IpctSt::kIpcStValFlowfilterEntry, val);
  } else {
    val = NULL;
    return UPLL_RC_ERR_GENERIC;
  }

  UPLL_LOG_DEBUG("AllocVal Success");
  return UPLL_RC_SUCCESS;
}

upll_rc_t VrtIfFlowFilterEntryMoMgr::GetValid(void *val, uint64_t indx,
                                            uint8_t *&valid,
                                            upll_keytype_datatype_t dt_type,
                                            MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  if (val == NULL) {
    UPLL_LOG_DEBUG("Memory is not Allocated");
    return UPLL_RC_ERR_GENERIC;
  }

  if (tbl == MAINTBL) {
    switch (indx) {
      case uudst::vrt_if_flowfilter_entry::kDbiFlowlistName:
        valid = &(reinterpret_cast<val_flowfilter_entry *>(val))->\
                valid[UPLL_IDX_FLOWLIST_NAME_FFE];
        break;
      case uudst::vrt_if_flowfilter_entry::kDbiAction:
        valid = &(reinterpret_cast<val_flowfilter_entry *>(val))->\
                valid[UPLL_IDX_ACTION_FFE];
        break;
      case uudst::vrt_if_flowfilter_entry::kDbiRedirectNode:
        valid = &(reinterpret_cast<val_flowfilter_entry *>(val))->\
                valid[UPLL_IDX_REDIRECT_NODE_FFE];
        break;
      case uudst::vrt_if_flowfilter_entry::kDbiRedirectPort:
        valid = &(reinterpret_cast<val_flowfilter_entry *>(val))->\
                valid[UPLL_IDX_REDIRECT_PORT_FFE];
        break;
      case uudst::vrt_if_flowfilter_entry::kDbiModifyDstMac:
        valid = &(reinterpret_cast<val_flowfilter_entry *>(val))->\
                valid[UPLL_IDX_MODIFY_DST_MAC_FFE];
        break;
      case uudst::vrt_if_flowfilter_entry::kDbiModifySrcMac:
        valid = &(reinterpret_cast<val_flowfilter_entry *>(val))->\
                valid[UPLL_IDX_MODIFY_SRC_MAC_FFE];
        break;
      case uudst::vrt_if_flowfilter_entry::kDbiNwmName:
        valid = &(reinterpret_cast<val_flowfilter_entry *>(val)->\
                  valid[UPLL_IDX_NWM_NAME_FFE]);
        break;
      case uudst::vrt_if_flowfilter_entry::kDbiDscp:
        valid = &(reinterpret_cast<val_flowfilter_entry *>(val))->\
                valid[UPLL_IDX_DSCP_FFE];
        break;
      case uudst::vrt_if_flowfilter_entry::kDbiPriority:
        valid = &(reinterpret_cast<val_flowfilter_entry *>(val))->\
                valid[UPLL_IDX_PRIORITY_FFE];
        break;
      default:
        return UPLL_RC_ERR_GENERIC;
    }
  } else {
    UPLL_LOG_DEBUG("Invalid Tbl");
    return UPLL_RC_ERR_GENERIC;
  }
  UPLL_LOG_DEBUG("GetValidAttributte is Succesfull");
  return UPLL_RC_SUCCESS;
}
upll_rc_t VrtIfFlowFilterEntryMoMgr::ValidateMessage(IpcReqRespHeader *req,
                                                        ConfigKeyVal *key) {
  UPLL_FUNC_TRACE;

  upll_rc_t rt_code = UPLL_RC_ERR_GENERIC;

  if ((NULL == req) || (NULL == key)) {
    UPLL_LOG_DEBUG("ConfigKeyval is NULL");
    return UPLL_RC_ERR_BAD_REQUEST;
  }

  if (UNC_KT_VRTIF_FLOWFILTER_ENTRY != key->get_key_type()) {
    UPLL_LOG_DEBUG(" Invalid keytype(%d)", key->get_key_type());
    return UPLL_RC_ERR_BAD_REQUEST;
  }
  if (key->get_st_num() != IpctSt::kIpcStKeyVrtIfFlowfilterEntry) {
     UPLL_LOG_DEBUG("Invalid key structure received. received struct num - %d",
                  key->get_st_num());
    return UPLL_RC_ERR_BAD_REQUEST;
  }
  if(req->option2 != UNC_OPT2_NONE) {
    UPLL_LOG_DEBUG(" Error: option2 is not NONE");
    return UPLL_RC_ERR_INVALID_OPTION2;
  }
   if((req->option1 != UNC_OPT1_NORMAL) 
              &&(req->option1 != UNC_OPT1_DETAIL)) {
     UPLL_LOG_DEBUG(" Error: option1 is not NORMAL");
     return UPLL_RC_ERR_INVALID_OPTION1;
   }
   if((req->option1 != UNC_OPT1_NORMAL) 
              &&(req->operation == UNC_OP_READ_SIBLING_COUNT)) {
     UPLL_LOG_DEBUG(" Error: option1 is not NORMAL for ReadSiblingCount");
     return UPLL_RC_ERR_INVALID_OPTION1;
   }

  /** Read key structure */
  key_vrt_if_flowfilter_entry_t *key_vrt_if_flowfilter_entry =
      reinterpret_cast<key_vrt_if_flowfilter_entry_t *>(key->get_key());

  /** Validate key structure */
  if (NULL == key_vrt_if_flowfilter_entry) {
    UPLL_LOG_DEBUG("KT_VRTIF_FLOWFILTER_ENTRY Key structure is empty!!");
    return UPLL_RC_ERR_BAD_REQUEST;
  }

  rt_code = ValidateVrtIfFlowfilterEntryKey(key_vrt_if_flowfilter_entry,
                                              req->operation);

  if (UPLL_RC_SUCCESS != rt_code) {
    UPLL_LOG_DEBUG(" key_vrtif_flowfilter syntax validation failed :"
                   "Err Code - %d",
                   rt_code);
    return rt_code;
  }

  /* validate value structure*/
  if (!key->get_cfg_val()) {
    if ((req->operation == UNC_OP_UPDATE) ||
         (req->operation == UNC_OP_CREATE)) {
       UPLL_LOG_DEBUG("val structure is mandatory");
       return UPLL_RC_ERR_BAD_REQUEST;
    } else {
      UPLL_LOG_TRACE("val structure is optional");
      return UPLL_RC_SUCCESS;
    }
  }

  if (key->get_cfg_val()->get_st_num() !=
                             IpctSt::kIpcStValFlowfilterEntry) {
     UPLL_LOG_DEBUG("Invalid val structure received. struct num - %d",
                   (key->get_cfg_val())->get_st_num());
    return UPLL_RC_ERR_BAD_REQUEST;
  }

  val_flowfilter_entry_t *val_flowfilter_entry =
      reinterpret_cast<val_flowfilter_entry_t *>(
          key->get_cfg_val()->get_val());

  if (NULL == val_flowfilter_entry) {
    UPLL_LOG_DEBUG("val_flowfilter_entry structure is null");
    return UPLL_RC_ERR_BAD_REQUEST;
  }

  // UpdateMo invokes val structure validate function
  // as UPDATE operation requires dmi
  if (req->operation == UNC_OP_UPDATE)
    return UPLL_RC_SUCCESS;

  return VbrFlowFilterEntryMoMgr::ValidateFlowfilterEntryValue(
         val_flowfilter_entry, req->operation);
}

upll_rc_t VrtIfFlowFilterEntryMoMgr::ValidateVrtIfValStruct(
    IpcReqRespHeader *req,
    ConfigKeyVal *key,
    DalDmlIntf *dmi) {
  upll_rc_t result_code = UPLL_RC_ERR_GENERIC;

  val_flowfilter_entry_t *val_flowfilter_entry =
      reinterpret_cast<val_flowfilter_entry_t *>(
          key->get_cfg_val()->get_val());

  bool db_action_valid = false;
  bool db_action_redirect = false;

  if ((val_flowfilter_entry->valid[UPLL_IDX_ACTION_FFE]
       == UNC_VF_INVALID) && (req->operation == UNC_OP_UPDATE)) {
    /** Read key struct from ConfigKeyVal argument*/
    key_vrt_if_flowfilter_entry_t *key_vrt_if_ffe =
        static_cast<key_vrt_if_flowfilter_entry_t*>(key->get_key());

    /** Check whether Action configured or not from DB */
    ConfigKeyVal *okey = NULL;

    result_code = GetChildConfigKey(okey, NULL);

    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("okey memory allocation failed- %d", result_code);
      return result_code;
    }

    key_vrt_if_flowfilter_entry_t *vrt_if_ffe_key =
        reinterpret_cast<key_vrt_if_flowfilter_entry_t *>(okey->get_key());

    /* copy key structure into okey key struct */
    uuu::upll_strncpy(
        vrt_if_ffe_key->flowfilter_key.if_key.vrt_key.vtn_key.vtn_name,
        key_vrt_if_ffe->flowfilter_key.if_key.vrt_key.vtn_key.vtn_name,
        (kMaxLenVtnName+1));
    uuu::upll_strncpy(
        vrt_if_ffe_key->flowfilter_key.if_key.vrt_key.vrouter_name,
        key_vrt_if_ffe->flowfilter_key.if_key.vrt_key.vrouter_name,
        (kMaxLenVnodeName+1));
    uuu::upll_strncpy(vrt_if_ffe_key->flowfilter_key.if_key.if_name,
                      key_vrt_if_ffe->flowfilter_key.if_key.if_name,
                      (kMaxLenInterfaceName+1));
    vrt_if_ffe_key->flowfilter_key.direction =
        key_vrt_if_ffe->flowfilter_key.direction;
    vrt_if_ffe_key->sequence_num = key_vrt_if_ffe->sequence_num;

    /* Check the action field configured in VBR_FLOWFILTER_ENTRY table*/
    DbSubOp dbop = {kOpReadSingle, kOpMatchNone, kOpInOutNone};
    result_code = ReadConfigDB(okey, req->datatype, UNC_OP_READ,
                               dbop, dmi, MAINTBL);

    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("ReadConfigDB is failed for VRT_IF_FLOWFILTER_ENTRY");
      delete okey;
      okey = NULL;
      return UPLL_RC_ERR_NO_SUCH_INSTANCE;
    }

    /* check the action value from the DB data */
    val_flowfilter_entry_t *val_ffe =
        reinterpret_cast<val_flowfilter_entry_t *>(
            okey->get_cfg_val()->get_val());

    if (val_ffe->valid[UPLL_IDX_ACTION_FFE] == UNC_VF_VALID) {
      db_action_valid = true;
      if (val_ffe->action == UPLL_FLOWFILTER_ACT_REDIRECT) {
        db_action_redirect = true;
      }
    }

    delete okey;
    okey = NULL;
  }

  /** validate val_flowfilter_entry value structure */
  return VbrFlowFilterEntryMoMgr::ValidateFlowfilterEntryValue(
      val_flowfilter_entry, req->operation,
      db_action_valid, db_action_redirect);
}

upll_rc_t VrtIfFlowFilterEntryMoMgr::ValidateVrtIfFlowfilterEntryKey(
    key_vrt_if_flowfilter_entry_t* key_vrt_if_flowfilter_entry,
    unc_keytype_operation_t operation) {

  UPLL_FUNC_TRACE;
  upll_rc_t rt_code = UPLL_RC_ERR_GENERIC;

  VrtIfMoMgr *mgrvrtif = reinterpret_cast<VrtIfMoMgr *>(
      const_cast<MoManager*>(GetMoManager(UNC_KT_VRT_IF)));

  if (NULL == mgrvrtif) {
    UPLL_LOG_DEBUG("unable to get VtnMoMgr object to validate key_vtn");
    return UPLL_RC_ERR_GENERIC;
  }

  rt_code = mgrvrtif->ValidateVrtIfKey(
      &(key_vrt_if_flowfilter_entry->flowfilter_key.if_key));

  if (UPLL_RC_SUCCESS != rt_code) {
    UPLL_LOG_DEBUG(" Vrtif_key syntax validation failed :Err Code - %d",
                   rt_code);

    return rt_code;
  }


  /** validate direction */
  if (!ValidateNumericRange(
          key_vrt_if_flowfilter_entry->flowfilter_key.direction,
          (uint8_t) UPLL_FLOWFILTER_DIR_IN,
          (uint8_t) UPLL_FLOWFILTER_DIR_OUT,
          true, true)) {
    UPLL_LOG_DEBUG("direction syntax validation failed ");
    return UPLL_RC_ERR_CFG_SYNTAX;
  }

  if ((operation != UNC_OP_READ_SIBLING_COUNT) &&
      (operation != UNC_OP_READ_SIBLING_BEGIN)) {
    /** validate Sequence number */
    if (!ValidateNumericRange(key_vrt_if_flowfilter_entry->sequence_num,
                              kMinFlowFilterSeqNum, kMaxFlowFilterSeqNum, true,
                              true)) {
      UPLL_LOG_DEBUG("Sequence number validation failed ");
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  } else {
    key_vrt_if_flowfilter_entry->sequence_num = 0;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VrtIfFlowFilterEntryMoMgr::ValidateCapability(IpcReqRespHeader *req,
                                   ConfigKeyVal *ikey,
                                   const char* ctrlr_name) {
  UPLL_FUNC_TRACE;

  upll_rc_t rt_code = UPLL_RC_ERR_GENERIC;

  /** Use  VbrFlowfilterEntryMoMgr::ValidateCapability
   *  to validate capability for val_flowfilter_entry structure*/
  VbrFlowFilterEntryMoMgr *mgrvbrff =
      reinterpret_cast<VbrFlowFilterEntryMoMgr *>(
        const_cast<MoManager *>(GetMoManager(UNC_KT_VBR_FLOWFILTER_ENTRY)));
  rt_code = mgrvbrff->ValidateCapability(req, ikey);

  if (UPLL_RC_SUCCESS != rt_code) {
    UPLL_LOG_DEBUG(" VRTIF_FLOWFILTER_ENTRY Attribute validation failed :"
                  "Err Code - %d",
                  rt_code);
  }
  return rt_code;
}
bool VrtIfFlowFilterEntryMoMgr::GetRenameKeyBindInfo(unc_key_type_t key_type,
                                    BindInfo *&binfo,
                                    int &nattr,
                                    MoMgrTables tbl ) {
  /* Main Table only update */
  if (MAINTBL == tbl) {
    nattr = NUM_KEY_MAIN_TBL_;
    binfo = vrt_if_flowfilter_entry_maintbl_bind_info;
  }

  if (UNC_KT_FLOWLIST == key_type) {
    nattr = NUM_KEY_RENAME_MAIN_TBL;
    binfo = vrt_if_flowlist_rename_bind_info;
  }

  UPLL_LOG_DEBUG("Successful Completeion");
  return PFC_TRUE;
}
upll_rc_t  VrtIfFlowFilterEntryMoMgr::CopyToConfigKey(ConfigKeyVal *&okey,
                                              ConfigKeyVal *ikey) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  if (!ikey || !(ikey->get_key())) {
    UPLL_LOG_DEBUG("Input Key Not Valid");
    return UPLL_RC_ERR_GENERIC;
  }

  key_rename_vnode_info *key_rename = reinterpret_cast<key_rename_vnode_info *>
                                      (ikey->get_key());
  key_vrt_if_flowfilter_entry_t *key_vrt_if =
      reinterpret_cast<key_vrt_if_flowfilter_entry_t*>
      (ConfigKeyVal::Malloc(sizeof(key_vrt_if_flowfilter_entry_t)));
  if (UNC_KT_VRTIF_FLOWFILTER_ENTRY  == ikey->get_key_type()) {
  if (!strlen(reinterpret_cast<char *>(key_rename->old_unc_vtn_name))) {
    UPLL_LOG_DEBUG("String Length not Valid to Perform the Operation");
    free(key_vrt_if);  // COV:RESOURCE ELAK
    return UPLL_RC_ERR_GENERIC;
  }

  uuu::upll_strncpy(key_vrt_if->flowfilter_key.if_key.vrt_key.vtn_key.vtn_name,
                    key_rename->old_unc_vtn_name,
                    (kMaxLenVtnName + 1));


    if (!strlen(reinterpret_cast<char *> (key_rename->old_unc_vnode_name))) {
      free(key_vrt_if);  // COV:RESOURCE ELAK
      return UPLL_RC_ERR_GENERIC;
  }
  uuu::upll_strncpy(key_vrt_if->flowfilter_key.if_key.vrt_key.vrouter_name,
                      key_rename->old_unc_vnode_name,
                      (kMaxLenVnodeName + 1));


  okey = new ConfigKeyVal(UNC_KT_VRTIF_FLOWFILTER_ENTRY, IpctSt::
                   kIpcStKeyVrtIfFlowfilterEntry, key_vrt_if, NULL);
  }
  if (UNC_KT_FLOWLIST == ikey->get_key_type()) {
    key_rename_vnode_info_t *key_rename =
      reinterpret_cast<key_rename_vnode_info_t *>(ikey->get_key());
    val_flowfilter_entry_t *val = reinterpret_cast<val_flowfilter_entry_t*>
                   (ConfigKeyVal::Malloc(sizeof(val_flowfilter_entry_t)));

    uuu::upll_strncpy(val->flowlist_name,
                      key_rename->old_flowlist_name,
                      (kMaxLenFlowListName+1));

    val->valid[UPLL_IDX_FLOWLIST_NAME_FFE] = UNC_VF_VALID;
    ConfigVal *ckv = new ConfigVal(IpctSt::kIpcStValFlowfilterEntry, val);
    okey = new ConfigKeyVal(UNC_KT_VRTIF_FLOWFILTER_ENTRY,
        IpctSt::kIpcStKeyVrtIfFlowfilterEntry, key_vrt_if, ckv);
    if (!okey)
      free(val);
  }

  if (!okey) {
    free(key_vrt_if);
    return UPLL_RC_ERR_GENERIC;
  }
  return result_code;
}
bool  VrtIfFlowFilterEntryMoMgr::CompareValidValue(void *&val1,
                                                   void *val2, bool audit) {
  UPLL_FUNC_TRACE;
  val_flowfilter_entry_t *val_ff_entry1 =
      reinterpret_cast<val_flowfilter_entry_t *>(val1);
  val_flowfilter_entry_t *val_ff_entry2 =
      reinterpret_cast<val_flowfilter_entry_t *>(val2);

  // if (audit) {
    for ( unsigned int loop = 0; loop < sizeof(val_ff_entry1->valid); ++loop ) {
      if (UNC_VF_INVALID == val_ff_entry1->valid[loop] &&
                  UNC_VF_VALID == val_ff_entry2->valid[loop])
       val_ff_entry1->valid[loop] = UNC_VF_VALID_NO_VALUE;
    }
  // }

  if (val_ff_entry1->valid[UPLL_IDX_FLOWLIST_NAME_FFE] == UNC_VF_VALID &&
     val_ff_entry2->valid[UPLL_IDX_FLOWLIST_NAME_FFE] == UNC_VF_VALID) {
    if (!strcmp(reinterpret_cast<char *>(val_ff_entry1->flowlist_name),
               reinterpret_cast<char *>(val_ff_entry2->flowlist_name)))
      val_ff_entry1->valid[UPLL_IDX_FLOWLIST_NAME_FFE] = UNC_VF_INVALID;
  }
  if (val_ff_entry1->valid[UPLL_IDX_ACTION_FFE] == UNC_VF_VALID &&
     val_ff_entry2->valid[UPLL_IDX_ACTION_FFE] == UNC_VF_VALID) {
    if (val_ff_entry1->action == val_ff_entry2->action)
      val_ff_entry1->valid[UPLL_IDX_ACTION_FFE] = UNC_VF_INVALID;
  }
  if (val_ff_entry1->valid[UPLL_IDX_REDIRECT_NODE_FFE] == UNC_VF_VALID &&
     val_ff_entry2->valid[UPLL_IDX_REDIRECT_NODE_FFE] == UNC_VF_VALID) {
    if (!strcmp(reinterpret_cast<char *>(val_ff_entry1->redirect_node),
              reinterpret_cast<char *>(val_ff_entry2->redirect_node)))
      val_ff_entry1->valid[UPLL_IDX_REDIRECT_NODE_FFE] = UNC_VF_INVALID;
  }
  if (val_ff_entry1->valid[UPLL_IDX_REDIRECT_PORT_FFE] == UNC_VF_VALID &&
     val_ff_entry2->valid[UPLL_IDX_REDIRECT_PORT_FFE] == UNC_VF_VALID) {
    if (!strcmp(reinterpret_cast<char *>(val_ff_entry1->redirect_port),
              reinterpret_cast<char *>(val_ff_entry2->redirect_port)))
      val_ff_entry1->valid[UPLL_IDX_REDIRECT_PORT_FFE] = UNC_VF_INVALID;
  }
  if (val_ff_entry1->valid[UPLL_IDX_MODIFY_DST_MAC_FFE] == UNC_VF_VALID &&
     val_ff_entry2->valid[UPLL_IDX_MODIFY_DST_MAC_FFE] == UNC_VF_VALID) {
    if (!memcmp(val_ff_entry1->modify_dstmac, val_ff_entry2->modify_dstmac,
         sizeof(val_ff_entry2->modify_dstmac)))
      val_ff_entry1->valid[UPLL_IDX_MODIFY_DST_MAC_FFE] = UNC_VF_INVALID;
  }
  if (val_ff_entry1->valid[UPLL_IDX_MODIFY_SRC_MAC_FFE] == UNC_VF_VALID &&
     val_ff_entry2->valid[UPLL_IDX_MODIFY_SRC_MAC_FFE] == UNC_VF_VALID) {
    if (!memcmp(val_ff_entry1->modify_srcmac, val_ff_entry2->modify_srcmac,
        sizeof(val_ff_entry2->modify_srcmac)))
      val_ff_entry1->valid[UPLL_IDX_MODIFY_SRC_MAC_FFE] = UNC_VF_INVALID;
  }
  if (val_ff_entry1->valid[UPLL_IDX_NWM_NAME_FFE] == UNC_VF_VALID &&
     val_ff_entry2->valid[UPLL_IDX_NWM_NAME_FFE] == UNC_VF_VALID) {
    if (!strcmp(reinterpret_cast<char *>(val_ff_entry1->nwm_name),
              reinterpret_cast<char *>(val_ff_entry2->nwm_name)))
      val_ff_entry1->valid[UPLL_IDX_NWM_NAME_FFE] = UNC_VF_INVALID;
  }
  if (val_ff_entry1->valid[UPLL_IDX_DSCP_FFE] == UNC_VF_VALID &&
     val_ff_entry2->valid[UPLL_IDX_DSCP_FFE] == UNC_VF_VALID) {
    if (val_ff_entry1->dscp == val_ff_entry2->dscp)
      val_ff_entry1->valid[UPLL_IDX_DSCP_FFE] = UNC_VF_INVALID;
  }
  if (val_ff_entry1->valid[UPLL_IDX_PRIORITY_FFE] == UNC_VF_VALID &&
     val_ff_entry2->valid[UPLL_IDX_PRIORITY_FFE] == UNC_VF_VALID) {
    if (val_ff_entry1->priority == val_ff_entry2->priority)
      val_ff_entry1->valid[UPLL_IDX_PRIORITY_FFE] = UNC_VF_INVALID;
  }
  UPLL_LOG_DEBUG("CompareValidValue :: Success");
  return false;
}
upll_rc_t VrtIfFlowFilterEntryMoMgr::ReadDetailEntry(
    ConfigKeyVal *ff_ckv,
    upll_keytype_datatype_t dt_type,
    unc_keytype_operation_t op,
    DbSubOp dbop,
    DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_ERR_GENERIC;

  if (NULL == ff_ckv) {
    UPLL_LOG_DEBUG("InPut ConfigKeyVal is NULL");
    return UPLL_RC_ERR_BAD_REQUEST;
  }

  //  SET_USER_DATA_DOMAIN(ff_ckv, domain_id);
  //  SET_USER_DATA_CTRLR(ff_ckv, ctrlr_id);

  result_code = ReadConfigDB(ff_ckv, dt_type, UNC_OP_READ, dbop, dmi, MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    return UPLL_RC_ERR_GENERIC;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VrtIfFlowFilterEntryMoMgr::TxUpdateController(
    unc_key_type_t keytype,
    uint32_t session_id,
    uint32_t config_id,
    uuc::UpdateCtrlrPhase phase,
    set<string> *affected_ctrlr_set,
    DalDmlIntf *dmi,
    ConfigKeyVal **err_ckv)  {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *req, *nreq = NULL, *ck_main = NULL;
  controller_domain_t ctrlr_dom;
  ctrlr_dom.ctrlr = NULL;
  ctrlr_dom.domain = NULL;
  DalCursor *dal_cursor_handle = NULL;
  IpcResponse ipc_resp;
  uint8_t flag = 0;
  DalResultCode db_result;
  uint8_t db_flag = 0;
  if (affected_ctrlr_set == NULL)
    return UPLL_RC_ERR_GENERIC;

  unc_keytype_operation_t op = (phase == uuc::kUpllUcpCreate)?UNC_OP_CREATE:
      ((phase == uuc::kUpllUcpUpdate)?UNC_OP_UPDATE:
       ((phase == uuc::kUpllUcpDelete)?UNC_OP_DELETE:UNC_OP_INVALID));

  result_code = DiffConfigDB(UPLL_DT_CANDIDATE, UPLL_DT_RUNNING,
                             op, req, nreq, &dal_cursor_handle, dmi, MAINTBL);
  unc_keytype_operation_t op1 = op;

  while (result_code == UPLL_RC_SUCCESS) {
    ck_main = NULL;
    db_result = dmi->GetNextRecord(dal_cursor_handle);
    result_code = DalToUpllResCode(db_result);
    if (result_code != UPLL_RC_SUCCESS)
      break;
    switch (op)   {
      case UNC_OP_CREATE:
      case UNC_OP_UPDATE:
        /* fall through intended */
        /*Restore the original op code for each DB record*/
        op1 = op;
        result_code = DupConfigKeyVal(ck_main, req, MAINTBL);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_TRACE("DupConfigKeyVal failed %d\n", result_code);
          return result_code;
        }
        break;
      case UNC_OP_DELETE:
        {
          /*Restore the original op code for each DB record*/
          op1 = op;
          result_code = GetChildConfigKey(ck_main, req);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_TRACE("GetChildConfigKey failed %d\n", result_code);
            return result_code;
          }
          DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutCtrlr};
          result_code = ReadConfigDB(ck_main, UPLL_DT_RUNNING, UNC_OP_READ,
                                     dbop, dmi, MAINTBL);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("Returning error %d\n", result_code);
            return UPLL_RC_ERR_GENERIC;
          }
        }
      default:
        break;
    }
    GET_USER_DATA_CTRLR_DOMAIN(ck_main, ctrlr_dom);
    if (ctrlr_dom.ctrlr == NULL) {
      return UPLL_RC_ERR_GENERIC;
    }
    /*
       if ((op == UNC_OP_CREATE) || (op == UNC_OP_UPDATE)) {
       void *main = GetVal(ck_main);
       void *val_nrec = (nreq) ? GetVal(nreq) : NULL;
       FilterAttributes(main, val_nrec, false, op);
       }
       */
    GET_USER_DATA_FLAGS(ck_main, db_flag);
    if (!(SET_FLAG_VLINK & db_flag)) {
      if (op1 != UNC_OP_UPDATE) {
        continue;
      }
      ConfigKeyVal *temp = NULL;
      result_code = GetChildConfigKey(temp, ck_main);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("GetChildConfigKey failed, err %d", result_code);
        return result_code;
      }
      // SET_USER_DATA_CTRLR_DOMAIN(temp, ctrlr_dom);
      DbSubOp dbop1 = { kOpReadSingle, kOpMatchCtrlr|kOpMatchDomain,
        kOpInOutFlag};
      result_code = ReadConfigDB(temp, UPLL_DT_RUNNING,
                                 UNC_OP_READ, dbop1, dmi, MAINTBL);
      if (result_code != UPLL_RC_SUCCESS) {
        if (result_code != UPLL_RC_ERR_INSTANCE_EXISTS) {
          UPLL_LOG_DEBUG("Unable to read from DB, err: %d", result_code);
          return result_code;
        }
      }
      GET_USER_DATA_FLAGS(temp, flag);
      if (!(SET_FLAG_VLINK & flag)) {
        UPLL_LOG_DEBUG("Vlink flag is not set for VrtIfFlowFilterEntry");
        continue;
      }
      op1 = UNC_OP_DELETE;
    } else {
      if (UNC_OP_UPDATE == op1) {
        ConfigKeyVal *temp = NULL;
        result_code = GetChildConfigKey(temp, ck_main);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_TRACE("GetChildConfigKey failed %d\n", result_code);
          return result_code;
        }
        DbSubOp dbop = { kOpReadSingle, kOpMatchCtrlr|kOpMatchDomain,
          kOpInOutFlag};
        result_code = ReadConfigDB(temp, UPLL_DT_RUNNING, UNC_OP_READ,
                                   dbop, dmi, MAINTBL);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("Returning error %d\n", result_code);
          return UPLL_RC_ERR_GENERIC;
        }
        GET_USER_DATA_FLAGS(temp, flag);
        if (!(SET_FLAG_VLINK & flag)) {
          op1 = UNC_OP_CREATE;
        } else {
          void *main = GetVal(ck_main);
          void *val_nrec = (nreq) ? GetVal(nreq) : NULL;
          FilterAttributes(main, val_nrec, false, op);
        }
      }
    }

    pfcdrv_val_flowfilter_entry_t *pfc_val =
        reinterpret_cast<pfcdrv_val_flowfilter_entry_t *>
        (ConfigKeyVal::Malloc(sizeof(pfcdrv_val_flowfilter_entry_t)));

    pfc_val->valid[PFCDRV_IDX_VAL_VBRIF_VEXTIF_FFE] = UNC_VF_VALID;
    pfc_val->valid[PFCDRV_IDX_FLOWFILTER_ENTRY_FFE] = UNC_VF_VALID;
    pfc_val->val_vbrif_vextif.valid[PFCDRV_IDX_INTERFACE_TYPE] = UNC_VF_VALID;
    pfc_val->val_vbrif_vextif.interface_type = PFCDRV_IF_TYPE_VBRIF;

    val_flowfilter_entry_t* val = reinterpret_cast<val_flowfilter_entry_t *>
        (GetVal(ck_main));
    memcpy(&pfc_val->val_ff_entry, val, sizeof(val_flowfilter_entry_t));

    // Inserting the controller to Set
    affected_ctrlr_set->insert
        (string(reinterpret_cast<char *>(ctrlr_dom.ctrlr)));
    upll_keytype_datatype_t dt_type = (op1 == UNC_OP_DELETE)?
        UPLL_DT_RUNNING:UPLL_DT_CANDIDATE;
    result_code = GetRenamedControllerKey(ck_main, dt_type,
                                          dmi, &ctrlr_dom);
    if (result_code != UPLL_RC_SUCCESS)
      break;

    UPLL_LOG_DEBUG("Controller : %s; Domain : %s", ctrlr_dom.ctrlr,
                   ctrlr_dom.domain);
    ck_main->SetCfgVal(new ConfigVal(IpctSt::kIpcStPfcdrvValFlowfilterEntry,
                                     pfc_val));
    result_code = SendIpcReq(session_id, config_id, op1, UPLL_DT_CANDIDATE,
                             ck_main, &ctrlr_dom, &ipc_resp);
    if (result_code == UPLL_RC_ERR_RESOURCE_DISCONNECTED) {
      UPLL_LOG_DEBUG(" driver result code - %d", result_code);
      result_code = UPLL_RC_SUCCESS;
    }
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("IpcSend failed %d\n", result_code);
      *err_ckv = ipc_resp.ckv_data;
      if (ck_main)
        delete ck_main;
      break;
    }
    if (ck_main) {
      delete ck_main;
      ck_main = NULL;
    }
  }
  if (dal_cursor_handle) {
    dmi->CloseCursor(dal_cursor_handle, true);
    dal_cursor_handle = NULL;
  }
  if (req)
    delete req;
  if (nreq)
    delete nreq;
  result_code = (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) ?
      UPLL_RC_SUCCESS : result_code;

  return result_code;
}

upll_rc_t VrtIfFlowFilterEntryMoMgr::SetVlinkPortmapConfiguration(
                        ConfigKeyVal *ikey,
                        upll_keytype_datatype_t dt_type,
                        DalDmlIntf *dmi, InterfacePortMapInfo flag) {
  upll_rc_t result_code = UPLL_RC_ERR_GENERIC;
  if (NULL == ikey || NULL == ikey->get_key()) {
    return result_code;
  }
  ConfigKeyVal *ckv = NULL;
  result_code = GetChildConfigKey(ckv, NULL);
  if (UPLL_RC_SUCCESS != result_code) {
    return result_code;
  }
  key_vrt_if_flowfilter_entry_t *ff_key =
      reinterpret_cast<key_vrt_if_flowfilter_entry_t *>(ckv->get_key());
  key_vrt_if_t *vrtif_key = reinterpret_cast<key_vrt_if_t *>(ikey->get_key());

  uuu::upll_strncpy(ff_key->flowfilter_key.if_key.vrt_key.vtn_key.vtn_name,
                   vrtif_key->vrt_key.vtn_key.vtn_name,
                   kMaxLenVtnName + 1);

  uuu::upll_strncpy(ff_key->flowfilter_key.if_key.vrt_key.vrouter_name,
                    vrtif_key->vrt_key.vrouter_name,
                    kMaxLenVtnName + 1);

  uuu::upll_strncpy(ff_key->flowfilter_key.if_key.if_name,
                    vrtif_key->if_name,
                    kMaxLenInterfaceName + 1);
  ff_key->flowfilter_key.direction = 0xFE;
  DbSubOp dbop = { kOpReadMultiple, kOpMatchNone, kOpInOutFlag };
  result_code = ReadConfigDB(ckv, dt_type ,
                             UNC_OP_READ, dbop, dmi, MAINTBL);
  if (UPLL_RC_SUCCESS != result_code) {
    delete ckv;
    return result_code;
  }
  if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
    UPLL_LOG_DEBUG("No Recrods in the Vbr_If_FlowFilter Table");
    return UPLL_RC_SUCCESS;
  }
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("Read ConfigDB failure %d", result_code);
    return result_code;
  }
  uint8_t  flag_port_map = 0;
  while (ckv) {
    flag_port_map = 0;
    GET_USER_DATA_FLAGS(ckv, flag_port_map);
    if (flag & kVlinkConfigured) {
      flag_port_map |= SET_FLAG_VLINK;
    } else {
      flag_port_map = 0;
    }
    SET_USER_DATA_FLAGS(ckv, flag_port_map);
    DbSubOp dbop_update = { kOpReadSingle, kOpMatchNone, kOpInOutFlag };
    result_code = UpdateConfigDB(ckv, dt_type, UNC_OP_UPDATE,
                                 dmi, &dbop_update, MAINTBL);
    if (UPLL_RC_SUCCESS != result_code) {
      return result_code;
    }
    ckv = ckv->get_next_cfg_key_val();
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VrtIfFlowFilterEntryMoMgr::CreateCandidateMo(IpcReqRespHeader *req,
                                                       ConfigKeyVal *ikey,
                                                       DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  ConfigKeyVal*  okey = NULL;
  uint8_t *ctrlr_id = NULL;
  controller_domain ctrlr_dom;
  memset(&ctrlr_dom, 0, sizeof(ctrlr_dom));
  if (ikey == NULL && req == NULL) {
    UPLL_LOG_DEBUG(
        "Cannot perform create operation due to insufficient parameters\n");
    return UPLL_RC_ERR_GENERIC;
  }
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  // validate syntax and semantics
  result_code = ValidateMessage(req, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG(" ValidateMessage failed ");
    return result_code;
  }

  result_code = ValidateAttribute(ikey, dmi, req);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG(" ValidateAttribute failed ");
    return result_code;
  }

  /*
     result_code= ValidateCapability(req, ikey);
     if (UPLL_RC_SUCCESS != result_code)
     return result_code;
     */
  // Check if Object already exists in CANDIDATE DB
  result_code = UpdateConfigDB(ikey, req->datatype, UNC_OP_READ, dmi);
  if (result_code == UPLL_RC_ERR_INSTANCE_EXISTS
      || result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    UPLL_LOG_DEBUG("Record already exists in Candidate DB");
    return result_code;
  }

  // Check if Object exists in RUNNING DB and move it to CANDIDATE DB
  result_code = UpdateConfigDB(ikey, UPLL_DT_RUNNING, UNC_OP_READ, dmi,
                               MAINTBL);
  if (result_code == UPLL_RC_ERR_INSTANCE_EXISTS) {
    result_code = RestoreChildren(ikey, req->datatype, UPLL_DT_RUNNING, dmi);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG(" RestoreChildren failed. err code(%d)", result_code);
      return result_code;
    }
  } else if (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    UPLL_LOG_DEBUG(" error reading DB. err code(%d)", result_code);
    return result_code;
  } else {
    UPLL_LOG_DEBUG("Record doesn't exist in reading Running DB ");
  }
  val_flowfilter_entry_t *flowfilter_val =
      reinterpret_cast<val_flowfilter_entry_t *> (GetVal(ikey));
  FlowListMoMgr *mgr = NULL;
  if (flowfilter_val->valid[UPLL_IDX_FLOWLIST_NAME_FFE] == UNC_VF_VALID) {
    mgr = reinterpret_cast<FlowListMoMgr *>
        (const_cast<MoManager *> (GetMoManager(UNC_KT_FLOWLIST)));

    // Check Flowlist object exist or not
    result_code = mgr->GetChildConfigKey(okey, NULL);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Recored %d", result_code);
      return result_code;
    }
    key_flowlist_t *key_flowlist = reinterpret_cast<key_flowlist_t*>
        (okey->get_key());
    uuu::upll_strncpy(key_flowlist->flowlist_name,
                      flowfilter_val->flowlist_name,
                      (kMaxLenFlowListName +1));
    result_code = mgr->IsReferenced(okey, req->datatype, dmi);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Flowlist not available %d", result_code);
      return result_code;
    }
    if (okey) {
      delete okey;
      okey = NULL;
    }
  }
  // N/w monitor
#if 0
  NwMonitorMoMgr *nmgr = reinterpret_cast<NwMonitorMoMgr *>
      (const_cast<MoManager *> (GetMoManager(UNC_KT_VBR_NWMONITOR)));
  //  result_code = nmgr->GetChildConfigKey(okey, NULL); //TODO
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Recored %d", result_code);
    return result_code;
  }
  key_nwm_t *key_nwm = reinterpret_cast<key_nwm_t*>(okey->get_key());
  uuu::upll_strncpy(reinterpret_cast<char*>(key_nwm->nwmonitor_name),
                    reinterpret_cast<const char*>(flowfilter_val->nwm_name),
                    kMaxLenNwmName +1);
  //  result_code = nmgr->IsReferenced(okey, req->datatype, dmi); //TODO
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Recored %d", result_code);
    return result_code;
  }
#endif

  // create a record in CANDIDATE DB
  VrtIfMoMgr *vrtifmgr =
      reinterpret_cast<VrtIfMoMgr *>(const_cast<MoManager *>(GetMoManager(
              UNC_KT_VRT_IF)));
  ConfigKeyVal *ckv = NULL;
  InterfacePortMapInfo flags = kVlinkPortMapNotConfigured;
  result_code = vrtifmgr->GetChildConfigKey(ckv, NULL);
    key_vrt_if_flowfilter_entry_t *ff_key = reinterpret_cast
      <key_vrt_if_flowfilter_entry_t *>(ikey->get_key());
    key_vrt_if_t *vrtif_key = reinterpret_cast<key_vrt_if_t *>(ckv->get_key());

    uuu::upll_strncpy(vrtif_key->vrt_key.vtn_key.vtn_name,
        ff_key->flowfilter_key.if_key.vrt_key.vtn_key.vtn_name,
        kMaxLenVtnName + 1);

    uuu::upll_strncpy(vrtif_key->vrt_key.vrouter_name,
        ff_key->flowfilter_key.if_key.vrt_key.vrouter_name,
        kMaxLenVtnName + 1);

    uuu::upll_strncpy(vrtif_key->if_name,
        ff_key->flowfilter_key.if_key.if_name,
        kMaxLenInterfaceName + 1);

    uint8_t* vexternal = reinterpret_cast<uint8_t*>
        (ConfigKeyVal::Malloc(kMaxLenVnodeName + 1));
    uint8_t* vex_if = reinterpret_cast<uint8_t*>
        (ConfigKeyVal::Malloc(kMaxLenInterfaceName + 1));
    result_code = vrtifmgr->GetVexternal(ckv, req->datatype, dmi,
        vexternal, vex_if, flags);
    if (UPLL_RC_SUCCESS != result_code) {
      return result_code;
    }
    uint8_t flag_port_map;
    if (flags & kVlinkConfigured) {
      flag_port_map = SET_FLAG_VLINK;
    } else {
      flag_port_map = 0;
    }
    free(vexternal);
    free(vex_if);
    SET_USER_DATA_FLAGS(ikey, flag_port_map);


  result_code = GetControllerDomainID(ikey, req->datatype, dmi);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Failed to Get the Controller Domain details, err:%d",
                   result_code);
  }
  GET_USER_DATA_CTRLR_DOMAIN(ikey, ctrlr_dom);

  UPLL_LOG_DEBUG("ctrlrid %s, domainid %s",
                 ctrlr_dom.ctrlr, ctrlr_dom.domain);
  ctrlr_id = ctrlr_dom.ctrlr;

  if (flowfilter_val->valid[UPLL_IDX_FLOWLIST_NAME_FFE] == UNC_VF_VALID) {
    result_code = mgr->AddFlowListToController(
        reinterpret_cast<char *>(flowfilter_val->flowlist_name), dmi,
        reinterpret_cast<char *> (ctrlr_id) , UNC_OP_CREATE);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Reference Count Updation Fails %d", result_code);
      return result_code;
    }
  }

  result_code = UpdateConfigDB(ikey, req->datatype, UNC_OP_CREATE, dmi);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Unable to update CandidateDB %d", result_code);
  }

  return result_code;
}

upll_rc_t VrtIfFlowFilterEntryMoMgr::GetControllerDomainID(ConfigKeyVal *ikey,
                                          upll_keytype_datatype_t dt_type,
                                          DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  controller_domain ctrlr_dom;
  memset(&ctrlr_dom, 0, sizeof(controller_domain));
  VrtIfMoMgr *mgrvrtif =
      reinterpret_cast<VrtIfMoMgr *>(const_cast<MoManager *>(GetMoManager(
                  UNC_KT_VRT_IF)));

  ConfigKeyVal *ckv = NULL;
  result_code = mgrvrtif->GetChildConfigKey(ckv, NULL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Unable to get the ParentConfigKey, resultcode=%d",
                    result_code);
    return result_code;
  }

  key_vrt_if_flowfilter_entry_t *temp_key =
      reinterpret_cast<key_vrt_if_flowfilter_entry_t*>(ikey->get_key());

  key_vrt_if_t *vrt_if_key = reinterpret_cast<key_vrt_if_t*>(ckv->get_key());

  uuu::upll_strncpy(vrt_if_key->vrt_key.vtn_key.vtn_name,
                    temp_key->flowfilter_key.if_key.vrt_key.vtn_key.vtn_name,
                    kMaxLenVtnName + 1);
  uuu::upll_strncpy(vrt_if_key->vrt_key.vrouter_name,
                    temp_key->flowfilter_key.if_key.vrt_key.vrouter_name,
                    kMaxLenVnodeName + 1);

  uuu::upll_strncpy(vrt_if_key->if_name,
                    temp_key->flowfilter_key.if_key.if_name,
                    kMaxLenInterfaceName + 1);


  ConfigKeyVal *vrt_key = NULL;
  result_code = mgrvrtif->GetParentConfigKey(vrt_key, ckv);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetParentConfigKey Failed, err %d", result_code);
    DELETE_IF_NOT_NULL(ckv);
    return result_code;
  }

  result_code = mgrvrtif->GetControllerDomainId(vrt_key, dt_type,
                                                &ctrlr_dom, dmi);
  if ((result_code != UPLL_RC_SUCCESS) || (ctrlr_dom.ctrlr == NULL)
      || (ctrlr_dom.domain == NULL)) {
    DELETE_IF_NOT_NULL(vrt_key);
    DELETE_IF_NOT_NULL(ckv);
    UPLL_LOG_INFO("GetControllerDomainId error err code(%d)", result_code);
    return UPLL_RC_ERR_GENERIC;
  }

  UPLL_LOG_DEBUG("ctrlrid %s, domainid %s",
                 ctrlr_dom.ctrlr, ctrlr_dom.domain);
  SET_USER_DATA_CTRLR_DOMAIN(ikey, ctrlr_dom);

  DELETE_IF_NOT_NULL(vrt_key);
  DELETE_IF_NOT_NULL(ckv);
  return result_code;
}

upll_rc_t VrtIfFlowFilterEntryMoMgr::GetParentConfigKey(ConfigKeyVal *&okey,
                                                        ConfigKeyVal *ikey) {
  UPLL_FUNC_TRACE;

  if (!ikey) {
    UPLL_LOG_DEBUG(" Input Key is NULL");
    return UPLL_RC_ERR_GENERIC;
  }

  unc_key_type_t ikey_type = ikey->get_key_type();
  if (ikey_type != UNC_KT_VRTIF_FLOWFILTER_ENTRY) {
    UPLL_LOG_DEBUG(" Invalid key type received. Key type - %d", ikey_type);
    return UPLL_RC_ERR_GENERIC;
  }

  key_vrt_if_flowfilter_entry_t *pkey =
      reinterpret_cast<key_vrt_if_flowfilter_entry_t*>(ikey->get_key());
  if (!pkey) {
    UPLL_LOG_DEBUG(" Input vrt if flow filter entry key is NULL ");
    return UPLL_RC_ERR_GENERIC;
  }

  key_vrt_if_flowfilter_t *vrt_if_ff_key =
      reinterpret_cast<key_vrt_if_flowfilter_t*>
      (ConfigKeyVal::Malloc(sizeof(key_vrt_if_flowfilter_t)));

  uuu::upll_strncpy(vrt_if_ff_key->if_key.vrt_key.vtn_key.vtn_name,
                    reinterpret_cast<key_vrt_if_flowfilter_entry_t *>
                    (pkey)->flowfilter_key.if_key.vrt_key.vtn_key.vtn_name,
                    kMaxLenVtnName + 1);
  uuu::upll_strncpy(vrt_if_ff_key->if_key.vrt_key.vrouter_name,
                    reinterpret_cast<key_vrt_if_flowfilter_entry_t *>
                    (pkey)->flowfilter_key.if_key.vrt_key.vrouter_name,
                    kMaxLenVnodeName + 1);
  uuu::upll_strncpy(vrt_if_ff_key->if_key.if_name,
                    reinterpret_cast<key_vrt_if_flowfilter_entry_t *>
                    (pkey)->flowfilter_key.if_key.if_name,
                    kMaxLenInterfaceName + 1);
  vrt_if_ff_key->direction = reinterpret_cast<key_vrt_if_flowfilter_entry_t *>
      (pkey)->flowfilter_key.direction;
  okey = new ConfigKeyVal(UNC_KT_VRTIF_FLOWFILTER,
                          IpctSt::kIpcStKeyVrtIfFlowfilter,
                          vrt_if_ff_key, NULL);
  SET_USER_DATA(okey, ikey);
  return UPLL_RC_SUCCESS;
}

upll_rc_t VrtIfFlowFilterEntryMoMgr::ConstructReadDetailResponse(
    ConfigKeyVal *ikey,
    ConfigKeyVal *drv_resp_ckv,
    DalDmlIntf *dmi,
    ConfigKeyVal **okey) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *tmp_okey = NULL;

  DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutNone };

  result_code =  GetChildConfigKey(tmp_okey, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetChildConfigKey failed err code (%d)", result_code);
    return result_code;
  }
  result_code = ReadConfigDB(tmp_okey, UPLL_DT_RUNNING, UNC_OP_READ,
                             dbop, dmi, MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("ReadConfigDB error (%d)", result_code);
    delete tmp_okey;
    return result_code;
  }

  ConfigVal *drv_resp_val = NULL;
  drv_resp_val =  drv_resp_ckv->get_cfg_val();
  while (drv_resp_val != NULL) {
    if (IpctSt::kIpcStValFlowfilterEntrySt != drv_resp_val->get_st_num()) {
      UPLL_LOG_DEBUG("Incorrect structure received from driver, struct num %d",
                     drv_resp_val->get_st_num());
      return  UPLL_RC_ERR_GENERIC;
    }
    val_flowfilter_entry_st_t *tmp_ffe_st =
        reinterpret_cast<val_flowfilter_entry_st_t*>
        (ConfigKeyVal::Malloc(sizeof(val_flowfilter_entry_st_t)));
    memcpy(tmp_ffe_st,
           reinterpret_cast<val_flowfilter_entry_st_t *>
           (drv_resp_val->get_val()),
           sizeof(val_flowfilter_entry_st_t));
    tmp_okey->AppendCfgVal(IpctSt::kIpcStValFlowfilterEntrySt, tmp_ffe_st);

    if ((drv_resp_val = drv_resp_val->get_next_cfg_val()) == NULL) {
      UPLL_LOG_DEBUG("No more entries in driver response\n");
      break;
    }

    if (IpctSt::kIpcStValFlowlistEntrySt != (drv_resp_val)->get_st_num()) {
      UPLL_LOG_DEBUG("No flowflist entries returned by driver");
      continue;
    }

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
  if (*okey == NULL) {
    *okey = tmp_okey;
  } else {
    (*okey)->AppendCfgKeyVal(tmp_okey);
  }
  return UPLL_RC_SUCCESS;
}
}  // namespace kt_momgr
}  // namespace upll
}  // namespace unc
