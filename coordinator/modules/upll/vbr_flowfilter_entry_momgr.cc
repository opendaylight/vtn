/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include "vbr_flowfilter_entry_momgr.hh"
#include "vbr_if_flowfilter_entry_momgr.hh"
#include "flowlist_momgr.hh"
#include "flowlist_entry_momgr.hh"
#include "vbr_momgr.hh"
#include "vbr_if_momgr.hh"
#include "upll_validation.hh"
#include "uncxx/upll_log.hh"
using unc::upll::ipc_util::IpcUtil;
namespace unc {
namespace upll {
namespace kt_momgr {

#define VTN_RENAME_FLAG         0x01
#define VBR_RENAME_FLAG         0x02
#define NO_VBR_RENAME_FLAG      ~VBR_RENAME_FLAG
#define FLOWLIST_RENAME_FLAG    0x04

#define FLOW_RENAME             0x04
#define NO_FLOWLIST_RENAME      ~FLOWLIST_RENAME_FLAG
// VbrFlowFilterEntry Table(Main Table)
BindInfo VbrFlowFilterEntryMoMgr::vbr_flowfilterentry_bind_info[] = {
  { uudst::vbr_flowfilter_entry::kDbiVtnName, CFG_KEY,
    offsetof(key_vbr_flowfilter_entry_t,
             flowfilter_key.vbr_key.vtn_key.vtn_name),
    uud::kDalChar, (kMaxLenVtnName + 1) },
  { uudst::vbr_flowfilter_entry::kDbiVbrName, CFG_KEY,
    offsetof(key_vbr_flowfilter_entry_t,
             flowfilter_key.vbr_key.vbridge_name),
    uud::kDalChar, (kMaxLenVnodeName + 1) },
  { uudst::vbr_flowfilter_entry::kDbiInputDirection, CFG_KEY,
    offsetof(key_vbr_flowfilter_entry_t, flowfilter_key.direction),
    uud::kDalUint8, 1 },
  { uudst::vbr_flowfilter_entry::kDbiSequenceNum, CFG_KEY,
    offsetof(key_vbr_flowfilter_entry_t, sequence_num),
    uud::kDalUint16, 1 },
  { uudst::vbr_flowfilter_entry::kDbiCtrlrName, CK_VAL,
    offsetof(key_user_data_t, ctrlr_id),
    uud::kDalChar, (kMaxLenCtrlrId + 1) },
  { uudst::vbr_flowfilter_entry::kDbiDomainId, CK_VAL,
    offsetof(key_user_data_t, domain_id),
    uud::kDalChar, (kMaxLenDomainId + 1) },
  { uudst::vbr_flowfilter_entry::kDbiFlowlistName, CFG_VAL,
    offsetof(val_flowfilter_entry_t, flowlist_name),
    uud::kDalChar, (kMaxLenFlowListName + 1) },
  { uudst::vbr_flowfilter_entry::kDbiAction, CFG_VAL,
    offsetof(val_flowfilter_entry_t, action),
    uud::kDalUint8, 1 },
  { uudst::vbr_flowfilter_entry::kDbiRedirectNode, CFG_VAL,
    offsetof(val_flowfilter_entry_t, redirect_node),
    uud::kDalChar, (kMaxLenVnodeName + 1) },
  { uudst::vbr_flowfilter_entry::kDbiRedirectPort, CFG_VAL,
    offsetof(val_flowfilter_entry_t, redirect_port),
    uud::kDalChar, (kMaxLenInterfaceName + 1) },
  // Adding Redirection
  { uudst::vbr_flowfilter_entry::kDbiRedirectDirection, CFG_VAL,
    offsetof(val_flowfilter_entry_t, redirect_direction),
    uud::kDalUint8 , 1 },
  // end
  { uudst::vbr_flowfilter_entry::kDbiModifyDstMac, CFG_VAL,
    offsetof(val_flowfilter_entry_t, modify_dstmac),
    uud::kDalChar, 6},
  { uudst::vbr_flowfilter_entry::kDbiModifySrcMac, CFG_VAL,
    offsetof(val_flowfilter_entry_t, modify_srcmac),
    uud::kDalChar, 6},
  { uudst::vbr_flowfilter_entry::kDbiNwmName, CFG_VAL,
    offsetof(val_flowfilter_entry_t, nwm_name),
    uud::kDalChar, (kMaxLenNwmName + 1) },
  { uudst::vbr_flowfilter_entry::kDbiDscp, CFG_VAL,
    offsetof(val_flowfilter_entry_t, dscp),
    uud::kDalUint8, 1 },
  { uudst::vbr_flowfilter_entry::kDbiPriority, CFG_VAL,
    offsetof(val_flowfilter_entry_t, priority),
    uud::kDalUint8, 1 },
  { uudst::vbr_flowfilter_entry::kDbiFlags, CK_VAL,
    offsetof(key_user_data_t, flags),
    uud::kDalUint8, 1 },
  { uudst::vbr_flowfilter_entry::kDbiValidFlowlistName, CFG_META_VAL,
    offsetof(val_flowfilter_entry_t, valid[0]),
    uud::kDalUint8, 1 },
  { uudst::vbr_flowfilter_entry::kDbiValidAction, CFG_META_VAL,
    offsetof(val_flowfilter_entry_t, valid[1]),
    uud::kDalUint8, 1 },
  { uudst::vbr_flowfilter_entry::kDbiValidRedirectNode, CFG_META_VAL,
    offsetof(val_flowfilter_entry_t, valid[2]),
    uud::kDalUint8, 1 },
  { uudst::vbr_flowfilter_entry::kDbiValidRedirectPort, CFG_META_VAL,
    offsetof(val_flowfilter_entry_t, valid[3]),
    uud::kDalUint8, 1 },
  // #if
  { uudst::vbr_flowfilter_entry::kDbiValidRedirectDirection, CFG_META_VAL,
    offsetof(val_flowfilter_entry_t, valid[4]),
    uud::kDalUint8, 1 },
  // en
  { uudst::vbr_flowfilter_entry::kDbiValidModifyDstMac, CFG_META_VAL,
    offsetof(val_flowfilter_entry_t, valid[5]),
    uud::kDalUint8, 1 },
  { uudst::vbr_flowfilter_entry::kDbiValidModifySrcMac, CFG_META_VAL,
    offsetof(val_flowfilter_entry_t, valid[6]),
    uud::kDalUint8, 1 },
  { uudst::vbr_flowfilter_entry::kDbiValidNwmName, CFG_META_VAL,
    offsetof(val_flowfilter_entry_t, valid[7]),
    uud::kDalUint8, 1 },
  { uudst::vbr_flowfilter_entry::kDbiValidDscp, CFG_META_VAL,
    offsetof(val_flowfilter_entry_t, valid[8]),
    uud::kDalUint8, 1 },
  { uudst::vbr_flowfilter_entry::kDbiValidPriority, CFG_META_VAL,
    offsetof(val_flowfilter_entry_t, valid[9]),
    uud::kDalUint8, 1 },
  { uudst::vbr_flowfilter_entry::kDbiCsRowStatus, CS_VAL,
    offsetof(val_flowfilter_entry_t, cs_row_status),
    uud::kDalUint8, 1 },
  { uudst::vbr_flowfilter_entry::kDbiCsFlowlistName, CS_VAL,
    offsetof(val_flowfilter_entry_t, cs_attr[0]),
    uud::kDalUint8, 1 },
  { uudst::vbr_flowfilter_entry::kDbiCsAction, CS_VAL,
    offsetof(val_flowfilter_entry_t, cs_attr[1]),
    uud::kDalUint8, 1 },
  { uudst::vbr_flowfilter_entry::kDbiCsRedirectNode, CS_VAL,
    offsetof(val_flowfilter_entry_t, cs_attr[2]),
    uud::kDalUint8, 1 },
  { uudst::vbr_flowfilter_entry::kDbiCsRedirectPort, CS_VAL,
    offsetof(val_flowfilter_entry_t, cs_attr[3]),
    uud::kDalUint8, 1 },
  // #if
  { uudst::vbr_flowfilter_entry::kDbiCsRedirectDirection, CS_VAL,
    offsetof(val_flowfilter_entry_t, cs_attr[4]),
    uud::kDalUint8, 1 },
  // end

  { uudst::vbr_flowfilter_entry::kDbiCsModifyDstMac, CS_VAL,
    offsetof(val_flowfilter_entry_t, cs_attr[5]),
    uud::kDalUint8, 1 },
  { uudst::vbr_flowfilter_entry::kDbiCsModifySrcMac, CS_VAL,
    offsetof(val_flowfilter_entry_t, cs_attr[6]),
    uud::kDalUint8, 1 },
  { uudst::vbr_flowfilter_entry::kDbiCsNwmName, CS_VAL,
    offsetof(val_flowfilter_entry_t, cs_attr[7]),
    uud::kDalUint8, 1 },
  { uudst::vbr_flowfilter_entry::kDbiCsDscp, CS_VAL,
    offsetof(val_flowfilter_entry_t, cs_attr[8]),
    uud::kDalUint8, 1 },
  { uudst::vbr_flowfilter_entry::kDbiCsPriority, CS_VAL,
    offsetof(val_flowfilter_entry_t, cs_attr[9]),
    uud::kDalUint8, 1 }
};

BindInfo VbrFlowFilterEntryMoMgr::vbr_flowfilter_entry_maintbl_bind_info[] = {
  { uudst::vbr_flowfilter_entry::kDbiVtnName, CFG_MATCH_KEY, offsetof(
     key_vbr_flowfilter_entry_t, flowfilter_key.vbr_key.vtn_key.vtn_name),
     uud::kDalChar, kMaxLenVtnName + 1 },
  { uudst::vbr_flowfilter_entry::kDbiVbrName, CFG_MATCH_KEY, offsetof(
     key_vbr_flowfilter_entry_t, flowfilter_key.vbr_key.vbridge_name),
     uud::kDalChar, kMaxLenVnodeName + 1 },
  { uudst::vbr_flowfilter_entry::kDbiVtnName, CFG_INPUT_KEY, offsetof(
     key_rename_vnode_info_t, new_unc_vtn_name),
     uud::kDalChar, kMaxLenVtnName + 1 },
  { uudst::vbr_flowfilter_entry::kDbiVbrName, CFG_INPUT_KEY, offsetof(
     key_rename_vnode_info_t, new_unc_vnode_name),
     uud::kDalChar, kMaxLenVnodeName + 1 },
  { uudst::vbr_flowfilter_entry::kDbiFlags, CK_VAL, offsetof(
     key_user_data_t, flags),
     uud::kDalUint8, 1 }
};

VbrFlowFilterEntryMoMgr::VbrFlowFilterEntryMoMgr() : MoMgrImpl() {
  UPLL_FUNC_TRACE;
  // Rename and ctrlr tables not required for this KT
  // setting max tables toto 1
  ntable = (MAX_MOMGR_TBLS);
  table = new Table *[ntable]();

  // For Main Table
  table[MAINTBL] = new Table(uudst::kDbiVbrFlowFilterEntryTbl,
      UNC_KT_VBR_FLOWFILTER_ENTRY, vbr_flowfilterentry_bind_info,
      IpctSt::kIpcStKeyVbrFlowfilterEntry, IpctSt::kIpcStValFlowfilterEntry,
      uudst::vbr_flowfilter_entry::kDbiVbrFlowFilterEntryNumCols);

  table[RENAMETBL] = NULL;

  table[CTRLRTBL] = NULL;

  table[CONVERTTBL] = NULL;
  nchild = 0;
  child = NULL;
}

bool VbrFlowFilterEntryMoMgr::GetRenameKeyBindInfo(unc_key_type_t key_type,
                                    BindInfo *&binfo,
                                    int &nattr,
                                    MoMgrTables tbl ) {
  UPLL_FUNC_TRACE;

  /* Main Table only update */
  nattr = sizeof(vbr_flowfilter_entry_maintbl_bind_info)/
          sizeof(vbr_flowfilter_entry_maintbl_bind_info[0]);
  binfo = vbr_flowfilter_entry_maintbl_bind_info;

  UPLL_LOG_DEBUG("Successful Completeion");
  return PFC_TRUE;
}

bool VbrFlowFilterEntryMoMgr::IsValidKey(void *key, uint64_t index,
                                         MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  upll_rc_t ret_val = UPLL_RC_SUCCESS;
  key_vbr_flowfilter_entry_t  *ff_key =
      reinterpret_cast<key_vbr_flowfilter_entry_t *>(key);
  if (ff_key == NULL)
    return UPLL_RC_ERR_GENERIC;

  switch (index) {
    case uudst::vbr_flowfilter_entry::kDbiVtnName:
      ret_val = ValidateKey(
          reinterpret_cast<char *>
          (ff_key->flowfilter_key.vbr_key.vtn_key.vtn_name),
          kMinLenVtnName, kMaxLenVtnName);
      if (ret_val != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("VTN Name is not valid(%d)", ret_val);
        return false;
      }
      break;
    case uudst::vbr_flowfilter_entry::kDbiVbrName:
      ret_val = ValidateKey(reinterpret_cast<char *>
                            (ff_key->flowfilter_key.vbr_key.vbridge_name),
                            kMinLenVnodeName, kMaxLenVnodeName);
      if (ret_val != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("VBR Name is not valid(%d)", ret_val);
        return false;
      }
      break;
    case uudst::vbr_flowfilter_entry::kDbiSequenceNum:
      if (!ValidateNumericRange(ff_key->sequence_num,
                                kMinFlowFilterSeqNum,
                                kMaxFlowFilterSeqNum,
                                true,
                                true)) {
        UPLL_LOG_DEBUG("sequence number syntax validation failed");
        return false;
      }
      break;
    case uudst::vbr_flowfilter_entry::kDbiInputDirection:
      if (ff_key->flowfilter_key.direction == 0xFE) {
        // if operation is read sibling begin or
        // read sibling count return false
        // for output binding
        ff_key->flowfilter_key.direction = 0;
        return false;
      } else if (!ValidateNumericRange(ff_key->flowfilter_key.direction,
                                (uint8_t) UPLL_FLOWFILTER_DIR_IN,
                                (uint8_t) UPLL_FLOWFILTER_DIR_OUT,
                                true, true)) {
        UPLL_LOG_DEBUG("direction syntax validation failed :");
        return UPLL_RC_ERR_CFG_SYNTAX;
      }
      break;
    default:
      UPLL_LOG_DEBUG("Invalid Key Index");
      return false;
  }
  return true;
}

upll_rc_t VbrFlowFilterEntryMoMgr::ValidateMessage(IpcReqRespHeader *req,
                                                   ConfigKeyVal *key) {
  UPLL_FUNC_TRACE;

  upll_rc_t rt_code = UPLL_RC_ERR_GENERIC;

  if ((NULL == req) || (NULL == key)) {
    UPLL_LOG_DEBUG("IpcReqRespHeader/ConfigKeyval is NULL");
    return UPLL_RC_ERR_BAD_REQUEST;
  }

  if (UNC_KT_VBR_FLOWFILTER_ENTRY != key->get_key_type()) {
    UPLL_LOG_DEBUG(" Invalid keytype(%d)", key->get_key_type());
    return UPLL_RC_ERR_BAD_REQUEST;
  }

  if (key->get_st_num() != IpctSt::kIpcStKeyVbrFlowfilterEntry) {
    UPLL_LOG_DEBUG("Invalid key structure received. received struct num - %d",
                   key->get_st_num());
    return UPLL_RC_ERR_BAD_REQUEST;
  }
  if (req->option2 != UNC_OPT2_NONE) {
    UPLL_LOG_DEBUG(" Error: option2 is not NONE");
    return UPLL_RC_ERR_INVALID_OPTION2;
  }
  if ((req->option1 != UNC_OPT1_NORMAL)
              &&(req->option1 != UNC_OPT1_DETAIL)) {
     UPLL_LOG_DEBUG(" Error: option1 is not NORMAL");
     return UPLL_RC_ERR_INVALID_OPTION1;
  }
  if ((req->option1 != UNC_OPT1_NORMAL)
              &&(req->operation == UNC_OP_READ_SIBLING_COUNT)) {
     UPLL_LOG_DEBUG(" Error: option1 is not NORMAL for ReadSiblingCount");
     return UPLL_RC_ERR_INVALID_OPTION1;
  }
  if ((req->option1 == UNC_OPT1_DETAIL) &&
      (req->datatype != UPLL_DT_STATE)) {
      UPLL_LOG_DEBUG(" Invalid Datatype(%d)", req->datatype);
      return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT;
  }
  if ((req->datatype == UPLL_DT_IMPORT) && (req->operation == UNC_OP_READ ||
       req->operation == UNC_OP_READ_SIBLING ||
       req->operation == UNC_OP_READ_SIBLING_BEGIN ||
       req->operation == UNC_OP_READ_NEXT ||
       req->operation == UNC_OP_READ_BULK ||
       req->operation == UNC_OP_READ_SIBLING_COUNT)) {
    return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT;
  }

  /** Read key & value structure */
  key_vbr_flowfilter_entry_t *key_vbr_flowfilter_entry =
      reinterpret_cast<key_vbr_flowfilter_entry_t *>(key->get_key());

  /** Validate keyStruct fields*/
  if (NULL == key_vbr_flowfilter_entry) {
    UPLL_LOG_DEBUG("KT_VBR_FLOWFILTER_ENTRY Key structure is empty!!");
    return UPLL_RC_ERR_BAD_REQUEST;
  }

  rt_code = ValidateVbrFlowfilterEntryKey(key_vbr_flowfilter_entry,
                                          req->operation);
  if (UPLL_RC_SUCCESS != rt_code) {
    UPLL_LOG_DEBUG("VBR Flowfilter Entry key syntax validation failed "
                   ":Err Code - %d",
                   rt_code);
    return rt_code;
  }

  /** UpdateMo invokes valFlowfilterEntry */
  if (req->operation == UNC_OP_UPDATE)
    return UPLL_RC_SUCCESS;

  return ValidateValFlowfilterEntry(req, key);
}

upll_rc_t VbrFlowFilterEntryMoMgr::ValidateValFlowfilterEntry(
    IpcReqRespHeader *req, ConfigKeyVal *key) {
  UPLL_FUNC_TRACE;

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
  return ValidateFlowfilterEntryValue(val_flowfilter_entry, req->operation);
}

upll_rc_t VbrFlowFilterEntryMoMgr::ValidateFlowfilterEntryValue(
    val_flowfilter_entry_t *val_flowfilter_entry, uint32_t operation) {
  UPLL_FUNC_TRACE;

  upll_rc_t rt_code = UPLL_RC_ERR_GENERIC;

  /**validate flowlist name */
  if (val_flowfilter_entry->valid[UPLL_IDX_FLOWLIST_NAME_FFE]
      == UNC_VF_VALID) {
    rt_code = ValidateKey(reinterpret_cast<char *>
                (val_flowfilter_entry->flowlist_name),
                      kMinLenFlowListName,
                      kMaxLenFlowListName);
    if (rt_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("FlowList name syntax check failed."
                "Received Flowlist name - %s",
                val_flowfilter_entry->flowlist_name);
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  } else if ((operation == UNC_OP_UPDATE)
      && (val_flowfilter_entry->valid[UPLL_IDX_FLOWLIST_NAME_FFE]
          == UNC_VF_VALID_NO_VALUE)) {
    UPLL_LOG_DEBUG("Reset flowlist name");
    memset(val_flowfilter_entry->flowlist_name, 0,
           sizeof(val_flowfilter_entry->flowlist_name));
  }

  /** validates action and checks whether REDIRECT & modify mac configured only
   * when action == REDIRECT */
  if ((rt_code = ValidateFlowfilterEntryAction(val_flowfilter_entry,
                                               operation))
      != UPLL_RC_SUCCESS) {
    return rt_code;
  }
  /** validate redirect_node/redirect_port because action == REDIRECT */
  if ((rt_code = ValidateRedirectField(val_flowfilter_entry, operation))
      != UPLL_RC_SUCCESS) {
    return rt_code;
  }

  /** validate modify_dstmac */
  /** no need to do specific mac address check, since it accepts valid range*/
  if ((operation == UNC_OP_UPDATE)
      && (val_flowfilter_entry->valid[UPLL_IDX_MODIFY_DST_MAC_FFE]
          == UNC_VF_VALID_NO_VALUE)) {
    UPLL_LOG_DEBUG("Reset modify_dstmac");
    memset(val_flowfilter_entry->modify_dstmac, 0,
           sizeof(val_flowfilter_entry->modify_dstmac));
  }

  /** validate modify_srcmac */
  if ((operation == UNC_OP_UPDATE)
      && (val_flowfilter_entry->valid[UPLL_IDX_MODIFY_SRC_MAC_FFE]
          == UNC_VF_VALID_NO_VALUE)) {
    UPLL_LOG_DEBUG("Reset modify_srcmac");
    memset(val_flowfilter_entry->modify_srcmac, 0,
           sizeof(val_flowfilter_entry->modify_srcmac));
  }

  /** validate nwm_name */
  if (val_flowfilter_entry->valid[UPLL_IDX_NWM_NAME_FFE] == UNC_VF_VALID) {
    rt_code = ValidateKey(reinterpret_cast<char *>
                  (val_flowfilter_entry->nwm_name),
                      kMinLenNwmName,
                      kMaxLenNwmName);
    if (rt_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Nwm name syntax check failed."
                "Received Nwm name - %s",
                val_flowfilter_entry->nwm_name);
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  } else if ((operation == UNC_OP_UPDATE)
      && (val_flowfilter_entry->valid[UPLL_IDX_NWM_NAME_FFE]
          == UNC_VF_VALID_NO_VALUE)) {
    UPLL_LOG_DEBUG("Reset Network monitor name");
    memset(val_flowfilter_entry->nwm_name, 0,
           sizeof(val_flowfilter_entry->nwm_name));
  }

  /** validate DSCP */
  /* In case of update if action is not configured, check DB. if
     action is not exists in DB also then send error */
  if (val_flowfilter_entry->valid[UPLL_IDX_DSCP_FFE] == UNC_VF_VALID) {
    if (!ValidateNumericRange(val_flowfilter_entry->dscp, kMinIPDscp,
                            kMaxIPDscp, true, true)) {
      UPLL_LOG_DEBUG("DSCP syntax validation failed");
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  } else if ((operation == UNC_OP_UPDATE)
      && (val_flowfilter_entry->valid[UPLL_IDX_DSCP_FFE]
          == UNC_VF_VALID_NO_VALUE)) {
    UPLL_LOG_DEBUG("Reset DSCP");
    val_flowfilter_entry->dscp = 0;
  }

  /** validate Priority*/
  if (val_flowfilter_entry->valid[UPLL_IDX_PRIORITY_FFE] == UNC_VF_VALID) {
    if (!ValidateNumericRange(val_flowfilter_entry->priority,
                              kMinVlanPriority, kMaxVlanPriority, true,
                              true)) {
      UPLL_LOG_DEBUG("Priority syntax validation failed :Err Code - %d",
                    rt_code);
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  } else if ((operation == UNC_OP_UPDATE)
      && (val_flowfilter_entry->valid[UPLL_IDX_PRIORITY_FFE]
          == UNC_VF_VALID_NO_VALUE)) {
    UPLL_LOG_DEBUG("Reset Priority");
    val_flowfilter_entry->priority = 0;
  }

  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrFlowFilterEntryMoMgr::ValidateVbrFlowfilterEntryKey(
    key_vbr_flowfilter_entry_t *key_vbr_flowfilter_entry,
    unc_keytype_operation_t operation) {
  upll_rc_t rt_code = UPLL_RC_ERR_GENERIC;

  /** validate key_vbr_flowfilter */
  /** validate vbr_key */
  VbrMoMgr *mgrvbr = reinterpret_cast<VbrMoMgr *>(
     const_cast<MoManager*>(GetMoManager(UNC_KT_VBRIDGE)));

  if (NULL == mgrvbr) {
    UPLL_LOG_DEBUG("unable to get VbrMoMgr object to validate key_vbridge");
    return UPLL_RC_ERR_GENERIC;
  }

  rt_code = mgrvbr->ValidateVbrKey(
      &(key_vbr_flowfilter_entry->flowfilter_key.vbr_key));
  if (UPLL_RC_SUCCESS != rt_code) {
    UPLL_LOG_DEBUG(" Vbr name syntax validation failed :"
                  "Err Code - %d",
                  rt_code);
    return rt_code;
  }

  /** Validate Direction */
  if (!ValidateNumericRange(key_vbr_flowfilter_entry->flowfilter_key.direction,
                            (uint8_t) UPLL_FLOWFILTER_DIR_IN,
                            (uint8_t) UPLL_FLOWFILTER_DIR_OUT, true, false)) {
    UPLL_LOG_DEBUG("direction syntax validation failed :");
    return UPLL_RC_ERR_CFG_SYNTAX;
  }

  if ((operation != UNC_OP_READ_SIBLING_COUNT) &&
      (operation != UNC_OP_READ_SIBLING_BEGIN)) {
  /** Validate Seq_num */
  if (!ValidateNumericRange(key_vbr_flowfilter_entry->sequence_num,
                            kMinFlowFilterSeqNum, kMaxFlowFilterSeqNum, true,
                            true)) {
    UPLL_LOG_DEBUG("sequence number syntax validation failed");
    return UPLL_RC_ERR_CFG_SYNTAX;
  }
  } else {
    key_vbr_flowfilter_entry->sequence_num = 0;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrFlowFilterEntryMoMgr::ValidateRedirectField(
    val_flowfilter_entry_t *val_flowfilter_entry, uint32_t operation) {

  UPLL_FUNC_TRACE;
  upll_rc_t rt_code = UPLL_RC_ERR_GENERIC;

  /** validate redirect_direction */
  if ((val_flowfilter_entry->valid[UPLL_IDX_REDIRECT_DIRECTION_FFE]
     == UNC_VF_VALID)) {
    if ((val_flowfilter_entry->redirect_direction
               != UPLL_FLOWFILTER_DIR_IN &&
             val_flowfilter_entry->redirect_direction
                       != UPLL_FLOWFILTER_DIR_OUT) ) {
      UPLL_LOG_DEBUG("redirect_direction: direction is invalid");
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  }
  /** Redirection can be configured using redirect_node, redirect_port,
    * redirect_direction, modify_dstmac and modify_srcmac attributes.
    *
    * redirect_node, redirect_port and redirect_direction attributes
    * are mandatory and should be specified together with same valid flags.
    **/
  if (((val_flowfilter_entry->valid[UPLL_IDX_REDIRECT_NODE_FFE]) !=
      (val_flowfilter_entry->valid[UPLL_IDX_REDIRECT_PORT_FFE])) ||
      ((val_flowfilter_entry->valid[UPLL_IDX_REDIRECT_PORT_FFE]) !=
      (val_flowfilter_entry->valid[UPLL_IDX_REDIRECT_DIRECTION_FFE]))) {
    return UPLL_RC_ERR_CFG_SYNTAX;
  }
  /** validate redirect_node */
  if (val_flowfilter_entry->valid[UPLL_IDX_REDIRECT_NODE_FFE] == UNC_VF_VALID) {
    rt_code = ValidateKey(
        reinterpret_cast<char*>(val_flowfilter_entry->redirect_node),
        kMinLenVnodeName, kMaxLenVnodeName);

    if (UPLL_RC_SUCCESS != rt_code) {
      UPLL_LOG_DEBUG("redirect_node syntax validation failed :Err Code - %d",
                    rt_code);
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  } else if ((operation == UNC_OP_UPDATE)
      && (val_flowfilter_entry->valid[UPLL_IDX_REDIRECT_NODE_FFE]
          == UNC_VF_VALID_NO_VALUE)) {
    UPLL_LOG_DEBUG("Reset redirect_node");

    memset(val_flowfilter_entry->redirect_node, 0,
           sizeof(val_flowfilter_entry->redirect_node));
  }

  /** validate redirect_port */
  if (val_flowfilter_entry->valid[UPLL_IDX_REDIRECT_PORT_FFE] == UNC_VF_VALID) {
    rt_code = ValidateKey(
        reinterpret_cast<char*>(val_flowfilter_entry->redirect_port),
        kMinLenInterfaceName, kMaxLenInterfaceName);

    if (UPLL_RC_SUCCESS != rt_code) {
      UPLL_LOG_DEBUG("redirect_port syntax validation failed :Err Code - %d",
                    rt_code);
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  } else if ((operation == UNC_OP_UPDATE)
      && (val_flowfilter_entry->valid[UPLL_IDX_REDIRECT_PORT_FFE]
          == UNC_VF_VALID_NO_VALUE)) {
    UPLL_LOG_DEBUG("Reset redirect_port");

    memset(val_flowfilter_entry->redirect_port, 0,
           sizeof(val_flowfilter_entry->redirect_port));
  }
  /* modify_dstmac and modify_srcmac attributes are optional and should be
   * specified along with redirect_node, redirect_port and redirect_direction
   * attributes.
   **/
  if ((val_flowfilter_entry->valid[UPLL_IDX_REDIRECT_PORT_FFE] ==
       UNC_VF_VALID_NO_VALUE) &&
      (val_flowfilter_entry->valid[UPLL_IDX_REDIRECT_NODE_FFE] ==
       UNC_VF_VALID_NO_VALUE)) {
    val_flowfilter_entry->valid[UPLL_IDX_REDIRECT_DIRECTION_FFE] =
    UNC_VF_VALID_NO_VALUE;
    val_flowfilter_entry->valid[UPLL_IDX_MODIFY_DST_MAC_FFE] =
    UNC_VF_VALID_NO_VALUE;
    val_flowfilter_entry->valid[UPLL_IDX_MODIFY_SRC_MAC_FFE] =
    UNC_VF_VALID_NO_VALUE;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrFlowFilterEntryMoMgr::ValidateFlowfilterEntryAction(
    val_flowfilter_entry_t *val_flowfilter_entry, uint32_t operation) {
  UPLL_FUNC_TRACE;

  /** validate action */
  if (val_flowfilter_entry->valid[UPLL_IDX_ACTION_FFE] == UNC_VF_VALID) {
    if (!ValidateNumericRange(val_flowfilter_entry->action,
                              (uint8_t) UPLL_FLOWFILTER_ACT_PASS,
                              (uint8_t) UPLL_FLOWFILTER_ACT_REDIRECT, true,
                              true)) {
      UPLL_LOG_DEBUG(" Action syntax validation failed ");
      return UPLL_RC_ERR_CFG_SYNTAX;
    }

  } else if ((operation == UNC_OP_UPDATE)
      && (val_flowfilter_entry->valid[UPLL_IDX_ACTION_FFE]
          == UNC_VF_VALID_NO_VALUE)) {
    UPLL_LOG_DEBUG("Reset Action ");
    val_flowfilter_entry->action = 0;
  }

#if 0  // action_is_redirect is set, but not used
  /** At the time of UPDATE if action is not set, then check DB,
   * db_up_action_valid flag is true when operation is update and valid flag for
   * action is INVALID, action_redirect is true when action is configured as
   * REDIRECT in DB */
  if (val_flowfilter_entry->valid[UPLL_IDX_ACTION_FFE] == UNC_VF_VALID) {
      if (val_flowfilter_entry->action == UPLL_FLOWFILTER_ACT_REDIRECT) {
         action_is_redirect = true;
      }
  } else if (db_up_action_valid) {
       if (action_redirect) {
         action_is_redirect = true;
       }
  }
#endif
  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrFlowFilterEntryMoMgr::ValidateCapability(IpcReqRespHeader *req,
                                                      ConfigKeyVal *ikey,
                                                      const char* ctrlr_name) {
  UPLL_FUNC_TRACE;
  // upll_rc_t result_code = UPLL_RC_ERR_GENERIC;

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
    UPLL_LOG_DEBUG("keytype(%d) is not supported by controller(%s)",
        ikey->get_key_type(), ctrlr_name);
    return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
  }

  val_flowfilter_entry_t *val_flowfilter_entry =
    reinterpret_cast<val_flowfilter_entry_t *>(GetVal(ikey));

  if (val_flowfilter_entry) {
    if (max_attrs > 0) {
      return ValFlowFilterEntryAttributeSupportCheck(val_flowfilter_entry,
                                                       attrs);
    } else {
      UPLL_LOG_DEBUG("Attribute list is empty for operation %d",
                     req->operation);
      return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
    }
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrFlowFilterEntryMoMgr::ValFlowFilterEntryAttributeSupportCheck(
  val_flowfilter_entry_t *val_flowfilter_entry, const uint8_t* attrs) {
  UPLL_FUNC_TRACE;

  if (val_flowfilter_entry != NULL) {
    if ((val_flowfilter_entry->valid[UPLL_IDX_FLOWLIST_NAME_FFE] ==
       UNC_VF_VALID)
        ||(val_flowfilter_entry->valid[UPLL_IDX_FLOWLIST_NAME_FFE]
            == UNC_VF_VALID_NO_VALUE)) {
      if (attrs[unc::capa::vbr_flowfilter_entry::kCapFlowlistName] == 0) {
        val_flowfilter_entry->valid[UPLL_IDX_FLOWLIST_NAME_FFE] =
            UNC_VF_NOT_SUPPORTED;
        UPLL_LOG_DEBUG("flowlist_name attr is not supported by ctrlr");
        return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
      }
    }

    if ((val_flowfilter_entry->valid[UPLL_IDX_ACTION_FFE] == UNC_VF_VALID)
        || (val_flowfilter_entry->valid[UPLL_IDX_ACTION_FFE]
            == UNC_VF_VALID_NO_VALUE)) {
      if (attrs[unc::capa::vbr_flowfilter_entry::kCapAction] == 0) {
        val_flowfilter_entry->valid[UPLL_IDX_ACTION_VFFE] =
            UNC_VF_NOT_SUPPORTED;

        UPLL_LOG_DEBUG("action attr is not supported by ctrlr");
        return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
      }
    }

    if ((val_flowfilter_entry->valid[UPLL_IDX_REDIRECT_NODE_FFE] ==
         UNC_VF_VALID)
        || (val_flowfilter_entry->valid[UPLL_IDX_REDIRECT_NODE_FFE]
            == UNC_VF_VALID_NO_VALUE)) {
      if (attrs[unc::capa::vbr_flowfilter_entry::kCapRedirectNode] == 0) {
        val_flowfilter_entry->valid[UPLL_IDX_REDIRECT_NODE_FFE] =
            UNC_VF_NOT_SUPPORTED;

        UPLL_LOG_DEBUG("redirect_node attr is not supported by ctrlr");
        return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
      }
    }

    if ((val_flowfilter_entry->valid[UPLL_IDX_REDIRECT_PORT_FFE] ==
         UNC_VF_VALID)
        || (val_flowfilter_entry->valid[UPLL_IDX_REDIRECT_PORT_FFE]
            == UNC_VF_VALID_NO_VALUE)) {
      if (attrs[unc::capa::vbr_flowfilter_entry::kCapRedirectPort] == 0) {
        val_flowfilter_entry->valid[UPLL_IDX_REDIRECT_PORT_FFE] =
            UNC_VF_NOT_SUPPORTED;

        UPLL_LOG_DEBUG("redirect_port attr is not supported by ctrlr");
        return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
      }
    }

    if ((val_flowfilter_entry->valid[UPLL_IDX_MODIFY_DST_MAC_FFE]
        == UNC_VF_VALID)
        || (val_flowfilter_entry->valid[UPLL_IDX_MODIFY_DST_MAC_FFE]
            == UNC_VF_VALID_NO_VALUE)) {
      if (attrs[unc::capa::vbr_flowfilter_entry::kCapModifyDstMac] == 0) {
        val_flowfilter_entry->valid[UPLL_IDX_MODIFY_DST_MAC_FFE] =
            UNC_VF_NOT_SUPPORTED;

        UPLL_LOG_DEBUG("modify_dst attr is not supported by ctrlr");
        return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
      }
    }

    if ((val_flowfilter_entry->valid[UPLL_IDX_MODIFY_SRC_MAC_FFE]
        == UNC_VF_VALID)
        || (val_flowfilter_entry->valid[UPLL_IDX_MODIFY_SRC_MAC_FFE]
            == UNC_VF_VALID_NO_VALUE)) {
      if (attrs[unc::capa::vbr_flowfilter_entry::kCapModifySrcMac] == 0) {
        val_flowfilter_entry->valid[UPLL_IDX_MODIFY_SRC_MAC_FFE] =
            UNC_VF_NOT_SUPPORTED;

        UPLL_LOG_DEBUG("modify_src attr is not supported by ctrlr");
        return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
      }
    }

    if ((val_flowfilter_entry->valid[UPLL_IDX_NWM_NAME_FFE] == UNC_VF_VALID)
        || (val_flowfilter_entry->valid[UPLL_IDX_NWM_NAME_FFE]
            == UNC_VF_VALID_NO_VALUE)) {
      if (attrs[unc::capa::vbr_flowfilter_entry::kCapNwmName] == 0) {
        val_flowfilter_entry->valid[UPLL_IDX_NWM_NAME_FFE] =
            UNC_VF_NOT_SUPPORTED;

        UPLL_LOG_DEBUG("Nwmname attr is not supported by ctrlr");
        return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
      }
    }

    if ((val_flowfilter_entry->valid[UPLL_IDX_DSCP_FFE] == UNC_VF_VALID)
        || (val_flowfilter_entry->valid[UPLL_IDX_DSCP_FFE]
            == UNC_VF_VALID_NO_VALUE)) {
      if (attrs[unc::capa::vbr_flowfilter_entry::kCapDscp] == 0) {
        val_flowfilter_entry->valid[UPLL_IDX_DSCP_FFE] = UNC_VF_NOT_SUPPORTED;

        UPLL_LOG_DEBUG("dscp attr is not supported by ctrlr");
        return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
      }
    }

    if ((val_flowfilter_entry->valid[UPLL_IDX_PRIORITY_FFE] == UNC_VF_VALID)
        || (val_flowfilter_entry->valid[UPLL_IDX_PRIORITY_FFE]
            == UNC_VF_VALID_NO_VALUE)) {
      if (attrs[unc::capa::vbr_flowfilter_entry::kCapPriority] == 0) {
        val_flowfilter_entry->valid[UPLL_IDX_PRIORITY_FFE] =
            UNC_VF_NOT_SUPPORTED;

        UPLL_LOG_DEBUG("Priority attr is not supported by ctrlr");
        return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
      }
    }
  } else {
    UPLL_LOG_DEBUG("value struct is NULL in "
               "ValFlowFilterEntryAttributeSupportCheck");
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrFlowFilterEntryMoMgr::CreateAuditMoImpl(ConfigKeyVal *ikey,
                                   DalDmlIntf *dmi,
                                   const char *ctrlr_id) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  UPLL_LOG_TRACE(" ikey is %s", ikey->ToStrAll().c_str());
  string vtn_name = "";

  controller_domain ctrlr_dom;
  memset(&ctrlr_dom, 0, sizeof(ctrlr_dom));
  result_code = GetControllerDomainID(ikey, &ctrlr_dom, UPLL_DT_AUDIT, dmi);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Failed to Get the Controller Domain details, err:%d",
                   result_code);
    return result_code;
  }

  val_flowfilter_entry_t *val_flowfilter_entry =
    static_cast<val_flowfilter_entry_t *>(
        ikey->get_cfg_val()->get_val());


  ctrlr_dom.ctrlr = NULL;
  ctrlr_dom.domain = NULL;
  GET_USER_DATA_CTRLR_DOMAIN(ikey, ctrlr_dom);
  UPLL_LOG_DEBUG("ctrlrid %s, domainid %s",
                 ctrlr_dom.ctrlr, ctrlr_dom.domain);

  FlowListMoMgr *mgr = reinterpret_cast<FlowListMoMgr *>
        (const_cast<MoManager *> (GetMoManager(UNC_KT_FLOWLIST)));
  if (mgr == NULL) {
    UPLL_LOG_DEBUG("Invalid FlowListMoMgr Instance");
    return UPLL_RC_ERR_GENERIC;
  }

  if (val_flowfilter_entry->valid[UPLL_IDX_FLOWLIST_NAME_FFE] == UNC_VF_VALID) {
    result_code = mgr->AddFlowListToController(
        reinterpret_cast<char *>
        (val_flowfilter_entry->flowlist_name), dmi,
        reinterpret_cast<char *> (const_cast<char *>(ctrlr_id)) ,
        UPLL_DT_AUDIT, UNC_OP_CREATE, TC_CONFIG_GLOBAL, vtn_name, false);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Reference Count Updation Fails %d", result_code);
      return result_code;
    }
  }
  result_code = SetValidAudit(ikey);
  if (UPLL_RC_SUCCESS != result_code) {
    return result_code;
  }
  // Create a record in AUDIT DB
  result_code = UpdateConfigDB(ikey, UPLL_DT_AUDIT,
                               UNC_OP_CREATE, dmi, TC_CONFIG_GLOBAL, vtn_name,
                               MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("UpdateConfigDB Failed err_code %d", result_code);
    return result_code;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrFlowFilterEntryMoMgr::VerifyRedirectDestination(
    ConfigKeyVal *ikey,
    DalDmlIntf *dmi,
    upll_keytype_datatype_t dt_type) {
  UPLL_FUNC_TRACE;
  MoMgrImpl *mgr = NULL;
  DbSubOp dbop = {kOpReadSingle, kOpMatchCtrlr|kOpMatchDomain, kOpInOutFlag};
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *okey = NULL;
  if (!ikey || !ikey->get_key()) {
    UPLL_LOG_DEBUG("input key is null");
    return UPLL_RC_ERR_GENERIC;
  }

  controller_domain ctrlr_dom;
  memset(&ctrlr_dom, 0, sizeof(controller_domain));
  key_vbr_flowfilter_entry_t *key_vbr_ffe =
    reinterpret_cast<key_vbr_flowfilter_entry_t *>(ikey->get_key());

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
    result_code = GetControllerDomainID(ikey, &ctrlr_dom, dt_type, dmi);
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
             == UPLL_FLOWFILTER_DIR_OUT ) {
      UPLL_LOG_DEBUG(" Redirect-direction is OUT");
       mgr = reinterpret_cast<MoMgrImpl *>
         (const_cast<MoManager *>(GetMoManager(UNC_KT_VBR_IF)));
      if (NULL == mgr) {
        UPLL_LOG_DEBUG("Unable to get VBRIDGE Interface object");
        return UPLL_RC_ERR_GENERIC;
      }
      result_code = mgr->GetChildConfigKey(okey, NULL);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Memory allocation failed for VBRIDGE key struct - %d",
           result_code);
        DELETE_IF_NOT_NULL(okey);
        return result_code;
      }
      key_vbr_if_t *vbrif_key = static_cast<key_vbr_if_t*>(
          okey->get_key());
      uuu::upll_strncpy(vbrif_key->vbr_key.vtn_key.vtn_name,
          key_vbr_ffe->flowfilter_key.vbr_key.vtn_key.vtn_name,
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
        if (ctrlrtype == UNC_CT_ODC) {
          UPLL_LOG_DEBUG("Cannnot set the vbr if with redirect-direction"
                         "as OUT for ctrlr %d", ctrlrtype);
          DELETE_IF_NOT_NULL(okey);
          return UPLL_RC_ERR_CFG_SEMANTIC;
        }
      // If Destination  Interface in UNC is vbridge interface  and If
      // Usage in UNC is PortMapped , configured  And  redirect-direction
      // is OUT.then the validation will get Success And will send the
      // vexternal/vexternal if to Driver.
        val_drv_vbr_if *val_drv_vbr =
           reinterpret_cast<val_drv_vbr_if *>(GetVal(okey));
        bool port_map_status =
            (val_drv_vbr->vbr_if_val.valid[UPLL_IDX_PM_VBRI] == UNC_VF_VALID)?
                                    true:false;
        if (dt_type == UPLL_DT_IMPORT) {
          DELETE_IF_NOT_NULL(okey);
          UPLL_LOG_DEBUG(" Datatype is Import : MergeValidate");
          if (port_map_status) {
            UPLL_LOG_DEBUG(" port_map_status flag is set : MergeValidate");
            return UPLL_RC_SUCCESS;
          } else {
            UPLL_LOG_INFO(" Datatype is Import:MergeValidate having Conflict");
            return UPLL_RC_ERR_CFG_SEMANTIC;
          }
        }
        if (port_map_status) {
          UPLL_LOG_DEBUG("redirect node/port As VBR/VBR_IF exists"
                "in DB And Redirect-direction is OUT");
          if (!strlen(reinterpret_cast<const char *>(val_drv_vbr->vex_name))) {
            UPLL_LOG_DEBUG("val_drv_vbr->vex_name name is not Valid");
            DELETE_IF_NOT_NULL(okey);
            return UPLL_RC_ERR_GENERIC;
          }
          if (!strlen(reinterpret_cast<const char *>
                              (val_drv_vbr->vex_if_name))) {
            UPLL_LOG_DEBUG("val_drv_vbr->vex_if_name name is not Valid");
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
          UPLL_LOG_DEBUG("Semantic error");
          return UPLL_RC_SUCCESS;
        }
       DELETE_IF_NOT_NULL(okey);
       UPLL_LOG_DEBUG("Portmap is not configured on this vbr and interface");
       return UPLL_RC_ERR_CFG_SEMANTIC;
     } else if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
      // If  Destination Interface in UNC is vterminal interface
      // redirect-direction is OUT .then the validation will
      // get UPLL_RC_ERR_CFG_SEMANTIC
      DELETE_IF_NOT_NULL(okey);
      mgr = reinterpret_cast<MoMgrImpl *>
        (const_cast<MoManager *>(GetMoManager(UNC_KT_VTERM_IF)));
      if (NULL == mgr) {
        UPLL_LOG_DEBUG("Unable to get VTERM Interface object");
        return UPLL_RC_ERR_GENERIC;
      }
      result_code = mgr->GetChildConfigKey(okey, NULL);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Memory allocation failed for VTERM key struct - %d",
            result_code);
        DELETE_IF_NOT_NULL(okey);
        return result_code;
      }
      key_vterm_if_t *vtermif_key = static_cast<key_vterm_if_t*>(
          okey->get_key());
      uuu::upll_strncpy(vtermif_key->vterm_key.vtn_key.vtn_name,
          key_vbr_ffe->flowfilter_key.vbr_key.vtn_key.vtn_name,
          kMaxLenVtnName + 1);
      uuu::upll_strncpy(vtermif_key->vterm_key.vterminal_name,
          reinterpret_cast<char *>
          (val_flowfilter_entry->redirect_node),
          (kMaxLenVnodeName + 1));
      uuu::upll_strncpy(vtermif_key->if_name,
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
        UPLL_LOG_DEBUG("requested vtn node/interface VTERMINAL/VTERMINAL_IF"
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
        UPLL_LOG_DEBUG("Unable to get VBRIDGE Interface object");
        return UPLL_RC_ERR_GENERIC;
      }
      result_code = mgr->GetChildConfigKey(okey, NULL);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Memory allocation failed for VBRIDGE key struct - %d",
          result_code);
        DELETE_IF_NOT_NULL(okey);
        return result_code;
      }
      key_vbr_if_t *vbrif_key = static_cast<key_vbr_if_t*>(
        okey->get_key());
      uuu::upll_strncpy(vbrif_key->vbr_key.vtn_key.vtn_name,
        key_vbr_ffe->flowfilter_key.vbr_key.vtn_key.vtn_name,
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
          UPLL_LOG_DEBUG("Unable to get VROUTER Interface object");
          return UPLL_RC_ERR_GENERIC;
        }
        result_code = mgr->GetChildConfigKey(okey, NULL);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("Memory allocation failed for VROUTER key struct - %d",
            result_code);
          DELETE_IF_NOT_NULL(okey);
          return result_code;
        }
        key_vrt_if_t *vrtif_key = static_cast<key_vrt_if_t*>(
          okey->get_key());
        uuu::upll_strncpy(vrtif_key->vrt_key.vtn_key.vtn_name,
          key_vbr_ffe->flowfilter_key.vbr_key.vtn_key.vtn_name,
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
upll_rc_t VbrFlowFilterEntryMoMgr::ValidateAttribute(ConfigKeyVal *ikey,
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
    if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE)
      result_code = UPLL_RC_ERR_CFG_SEMANTIC;
    UPLL_LOG_DEBUG("Flowlist name in val_flowfilter_entry does not exists"
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

    // mode is vtn mode, verifies flowlist existence in running tbl
    if (config_mode == TC_CONFIG_VTN) {
      result_code = mgr->UpdateConfigDB(okey, UPLL_DT_RUNNING,
                                      UNC_OP_READ, dmi, MAINTBL);

      if (UPLL_RC_ERR_INSTANCE_EXISTS != result_code) {
        if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE)
          result_code = UPLL_RC_ERR_CFG_SEMANTIC;
        UPLL_LOG_ERROR("Flowlist name in val_vtn_flowfilter_entry does not"
                       "exists in running FLOWLIST table");
        delete okey;
        okey = NULL;
        return result_code;
      } else {
        result_code = UPLL_RC_SUCCESS;
      }
    }
  }
  delete okey;
  okey = NULL;
  }

  if (req->datatype == UPLL_DT_IMPORT) {
    if (val_flowfilter_entry->valid[UPLL_IDX_NWM_NAME_FFE]
          == UNC_VF_VALID) {
      // validate nwm_name in KT_VBR_NWMONITOR table
      mgr = reinterpret_cast<MoMgrImpl *>
            (const_cast<MoManager *>(GetMoManager(UNC_KT_VBR_NWMONITOR)));

      if (NULL == mgr) {
        UPLL_LOG_DEBUG("Unable to get KT_VBR_NWMONITOR object");
        return UPLL_RC_ERR_GENERIC;
      }

      // allocate memory for key_nwm key_struct
      result_code = mgr->GetChildConfigKey(okey, NULL);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Memory allocation failed for key_nwm struct - %d",
                    result_code);
        return result_code;
      }

      key_vbr_flowfilter_entry_t *key_vbr_ffe =
        reinterpret_cast<key_vbr_flowfilter_entry_t *>(ikey->get_key());
      // fill key_nwm from key/val VBR_FLOWFILTER_ENTRY structs
      key_nwm_t *key_nwm = static_cast<key_nwm_t*>(
        okey->get_key());

      uuu::upll_strncpy(key_nwm->nwmonitor_name,
        val_flowfilter_entry->nwm_name,
        kMaxLenNwmName+1);

      uuu::upll_strncpy(key_nwm->vbr_key.vtn_key.vtn_name,
                        key_vbr_ffe->flowfilter_key.vbr_key.vtn_key.vtn_name,
                        kMaxLenVtnName+1);

      // Check nwm_name exists in table
      result_code = mgr->UpdateConfigDB(okey, req->datatype, UNC_OP_READ,
                                     dmi, MAINTBL);

      if (UPLL_RC_ERR_INSTANCE_EXISTS != result_code) {
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
  }
  UPLL_LOG_DEBUG("ValidateAttribute Successfull.");
  return result_code;
}

upll_rc_t VbrFlowFilterEntryMoMgr::GetValid(void *val, uint64_t indx,
    uint8_t *&valid, upll_keytype_datatype_t dt_type, MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  if (val == NULL) {
    UPLL_LOG_DEBUG("Memory is not Allocated");
    return UPLL_RC_ERR_GENERIC;
  }

  if (tbl == MAINTBL) {
    switch (indx) {
    case uudst::vbr_flowfilter_entry::kDbiFlowlistName:
      valid = &(reinterpret_cast<val_flowfilter_entry *>
                (val))->valid[UPLL_IDX_FLOWLIST_NAME_FFE];
      break;
    case uudst::vbr_flowfilter_entry::kDbiAction:
      valid = &(reinterpret_cast<val_flowfilter_entry *>
                (val))->valid[UPLL_IDX_ACTION_FFE];
      break;
    case uudst::vbr_flowfilter_entry::kDbiRedirectNode:
      valid = &(reinterpret_cast<val_flowfilter_entry *>
                (val))->valid[UPLL_IDX_REDIRECT_NODE_FFE];
      break;
    case uudst::vbr_flowfilter_entry::kDbiRedirectPort:
      valid = &(reinterpret_cast<val_flowfilter_entry *>
               (val))->valid[UPLL_IDX_REDIRECT_PORT_FFE];
      break;
    case uudst::vbr_flowfilter_entry::kDbiRedirectDirection:
       valid = &(reinterpret_cast<val_flowfilter_entry *>
                   (val))->valid[UPLL_IDX_REDIRECT_DIRECTION_FFE];
       break;

    case uudst::vbr_flowfilter_entry::kDbiModifyDstMac:
      valid = &(reinterpret_cast<val_flowfilter_entry *>
               (val))->valid[UPLL_IDX_MODIFY_DST_MAC_FFE];
      break;
    case uudst::vbr_flowfilter_entry::kDbiModifySrcMac:
      valid = &(reinterpret_cast<val_flowfilter_entry *>
               (val))->valid[UPLL_IDX_MODIFY_SRC_MAC_FFE];
      break;
    case uudst::vbr_flowfilter_entry::kDbiNwmName:
      valid = &(reinterpret_cast<val_flowfilter_entry *>
               (val))->valid[UPLL_IDX_NWM_NAME_FFE];
      break;
    case uudst::vbr_flowfilter_entry::kDbiDscp:
      valid = &(reinterpret_cast<val_flowfilter_entry *>
                (val))->valid[UPLL_IDX_DSCP_FFE];
      break;
    case uudst::vbr_flowfilter_entry::kDbiPriority:
      valid = &(reinterpret_cast<val_flowfilter_entry *>
                (val))->valid[UPLL_IDX_PRIORITY_FFE];
      break;
    default:
      return UPLL_RC_ERR_GENERIC;
    }
  }

  UPLL_LOG_DEBUG("GetValidAttributte is Succesfull");
  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrFlowFilterEntryMoMgr::AllocVal(ConfigVal *&ck_val,
    upll_keytype_datatype_t dt_type, MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  void *val;

  if (ck_val != NULL) {
    UPLL_LOG_DEBUG("Already COntains some Data .AllocVal Fails");
    return UPLL_RC_ERR_GENERIC;
  }

  switch (tbl) {
    case MAINTBL:
      val = reinterpret_cast<void *>
          (ConfigKeyVal::Malloc(sizeof(val_flowfilter_entry_t)));
      ck_val = new ConfigVal(IpctSt::kIpcStValFlowfilterEntry, val);
      break;
    default:
      val = NULL;
      return UPLL_RC_ERR_GENERIC;
  }

  UPLL_LOG_DEBUG(" AllocVal Successfull.");
  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrFlowFilterEntryMoMgr::GetChildConfigKey(ConfigKeyVal *&okey,
                                                     ConfigKeyVal *parent_key) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  key_vbr_flowfilter_entry_t *vbr_ffe_key;
  void *pkey = NULL;
  if (parent_key == NULL) {
    vbr_ffe_key = reinterpret_cast<key_vbr_flowfilter_entry_t*>
        (ConfigKeyVal::Malloc(sizeof(key_vbr_flowfilter_entry_t)));
    // If no direction is specified , 0xFE is filled to bind output direction
    vbr_ffe_key->flowfilter_key.direction = 0xFE;
    okey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVbrFlowfilterEntry,
                            vbr_ffe_key, NULL);
    UPLL_LOG_DEBUG("Parent Key Filled %d", result_code);
    return UPLL_RC_SUCCESS;
  } else {
    pkey = parent_key->get_key();
  }
  if (!pkey) {
    UPLL_LOG_DEBUG("Parent Key is NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  if (okey) {
    if (okey->get_key_type() != UNC_KT_VBR_FLOWFILTER_ENTRY)
      return UPLL_RC_ERR_GENERIC;
  }

  if ((okey) && (okey->get_key())) {
    vbr_ffe_key = reinterpret_cast<key_vbr_flowfilter_entry_t *>
        (okey->get_key());
  } else {
    vbr_ffe_key = reinterpret_cast<key_vbr_flowfilter_entry_t *>
        (ConfigKeyVal::Malloc(sizeof(key_vbr_flowfilter_entry_t)));
    // If no direction is specified , 0xFE is filled to bind output direction
    vbr_ffe_key->flowfilter_key.direction = 0xFE;
  }
  switch (parent_key->get_key_type()) {
    case UNC_KT_VTN:
      uuu::upll_strncpy(vbr_ffe_key->flowfilter_key.vbr_key.vtn_key.vtn_name,
                        reinterpret_cast<key_vtn_t *>
                        (pkey)->vtn_name,
                        kMaxLenVtnName + 1);
      break;
    case UNC_KT_VBRIDGE:
      uuu::upll_strncpy(vbr_ffe_key->flowfilter_key.vbr_key.vtn_key.vtn_name,
                        reinterpret_cast<key_vbr_t *>
                        (pkey)->vtn_key.vtn_name,
                        kMaxLenVtnName + 1);
      uuu::upll_strncpy(vbr_ffe_key->flowfilter_key.vbr_key.vbridge_name,
                        reinterpret_cast<key_vbr_t *>
                        (pkey)->vbridge_name,
                        (kMaxLenVnodeName + 1));
      break;
    case UNC_KT_VBR_FLOWFILTER:
      uuu::upll_strncpy(vbr_ffe_key->flowfilter_key.vbr_key.vtn_key.vtn_name,
                        reinterpret_cast<key_vbr_flowfilter_t *>
                        (pkey)->vbr_key.vtn_key.vtn_name,
                        kMaxLenVtnName + 1);
      uuu::upll_strncpy(vbr_ffe_key->flowfilter_key.vbr_key.vbridge_name,
                        reinterpret_cast<key_vbr_flowfilter_t *>
                        (pkey)->vbr_key.vbridge_name,
                        (kMaxLenVnodeName + 1));
      vbr_ffe_key->flowfilter_key.direction =
                        reinterpret_cast<key_vbr_flowfilter_t *>
                        (pkey)->direction;
      break;
    case UNC_KT_VBR_FLOWFILTER_ENTRY:
      uuu::upll_strncpy(vbr_ffe_key->flowfilter_key.vbr_key.vtn_key.vtn_name,
                        reinterpret_cast<key_vbr_flowfilter_entry_t *>
                        (pkey)->flowfilter_key.vbr_key.vtn_key.vtn_name,
                        kMaxLenVtnName + 1);
      uuu::upll_strncpy(vbr_ffe_key->flowfilter_key.vbr_key.vbridge_name,
                        reinterpret_cast<key_vbr_flowfilter_entry_t *>
                        (pkey)->flowfilter_key.vbr_key.vbridge_name,
                        (kMaxLenVnodeName + 1));
      vbr_ffe_key->flowfilter_key.direction =
                        reinterpret_cast<key_vbr_flowfilter_entry_t *>
                        (pkey)->flowfilter_key.direction;
      vbr_ffe_key->sequence_num =
                        reinterpret_cast<key_vbr_flowfilter_entry_t *>
                        (pkey)->sequence_num;
      break;
    case UNC_KT_VBR_NWMONITOR:
      uuu::upll_strncpy(vbr_ffe_key->flowfilter_key.vbr_key.vtn_key.vtn_name,
                        reinterpret_cast<key_vbr_flowfilter_entry_t *>
                        (pkey)->flowfilter_key.vbr_key.vtn_key.vtn_name,
                        kMaxLenVtnName + 1);
      break;
    default:
      if (vbr_ffe_key) free(vbr_ffe_key);
      return UPLL_RC_ERR_GENERIC;
  }


  if ((okey) && !(okey->get_key())) {
    UPLL_LOG_DEBUG("okey not null and flow list name updated");
    okey->SetKey(IpctSt::kIpcStKeyVbrFlowfilterEntry, vbr_ffe_key);
  }

  if (!okey) {
    okey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY,
                            IpctSt::kIpcStKeyVbrFlowfilterEntry,
                            vbr_ffe_key, NULL);
  }
  SET_USER_DATA(okey, parent_key);
  UPLL_LOG_DEBUG("okey filled Succesfully %d", result_code);
  return result_code;
}

upll_rc_t VbrFlowFilterEntryMoMgr::DupConfigKeyVal(ConfigKeyVal *&okey,
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

  if (req->get_key_type() != UNC_KT_VBR_FLOWFILTER_ENTRY) {
    UPLL_LOG_DEBUG(" DupConfigKeyval Failed.");
    return UPLL_RC_ERR_GENERIC;
  }

  ConfigVal *tmp1 = NULL, *tmp = (req)->get_cfg_val();
#if 0
  if (NULL == tmp) {
    UPLL_LOG_DEBUG("Memory Not Allocated");
    return UPLL_RC_ERR_GENERIC;
  }
#endif
  if (tmp) {
    if (tbl == MAINTBL) {
      val_flowfilter_entry_t *ival = NULL;
       ival = reinterpret_cast<val_flowfilter_entry_t *> (GetVal(req));
      if (NULL != ival) {
        val_flowfilter_entry_t *vbr_flowfilter_entry_val =
            reinterpret_cast<val_flowfilter_entry_t *>
           (ConfigKeyVal::Malloc(sizeof(val_flowfilter_entry_t)));

        memcpy(vbr_flowfilter_entry_val, ival, sizeof(val_flowfilter_entry_t));
        tmp1 = new ConfigVal(IpctSt::kIpcStValFlowfilterEntry,
            vbr_flowfilter_entry_val);
      }
      if (NULL == tmp1) {
        UPLL_LOG_DEBUG("Memory Not Allocated");
        return UPLL_RC_ERR_GENERIC;
      }
      tmp1->set_user_data(tmp->get_user_data());
    }
  }

  void *tkey = (req != NULL) ? (req)->get_key() : NULL;
  key_vbr_flowfilter_entry_t *ikey = NULL;
  ikey = reinterpret_cast<key_vbr_flowfilter_entry_t *> (tkey);
  key_vbr_flowfilter_entry_t *vbr_flowfilter_entry =
      reinterpret_cast<key_vbr_flowfilter_entry_t*>
        (ConfigKeyVal::Malloc(sizeof(key_vbr_flowfilter_entry_t)));

  memcpy(vbr_flowfilter_entry, ikey, sizeof(key_vbr_flowfilter_entry_t));

  okey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY,
      IpctSt::kIpcStKeyVbrFlowfilterEntry, vbr_flowfilter_entry,
      tmp1);

  if (okey) {
    SET_USER_DATA(okey, req)
  } else {
    UPLL_LOG_DEBUG("okey is Null");
    DELETE_IF_NOT_NULL(tmp1);
    return UPLL_RC_ERR_GENERIC;
  }

  UPLL_LOG_DEBUG("DupConfigkeyVal Succesfull.");
  // free(vbr_if_flowfilter_entry);
  // free(vbr_if_flowfilter_val);
  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrFlowFilterEntryMoMgr::GetRenamedUncKey(ConfigKeyVal *ikey,
    upll_keytype_datatype_t dt_type, DalDmlIntf *dmi, uint8_t *ctrlr_id) {
  UPLL_FUNC_TRACE;
  UPLL_LOG_TRACE("%s GetRenamedUncKey vbrff_entry start",
                  ikey->ToStrAll().c_str());
  ConfigKeyVal *unc_key = NULL;
  uint8_t rename = 0;
  if ((NULL == ikey) || (ctrlr_id == NULL) || (NULL == dmi)) {
    UPLL_LOG_DEBUG("ikey/ctrlr_id dmi NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  DbSubOp dbop = { kOpReadSingle, kOpMatchCtrlr, kOpInOutNone };
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  MoMgrImpl *mgrvbr = reinterpret_cast<MoMgrImpl *>
         (const_cast<MoManager *> (GetMoManager(UNC_KT_VBRIDGE)));
  if (!mgrvbr) {
    UPLL_LOG_TRACE("mgrvbr failed");
    return UPLL_RC_ERR_GENERIC;
  }
  val_rename_vnode *rename_val = reinterpret_cast<val_rename_vnode*>
      (ConfigKeyVal::Malloc(sizeof(val_rename_vnode)));
  if (!rename_val) {
    UPLL_LOG_TRACE("rename_val NULL");
    return UPLL_RC_ERR_GENERIC;
  }

  key_vbr_flowfilter_entry_t *ctrlr_key =
      reinterpret_cast<key_vbr_flowfilter_entry_t *> (ikey->get_key());
  if (!ctrlr_key) {
    UPLL_LOG_TRACE("rename_val NULL");
    free(rename_val);
    return UPLL_RC_ERR_GENERIC;
  }
  uuu::upll_strncpy(rename_val->ctrlr_vtn_name,
          ctrlr_key->flowfilter_key.vbr_key.vtn_key.vtn_name,
          (kMaxLenVtnName + 1));
  rename_val->valid[UPLL_CTRLR_VTN_NAME_VALID] = UNC_VF_VALID;

  uuu::upll_strncpy(rename_val->ctrlr_vnode_name,
                    ctrlr_key->flowfilter_key.vbr_key.vbridge_name,
                    (kMaxLenVnodeName + 1));
  rename_val->valid[UPLL_CTRLR_VNODE_NAME_VALID] = UNC_VF_VALID;

  result_code = mgrvbr->GetChildConfigKey(unc_key, NULL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetChildConfigKey returned error");
    free(rename_val);
    mgrvbr = NULL;
    return result_code;
  }
  if (!unc_key) {
    UPLL_LOG_DEBUG("unc_key NULL");
    free(rename_val);
    mgrvbr = NULL;
    return result_code;
  }
  SET_USER_DATA_CTRLR(unc_key, ctrlr_id);
  unc_key->AppendCfgVal(IpctSt::kIpcStValRenameVtn, rename_val);
  result_code = mgrvbr->ReadConfigDB(unc_key, dt_type, UNC_OP_READ,
      dbop, dmi, RENAMETBL);
  if ((UPLL_RC_SUCCESS != result_code) &&
      (UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code)) {
    UPLL_LOG_DEBUG("ReadConfigDB failed %d", result_code);
    DELETE_IF_NOT_NULL(unc_key);
    mgrvbr = NULL;
    return result_code;
  }

  if (result_code == UPLL_RC_SUCCESS) {
    key_vbr_flowfilter_entry_t *vbr_flowfilter_entry_key =
        reinterpret_cast<key_vbr_flowfilter_entry_t *> (unc_key->get_key());
    if (strcmp(reinterpret_cast<char *>(ctrlr_key->flowfilter_key
               .vbr_key.vtn_key.vtn_name),
               reinterpret_cast<const char *>(vbr_flowfilter_entry_key
               ->flowfilter_key.vbr_key.vtn_key.vtn_name))) {
       uuu::upll_strncpy(ctrlr_key->flowfilter_key.vbr_key.vtn_key.vtn_name,
            vbr_flowfilter_entry_key->flowfilter_key.vbr_key.vtn_key.vtn_name,
            (kMaxLenVtnName + 1));
       rename |= VTN_RENAME;
    }
    if (strcmp(reinterpret_cast<char *>(ctrlr_key->flowfilter_key
               .vbr_key.vbridge_name),
               reinterpret_cast<const char *>(vbr_flowfilter_entry_key->
               flowfilter_key.vbr_key.vbridge_name))) {
       uuu::upll_strncpy(ctrlr_key->flowfilter_key.vbr_key.vbridge_name,
              vbr_flowfilter_entry_key->flowfilter_key.vbr_key.vbridge_name,
              (kMaxLenVnodeName + 1));
      rename |= VBR_RENAME_FLAG;
    }
    SET_USER_DATA(ikey, unc_key);
    SET_USER_DATA_FLAGS(ikey, rename);
  }
  mgrvbr = NULL;
  DELETE_IF_NOT_NULL(unc_key);

  val_flowfilter_entry_t *val_flowfilter_entry =
      reinterpret_cast<val_flowfilter_entry_t *> (GetVal(ikey));
  if (!val_flowfilter_entry) {
    UPLL_LOG_ERROR("val_flowfilter_entry NULL");
    return UPLL_RC_SUCCESS;
  }

  if (UNC_VF_VALID == val_flowfilter_entry
                      ->valid[UPLL_IDX_FLOWLIST_NAME_FFE]) {
    val_rename_flowlist_t *rename_flowlist =
      reinterpret_cast<val_rename_flowlist_t*>
                   (ConfigKeyVal::Malloc(sizeof(val_rename_flowlist_t)));
    if (!rename_flowlist) {
      UPLL_LOG_DEBUG("rename_flowlist NULL %d", result_code);
      return UPLL_RC_ERR_GENERIC;
    }

    uuu::upll_strncpy(rename_flowlist->flowlist_newname,
                    val_flowfilter_entry->flowlist_name,
                    (kMaxLenFlowListName + 1));
  rename_flowlist->valid[UPLL_IDX_RENAME_FLOWLIST_RFL] = UNC_VF_VALID;

  MoMgrImpl* mgr = reinterpret_cast<MoMgrImpl *>
         (const_cast<MoManager *> (GetMoManager(UNC_KT_FLOWLIST)));
  if (!mgr) {
    UPLL_LOG_DEBUG("mgr failed");
    if (rename_flowlist) free(rename_flowlist);
    return UPLL_RC_ERR_GENERIC;
  }
  result_code = mgr->GetChildConfigKey(unc_key, NULL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetChildConfigKey Returned an error");
    free(rename_flowlist);
    mgr = NULL;
    return result_code;
  }
  SET_USER_DATA_CTRLR(unc_key, ctrlr_id);
  unc_key->AppendCfgVal(IpctSt::kIpcStValRenameFlowlist, rename_flowlist);
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
      key_flowlist_t *key_flowlist = NULL;
      key_flowlist = reinterpret_cast<key_flowlist_t *> (unc_key->get_key());

      uuu::upll_strncpy(val_flowfilter_entry->flowlist_name,
                      key_flowlist->flowlist_name,
                      (kMaxLenFlowListName + 1));
      rename |= FLOW_RENAME;
      SET_USER_DATA(ikey, unc_key);
      SET_USER_DATA_FLAGS(ikey, rename);
    }
    DELETE_IF_NOT_NULL(unc_key);
    mgr = NULL;
  }

  if (((UNC_VF_VALID ==
      val_flowfilter_entry->valid[UPLL_IDX_REDIRECT_NODE_FFE]) ||
          (val_flowfilter_entry->valid[UPLL_IDX_REDIRECT_NODE_FFE] ==
             UNC_VF_VALUE_NOT_MODIFIED)) &&
      ((UNC_VF_VALID ==
      val_flowfilter_entry->valid[UPLL_IDX_REDIRECT_PORT_FFE])||
       (val_flowfilter_entry->valid[UPLL_IDX_REDIRECT_PORT_FFE] ==
        UNC_VF_VALUE_NOT_MODIFIED))) {
    unc_key_type_t child_key[]= { UNC_KT_VBRIDGE, UNC_KT_VROUTER ,
      UNC_KT_VTERMINAL };
    bool isRedirectVnodeVbridge = false;
    for (unsigned int i = 0;
      i < sizeof(child_key)/sizeof(child_key[0]); i++) {
      const unc_key_type_t ktype = child_key[i];
      MoMgrImpl *mgrvbr = reinterpret_cast<MoMgrImpl *>(
          const_cast<MoManager *>(GetMoManager(ktype)));
      if (!mgrvbr) {
        UPLL_LOG_TRACE("mgrvbr failed");
        return UPLL_RC_ERR_GENERIC;
      }
      val_rename_vnode *rename_val = reinterpret_cast<val_rename_vnode*>
        (ConfigKeyVal::Malloc(sizeof(val_rename_vnode)));
      if (!rename_val) {
        UPLL_LOG_TRACE("rename_val NULL");
        return UPLL_RC_ERR_GENERIC;
      }

      uuu::upll_strncpy(rename_val->ctrlr_vnode_name,
                       val_flowfilter_entry->redirect_node,
                       (kMaxLenVnodeName + 1));
      rename_val->valid[UPLL_CTRLR_VNODE_NAME_VALID] = UNC_VF_VALID;

      result_code = mgrvbr->GetChildConfigKey(unc_key, NULL);
      if (UPLL_RC_SUCCESS != result_code) {
        UPLL_LOG_DEBUG("GetChildConfigKey Returned an error");
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
        UPLL_LOG_DEBUG("ReadConfigDB failed %d", result_code);
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
          key_vrt *vrt_key = reinterpret_cast<key_vrt *>(unc_key->get_key());
          uuu::upll_strncpy(val_flowfilter_entry->redirect_node,
                           vrt_key->vrouter_name,
                          (kMaxLenVnodeName + 1));
        } else if (unc_key->get_key_type() == UNC_KT_VTERMINAL) {
          key_vterm *vterm_key =
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
  UPLL_LOG_TRACE("%s GetRenamedUncKey vbrff_entry end",
                  ikey->ToStrAll().c_str());
  DELETE_IF_NOT_NULL(unc_key);
  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrFlowFilterEntryMoMgr::GetRenamedControllerKey(
    ConfigKeyVal *ikey,
    upll_keytype_datatype_t dt_type,
    DalDmlIntf *dmi,
    controller_domain *ctrlr_dom) {

  UPLL_FUNC_TRACE;
  ConfigKeyVal *okey = NULL;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  if (NULL == ctrlr_dom) {
    UPLL_LOG_DEBUG("ctrlr null");
    return UPLL_RC_ERR_GENERIC;
  }

  /* Get the controller's redirect node(vbridge/vrt) name -start*/
  val_flowfilter_entry_t *val_flowfilter_entry =
    reinterpret_cast<val_flowfilter_entry_t *> (GetVal(ikey));

  if (val_flowfilter_entry) {
    if (((UNC_VF_VALID ==
          val_flowfilter_entry->valid[UPLL_IDX_REDIRECT_NODE_FFE])

        ||(val_flowfilter_entry->valid[UPLL_IDX_REDIRECT_NODE_FFE] ==
        UNC_VF_VALUE_NOT_MODIFIED)) &&
        ((UNC_VF_VALID ==
         val_flowfilter_entry->valid[UPLL_IDX_REDIRECT_PORT_FFE])
        ||(val_flowfilter_entry->valid[UPLL_IDX_REDIRECT_PORT_FFE] ==
        UNC_VF_VALUE_NOT_MODIFIED))) {
      unc_key_type_t child_key[]= { UNC_KT_VBRIDGE, UNC_KT_VROUTER ,
        UNC_KT_VTERMINAL };
      bool isRedirectVnodeVbridge = false;
      for (unsigned int i = 0;
          i < sizeof(child_key)/sizeof(child_key[0]); i++) {
        const unc_key_type_t ktype = child_key[i];
        MoMgrImpl *mgrvbr = reinterpret_cast<MoMgrImpl *>(
            const_cast<MoManager *>(GetMoManager(ktype)));
        if (!mgrvbr) {
          UPLL_LOG_DEBUG("mgrvbr failed");
          return UPLL_RC_ERR_GENERIC;
        }

        result_code = mgrvbr->GetChildConfigKey(okey, NULL);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("GetChildConfigKey fail");
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

           UPLL_LOG_DEBUG("redirect node vrt name (%s) (%s)",
           reinterpret_cast<key_vterm_t *>(okey->get_key())->vterminal_name,
           reinterpret_cast<val_flowfilter_entry_t *>(ikey->get_cfg_val()->
           get_val())->flowlist_name);
         }


        DbSubOp dbop = { kOpReadSingle, kOpMatchCtrlr | kOpMatchDomain,
          kOpInOutFlag };
        result_code = mgrvbr->ReadConfigDB(okey, dt_type, UNC_OP_READ,
            dbop, dmi, RENAMETBL);
        if (result_code != UPLL_RC_SUCCESS) {
          if (UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
            UPLL_LOG_DEBUG("ReadConfigDB fail");
            DELETE_IF_NOT_NULL(okey);
            return result_code;
          }
        }

        if (result_code == UPLL_RC_SUCCESS) {
          val_rename_vnode *rename_val = NULL;
          isRedirectVnodeVbridge = true;
          rename_val = reinterpret_cast<val_rename_vnode *> (GetVal(okey));
          if (!rename_val) {
            UPLL_LOG_DEBUG("rename_val NULL.");
            DELETE_IF_NOT_NULL(okey);
            return UPLL_RC_ERR_GENERIC;
          }

          uuu::upll_strncpy(reinterpret_cast<val_flowfilter_entry_t*>
              (ikey->get_cfg_val()->get_val())->redirect_node,
              rename_val->ctrlr_vnode_name, (kMaxLenVnodeName + 1));
        }
        DELETE_IF_NOT_NULL(okey);
        if (isRedirectVnodeVbridge)
          break;
      }
    }
  }
  /* -end*/
  UPLL_LOG_TRACE("Start Input ConfigKeyVal %s",
      ikey->ToStrAll().c_str());
  MoMgrImpl *mgrvbr = reinterpret_cast<MoMgrImpl *>
    (const_cast<MoManager *> (GetMoManager(UNC_KT_VBRIDGE)));

  result_code = mgrvbr->GetChildConfigKey(okey, NULL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetChildConfigKey fail");
    return result_code;
  }

  SET_USER_DATA_CTRLR_DOMAIN(okey, *ctrlr_dom)

  UPLL_LOG_DEBUG("ctrlr : %s; domain : %s", ctrlr_dom->ctrlr,
      ctrlr_dom->domain);

  uuu::upll_strncpy(
      reinterpret_cast<key_vbr *> (okey->get_key())->vtn_key.vtn_name,
      reinterpret_cast<key_vbr_flowfilter_entry_t *>
      (ikey->get_key())->flowfilter_key.vbr_key.vtn_key.vtn_name,
      (kMaxLenVnodeName + 1));
  UPLL_LOG_DEBUG("vtn name (%s) (%s)",
      reinterpret_cast<key_vbr *> (okey->get_key())->vtn_key.vtn_name,
      reinterpret_cast<key_vbr_flowfilter_entry_t *>
      (ikey->get_key())->flowfilter_key.vbr_key.vtn_key.vtn_name);

  uuu::upll_strncpy(reinterpret_cast<key_vbr *>
      (okey->get_key())->vbridge_name,
      reinterpret_cast<key_vbr_flowfilter_entry_t *>
      (ikey->get_key())->flowfilter_key.vbr_key.vbridge_name,
      (kMaxLenVnodeName + 1));

  UPLL_LOG_DEBUG("vtn name (%s) (%s)",
      reinterpret_cast<key_vbr *>
      (okey->get_key())->vbridge_name,
      reinterpret_cast<key_vbr_flowfilter_entry_t *>
      (ikey->get_key())->flowfilter_key.vbr_key.vbridge_name);
  DbSubOp dbop = { kOpReadSingle, kOpMatchCtrlr | kOpMatchDomain,
    kOpInOutFlag };
  result_code = mgrvbr->ReadConfigDB(okey, dt_type, UNC_OP_READ,
      dbop, dmi, RENAMETBL);
  if ((result_code != UPLL_RC_SUCCESS) &&
      (UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code)) {
    UPLL_LOG_DEBUG("ReadConfigDB fail");
    DELETE_IF_NOT_NULL(okey);
    return result_code;
  }
  if (UPLL_RC_SUCCESS == result_code) {
    val_rename_vnode *rename_val = NULL;
    rename_val = reinterpret_cast<val_rename_vnode *> (GetVal(okey));
    if (!rename_val) {
      UPLL_LOG_DEBUG("Vbr Name is not Valid.");
      DELETE_IF_NOT_NULL(okey);
      return UPLL_RC_ERR_GENERIC;
    }

    uuu::upll_strncpy(reinterpret_cast<key_vbr_flowfilter_entry_t *>
        (ikey->get_key())->flowfilter_key.vbr_key.vtn_key.vtn_name,
        rename_val->ctrlr_vtn_name,
        (kMaxLenVtnName + 1));

    uuu::upll_strncpy(reinterpret_cast<char *>
        (reinterpret_cast<key_vbr_flowfilter_entry_t *>
         (ikey->get_key())->flowfilter_key.vbr_key.vbridge_name),
        reinterpret_cast<const char *> (rename_val->ctrlr_vnode_name),
        kMaxLenVnodeName + 1);
  }
  DELETE_IF_NOT_NULL(okey);

  // Flowlist is Renamed
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
  MoMgrImpl *mgrflist = reinterpret_cast<MoMgrImpl *>
    (const_cast<MoManager *> (GetMoManager(UNC_KT_FLOWLIST)));
  result_code = mgrflist->GetChildConfigKey(okey, NULL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetChildConfigKey fail");
    return result_code;
  }
  SET_USER_DATA_CTRLR(okey, ctrlr_dom->ctrlr);

  uuu::upll_strncpy(
      reinterpret_cast<key_flowlist_t *>
      (okey->get_key())->flowlist_name,
      reinterpret_cast<val_flowfilter_entry_t *>(ikey->get_cfg_val()->
        get_val())->flowlist_name,
      (kMaxLenFlowListName + 1));
  UPLL_LOG_DEBUG("flowlist name (%s) (%s)",
      reinterpret_cast<key_flowlist_t *>
      (okey->get_key())->flowlist_name,
      reinterpret_cast<val_flowfilter_entry_t *>(ikey->get_cfg_val()->
        get_val())->flowlist_name);
  //    UPLL_LOG_DEBUG("flowlist name (%s) (%s)",
  //    (okey->get_key())->flowlist_name,
  //    (ikey->get_cfg_val()->get_val())->flowlist_name);
  DbSubOp dbop1 = { kOpReadSingle, kOpMatchCtrlr, kOpInOutFlag };
  result_code = mgrflist->ReadConfigDB(okey, dt_type, UNC_OP_READ,
      dbop1, dmi, RENAMETBL);
  if ((result_code != UPLL_RC_SUCCESS) &&
      (UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code)) {
    UPLL_LOG_DEBUG("ReadConfigDB fail");
    DELETE_IF_NOT_NULL(okey);
    return result_code;
  }
  if (UPLL_RC_SUCCESS == result_code) {
    val_rename_flowlist_t *rename_val = NULL;
    rename_val = reinterpret_cast<val_rename_flowlist_t *> (GetVal(okey));

    if (!rename_val) {
      UPLL_LOG_DEBUG("flowlist is not valid");
      DELETE_IF_NOT_NULL(okey);
      return UPLL_RC_ERR_GENERIC;
    }

    uuu::upll_strncpy(reinterpret_cast<val_flowfilter_entry_t*>
        (ikey->get_cfg_val()->get_val())->flowlist_name,
        rename_val->flowlist_newname,
        (kMaxLenFlowListName + 1));
  }
  DELETE_IF_NOT_NULL(okey);
  UPLL_LOG_TRACE("%s GetRenamedCtrl vbr_ff_entry end",
      ikey->ToStrAll().c_str());
  UPLL_LOG_DEBUG("Renamed Controller key is sucessfull.");
  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrFlowFilterEntryMoMgr::GetControllerDomainID(ConfigKeyVal *ikey,
                                          controller_domain *ctrlr_dom,
                                          upll_keytype_datatype_t dt_type,
                                          DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *ckv = NULL;
  VbrMoMgr *mgrvbr =
    reinterpret_cast<VbrMoMgr *>(const_cast<MoManager *>(GetMoManager(
            UNC_KT_VBRIDGE)));
  result_code = mgrvbr->GetChildConfigKey(ckv, NULL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Unable to get the ParentConfigKey, resultcode=%d",
                    result_code);
    return result_code;
  }

  key_vbr_flowfilter_entry_t *ff_key = reinterpret_cast
    <key_vbr_flowfilter_entry_t *>(ikey->get_key());
  key_vbr_t *vbr_key = reinterpret_cast<key_vbr_t *>(ckv->get_key());

  uuu::upll_strncpy(vbr_key->vtn_key.vtn_name,
      ff_key->flowfilter_key.vbr_key.vtn_key.vtn_name,
      kMaxLenVtnName + 1);

  uuu::upll_strncpy(vbr_key->vbridge_name,
      ff_key->flowfilter_key.vbr_key.vbridge_name,
      kMaxLenVnodeName + 1);
  // Read Controller ID and Domain ID from the VBridge and set it to the
  // Input ConfigKeyVal
  DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutCtrlr|kOpInOutDomain };
  result_code =  mgrvbr->ReadConfigDB(ckv, dt_type, UNC_OP_READ,
                                            dbop, dmi, MAINTBL);

  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Unable to Read the details from DB for the parent err %d",
                    result_code);
    DELETE_IF_NOT_NULL(ckv);
    return result_code;
  }
  result_code = mgrvbr->GetControllerDomainId(ckv, ctrlr_dom);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetControllerDomainId failed %d ", result_code);
    DELETE_IF_NOT_NULL(ckv);
    return result_code;
  }
  uint8_t temp_flag = 0;
  GET_USER_DATA_FLAGS(ikey, temp_flag);
  SET_USER_DATA(ikey, ckv);
  SET_USER_DATA_FLAGS(ikey, temp_flag);
  DELETE_IF_NOT_NULL(ckv);
  return result_code;
}

upll_rc_t VbrFlowFilterEntryMoMgr::CreateCandidateMo(IpcReqRespHeader *req,
                                                     ConfigKeyVal *ikey,
                                                     DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  uint8_t *ctrlr_id = NULL;
  controller_domain ctrlr_dom;
  if (ikey == NULL && req == NULL) {
    UPLL_LOG_DEBUG(
        "Cannot perform create operation due to insufficient parameters");
    return UPLL_RC_ERR_GENERIC;
  }

  upll_rc_t result_code = UPLL_RC_SUCCESS;
  if (req->datatype != UPLL_DT_IMPORT) {
    // validate syntax and semantics
    result_code = ValidateMessage(req, ikey);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("ValidateMessage failed %d", result_code);
      return result_code;
    }
  }

  result_code = ValidateAttribute(ikey, dmi, req);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("ValidateAttribute failed %d", result_code);
    return result_code;
  }

  // Check if Object exists in RUNNING DB and move it to CANDIDATE DB
  val_flowfilter_entry_t *flowfilter_val =
      reinterpret_cast<val_flowfilter_entry_t *> (GetVal(ikey));
  result_code = GetControllerDomainID(ikey, &ctrlr_dom, req->datatype, dmi);
  if (result_code != UPLL_RC_SUCCESS) {
    if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
      result_code = UPLL_RC_ERR_PARENT_DOES_NOT_EXIST;
    }
    UPLL_LOG_DEBUG("Failed to Get the Controller Domain details, err:%d",
                   result_code);
    return result_code;
  }
  ctrlr_dom.ctrlr = NULL;
  ctrlr_dom.domain = NULL;
  GET_USER_DATA_CTRLR_DOMAIN(ikey, ctrlr_dom);

  UPLL_LOG_DEBUG("ctrlrid %s, domainid %s",
                 ctrlr_dom.ctrlr, ctrlr_dom.domain);

  // check whether this keytype is not requested under unified vbridge
  if (IsUnifiedVbr(ctrlr_dom.ctrlr)) {
    UPLL_LOG_DEBUG(
              "This KT is not allowed to be created under Unified Vbridge");
    return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_KT;
  }

  ctrlr_id = ctrlr_dom.ctrlr;

  result_code = ValidateCapability(req, ikey,
                                   reinterpret_cast<const char *>(ctrlr_id));
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("validate Capability Failed %d", result_code);
    return result_code;
  }
  TcConfigMode config_mode = TC_CONFIG_INVALID;
  std::string vtn_name = "";
  result_code = GetConfigModeInfo(req, config_mode, vtn_name);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetConfigMode failed");
    return result_code;
  }

  if (flowfilter_val->valid[UPLL_IDX_FLOWLIST_NAME_FFE] == UNC_VF_VALID) {
    std::string temp_vtn_name;
    if (TC_CONFIG_VTN == config_mode) {
      temp_vtn_name = vtn_name;
    } else {
      key_vbr_flowfilter_entry_t *temp_key = reinterpret_cast<
          key_vbr_flowfilter_entry_t *>(ikey->get_key());
      temp_vtn_name = reinterpret_cast<const char*>(
          temp_key->flowfilter_key.vbr_key.vtn_key.vtn_name);
    }
    FlowListMoMgr *mgr = reinterpret_cast<FlowListMoMgr *>
      (const_cast<MoManager *> (GetMoManager(UNC_KT_FLOWLIST)));
    result_code = mgr->AddFlowListToController(
        reinterpret_cast<char *>(flowfilter_val->flowlist_name), dmi,
        reinterpret_cast<char *>(ctrlr_id) , req->datatype, UNC_OP_CREATE,
        config_mode, temp_vtn_name, false);

    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Unable to Update the FlowList at ctrlr Table");
      return result_code;
    }
  }

  // Set the rename flag info
  result_code = SetRenameFlag(ikey, dmi, req);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("SetRenameFlag failed");
    return result_code;
  }

  // create a record in CANDIDATE DB
  DbSubOp dbop = { kOpNotRead, kOpMatchNone, kOpInOutDomain
    | kOpInOutCtrlr | kOpInOutFlag };
  result_code = UpdateConfigDB(ikey, req->datatype, UNC_OP_CREATE,
                               dmi, &dbop, config_mode, vtn_name, MAINTBL);
  return result_code;
}

upll_rc_t VbrFlowFilterEntryMoMgr::RestorePOMInCtrlTbl(
    ConfigKeyVal *ikey,
    upll_keytype_datatype_t dt_type,
    MoMgrTables tbl,
    DalDmlIntf* dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  controller_domain ctrlr_dom;
//  FlowListMoMgr *mgr = NULL;
//  uint8_t *ctrlr_id = NULL;

  if (!ikey || !(ikey->get_key())) {
    UPLL_LOG_DEBUG("Input Key Not Valid");
    return UPLL_RC_ERR_GENERIC;
  }
  if (tbl != MAINTBL ||
       ikey->get_key_type() != UNC_KT_VBR_FLOWFILTER_ENTRY) {
    UPLL_LOG_DEBUG("Ignoring  ktype/Table kt=%d, tbl=%d",
                    ikey->get_key_type(), tbl);
    return result_code;
  }
  val_flowfilter_entry_t *flowfilter_val =
      reinterpret_cast<val_flowfilter_entry_t *> (GetVal(ikey));

  if (NULL == flowfilter_val) {
    UPLL_LOG_DEBUG(" Value structure is NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  if (flowfilter_val->valid[UPLL_IDX_FLOWLIST_NAME_FFE] == UNC_VF_VALID) {
    result_code = GetControllerDomainID(ikey, &ctrlr_dom, dt_type, dmi);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Failed to Get the Controller Domain details, err:%d",
                     result_code);
      return result_code;
    }
    ctrlr_dom.ctrlr = NULL;
    ctrlr_dom.domain = NULL;
    GET_USER_DATA_CTRLR_DOMAIN(ikey, ctrlr_dom);

/*
    ctrlr_id = ctrlr_dom.ctrlr;
    mgr = reinterpret_cast<FlowListMoMgr *>
        (const_cast<MoManager *> (GetMoManager(UNC_KT_FLOWLIST)));

    result_code = mgr->AddFlowListToController(
              reinterpret_cast<char *>(flowfilter_val->flowlist_name),
              dmi,
              reinterpret_cast<char *>(ctrlr_id),
              dt_type,
              UNC_OP_CREATE);
    if (result_code != UPLL_RC_SUCCESS) {
       UPLL_LOG_DEBUG("Unable to Update the FlowList at ctrlr Table, err %d",
                     result_code);
       return result_code;
    }
*/
  }
  return result_code;
}
upll_rc_t VbrFlowFilterEntryMoMgr::DeleteMo(IpcReqRespHeader *req,
                                            ConfigKeyVal *ikey,
                                            DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_ERR_GENERIC;
  uint8_t *ctrlr_id = NULL;

  if (NULL == ikey && NULL == req) {
    UPLL_LOG_DEBUG("Request & InputKey are not are not Valid %d", result_code);
    return result_code;
  }

  result_code = ValidateMessage(req, ikey);

  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("validate Message Failed %d", result_code);
    return result_code;
  }
  /*
  result_code = ValidateCapability(req, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("validate Capability Failed %d", result_code);
    return result_code;
  } */
  result_code = UpdateConfigDB(ikey, UPLL_DT_CANDIDATE, UNC_OP_READ, dmi);

  if (UPLL_RC_ERR_INSTANCE_EXISTS != result_code) {
    UPLL_LOG_DEBUG("Instance does not exists %d", result_code);
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
    std::string temp_vtn_name;
    if (TC_CONFIG_VTN == config_mode) {
      temp_vtn_name = vtn_name;
    } else {
      key_vbr_flowfilter_entry_t *temp_key = reinterpret_cast<
          key_vbr_flowfilter_entry_t *>(ikey->get_key());
      temp_vtn_name = reinterpret_cast<const char*>(
          temp_key->flowfilter_key.vbr_key.vtn_key.vtn_name);
    }
    FlowListMoMgr *mgr = reinterpret_cast<FlowListMoMgr *>
        (const_cast<MoManager *> (GetMoManager(UNC_KT_FLOWLIST)));
    result_code = mgr->AddFlowListToController(
        reinterpret_cast<char *>(flowfilter_val->flowlist_name), dmi,
        reinterpret_cast<char *>(ctrlr_id), req->datatype, UNC_OP_DELETE,
        config_mode, temp_vtn_name, false);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG(" Send delete request to flowlist failed. err code(%d)",
                     result_code);
      DELETE_IF_NOT_NULL(okey);
      return result_code;
    }
  }
  result_code = UpdateConfigDB(ikey, UPLL_DT_CANDIDATE, UNC_OP_DELETE, dmi,
                               config_mode, vtn_name, MAINTBL);

  DELETE_IF_NOT_NULL(okey);
  UPLL_LOG_DEBUG("DeleteMo Operation Done %d", result_code);
  return result_code;
}

upll_rc_t VbrFlowFilterEntryMoMgr::MergeValidate(unc_key_type_t keytype,
    const char *ctrlr_id, ConfigKeyVal *ikey,
    DalDmlIntf *dmi, upll_import_type import_type) {
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
  ConfigKeyVal *tmp_ckval = ckval;
  while (NULL != ckval) {
    val_flowfilter_entry_t* val = reinterpret_cast<val_flowfilter_entry_t *>
    (GetVal(ckval));
    if (val->valid[UPLL_IDX_REDIRECT_NODE_FFE] ==
        UNC_VF_VALID) {
       if (val->valid[UPLL_IDX_REDIRECT_PORT_FFE] ==
           UNC_VF_VALID) {
         result_code = VerifyRedirectDestination(ckval, dmi, UPLL_DT_IMPORT);
         if (result_code != UPLL_RC_SUCCESS) {
            DELETE_IF_NOT_NULL(tmp_ckval);
            UPLL_LOG_DEBUG(
                "redirect-destination node/interface doesn't exists");
            return UPLL_RC_ERR_MERGE_CONFLICT;
          }
        }
    }
    ckval = ckval->get_next_cfg_key_val();
  }
  DELETE_IF_NOT_NULL(tmp_ckval);
  UPLL_LOG_DEBUG("MergeValidate result code (%d)", result_code);
  return result_code;
}

upll_rc_t VbrFlowFilterEntryMoMgr::UpdateAuditConfigStatus(
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

  UPLL_LOG_DEBUG("AuditUpdate Config Status Information %d", result_code);
  return result_code;
}

upll_rc_t VbrFlowFilterEntryMoMgr::UpdateMo(IpcReqRespHeader *req,
    ConfigKeyVal *ikey, DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  ConfigKeyVal* okey = NULL;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  uint8_t *ctrlr_id = NULL;

  if (NULL == ikey && NULL == req) {
    UPLL_LOG_DEBUG("Both Request and Input Key are Null");
    return UPLL_RC_ERR_GENERIC;
  }

  result_code = ValidateMessage(req, ikey);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("Validate Message Failed %d", result_code);
    return result_code;
  }

  result_code = ValidateValFlowfilterEntry(req, ikey);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("ValidateValFlowfilterEntry Failed %d", result_code);
    return result_code;
  }

  result_code = ValidateAttribute(ikey, dmi, req);
  if (UPLL_RC_SUCCESS != result_code) {
    return result_code;
  }
  TcConfigMode config_mode = TC_CONFIG_INVALID;
  std::string vtn_name = "";
  result_code = GetConfigModeInfo(req, config_mode, vtn_name);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetConfigMode failed");
    return result_code;
  }

  FlowListMoMgr *flowlist_mgr = reinterpret_cast<FlowListMoMgr *>
      (const_cast<MoManager *> (GetMoManager(UNC_KT_FLOWLIST)));
  val_flowfilter_entry_t *flowfilter_val =
    reinterpret_cast<val_flowfilter_entry_t *> (GetVal(ikey));
  // Get ctrlr_id to do Capa check
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
      delete okey;
      UPLL_LOG_DEBUG("ReadConfigDB failed %d", result_code);
      return result_code;
    }
    if (IsAllAttrInvalid(flowfilter_val)) {
      UPLL_LOG_INFO("No attributes to be updated");
      DELETE_IF_NOT_NULL(okey);
      return UPLL_RC_SUCCESS;
    }
    GET_USER_DATA_CTRLR(okey, ctrlr_id);

    UPLL_LOG_DEBUG("Calling validate Capability For UpdateMo");
    result_code = ValidateCapability(req, ikey,
                 reinterpret_cast<const char*>(ctrlr_id));
    if (result_code != UPLL_RC_SUCCESS) {
      delete okey;
      UPLL_LOG_DEBUG("validate Capability Failed %d", result_code);
      return result_code;
    }

    std::string temp_vtn_name;
    if (TC_CONFIG_VTN == config_mode) {
      temp_vtn_name = vtn_name;
    } else {
      key_vbr_flowfilter_entry_t *temp_key = reinterpret_cast<
          key_vbr_flowfilter_entry_t *>(ikey->get_key());
      temp_vtn_name.append(reinterpret_cast<const char*>(
          temp_key->flowfilter_key.vbr_key.vtn_key.vtn_name));
  UPLL_LOG_DEBUG(" temp_vtn in vbr ff %s", reinterpret_cast<const char*>(
          temp_key->flowfilter_key.vbr_key.vtn_key.vtn_name));
    }
  if (UNC_VF_VALID == flowfilter_val->valid[UPLL_IDX_FLOWLIST_NAME_FFE] ||
      UNC_VF_VALID_NO_VALUE == flowfilter_val->
      valid[UPLL_IDX_FLOWLIST_NAME_FFE]) {
    val_flowfilter_entry_t *temp_ffe_val = reinterpret_cast
      <val_flowfilter_entry_t *>(GetVal(okey));
    UPLL_LOG_DEBUG("flowlist name %s", flowfilter_val->flowlist_name);
    if (UNC_VF_VALID == flowfilter_val->valid[UPLL_IDX_FLOWLIST_NAME_VFFE] &&
        UNC_VF_VALID  == temp_ffe_val->
        valid[UPLL_IDX_FLOWLIST_NAME_VFFE]) {
      UPLL_LOG_DEBUG("Update option 1");
      // Skip the record if given flowlist is already configured in DB
      if (strncmp(reinterpret_cast<char *>(flowfilter_val->flowlist_name),
              reinterpret_cast<char *>(temp_ffe_val->flowlist_name),
              (kMaxLenFlowListName + 1))) {
        result_code = flowlist_mgr->AddFlowListToController(
          reinterpret_cast<char *>(temp_ffe_val->flowlist_name), dmi,
          reinterpret_cast<char *> (ctrlr_id), req->datatype, UNC_OP_DELETE,
          config_mode, temp_vtn_name, false);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("AddFlowListToController failed %d", result_code);
          delete okey;
          return result_code;
        }
        result_code = flowlist_mgr->AddFlowListToController(
          reinterpret_cast<char *>(flowfilter_val->flowlist_name), dmi,
          reinterpret_cast<char *>(ctrlr_id), req->datatype, UNC_OP_CREATE,
          config_mode, temp_vtn_name, false);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("AddFlowListToController failed %d", result_code);
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
          reinterpret_cast<char *>(ctrlr_id), req->datatype, UNC_OP_DELETE,
          config_mode, temp_vtn_name, false);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("AddFlowListToController failed %d", result_code);
        delete okey;
        return result_code;
      }
    }
  }
  // N/w monitor
  DbSubOp dbop1 = {kOpNotRead, kOpMatchNone, kOpInOutFlag};
  result_code = UpdateConfigDB(ikey, req->datatype, UNC_OP_UPDATE, dmi,
                               &dbop1, config_mode, vtn_name, MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Unable to Update the CandidateDB");
    return result_code;
  }
  delete okey;
  return result_code;
}

upll_rc_t VbrFlowFilterEntryMoMgr::IsReferenced(IpcReqRespHeader *req,
                                                ConfigKeyVal *ikey,
                                                DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  if (NULL == ikey) return UPLL_RC_ERR_GENERIC;
  DbSubOp dbop = { kOpReadExist, kOpMatchNone, kOpInOutNone };
  // Check the VBR FlowFilter Entry object existence
  result_code = UpdateConfigDB(ikey, req->datatype, UNC_OP_READ, dmi,
                               &dbop, MAINTBL);
  return result_code;
}

upll_rc_t VbrFlowFilterEntryMoMgr::ReadMo(IpcReqRespHeader *req,
                                          ConfigKeyVal *ikey,
                                          DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  ConfigKeyVal *dup_key = NULL, *l_key = NULL;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  controller_domain ctrlr_dom;
  DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutCtrlr|kOpInOutDomain  };

  result_code = ValidateMessage(req, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    return result_code;
    UPLL_LOG_DEBUG(" ValidateMessage in Read failed result(%d)", result_code);
  }

  memset(&ctrlr_dom, 0, sizeof(controller_domain));
  switch (req->datatype) {
    // Retrieving config information
    case UPLL_DT_CANDIDATE:  // MIXED ENUMS
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
          UPLL_LOG_DEBUG("DupConfigKeyVal fail in ReadMo for dup_key");
          return result_code;
        }

        result_code = ReadConfigDB(dup_key, req->datatype, UNC_OP_READ,
                                   dbop, dmi, MAINTBL);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("ReadConfigDB  fail in ReadMo for dup_key");
          delete dup_key;
          return result_code;
        }
        result_code =  DupConfigKeyVal(l_key, ikey, MAINTBL);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("DupConfigKeyVal fail in ReadMo for l_key");
          return result_code;
        }
        GET_USER_DATA_CTRLR_DOMAIN(dup_key, ctrlr_dom);
        SET_USER_DATA_CTRLR_DOMAIN(l_key, ctrlr_dom);
        // Adding Capacheck
        result_code = ValidateCapability(req, ikey,
                   reinterpret_cast<const char *>(ctrlr_dom.ctrlr));
        if (result_code != UPLL_RC_SUCCESS) {
          delete dup_key;
          delete l_key;
          UPLL_LOG_DEBUG("validate Capability Failed %d", result_code);
          return result_code;
        }

        // 1.Getting renamed name if renamed
        result_code = GetRenamedControllerKey(l_key, req->datatype,
                                              dmi, &ctrlr_dom);
        if (result_code != UPLL_RC_SUCCESS) {
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
                    PFCDRIVER_SERVICE_NAME, PFCDRIVER_SVID_LOGICAL, &ipc_req,
                    true, &ipc_resp)) {
          UPLL_LOG_DEBUG("SendReqToDriver failed for Key %d controller %s",
                         l_key->get_key_type(),
                         reinterpret_cast<char *>(ctrlr_dom.ctrlr));
          DELETE_IF_NOT_NULL(l_key);
          DELETE_IF_NOT_NULL(ipc_resp.ckv_data);
          return ipc_resp.header.result_code;
        }

        if (ipc_resp.header.result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("Driver response for Key %d controller %s result %d",
                        l_key->get_key_type(), ctrlr_dom.ctrlr,
                        ipc_resp.header.result_code);
          DELETE_IF_NOT_NULL(l_key);
          DELETE_IF_NOT_NULL(ipc_resp.ckv_data);
          return ipc_resp.header.result_code;
        }
        ConfigKeyVal *okey = NULL;
        result_code = ConstructReadDetailResponse(dup_key,
                                                  ipc_resp.ckv_data,
                                                  ctrlr_dom,
                                                  dmi ,
                                                  &okey);

        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("ConstructReadDetailResponse error code (%d)",
                         result_code);
          DELETE_IF_NOT_NULL(l_key);
          DELETE_IF_NOT_NULL(ipc_resp.ckv_data);
          return result_code;
        } else {
          if (okey != NULL) {
            ikey->ResetWith(okey);
            DELETE_IF_NOT_NULL(okey);
          }
        }
        DELETE_IF_NOT_NULL(ipc_resp.ckv_data);
        DELETE_IF_NOT_NULL(dup_key);
        DELETE_IF_NOT_NULL(l_key);
      }
      break;
    default:
      UPLL_LOG_DEBUG("Operation Not Allowed");
      result_code = UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT;
  }
  return result_code;
}

upll_rc_t VbrFlowFilterEntryMoMgr::ReadSiblingMo(IpcReqRespHeader *req,
                                                 ConfigKeyVal *ikey,
                                                 bool begin,
                                                 DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  controller_domain ctrlr_dom;
  ConfigKeyVal  *l_key = NULL, *tctrl_key = NULL;
  ConfigKeyVal *okey =NULL, *tmp_key = NULL;

  result_code = ValidateMessage(req, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_TRACE("validate Message Fialed %d", result_code);
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
      } else if (req->datatype == UPLL_DT_STATE &&
                 (req->option1 == UNC_OPT1_DETAIL ||
                  req->option2 == UNC_OPT2_NONE)) {
        result_code =  DupConfigKeyVal(tctrl_key, ikey, MAINTBL);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("DupConfigKeyVal error (%d)", result_code);
          return result_code;
        }
        result_code = ReadInfoFromDB(req, tctrl_key, dmi, &ctrlr_dom);
        if (UPLL_RC_SUCCESS != result_code) {
          UPLL_LOG_DEBUG("ReadConfigDb failed for tctrl_key err code(%d)",
                         result_code);
          return result_code;
        }

        result_code =  DupConfigKeyVal(l_key, ikey, MAINTBL);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG(" DupConfigKeyVal failed for l_key%d ", result_code);
          return result_code;
        }

        GET_USER_DATA_CTRLR_DOMAIN(tctrl_key, ctrlr_dom);
        SET_USER_DATA_CTRLR_DOMAIN(l_key, ctrlr_dom);
        // Added CApaCheck
        result_code = ValidateCapability(req, ikey,
                   reinterpret_cast<const char *>(ctrlr_dom.ctrlr));
        if (result_code != UPLL_RC_SUCCESS) {
          delete l_key;
          UPLL_LOG_DEBUG("validate Capability Failed %d", result_code);
          return result_code;
        }

        // 1.Getting renamed name if renamed
        result_code = GetRenamedControllerKey(l_key, req->datatype, dmi,
                                              &ctrlr_dom);
        if (result_code != UPLL_RC_SUCCESS) {
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
          reinterpret_cast<key_vbr_flowfilter_entry_t*>
              (l_key->get_key())->flowfilter_key.direction =
              reinterpret_cast<key_vbr_flowfilter_entry_t*>
              (tmp_key->get_key())->flowfilter_key.direction;

          reinterpret_cast<key_vbr_flowfilter_entry_t*>
              (l_key->get_key())->sequence_num =
              reinterpret_cast<key_vbr_flowfilter_entry_t*>
              (tmp_key->get_key())->sequence_num;
          ipc_req.ckv_data = l_key;

          if (!IpcUtil::SendReqToDriver(
                      (const char *)ctrlr_dom.ctrlr,
                      reinterpret_cast<char *>(ctrlr_dom.domain),
                      PFCDRIVER_SERVICE_NAME, PFCDRIVER_SVID_LOGICAL, &ipc_req,
                      true, &ipc_resp)) {
            UPLL_LOG_DEBUG("SendReqToDriver failed for Key %d controller %s",
                           l_key->get_key_type(),
                           reinterpret_cast<char *>(ctrlr_dom.ctrlr));
            return ipc_resp.header.result_code;
          }

          if (ipc_resp.header.result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("Driver response for Key %d controller %s result %d",
                          l_key->get_key_type(), ctrlr_dom.ctrlr,
                          ipc_resp.header.result_code);
            return ipc_resp.header.result_code;
          }

          result_code = ConstructReadDetailResponse(tmp_key,
                                                    ipc_resp.ckv_data,
                                                    ctrlr_dom,
                                                     dmi,
                                                    &okey);

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

upll_rc_t VbrFlowFilterEntryMoMgr::RenameMo(IpcReqRespHeader *req,
    ConfigKeyVal *ikey, DalDmlIntf *dmi, const char *ctrlr_id) {
  UPLL_FUNC_TRACE;
  UPLL_LOG_DEBUG("Implementation Not supported for this KT.");
  return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_KT;
}

#if 0
upll_rc_t VbrFlowFilterEntryMoMgr::UpdateConfigStatus(ConfigKeyVal *key,
    unc_keytype_operation_t op, uint32_t driver_result, ConfigKeyVal *upd_key,
    DalDmlIntf *dmi, ConfigKeyVal *ctrlr_key) {
  // UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  val_flowfilter_entry_t *vbrflowfilter_entry_val = NULL;

  unc_keytype_configstatus_t cs_status =
      (driver_result == 0) ? UNC_CS_APPLIED : UNC_CS_NOT_APPLIED;
  vbrflowfilter_entry_val = reinterpret_cast<val_flowfilter_entry_t *>
                            (GetVal(key));
  if (vbrflowfilter_entry_val == NULL)
    return UPLL_RC_ERR_GENERIC;
  if (op == UNC_OP_CREATE) {
    if (vbrflowfilter_entry_val->cs_row_status != UNC_CS_NOT_SUPPORTED)
      vbrflowfilter_entry_val->cs_row_status = cs_status;
    for (unsigned int loop = 0;
        loop < sizeof(vbrflowfilter_entry_val->valid); ++loop) {
      if ((UNC_VF_VALID == vbrflowfilter_entry_val->valid[loop])
          || (UNC_VF_VALID_NO_VALUE == vbrflowfilter_entry_val->valid[loop]))
        if (vbrflowfilter_entry_val->cs_attr[loop] != UNC_CS_NOT_SUPPORTED)
          vbrflowfilter_entry_val->cs_attr[loop] =
              vbrflowfilter_entry_val->cs_row_status;
    }
  } else if (op == UNC_OP_UPDATE) {
    void *fle_val1 = GetVal(key);
    void *fle_val2 = GetVal(upd_key);
    CompareValidValue(fle_val1, fle_val2, false);
    for (unsigned int loop = 0;
        loop < sizeof(vbrflowfilter_entry_val->valid); ++loop) {
      if (vbrflowfilter_entry_val->cs_attr[loop] != UNC_CS_NOT_SUPPORTED)
        if ((UNC_VF_VALID == vbrflowfilter_entry_val->valid[loop])
            || (UNC_VF_VALID_NO_VALUE == vbrflowfilter_entry_val->valid[loop]))

          //  if (CompareValidValue(vbrflowfilter_entry_val, upd_key->GetVal())

          vbrflowfilter_entry_val->cs_attr[loop] =
              vbrflowfilter_entry_val->cs_row_status;
    }
  } else {
    UPLL_LOG_DEBUG("Operation Not Supported.");
    return UPLL_RC_ERR_GENERIC;
  }

  UPLL_LOG_DEBUG("Update Config Status Successfull.");
  return result_code;
}
#endif

upll_rc_t VbrFlowFilterEntryMoMgr::GetControllerId(ConfigKeyVal *ikey,
        DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *temp = NULL;
  ConfigVal *tmpval = NULL;
  MoMgrImpl *mgr =
      reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>(GetMoManager(
          UNC_KT_VBRIDGE)));
  if (NULL == mgr) {
    UPLL_LOG_DEBUG("mgr is NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  result_code = mgr->GetChildConfigKey(temp, ikey);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetChildConfigKey Failed %d", result_code);
    return result_code;
  }
  val_vbr *vbr_val = reinterpret_cast<val_vbr *>
                  (ConfigKeyVal::Malloc(sizeof(val_vbr)));

  tmpval = new ConfigVal(IpctSt::kIpcStValVbr, vbr_val);
  temp->SetCfgVal(tmpval);
  DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutCtrlr};
  result_code = mgr->ReadConfigDB(temp, UPLL_DT_CANDIDATE, UNC_OP_READ,
                                    dbop, dmi, MAINTBL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("ReadConfigDB failed %d", result_code);
    DELETE_IF_NOT_NULL(temp);
    return result_code;
  }
  val_vbr *ival = reinterpret_cast<val_vbr *>(GetVal(temp));
  if (!ival) {
    DELETE_IF_NOT_NULL(temp);
    return UPLL_RC_ERR_GENERIC;
  }
  SET_USER_DATA_CTRLR(ikey, ival->controller_id);
  UPLL_LOG_DEBUG("GetController Id is Successfull");
  DELETE_IF_NOT_NULL(temp);
  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrFlowFilterEntryMoMgr::CopyToConfigKey(ConfigKeyVal *&okey,
                                              ConfigKeyVal *ikey) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  if (!ikey || !(ikey->get_key())) {
    UPLL_LOG_DEBUG("Input Key Not Valid");
    return UPLL_RC_ERR_GENERIC;
  }
  key_rename_vnode_info *key_rename = NULL;
  key_rename = reinterpret_cast<key_rename_vnode_info *> (ikey->get_key());
  key_vbr_flowfilter_entry_t * key_vbr_ff_entry =
         reinterpret_cast<key_vbr_flowfilter_entry_t *>
         (ConfigKeyVal::Malloc(sizeof(key_vbr_flowfilter_entry_t)));

  if (!strlen(reinterpret_cast<char *> (key_rename->old_unc_vtn_name))) {
    UPLL_LOG_DEBUG("old_unc_vtn_name NULL");
    if (key_vbr_ff_entry) free(key_vbr_ff_entry);
      return UPLL_RC_ERR_GENERIC;
  }
  uuu::upll_strncpy(key_vbr_ff_entry->flowfilter_key.vbr_key.vtn_key.vtn_name,
                    key_rename->old_unc_vtn_name,
                    (kMaxLenVtnName + 1));

  if (ikey->get_key_type() == UNC_KT_VBRIDGE) {
    if (!strlen(reinterpret_cast<char *> (key_rename->old_unc_vnode_name))) {
      UPLL_LOG_DEBUG("old_unc_vnode_name NULL");
      free(key_vbr_ff_entry);
      return UPLL_RC_ERR_GENERIC;
    }

    uuu::upll_strncpy(key_vbr_ff_entry->flowfilter_key.vbr_key.vbridge_name,
                       key_rename->old_unc_vnode_name,
                       (kMaxLenVnodeName + 1));
  } else {
    if (!strlen(reinterpret_cast<char *>(key_rename->new_unc_vnode_name))) {
      UPLL_LOG_DEBUG("new_unc_vnode_name NULL");
      free(key_vbr_ff_entry);
      return UPLL_RC_ERR_GENERIC;
    }
    uuu::upll_strncpy(key_vbr_ff_entry->flowfilter_key.vbr_key.vbridge_name,
                      key_rename->new_unc_vnode_name, (kMaxLenVnodeName + 1));
  }

  okey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY,
                          IpctSt::kIpcStKeyVbrFlowfilterEntry,
                          key_vbr_ff_entry, NULL);
  if (!okey) {
    UPLL_LOG_DEBUG("CopyToConfigKey okey NULL");
    free(key_vbr_ff_entry);
    return UPLL_RC_ERR_GENERIC;
  }
  return result_code;
}

upll_rc_t VbrFlowFilterEntryMoMgr::UpdateVnodeVal(
    ConfigKeyVal *ikey,
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
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  string vtn_name = "";

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
      UPLL_LOG_DEBUG("key_rename->old_flowlist_name NULL");
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
      UPLL_LOG_DEBUG("key_rename->old_unc_vnode_name NULL");
      DELETE_IF_NOT_NULL(okey);
      return UPLL_RC_ERR_GENERIC;
    }
    key_vbr_flowfilter_entry_t *vbr_ffe_key =
       reinterpret_cast<key_vbr_flowfilter_entry_t*>(okey->get_key());
    uuu::upll_strncpy(vbr_ffe_key->flowfilter_key.vbr_key.vtn_key.vtn_name,
                      key_rename->new_unc_vtn_name, (kMaxLenVtnName + 1));
    uuu::upll_strncpy(val_ff_entry->redirect_node,
      key_rename->old_unc_vnode_name,
      (kMaxLenVnodeName + 1));
    val_ff_entry->valid[UPLL_IDX_REDIRECT_NODE_FFE] = UNC_VF_VALID;
    UPLL_LOG_DEBUG("valid and vbridge name (%d) (%s)",
                   val_ff_entry->valid[UPLL_IDX_REDIRECT_NODE_FFE],
                   val_ff_entry->redirect_node);
  }

  // ConfigVal *cval = new ConfigVal(IpctSt::kIpcStValFlowfilterEntry,
  // val_ff_entry);
  // okey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY,
  // IpctSt::kIpcStKeyVbrFlowfilterEntry, NULL, cval);
  // if (!okey) {
  // UPLL_LOG_DEBUG("CopyToConfigKey okey  NULL");
  // if (val_ff_entry) free(val_ff_entry);
  // return UPLL_RC_ERR_GENERIC;
  // }

  DbSubOp dbop = { kOpReadMultiple, kOpMatchNone,
    kOpInOutCtrlr|kOpInOutDomain|kOpInOutFlag };

  // Read the record of key structure and old flowlist name in maintbl
  result_code = ReadConfigDB(okey, data_type, UNC_OP_READ, dbop, dmi,
                             MAINTBL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG(" pran ReadConfigDB failed ");
    DELETE_IF_NOT_NULL(okey);
    return result_code;
  }
  GET_USER_DATA_FLAGS(okey, rename);
  ConfigKeyVal *first_ckv = okey;
  while (okey != NULL) {
    result_code = GetChildConfigKey(kval, okey);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("GetChildConfigKey kval NULL");
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
      UPLL_LOG_DEBUG("new_flowlist_name NULL");
      if (val_ff_entry_new) free(val_ff_entry_new);
      DELETE_IF_NOT_NULL(first_ckv);
      DELETE_IF_NOT_NULL(kval);
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
      UPLL_LOG_DEBUG("new_unc_vnode_name NULL");
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
    result_code = GetControllerDomainID(okey, &ctrlr_dom, UPLL_DT_IMPORT, dmi);

    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Failed to Get the Controller Domain details, err:%d",
                     result_code);
    }
    ctrlr_dom.ctrlr = NULL;
    ctrlr_dom.domain = NULL;
    GET_USER_DATA_CTRLR_DOMAIN(okey, ctrlr_dom);
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
      UPLL_LOG_DEBUG(
          "Update The Existing record Err in vbrflowfilterentrytbl "
          "CANDIDATE DB(%d)", result_code);
      DELETE_IF_NOT_NULL(first_ckv);
      DELETE_IF_NOT_NULL(kval);
      return result_code;
    }
    DELETE_IF_NOT_NULL(kval);

    okey = okey->get_next_cfg_key_val();
  }
  DELETE_IF_NOT_NULL(first_ckv);
  UPLL_LOG_DEBUG("UpdateVnodeVal result_code (%d)", result_code);
  return result_code;
}

bool VbrFlowFilterEntryMoMgr::CompareValidValue(void *&val1, void *val2,
                                                bool copy_to_running) {
  UPLL_FUNC_TRACE;
  bool attr = true;
  val_flowfilter_entry_t *val_ff_entry1 =
      reinterpret_cast<val_flowfilter_entry_t *>(val1);
  val_flowfilter_entry_t *val_ff_entry2 =
      reinterpret_cast<val_flowfilter_entry_t *>(val2);

  for ( unsigned int loop = 0; loop < (sizeof(val_ff_entry1->valid)/
                             sizeof(val_ff_entry1->valid[0])) ; ++loop ) {
    if ( UNC_VF_INVALID == val_ff_entry1->valid[loop] &&
                UNC_VF_VALID == val_ff_entry2->valid[loop])
     val_ff_entry1->valid[loop] = UNC_VF_VALID_NO_VALUE;
  }

  if (val_ff_entry1->valid[UPLL_IDX_FLOWLIST_NAME_FFE] == UNC_VF_VALID
    && val_ff_entry2->valid[UPLL_IDX_FLOWLIST_NAME_FFE] == UNC_VF_VALID) {
    if (!strcmp(reinterpret_cast<char *>(val_ff_entry1->flowlist_name),
               reinterpret_cast<char *>(val_ff_entry2->flowlist_name)))
      val_ff_entry1->valid[UPLL_IDX_FLOWLIST_NAME_FFE] = UNC_VF_INVALID;
  }
  if (val_ff_entry1->valid[UPLL_IDX_ACTION_FFE] == UNC_VF_VALID &&
     val_ff_entry2->valid[UPLL_IDX_ACTION_FFE] == UNC_VF_VALID) {
    if (val_ff_entry1->action == val_ff_entry2->action)
      val_ff_entry1->valid[UPLL_IDX_ACTION_FFE] = UNC_VF_INVALID;
  }
  if (val_ff_entry1->valid[UPLL_IDX_REDIRECT_NODE_FFE] == UNC_VF_VALID
     && val_ff_entry2->valid[UPLL_IDX_REDIRECT_NODE_FFE] == UNC_VF_VALID) {
    if (!strcmp(reinterpret_cast<char *>(val_ff_entry1->redirect_node),
              reinterpret_cast<const char *>(val_ff_entry2->redirect_node)))
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
  for (unsigned int loop = 0;
      loop < sizeof(val_ff_entry1->valid)/ sizeof(uint8_t); ++loop) {
    if ((UNC_VF_VALID == (uint8_t) val_ff_entry1->valid[loop]) ||
        (UNC_VF_VALID_NO_VALUE == (uint8_t) val_ff_entry1->valid[loop]))
      attr = false;
  }
  return attr;
}


upll_rc_t VbrFlowFilterEntryMoMgr::ConstructReadDetailResponse(
    ConfigKeyVal *ikey,
    ConfigKeyVal *drv_resp_ckv,
    controller_domain ctrlr_dom,
    DalDmlIntf *dmi ,
    ConfigKeyVal **okey) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *tmp_okey = NULL;
  ConfigVal *drv_resp_val = NULL;
  drv_resp_val =  drv_resp_ckv->get_cfg_val();
  result_code =  GetChildConfigKey(tmp_okey, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetChildConfigKey failed err code (%d)", result_code);
    return result_code;
  }
  while (drv_resp_val != NULL) {
    val_flowfilter_entry_t *val_entry = NULL;
    if (IpctSt::kIpcStValFlowfilterEntry ==
        drv_resp_val->get_st_num()) {
      UPLL_LOG_TRACE("Get the val struct");
      val_entry = reinterpret_cast< val_flowfilter_entry_t *>
          (drv_resp_val->get_val());
       // SetRedirectNodeAndPortinRead will Set the
       // redirect-direction in val_entry
       result_code =  SetRedirectNodeAndPortForRead(ikey ,
                                                   ctrlr_dom,
                                                   val_entry ,
                                                   dmi);
       if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("SetRedirectNodeAndPortForRead Fails - %d",
            result_code);
        DELETE_IF_NOT_NULL(tmp_okey);
        return result_code;
      }
    }
      drv_resp_val = drv_resp_val->get_next_cfg_val();
  }
  tmp_okey->AppendCfgVal(drv_resp_ckv->GetCfgValAndUnlink());
  if (*okey == NULL) {
    *okey = tmp_okey;
  } else {
    (*okey)->AppendCfgKeyVal(tmp_okey);
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrFlowFilterEntryMoMgr::GetParentConfigKey(ConfigKeyVal *&okey,
                                                      ConfigKeyVal *ikey) {
  UPLL_FUNC_TRACE;

  if (!ikey) {
    UPLL_LOG_DEBUG(" Input Key is NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  DELETE_IF_NOT_NULL(okey);
  unc_key_type_t ikey_type = ikey->get_key_type();
  if (ikey_type != UNC_KT_VBR_FLOWFILTER_ENTRY) {
    UPLL_LOG_DEBUG(" Invalid key type received. Key type - %d", ikey_type);
    return UPLL_RC_ERR_GENERIC;
  }

  key_vbr_flowfilter_entry_t *pkey =
      reinterpret_cast<key_vbr_flowfilter_entry_t*>(ikey->get_key());
  if (!pkey) {
    UPLL_LOG_DEBUG(" Input vbr flow filter entry key is NULL ");
    return UPLL_RC_ERR_GENERIC;
  }

  key_vbr_flowfilter_t *vbr_ff_key = reinterpret_cast<key_vbr_flowfilter_t*>
      (ConfigKeyVal::Malloc(sizeof(key_vbr_flowfilter_t)));

  uuu::upll_strncpy(vbr_ff_key->vbr_key.vtn_key.vtn_name,
                    reinterpret_cast<key_vbr_flowfilter_entry_t *>
                    (pkey)->flowfilter_key.vbr_key.vtn_key.vtn_name,
                    kMaxLenVtnName + 1);
  uuu::upll_strncpy(vbr_ff_key->vbr_key.vbridge_name,
                    reinterpret_cast<key_vbr_flowfilter_entry_t *>
                    (pkey)->flowfilter_key.vbr_key.vbridge_name,
                    (kMaxLenVnodeName + 1));
  vbr_ff_key->direction = reinterpret_cast<key_vbr_flowfilter_entry_t *>
      (pkey)->flowfilter_key.direction;
  okey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER,
                          IpctSt::kIpcStKeyVbrFlowfilter, vbr_ff_key, NULL);
  SET_USER_DATA(okey, ikey);
  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrFlowFilterEntryMoMgr::DeleteChildrenPOM(
          ConfigKeyVal *ikey, upll_keytype_datatype_t dt_type,
          DalDmlIntf *dmi, TcConfigMode config_mode,
          string vtn_name) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_ERR_GENERIC;
  int nattr = 0;
  BindInfo *binfo = NULL;
  DalCursor *dal_cursor_handle = NULL;
  string query_string;
  unc_key_type_t deletedkt;

  if (NULL == ikey || NULL == dmi) {
    UPLL_LOG_DEBUG("Request & InputKey are not are not Valid %d", result_code);
    return result_code;
  }

  key_vbr_flowfilter_entry_t *pkey =
      reinterpret_cast<key_vbr_flowfilter_entry_t*>(ikey->get_key());
  if (!pkey) {
    UPLL_LOG_DEBUG(" Input vbr flow filter entry key is NULL ");
    return UPLL_RC_ERR_GENERIC;
  }

  if (((pkey->flowfilter_key.vbr_key.vbridge_name)[0]) == '\0') {
    deletedkt = UNC_KT_VTN;
  } else if (pkey->flowfilter_key.direction == 0xFE) {
    deletedkt = UNC_KT_VBRIDGE;
  } else {
    deletedkt = UNC_KT_VBR_FLOWFILTER;
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

  ConfigKeyVal *vbr_ffe_ckv  = NULL;
  result_code = GetChildConfigKey(vbr_ffe_ckv, NULL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetChildConfigKey failed");
    return result_code;
  }
  ConfigVal* vbr_ffe_cv = NULL;
  result_code = AllocVal(vbr_ffe_cv, UPLL_DT_CANDIDATE, MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Error in AllocVal");
    DELETE_IF_NOT_NULL(vbr_ffe_ckv);
    return result_code;
  }
  vbr_ffe_ckv->SetCfgVal(vbr_ffe_cv);
  GET_USER_DATA(vbr_ffe_ckv);
  uint32_t count = 0;
  void *tkey = vbr_ffe_ckv->get_key();
  void *p = reinterpret_cast<void *>(reinterpret_cast<char *>(tkey) +
                                     binfo[0].offset);
  dal_bind_info.BindOutput(binfo[0].index, binfo[0].app_data_type,
                           binfo[0].array_size, p);
  tkey = vbr_ffe_ckv->get_user_data();
  p = reinterpret_cast<void *>(reinterpret_cast<char *>(tkey) +
                               binfo[4].offset);
  dal_bind_info.BindOutput(binfo[4].index, binfo[4].app_data_type,
                           binfo[4].array_size, p);
  tkey = vbr_ffe_ckv->get_cfg_val()->get_val();
  p = reinterpret_cast<void *>(reinterpret_cast<char *>(tkey) +
                               binfo[6].offset);
  dal_bind_info.BindOutput(binfo[6].index, binfo[6].app_data_type,
                           binfo[6].array_size, p);

  dal_bind_info.BindOutput(uud::schema::DAL_COL_STD_INTEGER,
                           uud::kDalUint32, 1, &count);

  result_code = DalToUpllResCode(dmi->ExecuteAppQueryMultipleRecords(
                 query_string, 0, &dal_bind_info,
                 &dal_cursor_handle));
  while (result_code == UPLL_RC_SUCCESS) {
    result_code = DalToUpllResCode(dmi->GetNextRecord(dal_cursor_handle));
    if (UPLL_RC_SUCCESS == result_code) {
      // Call function to update refcount in scratch table
      key_vbr_flowfilter_entry_t *vbr_ffe_key =
          reinterpret_cast<key_vbr_flowfilter_entry_t *>
          (vbr_ffe_ckv->get_key());
      vtn_name = reinterpret_cast<const char *>
                 (vbr_ffe_key->flowfilter_key.vbr_key.vtn_key.vtn_name);
      val_flowfilter_entry_t *vbr_ffe_val =
           reinterpret_cast<val_flowfilter_entry_t *>(GetVal(vbr_ffe_ckv));
      uint8_t *ctrlr_id = NULL;
      GET_USER_DATA_CTRLR(vbr_ffe_ckv, ctrlr_id);
      FlowListMoMgr *fl_mgr =
          reinterpret_cast<FlowListMoMgr *>
          (const_cast<MoManager *>(GetMoManager(UNC_KT_FLOWLIST)));
      if (NULL == fl_mgr) {
        UPLL_LOG_DEBUG("fl_mgr is NULL");
        DELETE_IF_NOT_NULL(vbr_ffe_ckv);
        return UPLL_RC_ERR_GENERIC;
      }
      ConfigKeyVal *fl_ckv  = NULL;
      result_code = fl_mgr->GetChildConfigKey(fl_ckv, NULL);
      if (UPLL_RC_SUCCESS != result_code) {
        UPLL_LOG_DEBUG("GetChildConfigKey failed");
        DELETE_IF_NOT_NULL(vbr_ffe_ckv);
        return result_code;
      }
      key_flowlist_t *fl_key = reinterpret_cast<key_flowlist_t *>
          (fl_ckv->get_key());
      uuu::upll_strncpy(fl_key->flowlist_name,
                        vbr_ffe_val->flowlist_name,
                        (kMaxLenFlowListName+1));
      SET_USER_DATA_CTRLR(fl_ckv, ctrlr_id);
      result_code = fl_mgr->UpdateRefCountInScratchTbl(fl_ckv, dmi,
                                                       dt_type, UNC_OP_DELETE,
                                                       config_mode, vtn_name,
                                                       count);
      if (result_code != UPLL_RC_SUCCESS &&
          result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
        UPLL_LOG_DEBUG("UpdateRefCountInScratchTbl returned %d", result_code);
        DELETE_IF_NOT_NULL(vbr_ffe_ckv);
        DELETE_IF_NOT_NULL(fl_ckv);
        return result_code;
      } else if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
        result_code = fl_mgr->InsertRecInScratchTbl(fl_ckv, dmi, dt_type,
                                                    UNC_OP_DELETE,
                                                    config_mode, vtn_name,
                                                    count);
        if (UPLL_RC_SUCCESS != result_code) {
          UPLL_LOG_DEBUG("InsertRecInScratchTbl failed %d", result_code);
          DELETE_IF_NOT_NULL(vbr_ffe_ckv);
          DELETE_IF_NOT_NULL(fl_ckv);
          return result_code;
        }
      }
      DELETE_IF_NOT_NULL(fl_ckv);
    } else if (UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
      UPLL_LOG_DEBUG("GetNextRecord failed");
      DELETE_IF_NOT_NULL(vbr_ffe_ckv);
      dmi->CloseCursor(dal_cursor_handle, false);
      return result_code;
    }
  }
  if (result_code != UPLL_RC_SUCCESS &&
      result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    UPLL_LOG_DEBUG("ExecuteAppQueryMultipleRecords failed");
    DELETE_IF_NOT_NULL(vbr_ffe_ckv);
    dmi->CloseCursor(dal_cursor_handle, false);
    return result_code;
  }
  DELETE_IF_NOT_NULL(vbr_ffe_ckv);
  /*
  DbSubOp dbop = { kOpReadMultiple, kOpMatchNone, kOpInOutCtrlr };
  result_code = ReadConfigDB(temp_okey, UPLL_DT_CANDIDATE,
                             UNC_OP_READ, dbop, dmi, MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
      UPLL_LOG_DEBUG("UPLL_RC_ERR_NO_SUCH_INSTANCE");
      result_code = UPLL_RC_SUCCESS;
    }
    UPLL_LOG_DEBUG("Unable to read configuration from CandidateDb");
    DELETE_IF_NOT_NULL(temp_okey);
    return result_code;
  }
  ConfigKeyVal *okey = temp_okey;
  while (NULL != okey) {
    GET_USER_DATA_CTRLR(okey, ctrlr_id);
    val_flowfilter_entry_t *flowfilter_val =
        reinterpret_cast<val_flowfilter_entry_t *> (GetVal(okey));
    if (flowfilter_val->valid[UPLL_IDX_FLOWLIST_NAME_FFE] == UNC_VF_VALID) {
      FlowListMoMgr *mgr = reinterpret_cast<FlowListMoMgr *>
          (const_cast<MoManager *> (GetMoManager(UNC_KT_FLOWLIST)));
      if (NULL == mgr) {
        UPLL_LOG_DEBUG("mgr is NULL");
        DELETE_IF_NOT_NULL(temp_okey);
        return result_code;
      }

      std::string temp_vtn_name;
      if (TC_CONFIG_VTN == config_mode) {
        temp_vtn_name = vtn_name;
      } else {
        key_vbr_flowfilter_entry_t *temp_key = reinterpret_cast<
            key_vbr_flowfilter_entry_t *>(ikey->get_key());
        temp_vtn_name = reinterpret_cast<const char*>(
            temp_key->flowfilter_key.vbr_key.vtn_key.vtn_name);
      }
      result_code = mgr->AddFlowListToController(
          reinterpret_cast<char *>(flowfilter_val->flowlist_name), dmi,
          reinterpret_cast<char *>(ctrlr_id), dt_type, UNC_OP_DELETE,
          config_mode, temp_vtn_name);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG(" Send delete request to flowlist failed. err code(%d)",
                       result_code);
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
    return result_code;
  }
  result_code = UpdateConfigDB(temp_ikey, dt_type, UNC_OP_DELETE, dmi,
                               config_mode, vtn_name, MAINTBL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("UpdateConfigDB failed %d", result_code);
    DELETE_IF_NOT_NULL(temp_ikey);
    dmi->CloseCursor(dal_cursor_handle, false);
    return result_code;
  }
  dmi->CloseCursor(dal_cursor_handle, false);
  DELETE_IF_NOT_NULL(temp_ikey);
  UPLL_LOG_DEBUG("DeleteMo Operation Done %d", result_code);
  return result_code;
}

upll_rc_t VbrFlowFilterEntryMoMgr::SetValidAudit(ConfigKeyVal *&ikey) {
  UPLL_FUNC_TRACE;
  val_flowfilter_entry_t *val = reinterpret_cast
      <val_flowfilter_entry_t *>(GetVal(ikey));
  if (NULL == val) {
    UPLL_LOG_DEBUG("val is NULL");
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


upll_rc_t VbrFlowFilterEntryMoMgr::UpdateConfigStatus(ConfigKeyVal *vbr_key,
                                       unc_keytype_operation_t op,
                                       uint32_t driver_result,
                                       ConfigKeyVal *upd_key,
                                       DalDmlIntf *dmi,
                                       ConfigKeyVal *ctrlr_key) {
  UPLL_FUNC_TRACE;
  ConfigKeyVal * vbr_ffe_run_key = NULL;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  val_flowfilter_entry_t *vbr_ffe_val = NULL, *val_main = NULL;

  unc_keytype_configstatus_t cs_status =
      (driver_result == UPLL_RC_SUCCESS) ? UNC_CS_APPLIED : UNC_CS_NOT_APPLIED;
  vbr_ffe_val = reinterpret_cast<val_flowfilter_entry_t *>(GetVal(vbr_key));
  if (vbr_ffe_val == NULL) return UPLL_RC_ERR_GENERIC;
  if (op == UNC_OP_CREATE) {
    vbr_ffe_val->cs_row_status = cs_status;
  } else if (op == UNC_OP_UPDATE) {
    // For Reading The controller table for config status
    result_code = GetChildConfigKey(vbr_ffe_run_key, vbr_key);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("GetChildConfigKey is failed resultcode=%d",
                           result_code);
      return result_code;
    }
    DbSubOp dbop_maintbl = { kOpReadSingle, kOpMatchNone,
                                          kOpInOutFlag |kOpInOutCs };
    result_code = ReadConfigDB(vbr_ffe_run_key, UPLL_DT_RUNNING  ,
                                     UNC_OP_READ, dbop_maintbl, dmi, MAINTBL);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Unable to read configuration from RunningDb");
      DELETE_IF_NOT_NULL(vbr_ffe_run_key);
      return result_code;
    }
    val_main = reinterpret_cast
           <val_flowfilter_entry_t *>(GetVal(vbr_ffe_run_key));
    for (unsigned int loop = 0; loop < sizeof(val_main->valid)/
           sizeof(val_main->valid[0]); ++loop) {
      vbr_ffe_val->cs_attr[loop] = val_main->cs_attr[loop];
    }
    void *vbrffeval = reinterpret_cast<void *>(vbr_ffe_val);
    CompareValidValue(vbrffeval, GetVal(upd_key), true);
  } else {
    return UPLL_RC_ERR_GENERIC;
  }
  UPLL_LOG_TRACE("%s", (vbr_key->ToStrAll()).c_str());
  val_flowfilter_entry_t *vbr_ffe_val2 =
    reinterpret_cast<val_flowfilter_entry_t *>(GetVal(upd_key));
  if (UNC_OP_UPDATE == op) {
    UPLL_LOG_TRACE("%s", (upd_key->ToStrAll()).c_str());
    vbr_ffe_val->cs_row_status = vbr_ffe_val2->cs_row_status;
  }
  for (unsigned int loop = 0;
    loop < sizeof(vbr_ffe_val->valid) /
       sizeof(vbr_ffe_val->valid[0]); ++loop) {
    /* Setting CS to the not supported attributes*/
    if (UNC_VF_NOT_SUPPORTED == vbr_ffe_val->valid[loop]) {
      vbr_ffe_val->cs_attr[loop] = UNC_CS_NOT_SUPPORTED;
    } else if ((UNC_VF_VALID == vbr_ffe_val->valid[loop])
          || (UNC_VF_VALID_NO_VALUE == vbr_ffe_val->valid[loop])) {
      vbr_ffe_val->cs_attr[loop] = cs_status;
    } else if ((UNC_VF_INVALID == vbr_ffe_val->valid[loop]) &&
               (UNC_OP_CREATE == op)) {
      vbr_ffe_val->cs_attr[loop] = UNC_CS_NOT_APPLIED;
    } else if ((UNC_VF_INVALID == vbr_ffe_val->valid[loop]) &&
               (UNC_OP_UPDATE == op)) {
      if (val_main->valid[loop] == UNC_VF_VALID) {
        if (cs_status == UNC_CS_APPLIED) {
          vbr_ffe_val->cs_attr[loop] = cs_status;
        }
      }
    } else if ((UNC_VF_VALID == vbr_ffe_val->valid[loop]) &&
       (UNC_OP_UPDATE == op)) {
      if (cs_status == UNC_CS_APPLIED) {
        vbr_ffe_val->cs_attr[loop] = UNC_CS_APPLIED;
      }
    }
    if ((vbr_ffe_val->valid[loop] == UNC_VF_VALID_NO_VALUE)
                     &&(UNC_OP_UPDATE == op)) {
      vbr_ffe_val->cs_attr[loop]  = UNC_CS_UNKNOWN;
    }
  }
  DELETE_IF_NOT_NULL(vbr_ffe_run_key);
  return result_code;
}

bool VbrFlowFilterEntryMoMgr::FilterAttributes(void *&val1,
                                          void *val2,
                                          bool copy_to_running,
                                          unc_keytype_operation_t op) {
  UPLL_FUNC_TRACE;
  if (op != UNC_OP_CREATE)
    return CompareValidValue(val1, val2, copy_to_running);
  return false;
}

upll_rc_t VbrFlowFilterEntryMoMgr::SetRenameFlag(ConfigKeyVal *ikey,
    DalDmlIntf *dmi,
    IpcReqRespHeader *req) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  val_flowfilter_entry_t *val_ffe = reinterpret_cast
    <val_flowfilter_entry_t *>(GetVal(ikey));
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
              UNC_KT_VBR_FLOWFILTER)));
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
    if ((UNC_VF_VALID == val_ffe->valid[UPLL_IDX_FLOWLIST_NAME_FFE]) &&
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
      }
      uint8_t fl_rename = 0;
      result_code = fl_mgr->IsRenamed(fl_ckv, req->datatype, dmi, fl_rename);
      if (UPLL_RC_SUCCESS != result_code) {
        UPLL_LOG_DEBUG("IsRenamed failed %d", result_code);
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
    result_code = IsRenamed(ikey, req->datatype, dmi, rename);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("IsRenamed failed %d", result_code);
      return result_code;
    }
    if (UNC_VF_VALID == val_ffe->valid[UPLL_IDX_FLOWLIST_NAME_FFE]) {
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
        rename |= FLOW_RENAME;
      } else {
        rename &= NO_FLOWLIST_RENAME;
        /* reset flag*/
      }
      DELETE_IF_NOT_NULL(fl_ckv);
    } else if (UNC_VF_VALID_NO_VALUE == val_ffe->valid
               [UPLL_IDX_FLOWLIST_NAME_FFE]) {
       // No rename flowlist value should be set
       rename &= NO_FLOWLIST_RENAME;
    }
    SET_USER_DATA_FLAGS(ikey, rename);
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrFlowFilterEntryMoMgr::GetFlowlistConfigKey(
        const char *flowlist_name, ConfigKeyVal *&okey,
        DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_ERR_GENERIC;
  MoMgrImpl *mgr =
      reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>(GetMoManager(
              UNC_KT_FLOWLIST)));
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

bool VbrFlowFilterEntryMoMgr::IsAllAttrInvalid(
        val_flowfilter_entry_t *val) {
  for ( unsigned int loop = 0;
      loop < sizeof(val->valid)/sizeof(val->valid[0]); ++loop ) {
    if (UNC_VF_INVALID != val->valid[loop])
      return false;
  }
  return true;
}
upll_rc_t VbrFlowFilterEntryMoMgr::AdaptValToDriver(
    ConfigKeyVal *ck_new,
    ConfigKeyVal *ck_old,
    unc_keytype_operation_t op,
    upll_keytype_datatype_t dt_type,
    unc_key_type_t keytype,
    DalDmlIntf *dmi,
    bool &not_send_to_drv,
    bool audit_update_phase) {
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  if ((audit_update_phase) && (op != UNC_OP_DELETE)) {
    result_code = PerformRedirectTranslationForAudit(ck_new,
        dmi,
        dt_type);
    if (UPLL_RC_SUCCESS != result_code) {
      return result_code;
    }
  } else {
    if (op != UNC_OP_DELETE)  {
      // Perform the Semantic check for NWM which is referred in the
      // flowfilter_entry. If the referred NWM is deleted then return
      // SEMANTIC ERROR
      result_code = PerformSemanticCheckForNWM(ck_new, dmi, dt_type);
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
      result_code =  VerifyRedirectDestination(ck_new,
          dmi,
          UPLL_DT_CANDIDATE);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("VerifyRedirectDestination  failed result code - %d",
            result_code);
        return result_code;
      }
    }
  }
  return result_code;
}

}  // namespace kt_momgr
}  // namespace upll
}  // namespace unc
