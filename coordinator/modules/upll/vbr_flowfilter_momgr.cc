/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include "vbr_flowfilter_momgr.hh"
#include "vbr_flowfilter_entry_momgr.hh"
#include "vbr_momgr.hh"
#include "upll_validation.hh"
#include "uncxx/upll_log.hh"

namespace unc {
namespace upll {
namespace kt_momgr {
using unc::upll::ipc_util::IpcUtil;

#define VTN_RENAME_FLAG   0x01
#define VBR_RENAME_FLAG   0x02

// VbrFlowFilterMoMgr Table(Main Table)
BindInfo VbrFlowFilterMoMgr::vbr_flowfilter_bind_info[] = {
  { uudst::vbr_flowfilter::kDbiVtnName, CFG_KEY,
    offsetof(key_vbr_flowfilter_t, vbr_key.vtn_key.vtn_name),
    uud::kDalChar, (kMaxLenVtnName + 1) },
  { uudst::vbr_flowfilter::kDbiVbrName, CFG_KEY,
    offsetof(key_vbr_flowfilter_t, vbr_key.vbridge_name),
    uud::kDalChar, (kMaxLenVnodeName + 1) },
  { uudst::vbr_flowfilter::kDbiInputDirection, CFG_KEY,
    offsetof(key_vbr_flowfilter_t, direction),
    uud::kDalUint8, 1 },
  { uudst::vbr_flowfilter::kDbiCtrlrName, CK_VAL,
    offsetof(key_user_data_t, ctrlr_id),
    uud::kDalChar, (kMaxLenCtrlrId + 1) },
  { uudst::vbr_flowfilter::kDbiDomainId, CK_VAL,
    offsetof(key_user_data, domain_id),
    uud::kDalChar, (kMaxLenDomainId + 1) },
  { uudst::vbr_flowfilter::kDbiFlags, CK_VAL,
    offsetof(key_user_data_t, flags),
    uud::kDalUint8, 1 },
  { uudst::vbr_flowfilter::kDbiCsRowStatus, CS_VAL,
    offsetof(val_flowfilter_t, cs_row_status),
    uud::kDalUint8, 1 }
};

BindInfo VbrFlowFilterMoMgr::vbr_flowfilter_maintbl_bind_info[] = {
  { uudst::vbr_flowfilter::kDbiVtnName, CFG_MATCH_KEY,
    offsetof(key_vbr_flowfilter_t, vbr_key.vtn_key.vtn_name),
    uud::kDalChar, (kMaxLenVtnName + 1) },
  { uudst::vbr_flowfilter::kDbiVbrName, CFG_MATCH_KEY,
    offsetof(key_vbr_flowfilter_t, vbr_key.vbridge_name),
    uud::kDalChar, (kMaxLenVnodeName + 1) },
  { uudst::vbr_flowfilter::kDbiVtnName, CFG_INPUT_KEY,
    offsetof(key_rename_vnode_info_t, new_unc_vtn_name),
    uud::kDalChar, (kMaxLenVtnName + 1) },
  { uudst::vbr_flowfilter::kDbiVbrName, CFG_INPUT_KEY, offsetof(
    key_rename_vnode_info_t, new_unc_vnode_name),
    uud::kDalChar, (kMaxLenVnodeName + 1) },
  { uudst::vbr_flowfilter::kDbiFlags, CK_VAL,   // changed
    offsetof(key_user_data_t, flags),
    uud::kDalUint8, 1}
};

unc_key_type_t VbrFlowFilterMoMgr::vbr_flowfilter_child[] = {
  UNC_KT_VBR_FLOWFILTER_ENTRY };

VbrFlowFilterMoMgr::VbrFlowFilterMoMgr() : MoMgrImpl() {
  UPLL_FUNC_TRACE;
  // Rename and ctrlr tables not required for this KT
  ntable = (MAX_MOMGR_TBLS);
  table = new Table *[ntable]();

  // For Main Table
  table[MAINTBL] = new Table(uudst::kDbiVbrFlowFilterTbl,
      UNC_KT_VBR_FLOWFILTER, vbr_flowfilter_bind_info,
      IpctSt::kIpcStKeyVbrFlowfilter, IpctSt::kIpcStValFlowfilter,
      uudst::vbr_flowfilter::kDbiVbrFlowFilterNumCols);

  table[RENAMETBL] = NULL;

  table[CTRLRTBL] = NULL;

  table[CONVERTTBL] = NULL;

  nchild = sizeof(vbr_flowfilter_child) / sizeof(vbr_flowfilter_child[0]);
  child = vbr_flowfilter_child;
}

bool VbrFlowFilterMoMgr::GetRenameKeyBindInfo(unc_key_type_t key_type,
                                              BindInfo *&binfo,
                                              int &nattr,
                                              MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  nattr = sizeof(vbr_flowfilter_maintbl_bind_info)/
          sizeof(vbr_flowfilter_maintbl_bind_info[0]);
  binfo = vbr_flowfilter_maintbl_bind_info;

  UPLL_LOG_DEBUG("Successful Completion");
  return PFC_TRUE;
}

upll_rc_t VbrFlowFilterMoMgr::IsReferenced(IpcReqRespHeader *req,
                                           ConfigKeyVal *ikey,
                                           DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  if (NULL == ikey) return UPLL_RC_ERR_GENERIC;
  DbSubOp dbop = { kOpReadExist, kOpMatchNone, kOpInOutNone };
  // Check the  object existence
  result_code = UpdateConfigDB(ikey, req->datatype, UNC_OP_READ,
                               dmi, &dbop, MAINTBL);
  if (UPLL_RC_ERR_INSTANCE_EXISTS != result_code) {
    UPLL_LOG_DEBUG("No such instance to delete");
    return result_code;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrFlowFilterMoMgr::CreateCandidateMo(IpcReqRespHeader *req,
                                       ConfigKeyVal *ikey,
                                       DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  controller_domain ctrlr_dom;
  ctrlr_dom.ctrlr = NULL;
  ctrlr_dom.domain = NULL;
//  std::cout << "MoMgrImpl::CreateCandidateMo\n";
  if (ikey == NULL || req == NULL) {
        return UPLL_RC_ERR_GENERIC;
  }
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  if (req->datatype != UPLL_DT_IMPORT) {
    // validate syntax and semantics
    result_code = ValidateMessage(req, ikey);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("ValidateMessage failed, Error - %d", result_code);
      return result_code;
    }
  }
  result_code = ValidateAttribute(ikey, dmi, req);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("ValidateAttribute failed, Error - %d", result_code);
    return result_code;
  }

  // create a record in CANDIDATE DB
  VbrMoMgr *mgrvbr =
      reinterpret_cast<VbrMoMgr *>(const_cast<MoManager *>(GetMoManager(
                  UNC_KT_VBRIDGE)));
  if (NULL == mgrvbr) {
     UPLL_LOG_ERROR("Unable to get VbrMoMgr instance");
     return UPLL_RC_ERR_GENERIC;
  }

  ConfigKeyVal *ckv = NULL;
  result_code = mgrvbr->GetChildConfigKey(ckv, NULL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Unable to get the ParentConfigKey, resultcode=%d",
                   result_code);
    return result_code;
  }

  key_vbr_flowfilter_t *ff_key = reinterpret_cast
      <key_vbr_flowfilter_t *>(ikey->get_key());
  key_vbr_t *vbr_key = reinterpret_cast<key_vbr_t *>(ckv->get_key());

  uuu::upll_strncpy(vbr_key->vtn_key.vtn_name,
                    ff_key->vbr_key.vtn_key.vtn_name,
                    kMaxLenVtnName + 1);

  uuu::upll_strncpy(vbr_key->vbridge_name,
                    ff_key->vbr_key.vbridge_name,
                    kMaxLenVnodeName + 1);
  // Read Controller ID and Domain ID from the VBridge and set it to the
  // Input ConfigKeyVal
  DbSubOp dbop = { kOpReadSingle, kOpMatchNone,
    kOpInOutCtrlr|kOpInOutDomain };
  result_code =  mgrvbr->ReadConfigDB(ckv, req->datatype, UNC_OP_READ,
                                      dbop, dmi, MAINTBL);

  if (result_code != UPLL_RC_SUCCESS) {
    if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
      result_code = UPLL_RC_ERR_PARENT_DOES_NOT_EXIST;
    }
    UPLL_LOG_DEBUG("Unable to Read the details from DB for the parent err %d",
                   result_code);
    DELETE_IF_NOT_NULL(ckv);
    return result_code;
  }
  result_code = mgrvbr->GetControllerDomainId(ckv, &ctrlr_dom);

  // check whether this keytype is not requested under unified vbridge
  if (IsUnifiedVbr(ctrlr_dom.ctrlr)) {
     UPLL_LOG_DEBUG(
            "This KT is not allowed to be created under Unified Vbridge");
     DELETE_IF_NOT_NULL(ckv);
     return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_KT;
  }
  // GET_USER_DATA_CTRLR_DOMAIN(ckv, ctrlr_dom);
  SET_USER_DATA_CTRLR_DOMAIN(ikey, ctrlr_dom);

  UPLL_LOG_DEBUG("ctrlrid %s, domainid %s",
                 ctrlr_dom.ctrlr, ctrlr_dom.domain);

  result_code = ValidateCapability(
      req, ikey,
      reinterpret_cast<const char *>(ctrlr_dom.ctrlr));

  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("validate Capability Failed %d", result_code);
    DELETE_IF_NOT_NULL(ckv);
    return result_code;
  }

  // Set the rename flag info
  result_code = SetRenameFlag(ikey, dmi, req);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("SetRenameFlag failed");
    return result_code;
  }
  TcConfigMode config_mode = TC_CONFIG_INVALID;
  std::string vtn_name = "";
  result_code = GetConfigModeInfo(req, config_mode, vtn_name);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetConfigMode failed");
    return result_code;
  }

  result_code = UpdateConfigDB(ikey, req->datatype, UNC_OP_CREATE, dmi,
                               config_mode, vtn_name);
  DELETE_IF_NOT_NULL(ckv);
  return result_code;
}

upll_rc_t VbrFlowFilterMoMgr::GetControllerDomainID(ConfigKeyVal *ikey,
                                          upll_keytype_datatype_t dt_type,
                                          DalDmlIntf *dmi) {
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  VbrMoMgr *mgrvbr =
    reinterpret_cast<VbrMoMgr *>(const_cast<MoManager *>(GetMoManager(
              UNC_KT_VBRIDGE)));
  if (NULL == mgrvbr) {
    UPLL_LOG_DEBUG("mgrvbr is NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  ConfigKeyVal *ckv = NULL;
  result_code = mgrvbr->GetChildConfigKey(ckv, NULL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Unable to get the ParentConfigKey, resultcode=%d",
                    result_code);
    return result_code;
  }

  key_vbr_flowfilter_t *ff_key = reinterpret_cast
    <key_vbr_flowfilter_t *>(ikey->get_key());
  key_vbr_t *vbr_key = reinterpret_cast<key_vbr_t *>(ckv->get_key());

  uuu::upll_strncpy(vbr_key->vtn_key.vtn_name,
      ff_key->vbr_key.vtn_key.vtn_name,
      kMaxLenVtnName + 1);

  uuu::upll_strncpy(vbr_key->vbridge_name,
      ff_key->vbr_key.vbridge_name,
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

  controller_domain ctrlr_dom;
  memset(&ctrlr_dom, 0, sizeof(controller_domain));
  result_code = mgrvbr->GetControllerDomainId(ckv, &ctrlr_dom);
  SET_USER_DATA_CTRLR_DOMAIN(ikey, ctrlr_dom);
  UPLL_LOG_DEBUG("ctrlrid %s, domainid %s",
                  ctrlr_dom.ctrlr, ctrlr_dom.domain);
  DELETE_IF_NOT_NULL(ckv);
  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrFlowFilterMoMgr::CreateAuditMoImpl(ConfigKeyVal *ikey,
                                   DalDmlIntf *dmi,
                                   const char *ctrlr_id) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  UPLL_LOG_TRACE(" ikey is %s", ikey->ToStrAll().c_str());
  string vtn_name = "";

  result_code = GetControllerDomainID(ikey, UPLL_DT_AUDIT, dmi);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Failed to Get the Controller Domain details, err:%d",
                   result_code);
    return result_code;
  }

  UPLL_LOG_DEBUG("GetControllerDomainId Passed");
  controller_domain ctrlr_dom;
  memset(&ctrlr_dom, 0, sizeof(controller_domain));
  GET_USER_DATA_CTRLR_DOMAIN(ikey, ctrlr_dom);
  UPLL_LOG_DEBUG("ctrlrid %s, domainid %s",
                 ctrlr_dom.ctrlr, ctrlr_dom.domain);

  // Create a record in AUDIT DB
  result_code = UpdateConfigDB(ikey, UPLL_DT_AUDIT, UNC_OP_CREATE,
                               dmi, TC_CONFIG_GLOBAL, vtn_name, MAINTBL);

  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("UpdateConfigDB Failed err_code %d", result_code);
    return result_code;
  }

  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrFlowFilterMoMgr::ValidateAttribute(ConfigKeyVal *ikey,
                                                DalDmlIntf *dmi,
                                                IpcReqRespHeader *req) {
  // No operation
  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrFlowFilterMoMgr::SetRenameFlag(ConfigKeyVal *ikey,
                                            DalDmlIntf *dmi,
                                            IpcReqRespHeader *req) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *pkey = NULL;
  result_code = GetParentConfigKey(pkey, ikey);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetParentConfigKey failed %d", result_code);
    return result_code;
  }
  MoMgrImpl *mgr =
    reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>(GetMoManager(
            UNC_KT_VBRIDGE)));
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
  SET_USER_DATA_FLAGS(ikey, rename);
  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrFlowFilterMoMgr::ValidateCapability(IpcReqRespHeader *req,
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

  if (NULL == ctrlr_name) {
    UPLL_LOG_DEBUG("ctrlr_name is NULL");
    return UPLL_RC_ERR_GENERIC;
  }

  UPLL_LOG_TRACE("ctrlr_name : (%s)"
               "operation : (%d)",
               ctrlr_name, req->operation);

  bool result_code = false;
  uint32_t instance_count;
  const uint8_t *attrs = NULL;
  uint32_t max_attrs = 0;

  switch (req->operation) {
    case UNC_OP_CREATE: {
      UPLL_LOG_TRACE(
          "Calling GetCreateCapability Operation  %d ", req->operation);
      result_code = GetCreateCapability(ctrlr_name, ikey->get_key_type(),
                                        &instance_count, &max_attrs, &attrs);
      break;
    }
    default: {
      if (req->datatype == UPLL_DT_STATE) {
        UPLL_LOG_TRACE(
            "Calling GetStateCapability Operation  %d ", req->operation);
        result_code = GetStateCapability(ctrlr_name, ikey->get_key_type(),
                                      &max_attrs, &attrs);
      } else {
        UPLL_LOG_TRACE(
            "Calling GetReadCapability Operation  %d ", req->operation);
        result_code = GetReadCapability(ctrlr_name, ikey->get_key_type(),
                                      &max_attrs, &attrs);
      }
    }
  }

  if (!result_code) {
    UPLL_LOG_DEBUG(" keytype(%d) is not supported by controller(%s) "
                   "for opeartion(%d) And Result_code (%d)",
                   ikey->get_key_type(),
                   ctrlr_name, req->operation, result_code);
    return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrFlowFilterMoMgr::ValidateMessage(IpcReqRespHeader *req,
                                              ConfigKeyVal *key) {
  UPLL_FUNC_TRACE;
  upll_rc_t rt_code = UPLL_RC_ERR_GENERIC;

  if ((NULL == req) || (NULL == key)) {
    UPLL_LOG_DEBUG("IpcReqRespHeader/ConfigKeyval is NULL");
    return UPLL_RC_ERR_BAD_REQUEST;
  }

  if (UNC_KT_VBR_FLOWFILTER != key->get_key_type()) {
    UPLL_LOG_DEBUG(" Invalid keytype(%d)", key->get_key_type());
    return UPLL_RC_ERR_BAD_REQUEST;
  }

  if (key->get_st_num() != IpctSt::kIpcStKeyVbrFlowfilter) {
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
  key_vbr_flowfilter_t *key_vbr_flowfilter =
      reinterpret_cast<key_vbr_flowfilter_t *>(key->get_key());

  /** Validate keyStruct fields*/
  if (NULL == key_vbr_flowfilter) {
    UPLL_LOG_DEBUG("KT_VBR_FLOWFILTER Key structure is empty!!");
    return UPLL_RC_ERR_BAD_REQUEST;
  }

    rt_code = ValidateVbrFlowfilterKey(key_vbr_flowfilter,
                                       req->operation);

    if (UPLL_RC_SUCCESS != rt_code) {
      UPLL_LOG_DEBUG(" key_vbr_flowfilter syntax validation failed :"
                     "Err Code - %d",
                     rt_code);
      return rt_code;
    }

  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrFlowFilterMoMgr::ValidateVbrFlowfilterKey(
    key_vbr_flowfilter_t* key_vbr_flowfilter,
    unc_keytype_operation_t op) {
  UPLL_FUNC_TRACE;
  upll_rc_t rt_code = UPLL_RC_ERR_GENERIC;

  /** validate vbr_key */
  VbrMoMgr *mgrvbr =
      reinterpret_cast<VbrMoMgr *>(const_cast<MoManager*>(GetMoManager(
                  UNC_KT_VBRIDGE)));
  if (NULL == mgrvbr) {
    UPLL_LOG_DEBUG("unable to get VtnMoMgr object to validate key_vtn");
    return UPLL_RC_ERR_GENERIC;
  }

  rt_code = mgrvbr->ValidateVbrKey(&(key_vbr_flowfilter->vbr_key));

  if (UPLL_RC_SUCCESS != rt_code) {
    UPLL_LOG_DEBUG(" Vbr name syntax validation failed :"
                   "Err Code - %d",
                   rt_code);

    return rt_code;
  }

  if ((op != UNC_OP_READ_SIBLING_COUNT) &&
      (op != UNC_OP_READ_SIBLING_BEGIN)) {
    /** Validate Direction */
    // for VBR flowfilter only In direction is allowed.
    if (!ValidateNumericRange(key_vbr_flowfilter->direction,
                              (uint8_t) UPLL_FLOWFILTER_DIR_IN,
                              (uint8_t) UPLL_FLOWFILTER_DIR_OUT, true, false)) {
      UPLL_LOG_DEBUG("direction syntax validation failed:Err Code - %d",
                     rt_code);

      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  } else {
    // input direction should be not set for
    // sibling begin or count operation
    // as 0 or 1 are valid values setting an invalid value;
    key_vbr_flowfilter->direction = 0xFE;
  }
  return UPLL_RC_SUCCESS;
}

bool VbrFlowFilterMoMgr::IsValidKey(void *key,
                                    uint64_t index, MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  upll_rc_t ret_val = UPLL_RC_SUCCESS;
  key_vbr_flowfilter_t  *vbr_ff_key =
      reinterpret_cast<key_vbr_flowfilter_t *>(key);
  if (vbr_ff_key == NULL)
    return false;

  switch (index) {
    case uudst::vbr_flowfilter::kDbiVtnName:
      ret_val = ValidateKey(reinterpret_cast<char *>
                            (vbr_ff_key->vbr_key.vtn_key.vtn_name),
                            kMinLenVtnName, kMaxLenVtnName);
      if (ret_val != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("VTN Name is not valid(%d)", ret_val);
        return false;
      }
      break;
    case uudst::vbr_flowfilter::kDbiVbrName:
      ret_val = ValidateKey(reinterpret_cast<char *>
                            (vbr_ff_key->vbr_key.vbridge_name),
                            kMinLenVnodeName, kMaxLenVnodeName);
      if (ret_val != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("VBR Name is not valid(%d)", ret_val);
        return false;
      }
      break;
    case uudst::vbr_flowfilter::kDbiInputDirection:
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

upll_rc_t VbrFlowFilterMoMgr::AllocVal(ConfigVal *&ck_val,
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
      return UPLL_RC_ERR_GENERIC;
  }

  UPLL_LOG_DEBUG(" AllocVal Successfull.");
  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrFlowFilterMoMgr::GetChildConfigKey(ConfigKeyVal *&okey,
                                                ConfigKeyVal *parent_key) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  key_vbr_flowfilter_t *vbr_ff_key;
  void *pkey = NULL;
  if (parent_key == NULL) {
    vbr_ff_key = reinterpret_cast<key_vbr_flowfilter_t*>
        (ConfigKeyVal::Malloc(sizeof(key_vbr_flowfilter_t)));
    // If no direction is specified , 0xFE is filled to bind output direction
    vbr_ff_key->direction = 0xFE;
    okey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER,
                            IpctSt::kIpcStKeyVbrFlowfilter,
                            vbr_ff_key, NULL);
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
    if (okey->get_key_type() != UNC_KT_VBR_FLOWFILTER)
      return UPLL_RC_ERR_GENERIC;
  }
  if ((okey) && (okey->get_key())) {
    vbr_ff_key = reinterpret_cast<key_vbr_flowfilter_t *>
        (okey->get_key());
  } else {
    vbr_ff_key = reinterpret_cast<key_vbr_flowfilter_t *>
        (ConfigKeyVal::Malloc(sizeof(key_vbr_flowfilter_t)));
    // If no direction is specified , 0xFE is filled to bind output direction
    vbr_ff_key->direction = 0xFE;
  }
  switch (parent_key->get_key_type()) {
    case UNC_KT_VTN:
      uuu::upll_strncpy(vbr_ff_key->vbr_key.vtn_key.vtn_name,
                        reinterpret_cast<key_vtn_t *>
                        (pkey)->vtn_name,
                        kMaxLenVtnName + 1);
      break;
    case UNC_KT_VBRIDGE:
      uuu::upll_strncpy(vbr_ff_key->vbr_key.vtn_key.vtn_name,
                        reinterpret_cast<key_vbr_t *>
                        (pkey)->vtn_key.vtn_name,
                        kMaxLenVtnName + 1);
      uuu::upll_strncpy(vbr_ff_key->vbr_key.vbridge_name,
                        reinterpret_cast<key_vbr_t *>
                        (pkey)->vbridge_name,
                        (kMaxLenVnodeName + 1));
      break;
    case UNC_KT_VBR_FLOWFILTER:
      uuu::upll_strncpy(vbr_ff_key->vbr_key.vtn_key.vtn_name,
                        reinterpret_cast<key_vbr_flowfilter_t *>
                        (pkey)->vbr_key.vtn_key.vtn_name,
                        kMaxLenVtnName + 1);
      uuu::upll_strncpy(vbr_ff_key->vbr_key.vbridge_name,
                        reinterpret_cast<key_vbr_flowfilter_t *>
                        (pkey)->vbr_key.vbridge_name,
                        (kMaxLenVnodeName + 1));
      vbr_ff_key->direction =
          reinterpret_cast<key_vbr_flowfilter_t *>(pkey)->direction;
      break;
    default:
      if (vbr_ff_key) free(vbr_ff_key);
      return UPLL_RC_ERR_GENERIC;
  }

  if ((okey) && !(okey->get_key())) {
    UPLL_LOG_DEBUG("okey not null and flow list name updated");
    okey->SetKey(IpctSt::kIpcStKeyVbrFlowfilter, vbr_ff_key);
  }

  if (!okey)
    okey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER,
                            IpctSt::kIpcStKeyVbrFlowfilter,
                            vbr_ff_key, NULL);

  SET_USER_DATA(okey, parent_key);
  UPLL_LOG_DEBUG("okey filled Succesfully %d", result_code);
  return result_code;
}

upll_rc_t VbrFlowFilterMoMgr::DupConfigKeyVal(ConfigKeyVal *&okey,
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

  if (req->get_key_type() != UNC_KT_VBR_FLOWFILTER) {
    UPLL_LOG_DEBUG(" DupConfigKeyval Failed.");
    return UPLL_RC_ERR_GENERIC;
  }

  ConfigVal *tmp1 = NULL, *tmp = (req)->get_cfg_val();
#if 0
  if (NULL == tmp) {
    UPLL_LOG_DEBUG("memory Not Allocated");
    return UPLL_RC_ERR_GENERIC;
  }
#endif
  if (tmp) {
    if (tbl == MAINTBL) {
      val_flowfilter_t *ival = NULL;
      ival = reinterpret_cast<val_flowfilter_t *> (GetVal(req));
      if (NULL != ival) {
        val_flowfilter_t *vbr_flowfilter_val =
            reinterpret_cast<val_flowfilter_t *>
            (ConfigKeyVal::Malloc(sizeof(val_flowfilter_t)));
        memcpy(vbr_flowfilter_val, ival, sizeof(val_flowfilter_t));
        tmp1 = new ConfigVal(IpctSt::kIpcStValFlowfilter,
                             vbr_flowfilter_val);
      }
      if (NULL == tmp1) {
        UPLL_LOG_DEBUG("Memory Not Allocated");
        return UPLL_RC_ERR_GENERIC;
      }
      tmp1->set_user_data(tmp->get_user_data());
    }
  }

  void *tkey = req->get_key();
  if (tkey != NULL) {
    key_vbr_flowfilter_t *ikey =
    reinterpret_cast<key_vbr_flowfilter_t *> (tkey);
    key_vbr_flowfilter_t *vbr_flowfilter =
    reinterpret_cast<key_vbr_flowfilter_t*>
        (ConfigKeyVal::Malloc(sizeof(key_vbr_if_flowfilter_t)));

    memcpy(vbr_flowfilter, ikey, sizeof(key_vbr_flowfilter_t));
    okey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER,
                          IpctSt::kIpcStKeyVbrFlowfilter,
                          vbr_flowfilter, tmp1);
  }
  if (okey) {
    SET_USER_DATA(okey, req)
  } else {
    DELETE_IF_NOT_NULL(tmp1);
    return UPLL_RC_ERR_GENERIC;
  }
  UPLL_LOG_DEBUG("DupConfigkeyVal Succesfull.");
  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrFlowFilterMoMgr::UpdateMo(IpcReqRespHeader *req,
                                       ConfigKeyVal *ikey, DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  UPLL_LOG_DEBUG("Implementation Not supported for this KT.");
  return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_KT;
}

upll_rc_t VbrFlowFilterMoMgr::RenameMo(IpcReqRespHeader *req,
                                       ConfigKeyVal *ikey, DalDmlIntf *dmi,
                                       const char *ctrlr_id) {
  UPLL_FUNC_TRACE;
  UPLL_LOG_DEBUG("Implementation Not Supported for this KT.");
  return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_KT;
}

upll_rc_t VbrFlowFilterMoMgr::MergeValidate(unc_key_type_t keytype,
                   const char *ctrlr_id, ConfigKeyVal *ikey,
                   DalDmlIntf *dmi, upll_import_type import_type) {
  UPLL_FUNC_TRACE;
  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrFlowFilterMoMgr::GetRenamedUncKey(ConfigKeyVal *ikey,
                                               upll_keytype_datatype_t dt_type,
                                               DalDmlIntf *dmi,
                                               uint8_t *ctrlr_id) {
  UPLL_FUNC_TRACE;
  ConfigKeyVal *unc_key = NULL;
  DbSubOp dbop = { kOpReadSingle, kOpMatchCtrlr, kOpInOutNone };
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  UPLL_LOG_TRACE("%s GetRenamedUncKey vbrff start",
                  ikey->ToStrAll().c_str());
  if ((NULL == ikey) || (ctrlr_id == NULL) || (NULL == dmi)) {
    UPLL_LOG_DEBUG("ikey/ctrlr_id dmi NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  uint8_t rename = 0;
  MoMgrImpl *mgrvbr = reinterpret_cast<MoMgrImpl *>
            (const_cast<MoManager *> (GetMoManager(UNC_KT_VBRIDGE)));
  if (!mgrvbr) {
    UPLL_LOG_DEBUG("mgrvbr failed");
    return UPLL_RC_ERR_GENERIC;
  }
  val_rename_vnode *rename_val = reinterpret_cast<val_rename_vnode*>
      (ConfigKeyVal::Malloc(sizeof(val_rename_vnode)));
  if (!rename_val) {
    UPLL_LOG_TRACE("rename_val NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  key_vbr_flowfilter_t *ctrlr_key = NULL;
  ctrlr_key = reinterpret_cast<key_vbr_flowfilter_t *> (ikey->get_key());
  if (!ctrlr_key) {
    UPLL_LOG_TRACE("ctrlr_key NULL");
    free(rename_val);
    return UPLL_RC_ERR_GENERIC;
  }
  uuu::upll_strncpy(rename_val->ctrlr_vtn_name,
                    ctrlr_key->vbr_key.vtn_key.vtn_name,
                    (kMaxLenVtnName + 1));
  rename_val->valid[UPLL_CTRLR_VTN_NAME_VALID] = UNC_VF_VALID;

  uuu::upll_strncpy(rename_val->ctrlr_vnode_name,
                    ctrlr_key->vbr_key.vbridge_name,
                    (kMaxLenVnodeName + 1));
  rename_val->valid[UPLL_CTRLR_VNODE_NAME_VALID] = UNC_VF_VALID;

  result_code = mgrvbr->GetChildConfigKey(unc_key, NULL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetRenamedUnckey:GetChildConfigKey returned error");
    free(rename_val);
    mgrvbr = NULL;
    return result_code;
  }
  if (!unc_key) {
    UPLL_LOG_DEBUG("unc_key NULL");
    free(rename_val);
    mgrvbr = NULL;
    return UPLL_RC_ERR_GENERIC;
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
    key_vbr_flowfilter_t *vbr_flowfilter_key =
        reinterpret_cast<key_vbr_flowfilter_t *> (unc_key->get_key());
    if (strcmp(reinterpret_cast<char *>(ctrlr_key->vbr_key.vtn_key.vtn_name),
               reinterpret_cast<const char *>(vbr_flowfilter_key->
               vbr_key.vtn_key.vtn_name))) {
      uuu::upll_strncpy(ctrlr_key->vbr_key.vtn_key.vtn_name,
                      vbr_flowfilter_key->vbr_key.vtn_key.vtn_name,
                      (kMaxLenVtnName + 1));
      rename |= VTN_RENAME_FLAG;
    }

    if (strcmp(reinterpret_cast<char *>(ctrlr_key->vbr_key.vbridge_name),
               reinterpret_cast<const char *>
               (vbr_flowfilter_key->vbr_key.vbridge_name))) {
      uuu::upll_strncpy(ctrlr_key->vbr_key.vbridge_name,
                      vbr_flowfilter_key->vbr_key.vbridge_name,
                      (kMaxLenVnodeName + 1));
      rename |= VBR_RENAME_FLAG;
    }
    SET_USER_DATA(ikey, unc_key);
    SET_USER_DATA_FLAGS(ikey, rename);
  }

  UPLL_LOG_TRACE("%s GetRenamedUncKey vbrff end",
                  ikey->ToStrAll().c_str());
  DELETE_IF_NOT_NULL(unc_key);
    mgrvbr = NULL;
  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrFlowFilterMoMgr::GetRenamedControllerKey(
    ConfigKeyVal *ikey, upll_keytype_datatype_t dt_type, DalDmlIntf *dmi,
    controller_domain *ctrlr_dom) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *okey = NULL;
  UPLL_LOG_TRACE("Start Input ConfigKeyVal %s", ikey->ToStrAll().c_str());

  MoMgrImpl *mgrvbr = reinterpret_cast<MoMgrImpl *>
    (const_cast<MoManager *> (GetMoManager(UNC_KT_VBRIDGE)));
  if (NULL == mgrvbr) {
    UPLL_LOG_DEBUG("Unable to get kt_vbridge instance");
    return UPLL_RC_ERR_GENERIC;
  }

  result_code = mgrvbr->GetChildConfigKey(okey, NULL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetChildConfigKey fail");
    return result_code;
  }

  if (ctrlr_dom) {
    SET_USER_DATA_CTRLR_DOMAIN(okey, *ctrlr_dom);
  } else {
    UPLL_LOG_DEBUG("Controller and domain Id are null");
    DELETE_IF_NOT_NULL(okey);
    return UPLL_RC_ERR_GENERIC;
  }

  UPLL_LOG_DEBUG("ctrlr : %s; domain : %s", ctrlr_dom->ctrlr,
      ctrlr_dom->domain);

  uuu::upll_strncpy(
      reinterpret_cast<key_vbr *> (okey->get_key())->vtn_key.vtn_name,
      reinterpret_cast<key_vbr_flowfilter_t *>
      (ikey->get_key())->vbr_key.vtn_key.vtn_name,
      (kMaxLenVtnName + 1));
  UPLL_LOG_DEBUG("vtn name (%s) (%s)",
      reinterpret_cast<key_vbr *> (okey->get_key())->vtn_key.vtn_name,
      reinterpret_cast<key_vbr_flowfilter_t *>
      (ikey->get_key())->vbr_key.vtn_key.vtn_name);
  uuu::upll_strncpy(reinterpret_cast<key_vbr *> (okey->get_key())->vbridge_name,
      reinterpret_cast<key_vbr_flowfilter_t *>
      (ikey->get_key())->vbr_key.vbridge_name,
      (kMaxLenVnodeName + 1));
  UPLL_LOG_DEBUG("vbr name (%s) (%s)",
      reinterpret_cast<key_vbr *> (okey->get_key())->vbridge_name,
      reinterpret_cast<key_vbr_flowfilter_t *>
      (ikey->get_key())->vbr_key.vbridge_name);

  DbSubOp dbop = { kOpReadSingle, kOpMatchCtrlr | kOpMatchDomain,
    kOpInOutFlag };
  result_code =  mgrvbr->ReadConfigDB(okey, dt_type, UNC_OP_READ,
      dbop, dmi, RENAMETBL);
  if ( result_code != UPLL_RC_SUCCESS ) {
    if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
      UPLL_LOG_DEBUG("ReadConfigDB no instance");
      DELETE_IF_NOT_NULL(okey);
      return UPLL_RC_SUCCESS;
    }
    UPLL_LOG_DEBUG("ReadConfigDB fail");
    DELETE_IF_NOT_NULL(okey);
    return result_code;
  }

  val_rename_vnode *rename_val = NULL;
  rename_val = reinterpret_cast<val_rename_vnode *> (GetVal(okey));
  if (!rename_val) {
    UPLL_LOG_DEBUG("Vbr Name is not Valid.");
    DELETE_IF_NOT_NULL(okey);
    return UPLL_RC_ERR_GENERIC;
  }
  UPLL_LOG_DEBUG("vtn name renamed");
  uuu::upll_strncpy(reinterpret_cast<key_vbr_flowfilter_t *>
      (ikey->get_key())->vbr_key.vtn_key.vtn_name,
      rename_val->ctrlr_vtn_name,
      (kMaxLenVtnName + 1));
  UPLL_LOG_DEBUG("vtn re name(%s) (%s)",
      reinterpret_cast<key_vbr_flowfilter_t *>
      (ikey->get_key())->vbr_key.vtn_key.vtn_name,
      rename_val->ctrlr_vtn_name);


  UPLL_LOG_DEBUG("vbr name renamed");
  uuu::upll_strncpy(reinterpret_cast<key_vbr_flowfilter_t *>
      (ikey->get_key())->vbr_key.vbridge_name,
      rename_val->ctrlr_vnode_name,
      (kMaxLenVnodeName + 1));
  UPLL_LOG_DEBUG("vbr name (%s) (%s)",
      reinterpret_cast<key_vbr_flowfilter_t *>
      (ikey->get_key())->vbr_key.vbridge_name,
      rename_val->ctrlr_vnode_name);

  DELETE_IF_NOT_NULL(okey);
  UPLL_LOG_TRACE("End Input ConfigKeyVal %s", ikey->ToStrAll().c_str());
  UPLL_LOG_DEBUG("Renamed Controller key is sucessfull.");
  return UPLL_RC_SUCCESS;
}
upll_rc_t VbrFlowFilterMoMgr::UpdateAuditConfigStatus(
    unc_keytype_configstatus_t cs_status,
    uuc::UpdateCtrlrPhase phase,
    ConfigKeyVal *&ckv_running,
    DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  val_flowfilter_t *val = NULL;

  val = (ckv_running != NULL)?
    reinterpret_cast<val_flowfilter_t *> (GetVal(ckv_running)):NULL;

  if (NULL == val) {
    UPLL_LOG_DEBUG("Memory Not Allocated");
    return UPLL_RC_ERR_GENERIC;
  }
  if (uuc::kUpllUcpCreate == phase )
    val->cs_row_status = cs_status;
  if ((uuc::kUpllUcpUpdate == phase) &&
           (val->cs_row_status == UNC_CS_INVALID ||
            val->cs_row_status == UNC_CS_NOT_APPLIED))
    val->cs_row_status = cs_status;

  UPLL_LOG_DEBUG("AuditUpdate Config Status Information %d", result_code);
  return result_code;
}

upll_rc_t VbrFlowFilterMoMgr::ReadMo(IpcReqRespHeader *req, ConfigKeyVal *ikey,
                                     DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  ConfigKeyVal *l_key =NULL, *dup_key= NULL;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  controller_domain ctrlr_dom;
  DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutCtrlr|kOpInOutDomain };
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
          DELETE_IF_NOT_NULL(dup_key);
          return result_code;
        }
        result_code =  DupConfigKeyVal(l_key, ikey, MAINTBL);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("DupConfigKeyVal fail in ReadMo for l_key");
          DELETE_IF_NOT_NULL(dup_key);
          return result_code;
        }
        GET_USER_DATA_CTRLR_DOMAIN(dup_key, ctrlr_dom);
        SET_USER_DATA_CTRLR_DOMAIN(l_key, ctrlr_dom);
        // Added CApaCheck
        result_code = ValidateCapability(req, ikey,
                   reinterpret_cast<const char *>(ctrlr_dom.ctrlr));
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("validate Capability Failed %d", result_code);
          DELETE_IF_NOT_NULL(dup_key);
          DELETE_IF_NOT_NULL(l_key);
          return result_code;
        }

        // 1.Getting renamed name if renamed
        result_code = GetRenamedControllerKey(l_key, req->datatype,
                                              dmi, &ctrlr_dom);
        if (result_code != UPLL_RC_SUCCESS) {
          DELETE_IF_NOT_NULL(dup_key);
          DELETE_IF_NOT_NULL(l_key);
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
          DELETE_IF_NOT_NULL(dup_key);
          DELETE_IF_NOT_NULL(ipc_resp.ckv_data);
          return ipc_resp.header.result_code;
        }

        if (ipc_resp.header.result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("Driver response for Key %d controller %s result %d",
                        l_key->get_key_type(), ctrlr_dom.ctrlr,
                        ipc_resp.header.result_code);
          DELETE_IF_NOT_NULL(l_key);
          DELETE_IF_NOT_NULL(dup_key);
          DELETE_IF_NOT_NULL(ipc_resp.ckv_data);
          return ipc_resp.header.result_code;
        }
        ConfigKeyVal *okey = NULL;
        result_code = ConstructReadDetailResponse(dup_key,
                                                  ipc_resp.ckv_data,
                                                  ctrlr_dom,
                                                  &okey, dmi);
        DELETE_IF_NOT_NULL(dup_key);
        DELETE_IF_NOT_NULL(l_key);
        DELETE_IF_NOT_NULL(ipc_resp.ckv_data);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("ConstructReadDetailResponse error code (%d)",
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
      UPLL_LOG_DEBUG("Operation Not Allowed");
      result_code = UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT;
  }
  return result_code;
}

upll_rc_t VbrFlowFilterMoMgr::UpdateConfigStatus(ConfigKeyVal *key,
                                                 unc_keytype_operation_t op,
                                                 uint32_t driver_result,
                                                 ConfigKeyVal *upd_key,
                                                 DalDmlIntf *dmi,
                                                 ConfigKeyVal *ctrlr_key) {
  UPLL_FUNC_TRACE;
  val_flowfilter_t *vbrflowfilter_val = NULL;
  unc_keytype_configstatus_t cs_status =
      (driver_result == 0) ? UNC_CS_APPLIED : UNC_CS_NOT_APPLIED;
  vbrflowfilter_val = reinterpret_cast<val_flowfilter_t *> (GetVal(key));
  if (vbrflowfilter_val == NULL) return UPLL_RC_ERR_GENERIC;
  if (op == UNC_OP_CREATE) {
    if (vbrflowfilter_val->cs_row_status != UNC_CS_NOT_SUPPORTED)
      vbrflowfilter_val->cs_row_status = cs_status;
  } else {
      UPLL_LOG_DEBUG("Operation Not Supported.");
      return UPLL_RC_ERR_GENERIC;
    }

  UPLL_LOG_DEBUG("Update Config Status Successfull.");
  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrFlowFilterMoMgr::CopyToConfigKey(ConfigKeyVal *&okey,
                                              ConfigKeyVal *ikey) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  if (!ikey || !(ikey->get_key())) {
    UPLL_LOG_DEBUG("Input Key Not Valid");
    return UPLL_RC_ERR_GENERIC;
  }


  key_rename_vnode_info *key_rename = NULL;
  key_rename = reinterpret_cast<key_rename_vnode_info *> (ikey->get_key());
  key_vbr_flowfilter_t * key_vbr = reinterpret_cast<key_vbr_flowfilter_t*>
                   (ConfigKeyVal::Malloc(sizeof(key_vbr_flowfilter_t)));

  if (!strlen(reinterpret_cast<char *> (key_rename->old_unc_vtn_name))) {
    free(key_vbr);
    UPLL_LOG_DEBUG("String Length not Valid to Perform the Operation");
     return UPLL_RC_ERR_GENERIC;
  }

  uuu::upll_strncpy(key_vbr->vbr_key.vtn_key.vtn_name,
                    key_rename->old_unc_vtn_name,
                    (kMaxLenVtnName + 1));

  if (ikey->get_key_type() == UNC_KT_VBRIDGE) {
    if (!strlen(reinterpret_cast<char *> (key_rename->old_unc_vnode_name))) {
      free(key_vbr);
      return UPLL_RC_ERR_GENERIC;
    }
  uuu::upll_strncpy(key_vbr->vbr_key.vbridge_name,
                    key_rename->old_unc_vnode_name,
                    (kMaxLenVnodeName + 1));
  } else {
    if (!strlen(reinterpret_cast<char *>(key_rename->new_unc_vnode_name))) {
      UPLL_LOG_DEBUG("new_unc_vnode_name NULL");
      free(key_vbr);
      return UPLL_RC_ERR_GENERIC;
    }
    uuu::upll_strncpy(key_vbr->vbr_key.vbridge_name,
                      key_rename->new_unc_vnode_name, (kMaxLenVnodeName + 1));
  }

  okey = new ConfigKeyVal(UNC_KT_VBR_FLOWFILTER,
                          IpctSt::kIpcStKeyVbrFlowfilter,
                          key_vbr, NULL);
  if (!okey) {
    free(key_vbr);
    return UPLL_RC_ERR_GENERIC;
  }

  // free(key_vbr);
  return result_code;
}

upll_rc_t VbrFlowFilterMoMgr::ReadSiblingMo(IpcReqRespHeader *req,
                                            ConfigKeyVal *ikey,
                                            bool begin,
                                            DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  ConfigKeyVal *l_key = NULL, *tmp_key = NULL;
  ConfigKeyVal *okey = NULL, *tctrl_key = NULL;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  controller_domain ctrlr_dom;
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
          return result_code;
        }
        // Retrieving state information
      } else if (req->datatype == UPLL_DT_STATE &&
                 (req->option1 == UNC_OPT1_DETAIL ||
                  req->option2 != UNC_OPT2_NONE)) {
         result_code =  DupConfigKeyVal(tctrl_key, ikey, MAINTBL);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG(" DupConfigKeyVal failed for l_key%d ", result_code);
          return result_code;
        }

        result_code = ReadInfoFromDB(req, tctrl_key, dmi, &ctrlr_dom);
        if (UPLL_RC_SUCCESS != result_code) {
          UPLL_LOG_DEBUG("ReadConfigDb failed for tctrl_key%d ", result_code);
          return result_code;
        }

        result_code =  DupConfigKeyVal(l_key, ikey, MAINTBL);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("DupConfigKeyVal fail in ReadSiblingMo for l_key");
          DELETE_IF_NOT_NULL(tctrl_key);
          return result_code;
        }
        GET_USER_DATA_CTRLR_DOMAIN(tctrl_key, ctrlr_dom);
        SET_USER_DATA_CTRLR_DOMAIN(l_key, ctrlr_dom);
        // Added CApaCheck
        result_code = ValidateCapability(req, ikey,
                   reinterpret_cast<const char *>(ctrlr_dom.ctrlr));
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("validate Capability Failed %d", result_code);
          DELETE_IF_NOT_NULL(l_key);
          DELETE_IF_NOT_NULL(tctrl_key);
          return result_code;
        }

        // 1.Getting renamed name if renamed
        result_code = GetRenamedControllerKey(l_key, req->datatype,
                                              dmi, &ctrlr_dom);
        if (result_code != UPLL_RC_SUCCESS) {
          DELETE_IF_NOT_NULL(l_key);
          DELETE_IF_NOT_NULL(tctrl_key);
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
          reinterpret_cast <key_vbr_flowfilter_t*>
              (l_key->get_key())->direction =
              reinterpret_cast<key_vbr_flowfilter_t*>
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
            DELETE_IF_NOT_NULL(tctrl_key);
            return ipc_resp.header.result_code;
          }

          if (ipc_resp.header.result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("Driver response for Key %d controller %s result %d",
                          l_key->get_key_type(), ctrlr_dom.ctrlr,
                          ipc_resp.header.result_code);

            DELETE_IF_NOT_NULL(l_key);
            DELETE_IF_NOT_NULL(ipc_resp.ckv_data);
            DELETE_IF_NOT_NULL(tctrl_key);
            return ipc_resp.header.result_code;
          }
          result_code = ConstructReadDetailResponse(tmp_key,
                                                    ipc_resp.ckv_data,
                                                    ctrlr_dom,
                                                    &okey, dmi);

          DELETE_IF_NOT_NULL(ipc_resp.ckv_data);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("ConstructReadDetailResponse error code (%d)",
                           result_code);
            DELETE_IF_NOT_NULL(l_key);
            DELETE_IF_NOT_NULL(tctrl_key);
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

upll_rc_t VbrFlowFilterMoMgr::GetValid(void *val, uint64_t indx,
    uint8_t *&valid, upll_keytype_datatype_t dt_type, MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  if (val == NULL) {
    UPLL_LOG_DEBUG("Memory is not Allocated");
    return UPLL_RC_ERR_GENERIC;
  }

  if (tbl == MAINTBL) {
    valid = NULL;
  }

  UPLL_LOG_DEBUG("GetValidAttributte is Succesfull");
  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrFlowFilterMoMgr::ConstructReadDetailResponse(
    ConfigKeyVal *ikey,
    ConfigKeyVal *drv_resp_ckv,
     controller_domain ctrlr_dom,
    ConfigKeyVal **okey,
    DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  ConfigKeyVal *tmp_okey = NULL;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigVal *drv_resp_val = NULL;
  drv_resp_val =  drv_resp_ckv->get_cfg_val();
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
    free(val_ff);
    delete (tmp_okey);
    return UPLL_RC_ERR_GENERIC;
  }
  memcpy(val_ff, tmp_val_ff, sizeof(val_flowfilter_t));
  tmp_okey->AppendCfgVal(IpctSt::kIpcStValFlowfilter, val_ff);
  while (drv_resp_val != NULL) {
    val_flowfilter_entry_t *val_entry = NULL;
    if (IpctSt::kIpcStValFlowfilterEntry ==
        drv_resp_val->get_st_num()) {
      UPLL_LOG_TRACE("Get the val struct");
      val_entry = reinterpret_cast< val_flowfilter_entry_t *>
          (drv_resp_val->get_val());
       // SetRedirectNodeAndPortinRead will Set the
       // redirect-direction in val_entry
       result_code =  SetRedirectNodeAndPortForRead(ikey,
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
  }

  tmp_okey->AppendCfgVal(drv_resp_ckv->GetCfgValAndUnlink());

  if (*okey == NULL) {
    *okey = tmp_okey;
  } else {
    (*okey)->AppendCfgKeyVal(tmp_okey);
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrFlowFilterMoMgr::GetParentConfigKey(ConfigKeyVal *&okey,
                                                 ConfigKeyVal *ikey) {
  UPLL_FUNC_TRACE;

  if (!ikey) {
    UPLL_LOG_DEBUG(" Input Key is NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  DELETE_IF_NOT_NULL(okey);
  unc_key_type_t ikey_type = ikey->get_key_type();
  if (ikey_type != UNC_KT_VBR_FLOWFILTER) {
    UPLL_LOG_DEBUG(" Invalid key type received. Key type - %d", ikey_type);
    return UPLL_RC_ERR_GENERIC;
  }

  key_vbr_flowfilter_t *pkey =
      reinterpret_cast<key_vbr_flowfilter_t*>(ikey->get_key());
  if (!pkey) {
    UPLL_LOG_DEBUG(" Input vbr flow filter key is NULL ");
    return UPLL_RC_ERR_GENERIC;
  }

  key_vbr_t *vbr_key = reinterpret_cast<key_vbr_t*>
      (ConfigKeyVal::Malloc(sizeof(key_vbr_t)));

  uuu::upll_strncpy(vbr_key->vtn_key.vtn_name,
                    reinterpret_cast<key_vbr_flowfilter_t *>
                    (pkey)->vbr_key.vtn_key.vtn_name,
                    kMaxLenVtnName + 1);
  uuu::upll_strncpy(vbr_key->vbridge_name,
                    reinterpret_cast<key_vbr_flowfilter_t *>
                    (pkey)->vbr_key.vbridge_name,
                    (kMaxLenVnodeName + 1));
  okey = new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVbr, vbr_key, NULL);
  SET_USER_DATA(okey, ikey);
  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrFlowFilterMoMgr::DeleteChildrenPOM(
           ConfigKeyVal *ikey, upll_keytype_datatype_t dt_type,
           DalDmlIntf *dmi, TcConfigMode config_mode,
           string vtn_name) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_ERR_GENERIC;

  if (NULL == ikey || NULL == dmi) {
  UPLL_LOG_DEBUG("Delete Operation failed:Bad request");
  return result_code;
  }
  // Read the DB get the flowlist value and send the delete request to
  // flowlist momgr if flowlist is configured.

  ConfigKeyVal *tempckv = NULL;
  result_code = GetChildConfigKey(tempckv, ikey);
  if (UPLL_RC_SUCCESS != result_code) {
    return result_code;
  }
  result_code = UpdateConfigDB(tempckv, dt_type, UNC_OP_DELETE, dmi,
      config_mode, vtn_name, MAINTBL);
  if (UPLL_RC_SUCCESS != result_code) {
    DELETE_IF_NOT_NULL(tempckv);
    if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
      UPLL_LOG_DEBUG("UPLL_RC_ERR_NO_SUCH_INSTANCE");
      return UPLL_RC_SUCCESS;
    }
    UPLL_LOG_DEBUG("DeleteMo record Err in vtnpolicingmaptbl (%d)",
        result_code);
    return result_code;
  }
  delete tempckv;
  tempckv = NULL;
  return  UPLL_RC_SUCCESS;
}

upll_rc_t VbrFlowFilterMoMgr::SetValidAudit(ConfigKeyVal *&ikey) {
  UPLL_FUNC_TRACE;
  val_flowfilter_t *val = reinterpret_cast<val_flowfilter_t *>
    (ConfigKeyVal::Malloc(sizeof(val_flowfilter_t)));
  val->cs_row_status = UNC_CS_APPLIED;
  ikey->AppendCfgVal(IpctSt::kIpcStValFlowfilter, val);
  return UPLL_RC_SUCCESS;
}

bool VbrFlowFilterMoMgr::FilterAttributes(void *&val1,
                                          void *val2,
                                          bool copy_to_running,
                                          unc_keytype_operation_t op) {
  UPLL_FUNC_TRACE;
  if (op != UNC_OP_CREATE)
    return true;
  return false;
}
}  // namespace kt_momgr
}  // namespace upll
}  // namespace unc
