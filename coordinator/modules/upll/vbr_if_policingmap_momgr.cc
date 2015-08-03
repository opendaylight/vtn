/*
 * Copyright (c) 2012-2015 NEC Corporation
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
#include "uncxx/upll_log.hh"
#include "upll_validation.hh"
#include "unc/upll_ipc_enum.h"

namespace unc {
namespace upll {
namespace kt_momgr {

#define VTN_RENAME 0x01
#define VBR_RENAME 0x02
#define POLICINGPROFILE_RENAME 0x04
#define NO_POLICINGPROFILE_RENAME 0xFB
#define VLINK_CONFIGURED 0x01
#define PORTMAP_CONFIGURED 0x02
#define VLINK_PORTMAP_CONFIGURED 0x03
#define SET_FLAG_PORTMAP 0x20
#define SET_FLAG_VLINK 0x40
#define SET_FLAG_VLINK_PORTMAP 0x80
#define SET_FLAG_NO_VLINK_PORTMAP 0x9F

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
  { uudst::vbr_if_policingmap::kDbiVtnName, CFG_INPUT_KEY,
    offsetof(key_rename_vnode_info_t, new_unc_vtn_name),
    uud::kDalChar, (kMaxLenVtnName + 1) },
  { uudst::vbr_if_policingmap::kDbiVbrName, CFG_INPUT_KEY,
    offsetof(key_rename_vnode_info_t, new_unc_vnode_name),
    uud::kDalChar, (kMaxLenVnodeName + 1) },
  { uudst::vbr_if_policingmap::kDbiFlags, CK_VAL,
    offsetof(key_user_data_t, flags),
    uud::kDalUint8, 1 }
};

// Constructor
VbrIfPolicingMapMoMgr::VbrIfPolicingMapMoMgr() : MoMgrImpl() {
  UPLL_FUNC_TRACE;
  // Rename and ctrlr tables not required for this KT
  // setting max tables to 1
  ntable = MAX_MOMGR_TBLS;
  table = new Table *[ntable]();

  // For Main Table
  table[MAINTBL] = new Table(uudst::kDbiVbrIfPolicingMapTbl,
      UNC_KT_VBRIF_POLICINGMAP, vbrifpolicingmap_bind_info,
      IpctSt::kIpcStKeyVbrIf, IpctSt::kIpcStValPolicingmap,
      uudst::vbr_if_policingmap::kDbiVbrIfPolicingMapNumCols);

  /* For Rename Table*/
  table[CTRLRTBL] = NULL;

  /* For Rename Table*/
  table[RENAMETBL] = NULL;

  table[CONVERTTBL] = NULL;

  nchild = 0;
  child = NULL;
}

upll_rc_t VbrIfPolicingMapMoMgr::RestorePOMInCtrlTbl(
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
  if (tbl != MAINTBL ||
       (ikey->get_key_type() != UNC_KT_VBRIF_POLICINGMAP)) {
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
        UPLL_LOG_DEBUG("Failed to update PolicingProfile in CtrlrTbl err %d",
          result_code);
        return result_code;
      }
    }
  }
  */
  return result_code;
}

upll_rc_t VbrIfPolicingMapMoMgr::CreateCandidateMo(IpcReqRespHeader *req,
                                                   ConfigKeyVal *ikey,
                                                   DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  if (NULL == ikey || NULL == req || NULL == dmi) {
    UPLL_LOG_DEBUG("CreateCandidateMo Failed. Insufficient input parameters");
    return UPLL_RC_ERR_GENERIC;
  }

  upll_rc_t result_code = UPLL_RC_SUCCESS;
  uint8_t *ctrlr_id = NULL;
  val_policingmap_t *val_import = NULL;
  pfcdrv_val_vbrif_policingmap *pfc_val_import = NULL;
  UPLL_LOG_DEBUG("datatype (%d)", req->datatype);
  if (req->datatype == UPLL_DT_IMPORT) {
    if (ikey->get_cfg_val() && (ikey->get_cfg_val()->get_st_num() ==
                                IpctSt::kIpcStPfcdrvValVbrifPolicingmap)) {
      UPLL_LOG_TRACE("ikey = %s", ikey->ToStrAll().c_str());
      UPLL_LOG_TRACE("val struct num (%d)", ikey->get_cfg_val()->get_st_num());
      pfc_val_import =
          reinterpret_cast<pfcdrv_val_vbrif_policingmap *>
          (ikey->get_cfg_val()->get_val());
      if (pfc_val_import->valid[PFCDRV_IDX_VAL_POLICINGMAP_PM] ==
          UNC_VF_VALID) {
        val_import = reinterpret_cast<val_policingmap_t *>
            (ConfigKeyVal::Malloc(sizeof(val_policingmap_t)));
        memcpy(val_import, &pfc_val_import->val_policing_map,
               sizeof(val_policingmap_t));
        UPLL_LOG_DEBUG("policer name (%s)", val_import->policer_name);
        ikey->SetCfgVal(NULL);
        ikey->SetCfgVal(new ConfigVal(IpctSt::kIpcStValPolicingmap,
                                      val_import));
      }
    }
  }
  UPLL_LOG_TRACE("%s ikey PRINT", ikey->ToStrAll().c_str());
  if (req->datatype != UPLL_DT_IMPORT) {
    // validate syntax and semantics
    result_code = ValidateMessage(req, ikey);
    if (result_code != UPLL_RC_SUCCESS) {
      pfc_log_error("ValidateMessage Err (%d)", result_code);
      return result_code;
    }
  }
  result_code = ValidateAttribute(ikey, dmi, req);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("ValidateAttribute failed, Error - %d", result_code);
    return result_code;
  }

  ConfigKeyVal *okey = NULL;

  result_code = GetControllerId(ikey, okey, req->datatype, dmi);
  if (UPLL_RC_SUCCESS != result_code) {
    if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
      result_code = UPLL_RC_ERR_PARENT_DOES_NOT_EXIST;
    }
    UPLL_LOG_DEBUG("GetControllerId failed %d", result_code);
    return result_code;
  }

  GET_USER_DATA_CTRLR(okey, ctrlr_id);

  // check whether this keytype is not requested under unified vbridge
  if (IsUnifiedVbr(ctrlr_id)) {
    UPLL_LOG_DEBUG(
       "This KT is not allowed to be created under Unified Vbridge");
    return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_KT;
  }

  UPLL_LOG_TRACE("ctrlrid %s", ctrlr_id);

  // Get the config mode info
  TcConfigMode config_mode = TC_CONFIG_INVALID;
  std::string vtn_name = "";
  result_code = GetConfigModeInfo(req, config_mode, vtn_name);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetConfigMode failed");
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
        UPLL_LOG_ERROR("Profile Object (%d) not available in CANDIDATE DB",
                       result_code);
        DELETE_IF_NOT_NULL(okey);
        return UPLL_RC_ERR_CFG_SEMANTIC;
      } else {
        UPLL_LOG_DEBUG("CreateCandidateMo Error Accesing CANDIDATEDB(%d)",
                       result_code);
        DELETE_IF_NOT_NULL(okey);
        return result_code;
      }
    }
    // check the policing profile existence in running DB also
    if (config_mode == TC_CONFIG_VTN) {
      result_code = IsPolicyProfileReferenced(ikey, UPLL_DT_RUNNING, dmi,
                                            UNC_OP_READ);
      if (UPLL_RC_SUCCESS != result_code) {
        if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
          UPLL_LOG_ERROR("Profile Object (%d) not available in RUNNING DB",
                         result_code);
          DELETE_IF_NOT_NULL(okey);
          return UPLL_RC_ERR_CFG_SEMANTIC;
        } else {
          UPLL_LOG_DEBUG("CreateCandidateMo Error Accesing RUNNING DB(%d)",
                         result_code);
          DELETE_IF_NOT_NULL(okey);
          return result_code;
        }
      }
    }
  }


  // Capability Check
  result_code = ValidateCapability(req, ikey,
                                   reinterpret_cast<char *>(ctrlr_id));
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Key not supported by controller");
    DELETE_IF_NOT_NULL(okey);
    return result_code;
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
                          vexternal, vex_if, flags,
                          reinterpret_cast<char *>(ctrlr_id));
  FREE_IF_NOT_NULL(vexternal);
  FREE_IF_NOT_NULL(vex_if);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetVexternal failed %d", result_code);
    DELETE_IF_NOT_NULL(okey);
    return result_code;
  }

  /* VlanmapOnBoundary: vbr_if_flowfilter is not allowed on
   * vbr interface of boundary vlink which is SW or SD mapped */
  VlinkMoMgr *vlink_mgr =
      reinterpret_cast<VlinkMoMgr *>(const_cast<MoManager*>(GetMoManager(
       UNC_KT_VLINK)));
  if (!vlink_mgr) {
    UPLL_LOG_DEBUG("vlink_mgr is Fail");
    DELETE_IF_NOT_NULL(ckv);
    return UPLL_RC_ERR_GENERIC;
  }
  ConfigKeyVal *ck_vlink = NULL;
  vn_if_type   iftype;
  /* Verifies whether the requested vbridge interface is boundary
   * vlinked with logical port id is SW or SD. If it is part of SW or SD
   * boundary linked, cannot configure policing map in that vbridge interface*/
  result_code =  vlink_mgr->CheckIfMemberOfVlink(ckv, req->datatype,
                                                 ck_vlink, dmi, iftype);
  if (result_code == UPLL_RC_SUCCESS) {
    unc_key_type_t ktype1;
    if (iftype == kVlinkBoundaryNode1) {
       UPLL_LOG_DEBUG("iftype is kVlinkBoundaryNode1");
      ktype1 = vlink_mgr->GetVlinkVnodeIfKeyType(ck_vlink, 0);
      if (ktype1 == UNC_KT_VBR_VLANMAP) {
        UPLL_LOG_DEBUG("iftype is kVlinkBoundaryNode1 And "
                        "ktype is UNC_KT_VBR_VLANMAP");
        DELETE_IF_NOT_NULL(ckv);
        return UPLL_RC_ERR_CFG_SEMANTIC;
      }
    } else if (iftype == kVlinkBoundaryNode2) {
       UPLL_LOG_DEBUG("iftype is kVlinkBoundaryNode2");
      ktype1 = vlink_mgr->GetVlinkVnodeIfKeyType(ck_vlink, 1);
      if (ktype1 == UNC_KT_VBR_VLANMAP) {
        UPLL_LOG_DEBUG("iftype is kVlinkBoundaryNode2 And "
                        "ktype is UNC_KT_VBR_VLANMAP");
        DELETE_IF_NOT_NULL(ckv);
        return UPLL_RC_ERR_CFG_SEMANTIC;
      }
    }
  } else if (result_code != UPLL_RC_SUCCESS &&
      result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    DELETE_IF_NOT_NULL(ckv);
    UPLL_LOG_DEBUG("CheckIfMemberOfVlink is Fail");
    return result_code;
  }
  /* If its not member of vlink continue below steps */
  DELETE_IF_NOT_NULL(ckv);
  UPLL_LOG_DEBUG("GetVexternal flag %d", flags);
  uint8_t flag_port_map = 0;
  GET_USER_DATA_FLAGS(ikey, flag_port_map);
  if (flags & VLINK_CONFIGURED) {
    UPLL_LOG_DEBUG("VLINK_CONFIGURED");
    flag_port_map = flag_port_map | SET_FLAG_VLINK;
  } else if (flags & PORTMAP_CONFIGURED) {
    UPLL_LOG_DEBUG("PORTMAP_CONFIGURED");
    flag_port_map =  flag_port_map | SET_FLAG_PORTMAP;
  } else if (flags & VLINK_PORTMAP_CONFIGURED) {
    UPLL_LOG_DEBUG("VLINK_PORTMAP_CONFIGURED");
    flag_port_map = flag_port_map | SET_FLAG_VLINK_PORTMAP;
  }

  UPLL_LOG_DEBUG("flag_port_map %d", flag_port_map);
  SET_USER_DATA_FLAGS(ikey, flag_port_map);

  if (UNC_VF_VALID == val_pm->valid[UPLL_IDX_POLICERNAME_PM]) {
    // Update the controller reference count for policing profile if portmap is
    // configured.
    // 1)Get vbrif associated ctrlr name and invoke the PP and PPE functions to
    // check the refcount capability and update the refcount or create the
    // record in policingprofilectrltbl and policingprofileentryctrltbl.
    // 2)Create the record in policingprofileentryctrltbl

    if (flag_port_map & SET_FLAG_PORTMAP) {
      result_code = UpdateRefCountInPPCtrlr(ikey, req->datatype, dmi,
                                          UNC_OP_CREATE, config_mode,
                                          vtn_name);
      if (UPLL_RC_SUCCESS != result_code) {
        UPLL_LOG_DEBUG("Err while updating the policingprofile ctrlr table(%d)",
                     result_code);
        DELETE_IF_NOT_NULL(okey);
        return result_code;
      }
    }
  }
  DbSubOp dbop = { kOpReadSingle, kOpMatchNone,
    kOpInOutFlag|kOpInOutCtrlr|kOpInOutDomain };
  result_code = UpdateConfigDB(ikey, req->datatype, UNC_OP_CREATE,
                               dmi, &dbop, config_mode, vtn_name, MAINTBL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("CreateCandidateMo failed. UpdateConfigDb failed."
                   "Record creation failed - %d",
                   result_code);
    DELETE_IF_NOT_NULL(okey);
    return result_code;
  }
  UPLL_LOG_DEBUG("CreateCandidateMo Successful");
  DELETE_IF_NOT_NULL(okey);
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
  result_code = IsReferenced(req, ikey, dmi);
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
    kOpInOutCtrlr | kOpInOutDomain | kOpInOutFlag};
  result_code = ReadConfigDB(okey, req->datatype, UNC_OP_READ, dbop, dmi,
                             MAINTBL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("ReadConfigDB failed %d", result_code);
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

  val_policingmap_t *val_pm = reinterpret_cast<val_policingmap_t *>
    (GetVal(okey));
  if (UNC_VF_VALID == val_pm->valid[UPLL_IDX_POLICERNAME_PM]) {
    uint8_t flag_port_map = 0;
    GET_USER_DATA_FLAGS(okey, flag_port_map);
    if (flag_port_map & SET_FLAG_PORTMAP) {
      result_code = UpdateRefCountInPPCtrlr(okey, req->datatype, dmi,
          UNC_OP_DELETE, config_mode, vtn_name);
      if (UPLL_RC_SUCCESS != result_code) {
        DELETE_IF_NOT_NULL(okey);
        UPLL_LOG_DEBUG("UpdateRefCountInPPCtrlr Error DB (%d)", result_code);
        return result_code;
      }
    }
  }

  // Delete the record in vbrifpolicingmap table
  result_code = UpdateConfigDB(ikey, req->datatype, UNC_OP_DELETE, dmi,
                               config_mode, vtn_name, MAINTBL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("DeleteMo Failed. UpdateConfigdb failed to delete - %d",
                  result_code);
    DELETE_IF_NOT_NULL(okey);
    return result_code;
  }
  DELETE_IF_NOT_NULL(okey);
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

  // validate syntax and semantics
  result_code = ValidateMessage(req, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("ValidateMessage Err (%d)", result_code);
    return result_code;
  }

  TcConfigMode config_mode = TC_CONFIG_INVALID;
  std::string vtn_name = "";
  result_code = GetConfigModeInfo(req, config_mode, vtn_name);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetConfigMode failed");
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
          UPLL_LOG_ERROR("Profile Object (%d) not available in RUNNING DB",
                         result_code);
          return UPLL_RC_ERR_CFG_SEMANTIC;
        } else {
          UPLL_LOG_DEBUG(
              "IsPolicyProfileReferenced error Accesing RUNNING DB(%d)",
               result_code);
          return result_code;
        }
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
    DELETE_IF_NOT_NULL(tmpckv);
    UPLL_LOG_DEBUG("ReadConfigDB failed");
    return result_code;
  }
  /* Check whether all attributes are invalid in value structure.*/
  if (VtnPolicingMapMoMgr::IsAllAttrInvalid(
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
  uint8_t *ctrlr_id = NULL;
  GET_USER_DATA_CTRLR(tmpckv, ctrlr_id);
  SET_USER_DATA_CTRLR(ikey, ctrlr_id);

  // Capability Check
  result_code =
      ValidateCapability(req, ikey, reinterpret_cast<char*>(ctrlr_id));
  if (result_code != UPLL_RC_SUCCESS) {
    DELETE_IF_NOT_NULL(tmpckv);
    UPLL_LOG_DEBUG("Key not supported by controller");
    return result_code;
  }

  if (UNC_VF_VALID == val_ival->valid[UPLL_IDX_POLICERNAME_PM] &&
      UNC_VF_VALID == val_tmp_val->valid[UPLL_IDX_POLICERNAME_PM]) {
    UPLL_LOG_DEBUG(" Policer name valid in DB and ikey");
    result_code = UpdateRefCountInPPCtrlr(tmpckv, req->datatype, dmi,
        UNC_OP_DELETE, config_mode, vtn_name);
    if (UPLL_RC_SUCCESS != result_code) {
      DELETE_IF_NOT_NULL(tmpckv);
      UPLL_LOG_DEBUG("UpdateMo UpdateRefCountInPPCtrlr error in delete (%d)",
          result_code);
      return result_code;
    }

    result_code = UpdateRefCountInPPCtrlr(ikey, req->datatype, dmi,
        UNC_OP_CREATE, config_mode, vtn_name);
    if (UPLL_RC_SUCCESS != result_code) {
      DELETE_IF_NOT_NULL(tmpckv);
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
        UNC_OP_CREATE, config_mode, vtn_name);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("UpdateMo UpdateRefCountInPPCtrlr error in create (%d)",
          result_code);
      DELETE_IF_NOT_NULL(tmpckv);
      return result_code;
    }
  } else if (UNC_VF_VALID == val_tmp_val->valid[UPLL_IDX_POLICERNAME_PM] &&
      UNC_VF_VALID_NO_VALUE == val_ival->
      valid[UPLL_IDX_POLICERNAME_PM]) {
    UPLL_LOG_DEBUG(" Policer name validnovalue in ikey and valid in db");
    result_code = UpdateRefCountInPPCtrlr(tmpckv, req->datatype, dmi,
        UNC_OP_DELETE, config_mode, vtn_name);
    if (UPLL_RC_SUCCESS != result_code) {
      DELETE_IF_NOT_NULL(tmpckv);
      UPLL_LOG_DEBUG("UpdateMo UpdateRefCountInPPCtrlr error in create (%d)",
          result_code);
      return result_code;
    }
  }

  // Update the record in CANDIDATE DB
  uint8_t temp_flag = 0;
  GET_USER_DATA_FLAGS(ikey, temp_flag);
  UPLL_LOG_DEBUG("Flag in ikey: %d", temp_flag);
  DbSubOp dbop1 = {kOpNotRead, kOpMatchNone, kOpInOutNone};
  result_code = UpdateConfigDB(ikey, req->datatype, req->operation, dmi,
                               &dbop1, config_mode, vtn_name, MAINTBL);
  DELETE_IF_NOT_NULL(tmpckv);
  UPLL_LOG_TRACE("VtnPolicingMapMoMgr::UpdateMo update record status (%d)",
                result_code);

  return result_code;
}

upll_rc_t VbrIfPolicingMapMoMgr::IsReferenced(IpcReqRespHeader *req,
                                              ConfigKeyVal *ikey,
                                              DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  if (NULL == ikey) return UPLL_RC_ERR_GENERIC;
  DbSubOp dbop = { kOpReadExist, kOpMatchNone, kOpInOutNone };

  // Check the VBRIF PM object existence in vbrifpolicingmap
  result_code = UpdateConfigDB(ikey, req->datatype, UNC_OP_READ, dmi,
                               &dbop, MAINTBL);
  UPLL_LOG_TRACE("IsReferenced (%d)", result_code);
  return result_code;
}

upll_rc_t VbrIfPolicingMapMoMgr::ValidateAttribute(ConfigKeyVal *ikey,
                                                   DalDmlIntf *dmi,
                                                   IpcReqRespHeader *req) {
  // No operation
  return UPLL_RC_SUCCESS;
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
    <const char *>(val_pm->policer_name), dmi, dt_type);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_INFO("ValidateValidElements failed %d", result_code);
    delete okey;
    return result_code;
  }
  DELETE_IF_NOT_NULL(okey);
  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrIfPolicingMapMoMgr::UpdateRefCountInPPCtrlr(
    ConfigKeyVal *ikey, upll_keytype_datatype_t dt_type, DalDmlIntf *dmi,
    unc_keytype_operation_t op, TcConfigMode config_mode,
    string vtn_name) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  uint8_t *ctrlr_id = NULL;
  uint8_t pp_flag = 0, pp_flag1 = 0;

  // Get the contorller name from VBR MAINTBL
  GET_USER_DATA_CTRLR(ikey, ctrlr_id);

  PolicingProfileMoMgr *pp_mgr =
      reinterpret_cast<PolicingProfileMoMgr *>
      (const_cast<MoManager *>(GetMoManager(
          UNC_KT_POLICING_PROFILE)));
  if (!pp_mgr) {
    UPLL_LOG_TRACE("pp_mgr obj failure (%d)", result_code);
    return result_code;
  }
  #if 0
  PolicingProfileEntryMoMgr *ppe_mgr =
      reinterpret_cast<PolicingProfileEntryMoMgr *>
      (const_cast<MoManager *>(GetMoManager(
          UNC_KT_POLICING_PROFILE_ENTRY)));
  if (!ppe_mgr) {
    UPLL_LOG_TRACE("ppe_mgr obj failure (%d)", result_code);
    DELETE_IF_NOT_NULL(okey);
    return result_code;
  }
  #endif

  val_policingmap_t* val_vtn_policingmap =
      reinterpret_cast<val_policingmap_t *>(GetVal(ikey));

  if (UPLL_DT_IMPORT == dt_type) {
    GET_USER_DATA_FLAGS(ikey, pp_flag);
    if (pp_flag & POLICINGPROFILE_RENAME)
       pp_flag1 = PP_RENAME;
    UPLL_LOG_DEBUG("##### op1op1 flag %d", pp_flag);
    UPLL_LOG_DEBUG("##### op1op1 flag1 %d", pp_flag1);
  }

  if (NULL == ctrlr_id) {
    UPLL_LOG_DEBUG("UpdateRefCountInPPCtrlr ctrlr_id NULL (%d)",
        result_code);
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

  std::string temp_vtn_name;
  if (TC_CONFIG_VTN == config_mode) {
    temp_vtn_name = vtn_name;
  } else {
    temp_vtn_name = reinterpret_cast<const char *>(reinterpret_cast
                                                   <key_vbr_if_t *>
                                                   (ikey->get_key())->
                                                   vbr_key.vtn_key.vtn_name);
  }
  result_code = pp_mgr->PolicingProfileCtrlrTblOper(
      reinterpret_cast<const char*>(val_vtn_policingmap->policer_name),
      reinterpret_cast<const char*>(ctrlr_id), dmi, op, dt_type, pp_flag1,
      config_mode, temp_vtn_name, 1, false);

  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("PolicingProfileCtrlrTblOper err (%d)", result_code);
    return result_code;
  }

  // Create/Delete/Update the record in policingprofileentryctrltbl
  // based on oper
  UPLL_LOG_DEBUG("PPECtrl  profile name(%s)  ctrlr id(%s)",
                 val_vtn_policingmap->policer_name, ctrlr_id);
  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrIfPolicingMapMoMgr::GetControllerId(
    ConfigKeyVal *ikey,
    ConfigKeyVal *&okey,
    upll_keytype_datatype_t dt_type,
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
  result_code = mgr->ReadConfigDB(okey, dt_type, UNC_OP_READ, dbop,
                                  dmi, MAINTBL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_ERROR("GetControllerId ReadConfigDB failed (%d)", result_code);
    DELETE_IF_NOT_NULL(okey);
    return result_code;
  }

  controller_domain ctrlr_dom;
  ctrlr_dom.ctrlr = NULL;
  ctrlr_dom.domain = NULL;
  result_code = mgr->GetControllerDomainId(okey, &ctrlr_dom);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_INFO("GetControllerDomainId failed %d", result_code);
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
            DELETE_IF_NOT_NULL(dup_key);
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
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("GetReadVbrIfKey dup_key Err  (%d)", result_code);
            return result_code;
          }
          DbSubOp dbop = {kOpReadSingle, kOpMatchNone, kOpInOutNone};
          result_code = ReadConfigDB(temp_vbr_if_key, req->datatype,
                                     UNC_OP_READ, dbop, dmi, MAINTBL);
          if (UPLL_RC_SUCCESS != result_code) {
            DELETE_IF_NOT_NULL(temp_vbr_if_key);
            UPLL_LOG_DEBUG("ReadConfigDB failed %d", result_code);
            return result_code;
          }
          key_vbr_if_t *key_if = reinterpret_cast<key_vbr_if_t *>
                                  (temp_vbr_if_key->get_key());
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
            DELETE_IF_NOT_NULL(ppe_ckv);
            DELETE_IF_NOT_NULL(temp_vbr_if_key);
            return UPLL_RC_ERR_GENERIC;
          }
          IpcReqRespHeader *temp_req = reinterpret_cast<IpcReqRespHeader *>
              (ConfigKeyVal::Malloc(sizeof(IpcReqRespHeader)));
          memcpy(temp_req, req, sizeof(IpcReqRespHeader));
          temp_req->option1 = UNC_OPT1_NORMAL;
          // result_code = mgr->ReadInfoFromDB(req, ppe_ckv, dmi, &ctrlr_dom);
          result_code = mgr->ReadSiblingMo(temp_req, ppe_ckv, dmi);
          req->rep_count = temp_req->rep_count;
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("Read sibling of ppe failed (%d)", result_code);
            DELETE_IF_NOT_NULL(temp_vbr_if_key);
            DELETE_IF_NOT_NULL(ppe_ckv);
            ConfigKeyVal::Free(temp_req);
            return result_code;
          }
          ConfigKeyVal::Free(temp_req);
          ConfigKeyVal *temp_ppe_ckv = ppe_ckv;
          ConfigKeyVal *okey = NULL;
          while (NULL != temp_ppe_ckv) {
            ConfigKeyVal *temp_vbrif_pm_ckv = NULL;
            key_policingprofile_entry_t *temp_ppe_key = reinterpret_cast
                <key_policingprofile_entry_t *>(temp_ppe_ckv->get_key());
            result_code = GetChildEntryConfigKey(temp_vbrif_pm_ckv, ikey);
            if (UPLL_RC_SUCCESS != result_code) {
              DELETE_IF_NOT_NULL(ppe_ckv);
              DELETE_IF_NOT_NULL(temp_vbr_if_key);
              UPLL_LOG_DEBUG("GetChildEntryConfigKey failed");
              return result_code;
            }
            key_vbrif_policingmap_entry_t *temp_vbrif_pm_key = reinterpret_cast
                <key_vbrif_policingmap_entry_t *>(temp_vbrif_pm_ckv->get_key());
            temp_vbrif_pm_key->sequence_num = temp_ppe_key->sequence_num;
            uuu::upll_strncpy(
                reinterpret_cast<char*>(temp_vbrif_pm_key->vbrif_key.if_name),
                reinterpret_cast<char*>(key_if->if_name),
                (kMaxLenVnodeName + 1));
            result_code = ReadDTStateNormal(req, temp_vbrif_pm_ckv, dmi);
            if (result_code != UPLL_RC_SUCCESS) {
              DELETE_IF_NOT_NULL(ppe_ckv);
              DELETE_IF_NOT_NULL(temp_vbrif_pm_ckv);
              DELETE_IF_NOT_NULL(temp_vbr_if_key);
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
          DELETE_IF_NOT_NULL(temp_vbr_if_key);
          DELETE_IF_NOT_NULL(ppe_ckv);
          if ((okey != NULL) && (result_code == UPLL_RC_SUCCESS)) {
            ikey->ResetWith(okey);
            DELETE_IF_NOT_NULL(okey);
          }
        } else if ((req->option1 == UNC_OPT1_DETAIL) &&
            (req->option2 == UNC_OPT2_NONE)) {
          UPLL_LOG_DEBUG("ReadRecord ReadConfigDB error (%d) (%d)",
              req->datatype, req->option1);
          ConfigKeyVal *temp_vbr_if_key = NULL;
          key_vbrif_policingmap_entry_t *vbrif_entry_key = reinterpret_cast
            <key_vbrif_policingmap_entry_t *>(ikey->get_key());
          result_code = GetReadVbrIfKey(temp_vbr_if_key, ikey);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("GetReadVbrIfKey dup_key Err  (%d)", result_code);
            return result_code;
          }
          DbSubOp dbop = {kOpReadSingle, kOpMatchNone, kOpInOutNone};
          result_code = ReadConfigDB(temp_vbr_if_key, req->datatype,
                                     UNC_OP_READ, dbop, dmi, MAINTBL);
          if (UPLL_RC_SUCCESS != result_code) {
            DELETE_IF_NOT_NULL(temp_vbr_if_key);
            UPLL_LOG_DEBUG("ReadConfigDB failed %d", result_code);
            return result_code;
          }
          val_policingmap_t *val_pm = reinterpret_cast
            <val_policingmap_t *>(GetVal(temp_vbr_if_key));
         key_vbr_if_t *key_if = reinterpret_cast<key_vbr_if_t *>
                                (temp_vbr_if_key->get_key());
          ConfigKeyVal *ppe_ckv = NULL;
          result_code = ConstructPpeCkv(ppe_ckv, reinterpret_cast
              <const char *>(val_pm->policer_name),
              vbrif_entry_key->sequence_num);
          PolicingProfileEntryMoMgr *mgr = reinterpret_cast
            <PolicingProfileEntryMoMgr*>
            (const_cast<MoManager *>(GetMoManager
                                     (UNC_KT_POLICING_PROFILE_ENTRY)));
          if (!mgr) {
            DELETE_IF_NOT_NULL(ppe_ckv);
            DELETE_IF_NOT_NULL(temp_vbr_if_key);
            return UPLL_RC_ERR_GENERIC;
          }
          IpcReqRespHeader *temp_req = reinterpret_cast<IpcReqRespHeader *>
            (ConfigKeyVal::Malloc(sizeof(IpcReqRespHeader)));
          memcpy(temp_req, req, sizeof(IpcReqRespHeader));
          temp_req->option1 = UNC_OPT1_NORMAL;
          // result_code = mgr->ReadInfoFromDB(req, ppe_ckv, dmi, &ctrlr_dom);
          result_code = mgr->ReadSiblingMo(temp_req, ppe_ckv, dmi);
          req->rep_count = temp_req->rep_count;
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("Read sibling of ppe failed (%d)", result_code);
            ConfigKeyVal::Free(temp_req);
            DELETE_IF_NOT_NULL(temp_vbr_if_key);
            DELETE_IF_NOT_NULL(ppe_ckv);
            return result_code;
          }
          ConfigKeyVal::Free(temp_req);

          ConfigKeyVal *temp_ppe_ckv = ppe_ckv;
          ConfigKeyVal *okey = NULL;
          while (NULL != temp_ppe_ckv) {
            ConfigKeyVal *temp_vbrif_pm_ckv = NULL;
            key_policingprofile_entry_t *temp_ppe_key = reinterpret_cast
              <key_policingprofile_entry_t *>(temp_ppe_ckv->get_key());
            result_code = GetChildEntryConfigKey(temp_vbrif_pm_ckv, ikey);
            if (UPLL_RC_SUCCESS != result_code) {
              DELETE_IF_NOT_NULL(ppe_ckv);
              DELETE_IF_NOT_NULL(temp_vbr_if_key);
              UPLL_LOG_DEBUG("GetChildEntryConfigKey failed");
              return result_code;
            }
            key_vbrif_policingmap_entry_t *temp_vbrif_pm_key = reinterpret_cast
              <key_vbrif_policingmap_entry_t *>(temp_vbrif_pm_ckv->get_key());
            temp_vbrif_pm_key->sequence_num = temp_ppe_key->sequence_num;
            uuu::upll_strncpy(
                reinterpret_cast<char*>(temp_vbrif_pm_key->vbrif_key.if_name),
                reinterpret_cast<char*>(key_if->if_name),
                (kMaxLenVnodeName + 1));
            UPLL_LOG_DEBUG("vbrifpmentry sequence_num - %d", result_code);
            result_code = ReadEntryDetailRecord(req, temp_vbrif_pm_ckv, dmi);
            if (UPLL_RC_SUCCESS != result_code) {
              DELETE_IF_NOT_NULL(ppe_ckv);
              DELETE_IF_NOT_NULL(temp_vbrif_pm_ckv);
              DELETE_IF_NOT_NULL(temp_vbr_if_key);
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
          DELETE_IF_NOT_NULL(temp_vbr_if_key);
          DELETE_IF_NOT_NULL(ppe_ckv);
          if ((okey != NULL) && (result_code == UPLL_RC_SUCCESS)) {
            ikey->ResetWith(okey);
            DELETE_IF_NOT_NULL(okey);
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
    DELETE_IF_NOT_NULL(dup_key);
    return result_code;
  }

  result_code =  DupConfigKeyVal(l_key, ikey, MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("DupConfigKeyVal Faill in ReadSiblingMo for l_key");
    DELETE_IF_NOT_NULL(dup_key);
    return result_code;
  }
  GET_USER_DATA_CTRLR_DOMAIN(dup_key, ctrlr_dom);
  SET_USER_DATA_CTRLR_DOMAIN(l_key, ctrlr_dom);
  GET_USER_DATA_FLAGS(dup_key, db_flag);

  result_code = ValidateCapability(req, ikey,
                                   reinterpret_cast<char *>(ctrlr_dom.ctrlr));

  if (result_code != UPLL_RC_SUCCESS) {
    DELETE_IF_NOT_NULL(dup_key);
    DELETE_IF_NOT_NULL(l_key);
    UPLL_LOG_DEBUG("Key not supported by controller");
    return result_code;
  }

  //  GET_USER_DATA_FLAGS(dup_key, db_flag);
  UPLL_LOG_DEBUG("db_flag ::: (%d)", db_flag);
  result_code = GetRenamedControllerKey(l_key, req->datatype,
                                        dmi, &ctrlr_dom);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetRenamedControllerKey Faill");
    DELETE_IF_NOT_NULL(dup_key);
    DELETE_IF_NOT_NULL(l_key);
    return result_code;
  }
  pfcdrv_val_vbrif_policingmap *pfc_val =
      reinterpret_cast<pfcdrv_val_vbrif_policingmap *>
      (ConfigKeyVal::Malloc(sizeof(pfcdrv_val_vbrif_policingmap_t)));
  pfcdrv_val_vbrif_vextif *pfc_val_ext =
      reinterpret_cast<pfcdrv_val_vbrif_vextif *>
      (ConfigKeyVal::Malloc(sizeof(pfcdrv_val_vbrif_vextif_t)));

  UPLL_LOG_DEBUG("GetVexternalInformation (%d)", req->datatype);
  result_code = GetVexternalInformation(dup_key, UPLL_DT_RUNNING, pfc_val,
                                        pfc_val_ext, db_flag,
                                        (const char*)ctrlr_dom.ctrlr, dmi);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetVexternalInformation fail");
    DELETE_IF_NOT_NULL(dup_key);
    DELETE_IF_NOT_NULL(l_key);
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
    DELETE_IF_NOT_NULL(l_key);
    DELETE_IF_NOT_NULL(dup_key);
    DELETE_IF_NOT_NULL(ipc_response.ckv_data);
    return ipc_response.header.result_code;
  }

  if (ipc_response.header.result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Driver response for Key %d controller %s result %d",
                   l_key->get_key_type(), ctrlr_dom.ctrlr,
                   ipc_response.header.result_code);
    DELETE_IF_NOT_NULL(l_key);
    DELETE_IF_NOT_NULL(dup_key);
    DELETE_IF_NOT_NULL(ipc_response.ckv_data);
    return ipc_response.header.result_code;
  }
  ConfigKeyVal *okey = NULL;
  result_code = ConstructReadDetailResponse(dup_key, ipc_response.ckv_data,
                                            req->datatype, req->operation,
                                            dbop, dmi, &okey, ctrlr_dom);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("ReadSiblingDetail Error  (%d)", result_code);
    DELETE_IF_NOT_NULL(dup_key);
    DELETE_IF_NOT_NULL(l_key);
    DELETE_IF_NOT_NULL(ipc_response.ckv_data);
    return result_code;
  } else {
    if (okey != NULL) {
      ikey->ResetWith(okey);
      DELETE_IF_NOT_NULL(okey);
      req->rep_count = 1;
    }
  }
  DELETE_IF_NOT_NULL(ipc_response.ckv_data);
  DELETE_IF_NOT_NULL(l_key);
  DELETE_IF_NOT_NULL(dup_key);
  return result_code;
}

// Rename
bool VbrIfPolicingMapMoMgr::GetRenameKeyBindInfo(unc_key_type_t key_type,
                                                 BindInfo *&binfo, int &nattr,
                                                 MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  UPLL_LOG_DEBUG("GetRenameKeyBindInfo (%d) (%d)", key_type, tbl);
  nattr = sizeof(key_vbrifpm_maintbl_rename_bind_info)/
          sizeof(key_vbrifpm_maintbl_rename_bind_info[0]);
  binfo = key_vbrifpm_maintbl_rename_bind_info;
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

  key_rename_vnode_info *key_rename =
  reinterpret_cast<key_rename_vnode_info *>(ikey->get_key());
  key_vbr_if_t *key_vbr_if = reinterpret_cast<key_vbr_if_t *>
                             (ConfigKeyVal::Malloc(sizeof(key_vbr_if_t)));
  if (!strlen(reinterpret_cast<char *>(key_rename->old_unc_vtn_name))) {
    UPLL_LOG_DEBUG("old_unc_vtn_name NULL");
    FREE_IF_NOT_NULL(key_vbr_if);
    return UPLL_RC_ERR_GENERIC;
  }

  uuu::upll_strncpy(key_vbr_if->vbr_key.vtn_key.vtn_name,
                    key_rename->old_unc_vtn_name,
                    (kMaxLenVtnName+1));

  if (UNC_KT_VBRIDGE == ikey->get_key_type()) {
    if (!strlen(reinterpret_cast<char *>(key_rename->old_unc_vnode_name))) {
      UPLL_LOG_DEBUG("old_unc_vnode_name NULL");
      FREE_IF_NOT_NULL(key_vbr_if);
      return UPLL_RC_ERR_GENERIC;
    }
    uuu::upll_strncpy(key_vbr_if->vbr_key.vbridge_name,
                      key_rename->old_unc_vnode_name, (kMaxLenVnodeName + 1));
  } else {
    if (!strlen(reinterpret_cast<char *>(key_rename->new_unc_vnode_name))) {
      UPLL_LOG_DEBUG("new_unc_vnode_name NULL");
      FREE_IF_NOT_NULL(key_vbr_if);
      return UPLL_RC_ERR_GENERIC;
    }
    uuu::upll_strncpy(key_vbr_if->vbr_key.vbridge_name,
                      key_rename->new_unc_vnode_name, (kMaxLenVnodeName + 1));
  }

  okey = new ConfigKeyVal(UNC_KT_VBRIF_POLICINGMAP, IpctSt::kIpcStKeyVbrIf,
             key_vbr_if, NULL);
  if (!okey) {
    UPLL_LOG_DEBUG("okey NULL");
    FREE_IF_NOT_NULL(key_vbr_if);
    return UPLL_RC_ERR_GENERIC;
  }
  return result_code;
}

upll_rc_t VbrIfPolicingMapMoMgr::UpdateVnodeVal(ConfigKeyVal *ikey,
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

  UPLL_LOG_DEBUG("CopyToConfigKey datatype (%d)", data_type);
  // Copy the old policer name in val_policingmap
  val_policingmap_t *val = reinterpret_cast<val_policingmap_t *>
        (ConfigKeyVal::Malloc(sizeof(val_policingmap_t)));
  if (!val) return UPLL_RC_ERR_GENERIC;

  memset(val, 0, sizeof(val_policingmap_t));
  if (!strlen(reinterpret_cast<char *>(key_rename->old_policingprofile_name))) {
    UPLL_LOG_ERROR("key_rename->old_policingprofile_name NULL");
    FREE_IF_NOT_NULL(val);
    return UPLL_RC_ERR_GENERIC;
  }

  uuu::upll_strncpy(val->policer_name,
      key_rename->old_policingprofile_name,
      (kMaxLenPolicingProfileName + 1));
  val->valid[UPLL_IDX_POLICERNAME_PM] = UNC_VF_VALID;
  UPLL_LOG_DEBUG("policer name and valid (%d) (%s)",
                  val->valid[UPLL_IDX_POLICERNAME_PM], val->policer_name);

  result_code = GetChildConfigKey(okey, NULL);
  if (UPLL_RC_SUCCESS != result_code) {
     UPLL_LOG_DEBUG("CopyToConfigKey okey  NULL");
     FREE_IF_NOT_NULL(val);
     return result_code;
  }
  if (!okey) {
    free(val);
    return UPLL_RC_ERR_GENERIC;
  }
  okey->SetCfgVal(new ConfigVal(IpctSt::kIpcStValPolicingmap, val));

  DbSubOp dbop = { kOpReadMultiple, kOpMatchNone,
    kOpInOutCtrlr|kOpInOutDomain|kOpInOutFlag };

  // Read the record of key structure and old policer name in maintbl
  result_code = ReadConfigDB(okey, data_type, UNC_OP_READ, dbop, dmi,
    MAINTBL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_ERROR(" ReadConfigDB failed:%d", result_code);
    DELETE_IF_NOT_NULL(okey);
    return result_code;
  }
  ConfigKeyVal *first_ckv = okey;
  while (okey != NULL) {
     UPLL_LOG_DEBUG("okey not NULL");
    // Update the new policer name in MAINTBL
    result_code = GetChildConfigKey(kval, okey);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("GetChildConfigKey kval NULL");
      DELETE_IF_NOT_NULL(first_ckv);
      return result_code;
    }
    if (!kval) return UPLL_RC_ERR_GENERIC;
    // Copy the new policer name in val_policingmap
    val_policingmap_t *val1 = reinterpret_cast<val_policingmap_t *>
          (ConfigKeyVal::Malloc(sizeof(val_policingmap_t)));
    if (!val1) return UPLL_RC_ERR_GENERIC;
    memset(val1, 0, sizeof(val_policingmap_t));

    // New name null check
    if (!strlen(
            reinterpret_cast<char *>(key_rename->new_policingprofile_name))) {
      FREE_IF_NOT_NULL(val1);
      UPLL_LOG_ERROR("new_policingprofile_name NULL");
      DELETE_IF_NOT_NULL(kval);
      DELETE_IF_NOT_NULL(first_ckv);
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
#if 0
    ConfigKeyVal *okey1 = NULL;
    result_code = GetControllerId(data_type, okey, okey1, dmi);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("GetControllerId failed %d", result_code);
      return result_code;
    }
    delete okey1;
#endif
    GET_USER_DATA_CTRLR_DOMAIN(okey, ctrlr_dom);
    GET_USER_DATA_FLAGS(okey, rename);

    UPLL_LOG_DEBUG("Controller : %s; Domain : %s", ctrlr_dom.ctrlr,
        ctrlr_dom.domain);

    UPLL_LOG_DEBUG("okey flag (%d)", rename);

    if (!no_rename)
      rename = rename | POLICINGPROFILE_RENAME;
    else
      rename = rename & NO_POLICINGPROFILE_RENAME;

    SET_USER_DATA_FLAGS(kval, rename);
    SET_USER_DATA_CTRLR_DOMAIN(kval, ctrlr_dom);

    UPLL_LOG_DEBUG("kval flag (%d)", rename);
    // Update the new policer name in MAINTBL
    result_code = UpdateConfigDB(kval, data_type, UNC_OP_UPDATE, dmi,
                  TC_CONFIG_GLOBAL, vtn_name, MAINTBL);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("Create record Err in vtnpolicingmaptbl CANDIDATE DB(%d)",
        result_code);
      DELETE_IF_NOT_NULL(kval);
      DELETE_IF_NOT_NULL(first_ckv);
      return result_code;
    }

    DELETE_IF_NOT_NULL(kval);

    okey = okey->get_next_cfg_key_val();
  }
  DELETE_IF_NOT_NULL(first_ckv);
  return result_code;
}

upll_rc_t VbrIfPolicingMapMoMgr::MergeValidate(unc_key_type_t keytype,
                                             const char *ctrlr_id,
                                             ConfigKeyVal *ikey,
                                             DalDmlIntf *dmi,
                                             upll_import_type import_type) {
  UPLL_FUNC_TRACE;
  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrIfPolicingMapMoMgr::GetRenamedUncKey(
    ConfigKeyVal *ikey, upll_keytype_datatype_t dt_type, DalDmlIntf *dmi,
    uint8_t *ctrlr_id) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *unc_key = NULL;
  DbSubOp dbop = { kOpReadSingle, kOpMatchCtrlr, kOpInOutNone };
  UPLL_LOG_TRACE("%s GetRenamedUncKey vbrifpm start",
                  ikey->ToStrAll().c_str());
  if (NULL == ikey || NULL == dmi || NULL == ctrlr_id) {
    UPLL_LOG_DEBUG("GetRenamedUncKey failed. Insufficient input parameters.");
    return UPLL_RC_ERR_GENERIC;
  }
  uint8_t rename = 0;
  val_rename_vnode *rename_vnode = reinterpret_cast<val_rename_vnode *>
      (ConfigKeyVal::Malloc(sizeof(val_rename_vnode)));
  if (!rename_vnode) {
    UPLL_LOG_DEBUG("rename_vnode NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  key_vbr_if_t *ctrlr_key = reinterpret_cast<key_vbr_if_t *>(ikey->get_key());
  if (NULL == ctrlr_key) {
    UPLL_LOG_DEBUG("ctrlr_key NULL");
    free(rename_vnode);
    return UPLL_RC_ERR_GENERIC;
  }

  uuu::upll_strncpy(rename_vnode->ctrlr_vtn_name,
                    ctrlr_key->vbr_key.vtn_key.vtn_name,
                    (kMaxLenVtnName + 1));
  rename_vnode->valid[UPLL_CTRLR_VTN_NAME_VALID] = UNC_VF_VALID;

  uuu::upll_strncpy(rename_vnode->ctrlr_vnode_name,
                    ctrlr_key->vbr_key.vbridge_name,
                    (kMaxLenVnodeName + 1));
  rename_vnode->valid[UPLL_CTRLR_VNODE_NAME_VALID] = UNC_VF_VALID;

  MoMgrImpl *mgr =
      reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>(GetMoManager(
          UNC_KT_VBRIDGE)));
  if (NULL == mgr) {
    UPLL_LOG_DEBUG("mgr NULL");
    free(rename_vnode);
    return UPLL_RC_ERR_GENERIC;
  }

  result_code = mgr->GetChildConfigKey(unc_key, NULL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetChildConfigKey failed %d", result_code);
    free(rename_vnode);
    mgr = NULL;
    return result_code;
  }
  if (!unc_key) {
    UPLL_LOG_DEBUG("unc_key NULL");
    free(rename_vnode);
    mgr = NULL;
    return UPLL_RC_ERR_GENERIC;
  }

  unc_key->AppendCfgVal(IpctSt::kIpcStValRenameVbr, rename_vnode);

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
        rename |= VTN_RENAME;
        SET_USER_DATA(ikey, unc_key);
        SET_USER_DATA_FLAGS(ikey, rename);
      }
      if (strcmp(reinterpret_cast<char *>(ctrlr_key->vbr_key.vbridge_name),
                 reinterpret_cast<char *>(vbr_if_key->vbr_key.vbridge_name))) {
        uuu::upll_strncpy(ctrlr_key->vbr_key.vbridge_name,
                      vbr_if_key->vbr_key.vbridge_name,
                      (kMaxLenVnodeName + 1));
        rename |= VBR_RENAME;
        SET_USER_DATA(ikey, unc_key);
        SET_USER_DATA_FLAGS(ikey, rename);
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
  pfcdrv_val_vbrif_policingmap *pfc_val_import = NULL;
  val_policingmap_t *val_policingmap = NULL;

  if (ikey->get_cfg_val() &&
     (ikey->get_cfg_val()->get_st_num() ==
     IpctSt::kIpcStPfcdrvValVbrifPolicingmap)) {
    UPLL_LOG_TRACE("val struct num (%d)", ikey->get_cfg_val()->get_st_num());
    pfc_val_import = reinterpret_cast<pfcdrv_val_vbrif_policingmap *>
             (ikey->get_cfg_val()->get_val());
    val_policingmap = &pfc_val_import->val_policing_map;
    UPLL_LOG_DEBUG("policer name (%s)", val_policingmap->policer_name);
  } else if (ikey->get_cfg_val() &&
     (ikey->get_cfg_val()->get_st_num() ==
     IpctSt:: kIpcStValPolicingmap)) {
      val_policingmap =
        reinterpret_cast<val_policingmap_t *>(GetVal(ikey));
  }
  if (!val_policingmap) {
    UPLL_LOG_DEBUG("val_policingmap NULL");
    free(rename_policingprofile);
    return UPLL_RC_SUCCESS;
  }

  uuu::upll_strncpy(rename_policingprofile->policingprofile_newname,
                    val_policingmap->policer_name,
                    (kMaxLenPolicingProfileName + 1));
  rename_policingprofile->valid[UPLL_IDX_RENAME_PROFILE_RPP] = UNC_VF_VALID;

  mgr = reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>(GetMoManager(
      UNC_KT_POLICING_PROFILE)));
  if (NULL == mgr) {
    UPLL_LOG_DEBUG("mgr policing profile NULL");
    free(rename_policingprofile);
    return UPLL_RC_ERR_GENERIC;
  }

  result_code = mgr->GetChildConfigKey(unc_key, NULL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetChildConfigKey failed %d", result_code);
    free(rename_policingprofile);
    mgr = NULL;
    return result_code;
  }
  if (!unc_key) {
    UPLL_LOG_DEBUG("unc_key NULL");
    free(rename_policingprofile);
    mgr = NULL;
    return UPLL_RC_ERR_GENERIC;
  }

  unc_key->AppendCfgVal(IpctSt::kIpcStValRenamePolicingprofile,
                        rename_policingprofile);
  SET_USER_DATA_CTRLR(unc_key, ctrlr_id);

  result_code = mgr->ReadConfigDB(unc_key, dt_type, UNC_OP_READ, dbop, dmi,
                                  RENAMETBL);
  if ((UPLL_RC_SUCCESS != result_code) &&
      (UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code)) {
    UPLL_LOG_DEBUG("GetRenamedUncKey failed. ReadConfigDB failed to read %d ",
                  result_code);
    DELETE_IF_NOT_NULL(unc_key);
    mgr = NULL;
    return result_code;
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
    rename |= POLICINGPROFILE_RENAME;
    SET_USER_DATA(ikey, unc_key);
    SET_USER_DATA_FLAGS(ikey, rename);
      }
    }
  }
  UPLL_LOG_TRACE("%s GetRenamedUncKey vbrifpm end",
                  ikey->ToStrAll().c_str());
  DELETE_IF_NOT_NULL(unc_key);
  mgr = NULL;
  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrIfPolicingMapMoMgr::GetRenamedControllerKey(
    ConfigKeyVal *ikey, upll_keytype_datatype_t dt_type, DalDmlIntf *dmi,
    controller_domain *ctrlr_dom) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code;
  ConfigKeyVal *okey = NULL;

  if (NULL == ikey || NULL == dmi || NULL == ctrlr_dom) {
    UPLL_LOG_DEBUG(
        "GetRenamedControllerKey failed. Insufficient input resources");
    return UPLL_RC_ERR_GENERIC;
  }
  DbSubOp dbop = { kOpReadSingle, kOpMatchCtrlr | kOpMatchDomain,
                   kOpInOutCtrlr };

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

  if (NULL != ctrlr_dom) {
    SET_USER_DATA_CTRLR_DOMAIN(okey, *ctrlr_dom)
  } else {
    UPLL_LOG_DEBUG("ctrlr null");
    DELETE_IF_NOT_NULL(okey);
    return UPLL_RC_ERR_GENERIC;
  }

  UPLL_LOG_DEBUG("ctrlr : %s; domain : %s", ctrlr_dom->ctrlr,
      ctrlr_dom->domain);
  if (UNC_KT_VBRIF_POLICINGMAP == ikey->get_key_type()) {
    uuu::upll_strncpy(
        reinterpret_cast<key_vbr_t *>(okey->get_key())->vtn_key.vtn_name,
        reinterpret_cast<key_vbr_if_t *>
        (ikey->get_key())->vbr_key.vtn_key.vtn_name,
        (kMaxLenVtnName + 1));

    UPLL_LOG_DEBUG("vtn name (%s) (%s)",
        reinterpret_cast<key_vbr *>(okey->get_key())->vtn_key.vtn_name,
        reinterpret_cast<key_vbr_if_t *>
        (ikey->get_key())->vbr_key.vtn_key.vtn_name);

    uuu::upll_strncpy(
        reinterpret_cast<key_vbr *>(okey->get_key())->vbridge_name,
        reinterpret_cast<key_vbr_if_t *>(ikey->get_key())->vbr_key.vbridge_name,
        (kMaxLenVnodeName + 1));

    UPLL_LOG_DEBUG("vbr name (%s) (%s)",
        reinterpret_cast<key_vbr *>(okey->get_key())->vbridge_name,
        reinterpret_cast<key_vbr_if_t *>
        (ikey->get_key())->vbr_key.vbridge_name);
  } else if (UNC_KT_VBRIF_POLICINGMAP_ENTRY == ikey->get_key_type()) {
    uuu::upll_strncpy(
        reinterpret_cast<key_vbr_t *>(okey->get_key())->vtn_key.vtn_name,
        reinterpret_cast<key_vbrif_policingmap_entry_t *>
        (ikey->get_key())->vbrif_key.vbr_key.vtn_key.vtn_name,
        (kMaxLenVtnName + 1));

    UPLL_LOG_DEBUG("vtn name (%s) (%s)",
        reinterpret_cast<key_vbr *>(okey->get_key())->vtn_key.vtn_name,
        reinterpret_cast<key_vbrif_policingmap_entry_t *>
        (ikey->get_key())->vbrif_key.vbr_key.vtn_key.vtn_name);

    uuu::upll_strncpy(
        reinterpret_cast<key_vbr *>(okey->get_key())->vbridge_name,
        reinterpret_cast<key_vbrif_policingmap_entry_t *>
        (ikey->get_key())->vbrif_key.vbr_key.vbridge_name,
        (kMaxLenVnodeName + 1));

    UPLL_LOG_DEBUG("vbr name (%s) (%s)",
        reinterpret_cast<key_vbr *>(okey->get_key())->vbridge_name,
        reinterpret_cast<key_vbrif_policingmap_entry_t *>
        (ikey->get_key())->vbrif_key.vbr_key.vbridge_name);
  }

  result_code = mgr->ReadConfigDB(okey, dt_type, UNC_OP_READ, dbop, dmi,
      RENAMETBL);
  if ((result_code != UPLL_RC_SUCCESS) &&
      (UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code)) {
    UPLL_LOG_DEBUG("GetRenamedControllerKey failed. ReadConfigDB failed "
        "to read vbr renametbl - %d",
        result_code);
    DELETE_IF_NOT_NULL(okey);
    return result_code;
  }
  if (UPLL_RC_SUCCESS == result_code) {
    val_rename_vnode_t *rename_val =
      reinterpret_cast<val_rename_vnode_t *>(GetVal(okey));
    if (!rename_val) {
      UPLL_LOG_DEBUG("rename_val null");
      DELETE_IF_NOT_NULL(okey);
      return UPLL_RC_ERR_GENERIC;
    }
    if (UNC_KT_VBRIF_POLICINGMAP == ikey->get_key_type()) {
      uuu::upll_strncpy(reinterpret_cast<key_vbr_if_t *>(ikey->get_key())
          ->vbr_key.vtn_key.vtn_name,
          rename_val->ctrlr_vtn_name,
          (kMaxLenVtnName + 1));
      uuu::upll_strncpy(reinterpret_cast<key_vbr_if_t *>(ikey->get_key())
          ->vbr_key.vbridge_name,
          rename_val->ctrlr_vnode_name,
          (kMaxLenVnodeName + 1));
    } else if (UNC_KT_VBRIF_POLICINGMAP_ENTRY == ikey->get_key_type()) {
      uuu::upll_strncpy(reinterpret_cast<key_vbrif_policingmap_entry_t *>
          (ikey->get_key())->vbrif_key.vbr_key.vtn_key.vtn_name,
          rename_val->ctrlr_vtn_name,
          (kMaxLenVtnName + 1));
      uuu::upll_strncpy(reinterpret_cast<key_vbrif_policingmap_entry_t *>
          (ikey->get_key())->vbrif_key.vbr_key.vbridge_name,
          rename_val->ctrlr_vnode_name,
          (kMaxLenVnodeName + 1));
    }
  }
  DELETE_IF_NOT_NULL(okey);
  mgr = NULL;

  if (UNC_KT_VBRIF_POLICINGMAP == ikey->get_key_type()) {
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

    SET_USER_DATA_CTRLR(okey, ctrlr_dom->ctrlr);

    val_policingmap_t *val_policingmap =
      reinterpret_cast<val_policingmap_t *>(GetVal(ikey));

    if (NULL == val_policingmap) {
      DELETE_IF_NOT_NULL(okey);
      return UPLL_RC_SUCCESS;
    }

    key_policingprofile_t *key_policingprofile =
      reinterpret_cast<key_policingprofile_t *>(okey->get_key());

    if (NULL == key_policingprofile) {
      UPLL_LOG_DEBUG("val or key struct is NULL");
      DELETE_IF_NOT_NULL(okey);
      return UPLL_RC_ERR_GENERIC;
    }

    uuu::upll_strncpy(key_policingprofile->policingprofile_name,
        val_policingmap->policer_name,
        (kMaxLenPolicingProfileName + 1));

    DbSubOp dbop = { kOpReadSingle, kOpMatchCtrlr, kOpInOutCtrlr };
    result_code = mgr->ReadConfigDB(okey, dt_type, UNC_OP_READ, dbop, dmi,
        RENAMETBL);
    if ((result_code != UPLL_RC_SUCCESS) &&
        (UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code)) {
      UPLL_LOG_DEBUG("GetRenamedControllerKey failed. ReadConfigDB failed "
          "to read policingprofile renametbl - %d",
          result_code);
      DELETE_IF_NOT_NULL(okey);
      return result_code;
    }
    if (UPLL_RC_SUCCESS == result_code) {
      val_rename_policingprofile_t *rename_policingprofile =
        reinterpret_cast<val_rename_policingprofile_t *>(GetVal(okey));
      if (!rename_policingprofile) {
        UPLL_LOG_DEBUG("rename_policingprofile NULL")
          DELETE_IF_NOT_NULL(okey);
        return UPLL_RC_ERR_GENERIC;
      }

      uuu::upll_strncpy(val_policingmap->policer_name,
          rename_policingprofile->policingprofile_newname,
          (kMaxLenPolicingProfileName + 1));
    }
    DELETE_IF_NOT_NULL(okey);
  }
  return UPLL_RC_SUCCESS;
}

bool VbrIfPolicingMapMoMgr::CompareValidValue(void *&val1, void *val2,
                                              bool audit) {
  UPLL_FUNC_TRACE;
  bool invalid_attr = true;
  val_policingmap_t *val_pm1 = reinterpret_cast<val_policingmap_t *>(val1);
  val_policingmap_t *val_pm2 = reinterpret_cast<val_policingmap_t *>(val2);
  if (UNC_VF_INVALID == val_pm1->valid[UPLL_IDX_POLICERNAME_PM] &&
      UNC_VF_VALID == val_pm2->valid[UPLL_IDX_POLICERNAME_PM]) {
      val_pm1->valid[UPLL_IDX_POLICERNAME_PM] = UNC_VF_VALID_NO_VALUE;
  } else if (UNC_VF_VALID == val_pm1->valid[UPLL_IDX_POLICERNAME_PM] &&
      UNC_VF_VALID == val_pm2->valid[UPLL_IDX_POLICERNAME_PM]) {
    if (!strcmp(reinterpret_cast<char*>(val_pm1->policer_name),
             reinterpret_cast<char*>(val_pm2->policer_name))) {
      val_pm1->valid[UPLL_IDX_POLICERNAME_PM] = UNC_VF_INVALID;
    }
  }
  if ((UNC_VF_VALID == (uint8_t)val_pm1->valid[UPLL_IDX_POLICERNAME_PM]) ||
      (UNC_VF_VALID_NO_VALUE ==
       (uint8_t)val_pm1->valid[UPLL_IDX_POLICERNAME_PM])) {
      invalid_attr = false;
  }

  return invalid_attr;
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

#if 0
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
    UPLL_LOG_DEBUG("UpdateConfigStatus UNC_OP_CREATE %d", cs_status);
    val->cs_row_status = cs_status;
    if (val->cs_row_status != UNC_CS_NOT_SUPPORTED) {
      val->cs_row_status = cs_status;
      val->cs_attr[0] = cs_status;
    }
  } else {
    UPLL_LOG_DEBUG("Operation Not Supported.");
    return UPLL_RC_ERR_GENERIC;
  }

  UPLL_LOG_DEBUG("Update Config Status Successfull.");
  return UPLL_RC_SUCCESS;
}
#endif

upll_rc_t VbrIfPolicingMapMoMgr::UpdateAuditConfigStatus(
    unc_keytype_configstatus_t cs_status,
    uuc::UpdateCtrlrPhase phase,
    ConfigKeyVal *&ckv_running,
    DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  if (NULL == ckv_running)
    return UPLL_RC_ERR_GENERIC;
  val_policingmap_t *val;
  val = reinterpret_cast<val_policingmap_t *>
                              (GetVal(ckv_running));
  if (NULL == val) {
    UPLL_LOG_DEBUG("vbr_val NULL");
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
        return UPLL_RC_ERR_GENERIC;
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
    DELETE_IF_NOT_NULL(tmp1);
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
  bool cfgval_ctrlr = false;


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
  }

  if ((okey) && (okey->get_key())) {
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
    /* VlanmapOnBoundary: Added vlink case */
    case UNC_KT_VLINK: {
      uint8_t *vnode_name;
      uint8_t *vnode_ifname;
      uint8_t flags = 0;
      val_vlink *vlink_val = reinterpret_cast<val_vlink *>(GetVal(parent_key));
      if (!vlink_val) {
         UPLL_LOG_TRACE("ERROR");
         if (!okey || !(okey->get_key()))
           FREE_IF_NOT_NULL(vbr_if_key);
        return UPLL_RC_ERR_GENERIC;
      }

      GET_USER_DATA_FLAGS(parent_key->get_cfg_val(), flags);
      flags &=  VLINK_FLAG_NODE_POS;
      UPLL_LOG_DEBUG("Vlink flag node position %d", flags);

      if (flags == kVlinkVnode2) {
         cfgval_ctrlr = true;
         vnode_name = vlink_val->vnode2_name;
         vnode_ifname = vlink_val->vnode2_ifname;
      } else {
         vnode_name = vlink_val->vnode1_name;
         vnode_ifname = vlink_val->vnode1_ifname;
      }
      uuu::upll_strncpy(vbr_if_key->vbr_key.vtn_key.vtn_name,
                        reinterpret_cast<key_vlink *>(pkey)->vtn_key.vtn_name,
                        (kMaxLenVtnName + 1));
      if (vnode_name)
         uuu::upll_strncpy(vbr_if_key->vbr_key.vbridge_name, vnode_name,
                          (kMaxLenVnodeName + 1));
      if (vnode_ifname)
         uuu::upll_strncpy(vbr_if_key->if_name, vnode_ifname,
                          (kMaxLenInterfaceName + 1));
      break;
    }
    default:
      if (vbr_if_key) free(vbr_if_key);
      return UPLL_RC_ERR_GENERIC;
  }


  if ((okey) && !(okey->get_key())) {
    UPLL_LOG_DEBUG("okey not null and flow list name updated");
    okey->SetKey(IpctSt::kIpcStKeyVbrIf, vbr_if_key);
  }

  if (!okey) {
       okey = new ConfigKeyVal(UNC_KT_VBRIF_POLICINGMAP,
                IpctSt::kIpcStKeyVbrIf, vbr_if_key, NULL);
  }

  /* In case of node2 of vlink, set the user_data
   * from vlink's cfg_val */
  if (cfgval_ctrlr) {
    SET_USER_DATA(okey, parent_key->get_cfg_val());
  } else {
    SET_USER_DATA(okey, parent_key);
  }

  return result_code;
}

upll_rc_t VbrIfPolicingMapMoMgr::ValidateMessage(IpcReqRespHeader *req,
                                                 ConfigKeyVal *key) {
  UPLL_FUNC_TRACE;

  upll_rc_t rt_code = UPLL_RC_ERR_GENERIC;

  if ((NULL == req) || (NULL == key)) {
    UPLL_LOG_DEBUG("ConfigKeyval/IpcReqRespHeader is NULL");
    return UPLL_RC_ERR_BAD_REQUEST;
  }

  key_vbr_if_t *key_vbrif = NULL;
  key_vbrif_policingmap_entry_t *key_vbrif_policingmap_entry = NULL;
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

  if (UNC_KT_VBRIF_POLICINGMAP == key->get_key_type()) {
    if ((req->datatype == UPLL_DT_IMPORT) && (req->operation == UNC_OP_READ ||
         req->operation == UNC_OP_READ_SIBLING ||
         req->operation == UNC_OP_READ_SIBLING_BEGIN ||
         req->operation == UNC_OP_READ_NEXT ||
         req->operation == UNC_OP_READ_BULK ||
         req->operation == UNC_OP_READ_SIBLING_COUNT)) {
      return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT;
    }

    if (key->get_st_num() != IpctSt::kIpcStKeyVbrIf) {
      UPLL_LOG_DEBUG("Invalid key structure received. struct num - %d",
          key->get_st_num());
      return UPLL_RC_ERR_BAD_REQUEST;
    }
    key_vbrif = reinterpret_cast<key_vbr_if_t *>(key->get_key());
  } else if (UNC_KT_VBRIF_POLICINGMAP_ENTRY == key->get_key_type()) {
    if ((req->datatype == UPLL_DT_IMPORT) && (req->operation == UNC_OP_READ ||
         req->operation == UNC_OP_READ_SIBLING ||
         req->operation == UNC_OP_READ_SIBLING_BEGIN ||
         req->operation == UNC_OP_READ_NEXT ||
         req->operation == UNC_OP_READ_BULK ||
         req->operation == UNC_OP_READ_SIBLING_COUNT)) {
      return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT;
    }

    if (req->datatype != UPLL_DT_STATE) {
       UPLL_LOG_DEBUG(" Unsupported Datatype (%d)", req->datatype);
       return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT;
    }
    if (key->get_st_num() != IpctSt::kIpcStKeyVbrifPolicingmapEntry) {
      UPLL_LOG_DEBUG("Invalid key structure received. struct num - %d",
          key->get_st_num());
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
  VbrIfMoMgr *mgrvbrif =
      reinterpret_cast<VbrIfMoMgr *>(const_cast<MoManager *>(GetMoManager(
              UNC_KT_VBR_IF)));

  if (NULL == mgrvbrif) {
    UPLL_LOG_DEBUG("Unable to get VBR_IF object to validate key_vbrif");
    return UPLL_RC_ERR_GENERIC;
  }

  rt_code = mgrvbrif->ValidateVbrifKey(key_vbrif);

  if (rt_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("key_vbrif validation failed");
    return rt_code;
  }

  if (UNC_KT_VBRIF_POLICINGMAP_ENTRY == key->get_key_type()) {
    /* validate seq number */
    if ((req->operation != UNC_OP_READ_SIBLING_COUNT) &&
        (req->operation != UNC_OP_READ_SIBLING_BEGIN)) {
      if (!ValidateNumericRange(key_vbrif_policingmap_entry->sequence_num,
                                (uint8_t) kMinPolicingProfileSeqNum,
                                (uint8_t) kMaxPolicingProfileSeqNum, true,
                                true)) {
        UPLL_LOG_DEBUG("Sequence num syntax validation failed :Err Code - %d",
                       rt_code);
        return UPLL_RC_ERR_CFG_SYNTAX;
      }
    } else {
      key_vbrif_policingmap_entry->sequence_num = 0;
    }
    UPLL_LOG_DEBUG(
        "key struct validation is success for UNC_KT_VBR_POLICINGMAP_ENTRY");
    return UPLL_RC_SUCCESS;
  }

  UPLL_LOG_TRACE(" key struct validation is success");

  /** read datatype, operation, options from IpcReqRespHeader */
  if (NULL == req) {
    UPLL_LOG_DEBUG("IpcReqRespHeader is NULL");
    return UPLL_RC_ERR_BAD_REQUEST;
  }

  /** Use  VtnPolicingMapMoMgr::ValidatePolicingMapValue
   *  to validate value structure */
  rt_code = VtnPolicingMapMoMgr::ValidatePolicingMapValue(key, req->operation);

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

  if ((NULL == req) || (NULL == ikey)) {
    UPLL_LOG_DEBUG("IpcReqRespHeader/ConfigKeyval is NULL");
    return rt_code;
  }

  if (!ctrlr_name)
    ctrlr_name = static_cast<char *>(ikey->get_user_data());

  if (NULL == ctrlr_name) {
    UPLL_LOG_DEBUG(" ctrlr_name is NULL");
    return rt_code;
  }

  bool result_code = false;
  uint32_t max_instance_count = 0;
  const uint8_t *attrs = NULL;
  uint32_t max_attrs = 0;

  UPLL_LOG_TRACE("ctrlr_name (%s), operation : (%d)",
                 ctrlr_name, req->operation);

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


  val_policingmap_t *val_policingmap =
      reinterpret_cast<val_policingmap_t *>(GetVal(ikey));
  if (val_policingmap) {
    if (max_attrs > 0) {
      if ((val_policingmap->valid[UPLL_IDX_POLICERNAME_PM] == UNC_VF_VALID)
          || (val_policingmap->valid[UPLL_IDX_POLICERNAME_PM]
            == UNC_VF_VALID_NO_VALUE)) {
        if (attrs[unc::capa::vbr_if_policingmap::kCapPolicername] == 0) {
          val_policingmap->valid[UPLL_IDX_POLICERNAME_PM] =
            UNC_VF_NOT_SUPPORTED;
          UPLL_LOG_DEBUG("Policername attr is not supported by ctrlr");
          return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
        }
      }
    } else {
      UPLL_LOG_DEBUG("Attribute list is empty for operation %d",
                     req->operation);
      return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
    }
  }
  return UPLL_RC_SUCCESS;
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
  DELETE_IF_NOT_NULL(okey);
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

bool VbrIfPolicingMapMoMgr::IsValidKey(void *key, uint64_t index,
                                       MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  key_vbr_if *if_key = reinterpret_cast<key_vbr_if *>(key);
  upll_rc_t ret_val = UPLL_RC_SUCCESS;
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
    ConfigKeyVal **err_ckv,
    TxUpdateUtil *tx_util,
    TcConfigMode config_mode,
    std::string vtn_name)  {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  DalResultCode db_result;
  ConfigKeyVal *req = NULL, *nreq = NULL, *ck_main = NULL;
  controller_domain_t ctrlr_dom;
  ctrlr_dom.ctrlr = NULL;
  ctrlr_dom.domain = NULL;
  DalCursor *dal_cursor_handle = NULL;
  uint8_t db_flag = 0;
  uint8_t flag = 0;

  upll_keytype_datatype_t vext_datatype =  UPLL_DT_CANDIDATE;

  // TxUpdate skipped if config mode is VIRTUAL
  if (config_mode == TC_CONFIG_VIRTUAL) {
    return UPLL_RC_SUCCESS;
  }

  if (affected_ctrlr_set == NULL) {
    UPLL_LOG_DEBUG("affected_ctrlr_set is NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  //  Create phase is skipped and is performed during update phase.
  //  This is done for the following reason,
  //  Vbridge Interface configuration is commited to pfc,
  //  then portmap and policingmap are created and commited.
  //  The policingmap should be created inside the vexternal whose information
  //  will be sent to pfc during update of vbr_if but create request
  //  for policingmap is sent in create phase due to which commit fails.
  if (phase == uuc::kUpllUcpCreate) {
    return UPLL_RC_SUCCESS;
  }
  unc_keytype_operation_t op_arr[2] = {UNC_OP_INVALID, UNC_OP_INVALID};
  int nop = 0;
  // Create and Update are done is same phase.
  if (phase == uuc::kUpllUcpUpdate) {
    op_arr[0] = UNC_OP_UPDATE;
    op_arr[1] = UNC_OP_CREATE;
    nop = 2;
  } else if (phase == uuc::kUpllUcpDelete) {
    op_arr[0] = UNC_OP_DELETE;
    nop = 1;
  } else {
    UPLL_LOG_INFO("Invalid phase");
    return UPLL_RC_ERR_GENERIC;
  }
  for (int i = 0; i < nop; i++) {
    dal_cursor_handle = NULL;
    unc_keytype_operation_t op = op_arr[i];
    /* retreives the delta of running and audit configuration */
    UPLL_LOG_DEBUG("Operation is %d", op_arr[i]);
    if (tx_util->GetErrCount() > 0) {
      UPLL_LOG_ERROR("Error Occured From Driver Side..Exiting the loop");
      return UPLL_RC_ERR_GENERIC;
    }
    result_code = DiffConfigDB(UPLL_DT_CANDIDATE, UPLL_DT_RUNNING,
        op, req, nreq, &dal_cursor_handle, dmi, config_mode, vtn_name,
        MAINTBL);
    while (result_code == UPLL_RC_SUCCESS) {
      if (tx_util->GetErrCount() > 0) {
        UPLL_LOG_ERROR("TxUpdateUtil says exit the loop.");
        dmi->CloseCursor(dal_cursor_handle, true);
        DELETE_IF_NOT_NULL(nreq);
        DELETE_IF_NOT_NULL(req);
        return UPLL_RC_ERR_GENERIC;
      }

      //  Get Next Record
      ck_main = NULL;
      op = op_arr[i];
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
          /* fall through intended */
          UPLL_LOG_DEBUG("DupConfigKeyVal CREATE/UPDATE (%d)", op);
          result_code = DupConfigKeyVal(ck_main, req, MAINTBL);
          if (result_code != UPLL_RC_SUCCESS) {
            DELETE_IF_NOT_NULL(req);
            DELETE_IF_NOT_NULL(nreq);
            dmi->CloseCursor(dal_cursor_handle, true);
            UPLL_LOG_TRACE("DupConfigKeyVal failed %d\n", result_code);
            return result_code;
          }
          break;
        default:
          UPLL_LOG_DEBUG("TxUpdateController Invalid operation");
          DELETE_IF_NOT_NULL(req);
          DELETE_IF_NOT_NULL(nreq);
          dmi->CloseCursor(dal_cursor_handle, true);
          return UPLL_RC_ERR_GENERIC;
      }

      // Perform mode specific sematic check
      // check the record existence in running database.
      if (TC_CONFIG_GLOBAL != config_mode) {
        result_code = PerformModeSpecificSemanticCheck(ck_main, dmi, session_id,
                                                       config_id, op, keytype,
                                                       config_mode, vtn_name);
        if ((result_code != UPLL_RC_SUCCESS) &&
            (result_code != UPLL_RC_ERR_INSTANCE_EXISTS)) {
          UPLL_LOG_DEBUG("Mode specific semantic failed %d\n", result_code);
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
        UPLL_LOG_DEBUG("ctrlr_dom.ctrlr NULL");
        DELETE_IF_NOT_NULL(ck_main);
        DELETE_IF_NOT_NULL(req);
        DELETE_IF_NOT_NULL(nreq);
        dmi->CloseCursor(dal_cursor_handle, true);
        return UPLL_RC_ERR_GENERIC;
      }
      GET_USER_DATA_FLAGS(ck_main, db_flag);
      UPLL_LOG_DEBUG("db_flag (%d)", db_flag);

      if (!(SET_FLAG_PORTMAP & db_flag)) {
        if (op != UNC_OP_UPDATE) {
          UPLL_LOG_DEBUG("op != UNC_OP_UPDATE continue");
          DELETE_IF_NOT_NULL(ck_main);
          continue;
        } else {
          GET_USER_DATA_FLAGS(nreq, flag);
          UPLL_LOG_DEBUG("flag (%d)", flag);
          if (!(SET_FLAG_PORTMAP & flag)) {
            UPLL_LOG_DEBUG("SET_FLAG_PORTMAP & flag");
            DELETE_IF_NOT_NULL(ck_main);
            continue;
          }
          op = UNC_OP_DELETE;
          vext_datatype = UPLL_DT_RUNNING;
          UPLL_LOG_DEBUG("Data type changes as RUNNING op (%d) data type(%d)",
              op, vext_datatype);
          db_flag = flag;
        }
      } else if (UNC_OP_UPDATE == op) {
        GET_USER_DATA_FLAGS(nreq, flag);
        UPLL_LOG_DEBUG("flag (%d)", flag);
        if (!(SET_FLAG_PORTMAP & flag)) {
          op = UNC_OP_CREATE;
          vext_datatype = UPLL_DT_CANDIDATE;
          UPLL_LOG_DEBUG("Data type changes as CANDIDATE op %d,"
                         "datatype %d", op, vext_datatype);
        }
      }
      pfcdrv_val_vbrif_policingmap *pfc_val =
        reinterpret_cast<pfcdrv_val_vbrif_policingmap *>
        (ConfigKeyVal::Malloc(sizeof(pfcdrv_val_vbrif_policingmap_t)));
      pfcdrv_val_vbrif_vextif *pfc_val_ext =
        reinterpret_cast<pfcdrv_val_vbrif_vextif *>\
        (ConfigKeyVal::Malloc(sizeof(pfcdrv_val_vbrif_vextif_t)));

      if (UNC_OP_DELETE == op) {
        vext_datatype = UPLL_DT_RUNNING;
      }

      UPLL_LOG_DEBUG("GetVexternalInformation (%d)", vext_datatype);
      result_code = GetVexternalInformation(ck_main, vext_datatype, pfc_val,
          pfc_val_ext, db_flag, (const char*)ctrlr_dom.ctrlr, dmi);
      if (UPLL_RC_SUCCESS != result_code) {
        UPLL_LOG_DEBUG("GetVexternalInformation fail");
        DELETE_IF_NOT_NULL(ck_main);
        free(pfc_val_ext);
        free(pfc_val);
        DELETE_IF_NOT_NULL(req);
        DELETE_IF_NOT_NULL(nreq);
        dmi->CloseCursor(dal_cursor_handle, true);
        return result_code;
      }
      upll_keytype_datatype_t dt_type = (op == UNC_OP_DELETE)?
        UPLL_DT_RUNNING:UPLL_DT_CANDIDATE;

      UPLL_LOG_DEBUG("Controller : %s; Domain : %s", ctrlr_dom.ctrlr,
          ctrlr_dom.domain);
      result_code = GetRenamedControllerKey(ck_main, dt_type,
          dmi, &ctrlr_dom);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("GetRenamedControllerKey fail");
        DELETE_IF_NOT_NULL(ck_main);
        free(pfc_val_ext);
        free(pfc_val);
        DELETE_IF_NOT_NULL(req);
        DELETE_IF_NOT_NULL(nreq);
        dmi->CloseCursor(dal_cursor_handle, true);
        return result_code;
      }
      ConfigKeyVal *ckv_driver = NULL;
      result_code = DupConfigKeyVal(ckv_driver, ck_main, MAINTBL);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_TRACE("DupConfigKeyVal failed %d\n", result_code);
        DELETE_IF_NOT_NULL(req);
        DELETE_IF_NOT_NULL(nreq);
        DELETE_IF_NOT_NULL(ck_main);
        dmi->CloseCursor(dal_cursor_handle, true);
        return result_code;
      }

      val_policingmap_t* val =
          reinterpret_cast<val_policingmap_t *>(GetVal(ckv_driver));  // req
      UPLL_LOG_DEBUG("val_policingmap_t (%s)", val->policer_name);

      memcpy(&pfc_val->val_policing_map, val, sizeof(val_policingmap_t));
      pfc_val->valid[PFCDRV_IDX_VAL_POLICINGMAP_PM] = UNC_VF_VALID;

      memcpy(&pfc_val->val_vbrif_vextif, pfc_val_ext,
          sizeof(pfcdrv_val_vbrif_vextif_t));
      pfc_val->valid[PFCDRV_IDX_VAL_VBRIF_VEXTIF_PM] = UNC_VF_VALID;
      FREE_IF_NOT_NULL(pfc_val_ext);
      ckv_driver->SetCfgVal(new ConfigVal(
            IpctSt::kIpcStPfcdrvValVbrifPolicingmap,
            pfc_val));

      if (op == UNC_OP_UPDATE) {
        pfcdrv_val_vbrif_policingmap *pfc_val_old =
          reinterpret_cast<pfcdrv_val_vbrif_policingmap *>
          (ConfigKeyVal::Malloc(sizeof(pfcdrv_val_vbrif_policingmap_t)));
        val_policingmap_t* val =
          reinterpret_cast<val_policingmap_t *>(GetVal(nreq));
        memcpy(&pfc_val_old->val_policing_map, val, sizeof(val_policingmap_t));
        pfc_val_old->valid[PFCDRV_IDX_VAL_POLICINGMAP_PM] = UNC_VF_VALID;
        ckv_driver->AppendCfgVal(
            new ConfigVal(IpctSt::kIpcStPfcdrvValVbrifPolicingmap,
                          pfc_val_old));
      }

      affected_ctrlr_set->insert
        (string(reinterpret_cast<char *>(ctrlr_dom.ctrlr)));

      UPLL_LOG_DEBUG("Controller : %s; Domain : %s", ctrlr_dom.ctrlr,
          ctrlr_dom.domain);
      DELETE_IF_NOT_NULL(ck_main);

      ConfigKeyVal *ckv_unc = NULL;
      result_code = DupConfigKeyVal(ckv_unc, req, MAINTBL);
      if (result_code != UPLL_RC_SUCCESS) {
        DELETE_IF_NOT_NULL(req);
        DELETE_IF_NOT_NULL(nreq);
        DELETE_IF_NOT_NULL(ckv_driver);
        dmi->CloseCursor(dal_cursor_handle, true);
        UPLL_LOG_TRACE("DupConfigKeyVal failed %d\n", result_code);
        return result_code;
      }

      result_code = tx_util->EnqueueRequest(session_id, config_id,
                                            UPLL_DT_CANDIDATE, op, dmi,
                                            ckv_driver, ckv_unc, string());
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("EnqueueRequest failed %d", result_code);
        DELETE_IF_NOT_NULL(req);
        DELETE_IF_NOT_NULL(nreq);
        DELETE_IF_NOT_NULL(ckv_driver);
        DELETE_IF_NOT_NULL(ckv_unc);
        dmi->CloseCursor(dal_cursor_handle, true);
        return result_code;
      }
    }
    dmi->CloseCursor(dal_cursor_handle, true);
    DELETE_IF_NOT_NULL(req);
    DELETE_IF_NOT_NULL(nreq);
  }
  result_code = (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) ?
    UPLL_RC_SUCCESS : result_code;
  return result_code;
}

upll_rc_t VbrIfPolicingMapMoMgr::SetVlinkPortmapConfiguration(
    ConfigKeyVal *ikey,
    upll_keytype_datatype_t dt_type,
    DalDmlIntf *dmi,
    InterfacePortMapInfo flag,
    unc_keytype_operation_t oper,
    TcConfigMode config_mode,
    string vtn_name) {
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

  DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutFlag|kOpInOutCtrlr};
  result_code = ReadConfigDB(ckv, dt_type ,
      UNC_OP_READ, dbop, dmi, MAINTBL);
  if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
    UPLL_LOG_DEBUG("No Recrods in vbr_if_policingmap Table");
    DELETE_IF_NOT_NULL(ckv);
    return UPLL_RC_SUCCESS;
  }
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("Read ConfigDB failure %d", result_code);
    DELETE_IF_NOT_NULL(ckv);
    return result_code;
  }
  uint8_t  flag_port_map = 0;
  GET_USER_DATA_FLAGS(ckv, flag_port_map);
  val_policingmap_t *val_pm = reinterpret_cast<val_policingmap_t *>
      (GetVal(ckv));
  if (flag & VLINK_CONFIGURED) {
    if (flag_port_map  & SET_FLAG_VLINK) {
      UPLL_LOG_DEBUG("Vlink Flag is already set in DB");
      DELETE_IF_NOT_NULL(ckv);
      return UPLL_RC_SUCCESS;
    }
    UPLL_LOG_DEBUG("flag & VLINK_CONFIGURED");
    flag_port_map |= SET_FLAG_VLINK;
  } else if (flag & PORTMAP_CONFIGURED) {
    if (flag_port_map  & SET_FLAG_PORTMAP) {
      UPLL_LOG_DEBUG("Port-map Flag is already set in DB");
      DELETE_IF_NOT_NULL(ckv);
      return UPLL_RC_SUCCESS;
    }
    UPLL_LOG_DEBUG("flag & PORTMAP_CONFIGURED");
    flag_port_map |= SET_FLAG_PORTMAP;
  } else if (flag & VLINK_PORTMAP_CONFIGURED) {
    if (flag_port_map  & SET_FLAG_VLINK_PORTMAP) {
      UPLL_LOG_DEBUG("Port-map Flag is already set in DB");
      DELETE_IF_NOT_NULL(ckv);
      return UPLL_RC_SUCCESS;
    }
    UPLL_LOG_DEBUG("flag & PORTMAP_CONFIGURED");
    flag_port_map |= SET_FLAG_VLINK_PORTMAP;
  } else {
    UPLL_LOG_DEBUG("No portmap/vlink");
    flag_port_map &= SET_FLAG_NO_VLINK_PORTMAP;
  }
  SET_USER_DATA_FLAGS(ckv, flag_port_map);
  DbSubOp dbop_up = { kOpNotRead, kOpMatchNone, kOpInOutFlag };
  result_code = UpdateConfigDB(ckv, dt_type, UNC_OP_UPDATE,
      dmi, &dbop_up, config_mode, vtn_name, MAINTBL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("UpdateConfigDB failure %d", result_code);
    DELETE_IF_NOT_NULL(ckv);
    return result_code;
  }

  if (UNC_VF_VALID == val_pm->valid[UPLL_IDX_POLICERNAME_PM]) {
    // Update the controller reference count for policing profile  based on the
    // value of  portmap flag
    // 1)Get vbrif associated ctrlr name and invoke the PP and PPE functions to
    // check the refcount capability and update the refcount or create the
    // record in policingprofilectrltbl and policingprofileentryctrltbl.
    // 2)Create the record in policingprofileentryctrltbl
    if (flag_port_map & SET_FLAG_PORTMAP) {
      // If the portmap flag is configured then increment the reference count
      //  Set the operation as CREATE
      oper = UNC_OP_CREATE;
    } else {
      // If the portmap flag is not configured/cleared then decrement
      // the reference count set the operation as DELETE
      oper = UNC_OP_DELETE;
    }

      result_code = UpdateRefCountInPPCtrlr(ckv, dt_type, dmi,
                                          oper, config_mode, vtn_name);
      if (UPLL_RC_SUCCESS != result_code) {
        UPLL_LOG_DEBUG("Err while updating the policingprofile ctrlr table(%d)",
                     result_code);
        DELETE_IF_NOT_NULL(ckv);
        return result_code;
      }
    }
  DELETE_IF_NOT_NULL(ckv);
  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrIfPolicingMapMoMgr::GetVexternalInformation(ConfigKeyVal* ck_main,
  upll_keytype_datatype_t dt_type,
  pfcdrv_val_vbrif_policingmap_t *& pfc_val,
  pfcdrv_val_vbrif_vextif_t *&pfc_val_ext,
  uint8_t db_flag, const char *ctrlr_id, DalDmlIntf *dmi) {
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
        vexternal, vex_if, flags, ctrlr_id);
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
    DELETE_IF_NOT_NULL(ckv);
    return UPLL_RC_ERR_NOT_ALLOWED_AT_THIS_TIME;
  }
  FREE_IF_NOT_NULL(vex_if);
  FREE_IF_NOT_NULL(vexternal);
  DELETE_IF_NOT_NULL(ckv);
  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrIfPolicingMapMoMgr::ConstructReadDetailResponse(
    ConfigKeyVal *ikey,
    ConfigKeyVal *drv_resp_ckv,
    upll_keytype_datatype_t dt_type,
    unc_keytype_operation_t op,
    DbSubOp dbop,
    DalDmlIntf *dmi,
    ConfigKeyVal **okey,
    controller_domain ctrlr_dom) {
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
       delete tkey;
       tkey = NULL;
      if ((temp_cfg_val = temp_cfg_val->get_next_cfg_val()) == NULL) {
        UPLL_LOG_DEBUG("No val_policingmap_switch_st in configkeyval");
        continue;
      }

      if (IpctSt::kIpcStValPolicingmapSwitchSt != temp_cfg_val->get_st_num()) {
        UPLL_LOG_DEBUG("No PolicingmapSwitchSt entries returned by driver");
        continue;
      }

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
          key_vbr_if_t *vbrif_pm_key =
              reinterpret_cast<key_vbr_if_t*>(ikey->get_key());
          ConfigKeyVal *vbrif_key_val = NULL;
          unc::upll::kt_momgr::VbrIfMoMgr *vbrifmgr =
              reinterpret_cast<unc::upll::kt_momgr::VbrIfMoMgr *>
              (const_cast<MoManager *>(GetMoManager(UNC_KT_VBR_IF)));
          if (NULL == vbrifmgr) {
            free(val_switch_st);
            delete tmp_okey;
            return UPLL_RC_ERR_GENERIC;
          }
          /* GetVbrIfFromVExternal() will be called only during
           * READ/READ_SIB/READ_SIB_BEGIN,
           * hence dt_type is always UPLL_DT_RUNNING
           */
          result_code = vbrifmgr->GetVbrIfFromVExternal(
              vbrif_pm_key->vbr_key.vtn_key.vtn_name,
              drv_val_switch_st->vnode_if_name,
              vbrif_key_val,
              dmi, ctrlr_dom);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("Get vBridge info failed err code (%d)",
                           result_code);
            free(val_switch_st);
            delete tmp_okey;
            return result_code;
          }

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
          DELETE_IF_NOT_NULL(vbrif_key_val);
        }
        tmp_okey->AppendCfgVal(IpctSt::kIpcStValPolicingmapSwitchSt,
                               val_switch_st);
        temp_cfg_val = temp_cfg_val->get_next_cfg_val();
        if (temp_cfg_val == NULL)
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
    DELETE_IF_NOT_NULL(dup_key);
    return result_code;
  }
  val_policingmap_t *val_pm = NULL;
  val_pm = reinterpret_cast<val_policingmap_t*>(GetVal(dup_key));
  if (val_pm == NULL) {
    DELETE_IF_NOT_NULL(dup_key);
    return UPLL_RC_ERR_GENERIC;
  }
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
  DELETE_IF_NOT_NULL(dup_key);
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
  DELETE_IF_NOT_NULL(ppe_ckv);
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
     DELETE_IF_NOT_NULL(tkey);
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

upll_rc_t VbrIfPolicingMapMoMgr::ConstructReadEntryDetailResponse(
    ConfigKeyVal *ikey,
    ConfigKeyVal *drv_resp_ckv,
    upll_keytype_datatype_t dt_type,
    unc_keytype_operation_t op,
    DbSubOp dbop,
    DalDmlIntf *dmi,
    ConfigKeyVal **okey,
    controller_domain ctrlr_dom) {
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
      delete tkey;
      tkey = NULL;

      if ((temp_cfg_val = temp_cfg_val->get_next_cfg_val()) == NULL) {
        UPLL_LOG_DEBUG("Next Value structure is null");
        continue;
      }
      if (IpctSt::kIpcStValPolicingmapSwitchSt != temp_cfg_val->get_st_num()) {
        UPLL_LOG_DEBUG("No PolicingmapSwitchSt entries returned by driver");
        continue;
      }
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
          key_vbrif_policingmap_entry *vbrif_pme_key =
              reinterpret_cast<key_vbrif_policingmap_entry *>(ikey->get_key());
          ConfigKeyVal *vbrif_key_val = NULL;
          unc::upll::kt_momgr::VbrIfMoMgr *vbrifmgr =
              reinterpret_cast<unc::upll::kt_momgr::VbrIfMoMgr *>
              (const_cast<MoManager *>(GetMoManager(UNC_KT_VBR_IF)));
          if (NULL == vbrifmgr) {
            if (val_switch_st) free(val_switch_st);
            DELETE_IF_NOT_NULL(tmp_okey);
            return UPLL_RC_ERR_GENERIC;
          }
          /* GetVbrIfFromVExternal() will be called only during
           * READ/READ_SIB/READ_SIB_BEGIN,
           * hence dt_type is always UPLL_DT_RUNNING
           */
          result_code = vbrifmgr->GetVbrIfFromVExternal(
              vbrif_pme_key->vbrif_key.vbr_key.vtn_key.vtn_name,
              drv_val_switch_st->vnode_if_name,
              vbrif_key_val,
              dmi, ctrlr_dom);
          if ((result_code != UPLL_RC_SUCCESS) &&
              (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE)) {
            UPLL_LOG_DEBUG("Get vBridge info failed err code (%d)",
                           result_code);
            if (val_switch_st) free(val_switch_st);
            DELETE_IF_NOT_NULL(tmp_okey);
            DELETE_IF_NOT_NULL(vbrif_key_val);
            return result_code;
          }

          if (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
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
        tmp_okey->AppendCfgVal(IpctSt::kIpcStValPolicingmapSwitchSt,
                               val_switch_st);
        temp_cfg_val = temp_cfg_val->get_next_cfg_val();
        if (temp_cfg_val == NULL) {
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
    DELETE_IF_NOT_NULL(dup_key);
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
  delete ppe_ckv;
  if (result_code != UPLL_RC_ERR_INSTANCE_EXISTS) {
    UPLL_LOG_DEBUG("ReadDetailEntry error (%d)", result_code);
    delete dup_key;
    delete temp_key;
    temp_key = NULL;
    return result_code;
  }

  result_code =  GetChildEntryConfigKey(l_key, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("DupConfigKeyVal Faill in ReadSiblingMo for l_key");
    DELETE_IF_NOT_NULL(dup_key);
    DELETE_IF_NOT_NULL(temp_key);
    return result_code;
  }
  GET_USER_DATA_CTRLR_DOMAIN(temp_key, ctrlr_dom);
  SET_USER_DATA_CTRLR_DOMAIN(l_key, ctrlr_dom);
  result_code = ValidateCapability(req, ikey,
                                   reinterpret_cast<char *>(ctrlr_dom.ctrlr));

  if (result_code != UPLL_RC_SUCCESS) {
    DELETE_IF_NOT_NULL(dup_key);
    DELETE_IF_NOT_NULL(temp_key);
    DELETE_IF_NOT_NULL(l_key);
    UPLL_LOG_DEBUG("Key not supported by controller");
    return result_code;
  }
  GET_USER_DATA_FLAGS(temp_key, db_flag);
  result_code = GetRenamedControllerKey(l_key, req->datatype,
                                        dmi, &ctrlr_dom);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetRenamedControllerKey Faill");
    DELETE_IF_NOT_NULL(dup_key);
    DELETE_IF_NOT_NULL(temp_key);
    DELETE_IF_NOT_NULL(l_key);
    return result_code;
  }
  pfcdrv_val_vbrif_policingmap *pfc_val =
      reinterpret_cast<pfcdrv_val_vbrif_policingmap *>
      (ConfigKeyVal::Malloc(sizeof(pfcdrv_val_vbrif_policingmap_t)));
  pfcdrv_val_vbrif_vextif *pfc_val_ext =
      reinterpret_cast<pfcdrv_val_vbrif_vextif *>
      (ConfigKeyVal::Malloc(sizeof(pfcdrv_val_vbrif_vextif_t)));
  memset(pfc_val_ext, 0, sizeof(pfcdrv_val_vbrif_vextif_t));

  UPLL_LOG_DEBUG("GetVexternalInformation (%d)", req->datatype);
  result_code = GetVexternalInformation(temp_key, UPLL_DT_RUNNING, pfc_val,
                                        pfc_val_ext, db_flag,
                                        (const char*)ctrlr_dom.ctrlr, dmi);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetVexternalInformation fail");
    DELETE_IF_NOT_NULL(dup_key);
    DELETE_IF_NOT_NULL(l_key);
    DELETE_IF_NOT_NULL(temp_key);
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
  FREE_IF_NOT_NULL(pfc_val_ext);
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
    DELETE_IF_NOT_NULL(l_key);
    DELETE_IF_NOT_NULL(dup_key);
    DELETE_IF_NOT_NULL(ipc_response.ckv_data);
    DELETE_IF_NOT_NULL(temp_key);
    return ipc_response.header.result_code;
  }

  if (ipc_response.header.result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Driver response for Key %d controller %s result %d",
                   l_key->get_key_type(), ctrlr_dom.ctrlr,
                   ipc_response.header.result_code);
    DELETE_IF_NOT_NULL(l_key);
    DELETE_IF_NOT_NULL(dup_key);
    DELETE_IF_NOT_NULL(temp_key);
    DELETE_IF_NOT_NULL(ipc_response.ckv_data);
    return ipc_response.header.result_code;
  }

  DELETE_IF_NOT_NULL(l_key);
  ConfigKeyVal *okey = NULL;
  result_code = ConstructReadEntryDetailResponse(dup_key, ipc_response.ckv_data,
                                                 req->datatype, req->operation,
                                                 dbop, dmi, &okey, ctrlr_dom);
  DELETE_IF_NOT_NULL(temp_key);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("ReadSiblingDetail Error  (%d)", result_code);
    DELETE_IF_NOT_NULL(dup_key);
    DELETE_IF_NOT_NULL(l_key);
    DELETE_IF_NOT_NULL(ipc_response.ckv_data);
    return result_code;
  } else {
    if (okey != NULL) {
      ikey->ResetWith(okey);
      DELETE_IF_NOT_NULL(okey);
    }
  }
  DELETE_IF_NOT_NULL(dup_key);
  DELETE_IF_NOT_NULL(ipc_response.ckv_data);
  return result_code;
}

upll_rc_t VbrIfPolicingMapMoMgr::ReadSiblingCount(IpcReqRespHeader *req,
    ConfigKeyVal* ikey,
    DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
    controller_domain ctrlr_dom;
  result_code = ValidateMessage(req, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
     UPLL_LOG_DEBUG("ValidateMessage failed result_code %d",
                     result_code);
     return result_code;
  }

  if (UNC_KT_VBRIF_POLICINGMAP_ENTRY != ikey->get_key_type()) {
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
  if (UNC_KT_VBRIF_POLICINGMAP_ENTRY == ikey->get_key_type())
    if (req->datatype != UPLL_DT_STATE) {
    UPLL_LOG_DEBUG("ReadSiblingCount is not Allowed For Such datatype %d",
       req->datatype);
    return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT;
  }

  ConfigKeyVal *temp_vbr_if_key = NULL;
  key_vbrif_policingmap_entry_t *vbrif_entry_key = reinterpret_cast
    <key_vbrif_policingmap_entry_t *>(ikey->get_key());
  result_code = GetReadVbrIfKey(temp_vbr_if_key, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetReadVbrIfKey dup_key Err  (%d)", result_code);
    return result_code;
  }
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
  DELETE_IF_NOT_NULL(temp_vbr_if_key);
  PolicingProfileEntryMoMgr *mgr = reinterpret_cast
    <PolicingProfileEntryMoMgr*>
    (const_cast<MoManager *>(GetMoManager
                             (UNC_KT_POLICING_PROFILE_ENTRY)));
  if (!mgr) {
    DELETE_IF_NOT_NULL(ppe_ckv);
    return UPLL_RC_ERR_GENERIC;
  }
  IpcReqRespHeader *temp_req = reinterpret_cast<IpcReqRespHeader *>
    (ConfigKeyVal::Malloc(sizeof(IpcReqRespHeader)));
  memcpy(temp_req, req, sizeof(IpcReqRespHeader));
  temp_req->option1 = UNC_OPT1_NORMAL;
  temp_req->operation = UNC_OP_READ_SIBLING_COUNT;
  result_code = mgr->ReadSiblingMo(temp_req, ppe_ckv, dmi);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Read sibling of ppe failed (%d)", result_code);
    DELETE_IF_NOT_NULL(ppe_ckv);
    ConfigKeyVal::Free(temp_req);
    return result_code;
  }
  ConfigKeyVal::Free(temp_req);
  uint32_t *sibling_count = reinterpret_cast<uint32_t*>(GetVal(ppe_ckv));
  uint32_t *sib_count =
      reinterpret_cast<uint32_t*>(ConfigKeyVal::Malloc(sizeof(uint32_t)));
  *sib_count = *sibling_count;
  ikey->SetCfgVal(new ConfigVal(IpctSt::kIpcStUint32, sib_count));
  DELETE_IF_NOT_NULL(ppe_ckv);
  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrIfPolicingMapMoMgr::CreateAuditMoImpl(ConfigKeyVal *ikey,
                                   DalDmlIntf *dmi,
                                   const char *ctrlr_id) {
  UPLL_FUNC_TRACE;
  UPLL_LOG_DEBUG("Audit Create for VbrIfPolicingMapMoMgr called!!!");
  if (NULL == ikey || NULL == dmi) {
    UPLL_LOG_DEBUG("Insufficient input parameters");
    return UPLL_RC_ERR_GENERIC;
  }

  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *okey = NULL;
  string vtn_name = "";

  result_code = GetControllerId(ikey, okey, UPLL_DT_AUDIT, dmi);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetControllerId failed %d", result_code);
    return result_code;
  }
  delete okey;

  ConfigKeyVal *new_ikey = NULL;
  result_code = GetChildConfigKey(new_ikey, ikey);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetChildConfigKey failed");
    return result_code;
  }
  val_policingmap_t *val_pm = reinterpret_cast
    <val_policingmap_t *>(ConfigKeyVal::Malloc
    (sizeof(val_policingmap_t)));
  pfcdrv_val_vbrif_policingmap *pfc_val =
        reinterpret_cast<pfcdrv_val_vbrif_policingmap *>
        (GetVal(ikey));
  if (NULL == pfc_val) {
    return UPLL_RC_ERR_GENERIC;
  }
  if (pfc_val->valid[PFCDRV_IDX_VAL_POLICINGMAP_PM] == UNC_VF_VALID) {
    memcpy(val_pm, &(pfc_val->val_policing_map), sizeof(val_policingmap_t));
  } else {
    UPLL_LOG_DEBUG("No val_policingmap in driver structure");
    return UPLL_RC_ERR_GENERIC;
  }
  uint8_t flag_port_map = 0;
  GET_USER_DATA_FLAGS(ikey, flag_port_map);
  if (UNC_VF_VALID == pfc_val->valid[PFCDRV_IDX_VAL_VBRIF_VEXTIF_PM]) {
    flag_port_map |=  SET_FLAG_PORTMAP;
  } else {
    UPLL_LOG_DEBUG("Portmap not configured");
    return UPLL_RC_ERR_GENERIC;
  }
  SET_USER_DATA_FLAGS(new_ikey, flag_port_map);
  new_ikey->AppendCfgVal(IpctSt::kIpcStValPolicingmap, val_pm);

  if (UNC_VF_VALID == val_pm->valid[UPLL_IDX_POLICERNAME_PM]) {
    result_code = UpdateRefCountInPPCtrlr(new_ikey, UPLL_DT_AUDIT, dmi,
                                          UNC_OP_CREATE, TC_CONFIG_GLOBAL,
                                          vtn_name);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("UpdateRefCountInPPCtrlr Err in CANDIDATE DB(%d)",
                     result_code);
      return result_code;
    }
  }
  result_code = SetValidAudit(ikey);
  if (UPLL_RC_SUCCESS != result_code) {
    return result_code;
  }
  DbSubOp dbop = { kOpNotRead, kOpMatchNone,
    kOpInOutFlag|kOpInOutCtrlr|kOpInOutDomain|kOpInOutCs };
  result_code = UpdateConfigDB(new_ikey, UPLL_DT_AUDIT, UNC_OP_CREATE,
                               dmi, &dbop, TC_CONFIG_GLOBAL, vtn_name,
                               MAINTBL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("CreateCandidateMo failed. UpdateConfigDb failed."
                   "Record creation failed - %d",
                   result_code);
    return result_code;
  }
  UPLL_LOG_DEBUG("CreateCandidateMo Successful");
  delete new_ikey;
  return result_code;
}

upll_rc_t VbrIfPolicingMapMoMgr::AuditUpdateController(unc_key_type_t keytype,
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
  MoMgrTables tbl  = MAINTBL;
  controller_domain_t ctrlr_dom;
  ctrlr_dom.ctrlr = NULL;
  ctrlr_dom.domain = NULL;
  ConfigKeyVal  *ckv_running_db = NULL;
  ConfigKeyVal  *ckv_audit_db = NULL;
  ConfigKeyVal  *ckv_driver_req = NULL;
  ConfigKeyVal  *ckv_audit_dup_db = NULL;
  DalCursor *cursor = NULL;
  upll_keytype_datatype_t vext_datatype = UPLL_DT_RUNNING;
  string vtn_id = "";
  uint8_t *ctrlr = reinterpret_cast<uint8_t *>(const_cast<char *>(ctrlr_id));
  uint8_t auditdb_flag = 0;
  // Skipping the create phase if it comes as an input.
  // vbr if policingmap should get applied on controller(pfc) if portmap is
  // configured.
  // The portmap request should come in the update phase so
  // the vbrif policingmap creation should also be applied during update phase.
  if (phase1 == uuc::kUpllUcpCreate) {
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
    cursor = NULL;
    unc_keytype_operation_t op1 = op[i];
    uuc::UpdateCtrlrPhase phase = (op[i] == UNC_OP_UPDATE)?uuc::kUpllUcpUpdate:
      ((op[i] == UNC_OP_CREATE)?uuc::kUpllUcpCreate:
       ((op[i] == UNC_OP_DELETE)?uuc::kUpllUcpDelete:uuc::kUpllUcpInvalid));
    /* retreives the delta of running and audit configuration */
    UPLL_LOG_DEBUG("Operation is %d", op[i]);
    result_code = DiffConfigDB(UPLL_DT_RUNNING, UPLL_DT_AUDIT, op[i],
        ckv_running_db, ckv_audit_db,
        &cursor, dmi, ctrlr, TC_CONFIG_GLOBAL, vtn_id, tbl, true, true);
    if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      UPLL_LOG_DEBUG("No Diff found for operation %d", op[i]);
      DELETE_IF_NOT_NULL(ckv_running_db);
      DELETE_IF_NOT_NULL(ckv_audit_db);
      continue;
    }
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("DiffConfigDB failed - %d", result_code);
      DELETE_IF_NOT_NULL(ckv_running_db);
      DELETE_IF_NOT_NULL(ckv_audit_db);
      return result_code;
    }
    if (cursor == NULL) {
      UPLL_LOG_DEBUG("cursor is null");
      DELETE_IF_NOT_NULL(ckv_running_db);
      DELETE_IF_NOT_NULL(ckv_audit_db);
      return UPLL_RC_ERR_GENERIC;
    }
    while ((uud::kDalRcSuccess == (db_result = dmi->GetNextRecord(cursor))) &&
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
      /* ignore records of another controller for create and update operation */
      UPLL_LOG_DEBUG("Diff Record: Keytype: Operation:  is %d\n %d\n %s\n",
          keytype, op[i], ckv_running_db->ToStrAll().c_str());
      switch (phase) {
        case uuc::kUpllUcpDelete:
        case uuc::kUpllUcpCreate:
          UPLL_LOG_TRACE("Created  record is %s ",
              ckv_running_db->ToStrAll().c_str());
          result_code = DupConfigKeyVal(ckv_driver_req, ckv_running_db, tbl);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("DupConfigKeyVal failed. err_code & phase %d %d",
                result_code, phase);
            DELETE_IF_NOT_NULL(ckv_running_db);
            DELETE_IF_NOT_NULL(ckv_audit_db);
            dmi->CloseCursor(cursor, true);
            return result_code;
          }
          if (phase == uuc::kUpllUcpDelete)
            GET_USER_DATA_FLAGS(ckv_driver_req, auditdb_flag);
          break;
        case uuc::kUpllUcpUpdate:
          ckv_audit_dup_db = NULL;
          ckv_driver_req = NULL;
          UPLL_LOG_TRACE("UpdateRecord  record  is %s ",
              ckv_running_db->ToStrAll().c_str());
          UPLL_LOG_TRACE("UpdateRecord  record  is %s ",
              ckv_audit_db->ToStrAll().c_str());
          result_code = DupConfigKeyVal(ckv_driver_req, ckv_running_db, tbl);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("DupConfigKeyVal failed for running record."
                "err_code & phase %d %d", result_code, phase);
            DELETE_IF_NOT_NULL(ckv_running_db);
            DELETE_IF_NOT_NULL(ckv_audit_db);
            dmi->CloseCursor(cursor, true);
            return result_code;
          }
          result_code = DupConfigKeyVal(ckv_audit_dup_db, ckv_audit_db, tbl);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("DupConfigKeyVal failed for audit record."
                "err_code & phase %d %d", result_code, phase);
            DELETE_IF_NOT_NULL(ckv_driver_req);
            DELETE_IF_NOT_NULL(ckv_running_db);
            DELETE_IF_NOT_NULL(ckv_audit_db);
            dmi->CloseCursor(cursor, true);
            return result_code;
          }
          GET_USER_DATA_FLAGS(ckv_audit_dup_db, auditdb_flag);
          break;
        default:
          UPLL_LOG_DEBUG("Invalid operation %d", phase);
          DELETE_IF_NOT_NULL(ckv_running_db);
          DELETE_IF_NOT_NULL(ckv_audit_db);
          return UPLL_RC_ERR_NO_SUCH_OPERATION;
          break;
      }
      GET_USER_DATA_CTRLR_DOMAIN(ckv_driver_req, ctrlr_dom);
      if (NULL == ctrlr_dom.ctrlr || NULL == ctrlr_dom.domain) {
        UPLL_LOG_INFO("controller id or domain is NULL");
        DELETE_IF_NOT_NULL(ckv_driver_req);
        DELETE_IF_NOT_NULL(ckv_audit_dup_db);
        DELETE_IF_NOT_NULL(ckv_running_db);
        DELETE_IF_NOT_NULL(ckv_audit_db);
        dmi->CloseCursor(cursor, true);
        return UPLL_RC_ERR_GENERIC;
      }
      uint8_t db_flag = 0;
      GET_USER_DATA_FLAGS(ckv_driver_req, db_flag);
      uint8_t running_db_flag = 0;
      uint8_t audit_db_flag= 0;
      GET_USER_DATA_FLAGS(ckv_running_db, running_db_flag);

      if (op1 == UNC_OP_UPDATE) {
        GET_USER_DATA_FLAGS(ckv_audit_db, audit_db_flag);
      }
      // If port-map/vLink flag is not set at running and the operation is
      // UPDATE,check if port-map flag is set in audit DB, then port-map is
      // deleted in the UPDATE phase from UNC .So policingmap also should
      // get deleted from the controller, delete request is sent to the driver.
      // If port-map flag is set in running and the operation is UPDATE, check
      // if port-map flag is set in audit DB, if not set, then port-map is
      // created during UPDATE phase. So policingmap create request should be
      // sent to the driver.

      if (!(SET_FLAG_PORTMAP & running_db_flag)) {
        if (op1 != UNC_OP_UPDATE) {
          DELETE_IF_NOT_NULL(ckv_driver_req);
          DELETE_IF_NOT_NULL(ckv_audit_dup_db);
          continue;
        } else {
          if (!(SET_FLAG_PORTMAP & audit_db_flag)) {
            DELETE_IF_NOT_NULL(ckv_driver_req);
            DELETE_IF_NOT_NULL(ckv_audit_dup_db);
            continue;
          }
          op1 = UNC_OP_DELETE;
        }
      } else if (op1 == UNC_OP_UPDATE) {
        if (!(SET_FLAG_PORTMAP & audit_db_flag)) {
          op1 = UNC_OP_CREATE;
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
          // Setting the   value as OnlyCSDiff in the out parameter
          // ctrlr_affected.
          // The value Configdiff should be given more priority than the value
          // onlycs .
          // So  If the out parameter ctrlr_affected has already value as
          // configdiff then dont change the value
          if (*ctrlr_affected != uuc::kCtrlrAffectedConfigDiff) {
            UPLL_LOG_INFO("Setting the ctrlr_affected to OnlyCSDiff");
            *ctrlr_affected = uuc::kCtrlrAffectedOnlyCSDiff;
          }
          continue;
        }
      }

      DELETE_IF_NOT_NULL(ckv_audit_dup_db);
      pfcdrv_val_vbrif_policingmap *pfc_val =
        reinterpret_cast<pfcdrv_val_vbrif_policingmap *>
        (ConfigKeyVal::Malloc(sizeof(pfcdrv_val_vbrif_policingmap_t)));
      pfcdrv_val_vbrif_vextif *pfc_val_ext =
        reinterpret_cast<pfcdrv_val_vbrif_vextif *>\
        (ConfigKeyVal::Malloc(sizeof(pfcdrv_val_vbrif_vextif_t)));
      if (UNC_OP_DELETE == op1) {
        vext_datatype = UPLL_DT_AUDIT;
        db_flag = auditdb_flag;
      } else {
        vext_datatype = UPLL_DT_RUNNING;
      }
      UPLL_LOG_DEBUG("GetVexternalInformation (%d)", vext_datatype);
      result_code = GetVexternalInformation(ckv_driver_req,
                                            vext_datatype, pfc_val,
                                            pfc_val_ext, db_flag,
                                            (const char*)ctrlr_dom.ctrlr, dmi);
      if (UPLL_RC_SUCCESS != result_code) {
        UPLL_LOG_DEBUG("GetVexternalInformation fail");
        DELETE_IF_NOT_NULL(ckv_driver_req);
        DELETE_IF_NOT_NULL(ckv_running_db);
        DELETE_IF_NOT_NULL(ckv_audit_db);
        FREE_IF_NOT_NULL(pfc_val);
        FREE_IF_NOT_NULL(pfc_val_ext);
        dmi->CloseCursor(cursor, true);
        return result_code;
      }
      UPLL_LOG_DEBUG("Controller : %s; Domain : %s", ctrlr_dom.ctrlr,
          ctrlr_dom.domain);

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
        dmi->CloseCursor(cursor, true);
        FREE_IF_NOT_NULL(pfc_val);
        FREE_IF_NOT_NULL(pfc_val_ext);
        return result_code;
      }
      val_policingmap_t* val =
          reinterpret_cast<val_policingmap_t *>(GetVal(ckv_driver_req));
      UPLL_LOG_TRACE("val_policingmap_t (%s)", val->policer_name);

      memcpy(&pfc_val->val_policing_map, val, sizeof(val_policingmap_t));
      pfc_val->valid[PFCDRV_IDX_VAL_POLICINGMAP_PM] = UNC_VF_VALID;

      memcpy(&pfc_val->val_vbrif_vextif, pfc_val_ext,
          sizeof(pfcdrv_val_vbrif_vextif_t));
      pfc_val->valid[PFCDRV_IDX_VAL_VBRIF_VEXTIF_PM] = UNC_VF_VALID;

      FREE_IF_NOT_NULL(pfc_val_ext);

      ckv_driver_req->SetCfgVal(
          new ConfigVal(IpctSt::kIpcStPfcdrvValVbrifPolicingmap, pfc_val));

      UPLL_LOG_DEBUG("Controller : %s; Domain : %s", ctrlr_dom.ctrlr,
          ctrlr_dom.domain);

      if (op1 == UNC_OP_UPDATE) {
        pfcdrv_val_vbrif_policingmap *pfc_val_old =
            reinterpret_cast<pfcdrv_val_vbrif_policingmap *>
            (ConfigKeyVal::Malloc(sizeof(pfcdrv_val_vbrif_policingmap_t)));
        val_policingmap_t* val =
                    reinterpret_cast<val_policingmap_t *>(GetVal(ckv_audit_db));
        memcpy(&pfc_val_old->val_policing_map, val, sizeof(val_policingmap_t));
        pfc_val_old->valid[PFCDRV_IDX_VAL_POLICINGMAP_PM] = UNC_VF_VALID;
        ckv_driver_req->AppendCfgVal(
                        new ConfigVal(IpctSt::kIpcStPfcdrvValVbrifPolicingmap,
                                      pfc_val_old));
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
      if (!uui::IpcUtil::SendReqToDriver(
              (const char *)ctrlr_dom.ctrlr,
              reinterpret_cast<char *>(ctrlr_dom.domain),
              PFCDRIVER_SERVICE_NAME, PFCDRIVER_SVID_LOGICAL,
              &ipc_req, true, &ipc_response)) {
        UPLL_LOG_INFO("Request to driver for Key %d for controller %s failed ",
            ckv_driver_req->get_key_type(),
            reinterpret_cast<char *>(ctrlr_dom.ctrlr));
        DELETE_IF_NOT_NULL(ckv_driver_req);
        DELETE_IF_NOT_NULL(ipc_response.ckv_data);
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
          result_code = GetChildConfigKey(resp, ipc_response.ckv_data);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG(
                "GetChildConfigKey failed for ipc response ckv err_code %d",
                result_code);
            DELETE_IF_NOT_NULL(ipc_response.ckv_data);
            DELETE_IF_NOT_NULL(ckv_driver_req);
            DELETE_IF_NOT_NULL(ckv_running_db);
            DELETE_IF_NOT_NULL(ckv_audit_db);
            *err_ckv = NULL;
            dmi->CloseCursor(cursor, true);
            return result_code;
          }
          pfcdrv_val_vbrif_policingmap *pfc_val_out =
            reinterpret_cast<pfcdrv_val_vbrif_policingmap *>
            (GetVal(ipc_response.ckv_data));
          if (NULL == pfc_val_out) {
            DELETE_IF_NOT_NULL(resp);
            DELETE_IF_NOT_NULL(ckv_driver_req);
            DELETE_IF_NOT_NULL(ipc_response.ckv_data);
            DELETE_IF_NOT_NULL(ckv_running_db);
            DELETE_IF_NOT_NULL(ckv_audit_db);
            dmi->CloseCursor(cursor, true);
            *err_ckv = NULL;
            return UPLL_RC_ERR_GENERIC;
          }
          val_policingmap_t *out_val_pm = reinterpret_cast
            <val_policingmap_t *>(ConfigKeyVal::Malloc(sizeof
                  (val_policingmap_t)));
          memcpy(out_val_pm, &pfc_val_out->val_policing_map,
              sizeof(val_policingmap_t));
          resp->AppendCfgVal(IpctSt::kIpcStValPolicingmap, out_val_pm);
          result_code = UpdateAuditConfigStatus(UNC_CS_INVALID, phase,
                                                resp, dmi);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_TRACE("Update Audit config status failed %d",
                result_code);
            DELETE_IF_NOT_NULL(resp);
            DELETE_IF_NOT_NULL(ckv_driver_req);
            DELETE_IF_NOT_NULL(ipc_response.ckv_data);
            DELETE_IF_NOT_NULL(ckv_running_db);
            DELETE_IF_NOT_NULL(ckv_audit_db);
            dmi->CloseCursor(cursor, true);
            *err_ckv = NULL;
            return result_code;
          }
          result_code = UpdateConfigDB(resp, dt_type, UNC_OP_UPDATE,
              dmi, TC_CONFIG_GLOBAL, vtn_id, tbl);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG(
                "UpdateConfigDB failed for ipc response ckv err_code %d",
                result_code);
            DELETE_IF_NOT_NULL(resp);
            DELETE_IF_NOT_NULL(ckv_driver_req);
            DELETE_IF_NOT_NULL(ipc_response.ckv_data);
            DELETE_IF_NOT_NULL(ckv_running_db);
            DELETE_IF_NOT_NULL(ckv_audit_db);
            dmi->CloseCursor(cursor, true);
            *err_ckv = NULL;
            return result_code;
          }
          DELETE_IF_NOT_NULL(resp);
        }
        DELETE_IF_NOT_NULL(ckv_driver_req);
        DELETE_IF_NOT_NULL(ipc_response.ckv_data);
        DELETE_IF_NOT_NULL(ckv_audit_db);
        dmi->CloseCursor(cursor, true);
        return ipc_response.header.result_code;
      }
      DELETE_IF_NOT_NULL(ckv_driver_req);
      DELETE_IF_NOT_NULL(ipc_response.ckv_data);
      if (*ctrlr_affected == uuc::kCtrlrAffectedOnlyCSDiff) {
        UPLL_LOG_INFO("Reset ctrlr state from OnlyCSDiff to ConfigDiff");
      }
      // *ctrlr_affected = true;
      *ctrlr_affected = uuc::kCtrlrAffectedConfigDiff;
    }
    dmi->CloseCursor(cursor, true);
    DELETE_IF_NOT_NULL(ckv_running_db);
    DELETE_IF_NOT_NULL(ckv_audit_db);
  }
  UPLL_LOG_DEBUG("No more record");
  if (uud::kDalRcSuccess != db_result) {
    UPLL_LOG_DEBUG("GetNextRecord from database failed  - %d", db_result);
    result_code =  DalToUpllResCode(db_result);
  }
  result_code = (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE)
    ? UPLL_RC_SUCCESS : result_code;
  return result_code;
}

upll_rc_t VbrIfPolicingMapMoMgr::DeleteChildrenPOM(ConfigKeyVal *ikey,
        upll_keytype_datatype_t dt_type, DalDmlIntf *dmi,
        TcConfigMode config_mode, string vtn_name) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_ERR_GENERIC;
  int nattr = 0;
  BindInfo *binfo = NULL;
  DalCursor *dal_cursor_handle = NULL;
  string query_string;
  unc_key_type_t deletedkt;

  if (NULL == ikey || NULL == dmi) {
    UPLL_LOG_DEBUG("DeleteMo Failed. Insufficient input parameters");
    return result_code;
  }
  key_vbr_if_t *pkey =
      reinterpret_cast<key_vbr_if_t*>(ikey->get_key());
  if (!pkey) {
    UPLL_LOG_DEBUG(" Input vbr_if policingmap  key is NULL ");
    return UPLL_RC_ERR_GENERIC;
  }

  if (((pkey->vbr_key.vbridge_name)[0]) == '\0') {
    deletedkt = UNC_KT_VTN;
  } else if (((pkey->if_name)[0]) == '\0') {
    deletedkt = UNC_KT_VBRIDGE;
  } else {
    deletedkt = UNC_KT_VBR_IF;
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

  ConfigKeyVal *vbrif_pm_ckv  = NULL;
  result_code = GetChildConfigKey(vbrif_pm_ckv, NULL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetChildConfigKey failed");
    return result_code;
  }
  ConfigVal* vbrif_pm_cv = NULL;
  result_code = AllocVal(vbrif_pm_cv, UPLL_DT_CANDIDATE, MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Error in AllocVal");
    DELETE_IF_NOT_NULL(vbrif_pm_ckv);
    return result_code;
  }
  vbrif_pm_ckv->SetCfgVal(vbrif_pm_cv);
  GET_USER_DATA(vbrif_pm_ckv);
  uint32_t count = 0;
  void *tkey = vbrif_pm_ckv->get_key();
  void *p = reinterpret_cast<void *>(reinterpret_cast<char *>(tkey) +
                                     binfo[0].offset);
  dal_bind_info.BindOutput(binfo[0].index, binfo[0].app_data_type,
                           binfo[0].array_size, p);
  tkey = vbrif_pm_ckv->get_user_data();
  p = reinterpret_cast<void *>(reinterpret_cast<char *>(tkey) +
                               binfo[3].offset);
  dal_bind_info.BindOutput(binfo[3].index, binfo[3].app_data_type,
                           binfo[3].array_size, p);
  tkey = vbrif_pm_ckv->get_cfg_val()->get_val();
  p = reinterpret_cast<void *>(reinterpret_cast<char *>(tkey) +
                               binfo[5].offset);
  dal_bind_info.BindOutput(binfo[5].index, binfo[5].app_data_type,
                           binfo[5].array_size, p);

  dal_bind_info.BindOutput(uud::schema::DAL_COL_STD_INTEGER,
                           uud::kDalUint32, 1, &count);

  result_code = DalToUpllResCode(dmi->ExecuteAppQueryMultipleRecords(
                 query_string, 0, &dal_bind_info,
                 &dal_cursor_handle));
  while (result_code == UPLL_RC_SUCCESS) {
    result_code = DalToUpllResCode(dmi->GetNextRecord(dal_cursor_handle));
    if (UPLL_RC_SUCCESS == result_code) {
      // Call function to update refcount in scratch table
      key_vbr_if_t *vbrif_pm_key =
          reinterpret_cast<key_vbr_if_t *>(vbrif_pm_ckv->get_key());
      vtn_name = reinterpret_cast<const char *>
          (vbrif_pm_key->vbr_key.vtn_key.vtn_name);
      uint8_t *ctrlr_id = NULL;
      GET_USER_DATA_CTRLR(vbrif_pm_ckv, ctrlr_id);
      val_policingmap_t *vbrif_pm_val =
          reinterpret_cast<val_policingmap_t *>(GetVal(vbrif_pm_ckv));

      PolicingProfileMoMgr *pp_mgr =
          reinterpret_cast<PolicingProfileMoMgr *>
          (const_cast<MoManager *>(GetMoManager(UNC_KT_POLICING_PROFILE)));
      if (NULL == pp_mgr) {
        UPLL_LOG_DEBUG("pp_mgr is NULL");
        DELETE_IF_NOT_NULL(vbrif_pm_ckv);
        return UPLL_RC_ERR_GENERIC;
      }
      ConfigKeyVal *pp_ckv  = NULL;
      result_code = pp_mgr->GetChildConfigKey(pp_ckv, NULL);
      if (UPLL_RC_SUCCESS != result_code) {
        UPLL_LOG_DEBUG("GetChildConfigKey failed");
        DELETE_IF_NOT_NULL(vbrif_pm_ckv);
        return result_code;
      }
      key_policingprofile_t *pp_key =
          reinterpret_cast<key_policingprofile_t *>(pp_ckv->get_key());
      uuu::upll_strncpy(pp_key->policingprofile_name,
                        vbrif_pm_val->policer_name,
                        (kMaxLenPolicingProfileName+1));
      SET_USER_DATA_CTRLR(pp_ckv, ctrlr_id);

      result_code = pp_mgr->UpdateRefCountInScratchTbl(pp_ckv, dmi,
                                                       dt_type, UNC_OP_DELETE,
                                                       config_mode, vtn_name,
                                                       count);
      if (result_code != UPLL_RC_SUCCESS &&
          result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
        UPLL_LOG_DEBUG("UpdateRefCountInScratchTbl returned %d", result_code);
        DELETE_IF_NOT_NULL(vbrif_pm_ckv);
        DELETE_IF_NOT_NULL(pp_ckv);
        return result_code;
      } else if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
        result_code = pp_mgr->InsertRecInScratchTbl(pp_ckv, dmi, dt_type,
                                                    UNC_OP_DELETE,
                                                    config_mode, vtn_name,
                                                    count);
        if (UPLL_RC_SUCCESS != result_code) {
          UPLL_LOG_DEBUG("InsertRecInScratchTbl failed %d", result_code);
          DELETE_IF_NOT_NULL(vbrif_pm_ckv);
          DELETE_IF_NOT_NULL(pp_ckv);
          return result_code;
        }
      }
      DELETE_IF_NOT_NULL(pp_ckv);
    } else if (UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
      UPLL_LOG_DEBUG("GetNextRecord failed");
      DELETE_IF_NOT_NULL(vbrif_pm_ckv);
      dmi->CloseCursor(dal_cursor_handle, false);
      return result_code;
    }
  }
  if (result_code != UPLL_RC_SUCCESS &&
      result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    UPLL_LOG_DEBUG("ExecuteAppQueryMultipleRecords failed");
    DELETE_IF_NOT_NULL(vbrif_pm_ckv);
    dmi->CloseCursor(dal_cursor_handle, false);
    return result_code;
  }
  DELETE_IF_NOT_NULL(vbrif_pm_ckv);

  // 1)Get vbrif associated ctrlr name and invoke the PP and PPE functions to
  // decrement the refcount capability. If refcount is zero, remove the record
  // in policingprofilectrltbl and if refcount not zero update the refcount in
  // policingprofilectrltbl
  // 2)Delete the record in policingprofileentryctrltbl
  /*ConfigKeyVal *okey = NULL;
  result_code = GetChildConfigKey(okey, ikey);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetChildConfigKey failed %d", result_code);
    return result_code;
  }
  DbSubOp dbop = { kOpReadMultiple, kOpMatchNone,
    kOpInOutCtrlr | kOpInOutDomain | kOpInOutFlag};
  result_code = ReadConfigDB(okey, dt_type, UNC_OP_READ, dbop, dmi,
                             MAINTBL);
  if (UPLL_RC_SUCCESS != result_code) {
    if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
      UPLL_LOG_DEBUG("UPLL_RC_ERR_NO_SUCH_INSTANCE");
      DELETE_IF_NOT_NULL(okey);
      return UPLL_RC_SUCCESS;
    }
    UPLL_LOG_DEBUG("ReadConfigDB failed %d", result_code);
    DELETE_IF_NOT_NULL(okey);
    return result_code;
  }

  ConfigKeyVal *temp_okey = okey;
  while (temp_okey != NULL) {
    val_policingmap_t *val_pm = reinterpret_cast<val_policingmap_t *>
      (GetVal(temp_okey));
    if (UNC_VF_VALID == val_pm->valid[UPLL_IDX_POLICERNAME_PM]) {
      uint8_t flag_port_map = 0;
      GET_USER_DATA_FLAGS(okey, flag_port_map);
      if (flag_port_map & SET_FLAG_PORTMAP) {
        result_code = UpdateRefCountInPPCtrlr(temp_okey, dt_type, dmi,
            UNC_OP_DELETE, config_mode, vtn_name);
        if (UPLL_RC_SUCCESS != result_code) {
          UPLL_LOG_DEBUG("UpdateRefCountInPPCtrlr Error DB (%d)", result_code);
          DELETE_IF_NOT_NULL(okey);
          return result_code;
        }
      }
    }
    temp_okey = temp_okey->get_next_cfg_key_val();
  }
  DELETE_IF_NOT_NULL(okey);
  */
  // Delete the record in vbrifpolicingmap table
  ConfigKeyVal *temp_ikey = NULL;
  result_code = GetChildConfigKey(temp_ikey, ikey);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetChildConfigKey failed");
    return result_code;
  }
  result_code = UpdateConfigDB(temp_ikey, dt_type, UNC_OP_DELETE, dmi,
                               config_mode, vtn_name, MAINTBL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("DeleteMo Failed. UpdateConfigdb failed to delete - %d",
                  result_code);
    dmi->CloseCursor(dal_cursor_handle, false);
    DELETE_IF_NOT_NULL(temp_ikey);
    return result_code;
  }
  dmi->CloseCursor(dal_cursor_handle, false);
  DELETE_IF_NOT_NULL(temp_ikey);
  UPLL_LOG_DEBUG("DeleteMo Success");
  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrIfPolicingMapMoMgr::IsPolicingProfileConfigured(
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

upll_rc_t VbrIfPolicingMapMoMgr::SetValidAudit(ConfigKeyVal *&ikey) {
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

bool VbrIfPolicingMapMoMgr::FilterAttributes(void *&val1,
                                          void *val2,
                                          bool copy_to_running,
                                          unc_keytype_operation_t op) {
  UPLL_FUNC_TRACE;
  if (op != UNC_OP_CREATE)
    return CompareValidValue(val1, val2, copy_to_running);
  return false;
}

upll_rc_t VbrIfPolicingMapMoMgr::UpdateConfigStatus(ConfigKeyVal *vbrif_key,
                                       unc_keytype_operation_t op,
                                       uint32_t driver_result,
                                       ConfigKeyVal *upd_key,
                                       DalDmlIntf *dmi,
                                       ConfigKeyVal *ctrlr_key) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  val_policingmap_t *vbr_pm;

  unc_keytype_configstatus_t cs_status =
      (driver_result == UPLL_RC_SUCCESS) ? UNC_CS_APPLIED : UNC_CS_NOT_APPLIED;
  vbr_pm = reinterpret_cast<val_policingmap_t *>(GetVal(vbrif_key));
  if (vbr_pm == NULL) return UPLL_RC_ERR_GENERIC;
  if (op == UNC_OP_CREATE) {
    vbr_pm->cs_row_status = cs_status;
  } else if (op == UNC_OP_UPDATE) {
    void *vbrpm = reinterpret_cast<void *>(vbr_pm);
    CompareValidValue(vbrpm, GetVal(upd_key), true);
  } else {
    return UPLL_RC_ERR_GENERIC;
  }
  UPLL_LOG_TRACE("%s", (vbrif_key->ToStrAll()).c_str());
  val_policingmap_t *vbr_val2 =
      reinterpret_cast<val_policingmap_t *>(GetVal(upd_key));
  if (UNC_OP_UPDATE == op) {
    UPLL_LOG_TRACE("%s", (upd_key->ToStrAll()).c_str());
    vbr_pm->cs_row_status = vbr_val2->cs_row_status;
  }
  if (UNC_VF_NOT_SUPPORTED == vbr_pm->valid[0]) {
      vbr_pm->cs_attr[0] = UNC_CS_NOT_SUPPORTED;
  } else if ((UNC_VF_VALID == vbr_pm->valid[0])
        || (UNC_VF_VALID_NO_VALUE == vbr_pm->valid[0])) {
      vbr_pm->cs_attr[0] = cs_status;
  } else if ((UNC_VF_INVALID == vbr_pm->valid[0]) &&
             (UNC_OP_CREATE == op)) {
      vbr_pm->cs_attr[0] = UNC_CS_NOT_APPLIED;
  } else if ((UNC_VF_INVALID == vbr_pm->valid[0]) &&
             (UNC_OP_UPDATE == op)) {
      vbr_pm->cs_attr[0] = vbr_val2->cs_attr[0];
  }
  return result_code;
}

upll_rc_t VbrIfPolicingMapMoMgr::IsRenamed(ConfigKeyVal *ikey,
                               upll_keytype_datatype_t dt_type,
                               DalDmlIntf *dmi,
                               uint8_t &rename) {
  UPLL_FUNC_TRACE;
  UPLL_LOG_DEBUG("VbrIfPolicingMapMoMgr IsRenamed");
  if (NULL == ikey) {
    UPLL_LOG_DEBUG("ikey NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutFlag | kOpInOutCtrlr
                       | kOpInOutDomain };
  ConfigKeyVal *okey = NULL;
  upll_rc_t result_code;
  /* rename is set implies user wants the ikey
   * populated with val from db */
  if (!rename) {
    if (UNC_KT_VBRIF_POLICINGMAP == ikey->get_key_type()) {
      UPLL_LOG_DEBUG("UNC_KT_VBRIF_POLICINGMAP");
      result_code = GetChildConfigKey(okey, ikey);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_TRACE("Returning error %d", result_code);
        return result_code;
      }
    } else if (UNC_KT_VBRIF_POLICINGMAP_ENTRY == ikey->get_key_type()) {
      UPLL_LOG_DEBUG("UNC_KT_VBRIF_POLICINGMAP_CONTROLLER");

      key_vbr_if_t *out_key = reinterpret_cast<key_vbr_if_t *>
              (ConfigKeyVal::Malloc(sizeof(key_vbr_if_t)));

      key_vbrif_policingmap_entry_t *in_key = reinterpret_cast
        <key_vbrif_policingmap_entry_t *>(ikey->get_key());

      uuu::upll_strncpy(out_key->vbr_key.vtn_key.vtn_name,
          in_key->vbrif_key.vbr_key.vtn_key.vtn_name,
          (kMaxLenVtnName + 1));
      uuu::upll_strncpy(out_key->vbr_key.vbridge_name,
          in_key->vbrif_key.vbr_key.vbridge_name,
          (kMaxLenVnodeName + 1));
      uuu::upll_strncpy(out_key->if_name,
          in_key->vbrif_key.if_name,
          (kMaxLenVnodeName + 1));

      okey = new ConfigKeyVal(UNC_KT_VBRIF_POLICINGMAP,
                 IpctSt::kIpcStKeyVbrIf,
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
    if (okey != ikey)
      DELETE_IF_NOT_NULL(okey);
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
  if (okey != ikey)
    DELETE_IF_NOT_NULL(okey);
  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrIfPolicingMapMoMgr::SetRenameFlag(ConfigKeyVal *ikey,
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
              UNC_KT_VBR_IF)));
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
    if ((UNC_VF_VALID == val_pm->valid[UPLL_IDX_POLICERNAME_PM]) &&
        ((UNC_OP_CREATE == req->operation))) {
      ConfigKeyVal *pp_ckv = NULL;
      result_code = GetPolicingProfileConfigKey(reinterpret_cast<const char *>
          (val_pm->policer_name), pp_ckv, dmi);
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
    ConfigKeyVal *dup_ckv = NULL;
    result_code = GetChildConfigKey(dup_ckv, ikey);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG(" GetChildConfigKey failed");
      return result_code;
    }
    DbSubOp dbop1 = {kOpReadSingle, kOpMatchNone, kOpInOutFlag};
    result_code = ReadConfigDB(dup_ckv, req->datatype, UNC_OP_READ,
                                     dbop1, dmi, MAINTBL);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("ReadConfigDB failed %d", result_code);
      DELETE_IF_NOT_NULL(dup_ckv);
      return result_code;
    }
    GET_USER_DATA_FLAGS(dup_ckv, rename);
    DELETE_IF_NOT_NULL(dup_ckv);
    if (UNC_VF_VALID == val_pm->valid[UPLL_IDX_POLICERNAME_PM]) {
      ConfigKeyVal *pp_ckv = NULL;
      result_code = GetPolicingProfileConfigKey(reinterpret_cast<const char *>
          (val_pm->policer_name), pp_ckv, dmi);
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
        rename &= NO_POLICINGPROFILE_RENAME;
      }
      DELETE_IF_NOT_NULL(pp_ckv);
    } else if (UNC_VF_VALID_NO_VALUE == val_pm->valid
               [UPLL_IDX_POLICERNAME_PM]) {
       // No rename flowlist value should be set
       rename &= NO_POLICINGPROFILE_RENAME;
    }
    SET_USER_DATA_FLAGS(ikey, rename);
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrIfPolicingMapMoMgr::GetPolicingProfileConfigKey(
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
upll_rc_t VbrIfPolicingMapMoMgr::TxUpdateErrorHandler(ConfigKeyVal *ckv_unc,
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
