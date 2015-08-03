/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include "vtep_grp_momgr.hh"
#include "ctrlr_mgr.hh"
#include "vtn_momgr.hh"
#include "vtepgrp_mem_momgr.hh"
#define NUM_KEY_MAIN_TBL_ 5

namespace unc {
namespace upll {
namespace kt_momgr {

  BindInfo VtepGrpMoMgr::vtep_grp_bind_info[] = {
    { uudst::vtep_group::kDbiVtnName, CFG_KEY,
      offsetof(key_vtep_grp, vtn_key.vtn_name), uud::kDalChar,
      kMaxLenVtnName+1 },
    { uudst::vtep_group::kDbiVtepgrpName, CFG_KEY,
      offsetof(key_vtep_grp, vtepgrp_name), uud::kDalChar,
      kMaxLenVnodeName+1 },
    { uudst::vtep_group::kDbiCtrlrName, CFG_VAL,
      offsetof(val_vtep_grp, controller_id), uud::kDalChar, kMaxLenCtrlrId+1},
    { uudst::vtep_group::kDbiCtrlrName, CK_VAL,
      offsetof(key_user_data_t, ctrlr_id),
      uud::kDalChar, kMaxLenCtrlrId+1 },
    { uudst::vtep_group::kDbiDesc, CFG_VAL, offsetof(val_vtep_grp, description),
      uud::kDalChar, kMaxLenDescription+1},
    { uudst::vtep_group::kDbiFlags, CK_VAL, offsetof(key_user_data_t, flags),
      uud::kDalUint8, 1},
    { uudst::vtep_group::kDbiValidCtrlrName, CFG_META_VAL,
      offsetof(val_vtep_grp, valid[0]), uud::kDalUint8, 1},
    { uudst::vtep_group::kDbiValidDesc, CFG_META_VAL,
      offsetof(val_vtep_grp, valid[1]), uud::kDalUint8, 1},
    { uudst::vtep_group::kDbiCsRowstatus, CS_VAL,
      offsetof(val_vtep_grp, cs_row_status), uud::kDalUint8, 1},
    { uudst::vtep_group::kDbiCsCtrlrName, CS_VAL, offsetof(val_vtep_grp,
      cs_attr[0]),
      uud::kDalUint8, 1},
    { uudst::vtep_group::kDbiCsDesc, CS_VAL, offsetof(val_vtep_grp, cs_attr[1]),
      uud::kDalUint8, 1}
  };

unc_key_type_t VtepGrpMoMgr::vtep_grp_child[] = {
  UNC_KT_VTEP_GRP_MEMBER
};

VtepGrpMoMgr::VtepGrpMoMgr() {
  UPLL_FUNC_TRACE;
  Table *tbl = new Table(uudst::kDbiVtepGrpTbl, UNC_KT_VTEP_GRP,
      vtep_grp_bind_info,
      IpctSt::kIpcStKeyVtepGrp, IpctSt::kIpcStValVtepGrp,
      uudst::vtep_group::kDbiVtepGrpNumCols+1);
  ntable = MAX_MOMGR_TBLS;
  table = new Table *[ntable]();
  table[MAINTBL] = tbl;
  table[RENAMETBL] = NULL;
  table[CTRLRTBL] = NULL;
  table[CONVERTTBL] = NULL;

  nchild = sizeof(vtep_grp_child) / sizeof(*vtep_grp_child);
  child = vtep_grp_child;
#ifdef _STANDALONE_
  SetMoManager(UNC_KT_VTEP_GRP, reinterpret_cast<MoMgr *>(this));
#endif
}

/*
 *  * Based on the key type the bind info will pass
 *   *
 bool VtepGrpMoMgr::GetRenameKeyBindInfo(unc_key_type_t key_type,
 BindInfo *&binfo, int &nattr,
 MoMgrTables tbl ) {
 if (MAINTBL == tbl) {
 nattr = NUM_KEY_MAIN_TBL_;
 binfo = key_vtep_grp_maintbl_update_bind_info;
 }
 return PFC_TRUE;
 }
 */



bool VtepGrpMoMgr::IsValidKey(void *key, uint64_t index, MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  key_vtep_grp *vtep_grp_key = reinterpret_cast<key_vtep_grp *>(key);
  upll_rc_t ret_val = UPLL_RC_SUCCESS;
  switch (index) {
    case uudst::vtep_group::kDbiVtnName:
      ret_val = ValidateKey(reinterpret_cast<char *>
                            (vtep_grp_key->vtn_key.vtn_name),
                            kMinLenVtnName, kMaxLenVtnName);
      if (ret_val != UPLL_RC_SUCCESS) {
        UPLL_LOG_TRACE("VTN Name is not valid(%d)", ret_val);
        return false;
      }
      break;
    case uudst::vtep_group::kDbiVtepgrpName:
      ret_val = ValidateKey(reinterpret_cast<char *>
                            (vtep_grp_key->vtepgrp_name),
                            kMinLenVnodeName,
                            kMaxLenVnodeName);
      if (ret_val != UPLL_RC_SUCCESS) {
        UPLL_LOG_TRACE("VtepGroup Name is not valid(%d)", ret_val);
        return false;
      }
      break;
    default:
      UPLL_LOG_TRACE("Invalid Key Index");
      return false;
  }
  return true;
}

upll_rc_t VtepGrpMoMgr::GetChildConfigKey(ConfigKeyVal *&okey,
    ConfigKeyVal *parent_key) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  key_vtep_grp *vtep_grp_key = static_cast<key_vtep_grp *>
    (ConfigKeyVal::Malloc(sizeof(key_vtep_grp)));
  if (vtep_grp_key == NULL) return UPLL_RC_ERR_GENERIC;
  void *pkey;
  if (parent_key == NULL) {
    okey = new ConfigKeyVal(UNC_KT_VTEP_GRP, IpctSt::kIpcStKeyVtepGrp,
                            vtep_grp_key, NULL);
    return UPLL_RC_SUCCESS;
  } else {
    pkey = parent_key->get_key();
  }
  if (!pkey) {
    FREE_IF_NOT_NULL(vtep_grp_key);
    return UPLL_RC_ERR_GENERIC;
  }
  if (okey && (okey->get_key())) {
    FREE_IF_NOT_NULL(vtep_grp_key);
    if (okey->get_key_type() != UNC_KT_VTEP_GRP)
      return UPLL_RC_ERR_GENERIC;
    vtep_grp_key = reinterpret_cast<key_vtep_grp *>(okey->get_key());
  } else {
    okey = new ConfigKeyVal(UNC_KT_VTEP_GRP, IpctSt::kIpcStKeyVtepGrp,
                            vtep_grp_key, NULL);
    if (okey == NULL) {
      FREE_IF_NOT_NULL(vtep_grp_key);
      return UPLL_RC_ERR_GENERIC;
    }
  }
  /* presumes MoMgrs receive only supported keytypes */
  switch (parent_key->get_key_type()) {
    case UNC_KT_VTEP_GRP:
      uuu::upll_strncpy(vtep_grp_key->vtepgrp_name,
               reinterpret_cast<key_vtep_grp *>(pkey)->vtepgrp_name,
              (kMaxLenVnodeName+1));
      uuu::upll_strncpy(vtep_grp_key->vtn_key.vtn_name,
             reinterpret_cast<key_vtep *>(pkey)->vtn_key.vtn_name,
             (kMaxLenVtnName+1));
      break;
    case UNC_KT_VTN:
    default:
      uuu::upll_strncpy(vtep_grp_key->vtn_key.vtn_name,
             reinterpret_cast<key_vtn *>(pkey)->vtn_name,
             (kMaxLenVtnName+1));
  }
  SET_USER_DATA(okey, parent_key);
  return result_code;
}

upll_rc_t VtepGrpMoMgr::GetParentConfigKey(ConfigKeyVal *&okey,
    ConfigKeyVal *ikey ) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  key_vtep_grp *pkey = (ikey)?
    static_cast<key_vtep_grp *>(ikey->get_key()):NULL;
  if (!pkey) return UPLL_RC_ERR_GENERIC;

  if (ikey->get_key_type() != UNC_KT_VTEP_GRP)
    return UPLL_RC_ERR_GENERIC;
  key_vtn *vtn_key = reinterpret_cast<key_vtn *>
                     (ConfigKeyVal::Malloc(sizeof(key_vtn)));
  if (!vtn_key) return UPLL_RC_ERR_GENERIC;
  uuu::upll_strncpy(vtn_key->vtn_name,
         pkey->vtn_key.vtn_name,
         (kMaxLenVtnName+1));
  DELETE_IF_NOT_NULL(okey);
  okey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, vtn_key, NULL);
  if (okey == NULL) {
    FREE_IF_NOT_NULL(vtn_key);
    result_code = UPLL_RC_ERR_GENERIC;
  } else {
    SET_USER_DATA(okey, ikey);
  }
  return result_code;
}

upll_rc_t VtepGrpMoMgr::AllocVal(ConfigVal *&ck_val,
    upll_keytype_datatype_t dt_type, MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  void *val;
  if (ck_val != NULL) return UPLL_RC_ERR_GENERIC;
  switch (tbl) {
    case MAINTBL:
      val = reinterpret_cast<void *>
            (ConfigKeyVal::Malloc(sizeof(val_vtep_grp)));
      if (!val) return UPLL_RC_ERR_GENERIC;
      ck_val = new ConfigVal(IpctSt::kIpcStValVtepGrp, val);
      if (!ck_val) {
        FREE_IF_NOT_NULL(reinterpret_cast<val_vtep_grp *>(val));
        return UPLL_RC_ERR_GENERIC;
      }
      break;
    default:
      val = NULL;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtepGrpMoMgr::ValidateAttribute(ConfigKeyVal *ikey,
                                          DalDmlIntf *dmi,
                                          IpcReqRespHeader *req) {
  UPLL_FUNC_TRACE;
  controller_domain ctrlr_dom;
  ctrlr_dom.ctrlr = NULL;
  ctrlr_dom.domain = NULL;
  upll_rc_t result_code = GetControllerDomainId(ikey, UPLL_DT_CANDIDATE,
                                               &ctrlr_dom, dmi);
  if ((result_code != UPLL_RC_SUCCESS) || (ctrlr_dom.ctrlr == NULL)) {
    UPLL_LOG_INFO("GetControllerDomainId failed err_code %d", result_code);
    return UPLL_RC_ERR_GENERIC;
  }
  // Validate Controller Type
  unc_keytype_ctrtype_t ctrlrtype;
  uuc::CtrlrMgr *ctrlr_mgr = uuc::CtrlrMgr::GetInstance();
  if (!ctrlr_mgr->GetCtrlrType(
        reinterpret_cast<char *>(ctrlr_dom.ctrlr), req->datatype, &ctrlrtype)) {
    UPLL_LOG_DEBUG("Specified Controller Doesn't Exist");
    return UPLL_RC_ERR_CFG_SEMANTIC;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtepGrpMoMgr::GetControllerDomainId(ConfigKeyVal *ikey,
                                    upll_keytype_datatype_t dt_type,
                                    controller_domain *ctrlr_dom,
                                    DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  ConfigKeyVal *okey = NULL;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  if (!ikey || !ctrlr_dom || !(ikey->get_cfg_val())) {
    UPLL_LOG_INFO("Illegal parameter");
    return UPLL_RC_ERR_GENERIC;
  }
  val_vtep_grp *temp_vtep_grp = reinterpret_cast<val_vtep_grp *>
                                                (GetVal(ikey));
  if (temp_vtep_grp->valid[UPLL_IDX_CONTROLLER_ID_VTEPG] != UNC_VF_VALID) {
    temp_vtep_grp = NULL;
    result_code = GetChildConfigKey(okey, ikey);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_ERROR("GetChildConfigKey failed with result_code %d",
                      result_code);
      return result_code;
    }
    DbSubOp dbop = {kOpReadSingle, kOpMatchNone,
                    kOpInOutCtrlr | kOpInOutDomain | kOpInOutFlag};
    result_code = ReadConfigDB(okey, dt_type, UNC_OP_READ, dbop,
                               dmi, MAINTBL);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_ERROR("Record does Not Exists");
      DELETE_IF_NOT_NULL(okey);
      return result_code;
    }
    temp_vtep_grp = reinterpret_cast<val_vtep_grp *>(GetVal(okey));
    if (!temp_vtep_grp) {
      UPLL_LOG_DEBUG("value null");
      DELETE_IF_NOT_NULL(okey);
      return UPLL_RC_ERR_GENERIC;
    }
    if (temp_vtep_grp->valid[UPLL_IDX_CONTROLLER_ID_VTEPG] != UNC_VF_VALID
        || !strlen(reinterpret_cast<char*>(temp_vtep_grp->controller_id))) {
      ctrlr_dom->ctrlr = NULL;
      UPLL_LOG_DEBUG("Ctrlr null");
    } else {
      SET_USER_DATA(ikey, okey);
      GET_USER_DATA_CTRLR_DOMAIN(ikey, *ctrlr_dom);
      DELETE_IF_NOT_NULL(okey);
    }
  } else {
    SET_USER_DATA_CTRLR(ikey, temp_vtep_grp->controller_id);
    GET_USER_DATA_CTRLR_DOMAIN(ikey, *ctrlr_dom);
  }
  ctrlr_dom->domain = reinterpret_cast<uint8_t *>(const_cast<char*>(" "));
  UPLL_LOG_DEBUG("ctrlr_dom %s %s", ctrlr_dom->ctrlr, ctrlr_dom->domain);
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtepGrpMoMgr::DupConfigKeyVal(ConfigKeyVal *&okey,
    ConfigKeyVal *&req, MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  if (req == NULL) return UPLL_RC_ERR_GENERIC;
  if (okey != NULL) return UPLL_RC_ERR_GENERIC;
  if (req->get_key_type() != UNC_KT_VTEP_GRP)
    return UPLL_RC_ERR_GENERIC;
  ConfigVal *tmp1 = NULL, *tmp = (req)->get_cfg_val();
  if (tmp) {
    if (tbl == MAINTBL) {
      val_vtep_grp *ival = reinterpret_cast<val_vtep_grp *>(GetVal(req));
      val_vtep_grp *vtepgrp_val = reinterpret_cast<val_vtep_grp *>
        (ConfigKeyVal::Malloc(sizeof(val_vtep_grp)));
      if (!vtepgrp_val) return UPLL_RC_ERR_GENERIC;
      memcpy(vtepgrp_val, ival, sizeof(val_vtep_grp));
      tmp1 = new ConfigVal(IpctSt::kIpcStValVtepGrp, vtepgrp_val);
      if (!tmp1) {
        FREE_IF_NOT_NULL(vtepgrp_val);
        return UPLL_RC_ERR_GENERIC;
      }
    }
  };
  void *tkey = (req != NULL)?(req)->get_key():NULL;
  key_vtep_grp *ikey = reinterpret_cast<key_vtep_grp *>(tkey);
  key_vtep_grp *vtepgrp_key = reinterpret_cast<key_vtep_grp *>
    (ConfigKeyVal::Malloc(sizeof(key_vtep_grp)));
  if (!vtepgrp_key) {
    if (tmp1) delete tmp1;
    return UPLL_RC_ERR_GENERIC;
  }
  memcpy(vtepgrp_key, ikey, sizeof(key_vtep_grp));
  okey = new ConfigKeyVal(UNC_KT_VTEP_GRP, IpctSt::kIpcStKeyVtepGrp,
      vtepgrp_key, tmp1);
  if (okey) {
    SET_USER_DATA(okey, req);
  } else {
    if (tmp1) delete tmp1;
    FREE_IF_NOT_NULL(vtepgrp_key);
    return UPLL_RC_ERR_GENERIC;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtepGrpMoMgr::UpdateConfigStatus(ConfigKeyVal *vtepgrp_key,
    unc_keytype_operation_t op,
    uint32_t driver_result,
    ConfigKeyVal *upd_key,
    DalDmlIntf *dmi,
    ConfigKeyVal *ctrlr_key) {
  UPLL_FUNC_TRACE;
  val_vtep_grp_t *vtepgrp_val = reinterpret_cast<val_vtep_grp_t *>
                                (GetVal(vtepgrp_key));

  unc_keytype_configstatus_t cs_status =
    (driver_result == UPLL_RC_SUCCESS) ? UNC_CS_APPLIED : UNC_CS_NOT_APPLIED;
  val_vtep_grp_t *val_running = static_cast<val_vtep_grp_t *>(GetVal(upd_key));
  if (vtepgrp_val == NULL) return UPLL_RC_ERR_GENERIC;
  if (op == UNC_OP_CREATE) {
    vtepgrp_val->cs_row_status = cs_status;
  } else if (op == UNC_OP_UPDATE) {
    void *val = reinterpret_cast<void *>(vtepgrp_val);
    CompareValidValue(val, GetVal(upd_key), true);
    UPLL_LOG_TRACE("%s", (upd_key->ToStrAll()).c_str());
    vtepgrp_val->cs_row_status = val_running->cs_row_status;
  } else {
    return UPLL_RC_ERR_GENERIC;
  }
  for (unsigned int loop = 0;
      loop < sizeof(vtepgrp_val->valid) / sizeof(vtepgrp_val->valid[0]);
      ++loop) {
    if ((UNC_VF_VALID == vtepgrp_val->valid[loop])
        || (UNC_VF_VALID_NO_VALUE == vtepgrp_val->valid[loop])) {
      // Description is set to APPLIED
      if (loop == UPLL_IDX_DESCRIPTION_VTEPG)
        vtepgrp_val->cs_attr[loop] = UNC_CS_APPLIED;
      else
        vtepgrp_val->cs_attr[loop] = cs_status;
    } else if ((vtepgrp_val->valid[loop] == UNC_VF_INVALID) &&
               (UNC_OP_CREATE == op)) {
        vtepgrp_val->cs_attr[loop] = UNC_CS_APPLIED;
    } else if ((vtepgrp_val->valid[loop] == UNC_VF_INVALID) &&
               (UNC_OP_UPDATE == op)) {
        vtepgrp_val->cs_attr[loop] = val_running->cs_attr[loop];
    }
  }
  return UPLL_RC_SUCCESS;
}

bool VtepGrpMoMgr::FilterAttributes(void *&val1, void *val2,
                                    bool copy_to_running,
                                    unc_keytype_operation_t op) {
  UPLL_FUNC_TRACE;
  val_vtep_grp_t *val_vtepgrp1 = reinterpret_cast<val_vtep_grp_t *>(val1);
  val_vtepgrp1->valid[UPLL_IDX_DESCRIPTION_VTEPG] = UNC_VF_INVALID;
  if (op != UNC_OP_CREATE)
    return CompareValidValue(val1, val2, copy_to_running);
  return false;
}

bool VtepGrpMoMgr::CompareValidValue(void *&val1, void *val2,
                                     bool copy_to_running) {
  UPLL_FUNC_TRACE;
  bool invalid_attr = true;
  val_vtep_grp_t *val_vtepgrp1 = reinterpret_cast<val_vtep_grp_t *>(val1);
  val_vtep_grp_t *val_vtepgrp2  = reinterpret_cast<val_vtep_grp_t *>(val2);
  for ( unsigned int loop = 0;
      loop < sizeof(val_vtepgrp1->valid)/sizeof(uint8_t); ++loop ) {
    if ( UNC_VF_INVALID == val_vtepgrp1->valid[loop]
      && UNC_VF_VALID == val_vtepgrp2->valid[loop])
      val_vtepgrp1->valid[loop] = UNC_VF_VALID_NO_VALUE;
  }
  if (UNC_VF_VALID == val_vtepgrp1->valid[UPLL_IDX_DESCRIPTION_VTEPG]
      && UNC_VF_VALID == val_vtepgrp2->valid[UPLL_IDX_DESCRIPTION_VTEPG]) {
    if (!strcmp(reinterpret_cast<char*>(val_vtepgrp1->description),
          reinterpret_cast<char*>(val_vtepgrp2->description)))
      val_vtepgrp1->valid[UPLL_IDX_DESCRIPTION_VTEPG] = UNC_VF_INVALID;
  }
  if (UNC_VF_VALID == val_vtepgrp1->valid[UPLL_IDX_CONTROLLER_ID_VTEPG]
      && UNC_VF_VALID == val_vtepgrp2->valid[UPLL_IDX_CONTROLLER_ID_VTEPG]) {
    if (!strcmp(reinterpret_cast<char*>(val_vtepgrp1->controller_id),
          reinterpret_cast<char*>(val_vtepgrp2->controller_id)))
      val_vtepgrp1->valid[UPLL_IDX_CONTROLLER_ID_VTEPG] = UNC_VF_INVALID;
  }
  if (!copy_to_running)
     val_vtepgrp1->valid[UPLL_IDX_DESCRIPTION_VTEPG] = UNC_VF_INVALID;
  for (unsigned int loop = 0;
      loop < sizeof(val_vtepgrp1->valid) / sizeof(uint8_t); ++loop) {
    if ((UNC_VF_VALID == (uint8_t) val_vtepgrp1->valid[loop]) ||
        (UNC_VF_VALID_NO_VALUE == (uint8_t) val_vtepgrp1->valid[loop])) {
      invalid_attr = false;
      break;
    }
  }
  return invalid_attr;
}

upll_rc_t VtepGrpMoMgr::UpdateAuditConfigStatus(
    unc_keytype_configstatus_t cs_status,
    uuc::UpdateCtrlrPhase phase,
    ConfigKeyVal *&ckv_running,
    DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  val_vtep_grp_t *val = (ckv_running != NULL)?
    reinterpret_cast<val_vtep_grp_t *>(GetVal(ckv_running)):NULL;
  if (NULL == val) {
    return UPLL_RC_ERR_GENERIC;
  }
  if (uuc::kUpllUcpCreate == phase )
    val->cs_row_status = cs_status;
  if ((uuc::kUpllUcpUpdate == phase) &&
           (val->cs_row_status == UNC_CS_INVALID ||
            val->cs_row_status == UNC_CS_NOT_APPLIED))
    val->cs_row_status = cs_status;
  for (unsigned int loop = 0;
      loop < sizeof(val->valid)/sizeof(uint8_t); ++loop ) {
    if ((cs_status == UNC_CS_INVALID &&  UNC_VF_VALID == val->valid[loop]) ||
        cs_status == UNC_CS_APPLIED)
      val->cs_attr[loop] = cs_status;
  }
  return result_code;
}

upll_rc_t VtepGrpMoMgr::ValidateMessage(IpcReqRespHeader *req,
    ConfigKeyVal *ikey) {
  UPLL_FUNC_TRACE;
  upll_rc_t ret_val = UPLL_RC_SUCCESS;
  if (!ikey || !req || !(ikey->get_key())) {
    UPLL_LOG_DEBUG("IpcReqRespHeader or ConfigKeyVal is Null");
    return UPLL_RC_ERR_BAD_REQUEST;
  }
  unc_key_type_t keytype = ikey->get_key_type();
  if (UNC_KT_VTEP_GRP != keytype) {
    UPLL_LOG_DEBUG("Invalid keytype. Keytype- %d", keytype);
    return UPLL_RC_ERR_BAD_REQUEST;
  }
  if (ikey->get_st_num() != IpctSt::kIpcStKeyVtepGrp) {
    UPLL_LOG_DEBUG("Invalid struct received.Expected struct-kIpcStKeyVtepGrp,"
        "received struct -%s ", reinterpret_cast<const char *>
        (IpctSt::GetIpcStdef(ikey->get_st_num())));
    return UPLL_RC_ERR_BAD_REQUEST;
  }
  key_vtep_grp_t *key_vtep_grp = reinterpret_cast<key_vtep_grp_t *>
    (ikey->get_key());
  val_vtep_grp_t *val_vtep_grp = NULL;
  if ((ikey->get_cfg_val()) &&
      ((ikey->get_cfg_val())->get_st_num() == IpctSt::kIpcStValVtepGrp)) {
    val_vtep_grp =
      reinterpret_cast <val_vtep_grp_t *> (ikey->get_cfg_val()->get_val());
  }
  uint32_t dt_type = req->datatype;
  uint32_t operation = req->operation;
  uint32_t option1 = req->option1;
  uint32_t option2 = req->option2;

  ret_val = ValidateVTepGrpKey(key_vtep_grp, operation);
  if (ret_val != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Key struct Validation failed for VTEP_GRP");
    return UPLL_RC_ERR_CFG_SYNTAX;
  }
  if ((operation == UNC_OP_CREATE || operation == UNC_OP_UPDATE) &&
      (dt_type == UPLL_DT_CANDIDATE)) {
    if (val_vtep_grp != NULL) {
      ret_val = ValidateVTepGrpValue(val_vtep_grp, operation);
      if (ret_val != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Val struct validation failed for CREATE/Update op");
        return UPLL_RC_ERR_CFG_SYNTAX;
      }
      return UPLL_RC_SUCCESS;
    } else {
      UPLL_LOG_DEBUG("Value structure is mandatory for CREATE/Update op");
      return UPLL_RC_ERR_BAD_REQUEST;
    }
  } else if (((operation == UNC_OP_READ) ||
        (operation == UNC_OP_READ_SIBLING) ||
        (operation == UNC_OP_READ_SIBLING_BEGIN) ||
        (operation == UNC_OP_READ_SIBLING_COUNT)) &&
      ((dt_type == UPLL_DT_CANDIDATE) ||
       (dt_type == UPLL_DT_RUNNING) ||
       (dt_type == UPLL_DT_STARTUP) ||
       (dt_type == UPLL_DT_STATE))) {
    if (option1 == UNC_OPT1_NORMAL) {
      if (option2 == UNC_OPT2_NONE) {
        if (val_vtep_grp != NULL) {
          ret_val = ValidateVTepGrpValue(val_vtep_grp);
          if (ret_val != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("Val struct validation failed for "
                           "READ operation");
            return UPLL_RC_ERR_CFG_SYNTAX;
          }
          return UPLL_RC_SUCCESS;
        } else {
          UPLL_LOG_TRACE("Value strcture is an optional for READ op");
          return UPLL_RC_SUCCESS;
        }
      } else {
        UPLL_LOG_DEBUG("Option2 is not matching");
        return UPLL_RC_ERR_INVALID_OPTION2;
      }
    } else {
      UPLL_LOG_DEBUG("Option1 is not matching");
      return UPLL_RC_ERR_INVALID_OPTION1;
    }
  } else if ((operation == UNC_OP_DELETE) ||
        (((operation == UNC_OP_READ_NEXT) ||
        (operation == UNC_OP_READ_BULK)) &&
        ((dt_type == UPLL_DT_CANDIDATE) ||
          (dt_type == UPLL_DT_RUNNING) ||
          (dt_type == UPLL_DT_STARTUP)))) {
    UPLL_LOG_TRACE("Value structure is none for this operation:%d", operation);
    return UPLL_RC_SUCCESS;
  } else {
    UPLL_LOG_INFO("Invalid datatype(%d) or operation(%d)", dt_type,
        operation);
    return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtepGrpMoMgr::ValidateVTepGrpValue(val_vtep_grp_t *val_vtep_grp,
    uint32_t operation) {
  UPLL_FUNC_TRACE;
  bool ret_val =false;

  // Attribute syntax validation
  for (unsigned int valid_index = 0;
       valid_index < sizeof(val_vtep_grp->valid)/sizeof(val_vtep_grp->valid[0]);
       valid_index++) {
    if (val_vtep_grp->valid[valid_index] == UNC_VF_VALID) {
      switch (valid_index) {
        case UPLL_IDX_CONTROLLER_ID_VTEPG:
          ret_val = ValidateString(val_vtep_grp->controller_id,
                                   kMinLenCtrlrId, kMaxLenCtrlrId);
          break;
        case UPLL_IDX_DESCRIPTION_VTEPG:
          ret_val = ValidateDesc(val_vtep_grp->description,
                                 kMinLenDescription, kMaxLenDescription);
          break;
      }
      if (!ret_val) {
        return UPLL_RC_ERR_CFG_SYNTAX;
      }
    }
  }

  // Additional checks
  for (unsigned int valid_index = 0;
       valid_index < sizeof(val_vtep_grp->valid)/sizeof(val_vtep_grp->valid[0]);
       valid_index++) {
    uint8_t flag = val_vtep_grp->valid[valid_index];
    switch (operation) {
      case UNC_OP_CREATE:
        {
          switch (valid_index) {
            case UPLL_IDX_CONTROLLER_ID_VTEPG:
              if ((flag == UNC_VF_INVALID || flag == UNC_VF_VALID_NO_VALUE)) {
                UPLL_LOG_DEBUG("controller_id flag is invalid"
                               " or valid_no_value");
                return UPLL_RC_ERR_CFG_SYNTAX;
              }
              break;
            case UPLL_IDX_DESCRIPTION_VTEPG:
              break;
            default:
              break;
          }
        }
        break;
      case UNC_OP_UPDATE:
        {
          switch (valid_index) {
            case UPLL_IDX_CONTROLLER_ID_VTEPG:
              if (flag == UNC_VF_VALID_NO_VALUE) {
                UPLL_LOG_DEBUG("controller_id flag is valid_no_value");
                return UPLL_RC_ERR_CFG_SYNTAX;
              }
              break;
            case UPLL_IDX_DESCRIPTION_VTEPG:
              break;
            default:
              break;
          }
        }
        break;
    }
  }

  // Resets
  for (unsigned int valid_index = 0;
       valid_index < sizeof(val_vtep_grp->valid)/sizeof(val_vtep_grp->valid[0]);
       valid_index++) {
    uint8_t flag = val_vtep_grp->valid[valid_index];
    if (flag != UNC_VF_INVALID && flag != UNC_VF_VALID_NO_VALUE)
      continue;

    switch (valid_index) {
      case UPLL_IDX_CONTROLLER_ID_VTEPG:
        StringReset(val_vtep_grp->controller_id);
        break;
      case UPLL_IDX_DESCRIPTION_VTEPG:
        StringReset(val_vtep_grp->description);
        break;
      default:
        UPLL_LOG_TRACE("Never here");
        break;
    }
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtepGrpMoMgr::CtrlrIdAndDomainIdUpdationCheck(ConfigKeyVal *ikey,
                                                   ConfigKeyVal *okey) {
  UPLL_FUNC_TRACE;
  val_vtep_grp *vtepg_val = reinterpret_cast<val_vtep_grp *>(GetVal(ikey));
  val_vtep_grp *vtepg_val1 = reinterpret_cast<val_vtep_grp *>(GetVal(okey));
  if (vtepg_val->valid[UPLL_IDX_CONTROLLER_ID_VTEPG] == UNC_VF_VALID) {
    if (strncmp(reinterpret_cast<const char *>(vtepg_val->controller_id),
                reinterpret_cast<const char *>(vtepg_val1->controller_id),
                kMaxLenCtrlrId+1)) {
      UPLL_LOG_DEBUG("controller id comparision failed");
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  }
  return UPLL_RC_SUCCESS;
}
upll_rc_t VtepGrpMoMgr::ValidateVTepGrpKey(key_vtep_grp_t *key_vtep_grp,
                        uint32_t operation) {
  UPLL_FUNC_TRACE;
  upll_rc_t ret_val = UPLL_RC_SUCCESS;
  ret_val = ValidateKey(
  reinterpret_cast<char *>(key_vtep_grp->vtn_key.vtn_name),
      kMinLenVtnName, kMaxLenVtnName);
  if (ret_val != UPLL_RC_SUCCESS) {
    UPLL_LOG_INFO("Vtn Name syntax check failed."
                  "Received VTN Name - %s",
                  key_vtep_grp->vtn_key.vtn_name);
    return UPLL_RC_ERR_CFG_SYNTAX;
  }
  if ((operation != UNC_OP_READ_SIBLING_BEGIN) &&
      (operation != UNC_OP_READ_SIBLING_COUNT)) {
    UPLL_LOG_TRACE("UNC_KT_VTEP_GRP:vtepgrp_name (%s)",
                    key_vtep_grp->vtepgrp_name);
    ret_val = ValidateKey(reinterpret_cast<char *>(key_vtep_grp->vtepgrp_name),
              kMinLenVnodeName, kMaxLenVnodeName);
    if (ret_val != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Syntax check failed. vtepgrp_name (%s)",
                      key_vtep_grp->vtepgrp_name);
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  } else {
    UPLL_LOG_TRACE("Operation is %d", operation);
    StringReset(key_vtep_grp->vtepgrp_name);
  }
  return UPLL_RC_SUCCESS;
}
upll_rc_t VtepGrpMoMgr::ValVTepGrpAttributeSupportCheck(
    val_vtep_grp_t *val_vtep_grp,
    const uint8_t* attrs, unc_keytype_operation_t operation) {
  UPLL_FUNC_TRACE;

  if ((val_vtep_grp->valid[UPLL_IDX_DESCRIPTION_VTEPG] == UNC_VF_VALID) ||
      (val_vtep_grp->valid[UPLL_IDX_DESCRIPTION_VTEPG] ==
       UNC_VF_VALID_NO_VALUE)) {
    if (attrs[unc::capa::vtep_grp::kCapDesc] == 0) {
      val_vtep_grp->valid[UPLL_IDX_DESCRIPTION_VTEPG] = UNC_VF_INVALID;
      if (operation == UNC_OP_CREATE || operation == UNC_OP_UPDATE) {
        UPLL_LOG_DEBUG("Attribute Desc not supported in pfc controller");
        return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
      }
    }
  }
  return UPLL_RC_SUCCESS;
}
upll_rc_t VtepGrpMoMgr::ValidateCapability(IpcReqRespHeader *req,
    ConfigKeyVal *ikey, const char * ctrlr_name) {
  UPLL_FUNC_TRACE;
  if (!ikey || !req) {
    UPLL_LOG_DEBUG("ConfigKeyVal / IpcReqRespHeader is Null");
    return UPLL_RC_ERR_GENERIC;
  }

  if (!ctrlr_name) {
    ctrlr_name = reinterpret_cast<char*>((reinterpret_cast<key_user_data_t *>
                  (ikey->get_user_data()))->ctrlr_id);
    if (!ctrlr_name || !strlen(ctrlr_name)) {
      UPLL_LOG_DEBUG("Controller Name is NULL");
      return UPLL_RC_ERR_GENERIC;
    }
  }

  UPLL_LOG_TRACE("ctrlr_name(%s), operation : (%d)",
      ctrlr_name, req->operation);

  bool result_code = false;
  uint32_t max_instance_count = 0;
  uint32_t max_attrs = 0;
  const uint8_t *attrs = NULL;
  switch (req->operation) {
    case UNC_OP_CREATE:
      result_code = GetCreateCapability(ctrlr_name,
          ikey->get_key_type(), &max_instance_count, &max_attrs, &attrs);
      break;
    case UNC_OP_UPDATE: {
        result_code = GetUpdateCapability(ctrlr_name,
            ikey->get_key_type(), &max_attrs, &attrs);
      }
      break;
    case UNC_OP_READ:
    case UNC_OP_READ_SIBLING:
    case UNC_OP_READ_SIBLING_BEGIN:
    case UNC_OP_READ_SIBLING_COUNT:
      result_code = GetReadCapability(ctrlr_name,
          ikey->get_key_type(), &max_attrs, &attrs);
      break;

    default:
      UPLL_LOG_DEBUG("Invalid operation");
      break;
  }

  if (!result_code) {
    UPLL_LOG_DEBUG("keytype(%d) is not supported by controller(%s) "
        "for operation(%d)",
        ikey->get_key_type(), ctrlr_name, req->operation);
    return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
  }
  val_vtep_grp_t *val_vtep_grp = NULL;
  if ((ikey->get_cfg_val()) &&
      ((ikey->get_cfg_val())->get_st_num() == IpctSt::kIpcStValVtepGrp)) {
    val_vtep_grp =
      reinterpret_cast<val_vtep_grp_t *> (ikey->get_cfg_val()->get_val());
  }
  if (val_vtep_grp) {
    if (max_attrs > 0) {
      return ValVTepGrpAttributeSupportCheck(val_vtep_grp, attrs,
                                             req->operation);
    } else {
      UPLL_LOG_DEBUG("Attribute list is empty for operation %d",
                     req->operation);
      return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
    }
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtepGrpMoMgr::CreateVtunnelKey(ConfigKeyVal *ikey,
    ConfigKeyVal *&okey) {
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigVal *tmp;
  if ( NULL == ikey)
    return UPLL_RC_ERR_GENERIC;
  key_vtep_grp *vtepgrp_key = reinterpret_cast<key_vtep_grp *>
                              (ikey->get_key());

  val_vtunnel_t *val_vtunnel = reinterpret_cast<val_vtunnel_t *>
                               (ConfigKeyVal::Malloc(sizeof(val_vtunnel_t)));
  key_vtunnel_t *key_vtunnel = reinterpret_cast<key_vtunnel_t *>
                               (ConfigKeyVal::Malloc(sizeof(key_vtunnel_t)));

  if (!val_vtunnel || !key_vtunnel)
    return UPLL_RC_ERR_GENERIC;
  if (!strlen(reinterpret_cast<char *>(vtepgrp_key->vtn_key.vtn_name))) {
    free(val_vtunnel);
    free(key_vtunnel);
    return UPLL_RC_ERR_GENERIC;
  }
  if (!strlen(reinterpret_cast<char *>(vtepgrp_key->vtepgrp_name))) {
    free(val_vtunnel);
    free(key_vtunnel);
    return UPLL_RC_ERR_GENERIC;
  }

  uuu::upll_strncpy(val_vtunnel->vtn_name,
                    vtepgrp_key->vtn_key.vtn_name,
                   (kMaxLenVtnName+1));
  uuu::upll_strncpy(val_vtunnel->vtep_grp_name,
                    vtepgrp_key->vtepgrp_name,
                   (kMaxLenVnodeName+1));
  val_vtunnel->valid[UPLL_IDX_VTN_NAME_VTNL] = UNC_VF_VALID;
  val_vtunnel->valid[UPLL_IDX_VTEP_GRP_NAME_VTNL] = UNC_VF_VALID;

  tmp = new ConfigVal(IpctSt::kIpcStValVtunnel, val_vtunnel);
  okey = new ConfigKeyVal(UNC_KT_VTUNNEL, IpctSt::kIpcStKeyVtunnel,
                          key_vtunnel, tmp);
  UPLL_LOG_TRACE("VtnMoMgr::CreateVtunnelKey");
  if (!okey) {
    FREE_IF_NOT_NULL(key_vtunnel);
    FREE_IF_NOT_NULL(val_vtunnel);
    delete okey;
    return UPLL_RC_ERR_GENERIC;
  }
  return result_code;
}

upll_rc_t VtepGrpMoMgr::IsReferenced(IpcReqRespHeader *req,
    ConfigKeyVal *ikey, DalDmlIntf *dmi) {
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *okey = NULL;
  if ( !ikey || !(ikey->get_key()) || !dmi)
    return UPLL_RC_ERR_GENERIC;
  result_code = CreateVtunnelKey(ikey, okey);

  MoMgrImpl *mgr = reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>
                                            (GetMoManager(UNC_KT_VTUNNEL)));
  DbSubOp dbop = {kOpReadMultiple, kOpMatchNone, kOpInOutNone};
  result_code = mgr->ReadConfigDB(okey, req->datatype, UNC_OP_READ, dbop,
                                  dmi, MAINTBL);
  UPLL_LOG_TRACE("Vtepgroup ReadConfigDb");
  if (UPLL_RC_SUCCESS == result_code) {
    result_code =  UPLL_RC_ERR_CFG_SEMANTIC;
  }
  result_code = (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE)?
                 UPLL_RC_SUCCESS:result_code;
  DELETE_IF_NOT_NULL(okey);
  return result_code;
}


}  // namespace kt_momgr
}  // namespace upll
}  // namespace unc
