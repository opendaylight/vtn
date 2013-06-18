/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include "vbr_policingmap_momgr.hh"
#include "policingprofile_momgr.hh"
#include "policingprofile_entry_momgr.hh"
#include "unc/upll_errno.h"
#include "upll_validation.hh"
#include "unc/upll_ipc_enum.h"
#include "vtn_policingmap_momgr.hh"
#include "vbr_momgr.hh"
#include "vtn_momgr.hh"
using unc::upll::ipc_util::IpcUtil;
namespace unc {
namespace upll {
namespace kt_momgr {

#define VBR_KEY_COL    5
#define POLICY_KEY_COL 5
#define VTN_RENAME 0x01
#define VBR_RENAME 0x10
#define POLICINGPROFILE_RENAME 0x04

BindInfo VbrPolicingMapMoMgr::vbr_policingmap_bind_info[] = {
  { uudst::vbr_policingmap::kDbiVtnName, CFG_KEY,
    offsetof(key_vbr_t, vtn_key.vtn_name),
    uud::kDalChar, (kMaxLenVtnName + 1) },
  { uudst::vbr_policingmap::kDbiVbrName, CFG_KEY,
    offsetof(key_vbr_t, vbridge_name),
    uud::kDalChar, (kMaxLenVnodeName + 1) },
  { uudst::vbr_policingmap::kDbiCtrlrname, CK_VAL,
    offsetof(key_user_data, ctrlr_id),
    uud::kDalChar, (kMaxLenVnodeName + 1) },
  { uudst::vbr_policingmap::kDbiDomainId, CK_VAL,
    offsetof(key_user_data, domain_id),
    uud::kDalChar, (kMaxLenDomainId + 1) },
  { uudst::vbr_policingmap::kDbiPolicername, CFG_VAL,
    offsetof(val_policingmap_t, policer_name),
    uud::kDalChar, (kMaxLenPolicingProfileName + 1) },
  { uudst::vbr_policingmap::kDbiFlags, CK_VAL,
    offsetof(key_user_data_t, flags),
    uud::kDalUint8, 1 },
  { uudst::vbr_policingmap::kDbiValidPolicername, CFG_META_VAL,
    offsetof(val_policingmap_t, valid[0]),
    uud::kDalUint8, 1 },
  { uudst::vbr_policingmap::kDbiCsRowStatus, CS_VAL,
    offsetof(val_policingmap_t, cs_row_status),
    uud::kDalUint8, 1 },
  { uudst::vbr_policingmap::kDbiCsPolicername, CS_VAL,
    offsetof(val_policingmap_t, cs_attr[0]),
    uud::kDalUint8, 1 }
};

// Rename
BindInfo VbrPolicingMapMoMgr::key_vbrpm_maintbl_rename_bind_info[] = {
  { uudst::vbr_policingmap::kDbiVtnName, CFG_MATCH_KEY,
    offsetof(key_vbr_if_t, vbr_key.vtn_key.vtn_name),
    uud::kDalChar, (kMaxLenVtnName + 1) },
  { uudst::vbr_policingmap::kDbiVbrName, CFG_MATCH_KEY,
    offsetof(key_vbr_if_t, vbr_key.vbridge_name),
    uud::kDalChar, (kMaxLenVnodeName + 1) },
  { uudst::vbr_policingmap::kDbiVtnName, CFG_INPUT_KEY,
    offsetof(key_rename_vnode_info_t, new_unc_vtn_name),
    uud::kDalChar, (kMaxLenVtnName + 1) },
  { uudst::vbr_policingmap::kDbiVbrName, CFG_INPUT_KEY,
    offsetof(key_rename_vnode_info_t, new_unc_vnode_name),
    uud::kDalChar, (kMaxLenVnodeName + 1) },
  { uudst::vbr_policingmap::kDbiFlags, CFG_INPUT_KEY,
    offsetof(key_user_data_t, flags),
    uud::kDalUint8, 1 }
};

// Rename
BindInfo VbrPolicingMapMoMgr::key_vbrpm_policyname_maintbl_rename_bind_info[]
= {
  { uudst::vbr_policingmap::kDbiVtnName, CFG_MATCH_KEY,
    offsetof(key_vbr_if_t, vbr_key.vtn_key.vtn_name),
    uud::kDalChar, (kMaxLenVtnName + 1) },
  { uudst::vbr_policingmap::kDbiVbrName, CFG_MATCH_KEY,
    offsetof(key_vbr_if_t, vbr_key.vbridge_name),
    uud::kDalChar, (kMaxLenVnodeName + 1) },
  { uudst::vbr_policingmap::kDbiPolicername, CFG_INPUT_KEY,
    offsetof(key_rename_vnode_info_t, new_policingprofile_name),
    uud::kDalChar, (kMaxLenPolicingProfileName + 1) },
  { uudst::vbr_policingmap::kDbiFlags, CFG_INPUT_KEY,
    offsetof(key_user_data_t, flags),
    uud::kDalUint8, 1 }
};

// Constructor
VbrPolicingMapMoMgr::VbrPolicingMapMoMgr() : MoMgrImpl() {
  UPLL_FUNC_TRACE;
  // Rename and ctrlr tables not required for this KT
  // setting max tables to 1
  ntable = MAX_MOMGR_TBLS;
  table = new Table *[ntable];

  table[MAINTBL] = new Table(uudst::kDbiVbrPolicingMapTbl,
      UNC_KT_VBR_POLICINGMAP, vbr_policingmap_bind_info,
      IpctSt::kIpcStKeyVbr, IpctSt::kIpcStValPolicingmap,
      uudst::vbr_policingmap::kDbiVbrPolicingMapNumCols);

  /* For Rename Table*/
  table[CTRLRTBL] = NULL;

  /* For Rename Table*/
  table[RENAMETBL] = NULL;

  nchild = 0;
  child = NULL;
}

upll_rc_t VbrPolicingMapMoMgr::CreateCandidateMo(IpcReqRespHeader *req,
                                                 ConfigKeyVal *ikey,
                                                 DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  if (NULL == ikey || NULL == req || NULL == dmi) {
    UPLL_LOG_DEBUG(" CreateCandidateMo Failed. Insufficient input parameters");
    return UPLL_RC_ERR_GENERIC;
  }

  upll_rc_t result_code = UPLL_RC_SUCCESS;

  // validate syntax and semantics
  result_code = ValidateMessage(req, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("ValidateMessage Err (%d)", result_code);
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
  /*
  // Capability Check
  result_code = ValidateCapability(req, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Key not supported by controller");
    return result_code;
  } */

  // Check VBR object existence in VbrPolicingMap CANDIDATE DB
  // if record exists, return the error code
  result_code = IsReferenced(ikey, req->datatype, dmi);
  if (result_code == UPLL_RC_ERR_INSTANCE_EXISTS) {
    UPLL_LOG_DEBUG("VBR Object exists in CANDIDATE DB (%d)",
                   result_code);
    return result_code;
  } else if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    UPLL_LOG_DEBUG("VBR Instance not Available");
  } else if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("CreateCandidateMo Error Accesing CANDIDATE DB (%d)",
                   result_code);
    return result_code;
  }
#if 0
  ConfigKeyVal *tmpckv = NULL;
  result_code = DupConfigKeyVal(tmpckv, ikey, MAINTBL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetChildConfigKey failed");
    return result_code;
  }
#endif
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

  // 1)Get vbr associated ctrlr name and invoke the PP and PPE functions to
  // check the refcount capability and update the refcount or create the
  // record in policingprofilectrltbl and policingprofileentryctrltbl.
  result_code = UpdateRefCountInPPCtrlr(ikey, req->datatype, dmi,
                                        UNC_OP_CREATE);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("UpdateRefCountInPPCtrlr Err in CANDIDATE DB(%d)",
                   result_code);
    return result_code;
  }
  }


  UPLL_LOG_DEBUG("pyn Create  - (%s)", val_pm->policer_name);
  //  create a record in vbrpolicingmap table CANDIDATE DB
  result_code = UpdateConfigDB(ikey, req->datatype, UNC_OP_CREATE, dmi);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("Create Record failed - %d", result_code);
    return result_code;
  }
  UPLL_LOG_DEBUG("CreateCandidateMo Successful");
  return result_code;
}

upll_rc_t VbrPolicingMapMoMgr::DeleteMo(IpcReqRespHeader *req,
                                        ConfigKeyVal *ikey, DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_ERR_GENERIC;

  if (NULL == ikey || NULL == req || NULL == dmi) {
    UPLL_LOG_DEBUG(" DeleteMo Failed. Insufficient input parameters");
    return result_code;
  }

  // validate syntax and semantics
  result_code = ValidateMessage(req, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("ValidateMessage Err (%d)", result_code);
    return result_code;
  }

  // Check VBR object existence in VbrPolicingMap CANDIDATE DB
  // If record not exists, return error
  result_code = IsReferenced(ikey, req->datatype, dmi);
  if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    UPLL_LOG_DEBUG("VBR Instance not Available(%d)",
                   result_code);
    return result_code;
  } else if (result_code == UPLL_RC_ERR_INSTANCE_EXISTS) {
    UPLL_LOG_DEBUG("VBR Instance Available");
  } else if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("IsReferenced Error accessing DB (%d)", result_code);
    return result_code;
  }

  // 1)Get vbr associated ctrlr name and invoke the PP and PPE functions to
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
  delete okey;
  // Delete the record in vbrpolicingmap table
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

upll_rc_t VbrPolicingMapMoMgr::UpdateMo(IpcReqRespHeader *req,
                                        ConfigKeyVal *ikey, DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  if (NULL == ikey || NULL == req || NULL == dmi) {
    UPLL_LOG_DEBUG("UpdateMo ikey and req NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  upll_rc_t result_code = UPLL_RC_SUCCESS;

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

  // Check VBR object existence in VbrPolicingMap CANDIDATE DB
  // If record not exists, return error
  result_code = IsReferenced(ikey, req->datatype, dmi);
  if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    UPLL_LOG_DEBUG("IsReferenced record not available (%d)", result_code);
    return result_code;
  } else if (result_code == UPLL_RC_ERR_INSTANCE_EXISTS) {
    UPLL_LOG_DEBUG("IsReferenced record available");
  } else if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("IsReferenced error accessing DB (%d)", result_code);
    return result_code;
  }

  // Check vbr Policingmap object exists in PolicingProfileTbl CANDIDATE DB
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
  // 1)Get vbr associated ctrlr name and invoke the PP and PPE functions to
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
  UPLL_LOG_DEBUG("VtnPolicingMapMoMgr::UpdateMo update record status (%d)",
                 result_code);
  return result_code;
}

upll_rc_t VbrPolicingMapMoMgr::IsReferenced(ConfigKeyVal *ikey,
                                            upll_keytype_datatype_t dt_type,
                                            DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  if (NULL == ikey) return UPLL_RC_ERR_GENERIC;
  DbSubOp dbop = { kOpReadExist, kOpMatchNone, kOpInOutNone };
  // Check the VBR PM object existence
  result_code = UpdateConfigDB(ikey, dt_type, UNC_OP_READ, dmi, &dbop, MAINTBL);
  UPLL_LOG_DEBUG("IsReferenced status (%d)", result_code);
  return result_code;
}

upll_rc_t VbrPolicingMapMoMgr::IsPolicyProfileReferenced(
    ConfigKeyVal *ikey, upll_keytype_datatype_t dt_type, DalDmlIntf* dmi,
    unc_keytype_operation_t op) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *okey = NULL;

  // Get the memory allocated policy profile key structure
  MoMgrImpl *mgr =
      static_cast<MoMgrImpl *>((const_cast<MoManager*>(GetMoManager(
          UNC_KT_POLICING_PROFILE))));
  result_code = mgr->GetChildConfigKey(okey, NULL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("IsPolicyProfileReferenced GetChildConfigKey (%d)",
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


  // Check the policingprofile object in policingprofiletbl
  result_code = mgr->UpdateConfigDB(okey, dt_type, UNC_OP_READ, dmi, MAINTBL);
  UPLL_LOG_DEBUG("IsPolicyProfileReferenced UpdateConfigDB status (%d)",
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

upll_rc_t VbrPolicingMapMoMgr::UpdateRefCountInPPCtrlr(
    ConfigKeyVal *ikey, upll_keytype_datatype_t dt_type, DalDmlIntf *dmi,
    unc_keytype_operation_t op) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *okey = NULL;
  uint8_t *ctrlr_id = NULL;

  GET_USER_DATA_CTRLR(ikey, ctrlr_id);

  PolicingProfileMoMgr *pp_mgr =
      reinterpret_cast<PolicingProfileMoMgr *>
      (const_cast<MoManager *>(GetMoManager(
          UNC_KT_POLICING_PROFILE)));
  #if 0
  PolicingProfileEntryMoMgr *ppe_mgr =
      reinterpret_cast<PolicingProfileEntryMoMgr *>
      (const_cast<MoManager *>(GetMoManager(
          UNC_KT_POLICING_PROFILE_ENTRY)));
  #endif
  val_policingmap_t* val_vtn_policingmap =
      reinterpret_cast<val_policingmap_t *>(GetVal(ikey));

  if (NULL == ctrlr_id || NULL == pp_mgr /*|| NULL == ppe_mgr*/) {
    UPLL_LOG_DEBUG(
        "VbrPolicingMapMoMgr::UpdateRefCountInPPCtrlr ctrlr_id NULL (%d)",
        result_code);
    CONFIGKEYVALCLEAN(okey);
    return UPLL_RC_ERR_GENERIC;
  }

/* Check the ref count capability of vbr associated controller name in
   policingprofilectrltbl If refcount reached the
   max capability, then return error.
   If ref count is less than capability and the record exist in
   policingprofilectrltbl, based on the operation refcount should be
   incremented or decremented and update policingprofilectrltbl.
   create the record with refcount if record not
   exist in policingprofilectrl tbl for the vbr associated controller name
*/
  result_code = pp_mgr->PolicingProfileCtrlrTblOper(
      reinterpret_cast<const char *>(val_vtn_policingmap->policer_name),
      reinterpret_cast<const char *>(ctrlr_id), dmi, op, dt_type);
  if (UPLL_RC_SUCCESS != result_code) {
    CONFIGKEYVALCLEAN(okey);
    return result_code;
  }

  // Create/Delete/Update the record in policingprofileentryctrltbl
  // based on oper
  #if 0
  result_code = ppe_mgr->PolicingProfileEntryCtrlrTblOper(
                reinterpret_cast<const char*>
                (val_vtn_policingmap->policer_name),
                reinterpret_cast<const char*>(ctrlr_id), dmi, op, dt_type);
  if (UPLL_RC_SUCCESS != result_code) {
    CONFIGKEYVALCLEAN(okey);
    return result_code;
  }
  #endif
  CONFIGKEYVALCLEAN(okey);
  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrPolicingMapMoMgr::GetControllerId(ConfigKeyVal *ikey,
                                               ConfigKeyVal *&okey,
                                               DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  VbrMoMgr *mgr =
    reinterpret_cast<VbrMoMgr *>(const_cast<MoManager *>(GetMoManager(
            UNC_KT_VBRIDGE)));
  if (!mgr) {
    UPLL_LOG_DEBUG("GetControllerId GetMoManager failed (%d)", result_code);
    return UPLL_RC_ERR_GENERIC;
  }
  result_code = mgr->GetChildConfigKey(okey, NULL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetControllerId GetChildConfigKey failed (%d)",
               result_code);
    return result_code;
  }
  key_vbr_t *vbr_okey = reinterpret_cast<key_vbr_t *>(okey->get_key());
  key_vbr_t *vbr_ikey = reinterpret_cast<key_vbr_t *>(ikey->get_key());
  uuu::upll_strncpy(vbr_okey->vbridge_name, vbr_ikey->vbridge_name,
                    (kMaxLenVnodeName+1));
  uuu::upll_strncpy(vbr_okey->vtn_key.vtn_name, vbr_ikey->vtn_key.vtn_name,
      (kMaxLenVtnName+1));

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

upll_rc_t VbrPolicingMapMoMgr::ReadMo(IpcReqRespHeader *req, ConfigKeyVal *ikey,
                                      DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  controller_domain ctrlr_dom;
  ConfigKeyVal *l_key = NULL, *okey = NULL;
  ConfigKeyVal *dup_key = NULL;
  ConfigKeyVal *dup_key1 = NULL;
  DbSubOp dbop = { kOpReadSingle, kOpMatchNone,
    kOpInOutCtrlr|kOpInOutDomain|kOpInOutFlag };

  result_code = ValidateMessage(req, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("ReadMo ValidateMessage Err  (%d)", result_code);
    return result_code;
  }

  memset(&ctrlr_dom, 0, sizeof(controller_domain));
  if (UNC_KT_VBR_POLICINGMAP == ikey->get_key_type()) {
    switch (req->datatype) {
      case UPLL_DT_CANDIDATE:
      case UPLL_DT_RUNNING:
      case UPLL_DT_STARTUP:
      case UPLL_DT_STATE:
        if (req->option1 == UNC_OPT1_NORMAL &&
            req->option2 == UNC_OPT2_NONE) {
          result_code = ReadInfoFromDB(req, ikey, dmi, &ctrlr_dom);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG(" Read request failed result(%d)", result_code);
          }
          return result_code;
        } else if ((UPLL_DT_STATE == req->datatype) &&
                   (req->option1 == UNC_OPT1_DETAIL)&&
                   (req->option2 == UNC_OPT2_NONE)) {
          result_code = ReadDetailRecord(req, ikey, dmi);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("ReadDetailRecord result_code (%d)", result_code);
          }
          return result_code;
        } else {
          UPLL_LOG_DEBUG("ReadRecord ReadConfigDB error (%d) (%d)",
                         req->datatype, req->option1);
          return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT;
        }
        break;
      default:
        return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT;
        break;
    }
  } else if (UNC_KT_VBR_POLICINGMAP_ENTRY == ikey->get_key_type()) {
    switch (req->datatype) {
      case UPLL_DT_STATE:
        if ((req->option1 == UNC_OPT1_NORMAL)&&
            (req->option2 == UNC_OPT2_NONE)) {
          result_code =  GetReadVbrKey(dup_key, ikey);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("ReadConfigDB failed");
            CONFIGKEYVALCLEAN(dup_key);
            return result_code;
          }

          result_code = ReadConfigDB(dup_key, req->datatype, UNC_OP_READ,
                                     dbop, dmi, MAINTBL);

          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("ReadConfigDB failed");
            CONFIGKEYVALCLEAN(dup_key);
            return result_code;
          }

          result_code = ReadDTsateNormal(ikey, dup_key, req->datatype, dbop,
                                         dmi, &okey);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("ReadDTsateNormal failed");
            CONFIGKEYVALCLEAN(dup_key);
            return result_code;
          } else {
            if (okey != NULL) {
              ikey->ResetWith(okey);
            }
          }
          return result_code;
        } else if ((req->option1 == UNC_OPT1_DETAIL) &&
                   (req->option2 == UNC_OPT2_NONE)) {
          // Read the vbr and policyname from main tbl
          result_code = GetReadVbrKey(dup_key, ikey);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("ReadConfigDB failed");
            CONFIGKEYVALCLEAN(dup_key);
            return result_code;
          }
          result_code = ReadConfigDB(dup_key,  req->datatype, UNC_OP_READ,
                                     dbop, dmi, MAINTBL);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("ReadConfigDB failed");
            CONFIGKEYVALCLEAN(dup_key);
            return result_code;
          }

          // Get the renamed controller vtn/vbr and policyprofile name
          result_code = GetReadVbrEntryKey(dup_key1, ikey);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("ReadConfigDB failed");
            CONFIGKEYVALCLEAN(dup_key);
            return result_code;
          }

          // result_code = ReadConfigDB(dup_key1,  req->datatype, UNC_OP_READ,
          //                           dbop, dmi, MAINTBL);

          GET_USER_DATA_CTRLR_DOMAIN(dup_key, ctrlr_dom);
          SET_USER_DATA_CTRLR_DOMAIN(dup_key1, ctrlr_dom);
          // 1.Getting renamed name if renamed
#if 0  // need to uncomment in IMPORT CASE
          result_code = GetRenamedControllerKey(dup_key1, req->datatype,
                                                dmi, &ctrlr_dom);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("GetRenamedControllerKey failed");
            CONFIGKEYVALCLEAN(dup_key);
            CONFIGKEYVALCLEAN(dup_key1);
            return result_code;
          }
#endif
          IpcRequest ipc_req;
          memset(&ipc_req, 0, sizeof(ipc_req));
          ipc_req.header.clnt_sess_id = req->clnt_sess_id;
          ipc_req.header.config_id = req->config_id;
          ipc_req.header.operation = req->operation;
          ipc_req.header.option1 = req->option1;
          ipc_req.header.datatype = req->datatype;
          ipc_req.ckv_data = dup_key1;

          IpcResponse ipc_resp;

          memset(&ipc_resp, 0, sizeof(IpcResponse));
          ipc_resp.ckv_data = NULL;

          memcpy(&(ipc_req.header), req, sizeof(IpcReqRespHeader));
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
          result_code = ConstructReadDetailEntryResponse(dup_key1,
                                                         dup_key,
                                                         ipc_resp.ckv_data,
                                                         req->datatype,
                                                         req->operation,
                                                         dbop,
                                                         dmi, &okey);



          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("ReadDetail failed");
            return result_code;
          } else {
            if (okey != NULL) {
              ikey->ResetWith(okey);
            }
          }
          DELETE_IF_NOT_NULL(dup_key);
          DELETE_IF_NOT_NULL(dup_key1);

        } else {
          UPLL_LOG_DEBUG("ReadRecord ReadConfigDB error (%d) (%d)",
                         req->datatype, req->option1);
          return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT;
        }
        break;
      default:
        UPLL_LOG_DEBUG(
            "VbrPolicingMapMoMgr::ReadRecord ReadConfigDB error (%d) (%d)",
            req->datatype, req->option1);
        return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT;
        break;
    }
  } else {
    return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrPolicingMapMoMgr::GetReadVbrEntryKey(ConfigKeyVal *&dup_key,
                                             ConfigKeyVal *ikey) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  key_vbr_policingmap_entry* vbr_key =
      reinterpret_cast<key_vbr_policingmap_entry *>
      (ConfigKeyVal::Malloc(sizeof(key_vbr_policingmap_entry)));
  dup_key = new ConfigKeyVal(UNC_KT_VBR_POLICINGMAP_ENTRY,
                             IpctSt::kIpcStKeyVbrPolicingmapEntry,
                             vbr_key, NULL);

  key_vbr_policingmap_entry *key =
      reinterpret_cast<key_vbr_policingmap_entry *>(dup_key->get_key());
  key_vbr_policingmap_entry *key1 =
    reinterpret_cast<key_vbr_policingmap_entry *>(ikey->get_key());

  uuu::upll_strncpy(reinterpret_cast<char*>(key->vbr_key.vtn_key.vtn_name),
          reinterpret_cast<char*>(key1->vbr_key.vtn_key.vtn_name),
          (kMaxLenVtnName + 1));
  uuu::upll_strncpy(reinterpret_cast<char*>(key->vbr_key.vbridge_name),
          reinterpret_cast<char*>(key1->vbr_key.vbridge_name),
          (kMaxLenVnodeName + 1));
  vbr_key->sequence_num = key1->sequence_num;
  return result_code;
}

upll_rc_t VbrPolicingMapMoMgr::GetReadVbrKey(ConfigKeyVal *&dup_key,
                                             ConfigKeyVal *ikey) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  result_code =  GetChildConfigKey(dup_key, NULL);
  if (result_code != UPLL_RC_SUCCESS) {
     return result_code;
  }

  key_vbr_t *key = reinterpret_cast<key_vbr_t *>(dup_key->get_key());
  key_vbr_policingmap_entry *key1 =
    reinterpret_cast<key_vbr_policingmap_entry *>(ikey->get_key());

  uuu::upll_strncpy(reinterpret_cast<char*>(key->vtn_key.vtn_name),
          reinterpret_cast<char*>(key1->vbr_key.vtn_key.vtn_name),
          (kMaxLenVtnName + 1));
  uuu::upll_strncpy(reinterpret_cast<char*>(key->vbridge_name),
          reinterpret_cast<char*>(key1->vbr_key.vbridge_name),
          (kMaxLenVnodeName + 1));
  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrPolicingMapMoMgr::ReadSiblingMo(IpcReqRespHeader *req,
                                             ConfigKeyVal *ikey, bool begin,
                                             DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *l_key = NULL, *tctrl_key = NULL, *tmp_key = NULL, *okey = NULL;
  ConfigKeyVal *dup_key = NULL, *dup_key1 = NULL;
  controller_domain ctrlr_dom;

  // validate syntax and semantics

  DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutCtrlr|kOpInOutDomain };
  memset(&ctrlr_dom, 0, sizeof(controller_domain));
  result_code = ValidateMessage(req, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("ValidateMessage Err (%d)", result_code);
    return result_code;
  }
  memset(&ctrlr_dom, 0, sizeof(controller_domain));
  
  if ((UNC_KT_VBR_POLICINGMAP == ikey->get_key_type()) &&
      UNC_OP_READ_SIBLING == req->operation ) {
    /* Since read sibling  not applicable for policingmap*/
    UPLL_LOG_DEBUG("ReadSibling not allowed ");
    return UPLL_RC_ERR_NO_SUCH_INSTANCE;
  }
  if (UNC_KT_VBR_POLICINGMAP == ikey->get_key_type()) {
    switch (req->datatype) {
      case UPLL_DT_CANDIDATE:
      case UPLL_DT_RUNNING:
      case UPLL_DT_STARTUP:
      case UPLL_DT_STATE:
        if (req->option1 == UNC_OPT1_NORMAL &&
            req->option2 == UNC_OPT2_NONE) {
          result_code = ReadInfoFromDB(req, ikey, dmi, &ctrlr_dom);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("Read sibling request failed (%d)", result_code);
          }
          return result_code;
        } else if ((UPLL_DT_STATE == req->datatype) &&
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
        return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT;
        break;
    }
  } else if (UNC_KT_VBR_POLICINGMAP_ENTRY == ikey->get_key_type()) {
    switch (req->datatype) {
      case UPLL_DT_STATE:
        if (req->option1 == UNC_OPT1_NORMAL) {
          result_code =  GetReadVbrKey(dup_key, ikey);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("ReadConfigDB failed");

            CONFIGKEYVALCLEAN(dup_key);
            return result_code;
          }

          result_code = ReadConfigDB(dup_key, req->datatype, UNC_OP_READ,
                                     dbop, dmi, MAINTBL);

          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("ReadConfigDB failed");
            CONFIGKEYVALCLEAN(dup_key);
            return result_code;
          }
          key_policingprofile_entry_t* l_pp_key = NULL;
          PolicingProfileEntryMoMgr *mgr =
              reinterpret_cast<PolicingProfileEntryMoMgr*>
              (const_cast<MoManager *>(GetMoManager
                                       (UNC_KT_POLICING_PROFILE_ENTRY)));

          l_pp_key  = reinterpret_cast<key_policingprofile_entry_t*>
              (ConfigKeyVal::Malloc(sizeof(key_policingprofile_entry_t)));
          tctrl_key = new ConfigKeyVal(UNC_KT_POLICING_PROFILE_ENTRY,
                                       IpctSt::kIpcStKeyPolicingprofileEntry,
                                       l_pp_key, NULL);

          val_policingmap_t* val_policingmap =
              reinterpret_cast<val_policingmap_t *>(GetVal(dup_key));
          l_pp_key->sequence_num = reinterpret_cast<key_vbr_policingmap_entry*>
              (ikey->get_key())->sequence_num;
          uuu::upll_strncpy(l_pp_key->policingprofile_key.policingprofile_name,
                            val_policingmap->policer_name,
                            (kMaxLenPolicingProfileName + 1));

          result_code = mgr->ReadSiblingMo(req, tctrl_key, dmi);
          if (UPLL_RC_SUCCESS != result_code) {
            UPLL_LOG_DEBUG("ReadConfigDb failed for tctrl_key%d ", result_code);
            return result_code;
          }

          tmp_key = tctrl_key;
          while (tmp_key != NULL) {
            ConfigKeyVal *temp_vbr_pm_ckv = NULL;
            key_policingprofile_entry_t *temp_ppe_key = reinterpret_cast
                <key_policingprofile_entry_t *>(tmp_key->get_key());
            result_code = GetReadVbrEntryKey(temp_vbr_pm_ckv, ikey);
            if (UPLL_RC_SUCCESS != result_code) {
              UPLL_LOG_DEBUG("GetReadVbrKey failed");
              return result_code;
            }
            key_vbr_policingmap_entry_t *temp_vbr_pm_key = reinterpret_cast
                <key_vbr_policingmap_entry_t *>(temp_vbr_pm_ckv->get_key());
            temp_vbr_pm_key->sequence_num = temp_ppe_key->sequence_num;
            UPLL_LOG_DEBUG("sequence_num tctrl_key %d ",
                           reinterpret_cast < key_policingprofile_entry_t*>
                           (tmp_key->get_key())->sequence_num);
            result_code = ReadDTsateNormal(temp_vbr_pm_ckv, dup_key,
                                           req->datatype, dbop, dmi, &okey);
            if (result_code != UPLL_RC_SUCCESS) {
              UPLL_LOG_DEBUG("ReadDTsateNormal failed");
              CONFIGKEYVALCLEAN(dup_key);
              return result_code;
            }
            tmp_key = tmp_key->get_next_cfg_key_val();
          }
          if ((okey != NULL) && (result_code == UPLL_RC_SUCCESS)) {
            ikey->ResetWith(okey);
          }
        } else if ((req->option1 == UNC_OPT1_DETAIL) &&
                   (req->option2 == UNC_OPT2_NONE)) {
          result_code = GetReadVbrKey(dup_key, ikey);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("ReadConfigDB failed");
            CONFIGKEYVALCLEAN(dup_key);
            return result_code;
          }
          result_code = ReadConfigDB(dup_key,  req->datatype, UNC_OP_READ,
                                     dbop, dmi, MAINTBL);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("ReadConfigDB failed");
            CONFIGKEYVALCLEAN(dup_key);
            return result_code;
          }

          // Get the renamed controller vtn/vbr and policyprofile name
          result_code = GetReadVbrEntryKey(dup_key1, ikey);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("ReadConfigDB failed");
            CONFIGKEYVALCLEAN(dup_key);
            return result_code;
          }
          GET_USER_DATA_CTRLR_DOMAIN(dup_key, ctrlr_dom);
          SET_USER_DATA_CTRLR_DOMAIN(dup_key1, ctrlr_dom);
          // 1.Getting renamed name if renamed
#if 0  // need to uncomment for IMPORT case
          result_code = GetRenamedControllerKey(dup_key1, req->datatype,
                                                dmi, &ctrlr_dom);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("GetRenamedControllerKey failed");
            //  CONFIGKEYVALCLEAN(dup_key);
            //  CONFIGKEYVALCLEAN(dup_key1);
            return result_code;
          }
#endif
          key_policingprofile_entry_t* l_pp_key = NULL;
          PolicingProfileEntryMoMgr *mgr =
              reinterpret_cast<PolicingProfileEntryMoMgr*>
              (const_cast<MoManager *>(GetMoManager
                                       (UNC_KT_POLICING_PROFILE_ENTRY)));

          l_pp_key  = reinterpret_cast<key_policingprofile_entry_t*>
              (ConfigKeyVal::Malloc(sizeof(key_policingprofile_entry_t)));
          tctrl_key = new ConfigKeyVal(UNC_KT_POLICING_PROFILE_ENTRY,
                                       IpctSt::kIpcStKeyPolicingprofileEntry,
                                       l_pp_key, NULL);

          val_policingmap_t* val_policingmap =
              reinterpret_cast<val_policingmap_t *>(GetVal(dup_key));
          l_pp_key->sequence_num =reinterpret_cast < key_vbr_policingmap_entry*>
              (ikey->get_key())->sequence_num;
          uuu::upll_strncpy(l_pp_key->policingprofile_key.policingprofile_name,
                            val_policingmap->policer_name,
                            (kMaxLenPolicingProfileName + 1));


          SET_USER_DATA_CTRLR_DOMAIN(tctrl_key, ctrlr_dom);
          IpcRequest ipc_req;
          memset(&ipc_req, 0, sizeof(ipc_req));
          ipc_req.header.clnt_sess_id = req->clnt_sess_id;
          ipc_req.header.config_id = req->config_id;
          ipc_req.header.operation = UNC_OP_READ;
          ipc_req.header.option1 = req->option1;
          ipc_req.header.datatype = req->datatype;
          IpcResponse ipc_resp;
          memset(&ipc_resp, 0, sizeof(IpcResponse));
          ipc_resp.ckv_data = NULL;
          //   memcpy(&(ipc_req.header), req, sizeof(IpcReqRespHeader));
          IpcReqRespHeader *temp_req = reinterpret_cast<IpcReqRespHeader *>
              (ConfigKeyVal::Malloc(sizeof(IpcReqRespHeader)));
          memcpy(temp_req, req, sizeof(IpcReqRespHeader));
          temp_req->option1 = UNC_OPT1_NORMAL;
          // result_code = mgr->ReadInfoFromDB(req, ppe_ckv, dmi, &ctrlr_dom);
          //           result_code = mgr->ReadSiblingMo(temp_req, ppe_ckv, dmi);
          result_code = mgr->ReadSiblingMo(temp_req, tctrl_key, dmi);
          if (UPLL_RC_SUCCESS != result_code) {
            UPLL_LOG_DEBUG("ReadConfigDb failed for tctrl_key%d ", result_code);
            return result_code;
          }

          tmp_key = tctrl_key;
          while (tmp_key != NULL) {
            reinterpret_cast <key_vbr_policingmap_entry*>
                (dup_key1->get_key())->sequence_num =
                reinterpret_cast <key_policingprofile_entry_t*>
                (tmp_key->get_key())->sequence_num;
            ipc_req.ckv_data = dup_key1;
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
              UPLL_LOG_DEBUG("Driver response for Key %d ctrlr %s result %d",
                            l_key->get_key_type(), ctrlr_dom.ctrlr,
                            ipc_resp.header.result_code);
              return ipc_resp.header.result_code;
            }

            result_code = ConstructReadDetailEntryResponse(dup_key1,
                                                           dup_key,
                                                           ipc_resp.ckv_data,
                                                           req->datatype,
                                                           req->operation,
                                                           dbop,
                                                           dmi, &okey);


            if (result_code != UPLL_RC_SUCCESS) {
              UPLL_LOG_DEBUG("ReadDetail failed");
              return result_code;
            }
            tmp_key = tmp_key->get_next_cfg_key_val();
          }
          if ((okey != NULL) && (result_code == UPLL_RC_SUCCESS)) {
            ikey->ResetWith(okey);
          }
          DELETE_IF_NOT_NULL(dup_key);
          DELETE_IF_NOT_NULL(dup_key1);
        } else {
          UPLL_LOG_DEBUG("ReadRecord ReadConfigDB error (%d) (%d)",
                         req->datatype, req->option1);
          return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT;
        }
        break;


      default:
        UPLL_LOG_DEBUG(
            "VbrPolicingMapMoMgr::ReadRecord ReadConfigDB error (%d) (%d)",
            req->datatype, req->option1);
        return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT;
        break;
    }
  } else {
    return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT;
  }
  return result_code;
}

upll_rc_t VbrPolicingMapMoMgr::ReadDetailRecord(IpcReqRespHeader *req,
                                               ConfigKeyVal *&ikey,
                                               DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  ConfigKeyVal *l_key = NULL;
  ConfigKeyVal *dup_key = NULL;
  ConfigKeyVal *okey = NULL;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  controller_domain ctrlr_dom;
  DbSubOp dbop = { kOpReadSingle, kOpMatchNone,
    kOpInOutCtrlr|kOpInOutDomain|kOpInOutFlag };

  memset(&ctrlr_dom, 0, sizeof(controller_domain));
  result_code =  DupConfigKeyVal(dup_key, ikey, MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("DupConfigKeyVal Faill in ReadMo for dup_key");
    return result_code;
  }
  result_code = ReadConfigDB(dup_key, req->datatype,
                             UNC_OP_READ, dbop, dmi, MAINTBL);
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
  /*
  // 1.Getting renamed name if renamed
  result_code = GetRenamedControllerKey(l_key, req->datatype,
                                        dmi, &ctrlr_dom);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetRenamedControllerKey failed");
    CONFIGKEYVALCLEAN(l_key);
    CONFIGKEYVALCLEAN(dup_key);
    return result_code;
  }
  */

  IpcRequest ipc_req;
  memset(&ipc_req, 0, sizeof(ipc_req));
  ipc_req.header.clnt_sess_id = req->clnt_sess_id;
  ipc_req.header.config_id = req->config_id;
  ipc_req.header.operation = UNC_OP_READ;
  ipc_req.header.option1 = req->option1;
  ipc_req.header.datatype = req->datatype;

  IpcResponse ipc_resp;
  memset(&ipc_resp, 0, sizeof(IpcResponse));
  ipc_resp.ckv_data = NULL;

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

  result_code = ConstructReadDetailResponse(dup_key,
                                            ipc_resp.ckv_data,
                                            req->datatype,
                                            req->operation,
                                            dbop, dmi, &okey);

  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("IpcResponse failed");
    CONFIGKEYVALCLEAN(l_key);
    CONFIGKEYVALCLEAN(dup_key);
    return result_code;
  } else {
    if (okey != NULL) {
      ikey->ResetWith(okey);
      req->rep_count = 1;
    }
  }
  DELETE_IF_NOT_NULL(dup_key);
  DELETE_IF_NOT_NULL(l_key);
  return result_code;
}
 
// Rename
bool VbrPolicingMapMoMgr::GetRenameKeyBindInfo(unc_key_type_t key_type,
                                               BindInfo *&binfo, int &nattr,
                                               MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  UPLL_LOG_DEBUG("GetRenameKeyBindInfo (%d) (%d)", key_type, tbl);
  switch (key_type) {
    case UNC_KT_VBR_POLICINGMAP:
      nattr = VBR_KEY_COL;
      binfo = key_vbrpm_maintbl_rename_bind_info;
    break;
    case UNC_KT_POLICING_PROFILE:
      nattr = POLICY_KEY_COL;
      binfo = key_vbrpm_policyname_maintbl_rename_bind_info;
    break;
    default:
      UPLL_LOG_DEBUG("GetRenameKeyBindInfo Invalid key type (%d) (%d)",
                     key_type, tbl);
      return PFC_FALSE;
  }
  return PFC_TRUE;
}

// Rename
upll_rc_t VbrPolicingMapMoMgr::CopyToConfigKey(ConfigKeyVal *&okey,
                                               ConfigKeyVal *ikey) {
  UPLL_FUNC_TRACE;
  if (!ikey || !(ikey->get_key())) {
    UPLL_LOG_DEBUG("CopyToConfigKey ikey NULL");
    return UPLL_RC_ERR_GENERIC;
  }

  upll_rc_t result_code = UPLL_RC_SUCCESS;

  if (UNC_KT_VBR_POLICINGMAP == ikey->get_key_type()) {
    key_rename_vnode_info *key_rename =
        reinterpret_cast<key_rename_vnode_info *>(ikey->get_key());
    key_vbr_t *key_vbr =
        reinterpret_cast<key_vbr_t *>(ConfigKeyVal::Malloc(sizeof(key_vbr_t)));
    if (!strlen(reinterpret_cast<char *>(key_rename->old_unc_vtn_name))) {
      UPLL_LOG_DEBUG("old_unc_vtn_name NULL");
      free(key_vbr);
      return UPLL_RC_ERR_GENERIC;
    }
    uuu::upll_strncpy(key_vbr->vtn_key.vtn_name,
                      key_rename->old_unc_vtn_name,
                      (kMaxLenVtnName + 1));

    if (!strlen(reinterpret_cast<char *>(key_rename->old_unc_vnode_name))) {
      UPLL_LOG_DEBUG("old_unc_vnode_name NULL");
      free(key_vbr);
      return UPLL_RC_ERR_GENERIC;
    }
    uuu::upll_strncpy(key_vbr->vbridge_name,
                      key_rename->old_unc_vnode_name,
                      (kMaxLenVnodeName + 1));

    okey = new ConfigKeyVal(UNC_KT_VBR_POLICINGMAP, IpctSt::kIpcStKeyVbr,
                            key_vbr, NULL);
    if (!okey) {
      UPLL_LOG_DEBUG("okey NULL");
      free(key_vbr);
      return UPLL_RC_ERR_GENERIC;
    }
  } else if (UNC_KT_POLICING_PROFILE == ikey->get_key_type()) {
    key_rename_vnode_info_t *key_rename =
        reinterpret_cast<key_rename_vnode_info_t *>(ikey->get_key());

    val_policingmap_t *val = reinterpret_cast<val_policingmap_t *>
        (ConfigKeyVal::Malloc(sizeof(val_policingmap_t)));


    if (!strlen(
        reinterpret_cast<char *>(key_rename->old_policingprofile_name))) {
      UPLL_LOG_DEBUG("old_policingprofile_name NULL");
      free(val);
      return UPLL_RC_ERR_GENERIC;
    }

    uuu::upll_strncpy(val->policer_name,
                      key_rename->old_policingprofile_name,
                      (kMaxLenPolicingProfileName + 1));
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

upll_rc_t VbrPolicingMapMoMgr::MergeValidate(unc_key_type_t keytype,
                                             const char *ctrlr_id,
                                             ConfigKeyVal *ikey,
                                             DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  DbSubOp dbop = { kOpReadMultiple, kOpMatchNone, kOpInOutNone };
  ConfigKeyVal *tkey;
  ConfigKeyVal *pp_keyval = NULL;

  if (NULL == ikey) {
    UPLL_LOG_DEBUG("ikey NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  result_code = ReadConfigDB(ikey, UPLL_DT_IMPORT, UNC_OP_READ, dbop, dmi,
                             MAINTBL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("ReadConfigDB NULL");
    return result_code;
  }

  MoMgrImpl *mgr =
      reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>(GetMoManager(
          UNC_KT_POLICING_PROFILE)));
  tkey = ikey;
  while (ikey != NULL) {
    result_code = mgr->GetChildConfigKey(pp_keyval, NULL);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("GetChildConfigKey NULL");
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
      UPLL_LOG_DEBUG("UpdateConfigDB NULL");
      CONFIGKEYVALCLEAN(pp_keyval);
      return UPLL_RC_ERR_MERGE_CONFLICT;
    }
    ikey = tkey->get_next_cfg_key_val();
    CONFIGKEYVALCLEAN(pp_keyval);
  }
  if (tkey) delete tkey;
  return result_code;
}

upll_rc_t VbrPolicingMapMoMgr::GetRenamedUncKey(ConfigKeyVal *ikey,
                                                upll_keytype_datatype_t dt_type,
                                                DalDmlIntf *dmi,
                                                uint8_t *ctrlr_id) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code;
  ConfigKeyVal *unc_key = NULL;
  DbSubOp dbop = { kOpReadSingle, kOpMatchCtrlr, kOpInOutNone };

  if (NULL == ikey || NULL == dmi || NULL == ctrlr_id) {
    UPLL_LOG_DEBUG(" GetRenamedUncKey failed. Insufficient input parameters.");
    return UPLL_RC_ERR_GENERIC;
  }

  key_vbr *ctrlr_key = reinterpret_cast<key_vbr *>(ikey->get_key());
  if (NULL == ctrlr_key) {
    UPLL_LOG_DEBUG("ctrlr_key NULL");
    return UPLL_RC_ERR_GENERIC;
  }

  val_rename_vnode *rename_vnode = reinterpret_cast<val_rename_vnode *>
      (ConfigKeyVal::Malloc(sizeof(val_rename_vnode)));

  uuu::upll_strncpy(rename_vnode->ctrlr_vtn_name,
                    ctrlr_key->vtn_key.vtn_name,
                    (kMaxLenVtnName + 1));
  uuu::upll_strncpy(rename_vnode->ctrlr_vnode_name,
                    ctrlr_key->vbridge_name,
                    (kMaxLenVnodeName + 1));
  MoMgrImpl *mgr =
      reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>(GetMoManager(
          UNC_KT_VBRIDGE)));
  if (NULL == mgr) {
    UPLL_LOG_DEBUG("mgr NULL");
    free(rename_vnode);
    return UPLL_RC_ERR_GENERIC;
  }

  // Get the memory allocated vbr key structure
  result_code = mgr->GetChildConfigKey(unc_key, NULL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG(" GetRenamedUncKey failed. GetChildConfigKey failed to "
                   "allocate memory for ConfigKeyVal - %d",
                   result_code);
    free(rename_vnode);
    return result_code;
  }
  if (!unc_key) {
    free(rename_vnode);
    return UPLL_RC_ERR_GENERIC;
  }
  SET_USER_DATA_CTRLR(unc_key, ctrlr_id);
  unc_key->AppendCfgVal(IpctSt::kIpcStValRenameVbr, rename_vnode);

  // Read the data from VBR RenameTbl
  result_code = mgr->ReadConfigDB(unc_key, dt_type, UNC_OP_READ, dbop, dmi,
                                  RENAMETBL);
  if (UPLL_RC_SUCCESS != result_code
         && UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
    UPLL_LOG_DEBUG(" GetRenamedUncKey failed. ReadConfigDB failed to read %d ",
                   result_code);
    CONFIGKEYVALCLEAN(unc_key);
    return result_code;
  }

  // Update the unc name in key structure
  if (UPLL_RC_SUCCESS == result_code) {
    key_vbr *vbr_key = reinterpret_cast<key_vbr *>(unc_key->get_key());
    if (vbr_key) {
      if (strcmp(reinterpret_cast<char *>(ctrlr_key->vtn_key.vtn_name),
                reinterpret_cast<const char *>(vbr_key->vtn_key.vtn_name))) {
    uuu::upll_strncpy(ctrlr_key->vtn_key.vtn_name,
                      vbr_key->vtn_key.vtn_name,
                      (kMaxLenVtnName + 1));
      }

      if (strcmp(reinterpret_cast<char *>(ctrlr_key->vbridge_name),
                reinterpret_cast<const char *>(vbr_key->vbridge_name))) {
    uuu::upll_strncpy(ctrlr_key->vbridge_name,
                      vbr_key->vbridge_name,
                      (kMaxLenVnodeName + 1));
      }
    }
  }

  CONFIGKEYVALCLEAN(unc_key);
  mgr = NULL;

  /* ********** Get the Renamed Policer name ******* */
  val_rename_policingprofile *rename_policingprofile =
      reinterpret_cast<val_rename_policingprofile *>(ConfigKeyVal::Malloc(
          sizeof(val_rename_policingprofile)));


  // Get the policer name from ikey
  val_policingmap_t *val_policingmap =
      reinterpret_cast<val_policingmap_t *>(GetVal(ikey));
  if (!val_policingmap) {
    UPLL_LOG_DEBUG("val_policingmap NULL");
    free(rename_policingprofile);
    return UPLL_RC_ERR_GENERIC;
  }
  // Copy the ctrl policer name in rename
  uuu::upll_strncpy(
      rename_policingprofile->policingprofile_newname,
      val_policingmap->policer_name,
      (kMaxLenPolicingProfileName + 1));
  mgr = reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>(GetMoManager(
      UNC_KT_POLICING_PROFILE)));
  if (NULL == mgr) {
    UPLL_LOG_DEBUG("mgr policing profile NULL");
    free(rename_policingprofile);
    return UPLL_RC_ERR_GENERIC;
  }

  // Get the memory allocated policingprofile key structure
  result_code = mgr->GetChildConfigKey(unc_key, NULL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG(" GetRenamedUncKey failed. GetChildConfigKey failed to "
                   "create policingprofile ConfigKeyVal %d",
                   result_code);
    free(rename_policingprofile);
    return result_code;
  }
  if (!unc_key) {
    free(rename_policingprofile);
    return UPLL_RC_ERR_GENERIC;
  }
  SET_USER_DATA_CTRLR(unc_key, ctrlr_id);

  unc_key->AppendCfgVal(IpctSt::kIpcStValRenamePolicingprofile,
                        rename_policingprofile);

  // Read the UNC name from RENAMETBL
  result_code = mgr->ReadConfigDB(unc_key, dt_type, UNC_OP_READ, dbop, dmi,
                                  RENAMETBL);
  if (UPLL_RC_SUCCESS != result_code
      && UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
    UPLL_LOG_DEBUG(" GetRenamedUncKey failed. ReadConfigDB failed to read %d ",
                   result_code);
    CONFIGKEYVALCLEAN(unc_key);
    return UPLL_RC_ERR_GENERIC;
  }

  // Get the UNC name
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

upll_rc_t VbrPolicingMapMoMgr::GetRenamedControllerKey(
    ConfigKeyVal *ikey, upll_keytype_datatype_t dt_type, DalDmlIntf *dmi,
    controller_domain *ctrlr_dom) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code;
  uint8_t rename = 0;
  ConfigKeyVal *okey = NULL;

  if (NULL == ikey || NULL == dmi) {
    UPLL_LOG_DEBUG(
        " GetRenamedControllerKey failed. Insufficient input resources");
    return UPLL_RC_ERR_GENERIC;
  }

  DbSubOp dbop = { kOpReadSingle, kOpMatchCtrlr, kOpInOutCtrlr };

  // Check the rename
  result_code = IsRenamed(ikey, dt_type, dmi, rename);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG(" GetRenamedControllerKey failed. IsRenamed failed to "
                   "check rename - %d",
                   result_code);
    return result_code;
  }

  if (rename == 0) {
    pfc_log_debug(
        "GetRenamedControllerKey No Rename");
     return UPLL_RC_SUCCESS;
  }

  if (rename & VTN_RENAME || rename & VBR_RENAME) {
    // Get the vbr object
    MoMgrImpl *mgr =
      reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>(GetMoManager(
          UNC_KT_VBRIDGE)));
    if (NULL == mgr) {
      UPLL_LOG_DEBUG("mgr NULL");
      return UPLL_RC_ERR_GENERIC;
    }

    // Get memory allocated vbr key structure
    result_code = mgr->GetChildConfigKey(okey, NULL);
    if (result_code != UPLL_RC_SUCCESS) {
       UPLL_LOG_DEBUG(" GetRenamedControllerKey failed. "
                     "GetChildConfigKey failed to create vbr ConfigKey - %d",
                     result_code);
       return result_code;
    }

    if (!okey) return UPLL_RC_ERR_GENERIC;

    key_vbr_t *ovbr_key = reinterpret_cast<key_vbr_t*>(okey->get_key());
    key_vbr_t *ivbr_key = reinterpret_cast<key_vbr_t*>(ikey->get_key());

    uuu::upll_strncpy(reinterpret_cast<char*>(ovbr_key->vtn_key.vtn_name),
          reinterpret_cast<char*>(ivbr_key->vtn_key.vtn_name),
          (kMaxLenVtnName+1));
    uuu::upll_strncpy(reinterpret_cast<char*>(ovbr_key->vbridge_name),
          reinterpret_cast<char*>(ivbr_key->vbridge_name),
          (kMaxLenVnodeName+1));

    if (NULL != ctrlr_dom) SET_USER_DATA_CTRLR_DOMAIN(okey, *ctrlr_dom);

    // Read the renamed vtn/vbr
    result_code = mgr->ReadConfigDB(okey, dt_type, UNC_OP_READ, dbop, dmi,
                                    RENAMETBL);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG(" GetRenamedControllerKey failed. ReadConfigDB failed "
                     "to read vbr renametbl - %d",
                     result_code);
      CONFIGKEYVALCLEAN(okey);
      return result_code;
    }
    // Get the key from okey
    val_rename_vnode_t *rename_val =
      reinterpret_cast<val_rename_vnode_t *>(GetVal(okey));
    if (!rename_val) {
      UPLL_LOG_DEBUG("rename_val NULL");
      CONFIGKEYVALCLEAN(okey);
      return UPLL_RC_ERR_GENERIC;
    }

    if (rename & VTN_RENAME) { /* vtn renamed */
      uuu::upll_strncpy(ivbr_key->vtn_key.vtn_name,
                      rename_val->ctrlr_vtn_name,
                      (kMaxLenVtnName + 1));
    }
    if (rename & VBR_RENAME) { /* vnode renamed */
      uuu::upll_strncpy(ivbr_key->vbridge_name,
                      rename_val->ctrlr_vnode_name,
                      (kMaxLenVnodeName + 1));
    }
    SET_USER_DATA_FLAGS(ikey, rename);
    mgr = NULL;
  }

  CONFIGKEYVALCLEAN(okey);

  if (rename & POLICINGPROFILE_RENAME) {
    // Get policingprofile object
    MoMgrImpl *mgr = reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>
          (GetMoManager(UNC_KT_POLICING_PROFILE)));
    if (NULL == mgr) {
      UPLL_LOG_DEBUG("mgr NULL");
      return UPLL_RC_ERR_GENERIC;
    }
    // Get policingprofile key structure
    result_code = mgr->GetChildConfigKey(okey, NULL);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG(" GetRenamedControllerKey GetChildConfigKey failed"
                   "to create policingprofile ConfigKey - %d",
                   result_code);
      return result_code;
    }
    if (!okey) return UPLL_RC_ERR_GENERIC;

    val_policingmap_t *val_policingmap =
      reinterpret_cast<val_policingmap_t *>(GetVal(ikey));

    key_policingprofile_t *key_policingprofile =
      reinterpret_cast<key_policingprofile_t *>(okey->get_key());


    uuu::upll_strncpy(key_policingprofile->policingprofile_name,
                    val_policingmap->policer_name,
                    (kMaxLenPolicingProfileName + 1));

    if (NULL != ctrlr_dom) SET_USER_DATA_CTRLR_DOMAIN(okey, *ctrlr_dom);

    result_code = mgr->ReadConfigDB(okey, dt_type, UNC_OP_READ, dbop, dmi,
                                  RENAMETBL);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG(" GetRenamedControllerKey failed. ReadConfigDB failed "
                   "to read policingprofile renametbl - %d",
                   result_code);
      CONFIGKEYVALCLEAN(okey);
      return result_code;
    }
    val_rename_policingprofile_t *rename_policingprofile =
      reinterpret_cast<val_rename_policingprofile_t *>(GetVal(okey));
    if (!rename_policingprofile) {
      CONFIGKEYVALCLEAN(okey);
      return UPLL_RC_ERR_GENERIC;
    }

    uuu::upll_strncpy(
       reinterpret_cast<char *>(val_policingmap->policer_name),
       reinterpret_cast<const char *>(rename_policingprofile
       ->policingprofile_newname),
       (kMaxLenPolicingProfileName + 1));

    SET_USER_DATA_FLAGS(ikey, rename);
    mgr = NULL;
    CONFIGKEYVALCLEAN(okey);
  }
  CONFIGKEYVALCLEAN(okey);
  return UPLL_RC_SUCCESS;
}

bool VbrPolicingMapMoMgr::CompareValidValue(void *&val1, void *val2,
                                            bool audit) {
  UPLL_FUNC_TRACE;
  val_policingmap_t *val_pm1 = reinterpret_cast<val_policingmap_t *>(val1);
  val_policingmap_t *val_pm2 = reinterpret_cast<val_policingmap_t *>(val2);
//  if (audit) {
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

upll_rc_t VbrPolicingMapMoMgr::UpdateConfigStatus(ConfigKeyVal *ckv,
    unc_keytype_operation_t op, uint32_t driver_result,
    ConfigKeyVal *nreq, DalDmlIntf *dmi,
    ConfigKeyVal *ctrlr_key) {
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

upll_rc_t VbrPolicingMapMoMgr::UpdateAuditConfigStatus(
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
  for (unsigned int loop = 0; loop < sizeof(val->valid) / sizeof(uint8_t);
      ++loop) {
    if (cs_status == UNC_CS_INVALID && UNC_VF_VALID == val->valid[loop])
       val->cs_attr[loop] = cs_status;
    else
       val->cs_attr[loop] = cs_status;
  }
  return result_code;
}

upll_rc_t VbrPolicingMapMoMgr::ValidateAttribute(ConfigKeyVal *ikey,
                                                 DalDmlIntf *dmi,
                                                 IpcReqRespHeader *req) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  ConfigKeyVal *okey = NULL;
  if (!ikey || !ikey->get_key()) {
    UPLL_LOG_DEBUG("input key is null");
    return UPLL_RC_ERR_GENERIC;
  }

  key_vbr_t *vbr_key_pm = reinterpret_cast<key_vbr_t*>(ikey->get_key());

  MoMgrImpl *mgr = reinterpret_cast<MoMgrImpl *>
      (const_cast<MoManager *>(GetMoManager(UNC_KT_VBRIDGE)));

  result_code = mgr->GetChildConfigKey(okey, NULL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Memory allocation failed for KT_VBRIDGE key struct - %d",
                    result_code);
    return result_code;
  }

  key_vbr_t *vbr_key =
      reinterpret_cast<key_vbr_t *>(okey->get_key());

  uuu::upll_strncpy(vbr_key->vtn_key.vtn_name,
        vbr_key_pm->vtn_key.vtn_name,
        kMaxLenVtnName + 1);

  uuu::upll_strncpy(vbr_key->vbridge_name,
        vbr_key_pm->vbridge_name,
        kMaxLenVnodeName + 1);

  /* Checks the given key_vbr exists in DB or not */
  result_code = mgr->UpdateConfigDB(okey, req->datatype, UNC_OP_READ,
                                    dmi, MAINTBL);

  if (UPLL_RC_ERR_INSTANCE_EXISTS != result_code) {
    UPLL_LOG_DEBUG(" Parent KT_VBRIDGE key does not exists");
    result_code = UPLL_RC_ERR_PARENT_DOES_NOT_EXIST;
  } else {
    result_code = UPLL_RC_SUCCESS;
  }

  delete okey;
  UPLL_LOG_DEBUG("ValidateAttribute Successfull.");
  return result_code;
}

bool VbrPolicingMapMoMgr::CompareKey(void *key1, void *key2) {
  UPLL_FUNC_TRACE;
  key_vbr_t *vbr_key1, *vbr_key2;
  bool match = false;
  vbr_key1 = reinterpret_cast<key_vbr_t *>(key1);
  vbr_key2 = reinterpret_cast<key_vbr_t *>(key2);
  if (vbr_key1 == vbr_key2) return true;
  if ((!vbr_key1) || (!vbr_key2)) return false;
  if (strncmp(reinterpret_cast<const char *>(vbr_key1->vtn_key.vtn_name),
              reinterpret_cast<const char *>(vbr_key2->vtn_key.vtn_name),
              kMaxLenVtnName + 1) == 0)
    if (strncmp(reinterpret_cast<const char *>(vbr_key1->vbridge_name),
                reinterpret_cast<const char *>(vbr_key2->vbridge_name),
                kMaxLenVtnName + 1) == 0) match = true;
  return match;
}

upll_rc_t VbrPolicingMapMoMgr::GetValid(void *val, uint64_t indx,
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
      case uudst::vbr_policingmap::kDbiPolicername:
        valid =
            &(reinterpret_cast<val_policingmap_t *>(val)
                       ->valid[UPLL_IDX_POLICERNAME_PM]);
        break;
      default:
        valid = NULL;
        UPLL_LOG_DEBUG("Invalid Index");
        return UPLL_RC_ERR_GENERIC;
    }
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrPolicingMapMoMgr::AllocVal(ConfigVal *&ck_val,
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
    val = reinterpret_cast<void *>(ConfigKeyVal::Malloc
        (sizeof(val_policingmap_t)));


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

upll_rc_t VbrPolicingMapMoMgr::DupConfigKeyVal(ConfigKeyVal *&okey,
                                               ConfigKeyVal *&req,
                                               MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  if (req == NULL || okey != NULL || req->get_key_type() !=
                 UNC_KT_VBR_POLICINGMAP || tbl != MAINTBL) {
    UPLL_LOG_DEBUG(" DupConfigKeyVal failed. Input ConfigKeyVal is NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  ConfigVal *tmp1 = NULL, *tmp = (req)->get_cfg_val();

  val_policingmap_t *policingmap_val = NULL;

  if (tmp) {
    if (tbl == MAINTBL) {
      val_policingmap_t *ival = reinterpret_cast<val_policingmap_t *>
                              (GetVal(req));
      if (NULL == ival) {
        UPLL_LOG_DEBUG("DupConfigKeyVal val_policingmap_t alloc failure");
        return UPLL_RC_ERR_GENERIC;
      }
      policingmap_val =
      reinterpret_cast<val_policingmap_t *>(ConfigKeyVal::Malloc
          (sizeof(val_policingmap_t)));

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
  void *tkey = (req != NULL) ? req->get_key() : NULL;

  key_vbr_t *ikey = reinterpret_cast<key_vbr_t *>(tkey);
  key_vbr_t *vbr_key = reinterpret_cast<key_vbr_t *>
      (ConfigKeyVal::Malloc(sizeof(key_vbr_t)));
  memcpy(vbr_key, ikey, sizeof(key_vbr_t));
  okey = new ConfigKeyVal(UNC_KT_VBR_POLICINGMAP, IpctSt::kIpcStKeyVbr, vbr_key,
                          tmp1);
  if (!okey) {
    if (vbr_key) free(vbr_key);
    if (policingmap_val) free(policingmap_val);
    UPLL_LOG_DEBUG("okey failed");
    return UPLL_RC_ERR_GENERIC;
  }
  SET_USER_DATA(okey, req);
  UPLL_LOG_DEBUG("DupConfigKeyVal Successful");
  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrPolicingMapMoMgr::GetChildConfigKey(ConfigKeyVal *&okey,
                                                 ConfigKeyVal *parent_key) {
  UPLL_FUNC_TRACE;
  key_vbr_t *vbr_key;
  void *pkey;
  if (parent_key == NULL) {
    vbr_key = reinterpret_cast<key_vbr_t *>
        (ConfigKeyVal::Malloc(sizeof(key_vbr_t)));
    okey = new ConfigKeyVal(UNC_KT_VBR_POLICINGMAP, IpctSt::kIpcStKeyVbr,
                            vbr_key, NULL);
    UPLL_LOG_DEBUG(" GetChildConfigKey Success.");
    return UPLL_RC_SUCCESS;
  } else {
    pkey = parent_key->get_key();
  }
  if (!pkey) {
    UPLL_LOG_DEBUG("pkey null");
    return UPLL_RC_ERR_GENERIC;
  }
  if (okey) {
    if (okey->get_key_type() != UNC_KT_VBR_POLICINGMAP)
      return UPLL_RC_ERR_GENERIC;
    vbr_key = reinterpret_cast<key_vbr_t *>(okey->get_key());
  } else {
    vbr_key = reinterpret_cast<key_vbr_t *>
        (ConfigKeyVal::Malloc(sizeof(key_vbr_t)));
  }
  switch (parent_key->get_key_type()) {
    case UNC_KT_VTN:
      uuu::upll_strncpy(vbr_key->vtn_key.vtn_name,
                        reinterpret_cast<key_vtn *>(pkey)->vtn_name,
                        (kMaxLenVtnName + 1));
      break;
    case UNC_KT_VBRIDGE:
    case UNC_KT_VBR_POLICINGMAP:
      uuu::upll_strncpy(vbr_key->vtn_key.vtn_name,
                        reinterpret_cast<key_vbr *>(pkey)->vtn_key.vtn_name,
                        (kMaxLenVtnName + 1));
      uuu::upll_strncpy(vbr_key->vbridge_name,
                        reinterpret_cast<key_vbr *>(pkey)->vbridge_name,
                        (kMaxLenVnodeName + 1));
      break;
    default:
      if (vbr_key) free(vbr_key);
      return UPLL_RC_ERR_GENERIC;
  }
  if (!okey) {
    okey = new ConfigKeyVal(UNC_KT_VBR_POLICINGMAP, IpctSt::kIpcStKeyVbr,
                            vbr_key, NULL);
  }
  SET_USER_DATA(okey, parent_key);
  UPLL_LOG_DEBUG(" GetChildConfigKey Succesful");
  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrPolicingMapMoMgr::ValidateMessage(IpcReqRespHeader *req,
                                               ConfigKeyVal *key) {
  UPLL_FUNC_TRACE;

  upll_rc_t rt_code = UPLL_RC_ERR_GENERIC;

  if (NULL == req || NULL == key) {
    UPLL_LOG_DEBUG("req/ConfigKeyval is NULL");
    return UPLL_RC_ERR_BAD_REQUEST;
  }

  key_vbr_t *key_vbr = NULL;
  key_vbr_policingmap_entry_t *key_vbr_policingmap_entry = NULL;
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

  /** check key type is UNC_KT_VBR_POLICINGMAP */
  if (UNC_KT_VBR_POLICINGMAP == key->get_key_type()) {
    if (key->get_st_num() != IpctSt::kIpcStKeyVbr) {
      UPLL_LOG_DEBUG(
          " Invalid structure received expected struct -"
          "kIpcStKeyVbr, received struct - %s ",
          reinterpret_cast<const char *>
          (IpctSt::GetIpcStdef(key->get_st_num())));
      return UPLL_RC_ERR_BAD_REQUEST;
    }
    /** Read key structure */
    key_vbr = reinterpret_cast<key_vbr_t *>(key->get_key());

  } else if (UNC_KT_VBR_POLICINGMAP_ENTRY == key->get_key_type()) {
    if (key->get_st_num() != IpctSt::kIpcStKeyVbrPolicingmapEntry) {
      UPLL_LOG_DEBUG(
          " Invalid structure received expected struct -"
          "kIpcStKeyVbrPolicingmapEntry, received struct - %s ",
          reinterpret_cast<const char *>(IpctSt::GetIpcStdef
          (key->get_st_num())));
      return UPLL_RC_ERR_BAD_REQUEST;
    }
    key_vbr_policingmap_entry =
        reinterpret_cast<key_vbr_policingmap_entry_t*>(key->get_key());

    key_vbr = &(key_vbr_policingmap_entry->vbr_key);

  } else {
    UPLL_LOG_DEBUG(" Invalid keytype(%d)", key->get_key_type());
    return UPLL_RC_ERR_BAD_REQUEST;
  }

  if (NULL == key_vbr) {
    UPLL_LOG_DEBUG("KT_VBR_POLICINGMAP Key structure is empty!!");
    return UPLL_RC_ERR_BAD_REQUEST;
  }

  /** validate key struct */
  VbrMoMgr *mgrvbr =
      reinterpret_cast<VbrMoMgr *>(const_cast<MoManager*>(GetMoManager(
          UNC_KT_VBRIDGE)));

  if (NULL == mgrvbr) {
    UPLL_LOG_DEBUG("unable to get VbrMoMgr object to validate key_vbr");
    return UPLL_RC_ERR_GENERIC;
  }

  rt_code = mgrvbr->ValidateVbrKey(key_vbr);

  if (UPLL_RC_SUCCESS != rt_code) {
    UPLL_LOG_DEBUG(" Vbr key syntax validation failed :"
                  "Err Code - %d",
                  rt_code);
    return rt_code;
  }

  if (UNC_KT_VBR_POLICINGMAP_ENTRY == key->get_key_type()) {
    if ((req->operation != UNC_OP_READ_SIBLING_COUNT) &&
        (req->operation != UNC_OP_READ_SIBLING_BEGIN)) {
      if (!ValidateNumericRange(key_vbr_policingmap_entry->sequence_num,
                                (uint8_t) kMinPolicingProfileSeqNum,
                                (uint8_t) kMaxPolicingProfileSeqNum, true,
                                true)) {
        UPLL_LOG_DEBUG("Sequence num syntax validation failed :Err Code - %d",
                       rt_code);
        return UPLL_RC_ERR_CFG_SYNTAX;
      }
    } else {
      key_vbr_policingmap_entry->sequence_num = 0;
    }

  UPLL_LOG_DEBUG(
        "key struct validation is success for UNC_KT_VBR_POLICINGMAP_ENTRY");
    return UPLL_RC_SUCCESS;
  }

  /** validate val struct for UNC_KT_VBR_POLICINGMAP */
  UPLL_LOG_DEBUG(" key struct validation is success");

  val_policingmap_t *val_policingmap = NULL;

  if (key->get_cfg_val() && (key->get_cfg_val()->get_st_num() ==
      IpctSt::kIpcStValPolicingmap)) {
      val_policingmap =
      reinterpret_cast<val_policingmap_t *>(key->get_cfg_val()->get_val());
  }

  rt_code = VtnPolicingMapMoMgr::ValidatePolicingMapValue(val_policingmap, req);

  if (UPLL_RC_SUCCESS != rt_code) {
    UPLL_LOG_DEBUG(" val_policingmap syntax validation failed :"
                  "Err Code - %d",
                  rt_code);
  }
  return rt_code;
}

upll_rc_t VbrPolicingMapMoMgr::ValidateCapability(IpcReqRespHeader *req,
                                           ConfigKeyVal *ikey,
                                           const char *ctrlr_name) {
  UPLL_FUNC_TRACE;

  upll_rc_t rt_code = UPLL_RC_ERR_GENERIC;

  /** Use  VtnPolicingMapMoMgr::ValidateCapability
   *  to validate capability for val_policingmap structure*/
  VtnPolicingMapMoMgr *mgrvtnpmap = reinterpret_cast<VtnPolicingMapMoMgr *>
               (const_cast<MoManager *>(GetMoManager(UNC_KT_VTN_POLICINGMAP)));

  rt_code = mgrvtnpmap->ValidateCapability(req, ikey);

  if (UPLL_RC_SUCCESS != rt_code) {
    UPLL_LOG_DEBUG(" PolicierName syntax validation failed :"
                  "Err Code - %d",
                  rt_code);
  }
  return rt_code;
}

bool VbrPolicingMapMoMgr::IsValidKey(void *key, uint64_t index) {
  UPLL_FUNC_TRACE;
  key_vbr *vbr_key = reinterpret_cast<key_vbr *>(key);
  bool ret_val = false;
  switch (index) {
    case uudst::vbridge::kDbiVtnName:
      ret_val = ValidateKey(reinterpret_cast<char *>(vbr_key->vtn_key.vtn_name),
                            kMinLenVtnName, kMaxLenVtnName);
      if (ret_val != UPLL_RC_SUCCESS) {
        UPLL_LOG_TRACE("VTN Name is not valid(%d)", ret_val);
        return false;
      }
      break;
    case uudst::vbridge::kDbiVbrName:
      ret_val = ValidateKey(reinterpret_cast<char *>(vbr_key->vbridge_name),
                            kMinLenVnodeName,
                            kMaxLenVnodeName);
      if (ret_val != UPLL_RC_SUCCESS) {
        UPLL_LOG_TRACE("VBR Name is not valid(%d)", ret_val);
        return false;
      }
      break;
    default:
      UPLL_LOG_TRACE("Invalid Key Index");
      return false;
  }
  return true;
}

upll_rc_t VbrPolicingMapMoMgr::IsKeyInUse(upll_keytype_datatype_t dt_type,
                     const ConfigKeyVal *ckv,
                     bool *in_use,
                     DalDmlIntf *dmi) {
    return UPLL_RC_SUCCESS;
}
upll_rc_t VbrPolicingMapMoMgr::ConstructReadDetailEntryResponse(
                                         ConfigKeyVal *ikey,
                                         ConfigKeyVal *dup_key,
                                         ConfigKeyVal *drv_resp_ckv,
                                         upll_keytype_datatype_t dt_type,
                                         unc_keytype_operation_t op,
                                         DbSubOp dbop,
                                         DalDmlIntf *dmi,
                                         ConfigKeyVal **okey  ) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *tmp_okey = NULL;
  ConfigVal *drv_resp_val = NULL;
  drv_resp_val =  drv_resp_ckv->get_cfg_val();

  result_code =  GetReadVbrEntryKey(tmp_okey, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetChildConfigKey failed err code (%d)", result_code);
    return result_code;
  }
  val_policingmap_t *val_pm = reinterpret_cast<val_policingmap_t *>
      (ConfigKeyVal::Malloc(sizeof(val_policingmap_t)));
  val_policingmap_t* val_policingmap =
      reinterpret_cast<val_policingmap_t *>(GetVal(dup_key));
  memcpy(val_pm, val_policingmap, sizeof(val_policingmap_t));

  tmp_okey->AppendCfgVal(IpctSt::kIpcStValPolicingmap, val_pm);

  while (drv_resp_val != NULL) {
    val_policingmap_controller_st *val_entry_st = NULL;
    if (IpctSt::kIpcStValPolicingmapControllerSt ==
        drv_resp_val->get_st_num()) {
      UPLL_LOG_TRACE("Get the val struct");
      val_entry_st = reinterpret_cast< val_policingmap_controller_st *>
          (drv_resp_val->get_val());
    } else {
      UPLL_LOG_DEBUG("Incorrect structure received from driver, struct num %d",
                     drv_resp_val->get_st_num());
      return  UPLL_RC_ERR_GENERIC;
    }
    UPLL_LOG_TRACE("valida of val structure");
    if (val_entry_st->valid[UPLL_IDX_SEQ_NUM_FFES] == UNC_VF_VALID) {
      ConfigKeyVal *tkey = NULL;



      key_policingprofile_entry_t *key_policingprofile_entry =
          reinterpret_cast<key_policingprofile_entry_t *>
          (ConfigKeyVal::Malloc(sizeof(key_policingprofile_entry_t)));
      UPLL_LOG_TRACE("Before tkey");
      tkey = new ConfigKeyVal(UNC_KT_POLICING_PROFILE_ENTRY,
                              IpctSt::kIpcStKeyPolicingprofileEntry,
                              key_policingprofile_entry, NULL);
      key_policingprofile_entry->sequence_num = val_entry_st->sequence_num;

      uuu::upll_strncpy(
          key_policingprofile_entry->policingprofile_key.policingprofile_name,
          val_policingmap->policer_name,
          (kMaxLenPolicingProfileName+1));

      PolicingProfileEntryMoMgr *mgr =
          reinterpret_cast<PolicingProfileEntryMoMgr*>
          (const_cast<MoManager *>(GetMoManager
                                   (UNC_KT_POLICING_PROFILE_ENTRY)));
      result_code = mgr->ReadDetailEntry(
          tkey, dt_type,  dbop, dmi);
      if (result_code != UPLL_RC_SUCCESS) {
        delete tmp_okey;
        delete tkey;
        return result_code;
      }
      UPLL_LOG_TRACE("Befor val pp entry");
      val_policingprofile_entry_t *temp_val_policingprofile =
          reinterpret_cast<val_policingprofile_entry_t *>
          (tkey->get_cfg_val()->get_val());
      UPLL_LOG_TRACE("Get val pf pp entry");

      val_policingmap_controller_st *tmp_cont_st =
          reinterpret_cast<val_policingmap_controller_st * >
          (ConfigKeyVal::Malloc(sizeof(val_policingmap_controller_st)));


      memcpy(tmp_cont_st, val_entry_st, sizeof(val_policingmap_controller_st));
      tmp_okey->AppendCfgVal(IpctSt::kIpcStValPolicingmapControllerSt,
                             tmp_cont_st);
      val_policingprofile_entry_t *tmp_val_ppe =
          reinterpret_cast<val_policingprofile_entry_t * >
          (ConfigKeyVal::Malloc(sizeof(val_policingprofile_entry_t)));

      memcpy(tmp_val_ppe, temp_val_policingprofile,
             sizeof(val_policingprofile_entry_t));
      tmp_okey->AppendCfgVal(IpctSt::kIpcStValPolicingprofileEntry,
                             tmp_val_ppe);
      //      temp_cfg_val = temp_cfg_val->get_next_cfg_val();
      delete tkey;
      tkey = NULL;
      if ((drv_resp_val = drv_resp_val->get_next_cfg_val())== NULL) {
        UPLL_LOG_TRACE("if it s fail");
        continue;
      }
      if (IpctSt::kIpcStValPolicingmapSwitchSt != drv_resp_val->get_st_num()) {
        UPLL_LOG_DEBUG("No PolicingmapSwitchSt entries returned by driver");
        continue;
      }

      while (IpctSt::kIpcStValPolicingmapSwitchSt ==
             drv_resp_val->get_st_num()) {
        val_policingmap_switch_st * val_switch_st =
            reinterpret_cast<val_policingmap_switch_st*>
            (ConfigKeyVal::Malloc(sizeof(val_policingmap_switch_st)));
        memcpy(val_switch_st,
               reinterpret_cast<val_policingmap_switch_st*>
               (drv_resp_val->get_val()),
               sizeof(val_policingmap_switch_st));

        tmp_okey->AppendCfgVal(IpctSt::kIpcStValPolicingmapSwitchSt,
                               drv_resp_val->get_val());
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

upll_rc_t VbrPolicingMapMoMgr::ConstructReadDetailResponse(
                                         ConfigKeyVal *ikey,
                                         ConfigKeyVal *drv_resp_ckv,
                                         upll_keytype_datatype_t dt_type,
                                         unc_keytype_operation_t op,
                                         DbSubOp dbop,
                                         DalDmlIntf *dmi,
                                         ConfigKeyVal **okey  ) {
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
  val_policingmap_t *val_pm = reinterpret_cast<val_policingmap_t *>
      (ConfigKeyVal::Malloc(sizeof(val_policingmap_t)));
  val_policingmap_t* val_policingmap =
      reinterpret_cast<val_policingmap_t *>(GetVal(ikey));
  memcpy(val_pm, val_policingmap, sizeof(val_policingmap_t));

  tmp_okey->AppendCfgVal(IpctSt::kIpcStValPolicingmap, val_pm);

  while (drv_resp_val != NULL) {
    val_policingmap_controller_st *val_entry_st = NULL;
    if (IpctSt::kIpcStValPolicingmapControllerSt ==
        drv_resp_val->get_st_num()) {
      val_entry_st = reinterpret_cast< val_policingmap_controller_st *>
          (drv_resp_val->get_val());
    } else {
      UPLL_LOG_DEBUG("Incorrect structure received from driver, struct num %d",
                     drv_resp_val->get_st_num());
      delete tmp_okey;
      return  UPLL_RC_ERR_GENERIC;
    }
    if (val_entry_st->valid[UPLL_IDX_SEQ_NUM_FFES] == UNC_VF_VALID) {
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

      PolicingProfileEntryMoMgr *mgr =
          reinterpret_cast<PolicingProfileEntryMoMgr*>
          (const_cast<MoManager *>(GetMoManager
                                   (UNC_KT_POLICING_PROFILE_ENTRY)));
      result_code = mgr->ReadDetailEntry(
          tkey, dt_type,  dbop, dmi);
      if (result_code != UPLL_RC_SUCCESS) {
        delete tkey;
        return result_code;
      }
      val_policingprofile_entry_t *temp_val_policingprofile =
          reinterpret_cast<val_policingprofile_entry_t *>
          (tkey->get_cfg_val()->get_val());

      val_policingmap_controller_st *tmp_cont_st =
          reinterpret_cast<val_policingmap_controller_st * >
          (ConfigKeyVal::Malloc(sizeof(val_policingmap_controller_st)));


      memcpy(tmp_cont_st, val_entry_st, sizeof(val_policingmap_controller_st));
      tmp_okey->AppendCfgVal(IpctSt::kIpcStValPolicingmapControllerSt,
                             val_entry_st);
      val_policingprofile_entry_t *tmp_val_ppe =
          reinterpret_cast<val_policingprofile_entry_t * >
          (ConfigKeyVal::Malloc(sizeof(val_policingprofile_entry_t)));

      memcpy(tmp_val_ppe, temp_val_policingprofile,
             sizeof(val_policingprofile_entry_t));
      tmp_okey->AppendCfgVal(IpctSt::kIpcStValPolicingprofileEntry,
                             tmp_val_ppe);
      //      temp_cfg_val = temp_cfg_val->get_next_cfg_val();
      delete tkey;
      tkey = NULL;
      if ((drv_resp_val = drv_resp_val->get_next_cfg_val())== NULL) {
        continue;
      }
      if (IpctSt::kIpcStValPolicingmapSwitchSt != drv_resp_val->get_st_num()) {
        UPLL_LOG_DEBUG("No PolicingmapSwitchSt entries returned by driver");
        continue;
      }

      while (IpctSt::kIpcStValPolicingmapSwitchSt ==
             drv_resp_val->get_st_num()) {
        val_policingmap_switch_st * val_switch_st =
            reinterpret_cast<val_policingmap_switch_st*>
            (ConfigKeyVal::Malloc(sizeof(val_policingmap_switch_st)));
        memcpy(val_switch_st,
               reinterpret_cast<val_policingmap_switch_st*>
               (drv_resp_val->get_val()),
               sizeof(val_policingmap_switch_st));

        tmp_okey->AppendCfgVal(IpctSt::kIpcStValPolicingmapSwitchSt,
                               drv_resp_val->get_val());
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

upll_rc_t VbrPolicingMapMoMgr::ReadDTsateNormal(
    ConfigKeyVal *ikey,
    ConfigKeyVal* dup_key,
    upll_keytype_datatype_t  dt_type,
    DbSubOp dbop,
    DalDmlIntf *dmi,
    ConfigKeyVal**okey) {
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *tmp_okey = NULL;
  ConfigKeyVal* tkey = NULL;
  val_policingmap_t* val_policingmap = NULL;
  result_code =  GetReadVbrEntryKey(tmp_okey, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG(" GetReadVbrEntryKey fail ");
    return result_code;
  }
  val_policingmap = reinterpret_cast<val_policingmap_t*>(GetVal(dup_key));
  if (val_policingmap == NULL) {
    UPLL_LOG_DEBUG("Invalid value");
    return UPLL_RC_ERR_GENERIC;
  }
  val_policingmap_t *val_pm = reinterpret_cast<val_policingmap_t *>
      (ConfigKeyVal::Malloc(sizeof(val_policingmap_t)));
  memcpy(val_pm, val_policingmap, sizeof(val_policingmap_t));

  tmp_okey->AppendCfgVal(IpctSt::kIpcStValPolicingmap, val_pm);

  key_vbr_policingmap_entry *key =
      reinterpret_cast<key_vbr_policingmap_entry *>(ikey->get_key());

  key_policingprofile_entry_t *key_policingprofile_entry =
      reinterpret_cast<key_policingprofile_entry_t *>
      (ConfigKeyVal::Malloc(sizeof(key_vbrif_policingmap_entry_t)));

  tkey = new ConfigKeyVal(UNC_KT_POLICING_PROFILE_ENTRY,
                          IpctSt::kIpcStKeyPolicingprofileEntry,
                          key_policingprofile_entry, NULL);

  key_policingprofile_entry->sequence_num = key->sequence_num;

  uuu::upll_strncpy(
      key_policingprofile_entry->policingprofile_key.policingprofile_name,
      val_policingmap->policer_name,
      (kMaxLenPolicingProfileName+1));

  PolicingProfileEntryMoMgr *mgr =
      reinterpret_cast<PolicingProfileEntryMoMgr*>
      (const_cast<MoManager *>(GetMoManager
                               (UNC_KT_POLICING_PROFILE_ENTRY)));


  result_code = mgr->ReadDetailEntry(
      tkey, dt_type, dbop, dmi);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("ReadDetailEntry failed");
    CONFIGKEYVALCLEAN(tkey);
    delete tmp_okey;
    return result_code;
  }

  val_policingprofile_entry_t *temp_val_policingprofile =
      reinterpret_cast<val_policingprofile_entry_t *>
      (tkey->get_cfg_val()->get_val());
  val_policingprofile_entry_t *tmp_val_ppe =
      reinterpret_cast<val_policingprofile_entry_t * >
      (ConfigKeyVal::Malloc(sizeof(val_policingprofile_entry_t)));

  memcpy(tmp_val_ppe, temp_val_policingprofile,
         sizeof(val_policingprofile_entry_t));
  tmp_okey->AppendCfgVal(IpctSt::kIpcStValPolicingprofileEntry,
                         tmp_val_ppe);
  if (*okey == NULL) {
    *okey = tmp_okey;
  } else {
    (*okey)->AppendCfgKeyVal(tmp_okey);
  }

  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrPolicingMapMoMgr::GetParentConfigKey(ConfigKeyVal *&okey,
                                                  ConfigKeyVal *ikey) {
  UPLL_FUNC_TRACE;

  if (!ikey) {
    UPLL_LOG_DEBUG(" Input Key is NULL");
    return UPLL_RC_ERR_GENERIC;
  }

  unc_key_type_t ikey_type = ikey->get_key_type();
  if (ikey_type != UNC_KT_VBR_POLICINGMAP) {
    UPLL_LOG_DEBUG(" Invalid key type received. Key type - %d", ikey_type);
    return UPLL_RC_ERR_GENERIC;
  }

  key_vbr_t *pkey = reinterpret_cast<key_vbr_t*>(ikey->get_key());
  if (!pkey) {
    UPLL_LOG_DEBUG(" Input vbr policing map key is NULL ");
    return UPLL_RC_ERR_GENERIC;
  }

  key_vbr_t *vbr_key = reinterpret_cast<key_vbr_t*>
      (ConfigKeyVal::Malloc(sizeof(key_vbr_t)));

  memcpy(vbr_key, reinterpret_cast<key_vbr_t*>(pkey), sizeof(key_vbr_t));
  okey = new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVbr, vbr_key, NULL);
  SET_USER_DATA(okey, ikey);
  return UPLL_RC_SUCCESS;
}
upll_rc_t VbrPolicingMapMoMgr::ReadSiblingCount(IpcReqRespHeader *req,
    ConfigKeyVal* ikey,
    DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  controller_domain ctrlr_dom;
  upll_rc_t result_code  = UPLL_RC_SUCCESS;
  result_code = ValidateMessage(req, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("ValidateMessage failed result_code %d",
                   result_code);
    return result_code;
  }
  if (UNC_KT_VBR_POLICINGMAP_ENTRY != ikey->get_key_type() &&
      req->datatype != UPLL_DT_STATE) {
    result_code = ReadInfoFromDB(req, ikey, dmi, &ctrlr_dom);
    return result_code;
  }
  ConfigKeyVal *temp_vbr_key = NULL;
  ConfigKeyVal  *tctrl_key = NULL;
  result_code = GetReadVbrKey(temp_vbr_key, ikey);
  DbSubOp dbop = {kOpReadSingle, kOpMatchNone, kOpInOutNone};
  result_code = ReadConfigDB(temp_vbr_key, req->datatype, UNC_OP_READ,
                             dbop, dmi, MAINTBL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("ReadConfigDB failed %d", result_code);
    delete temp_vbr_key;
    return result_code;
  }
  val_policingmap_t *val_pm = reinterpret_cast
      <val_policingmap_t *>(GetVal(temp_vbr_key));

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
  key_policingprofile_entry_t* l_pp_key = NULL;

  l_pp_key  = reinterpret_cast<key_policingprofile_entry_t*>
      (ConfigKeyVal::Malloc(sizeof(key_policingprofile_entry_t)));
  tctrl_key = new ConfigKeyVal(UNC_KT_POLICING_PROFILE_ENTRY,
                               IpctSt::kIpcStKeyPolicingprofileEntry,
                               l_pp_key, NULL);
  l_pp_key->sequence_num = reinterpret_cast <key_vbr_policingmap_entry*>
      (ikey->get_key())->sequence_num;
  uuu::upll_strncpy(l_pp_key->policingprofile_key.policingprofile_name,
                    val_pm->policer_name,
                    (kMaxLenPolicingProfileName + 1));

  // result_code = mgr->ReadInfoFromDB(req, ppe_ckv, dmi, &ctrlr_dom);
  result_code = mgr->ReadSiblingMo(temp_req, tctrl_key, dmi);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Read sibling of ppe failed (%d)", result_code);
    return result_code;
  }
  ConfigKeyVal *temp_ppe_ckv = tctrl_key;
  uint8_t sibling_count = 0;
  while (temp_ppe_ckv !=NULL) {
    sibling_count++;
    temp_ppe_ckv = temp_ppe_ckv->get_next_cfg_key_val();
  }
  uint32_t *sib_count =
      reinterpret_cast<uint32_t*>(ConfigKeyVal::Malloc(sizeof(uint32_t)));
  *sib_count = sibling_count;
  ikey->SetCfgVal(new ConfigVal(IpctSt::kIpcStUint32, sib_count));
  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrPolicingMapMoMgr::OnPolicerFullAlarm(
       string ctrlr_name ,
       string domain_id,
       const key_vtn_t &key_vtn,
       const pfcdrv_policier_alarm_data_t &alarm_data,
       bool &alarm_raised,
       DalDmlIntf *dmi ) {
  controller_domain ctrlr_dom;
  ConfigKeyVal *ikey = NULL;
  char *alarm_status = NULL;
  char al_raised[] = "Raised";
  char al_cleared[] = "Cleared";
  key_vtn_t *vtn_key = NULL;
  key_vbr_t *vbr_key = NULL;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  bool is_vbr = false;
  VtnPolicingMapMoMgr *mgrvtnpom = NULL;
  VtnMoMgr *mgrvtn = NULL;
  VbrMoMgr *mgrvbr = NULL;
  char *vtn_name = NULL;
  char *vbr_name = NULL;

  if (!strlen(reinterpret_cast<char*>
              (const_cast<uint8_t*>(alarm_data.vnode_name)))) {
    is_vbr = false;
  } else {
    is_vbr = true;
  }

  if (is_vbr) {
    result_code = GetChildConfigKey(ikey, NULL);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("GetChildConfigKey failed %d", result_code);
      return result_code;
    }

    vbr_key = reinterpret_cast<key_vbr_t*>(ikey->get_key());
    uuu::upll_strncpy(vbr_key->vtn_key.vtn_name, key_vtn.vtn_name,
                      (kMaxLenVtnName + 1));
    uuu::upll_strncpy(vbr_key->vbridge_name, alarm_data.vnode_name,
                      (kMaxLenVnodeName + 1));
    mgrvbr = reinterpret_cast<VbrMoMgr*>
        (const_cast<MoManager*>(GetMoManager(UNC_KT_VBRIDGE)));
    if (NULL == mgrvbr) {
      UPLL_LOG_DEBUG("unable to get VbrMoMgr object to validate key_vbr");
      return UPLL_RC_ERR_GENERIC;
    }

    result_code = mgrvbr->ValidateVbrKey(vbr_key);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG(" Vbr key syntax validation failed :" "Err Code - %d",
                     result_code);
      return result_code;
    }
  } else {
    mgrvtnpom = reinterpret_cast<VtnPolicingMapMoMgr*>
        (const_cast<MoManager*>(GetMoManager(UNC_KT_VTN_POLICINGMAP)));
    result_code = GetChildConfigKey(ikey, NULL);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("GetChildConfigKey failed %d", result_code);
      return result_code;
    }
    vtn_key = reinterpret_cast<key_vtn_t*>(ikey->get_key());
    uuu::upll_strncpy(vtn_key->vtn_name, key_vtn.vtn_name,
                      (kMaxLenVtnName + 1));

    mgrvtn = reinterpret_cast<VtnMoMgr*>
        (const_cast<MoManager*>(GetMoManager(UNC_KT_VTN)));
    if (NULL == mgrvtn) {
      UPLL_LOG_DEBUG("unable to get VtnMoMgr object to validate key_vtn");
      return UPLL_RC_ERR_GENERIC;
    }

    result_code = mgrvtn->ValidateVtnKey(vtn_key);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("Vtn Key validation failed - %d", result_code);
      delete ikey;
      return result_code;
    }
  }
  memset(&ctrlr_dom, 0, sizeof(controller_domain));
  ctrlr_dom.ctrlr = reinterpret_cast<uint8_t*>
      (const_cast<char*>(ctrlr_name.c_str()));
  ctrlr_dom.domain = reinterpret_cast<uint8_t*>
      (const_cast<char*>(domain_id.c_str()));
  SET_USER_DATA_CTRLR_DOMAIN(ikey, ctrlr_dom);

  if (is_vbr) {
    result_code = GetRenamedControllerKey(ikey, UPLL_DT_RUNNING,
                                           dmi, &ctrlr_dom);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("GetRenamedControllerKey failed %d", result_code);
      return result_code;
    }
    vbr_key = reinterpret_cast<key_vbr_t*>(ikey->get_key());
    vtn_name = reinterpret_cast<char*>(vbr_key->vtn_key.vtn_name);
    vbr_name = reinterpret_cast<char*>(vbr_key->vbridge_name);
  } else {
    result_code =  mgrvtnpom->GetRenamedControllerKey(ikey, UPLL_DT_RUNNING,
                                                   dmi, &ctrlr_dom);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("GetRenamedControllerKey failed %d", result_code);
      return result_code;
    }
    vtn_key = reinterpret_cast<key_vtn_t*>(ikey->get_key());
    vtn_name = reinterpret_cast<char*>(vtn_key->vtn_name);
  }

  if (alarm_raised) {
    alarm_status = al_raised;
  } else {
    alarm_status = al_cleared;
  }

  UPLL_LOG_INFO("Policer Full alarm : status - %s controller - %s, domain - %s,"
                "vtn - %s, vnode - %s, port - %s, dpid - %"PFC_PFMT_u64,
                alarm_status, ctrlr_name.c_str(), domain_id.c_str(),
                vtn_name, vbr_name, reinterpret_cast<char*>
                (const_cast<uint8_t*>(alarm_data.port_name)),
                alarm_data.ofs_dpid);

  return result_code;
}

upll_rc_t VbrPolicingMapMoMgr::OnPolicerFailAlarm(
       string ctrlr_name ,
       string domain_id,
       const key_vtn_t &key_vtn,
       const pfcdrv_policier_alarm_data_t &alarm_data,
       bool &alarm_raised,
       DalDmlIntf *dmi ) {
  controller_domain ctrlr_dom;
  ConfigKeyVal *ikey = NULL;
  char *alarm_status = NULL;
  char al_raised[] = "Raised";
  char al_cleared[] = "Cleared";
  key_vtn_t *vtn_key = NULL;
  key_vbr_t *vbr_key = NULL;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  bool is_vbr = false;
  VtnPolicingMapMoMgr *mgrvtnpom = NULL;
  VtnMoMgr *mgrvtn = NULL;
  VbrMoMgr *mgrvbr = NULL;
  char *vtn_name = NULL;
  char *vbr_name = NULL;

  if (!strlen(reinterpret_cast<char*>
              (const_cast<uint8_t*>(alarm_data.vnode_name)))) {
    is_vbr = false;
  } else {
    is_vbr = true;
  }

  if (is_vbr) {
    result_code = GetChildConfigKey(ikey, NULL);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("GetChildConfigKey failed %d", result_code);
      return result_code;
    }

    vbr_key = reinterpret_cast<key_vbr_t*>(ikey->get_key());
    uuu::upll_strncpy(vbr_key->vtn_key.vtn_name, key_vtn.vtn_name,
                      (kMaxLenVtnName + 1));
    uuu::upll_strncpy(vbr_key->vbridge_name, alarm_data.vnode_name,
                      (kMaxLenVnodeName + 1));
    mgrvbr = reinterpret_cast<VbrMoMgr*>
        (const_cast<MoManager*>(GetMoManager(UNC_KT_VBRIDGE)));
    if (NULL == mgrvbr) {
      UPLL_LOG_DEBUG("unable to get VbrMoMgr object to validate key_vbr");
      return UPLL_RC_ERR_GENERIC;
    }

    result_code = mgrvbr->ValidateVbrKey(vbr_key);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG(" Vbr key syntax validation failed :" "Err Code - %d",
                     result_code);
      return result_code;
    }
  } else {
    mgrvtnpom = reinterpret_cast<VtnPolicingMapMoMgr*>
        (const_cast<MoManager*>(GetMoManager(UNC_KT_VTN_POLICINGMAP)));
    result_code = GetChildConfigKey(ikey, NULL);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("GetChildConfigKey failed %d", result_code);
      return result_code;
    }
    vtn_key = reinterpret_cast<key_vtn_t*>(ikey->get_key());
    uuu::upll_strncpy(vtn_key->vtn_name, key_vtn.vtn_name,
                      (kMaxLenVtnName + 1));

    mgrvtn = reinterpret_cast<VtnMoMgr*>
        (const_cast<MoManager*>(GetMoManager(UNC_KT_VTN)));
    if (NULL == mgrvtn) {
      UPLL_LOG_DEBUG("unable to get VtnMoMgr object to validate key_vtn");
      return UPLL_RC_ERR_GENERIC;
    }

    result_code = mgrvtn->ValidateVtnKey(vtn_key);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("Vtn Key validation failed - %d", result_code);
      delete ikey;
      return result_code;
    }
  }
  memset(&ctrlr_dom, 0, sizeof(controller_domain));
  ctrlr_dom.ctrlr = reinterpret_cast<uint8_t*>
      (const_cast<char*>(ctrlr_name.c_str()));
  ctrlr_dom.domain = reinterpret_cast<uint8_t*>
      (const_cast<char*>(domain_id.c_str()));
  SET_USER_DATA_CTRLR_DOMAIN(ikey, ctrlr_dom);

  if (is_vbr) {
    result_code = GetRenamedControllerKey(ikey, UPLL_DT_RUNNING,
                                           dmi, &ctrlr_dom);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("GetRenamedControllerKey failed %d", result_code);
      return result_code;
    }
    vbr_key = reinterpret_cast<key_vbr_t*>(ikey->get_key());
    vtn_name = reinterpret_cast<char*>(vbr_key->vtn_key.vtn_name);
    vbr_name = reinterpret_cast<char*>(vbr_key->vbridge_name);
  } else {
    result_code =  mgrvtnpom->GetRenamedControllerKey(ikey, UPLL_DT_RUNNING,
                                                   dmi, &ctrlr_dom);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("GetRenamedControllerKey failed %d", result_code);
      return result_code;
    }
    vtn_key = reinterpret_cast<key_vtn_t*>(ikey->get_key());
    vtn_name = reinterpret_cast<char*>(vtn_key->vtn_name);
  }

  if (alarm_raised) {
    alarm_status = al_raised;
  } else {
    alarm_status = al_cleared;
  }

  UPLL_LOG_INFO("Policer Fail alarm : status - %s controller - %s, domain - %s,"
                "vtn - %s, vnode - %s, port - %s, dpid - %"PFC_PFMT_u64,
                alarm_status, ctrlr_name.c_str(), domain_id.c_str(),
                vtn_name, vbr_name, reinterpret_cast<char*>
                (const_cast<uint8_t*>(alarm_data.port_name)),
                alarm_data.ofs_dpid);

  return result_code;
}
}  // kt_momgr
}  // upll
}  // unc

