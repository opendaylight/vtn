/*
 * Copyright (c) 2013-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include "pfc/log.h"
#include "vterm_if_flowfilter_entry_momgr.hh"
#include "vterm_if_momgr.hh"
#include "vbr_if_momgr.hh"
#include "vterm_if_flowfilter_momgr.hh"
#include "vbr_flowfilter_entry_momgr.hh"
#include "unc/upll_errno.h"
#include "upll_validation.hh"
#include "unc/upll_ipc_enum.h"
#include "flowlist_momgr.hh"
#include "uncxx/upll_log.hh"

namespace unc {
namespace upll {
namespace kt_momgr {

using unc::upll::ipc_util::IpcUtil;
#define FLOWLIST_RENAME_FLAG    0x04  // For 3rd Bit
#define VTN_RENAME_FLAG         0x01  // For first Bit
#define VBR_RENAME_FLAG         0x02  // For 2nd Bit
#define NO_VBR_RENAME_FLAG      ~VBR_RENAME_FLAG
#define SET_FLAG_VLINK          0x40
#define SET_FLAG_PORTMAP        0x20
#define SET_FLAG_VLINK_PORTMAP (SET_FLAG_VLINK | SET_FLAG_PORTMAP)
#define SET_FLAG_NO_VLINK_PORTMAP ~SET_FLAG_VLINK_PORTMAP

#define FLOW_RENAME             0x04
#define NO_FLOWLIST_RENAME      ~FLOW_RENAME

BindInfo VtermIfFlowFilterEntryMoMgr::vterm_if_flowfilter_entry_bind_info[] = {
  { uudst::vterm_if_flowfilter_entry::kDbiVtnName, CFG_KEY,
    offsetof(key_vterm_if_flowfilter_entry_t,
             flowfilter_key.if_key.vterm_key.vtn_key.vtn_name),
    uud::kDalChar, (kMaxLenVtnName + 1) },
  { uudst::vterm_if_flowfilter_entry::kDbiVtermName, CFG_KEY,
    offsetof(key_vterm_if_flowfilter_entry_t,
             flowfilter_key.if_key.vterm_key.vterminal_name),
    uud::kDalChar, (kMaxLenVnodeName + 1) },
  { uudst::vterm_if_flowfilter_entry::kDbiVtermIfName, CFG_KEY,
    offsetof(key_vterm_if_flowfilter_entry_t, flowfilter_key.if_key.if_name),
    uud::kDalChar, (kMaxLenInterfaceName + 1) },
  { uudst::vterm_if_flowfilter_entry::kDbiInputDirection, CFG_KEY,
    offsetof(key_vterm_if_flowfilter_entry_t, flowfilter_key.direction),
    uud::kDalUint8, 1 },
  { uudst::vterm_if_flowfilter_entry::kDbiSequenceNum, CFG_KEY,
    offsetof(key_vterm_if_flowfilter_entry_t, sequence_num),
    uud::kDalUint16, 1 },
  { uudst::vterm_if_flowfilter_entry::kDbiCtrlrName, CK_VAL,
    offsetof(key_user_data_t, ctrlr_id),
    uud::kDalChar, (kMaxLenCtrlrId + 1) },
  { uudst::vterm_if_flowfilter_entry::kDbiDomainId, CK_VAL,
    offsetof(key_user_data, domain_id),
    uud::kDalChar, (kMaxLenDomainId + 1) },
  { uudst::vterm_if_flowfilter_entry::kDbiFlowlistName, CFG_VAL,
    offsetof(val_flowfilter_entry_t, flowlist_name),
    uud::kDalChar, (kMaxLenFlowListName + 1) },
  { uudst::vterm_if_flowfilter_entry::kDbiAction, CFG_VAL,
    offsetof(val_flowfilter_entry_t, action),
    uud::kDalUint8, 1 },
  { uudst::vterm_if_flowfilter_entry::kDbiRedirectNode, CFG_VAL,
    offsetof(val_flowfilter_entry_t, redirect_node),
    uud::kDalChar, (kMaxLenVnodeName + 1) },
  { uudst::vterm_if_flowfilter_entry::kDbiRedirectPort, CFG_VAL,
    offsetof(val_flowfilter_entry_t, redirect_port),
    uud::kDalChar, kMaxLenInterfaceName + 1 },
  // Adding Redirection
  { uudst::vterm_if_flowfilter_entry::kDbiRedirectDirection, CFG_VAL,
    offsetof(val_flowfilter_entry_t, redirect_direction),
    uud::kDalUint8,  1 },
  // end

  { uudst::vterm_if_flowfilter_entry::kDbiModifyDstMac, CFG_VAL,
    offsetof(val_flowfilter_entry_t, modify_dstmac),
    uud::kDalChar, 6},
  { uudst::vterm_if_flowfilter_entry::kDbiModifySrcMac, CFG_VAL,
    offsetof(val_flowfilter_entry_t, modify_srcmac),
    uud::kDalChar, 6},
  { uudst::vterm_if_flowfilter_entry::kDbiNwmName, CFG_VAL,
    offsetof(val_flowfilter_entry_t, nwm_name),
    uud::kDalChar, (kMaxLenNwmName + 1) },
  { uudst::vterm_if_flowfilter_entry::kDbiDscp, CFG_VAL,
    offsetof(val_flowfilter_entry_t, dscp),
    uud::kDalUint8, 1 },
  { uudst::vterm_if_flowfilter_entry::kDbiPriority, CFG_VAL,
    offsetof(val_flowfilter_entry_t, priority),
    uud::kDalUint8, 1 },
  { uudst::vterm_if_flowfilter_entry::kDbiFlags, CK_VAL,
    offsetof(key_user_data_t, flags),
    uud::kDalUint8, 1 },
  { uudst::vterm_if_flowfilter_entry::kDbiValidFlowlistName, CFG_META_VAL,
    offsetof(val_flowfilter_entry_t, valid[0]),
    uud::kDalUint8, 1 },
  { uudst::vterm_if_flowfilter_entry::kDbiValidAction, CFG_META_VAL,
    offsetof(val_flowfilter_entry_t, valid[1]),
    uud::kDalUint8, 1 },
  { uudst::vterm_if_flowfilter_entry::kDbiValidRedirectNode, CFG_META_VAL,
    offsetof(val_flowfilter_entry_t, valid[2]),
    uud::kDalUint8, 1 },
  { uudst::vterm_if_flowfilter_entry::kDbiValidRedirectPort, CFG_META_VAL,
    offsetof(val_flowfilter_entry_t, valid[3]),
    uud::kDalUint8, 1 },
  { uudst::vterm_if_flowfilter_entry::kDbiValidRedirectDirection, CFG_META_VAL,
    offsetof(val_flowfilter_entry_t, valid[4]),
    uud::kDalUint8, 1 },

  { uudst::vterm_if_flowfilter_entry::kDbiValidModifyDstMac, CFG_META_VAL,
    offsetof(val_flowfilter_entry_t, valid[5]),
    uud::kDalUint8, 1 },
  { uudst::vterm_if_flowfilter_entry::kDbiValidModifySrcMac, CFG_META_VAL,
    offsetof(val_flowfilter_entry_t, valid[6]),
    uud::kDalUint8, 1 },
  { uudst::vterm_if_flowfilter_entry::kDbiValidNwmName, CFG_META_VAL,
    offsetof(val_flowfilter_entry_t, valid[7]),
    uud::kDalUint8, 1 },
  { uudst::vterm_if_flowfilter_entry::kDbiValidDscp, CFG_META_VAL,
    offsetof(val_flowfilter_entry_t, valid[8]),
    uud::kDalUint8, 1 },
  { uudst::vterm_if_flowfilter_entry::kDbiValidPriority, CFG_META_VAL,
    offsetof(val_flowfilter_entry_t, valid[9]),
    uud::kDalUint8, 1 },
  { uudst::vterm_if_flowfilter_entry::kDbiCsRowStatus, CS_VAL,
    offsetof(val_flowfilter_entry_t, cs_row_status),
    uud::kDalUint8, 1 },
  { uudst::vterm_if_flowfilter_entry::kDbiCsFlowlistName, CS_VAL,
    offsetof(val_flowfilter_entry_t, cs_attr[0]),
    uud::kDalUint8, 1 },
  { uudst::vterm_if_flowfilter_entry::kDbiCsAction, CS_VAL,
    offsetof(val_flowfilter_entry_t, cs_attr[1]),
    uud::kDalUint8, 1 },
  { uudst::vterm_if_flowfilter_entry::kDbiCsRedirectNode, CS_VAL,
    offsetof(val_flowfilter_entry_t, cs_attr[2]),
    uud::kDalUint8, 1 },
  { uudst::vterm_if_flowfilter_entry::kDbiCsRedirectPort, CS_VAL,
    offsetof(val_flowfilter_entry_t, cs_attr[3]),
    uud::kDalUint8, 1 },
  { uudst::vterm_if_flowfilter_entry::kDbiCsRedirectDirection, CS_VAL,
    offsetof(val_flowfilter_entry_t, cs_attr[4]),
    uud::kDalUint8, 1 },

  { uudst::vterm_if_flowfilter_entry::kDbiCsModifyDstMac, CS_VAL,
    offsetof(val_flowfilter_entry_t, cs_attr[5]),
    uud::kDalUint8, 1 },
  { uudst::vterm_if_flowfilter_entry::kDbiCsModifySrcMac, CS_VAL,
    offsetof(val_flowfilter_entry_t, cs_attr[6]),
    uud::kDalUint8, 1 },
  { uudst::vterm_if_flowfilter_entry::kDbiCsNwmName, CS_VAL,
    offsetof(val_flowfilter_entry_t, cs_attr[7]),
    uud::kDalUint8, 1 },
  { uudst::vterm_if_flowfilter_entry::kDbiCsDscp, CS_VAL,
    offsetof(val_flowfilter_entry_t, cs_attr[8]),
    uud::kDalUint8, 1 },
  { uudst::vterm_if_flowfilter_entry::kDbiCsPriority, CS_VAL,
    offsetof(val_flowfilter_entry_t, cs_attr[9]),
    uud::kDalUint8, 1 }
};

BindInfo VtermIfFlowFilterEntryMoMgr::
                vterm_if_flowfilter_entry_main_tbl_bind_info[] = {
  { uudst::vterm_if_flowfilter_entry::kDbiVtnName, CFG_MATCH_KEY,
    offsetof(key_vterm_if_flowfilter_entry_t,
             flowfilter_key.if_key.vterm_key.vtn_key.vtn_name),
    uud::kDalChar, (kMaxLenVtnName + 1) },
  { uudst::vterm_if_flowfilter_entry::kDbiVtermName, CFG_MATCH_KEY,
    offsetof(key_vterm_if_flowfilter_entry_t,
             flowfilter_key.if_key.vterm_key.vterminal_name),
    uud::kDalChar, (kMaxLenVnodeName + 1) },
  { uudst::vterm_if_flowfilter_entry::kDbiVtnName, CFG_INPUT_KEY,
    offsetof(key_rename_vnode_info_t, new_unc_vtn_name),
    uud::kDalChar, (kMaxLenVtnName + 1) },
  { uudst::vterm_if_flowfilter_entry::kDbiVtermName, CFG_INPUT_KEY,
    offsetof(key_rename_vnode_info_t, new_unc_vnode_name),
    uud::kDalChar, (kMaxLenVnodeName + 1) },
  { uudst::vterm_if_flowfilter_entry::kDbiFlags, CK_VAL,
    offsetof(key_user_data_t, flags),
    uud::kDalUint8, 1 }
};


VtermIfFlowFilterEntryMoMgr::VtermIfFlowFilterEntryMoMgr() : MoMgrImpl() {
  UPLL_FUNC_TRACE;
  // Rename and ctrlr tables not required for this KT
  // setting rename table and controller index to null
  ntable = MAX_MOMGR_TBLS;
  table = new Table *[ntable]();
  table[MAINTBL] = new Table(uudst::kDbiVtermIfFlowFilterEntryTbl,
      UNC_KT_VTERMIF_FLOWFILTER_ENTRY, vterm_if_flowfilter_entry_bind_info,
      IpctSt::kIpcStKeyVtermIfFlowfilterEntry, IpctSt::kIpcStValFlowfilterEntry,
      uudst::vterm_if_flowfilter_entry::kDbiVtermIfFlowFilterEntryNumCols);
  table[RENAMETBL] = NULL;
  table[CTRLRTBL] = NULL;
  table[CONVERTTBL] = NULL;

  nchild = 0;
  child = NULL;
}

bool VtermIfFlowFilterEntryMoMgr::GetRenameKeyBindInfo(
    unc_key_type_t key_type,
    BindInfo *&binfo,
    int &nattr,
    MoMgrTables tbl ) {
  UPLL_FUNC_TRACE;
  /* Main Table only update */
  if (MAINTBL == tbl) {
    nattr = sizeof(vterm_if_flowfilter_entry_main_tbl_bind_info)/
            sizeof(vterm_if_flowfilter_entry_main_tbl_bind_info[0]);
    binfo = vterm_if_flowfilter_entry_main_tbl_bind_info;
  } else {
    UPLL_LOG_ERROR("Invalid Table");
    return PFC_FALSE;
  }

  UPLL_LOG_DEBUG("Successful Completeion");
  return PFC_TRUE;
}

upll_rc_t VtermIfFlowFilterEntryMoMgr::ValidateMessage(IpcReqRespHeader *req,
                                                     ConfigKeyVal *key) {
  UPLL_FUNC_TRACE;
  upll_rc_t rt_code = UPLL_RC_ERR_GENERIC;

  if ((NULL == req) || (NULL == key)) {
    UPLL_LOG_ERROR("ConfigKeyval is NULL");
    return UPLL_RC_ERR_BAD_REQUEST;
  }

  if (UNC_KT_VTERMIF_FLOWFILTER_ENTRY != key->get_key_type()) {
    UPLL_LOG_ERROR(" Invalid keytype(%d)", key->get_key_type());
    return UPLL_RC_ERR_BAD_REQUEST;
  }

  /** Read key structure */
  if (key->get_st_num() != IpctSt::kIpcStKeyVtermIfFlowfilterEntry) {
    UPLL_LOG_ERROR("Invalid key structure received. received struct num - %d",
                   key->get_st_num());
    return UPLL_RC_ERR_BAD_REQUEST;
  }
  if (req->option2 != UNC_OPT2_NONE) {
    UPLL_LOG_ERROR(" Error: option2 is not NONE");
    return UPLL_RC_ERR_INVALID_OPTION2;
  }
  if ((req->option1 != UNC_OPT1_NORMAL)
              &&(req->option1 != UNC_OPT1_DETAIL)) {
     UPLL_LOG_ERROR(" Error: option1 is not NORMAL");
     return UPLL_RC_ERR_INVALID_OPTION1;
  }
  if ((req->option1 != UNC_OPT1_NORMAL)
             &&(req->operation == UNC_OP_READ_SIBLING_COUNT)) {
    UPLL_LOG_ERROR(" Error: option1 is not NORMAL for ReadSiblingCount");
    return UPLL_RC_ERR_INVALID_OPTION1;
  }
  if ((req->option1 == UNC_OPT1_DETAIL) &&
      (req->datatype != UPLL_DT_STATE)) {
      UPLL_LOG_ERROR(" Invalid Datatype(%d)", req->datatype);
      return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT;
  }
  if ((req->datatype == UPLL_DT_IMPORT) && (req->operation == UNC_OP_READ ||
       req->operation == UNC_OP_READ_SIBLING ||
       req->operation == UNC_OP_READ_SIBLING_BEGIN ||
       req->operation == UNC_OP_READ_NEXT ||
       req->operation == UNC_OP_READ_BULK ||
       req->operation == UNC_OP_READ_SIBLING_COUNT)) {
    UPLL_LOG_ERROR("Invalid DT for the operation");
    return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT;
  }

  key_vterm_if_flowfilter_entry_t *key_vterm_if_flowfilter_entry =
      reinterpret_cast<key_vterm_if_flowfilter_entry_t *>(key->get_key());

  /** Validate key structure */
  if (NULL == key_vterm_if_flowfilter_entry) {
    UPLL_LOG_ERROR("KT_VTERMIF_FLOWFILTER_ENTRY Key structure is empty!!");
    return UPLL_RC_ERR_BAD_REQUEST;
  }

  rt_code = ValidateVtermIfFlowfilterEntryKey(key_vterm_if_flowfilter_entry,
                                            req->operation);

  if (UPLL_RC_SUCCESS != rt_code) {
    UPLL_LOG_ERROR("key_vtermif_flowfilter_entry syntax validation failed :"
                   "Err Code - %d",
                   rt_code);
    return rt_code;
  }

  /** Validate value structure */
  if (!key->get_cfg_val()) {
    if ((req->operation == UNC_OP_UPDATE) ||
        (req->operation == UNC_OP_CREATE)) {
      UPLL_LOG_ERROR("val structure is mandatory");
      return UPLL_RC_ERR_BAD_REQUEST;
    } else {
      UPLL_LOG_TRACE("val structure is optional");
      return UPLL_RC_SUCCESS;
    }
  }

  if (key->get_cfg_val()->get_st_num() !=
      IpctSt::kIpcStValFlowfilterEntry) {
    UPLL_LOG_ERROR("Invalid val structure received. struct num - %d",
                   (key->get_cfg_val())->get_st_num());
    return UPLL_RC_ERR_BAD_REQUEST;
  }

  val_flowfilter_entry_t *val_flowfilter_entry =
      reinterpret_cast<val_flowfilter_entry_t *>(
          key->get_cfg_val()->get_val());

  if (NULL == val_flowfilter_entry) {
    UPLL_LOG_ERROR("val_flowfilter_entry structure is null");
    return UPLL_RC_ERR_BAD_REQUEST;
  }

  /** updatemo invokes ValidateFlowfilterEntryValue with valid dmi*/
  if (req->operation == UNC_OP_UPDATE)
    return UPLL_RC_SUCCESS;

  return VbrFlowFilterEntryMoMgr::ValidateFlowfilterEntryValue(
      val_flowfilter_entry, req->operation);
}

upll_rc_t VtermIfFlowFilterEntryMoMgr::ValidateVtermIfValStruct(
    IpcReqRespHeader *req,
    ConfigKeyVal *ikey) {
  UPLL_FUNC_TRACE;

  val_flowfilter_entry_t *val_flowfilter_entry =
      reinterpret_cast<val_flowfilter_entry_t *>(
          ikey->get_cfg_val()->get_val());

  /** Validate value structure */
  return VbrFlowFilterEntryMoMgr::ValidateFlowfilterEntryValue(
         val_flowfilter_entry, req->operation);
}

upll_rc_t VtermIfFlowFilterEntryMoMgr::ValidateVtermIfFlowfilterEntryKey(
    key_vterm_if_flowfilter_entry_t* key_vterm_if_flowfilter_entry,
    unc_keytype_operation_t operation) {
  UPLL_FUNC_TRACE;
  upll_rc_t rt_code = UPLL_RC_ERR_GENERIC;

  VtermIfMoMgr *mgrvtermif = reinterpret_cast<VtermIfMoMgr *>(
      const_cast<MoManager *>(GetMoManager(UNC_KT_VTERM_IF)));

  if (NULL == mgrvtermif) {
    UPLL_LOG_ERROR("unable to get VtermIfMoMgr object to validate key_vtermif");
    return UPLL_RC_ERR_GENERIC;
  }

  rt_code = mgrvtermif->ValidateVtermIfKey(
      &(key_vterm_if_flowfilter_entry->flowfilter_key.if_key));

  if (UPLL_RC_SUCCESS != rt_code) {
    UPLL_LOG_DEBUG(" if_key syntax validation failed :Err Code - %d", rt_code);
    return rt_code;
  }

  /** validate direction */
  if (!ValidateNumericRange(
          key_vterm_if_flowfilter_entry->flowfilter_key.direction,
          (uint8_t) UPLL_FLOWFILTER_DIR_IN, (uint8_t) UPLL_FLOWFILTER_DIR_OUT,
          true, true)) {
    UPLL_LOG_ERROR("direction syntax validation failed");
    return UPLL_RC_ERR_CFG_SYNTAX;
  }

  if ((operation != UNC_OP_READ_SIBLING_COUNT) &&
      (operation != UNC_OP_READ_SIBLING_BEGIN)) {
    /** validate Sequence number */
    if (!ValidateNumericRange(key_vterm_if_flowfilter_entry->sequence_num,
                              kMinFlowFilterSeqNum, kMaxFlowFilterSeqNum, true,
                              true)) {
      UPLL_LOG_ERROR("Sequence number validation failed ");
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  } else {
    key_vterm_if_flowfilter_entry->sequence_num = 0;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtermIfFlowFilterEntryMoMgr::ValidateCapability(IpcReqRespHeader *req,
                                                       ConfigKeyVal *ikey,
                                                       const char* ctrlr_name) {
  UPLL_FUNC_TRACE;

  if ((NULL == req) || (NULL == ikey)) {
    UPLL_LOG_ERROR("IpcReqRespHeader/ConfigKeyval is NULL");
    return UPLL_RC_ERR_GENERIC;
  }

  if (!ctrlr_name)
    ctrlr_name = static_cast<char *>(ikey->get_user_data());

  if (NULL == ctrlr_name) {
    UPLL_LOG_ERROR("ctrlr_name is NULL");
    return UPLL_RC_ERR_GENERIC;
  }

  UPLL_LOG_TRACE("ctrlr_name : (%s)"
               "operation : (%d)",
               ctrlr_name, req->operation);

  bool ret_code = false;
  uint32_t instance_count = 0;
  const uint8_t *attrs = NULL;
  uint32_t max_attrs = 0;

  switch (req->operation) {
    case UNC_OP_CREATE: {
      UPLL_LOG_TRACE("Calling GetCreateCapability Operation  %d ",
                     req->operation);
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
      if (req->datatype == UPLL_DT_STATE) {
        UPLL_LOG_TRACE("Calling GetStateCapability Operation  %d ",
                       req->operation);
        ret_code = GetStateCapability(ctrlr_name, ikey->get_key_type(),
                                      &max_attrs, &attrs);
      } else {
        UPLL_LOG_TRACE("Calling GetReadCapability Operation  %d ",
                       req->operation);
        ret_code = GetReadCapability(ctrlr_name, ikey->get_key_type(),
                                      &max_attrs, &attrs);
      }
      break;
    }
  }

  if (!ret_code) {
    UPLL_LOG_ERROR("keytype(%d) is not supported by controller(%s)",
        ikey->get_key_type(), ctrlr_name);
    return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
  }

  val_flowfilter_entry_t *val_flowfilter_entry =
      reinterpret_cast<val_flowfilter_entry_t *>(GetVal(ikey));
  if (max_attrs > 0) {
    return VbrFlowFilterEntryMoMgr::ValFlowFilterEntryAttributeSupportCheck(
           val_flowfilter_entry, attrs);
  } else {
    UPLL_LOG_ERROR("Attribute list is empty for operation %d", req->operation);
    return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
  }
}

upll_rc_t VtermIfFlowFilterEntryMoMgr::RestorePOMInCtrlTbl(
    ConfigKeyVal *ikey,
    upll_keytype_datatype_t dt_type,
    MoMgrTables tbl,
    DalDmlIntf* dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  controller_domain ctrlr_dom;
//  FlowListMoMgr *mgr = NULL;
// uint8_t *ctrlr_id = NULL;

  if (!ikey || !(ikey->get_key())) {
    UPLL_LOG_ERROR("Input Key Not Valid");
    return UPLL_RC_ERR_GENERIC;
  }
  if (tbl != MAINTBL ||
       ikey->get_key_type() != UNC_KT_VTERMIF_FLOWFILTER_ENTRY) {
    UPLL_LOG_ERROR("Ignoring  ktype/Table kt=%d, tbl=%d",
                    ikey->get_key_type(), tbl);
    // TODO(pom): check with Pynthamil
    return result_code;
  }

  memset(&ctrlr_dom, 0, sizeof(controller_domain));
  val_flowfilter_entry_t *flowfilter_val =
      reinterpret_cast<val_flowfilter_entry_t *> (GetVal(ikey));
  if (NULL == flowfilter_val) {
    UPLL_LOG_ERROR(" Value structure is NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  if (flowfilter_val->valid[UPLL_IDX_FLOWLIST_NAME_FFE] == UNC_VF_VALID) {
    result_code = GetControllerDomainID(ikey, dt_type, dmi);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_ERROR("Failed to Get the Controller Domain details, err:%d",
                     result_code);
      return result_code;
    }

    GET_USER_DATA_CTRLR_DOMAIN(ikey, ctrlr_dom);
    UPLL_LOG_DEBUG("ctrlrid %s, domainid %s",
                 ctrlr_dom.ctrlr, ctrlr_dom.domain);
  }
  return result_code;
}

upll_rc_t VtermIfFlowFilterEntryMoMgr::CreateCandidateMo(IpcReqRespHeader *req,
                                                       ConfigKeyVal *ikey,
                                                       DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  uint8_t *ctrlr_id = NULL;
  controller_domain ctrlr_dom;
  if (ikey == NULL || req == NULL || dmi == NULL) {
    UPLL_LOG_ERROR(
        "Cannot perform create operation due to insufficient parameters");
    return UPLL_RC_ERR_GENERIC;
  }
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  UPLL_LOG_TRACE("ikey %s", ikey->ToStrAll().c_str());

  if (req->datatype != UPLL_DT_IMPORT) {
    // validate syntax and semantics
    result_code = ValidateMessage(req, ikey);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_ERROR(" ValidateMessage failed ");
      return result_code;
    }
  }
  result_code = ValidateAttribute(ikey, dmi, req);
  if (UPLL_RC_SUCCESS != result_code) {
     UPLL_LOG_ERROR(" ValidateAttribute failed ");
     return result_code;
  }
  val_flowfilter_entry_t *flowfilter_val =
      reinterpret_cast<val_flowfilter_entry_t *> (GetVal(ikey));
  memset(&ctrlr_dom, 0, sizeof(controller_domain));
  result_code = GetControllerDomainID(ikey, req->datatype, dmi);
  if (result_code != UPLL_RC_SUCCESS) {
    if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
      result_code = UPLL_RC_ERR_PARENT_DOES_NOT_EXIST;
    }
    UPLL_LOG_ERROR("Failed to Get the Controller Domain details, err:%d",
                   result_code);
    return result_code;
  }

  GET_USER_DATA_CTRLR_DOMAIN(ikey, ctrlr_dom);
  UPLL_LOG_DEBUG("ctrlrid %s, domainid %s",
                 ctrlr_dom.ctrlr, ctrlr_dom.domain);
  ctrlr_id = ctrlr_dom.ctrlr;

  result_code = ValidateCapability(req, ikey,
                   reinterpret_cast<const char *>(ctrlr_id));
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_ERROR("validate Capability Failed %d", result_code);
    return result_code;
  }
  VtermIfMoMgr *mgrvtermif = reinterpret_cast<VtermIfMoMgr *>
      (const_cast<MoManager *>(GetMoManager(UNC_KT_VTERM_IF)));

  InterfacePortMapInfo flags = kVlinkPortMapNotConfigured;
  ConfigKeyVal *ff_ckv = NULL;
  result_code = GetParentConfigKey(ff_ckv, ikey);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_ERROR("GetParentConfigKey failed %d", result_code);
    return result_code;
  }
  VtermIfFlowFilterMoMgr *mgr = reinterpret_cast<VtermIfFlowFilterMoMgr *>
      (const_cast<MoManager *>(GetMoManager(UNC_KT_VTERMIF_FLOWFILTER)));
  ConfigKeyVal *vtermif_ckv = NULL;
  result_code = mgr->GetParentConfigKey(vtermif_ckv, ff_ckv);
  DELETE_IF_NOT_NULL(ff_ckv);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_ERROR("GetParentConfigKey failed %d", result_code);
    return result_code;
  }
  result_code = mgrvtermif->GetPortmapInfo(vtermif_ckv, req->datatype,
                                           dmi, flags);
  DELETE_IF_NOT_NULL(vtermif_ckv);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_ERROR("GetPortmapinfo Failed err_code %d", result_code);
    return result_code;
  }

  uint8_t flag_port_map = 0, iflag = 0;
  if (flags & kPortMapConfigured) {
    flag_port_map = flag_port_map | SET_FLAG_PORTMAP;
    UPLL_LOG_DEBUG("flag_port_map %d", flag_port_map);
  }
  GET_USER_DATA_FLAGS(ikey, iflag);
  UPLL_LOG_DEBUG("iflag %d", iflag);
  iflag |= flag_port_map;
  SET_USER_DATA_FLAGS(ikey, iflag);
  UPLL_LOG_DEBUG(" flag_port_map (%d) (%d)", iflag, flag_port_map);
  TcConfigMode config_mode = TC_CONFIG_INVALID;
  std::string vtn_name = "";
  result_code = GetConfigModeInfo(req, config_mode, vtn_name);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetConfigMode failed");
    return result_code;
  }

  if (flowfilter_val->valid[UPLL_IDX_FLOWLIST_NAME_FFE] == UNC_VF_VALID) {
    if (flag_port_map & SET_FLAG_PORTMAP) {
      FlowListMoMgr *mgr = reinterpret_cast<FlowListMoMgr *>
        (const_cast<MoManager *> (GetMoManager(UNC_KT_FLOWLIST)));
      std::string temp_vtn_name;
      if (TC_CONFIG_VTN == config_mode) {
        temp_vtn_name = vtn_name;
      } else {
        key_vterm_if_flowfilter_entry_t *temp_key = reinterpret_cast<
            key_vterm_if_flowfilter_entry_t *>(ikey->get_key());
        temp_vtn_name = reinterpret_cast<const char*>(
            temp_key->flowfilter_key.if_key.vterm_key.vtn_key.vtn_name);
      }
      result_code = mgr->AddFlowListToController(
          reinterpret_cast<char *>(flowfilter_val->flowlist_name), dmi,
          reinterpret_cast<char *> (ctrlr_id), req->datatype, UNC_OP_CREATE,
          config_mode, temp_vtn_name, false);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Reference Count Updation Fails %d", result_code);
        return result_code;
      }
    }
  }
  DbSubOp dbop1 = { kOpNotRead, kOpMatchNone, kOpInOutDomain
    | kOpInOutCtrlr | kOpInOutFlag };
  result_code = UpdateConfigDB(ikey, req->datatype, UNC_OP_CREATE, dmi,
                               &dbop1, config_mode, vtn_name, MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_ERROR("Unable to update CandidateDB %d", result_code);
  }
  return result_code;
}

upll_rc_t VtermIfFlowFilterEntryMoMgr::CreateAuditMoImpl(ConfigKeyVal *ikey,
                                   DalDmlIntf *dmi,
                                   const char *ctrlr_id) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  string vtn_name = "";
  UPLL_LOG_TRACE(" ikey is %s", ikey->ToStrAll().c_str());

  VtermIfMoMgr *mgrvtermif = reinterpret_cast<VtermIfMoMgr *>
        (const_cast<MoManager *>(GetMoManager(UNC_KT_VTERM_IF)));
  InterfacePortMapInfo flags = kVlinkPortMapNotConfigured;
  ConfigKeyVal *ff_ckv = NULL;
  result_code = GetParentConfigKey(ff_ckv, ikey);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_ERROR("GetParentConfigKey failed %d", result_code);
    return result_code;
  }
  VtermIfFlowFilterMoMgr *mgr = reinterpret_cast<VtermIfFlowFilterMoMgr *>
      (const_cast<MoManager *>(GetMoManager(UNC_KT_VTERMIF_FLOWFILTER)));
  ConfigKeyVal *vtermif_ckv = NULL;
  result_code = mgr->GetParentConfigKey(vtermif_ckv, ff_ckv);
  DELETE_IF_NOT_NULL(ff_ckv);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_ERROR("GetParentConfigKey failed %d", result_code);
    return result_code;
  }
  result_code = mgrvtermif->GetPortmapInfo(vtermif_ckv, UPLL_DT_AUDIT,
                                           dmi, flags);
  DELETE_IF_NOT_NULL(vtermif_ckv);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_ERROR("GetPortmapinfo Failed err_code %d", result_code);
    return result_code;
  }

  uint8_t flag_port_map = 0;
  GET_USER_DATA_FLAGS(ikey, flag_port_map);
  if (flags & kPortMapConfigured) {
    flag_port_map |= SET_FLAG_PORTMAP;
  }

  SET_USER_DATA_FLAGS(ikey, flag_port_map);

  controller_domain ctrlr_dom;
  memset(&ctrlr_dom, 0, sizeof(ctrlr_dom));
  result_code = GetControllerDomainID(ikey, UPLL_DT_AUDIT, dmi);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_ERROR("Failed to Get the Controller Domain details, err:%d",
                   result_code);
    return result_code;
  }

  GET_USER_DATA_CTRLR_DOMAIN(ikey, ctrlr_dom);
  UPLL_LOG_DEBUG("ctrlrid %s, domainid %s",
                 ctrlr_dom.ctrlr, ctrlr_dom.domain);

  val_flowfilter_entry_t *val_ffe = reinterpret_cast
                                    <val_flowfilter_entry_t *>(GetVal(ikey));
  if (val_ffe->valid[UPLL_IDX_FLOWLIST_NAME_FFE] == UNC_VF_VALID) {
    FlowListMoMgr *mgr = reinterpret_cast<FlowListMoMgr *>
          (const_cast<MoManager *> (GetMoManager(UNC_KT_FLOWLIST)));
    if (mgr == NULL) {
      UPLL_LOG_ERROR("Invalid FlowListMoMgr Instance");
      return UPLL_RC_ERR_GENERIC;
    }
    result_code = mgr->AddFlowListToController(
        reinterpret_cast<char *>
        (val_ffe->flowlist_name), dmi,
        reinterpret_cast<char *> (const_cast<char *>(ctrlr_id)),
        UPLL_DT_AUDIT, UNC_OP_CREATE, TC_CONFIG_GLOBAL, vtn_name, false);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_ERROR("Reference Count Updation Fails %d", result_code);
      return result_code;
    }
  }

  UPLL_LOG_TRACE("ikey After GetRenamedUncKey %s", ikey->ToStrAll().c_str());
  // Create a record in AUDIT DB
  result_code = UpdateConfigDB(ikey, UPLL_DT_AUDIT,
                               UNC_OP_CREATE, dmi, TC_CONFIG_GLOBAL,
                               vtn_name, MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_ERROR("UpdateConfigDB Failed err_code %d", result_code);
    return result_code;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtermIfFlowFilterEntryMoMgr::AuditUpdateController(
    unc_key_type_t keytype,
    const char *ctrlr_id,
    uint32_t session_id,
    uint32_t config_id,
    uuc::UpdateCtrlrPhase phase1,
    DalDmlIntf *dmi,
    ConfigKeyVal **err_ckv,
    KTxCtrlrAffectedState *ctrlr_affected) {

  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  DalResultCode db_result = uud::kDalRcSuccess;
  controller_domain_t ctrlr_dom;
  ctrlr_dom.ctrlr = NULL;
  ctrlr_dom.domain = NULL;
  ConfigKeyVal  *ckv_running_db = NULL;
  ConfigKeyVal  *ckv_audit_db = NULL;
  ConfigKeyVal  *ckv_driver_req = NULL;
  ConfigKeyVal  *ckv_audit_dup_db = NULL;
  DalCursor *cursor = NULL;
  uint8_t db_flag = 0;
  string vtn_name = "";
  uint8_t *ctrlr = reinterpret_cast<uint8_t *>(const_cast<char *>(ctrlr_id));

  // Skipping the create phase if it comes as an input.
  // vbr if flowfilter should get applied on controller(pfc) if portma/vlink is
  // configured.
  // The portmap/vlink request should come in the update phase so
  // the vbrif policingmap creation should also be applied during update phase.
  if (phase1 == uuc::kUpllUcpCreate) {
    UPLL_LOG_DEBUG("Create phase is skipped");
    return result_code;
  }
  unc_keytype_operation_t op[2] = {UNC_OP_INVALID, UNC_OP_INVALID};
  int nop = 0;
  if (phase1 == uuc::kUpllUcpUpdate) {
    op[0] = UNC_OP_UPDATE;
    op[1] = UNC_OP_CREATE;
    nop = 2;
  } else if (phase1 == uuc::kUpllUcpDelete) {
    op[0] = UNC_OP_DELETE;
    nop = 1;
  }

  for (int i = 0; i < nop; i++) {
    UPLL_LOG_DEBUG("Operation is %d", op[i]);
    cursor = NULL;
    unc_keytype_operation_t op1 = op[i];
    uuc::UpdateCtrlrPhase phase = (op[i] == UNC_OP_UPDATE)?uuc::kUpllUcpUpdate:
      ((op[i] == UNC_OP_CREATE)?uuc::kUpllUcpCreate:
       ((op[i] == UNC_OP_DELETE)?uuc::kUpllUcpDelete:uuc::kUpllUcpInvalid));
    result_code = DiffConfigDB(UPLL_DT_RUNNING, UPLL_DT_AUDIT, op[i],
        ckv_running_db, ckv_audit_db,
        &cursor, dmi, ctrlr, TC_CONFIG_GLOBAL, vtn_name, MAINTBL, true, true);
    if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
      result_code = UPLL_RC_SUCCESS;
      UPLL_LOG_DEBUG("No more diff found for operation %d", op[i]);
      DELETE_IF_NOT_NULL(ckv_running_db);
      DELETE_IF_NOT_NULL(ckv_audit_db);
      continue;
    }
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_ERROR("DiffConfigDB failed - %d", result_code);
      if (cursor)
        dmi->CloseCursor(cursor, true);
      DELETE_IF_NOT_NULL(ckv_running_db);
      DELETE_IF_NOT_NULL(ckv_audit_db);
      return result_code;
    }
    if (cursor == NULL) {
      UPLL_LOG_ERROR("cursor is null");
      DELETE_IF_NOT_NULL(ckv_running_db);
      DELETE_IF_NOT_NULL(ckv_audit_db);
      return UPLL_RC_ERR_GENERIC;
    }
    while (uud::kDalRcSuccess == (db_result = dmi->GetNextRecord(cursor)) &&
           ((result_code = ContinueAuditProcess()) == UPLL_RC_SUCCESS)) {
      op1 = op[i];
      if (phase != uuc::kUpllUcpDelete) {
        uint8_t *db_ctrlr = NULL;
        GET_USER_DATA_CTRLR(ckv_running_db, db_ctrlr);
        UPLL_LOG_DEBUG("db ctrl_id and audit ctlr_id are  %s %s",
                        db_ctrlr, ctrlr_id);
        // Skipping the controller ID if the controller id in DB and
        // controller id available for Audit are not the same
        if (db_ctrlr && strncmp(reinterpret_cast<const char *>(db_ctrlr),
              reinterpret_cast<const char *>(ctrlr_id),
              strlen(reinterpret_cast<const char *>(ctrlr_id)) + 1)) {
          continue;
        }
      }
      switch (phase) {
        case uuc::kUpllUcpDelete:
        case uuc::kUpllUcpCreate:
          UPLL_LOG_TRACE("Created  record is %s ",
                         ckv_running_db->ToStrAll().c_str());
          result_code = DupConfigKeyVal(ckv_driver_req,
                                        ckv_running_db, MAINTBL);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_ERROR("DupConfigKeyVal failed. err_code & phase %d %d",
                result_code, phase);
            DELETE_IF_NOT_NULL(ckv_running_db);
            DELETE_IF_NOT_NULL(ckv_audit_db);
            dmi->CloseCursor(cursor, true);
            return result_code;
          }
          break;
        case uuc::kUpllUcpUpdate:
          ckv_audit_dup_db = NULL;
          ckv_driver_req = NULL;
          UPLL_LOG_TRACE("UpdateRecord  record  is %s ",
              ckv_running_db->ToStrAll().c_str());
          UPLL_LOG_TRACE("UpdateRecord  record  is %s ",
              ckv_audit_db->ToStrAll().c_str());
          result_code = DupConfigKeyVal(ckv_driver_req,
                                        ckv_running_db, MAINTBL);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_ERROR("DupConfigKeyVal failed for running record. "
                           "err_code & phase %d %d", result_code, phase);
            DELETE_IF_NOT_NULL(ckv_running_db);
            DELETE_IF_NOT_NULL(ckv_audit_db);
            dmi->CloseCursor(cursor, true);
            return result_code;
          }
          result_code = DupConfigKeyVal(ckv_audit_dup_db,
                                        ckv_audit_db, MAINTBL);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_ERROR("DupConfigKeyVal failed for audit record. "
                           "err_code & phase %d %d", result_code, phase);
            DELETE_IF_NOT_NULL(ckv_running_db);
            DELETE_IF_NOT_NULL(ckv_audit_db);
            DELETE_IF_NOT_NULL(ckv_driver_req);
            dmi->CloseCursor(cursor, true);
            return result_code;
          }
          break;
        default:
          UPLL_LOG_DEBUG("Invalid operation %d", phase);
          DELETE_IF_NOT_NULL(ckv_running_db);
          DELETE_IF_NOT_NULL(ckv_audit_db);
          dmi->CloseCursor(cursor, true);
          return UPLL_RC_ERR_NO_SUCH_OPERATION;
          break;
      }
      GET_USER_DATA_CTRLR_DOMAIN(ckv_driver_req, ctrlr_dom);
      if ((NULL == ctrlr_dom.ctrlr) || (NULL == ctrlr_dom.domain)) {
        UPLL_LOG_ERROR("controller id or domain is NULL");
        DELETE_IF_NOT_NULL(ckv_running_db);
        DELETE_IF_NOT_NULL(ckv_audit_db);
        DELETE_IF_NOT_NULL(ckv_driver_req);
        DELETE_IF_NOT_NULL(ckv_audit_dup_db);
        dmi->CloseCursor(cursor, true);
        return UPLL_RC_ERR_GENERIC;
      }
      UPLL_LOG_DEBUG("Controller : %s; Domain : %s", ctrlr_dom.ctrlr,
          ctrlr_dom.domain);
      db_flag = 0;
      GET_USER_DATA_FLAGS(ckv_driver_req, db_flag);
      UPLL_LOG_DEBUG("dbflag 1 - %d", db_flag);
      // If portmap/vlink flag is not set at running and the operation is
      // update then portmap/vlink is deleted in the update phase from UNC
      // hence flowfilter seq no also should get deleted from controller
      // hence sending the delete request to the controller driver
      if (SET_FLAG_PORTMAP & db_flag) {
        // Continue with further operations
      } else {
        if (UNC_OP_UPDATE == op1) {
          op1 = UNC_OP_DELETE;
        } else {
          // No PortMap/Vlink Configured,
          // Configuration is not Sent to driver
          DELETE_IF_NOT_NULL(ckv_driver_req);
          DELETE_IF_NOT_NULL(ckv_audit_dup_db);
          continue;
        }
      }
      if (UNC_OP_UPDATE == op1) {
        void *running_val = NULL;
        bool invalid_attr = false;
        running_val = GetVal(ckv_driver_req);
        invalid_attr = FilterAttributes(running_val,
            GetVal(ckv_audit_dup_db), false, UNC_OP_UPDATE);
        if (invalid_attr) {
          DELETE_IF_NOT_NULL(ckv_driver_req);
          DELETE_IF_NOT_NULL(ckv_audit_dup_db);
          // Assuming that the diff found only in ConfigStatus
          // Setting the value as OnlyCSDiff in the out parameter ctrlr_affected
          // The value Configdiff should be given more priority than the value
          // onlycs . So if the out parameter ctrlr_affected has already
          // value as configdiff then dont change the value
          if (*ctrlr_affected != uuc::kCtrlrAffectedConfigDiff) {
             UPLL_LOG_INFO("Setting the ctrlr_affected to OnlyCSDiff");
             *ctrlr_affected = uuc::kCtrlrAffectedOnlyCSDiff;
          }
          continue;
        }
      }

      DELETE_IF_NOT_NULL(ckv_audit_dup_db);

      upll_keytype_datatype_t dt_type = (op1 == UNC_OP_DELETE)?
        UPLL_DT_AUDIT:UPLL_DT_RUNNING;

      if (op1 != UNC_OP_DELETE) {
        result_code = PerformRedirectTranslationForAudit(ckv_driver_req,
                                                         dmi,
                                                         dt_type);
        if (UPLL_RC_SUCCESS != result_code) {
          UPLL_LOG_DEBUG("Unable to perform redirect node translation %d",
                         result_code);
          DELETE_IF_NOT_NULL(ckv_driver_req);
          DELETE_IF_NOT_NULL(ckv_running_db);
          DELETE_IF_NOT_NULL(ckv_audit_db);
          dmi->CloseCursor(cursor, true);
          return result_code;
        }
      }
      result_code = GetRenamedControllerKey(ckv_driver_req, dt_type,
          dmi, &ctrlr_dom);

      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_ERROR(" GetRenamedControllerKey failed err code(%d)",
            result_code);
        DELETE_IF_NOT_NULL(ckv_driver_req);
        DELETE_IF_NOT_NULL(ckv_running_db);
        DELETE_IF_NOT_NULL(ckv_audit_db);
        dmi->CloseCursor(cursor, true);
        return result_code;
      }

      if (op1 == UNC_OP_UPDATE) {
        ConfigVal *old_cval = ((ckv_audit_db->get_cfg_val())->DupVal());
        ckv_driver_req->AppendCfgVal(old_cval);
      }

      IpcResponse ipc_response;
      memset(&ipc_response, 0, sizeof(IpcResponse));
      IpcRequest ipc_req;
      memset(&ipc_req, 0, sizeof(IpcRequest));
      ipc_req.header.clnt_sess_id = session_id;
      ipc_req.header.config_id = config_id;
      ipc_req.header.operation = op1;
      ipc_req.header.datatype = UPLL_DT_CANDIDATE;
      ipc_req.ckv_data = ckv_driver_req;
      if (!IpcUtil::SendReqToDriver(
              (const char *)ctrlr_dom.ctrlr,
              reinterpret_cast<char *> (ctrlr_dom.domain),
              PFCDRIVER_SERVICE_NAME, PFCDRIVER_SVID_LOGICAL,
              &ipc_req, true, &ipc_response)) {
        UPLL_LOG_ERROR("Request to driver for Key %d for controller %s failed ",
            ckv_driver_req->get_key_type(),
            reinterpret_cast<char *>(ctrlr_dom.ctrlr));
        DELETE_IF_NOT_NULL(ckv_driver_req);
        DELETE_IF_NOT_NULL(ckv_running_db);
        DELETE_IF_NOT_NULL(ckv_audit_db);
        dmi->CloseCursor(cursor, true);
        return ipc_response.header.result_code;
      }
      if (ipc_response.header.result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("driver return failure err_code is %d",
                       ipc_response.header.result_code);
        *err_ckv = ckv_running_db;
        if (phase != uuc::kUpllUcpDelete) {
          ConfigKeyVal *resp = NULL;
          result_code = DupConfigKeyVal(resp, ipc_response.ckv_data, MAINTBL);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_ERROR("DupConfigKeyVal Failed");
            DELETE_IF_NOT_NULL(ckv_driver_req);
            DELETE_IF_NOT_NULL(ipc_response.ckv_data);
            DELETE_IF_NOT_NULL(ckv_running_db);
            DELETE_IF_NOT_NULL(ckv_audit_db);
            *err_ckv = NULL;
            dmi->CloseCursor(cursor, true);
            return result_code;
          }

          result_code = UpdateAuditConfigStatus(UNC_CS_INVALID, phase,
                                                resp, dmi);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_ERROR("Update Audit config status failed %d",
                result_code);
            DELETE_IF_NOT_NULL(ckv_driver_req);
            DELETE_IF_NOT_NULL(resp);
            DELETE_IF_NOT_NULL(ipc_response.ckv_data);
            DELETE_IF_NOT_NULL(ckv_running_db);
            DELETE_IF_NOT_NULL(ckv_audit_db);
            *err_ckv = NULL;
            dmi->CloseCursor(cursor, true);
            return result_code;
          }

          DbSubOp dbop = { kOpNotRead, kOpMatchNone, kOpInOutCs };
          result_code = UpdateConfigDB(resp, dt_type, UNC_OP_UPDATE,
              dmi, &dbop, TC_CONFIG_GLOBAL, vtn_name, MAINTBL);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_ERROR(
                "UpdateConfigDB failed for ipc response ckv err_code %d",
                result_code);
            DELETE_IF_NOT_NULL(ckv_driver_req);
            DELETE_IF_NOT_NULL(resp);
            DELETE_IF_NOT_NULL(ipc_response.ckv_data);
            DELETE_IF_NOT_NULL(ckv_running_db);
            DELETE_IF_NOT_NULL(ckv_audit_db);
            err_ckv = NULL;
            dmi->CloseCursor(cursor, true);
            return result_code;
          }
          DELETE_IF_NOT_NULL(resp);
        }
        return ipc_response.header.result_code;
      }
      DELETE_IF_NOT_NULL(ckv_driver_req);
      DELETE_IF_NOT_NULL(ipc_response.ckv_data);
      // *ctrlr_affected = true;
      if (*ctrlr_affected == uuc::kCtrlrAffectedOnlyCSDiff) {
        UPLL_LOG_INFO("Reset ctrlr state from OnlyCSDiff to ConfigDiff");
      }
      UPLL_LOG_DEBUG("Setting the ctrlr_affected to ConfigDiff");

      *ctrlr_affected = uuc::kCtrlrAffectedConfigDiff;
    }
    dmi->CloseCursor(cursor, true);
    DELETE_IF_NOT_NULL(ckv_running_db);
    DELETE_IF_NOT_NULL(ckv_audit_db);
  }
  if (uud::kDalRcSuccess != db_result) {
    UPLL_LOG_DEBUG("GetNextRecord from database failed  - %d", db_result);
    result_code =  DalToUpllResCode(db_result);
  }
  result_code = (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE)
    ? UPLL_RC_SUCCESS : result_code;
  return result_code;
}

upll_rc_t VtermIfFlowFilterEntryMoMgr::DeleteMo(IpcReqRespHeader *req,
                                              ConfigKeyVal *ikey,
                                              DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  uint8_t *ctrlr_id = NULL;
  upll_rc_t result_code = UPLL_RC_ERR_GENERIC;
  if (NULL == ikey || NULL == req || NULL == dmi) {
    UPLL_LOG_ERROR("Invalid input parameters");
    return result_code;
  }

  result_code = ValidateMessage(req, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
     UPLL_LOG_ERROR("ValidateMessage failed %d", result_code);
     return result_code;
  }

  result_code = UpdateConfigDB(ikey, UPLL_DT_CANDIDATE, UNC_OP_READ, dmi);
  if (UPLL_RC_ERR_INSTANCE_EXISTS != result_code) {
    UPLL_LOG_ERROR("Instance does not exists %d", result_code);
    return result_code;
  }

  ConfigKeyVal *okey = NULL;
  result_code = GetChildConfigKey(okey, ikey);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_ERROR("GetChildConfigKey failed %d", result_code);
    return result_code;
  }
  DbSubOp dbop = { kOpReadSingle, kOpMatchNone,
                   kOpInOutCtrlr|kOpInOutFlag };
  result_code = ReadConfigDB(okey, UPLL_DT_CANDIDATE,
      UNC_OP_READ, dbop, dmi, MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_ERROR("Unable to read configuration from CandidateDb");
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

  GET_USER_DATA_CTRLR(okey, ctrlr_id);
  val_flowfilter_entry_t *flowfilter_val =
    reinterpret_cast<val_flowfilter_entry_t *> (GetVal(okey));
  if (flowfilter_val->valid[UPLL_IDX_FLOWLIST_NAME_FFE] == UNC_VF_VALID) {
    uint8_t flag_port_map = 0;
    GET_USER_DATA_FLAGS(okey, flag_port_map);
    if (flag_port_map & SET_FLAG_PORTMAP) {
      FlowListMoMgr *mgr = reinterpret_cast<FlowListMoMgr *>
        (const_cast<MoManager *> (GetMoManager(UNC_KT_FLOWLIST)));
      std::string temp_vtn_name;
      if (TC_CONFIG_VTN == config_mode) {
        temp_vtn_name = vtn_name;
      } else {
        key_vterm_if_flowfilter_entry_t *temp_key = reinterpret_cast<
            key_vterm_if_flowfilter_entry_t *>(ikey->get_key());
        temp_vtn_name = reinterpret_cast<const char*>(
            temp_key->flowfilter_key.if_key.vterm_key.vtn_key.vtn_name);
      }
      result_code = mgr->AddFlowListToController(
          reinterpret_cast<char*>(flowfilter_val->flowlist_name), dmi,
          reinterpret_cast<char *>(ctrlr_id), req->datatype, UNC_OP_DELETE,
          config_mode, temp_vtn_name, false);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_ERROR(" Send delete request to flowlist failed. err code(%d)",
            result_code);
        DELETE_IF_NOT_NULL(okey);
        return result_code;
      }
    }
  }
  result_code = UpdateConfigDB(ikey, UPLL_DT_CANDIDATE, UNC_OP_DELETE, dmi,
                             config_mode, vtn_name, MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_ERROR("UpdateConfigDB failed to delete %d", result_code);
    DELETE_IF_NOT_NULL(okey);
    return result_code;
  }
  DELETE_IF_NOT_NULL(okey);
  return UPLL_RC_SUCCESS;
}
upll_rc_t VtermIfFlowFilterEntryMoMgr::GetChildConfigKey(
    ConfigKeyVal *&okey, ConfigKeyVal *parent_key) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  key_vterm_if_flowfilter_entry_t *vterm_if_ffe_key = NULL;
  void *pkey = NULL;

  if (parent_key == NULL) {
    vterm_if_ffe_key = reinterpret_cast<key_vterm_if_flowfilter_entry_t*>
        (ConfigKeyVal::Malloc(sizeof(key_vterm_if_flowfilter_entry_t)));
    // If no direction is specified , 0xFE is filled to bind output direction
    vterm_if_ffe_key->flowfilter_key.direction = 0xFE;
    okey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVtermIfFlowfilterEntry,
                            vterm_if_ffe_key, NULL);
    UPLL_LOG_DEBUG("Parent Key Filled ");
    return UPLL_RC_SUCCESS;
  } else {
    pkey = parent_key->get_key();
  }
  if (!pkey) {
    UPLL_LOG_ERROR("Parent Key is NULL");
    return UPLL_RC_ERR_GENERIC;
  }

  if (okey) {
    if (okey->get_key_type() != UNC_KT_VTERMIF_FLOWFILTER_ENTRY) {
      UPLL_LOG_ERROR("Wrong KT");
      return UPLL_RC_ERR_GENERIC;
    }
  }

  if ((okey) && (okey->get_key())) {
    vterm_if_ffe_key = reinterpret_cast<key_vterm_if_flowfilter_entry_t *>
        (okey->get_key());
  } else {
    vterm_if_ffe_key = reinterpret_cast<key_vterm_if_flowfilter_entry_t *>
        (ConfigKeyVal::Malloc(sizeof(key_vterm_if_flowfilter_entry_t)));
    vterm_if_ffe_key->flowfilter_key.direction = 0xFE;
  }
  switch (parent_key->get_key_type()) {
    case UNC_KT_VTN:
      uuu::upll_strncpy(
          vterm_if_ffe_key->flowfilter_key.if_key.vterm_key.vtn_key.vtn_name,
          reinterpret_cast<key_vtn_t *>
          (pkey)->vtn_name,
          kMaxLenVtnName + 1);
      break;
    case UNC_KT_VTERMINAL:
      uuu::upll_strncpy(
          vterm_if_ffe_key->flowfilter_key.if_key.vterm_key.vtn_key.vtn_name,
          reinterpret_cast<key_vterm_t *>
          (pkey)->vtn_key.vtn_name,
          kMaxLenVtnName + 1);
      uuu::upll_strncpy(
          vterm_if_ffe_key->flowfilter_key.if_key.vterm_key.vterminal_name,
          reinterpret_cast<key_vterm_t *>
          (pkey)->vterminal_name,
          kMaxLenVnodeName + 1);
      break;
    case UNC_KT_VTERM_IF:
      uuu::upll_strncpy(
          vterm_if_ffe_key->flowfilter_key.if_key.vterm_key.vtn_key.vtn_name,
          reinterpret_cast<key_vterm_if_t *>
          (pkey)->vterm_key.vtn_key.vtn_name,
          kMaxLenVtnName + 1);
      uuu::upll_strncpy(
          vterm_if_ffe_key->flowfilter_key.if_key.vterm_key.vterminal_name,
          reinterpret_cast<key_vterm_if_t *>
          (pkey)->vterm_key.vterminal_name,
          kMaxLenVnodeName + 1);
      uuu::upll_strncpy(vterm_if_ffe_key->flowfilter_key.if_key.if_name,
                        reinterpret_cast<key_vterm_if_t *>
                        (pkey)->if_name,
                        kMaxLenInterfaceName + 1);
      break;
    case UNC_KT_VTERMIF_FLOWFILTER:
      uuu::upll_strncpy(
          vterm_if_ffe_key->flowfilter_key.if_key.vterm_key.vtn_key.vtn_name,
          reinterpret_cast<key_vterm_if_flowfilter_t *>
          (pkey)->if_key.vterm_key.vtn_key.vtn_name,
          kMaxLenVtnName + 1);
      uuu::upll_strncpy(
          vterm_if_ffe_key->flowfilter_key.if_key.vterm_key.vterminal_name,
          reinterpret_cast<key_vterm_if_flowfilter_t *>
          (pkey)->if_key.vterm_key.vterminal_name,
          kMaxLenVnodeName + 1);
      uuu::upll_strncpy(vterm_if_ffe_key->flowfilter_key.if_key.if_name,
                        reinterpret_cast<key_vterm_if_flowfilter_t *>
                        (pkey)->if_key.if_name,
                        kMaxLenInterfaceName + 1);
      vterm_if_ffe_key->flowfilter_key.direction =
          reinterpret_cast<key_vterm_if_flowfilter_t *>
          (pkey)->direction;
      break;
    case UNC_KT_VTERMIF_FLOWFILTER_ENTRY:
      uuu::upll_strncpy(
          vterm_if_ffe_key->flowfilter_key.if_key.vterm_key.vtn_key.vtn_name,
          reinterpret_cast<key_vterm_if_flowfilter_entry_t *>
          (pkey)->flowfilter_key.if_key.vterm_key.vtn_key.vtn_name,
          kMaxLenVtnName + 1);
      uuu::upll_strncpy(
          vterm_if_ffe_key->flowfilter_key.if_key.vterm_key.vterminal_name,
          reinterpret_cast<key_vterm_if_flowfilter_entry_t *>
          (pkey)->flowfilter_key.if_key.vterm_key.vterminal_name,
          kMaxLenVnodeName + 1);
      uuu::upll_strncpy(vterm_if_ffe_key->flowfilter_key.if_key.if_name,
          reinterpret_cast<key_vterm_if_flowfilter_entry_t *>
          (pkey)->flowfilter_key.if_key.if_name,
          kMaxLenInterfaceName + 1);
      vterm_if_ffe_key->flowfilter_key.direction =
          reinterpret_cast<key_vterm_if_flowfilter_entry_t *>
          (pkey)->flowfilter_key.direction;
      vterm_if_ffe_key->sequence_num =
          reinterpret_cast<key_vterm_if_flowfilter_entry_t *>
          (pkey)->sequence_num;
      break;
    case UNC_KT_VBR_NWMONITOR:
      uuu::upll_strncpy(
          vterm_if_ffe_key->flowfilter_key.if_key.vterm_key.vtn_key.vtn_name,
          reinterpret_cast<key_vterm_if_flowfilter_entry_t *>
          (pkey)->flowfilter_key.if_key.vterm_key.vtn_key.vtn_name,
          kMaxLenVtnName + 1);
      break;
    default:
      UPLL_LOG_ERROR("Wrong KT");
      if ((NULL == okey) || (NULL == okey->get_key())) {
        free(vterm_if_ffe_key);
      }
      return UPLL_RC_ERR_GENERIC;
  }

  if ((okey) && !(okey->get_key())) {
    UPLL_LOG_DEBUG("okey not null and flow list name updated");
    okey->SetKey(IpctSt::kIpcStKeyVtermIfFlowfilterEntry, vterm_if_ffe_key);
  }

  if (!okey) {
    okey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVtermIfFlowfilterEntry,
                            vterm_if_ffe_key, NULL);
  }
  SET_USER_DATA(okey, parent_key);
  UPLL_LOG_DEBUG("GetChildConfigKey :: okey filled Succesfully ");
  return result_code;
}

upll_rc_t VtermIfFlowFilterEntryMoMgr::DupConfigKeyVal(ConfigKeyVal *&okey,
                                                     ConfigKeyVal *&req,
                                                     MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  if (req == NULL) {
    UPLL_LOG_ERROR("Request is null");
    return UPLL_RC_ERR_GENERIC;
  }
  if (okey != NULL) {
    UPLL_LOG_ERROR("Request is null");
    return UPLL_RC_ERR_GENERIC;
  }
  if (req->get_key_type() != UNC_KT_VTERMIF_FLOWFILTER_ENTRY) {
    UPLL_LOG_ERROR(" DupConfigKeyval Failed.");
    return UPLL_RC_ERR_GENERIC;
  }
  ConfigVal *tmp1 = NULL, *tmp = (req)->get_cfg_val();

  if (tmp) {
    if (tbl == MAINTBL) {
      val_flowfilter_entry_t *ival = reinterpret_cast <val_flowfilter_entry_t*>
          (GetVal(req));

      if (NULL != ival) {
      val_flowfilter_entry_t *flowfilter_val = NULL;
        flowfilter_val = reinterpret_cast<val_flowfilter_entry_t *>
            (ConfigKeyVal::Malloc(sizeof(val_flowfilter_entry_t)));

        memcpy(flowfilter_val, ival, sizeof(val_flowfilter_entry_t));
        tmp1 = new ConfigVal(IpctSt::kIpcStValFlowfilterEntry,
                             flowfilter_val);
        tmp1->set_user_data(tmp->get_user_data());
      }
    }
  }

  void *tkey = (req != NULL) ? (req)->get_key() : NULL;

  if (tkey == NULL) {
    DELETE_IF_NOT_NULL(tmp1);
    UPLL_LOG_ERROR("key from request is null");
    return UPLL_RC_ERR_GENERIC;
  }

  key_vterm_if_flowfilter_entry_t *ikey = reinterpret_cast
      <key_vterm_if_flowfilter_entry_t *> (tkey);
  key_vterm_if_flowfilter_entry_t *vterm_if_flowfilter_entry =
      reinterpret_cast<key_vterm_if_flowfilter_entry_t*>
      (ConfigKeyVal::Malloc(sizeof(key_vterm_if_flowfilter_entry_t)));
  memcpy(vterm_if_flowfilter_entry, ikey,
         sizeof(key_vterm_if_flowfilter_entry_t));
  okey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
                          IpctSt::kIpcStKeyVtermIfFlowfilterEntry,
                          vterm_if_flowfilter_entry, tmp1);
  if (okey) {
    SET_USER_DATA(okey, req)
  } else {
    UPLL_LOG_ERROR("okey is Null");
    FREE_IF_NOT_NULL(vterm_if_flowfilter_entry);
    DELETE_IF_NOT_NULL(tmp1);
    return UPLL_RC_ERR_GENERIC;
  }

  UPLL_LOG_DEBUG("DupConfigkeyVal Succesfull.");
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtermIfFlowFilterEntryMoMgr::GetRenamedUncKey(
  ConfigKeyVal *ikey, upll_keytype_datatype_t dt_type, DalDmlIntf *dmi,
  uint8_t *ctrlr_id) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *unc_key = NULL;
  DbSubOp dbop = { kOpReadSingle, kOpMatchCtrlr, kOpInOutNone };
  if ((NULL == ikey) || (ctrlr_id == NULL) || (NULL == dmi)) {
    UPLL_LOG_ERROR("ikey/ctrlr_id dmi NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  uint8_t rename = 0;
  UPLL_LOG_TRACE("%s GetRenamedUncKey vtermifff_entry start",
                  ikey->ToStrAll().c_str());
  MoMgrImpl *VtermMoMgr = static_cast<MoMgrImpl*>
    ((const_cast<MoManager*>(GetMoManager(UNC_KT_VTERMINAL))));
  if (NULL == VtermMoMgr) {
    UPLL_LOG_ERROR("VtermMoMgr NULL");
    return UPLL_RC_ERR_GENERIC;
  }

  val_rename_vnode *rename_val = reinterpret_cast<val_rename_vnode*>
      (ConfigKeyVal::Malloc(sizeof(val_rename_vnode)));
  if (!rename_val) {
    UPLL_LOG_ERROR("rename_val NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  key_vterm_if_flowfilter_entry_t *ctrlr_key = reinterpret_cast
    <key_vterm_if_flowfilter_entry_t *>(ikey->get_key());
  if (!ctrlr_key) {
    UPLL_LOG_ERROR("rename_val NULL");
    free(rename_val);
    return UPLL_RC_ERR_GENERIC;
  }
  uuu::upll_strncpy(
  rename_val->ctrlr_vtn_name,
  ctrlr_key->flowfilter_key.if_key.vterm_key.
  vtn_key.vtn_name, kMaxLenVtnName + 1);
  rename_val->valid[UPLL_CTRLR_VTN_NAME_VALID] = UNC_VF_VALID;

  uuu::upll_strncpy(rename_val->ctrlr_vnode_name,
                    ctrlr_key->flowfilter_key.if_key.vterm_key.vterminal_name,
                    kMaxLenVnodeName + 1);
  rename_val->valid[UPLL_CTRLR_VNODE_NAME_VALID] = UNC_VF_VALID;

  result_code = VtermMoMgr->GetChildConfigKey(unc_key, NULL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_ERROR("Failed to Get config key structure");
    free(rename_val);
    VtermMoMgr = NULL;
    return result_code;
  }
  if (!unc_key) {
    UPLL_LOG_ERROR("unc_key NULL");
    free(rename_val);
    VtermMoMgr = NULL;
    return UPLL_RC_ERR_GENERIC;
  }
  SET_USER_DATA_CTRLR(unc_key, ctrlr_id);
  unc_key->AppendCfgVal(IpctSt::kIpcStValRenameVtn, rename_val);
  result_code = VtermMoMgr->ReadConfigDB(unc_key, dt_type ,
                                  UNC_OP_READ, dbop, dmi, RENAMETBL);
  if ((UPLL_RC_SUCCESS != result_code) &&
      (UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code)) {
    UPLL_LOG_ERROR("ReadConfigDB failed %d", result_code);
    DELETE_IF_NOT_NULL(unc_key);
    VtermMoMgr = NULL;
    return result_code;
  }

  if (result_code == UPLL_RC_SUCCESS) {
    key_vterm_if_flowfilter_entry_t *vterm_flowfilter_entry_key =
        reinterpret_cast <key_vterm_if_flowfilter_entry_t *>
        (unc_key->get_key());
    if (strcmp(
            reinterpret_cast<char *>(ctrlr_key->flowfilter_key.if_key.
                                     vterm_key.vtn_key.vtn_name),
            reinterpret_cast<const char *>(vterm_flowfilter_entry_key->
                                           flowfilter_key.if_key.vterm_key.
                                           vtn_key.vtn_name))) {
      uuu::upll_strncpy(
      ctrlr_key->flowfilter_key.if_key.vterm_key.vtn_key.vtn_name,
      vterm_flowfilter_entry_key->flowfilter_key.if_key.
      vterm_key.vtn_key.vtn_name,
      kMaxLenVtnName + 1);
      rename |= VTN_RENAME_FLAG;
    }
    if (strcmp
        (reinterpret_cast<char *>(ctrlr_key->flowfilter_key.if_key.
                                  vterm_key.vterminal_name),
         reinterpret_cast<const char *>(vterm_flowfilter_entry_key->
                                        flowfilter_key.if_key.
                                        vterm_key.vterminal_name))) {
      uuu::upll_strncpy(
      ctrlr_key->flowfilter_key.if_key.vterm_key.vterminal_name,
      vterm_flowfilter_entry_key->flowfilter_key.if_key.
      vterm_key.vterminal_name,
      kMaxLenVnodeName + 1);
      rename |= VBR_RENAME_FLAG;
    }
    SET_USER_DATA(ikey, unc_key);
    SET_USER_DATA_FLAGS(ikey, rename);
  }
  VtermMoMgr = NULL;
  DELETE_IF_NOT_NULL(unc_key);

  val_flowfilter_entry_t *val_flowfilter_entry = reinterpret_cast
           <val_flowfilter_entry_t *>(GetVal(ikey));

  if (!val_flowfilter_entry) {
    UPLL_LOG_DEBUG("val_flowfilter_entry NULL");
    return UPLL_RC_SUCCESS;
  }

  if (UNC_VF_VALID == val_flowfilter_entry
                      ->valid[UPLL_IDX_FLOWLIST_NAME_FFE]) {
  val_rename_flowlist_t *rename_flowlist =
      reinterpret_cast<val_rename_flowlist_t*>
                   (ConfigKeyVal::Malloc(sizeof(val_rename_flowlist_t)));
  if (!rename_flowlist) {
    UPLL_LOG_ERROR("rename_flowlist NULL");
    free(rename_flowlist);
    return UPLL_RC_ERR_GENERIC;
  }

  uuu::upll_strncpy(rename_flowlist->flowlist_newname,
                    val_flowfilter_entry->flowlist_name,
                    (kMaxLenFlowListName + 1));
  rename_flowlist->valid[UPLL_IDX_RENAME_FLOWLIST_RFL] = UNC_VF_VALID;
  //  FlowList Renamed
  MoMgrImpl* mgr = static_cast<MoMgrImpl*>
             ((const_cast<MoManager*>(GetMoManager(UNC_KT_FLOWLIST))));
  if (NULL == mgr) {
    UPLL_LOG_ERROR("mgr NULL");
    FREE_IF_NOT_NULL(rename_flowlist);
    return UPLL_RC_ERR_GENERIC;
  }
  UPLL_LOG_TRACE("flowlist name (%s) (%s) ctrlr_name (%s)",
            rename_flowlist->flowlist_newname,
            val_flowfilter_entry->flowlist_name,
            ctrlr_id);
  result_code = mgr->GetChildConfigKey(unc_key, NULL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_ERROR("Failed to Get config key structure");
    if (rename_flowlist) free(rename_flowlist);
    mgr = NULL;
    return UPLL_RC_ERR_GENERIC;
  }

  SET_USER_DATA_CTRLR(unc_key, ctrlr_id);
  unc_key->AppendCfgVal(IpctSt::kIpcStValRenameFlowlist, rename_flowlist);
  result_code = mgr->ReadConfigDB(unc_key, dt_type, UNC_OP_READ, dbop, dmi,
                                RENAMETBL);
  if ((UPLL_RC_SUCCESS != result_code) &&
      (UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code)) {
    UPLL_LOG_ERROR("ReadConfigDB failed %d", result_code);
    DELETE_IF_NOT_NULL(unc_key);
    mgr = NULL;
    return result_code;
  }
  if (result_code == UPLL_RC_SUCCESS) {
  key_flowlist_t *key_flowlist = reinterpret_cast <key_flowlist_t *>
    (unc_key->get_key());
  uuu::upll_strncpy(val_flowfilter_entry->flowlist_name,
                    key_flowlist->flowlist_name,
                    (kMaxLenFlowListName + 1));
  rename |= FLOWLIST_RENAME_FLAG;
  SET_USER_DATA(ikey, unc_key);
  SET_USER_DATA_FLAGS(ikey, rename);
  }
  DELETE_IF_NOT_NULL(unc_key);
  mgr = NULL;
  }

  if ((UNC_VF_VALID ==
      val_flowfilter_entry->valid[UPLL_IDX_REDIRECT_NODE_FFE]) &&
      (UNC_VF_VALID ==
      val_flowfilter_entry->valid[UPLL_IDX_REDIRECT_PORT_FFE])) {
    unc_key_type_t child_key[]= { UNC_KT_VBRIDGE, UNC_KT_VROUTER,
                                  UNC_KT_VTERMINAL};
    bool isRedirectVnodeVbridge = false;
    for (unsigned int i = 0;
      i < sizeof(child_key)/sizeof(child_key[0]); i++) {
      const unc_key_type_t ktype = child_key[i];
      MoMgrImpl *mgrvbr = reinterpret_cast<MoMgrImpl *>(
          const_cast<MoManager *>(GetMoManager(ktype)));
      if (!mgrvbr) {
        UPLL_LOG_ERROR("mgrvbr failed");
        return UPLL_RC_ERR_GENERIC;
      }
      val_rename_vnode *rename_val = reinterpret_cast<val_rename_vnode*>
        (ConfigKeyVal::Malloc(sizeof(val_rename_vnode)));
      if (!rename_val) {
        UPLL_LOG_ERROR("rename_val NULL");
        return UPLL_RC_ERR_GENERIC;
      }

      uuu::upll_strncpy(rename_val->ctrlr_vnode_name,
                       val_flowfilter_entry->redirect_node,
                       (kMaxLenVnodeName + 1));
      rename_val->valid[UPLL_CTRLR_VNODE_NAME_VALID] = UNC_VF_VALID;

      result_code = mgrvbr->GetChildConfigKey(unc_key, NULL);
      if (UPLL_RC_SUCCESS != result_code) {
        UPLL_LOG_ERROR("GetChildConfigKey Returned an error");
        if (rename_val) free(rename_val);
        mgrvbr = NULL;
        return result_code;
      }
      SET_USER_DATA_CTRLR(unc_key, ctrlr_id);
      unc_key->AppendCfgVal(IpctSt::kIpcStValRenameVtn, rename_val);
      result_code = mgrvbr->ReadConfigDB(unc_key, dt_type,
                                         UNC_OP_READ, dbop, dmi,
        RENAMETBL);
      if ((UPLL_RC_SUCCESS != result_code) &&
        (UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code)) {
        UPLL_LOG_ERROR("ReadConfigDB failed %d", result_code);
        DELETE_IF_NOT_NULL(unc_key);
        mgrvbr = NULL;
        return result_code;
      }

      if (result_code == UPLL_RC_SUCCESS) {
        if (unc_key->get_key_type() == UNC_KT_VBRIDGE) {
          isRedirectVnodeVbridge = true;
          key_vbr *vbr_key = reinterpret_cast<key_vbr *>(unc_key->get_key());
          uuu::upll_strncpy(val_flowfilter_entry->redirect_node,
                          vbr_key->vbridge_name,
                          (kMaxLenVnodeName + 1));
        } else if (unc_key->get_key_type() == UNC_KT_VROUTER) {
          isRedirectVnodeVbridge = true;
          key_vrt *vrt_key = reinterpret_cast<key_vrt *>(unc_key->get_key());
          uuu::upll_strncpy(val_flowfilter_entry->redirect_node,
                           vrt_key->vrouter_name,
                          (kMaxLenVnodeName + 1));
        } else if (unc_key->get_key_type() == UNC_KT_VTERMINAL) {
          isRedirectVnodeVbridge = true;
          key_vterm_t *vterm_key =
              reinterpret_cast<key_vterm *>(unc_key->get_key());
          uuu::upll_strncpy(val_flowfilter_entry->redirect_node,
                            vterm_key->vterminal_name,
                            (kMaxLenVnodeName + 1));
        }
      }
      DELETE_IF_NOT_NULL(unc_key);
      mgrvbr = NULL;
      if (isRedirectVnodeVbridge) {
        UPLL_LOG_DEBUG("RedirectVnode is Vbridge");
        break;
      }
    }
  }
  UPLL_LOG_TRACE("%s GetRenamedUncKey vbrifff_entry end",
                  ikey->ToStrAll().c_str());
  DELETE_IF_NOT_NULL(unc_key);
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtermIfFlowFilterEntryMoMgr::GetRenamedControllerKey(
  ConfigKeyVal *ikey, upll_keytype_datatype_t dt_type, DalDmlIntf *dmi,
  controller_domain *ctrlr_dom) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *okey = NULL;

  /* Get the controller's redirect node(vbridge/vrt) name -start*/
  val_flowfilter_entry_t *val_ff_entry =
    reinterpret_cast<val_flowfilter_entry_t *> (GetVal(ikey));
  if (NULL == ctrlr_dom) {
    UPLL_LOG_ERROR("ctrlr null");
    return UPLL_RC_ERR_GENERIC;
  }

  if (val_ff_entry) {
    if (((UNC_VF_VALID ==
          val_ff_entry->valid[UPLL_IDX_REDIRECT_NODE_FFE]) ||
          (UNC_VF_VALUE_NOT_MODIFIED ==
          val_ff_entry->valid[UPLL_IDX_REDIRECT_NODE_FFE]))
        &&
         ((UNC_VF_VALID ==
          val_ff_entry->valid[UPLL_IDX_REDIRECT_PORT_FFE]) ||
          (UNC_VF_VALUE_NOT_MODIFIED ==
          val_ff_entry->valid[UPLL_IDX_REDIRECT_PORT_FFE]))
        ) {
      unc_key_type_t child_key[]= { UNC_KT_VBRIDGE, UNC_KT_VROUTER,
                                    UNC_KT_VTERMINAL};
      bool isRedirectVnode = false;
      for (unsigned int i = 0;
          i < sizeof(child_key)/sizeof(child_key[0]); i++) {
        const unc_key_type_t ktype = child_key[i];
        MoMgrImpl *mgrvbr = reinterpret_cast<MoMgrImpl *>(
            const_cast<MoManager *>(GetMoManager(ktype)));
        if (!mgrvbr) {
          UPLL_LOG_ERROR("mgrvbr failed");
          return UPLL_RC_ERR_GENERIC;
        }

        result_code = mgrvbr->GetChildConfigKey(okey, NULL);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_ERROR("GetChildConfigKey fail");
          return result_code;
        }
        SET_USER_DATA_CTRLR_DOMAIN(okey, *ctrlr_dom);
        UPLL_LOG_DEBUG("ctrlr : %s; domain : %s", ctrlr_dom->ctrlr,
            ctrlr_dom->domain);
        if (okey->get_key_type() == UNC_KT_VBRIDGE) {
          uuu::upll_strncpy(reinterpret_cast<key_vbr_t *>
              (okey->get_key())->vbridge_name,
              reinterpret_cast<val_flowfilter_entry_t *>(ikey->get_cfg_val()->
                get_val())->redirect_node, (kMaxLenVnodeName + 1));

          UPLL_LOG_DEBUG("redirect node vbr name (%s) (%s)",
              reinterpret_cast<key_vbr_t *>(okey->get_key())->vbridge_name,
              reinterpret_cast<val_flowfilter_entry_t *>(ikey->get_cfg_val()->
                get_val())->redirect_node);
        } else if (okey->get_key_type() == UNC_KT_VROUTER) {
          uuu::upll_strncpy(reinterpret_cast<key_vrt_t *>
              (okey->get_key())->vrouter_name,
              reinterpret_cast<val_flowfilter_entry_t *>(ikey->get_cfg_val()->
                get_val())->redirect_node, (kMaxLenVnodeName + 1));

          UPLL_LOG_DEBUG("redirect node vrt name (%s) (%s)",
              reinterpret_cast<key_vrt_t *>(okey->get_key())->vrouter_name,
              reinterpret_cast<val_flowfilter_entry_t *>(ikey->get_cfg_val()->
                get_val())->redirect_node);
        } else if (okey->get_key_type() == UNC_KT_VTERMINAL) {
          uuu::upll_strncpy(reinterpret_cast<key_vterm_t *>
              (okey->get_key())->vterminal_name,
              reinterpret_cast<val_flowfilter_entry_t *>(ikey->get_cfg_val()->
                get_val())->redirect_node, (kMaxLenVnodeName + 1));

          UPLL_LOG_DEBUG("redirect node vterm name (%s) (%s)",
              reinterpret_cast<key_vterm_t *>(okey->get_key())->vterminal_name,
              reinterpret_cast<val_flowfilter_entry_t *>(ikey->get_cfg_val()->
                get_val())->redirect_node);
        }

        DbSubOp dbop = { kOpReadSingle, kOpMatchCtrlr | kOpMatchDomain,
          kOpInOutFlag };
        result_code = mgrvbr->ReadConfigDB(okey, dt_type, UNC_OP_READ,
            dbop, dmi, RENAMETBL);
        if (result_code != UPLL_RC_SUCCESS) {
          if (UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
            UPLL_LOG_ERROR("ReadConfigDB fail");
            DELETE_IF_NOT_NULL(okey);
            return result_code;
          }
        }

        if (result_code == UPLL_RC_SUCCESS) {
          val_rename_vnode *rename_val = NULL;
          isRedirectVnode = true;
          rename_val = reinterpret_cast<val_rename_vnode *> (GetVal(okey));
          if (!rename_val) {
            UPLL_LOG_ERROR("rename_val NULL.");
            DELETE_IF_NOT_NULL(okey);
            return UPLL_RC_ERR_GENERIC;
          }

          uuu::upll_strncpy(reinterpret_cast<val_flowfilter_entry_t*>
              (ikey->get_cfg_val()->get_val())->redirect_node,
              rename_val->ctrlr_vnode_name, (kMaxLenVnodeName + 1));
        }
        DELETE_IF_NOT_NULL(okey);
        if (isRedirectVnode)
          break;
      }
    }
  }
  /* -end*/
  UPLL_LOG_TRACE("Start... Input ConfigKeyVal %s", ikey->ToStrAll().c_str());

  MoMgrImpl *VtermMoMgr = static_cast<MoMgrImpl*>
    ((const_cast<MoManager*>(GetMoManager(UNC_KT_VTERMINAL))));
  if (NULL == VtermMoMgr) {
    UPLL_LOG_ERROR("obj null");
    return UPLL_RC_ERR_GENERIC;
  }

  result_code = VtermMoMgr->GetChildConfigKey(okey, NULL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_ERROR("GetChildConfigKey fail");
    return result_code;
  }

  SET_USER_DATA_CTRLR_DOMAIN(okey, *ctrlr_dom);

  UPLL_LOG_DEBUG("ctrlr : %s; domain : %s", ctrlr_dom->ctrlr,
      ctrlr_dom->domain);
  uuu::upll_strncpy(
      reinterpret_cast<key_vterm *>(okey->get_key())->vtn_key.vtn_name,
      reinterpret_cast <key_vterm_if_flowfilter_entry_t*>
      (ikey->get_key())->flowfilter_key.if_key.vterm_key.vtn_key.vtn_name,
      (kMaxLenVtnName + 1));

  UPLL_LOG_DEBUG("vtn name (%s) (%s)",
      reinterpret_cast<key_vterm *>(okey->get_key())->vtn_key.vtn_name,
      reinterpret_cast <key_vterm_if_flowfilter_entry_t*>
      (ikey->get_key())->flowfilter_key.if_key.vterm_key.vtn_key.vtn_name);

  uuu::upll_strncpy(
      reinterpret_cast<key_vterm *>(okey->get_key())->vterminal_name,
      reinterpret_cast <key_vterm_if_flowfilter_entry_t*>
      (ikey->get_key())->flowfilter_key.if_key.vterm_key.vterminal_name,
      (kMaxLenVnodeName + 1));

  UPLL_LOG_DEBUG("vterm name (%s) (%s)",
      reinterpret_cast<key_vterm *>(okey->get_key())->vterminal_name,
      reinterpret_cast <key_vterm_if_flowfilter_entry_t*>
      (ikey->get_key())->flowfilter_key.if_key.vterm_key.vterminal_name);

  DbSubOp dbop = { kOpReadSingle, kOpMatchCtrlr | kOpMatchDomain,
    kOpInOutFlag };
  result_code=  VtermMoMgr->ReadConfigDB(okey, dt_type, UNC_OP_READ,
      dbop, dmi, RENAMETBL);
  if (( result_code != UPLL_RC_SUCCESS ) &&
      (UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code)) {
    UPLL_LOG_ERROR("ReadConfigDB fail");
    DELETE_IF_NOT_NULL(okey);
    return UPLL_RC_ERR_GENERIC;
  }
  if (UPLL_RC_SUCCESS == result_code) {
    val_rename_vnode *rename_val =reinterpret_cast <val_rename_vnode *>
      (GetVal(okey));

    if (!rename_val) {
      UPLL_LOG_ERROR("Vterm Name is not Valid");
      DELETE_IF_NOT_NULL(okey);
      return UPLL_RC_ERR_GENERIC;
    }

    uuu::upll_strncpy(
        reinterpret_cast<key_vterm_if_flowfilter_entry_t *>
        (ikey->get_key())->flowfilter_key.if_key.vterm_key.vtn_key.vtn_name,
        rename_val->ctrlr_vtn_name,
        kMaxLenVtnName + 1);

    UPLL_LOG_DEBUG("vtn name (%s) (%s)",
        reinterpret_cast<key_vterm_if_flowfilter_entry_t *>
        (ikey->get_key())->flowfilter_key.if_key.vterm_key.vtn_key.vtn_name,
        rename_val->ctrlr_vtn_name);

    uuu::upll_strncpy(
        reinterpret_cast<key_vterm_if_flowfilter_entry_t*>
        (ikey->get_key())->flowfilter_key.if_key.vterm_key.vterminal_name,
        rename_val->ctrlr_vnode_name,
        kMaxLenVnodeName + 1);

    UPLL_LOG_DEBUG("vterm name (%s) (%s)",
        reinterpret_cast<key_vterm_if_flowfilter_entry_t*>
        (ikey->get_key())->flowfilter_key.if_key.vterm_key.vterminal_name,
        rename_val->ctrlr_vnode_name);
  }
  DELETE_IF_NOT_NULL(okey);

  //  flowlist_name
  val_flowfilter_entry_t *val_ffe = reinterpret_cast
      <val_flowfilter_entry_t *>(GetVal(ikey));
  if (NULL == val_ffe) {
    UPLL_LOG_DEBUG("value structure is null");
    return UPLL_RC_SUCCESS;
  }
  if (strlen(reinterpret_cast<char *>
        (val_ffe->flowlist_name)) == 0) {
    return UPLL_RC_SUCCESS;
  }
  MoMgrImpl *mgrflist = static_cast<MoMgrImpl*>
    ((const_cast<MoManager*> (GetMoManager(UNC_KT_FLOWLIST))));
  if (mgrflist == NULL) {
    UPLL_LOG_ERROR("obj null");
    return UPLL_RC_ERR_GENERIC;
  }

  result_code = mgrflist->GetChildConfigKey(okey, NULL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_ERROR("GetChildConfigKey fail");
    return result_code;
  }
  SET_USER_DATA_CTRLR(okey, ctrlr_dom->ctrlr);
  UPLL_LOG_DEBUG("ctrlr : %s; domain : %s", ctrlr_dom->ctrlr,
      ctrlr_dom->domain);

  uuu::upll_strncpy(
      reinterpret_cast<key_flowlist_t *>(okey->get_key())->flowlist_name,
      reinterpret_cast<val_flowfilter_entry_t*>
      (ikey->get_cfg_val()->get_val())->flowlist_name,
      kMaxLenFlowListName + 1);
  UPLL_LOG_DEBUG("flowlist name (%s) (%s)",
      reinterpret_cast<key_flowlist_t *>(okey->get_key())->flowlist_name,
      reinterpret_cast<val_flowfilter_entry_t*>
      (ikey->get_cfg_val()->get_val())->flowlist_name);

  DbSubOp dbop1 = { kOpReadSingle, kOpMatchCtrlr, kOpInOutFlag };
  /* ctrlr_name */
  result_code =  mgrflist->ReadConfigDB(okey, dt_type, UNC_OP_READ,
      dbop1, dmi, RENAMETBL);
  if (( result_code != UPLL_RC_SUCCESS ) &&
      (UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code)) {
    UPLL_LOG_ERROR("ReadConfigDB fail");
    DELETE_IF_NOT_NULL(okey);
    return UPLL_RC_ERR_GENERIC;
  }
  if (UPLL_RC_SUCCESS == result_code) {
    val_rename_flowlist_t *rename_val =reinterpret_cast <val_rename_flowlist_t*>
      (GetVal(okey));
    if (!rename_val) {
      UPLL_LOG_ERROR("flowlist is not valid");
      DELETE_IF_NOT_NULL(okey);
      return UPLL_RC_ERR_GENERIC;
    }
    uuu::upll_strncpy(
        reinterpret_cast<val_flowfilter_entry_t*>
        (ikey->get_cfg_val()->get_val())->flowlist_name,
        rename_val->flowlist_newname,
        (kMaxLenFlowListName + 1));
  }
  DELETE_IF_NOT_NULL(okey);
  UPLL_LOG_TRACE("End... Input ConfigKeyVal %s", ikey->ToStrAll().c_str());
  UPLL_LOG_DEBUG("Renamed Controller key is sucessfull.");
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtermIfFlowFilterEntryMoMgr::RenameMo(IpcReqRespHeader *req,
                                         ConfigKeyVal *ikey, DalDmlIntf *dmi,
                                         const char *ctrlr_id) {
  UPLL_FUNC_TRACE;
  UPLL_LOG_ERROR("Implementation Not supported for this KT.");
  return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_KT;
}

upll_rc_t VtermIfFlowFilterEntryMoMgr::MergeValidate(unc_key_type_t keytype,
    const char *ctrlr_id,
    ConfigKeyVal *ikey,
    DalDmlIntf *dmi,
    upll_import_type import_type) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  ConfigKeyVal *ckval = NULL;
  if (NULL == ctrlr_id) {
    UPLL_LOG_DEBUG("ctrlr_id NULL");
    return result_code;
  }
  result_code = GetChildConfigKey(ckval, NULL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("ckval fail");
    return result_code;
  }
  DbSubOp dbop = { kOpReadMultiple, kOpMatchNone, kOpInOutNone};
  result_code = ReadConfigDB(ckval, UPLL_DT_IMPORT,
              UNC_OP_READ, dbop, dmi, MAINTBL);
  if (UPLL_RC_SUCCESS != result_code) {
    DELETE_IF_NOT_NULL(ckval);
    if (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      UPLL_LOG_DEBUG("ReadConfigDB fail");
      return result_code;
    }
    return UPLL_RC_SUCCESS;
  }
  ConfigKeyVal *first_ckv = ckval;
  while (NULL != ckval) {
    val_flowfilter_entry_t* val = reinterpret_cast<val_flowfilter_entry_t *>
    (GetVal(ckval));
    if ((val->valid[UPLL_IDX_REDIRECT_NODE_FFE] ==
     UNC_VF_VALID) &&
     (val->valid[UPLL_IDX_REDIRECT_PORT_FFE] ==
     UNC_VF_VALID) && (val->valid[UPLL_IDX_REDIRECT_DIRECTION_FFE] ==
             UNC_VF_VALID)) {
       result_code = VerifyRedirectDestination(ckval, dmi, UPLL_DT_IMPORT);
       if (result_code != UPLL_RC_SUCCESS) {
          DELETE_IF_NOT_NULL(first_ckv);
          UPLL_LOG_DEBUG("redirect-destination node/interface doesn't exists");
          return UPLL_RC_ERR_MERGE_CONFLICT;
       }
    }
    ckval = ckval->get_next_cfg_key_val();
  }
  DELETE_IF_NOT_NULL(first_ckv);
  UPLL_LOG_DEBUG("MergeValidate result code (%d)", result_code);
  return result_code;
}

upll_rc_t VtermIfFlowFilterEntryMoMgr::UpdateAuditConfigStatus(
    unc_keytype_configstatus_t cs_status,
    uuc::UpdateCtrlrPhase phase,
    ConfigKeyVal *&ckv_running,
    DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  val_flowfilter_entry_t *val;
  val = (ckv_running != NULL)?reinterpret_cast<val_flowfilter_entry_t *>
                              (GetVal(ckv_running)):NULL;
  if (NULL == val) {
    UPLL_LOG_ERROR("val struct is NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  if (uuc::kUpllUcpCreate == phase )
    val->cs_row_status = cs_status;
  if ((uuc::kUpllUcpUpdate == phase) &&
           (val->cs_row_status == UNC_CS_INVALID ||
            val->cs_row_status == UNC_CS_NOT_APPLIED))
    val->cs_row_status = cs_status;
  for ( unsigned int loop = 0;
      loop < sizeof(val->valid)/sizeof(uint8_t); ++loop ) {
    if ((cs_status == UNC_CS_INVALID && UNC_VF_VALID == val->valid[loop]) ||
         cs_status == UNC_CS_APPLIED)
       val->cs_attr[loop] = cs_status;
  }
  UPLL_LOG_DEBUG("UpdateAuditConfigStatus::Success");
  return result_code;
}

bool VtermIfFlowFilterEntryMoMgr::IsValidKey(void *key, uint64_t index,
                                             MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  upll_rc_t ret_val = UPLL_RC_SUCCESS;
  key_vterm_if_flowfilter_entry_t  *ff_key =
      reinterpret_cast<key_vterm_if_flowfilter_entry_t *>(key);
  if (ff_key == NULL) {
    UPLL_LOG_ERROR("key is NULL");
    return false;
  }

  switch (index) {
    case uudst::vterm_if_flowfilter_entry::kDbiVtnName:
      ret_val = ValidateKey(
          reinterpret_cast<char *>
          (ff_key->flowfilter_key.if_key.vterm_key.vtn_key.vtn_name),
          kMinLenVtnName, kMaxLenVtnName);
      if (ret_val != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("VTN Name is not valid(%d)", ret_val);
        return false;
      }
      break;
    case uudst::vterm_if_flowfilter_entry::kDbiVtermName:
      ret_val = ValidateKey(reinterpret_cast<char *>
                          (ff_key->flowfilter_key.if_key.
                           vterm_key.vterminal_name),
                          kMinLenVnodeName, kMaxLenVnodeName);
      if (ret_val != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("VTERM Name is not valid(%d)", ret_val);
        return false;
      }
      break;
    case uudst::vterm_if_flowfilter_entry::kDbiVtermIfName:
      ret_val = ValidateKey(reinterpret_cast<char *>
                            (ff_key->flowfilter_key.if_key.if_name),
                            kMinLenInterfaceName, kMaxLenInterfaceName);
      if (ret_val != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("VTERMIF  Name is not valid(%d)", ret_val);
        return false;
      }
      break;
    case uudst::vterm_if_flowfilter_entry::kDbiInputDirection:

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
    case uudst::vterm_if_flowfilter_entry::kDbiSequenceNum:
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
  }
  return true;
}

upll_rc_t VtermIfFlowFilterEntryMoMgr::UpdateMo(IpcReqRespHeader *req,
                                              ConfigKeyVal *ikey,
                                              DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  ConfigKeyVal *okey = NULL;
  uint8_t *ctrlr_id = NULL;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  if (NULL == ikey || NULL == req || NULL == dmi) return UPLL_RC_ERR_GENERIC;
  result_code = ValidateMessage(req, ikey);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_ERROR("ValidateMessage failed %d", result_code);
    return result_code;
  }

  result_code = ValidateVtermIfValStruct(req, ikey);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_ERROR("val structure validation failed-Err code(%d)",
                    result_code);
    return result_code;
  }

  val_flowfilter_entry_t *flowfilter_val =
    reinterpret_cast<val_flowfilter_entry_t *> (GetVal(ikey));

  result_code = ValidateAttribute(ikey, dmi, req);
  if (UPLL_RC_SUCCESS != result_code) {
     UPLL_LOG_ERROR("ValidateAttribute failed %d ", result_code);
     return result_code;
  }

  // Check and update the flowlist reference count if the flowlist object
  // is referred
  FlowListMoMgr *flowlist_mgr = reinterpret_cast<FlowListMoMgr *>
      (const_cast<MoManager *> (GetMoManager(UNC_KT_FLOWLIST)));
  // Get controller id to do capa check
  result_code = GetChildConfigKey(okey, ikey);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_ERROR("GetChildConfigKey failed");
    return result_code;
  }
  DbSubOp dbop = { kOpReadSingle, kOpMatchNone,
                   kOpInOutCtrlr|kOpInOutDomain|kOpInOutFlag };
  result_code = ReadConfigDB(okey, UPLL_DT_CANDIDATE, UNC_OP_READ, dbop, dmi,
        MAINTBL);

  if (UPLL_RC_SUCCESS != result_code) {
    delete okey;
    UPLL_LOG_ERROR("ReadConfigDB failed %d", result_code);
    return result_code;
  }
  /* Check whether all attributes are invalid in value structure.*/
  if (VbrFlowFilterEntryMoMgr::IsAllAttrInvalid(flowfilter_val)) {
    UPLL_LOG_INFO("No attributes to be updated");
    DELETE_IF_NOT_NULL(okey);
    return UPLL_RC_SUCCESS;
  }
  GET_USER_DATA_CTRLR(okey, ctrlr_id);
  result_code= ValidateCapability(req, ikey,
                   reinterpret_cast<const char*>(ctrlr_id));
  if (UPLL_RC_SUCCESS != result_code) {
     delete okey;
     UPLL_LOG_ERROR("ValidateCapability failed %d", result_code);
     return result_code;
  }
  TcConfigMode config_mode = TC_CONFIG_INVALID;
  std::string vtn_name = "";
  result_code = GetConfigModeInfo(req, config_mode, vtn_name);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetConfigMode failed");
    return result_code;
  }

  uint8_t dbflag = 0;
  GET_USER_DATA_FLAGS(okey, dbflag);
  if (UNC_VF_VALID == flowfilter_val->valid[UPLL_IDX_FLOWLIST_NAME_FFE] ||
      UNC_VF_VALID_NO_VALUE == flowfilter_val->
      valid[UPLL_IDX_FLOWLIST_NAME_FFE]) {
      if (dbflag & SET_FLAG_PORTMAP) {
        val_flowfilter_entry_t *temp_ffe_val = reinterpret_cast
        <val_flowfilter_entry_t *>(GetVal(okey));
        UPLL_LOG_DEBUG("flowlist name %s", flowfilter_val->flowlist_name);
        std::string temp_vtn_name;
        if (TC_CONFIG_VTN == config_mode) {
          temp_vtn_name = vtn_name;
        } else {
          key_vterm_if_flowfilter_entry_t *temp_key = reinterpret_cast<
              key_vterm_if_flowfilter_entry_t *>(ikey->get_key());
          temp_vtn_name = reinterpret_cast<const char*>(
              temp_key->flowfilter_key.if_key.vterm_key.vtn_key.vtn_name);
        }
        if ((UNC_VF_VALID == flowfilter_val->valid[UPLL_IDX_FLOWLIST_NAME_VFFE])
            && (UNC_VF_VALID  == temp_ffe_val->
          valid[UPLL_IDX_FLOWLIST_NAME_VFFE])) {
          UPLL_LOG_DEBUG("Update option 1");

         // Skip the record if flowlist is already configured in DB
         if (strncmp(reinterpret_cast<char *>(flowfilter_val->flowlist_name),
              reinterpret_cast<char *>(temp_ffe_val->flowlist_name),
              (kMaxLenFlowListName + 1))) {
            result_code = flowlist_mgr->AddFlowListToController(
            reinterpret_cast<char *>(temp_ffe_val->flowlist_name), dmi,
            reinterpret_cast<char *>(ctrlr_id), req->datatype, UNC_OP_DELETE,
            config_mode, temp_vtn_name, false);
            if (result_code != UPLL_RC_SUCCESS) {
              UPLL_LOG_ERROR("AddFlowListToController failed %d", result_code);
              delete okey;
              return result_code;
            }
            result_code = flowlist_mgr->AddFlowListToController(
              reinterpret_cast<char *>(flowfilter_val->flowlist_name), dmi,
              reinterpret_cast<char *> (ctrlr_id), req->datatype, UNC_OP_CREATE,
              config_mode, temp_vtn_name, false);
            if (result_code != UPLL_RC_SUCCESS) {
              UPLL_LOG_ERROR("AddFlowListToController failed %d", result_code);
              delete okey;
              return result_code;
            }
         }
        } else if (UNC_VF_VALID == flowfilter_val->
          valid[UPLL_IDX_FLOWLIST_NAME_VFFE] &&
          (UNC_VF_INVALID == temp_ffe_val->
           valid[UPLL_IDX_FLOWLIST_NAME_VFFE] || UNC_VF_VALID_NO_VALUE ==
           temp_ffe_val->valid[UPLL_IDX_FLOWLIST_NAME_VFFE])) {
            UPLL_LOG_DEBUG("Update option 2");
            result_code = flowlist_mgr->AddFlowListToController(
              reinterpret_cast<char *>(flowfilter_val->flowlist_name), dmi,
              reinterpret_cast<char *>(ctrlr_id), req->datatype, UNC_OP_CREATE,
              config_mode, temp_vtn_name, false);
            if (result_code != UPLL_RC_SUCCESS) {
              UPLL_LOG_ERROR("AddFlowListToController failed %d", result_code);
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
              reinterpret_cast<char *>(ctrlr_id), req->datatype, UNC_OP_DELETE,
              config_mode, temp_vtn_name, false);
              if (result_code != UPLL_RC_SUCCESS) {
                UPLL_LOG_ERROR("AddFlowListToController failed %d",
                                result_code);
                delete okey;
                return result_code;
              }
          }
        }
  }

  DbSubOp dbop1 = {kOpNotRead, kOpMatchNone, kOpInOutNone};
  result_code = UpdateConfigDB(ikey, req->datatype, UNC_OP_UPDATE, dmi,
                               &dbop1, config_mode, vtn_name, MAINTBL);

  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_ERROR("DB Error while updating the candidatedb");
    DELETE_IF_NOT_NULL(okey);
    return result_code;
  }
  DELETE_IF_NOT_NULL(okey);
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtermIfFlowFilterEntryMoMgr::AllocVal(ConfigVal *&ck_val,
                                              upll_keytype_datatype_t dt_type,
                                              MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  void *val;  //  *ck_nxtval;
  if (ck_val != NULL) {
    UPLL_LOG_ERROR("ck_val Consist the Value");
    return UPLL_RC_ERR_GENERIC;
  }
  switch (tbl) {
  case MAINTBL:
    val = reinterpret_cast <void *>
        (ConfigKeyVal::Malloc(sizeof(val_flowfilter_entry_t)));
    ck_val = new ConfigVal(IpctSt::kIpcStValFlowfilterEntry, val);
    break;
  default:
    val = NULL;
    return UPLL_RC_ERR_GENERIC;
  }
  UPLL_LOG_DEBUG("AllocVal Success");
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtermIfFlowFilterEntryMoMgr::VerifyRedirectDestination(
    ConfigKeyVal *ikey,
    DalDmlIntf *dmi,
    upll_keytype_datatype_t dt_type) {
  UPLL_FUNC_TRACE;
  MoMgrImpl *mgr = NULL;
  DbSubOp dbop = {kOpReadSingle, kOpMatchCtrlr|kOpMatchDomain, kOpInOutFlag};
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *okey = NULL;
  if (!ikey || !ikey->get_key()) {
    UPLL_LOG_ERROR("input key is null");
    return UPLL_RC_ERR_GENERIC;
  }
  controller_domain ctrlr_dom;
  memset(&ctrlr_dom, 0, sizeof(controller_domain));
  key_vterm_if_flowfilter_entry_t *key_vtrm =
    reinterpret_cast<key_vterm_if_flowfilter_entry_t *>(ikey->get_key());

  /* read val_vtn_flowfilter_entry from ikey*/
  val_flowfilter_entry_t *val_flowfilter_entry =
    static_cast<val_flowfilter_entry_t *>(
        ikey->get_cfg_val()->get_val());
  if (val_flowfilter_entry == NULL) {
    UPLL_LOG_DEBUG(" val_flowfilter_entry is null");
    return UPLL_RC_ERR_GENERIC;
  }

  // Symentic Validation for redirect destination
  // During commit (only), the valid flags will be set to VALUE _NOT_MODIFIED.
  // Skip the validation if redirect node and redirect port and redirect
  // direction are already available in the running configuration
  if ((val_flowfilter_entry->valid[UPLL_IDX_REDIRECT_NODE_FFE]
       == UNC_VF_VALUE_NOT_MODIFIED)
      &&
      (val_flowfilter_entry->valid[UPLL_IDX_REDIRECT_PORT_FFE]
       == UNC_VF_VALUE_NOT_MODIFIED) &&
     (val_flowfilter_entry->valid[UPLL_IDX_REDIRECT_DIRECTION_FFE] ==
      UNC_VF_INVALID)) {
    return UPLL_RC_SUCCESS;
  } else if (((val_flowfilter_entry->valid[UPLL_IDX_REDIRECT_NODE_FFE] ==
               UNC_VF_VALID) ||
              (val_flowfilter_entry->valid[UPLL_IDX_REDIRECT_NODE_FFE] ==
               UNC_VF_VALUE_NOT_MODIFIED)) &&
             ((val_flowfilter_entry->valid[UPLL_IDX_REDIRECT_PORT_FFE] ==
               UNC_VF_VALID) ||
              (val_flowfilter_entry->valid[UPLL_IDX_REDIRECT_PORT_FFE] ==
               UNC_VF_VALUE_NOT_MODIFIED))) {
    DbSubOp dbop_up = { kOpReadExist, kOpMatchCtrlr|kOpMatchDomain,
      kOpInOutNone };
    result_code = GetControllerDomainID(ikey, dt_type, dmi);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Failed to Get the Controller Domain details, err:%d",
          result_code);
      return result_code;
    }
    ctrlr_dom.ctrlr = NULL;
    ctrlr_dom.domain = NULL;
    GET_USER_DATA_CTRLR_DOMAIN(ikey, ctrlr_dom);

    UPLL_LOG_DEBUG("ctrlrid %s, domainid %s",
        ctrlr_dom.ctrlr, ctrlr_dom.domain);

    unc_keytype_ctrtype_t ctrlrtype = UNC_CT_UNKNOWN;
    uuc::CtrlrMgr *ctrlr_mgr = uuc::CtrlrMgr::GetInstance();
    if (!ctrlr_mgr->GetCtrlrType(
            reinterpret_cast<const char*>(ctrlr_dom.ctrlr),
            dt_type, &ctrlrtype)) {
      UPLL_LOG_ERROR("Controller name %s not found in datatype %d",
                    reinterpret_cast<const char*>(ctrlr_dom.ctrlr), dt_type);
      return UPLL_RC_ERR_GENERIC;
    }
    // Verify whether the vtnnode and interface are exists in DB
    // 1. Check for the vbridge Node
    if (val_flowfilter_entry->redirect_direction
             == UPLL_FLOWFILTER_DIR_OUT) {
       UPLL_LOG_DEBUG(" Redirect-direction is OUT");
       mgr = reinterpret_cast<MoMgrImpl *>
         (const_cast<MoManager *>(GetMoManager(UNC_KT_VBR_IF)));
        if (NULL == mgr) {
          UPLL_LOG_ERROR("Unable to get VBRIDGE Interface object");
          return UPLL_RC_ERR_GENERIC;
        }
        result_code = mgr->GetChildConfigKey(okey, NULL);
        if (result_code != UPLL_RC_SUCCESS) {
           UPLL_LOG_ERROR(
               "Memory allocation failed for VBRIDGE key struct - %d",
               result_code);
           return result_code;
        }
        // flowfilter_key.if_key.vterm_key.vtn_key.vtn_name),
        key_vbr_if_t *vbrif_key = static_cast<key_vbr_if_t*>(
          okey->get_key());
        uuu::upll_strncpy(vbrif_key->vbr_key.vtn_key.vtn_name,
          key_vtrm->flowfilter_key.if_key.vterm_key.vtn_key.vtn_name,
          kMaxLenVtnName + 1);

        uuu::upll_strncpy(vbrif_key->vbr_key.vbridge_name,
           reinterpret_cast<char *>
            (val_flowfilter_entry->redirect_node),
            (kMaxLenVnodeName + 1));
        uuu::upll_strncpy(vbrif_key->if_name,
           reinterpret_cast<char *>
            (val_flowfilter_entry->redirect_port),
            kMaxLenInterfaceName + 1);
        /* Check vtnnode and interface exists in table*/

        SET_USER_DATA_CTRLR_DOMAIN(okey, ctrlr_dom);

        result_code = mgr->ReadConfigDB(okey, dt_type,
                        UNC_OP_READ , dbop , dmi, MAINTBL);

        if (UPLL_RC_SUCCESS == result_code) {
        // If Destination  Interface in UNC is vbridge interface  and If
        // Usage in UNC is PortMapped , configured  And  redirect-direction
        // is OUT.then the validation will get Success And will send the
        // vexternal/vexternal if to Driver.
        if (ctrlrtype == UNC_CT_ODC) {
          UPLL_LOG_DEBUG("Cannnot set the vbr if with redirect-direction"
                         "as OUT for ctrlr %d", ctrlrtype);
          DELETE_IF_NOT_NULL(okey);
          return UPLL_RC_ERR_CFG_SEMANTIC;
        }
        val_drv_vbr_if *val_drv_vbr =
           reinterpret_cast<val_drv_vbr_if *>(GetVal(okey));
        bool port_map_status =
            (val_drv_vbr->vbr_if_val.valid[UPLL_IDX_PM_VBRI] == UNC_VF_VALID)?
                                    true:false;
        if (dt_type == UPLL_DT_IMPORT) {
          DELETE_IF_NOT_NULL(okey);
          UPLL_LOG_DEBUG(" Datatype is Import : MergeValidate");
          if (port_map_status)  {
            UPLL_LOG_DEBUG(" Datatype is Import :Port map configured");
            return UPLL_RC_SUCCESS;
          } else  {
            UPLL_LOG_INFO("Datatype is Import : MergeValidate Conflict");
            return UPLL_RC_ERR_CFG_SEMANTIC;
          }
        }
        if (port_map_status) {
          UPLL_LOG_DEBUG("redirect node/port As VBR/VBR_IF exists"
                         "in DB And Redirect-direction is OUT");
          if (!strlen(reinterpret_cast<const char *>(val_drv_vbr->vex_name))) {
            DELETE_IF_NOT_NULL(okey);
            return UPLL_RC_ERR_GENERIC;
          }
          if (!strlen(reinterpret_cast<const char *>
                              (val_drv_vbr->vex_if_name))) {
            DELETE_IF_NOT_NULL(okey);
            return UPLL_RC_ERR_GENERIC;
          }
          uuu::upll_strncpy(val_flowfilter_entry->redirect_node,
                                    val_drv_vbr->vex_name,
                        (kMaxLenInterfaceName + 1));
          uuu::upll_strncpy(val_flowfilter_entry->redirect_port,
                                val_drv_vbr->vex_if_name,
                        (kMaxLenInterfaceName + 1));
          DELETE_IF_NOT_NULL(okey);
          return UPLL_RC_SUCCESS;
       }
       UPLL_LOG_ERROR("Semantic error");
       DELETE_IF_NOT_NULL(okey);
       return UPLL_RC_ERR_CFG_SEMANTIC;
     } else if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
       // If  Destination Interface in UNC is vterminal interface
       // redirect-direction is OUT .then the validation will
       // get UPLL_RC_ERR_CFG_SEMANTIC
       DELETE_IF_NOT_NULL(okey);
      mgr = reinterpret_cast<MoMgrImpl *>
        (const_cast<MoManager *>(GetMoManager(UNC_KT_VTERM_IF)));
      if (NULL == mgr) {
        UPLL_LOG_ERROR("Unable to get VTERM Interface object");
        return UPLL_RC_ERR_GENERIC;
      }
      result_code = mgr->GetChildConfigKey(okey, NULL);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_ERROR("Memory allocation failed for VTERM key struct - %d",
            result_code);
        return result_code;
      }
      key_vterm_if_t *vrtif_key = static_cast<key_vterm_if_t*>(
          okey->get_key());
      uuu::upll_strncpy(vrtif_key->vterm_key.vtn_key.vtn_name,
          key_vtrm->flowfilter_key.if_key.vterm_key.vtn_key.vtn_name,
          kMaxLenVtnName + 1);

      uuu::upll_strncpy(vrtif_key->vterm_key.vterminal_name,
          reinterpret_cast<char *>
          (val_flowfilter_entry->redirect_node),
          (kMaxLenVnodeName + 1));
      uuu::upll_strncpy(vrtif_key->if_name,
          reinterpret_cast<char *>
          (val_flowfilter_entry->redirect_port),
          kMaxLenInterfaceName + 1);

      UPLL_LOG_DEBUG("ctrlrid %s, domainid %s",
          ctrlr_dom.ctrlr, ctrlr_dom.domain);
      SET_USER_DATA_CTRLR_DOMAIN(okey, ctrlr_dom);
      /* Check vtnnode and interface exists in table*/
      result_code = mgr->UpdateConfigDB(okey, dt_type,
          UNC_OP_READ, dmi, &dbop_up, MAINTBL);
      if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
        UPLL_LOG_ERROR("vtn node/interface VTERMINAL/VTERMINAL_IF"
                        "in val struct does not exist in DB");
        DELETE_IF_NOT_NULL(okey);
        return UPLL_RC_ERR_CFG_SEMANTIC;
      } else if (UPLL_RC_ERR_INSTANCE_EXISTS == result_code) {
        // If Destination Interface in UNC is vrouter interface
        // redirect-direction is OUT.then  the validation will get
        // Success.
         UPLL_LOG_DEBUG("vtn node/interface in VTERM_IF exists"
           "in DB");
         DELETE_IF_NOT_NULL(okey);
         return UPLL_RC_SUCCESS;
       } else {
         UPLL_LOG_DEBUG("DB Related Error");
         DELETE_IF_NOT_NULL(okey);
         return result_code;
       }
      }
      DELETE_IF_NOT_NULL(okey);
    } else if (val_flowfilter_entry->redirect_direction
                      == UPLL_FLOWFILTER_DIR_IN ) {
      mgr = reinterpret_cast<MoMgrImpl *>
        (const_cast<MoManager *>(GetMoManager(UNC_KT_VBR_IF)));
      if (NULL == mgr) {
        UPLL_LOG_ERROR("Unable to get VBRIDGE Interface object");
        return UPLL_RC_ERR_GENERIC;
      }
      result_code = mgr->GetChildConfigKey(okey, NULL);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_ERROR("Memory allocation failed for VBRIDGE key struct - %d",
          result_code);
        return result_code;
      }
      key_vbr_if_t *vbrif_key = static_cast<key_vbr_if_t*>(
        okey->get_key());
      uuu::upll_strncpy(vbrif_key->vbr_key.vtn_key.vtn_name,
        key_vtrm->flowfilter_key.if_key.vterm_key.vtn_key.vtn_name,
        kMaxLenVtnName + 1);

      uuu::upll_strncpy(vbrif_key->vbr_key.vbridge_name,
        reinterpret_cast<char *>
        (val_flowfilter_entry->redirect_node),
        (kMaxLenVnodeName + 1));
      uuu::upll_strncpy(vbrif_key->if_name,
        reinterpret_cast<char *>
        (val_flowfilter_entry->redirect_port),
        kMaxLenInterfaceName + 1);

      /* Check vtnnode and interface exists in table*/

      SET_USER_DATA_CTRLR_DOMAIN(okey, ctrlr_dom);
      result_code = mgr->UpdateConfigDB(okey, dt_type,
          UNC_OP_READ, dmi, &dbop_up, MAINTBL);
      if (UPLL_RC_ERR_INSTANCE_EXISTS == result_code) {
        UPLL_LOG_DEBUG("Redirect node/port in val_flowfilter_entry As"
        "VRT_VRT_IF  exists in VRT_IF table in  DB");
        DELETE_IF_NOT_NULL(okey);
        return UPLL_RC_SUCCESS;
      } else if (UPLL_RC_ERR_INSTANCE_EXISTS == result_code) {
         UPLL_LOG_DEBUG("Redirect node/port in val_flowfilter_entry As"
        "VBR_IF  exists in VBR_IF table in  DB");
        DELETE_IF_NOT_NULL(okey);
        return UPLL_RC_SUCCESS;
      } else if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
        if (ctrlrtype == UNC_CT_ODC) {
          UPLL_LOG_DEBUG("redirect-direction can be set as IN only"
                         "for vbr if with ctrlr %d", ctrlrtype);
          DELETE_IF_NOT_NULL(okey);
          return UPLL_RC_ERR_CFG_SEMANTIC;
        }
        DELETE_IF_NOT_NULL(okey);
        mgr = reinterpret_cast<MoMgrImpl *>
         (const_cast<MoManager *>(GetMoManager(UNC_KT_VRT_IF)));
        if (NULL == mgr) {
          UPLL_LOG_ERROR("Unable to get VROUTER Interface object");
          return UPLL_RC_ERR_GENERIC;
        }
        result_code = mgr->GetChildConfigKey(okey, NULL);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_ERROR("Memory allocation failed for VROUTER key struct - %d",
            result_code);
          return result_code;
        }
        key_vrt_if_t *vrtif_key = static_cast<key_vrt_if_t*>(
          okey->get_key());
        uuu::upll_strncpy(vrtif_key->vrt_key.vtn_key.vtn_name,
          key_vtrm->flowfilter_key.if_key.vterm_key.vtn_key.vtn_name,
          kMaxLenVtnName + 1);

        uuu::upll_strncpy(vrtif_key->vrt_key.vrouter_name,
          reinterpret_cast<char *>
          (val_flowfilter_entry->redirect_node),
          (kMaxLenVnodeName + 1));
        uuu::upll_strncpy(vrtif_key->if_name,
          reinterpret_cast<char *>
          (val_flowfilter_entry->redirect_port),
          kMaxLenInterfaceName + 1);

        UPLL_LOG_DEBUG("ctrlrid %s, domainid %s",
          ctrlr_dom.ctrlr, ctrlr_dom.domain);
        SET_USER_DATA_CTRLR_DOMAIN(okey, ctrlr_dom);
      /* Check vtnnode and interface exists in table*/
        result_code = mgr->UpdateConfigDB(okey, dt_type,
          UNC_OP_READ, dmi, &dbop_up, MAINTBL);
        if (UPLL_RC_ERR_INSTANCE_EXISTS == result_code) {
          UPLL_LOG_DEBUG("Redirect node/port in val_flowfilter_entry As"
          "VRT_VRT_IF  exists in VRT_IF table in  DB");
          DELETE_IF_NOT_NULL(okey);
          return UPLL_RC_SUCCESS;
        } else if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
          DELETE_IF_NOT_NULL(okey);
          UPLL_LOG_DEBUG("Direction is IN and interface is not"
                              " VBR/VBR_IF/VRT/VRT_IF");
          return UPLL_RC_ERR_CFG_SEMANTIC;
        } else {
          DELETE_IF_NOT_NULL(okey);
          UPLL_LOG_DEBUG("Related to DB error");
          return result_code;
        }
     }
       UPLL_LOG_DEBUG("Related to DB error");
       DELETE_IF_NOT_NULL(okey);
    }
  }
  return result_code;
}


upll_rc_t VtermIfFlowFilterEntryMoMgr::ValidateAttribute(ConfigKeyVal *ikey,
                                                       DalDmlIntf *dmi,
                                                       IpcReqRespHeader *req) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *okey = NULL;
  if (!ikey || !ikey->get_key()) {
    UPLL_LOG_ERROR("input key is null");
    return UPLL_RC_ERR_GENERIC;
  }
  MoMgrImpl *mgr = NULL;

  /* read val_vtn_flowfilter_entry from ikey*/
  val_flowfilter_entry_t *val_flowfilter_entry =
    static_cast<val_flowfilter_entry_t *>(
        ikey->get_cfg_val()->get_val());
  if (val_flowfilter_entry->valid[UPLL_IDX_FLOWLIST_NAME_FFE]
        == UNC_VF_VALID) {
  /* validate flowlist_name in val_flowfilter_entry exists in FLOWLIST table*/
  mgr = reinterpret_cast<MoMgrImpl *>
      (const_cast<MoManager *>(GetMoManager(UNC_KT_FLOWLIST)));

  if (NULL == mgr) {
    UPLL_LOG_ERROR("Unable to get FLOWLIST object");
    return UPLL_RC_ERR_GENERIC;
  }

  /** allocate memory for FLOWLIST key_struct */
  result_code = mgr->GetChildConfigKey(okey, NULL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_ERROR("Memory allocation failed for FLOWLIST key struct - %d",
                    result_code);
    return result_code;
  }

  /** fill key_flowlist_t from val_flowfilter_entry*/
  key_flowlist_t *key_flowlist = static_cast<key_flowlist_t*>(
    okey->get_key());
  uuu::upll_strncpy(key_flowlist->flowlist_name,
    val_flowfilter_entry->flowlist_name,
    kMaxLenFlowListName+1);

  UPLL_LOG_TRACE("Flowlist name in val_flowfilter_entry %s data type (%d)",
                 key_flowlist->flowlist_name, req->datatype);

  /* Check flowlist_name exists in table*/
  result_code = mgr->UpdateConfigDB(okey, req->datatype,
                                     UNC_OP_READ, dmi, MAINTBL);

  if (UPLL_RC_ERR_INSTANCE_EXISTS != result_code) {
    if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE)
      result_code = UPLL_RC_ERR_CFG_SEMANTIC;
    UPLL_LOG_ERROR("Flowlist name in val_flowfilter_entry does not exists"
                   "in FLOWLIST table result code (%d)", result_code);
    delete okey;
    okey = NULL;
    return result_code;
  } else {
    result_code = UPLL_RC_SUCCESS;
  }
  // Get the mode info from tclib
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

    // mode is vtn mode, verifies flowlist existance in running db
    if (config_mode == TC_CONFIG_VTN) {
      result_code = mgr->UpdateConfigDB(okey, UPLL_DT_RUNNING,
                                       UNC_OP_READ, dmi, MAINTBL);

      if (UPLL_RC_ERR_INSTANCE_EXISTS != result_code) {
        if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE)
          result_code = UPLL_RC_ERR_CFG_SEMANTIC;
        UPLL_LOG_DEBUG("Flowlist name in val_vtn_flowfilter_entry does not"
                       "exists in running FLOWLIST table");
        delete okey;
        okey = NULL;
        return result_code;
      } else {
        result_code = UPLL_RC_SUCCESS;
      }
    }
  }
  /****************PC End******************/
  delete okey;
  okey = NULL;
  }

  if (req->datatype == UPLL_DT_IMPORT) {
    if (val_flowfilter_entry->valid[UPLL_IDX_NWM_NAME_FFE]
        == UNC_VF_VALID) {
      // validate nwm_name in KT_VBR_NWMONITOR table*/
      mgr = reinterpret_cast<MoMgrImpl *>
         (const_cast<MoManager *>(GetMoManager(UNC_KT_VBR_NWMONITOR)));

      if (NULL == mgr) {
        UPLL_LOG_ERROR("Unable to get KT_VBR_NWMONITOR object");
        return UPLL_RC_ERR_GENERIC;
      }

      // allocate memory for key_nwm key_struct
      result_code = mgr->GetChildConfigKey(okey, NULL);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_ERROR("Memory allocation failed for key_nwm struct - %d",
                    result_code);
        return result_code;
      }

      key_vterm_if_flowfilter_entry_t *key_vtermif_ffe =
        reinterpret_cast<key_vterm_if_flowfilter_entry_t *>(ikey->get_key());
      // fill key_nwm from key/val VTERMIF_FLOWFILTER_ENTRY structs
      key_nwm_t *key_nwm = static_cast<key_nwm_t*>(
        okey->get_key());

      uuu::upll_strncpy(key_nwm->nwmonitor_name,
        val_flowfilter_entry->nwm_name,
        kMaxLenNwmName+1);

      uuu::upll_strncpy(key_nwm->vbr_key.vtn_key.vtn_name,
        key_vtermif_ffe->flowfilter_key.if_key.vterm_key.vtn_key.vtn_name,
        kMaxLenVtnName+1);
      // Check nwm_name exists in table
      result_code = mgr->UpdateConfigDB(okey, req->datatype, UNC_OP_READ,
                                      dmi, MAINTBL);

      if (UPLL_RC_ERR_INSTANCE_EXISTS != result_code) {
        UPLL_LOG_ERROR("NWM name in val_flowfilter_entry does not exists"
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
  }
  UPLL_LOG_DEBUG("ValidateAttribute Successfull.");
  return result_code;
}

upll_rc_t VtermIfFlowFilterEntryMoMgr::IsReferenced(IpcReqRespHeader *req,
                                                    ConfigKeyVal *ikey,
                                                    DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  if (NULL == ikey) return UPLL_RC_ERR_GENERIC;
  DbSubOp dbop = { kOpReadExist, kOpMatchNone, kOpInOutNone };
  // Check the  object existence
  result_code = UpdateConfigDB(ikey, req->datatype, UNC_OP_READ, dmi,
                               &dbop, MAINTBL);
  return result_code;
}

upll_rc_t VtermIfFlowFilterEntryMoMgr::GetValid(void *val, uint64_t indx,
                                            uint8_t *&valid,
                                            upll_keytype_datatype_t dt_type,
                                            MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  if (val == NULL) {
    UPLL_LOG_ERROR("Memory is not Allocated");
     return UPLL_RC_ERR_GENERIC;
  }

  if (tbl != MAINTBL) {
    UPLL_LOG_ERROR("Wrong table");
    return UPLL_RC_ERR_GENERIC;
  }

  val_flowfilter_entry_t *val_ffe = reinterpret_cast<val_flowfilter_entry_t *>
                                    (val);

  switch (indx) {
    case uudst::vterm_if_flowfilter_entry::kDbiFlowlistName:
       valid = &val_ffe->valid[UPLL_IDX_FLOWLIST_NAME_FFE];
       break;
    case uudst::vterm_if_flowfilter_entry::kDbiAction:
      valid = &val_ffe->valid[UPLL_IDX_ACTION_FFE];
       break;
    case uudst::vterm_if_flowfilter_entry::kDbiRedirectNode:
        valid = &val_ffe->valid[UPLL_IDX_REDIRECT_NODE_FFE];
       break;
    case uudst::vterm_if_flowfilter_entry::kDbiRedirectPort:
       valid = &val_ffe->valid[UPLL_IDX_REDIRECT_PORT_FFE];
       break;
    case uudst::vterm_if_flowfilter_entry::kDbiRedirectDirection:
       valid = &val_ffe->valid[UPLL_IDX_REDIRECT_DIRECTION_FFE];
       break;
    case uudst::vterm_if_flowfilter_entry::kDbiModifyDstMac:
        valid = &val_ffe->valid[UPLL_IDX_MODIFY_DST_MAC_FFE];
        break;
    case uudst::vterm_if_flowfilter_entry::kDbiModifySrcMac:
        valid = &val_ffe->valid[UPLL_IDX_MODIFY_SRC_MAC_FFE];
        break;
    case uudst::vterm_if_flowfilter_entry::kDbiNwmName:
        valid = &val_ffe->valid[UPLL_IDX_NWM_NAME_FFE];
        break;
    case uudst::vterm_if_flowfilter_entry::kDbiDscp:
        valid = &val_ffe->valid[UPLL_IDX_DSCP_FFE];
      break;
    case uudst::vterm_if_flowfilter_entry::kDbiPriority:
        valid = &val_ffe->valid[UPLL_IDX_PRIORITY_FFE];
        break;
    default:
       return UPLL_RC_ERR_GENERIC;
  }
  UPLL_LOG_DEBUG("GetValidAttributte is Succesfull");
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtermIfFlowFilterEntryMoMgr::ReadMo(IpcReqRespHeader *req,
                                            ConfigKeyVal *ikey,
                                            DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  controller_domain ctrlr_dom;
  ConfigKeyVal* dup_key = NULL, *l_key = NULL;

  DbSubOp dbop1 = { kOpReadSingle, kOpMatchNone, kOpInOutCtrlr|kOpInOutDomain|
                                                 kOpInOutFlag};

  result_code = ValidateMessage(req, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_ERROR("ValidateMessage %d", result_code);
    return result_code;
  }

  memset(&ctrlr_dom, 0, sizeof(controller_domain));
  switch (req->datatype) {
    //  Retrieving config information
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
                 (req->option1 == UNC_OPT1_DETAIL) &&
                 (req->option2 == UNC_OPT2_NONE)) {
        result_code =  DupConfigKeyVal(dup_key, ikey, MAINTBL);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_ERROR(" DupConfigKeyVal failed for dup_key%d ", result_code);
          return result_code;
        }

        result_code = ReadConfigDB(dup_key, req->datatype, UNC_OP_READ, dbop1,
                                   dmi, MAINTBL);
        if (result_code != UPLL_RC_SUCCESS) {
          delete dup_key;
          UPLL_LOG_ERROR("ReadConfigDB failed %d", result_code);
          return result_code;
        }
        uint8_t db_flag = 0;
        GET_USER_DATA_FLAGS(dup_key, db_flag);
        if ((db_flag & SET_FLAG_PORTMAP)) {
          UPLL_LOG_DEBUG("Portmap is configured");
        } else {
          UPLL_LOG_ERROR("Portmap not configured");
          DELETE_IF_NOT_NULL(dup_key);
          return UPLL_RC_ERR_NOT_ALLOWED_AT_THIS_TIME;
        }
        result_code =  DupConfigKeyVal(l_key, ikey, MAINTBL);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_ERROR(" DupConfigKeyVal failed for l_key%d ", result_code);
          delete dup_key;
          return result_code;
        }

        GET_USER_DATA_CTRLR_DOMAIN(dup_key, ctrlr_dom);
        SET_USER_DATA_CTRLR_DOMAIN(l_key, ctrlr_dom);

        result_code = ValidateCapability(req, ikey,
                   reinterpret_cast<const char *>(ctrlr_dom.ctrlr));
        if (result_code != UPLL_RC_SUCCESS) {
          delete dup_key;
          delete l_key;
          UPLL_LOG_ERROR("validate Capability Failed %d", result_code);
          return result_code;
        }

        // 1.Getting renamed name if renamed
        result_code = GetRenamedControllerKey(l_key, req->datatype, dmi,
                                              &ctrlr_dom);
        if (result_code != UPLL_RC_SUCCESS) {
          delete dup_key;
          delete l_key;
          UPLL_LOG_ERROR("GetRenamedControllerKey failed %d", result_code);
          return result_code;
        }

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
          UPLL_LOG_ERROR("SendReqToDriver failed for Key %d controller %s",
                         l_key->get_key_type(),
                         reinterpret_cast<char *>(ctrlr_dom.ctrlr));
          DELETE_IF_NOT_NULL(ipc_req.ckv_data);
          DELETE_IF_NOT_NULL(ipc_resp.ckv_data);
          DELETE_IF_NOT_NULL(dup_key);
          return ipc_resp.header.result_code;
        }

        if (ipc_resp.header.result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_ERROR("Driver response for Key %d controller %s result %d",
                        l_key->get_key_type(), ctrlr_dom.ctrlr,
                        ipc_resp.header.result_code);
          DELETE_IF_NOT_NULL(ipc_req.ckv_data);
          DELETE_IF_NOT_NULL(ipc_resp.ckv_data);
          DELETE_IF_NOT_NULL(dup_key);
          return ipc_resp.header.result_code;
        }
        ConfigKeyVal *okey = NULL;
        result_code = ConstructReadDetailResponse(dup_key,
                                                  ipc_resp.ckv_data,
                                                  ctrlr_dom,
                                                  &okey,
                                                   dmi);

        DELETE_IF_NOT_NULL(ipc_resp.ckv_data);
        DELETE_IF_NOT_NULL(dup_key);
        DELETE_IF_NOT_NULL(l_key);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_ERROR("ConstructReadDetailResponse error code (%d)",
                         result_code);
          return result_code;
        } else {
          if (okey != NULL) {
            ikey->ResetWith(okey);
            DELETE_IF_NOT_NULL(okey);
          }
        }
      }
      break;
    default:
      result_code = UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT;
  }
  return result_code;
}

upll_rc_t VtermIfFlowFilterEntryMoMgr::ReadSiblingMo(IpcReqRespHeader *req,
                                                   ConfigKeyVal *ikey,
                                                   bool begin,
                                                   DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  controller_domain ctrlr_dom;
  ConfigKeyVal *l_key = NULL, *tctrl_key = NULL;
  ConfigKeyVal *okey =NULL, *tmp_key = NULL;

  result_code = ValidateMessage(req, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_ERROR("validate Message Fialed %d", result_code);
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
      } else if ((req->datatype == UPLL_DT_STATE) &&
                 (req->option1 == UNC_OPT1_DETAIL) &&
                 (req->option2 == UNC_OPT2_NONE)) {
        result_code =  DupConfigKeyVal(tctrl_key, ikey, MAINTBL);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_ERROR("DupConfigKeyVal error (%d)", result_code);
          return result_code;
        }
        result_code = ReadInfoFromDB(req, tctrl_key, dmi, &ctrlr_dom);
        if (UPLL_RC_SUCCESS != result_code) {
          UPLL_LOG_ERROR("ReadConfigDb failed for tctrl_key err code(%d)",
                         result_code);
          DELETE_IF_NOT_NULL(tctrl_key);
          return result_code;
        }

        uint8_t db_flag = 0;
        GET_USER_DATA_FLAGS(tctrl_key, db_flag);
        if ((db_flag & SET_FLAG_PORTMAP)) {
          UPLL_LOG_ERROR("Portmap is configured");
        } else {
          UPLL_LOG_ERROR("Portmap not configured");
          DELETE_IF_NOT_NULL(tctrl_key);
          return UPLL_RC_ERR_NOT_ALLOWED_AT_THIS_TIME;
        }
        result_code =  DupConfigKeyVal(l_key, ikey, MAINTBL);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_ERROR(" DupConfigKeyVal failed for l_key%d ", result_code);
          DELETE_IF_NOT_NULL(tctrl_key);
          return result_code;
        }

        GET_USER_DATA_CTRLR_DOMAIN(tctrl_key, ctrlr_dom);
        SET_USER_DATA_CTRLR_DOMAIN(l_key, ctrlr_dom);

        result_code = ValidateCapability(req, ikey,
                   reinterpret_cast<const char *>(ctrlr_dom.ctrlr));
        if (result_code != UPLL_RC_SUCCESS) {
          delete l_key;
          DELETE_IF_NOT_NULL(tctrl_key);
          UPLL_LOG_ERROR("validate Capability Failed %d", result_code);
          return result_code;
        }

        // 1.Getting renamed name if renamed
        result_code = GetRenamedControllerKey(l_key, req->datatype, dmi,
                                              &ctrlr_dom);
        if (result_code != UPLL_RC_SUCCESS) {
          delete l_key;
          DELETE_IF_NOT_NULL(tctrl_key);
          UPLL_LOG_ERROR("GetRenamedControllerKey  failed %d", result_code);
          return result_code;
        }

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
        while (tmp_key != NULL) {
          reinterpret_cast<key_vterm_if_flowfilter_entry_t*>
              (l_key->get_key())->flowfilter_key.direction =
              reinterpret_cast<key_vterm_if_flowfilter_entry_t*>
              (tmp_key->get_key())->flowfilter_key.direction;

          reinterpret_cast<key_vterm_if_flowfilter_entry_t*>
              (l_key->get_key())->sequence_num =
              reinterpret_cast<key_vterm_if_flowfilter_entry_t*>
              (tmp_key->get_key())->sequence_num;
          ipc_req.ckv_data = l_key;

          if (!IpcUtil::SendReqToDriver(
                  (const char *)ctrlr_dom.ctrlr,
                  reinterpret_cast<char *>(ctrlr_dom.domain),
                  PFCDRIVER_SERVICE_NAME, PFCDRIVER_SVID_LOGICAL, &ipc_req,
                  true, &ipc_resp)) {
            UPLL_LOG_ERROR("SendReqToDriver failed for Key %d controller %s",
                           l_key->get_key_type(),
                           reinterpret_cast<char *>(ctrlr_dom.ctrlr));
            DELETE_IF_NOT_NULL(tctrl_key);
            DELETE_IF_NOT_NULL(ipc_resp.ckv_data);
            DELETE_IF_NOT_NULL(l_key);
            DELETE_IF_NOT_NULL(okey);
            return ipc_resp.header.result_code;
          }

          if (ipc_resp.header.result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_ERROR("Driver response for Key %d controller %s result %d",
                          l_key->get_key_type(), ctrlr_dom.ctrlr,
                          ipc_resp.header.result_code);
            DELETE_IF_NOT_NULL(tctrl_key);
            DELETE_IF_NOT_NULL(ipc_resp.ckv_data);
            DELETE_IF_NOT_NULL(l_key);
            DELETE_IF_NOT_NULL(okey);
            return ipc_resp.header.result_code;
          }

          result_code = ConstructReadDetailResponse(tmp_key,
                                                    ipc_resp.ckv_data,
                ctrlr_dom,
                                                    &okey,
                dmi);

          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_ERROR("ConstructReadDetailResponse error code (%d)",
                           result_code);
            DELETE_IF_NOT_NULL(tctrl_key);
            DELETE_IF_NOT_NULL(ipc_resp.ckv_data);
            DELETE_IF_NOT_NULL(l_key);
            DELETE_IF_NOT_NULL(okey);
            return result_code;
          }
          DELETE_IF_NOT_NULL(ipc_resp.ckv_data);
          tmp_key = tmp_key->get_next_cfg_key_val();
        }
        if ((okey != NULL) && (result_code == UPLL_RC_SUCCESS)) {
          ikey->ResetWith(okey);
          DELETE_IF_NOT_NULL(okey);
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

upll_rc_t VtermIfFlowFilterEntryMoMgr::CopyToConfigKey(ConfigKeyVal *&okey,
                                              ConfigKeyVal *ikey) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  if (!ikey || !(ikey->get_key())) {
    UPLL_LOG_ERROR("Input Key Not Valid");
    return UPLL_RC_ERR_GENERIC;
  }

  key_rename_vnode_info *key_rename = NULL;
  key_rename = reinterpret_cast<key_rename_vnode_info *> (ikey->get_key());
  key_vterm_if_flowfilter_entry_t * key_vterm_if_ff_entry =
            reinterpret_cast<key_vterm_if_flowfilter_entry_t*>
            (ConfigKeyVal::Malloc(sizeof(key_vterm_if_flowfilter_entry_t)));

  if (!strlen(reinterpret_cast<char *> (key_rename->old_unc_vtn_name))) {
    UPLL_LOG_ERROR("old_unc_vtn_name NULL");
     if (key_vterm_if_ff_entry) free(key_vterm_if_ff_entry);
     return UPLL_RC_ERR_GENERIC;
  }

  uuu::upll_strncpy(
       key_vterm_if_ff_entry->flowfilter_key.if_key.vterm_key.vtn_key.vtn_name,
       key_rename->old_unc_vtn_name,
       (kMaxLenVtnName + 1));

  if (UNC_KT_VTERMINAL == ikey->get_key_type()) {
    if (!strlen(reinterpret_cast<char *>(key_rename->old_unc_vnode_name))) {
      UPLL_LOG_ERROR("old_unc_vnode_name NULL");
      free(key_vterm_if_ff_entry);
      return UPLL_RC_ERR_GENERIC;
    }
    uuu::upll_strncpy(key_vterm_if_ff_entry->flowfilter_key.if_key.
                                             vterm_key.vterminal_name,
                      key_rename->old_unc_vnode_name, (kMaxLenVnodeName + 1));
  } else {
    if (!strlen(reinterpret_cast<char *>(key_rename->new_unc_vnode_name))) {
      UPLL_LOG_ERROR("new_unc_vnode_name NULL");
      free(key_vterm_if_ff_entry);
      return UPLL_RC_ERR_GENERIC;
    }
    uuu::upll_strncpy(key_vterm_if_ff_entry->flowfilter_key.if_key.
                                             vterm_key.vterminal_name,
                      key_rename->new_unc_vnode_name, (kMaxLenVnodeName + 1));
  }
  key_vterm_if_ff_entry->flowfilter_key.direction = 0xFE;

  okey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
                          IpctSt::kIpcStKeyVtermIfFlowfilterEntry,
                          key_vterm_if_ff_entry, NULL);

  if (!okey) {
    free(key_vterm_if_ff_entry);
    UPLL_LOG_ERROR("okey is null");
    return UPLL_RC_ERR_GENERIC;
  }
  return result_code;
}

upll_rc_t VtermIfFlowFilterEntryMoMgr::UpdateVnodeVal(ConfigKeyVal *ikey,
                                           DalDmlIntf *dmi,
                                           upll_keytype_datatype_t data_type,
                                           bool &no_rename) {
  UPLL_FUNC_TRACE;
  ConfigKeyVal *okey = NULL;
  ConfigKeyVal *kval = NULL;
  controller_domain ctrlr_dom;
  ctrlr_dom.ctrlr = NULL;
  ctrlr_dom.domain = NULL;

  uint8_t rename = 0;
  string vtn_name = "";
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  key_rename_vnode_info_t *key_rename =
             reinterpret_cast<key_rename_vnode_info_t *>(ikey->get_key());

  // Allocate ConfigKeyVal to update renamed key in FFE value
  result_code = GetChildConfigKey(okey, NULL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_ERROR("GetChildConfigKey failed");
    return result_code;
  }
  val_flowfilter_entry_t *val_ff_entry =
      reinterpret_cast<val_flowfilter_entry_t *>
      (ConfigKeyVal::Malloc(sizeof(val_flowfilter_entry_t)));
  okey->SetCfgVal(new ConfigVal(IpctSt::kIpcStValFlowfilterEntry,
                                val_ff_entry));

  if (ikey->get_key_type() == UNC_KT_FLOWLIST) {
    // If flowlist is renamed, update 'match flowlist' in FFE
    if (!strlen(reinterpret_cast<char *>(key_rename->old_flowlist_name))) {
      UPLL_LOG_ERROR("key_rename->old_flowlist_name NULL");
      DELETE_IF_NOT_NULL(okey);
      return UPLL_RC_ERR_GENERIC;
    }
    uuu::upll_strncpy(val_ff_entry->flowlist_name,
      key_rename->old_flowlist_name,
      (kMaxLenFlowListName + 1));
    val_ff_entry->valid[UPLL_IDX_FLOWLIST_NAME_FFE] = UNC_VF_VALID;
    UPLL_LOG_DEBUG("valid and flowlist name (%d) (%s)",
                   val_ff_entry->valid[UPLL_IDX_FLOWLIST_NAME_FFE],
                   val_ff_entry->flowlist_name);
  } else if ((ikey->get_key_type() == UNC_KT_VBRIDGE) ||
             (ikey->get_key_type() == UNC_KT_VROUTER) ||
             (ikey->get_key_type() == UNC_KT_VTERMINAL)) {
    // If vnode is renamed, update 'redirect destination' in FFE
    // for the parent vtn
    if (!strlen(reinterpret_cast<char *>(key_rename->old_unc_vnode_name))) {
      UPLL_LOG_ERROR("key_rename->old_unc_vnode_name NULL");
      DELETE_IF_NOT_NULL(okey);
      return UPLL_RC_ERR_GENERIC;
    }
    key_vterm_if_flowfilter_entry_t *vtermif_ffe_key =
        reinterpret_cast<key_vterm_if_flowfilter_entry_t*>(okey->get_key());
    uuu::upll_strncpy(
        vtermif_ffe_key->flowfilter_key.if_key.vterm_key.vtn_key.vtn_name,
        key_rename->new_unc_vtn_name, (kMaxLenVtnName + 1));
    uuu::upll_strncpy(val_ff_entry->redirect_node,
      key_rename->old_unc_vnode_name,
      (kMaxLenVnodeName + 1));
    val_ff_entry->valid[UPLL_IDX_REDIRECT_NODE_FFE] = UNC_VF_VALID;
    UPLL_LOG_DEBUG("valid and vbridge name (%d) (%s)",
                   val_ff_entry->valid[UPLL_IDX_REDIRECT_NODE_FFE],
                   val_ff_entry->redirect_node);
  }

  DbSubOp dbop = { kOpReadMultiple, kOpMatchNone,
    kOpInOutCtrlr | kOpInOutDomain | kOpInOutFlag };

  // Read the record of key structure and old flowlist name in maintbl
  result_code = ReadConfigDB(okey, data_type, UNC_OP_READ, dbop, dmi,
                             MAINTBL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_ERROR(" ReadConfigDB failed ");
    DELETE_IF_NOT_NULL(okey);
    return result_code;
  }
  ConfigKeyVal *first_ckv = okey;
  while (okey != NULL) {
    result_code = GetChildConfigKey(kval, okey);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_ERROR("GetChildConfigKey kval NULL");
      DELETE_IF_NOT_NULL(first_ckv);
      return result_code;
    }

    if (!kval) {
      UPLL_LOG_ERROR("GetChildConfigKey kval NULL");
      DELETE_IF_NOT_NULL(first_ckv);
      return UPLL_RC_ERR_GENERIC;
    }
    // Copy the new flowlist name in val_flowfilter_entry
    val_flowfilter_entry_t *val_ff_entry_new =
        reinterpret_cast<val_flowfilter_entry_t *>
        (ConfigKeyVal::Malloc(sizeof(val_flowfilter_entry_t)));
    if (!val_ff_entry_new) {
      UPLL_LOG_ERROR("val_ff_entry_new NULL");
      DELETE_IF_NOT_NULL(kval);
      DELETE_IF_NOT_NULL(first_ckv);
      return UPLL_RC_ERR_GENERIC;
    }

    if (ikey->get_key_type() == UNC_KT_FLOWLIST) {
    // New Name NuLL CHECK
    if (!strlen(reinterpret_cast<char *>(key_rename->new_flowlist_name))) {
      UPLL_LOG_ERROR("new_flowlist_name NULL");
      if (val_ff_entry_new) free(val_ff_entry_new);
      DELETE_IF_NOT_NULL(kval);
      DELETE_IF_NOT_NULL(first_ckv);
      return UPLL_RC_ERR_GENERIC;
    }
    // Copy the new flowlist_name into val_flowfilter_entry
    uuu::upll_strncpy(val_ff_entry_new->flowlist_name,
                     key_rename->new_flowlist_name,
                    (kMaxLenFlowListName + 1));
    val_ff_entry_new->valid[UPLL_IDX_FLOWLIST_NAME_FFE] = UNC_VF_VALID;
    UPLL_LOG_DEBUG("flowlist name and valid (%d) (%s)",
                    val_ff_entry_new->valid[UPLL_IDX_FLOWLIST_NAME_FFE],
                    val_ff_entry_new->flowlist_name);
    } else if ((ikey->get_key_type() == UNC_KT_VBRIDGE) ||
               (ikey->get_key_type() == UNC_KT_VROUTER) ||
               (ikey->get_key_type() == UNC_KT_VTERMINAL)) {
    // New Name NuLL CHECK
    if (!strlen(reinterpret_cast<char *>(key_rename->new_unc_vnode_name))) {
      UPLL_LOG_ERROR("new_unc_vnode_name NULL");
      if (val_ff_entry_new) free(val_ff_entry_new);
      DELETE_IF_NOT_NULL(kval);
      DELETE_IF_NOT_NULL(first_ckv);
      return UPLL_RC_ERR_GENERIC;
    }
    // Copy the new vbridge name into val_flowfilter_entry
    uuu::upll_strncpy(val_ff_entry_new->redirect_node,
                     key_rename->new_unc_vnode_name,
                    (kMaxLenVnodeName + 1));

    val_ff_entry_new->valid[UPLL_IDX_REDIRECT_NODE_FFE] = UNC_VF_VALID;
    UPLL_LOG_DEBUG("vbridge name and valid (%d) (%s)",
                    val_ff_entry_new->valid[UPLL_IDX_REDIRECT_NODE_FFE],
                    val_ff_entry_new->redirect_node);
    }
    ConfigVal *cval1 = new ConfigVal(IpctSt::kIpcStValFlowfilterEntry,
                                     val_ff_entry_new);

    kval->SetCfgVal(cval1);
    memset(&ctrlr_dom, 0, sizeof(controller_domain));
    result_code = GetControllerDomainID(okey, UPLL_DT_IMPORT, dmi);

    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_ERROR("Failed to Get the Controller Domain details, err:%d",
                     result_code);
      DELETE_IF_NOT_NULL(kval);
      DELETE_IF_NOT_NULL(first_ckv);
      return result_code;
    }
    GET_USER_DATA_CTRLR_DOMAIN(okey, ctrlr_dom);
    GET_USER_DATA_FLAGS(okey, rename);

    if (ikey->get_key_type() == UNC_KT_FLOWLIST) {
      if (!no_rename)
        rename = rename | FLOW_RENAME;
      else
        rename = rename & NO_FLOWLIST_RENAME;
    }

    SET_USER_DATA_FLAGS(kval, rename);
    SET_USER_DATA_CTRLR_DOMAIN(kval, ctrlr_dom);

    // Update the new flowlist name in MAINTBL
    result_code = UpdateConfigDB(kval, data_type, UNC_OP_UPDATE, dmi,
                  TC_CONFIG_GLOBAL, vtn_name, MAINTBL);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_ERROR("Create record Err in CANDIDATE DB(%d)",
        result_code);
      DELETE_IF_NOT_NULL(kval);
      DELETE_IF_NOT_NULL(first_ckv);
      return result_code;
    }
    DELETE_IF_NOT_NULL(kval);
    okey = okey->get_next_cfg_key_val();
  }
  DELETE_IF_NOT_NULL(first_ckv);
  UPLL_LOG_DEBUG("UpdateVnodeVal result_code (%d)", result_code);
  return result_code;
}

bool VtermIfFlowFilterEntryMoMgr::CompareValidValue(void *&val1,
                                                    void *val2,
                                                    bool copy_to_running) {
  UPLL_FUNC_TRACE;
  bool attr = true;
  val_flowfilter_entry_t *val_ff_entry1 =
      reinterpret_cast<val_flowfilter_entry_t *>(val1);
  val_flowfilter_entry_t *val_ff_entry2 =
      reinterpret_cast<val_flowfilter_entry_t *>(val2);

  for ( unsigned int loop = 0; loop < sizeof(val_ff_entry1->valid); ++loop ) {
      if ( UNC_VF_INVALID == val_ff_entry1->valid[loop] &&
                  UNC_VF_VALID == val_ff_entry2->valid[loop])
        val_ff_entry1->valid[loop] = UNC_VF_VALID_NO_VALUE;
  }

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
      val_ff_entry1->valid[UPLL_IDX_REDIRECT_NODE_FFE] =
      (copy_to_running)?UNC_VF_INVALID:UNC_VF_VALUE_NOT_MODIFIED;
  }
  if (val_ff_entry1->valid[UPLL_IDX_REDIRECT_PORT_FFE] == UNC_VF_VALID &&
     val_ff_entry2->valid[UPLL_IDX_REDIRECT_PORT_FFE] == UNC_VF_VALID) {
    if (!strcmp(reinterpret_cast<char *>(val_ff_entry1->redirect_port),
              reinterpret_cast<char *>(val_ff_entry2->redirect_port)))
      val_ff_entry1->valid[UPLL_IDX_REDIRECT_PORT_FFE] =
      (copy_to_running)?UNC_VF_INVALID:UNC_VF_VALUE_NOT_MODIFIED;
  }



  if (val_ff_entry1->valid[UPLL_IDX_REDIRECT_DIRECTION_FFE] == UNC_VF_VALID &&
     val_ff_entry2->valid[UPLL_IDX_REDIRECT_DIRECTION_FFE] == UNC_VF_VALID) {
    if (val_ff_entry1->redirect_direction == val_ff_entry2->redirect_direction)
      val_ff_entry1->valid[UPLL_IDX_REDIRECT_DIRECTION_FFE] = UNC_VF_INVALID;
  }

  if (val_ff_entry1->valid[UPLL_IDX_MODIFY_DST_MAC_FFE] == UNC_VF_VALID &&
     val_ff_entry2->valid[UPLL_IDX_MODIFY_DST_MAC_FFE] == UNC_VF_VALID) {
    if (!memcmp(reinterpret_cast<char *>(val_ff_entry1->modify_dstmac),
              reinterpret_cast<char *>(val_ff_entry2->modify_dstmac),
              sizeof(val_ff_entry2->modify_dstmac)))
      val_ff_entry1->valid[UPLL_IDX_MODIFY_DST_MAC_FFE] =
      (copy_to_running)?UNC_VF_INVALID:UNC_VF_VALUE_NOT_MODIFIED;
  }
  if (val_ff_entry1->valid[UPLL_IDX_MODIFY_SRC_MAC_FFE] == UNC_VF_VALID &&
     val_ff_entry2->valid[UPLL_IDX_MODIFY_SRC_MAC_FFE] == UNC_VF_VALID) {
    if (!memcmp(reinterpret_cast<char *>(val_ff_entry1->modify_srcmac),
              reinterpret_cast<char *>(val_ff_entry2->modify_srcmac),
       sizeof(val_ff_entry2->modify_srcmac)))
      val_ff_entry1->valid[UPLL_IDX_MODIFY_SRC_MAC_FFE] =
      (copy_to_running)?UNC_VF_INVALID:UNC_VF_VALUE_NOT_MODIFIED;
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
  UPLL_LOG_DEBUG("CompareValidValue : Success");
  for (unsigned int loop = 0;
      loop < sizeof(val_ff_entry1->valid)/ sizeof(uint8_t); ++loop) {
    if ((UNC_VF_VALID == (uint8_t) val_ff_entry1->valid[loop]) ||
        (UNC_VF_VALID_NO_VALUE == (uint8_t) val_ff_entry1->valid[loop]))
      attr = false;
  }
  return attr;
}

upll_rc_t VtermIfFlowFilterEntryMoMgr::AdaptValToDriver(
    ConfigKeyVal *ckv_new,
    ConfigKeyVal *ck_old,  // can be NULL, unused argument
    unc_keytype_operation_t op,
    upll_keytype_datatype_t dt_type,
    unc_key_type_t keytype,
    DalDmlIntf *dmi,
    bool *not_send_to_drv,  // can be NULL, unused argument
    bool audit_update_phase) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  if (!ckv_new || !ckv_new->get_key()) {
    UPLL_LOG_INFO("input key is null");
    return UPLL_RC_ERR_GENERIC;
  }
  if (!ckv_new->get_cfg_val() || !ckv_new->get_cfg_val()->get_val()) {
    UPLL_LOG_INFO("Val structure is NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  if (op == UNC_OP_CREATE || op == UNC_OP_UPDATE)  {
    // Perform the Semantic check for NWM which is referred in the
    // flowfilter_entry. If the referred NWM is deleted then return
    // SEMANTIC ERROR
    result_code = PerformSemanticCheckForNWM(ckv_new, dmi, dt_type);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_INFO("VBR FFE: Network Monitor validation failed %d",
            result_code);
        return result_code;
      }
    //  redirect destination is configured  and interface in UNC is
    //  PortMapped and  redirect-direction is OUT, then for the
    //  redirect-destination vbridge/vbridgeif UPLL need to send the
    //  corrosponding vexternal/vexternalif to the driver.
    //  The corrosponding vexternal/vexternalif conversion is done in the
    //  VerifyRedirectDestination API
    result_code =  VerifyRedirectDestination(ckv_new,
          dmi,
          dt_type);
    if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_INFO("VerifyRedirectDestination  failed result code - %d",
            result_code);
        return result_code;
    }
  }
  return result_code;
}

upll_rc_t VtermIfFlowFilterEntryMoMgr::TxUpdateController(
    unc_key_type_t keytype,
    uint32_t session_id,
    uint32_t config_id,
    uuc::UpdateCtrlrPhase phase,
    set<string> *affected_ctrlr_set,
    DalDmlIntf *dmi,
    ConfigKeyVal **err_ckv,
    TxUpdateUtil *tx_util,
    TcConfigMode config_mode,
    std::string vtn_name)  {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *req = NULL, *nreq = NULL, *ck_main = NULL;
  controller_domain_t ctrlr_dom;
  ctrlr_dom.ctrlr = NULL;
  ctrlr_dom.domain = NULL;
  DalCursor *dal_cursor_handle = NULL;
  DalResultCode db_result;
  uint8_t db_flag = 0;
  uint8_t db_flag_running = 0;

  // TxUpdate skipped if config mode is VIRTUAL
  if (config_mode == TC_CONFIG_VIRTUAL) {
    return UPLL_RC_SUCCESS;
  }

  if (affected_ctrlr_set == NULL)
      return UPLL_RC_ERR_GENERIC;

  unc_keytype_operation_t op = (phase == uuc::kUpllUcpCreate)?UNC_OP_CREATE:
               ((phase == uuc::kUpllUcpUpdate)?UNC_OP_UPDATE:
               ((phase == uuc::kUpllUcpDelete)?UNC_OP_DELETE:UNC_OP_INVALID));

  result_code = DiffConfigDB(UPLL_DT_CANDIDATE, UPLL_DT_RUNNING,
                     op, req, nreq, &dal_cursor_handle, dmi, config_mode,
                     vtn_name, MAINTBL);

  unc_keytype_operation_t op1 = op;
  while (result_code == UPLL_RC_SUCCESS) {
    if (tx_util->GetErrCount() > 0) {
      UPLL_LOG_ERROR("TxUpdateUtil says exit the loop.");
      dmi->CloseCursor(dal_cursor_handle, true);
      DELETE_IF_NOT_NULL(nreq);
      DELETE_IF_NOT_NULL(req);
      return UPLL_RC_ERR_GENERIC;
    }

    ck_main = NULL;
    db_result = dmi->GetNextRecord(dal_cursor_handle);
    result_code = DalToUpllResCode(db_result);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG(" GetNextRecord failed err code(%d)", result_code);
      break;
    }

    UPLL_LOG_TRACE("KeyVal= %s ", (req->ToStrAll()).c_str());

    switch (op)   {
      case UNC_OP_CREATE:
      case UNC_OP_UPDATE:
      case UNC_OP_DELETE:
     /* fall through intended */
        op1 = op;
        result_code = DupConfigKeyVal(ck_main, req, MAINTBL);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_ERROR("DupConfigKeyVal failed %d", result_code);
          dmi->CloseCursor(dal_cursor_handle, true);
          DELETE_IF_NOT_NULL(req);
          DELETE_IF_NOT_NULL(nreq);
          return result_code;
        }
       break;
      default:
        UPLL_LOG_ERROR("TxUpdateController Invalid operation");
        dmi->CloseCursor(dal_cursor_handle, true);
        DELETE_IF_NOT_NULL(req);
        DELETE_IF_NOT_NULL(nreq);
        return UPLL_RC_ERR_GENERIC;
    }
    // Perform mode specific sematic check
    // check the record existence in running database
    if (TC_CONFIG_GLOBAL != config_mode) {
      result_code = PerformModeSpecificSemanticCheck(ck_main, dmi, session_id,
                                                     config_id, op, keytype,
                                                     config_mode, vtn_name);
      if ((result_code != UPLL_RC_SUCCESS) &&
          (result_code != UPLL_RC_ERR_INSTANCE_EXISTS)) {
        UPLL_LOG_DEBUG("Mode specific semantic check failed %d\n", result_code);
        DELETE_IF_NOT_NULL(req);
        DELETE_IF_NOT_NULL(nreq);
        dmi->CloseCursor(dal_cursor_handle, true);
        if (result_code == UPLL_RC_ERR_CFG_SEMANTIC) {
          *err_ckv = ck_main;
          return result_code;
        }
        DELETE_IF_NOT_NULL(ck_main);
        return result_code;
      }
    }
    GET_USER_DATA_CTRLR_DOMAIN(ck_main, ctrlr_dom);
    if (ctrlr_dom.ctrlr == NULL) {
      UPLL_LOG_ERROR("ctrlr id is null");
      DELETE_IF_NOT_NULL(ck_main);
      dmi->CloseCursor(dal_cursor_handle, true);
      DELETE_IF_NOT_NULL(req);
      DELETE_IF_NOT_NULL(nreq);
      return UPLL_RC_ERR_GENERIC;
    }
    UPLL_LOG_DEBUG("Controller : %s; Domain : %s", ctrlr_dom.ctrlr,
                 ctrlr_dom.domain);
    GET_USER_DATA_FLAGS(ck_main, db_flag);

    upll_keytype_datatype_t dt_type = (op1 == UNC_OP_DELETE)?
             UPLL_DT_RUNNING:UPLL_DT_CANDIDATE;
    if (op1 == UNC_OP_CREATE) {
      result_code = AdaptValToDriver(ck_main, NULL, op1,
                                       dt_type, keytype, dmi, NULL, false);
      if (result_code != UPLL_RC_SUCCESS) {
        if (result_code == UPLL_RC_ERR_CFG_SEMANTIC) {
          UPLL_LOG_DEBUG(" Semantic validation "
                         "failed %d", result_code);
          *err_ckv = ck_main;
        } else {
          UPLL_LOG_INFO("AdapatValToDriver failed %d", result_code);
          DELETE_IF_NOT_NULL(ck_main);
        }
        dmi->CloseCursor(dal_cursor_handle, true);
        delete req;
        if (nreq) delete nreq;
        return result_code;
      }
    }

    if (!(SET_FLAG_PORTMAP & db_flag)) {
      if (op != UNC_OP_UPDATE) {
        DELETE_IF_NOT_NULL(ck_main);
        continue;
      } else {
          ConfigKeyVal *tmp_ckv = NULL;
          result_code = DupConfigKeyVal(tmp_ckv, ck_main, MAINTBL);
          if (result_code != UPLL_RC_SUCCESS) {
             UPLL_LOG_TRACE("DupConfigKeyVal failed %d", result_code);
             DELETE_IF_NOT_NULL(ck_main);
             dmi->CloseCursor(dal_cursor_handle, true);
             delete req;
             if (nreq) {
               delete nreq;
             }
             return result_code;
          }
          // Call the Filter Attribute for avoiding the duplicate validation of
          // redirect node and redirect port attribute
          void *main = GetVal(tmp_ckv);
          void *val_nrec = (nreq) ? GetVal(nreq) : NULL;
          if (FilterAttributes(main, val_nrec, false, op)) {
            DELETE_IF_NOT_NULL(ck_main);
            DELETE_IF_NOT_NULL(tmp_ckv);
            continue;
          }
          // Perform semantic validation for redirection and the
          // networkmonitor
          result_code = AdaptValToDriver(tmp_ckv, NULL, op1,
                                         dt_type, keytype, dmi, NULL, false);
          DELETE_IF_NOT_NULL(tmp_ckv);
          if (result_code != UPLL_RC_SUCCESS) {
            if (result_code == UPLL_RC_ERR_CFG_SEMANTIC) {
               UPLL_LOG_DEBUG(" Semantic validation "
                       "failed %d", result_code);
               *err_ckv = ck_main;
            } else {
                 UPLL_LOG_INFO("AdapatValToDriver failed %d", result_code);
                 DELETE_IF_NOT_NULL(ck_main);
            }
            dmi->CloseCursor(dal_cursor_handle, true);
            delete req;
            if (nreq) delete nreq;
            return result_code;
          }
          GET_USER_DATA_FLAGS(nreq, db_flag_running);
          if (!(SET_FLAG_PORTMAP & db_flag_running)) {
             UPLL_LOG_DEBUG("Portmap flag is not set for vtermifFFEntry");
             DELETE_IF_NOT_NULL(ck_main);
             continue;
          }
          op1 = UNC_OP_DELETE;
          db_flag = db_flag_running;
      }
    } else if (op == UNC_OP_UPDATE) {
      GET_USER_DATA_FLAGS(nreq, db_flag_running);
      if (!(SET_FLAG_PORTMAP & db_flag_running)) {
        UPLL_LOG_DEBUG("Portmap flag is not set at running");
        op1 = UNC_OP_CREATE;
      } else {
        void *main = GetVal(ck_main);
        void *val_nrec = (nreq) ? GetVal(nreq) : NULL;
        FilterAttributes(main, val_nrec, false, op);
      }
      // Perform semantic validation for redirection and the
      // networkmonitor
      result_code = AdaptValToDriver(ck_main, NULL, op1,
                                     dt_type, keytype, dmi, NULL, false);
      if (result_code != UPLL_RC_SUCCESS) {
        if (result_code == UPLL_RC_ERR_CFG_SEMANTIC) {
          UPLL_LOG_DEBUG(" Semantic validation "
                         "failed %d", result_code);
          *err_ckv = ck_main;
        } else {
          UPLL_LOG_INFO("AdapatValToDriver failed %d", result_code);
          DELETE_IF_NOT_NULL(ck_main);
        }
        dmi->CloseCursor(dal_cursor_handle, true);
        DELETE_IF_NOT_NULL(req);
        DELETE_IF_NOT_NULL(nreq);
        return result_code;
      }
    }
    result_code = GetRenamedControllerKey(ck_main, dt_type,
                                          dmi, &ctrlr_dom);

    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG(" GetRenamedControllerKey failed err code(%d)",
                      result_code);
      DELETE_IF_NOT_NULL(ck_main);
      break;
    }
    ConfigKeyVal *ckv_driver = NULL;
    result_code = DupConfigKeyVal(ckv_driver, ck_main, MAINTBL);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_TRACE("DupConfigKeyVal failed %d", result_code);
      DELETE_IF_NOT_NULL(ck_main);
      dmi->CloseCursor(dal_cursor_handle, true);
      DELETE_IF_NOT_NULL(req);
      DELETE_IF_NOT_NULL(nreq);
      return result_code;
    }
    if (op1 == UNC_OP_UPDATE) {
      ConfigVal *old_cval = ((nreq->get_cfg_val())->DupVal());
      ckv_driver->AppendCfgVal(old_cval);
    }

    // Inserting the controller to Set
    affected_ctrlr_set->insert
      (string(reinterpret_cast<char *>(ctrlr_dom.ctrlr)));

    DELETE_IF_NOT_NULL(ck_main);

    ConfigKeyVal *ckv_unc = NULL;
    result_code = DupConfigKeyVal(ckv_unc, req, MAINTBL);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_TRACE("DupConfigKeyVal failed %d", result_code);
      dmi->CloseCursor(dal_cursor_handle, true);
      DELETE_IF_NOT_NULL(req);
      DELETE_IF_NOT_NULL(nreq);
      DELETE_IF_NOT_NULL(ckv_driver);
      return result_code;
    }

    result_code = tx_util->EnqueueRequest(session_id, config_id,
                                          UPLL_DT_CANDIDATE, op1, dmi,
                                          ckv_driver, ckv_unc, string());
    if (result_code != UPLL_RC_SUCCESS) {
      dmi->CloseCursor(dal_cursor_handle, true);
      DELETE_IF_NOT_NULL(req);
      DELETE_IF_NOT_NULL(nreq);
      DELETE_IF_NOT_NULL(ckv_driver);
      DELETE_IF_NOT_NULL(ckv_unc);
      return result_code;
    }
  }
  dmi->CloseCursor(dal_cursor_handle, true);
  DELETE_IF_NOT_NULL(req);
  DELETE_IF_NOT_NULL(nreq);
  result_code = (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) ?
      UPLL_RC_SUCCESS : result_code;
  return result_code;
}

upll_rc_t VtermIfFlowFilterEntryMoMgr::SetPortmapConfiguration(
                                 ConfigKeyVal *ikey,
                                 upll_keytype_datatype_t dt_type,
                                 DalDmlIntf *dmi,
                                 InterfacePortMapInfo flags,
                                 TcConfigMode config_mode,
                                 string vtn_name) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_ERR_GENERIC;
  if (NULL == ikey || NULL == ikey->get_key()) {
    return result_code;
  }
  ConfigKeyVal *ckv = NULL;
  uint8_t *ctrlr_id = NULL;
  controller_domain ctrlr_dom;
  memset(&ctrlr_dom, 0, sizeof(ctrlr_dom));

  result_code = GetChildConfigKey(ckv, ikey);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_ERROR("GetChildConfigKey failed %d", result_code);
    return result_code;
  }
  DbSubOp dbop = { kOpReadMultiple, kOpMatchNone, kOpInOutFlag };
  result_code = ReadConfigDB(ckv, dt_type ,
                             UNC_OP_READ, dbop, dmi, MAINTBL);
  if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
    UPLL_LOG_DEBUG("No Recrods in the Vterm_If_FlowFilter_Entry Table");
    DELETE_IF_NOT_NULL(ckv);
    return UPLL_RC_SUCCESS;
  }
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_ERROR("Read ConfigDB failure %d", result_code);
    DELETE_IF_NOT_NULL(ckv);
    return result_code;
  }
  uint8_t  flag_port_map = 0;
  ConfigKeyVal *ckv_first = ckv;
  while (ckv) {
    flag_port_map = 0;
    val_flowfilter_entry_t *flowfilter_val =
      reinterpret_cast<val_flowfilter_entry_t *> (GetVal(ckv));
    GET_USER_DATA_FLAGS(ckv, flag_port_map);
    if (flags & kPortMapConfigured) {
      UPLL_LOG_DEBUG("portmap for flowfilterentry");
      flag_port_map |=  SET_FLAG_PORTMAP;
    } else {
      flag_port_map &= SET_FLAG_NO_VLINK_PORTMAP;
    }
    SET_USER_DATA_FLAGS(ckv, flag_port_map);

    UPLL_LOG_DEBUG("SET_USER_DATA_FLAGS flag_port_map %d", flag_port_map);
    DbSubOp dbop_up = { kOpNotRead, kOpMatchNone, kOpInOutFlag };
    result_code = UpdateConfigDB(ckv, dt_type, UNC_OP_UPDATE,
                                 dmi, &dbop_up, config_mode,
                                 vtn_name, MAINTBL);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_ERROR("Unable to update portmap for flowfilterentry");
      DELETE_IF_NOT_NULL(ckv_first);
      return result_code;
    }
    if (flowfilter_val->valid[UPLL_IDX_FLOWLIST_NAME_FFE] == UNC_VF_VALID) {
      unc_keytype_operation_t op = UNC_OP_INVALID;
      FlowListMoMgr *mgr = reinterpret_cast<FlowListMoMgr *>
          (const_cast<MoManager *> (GetMoManager(UNC_KT_FLOWLIST)));
      if ((flag_port_map & SET_FLAG_VLINK) ||
           (flag_port_map & SET_FLAG_PORTMAP)) {
          op = UNC_OP_CREATE;
      } else  {
          op = UNC_OP_DELETE;
      }
      result_code = GetControllerDomainID(ikey, dt_type, dmi);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_ERROR("Failed to Get the Controller Domain details, err:%d",
                   result_code);
        DELETE_IF_NOT_NULL(ckv_first);
        return result_code;
      }
      GET_USER_DATA_CTRLR_DOMAIN(ikey, ctrlr_dom);
      UPLL_LOG_DEBUG("ctrlrid %s, domainid %s",
                 ctrlr_dom.ctrlr, ctrlr_dom.domain);
      ctrlr_id = ctrlr_dom.ctrlr;
      std::string temp_vtn_name;
      if (TC_CONFIG_VTN == config_mode) {
        temp_vtn_name = vtn_name;
      } else {
        key_vterm_if_flowfilter_entry_t *temp_key = reinterpret_cast<
            key_vterm_if_flowfilter_entry_t *>(ikey->get_key());
        temp_vtn_name = reinterpret_cast<const char*>(
            temp_key->flowfilter_key.if_key.vterm_key.vtn_key.vtn_name);
      }
      result_code = mgr->AddFlowListToController(
           reinterpret_cast<char*>(flowfilter_val->flowlist_name), dmi,
           reinterpret_cast<char *>(ctrlr_id), dt_type, op,
           config_mode, temp_vtn_name, false);
      if (result_code != UPLL_RC_SUCCESS) {
         UPLL_LOG_ERROR(" Send delete request to flowlist failed. err code(%d)",
                      result_code);
         DELETE_IF_NOT_NULL(ckv_first);
         return result_code;
      }
    }
    ckv = ckv->get_next_cfg_key_val();
  }
  DELETE_IF_NOT_NULL(ckv_first);
  return UPLL_RC_SUCCESS;
}

// TODO(pom): check whether needs to be removed
upll_rc_t VtermIfFlowFilterEntryMoMgr::GetVexternalInformation(
                   ConfigKeyVal* ck_main,
                   upll_keytype_datatype_t dt_type,
                   pfcdrv_val_flowfilter_entry_t*& pfc_val,
                   uint8_t db_flag, DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtermIfFlowFilterEntryMoMgr::GetControllerDomainID(ConfigKeyVal *ikey,
                                          upll_keytype_datatype_t dt_type,
                                          DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  controller_domain ctrlr_dom;
  memset(&ctrlr_dom, 0, sizeof(controller_domain));
  VtermIfMoMgr *mgrvtermif =
      reinterpret_cast<VtermIfMoMgr *>(const_cast<MoManager *>(GetMoManager(
                  UNC_KT_VTERM_IF)));

  ConfigKeyVal *ckv = NULL;
  result_code = mgrvtermif->GetChildConfigKey(ckv, NULL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_ERROR("Unable to get the ParentConfigKey, resultcode=%d",
                    result_code);
    return result_code;
  }

  key_vterm_if_flowfilter_entry *temp_key =
      reinterpret_cast<key_vterm_if_flowfilter_entry_t*>(ikey->get_key());

  key_vterm_if_t *vterm_if_key =
      reinterpret_cast<key_vterm_if_t*>(ckv->get_key());

  uuu::upll_strncpy(vterm_if_key->vterm_key.vtn_key.vtn_name,
                    temp_key->flowfilter_key.if_key.vterm_key.vtn_key.vtn_name,
                    kMaxLenVtnName + 1);
  uuu::upll_strncpy(vterm_if_key->vterm_key.vterminal_name,
                    temp_key->flowfilter_key.if_key.vterm_key.vterminal_name,
                    kMaxLenVnodeName + 1);

  uuu::upll_strncpy(vterm_if_key->if_name,
                    temp_key->flowfilter_key.if_key.if_name,
                    kMaxLenInterfaceName + 1);


  ConfigKeyVal *vterm_key = NULL;
  result_code = mgrvtermif->GetParentConfigKey(vterm_key, ckv);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_ERROR("Failed to get the parent ConfigKey, err %d", result_code);
    DELETE_IF_NOT_NULL(ckv);  // Resource Leak Fix
    return result_code;
  }

  result_code = mgrvtermif->GetControllerDomainId(vterm_key, dt_type,
                                                &ctrlr_dom, dmi);
  if ((result_code != UPLL_RC_SUCCESS) || (ctrlr_dom.ctrlr == NULL)
      || (ctrlr_dom.domain == NULL)) {
    UPLL_LOG_ERROR("GetControllerDomainId error err code(%d)", result_code);
    DELETE_IF_NOT_NULL(ckv);
    DELETE_IF_NOT_NULL(vterm_key);
    return result_code;
  }

  UPLL_LOG_DEBUG("ctrlrid %s, domainid %s",
                 ctrlr_dom.ctrlr, ctrlr_dom.domain);
  SET_USER_DATA_CTRLR_DOMAIN(ikey, ctrlr_dom);

  DELETE_IF_NOT_NULL(ckv);
  DELETE_IF_NOT_NULL(vterm_key);
  return result_code;
}

upll_rc_t VtermIfFlowFilterEntryMoMgr::GetParentConfigKey(ConfigKeyVal *&okey,
                                                        ConfigKeyVal *ikey) {
  UPLL_FUNC_TRACE;

  if (!ikey) {
    UPLL_LOG_ERROR(" Input Key is NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  DELETE_IF_NOT_NULL(okey);
  unc_key_type_t ikey_type = ikey->get_key_type();
  if (ikey_type != UNC_KT_VTERMIF_FLOWFILTER_ENTRY) {
    UPLL_LOG_ERROR(" Invalid key type received. Key type - %d", ikey_type);
    return UPLL_RC_ERR_GENERIC;
  }

  key_vterm_if_flowfilter_entry_t *pkey =
      reinterpret_cast<key_vterm_if_flowfilter_entry_t*>(ikey->get_key());
  if (!pkey) {
    UPLL_LOG_ERROR(" Input vterm if flow filter entry key is NULL ");
    return UPLL_RC_ERR_GENERIC;
  }

  key_vterm_if_flowfilter_t *vterm_if_ff_key =
      reinterpret_cast<key_vterm_if_flowfilter_t*>
      (ConfigKeyVal::Malloc(sizeof(key_vterm_if_flowfilter_t)));

  uuu::upll_strncpy(vterm_if_ff_key->if_key.vterm_key.vtn_key.vtn_name,
                    reinterpret_cast<key_vterm_if_flowfilter_entry_t *>
                    (pkey)->flowfilter_key.if_key.vterm_key.vtn_key.vtn_name,
                    kMaxLenVtnName + 1);
  uuu::upll_strncpy(vterm_if_ff_key->if_key.vterm_key.vterminal_name,
                    reinterpret_cast<key_vterm_if_flowfilter_entry_t *>
                    (pkey)->flowfilter_key.if_key.vterm_key.vterminal_name,
                    kMaxLenVnodeName + 1);
  uuu::upll_strncpy(vterm_if_ff_key->if_key.if_name,
                    reinterpret_cast<key_vterm_if_flowfilter_entry_t *>
                    (pkey)->flowfilter_key.if_key.if_name,
                    kMaxLenInterfaceName + 1);
  vterm_if_ff_key->direction =
      reinterpret_cast<key_vterm_if_flowfilter_entry_t *>
      (pkey)->flowfilter_key.direction;
  okey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER,
                          IpctSt::kIpcStKeyVtermIfFlowfilter,
                          vterm_if_ff_key, NULL);
  SET_USER_DATA(okey, ikey);
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtermIfFlowFilterEntryMoMgr::ConstructReadDetailResponse(
    ConfigKeyVal *ikey,
    ConfigKeyVal *drv_resp_ckv,
    controller_domain ctrlr_dom ,
    ConfigKeyVal **okey,
     DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *tmp_okey = NULL;
  ConfigVal *drv_resp_val = NULL;
  drv_resp_val =  drv_resp_ckv->get_cfg_val();
  result_code =  GetChildConfigKey(tmp_okey, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_ERROR("GetChildConfigKey failed err code (%d)", result_code);
    return result_code;
  }
  while (drv_resp_val != NULL) {
    val_flowfilter_entry_t *val_entry = NULL;
    if (IpctSt::kIpcStValFlowfilterEntry ==
      drv_resp_val->get_st_num()) {
      UPLL_LOG_TRACE("Get the val struct");
      val_entry = reinterpret_cast< val_flowfilter_entry_t *>
          (drv_resp_val->get_val());
      result_code =  SetRedirectNodeAndPortForRead(ikey ,
                                                   ctrlr_dom,
                                                   val_entry,
                                                   dmi);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("SetRedirectNodeAndPortForRead Fails - %d",
          result_code);
      DELETE_IF_NOT_NULL(tmp_okey);
      return result_code;
    }
  }
  drv_resp_val = drv_resp_val->get_next_cfg_val();
  if (!drv_resp_val)
  break;
  }
  tmp_okey->AppendCfgVal(drv_resp_ckv->GetCfgValAndUnlink());
  if (*okey == NULL) {
    *okey = tmp_okey;
  } else {
    (*okey)->AppendCfgKeyVal(tmp_okey);
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtermIfFlowFilterEntryMoMgr::DeleteChildrenPOM(
          ConfigKeyVal *ikey, upll_keytype_datatype_t dt_type,
          DalDmlIntf *dmi,
          TcConfigMode config_mode,
          string vtn_name) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_ERR_GENERIC;
  int nattr = 0;
  BindInfo *binfo = NULL;
  DalCursor *dal_cursor_handle = NULL;
  string query_string;
  unc_key_type_t deletedkt;

  if (NULL == ikey || NULL == dmi) return result_code;

  key_vterm_if_flowfilter_entry_t *pkey =
      reinterpret_cast<key_vterm_if_flowfilter_entry_t*>(ikey->get_key());
  if (!pkey) {
    UPLL_LOG_DEBUG(" Input vterm_if flow filter entry key is NULL ");
    return UPLL_RC_ERR_GENERIC;
  }

  if (((pkey->flowfilter_key.if_key.vterm_key.vterminal_name)[0]) == '\0') {
    deletedkt = UNC_KT_VTN;
  } else if (((pkey->flowfilter_key.if_key.if_name)[0]) == '\0') {
    deletedkt = UNC_KT_VTERMINAL;
  } else if (pkey->flowfilter_key.direction == 0xFE) {
    deletedkt = UNC_KT_VTERM_IF;
  } else {
    deletedkt = UNC_KT_VTERMIF_FLOWFILTER;
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

  ConfigKeyVal *vtermif_ffe_ckv  = NULL;
  result_code = GetChildConfigKey(vtermif_ffe_ckv, NULL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetChildConfigKey failed");
    return result_code;
  }
  ConfigVal* vtermif_ffe_cv = NULL;
  result_code = AllocVal(vtermif_ffe_cv, UPLL_DT_CANDIDATE, MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Error in AllocVal");
    DELETE_IF_NOT_NULL(vtermif_ffe_ckv);
    return result_code;
  }
  vtermif_ffe_ckv->SetCfgVal(vtermif_ffe_cv);
  GET_USER_DATA(vtermif_ffe_ckv);
  uint32_t count = 0;
  void *tkey = vtermif_ffe_ckv->get_key();
  void *p = reinterpret_cast<void *>(reinterpret_cast<char *>(tkey) +
                                     binfo[0].offset);
  dal_bind_info.BindOutput(binfo[0].index, binfo[0].app_data_type,
                           binfo[0].array_size, p);
  tkey = vtermif_ffe_ckv->get_user_data();
  p = reinterpret_cast<void *>(reinterpret_cast<char *>(tkey) +
                               binfo[5].offset);
  dal_bind_info.BindOutput(binfo[5].index, binfo[5].app_data_type,
                           binfo[5].array_size, p);
  tkey = vtermif_ffe_ckv->get_cfg_val()->get_val();
  p = reinterpret_cast<void *>(reinterpret_cast<char *>(tkey) +
                               binfo[7].offset);
  dal_bind_info.BindOutput(binfo[7].index, binfo[7].app_data_type,
                           binfo[7].array_size, p);

  dal_bind_info.BindOutput(uud::schema::DAL_COL_STD_INTEGER,
                           uud::kDalUint32, 1, &count);

  result_code = DalToUpllResCode(dmi->ExecuteAppQueryMultipleRecords(
                 query_string, 0, &dal_bind_info,
                 &dal_cursor_handle));
  while (result_code == UPLL_RC_SUCCESS) {
    result_code = DalToUpllResCode(dmi->GetNextRecord(dal_cursor_handle));
    if (UPLL_RC_SUCCESS == result_code) {
      // Call function to update refcount in scratch table
      key_vterm_if_flowfilter_entry_t *vtermif_ffe_key =
          reinterpret_cast<key_vterm_if_flowfilter_entry_t *>
          (vtermif_ffe_ckv->get_key());
      vtn_name = reinterpret_cast<const char *>
          (vtermif_ffe_key->flowfilter_key.if_key.vterm_key.vtn_key.vtn_name);
      uint8_t *ctrlr_id = NULL;
      GET_USER_DATA_CTRLR(vtermif_ffe_ckv, ctrlr_id);
      val_flowfilter_entry_t *vtermif_ffe_val =
          reinterpret_cast<val_flowfilter_entry_t *>(GetVal(vtermif_ffe_ckv));

      FlowListMoMgr *fl_mgr =
          reinterpret_cast<FlowListMoMgr *>
          (const_cast<MoManager *>(GetMoManager(UNC_KT_FLOWLIST)));
      if (NULL == fl_mgr) {
        UPLL_LOG_DEBUG("fl_mgr is NULL");
        DELETE_IF_NOT_NULL(vtermif_ffe_ckv);
        return UPLL_RC_ERR_GENERIC;
      }
      ConfigKeyVal *fl_ckv  = NULL;
      result_code = fl_mgr->GetChildConfigKey(fl_ckv, NULL);
      if (UPLL_RC_SUCCESS != result_code) {
        UPLL_LOG_DEBUG("GetChildConfigKey failed");
        DELETE_IF_NOT_NULL(vtermif_ffe_ckv);
        return result_code;
      }
      key_flowlist_t *fl_key = reinterpret_cast<key_flowlist_t *>
          (fl_ckv->get_key());
      uuu::upll_strncpy(fl_key->flowlist_name,
                        vtermif_ffe_val->flowlist_name,
                        (kMaxLenFlowListName+1));
      SET_USER_DATA_CTRLR(fl_ckv, ctrlr_id);

      result_code = fl_mgr->UpdateRefCountInScratchTbl(fl_ckv, dmi,
                                                       dt_type, UNC_OP_DELETE,
                                                       config_mode, vtn_name,
                                                       count);
      if (UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code &&
          UPLL_RC_SUCCESS != result_code) {
        UPLL_LOG_DEBUG("UpdateRefCountInScratchTbl returned %d", result_code);
        DELETE_IF_NOT_NULL(vtermif_ffe_ckv);
        DELETE_IF_NOT_NULL(fl_ckv);
        return result_code;
      } else if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
        result_code = fl_mgr->InsertRecInScratchTbl(fl_ckv, dmi,
                                                    dt_type, UNC_OP_DELETE,
                                                    config_mode, vtn_name,
                                                    count);
        if (UPLL_RC_SUCCESS != result_code) {
          UPLL_LOG_DEBUG("InsertRecInScratchTbl failed %d", result_code);
          DELETE_IF_NOT_NULL(vtermif_ffe_ckv);
          DELETE_IF_NOT_NULL(fl_ckv);
          return result_code;
        }
      }
      DELETE_IF_NOT_NULL(fl_ckv);
    } else if (UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
      UPLL_LOG_DEBUG("GetNextRecord failed");
      DELETE_IF_NOT_NULL(vtermif_ffe_ckv);
      dmi->CloseCursor(dal_cursor_handle, false);
      return result_code;
    }
  }
  if (result_code != UPLL_RC_SUCCESS &&
      result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    UPLL_LOG_DEBUG("ExecuteAppQueryMultipleRecords failed");
    DELETE_IF_NOT_NULL(vtermif_ffe_ckv);
    dmi->CloseCursor(dal_cursor_handle, false);
    return result_code;
  }
  DELETE_IF_NOT_NULL(vtermif_ffe_ckv);
  /*
  DbSubOp dbop = { kOpReadMultiple, kOpMatchNone,
    kOpInOutCtrlr|kOpInOutDomain|kOpInOutFlag };
  result_code = ReadConfigDB(temp_okey, UPLL_DT_CANDIDATE,
                             UNC_OP_READ, dbop, dmi, MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
      UPLL_LOG_DEBUG("Instance does not exist!");
      DELETE_IF_NOT_NULL(temp_okey);
      return UPLL_RC_SUCCESS;
    }
    UPLL_LOG_ERROR("Unable to read configuration from CandidateDb");
    DELETE_IF_NOT_NULL(temp_okey);
    return result_code;
  }
  okey = temp_okey;
  while (NULL != okey) {
    GET_USER_DATA_CTRLR(okey, ctrlr_id);
    val_flowfilter_entry_t *flowfilter_val =
             reinterpret_cast<val_flowfilter_entry_t *> (GetVal(okey));
    if (flowfilter_val->valid[UPLL_IDX_FLOWLIST_NAME_FFE] == UNC_VF_VALID) {
      uint8_t flag_port_map = 0;
      GET_USER_DATA_FLAGS(okey, flag_port_map);
      if (flag_port_map & SET_FLAG_PORTMAP) {
        FlowListMoMgr *mgr = reinterpret_cast<FlowListMoMgr *>
          (const_cast<MoManager *> (GetMoManager(UNC_KT_FLOWLIST)));
        std::string temp_vtn_name;
        if (TC_CONFIG_VTN == config_mode) {
          temp_vtn_name = vtn_name;
        } else {
          key_vterm_if_flowfilter_entry_t *temp_key = reinterpret_cast<
              key_vterm_if_flowfilter_entry_t *>(ikey->get_key());
          temp_vtn_name = reinterpret_cast<const char*>(
              temp_key->flowfilter_key.if_key.vterm_key.vtn_key.vtn_name);
        }
        result_code = mgr->AddFlowListToController(
            reinterpret_cast<char*>(flowfilter_val->flowlist_name), dmi,
            reinterpret_cast<char *>(ctrlr_id), dt_type, UNC_OP_DELETE,
            config_mode, temp_vtn_name);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_ERROR(
              "Send delete request to flowlist failed. err code(%d)",
              result_code);
          DELETE_IF_NOT_NULL(temp_okey);
          return result_code;
        }
      }
    }
    result_code = UpdateConfigDB(okey, dt_type, UNC_OP_DELETE, dmi,
                               config_mode, vtn_name, MAINTBL);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_ERROR("UpdateConfigDB failed %d", result_code);
      DELETE_IF_NOT_NULL(temp_okey);
      return result_code;
    }
    okey = okey->get_next_cfg_key_val();
  }*/
  ConfigKeyVal *temp_okey = NULL;
  result_code = GetChildConfigKey(temp_okey, ikey);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_ERROR("GetChildConfigKey failed %d", result_code);
    return result_code;
  }
  result_code = UpdateConfigDB(temp_okey, dt_type, UNC_OP_DELETE, dmi,
                             config_mode, vtn_name, MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_ERROR("UpdateConfigDB failed %d", result_code);
    dmi->CloseCursor(dal_cursor_handle, false);
    DELETE_IF_NOT_NULL(temp_okey);
    return result_code;
  }
  dmi->CloseCursor(dal_cursor_handle, false);
  DELETE_IF_NOT_NULL(temp_okey);
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtermIfFlowFilterEntryMoMgr::SetValidAudit(ConfigKeyVal *&ikey) {
  UPLL_FUNC_TRACE;
  val_flowfilter_entry_t *val = reinterpret_cast
      <val_flowfilter_entry_t *>(GetVal(ikey));
  if (NULL == val) {
    UPLL_LOG_ERROR("val is NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  for (unsigned int loop = 0;
        loop < sizeof(val->valid) / sizeof(val->valid[0]);
        ++loop) {
    if (val->valid[loop] == UNC_VF_VALID) {
      val->cs_attr[loop] = UNC_CS_APPLIED;
    } else if (val->valid[loop] == UNC_VF_INVALID) {
      val->cs_attr[loop] = UNC_CS_NOT_APPLIED;
    }
  }
  val->cs_row_status = UNC_CS_APPLIED;
  return UPLL_RC_SUCCESS;
}

bool VtermIfFlowFilterEntryMoMgr::FilterAttributes(void *&val1,
                                          void *val2,
                                          bool copy_to_running,
                                          unc_keytype_operation_t op) {
  UPLL_FUNC_TRACE;
  if (op != UNC_OP_CREATE)
    return CompareValidValue(val1, val2, copy_to_running);
  return false;
}

upll_rc_t VtermIfFlowFilterEntryMoMgr::UpdateConfigStatus(ConfigKeyVal *ikey,
                                       unc_keytype_operation_t op,
                                       uint32_t driver_result,
                                       ConfigKeyVal *upd_key,
                                       DalDmlIntf *dmi,
                                       ConfigKeyVal *ctrlr_key) {
  UPLL_FUNC_TRACE;
  ConfigKeyVal * vbr_ffe_run_key = NULL;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  val_flowfilter_entry_t *ffe_val = NULL, *val_main = NULL;

  unc_keytype_configstatus_t cs_status =
      (driver_result == UPLL_RC_SUCCESS) ? UNC_CS_APPLIED : UNC_CS_NOT_APPLIED;
  ffe_val = reinterpret_cast<val_flowfilter_entry_t *>(GetVal(ikey));
  if (ffe_val == NULL) return UPLL_RC_ERR_GENERIC;
  if (op == UNC_OP_CREATE) {
    ffe_val->cs_row_status = cs_status;
  } else if (op == UNC_OP_UPDATE) {
    result_code = GetChildConfigKey(vbr_ffe_run_key, ikey);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_ERROR("GetChildConfigKey is failed resultcode=%d",
                           result_code);
      return result_code;
    }
    DbSubOp dbop_maintbl = { kOpReadSingle, kOpMatchNone,
                                          kOpInOutFlag |kOpInOutCs };
    result_code = ReadConfigDB(vbr_ffe_run_key, UPLL_DT_RUNNING  ,
                                     UNC_OP_READ, dbop_maintbl, dmi, MAINTBL);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_ERROR("Unable to read configuration from RunningDb");
      DELETE_IF_NOT_NULL(vbr_ffe_run_key);
      return result_code;
    }
    val_main = reinterpret_cast
           <val_flowfilter_entry_t *>(GetVal(vbr_ffe_run_key));
    if (NULL == val_main) {
      UPLL_LOG_DEBUG("Val struct is NULL");
      DELETE_IF_NOT_NULL(vbr_ffe_run_key);
      return UPLL_RC_ERR_GENERIC;
    }
    for (unsigned int loop = 0; loop < sizeof(val_main->valid)/
           sizeof(val_main->valid[0]); ++loop) {
           ffe_val->cs_attr[loop] = val_main->cs_attr[loop];
    }
    void *ffeval = reinterpret_cast<void *>(ffe_val);
    CompareValidValue(ffeval, GetVal(upd_key), true);
  } else {
    UPLL_LOG_ERROR("Invalid operation");
    return UPLL_RC_ERR_GENERIC;
  }
  UPLL_LOG_TRACE("%s", (ikey->ToStrAll()).c_str());
  val_flowfilter_entry_t *ffe_val2 =
     reinterpret_cast<val_flowfilter_entry_t *>(GetVal(upd_key));
  if (UNC_OP_UPDATE == op) {
    UPLL_LOG_TRACE("%s", (upd_key->ToStrAll()).c_str());
    ffe_val->cs_row_status = ffe_val2->cs_row_status;
  }
  for (unsigned int loop = 0;
    loop < sizeof(ffe_val->valid) / sizeof(ffe_val->valid[0]); ++loop) {
    /* Setting CS to the not supported attributes*/
    if (UNC_VF_NOT_SUPPORTED == ffe_val->valid[loop]) {
        ffe_val->cs_attr[loop] = UNC_CS_NOT_SUPPORTED;
    } else if ((UNC_VF_VALID == ffe_val->valid[loop])
          || (UNC_VF_VALID_NO_VALUE == ffe_val->valid[loop])) {
        ffe_val->cs_attr[loop] = cs_status;
    } else if ((UNC_VF_INVALID == ffe_val->valid[loop]) &&
               (UNC_OP_CREATE == op)) {
        ffe_val->cs_attr[loop] = UNC_CS_NOT_APPLIED;
    } else if ((UNC_VF_INVALID == ffe_val->valid[loop]) &&
               (UNC_OP_UPDATE == op)) {
      if (val_main->valid[loop] == UNC_VF_VALID) {
        if (cs_status == UNC_CS_APPLIED) {
          ffe_val->cs_attr[loop] = cs_status;
        }
      }
    } else if ((UNC_VF_VALID == ffe_val->valid[loop]) &&
       (UNC_OP_UPDATE == op)) {
      if (cs_status == UNC_CS_APPLIED) {
        ffe_val->cs_attr[loop] = UNC_CS_APPLIED;
      }
    }
    if ((ffe_val->valid[loop] == UNC_VF_VALID_NO_VALUE)
       &&(UNC_OP_UPDATE == op)) {
      ffe_val->cs_attr[loop]  = UNC_CS_UNKNOWN;
    }
    UPLL_LOG_TRACE("Value : %d Cs : %d", loop, ffe_val->cs_attr[loop]);
  }
  DELETE_IF_NOT_NULL(vbr_ffe_run_key);
  return result_code;
}

upll_rc_t VtermIfFlowFilterEntryMoMgr::SetRenameFlag(ConfigKeyVal *ikey,
    DalDmlIntf *dmi,
    IpcReqRespHeader *req) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  val_flowfilter_entry_t *val_ffe = reinterpret_cast
    <val_flowfilter_entry_t *>(GetVal(ikey));
  if (!val_ffe) {
    UPLL_LOG_ERROR("Val is NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  ConfigKeyVal *pkey = NULL;
  if (UNC_OP_CREATE == req->operation) {
    result_code = GetParentConfigKey(pkey, ikey);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_ERROR("GetParentConfigKey failed %d", result_code);
      return result_code;
    }
    MoMgrImpl *mgr =
      reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>(GetMoManager(
              UNC_KT_VTERMIF_FLOWFILTER)));
    if (!mgr) {
      UPLL_LOG_ERROR("mgr is NULL");
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
    if ((UNC_VF_VALID == val_ffe->valid[UPLL_IDX_FLOWLIST_NAME_FFE]) &&
        ((UNC_OP_CREATE == req->operation))) {
      ConfigKeyVal *fl_ckv = NULL;
      result_code = GetFlowlistConfigKey(reinterpret_cast<const char *>
          (val_ffe->flowlist_name), fl_ckv, dmi);
      if (UPLL_RC_SUCCESS != result_code) {
        UPLL_LOG_ERROR("GetFlowlistConfigKey failed %d", result_code);
        return result_code;
      }
      MoMgrImpl *fl_mgr =
      reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>(GetMoManager(
              UNC_KT_FLOWLIST)));
      if (!fl_mgr) {
        UPLL_LOG_ERROR("fl_mgr is NULL");
        DELETE_IF_NOT_NULL(fl_ckv);
        return UPLL_RC_ERR_GENERIC;
      }
      uint8_t fl_rename = 0;
      result_code = fl_mgr->IsRenamed(fl_ckv, req->datatype, dmi, fl_rename);
      if (UPLL_RC_SUCCESS != result_code) {
        UPLL_LOG_ERROR("IsRenamed failed %d", result_code);
        DELETE_IF_NOT_NULL(fl_ckv);
        return result_code;
      }
      if (fl_rename & 0x01) {
        rename |= FLOW_RENAME;
      }
      DELETE_IF_NOT_NULL(fl_ckv);
    }
    SET_USER_DATA_FLAGS(ikey, rename);
  } else if (UNC_OP_UPDATE == req->operation) {
    uint8_t rename = 0;
    ConfigKeyVal *dup_ckv = NULL;
    result_code = GetChildConfigKey(dup_ckv, ikey);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_ERROR(" GetChildConfigKey failed");
      return result_code;
    }
    DbSubOp dbop1 = {kOpReadSingle, kOpMatchNone, kOpInOutFlag};
    result_code = ReadConfigDB(dup_ckv, req->datatype, UNC_OP_READ,
                                     dbop1, dmi, MAINTBL);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_ERROR("ReadConfigDB failed %d", result_code);
      DELETE_IF_NOT_NULL(dup_ckv);
      return result_code;
    }
    GET_USER_DATA_FLAGS(dup_ckv, rename);
    DELETE_IF_NOT_NULL(dup_ckv);
    if (UNC_VF_VALID == val_ffe->valid[UPLL_IDX_FLOWLIST_NAME_FFE]) {
      ConfigKeyVal *fl_ckv = NULL;
      result_code = GetFlowlistConfigKey(reinterpret_cast<const char *>
          (val_ffe->flowlist_name), fl_ckv, dmi);
      if (UPLL_RC_SUCCESS != result_code) {
        UPLL_LOG_ERROR("GetFlowlistConfigKey failed %d", result_code);
        return result_code;
      }
      MoMgrImpl *fl_mgr =
      reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>(GetMoManager(
              UNC_KT_FLOWLIST)));
      if (!fl_mgr) {
        UPLL_LOG_ERROR("fl_mgr is NULL");
        DELETE_IF_NOT_NULL(fl_ckv);
        return UPLL_RC_ERR_GENERIC;
      }
      uint8_t fl_rename = 0;
      result_code = fl_mgr->IsRenamed(fl_ckv, req->datatype, dmi, fl_rename);
      if (UPLL_RC_SUCCESS != result_code) {
        UPLL_LOG_ERROR("IsRenamed failed %d", result_code);
        DELETE_IF_NOT_NULL(fl_ckv);
        return result_code;
      }
      UPLL_LOG_DEBUG("IsRenamed rename value : %d", rename);
      if (fl_rename & 0x01) {
        rename |= FLOW_RENAME;
        UPLL_LOG_ERROR("rename value after flowlist : %d", rename);
      } else {
        rename &= NO_FLOWLIST_RENAME;
        /* reset flag*/
      }
      DELETE_IF_NOT_NULL(fl_ckv);
    } else if (UNC_VF_VALID_NO_VALUE == val_ffe->valid
               [UPLL_IDX_FLOWLIST_NAME_FFE]) {
       rename &= NO_FLOWLIST_RENAME;
    }
    SET_USER_DATA_FLAGS(ikey, rename);
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtermIfFlowFilterEntryMoMgr::GetFlowlistConfigKey(
        const char *flowlist_name, ConfigKeyVal *&okey,
        DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_ERR_GENERIC;
  MoMgrImpl *mgr =
      reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>(GetMoManager(
              UNC_KT_FLOWLIST)));
  if (!mgr) return result_code;
  result_code = mgr->GetChildConfigKey(okey, NULL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_ERROR("GetChildConfigKey failed %d", result_code);
    return result_code;
  }
  key_flowlist_t *okey_key = reinterpret_cast<key_flowlist_t *>
      (okey->get_key());
  uuu::upll_strncpy(okey_key->flowlist_name,
        flowlist_name,
        (kMaxLenFlowListName+1));
  return UPLL_RC_SUCCESS;
}
upll_rc_t VtermIfFlowFilterEntryMoMgr::TxUpdateErrorHandler(
    ConfigKeyVal *ckv_unc,
    ConfigKeyVal *ckv_driver,
    DalDmlIntf *dmi,
    upll_keytype_datatype_t dt_type,
    ConfigKeyVal **err_ckv,
    IpcResponse *ipc_resp) {
  UPLL_FUNC_TRACE;
  *err_ckv = ckv_unc;
  DELETE_IF_NOT_NULL(ckv_driver);
  DELETE_IF_NOT_NULL(ipc_resp->ckv_data);
  return UPLL_RC_SUCCESS;
}
}  // namespace kt_momgr
}  // namespace upll
}  // namespace unc
