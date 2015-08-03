/*
 * Copyright (c) 2013-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include "vterminal_momgr.hh"
#include "vbr_if_momgr.hh"
#define NUM_KEY_MAIN_TBL_  5
#define NUM_KEY_RENAME_TBL_ 4


namespace unc {
namespace upll {
namespace kt_momgr {
unc_key_type_t VterminalMoMgr::vterminal_child[] = { UNC_KT_VTERM_IF };

BindInfo VterminalMoMgr::vterminal_bind_info[] = {
  { uudst::vterminal::kDbiVtnName, CFG_KEY,
    offsetof(key_vterm, vtn_key.vtn_name),
    uud::kDalChar, (kMaxLenVtnName + 1) },
  { uudst::vterminal::kDbiVterminalName,
    CFG_KEY,
    offsetof(key_vterm, vterminal_name),
    uud::kDalChar, (kMaxLenVnodeName + 1) },
  { uudst::vterminal::kDbiCtrlrName,
    CFG_VAL,
    offsetof(val_vterm, controller_id),
    uud::kDalChar, (kMaxLenCtrlrId + 1) },
  { uudst::vterminal::kDbiCtrlrName,
    CK_VAL,
    offsetof(key_user_data, ctrlr_id),
    uud::kDalChar, (kMaxLenCtrlrId + 1) },
  { uudst::vterminal::kDbiDomainId,
    CFG_VAL,
    offsetof(val_vterm, domain_id),
    uud::kDalChar, (kMaxLenDomainId + 1) },
  { uudst::vterminal::kDbiDomainId,
    CK_VAL,
    offsetof(key_user_data, domain_id),
    uud::kDalChar, (kMaxLenDomainId + 1) },
  { uudst::vterminal::kDbiVterminalDesc,
    CFG_VAL,
    offsetof(val_vterm, vterm_description),
    uud::kDalChar, (kMaxLenDescription + 1) },
  { uudst::vterminal::kDbiOperStatus,
    ST_VAL,
    offsetof(val_db_vterm_st,
        vterm_val_st.oper_status),
    uud::kDalUint8, 1 },
  { uudst::vterminal::kDbiDownCount,
    ST_VAL,
    offsetof(val_db_vterm_st, down_count),
    uud::kDalUint32, 1 },
  { uudst::vterminal::kDbiUnknownCount,
    ST_VAL,
    offsetof(val_db_vterm_st, unknown_count),
    uud::kDalUint32, 1 },
  { uudst::vterminal::kDbiValidCtrlrName,
    CFG_META_VAL, offsetof(val_vterm,
        valid[0]),
    uud::kDalUint8, 1 },
  { uudst::vterminal::kDbiValidDomainId,
    CFG_META_VAL, offsetof(val_vterm,
        valid[1]),
    uud::kDalUint8, 1 },
  { uudst::vterminal::
    kDbiValidVterminalDesc,
    CFG_META_VAL, offsetof(val_vterm,
        valid[2]),
    uud::kDalUint8, 1 },
  { uudst::vterminal::kDbiValidOperStatus,
    ST_META_VAL, offsetof(val_vterm_st,
        valid[0]),
    uud::kDalUint8, 1 },
  { uudst::vterminal::kDbiCsRowStatus,
    CS_VAL, offsetof(val_vterm,
        cs_row_status),
    uud::kDalUint8, 1 },
  { uudst::vterminal::kDbiCsCtrlrName,
    CS_VAL,
    offsetof(val_vterm, cs_attr[0]),
    uud::kDalUint8, 1 },
  { uudst::vterminal::kDbiCsDomainId,
    CS_VAL,
    offsetof(val_vterm, cs_attr[1]),
    uud::kDalUint8, 1 },
  { uudst::vterminal::kDbiCsVterminalDesc,
    CS_VAL,
    offsetof(val_vterm, cs_attr[2]),
    uud::kDalUint8, 1 },
  { uudst::vterminal::kDbiVterminalFlags,
    CK_VAL,
    offsetof(key_user_data_t, flags),
    uud::kDalUint8, 1 }
};

BindInfo VterminalMoMgr::vterminal_rename_bind_info[] = {
  { uudst::vnode_rename::kDbiUncVtnName, CFG_KEY, offsetof(key_vterm,
      vtn_key.vtn_name),
  uud::kDalChar, (kMaxLenVtnName + 1) },
  { uudst::vnode_rename::kDbiUncvnodeName, CFG_KEY, offsetof(key_vterm,
      vterminal_name),
  uud::kDalChar, (kMaxLenVnodeName + 1) },
  { uudst::vnode_rename::kDbiCtrlrName, CK_VAL, offsetof(key_user_data_t,
      ctrlr_id),
  uud::kDalChar, (kMaxLenCtrlrId + 1) },
  { uudst::vnode_rename::kDbiDomainId, CK_VAL, offsetof(key_user_data_t,
      domain_id),
  uud::kDalChar, (kMaxLenDomainId + 1) },
  { uudst::vnode_rename::kDbiCtrlrVtnName, CFG_VAL, offsetof(val_rename_vnode,
      ctrlr_vtn_name),
  uud::kDalChar, (kMaxLenVtnName + 1) },
  { uudst::vnode_rename::kDbiCtrlrVnodeName, CFG_VAL, offsetof(
      val_rename_vnode, ctrlr_vnode_name),
  uud::kDalChar, (kMaxLenVnodeName + 1) } };

BindInfo VterminalMoMgr::key_vterminal_maintbl_bind_info[] = {
  { uudst::vterminal::kDbiVtnName, CFG_MATCH_KEY, offsetof(key_vterm_t,
      vtn_key.vtn_name),
  uud::kDalChar, kMaxLenVtnName + 1 },
  { uudst::vterminal::kDbiVterminalName, CFG_MATCH_KEY, offsetof(key_vterm_t,
      vterminal_name),
  uud::kDalChar, kMaxLenVnodeName + 1 },
  { uudst::vterminal::kDbiVtnName, CFG_INPUT_KEY, offsetof(
      key_rename_vnode_info_t, new_unc_vtn_name),
  uud::kDalChar, kMaxLenVtnName + 1 },
  { uudst::vterminal::kDbiVterminalName, CFG_INPUT_KEY, offsetof(
      key_rename_vnode_info_t, new_unc_vnode_name),
  uud::kDalChar, kMaxLenVnodeName + 1 },
  { uudst::vterminal::kDbiVterminalFlags, CK_VAL, offsetof(key_user_data_t,
      flags),
  uud::kDalUint8, 1 } };

BindInfo VterminalMoMgr::key_vterminal_renametbl_update_bind_info[] = {
  { uudst::vnode_rename::kDbiUncVtnName, CFG_MATCH_KEY, offsetof(
      key_vterm_t, vtn_key.vtn_name),
  uud::kDalChar, kMaxLenVtnName + 1 },
  { uudst::vnode_rename::kDbiUncvnodeName, CFG_MATCH_KEY, offsetof(
      key_vterm_t, vterminal_name),
  uud::kDalChar, kMaxLenVtnName + 1 },
  { uudst::vnode_rename::kDbiUncVtnName, CFG_INPUT_KEY, offsetof(
      key_rename_vnode_info_t, new_unc_vtn_name),
  uud::kDalChar, kMaxLenVtnName + 1 },
  { uudst::vnode_rename::kDbiUncvnodeName, CFG_INPUT_KEY, offsetof(
      key_rename_vnode_info_t, new_unc_vnode_name),
  uud::kDalChar, kMaxLenVtnName + 1 } };

upll_rc_t VterminalMoMgr::GetValid(void *val,
    uint64_t indx,
    uint8_t *&valid,
    upll_keytype_datatype_t dt_type,
    MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  if (val == NULL) return UPLL_RC_ERR_GENERIC;
  /** Gets the valid array position of the variable in the value
   *  structure from the MAINTBL.
   */
  if (tbl == MAINTBL) {
    switch (indx) {
      case uudst::vterminal::kDbiOperStatus:
        valid = &(reinterpret_cast<val_db_vterm_st *>(val))->
          vterm_val_st.valid[UPLL_IDX_OPER_STATUS_VTERMS];
        break;
      case uudst::vterminal::kDbiDownCount:
      case uudst::vterminal::kDbiUnknownCount:
        valid = NULL;
        break;
      case uudst::vterminal::kDbiCtrlrName:
        valid = &(reinterpret_cast<val_vterm *>(val))->
          valid[UPLL_IDX_CONTROLLER_ID_VTERM];
        break;
      case uudst::vterminal::kDbiDomainId:
        valid = &(reinterpret_cast<val_vterm *>(val))->
          valid[UPLL_IDX_DOMAIN_ID_VTERM];
        break;
      case uudst::vterminal::kDbiVterminalDesc:
        valid = &(reinterpret_cast<val_vterm *>(val))->
          valid[UPLL_IDX_DESC_VTERM];
        break;
      default:
        UPLL_LOG_DEBUG("Given index not available in MAINTBL");
        return UPLL_RC_ERR_GENERIC;
    }
    /** Gets the valid array position of the variable in the value
     *  structure from the RENAMETBL.
     */
  } else if (tbl == RENAMETBL) {
    switch (indx) {
      case uudst::vnode_rename::kDbiCtrlrVtnName:
        valid = &(reinterpret_cast<val_rename_vnode*>
            (val))->valid[UPLL_CTRLR_VTN_NAME_VALID];
        break;
      case  uudst::vnode_rename::kDbiCtrlrVnodeName:
        valid = &(reinterpret_cast<val_rename_vnode*>
            (val))->valid[UPLL_CTRLR_VNODE_NAME_VALID];
        break;
      default:
        UPLL_LOG_DEBUG("Given index not available in RENAMETBL");
        return UPLL_RC_ERR_GENERIC;
    }
    return UPLL_RC_SUCCESS;
  }
  return UPLL_RC_SUCCESS;
}

bool VterminalMoMgr::FilterAttributes(void *&val1,
    void *val2,
    bool copy_to_running,
    unc_keytype_operation_t op) {
  UPLL_FUNC_TRACE;
  val_vterm_t *val_vterm1 = reinterpret_cast<val_vterm_t *>(val1);
  val_vterm_t *val_vterm2 = reinterpret_cast<val_vterm_t *>(val2);
  /**
   * No need to configure description in controller.
   */
  val_vterm1->valid[UPLL_IDX_DESC_VTERM] = UNC_VF_INVALID;
  /**
   * Filters the attributes which need not be sent to controller.
   */
  if (op == UNC_OP_UPDATE) {
    val_vterm1->valid[UPLL_IDX_CONTROLLER_ID_VTERM] = UNC_VF_INVALID;
    val_vterm1->valid[UPLL_IDX_DOMAIN_ID_VTERM] = UNC_VF_INVALID;
    val_vterm2->valid[UPLL_IDX_CONTROLLER_ID_VTERM] = UNC_VF_INVALID;
    val_vterm2->valid[UPLL_IDX_DOMAIN_ID_VTERM] = UNC_VF_INVALID;
  }
  if (op != UNC_OP_CREATE)
    return CompareValidValue(val1, val2, copy_to_running);
  return false;
}

bool VterminalMoMgr::CompareValidValue(void *&val1,
    void *val2,
    bool copy_to_running) {
  UPLL_FUNC_TRACE;
  bool invalid_attr = true;
  val_vterm_t *val_vterm1 = reinterpret_cast<val_vterm_t *>(val1);
  val_vterm_t *val_vterm2 = reinterpret_cast<val_vterm_t *>(val2);
  if (!val_vterm1 || !val_vterm2) {
    UPLL_LOG_DEBUG("Invalid param");
    return false;
  }
  for (unsigned int loop = 0;
      loop < sizeof(val_vterm1->valid) / sizeof(uint8_t); ++loop) {
    /**
     * Compares the valid value between two database
     * records(CANDIDATE DB, RUNNING DB).
     * if both the values are different, update the valid flag for
     * corresponding attribute as validnovalue in the first record.
     */
    if (UNC_VF_INVALID == val_vterm1->valid[loop]
        && UNC_VF_VALID == val_vterm2->valid[loop])
      val_vterm1->valid[loop] = UNC_VF_VALID_NO_VALUE;
  }
  /**
   * Compares the valid value between two database
   * records(CANDIDATE DB, RUNNING DB).
   * if both the values are same, update the valid flag for
   * corresponding attribute as invalid in the first record.
   */
  if (UNC_VF_INVALID != val_vterm1->valid[UPLL_IDX_DESC_VTERM]) {
    if (!copy_to_running ||
        ((UNC_VF_VALID == val_vterm1->valid[UPLL_IDX_DESC_VTERM]) &&
         !strcmp(reinterpret_cast<char*>(val_vterm1->vterm_description),
           reinterpret_cast<char*>(val_vterm2->vterm_description))))
      val_vterm1->valid[UPLL_IDX_DESC_VTERM] = UNC_VF_INVALID;
  }
  if (UNC_VF_VALID == val_vterm1->valid[UPLL_IDX_CONTROLLER_ID_VTERM]
      && UNC_VF_VALID == val_vterm2->valid[UPLL_IDX_CONTROLLER_ID_VTERM]) {
    if (!strcmp(reinterpret_cast<char*>(val_vterm1->controller_id),
          reinterpret_cast<char*>(val_vterm2->controller_id)))
      val_vterm1->valid[UPLL_IDX_CONTROLLER_ID_VTERM] = UNC_VF_INVALID;
  }
  if (UNC_VF_VALID == val_vterm1->valid[UPLL_IDX_DOMAIN_ID_VTERM]
      && UNC_VF_VALID == val_vterm2->valid[UPLL_IDX_DOMAIN_ID_VTERM]) {
    if (!strcmp(reinterpret_cast<char*>(val_vterm1->domain_id),
          reinterpret_cast<char*>(val_vterm2->domain_id)))
      val_vterm1->valid[UPLL_IDX_DOMAIN_ID_VTERM] = UNC_VF_INVALID;
  }
  /**
   * Description is not send to Controller
   */
  if (!copy_to_running)
    val_vterm1->valid[UPLL_IDX_DESC_VTERM] = UNC_VF_INVALID;
  for (unsigned int loop = 0;
      loop < sizeof(val_vterm1->valid) / sizeof(uint8_t); ++loop) {
    if ((UNC_VF_VALID == (uint8_t) val_vterm1->valid[loop]) ||
        (UNC_VF_VALID_NO_VALUE == (uint8_t) val_vterm1->valid[loop])) {
      invalid_attr = false;
      break;
    }
  }
  /**
   * Return false, get update only description.
   */
  return invalid_attr;
}

upll_rc_t VterminalMoMgr::SwapKeyVal(ConfigKeyVal *ikey,
    ConfigKeyVal *&okey,
    DalDmlIntf *dmi,
    uint8_t *ctrlr,
    bool &no_rename) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  okey = NULL;
  if (!ikey || !(ikey->get_key()))
    return UPLL_RC_ERR_GENERIC;

  if (ikey->get_key_type() != UNC_KT_VTERMINAL) return UPLL_RC_ERR_BAD_REQUEST;

  val_rename_vterm_t *tval = reinterpret_cast<val_rename_vterm_t *>
    (GetVal(ikey));
  if (!tval) {
    UPLL_LOG_DEBUG("tval is null");
    return UPLL_RC_ERR_GENERIC;
  }
  /**
   * The PFC Name and New Name should not be equal
   */
  if (!strcmp(reinterpret_cast<char *>(tval->new_name),
        reinterpret_cast<char *>(reinterpret_cast<key_vterm_t *>
          (ikey->get_key())->vterminal_name)))
    return UPLL_RC_ERR_GENERIC;

  key_vterm_t *key_vterm = reinterpret_cast<key_vterm_t *>
    (ConfigKeyVal::Malloc(sizeof(key_vterm_t)));

  if (tval->valid[UPLL_IDX_NEW_NAME_RVTERM] == UNC_VF_VALID_NO_VALUE) {
    uuu::upll_strncpy(key_vterm->vterminal_name, (static_cast<key_vterm_t *>
          (ikey->get_key())->vterminal_name), (kMaxLenVnodeName+1));
    no_rename = true;
  } else {
    if (reinterpret_cast<val_rename_vterm_t *>
        (tval)->valid[UPLL_IDX_NEW_NAME_RVTERM] == UNC_VF_VALID) {
      /**
       * checking the string is empty or not
       */
      if (!strlen(reinterpret_cast<char *>(static_cast<val_rename_vterm_t *>
              (tval)->new_name))) {
        FREE_IF_NOT_NULL(key_vterm);
        return UPLL_RC_ERR_GENERIC;
      }
      uuu::upll_strncpy(key_vterm->vterminal_name,
          (static_cast<val_rename_vterm_t *>
           ((ikey->get_cfg_val())->get_val()))->new_name,
          (kMaxLenVnodeName+1));
    } else {
      FREE_IF_NOT_NULL(key_vterm);
      return UPLL_RC_ERR_GENERIC;
    }
  }
  /**
   * Checking the vterminal parent is renamed than get the UNC name.
   */
  ConfigKeyVal *pkey = NULL;
  result_code = GetParentConfigKey(pkey, ikey);
  if (UPLL_RC_SUCCESS != result_code || pkey == NULL) {
    if (pkey) DELETE_IF_NOT_NULL(pkey);
    FREE_IF_NOT_NULL(key_vterm);
    return result_code;
  }
  MoMgrImpl *mgr = reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>
      (GetMoManager(UNC_KT_VTN)));
  result_code = mgr->GetRenamedUncKey(pkey, UPLL_DT_IMPORT, dmi, ctrlr);
  if (UPLL_RC_SUCCESS != result_code
      && UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
    DELETE_IF_NOT_NULL(pkey);
    FREE_IF_NOT_NULL(key_vterm);
    return result_code;
  }
  /**
   * Use the UNC VTN name if PFC VTN name is renamed.
   */
  if (strlen(reinterpret_cast<char *>(reinterpret_cast<key_vtn_t *>
          (pkey->get_key())->vtn_name)))
    uuu::upll_strncpy(key_vterm->vtn_key.vtn_name, reinterpret_cast<key_vtn_t *>
        (pkey->get_key())->vtn_name, (kMaxLenVtnName+1));
  DELETE_IF_NOT_NULL(pkey);
  pkey = NULL;
  okey = new ConfigKeyVal(UNC_KT_VTERMINAL, IpctSt::kIpcStKeyVterminal,
      key_vterm, NULL);
  if (!okey) {
    FREE_IF_NOT_NULL(key_vterm);
    return UPLL_RC_ERR_GENERIC;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VterminalMoMgr::UpdateConfigStatus(ConfigKeyVal *vterm_key,
    unc_keytype_operation_t op,
    uint32_t driver_result,
    ConfigKeyVal *upd_key,
    DalDmlIntf *dmi,
    ConfigKeyVal *ctrlr_key) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  val_vterm *vterm_val;
  /**
   * Attribute successfully applied in controller
   * set cs_status as UNC_CS_APPLIED.
   */
  unc_keytype_configstatus_t cs_status =
    (driver_result == UPLL_RC_SUCCESS) ? UNC_CS_APPLIED
    : UNC_CS_NOT_APPLIED;
  vterm_val = reinterpret_cast<val_vterm *>(GetVal(vterm_key));
  val_vterm *vterm_val2 = reinterpret_cast<val_vterm *>(GetVal(upd_key));
  if (vterm_val == NULL) return UPLL_RC_ERR_GENERIC;
  UPLL_LOG_TRACE("Key in Candidate %s", (vterm_key->ToStrAll()).c_str());
  /**
   * operation is CREATE, set cs_row_status based on driver result.
   */
  if (op == UNC_OP_CREATE) {
    vterm_val->cs_row_status = cs_status;
    val_db_vterm_st *val_vtermst = reinterpret_cast<val_db_vterm_st *>
      (ConfigKeyVal::Malloc(sizeof(val_db_vterm_st)));
    if (driver_result == UPLL_RC_ERR_CTR_DISCONNECTED) {
      val_vtermst->vterm_val_st.oper_status = UPLL_OPER_STATUS_UNKNOWN;
    } else {
      val_vtermst->vterm_val_st.oper_status = UPLL_OPER_STATUS_UNINIT;
    }
    val_vtermst->down_count  = 0;
    val_vtermst->unknown_count  = 0;
    val_vtermst->vterm_val_st.valid[UPLL_IDX_OPER_STATUS_VTERMS] = UNC_VF_VALID;
    vterm_key->AppendCfgVal(IpctSt::kIpcStValVterminalSt, val_vtermst);
    /**
     * Operation is UPDATE, get cs_row_status from RUNNING and update.
     */
  } else if (op == UNC_OP_UPDATE) {
    void *vtermval = reinterpret_cast<void *>(vterm_val);
    CompareValidValue(vtermval, GetVal(upd_key), true);
    UPLL_LOG_TRACE("Key in Running %s", (upd_key->ToStrAll()).c_str());
    vterm_val->cs_row_status = vterm_val2->cs_row_status;
  } else {
    return UPLL_RC_ERR_GENERIC;
  }
  for (unsigned int loop = 0;
      loop < sizeof(vterm_val->valid) / sizeof(vterm_val->valid[0]); ++loop) {
    if ((UNC_VF_VALID == vterm_val->valid[loop])
        || (UNC_VF_VALID_NO_VALUE == vterm_val->valid[loop])) {
      /**
       * Description is set to APPLIED
       */
      if (loop == UPLL_IDX_DESC_VTERM)
        vterm_val->cs_attr[loop] = UNC_CS_APPLIED;
      else
        vterm_val->cs_attr[loop] = cs_status;
    } else if ((UNC_VF_INVALID == vterm_val->valid[loop]) &&
        (UNC_OP_CREATE == op)) {
      vterm_val->cs_attr[loop] = UNC_CS_APPLIED;
    } else if ((UNC_VF_INVALID == vterm_val->valid[loop]) &&
        (UNC_OP_UPDATE == op)) {
      vterm_val->cs_attr[loop] = vterm_val2->cs_attr[loop];
    }
  }
  return result_code;
}
upll_rc_t VterminalMoMgr::UpdateAuditConfigStatus(
    unc_keytype_configstatus_t cs_status,
    uuc::UpdateCtrlrPhase phase,
    ConfigKeyVal *&ckv_running,
    DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  val_vterm_t *val;
  val = (ckv_running != NULL) ? reinterpret_cast<val_vterm_t *>
    (GetVal(ckv_running)) : NULL;
  if (NULL == val) {
    return UPLL_RC_ERR_GENERIC;
  }
  if (uuc::kUpllUcpCreate == phase)
    val->cs_row_status = cs_status;
  if ((uuc::kUpllUcpUpdate == phase) &&
      (val->cs_row_status == UNC_CS_INVALID ||
       val->cs_row_status == UNC_CS_NOT_APPLIED))
    val->cs_row_status = cs_status;
  for (unsigned int loop = 0; loop < sizeof(val->valid) / sizeof(uint8_t);
      ++loop) {
    if ((cs_status == UNC_CS_INVALID && UNC_VF_VALID == val->valid[loop]) ||
        cs_status == UNC_CS_APPLIED)
      val->cs_attr[loop] = cs_status;
  }
  return result_code;
}
upll_rc_t VterminalMoMgr::ValidateCapability(IpcReqRespHeader *req,
    ConfigKeyVal *ikey,
    const char *ctrlr_name) {
  UPLL_FUNC_TRACE;
  upll_rc_t ret_val = UPLL_RC_ERR_GENERIC;
  if (!ikey || !req) {
    UPLL_LOG_DEBUG("ConfigKeyVal / IpcReqRespHeader is Null");
    return ret_val;
  }

  if (!ctrlr_name) {
    ctrlr_name = reinterpret_cast<char*>((reinterpret_cast<key_user_data_t *>
          (ikey->get_user_data()))->ctrlr_id);
    if (!ctrlr_name || !strlen(ctrlr_name)) {
      UPLL_LOG_DEBUG("Controller Name is NULL");
      return UPLL_RC_ERR_GENERIC;
    }
  }

  bool result_code = false;
  uint32_t max_attrs = 0;
  uint32_t max_instance_count = 0;
  const uint8_t *attrs = NULL;

  switch (req->operation) {
    case UNC_OP_CREATE:
      result_code = GetCreateCapability(ctrlr_name, ikey->get_key_type(),
          &max_instance_count, &max_attrs,
          &attrs);
      break;

    case UNC_OP_UPDATE:
      result_code = GetUpdateCapability(ctrlr_name, ikey->get_key_type(),
          &max_attrs, &attrs);
      break;
    case UNC_OP_READ:
    case UNC_OP_READ_SIBLING:
    case UNC_OP_READ_SIBLING_BEGIN:
    case UNC_OP_READ_SIBLING_COUNT:
      result_code = GetReadCapability(ctrlr_name, ikey->get_key_type(),
          &max_attrs, &attrs);
      break;
    default:
      UPLL_LOG_DEBUG("Invalid Operation Code - (%d)", req->operation);
      return UPLL_RC_ERR_GENERIC;
  }
  if (!result_code) {
    UPLL_LOG_DEBUG("key_type - %d is not supported by controller - %s",
        ikey->get_key_type(), ctrlr_name);
    return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
  }

  val_vterm *vterm_val = NULL;
  if (ikey->get_cfg_val() && (ikey->get_cfg_val()->get_st_num() ==
        IpctSt::kIpcStValVterminal)) {
    vterm_val =
      reinterpret_cast<val_vterm *>(ikey->get_cfg_val()->get_val());
  }
  if (vterm_val) {
    if (max_attrs > 0) {
      /**
       * Checks if the specified key type(KT_VTERMINAL) and
       * associated attributes are supported on the given controller
       * based on the valid flag.
       */
      ret_val = ValVterminalAttributeSupportCheck(vterm_val, attrs,
          req->operation);
      return ret_val;
    } else {
      UPLL_LOG_DEBUG("Attribute list is empty for operation %d",
          req->operation);
      return UPLL_RC_ERR_GENERIC;
    }
  }
  return UPLL_RC_SUCCESS;
}
upll_rc_t VterminalMoMgr::ValVterminalAttributeSupportCheck(
    val_vterm_t *vterm_val,
    const uint8_t *attrs,
    unc_keytype_operation_t operation) {
  UPLL_FUNC_TRACE;
  if (vterm_val != NULL) {
    if ((vterm_val->valid[UPLL_IDX_DESC_VTERM] == UNC_VF_VALID)
        || (vterm_val->valid[UPLL_IDX_DESC_VTERM] == UNC_VF_VALID_NO_VALUE)) {
      /** Checks attributes are supported on the given controller,
       *  based on the valid flag.
       */
      if (attrs[unc::capa::vterminal::kCapDesc] == 0) {
        vterm_val->valid[UPLL_IDX_DESC_VTERM] = UNC_VF_INVALID;
        if (operation == UNC_OP_CREATE || operation == UNC_OP_UPDATE) {
          UPLL_LOG_DEBUG("Desc. Attribute is not supported by ctrlr");
          return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
        }
      }
    }
  } else {
    UPLL_LOG_DEBUG("Error val_vterm Struct is NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VterminalMoMgr::ValidateMessage(IpcReqRespHeader *req,
    ConfigKeyVal *ikey) {
  UPLL_FUNC_TRACE;
  upll_rc_t ret_val = UPLL_RC_SUCCESS;
  if (!req || !ikey || !(ikey->get_key())) {
    UPLL_LOG_DEBUG("ConfigKeyVal / IpcReqRespHeader is Null");
    return UPLL_RC_ERR_GENERIC;
  }
  if (ikey->get_st_num() != IpctSt::kIpcStKeyVterminal) {
    UPLL_LOG_DEBUG("Invalid key structure received. received struct - %d",
        (ikey->get_st_num()));
    return UPLL_RC_ERR_BAD_REQUEST;
  }
  if (UNC_KT_VTERMINAL != ikey->get_key_type()) {
    UPLL_LOG_DEBUG("Invalid keytype received. Received keytype - %d",
                   ikey->get_key_type());
    return UPLL_RC_ERR_BAD_REQUEST;
  }
  key_vterm *vterm_key = reinterpret_cast<key_vterm *> (ikey->get_key());
  upll_keytype_datatype_t dt_type = req->datatype;
  unc_keytype_operation_t operation = req->operation;
  unc_keytype_option1_t option1 = req->option1;
  unc_keytype_option2_t option2 = req->option2;
  /**
   * Validates the syntax of the specified key structure
   */
  ret_val = ValidateVterminalKey(vterm_key, operation);
  if (ret_val != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("syntax check failed for key_vterm struct");
    return UPLL_RC_ERR_CFG_SYNTAX;
  }

  if ((operation == UNC_OP_CREATE || operation == UNC_OP_UPDATE) &&
      ((dt_type == UPLL_DT_CANDIDATE) || (dt_type == UPLL_DT_IMPORT))) {
    ConfigVal *cfg_val = ikey->get_cfg_val();
    if (cfg_val == NULL) {
      UPLL_LOG_DEBUG("ConfigVal struct is empty");
      return UPLL_RC_ERR_BAD_REQUEST;
    }
    if (cfg_val->get_st_num() != IpctSt::kIpcStValVterminal) {
      UPLL_LOG_DEBUG(
          "Invalid val structure received.received struct - %d",
          cfg_val->get_st_num());
      return UPLL_RC_ERR_BAD_REQUEST;
    }
    val_vterm *vterm_val =
      reinterpret_cast<val_vterm *>(ikey->get_cfg_val()->get_val());
    if (vterm_val == NULL) {
      UPLL_LOG_DEBUG("val struct is mandatory for create and update op");
      return UPLL_RC_ERR_BAD_REQUEST;
    }
    /**
     * Validates the syntax of the specified value structure
     */
    ret_val = ValidateVterminalValue(vterm_val, operation);
    if (ret_val != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("syntax check failed for val_vterm structure");
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
    return UPLL_RC_SUCCESS;
  } else if ((operation == UNC_OP_RENAME) && (dt_type == UPLL_DT_IMPORT)) {
    if (option1 != UNC_OPT1_NORMAL) {
      UPLL_LOG_DEBUG("Error option1 is not NORMAL");
      return UPLL_RC_ERR_INVALID_OPTION1;
    }
    if (option2 != UNC_OPT2_NONE) {
      UPLL_LOG_DEBUG("Error option2 is not NONE");
      return UPLL_RC_ERR_INVALID_OPTION2;
    }
    ConfigVal *cfg_val = ikey->get_cfg_val();
    if (!cfg_val) return UPLL_RC_ERR_CFG_SYNTAX;
    if (cfg_val->get_st_num() != IpctSt::kIpcStValRenameVterminal) {
      UPLL_LOG_DEBUG(
          "Invalid val_rename structure received.received struct - %d",
          cfg_val->get_st_num());
      return UPLL_RC_ERR_BAD_REQUEST;
    }
    val_rename_vterm *vterm_rename =
      reinterpret_cast<val_rename_vterm *>(cfg_val->get_val());
    if (vterm_rename == NULL) {
      UPLL_LOG_DEBUG(
          "syntax check failed for rename_vterm struct");
      return UPLL_RC_ERR_BAD_REQUEST;
    }
    /**
     * Validates the syntax of the specified rename value structure
     */
    ret_val = ValidateVterminalRenameValue(vterm_rename);
    if (ret_val != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("syntax check failed for val_rename_vterm structure");
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
    return UPLL_RC_SUCCESS;
  } else if ((operation == UNC_OP_READ || operation == UNC_OP_READ_SIBLING
        || operation == UNC_OP_READ_SIBLING_BEGIN) &&
      (dt_type == UPLL_DT_IMPORT)) {
    if (option1 != UNC_OPT1_NORMAL) {
      UPLL_LOG_DEBUG("Error option1 is not NORMAL");
      return UPLL_RC_ERR_INVALID_OPTION1;
    }
    if (option2 != UNC_OPT2_NONE) {
      UPLL_LOG_DEBUG("Error option2 is not NONE");
      return UPLL_RC_ERR_INVALID_OPTION2;
    }
    ConfigVal *cfg_val = ikey->get_cfg_val();
    if (!cfg_val) return UPLL_RC_SUCCESS;
    if (cfg_val->get_st_num() != IpctSt::kIpcStValRenameVterminal) {
      UPLL_LOG_DEBUG(
          "Invalid val_rename structure received.received struct - %d",
          cfg_val->get_st_num());
      return UPLL_RC_ERR_BAD_REQUEST;
    }
    val_rename_vterm *vterm_rename =
      reinterpret_cast<val_rename_vterm *>(cfg_val->get_val());
    if (vterm_rename == NULL) {
      UPLL_LOG_DEBUG("syntax check for val_rename_vterm struct is optional");
      return UPLL_RC_SUCCESS;
    }
    /**
     * Validates the syntax of the specified rename value structure
     */
    ret_val = ValidateVterminalRenameValue(vterm_rename);
    if (ret_val != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("syntax check failed for val_rename_vterm structure");
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
    return UPLL_RC_SUCCESS;
  } else if ((operation == UNC_OP_READ || operation == UNC_OP_READ_SIBLING ||
              operation == UNC_OP_READ_SIBLING_BEGIN ||
              operation == UNC_OP_READ_SIBLING_COUNT) &&
      (dt_type == UPLL_DT_CANDIDATE || dt_type == UPLL_DT_RUNNING ||
       dt_type == UPLL_DT_STARTUP || dt_type == UPLL_DT_STATE)) {
    if (option1 != UNC_OPT1_NORMAL) {
      UPLL_LOG_DEBUG("Error option1 is not NORMAL");
      return UPLL_RC_ERR_INVALID_OPTION1;
    }
    if (option2 != UNC_OPT2_NONE) {
      UPLL_LOG_DEBUG("Error option2 is not NONE");
      return UPLL_RC_ERR_INVALID_OPTION2;
    }
    ConfigVal *cfg_val = ikey->get_cfg_val();
    if (!cfg_val) return UPLL_RC_SUCCESS;
    if (cfg_val->get_st_num() != IpctSt::kIpcStValVterminal) {
      UPLL_LOG_DEBUG("value structure matching is invalid. st.num - %d",
          cfg_val->get_st_num());
      return UPLL_RC_ERR_BAD_REQUEST;
    }
    val_vterm *vterm_val =
      reinterpret_cast<val_vterm *>(ikey->get_cfg_val()->get_val());
    if (vterm_val == NULL) {
      UPLL_LOG_DEBUG("syntax check for val vterm struct is an optional");
      return UPLL_RC_SUCCESS;
    }
    ret_val = ValidateVterminalValue(vterm_val, operation);
    if (ret_val != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("syntax check failure for val vterm structure");
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  } else if ((operation == UNC_OP_DELETE) && (dt_type == UPLL_DT_CANDIDATE)) {
    UPLL_LOG_TRACE("Value structure is none for operation type:%d",
        operation);
    return UPLL_RC_SUCCESS;
  } else {
    UPLL_LOG_DEBUG("Error Unsupported Datatype-(%d) or Operation-(%d)", dt_type,
        operation);
  }
  return UPLL_RC_SUCCESS;
}
upll_rc_t VterminalMoMgr::ValidateVterminalKey(key_vterm *vterm_key,
    unc_keytype_operation_t operation) {
  UPLL_FUNC_TRACE;
  upll_rc_t ret_val = UPLL_RC_SUCCESS;
  /**
   * validating vterm_key->vtn name
   */
  ret_val = ValidateKey(reinterpret_cast<char *>(vterm_key->vtn_key.vtn_name),
      kMinLenVtnName,
      kMaxLenVtnName);
  if (ret_val != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Syntax check failed.Received vtn_name - %s",
        vterm_key->vtn_key.vtn_name);
    return UPLL_RC_ERR_CFG_SYNTAX;
  }
  /**
   * Validates only parent name for operation UNC_OP_READ_SIBLING_COUNT
   * UNC_OP_READ_SIBLING_BEGIN operation.
   */
  if (operation != UNC_OP_READ_SIBLING_COUNT &&
      operation != UNC_OP_READ_SIBLING_BEGIN) {
    ret_val = ValidateKey(reinterpret_cast<char *> (vterm_key->vterminal_name),
        kMinLenVnodeName,
        kMaxLenVnodeName);
    if (ret_val != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Syntax check failed. Received vterminal_name - %s",
          vterm_key->vterminal_name);
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  } else {
    UPLL_LOG_TRACE("Operation is %d", operation);
    StringReset(vterm_key->vterminal_name);
  }
  return UPLL_RC_SUCCESS;
}
upll_rc_t VterminalMoMgr::ValidateVterminalValue(val_vterm *vterm_val,
    uint32_t operation) {
  UPLL_FUNC_TRACE;
  bool ret_val = false;

  /**
   * Attribute syntax validation
   */
  for (unsigned int valid_index = 0;
      valid_index < sizeof(vterm_val->valid) / sizeof(vterm_val->valid[0]);
      valid_index++) {
    if (vterm_val->valid[valid_index] == UNC_VF_VALID) {
      switch (valid_index) {
        case UPLL_IDX_CONTROLLER_ID_VTERM:
          ret_val = ValidateString(vterm_val->controller_id,
              kMinLenCtrlrId, kMaxLenCtrlrId);
          break;
        case UPLL_IDX_DOMAIN_ID_VTERM:
          ret_val = ValidateDefaultStr(vterm_val->domain_id,
              kMinLenDomainId, kMaxLenDomainId);
          break;
        case UPLL_IDX_DESC_VTERM:
          ret_val = ValidateDesc(vterm_val->vterm_description,
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
      valid_index < sizeof(vterm_val->valid) / sizeof(vterm_val->valid[0]);
      valid_index++) {
    uint8_t flag = vterm_val->valid[valid_index];
    switch (operation) {
      case UNC_OP_CREATE:
        {
          switch (valid_index) {
            case UPLL_IDX_CONTROLLER_ID_VTERM:
            case UPLL_IDX_DOMAIN_ID_VTERM:
              if ((flag == UNC_VF_INVALID || flag == UNC_VF_VALID_NO_VALUE)) {
                UPLL_LOG_DEBUG("controller_id or domain_id flag is invalid"
                    " or valid_no_value");
                return UPLL_RC_ERR_CFG_SYNTAX;
              }
              break;
            case UPLL_IDX_DESC_VTERM:
              break;
            default:
              break;
          }
        }
        break;
      case UNC_OP_UPDATE:
        {
          switch (valid_index) {
            case UPLL_IDX_CONTROLLER_ID_VTERM:
            case UPLL_IDX_DOMAIN_ID_VTERM:
              if (flag == UNC_VF_VALID_NO_VALUE) {
                UPLL_LOG_DEBUG("ctlr_id or domain_id flag is valid_no_value");
                return UPLL_RC_ERR_CFG_SYNTAX;
              }
              break;
            case UPLL_IDX_DESC_VTERM:
            default:
              break;
          }
        }
        break;
    }
  }

  // Resets
  for (unsigned int valid_index = 0;
      valid_index < sizeof(vterm_val->valid) / sizeof(vterm_val->valid[0]);
      valid_index++) {
    uint8_t flag = vterm_val->valid[valid_index];
    if (flag != UNC_VF_INVALID && flag != UNC_VF_VALID_NO_VALUE)
      continue;

    switch (valid_index) {
      case UPLL_IDX_DESC_VTERM:
        StringReset(vterm_val->vterm_description);
        break;
      default:
        UPLL_LOG_TRACE("Never here");
        break;
    }
  }
  return UPLL_RC_SUCCESS;
}
upll_rc_t VterminalMoMgr::ValidateVterminalRenameValue
(val_rename_vterm *vterm_rename) {
  UPLL_FUNC_TRACE;
  upll_rc_t ret_val = UPLL_RC_SUCCESS;
  /**
   * validating renamed name
   */
  if (vterm_rename->valid[UPLL_IDX_NEW_NAME_RVTERM] == UNC_VF_VALID) {
    ret_val = ValidateKey(reinterpret_cast<char *>(vterm_rename->new_name),
        kMinLenVnodeName,
        kMaxLenVnodeName);
    if (ret_val != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Syntax check failed."
          "Received new_name - %s",
          vterm_rename->new_name);
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VterminalMoMgr::CtrlrIdAndDomainIdUpdationCheck(ConfigKeyVal *ikey,
    ConfigKeyVal *okey) {
  UPLL_FUNC_TRACE;
  val_vterm *vterm_val = reinterpret_cast<val_vterm *>(GetVal(ikey));
  val_vterm *vterm_val1 = reinterpret_cast<val_vterm *>(GetVal(okey));
  /**
   * controller_id is valid, to compares controller id and domain id.
   */
  if (vterm_val->valid[UPLL_IDX_CONTROLLER_ID_VTERM] == UNC_VF_VALID) {
    if (strncmp(reinterpret_cast<const char *>(vterm_val->controller_id),
          reinterpret_cast<const char *>(vterm_val1->controller_id),
          kMaxLenCtrlrId+1)) {
      UPLL_LOG_DEBUG("controller id comparison failed");
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  }
  if (vterm_val->valid[UPLL_IDX_DOMAIN_ID_VTERM] == UNC_VF_VALID) {
    if (strncmp(reinterpret_cast<const char *>(vterm_val->domain_id),
          reinterpret_cast<const char *>(vterm_val1->domain_id),
          kMaxLenDomainId+1)) {
      UPLL_LOG_DEBUG("domain id comparison failed");
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  }
  return UPLL_RC_SUCCESS;
}
upll_rc_t VterminalMoMgr::DupConfigKeyVal(ConfigKeyVal *&okey,
    ConfigKeyVal *&req,
    MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  if (req == NULL) {
    UPLL_LOG_DEBUG("Input ConfigKeyVal req is NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  if (okey != NULL) {
    UPLL_LOG_DEBUG("Output ConfigKeyVal okey is not NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  if (req->get_key_type() != UNC_KT_VTERMINAL) {
    UPLL_LOG_DEBUG("Input ConfigKeyVal keytype mismatch");
    return UPLL_RC_ERR_GENERIC;
  }
  ConfigVal *tmp1 = NULL, *tmp = (req)->get_cfg_val();
  /**
   * Duplicates the input configkeyval including the key and val
   * based on the tbl specified.
   */
  if (tbl == MAINTBL || tbl == RENAMETBL) {
    if (tmp) {
      if (tbl == MAINTBL) {
        val_vterm *ival = reinterpret_cast<val_vterm *>(GetVal(req));
        if (ival == NULL) {
          UPLL_LOG_DEBUG("Null Val structure");
          return UPLL_RC_ERR_GENERIC;
        }
        val_vterm *vterm_val = reinterpret_cast<val_vterm *>
          (ConfigKeyVal::Malloc(sizeof(val_vterm)));
        memcpy(vterm_val, ival, sizeof(val_vterm));
        tmp1 = new ConfigVal(IpctSt::kIpcStValVterminal, vterm_val);
      } else if (tbl == RENAMETBL) {
        void *rename_val;
        ConfigVal *ck_v = req->get_cfg_val();
        if (ck_v != NULL && ck_v->get_st_num() == IpctSt::kIpcInvalidStNum) {
          val_rename_vnode *ival = reinterpret_cast<val_rename_vnode *>
            (GetVal(req));
          if (ival == NULL) {
            UPLL_LOG_DEBUG("Null Val structure");
            return UPLL_RC_ERR_GENERIC;
          }
          rename_val = reinterpret_cast<void *>
            (ConfigKeyVal::Malloc(sizeof(val_rename_vnode)));
          memcpy(rename_val, ival, sizeof(val_rename_vnode));
        } else {
          val_rename_vterm *ival = reinterpret_cast<val_rename_vterm *>
            (GetVal(req));
          if (ival == NULL) {
            UPLL_LOG_DEBUG("Null Val structure");
            return UPLL_RC_ERR_GENERIC;
          }
          rename_val = reinterpret_cast<void *>
            (ConfigKeyVal::Malloc(sizeof(val_rename_vterm)));
          memcpy(rename_val, ival, sizeof(val_rename_vterm));
        }
        tmp1 = new ConfigVal(IpctSt::kIpcStValRenameVterminal, rename_val);
      }
      tmp = tmp->get_next_cfg_val();
    };
  }
  if (tmp) {
    if (tbl == MAINTBL) {
      val_db_vterm_st *ival = reinterpret_cast<val_db_vterm_st *>
        (tmp->get_val());
      if (ival == NULL) {
        UPLL_LOG_DEBUG("Null Val structure");
        DELETE_IF_NOT_NULL(tmp1);
        return UPLL_RC_ERR_GENERIC;
      }
      val_db_vterm_st *val_vterm = reinterpret_cast<val_db_vterm_st *>
        (ConfigKeyVal::Malloc(sizeof(val_db_vterm_st)));
      memcpy(val_vterm, ival, sizeof(val_db_vterm_st));
      ConfigVal *tmp2 = new ConfigVal(IpctSt::kIpcStValVterminalSt, val_vterm);
      if (tmp1)
        tmp1->AppendCfgVal(tmp2);
    }
  };
  void *tkey = (req)->get_key();
  if (!tkey) {
    UPLL_LOG_DEBUG("Null tkey");
    DELETE_IF_NOT_NULL(tmp1);
    return UPLL_RC_ERR_GENERIC;
  }
  key_vterm *ikey = reinterpret_cast<key_vterm *>(tkey);
  key_vterm *vterm_key = reinterpret_cast<key_vterm *>
    (ConfigKeyVal::Malloc(sizeof(key_vterm)));
  memcpy(vterm_key, ikey, sizeof(key_vterm));
  okey = new ConfigKeyVal(UNC_KT_VTERMINAL, IpctSt::kIpcStKeyVterminal,
      vterm_key, tmp1);
  if (!okey) {
    DELETE_IF_NOT_NULL(tmp1);
    FREE_IF_NOT_NULL(vterm_key);
    return UPLL_RC_ERR_GENERIC;
  }
  SET_USER_DATA(okey, req);
  return UPLL_RC_SUCCESS;
}
upll_rc_t VterminalMoMgr::AllocVal(ConfigVal *&ck_val,
    upll_keytype_datatype_t dt_type,
    MoMgrTables tbl) {
  void *val;

  if (ck_val != NULL) return UPLL_RC_ERR_GENERIC;
  /**
   * Allocates for the specified val in the given configuration in the
   * specified table.
   */
  switch (tbl) {
    case MAINTBL:
      val = reinterpret_cast<void *>(ConfigKeyVal::Malloc(sizeof(val_vterm)));
      ck_val = new ConfigVal(IpctSt::kIpcStValVterminal, val);
      if (dt_type == UPLL_DT_STATE) {
        val = reinterpret_cast<void *>
          (ConfigKeyVal::Malloc(sizeof(val_db_vterm_st)));
        ConfigVal *ck_nxtval = new ConfigVal(IpctSt::kIpcStValVterminalSt, val);
        ck_val->AppendCfgVal(ck_nxtval);
      }
      break;
    case RENAMETBL:
      val = reinterpret_cast<void *>
        (ConfigKeyVal::Malloc(sizeof(val_rename_vnode)));
      ck_val = new ConfigVal(IpctSt::kIpcInvalidStNum, val);
      break;
    default:
      val = NULL;
  }
  return UPLL_RC_SUCCESS;
}
upll_rc_t VterminalMoMgr::GetParentConfigKey(ConfigKeyVal *&okey,
    ConfigKeyVal *ikey) {
  UPLL_FUNC_TRACE;

  if (ikey == NULL) {
    UPLL_LOG_DEBUG("Null ikey param");
    return UPLL_RC_ERR_GENERIC;
  }
  DELETE_IF_NOT_NULL(okey);
  unc_key_type_t ikey_type = ikey->get_key_type();
  if (ikey_type != UNC_KT_VTERMINAL)
    return UPLL_RC_ERR_GENERIC;
  void *pkey = ikey->get_key();
  if (!pkey)
    return UPLL_RC_ERR_GENERIC;
  key_vtn *vtn_key = reinterpret_cast<key_vtn *>
    (ConfigKeyVal::Malloc(sizeof(key_vtn)));
  uuu::upll_strncpy(vtn_key->vtn_name,
      reinterpret_cast<key_vterm *>(pkey)->vtn_key.vtn_name,
      (kMaxLenVtnName+1));
  /**
   * Get a configkeyval of the parent keytype
   */
  okey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, vtn_key, NULL);
  if (okey == NULL) {
    FREE_IF_NOT_NULL(vtn_key);
    return UPLL_RC_ERR_GENERIC;
  } else {
    SET_USER_DATA(okey, ikey);
  }
  return UPLL_RC_SUCCESS;
}
upll_rc_t VterminalMoMgr::GetRenamedControllerKey(ConfigKeyVal *ikey,
    upll_keytype_datatype_t dt_type,
    DalDmlIntf *dmi,
    controller_domain *ctrlr_dom) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code;
  uint8_t rename = 0;
  ConfigKeyVal *okey = NULL;
  if (ikey == NULL || dmi == NULL) {
    UPLL_LOG_DEBUG("Null param ikey/dmi");
    return UPLL_RC_ERR_GENERIC;
  }
  /**
   * get renamed controller key from unc key
   */
  result_code = IsRenamed(ikey, dt_type, dmi, rename);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Returning %d", result_code);
    return result_code;
  }

  if (!ctrlr_dom->ctrlr ||
      !strlen(reinterpret_cast<const char *>(ctrlr_dom->ctrlr)) ||
      !ctrlr_dom->domain ||
      !strlen(reinterpret_cast<const char *>(ctrlr_dom->domain))) {
    GET_USER_DATA_CTRLR_DOMAIN(ikey, *ctrlr_dom);
  }
  if (rename == 0)
    return UPLL_RC_SUCCESS;
  result_code = GetChildConfigKey(okey, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Returning error %d", result_code);
    return result_code;
  }
  SET_USER_DATA_CTRLR_DOMAIN(okey, *ctrlr_dom);
  DbSubOp dbop = { kOpReadSingle, kOpMatchCtrlr | kOpMatchDomain,
    kOpInOutFlag };
  result_code = ReadConfigDB(okey, dt_type, UNC_OP_READ, dbop, dmi, RENAMETBL);
  if (result_code != UPLL_RC_SUCCESS &&
      result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    DELETE_IF_NOT_NULL(okey);
    return result_code;
  }

  key_vterm *ctrlr_key = reinterpret_cast<key_vterm *>(ikey->get_key());
  if (result_code == UPLL_RC_SUCCESS) {
    val_rename_vnode *rename_val = reinterpret_cast<val_rename_vnode *>
        (GetVal(okey));
    if (!rename_val) {
      UPLL_LOG_DEBUG("Val is Empty");
      DELETE_IF_NOT_NULL(okey);
      return UPLL_RC_ERR_GENERIC;
    }
    key_vterm *ctrlr_key = reinterpret_cast<key_vterm *>(ikey->get_key());
    if (!ctrlr_key) {
      UPLL_LOG_DEBUG("Key is Empty");
      DELETE_IF_NOT_NULL(okey);
      return UPLL_RC_ERR_GENERIC;
    }
    UPLL_LOG_TRACE("Rename flag %d", rename);
    if (rename & VTN_RENAME) { /* vtn renamed */
      uuu::upll_strncpy(ctrlr_key->vtn_key.vtn_name, rename_val->ctrlr_vtn_name,
                        (kMaxLenVtnName+1));
    }
    if (rename & VN_RENAME) { /* vnode renamed */
      uuu::upll_strncpy(ctrlr_key->vterminal_name, rename_val->ctrlr_vnode_name,
                        (kMaxLenVnodeName+1));
    }
  } else if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    MoMgrImpl *vtn_mgr =
        reinterpret_cast<MoMgrImpl *>(const_cast<MoManager*>
        (GetMoManager(UNC_KT_VTN)));
    if (!vtn_mgr) {
      UPLL_LOG_DEBUG("mgr is NULL");
      DELETE_IF_NOT_NULL(okey);
      return UPLL_RC_ERR_GENERIC;
    }
    DELETE_IF_NOT_NULL(okey);
    result_code = vtn_mgr->GetChildConfigKey(okey, NULL);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("GetChildConfigKey Failed with result_code %d",
                     result_code);
      DELETE_IF_NOT_NULL(okey);
      return result_code;
    }
    key_vtn *unc_key = reinterpret_cast<key_vtn *>(okey->get_key());
    uuu::upll_strncpy(unc_key->vtn_name, ctrlr_key->vtn_key.vtn_name,
                      (kMaxLenVtnName+1));
    SET_USER_DATA_CTRLR_DOMAIN(okey, *ctrlr_dom);
    result_code = vtn_mgr->GetRenamedControllerKey(okey, dt_type,
                                                   dmi, ctrlr_dom);
    if (result_code == UPLL_RC_SUCCESS) {
      if (strcmp(reinterpret_cast<char *>(ctrlr_key->vtn_key.vtn_name),
                 reinterpret_cast<char*>(unc_key->vtn_name))) {
        UPLL_LOG_DEBUG("Not Same Vtn Name");
        uuu::upll_strncpy(ctrlr_key->vtn_key.vtn_name, unc_key->vtn_name,
                          (kMaxLenVtnName+1));
      }
    }
  }

  SET_USER_DATA_FLAGS(ikey, rename);
  DELETE_IF_NOT_NULL(okey);
  return UPLL_RC_SUCCESS;
}
upll_rc_t VterminalMoMgr::GetRenamedUncKey(ConfigKeyVal *ikey,
    upll_keytype_datatype_t dt_type,
    DalDmlIntf *dmi,
    uint8_t *ctrlr_id) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *unc_key = NULL;
  DbSubOp dbop = { kOpReadSingle, kOpMatchCtrlr, kOpInOutNone };
  /**
   * Get renamed unc key from controller key
   */
  val_rename_vnode *rename_vnode = reinterpret_cast<val_rename_vnode *>(
      ConfigKeyVal::Malloc(sizeof(val_rename_vnode)));
  key_vterm *ctrlr_key = reinterpret_cast<key_vterm *>(ikey->get_key());
  upll_rc_t ret_val = ValidateKey(reinterpret_cast<char *>
      (ctrlr_key->vtn_key.vtn_name),
      kMinLenVtnName, kMaxLenVtnName);
  if (ret_val == UPLL_RC_SUCCESS)  {
    uuu::upll_strncpy(rename_vnode->ctrlr_vtn_name, ctrlr_key->vtn_key.vtn_name,
        (kMaxLenVtnName+1));
    rename_vnode->valid[UPLL_CTRLR_VTN_NAME_VALID] = UNC_VF_VALID;
  }
  ret_val = ValidateKey(reinterpret_cast<char *>(ctrlr_key->vterminal_name),
      kMinLenVnodeName, kMaxLenVnodeName);
  if (ret_val == UPLL_RC_SUCCESS)  {
    uuu::upll_strncpy(rename_vnode->ctrlr_vnode_name, ctrlr_key->vterminal_name,
        (kMaxLenVnodeName+1));
    rename_vnode->valid[UPLL_CTRLR_VNODE_NAME_VALID] = UNC_VF_VALID;
  }
  result_code = GetChildConfigKey(unc_key, NULL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetChildConfigKey Failed with result_code %d",
        result_code);
    FREE_IF_NOT_NULL(rename_vnode);
    return result_code;
  }
  if (ctrlr_id) {
    SET_USER_DATA_CTRLR(unc_key, ctrlr_id);
  } else {
    dbop.matchop = kOpMatchNone;
  }
  unc_key->AppendCfgVal(IpctSt::kIpcStValRenameVtn, rename_vnode);
  uint8_t rename = 0;
  UPLL_LOG_TRACE("Before Read from Rename Table %s",
      (unc_key->ToStrAll()).c_str());

  dbop.inoutop = kOpInOutCtrlr | kOpInOutDomain;
  result_code = ReadConfigDB(unc_key, dt_type, UNC_OP_READ, dbop, dmi,
      RENAMETBL);
  if (result_code == UPLL_RC_SUCCESS) {
    key_vterm *vterm_key = reinterpret_cast<key_vterm *>(unc_key->get_key());
    if (strcmp(reinterpret_cast<char *>(ctrlr_key->vtn_key.vtn_name),
          reinterpret_cast<char*>(vterm_key->vtn_key.vtn_name))) {
      UPLL_LOG_DEBUG("Not Same Vtn Name");
      uuu::upll_strncpy(ctrlr_key->vtn_key.vtn_name,
          vterm_key->vtn_key.vtn_name,
          (kMaxLenVtnName+1));
      rename |= VTN_RENAME;
    }
    if (strcmp(reinterpret_cast<char *>(ctrlr_key->vterminal_name),
          reinterpret_cast<const char *>(vterm_key->vterminal_name))) {
      UPLL_LOG_DEBUG("Not same vterminal Name");
      uuu::upll_strncpy(ctrlr_key->vterminal_name, vterm_key->vterminal_name,
          (kMaxLenVnodeName+1));
      rename |= VN_RENAME;
    }
  SET_USER_DATA(ikey, unc_key);
  } else if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    upll_rc_t res_code = UPLL_RC_SUCCESS;
    MoMgrImpl *vtn_mgr =
        reinterpret_cast<MoMgrImpl *>(const_cast<MoManager*>
        (GetMoManager(UNC_KT_VTN)));
    if (!vtn_mgr) {
      UPLL_LOG_DEBUG("mgr is NULL");
      DELETE_IF_NOT_NULL(unc_key);
      return UPLL_RC_ERR_GENERIC;
    }
    DELETE_IF_NOT_NULL(unc_key);
    res_code = vtn_mgr->GetChildConfigKey(unc_key, NULL);
    if (res_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("GetChildConfigKey Failed with result_code %d",
                     res_code);
      return res_code;
    }
    SET_USER_DATA_CTRLR(unc_key, ctrlr_id);
    key_vtn *vtn_key = reinterpret_cast<key_vtn *>(unc_key->get_key());
    uuu::upll_strncpy(vtn_key->vtn_name, ctrlr_key->vtn_key.vtn_name,
                      (kMaxLenVtnName+1));
    res_code = vtn_mgr->GetRenamedUncKey(unc_key, dt_type,
                                            dmi, ctrlr_id);
    if (res_code == UPLL_RC_SUCCESS) {
      if (strcmp(reinterpret_cast<char *>(ctrlr_key->vtn_key.vtn_name),
                 reinterpret_cast<char*>(vtn_key->vtn_name))) {
        UPLL_LOG_DEBUG("Not Same Vtn Name");
        uuu::upll_strncpy(ctrlr_key->vtn_key.vtn_name, vtn_key->vtn_name,
                          (kMaxLenVtnName+1));
        rename |= VTN_RENAME;
      }
    }
  SET_USER_DATA_CTRLR(ikey, ctrlr_id);
  }
  SET_USER_DATA_FLAGS(ikey, rename);
  DELETE_IF_NOT_NULL(unc_key);
  return result_code;
}
upll_rc_t VterminalMoMgr::GetVnodeName(ConfigKeyVal *ikey,
    uint8_t *&vtn_name,
    uint8_t *&vnode_name) {
  UPLL_FUNC_TRACE;
  key_vterm_t *vterm_key = reinterpret_cast<key_vterm_t *>(ikey->get_key());
  if (vterm_key == NULL) return UPLL_RC_ERR_GENERIC;
  /**
   * Get the vtn_name and vterminal_name from ConfigKeyVal
   */
  vtn_name = vterm_key->vtn_key.vtn_name;
  vnode_name = vterm_key->vterminal_name;
  return UPLL_RC_SUCCESS;
}

upll_rc_t VterminalMoMgr::GetRenameInfo(ConfigKeyVal *ikey,
    ConfigKeyVal *okey,
    ConfigKeyVal *&rename_info,
    DalDmlIntf *dmi,
    const char *ctrlr_id,
    bool &renamed) {
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  if (!okey || !ikey || (rename_info != NULL))
    return UPLL_RC_ERR_GENERIC;

  key_vterm_t *vterm_key = NULL;
  key_rename_vnode_info *vterm_rename_info =
    reinterpret_cast<key_rename_vnode_info *>
    (ConfigKeyVal::Malloc(sizeof(key_rename_vnode_info)));

  vterm_key = reinterpret_cast<key_vterm_t *>(ikey->get_key());
  if (!vterm_key) {
    FREE_IF_NOT_NULL(vterm_rename_info);
    return UPLL_RC_ERR_GENERIC;
  }
  if (renamed) {
    if (!reinterpret_cast<val_rename_vnode_t *>(GetVal(ikey)) ||
        !(strlen(reinterpret_cast<char *>
            (reinterpret_cast<val_rename_vnode_t *>
             (GetVal(ikey))->ctrlr_vnode_name)))) {
      FREE_IF_NOT_NULL(vterm_rename_info);
      return UPLL_RC_ERR_GENERIC;
    }
    uuu::upll_strncpy(vterm_rename_info->ctrlr_vnode_name,
        reinterpret_cast<val_rename_vnode_t *>(GetVal(ikey))->ctrlr_vnode_name,
        (kMaxLenVnodeName+1));
    if (!reinterpret_cast<val_rename_vnode_t *>(GetVal(ikey)) ||
        !(strlen(reinterpret_cast<char *>
            (reinterpret_cast<val_rename_vnode_t *>
             (GetVal(ikey))->ctrlr_vtn_name)))) {
      FREE_IF_NOT_NULL(vterm_rename_info);
      return UPLL_RC_ERR_GENERIC;
    }
    uuu::upll_strncpy(vterm_rename_info->ctrlr_vtn_name,
        reinterpret_cast<val_rename_vnode_t *>(GetVal(ikey))->ctrlr_vtn_name,
        (kMaxLenVtnName+1));
  } else {
    if (strlen(reinterpret_cast<char *>(vterm_key->vterminal_name)) == 0) {
      FREE_IF_NOT_NULL(vterm_rename_info);
      return UPLL_RC_ERR_GENERIC;
    }
    uuu::upll_strncpy(vterm_rename_info->ctrlr_vnode_name,
        vterm_key->vterminal_name,
        (kMaxLenVnodeName+1));
    if (strlen(reinterpret_cast<char *>(vterm_key->vtn_key.vtn_name)) == 0) {
      FREE_IF_NOT_NULL(vterm_rename_info);
      return UPLL_RC_ERR_GENERIC;
    }
    uuu::upll_strncpy(vterm_rename_info->ctrlr_vtn_name,
        vterm_key->vtn_key.vtn_name, (kMaxLenVtnName+1));
  }

  if (strlen(reinterpret_cast<char *>(vterm_key->vterminal_name)) == 0) {
    FREE_IF_NOT_NULL(vterm_rename_info);
    return UPLL_RC_ERR_GENERIC;
  }
  uuu::upll_strncpy(vterm_rename_info->old_unc_vnode_name,
      vterm_key->vterminal_name,
      (kMaxLenVnodeName+1));
  if (strlen(reinterpret_cast<char *>(vterm_key->vtn_key.vtn_name)) == 0) {
    FREE_IF_NOT_NULL(vterm_rename_info);
    return UPLL_RC_ERR_GENERIC;
  }
  uuu::upll_strncpy(vterm_rename_info->new_unc_vtn_name,
      vterm_key->vtn_key.vtn_name, (kMaxLenVtnName+1));
  uuu::upll_strncpy(vterm_rename_info->old_unc_vtn_name,
      vterm_key->vtn_key.vtn_name, (kMaxLenVtnName+1));

  vterm_key = reinterpret_cast<key_vterm_t *>(okey->get_key());
  if (!vterm_key) {
    FREE_IF_NOT_NULL(vterm_rename_info);
    return UPLL_RC_ERR_GENERIC;
  }
  if (strlen(reinterpret_cast<char *>(vterm_key->vtn_key.vtn_name)) == 0) {
    FREE_IF_NOT_NULL(vterm_rename_info);
    return UPLL_RC_ERR_GENERIC;
  }
  uuu::upll_strncpy(vterm_rename_info->new_unc_vnode_name,
      vterm_key->vterminal_name, (kMaxLenVnodeName+1));
  /**
   * collects the Unc new name, Unc old name and Ctrlr name
   * informations and creates the configkeyval.
   */
  rename_info = new ConfigKeyVal(UNC_KT_VTERMINAL, IpctSt::kIpcInvalidStNum,
      vterm_rename_info, NULL);
  if (!rename_info) {
    FREE_IF_NOT_NULL(vterm_rename_info);
    return UPLL_RC_ERR_GENERIC;
  }
  UPLL_LOG_TRACE("Renamed bool value is %d", renamed);
  if (!renamed) {
    val_rename_vnode_t *vnode = reinterpret_cast<val_rename_vnode_t*>
      (ConfigKeyVal::Malloc(sizeof(val_rename_vnode_t)));
    ConfigKeyVal *tmp_key = NULL;
    result_code = GetChildConfigKey(tmp_key, ikey);
    if (UPLL_RC_SUCCESS != result_code || tmp_key == NULL) {
      FREE_IF_NOT_NULL(vnode);
      DELETE_IF_NOT_NULL(rename_info);
      return result_code;
    }
    DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutCtrlr|kOpInOutDomain};
    result_code = ReadConfigDB(tmp_key, UPLL_DT_IMPORT, UNC_OP_READ, dbop, dmi,
        MAINTBL);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_INFO("ReadConfigDB Failed %d\n", result_code);
      FREE_IF_NOT_NULL(vnode);
      DELETE_IF_NOT_NULL(tmp_key);
      DELETE_IF_NOT_NULL(rename_info);
      return result_code;
    }
    controller_domain ctrlr_dom;
    result_code = GetControllerDomainId(tmp_key, &ctrlr_dom);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_INFO("Returning error %d\n", result_code);
      DELETE_IF_NOT_NULL(tmp_key);
      DELETE_IF_NOT_NULL(rename_info);
      FREE_IF_NOT_NULL(vnode);
      return result_code;
    }
    SET_USER_DATA_CTRLR_DOMAIN(okey, ctrlr_dom);
    uuu::upll_strncpy(vnode->ctrlr_vtn_name, reinterpret_cast<key_vterm_t *>
        (ikey->get_key())->vtn_key.vtn_name, (kMaxLenVtnName+1));
    uuu::upll_strncpy(vnode->ctrlr_vnode_name, reinterpret_cast<key_vterm_t *>
        (ikey->get_key())->vterminal_name, (kMaxLenVnodeName+1));
    ConfigVal *rename_val_ = new ConfigVal(IpctSt::kIpcInvalidStNum, vnode);
    okey->SetCfgVal(rename_val_);
    vnode->valid[UPLL_CTRLR_VTN_NAME_VALID] = UNC_VF_VALID;
    vnode->valid[UPLL_CTRLR_VNODE_NAME_VALID] = UNC_VF_VALID;
    UPLL_LOG_TRACE("Before Create entry in rename table %s",
        (okey->ToStrAll()).c_str());
    dbop.readop = kOpNotRead;
    string temp_vtn_name = "";
    result_code = UpdateConfigDB(okey, UPLL_DT_IMPORT, UNC_OP_CREATE, dmi,
        &dbop, TC_CONFIG_GLOBAL, temp_vtn_name, RENAMETBL);
    DELETE_IF_NOT_NULL(tmp_key);
    if (result_code != UPLL_RC_SUCCESS)
      DELETE_IF_NOT_NULL(rename_info);
  }
  return result_code;
}

bool VterminalMoMgr::GetRenameKeyBindInfo(unc_key_type_t key_type,
    BindInfo *&binfo,
    int &nattr, MoMgrTables tbl) {
  /* Main Table or rename table only update */
  if (MAINTBL == tbl) {
    nattr = NUM_KEY_MAIN_TBL_;
    binfo = key_vterminal_maintbl_bind_info;
  } else if (RENAMETBL == tbl) {
    nattr = NUM_KEY_RENAME_TBL_;
    binfo = key_vterminal_renametbl_update_bind_info;
  } else {
    UPLL_LOG_TRACE("Invalid table");
    return false;
  }
  return true;
}

upll_rc_t VterminalMoMgr::CopyToConfigKey(ConfigKeyVal *&okey,
    ConfigKeyVal *ikey) {
  if (!ikey || !(ikey->get_key())) return UPLL_RC_ERR_GENERIC;

  upll_rc_t result_code = UPLL_RC_SUCCESS;
  /**
   * create configkey for the specified key type
   */
  key_rename_vnode_info *key_rename = reinterpret_cast<key_rename_vnode_info *>
    (ikey->get_key());
  key_vterm_t * key_vterm = reinterpret_cast<key_vterm_t *>
    (ConfigKeyVal::Malloc(sizeof(key_vterm_t)));
  if (!strlen(reinterpret_cast<char *>(key_rename->old_unc_vtn_name))) {
    FREE_IF_NOT_NULL(key_vterm);
    return UPLL_RC_ERR_GENERIC;
  }
  /**
   * Copy the old name from the rename_info into okey
   */
  uuu::upll_strncpy(key_vterm->vtn_key.vtn_name, key_rename->old_unc_vtn_name,
      (kMaxLenVtnName+1));
  if (ikey->get_key_type() == table[MAINTBL]->get_key_type()) {
    if (!strlen(reinterpret_cast<char *>(key_rename->old_unc_vnode_name))) {
      FREE_IF_NOT_NULL(key_vterm);
      return UPLL_RC_ERR_GENERIC;
    }
    uuu::upll_strncpy(key_vterm->vterminal_name, key_rename->old_unc_vnode_name,
        (kMaxLenVnodeName+1));
  }
  okey = new ConfigKeyVal(UNC_KT_VTERMINAL, IpctSt::kIpcStKeyVterminal,
      key_vterm, NULL);
  if (!okey) {
    FREE_IF_NOT_NULL(key_vterm);
    return UPLL_RC_ERR_GENERIC;
  }
  SET_USER_DATA(okey, ikey);
  return result_code;
}
upll_rc_t VterminalMoMgr::AdaptValToVtnService(ConfigKeyVal *ikey,
                                               AdaptType adapt_type) {
  UPLL_FUNC_TRACE;
  if (!ikey) {
    UPLL_LOG_DEBUG("Invalid ikey");
    return UPLL_RC_ERR_GENERIC;
  }
  key_vterm_t *vterm_key = reinterpret_cast<key_vterm_t*>(ikey->get_key());
  while (ikey) {
    ConfigVal *cval = ikey->get_cfg_val();
    if (!cval) {
      UPLL_LOG_DEBUG("Config Val is Null");
      return UPLL_RC_ERR_GENERIC;
    }
    while (cval) {
      if (IpctSt::kIpcStValVterminalSt == cval->get_st_num()) {
        controller_domain ctrlr_dom = {NULL, NULL};
        GET_USER_DATA_CTRLR_DOMAIN(ikey, ctrlr_dom);
        CheckOperStatus<val_vterm_st>(vterm_key->vtn_key.vtn_name,
                                      cval, UNC_KT_VTERMINAL, ctrlr_dom);
      }
      cval = cval->get_next_cfg_val();
    }
    if (adapt_type == ADAPT_ONE)
      break;
    ikey = ikey->get_next_cfg_key_val();
  }
  return UPLL_RC_SUCCESS;
}

VterminalMoMgr::VterminalMoMgr() {
  UPLL_FUNC_TRACE;
  ntable = MAX_MOMGR_TBLS;
  table = new Table *[ntable]();
  table[MAINTBL] = new Table(uudst::kDbiVterminalTbl, UNC_KT_VTERMINAL,
      vterminal_bind_info,
      IpctSt::kIpcStKeyVterminal, IpctSt::kIpcStValVterminal,
      (uudst::vterminal::kDbiVterminalNumCols+2));
  table[RENAMETBL] = new Table(uudst::kDbiVNodeRenameTbl, UNC_KT_VTERMINAL,
      vterminal_rename_bind_info, IpctSt::kIpcInvalidStNum,
      IpctSt::kIpcInvalidStNum,
      uudst::vnode_rename::kDbiVnodeRenameNumCols);
  table[CTRLRTBL] = NULL;
  table[CONVERTTBL] = NULL;

  nchild = sizeof(vterminal_child) / sizeof(*vterminal_child);
  child = vterminal_child;
}

bool VterminalMoMgr::IsValidKey(void *key,
    uint64_t index, MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  key_vterm *vterm_key = reinterpret_cast<key_vterm *>(key);
  upll_rc_t ret_val = UPLL_RC_SUCCESS;
  switch (index) {
    /**
     * check if individual portions of a key are valid
     */
    case uudst::vterminal::kDbiVtnName:
    case uudst::vnode_rename::kDbiUncVtnName:
      ret_val = ValidateKey(reinterpret_cast<char *>
          (vterm_key->vtn_key.vtn_name),
          kMinLenVtnName, kMaxLenVtnName);
      if (ret_val != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("VTN Name is not valid(%d)", ret_val);
        return PFC_FALSE;
      }
      break;
    case uudst::vterminal::kDbiVterminalName:
    case uudst::vnode_rename::kDbiUncvnodeName:
      ret_val = ValidateKey(reinterpret_cast<char *>(vterm_key->vterminal_name),
          kMinLenVnodeName,
          kMaxLenVnodeName);
      if (ret_val != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("VTERMINAL Name is not valid(%d)", ret_val);
        return PFC_FALSE;
      }
      break;
    default:
      UPLL_LOG_DEBUG("Invalid Key Index");
      return PFC_FALSE;
      break;
  }
  return PFC_TRUE;
}

upll_rc_t VterminalMoMgr::MergeValidate(unc_key_type_t keytype,
                                        const char *ctrlr_id,
                                        ConfigKeyVal *ikey,
                                        DalDmlIntf *dmi,
                                        upll_import_type import_type) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  DbSubOp dbop = { kOpReadMultiple, kOpMatchNone,
    kOpInOutCtrlr | kOpInOutDomain };
  ConfigKeyVal *tkey = NULL;

  if (!ikey || !ikey->get_key() || !(strlen(reinterpret_cast<const char *>
          (ctrlr_id)))) {
    UPLL_LOG_DEBUG("Input is NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  ConfigKeyVal *dup_key = NULL;
  result_code = GetChildConfigKey(dup_key, ikey);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetChildConfigKey Failed");
    DELETE_IF_NOT_NULL(dup_key);
    return result_code;
  }
  /* Getting FULL Key (VTN & VTERMINAL Name) */
  result_code = ReadConfigDB(dup_key, UPLL_DT_IMPORT, UNC_OP_READ, dbop, dmi,
      MAINTBL);
  if (UPLL_RC_SUCCESS != result_code) {
    DELETE_IF_NOT_NULL(dup_key);
    return result_code;
  }
  ConfigKeyVal *travel = dup_key;
  while (travel) {
    result_code = GetChildConfigKey(tkey, travel);

    if (UPLL_RC_SUCCESS != result_code || tkey == NULL) {
      UPLL_LOG_DEBUG("GetChildConfigKey is Failed");
      DELETE_IF_NOT_NULL(tkey);
      DELETE_IF_NOT_NULL(dup_key);
      return result_code;
    }
    /* Checks the give node is unique or not */
    if (import_type == UPLL_IMPORT_TYPE_FULL) {
      result_code = VnodeChecks(tkey, UPLL_DT_CANDIDATE, dmi, false);
      if (UPLL_RC_ERR_INSTANCE_EXISTS == result_code ||
          UPLL_RC_ERR_CFG_SEMANTIC == result_code) {
        ikey->ResetWith(tkey);
        DELETE_IF_NOT_NULL(tkey);
        DELETE_IF_NOT_NULL(dup_key);
        UPLL_LOG_DEBUG("VTerminal Name Conflict %s",
                       (ikey->ToStrAll()).c_str());
        return UPLL_RC_ERR_MERGE_CONFLICT;
      }
    } else {
      result_code = PartialImport_VnodeChecks(tkey,
          UPLL_DT_CANDIDATE, ctrlr_id , dmi);
      if (UPLL_RC_ERR_INSTANCE_EXISTS == result_code ||
          UPLL_RC_ERR_CFG_SEMANTIC == result_code) {
        ikey->ResetWith(tkey);
        DELETE_IF_NOT_NULL(tkey);
        DELETE_IF_NOT_NULL(dup_key);
        UPLL_LOG_DEBUG("VTerminal Name Conflict %s",
                       (ikey->ToStrAll()).c_str());
        return UPLL_RC_ERR_MERGE_CONFLICT;
      }
    }
    /* Any other DB error */
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("VnodeChecks Failed %d", result_code);
      DELETE_IF_NOT_NULL(tkey);
      DELETE_IF_NOT_NULL(dup_key);
      return result_code;
    }
    DELETE_IF_NOT_NULL(tkey);
    travel = travel->get_next_cfg_key_val();
  }
  DELETE_IF_NOT_NULL(dup_key);

  if (import_type == UPLL_IMPORT_TYPE_PARTIAL) {
    memset((ikey->get_key()), 0, sizeof(key_vtn));
    result_code = PartialMergeValidate(keytype, ctrlr_id, ikey, dmi);
  }
  return result_code;
}

upll_rc_t VterminalMoMgr::CreateVnodeConfigKey(ConfigKeyVal *ikey,
    ConfigKeyVal *&okey) {
  UPLL_FUNC_TRACE;
  if (ikey == NULL || ikey->get_key() == NULL) {
    UPLL_LOG_DEBUG("Null Input param ikey");
    return UPLL_RC_ERR_GENERIC;
  }
  key_vterm *temp_key_vterm = reinterpret_cast<key_vterm *>
    (ConfigKeyVal::Malloc(sizeof(key_vterm)));
  uuu::upll_strncpy(temp_key_vterm->vtn_key.vtn_name,
      reinterpret_cast<key_vterm*>(ikey->get_key())->vtn_key.vtn_name,
      (kMaxLenVtnName+1));
  uuu::upll_strncpy(temp_key_vterm->vterminal_name,
      reinterpret_cast<key_vterm*>(ikey->get_key())->vterminal_name,
      (kMaxLenVnodeName+1));

  okey = new ConfigKeyVal(UNC_KT_VTERMINAL, IpctSt::kIpcStKeyVterminal,
      temp_key_vterm, NULL);
  if (!okey) {
    FREE_IF_NOT_NULL(temp_key_vterm);
    return UPLL_RC_ERR_GENERIC;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VterminalMoMgr::GetChildConfigKey(ConfigKeyVal *&okey,
    ConfigKeyVal *parent_key) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  key_vterm *vterm_key = NULL;
  if (okey && (okey->get_key())) {
    vterm_key = reinterpret_cast<key_vterm_t *>
      (okey->get_key());
  } else {
    vterm_key = reinterpret_cast<key_vterm *>
      (ConfigKeyVal::Malloc(sizeof(key_vterm)));
  }
  void *pkey;
  if (parent_key == NULL) {
    if (!okey)
      okey = new ConfigKeyVal(UNC_KT_VTERMINAL, IpctSt::kIpcStKeyVterminal,
          vterm_key, NULL);
    else if (okey->get_key() != vterm_key)
      okey->SetKey(IpctSt::kIpcStKeyVterminal, vterm_key);
    return UPLL_RC_SUCCESS;
  } else {
    pkey = parent_key->get_key();
  }
  if (!pkey) {
    if (!okey || !(okey->get_key()))
      FREE_IF_NOT_NULL(vterm_key);
      DELETE_IF_NOT_NULL(okey);
    return UPLL_RC_ERR_GENERIC;
  }

  /* presumes MoMgrs receive only supported keytypes */
  switch (parent_key->get_key_type()) {
    case UNC_KT_VTERMINAL:
      uuu::upll_strncpy(vterm_key->vterminal_name, reinterpret_cast<key_vterm *>
          (pkey)->vterminal_name, (kMaxLenVnodeName+1));
      uuu::upll_strncpy(vterm_key->vtn_key.vtn_name,
          reinterpret_cast<key_vterm *>(pkey)->vtn_key.vtn_name,
          (kMaxLenVtnName+1));
      break;
    case UNC_KT_VTN:
    default:
      uuu::upll_strncpy(vterm_key->vtn_key.vtn_name,
          reinterpret_cast<key_vtn *>(pkey)->vtn_name, (kMaxLenVtnName+1));
      StringReset(vterm_key->vterminal_name);
  }
  if (!okey)
    /**
     * get a configkeyval of a specified keytype from an input configkeyval
     */
    okey = new ConfigKeyVal(UNC_KT_VTERMINAL, IpctSt::kIpcStKeyVterminal,
        vterm_key, NULL);
  else if (okey->get_key() != vterm_key)
    okey->SetKey(IpctSt::kIpcStKeyVterminal, vterm_key);
  if (okey == NULL) {
    FREE_IF_NOT_NULL(vterm_key);
    result_code = UPLL_RC_ERR_GENERIC;
  } else {
    SET_USER_DATA(okey, parent_key);
  }
  return result_code;
}

upll_rc_t VterminalMoMgr::GetControllerDomainId(ConfigKeyVal *ikey,
    controller_domain *ctrlr_dom) {
  UPLL_FUNC_TRACE;
  if (!ikey || !ctrlr_dom) {
    UPLL_LOG_DEBUG("Illegal parameter");
    return UPLL_RC_ERR_GENERIC;
  }
  val_vterm * temp_vterm = reinterpret_cast<val_vterm *> (GetVal(ikey));
  if (temp_vterm == NULL
      || temp_vterm->valid[UPLL_IDX_CONTROLLER_ID_VTERM] != UNC_VF_VALID
      || !strlen(reinterpret_cast<char*>(temp_vterm->controller_id))) {
    ctrlr_dom->ctrlr = NULL;
    UPLL_LOG_TRACE("Ctrlr_Name null");
  } else {
    SET_USER_DATA_CTRLR(ikey, temp_vterm->controller_id);
  }
  if (temp_vterm == NULL
      || temp_vterm->valid[UPLL_IDX_DOMAIN_ID_VTERM] != UNC_VF_VALID
      || !strlen(reinterpret_cast<char*>(
          temp_vterm->domain_id))) {
    ctrlr_dom->domain = NULL;
    UPLL_LOG_TRACE("Domain null");
  } else {
    /**
     *  Returns the controller and domain name from given ConfigKeyVal
     */
    SET_USER_DATA_DOMAIN(ikey, temp_vterm->domain_id);
    GET_USER_DATA_CTRLR_DOMAIN(ikey, *ctrlr_dom);
  }
  UPLL_LOG_TRACE("ctrlr_dom %s %s", ctrlr_dom->ctrlr, ctrlr_dom->domain);
  return UPLL_RC_SUCCESS;
}


/* U14 start */
/*
 * This function checks the vTermianl is part of vbr_if vexternal
 * in running configuration.
 * The vTermianl exists then reuturn merge conflict otherwise
 * database error or success
 */
upll_rc_t
VterminalMoMgr::PartialMergeValidate(unc_key_type_t keytype,
                                     const char *ctrlr_id,
                                     ConfigKeyVal *ikey,
                                     DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  int nattr = 0;
  BindInfo *binfo = NULL;

  const uudst::kDalTableIndex tbl_index = GetTable(MAINTBL, UPLL_DT_IMPORT);

  if (tbl_index >= uudst::kDalNumTables) {
    UPLL_LOG_DEBUG("Invalid Table Index - %d", tbl_index);
    return UPLL_RC_ERR_GENERIC;
  }
  DalBindInfo dal_bind_info(tbl_index);

  if (!GetBindInfo(MAINTBL, UPLL_DT_IMPORT, binfo, nattr)) {
    return UPLL_RC_ERR_GENERIC;
  }
  nattr = 2;
  ConfigKeyVal *vterm_ckv  = NULL;

  result_code = GetChildConfigKey(vterm_ckv, NULL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetChildConfigKey failed");
    return result_code;
  }

  void *tkey = vterm_ckv->get_key();
  for (int i = 0; i < nattr; i++) {
    uint64_t indx = binfo[i].index;
    void *p = reinterpret_cast<void *>(reinterpret_cast<char *>(tkey)
                                 + binfo[i].offset);
    dal_bind_info.BindOutput(indx, binfo[i].app_data_type,
                             binfo[i].array_size, p);
  }

  string query_string =
     "select ctrlr_vtn_name as vtn_name, ctrlr_vnode_name as vterminal_name "
     "from im_vnode_rename_tbl as tmp where exists(select 1 from ru_vbr_if_tbl "
     "where ru_vbr_if_tbl.vex_name = tmp.ctrlr_vnode_name) UNION "
     "select ctrlr_vtn_name, ctrlr_vnode_name from im_vnode_rename_tbl "
     "as tmp where exists(select 1 from ru_vbr_if_tbl where "
     "ru_vbr_if_tbl.vex_name = tmp.ctrlr_vnode_name); ";
  result_code = DalToUpllResCode(dmi->
                                 ExecuteAppQuerySingleRecord(query_string,
                                                             &dal_bind_info));
  if (UPLL_RC_SUCCESS != result_code &&
      UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
  UPLL_LOG_DEBUG("ReadConfigDB failed %d", result_code);
  delete vterm_ckv;
  return result_code;
  }
  if (UPLL_RC_SUCCESS == result_code) {
    UPLL_LOG_DEBUG("MergeConflicts with %s", vterm_ckv->ToStr().c_str());
    ikey->ResetWith(vterm_ckv);
    delete vterm_ckv;
    return UPLL_RC_ERR_MERGE_CONFLICT;
  }
  delete vterm_ckv;
  return UPLL_RC_SUCCESS;
}


/*U14 End */

}  // namespace kt_momgr
}  // namespace upll
}  // namespace unc

