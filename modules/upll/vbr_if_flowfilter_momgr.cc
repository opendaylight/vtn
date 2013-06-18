/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include "vbr_if_flowfilter_momgr.hh"
#include "vbr_if_momgr.hh"
#include "vbr_if_flowfilter_entry_momgr.hh"
#include "upll_log.hh"


namespace unc {
namespace upll {
namespace kt_momgr {

using unc::upll::ipc_util::IpcUtil;
#define NUM_KEY_MAIN_TBL  7
#define VTN_RENAME_FLAG   0x01
#define VBR_RENAME_FLAG   0x02
#define SET_FLAG_VLINK 0x08
#define SET_FLAG_PORTMAP 0x10
#define SET_FLAG_VLINK_PORTMAP (SET_FLAG_VLINK | SET_FLAG_PORTMAP)

// VbrIfFlowFilter Table(Main Table)
BindInfo VbrIfFlowFilterMoMgr::vbr_if_flowfilter_bind_info[] = {
  { uudst::vbr_if_flowfilter::kDbiVtnName, CFG_KEY,
    offsetof(key_vbr_if_flowfilter_t, if_key.vbr_key.vtn_key.vtn_name),
    uud::kDalChar, (kMaxLenVtnName + 1) },
  { uudst::vbr_if_flowfilter::kDbiVbrName, CFG_KEY,
    offsetof(key_vbr_if_flowfilter_t, if_key.vbr_key.vbridge_name),
    uud::kDalChar, (kMaxLenVnodeName + 1) },
  { uudst::vbr_if_flowfilter::kDbiVbrIfName, CFG_KEY,
    offsetof(key_vbr_if_flowfilter_t, if_key.if_name),
    uud::kDalChar, (kMaxLenInterfaceName + 1) },
  { uudst::vbr_if_flowfilter::kDbiInputDirection, CFG_KEY,
    offsetof(key_vbr_if_flowfilter_t, direction),
    uud::kDalUint8, 1 },
  { uudst::vbr_if_flowfilter::kDbiCtrlrName, CK_VAL,
    offsetof(key_user_data_t, ctrlr_id),
    uud::kDalChar, (kMaxLenCtrlrId + 1) },
  { uudst::vbr_if_flowfilter::kDbiDomainId, CK_VAL,
    offsetof(key_user_data, domain_id),
    uud::kDalChar, (kMaxLenDomainId + 1) },
  { uudst::vbr_if_flowfilter::kDbiFlags, CK_VAL,
    offsetof(key_user_data_t, flags),
    uud::kDalUint8, 1 },
  { uudst::vbr_if_flowfilter::kDbiCsRowStatus, CS_VAL,
    offsetof(val_flowfilter_t, cs_row_status),
    uud::kDalUint8, 1 },
};

BindInfo VbrIfFlowFilterMoMgr::vbr_if_flowfiltermaintbl_bind_info[] = {
  { uudst::vbr_if_flowfilter::kDbiVtnName, CFG_MATCH_KEY,
    offsetof(key_vbr_if_flowfilter_t, if_key.vbr_key.vtn_key.vtn_name),
    uud::kDalChar, (kMaxLenVtnName + 1) },
  { uudst::vbr_if_flowfilter::kDbiVbrName, CFG_MATCH_KEY,
    offsetof(key_vbr_if_flowfilter_t, if_key.vbr_key.vbridge_name),
    uud::kDalChar, (kMaxLenVnodeName + 1) },
  { uudst::vbr_if_flowfilter::kDbiVbrName, CFG_MATCH_KEY,
    offsetof(key_vbr_if_flowfilter_t, if_key.if_name),
    uud::kDalChar, (kMaxLenInterfaceName + 1) },
  { uudst::vbr_if_flowfilter::kDbiInputDirection, CFG_MATCH_KEY,
    offsetof(key_vbr_if_flowfilter_t, direction),
    uud::kDalUint8, 1 },
  { uudst::vbr_if_flowfilter::kDbiVtnName, CFG_INPUT_KEY,
    offsetof(key_rename_vnode_info_t, new_unc_vtn_name),
    uud::kDalChar, (kMaxLenVtnName + 1) },
  { uudst::vbr_if_flowfilter::kDbiVbrName, CFG_INPUT_KEY,
    offsetof(key_rename_vnode_info_t, new_unc_vnode_name),
    uud::kDalChar, (kMaxLenVnodeName + 1) },
  { uudst::vbr_if_flowfilter::kDbiFlags, CFG_INPUT_KEY,
    offsetof(key_user_data_t, flags),
    uud::kDalUint8, 1 }
};

unc_key_type_t VbrIfFlowFilterMoMgr::vbr_if_flowfilter_child[] = {
  UNC_KT_VBRIF_FLOWFILTER_ENTRY };

VbrIfFlowFilterMoMgr::VbrIfFlowFilterMoMgr() : MoMgrImpl() {
  UPLL_FUNC_TRACE;
  // Rename and ctrlr tables not required for this KT
  // setting  table indexed for ctrl and rename table to NULL;
  ntable = MAX_MOMGR_TBLS;
  table = new Table *[ntable];

  // For Main Table
  table[MAINTBL] = new Table(uudst::kDbiVbrIfFlowFilterTbl,
      UNC_KT_VBRIF_FLOWFILTER, vbr_if_flowfilter_bind_info,
      IpctSt::kIpcStKeyVbrIfFlowfilter, IpctSt::kIpcStValFlowfilter,
      uudst::vbr_if_flowfilter::kDbiVbrIfFlowFilterNumCols);
  table[RENAMETBL] = NULL;
  table[CTRLRTBL] = NULL;
  nchild = sizeof(vbr_if_flowfilter_child) / sizeof(vbr_if_flowfilter_child[0]);
  child = vbr_if_flowfilter_child;
}

upll_rc_t VbrIfFlowFilterMoMgr::CreateCandidateMo(IpcReqRespHeader *req,
    ConfigKeyVal *ikey,
    DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  if (ikey == NULL || req == NULL) {
    return UPLL_RC_ERR_GENERIC;
  }
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  // validate syntax and semantics
  result_code = ValidateMessage(req, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("ValidateMessage failed, Error - %d", result_code);
    return result_code;
  }
  result_code = ValidateAttribute(ikey, dmi, req);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("ValidateAttribute failed, Error - %d", result_code);
    return result_code;
  }
  result_code = UpdateConfigDB(ikey, req->datatype, UNC_OP_READ, dmi);
  if (result_code == UPLL_RC_ERR_INSTANCE_EXISTS
      || result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    UPLL_LOG_DEBUG(" An instance of the object exists. err code(%d)",
                   result_code);
    return result_code;
  }

  result_code = UpdateConfigDB(ikey, UPLL_DT_RUNNING,
                               UNC_OP_READ, dmi, MAINTBL);
  if (result_code == UPLL_RC_ERR_INSTANCE_EXISTS) {
    result_code = RestoreChildren(ikey, req->datatype, UPLL_DT_RUNNING, dmi);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG(" restoring the object failed. err code(%d)", result_code);
      return result_code;
    }
  } else if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    // create a record in CANDIDATE DB
    VbrIfMoMgr *mgrvbrif =
        reinterpret_cast<VbrIfMoMgr *>(const_cast<MoManager *>(GetMoManager(
                    UNC_KT_VBR_IF)));
    ConfigKeyVal *ckv = NULL;

    InterfacePortMapInfo flags = kVlinkPortMapNotConfigured;

    result_code = mgrvbrif->GetChildConfigKey(ckv, NULL);
    key_vbr_if_flowfilter_t *ff_key = reinterpret_cast
        <key_vbr_if_flowfilter_t *>(ikey->get_key());
    key_vbr_if_t *vbrif_key = reinterpret_cast<key_vbr_if_t *>(ckv->get_key());

    uuu::upll_strncpy(vbrif_key->vbr_key.vtn_key.vtn_name,
                      ff_key->if_key.vbr_key.vtn_key.vtn_name,
                      kMaxLenVtnName + 1);

    uuu::upll_strncpy(vbrif_key->vbr_key.vbridge_name,
                      ff_key->if_key.vbr_key.vbridge_name,
                      kMaxLenVtnName + 1);

    uuu::upll_strncpy(vbrif_key->if_name,
                      ff_key->if_key.if_name,
                      kMaxLenInterfaceName + 1);

    uint8_t* vexternal = reinterpret_cast<uint8_t*>
        (ConfigKeyVal::Malloc(kMaxLenVnodeName + 1));
    uint8_t* vex_if = reinterpret_cast<uint8_t*>
        (ConfigKeyVal::Malloc(kMaxLenInterfaceName + 1));
    result_code = mgrvbrif->GetVexternal(ckv, req->datatype, dmi,
                                         vexternal, vex_if, flags);
    if (UPLL_RC_SUCCESS != result_code) {
      return result_code;
    }
    uint8_t flag_port_map;
    if (flags & kVlinkConfigured) {
      flag_port_map = SET_FLAG_VLINK;
    } else if (flags & kPortMapConfigured) {
      flag_port_map = SET_FLAG_PORTMAP;
    } else if (flags & kVlinkPortMapConfigured) {
      flag_port_map = SET_FLAG_VLINK_PORTMAP;
    } else {
      // No port map of vlink configuration
      flag_port_map = 0;
    }

    free(vexternal);
    free(vex_if);
    SET_USER_DATA_FLAGS(ikey, flag_port_map);

    controller_domain ctrlr_dom;
    memset(&ctrlr_dom, 0, sizeof(controller_domain));
    result_code = GetControllerDomainID(ikey, req->datatype, dmi);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Failed to Get the Controller Domain details, err:%d",
                     result_code);
    }
    GET_USER_DATA_CTRLR_DOMAIN(ikey, ctrlr_dom);

    UPLL_LOG_DEBUG("ctrlrid %s, domainid %s",
                   ctrlr_dom.ctrlr, ctrlr_dom.domain);

    result_code = UpdateConfigDB(ikey, req->datatype, UNC_OP_CREATE, dmi);
    if (UPLL_RC_SUCCESS != result_code)
      return result_code;
  } else {
    UPLL_LOG_DEBUG("Error in reading DB\n");
  }
  return result_code;
}

bool VbrIfFlowFilterMoMgr::GetRenameKeyBindInfo(unc_key_type_t key_type,
                                    BindInfo *&binfo,
                                    int &nattr,
                                    MoMgrTables tbl) {
  UPLL_FUNC_TRACE;

  /* Main Table only update */
  if (MAINTBL == tbl) {
    nattr = NUM_KEY_MAIN_TBL;
    binfo = vbr_if_flowfiltermaintbl_bind_info;
  } else {
    return PFC_FALSE;
  }

  UPLL_LOG_DEBUG("Successful Completion");
  return PFC_TRUE;
}

upll_rc_t VbrIfFlowFilterMoMgr::ValidateCapability(IpcReqRespHeader *req,
                                 ConfigKeyVal *ikey,
                                 const char *ctrlr_name) {
  UPLL_FUNC_TRACE;

  if ((NULL == req) || (NULL == ikey)) {
    UPLL_LOG_DEBUG("IpcReqRespHeader/ConfigKeyval is NULL");
    return UPLL_RC_ERR_GENERIC;
  }

  if (!ctrlr_name) {
    ctrlr_name = static_cast<char *>(ikey->get_user_data());
  }
  uint32_t dt_type = req->datatype;
  uint32_t operation = req->operation;
  uint32_t option1 = req->option1;
  uint32_t option2 = req->option2;

  UPLL_LOG_DEBUG("dt_type   : (%d)"
               "operation : (%d)"
               "option1   : (%d)"
               "option2   : (%d)",
               dt_type, operation, option1, option2);

  bool result_code = false;
  uint32_t instance_count;
  const uint8_t *attrs = NULL;
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
    default: {
      result_code = GetReadCapability(ctrlr_name, ikey->get_key_type(),
                                      &max_attrs, &attrs);
    }
  }

  if (!result_code) {
    UPLL_LOG_DEBUG("keytype(%d) is not supported by controller(%s) "
                   "for opeartion(%d)",
                   ikey->get_key_type(), ctrlr_name, operation);
    return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_KT;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrIfFlowFilterMoMgr::ValidateMessage(IpcReqRespHeader *req,
                                                ConfigKeyVal *key) {
  UPLL_FUNC_TRACE;
  upll_rc_t rt_code = UPLL_RC_ERR_GENERIC;

  if ((NULL == req) || (NULL == key)) {
    UPLL_LOG_DEBUG("ConfigKeyval is NULL");
    return UPLL_RC_ERR_BAD_REQUEST;
  }

  if (UNC_KT_VBRIF_FLOWFILTER != key->get_key_type()) {
    UPLL_LOG_DEBUG(" Invalid keytype(%d)", key->get_key_type());
    return UPLL_RC_ERR_BAD_REQUEST;
  }
  if (key->get_st_num() != IpctSt::kIpcStKeyVbrIfFlowfilter) {
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
  key_vbr_if_flowfilter_t *key_vbr_if_flowfilter =
      reinterpret_cast<key_vbr_if_flowfilter_t *>(key->get_key());

  /** Validate key structure */
  if (NULL == key_vbr_if_flowfilter) {
    UPLL_LOG_DEBUG("KT_VBRIF_FLOWFILTER Key structure is empty!!");
    return UPLL_RC_ERR_BAD_REQUEST;
  }

    rt_code = ValidateVbrIfFlowfilterKey(key_vbr_if_flowfilter,
                                         req->operation);

    if (UPLL_RC_SUCCESS != rt_code) {
      UPLL_LOG_DEBUG(" key_vbrif_flowfilter syntax validation failed :"
                     "Err Code - %d",
                     rt_code);
      return rt_code;
    }

  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrIfFlowFilterMoMgr::ValidateVbrIfFlowfilterKey(
    key_vbr_if_flowfilter_t* key_vbr_if_flowfilter,
    unc_keytype_operation_t op) {

  UPLL_FUNC_TRACE;

  upll_rc_t rt_code = UPLL_RC_ERR_GENERIC;

  VbrIfMoMgr *mgrvbrif =
      reinterpret_cast<VbrIfMoMgr *>(const_cast<MoManager *>(GetMoManager(
                  UNC_KT_VBR_IF)));

  if (NULL == mgrvbrif) {
    UPLL_LOG_DEBUG("unable to get VtnMoMgr object to validate key_vtn");
    return UPLL_RC_ERR_GENERIC;
  }

  rt_code = mgrvbrif->ValidateVbrifKey(&(key_vbr_if_flowfilter->if_key));

  if (UPLL_RC_SUCCESS != rt_code) {
    UPLL_LOG_DEBUG(" Vbrif_key syntax validation failed :Err Code - %d",
                   rt_code);

    return rt_code;
  }


  if ((op != UNC_OP_READ_SIBLING_COUNT) &&
      (op != UNC_OP_READ_SIBLING_BEGIN)) {
    /** validate direction */
    if (!ValidateNumericRange(key_vbr_if_flowfilter->direction,
                              (uint8_t) UPLL_FLOWFILTER_DIR_IN,
                              (uint8_t) UPLL_FLOWFILTER_DIR_OUT, true, true)) {
      UPLL_LOG_DEBUG("direction syntax validation failed ");
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  } else {
    // input direction should be not set for
    // sibling begin or count operation
    // as 0 or 1 are valid values setting an invalid value;
    key_vbr_if_flowfilter->direction = 0xFE;
  }

  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrIfFlowFilterMoMgr::ValidateAttribute(ConfigKeyVal *ikey,
                                                  DalDmlIntf *dmi,
                                                  IpcReqRespHeader *req) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  ConfigKeyVal *okey = NULL;
  if (!ikey || !ikey->get_key()) {
    UPLL_LOG_DEBUG("input key is null");
    return UPLL_RC_ERR_GENERIC;
  }
  key_vbr_if_flowfilter_t *key_vbrif_ff =
      reinterpret_cast<key_vbr_if_flowfilter_t *>(ikey->get_key());
  MoMgrImpl *mgr = reinterpret_cast<MoMgrImpl *>
      (const_cast<MoManager *>(GetMoManager(UNC_KT_VBR_IF)));

  result_code = mgr->GetChildConfigKey(okey, NULL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetChildConfigKey failed for KT_VBR_IF key struct - %d",
                    result_code);
    return result_code;
  }

  key_vbr_if_t *vbrif_key =
      reinterpret_cast<key_vbr_if_t *>(okey->get_key());

  uuu::upll_strncpy(vbrif_key->vbr_key.vtn_key.vtn_name,
        key_vbrif_ff->if_key.vbr_key.vtn_key.vtn_name,
        kMaxLenVtnName + 1);

    uuu::upll_strncpy(vbrif_key->vbr_key.vbridge_name,
        key_vbrif_ff->if_key.vbr_key.vbridge_name,
        kMaxLenVtnName + 1);

    uuu::upll_strncpy(vbrif_key->if_name,
        key_vbrif_ff->if_key.if_name,
        kMaxLenInterfaceName + 1);

  /* Checks the given vbr_if exists or not */
  result_code = mgr->UpdateConfigDB(okey, req->datatype, UNC_OP_READ,
                                    dmi, MAINTBL);
  if (UPLL_RC_ERR_INSTANCE_EXISTS != result_code) {
    UPLL_LOG_DEBUG(" Parent VBRIF key does not exists");
    delete okey;
    okey = NULL;
    return UPLL_RC_ERR_PARENT_DOES_NOT_EXIST;
  } else {
    result_code = UPLL_RC_SUCCESS;
  }

  delete okey;
  okey = NULL;

  UPLL_LOG_DEBUG("ValidateAttribute Successfull.");
  return result_code;
}

upll_rc_t VbrIfFlowFilterMoMgr::IsReferenced(ConfigKeyVal *ikey,
                                           upll_keytype_datatype_t dt_type,
                                           DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  if (NULL == ikey) return UPLL_RC_ERR_GENERIC;
  DbSubOp dbop = { kOpReadExist, kOpMatchNone, kOpInOutNone };
  // Check the object existence
  result_code = UpdateConfigDB(ikey, dt_type, UNC_OP_READ, dmi, &dbop, MAINTBL);
  if (UPLL_RC_ERR_INSTANCE_EXISTS != result_code) {
    UPLL_LOG_DEBUG("UpdateConfigDB failed %d", result_code);
    return result_code;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrIfFlowFilterMoMgr::AllocVal(ConfigVal *&ck_val,
                                         upll_keytype_datatype_t dt_type,
                                         MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  void *val = NULL;
  if (ck_val != NULL)
    return UPLL_RC_ERR_GENERIC;
  switch (tbl) {
    case MAINTBL:
        val = reinterpret_cast<void *>
            (ConfigKeyVal::Malloc(sizeof(val_flowfilter_t)));
        ck_val = new ConfigVal(IpctSt::kIpcStValFlowfilter, val);

      break;
    default:
      val = NULL;
      break;
  }

  UPLL_LOG_DEBUG(" AllocVal Successfull.");
  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrIfFlowFilterMoMgr::GetChildConfigKey(ConfigKeyVal *&okey,
                                                  ConfigKeyVal *parent_key) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  key_vbr_if_flowfilter_t *vbr_if_ff_key;
  void *pkey = NULL;

  if (parent_key == NULL) {
    vbr_if_ff_key = reinterpret_cast<key_vbr_if_flowfilter_t*>
        (ConfigKeyVal::Malloc(sizeof(key_vbr_if_flowfilter_t)));
    okey = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER,
                            IpctSt::kIpcStKeyVbrIfFlowfilter,
                            vbr_if_ff_key, NULL);
    return UPLL_RC_SUCCESS;
  } else {
    pkey = parent_key->get_key();
  }
  if (!pkey) return UPLL_RC_ERR_GENERIC;

  if (okey) {
    if (okey->get_key_type() != UNC_KT_VBRIF_FLOWFILTER)
      return UPLL_RC_ERR_GENERIC;
    vbr_if_ff_key = reinterpret_cast<key_vbr_if_flowfilter_t *>
        (okey->get_key());
  } else {
    vbr_if_ff_key = reinterpret_cast<key_vbr_if_flowfilter_t *>
        (ConfigKeyVal::Malloc(sizeof(key_vbr_if_flowfilter_t)));
  }

  switch (parent_key->get_key_type()) {
    case UNC_KT_VTN:
      uuu::upll_strncpy(
          vbr_if_ff_key->if_key.vbr_key.vtn_key.vtn_name,
          reinterpret_cast<key_vtn_t *>
          (pkey)->vtn_name,
          kMaxLenVtnName + 1);
      break;
    case UNC_KT_VBRIDGE:
      uuu::upll_strncpy(
          vbr_if_ff_key->if_key.vbr_key.vtn_key.vtn_name,
          reinterpret_cast<key_vbr_t *>
          (pkey)->vtn_key.vtn_name,
          kMaxLenVtnName + 1);
      uuu::upll_strncpy(
          vbr_if_ff_key->if_key.vbr_key.vbridge_name,
          reinterpret_cast<key_vbr_t *>
          (pkey)->vbridge_name,
          kMaxLenVnodeName + 1);
      break;
    case UNC_KT_VBR_IF:
      uuu::upll_strncpy(
          vbr_if_ff_key->if_key.vbr_key.vtn_key.vtn_name,
          reinterpret_cast<key_vbr_if_t *>
          (pkey)->vbr_key.vtn_key.vtn_name,
          kMaxLenVtnName + 1);
      uuu::upll_strncpy(
          vbr_if_ff_key->if_key.vbr_key.vbridge_name,
          reinterpret_cast<key_vbr_if_t *>
          (pkey)->vbr_key.vbridge_name,
          kMaxLenVnodeName + 1);
      uuu::upll_strncpy(vbr_if_ff_key->if_key.if_name,
                        reinterpret_cast<key_vbr_if_t *>
                        (pkey)->if_name,
                        kMaxLenInterfaceName + 1);
      break;
    case UNC_KT_VBRIF_FLOWFILTER:
      uuu::upll_strncpy(
          vbr_if_ff_key->if_key.vbr_key.vtn_key.vtn_name,
          reinterpret_cast<key_vbr_if_flowfilter_t *>
          (pkey)->if_key.vbr_key.vtn_key.vtn_name,
          kMaxLenVtnName + 1);
      uuu::upll_strncpy(
          vbr_if_ff_key->if_key.vbr_key.vbridge_name,
          reinterpret_cast<key_vbr_if_flowfilter_t *>
          (pkey)->if_key.vbr_key.vbridge_name,
          kMaxLenVnodeName + 1);
      uuu::upll_strncpy(vbr_if_ff_key->if_key.if_name,
                        reinterpret_cast<key_vbr_if_flowfilter_t *>
                        (pkey)->if_key.if_name,
                        kMaxLenInterfaceName + 1);
      vbr_if_ff_key->direction =
          reinterpret_cast<key_vbr_if_flowfilter_t *>
          (pkey)->direction;
      break;
    default:
      if (vbr_if_ff_key) free(vbr_if_ff_key);
      return UPLL_RC_ERR_GENERIC;
  }

  if (!okey) {
    okey = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER,
                            IpctSt::kIpcStKeyVbrIfFlowfilter,
                            vbr_if_ff_key, NULL);
  }
  SET_USER_DATA(okey, parent_key);
  UPLL_LOG_DEBUG("okey filled Succesfully %d", result_code);
  return result_code;
}

upll_rc_t VbrIfFlowFilterMoMgr::DupConfigKeyVal(ConfigKeyVal *&okey,
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

  if (req->get_key_type() != UNC_KT_VBRIF_FLOWFILTER) {
    UPLL_LOG_DEBUG(" DupConfigKeyval Failed.");
    return UPLL_RC_ERR_GENERIC;
  }

  ConfigVal *tmp1 = NULL, *tmp = (req)->get_cfg_val();

  if (tmp) {
    if (tbl == MAINTBL) {
      val_flowfilter_t *ival = NULL;
      ival = reinterpret_cast<val_flowfilter_t *> (GetVal(req));
      if (NULL != ival) {
      val_flowfilter_t *vbr_if_flowfilter_val = NULL;
        vbr_if_flowfilter_val = reinterpret_cast<val_flowfilter_t *>
            (ConfigKeyVal::Malloc(sizeof(val_flowfilter_t)));
        memcpy(vbr_if_flowfilter_val, ival, sizeof(val_flowfilter_t));
        tmp1 = new ConfigVal(IpctSt::kIpcStValFlowfilter,
                             vbr_if_flowfilter_val);
      tmp1->set_user_data(tmp->get_user_data());
      }
    }
  }

  void *tkey = req->get_key();
  if (tkey != NULL) {
    key_vbr_if_flowfilter_t *ikey = NULL;
    ikey = reinterpret_cast<key_vbr_if_flowfilter_t *> (tkey);
    key_vbr_if_flowfilter_t *vbr_if_flowfilter =
        reinterpret_cast<key_vbr_if_flowfilter_t*>
          (ConfigKeyVal::Malloc(sizeof(key_vbr_if_flowfilter_t)));

    memcpy(vbr_if_flowfilter, ikey, sizeof(key_vbr_if_flowfilter_t));
    okey = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER,
                          IpctSt::kIpcStKeyVbrIfFlowfilter, vbr_if_flowfilter,
                          tmp1);
    SET_USER_DATA(okey, req);

    UPLL_LOG_DEBUG("DupConfigkeyVal Succesfull.");
    return UPLL_RC_SUCCESS;
  }
  // free(vbr_if_flowfilter_val);
  // free(tmp1);
  DELETE_IF_NOT_NULL(tmp1);
  return UPLL_RC_ERR_GENERIC;
}

upll_rc_t VbrIfFlowFilterMoMgr::UpdateMo(IpcReqRespHeader *req,
                                         ConfigKeyVal *ikey, DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  UPLL_LOG_DEBUG("Implementation Not supported for this KT.");
  return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_KT;
}

upll_rc_t VbrIfFlowFilterMoMgr::RenameMo(IpcReqRespHeader *req,
                                         ConfigKeyVal *ikey, DalDmlIntf *dmi,
                                         const char *ctrlr_id) {
  UPLL_FUNC_TRACE;
  UPLL_LOG_DEBUG("Implementation Not supported for this KT.");
  return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_KT;
}

upll_rc_t VbrIfFlowFilterMoMgr::MergeValidate(unc_key_type_t keytype,
                                              const char *ctrlr_id,
                                              ConfigKeyVal *ikey,
                                              DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  UPLL_LOG_DEBUG("Merge Validate is successfull.");
  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrIfFlowFilterMoMgr::GetRenamedUncKey(
    ConfigKeyVal *ikey, upll_keytype_datatype_t dt_type, DalDmlIntf *dmi,
    uint8_t *ctrlr_id) {
  UPLL_FUNC_TRACE;
  ConfigKeyVal *unc_key = NULL;
  DbSubOp dbop = { kOpReadSingle, kOpMatchCtrlr, kOpInOutNone };

  MoMgrImpl *mgrvbr = reinterpret_cast<MoMgrImpl *>
             (const_cast<MoManager *> (GetMoManager(UNC_KT_VBRIDGE)));
  if (mgrvbr == NULL) {
    return UPLL_RC_ERR_GENERIC;
  }
  val_rename_vnode *rename_val = reinterpret_cast
          <val_rename_vnode*>(ConfigKeyVal::Malloc(sizeof(val_rename_vnode)));

  key_vbr_if_flowfilter_t *ctrlr_key =
      reinterpret_cast<key_vbr_if_flowfilter_t *> (ikey->get_key());
  uuu::upll_strncpy(rename_val->ctrlr_vtn_name,
                    ctrlr_key->if_key.vbr_key.vtn_key.vtn_name,
                    (kMaxLenVtnName + 1));
  uuu::upll_strncpy(rename_val->ctrlr_vnode_name,
                    ctrlr_key->if_key.vbr_key.vbridge_name,
                    (kMaxLenVnodeName + 1));
  if (UPLL_RC_SUCCESS != mgrvbr->GetChildConfigKey(unc_key, NULL)) {
    FREE_IF_NOT_NULL(rename_val);
    UPLL_LOG_DEBUG("Unable to get the configkey for Vribdge");
    return UPLL_RC_ERR_GENERIC;
  }

  if (ctrlr_id == NULL) {
    free(rename_val);
    DELETE_IF_NOT_NULL(unc_key);
    UPLL_LOG_DEBUG("Controller Name is Not Valid");
    return UPLL_RC_ERR_GENERIC;
  }

  unc_key->set_user_data(ctrlr_id);
  unc_key->AppendCfgVal(IpctSt::kIpcStValRenameVtn, rename_val);
  upll_rc_t result_code = mgrvbr->ReadConfigDB(unc_key, dt_type, UNC_OP_READ,
                                               dbop, dmi, RENAMETBL);
  if (result_code == UPLL_RC_SUCCESS) {
    key_vbr_if_flowfilter_t *vbr_if_flowfilter_key =
        reinterpret_cast<key_vbr_if_flowfilter_t *> (unc_key->get_key());
    uuu::upll_strncpy(
                     ctrlr_key->if_key.vbr_key.vtn_key.vtn_name,
                     vbr_if_flowfilter_key->if_key.vbr_key.vtn_key.vtn_name,
                     (kMaxLenVtnName + 1));

    uuu::upll_strncpy(ctrlr_key->if_key.vbr_key.vbridge_name,
                      vbr_if_flowfilter_key->if_key.vbr_key.vbridge_name,
                      (kMaxLenVnodeName + 1));
  }

  UPLL_LOG_DEBUG("Keyis filled with UncKey Successfully %d", result_code);
  free(rename_val);
  delete unc_key;
  return result_code;
}

upll_rc_t VbrIfFlowFilterMoMgr::GetRenamedControllerKey(
    ConfigKeyVal *&ikey, upll_keytype_datatype_t dt_type, DalDmlIntf *dmi,
    controller_domain *ctrlr_dom) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_ERR_GENERIC;
  ConfigKeyVal *okey = NULL;
  uint8_t rename = 0;
  IsRenamed(ikey, dt_type, dmi, rename);

  if (!rename) {
     return UPLL_RC_SUCCESS;
  }
  // Vtn renamed
  key_vbr_if_flowfilter_t *ctrlr_key =
      reinterpret_cast<key_vbr_if_flowfilter_t*>
      (ConfigKeyVal::Malloc(sizeof(key_vbr_if_flowfilter_t)));

  if (rename & VTN_RENAME_FLAG) {
    MoMgrImpl *mgrvtn = reinterpret_cast<MoMgrImpl *>
      (const_cast<MoManager *> (GetMoManager(UNC_KT_VTN)));

    if (mgrvtn == NULL) {
      return UPLL_RC_ERR_GENERIC;
    }

    mgrvtn->GetChildConfigKey(okey, NULL);
    if (ctrlr_dom)
      SET_USER_DATA_CTRLR_DOMAIN(okey, *ctrlr_dom);

    uuu::upll_strncpy(reinterpret_cast<key_vtn *>(okey->get_key())->vtn_name,
                      reinterpret_cast<key_vbr_if_flowfilter_t *>
                      (ikey->get_key())->if_key.vbr_key.vtn_key.vtn_name,
                      (kMaxLenVtnName + 1));

    DbSubOp dbop = { kOpReadSingle, kOpMatchCtrlr, kOpInOutFlag };
    result_code =mgrvtn->ReadConfigDB(okey, dt_type, UNC_OP_READ, dbop,
                                      dmi, RENAMETBL);
    if (result_code != UPLL_RC_SUCCESS) {
      free(ctrlr_key);
      return result_code;
    }
    val_rename_vtn *rename_val = NULL;
    rename_val = reinterpret_cast<val_rename_vtn *> (GetVal(okey));

    if (!rename_val
        || (rename_val->valid[UPLL_IDX_NEW_NAME_RVTN] != UNC_VF_VALID)) {
      UPLL_LOG_DEBUG("Vtn Name is not Valid.");
      free(ctrlr_key);
      return UPLL_RC_ERR_GENERIC;
    }

    uuu::upll_strncpy(
        ctrlr_key->if_key.vbr_key.vtn_key.vtn_name,
        rename_val->new_name,
        (kMaxLenVtnName + 1));
    SET_USER_DATA_FLAGS(ikey, VTN_RENAME);
  }
  // Vbr Renamed
  if (rename & VBR_RENAME_FLAG) {
    MoMgrImpl *mgrvbr = reinterpret_cast<MoMgrImpl *>
      (const_cast<MoManager *> (GetMoManager(UNC_KT_VBRIDGE)));

    if (mgrvbr == NULL) return UPLL_RC_ERR_GENERIC;
    mgrvbr->GetChildConfigKey(okey, NULL);

    if (ctrlr_dom)
      SET_USER_DATA_CTRLR_DOMAIN(okey, *ctrlr_dom);
    uuu::upll_strncpy(
            reinterpret_cast<key_vbr *> (okey->get_key())->vbridge_name,
            reinterpret_cast<key_vbr_if_flowfilter_t *>
            (ikey->get_key())->if_key.vbr_key.vbridge_name,
            (kMaxLenVnodeName + 1));

    DbSubOp dbop = { kOpReadSingle, kOpMatchCtrlr, kOpInOutFlag };
    result_code = mgrvbr->ReadConfigDB(okey, dt_type, UNC_OP_READ,
                                       dbop, dmi, RENAMETBL);
    if (result_code != UPLL_RC_SUCCESS) {
      delete okey;
      UPLL_LOG_DEBUG("Unable to Read from DB");
      return result_code;
    }

    val_rename_vbr *rename_val = NULL;
    rename_val = reinterpret_cast<val_rename_vbr *> (GetVal(okey));

    if (!rename_val
        || (rename_val->valid[UPLL_IDX_NEW_NAME_RVBR] != UNC_VF_VALID)) {
      UPLL_LOG_DEBUG("Vbr Name is not Valid.");
      free(ctrlr_key);
      return UPLL_RC_ERR_GENERIC;
    }

    uuu::upll_strncpy(ctrlr_key->if_key.vbr_key.vbridge_name,
                      rename_val->new_name,
                      (kMaxLenVnodeName + 1));
    SET_USER_DATA_FLAGS(ikey, VTN_RENAME);
  }

  UPLL_LOG_DEBUG("Renamed Controller key is sucessfull.");
  // free(ctrlr_key);
  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrIfFlowFilterMoMgr::UpdateAuditConfigStatus(
    unc_keytype_configstatus_t cs_status,
    uuc::UpdateCtrlrPhase phase,
    ConfigKeyVal *&ckv_running) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  val_flowfilter_t *vbr_flowfilter_val = NULL;
  vbr_flowfilter_val = (ckv_running != NULL)?
     reinterpret_cast<val_flowfilter_t *> (GetVal(ckv_running)):NULL;

  if (NULL == vbr_flowfilter_val) {
    UPLL_LOG_DEBUG("val strct is empty");
    return UPLL_RC_ERR_GENERIC;
  }

  if (uuc::kUpllUcpCreate == phase)
    vbr_flowfilter_val->cs_row_status = cs_status;

  UPLL_LOG_DEBUG("AuditUpdate Config Status Information %d", result_code);
  return result_code;
}

upll_rc_t VbrIfFlowFilterMoMgr::ReadMo(IpcReqRespHeader *req,
                                       ConfigKeyVal *ikey, DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  controller_domain ctrlr_dom;
  uint8_t db_flag = 0;
  ConfigKeyVal *l_key = NULL, *dup_key = NULL, *flag_key = NULL;
  DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutCtrlr|kOpInOutDomain };
  result_code = ValidateMessage(req, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("MoMgrImpl::ReadMo - ValidateMessage %d", result_code);
    return result_code;
  }

  memset(&ctrlr_dom, 0, sizeof(controller_domain));
  switch (req->datatype) {
    // Retrieving config information
    case UPLL_DT_CANDIDATE:
    case UPLL_DT_RUNNING:
    case UPLL_DT_STARTUP:
    case UPLL_DT_STATE:
      if ((req->option1 == UNC_OPT1_NORMAL)&&
          (req->option2 == UNC_OPT2_NONE)) {
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

        result_code = ReadConfigDB(dup_key, req->datatype,
                                   UNC_OP_READ, dbop, dmi, MAINTBL);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("ReadConfigDB  fail in ReadMo for dup_key");
          delete dup_key;
          return result_code;
        }
        result_code =  DupConfigKeyVal(l_key, ikey, MAINTBL);
        if (result_code != UPLL_RC_SUCCESS) {
          DELETE_IF_NOT_NULL(dup_key);
          UPLL_LOG_DEBUG("DupConfigKeyVal fail in ReadMo for l_key");
          return result_code;
        }
        GET_USER_DATA_CTRLR_DOMAIN(dup_key, ctrlr_dom);
        SET_USER_DATA_CTRLR_DOMAIN(l_key, ctrlr_dom);
        // 1.Getting renamed name if renamed
        result_code = GetRenamedControllerKey(l_key, req->datatype,
                                              dmi, &ctrlr_dom);
        if (result_code != UPLL_RC_SUCCESS) {
          DELETE_IF_NOT_NULL(dup_key);
          DELETE_IF_NOT_NULL(l_key);
          return result_code;
        }
        // 2.send request to driver
        // ///////////////////////

        result_code =  DupConfigKeyVal(flag_key, ikey, MAINTBL);
        if (result_code != UPLL_RC_SUCCESS) {
          DELETE_IF_NOT_NULL(dup_key);
          DELETE_IF_NOT_NULL(l_key);
          UPLL_LOG_DEBUG(" DupConfigKeyVal failed for l_key%d ", result_code);
          return result_code;
        }

        DbSubOp dbop2 = { kOpReadMultiple, kOpMatchNone, kOpInOutFlag };
        result_code = ReadConfigDB(flag_key, req->datatype ,
                                   UNC_OP_READ, dbop2, dmi, MAINTBL);
        if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
          UPLL_LOG_DEBUG("No Recrods in the Vbr_If_FlowFilter_Entry Table");
          DELETE_IF_NOT_NULL(dup_key);
          DELETE_IF_NOT_NULL(l_key);
          DELETE_IF_NOT_NULL(flag_key);
          return UPLL_RC_SUCCESS;
        }

        GET_USER_DATA_FLAGS(flag_key, db_flag);
        pfcdrv_val_vbrif_vextif *pfc_val =
            reinterpret_cast<pfcdrv_val_vbrif_vextif *>
            (ConfigKeyVal::Malloc(sizeof(pfcdrv_val_vbrif_vextif)));

        result_code = GetVexternalInformation(dup_key, UPLL_DT_CANDIDATE,
                                              pfc_val, db_flag, dmi);
        if (UPLL_RC_SUCCESS != result_code) {
          DELETE_IF_NOT_NULL(dup_key);
          DELETE_IF_NOT_NULL(l_key);
          DELETE_IF_NOT_NULL(flag_key);
          return result_code;
        }

        l_key->SetCfgVal(new ConfigVal(IpctSt::kIpcStPfcdrvValVbrifVextif,
                                       pfc_val));
        // ///////////////////////////
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
          DELETE_IF_NOT_NULL(dup_key);
          DELETE_IF_NOT_NULL(l_key);
          DELETE_IF_NOT_NULL(flag_key);
          return UPLL_RC_ERR_GENERIC;
        }
        if (ipc_resp.header.result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("Driver response for Key %d controller %s result %d",
                        l_key->get_key_type(), ctrlr_dom.ctrlr,
                        ipc_resp.header.result_code);
          DELETE_IF_NOT_NULL(dup_key);
          DELETE_IF_NOT_NULL(l_key);
          DELETE_IF_NOT_NULL(flag_key);
          return ipc_resp.header.result_code;
        }
        ConfigKeyVal *okey = NULL;
        result_code = ConstructReadDetailResponse(dup_key,
                                                  ipc_resp.ckv_data,
                                                  req->datatype,
                                                  req->operation,
                                                  dbop, dmi, &okey);

        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("ConstructReadDetailResponse error code (%d)",
                         result_code);
          DELETE_IF_NOT_NULL(dup_key);
          DELETE_IF_NOT_NULL(l_key);
          DELETE_IF_NOT_NULL(flag_key);
          return result_code;
        } else {
          if (okey != NULL) {
            ikey->ResetWith(okey);
          }
        }
        DELETE_IF_NOT_NULL(dup_key);
        DELETE_IF_NOT_NULL(l_key);
        DELETE_IF_NOT_NULL(flag_key);
      }
      break;
    default:
      result_code = UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT;
  }
  return result_code;
}

bool VbrIfFlowFilterMoMgr::IsValidKey(void *key,
                                      uint64_t index) {
  UPLL_FUNC_TRACE;
  upll_rc_t ret_val = UPLL_RC_SUCCESS;
  key_vbr_if_flowfilter_t *vbr_ff_key =
      reinterpret_cast<key_vbr_if_flowfilter_t *>(key);
  if (vbr_ff_key == NULL) {
    return false;
  }

  switch (index) {
    case uudst::vbr_if_flowfilter::kDbiVtnName:
      ret_val = ValidateKey(reinterpret_cast<char *>
                            (vbr_ff_key->if_key.vbr_key.vtn_key.vtn_name),
                            kMinLenVtnName, kMaxLenVtnName);
      if (ret_val != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("VTN Name is not valid(%d)", ret_val);
        return false;
      }
      break;
    case uudst::vbr_if_flowfilter::kDbiVbrName:
      ret_val = ValidateKey(reinterpret_cast<char *>
                            (vbr_ff_key->if_key.vbr_key.vbridge_name),
                            kMinLenVnodeName, kMaxLenVnodeName);
      if (ret_val != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("VBR Name is not valid(%d)", ret_val);
        return false;
      }
      break;
    case uudst::vbr_if_flowfilter::kDbiVbrIfName:
      ret_val = ValidateKey(reinterpret_cast<char *>
                            (vbr_ff_key->if_key.if_name),
                            kMinLenInterfaceName, kMaxLenInterfaceName);
      if (ret_val != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("VBR interface name is not valid(%d)", ret_val);
        return false;
      }
      break;
    case uudst::vbr_if_flowfilter::kDbiInputDirection:
      if (vbr_ff_key->direction == 0xFE) {
        // if operation is read sibling begin or
        // read sibling count return false
        // for output binding
        vbr_ff_key->direction = 0;
        return false;
      } else {
        if (!ValidateNumericRange(vbr_ff_key->direction,
                                  (uint8_t) UPLL_FLOWFILTER_DIR_IN,
                                  (uint8_t) UPLL_FLOWFILTER_DIR_OUT,
                                  true, true)) {
          UPLL_LOG_DEBUG(" input direction syntax validation failed ");
          return false;
        }
      }
      break;
    default:
      UPLL_LOG_DEBUG("Invalid Key Index");
      return false;
  }
  return true;
}


upll_rc_t VbrIfFlowFilterMoMgr::UpdateConfigStatus(ConfigKeyVal *key,
                                                   unc_keytype_operation_t op,
                                                   uint32_t driver_result,
                                                   ConfigKeyVal *upd_key,
                                                   DalDmlIntf *dmi,
                                                   ConfigKeyVal *ctrlr_key) {
  UPLL_FUNC_TRACE;
  val_flowfilter_t *vbr_if_flowfilter_val = NULL;
  unc_keytype_configstatus_t cs_status =
      (driver_result == 0) ? UNC_CS_APPLIED : UNC_CS_NOT_APPLIED;
  vbr_if_flowfilter_val = reinterpret_cast<val_flowfilter_t *> (GetVal(key));
  if (vbr_if_flowfilter_val == NULL) return UPLL_RC_ERR_GENERIC;

  if (op == UNC_OP_CREATE) {
    if (vbr_if_flowfilter_val->cs_row_status != UNC_CS_NOT_SUPPORTED)
      vbr_if_flowfilter_val->cs_row_status = cs_status;
  } else {
    UPLL_LOG_DEBUG("Operation Not Supported.");
    return UPLL_RC_ERR_GENERIC;
  }

  UPLL_LOG_DEBUG("Update Config Status Successfull.");
  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrIfFlowFilterMoMgr::CopyToConfigKey(ConfigKeyVal *&okey,
                                              ConfigKeyVal *ikey) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  if (!ikey || !(ikey->get_key())) {
    UPLL_LOG_DEBUG("Input Key Not Valid");
    return UPLL_RC_ERR_GENERIC;
  }

  key_rename_vnode_info *key_rename = NULL;
  key_rename = reinterpret_cast<key_rename_vnode_info *> (ikey->get_key());
  key_vbr_if_flowfilter_t * key_vbr_if =
      reinterpret_cast<key_vbr_if_flowfilter_t*>
      (ConfigKeyVal::Malloc(sizeof(key_vbr_if_flowfilter_t)));

    if (!strlen(reinterpret_cast<char *> (key_rename->old_unc_vtn_name))) {
    UPLL_LOG_DEBUG("String Length not Valid to Perform the Operation");
     free(key_vbr_if);
     return UPLL_RC_ERR_GENERIC;
  }

  uuu::upll_strncpy(key_vbr_if->if_key.vbr_key.vtn_key.vtn_name,
                    key_rename->old_unc_vtn_name,
                    (kMaxLenVtnName + 1));

  if (ikey->get_key_type() == table[MAINTBL]->get_key_type()) {
    if (!strlen(reinterpret_cast<char *> (key_rename->old_unc_vnode_name))) {
      free(key_vbr_if);
      return UPLL_RC_ERR_GENERIC;
    }
    uuu::upll_strncpy(key_vbr_if->if_key.vbr_key.vbridge_name,
                      key_rename->old_unc_vnode_name,
                      (kMaxLenVnodeName + 1));
  }

  okey = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER,
                           IpctSt::kIpcStKeyVbrIfFlowfilter,
                           key_vbr_if, NULL);
  if (!okey) {
    free(key_vbr_if);
    return UPLL_RC_ERR_GENERIC;
  }

  // free(key_vbr_if); // Should not free this memory
  return result_code;
}
upll_rc_t VbrIfFlowFilterMoMgr::ConstructReadDetailResponse(
    ConfigKeyVal *ikey,
    ConfigKeyVal *drv_resp_ckv,
    upll_keytype_datatype_t dt_type,
    unc_keytype_operation_t op,
    DbSubOp dbop,
    DalDmlIntf *dmi,
    ConfigKeyVal **okey) {
  UPLL_FUNC_TRACE;
  ConfigKeyVal *tmp_okey = NULL;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  result_code =  GetChildConfigKey(tmp_okey, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetChildConfigKey failed err code (%d)", result_code);
    return result_code;
  }

  val_flowfilter_t *val_ff = reinterpret_cast<val_flowfilter_t *>
      (ConfigKeyVal::Malloc(sizeof(val_flowfilter_t)));
  val_flowfilter_t *tmp_val_ff = reinterpret_cast<val_flowfilter_t *>
      (GetVal(ikey));
  if (!tmp_val_ff) {
    UPLL_LOG_DEBUG(" Invalid value read from DB");
    delete tmp_okey;
    free(val_ff);
    return UPLL_RC_ERR_GENERIC;
  }
  memcpy(val_ff, tmp_val_ff, sizeof(val_flowfilter_t));
  tmp_okey->AppendCfgVal(IpctSt::kIpcStValFlowfilter, val_ff);

  ConfigVal *drv_resp_val = NULL;
  drv_resp_val =  drv_resp_ckv->get_cfg_val();
  while (drv_resp_val != NULL) {
    val_flowfilter_entry_st_t *val_ffe_st = NULL;
    if (IpctSt::kIpcStValFlowfilterEntrySt == drv_resp_val->get_st_num()) {
      val_ffe_st = reinterpret_cast<val_flowfilter_entry_st_t *>
          (drv_resp_val->get_val());
    } else {
      UPLL_LOG_DEBUG("Incorrect structure received from driver, struct num %d",
                     drv_resp_val->get_st_num());
      return  UPLL_RC_ERR_GENERIC;
    }

    if ((val_ffe_st)->valid[UPLL_IDX_SEQ_NUM_FFES] == UNC_VF_VALID) {
      ConfigKeyVal *tmp_ffe_key = NULL;

      key_vbr_if_flowfilter_t *key_vbrif_ff =
          reinterpret_cast<key_vbr_if_flowfilter_t*>(ikey->get_key());

      key_vbr_if_flowfilter_entry_t *key_vbrif_ffe =
          reinterpret_cast<key_vbr_if_flowfilter_entry_t*>
          (ConfigKeyVal::Malloc(sizeof(key_vbr_if_flowfilter_entry_t)));
      tmp_ffe_key = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER_ENTRY,
                              IpctSt::kIpcStKeyVbrIfFlowfilterEntry,
                              key_vbrif_ffe, NULL);
      key_vbrif_ffe->sequence_num = val_ffe_st->sequence_num;
      uuu::upll_strncpy(
          key_vbrif_ffe->flowfilter_key.if_key.vbr_key.vtn_key.vtn_name,
          key_vbrif_ff->if_key.vbr_key.vtn_key.vtn_name,
          (kMaxLenVtnName+1));

      uuu::upll_strncpy(
          key_vbrif_ffe->flowfilter_key.if_key.vbr_key.vbridge_name,
          key_vbrif_ff->if_key.vbr_key.vbridge_name,
          (kMaxLenVnodeName+1));

      uuu::upll_strncpy(key_vbrif_ffe->flowfilter_key.if_key.if_name,
                        key_vbrif_ff->if_key.if_name,
                        (kMaxLenInterfaceName +1));

      key_vbrif_ffe->flowfilter_key.direction =
          (reinterpret_cast<key_vbr_if_flowfilter*>(
                  ikey->get_key()))->direction;

      VbrIfFlowFilterEntryMoMgr *mgr =
          reinterpret_cast<VbrIfFlowFilterEntryMoMgr*>
          (const_cast<MoManager *>(GetMoManager
                                   (UNC_KT_VBRIF_FLOWFILTER_ENTRY)));
      result_code = mgr->ReadDetailEntry(tmp_ffe_key, dt_type,
                                         UNC_OP_READ, dbop, dmi);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG(" flowfilter entry read detail error (%d)", result_code);
        delete tmp_ffe_key;
        return result_code;
      }

      val_flowfilter_entry_st_t *tmp_ffe_st =
          reinterpret_cast<val_flowfilter_entry_st_t* >
          (ConfigKeyVal::Malloc(sizeof(val_flowfilter_entry_st_t)));
      memcpy(tmp_ffe_st, val_ffe_st, sizeof(val_flowfilter_entry_st_t));

      tmp_okey->AppendCfgVal(IpctSt::kIpcStValFlowfilterEntrySt, tmp_ffe_st);
      val_flowfilter_entry_t* tmp_val_ffe =
          reinterpret_cast <val_flowfilter_entry_t*>
          (ConfigKeyVal::Malloc(sizeof(val_flowfilter_entry_t)));
      memcpy(tmp_val_ffe,
             reinterpret_cast<val_flowfilter_entry_t*>
             (tmp_ffe_key->get_cfg_val()->get_val()),
             sizeof(val_flowfilter_entry_t));

      tmp_okey->AppendCfgVal(IpctSt::kIpcStValFlowfilterEntry, tmp_val_ffe);

      delete tmp_ffe_key;
      tmp_ffe_key = NULL;

      if ((drv_resp_val = drv_resp_val->get_next_cfg_val()) == NULL) {
        UPLL_LOG_DEBUG("No more entries in driver response\n");
        break;
      }

      if (IpctSt::kIpcStValFlowlistEntrySt != drv_resp_val->get_st_num()) {
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
        tmp_okey->AppendCfgVal(IpctSt::kIpcStValFlowlistEntrySt, tmp_val_fl_st);
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

upll_rc_t VbrIfFlowFilterMoMgr::ReadSiblingMo(IpcReqRespHeader *req,
                                              ConfigKeyVal *ikey,
                                              bool begin, DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  uint8_t db_flag = 0;
  controller_domain ctrlr_dom;
  ConfigKeyVal *dup_key = NULL, *l_key = NULL, *tmp_key = NULL;
  ConfigKeyVal *okey = NULL, *flag_key = NULL, *tctrl_key = NULL;
  DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutCtrlr|kOpInOutDomain  };

  result_code = ValidateMessage(req, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_TRACE("validate Message Failed %d", result_code);
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
                 (req->option1 == UNC_OPT1_DETAIL) &&
                 (req->option2 == UNC_OPT2_NONE)) {
        result_code =  DupConfigKeyVal(tctrl_key, ikey, MAINTBL);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG(" DupConfigKeyVal failed for l_key%d ", result_code);
          return result_code;
        }

        result_code = ReadInfoFromDB(req, tctrl_key, dmi, &ctrlr_dom);
        if (UPLL_RC_SUCCESS != result_code) {
          UPLL_LOG_DEBUG("ReadConfigDb failed for tctrl_key%d ", result_code);
          DELETE_IF_NOT_NULL(tctrl_key);
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
          UPLL_LOG_DEBUG("DupConfigKeyVal fail in ReadSiblingMo for l_key");
          DELETE_IF_NOT_NULL(tctrl_key);
          DELETE_IF_NOT_NULL(dup_key);
          return result_code;
        }
        GET_USER_DATA_CTRLR_DOMAIN(dup_key, ctrlr_dom);
        SET_USER_DATA_CTRLR_DOMAIN(l_key, ctrlr_dom);
        // 1.Getting renamed name if renamed
        result_code = GetRenamedControllerKey(l_key, req->datatype,
                                              dmi, &ctrlr_dom);
        if (result_code != UPLL_RC_SUCCESS) {
          DELETE_IF_NOT_NULL(l_key);
          DELETE_IF_NOT_NULL(tctrl_key);
          DELETE_IF_NOT_NULL(dup_key);
          return result_code;
        }

        // 2.send request to driver
        // ///////////////////////

        result_code =  DupConfigKeyVal(flag_key, ikey, MAINTBL);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG(" DupConfigKeyVal failed for l_key%d ", result_code);
          DELETE_IF_NOT_NULL(l_key);
          DELETE_IF_NOT_NULL(tctrl_key);
          DELETE_IF_NOT_NULL(dup_key);
          return result_code;
        }

        DbSubOp dbop2 = { kOpReadSingle, kOpMatchNone, kOpInOutFlag };
        result_code = ReadConfigDB(flag_key, req->datatype ,
                                   UNC_OP_READ, dbop2, dmi, MAINTBL);
        if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
          UPLL_LOG_DEBUG("No Recrods in the Vbr_If_FlowFilter_Entry Table");
          DELETE_IF_NOT_NULL(l_key);
          DELETE_IF_NOT_NULL(flag_key);
          DELETE_IF_NOT_NULL(tctrl_key);
          DELETE_IF_NOT_NULL(dup_key);
          return UPLL_RC_SUCCESS;
        }

        GET_USER_DATA_FLAGS(flag_key, db_flag);
        pfcdrv_val_vbrif_vextif *pfc_val =
            reinterpret_cast<pfcdrv_val_vbrif_vextif *>
            (ConfigKeyVal::Malloc(sizeof(pfcdrv_val_vbrif_vextif)));

        result_code = GetVexternalInformation(tctrl_key, UPLL_DT_CANDIDATE,
                                              pfc_val, db_flag, dmi);
        if (UPLL_RC_SUCCESS != result_code) {
          UPLL_LOG_DEBUG("GetVexternalInformation failed err(%d)", result_code);
          DELETE_IF_NOT_NULL(l_key);
          DELETE_IF_NOT_NULL(flag_key);
          DELETE_IF_NOT_NULL(tctrl_key);
          DELETE_IF_NOT_NULL(dup_key);
          return result_code;
        }

        l_key->SetCfgVal(new ConfigVal(IpctSt::kIpcStPfcdrvValVbrifVextif,
                                       pfc_val));
        // ///////////////////////////
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
          reinterpret_cast<key_vbr_if_flowfilter_t*>
              (l_key->get_key())->direction =
              reinterpret_cast<key_vbr_if_flowfilter_t*>
              (tmp_key->get_key())->direction;
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
            DELETE_IF_NOT_NULL(flag_key);
            DELETE_IF_NOT_NULL(tctrl_key);
            DELETE_IF_NOT_NULL(dup_key);
            return UPLL_RC_ERR_GENERIC;
          }

          if (ipc_resp.header.result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("Driver response for Key %d controller %s result %d",
                          l_key->get_key_type(), ctrlr_dom.ctrlr,
                          ipc_resp.header.result_code);
            DELETE_IF_NOT_NULL(l_key);
            DELETE_IF_NOT_NULL(flag_key);
            DELETE_IF_NOT_NULL(tctrl_key);
            DELETE_IF_NOT_NULL(dup_key);
            return ipc_resp.header.result_code;
          }

          result_code = ConstructReadDetailResponse(tmp_key,
                                                    ipc_resp.ckv_data,
                                                    req->datatype,
                                                    req->operation,
                                                    dbop, dmi, &okey);

          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("ConstructReadDetailResponse error code (%d)",
                           result_code);
            DELETE_IF_NOT_NULL(l_key);
            DELETE_IF_NOT_NULL(flag_key);
            DELETE_IF_NOT_NULL(tctrl_key);
            DELETE_IF_NOT_NULL(dup_key);
            return result_code;
          }
          tmp_key = tmp_key->get_next_cfg_key_val();
        }
        if ((okey != NULL) && (result_code == UPLL_RC_SUCCESS)) {
          ikey->ResetWith(okey);
        }
        DELETE_IF_NOT_NULL(l_key);
        DELETE_IF_NOT_NULL(flag_key);
        DELETE_IF_NOT_NULL(tctrl_key);
        DELETE_IF_NOT_NULL(dup_key);
      }
      break;
    default:
      result_code = UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT;
  }
  return result_code;
}

upll_rc_t VbrIfFlowFilterMoMgr::TxUpdateController(unc_key_type_t keytype,
                                        uint32_t session_id,
                                        uint32_t config_id,
                                        uuc::UpdateCtrlrPhase phase,
                                        set<string> *affected_ctrlr_set,
                                        DalDmlIntf *dmi,
                                        ConfigKeyVal **err_ckv)  {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  DalResultCode db_result;
  ConfigKeyVal *req = NULL, *nreq = NULL, *ck_main = NULL;
  controller_domain_t ctrlr_dom;
  ctrlr_dom.ctrlr = NULL;
  ctrlr_dom.domain = NULL;
  DalCursor *dal_cursor_handle = NULL;
  uint8_t db_flag = 0;
  uint8_t db_flag_running = 0;
  upll_keytype_datatype_t vext_datatype =  UPLL_DT_CANDIDATE;
  if (affected_ctrlr_set == NULL)
      return UPLL_RC_ERR_GENERIC;
  unc_keytype_operation_t op = (phase == uuc::kUpllUcpCreate)?UNC_OP_CREATE:
               ((phase == uuc::kUpllUcpUpdate)?UNC_OP_UPDATE:
               ((phase == uuc::kUpllUcpDelete)?UNC_OP_DELETE:UNC_OP_INVALID));

  unc_keytype_operation_t op1 = op;
  if (UNC_OP_UPDATE == op) {
    // update operation is not supported
    // return success
    // return UPLL_RC_SUCCESS;
  }

  result_code = DiffConfigDB(UPLL_DT_CANDIDATE, UPLL_DT_RUNNING,
                     op, req, nreq, &dal_cursor_handle, dmi, MAINTBL);

  while (result_code == UPLL_RC_SUCCESS) {
    ck_main = NULL;
    db_result = dmi->GetNextRecord(dal_cursor_handle);
    result_code = DalToUpllResCode(db_result);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG(" GetNextRecord failed err code(%d)", result_code);
      break;
    }
    switch (op)   {
      case UNC_OP_CREATE:
      case UNC_OP_UPDATE:
        op1 = op;
        result_code = DupConfigKeyVal(ck_main, req, MAINTBL);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_TRACE("DupConfigKeyVal failed %d\n", result_code);
          return result_code;
        }
        break;
      case UNC_OP_DELETE:
        op1 = op;
        result_code = GetChildConfigKey(ck_main, req);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_TRACE("GetChildConfigKey failed %d\n", result_code);
          return result_code;
        }
      default:
        break;
    }

    /*
      DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutNone};
      result_code = ReadConfigDB(ck_main, UPLL_DT_RUNNING, UNC_OP_READ,
                                             dbop, dmi, MAINTBL);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Returning error %d\n", result_code);
        return UPLL_RC_ERR_GENERIC;
      }

    */
    GET_USER_DATA_CTRLR_DOMAIN(ck_main, ctrlr_dom);
    if (ctrlr_dom.ctrlr == NULL) {
        return UPLL_RC_ERR_GENERIC;
    }
    db_flag = 0;
    GET_USER_DATA_FLAGS(ck_main, db_flag);
    ConfigKeyVal *tmp_cfgkeyval = NULL;
    if (!(SET_FLAG_PORTMAP & db_flag) && !(SET_FLAG_VLINK & db_flag)) {
      if (op1 != UNC_OP_UPDATE) {
        continue;
      } else {
        result_code = GetChildConfigKey(tmp_cfgkeyval, ck_main);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("GetChildConfigKey failed, err %d", result_code);
          return result_code;
        }
        SET_USER_DATA_CTRLR_DOMAIN(tmp_cfgkeyval, ctrlr_dom);
        DbSubOp dbop1 = { kOpReadSingle, kOpMatchCtrlr|kOpMatchDomain,
          kOpInOutFlag};
        result_code = ReadConfigDB(tmp_cfgkeyval, UPLL_DT_RUNNING,
                                   UNC_OP_READ, dbop1, dmi, MAINTBL);
        if (result_code != UPLL_RC_SUCCESS) {
          if (result_code != UPLL_RC_ERR_INSTANCE_EXISTS) {
            UPLL_LOG_DEBUG("Unable to read from DB, err: %d", result_code);
            return result_code;
          }
        }
        GET_USER_DATA_FLAGS(tmp_cfgkeyval, db_flag_running);
        if (!(SET_FLAG_PORTMAP & db_flag_running) &&
            !(SET_FLAG_VLINK & db_flag_running)) {
          UPLL_LOG_DEBUG("portmap flag is not available at running as well ");
          continue;
        }
        op1 = UNC_OP_DELETE;
        vext_datatype = UPLL_DT_RUNNING;
        db_flag = db_flag_running;
      }
    } else if (op1 == UNC_OP_UPDATE) {
        result_code = GetChildConfigKey(tmp_cfgkeyval, ck_main);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("GetChildConfigKey failed, err %d", result_code);
            return result_code;
          }
          SET_USER_DATA_CTRLR_DOMAIN(tmp_cfgkeyval, ctrlr_dom);
          DbSubOp dbop1 = { kOpReadSingle, kOpMatchCtrlr|kOpMatchDomain,
            kOpInOutFlag};
          result_code = ReadConfigDB(tmp_cfgkeyval, UPLL_DT_RUNNING,
                                     UNC_OP_READ, dbop1, dmi, MAINTBL);
          if (result_code != UPLL_RC_SUCCESS) {
             if (result_code != UPLL_RC_ERR_INSTANCE_EXISTS) {
               UPLL_LOG_DEBUG("Unable to read from DB, err: %d", result_code);
               return result_code;
             }
          }
          GET_USER_DATA_FLAGS(tmp_cfgkeyval, db_flag_running);
          if (!(SET_FLAG_PORTMAP & db_flag_running) &&
              !(SET_FLAG_VLINK & db_flag_running)) {
             UPLL_LOG_DEBUG("Portmap flag is not set at running");
             op1 = UNC_OP_CREATE;
             vext_datatype = UPLL_DT_CANDIDATE;
          }
    }


    /*pfcdrv_val_flowfilter_entry_t *pfc_val =
        reinterpret_cast<pfcdrv_val_flowfilter_entry_t *>
        (ConfigKeyVal::Malloc(sizeof(pfcdrv_val_flowfilter_entry_t)));

    */
    pfcdrv_val_vbrif_vextif *pfc_val =
        reinterpret_cast<pfcdrv_val_vbrif_vextif *>
        (ConfigKeyVal::Malloc(sizeof(pfcdrv_val_vbrif_vextif)));

    result_code = GetVexternalInformation(ck_main, vext_datatype,
                                          pfc_val, db_flag, dmi);
    if (UPLL_RC_SUCCESS != result_code) {
      return result_code;
    }
    upll_keytype_datatype_t dt_type = (op1 == UNC_OP_DELETE)?
             UPLL_DT_RUNNING:UPLL_DT_CANDIDATE;
    result_code = GetRenamedControllerKey(ck_main, dt_type,
                                          dmi, &ctrlr_dom);
    if (result_code != UPLL_RC_SUCCESS)
      return result_code;

    ck_main->SetCfgVal(new ConfigVal(IpctSt::kIpcStPfcdrvValVbrifVextif,
                                     pfc_val));
    UPLL_LOG_DEBUG("Controller : %s; Domain : %s", ctrlr_dom.ctrlr,
                 ctrlr_dom.domain);

    // Inserting the controller to Set
    affected_ctrlr_set->insert
      (string(reinterpret_cast<char *>(ctrlr_dom.ctrlr)));

    IpcResponse ipc_resp;
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
  dmi->CloseCursor(dal_cursor_handle, true);
  if (req)
    delete req;
  if (nreq)
    delete nreq;
  result_code = (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) ?
                 UPLL_RC_SUCCESS : result_code;
  return result_code;
}

upll_rc_t VbrIfFlowFilterMoMgr::SetVlinkPortmapConfiguration(
    ConfigKeyVal *ikey,
    upll_keytype_datatype_t dt_type,
    DalDmlIntf *dmi, InterfacePortMapInfo flags) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_ERR_GENERIC;
  if (NULL == ikey || NULL == ikey->get_key()) {
    return result_code;
  }
  if (!flags) {
    //  return UPLL_RC_SUCCESS;
  }
  ConfigKeyVal *ckv = NULL;
  result_code = GetChildConfigKey(ckv, NULL);
  if (UPLL_RC_SUCCESS != result_code) {
    return result_code;
  }
  key_vbr_if_flowfilter_t *ff_key = reinterpret_cast<key_vbr_if_flowfilter_t *>
      (ckv->get_key());
  key_vbr_if_t *vbrif_key = reinterpret_cast<key_vbr_if_t *>(ikey->get_key());

  uuu::upll_strncpy(ff_key->if_key.vbr_key.vtn_key.vtn_name,
                    vbrif_key->vbr_key.vtn_key.vtn_name,
                    kMaxLenVtnName + 1);

  uuu::upll_strncpy(ff_key->if_key.vbr_key.vbridge_name,
                    vbrif_key->vbr_key.vbridge_name,
                    kMaxLenVtnName + 1);

  uuu::upll_strncpy(ff_key->if_key.if_name,
                    vbrif_key->if_name,
                    kMaxLenInterfaceName + 1);
  DbSubOp dbop = { kOpReadMultiple, kOpMatchNone, kOpInOutFlag };
  // The below statement allows to read the partial key with
  // direction as output only
  ff_key->direction = 0xFE;
  result_code = ReadConfigDB(ckv, dt_type ,
                             UNC_OP_READ, dbop, dmi, MAINTBL);
  if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
    UPLL_LOG_DEBUG("No Recrods in the Vbr_If_FlowFilter Table");
    return UPLL_RC_SUCCESS;
  }
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("Read ConfigDB failure %d", result_code);
    delete ckv;
    return result_code;
  }
  uint8_t flag_port_map = 0;
  while (ckv) {
    GET_USER_DATA_FLAGS(ckv, flag_port_map);
    if (flags & kVlinkConfigured) {
      UPLL_LOG_DEBUG("only vlink");
      flag_port_map = SET_FLAG_VLINK;
    } else if (flags & kPortMapConfigured) {
      UPLL_LOG_DEBUG("only portmap");
      flag_port_map = SET_FLAG_PORTMAP;
    } else if (flags & kVlinkPortMapConfigured) {
      UPLL_LOG_DEBUG("portmap with vlink");
      flag_port_map = SET_FLAG_VLINK_PORTMAP;
    } else {
      // No port map of vlink configuration
      UPLL_LOG_DEBUG("NO portmap");
      flag_port_map = 0;
    }

    uint8_t rename_flag = 0;
    GET_USER_DATA_FLAGS(ckv, rename_flag);
    rename_flag |= flag_port_map;
    SET_USER_DATA_FLAGS(ckv, flag_port_map);

    DbSubOp dbop_up = { kOpNotRead, kOpMatchNone, kOpInOutFlag };
    result_code = UpdateConfigDB(ckv, dt_type, UNC_OP_UPDATE,
                                 dmi, &dbop_up, MAINTBL);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("failed to update the portmap configuration");
      return result_code;
    }

    VbrIfFlowFilterEntryMoMgr *mgr =
        reinterpret_cast<VbrIfFlowFilterEntryMoMgr *>
        (const_cast<MoManager *>(GetMoManager(UNC_KT_VBRIF_FLOWFILTER_ENTRY)));
    if (mgr == NULL) {
      return UPLL_RC_ERR_GENERIC;
    }
    result_code = mgr->SetVlinkPortmapConfiguration(ikey, dt_type, dmi, flags);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("update portmap flag for flowfilterentry failed, err %d",
                     result_code);
      return result_code;
    }
    ckv = ckv->get_next_cfg_key_val();
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrIfFlowFilterMoMgr::GetVexternalInformation(ConfigKeyVal* ck_main,
                 upll_keytype_datatype_t dt_type,
                 pfcdrv_val_vbrif_vextif *& pfc_val,
                 uint8_t db_flag, DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_ERR_GENERIC;
  ConfigKeyVal *ckv = NULL;
  InterfacePortMapInfo flags = kVlinkPortMapNotConfigured;

  if (db_flag & SET_FLAG_PORTMAP)  {
    UPLL_LOG_DEBUG("portMap is configured!!!");
    VbrIfMoMgr *mgrvbrif = reinterpret_cast<VbrIfMoMgr *>
        (const_cast<MoManager *>(GetMoManager(UNC_KT_VBR_IF)));
    if (mgrvbrif == NULL) {
      UPLL_LOG_DEBUG("Unable to get the instance of VBRIF");
      return result_code;
    }
    result_code = mgrvbrif->GetChildConfigKey(ckv, NULL);
    key_vbr_if_t *vbr_if_key = reinterpret_cast<key_vbr_if_t *>
        (ckv->get_key());
    key_vbr_if_flowfilter_t * temp_key =
        reinterpret_cast<key_vbr_if_flowfilter_t*>(ck_main->get_key());

    uuu::upll_strncpy(vbr_if_key->vbr_key.vtn_key.vtn_name,
                      temp_key->if_key.vbr_key.vtn_key.vtn_name,
                      kMaxLenVtnName + 1);

    uuu::upll_strncpy(vbr_if_key->vbr_key.vbridge_name,
                      temp_key->if_key.vbr_key.vbridge_name,
                      kMaxLenVtnName + 1);

    uuu::upll_strncpy(vbr_if_key->if_name,
                      temp_key->if_key.if_name,
                      kMaxLenInterfaceName + 1);

    uint8_t* vexternal = reinterpret_cast<uint8_t*>
        (ConfigKeyVal::Malloc(kMaxLenVnodeName + 1));
    uint8_t* vex_if = reinterpret_cast<uint8_t*>
        (ConfigKeyVal::Malloc(kMaxLenInterfaceName + 1));

    result_code = mgrvbrif->GetVexternal(ckv, dt_type, dmi,
                                         vexternal, vex_if,
                                         flags);
    if (UPLL_RC_SUCCESS != result_code) {
      FREE_IF_NOT_NULL(vexternal);
      FREE_IF_NOT_NULL(vex_if);
      DELETE_IF_NOT_NULL(ckv);
      UPLL_LOG_DEBUG("Failed to get VExternal details, err:%d", result_code);
      return result_code;
    }

    // pfc_val->valid[PFCDRV_IDX_VAL_VBRIF_VEXTIF_FFE] = UNC_VF_VALID;

    pfc_val->valid[PFCDRV_IDX_INTERFACE_TYPE] = UNC_VF_VALID;
    pfc_val->interface_type = PFCDRV_IF_TYPE_VEXTIF;
    pfc_val->valid[PFCDRV_IDX_VEXTERNAL_NAME_VBRIF] = UNC_VF_VALID;
    uuu::upll_strncpy(pfc_val->vexternal_name,
                      vexternal,
                      kMaxLenVnodeName + 1);
    pfc_val->valid[PFCDRV_IDX_VEXT_IF_NAME_VBRIF] = UNC_VF_VALID;
    uuu::upll_strncpy(pfc_val->vext_if_name,
                      vex_if,
                      kMaxLenInterfaceName + 1);
    // pfc_val->val_vbrif_vextif.valid[PFCDRV_IDX_INTERFACE_TYPE] =
    // UNC_VF_VALID;
    // pfc_val->val_vbrif_vextif.interface_type = PFCDRV_IF_TYPE_VEXTIF;
    free(vexternal);
    free(vex_if);
    DELETE_IF_NOT_NULL(ckv);
  } else if (db_flag & SET_FLAG_VLINK) {
    UPLL_LOG_DEBUG("vlink is configured!");
    pfc_val->valid[PFCDRV_IDX_VEXTERNAL_NAME_VBRIF] = UNC_VF_INVALID;
    pfc_val->valid[PFCDRV_IDX_INTERFACE_TYPE] = UNC_VF_VALID;

    pfc_val->interface_type = PFCDRV_IF_TYPE_VBRIF;
  } else {
     UPLL_LOG_DEBUG("Portmap/Vlink is not configured");
     return UPLL_RC_ERR_NOT_ALLOWED_AT_THIS_TIME;
  }
  UPLL_LOG_DEBUG("GetVExternalInformation returned successfully");
  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrIfFlowFilterMoMgr::GetControllerDomainID(ConfigKeyVal *ikey,
                                          upll_keytype_datatype_t dt_type,
                                          DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  controller_domain ctrlr_dom;
  memset(&ctrlr_dom, 0, sizeof(controller_domain));
  VbrIfMoMgr *mgrvbrif =
      reinterpret_cast<VbrIfMoMgr *>(const_cast<MoManager *>(GetMoManager(
                  UNC_KT_VBR_IF)));

  ConfigKeyVal *ckv = NULL;
  result_code = mgrvbrif->GetChildConfigKey(ckv, NULL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Unable to get the ParentConfigKey, resultcode=%d",
                    result_code);
    return result_code;
  }

  key_vbr_if_flowfilter_t *temp_key =
      reinterpret_cast<key_vbr_if_flowfilter_t*>(ikey->get_key());

  key_vbr_if_t *vbr_if_key = reinterpret_cast<key_vbr_if_t*>(ckv->get_key());

  uuu::upll_strncpy(vbr_if_key->vbr_key.vtn_key.vtn_name,
                    temp_key->if_key.vbr_key.vtn_key.vtn_name,
                    kMaxLenVtnName + 1);
  uuu::upll_strncpy(vbr_if_key->vbr_key.vbridge_name,
                    temp_key->if_key.vbr_key.vbridge_name,
                    kMaxLenVnodeName + 1);

  uuu::upll_strncpy(vbr_if_key->if_name,
                    temp_key->if_key.if_name,
                    kMaxLenInterfaceName + 1);


  ConfigKeyVal *vbr_key = NULL;
  result_code = mgrvbrif->GetParentConfigKey(vbr_key, ckv);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetParentConfigKey Failed, err %d", result_code);
    DELETE_IF_NOT_NULL(ckv);
    return result_code;
  }

  result_code = mgrvbrif->GetControllerDomainId(vbr_key, dt_type,
                                                &ctrlr_dom, dmi);
  if ((result_code != UPLL_RC_SUCCESS) || (ctrlr_dom.ctrlr == NULL)
      || (ctrlr_dom.domain == NULL)) {
    DELETE_IF_NOT_NULL(ckv);
    DELETE_IF_NOT_NULL(vbr_key);
    UPLL_LOG_DEBUG("GetControllerDomainId error err code(%d)", result_code);
    return UPLL_RC_ERR_GENERIC;
  }

  UPLL_LOG_DEBUG("ctrlrid %s, domainid %s",
                   ctrlr_dom.ctrlr, ctrlr_dom.domain);
  SET_USER_DATA_CTRLR_DOMAIN(ikey, ctrlr_dom);

  DELETE_IF_NOT_NULL(ckv);
  DELETE_IF_NOT_NULL(vbr_key);
  return result_code;
}

upll_rc_t VbrIfFlowFilterMoMgr::GetParentConfigKey(ConfigKeyVal *&okey,
                                                   ConfigKeyVal *ikey) {
  UPLL_FUNC_TRACE;

  if (!ikey) {
    UPLL_LOG_DEBUG(" Input Key is NULL");
    return UPLL_RC_ERR_GENERIC;
  }

  unc_key_type_t ikey_type = ikey->get_key_type();
  if (ikey_type != UNC_KT_VBRIF_FLOWFILTER) {
    UPLL_LOG_DEBUG(" Invalid key type received. Key type - %d", ikey_type);
    return UPLL_RC_ERR_GENERIC;
  }

  key_vbr_if_flowfilter_t *pkey =
      reinterpret_cast<key_vbr_if_flowfilter_t*>(ikey->get_key());
  if (!pkey) {
    UPLL_LOG_DEBUG(" Input vbr if flow filter key is NULL ");
    return UPLL_RC_ERR_GENERIC;
  }

  key_vbr_if_t *vbr_if_key = reinterpret_cast<key_vbr_if_t*>
      (ConfigKeyVal::Malloc(sizeof(key_vbr_if_t)));

  uuu::upll_strncpy(vbr_if_key->vbr_key.vtn_key.vtn_name,
                    reinterpret_cast<key_vbr_if_flowfilter_t *>
                    (pkey)->if_key.vbr_key.vtn_key.vtn_name,
                    kMaxLenVtnName + 1);
  uuu::upll_strncpy(vbr_if_key->vbr_key.vbridge_name,
                    reinterpret_cast<key_vbr_if_flowfilter_t *>
                    (pkey)->if_key.vbr_key.vbridge_name,
                    kMaxLenVnodeName + 1);
  uuu::upll_strncpy(vbr_if_key->if_name,
                    reinterpret_cast<key_vbr_if_flowfilter_t *>
                    (pkey)->if_key.if_name,
                    kMaxLenInterfaceName + 1);
  okey = new ConfigKeyVal(UNC_KT_VBR_IF,
                          IpctSt::kIpcStKeyVbrIf,
                          vbr_if_key, NULL);
  SET_USER_DATA(okey, ikey);
  return UPLL_RC_SUCCESS;
}
}  // namespace kt_momgr
}  // namespace upll
}  // namespace unc
