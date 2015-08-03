/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include "vtep_momgr.hh"
#define NUM_KEY_MAIN_TBL_ 4
namespace unc {
namespace upll {
namespace kt_momgr {

namespace dbl = unc::upll::dal;
namespace tbl = unc::upll::dal::schema::table;


BindInfo VtepMoMgr::vtep_bind_info[] = {
  { uudst::vtep::kDbiVtnName, CFG_KEY, offsetof(key_vtep, vtn_key.vtn_name),
    uud::kDalChar, kMaxLenVtnName+1 },
  { uudst::vtep::kDbiVtepName, CFG_KEY, offsetof(key_vtep, vtep_name),
    uud::kDalChar, kMaxLenVnodeName+1 },
  { uudst::vtep::kDbiDesc, CFG_VAL, offsetof(val_vtep, description),
    uud::kDalChar, kMaxLenDescription+1},
  { uudst::vtep::kDbiCtrlrName, CFG_VAL, offsetof(val_vtep, controller_id),
    uud::kDalChar, kMaxLenCtrlrId+1},
  { uudst::vtep::kDbiCtrlrName, CK_VAL,
    offsetof(key_user_data_t, ctrlr_id),
    uud::kDalChar, kMaxLenCtrlrId+1 },
  { uudst::vtep::kDbiDomainId, CFG_VAL, offsetof(val_vtep, domain_id),
    uud::kDalChar, kMaxLenDomainId+1},
  { uudst::vtep::kDbiDomainId, CK_VAL,
    offsetof(key_user_data_t, domain_id),
    uud::kDalChar, kMaxLenDomainId+1 },
  { uudst::vtep::kDbiOperStatus, ST_VAL, offsetof(val_db_vtep_st,
    vtep_val_st.oper_status), uud::kDalUint8, 1},
  { uudst::vtep::kDbiDownCount, ST_VAL, offsetof(val_db_vtep_st, down_count),
    uud::kDalUint32, 1},
  { uudst::vtep::kDbiFlags,  CK_VAL, offsetof(key_user_data_t, flags),
    uud::kDalUint8, 1},
  { uudst::vtep::kDbiValidDesc, CFG_META_VAL,
    offsetof(val_vtep, valid[0]), uud::kDalUint8, 1},
  { uudst::vtep::kDbiValidCtrlrName, CFG_META_VAL,
    offsetof(val_vtep, valid[1]), uud::kDalUint8, 1},
  { uudst::vtep::kDbiValidDomainId, CFG_META_VAL,
    offsetof(val_vtep, valid[2]), uud::kDalUint8, 1},
  { uudst::vtep::kDbiValidOperStatus, ST_META_VAL,
    offsetof(val_db_vtep_st, vtep_val_st.valid[0]), uud::kDalUint8, 1},
  { uudst::vtep::kDbiCsRowstatus, CS_VAL, offsetof(val_vtep, cs_row_status),
    uud::kDalUint8, 1},
  { uudst::vtep::kDbiCsDesc, CS_VAL, offsetof(val_vtep, cs_attr[0]),
    uud::kDalUint8, 1},
  { uudst::vtep::kDbiCsCtrlrName, CS_VAL, offsetof(val_vtep, cs_attr[1]),
    uud::kDalUint8, 1},
  { uudst::vtep::kDbiCsDomainId, CS_VAL, offsetof(val_vtep,
    cs_attr[2]), uud::kDalUint8, 1}
};

unc_key_type_t VtepMoMgr::vtep_child[] = {
  UNC_KT_VTEP_IF
};

VtepMoMgr::VtepMoMgr() {
  UPLL_FUNC_TRACE;
  Table *tbl = new Table(uudst::kDbiVtepTbl, UNC_KT_VTEP, vtep_bind_info,
      IpctSt::kIpcStKeyVtep, IpctSt::kIpcStValVtep,
      uudst::vtep::kDbiVtepNumCols+2);
  ntable = MAX_MOMGR_TBLS;
  table = new Table *[ntable]();
  table[MAINTBL] = tbl;
  table[RENAMETBL] = NULL;
  table[CTRLRTBL] = NULL;
  table[CONVERTTBL] = NULL;

  nchild = sizeof(vtep_child) / sizeof(*vtep_child);
  child = vtep_child;
#ifdef _STANDALONE_
  SetMoManager(UNC_KT_VTEP, reinterpret_cast<MoMgr *>(this));
#endif
}

/*
 *  * Based on the key type the bind info will pass
 *
 bool VtepMoMgr::GetRenameKeyBindInfo(unc_key_type_t key_type, BindInfo *&binfo,
 int &nattr,
 MoMgrTables tbl ) {
 UPLL_FUNC_TRACE;
 if (MAINTBL == tbl) {

 nattr = NUM_KEY_MAIN_TBL_;
 binfo = key_vtep_maintbl_bind_info;
 return true;
 }
 return false;;
 }*/


bool VtepMoMgr::IsValidKey(void *key, uint64_t index, MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  key_vtep *vtep_key = reinterpret_cast<key_vtep *>(key);
  upll_rc_t ret_val = UPLL_RC_SUCCESS;
  switch (index) {
    case uudst::vtep::kDbiVtnName:
      ret_val = ValidateKey(reinterpret_cast<char *>
                (vtep_key->vtn_key.vtn_name), kMinLenVtnName, kMaxLenVtnName);
      if (ret_val != UPLL_RC_SUCCESS) {
        UPLL_LOG_TRACE("VTN Name is not valid(%d)", ret_val);
        return false;
      }
      break;
    case uudst::vtep::kDbiVtepName:
      ret_val = ValidateKey(reinterpret_cast<char *>
                (vtep_key->vtep_name), kMinLenVnodeName, kMaxLenVnodeName);
      if (ret_val != UPLL_RC_SUCCESS) {
        UPLL_LOG_TRACE("Vtep Name is not valid(%d)", ret_val);
        return false;
      }
      break;
    default:
      UPLL_LOG_TRACE("Invalid Key Index");
      return false;
  }
  return true;
}

upll_rc_t VtepMoMgr::GetChildConfigKey(ConfigKeyVal *&okey,
    ConfigKeyVal *parent_key) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  key_vtep *vtep_key = static_cast<key_vtep *>
                       (ConfigKeyVal::Malloc(sizeof(key_vtep)));
  void *pkey;
  if (parent_key == NULL) {
    if (okey) delete okey;
    okey = new ConfigKeyVal(UNC_KT_VTEP, IpctSt::kIpcStKeyVtep, vtep_key, NULL);
    return UPLL_RC_SUCCESS;
  } else {
    pkey = parent_key->get_key();
  }
  if (!pkey) {
    FREE_IF_NOT_NULL(vtep_key);
    return UPLL_RC_ERR_GENERIC;
  }
  switch (parent_key->get_key_type()) {
    case UNC_KT_VTEP:
      uuu::upll_strncpy(vtep_key->vtep_name,
                        static_cast<key_vtep *>(pkey)->vtep_name,
                        (kMaxLenVnodeName+1));
      uuu::upll_strncpy(vtep_key->vtn_key.vtn_name,
                        static_cast<key_vtep *>(pkey)->vtn_key.vtn_name,
                        (kMaxLenVtnName+1));
      break;
    case UNC_KT_VTN:
    default:
      uuu::upll_strncpy(vtep_key->vtn_key.vtn_name,
                        reinterpret_cast<key_vtn *>(pkey)->vtn_name,
          (kMaxLenVtnName+1) );
      *(vtep_key->vtep_name)  = *"";
  }
  okey = new ConfigKeyVal(UNC_KT_VTEP, IpctSt::kIpcStKeyVtep, vtep_key, NULL);
  if (okey == NULL) {
    FREE_IF_NOT_NULL(vtep_key);
    result_code = UPLL_RC_ERR_GENERIC;
  } else {
    SET_USER_DATA(okey, parent_key);
  }
  /* presumes MoMgrs receive only supported keytypes */
  return result_code;
}

upll_rc_t VtepMoMgr::GetParentConfigKey(ConfigKeyVal *&okey,
    ConfigKeyVal *ikey ) {
  UPLL_FUNC_TRACE;
  if ((ikey == NULL) || (ikey->get_key() == NULL)) {
    UPLL_LOG_DEBUG("Null ikey param");
    return UPLL_RC_ERR_GENERIC;
  }
  key_vtep *vtep_key = static_cast<key_vtep *>(ikey->get_key());
  if (!vtep_key) return UPLL_RC_ERR_GENERIC;

  unc_key_type_t ikey_type = ikey->get_key_type();
  if (ikey_type != UNC_KT_VTEP)
    return UPLL_RC_ERR_GENERIC;
  key_vtn *vtn_key = static_cast<key_vtn *>
                     (ConfigKeyVal::Malloc(sizeof(key_vtn)));
  uuu::upll_strncpy(vtn_key->vtn_name, vtep_key->vtn_key.vtn_name,
      (kMaxLenVtnName+1));
  DELETE_IF_NOT_NULL(okey);
  okey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, vtn_key, NULL);
  if (okey == NULL) {
    FREE_IF_NOT_NULL(vtn_key);
    return UPLL_RC_ERR_GENERIC;
  } else {
    SET_USER_DATA(okey, ikey);
  }
  return UPLL_RC_SUCCESS;
}


upll_rc_t VtepMoMgr::AllocVal(ConfigVal *&ck_val,
    upll_keytype_datatype_t dt_type, MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  void *val;

  if (ck_val != NULL) return UPLL_RC_ERR_GENERIC;
  switch (tbl) {
    case MAINTBL:
      val =reinterpret_cast<void *>(ConfigKeyVal::Malloc(sizeof(val_vtep)));
      ck_val = new ConfigVal(IpctSt::kIpcStValVtep, val);
      if (!ck_val) {
        FREE_IF_NOT_NULL(reinterpret_cast<val_vtep *>(val));
        return UPLL_RC_ERR_GENERIC;
      }
      if (dt_type == UPLL_DT_STATE) {
        val = reinterpret_cast<void *>
          (ConfigKeyVal::Malloc(sizeof(val_db_vtep_st)));
        ConfigVal *ck_nxtval = new ConfigVal(IpctSt::kIpcStValVtepSt, val);
        if (!ck_nxtval) {
          delete ck_val;
          FREE_IF_NOT_NULL(reinterpret_cast<val_db_vtep_st *>(val));
          return UPLL_RC_ERR_GENERIC;
        }
        ck_val->AppendCfgVal(ck_nxtval);
      }
      break;
    default:
      val = NULL;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtepMoMgr::DupConfigKeyVal(ConfigKeyVal *&okey,
    ConfigKeyVal *&req, MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  if (req == NULL) return UPLL_RC_ERR_GENERIC;
  if (okey != NULL) return UPLL_RC_ERR_GENERIC;
  if (req->get_key_type() != UNC_KT_VTEP)
    return UPLL_RC_ERR_GENERIC;
  ConfigVal *tmp1 = NULL, *tmp = (req)->get_cfg_val();
  if (tmp) {
    if (tbl == MAINTBL) {
      val_vtep *ival = static_cast<val_vtep *>(GetVal(req));
      if (ival == NULL) {
        UPLL_LOG_DEBUG("Null Val structure");
        return UPLL_RC_ERR_GENERIC;
      }
      val_vtep *vtep_val = static_cast<val_vtep *>
        (ConfigKeyVal::Malloc(sizeof(val_vtep)));
      memcpy(vtep_val, ival, sizeof(val_vtep));
      tmp1 = new ConfigVal(IpctSt::kIpcStValVtep, vtep_val);
      if (!tmp1) {
        FREE_IF_NOT_NULL(vtep_val);
        return UPLL_RC_ERR_GENERIC;
      }
    }
    tmp = tmp->get_next_cfg_val();
  };
  if (tmp) {
    if (tbl == MAINTBL) {
      val_db_vtep_st *ival = static_cast<val_db_vtep_st *>(tmp->get_val());
      val_db_vtep_st *vtep_st = static_cast<val_db_vtep_st *>
                                (ConfigKeyVal::Malloc(sizeof(val_db_vtep_st)));
      memcpy(vtep_st, ival, sizeof(val_db_vtep_st));
      ConfigVal *tmp2 = new ConfigVal(IpctSt::kIpcStValVtepSt, vtep_st);
      if (!tmp2) {
        delete tmp1;
        FREE_IF_NOT_NULL(vtep_st);
        return UPLL_RC_ERR_GENERIC;
      }
      tmp1->AppendCfgVal(tmp2);
    }
  };
  void *tkey = (req != NULL)?(req)->get_key():NULL;
  if (!tkey) {
    UPLL_LOG_DEBUG("Null tkey");
    DELETE_IF_NOT_NULL(tmp1);
    return UPLL_RC_ERR_GENERIC;
  }
  key_vtep *ikey = static_cast<key_vtep *>(tkey);
  key_vtep *vtep_key = static_cast<key_vtep *>
                       (ConfigKeyVal::Malloc(sizeof(key_vtep)));
  memcpy(vtep_key, ikey, sizeof(key_vtep));
  okey = new ConfigKeyVal(UNC_KT_VTEP, IpctSt::kIpcStKeyVtep, vtep_key, tmp1);
  if (okey) {
    SET_USER_DATA(okey, req);
  } else {
    if (tmp1) delete tmp1;
    FREE_IF_NOT_NULL(vtep_key);
    return UPLL_RC_ERR_GENERIC;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtepMoMgr::UpdateConfigStatus(ConfigKeyVal *vtep_key,
    unc_keytype_operation_t op,
    uint32_t driver_result,
    ConfigKeyVal *upd_key,
    DalDmlIntf *dmi,
    ConfigKeyVal *ctrlr_key) {
  UPLL_FUNC_TRACE;
  unc_keytype_configstatus_t cs_status =
                (driver_result == UPLL_RC_SUCCESS)? UNC_CS_APPLIED
                                                  : UNC_CS_NOT_APPLIED;
  val_vtep *vtep_val = static_cast<val_vtep *>(GetVal(vtep_key));
  if (vtep_val == NULL) return UPLL_RC_ERR_GENERIC;
  val_vtep *val_running = static_cast<val_vtep *>(GetVal(upd_key));
  if (op == UNC_OP_CREATE) {
    vtep_val->cs_row_status = cs_status;
    /* initialize entity/state information */
    val_db_vtep_st *vtep_db_valst = static_cast<val_db_vtep_st *>
      (ConfigKeyVal::Malloc(sizeof(val_db_vtep_st)));
    val_vtep_st *vtepst_val = &vtep_db_valst->vtep_val_st;
    vtepst_val->oper_status = (driver_result == UPLL_RC_ERR_CTR_DISCONNECTED)?
                               UPLL_OPER_STATUS_UNKNOWN:
                              UPLL_OPER_STATUS_UP;
    vtepst_val->valid[UPLL_IDX_OPER_STATUS_VTEPS] = UNC_VF_VALID;
    vtep_db_valst->down_count  = 0;
    vtep_db_valst->unknown_count = 0;
    vtep_key->AppendCfgVal(IpctSt::kIpcStValVtepSt, vtep_db_valst);
  } else if (op == UNC_OP_UPDATE) {
    void *val = reinterpret_cast<void *>(vtep_val);
    CompareValidValue(val, GetVal(upd_key), true);
    UPLL_LOG_TRACE("%s", (upd_key->ToStrAll()).c_str());
    vtep_val->cs_row_status = val_running->cs_row_status;
  } else {
    return UPLL_RC_ERR_GENERIC;
  }
  for (unsigned int loop = 0;
      loop < sizeof(vtep_val->valid)/sizeof(vtep_val->valid[0]);
      ++loop ) {
    if ((UNC_VF_VALID == vtep_val->valid[loop]) ||
        (UNC_VF_VALID_NO_VALUE == vtep_val->valid[loop])) {
      // Description is set to APPLIED
      if (loop == UPLL_IDX_DESC_VTEP)
        vtep_val->cs_attr[loop] = UNC_CS_APPLIED;
      else
        vtep_val->cs_attr[loop] = cs_status;
    } else if ((vtep_val->valid[loop] == UNC_VF_INVALID) &&
               (UNC_OP_CREATE == op)) {
      vtep_val->cs_attr[loop] = UNC_CS_APPLIED;
    } else if ((vtep_val->valid[loop] == UNC_VF_INVALID) &&
               (UNC_OP_UPDATE == op)) {
     vtep_val->cs_attr[loop] = val_running->cs_attr[loop];
    }
  }
  return UPLL_RC_SUCCESS;
}

bool VtepMoMgr::FilterAttributes(void *&val1, void *val2,
                                 bool copy_to_running,
                                 unc_keytype_operation_t op) {
  UPLL_FUNC_TRACE;
  val_vtep_t *val_vtep1 = reinterpret_cast<val_vtep_t *>(val1);
  val_vtep_t *val_vtep2 = reinterpret_cast<val_vtep_t *>(val2);
  /* No need to configure description in controller. */
  val_vtep1->valid[UPLL_IDX_DESC_VTEP] = UNC_VF_INVALID;
  if (op == UNC_OP_UPDATE) {
    val_vtep1->valid[UPLL_IDX_CONTROLLER_ID_VTEP] = UNC_VF_INVALID;
    val_vtep1->valid[UPLL_IDX_DOMAIN_ID_VTEP] = UNC_VF_INVALID;
    val_vtep2->valid[UPLL_IDX_CONTROLLER_ID_VTEP] = UNC_VF_INVALID;
    val_vtep2->valid[UPLL_IDX_DOMAIN_ID_VTEP] = UNC_VF_INVALID;
  }
  if (op != UNC_OP_CREATE)
    return CompareValidValue(val1, val2, copy_to_running);
  return false;
}

bool VtepMoMgr::CompareValidValue(void *&val1, void *val2,
                                  bool copy_to_running) {
  UPLL_FUNC_TRACE;
  bool invalid_attr = true;
  val_vtep_t *val_vtep1 = reinterpret_cast<val_vtep_t *>(val1);
  val_vtep_t *val_vtep2  = reinterpret_cast<val_vtep_t *>(val2);
  for (unsigned int loop = 0;
       loop < sizeof(val_vtep1->valid)/sizeof(uint8_t); ++loop ) {
    if (UNC_VF_INVALID == val_vtep1->valid[loop]
        && UNC_VF_VALID == val_vtep2->valid[loop])
      val_vtep1->valid[loop] = UNC_VF_VALID_NO_VALUE;
  }
  if (copy_to_running) {
    if (UNC_VF_VALID == val_vtep1->valid[UPLL_IDX_DESC_VTEP]
        && UNC_VF_VALID == val_vtep2->valid[UPLL_IDX_DESC_VTEP]) {
      if (!strcmp(reinterpret_cast<char*>(val_vtep1->description),
                  reinterpret_cast<char*>(val_vtep2->description)))
        val_vtep1->valid[UPLL_IDX_DESC_VTEP] = UNC_VF_INVALID;
    }
    if (UNC_VF_VALID == val_vtep1->valid[UPLL_IDX_CONTROLLER_ID_VTEP]
        && UNC_VF_VALID == val_vtep2->valid[UPLL_IDX_CONTROLLER_ID_VTEP]) {
      if (!strcmp(reinterpret_cast<char*>(val_vtep1->controller_id),
                  reinterpret_cast<char*>(val_vtep2->controller_id)))
        val_vtep1->valid[UPLL_IDX_CONTROLLER_ID_VTEP] = UNC_VF_INVALID;
    }
    if (UNC_VF_VALID == val_vtep1->valid[UPLL_IDX_DOMAIN_ID_VTEP]
        && UNC_VF_VALID == val_vtep2->valid[UPLL_IDX_DOMAIN_ID_VTEP]) {
      if (!strcmp(reinterpret_cast<char*>(val_vtep1->domain_id),
                  reinterpret_cast<char*>(val_vtep2->domain_id)))
        val_vtep1->valid[UPLL_IDX_DOMAIN_ID_VTEP] = UNC_VF_INVALID;
    }
  }
  if (!copy_to_running)
    val_vtep1->valid[UPLL_IDX_DESC_VTEP] = UNC_VF_INVALID;
  for (unsigned int loop = 0;
       loop < sizeof(val_vtep1->valid) / sizeof(uint8_t); ++loop) {
    if ((UNC_VF_VALID == (uint8_t) val_vtep1->valid[loop]) ||
        (UNC_VF_VALID_NO_VALUE == (uint8_t) val_vtep1->valid[loop])) {
      invalid_attr = false;
      break;
    }
  }
  return invalid_attr;
}

upll_rc_t VtepMoMgr::UpdateAuditConfigStatus(
    unc_keytype_configstatus_t cs_status,
    uuc::UpdateCtrlrPhase phase,
    ConfigKeyVal *&ckv_running,
    DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  val_vtep_t *val = (ckv_running != NULL)?reinterpret_cast<val_vtep_t *>
    (GetVal(ckv_running)):NULL;
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
      loop < sizeof(val->valid)/sizeof(uint8_t); ++loop) {
    if ((cs_status == UNC_CS_INVALID && UNC_VF_VALID == val->valid[loop])
        || cs_status == UNC_CS_APPLIED)
      val->cs_attr[loop] = cs_status;
  }
  return result_code;
}

/* Pure Virtual functions from VnodeMoMgr */
upll_rc_t VtepMoMgr::GetControllerDomainId(ConfigKeyVal *ikey,
    controller_domain *ctrlr_dom) {
  UPLL_FUNC_TRACE;
  if (!ikey || !ctrlr_dom) {
    UPLL_LOG_INFO("Illegal parameter");
    return UPLL_RC_ERR_GENERIC;
  }
  val_vtep *temp_vtep = static_cast<val_vtep *>(GetVal(ikey));
  if (!temp_vtep) {
    UPLL_LOG_DEBUG("value null");
    return UPLL_RC_ERR_GENERIC;
  }
  if (temp_vtep->valid[UPLL_IDX_CONTROLLER_ID_VTEP] != UNC_VF_VALID
      || !strlen(reinterpret_cast<char*>(temp_vtep->controller_id))) {
    ctrlr_dom->ctrlr = NULL;
    UPLL_LOG_DEBUG("Ctrlr null");
  } else {
    SET_USER_DATA_CTRLR(ikey, temp_vtep->controller_id);
  }
  if (temp_vtep->valid[UPLL_IDX_DOMAIN_ID_VTEP] != UNC_VF_VALID
      || !strlen(reinterpret_cast<char*>(temp_vtep->domain_id))) {
    ctrlr_dom->domain = NULL;
    UPLL_LOG_DEBUG("Domain null");
  } else {
    SET_USER_DATA_DOMAIN(ikey, temp_vtep->domain_id);
    GET_USER_DATA_CTRLR_DOMAIN(ikey, *ctrlr_dom);
  }
  UPLL_LOG_DEBUG("ctrlr_dom %s %s", ctrlr_dom->ctrlr, ctrlr_dom->domain);
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtepMoMgr::GetVnodeName(ConfigKeyVal *ikey, uint8_t *&vtn_name,
    uint8_t *&vnode_name) {
  UPLL_FUNC_TRACE;
  if (ikey == NULL) {
    UPLL_LOG_DEBUG("ConfigKeyVal is NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  key_vtep_t *vtep_key = static_cast<key_vtep_t *>(ikey->get_key());
  if (vtep_key == NULL) return UPLL_RC_ERR_GENERIC;
  vtn_name = vtep_key->vtn_key.vtn_name;
  vnode_name = vtep_key->vtep_name;
  return  UPLL_RC_SUCCESS;
}

upll_rc_t VtepMoMgr::ValidateMessage(IpcReqRespHeader *req,
    ConfigKeyVal *ikey) {
  UPLL_FUNC_TRACE;
  upll_rc_t ret_val = UPLL_RC_ERR_GENERIC;
  if (!ikey || !req || !(ikey->get_key())) {
    UPLL_LOG_DEBUG("IpcReqRespHeader or ConfigKeyVal is Null");
    return UPLL_RC_ERR_BAD_REQUEST;
  }
  unc_key_type_t keytype = ikey->get_key_type();
  if (UNC_KT_VTEP != keytype) {
    UPLL_LOG_DEBUG("Invalid keytype. Keytype- %d", keytype);
    return UPLL_RC_ERR_BAD_REQUEST;
  }
  if (ikey->get_st_num() != IpctSt::kIpcStKeyVtep) {
    UPLL_LOG_DEBUG("Invalid structure received.Expected struct-kIpcStKeyVtep, "
        "received struct -%s ", reinterpret_cast<const char *>
        (IpctSt::GetIpcStdef(ikey->get_st_num())));
    return UPLL_RC_ERR_BAD_REQUEST;
  }
  key_vtep_t *key_vtep = reinterpret_cast<key_vtep_t *>(ikey->get_key());
  val_vtep_t *val_vtep = NULL;
  if ((ikey->get_cfg_val()) &&
      ((ikey->get_cfg_val())->get_st_num() == IpctSt::kIpcStValVtep)) {
    val_vtep =
      reinterpret_cast<val_vtep_t *> (ikey->get_cfg_val()->get_val());
  }
  uint32_t dt_type = req->datatype;
  uint32_t operation = req->operation;
  uint32_t option1 = req->option1;
  uint32_t option2 = req->option2;
  ret_val = ValidateVTepKey(key_vtep, operation);
  if (ret_val != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Syntax check failed for VTEP key structure");
    return UPLL_RC_ERR_CFG_SYNTAX;
  } else {
    if (((operation == UNC_OP_CREATE) || (operation == UNC_OP_UPDATE)) &&
         (dt_type == UPLL_DT_CANDIDATE)) {
      if (val_vtep != NULL) {
        ret_val = ValidateVTepValue(val_vtep, operation);
        if (ret_val != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("Syntax check failed for VTEP value structure");
          return UPLL_RC_ERR_CFG_SYNTAX;
        }
        return UPLL_RC_SUCCESS;
      } else {
        UPLL_LOG_DEBUG("Value structure is mandatory");
        return UPLL_RC_ERR_BAD_REQUEST;
      }
    } else if (((operation == UNC_OP_READ) ||
          (operation == UNC_OP_READ_SIBLING) ||
          (operation == UNC_OP_READ_SIBLING_BEGIN) ||
          (operation == UNC_OP_READ_SIBLING_COUNT))
        && ((dt_type == UPLL_DT_CANDIDATE) ||
          (dt_type == UPLL_DT_RUNNING) ||
          (dt_type == UPLL_DT_STARTUP) ||
          (dt_type == UPLL_DT_STATE))) {
      if (option1 == UNC_OPT1_NORMAL) {
        if (option2 == UNC_OPT2_NONE) {
          if (val_vtep != NULL) {
            ret_val = ValidateVTepValue(val_vtep);
            if (ret_val != UPLL_RC_SUCCESS) {
              UPLL_LOG_DEBUG("Syntax check failed for VTEP value structure");
              return UPLL_RC_ERR_CFG_SYNTAX;
            }
            return UPLL_RC_SUCCESS;
          } else {
            UPLL_LOG_TRACE("value structure is an optional for READ op");
            return UPLL_RC_SUCCESS;
          }
        } else {
          UPLL_LOG_DEBUG("option2 is not matching");
          return UPLL_RC_ERR_INVALID_OPTION2;
        }
      } else {
        UPLL_LOG_DEBUG("option1 is not matching");
        return UPLL_RC_ERR_INVALID_OPTION1;
      }
    } else if ((operation == UNC_OP_DELETE) ||
        (((operation == UNC_OP_READ_NEXT) ||
        (operation == UNC_OP_READ_BULK)) &&
        ((dt_type == UPLL_DT_CANDIDATE) ||
          (dt_type == UPLL_DT_RUNNING) ||
          (dt_type == UPLL_DT_STARTUP)))) {
      UPLL_LOG_TRACE("Value struct is none for operation type:%d", operation);
      return UPLL_RC_SUCCESS;
    } else {
      UPLL_LOG_DEBUG("Invalid datatype(%d) or operation(%d)", dt_type,
          operation);
      return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT;
    }
  }
  return UPLL_RC_SUCCESS;
}
upll_rc_t VtepMoMgr::ValidateVTepValue(val_vtep_t *val_vtep,
    uint32_t operation) {
  UPLL_FUNC_TRACE;
  bool ret_val =false;

  // Attribute syntax validation
  for (unsigned int valid_index = 0;
       valid_index < sizeof(val_vtep->valid) / sizeof(val_vtep->valid[0]);
       valid_index++) {
    if (val_vtep->valid[valid_index] == UNC_VF_VALID) {
      switch (valid_index) {
        case UPLL_IDX_CONTROLLER_ID_VTEP:
          ret_val = ValidateString(val_vtep->controller_id,
                                   kMinLenCtrlrId, kMaxLenCtrlrId);
          break;
        case UPLL_IDX_DOMAIN_ID_VTEP:
          ret_val = ValidateDefaultStr(val_vtep->domain_id,
                                       kMinLenDomainId, kMaxLenDomainId);
          break;
        case UPLL_IDX_DESC_VTEP:
          ret_val = ValidateDesc(val_vtep->description,
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
       valid_index < sizeof(val_vtep->valid) / sizeof(val_vtep->valid[0]);
       valid_index++) {
    uint8_t flag = val_vtep->valid[valid_index];
    switch (operation) {
      case UNC_OP_CREATE:
        {
          switch (valid_index) {
            case UPLL_IDX_CONTROLLER_ID_VTEP:
            case UPLL_IDX_DOMAIN_ID_VTEP:
              if ((flag == UNC_VF_INVALID || flag == UNC_VF_VALID_NO_VALUE)) {
                UPLL_LOG_DEBUG("controller_id or domain_id flag is invalid"
                               " or valid_no_value");
                return UPLL_RC_ERR_CFG_SYNTAX;
              }
              break;
            case UPLL_IDX_DESC_VTEP:
              break;
            default:
              break;
          }
        }
        break;
      case UNC_OP_UPDATE:
        {
          switch (valid_index) {
            case UPLL_IDX_CONTROLLER_ID_VTEP:
            case UPLL_IDX_DOMAIN_ID_VTEP:
              if (flag == UNC_VF_VALID_NO_VALUE) {
                UPLL_LOG_DEBUG("controller_id or domain_id flag is"
                                                 " valid_no_value");
                return UPLL_RC_ERR_CFG_SYNTAX;
              }
              break;
            case UPLL_IDX_DESC_VTEP:
            default:
              break;
          }
        }
        break;
    }
  }

  // Resets
  for (unsigned int valid_index = 0;
       valid_index < sizeof(val_vtep->valid) / sizeof(val_vtep->valid[0]);
       valid_index++) {
    uint8_t flag = val_vtep->valid[valid_index];
    if (flag != UNC_VF_INVALID && flag != UNC_VF_VALID_NO_VALUE)
      continue;

    switch (valid_index) {
      case UPLL_IDX_CONTROLLER_ID_VTEP:
        StringReset(val_vtep->controller_id);
        break;
      case UPLL_IDX_DOMAIN_ID_VTEP:
        StringReset(val_vtep->domain_id);
        break;
      case UPLL_IDX_DESC_VTEP:
        StringReset(val_vtep->description);
        break;
      default:
        UPLL_LOG_TRACE("Never here");
        break;
    }
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtepMoMgr::CtrlrIdAndDomainIdUpdationCheck(ConfigKeyVal *ikey,
                                                   ConfigKeyVal *okey) {
  UPLL_FUNC_TRACE;
  val_vtep *vtep_val = reinterpret_cast<val_vtep *>(GetVal(ikey));
  val_vtep *vtep_val1 = reinterpret_cast<val_vtep *>(GetVal(okey));
  if (vtep_val->valid[UPLL_IDX_CONTROLLER_ID_VTEP] == UNC_VF_VALID) {
    if (strncmp(reinterpret_cast<const char *>(vtep_val->controller_id),
                reinterpret_cast<const char *>(vtep_val1->controller_id),
                kMaxLenCtrlrId+1)) {
      UPLL_LOG_DEBUG("controller id comparision failed");
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  }
  if (vtep_val->valid[UPLL_IDX_DOMAIN_ID_VTEP] == UNC_VF_VALID) {
    if (strncmp(reinterpret_cast<const char *>(vtep_val->domain_id),
                reinterpret_cast<const char *>(vtep_val1->domain_id),
                kMaxLenDomainId+1)) {
      UPLL_LOG_DEBUG("domain id comparision failed");
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  }
  return UPLL_RC_SUCCESS;
}
upll_rc_t VtepMoMgr::ValidateVTepKey(key_vtep_t *key_vtep,
                     uint32_t operation) {
  UPLL_FUNC_TRACE;
  upll_rc_t ret_val = UPLL_RC_SUCCESS;
  ret_val = ValidateKey(
  reinterpret_cast<char *>(key_vtep->vtn_key.vtn_name),
      kMinLenVtnName, kMaxLenVtnName);
  if (ret_val != UPLL_RC_SUCCESS) {
    UPLL_LOG_INFO("Vtn Name syntax check failed."
                  "Received VTN Name - %s",
                  key_vtep->vtn_key.vtn_name);
    return UPLL_RC_ERR_CFG_SYNTAX;
  }
  if ((operation != UNC_OP_READ_SIBLING_BEGIN) &&
      (operation != UNC_OP_READ_SIBLING_COUNT)) {
    UPLL_LOG_TRACE("UNC_KT_VTEP: vtep_name  (%s)", key_vtep->vtep_name);
    ret_val = ValidateKey(reinterpret_cast<char *>(key_vtep->vtep_name),
              kMinLenVnodeName, kMaxLenVnodeName);
    if (ret_val != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Syntax check failed.vtep_name-(%s)", key_vtep->vtep_name);
      return ret_val;
    }
  } else {
    UPLL_LOG_TRACE("Operation is %d", operation);
    StringReset(key_vtep->vtep_name);
  }
  return UPLL_RC_SUCCESS;
}
upll_rc_t VtepMoMgr::ValVTepAttributeSupportCheck(val_vtep_t *val_vtep,
    const uint8_t* attrs, unc_keytype_operation_t operation) {
  UPLL_FUNC_TRACE;
  if ((val_vtep->valid[UPLL_IDX_DESC_VTEP] == UNC_VF_VALID)
      || (val_vtep->valid[UPLL_IDX_DESC_VTEP] == UNC_VF_VALID_NO_VALUE)) {
    if (attrs[unc::capa::vtep::kCapDesc] == 0) {
      val_vtep->valid[UPLL_IDX_DESC_VTEP] = UNC_VF_INVALID;
      if (operation == UNC_OP_CREATE || operation == UNC_OP_UPDATE) {
        UPLL_LOG_DEBUG("CapDesc is not supported for PFC Controller");
        return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
      }
    }
  }
  return UPLL_RC_SUCCESS;
}
upll_rc_t VtepMoMgr::ValidateCapability(IpcReqRespHeader *req,
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
      result_code = GetCreateCapability(ctrlr_name, ikey->get_key_type(),
          &max_instance_count, &max_attrs, &attrs);
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
      UPLL_LOG_DEBUG("Invalid operation");
      break;
  }
  if (!result_code) {
    UPLL_LOG_DEBUG("keytype(%d) is not supported by controller(%s) "
        "for operation(%d)",
        ikey->get_key_type(), ctrlr_name, req->operation);
    return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
  }
  val_vtep_t *val_vtep = NULL;
  if ((ikey->get_cfg_val()) &&
      ((ikey->get_cfg_val())->get_st_num() == IpctSt::kIpcStValVtep)) {
    val_vtep =
      reinterpret_cast<val_vtep_t *>(ikey->get_cfg_val()->get_val());
  }
  if (val_vtep) {
    if (max_attrs > 0) {
      return ValVTepAttributeSupportCheck(val_vtep, attrs, req->operation);
    } else {
      UPLL_LOG_DEBUG("Attribute list is empty for operation %d",
                                                 req->operation);
      return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
    }
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtepMoMgr::CreateVnodeConfigKey(ConfigKeyVal *ikey,
    ConfigKeyVal *&okey) {
  UPLL_FUNC_TRACE;
  if (ikey == NULL)
    return UPLL_RC_ERR_GENERIC;
  key_vtep *temp_key_vtep = static_cast<key_vtep *>
                            (ConfigKeyVal::Malloc(sizeof(key_vtep)));
  uuu::upll_strncpy(temp_key_vtep->vtn_key.vtn_name,
      static_cast<key_vtep *>(ikey->get_key())->vtn_key.vtn_name,
      (kMaxLenVtnName+1));
  uuu::upll_strncpy(temp_key_vtep->vtep_name,
      static_cast<key_vtep *>(ikey->get_key())->vtep_name,
      (kMaxLenVnodeName+1));

  okey = new ConfigKeyVal(UNC_KT_VTEP, IpctSt::kIpcStKeyVtep,
      reinterpret_cast<void *>(temp_key_vtep), NULL);
  return UPLL_RC_SUCCESS;
}
/*
   upll_rc_t VtepMoMgr::CopyToConfigKey(ConfigKeyVal *&okey,
   ConfigKeyVal *ikey) {
   UPLL_FUNC_TRACE;
   if (!ikey || !(ikey->get_key()))
   return UPLL_RC_ERR_GENERIC;
   upll_rc_t result_code = UPLL_RC_SUCCESS;

   key_rename_vnode_info *key_rename = (key_rename_vnode_info *)ikey->get_key();
   key_vtep_t * vtep_key = (key_vtep_t *) malloc ( sizeof (key_vtep_t));
   if (!vtep_key)
   return UPLL_RC_ERR_GENERIC;
   if (!strlen ((char *)key_rename->old_unc_vtn_name))
   return UPLL_RC_ERR_GENERIC;
   strcpy ((char *)vtep_key ->vtn_key.vtn_name,
   (char *)key_rename->old_unc_vtn_name);

   okey = new ConfigKeyVal (UNC_KT_VTEP, IpctSt::kIpcStKeyVtep, vtep_key, NULL);
   if (!okey) {
   FREE_IF_NOT_NULL(vtep_key);
   return UPLL_RC_ERR_GENERIC;
   }
   return result_code;
   }
   */
upll_rc_t VtepMoMgr::CreateVtepGrpConfigKey(ConfigKeyVal *&okey,
    ConfigKeyVal *ikey) {
  UPLL_FUNC_TRACE;
  key_vtep_grp_member_t *vtep_grp_mem_key = static_cast<key_vtep_grp_member_t *>
    (ConfigKeyVal::Malloc(sizeof(key_vtep_grp_member_t)));
  key_vtep *vtep_key = static_cast<key_vtep *>(ikey->get_key());
  if (!strlen(reinterpret_cast<char *>(vtep_key->vtn_key.vtn_name))) {
    FREE_IF_NOT_NULL(vtep_grp_mem_key);
    return UPLL_RC_ERR_GENERIC;
  }
  uuu::upll_strncpy(vtep_grp_mem_key->vtepgrp_key.vtn_key.vtn_name,
      reinterpret_cast<char *>(vtep_key->vtn_key.vtn_name),
      (kMaxLenVtnName+1));
  if (!strlen(reinterpret_cast<char *>(vtep_key->vtep_name))) {
    FREE_IF_NOT_NULL(vtep_grp_mem_key);
    return UPLL_RC_ERR_GENERIC;
  }
  uuu::upll_strncpy(vtep_grp_mem_key->vtepmember_name,
      reinterpret_cast<char *>(vtep_key->vtep_name),
      (kMaxLenVnodeName+1));
  okey = new ConfigKeyVal(UNC_KT_VTEP_GRP_MEMBER,
      IpctSt::kIpcStKeyVtepGrpMember, vtep_grp_mem_key, NULL);
  if (!okey) {
    FREE_IF_NOT_NULL(vtep_grp_mem_key);
    return UPLL_RC_ERR_GENERIC;
  }
  return UPLL_RC_SUCCESS;
}


upll_rc_t VtepMoMgr::IsReferenced(IpcReqRespHeader *req,
    ConfigKeyVal *ikey,
    DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *okey = NULL;
  if (!ikey || !(ikey->get_key()) || !dmi)
    return UPLL_RC_ERR_GENERIC;
  MoMgrImpl *mgr1 = reinterpret_cast<MoMgrImpl *>(const_cast<MoManager*>
      (GetMoManager(UNC_KT_VTEP_IF)));
  if (!mgr1) return UPLL_RC_ERR_GENERIC;
  result_code = mgr1->IsReferenced(req, ikey, dmi);
  result_code = (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE)?
                    UPLL_RC_SUCCESS:result_code;
  UPLL_LOG_DEBUG("Interface IsReferenced returned: (%d)", result_code);
  if (UPLL_RC_SUCCESS != result_code)
    return result_code;
  result_code = CreateVtepGrpConfigKey(okey, ikey);
  if (UPLL_RC_SUCCESS != result_code) {
    return result_code;
  }
  MoMgrImpl *mgr2 = reinterpret_cast<MoMgrImpl *>(const_cast<MoManager*>
      (GetMoManager(UNC_KT_VTEP_GRP_MEMBER)));
  if (!mgr2) {
    DELETE_IF_NOT_NULL(okey);
    return UPLL_RC_ERR_GENERIC;
  }
  result_code =  mgr2->UpdateConfigDB(okey, req->datatype, UNC_OP_READ,
                                      dmi, MAINTBL);
  DELETE_IF_NOT_NULL(okey);
  if (UPLL_RC_ERR_INSTANCE_EXISTS == result_code)
    result_code = UPLL_RC_ERR_CFG_SEMANTIC;

  result_code = (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE)?
         UPLL_RC_SUCCESS:result_code;
  return result_code;
}

upll_rc_t VtepMoMgr::AdaptValToVtnService(ConfigKeyVal *ikey,
                          AdaptType adapt_type) {
  UPLL_FUNC_TRACE;
  if (!ikey) {
    UPLL_LOG_DEBUG("Invalid ikey");
    return UPLL_RC_ERR_GENERIC;
  }
  key_vtep *vtep_key = reinterpret_cast<key_vtep*>(ikey->get_key());
  while (ikey) {
    ConfigVal *cval = ikey->get_cfg_val();
    if (!cval) {
      UPLL_LOG_DEBUG("Config Val is Null");
      return UPLL_RC_ERR_GENERIC;
    }
    while (cval) {
      if (IpctSt::kIpcStValVtepSt == cval->get_st_num()) {
        controller_domain ctrlr_dom = {NULL, NULL};
        GET_USER_DATA_CTRLR_DOMAIN(ikey, ctrlr_dom);
        CheckOperStatus<val_vtep_st>(vtep_key->vtn_key.vtn_name,
                                     cval, UNC_KT_VTEP, ctrlr_dom);
      }
      cval = cval->get_next_cfg_val();
    }
    if (adapt_type == ADAPT_ONE)
      break;
    ikey = ikey->get_next_cfg_key_val();
  }
  return UPLL_RC_SUCCESS;
}

/*
   upll_rc_t VtepMoMgr::MergeValidate(unc_key_type_t keytype,
   const char *ctrlr_id,
   ConfigKeyVal *ikey, DalDmlIntf *dmi) {
   UPLL_FUNC_TRACE;
   upll_rc_t result_code = UPLL_RC_SUCCESS;
   DbSubOp dbop = {kOpReadMultiple, kOpMatchNone, kOpInOutNone};
   if (!ikey || !(ikey->get_key()) || !dmi
   || !(strlen((char*)ctrlr_id)))
   return UPLL_RC_ERR_GENERIC;
 * Here getting FULL Key (VTN & VBR Name )

 result_code = ReadConfigDB(ikey, UPLL_DT_IMPORT, UNC_OP_READ,
 dbop, dmi, MAINTBL);
 if (UPLL_RC_SUCCESS != result_code)
 return result_code;
 checks the vnode name present in the running vnode under the
 * same vtn

 while (!ikey) {
 Same Name should not present in the vnodes in running
 result_code = VnodeChecks(ikey, UPLL_DT_RUNNING, dmi, false);

 if (UPLL_RC_ERR_INSTANCE_EXISTS == result_code )
 return UPLL_RC_ERR_MERGE_CONFLICT;
 Any other DB error
 if ( UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
 return result_code;
 }


 ikey = ikey->get_next_cfg_key_val();
 }

 return result_code;
 }
 */

}  // namespace kt_momgr
}  // namespace upll
}  // namespace unc
