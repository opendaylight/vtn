/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include "pfc/log.h"
#include "vrt_if_flowfilter_momgr.hh"
#include "vrt_if_flowfilter_entry_momgr.hh"
#include "unc/upll_errno.h"
#include "upll_validation.hh"
#include "unc/upll_ipc_enum.h"
#include "uncxx/upll_log.hh"
#include "vrt_if_momgr.hh"
#include "vbr_momgr.hh"

namespace unc {
namespace upll {
namespace kt_momgr {

using unc::upll::ipc_util::IpcUtil;
#define FLOWLIST_RENAME_FLAG 0x04  // For 3rd Bit
#define VTN_RENAME_FLAG 0x01  // For first Bit
#define VRT_RENAME_FLAG 0x02  // For 2nd Bit
#define SET_FLAG_VLINK 0x40
#define SET_FLAG_NO_VLINK_PORTMAP 0x9F
//
//  Vrt_If_FlowFilter Table(Main Table)
BindInfo VrtIfFlowFilterMoMgr::vrt_if_flowfilter_bind_info[] = {
  { uudst::vrt_if_flowfilter::kDbiVtnName, CFG_KEY,
    offsetof(key_vrt_if_flowfilter_t, if_key.vrt_key.vtn_key.vtn_name),
    uud::kDalChar, (kMaxLenVtnName + 1) },
  { uudst::vrt_if_flowfilter::kDbiVrtName, CFG_KEY,
    offsetof(key_vrt_if_flowfilter_t, if_key.vrt_key.vrouter_name),
    uud::kDalChar, (kMaxLenVnodeName + 1) },
  { uudst::vrt_if_flowfilter::kDbiVrtIfName, CFG_KEY,
    offsetof(key_vrt_if_flowfilter_t, if_key.if_name),
    uud::kDalChar, (kMaxLenInterfaceName + 1) },
  { uudst::vrt_if_flowfilter::kDbiInputDirection, CFG_KEY,
    offsetof(key_vrt_if_flowfilter_t, direction),
    uud::kDalUint8, 1 },
  { uudst::vrt_if_flowfilter::kDbiCtrlrName, CK_VAL,
    offsetof(key_user_data_t, ctrlr_id),
    uud::kDalChar, (kMaxLenCtrlrId + 1) },
  { uudst::vrt_if_flowfilter::kDbiDomainId, CK_VAL,
    offsetof(key_user_data, domain_id),
    uud::kDalChar, kMaxLenDomainId + 1 },
  { uudst::vrt_if_flowfilter::kDbiFlags, CK_VAL,
    offsetof(key_user_data_t, flags),
    uud::kDalUint8, 1 },
  { uudst::vrt_if_flowfilter::kDbiCsRowStatus, CS_VAL,
    offsetof(val_flowfilter_t, cs_row_status),
    uud::kDalUint8, 1 }
};
BindInfo VrtIfFlowFilterMoMgr::vrtIfflowfiltermaintbl_bind_info[] = {
  { uudst::vrt_if_flowfilter::kDbiVtnName, CFG_MATCH_KEY,
    offsetof(key_vrt_if_flowfilter_t, if_key.vrt_key.vtn_key.vtn_name),
    uud::kDalChar, (kMaxLenVtnName + 1) },
  { uudst::vrt_if_flowfilter::kDbiVrtName, CFG_MATCH_KEY,
    offsetof(key_vrt_if_flowfilter_t, if_key.vrt_key.vrouter_name),
    uud::kDalChar, (kMaxLenVnodeName + 1) },
  { uudst::vrt_if_flowfilter::kDbiVtnName, CFG_INPUT_KEY,
    offsetof(key_rename_vnode_info_t, new_unc_vtn_name),
    uud::kDalChar, (kMaxLenVtnName + 1) },
  { uudst::vrt_if_flowfilter::kDbiVrtName, CFG_INPUT_KEY,
    offsetof(key_rename_vnode_info_t, new_unc_vnode_name),
    uud::kDalChar, (kMaxLenVnodeName + 1) },
  { uudst::vrt_if_flowfilter::kDbiFlags, CK_VAL,
    offsetof(key_user_data_t, flags),
    uud::kDalUint8, 1 }
};

unc_key_type_t VrtIfFlowFilterMoMgr::vrt_if_flowfilter_child[] = {
  UNC_KT_VRTIF_FLOWFILTER_ENTRY
    };
  VrtIfFlowFilterMoMgr::VrtIfFlowFilterMoMgr() : MoMgrImpl() {
  UPLL_FUNC_TRACE;
  // Rename and ctrlr tables not required for this KT
  // setting table index for ctrl and rename table as NULL
  ntable = (MAX_MOMGR_TBLS);
  table = new Table *[ntable]();
  table[MAINTBL] = new Table(uudst::kDbiVrtIfFlowFilterTbl,
      UNC_KT_VRTIF_FLOWFILTER, vrt_if_flowfilter_bind_info,
      IpctSt::kIpcStKeyVrtIfFlowfilter, IpctSt::kIpcStValFlowfilter,
      uudst::vrt_if_flowfilter::kDbiVrtIfFlowFilterNumCols);
  table[RENAMETBL] = NULL;
  table[CTRLRTBL] = NULL;
  table[CONVERTTBL] = NULL;


  nchild = sizeof(vrt_if_flowfilter_child) / sizeof(vrt_if_flowfilter_child[0]);
  child = vrt_if_flowfilter_child;
}

upll_rc_t VrtIfFlowFilterMoMgr::CreateCandidateMo(IpcReqRespHeader *req,
    ConfigKeyVal *ikey,
    DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;

  if (ikey == NULL || req == NULL) {
    return UPLL_RC_ERR_GENERIC;
  }
  UPLL_LOG_TRACE("InputConfigKeyVal %s", ikey->ToStrAll().c_str());
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  if (req->datatype != UPLL_DT_IMPORT) {
    // validate syntax and semantics
    result_code = ValidateMessage(req, ikey);
    if (result_code != UPLL_RC_SUCCESS) {
      pfc_log_debug("ValidateMessage failed, Error - %d", result_code);
      return result_code;
    }
  }
  result_code = ValidateAttribute(ikey, dmi, req);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("ValidateAttribute failed, Error - %d", result_code);
    return result_code;
  }

    // create a record in CANDIDATE DB
    VrtIfMoMgr *mgr =
      reinterpret_cast<VrtIfMoMgr *>(const_cast<MoManager *>(GetMoManager(
              UNC_KT_VRT_IF)));
    ConfigKeyVal *ckv = NULL;
    InterfacePortMapInfo flags = kVlinkPortMapNotConfigured;

    result_code = mgr->GetChildConfigKey(ckv, NULL);
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
    result_code = mgr->GetVexternal(ckv, req->datatype, dmi,
        vexternal, vex_if, flags);
    if (UPLL_RC_SUCCESS != result_code) {
      DELETE_IF_NOT_NULL(ckv);
      FREE_IF_NOT_NULL(vexternal);
      FREE_IF_NOT_NULL(vex_if);
      return result_code;
    }
    uint8_t flag_port_map = 0;
    GET_USER_DATA_FLAGS(ikey, flag_port_map);
    if (flags & kVlinkConfigured) {
      flag_port_map = flag_port_map|SET_FLAG_VLINK;
    }
    free(vexternal);
    free(vex_if);
    DELETE_IF_NOT_NULL(ckv);
    SET_USER_DATA_FLAGS(ikey, flag_port_map);

    controller_domain ctrlr_dom;
    memset(&ctrlr_dom, 0, sizeof(controller_domain));
    result_code = GetControllerDomainID(ikey, req->datatype, dmi);
    if (result_code != UPLL_RC_SUCCESS) {
      if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
        return UPLL_RC_ERR_PARENT_DOES_NOT_EXIST;
      }
      UPLL_LOG_DEBUG("Failed to Get the Controller Domain details, err:%d",
                     result_code);
      return result_code;
    }
    GET_USER_DATA_CTRLR_DOMAIN(ikey, ctrlr_dom);

    UPLL_LOG_DEBUG("ctrlrid %s, domainid %s",
                   ctrlr_dom.ctrlr, ctrlr_dom.domain);

  result_code = ValidateCapability(req, ikey,
                   reinterpret_cast<const char *>(ctrlr_dom.ctrlr));
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("validate Capability Failed %d", result_code);
    return result_code;
  }
  TcConfigMode config_mode;
  std::string vtn_name = "";
  result_code = GetConfigModeInfo(req, config_mode, vtn_name);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetConfigMode failed");
    return result_code;
  }

  result_code = UpdateConfigDB(ikey, req->datatype, UNC_OP_CREATE, dmi,
                               config_mode, vtn_name);
  return result_code;
}



upll_rc_t VrtIfFlowFilterMoMgr::IsReferenced(IpcReqRespHeader *req,
                                             ConfigKeyVal *ikey,
                                             DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  if (NULL == ikey) return UPLL_RC_ERR_GENERIC;
  DbSubOp dbop = { kOpReadExist, kOpMatchNone, kOpInOutNone };
  // Check the object existence
  result_code = UpdateConfigDB(ikey, req->datatype, UNC_OP_READ, dmi,
                               &dbop, MAINTBL);
  if (UPLL_RC_ERR_INSTANCE_EXISTS != result_code) {
    UPLL_LOG_DEBUG("UpdateConfigDB failed %d ", result_code);
    return result_code;
  }
  return UPLL_RC_SUCCESS;
}


upll_rc_t VrtIfFlowFilterMoMgr::ValidateAttribute(ConfigKeyVal *ikey,
                                                  DalDmlIntf *dmi,
                                                  IpcReqRespHeader *req) {
  // No operation
  return UPLL_RC_SUCCESS;
}

upll_rc_t VrtIfFlowFilterMoMgr::SetRenameFlag(ConfigKeyVal *ikey,
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
            UNC_KT_VRT_IF)));
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
  SET_USER_DATA_FLAGS(ikey, rename);
  DELETE_IF_NOT_NULL(pkey);
  return UPLL_RC_SUCCESS;
}


bool VrtIfFlowFilterMoMgr::IsValidKey(void *key,
                                      uint64_t index, MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  upll_rc_t ret_val = UPLL_RC_SUCCESS;
  key_vrt_if_flowfilter_t  *ff_key =
      reinterpret_cast<key_vrt_if_flowfilter_t *>(key);
  if (ff_key == NULL) {
    UPLL_LOG_DEBUG("Invalid key");
    return false;
  }

  switch (index) {
    case uudst::vrt_if_flowfilter::kDbiVtnName:
      ret_val = ValidateKey(
          reinterpret_cast<char *>
          (ff_key->if_key.vrt_key.vtn_key.vtn_name),
          kMinLenVtnName, kMaxLenVtnName);
      if (ret_val != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("VTN Name is not valid(%d)", ret_val);
        return false;
      }
      break;
    case uudst::vrt_if_flowfilter::kDbiVrtName:
      ret_val = ValidateKey(reinterpret_cast<char *>
                            (ff_key->if_key.vrt_key.vrouter_name),
                            kMinLenVnodeName, kMaxLenVnodeName);
      if (ret_val != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("VRT Name is not valid(%d)", ret_val);
        return false;
      }
      break;
    case uudst::vrt_if_flowfilter::kDbiVrtIfName:
      ret_val = ValidateKey(reinterpret_cast<char *>
                            (ff_key->if_key.if_name),
                            kMinLenInterfaceName, kMaxLenInterfaceName);
      if (ret_val != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("VRT interface name is not valid(%d)", ret_val);
        return false;
      }
      break;
    case uudst::vrt_if_flowfilter::kDbiInputDirection:
      if (ff_key->direction == 0xFE) {
        // if operation is read sibling begin or
        // read sibling count return false
        // for output binding
        ff_key->direction = 0;
        return false;
      } else {
        if (!ValidateNumericRange(ff_key->direction,
                                  (uint8_t) UPLL_FLOWFILTER_DIR_IN,
                                  (uint8_t) UPLL_FLOWFILTER_DIR_OUT,
                                  true, true)) {
          UPLL_LOG_DEBUG("direction syntax validation failed :");
          return false;  // UPLL_RC_ERR_CFG_SYNTAX;
        }
      }
      break;
    default:
      UPLL_LOG_DEBUG("Invalid Key Index");
      return false;
  }
  return true;
}

upll_rc_t VrtIfFlowFilterMoMgr::GetRenamedControllerKey(
    ConfigKeyVal *ikey, upll_keytype_datatype_t dt_type, DalDmlIntf *dmi,
    controller_domain *ctrlr_dom) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *okey = NULL;

  MoMgrImpl *VrtMoMgr =  reinterpret_cast<MoMgrImpl*>(const_cast<MoManager*>
      (GetMoManager(UNC_KT_VROUTER)));
  if (VrtMoMgr == NULL) {
    UPLL_LOG_DEBUG("obj null");
    return UPLL_RC_ERR_GENERIC;
  }

  result_code = VrtMoMgr->GetChildConfigKey(okey, NULL);
  if ( result_code != UPLL_RC_SUCCESS ) {
    UPLL_LOG_DEBUG("GetChildConfigKey fail");
    return UPLL_RC_ERR_GENERIC;
  }
  if (ctrlr_dom) {
    SET_USER_DATA_CTRLR_DOMAIN(okey, *ctrlr_dom);
  } else {
    UPLL_LOG_DEBUG("ctrlr null");
    DELETE_IF_NOT_NULL(okey);
    return UPLL_RC_ERR_GENERIC;
  }

  UPLL_LOG_DEBUG("ctrlr : %s; domain : %s", ctrlr_dom->ctrlr,
      ctrlr_dom->domain);

  strncpy(reinterpret_cast<char *>
      (reinterpret_cast<key_vrt *>(okey->get_key())->vtn_key.vtn_name),
      reinterpret_cast<const char *>
      (reinterpret_cast<key_vrt_if_flowfilter_t *>
       (ikey->get_key())->if_key.vrt_key.vtn_key.vtn_name),
      kMaxLenVtnName + 1);
  UPLL_LOG_DEBUG("vrt name (%s) (%s)",
      reinterpret_cast<char *>
      (reinterpret_cast<key_vrt *>(okey->get_key())->vtn_key.vtn_name),
      reinterpret_cast<const char *>
      (reinterpret_cast<key_vrt_if_flowfilter_t *>
       (ikey->get_key())->if_key.vrt_key.vtn_key.vtn_name));

  strncpy(reinterpret_cast<char *>
      (reinterpret_cast<key_vrt *>(okey->get_key())->vrouter_name),
      reinterpret_cast<const char *>
      (reinterpret_cast<key_vrt_if_flowfilter_t *>
       (ikey->get_key())->if_key.vrt_key.vrouter_name),
      kMaxLenVnodeName + 1);
  UPLL_LOG_DEBUG("vrt name (%s) (%s)",
      reinterpret_cast<char *>
      (reinterpret_cast<key_vrt *>(okey->get_key())->vrouter_name),
      reinterpret_cast<const char *>
      (reinterpret_cast<key_vrt_if_flowfilter_t *>
       (ikey->get_key())->if_key.vrt_key.vrouter_name));
  DbSubOp dbop = { kOpReadSingle, kOpMatchCtrlr | kOpMatchDomain,
    kOpInOutFlag };
  /* ctrlr_name */
  result_code =  VrtMoMgr->ReadConfigDB(okey, dt_type, UNC_OP_READ,
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
  // NULL Checks Missing
  val_rename_vnode *rename_val = reinterpret_cast <val_rename_vnode *>
    ((GetVal(okey)));
  if (!rename_val) {
    UPLL_LOG_DEBUG("vrt Name is not Valid");
    DELETE_IF_NOT_NULL(okey);
    return UPLL_RC_ERR_GENERIC;
  }

  uuu::upll_strncpy(
      reinterpret_cast<key_vrt_if_flowfilter_t*>
      (ikey->get_key())->if_key.vrt_key.vtn_key.vtn_name,
      rename_val->ctrlr_vtn_name,
      (kMaxLenVtnName + 1));

  uuu::upll_strncpy(
      reinterpret_cast<key_vrt_if_flowfilter_t*>
      (ikey->get_key())->if_key.vrt_key.vrouter_name,
      rename_val->ctrlr_vnode_name,
      (kMaxLenVnodeName + 1));

  DELETE_IF_NOT_NULL(okey);
  UPLL_LOG_TRACE("End ... GetRenamedCtrl InputConfigKeyVal %s",
      ikey->ToStrAll().c_str());
  UPLL_LOG_DEBUG("GetRenamedControllerKey::Success");
  return UPLL_RC_SUCCESS;
}

upll_rc_t VrtIfFlowFilterMoMgr::GetRenamedUncKey(
    ConfigKeyVal *ikey, upll_keytype_datatype_t dt_type, DalDmlIntf *dmi,
    uint8_t *ctrlr_id) {
  UPLL_FUNC_TRACE;
  ConfigKeyVal *unc_key = NULL;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  DbSubOp dbop = { kOpReadSingle, kOpMatchCtrlr, kOpInOutNone };
  UPLL_LOG_TRACE("%s GetRenamedUncKey vrtifff start",
                  ikey->ToStrAll().c_str());
  if ((NULL == ikey) || (ctrlr_id == NULL) || (NULL == dmi)) {
    UPLL_LOG_DEBUG("ikey/ctrlr_id dmi NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  uint8_t rename = 0;
  MoMgrImpl *VrtMoMgr = reinterpret_cast <MoMgrImpl *>(const_cast<MoManager*>
                                  (GetMoManager(UNC_KT_VROUTER)));
  if (VrtMoMgr == NULL) {
    UPLL_LOG_DEBUG("VrtMoMgr NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  val_rename_vnode *rename_val = reinterpret_cast<val_rename_vnode*>
      (ConfigKeyVal::Malloc(sizeof(val_rename_vnode)));
  if (!rename_val) {
    UPLL_LOG_DEBUG("rename_val NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  key_vrt_if_flowfilter_t *ctrlr_key =
          reinterpret_cast <key_vrt_if_flowfilter_t *>
                           (ikey->get_key());
  if (!ctrlr_key) {
    UPLL_LOG_DEBUG("rename_val NULL");
    free(rename_val);
    return UPLL_RC_ERR_GENERIC;
  }
  uuu::upll_strncpy(rename_val->ctrlr_vtn_name,
          ctrlr_key->if_key.vrt_key.vtn_key.vtn_name,
         (kMaxLenVtnName + 1));
  rename_val->valid[UPLL_CTRLR_VTN_NAME_VALID] = UNC_VF_VALID;

  uuu::upll_strncpy(rename_val->ctrlr_vnode_name,
          ctrlr_key->if_key.vrt_key.vrouter_name,
          (kMaxLenVnodeName + 1));
  rename_val->valid[UPLL_CTRLR_VNODE_NAME_VALID] = UNC_VF_VALID;

  result_code =  VrtMoMgr->GetChildConfigKey(unc_key, NULL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Failed To Get Child Config Key structure");
    free(rename_val);
    VrtMoMgr = NULL;
    return result_code;
  }
  if (!unc_key) {
    UPLL_LOG_DEBUG("unc_key NULL");
    free(rename_val);
    VrtMoMgr = NULL;
    return UPLL_RC_ERR_GENERIC;
  }
  SET_USER_DATA_CTRLR(unc_key, ctrlr_id);
  unc_key->AppendCfgVal(IpctSt::kIpcStValRenameVtn, rename_val);
  result_code = VrtMoMgr->ReadConfigDB(unc_key, dt_type,
                                                   UNC_OP_READ, dbop, dmi,
                                                   RENAMETBL);
  if (result_code == UPLL_RC_SUCCESS) {
    key_vrt_if_flowfilter_t *key_vrt_if_flowfilter =
       reinterpret_cast <key_vrt_if_flowfilter_t *> (unc_key->get_key());
    if (strcmp(reinterpret_cast<char *>(ctrlr_key->
               if_key.vrt_key.vtn_key.vtn_name),
               reinterpret_cast<const char *>(key_vrt_if_flowfilter->
               if_key.vrt_key.vtn_key.vtn_name))) {
      uuu::upll_strncpy(
         ctrlr_key->if_key.vrt_key.vtn_key.vtn_name,
         key_vrt_if_flowfilter->if_key.vrt_key.vtn_key.vtn_name,
         (kMaxLenVtnName + 1));
      rename |= VTN_RENAME_FLAG;
    }
    if (strcmp(
            reinterpret_cast<char *>(ctrlr_key->if_key.vrt_key.vrouter_name),
            reinterpret_cast<const char *>(key_vrt_if_flowfilter->
                                           if_key.vrt_key.vrouter_name))) {
      uuu::upll_strncpy(
          ctrlr_key->if_key.vrt_key.vrouter_name,
          key_vrt_if_flowfilter->if_key.vrt_key.vrouter_name,
          (kMaxLenVnodeName + 1));
      rename |= VRT_RENAME_FLAG;
    }
    SET_USER_DATA(ikey, unc_key);
    SET_USER_DATA_FLAGS(ikey, rename);
  }

  UPLL_LOG_TRACE("%s GetRenamedUncKey vbrifff end",
                  ikey->ToStrAll().c_str());
  DELETE_IF_NOT_NULL(unc_key);
  VrtMoMgr = NULL;
  if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code)
    result_code = UPLL_RC_SUCCESS;
  return result_code;
}

upll_rc_t VrtIfFlowFilterMoMgr::GetChildConfigKey(ConfigKeyVal *&okey,
                                                  ConfigKeyVal *parent_key) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  key_vrt_if_flowfilter_t *vrt_if_ff_key;
  void *pkey = NULL;

  if (parent_key == NULL) {
    vrt_if_ff_key = reinterpret_cast<key_vrt_if_flowfilter_t*>
        (ConfigKeyVal::Malloc(sizeof(key_vrt_if_flowfilter_t)));
    // If no direction is specified , 0xFE is filled to bind output direction
    vrt_if_ff_key->direction = 0xFE;
    okey = new ConfigKeyVal(UNC_KT_VRTIF_FLOWFILTER,
                            IpctSt::kIpcStKeyVrtIfFlowfilter,
                            vrt_if_ff_key, NULL);
    UPLL_LOG_DEBUG("Parent Key Filled ");
    return UPLL_RC_SUCCESS;
  } else {
    pkey = parent_key->get_key();
  }
  if (!pkey) return UPLL_RC_ERR_GENERIC;

  if (okey) {
    if (okey->get_key_type() != UNC_KT_VRTIF_FLOWFILTER)
      return UPLL_RC_ERR_GENERIC;
  }
  if ((okey) && (okey->get_key())) {
    vrt_if_ff_key = reinterpret_cast<key_vrt_if_flowfilter_t *>
        (okey->get_key());
  } else {
    vrt_if_ff_key = reinterpret_cast<key_vrt_if_flowfilter_t *>
        (ConfigKeyVal::Malloc(sizeof(key_vrt_if_flowfilter_t)));
    // If no direction is specified , 0xFE is filled to bind output direction
    vrt_if_ff_key->direction = 0xFE;
  }

  switch (parent_key->get_key_type()) {
    case UNC_KT_VTN:
      uuu::upll_strncpy(
          vrt_if_ff_key->if_key.vrt_key.vtn_key.vtn_name,
          reinterpret_cast<key_vtn_t *>
          (pkey)->vtn_name,
          kMaxLenVtnName + 1);
      break;
    case UNC_KT_VROUTER:
      uuu::upll_strncpy(
          vrt_if_ff_key->if_key.vrt_key.vtn_key.vtn_name,
          reinterpret_cast<key_vrt_t *>
          (pkey)->vtn_key.vtn_name,
          kMaxLenVtnName + 1);
      uuu::upll_strncpy(
          vrt_if_ff_key->if_key.vrt_key.vrouter_name,
          reinterpret_cast<key_vrt_t *>
          (pkey)->vrouter_name,
          kMaxLenVnodeName + 1);
      break;
    case UNC_KT_VRT_IF:
      uuu::upll_strncpy(
          vrt_if_ff_key->if_key.vrt_key.vtn_key.vtn_name,
          reinterpret_cast<key_vrt_if_t *>
          (pkey)->vrt_key.vtn_key.vtn_name,
          kMaxLenVtnName + 1);
      uuu::upll_strncpy(
          vrt_if_ff_key->if_key.vrt_key.vrouter_name,
          reinterpret_cast<key_vrt_if_t *>
          (pkey)->vrt_key.vrouter_name,
          kMaxLenVnodeName + 1);
      uuu::upll_strncpy(vrt_if_ff_key->if_key.if_name,
                        reinterpret_cast<key_vrt_if_t *>
                        (pkey)->if_name,
                        kMaxLenInterfaceName + 1);
      break;
    case UNC_KT_VRTIF_FLOWFILTER:
      uuu::upll_strncpy(
          vrt_if_ff_key->if_key.vrt_key.vtn_key.vtn_name,
          reinterpret_cast<key_vrt_if_flowfilter_t *>
          (pkey)->if_key.vrt_key.vtn_key.vtn_name,
          kMaxLenVtnName + 1);
      uuu::upll_strncpy(
          vrt_if_ff_key->if_key.vrt_key.vrouter_name,
          reinterpret_cast<key_vrt_if_flowfilter_t *>
          (pkey)->if_key.vrt_key.vrouter_name,
          kMaxLenVnodeName + 1);
      uuu::upll_strncpy(vrt_if_ff_key->if_key.if_name,
                        reinterpret_cast<key_vrt_if_flowfilter_t *>
                        (pkey)->if_key.if_name,
                        kMaxLenInterfaceName + 1);
      vrt_if_ff_key->direction = reinterpret_cast<key_vrt_if_flowfilter_t *>
          (pkey)->direction;
      break;
    default:
      if (vrt_if_ff_key) free(vrt_if_ff_key);
      return UPLL_RC_ERR_GENERIC;
  }


  if ((okey) && !(okey->get_key())) {
    UPLL_LOG_DEBUG("okey not null and flow list name updated");
    okey->SetKey(IpctSt::kIpcStKeyVrtIfFlowfilter, vrt_if_ff_key);
  }

  if (!okey) {
    okey = new ConfigKeyVal(UNC_KT_VRTIF_FLOWFILTER,
                            IpctSt::kIpcStKeyVrtIfFlowfilter,
                            vrt_if_ff_key, NULL);
  }
  SET_USER_DATA(okey, parent_key);
  UPLL_LOG_DEBUG("okey filled Succesfully %d", result_code);
  return result_code;
}

upll_rc_t VrtIfFlowFilterMoMgr::ReadMo(IpcReqRespHeader *req,
                                       ConfigKeyVal *ikey,
                                       DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *l_key = NULL, *dup_key = NULL;
  controller_domain ctrlr_dom;
  DbSubOp dbop = { kOpReadSingle, kOpMatchNone,
    kOpInOutCtrlr|kOpInOutDomain|kOpInOutFlag };

  result_code = ValidateMessage(req, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
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
          UPLL_LOG_DEBUG("DupConfigKeyVal fail in ReadMo for l_key");
          delete dup_key;
          return result_code;
        }
        GET_USER_DATA_CTRLR_DOMAIN(dup_key, ctrlr_dom);
        SET_USER_DATA_CTRLR_DOMAIN(l_key, ctrlr_dom);

        // Added CapaCheck
        result_code = ValidateCapability(req, ikey,
                   reinterpret_cast<const char *>(ctrlr_dom.ctrlr));
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("validate Capability Failed %d", result_code);
          delete dup_key;
          delete l_key;
          return result_code;
        }

       // 1.Getting renamed name if renamed
        result_code = GetRenamedControllerKey(l_key, req->datatype,
                                              dmi, &ctrlr_dom);
        if (result_code != UPLL_RC_SUCCESS) {
          delete dup_key;
          delete l_key;
          return result_code;
        }

        uint8_t vlink_flag = 0;
        GET_USER_DATA_FLAGS(dup_key, vlink_flag);
        if (!(SET_FLAG_VLINK & vlink_flag)) {
          UPLL_LOG_DEBUG("Vlink Not Configured");
          delete dup_key;
          delete l_key;
          return UPLL_RC_ERR_NOT_ALLOWED_AT_THIS_TIME;
        }

        pfcdrv_val_vbrif_vextif_t *pfc_val =
            reinterpret_cast<pfcdrv_val_vbrif_vextif_t *>
            (ConfigKeyVal::Malloc(sizeof(pfcdrv_val_vbrif_vextif_t)));

        pfc_val->valid[PFCDRV_IDX_INTERFACE_TYPE] = UNC_VF_VALID;
        pfc_val->valid[PFCDRV_IDX_VEXTERNAL_NAME_VBRIF] = UNC_VF_INVALID;
        pfc_val->valid[PFCDRV_IDX_VEXT_IF_NAME_VBRIF] = UNC_VF_INVALID;
        pfc_val->interface_type = PFCDRV_IF_TYPE_VBRIF;

        l_key->SetCfgVal(new ConfigVal(IpctSt::kIpcStPfcdrvValVbrifVextif,
                                       pfc_val));

        // 2.send request to driver
        IpcResponse ipc_resp;
        memset(&ipc_resp, 0, sizeof(IpcResponse));
        IpcRequest ipc_req;
        memset(&ipc_req, 0, sizeof(ipc_req));
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
                                                  ipc_resp.ckv_data, ctrlr_dom,
                                                  &okey, dmi);
        DELETE_IF_NOT_NULL(l_key);
        DELETE_IF_NOT_NULL(ipc_resp.ckv_data);
        DELETE_IF_NOT_NULL(dup_key);
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

upll_rc_t VrtIfFlowFilterMoMgr::UpdateConfigStatus(ConfigKeyVal *key,
                                                   unc_keytype_operation_t op,
                                                   uint32_t driver_result,
                                                   ConfigKeyVal *upd_key,
                                                   DalDmlIntf *dmi,
                                                   ConfigKeyVal *ctrlr_key) {
  UPLL_FUNC_TRACE;
  val_flowfilter_t *vrt_flowfilter_val = NULL;
  unc_keytype_configstatus_t cs_status =
      (driver_result == 0) ? UNC_CS_APPLIED : UNC_CS_NOT_APPLIED;
  vrt_flowfilter_val =  reinterpret_cast <val_flowfilter_t *> (GetVal(key));
  if (vrt_flowfilter_val == NULL) {
    UPLL_LOG_DEBUG("vrt_flowfilter_val is Null ");
    return UPLL_RC_ERR_GENERIC;
  }
  if (op == UNC_OP_CREATE) {
    if (vrt_flowfilter_val->cs_row_status != UNC_CS_NOT_SUPPORTED)
      vrt_flowfilter_val->cs_row_status = cs_status;
  } else {
    UPLL_LOG_DEBUG("UPLL_RC_ERR_GENERIC %d", UPLL_RC_ERR_GENERIC);
    return UPLL_RC_ERR_GENERIC;
  }
  // return UPLL_RC_ERR_GENERIC;
  UPLL_LOG_DEBUG("UpdateConfigStatus Success");
  return UPLL_RC_SUCCESS;
}

upll_rc_t VrtIfFlowFilterMoMgr::AllocVal(ConfigVal *&ck_val,
                                         upll_keytype_datatype_t dt_type,
                                         MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  void *val;  //   *ck_nxtval;
  if (ck_val != NULL) {
    UPLL_LOG_DEBUG("ck_val is NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  switch (tbl) {
    case MAINTBL:
      val = reinterpret_cast <void *>
          (ConfigKeyVal::Malloc(sizeof(val_flowfilter_t)));
      ck_val = new ConfigVal(IpctSt::kIpcStValFlowfilter, val);
        break;
    default:
      val = NULL;
      return UPLL_RC_ERR_GENERIC;
      break;
  }
  UPLL_LOG_DEBUG(" AllocVal SUCCESS ");
  return UPLL_RC_SUCCESS;
}

upll_rc_t VrtIfFlowFilterMoMgr::DupConfigKeyVal(ConfigKeyVal *&okey,
                                                ConfigKeyVal *&req,
                                                MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  if (req == NULL) {
    UPLL_LOG_DEBUG(" req is Null ");
    return UPLL_RC_ERR_GENERIC;
  }
  if (okey != NULL) {
    UPLL_LOG_DEBUG("okey is Not Null");
    return UPLL_RC_ERR_GENERIC;
  }
  if (req->get_key_type() != UNC_KT_VRTIF_FLOWFILTER)
    return UPLL_RC_ERR_GENERIC;

  ConfigVal *tmp1 = NULL, *tmp = (req)->get_cfg_val();

  if (tmp) {
    if (tbl == MAINTBL) {
      val_flowfilter_t *ival = reinterpret_cast <val_flowfilter_t*>
          (GetVal(req));
      if (NULL != ival) {
        val_flowfilter_t *val_flowfilter = reinterpret_cast<val_flowfilter_t *>
            (ConfigKeyVal::Malloc(sizeof(val_flowfilter_t)));
        memcpy(val_flowfilter, ival, sizeof(val_flowfilter_t));
        tmp1 = new ConfigVal(IpctSt::kIpcStValFlowfilter, val_flowfilter);
      }
      if (NULL == tmp1) {
        UPLL_LOG_DEBUG("Memory Not Allocated");
        return UPLL_RC_ERR_GENERIC;
      }
      tmp1->set_user_data(tmp->get_user_data());
    }
  }

  void *tkey = (req != NULL) ? (req)->get_key() : NULL;
  if (tkey != NULL) {
    key_vrt_if_flowfilter_t *ikey = NULL;
    ikey = reinterpret_cast<key_vrt_if_flowfilter_t *> (tkey);
    key_vrt_if_flowfilter_t *vrt_if_flowfilter =
      reinterpret_cast<key_vrt_if_flowfilter_t*>
        (ConfigKeyVal::Malloc(sizeof(key_vrt_if_flowfilter_t)));

    memcpy(vrt_if_flowfilter, ikey, sizeof(key_vrt_if_flowfilter_t));
    okey = new ConfigKeyVal(UNC_KT_VRTIF_FLOWFILTER,
                          IpctSt::kIpcStKeyVrtIfFlowfilter, vrt_if_flowfilter,
                          tmp1);
    SET_USER_DATA(okey, req);

    UPLL_LOG_DEBUG("DupConfigkeyVal Succesfull.");
    // free(val_flowfilter);
    // free(vrt_if_flowfilter);
    return UPLL_RC_SUCCESS;
  }
  UPLL_LOG_DEBUG("Error in  DupConfigKeyVal");
  DELETE_IF_NOT_NULL(tmp1);
  return UPLL_RC_ERR_GENERIC;
}

upll_rc_t VrtIfFlowFilterMoMgr::UpdateMo(IpcReqRespHeader *req,
                                         ConfigKeyVal *ikey, DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  UPLL_LOG_DEBUG("Operation not allowed for this KT");
  return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_KT;
}

upll_rc_t VrtIfFlowFilterMoMgr::RenameMo(IpcReqRespHeader *req,
                                         ConfigKeyVal *ikey, DalDmlIntf *dmi,
                                         const char *ctrlr_id) {
  UPLL_FUNC_TRACE;
  UPLL_LOG_DEBUG("RenameMo:: RenameMo Success ");
  return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_KT;
}

upll_rc_t VrtIfFlowFilterMoMgr::MergeValidate(unc_key_type_t keytype,
                                             const char *ctrlr_id,
                                             ConfigKeyVal *ikey,
                                             DalDmlIntf *dmi,
                                             upll_import_type import_type) {
  UPLL_FUNC_TRACE;
  return UPLL_RC_SUCCESS;
}
upll_rc_t VrtIfFlowFilterMoMgr::ValidateCapability(IpcReqRespHeader *req,
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

  UPLL_LOG_DEBUG("dt_type   : (%d)"
                 "operation : (%d)", req->datatype, req->operation);

  bool result_code = false;
  uint32_t max_instance_count;
  const uint8_t *attrs = NULL;
  uint32_t max_attrs = 0;

  switch (req->operation) {
    case UNC_OP_CREATE: {
      result_code = GetCreateCapability(ctrlr_name, ikey->get_key_type(),
                                        &max_instance_count,
                                        &max_attrs, &attrs);
      break;
    }
  default : {
      if (req->datatype == UPLL_DT_STATE) {
        UPLL_LOG_TRACE("Calling GetStateCapability Operation %d ",
                       req->operation);
        result_code = GetStateCapability(ctrlr_name, ikey->get_key_type(),
                                      &max_attrs, &attrs);
      } else {
        UPLL_LOG_TRACE("Calling GetReadCapability Operation %d ",
                       req->operation);
        result_code = GetReadCapability(ctrlr_name, ikey->get_key_type(),
                                      &max_attrs, &attrs);
      }
    }
  }

  if (!result_code) {
    UPLL_LOG_DEBUG("keytype(%d) is not supported by controller(%s) "
                   "for opeartion(%d)",
                   ikey->get_key_type(), ctrlr_name, req->operation);
    return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VrtIfFlowFilterMoMgr::ValidateMessage(IpcReqRespHeader* req,
                                                ConfigKeyVal *key) {
  UPLL_FUNC_TRACE;

  upll_rc_t rt_code = UPLL_RC_ERR_GENERIC;

  if ((NULL == req) || (NULL == key)) {
    UPLL_LOG_DEBUG("ConfigKeyval is NULL");
    return UPLL_RC_ERR_BAD_REQUEST;
  }

  if (UNC_KT_VRTIF_FLOWFILTER != key->get_key_type()) {
    UPLL_LOG_DEBUG(" Invalid keytype(%d)", key->get_key_type());
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

  if (req->datatype != UPLL_DT_IMPORT) {
    /** Read key structure */
    if (key->get_st_num() != IpctSt::kIpcStKeyVrtIfFlowfilter) {
      UPLL_LOG_DEBUG("Invalid key structure received. received struct num - %d",
                   key->get_st_num());
      return UPLL_RC_ERR_BAD_REQUEST;
    }
  }
  key_vrt_if_flowfilter_t *key_vrt_if_flowfilter =
      reinterpret_cast<key_vrt_if_flowfilter_t *>(key->get_key());

  /** Validate key structure */
  if (NULL == key_vrt_if_flowfilter) {
    UPLL_LOG_DEBUG("KT_VRTIF_FLOWFILTER Key structure is empty!!");
    return UPLL_RC_ERR_BAD_REQUEST;
  }

  rt_code = ValidateVrtIfFlowfilterKey(key_vrt_if_flowfilter,
                                       req->operation);

  if (UPLL_RC_SUCCESS != rt_code) {
    UPLL_LOG_DEBUG(" key_vrtif_flowfilter syntax validation failed :"
                   "Err Code - %d",
                   rt_code);
    return rt_code;
  }

  return UPLL_RC_SUCCESS;
}

upll_rc_t VrtIfFlowFilterMoMgr::ValidateVrtIfFlowfilterKey(
    key_vrt_if_flowfilter_t* key_vrt_if_flowfilter,
    unc_keytype_operation_t op) {

  UPLL_FUNC_TRACE;

  // TODO(author)

  upll_rc_t rt_code = UPLL_RC_ERR_GENERIC;

  VrtIfMoMgr *mgrvrt = reinterpret_cast<VrtIfMoMgr *>(const_cast<MoManager*>(
          GetMoManager(UNC_KT_VRT_IF)));

  if (NULL == mgrvrt) {
    UPLL_LOG_DEBUG("unable to get VtnMoMgr object to validate key_vtn");
    return UPLL_RC_ERR_GENERIC;
  }

  rt_code = mgrvrt->ValidateVrtIfKey(&(key_vrt_if_flowfilter->if_key));

  if (UPLL_RC_SUCCESS != rt_code) {
    UPLL_LOG_DEBUG(" Vrt_key syntax validation failed :Err Code - %d", rt_code);

    return rt_code;
  }

  if ((op != UNC_OP_READ_SIBLING_COUNT) &&
      (op != UNC_OP_READ_SIBLING_BEGIN)) {
    /** validate direction */
    if (!ValidateNumericRange(key_vrt_if_flowfilter->direction,
                              (uint8_t) UPLL_FLOWFILTER_DIR_IN,
                              (uint8_t) UPLL_FLOWFILTER_DIR_OUT, true, true)) {
      UPLL_LOG_DEBUG("direction syntax validation failed ");
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  } else {
    // input direction should be not set for
    // sibling begin or count operation
    // as 0 or 1 are valid values setting an invalid value;
    key_vrt_if_flowfilter->direction = 0xFE;
  }
  return UPLL_RC_SUCCESS;
}
bool VrtIfFlowFilterMoMgr::GetRenameKeyBindInfo(unc_key_type_t key_type,
                                    BindInfo *&binfo,
                                    int &nattr,
                                    MoMgrTables tbl ) {
  /* Main Table only update */
  if (MAINTBL == tbl) {
    nattr = sizeof(vrtIfflowfiltermaintbl_bind_info)/
            sizeof(vrtIfflowfiltermaintbl_bind_info[0]);
    binfo = vrtIfflowfiltermaintbl_bind_info;
  }
  UPLL_LOG_DEBUG(" GetRenameKeyBindInfo::Successful Completeion");
  return PFC_TRUE;
}
upll_rc_t VrtIfFlowFilterMoMgr::CopyToConfigKey(ConfigKeyVal *&okey,
                                              ConfigKeyVal *ikey) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  if (!ikey || !(ikey->get_key())) {
    UPLL_LOG_DEBUG("Input Key Not Valid");
    return UPLL_RC_ERR_GENERIC;
  }

  key_rename_vnode_info *key_rename =
      reinterpret_cast<key_rename_vnode_info *>(ikey->get_key());
  key_vrt_if_flowfilter_t * key_vrt_if =
      reinterpret_cast<key_vrt_if_flowfilter_t *>
      (ConfigKeyVal::Malloc(sizeof(key_vrt_if_flowfilter_t)));

  if (!strlen(reinterpret_cast<char *> (key_rename->old_unc_vtn_name))) {
    UPLL_LOG_DEBUG("old_unc_vtn_name NULL");
    if (key_vrt_if) free(key_vrt_if);
    return UPLL_RC_ERR_GENERIC;
  }

  uuu::upll_strncpy(key_vrt_if->if_key.vrt_key.vtn_key.vtn_name,
                    key_rename->old_unc_vtn_name,
                    (kMaxLenVtnName + 1));

  if (UNC_KT_VROUTER == ikey->get_key_type()) {
    if (!strlen(reinterpret_cast<char *>(key_rename->old_unc_vnode_name))) {
      UPLL_LOG_DEBUG("old_unc_vnode_name NULL");
      free(key_vrt_if);
      return UPLL_RC_ERR_GENERIC;
    }
    uuu::upll_strncpy(key_vrt_if->if_key.vrt_key.vrouter_name,
                      key_rename->old_unc_vnode_name, (kMaxLenVnodeName + 1));
  } else {
    if (!strlen(reinterpret_cast<char *>(key_rename->new_unc_vnode_name))) {
      UPLL_LOG_DEBUG("new_unc_vnode_name NULL");
      free(key_vrt_if);
      return UPLL_RC_ERR_GENERIC;
    }

    uuu::upll_strncpy(key_vrt_if->if_key.vrt_key.vrouter_name,
                      key_rename->new_unc_vnode_name, (kMaxLenVnodeName + 1));
  }
  key_vrt_if->direction = 0xFE;

  okey = new ConfigKeyVal(UNC_KT_VRTIF_FLOWFILTER, IpctSt::
                   kIpcStKeyVrtIfFlowfilter, key_vrt_if, NULL);
  if (!okey) {
    UPLL_LOG_DEBUG("okey NULL");
    free(key_vrt_if);
    return UPLL_RC_ERR_GENERIC;
  }
  UPLL_LOG_DEBUG("CopyToConfigKey is  Success");
  return result_code;
}


upll_rc_t VrtIfFlowFilterMoMgr::ReadSiblingMo(IpcReqRespHeader *req,
                                              ConfigKeyVal *ikey,
                                              bool begin, DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_ERR_GENERIC;
  ConfigKeyVal *l_key = NULL;
  ConfigKeyVal *okey = NULL, *tctrl_key = NULL, *tmp_key = NULL;
  controller_domain ctrlr_dom;

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
          UPLL_LOG_DEBUG("DupConfigKeyVal fail in ReadSiblingMo for tctrl_key");
          return result_code;
        }
        result_code = ReadInfoFromDB(req, tctrl_key, dmi, &ctrlr_dom);
        if (UPLL_RC_SUCCESS != result_code) {
          UPLL_LOG_DEBUG("ReadConfigDb failed for tctrl_key%d ", result_code);
          DELETE_IF_NOT_NULL(tctrl_key);
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
        // Adding CapaXheck
        result_code = ValidateCapability(req, ikey,
                   reinterpret_cast<const char *>(ctrlr_dom.ctrlr));
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("validate Capability Failed %d", result_code);
          DELETE_IF_NOT_NULL(tctrl_key);
          DELETE_IF_NOT_NULL(l_key);
          return result_code;
        }

        // 1.Getting renamed name if renamed
        result_code = GetRenamedControllerKey(l_key, req->datatype,
                                              dmi, &ctrlr_dom);
        if (result_code != UPLL_RC_SUCCESS) {
          DELETE_IF_NOT_NULL(tctrl_key);
          DELETE_IF_NOT_NULL(l_key);
          return result_code;
        }

        uint8_t vlink_flag = 0;
        GET_USER_DATA_FLAGS(tctrl_key, vlink_flag);
        if (!(SET_FLAG_VLINK & vlink_flag)) {
          UPLL_LOG_DEBUG("Vlink Not Configured");
          DELETE_IF_NOT_NULL(tctrl_key);
          DELETE_IF_NOT_NULL(l_key);
          return UPLL_RC_ERR_NOT_ALLOWED_AT_THIS_TIME;
        }

        pfcdrv_val_vbrif_vextif_t *pfc_val =
        reinterpret_cast<pfcdrv_val_vbrif_vextif_t *>
        (ConfigKeyVal::Malloc(sizeof(pfcdrv_val_vbrif_vextif_t)));

        pfc_val->valid[PFCDRV_IDX_INTERFACE_TYPE] = UNC_VF_VALID;
        pfc_val->valid[PFCDRV_IDX_VEXTERNAL_NAME_VBRIF] = UNC_VF_INVALID;
        pfc_val->valid[PFCDRV_IDX_VEXT_IF_NAME_VBRIF] = UNC_VF_INVALID;
        pfc_val->interface_type = PFCDRV_IF_TYPE_VBRIF;

        l_key->SetCfgVal(new ConfigVal(IpctSt::kIpcStPfcdrvValVbrifVextif,
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
        while (tmp_key != NULL) {
          reinterpret_cast<key_vrt_if_flowfilter_t*>
              (l_key->get_key())->direction =
              reinterpret_cast<key_vrt_if_flowfilter_t*>
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
            DELETE_IF_NOT_NULL(tctrl_key);
            DELETE_IF_NOT_NULL(l_key);
            DELETE_IF_NOT_NULL(ipc_resp.ckv_data);
            return ipc_resp.header.result_code;
          }

          if (ipc_resp.header.result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("Driver response for Key %d controller %s result %d",
                           l_key->get_key_type(), ctrlr_dom.ctrlr,
                           ipc_resp.header.result_code);
            DELETE_IF_NOT_NULL(tctrl_key);
            DELETE_IF_NOT_NULL(l_key);
            DELETE_IF_NOT_NULL(ipc_resp.ckv_data);
            return ipc_resp.header.result_code;
          }

          result_code = ConstructReadDetailResponse(tmp_key,
                                                    ipc_resp.ckv_data,
                                                    ctrlr_dom,
                                                    &okey, dmi);

          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("ConstructReadDetailResponse error code (%d)",
                           result_code);
            DELETE_IF_NOT_NULL(tctrl_key);
            DELETE_IF_NOT_NULL(l_key);
            DELETE_IF_NOT_NULL(ipc_resp.ckv_data);
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

upll_rc_t VrtIfFlowFilterMoMgr::TxUpdateController(
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
  uint8_t flag = 0;

  // TxUpdate skipped if config mode is VIRTUAL
  if (config_mode == TC_CONFIG_VIRTUAL) {
    return UPLL_RC_SUCCESS;
  }

  if (affected_ctrlr_set == NULL)
    return UPLL_RC_ERR_GENERIC;
  unc_keytype_operation_t op = (phase == uuc::kUpllUcpCreate)?UNC_OP_CREATE:
      ((phase == uuc::kUpllUcpUpdate)?UNC_OP_UPDATE:
       ((phase == uuc::kUpllUcpDelete)?UNC_OP_DELETE:UNC_OP_INVALID));
  if (UNC_OP_UPDATE == op) {
    // update operation is not supported
    // return success
    // return UPLL_RC_SUCCESS;
  }
  unc_keytype_operation_t op1 = op;
  if (tx_util->GetErrCount() > 0) {
     UPLL_LOG_ERROR("Error Occured From Driver Side..Exiting the loop");
     return UPLL_RC_ERR_GENERIC;
  }

  result_code = DiffConfigDB(UPLL_DT_CANDIDATE, UPLL_DT_RUNNING,
                             op, req, nreq, &dal_cursor_handle, dmi,
                            config_mode, vtn_name, MAINTBL);
  while (result_code == UPLL_RC_SUCCESS) {
    if (tx_util->GetErrCount() > 0) {
      UPLL_LOG_ERROR("TxUpdateUtil says exit the loop.");
      dmi->CloseCursor(dal_cursor_handle, true);
      DELETE_IF_NOT_NULL(req);
      DELETE_IF_NOT_NULL(nreq);
      return UPLL_RC_ERR_GENERIC;
    }

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
      case UNC_OP_DELETE:
        op1 = op;
        result_code = DupConfigKeyVal(ck_main, req, MAINTBL);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_TRACE("DupConfigKeyVal failed %d", result_code);
          return result_code;
        }
        break;
      default:
        break;
    }
    if (NULL == ck_main) {
      UPLL_LOG_DEBUG("ck_main is NULL");
      return UPLL_RC_ERR_GENERIC;
    }
    GET_USER_DATA_CTRLR_DOMAIN(ck_main, ctrlr_dom);
    if (ctrlr_dom.ctrlr == NULL) {
      DELETE_IF_NOT_NULL(ck_main);
      return UPLL_RC_ERR_GENERIC;
    }

    GET_USER_DATA_FLAGS(ck_main, db_flag);

    if (!(SET_FLAG_VLINK & db_flag)) {
      if (op1 != UNC_OP_UPDATE) {
        DELETE_IF_NOT_NULL(ck_main);
        continue;
      }
      GET_USER_DATA_FLAGS(nreq, flag);
      if (!(SET_FLAG_VLINK & flag)) {
        DELETE_IF_NOT_NULL(ck_main);
        continue;
      }
      op1 = UNC_OP_DELETE;
    } else {
      if (UNC_OP_UPDATE == op1) {
        GET_USER_DATA_FLAGS(nreq, flag);
        if (!(SET_FLAG_VLINK & flag)) {
          op1 = UNC_OP_CREATE;
        }
      }
    }

    if (op1 == UNC_OP_UPDATE) {
      DELETE_IF_NOT_NULL(ck_main);
      continue;
    }

    upll_keytype_datatype_t dt_type = (op1 == UNC_OP_DELETE)?
        UPLL_DT_RUNNING:UPLL_DT_CANDIDATE;
    result_code = GetRenamedControllerKey(ck_main, dt_type,
                                          dmi, &ctrlr_dom);
    if (result_code != UPLL_RC_SUCCESS) {
      DELETE_IF_NOT_NULL(ck_main);
      break;
    }
    ConfigKeyVal *ckv_driver = NULL;
    result_code = DupConfigKeyVal(ckv_driver, ck_main, MAINTBL);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_TRACE("DupConfigKeyVal failed %d", result_code);
      dmi->CloseCursor(dal_cursor_handle, true);
      DELETE_IF_NOT_NULL(nreq);
      DELETE_IF_NOT_NULL(req);
      DELETE_IF_NOT_NULL(ck_main);
      return result_code;
    }
    pfcdrv_val_vbrif_vextif_t *pfc_val =
    reinterpret_cast<pfcdrv_val_vbrif_vextif_t *>
    (ConfigKeyVal::Malloc(sizeof(pfcdrv_val_vbrif_vextif_t)));

    pfc_val->valid[PFCDRV_IDX_INTERFACE_TYPE] = UNC_VF_VALID;
    pfc_val->valid[PFCDRV_IDX_VEXTERNAL_NAME_VBRIF] = UNC_VF_INVALID;
    pfc_val->valid[PFCDRV_IDX_VEXT_IF_NAME_VBRIF] = UNC_VF_INVALID;
    pfc_val->interface_type = PFCDRV_IF_TYPE_VBRIF;

    ckv_driver->SetCfgVal(new ConfigVal(IpctSt::kIpcStPfcdrvValVbrifVextif,
                                     pfc_val));

    UPLL_LOG_DEBUG("Controller : %s; Domain : %s", ctrlr_dom.ctrlr,
                   ctrlr_dom.domain);
    // Inserting the controller to Set
    affected_ctrlr_set->insert
        (string(reinterpret_cast<char *>(ctrlr_dom.ctrlr)));

    DELETE_IF_NOT_NULL(ck_main);
    ConfigKeyVal *ckv_unc = NULL;
    result_code = DupConfigKeyVal(ckv_unc, req, MAINTBL);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_TRACE("DupConfigKeyVal failed %d", result_code);
      dmi->CloseCursor(dal_cursor_handle, true);
      DELETE_IF_NOT_NULL(nreq);
      DELETE_IF_NOT_NULL(req);
      DELETE_IF_NOT_NULL(ckv_driver);
      return result_code;
    }

    result_code = tx_util->EnqueueRequest(session_id, config_id,
                                          UPLL_DT_CANDIDATE, op1, dmi,
                                          ckv_driver, ckv_unc, string());
    if (result_code != UPLL_RC_SUCCESS) {
      dmi->CloseCursor(dal_cursor_handle, true);
      DELETE_IF_NOT_NULL(nreq);
      DELETE_IF_NOT_NULL(req);
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

upll_rc_t VrtIfFlowFilterMoMgr::SetVlinkPortmapConfiguration(
    ConfigKeyVal *ikey, upll_keytype_datatype_t dt_type,
    DalDmlIntf *dmi, InterfacePortMapInfo flags,
    unc_keytype_operation_t oper,
    TcConfigMode config_mode,
    string vtn_name) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_ERR_GENERIC;
  if (NULL == ikey || NULL == ikey->get_key()) {
    return result_code;
  }
  ConfigKeyVal *ckv = NULL;
  result_code = GetChildConfigKey(ckv, NULL);
  if (UPLL_RC_SUCCESS != result_code) {
    return result_code;
  }
  key_vrt_if_flowfilter_t *ff_key = reinterpret_cast
    <key_vrt_if_flowfilter_t *>(ckv->get_key());
  key_vrt_if_t *vrtif_key = reinterpret_cast<key_vrt_if_t *>(ikey->get_key());

  uuu::upll_strncpy(ff_key->if_key.vrt_key.vtn_key.vtn_name,
      vrtif_key->vrt_key.vtn_key.vtn_name,
      kMaxLenVtnName + 1);

  uuu::upll_strncpy(ff_key->if_key.vrt_key.vrouter_name,
      vrtif_key->vrt_key.vrouter_name,
      kMaxLenVnodeName + 1);

  uuu::upll_strncpy(ff_key->if_key.if_name,
      vrtif_key->if_name,
      kMaxLenInterfaceName + 1);
  ff_key->direction = 0xFE;

  DbSubOp dbop = { kOpReadMultiple, kOpMatchNone, kOpInOutFlag };
  result_code = ReadConfigDB(ckv, dt_type, UNC_OP_READ, dbop, dmi, MAINTBL);
  if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
    UPLL_LOG_DEBUG("No Recrods in the Vrt_If_FlowFilter Table");
    DELETE_IF_NOT_NULL(ckv);
    return UPLL_RC_SUCCESS;
  }
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("Read ConfigDB failure %d", result_code);
    DELETE_IF_NOT_NULL(ckv);
    return result_code;
  }
  uint8_t  flag_port_map = 0;
  ConfigKeyVal *temp_ckv = ckv;
  while (NULL != temp_ckv) {
    flag_port_map = 0;
    GET_USER_DATA_FLAGS(temp_ckv, flag_port_map);
    if (flags & kVlinkConfigured) {
      if (flag_port_map & SET_FLAG_VLINK) {
        UPLL_LOG_DEBUG("vlink flag is already set in DB");
        DELETE_IF_NOT_NULL(ckv);
        return UPLL_RC_SUCCESS;
      }
      flag_port_map |= SET_FLAG_VLINK;
    } else {
      UPLL_LOG_DEBUG("No vlink");
      flag_port_map &= SET_FLAG_NO_VLINK_PORTMAP;
    }
    SET_USER_DATA_FLAGS(temp_ckv, flag_port_map);
    DbSubOp dbop_update = { kOpReadSingle, kOpMatchNone, kOpInOutFlag };
    result_code = UpdateConfigDB(temp_ckv, dt_type, UNC_OP_UPDATE,
        dmi, &dbop_update, config_mode, vtn_name, MAINTBL);
    if (UPLL_RC_SUCCESS != result_code) {
      DELETE_IF_NOT_NULL(ckv);
      return result_code;
    }
    VrtIfFlowFilterEntryMoMgr *mgr =
      reinterpret_cast<VrtIfFlowFilterEntryMoMgr *>
      (const_cast<MoManager *>(GetMoManager(UNC_KT_VRTIF_FLOWFILTER_ENTRY)));
    if (mgr == NULL) {
      DELETE_IF_NOT_NULL(ckv);
      return UPLL_RC_ERR_GENERIC;
    }
    UPLL_LOG_DEBUG("Sending vlink flag=%d for ff entry", flags);
    result_code = mgr->SetVlinkPortmapConfiguration(temp_ckv, dt_type,
                                                    dmi, flags, oper,
                                                    config_mode, vtn_name);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("Unable to update vlink flag for flowfilterentry, err %d",
          result_code);
      DELETE_IF_NOT_NULL(ckv);
      return result_code;
    }
    temp_ckv = temp_ckv->get_next_cfg_key_val();
  }
  DELETE_IF_NOT_NULL(ckv);
  return UPLL_RC_SUCCESS;
}

upll_rc_t VrtIfFlowFilterMoMgr::GetControllerDomainID(ConfigKeyVal *ikey,
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

  key_vrt_if_flowfilter_t *temp_key =
      reinterpret_cast<key_vrt_if_flowfilter_t*>(ikey->get_key());

  key_vrt_if_t *vrt_if_key = reinterpret_cast<key_vrt_if_t*>(ckv->get_key());

  uuu::upll_strncpy(vrt_if_key->vrt_key.vtn_key.vtn_name,
                    temp_key->if_key.vrt_key.vtn_key.vtn_name,
                    kMaxLenVtnName + 1);
  uuu::upll_strncpy(vrt_if_key->vrt_key.vrouter_name,
                    temp_key->if_key.vrt_key.vrouter_name,
                    kMaxLenVnodeName + 1);

  uuu::upll_strncpy(vrt_if_key->if_name,
                    temp_key->if_key.if_name,
                    kMaxLenInterfaceName + 1);


  ConfigKeyVal *vrt_key = NULL;
  result_code = mgrvrtif->GetParentConfigKey(vrt_key, ckv);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetParentConfigKey failed, err %d", result_code);
    DELETE_IF_NOT_NULL(ckv);
    return result_code;
  }

  result_code = mgrvrtif->GetControllerDomainId(vrt_key, dt_type,
                                                &ctrlr_dom, dmi);
  if ((result_code != UPLL_RC_SUCCESS) || (ctrlr_dom.ctrlr == NULL)
      || (ctrlr_dom.domain == NULL)) {
    UPLL_LOG_INFO("GetControllerDomainId error err code(%d)", result_code);
    DELETE_IF_NOT_NULL(vrt_key);
    DELETE_IF_NOT_NULL(ckv);
    return result_code;
  }

  SET_USER_DATA_CTRLR_DOMAIN(ikey, ctrlr_dom);
  UPLL_LOG_DEBUG("ctrlrid %s, domainid %s",
                   ctrlr_dom.ctrlr, ctrlr_dom.domain);
  DELETE_IF_NOT_NULL(vrt_key);
  DELETE_IF_NOT_NULL(ckv);
  return result_code;
}

upll_rc_t VrtIfFlowFilterMoMgr::GetParentConfigKey(ConfigKeyVal *&okey,
                                                 ConfigKeyVal *ikey) {
  UPLL_FUNC_TRACE;

  if (!ikey) {
    UPLL_LOG_DEBUG(" Input Key is NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  DELETE_IF_NOT_NULL(okey);
  unc_key_type_t ikey_type = ikey->get_key_type();
  if (ikey_type != UNC_KT_VRTIF_FLOWFILTER) {
    UPLL_LOG_DEBUG(" Invalid key type received. Key type - %d", ikey_type);
    return UPLL_RC_ERR_GENERIC;
  }

  key_vrt_if_flowfilter_t *pkey =
      reinterpret_cast<key_vrt_if_flowfilter_t*>(ikey->get_key());
  if (!pkey) {
    UPLL_LOG_DEBUG(" Input vrt if flow filter key is NULL ");
    return UPLL_RC_ERR_GENERIC;
  }

  key_vrt_if_t *vrt_if_key = reinterpret_cast<key_vrt_if_t*>
      (ConfigKeyVal::Malloc(sizeof(key_vrt_if_t)));

  uuu::upll_strncpy(vrt_if_key->vrt_key.vtn_key.vtn_name,
                    reinterpret_cast<key_vrt_if_flowfilter_t *>
                    (pkey)->if_key.vrt_key.vtn_key.vtn_name,
                    kMaxLenVtnName + 1);
  uuu::upll_strncpy(vrt_if_key->vrt_key.vrouter_name,
                    reinterpret_cast<key_vrt_if_flowfilter_t *>
                    (pkey)->if_key.vrt_key.vrouter_name,
                    kMaxLenVnodeName + 1);
  uuu::upll_strncpy(vrt_if_key->if_name,
                    reinterpret_cast<key_vrt_if_flowfilter_t *>
                    (pkey)->if_key.if_name,
                    kMaxLenInterfaceName + 1);
  okey = new ConfigKeyVal(UNC_KT_VRT_IF,
                          IpctSt::kIpcStKeyVrtIf,
                          vrt_if_key, NULL);
  SET_USER_DATA(okey, ikey);
  return UPLL_RC_SUCCESS;
}

upll_rc_t VrtIfFlowFilterMoMgr::ConstructReadDetailResponse(
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
    DELETE_IF_NOT_NULL(tmp_okey);
    free(val_ff);
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
      // redirect-direction is IN or OUT
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

upll_rc_t VrtIfFlowFilterMoMgr::CreateAuditMoImpl(ConfigKeyVal *ikey,
                                   DalDmlIntf *dmi,
                                   const char *ctrlr_id) {
  UPLL_FUNC_TRACE;
  uint8_t flags = 0;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  string vtn_name = "";
  UPLL_LOG_TRACE(" ikey is %s", ikey->ToStrAll().c_str());

  pfcdrv_val_vbrif_vextif_t *pfc_val =
    reinterpret_cast<pfcdrv_val_vbrif_vextif_t *> (GetVal(ikey));

  if (pfc_val == NULL) {
    UPLL_LOG_DEBUG("Val Structure is EWmpty");
    return UPLL_RC_ERR_GENERIC;
  }
  GET_USER_DATA_FLAGS(ikey, flags);
  if (pfc_val->interface_type == PFCDRV_IF_TYPE_VBRIF) {
    flags |= SET_FLAG_VLINK;
  }

  ConfigKeyVal *okey = NULL;
  result_code = GetChildConfigKey(okey, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetChildConfigKey Failed");
    return result_code;
  }

  SET_USER_DATA_FLAGS(okey, flags);

  controller_domain ctrlr_dom;
  memset(&ctrlr_dom, 0, sizeof(controller_domain));
  result_code = GetControllerDomainID(okey, UPLL_DT_AUDIT, dmi);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Failed to Get the Controller Domain details, err:%d",
                   result_code);
  }
  GET_USER_DATA_CTRLR_DOMAIN(okey, ctrlr_dom);
  UPLL_LOG_DEBUG("ctrlrid %s, domainid %s",
                 ctrlr_dom.ctrlr, ctrlr_dom.domain);

  // Create a record in AUDIT DB
  result_code = UpdateConfigDB(okey, UPLL_DT_AUDIT, UNC_OP_CREATE,
                               dmi, TC_CONFIG_GLOBAL, vtn_name, MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("UpdateConfigDB Failed err_code %d", result_code);
    DELETE_IF_NOT_NULL(okey);
    return result_code;
  }
  DELETE_IF_NOT_NULL(okey);
  return result_code;
}

upll_rc_t VrtIfFlowFilterMoMgr::UpdateAuditConfigStatus(
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
    UPLL_LOG_DEBUG("UpdateAuditConfigStatus :: Memory Not Allocated");
    return UPLL_RC_ERR_GENERIC;
  }
  if (uuc::kUpllUcpCreate == phase )
    val->cs_row_status = cs_status;
  if ((uuc::kUpllUcpUpdate == phase) &&
           (val->cs_row_status == UNC_CS_INVALID ||
            val->cs_row_status == UNC_CS_NOT_APPLIED))
    val->cs_row_status = cs_status;

  UPLL_LOG_DEBUG("Update  Audit Config Status Successfull");
  return result_code;
}

upll_rc_t VrtIfFlowFilterMoMgr::AuditUpdateController(unc_key_type_t keytype,
                             const char *ctrlr_id,
                             uint32_t session_id,
                             uint32_t config_id,
                             uuc::UpdateCtrlrPhase phase,
                             DalDmlIntf *dmi,
                             ConfigKeyVal **err_ckv,
                             KTxCtrlrAffectedState *ctrlr_affected) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  DalResultCode db_result = uud::kDalRcSuccess;
  MoMgrTables tbl  = MAINTBL;
  controller_domain_t ctrlr_dom;
  ctrlr_dom.ctrlr = NULL;
  ctrlr_dom.domain = NULL;
  ConfigKeyVal  *ckv_running_db = NULL;
  ConfigKeyVal  *ckv_audit_db = NULL;
  ConfigKeyVal  *ckv_driver_req = NULL;
  ConfigKeyVal  *ckv_audit_dup_db = NULL;
  DalCursor *cursor = NULL;
  uint8_t db_flag = 0;
  // uint8_t flag = 0;
  string vtn_name = "";
  uint8_t *ctrlr = reinterpret_cast<uint8_t *>(const_cast<char *>(ctrlr_id));
  /* decides whether to retrieve from controller table or main table */
  GET_TABLE_TYPE(keytype, tbl);
  unc_keytype_operation_t op = (phase == uuc::kUpllUcpCreate)?UNC_OP_CREATE:
               ((phase == uuc::kUpllUcpUpdate)?UNC_OP_UPDATE:
               ((phase == uuc::kUpllUcpDelete)?UNC_OP_DELETE:UNC_OP_INVALID));

  unc_keytype_operation_t op1 = op;
  if (phase == uuc::kUpllUcpDelete2)
     return result_code;
  /* retreives the delta of running and audit configuration */
  UPLL_LOG_DEBUG("Operation is %d", op);
  result_code = DiffConfigDB(UPLL_DT_RUNNING, UPLL_DT_AUDIT, op,
        ckv_running_db, ckv_audit_db,
        &cursor, dmi, ctrlr, TC_CONFIG_GLOBAL, vtn_name, tbl, true, true);
  if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    UPLL_LOG_DEBUG("No more diff found for operation %d", op);
    DELETE_IF_NOT_NULL(ckv_running_db);
    DELETE_IF_NOT_NULL(ckv_audit_db);
    return UPLL_RC_SUCCESS;
  }
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("DiffConfigDB failed - %d", result_code);
    DELETE_IF_NOT_NULL(ckv_running_db);
    DELETE_IF_NOT_NULL(ckv_audit_db);
    return result_code;
  }
  while (uud::kDalRcSuccess == (db_result = dmi->GetNextRecord(cursor)) &&
         ((result_code = ContinueAuditProcess()) == UPLL_RC_SUCCESS)) {
    op1 = op;
    if (phase != uuc::kUpllUcpDelete) {
      uint8_t *db_ctrlr = NULL;
      GET_USER_DATA_CTRLR(ckv_running_db, db_ctrlr);
      UPLL_LOG_DEBUG("db ctrl_id and audit ctlr_id are  %s %s",
                     db_ctrlr, ctrlr_id);
      // Skipping the controller ID if the controller id in DB and
      // controller id available for Audit are not the same
      if (db_ctrlr &&
          strncmp(reinterpret_cast<const char *>(db_ctrlr),
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
          result_code = DupConfigKeyVal(ckv_driver_req, ckv_running_db,
                                        MAINTBL);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("DupConfigKeyVal failed. err_code & phase %d %d",
                           result_code, phase);
            DELETE_IF_NOT_NULL(ckv_running_db);
            DELETE_IF_NOT_NULL(ckv_audit_db);
            if (cursor)
              dmi->CloseCursor(cursor, true);
            return result_code;
          }
         break;
      case uuc::kUpllUcpUpdate:
          ckv_audit_dup_db = NULL;
          ckv_driver_req = NULL;
          UPLL_LOG_TRACE("UpdateRecord  running DB record  is %s ",
                          ckv_running_db->ToStrAll().c_str());
          UPLL_LOG_TRACE("UpdateRecord  AuditDB record  is %s ",
                          ckv_audit_db->ToStrAll().c_str());
          result_code = DupConfigKeyVal(ckv_driver_req, ckv_running_db,
                                        MAINTBL);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("DupConfigKeyVal failed for running record. "
                           "err_code & phase %d %d", result_code, phase);
            DELETE_IF_NOT_NULL(ckv_running_db);
            DELETE_IF_NOT_NULL(ckv_audit_db);
            if (cursor)
              dmi->CloseCursor(cursor, true);
            return result_code;
          }
          result_code = DupConfigKeyVal(ckv_audit_dup_db,
                                        ckv_audit_db, MAINTBL);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("DupConfigKeyVal failed for audit record. "
                           "err_code & phase %d %d", result_code, phase);
            DELETE_IF_NOT_NULL(ckv_driver_req);
            DELETE_IF_NOT_NULL(ckv_running_db);
            DELETE_IF_NOT_NULL(ckv_audit_db);
            if (cursor)
              dmi->CloseCursor(cursor, true);
            return result_code;
          }
        break;
      default:
        UPLL_LOG_DEBUG("Invalid operation %d", phase);
        DELETE_IF_NOT_NULL(ckv_running_db);
        DELETE_IF_NOT_NULL(ckv_audit_db);
        return UPLL_RC_ERR_NO_SUCH_OPERATION;
    }
    if (!ckv_driver_req) return UPLL_RC_ERR_GENERIC;
    GET_USER_DATA_CTRLR_DOMAIN(ckv_driver_req, ctrlr_dom);
    if ((NULL == ctrlr_dom.ctrlr) || (NULL == ctrlr_dom.domain)) {
      UPLL_LOG_INFO("controller id or domain is NULL");
      DELETE_IF_NOT_NULL(ckv_driver_req);
      DELETE_IF_NOT_NULL(ckv_audit_dup_db);
      DELETE_IF_NOT_NULL(ckv_running_db);
      DELETE_IF_NOT_NULL(ckv_audit_db);
      if (cursor)
        dmi->CloseCursor(cursor, true);
      return UPLL_RC_ERR_GENERIC;
    }
    UPLL_LOG_DEBUG("Controller : %s; Domain : %s", ctrlr_dom.ctrlr,
                 ctrlr_dom.domain);
    db_flag = 0;
    GET_USER_DATA_FLAGS(ckv_driver_req, db_flag);

    // If vlink flag is not set at running and the operation is update
    // then vlink is deleted in the update phase from UNC
    // hence flowfilter also should get deleted from controller
    // hence sending the delete request to the controller driver
    if ((SET_FLAG_VLINK & db_flag)) {
      // Continue the operations
    } else {
      if (UNC_OP_UPDATE == op1) {
        op1 = UNC_OP_DELETE;
      } else {
        // No Vlink Configured, Configuration is  not
        // sent to the driver of the controller.
        DELETE_IF_NOT_NULL(ckv_driver_req);
        DELETE_IF_NOT_NULL(ckv_audit_dup_db);
        continue;
      }
    }
    if (UNC_OP_UPDATE == op1) {
      void *running_val = NULL;
      bool invalid_attr = false;
      running_val = GetVal(ckv_running_db);
      invalid_attr = FilterAttributes(running_val,
           GetVal(ckv_audit_dup_db), false, UNC_OP_UPDATE);
      if (invalid_attr) {
        DELETE_IF_NOT_NULL(ckv_driver_req);
        DELETE_IF_NOT_NULL(ckv_audit_dup_db);
        // Assuming that the diff found only in ConfigStatus
        // Setting the   value as OnlyCSDiff in the out parameter ctrlr_affected
        // The value Configdiff should be given more priority than the value
        // onlycs. So  If the out parameter ctrlr_affected has
        // already value as configdiff then dont change the value
        if (*ctrlr_affected != uuc::kCtrlrAffectedConfigDiff) {
           UPLL_LOG_INFO("Setting the ctrlr_affected to OnlyCSDiff");
           *ctrlr_affected = uuc::kCtrlrAffectedOnlyCSDiff;
        }
        continue;
      }
    }

    DELETE_IF_NOT_NULL(ckv_audit_dup_db);
    upll_keytype_datatype_t dt_type = (op1 == UNC_OP_DELETE)?
      UPLL_DT_AUDIT : UPLL_DT_RUNNING;

    result_code = GetRenamedControllerKey(ckv_driver_req, dt_type,
                                          dmi, &ctrlr_dom);
    if (result_code != UPLL_RC_SUCCESS &&
        result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      UPLL_LOG_DEBUG(" GetRenamedControllerKey failed err code(%d)",
                     result_code);
      DELETE_IF_NOT_NULL(ckv_driver_req);
      DELETE_IF_NOT_NULL(ckv_running_db);
      DELETE_IF_NOT_NULL(ckv_audit_db);
      if (cursor)
        dmi->CloseCursor(cursor, true);
      return result_code;
    }

    pfcdrv_val_vbrif_vextif_t *pfc_val =
    reinterpret_cast<pfcdrv_val_vbrif_vextif_t *>
    (ConfigKeyVal::Malloc(sizeof(pfcdrv_val_vbrif_vextif_t)));

    pfc_val->valid[PFCDRV_IDX_INTERFACE_TYPE] = UNC_VF_VALID;
    pfc_val->valid[PFCDRV_IDX_VEXTERNAL_NAME_VBRIF] = UNC_VF_INVALID;
    pfc_val->valid[PFCDRV_IDX_VEXT_IF_NAME_VBRIF] = UNC_VF_INVALID;
    pfc_val->interface_type = PFCDRV_IF_TYPE_VBRIF;

    ckv_driver_req->SetCfgVal(new ConfigVal(IpctSt::kIpcStPfcdrvValVbrifVextif,
                                            pfc_val));

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
            (const char *)ctrlr_dom.ctrlr, reinterpret_cast<char *>
            (ctrlr_dom.domain), PFCDRIVER_SERVICE_NAME,
            PFCDRIVER_SVID_LOGICAL, &ipc_req, true, &ipc_response)) {
      UPLL_LOG_INFO("Request to driver for Key %d for controller %s failed ",
                    ckv_driver_req->get_key_type(),
                    reinterpret_cast<char *>(ctrlr_dom.ctrlr));

      DELETE_IF_NOT_NULL(ckv_driver_req);
      DELETE_IF_NOT_NULL(ckv_running_db);
      DELETE_IF_NOT_NULL(ckv_audit_db);
      if (cursor)
        dmi->CloseCursor(cursor, true);
      return ipc_response.header.result_code;
    }
    if (ipc_response.header.result_code != UPLL_RC_SUCCESS) {
       UPLL_LOG_DEBUG("driver return failure err_code is %d",
                      ipc_response.header.result_code);
      *err_ckv = ckv_running_db;

       if (phase != uuc::kUpllUcpDelete) {
         ConfigKeyVal *resp = NULL;

         result_code = GetChildConfigKey(resp, ipc_response.ckv_data);
         if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG(
                "DupConfigKeyVal failed for ipc response ckv err_code %d",
                 result_code);
            DELETE_IF_NOT_NULL(ipc_response.ckv_data);
            DELETE_IF_NOT_NULL(ckv_driver_req);
            DELETE_IF_NOT_NULL(ckv_running_db);
            DELETE_IF_NOT_NULL(ckv_audit_db);
            *err_ckv = NULL;
            if (cursor)
               dmi->CloseCursor(cursor, true);
            return result_code;
         }
         DbSubOp dbop = { kOpReadMultiple, kOpMatchNone, kOpInOutNone };
         result_code = ReadConfigDB(resp, dt_type, UNC_OP_READ,
                                    dbop, dmi, MAINTBL);
         if (result_code != UPLL_RC_SUCCESS) {
           UPLL_LOG_DEBUG("ReadConfigDB Failed");
           DELETE_IF_NOT_NULL(ckv_driver_req);
           DELETE_IF_NOT_NULL(ipc_response.ckv_data);
           DELETE_IF_NOT_NULL(ckv_running_db);
           DELETE_IF_NOT_NULL(ckv_audit_db);
           *err_ckv = NULL;
           if (cursor)
             dmi->CloseCursor(cursor, true);
            return result_code;
         }

         result_code = UpdateAuditConfigStatus(UNC_CS_INVALID, phase,
                                               resp, dmi);
         if (result_code != UPLL_RC_SUCCESS) {
           UPLL_LOG_TRACE("Update Audit config status failed %d",
                   result_code);
           DELETE_IF_NOT_NULL(ckv_driver_req);
           DELETE_IF_NOT_NULL(ipc_response.ckv_data);
           DELETE_IF_NOT_NULL(ckv_running_db);
           DELETE_IF_NOT_NULL(ckv_audit_db);
           *err_ckv = NULL;
           if (cursor)
             dmi->CloseCursor(cursor, true);
           return result_code;
         }
         DbSubOp dbop1 = { kOpNotRead, kOpMatchNone, kOpInOutCs };
         result_code = UpdateConfigDB(resp, dt_type, UNC_OP_UPDATE,
                                      dmi, &dbop1, TC_CONFIG_GLOBAL,
                                      vtn_name, tbl);
         if (result_code != UPLL_RC_SUCCESS) {
           UPLL_LOG_DEBUG(
               "UpdateConfigDB failed for ipc response ckv err_code %d",
               result_code);
           DELETE_IF_NOT_NULL(ckv_driver_req);
           DELETE_IF_NOT_NULL(ipc_response.ckv_data);
           DELETE_IF_NOT_NULL(ckv_running_db);
           DELETE_IF_NOT_NULL(ckv_audit_db);
           *err_ckv = NULL;
           if (cursor)
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

  if (cursor)
      dmi->CloseCursor(cursor, true);
  if (uud::kDalRcSuccess != db_result) {
     UPLL_LOG_DEBUG("GetNextRecord from database failed  - %d", db_result);
     result_code =  DalToUpllResCode(db_result);
  }

  DELETE_IF_NOT_NULL(ckv_running_db);
  DELETE_IF_NOT_NULL(ckv_audit_db);
  result_code = (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE)
                ? UPLL_RC_SUCCESS : result_code;
  return result_code;
}

upll_rc_t VrtIfFlowFilterMoMgr::DeleteChildrenPOM(
    ConfigKeyVal *ikey,
    upll_keytype_datatype_t dt_type,
    DalDmlIntf *dmi,
    TcConfigMode config_mode,
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
  if (!tempckv || UPLL_RC_SUCCESS != result_code) {
    DELETE_IF_NOT_NULL(tempckv);
    return result_code;
  }
  result_code = UpdateConfigDB(tempckv, dt_type, UNC_OP_DELETE, dmi,
      config_mode, vtn_name, MAINTBL);
  if (UPLL_RC_SUCCESS != result_code) {
    if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
      UPLL_LOG_DEBUG("UPLL_RC_ERR_NO_SUCH_INSTANCE");
      DELETE_IF_NOT_NULL(tempckv);
      return UPLL_RC_SUCCESS;
    }
    UPLL_LOG_DEBUG("DeleteMo record Err in vtnpolicingmaptbl (%d)",
        result_code);
    DELETE_IF_NOT_NULL(tempckv);
    return result_code;
  }
  delete tempckv;
  tempckv = NULL;
  return  UPLL_RC_SUCCESS;
}

upll_rc_t VrtIfFlowFilterMoMgr::SetValidAudit(ConfigKeyVal *&ikey) {
  UPLL_FUNC_TRACE;
  val_flowfilter_t *val = reinterpret_cast<val_flowfilter_t *>
    (ConfigKeyVal::Malloc(sizeof(val_flowfilter_t)));
  val->cs_row_status = UNC_CS_APPLIED;
  ikey->AppendCfgVal(IpctSt::kIpcStValFlowfilter, val);
  return UPLL_RC_SUCCESS;
}

bool VrtIfFlowFilterMoMgr::FilterAttributes(void *&val1,
                                          void *val2,
                                          bool copy_to_running,
                                          unc_keytype_operation_t op) {
  UPLL_FUNC_TRACE;
  if (op != UNC_OP_CREATE)
    return true;
  return false;
}
upll_rc_t VrtIfFlowFilterMoMgr::TxUpdateErrorHandler(
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
