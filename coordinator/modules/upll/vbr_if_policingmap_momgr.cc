/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
#include "momgr_impl.hh"
#include "vtn_momgr.hh"
#include "vbr_momgr.hh"
#include "policingprofile_momgr.hh"
#include "policingprofile_entry_momgr.hh"
#include "vtn_policingmap_momgr.hh"
#include "vbr_if_policingmap_momgr.hh"
#include "vbr_if_momgr.hh"
#include "upll_log.hh"
#include "upll_validation.hh"
#include "unc/upll_ipc_enum.h"

namespace unc {
namespace upll {
namespace kt_momgr {

#define VBRIF_KEY_COL  5
#define POLICY_KEY_COL 5
#define VTN_RENAME 0x01
#define VBR_RENAME 0x10
#define POLICINGPROFILE_RENAME 0x04
#define VLINK_CONFIGURED 0x01
#define PORTMAP_CONFIGURED 0x02
#define VLINK_PORTMAP_CONFIGURED 0x03
#define SET_FLAG_VLINK 0x08
#define SET_FLAG_PORTMAP 0x0A
#define SET_FLAG_VLINK_PORTMAP 0x018

BindInfo VbrIfPolicingMapMoMgr::vbrifpolicingmap_bind_info[] = {
  { uudst::vbr_if_policingmap::kDbiVtnName, CFG_KEY,
    offsetof(key_vbr_if_t, vbr_key.vtn_key.vtn_name),
    uud::kDalChar, (kMaxLenVtnName + 1) },
  { uudst::vbr_if_policingmap::kDbiVbrName, CFG_KEY,
    offsetof(key_vbr_if_t, vbr_key.vbridge_name),
    uud::kDalChar, (kMaxLenVnodeName + 1) },
  { uudst::vbr_if_policingmap::kDbiVbrIfName, CFG_KEY,
    offsetof(key_vbr_if_t, if_name),
    uud::kDalChar, (kMaxLenInterfaceName + 1) },
  { uudst::vbr_if_policingmap::kDbiCtrlrname, CK_VAL,
    offsetof(key_user_data_t, ctrlr_id),
    uud::kDalChar, (kMaxLenCtrlrId + 1) },
  { uudst::vbr_if_policingmap::kDbiDomainId, CK_VAL,
    offsetof(key_user_data_t, domain_id),
    uud::kDalChar, (kMaxLenDomainId + 1) },
  { uudst::vbr_if_policingmap::kDbiPolicername, CFG_VAL,
    offsetof(val_policingmap_t, policer_name),
    uud::kDalChar, (kMaxLenPolicingProfileName + 1) },
  { uudst::vbr_if_policingmap::kDbiFlags, CK_VAL,
    offsetof(key_user_data_t, flags),
    uud::kDalUint8, 1 },
  { uudst::vbr_if_policingmap::kDbiValidPolicername, CFG_META_VAL,
    offsetof(val_policingmap_t, valid[0]),
    uud::kDalUint8, 1 },
  { uudst::vbr_if_policingmap::kDbiCsRowStatus, CS_VAL,
    offsetof(val_policingmap_t, cs_row_status),
    uud::kDalUint8, 1 },
  { uudst::vbr_if_policingmap::kDbiCsPolicername, CS_VAL,
    offsetof(val_policingmap_t, cs_attr[0]),
    uud::kDalUint8, 1 }
};


// Rename
BindInfo VbrIfPolicingMapMoMgr::key_vbrifpm_maintbl_rename_bind_info[] = {
  { uudst::vbr_if_policingmap::kDbiVtnName, CFG_MATCH_KEY,
    offsetof(key_vbr_if_t, vbr_key.vtn_key.vtn_name),
    uud::kDalChar, (kMaxLenVtnName + 1) },
  { uudst::vbr_if_policingmap::kDbiVbrName, CFG_MATCH_KEY,
    offsetof(key_vbr_if_t, vbr_key.vbridge_name),
    uud::kDalChar, (kMaxLenVnodeName + 1) },
  { uudst::vbr_if_policingmap::kDbiVbrIfName, CFG_MATCH_KEY,
    offsetof(key_vbr_if_t, if_name),
    uud::kDalChar, (kMaxLenInterfaceName + 1) },
  { uudst::vbr_if_policingmap::kDbiVtnName, CFG_INPUT_KEY,
    offsetof(key_rename_vnode_info_t, new_unc_vtn_name),
    uud::kDalChar, (kMaxLenVtnName + 1) },
  { uudst::vbr_if_policingmap::kDbiVbrName, CFG_INPUT_KEY,
    offsetof(key_rename_vnode_info_t, new_unc_vnode_name),
    uud::kDalChar, (kMaxLenVnodeName + 1) },
  { uudst::vbr_if_policingmap::kDbiFlags, CFG_INPUT_KEY,
    offsetof(key_user_data_t, flags),
    uud::kDalUint8, 1 }
};

// Rename
BindInfo VbrIfPolicingMapMoMgr::
key_vbrifpm_policyname_maintbl_rename_bind_info[] = {
  { uudst::vbr_if_policingmap::kDbiVtnName, CFG_MATCH_KEY,
    offsetof(key_vbr_if_t, vbr_key.vtn_key.vtn_name),
    uud::kDalChar, (kMaxLenVtnName + 1) },
  { uudst::vbr_if_policingmap::kDbiVbrName, CFG_MATCH_KEY,
    offsetof(key_vbr_if_t, vbr_key.vbridge_name),
    uud::kDalChar, (kMaxLenVnodeName + 1) },
  { uudst::vbr_if_policingmap::kDbiPolicername, CFG_INPUT_KEY,
    offsetof(key_rename_vnode_info_t, new_policingprofile_name),
    uud::kDalChar, (kMaxLenPolicingProfileName + 1) },
  { uudst::vbr_if_policingmap::kDbiFlags, CFG_INPUT_KEY,
    offsetof(key_user_data_t, flags),
    uud::kDalUint8, 1 }
};

// Constructor
VbrIfPolicingMapMoMgr::VbrIfPolicingMapMoMgr() : MoMgrImpl() {
  UPLL_FUNC_TRACE;
  // Rename and ctrlr tables not required for this KT
  // setting max tables to 1
  ntable = MAX_MOMGR_TBLS;
  table = new Table *[ntable];

  // For Main Table
  table[MAINTBL] = new Table(uudst::kDbiVbrIfPolicingMapTbl,
      UNC_KT_VBRIF_POLICINGMAP, vbrifpolicingmap_bind_info,
      IpctSt::kIpcStKeyVbrIf, IpctSt::kIpcStValPolicingmap,
      uudst::vbr_if_policingmap::kDbiVbrIfPolicingMapNumCols);

  /* For Rename Table*/
  table[CTRLRTBL] = NULL;

  /* For Rename Table*/
  table[RENAMETBL] = NULL;

  nchild = 0;
  child = NULL;
}

upll_rc_t VbrIfPolicingMapMoMgr::CreateCandidateMo(IpcReqRespHeader *req,
                                                   ConfigKeyVal *ikey,
                                                   DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  if (NULL == ikey || NULL == req || NULL == dmi) {
    pfc_log_debug("CreateCandidateMo Failed. Insufficient input parameters");
    return UPLL_RC_ERR_GENERIC;
  }

  upll_rc_t result_code = UPLL_RC_SUCCESS;

  // validate syntax and semantics
  result_code = ValidateMessage(req, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    pfc_log_error("ValidateMessage Err (%d)", result_code);
    return result_code;
  }
  result_code = ValidateAttribute(ikey, dmi, req);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("ValidateAttribute failed, Error - %d", result_code);
    return result_code;
  }

  ConfigKeyVal *okey = NULL;
  result_code = GetControllerId(ikey, okey, dmi);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetControllerId failed %d", result_code);
    return result_code;
  }
  delete okey;

  // Capability Check
  result_code = ValidateCapability(req, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Key not supported by controller");
    return result_code;
  }

  // Check VBRIF object existence in VbrIfPolicingMap CANDIDATE DB
  // if record exists, return the error code
  result_code = IsReferenced(ikey, req->datatype, dmi);
  if (result_code == UPLL_RC_ERR_INSTANCE_EXISTS) {
    UPLL_LOG_DEBUG("CreateCandidateMo Object exists in CANDIDATE DB (%d)",
                   result_code);
    return result_code;
  } else if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    UPLL_LOG_DEBUG("CreateCandidateMo Instance Available");
  } else if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("CreateCandidateMo Error Accesing CANDIDATE DB (%d)",
                   result_code);
    return result_code;
  }

  val_policingmap_t *val_pm = reinterpret_cast<val_policingmap_t *>
      (GetVal(ikey));
  if (UNC_VF_VALID == val_pm->valid[UPLL_IDX_POLICERNAME_PM]) {
    // Check Policingmap object exists in PolicingProfileTbl CANDIDATE DB
    // If record not exists, return error code
    result_code = IsPolicyProfileReferenced(ikey, req->datatype, dmi,
                                            UNC_OP_READ);
    if (UPLL_RC_SUCCESS != result_code) {
      if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
        UPLL_LOG_DEBUG("Profile Object (%d) not available in CANDIDATE DB",
                       result_code);
        return UPLL_RC_ERR_CFG_SEMANTIC;
      } else {
        UPLL_LOG_DEBUG("CreateCandidateMo Error Accesing CANDIDATEDB(%d)",
                       result_code);
        return result_code;
      }
    }
    // 1)Get vbrif associated ctrlr name and invoke the PP and PPE functions to
    // check the refcount capability and update the refcount or create the
    // record in policingprofilectrltbl and policingprofileentryctrltbl.
    // 2)Create the record in policingprofileentryctrltbl
    result_code = UpdateRefCountInPPCtrlr(ikey, req->datatype, dmi,
                                          UNC_OP_CREATE);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("UpdateRefCountInPPCtrlr Err in CANDIDATE DB(%d)",
                     result_code);
      return result_code;
    }
  }

  // create a record in vbrifpolicingmap table CANDIDATE DB
  VbrIfMoMgr *mgrvbrif =
      reinterpret_cast<VbrIfMoMgr *>(const_cast<MoManager *>(GetMoManager(
                  UNC_KT_VBR_IF)));
  ConfigKeyVal *ckv = NULL;
  uint8_t* vexternal = reinterpret_cast<uint8_t*>
      (ConfigKeyVal::Malloc(kMaxLenVnodeName + 1));
  uint8_t* vex_if = reinterpret_cast<uint8_t*>
      (ConfigKeyVal::Malloc(kMaxLenInterfaceName + 1));
  InterfacePortMapInfo flags = kVlinkPortMapNotConfigured;

  result_code = mgrvbrif->GetChildConfigKey(ckv, NULL);

  key_vbr_if_t *temp_key = reinterpret_cast<key_vbr_if_t*>
      (ikey->get_key());

  key_vbr_if_t *vbr_if_key = reinterpret_cast<key_vbr_if_t*>
      (ckv->get_key());

  uuu::upll_strncpy(vbr_if_key->vbr_key.vtn_key.vtn_name,
                    temp_key->vbr_key.vtn_key.vtn_name,
                    kMaxLenVtnName + 1);

  uuu::upll_strncpy(vbr_if_key->vbr_key.vbridge_name,
                    temp_key->vbr_key.vbridge_name,
                    kMaxLenVnodeName + 1);

  uuu::upll_strncpy(vbr_if_key->if_name,
                    temp_key->if_name,
                    kMaxLenInterfaceName + 1);

  result_code = mgrvbrif->GetVexternal(ckv, req->datatype, dmi,
                                       vexternal, vex_if, flags);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetVexternal failed %d", result_code);
    return result_code;
  }
  UPLL_LOG_DEBUG("GetVexternal flag %d", flags);
  uint8_t flag_port_map = 0;
  if (flags & VLINK_CONFIGURED) {
    UPLL_LOG_DEBUG("VLINK_CONFIGURED");
    flag_port_map = SET_FLAG_VLINK;
  } else if (flags & PORTMAP_CONFIGURED) {
    UPLL_LOG_DEBUG("PORTMAP_CONFIGURED");
    flag_port_map =  SET_FLAG_PORTMAP;
  } else if (flags & VLINK_PORTMAP_CONFIGURED) {
    UPLL_LOG_DEBUG("VLINK_PORTMAP_CONFIGURED");
    flag_port_map = SET_FLAG_VLINK_PORTMAP;
  } else {
    UPLL_LOG_DEBUG("flag_port_map 0");
    flag_port_map = 0;
  }

  UPLL_LOG_DEBUG("flag_port_map %d", flag_port_map);
  SET_USER_DATA_FLAGS(ikey, flag_port_map);
  DbSubOp dbop = { kOpReadSingle, kOpMatchNone,
    kOpInOutFlag|kOpInOutCtrlr|kOpInOutDomain };
  result_code = UpdateConfigDB(ikey, req->datatype, UNC_OP_CREATE,
                               dmi, &dbop, MAINTBL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("CreateCandidateMo failed. UpdateConfigDb failed."
                   "Record creation failed - %d",
                   result_code);
    return result_code;
  }
  UPLL_LOG_DEBUG("CreateCandidateMo Successful");
  return result_code;
}

upll_rc_t VbrIfPolicingMapMoMgr::DeleteMo(IpcReqRespHeader *req,
                                          ConfigKeyVal *ikey, DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_ERR_GENERIC;

  if (NULL == ikey || NULL == req || NULL == dmi) {
    UPLL_LOG_DEBUG("DeleteMo Failed. Insufficient input parameters");
    return result_code;
  }

  // validate syntax and semantics
  result_code = ValidateMessage(req, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("ValidateMessage Err (%d)", result_code);
    return result_code;
  }

  // Check VBRIF object existence in VbrIfPolicingMap CANDIDATE DB
  // If record not exists, return error
  result_code = IsReferenced(ikey, req->datatype, dmi);
  if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    UPLL_LOG_DEBUG("DeleteMo Instance not Available(%d)", result_code);
    return result_code;
  } else if (result_code == UPLL_RC_ERR_INSTANCE_EXISTS) {
    UPLL_LOG_DEBUG("DeleteMo Instance Available");
  } else if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("DeleteMo IsReferenced Error accessing DB (%d)",
        result_code);
    return result_code;
  }

  // 1)Get vbrif associated ctrlr name and invoke the PP and PPE functions to
  // decrement the refcount capability. If refcount is zero, remove the record
  // in policingprofilectrltbl and if refcount not zero update the refcount in
  // policingprofilectrltbl
  // 2)Delete the record in policingprofileentryctrltbl
  ConfigKeyVal *okey = NULL;
  result_code = GetChildConfigKey(okey, ikey);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetChildConfigKey failed %d", result_code);
    return result_code;
  }
  DbSubOp dbop = { kOpReadSingle, kOpMatchNone,
                   kOpInOutCtrlr | kOpInOutDomain };
  result_code = ReadConfigDB(okey, req->datatype, UNC_OP_READ, dbop, dmi,
                             MAINTBL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("ReadConfigDB failed %d", result_code);
    delete okey;
    return result_code;
  }
  val_policingmap_t *val_pm = reinterpret_cast<val_policingmap_t *>
    (GetVal(okey));
  if (UNC_VF_VALID == val_pm->valid[UPLL_IDX_POLICERNAME_PM]) {
    result_code = UpdateRefCountInPPCtrlr(okey, req->datatype, dmi,
                                          UNC_OP_DELETE);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("UpdateRefCountInPPCtrlr Error DB (%d)", result_code);
      return result_code;
    }
  }

  // Delete the record in vbrifpolicingmap table
  result_code = UpdateConfigDB(ikey, UPLL_DT_CANDIDATE, UNC_OP_DELETE, dmi,
                               MAINTBL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("DeleteMo Failed. UpdateConfigdb failed to delete - %d",
                  result_code);
    return result_code;
  }
  UPLL_LOG_DEBUG("DeleteMo Success");
  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrIfPolicingMapMoMgr::UpdateMo(IpcReqRespHeader *req,
                                          ConfigKeyVal *ikey, DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  if (NULL == ikey || NULL == req || NULL == dmi) {
    UPLL_LOG_TRACE("UpdateMo ikey and req NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  ConfigKeyVal *tmp_key = NULL;

  // validate syntax and semantics
  result_code = ValidateMessage(req, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("ValidateMessage Err (%d)", result_code);
    return result_code;
  }

  ConfigKeyVal *okey = NULL;
  result_code = GetControllerId(ikey, okey, dmi);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetControllerId failed %d", result_code);
    return result_code;
  }
  delete okey;

  // Capability Check
  result_code = ValidateCapability(req, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Key not supported by controller");
    return result_code;
  }

  // Check VBRIF object existence in VbrIfPolicingMap CANDIDATE DB
  // If record not exists, return error
  result_code = IsReferenced(ikey, req->datatype, dmi);
  if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    UPLL_LOG_TRACE("UpdateMo IsReferenced record not available (%d)",
        result_code);
    return result_code;
  } else if (result_code == UPLL_RC_ERR_INSTANCE_EXISTS) {
    UPLL_LOG_TRACE("UpdateMo IsReferenced record available");
  } else if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_TRACE("UpdateMo IsReferenced error accessing DB (%d)",
        result_code);
    return result_code;
  }

  // Check vbrif Policingmap object exists in PolicingProfileTbl CANDIDATE DB
  // If record not exists, return error code
  val_policingmap_t *val_ival = reinterpret_cast<val_policingmap_t *>
    (GetVal(ikey));
  if (UNC_VF_VALID == val_ival->valid[UPLL_IDX_POLICERNAME_PM]) {
    result_code = IsPolicyProfileReferenced(ikey, req->datatype, dmi,
                                            UNC_OP_READ);
    if (UPLL_RC_SUCCESS != result_code) {
      if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
        UPLL_LOG_DEBUG("IsPolicyProfileReferenced record not available (%d)",
                       result_code);
        return UPLL_RC_ERR_CFG_SEMANTIC;
      } else {
        UPLL_LOG_DEBUG(
            "IsPolicyProfileReferenced error accessing DB (%d)",
            result_code);
        return result_code;
      }
    }
  }

  // 1)Get vbrif associated ctrlr name and invoke the PP and PPE functions to
  // check the refcount capability and update the refcount or create the
  // record in policingprofilectrltbl and policingprofileentryctrltbl.
  ConfigKeyVal *tmpckv = NULL;
  result_code = GetChildConfigKey(tmpckv, ikey);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetChildConfigKey failed");
    return result_code;
  }
  DbSubOp dbop = { kOpReadSingle, kOpMatchNone,
                   kOpInOutCtrlr|kOpInOutDomain };
  result_code = ReadConfigDB(tmpckv, req->datatype, UNC_OP_READ, dbop, dmi,
      MAINTBL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("ReadConfigDB failed");
    return result_code;
  }
  uint8_t *ctrlr_id = NULL;
  GET_USER_DATA_CTRLR(tmpckv, ctrlr_id);
  SET_USER_DATA_CTRLR(ikey, ctrlr_id);
  val_policingmap_t *val_tmp_val = reinterpret_cast<val_policingmap_t *>
    (GetVal(tmpckv));
  if (UNC_VF_VALID == val_ival->valid[UPLL_IDX_POLICERNAME_PM] &&
      UNC_VF_VALID == val_tmp_val->valid[UPLL_IDX_POLICERNAME_PM]) {
    UPLL_LOG_DEBUG(" Policer name valid in DB and ikey");
    result_code = UpdateRefCountInPPCtrlr(tmpckv, req->datatype, dmi,
        UNC_OP_DELETE);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("UpdateMo UpdateRefCountInPPCtrlr error in delete (%d)",
          result_code);
      return result_code;
    }
    result_code = UpdateRefCountInPPCtrlr(ikey, req->datatype, dmi,
        UNC_OP_CREATE);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("UpdateMo UpdateRefCountInPPCtrlr error in create (%d)",
          result_code);
      return result_code;
    }
  } else if ((UNC_VF_VALID_NO_VALUE == val_tmp_val->
      valid[UPLL_IDX_POLICERNAME_PM] ||
      UNC_VF_INVALID == val_tmp_val->valid[UPLL_IDX_POLICERNAME_PM])&&
      UNC_VF_VALID == val_ival->valid[UPLL_IDX_POLICERNAME_PM]) {
    UPLL_LOG_DEBUG(" Policer name valid in ikey and validnovalue in DB");
    result_code = UpdateRefCountInPPCtrlr(ikey, req->datatype, dmi,
        UNC_OP_CREATE);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("UpdateMo UpdateRefCountInPPCtrlr error in create (%d)",
          result_code);
      return result_code;
    }
  } else if (UNC_VF_VALID == val_tmp_val->valid[UPLL_IDX_POLICERNAME_PM] &&
      UNC_VF_VALID_NO_VALUE == val_ival->
      valid[UPLL_IDX_POLICERNAME_PM]) {
    UPLL_LOG_DEBUG(" Policer name validnovalue in ikey and valid in db");
    result_code = UpdateRefCountInPPCtrlr(tmpckv, req->datatype, dmi,
        UNC_OP_DELETE);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("UpdateMo UpdateRefCountInPPCtrlr error in create (%d)",
          result_code);
      return result_code;
    }
  }

  // Update the record in CANDIDATE DB
  result_code = UpdateConfigDB(ikey, req->datatype, req->operation, dmi,
                               MAINTBL);
  UPLL_LOG_TRACE("VtnPolicingMapMoMgr::UpdateMo update record status (%d)",
                result_code);

  CONFIGKEYVALCLEAN(tmp_key);
  return result_code;
}

upll_rc_t VbrIfPolicingMapMoMgr::IsReferenced(ConfigKeyVal *ikey,
                                              upll_keytype_datatype_t dt_type,
                                              DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  if (NULL == ikey) return UPLL_RC_ERR_GENERIC;
  DbSubOp dbop = { kOpReadExist, kOpMatchNone, kOpInOutNone };

  // Check the VBRIF PM object existence in vbrifpolicingmap
  result_code = UpdateConfigDB(ikey, dt_type, UNC_OP_READ, dmi, &dbop, MAINTBL);
  UPLL_LOG_TRACE("IsReferenced (%d)", result_code);
  return result_code;
}

upll_rc_t VbrIfPolicingMapMoMgr::ValidateAttribute(ConfigKeyVal *ikey,
                                                   DalDmlIntf *dmi,
                                                   IpcReqRespHeader *req) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  ConfigKeyVal *okey = NULL;
  if (!ikey || !ikey->get_key()) {
    UPLL_LOG_DEBUG("input key is null");
    return UPLL_RC_ERR_GENERIC;
  }
  key_vbr_if_t *key_vbrif_pm =
      reinterpret_cast<key_vbr_if_t *>(ikey->get_key());

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
        key_vbrif_pm->vbr_key.vtn_key.vtn_name,
        kMaxLenVtnName + 1);

    uuu::upll_strncpy(vbrif_key->vbr_key.vbridge_name,
        key_vbrif_pm->vbr_key.vbridge_name,
        kMaxLenVtnName + 1);

    uuu::upll_strncpy(vbrif_key->if_name,
        key_vbrif_pm->if_name,
        kMaxLenInterfaceName + 1);

  /* Checks the given vbr_if exists or not */
  result_code = mgr->UpdateConfigDB(okey, req->datatype, UNC_OP_READ,
                                    dmi, MAINTBL);
  if (UPLL_RC_ERR_INSTANCE_EXISTS != result_code) {
    UPLL_LOG_DEBUG(" Parent VBRIF key does not exists");
    result_code = UPLL_RC_ERR_PARENT_DOES_NOT_EXIST;
  } else {
    result_code = UPLL_RC_SUCCESS;
  }
  delete okey;
  UPLL_LOG_DEBUG("ValidateAttribute Successfull.");
  return result_code;
}

upll_rc_t VbrIfPolicingMapMoMgr::IsPolicyProfileReferenced(
    ConfigKeyVal *ikey, upll_keytype_datatype_t dt_type, DalDmlIntf* dmi,
    unc_keytype_operation_t op) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *okey = NULL;

  // Get the memory allocated policy profile key structure
  MoMgrImpl *mgr =
      static_cast<MoMgrImpl *>((const_cast<MoManager*>(GetMoManager(
          UNC_KT_POLICING_PROFILE))));
  if (!mgr) {
    UPLL_LOG_TRACE("IsPolicyProfileReferenced pp obj failure (%d)",
                  result_code);
    return UPLL_RC_ERR_GENERIC;
  }

  result_code = mgr->GetChildConfigKey(okey, NULL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_TRACE("IsPolicyProfileReferenced GetChildConfigKey (%d)",
                  result_code);
    return result_code;
  }
  if (!okey) return UPLL_RC_ERR_GENERIC;

  // Copy the policer name from input and assign it in policingprofile
  // key structure
  uuu::upll_strncpy(
      reinterpret_cast<key_policingprofile_t *>
      (okey->get_key())->policingprofile_name,
      reinterpret_cast<val_policingmap_t*>
      (ikey->get_cfg_val()->get_val())->policer_name,
      (kMaxLenPolicingProfileName + 1));

  DbSubOp dbop = { kOpReadExist, kOpMatchNone, kOpInOutNone };

  // Check the policingprofile object in policingprofiletbl
  result_code = mgr->UpdateConfigDB(okey, dt_type, op, dmi, &dbop, MAINTBL);
  UPLL_LOG_TRACE("IsPolicyProfileReferenced UpdateConfigDB status (%d)",
                result_code);
  if (UPLL_RC_ERR_INSTANCE_EXISTS != result_code) {
    result_code = UPLL_RC_SUCCESS;
  }
  val_policingmap_t *val_pm = reinterpret_cast<val_policingmap_t*>
      (GetVal(ikey));
  PolicingProfileEntryMoMgr *ppe_mgr =
    reinterpret_cast<PolicingProfileEntryMoMgr*>
    (const_cast<MoManager *>(GetMoManager
                             (UNC_KT_POLICING_PROFILE_ENTRY)));
  result_code = ppe_mgr->ValidateValidElements(reinterpret_cast
    <const char *>(val_pm->policer_name), dmi, UPLL_DT_CANDIDATE);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("ValidateValidElements failed %d", result_code);
    delete okey;
    return result_code;
  }
  CONFIGKEYVALCLEAN(okey);
  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrIfPolicingMapMoMgr::UpdateRefCountInPPCtrlr(
    ConfigKeyVal *ikey, upll_keytype_datatype_t dt_type, DalDmlIntf *dmi,
    unc_keytype_operation_t op) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *okey = NULL;
  uint8_t *ctrlr_id = NULL;

  // Get the contorller name from VBR MAINTBL
  GET_USER_DATA_CTRLR(ikey, ctrlr_id);

  PolicingProfileMoMgr *pp_mgr =
      reinterpret_cast<PolicingProfileMoMgr *>
      (const_cast<MoManager *>(GetMoManager(
          UNC_KT_POLICING_PROFILE)));
  if (!pp_mgr) {
    UPLL_LOG_TRACE("pp_mgr obj failure (%d)", result_code);
    CONFIGKEYVALCLEAN(okey);
    return result_code;
  }
  #if 0
  PolicingProfileEntryMoMgr *ppe_mgr =
      reinterpret_cast<PolicingProfileEntryMoMgr *>
      (const_cast<MoManager *>(GetMoManager(
          UNC_KT_POLICING_PROFILE_ENTRY)));
  if (!ppe_mgr) {
    UPLL_LOG_TRACE("ppe_mgr obj failure (%d)", result_code);
    CONFIGKEYVALCLEAN(okey);
    return result_code;
  }
  #endif

  val_policingmap_t* val_vtn_policingmap =
      reinterpret_cast<val_policingmap_t *>(GetVal(ikey));

  if (NULL == ctrlr_id) {
    UPLL_LOG_DEBUG("UpdateRefCountInPPCtrlr ctrlr_id NULL (%d)",
        result_code);
    CONFIGKEYVALCLEAN(okey);
    return UPLL_RC_ERR_GENERIC;
  }

  /* Check the ref count capability of vbrif associated controller name in
  policingprofilectrltbl If refcount reached the
  max capability, then return error.
  If ref count is less than capability and the record exist in
  policingprofilectrltbl, based on the operation refcount should be
  incremented or decremented and update policingprofilectrltbl.
  create the record with refcount if record not
  exist in policingprofilectrl tbl for the vbr associated controller name
  */
  result_code = pp_mgr->PolicingProfileCtrlrTblOper(
      reinterpret_cast<const char*>(val_vtn_policingmap->policer_name),
      reinterpret_cast<const char*>(ctrlr_id), dmi, op, dt_type);

  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("PolicingProfileCtrlrTblOper err (%d)", result_code);
    CONFIGKEYVALCLEAN(okey);
    return result_code;
  }

  // Create/Delete/Update the record in policingprofileentryctrltbl
  // based on oper
  UPLL_LOG_DEBUG("PPECtrl  profile name(%s)  ctrlr id(%s)",
                 val_vtn_policingmap->policer_name, ctrlr_id);
  #if 0
  result_code = ppe_mgr->PolicingProfileEntryCtrlrTblOper(
      reinterpret_cast<const char*>(val_vtn_policingmap->policer_name),
      reinterpret_cast<const char*>(ctrlr_id), dmi, op, dt_type);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("PolicingProfileEntryCtrlrTblOper err (%d)", result_code);
    CONFIGKEYVALCLEAN(okey);
    return result_code;
  }
  #endif
  CONFIGKEYVALCLEAN(okey);
  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrIfPolicingMapMoMgr::GetControllerId(ConfigKeyVal *ikey,
                                                 ConfigKeyVal *&okey,
                                                 DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  VbrMoMgr *mgr =
    reinterpret_cast<VbrMoMgr *>(const_cast<MoManager *>(GetMoManager(
            UNC_KT_VBRIDGE)));
  if (!mgr) {
    UPLL_LOG_TRACE("mgr obj failure (%d)", result_code);
    return UPLL_RC_ERR_GENERIC;
  }

  result_code = mgr->GetChildConfigKey(okey, NULL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_TRACE("GetChildConfigKey failure (%d)", result_code);
    return result_code;
  }
  key_vbr_t *vbr_key = reinterpret_cast
      <key_vbr_t *>(okey->get_key());
  key_vbr_if_t *vbr_if_ikey =  reinterpret_cast
      <key_vbr_if_t *>(ikey->get_key());
  uuu::upll_strncpy(
    vbr_key->vtn_key.vtn_name,
    vbr_if_ikey->vbr_key.vtn_key.vtn_name,
    (kMaxLenVtnName + 1));
  uuu::upll_strncpy(
    vbr_key->vbridge_name,
    vbr_if_ikey->vbr_key.vbridge_name,
    (kMaxLenVnodeName + 1));
  DbSubOp dbop = { kOpReadSingle, kOpMatchNone,
                   kOpInOutCtrlr|kOpInOutDomain };
  result_code = mgr->ReadConfigDB(okey, UPLL_DT_CANDIDATE, UNC_OP_READ, dbop,
                                  dmi, MAINTBL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetControllerId ReadConfigDB failed (%d)", result_code);
    CONFIGKEYVALCLEAN(okey);
    return result_code;
  }

  controller_domain ctrlr_dom;
  ctrlr_dom.ctrlr = NULL;
  ctrlr_dom.domain = NULL;
  result_code = mgr->GetControllerDomainId(okey, &ctrlr_dom);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetControllerDomainId failed %d", result_code);
    return result_code;
  }
  SET_USER_DATA_CTRLR_DOMAIN(ikey, ctrlr_dom);

  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrIfPolicingMapMoMgr::ReadMo(IpcReqRespHeader *req,
                                        ConfigKeyVal *ikey, DalDmlIntf *dmi) {
  /* TODO */
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *dup_key = NULL;
  controller_domain ctrlr_dom;

  result_code = ValidateMessage(req, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("ReadMo ValidateMessage Err  (%d)", result_code);
    return result_code;
  }
  memset(&ctrlr_dom, 0, sizeof(controller_domain));
  if (UNC_KT_VBRIF_POLICINGMAP == ikey->get_key_type()) {
    switch (req->datatype) {
      case UPLL_DT_CANDIDATE:
      case UPLL_DT_RUNNING:
      case UPLL_DT_STARTUP:
      case UPLL_DT_STATE:
        if (req->option1 == UNC_OPT1_NORMAL) {
          result_code = ReadInfoFromDB(req, ikey, dmi, &ctrlr_dom);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG(" Read request failed result(%d)", result_code);
          }
        } else if ((UPLL_DT_STATE == req->datatype) &&
            (req->option1 == UNC_OPT1_DETAIL) &&
            (req->option2 == UNC_OPT2_NONE)) {

          result_code = ReadDetailRecord(req, ikey, dmi);
          UPLL_LOG_DEBUG("ReadDetailRecord result_code (%d)", result_code);
        }
        return result_code;
        break;
      default:
        UPLL_LOG_DEBUG("Invalid state/option");
        return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT;
        break;
    }
  } else if (UNC_KT_VBRIF_POLICINGMAP_ENTRY == ikey->get_key_type()) {
    switch (req->datatype) {
      case UPLL_DT_STATE:
        if ((req->option1 == UNC_OPT1_NORMAL) &&
            (req->option2 == UNC_OPT2_NONE)) {
          result_code = ReadDTStateNormal(req, ikey, dmi);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("ReadDTStateNormal failed");
            CONFIGKEYVALCLEAN(dup_key);
            return result_code;
          }
        } else if ((req->option1 == UNC_OPT1_DETAIL) &&
            (req->option2 == UNC_OPT2_NONE)) {
          UPLL_LOG_DEBUG("ReadRecord ReadConfigDB error (%d) (%d)",
              req->datatype, req->option1);
          result_code = ReadEntryDetailRecord(req, ikey, dmi);
          if (UPLL_RC_SUCCESS != result_code) {
            UPLL_LOG_DEBUG("ReadEntryDetailRecord failed %d", result_code);
            return result_code;
          }
        } else {  // Till here
          UPLL_LOG_DEBUG("ReadRecord ReadConfigDB error (%d) (%d)",
              req->datatype, req->option1);
          return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT;
        }
        break;
      default:
        UPLL_LOG_DEBUG(
            "VtnPolicingMapMoMgr::ReadRecord ReadConfigDB error (%d) (%d)",
            req->datatype, req->option1);
        return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT;
        break;
    }
  } else {
    return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT;
  }
  return result_code;
}

upll_rc_t VbrIfPolicingMapMoMgr::GetReadVbrIfKey(ConfigKeyVal *&dup_key,
                                                 ConfigKeyVal *ikey) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  result_code =  GetChildConfigKey(dup_key, NULL);
  if (result_code != UPLL_RC_SUCCESS) {
     UPLL_LOG_DEBUG("GetChildConfigKey Error");
     return result_code;
  }
  if (!dup_key) {
     UPLL_LOG_DEBUG("dup_key NULL");
     return UPLL_RC_ERR_GENERIC;
  }
  key_vbr_if_t *key = reinterpret_cast<key_vbr_if_t *>(dup_key->get_key());
  key_vbrif_policingmap_entry *key1 =
    reinterpret_cast<key_vbrif_policingmap_entry *>(ikey->get_key());

  uuu::upll_strncpy(reinterpret_cast<char*>(key->vbr_key.vtn_key.vtn_name),
          reinterpret_cast<char*>(key1->vbrif_key.vbr_key.vtn_key.vtn_name),
          (kMaxLenVtnName + 1));
  uuu::upll_strncpy(reinterpret_cast<char*>(key->vbr_key.vbridge_name),
          reinterpret_cast<char*>(key1->vbrif_key.vbr_key.vbridge_name),
          (kMaxLenVnodeName + 1));
  uuu::upll_strncpy(reinterpret_cast<char*>(key->if_name),
          reinterpret_cast<char*>(key1->vbrif_key.if_name),
          (kMaxLenVnodeName + 1));

  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrIfPolicingMapMoMgr::GetChildEntryConfigKey(ConfigKeyVal *&okey,
                                                 ConfigKeyVal *ikey) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_ERR_GENERIC;
  if (NULL == ikey || NULL != okey) {
    UPLL_LOG_DEBUG("GetChildEntryConfigKey failed");
    return result_code;
  }

  key_vbrif_policingmap_entry *key =
      reinterpret_cast<key_vbrif_policingmap_entry *>
      (ConfigKeyVal::Malloc(sizeof(key_vbrif_policingmap_entry)));
  key_vbrif_policingmap_entry *key1 =
    reinterpret_cast<key_vbrif_policingmap_entry *>(ikey->get_key());

  uuu::upll_strncpy(
      reinterpret_cast<char*>(key->vbrif_key.vbr_key.vtn_key.vtn_name),
      reinterpret_cast<char*>(key1->vbrif_key.vbr_key.vtn_key.vtn_name),
      (kMaxLenVtnName + 1));
  uuu::upll_strncpy(
      reinterpret_cast<char*>(key->vbrif_key.vbr_key.vbridge_name),
      reinterpret_cast<char*>(key1->vbrif_key.vbr_key.vbridge_name),
      (kMaxLenVnodeName + 1));
  uuu::upll_strncpy(reinterpret_cast<char*>(key->vbrif_key.if_name),
          reinterpret_cast<char*>(key1->vbrif_key.if_name),
          (kMaxLenVnodeName + 1));
  key->sequence_num = key1->sequence_num;
  okey = new ConfigKeyVal(UNC_KT_VBRIF_POLICINGMAP_ENTRY,
         IpctSt::kIpcStKeyVbrifPolicingmapEntry, key, NULL);

  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrIfPolicingMapMoMgr::ConstructVbrIfPolicingmapEntryCkv(
    ConfigKeyVal *&okey, ConfigKeyVal *ikey, uint8_t sequence_num) {
  UPLL_FUNC_TRACE;
  if (NULL == ikey) {
    UPLL_LOG_DEBUG("ikey is null");
    return UPLL_RC_ERR_GENERIC;
  }
  key_vbr_if_t *vbr_if_key = reinterpret_cast<key_vbr_if_t *>
      (ikey->get_key());
  key_vbrif_policingmap_entry_t *vbrif_pm_key =
      reinterpret_cast<key_vbrif_policingmap_entry_t *>
      (ConfigKeyVal::Malloc(sizeof(key_vbrif_policingmap_entry_t)));
  uuu::upll_strncpy(
      reinterpret_cast<char*>(vbrif_pm_key->vbrif_key.vbr_key.vtn_key.vtn_name),
      reinterpret_cast<char*>(vbr_if_key->vbr_key.vtn_key.vtn_name),
      (kMaxLenVtnName + 1));
  uuu::upll_strncpy(
      reinterpret_cast<char*>(vbrif_pm_key->vbrif_key.vbr_key.vbridge_name),
      reinterpret_cast<char*>(vbr_if_key->vbr_key.vbridge_name),
      (kMaxLenVnodeName + 1));
  uuu::upll_strncpy(reinterpret_cast<char*>(vbrif_pm_key->vbrif_key.if_name),
          reinterpret_cast<char*>(vbr_if_key->if_name),
          (kMaxLenVnodeName + 1));
  vbrif_pm_key->sequence_num = sequence_num;
  okey = new ConfigKeyVal(UNC_KT_VBRIF_POLICINGMAP_ENTRY,
         IpctSt::kIpcStKeyVbrifPolicingmapEntry, vbrif_pm_key, NULL);
  if (NULL == okey) {
    return UPLL_RC_ERR_GENERIC;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrIfPolicingMapMoMgr::ConstructPpeCkv(ConfigKeyVal *&okey,
    const char *ppe_name, uint8_t sequence_num) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_ERR_GENERIC;
  PolicingProfileEntryMoMgr *mgr = reinterpret_cast
        <PolicingProfileEntryMoMgr*>
        (const_cast<MoManager *>(GetMoManager
                                 (UNC_KT_POLICING_PROFILE_ENTRY)));
  result_code = mgr->GetChildConfigKey(okey, NULL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetChildConfigKey failed");
  }
  key_policingprofile_entry_t *key_ppe = reinterpret_cast
    <key_policingprofile_entry_t *>(okey->get_key());
  uuu::upll_strncpy(key_ppe->policingprofile_key.policingprofile_name,
      ppe_name, kMaxLenPolicingProfileName + 1);
  key_ppe->sequence_num = sequence_num;
  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrIfPolicingMapMoMgr::ReadSiblingMo(IpcReqRespHeader *req,
    ConfigKeyVal *ikey, bool begin,
    DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  controller_domain ctrlr_dom;
  result_code = ValidateMessage(req, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("ReadSiblingMo ValidateMessage Err  (%d)", result_code);
    return result_code;
  }

  memset(&ctrlr_dom, 0, sizeof(controller_domain));
  if ((UNC_KT_VBRIF_POLICINGMAP == ikey->get_key_type()) &&
      UNC_OP_READ_SIBLING == req->operation ) {
    /* Since read sibling  not applicable for policingmap*/
    UPLL_LOG_DEBUG("ReadSibling not allowed ");
    return UPLL_RC_ERR_NO_SUCH_INSTANCE;
  }

  if (UNC_KT_VBRIF_POLICINGMAP == ikey->get_key_type()) {
    switch (req->datatype) {
      case UPLL_DT_CANDIDATE:
      case UPLL_DT_RUNNING:
      case UPLL_DT_STARTUP:
      case UPLL_DT_STATE:
        if (req->option1 == UNC_OPT1_NORMAL) {
          result_code = ReadInfoFromDB(req, ikey, dmi, &ctrlr_dom);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("Read sibling request failed (%d)", result_code);
            return result_code;
          }
          UPLL_LOG_DEBUG("Read sibling request success (%d)", result_code);
          return result_code;
        } else if ((req->datatype == UPLL_DT_STATE) &&
            (req->option1 == UNC_OPT1_DETAIL)&&
            (req->option2 == UNC_OPT2_NONE)) {
          result_code = ReadDetailRecord(req, ikey, dmi);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("ReadDetailRecord result_code (%d)", result_code);
          }
        }
        return result_code;
        break;
      default:
        UPLL_LOG_DEBUG("Not Allowed for this DT (%d)", result_code);
        return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT;
        break;
    }
  } else if (UNC_KT_VBRIF_POLICINGMAP_ENTRY == ikey->get_key_type()) {
    switch (req->datatype) {
      case UPLL_DT_STATE:
        if (req->option1 == UNC_OPT1_NORMAL) {
          ConfigKeyVal *temp_vbr_if_key = NULL;
          key_vbrif_policingmap_entry_t *vbrif_entry_key = reinterpret_cast
              <key_vbrif_policingmap_entry_t *>(ikey->get_key());
          result_code = GetReadVbrIfKey(temp_vbr_if_key, ikey);
          DbSubOp dbop = {kOpReadSingle, kOpMatchNone, kOpInOutNone};
          result_code = ReadConfigDB(temp_vbr_if_key, req->datatype,
                                     UNC_OP_READ, dbop, dmi, MAINTBL);
          if (UPLL_RC_SUCCESS != result_code) {
            UPLL_LOG_DEBUG("ReadConfigDB failed %d", result_code);
            return result_code;
          }
          val_policingmap_t *val_pm = reinterpret_cast
              <val_policingmap_t *>(GetVal(temp_vbr_if_key));
          ConfigKeyVal *ppe_ckv = NULL;
          result_code = ConstructPpeCkv(ppe_ckv, reinterpret_cast
                                        <const char *>(val_pm->policer_name),
                                        vbrif_entry_key->sequence_num);
          PolicingProfileEntryMoMgr *mgr = reinterpret_cast
              <PolicingProfileEntryMoMgr*>
              (const_cast<MoManager *>(GetMoManager
                                       (UNC_KT_POLICING_PROFILE_ENTRY)));
          if (!mgr) {
            return UPLL_RC_ERR_GENERIC;
          }
          IpcReqRespHeader *temp_req = reinterpret_cast<IpcReqRespHeader *>
              (ConfigKeyVal::Malloc(sizeof(IpcReqRespHeader)));
          memcpy(temp_req, req, sizeof(IpcReqRespHeader));
          temp_req->option1 = UNC_OPT1_NORMAL;
          // result_code = mgr->ReadInfoFromDB(req, ppe_ckv, dmi, &ctrlr_dom);
          result_code = mgr->ReadSiblingMo(temp_req, ppe_ckv, dmi);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("Read sibling of ppe failed (%d)", result_code);
            return result_code;
          }
          ConfigKeyVal *temp_ppe_ckv = ppe_ckv;
          ConfigKeyVal *okey = NULL;
          while (NULL != temp_ppe_ckv) {
            ConfigKeyVal *temp_vbrif_pm_ckv = NULL;
            key_policingprofile_entry_t *temp_ppe_key = reinterpret_cast
                <key_policingprofile_entry_t *>(temp_ppe_ckv->get_key());
            result_code = GetChildEntryConfigKey(temp_vbrif_pm_ckv, ikey);
            if (UPLL_RC_SUCCESS != result_code) {
              UPLL_LOG_DEBUG("GetChildEntryConfigKey failed");
              return result_code;
            }
            key_vbrif_policingmap_entry_t *temp_vbrif_pm_key = reinterpret_cast
                <key_vbrif_policingmap_entry_t *>(temp_vbrif_pm_ckv->get_key());
            temp_vbrif_pm_key->sequence_num = temp_ppe_key->sequence_num;
            result_code = ReadDTStateNormal(req, temp_vbrif_pm_ckv, dmi);
            if (result_code != UPLL_RC_SUCCESS) {
              UPLL_LOG_DEBUG("ReadDTStateNormal failed");
              return result_code;
            }
            UPLL_LOG_DEBUG("vbrifpmentry sequence_num - %d", result_code);
            temp_ppe_ckv = temp_ppe_ckv->get_next_cfg_key_val();
            if (NULL == okey) {
              okey = temp_vbrif_pm_ckv;
            } else {
              okey->AppendCfgKeyVal(temp_vbrif_pm_ckv);
            }
          }
          if ((okey != NULL) && (result_code == UPLL_RC_SUCCESS)) {
            ikey->ResetWith(okey);
          }
        } else if ((req->option1 == UNC_OPT1_DETAIL) &&
            (req->option2 == UNC_OPT2_NONE)) {
          UPLL_LOG_DEBUG("ReadRecord ReadConfigDB error (%d) (%d)",
              req->datatype, req->option1);
          ConfigKeyVal *temp_vbr_if_key = NULL;
          key_vbrif_policingmap_entry_t *vbrif_entry_key = reinterpret_cast
            <key_vbrif_policingmap_entry_t *>(ikey->get_key());
          result_code = GetReadVbrIfKey(temp_vbr_if_key, ikey);
          DbSubOp dbop = {kOpReadSingle, kOpMatchNone, kOpInOutNone};
          result_code = ReadConfigDB(temp_vbr_if_key, req->datatype,
                                     UNC_OP_READ, dbop, dmi, MAINTBL);
          if (UPLL_RC_SUCCESS != result_code) {
            UPLL_LOG_DEBUG("ReadConfigDB failed %d", result_code);
            return result_code;
          }
          val_policingmap_t *val_pm = reinterpret_cast
            <val_policingmap_t *>(GetVal(temp_vbr_if_key));
          ConfigKeyVal *ppe_ckv = NULL;
          result_code = ConstructPpeCkv(ppe_ckv, reinterpret_cast
              <const char *>(val_pm->policer_name),
              vbrif_entry_key->sequence_num);
          PolicingProfileEntryMoMgr *mgr = reinterpret_cast
            <PolicingProfileEntryMoMgr*>
            (const_cast<MoManager *>(GetMoManager
                                     (UNC_KT_POLICING_PROFILE_ENTRY)));
          if (!mgr) {
            return UPLL_RC_ERR_GENERIC;
          }
          IpcReqRespHeader *temp_req = reinterpret_cast<IpcReqRespHeader *>
            (ConfigKeyVal::Malloc(sizeof(IpcReqRespHeader)));
          memcpy(temp_req, req, sizeof(IpcReqRespHeader));
          temp_req->option1 = UNC_OPT1_NORMAL;
          // result_code = mgr->ReadInfoFromDB(req, ppe_ckv, dmi, &ctrlr_dom);
          result_code = mgr->ReadSiblingMo(temp_req, ppe_ckv, dmi);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("Read sibling of ppe failed (%d)", result_code);
            return result_code;
          }
          ConfigKeyVal *temp_ppe_ckv = ppe_ckv;
          ConfigKeyVal *okey = NULL;
          while (NULL != temp_ppe_ckv) {
            ConfigKeyVal *temp_vbrif_pm_ckv = NULL;
            key_policingprofile_entry_t *temp_ppe_key = reinterpret_cast
              <key_policingprofile_entry_t *>(temp_ppe_ckv->get_key());
            result_code = GetChildEntryConfigKey(temp_vbrif_pm_ckv, ikey);
            if (UPLL_RC_SUCCESS != result_code) {
              UPLL_LOG_DEBUG("GetChildEntryConfigKey failed");
              return result_code;
            }
            key_vbrif_policingmap_entry_t *temp_vbrif_pm_key = reinterpret_cast
              <key_vbrif_policingmap_entry_t *>(temp_vbrif_pm_ckv->get_key());
            temp_vbrif_pm_key->sequence_num = temp_ppe_key->sequence_num;
            UPLL_LOG_DEBUG("vbrifpmentry sequence_num - %d", result_code);
            result_code = ReadEntryDetailRecord(req, temp_vbrif_pm_ckv, dmi);
            if (UPLL_RC_SUCCESS != result_code) {
              UPLL_LOG_DEBUG("ReadEntryDetailRecord failed %d", result_code);
              return result_code;
            }
            temp_ppe_ckv = temp_ppe_ckv->get_next_cfg_key_val();
            if (NULL == okey) {
              okey = temp_vbrif_pm_ckv;
            } else {
              okey->AppendCfgKeyVal(temp_vbrif_pm_ckv);
            }
          }
          if ((okey != NULL) && (result_code == UPLL_RC_SUCCESS)) {
            ikey->ResetWith(okey);
          }
        } else {
          result_code = UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT;
        }
        break;
      default:
        result_code = UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT;
        break;
    }
  } else {
    result_code =  UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_KT;
  }
  return result_code;
}

upll_rc_t VbrIfPolicingMapMoMgr::ReadDetailRecord(IpcReqRespHeader *req,
                                               ConfigKeyVal *&ikey,
                                               DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  ConfigKeyVal *l_key = NULL;
  ConfigKeyVal *dup_key = NULL;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  controller_domain ctrlr_dom;
  uint8_t db_flag = 0;
  DbSubOp dbop = { kOpReadSingle, kOpMatchNone,
    kOpInOutCtrlr|kOpInOutDomain|kOpInOutFlag };
  // On Thisdup key we r applying the Readsibling op, that gives dir
  memset(&ctrlr_dom, 0, sizeof(controller_domain));
  result_code =  DupConfigKeyVal(dup_key, ikey, MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("DupConfigKeyVal dup_key Err  (%d)", result_code);
    return result_code;
  }
  result_code = ReadConfigDB(dup_key, req->datatype, UNC_OP_READ,
                             dbop, dmi, MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("ReadConfigDB Error  (%d)", result_code);
    CONFIGKEYVALCLEAN(dup_key);
    return result_code;
  }

  result_code =  DupConfigKeyVal(l_key, ikey, MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("DupConfigKeyVal Faill in ReadSiblingMo for l_key");
    CONFIGKEYVALCLEAN(dup_key);
    return result_code;
  }
  GET_USER_DATA_CTRLR_DOMAIN(dup_key, ctrlr_dom);
  SET_USER_DATA_CTRLR_DOMAIN(l_key, ctrlr_dom);
  GET_USER_DATA_FLAGS(dup_key, db_flag);

  //  GET_USER_DATA_FLAGS(dup_key, db_flag);
  UPLL_LOG_DEBUG("db_flag ::: (%d)", db_flag);
  result_code = GetRenamedControllerKey(l_key, req->datatype,
                                        dmi, &ctrlr_dom);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetRenamedControllerKey Faill");
    CONFIGKEYVALCLEAN(dup_key);
    CONFIGKEYVALCLEAN(l_key);
    return result_code;
  }
  pfcdrv_val_vbrif_policingmap *pfc_val =
      reinterpret_cast<pfcdrv_val_vbrif_policingmap *>
      (ConfigKeyVal::Malloc(sizeof(pfcdrv_val_vbrif_policingmap_t)));
  pfcdrv_val_vbrif_vextif *pfc_val_ext =
      reinterpret_cast<pfcdrv_val_vbrif_vextif *>
      (ConfigKeyVal::Malloc(sizeof(pfcdrv_val_vbrif_vextif_t)));

  UPLL_LOG_DEBUG("GetVexternalInformation (%d)", req->datatype);
  if (req->datatype == UPLL_DT_STATE) req->datatype = UPLL_DT_RUNNING;
  string s(l_key->ToStrAll());
  result_code = GetVexternalInformation(dup_key, req->datatype, pfc_val,
                                        pfc_val_ext, db_flag, dmi);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetVexternalInformation fail");
    CONFIGKEYVALCLEAN(dup_key);
    CONFIGKEYVALCLEAN(l_key);
    return result_code;
  }

  val_policingmap_t* val = reinterpret_cast<val_policingmap_t *>
      (GetVal(dup_key));
  UPLL_LOG_DEBUG("val_policingmap_t (%s)", val->policer_name);

  pfc_val->valid[PFCDRV_IDX_VAL_POLICINGMAP_PM] = UNC_VF_VALID;
  memcpy(&pfc_val->val_policing_map, val, sizeof(val_policingmap_t));

  pfc_val->valid[PFCDRV_IDX_VAL_VBRIF_VEXTIF_PM] = UNC_VF_VALID;
  memcpy(&pfc_val->val_vbrif_vextif, pfc_val_ext,
         sizeof(pfcdrv_val_vbrif_vextif_t));

  l_key->SetCfgVal(new ConfigVal(IpctSt::kIpcStPfcdrvValVbrifPolicingmap,
                                 pfc_val));

  IpcResponse ipc_response;
  memset(&ipc_response, 0, sizeof(IpcResponse));
  IpcRequest ipc_req;
  memset(&ipc_req, 0, sizeof(ipc_req));
  ipc_req.header.clnt_sess_id = req->clnt_sess_id;
  ipc_req.header.config_id = req->config_id;
  ipc_req.header.operation = UNC_OP_READ;
  ipc_req.header.option1 = req->option1;
  ipc_req.header.datatype = UPLL_DT_STATE;
  ipc_req.ckv_data = l_key;
  if (!uui::IpcUtil::SendReqToDriver(
          (const char *)ctrlr_dom.ctrlr,
          reinterpret_cast<char *>(ctrlr_dom.domain),
          PFCDRIVER_SERVICE_NAME, PFCDRIVER_SVID_LOGICAL, &ipc_req,
          true, &ipc_response)) {
    UPLL_LOG_DEBUG("SendReqToDriver failed for Key %d controller %s",
                   l_key->get_key_type(),
                   reinterpret_cast<char *>(ctrlr_dom.ctrlr));
    CONFIGKEYVALCLEAN(l_key);
    CONFIGKEYVALCLEAN(dup_key);
    return UPLL_RC_ERR_GENERIC;
  }

  if (ipc_response.header.result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Driver response for Key %d controller %s result %d",
                   l_key->get_key_type(), ctrlr_dom.ctrlr,
                   ipc_response.header.result_code);
    CONFIGKEYVALCLEAN(l_key);
    CONFIGKEYVALCLEAN(dup_key);
    return ipc_response.header.result_code;
  }
  ConfigKeyVal *okey = NULL;
  result_code = ConstructReadDetailResponse(dup_key, ipc_response.ckv_data,
                                            req->datatype, req->operation,
                                            dbop, dmi, &okey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("ReadSiblingDetail Error  (%d)", result_code);
    CONFIGKEYVALCLEAN(dup_key);
    CONFIGKEYVALCLEAN(l_key);
    return result_code;
  } else {
    if (okey != NULL) {
      ikey->ResetWith(okey);
      req->rep_count = 1;
    }
  }
  return result_code;
}

// Rename
bool VbrIfPolicingMapMoMgr::GetRenameKeyBindInfo(unc_key_type_t key_type,
                                                 BindInfo *&binfo, int &nattr,
                                                 MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  UPLL_LOG_DEBUG("GetRenameKeyBindInfo (%d) (%d)", key_type, tbl);
  switch (key_type) {
    case UNC_KT_VBRIF_POLICINGMAP:
      nattr = VBRIF_KEY_COL;
      binfo = key_vbrifpm_maintbl_rename_bind_info;
    break;
    case UNC_KT_POLICING_PROFILE:
      nattr = POLICY_KEY_COL;
      binfo = key_vbrifpm_policyname_maintbl_rename_bind_info;
    break;
    default:
      UPLL_LOG_DEBUG("GetRenameKeyBindInfo Invalid key type (%d) (%d)",
                     key_type, tbl);
      return PFC_FALSE;
  }
  return PFC_TRUE;
}

// Rename
upll_rc_t VbrIfPolicingMapMoMgr::CopyToConfigKey(ConfigKeyVal *&okey,
    ConfigKeyVal *ikey) {
  UPLL_FUNC_TRACE;
  if (!ikey || !(ikey->get_key())) {
    UPLL_LOG_DEBUG("CopyToConfigKey ikey NULL");
    return UPLL_RC_ERR_GENERIC;
  }

  upll_rc_t result_code = UPLL_RC_SUCCESS;

  if (UNC_KT_VBRIF_POLICINGMAP == ikey->get_key_type()) {
    key_rename_vnode_info *key_rename =
    reinterpret_cast<key_rename_vnode_info *>(ikey->get_key());
    key_vbr_if_t *key_vbr_if = reinterpret_cast<key_vbr_if_t *>
                               (ConfigKeyVal::Malloc(sizeof(key_vbr_if_t)));
    if (!strlen(reinterpret_cast<char *>(key_rename->old_unc_vtn_name))) {
      UPLL_LOG_DEBUG("old_unc_vtn_name NULL");
      free(key_vbr_if);
      return UPLL_RC_ERR_GENERIC;
    }

    uuu::upll_strncpy(key_vbr_if->vbr_key.vtn_key.vtn_name,
                      key_rename->old_unc_vtn_name,
                      (kMaxLenVtnName+1));

    if (!strlen(reinterpret_cast<char *>(key_rename->old_unc_vnode_name))) {
      UPLL_LOG_DEBUG("old_unc_vnode_name NULL");
      free(key_vbr_if);
      return UPLL_RC_ERR_GENERIC;
    }
    uuu::upll_strncpy(key_vbr_if->vbr_key.vbridge_name,
                      key_rename->old_unc_vnode_name,
                      (kMaxLenVnodeName+1));

    okey = new ConfigKeyVal(UNC_KT_VBRIF_POLICINGMAP, IpctSt::kIpcStKeyVbrIf,
        key_vbr_if, NULL);
    if (!okey) {
      UPLL_LOG_DEBUG("okey NULL");
      free(key_vbr_if);
      return UPLL_RC_ERR_GENERIC;
    }
  } else if (UNC_KT_POLICING_PROFILE == ikey->get_key_type()) {
    key_rename_vnode_info_t *key_rename =
    reinterpret_cast<key_rename_vnode_info_t *>(ikey->get_key());

    val_policingmap_t *val = reinterpret_cast<val_policingmap_t *>
                             (ConfigKeyVal::Malloc(sizeof(val_policingmap_t)));

    if (!strlen(reinterpret_cast<char *>
       (key_rename->old_policingprofile_name))) {
      UPLL_LOG_DEBUG("old_policingprofile_name NULL");
      free(val);
      return UPLL_RC_ERR_GENERIC;
    }

    uuu::upll_strncpy(val->policer_name,
                      key_rename->old_policingprofile_name,
                      (kMaxLenPolicingProfileName+1));
    ConfigVal *cval = new ConfigVal(IpctSt::kIpcStValPolicingmap, val);
    okey = new ConfigKeyVal(UNC_KT_POLICING_PROFILE,
        IpctSt::kIpcStKeyPolicingprofile, NULL, cval);
    if (!okey) {
      UPLL_LOG_DEBUG("okey NULL");
      free(val);
      return UPLL_RC_ERR_GENERIC;
    }
  } else {
    UPLL_LOG_DEBUG("CopyToConfigKey invalid key type NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  return result_code;
}

upll_rc_t VbrIfPolicingMapMoMgr::MergeValidate(unc_key_type_t keytype,
                                               const char *ctrlr_id,
                                               ConfigKeyVal *ikey,
                                               DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  DbSubOp dbop = { kOpReadMultiple, kOpMatchNone, kOpInOutNone };
  ConfigKeyVal *tkey;
  ConfigKeyVal *pp_keyval = NULL;
  if (NULL == ikey) {
    UPLL_LOG_DEBUG("MergeValidate Failed:Insufficient input parameters ");
    return UPLL_RC_ERR_GENERIC;
  }

  result_code = ReadConfigDB(ikey, UPLL_DT_IMPORT, UNC_OP_READ, dbop, dmi,
                             MAINTBL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("MergeValidate Failed:ReadConfigDB Failed%d", result_code);
    return result_code;
  }

  MoMgrImpl *mgr =
      reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>(GetMoManager(
          UNC_KT_POLICING_PROFILE)));
  if (NULL == mgr) {
    UPLL_LOG_DEBUG("mgr NULL");
    return UPLL_RC_ERR_GENERIC;
  }

  tkey = ikey;

  while (ikey != NULL) {
    result_code = mgr->GetChildConfigKey(pp_keyval, NULL);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("MergeValidate Failed:GetChildConfigKey Failed - %d",
                    result_code);
      return result_code;
    }
    if (!pp_keyval) return UPLL_RC_ERR_GENERIC;
    val_policingmap_t *policingmap_val =
        reinterpret_cast<val_policingmap_t *>(GetVal(ikey));

    key_policingprofile_t *key_policingprofie =
        reinterpret_cast<key_policingprofile_t *>(pp_keyval->get_key());

    uuu::upll_strncpy(key_policingprofie->policingprofile_name,
                      policingmap_val->policer_name,
                      (kMaxLenPolicingProfileName + 1));

    result_code = mgr->UpdateConfigDB(pp_keyval, UPLL_DT_CANDIDATE, UNC_OP_READ,
                                      dmi, MAINTBL);
    if (result_code != UPLL_RC_ERR_INSTANCE_EXISTS) {
      UPLL_LOG_DEBUG("Verification  policing profile is not configured");
      CONFIGKEYVALCLEAN(pp_keyval);
      return UPLL_RC_ERR_MERGE_CONFLICT;
    }
    CONFIGKEYVALCLEAN(pp_keyval);
    ikey = tkey->get_next_cfg_key_val();
  }
  CONFIGKEYVALCLEAN(tkey);
  UPLL_LOG_DEBUG("MergeValidate is Successful: %d", result_code);
  return result_code;
}

upll_rc_t VbrIfPolicingMapMoMgr::GetRenamedUncKey(
    ConfigKeyVal *ikey, upll_keytype_datatype_t dt_type, DalDmlIntf *dmi,
    uint8_t *ctrlr_id) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *unc_key = NULL;
  DbSubOp dbop = { kOpReadSingle, kOpMatchCtrlr, kOpInOutNone };

  if (NULL == ikey || NULL == dmi || NULL == ctrlr_id) {
    UPLL_LOG_DEBUG("GetRenamedUncKey failed. Insufficient input parameters.");
    return UPLL_RC_ERR_GENERIC;
  }

  val_rename_vnode *rename_vnode = reinterpret_cast<val_rename_vnode *>
      (ConfigKeyVal::Malloc(sizeof(val_rename_vnode)));

  key_vbr_if_t *ctrlr_key = reinterpret_cast<key_vbr_if_t *>(ikey->get_key());
  if (NULL == ctrlr_key) {
    free(rename_vnode);
    return UPLL_RC_ERR_GENERIC;
  }

  uuu::upll_strncpy(rename_vnode->ctrlr_vtn_name,
                    ctrlr_key->vbr_key.vtn_key.vtn_name,
                    (kMaxLenVtnName + 1));
  uuu::upll_strncpy(rename_vnode->ctrlr_vnode_name,
                    ctrlr_key->vbr_key.vbridge_name,
                    (kMaxLenVnodeName + 1));

  MoMgrImpl *mgr =
      reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>(GetMoManager(
          UNC_KT_VBRIDGE)));
  if (NULL == mgr) {
    free(rename_vnode);
    return UPLL_RC_ERR_GENERIC;
  }

  result_code = mgr->GetChildConfigKey(unc_key, NULL);
  if (UPLL_RC_SUCCESS != result_code) {
    free(rename_vnode);
    pfc_log_debug("GetRenamedUncKey failed. GetChildConfigKey failed to "
                  "allocate memory for ConfigKeyVal - %d",
                  result_code);
    return result_code;
  }
  if (!unc_key) {
    free(rename_vnode);
    return UPLL_RC_ERR_GENERIC;
  }

  unc_key->AppendCfgVal(IpctSt::kIpcStValRenameVbr, rename_vnode);

  SET_USER_DATA_CTRLR(unc_key, ctrlr_id);

  result_code = mgr->ReadConfigDB(unc_key, dt_type, UNC_OP_READ, dbop, dmi,
                                  RENAMETBL);
  if (UPLL_RC_SUCCESS != result_code
      && UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
    CONFIGKEYVALCLEAN(unc_key);
    pfc_log_debug("GetRenamedUncKey failed. ReadConfigDB failed to read %d ",
                  result_code);
    return UPLL_RC_ERR_GENERIC;
  }

  if (UPLL_RC_SUCCESS == result_code) {
    key_vbr_if_t *vbr_if_key =
        reinterpret_cast<key_vbr_if_t *>(unc_key->get_key());
    if (vbr_if_key) {
      if (strcmp(reinterpret_cast<char *>(ctrlr_key->vbr_key.vtn_key.vtn_name),
          reinterpret_cast<const char *>
          (vbr_if_key->vbr_key.vtn_key.vtn_name))) {
        uuu::upll_strncpy(
            reinterpret_cast<char *>(ctrlr_key->vbr_key.vtn_key.vtn_name),
            reinterpret_cast<const char *>
            (vbr_if_key->vbr_key.vtn_key.vtn_name),
            (kMaxLenVtnName + 1));
      }
      if (strcmp(reinterpret_cast<char *>(ctrlr_key->vbr_key.vbridge_name),
                 reinterpret_cast<char *>(vbr_if_key->vbr_key.vbridge_name))) {
        uuu::upll_strncpy(ctrlr_key->vbr_key.vbridge_name,
                      vbr_if_key->vbr_key.vbridge_name,
                      (kMaxLenVnodeName + 1));
      }
    }
  }

  mgr = NULL;
  CONFIGKEYVALCLEAN(unc_key);

  val_rename_policingprofile *rename_policingprofile =
      reinterpret_cast<val_rename_policingprofile *>
      (ConfigKeyVal::Malloc(sizeof(val_rename_policingprofile)));

  val_policingmap_t *val_policingmap =
      reinterpret_cast<val_policingmap_t *>(GetVal(ikey));
  if (!val_policingmap) {
    UPLL_LOG_DEBUG("val_policingmap NULL");
    free(rename_policingprofile);
    free(rename_vnode);
    return UPLL_RC_ERR_GENERIC;
  }

  uuu::upll_strncpy(rename_policingprofile->policingprofile_newname,
                    val_policingmap->policer_name,
                    (kMaxLenPolicingProfileName + 1));

  mgr = reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>(GetMoManager(
      UNC_KT_POLICING_PROFILE)));
  if (NULL == mgr) {
    UPLL_LOG_DEBUG("mgr policing profile NULL");
    free(rename_vnode);
    free(rename_policingprofile);
    return UPLL_RC_ERR_GENERIC;
  }

  result_code = mgr->GetChildConfigKey(unc_key, NULL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetRenamedUncKey failed. GetChildConfigKey failed to "
                  "create policingprofile ConfigKeyVal %d",
                  result_code);
    free(rename_vnode);
    free(rename_policingprofile);
    return result_code;
  }
  if (!unc_key) {
    free(rename_vnode);
    free(rename_policingprofile);
    return UPLL_RC_ERR_GENERIC;
  }

  unc_key->AppendCfgVal(IpctSt::kIpcStValRenamePolicingprofile,
                        rename_policingprofile);
  SET_USER_DATA_CTRLR(unc_key, ctrlr_id);

  result_code = mgr->ReadConfigDB(unc_key, dt_type, UNC_OP_READ, dbop, dmi,
                                  RENAMETBL);
  if (UPLL_RC_SUCCESS != result_code
     &&  UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
    UPLL_LOG_DEBUG("GetRenamedUncKey failed. ReadConfigDB failed to read %d ",
                  result_code);
    CONFIGKEYVALCLEAN(unc_key);
    return UPLL_RC_ERR_GENERIC;
  }

  if (result_code == UPLL_RC_SUCCESS) {
    key_policingprofile_t *key_policingprofile =
        reinterpret_cast<key_policingprofile_t *>(unc_key->get_key());
    if (key_policingprofile) {
      if (strcmp(reinterpret_cast<char *>(val_policingmap->policer_name),
         reinterpret_cast<const char *>(key_policingprofile
         ->policingprofile_name))) {
    uuu::upll_strncpy(
                      val_policingmap->policer_name,
                      key_policingprofile->policingprofile_name,
                      (kMaxLenPolicingProfileName + 1));
      }
    }
  }
  CONFIGKEYVALCLEAN(unc_key);
  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrIfPolicingMapMoMgr::GetRenamedControllerKey(
    ConfigKeyVal *ikey, upll_keytype_datatype_t dt_type, DalDmlIntf *dmi,
    controller_domain *ctrlr_dom) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code;
  uint8_t rename = 0;
  ConfigKeyVal *okey = NULL;

  if (NULL == ikey || NULL == dmi || NULL == ctrlr_dom) {
    pfc_log_debug(
        "GetRenamedControllerKey failed. Insufficient input resources");
    return UPLL_RC_ERR_GENERIC;
  }

  DbSubOp dbop = { kOpReadSingle, kOpMatchCtrlr, kOpInOutCtrlr };

  result_code = IsRenamed(ikey, dt_type, dmi, rename);
  if (UPLL_RC_SUCCESS != result_code) {
    pfc_log_debug("GetRenamedControllerKey failed. IsRenamed failed to "
                  "check rename - %d",
                  result_code);
    return result_code;
  }

  if (0 == rename) {
    pfc_log_debug(
        "GetRenamedControllerKey No Rename");
    return UPLL_RC_SUCCESS;
  }

  if (rename & VTN_RENAME || rename & VBR_RENAME) {
    MoMgrImpl *mgr =
      reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>(GetMoManager(
          UNC_KT_VBRIDGE)));
    if (NULL == mgr) {
      UPLL_LOG_DEBUG("mgr NULL");
      return UPLL_RC_ERR_GENERIC;
    }

    result_code = mgr->GetChildConfigKey(okey, NULL);
    if (result_code != UPLL_RC_SUCCESS) {
       UPLL_LOG_DEBUG(" GetRenamedControllerKey failed. "
                     "GetChildConfigKey failed to create vbr ConfigKey - %d",
                     result_code);
       return result_code;
    }
    if (!okey) return UPLL_RC_ERR_GENERIC;
    key_vbr_if_t *ovbr_key = reinterpret_cast<key_vbr_if_t*>(okey->get_key());
    key_vbr_if_t *ivbr_key = reinterpret_cast<key_vbr_if_t*>(ikey->get_key());

    uuu::upll_strncpy(reinterpret_cast<char*>
          (ovbr_key->vbr_key.vtn_key.vtn_name),
          reinterpret_cast<char*>(ivbr_key->vbr_key.vtn_key.vtn_name),
          (kMaxLenVtnName+1));
    uuu::upll_strncpy(reinterpret_cast<char*>(ovbr_key->vbr_key.vbridge_name),
          reinterpret_cast<char*>(ivbr_key->vbr_key.vbridge_name),
          (kMaxLenVnodeName+1));

    if (NULL != ctrlr_dom) {
      SET_USER_DATA_CTRLR_DOMAIN(okey, *ctrlr_dom);
    }

    /* TODO Commented the below code to fix the compilation issues */
    result_code = mgr->ReadConfigDB(okey, dt_type, UNC_OP_READ, dbop, dmi,
                                    RENAMETBL);
    if (result_code != UPLL_RC_SUCCESS) {
      pfc_log_debug("GetRenamedControllerKey failed. ReadConfigDB failed "
                    "to read vbr renametbl - %d",
                    result_code);
      CONFIGKEYVALCLEAN(okey);
      return result_code;
    }
    val_rename_vnode_t *rename_val =
        reinterpret_cast<val_rename_vnode_t *>(GetVal(okey));
    if (!rename_val) {
      CONFIGKEYVALCLEAN(okey);
      return UPLL_RC_ERR_GENERIC;
    }
    if (rename & VTN_RENAME) {  // vtn renamed
      uuu::upll_strncpy(ivbr_key->vbr_key.vtn_key.vtn_name,
                        rename_val->ctrlr_vtn_name,
                        (kMaxLenVtnName + 1));
    }
    if (rename & VBR_RENAME) {  // vnode renamed
      uuu::upll_strncpy(ivbr_key->vbr_key.vbridge_name,
                        rename_val->ctrlr_vnode_name,
                        (kMaxLenVnodeName + 1));
    }
    SET_USER_DATA_FLAGS(ikey, rename);
    mgr = NULL;
  }

  CONFIGKEYVALCLEAN(okey);

  if (rename & POLICINGPROFILE_RENAME) {
    MoMgrImpl *mgr = reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>
             (GetMoManager(UNC_KT_POLICING_PROFILE)));
    if (NULL == mgr) {
      UPLL_LOG_DEBUG("mgr NULL");
      return UPLL_RC_ERR_GENERIC;
    }
    result_code = mgr->GetChildConfigKey(okey, NULL);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("GetRenamedControllerKey failed. GetChildConfigKey failed "
                  "to create policingprofile ConfigKey - %d",
                  result_code);
      return result_code;
    }
    if (!okey) return UPLL_RC_ERR_GENERIC;
    val_policingmap_t *val_policingmap =
      reinterpret_cast<val_policingmap_t *>(GetVal(ikey));

    key_policingprofile_t *key_policingprofile =
      reinterpret_cast<key_policingprofile_t *>(okey->get_key());

    if (NULL == val_policingmap || NULL == key_policingprofile) {
      UPLL_LOG_DEBUG("mgr NULL");
      CONFIGKEYVALCLEAN(okey);
      return UPLL_RC_ERR_GENERIC;
    }

    uuu::upll_strncpy(key_policingprofile->policingprofile_name,
                    val_policingmap->policer_name,
                    (kMaxLenPolicingProfileName + 1));

    if (ctrlr_dom != NULL) {
      SET_USER_DATA_CTRLR_DOMAIN(okey, *ctrlr_dom);
    }

    result_code = mgr->ReadConfigDB(okey, dt_type, UNC_OP_READ, dbop, dmi,
                                  RENAMETBL);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("GetRenamedControllerKey failed. ReadConfigDB failed "
                  "to read policingprofile renametbl - %d",
                  result_code);
      CONFIGKEYVALCLEAN(okey);
      return result_code;
    }

    val_rename_policingprofile_t *rename_policingprofile =
      reinterpret_cast<val_rename_policingprofile_t *>(GetVal(okey));
    if (!rename_policingprofile) {
      UPLL_LOG_DEBUG("rename_policingprofile NULL")
      CONFIGKEYVALCLEAN(okey);
      return UPLL_RC_ERR_GENERIC;
    }

    uuu::upll_strncpy(val_policingmap->policer_name,
                      rename_policingprofile->policingprofile_newname,
                      (kMaxLenPolicingProfileName + 1));
    SET_USER_DATA_FLAGS(ikey, rename);
    CONFIGKEYVALCLEAN(okey);
  }
  CONFIGKEYVALCLEAN(okey);
  return UPLL_RC_SUCCESS;
}

bool VbrIfPolicingMapMoMgr::CompareValidValue(void *&val1, void *val2,
                                              bool audit) {
  val_policingmap_t *val_pm1 = reinterpret_cast<val_policingmap_t *>(val1);
  val_policingmap_t *val_pm2 = reinterpret_cast<val_policingmap_t *>(val2);
 // if (audit) {
    if (UNC_VF_INVALID == val_pm1->valid[UPLL_IDX_POLICERNAME_PM] &&
      UNC_VF_VALID == val_pm2->valid[UPLL_IDX_POLICERNAME_PM])
      val_pm1->valid[UPLL_IDX_POLICERNAME_PM] = UNC_VF_VALID_NO_VALUE;
//  }
  if (UNC_VF_VALID == val_pm1->valid[UPLL_IDX_POLICERNAME_PM] &&
      UNC_VF_VALID == val_pm2->valid[UPLL_IDX_POLICERNAME_PM]) {
     if (!strcmp(reinterpret_cast<char*>(val_pm1->policer_name),
               reinterpret_cast<char*>(val_pm2->policer_name)))
     val_pm1->valid[UPLL_IDX_POLICERNAME_PM] = UNC_VF_INVALID;
  }
  return false;
}

bool VbrIfPolicingMapMoMgr::CompareKey(ConfigKeyVal *key1, ConfigKeyVal *key2) {
  UPLL_FUNC_TRACE;
  key_vbr_if_t *vbr_if_key1, *vbr_if_key2;
  bool match = false;
  vbr_if_key1 = reinterpret_cast<key_vbr_if_t *>(key1);
  vbr_if_key2 = reinterpret_cast<key_vbr_if_t *>(key2);
  if (vbr_if_key1 == vbr_if_key2) {
    return true;
  }
  if ((!vbr_if_key1) || (!vbr_if_key2)) {
    UPLL_LOG_DEBUG("PolicingProfileEntryMoMgr::CompareKey failed");
    return false;
  }
  if (strncmp(
      reinterpret_cast<const char *>(vbr_if_key1->vbr_key.vtn_key.vtn_name),
      reinterpret_cast<const char *>(vbr_if_key2->vbr_key.vtn_key.vtn_name),
      kMaxLenVtnName + 1) == 0) {
    match = true;
    UPLL_LOG_DEBUG("PolicingProfileEntryMoMgr::CompareKey, Both Keys are same");
  }
  return match;
}

upll_rc_t VbrIfPolicingMapMoMgr::UpdateConfigStatus(ConfigKeyVal *ckv,
    unc_keytype_operation_t op, uint32_t driver_result, ConfigKeyVal *nreq,
    DalDmlIntf *dmi, ConfigKeyVal *ctrlr_key) {
  UPLL_FUNC_TRACE;
  val_policingmap_t *val = NULL;
  unc_keytype_configstatus_t cs_status =
      (driver_result == 0) ? UNC_CS_APPLIED : UNC_CS_NOT_APPLIED;
  val = reinterpret_cast<val_policingmap_t *> (GetVal(ckv));
  if (val == NULL) return UPLL_RC_ERR_GENERIC;
  if (op == UNC_OP_CREATE) {
    if (val->cs_row_status != UNC_CS_NOT_SUPPORTED)
      val->cs_row_status = cs_status;
  } else {
      UPLL_LOG_DEBUG("Operation Not Supported.");
      return UPLL_RC_ERR_GENERIC;
    }

  UPLL_LOG_DEBUG("Update Config Status Successfull.");
  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrIfPolicingMapMoMgr::UpdateAuditConfigStatus(
                               unc_keytype_configstatus_t cs_status,
                               uuc::UpdateCtrlrPhase phase,
                               ConfigKeyVal *&ckv_running) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  val_vbr_t *val;
  val =
      (ckv_running != NULL) ? reinterpret_cast<val_vbr_t *>
                              (GetVal(ckv_running)) :NULL;
  if (NULL == val) {
    UPLL_LOG_DEBUG("vbr_val NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  if (uuc::kUpllUcpCreate == phase)
    val->cs_row_status = cs_status;
  for (unsigned int loop = 0; loop < (sizeof(val->valid)/sizeof(uint8_t));
      ++loop) {
    if (cs_status == UNC_CS_INVALID && UNC_VF_VALID == val->valid[loop])
       val->cs_attr[loop] = cs_status;
    else
       val->cs_attr[loop] = cs_status;
  }
  return result_code;
}

upll_rc_t VbrIfPolicingMapMoMgr::GetValid(void *val, uint64_t indx,
                                          uint8_t *&valid,
                                          upll_keytype_datatype_t dt_type,
                                          MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  if (val == NULL || tbl != MAINTBL) {
    UPLL_LOG_DEBUG("GetValid val ot tbl NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  if (tbl == MAINTBL) {
    switch (indx) {
      case uudst::vbr_if_policingmap::kDbiPolicername:
        valid = &(reinterpret_cast<val_policingmap_t *>
                (val)->valid[UPLL_IDX_POLICERNAME_PM]);
        break;
      default:
        UPLL_LOG_DEBUG("Invalid Index");
        valid = NULL;
        break;
    }
  }
  UPLL_LOG_DEBUG("GetValid IS successful:-");
  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrIfPolicingMapMoMgr::AllocVal(ConfigVal *&ck_val,
                                        upll_keytype_datatype_t dt_type,
                                        MoMgrTables tbl) {
UPLL_FUNC_TRACE;
void *val;
if (ck_val != NULL) {
  UPLL_LOG_DEBUG("ck_val is not NULL");
  return UPLL_RC_ERR_GENERIC;
}
switch (tbl) {
  case MAINTBL:
    val = reinterpret_cast<void *>
        (ConfigKeyVal::Malloc(sizeof(val_policingmap_t)));
    ck_val = new ConfigVal(IpctSt::kIpcStValPolicingmap, val);
  break;
  default:
    UPLL_LOG_DEBUG("AllocVal val_policingmap_t Allocation failure");
    val = NULL;
    break;
  }
  if (NULL == ck_val) {
    UPLL_LOG_DEBUG("AllocVal ck_val Allocation failure");
    return UPLL_RC_ERR_GENERIC;
  }

  UPLL_LOG_DEBUG("AllocVal ck_val Allocation successful");
  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrIfPolicingMapMoMgr::DupConfigKeyVal(ConfigKeyVal *&okey,
                                                 ConfigKeyVal *&req,
                                                 MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  if (req == NULL || okey != NULL || tbl != MAINTBL) {
    UPLL_LOG_DEBUG(" DupConfigKeyVal failed. Input ConfigKeyVal is NULL");
    return UPLL_RC_ERR_GENERIC;
  }

  ConfigVal *tmp1 = NULL, *tmp = (req)->get_cfg_val();
  val_policingmap_t *policingmap_val = NULL;
  if (tmp) {
    if (tbl == MAINTBL) {
      val_policingmap_t *ival = reinterpret_cast<val_policingmap_t *>(GetVal(
          req));
      if (NULL == ival) {
        UPLL_LOG_DEBUG("DupConfigKeyVal val_policingmap_t alloc failure");
        return UPLL_RC_ERR_GENERIC;
      }
      policingmap_val = reinterpret_cast<val_policingmap_t *>
          (ConfigKeyVal::Malloc(sizeof(val_policingmap_t)));
      memcpy(policingmap_val, ival, sizeof(val_policingmap_t));
      tmp1 = new ConfigVal(IpctSt::kIpcStValPolicingmap, policingmap_val);

      if (!tmp1) {
        UPLL_LOG_DEBUG("ConfigVal Alloc failed");
        free(policingmap_val);
        return UPLL_RC_ERR_GENERIC;
      }
      UPLL_LOG_DEBUG("val_policingmap_t policingmap_val Alloc Successful");
    }
    if (tmp1) {
      tmp1->set_user_data(tmp->get_user_data());
//      tmp = tmp->get_next_cfg_val();
    }
  }
  void *tkey = (req != NULL) ? (req)->get_key() : NULL;
  key_vbr_if_t *ikey = reinterpret_cast<key_vbr_if_t *>(tkey);
  key_vbr_if_t *vbr_if_key = reinterpret_cast<key_vbr_if_t *>
  (ConfigKeyVal::Malloc(sizeof(key_vbr_if_t)));
  memcpy(vbr_if_key, ikey, sizeof(key_vbr_if_t));
  okey = new ConfigKeyVal(UNC_KT_VBRIF_POLICINGMAP, IpctSt::kIpcStKeyVbrIf,
                          vbr_if_key, tmp1);
  if (!okey) {
    if (vbr_if_key) free(vbr_if_key);
    if (policingmap_val) free(policingmap_val);
    UPLL_LOG_DEBUG("okey failed");
    return UPLL_RC_ERR_GENERIC;
  }
  SET_USER_DATA(okey, req);
  UPLL_LOG_DEBUG("PolicingProfileEntryMoMgr::Successful Compilation ");
  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrIfPolicingMapMoMgr::GetChildConfigKey(ConfigKeyVal *&okey,
                                                   ConfigKeyVal *parent_key) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  key_vbr_if_t *vbr_if_key = NULL;
  void *pkey = NULL;

  if (parent_key == NULL) {
    vbr_if_key = reinterpret_cast<key_vbr_if_t *>
        (ConfigKeyVal::Malloc(sizeof(key_vbr_if_t)));
    okey = new ConfigKeyVal(UNC_KT_VBRIF_POLICINGMAP, IpctSt::kIpcStKeyVbrIf,
                            vbr_if_key, NULL);
    return UPLL_RC_SUCCESS;
  } else {
    pkey = parent_key->get_key();
  }

  if (!pkey) {
    UPLL_LOG_DEBUG("error Generated::Key type not supported :");
    return UPLL_RC_ERR_GENERIC;
  }

  if (okey) {
    if (okey->get_key_type() != UNC_KT_VBRIF_POLICINGMAP)
      return UPLL_RC_ERR_GENERIC;
    vbr_if_key = reinterpret_cast<key_vbr_if_t *>(okey->get_key());
  } else {
      vbr_if_key = reinterpret_cast<key_vbr_if_t *>
        (ConfigKeyVal::Malloc(sizeof(key_vbr_if_t)));
  }
  switch (parent_key->get_key_type()) {
    case UNC_KT_VTN:
      uuu::upll_strncpy(vbr_if_key->vbr_key.vtn_key.vtn_name,
                        reinterpret_cast<key_vtn *>(pkey)->vtn_name,
                        (kMaxLenVtnName + 1));
      break;
    case UNC_KT_VBRIDGE:
      uuu::upll_strncpy(vbr_if_key->vbr_key.vtn_key.vtn_name,
                        reinterpret_cast<key_vbr *>(pkey)->vtn_key.vtn_name,
                        (kMaxLenVtnName + 1));
      uuu::upll_strncpy(vbr_if_key->vbr_key.vbridge_name,
                        reinterpret_cast<key_vbr *>(pkey)->vbridge_name,
                        (kMaxLenVnodeName + 1));
      break;
    case UNC_KT_VBR_IF:
    case UNC_KT_VBRIF_POLICINGMAP:
      uuu::upll_strncpy(
          vbr_if_key->vbr_key.vtn_key.vtn_name,
          reinterpret_cast<key_vbr_if *>(pkey)->vbr_key.vtn_key.vtn_name,
          (kMaxLenVtnName + 1));
      uuu::upll_strncpy(
          vbr_if_key->vbr_key.vbridge_name,
          reinterpret_cast<key_vbr_if *>(pkey)->vbr_key.vbridge_name,
          (kMaxLenVnodeName + 1));
      uuu::upll_strncpy(vbr_if_key->if_name,
                        reinterpret_cast<key_vbr_if *>(pkey)->if_name,
                        (kMaxLenInterfaceName + 1));
      break;
    default:
      if (vbr_if_key) free(vbr_if_key);
      return UPLL_RC_ERR_GENERIC;
  }

  if (!okey) {
       okey = new ConfigKeyVal(UNC_KT_VBRIF_POLICINGMAP,
                IpctSt::kIpcStKeyVbrIf, vbr_if_key, NULL);
  }
  SET_USER_DATA(okey, parent_key);
  return result_code;
}

upll_rc_t VbrIfPolicingMapMoMgr::ValidateMessage(IpcReqRespHeader *req,
                                                 ConfigKeyVal *key) {
  UPLL_FUNC_TRACE;

  upll_rc_t rt_code = UPLL_RC_ERR_GENERIC;

  if ((NULL == req) || (NULL == key)) {
    UPLL_LOG_DEBUG("ConfigKeyval is NULL");
    return UPLL_RC_ERR_BAD_REQUEST;
  }

  key_vbr_if_t *key_vbrif = NULL;
  key_vbrif_policingmap_entry_t *key_vbrif_policingmap_entry = NULL;
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

  if (UNC_KT_VBRIF_POLICINGMAP == key->get_key_type()) {
    if (key->get_st_num() != IpctSt::kIpcStKeyVbrIf) {
      UPLL_LOG_DEBUG(
          " Invalid structure received expected struct -"
          "kIpcStKeyVbrIf, received struct - %s ",
          reinterpret_cast<const char *>
          (IpctSt::GetIpcStdef(key->get_st_num())));
      return UPLL_RC_ERR_BAD_REQUEST;
    }
    key_vbrif = reinterpret_cast<key_vbr_if_t *>(key->get_key());
  } else if (UNC_KT_VBRIF_POLICINGMAP_ENTRY == key->get_key_type()) {
    if (key->get_st_num() != IpctSt::kIpcStKeyVbrifPolicingmapEntry) {
      UPLL_LOG_DEBUG(
          " Invalid structure received expected struct -"
          "kIpcStKeyVbrifPolicingmapEntry, received struct - %s ",
          reinterpret_cast<const char *>
          (IpctSt::GetIpcStdef(key->get_st_num())));
      return UPLL_RC_ERR_BAD_REQUEST;
    }
    key_vbrif_policingmap_entry =
        reinterpret_cast<key_vbrif_policingmap_entry_t*>(key->get_key());
    key_vbrif = &(key_vbrif_policingmap_entry->vbrif_key);
  } else {
    UPLL_LOG_DEBUG("Invalid keytype(%d)", key->get_key_type());
    return UPLL_RC_ERR_BAD_REQUEST;
  }

  /** Validate Key structure */
  if (NULL == key_vbrif) {
    UPLL_LOG_DEBUG("KT_VBRIF_POLICINGMAP Key structure is empty!!");
    return UPLL_RC_ERR_BAD_REQUEST;
  }

  /** Use VbrIfMoMgr api to validate key struct */

  if (UNC_KT_VBRIF_POLICINGMAP_ENTRY == key->get_key_type()) {
    /* validate seq number */
    if (!ValidateNumericRange(key_vbrif_policingmap_entry->sequence_num,
            kMinPolicingProfileSeqNum,
            kMaxPolicingProfileSeqNum, true,
            true)) {
      UPLL_LOG_DEBUG("Sequence num syntax validation failed :Err Code - %d",
          rt_code);
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
    UPLL_LOG_TRACE(
        "key struct validation is success for UNC_KT_VBR_POLICINGMAP_ENTRY");
    return UPLL_RC_SUCCESS;
  }

  UPLL_LOG_TRACE(" key struct validation is success");

  /** read datatype, operation, options from IpcReqRespHeader */
  if (NULL == req) {
    UPLL_LOG_DEBUG("IpcReqRespHeader is NULL");
    return UPLL_RC_ERR_BAD_REQUEST;
  }

  val_policingmap_t *val_policingmap = NULL;

  if (key->get_cfg_val() && (key->get_cfg_val()->get_st_num() ==
      IpctSt::kIpcStValPolicingmap)) {
      val_policingmap =
      reinterpret_cast<val_policingmap_t *>(key->get_cfg_val()->get_val());
  }

  /** Use  VtnPolicingMapMoMgr::ValidatePolicingMapValue
   *  to validate value structure */
  rt_code = VtnPolicingMapMoMgr::ValidatePolicingMapValue(val_policingmap, req);

  if (UPLL_RC_SUCCESS != rt_code) {
    UPLL_LOG_DEBUG(" PolicierName syntax validation failed :"
                  "Err Code - %d",
                  rt_code);
  }
  return rt_code;
}

upll_rc_t VbrIfPolicingMapMoMgr::ValidateCapability(IpcReqRespHeader *req,
                                              ConfigKeyVal *ikey,
                                              const char *ctrlr_name) {
  UPLL_FUNC_TRACE;

  upll_rc_t rt_code = UPLL_RC_ERR_GENERIC;

  if (NULL == ikey) {
    UPLL_LOG_DEBUG("ConfigKeyval is NULL");
    return rt_code;
  }
  /** Use  VtnPolicingMapMoMgr::ValidateCapability
   *  to validate capability for val_policingmap structure*/
  VtnPolicingMapMoMgr *mgrvtnpmap =
      reinterpret_cast<VtnPolicingMapMoMgr *>(const_cast<MoManager *>
          (GetMoManager(UNC_KT_VTN_POLICINGMAP)));

  rt_code = mgrvtnpmap->ValidateCapability(req, ikey);

  if (UPLL_RC_SUCCESS != rt_code) {
    UPLL_LOG_DEBUG(" PolicierName Attribute validation failed :"
                  "Err Code - %d",
                  rt_code);
  }
  return rt_code;
}
upll_rc_t VbrIfPolicingMapMoMgr::GetParentConfigKey(ConfigKeyVal *&okey,
                                                    ConfigKeyVal *ikey) {
  UPLL_FUNC_TRACE;

  if (!ikey) {
    UPLL_LOG_DEBUG(" Input Key is NULL");
    return UPLL_RC_ERR_GENERIC;
  }

  unc_key_type_t ikey_type = ikey->get_key_type();
  if (ikey_type != UNC_KT_VBRIF_POLICINGMAP) {
    UPLL_LOG_DEBUG(" Invalid key type received. Key type - %d", ikey_type);
    return UPLL_RC_ERR_GENERIC;
  }

  key_vbr_if_t *pkey = reinterpret_cast<key_vbr_if_t*>(ikey->get_key());
  if (!pkey) {
    UPLL_LOG_DEBUG(" Input vbr if policing map key is NULL ");
    return UPLL_RC_ERR_GENERIC;
  }

  key_vbr_if_t *vbr_if_key = reinterpret_cast<key_vbr_if_t*>
      (ConfigKeyVal::Malloc(sizeof(key_vbr_if_t)));

  memcpy(vbr_if_key, reinterpret_cast<key_vbr_if_t*>(pkey),
         sizeof(key_vbr_if_t));
  okey = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf,
                          vbr_if_key, NULL);
  SET_USER_DATA(okey, ikey);
  return UPLL_RC_SUCCESS;
}

bool VbrIfPolicingMapMoMgr::IsValidKey(void *key, uint64_t index) {
  UPLL_FUNC_TRACE;
  key_vbr_if *if_key = reinterpret_cast<key_vbr_if *>(key);
  bool ret_val = UPLL_RC_SUCCESS;
  switch (index) {
    case uudst::vbridge_interface::kDbiVtnName:
      ret_val = ValidateKey(
          reinterpret_cast<char *>(if_key->vbr_key.vtn_key.vtn_name),
          kMinLenVtnName, kMaxLenVtnName);
      if (ret_val != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("VTN Name is not valid(%d)", ret_val);
        return false;
      }
      break;
    case uudst::vbridge_interface::kDbiVbrName:
      ret_val = ValidateKey(
          reinterpret_cast<char *>(if_key->vbr_key.vbridge_name),
          kMinLenVnodeName, kMaxLenVnodeName);
      if (ret_val != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("VBR Name is not valid(%d)", ret_val);
        return false;
      }
      break;
    case uudst::vbridge_interface::kDbiIfName:
      ret_val = ValidateKey(reinterpret_cast<char *>(if_key->if_name),
                            kMinLenInterfaceName, kMaxLenInterfaceName);
      if (ret_val != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("VBR IF Name is not valid(%d)", ret_val);
        return false;
      }
      break;
    default:
      UPLL_LOG_DEBUG("Wrong Index");
      break;
  }
  return true;
}

upll_rc_t VbrIfPolicingMapMoMgr::SwapKeyVal(ConfigKeyVal *ikey,
                       ConfigKeyVal *&okey,
                       DalDmlIntf *dmi, uint8_t *ctrlr) {
  return UPLL_RC_SUCCESS;
}
upll_rc_t VbrIfPolicingMapMoMgr::IsKeyInUse(upll_keytype_datatype_t dt_type,
                     const ConfigKeyVal *ckv,
                     bool *in_use,
                     DalDmlIntf *dmi) {
    return UPLL_RC_SUCCESS;
}

upll_rc_t VbrIfPolicingMapMoMgr::TxUpdateController(unc_key_type_t keytype,
    uint32_t session_id,
    uint32_t config_id,
    uuc::UpdateCtrlrPhase phase,
    set<string> *affected_ctrlr_set,
    DalDmlIntf *dmi,
    ConfigKeyVal **err_ckv)  {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  DalResultCode db_result;
  ConfigKeyVal *req, *nreq = NULL, *ck_main = NULL;
  controller_domain_t ctrlr_dom;
  ctrlr_dom.ctrlr = NULL;
  ctrlr_dom.domain = NULL;
  DalCursor *dal_cursor_handle = NULL;
  IpcResponse ipc_resp;
  // uint8_t flags = 0;
  uint8_t db_flag = 0;
  uint8_t flag = 0;

  upll_keytype_datatype_t vext_datatype =  UPLL_DT_CANDIDATE;

  if (affected_ctrlr_set == NULL) {
    UPLL_LOG_DEBUG("affected_ctrlr_set is NULL\n");
    return UPLL_RC_ERR_GENERIC;
  }
  unc_keytype_operation_t op = (phase == uuc::kUpllUcpCreate)?UNC_OP_CREATE:
     ((phase == uuc::kUpllUcpUpdate)?UNC_OP_UPDATE:
     ((phase == uuc::kUpllUcpDelete)?UNC_OP_DELETE:UNC_OP_INVALID));

  result_code = DiffConfigDB(UPLL_DT_CANDIDATE, UPLL_DT_RUNNING,
      op, req, nreq, &dal_cursor_handle, dmi, MAINTBL);
  while (result_code == UPLL_RC_SUCCESS) {
    //  Get Next Record
    db_result = dmi->GetNextRecord(dal_cursor_handle);
    result_code = DalToUpllResCode(db_result);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG(" GetNextRecord failed err code(%d)", result_code);
      break;
    }
    switch (op)   {
      case UNC_OP_CREATE:
      case UNC_OP_UPDATE:
        /* fall through intended */
        UPLL_LOG_DEBUG("DupConfigKeyVal CREATE/UPDATE (%d)", op);
        result_code = DupConfigKeyVal(ck_main, req, MAINTBL);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_TRACE("DupConfigKeyVal failed %d\n", result_code);
          return result_code;
        }
        break;
      case UNC_OP_DELETE:
        UPLL_LOG_DEBUG("DupConfigKeyVal DELETE (%d)", op);
        result_code = GetChildConfigKey(ck_main, req);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_TRACE("GetChildConfigKey failed %d\n", result_code);
          return result_code;
        }
      default:
        break;
    }

#if 0
    if (op == UNC_OP_DELETE) {
      UPLL_LOG_DEBUG("UNC_OP_DELETE\n");
      if (ck_main->get_cfg_val()) {
        UPLL_LOG_DEBUG("Invalid param\n");
        return UPLL_RC_ERR_GENERIC;
      }
      DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutNone};
      result_code = ReadConfigDB(ck_main, UPLL_DT_RUNNING, UNC_OP_READ,
          dbop, dmi, MAINTBL);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Returning error %d\n", result_code);
        return UPLL_RC_ERR_GENERIC;
      }
    }
#endif
    GET_USER_DATA_CTRLR_DOMAIN(ck_main, ctrlr_dom);
    if (ctrlr_dom.ctrlr == NULL) {
      UPLL_LOG_DEBUG("ctrlr_dom.ctrlr NULL\n");
      return UPLL_RC_ERR_GENERIC;
    }
#if 0
    if ((op == UNC_OP_CREATE) || (op == UNC_OP_UPDATE)) {
      UPLL_LOG_DEBUG("C/U");
      void *main = GetVal(ck_main);
      void *val_nrec = (nreq) ? GetVal(nreq) : NULL;
      FilterAttributes(main, val_nrec, false, op);
    }
#endif
    GET_USER_DATA_FLAGS(ck_main, db_flag);
    UPLL_LOG_DEBUG("db_flag (%d)", db_flag);

    if (!(SET_FLAG_PORTMAP & db_flag)) {
      if (op != UNC_OP_UPDATE) {
        UPLL_LOG_DEBUG("op != UNC_OP_UPDATE continue\n");
        continue;
      } else {
        ConfigKeyVal *temp = NULL;
        result_code = GetChildConfigKey(temp, ck_main);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_TRACE("GetChildConfigKey failed %d\n", result_code);
          return result_code;
        }
        SET_USER_DATA_CTRLR_DOMAIN(temp, ctrlr_dom);
        DbSubOp dbop = { kOpReadSingle, kOpMatchCtrlr|kOpMatchDomain,
          kOpInOutFlag};
        result_code = ReadConfigDB(temp, UPLL_DT_RUNNING, UNC_OP_READ,
                                   dbop, dmi, MAINTBL);
        if (result_code != UPLL_RC_SUCCESS) {
          if (result_code != UPLL_RC_ERR_INSTANCE_EXISTS) {
            UPLL_LOG_DEBUG("Unable to read from DB, err: %d", result_code);
            return result_code;
          }
        }
        GET_USER_DATA_FLAGS(temp, flag);
        UPLL_LOG_DEBUG("flag (%d)", flag);
        if (!(SET_FLAG_PORTMAP & flag)) {
          UPLL_LOG_DEBUG("SET_FLAG_PORTMAP & flag\n");
          continue;
        }
        op = UNC_OP_DELETE;
        vext_datatype = UPLL_DT_RUNNING;
        UPLL_LOG_DEBUG("Data type changes as RUNNING op (%d) data type(%d)",
                       op, vext_datatype);
        db_flag = flag;
      }
    } else if (UNC_OP_UPDATE == op) {
      ConfigKeyVal *temp = NULL;
      result_code = GetChildConfigKey(temp, ck_main);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_TRACE("GetChildConfigKey failed %d\n", result_code);
        return result_code;
      }
      SET_USER_DATA_CTRLR_DOMAIN(temp, ctrlr_dom);
      DbSubOp dbop = { kOpReadSingle, kOpMatchCtrlr|kOpMatchDomain,
        kOpInOutFlag};
      result_code = ReadConfigDB(temp, UPLL_DT_RUNNING, UNC_OP_READ,
                                 dbop, dmi, MAINTBL);
      if (result_code != UPLL_RC_SUCCESS) {
        if (result_code != UPLL_RC_ERR_INSTANCE_EXISTS) {
          UPLL_LOG_DEBUG("Unable to read from DB, err: %d", result_code);
          return result_code;
        }
      }
      GET_USER_DATA_FLAGS(temp, flag);
      UPLL_LOG_DEBUG("flag (%d)", flag);
      if (!(SET_FLAG_PORTMAP & flag)) {
        op = UNC_OP_CREATE;
        vext_datatype = UPLL_DT_CANDIDATE;
        UPLL_LOG_DEBUG("Data type changes as CANDIDATE op (%d) , datatype (%d)",
                       op, vext_datatype);
      }
    }
#if 0
    if (!(SET_FLAG_PORTMAP & db_flag) && !(SET_FLAG_PORTMAP & db_flag)) {
      UPLL_LOG_DEBUG("SET_FLAG_PORTMAP & db_flag");
      result_code = UPLL_RC_SUCCESS;
      continue;
    }
#endif
    pfcdrv_val_vbrif_policingmap *pfc_val =
        reinterpret_cast<pfcdrv_val_vbrif_policingmap *>
        (ConfigKeyVal::Malloc(sizeof(pfcdrv_val_vbrif_policingmap_t)));
    pfcdrv_val_vbrif_vextif *pfc_val_ext =
        reinterpret_cast<pfcdrv_val_vbrif_vextif *>\
        (ConfigKeyVal::Malloc(sizeof(pfcdrv_val_vbrif_vextif_t)));

    UPLL_LOG_DEBUG("GetVexternalInformation (%d)", vext_datatype);
    string s(ck_main->ToStrAll());
    result_code = GetVexternalInformation(ck_main, vext_datatype, pfc_val,
        pfc_val_ext, db_flag, dmi);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("GetVexternalInformation fail");
      return result_code;
    }
    upll_keytype_datatype_t dt_type = (op == UNC_OP_DELETE)?
      UPLL_DT_RUNNING:UPLL_DT_CANDIDATE;

    UPLL_LOG_DEBUG("Controller : %s; Domain : %s", ctrlr_dom.ctrlr,
        ctrlr_dom.domain);
    string s1(ck_main->ToStrAll());
    result_code = GetRenamedControllerKey(ck_main, dt_type,
        dmi, &ctrlr_dom);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("GetRenamedControllerKey fail");
      break;
    }

    val_policingmap_t* val = reinterpret_cast<val_policingmap_t *>(GetVal(req));
    UPLL_LOG_DEBUG("val_policingmap_t (%s)", val->policer_name);

    pfc_val->valid[PFCDRV_IDX_VAL_POLICINGMAP_PM] = UNC_VF_VALID;
    memcpy(&pfc_val->val_policing_map, val, sizeof(val_policingmap_t));

    pfc_val->valid[PFCDRV_IDX_VAL_VBRIF_VEXTIF_PM] = UNC_VF_VALID;
    memcpy(&pfc_val->val_vbrif_vextif, pfc_val_ext,
           sizeof(pfcdrv_val_vbrif_vextif_t));

//    delete ck_main->get_cfg_val();
//    ck_main->AppendCfgVal(IpctSt::kIpcStPfcdrvValVbrifPolicingmap, pfc_val);
    ck_main->SetCfgVal(new ConfigVal(IpctSt::kIpcStPfcdrvValVbrifPolicingmap,
                                     pfc_val));

    affected_ctrlr_set->insert
      (string(reinterpret_cast<char *>(ctrlr_dom.ctrlr)));

    UPLL_LOG_DEBUG("Controller : %s; Domain : %s", ctrlr_dom.ctrlr,
        ctrlr_dom.domain);
    memset(&ipc_resp, 0, sizeof(IpcResponse));
    result_code = SendIpcReq(session_id, config_id, op, UPLL_DT_CANDIDATE,
        ck_main, &ctrlr_dom, &ipc_resp);
    if (result_code == UPLL_RC_ERR_RESOURCE_DISCONNECTED) {
      UPLL_LOG_DEBUG("driver result code - %d", result_code);
      result_code = UPLL_RC_SUCCESS;
    }
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("IpcSend failed %d\n", result_code);
      *err_ckv = ipc_resp.ckv_data;
      if (ck_main)
        delete ck_main;
      break;
    }
#if 0
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("IpcSend failed %d\n", result_code);
      return result_code;
    }

    UPLL_LOG_DEBUG("SendIpcReq");
    if (ipc_resp.header.result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("ipc_resp.header.result_code fail");
      *err_ckv = ipc_resp.ckv_data;
    }
#endif
    if (ck_main)
      delete ck_main;
    ck_main = NULL;
  }
  dmi->CloseCursor(dal_cursor_handle, true);
  if (nreq)
    delete nreq;
  result_code = (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) ?
      UPLL_RC_SUCCESS : result_code;
  return result_code;
}

upll_rc_t VbrIfPolicingMapMoMgr::SetVlinkPortmapConfiguration(
    ConfigKeyVal *ikey,
    upll_keytype_datatype_t dt_type,
    DalDmlIntf *dmi,
    InterfacePortMapInfo flag) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_ERR_GENERIC;
  if (NULL == ikey || NULL == ikey->get_key()) {
    return result_code;
  }
  if (!flag) {
//    return UPLL_RC_SUCCESS;
  }
  ConfigKeyVal *ckv = NULL;
  result_code = GetChildConfigKey(ckv, NULL);
  if (UPLL_RC_SUCCESS != result_code) {
    return result_code;
  }
  key_vbr_if_t *pp_key = reinterpret_cast<key_vbr_if_t *>(ckv->get_key());
  key_vbr_if_t *vbrif_key = reinterpret_cast<key_vbr_if_t *>(ikey->get_key());

  uuu::upll_strncpy(pp_key->vbr_key.vtn_key.vtn_name,
      vbrif_key->vbr_key.vtn_key.vtn_name,
      kMaxLenVtnName + 1);
  UPLL_LOG_DEBUG("vtn name (%s)", pp_key->vbr_key.vtn_key.vtn_name);

  uuu::upll_strncpy(pp_key->vbr_key.vbridge_name,
      vbrif_key->vbr_key.vbridge_name,
      kMaxLenVnodeName + 1);
  UPLL_LOG_DEBUG("vbr name (%s)", pp_key->vbr_key.vbridge_name);

  uuu::upll_strncpy(pp_key->if_name,
      vbrif_key->if_name,
      kMaxLenInterfaceName + 1);
  UPLL_LOG_DEBUG("vtn name (%s)", pp_key->if_name);


  UPLL_LOG_DEBUG("dt_type (%d)", dt_type);

  DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutFlag };
  result_code = ReadConfigDB(ckv, dt_type ,
      UNC_OP_READ, dbop, dmi, MAINTBL);
  if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
    UPLL_LOG_DEBUG("No Recrods in vbr_if_policingmap Table");
    return UPLL_RC_SUCCESS;
  }
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("Read ConfigDB failure %d", result_code);
    delete ckv;
    return result_code;
  }
  uint8_t  flag_port_map = 0;
  GET_USER_DATA_FLAGS(ckv, flag_port_map);
  if (flag & VLINK_CONFIGURED) {
    UPLL_LOG_DEBUG("flag & VLINK_CONFIGURED");
    flag_port_map |= SET_FLAG_VLINK;
  } else if (flag & PORTMAP_CONFIGURED) {
    UPLL_LOG_DEBUG("flag & PORTMAP_CONFIGURED");
    flag_port_map |= SET_FLAG_PORTMAP;
  } else if (flag & VLINK_PORTMAP_CONFIGURED) {
    UPLL_LOG_DEBUG("flag & PORTMAP_CONFIGURED");
    flag_port_map |= SET_FLAG_VLINK_PORTMAP;
  } else {
    UPLL_LOG_DEBUG("default flag_port_map");
    flag_port_map = 0;
//    return UPLL_RC_ERR_GENERIC;
  }
  uint8_t rename_flag = 0;
  GET_USER_DATA_FLAGS(ckv, rename_flag);
  rename_flag |= flag_port_map;
  SET_USER_DATA_FLAGS(ckv, flag_port_map);
  DbSubOp dbop_up = { kOpNotRead, kOpMatchNone, kOpInOutFlag };
  result_code = UpdateConfigDB(ckv, dt_type, UNC_OP_UPDATE,
      dmi, &dbop_up, MAINTBL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("UpdateConfigDB failure %d", result_code);
    return result_code;
  }

  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrIfPolicingMapMoMgr::GetVexternalInformation(ConfigKeyVal* ck_main,
  upll_keytype_datatype_t dt_type,
  pfcdrv_val_vbrif_policingmap_t *& pfc_val,
  pfcdrv_val_vbrif_vextif_t *&pfc_val_ext,
  uint8_t db_flag, DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_ERR_GENERIC;
  uint8_t* vexternal = reinterpret_cast<uint8_t*>
      (ConfigKeyVal::Malloc(kMaxLenVnodeName + 1));
  uint8_t* vex_if = reinterpret_cast<uint8_t*>
      (ConfigKeyVal::Malloc(kMaxLenInterfaceName + 1));
  InterfacePortMapInfo flags = kVlinkPortMapNotConfigured;
  ConfigKeyVal *ckv = NULL;
  if ((db_flag & SET_FLAG_PORTMAP) || (db_flag & SET_FLAG_VLINK_PORTMAP)) {
    UPLL_LOG_DEBUG("SET_FLAG_PORTMAP SET_FLAG_VLINK_PORTMAP");
    key_vbr_if_t * temp_key =
      reinterpret_cast<key_vbr_if_t*>(ck_main->get_key());

    VbrIfMoMgr *mgrvbrif =
      reinterpret_cast<VbrIfMoMgr *>(const_cast<MoManager *>(GetMoManager(
              UNC_KT_VBR_IF)));

    result_code = mgrvbrif->GetChildConfigKey(ckv, NULL);

    key_vbr_if_t *vbr_if_key = reinterpret_cast<key_vbr_if_t*>
      (ckv->get_key());

    uuu::upll_strncpy(vbr_if_key->vbr_key.vtn_key.vtn_name,
        temp_key->vbr_key.vtn_key.vtn_name,
        kMaxLenVtnName + 1);
    UPLL_LOG_DEBUG("vtn name from temp_key (%s)",
                   vbr_if_key->vbr_key.vtn_key.vtn_name);

    uuu::upll_strncpy(vbr_if_key->vbr_key.vbridge_name,
        temp_key->vbr_key.vbridge_name,
        kMaxLenVnodeName + 1);
    UPLL_LOG_DEBUG("vbr name from temp_key (%s)",
                   vbr_if_key->vbr_key.vbridge_name);

    uuu::upll_strncpy(vbr_if_key->if_name,
        temp_key->if_name,
        kMaxLenInterfaceName + 1);
    UPLL_LOG_DEBUG("vbrif name from temp_key (%s)", vbr_if_key->if_name);

    result_code = mgrvbrif->GetVexternal(ckv, dt_type, dmi,
        vexternal, vex_if, flags);
    if (UPLL_RC_SUCCESS != result_code) {
      free(vex_if);
      free(vexternal);
      delete ckv;
      return result_code;
    }

    uuu::upll_strncpy(pfc_val_ext->vexternal_name,
        vexternal,
        kMaxLenVnodeName + 1);
    pfc_val_ext->valid[PFCDRV_IDX_VEXTERNAL_NAME_VBRIF] = UNC_VF_VALID;
    UPLL_LOG_DEBUG("vexternal name (%s)", pfc_val_ext->vexternal_name);

    uuu::upll_strncpy(pfc_val_ext->vext_if_name,
        vex_if,
        kMaxLenInterfaceName + 1);
    pfc_val_ext->valid[PFCDRV_IDX_VEXT_IF_NAME_VBRIF] = UNC_VF_VALID;
    UPLL_LOG_DEBUG("vex_if (%s)", pfc_val_ext->vext_if_name);

    pfc_val_ext->valid[PFCDRV_IDX_INTERFACE_TYPE] = UNC_VF_VALID;
    pfc_val_ext->interface_type = PFCDRV_IDX_VAL_VBRIF_VEXTIF_PM;
  } else {
    UPLL_LOG_DEBUG("Not allowed at this time");
    free(vex_if);
    free(vexternal);
    return UPLL_RC_ERR_NOT_ALLOWED_AT_THIS_TIME;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrIfPolicingMapMoMgr::ConstructReadDetailResponse(
    ConfigKeyVal *ikey,
    ConfigKeyVal *drv_resp_ckv,
    upll_keytype_datatype_t dt_type,
    unc_keytype_operation_t op,
    DbSubOp dbop,
    DalDmlIntf *dmi,
    ConfigKeyVal **okey) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *tmp_okey = NULL;
  ConfigVal *temp_cfg_val = NULL;
  result_code =  GetChildConfigKey(tmp_okey, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetChildConfigKey failed err code (%d)", result_code);
    return result_code;
  }
  val_policingmap_t *val_policingmap =
    reinterpret_cast<val_policingmap_t *>(GetVal(ikey));
  val_policingmap_t *out_val_policingmap =
    reinterpret_cast<val_policingmap_t *>
    (ConfigKeyVal::Malloc(sizeof(val_policingmap_t)));
  memcpy(out_val_policingmap, val_policingmap, sizeof(val_policingmap_t));
  tmp_okey->AppendCfgVal(IpctSt::kIpcStValPolicingmap, out_val_policingmap);
  temp_cfg_val =  drv_resp_ckv->get_cfg_val();

  while (temp_cfg_val != NULL) {
    val_policingmap_controller_st *val_entry_st = NULL;
    if (IpctSt::kIpcStValPolicingmapControllerSt ==
        temp_cfg_val->get_st_num()) {
      val_entry_st = reinterpret_cast<val_policingmap_controller_st *>
        (temp_cfg_val->get_val());
    } else {
      UPLL_LOG_DEBUG("No val_entry_st (%d)", temp_cfg_val->get_st_num());
      delete tmp_okey;
      return UPLL_RC_ERR_GENERIC;
    }

    if (val_entry_st->valid[UPLL_IDX_SEQ_NUM_FFES] == UNC_VF_VALID) {
      UPLL_LOG_DEBUG("val_entry_st valid");
      ConfigKeyVal *tkey = NULL;

      key_policingprofile_entry_t *key_policingprofile_entry =
        reinterpret_cast<key_policingprofile_entry_t *>
        (ConfigKeyVal::Malloc(sizeof(key_policingprofile_entry_t)));

      tkey = new ConfigKeyVal(UNC_KT_POLICING_PROFILE_ENTRY,
          IpctSt::kIpcStKeyPolicingprofileEntry,
          key_policingprofile_entry, NULL);
      key_policingprofile_entry->sequence_num = val_entry_st->sequence_num;

      uuu::upll_strncpy(
          key_policingprofile_entry->policingprofile_key.policingprofile_name,
          val_policingmap->policer_name,
          (kMaxLenPolicingProfileName+1));
      PolicingProfileEntryMoMgr *mgr = reinterpret_cast
        <PolicingProfileEntryMoMgr*>
        (const_cast<MoManager *>(GetMoManager
                                 (UNC_KT_POLICING_PROFILE_ENTRY)));

      result_code = mgr->ReadDetailEntry(
          tkey, dt_type,  dbop, dmi);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("ReadDetailEntry error (%d)", result_code);
        delete tmp_okey;
        delete tkey;
        return result_code;
      }
      val_policingprofile_entry_t *out_val_ppe =
          reinterpret_cast<val_policingprofile_entry_t *>
          (ConfigKeyVal::Malloc(sizeof(val_policingprofile_entry_t)));
      val_policingprofile_entry_t *temp_val_policingprofile =
        reinterpret_cast<val_policingprofile_entry_t *>
        (tkey->get_cfg_val()->get_val());
      memcpy(out_val_ppe, temp_val_policingprofile,
          sizeof(val_policingprofile_entry_t));

      val_policingmap_controller_st *out_val_entry_st =
          reinterpret_cast<val_policingmap_controller_st *>
          (ConfigKeyVal::Malloc(sizeof(val_policingmap_controller_st)));
      memcpy(out_val_entry_st, val_entry_st,
          sizeof(val_policingmap_controller_st));
      tmp_okey->AppendCfgVal(IpctSt::kIpcStValPolicingmapControllerSt,
          out_val_entry_st);
      tmp_okey->AppendCfgVal(IpctSt::kIpcStValPolicingprofileEntry,
          out_val_ppe);
      if (tkey) {
        delete tkey;
      }

      if ((temp_cfg_val = temp_cfg_val->get_next_cfg_val()) == NULL) {
        UPLL_LOG_DEBUG("Next Vlaue structure is null\n");
        break;
      }


      if (IpctSt::kIpcStValFlowlistEntrySt == temp_cfg_val->get_st_num()) {
        while (IpctSt::kIpcStValPolicingmapSwitchSt ==
            temp_cfg_val->get_st_num()) {
          tmp_okey->AppendCfgVal(IpctSt::kIpcStValPolicingmapSwitchSt,
              temp_cfg_val->get_val());
          temp_cfg_val = temp_cfg_val->get_next_cfg_val();
          if (temp_cfg_val == NULL)
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

upll_rc_t VbrIfPolicingMapMoMgr::ReadDTStateNormal(
                  IpcReqRespHeader *req,
                  ConfigKeyVal *ikey,
                  DalDmlIntf *dmi) {
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *dup_key = NULL;
  DbSubOp dbop = {kOpReadSingle, kOpMatchNone, kOpInOutCtrlr};
  uint8_t *ctrlr_id = NULL;
  result_code = GetReadVbrIfKey(dup_key, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetReadVbrIfKey failed");
    return result_code;
  }
  result_code = ReadConfigDB(dup_key, req->datatype, UNC_OP_READ,
      dbop, dmi, MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetChildConfigKey failed");
    CONFIGKEYVALCLEAN(dup_key);
    return result_code;
  }
  val_policingmap_t *val_pm = NULL;
  val_pm = reinterpret_cast<val_policingmap_t*>(GetVal(dup_key));
  val_policingmap_t *out_pm_val = reinterpret_cast<val_policingmap_t*>
    (ConfigKeyVal::Malloc(sizeof(val_policingmap_t)));
  memcpy(out_pm_val, val_pm, sizeof(val_policingmap_t));

  ikey->AppendCfgVal(IpctSt::kIpcStValPolicingmap, out_pm_val);

  key_vbrif_policingmap_entry *key =
       reinterpret_cast<key_vbrif_policingmap_entry *>(ikey->get_key());


  PolicingProfileEntryMoMgr *mgr = reinterpret_cast<PolicingProfileEntryMoMgr*>
             (const_cast<MoManager *>(GetMoManager
             (UNC_KT_POLICING_PROFILE_ENTRY)));
  GET_USER_DATA_CTRLR(dup_key, ctrlr_id);
  ConfigKeyVal *ppe_ckv = NULL;
  result_code = mgr->ReadPolicingProfileEntry(reinterpret_cast
    <const char *>(val_pm->policer_name), key->sequence_num,
    reinterpret_cast<const char *>(ctrlr_id), dmi, req->datatype, ppe_ckv);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("ReadPolicingProfileEntry failed %d", result_code);
    return result_code;
  }
  val_policingprofile_entry_t *temp_val_policingprofile =
      reinterpret_cast<val_policingprofile_entry_t *>
      (GetVal(ppe_ckv));
  val_policingprofile_entry_t *out_temp_val_policingprofile =
      reinterpret_cast<val_policingprofile_entry_t *>
      (ConfigKeyVal::Malloc(sizeof(val_policingprofile_entry_t)));
  memcpy(out_temp_val_policingprofile, temp_val_policingprofile,
    sizeof(val_policingprofile_entry_t));
  ikey->AppendCfgVal(IpctSt::kIpcStValPolicingprofileEntry,
                       out_temp_val_policingprofile);
  return UPLL_RC_SUCCESS;
}


upll_rc_t VbrIfPolicingMapMoMgr:: ReadSiblingDTsateNormal(
                  ConfigKeyVal *ikey,
                  ConfigKeyVal* tctrl_key,
                  upll_keytype_datatype_t  dt_type,
                  DbSubOp dbop,
                  DalDmlIntf *dmi,
                  ConfigKeyVal **resp_key,
                  int count) {
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal* tkey = NULL;
  ConfigKeyVal* okey = NULL;
  val_policingmap_t* val_pm = NULL;

  result_code =  DupConfigKeyVal(okey, ikey, MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
     UPLL_LOG_DEBUG("DupConfigKeyVal Faill in  dup_key");
          return result_code;
  }

  val_pm = reinterpret_cast<val_policingmap_t*>(GetVal(tctrl_key));
  if (val_pm == NULL) {
    UPLL_LOG_DEBUG(" Invalid value");
    delete okey;
    return UPLL_RC_ERR_GENERIC;
  }
  val_policingmap_t *val_polmap =
    reinterpret_cast<val_policingmap_t *>
        (ConfigKeyVal::Malloc(sizeof(val_policingmap_t)));
  memcpy(val_polmap, val_pm, sizeof(val_policingmap_t));

  okey->AppendCfgVal(IpctSt::kIpcStValPolicingmap, val_polmap);

  key_vbrif_policingmap_entry *key =
       reinterpret_cast<key_vbrif_policingmap_entry *>(ikey->get_key());

  key_policingprofile_entry_t *key_policingprofile_entry =
     reinterpret_cast<key_policingprofile_entry_t *>
     (ConfigKeyVal::Malloc(sizeof(key_vbrif_policingmap_entry_t)));

  key_policingprofile_entry->sequence_num = key->sequence_num;

  uuu::upll_strncpy(
     key_policingprofile_entry->policingprofile_key.policingprofile_name,
                    val_pm->policer_name,
                    (kMaxLenPolicingProfileName+1));

  tkey = new ConfigKeyVal(UNC_KT_POLICING_PROFILE_ENTRY,
                          IpctSt::kIpcStKeyPolicingprofileEntry,
                          key_policingprofile_entry, NULL);

  PolicingProfileEntryMoMgr *mgr = reinterpret_cast<PolicingProfileEntryMoMgr*>
             (const_cast<MoManager *>(GetMoManager
             (UNC_KT_POLICING_PROFILE_ENTRY)));

  UPLL_LOG_DEBUG("Policer name and seq num (%s) (%d)",
        key_policingprofile_entry->policingprofile_key.policingprofile_name,
        key_policingprofile_entry->sequence_num);

  result_code = mgr->ReadDetailEntry(
                   tkey, dt_type,  dbop, dmi);
  if (result_code != UPLL_RC_SUCCESS) {
     UPLL_LOG_DEBUG("ReadDetailEntry failed");
     CONFIGKEYVALCLEAN(tkey);
     delete okey;
     return result_code;
  }
  if (GetVal(tkey)) {
    val_policingprofile_entry_t *temp_val_policingprofile =
        reinterpret_cast<val_policingprofile_entry_t *>
        (tkey->get_cfg_val()->get_val());
        val_policingprofile_entry_t* val_pp_entry =
        reinterpret_cast<val_policingprofile_entry_t*>
        (ConfigKeyVal::Malloc(sizeof(val_policingprofile_entry_t)));
        memcpy(val_pp_entry, temp_val_policingprofile,
                sizeof(val_policingprofile_entry_t));
    okey->AppendCfgVal(IpctSt::kIpcStValPolicingprofileEntry,
                         val_pp_entry);
  }

  if (!count) {
    UPLL_LOG_DEBUG("count is 0");
    *resp_key = okey;
  } else {
      UPLL_LOG_DEBUG("AppendCfgKeyVal in resp_key");
      (*resp_key)->AppendCfgKeyVal(okey);
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrIfPolicingMapMoMgr::ReadDTSiblingDetail(ConfigKeyVal *ikey,
                                         ConfigKeyVal *dup_key,
                                         IpcResponse *ipc_response,
                                         upll_keytype_datatype_t dt_type,
                                         unc_keytype_operation_t op,
                                         DbSubOp dbop,
                                         DalDmlIntf *dmi,
                                         int count,
                                         ConfigKeyVal** resp_key,
                                         ConfigKeyVal* tctrl_key) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigVal *temp_cfg_val = NULL;
  ConfigKeyVal *okey = NULL;
#if 0
  result_code =  DupConfigKeyVal(okey, ikey, MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
     UPLL_LOG_DEBUG("DupConfigKeyVal Faill in ReadMo for dup_key");
          return result_code;
  }
#endif
  result_code = GetReadVbrIfKey(okey, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
     UPLL_LOG_DEBUG("GetReadVbrIfKey Faill ");
          return result_code;
  }
  reinterpret_cast<key_vbrif_policingmap_entry*>
      (okey->get_key())->sequence_num =
      reinterpret_cast<key_vbrif_policingmap_entry*>
      (tctrl_key->get_key())->sequence_num;

  val_policingmap_t *val_policingmap =
      reinterpret_cast<val_policingmap_t *>(GetVal(dup_key));
  val_policingmap_t *val_polmap =
    reinterpret_cast<val_policingmap_t *>
        (ConfigKeyVal::Malloc(sizeof(val_policingmap_t)));
  memcpy(val_polmap, val_policingmap, sizeof(val_policingmap_t));
  okey->AppendCfgVal(IpctSt::kIpcStValPolicingmap, val_polmap);

  if (ipc_response->ckv_data)
    temp_cfg_val =  ipc_response->ckv_data->get_cfg_val();

  while (temp_cfg_val != NULL) {
     val_policingmap_controller_st *val_entry_st = NULL;
    if (IpctSt::kIpcStValPolicingmapControllerSt ==
           temp_cfg_val->get_st_num()) {
      val_entry_st = reinterpret_cast<val_policingmap_controller_st *>
          (temp_cfg_val->get_val());
    } else {
      UPLL_LOG_DEBUG("No val_entry_st (%d)", temp_cfg_val->get_st_num());
      delete okey;
      return UPLL_RC_ERR_GENERIC;
    }

    if (val_entry_st->valid[UPLL_IDX_SEQ_NUM_FFES] == UNC_VF_VALID) {
      ConfigKeyVal *tkey = NULL;

      key_policingprofile_entry_t *key_policingprofile_entry =
        reinterpret_cast<key_policingprofile_entry_t *>
        (ConfigKeyVal::Malloc(sizeof(key_vbrif_policingmap_entry_t)));

      tkey = new ConfigKeyVal(UNC_KT_POLICING_PROFILE_ENTRY,
                              IpctSt::kIpcStKeyPolicingprofileEntry,
                              key_policingprofile_entry, NULL);
      key_policingprofile_entry->sequence_num = val_entry_st->sequence_num;

      uuu::upll_strncpy(
           key_policingprofile_entry->policingprofile_key.policingprofile_name,
           val_policingmap->policer_name,
           (kMaxLenPolicingProfileName+1));
      PolicingProfileEntryMoMgr *mgr = reinterpret_cast
                <PolicingProfileEntryMoMgr*>
                (const_cast<MoManager *>(GetMoManager
                (UNC_KT_POLICING_PROFILE_ENTRY)));

      result_code = mgr->ReadDetailEntry(
                    tkey, dt_type,  dbop, dmi);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("ReadDetailEntry error (%d)", result_code);
        delete okey;
        delete val_polmap;
        delete tkey;
        return result_code;
      }
      val_policingprofile_entry_t *temp_val_policingprofile =
          reinterpret_cast<val_policingprofile_entry_t *>
          (tkey->get_cfg_val()->get_val());
      val_policingmap_controller_st* val_pol_st =
        reinterpret_cast<val_policingmap_controller_st *>
        (ConfigKeyVal::Malloc(sizeof(val_policingmap_controller_st)));
      memcpy(val_pol_st, val_entry_st, sizeof(val_policingmap_controller_st));
      okey->AppendCfgVal(IpctSt::kIpcStValPolicingmapControllerSt,
                         val_pol_st);
      okey->AppendCfgVal(IpctSt::kIpcStValPolicingprofileEntry,
                         temp_val_policingprofile);

     if ((temp_cfg_val = temp_cfg_val->get_next_cfg_val()) == NULL) {
         UPLL_LOG_DEBUG("Next Vlaue structure is null\n");
         break;
     }

      while (IpctSt::kIpcStValPolicingmapSwitchSt ==
             temp_cfg_val->get_st_num()) {
        val_policingmap_switch_st* val_polswitch = reinterpret_cast
            <val_policingmap_switch_st*>(temp_cfg_val->get_val());
        val_policingmap_switch_st* val_polswitch_st =
            reinterpret_cast<val_policingmap_switch_st *>
            (ConfigKeyVal::Malloc(sizeof(val_policingmap_switch_st)));
        memcpy(val_polswitch_st, val_polswitch,
               sizeof(val_policingmap_switch_st));
        okey->AppendCfgVal(IpctSt::kIpcStValPolicingmapSwitchSt,
                           val_polswitch_st);
        temp_cfg_val = temp_cfg_val->get_next_cfg_val();
        if (temp_cfg_val == NULL)
          break;
      }
    }
  }
  if (!count) {
    UPLL_LOG_DEBUG("count is 0");
    *resp_key = okey;
  } else {
      UPLL_LOG_DEBUG("AppendCfgKeyVal in resp_key");
      (*resp_key)->AppendCfgKeyVal(okey);
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrIfPolicingMapMoMgr::ConstructReadEntryDetailResponse(
    ConfigKeyVal *ikey,
    ConfigKeyVal *drv_resp_ckv,
    upll_keytype_datatype_t dt_type,
    unc_keytype_operation_t op,
    DbSubOp dbop,
    DalDmlIntf *dmi,
    ConfigKeyVal **okey) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *tmp_okey = NULL;
  ConfigVal *temp_cfg_val = NULL;
  result_code =  GetChildEntryConfigKey(tmp_okey, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetChildConfigKey failed err code (%d)", result_code);
    return result_code;
  }
  val_policingmap_t *val_policingmap =
    reinterpret_cast<val_policingmap_t *>(GetVal(ikey));
  val_policingmap_t *out_val_policingmap =
      reinterpret_cast<val_policingmap_t *>
      (ConfigKeyVal::Malloc(sizeof(val_policingmap_t)));
  memcpy(out_val_policingmap, val_policingmap, sizeof(val_policingmap_t));
  tmp_okey->AppendCfgVal(IpctSt::kIpcStValPolicingmap, out_val_policingmap);
  temp_cfg_val =  drv_resp_ckv->get_cfg_val();

  while (temp_cfg_val != NULL) {
    val_policingmap_controller_st *val_entry_st = NULL;
    if (IpctSt::kIpcStValPolicingmapControllerSt ==
        temp_cfg_val->get_st_num()) {
      val_entry_st = reinterpret_cast<val_policingmap_controller_st *>
        (temp_cfg_val->get_val());
    } else {
      UPLL_LOG_DEBUG("No val_entry_st (%d)", temp_cfg_val->get_st_num());
      delete tmp_okey;
      return UPLL_RC_ERR_GENERIC;
    }

    if (val_entry_st->valid[UPLL_IDX_SEQ_NUM_FFES] == UNC_VF_VALID) {
      UPLL_LOG_DEBUG("val_entry_st valid");
      ConfigKeyVal *tkey = NULL;

      key_policingprofile_entry_t *key_policingprofile_entry =
        reinterpret_cast<key_policingprofile_entry_t *>
        (ConfigKeyVal::Malloc(sizeof(key_vbrif_policingmap_entry_t)));

      tkey = new ConfigKeyVal(UNC_KT_POLICING_PROFILE_ENTRY,
          IpctSt::kIpcStKeyPolicingprofileEntry,
          key_policingprofile_entry, NULL);
      key_policingprofile_entry->sequence_num = val_entry_st->sequence_num;

      uuu::upll_strncpy(
          key_policingprofile_entry->policingprofile_key.policingprofile_name,
          val_policingmap->policer_name,
          (kMaxLenPolicingProfileName+1));
      PolicingProfileEntryMoMgr *mgr = reinterpret_cast
        <PolicingProfileEntryMoMgr*>
        (const_cast<MoManager *>(GetMoManager
                                 (UNC_KT_POLICING_PROFILE_ENTRY)));

      result_code = mgr->ReadDetailEntry(
          tkey, dt_type,  dbop, dmi);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("ReadDetailEntry error (%d)", result_code);
        delete tmp_okey;
        delete tkey;
        return result_code;
      }
      val_policingprofile_entry_t *out_val_ppe =
          reinterpret_cast<val_policingprofile_entry_t *>
          (ConfigKeyVal::Malloc(sizeof(val_policingprofile_entry_t)));
      val_policingprofile_entry_t *temp_val_policingprofile =
        reinterpret_cast<val_policingprofile_entry_t *>
        (tkey->get_cfg_val()->get_val());
      memcpy(out_val_ppe, temp_val_policingprofile,
          sizeof(val_policingprofile_entry_t));

      val_policingmap_controller_st *out_val_entry_st =
          reinterpret_cast<val_policingmap_controller_st *>
          (ConfigKeyVal::Malloc(sizeof(val_policingmap_controller_st)));
      memcpy(out_val_entry_st, val_entry_st,
             sizeof(val_policingmap_controller_st));
      tmp_okey->AppendCfgVal(IpctSt::kIpcStValPolicingmapControllerSt,
          out_val_entry_st);
      tmp_okey->AppendCfgVal(IpctSt::kIpcStValPolicingprofileEntry,
          out_val_ppe);
      if (tkey) {
        delete tkey;
      }

      if ((temp_cfg_val = temp_cfg_val->get_next_cfg_val()) == NULL) {
        UPLL_LOG_DEBUG("Next Vlaue structure is null\n");
        break;
      }

      if (IpctSt::kIpcStValFlowlistEntrySt == temp_cfg_val->get_st_num()) {
        while (IpctSt::kIpcStValPolicingmapSwitchSt ==
            temp_cfg_val->get_st_num()) {
          tmp_okey->AppendCfgVal(IpctSt::kIpcStValPolicingmapSwitchSt,
              temp_cfg_val->get_val());
          temp_cfg_val = temp_cfg_val->get_next_cfg_val();
          if (temp_cfg_val == NULL)
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


upll_rc_t VbrIfPolicingMapMoMgr::ReadEntryDetailRecord(IpcReqRespHeader *req,
                                               ConfigKeyVal *ikey,
                                               DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_ERR_GENERIC;
  ConfigKeyVal *l_key = NULL;
  ConfigKeyVal *dup_key = NULL;
  controller_domain ctrlr_dom;
  uint8_t db_flag = 0;
  DbSubOp dbop = { kOpReadSingle, kOpMatchNone,
    kOpInOutCtrlr|kOpInOutDomain|kOpInOutFlag };
  // On Thisdup key we r applying the Readsibling op, that gives dir
  ConfigKeyVal *temp_key = NULL;
  memset(&ctrlr_dom, 0, sizeof(controller_domain));
  result_code =  GetReadVbrIfKey(temp_key, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetReadVbrIfKey dup_key Err  (%d)", result_code);
    return result_code;
  }
  key_vbrif_policingmap_entry_t *in_key = reinterpret_cast
      <key_vbrif_policingmap_entry_t *>(ikey->get_key());

  result_code = GetChildEntryConfigKey(dup_key, ikey);

  result_code = ReadConfigDB(temp_key, req->datatype, UNC_OP_READ,
                             dbop, dmi, MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("ReadConfigDB Error  (%d)", result_code);
    CONFIGKEYVALCLEAN(dup_key);
    delete temp_key;
    return result_code;
  }
  val_policingmap_t *val_policingmap =
      reinterpret_cast<val_policingmap_t *>(GetVal(temp_key));
  val_policingmap_t *out_val_policingmap =
      reinterpret_cast<val_policingmap_t *>
      (ConfigKeyVal::Malloc(sizeof(val_policingmap_t)));
  memcpy(out_val_policingmap, val_policingmap,
         sizeof(val_policingmap_t));
  ConfigVal *cval = new ConfigVal(IpctSt::kIpcStValPolicingmap,
                                  out_val_policingmap);
  dup_key->AppendCfgVal(cval);
  key_policingprofile_entry_t *key_ppe =
      reinterpret_cast<key_policingprofile_entry_t *>
      (ConfigKeyVal::Malloc(sizeof(key_policingprofile_entry_t)));

  ConfigKeyVal *ppe_ckv = new ConfigKeyVal(
      UNC_KT_POLICING_PROFILE_ENTRY,
      IpctSt::kIpcStKeyPolicingprofileEntry,
      key_ppe, NULL);
  key_ppe->sequence_num = in_key->sequence_num;

  uuu::upll_strncpy(key_ppe->policingprofile_key.policingprofile_name,
                    val_policingmap->policer_name,
                    (kMaxLenPolicingProfileName+1));

  PolicingProfileEntryMoMgr *mgr = reinterpret_cast
      <PolicingProfileEntryMoMgr*>
      (const_cast<MoManager *>(GetMoManager
                               (UNC_KT_POLICING_PROFILE_ENTRY)));

  result_code = mgr->UpdateConfigDB(ppe_ckv, req->datatype, UNC_OP_READ, dmi,
                                    MAINTBL);
  if (result_code != UPLL_RC_ERR_INSTANCE_EXISTS) {
    UPLL_LOG_DEBUG("ReadDetailEntry error (%d)", result_code);
    return result_code;
  }

  result_code =  GetChildEntryConfigKey(l_key, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("DupConfigKeyVal Faill in ReadSiblingMo for l_key");
    CONFIGKEYVALCLEAN(dup_key);
    return result_code;
  }
  GET_USER_DATA_CTRLR_DOMAIN(temp_key, ctrlr_dom);
  SET_USER_DATA_CTRLR_DOMAIN(l_key, ctrlr_dom);

  GET_USER_DATA_FLAGS(temp_key, db_flag);
  /*
  result_code = GetRenamedControllerKey(l_key, req->datatype,
                                        dmi, &ctrlr_dom);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetRenamedControllerKey Faill");
    CONFIGKEYVALCLEAN(dup_key);
    CONFIGKEYVALCLEAN(l_key);
    return result_code;
  }
  */
  pfcdrv_val_vbrif_policingmap *pfc_val =
      reinterpret_cast<pfcdrv_val_vbrif_policingmap *>
      (ConfigKeyVal::Malloc(sizeof(pfcdrv_val_vbrif_policingmap_t)));
  pfcdrv_val_vbrif_vextif *pfc_val_ext =
      reinterpret_cast<pfcdrv_val_vbrif_vextif *>
      (ConfigKeyVal::Malloc(sizeof(pfcdrv_val_vbrif_vextif_t)));
  memset(pfc_val_ext, 0, sizeof(pfcdrv_val_vbrif_vextif_t));

  UPLL_LOG_DEBUG("GetVexternalInformation (%d)", req->datatype);
  if (req->datatype == UPLL_DT_STATE) req->datatype = UPLL_DT_RUNNING;
  string s(l_key->ToStrAll());
  result_code = GetVexternalInformation(temp_key, req->datatype, pfc_val,
                                        pfc_val_ext, db_flag, dmi);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetVexternalInformation fail");
    CONFIGKEYVALCLEAN(dup_key);
    CONFIGKEYVALCLEAN(l_key);
    return result_code;
  }

  val_policingmap_t* val = reinterpret_cast<val_policingmap_t *>
      (GetVal(dup_key));
  UPLL_LOG_DEBUG("val_policingmap_t (%s)", val->policer_name);

  pfc_val->valid[PFCDRV_IDX_VAL_POLICINGMAP_PM] = UNC_VF_VALID;
  memcpy(&pfc_val->val_policing_map, val, sizeof(val_policingmap_t));

  pfc_val->valid[PFCDRV_IDX_VAL_VBRIF_VEXTIF_PM] = UNC_VF_VALID;
  memcpy(&pfc_val->val_vbrif_vextif, pfc_val_ext,
         sizeof(pfcdrv_val_vbrif_vextif_t));
  l_key->SetCfgVal(new ConfigVal(IpctSt::kIpcStPfcdrvValVbrifPolicingmap,
                                 pfc_val));

  IpcResponse ipc_response;
  memset(&ipc_response, 0, sizeof(IpcResponse));
  IpcRequest ipc_req;
  memset(&ipc_req, 0, sizeof(ipc_req));
  ipc_req.header.clnt_sess_id = req->clnt_sess_id;
  ipc_req.header.config_id = req->config_id;
  ipc_req.header.operation = UNC_OP_READ;
  ipc_req.header.option1 = req->option1;
  ipc_req.header.datatype = UPLL_DT_STATE;
  ipc_req.ckv_data = l_key;
  if (!uui::IpcUtil::SendReqToDriver(
          (const char *)ctrlr_dom.ctrlr,
          reinterpret_cast<char *>(ctrlr_dom.domain),
          PFCDRIVER_SERVICE_NAME, PFCDRIVER_SVID_LOGICAL, &ipc_req,
          true, &ipc_response)) {
    UPLL_LOG_DEBUG("SendReqToDriver failed for Key %d controller %s",
                   l_key->get_key_type(),
                   reinterpret_cast<char *>(ctrlr_dom.ctrlr));
    CONFIGKEYVALCLEAN(l_key);
    CONFIGKEYVALCLEAN(dup_key);
    return UPLL_RC_ERR_GENERIC;
  }

  if (ipc_response.header.result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Driver response for Key %d controller %s result %d",
                   l_key->get_key_type(), ctrlr_dom.ctrlr,
                   ipc_response.header.result_code);
    CONFIGKEYVALCLEAN(l_key);
    CONFIGKEYVALCLEAN(dup_key);
    return ipc_response.header.result_code;
  }
  ConfigKeyVal *okey = NULL;
  result_code = ConstructReadEntryDetailResponse(dup_key, ipc_response.ckv_data,
                                                 req->datatype, req->operation,
                                                 dbop, dmi, &okey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("ReadSiblingDetail Error  (%d)", result_code);
    CONFIGKEYVALCLEAN(dup_key);
    CONFIGKEYVALCLEAN(l_key);
    return result_code;
  } else {
    if (okey != NULL) {
      ikey->ResetWith(okey);
    }
  }
  return result_code;
}

upll_rc_t VbrIfPolicingMapMoMgr::ReadSiblingCount(IpcReqRespHeader *req,
    ConfigKeyVal* ikey,
    DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code;
  if (UNC_KT_VBRIF_POLICINGMAP_ENTRY != ikey->get_key_type() &&
      req->datatype != UPLL_DT_STATE) {
    controller_domain ctrlr_dom;
    result_code = ValidateMessage(req, ikey);
    if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("ValidateMessage failed result_code %d",
                      result_code);
        return result_code;
    }
    result_code = ReadInfoFromDB(req, ikey, dmi, &ctrlr_dom);
    return result_code;
  }
  ConfigKeyVal *temp_vbr_if_key = NULL;
  key_vbrif_policingmap_entry_t *vbrif_entry_key = reinterpret_cast
    <key_vbrif_policingmap_entry_t *>(ikey->get_key());
  result_code = GetReadVbrIfKey(temp_vbr_if_key, ikey);
  DbSubOp dbop = {kOpReadSingle, kOpMatchNone, kOpInOutNone};
  result_code = ReadConfigDB(temp_vbr_if_key, req->datatype, UNC_OP_READ,
      dbop, dmi, MAINTBL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("ReadConfigDB failed %d", result_code);
    delete temp_vbr_if_key;
    return result_code;
  }
  val_policingmap_t *val_pm = reinterpret_cast
    <val_policingmap_t *>(GetVal(temp_vbr_if_key));
  ConfigKeyVal *ppe_ckv = NULL;
  result_code = ConstructPpeCkv(ppe_ckv, reinterpret_cast
      <const char *>(val_pm->policer_name),
      vbrif_entry_key->sequence_num);
  PolicingProfileEntryMoMgr *mgr = reinterpret_cast
    <PolicingProfileEntryMoMgr*>
    (const_cast<MoManager *>(GetMoManager
                             (UNC_KT_POLICING_PROFILE_ENTRY)));
  if (!mgr) {
    return UPLL_RC_ERR_GENERIC;
  }
  IpcReqRespHeader *temp_req = reinterpret_cast<IpcReqRespHeader *>
    (ConfigKeyVal::Malloc(sizeof(IpcReqRespHeader)));
  memcpy(temp_req, req, sizeof(IpcReqRespHeader));
  temp_req->option1 = UNC_OPT1_NORMAL;
  temp_req->operation = UNC_OP_READ_SIBLING_BEGIN;
  // result_code = mgr->ReadInfoFromDB(req, ppe_ckv, dmi, &ctrlr_dom);
  result_code = mgr->ReadSiblingMo(temp_req, ppe_ckv, dmi);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Read sibling of ppe failed (%d)", result_code);
    return result_code;
  }
  ConfigKeyVal *temp_ppe_ckv = ppe_ckv;
  uint8_t sibling_count = 0;
  while (temp_ppe_ckv !=NULL) {
      sibling_count++;
      temp_ppe_ckv = temp_ppe_ckv->get_next_cfg_key_val();
  }
  uint32_t *sib_count =
      reinterpret_cast<uint32_t*>(ConfigKeyVal::Malloc(sizeof(uint32_t)));
  *sib_count = sibling_count;
  ikey->SetCfgVal(new ConfigVal(IpctSt::kIpcStUint32, sib_count));
  delete ppe_ckv;
  return UPLL_RC_SUCCESS;
}
}  // kt_momgr
}  // upll
}  // unc
