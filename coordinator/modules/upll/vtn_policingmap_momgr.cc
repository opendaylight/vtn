/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include "momgr_impl.hh"
#include "vterm_if_momgr.hh"
#include "vtn_momgr.hh"
#include "policingprofile_momgr.hh"
#include "policingprofile_entry_momgr.hh"
#include "vtn_policingmap_momgr.hh"
#include "unc/upll_errno.h"
#include "upll_validation.hh"
#include "unc/upll_ipc_enum.h"
#include "uncxx/upll_log.hh"
#include "vbr_if_momgr.hh"

using unc::upll::ipc_util::IpcUtil;
namespace unc {
namespace upll {
namespace kt_momgr {
#define VTN_RENAME 0x01
#define POLICINGPROFILE_RENAME 0x02
#define NO_POLICINGPROFILE_RENAME 0xFD

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
  { uudst::vtn_policingmap::kDbiFlags, CK_VAL,
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
  { uudst::vtn_policingmap_ctrlr::kDbiFlags, CK_VAL,
    offsetof(key_user_data_t, flags),
    uud::kDalUint8, 1 }
};


// Constructor
VtnPolicingMapMoMgr::VtnPolicingMapMoMgr() : MoMgrImpl() {
  UPLL_FUNC_TRACE;
  // Rename operation is not support for this KT
  // setting max tables to 2
  ntable = MAX_MOMGR_TBLS;
  table = new Table *[ntable]();

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

  table[CONVERTTBL] = NULL;

  nchild = 0;
  child = NULL;
}

upll_rc_t VtnPolicingMapMoMgr::RestorePOMInCtrlTbl(
    ConfigKeyVal *ikey,
    upll_keytype_datatype_t dt_type,
    MoMgrTables tbl,
    DalDmlIntf* dmi) {

  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  if (!ikey || !(ikey->get_key())) {
    UPLL_LOG_DEBUG("Input Key Not Valid");
    return UPLL_RC_ERR_GENERIC;
  }
  if (tbl != CTRLRTBL ||
       (ikey->get_key_type() != UNC_KT_VTN_POLICINGMAP)) {
    UPLL_LOG_DEBUG("Ignoring  ktype/Table kt=%d, tbl=%d",
                   ikey->get_key_type(), tbl);
    return result_code;
  }

  val_policingmap_t *val_pm = reinterpret_cast<val_policingmap_t *>
    (GetVal(ikey));
  if (NULL == val_pm) {
    UPLL_LOG_DEBUG(" Value structure is NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  /*
  if (UNC_VF_VALID == val_pm->valid[UPLL_IDX_POLICERNAME_PM]) {
    result_code = UpdateRefCountInPPCtrlr(ikey, dt_type, dmi,
                                          UNC_OP_CREATE);
    if (UPLL_RC_SUCCESS != result_code) {
      if (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
        UPLL_LOG_DEBUG("Failed to update PolicingProfile in CtrlrTbl %d",
                       result_code);
        return result_code;
      }
    }
  }
  */
  return result_code;
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
  if (req->datatype != UPLL_DT_IMPORT) {
    // validate syntax and semantics
    result_code = ValidateMessage(req, ikey);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("ValidateMessage Err (%d)", result_code);
      return result_code;
    }
  }

  result_code = ValidateAttribute(ikey, dmi, req);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("ValidateAttribute failed, Error - %d", result_code);
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

  // Get mode id and vtn_name from tc
  TcConfigMode config_mode = TC_CONFIG_INVALID;
  std::string vtn_name = "";
  result_code = GetConfigModeInfo(req, config_mode, vtn_name);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetConfigMode failed");
    return result_code;
  }

  if (UNC_VF_VALID == val_pm->valid[UPLL_IDX_POLICERNAME_PM]) {
    result_code = IsPolicyProfileReferenced(ikey, req->datatype, dmi,
        UNC_OP_READ);
    if (UPLL_RC_SUCCESS != result_code) {
      if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
        UPLL_LOG_ERROR("IsPolicyProfileReferenced record not available (%d)",
            result_code);
        return UPLL_RC_ERR_CFG_SEMANTIC;
      } else {
        UPLL_LOG_DEBUG(
            "IsPolicyProfileReferenced error accessing DB (%d)",
            result_code);
        return result_code;
      }
    }
    //  check the policing profile existence in running DB also
    //  if config mode is VTN
    if (config_mode == TC_CONFIG_VTN) {
      result_code = IsPolicyProfileReferenced(ikey, UPLL_DT_RUNNING , dmi,
                                            UNC_OP_READ);
      if (UPLL_RC_SUCCESS != result_code) {
        if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
           UPLL_LOG_ERROR("IsPolicyProfileReferenced record not available (%d)",
                          result_code);
           return UPLL_RC_ERR_CFG_SEMANTIC;
        } else {
          UPLL_LOG_DEBUG("IsPolicyProfileReferenced error accessing DB (%d)",
                         result_code);
          return result_code;
        }
      }
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

  if (UNC_VF_VALID == val_pm->valid[UPLL_IDX_POLICERNAME_PM]) {
    result_code = UpdateRefCountInPPCtrlr(ikey, req->datatype, dmi,
        UNC_OP_CREATE, config_mode, vtn_name);
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
      config_mode, vtn_name, MAINTBL);
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
      dmi, config_mode, vtn_name);
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
  result_code = IsReferenced(req, ikey, dmi);
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
    UPLL_LOG_ERROR(" ReadConfigDB failed:%d", result_code);
    DELETE_IF_NOT_NULL(tempckv);
    return result_code;
  }
  TcConfigMode config_mode = TC_CONFIG_INVALID;
  std::string vtn_name = "";
  result_code = GetConfigModeInfo(req, config_mode, vtn_name);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetConfigMode failed");
    return result_code;
  }

  val_policingmap_t *val_pm = reinterpret_cast<val_policingmap_t *>
    (GetVal(tempckv));
  if (UNC_VF_VALID == val_pm->valid[UPLL_IDX_POLICERNAME_PM]) {
    result_code = UpdateRefCountInPPCtrlr(tempckv, req->datatype, dmi,
        UNC_OP_DELETE, config_mode, vtn_name);
    if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
      result_code = UPLL_RC_SUCCESS;
    } else if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("DeleteMo UpdateRefCountInPPCtrlr Error DB (%d)",
          result_code);
      DELETE_IF_NOT_NULL(tempckv);
      return result_code;
    }
  }
  // Delete a record in vtnpolicingmap CANDIDATE DB
  result_code = UpdateConfigDB(ikey, req->datatype, req->operation, dmi,
      config_mode, vtn_name, MAINTBL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_ERROR("Failed to detete the record from DB err %d",
                    result_code);
    DELETE_IF_NOT_NULL(tempckv);
    return result_code;
  }

  // Delete the record in vtnpolicingmapctrltbl
  result_code = UpdateRecordInVtnPmCtrlr(tempckv, req->datatype, UNC_OP_DELETE,
      dmi, config_mode, vtn_name);
  if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
    result_code = UPLL_RC_SUCCESS;
  } else if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("DeleteMo record Err in vtnpolicingmapctrltbl (%d)",
        result_code);
    DELETE_IF_NOT_NULL(tempckv);
    return result_code;
  }

  DELETE_IF_NOT_NULL(tempckv);
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


  // validate syntax and semantics
  result_code = ValidateMessage(req, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("UpdateMo ValidateMessage Err  (%d)", result_code);
    return result_code;
  }
  TcConfigMode config_mode = TC_CONFIG_INVALID;
  std::string vtn_name = "";
  result_code = GetConfigModeInfo(req, config_mode, vtn_name);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetConfigMode failed");
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
        UPLL_LOG_ERROR("IsPolicyProfileReferenced record not available (%d)",
            result_code);
        return UPLL_RC_ERR_CFG_SEMANTIC;
      } else {
        UPLL_LOG_DEBUG(
            "IsPolicyProfileReferenced error accessing DB (%d)",
            result_code);
        return result_code;
      }
    }
    // check the policing profile existence in running DB also
    if (config_mode == TC_CONFIG_VTN) {
      result_code = IsPolicyProfileReferenced(ikey, UPLL_DT_RUNNING, dmi,
          UNC_OP_READ);
      if (UPLL_RC_SUCCESS != result_code) {
        if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
          UPLL_LOG_ERROR("IsPolicyProfileReferenced record not available (%d)",
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
    DELETE_IF_NOT_NULL(tmpckv);
    UPLL_LOG_ERROR("ReadConfigDB failed, err %d", result_code);
    return result_code;
  }
  /*All invalid check*/
  if (IsAllAttrInvalid(
    reinterpret_cast<val_policingmap_t *>(GetVal(ikey)))) {
    UPLL_LOG_INFO("No attributes to be updated");
    DELETE_IF_NOT_NULL(tmpckv);
    return UPLL_RC_SUCCESS;
  }

  val_policingmap_t *val_tmp_val = reinterpret_cast<val_policingmap_t *>
    (GetVal(tmpckv));
  // If the incoming policingprofile name is same as the record
  // present in DB return UPLL_RC_SUCCESS
  if (!strncmp(reinterpret_cast<char *>(val_ival->policer_name),
              reinterpret_cast<char *>(val_tmp_val->policer_name),
              (kMaxLenPolicingProfileName + 1))) {
    UPLL_LOG_DEBUG("Update Request with same policingprofile name");
    DELETE_IF_NOT_NULL(tmpckv);
    return UPLL_RC_SUCCESS;
  }
  if (UNC_VF_VALID == val_ival->valid[UPLL_IDX_POLICERNAME_PM] &&
      UNC_VF_VALID == val_tmp_val->valid[UPLL_IDX_POLICERNAME_PM]) {
    UPLL_LOG_DEBUG(" Policer name valid in DB and ikey");
    result_code = UpdateRefCountInPPCtrlr(tmpckv, req->datatype, dmi,
        UNC_OP_DELETE, config_mode, vtn_name);
    if (UPLL_RC_SUCCESS != result_code) {
      if (UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
        DELETE_IF_NOT_NULL(tmpckv);
        UPLL_LOG_DEBUG("UpdateMo UpdateRefCountInPPCtrlr error in delete (%d)",
          result_code);
        return result_code;
      }
    }
    result_code = UpdateRefCountInPPCtrlr(ikey, req->datatype, dmi,
        UNC_OP_CREATE, config_mode, vtn_name);
    if (UPLL_RC_SUCCESS != result_code) {
      if (UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
        DELETE_IF_NOT_NULL(tmpckv);
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
        UNC_OP_CREATE, config_mode, vtn_name);
    if (UPLL_RC_SUCCESS != result_code) {
      if (UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
        DELETE_IF_NOT_NULL(tmpckv);
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
        UNC_OP_DELETE, config_mode, vtn_name);
    if (UPLL_RC_SUCCESS != result_code) {
      if (UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
        DELETE_IF_NOT_NULL(tmpckv);
        UPLL_LOG_DEBUG("UpdateMo UpdateRefCountInPPCtrlr error in delete (%d)",
            result_code);
        return result_code;
      }
    }
  }
  /* Update the record in CANDIDATE DB */
  uint8_t temp_flag = 0;
  GET_USER_DATA_FLAGS(ikey, temp_flag);
  UPLL_LOG_DEBUG("Flag in ikey: %d", temp_flag);
  DbSubOp dbop1 = {kOpNotRead, kOpMatchNone, kOpInOutFlag};
  result_code = UpdateConfigDB(ikey, req->datatype, req->operation, dmi,
      &dbop1, config_mode, vtn_name, MAINTBL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("Create record Err in vtnpolicingmaptbl CANDIDATE DB(%d)",
        result_code);
    DELETE_IF_NOT_NULL(tmpckv);
    return result_code;
  }

  /* Update the record in vtnpolicingmapctrltbl */
  result_code = UpdateRecordInVtnPmCtrlr(ikey, req->datatype, UNC_OP_UPDATE,
      dmi, config_mode, vtn_name);
  if (UPLL_RC_SUCCESS != result_code) {
    if (UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
      DELETE_IF_NOT_NULL(tmpckv);
      UPLL_LOG_DEBUG("Create record Err in vtnpolicingmaptbl CANDIDATE DB(%d)",
          result_code);
      return result_code;
    }
  }
  DELETE_IF_NOT_NULL(tmpckv);
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtnPolicingMapMoMgr::IsReferenced(IpcReqRespHeader *req,
    ConfigKeyVal *ikey,
    DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  if (NULL == ikey) return UPLL_RC_ERR_GENERIC;
  DbSubOp dbop = { kOpReadExist, kOpMatchNone, kOpInOutNone };

  // Check the exixtence in vtnpolicingmaptbl
  result_code = UpdateConfigDB(ikey, req->datatype, UNC_OP_READ, dmi,
                               &dbop, MAINTBL);
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
    UPLL_LOG_ERROR("UpdateConfigDB failed %d", result_code);
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
    <const char *>(val_pm->policer_name), dmi, dt_type);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_INFO("ValidateValidElements failed %d", result_code);
    DELETE_IF_NOT_NULL(okey);
    return result_code;
  }
  DELETE_IF_NOT_NULL(okey);
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtnPolicingMapMoMgr::UpdateRefCountInPPCtrlr(
    ConfigKeyVal *ikey, upll_keytype_datatype_t dt_type, DalDmlIntf *dmi,
    unc_keytype_operation_t op, TcConfigMode config_mode,
    string vtn_name) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  ConfigKeyVal *okey = NULL;
  uint8_t *ctrlr_id = NULL;
  uint8_t pp_flag = 0, pp_flag1 = 0;
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
  result_code = vtnmgr->GetControllerDomainSpan(okey, dt_type, dmi);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_INFO("GetControllerSpan, err %d", result_code);
    DELETE_IF_NOT_NULL(okey);
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

  if (UPLL_DT_IMPORT == dt_type) {
    UPLL_LOG_DEBUG("##### op1op1");
    GET_USER_DATA_FLAGS(ikey, pp_flag);
    if (pp_flag & POLICINGPROFILE_RENAME)
       pp_flag1 = PP_RENAME;
    UPLL_LOG_DEBUG("##### op1op1 flag %d", pp_flag);
    UPLL_LOG_DEBUG("##### op1op1 flag1 %d", pp_flag1);
  }
  // Vtn Associated ctrl name
  ConfigKeyVal *tmp_ckv = okey;
  while (NULL != okey) {
    // check for vnode_ref_cnt(vnodes count) in vtn_ctrlr_tbl val structure.
    // If the count is '0' then continue for next vtn_ctrlr_tbl.
    val_vtn_ctrlr *ctr_val =
        reinterpret_cast<val_vtn_ctrlr *>(GetVal(okey));
    if (!ctr_val) {
      UPLL_LOG_ERROR("Vtn controller table val structure is NULL");
      DELETE_IF_NOT_NULL(okey);
      return UPLL_RC_ERR_GENERIC;
    }
    if (ctr_val->vnode_ref_cnt <= 0) {
      UPLL_LOG_DEBUG("skipping entry");
      okey = okey->get_next_cfg_key_val();
      continue;
    }
    // Get the controller name
    GET_USER_DATA_CTRLR(okey, ctrlr_id);

    if (NULL == ctrlr_id) {
      UPLL_LOG_ERROR("UpdateRefCountInPPCtrlr ctrlr_id NULL");
      DELETE_IF_NOT_NULL(okey);
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
    UPLL_LOG_DEBUG("Ctrlrid in GetPolicingProfileCtrlrKeyval %s, %d",
        ctrlr_id, pp_flag);

    std::string temp_vtn_name;
    if (TC_CONFIG_VTN == config_mode) {
      temp_vtn_name = vtn_name;
    } else {
      temp_vtn_name = reinterpret_cast<const char *>(reinterpret_cast
                                                     <key_vtn_t *>
                                                     (ikey->get_key())->
                                                     vtn_name);
    }
    result_code = pp_mgr->PolicingProfileCtrlrTblOper(
        reinterpret_cast<const char *>(val_vtn_policingmap->policer_name),
        reinterpret_cast<const char *>(ctrlr_id), dmi, op, dt_type, pp_flag1,
        config_mode, temp_vtn_name, 1, false);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_INFO("Refcnt update in PP ctrlrtbl err %d", result_code);
      DELETE_IF_NOT_NULL(okey);
      return result_code;
    }
    // Create the record in policingprofileentryctrltbl
    #if 0
    result_code = ppe_mgr->PolicingProfileEntryCtrlrTblOper(
        reinterpret_cast<const char *>(val_vtn_policingmap->policer_name),
        reinterpret_cast<const char *>(ctrlr_id), dmi, op, dt_type);
    if (UPLL_RC_SUCCESS != result_code) {
      DELETE_IF_NOT_NULL(okey);
      return result_code;
    }
    #endif
    okey = okey->get_next_cfg_key_val();
  }
  DELETE_IF_NOT_NULL(tmp_ckv);
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtnPolicingMapMoMgr::UpdateRecordInVtnPmCtrlr(
    ConfigKeyVal *ikey, upll_keytype_datatype_t dt_type,
    unc_keytype_operation_t op, DalDmlIntf *dmi,
    TcConfigMode config_mode, string vtn_name) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *ctrlckv = NULL, *temp_okey = NULL;
  key_vtn_t *vtn_key = reinterpret_cast<key_vtn_t *>(ikey->get_key());
  if (NULL == vtn_key) {
    UPLL_LOG_DEBUG("key is NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  VtnMoMgr *vtnmgr =
    static_cast<VtnMoMgr *>((const_cast<MoManager *>
          (GetMoManager(UNC_KT_VTN))));
  // Get the memory allocated vtn key structure
  result_code = vtnmgr->GetChildConfigKey(temp_okey, NULL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG(" GetChildConfigKey error (%d)",
        result_code);
    return result_code;
  }
  key_vtn_t *vtn_okey = reinterpret_cast<key_vtn_t *>(temp_okey->get_key());
  if (NULL == vtn_okey) {
    UPLL_LOG_DEBUG("key is NULL");
    DELETE_IF_NOT_NULL(temp_okey);
    return UPLL_RC_ERR_GENERIC;
  }
  uuu::upll_strncpy(vtn_okey->vtn_name, vtn_key->vtn_name,
      kMaxLenVtnName+1);
  UPLL_LOG_DEBUG(" vtn_name %s", vtn_okey->vtn_name);
  // Get the vtn associated controller name
  result_code = vtnmgr->GetControllerDomainSpan(temp_okey, dt_type, dmi);
  if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    UPLL_LOG_DEBUG(" No entry in vtn ctrlr tbl");
    DELETE_IF_NOT_NULL(temp_okey);
    return UPLL_RC_SUCCESS;
  }
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_ERROR("GetControllerSpan error (%d)",
        result_code);
    DELETE_IF_NOT_NULL(temp_okey);
    return result_code;
  }
  ConfigKeyVal *okey = temp_okey;
  while (NULL != okey) {
    // check for vnode_ref_cnt(vnodes count) in vtn_ctrlr_tbl val structure.
    // If the count is '0' then continue for next vtn_ctrlr_tbl.
    val_vtn_ctrlr *ctr_val =
        reinterpret_cast<val_vtn_ctrlr *>(GetVal(okey));
    if (!ctr_val) {
      UPLL_LOG_ERROR("Vtn controller table val structure is NULL");
      DELETE_IF_NOT_NULL(temp_okey);
      return UPLL_RC_ERR_GENERIC;
    }
    if (ctr_val->vnode_ref_cnt <= 0) {
      UPLL_LOG_DEBUG("skipping entry");
      okey = okey->get_next_cfg_key_val();
      continue;
    }
    controller_domain ctrlr_dom;
    ctrlr_dom.ctrlr = NULL;
    ctrlr_dom.domain = NULL;
    GET_USER_DATA_CTRLR_DOMAIN(okey, ctrlr_dom);
    result_code = GetPMCtrlKeyval(ctrlckv, ikey, &ctrlr_dom);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("CheckRefCountCapability GetPMCtrlKeyval error (%d)",
          result_code);
      DELETE_IF_NOT_NULL(temp_okey);
      return result_code;
    }
    if ((op == UNC_OP_CREATE) || (op ==UNC_OP_UPDATE)) {
      IpcReqRespHeader *temp_req = reinterpret_cast<IpcReqRespHeader *>
                 (ConfigKeyVal::Malloc(sizeof(IpcReqRespHeader)));

      temp_req->datatype = dt_type;
      temp_req->operation = op;

      result_code = ValidateCapability(
        temp_req, ikey, reinterpret_cast<char *>(ctrlr_dom.ctrlr));

      free(temp_req);

      unc_keytype_ctrtype_t ctrlrtype = UNC_CT_UNKNOWN;
      uuc::CtrlrMgr *ctrlr_mgr = uuc::CtrlrMgr::GetInstance();
      if (result_code != UPLL_RC_SUCCESS) {
         DELETE_IF_NOT_NULL(ctrlckv);
         // VTN PolicingMap is not supported for other than PFC Controller
         // so skip adding entry for such sontroller in ctrlr table
         if ((!ctrlr_mgr->GetCtrlrType(
                     reinterpret_cast<char *>(ctrlr_dom.ctrlr),
                     dt_type, &ctrlrtype)) || (ctrlrtype != UNC_CT_PFC)) {
            result_code = UPLL_RC_SUCCESS;
            UPLL_LOG_DEBUG("Controller type is  %d", ctrlrtype);
            okey = okey->get_next_cfg_key_val();
            continue;
         }
          DELETE_IF_NOT_NULL(okey);
          UPLL_LOG_DEBUG("Key not supported by controller");
          return result_code;
      }
      val_policingmap_t *val_policingmap =
        reinterpret_cast<val_policingmap_t*>(GetVal(ikey));
     val_vtnpolicingmap_ctrl *val_vtnpmap_ctrl = static_cast<
              val_vtnpolicingmap_ctrl*>(GetVal(ctrlckv));
      for ( unsigned int loop = 0;
              loop < sizeof
              (val_vtnpmap_ctrl->valid)/sizeof(val_vtnpmap_ctrl->valid[0]);
              ++loop ) {
         if (val_policingmap->valid[loop] == UNC_VF_NOT_SUPPORTED)
           val_vtnpmap_ctrl->valid[loop] = UNC_VF_INVALID;
         else
           val_vtnpmap_ctrl->valid[loop] =
                val_policingmap->valid[loop];
      }
    }

    uint8_t flag = 0;
    GET_USER_DATA_FLAGS(ikey, flag);
    SET_USER_DATA_FLAGS(ctrlckv, flag);

    // Create/Update/Delete a record in CANDIDATE DB
    result_code = UpdateConfigDB(ctrlckv, dt_type, op, dmi, config_mode,
                                 vtn_name, CTRLRTBL);
    if (UPLL_RC_SUCCESS != result_code) {
      if ((UNC_OP_CREATE == op) &&
          (UPLL_RC_ERR_INSTANCE_EXISTS == result_code)) {
        DELETE_IF_NOT_NULL(ctrlckv);
        okey = okey->get_next_cfg_key_val();
        continue;
      } else if ((UNC_OP_DELETE == op) &&
                 (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code)) {
        DELETE_IF_NOT_NULL(ctrlckv);
        okey = okey->get_next_cfg_key_val();
        continue;
      }
      DELETE_IF_NOT_NULL(temp_okey);
      DELETE_IF_NOT_NULL(ctrlckv);
      UPLL_LOG_DEBUG("Create record Err in vtnpolicingmapctrlrtbl CAN DB(%d)",
          result_code);
      return result_code;
    }
    DELETE_IF_NOT_NULL(ctrlckv);
    okey = okey->get_next_cfg_key_val();
  }
  DELETE_IF_NOT_NULL(temp_okey);
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtnPolicingMapMoMgr::UpdateControllerTableForVtn(
    uint8_t* vtn_name,
    controller_domain *ctrlr_dom,
    unc_keytype_operation_t op,
    upll_keytype_datatype_t dt_type,
    DalDmlIntf *dmi,
    uint8_t flag,
    TcConfigMode config_mode) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  UPLL_LOG_DEBUG("UpdateControllerTableForVtn dt_type (%d)", dt_type);
  ConfigKeyVal *ctrlckv = NULL, *ikey = NULL;
  uint8_t pp_flag = 0;
  result_code = GetChildConfigKey(ikey, NULL);
  if (result_code != UPLL_RC_SUCCESS) {
    return result_code;
  }
  if (!ikey) return UPLL_RC_ERR_GENERIC;
  unc_keytype_ctrtype_t ctrlrtype = UNC_CT_UNKNOWN;
  uuc::CtrlrMgr *ctrlr_mgr = uuc::CtrlrMgr::GetInstance();
  key_vtn_t *vtn_key = reinterpret_cast<key_vtn_t *>
      (ikey->get_key());
  uuu::upll_strncpy(vtn_key->vtn_name, vtn_name, kMaxLenVtnName+1);
  DbSubOp dbop = {kOpReadSingle, kOpMatchNone, kOpInOutCs | kOpInOutFlag};
  // Read the Configuration from the MainTable
  result_code = ReadConfigDB(ikey, dt_type,
      UNC_OP_READ, dbop, dmi, MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      UPLL_LOG_DEBUG("No Records in main table to be created in ctrlr tbl");
      DELETE_IF_NOT_NULL(ikey);
      return UPLL_RC_SUCCESS;
    }
    delete ikey;
    return result_code;
  }
  string vtnname(reinterpret_cast<const char *>(vtn_name));
  if (flag != 0) {
    uint8_t temp_flag = 0;
    GET_USER_DATA_FLAGS(ikey, temp_flag);
    flag = flag | temp_flag;
    SET_USER_DATA_FLAGS(ikey, flag);
    DbSubOp dbop1 = {kOpNotRead, kOpMatchNone, kOpInOutCs | kOpInOutFlag};
    result_code = UpdateConfigDB(ikey, dt_type, UNC_OP_UPDATE,
        dmi, &dbop1, config_mode, vtnname, MAINTBL);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("UpdateConfigDB failed %d", result_code);
      return result_code;
    }
  }
  if (ikey != NULL) {
    result_code = GetPMCtrlKeyval(ctrlckv, ikey, ctrlr_dom);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_INFO("GetPMCtrlKeyval error (%d)", result_code);
      DELETE_IF_NOT_NULL(ikey);
      return UPLL_RC_ERR_GENERIC;
    }
    if (ctrlckv == NULL) {
      UPLL_LOG_DEBUG("ctrlckv NULL");
      DELETE_IF_NOT_NULL(ikey);
      return UPLL_RC_ERR_GENERIC;
    }

    if (op == UNC_OP_CREATE) {
      IpcReqRespHeader *temp_req = reinterpret_cast<IpcReqRespHeader *>
                (ConfigKeyVal::Malloc(sizeof(IpcReqRespHeader)));
      temp_req->datatype = UPLL_DT_CANDIDATE;
      temp_req->operation = op;

      result_code = ValidateCapability(
          temp_req, ikey,
          reinterpret_cast<char *>(ctrlr_dom->ctrlr));

      free(temp_req);

      if (result_code != UPLL_RC_SUCCESS) {
         DELETE_IF_NOT_NULL(ctrlckv);
         DELETE_IF_NOT_NULL(ikey);
         // VTN PolicingMap is not supported for other than PFC Controller
         // so skip adding entry for such sontroller in ctrlr table
         if ((!ctrlr_mgr->GetCtrlrType(
                     reinterpret_cast<char *>(ctrlr_dom->ctrlr),
                     dt_type, &ctrlrtype)) || (ctrlrtype != UNC_CT_PFC)) {
            result_code = UPLL_RC_SUCCESS;
            UPLL_LOG_DEBUG("Controller type is  %d", ctrlrtype);
            return result_code;
         }
         UPLL_LOG_DEBUG("Key not supported by controller");
         return result_code;
      }

      val_policingmap_t *val_policingmap = reinterpret_cast<val_policingmap_t*>
          (GetVal(ikey));

      val_vtnpolicingmap_ctrl *val_vtnpmap_ctrl =
          reinterpret_cast<val_vtnpolicingmap_ctrl*>
          (GetVal(ctrlckv));
      for (unsigned int loop = 0; loop < sizeof
           (val_vtnpmap_ctrl->valid)/sizeof(val_vtnpmap_ctrl->valid[0]);
           ++loop ) {
         if (val_policingmap->valid[loop] == UNC_VF_NOT_SUPPORTED)
           val_vtnpmap_ctrl->valid[loop] = UNC_VF_INVALID;
         else
           val_vtnpmap_ctrl->valid[loop] =
                  val_policingmap->valid[loop];
      }
    }
    // Create the entry in VTNPMCtrl table
    if (UPLL_DT_AUDIT == dt_type && UNC_OP_CREATE == op) {
      val_vtnpolicingmap_ctrl_t *valvtnctrlr = reinterpret_cast
        <val_vtnpolicingmap_ctrl_t *>(GetVal(ctrlckv));
      val_policingmap_t *valvtn = reinterpret_cast
        <val_policingmap_t *>(GetVal(ikey));
      valvtnctrlr->cs_attr[0] = valvtn->cs_attr[0];
      valvtnctrlr->cs_row_status = valvtn->cs_row_status;
      UPLL_LOG_DEBUG("Setting cs for ctrlr tbl in vtnpm cs %d row %d",
                     valvtnctrlr->cs_attr[0], valvtnctrlr->cs_row_status);
      UPLL_LOG_DEBUG("Setting cs for main tbl in vtnpm cs %d row %d",
                     valvtn->cs_attr[0], valvtn->cs_row_status);
    }
    result_code = UpdateConfigDB(ctrlckv, dt_type, op,
        dmi, config_mode, vtnname, CTRLRTBL);
    if (UPLL_RC_SUCCESS != result_code) {
      DELETE_IF_NOT_NULL(ctrlckv);
      DELETE_IF_NOT_NULL(ikey);
      UPLL_LOG_ERROR("Err while updating in ctrlr table for candidateDb(%d)",
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
        dmi, op, dt_type, pp_flag, config_mode, vtnname, 1, false);
    if (UPLL_RC_SUCCESS != result_code) {
       DELETE_IF_NOT_NULL(ctrlckv);
       DELETE_IF_NOT_NULL(ikey);
       return result_code;
    }

    DELETE_IF_NOT_NULL(ikey);
    DELETE_IF_NOT_NULL(ctrlckv);
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
    if (UNC_VF_VALID == valvtnctrlr->valid[UPLL_IDX_POLICERNAME_PM]) {
      uuu::upll_strncpy(
          reinterpret_cast<char *>(valvtnctrlr->policer_name),
          reinterpret_cast<const char*>(reinterpret_cast<val_policingmap_t*>
            (ikey->get_cfg_val()->get_val())->policer_name),
          kMaxLenPolicingProfileName + 1);
    }
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
    const char* policingprofile_name, upll_keytype_datatype_t dt_type,
    DalDmlIntf *dmi) {
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
  result_code = ReadConfigDB(ckv, dt_type, UNC_OP_READ, dbop,
              dmi, MAINTBL);
  DELETE_IF_NOT_NULL(ckv);
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
            DELETE_IF_NOT_NULL(ipc_response.ckv_data);
            return result_code;
          }
          ConfigKeyVal *temp_key = NULL;
          result_code = CopyVtnControllerCkv(ikey, temp_key);
          if (UPLL_RC_SUCCESS != result_code) {
            UPLL_LOG_DEBUG("CopyVtnControllerCkv failed %d", result_code);
            delete temp_key;
            DELETE_IF_NOT_NULL(ipc_response.ckv_data);
            return result_code;
          }

          DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutNone };
          result_code = ReadConfigDB(temp_key, req->datatype, UNC_OP_READ, dbop,
              dmi, MAINTBL);
          if (UPLL_RC_SUCCESS != result_code) {
            UPLL_LOG_DEBUG("ReadConfigDB failed %d", result_code);
            delete temp_key;
            DELETE_IF_NOT_NULL(ipc_response.ckv_data);
            return result_code;
          }

          result_code = ReadControllerStateDetail(ikey, temp_key,
                                                  &ipc_response, req->datatype,
                                                  req->operation, dmi);
          DELETE_IF_NOT_NULL(ipc_response.ckv_data);
          DELETE_IF_NOT_NULL(temp_key);
          if (result_code != UPLL_RC_SUCCESS) {
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
    UPLL_LOG_DEBUG("ReadSiblingMo ValidateReadAttribute Err  (%d)",
                   result_code);
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
    FREE_IF_NOT_NULL(ctrlr_dom.ctrlr);
    FREE_IF_NOT_NULL(ctrlr_dom.domain);
    return result_code;
  }
  DbSubOp dbop = {kOpReadMultiple, kOpMatchNone,
    kOpInOutCtrlr | kOpInOutDomain};
  result_code = ReadConfigDB(temp_vtn_pm_ckv, req->datatype,
                             UNC_OP_READ, dbop, req->rep_count, dmi, CTRLRTBL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("ReadConfigDB failed %d", result_code);
    DELETE_IF_NOT_NULL(temp_vtn_pm_ckv);
    FREE_IF_NOT_NULL(ctrlr_dom.ctrlr);
    FREE_IF_NOT_NULL(ctrlr_dom.domain);
    return result_code;
  }
  vtn_pm_ckv = temp_vtn_pm_ckv;
  ConfigKeyVal *okey = NULL;
  ConfigKeyVal *dup_ckv = NULL;
  uint32_t tmp_sib_count = 0;

  if (req->operation == UNC_OP_READ_SIBLING) {
    while (NULL != vtn_pm_ckv) {
      controller_domain temp_cd;
      GET_USER_DATA_CTRLR_DOMAIN(vtn_pm_ckv, temp_cd);
      ctrl_len = dom_len = 0;
      ctrl_len =  strcmp((const char*)(ctrlr_dom.ctrlr),
                         (const char*)(temp_cd.ctrlr));
      dom_len =  strcmp((const char*)(ctrlr_dom.domain),
                        (const char*)(temp_cd.domain));

      if ((ctrl_len < 0) || ((ctrl_len == 0) && (dom_len < 0))) {
        result_code = GetChildControllerConfigKey(dup_ckv, vtn_pm_ckv,
                                                  &temp_cd);
        if (UPLL_RC_SUCCESS != result_code) {
          UPLL_LOG_DEBUG("GetChildControllerConfigKey failed");
          DELETE_IF_NOT_NULL(temp_vtn_pm_ckv);
          DELETE_IF_NOT_NULL(okey);
          FREE_IF_NOT_NULL(ctrlr_dom.ctrlr);
          FREE_IF_NOT_NULL(ctrlr_dom.domain);
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
          DELETE_IF_NOT_NULL(dup_ckv);
          DELETE_IF_NOT_NULL(temp_vtn_pm_ckv);
          DELETE_IF_NOT_NULL(okey);
          FREE_IF_NOT_NULL(ctrlr_dom.ctrlr);
          FREE_IF_NOT_NULL(ctrlr_dom.domain);
          return result_code;
        }
        if (NULL == okey) {
          okey = dup_ckv;
        } else {
          okey->AppendCfgKeyVal(dup_ckv);
        }
        tmp_sib_count++;
        if (tmp_sib_count == req->rep_count)
          break;
      } else {
        result_code = UPLL_RC_ERR_NO_SUCH_INSTANCE;
      }
      vtn_pm_ckv = vtn_pm_ckv->get_next_cfg_key_val();
    }
    if ((okey != NULL) && (result_code == UPLL_RC_SUCCESS)) {
      ikey->ResetWith(okey);
      req->rep_count = tmp_sib_count;
      DELETE_IF_NOT_NULL(okey);
    } else {
      UPLL_LOG_DEBUG("ReadSiblingControllerStateNormal failed %d",
                      result_code);
      DELETE_IF_NOT_NULL(okey);
    }
  } else if (req->operation == UNC_OP_READ_SIBLING_BEGIN) {
    while (NULL != vtn_pm_ckv) {
      controller_domain temp_cd;
      GET_USER_DATA_CTRLR_DOMAIN(vtn_pm_ckv, temp_cd);
      result_code = GetChildControllerConfigKey(dup_ckv, vtn_pm_ckv,
                                                &temp_cd);
      if (UPLL_RC_SUCCESS != result_code) {
        UPLL_LOG_DEBUG("GetChildControllerConfigKey failed");
        DELETE_IF_NOT_NULL(temp_vtn_pm_ckv);
        DELETE_IF_NOT_NULL(okey);
        DELETE_IF_NOT_NULL(dup_ckv);
        FREE_IF_NOT_NULL(ctrlr_dom.ctrlr);
        FREE_IF_NOT_NULL(ctrlr_dom.domain);
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
        DELETE_IF_NOT_NULL(dup_ckv);
        DELETE_IF_NOT_NULL(temp_vtn_pm_ckv);
        DELETE_IF_NOT_NULL(okey);
        FREE_IF_NOT_NULL(ctrlr_dom.ctrlr);
        FREE_IF_NOT_NULL(ctrlr_dom.domain);
        return result_code;
      }
      if (NULL == okey) {
        okey = dup_ckv;
      } else {
        okey->AppendCfgKeyVal(dup_ckv);
      }
      tmp_sib_count++;
      if (tmp_sib_count == req->rep_count)
        break;
      vtn_pm_ckv = vtn_pm_ckv->get_next_cfg_key_val();
    }
    if ((okey != NULL) && (result_code == UPLL_RC_SUCCESS)) {
      ikey->ResetWith(okey);
      req->rep_count = tmp_sib_count;
      DELETE_IF_NOT_NULL(okey);
    }
  } else {
    result_code = UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT;
  }
  DELETE_IF_NOT_NULL(temp_vtn_pm_ckv);
  FREE_IF_NOT_NULL(ctrlr_dom.ctrlr);
  FREE_IF_NOT_NULL(ctrlr_dom.domain);
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
  ConfigKeyVal *dup_ckv = NULL;
  uint32_t tmp_sib_count = 0;
  if (req->operation == UNC_OP_READ_SIBLING) {
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

      if ((ctrl_len < 0) || ((ctrl_len == 0) && (dom_len < 0))) {
        result_code = GetChildControllerConfigKey(dup_ckv, vtn_pm_ckv,
                                                  &temp_cd);
        if (UPLL_RC_SUCCESS != result_code) {
          UPLL_LOG_DEBUG("GetChildControllerConfigKey failed");
          FREE_IF_NOT_NULL(ctrlr_dom.ctrlr);
          FREE_IF_NOT_NULL(ctrlr_dom.domain);
          DELETE_IF_NOT_NULL(okey);
          return result_code;
        }
        val_policingmap_controller_t *out_val_pm_ctrlr =
            reinterpret_cast<val_policingmap_controller_t *>
            (ConfigKeyVal::Malloc(sizeof(val_policingmap_controller_t)));
        memcpy(out_val_pm_ctrlr, val_pm_ctrlr,
               sizeof(val_policingmap_controller_t));
        dup_ckv->AppendCfgVal(IpctSt::kIpcStValPolicingmapController,
                              out_val_pm_ctrlr);
        UPLL_LOG_TRACE("%s DUP_CKV ", dup_ckv->ToStrAll().c_str());
        IpcResponse ipc_response;
        memset(&ipc_response, 0, sizeof(IpcResponse));
        result_code = SendIpcrequestToDriver(
            dup_ckv, req, &ipc_response, dmi);
        if (UPLL_RC_ERR_CTR_DISCONNECTED == ipc_response.header.result_code) {
          result_code = UPLL_RC_ERR_NO_SUCH_INSTANCE;
          DELETE_IF_NOT_NULL(ipc_response.ckv_data);
          DELETE_IF_NOT_NULL(dup_ckv);
          // move to next controller
          while ((vtn_pm_ckv = vtn_pm_ckv->get_next_cfg_key_val())) {
            controller_domain temp_cd2;
            temp_cd2.ctrlr = NULL;
            temp_cd2.domain = NULL;
            GET_USER_DATA_CTRLR_DOMAIN(vtn_pm_ckv, temp_cd2);
            if (strcmp(reinterpret_cast<const char *>(temp_cd.ctrlr),
                        reinterpret_cast<const char *>(temp_cd2.ctrlr)) == 0) {
              UPLL_LOG_TRACE("Controller %s is disconnected", temp_cd2.ctrlr);
              continue;
            } else {
              break;
            }
          }
          continue;
        } else if (UPLL_RC_ERR_NO_SUCH_INSTANCE ==
                   ipc_response.header.result_code) {
          UPLL_LOG_DEBUG("Record not found for domain %s",
                         (const char *)temp_cd.domain);
          DELETE_IF_NOT_NULL(ipc_response.ckv_data);
          DELETE_IF_NOT_NULL(dup_ckv);
          // move to next domain or next controller
          vtn_pm_ckv = vtn_pm_ckv->get_next_cfg_key_val();
          continue;
        } else if (UPLL_RC_SUCCESS != result_code) {
          UPLL_LOG_DEBUG("SendIpcrequestToDriver failed %d", result_code);
          DELETE_IF_NOT_NULL(ipc_response.ckv_data);
          FREE_IF_NOT_NULL(ctrlr_dom.ctrlr);
          FREE_IF_NOT_NULL(ctrlr_dom.domain);
          DELETE_IF_NOT_NULL(dup_ckv);
          DELETE_IF_NOT_NULL(okey);
          return result_code;
        }
        ConfigKeyVal *temp_key = NULL;
        result_code = CopyVtnControllerCkv(dup_ckv, temp_key);
        if (UPLL_RC_SUCCESS != result_code) {
          UPLL_LOG_DEBUG("CopyVtnControllerCkv failed %d", result_code);
          FREE_IF_NOT_NULL(ctrlr_dom.ctrlr);
          FREE_IF_NOT_NULL(ctrlr_dom.domain);
          DELETE_IF_NOT_NULL(dup_ckv);
          DELETE_IF_NOT_NULL(temp_vtn_pm_ckv);
          DELETE_IF_NOT_NULL(ipc_response.ckv_data);
          DELETE_IF_NOT_NULL(okey);
          return result_code;
        }
        result_code = ReadConfigDB(temp_key, req->datatype, UNC_OP_READ,
                                   dbop, dmi, MAINTBL);
        if (UPLL_RC_SUCCESS != result_code) {
          UPLL_LOG_DEBUG("ReadConfigDB failed %d", result_code);
          FREE_IF_NOT_NULL(ctrlr_dom.ctrlr);
          FREE_IF_NOT_NULL(ctrlr_dom.domain);
          DELETE_IF_NOT_NULL(dup_ckv);
          DELETE_IF_NOT_NULL(temp_vtn_pm_ckv);
          DELETE_IF_NOT_NULL(temp_key);
          DELETE_IF_NOT_NULL(okey);
          DELETE_IF_NOT_NULL(ipc_response.ckv_data);
          return result_code;
        }
        result_code = ReadControllerStateDetail(dup_ckv, temp_key,
                                                &ipc_response, req->datatype,
                                                req->operation, dmi);

        DELETE_IF_NOT_NULL(ipc_response.ckv_data);
        if (result_code != UPLL_RC_SUCCESS) {
          FREE_IF_NOT_NULL(ctrlr_dom.ctrlr);
          FREE_IF_NOT_NULL(ctrlr_dom.domain);
          DELETE_IF_NOT_NULL(dup_ckv);
          DELETE_IF_NOT_NULL(temp_key);
          DELETE_IF_NOT_NULL(temp_vtn_pm_ckv);
          DELETE_IF_NOT_NULL(okey);
          return result_code;
        }
        DELETE_IF_NOT_NULL(temp_key);
        if (NULL == okey) {
          okey = dup_ckv;
        } else {
          okey->AppendCfgKeyVal(dup_ckv);
        }
        tmp_sib_count++;
        if (tmp_sib_count == req->rep_count)
          break;
      } else {
        result_code = UPLL_RC_ERR_NO_SUCH_INSTANCE;
      }
      vtn_pm_ckv = vtn_pm_ckv->get_next_cfg_key_val();
    }
    FREE_IF_NOT_NULL(ctrlr_dom.ctrlr);
    FREE_IF_NOT_NULL(ctrlr_dom.domain);
    if ((okey != NULL) && ((result_code == UPLL_RC_SUCCESS)
                           || (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE))) {
      result_code = UPLL_RC_SUCCESS;
      ikey->ResetWith(okey);
      DELETE_IF_NOT_NULL(okey);
      req->rep_count = tmp_sib_count;
    } else {
      DELETE_IF_NOT_NULL(okey);
    }
  } else  if (req->operation == UNC_OP_READ_SIBLING_BEGIN) {
    while (NULL != vtn_pm_ckv) {
      UPLL_LOG_DEBUG("KT of vtn_pm_ckv in while %d",
                     vtn_pm_ckv->get_key_type());
      controller_domain temp_cd;
      GET_USER_DATA_CTRLR_DOMAIN(vtn_pm_ckv, temp_cd);

      result_code = GetChildControllerConfigKey(dup_ckv, vtn_pm_ckv,
                                                &temp_cd);
      if (UPLL_RC_SUCCESS != result_code) {
        UPLL_LOG_DEBUG("GetChildControllerConfigKey failed");
        DELETE_IF_NOT_NULL(okey);
        return result_code;
      }
      val_policingmap_controller_t *out_val_pm_ctrlr =
          reinterpret_cast<val_policingmap_controller_t *>
          (ConfigKeyVal::Malloc(sizeof(val_policingmap_controller_t)));
      memcpy(out_val_pm_ctrlr, val_pm_ctrlr,
             sizeof(val_policingmap_controller_t));
      dup_ckv->AppendCfgVal(IpctSt::kIpcStValPolicingmapController,
                            out_val_pm_ctrlr);
      UPLL_LOG_TRACE("%s DUP_CKV ", dup_ckv->ToStrAll().c_str());
      IpcResponse ipc_response;
      memset(&ipc_response, 0, sizeof(IpcResponse));
      result_code = SendIpcrequestToDriver(
          dup_ckv, req, &ipc_response, dmi);
      if (UPLL_RC_ERR_CTR_DISCONNECTED == ipc_response.header.result_code) {
        result_code = UPLL_RC_ERR_NO_SUCH_INSTANCE;
        DELETE_IF_NOT_NULL(ipc_response.ckv_data);
        DELETE_IF_NOT_NULL(dup_ckv);
        // move to next controller
        while ((vtn_pm_ckv = vtn_pm_ckv->get_next_cfg_key_val())) {
          controller_domain temp_cd2;
          temp_cd2.ctrlr = NULL;
          temp_cd2.domain = NULL;
          GET_USER_DATA_CTRLR_DOMAIN(vtn_pm_ckv, temp_cd2);
          if (strcmp(reinterpret_cast<const char *>(temp_cd.ctrlr),
                      reinterpret_cast<const char *>(temp_cd2.ctrlr)) == 0) {
            UPLL_LOG_TRACE("Controller %s is disconnected", temp_cd2.ctrlr);
            continue;
          } else {
            break;
          }
        }
        continue;
      } else if (UPLL_RC_ERR_NO_SUCH_INSTANCE ==
                 ipc_response.header.result_code) {
        UPLL_LOG_DEBUG("Record not found for domain %s",
                       (const char *)temp_cd.domain);
        DELETE_IF_NOT_NULL(ipc_response.ckv_data);
        DELETE_IF_NOT_NULL(dup_ckv);
        // move to next domain or next controller
        vtn_pm_ckv = vtn_pm_ckv->get_next_cfg_key_val();
        continue;
      } else if (UPLL_RC_SUCCESS != result_code) {
        UPLL_LOG_DEBUG("SendIpcrequestToDriver failed %d", result_code);
        DELETE_IF_NOT_NULL(dup_ckv);
        DELETE_IF_NOT_NULL(temp_vtn_pm_ckv);
        DELETE_IF_NOT_NULL(ipc_response.ckv_data);
        DELETE_IF_NOT_NULL(okey);
        return result_code;
      }
      ConfigKeyVal *temp_key = NULL;
      result_code = CopyVtnControllerCkv(dup_ckv, temp_key);
      if (UPLL_RC_SUCCESS != result_code) {
        UPLL_LOG_DEBUG("CopyVtnControllerCkv failed %d", result_code);
        DELETE_IF_NOT_NULL(dup_ckv);
        DELETE_IF_NOT_NULL(temp_vtn_pm_ckv);
        DELETE_IF_NOT_NULL(ipc_response.ckv_data);
        DELETE_IF_NOT_NULL(okey);
        return result_code;
      }
      result_code = ReadConfigDB(temp_key, req->datatype, UNC_OP_READ,
                                 dbop, dmi, MAINTBL);
      if (UPLL_RC_SUCCESS != result_code) {
        UPLL_LOG_DEBUG("ReadConfigDB failed %d", result_code);
        DELETE_IF_NOT_NULL(dup_ckv);
        DELETE_IF_NOT_NULL(temp_key);
        DELETE_IF_NOT_NULL(temp_vtn_pm_ckv);
        DELETE_IF_NOT_NULL(ipc_response.ckv_data);
        DELETE_IF_NOT_NULL(okey);
        return result_code;
      }
      result_code = ReadControllerStateDetail(dup_ckv, temp_key, &ipc_response,
                                              req->datatype, req->operation,
                                              dmi);

      DELETE_IF_NOT_NULL(ipc_response.ckv_data);
      if (result_code != UPLL_RC_SUCCESS) {
        DELETE_IF_NOT_NULL(temp_key);
        DELETE_IF_NOT_NULL(dup_ckv);
        DELETE_IF_NOT_NULL(temp_vtn_pm_ckv);
        DELETE_IF_NOT_NULL(okey);
        return result_code;
      }
      DELETE_IF_NOT_NULL(temp_key);
      if (NULL == okey) {
        okey = dup_ckv;
      } else {
        okey->AppendCfgKeyVal(dup_ckv);
      }

      tmp_sib_count++;
      if (tmp_sib_count == req->rep_count)
        break;
      vtn_pm_ckv = vtn_pm_ckv->get_next_cfg_key_val();
    }
    if ((okey != NULL) && ((result_code == UPLL_RC_SUCCESS)
      || (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE))) {
      result_code = UPLL_RC_SUCCESS;
      ikey->ResetWith(okey);
      DELETE_IF_NOT_NULL(okey);
      req->rep_count = tmp_sib_count;
    }
  } else {
    result_code = UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT;
  }
  DELETE_IF_NOT_NULL(temp_vtn_pm_ckv);
  return result_code;
}

bool VtnPolicingMapMoMgr::GetRenameKeyBindInfo(unc_key_type_t key_type,
    BindInfo *&binfo, int &nattr,
    MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  UPLL_LOG_DEBUG("GetRenameKeyBindInfo (%d) (%d)", key_type, tbl);
  if (MAINTBL == tbl) {
    nattr = sizeof(key_vtnpm_vtn_maintbl_rename_bind_info)/
            sizeof(key_vtnpm_vtn_maintbl_rename_bind_info[0]);
    binfo = key_vtnpm_vtn_maintbl_rename_bind_info;
  } else if (CTRLRTBL == tbl) {
    nattr = sizeof(key_vtnpm_vtn_ctrlrtbl_rename_bind_info)/
            sizeof(key_vtnpm_vtn_ctrlrtbl_rename_bind_info[0]);
     binfo = key_vtnpm_vtn_ctrlrtbl_rename_bind_info;
  } else {
     UPLL_LOG_DEBUG("GetRenameKeyBindInfo Invalid Tbl (%d)", tbl);
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

  key_rename_vnode_info_t *key_rename =
    reinterpret_cast<key_rename_vnode_info_t *>(ikey->get_key());
  key_vtn_t *key_vtn = reinterpret_cast<key_vtn_t *>
    (ConfigKeyVal::Malloc(sizeof(key_vtn_t)));

  if (!strlen(reinterpret_cast<char *>(key_rename->old_unc_vtn_name))) {
    UPLL_LOG_DEBUG("CopyToConfigKey key_rename->old_unc_vtn_name NULL");
    FREE_IF_NOT_NULL(key_vtn);
    return UPLL_RC_ERR_GENERIC;
  }

  uuu::upll_strncpy(key_vtn->vtn_name,
      key_rename->old_unc_vtn_name,
      (kMaxLenVtnName + 1));

  okey = new ConfigKeyVal(UNC_KT_VTN_POLICINGMAP,
                          IpctSt::kIpcStKeyVtn, key_vtn, NULL);
  if (!okey) {
    UPLL_LOG_DEBUG("CopyToConfigKey okey NULL");
    FREE_IF_NOT_NULL(key_vtn);
    return UPLL_RC_ERR_GENERIC;
  }
  return result_code;
}

upll_rc_t VtnPolicingMapMoMgr::UpdateVnodeVal(ConfigKeyVal *ikey,
                                              DalDmlIntf *dmi,
                                              upll_keytype_datatype_t data_type,
                                              bool &no_rename) {
  UPLL_FUNC_TRACE;
  ConfigKeyVal *okey = NULL;
  ConfigKeyVal *kval = NULL;
  ConfigKeyVal *ckval = NULL;
  ConfigKeyVal *ckey = NULL;
  controller_domain ctrlr_dom;
  ctrlr_dom.ctrlr = NULL;
  ctrlr_dom.domain = NULL;

  uint8_t rename = 0;
  string vtn_name = "";
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  UPLL_LOG_DEBUG("CopyToConfigKey datatype (%d)", data_type);

  key_rename_vnode_info_t *key_rename =
  reinterpret_cast<key_rename_vnode_info_t *>(ikey->get_key());

  // Copy the old policer name in val_policingmap
  val_policingmap_t *val = reinterpret_cast<val_policingmap_t *>
          (ConfigKeyVal::Malloc(sizeof(val_policingmap_t)));
  if (!val) return UPLL_RC_ERR_GENERIC;

  memset(val, 0, sizeof(val_policingmap_t));
  if (!strlen(reinterpret_cast<char *>(key_rename->old_policingprofile_name))) {
    FREE_IF_NOT_NULL(val);
    return UPLL_RC_ERR_GENERIC;
  }

  uuu::upll_strncpy(val->policer_name,
      key_rename->old_policingprofile_name,
      (kMaxLenPolicingProfileName + 1));
  val->valid[UPLL_IDX_POLICERNAME_PM] = UNC_VF_VALID;
  UPLL_LOG_DEBUG("policer name and valid (%d) (%s)",
                  val->valid[UPLL_IDX_POLICERNAME_PM], val->policer_name);
//  ConfigVal *cval = new ConfigVal(IpctSt::kIpcStValPolicingmap, val);

  result_code = GetChildConfigKey(okey, NULL);
  if (UPLL_RC_SUCCESS != result_code) {
     UPLL_LOG_DEBUG("CopyToConfigKey okey  NULL");
     FREE_IF_NOT_NULL(val);
     return result_code;
  }
  if (!okey) {
    FREE_IF_NOT_NULL(val);
    return UPLL_RC_ERR_GENERIC;
  }
  okey->SetCfgVal(new ConfigVal(IpctSt::kIpcStValPolicingmap, val));

  DbSubOp dbop = { kOpReadMultiple, kOpMatchNone, kOpInOutFlag };

  // Read the record of key structure and old policer name in maintbl
  result_code = ReadConfigDB(okey, data_type, UNC_OP_READ, dbop, dmi,
    MAINTBL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_ERROR("Failed to Read from DB err %d", result_code);
    DELETE_IF_NOT_NULL(okey);
    return result_code;
  }
  ConfigKeyVal *tmp_okey = okey;
  while (okey != NULL) {
    // Update the new policer name in MAINTBL
    result_code = GetChildConfigKey(kval, okey);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("GetChildConfigKey kval NULL");
      DELETE_IF_NOT_NULL(okey);
      return result_code;
    }
    if (!kval) return UPLL_RC_ERR_GENERIC;

    // Copy the new policer name in val_policingmap
    val_policingmap_t *val1 = reinterpret_cast<val_policingmap_t *>
        (ConfigKeyVal::Malloc(sizeof(val_policingmap_t)));
    if (!val1) return UPLL_RC_ERR_GENERIC;
    memset(val1, 0, sizeof(val_policingmap_t));

    // New name null check
    if (!strlen(reinterpret_cast<char *>
                (key_rename->new_policingprofile_name))) {
      FREE_IF_NOT_NULL(val1);
      UPLL_LOG_DEBUG("new_policingprofile_name NULL");
      DELETE_IF_NOT_NULL(kval);
      DELETE_IF_NOT_NULL(okey);
      return UPLL_RC_ERR_GENERIC;
    }
    // Copy the new policer name into val_policingmap
    uuu::upll_strncpy(val1->policer_name,
      key_rename->new_policingprofile_name,
      (kMaxLenPolicingProfileName + 1));
    val1->valid[UPLL_IDX_POLICERNAME_PM] = UNC_VF_VALID;
    UPLL_LOG_DEBUG("policer name and valid (%d) (%s)",
                    val1->valid[UPLL_IDX_POLICERNAME_PM], val1->policer_name);
    ConfigVal *cval1 = new ConfigVal(IpctSt::kIpcStValPolicingmap, val1);

    kval->SetCfgVal(cval1);

    GET_USER_DATA_FLAGS(okey, rename);

    UPLL_LOG_DEBUG("okey flag (%d)", rename);
    if (!no_rename)
      rename = rename | POLICINGPROFILE_RENAME;
    else
      rename = rename & NO_POLICINGPROFILE_RENAME;

    SET_USER_DATA_FLAGS(kval, rename);
    UPLL_LOG_DEBUG("kval flag (%d)", rename);

    // Update the new policer name in MAINTBL
    result_code = UpdateConfigDB(kval, data_type, UNC_OP_UPDATE, dmi,
                  TC_CONFIG_GLOBAL, vtn_name, MAINTBL);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("Create record Err in vtnpolicingmaptbl CANDIDATE DB(%d)",
        result_code);
      DELETE_IF_NOT_NULL(kval);
      DELETE_IF_NOT_NULL(okey);
      return result_code;
    }

  // Get the momory alloctaed vtn key structure
  VtnMoMgr *vtnmgr =
    static_cast<VtnMoMgr *>((const_cast<MoManager *>
          (GetMoManager(UNC_KT_VTN))));
  result_code = vtnmgr->GetChildConfigKey(ckey, NULL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("UpdateRefCountInPPCtrlr GetChildConfigKey error (%d)",
        result_code);
    return result_code;
  }
  if (!ckey) return UPLL_RC_ERR_GENERIC;
  // To get the vtn associated controller name
  key_vtn_t *vtn_okey = reinterpret_cast<key_vtn_t *>(ckey->get_key());
  key_vtn_t *vtn_ikey = reinterpret_cast<key_vtn_t *>(okey->get_key());
  uuu::upll_strncpy(vtn_okey->vtn_name, vtn_ikey->vtn_name,
      kMaxLenVtnName+1);

  UPLL_LOG_DEBUG("vtn name ckey (%s) okey (%s)", vtn_okey->vtn_name,
                 vtn_ikey->vtn_name);
  result_code = vtnmgr->GetControllerDomainSpan(ckey, data_type, dmi);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetControllerSpan  no instance/error (%d)", result_code);
    if (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    DELETE_IF_NOT_NULL(kval);
    DELETE_IF_NOT_NULL(okey);
    DELETE_IF_NOT_NULL(ckey);
    return result_code;
    }
    DELETE_IF_NOT_NULL(kval);
    DELETE_IF_NOT_NULL(ckey);
    continue;
  }
  ConfigKeyVal *tmp_ckey = ckey;
  while (ckey != NULL) {
    // check for vnode_ref_cnt(vnodes count) in vtn_ctrlr_tbl val structure.
    // If the count is '0' then continue for next vtn_ctrlr_tbl.
    val_vtn_ctrlr *ctr_val =
        reinterpret_cast<val_vtn_ctrlr *>(GetVal(ckey));
    if (!ctr_val) {
      UPLL_LOG_ERROR("Vtn controller table val structure is NULL");
      DELETE_IF_NOT_NULL(okey);
      DELETE_IF_NOT_NULL(kval);
      DELETE_IF_NOT_NULL(ckey);
      return UPLL_RC_ERR_GENERIC;
    }
    if (ctr_val->vnode_ref_cnt <= 0) {
      UPLL_LOG_DEBUG("skipping entry");
      ckey = ckey->get_next_cfg_key_val();
      continue;
    }
    GET_USER_DATA_CTRLR_DOMAIN(ckey, ctrlr_dom);

    UPLL_LOG_DEBUG("ctrlr : %s; domain : %s", ctrlr_dom.ctrlr,
                    ctrlr_dom.domain);

    // Update the new policer name in CTRLTBL
    result_code = GetChildConfigKey(ckval, okey);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("GetChildConfigKey ckval NULL");
      DELETE_IF_NOT_NULL(kval);
      DELETE_IF_NOT_NULL(okey);
      DELETE_IF_NOT_NULL(ckey);
      return result_code;
    }
    if (!ckval) return UPLL_RC_ERR_GENERIC;

    val_vtnpolicingmap_ctrl  *policingmap_ctrlr_val =
        reinterpret_cast<val_vtnpolicingmap_ctrl *>
        (ConfigKeyVal::Malloc(sizeof(val_vtnpolicingmap_ctrl)));

    // Copy the new policer name into val_vtnpolicingmap_ctrl
    uuu::upll_strncpy(policingmap_ctrlr_val->policer_name,
      key_rename->new_policingprofile_name,
      (kMaxLenPolicingProfileName + 1));
    policingmap_ctrlr_val->valid[UPLL_IDX_POLICERNAME_PM] = UNC_VF_VALID;
    UPLL_LOG_DEBUG("policer name and valid (%d) (%s)",
                    policingmap_ctrlr_val->valid[UPLL_IDX_POLICERNAME_PM],
                    policingmap_ctrlr_val->policer_name);

    SET_USER_DATA_CTRLR_DOMAIN(ckval, ctrlr_dom);
    ckval->SetCfgVal(new ConfigVal(IpctSt::kIpcInvalidStNum,
                                   policingmap_ctrlr_val));

    SET_USER_DATA_FLAGS(ckval, rename);

    UPLL_LOG_TRACE("ckval %s", ckval->ToStrAll().c_str());

    // Update the new policer name in CTRLTBL
    result_code = UpdateConfigDB(ckval, data_type, UNC_OP_UPDATE, dmi,
                                 TC_CONFIG_GLOBAL, vtn_name, CTRLRTBL);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("Create record Err in vtnpolicingmaptbl CANDIDATE DB(%d)",
        result_code);
        DELETE_IF_NOT_NULL(kval);
        DELETE_IF_NOT_NULL(ckval);
        DELETE_IF_NOT_NULL(okey);
        return result_code;
    }

    DELETE_IF_NOT_NULL(ckval);

    ckey = ckey->get_next_cfg_key_val();
    }
    DELETE_IF_NOT_NULL(kval);
    DELETE_IF_NOT_NULL(tmp_ckey);
    okey = okey->get_next_cfg_key_val();
  }
  DELETE_IF_NOT_NULL(tmp_okey);
  return result_code;
}

upll_rc_t VtnPolicingMapMoMgr::MergeValidate(unc_key_type_t keytype,
                                             const char *ctrlr_id,
                                             ConfigKeyVal *ikey,
                                             DalDmlIntf *dmi,
                                             upll_import_type import_type) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_ERR_GENERIC;
  if (import_type == UPLL_IMPORT_TYPE_PARTIAL) {
    // Validate within IMPORT database for normal and multidomain case
    result_code = PI_MergeValidate_for_Vtn_Policingmap(keytype, ctrlr_id,
                                                       ikey, dmi);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_INFO("PI_MergeValidate_for_vtn_pm failed:%d", result_code);
      return result_code;
    }
    unc_keytype_operation_t op[] = { UNC_OP_DELETE, UNC_OP_CREATE };
    int nop = sizeof(op) / sizeof(op[0]);

    // Validate with IMPORT database with RUNNING database
    result_code = ValidateImportWithRunning(keytype, ctrlr_id,
                                       ikey, op, nop, dmi);
    if ((result_code != UPLL_RC_SUCCESS) &&
        (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE)) {
           UPLL_LOG_INFO("ValidateImportWithRunning DB err (%d)", result_code);
           return result_code;
    }
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtnPolicingMapMoMgr::MergeImportToCandidate(unc_key_type_t keytype,
                                            const char *ctrlr_name,
                                            DalDmlIntf *dmi,
                                            upll_import_type import_type) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *ckval = NULL;
  ConfigKeyVal *pm_imkey = NULL, *pm_cdkey = NULL, *ckval_dom = NULL;
  ConfigKeyVal *ckv_import = NULL, *ckv_cand = NULL;
  controller_domain ctrlr_dom;
  ctrlr_dom.ctrlr = NULL;
  ctrlr_dom.domain = NULL;
  uint8_t flag = 0;
  uint8_t *ctrlr_id = NULL;
  string vtn_id = "";

  if (UPLL_IMPORT_TYPE_PARTIAL == import_type) {
    UPLL_LOG_DEBUG("Partial Import");
    return (MoMgrImpl::MergeImportToCandidate(keytype, ctrlr_name,
                                               dmi, import_type));
  }

  if (NULL == ctrlr_name) {
    UPLL_LOG_ERROR("MergeValidate ctrlr_id NULL");
    return UPLL_RC_ERR_GENERIC;
  }

  VtnMoMgr *vtnmgr =
    static_cast<VtnMoMgr *>((const_cast<MoManager *>
        (GetMoManager(UNC_KT_VTN))));
  result_code = vtnmgr->GetChildConfigKey(ckval, NULL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetChildConfigKey ckval NULL");
    return result_code;
  }

  DbSubOp dbop = { kOpReadMultiple, kOpMatchNone, kOpInOutFlag };
  /* Read all vtn from VTN main table in import database and check with
   * Candidate database */
  result_code = vtnmgr->ReadConfigDB(ckval, UPLL_DT_IMPORT,
              UNC_OP_READ, dbop, dmi, MAINTBL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("ReadConfigDB ckval NULL (%d)", result_code);
    if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
       UPLL_LOG_DEBUG("NO record in vtn tbl (%d)", result_code);
       result_code = UPLL_RC_SUCCESS;
    }
    DELETE_IF_NOT_NULL(ckval);
    return result_code;
  }
  ConfigKeyVal *tmp_ckval = ckval;
  while (ckval != NULL) {
  /* Get the instance count from vtn ctrl table in candidate.
   * If refcount is more than 1,
   *    which means that the vtn is already exists in candidate
   * If refcount is zero or 1,
   *    which means that the imported vtn is not exists in candidate
   */
    uint32_t imp_instance_count, cand_instance_count;
    /* Get the instance count from vtn ctrl tbl from import db*/
    result_code = vtnmgr->GetChildConfigKey(ckv_import, ckval);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("GetChildConfigKey failed %d", result_code);
      DELETE_IF_NOT_NULL(tmp_ckval);
      return result_code;
    }
    result_code = vtnmgr->GetInstanceCount(ckv_import, NULL,
         UPLL_DT_IMPORT, &imp_instance_count, dmi, CTRLRTBL);
    DELETE_IF_NOT_NULL(ckv_import);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_ERROR("GetInstanceCount failed %d", result_code);
      DELETE_IF_NOT_NULL(tmp_ckval);
      return result_code;
    }

    /* Get the instance count from vtn ctrl tbl from candidate db*/
    result_code = vtnmgr->GetChildConfigKey(ckv_cand, ckval);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("GetChildConfigKey failed %d", result_code);
      DELETE_IF_NOT_NULL(tmp_ckval);
      return result_code;
    }
    result_code = vtnmgr->GetInstanceCount(ckv_cand, NULL,
         UPLL_DT_CANDIDATE, &cand_instance_count, dmi, CTRLRTBL);
    DELETE_IF_NOT_NULL(ckv_cand);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_ERROR("GetInstanceCount failed %d", result_code);
      DELETE_IF_NOT_NULL(tmp_ckval);
      return result_code;
    }
    UPLL_LOG_TRACE("Import count (%d) Candidate count (%d)",
                    imp_instance_count, cand_instance_count);
    if (imp_instance_count == cand_instance_count) {
      /* If imported ctrlr's VTN not exists in Candidate, then check the
       * existence of imported ctrlr's policingmap
       * 1)If the imported ctrlr VTN does not have policingmap, then continue
       * with the next VTN in imported db
       * 2)If the imported ctrlr VTN has policingmap, then merge this
       * policingmap into candidate db
      */
      UPLL_LOG_DEBUG("VTN not exists in candidate(%d)", result_code);

      /* Check the imported ctrl VTN's policingmap existence in Import db */
      key_vtn_t *vtn_ikey = reinterpret_cast<key_vtn_t *>(ckval->get_key());
      key_vtn_t *vtn_pm_imkey = reinterpret_cast<key_vtn_t*>
        (ConfigKeyVal::Malloc(sizeof(key_vtn_t)));
      uuu::upll_strncpy(vtn_pm_imkey->vtn_name, vtn_ikey->vtn_name,
                      kMaxLenVtnName+1);
      pm_imkey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn,
                                 vtn_pm_imkey, NULL);

      DbSubOp dbop1 = { kOpReadSingle, kOpMatchNone, kOpInOutFlag };
      upll_rc_t result_import = ReadConfigDB(pm_imkey, UPLL_DT_IMPORT,
             UNC_OP_READ, dbop1, dmi, MAINTBL);
      if (result_import != UPLL_RC_SUCCESS &&
          result_import != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
          DELETE_IF_NOT_NULL(pm_imkey);
          DELETE_IF_NOT_NULL(tmp_ckval);
          return result_code;
      }

      if (result_import == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
        /* If the imported ctrlr VTN does not have policingmap, then continue
         * with the next VTN in imported db */
        UPLL_LOG_DEBUG("policingmap not exists in import(%d)", result_code);
        DELETE_IF_NOT_NULL(pm_imkey);
        ckval = ckval->get_next_cfg_key_val();
        continue;
      } else if (result_import == UPLL_RC_SUCCESS) {
        /* If imported ctrlr VTN has policingmap, then merge this policingmap
         * into candidate db */

        /* Create the policingmap in main tbl in candidate database */
        result_code = UpdateConfigDB(pm_imkey, UPLL_DT_CANDIDATE, UNC_OP_CREATE,
                               dmi, TC_CONFIG_GLOBAL, vtn_id, MAINTBL);
       if (result_code != UPLL_RC_SUCCESS) {
         UPLL_LOG_DEBUG("create in CandidateDB failed (%d) ", result_code);
         DELETE_IF_NOT_NULL(pm_imkey);
         DELETE_IF_NOT_NULL(tmp_ckval);
         return result_code;
       }

       // Get the VTN across domain
       result_code = vtnmgr->GetChildConfigKey(ckval_dom, NULL);
       if (UPLL_RC_SUCCESS != result_code) {
         UPLL_LOG_DEBUG("GetChildConfigKey ckval NULL");
         DELETE_IF_NOT_NULL(pm_imkey);
         DELETE_IF_NOT_NULL(tmp_ckval);
         return result_code;
       }

       key_vtn_t *vtn_okey = reinterpret_cast<key_vtn_t *>
                                              (ckval_dom->get_key());
       key_vtn_t *vtn_ikey = reinterpret_cast<key_vtn_t *>(pm_imkey->get_key());
       uuu::upll_strncpy(vtn_okey->vtn_name, vtn_ikey->vtn_name,
                    kMaxLenVtnName+1);

       result_code = vtnmgr->GetControllerDomainSpan(ckval_dom,
                                   UPLL_DT_IMPORT, dmi);
       if (result_code != UPLL_RC_SUCCESS) {
         if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
           UPLL_LOG_DEBUG("GetControllerSpan no instance/error (%d)",
                 result_code);
           DELETE_IF_NOT_NULL(pm_imkey);
           ckval = ckval->get_next_cfg_key_val();
           continue;
         }
       }

       ConfigKeyVal *tmp_ckval_dom = ckval_dom;
       while (ckval_dom != NULL) {
        // check for vnode_ref_cnt(vnodes count) in vtn_ctrlr_tbl val structure.
        // If the count is '0' then continue for next vtn_ctrlr_tbl.
        val_vtn_ctrlr *ctr_val =
            reinterpret_cast<val_vtn_ctrlr *>(GetVal(ckval_dom));
        if (!ctr_val) {
          UPLL_LOG_ERROR("Vtn controller table val structure is NULL");
          DELETE_IF_NOT_NULL(tmp_ckval_dom);
          DELETE_IF_NOT_NULL(pm_imkey);
          DELETE_IF_NOT_NULL(tmp_ckval);
          return UPLL_RC_ERR_GENERIC;
        }
        if (ctr_val->vnode_ref_cnt <= 0) {
          UPLL_LOG_DEBUG("skipping entry");
          ckval_dom = ckval_dom->get_next_cfg_key_val();
          continue;
        }
         /* Create the record in vtn policingmap ctrlr table with as per the
          * ctrlr and domain */
         ConfigKeyVal *ctrlckv = NULL;
         GET_USER_DATA_CTRLR_DOMAIN(ckval_dom, ctrlr_dom);
         GET_USER_DATA_FLAGS(pm_imkey, flag);
         UPLL_LOG_DEBUG("ctrlr : %s; domain : %s", ctrlr_dom.ctrlr,
                       ctrlr_dom.domain);
         UPLL_LOG_DEBUG("flag (%d)", flag);

         result_code = GetChildConfigKey(ctrlckv, pm_imkey);
         if (UPLL_RC_SUCCESS != result_code) {
           UPLL_LOG_DEBUG("GetChildConfigKey ckval NULL");
           DELETE_IF_NOT_NULL(tmp_ckval_dom);
           DELETE_IF_NOT_NULL(pm_imkey);
           DELETE_IF_NOT_NULL(tmp_ckval);
          return result_code;
         }

         val_policingmap_t *val2 = reinterpret_cast<val_policingmap_t *>
                                   (GetVal(pm_imkey));
         val_vtnpolicingmap_ctrl  *policingmap_ctrlr_val =
         reinterpret_cast<val_vtnpolicingmap_ctrl *>
         (ConfigKeyVal::Malloc(sizeof(val_vtnpolicingmap_ctrl)));
         uuu::upll_strncpy(policingmap_ctrlr_val->policer_name,
              val2->policer_name, (kMaxLenPolicingProfileName + 1));

         policingmap_ctrlr_val->valid[UPLL_IDX_POLICERNAME_PM] = UNC_VF_VALID;
         UPLL_LOG_DEBUG("policer_name (%s) valid (%d)",
                    policingmap_ctrlr_val->policer_name,
                    policingmap_ctrlr_val->valid[UPLL_IDX_POLICERNAME_PM]);
         SET_USER_DATA_CTRLR_DOMAIN(ctrlckv, ctrlr_dom);
         SET_USER_DATA_FLAGS(ctrlckv, flag);
         UPLL_LOG_DEBUG("flag (%d)", flag);
         ctrlckv->SetCfgVal(new ConfigVal(IpctSt::kIpcInvalidStNum,
                                policingmap_ctrlr_val));

         /* Create a record in ctrlr tbl in candidate db */
         result_code = UpdateConfigDB(ctrlckv, UPLL_DT_CANDIDATE,
                                      UNC_OP_CREATE, dmi, TC_CONFIG_GLOBAL,
                                      vtn_id, CTRLRTBL);
         if (result_code != UPLL_RC_SUCCESS) {
           UPLL_LOG_DEBUG("Err while inserting in ctrlr table (%d)",
                         result_code);
           DELETE_IF_NOT_NULL(ctrlckv);
           DELETE_IF_NOT_NULL(tmp_ckval_dom);
           DELETE_IF_NOT_NULL(pm_imkey);
           DELETE_IF_NOT_NULL(tmp_ckval);
           return result_code;
         }
         DELETE_IF_NOT_NULL(ctrlckv);
         ckval_dom = ckval_dom->get_next_cfg_key_val();
       }
       DELETE_IF_NOT_NULL(tmp_ckval_dom);
     }
     DELETE_IF_NOT_NULL(pm_imkey);
  } else if (imp_instance_count < cand_instance_count) {
      /* If imported ctrlr's VTN exists in Candidate, then check the existence
        of imported ctrlr VTN's policingmap and UNC VTN's policingmap
        1)If the imported ctrlr VTN does not have policingmap, then apply the
          UNC VTN's policingmap to imported ctrl VTN also in candidate db
        2)If the imported ctrlr VTN has policingmap, then return MERGE_CONFLICT
          error, because of PFC limitation
     */
     UPLL_LOG_DEBUG("VTN exists in candidate(%d)", result_code);

      /* Check the policingmap existence in Import db */
      key_vtn_t *vtn_ikey = reinterpret_cast<key_vtn_t *>(ckval->get_key());
      key_vtn_t *vtn_pm_imkey = reinterpret_cast<key_vtn_t*>
        (ConfigKeyVal::Malloc(sizeof(key_vtn_t)));
      uuu::upll_strncpy(vtn_pm_imkey->vtn_name, vtn_ikey->vtn_name,
                       kMaxLenVtnName+1);
      pm_imkey = new ConfigKeyVal(UNC_KT_VTN,
                              IpctSt::kIpcStKeyVtn,
                              vtn_pm_imkey, NULL);

      DbSubOp dbop1 = { kOpReadSingle, kOpMatchNone, kOpInOutFlag };
      upll_rc_t result_import = ReadConfigDB(pm_imkey, UPLL_DT_IMPORT,
              UNC_OP_READ, dbop1, dmi, MAINTBL);
      if (result_import != UPLL_RC_SUCCESS &&
          result_import != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
          DELETE_IF_NOT_NULL(pm_imkey);
          DELETE_IF_NOT_NULL(tmp_ckval);
          return result_code;
      }

      /* Check the policingmap existence in candidate db */
      key_vtn_t *vtn_pm_cdkey = reinterpret_cast<key_vtn_t*>
        (ConfigKeyVal::Malloc(sizeof(key_vtn_t)));
      uuu::upll_strncpy(vtn_pm_cdkey->vtn_name, vtn_ikey->vtn_name,
                       kMaxLenVtnName+1);
      pm_cdkey = new ConfigKeyVal(UNC_KT_VTN,
                              IpctSt::kIpcStKeyVtn,
                              vtn_pm_cdkey, NULL);

      upll_rc_t result_cand = ReadConfigDB(pm_cdkey, UPLL_DT_CANDIDATE,
              UNC_OP_READ, dbop1, dmi, MAINTBL);
       if (result_cand != UPLL_RC_SUCCESS &&
           result_cand != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
          DELETE_IF_NOT_NULL(pm_imkey);
          DELETE_IF_NOT_NULL(pm_cdkey);
          DELETE_IF_NOT_NULL(tmp_ckval);
          return result_code;
      }

      if ((result_import == UPLL_RC_ERR_NO_SUCH_INSTANCE ||
          result_import == UPLL_RC_SUCCESS) &&
          result_cand == UPLL_RC_SUCCESS) {
      /*1)If the imported ctrlr VTN does not have policingmap, then apply the
          UNC VTN's policingmap to imported ctrl VTN also in candidate db
        2)If the imported ctrlr VTN has policingmap, then return MERGE_CONFLICT
          error, because of PFC limitation */

       /* The below code can be uncommented, once the PFC limitation issue got
          solved
        PFC Limitation :
          Simultaneous execution of policing-profile deletion and policing-map
          modification
        If imported ctrl has policingmap,
           *  Ignore this policing map and decrement the refcount of PPCTRL tbl,
           *  so that this impoted ctrl's policing profile and matched flowlist
           *  will not applied into imported ctrl during AUDIT
        if (result_import == UPLL_RC_SUCCESS) {
          result_code = DecRefCountInPPCtrlTbl(pm_imkey, dmi);
          DELETE_IF_NOT_NULL(pm_imkey);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("Err in DecRefCountInPPCtrl (%d)", result_code);
            DELETE_IF_NOT_NULL(tmp_ckval);
            return result_code;
          }
        }
        UPLL_LOG_DEBUG("DecRefCountInPMCtrlTbl success (%d)", result_code);
       */

       /* If imported ctrl also has policingmap,
        * Return MERGE_CONFLICT, because limitation on PFC */
        if (result_import == UPLL_RC_SUCCESS) {
          UPLL_LOG_ERROR("IM & CAND has PM, return MERGE CONFLICT, PFC limit");
          DELETE_IF_NOT_NULL(pm_imkey);
          DELETE_IF_NOT_NULL(pm_cdkey);
          DELETE_IF_NOT_NULL(tmp_ckval);
          return UPLL_RC_ERR_MERGE_CONFLICT;
        }

        DELETE_IF_NOT_NULL(pm_imkey);
        result_code = vtnmgr->GetChildConfigKey(ckval_dom, NULL);
        if (UPLL_RC_SUCCESS != result_code) {
          UPLL_LOG_DEBUG("GetChildConfigKey ckval NULL");
          DELETE_IF_NOT_NULL(pm_cdkey);
          DELETE_IF_NOT_NULL(tmp_ckval);
          return result_code;
        }

        uint8_t pp_flag = 0;
        key_vtn_t *vtn_okey = reinterpret_cast<key_vtn_t *>
                              (ckval_dom->get_key());
        key_vtn *vtn_ikey = reinterpret_cast<key_vtn_t *>(pm_cdkey->get_key());
        uuu::upll_strncpy(vtn_okey->vtn_name, vtn_ikey->vtn_name,
                       kMaxLenVtnName+1);

        result_code = vtnmgr->GetControllerDomainSpan(ckval_dom,
                                      UPLL_DT_IMPORT, dmi);
        if (result_code != UPLL_RC_SUCCESS) {
          if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
            UPLL_LOG_DEBUG("GetControllerSpan no instance/error (%d)",
                   result_code);
            DELETE_IF_NOT_NULL(pm_cdkey);
            ckval = ckval->get_next_cfg_key_val();
            continue;
          }
        }
        PolicingProfileMoMgr *pp_mgr =
          reinterpret_cast<PolicingProfileMoMgr *>
          (const_cast<MoManager *>(GetMoManager(UNC_KT_POLICING_PROFILE)));

        ConfigKeyVal *tmp_ckval_dom = ckval_dom;
        while (ckval_dom != NULL) {
            // check for vnode_ref_cnt in vtn_ctrlr_tbl val structure.
            // If the count is '0' then continue for next vtn_ctrlr_tbl.
            val_vtn_ctrlr *ctr_val =
                reinterpret_cast<val_vtn_ctrlr *>(GetVal(ckval_dom));
            if (!ctr_val) {
              UPLL_LOG_ERROR("Vtn controller table val structure is NULL");
              DELETE_IF_NOT_NULL(tmp_ckval_dom);
              DELETE_IF_NOT_NULL(pm_cdkey);
              DELETE_IF_NOT_NULL(tmp_ckval);
              return UPLL_RC_ERR_GENERIC;
            }
            if (ctr_val->vnode_ref_cnt <= 0) {
              UPLL_LOG_DEBUG("skipping entry");
              ckval_dom = ckval_dom->get_next_cfg_key_val();
              continue;
            }
            // Create the entry in ctrlr table with as per the ctrlr and domain
            ConfigKeyVal *ctrlckv = NULL;
            GET_USER_DATA_CTRLR_DOMAIN(ckval_dom, ctrlr_dom);
            GET_USER_DATA_FLAGS(pm_cdkey, flag);
            UPLL_LOG_DEBUG("ctrlr : %s; domain : %s", ctrlr_dom.ctrlr,
                         ctrlr_dom.domain);
            UPLL_LOG_DEBUG("flag (%d)", flag);

            result_code = GetChildConfigKey(ctrlckv, pm_cdkey);
            if (UPLL_RC_SUCCESS != result_code) {
              UPLL_LOG_DEBUG("GetChildConfigKey ckval NULL");
              DELETE_IF_NOT_NULL(tmp_ckval_dom);
              DELETE_IF_NOT_NULL(pm_cdkey);
              DELETE_IF_NOT_NULL(tmp_ckval);
              return result_code;
           }

           val_policingmap_t *val2 = reinterpret_cast<val_policingmap_t *>
                                     (GetVal(pm_cdkey));
           val_vtnpolicingmap_ctrl  *policingmap_ctrlr_val =
           reinterpret_cast<val_vtnpolicingmap_ctrl *>
             (ConfigKeyVal::Malloc(sizeof(val_vtnpolicingmap_ctrl)));
           uuu::upll_strncpy(policingmap_ctrlr_val->policer_name,
             val2->policer_name, (kMaxLenPolicingProfileName + 1));

           policingmap_ctrlr_val->valid[UPLL_IDX_POLICERNAME_PM] =
             UNC_VF_VALID;

           GET_USER_DATA_CTRLR(ckval_dom, ctrlr_id);
           std::string temp_vtn_name;
           temp_vtn_name = reinterpret_cast<const char *>(reinterpret_cast
                           <key_vtn_t *>(pm_cdkey->get_key())->vtn_name);
           result_code = pp_mgr->PolicingProfileCtrlrTblOper(
             reinterpret_cast<const char *>(val2->policer_name),
             reinterpret_cast<const char *>(ctrlr_id), dmi, UNC_OP_CREATE,
             UPLL_DT_CANDIDATE, pp_flag, TC_CONFIG_GLOBAL, temp_vtn_name,
             1, false);
           if (UPLL_RC_SUCCESS != result_code) {
             UPLL_LOG_INFO("PolicingProfileCtrlrTblOper err:%d", result_code);
             DELETE_IF_NOT_NULL(ctrlckv);
             DELETE_IF_NOT_NULL(tmp_ckval_dom);
             DELETE_IF_NOT_NULL(pm_cdkey);
             DELETE_IF_NOT_NULL(tmp_ckval);
             return result_code;
           }

           UPLL_LOG_DEBUG("policer_name (%s) valid (%d)",
             policingmap_ctrlr_val->policer_name,
             policingmap_ctrlr_val->valid[UPLL_IDX_POLICERNAME_PM]);
           SET_USER_DATA_CTRLR_DOMAIN(ctrlckv, ctrlr_dom);
           SET_USER_DATA_FLAGS(ctrlckv, flag);
           UPLL_LOG_DEBUG("flag (%d)", flag);
           ctrlckv->SetCfgVal(new ConfigVal(IpctSt::kIpcInvalidStNum,
                                  policingmap_ctrlr_val));

            // Create a record in CANDIDATE DB
            result_code = UpdateConfigDB(ctrlckv, UPLL_DT_CANDIDATE,
                          UNC_OP_CREATE, dmi, TC_CONFIG_GLOBAL, vtn_id,
                          CTRLRTBL);
            if (result_code != UPLL_RC_SUCCESS) {
              UPLL_LOG_DEBUG("Err while inserting in ctrlr table (%d)",
                           result_code);
              DELETE_IF_NOT_NULL(ctrlckv);
              DELETE_IF_NOT_NULL(tmp_ckval_dom);
              DELETE_IF_NOT_NULL(pm_cdkey);
              DELETE_IF_NOT_NULL(tmp_ckval);
              return result_code;
            }
            DELETE_IF_NOT_NULL(ctrlckv);
            ckval_dom = ckval_dom->get_next_cfg_key_val();
          }
          DELETE_IF_NOT_NULL(tmp_ckval_dom);
          DELETE_IF_NOT_NULL(pm_cdkey);
      } else if (result_import == UPLL_RC_SUCCESS &&
        result_cand == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
        UPLL_LOG_DEBUG("policingmap not exists in candidate");
        /* The below code can be uncommented, once the PFC limitation issue
           got solved
           PFC Limitation :
           Simultaneous execution of policing-profile deletion and policing-map
           modification
           If imported ctrl has policingmap and UNC doesn't have policingmap,
           Ignore this policing map and decrement the refcount of PPCTRL tbl,
           so that this impoted ctrl's policing profile and matched flowlist
           will not applied into imported ctrl during AUDIT
        result_code = DecRefCountInPPCtrlTbl(pm_imkey, dmi);
        DELETE_IF_NOT_NULL(pm_cdkey);
        DELETE_IF_NOT_NULL(pm_imkey);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("Err in DecRefCountInPPCtrl (%d)", result_code);
          DELETE_IF_NOT_NULL(tmp_ckval);
          return result_code;
        }
        UPLL_LOG_DEBUG("DecRefCountInFLCtrlTbl success (%d)", result_code);
        */
       /* If imported ctrl has policingmap and candidate does not have
        * policingmap, Return MERGE_CONFLICT, because limitation on PFC */
          UPLL_LOG_ERROR("IMPORT DB has PM, ret MERGE CONFLICT, PFC limit");
          DELETE_IF_NOT_NULL(pm_imkey);
          DELETE_IF_NOT_NULL(pm_cdkey);
          DELETE_IF_NOT_NULL(tmp_ckval);
          return UPLL_RC_ERR_MERGE_CONFLICT;
      } else if (result_import == UPLL_RC_ERR_NO_SUCH_INSTANCE &&
                  result_cand == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
         DELETE_IF_NOT_NULL(pm_imkey);
         DELETE_IF_NOT_NULL(pm_cdkey);
      }
    }
    ckval = ckval->get_next_cfg_key_val();
  }
  DELETE_IF_NOT_NULL(tmp_ckval);
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtnPolicingMapMoMgr::GetRenamedUncKey(ConfigKeyVal *ctrlr_key,
    upll_keytype_datatype_t dt_type,
    DalDmlIntf *dmi,
    uint8_t *ctrlr_id) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code;
  ConfigKeyVal *unc_key = NULL;
  UPLL_LOG_TRACE("%s GetRenamedUncKey vtnpm start",
                  ctrlr_key->ToStrAll().c_str());
  if (NULL == ctrlr_key || NULL == dmi || ctrlr_id[0] == '\0') {
    UPLL_LOG_DEBUG("GetRenamedUncKey failed. Insufficient input parameters.");
    return UPLL_RC_ERR_GENERIC;
  }

  uint8_t rename = 0;
  key_vtn *ctrlr_vtn_key = reinterpret_cast<key_vtn *>(ctrlr_key->get_key());
  if (!ctrlr_vtn_key) {
    UPLL_LOG_DEBUG("ctrlr_vtn_key NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  DbSubOp dbop = { kOpReadSingle, kOpMatchCtrlr, kOpInOutCtrlr |
                   kOpInOutDomain };
  val_rename_vtn *rename_vtn = reinterpret_cast<val_rename_vtn *>
      (ConfigKeyVal::Malloc(sizeof(val_rename_vtn)));
  if (!rename_vtn) {
    UPLL_LOG_DEBUG("rename_vtn NULL");
    return UPLL_RC_ERR_GENERIC;
  }
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
    UPLL_LOG_DEBUG("unc_key NULL");
    free(rename_vtn);
    return UPLL_RC_ERR_GENERIC;
  }

  ConfigVal *p_val = new ConfigVal(
      IpctSt::kIpcStValRenameVtn, rename_vtn);

  unc_key->SetCfgVal(p_val);

  SET_USER_DATA_CTRLR(unc_key, ctrlr_id);

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
    UPLL_LOG_DEBUG("GetRenamedUncKey ReadConfigDB success");
    key_vtn *vtn_key = reinterpret_cast<key_vtn *>(unc_key->get_key());
    if (vtn_key) {
      if (strcmp(reinterpret_cast<const char *>(ctrlr_vtn_key->vtn_name),
            reinterpret_cast<const char *>(vtn_key->vtn_name))) {
        uuu::upll_strncpy(ctrlr_vtn_key->vtn_name,
            vtn_key->vtn_name,
            (kMaxLenVtnName + 1));
        rename |= VTN_RENAME;
        SET_USER_DATA(ctrlr_key, unc_key);
        SET_USER_DATA_FLAGS(ctrlr_key, rename);
      }
    }
  }

  mgr = NULL;
  DELETE_IF_NOT_NULL(unc_key);

  val_rename_policingprofile *rename_policingprofile =
    reinterpret_cast<val_rename_policingprofile *>
    (ConfigKeyVal::Malloc(sizeof(val_rename_policingprofile)));
  if (!rename_policingprofile) {
    UPLL_LOG_DEBUG("rename_policingprofile NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  val_policingmap_t *val_policingmap =
    reinterpret_cast<val_policingmap_t *>(GetVal(ctrlr_key));
  if (!val_policingmap) {
    free(rename_policingprofile);
    return UPLL_RC_SUCCESS;
  }

  memset(rename_policingprofile->policingprofile_newname, '\0',
      sizeof(rename_policingprofile->policingprofile_newname));
  uuu::upll_strncpy(
      rename_policingprofile->policingprofile_newname,
      val_policingmap->policer_name,
      (kMaxLenPolicingProfileName + 1));

  rename_policingprofile->valid[UPLL_IDX_RENAME_PROFILE_RPP] = UNC_VF_VALID;

  MoMgrImpl *pp_mgr =
    reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>(GetMoManager(
            UNC_KT_POLICING_PROFILE)));
  if (NULL == pp_mgr) {
    UPLL_LOG_DEBUG("mgr NULL");
    if (rename_policingprofile) free(rename_policingprofile);
    return UPLL_RC_ERR_GENERIC;
  }

  result_code = pp_mgr->GetChildConfigKey(unc_key, NULL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetChildConfigKey failed %d", result_code);
    if (rename_policingprofile) free(rename_policingprofile);
    return result_code;
  }
  if (!unc_key) {
    UPLL_LOG_DEBUG("unc_key NULL");
    if (rename_policingprofile) free(rename_policingprofile);
    return UPLL_RC_ERR_GENERIC;
  }

  ConfigVal *p_config_val = new ConfigVal(
      IpctSt::kIpcStValRenamePolicingprofile, rename_policingprofile);

  unc_key->SetCfgVal(p_config_val);
  SET_USER_DATA_CTRLR(unc_key, ctrlr_id);
  result_code = pp_mgr->ReadConfigDB(unc_key, dt_type, UNC_OP_READ, dbop, dmi,
      RENAMETBL);
  if ((UPLL_RC_SUCCESS != result_code) &&
      (UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code)) {
    UPLL_LOG_DEBUG("ReadConfigDB failed %d", result_code);
    pp_mgr = NULL;
    DELETE_IF_NOT_NULL(unc_key);
    return result_code;
  }

  if (result_code == UPLL_RC_SUCCESS) {
    key_policingprofile_t *key_policingprofile =
      reinterpret_cast<key_policingprofile_t *>(unc_key->get_key());
    uuu::upll_strncpy(
        val_policingmap->policer_name,
        key_policingprofile->policingprofile_name,
        (kMaxLenPolicingProfileName + 1));
    rename |= POLICINGPROFILE_RENAME;
    uint8_t *pm_ctrlr_id = NULL;
    GET_USER_DATA_CTRLR(unc_key, pm_ctrlr_id);
//    SET_USER_DATA(ctrlr_key, unc_key);
    SET_USER_DATA_CTRLR(ctrlr_key, pm_ctrlr_id);
    SET_USER_DATA_FLAGS(ctrlr_key, rename);
  }
  int flg = 0;
  GET_USER_DATA_FLAGS(ctrlr_key, flg);
  UPLL_LOG_DEBUG(" dt_type (%d), flag (%d)", dt_type, flg);
  UPLL_LOG_TRACE("%s GetRenamedUncKey vtnpm end",
                  ctrlr_key->ToStrAll().c_str());
  DELETE_IF_NOT_NULL(unc_key);
  pp_mgr = NULL;
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtnPolicingMapMoMgr::GetControllerDomainSpan(
    ConfigKeyVal *ikey, upll_keytype_datatype_t dt_type, DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code;
  DbSubOp dbop = { kOpReadMultiple, kOpMatchNone,
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
  string vtn_name = "";

  UPLL_FUNC_TRACE;
  if (op != UNC_OP_DELETE) {
    result_code = DupConfigKeyVal(ck_vtn, vtn_key, MAINTBL);
    if (!ck_vtn || result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_INFO("Returning error %d", result_code);
      return UPLL_RC_ERR_GENERIC;
    }
    pm_val = reinterpret_cast<val_policingmap_t *>(GetVal(ck_vtn));
    if (!pm_val) {
      DELETE_IF_NOT_NULL(ck_vtn);
      UPLL_LOG_INFO("invalid val");
      return UPLL_RC_ERR_GENERIC;
    }
  } else {
    result_code = GetChildConfigKey(ck_vtn, vtn_key);
    if (!ck_vtn || result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Returning error %d", result_code);
      return UPLL_RC_ERR_GENERIC;
    }
  }
  switch (op) {
    case UNC_OP_CREATE:
      pm_val->cs_row_status = UNC_CS_APPLIED;
      break;
    case UNC_OP_UPDATE:
      pmval = reinterpret_cast<void *>(&pm_val);
      npmval = (nreq)?GetVal(nreq):NULL;
      if (!npmval) {
        DELETE_IF_NOT_NULL(ck_vtn);
        UPLL_LOG_ERROR("Invalid param");
        return UPLL_RC_ERR_GENERIC;
      }
      CompareValidValue(pmval, npmval, false);
      pm_val->cs_row_status =
             reinterpret_cast<val_policingmap_t *>
                       (GetVal(nreq))->cs_row_status;
      break;
    case UNC_OP_DELETE:
      break;
    default:
      DELETE_IF_NOT_NULL(ck_vtn);
      UPLL_LOG_DEBUG("Inalid operation");
      return UPLL_RC_ERR_GENERIC;
  }

  DbSubOp dbop = {kOpNotRead, kOpMatchNone, kOpInOutNone};
  dbop.inoutop = kOpInOutCs | kOpInOutFlag;
  result_code = UpdateConfigDB(ck_vtn, UPLL_DT_STATE, op, dmi, &dbop,
                               TC_CONFIG_GLOBAL, vtn_name, MAINTBL);
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

  if (NULL == ikey || NULL == dmi) {
    UPLL_LOG_DEBUG("Insufficient input resources");
    return UPLL_RC_ERR_GENERIC;
  }
  UPLL_LOG_TRACE("Start .. InputConfigKeyVal %s", ikey->ToStrAll().c_str());

  if (UNC_KT_VTN_POLICINGMAP_CONTROLLER == ikey->get_key_type()) {
    UPLL_LOG_DEBUG("vtn_pm_ctrl vtn name renamed");
    MoMgrImpl *vtn_mgr = reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>
        (GetMoManager(UNC_KT_VTN)));
    if (NULL == vtn_mgr) {
      UPLL_LOG_DEBUG("mgr NULL");
      return UPLL_RC_ERR_GENERIC;
    }
  /* Read controller name from running rename table,
   * since there is no rename table for audit case */
  if (dt_type == UPLL_DT_AUDIT)
    dt_type = UPLL_DT_RUNNING;

    result_code = vtn_mgr->GetChildConfigKey(okey, NULL);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("GetChildConfigKey fail (%d)", result_code);
      return result_code;
    }
    if (!okey) return UPLL_RC_ERR_GENERIC;

    DbSubOp dbop = { kOpReadSingle, kOpMatchCtrlr | kOpMatchDomain,
                     kOpInOutFlag };

    if (ctrlr_dom) {
      SET_USER_DATA_CTRLR_DOMAIN(okey, *ctrlr_dom);
    } else {
      UPLL_LOG_DEBUG("ctrlr_dom null");
      DELETE_IF_NOT_NULL(okey);
      return UPLL_RC_ERR_GENERIC;
    }

    UPLL_LOG_DEBUG("ctrlr : %s; domain : %s", ctrlr_dom->ctrlr,
        ctrlr_dom->domain);

    uuu::upll_strncpy(
        reinterpret_cast<key_vtn *>(okey->get_key())->vtn_name,
        reinterpret_cast<key_vtn_policingmap_controller_t *>
        (ikey->get_key())->vtn_key.vtn_name,
        (kMaxLenVtnName + 1));

    UPLL_LOG_DEBUG("vtn name (%s) (%s)",
        reinterpret_cast<key_vtn *>(okey->get_key())->vtn_name,
        reinterpret_cast<key_vtn_policingmap_controller_t *>
        (ikey->get_key())->vtn_key.vtn_name);

    result_code = vtn_mgr->ReadConfigDB(okey, dt_type, UNC_OP_READ, dbop, dmi,
        RENAMETBL);
    if (UPLL_RC_SUCCESS != result_code) {
      if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
        UPLL_LOG_DEBUG("ReadConfigDB no instance");
        DELETE_IF_NOT_NULL(okey);
        return UPLL_RC_SUCCESS;
      }
      UPLL_LOG_DEBUG("GetRenamedControllerKey ReadConfigDB error");
      DELETE_IF_NOT_NULL(okey);
      return result_code;
    }

    val_rename_vtn *rename_val =
      reinterpret_cast<val_rename_vtn *>(GetVal(okey));
    if (!rename_val) {
      UPLL_LOG_DEBUG("rename_val NULL");
      DELETE_IF_NOT_NULL(okey);
      return UPLL_RC_ERR_GENERIC;
    }

    uuu::upll_strncpy(
        reinterpret_cast<key_vtn_policingmap_controller_t*>
        (ikey->get_key())->vtn_key.vtn_name,
        rename_val->new_name,
        kMaxLenVtnName + 1);

    UPLL_LOG_DEBUG("renamed vtn_pm_ctrl  vtn name (%s) (%s)",
        reinterpret_cast<key_vtn_policingmap_controller_t*>
        (ikey->get_key())->vtn_key.vtn_name,
        rename_val->new_name);

    DELETE_IF_NOT_NULL(okey);
    vtn_mgr = NULL;
    return UPLL_RC_SUCCESS;
  }

  // Getting controller name for UNC_KT_VTN_POLICINGMAP
  MoMgrImpl *vtn_mgr = reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>
      (GetMoManager(UNC_KT_VTN)));
  if (NULL == vtn_mgr) {
    UPLL_LOG_DEBUG("mgr NULL");
    return UPLL_RC_ERR_GENERIC;
  }

  result_code = vtn_mgr->GetChildConfigKey(okey, NULL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetChildConfigKey fail (%d)", result_code);
    return result_code;
  }

  DbSubOp dbop = { kOpReadSingle, kOpMatchCtrlr | kOpMatchDomain,
    kOpInOutFlag };

  if (ctrlr_dom) {
    SET_USER_DATA_CTRLR_DOMAIN(okey, *ctrlr_dom);
  } else {
    UPLL_LOG_DEBUG("ctrlr_dom null");
    DELETE_IF_NOT_NULL(okey);
    return UPLL_RC_ERR_GENERIC;
  }

  UPLL_LOG_DEBUG("ctrlr : %s; domain : %s", ctrlr_dom->ctrlr,
      ctrlr_dom->domain);

  uuu::upll_strncpy(
      reinterpret_cast<key_vtn *>(okey->get_key())->vtn_name,
      reinterpret_cast<key_vtn_t *>
      (ikey->get_key())->vtn_name,
      (kMaxLenVtnName + 1));

  UPLL_LOG_DEBUG("vtn name (%s) (%s)",
      reinterpret_cast<key_vtn *>(okey->get_key())->vtn_name,
      reinterpret_cast<key_vtn_t *>(ikey->get_key())->vtn_name);

  result_code = vtn_mgr->ReadConfigDB(okey, dt_type, UNC_OP_READ, dbop, dmi,
      RENAMETBL);
  if ((UPLL_RC_SUCCESS != result_code) &&
      (UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code)) {
    UPLL_LOG_DEBUG("GetRenamedControllerKey ReadConfigDB error");
    DELETE_IF_NOT_NULL(okey);
    return result_code;
  }
  if (UPLL_RC_SUCCESS == result_code) {
    val_rename_vtn *rename_val =
      reinterpret_cast<val_rename_vtn *>(GetVal(okey));
    if (!rename_val) {
      UPLL_LOG_DEBUG("rename_val NULL");
      DELETE_IF_NOT_NULL(okey);
      return UPLL_RC_ERR_GENERIC;
    }

    uuu::upll_strncpy(
        reinterpret_cast<key_vtn_t*>(ikey->get_key())->vtn_name,
        rename_val->new_name,
        kMaxLenVtnName + 1);
  }
  DELETE_IF_NOT_NULL(okey);
  vtn_mgr = NULL;
  UPLL_LOG_DEBUG("vtn name renamed");
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
  SET_USER_DATA_CTRLR(okey, ctrlr_dom->ctrlr);

  val_policingmap_t *val_policingmap =
    reinterpret_cast<val_policingmap_t *>(GetVal(ikey));

  if (!val_policingmap) {
    DELETE_IF_NOT_NULL(okey);
    return UPLL_RC_SUCCESS;
  }

  key_policingprofile_t *key_policingprofile =
    reinterpret_cast<key_policingprofile_t *>(okey->get_key());

  uuu::upll_strncpy(key_policingprofile->policingprofile_name,
      val_policingmap->policer_name,
      (kMaxLenPolicingProfileName + 1));

  UPLL_LOG_DEBUG("policer name (%s) (%s)", val_policingmap->policer_name,
      key_policingprofile->policingprofile_name);

  DbSubOp dbop1 = { kOpReadSingle, kOpMatchCtrlr, kOpInOutFlag };
  result_code = pp_mgr->ReadConfigDB(okey, dt_type, UNC_OP_READ, dbop1, dmi,
      RENAMETBL);
  if ((result_code != UPLL_RC_SUCCESS) &&
      (UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code)) {
    UPLL_LOG_DEBUG("ReadConfigDB fail (%d)", result_code);
    DELETE_IF_NOT_NULL(okey);
    return result_code;
  }
  if (UPLL_RC_SUCCESS == result_code) {
    val_rename_policingprofile_t *rename_policingprofile =
      reinterpret_cast<val_rename_policingprofile_t *>(GetVal(okey));
    if (!rename_policingprofile) {
      DELETE_IF_NOT_NULL(okey);
      UPLL_LOG_DEBUG("rename_policingprofile NULL");
      return UPLL_RC_ERR_GENERIC;
    }

    uuu::upll_strncpy(
        reinterpret_cast<char *>(val_policingmap->policer_name),
        reinterpret_cast<const char *>
        (rename_policingprofile->policingprofile_newname),
        (kMaxLenPolicingProfileName + 1));

    UPLL_LOG_DEBUG("policer rename (%s) (%s)",
        reinterpret_cast<char *>(val_policingmap->policer_name),
        reinterpret_cast<const char *>
        (rename_policingprofile->policingprofile_newname));
  }
  pp_mgr = NULL;

  DELETE_IF_NOT_NULL(okey);
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtnPolicingMapMoMgr::TxCopyCandidateToRunning(
    unc_key_type_t keytype, CtrlrCommitStatusList *ctrlr_commit_status,
    DalDmlIntf *dmi, TcConfigMode config_mode, string vtn_name) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  DalResultCode db_result;
  unc_keytype_operation_t op[] = { UNC_OP_DELETE, UNC_OP_CREATE,
    UNC_OP_UPDATE };
  int nop = sizeof(op) / sizeof(op[0]);
  ConfigKeyVal *vtn_key = NULL, *req = NULL, *nreq = NULL,
               *vtn_ck_run = NULL;
  DalCursor *cfg1_cursor = NULL;
  uint8_t *ctrlr_id = NULL;
  /*IpcReqRespHeader *req_header = reinterpret_cast<IpcReqRespHeader *>
    (ConfigKeyVal::Malloc(sizeof(IpcReqRespHeader)));
    */
  map<string, int> ctrlr_result;
  CtrlrCommitStatusList::iterator ccsListItr;
  CtrlrCommitStatus *ccStatusPtr;

  // mode is virtual and so ignore it
  if (config_mode == TC_CONFIG_VIRTUAL) {
    return UPLL_RC_SUCCESS;
  }

  if (ctrlr_commit_status != NULL) {
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
  }

  for (int i = 0; i < nop; i++) {
    cfg1_cursor = NULL;
    if (op[i] != UNC_OP_UPDATE) {
      result_code = DiffConfigDB(UPLL_DT_CANDIDATE, UPLL_DT_RUNNING, op[i], req,
          nreq, &cfg1_cursor, dmi, NULL, config_mode, vtn_name, MAINTBL, true);
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
          DELETE_IF_NOT_NULL(req);
          DELETE_IF_NOT_NULL(nreq);
          dmi->CloseCursor(cfg1_cursor, true);
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
    cfg1_cursor = NULL;
    MoMgrTables tbl = (op[i] == UNC_OP_UPDATE)?MAINTBL:CTRLRTBL;
    result_code = DiffConfigDB(UPLL_DT_CANDIDATE, UPLL_DT_RUNNING, op[i], req,
                               nreq, &cfg1_cursor, dmi, NULL, config_mode,
                               vtn_name, tbl, true);

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
        DbSubOp dbop = { kOpReadMultiple, kOpMatchNone, kOpInOutCtrlr |
                         kOpInOutDomain | kOpInOutCs };
        result_code = GetChildConfigKey(vtn_ctrlr_key, req);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("GetChildConfigKey is failed resultcode=%d",
              result_code);
          DELETE_IF_NOT_NULL(req);
          DELETE_IF_NOT_NULL(nreq);
          dmi->CloseCursor(cfg1_cursor, true);
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
          DELETE_IF_NOT_NULL(vtn_ctrlr_key);
          if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
            result_code = UpdateMainTbl(req, op[i], UPLL_RC_SUCCESS,
                                        nreq, dmi);
            if (result_code != UPLL_RC_SUCCESS) {
              UPLL_LOG_DEBUG("Error updating main table%d", result_code);
              DELETE_IF_NOT_NULL(req);
              DELETE_IF_NOT_NULL(nreq);
              dmi->CloseCursor(cfg1_cursor, true);
              return result_code;
            } else {
              continue;
            }
          } else  {
            UPLL_LOG_DEBUG("DB err while reading records from ctrlrtbl, err %d",
                result_code);
            DELETE_IF_NOT_NULL(req);
            DELETE_IF_NOT_NULL(nreq);
            dmi->CloseCursor(cfg1_cursor, true);
            return result_code;
          }
        }

        for (ConfigKeyVal *tmp = vtn_ctrlr_key; tmp != NULL; tmp =
             tmp->get_next_cfg_key_val()) {
          GET_USER_DATA_CTRLR(tmp, ctrlr_id);
          string controller(reinterpret_cast<char *> (ctrlr_id));

          UPLL_LOG_DEBUG("Controller ID =%s", controller.c_str());
          DbSubOp dbop_maintbl = { kOpReadSingle, kOpMatchNone, kOpInOutFlag };
          result_code = GetChildConfigKey(vtn_key, req);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("GetChildConfigKey is failed resultcode=%d",
                result_code);
            DELETE_IF_NOT_NULL(req);
            DELETE_IF_NOT_NULL(nreq);
            dmi->CloseCursor(cfg1_cursor, true);
            return result_code;
          }
          result_code = ReadConfigDB(vtn_key, UPLL_DT_CANDIDATE,
                                     UNC_OP_READ, dbop_maintbl, dmi, MAINTBL);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("Unable to read configuration from CandidateDb");
            DELETE_IF_NOT_NULL(vtn_key);
            DELETE_IF_NOT_NULL(req);
            DELETE_IF_NOT_NULL(nreq);
            dmi->CloseCursor(cfg1_cursor, true);
            return result_code;
          }
          if (ctrlr_result.empty()) {
            UPLL_LOG_TRACE("ctrlr_commit_status is NULL");
            result_code = UpdateConfigStatus(vtn_key, op[i],
                                             UPLL_RC_ERR_CTR_DISCONNECTED, nreq,
                                             dmi, tmp);
          } else {
            result_code = UpdateConfigStatus(vtn_key, op[i],
                                             ctrlr_result[controller], nreq,
                                             dmi, tmp);
          }
          if (result_code != UPLL_RC_SUCCESS) break;

          result_code = UpdateConfigDB(tmp,
                                       UPLL_DT_RUNNING, op[i], dmi,
                                       config_mode, vtn_name, CTRLRTBL);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("UpdateConfigDB for ctrlr tbl is failed ");
            DELETE_IF_NOT_NULL(vtn_ctrlr_key);
            DELETE_IF_NOT_NULL(vtn_key);
            DELETE_IF_NOT_NULL(req);
            DELETE_IF_NOT_NULL(nreq);
            dmi->CloseCursor(cfg1_cursor, true);
            return result_code;
          }
          result_code = UpdateConfigDB(vtn_key, UPLL_DT_RUNNING,
                                       op[i], dmi, config_mode, vtn_name,
                                       MAINTBL);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("UpdateConfigDB for main tbl is failed");
            DELETE_IF_NOT_NULL(vtn_ctrlr_key);
            DELETE_IF_NOT_NULL(vtn_key);
            DELETE_IF_NOT_NULL(req);
            DELETE_IF_NOT_NULL(nreq);
            dmi->CloseCursor(cfg1_cursor, true);
            return result_code;
          }  // COV UNREACHABLE
#if 0
          EnqueCfgNotification(op[i], UPLL_DT_RUNNING,
                               vtn_ctrlr_key);
#endif
          EnqueCfgNotification(op[i], UPLL_DT_RUNNING,
                               vtn_key);
          DELETE_IF_NOT_NULL(vtn_key);
        }
      } else {
        if (op[i] == UNC_OP_CREATE) {
          DbSubOp dbop = { kOpReadSingle, kOpMatchNone,
            kOpInOutFlag |kOpInOutCs };
          result_code = GetChildConfigKey(vtn_key, req);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("GetChildConfigKey is failed -%d", result_code);
            DELETE_IF_NOT_NULL(req);
            DELETE_IF_NOT_NULL(nreq);
            dmi->CloseCursor(cfg1_cursor, true);
            return result_code;
          }
          result_code = ReadConfigDB
              (vtn_key, UPLL_DT_RUNNING /*UPLL_DT_CANDIDATE*/,
              UNC_OP_READ, dbop, dmi, MAINTBL);
          if ((result_code != UPLL_RC_SUCCESS) &&
              (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE)) {
            UPLL_LOG_DEBUG("ReadConfigDB is failed -%d", result_code);
            DELETE_IF_NOT_NULL(vtn_key);
            DELETE_IF_NOT_NULL(req);
            DELETE_IF_NOT_NULL(nreq);
            dmi->CloseCursor(cfg1_cursor, true);
            return result_code;
          }
          /* set consolidated config status to UNKNOWN to init vtn cs_status
           * to the cs_status of first controller
           */
          uint32_t cur_instance_count;
          result_code = GetInstanceCount(vtn_key, NULL,
              UPLL_DT_CANDIDATE, &cur_instance_count,
              dmi, CTRLRTBL);
          if (UPLL_RC_SUCCESS != result_code) {
            UPLL_LOG_DEBUG("GetInstanceCount failed %d", result_code);
            DELETE_IF_NOT_NULL(vtn_key);
            DELETE_IF_NOT_NULL(req);
            DELETE_IF_NOT_NULL(nreq);
            dmi->CloseCursor(cfg1_cursor, true);
            return result_code;
          }
          if (cur_instance_count == 1)
            reinterpret_cast<val_policingmap*>(GetVal(vtn_key))->cs_row_status
              = UNC_CS_UNKNOWN;

          /* Capability check
           * req_header->operation = op[i];
           * strcpy((char*)req_header->datatype, (char*)UNC_DT_CANDIDATE);
           * result_code = ValidateCapability(req_header, vtn_ctrlr_key);
           *                                                 */
          result_code = DupConfigKeyVal(vtn_ctrlr_key, req, tbl);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("DupConfigKeyVal is failed -%d", result_code);
            DELETE_IF_NOT_NULL(vtn_key);
            DELETE_IF_NOT_NULL(req);
            DELETE_IF_NOT_NULL(nreq);
            dmi->CloseCursor(cfg1_cursor, true);
            return result_code;
          }

          GET_USER_DATA_CTRLR(vtn_ctrlr_key, ctrlr_id);
          string controller(reinterpret_cast<char *>(ctrlr_id));
          if (ctrlr_result.empty()) {
            UPLL_LOG_TRACE("ctrlr_commit_status is NULL");
            result_code = UpdateConfigStatus(vtn_key, op[i],
                UPLL_RC_ERR_CTR_DISCONNECTED, nreq,
                dmi, vtn_ctrlr_key);
          } else {
            result_code = UpdateConfigStatus(vtn_key, op[i],
                ctrlr_result[controller], nreq,
                dmi, vtn_ctrlr_key);
          }
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("Error in updating configstatus, resultcode=%d",
                           result_code);
            DELETE_IF_NOT_NULL(vtn_ctrlr_key);
            DELETE_IF_NOT_NULL(vtn_key);
            DELETE_IF_NOT_NULL(req);
            DELETE_IF_NOT_NULL(nreq);
            dmi->CloseCursor(cfg1_cursor, true);
            return result_code;
          }
        } else if (op[i] == UNC_OP_DELETE) {
          // Reading Main Running DB for delete op
          DbSubOp dbop1 = { kOpReadSingle, kOpMatchNone,
            kOpInOutFlag | kOpInOutCs };
          result_code = GetChildConfigKey(vtn_ck_run, req);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("GetChildConfigKey is failed resultcode=%d",
                result_code);
            DELETE_IF_NOT_NULL(req);
            DELETE_IF_NOT_NULL(nreq);
            dmi->CloseCursor(cfg1_cursor, true);
            return result_code;
          }
          result_code = ReadConfigDB(vtn_ck_run, UPLL_DT_RUNNING,
              UNC_OP_READ, dbop1, dmi, MAINTBL);
          if (result_code != UPLL_RC_SUCCESS &&
              result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
            UPLL_LOG_DEBUG("Unable to read configuration from Running");
            DELETE_IF_NOT_NULL(vtn_ck_run);
            DELETE_IF_NOT_NULL(req);
            DELETE_IF_NOT_NULL(nreq);
            dmi->CloseCursor(cfg1_cursor, true);
            return result_code;
          }
          if (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
            GET_USER_DATA_CTRLR(req, ctrlr_id);
            result_code = SetVtnPmConsolidatedStatus(vtn_ck_run, ctrlr_id, dmi);
            if (result_code != UPLL_RC_SUCCESS) {
              UPLL_LOG_DEBUG("Could not set consolidated status %d",
                             result_code);
              DELETE_IF_NOT_NULL(vtn_ck_run);
              DELETE_IF_NOT_NULL(req);
              DELETE_IF_NOT_NULL(nreq);
              dmi->CloseCursor(cfg1_cursor, true);
              return result_code;
            }
          }
          DELETE_IF_NOT_NULL(vtn_ck_run);
          result_code = GetChildConfigKey(vtn_ctrlr_key, req);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("Error in getting the configkey, resultcode=%d",
                result_code);
            DELETE_IF_NOT_NULL(req);
            DELETE_IF_NOT_NULL(nreq);
            dmi->CloseCursor(cfg1_cursor, true);
            return result_code;
          }
        }
        result_code = UpdateConfigDB(vtn_ctrlr_key, UPLL_DT_RUNNING,
                                     op[i], dmi, config_mode, vtn_name,
                                     CTRLRTBL);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("UpdateConfigDB in ctrlr tbl is failed -%d",
                         result_code);
          DELETE_IF_NOT_NULL(vtn_ctrlr_key);
          DELETE_IF_NOT_NULL(req);
          DELETE_IF_NOT_NULL(nreq);
          dmi->CloseCursor(cfg1_cursor, true);
          return result_code;
        }
        if (op[i] != UNC_OP_DELETE) {
          result_code = UpdateConfigDB(vtn_key, UPLL_DT_RUNNING,
                                       UNC_OP_UPDATE, dmi, config_mode,
                                       vtn_name, MAINTBL);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("UpdateConfigDB in main tbl is failed -%d",
                result_code);
            DELETE_IF_NOT_NULL(req);
            DELETE_IF_NOT_NULL(nreq);
            DELETE_IF_NOT_NULL(vtn_key);
            dmi->CloseCursor(cfg1_cursor, true);
            return result_code;
          }
        }
        EnqueCfgNotification(op[i], UPLL_DT_RUNNING,
                             vtn_key);
      }
      DELETE_IF_NOT_NULL(vtn_key);
      DELETE_IF_NOT_NULL(vtn_ctrlr_key);
      result_code = DalToUpllResCode(db_result);
    }
    if (cfg1_cursor) {
      dmi->CloseCursor(cfg1_cursor, true);
      cfg1_cursor = NULL;
    }
    DELETE_IF_NOT_NULL(nreq);
    DELETE_IF_NOT_NULL(req);
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
  unc_keytype_configstatus_t  ctrlr_status;
  uint8_t cs_status;
  ctrlr_status = (driver_result == 0)?UNC_CS_APPLIED: UNC_CS_NOT_APPLIED;
  val_policingmap_t *pm_val =
    reinterpret_cast<val_policingmap_t *>(GetVal(ckv));
  val_vtnpolicingmap_ctrl_t *ctrlr_val_pm =
    reinterpret_cast<val_vtnpolicingmap_ctrl_t *>(GetVal(ctrlr_key));
  if ((pm_val == NULL) || (ctrlr_val_pm == NULL)) {
    UPLL_LOG_DEBUG("Memory Allocation failed for Valstructure");
    return UPLL_RC_ERR_GENERIC;
  }
  cs_status = (pm_val->cs_row_status);
  UPLL_LOG_TRACE("cs_status %d ctrlr_status %d\n", cs_status, ctrlr_status);
  if (op == UNC_OP_CREATE) {
    ctrlr_val_pm->cs_row_status = ctrlr_status;
    ctrlr_val_pm->cs_attr[0] = ctrlr_status;
    /* update the vtn status in main tbl */
    if (pm_val->cs_row_status == UNC_CS_UNKNOWN) {
        /* first entry in ctrlr table */
      cs_status = ctrlr_status;
    } else if (pm_val->cs_row_status == UNC_CS_INVALID) {
      cs_status = UNC_CS_INVALID;
    } else if (pm_val->cs_row_status == UNC_CS_APPLIED) {
        if (ctrlr_status == UNC_CS_NOT_APPLIED) {
          cs_status = UNC_CS_PARTIALLY_APPLIED;
        }
    } else if (pm_val->cs_row_status == UNC_CS_NOT_APPLIED) {
        if (ctrlr_status == UNC_CS_APPLIED) {
          cs_status =  UNC_CS_PARTIALLY_APPLIED;
        }
    } else {
        cs_status = UNC_CS_PARTIALLY_APPLIED;
    }
    pm_val->cs_row_status = cs_status;
    pm_val->cs_attr[0] = cs_status;
  }
  // Updating the Controller cs_row_status
  if ((op == UNC_OP_UPDATE) && (nreq != NULL)) {
      val_vtnpolicingmap_ctrl *run_ctrlr_val =
                   reinterpret_cast<val_vtnpolicingmap_ctrl*>
                                       (GetVal(nreq));
    if (run_ctrlr_val != NULL)
      ctrlr_val_pm->cs_row_status = run_ctrlr_val->cs_row_status;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtnPolicingMapMoMgr::ValidateReadAttribute(ConfigKeyVal *ikey,
    DalDmlIntf *dmi,
    IpcReqRespHeader *req) {
  UPLL_FUNC_TRACE;
  ConfigKeyVal* vtnkey = NULL;
  upll_rc_t result_code;
  if (UNC_OPT1_DETAIL == req->option1 &&
      (UNC_OP_READ == req->operation ||
       UNC_OP_READ_SIBLING == req->operation ||
       UNC_OP_READ_SIBLING_BEGIN == req->operation)) {
    ConfigKeyVal *okey = NULL;
    // opt1 =detail supported for KT_UNC_VTN_POLICINGMAP_CONTROLLER
      key_vtn_policingmap_controller *l_key =
          reinterpret_cast<key_vtn_policingmap_controller*>(ikey->get_key());

      key_vtn_t *vtn_key = reinterpret_cast<key_vtn_t *>
          (ConfigKeyVal::Malloc(sizeof(key_vtn)));
      uuu::upll_strncpy(vtn_key->vtn_name,
                        l_key->vtn_key.vtn_name,
                        (kMaxLenVtnName+1));

      vtnkey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn,
                                vtn_key, NULL);
      DbSubOp dbop1 = { kOpReadExist, kOpMatchNone, kOpInOutNone};
      result_code = UpdateConfigDB(vtnkey, req->datatype,
                                   UNC_OP_READ, dmi, &dbop1, MAINTBL);
      if (result_code != UPLL_RC_ERR_INSTANCE_EXISTS) {
        UPLL_LOG_DEBUG("PolicingMap does not exists in main table -%d",
                       result_code);
        DELETE_IF_NOT_NULL(vtnkey);
        return result_code;
      }
      DELETE_IF_NOT_NULL(vtnkey);

      result_code = CopyVtnControllerCkv(ikey, okey);
      if (UPLL_RC_SUCCESS != result_code) {
        UPLL_LOG_DEBUG("CopyVtnControllerCkv failed %d", result_code);
        DELETE_IF_NOT_NULL(okey);
        return result_code;
      }

     /** For READ operation match controller and domain name */
     if (UNC_OP_READ == req->operation)
       dbop1.matchop = kOpMatchCtrlr | kOpMatchDomain;

    result_code = UpdateConfigDB(okey, req->datatype, UNC_OP_READ,
                                 dmi, &dbop1, CTRLRTBL);
    if (result_code != UPLL_RC_ERR_INSTANCE_EXISTS) {
      DELETE_IF_NOT_NULL(okey);
      if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
        UPLL_LOG_DEBUG("PolicingMap does not exists in ctrlrtble- %d",
                       result_code);
        return UPLL_RC_ERR_NOT_ALLOWED_AT_THIS_TIME;
      }
      UPLL_LOG_DEBUG("UpdateConfigDB failed %d", result_code);
      return result_code;
    }
    DELETE_IF_NOT_NULL(okey);
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtnPolicingMapMoMgr::ValidateAttribute(ConfigKeyVal *ikey,
    DalDmlIntf *dmi,
    IpcReqRespHeader *req) {
  // No operation
  return UPLL_RC_SUCCESS;
}

bool VtnPolicingMapMoMgr::CompareValidValue(void *&val1, void *val2,
    bool audit) {
  UPLL_FUNC_TRACE;
  bool invalid_attr = true;
  val_policingmap_t *val_pm1 = reinterpret_cast<val_policingmap_t *>(val1);
  val_policingmap_t *val_pm2 = reinterpret_cast<val_policingmap_t *>(val2);

  if (UNC_VF_INVALID == val_pm1->valid[UPLL_IDX_POLICERNAME_PM] &&
      UNC_VF_VALID == val_pm2->valid[UPLL_IDX_POLICERNAME_PM]) {
    val_pm1->valid[UPLL_IDX_POLICERNAME_PM] = UNC_VF_VALID_NO_VALUE;
  }

  if (UNC_VF_VALID == val_pm1->valid[UPLL_IDX_POLICERNAME_PM]
      && UNC_VF_VALID == val_pm2->valid[UPLL_IDX_POLICERNAME_PM]) {
    if (!strcmp(reinterpret_cast<char*>(val_pm1->policer_name),
          reinterpret_cast<char*>(val_pm2->policer_name)))
      val_pm1->valid[UPLL_IDX_POLICERNAME_PM] = UNC_VF_INVALID;
  }
  if ((UNC_VF_VALID == (uint8_t)val_pm1->valid[UPLL_IDX_POLICERNAME_PM]) ||
    (UNC_VF_VALID_NO_VALUE == (uint8_t)val_pm1->valid[UPLL_IDX_POLICERNAME_PM]))
    invalid_attr = false;
  return invalid_attr;
}

upll_rc_t VtnPolicingMapMoMgr::UpdateAuditConfigStatus(
    unc_keytype_configstatus_t cs_status,
    uuc::UpdateCtrlrPhase phase,
    ConfigKeyVal *&ckv_running,
    DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  val_vtnpolicingmap_ctrl *val;
  val =
    (ckv_running != NULL) ? reinterpret_cast<val_vtnpolicingmap_ctrl *>(GetVal(
          ckv_running)) :
    NULL;
  if (NULL == val) {
    UPLL_LOG_DEBUG("UpdateAuditConfigStatus vtn_pm_val NULL (%d)", result_code);
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
      DELETE_IF_NOT_NULL(tmp1);
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
  }

  if ((okey) && (okey->get_key())) {
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

  if ((okey) && !(okey->get_key())) {
    UPLL_LOG_DEBUG("okey not null and flow list name updated");
    okey->SetKey(IpctSt::kIpcStKeyVtn, vtn_key);
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
    if ((req->datatype == UPLL_DT_IMPORT) && (req->operation == UNC_OP_READ ||
         req->operation == UNC_OP_READ_SIBLING ||
         req->operation == UNC_OP_READ_SIBLING_BEGIN ||
         req->operation == UNC_OP_READ_NEXT ||
         req->operation == UNC_OP_READ_BULK ||
         req->operation == UNC_OP_READ_SIBLING_COUNT)) {
      return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT;
    }

    if (req->option1 != UNC_OPT1_NORMAL) {
      UPLL_LOG_DEBUG(" Invalid option1(%d)", req->option1);
      return UPLL_RC_ERR_INVALID_OPTION1;
    }

    if (req->option2 != UNC_OPT2_NONE) {
      UPLL_LOG_DEBUG(" Invalid option2(%d)", req->option2);
      return UPLL_RC_ERR_INVALID_OPTION2;
    }

    if (key->get_st_num() != IpctSt::kIpcStKeyVtn) {
      UPLL_LOG_DEBUG("Invalid key structure received. struct num - %d",
                     key->get_st_num());
      return UPLL_RC_ERR_BAD_REQUEST;
    }
    key_vtn = reinterpret_cast<key_vtn_t *>(key->get_key());
  } else if (UNC_KT_VTN_POLICINGMAP_CONTROLLER == key->get_key_type()) {
    if ((req->datatype == UPLL_DT_IMPORT) && (req->operation == UNC_OP_READ ||
         req->operation == UNC_OP_READ_SIBLING ||
         req->operation == UNC_OP_READ_SIBLING_BEGIN ||
         req->operation == UNC_OP_READ_NEXT ||
         req->operation == UNC_OP_READ_BULK ||
         req->operation == UNC_OP_READ_SIBLING_COUNT)) {
      return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT;
    }

    if (req->datatype != UPLL_DT_STATE) {
      UPLL_LOG_DEBUG(" Invalid Datatype(%d)", req->datatype);
      return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT;
    }

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
    key_vtn =  reinterpret_cast< key_vtn_t*>
        (&(key_vtn_policingmap_controller->vtn_key));

  if ((req->operation != UNC_OP_READ_SIBLING_COUNT) &&
      (req->operation != UNC_OP_READ_SIBLING_BEGIN)) {
    rt_code = ValidateKey(reinterpret_cast<char*>(
      key_vtn_policingmap_controller->controller_name),
        kMinLenCtrlrId, kMaxLenCtrlrId);

    if (rt_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Controllername syntax validation failed: Err code-%d",
                    rt_code);
      return rt_code;
    }
    if (!ValidateDefaultStr(key_vtn_policingmap_controller->domain_id,
        kMinLenDomainId, kMaxLenDomainId)) {
      UPLL_LOG_DEBUG("DomainId syntax validation failed:");
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  } else {
    memset(key_vtn_policingmap_controller->controller_name, 0, kMaxLenCtrlrId);
    memset(key_vtn_policingmap_controller->domain_id, 0, kMaxLenDomainId);
  }

  } else {
    UPLL_LOG_DEBUG(" Invalid keytype(%d)", key->get_key_type());
    return UPLL_RC_ERR_BAD_REQUEST;
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
      UPLL_LOG_DEBUG(" Vtn name syntax validation failed Err Code - %d",
                     rt_code);
    return rt_code;
  }

  if (UNC_KT_VTN_POLICINGMAP == key->get_key_type()) {
    UPLL_LOG_TRACE(" key struct validation is success");

    return ValidatePolicingMapValue(key, req->operation);
  }

  /** Validate UNC_KT_VTN_POLICINGMAP_CONTROLLER key, val structure */

  return ValidateVtnPolicingMapControllerValue(
         key, req);
}

upll_rc_t VtnPolicingMapMoMgr::ValidatePolicingMapValue(
    ConfigKeyVal *key, uint32_t operation) {
  UPLL_FUNC_TRACE;

  val_policingmap_t *val_policingmap = NULL;

  if (!key->get_cfg_val()) {
    if ((operation == UNC_OP_CREATE) || (operation == UNC_OP_UPDATE)) {
      UPLL_LOG_DEBUG(" val structure is mandatory");
      return UPLL_RC_ERR_BAD_REQUEST;
    } else {
      UPLL_LOG_TRACE("val stucture is optional");
      return UPLL_RC_SUCCESS;
    }
  }

  if (key->get_cfg_val()->get_st_num() != IpctSt::kIpcStValPolicingmap) {
    UPLL_LOG_DEBUG("Invalid val structure received. struct num - %d",
                 (key->get_cfg_val())->get_st_num());
    return UPLL_RC_ERR_BAD_REQUEST;
  }

  val_policingmap =
      reinterpret_cast<val_policingmap_t *>(key->get_cfg_val()->get_val());

  if (val_policingmap == NULL) {
    UPLL_LOG_DEBUG("KT_VTN_POLICINGMAP val structure is null");
    return UPLL_RC_ERR_BAD_REQUEST;
  }

  if ((val_policingmap->valid[UPLL_IDX_POLICERNAME_PM] == UNC_VF_INVALID) &&
       (operation != UNC_OP_UPDATE)) {
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
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtnPolicingMapMoMgr::ValidateVtnPolicingMapControllerValue(
    ConfigKeyVal *key, IpcReqRespHeader *req) {
  UPLL_FUNC_TRACE;

  if (!key->get_cfg_val()) {
    UPLL_LOG_TRACE("val stucture is optional");
    return UPLL_RC_SUCCESS;
  }

  if (key->get_cfg_val()->get_st_num() !=
      IpctSt::kIpcStValPolicingmapController) {
    UPLL_LOG_DEBUG("Invalid val structure received. struct num - %d",
                   (key->get_cfg_val())->get_st_num());
    return UPLL_RC_ERR_BAD_REQUEST;
  }

  val_policingmap_controller_t *val_policingmap_controller =
      reinterpret_cast<val_policingmap_controller_t *>
      (key->get_cfg_val()->get_val());

  if (NULL == val_policingmap_controller) {
    UPLL_LOG_TRACE(" Value structure is NULL");
    return UPLL_RC_ERR_BAD_REQUEST;
  }

  if (req->datatype != UPLL_DT_STATE) {
    UPLL_LOG_DEBUG("Error Unsupported datatype (%d)", req->datatype);
    return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT;
  }

  if (req->option2 != UNC_OPT2_NONE) {
    UPLL_LOG_DEBUG(" Error: option2 is not NONE");
    return UPLL_RC_ERR_INVALID_OPTION2;
  }

  if ((req->option1 != UNC_OPT1_NORMAL) &&
       ((req->option1 == UNC_OPT1_DETAIL)
       && (req->operation == UNC_OP_READ_SIBLING_COUNT))) {
    UPLL_LOG_DEBUG(" Error: option1 is not NORMAL/DETAIL");
    return UPLL_RC_ERR_INVALID_OPTION1;
  }

  if ((req->operation == UNC_OP_READ) ||
      (req->operation == UNC_OP_READ_SIBLING) ||
      (req->operation == UNC_OP_READ_SIBLING_COUNT) ||
      (req->operation == UNC_OP_READ_SIBLING_BEGIN)) {
    /** Validate sequence number*/
    if (val_policingmap_controller->valid[UPLL_IDX_SEQ_NUM_PMC]
        == UNC_VF_VALID) {
      /* validate name range is between 1 and 32 */
      if (!ValidateNumericRange(val_policingmap_controller->sequence_num,
            (uint8_t)kMinPolicingProfileSeqNum,
            (uint8_t)kMaxPolicingProfileSeqNum, true, true)) {
        UPLL_LOG_DEBUG(" Syntax range check failed for seq_num ");
        return UPLL_RC_ERR_CFG_SYNTAX;
      }
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

  if (NULL == ctrlr_name) {
    UPLL_LOG_DEBUG("ctrlr_name is NULL");
    return UPLL_RC_ERR_GENERIC;
  }

  bool result_code = false;
  uint32_t max_instance_count = 0;
  const uint8_t *attrs = NULL;
  uint32_t max_attrs = 0;

  UPLL_LOG_TRACE("ctrlr_name(%s), datatype : (%d)",
               ctrlr_name, req->datatype);

  switch (req->operation) {
    case UNC_OP_CREATE:
      result_code = GetCreateCapability(ctrlr_name, ikey->get_key_type(),
                                        &max_instance_count,
                                        &max_attrs, &attrs);
      break;
    case UNC_OP_UPDATE:
      result_code = GetUpdateCapability(ctrlr_name, ikey->get_key_type(),
                                        &max_attrs, &attrs);
      break;
    default:
      if (req->datatype == UPLL_DT_STATE)
         result_code = GetStateCapability(ctrlr_name, ikey->get_key_type(),
                                      &max_attrs, &attrs);
      else
        result_code = GetReadCapability(ctrlr_name, ikey->get_key_type(),
                                      &max_attrs, &attrs);
      break;
  }

  if (!result_code) {
    UPLL_LOG_DEBUG("keytype(%d) is not supported by controller(%s)"
        " for operation(%d)",
        ikey->get_key_type(), ctrlr_name, req->operation);
    return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
  }

  if (max_attrs > 0) {
    return ValVtnPolicingMapAttributeSupportCheck(ikey, attrs);
  } else {
    UPLL_LOG_DEBUG("Attribute list is empty for operation %d", req->operation);
    return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
  }
}

upll_rc_t VtnPolicingMapMoMgr::ValVtnPolicingMapAttributeSupportCheck(
    ConfigKeyVal *ikey,  const uint8_t* attrs) {
  UPLL_FUNC_TRACE;

  if (UNC_KT_VTN_POLICINGMAP_CONTROLLER == ikey->get_key_type()) {
    val_policingmap_controller_t *val_policingmap_controller =
      reinterpret_cast<val_policingmap_controller_t *>(GetVal(ikey));

    if (val_policingmap_controller) {
      if ((val_policingmap_controller->valid[UPLL_IDX_SEQ_NUM_PMC]
            == UNC_VF_VALID)
          || (val_policingmap_controller->valid[UPLL_IDX_SEQ_NUM_PMC]
            == UNC_VF_VALID_NO_VALUE)) {
        if (attrs[unc::capa::vtn_policingmap_controller::kCapSeqNum] == 0) {
          val_policingmap_controller->valid[UPLL_IDX_SEQ_NUM_PMC] =
            UNC_VF_NOT_SUPPORTED;
          UPLL_LOG_DEBUG("SeqNum attr is not supported by ctrlr");
        }
      }
    }
    return UPLL_RC_SUCCESS;
  }

  val_policingmap_t *val_policingmap =
    reinterpret_cast<val_policingmap_t *>(GetVal(ikey));

  if (val_policingmap) {
    if ((val_policingmap->valid[UPLL_IDX_POLICERNAME_PM] == UNC_VF_VALID)
        || (val_policingmap->valid[UPLL_IDX_POLICERNAME_PM]
          == UNC_VF_VALID_NO_VALUE)) {
      if (attrs[unc::capa::vtn_policingmap::kCapPolicername] == 0) {
        val_policingmap->valid[UPLL_IDX_POLICERNAME_PM] =
          UNC_VF_NOT_SUPPORTED;

        UPLL_LOG_DEBUG("Policername attr is not supported by ctrlr");
      }
    }
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtnPolicingMapMoMgr::GetParentConfigKey(ConfigKeyVal *&okey,
                                                  ConfigKeyVal *ikey) {
  UPLL_FUNC_TRACE;

  if (!ikey) {
    UPLL_LOG_DEBUG(" Input Key is NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  DELETE_IF_NOT_NULL(okey);
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

bool VtnPolicingMapMoMgr::IsValidKey(void *key, uint64_t index,
                                     MoMgrTables tbl) {
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
      val_policingmap_controller_st *val_ppe_st =
            reinterpret_cast<val_policingmap_controller_st *>
            (ConfigKeyVal::Malloc(sizeof(val_policingmap_controller_st)));
      memcpy(val_ppe_st, val_entry_st,
               sizeof(val_policingmap_controller_st));
      ikey->AppendCfgVal(IpctSt::kIpcStValPolicingmapControllerSt,
                         val_ppe_st);

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

      if (IpctSt::kIpcStValPolicingmapSwitchSt != temp_cfg_val->get_st_num()) {
        UPLL_LOG_DEBUG("No PolicingmapSwitchSt entries returned by driver");
        continue;
      }

      /* Fill controller and domain name to pss in
         GetVbrIfFromVExternal */
      controller_domain ctrlr_dom;
      ctrlr_dom.ctrlr = reinterpret_cast <uint8_t*>
          (ConfigKeyVal::Malloc(kMaxLenCtrlrId + 1));
      ctrlr_dom.domain = reinterpret_cast <uint8_t*>
          (ConfigKeyVal::Malloc(kMaxLenDomainId + 1));
      uuu::upll_strncpy(ctrlr_dom.ctrlr, vtn_pm_ctrlr_key->controller_name,
          kMaxLenCtrlrId + 1);
      uuu::upll_strncpy(ctrlr_dom.domain, vtn_pm_ctrlr_key->domain_id,
          kMaxLenDomainId + 1);
      while (IpctSt::kIpcStValPolicingmapSwitchSt ==
             temp_cfg_val->get_st_num()) {
        val_policingmap_switch_st_t *val_switch_st =
            reinterpret_cast<val_policingmap_switch_st_t*>
            (ConfigKeyVal::Malloc(sizeof(val_policingmap_switch_st_t)));
        val_policingmap_switch_st_t *drv_val_switch_st =
            reinterpret_cast<val_policingmap_switch_st_t*>
            (temp_cfg_val->get_val());
        memcpy(val_switch_st, drv_val_switch_st,
               sizeof(val_policingmap_switch_st_t));
        if (drv_val_switch_st->valid[UPLL_IDX_VNODE_IF_NAME_PMSS] ==
            UNC_VF_VALID) {
          ConfigKeyVal *vbrif_key_val = NULL;
          unc::upll::kt_momgr::VbrIfMoMgr *vbrifmgr =
              reinterpret_cast<unc::upll::kt_momgr::VbrIfMoMgr *>
              (const_cast<MoManager *>(GetMoManager(UNC_KT_VBR_IF)));
          if (NULL == vbrifmgr) {
            free(val_switch_st);
            FREE_IF_NOT_NULL(ctrlr_dom.ctrlr);
            FREE_IF_NOT_NULL(ctrlr_dom.domain);
            return UPLL_RC_ERR_GENERIC;
          }
          /* GetVbrIfFromVExternal() will be called only during
           * READ/READ_SIB/READ_SIB_BEGIN,
           * hence dt_type is always UPLL_DT_RUNNING
           */
          // Get the vtn_name from the request
          result_code = vbrifmgr->GetVbrIfFromVExternal(
              reinterpret_cast<key_vtn_policingmap_controller_t*>(
                  ikey->get_key())->vtn_key.vtn_name,
              drv_val_switch_st->vnode_if_name,
              vbrif_key_val,
              dmi, ctrlr_dom);
          if ((result_code != UPLL_RC_SUCCESS) &&
              (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE)) {
            UPLL_LOG_DEBUG("Get vBridge info failed err code (%d)",
                           result_code);
            if (val_switch_st) free(val_switch_st);
            DELETE_IF_NOT_NULL(vbrif_key_val);
            FREE_IF_NOT_NULL(ctrlr_dom.ctrlr);
            FREE_IF_NOT_NULL(ctrlr_dom.domain);
            return result_code;
          } else if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
            upll_rc_t ret_code = UPLL_RC_SUCCESS;
            UPLL_LOG_DEBUG("Get vbr info failed err code (%d)",
                                                    result_code);
            DbSubOp dbop = { kOpReadSingle, kOpMatchCtrlr | kOpMatchDomain,
                                kOpInOutNone };
            ConfigKeyVal *vterm_key_val = NULL;
            key_vterm_if   *vterm_key_if  = NULL;
            unc::upll::kt_momgr::VtermIfMoMgr *vtermifmgr =
            reinterpret_cast<unc::upll::kt_momgr::VtermIfMoMgr *>
              (const_cast<MoManager *>(GetMoManager(UNC_KT_VTERM_IF)));
            if (NULL == vtermifmgr) {
              free(val_switch_st);
              DELETE_IF_NOT_NULL(vbrif_key_val);
              FREE_IF_NOT_NULL(ctrlr_dom.ctrlr);
              FREE_IF_NOT_NULL(ctrlr_dom.domain);
              return UPLL_RC_ERR_GENERIC;
            }
             vterm_key_if = reinterpret_cast<key_vterm_if_t *>
                           (ConfigKeyVal::Malloc(sizeof(key_vterm_if_t)));
            uuu::upll_strncpy(vterm_key_if->vterm_key.vtn_key.vtn_name,
            vtn_pm_ctrlr_key->vtn_key.vtn_name, (kMaxLenVtnName + 1));

            uuu::upll_strncpy(vterm_key_if->vterm_key.vterminal_name,
                      drv_val_switch_st->vnode_if_name,
                      (kMaxLenVnodeName + 1));
            vterm_key_val = new ConfigKeyVal(UNC_KT_VTERM_IF,
                            IpctSt::kIpcStKeyVtermIf,
                            vterm_key_if, NULL);

            SET_USER_DATA_CTRLR(vterm_key_val,
                                (const char*)vtn_pm_ctrlr_key->controller_name);
            SET_USER_DATA_DOMAIN(vterm_key_val,
                                 (const char*)vtn_pm_ctrlr_key->domain_id);
            ret_code = vtermifmgr->ReadConfigDB(vterm_key_val, dt_type,
            UNC_OP_READ , dbop , dmi, MAINTBL);
            if ((ret_code != UPLL_RC_SUCCESS) &&
                (ret_code != UPLL_RC_ERR_NO_SUCH_INSTANCE)) {
              UPLL_LOG_DEBUG("Get vterm info failed err code (%d)",
                                                           ret_code);
              DELETE_IF_NOT_NULL(vterm_key_val);
              if (val_switch_st) free(val_switch_st);
              DELETE_IF_NOT_NULL(vbrif_key_val);
              FREE_IF_NOT_NULL(ctrlr_dom.ctrlr);
              FREE_IF_NOT_NULL(ctrlr_dom.domain);
              return ret_code;
            }
            if (ret_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
              key_vterm_if_t *key_vtermif = reinterpret_cast<key_vterm_if_t*>
               (vterm_key_val->get_key());
              val_switch_st->valid[UPLL_IDX_VNODE_NAME_PMSS] = UNC_VF_VALID;
              uuu::upll_strncpy(val_switch_st->vnode_name,
               key_vtermif->vterm_key.vterminal_name,
               (kMaxLenVnodeName + 1));
              val_switch_st->valid[UPLL_IDX_VNODE_IF_NAME_PMSS] = UNC_VF_VALID;
              uuu::upll_strncpy(val_switch_st->vnode_if_name,
              key_vtermif->if_name,
              (kMaxLenInterfaceName + 1));
            }
               UPLL_LOG_DEBUG("Get vterm info available");
               DELETE_IF_NOT_NULL(vterm_key_val);
          } else if (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
            key_vbr_if_t *key_vbrif = reinterpret_cast<key_vbr_if_t*>
              (vbrif_key_val->get_key());
            val_switch_st->valid[UPLL_IDX_VNODE_NAME_PMSS] = UNC_VF_VALID;
            uuu::upll_strncpy(val_switch_st->vnode_name,
                            key_vbrif->vbr_key.vbridge_name,
                            (kMaxLenVnodeName + 1));

            val_switch_st->valid[UPLL_IDX_VNODE_IF_NAME_PMSS] = UNC_VF_VALID;
            uuu::upll_strncpy(val_switch_st->vnode_if_name,
                            key_vbrif->if_name,
                            (kMaxLenInterfaceName + 1));
          }
          DELETE_IF_NOT_NULL(vbrif_key_val);
        }
        ikey->AppendCfgVal(IpctSt::kIpcStValPolicingmapSwitchSt,
                           val_switch_st);
        temp_cfg_val = temp_cfg_val->get_next_cfg_val();
        if (temp_cfg_val == NULL)
          break;
      }
      FREE_IF_NOT_NULL(ctrlr_dom.ctrlr);
      FREE_IF_NOT_NULL(ctrlr_dom.domain);
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
    DELETE_IF_NOT_NULL(main_ckv);
    return result_code;
  }
  result_code = GetChildConfigKey(ctrlr_ckv, main_ckv);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetChildConfigKey failed %d", result_code);
  }
  DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutNone };
  DbSubOp dbop1 = { kOpReadSingle, kOpMatchCtrlr, kOpInOutNone };

  /** For READ operation match controller and domain name */
  if (op == UNC_OP_READ)
    dbop1.matchop = kOpMatchCtrlr | kOpMatchDomain;

  result_code = ReadConfigDB(main_ckv, dt_type, UNC_OP_READ, dbop, dmi,
                             MAINTBL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("ReadConfigDB failed in maintbl %d", result_code);
    delete ctrlr_ckv;
    delete main_ckv;
    return result_code;
  }
  result_code = ReadConfigDB(ctrlr_ckv, dt_type, UNC_OP_READ, dbop1, dmi,
                             CTRLRTBL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("ReadConfigDB failed in ctrlrtbl %d", result_code);
    delete ctrlr_ckv;
    delete main_ckv;
    return result_code;
  }

  val_policingmap_t *tmp_val_main = reinterpret_cast<val_policingmap_t *>
      (GetVal(main_ckv));
  val_policingmap_t *val_main = reinterpret_cast<val_policingmap_t *>
      (ConfigKeyVal::Malloc(sizeof(val_policingmap_t)));
  memcpy(val_main, tmp_val_main, sizeof(val_policingmap_t));
  val_vtnpolicingmap_ctrl *val_ctrlr =
      reinterpret_cast<val_vtnpolicingmap_ctrl *>
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
  result_code = ppe_mgr->ReadPolicingProfileEntry(
      reinterpret_cast<const char *>(val_main->policer_name),
      in_val_vtn_ctrlr_pm->sequence_num,
      reinterpret_cast<const char *>(ctrlr_id),
      dmi, dt_type, ppe_ckv);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("ReadPolicingProfileEntry failed %d", result_code);
    FREE_IF_NOT_NULL(val_main);
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
  DELETE_IF_NOT_NULL(ctrlr_ckv);
  DELETE_IF_NOT_NULL(main_ckv);
  if (ppe_ckv) {
    delete ppe_ckv;
    ppe_ckv = NULL;
  }
  return UPLL_RC_SUCCESS;
}


upll_rc_t VtnPolicingMapMoMgr::ConstructReadSiblingNormalResponse(
    ConfigKeyVal *&ikey,
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
  if (!okey || UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetChildVtnCtrlrKey failed ");
    DELETE_IF_NOT_NULL(okey);
    FREE_IF_NOT_NULL(out_val_vtn_ctrlr_pm);
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
    DELETE_IF_NOT_NULL(okey);
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
  ConfigKeyVal *temp_ckv = NULL, *capa_key = NULL;
  result_code = GetChildVtnCtrlrKey(temp_ckv, ikey);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetChildVtnCtrlrKey failed %d", result_code);
    FREE_IF_NOT_NULL(ctrlr_dom.ctrlr);
    FREE_IF_NOT_NULL(ctrlr_dom.domain);
    return result_code;
  }
  val_policingmap_controller_t *out_val_pm_ctrlr =
      reinterpret_cast<val_policingmap_controller_t *>
      (ConfigKeyVal::Malloc(sizeof(val_policingmap_controller_t)));
  memcpy(out_val_pm_ctrlr, val_pm_ctrlr,
      sizeof(val_policingmap_controller_t));
  tmp1 = new ConfigVal(IpctSt::kIpcStValPolicingmapController,
      out_val_pm_ctrlr);
  temp_ckv->AppendCfgVal(tmp1);
  // Adding GetUncControllerKey
  result_code = GetRenamedControllerKey(temp_ckv, req->datatype, dmi,
                                            &ctrlr_dom);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetRenamedControllerKey Failed %d", result_code);
    DELETE_IF_NOT_NULL(temp_ckv);
    FREE_IF_NOT_NULL(ctrlr_dom.ctrlr);
    FREE_IF_NOT_NULL(ctrlr_dom.domain);
    return result_code;
  }

  // Adding capa for Read/ReadSibling/ReadSiblingBegin Commonly Here
  result_code = GetChildVtnCtrlrKey(capa_key, ikey);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetChildVtnCtrlrKey  failed for capa_key %d", result_code);
    DELETE_IF_NOT_NULL(temp_ckv);
    FREE_IF_NOT_NULL(ctrlr_dom.ctrlr);
    FREE_IF_NOT_NULL(ctrlr_dom.domain);
    return result_code;
  }
  ConfigVal *capa_tmp = NULL;
  val_policingmap_controller_t *capa_val_ctrlr =
      reinterpret_cast<val_policingmap_controller_t *>
      (ConfigKeyVal::Malloc(sizeof(val_policingmap_controller_t)));
  memcpy(capa_val_ctrlr, val_pm_ctrlr,
      sizeof(val_policingmap_controller_t));
  capa_tmp = new ConfigVal(IpctSt::kIpcStValPolicingmapController,
      capa_val_ctrlr);
  capa_key->AppendCfgVal(capa_tmp);
  result_code = ValidateCapability(
      req, capa_key,
      reinterpret_cast<const char *>(ctrlr_dom.ctrlr));
  if (result_code != UPLL_RC_SUCCESS) {
     UPLL_LOG_DEBUG("Key not supported by controller IN ReadSibling");
     DELETE_IF_NOT_NULL(capa_key);
     DELETE_IF_NOT_NULL(temp_ckv);
     FREE_IF_NOT_NULL(ctrlr_dom.ctrlr);
     FREE_IF_NOT_NULL(ctrlr_dom.domain);
     return result_code;
  }
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
    UPLL_LOG_INFO("SendReqToDriver failed for Key %d controller %s",
                   temp_ckv->get_key_type(),
                   reinterpret_cast<char *>(ctrlr_dom.ctrlr));
    DELETE_IF_NOT_NULL(capa_key);
    DELETE_IF_NOT_NULL(temp_ckv);
    FREE_IF_NOT_NULL(ctrlr_dom.ctrlr);
    FREE_IF_NOT_NULL(ctrlr_dom.domain);
    return ipc_response->header.result_code;
  }

  if (ipc_response->header.result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Driver response for Key %d controller %s result %d",
                   temp_ckv->get_key_type(), ctrlr_dom.ctrlr,
                   ipc_response->header.result_code);
    DELETE_IF_NOT_NULL(capa_key);
    DELETE_IF_NOT_NULL(temp_ckv);
    FREE_IF_NOT_NULL(ctrlr_dom.ctrlr);
    FREE_IF_NOT_NULL(ctrlr_dom.domain);
    return ipc_response->header.result_code;
  }
  // Check with Rakesh
  DELETE_IF_NOT_NULL(capa_key);
  DELETE_IF_NOT_NULL(temp_ckv);
  FREE_IF_NOT_NULL(ctrlr_dom.ctrlr);
  FREE_IF_NOT_NULL(ctrlr_dom.domain);
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtnPolicingMapMoMgr::ReadSiblingCount(IpcReqRespHeader *req,
    ConfigKeyVal* ikey,
    DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code;
  controller_domain ctrlr_dom;
  result_code = ValidateMessage(req, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("ValidateMessage failed result_code %d",
                      result_code);
        return result_code;
  }
  if (UNC_KT_VTN_POLICINGMAP == ikey->get_key_type()) {
      if (req->datatype == UPLL_DT_STATE ||
         req->datatype == UPLL_DT_STARTUP ||
           req->datatype == UPLL_DT_RUNNING ||
             req->datatype == UPLL_DT_CANDIDATE ) {
        result_code = ReadInfoFromDB(req, ikey, dmi, &ctrlr_dom);
        return result_code;
      } else {
        UPLL_LOG_DEBUG("ReadSiblingCount is not Allowed For Such datatype %d",
        req->datatype);
        return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT;
      }
  }
  if (UNC_KT_VTN_POLICINGMAP_CONTROLLER == ikey->get_key_type())
    if (req->datatype != UPLL_DT_STATE) {
    UPLL_LOG_DEBUG("ReadSiblingCount is not Allowed For Such datatype %d",
       req->datatype);
    return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT;
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

upll_rc_t VtnPolicingMapMoMgr::CreateAuditMoImpl(ConfigKeyVal *ikey,
                                   DalDmlIntf *dmi,
                                   const char *ctrlr_id) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  string vtn_id = "";

  if (NULL == ikey || NULL == dmi) {
    UPLL_LOG_DEBUG("ikey or dmi NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  UPLL_LOG_TRACE("%s Vtn_Pm CreateAuditMoImpl ikey",
                    ikey->ToStrAll().c_str());
  // bool ctrl_instance = true;


  // Check Policingmap object exists in PolicingProfileTbl CANDIDATE DB
  // If record not exists, return error code
  val_policingmap_t *val_pm = reinterpret_cast<val_policingmap_t *>
    (GetVal(ikey));
  if (NULL == val_pm) {
    UPLL_LOG_DEBUG(" Value structure is NULL");
    return UPLL_RC_ERR_GENERIC;
  }

  if (UNC_VF_VALID == val_pm->valid[UPLL_IDX_POLICERNAME_PM]) {
    result_code = UpdateRefCountInPPCtrlr(ikey, UPLL_DT_AUDIT, dmi,
        UNC_OP_CREATE, TC_CONFIG_GLOBAL, vtn_id);
    if (UPLL_RC_SUCCESS != result_code) {
      if (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
        UPLL_LOG_DEBUG("UpdateRefCountInPPCtrlr Err in CANDIDATE DB(%d)",
          result_code);
        return result_code;
      }
      // ctrl_instance = false;
    }
  }

  result_code = SetValidAudit(ikey);
  if (UPLL_RC_SUCCESS != result_code) {
    return result_code;
  }
  DbSubOp dbop = { kOpReadExist, kOpMatchNone, kOpInOutNone };
  result_code = UpdateConfigDB(ikey, UPLL_DT_AUDIT, UNC_OP_READ, dmi, &dbop,
     MAINTBL);
  if (UPLL_RC_ERR_INSTANCE_EXISTS != result_code &&
      UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
    UPLL_LOG_DEBUG("UpdateConfigDB failed ");
    return result_code;
  }
  if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
  // create a record in vtnpolicingmap CANDIDATE DB
    result_code = UpdateConfigDB(ikey, UPLL_DT_AUDIT, UNC_OP_CREATE, dmi,
       TC_CONFIG_GLOBAL, vtn_id, MAINTBL);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("Create record Err in vtnpolicingmaptbl CANDIDATE DB(%d)",
          result_code);
      return result_code;
    }
  }
  key_vtn_t *tmp_key = reinterpret_cast<key_vtn_t *>(ikey->get_key());
  UPLL_LOG_DEBUG("vtn name in createcand %s", tmp_key->vtn_name);
  // Create the record in vtnpolicingmapctrltbl

  result_code = UpdateRecordInVtnPmCtrlr(ikey, UPLL_DT_AUDIT, UNC_OP_CREATE,
                                         dmi, TC_CONFIG_GLOBAL, NULL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("Create record Err in vtnpolicingmaptbl CANDIDATE DB(%d)",
      result_code);
    return result_code;
  }

  return result_code;
}

upll_rc_t VtnPolicingMapMoMgr::DeleteChildrenPOM(ConfigKeyVal *ikey,
    upll_keytype_datatype_t dt_type, DalDmlIntf *dmi,
    TcConfigMode config_mode, string vtn_name) {
  UPLL_FUNC_TRACE;
  if (NULL == ikey || NULL == dmi) {
    UPLL_LOG_DEBUG("DeleteMo ikey and req NULL");
    return UPLL_RC_ERR_GENERIC;
  }

  uint8_t *ctrlr_id = NULL;
  upll_rc_t result_code = UPLL_RC_ERR_GENERIC;
  int nattr = 0;
  BindInfo *binfo = NULL;
  DalCursor *dal_cursor_handle = NULL;
  string query_string;

  // 1)Decerment the refcount and check if the refcount as 0 then remove
  // the record and if refcount is not 0, then update the refcount in
  // policingprofilectrltbl

  // 2)Delete the record in policingprofileentryctrltbl

  key_vtn_t *pkey =
      reinterpret_cast<key_vtn_t*>(ikey->get_key());
  if (!pkey) {
    UPLL_LOG_DEBUG(" Input vtn policingmap  key is NULL ");
    return UPLL_RC_ERR_GENERIC;
  }

  result_code = GetFLPPCountQuery(ikey, UNC_KT_VTN, query_string);
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

  ConfigKeyVal *vtn_pm_ckv  = NULL;
  result_code = GetChildConfigKey(vtn_pm_ckv, NULL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetChildConfigKey failed");
    return result_code;
  }
  ConfigVal* vtn_pm_cv = NULL;
  result_code = AllocVal(vtn_pm_cv, UPLL_DT_CANDIDATE, MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Error in AllocVal");
    DELETE_IF_NOT_NULL(vtn_pm_ckv);
    return result_code;
  }
  vtn_pm_ckv->SetCfgVal(vtn_pm_cv);

  uint32_t count = 0;
  void *tkey = vtn_pm_ckv->get_key();
  void *p = reinterpret_cast<void *>(reinterpret_cast<char *>(tkey) +
                                     binfo[0].offset);
  dal_bind_info.BindOutput(binfo[0].index, binfo[0].app_data_type,
                           binfo[0].array_size, p);
  tkey = vtn_pm_ckv->get_cfg_val()->get_val();
  p = reinterpret_cast<void *>(reinterpret_cast<char *>(tkey) +
                               binfo[1].offset);
  dal_bind_info.BindOutput(binfo[1].index, binfo[1].app_data_type,
                           binfo[1].array_size, p);

  dal_bind_info.BindOutput(uud::schema::DAL_COL_STD_INTEGER,
                           uud::kDalUint32, 1, &count);

  // Get the vtn span
  ConfigKeyVal *okey = NULL;
  VtnMoMgr *vtnmgr =
    static_cast<VtnMoMgr *>((const_cast<MoManager *>
          (GetMoManager(UNC_KT_VTN))));
  result_code = vtnmgr->GetChildConfigKey(okey, NULL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("UpdateRefCountInPPCtrlr GetChildConfigKey error (%d)",
        result_code);
    return result_code;
  }
  if (!okey) {
    DELETE_IF_NOT_NULL(vtn_pm_ckv);
    return UPLL_RC_ERR_GENERIC;
  }
  // To get the vtn associated controller name
  key_vtn_t *vtn_okey = reinterpret_cast<key_vtn_t *>(okey->get_key());
  key_vtn_t *vtn_ikey = reinterpret_cast<key_vtn_t *>(ikey->get_key());
  uuu::upll_strncpy(vtn_okey->vtn_name, vtn_ikey->vtn_name,
      kMaxLenVtnName+1);
  result_code = vtnmgr->GetControllerDomainSpan(okey, dt_type, dmi);
  if (result_code != UPLL_RC_SUCCESS &&
      result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    UPLL_LOG_INFO("GetControllerSpan, err %d", result_code);
    DELETE_IF_NOT_NULL(vtn_pm_ckv);
    DELETE_IF_NOT_NULL(okey);
    return result_code;
  }
  if (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    ConfigKeyVal *temp_okey = okey;
    result_code = DalToUpllResCode(dmi->ExecuteAppQueryMultipleRecords(
                   query_string, 0, &dal_bind_info,
                   &dal_cursor_handle));
    while (result_code == UPLL_RC_SUCCESS) {
      result_code = DalToUpllResCode(dmi->GetNextRecord(dal_cursor_handle));
      if (UPLL_RC_SUCCESS == result_code) {
        // Call function to update refcount in scratch table
        key_vtn_t *vtn_pm_key =
            reinterpret_cast<key_vtn_t *>(vtn_pm_ckv->get_key());
        vtn_name = reinterpret_cast<const char *>(vtn_pm_key->vtn_name);
        val_policingmap_t *vtn_pm_val =
            reinterpret_cast<val_policingmap_t *>(GetVal(vtn_pm_ckv));

        PolicingProfileMoMgr *pp_mgr =
            reinterpret_cast<PolicingProfileMoMgr *>
            (const_cast<MoManager *>(GetMoManager(UNC_KT_POLICING_PROFILE)));
        if (NULL == pp_mgr) {
          UPLL_LOG_DEBUG("pp_mgr is NULL");
          DELETE_IF_NOT_NULL(vtn_pm_ckv);
          DELETE_IF_NOT_NULL(okey);
          return UPLL_RC_ERR_GENERIC;
        }
        ConfigKeyVal *pp_ckv  = NULL;
        result_code = pp_mgr->GetChildConfigKey(pp_ckv, NULL);
        if (UPLL_RC_SUCCESS != result_code) {
          UPLL_LOG_DEBUG("GetChildConfigKey failed");
          DELETE_IF_NOT_NULL(vtn_pm_ckv);
          return result_code;
        }
        key_policingprofile_t *pp_key =
            reinterpret_cast<key_policingprofile_t *>(pp_ckv->get_key());
        uuu::upll_strncpy(pp_key->policingprofile_name,
                          vtn_pm_val->policer_name,
                          (kMaxLenPolicingProfileName+1));

        // Call the update ref count function for each controller
        // in which the VTN is spanned across
        while (temp_okey != NULL) {
          // check for vnode_ref_cnt in vtn_ctrlr_tbl val structure.
          // If the count is '0' then continue for next vtn_ctrlr_tbl.
          val_vtn_ctrlr *ctr_val =
              reinterpret_cast<val_vtn_ctrlr *>(GetVal(temp_okey));
          if (!ctr_val) {
             UPLL_LOG_ERROR("Vtn controller table val structure is NULL");
            DELETE_IF_NOT_NULL(vtn_pm_ckv);
            DELETE_IF_NOT_NULL(okey);
            return UPLL_RC_ERR_GENERIC;
          }
          if (ctr_val->vnode_ref_cnt <= 0) {
            UPLL_LOG_DEBUG("skipping entry");
            temp_okey = temp_okey->get_next_cfg_key_val();
            continue;
          }
          // update refcount in scratch table
          unc_keytype_ctrtype_t ctrlrtype = UNC_CT_UNKNOWN;
          uuc::CtrlrMgr *ctrlr_mgr = uuc::CtrlrMgr::GetInstance();
          GET_USER_DATA_CTRLR(temp_okey, ctrlr_id);
          if ((!ctrlr_mgr->GetCtrlrType(reinterpret_cast<char *>(ctrlr_id),
                        dt_type, &ctrlrtype)) || (ctrlrtype != UNC_CT_PFC)) {
             UPLL_LOG_DEBUG("Controller type is  %d", ctrlrtype);
             continue;
          }
          SET_USER_DATA_CTRLR(pp_ckv, ctrlr_id);
          result_code = pp_mgr->UpdateRefCountInScratchTbl(pp_ckv, dmi,
                                                           dt_type,
                                                           UNC_OP_DELETE,
                                                           config_mode,
                                                           vtn_name,
                                                           count);
          if (result_code != UPLL_RC_SUCCESS &&
              result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
            UPLL_LOG_DEBUG("UpdateRefCountInScratchTbl returned %d",
                            result_code);
            DELETE_IF_NOT_NULL(vtn_pm_ckv);
            DELETE_IF_NOT_NULL(okey);
            DELETE_IF_NOT_NULL(pp_ckv);
            return result_code;
          } else if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
            result_code = pp_mgr->InsertRecInScratchTbl(pp_ckv, dmi, dt_type,
                                                        UNC_OP_DELETE,
                                                        config_mode, vtn_name,
                                                        count);
            if (UPLL_RC_SUCCESS != result_code) {
              UPLL_LOG_DEBUG("InsertRecInScratchTbl failed %d", result_code);
              DELETE_IF_NOT_NULL(vtn_pm_ckv);
              DELETE_IF_NOT_NULL(okey);
              DELETE_IF_NOT_NULL(pp_ckv);
              return result_code;
            }
          }
          temp_okey = temp_okey->get_next_cfg_key_val();
        }
        temp_okey = okey;
        DELETE_IF_NOT_NULL(pp_ckv);
      } else if (UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
        UPLL_LOG_DEBUG("GetNextRecord failed");
        DELETE_IF_NOT_NULL(okey);
        DELETE_IF_NOT_NULL(vtn_pm_ckv);
        dmi->CloseCursor(dal_cursor_handle, false);
        return result_code;
      }
    }
    if (result_code != UPLL_RC_SUCCESS &&
        result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      UPLL_LOG_DEBUG("ExecuteAppQueryMultipleRecords failed");
      DELETE_IF_NOT_NULL(okey);
      DELETE_IF_NOT_NULL(vtn_pm_ckv);
      dmi->CloseCursor(dal_cursor_handle, false);
      return result_code;
    }
  }
  DELETE_IF_NOT_NULL(okey);
  DELETE_IF_NOT_NULL(vtn_pm_ckv);

  /*
  DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutNone };
  result_code = ReadConfigDB(tempckv, dt_type, UNC_OP_READ, dbop, dmi,
      MAINTBL);
  if (UPLL_RC_SUCCESS != result_code) {
    if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
      UPLL_LOG_DEBUG("UPLL_RC_ERR_NO_SUCH_INSTANCE");
      DELETE_IF_NOT_NULL(tempckv);
      return UPLL_RC_SUCCESS;
    }
    UPLL_LOG_DEBUG(" ReadConfigDB failed ");
    DELETE_IF_NOT_NULL(tempckv);
    return result_code;
  }
  val_policingmap_t *val_pm = reinterpret_cast<val_policingmap_t *>
    (GetVal(tempckv));
  if (UNC_VF_VALID == val_pm->valid[UPLL_IDX_POLICERNAME_PM]) {
    result_code = UpdateRefCountInPPCtrlr(tempckv, dt_type, dmi,
        UNC_OP_DELETE, config_mode, vtn_name);
    if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
      result_code = UPLL_RC_SUCCESS;
    } else if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("DeleteMo returns (%d)",
          result_code);
      DELETE_IF_NOT_NULL(tempckv);
      return result_code;
    }
  }
  */
  ConfigKeyVal *temp_ikey = NULL;
  result_code = GetChildConfigKey(temp_ikey, ikey);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetChildConfigKey failed");
    dmi->CloseCursor(dal_cursor_handle, false);
    return result_code;
  }

  // Delete a record in vtnpolicingmap CANDIDATE DB
  result_code = UpdateConfigDB(temp_ikey, dt_type, UNC_OP_DELETE, dmi,
      config_mode, vtn_name, MAINTBL);
  DELETE_IF_NOT_NULL(temp_ikey);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("DeleteMo record Err in vtnpolicingmaptbl (%d)",
        result_code);
    dmi->CloseCursor(dal_cursor_handle, false);
    return result_code;
  }
  ConfigKeyVal *tempckv = NULL;
  result_code = GetChildConfigKey(tempckv, ikey);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG(" GetChildConfigKey failed");
    dmi->CloseCursor(dal_cursor_handle, false);
    return result_code;
  }

  DbSubOp dbop_update = {kOpNotRead, kOpMatchNone, kOpInOutNone};
  // Delete the record in vtnpolicingmapctrltbl
  result_code = UpdateConfigDB(tempckv, dt_type, UNC_OP_DELETE, dmi,
      &dbop_update, config_mode, vtn_name, CTRLRTBL);
  DELETE_IF_NOT_NULL(tempckv);
  if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
    result_code = UPLL_RC_SUCCESS;
  } else if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("DeleteMo record Err in vtnpolicingmapctrltbl (%d)",
        result_code);
    dmi->CloseCursor(dal_cursor_handle, false);
    return result_code;
  }

  dmi->CloseCursor(dal_cursor_handle, false);
  return result_code;
}

upll_rc_t VtnPolicingMapMoMgr::SetValidAudit(ConfigKeyVal *&ikey) {
  UPLL_FUNC_TRACE;
  val_policingmap_t *val = reinterpret_cast
      <val_policingmap_t *>(GetVal(ikey));
  if (NULL == val) {
    UPLL_LOG_DEBUG("val is NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  if (val->valid[0] == UNC_VF_VALID) {
    val->cs_attr[0] = UNC_CS_APPLIED;
  } else if (val->valid[0] == UNC_VF_INVALID) {
    val->cs_attr[0] = UNC_CS_NOT_APPLIED;
  }
  val->cs_row_status = UNC_CS_APPLIED;
  return UPLL_RC_SUCCESS;
}

bool VtnPolicingMapMoMgr::FilterAttributes(void *&val1,
                                          void *val2,
                                          bool copy_to_running,
                                          unc_keytype_operation_t op) {
  UPLL_FUNC_TRACE;
  if (op != UNC_OP_CREATE)
    return CompareValidValue(val1, val2, copy_to_running);
  return false;
}
upll_rc_t VtnPolicingMapMoMgr::IsRenamed(ConfigKeyVal *ikey,
                               upll_keytype_datatype_t dt_type,
                               DalDmlIntf *dmi,
                               uint8_t &rename) {
  UPLL_FUNC_TRACE;
  UPLL_LOG_DEBUG("VtnPolicingMapMoMgr IsRenamed");
  DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutFlag | kOpInOutCtrlr
                       | kOpInOutDomain };
  if (NULL == ikey) {
    UPLL_LOG_DEBUG("ikey NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  ConfigKeyVal *okey = NULL;
  upll_rc_t result_code;
  /* rename is set implies user wants the ikey
   * populated with val from db */
  if (!rename) {
    if (UNC_KT_VTN_POLICINGMAP == ikey->get_key_type()) {
      UPLL_LOG_DEBUG("UNC_KT_VTN_POLICINGMAP");
      result_code = GetChildConfigKey(okey, ikey);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_TRACE("Returning error %d", result_code);
        return result_code;
      }
    } else if (UNC_KT_VTN_POLICINGMAP_CONTROLLER == ikey->get_key_type()) {
      UPLL_LOG_DEBUG("UNC_KT_VTN_POLICINGMAP_CONTROLLER");

      key_vtn_t *out_key = reinterpret_cast<key_vtn_t *>
                   (ConfigKeyVal::Malloc(sizeof(key_vtn_t)));

      key_vtn_policingmap_controller_t *in_key = reinterpret_cast
        <key_vtn_policingmap_controller_t *>(ikey->get_key());

      uuu::upll_strncpy(out_key->vtn_name,
          in_key->vtn_key.vtn_name,
          (kMaxLenVtnName + 1));

      okey = new ConfigKeyVal(UNC_KT_VTN_POLICINGMAP,
                 IpctSt::kIpcStKeyVtn,
                 out_key, NULL);
    } else {
      UPLL_LOG_DEBUG("Invalid KeyType");
      return UPLL_RC_ERR_GENERIC;
    }
  } else {
    okey = ikey;
  }

  result_code = ReadConfigDB(okey, dt_type, UNC_OP_READ, dbop, dmi,
                                       MAINTBL);
  if ((result_code != UPLL_RC_SUCCESS) &&
       (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE))  {
    UPLL_LOG_DEBUG("Returning error code %d", result_code);
    if (okey != ikey) delete okey;
    return result_code;
  }
  GET_USER_DATA_FLAGS(okey, rename);
  controller_domain_t ctrlr_dom;
  ctrlr_dom.ctrlr = NULL;
  ctrlr_dom.domain = NULL;

  GET_USER_DATA_CTRLR_DOMAIN(ikey, ctrlr_dom);
    UPLL_LOG_DEBUG("Controller : %s; Domain : %s", ctrlr_dom.ctrlr,
                 ctrlr_dom.domain);
  SET_USER_DATA_CTRLR_DOMAIN(okey, ctrlr_dom);
    UPLL_LOG_DEBUG("Controller : %s; Domain : %s", ctrlr_dom.ctrlr,
                 ctrlr_dom.domain);

  SET_USER_DATA(ikey, okey);
  rename &= RENAME;
  if (okey != ikey) delete okey;
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtnPolicingMapMoMgr::SetRenameFlag(ConfigKeyVal *ikey,
    DalDmlIntf *dmi,
    IpcReqRespHeader *req) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  val_policingmap_t *val_pm = reinterpret_cast
    <val_policingmap_t *>(GetVal(ikey));
  if (!val_pm) {
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
              UNC_KT_VTN)));
    if (!mgr) {
      UPLL_LOG_DEBUG("mgr is NULL");
      DELETE_IF_NOT_NULL(pkey);
      return UPLL_RC_ERR_GENERIC;
    }
    uint8_t rename = 0;
    DbSubOp dbop = {kOpReadSingle, kOpMatchNone, kOpInOutFlag};
    result_code = mgr->ReadConfigDB(pkey, req->datatype,
                                  UNC_OP_READ, dbop, dmi, MAINTBL);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("ReadConfigDB failed %d", result_code);
      DELETE_IF_NOT_NULL(pkey);
      return result_code;
    }
    GET_USER_DATA_FLAGS(pkey, rename);
    DELETE_IF_NOT_NULL(pkey);
    // Check policingprofile is renamed
    if ((UNC_VF_VALID == val_pm->valid[UPLL_IDX_POLICERNAME_PM]) &&
        ((UNC_OP_CREATE == req->operation))) {
      ConfigKeyVal *pp_ckv = NULL;
      result_code = GetPolicingProfileConfigKey(reinterpret_cast<const char *>
          (val_pm->policer_name), pp_ckv, dmi);
      if (UPLL_RC_SUCCESS != result_code) {
        UPLL_LOG_DEBUG("GetPolicingProfileConfigKey failed %d", result_code);
        return result_code;
      }
      MoMgrImpl *pp_mgr =
      reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>(GetMoManager(
              UNC_KT_POLICING_PROFILE)));
      if (NULL == pp_mgr) {
        UPLL_LOG_DEBUG("pp_mgr is NULL");
        DELETE_IF_NOT_NULL(pp_ckv);
        return UPLL_RC_ERR_GENERIC;
      }
      uint8_t pp_rename = 0;
      result_code = pp_mgr->IsRenamed(pp_ckv, req->datatype, dmi, pp_rename);
      if (UPLL_RC_SUCCESS != result_code) {
        UPLL_LOG_DEBUG("IsRenamed failed %d", result_code);
        DELETE_IF_NOT_NULL(pp_ckv);
        return result_code;
      }
      if (pp_rename & 0x01) {
        rename |= POLICINGPROFILE_RENAME;
      }
      DELETE_IF_NOT_NULL(pp_ckv);
    }
    SET_USER_DATA_FLAGS(ikey, rename);
  } else if (UNC_OP_UPDATE == req->operation) {
    uint8_t rename = 0;
    result_code = IsRenamed(ikey, req->datatype, dmi, rename);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("IsRenamed failed %d", result_code);
      return result_code;
    }
    if (UNC_VF_VALID == val_pm->valid[UPLL_IDX_POLICERNAME_PM]) {
      ConfigKeyVal *pp_ckv = NULL;
      result_code = GetPolicingProfileConfigKey(reinterpret_cast<const char *>
          (val_pm->policer_name), pp_ckv, dmi);
      if (UPLL_RC_SUCCESS != result_code) {
        UPLL_LOG_DEBUG("GetPolicingProfileConfigKey failed %d", result_code);
        return result_code;
      }
      MoMgrImpl *pp_mgr =
      reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>(GetMoManager(
              UNC_KT_POLICING_PROFILE)));
      if (NULL == pp_mgr) {
        UPLL_LOG_DEBUG("pp_mgr is NULL");
        DELETE_IF_NOT_NULL(pp_ckv);
        return UPLL_RC_ERR_GENERIC;
      }
      uint8_t pp_rename = 0;
      result_code = pp_mgr->IsRenamed(pp_ckv, req->datatype, dmi, pp_rename);
      if (UPLL_RC_SUCCESS != result_code) {
        UPLL_LOG_DEBUG("IsRenamed failed %d", result_code);
        DELETE_IF_NOT_NULL(pp_ckv);
        return result_code;
      }
      if (pp_rename & 0x01) {
        rename |= POLICINGPROFILE_RENAME;
      } else {
        UPLL_LOG_DEBUG(" In else part rename %d ", rename);
        rename = rename & NO_POLICINGPROFILE_RENAME;
      }
      DELETE_IF_NOT_NULL(pp_ckv);
    } else if (UNC_VF_VALID_NO_VALUE == val_pm->valid
               [UPLL_IDX_POLICERNAME_PM]) {
       rename = rename & NO_POLICINGPROFILE_RENAME;
    }
    SET_USER_DATA_FLAGS(ikey, rename);
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtnPolicingMapMoMgr::GetPolicingProfileConfigKey(
        const char *pp_name, ConfigKeyVal *&okey,
        DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_ERR_GENERIC;
  MoMgrImpl *mgr =
      reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>(GetMoManager(
              UNC_KT_POLICING_PROFILE)));
  result_code = mgr->GetChildConfigKey(okey, NULL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetChildConfigKey failed %d", result_code);
    return result_code;
  }
  key_policingprofile_t *okey_key = reinterpret_cast<key_policingprofile_t *>
      (okey->get_key());
  uuu::upll_strncpy(okey_key->policingprofile_name,
        pp_name,
        (kMaxLenPolicingProfileName+1));
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtnPolicingMapMoMgr::SetVtnPmConsolidatedStatus(ConfigKeyVal *ikey,
                                                          uint8_t *ctrlr_id,
                                                          DalDmlIntf *dmi)  {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *ctrlr_ckv = NULL;
  val_vtnpolicingmap_ctrl_t *ctrlr_val = NULL;
  uint8_t *vtn_exist_on_ctrlr = NULL;
  bool applied = false, not_applied = false, invalid = false;
  unc_keytype_configstatus_t c_status = UNC_CS_NOT_APPLIED;
  string vtn_name = "";

  DbSubOp dbop = { kOpReadMultiple, kOpMatchNone,
                   kOpInOutCtrlr | kOpInOutDomain | kOpInOutCs };
  if (!ikey || !dmi) {
    UPLL_LOG_DEBUG("Invalid Input");
    return UPLL_RC_ERR_GENERIC;
  }
  result_code = GetChildConfigKey(ctrlr_ckv, ikey);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetChildConfigKey failed err code %d", result_code);
    return result_code;
  }
  result_code = ReadConfigDB(ctrlr_ckv, UPLL_DT_RUNNING, UNC_OP_READ, dbop, dmi,
                             CTRLRTBL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("ReadConfigDB from ctrltbl failed err code %d",
                   result_code);
    DELETE_IF_NOT_NULL(ctrlr_ckv);
    return result_code;
  }

  for (ConfigKeyVal *tmp = ctrlr_ckv; tmp != NULL;
                     tmp = tmp->get_next_cfg_key_val()) {
    ctrlr_val = reinterpret_cast<val_vtnpolicingmap_ctrl_t *>(GetVal(tmp));
    if (!ctrlr_val) {
      UPLL_LOG_DEBUG("Controller Value is empty");
      tmp = NULL;
      DELETE_IF_NOT_NULL(ctrlr_ckv);
      return UPLL_RC_ERR_GENERIC;
    }
    GET_USER_DATA_CTRLR(tmp, vtn_exist_on_ctrlr);
    if (!strcmp(reinterpret_cast<char *>(vtn_exist_on_ctrlr),
                reinterpret_cast<char *>(ctrlr_id)))
      continue;  // skipping entry of deleted controller

    switch (ctrlr_val->cs_row_status) {
      case UNC_CS_APPLIED:
        applied = true;
      break;
      case UNC_CS_NOT_APPLIED:
        not_applied = true;
      break;
      case UNC_CS_INVALID:
        invalid = true;
      break;
      default:
        UPLL_LOG_DEBUG("Invalid status");
        DELETE_IF_NOT_NULL(ctrlr_ckv);
        // return result_code;
    }
    vtn_exist_on_ctrlr = NULL;
  }
  if (invalid) {
    c_status = UNC_CS_INVALID;
  } else if (applied && !not_applied) {
    c_status = UNC_CS_APPLIED;
  } else if (!applied && not_applied) {
    c_status = UNC_CS_NOT_APPLIED;
  } else if (applied && not_applied) {
    c_status = UNC_CS_PARTIALLY_APPLIED;
  } else {
    c_status = UNC_CS_APPLIED;
  }
  // Set cs_status
  val_vtnpolicingmap_ctrl_t *val = static_cast<val_vtnpolicingmap_ctrl_t *>
                                  (GetVal(ikey));
  val->cs_row_status = c_status;
  val->cs_attr[0] = c_status;
  DbSubOp dbop_update = {kOpNotRead, kOpMatchNone, kOpInOutCs};
  result_code = UpdateConfigDB(ikey, UPLL_DT_RUNNING, UNC_OP_UPDATE, dmi,
                               &dbop_update, TC_CONFIG_GLOBAL, vtn_name,
                               MAINTBL);
  DELETE_IF_NOT_NULL(ctrlr_ckv);
  return result_code;
}

upll_rc_t VtnPolicingMapMoMgr::SetConsolidatedStatus(ConfigKeyVal *ikey,
                                               DalDmlIntf *dmi) {
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *ckv = NULL;
  string vtn_name = "";
  DbSubOp dbop = {kOpReadMultiple, kOpMatchNone, kOpInOutCs};
  result_code = GetChildConfigKey(ckv, ikey);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("Unable to Get the configKey");
    return result_code;
  }
  result_code = ReadConfigDB(ckv,
                             UPLL_DT_RUNNING,
                             UNC_OP_READ, dbop, dmi, CTRLRTBL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("Unable to read the configuration from CTRLR Table");
    if (ckv != NULL) {
      delete ckv;
      ckv = NULL;
    }
    return result_code;
  }
  std::list< unc_keytype_configstatus_t > list_cs_row;
  std::list< unc_keytype_configstatus_t > list_cs_attr;
  val_vtnpolicingmap_ctrl_t *val;
  ConfigKeyVal *temp_ckv = ckv;
  for ( ; temp_ckv != NULL ; temp_ckv = temp_ckv->get_next_cfg_key_val()) {
      val = reinterpret_cast<val_vtnpolicingmap_ctrl_t *>(GetVal(temp_ckv));
      list_cs_row.push_back((unc_keytype_configstatus_t)val->cs_row_status);
      list_cs_attr.push_back((unc_keytype_configstatus_t)val->cs_attr[0]);
  }
  DELETE_IF_NOT_NULL(ckv);
  val_policingmap_t *val_temp =
      reinterpret_cast<val_policingmap_t *>(GetVal(ikey));
  val_temp->cs_row_status = GetConsolidatedCsStatus(list_cs_row);
  val_temp->cs_attr[0] = GetConsolidatedCsStatus(list_cs_attr);
  result_code = UpdateConfigDB(ikey,
                               UPLL_DT_RUNNING,
                               UNC_OP_UPDATE, dmi, TC_CONFIG_GLOBAL, vtn_name,
                               MAINTBL);
  if (UPLL_RC_SUCCESS != result_code) {
    return result_code;
  }
  return result_code;
}

bool VtnPolicingMapMoMgr::IsAllAttrInvalid(
        val_policingmap_t *val) {
  UPLL_FUNC_TRACE;
  for ( unsigned int loop = 0;
      loop < sizeof(val->valid)/sizeof(val->valid[0]); ++loop ) {
    if (UNC_VF_INVALID != val->valid[loop])
      return false;
  }
  return true;
}

upll_rc_t VtnPolicingMapMoMgr::GetOperation(uuc::UpdateCtrlrPhase phase,
                                            unc_keytype_operation_t &op) {
  if (uuc::kUpllUcpDelete2 == phase) {
    UPLL_LOG_DEBUG("Delete phase 1");
    return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_KT;
  } else if (uuc::kUpllUcpUpdate == phase) {
    op = UNC_OP_UPDATE;
  } else if (uuc::kUpllUcpCreate == phase) {
    op = UNC_OP_CREATE;
  } else if (uuc::kUpllUcpDelete == phase) {
    op = UNC_OP_DELETE;
  } else if (uuc::kUpllUcpInit == phase) {
    return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_KT;
  } else {
    return UPLL_RC_ERR_GENERIC;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtnPolicingMapMoMgr::CreatePIForVtnPom(IpcReqRespHeader *req,
                                                 ConfigKeyVal *ikey,
                                                 DalDmlIntf *dmi,
                                                 const char *ctrlr_id) {
  UPLL_FUNC_TRACE;
  if (NULL == ikey || NULL == req || NULL == dmi) {
    UPLL_LOG_DEBUG("ikey or req or dmi NULL");
    return UPLL_RC_ERR_BAD_REQUEST;
  }
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *tmp_ckv = NULL;
  uint8_t flag = 0;

  result_code = GetRenamedUncKey(ikey, req->datatype, dmi,
                                reinterpret_cast<uint8_t *>(
                                const_cast<char *>(ctrlr_id)));
  if (UPLL_RC_SUCCESS != result_code &&
      UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
      UPLL_LOG_DEBUG("GetRenamedUncKey failed %d", result_code);
      return result_code;
  }

  result_code = ValidateAttribute(ikey, dmi, req);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("ValidateAttribute failed, Error - %d", result_code);
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
  }
  TcConfigMode config_mode = TC_CONFIG_INVALID;
  std::string vtn_name = "";
  result_code = GetConfigModeInfo(req, config_mode, vtn_name);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetConfigMode failed");
    return result_code;
  }

  if (UNC_VF_VALID == val_pm->valid[UPLL_IDX_POLICERNAME_PM]) {
    result_code = UpdateRefCountInPPCtrlrTbl(ikey, req->datatype, dmi,
        ctrlr_id, config_mode, vtn_name);
    if (UPLL_RC_SUCCESS != result_code) {
        UPLL_LOG_DEBUG("UpdateRefCountInPPCtrlr Err in CANDIDATE DB(%d)",
          result_code);
        return result_code;
    }
  }

  // create a record in vtnpolicingmap CANDIDATE DB
  result_code = UpdateConfigDB(ikey, req->datatype, req->operation, dmi,
      config_mode, vtn_name, MAINTBL);
  if (result_code == UPLL_RC_ERR_INSTANCE_EXISTS) {
    result_code = DupConfigKeyVal(tmp_ckv, ikey, MAINTBL);
    if (result_code != UPLL_RC_SUCCESS) {
       UPLL_LOG_DEBUG("DupConfigKeyVal failed");
       return result_code;
    }
    result_code = CompareValueStructure(tmp_ckv, req->datatype, dmi);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("CompareValueStructure failed");
      DELETE_IF_NOT_NULL(tmp_ckv);
      return result_code;
    }
    DELETE_IF_NOT_NULL(tmp_ckv);
  }

  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("Create record Err in vtnpolicingmaptbl CANDIDATE DB(%d)",
        result_code);
    return result_code;
  }

  GET_USER_DATA_FLAGS(ikey, flag);
  UPLL_LOG_DEBUG("flag value (%d)", flag);

  // Create the record in vtnpolicingmapctrltbl
  if (!(flag & VTN_RENAME)) {
    result_code = UpdateRecordInVtnPmCtrlr(ikey, req->datatype, UNC_OP_CREATE,
      dmi, config_mode, vtn_name);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("Create record Err in vtnpolicingmaptbl CANDIDATE DB(%d)",
      result_code);
      return result_code;
    }
  } else {
    result_code = UpdateRecordInVtnPmCtrlrTbl(ikey, req->datatype,
                                              UNC_OP_CREATE, dmi,
                                              config_mode, vtn_name);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("Create record Err in vtnpolicingmaptbl CANDIDATE DB(%d)",
      result_code);
      return result_code;
    }
  }
  return result_code;
}

upll_rc_t VtnPolicingMapMoMgr::CompareValueStructure(ConfigKeyVal *tmp_ckv,
                                          upll_keytype_datatype_t datatype,
                                          DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_ERR_GENERIC;
  ConfigKeyVal *imp_ckv = NULL;

  result_code = GetChildConfigKey(imp_ckv, tmp_ckv);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetChildConfigKey failed %d", result_code);
    return result_code;
  }
  DbSubOp dbop = {kOpReadSingle, kOpMatchNone, kOpInOutCtrlr | kOpInOutDomain};
  // Read the Configuration from the MainTable
  result_code = ReadConfigDB(imp_ckv, datatype,
                             UNC_OP_READ, dbop, dmi, MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
      DELETE_IF_NOT_NULL(imp_ckv);
      UPLL_LOG_DEBUG("ReadConfigDB err- vtn_ff_entry main tbl");
      return result_code;
  }
  result_code = CompareValStructure(GetVal(tmp_ckv), GetVal(imp_ckv));
  DELETE_IF_NOT_NULL(imp_ckv);
  return result_code;
}

upll_rc_t VtnPolicingMapMoMgr::CompareValStructure(void *val1,
                                                   void *val2) {
  UPLL_FUNC_TRACE;

  val_policingmap_t *val_pm1 = reinterpret_cast<val_policingmap_t *>(val1);
  val_policingmap_t *val_pm2 = reinterpret_cast<val_policingmap_t *>(val2);

  if (val_pm1->valid[0] != val_pm2->valid[0]) {
     UPLL_LOG_DEBUG("Valid flag mismatched");
     return UPLL_RC_ERR_MERGE_CONFLICT;
  }

  if (UNC_VF_VALID == val_pm1->valid[UPLL_IDX_POLICERNAME_PM]
      && UNC_VF_VALID == val_pm2->valid[UPLL_IDX_POLICERNAME_PM]) {
    if (strcmp(reinterpret_cast<char*>(val_pm1->policer_name),
          reinterpret_cast<char*>(val_pm2->policer_name))) {
      UPLL_LOG_DEBUG("profile name mismatched");
      return UPLL_RC_ERR_MERGE_CONFLICT;
    }
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtnPolicingMapMoMgr::UpdateRefCountInPPCtrlrTbl(
    ConfigKeyVal *ikey, upll_keytype_datatype_t dt_type, DalDmlIntf *dmi,
    const char *ctrlr_id, TcConfigMode config_mode, string vtn_name) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_ERR_GENERIC;

  uint8_t pp_flag = 0, pp_flag1 = 0;

  PolicingProfileMoMgr *pp_mgr =
    reinterpret_cast<PolicingProfileMoMgr *>
    (const_cast<MoManager *>(GetMoManager(UNC_KT_POLICING_PROFILE)));

  val_policingmap_t* val_vtn_policingmap =
    reinterpret_cast<val_policingmap_t *>(GetVal(ikey));

  if (UPLL_DT_IMPORT == dt_type) {
    UPLL_LOG_DEBUG("##### op1op1");
    GET_USER_DATA_FLAGS(ikey, pp_flag);
    if (pp_flag & POLICINGPROFILE_RENAME)
       pp_flag1 = PP_RENAME;
    UPLL_LOG_DEBUG("##### op1op1 flag %d", pp_flag);
    UPLL_LOG_DEBUG("##### op1op1 flag1 %d", pp_flag1);
  }

  UPLL_LOG_DEBUG("PP name in GetPolicingProfileCtrlrKeyval %s",
      val_vtn_policingmap->policer_name);
  UPLL_LOG_DEBUG("Ctrlrid in GetPolicingProfileCtrlrKeyval %s, %d",
      ctrlr_id, pp_flag);
  std::string temp_vtn_name;
  if (TC_CONFIG_VTN == config_mode) {
    temp_vtn_name = vtn_name;
  } else {
    temp_vtn_name = reinterpret_cast<const char *>(reinterpret_cast<key_vtn_t *>
                                                   (ikey->get_key())->
                                                   vtn_name);
  }
  result_code = pp_mgr->PolicingProfileCtrlrTblOper(
      reinterpret_cast<const char *>(val_vtn_policingmap->policer_name),
      reinterpret_cast<const char *>(ctrlr_id), dmi, UNC_OP_CREATE,
      dt_type, pp_flag1, config_mode, temp_vtn_name, 1, false);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("PolicingProfileCtrlrTblOper failed");
    return result_code;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtnPolicingMapMoMgr::UpdateRecordInVtnPmCtrlrTbl(
    ConfigKeyVal *ikey, upll_keytype_datatype_t dt_type,
    unc_keytype_operation_t op, DalDmlIntf *dmi,
    TcConfigMode config_mode, string vtn_name) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *ctrlckv = NULL;
  key_vtn_t *vtn_key = reinterpret_cast<key_vtn_t *>(ikey->get_key());
  if (NULL == vtn_key) {
    UPLL_LOG_DEBUG("key is NULL");
    return UPLL_RC_ERR_GENERIC;
  }

  controller_domain ctrlr_dom;
  ctrlr_dom.ctrlr = NULL;
  ctrlr_dom.domain = NULL;
  GET_USER_DATA_CTRLR_DOMAIN(ikey, ctrlr_dom);
  result_code = GetPMCtrlKeyval(ctrlckv, ikey, &ctrlr_dom);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("CheckRefCountCapability GetPMCtrlKeyval error (%d)",
        result_code);
    return result_code;
  }
  if ((op == UNC_OP_CREATE) || (op ==UNC_OP_UPDATE)) {
     IpcReqRespHeader *temp_req = reinterpret_cast<IpcReqRespHeader *>
               (ConfigKeyVal::Malloc(sizeof(IpcReqRespHeader)));

     temp_req->datatype = dt_type;
     temp_req->operation = op;

     result_code = ValidateCapability(
       temp_req, ikey, reinterpret_cast<char *>(ctrlr_dom.ctrlr));

     free(temp_req);

     unc_keytype_ctrtype_t ctrlrtype = UNC_CT_UNKNOWN;
     uuc::CtrlrMgr *ctrlr_mgr = uuc::CtrlrMgr::GetInstance();
     if (result_code != UPLL_RC_SUCCESS) {
        DELETE_IF_NOT_NULL(ctrlckv);
        // VTN PolicingMap is not supported for other than PFC Controller
        // so skip adding entry for such sontroller in ctrlr table
        if ((!ctrlr_mgr->GetCtrlrType(reinterpret_cast<char *>(ctrlr_dom.ctrlr),
                      dt_type, &ctrlrtype)) || (ctrlrtype != UNC_CT_PFC)) {
           UPLL_LOG_DEBUG("Controller type is  %d", ctrlrtype);
           return UPLL_RC_SUCCESS;
        }
        UPLL_LOG_DEBUG("Key not supported by controller");
        return result_code;
    }
    val_policingmap_t *val_policingmap =
      reinterpret_cast<val_policingmap_t*>(GetVal(ikey));
    val_vtnpolicingmap_ctrl *val_vtnpmap_ctrl = static_cast<
            val_vtnpolicingmap_ctrl*>(GetVal(ctrlckv));
    for ( unsigned int loop = 0;
            loop < sizeof
            (val_vtnpmap_ctrl->valid)/sizeof(val_vtnpmap_ctrl->valid[0]);
            ++loop ) {
       if (val_policingmap->valid[loop] == UNC_VF_NOT_SUPPORTED)
         val_vtnpmap_ctrl->valid[loop] = UNC_VF_INVALID;
       else
         val_vtnpmap_ctrl->valid[loop] =
              val_policingmap->valid[loop];
    }
  }

  uint8_t flag = 0;
  GET_USER_DATA_FLAGS(ikey, flag);
  SET_USER_DATA_FLAGS(ctrlckv, flag);

  // Create/Update/Delete a record in CANDIDATE DB
  result_code = UpdateConfigDB(ctrlckv, dt_type, op, dmi, config_mode, vtn_name,
                               CTRLRTBL);
  if (UPLL_RC_SUCCESS != result_code) {
    if ((UNC_OP_CREATE == op) &&
        (UPLL_RC_ERR_INSTANCE_EXISTS == result_code)) {
      DELETE_IF_NOT_NULL(ctrlckv);
      return UPLL_RC_SUCCESS;
    }
    DELETE_IF_NOT_NULL(ctrlckv);
    UPLL_LOG_DEBUG("Create record Err in vtnpolicingmapctrlrtbl CAN DB(%d)",
          result_code);
    return result_code;
  }
  DELETE_IF_NOT_NULL(ctrlckv);
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtnPolicingMapMoMgr::GetDomainsForController(
    ConfigKeyVal *ckv_drvr,
    ConfigKeyVal *&ctrlr_ckv,
    DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = GetChildConfigKey(ctrlr_ckv, ckv_drvr);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetChildConfigKey failed %d", result_code);
    return result_code;
  }
  DbSubOp dbop = { kOpReadMultiple, kOpMatchCtrlr,
    kOpInOutCtrlr | kOpInOutDomain};
  return ReadConfigDB(ctrlr_ckv, UPLL_DT_RUNNING, UNC_OP_READ, dbop, dmi,
                             CTRLRTBL);
}
}  // namespace kt_momgr
}  // namespace upll
}  // namespace unc
