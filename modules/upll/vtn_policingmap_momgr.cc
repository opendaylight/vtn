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
#include "policingprofile_momgr.hh"
#include "policingprofile_entry_momgr.hh"
#include "vtn_policingmap_momgr.hh"
#include "unc/upll_errno.h"
#include "upll_validation.hh"
#include "unc/upll_ipc_enum.h"
#include "upll_log.hh"
using unc::upll::ipc_util::IpcUtil;
namespace unc {
namespace upll {
namespace kt_momgr {
#define VTN_KEY_COL    3
#define POLICY_KEY_COL 4
#define VTN_RENAME 0x01
#define POLICINGPROFILE_RENAME 0x04

// VtnPolicingmap Table (Main Table)
BindInfo VtnPolicingMapMoMgr::vtnpolicingmap_bind_info[] = {
  { uudst::vtn_policingmap::kDbiVtnName, CFG_KEY,
    offsetof(key_vtn_t, vtn_name),
    uud::kDalChar, (kMaxLenVtnName + 1) },
  { uudst::vtn_policingmap::kDbiPolicername, CFG_VAL,
    offsetof(val_policingmap_t, policer_name),
    uud::kDalChar, (kMaxLenPolicingProfileName + 1) },
  { uudst::vtn_policingmap::kDbiFlags, CK_VAL,
    offsetof(key_user_data_t, flags),
    uud::kDalUint8, 1 },
  { uudst::vtn_policingmap::kDbiValidPolicername, CFG_META_VAL,
    offsetof(val_policingmap_t, valid[0]),
    uud::kDalUint8, 1 },
  { uudst::vtn_policingmap::kDbiCsRowStatus, CS_VAL,
    offsetof(val_policingmap_t, cs_row_status),
    uud::kDalUint8, 1 },
  { uudst::vtn_policingmap::kDbiCsPolicername, CS_VAL,
    offsetof(val_policingmap_t, cs_attr[0]),
    uud::kDalUint8, 1 }
};

// Vtn PolicingmapCtrlr Table (Ctrlr Table)
BindInfo VtnPolicingMapMoMgr::vtnpolicingmap_controller_bind_info[] = {
  { uudst::vtn_policingmap_ctrlr::kDbiVtnName, CFG_KEY,
    offsetof(key_vtn_t, vtn_name),
    uud::kDalChar, (kMaxLenVtnName + 1) },
  { uudst::vtn_policingmap_ctrlr::kDbiCtrlrname, CK_VAL,
    offsetof(key_user_data_t, ctrlr_id),
    uud::kDalChar, (kMaxLenCtrlrId + 1) },
  { uudst::vtn_policingmap_ctrlr::kDbiDomainId, CK_VAL,
    offsetof(key_user_data_t, domain_id),
    uud::kDalChar, (kMaxLenDomainId + 1) },
  { uudst::vtn_policingmap_ctrlr::kDbiPolicername, CFG_VAL,
    offsetof(val_vtnpolicingmap_ctrl_t, policer_name),
    uud::kDalChar, (kMaxLenPolicingProfileName + 1) },
  { uudst::vtn_policingmap_ctrlr::kDbiFlags, CK_VAL,
    offsetof(key_user_data_t, flags),
    uud::kDalUint8, 1 },
  { uudst::vtn_policingmap_ctrlr::kDbiValidPolicername, CFG_META_VAL,
    offsetof(val_vtnpolicingmap_ctrl_t, valid[0]),
    uud::kDalUint8, 1 },
  { uudst::vtn_policingmap_ctrlr::kDbiCsRowStatus, CS_VAL,
    offsetof(val_vtnpolicingmap_ctrl_t, cs_row_status),
    uud::kDalUint8, 1 },
  { uudst::vtn_policingmap_ctrlr::kDbiCsPolicername, CS_VAL,
    offsetof(val_vtnpolicingmap_ctrl_t, cs_attr[0]),
    uud::kDalUint8, 1 }
};

// Rename VTN name MAINTBL
BindInfo VtnPolicingMapMoMgr::key_vtnpm_vtn_maintbl_rename_bind_info[] = {
  { uudst::vtn_policingmap::kDbiVtnName, CFG_MATCH_KEY,
    offsetof(key_vtn_t, vtn_name),
    uud::kDalChar, (kMaxLenVtnName + 1) },
  { uudst::vtn_policingmap::kDbiVtnName, CFG_INPUT_KEY,
    offsetof(key_rename_vnode_info_t, new_unc_vtn_name),
    uud::kDalChar, (kMaxLenVtnName + 1) },
  { uudst::vtn_policingmap::kDbiFlags, CFG_INPUT_KEY,
    offsetof(key_user_data_t, flags),
    uud::kDalUint8, 1 }
};

// Rename VTN name CTRLRTBL
BindInfo VtnPolicingMapMoMgr::key_vtnpm_vtn_ctrlrtbl_rename_bind_info[] = {
  { uudst::vtn_policingmap_ctrlr::kDbiVtnName, CFG_MATCH_KEY,
    offsetof(key_vtn_t, vtn_name),
    uud::kDalChar, (kMaxLenVtnName + 1) },
  { uudst::vtn_policingmap_ctrlr::kDbiVtnName, CFG_INPUT_KEY,
    offsetof(key_rename_vnode_info_t, new_unc_vtn_name),
    uud::kDalChar, (kMaxLenVtnName + 1) },
  { uudst::vtn_policingmap_ctrlr::kDbiFlags, CFG_INPUT_KEY,
    offsetof(key_user_data_t, flags),
    uud::kDalUint8, 1 }
};

// Rename Policer name MAINTBL
BindInfo VtnPolicingMapMoMgr::key_vtnpm_Policyname_maintbl_rename_bind_info[]
= {
  { uudst::vtn_policingmap::kDbiVtnName, CFG_MATCH_KEY,
    offsetof(key_vtn_t, vtn_name),
    uud::kDalChar, (kMaxLenVtnName + 1) },
  { uudst::vtn_policingmap::kDbiPolicername, CFG_MATCH_KEY,
    offsetof(key_rename_vnode_info_t, old_policingprofile_name),
    uud::kDalChar, (kMaxLenPolicingProfileName + 1) },
  { uudst::vtn_policingmap::kDbiPolicername, CFG_INPUT_KEY,
    offsetof(key_rename_vnode_info_t, new_policingprofile_name),
    uud::kDalChar, (kMaxLenPolicingProfileName + 1) },
  { uudst::vtn_policingmap::kDbiFlags, CFG_INPUT_KEY,
    offsetof(key_user_data_t, flags),
    uud::kDalUint8, 1 }
};

// Rename Policer name CTRLRTBL
BindInfo VtnPolicingMapMoMgr::key_vtnpm_Policyname_ctrlrtbl_rename_bind_info[]
= {
  { uudst::vtn_policingmap_ctrlr::kDbiVtnName, CFG_MATCH_KEY,
    offsetof(key_vtn_t, vtn_name),
    uud::kDalChar, (kMaxLenVtnName + 1) },
  { uudst::vtn_policingmap_ctrlr::kDbiPolicername, CFG_MATCH_KEY,
    offsetof(key_rename_vnode_info_t, old_policingprofile_name),
    uud::kDalChar, (kMaxLenPolicingProfileName + 1) },
  { uudst::vtn_policingmap_ctrlr::kDbiPolicername, CFG_INPUT_KEY,
    offsetof(key_rename_vnode_info_t, new_policingprofile_name),
    uud::kDalChar, (kMaxLenPolicingProfileName + 1) },
  { uudst::vtn_policingmap_ctrlr::kDbiFlags, CFG_INPUT_KEY,
    offsetof(key_user_data_t, flags),
    uud::kDalUint8, 1 }
};

// Constructor
VtnPolicingMapMoMgr::VtnPolicingMapMoMgr() : MoMgrImpl() {
  UPLL_FUNC_TRACE;
  // Rename operation is not support for this KT
  // setting max tables to 2
  ntable = MAX_MOMGR_TBLS;
  table = new Table *[ntable];

  table[MAINTBL] = new Table(uudst::kDbiVtnPolicingMapTbl,
      UNC_KT_VTN_POLICINGMAP, vtnpolicingmap_bind_info,
      IpctSt::kIpcStKeyVtn, IpctSt::kIpcStValPolicingmap,
      uudst::vtn_policingmap::kDbiVtnPolicingMapNumCols);

  table[RENAMETBL] = NULL;

  /* For Controller Table */
  table[CTRLRTBL] = new Table(uudst::kDbiVtnPolicingMapCtrlrTbl,
      UNC_KT_VTN_POLICINGMAP, vtnpolicingmap_controller_bind_info,
      IpctSt::kIpcStKeyVtnPolicingmapController,
      IpctSt::kIpcStValPolicingmapController,
      uudst::vtn_policingmap_ctrlr::kDbiVtnPolicingMapCtrlrNumCols);

  nchild = 0;
  child = NULL;
}

upll_rc_t VtnPolicingMapMoMgr::CreateCandidateMo(IpcReqRespHeader *req,
    ConfigKeyVal *ikey,
    DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  if (NULL == ikey || NULL == req || NULL == dmi) {
    UPLL_LOG_DEBUG("ikey or req or dmi NULL");
    return UPLL_RC_ERR_BAD_REQUEST;
  }
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  bool ctrl_instance = true;

  UPLL_LOG_DEBUG("Data Check (%d) (%d)", req->operation, req->datatype);

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

  // Capability Check
#if 0
  result_code = ValidateCapability(req, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Key not supported by controller");
    return result_code;
  }
#endif

  
  // Check VTNPM object already exists in VtnPolicingMap tbl CANDIDATE DB
  // if record exists, return the error code
  result_code = IsReferenced(ikey, req->datatype, dmi);
  if (result_code == UPLL_RC_ERR_INSTANCE_EXISTS) {
    UPLL_LOG_DEBUG("Object exists in CANDIDATE DB (%d)", result_code);
    return result_code;
  } else if (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    UPLL_LOG_DEBUG("Instance not Available");
    return result_code;
  }

  // Check Policingmap object exists in PolicingProfileTbl CANDIDATE DB
  // If record not exists, return error code
  val_policingmap_t *val_pm = reinterpret_cast<val_policingmap_t *>
    (GetVal(ikey));
  if (NULL == val_pm) {
    UPLL_LOG_DEBUG(" Value structure is NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  if (UNC_VF_VALID == val_pm->valid[UPLL_IDX_POLICERNAME_PM]) {
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

    // 1)Check the ref count capability of vtn associated controller name in
    // policingprofilectrltbl If any of one refcount reached the
    // max capability, then return error
    // If all the ref count is less than capability
    // then increment and update the refcount if record exist in
    // policingprofilectrltbl and create the record with refcount if record not
    // exist in policingprofilectrl tbl for the vtn associated controller name

    // 2)Create the record in policingprofileentryctrltbl
    result_code = UpdateRefCountInPPCtrlr(ikey, req->datatype, dmi,
        UNC_OP_CREATE);
    if (UPLL_RC_SUCCESS != result_code) {
      if (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
        UPLL_LOG_DEBUG("UpdateRefCountInPPCtrlr Err in CANDIDATE DB(%d)",
          result_code);
        return result_code;
      }
      ctrl_instance = false;
    }
  }
  // create a record in vtnpolicingmap CANDIDATE DB
  result_code = UpdateConfigDB(ikey, req->datatype, req->operation, dmi,
      MAINTBL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("Create record Err in vtnpolicingmaptbl CANDIDATE DB(%d)",
        result_code);
    return result_code;
  }
  key_vtn_t *tmp_key = reinterpret_cast<key_vtn_t *>(ikey->get_key());
  UPLL_LOG_DEBUG("vtn name in createcand %s", tmp_key->vtn_name);
  // Create the record in vtnpolicingmapctrltbl
  if (ctrl_instance) {
    result_code = UpdateRecordInVtnPmCtrlr(ikey, req->datatype, UNC_OP_CREATE,
      dmi);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("Create record Err in vtnpolicingmaptbl CANDIDATE DB(%d)",
        result_code);
      return result_code;
    }
  }
  return result_code;
}

upll_rc_t VtnPolicingMapMoMgr::DeleteMo(IpcReqRespHeader *req,
    ConfigKeyVal *ikey, DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  if (NULL == ikey || NULL == req || NULL == dmi) {
    UPLL_LOG_DEBUG("DeleteMo ikey and req NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  // validate syntax and semantics
  result_code = ValidateMessage(req, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("ValidateMessage Err (%d)", result_code);
    return result_code;
  }

  // Check VTNPM object already exists in VtnPolicingMap CANDIDATE DB
  // If record not exists, return error
  result_code = IsReferenced(ikey, req->datatype, dmi);
  if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    UPLL_LOG_DEBUG("DeleteMo Instance not Available (%d)", result_code);
    return result_code;
  } else if (result_code == UPLL_RC_ERR_INSTANCE_EXISTS) {
    UPLL_LOG_DEBUG("DeleteMo Instance Available");
  } else  {
    UPLL_LOG_DEBUG(
        "DeleteMo IsReferenced Error accessing DB (%d)", result_code);
    return result_code;
  }

  // 1)Decerment the refcount and check if the refcount as 0 then remove
  // the record and if refcount is not 0, then update the refcount in
  // policingprofilectrltbl

  // 2)Delete the record in policingprofileentryctrltbl
  ConfigKeyVal *tempckv = NULL;
  result_code = GetChildConfigKey(tempckv, ikey);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG(" GetChildConfigKey failed");
    return result_code;
  }
  DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutNone };
  result_code = ReadConfigDB(tempckv, req->datatype, UNC_OP_READ, dbop, dmi,
      MAINTBL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG(" ReadConfigDB failed ");
    return result_code;
  }
  val_policingmap_t *val_pm = reinterpret_cast<val_policingmap_t *>
    (GetVal(tempckv));
  if (UNC_VF_VALID == val_pm->valid[UPLL_IDX_POLICERNAME_PM]) {
    result_code = UpdateRefCountInPPCtrlr(tempckv, req->datatype, dmi,
        UNC_OP_DELETE);
    if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
      result_code = UPLL_RC_SUCCESS;
    } else if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("DeleteMo UpdateRefCountInPPCtrlr Error DB (%d)",
          result_code);
      return result_code;
    }
  }
  // Delete a record in vtnpolicingmap CANDIDATE DB
  result_code = UpdateConfigDB(ikey, req->datatype, req->operation, dmi,
      MAINTBL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("DeleteMo record Err in vtnpolicingmaptbl (%d)",
        result_code);
    return result_code;
  }

  // Delete the record in vtnpolicingmapctrltbl
  result_code = UpdateRecordInVtnPmCtrlr(tempckv, req->datatype, UNC_OP_DELETE,
      dmi);
  if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
    result_code = UPLL_RC_SUCCESS;
  } else if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("DeleteMo record Err in vtnpolicingmapctrltbl (%d)",
        result_code);
    return result_code;
  }

  CONFIGKEYVALCLEAN(tempckv);
  return result_code;
}

upll_rc_t VtnPolicingMapMoMgr::UpdateMo(IpcReqRespHeader *req,
    ConfigKeyVal *ikey, DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  if (NULL == ikey || NULL == req || NULL == dmi) {
    UPLL_LOG_DEBUG("UpdateMo ikey and req NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  ConfigKeyVal *tmp_key = NULL;

  // validate syntax and semantics
  result_code = ValidateMessage(req, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("UpdateMo ValidateMessage Err  (%d)", result_code);
    return result_code;
  }

  // Capability Check
#if 0
  result_code = ValidateCapability(req, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Key not supported by controller");
    return result_code;
  }
#endif

  // Check if VTNPM object exists in VtnPolicingMap CANDIDATE DB
  // If record not exists, return error
  result_code = IsReferenced(ikey, req->datatype, dmi);
  if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    UPLL_LOG_DEBUG("UpdateMo IsReferenced record not available (%d)",
        result_code);
    return result_code;
  } else if (result_code == UPLL_RC_ERR_INSTANCE_EXISTS) {
    UPLL_LOG_DEBUG("UpdateMo IsReferenced record available");
  } else if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("UpdateMo IsReferenced error accessing DB (%d)",
        result_code);
    return result_code;
  }

  // Check Policingmap object exists in PolicingProfileTbl CANDIDATE DB
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
  /* 1)Check the ref count capability of vtn associated controller name in
     policingprofilectrltbl If any of one refcount reached the
     max capability, then return error
     If all the ref count is less than capability
     then increment and update the refcount if record exist in
     policingprofilectrltbl and create the record with refcount if record not
     exist in policingprofilectrl tbl for the vtn associated controller name
     2)Update the record in policingprofileentryctrltbl
     */
  ConfigKeyVal *tmpckv = NULL;
  result_code = GetChildConfigKey(tmpckv, ikey);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetChildConfigKey failed");
    return result_code;
  }
  DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutNone };
  result_code = ReadConfigDB(tmpckv, req->datatype, UNC_OP_READ, dbop, dmi,
      MAINTBL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("ReadConfigDB failed");
    return result_code;
  }
  val_policingmap_t *val_tmp_val = reinterpret_cast<val_policingmap_t *>
    (GetVal(tmpckv));
  if (UNC_VF_VALID == val_ival->valid[UPLL_IDX_POLICERNAME_PM] &&
      UNC_VF_VALID == val_tmp_val->valid[UPLL_IDX_POLICERNAME_PM]) {
    UPLL_LOG_DEBUG(" Policer name valid in DB and ikey");
    result_code = UpdateRefCountInPPCtrlr(tmpckv, req->datatype, dmi,
        UNC_OP_DELETE);
    if (UPLL_RC_SUCCESS != result_code) {
      if (UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
        UPLL_LOG_DEBUG("UpdateMo UpdateRefCountInPPCtrlr error in delete (%d)",
          result_code);
        return result_code;
      }
    }
    result_code = UpdateRefCountInPPCtrlr(ikey, req->datatype, dmi,
        UNC_OP_CREATE);
    if (UPLL_RC_SUCCESS != result_code) {
      if (UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
        UPLL_LOG_DEBUG("UpdateMo UpdateRefCountInPPCtrlr error in create (%d)",
            result_code);
        return result_code;
      }
    }
  } else if ((UNC_VF_VALID_NO_VALUE == val_tmp_val->
      valid[UPLL_IDX_POLICERNAME_PM] ||
      UNC_VF_INVALID == val_tmp_val->valid[UPLL_IDX_POLICERNAME_PM])&&
      UNC_VF_VALID == val_ival->valid[UPLL_IDX_POLICERNAME_PM]) {
    UPLL_LOG_DEBUG(" Policer name valid in ikey and validnovalue in DB");
    result_code = UpdateRefCountInPPCtrlr(ikey, req->datatype, dmi,
        UNC_OP_CREATE);
    if (UPLL_RC_SUCCESS != result_code) {
      if (UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
        UPLL_LOG_DEBUG("UpdateMo UpdateRefCountInPPCtrlr error in create (%d)",
            result_code);
        return result_code;
      }
    }
  } else if (UNC_VF_VALID == val_tmp_val->valid[UPLL_IDX_POLICERNAME_PM] &&
      UNC_VF_VALID_NO_VALUE == val_ival->
      valid[UPLL_IDX_POLICERNAME_PM]) {
    UPLL_LOG_DEBUG(" Policer name validnovalue in ikey and valid in db");
    result_code = UpdateRefCountInPPCtrlr(tmpckv, req->datatype, dmi,
        UNC_OP_DELETE);
    if (UPLL_RC_SUCCESS != result_code) {
      if (UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
        UPLL_LOG_DEBUG("UpdateMo UpdateRefCountInPPCtrlr error in delete (%d)",
            result_code);
        return result_code;
      }
    }
  }
  /* Update the record in CANDIDATE DB */
  result_code = UpdateConfigDB(ikey, req->datatype, req->operation, dmi,
      MAINTBL);
  if (UPLL_RC_SUCCESS != result_code) {
    CONFIGKEYVALCLEAN(tmp_key);
    UPLL_LOG_DEBUG("Create record Err in vtnpolicingmaptbl CANDIDATE DB(%d)",
        result_code);
    return result_code;
  }

  /* Update the record in vtnpolicingmapctrltbl */
  result_code = UpdateRecordInVtnPmCtrlr(ikey, req->datatype, UNC_OP_UPDATE,
      dmi);
  if (UPLL_RC_SUCCESS != result_code) {
    if (UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
      CONFIGKEYVALCLEAN(tmp_key);
      UPLL_LOG_DEBUG("Create record Err in vtnpolicingmaptbl CANDIDATE DB(%d)",
          result_code);
      return result_code;
    }
  }
  CONFIGKEYVALCLEAN(tmp_key);
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtnPolicingMapMoMgr::IsReferenced(ConfigKeyVal *ikey,
    upll_keytype_datatype_t dt_type,
    DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  if (NULL == ikey) return UPLL_RC_ERR_GENERIC;
  DbSubOp dbop = { kOpReadExist, kOpMatchNone, kOpInOutNone };

  // Check the exixtence in vtnpolicingmaptbl
  result_code = UpdateConfigDB(ikey, dt_type, UNC_OP_READ, dmi, &dbop, MAINTBL);
  UPLL_LOG_DEBUG("VtnPolicingMapMoMgr::IsReferenced IsReferenced status");

  return result_code;
}

upll_rc_t VtnPolicingMapMoMgr::IsPolicyProfileReferenced(
    ConfigKeyVal *ikey, upll_keytype_datatype_t dt_type, DalDmlIntf* dmi,
    unc_keytype_operation_t op) {

  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  ConfigKeyVal *okey = NULL;
  // Get the memory allocated policy profile key structure
  MoMgrImpl *mgr =
    reinterpret_cast<MoMgrImpl *>((const_cast<MoManager*>(GetMoManager(
              UNC_KT_POLICING_PROFILE))));
  if (!mgr) {
    UPLL_LOG_DEBUG("IsPolicyProfileReferenced mgr NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  result_code = mgr->GetChildConfigKey(okey, NULL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("IsPolicyProfileReferenced GetChildConfigKey (%d)",
        result_code);
    return result_code;
  }
  if (!okey) return UPLL_RC_ERR_GENERIC;
  // Get the policer name and fill in policingprofile key structure
  uuu::upll_strncpy(
      reinterpret_cast<key_policingprofile_t *>
      (okey->get_key())->policingprofile_name,
      reinterpret_cast<val_policingmap_t*>
      (ikey->get_cfg_val()->get_val())->policer_name,
      (kMaxLenPolicingProfileName + 1));
  DbSubOp dbop = { kOpReadExist, kOpMatchNone, kOpInOutNone };

  // Check the existence in policingprofiletbl
  result_code = mgr->UpdateConfigDB(okey, dt_type, op, dmi, &dbop, MAINTBL);
  if (UPLL_RC_ERR_INSTANCE_EXISTS != result_code) {
    UPLL_LOG_DEBUG("UpdateConfigDB failed %d", result_code);
    delete okey;
    return result_code;
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
    return result_code;
  }
  CONFIGKEYVALCLEAN(okey);
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtnPolicingMapMoMgr::UpdateRefCountInPPCtrlr(
    ConfigKeyVal *ikey, upll_keytype_datatype_t dt_type, DalDmlIntf *dmi,
    unc_keytype_operation_t op) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  ConfigKeyVal *okey = NULL;
  uint8_t *ctrlr_id = NULL;

  // Get the momory alloctaed vtn key structure
  VtnMoMgr *vtnmgr =
    static_cast<VtnMoMgr *>((const_cast<MoManager *>
          (GetMoManager(UNC_KT_VTN))));
  result_code = vtnmgr->GetChildConfigKey(okey, NULL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("UpdateRefCountInPPCtrlr GetChildConfigKey error (%d)",
        result_code);
    return result_code;
  }
  if (!okey) return UPLL_RC_ERR_GENERIC;
  // To get the vtn associated controller name
  key_vtn_t *vtn_okey = reinterpret_cast<key_vtn_t *>(okey->get_key());
  key_vtn_t *vtn_ikey = reinterpret_cast<key_vtn_t *>(ikey->get_key());
  uuu::upll_strncpy(vtn_okey->vtn_name, vtn_ikey->vtn_name,
      kMaxLenVtnName+1);
  result_code = vtnmgr->GetControllerDomainSpan(okey, UPLL_DT_CANDIDATE, dmi);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetControllerSpan  no instance/error (%d)", result_code);
    CONFIGKEYVALCLEAN(okey);
    return result_code;
  }

  PolicingProfileMoMgr *pp_mgr =
    reinterpret_cast<PolicingProfileMoMgr *>
    (const_cast<MoManager *>(GetMoManager(UNC_KT_POLICING_PROFILE)));
  /*
  PolicingProfileEntryMoMgr *ppe_mgr =
    reinterpret_cast<PolicingProfileEntryMoMgr *>
    (const_cast<MoManager *>(GetMoManager(
                                          UNC_KT_POLICING_PROFILE_ENTRY)));*/

  val_policingmap_t* val_vtn_policingmap =
    reinterpret_cast<val_policingmap_t *>(GetVal(ikey));

  // Vtn Associated ctrl name
  ConfigKeyVal *tmp_ckv = okey;
  while (NULL != okey) {
    // Get the controller name
    GET_USER_DATA_CTRLR(okey, ctrlr_id);

    if (NULL == ctrlr_id) {
      UPLL_LOG_DEBUG("UpdateRefCountInPPCtrlr ctrlr_id NULL");
      CONFIGKEYVALCLEAN(okey);
      return UPLL_RC_ERR_GENERIC;
    }

    // Check the ref count capability of vtn associated controller name in
    // policingprofilectrltbl If any of one refcount reached the
    // max capability, then return error
    // If all the ref count is less than capability
    // then increment and update the refcount if record exist in
    // policingprofilectrltbl and create the record with refcount if record not
    // exist in policingprofilectrl tbl for the vtn associated controller name
    UPLL_LOG_DEBUG("PP name in GetPolicingProfileCtrlrKeyval %s",
        val_vtn_policingmap->policer_name);
    UPLL_LOG_DEBUG("Ctrlrid in GetPolicingProfileCtrlrKeyval %s",
        ctrlr_id);
    result_code = pp_mgr->PolicingProfileCtrlrTblOper(
        reinterpret_cast<const char *>(val_vtn_policingmap->policer_name),
        reinterpret_cast<const char *>(ctrlr_id), dmi, op, dt_type);
    if (UPLL_RC_SUCCESS != result_code) {
      CONFIGKEYVALCLEAN(okey);
      return result_code;
    }
    // Create the record in policingprofileentryctrltbl
    #if 0
    result_code = ppe_mgr->PolicingProfileEntryCtrlrTblOper(
        reinterpret_cast<const char *>(val_vtn_policingmap->policer_name),
        reinterpret_cast<const char *>(ctrlr_id), dmi, op, dt_type);
    if (UPLL_RC_SUCCESS != result_code) {
      CONFIGKEYVALCLEAN(okey);
      return result_code;
    }
    #endif
    okey = okey->get_next_cfg_key_val();
  }
  CONFIGKEYVALCLEAN(tmp_ckv);
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtnPolicingMapMoMgr::UpdateRecordInVtnPmCtrlr(
    ConfigKeyVal *ikey, upll_keytype_datatype_t dt_type,
    unc_keytype_operation_t op, DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *ctrlckv = NULL, *okey = NULL;
  key_vtn_t *vtn_key = reinterpret_cast<key_vtn_t *>(ikey->get_key());
  if (NULL == vtn_key) {
    UPLL_LOG_DEBUG("key is NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  VtnMoMgr *vtnmgr =
    static_cast<VtnMoMgr *>((const_cast<MoManager *>
          (GetMoManager(UNC_KT_VTN))));
  // Get the memory allocated vtn key structure
  result_code = vtnmgr->GetChildConfigKey(okey, NULL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG(" GetChildConfigKey error (%d)",
        result_code);
    return result_code;
  }
  if (!okey) return UPLL_RC_ERR_GENERIC;
  key_vtn_t *vtn_okey = reinterpret_cast<key_vtn_t *>(okey->get_key());
  if (NULL == vtn_okey) {
    UPLL_LOG_DEBUG("key is NULL");
    delete okey;
    return UPLL_RC_ERR_GENERIC;
  }
  uuu::upll_strncpy(vtn_okey->vtn_name, vtn_key->vtn_name,
      kMaxLenVtnName+1);
  UPLL_LOG_DEBUG(" vtn_name %s", vtn_okey->vtn_name);
  // Get the vtn associated controller name
  result_code = vtnmgr->GetControllerDomainSpan(okey, UPLL_DT_CANDIDATE, dmi);
  if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    UPLL_LOG_DEBUG(" No entry in vtn ctrlr tbl");
    return UPLL_RC_SUCCESS;
  }
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetControllerSpan error (%d)",
        result_code);
    CONFIGKEYVALCLEAN(okey);
    return result_code;
  }

  while (NULL != okey) {
    controller_domain ctrlr_dom;
    ctrlr_dom.ctrlr = NULL;
    ctrlr_dom.domain = NULL;
    GET_USER_DATA_CTRLR_DOMAIN(okey, ctrlr_dom);
    result_code = GetPMCtrlKeyval(ctrlckv, ikey, &ctrlr_dom);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("CheckRefCountCapability GetPMCtrlKeyval error (%d)",
          result_code);
      CONFIGKEYVALCLEAN(okey);
      CONFIGKEYVALCLEAN(ctrlckv);
      return result_code;
    }

    // Create/Update/Delete a record in CANDIDATE DB
    result_code = UpdateConfigDB(ctrlckv, dt_type, op, dmi, CTRLRTBL);
    if (UPLL_RC_SUCCESS != result_code) {
      CONFIGKEYVALCLEAN(okey);
      CONFIGKEYVALCLEAN(ctrlckv);
      UPLL_LOG_DEBUG("Create record Err in vtnpolicingmapctrlrtbl CAN DB(%d)",
          result_code);
      return result_code;
    }
    CONFIGKEYVALCLEAN(ctrlckv);
    okey = okey->get_next_cfg_key_val();
  }
  CONFIGKEYVALCLEAN(okey);
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtnPolicingMapMoMgr::UpdateControllerTableForVtn(
    uint8_t* vtn_name,
    controller_domain *ctrlr_dom,
    unc_keytype_operation_t op,
    DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  /* TODO */
  ConfigKeyVal *ctrlckv = NULL, *ikey = NULL;
  result_code = GetChildConfigKey(ikey, NULL);
  if (result_code != UPLL_RC_SUCCESS) {
    return result_code;
  }
  if (!ikey) return UPLL_RC_ERR_GENERIC;
  key_vtn_t *vtn_key = reinterpret_cast<key_vtn_t *>
      (ikey->get_key());
  uuu::upll_strncpy(vtn_key->vtn_name, vtn_name, kMaxLenVtnName+1);
  DbSubOp dbop = {kOpReadSingle, kOpMatchNone, kOpInOutNone};
  // Read the Configuration from the MainTable
  result_code = ReadConfigDB(ikey, UPLL_DT_CANDIDATE,
      UNC_OP_READ, dbop, dmi, MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      UPLL_LOG_DEBUG("No Records in main table to be created in ctrlr tbl");
      CONFIGKEYVALCLEAN(ikey);
      return UPLL_RC_SUCCESS;
    }
    delete ikey;
    return result_code;
  }
  if (ikey != NULL) {
    result_code = GetPMCtrlKeyval(ctrlckv, ikey, ctrlr_dom);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("GetPMCtrlKeyval error (%d)", result_code);
      CONFIGKEYVALCLEAN(ikey);
      return UPLL_RC_ERR_GENERIC;
    }
    if (ctrlckv == NULL) {
      UPLL_LOG_DEBUG("ctrlckv NULL");
      CONFIGKEYVALCLEAN(ikey);
      return UPLL_RC_ERR_GENERIC;
    }
    // Create the entry in VTNPMCtrl table
    result_code = UpdateConfigDB(ctrlckv, UPLL_DT_CANDIDATE, op,
        dmi, CTRLRTBL);
    if (UPLL_RC_SUCCESS != result_code) {
      CONFIGKEYVALCLEAN(ctrlckv);
      CONFIGKEYVALCLEAN(ikey);
      UPLL_LOG_DEBUG("Err while updating in ctrlr table for candidateDb(%d)",
          result_code);
      return result_code;
    }

    PolicingProfileMoMgr *pp_mgr =
      reinterpret_cast<PolicingProfileMoMgr *>
      (const_cast<MoManager *>(GetMoManager(UNC_KT_POLICING_PROFILE)));

    val_policingmap_t* val_vtn_policingmap =
      reinterpret_cast<val_policingmap_t *>(GetVal(ikey));

    UPLL_LOG_DEBUG("PP name in GetPolicingProfileCtrlrKeyval %s",
        val_vtn_policingmap->policer_name);
    UPLL_LOG_DEBUG("Ctrlrid in GetPolicingProfileCtrlrKeyval %s",
        ctrlr_dom->ctrlr);

    // Cretae the entry in PPCtrl table
    result_code = pp_mgr->PolicingProfileCtrlrTblOper(
        reinterpret_cast<const char *>(val_vtn_policingmap->policer_name),
        reinterpret_cast<const char *>(ctrlr_dom->ctrlr),
        dmi, op, UPLL_DT_CANDIDATE);
    if (UPLL_RC_SUCCESS != result_code) {
       CONFIGKEYVALCLEAN(ctrlckv);
       CONFIGKEYVALCLEAN(ikey);
       return result_code;
    }

    CONFIGKEYVALCLEAN(ikey);
    CONFIGKEYVALCLEAN(ctrlckv);
  }
  UPLL_LOG_DEBUG("Successful completion of the controller table updation");
  return result_code;
}

upll_rc_t VtnPolicingMapMoMgr::GetPMCtrlKeyval(ConfigKeyVal *&ctrlckv,
    ConfigKeyVal *&ikey,
    controller_domain *ctrlr_dom) {
  UPLL_FUNC_TRACE;
  ConfigVal *ctrlcv = NULL;

  key_vtn_t *key_vtnpmctrlr =
    reinterpret_cast<key_vtn_t *>(ConfigKeyVal::Malloc(sizeof(key_vtn_t)));
  // Get the vtn name and ctrlr id and fill it with
  // key_vtn_policingmap_controller_t structure
  uuu::upll_strncpy(
      key_vtnpmctrlr->vtn_name,
      reinterpret_cast<key_vtn_t*>
      (ikey->get_key())->vtn_name,
      (kMaxLenVtnName + 1));

  // Get the policer name and fill it with val_vtnpolicingmap_ctrl_t structure
  val_vtnpolicingmap_ctrl_t *valvtnctrlr =
    reinterpret_cast<val_vtnpolicingmap_ctrl_t *>
    (ConfigKeyVal::Malloc(sizeof(val_vtnpolicingmap_ctrl_t)));

  if (NULL != GetVal(ikey)) {
    UPLL_LOG_DEBUG("ikey with val structure");
    valvtnctrlr->valid[UPLL_IDX_POLICERNAME_PM] =
      reinterpret_cast<val_policingmap_t*>(ikey->get_cfg_val()->get_val())
      ->valid[UPLL_IDX_POLICERNAME_PM];
    if (UNC_VF_VALID == valvtnctrlr->valid[UPLL_IDX_POLICERNAME_PM])
      uuu::upll_strncpy(
          reinterpret_cast<char *>(valvtnctrlr->policer_name),
          reinterpret_cast<const char*>(reinterpret_cast<val_policingmap_t*>
            (ikey->get_cfg_val()->get_val())->policer_name),
          kMaxLenPolicingProfileName + 1);
  } else {
    UPLL_LOG_DEBUG("ikey with no val structure");
    free(valvtnctrlr);
    free(key_vtnpmctrlr);
    return UPLL_RC_ERR_GENERIC;
  }

  ctrlcv = new ConfigVal(IpctSt::kIpcInvalidStNum, valvtnctrlr);
  ctrlckv = new ConfigKeyVal(UNC_KT_VTN_POLICINGMAP,
      IpctSt::kIpcStKeyVtn,
      key_vtnpmctrlr, ctrlcv);

  if (ctrlckv == NULL) {
    UPLL_LOG_DEBUG("GetPMCtrlKeyval error ");
    free(key_vtnpmctrlr);
    if (NULL != GetVal(ikey)) free(valvtnctrlr);
    return UPLL_RC_ERR_GENERIC;
  } else {
    SET_USER_DATA_CTRLR_DOMAIN(ctrlckv, *ctrlr_dom);
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtnPolicingMapMoMgr::IsPolicingProfileConfigured(
    const char* policingprofile_name, DalDmlIntf *dmi) {
  upll_rc_t result_code = UPLL_RC_ERR_GENERIC;
  ConfigKeyVal *ckv = NULL;
  result_code = GetChildConfigKey(ckv, NULL);
  if (UPLL_RC_SUCCESS != result_code) {
    return result_code;
  }
  if (!ckv) return UPLL_RC_ERR_GENERIC;
  val_policingmap_t *pm_val = reinterpret_cast
    < val_policingmap_t *>(ConfigKeyVal::Malloc(sizeof(val_policingmap_t)));
  uuu::upll_strncpy(pm_val->policer_name, policingprofile_name,
      (kMaxLenPolicingProfileName + 1));
  pm_val->valid[UPLL_IDX_POLICERNAME_PM] = UNC_VF_VALID;
  ckv->AppendCfgVal(IpctSt::kIpcStValPolicingmap, pm_val);
  DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutNone };
  result_code = ReadConfigDB(ckv, UPLL_DT_CANDIDATE, UNC_OP_READ, dbop,
              dmi, MAINTBL);
  CONFIGKEYVALCLEAN(ckv);
  if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
    return UPLL_RC_SUCCESS;
  } else if (UPLL_RC_SUCCESS == result_code) {
    result_code = UPLL_RC_ERR_INSTANCE_EXISTS;
  }
  return result_code;
}
upll_rc_t VtnPolicingMapMoMgr::ReadMo(IpcReqRespHeader *req,
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
  result_code = ValidateReadAttribute(ikey, dmi, req);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("ReadMo ValidateReadAttribute Err  (%d)", result_code);
    return result_code;
  }

  memset(&ctrlr_dom, 0, sizeof(controller_domain));
  if (ikey->get_key_type() == UNC_KT_VTN_POLICINGMAP) {
    result_code = ReadInfoFromDB(req, ikey, dmi, &ctrlr_dom);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG(" Read request failed result(%d)", result_code);
    }
  } else if (ikey->get_key_type() == UNC_KT_VTN_POLICINGMAP_CONTROLLER) {
    result_code = ReadVtnPolicingMapController(req, ikey, dmi);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("ReadVtnPolicingMapController failed error (%d)",
                     result_code);
    }
  } else {
    UPLL_LOG_DEBUG(" Invalid Keytype recived");
    result_code = UPLL_RC_ERR_BAD_REQUEST;
  }
  return result_code;
}

upll_rc_t VtnPolicingMapMoMgr::ReadVtnPolicingMapController(
    IpcReqRespHeader *req,
    ConfigKeyVal *ikey,
    DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
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
           IpcResponse ipc_response;
          memset(&ipc_response, 0, sizeof(IpcResponse));
          result_code = SendIpcrequestToDriver(
             ikey, req, &ipc_response, dmi);
          if (UPLL_RC_SUCCESS != result_code) {
            UPLL_LOG_DEBUG("SendIpcrequestToDriver failed %d", result_code);
            return result_code;
          }
          ConfigKeyVal *temp_key = NULL;
          result_code = CopyVtnControllerCkv(ikey, temp_key);
          if (UPLL_RC_SUCCESS != result_code) {
            UPLL_LOG_DEBUG("CopyVtnControllerCkv failed %d", result_code);
            return result_code;
          }

          DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutNone };
          result_code = ReadConfigDB(temp_key, req->datatype, UNC_OP_READ, dbop,
              dmi, MAINTBL);
          if (UPLL_RC_SUCCESS != result_code) {
            UPLL_LOG_DEBUG("ReadConfigDB failed %d", result_code);
            delete temp_key;
            return result_code;
          }

          result_code = ReadControllerStateDetail(ikey, temp_key, 
                                                  &ipc_response, req->datatype,
                                                  req->operation, dmi);
          if (result_code != UPLL_RC_SUCCESS) {
            CONFIGKEYVALCLEAN(ikey);
            return result_code;
          }
        } else {
          UPLL_LOG_DEBUG(
              "ReadConfigDB error (%d) (%d)",
              req->datatype, req->option1);
          return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT;
        }
        break;
      default:
        UPLL_LOG_DEBUG(
            "ReadConfigDB error (%d) (%d)",
            req->datatype, req->option1);
        return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT;
        break;
   }
  return result_code;
}

upll_rc_t VtnPolicingMapMoMgr::ReadSiblingMo(IpcReqRespHeader *req,
                                             ConfigKeyVal *ikey,
                                             bool begin,
                                             DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  controller_domain ctrlr_dom;
  result_code = ValidateMessage(req, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG(" ValidateMessage Fail err code(%d)", result_code);
    return result_code;
  }
  result_code = ValidateReadAttribute(ikey, dmi, req);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("ReadSiblingMo ValidateReadAttribute Err  (%d)", result_code);
    return result_code;
  }

  memset(&ctrlr_dom, 0, sizeof(controller_domain));
  if (ikey->get_key_type() == UNC_KT_VTN_POLICINGMAP) {
    if (req->operation == UNC_OP_READ_SIBLING) {
      UPLL_LOG_DEBUG(" No sibling present for vtn policingmap ");
      return UPLL_RC_ERR_NO_SUCH_INSTANCE;
    }
    result_code = ReadInfoFromDB(req, ikey, dmi, &ctrlr_dom);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG(" Read request failed result(%d)", result_code);
    }
  } else if (ikey->get_key_type() == UNC_KT_VTN_POLICINGMAP_CONTROLLER) {
    result_code = ReadSiblingPolicingMapController(req, ikey, dmi);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("ReadSiblingPolicingMapController failed error (%d)",
                     result_code);
    }
  } else {
    UPLL_LOG_DEBUG(" Invalid Keytype recived");
    result_code = UPLL_RC_ERR_BAD_REQUEST;
  }
  return result_code;
}

upll_rc_t VtnPolicingMapMoMgr::ReadSiblingPolicingMapController(
    IpcReqRespHeader *req,
    ConfigKeyVal *ikey,
    DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
 upll_rc_t  result_code = UPLL_RC_SUCCESS;
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

upll_rc_t  VtnPolicingMapMoMgr::ReadSiblingControllerStateNormal(
    ConfigKeyVal *ikey,
    IpcReqRespHeader *req,
    DalDmlIntf *dmi) {

  UPLL_FUNC_TRACE;
  controller_domain ctrlr_dom;
  upll_rc_t  result_code = UPLL_RC_SUCCESS;
  // ConfigKeyVal *l_key = NULL;
  //  ConfigKeyVal *dup_key = NULL;
  ctrlr_dom.ctrlr = NULL;
  ctrlr_dom.domain = NULL;
  int ctrl_len = 0;
  int dom_len = 0;
  ConfigKeyVal *temp_vtn_pm_ckv = NULL;
  ConfigKeyVal *vtn_pm_ckv = NULL;
  key_vtn_policingmap_controller_t *vtn_pm_ctrlr_key = reinterpret_cast
      <key_vtn_policingmap_controller_t *>(ikey->get_key());
  val_policingmap_controller_t *val_pm_ctrlr = reinterpret_cast
      <val_policingmap_controller_t *>(GetVal(ikey));
  ctrlr_dom.ctrlr = reinterpret_cast <uint8_t*>
      (ConfigKeyVal::Malloc(kMaxLenCtrlrId + 1));
  ctrlr_dom.domain = reinterpret_cast <uint8_t*>
      (ConfigKeyVal::Malloc(kMaxLenDomainId + 1));
  uuu::upll_strncpy(ctrlr_dom.ctrlr,
                    vtn_pm_ctrlr_key->controller_name,
                    (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(ctrlr_dom.domain,
                    vtn_pm_ctrlr_key->domain_id,
                    (kMaxLenDomainId + 1));
  result_code = CopyVtnControllerCkv(ikey, temp_vtn_pm_ckv);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("CopyVtnControllerCkv failed");
    return result_code;
  }
  DbSubOp dbop = {kOpReadMultiple, kOpMatchNone,
    kOpInOutCtrlr | kOpInOutDomain};
  result_code = ReadConfigDB(temp_vtn_pm_ckv, req->datatype,
                             UNC_OP_READ, dbop,req->rep_count,dmi, CTRLRTBL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("ReadConfigDB failed %d", result_code);
    return result_code;
  }
  vtn_pm_ckv = temp_vtn_pm_ckv;
  ConfigKeyVal *okey = NULL;
  
  if (req->operation == UNC_OP_READ_SIBLING) {
  while (NULL != vtn_pm_ckv) {
    controller_domain temp_cd;
    GET_USER_DATA_CTRLR_DOMAIN(vtn_pm_ckv, temp_cd);
    ctrl_len = dom_len = 0;
    ctrl_len =  strcmp((const char*)(ctrlr_dom.ctrlr),
                       (const char*)(temp_cd.ctrlr));
    dom_len =  strcmp((const char*)(ctrlr_dom.domain),
                      (const char*)(temp_cd.domain));

    ConfigKeyVal *dup_ckv = NULL;
  if ((ctrl_len < 0) || ((ctrl_len == 0) && (dom_len < 0))) {
      result_code = GetChildControllerConfigKey(dup_ckv, vtn_pm_ckv,
                                              &temp_cd);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("GetChildControllerConfigKey failed");
      return result_code;
    }
    val_policingmap_controller_t *out_val_pm_ctrlr =
        reinterpret_cast<val_policingmap_controller_t *>
        (ConfigKeyVal::Malloc(sizeof(val_policingmap_controller_t)));
    memcpy(out_val_pm_ctrlr, val_pm_ctrlr,
           sizeof(val_policingmap_controller_t));
    dup_ckv->AppendCfgVal(IpctSt::kIpcStValPolicingmapController,
                          out_val_pm_ctrlr);
    UPLL_LOG_TRACE(" pppppp  %s", (dup_ckv->ToStrAll()).c_str());
    result_code = ReadControllerStateNormal(dup_ckv, req->datatype,
                                            req->operation, dmi);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("ReadDTStateNormal failed");
      return result_code;
    }
  }
    vtn_pm_ckv = vtn_pm_ckv->get_next_cfg_key_val();
    if (NULL == okey) {
      okey = dup_ckv;
    } else {
      okey->AppendCfgKeyVal(dup_ckv);
    }
  }
   if ((okey != NULL) && (result_code == UPLL_RC_SUCCESS)) {
     ikey->ResetWith(okey);
   }
 } else if (req->operation == UNC_OP_READ_SIBLING_BEGIN) {
    while (NULL != vtn_pm_ckv) {
    controller_domain temp_cd;
    GET_USER_DATA_CTRLR_DOMAIN(vtn_pm_ckv, temp_cd);
    ConfigKeyVal *dup_ckv = NULL;
    result_code = GetChildControllerConfigKey(dup_ckv, vtn_pm_ckv,
                                              &temp_cd);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("GetChildControllerConfigKey failed");
      return result_code;
    }
    val_policingmap_controller_t *out_val_pm_ctrlr =
        reinterpret_cast<val_policingmap_controller_t *>
        (ConfigKeyVal::Malloc(sizeof(val_policingmap_controller_t)));
    memcpy(out_val_pm_ctrlr, val_pm_ctrlr,
           sizeof(val_policingmap_controller_t));
    dup_ckv->AppendCfgVal(IpctSt::kIpcStValPolicingmapController,
                          out_val_pm_ctrlr);
    result_code = ReadControllerStateNormal(dup_ckv, req->datatype,
                                            req->operation, dmi);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("ReadDTStateNormal failed");
      return result_code;
  }
    vtn_pm_ckv = vtn_pm_ckv->get_next_cfg_key_val();
    if (NULL == okey) {
      okey = dup_ckv;
    } else {
      okey->AppendCfgKeyVal(dup_ckv);
    }
  }
   if ((okey != NULL) && (result_code == UPLL_RC_SUCCESS)) {
     ikey->ResetWith(okey);
   }
 } else {
    result_code = UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT;
 }
  return result_code;
}
upll_rc_t  VtnPolicingMapMoMgr::ReadSiblingControllerStateDetail(
    ConfigKeyVal *ikey,
    IpcReqRespHeader *req,
    DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  controller_domain ctrlr_dom;
  ctrlr_dom.ctrlr = NULL;
  ctrlr_dom.domain = NULL;
  int ctrl_len = 0;
  int dom_len = 0;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *temp_vtn_pm_ckv = NULL;
  ConfigKeyVal *vtn_pm_ckv = NULL;
  key_vtn_policingmap_controller_t *vtn_pm_ctrlr_key = reinterpret_cast
      <key_vtn_policingmap_controller_t *>(ikey->get_key());
  val_policingmap_controller_t *val_pm_ctrlr = reinterpret_cast
      <val_policingmap_controller_t *>(GetVal(ikey));
  ctrlr_dom.ctrlr = reinterpret_cast <uint8_t*>
      (ConfigKeyVal::Malloc(kMaxLenCtrlrId + 1));
  ctrlr_dom.domain = reinterpret_cast <uint8_t*>
      (ConfigKeyVal::Malloc(kMaxLenDomainId + 1));
  uuu::upll_strncpy(ctrlr_dom.ctrlr,
                    vtn_pm_ctrlr_key->controller_name,
                    (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(ctrlr_dom.domain,
                    vtn_pm_ctrlr_key->domain_id,
                    (kMaxLenDomainId + 1));
  result_code = CopyVtnControllerCkv(ikey, temp_vtn_pm_ckv);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("CopyVtnControllerCkv failed");
    return result_code;
  }
  UPLL_LOG_DEBUG("KT of temp_vtn_pm_ckv %d",
                 temp_vtn_pm_ckv->get_key_type());
  DbSubOp dbop = {kOpReadMultiple, kOpMatchNone,
    kOpInOutCtrlr | kOpInOutDomain};
  result_code = ReadConfigDB(temp_vtn_pm_ckv, req->datatype,
                             UNC_OP_READ, dbop, dmi, CTRLRTBL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("ReadConfigDB failed %d", result_code);
    delete temp_vtn_pm_ckv;
    return result_code;
  }
  vtn_pm_ckv = temp_vtn_pm_ckv;
  ConfigKeyVal *okey = NULL;

  if (req->operation == UNC_OP_READ_SIBLING) {
  while (NULL != vtn_pm_ckv) {
    UPLL_LOG_DEBUG("KT of vtn_pm_ckv in while %d",
                   vtn_pm_ckv->get_key_type());
    controller_domain temp_cd;
    GET_USER_DATA_CTRLR_DOMAIN(vtn_pm_ckv, temp_cd);
    ctrl_len = dom_len = 0;
    ctrl_len =  strcmp((const char*)(ctrlr_dom.ctrlr),
                       (const char*)(temp_cd.ctrlr));
    dom_len =  strcmp((const char*)(ctrlr_dom.domain),
                      (const char*)(temp_cd.domain));
    ConfigKeyVal *dup_ckv = NULL;

   if ((ctrl_len < 0) || ((ctrl_len == 0) && (dom_len < 0))) {
    result_code = GetChildControllerConfigKey(dup_ckv, vtn_pm_ckv,
                                              &temp_cd);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("GetChildControllerConfigKey failed");
      return result_code;
    }
    val_policingmap_controller_t *out_val_pm_ctrlr =
        reinterpret_cast<val_policingmap_controller_t *>
        (ConfigKeyVal::Malloc(sizeof(val_policingmap_controller_t)));
    memcpy(out_val_pm_ctrlr, val_pm_ctrlr,
           sizeof(val_policingmap_controller_t));
    dup_ckv->AppendCfgVal(IpctSt::kIpcStValPolicingmapController,
                          out_val_pm_ctrlr);
    string s1(dup_ckv->ToStrAll());
    UPLL_LOG_INFO("%s DUP_CKV PRINT", s1.c_str());
    IpcResponse ipc_response;
    memset(&ipc_response, 0, sizeof(IpcResponse));
    result_code = SendIpcrequestToDriver(
        dup_ckv, req, &ipc_response, dmi);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("SendIpcrequestToDriver failed %d", result_code);
      return result_code;
    }
    ConfigKeyVal *temp_key = NULL;
    result_code = CopyVtnControllerCkv(dup_ckv, temp_key);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("CopyVtnControllerCkv failed %d", result_code);
      return result_code;
    }
    result_code = ReadConfigDB(temp_key, req->datatype, UNC_OP_READ,
                               dbop, dmi, MAINTBL);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("ReadConfigDB failed %d", result_code);
      return result_code;
    }
    result_code = ReadControllerStateDetail(dup_ckv, temp_key, &ipc_response,
                                            req->datatype, req->operation, dmi);
    if (result_code != UPLL_RC_SUCCESS) {
      CONFIGKEYVALCLEAN(ikey);
      return result_code;
    }
    if (NULL == okey) {
      okey = dup_ckv;
    } else {
      okey->AppendCfgKeyVal(dup_ckv);
    }
   } else {
        result_code = UPLL_RC_ERR_NO_SUCH_INSTANCE;
   }
    vtn_pm_ckv = vtn_pm_ckv->get_next_cfg_key_val();
  }
  if ((okey != NULL) && (result_code == UPLL_RC_SUCCESS)) {
    ikey->ResetWith(okey);
  }
 } else  if (req->operation == UNC_OP_READ_SIBLING_BEGIN) {
  while (NULL != vtn_pm_ckv) {
    UPLL_LOG_DEBUG("KT of vtn_pm_ckv in while %d",
                   vtn_pm_ckv->get_key_type());
    controller_domain temp_cd;
    ConfigKeyVal *dup_ckv = NULL;
    GET_USER_DATA_CTRLR_DOMAIN(vtn_pm_ckv, temp_cd);
    result_code = GetChildControllerConfigKey(dup_ckv, vtn_pm_ckv,
                                              &temp_cd);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("GetChildControllerConfigKey failed");
      return result_code;
    }
    val_policingmap_controller_t *out_val_pm_ctrlr =
        reinterpret_cast<val_policingmap_controller_t *>
        (ConfigKeyVal::Malloc(sizeof(val_policingmap_controller_t)));
    memcpy(out_val_pm_ctrlr, val_pm_ctrlr,
           sizeof(val_policingmap_controller_t));
    dup_ckv->AppendCfgVal(IpctSt::kIpcStValPolicingmapController,
                          out_val_pm_ctrlr);
    string s1(dup_ckv->ToStrAll());
    UPLL_LOG_INFO("%s DUP_CKV PRINT", s1.c_str());
    IpcResponse ipc_response;
    memset(&ipc_response, 0, sizeof(IpcResponse));
    result_code = SendIpcrequestToDriver(
        dup_ckv, req, &ipc_response, dmi);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("SendIpcrequestToDriver failed %d", result_code);
      return result_code;
    }
    ConfigKeyVal *temp_key = NULL;
    result_code = CopyVtnControllerCkv(dup_ckv, temp_key);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("CopyVtnControllerCkv failed %d", result_code);
      return result_code;
    }
    result_code = ReadConfigDB(temp_key, req->datatype, UNC_OP_READ,
                               dbop, dmi, MAINTBL);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("ReadConfigDB failed %d", result_code);
      return result_code;
    }
    result_code = ReadControllerStateDetail(dup_ckv, temp_key, &ipc_response,
                                            req->datatype, req->operation, dmi);
    if (result_code != UPLL_RC_SUCCESS) {
      CONFIGKEYVALCLEAN(ikey);
      return result_code;
    }
    if (NULL == okey) {
      okey = dup_ckv;
    } else {
      okey->AppendCfgKeyVal(dup_ckv);
    }
    vtn_pm_ckv = vtn_pm_ckv->get_next_cfg_key_val();
  }
  if ((okey != NULL) && (result_code == UPLL_RC_SUCCESS)) {
    ikey->ResetWith(okey);
  }
}
else {
    result_code = UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT;
  }
  return result_code;
}

bool VtnPolicingMapMoMgr::GetRenameKeyBindInfo(unc_key_type_t key_type,
    BindInfo *&binfo, int &nattr,
    MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  UPLL_LOG_DEBUG("GetRenameKeyBindInfo (%d) (%d)", key_type, tbl);
  switch (key_type) {
    case UNC_KT_VTN_POLICINGMAP:
    case UNC_KT_VTN_POLICINGMAP_CONTROLLER:
      nattr = VTN_KEY_COL;
      if (MAINTBL == tbl) {
        binfo = key_vtnpm_vtn_maintbl_rename_bind_info;
      } else if (CTRLRTBL == tbl) {
        binfo = key_vtnpm_vtn_ctrlrtbl_rename_bind_info;
      } else {
        UPLL_LOG_DEBUG("GetRenameKeyBindInfo Invalid Tbl (%d)", key_type);
        return PFC_FALSE;
      }
      break;
    case UNC_KT_POLICING_PROFILE:
      nattr = POLICY_KEY_COL;
      if (MAINTBL == tbl) {
        binfo = key_vtnpm_Policyname_maintbl_rename_bind_info;
      } else if (CTRLRTBL == tbl) {
        binfo = key_vtnpm_Policyname_ctrlrtbl_rename_bind_info;
      } else {
        UPLL_LOG_DEBUG("GetRenameKeyBindInfo Invalid Tbl (%d)", key_type);
        return PFC_FALSE;
      }
      break;
    default:
      UPLL_LOG_DEBUG("GetRenameKeyBindInfo Invalid key type (%d) (%d)",
          key_type, tbl);
      return PFC_FALSE;
  }
  return PFC_TRUE;
}

upll_rc_t VtnPolicingMapMoMgr::CopyToConfigKey(ConfigKeyVal *&okey,
    ConfigKeyVal *ikey) {
  UPLL_FUNC_TRACE;
  if (!ikey || !(ikey->get_key())) {
    UPLL_LOG_DEBUG("CopyToConfigKey ikey NULL");
    return UPLL_RC_ERR_GENERIC;
  }

  upll_rc_t result_code = UPLL_RC_SUCCESS;

  if (UNC_KT_VTN_POLICINGMAP == ikey->get_key_type()) {
    key_rename_vnode_info_t *key_rename =
      reinterpret_cast<key_rename_vnode_info_t *>(ikey->get_key());
    key_vtn_t *key_vtn = reinterpret_cast<key_vtn_t *>
      (ConfigKeyVal::Malloc(sizeof(key_vtn_t)));

    if (!strlen(reinterpret_cast<char *>(key_rename->old_unc_vtn_name))) {
      UPLL_LOG_DEBUG("CopyToConfigKey key_rename->old_unc_vtn_name NULL");
      free(key_vtn);
      return UPLL_RC_ERR_GENERIC;
    }

    uuu::upll_strncpy(key_vtn->vtn_name,
        key_rename->old_unc_vtn_name,
        (kMaxLenVtnName + 1));

    okey = new ConfigKeyVal(UNC_KT_VTN_POLICINGMAP,
                            IpctSt::kIpcStKeyVtn, key_vtn, NULL);
    if (!okey) {
      UPLL_LOG_DEBUG("CopyToConfigKey okey NULL");
      free(key_vtn);
      return UPLL_RC_ERR_GENERIC;
    }
  } else if (UNC_KT_POLICING_PROFILE == ikey->get_key_type()) {
    key_rename_vnode_info_t *key_rename =
      reinterpret_cast<key_rename_vnode_info_t *>(ikey->get_key());

    val_policingmap_t *val = reinterpret_cast<val_policingmap_t *>
        (ConfigKeyVal::Malloc(sizeof(val_policingmap_t)));
    if (!strlen(reinterpret_cast<char *>
                (key_rename->old_policingprofile_name))) {
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
      UPLL_LOG_DEBUG("CopyToConfigKey okey  NULL");
      free(val);
      return UPLL_RC_ERR_GENERIC;
    }
  } else {
    UPLL_LOG_DEBUG("CopyToConfigKey invalid key type NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  return result_code;
}

upll_rc_t VtnPolicingMapMoMgr::MergeValidate(unc_key_type_t keytype,
    const char *ctrlr_id,
    ConfigKeyVal *ikey,
    DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  DbSubOp dbop = { kOpReadMultiple, kOpMatchNone, kOpInOutNone };
  ConfigKeyVal *tkey;
  ConfigKeyVal *keyval = NULL;

  if (NULL == ikey) {
    UPLL_LOG_DEBUG("MergeValidate ikey NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  // Read the record in IMPORT vtnpolicingmap table
  result_code = ReadConfigDB(ikey, UPLL_DT_IMPORT, UNC_OP_READ, dbop, dmi,
      MAINTBL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("MergeValidate ReadConfigDB error (%d)", result_code);
    return result_code;
  }
  MoMgrImpl *mgr = static_cast<MoMgrImpl *>((const_cast<MoManager *>
        (GetMoManager(UNC_KT_POLICING_PROFILE))));
  tkey = ikey;
  while (ikey != NULL) {
    result_code = mgr->GetChildConfigKey(keyval, NULL);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("MergeValidate GetChildConfigKey error (%d)", result_code);
      return result_code;
    }
    if (!keyval) return UPLL_RC_ERR_GENERIC;
    val_policingmap_t* val_vtn_policingmap =
      reinterpret_cast<val_policingmap_t *>(GetVal(ikey));
    key_policingprofile_t *key_policingprofile =
      reinterpret_cast<key_policingprofile_t*>(keyval->get_key());
    uuu::upll_strncpy(key_policingprofile->policingprofile_name,
        val_vtn_policingmap->policer_name,
        (kMaxLenPolicingProfileName + 1));
    // Check Import DB's policingmap record with Candidate DB's
    // policingprofile table
    result_code = mgr->UpdateConfigDB(keyval, UPLL_DT_CANDIDATE, UNC_OP_READ,
        dmi, MAINTBL);
    if (result_code != UPLL_RC_ERR_INSTANCE_EXISTS) {
      UPLL_LOG_DEBUG("MergeValidate UpdateConfigDB UPLL_RC_ERR_MERGE_CONFLICT");
      CONFIGKEYVALCLEAN(keyval);
      return UPLL_RC_ERR_MERGE_CONFLICT;
    }
    ikey = tkey->get_next_cfg_key_val();
    CONFIGKEYVALCLEAN(keyval);
  }
  CONFIGKEYVALCLEAN(tkey);
  return result_code;
}

upll_rc_t VtnPolicingMapMoMgr::GetRenamedUncKey(ConfigKeyVal *ctrlr_key,
    upll_keytype_datatype_t dt_type,
    DalDmlIntf *dmi,
    uint8_t *ctrlr_id) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code;
  ConfigKeyVal *unc_key = NULL;

  if (NULL == ctrlr_key || NULL == dmi || ctrlr_id[0] == '\0') {
    UPLL_LOG_DEBUG("GetRenamedUncKey failed. Insufficient input parameters.");
    return UPLL_RC_ERR_GENERIC;
  }

  key_vtn *ctrlr_vtn_key = reinterpret_cast<key_vtn *>(ctrlr_key->get_key());

  DbSubOp dbop = { kOpReadSingle, kOpMatchCtrlr, kOpInOutNone };

  val_rename_vtn *rename_vtn = reinterpret_cast<val_rename_vtn *>
      (ConfigKeyVal::Malloc(sizeof(val_rename_vtn)));
  uuu::upll_strncpy(rename_vtn->new_name, ctrlr_vtn_key->vtn_name,
      (kMaxLenVtnName + 1));

  rename_vtn->valid[UPLL_IDX_NEW_NAME_RVTN] = UNC_VF_VALID;

  MoMgrImpl *mgr =
    reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>(GetMoManager(
            UNC_KT_VTN)));
  if (NULL == mgr) {
    UPLL_LOG_DEBUG("mgr NULL");
    free(rename_vtn);
    return UPLL_RC_ERR_GENERIC;
  }

  result_code = mgr->GetChildConfigKey(unc_key, NULL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetRenamedUncKey GetChildConfigKey (%d)", result_code);
    free(rename_vtn);
    return result_code;
  }
  if (!unc_key) {
    free(rename_vtn);
    return UPLL_RC_ERR_GENERIC;
  }

  ConfigVal *p_val = new ConfigVal(
      IpctSt::kIpcStValRenameVtn, rename_vtn);

  unc_key->SetCfgVal(p_val);

  SET_USER_DATA_CTRLR(unc_key, ctrlr_id);

  result_code = mgr->ReadConfigDB(unc_key, dt_type, UNC_OP_READ, dbop, dmi,
      RENAMETBL);
  if (UPLL_RC_SUCCESS != result_code
      && UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
    pfc_log_debug("GetRenamedUncKey failed. ReadConfigDB failed to read %d ",
        result_code);
    CONFIGKEYVALCLEAN(unc_key);
    return UPLL_RC_ERR_GENERIC;
  }

  if (result_code == UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetRenamedUncKey ReadConfigDB success");
    key_vtn *vtn_key = reinterpret_cast<key_vtn *>(unc_key->get_key());
    if (vtn_key) {
      if (strcmp(reinterpret_cast<const char *>(ctrlr_vtn_key->vtn_name),
            reinterpret_cast<const char *>(vtn_key->vtn_name))) {
        uuu::upll_strncpy(ctrlr_vtn_key->vtn_name,
            vtn_key->vtn_name,
            (kMaxLenVtnName + 1));
      }
    }
  }

  mgr = NULL;
  CONFIGKEYVALCLEAN(unc_key);

  val_rename_policingprofile *rename_policingprofile =
    reinterpret_cast<val_rename_policingprofile *>
    (ConfigKeyVal::Malloc(sizeof(val_rename_policingprofile)));

  val_policingmap_t *val_policingmap =
    reinterpret_cast<val_policingmap_t *>(GetVal(ctrlr_key));
  if (!val_policingmap) {
    free(rename_policingprofile);
    return UPLL_RC_ERR_GENERIC;
  }

  memset(rename_policingprofile->policingprofile_newname, '\0',
      sizeof(rename_policingprofile->policingprofile_newname));
  uuu::upll_strncpy(
      rename_policingprofile->policingprofile_newname,
      val_policingmap->policer_name,
      (kMaxLenPolicingProfileName + 1));

  MoMgrImpl *pp_mgr =
    reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>(GetMoManager(
            UNC_KT_POLICING_PROFILE)));
  if (NULL == pp_mgr) {
    UPLL_LOG_DEBUG("mgr NULL");
    free(rename_policingprofile);
    return UPLL_RC_ERR_GENERIC;
  }

  result_code = pp_mgr->GetChildConfigKey(unc_key, NULL);
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

  ConfigVal *p_config_val = new ConfigVal(
      IpctSt::kIpcStValRenamePolicingprofile, rename_policingprofile);

  unc_key->SetCfgVal(p_config_val);
  SET_USER_DATA_CTRLR(unc_key, ctrlr_id);
  result_code = pp_mgr->ReadConfigDB(unc_key, dt_type, UNC_OP_READ, dbop, dmi,
      RENAMETBL);
  if (UPLL_RC_SUCCESS != result_code
      && UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
    UPLL_LOG_DEBUG(" GetRenamedUncKey failed. ReadConfigDB failed to read %d ",
        result_code);
    CONFIGKEYVALCLEAN(unc_key);
    return UPLL_RC_ERR_GENERIC;
  }

  if (result_code == UPLL_RC_SUCCESS) {  // COV DEAD CODE
    key_policingprofile_t *key_policingprofile =
      reinterpret_cast<key_policingprofile_t *>(unc_key->get_key());
    uuu::upll_strncpy(
        val_policingmap->policer_name,
        key_policingprofile->policingprofile_name,
        (kMaxLenPolicingProfileName + 1));
  }
  CONFIGKEYVALCLEAN(unc_key);
  return result_code;
}

upll_rc_t VtnPolicingMapMoMgr::TxUpdateController(
    unc_key_type_t keytype, uint32_t session_id, uint32_t config_id,
    uuc::UpdateCtrlrPhase phase,
    set<string> *affected_ctrlr_set,
    DalDmlIntf *dmi,
    ConfigKeyVal **err_ckv) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  DalResultCode dal_result = uud::kDalRcSuccess;
  ConfigKeyVal *req = NULL, *nreq = NULL, *ck_main = NULL;
  controller_domain ctrlr_dom;  // UNINIT
  ctrlr_dom.ctrlr = NULL;
  ctrlr_dom.domain = NULL;
  DalCursor *dal_cursor_handle = NULL;
  IpcResponse resp;
  memset(&resp, 0, sizeof(IpcResponse));

  unc_keytype_operation_t op = (phase == uuc::kUpllUcpCreate)?UNC_OP_CREATE:
      ((phase == uuc::kUpllUcpUpdate)?UNC_OP_UPDATE:
       ((phase == uuc::kUpllUcpDelete)?UNC_OP_DELETE:UNC_OP_INVALID));
  switch (op) {
    case UNC_OP_CREATE:
    case UNC_OP_DELETE:
      result_code = DiffConfigDB(UPLL_DT_CANDIDATE, UPLL_DT_RUNNING, op,
                                 req, nreq, &dal_cursor_handle, dmi, CTRLRTBL);
      break;
    case UNC_OP_UPDATE:
      result_code = DiffConfigDB(UPLL_DT_CANDIDATE, UPLL_DT_RUNNING, op,
                                 req, nreq, &dal_cursor_handle, dmi, MAINTBL);
      break;
    default:
      UPLL_LOG_DEBUG("TxUpdateController Invalid operation \n");
      return UPLL_RC_ERR_GENERIC;
  }
  resp.header.clnt_sess_id = session_id;
  resp.header.config_id = config_id;
  while (result_code == UPLL_RC_SUCCESS) {
    /* Get Next Record */
    dal_result = dmi->GetNextRecord(dal_cursor_handle);
    result_code = DalToUpllResCode(dal_result);
    if (result_code != UPLL_RC_SUCCESS)
      break;
    ck_main = NULL;
    if ( (op == UNC_OP_CREATE) || (op == UNC_OP_DELETE) ) {
      result_code = DupConfigKeyVal(ck_main, req, MAINTBL);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("DupConfigKeyVal err (%d)", result_code);
        return result_code;
      }
      GET_USER_DATA_CTRLR_DOMAIN(ck_main, ctrlr_dom);
      if (ctrlr_dom.ctrlr == NULL) {
        UPLL_LOG_DEBUG("Invalid controller");
        if (ck_main) delete ck_main;
        return UPLL_RC_ERR_GENERIC;
      }
      UPLL_LOG_DEBUG("ctrlr : %s; domain : %s", ctrlr_dom.ctrlr,
                     ctrlr_dom.domain);
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
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("DupConfigKeyVal failed");
        return result_code;
      }
      /*
         result_code = ValidateCapability(&(ipc_req.header), ck_main);
         if (result_code != UPLL_RC_SUCCESS) {
         if (ck_main != NULL) {
         delete ck_main;
         ck_main = NULL;
         }
         return result_code;
         }
         */
      result_code = GetChildConfigKey(ck_ctrlr, ck_main);
      if (result_code != UPLL_RC_SUCCESS)
        return result_code;
      if (GetControllerDomainSpan(ck_ctrlr, UPLL_DT_CANDIDATE, dmi) ==
          UPLL_RC_ERR_NO_SUCH_INSTANCE) {
        CONFIGKEYVALCLEAN(ck_ctrlr);
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
          return UPLL_RC_ERR_GENERIC;
        }

        result_code = TxUpdateProcess(ck_main, &resp, op, dmi, &ctrlr_dom);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("TxUpdate Process returns with %d\n", result_code);
          *err_ckv = resp.ckv_data;
          CONFIGKEYVALCLEAN(ck_main);
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
  if (dal_cursor_handle) {
    dmi->CloseCursor(dal_cursor_handle, true);
    dal_cursor_handle = NULL;
  }
  result_code = (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE)?
      UPLL_RC_SUCCESS:result_code;
  return result_code;
}

upll_rc_t VtnPolicingMapMoMgr::TxUpdateProcess(ConfigKeyVal *ck_main,
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
    DbSubOp dbop = {kOpReadSingle, kOpMatchNone, kOpInOutCs};
    result_code = ReadConfigDB(dup_ckmain, UPLL_DT_CANDIDATE,
                               UNC_OP_READ, dbop, dmi, MAINTBL);
    if (result_code != UPLL_RC_SUCCESS) {
      string s(dup_ckmain->ToStrAll());
      UPLL_LOG_INFO("%s Policingprofile read failed %d",
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

upll_rc_t VtnPolicingMapMoMgr::GetControllerDomainSpan(
    ConfigKeyVal *ikey, upll_keytype_datatype_t dt_type, DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code;
  DbSubOp dbop = { kOpReadExist, kOpMatchNone,
          kOpInOutCtrlr|kOpInOutDomain};

  result_code = ReadConfigDB(ikey, dt_type, UNC_OP_READ, dbop, dmi, CTRLRTBL);
  UPLL_LOG_DEBUG("GetControllerDomainSpan successful:- %d", result_code);
  return result_code;
}


upll_rc_t VtnPolicingMapMoMgr::UpdateMainTbl(ConfigKeyVal *vtn_key,
      unc_keytype_operation_t op, uint32_t driver_result,
      ConfigKeyVal *nreq, DalDmlIntf *dmi) {
  ConfigKeyVal *ck_vtn = NULL;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  val_policingmap_t *pm_val = NULL;
  void *pmval = NULL;
  void *npmval = NULL;

  UPLL_FUNC_TRACE;
  if (op != UNC_OP_DELETE) {
    result_code = DupConfigKeyVal(ck_vtn, vtn_key, MAINTBL);
    if (!ck_vtn || result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Returning error %d\n", result_code);
      return UPLL_RC_ERR_GENERIC;
    }
    pm_val = reinterpret_cast<val_policingmap_t *>(GetVal(ck_vtn));
    if (!pm_val) {
      UPLL_LOG_DEBUG("invalid val \n");
      return UPLL_RC_ERR_GENERIC;
    }
  } else {
    result_code = GetChildConfigKey(ck_vtn, vtn_key);
    if (!ck_vtn || result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Returning error %d\n", result_code);
      return UPLL_RC_ERR_GENERIC;
    }
  }
  switch (op) {
    case UNC_OP_CREATE:
      pm_val->cs_row_status = UNC_CS_NOT_APPLIED;
      break;
    case UNC_OP_UPDATE:
      pmval = reinterpret_cast<void *>(&pm_val);
      npmval = (nreq)?GetVal(nreq):NULL;
      if (!npmval) {
        UPLL_LOG_DEBUG("Invalid param\n");
        return UPLL_RC_ERR_GENERIC;
      }
      CompareValidValue(pmval, npmval, false);
      break;
    case UNC_OP_DELETE:
      break;
    default:
      UPLL_LOG_DEBUG("Inalid operation\n");
      return UPLL_RC_ERR_GENERIC;
  }
  result_code = UpdateConfigDB(ck_vtn, UPLL_DT_STATE, op, dmi, MAINTBL);
  EnqueCfgNotification(op, UPLL_DT_RUNNING, ck_vtn);
  delete ck_vtn;
  return result_code;
}

upll_rc_t VtnPolicingMapMoMgr::GetRenamedControllerKey(
    ConfigKeyVal *ikey, upll_keytype_datatype_t dt_type, DalDmlIntf *dmi,
    controller_domain *ctrlr_dom) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *okey = NULL;
  uint8_t rename = 0;  // rename_pp = 0;

  if (NULL == ikey || NULL == dmi) {
    UPLL_LOG_DEBUG("Insufficient input resources");
    return UPLL_RC_ERR_GENERIC;
  }

  result_code = IsRenamed(ikey, dt_type, dmi, rename);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetRenamedControllerKey failed. IsRenamed failed to "
        "check rename - %d",
        result_code);
    return result_code;
  }

  if (rename == 0) {
    UPLL_LOG_DEBUG("Name has not renamed");
    return UPLL_RC_SUCCESS;
  }

  if (rename & VTN_RENAME) {
    DbSubOp dbop = { kOpReadSingle, kOpMatchCtrlr|kOpMatchDomain,
      kOpInOutFlag };

    MoMgrImpl *vtn_mgr = reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>
        (GetMoManager(UNC_KT_VTN)));
    if (NULL == vtn_mgr) {
      UPLL_LOG_DEBUG("mgr NULL");
      return UPLL_RC_ERR_GENERIC;
    }

    result_code = vtn_mgr->GetChildConfigKey(okey, NULL);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG(" GetRenamedControllerKey failed. "
          "GetChildConfigKey failed to create vbr ConfigKey - %d",
          result_code);
      return result_code;
    }
    if (!okey) return UPLL_RC_ERR_GENERIC;
    key_vtn_t *ovtn_key = reinterpret_cast<key_vtn_t*>(okey->get_key());
    key_vtn_t *ivtn_key = reinterpret_cast<key_vtn_t*>(ikey->get_key());

    uuu::upll_strncpy(reinterpret_cast<char*>(ovtn_key->vtn_name),
        reinterpret_cast<char*>(ivtn_key->vtn_name),
        kMaxLenVtnName+1);

    if (!ctrlr_dom)
      SET_USER_DATA_CTRLR_DOMAIN(okey, *ctrlr_dom);

    result_code = vtn_mgr->ReadConfigDB(okey, dt_type, UNC_OP_READ, dbop, dmi,
        RENAMETBL); /* ctrlr_name */
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("GetRenamedControllerKey ReadConfigDB error");
      CONFIGKEYVALCLEAN(okey);
      return result_code;
    }

    val_rename_vtn *rename_val =
      reinterpret_cast<val_rename_vtn *>(GetVal(okey));
    if (!rename_val
        || (rename_val->valid[UPLL_IDX_NEW_NAME_RVTN] != UNC_VF_VALID)) {
      CONFIGKEYVALCLEAN(okey);
      UPLL_LOG_DEBUG("rename_val NULL");
      return UPLL_RC_ERR_GENERIC;
    }

    uuu::upll_strncpy(reinterpret_cast<char *>(ivtn_key->vtn_name),
        reinterpret_cast<const char *>(rename_val->new_name),
        kMaxLenVtnName + 1);

    SET_USER_DATA_FLAGS(ikey, rename);
    CONFIGKEYVALCLEAN(okey);
    vtn_mgr = NULL;
  }

  CONFIGKEYVALCLEAN(okey);

  if (rename & POLICINGPROFILE_RENAME) {
    MoMgrImpl *pp_mgr = reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>
        (GetMoManager(UNC_KT_POLICING_PROFILE)));
    if (NULL == pp_mgr) {
      UPLL_LOG_DEBUG("mgr NULL");
      return UPLL_RC_ERR_GENERIC;
    }

    result_code = pp_mgr->GetChildConfigKey(okey, NULL);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG(" GetRenamedControllerKey failed. GetChildConfigKey failed"
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

    if (!ctrlr_dom) SET_USER_DATA_CTRLR_DOMAIN(okey, *ctrlr_dom);

    DbSubOp dbop1 = { kOpReadSingle, kOpMatchCtrlr, kOpInOutFlag };
    result_code = pp_mgr->ReadConfigDB(okey, dt_type, UNC_OP_READ, dbop1, dmi,
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
      UPLL_LOG_DEBUG("rename_policingprofile NULL");
      return UPLL_RC_ERR_GENERIC;
    }

    uuu::upll_strncpy(
        reinterpret_cast<char *>(val_policingmap->policer_name),
        reinterpret_cast<const char *>
        (rename_policingprofile->policingprofile_newname),
        (kMaxLenPolicingProfileName + 1));

    SET_USER_DATA_FLAGS(ikey, rename);
    pp_mgr = NULL;
  }

  CONFIGKEYVALCLEAN(okey);
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtnPolicingMapMoMgr::TxCopyCandidateToRunning(
    unc_key_type_t keytype, CtrlrCommitStatusList *ctrlr_commit_status,
    DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  DalResultCode db_result;
  unc_keytype_operation_t op[] = { UNC_OP_DELETE, UNC_OP_CREATE,
    UNC_OP_UPDATE };
  int nop = sizeof(op) / sizeof(op[0]);
  ConfigKeyVal *vtn_key = NULL, *req = NULL, *nreq = NULL;
  DalCursor *cfg1_cursor = NULL;
  uint8_t *ctrlr_id = NULL;
  /*IpcReqRespHeader *req_header = reinterpret_cast<IpcReqRespHeader *>
      (ConfigKeyVal::Malloc(sizeof(IpcReqRespHeader)));
   */
  map<string, int> ctrlr_result;
  CtrlrCommitStatusList::iterator ccsListItr;
  CtrlrCommitStatus *ccStatusPtr;

  if ((ctrlr_commit_status == NULL) || (dmi == NULL)) {
    return UPLL_RC_ERR_GENERIC;
  }

  for (ccsListItr = ctrlr_commit_status->begin();
      ccsListItr != ctrlr_commit_status->end(); ++ccsListItr) {
    ccStatusPtr = *ccsListItr;
    ctrlr_id = reinterpret_cast<uint8_t *>(&ccStatusPtr->ctrlr_id);
    ctrlr_result[ccStatusPtr->ctrlr_id] = ccStatusPtr->upll_ctrlr_result;
    if (ccStatusPtr->upll_ctrlr_result != UPLL_RC_SUCCESS) {
      for (ConfigKeyVal *ck_err = ccStatusPtr->err_ckv; ck_err != NULL;
          ck_err = ck_err->get_next_cfg_key_val()) {
        if (ck_err->get_key_type() != keytype) continue;
        result_code = GetRenamedUncKey(ck_err, UPLL_DT_CANDIDATE, dmi,
            ctrlr_id);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("GetRenamedControllerKey error(%d)", result_code);
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
          break;
        }
        result_code = UpdateMainTbl(req, op[i], UPLL_RC_SUCCESS,
                                    nreq, dmi);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("Updating Main table Error %d", result_code);
          return result_code;
        }
      }
      if (cfg1_cursor) {
        dmi->CloseCursor(cfg1_cursor, true);
        cfg1_cursor = NULL;
      }
      if (req) {
        delete req;
        req = NULL;
      }
    }
    UPLL_LOG_DEBUG("Updating main table complete with op %d", op[i]);
  }

  for (int i = 0; i < nop; i++) {
    MoMgrTables tbl = (op[i] == UNC_OP_UPDATE)?MAINTBL:CTRLRTBL;
    result_code = DiffConfigDB(UPLL_DT_CANDIDATE, UPLL_DT_RUNNING, op[i], req,
                               nreq, &cfg1_cursor, dmi, tbl);

    ConfigKeyVal *vtn_ctrlr_key = NULL;
    while (result_code == UPLL_RC_SUCCESS) {
      db_result = dmi->GetNextRecord(cfg1_cursor);
      result_code = DalToUpllResCode(db_result);
#if 0
      if (result_code != UPLL_RC_SUCCESS)
        break;
#endif
      if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
        result_code = UPLL_RC_SUCCESS;
        break;
      }
      if (op[i] == UNC_OP_UPDATE) {
        DbSubOp dbop = { kOpReadMultiple, kOpMatchNone, kOpInOutCtrlr };
        result_code = GetChildConfigKey(vtn_ctrlr_key, req);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("GetChildConfigKey is failed resultcode=%d",
                         result_code);
          return result_code;
        }
        /* Capability check
         * req_header->operation = op[i];
         * strcpy((char*)req_header->datatype, (char*)UNC_DT_CANDIDATE);
         * result_code = ValidateCapability(req_header, vtn_ctrlr_key);
         *                                         */
        result_code = ReadConfigDB(vtn_ctrlr_key, UPLL_DT_CANDIDATE,
                                   UNC_OP_READ, dbop, dmi, CTRLRTBL);
        if (result_code != UPLL_RC_SUCCESS) {
          delete vtn_ctrlr_key;
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

        result_code = DupConfigKeyVal(vtn_key, req, tbl);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("DupConfigKeyVal is failed result_code = %d",
                         result_code);
          CONFIGKEYVALCLEAN(vtn_ctrlr_key);
          return result_code;
        }
        GET_USER_DATA_CTRLR(vtn_ctrlr_key, ctrlr_id);
        string controller(reinterpret_cast<char *>(ctrlr_id));
        for (ConfigKeyVal *tmp = vtn_ctrlr_key; tmp != NULL; tmp =
             tmp->get_next_cfg_key_val()) {
          result_code = UpdateConfigStatus(vtn_key, op[i],
                                           ctrlr_result[controller], nreq,
                                           dmi, tmp);
          if (result_code != UPLL_RC_SUCCESS) break;

          result_code = UpdateConfigDB(vtn_ctrlr_key,
                                       UPLL_DT_RUNNING, op[i], dmi, CTRLRTBL);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("UpdateConfigDB for ctrlr tbl is failed ");
            CONFIGKEYVALCLEAN(vtn_ctrlr_key);
            CONFIGKEYVALCLEAN(vtn_key);
            return result_code;
          }
          result_code = UpdateConfigDB(vtn_key, UPLL_DT_RUNNING,
                                       op[i], dmi, MAINTBL);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("UpdateConfigDB for main tbl is failed");
            CONFIGKEYVALCLEAN(vtn_ctrlr_key);
            CONFIGKEYVALCLEAN(vtn_key);
            return result_code;
          }  // COV UNREACHABLE
#if 0
          EnqueCfgNotification(op[i], UPLL_DT_RUNNING,
                               vtn_ctrlr_key);
#endif
          EnqueCfgNotification(op[i], UPLL_DT_RUNNING,
                               vtn_key);
        }
      } else {
        if (op[i] == UNC_OP_CREATE) {
          DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutFlag };
          result_code = GetChildConfigKey(vtn_key, req);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("GetChildConfigKey is failed -%d", result_code);
            return result_code;
          }
          result_code = ReadConfigDB(vtn_key, UPLL_DT_CANDIDATE,
                                     UNC_OP_READ, dbop, dmi, MAINTBL);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("ReadConfigDB is failed -%d", result_code);
            CONFIGKEYVALCLEAN(vtn_key);
            return result_code;
          }
          /* Capability check
           * req_header->operation = op[i];
           * strcpy((char*)req_header->datatype, (char*)UNC_DT_CANDIDATE);
           * result_code = ValidateCapability(req_header, vtn_ctrlr_key);
           *                                                 */
          result_code = DupConfigKeyVal(vtn_ctrlr_key, req, tbl);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("DupConfigKeyVal is failed -%d", result_code);
            CONFIGKEYVALCLEAN(vtn_key);
            return result_code;
          }

          GET_USER_DATA_CTRLR(vtn_ctrlr_key, ctrlr_id);
          string controller(reinterpret_cast<char *>(ctrlr_id));
          result_code = UpdateConfigStatus(vtn_key, op[i],
                                           ctrlr_result[controller], NULL,
                                           dmi, vtn_ctrlr_key);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("Error in updating configstatus, resultcode=%d",
                           result_code);
            CONFIGKEYVALCLEAN(vtn_ctrlr_key);
            CONFIGKEYVALCLEAN(vtn_key);
            return result_code;
          }
        } else if (op[i] == UNC_OP_DELETE) {
          result_code = GetChildConfigKey(vtn_ctrlr_key, req);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("Error in getting the configkey, resultcode=%d",
                           result_code);
            return result_code;
          }
        }
        result_code = UpdateConfigDB(vtn_ctrlr_key, UPLL_DT_RUNNING,
                                     op[i], dmi, CTRLRTBL);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("UpdateConfigDB in ctrlr tbl is failed -%d",
                         result_code);
          CONFIGKEYVALCLEAN(vtn_ctrlr_key);
          return result_code;
        }
        if (op[i] != UNC_OP_DELETE) {
          result_code = UpdateConfigDB(vtn_key, UPLL_DT_RUNNING,
                                       UNC_OP_UPDATE, dmi, MAINTBL);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("UpdateConfigDB in main tbl is failed -%d",
                           result_code);
            return result_code;
          }
        }
        EnqueCfgNotification(op[i], UPLL_DT_RUNNING,
                             vtn_key);
      }
      if (vtn_key) delete vtn_key;
      vtn_key = vtn_ctrlr_key = NULL;
      result_code = DalToUpllResCode(db_result);
    }
    if (cfg1_cursor) {
      dmi->CloseCursor(cfg1_cursor, true);
      cfg1_cursor = NULL;
    }
    if (nreq) delete nreq;
    if (req) delete req;
    nreq = req = NULL;
  }
  result_code = (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) ?
      UPLL_RC_SUCCESS : result_code;
  UPLL_LOG_DEBUG("TxcopyCandidatetoRunning is successful -%d", result_code);
  return result_code;
}

upll_rc_t VtnPolicingMapMoMgr::UpdateConfigStatus(ConfigKeyVal *ckv,
    unc_keytype_operation_t op, uint32_t driver_result, ConfigKeyVal *nreq,
    DalDmlIntf *dmi, ConfigKeyVal *ctrlr_key) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
#if 0
  unc_keytype_configstatus_t status = UNC_CS_UNKNOWN,
                             cs_status = UNC_CS_UNKNOWN;
  cs_status = (driver_result == 0)?UNC_CS_APPLIED: UNC_CS_NOT_APPLIED;
  val_policingmap_t *pm_val =
    reinterpret_cast<val_policingmap_t *>(GetVal(ckv));
  val_vtnpolicingmap_ctrl_t *ctrlr_val_pm =
    reinterpret_cast<val_vtnpolicingmap_ctrl_t *>(GetVal(ctrlr_key));
  if ((pm_val == NULL) || (ctrlr_val_pm == NULL)) {
    UPLL_LOG_DEBUG("Memory Allocation failed for Valstructure");
    return UPLL_RC_ERR_GENERIC;
  }

  if (op == UNC_OP_CREATE) {
    /* update the vtn status in main tbl */
    switch (pm_val->cs_row_status) {
      case UNC_CS_UNKNOWN:
        status = cs_status;
        break;
      case UNC_CS_PARTAILLY_APPLIED:
        if (ctrlr_val_pm->cs_row_status == UNC_CS_NOT_APPLIED) {
          // if this vtn has caused it then to change to applied.
        }
        break;
      case UNC_CS_APPLIED:
      case UNC_CS_NOT_APPLIED:
      case UNC_CS_INVALID:
      default:
        status = (cs_status == UNC_CS_APPLIED)?UNC_CS_PARTAILLY_APPLIED:status;
        break;
    }
    if (ctrlr_val_pm->cs_row_status != UNC_CS_NOT_SUPPORTED)
      ctrlr_val_pm->cs_row_status = cs_status;
// pyn need code
    pm_val->cs_row_status = status;
  } else if (op == UNC_OP_UPDATE) {
    if (ctrlr_val_pm->cs_row_status != UNC_CS_NOT_SUPPORTED)
      ctrlr_val_pm->cs_row_status = cs_status;
    pm_val->cs_row_status = status;
  }
#endif
  UPLL_LOG_DEBUG("UpdateConfigStatus Successfull.");
  return result_code;
}


upll_rc_t VtnPolicingMapMoMgr::ValidateReadAttribute(ConfigKeyVal *ikey,
    DalDmlIntf *dmi,
    IpcReqRespHeader *req) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code;
  if (UNC_OPT1_DETAIL == req->option1 &&
    (UNC_OP_READ == req->operation ||
     UNC_OP_READ_SIBLING == req->operation ||
     UNC_OP_READ_SIBLING_BEGIN == req->operation)) {
    ConfigKeyVal *okey = NULL;
    if (UNC_KT_VTN_POLICINGMAP_CONTROLLER == ikey->get_key_type()) {
      result_code = CopyVtnControllerCkv(ikey, okey);
      if (UPLL_RC_SUCCESS != result_code) {
        UPLL_LOG_DEBUG("CopyVtnControllerCkv failed %d", result_code);
        return result_code;
      }
    } else if (UNC_KT_VTN_POLICINGMAP == ikey->get_key_type()) {
      result_code = GetChildConfigKey(okey, ikey);
      if (UPLL_RC_SUCCESS != result_code) {
        UPLL_LOG_DEBUG("GetChildConfigKey failed %d", result_code);
        return result_code;
      }
    }
    DbSubOp dbop = {kOpReadMultiple, kOpMatchNone,
                    kOpInOutCtrlr | kOpInOutDomain};
    result_code = ReadConfigDB(okey, req->datatype, UNC_OP_READ, dbop, dmi,
                             CTRLRTBL);
    if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
      return UPLL_RC_ERR_NOT_ALLOWED_AT_THIS_TIME;
    } else if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("ReadConfigDB failed %d", result_code);
      delete okey;
      return result_code;
    }
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtnPolicingMapMoMgr::ValidateAttribute(ConfigKeyVal *ikey,
    DalDmlIntf *dmi,
    IpcReqRespHeader *req) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *okey = NULL;
  if (!ikey || !ikey->get_key()) {
    UPLL_LOG_DEBUG("input key is null");
    return UPLL_RC_ERR_GENERIC;
  }
  key_vtn_t *key_vtn_pm =
      reinterpret_cast<key_vtn_t *>(ikey->get_key());
  MoMgrImpl *mgr = reinterpret_cast<MoMgrImpl *>
      (const_cast<MoManager *>(GetMoManager(UNC_KT_VTN)));

  result_code = mgr->GetChildConfigKey(okey, NULL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Key not supported result - %d", result_code);
    return result_code;
  }

  key_vtn_t *vtn_key = reinterpret_cast<key_vtn_t *>(okey->get_key());
  uuu::upll_strncpy(vtn_key->vtn_name, key_vtn_pm->vtn_name,
                    (kMaxLenVtnName+1));

  /* Checks the given vtn exists or not */
  result_code = mgr->UpdateConfigDB(okey, req->datatype, UNC_OP_READ,
                                    dmi, MAINTBL);
  if (UPLL_RC_ERR_INSTANCE_EXISTS != result_code) {
    UPLL_LOG_DEBUG(" Parent VTN key does not exists");
    result_code = UPLL_RC_ERR_PARENT_DOES_NOT_EXIST;
  } else {
    result_code = UPLL_RC_SUCCESS;
  }
  delete okey;
  return result_code;
}

bool VtnPolicingMapMoMgr::CompareValidValue(void *&val1, void *val2,
    bool audit) {
  UPLL_FUNC_TRACE;
  bool invalid_attr = true;
  val_policingmap_t *val_pm1 = reinterpret_cast<val_policingmap_t *>(val1);
  val_policingmap_t *val_pm2 = reinterpret_cast<val_policingmap_t *>(val2);

//  if (audit) {
    if (UNC_VF_INVALID == val_pm1->valid[UPLL_IDX_POLICERNAME_PM] &&
        UNC_VF_VALID == val_pm2->valid[UPLL_IDX_POLICERNAME_PM]) {
      val_pm1->valid[UPLL_IDX_POLICERNAME_PM] = UNC_VF_VALID_NO_VALUE;
    }
 // }

  if (UNC_VF_VALID == val_pm1->valid[UPLL_IDX_POLICERNAME_PM]
      && UNC_VF_VALID == val_pm2->valid[UPLL_IDX_POLICERNAME_PM]) {
    if (!strcmp(reinterpret_cast<char*>(val_pm1->policer_name),
          reinterpret_cast<char*>(val_pm2->policer_name)))
      val_pm1->valid[UPLL_IDX_POLICERNAME_PM] = UNC_VF_INVALID;
  }
  return invalid_attr;
}

upll_rc_t VtnPolicingMapMoMgr::UpdateAuditConfigStatus(
    unc_keytype_configstatus_t cs_status,
    uuc::UpdateCtrlrPhase phase,
    ConfigKeyVal *&ckv_running) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  val_policingmap_t *val;
  val =
    (ckv_running != NULL) ? reinterpret_cast<val_policingmap_t *>(GetVal(
          ckv_running)) :
    NULL;
  if (NULL == val) {
    UPLL_LOG_DEBUG("UpdateAuditConfigStatus vtn_pm_val NULL (%d)", result_code);
    return UPLL_RC_ERR_GENERIC;
  }
  if (uuc::kUpllUcpCreate == phase )
    val->cs_row_status = cs_status;
  for ( unsigned int loop = 0;
      loop < sizeof(val->valid)/sizeof(uint8_t); ++loop ) {
    if (cs_status == UNC_CS_INVALID && UNC_VF_VALID == val->valid[loop])
      val->cs_attr[loop] = cs_status;
    else
      val->cs_attr[loop] = cs_status;
  }
  return result_code;
}

upll_rc_t VtnPolicingMapMoMgr::GetValid(void *val, uint64_t indx,
    uint8_t *&valid,
    upll_keytype_datatype_t dt_type,
    MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  if (val == NULL) {
    UPLL_LOG_DEBUG("GetValid val NULL");
    return UPLL_RC_ERR_GENERIC;
  }

  if (tbl == MAINTBL) {
    UPLL_LOG_DEBUG("GetValid MAINTBL");
    switch (indx) {
      case uudst::vtn_policingmap::kDbiPolicername:
        valid =
          &(reinterpret_cast<val_policingmap_t *>(val))
          ->valid[UPLL_IDX_POLICERNAME_PM];
        break;
      default:
        return UPLL_RC_ERR_GENERIC;
    }
  } else if (tbl == CTRLRTBL) {
    switch (indx) {
      case uudst::vtn_policingmap_ctrlr::kDbiPolicername:
        valid =
          &(reinterpret_cast<val_policingmap_controller_t *>(val))
          ->valid[UPLL_IDX_POLICERNAME_PM];
        break;
      default:
        return UPLL_RC_ERR_GENERIC;
    }
  } else {
    return UPLL_RC_ERR_GENERIC;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtnPolicingMapMoMgr::AllocVal(ConfigVal *&ck_val,
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
      UPLL_LOG_DEBUG("AllocVal MAINTBL");
      val = reinterpret_cast<void *>
          (ConfigKeyVal::Malloc(sizeof(val_policingmap_t)));
      ck_val = new ConfigVal(IpctSt::kIpcStValPolicingmap, val);
      break;
    case CTRLRTBL:
      UPLL_LOG_DEBUG("AllocVal CTRLTBL");
      val = reinterpret_cast<void *>
          (ConfigKeyVal::Malloc(sizeof(val_vtnpolicingmap_ctrl)));
      ck_val = new ConfigVal(IpctSt::kIpcInvalidStNum, val);
      break;
    default:
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

upll_rc_t VtnPolicingMapMoMgr::DupConfigKeyVal(ConfigKeyVal *&okey,
    ConfigKeyVal *&req,
    MoMgrTables tbl) {
  UPLL_FUNC_TRACE;

  if (req == NULL || okey != NULL) {
    UPLL_LOG_DEBUG("req NULL");
    return UPLL_RC_ERR_GENERIC;
  }

  if ((req->get_key_type() != UNC_KT_VTN_POLICINGMAP) &&
      (req->get_key_type() != UNC_KT_VTN_POLICINGMAP_CONTROLLER)) {
    UPLL_LOG_DEBUG("Invalid KeyType");
    return UPLL_RC_ERR_GENERIC;
  }

  ConfigVal *tmp1 = NULL, *tmp = (req)->get_cfg_val();

  if (tmp) {
    if (tbl == MAINTBL) {
      val_policingmap_t *ival = reinterpret_cast<val_policingmap_t *>
          (GetVal(req));
      if (NULL == ival) {
        UPLL_LOG_DEBUG("DupConfigKeyVal val_policingmap_t alloc failure");
        return UPLL_RC_ERR_GENERIC;
      }
      val_policingmap_t *policingmap_val =
          reinterpret_cast<val_policingmap_t *>
          (ConfigKeyVal::Malloc(sizeof(val_policingmap_t)));
      memcpy(policingmap_val, ival, sizeof(val_policingmap_t));
      if (NULL == policingmap_val) {
        UPLL_LOG_DEBUG("DupConfigKeyVal val_policingmap_t alloc failure");
        return UPLL_RC_ERR_GENERIC;
      }
      tmp1 = new ConfigVal(IpctSt::kIpcStValPolicingmap, policingmap_val);

      if (!tmp1) {
        UPLL_LOG_DEBUG("ConfigVal Alloc failed");
        free(policingmap_val);
        return UPLL_RC_ERR_GENERIC;
      }
      UPLL_LOG_DEBUG("val_policingmap_t Alloc Successful");
    } else if (tbl == CTRLRTBL) {
      val_vtnpolicingmap_ctrl *ival =
          reinterpret_cast<val_vtnpolicingmap_ctrl *>(GetVal(req));
      if (NULL == ival) {
        UPLL_LOG_DEBUG("val_policingmap_controller_t alloc failure");
        return UPLL_RC_ERR_GENERIC;
      }

      val_vtnpolicingmap_ctrl  *policingmap_ctrlr_val =
          reinterpret_cast<val_vtnpolicingmap_ctrl *>
          (ConfigKeyVal::Malloc(sizeof(val_vtnpolicingmap_ctrl)));
      memcpy(policingmap_ctrlr_val, ival, sizeof(val_vtnpolicingmap_ctrl));
      tmp1 = new ConfigVal(IpctSt::kIpcInvalidStNum,
                           policingmap_ctrlr_val);

      if (!tmp1) {
        UPLL_LOG_DEBUG("ConfigVal Alloc failed");
        free(policingmap_ctrlr_val);
        return UPLL_RC_ERR_GENERIC;
      }
      UPLL_LOG_DEBUG("DupConfigKeyValval_policingmap_t Alloc Successful");
    } else {
      UPLL_LOG_DEBUG("DupConfigKeyVal Invalid tbl Failed");
      return UPLL_RC_ERR_GENERIC;
    }

    if (tmp1) {
      tmp1->set_user_data(tmp->get_user_data());
      //    tmp = tmp->get_next_cfg_val();
    }
  }

  void *tkey = (req)->get_key();  // COV FORWARD NULL
  if (tbl == MAINTBL || tbl == CTRLRTBL) {
    key_vtn *ikey = reinterpret_cast<key_vtn *>(tkey);
    key_vtn *vtn_key = reinterpret_cast<key_vtn *>
        (ConfigKeyVal::Malloc(sizeof(key_vtn_t)));
    memcpy(vtn_key, ikey, sizeof(key_vtn_t));
    okey = new ConfigKeyVal(UNC_KT_VTN_POLICINGMAP,
                            IpctSt::kIpcStKeyVtn, vtn_key, tmp1);
    if (NULL == okey) {
      free(vtn_key);
      UPLL_LOG_DEBUG("DupConfigKeyVal okey Allocation Failed");
      return UPLL_RC_ERR_GENERIC;
    }
  } else {
    UPLL_LOG_DEBUG("DupConfigKeyVal Invalid tbl Failed");
    return UPLL_RC_ERR_GENERIC;
  }
  SET_USER_DATA(okey, req);
  UPLL_LOG_DEBUG("VtnPolicingMapMoMgr::DupConfigKeyVal Allocation Successful");

  return UPLL_RC_SUCCESS;
}

upll_rc_t VtnPolicingMapMoMgr::GetChildConfigKey(ConfigKeyVal *&okey,
    ConfigKeyVal *parent_key) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  key_vtn_t *vtn_key;
  void *pkey = NULL;

  if (parent_key == NULL) {
    vtn_key = reinterpret_cast<key_vtn_t *>
        (ConfigKeyVal::Malloc(sizeof(key_vtn_t)));
    okey = new ConfigKeyVal(UNC_KT_VTN_POLICINGMAP, IpctSt::kIpcStKeyVtn,
        vtn_key, NULL);
    UPLL_LOG_DEBUG("GetChildConfigKey parent key with NULL");
    return UPLL_RC_SUCCESS;
  } else {
    pkey = parent_key->get_key();
  }
  if (!pkey) {
    UPLL_LOG_DEBUG("GetChildConfigKey pkey NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  if (okey) {
    if (okey->get_key_type() != UNC_KT_VTN_POLICINGMAP)
      return UPLL_RC_ERR_GENERIC;
    vtn_key = reinterpret_cast<key_vtn_t *>(okey->get_key());
  } else {
    vtn_key = reinterpret_cast<key_vtn_t *>
        (ConfigKeyVal::Malloc(sizeof(key_vtn_t)));
  }

  switch (parent_key->get_key_type()) {
    case UNC_KT_VTN:
    case UNC_KT_VTN_POLICINGMAP:
      uuu::upll_strncpy(vtn_key->vtn_name,
          reinterpret_cast<key_vtn *>(pkey)->vtn_name,
          (kMaxLenVtnName + 1));
      break;
    default:
      if (vtn_key) free(vtn_key);
      return UPLL_RC_ERR_GENERIC;
  }

  if (!okey) {
    okey = new ConfigKeyVal(UNC_KT_VTN_POLICINGMAP, IpctSt::kIpcStKeyVtn,
        vtn_key, NULL);
  }
  SET_USER_DATA(okey, parent_key);
  UPLL_LOG_DEBUG("GetChildConfigKey Allocation Successful");
  return result_code;
}


upll_rc_t VtnPolicingMapMoMgr::GetChildControllerConfigKey(
    ConfigKeyVal *&okey, ConfigKeyVal *ikey,
    controller_domain *ctrlr_dom) {
  UPLL_FUNC_TRACE;
  if (NULL == ikey) {
    UPLL_LOG_DEBUG("parent key is null");
    return UPLL_RC_ERR_GENERIC;
  }
  key_vtn_policingmap_controller_t *out_key =
      reinterpret_cast<key_vtn_policingmap_controller_t *>
      (ConfigKeyVal::Malloc(sizeof(key_vtn_policingmap_controller_t)));
  if (UNC_KT_VTN_POLICINGMAP_CONTROLLER ==
      ikey->get_key_type()) {
    UPLL_LOG_DEBUG("UNC_KT_VTN_POLICINGMAP_CONTROLLER");
    key_vtn_policingmap_controller_t *in_key = reinterpret_cast
      <key_vtn_policingmap_controller_t *>(ikey->get_key());
    uuu::upll_strncpy(out_key->vtn_key.vtn_name,
          in_key->vtn_key.vtn_name,
          (kMaxLenVtnName + 1));
    uuu::upll_strncpy(out_key->controller_name,
                    in_key->controller_name,
                    (kMaxLenCtrlrId + 1));
    uuu::upll_strncpy(out_key->domain_id,
                    in_key->domain_id,
                    (kMaxLenDomainId + 1));
  }  else if (UNC_KT_VTN_POLICINGMAP == ikey->get_key_type()) {
    UPLL_LOG_DEBUG("parent kt - UNC_KT_VTN_POLICINGMAP");
    key_vtn_t *in_key = reinterpret_cast
      <key_vtn_t *>(ikey->get_key());
    uuu::upll_strncpy(out_key->vtn_key.vtn_name,
          in_key->vtn_name,
          (kMaxLenVtnName + 1));
    uuu::upll_strncpy(out_key->controller_name,
                    ctrlr_dom->ctrlr,
                    (kMaxLenCtrlrId + 1));
    uuu::upll_strncpy(out_key->domain_id,
                    ctrlr_dom->domain,
                    (kMaxLenDomainId + 1));
  }
  okey = new ConfigKeyVal(UNC_KT_VTN_POLICINGMAP_CONTROLLER,
        IpctSt::kIpcStKeyVtnPolicingmapController,
        out_key, NULL);
  if (NULL == okey) {
    UPLL_LOG_DEBUG("Okey is NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtnPolicingMapMoMgr::ValidateMessage(IpcReqRespHeader *req,
    ConfigKeyVal *key) {
  UPLL_FUNC_TRACE;

  upll_rc_t rt_code = UPLL_RC_ERR_GENERIC;

  if ((NULL == req) || (NULL == key)) {
    UPLL_LOG_DEBUG("IpcReqRespHeader/ConfigKeyval is NULL");
    return UPLL_RC_ERR_BAD_REQUEST;
  }

  key_vtn_t *key_vtn = NULL;

  key_vtn_policingmap_controller_t *key_vtn_policingmap_controller = NULL;

  if (UNC_KT_VTN_POLICINGMAP == key->get_key_type()) {
   if (req->option1 != UNC_OPT1_NORMAL) {
    UPLL_LOG_DEBUG(" Invalid option1(%d)", req->option1);
    return UPLL_RC_ERR_INVALID_OPTION1;
  }

  if (req->option2 != UNC_OPT2_NONE) {
    UPLL_LOG_DEBUG(" Invalid option2(%d)", req->option2);
    return UPLL_RC_ERR_INVALID_OPTION2;
  }
   
  if (key->get_st_num() != IpctSt::kIpcStKeyVtn) {
      UPLL_LOG_DEBUG(
          " Invalid structure received expected struct -"
          "kIpcStKeyVtn, received struct - %s ",
          reinterpret_cast<const char *> (IpctSt::GetIpcStdef(
              key->get_st_num())));
      return UPLL_RC_ERR_BAD_REQUEST;
    }
    key_vtn = reinterpret_cast<key_vtn_t *>(key->get_key());
  } else if (UNC_KT_VTN_POLICINGMAP_CONTROLLER == key->get_key_type()) {

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

    if (key->get_st_num() != IpctSt::kIpcStKeyVtnPolicingmapController) {
      UPLL_LOG_DEBUG(
          " Invalid structure received expected struct -"
          "kIpcStKeyVtnPolicingmapController, received struct - %s ",
          reinterpret_cast<const char *> (IpctSt::GetIpcStdef(
              key->get_st_num())));
      return UPLL_RC_ERR_BAD_REQUEST;
    }

    key_vtn_policingmap_controller =
      reinterpret_cast<key_vtn_policingmap_controller_t *>(key->get_key());
    key_vtn = &(key_vtn_policingmap_controller->vtn_key);
  } else {
    UPLL_LOG_DEBUG(" Invalid keytype(%d)", key->get_key_type());
    return UPLL_RC_ERR_GENERIC;
  }

  if (NULL == key_vtn) {
    UPLL_LOG_DEBUG("KT_VTN_POLICINGMAP Key structure is empty!!");
    return UPLL_RC_ERR_BAD_REQUEST;
  }

  /** validate key struct */
  VtnMoMgr *mgrvtn =
    reinterpret_cast<VtnMoMgr *>(const_cast<MoManager*>(GetMoManager(
            UNC_KT_VTN)));

  if (NULL == mgrvtn) {
    UPLL_LOG_DEBUG("unable to get VtnMoMgr object to validate key_vtn");
    return UPLL_RC_ERR_GENERIC;
  }

  rt_code = mgrvtn->ValidateVtnKey(key_vtn);

  if (UPLL_RC_SUCCESS != rt_code) {
    return rt_code;
  }

  /** read datatype, operation, options from IpcReqRespHeader */
  if (UNC_KT_VTN_POLICINGMAP == key->get_key_type()) {
    UPLL_LOG_TRACE(" key struct validation is success");
    val_policingmap_t *val_policingmap = NULL;

    if (key->get_cfg_val() && (key->get_cfg_val()->get_st_num() ==
          IpctSt::kIpcStValPolicingmap)) {
      val_policingmap =
        reinterpret_cast<val_policingmap_t *>(key->get_cfg_val()->get_val());
    }
    return ValidatePolicingMapValue(val_policingmap, req);
  }

  /** Validate UNC_KT_VTN_POLICINGMAP_CONTROLLER key, val structure */
  /** validate syntax and semantic check(controller name should exists
    in controller table otherwise return semantic error*/
  /* TODO ValidateControllerName() verify inclusion
     rt_code = ValidateControllerName(
     reinterpret_cast<char*>(key_vtn_policingmap_controller->controller_name));

     if (rt_code != UPLL_RC_SUCCESS) {
     return rt_code;
     }
     */
  val_policingmap_controller_t *val_policingmap_controller = NULL;

  if (key->get_cfg_val() && (key->get_cfg_val()->get_st_num() ==
        IpctSt::kIpcStValPolicingmapController)) {
    val_policingmap_controller =
      reinterpret_cast<val_policingmap_controller_t *>
      (key->get_cfg_val()->get_val());
  }

  UPLL_LOG_TRACE("KT_VTN_POLICINGMAP_CONTROLLER key struct validation"
      "is success");
  /** Read operation, datatype, option1 , option2 from req */
  uint32_t dt_type = req->datatype;
  uint32_t operation = req->operation;
  uint32_t option1 = req->option1;
  uint32_t option2 = req->option2;

  if (READ_SUPPORTED_OPERATION) {
    if (dt_type == UPLL_DT_STATE) {
      /** check option1 and option2 */
      if ((option1 != UNC_OPT1_NORMAL)
          && (option1 == UNC_OPT1_DETAIL
            && operation == UNC_OP_READ_SIBLING_COUNT)) {
        UPLL_LOG_DEBUG(" Error: option1 is not NORMAL/DETAIL");
        return UPLL_RC_ERR_INVALID_OPTION1;
      }
      if (option2 != UNC_OPT2_NONE) {
        UPLL_LOG_DEBUG(" Error: option2 is not NONE");
        return UPLL_RC_ERR_INVALID_OPTION2;
      }
      if (val_policingmap_controller) {
        return ValidateVtnPolicingMapControllerValue(
            val_policingmap_controller, operation);

      } else if (operation == UNC_OP_READ_SIBLING_COUNT) {
        return UPLL_RC_SUCCESS;
      } else {
        UPLL_LOG_DEBUG("Mandatory value structure is NULL");
        return UPLL_RC_ERR_CFG_SYNTAX;
      }
    } else {
      UPLL_LOG_DEBUG("Error Unsupported datatype (%d)", dt_type);
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  }
  UPLL_LOG_DEBUG("Error Unsupported operation (%d)", operation);
  return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_KT;
}

upll_rc_t VtnPolicingMapMoMgr::ValidatePolicingMapValue(
    val_policingmap_t *val_policingmap, IpcReqRespHeader *req) {

  UPLL_FUNC_TRACE;

  uint32_t dt_type = req->datatype;
  uint32_t operation = req->operation;
  uint32_t option1 = req->option1;
  uint32_t option2 = req->option2;

  if ((operation == UNC_OP_CREATE) || (operation == UNC_OP_UPDATE)) {
    if (dt_type == UPLL_DT_CANDIDATE) {
      if (val_policingmap) {
        return ValidatePolicingMapValue(val_policingmap, operation);
      } else {
        UPLL_LOG_TRACE("Error value struct is mandatory for CREATE/UPDATE");
        return UPLL_RC_ERR_BAD_REQUEST;
      }
    } else {
      UPLL_LOG_DEBUG("Unsupported datatype for CREATE/UPDATE");
      return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT;
    }
  } else if (READ_SUPPORTED_OPERATION) {
    if (READ_SUPPORTED_DATATYPE) {
      if ((option1 != UNC_OPT1_NORMAL)
          && ((option1 == UNC_OPT1_DETAIL)
            && (operation == UNC_OP_READ_SIBLING_COUNT))) {
        UPLL_LOG_DEBUG(" Error: option1 is not NORMAL/DETAIL");
        return UPLL_RC_ERR_INVALID_OPTION1;
      }
      if (option2 != UNC_OPT2_NONE) {
        UPLL_LOG_DEBUG(" Error: option2 is not NONE");
        return UPLL_RC_ERR_INVALID_OPTION2;
      }
      if (val_policingmap) {
        return ValidatePolicingMapValue(val_policingmap, operation);
      }
      /** value struct is optional*/
      return UPLL_RC_SUCCESS;
    } else {
      UPLL_LOG_DEBUG("Error Unsupported datatype (%d)", dt_type);
      return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT;
    }
  } else if (OPEARTION_WITH_VAL_STRUCT_NONE) {
    /** Value struct is NONE for this operations */
    UPLL_LOG_TRACE("Operation type is %d", operation);
    return UPLL_RC_SUCCESS;
  }

  UPLL_LOG_DEBUG("Error Unsupported operation (%d)", operation);
  return UPLL_RC_ERR_CFG_SYNTAX;
}

upll_rc_t VtnPolicingMapMoMgr::ValidatePolicingMapValue(
    val_policingmap_t *val_policingmap, uint32_t operation) {

  UPLL_FUNC_TRACE;
  if (val_policingmap != NULL) {
    if (val_policingmap->valid[UPLL_IDX_POLICERNAME_PM] == UNC_VF_INVALID) {
      UPLL_LOG_DEBUG("policer name flag is invalid");
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
    /** Validate policier name*/
    if (val_policingmap->valid[UPLL_IDX_POLICERNAME_PM] == UNC_VF_VALID) {
      /* validate name range is between 1 and 32 */
      if (ValidateKey(reinterpret_cast<char *>(val_policingmap->policer_name),
            kMinLenPolicingProfileName, kMaxLenPolicingProfileName + 1)
          != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG(" Syntax range check failed for name ");
        return UPLL_RC_ERR_CFG_SYNTAX;
      }
    } else if (val_policingmap->valid[UPLL_IDX_POLICERNAME_PM] ==
        UNC_VF_VALID_NO_VALUE &&
        (operation == UNC_OP_UPDATE || operation == UNC_OP_CREATE)) {
      UPLL_LOG_TRACE("Reset Policingname");
      memset(val_policingmap->policer_name, 0,
          sizeof(val_policingmap->policer_name));
    }
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtnPolicingMapMoMgr::ValidateVtnPolicingMapControllerValue(
    val_policingmap_controller_t *val_policingmap_controller,
    uint32_t operation) {

  UPLL_FUNC_TRACE;

  if (val_policingmap_controller != NULL) {
    /** Validate policier name*/
    if (val_policingmap_controller->valid[UPLL_IDX_SEQ_NUM_PMC]
        == UNC_VF_VALID) {
      /* validate name range is between 1 and 32 */
      if (!ValidateNumericRange(val_policingmap_controller->sequence_num,
            (uint8_t)kMinPolicingProfileSeqNum,
            (uint8_t)kMaxPolicingProfileSeqNum, true, true)) {
        UPLL_LOG_DEBUG(" Syntax range check failed for seq_num ");
        return UPLL_RC_ERR_CFG_SYNTAX;
      }
    } else if ((operation == UNC_OP_UPDATE)
        && (val_policingmap_controller->valid[UPLL_IDX_SEQ_NUM_PMC]
          == UNC_VF_VALID_NO_VALUE)) {
      UPLL_LOG_TRACE("Reset seq_num");
      val_policingmap_controller->sequence_num = 0;
    }
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtnPolicingMapMoMgr::ValidateCapability(IpcReqRespHeader *req,
    ConfigKeyVal *ikey, const char * ctrlr_name) {
  UPLL_FUNC_TRACE;
  if ((NULL == req) || (NULL == ikey)) {
    UPLL_LOG_DEBUG("IpcReqRespHeader/ConfigKeyval is NULL");
    return UPLL_RC_ERR_GENERIC;
  }

  if (!ctrlr_name)
    ctrlr_name = static_cast<char *>(ikey->get_user_data());

  bool result_code = false;
  uint32_t instance_count = 0;
  const uint8_t *attrs = NULL;
  uint32_t max_attrs = 0;

  /**VTN_POLICINGMAP, VBR_POLICINGMAP, VBRIF_POLICINGMAP, shares this code
   * to validate val_policingmap value structure capability */

  uint32_t dt_type = req->datatype;
  uint32_t operation = req->operation;
  uint32_t option1 = req->option1;
  uint32_t option2 = req->option2;

  UPLL_LOG_TRACE("dt_type   : (%d)"
      "operation : (%d)"
      "option1   : (%d)"
      "option2   : (%d)",
      dt_type, operation, option1, option2);

  switch (operation) {
    case UNC_OP_CREATE:
      result_code = GetCreateCapability(ctrlr_name, ikey->get_key_type(),
                                        &instance_count, &max_attrs, &attrs);
      break;
    case UNC_OP_UPDATE:
      result_code = GetUpdateCapability(ctrlr_name, ikey->get_key_type(),
                                        &max_attrs, &attrs);
      break;
    default:
      result_code = GetReadCapability(ctrlr_name, ikey->get_key_type(),
                                      &max_attrs, &attrs);
      break;
  }

  if (!result_code) {
    UPLL_LOG_DEBUG("keytype(%d) is not supported by controller(%s)"
        " for operation(%d)",
        ikey->get_key_type(), ctrlr_name, operation);
    return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_KT;
  }

  if (UNC_KT_VTN_POLICINGMAP_CONTROLLER == ikey->get_key_type()) {
    /** capability check for UNC_KT_VTN_POLICINGMAP_CONTROLLER */
    return ValidateVtnPolicingMapControllerCapability(req, ikey,
        ctrlr_name, max_attrs, attrs);
  }

  val_policingmap_t *val_policingmap = NULL;

  if (ikey->get_cfg_val() && (ikey->get_cfg_val()->get_st_num() ==
        IpctSt::kIpcStValPolicingmap)) {
    val_policingmap =
      reinterpret_cast<val_policingmap_t *>(ikey->get_cfg_val()->get_val());
  }

  if ((operation == UNC_OP_CREATE) || (operation == UNC_OP_UPDATE)) {
    if (dt_type == UPLL_DT_CANDIDATE) {
      if (val_policingmap) {
        if (max_attrs > 0) {
          return ValVtnPolicingMapAttributeSupportCheck(ikey,
              attrs);
        } else {
          UPLL_LOG_DEBUG("Attribute list is empty for operation %d", operation);
          return UPLL_RC_ERR_CFG_SYNTAX;
        }
      } else {
        UPLL_LOG_TRACE("Error value struct is mandatory for CREATE/UPDATE");
        return UPLL_RC_ERR_CFG_SYNTAX;
      }
    } else {
      UPLL_LOG_DEBUG("Unsupported datatype for CREATE/UPDATE");
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  } else if (READ_SUPPORTED_OPERATION) {
    if (READ_SUPPORTED_DATATYPE) {
      if (option1 != UNC_OPT1_NORMAL) {
        UPLL_LOG_DEBUG(" Error: option1 is not NORMAL");
        return UPLL_RC_ERR_INVALID_OPTION1;
      }
      if (option2 != UNC_OPT2_NONE) {
        UPLL_LOG_DEBUG(" Error: option2 is not NONE");
        return UPLL_RC_ERR_INVALID_OPTION2;
      }
      if (val_policingmap) {
        if (max_attrs > 0) {
          return ValVtnPolicingMapAttributeSupportCheck(ikey,
              attrs);
        } else {
          UPLL_LOG_DEBUG("Attribute list is empty for operation %d", operation);
          return UPLL_RC_ERR_CFG_SYNTAX;
        }
      }
      /** value struct is optional */
      return UPLL_RC_SUCCESS;
    } else {
      UPLL_LOG_DEBUG("Error Unsupported datatype (%d)", dt_type);
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  } else if (OPEARTION_WITH_VAL_STRUCT_NONE) {
    /** Value struct is NONE for this operations */
    UPLL_LOG_TRACE("Skip attribute validation, Operation type is %d",
        operation);
    return UPLL_RC_SUCCESS;
  }
  UPLL_LOG_DEBUG("Error Unsupported operation(%d)", operation);
  return UPLL_RC_ERR_CFG_SYNTAX;
}

upll_rc_t VtnPolicingMapMoMgr::ValVtnPolicingMapAttributeSupportCheck(
    ConfigKeyVal *ikey,  const uint8_t* attrs) {
  UPLL_FUNC_TRACE;

  if (UNC_KT_VTN_POLICINGMAP_CONTROLLER == ikey->get_key_type()) {
    val_policingmap_controller_t *val_policingmap_controller = NULL;

    if (ikey->get_cfg_val() && (ikey->get_cfg_val()->get_st_num() ==
          IpctSt::kIpcStValPolicingmapController)) {
      val_policingmap_controller =
        reinterpret_cast<val_policingmap_controller_t *>
        (ikey->get_cfg_val()->get_val());
    }
    if (val_policingmap_controller != NULL) {
      if ((val_policingmap_controller->valid[UPLL_IDX_SEQ_NUM_PMC]
            == UNC_VF_VALID)
          || (val_policingmap_controller->valid[UPLL_IDX_SEQ_NUM_PMC]
            == UNC_VF_VALID_NO_VALUE)) {
        if (attrs[unc::capa::vtn_policingmap_controller::kCapSeqNum] == 0) {
          val_policingmap_controller->valid[UPLL_IDX_SEQ_NUM_PMC] =
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

  /**VTN_POLICINGMAP, VBR_POLICINGMAP, VBRIF_POLICINGMAP, shares this code
   * to validate val_policingmap value structure capability */
  if (GetVal(ikey)) {
    val_policingmap_t *val_policingmap =
      reinterpret_cast<val_policingmap_t *>(ikey->get_cfg_val()->get_val());
    if (val_policingmap != NULL) {
      if ((val_policingmap->valid[UPLL_IDX_POLICERNAME_PM] == UNC_VF_VALID)
          || (val_policingmap->valid[UPLL_IDX_POLICERNAME_PM]
            == UNC_VF_VALID_NO_VALUE)) {
        if (attrs[unc::capa::vtn_policingmap::kCapPolicername] == 0) {
          val_policingmap->valid[UPLL_IDX_POLICERNAME_PM] =
            UNC_VF_NOT_SOPPORTED;
          UPLL_LOG_DEBUG("Policername attr is not supported by ctrlr");
          return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
        }
      }
    } else {
      UPLL_LOG_DEBUG("Error value struct is NULL");
      return UPLL_RC_ERR_GENERIC;
    }
  } else {
    UPLL_LOG_DEBUG("val structure NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtnPolicingMapMoMgr::ValidateVtnPolicingMapControllerCapability(
    IpcReqRespHeader *req, ConfigKeyVal *ikey, const char *ctrlr_name,
    uint32_t max_attrs, const uint8_t *attrs) {
  UPLL_FUNC_TRACE;

  val_policingmap_controller_t *val_policingmap_controller = NULL;

  if (ikey->get_cfg_val() && (ikey->get_cfg_val()->get_st_num() ==
        IpctSt::kIpcStValPolicingmapController)) {
    val_policingmap_controller =
      reinterpret_cast<val_policingmap_controller_t *>
      (ikey->get_cfg_val()->get_val());
  }

  uint32_t dt_type = req->datatype;
  uint32_t operation = req->operation;
  uint32_t option1 = req->option1;
  uint32_t option2 = req->option2;

  if (READ_SUPPORTED_OPERATION) {
    if (dt_type == UPLL_DT_STATE) {
      /** check option1 and option2 */
      if ((option1 == UNC_OPT1_NORMAL)
          || (option1 == UNC_OPT1_DETAIL
            && operation != UNC_OP_READ_SIBLING_COUNT)) {
        if (option2 == UNC_OPT2_NONE) {
          if (val_policingmap_controller) {
            if (max_attrs > 0) {
              return ValVtnPolicingMapAttributeSupportCheck(
                  ikey, attrs);
            } else {
              UPLL_LOG_DEBUG("Attribute list is empty for operation %d",
                  operation);
              return UPLL_RC_ERR_CFG_SYNTAX;
            }
          } else if (operation == UNC_OP_READ_SIBLING_COUNT) {
            return UPLL_RC_SUCCESS;
          } else {
            UPLL_LOG_DEBUG("Mandatory value structure is NULL");
            return UPLL_RC_ERR_CFG_SYNTAX;
          }
        } else {
          UPLL_LOG_DEBUG(" Error: option2 is not NONE");
          return UPLL_RC_ERR_INVALID_OPTION2;
        }
      } else {
        UPLL_LOG_DEBUG(" Error: option1 is not NORMAL/DETAIL");
        return UPLL_RC_ERR_INVALID_OPTION1;
      }
    } else {
      UPLL_LOG_DEBUG("Error Unsupported datatype(%d)", dt_type);
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  }
  UPLL_LOG_DEBUG("Error Unsupported operation (%d)", operation);
  return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_KT;
}

upll_rc_t VtnPolicingMapMoMgr::GetParentConfigKey(ConfigKeyVal *&okey,
                                                  ConfigKeyVal *ikey) {
  UPLL_FUNC_TRACE;

  if (!ikey) {
    UPLL_LOG_DEBUG(" Input Key is NULL");
    return UPLL_RC_ERR_GENERIC;
  }

  unc_key_type_t ikey_type = ikey->get_key_type();
  if (ikey_type != UNC_KT_VTN_POLICINGMAP) {
    UPLL_LOG_DEBUG(" Invalid key type received. Key type - %d", ikey_type);
    return UPLL_RC_ERR_GENERIC;
  }

  key_vtn_t *pkey = reinterpret_cast<key_vtn_t*>(ikey->get_key());
  if (!pkey) {
    UPLL_LOG_DEBUG(" Input vtn policing map key is NULL ");
    return UPLL_RC_ERR_GENERIC;
  }

  key_vtn_t *vtn_key = reinterpret_cast<key_vtn_t*>
      (ConfigKeyVal::Malloc(sizeof(key_vtn_t)));

  memcpy(vtn_key, reinterpret_cast<key_vtn_t*>(pkey), (kMaxLenVtnName + 1));
  okey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, vtn_key, NULL);
  SET_USER_DATA(okey, ikey);
  return UPLL_RC_SUCCESS;
}

bool VtnPolicingMapMoMgr::IsValidKey(void *key, uint64_t index) {
  UPLL_FUNC_TRACE;
  key_vtn *vtn_key = reinterpret_cast<key_vtn *>(key);
  upll_rc_t ret_val = UPLL_RC_SUCCESS;

  ret_val = ValidateKey(reinterpret_cast<char *>(vtn_key->vtn_name),
                        kMinLenVtnName, kMaxLenVtnName + 1);

  if (ret_val != UPLL_RC_SUCCESS) {
    UPLL_LOG_TRACE("VTN Name is not valid(%d)", ret_val);
    return false;
  }
  return true;
}
upll_rc_t VtnPolicingMapMoMgr::IsKeyInUse(upll_keytype_datatype_t dt_type,
    const ConfigKeyVal *ckv,
    bool *in_use,
    DalDmlIntf *dmi) {
  return UPLL_RC_SUCCESS;
}


upll_rc_t VtnPolicingMapMoMgr::ReadControllerStateDetail(ConfigKeyVal *&ikey,
    ConfigKeyVal *vtn_dup_key,
    IpcResponse *ipc_response,
    upll_keytype_datatype_t dt_type,
    unc_keytype_operation_t op,
    DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  char policingprofile_name[33] = {0};
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigVal *temp_cfg_val = NULL;

  if (ipc_response->ckv_data == NULL) {
    UPLL_LOG_DEBUG("Invalid data received from driver");
    return UPLL_RC_ERR_GENERIC;
  }
  temp_cfg_val = ipc_response->ckv_data->get_cfg_val();
  key_vtn_policingmap_controller_t *vtn_pm_ctrlr_key = reinterpret_cast
      <key_vtn_policingmap_controller_t *>(ipc_response->ckv_data->get_key());
  val_policingmap_t *val_policingmap =
      reinterpret_cast<val_policingmap_t *>(GetVal(vtn_dup_key));
  val_policingmap_t *out_val_policingmap =
      reinterpret_cast<val_policingmap_t *>
      (ConfigKeyVal::Malloc(sizeof(val_policingmap_t)));
  memcpy(out_val_policingmap, val_policingmap, sizeof(val_policingmap_t));
  ikey->AppendCfgVal(IpctSt::kIpcStValPolicingmap, out_val_policingmap);
  delete vtn_dup_key;
#if 0
  val_policingmap_controller_t* val_policingmap_controller =
      reinterpret_cast<val_policingmap_controller_t *>(GetVal(ikey));
#endif
  while (temp_cfg_val != NULL) {
    val_policingmap_controller_st *val_entry_st = NULL;
    if (IpctSt::kIpcStValPolicingmapControllerSt ==
        temp_cfg_val->get_st_num()) {
      val_entry_st = reinterpret_cast< val_policingmap_controller_st *>
          (temp_cfg_val->get_val());
    } else {
      return  UPLL_RC_ERR_GENERIC;
    }

    if (val_entry_st->valid[UPLL_IDX_SEQ_NUM_FFES] == UNC_VF_VALID) {
      UPLL_LOG_DEBUG("seq num is valid, seq num - %d",
                     val_entry_st->sequence_num);
      ConfigKeyVal *ppe_ckv = NULL;

      uuu::upll_strncpy(
          policingprofile_name,
          out_val_policingmap->policer_name,
          (kMaxLenPolicingProfileName + 1));
      uint8_t seq_num  = val_entry_st->sequence_num;

      PolicingProfileEntryMoMgr *mgr =
          reinterpret_cast<PolicingProfileEntryMoMgr*>
          (const_cast<MoManager *>(GetMoManager
                                   (UNC_KT_POLICING_PROFILE_ENTRY)));

      result_code = mgr->ReadPolicingProfileEntry(
          (const char*)(policingprofile_name),
          (uint8_t)seq_num,
          (const char*)vtn_pm_ctrlr_key->controller_name,
          dmi,
          dt_type,
          ppe_ckv);

      if (result_code != UPLL_RC_SUCCESS) {
        return result_code;
      }
      ikey->AppendCfgVal(IpctSt::kIpcStValPolicingmapControllerSt,
                         val_entry_st);

      // val_policingmap_controller->sequence_num = val_entry_st->sequence_num;

      if (ppe_ckv) {
        val_policingprofile_entry_t *temp_val_policingprofile =
            reinterpret_cast<val_policingprofile_entry_t *>
            (ppe_ckv->get_cfg_val()->get_val());
        val_policingprofile_entry_t *val_ppe =
            reinterpret_cast<val_policingprofile_entry_t *>
            (ConfigKeyVal::Malloc(sizeof(val_policingprofile_entry_t)));
        memcpy(val_ppe, temp_val_policingprofile,
               sizeof(val_policingprofile_entry_t));

        ikey->AppendCfgVal(IpctSt::kIpcStValPolicingprofileEntry,
                           val_ppe);
        delete ppe_ckv;
        ppe_ckv = NULL;
      }

      temp_cfg_val = temp_cfg_val->get_next_cfg_val();
      if (NULL == temp_cfg_val) {
        UPLL_LOG_DEBUG("No val_policingmap_switch_st in configkeyval");
        continue;
      }

      while (IpctSt::kIpcStValPolicingmapSwitchSt ==
             temp_cfg_val->get_st_num()) {
        UPLL_LOG_DEBUG("kIpcStValPolicingmapSwitchSt");
        ikey->AppendCfgVal(IpctSt::kIpcStValPolicingmapSwitchSt,
                           temp_cfg_val->get_val());
        temp_cfg_val = temp_cfg_val->get_next_cfg_val();
        if (temp_cfg_val == NULL)
          break;
      }
    }
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtnPolicingMapMoMgr::ReadSibDTStateNormal(ConfigKeyVal *ikey,
    ConfigKeyVal *vtn_dup_key,
    ConfigKeyVal *tctrl_key,
    upll_keytype_datatype_t dt_type,
    unc_keytype_operation_t op,
    DbSubOp dbop,
    DalDmlIntf *dmi,
    uint8_t* ctrlr_id) {
  UPLL_FUNC_TRACE;
  char policingprofile_name[33] = {0};
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  val_policingmap_t *val_policingmap =
    reinterpret_cast<val_policingmap_t *>(GetVal(vtn_dup_key));

  ikey->AppendCfgVal(IpctSt::kIpcStValPolicingmap, val_policingmap);

  val_policingmap_controller_t *val_policingmap_controller =
    reinterpret_cast<val_policingmap_controller_t *>(GetVal(ikey));

  key_policingprofile_entry_t* key_policingprofile_entry =
    reinterpret_cast<key_policingprofile_entry_t *>(tctrl_key->get_key());

  val_policingmap_controller->sequence_num =
    key_policingprofile_entry->sequence_num;

  ikey->AppendCfgVal(IpctSt::kIpcStValPolicingmapController,
      val_policingmap_controller);

  ConfigKeyVal *tkey = NULL;

  if (val_policingmap->policer_name[0] != '\0') {
    uuu::upll_strncpy(
        policingprofile_name,
        val_policingmap->policer_name,
        (kMaxLenPolicingProfileName + 1));
  }

  PolicingProfileEntryMoMgr *mgr = reinterpret_cast<PolicingProfileEntryMoMgr*>
    (const_cast<MoManager *>(GetMoManager(UNC_KT_POLICING_PROFILE_ENTRY)));

  result_code = mgr->ReadPolicingProfileEntry(
      (const char*)(policingprofile_name),
      (uint8_t)key_policingprofile_entry->sequence_num,
      (const char*)ctrlr_id,
      dmi,
      dt_type,
      tkey);
  if (result_code != UPLL_RC_SUCCESS) {
    return result_code;
  }
  if (tkey) {
    val_policingprofile_entry_t *temp_val_policingprofile =
      reinterpret_cast<val_policingprofile_entry_t *>
      (tkey->get_cfg_val()->get_val());

    ikey->AppendCfgVal(IpctSt::kIpcStValPolicingprofileEntry,
        temp_val_policingprofile);
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtnPolicingMapMoMgr::CopyVtnControllerCkv(ConfigKeyVal *ikey,
                               ConfigKeyVal *&okey) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_ERR_GENERIC;
  // controller_domain ctrlr_dom;
  key_vtn_policingmap_controller_t *key_pm_ctrlr = reinterpret_cast
      <key_vtn_policingmap_controller_t *>(ikey->get_key());
  result_code = GetChildConfigKey(okey, NULL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetChildConfigKey failed %d", result_code);
    return result_code;
  }
  key_vtn_t *key_pm_vtn = reinterpret_cast
      <key_vtn_t *>(okey->get_key());
  uuu::upll_strncpy(key_pm_vtn->vtn_name,
          key_pm_ctrlr->vtn_key.vtn_name,
          (kMaxLenVtnName + 1));
  SET_USER_DATA_CTRLR(okey, key_pm_ctrlr->controller_name);
  SET_USER_DATA_DOMAIN(okey, key_pm_ctrlr->domain_id);
  // SET_USER_DATA_CTRLR_DOMAIN(okey, ctrlr_dom);
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtnPolicingMapMoMgr::ReadControllerStateNormal(ConfigKeyVal *&ikey,
    upll_keytype_datatype_t dt_type, unc_keytype_operation_t op,
    DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_ERR_GENERIC;
  ConfigKeyVal *main_ckv = NULL, *ctrlr_ckv = NULL;
  result_code = CopyVtnControllerCkv(ikey, main_ckv);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("CopyVtnControllerCkv failed %d", result_code);
    return result_code;
  }
  result_code = GetChildConfigKey(ctrlr_ckv, main_ckv);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetChildConfigKey failed %d", result_code);
  }
  DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutCs };
  DbSubOp dbop1 = { kOpReadSingle, kOpMatchCtrlr, kOpInOutCs };

  result_code = ReadConfigDB(main_ckv, dt_type, op, dbop, dmi,
                             MAINTBL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("ReadConfigDB failed in maintbl %d", result_code);
    delete ctrlr_ckv;
    delete main_ckv;
    return result_code;
  }
  result_code = ReadConfigDB(ctrlr_ckv, dt_type, op, dbop1, dmi,
                             CTRLRTBL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("ReadConfigDB failed in ctrlrtbl %d", result_code);
    return result_code;
  }

  val_policingmap_t *val_main = reinterpret_cast<val_policingmap_t *>
      (GetVal(main_ckv));
  val_policingmap_t *val_ctrlr = reinterpret_cast<val_policingmap_t *>
      (GetVal(ctrlr_ckv));
  val_main->valid[UPLL_IDX_POLICERNAME_PM] =
      val_ctrlr->valid[UPLL_IDX_POLICERNAME_PM];
  val_main->cs_attr[UPLL_IDX_POLICERNAME_PM] =
      val_ctrlr->cs_attr[UPLL_IDX_POLICERNAME_PM];
  val_main->cs_row_status = val_ctrlr->cs_row_status;

  val_policingmap_controller_t *in_val_vtn_ctrlr_pm = reinterpret_cast
      <val_policingmap_controller_t *>(GetVal(ikey));
  uint8_t *ctrlr_id = NULL;
  GET_USER_DATA_CTRLR(ctrlr_ckv, ctrlr_id);
  ConfigKeyVal *ppe_ckv = NULL, *temp_ppe_ckv = NULL;
  if (UNC_VF_INVALID == in_val_vtn_ctrlr_pm->valid[UPLL_IDX_SEQ_NUM_PMC]) {
        in_val_vtn_ctrlr_pm->sequence_num = 0;
  }
  UPLL_LOG_DEBUG("Sequence number - %d", in_val_vtn_ctrlr_pm->sequence_num);
  PolicingProfileEntryMoMgr *ppe_mgr =
      reinterpret_cast<PolicingProfileEntryMoMgr*>
      (const_cast<MoManager *>(GetMoManager
                               (UNC_KT_POLICING_PROFILE_ENTRY)));
  result_code = ppe_mgr->ReadPolicingProfileEntry(reinterpret_cast
                                                  <const char *>(val_main->policer_name), in_val_vtn_ctrlr_pm->sequence_num,
                                                  reinterpret_cast<const char *>(ctrlr_id), dmi, dt_type, ppe_ckv);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("ReadPolicingProfileEntry failed %d", result_code);
    return result_code;
  }

  ikey->DeleteCfgVal();
  ikey->AppendCfgVal(IpctSt::kIpcStValPolicingmap,
                     val_main);
  temp_ppe_ckv = ppe_ckv;
  while (NULL != temp_ppe_ckv) {
    key_policingprofile_entry_t *key_ppe = reinterpret_cast
        <key_policingprofile_entry_t*>(temp_ppe_ckv->get_key());
    val_policingprofile_entry_t *val_ppe = reinterpret_cast
        <val_policingprofile_entry_t *>(GetVal(temp_ppe_ckv));
   val_policingmap_controller_t *out_val_vtn_ctrlr_pm = reinterpret_cast
      <val_policingmap_controller_t *>
      (ConfigKeyVal::Malloc(sizeof(val_policingmap_controller_t)));
    out_val_vtn_ctrlr_pm->valid[UPLL_IDX_SEQ_NUM_PMC] = UNC_VF_VALID;
    out_val_vtn_ctrlr_pm->sequence_num = key_ppe->sequence_num;
    ikey->AppendCfgVal(IpctSt::kIpcStValPolicingmapController,
                     out_val_vtn_ctrlr_pm);
    val_policingprofile_entry_t *in_val_ppe =
        reinterpret_cast<val_policingprofile_entry_t *>
        (ConfigKeyVal::Malloc(sizeof(val_policingprofile_entry_t)));
    memcpy(in_val_ppe, val_ppe, sizeof(val_policingprofile_entry_t));
    ikey->AppendCfgVal(IpctSt::kIpcStValPolicingprofileEntry, in_val_ppe);
    temp_ppe_ckv = temp_ppe_ckv->get_next_cfg_key_val();
  }
  if (ppe_ckv) {
    delete ppe_ckv;
    ppe_ckv = NULL;
  }
  return UPLL_RC_SUCCESS;
}


upll_rc_t VtnPolicingMapMoMgr::ConstructReadSiblingNormalResponse(ConfigKeyVal *&ikey,
    upll_keytype_datatype_t dt_type, DalDmlIntf *dmi,
    ConfigKeyVal *ctrlr_ckv, ConfigKeyVal *&resp_ckv) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_ERR_GENERIC;
  ConfigKeyVal *main_ckv = NULL;
  result_code = GetChildConfigKey(main_ckv, ctrlr_ckv);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetChildConfigKey failed %d", result_code);
  }
  DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutCs };

  result_code = ReadConfigDB(main_ckv, dt_type, UNC_OP_READ, dbop, dmi,
                             MAINTBL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("ReadConfigDB failed in maintbl %d", result_code);
    delete main_ckv;
    return result_code;
  }

  val_policingmap_t *val_main = reinterpret_cast<val_policingmap_t *>
        (GetVal(main_ckv));
  val_policingmap_t *val_ctrlr = reinterpret_cast<val_policingmap_t *>
        (GetVal(ctrlr_ckv));
  val_main->valid[UPLL_IDX_POLICERNAME_PM] =
      val_ctrlr->valid[UPLL_IDX_POLICERNAME_PM];
  val_main->cs_attr[UPLL_IDX_POLICERNAME_PM] =
      val_ctrlr->cs_attr[UPLL_IDX_POLICERNAME_PM];
  val_main->cs_row_status = val_ctrlr->cs_row_status;

  val_policingmap_controller_t *in_val_vtn_ctrlr_pm = reinterpret_cast
    <val_policingmap_controller_t *>(GetVal(ikey));
  val_policingmap_controller_t *out_val_vtn_ctrlr_pm =
      reinterpret_cast<val_policingmap_controller_t *>
    (ConfigKeyVal::Malloc(sizeof(val_policingmap_controller_t)));
  memcpy(out_val_vtn_ctrlr_pm, in_val_vtn_ctrlr_pm,
      sizeof(val_policingmap_controller_t));
  ConfigKeyVal *okey = NULL;
  result_code = GetChildVtnCtrlrKey(okey, ikey);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetChildVtnCtrlrKey failed ");
    return result_code;
  }
  okey->AppendCfgVal(IpctSt::kIpcStValPolicingmap,
      val_main);
  okey->AppendCfgVal(IpctSt::kIpcStValPolicingmapController,
      out_val_vtn_ctrlr_pm);
  uint8_t *ctrlr_id = NULL;
  GET_USER_DATA_CTRLR(ctrlr_ckv, ctrlr_id);
  ConfigKeyVal *ppe_ckv = NULL, *temp_ppe_ckv = NULL;
  if (UNC_VF_INVALID == out_val_vtn_ctrlr_pm->valid[UPLL_IDX_SEQ_NUM_PMC ]) {
    out_val_vtn_ctrlr_pm->sequence_num = 0;
  }
  UPLL_LOG_DEBUG("Sequence number - %d", out_val_vtn_ctrlr_pm->sequence_num);
  PolicingProfileEntryMoMgr *ppe_mgr =
    reinterpret_cast<PolicingProfileEntryMoMgr*>
    (const_cast<MoManager *>(GetMoManager
                             (UNC_KT_POLICING_PROFILE_ENTRY)));
  result_code = ppe_mgr->ReadPolicingProfileEntry(reinterpret_cast
    <const char *>(val_main->policer_name), out_val_vtn_ctrlr_pm->sequence_num,
    reinterpret_cast<const char *>(ctrlr_id), dmi, dt_type, ppe_ckv);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("ReadPolicingProfileEntry failed %d", result_code);
    return result_code;
  }
  temp_ppe_ckv = ppe_ckv;
  while (NULL != temp_ppe_ckv) {
    val_policingprofile_entry_t *val_ppe = reinterpret_cast
      <val_policingprofile_entry_t *>(GetVal(temp_ppe_ckv));
    val_policingprofile_entry_t *in_val_ppe =
        reinterpret_cast<val_policingprofile_entry_t *>
        (ConfigKeyVal::Malloc(sizeof(val_policingprofile_entry_t)));
    memcpy(in_val_ppe, val_ppe, sizeof(val_policingprofile_entry_t));
    okey->AppendCfgVal(IpctSt::kIpcStValPolicingprofileEntry, in_val_ppe);
    temp_ppe_ckv = temp_ppe_ckv->get_next_cfg_key_val();
  }
  if (ppe_ckv) {
    delete ppe_ckv;
    ppe_ckv = NULL;
  }
  if (!resp_ckv) {
    resp_ckv = okey;
  } else {
    resp_ckv->AppendCfgKeyVal(okey);
  }
  return UPLL_RC_SUCCESS;
}


upll_rc_t VtnPolicingMapMoMgr::GetChildVtnCtrlrKey(
    ConfigKeyVal *&okey, ConfigKeyVal *ikey) {
  UPLL_FUNC_TRACE;
  if (NULL == ikey) {
    UPLL_LOG_DEBUG("ikey is NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  key_vtn_policingmap_controller_t *vtn_pm_ctrlr_key = reinterpret_cast
    <key_vtn_policingmap_controller_t *>(ikey->get_key());
  key_vtn_policingmap_controller_t *out_key =
      reinterpret_cast<key_vtn_policingmap_controller_t *>
      (ConfigKeyVal::Malloc(sizeof(key_vtn_policingmap_controller_t)));
  uuu::upll_strncpy(
      out_key->vtn_key.vtn_name,
      vtn_pm_ctrlr_key->vtn_key.vtn_name,
      (kMaxLenVtnName +1));
  uuu::upll_strncpy(
      out_key->controller_name,
      vtn_pm_ctrlr_key->controller_name,
      (kMaxLenVtnName +1));
  uuu::upll_strncpy(
      out_key->domain_id,
      vtn_pm_ctrlr_key->domain_id,
      (kMaxLenVtnName +1));
  okey = new ConfigKeyVal(UNC_KT_VTN_POLICINGMAP_CONTROLLER,
      IpctSt::kIpcStKeyVtnPolicingmapController,
      out_key, NULL);
  if (!okey) {
    UPLL_LOG_DEBUG("GetChildVtnCtrlrKey failed. okey is null");
    return UPLL_RC_ERR_GENERIC;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtnPolicingMapMoMgr::SendIpcrequestToDriver(
    ConfigKeyVal *ikey, IpcReqRespHeader *req,
    IpcResponse *ipc_response, DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_ERR_GENERIC;
  controller_domain ctrlr_dom;
  key_vtn_policingmap_controller_t *vtn_pm_ctrlr_key = reinterpret_cast
    <key_vtn_policingmap_controller_t *>(ikey->get_key());
  ctrlr_dom.ctrlr = reinterpret_cast <uint8_t*>
      (ConfigKeyVal::Malloc(kMaxLenCtrlrId + 1));
  ctrlr_dom.domain = reinterpret_cast <uint8_t*>
      (ConfigKeyVal::Malloc(kMaxLenDomainId + 1));
  uuu::upll_strncpy(ctrlr_dom.ctrlr, vtn_pm_ctrlr_key->controller_name,
      kMaxLenCtrlrId + 1);
  uuu::upll_strncpy(ctrlr_dom.domain, vtn_pm_ctrlr_key->domain_id,
      kMaxLenDomainId + 1);
  ConfigVal *tmp1 = NULL;
  val_policingmap_controller_t *val_pm_ctrlr = reinterpret_cast
    <val_policingmap_controller_t *>(GetVal(ikey));
  val_policingmap_controller_t *out_val_pm_ctrlr =
      reinterpret_cast<val_policingmap_controller_t *>
      (ConfigKeyVal::Malloc(sizeof(val_policingmap_controller_t)));
  memcpy(out_val_pm_ctrlr, val_pm_ctrlr,
      sizeof(val_policingmap_controller_t));
  tmp1 = new ConfigVal(IpctSt::kIpcStValPolicingmapController,
      out_val_pm_ctrlr);
  ConfigKeyVal *temp_ckv = NULL;
  result_code = GetChildVtnCtrlrKey(temp_ckv, ikey);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetChildVtnCtrlrKey failed %d", result_code);
    return result_code;
  }
  if (!temp_ckv) {
    UPLL_LOG_DEBUG("temp_ckv is NULL");
    delete tmp1;
    return UPLL_RC_ERR_GENERIC;
  }
  temp_ckv->AppendCfgVal(tmp1);
  IpcRequest ipc_req;
  memset(&ipc_req, 0, sizeof(ipc_req));
  ipc_req.header.clnt_sess_id = req->clnt_sess_id;
  ipc_req.header.config_id = req->config_id;
  ipc_req.header.operation = UNC_OP_READ;
  ipc_req.header.option1 = req->option1;
  ipc_req.header.datatype = req->datatype;
  ipc_req.ckv_data = temp_ckv;
  if (!IpcUtil::SendReqToDriver(
          reinterpret_cast<const char *>(ctrlr_dom.ctrlr),
          reinterpret_cast<char *>(ctrlr_dom.domain),
          PFCDRIVER_SERVICE_NAME, PFCDRIVER_SVID_LOGICAL, &ipc_req,
          true, ipc_response)) {
    UPLL_LOG_DEBUG("SendReqToDriver failed for Key %d controller %s",
                   temp_ckv->get_key_type(),
                   reinterpret_cast<char *>(ctrlr_dom.ctrlr));
    return UPLL_RC_ERR_GENERIC;
  }

  if (ipc_response->header.result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Driver response for Key %d controller %s result %d",
                   temp_ckv->get_key_type(), ctrlr_dom.ctrlr,
                   ipc_response->header.result_code);
    return ipc_response->header.result_code;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtnPolicingMapMoMgr::ReadSiblingCount(IpcReqRespHeader *req,
    ConfigKeyVal* ikey,
    DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code;
  controller_domain ctrlr_dom;
  if (UNC_KT_VTN_POLICINGMAP == ikey->get_key_type()) {
    result_code = ValidateMessage(req, ikey);
    if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("ValidateMessage failed result_code %d",
                      result_code);
        return result_code;
    }
    result_code = ReadInfoFromDB(req, ikey, dmi, &ctrlr_dom);
    if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Read from DB failed result_code %d", result_code);
        return result_code;
    }
    return result_code;
  }
  ConfigKeyVal *temp_vtn_pm_ckv = NULL;
  ConfigKeyVal *vtn_pm_ckv = NULL;
  key_vtn_policingmap_controller_t *vtn_pm_ctrlr_key = reinterpret_cast
    <key_vtn_policingmap_controller_t *>(ikey->get_key());
  ctrlr_dom.ctrlr = reinterpret_cast <uint8_t*>
    (ConfigKeyVal::Malloc(kMaxLenCtrlrId + 1));
  ctrlr_dom.domain = reinterpret_cast <uint8_t*>
    (ConfigKeyVal::Malloc(kMaxLenDomainId + 1));
  uuu::upll_strncpy(ctrlr_dom.ctrlr,
      vtn_pm_ctrlr_key->controller_name,
      (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(ctrlr_dom.domain,
      vtn_pm_ctrlr_key->domain_id,
      (kMaxLenDomainId + 1));
  result_code = CopyVtnControllerCkv(ikey, temp_vtn_pm_ckv);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("CopyVtnControllerCkv failed");
    return result_code;
  }
  DbSubOp dbop = {kOpReadMultiple, kOpMatchNone,
    kOpInOutCtrlr | kOpInOutDomain};
  result_code = ReadConfigDB(temp_vtn_pm_ckv, req->datatype,
      UNC_OP_READ, dbop, dmi, CTRLRTBL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("ReadConfigDB failed %d", result_code);
    delete temp_vtn_pm_ckv;
    return result_code;
  }
  uint8_t sibling_count = 0;
  vtn_pm_ckv = temp_vtn_pm_ckv;
  while (vtn_pm_ckv !=NULL) {
    sibling_count++;
    vtn_pm_ckv = vtn_pm_ckv->get_next_cfg_key_val();
  }
  uint32_t *sib_count =
      reinterpret_cast<uint32_t*>(ConfigKeyVal::Malloc(sizeof(uint32_t)));
  *sib_count = sibling_count;
  ikey->SetCfgVal(new ConfigVal(IpctSt::kIpcStUint32, sib_count));
  return UPLL_RC_SUCCESS;
}
}  // kt_momgr
}  // upll
}  // unc
