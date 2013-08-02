/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include "vbr_flowfilter_entry_momgr.hh"
#include "flowlist_momgr.hh"
#include "flowlist_entry_momgr.hh"
#include "vbr_momgr.hh"
#include "upll_validation.hh"
#include "upll_log.hh"
using unc::upll::ipc_util::IpcUtil;
namespace unc {
namespace upll {
namespace kt_momgr {

#define NUM_KEY_MAIN_TBL        6
#define NUM_KEY_RENAME_MAIN_TBL 5
#define VTN_RENAME_FLAG         0x01
#define VBR_RENAME_FLAG         0x10
#define FLOWLIST_RENAME_FLAG    0x04

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
  { uudst::vbr_flowfilter_entry::kDbiValidModifyDstMac, CFG_META_VAL,
    offsetof(val_flowfilter_entry_t, valid[4]),
    uud::kDalUint8, 1 },
  { uudst::vbr_flowfilter_entry::kDbiValidModifySrcMac, CFG_META_VAL,
    offsetof(val_flowfilter_entry_t, valid[5]),
    uud::kDalUint8, 1 },
  { uudst::vbr_flowfilter_entry::kDbiValidNwmName, CFG_META_VAL,
    offsetof(val_flowfilter_entry_t, valid[6]),
    uud::kDalUint8, 1 },
  { uudst::vbr_flowfilter_entry::kDbiValidDscp, CFG_META_VAL,
    offsetof(val_flowfilter_entry_t, valid[7]),
    uud::kDalUint8, 1 },
  { uudst::vbr_flowfilter_entry::kDbiValidPriority, CFG_META_VAL,
    offsetof(val_flowfilter_entry_t, valid[8]),
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
  { uudst::vbr_flowfilter_entry::kDbiCsModifyDstMac, CS_VAL,
    offsetof(val_flowfilter_entry_t, cs_attr[4]),
    uud::kDalUint8, 1 },
  { uudst::vbr_flowfilter_entry::kDbiCsModifySrcMac, CS_VAL,
    offsetof(val_flowfilter_entry_t, cs_attr[5]),
    uud::kDalUint8, 1 },
  { uudst::vbr_flowfilter_entry::kDbiCsNwmName, CS_VAL,
    offsetof(val_flowfilter_entry_t, cs_attr[6]),
    uud::kDalUint8, 1 },
  { uudst::vbr_flowfilter_entry::kDbiCsDscp, CS_VAL,
    offsetof(val_flowfilter_entry_t, cs_attr[7]),
    uud::kDalUint8, 1 },
  { uudst::vbr_flowfilter_entry::kDbiCsPriority, CS_VAL,
    offsetof(val_flowfilter_entry_t, cs_attr[8]),
    uud::kDalUint8, 1 }
};

BindInfo VbrFlowFilterEntryMoMgr::vbr_flowfilter_entry_maintbl_bind_info[] = {
  { uudst::vbr_flowfilter_entry::kDbiVtnName, CFG_MATCH_KEY, offsetof(
     key_vbr_flowfilter_entry_t, flowfilter_key.vbr_key.vtn_key.vtn_name),
     uud::kDalChar, kMaxLenVtnName + 1 },
  { uudst::vbr_flowfilter_entry::kDbiVbrName, CFG_MATCH_KEY, offsetof(
     key_vbr_flowfilter_entry_t, flowfilter_key.vbr_key.vbridge_name),
     uud::kDalChar, kMaxLenVnodeName + 1 },
  { uudst::vbr_flowfilter_entry::kDbiInputDirection, CFG_MATCH_KEY,
    offsetof(key_vbr_flowfilter_entry_t, flowfilter_key.direction),
    uud::kDalUint8, 1 },
  { uudst::vbr_flowfilter_entry::kDbiSequenceNum, CFG_MATCH_KEY,
    offsetof(key_vbr_flowfilter_entry_t, sequence_num),
     uud::kDalUint16, 1 },
  { uudst::vbr_flowfilter_entry::kDbiVtnName, CFG_INPUT_KEY, offsetof(
     key_rename_vnode_info_t, new_unc_vtn_name),
     uud::kDalChar, kMaxLenVtnName + 1 },
  { uudst::vbr_flowfilter_entry::kDbiVbrName, CFG_INPUT_KEY, offsetof(
     key_rename_vnode_info_t, new_unc_vnode_name),
     uud::kDalChar, kMaxLenVnodeName + 1 },
  { uudst::vbr_flowfilter_entry::kDbiFlags, CFG_INPUT_KEY, offsetof(
     key_user_data_t, flags),
     uud::kDalUint8, 1 }
};


BindInfo VbrFlowFilterEntryMoMgr::vbr_flowlist_rename_bind_info[] = {
  { uudst::vbr_flowfilter_entry::kDbiVtnName, CFG_MATCH_KEY, offsetof(
     key_vbr_flowfilter_entry_t, flowfilter_key.vbr_key.vtn_key.vtn_name),
     uud::kDalChar, kMaxLenVtnName + 1 },
  { uudst::vbr_flowfilter_entry::kDbiVbrName, CFG_MATCH_KEY, offsetof(
     key_vbr_flowfilter_entry_t, flowfilter_key.vbr_key.vbridge_name),
     uud::kDalChar, kMaxLenVnodeName + 1 },
  { uudst::vbr_flowfilter_entry::kDbiInputDirection, CFG_MATCH_KEY,
    offsetof(key_vbr_flowfilter_entry_t, flowfilter_key.direction),
    uud::kDalUint8, 1 },
  { uudst::vbr_flowfilter_entry::kDbiSequenceNum, CFG_MATCH_KEY, offsetof(
     key_vbr_flowfilter_entry_t, sequence_num),
     uud::kDalUint16, 1 },
  { uudst::vbr_flowfilter_entry::kDbiFlowlistName, CFG_INPUT_KEY, offsetof(
     key_rename_vnode_info_t, new_flowlist_name),
     uud::kDalChar, kMaxLenFlowListName + 1 },
  { uudst::vbr_flowfilter_entry::kDbiFlags, CFG_INPUT_KEY, offsetof(
     key_user_data_t, flags),
     uud::kDalUint8, 1 }
};

VbrFlowFilterEntryMoMgr::VbrFlowFilterEntryMoMgr() : MoMgrImpl() {
  UPLL_FUNC_TRACE;
  // Rename and ctrlr tables not required for this KT
  // setting max tables toto 1
  ntable = (MAX_MOMGR_TBLS);
  table = new Table *[ntable];

  // For Main Table
  table[MAINTBL] = new Table(uudst::kDbiVbrFlowFilterEntryTbl,
      UNC_KT_VBR_FLOWFILTER_ENTRY, vbr_flowfilterentry_bind_info,
      IpctSt::kIpcStKeyVbrFlowfilterEntry, IpctSt::kIpcStValFlowfilterEntry,
      uudst::vbr_flowfilter_entry::kDbiVbrFlowFilterEntryNumCols);

  table[RENAMETBL] = NULL;

  table[CTRLRTBL] = NULL;

  nchild = 0;
  child = NULL;
}

bool VbrFlowFilterEntryMoMgr::GetRenameKeyBindInfo(unc_key_type_t key_type,
                                    BindInfo *&binfo,
                                    int &nattr,
                                    MoMgrTables tbl ) {
  UPLL_FUNC_TRACE;
  if (UNC_KT_VBR_FLOWFILTER_ENTRY == key_type) {
  /* Main Table only update */
  if (MAINTBL == tbl) {
    nattr = NUM_KEY_MAIN_TBL;
    binfo = vbr_flowfilter_entry_maintbl_bind_info;
  } else {
    return PFC_FALSE;
  }
  }
  /* Check for Flowlist Rename*/
  if (UNC_KT_FLOWLIST == key_type) {
    nattr = NUM_KEY_RENAME_MAIN_TBL;
    binfo = vbr_flowlist_rename_bind_info;
  }

  UPLL_LOG_DEBUG("Successful Completeion");
  return PFC_TRUE;
}

bool VbrFlowFilterEntryMoMgr::IsValidKey(void *key, uint64_t index) {
  UPLL_FUNC_TRACE;
  bool ret_val = UPLL_RC_SUCCESS;
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
      if (!ValidateNumericRange(ff_key->flowfilter_key.direction,
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
    IpcReqRespHeader *req, ConfigKeyVal *key, DalDmlIntf *dmi) {
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

  upll_rc_t result_code = UPLL_RC_ERR_GENERIC;
  bool db_action_valid = false;
  bool db_action_redirect = false;

  if ((val_flowfilter_entry->valid[UPLL_IDX_ACTION_FFE]
       == UNC_VF_INVALID) && (req->operation == UNC_OP_UPDATE)) {
    /** Read key struct from ConfigKeyVal argument*/
    key_vbr_flowfilter_entry_t *key_vbr_flowfilter_entry =
        static_cast<key_vbr_flowfilter_entry_t*>(key->get_key());

    /** Check whether Action configured or not from DB */
    ConfigKeyVal *okey = NULL;

    result_code = GetChildConfigKey(okey, NULL);

    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("okey memory allocation failed- %d", result_code);
      return result_code;
    }

    key_vbr_flowfilter_entry_t *vbr_ffe_key =
        reinterpret_cast<key_vbr_flowfilter_entry_t *>(okey->get_key());

    /* copy key structure into okey key struct */
    uuu::upll_strncpy(
        vbr_ffe_key->flowfilter_key.vbr_key.vtn_key.vtn_name,
        key_vbr_flowfilter_entry->flowfilter_key.vbr_key.vtn_key.vtn_name,
        (kMaxLenVtnName+1));

    uuu::upll_strncpy(
        vbr_ffe_key->flowfilter_key.vbr_key.vbridge_name,
        key_vbr_flowfilter_entry->flowfilter_key.vbr_key.vbridge_name,
        (kMaxLenVnodeName+1));

    vbr_ffe_key->flowfilter_key.direction =
        key_vbr_flowfilter_entry->flowfilter_key.direction;
    vbr_ffe_key->sequence_num = key_vbr_flowfilter_entry->sequence_num;

    /* Check the action field configured in VBR_FLOWFILTER_ENTRY table*/
    DbSubOp dbop = {kOpReadSingle, kOpMatchNone, kOpInOutNone};
    result_code = ReadConfigDB(okey, req->datatype, UNC_OP_READ,
                               dbop, dmi, MAINTBL);

    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("ReadConfigDB is failed for VBR_FLOWFILTER_ENTRY");
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
  return ValidateFlowfilterEntryValue(val_flowfilter_entry, req->operation,
                                      db_action_valid, db_action_redirect);
}

upll_rc_t VbrFlowFilterEntryMoMgr::ValidateFlowfilterEntryValue(
    val_flowfilter_entry_t *val_flowfilter_entry, uint32_t operation,
    bool action_valid, bool action_redirect) {
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
      pfc_log_debug("FlowList name syntax check failed."
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
                                               operation, action_valid,
                                               action_redirect))
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
      pfc_log_debug("Nwm name syntax check failed."
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
    if ((val_flowfilter_entry->valid[UPLL_IDX_ACTION_FFE] ==
      UNC_VF_VALID) || action_valid) {
      if (!ValidateNumericRange(val_flowfilter_entry->dscp, kMinIPDscp,
                              kMaxIPDscp, true, true)) {
        UPLL_LOG_DEBUG("DSCP syntax validation failed");
        return UPLL_RC_ERR_CFG_SYNTAX;
      }
    } else {
       UPLL_LOG_DEBUG("Error DSCP is configured when Action is not filled");
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
    if ((val_flowfilter_entry->valid[UPLL_IDX_ACTION_FFE] ==
      UNC_VF_VALID) || action_valid) {
      if (!ValidateNumericRange(val_flowfilter_entry->priority,
                                kMinVlanPriority, kMaxVlanPriority, true,
                                true)) {
        UPLL_LOG_DEBUG("Priority syntax validation failed :Err Code - %d",
                      rt_code);
        return UPLL_RC_ERR_CFG_SYNTAX;
      }
    } else {
      UPLL_LOG_DEBUG("Error PRIORITY is configured when Action is not filled");
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
  upll_rc_t rt_code = UPLL_RC_ERR_GENERIC;

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
  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrFlowFilterEntryMoMgr::ValidateFlowfilterEntryAction(
    val_flowfilter_entry_t *val_flowfilter_entry, uint32_t operation,
    bool db_up_action_valid, bool action_redirect) {
  UPLL_FUNC_TRACE;

  bool action_is_redirect = false;
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

  /** At the time of UPDATE if action is not set, then check DB, db_up_action_valid flag is
 * true when operation is update and valid flag for action is INVALID, action_redirect is
 * true when action is configured as REDIRECT in DB */

  if (val_flowfilter_entry->valid[UPLL_IDX_ACTION_FFE] == UNC_VF_VALID) {
      if (val_flowfilter_entry->action == UPLL_FLOWFILTER_ACT_REDIRECT) {
         action_is_redirect = true;
      }
  } else if (db_up_action_valid) {
       if (action_redirect) {
         action_is_redirect = true;
       }
  }

  if ((!action_is_redirect)
      && ((val_flowfilter_entry->valid[UPLL_IDX_REDIRECT_NODE_FFE]
          == UNC_VF_VALID)
          || (val_flowfilter_entry->valid[UPLL_IDX_REDIRECT_PORT_FFE]
              == UNC_VF_VALID)
          || (val_flowfilter_entry->valid[UPLL_IDX_MODIFY_DST_MAC_FFE]
              == UNC_VF_VALID)
          || (val_flowfilter_entry->valid[UPLL_IDX_MODIFY_SRC_MAC_FFE]
              == UNC_VF_VALID))) {
    UPLL_LOG_DEBUG(
        "redirect_node/redirect_port/modify_dstmac/modify_srcmac is configured"
        "when action is not REDIRECT");
    return UPLL_RC_ERR_CFG_SYNTAX;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrFlowFilterEntryMoMgr::ValidateCapability(IpcReqRespHeader *req,
                                                      ConfigKeyVal *ikey,
                                                      const char* ctrlr_name) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_ERR_GENERIC;

  if ((NULL == req) || (NULL == ikey)) {
    UPLL_LOG_DEBUG("IpcReqRespHeader/ConfigKeyval is NULL");
    return result_code;
  }

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
      break;
    }
  }

  if (!ret_code) {
    UPLL_LOG_DEBUG("keytype(%d) is not supported by controller(%s)",
        ikey->get_key_type(), ctrlr_name);
    return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_KT;
  }

  val_flowfilter_entry_t *val_flowfilter_entry = NULL;

  if (ikey->get_cfg_val() && (ikey->get_cfg_val()->get_st_num()
    == IpctSt::kIpcStValFlowfilterEntry)) {
    val_flowfilter_entry =
      reinterpret_cast<val_flowfilter_entry_t *>(
         ikey->get_cfg_val()->get_val());
  }

  if ((operation == UNC_OP_CREATE) || (operation == UNC_OP_UPDATE)) {
    if (dt_type == UPLL_DT_CANDIDATE) {
      if (val_flowfilter_entry) {
        if (max_attrs > 0) {
          return ValFlowFilterEntryAttributeSupportCheck(val_flowfilter_entry,
                                                            attrs);
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
      if ((option1 != UNC_OPT1_NORMAL)
          && (option1 == UNC_OPT1_DETAIL
            && operation == UNC_OP_READ_SIBLING_COUNT)) {
        UPLL_LOG_DEBUG(" Error: option1 is not NORMAL");
        return UPLL_RC_ERR_INVALID_OPTION1;
      }

      if (option2 != UNC_OPT2_NONE) {
        UPLL_LOG_DEBUG(" Error: option2 is not NONE");
        return UPLL_RC_ERR_INVALID_OPTION2;
      }

      if (val_flowfilter_entry) {
        if (max_attrs > 0) {
          return ValFlowFilterEntryAttributeSupportCheck(val_flowfilter_entry,
                                                         attrs);
        } else {
          UPLL_LOG_DEBUG("Attribute list is empty for operation %d", operation);
          return UPLL_RC_ERR_CFG_SYNTAX;
        }
      }
      return UPLL_RC_SUCCESS;
    } else {
      UPLL_LOG_DEBUG("Error Unsupported datatype (%d)", dt_type);
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  } else if (OPEARTION_WITH_VAL_STRUCT_NONE) {
    /** Value struct is NONE for this operations */
    UPLL_LOG_DEBUG("Skip Attribute validation, Operation type is %d",
      operation);
    return UPLL_RC_SUCCESS;
  }

  UPLL_LOG_DEBUG("Error Unsupported operation (%d)", operation);
  return UPLL_RC_ERR_CFG_SYNTAX;
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
            UNC_VF_NOT_SOPPORTED;

        UPLL_LOG_DEBUG("flowlist_name attr is not supported by ctrlr");
        return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
      }
    }

    if ((val_flowfilter_entry->valid[UPLL_IDX_ACTION_FFE] == UNC_VF_VALID)
        || (val_flowfilter_entry->valid[UPLL_IDX_ACTION_FFE]
            == UNC_VF_VALID_NO_VALUE)) {
      if (attrs[unc::capa::vbr_flowfilter_entry::kCapAction] == 0) {
        val_flowfilter_entry->valid[UPLL_IDX_ACTION_VFFE] =
            UNC_VF_NOT_SOPPORTED;

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
            UNC_VF_NOT_SOPPORTED;

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
            UNC_VF_NOT_SOPPORTED;

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
            UNC_VF_NOT_SOPPORTED;

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
            UNC_VF_NOT_SOPPORTED;

        UPLL_LOG_DEBUG("modify_src attr is not supported by ctrlr");
        return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
      }
    }

    if ((val_flowfilter_entry->valid[UPLL_IDX_NWM_NAME_FFE] == UNC_VF_VALID)
        || (val_flowfilter_entry->valid[UPLL_IDX_NWM_NAME_FFE]
            == UNC_VF_VALID_NO_VALUE)) {
      if (attrs[unc::capa::vbr_flowfilter_entry::kCapNwmName] == 0) {
        val_flowfilter_entry->valid[UPLL_IDX_NWM_NAME_FFE] =
            UNC_VF_NOT_SOPPORTED;

        UPLL_LOG_DEBUG("Nwmname attr is not supported by ctrlr");
        return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
      }
      return UPLL_RC_SUCCESS;
    }
    /*
    if ((val_flowfilter_entry->valid[UPLL_IDX_DSCP_FFE] == UNC_VF_VALID)
        || (val_flowfilter_entry->valid[UPLL_IDX_DSCP_FFE]
            == UNC_VF_VALID_NO_VALUE)) {
      if (attrs[unc::capa::vbr_flowfilter_entry::kCapDscp] == 0) {
        val_flowfilter_entry->valid[UPLL_IDX_DSCP_FFE] = UNC_VF_NOT_SOPPORTED;

        UPLL_LOG_DEBUG("dscp attr is not supported by ctrlr");
        return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
      }
      return UPLL_RC_SUCCESS;
    }

    if ((val_flowfilter_entry->valid[UPLL_IDX_PRIORITY_FFE] == UNC_VF_VALID)
        || (val_flowfilter_entry->valid[UPLL_IDX_PRIORITY_FFE]
            == UNC_VF_VALID_NO_VALUE)) {
      if (attrs[unc::capa::vbr_flowfilter_entry::kCapPriority] == 0) {
        val_flowfilter_entry->valid[UPLL_IDX_PRIORITY_FFE] =
            UNC_VF_NOT_SOPPORTED;

        UPLL_LOG_DEBUG("Priority attr is not supported by ctrlr");
        return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
      }
      return UPLL_RC_SUCCESS;
    }
    */
  } else {
    UPLL_LOG_DEBUG("Error value struct is NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  return UPLL_RC_SUCCESS;
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

  key_vbr_flowfilter_entry_t *key_vbr_ffe =
      reinterpret_cast<key_vbr_flowfilter_entry_t *>(ikey->get_key());
  MoMgrImpl *mgr = reinterpret_cast<MoMgrImpl *>
      (const_cast<MoManager *>(GetMoManager(UNC_KT_VBR_FLOWFILTER)));

  result_code = mgr->GetChildConfigKey(okey, NULL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetChildConfigKey returned error - %d", result_code);
    return result_code;
  }

  key_vbr_flowfilter_t *vbr_ff_key =
      reinterpret_cast<key_vbr_flowfilter_t *>(okey->get_key());

  uuu::upll_strncpy(vbr_ff_key->vbr_key.vtn_key.vtn_name,
        key_vbr_ffe->flowfilter_key.vbr_key.vtn_key.vtn_name,
        kMaxLenVtnName + 1);

  uuu::upll_strncpy(vbr_ff_key->vbr_key.vbridge_name,
        key_vbr_ffe->flowfilter_key.vbr_key.vbridge_name,
        kMaxLenVnodeName + 1);

  vbr_ff_key->direction = key_vbr_ffe->flowfilter_key.direction;

  /* Checks the given vbr_flowfilter exists in DB or not */
  result_code = mgr->UpdateConfigDB(okey, req->datatype, UNC_OP_READ,
                                    dmi, MAINTBL);

  if (UPLL_RC_ERR_INSTANCE_EXISTS != result_code) {
    UPLL_LOG_DEBUG(" Parent VBR_FLOWFILTER key does not exists");
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

    /** fill key_nwm from key/val VBR_FLOWFILTER_ENTRY structs*/
    key_nwm_t *key_nwm = static_cast<key_nwm_t*>(
      okey->get_key());

    uuu::upll_strncpy(key_nwm->nwmonitor_name,
      val_flowfilter_entry->nwm_name,
      kMaxLenVnodeName+1);

    uuu::upll_strncpy(key_nwm->vbr_key.vtn_key.vtn_name,
      key_vbr_ffe->flowfilter_key.vbr_key.vtn_key.vtn_name,
      kMaxLenVtnName+1);

    uuu::upll_strncpy(key_nwm->vbr_key.vbridge_name,
      key_vbr_ffe->flowfilter_key.vbr_key.vbridge_name,
      kMaxLenVnodeName+1);

    /* Check nwm_name exists in table*/
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
    vbr_ffe_key = reinterpret_cast<key_vbr_flowfilter_entry_t *>
        (okey->get_key());
  } else {
    vbr_ffe_key = reinterpret_cast<key_vbr_flowfilter_entry_t *>
        (ConfigKeyVal::Malloc(sizeof(key_vbr_flowfilter_entry_t)));
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
    default:
      if (vbr_ffe_key) free(vbr_ffe_key);
      return UPLL_RC_ERR_GENERIC;
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
    return UPLL_RC_ERR_GENERIC;
  }

  UPLL_LOG_DEBUG("DupConfigkeyVal Succesfull.");
  // free(vbr_if_flowfilter_entry);
  // free(vbr_if_flowfilter_val);
  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrFlowFilterEntryMoMgr::GetRenamedUncKey(ConfigKeyVal *ikey,
    upll_keytype_datatype_t dt_type, DalDmlIntf *dmi, uint8_t *ctrlr_id) {
  // ConfigKeyVal *unc_key = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY);
  ConfigKeyVal *unc_key = NULL;
  if (NULL == ikey) {
    return UPLL_RC_ERR_GENERIC;
  }

  DbSubOp dbop = { kOpReadSingle, kOpMatchCtrlr, kOpInOutNone };
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  MoMgrImpl *mgrvbr = reinterpret_cast<MoMgrImpl *>
         (const_cast<MoManager *> (GetMoManager(UNC_KT_VBRIDGE)));
  val_rename_vnode *rename_val = reinterpret_cast<val_rename_vnode*>
      (ConfigKeyVal::Malloc(sizeof(val_rename_vnode)));

  key_vbr_flowfilter_entry_t *ctrlr_key =
      reinterpret_cast<key_vbr_flowfilter_entry_t *> (ikey->get_key());

  uuu::upll_strncpy(rename_val->ctrlr_vtn_name,
          ctrlr_key->flowfilter_key.vbr_key.vtn_key.vtn_name,
          (kMaxLenVtnName + 1));
  uuu::upll_strncpy(rename_val->ctrlr_vnode_name,
                    ctrlr_key->flowfilter_key.vbr_key.vbridge_name,
                    (kMaxLenVnodeName + 1));
  result_code = mgrvbr->GetChildConfigKey(unc_key, NULL);

  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_ERROR("GetRenamedUnckey:GetChildConfigKey returned error");
    free(rename_val);  // RESOURCE LEAK
    return result_code;
  }
  if (ctrlr_id == NULL) {
    UPLL_LOG_DEBUG("Controller Name is Not Valid");
    free(rename_val);
    delete unc_key;
    return UPLL_RC_ERR_GENERIC;
  }

  unc_key->set_user_data(ctrlr_id);
  unc_key->AppendCfgVal(IpctSt::kIpcStValRenameVtn, rename_val);
  result_code = mgrvbr->ReadConfigDB(unc_key, dt_type, UNC_OP_READ,
      dbop, dmi, RENAMETBL);
  if (result_code == UPLL_RC_SUCCESS) {
    key_vbr_flowfilter_entry_t *vbr_flowfilter_entry_key =
        reinterpret_cast<key_vbr_flowfilter_entry_t *> (unc_key->get_key());

    uuu::upll_strncpy(ctrlr_key->flowfilter_key.vbr_key.vtn_key.vtn_name,
            vbr_flowfilter_entry_key->flowfilter_key.vbr_key.vtn_key.vtn_name,
            (kMaxLenVtnName + 1));

    uuu::upll_strncpy(ctrlr_key->flowfilter_key.vbr_key.vbridge_name,
              vbr_flowfilter_entry_key->flowfilter_key.vbr_key.vbridge_name,
              (kMaxLenVnodeName + 1));
  }

  delete unc_key;
  unc_key = NULL;
  val_rename_flowlist_t *rename_flowlist =
      reinterpret_cast<val_rename_flowlist_t*>
                   (ConfigKeyVal::Malloc(sizeof(val_rename_flowlist_t)));

  val_flowfilter_entry_t *val_flowfilter_entry =
      reinterpret_cast<val_flowfilter_entry_t *> (GetVal(ikey));

  uuu::upll_strncpy(rename_flowlist->flowlist_newname,
                    val_flowfilter_entry->flowlist_name,
                    (kMaxLenFlowListName + 1));

  MoMgrImpl* mgr = reinterpret_cast<MoMgrImpl *>
         (const_cast<MoManager *> (GetMoManager(UNC_KT_FLOWLIST)));
  result_code = mgr->GetChildConfigKey(unc_key, NULL);

  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_ERROR("GetChildConfigKey Returned an error");
    free(rename_flowlist);  // RESOURCE LEAK
    return result_code;
  }
  SET_USER_DATA_CTRLR(unc_key, ctrlr_id);
  unc_key->AppendCfgVal(IpctSt::kIpcStValRenameFlowlist, rename_flowlist);
  result_code = mgr->ReadConfigDB(unc_key, dt_type, UNC_OP_READ, dbop, dmi,
      RENAMETBL);
  if (result_code == UPLL_RC_SUCCESS) {
    key_flowlist_t *key_flowlist = NULL;
    key_flowlist = reinterpret_cast<key_flowlist_t *> (unc_key->get_key());

    uuu::upll_strncpy(val_flowfilter_entry->flowlist_name,
                      key_flowlist->flowlist_name,
                      (kMaxLenFlowListName + 1));
  }

  UPLL_LOG_DEBUG("Key is filled with UncKey Successfully %d", result_code);
  free(rename_val);
  delete unc_key;
  return result_code;
}

upll_rc_t VbrFlowFilterEntryMoMgr::GetRenamedControllerKey(
    ConfigKeyVal *ikey,
    upll_keytype_datatype_t dt_type,
    DalDmlIntf *dmi,
    controller_domain *ctrlr_dom) {

  UPLL_FUNC_TRACE;
  ConfigKeyVal *okey = NULL;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  uint8_t rename = 0;
  IsRenamed(ikey, dt_type, dmi, rename);
  if (!rename)
    return UPLL_RC_SUCCESS;
  /* Vtn renamed */
  MoMgrImpl *mgrvtn = reinterpret_cast<MoMgrImpl *>
      (const_cast<MoManager *> (GetMoManager(UNC_KT_VTN)));
  if (rename & VTN_RENAME_FLAG) {
    mgrvtn->GetChildConfigKey(okey, NULL);

    if (ctrlr_dom != NULL) {
      SET_USER_DATA_CTRLR_DOMAIN(okey, *ctrlr_dom);
    } else {
      delete okey;  // RESOURCE LEAK
      return UPLL_RC_ERR_GENERIC;
    }

    uuu::upll_strncpy(reinterpret_cast<key_vtn *>(okey->get_key())->vtn_name,
                      reinterpret_cast<key_vbr_flowfilter_entry_t *>
                     (ikey->get_key())->flowfilter_key.vbr_key.vtn_key.vtn_name,
                      (kMaxLenVtnName + 1));
    DbSubOp dbop = { kOpReadSingle, kOpMatchCtrlr, kOpInOutFlag };
    mgrvtn->ReadConfigDB(okey, dt_type, UNC_OP_READ, dbop, dmi, RENAMETBL);
    val_rename_vtn *rename_val = NULL;
    rename_val = reinterpret_cast<val_rename_vtn *> (GetVal(okey));

    if (!rename_val
        || (rename_val->valid[UPLL_IDX_NEW_NAME_RVTN] != UNC_VF_VALID)) {
      UPLL_LOG_DEBUG("Vtn Name is not Valid.");
      return UPLL_RC_ERR_GENERIC;
    }

    uuu::upll_strncpy(reinterpret_cast<key_vbr_flowfilter_entry_t *>
              (ikey->get_key())->flowfilter_key.vbr_key.vtn_key.vtn_name,
              rename_val->new_name,
              (kMaxLenVtnName + 1));
    delete okey;
  }
  // Vbr Renamed
  MoMgrImpl *mgrvbr = reinterpret_cast<MoMgrImpl *>
           (const_cast<MoManager *> (GetMoManager(UNC_KT_VBRIDGE)));
  if (rename & VBR_RENAME_FLAG) {
  result_code = mgrvbr->GetChildConfigKey(okey, NULL);  // COV USE AFTER FREE
  if (result_code != UPLL_RC_SUCCESS) {
    return result_code;
  }
    if (NULL != ctrlr_dom)
      SET_USER_DATA_CTRLR_DOMAIN(okey, *ctrlr_dom)
    else
      return UPLL_RC_ERR_GENERIC;

    uuu::upll_strncpy(reinterpret_cast<key_vbr *>
                      (okey->get_key())->vbridge_name,
           reinterpret_cast<key_vbr_flowfilter_entry_t *>
         (ikey->get_key())->flowfilter_key.vbr_key.vbridge_name,
         (kMaxLenVnodeName + 1));
    DbSubOp dbop = { kOpReadSingle, kOpMatchCtrlr, kOpInOutFlag };
    result_code = mgrvbr->ReadConfigDB(okey, dt_type, UNC_OP_READ,
                              dbop, dmi, RENAMETBL);
    if (result_code != UPLL_RC_SUCCESS) {  // COV CHECKED RETURNS
     return result_code;
    }
    val_rename_vbr *rename_val = NULL;
    rename_val = reinterpret_cast<val_rename_vbr *> (GetVal(okey));
    if (!rename_val
        || (rename_val->valid[UPLL_IDX_NEW_NAME_RVBR] != UNC_VF_VALID)) {
      UPLL_LOG_DEBUG("Vbr Name is not Valid.");
      return UPLL_RC_ERR_GENERIC;
    }
    uuu::upll_strncpy(reinterpret_cast<char *>
           (reinterpret_cast<key_vbr_flowfilter_entry_t *>
           (ikey->get_key())->flowfilter_key.vbr_key.vbridge_name),
           reinterpret_cast<const char *> (rename_val->new_name),
           kMaxLenVnodeName + 1);
    delete okey;
  }

  // Flowlist is Renamed
  if (rename & FLOWLIST_RENAME_FLAG) {
    MoMgrImpl *mgrflist = reinterpret_cast<MoMgrImpl *>
          (const_cast<MoManager *> (GetMoManager(UNC_KT_FLOWLIST)));
    mgrflist->GetChildConfigKey(okey, ikey);

    if (NULL != ctrlr_dom) {
      SET_USER_DATA_CTRLR(okey, ctrlr_dom->ctrlr);
    } else {
      return UPLL_RC_ERR_GENERIC;
    }

    DbSubOp dbop = { kOpReadSingle, kOpMatchCtrlr, kOpInOutFlag };
    result_code = mgrflist->ReadConfigDB(okey, dt_type, UNC_OP_READ,
                                dbop, dmi, RENAMETBL);
    if (result_code != UPLL_RC_SUCCESS) {  // COV CHECKED RETURNS
      return result_code;
    }
    val_rename_flowlist_t *rename_val = NULL;
    rename_val = reinterpret_cast<val_rename_flowlist_t *> (GetVal(okey));

    if (!rename_val
        || (rename_val->valid[UPLL_IDX_NEW_NAME_RVTN] != UNC_VF_VALID)) {
      UPLL_LOG_DEBUG("flowlist is not valid");
      return UPLL_RC_ERR_GENERIC;
    }

    uuu::upll_strncpy(reinterpret_cast<val_flowfilter_entry_t*>
        (ikey->get_cfg_val()->get_val())->flowlist_name,
        rename_val->flowlist_newname,
        (kMaxLenFlowListName + 1));
    SET_USER_DATA_FLAGS(ikey, VTN_RENAME);   // TODO(me) :FLOWLIST_RENAME
    delete okey;
  }

  UPLL_LOG_DEBUG("Renamed Controller key is sucessfull.");
  return UPLL_RC_SUCCESS;
}
upll_rc_t VbrFlowFilterEntryMoMgr::GetControllerDomainID(ConfigKeyVal *ikey,
                                          controller_domain *ctrlr_dom,
                                          upll_keytype_datatype_t dt_type,
                                          DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  VbrMoMgr *mgrvbr =
    reinterpret_cast<VbrMoMgr *>(const_cast<MoManager *>(GetMoManager(
            UNC_KT_VBRIDGE)));
  ConfigKeyVal *ckv = NULL;
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
    delete ckv;
    return result_code;
  }
  result_code = mgrvbr->GetControllerDomainId(ckv, ctrlr_dom);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetControllerDomainId failed %d ", result_code);
    return result_code;
  }
  return result_code;
}

upll_rc_t VbrFlowFilterEntryMoMgr::CreateCandidateMo(IpcReqRespHeader *req,
    ConfigKeyVal *ikey, DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  uint8_t *ctrlr_id = NULL;
  ConfigKeyVal* okey = NULL;
  controller_domain ctrlr_dom;
  if (ikey == NULL && req == NULL) {
    UPLL_LOG_DEBUG(
        "Cannot perform create operation due to insufficient parameters\n");
    return UPLL_RC_ERR_GENERIC;
  }

  upll_rc_t result_code = UPLL_RC_SUCCESS;
  // validate syntax and semantics
  result_code = ValidateMessage(req, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("ValidateMessage failed %d", result_code);
    return result_code;
  }

  result_code = ValidateAttribute(ikey, dmi, req);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("ValidateAttribute failed %d", result_code);
    return result_code;
  }

  /*
  result_code = ValidateCapability(req, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("validate Capability Failed %d", result_code);
    return result_code;
  }
  */
  // Check if Object already exists in CANDIDATE DB
  result_code = UpdateConfigDB(ikey, req->datatype, UNC_OP_READ, dmi);
  if (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    UPLL_LOG_DEBUG("UpdateConfigDB failed Candidate DB");
    return result_code;
  }

  // Check if Object exists in RUNNING DB and move it to CANDIDATE DB
  result_code = UpdateConfigDB(ikey, UPLL_DT_RUNNING, UNC_OP_READ, dmi,
      MAINTBL);
  if (result_code == UPLL_RC_ERR_INSTANCE_EXISTS) {
    result_code = RestoreChildren(ikey, req->datatype, UPLL_DT_RUNNING, dmi);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Restore Chidlren Failed");
      return result_code;
    }
  } else if (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    UPLL_LOG_DEBUG("UpdateConfigDb failed to read running");
    return result_code;
  } else {
    UPLL_LOG_DEBUG("No instance found in running db");
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
    result_code = mgr->UpdateConfigDB(okey, req->datatype, UNC_OP_READ,
                                      dmi, MAINTBL);
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
  result_code = GetControllerDomainID(ikey, &ctrlr_dom, req->datatype, dmi);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Failed to Get the Controller Domain details, err:%d",
                     result_code);
  }
  SET_USER_DATA_CTRLR_DOMAIN(ikey, ctrlr_dom);

  UPLL_LOG_DEBUG("ctrlrid %s, domainid %s",
                  ctrlr_dom.ctrlr, ctrlr_dom.domain);
  ctrlr_id = ctrlr_dom.ctrlr;
  if (flowfilter_val->valid[UPLL_IDX_FLOWLIST_NAME_FFE] == UNC_VF_VALID) {
    result_code = mgr->AddFlowListToController(
        reinterpret_cast<char *>(flowfilter_val->flowlist_name), dmi,
        reinterpret_cast<char *>(ctrlr_id) , UNC_OP_CREATE);

    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Unable to Update the FlowList at ctrlr Table");
      return result_code;
    }
  }

  // N/w monitor
#if 0
  NwMonitorMoMgr *nmgr = reinterpret_cast<NwMonitorMoMgr *>
  (const_cast<MoManager *> (GetMoManager(UNC_KT_VBR_NWMONITOR)));
  //  result_code = nmgr->GetChildConfigKey(okey, NULL); // TODO
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Recored %d", result_code);
    return result_code;
  }
  key_nwm_t *key_nwm = reinterpret_cast<key_nwm_t*>(okey->get_key());
  strncpy(reinterpret_cast<char*>(key_nwm->nwmonitor_name),
     reinterpret_cast<const char*>(flowfilter_val->nwm_name),
               kMaxLenNwmName +1);
//  result_code = nmgr->IsReferenced(okey, req->datatype, dmi); // TODO
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Recored %d", result_code);
    return result_code;
  }
#endif

  // create a record in CANDIDATE DB
  DbSubOp dbop = { kOpNotRead, kOpMatchNone, kOpInOutDomain
                       | kOpInOutCtrlr };
  result_code = UpdateConfigDB(ikey, req->datatype, UNC_OP_CREATE,
                               dmi, &dbop, MAINTBL);
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

  GET_USER_DATA_CTRLR(okey, ctrlr_id);
  val_flowfilter_entry_t *flowfilter_val =
      reinterpret_cast<val_flowfilter_entry_t *> (GetVal(okey));
  if (flowfilter_val->valid[UPLL_IDX_FLOWLIST_NAME_FFE] == UNC_VF_VALID) {
    FlowListMoMgr *mgr = reinterpret_cast<FlowListMoMgr *>
        (const_cast<MoManager *> (GetMoManager(UNC_KT_FLOWLIST)));
    result_code = mgr->AddFlowListToController(
        reinterpret_cast<char *>(flowfilter_val->flowlist_name), dmi,
        reinterpret_cast<char *>(ctrlr_id), UNC_OP_DELETE);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG(" Send delete request to flowlist failed. err code(%d)",
                     result_code);
      return result_code;
    }
  }
  result_code = UpdateConfigDB(ikey, UPLL_DT_CANDIDATE, UNC_OP_DELETE, dmi,
                               MAINTBL);

  UPLL_LOG_DEBUG("DeleteMo Operation Done %d", result_code);
  return result_code;
}

upll_rc_t VbrFlowFilterEntryMoMgr::MergeValidate(unc_key_type_t keytype,
    const char *ctrlr_id, ConfigKeyVal *ikey, DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  DbSubOp dbop = { kOpReadMultiple, kOpMatchNone, kOpInOutNone };
  ConfigKeyVal *tkey;
  ConfigKeyVal *keyval = NULL;

  if (NULL == ikey) {
    UPLL_LOG_DEBUG("Memory is Allocated.");
    return UPLL_RC_ERR_GENERIC;
  }
  result_code = ReadConfigDB(ikey, UPLL_DT_IMPORT, UNC_OP_READ, dbop, dmi,
      MAINTBL);
  if (UPLL_RC_SUCCESS != result_code) {
    return result_code;
  }

  MoMgrImpl *mgr = reinterpret_cast<MoMgrImpl *>
                  (const_cast<MoManager *> (GetMoManager(UNC_KT_FLOWLIST)));
  tkey = ikey;
  while (ikey != NULL) {
    result_code = mgr->GetChildConfigKey(keyval, NULL);

    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("GetChildConfigKey is Failed %d", result_code);
      return result_code;
    }

    val_flowfilter_entry_t *flowfilter_val =
           reinterpret_cast<val_flowfilter_entry_t *> (GetVal(ikey));
    key_flowlist_t *key_flowlist = NULL;
    key_flowlist = reinterpret_cast<key_flowlist_t *> (keyval->get_key());

    uuu::upll_strncpy(key_flowlist->flowlist_name,
                 flowfilter_val->flowlist_name,
                (kMaxLenFlowListName + 1));
    result_code = mgr->UpdateConfigDB(keyval, UPLL_DT_CANDIDATE, UNC_OP_READ,
        dmi, MAINTBL);

    if (result_code != UPLL_RC_ERR_INSTANCE_EXISTS) {
      UPLL_LOG_DEBUG("Instance Already Exists");
      return UPLL_RC_ERR_MERGE_CONFLICT;
    }

    ikey = tkey->get_next_cfg_key_val();
  }
  if (tkey)
    delete tkey;
  return result_code;
}

upll_rc_t VbrFlowFilterEntryMoMgr::UpdateAuditConfigStatus(
    unc_keytype_configstatus_t cs_status,
    uuc::UpdateCtrlrPhase phase,
    ConfigKeyVal *&ckv_running) {
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  val_flowfilter_entry_t *val;
  val = (ckv_running != NULL)?reinterpret_cast<val_flowfilter_entry_t *>
                              (GetVal(ckv_running)):NULL;
  if (NULL == val) {
    return UPLL_RC_ERR_GENERIC;
  }
  if (uuc::kUpllUcpCreate == phase )
     val->cs_row_status = cs_status;
  for ( unsigned int loop = 0;
        loop < sizeof(val->valid)/sizeof(val->valid[0]); loop++ ) {
    if ( cs_status == UNC_CS_INVALID &&  UNC_VF_VALID == val->valid[loop])
         val->cs_attr[loop] = cs_status;
    else
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

  result_code = ValidateValFlowfilterEntry(req, ikey, dmi);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("ValidateValFlowfilterEntry Failed %d", result_code);
    return result_code;
  }

  result_code = ValidateAttribute(ikey, dmi, req);
  if (UPLL_RC_SUCCESS != result_code) {
    return result_code;
  }
#if 0
  result_code = ValidateCapability(req, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("validate Capability Failed %d", result_code);
    return result_code;
  }
#endif
  result_code = UpdateConfigDB(ikey, req->datatype, UNC_OP_READ, dmi, MAINTBL);
  if (UPLL_RC_ERR_INSTANCE_EXISTS != result_code) {
    UPLL_LOG_DEBUG("UpdateConfigDB failed %d", result_code);
    return result_code;
  }

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
          reinterpret_cast<char *> (ctrlr_id), UNC_OP_DELETE);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("AddFlowListToController failed %d", result_code);
        delete okey;
        return result_code;
      }
      result_code = flowlist_mgr->AddFlowListToController(
          reinterpret_cast<char *>(flowfilter_val->flowlist_name), dmi,
          reinterpret_cast<char *>(ctrlr_id), UNC_OP_CREATE);
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
          reinterpret_cast<char *>(ctrlr_id), UNC_OP_CREATE);
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
          reinterpret_cast<char *>(ctrlr_id), UNC_OP_DELETE);
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
  //  result_code = nmgr->GetChildConfigKey(okey, NULL); // TODO
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Recored %d", result_code);
    return result_code;
  }
  key_nwm_t *key_nwm = reinterpret_cast<key_nwm_t*>(okey->get_key());
  strncpy(reinterpret_cast<char*>(key_nwm->nwmonitor_name),
      reinterpret_cast<const char*>(flowfilter_val->nwm_name),
      kMaxLenNwmName +1);
  //  result_code = nmgr->IsReferenced(okey, req->datatype, dmi); // TODO
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Recored %d", result_code);
    return result_code;
  }
#endif

  result_code = UpdateConfigDB(ikey, req->datatype, UNC_OP_UPDATE, dmi,
                               MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Unable to Update the CandidateDB");
    return result_code;
  }
  delete okey;
  return result_code;
}

upll_rc_t VbrFlowFilterEntryMoMgr::IsReferenced(ConfigKeyVal *ikey,
                                           upll_keytype_datatype_t dt_type,
                                           DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  if (NULL == ikey) return UPLL_RC_ERR_GENERIC;
  DbSubOp dbop = { kOpReadExist, kOpMatchNone, kOpInOutNone };
  // Check the VBR FlowFilter Entry object existence
  result_code = UpdateConfigDB(ikey, dt_type, UNC_OP_READ, dmi, &dbop, MAINTBL);
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

upll_rc_t VbrFlowFilterEntryMoMgr::ReadSiblingMo(IpcReqRespHeader *req,
                                                 ConfigKeyVal *ikey,
                                                 bool begin,
                                                 DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  controller_domain ctrlr_dom;
  ConfigKeyVal  *dup_key = NULL, *l_key = NULL, *tctrl_key = NULL;
  ConfigKeyVal *okey =NULL, *tmp_key = NULL;
  DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutCtrlr|kOpInOutDomain };

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

        result_code =  DupConfigKeyVal(dup_key, tctrl_key, MAINTBL);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG(" DupConfigKeyVal failed for dup_key%d ", result_code);
          DELETE_IF_NOT_NULL(tctrl_key);
          return result_code;
        }

        result_code = ReadConfigDB(dup_key, req->datatype, UNC_OP_READ,
                                   dbop, dmi, MAINTBL);
        if (UPLL_RC_SUCCESS != result_code) {
          UPLL_LOG_DEBUG("ReadConfigDb failed for tctrl_key err code(%d)",
                         result_code);
          DELETE_IF_NOT_NULL(tctrl_key);
          DELETE_IF_NOT_NULL(dup_key);
          return result_code;
        }

        result_code =  DupConfigKeyVal(l_key, ikey, MAINTBL);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG(" DupConfigKeyVal failed for l_key%d ", result_code);
          return result_code;
        }

        GET_USER_DATA_CTRLR_DOMAIN(dup_key, ctrlr_dom);
        SET_USER_DATA_CTRLR_DOMAIN(l_key, ctrlr_dom);
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

upll_rc_t VbrFlowFilterEntryMoMgr::RenameMo(IpcReqRespHeader *req,
    ConfigKeyVal *ikey, DalDmlIntf *dmi, const char *ctrlr_id) {
  UPLL_FUNC_TRACE;
  UPLL_LOG_DEBUG("Implementation Not supported for this KT.");
  return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_KT;
}

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

upll_rc_t VbrFlowFilterEntryMoMgr::GetControllerId(ConfigKeyVal *ikey,
        DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *temp = NULL;
  ConfigVal *tmpval = NULL;
  MoMgrImpl *mgr =
      reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>(GetMoManager(
          UNC_KT_VBRIDGE)));
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
  val_vbr *ival = reinterpret_cast<val_vbr *>(GetVal(temp));
  if (!ival) {
    delete temp;
    return UPLL_RC_ERR_GENERIC;
  }
  #if 0  // TODO(pranjal)
  if (!ival->controller_id) {
    UPLL_LOG_DEBUG("Controller Id is not valid");
    free(vbr_val);
    return UPLL_RC_ERR_GENERIC;
  }
  #endif
  SET_USER_DATA_CTRLR(ikey, ival->controller_id);
  UPLL_LOG_DEBUG("GetController Id is Successfull");
  free(ival);
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
  if (UNC_KT_VBR_FLOWFILTER_ENTRY == ikey->get_key_type()) {
    key_rename_vnode_info *key_rename = NULL;
    key_rename = reinterpret_cast<key_rename_vnode_info *> (ikey->get_key());
    key_vbr_flowfilter_entry_t * key_vbr_ff_entry =
         reinterpret_cast<key_vbr_flowfilter_entry_t *>
         (ConfigKeyVal::Malloc(sizeof(key_vbr_flowfilter_entry_t)));

    if (!strlen(reinterpret_cast<char *> (key_rename->old_unc_vtn_name))) {
      UPLL_LOG_DEBUG("String Length not Valid to Perform the Operation");
      free(key_vbr_ff_entry);
      return UPLL_RC_ERR_GENERIC;
    }

    uuu::upll_strncpy(key_vbr_ff_entry->flowfilter_key.vbr_key.vtn_key.vtn_name,
                    key_rename->old_unc_vtn_name,
                    (kMaxLenVtnName + 1));

    if (ikey->get_key_type() == table[MAINTBL]->get_key_type()) {
      if (!strlen(reinterpret_cast<char *> (key_rename->old_unc_vnode_name))) {
        free(key_vbr_ff_entry);
        return UPLL_RC_ERR_GENERIC;
      }

      uuu::upll_strncpy(key_vbr_ff_entry->flowfilter_key.vbr_key.vbridge_name,
                       key_rename->old_unc_vnode_name,
                       (kMaxLenVnodeName + 1));
    }

    okey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY,
                          IpctSt::kIpcStKeyVbrFlowfilterEntry,
                          key_vbr_ff_entry, NULL);

  } else if (UNC_KT_FLOWLIST == ikey->get_key_type()) {
    key_rename_vnode_info_t *key_rename =
      reinterpret_cast<key_rename_vnode_info_t *>(ikey->get_key());
    val_flowfilter_entry_t *val =  reinterpret_cast<val_flowfilter_entry_t*>
                   (ConfigKeyVal::Malloc(sizeof(val_flowfilter_entry_t)));

    uuu::upll_strncpy(val->flowlist_name,
                      key_rename->old_flowlist_name,
                      (kMaxLenFlowListName+1));

    val->valid[UPLL_IDX_FLOWLIST_PPE] = UNC_VF_VALID;

    ConfigVal *ckv = new ConfigVal(IpctSt::kIpcStValFlowfilterEntry, val);
    okey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER_ENTRY,
        IpctSt::kIpcStKeyVbrFlowfilterEntry, NULL, ckv);
  } else {
    UPLL_LOG_DEBUG("CopyToConfigKey invalid key type NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  return result_code;
}

bool VbrFlowFilterEntryMoMgr::CompareValidValue(void *&val1, void *val2,
                                                bool audit) {
  UPLL_FUNC_TRACE;
  val_flowfilter_entry_t *val_ff_entry1 =
      reinterpret_cast<val_flowfilter_entry_t *>(val1);
  val_flowfilter_entry_t *val_ff_entry2 =
      reinterpret_cast<val_flowfilter_entry_t *>(val2);

  //  if (audit) {
    for ( unsigned int loop = 0; loop < (sizeof(val_ff_entry1->valid)/
                               sizeof(val_ff_entry1->valid[0])) ; ++loop ) {
      if ( UNC_VF_INVALID == val_ff_entry1->valid[loop] &&
                  UNC_VF_VALID == val_ff_entry2->valid[loop])
       val_ff_entry1->valid[loop] = UNC_VF_VALID_NO_VALUE;
    }
  //  }

  if (val_ff_entry1->valid[UPLL_IDX_FLOWLIST_NAME_FFE] == UNC_VF_VALID
    && val_ff_entry2->valid[UPLL_IDX_FLOWLIST_NAME_FFE] == UNC_VF_VALID) {
    if (!strcmp(reinterpret_cast<char *>(val_ff_entry1->flowlist_name),
               reinterpret_cast<char *>(val_ff_entry2->flowlist_name)))
      val_ff_entry1->valid[UPLL_IDX_FLOWLIST_NAME_FFE] = UNC_VF_INVALID;
  }
  if (val_ff_entry1->valid[UPLL_IDX_ACTION_FFE] == UNC_VF_VALID &&
     val_ff_entry2->valid[UPLL_IDX_ACTION_FFE] == UNC_VF_VALID) {
    if (val_ff_entry1->action != val_ff_entry2->action)
      val_ff_entry1->valid[UPLL_IDX_ACTION_FFE] = UNC_VF_INVALID;
  }
  if (val_ff_entry1->valid[UPLL_IDX_REDIRECT_NODE_FFE] == UNC_VF_VALID
     && val_ff_entry2->valid[UPLL_IDX_REDIRECT_NODE_FFE] == UNC_VF_VALID) {
    if (reinterpret_cast<char *>(val_ff_entry1->redirect_node) !=
              reinterpret_cast<const char *>(val_ff_entry2->redirect_node))
      val_ff_entry1->valid[UPLL_IDX_REDIRECT_NODE_FFE] = UNC_VF_INVALID;
  }
  if (val_ff_entry1->valid[UPLL_IDX_REDIRECT_PORT_FFE] == UNC_VF_VALID &&
     val_ff_entry2->valid[UPLL_IDX_REDIRECT_PORT_FFE] == UNC_VF_VALID) {
    if (reinterpret_cast<char *>(val_ff_entry1->redirect_port) !=
              reinterpret_cast<char *>(val_ff_entry2->redirect_port))
      val_ff_entry1->valid[UPLL_IDX_REDIRECT_PORT_FFE] = UNC_VF_INVALID;
  }
  if (val_ff_entry1->valid[UPLL_IDX_MODIFY_DST_MAC_FFE] == UNC_VF_VALID &&
     val_ff_entry2->valid[UPLL_IDX_MODIFY_DST_MAC_FFE] == UNC_VF_VALID) {
    if (reinterpret_cast<char *>(val_ff_entry1->modify_dstmac) !=
        reinterpret_cast<char *>(val_ff_entry2->modify_dstmac))
      val_ff_entry1->valid[UPLL_IDX_MODIFY_DST_MAC_FFE] = UNC_VF_INVALID;
  }
  if (val_ff_entry1->valid[UPLL_IDX_MODIFY_SRC_MAC_FFE] == UNC_VF_VALID &&
     val_ff_entry2->valid[UPLL_IDX_MODIFY_SRC_MAC_FFE] == UNC_VF_VALID) {
    if (reinterpret_cast<char *>(val_ff_entry1->modify_srcmac) !=
              reinterpret_cast<char *>(val_ff_entry2->modify_srcmac))
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
    if (val_ff_entry1->dscp != val_ff_entry2->dscp)
      val_ff_entry1->valid[UPLL_IDX_DSCP_FFE] = UNC_VF_INVALID;
  }
  if (val_ff_entry1->valid[UPLL_IDX_PRIORITY_FFE] == UNC_VF_VALID &&
     val_ff_entry2->valid[UPLL_IDX_PRIORITY_FFE] == UNC_VF_VALID) {
    if (val_ff_entry1->priority != val_ff_entry2->priority)
      val_ff_entry1->valid[UPLL_IDX_PRIORITY_FFE] = UNC_VF_INVALID;
  }
  return false;
}
upll_rc_t VbrFlowFilterEntryMoMgr::ReadDetailEntry(
    ConfigKeyVal *ff_ckv, upll_keytype_datatype_t dt_type,
    DbSubOp dbop, DalDmlIntf *dmi
    ) {
  upll_rc_t result_code = UPLL_RC_ERR_GENERIC;
  if (NULL == ff_ckv) {
    return UPLL_RC_ERR_GENERIC;
  }

    // SET_USER_DATA_DOMAIN(ff_ckv, domain_id);
    // SET_USER_DATA_CTRLR(ff_ckv, ctrlr_id);

  result_code = ReadConfigDB(ff_ckv, dt_type, UNC_OP_READ, dbop, dmi, MAINTBL);
  return result_code;
}

upll_rc_t VbrFlowFilterEntryMoMgr::ConstructReadDetailResponse(
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

upll_rc_t VbrFlowFilterEntryMoMgr::GetParentConfigKey(ConfigKeyVal *&okey,
                                                      ConfigKeyVal *ikey) {
  UPLL_FUNC_TRACE;

  if (!ikey) {
    UPLL_LOG_DEBUG(" Input Key is NULL");
    return UPLL_RC_ERR_GENERIC;
  }

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

}  // namespace kt_momgr
}  // namespace upll
}  // namespace unc
