/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include "nwm_momgr.hh"
#include "upll_log.hh"
#include "vbr_momgr.hh"

#define NUM_KEY_MAIN_TBL_ 6
namespace unc {
namespace upll {
namespace kt_momgr {

BindInfo NwMonitorMoMgr::nwm_bind_info[] = {
    { uudst::vbridge_networkmonitor_group::kDbiVtnName, CFG_KEY, offsetof(
        key_nwm, vbr_key.vtn_key.vtn_name),
      uud::kDalChar, 32 },
    { uudst::vbridge_networkmonitor_group::kDbiVbrName, CFG_KEY, offsetof(
        key_nwm, vbr_key.vbridge_name),
      uud::kDalChar, 32 },
    { uudst::vbridge_networkmonitor_group::kDbiNwmName, CFG_KEY, offsetof(
        key_nwm, nwmonitor_name),
      uud::kDalChar, 32 },
    { uudst::vbridge_networkmonitor_group::kDbiAdminStatus, CFG_VAL, offsetof(
        val_nwm, admin_status),
      uud::kDalUint8, 1 },
    { uudst::vbridge_networkmonitor_group::kDbiOperStatus, ST_VAL, offsetof(
        val_nwm_st, status),
      uud::kDalUint8, 1 },
    { uudst::vbridge_networkmonitor_group::kDbiCtrlrName, CK_VAL, offsetof(
        key_user_data_t, ctrlr_id),
      uud::kDalChar, 32 },
    { uudst::vbridge_networkmonitor_group::kDbiDomainId, CK_VAL, offsetof(
        key_user_data_t, domain_id),
      uud::kDalChar, 32 },
    { uudst::vbridge_networkmonitor_group::kDbiValidAdminStatus, CFG_META_VAL,
      offsetof(val_nwm, valid[UPLL_IDX_ADMIN_STATUS_NWM]), uud::kDalUint8, 1 },
    { uudst::vbridge_networkmonitor_group::kDbiValidOperStatus, ST_META_VAL,
      offsetof(val_nwm_st, valid[UPLL_IDX_STATUS_NWMS]), uud::kDalUint8, 1 },
    { uudst::vbridge_networkmonitor_group::kDbiCsAdminStatus, CS_VAL, offsetof(
        val_nwm, cs_attr[0]),
      uud::kDalUint8, 1 },
    { uudst::vbridge_networkmonitor_group::kDbiCsRowstatus, CS_VAL, offsetof(
        val_nwm, cs_row_status),
      uud::kDalUint8, 1 },
    { uudst::vbridge_networkmonitor_group::kDbiFlags, CK_VAL, offsetof(
        key_user_data, flags),
      uud::kDalUint8, 1 } };

BindInfo NwMonitorMoMgr::key_nwm_maintbl_update_bind_info[] = {
    { uudst::vbridge_networkmonitor_group::kDbiVtnName, CFG_MATCH_KEY, offsetof(
        key_nwm, vbr_key.vtn_key.vtn_name),
      uud::kDalChar, kMaxLenVtnName + 1 },
    { uudst::vbridge_networkmonitor_group::kDbiVbrName, CFG_MATCH_KEY, offsetof(
        key_nwm, vbr_key.vbridge_name),
      uud::kDalChar, kMaxLenVnodeName + 1 },
    { uudst::vbridge_networkmonitor_group::kDbiNwmName, CFG_MATCH_KEY, offsetof(
        key_nwm, nwmonitor_name),
      uud::kDalChar, kMaxLenVnodeName + 1 },
    { uudst::vbridge_networkmonitor_group::kDbiVtnName, CFG_INPUT_KEY, offsetof(
        key_nwm, vbr_key.vtn_key.vtn_name),
      uud::kDalChar, kMaxLenVtnName + 1 },
    { uudst::vbridge_networkmonitor_group::kDbiVbrName, CFG_INPUT_KEY, offsetof(
        key_nwm, vbr_key.vbridge_name),
      uud::kDalChar, kMaxLenVnodeName + 1 },
    { uudst::vbridge_networkmonitor_group::kDbiFlags, CK_VAL, offsetof(
        key_user_data, flags),
      uud::kDalUint8, 1 } };

unc_key_type_t NwMonitorMoMgr::nwm_child[] = { UNC_KT_VBR_NWMONITOR_HOST };

NwMonitorMoMgr::NwMonitorMoMgr() {
  UPLL_FUNC_TRACE
  ntable = MAX_MOMGR_TBLS;
  table = new Table *[ntable];
  table[MAINTBL] = new Table(
      uudst::kDbiVbrNwMonTbl, UNC_KT_VBR_NWMONITOR, nwm_bind_info,
      IpctSt::kIpcStKeyNwm, IpctSt::kIpcStValNwm,
      uudst::vbridge_networkmonitor_group::kDbiVbrNwMonGrpNumCols);
  table[RENAMETBL] = NULL;
  table[CTRLRTBL] = NULL;
  child = nwm_child;
  nchild = sizeof(nwm_child) / sizeof(*nwm_child);
}

/*
 * Based on the key type the bind info will pass
 **/

bool NwMonitorMoMgr::GetRenameKeyBindInfo(unc_key_type_t key_type,
                                          BindInfo *&binfo, int &nattr,
                                          MoMgrTables tbl) {
  /* Main Table or rename table only update */
  if (MAINTBL == tbl) {
    nattr = NUM_KEY_MAIN_TBL_;
    binfo = key_nwm_maintbl_update_bind_info;
  }
  return PFC_TRUE;
}

bool NwMonitorMoMgr::IsValidKey(void *key, uint64_t index) {
  key_nwm *nwm_key = reinterpret_cast<key_nwm *>(key);
  pfc_log_info("Entering IsValidKey");
  bool ret_val = UPLL_RC_SUCCESS;
  switch (index) {
    case uudst::vbridge_networkmonitor_group::kDbiVtnName:
      ret_val = ValidateKey(reinterpret_cast<char *>
                            (nwm_key->vbr_key.vtn_key.vtn_name),
                            kMinLenVtnName, kMaxLenVtnName);
      if (ret_val != UPLL_RC_SUCCESS) {
        pfc_log_info("VTN Name is not valid(%d)", ret_val);
        return false;
      }
      break;
    case uudst::vbridge_networkmonitor_group::kDbiVbrName:
      ret_val = ValidateKey(reinterpret_cast<char *>
                            (nwm_key->vbr_key.vbridge_name),
                            kMinLenVnodeName, kMaxLenVnodeName);
      if (ret_val != UPLL_RC_SUCCESS) {
        pfc_log_info("VBridge Name is not valid(%d)", ret_val);
        return false;
      }
      break;
    case uudst::vbridge_networkmonitor_group::kDbiNwmName:
      ret_val = ValidateKey(reinterpret_cast<char *>
                            (nwm_key->nwmonitor_name), kMinLenNwmName,
                            kMaxLenNwmName);
      if (ret_val != UPLL_RC_SUCCESS) {
        pfc_log_info("NwMonitor Name is not valid(%d)", ret_val);
        return false;
      }
      break;
    default:
      pfc_log_info("Invalid Key Index");
      break;
  }
  pfc_log_info("Leaving IsValidKey");
  return true;
}

upll_rc_t NwMonitorMoMgr::GetChildConfigKey(ConfigKeyVal *&okey,
                                            ConfigKeyVal *parent_key) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  key_nwm *nwm_key;
  void *pkey;
  if (parent_key == NULL) {
    nwm_key = reinterpret_cast<key_nwm *>(malloc(sizeof(key_nwm)));
    if (!nwm_key) return UPLL_RC_ERR_GENERIC;
    memset(nwm_key, 0, sizeof(key_nwm));
    okey = new ConfigKeyVal(UNC_KT_VBR_NWMONITOR, IpctSt::kIpcStKeyNwm, nwm_key,
                            NULL);
    return UPLL_RC_SUCCESS;
  } else {
    pkey = parent_key->get_key();
  }
  if (!pkey) return UPLL_RC_ERR_GENERIC;
  if (okey) {
    if (okey->get_key_type() != UNC_KT_VBR_NWMONITOR)
      return UPLL_RC_ERR_GENERIC;
    nwm_key = reinterpret_cast<key_nwm *>(okey->get_key());
  } else {
    nwm_key = reinterpret_cast<key_nwm *>(malloc(sizeof(key_nwm)));
    if (!nwm_key) return UPLL_RC_ERR_GENERIC;
    memset(nwm_key, 0, sizeof(key_nwm));
  }
  unc_key_type_t keytype = parent_key->get_key_type();
  switch (keytype) {
    case UNC_KT_VTN:
      uuu::upll_strncpy(nwm_key->vbr_key.vtn_key.vtn_name,
             reinterpret_cast<key_vtn *>(pkey)->vtn_name,
             (kMaxLenVtnName + 1));
      break;
    case UNC_KT_VBRIDGE:
      uuu::upll_strncpy(nwm_key->vbr_key.vtn_key.vtn_name,
             reinterpret_cast<key_vbr *>(pkey)->vtn_key.vtn_name,
             (kMaxLenVtnName + 1));
      uuu::upll_strncpy(nwm_key->vbr_key.vbridge_name,
             reinterpret_cast<key_vbr *>(pkey)->vbridge_name,
             (kMaxLenVnodeName + 1));
      break;
    case UNC_KT_VBR_NWMONITOR:
      uuu::upll_strncpy(nwm_key->vbr_key.vtn_key.vtn_name,
             reinterpret_cast<key_nwm *>(pkey)->vbr_key.vtn_key.vtn_name,
             (kMaxLenVtnName + 1));
      uuu::upll_strncpy(nwm_key->vbr_key.vbridge_name,
             reinterpret_cast<key_nwm *>(pkey)->vbr_key.vbridge_name,
             (kMaxLenVnodeName + 1));
      uuu::upll_strncpy(nwm_key->nwmonitor_name,
             reinterpret_cast<key_nwm *>(pkey)->nwmonitor_name,
             (kMaxLenNwmName+1));
    default:
      break;
  }
  if (!okey)
    okey = new ConfigKeyVal(UNC_KT_VBR_NWMONITOR, IpctSt::kIpcStKeyNwm, nwm_key,
                            NULL);
  if (okey == NULL) {
    free(nwm_key);
    result_code = UPLL_RC_ERR_GENERIC;
  } else {
    SET_USER_DATA(okey, parent_key);
  }
  return result_code;
}

upll_rc_t NwMonitorMoMgr::GetParentConfigKey(ConfigKeyVal *&okey,
                                             ConfigKeyVal *ikey) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  if (!ikey) {
    UPLL_LOG_DEBUG("ikey is NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  unc_key_type_t ikey_type = ikey->get_key_type();

  if (ikey_type != UNC_KT_VBR_NWMONITOR) return UPLL_RC_ERR_GENERIC;
  key_nwm *pkey = reinterpret_cast<key_nwm *>(ikey->get_key());
  if (!pkey) return UPLL_RC_ERR_GENERIC;
  key_vbr *vbr_key = reinterpret_cast<key_vbr *>(malloc(sizeof(key_vbr)));
  if (!vbr_key) return UPLL_RC_ERR_GENERIC;
  memset(vbr_key, 0, sizeof(key_vbr));
  uuu::upll_strncpy(vbr_key->vtn_key.vtn_name,
         reinterpret_cast<key_nwm *>(pkey)->vbr_key.vtn_key.vtn_name,
         (kMaxLenVtnName + 1));
  uuu::upll_strncpy(vbr_key->vbridge_name,
         reinterpret_cast<key_nwm *>(pkey)->vbr_key.vbridge_name,
         (kMaxLenVnodeName + 1));
  okey = new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVbr, vbr_key, NULL);
  if (okey == NULL) {
    free(vbr_key);
    result_code = UPLL_RC_ERR_GENERIC;
  } else {
    SET_USER_DATA(okey, ikey);
  }
  return result_code;
}

upll_rc_t NwMonitorMoMgr::AllocVal(ConfigVal *&ck_val,
                                   upll_keytype_datatype_t dt_type,
                                   MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  void *val;
  //  nxt_val;
  // ConfigVal *ck_nxtval;

  if (ck_val != NULL) return UPLL_RC_ERR_GENERIC;
  switch (tbl) {
    case MAINTBL:
      val = malloc(sizeof(val_nwm));
      if (!val) return UPLL_RC_ERR_GENERIC;
      memset(val, 0, sizeof(val_nwm));
      ck_val = new ConfigVal(IpctSt::kIpcStValNwm, val);
      if (dt_type == UPLL_DT_STATE) {
        val = malloc(sizeof(val_nwm_st));
        if (!val) return UPLL_RC_ERR_GENERIC;
        memset(val, 0, sizeof(val_nwm_st));
        ConfigVal *ck_nxtval = new ConfigVal(IpctSt::kIpcStValNwmSt, val);
        ck_val->AppendCfgVal(ck_nxtval);
      }
      break;
    default:
      val = NULL;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t NwMonitorMoMgr::DupConfigKeyVal(ConfigKeyVal *&okey,
                                          ConfigKeyVal *&req, MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  if (req == NULL) return UPLL_RC_ERR_GENERIC;
  if (okey != NULL) return UPLL_RC_ERR_GENERIC;
  if (req->get_key_type() != UNC_KT_VBR_NWMONITOR) return UPLL_RC_ERR_GENERIC;
  ConfigVal *tmp1 = NULL, *tmp = (req)->get_cfg_val();

  if (tmp) {
    if (tbl == MAINTBL) {
      val_nwm *ival = reinterpret_cast<val_nwm *>(GetVal(req));
      val_nwm *nwm_val = reinterpret_cast<val_nwm *>(malloc(sizeof(val_nwm)));
      if (!nwm_val) return UPLL_RC_ERR_GENERIC;
      memcpy(nwm_val, ival, sizeof(val_nwm));
      tmp1 = new ConfigVal(IpctSt::kIpcStValNwm, nwm_val);
    }
    tmp = tmp->get_next_cfg_val();
  };
  if (tmp) {
    if (tbl == MAINTBL) {
      val_nwm_st *ival = reinterpret_cast<val_nwm_st *>(tmp->get_val());
      val_nwm_st *val_nwmst =
          reinterpret_cast<val_nwm_st *>(malloc(sizeof(val_nwm_st)));
      if (!val_nwmst) {
        /* Addressed RESOURCE_LEAK */
        DELETE_IF_NOT_NULL(tmp1);
        return UPLL_RC_ERR_GENERIC;
      }
      memcpy(val_nwmst, ival, sizeof(val_nwm_st));
      ConfigVal *tmp2 = new ConfigVal(IpctSt::kIpcStValNwmSt, val_nwmst);
      tmp1->AppendCfgVal(tmp2);
    }
  };
  void *tkey = (req != NULL) ? (req)->get_key() : NULL;
  key_nwm *ikey = reinterpret_cast<key_nwm *>(tkey);
  key_nwm *nwm_key = reinterpret_cast<key_nwm *>(malloc(sizeof(key_nwm)));
  if (!ikey || !nwm_key) {
    UPLL_LOG_INFO("Invalid params\n");
    /* Addressed RESOURCE_LEAK */
    FREE_IF_NOT_NULL(nwm_key);
    DELETE_IF_NOT_NULL(tmp1);
    return UPLL_RC_ERR_GENERIC;
  }
  memcpy(nwm_key, ikey, sizeof(key_nwm));
  okey = new ConfigKeyVal(UNC_KT_VBR_NWMONITOR, IpctSt::kIpcStKeyNwm, nwm_key,
                          tmp1);
  if (okey) {
    SET_USER_DATA(okey, req);
  } else {
    delete tmp1;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t NwMonitorMoMgr::UpdateConfigStatus(ConfigKeyVal *ikey,
                                             unc_keytype_operation_t op,
                                             uint32_t driver_result,
                                             ConfigKeyVal *upd_key,
                                             DalDmlIntf *dmi,
                                             ConfigKeyVal *ctrlr_key) {
  UPLL_FUNC_TRACE;
  if (ikey == NULL) {
    UPLL_LOG_DEBUG("ConfigKeyVal is NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  val_nwm *nwm_val = reinterpret_cast<val_nwm_t *>(GetVal(ikey));

  unc_keytype_configstatus_t cs_status =
      (driver_result == 0) ? UNC_CS_APPLIED : UNC_CS_NOT_APPLIED;
  if (nwm_val == NULL) return UPLL_RC_ERR_GENERIC;
  nwm_val->cs_row_status = cs_status;
  if (op == UNC_OP_UPDATE) {
    void *val = reinterpret_cast<void*>(nwm_val);
    CompareValidValue(val, GetVal(upd_key), true);
  } else if (op != UNC_OP_CREATE) {
    return UPLL_RC_ERR_GENERIC;
  }
  for (unsigned int loop = 0;
         loop < (sizeof(nwm_val->valid) / sizeof(nwm_val->valid[0]));
        ++loop) {
    if (nwm_val->valid[loop] != UNC_VF_NOT_SOPPORTED) {
      if ((UNC_VF_VALID == (uint8_t) nwm_val->valid[loop])
            || (UNC_VF_VALID_NO_VALUE == (uint8_t) nwm_val->valid[loop]))
         nwm_val->cs_attr[loop] = cs_status;
    } else {
      nwm_val->cs_attr[loop] = UNC_CS_NOT_SUPPORTED;
    }
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t NwMonitorMoMgr::UpdateAuditConfigStatus(
    unc_keytype_configstatus_t cs_status, uuc::UpdateCtrlrPhase phase,
    ConfigKeyVal *&ckv_running) {
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  val_nwm_t *val;
  val = (ckv_running != NULL) ?
      reinterpret_cast<val_nwm_t *>(GetVal(ckv_running)) : NULL;
  if (NULL == val) {
    return UPLL_RC_ERR_GENERIC;
  }
  if (uuc::kUpllUcpCreate == phase) val->cs_row_status = cs_status;
  for (unsigned int loop = 0; loop < sizeof(val->valid) / sizeof(uint8_t);
      ++loop) {
    if ((cs_status == UNC_CS_INVALID &&
        UNC_VF_VALID == val->valid[loop]) || cs_status == UNC_CS_APPLIED) {
      val->cs_attr[loop] = cs_status;
    }
  }
  return result_code;
}

upll_rc_t NwMonitorMoMgr::CopyToConfigKey(ConfigKeyVal *&okey,
                                          ConfigKeyVal *ikey) {
  if (!ikey || !(ikey->get_key())) return UPLL_RC_ERR_GENERIC;

  upll_rc_t result_code = UPLL_RC_SUCCESS;

  key_rename_vnode_info *key_rename =
      reinterpret_cast<key_rename_vnode_info *>(ikey->get_key());
  key_nwm_t * key_vwm = new key_nwm_t();
  if (!key_vwm) return UPLL_RC_ERR_GENERIC;
  memset(key_vwm, 0 , sizeof(key_nwm_t));
  if (!strlen(reinterpret_cast<char *>(key_rename->old_unc_vtn_name))) {
    /* Addressed RESOURCE_LEAK */
    DELETE_IF_NOT_NULL(key_vwm);
    return UPLL_RC_ERR_GENERIC;
  }
  uuu::upll_strncpy(key_vwm->vbr_key.vtn_key.vtn_name,
         key_rename->old_unc_vtn_name, (kMaxLenVtnName + 1));
  if (ikey->get_key_type() == UNC_KT_VBRIDGE) {
    if (!strlen(reinterpret_cast<char *>(key_rename->old_unc_vnode_name))) {
      /* Addressed RESOURCE_LEAK */
      DELETE_IF_NOT_NULL(key_vwm);
      return UPLL_RC_ERR_GENERIC;
    }
    uuu::upll_strncpy(key_vwm->vbr_key.vbridge_name,
           key_rename->old_unc_vnode_name, (kMaxLenVnodeName + 1));
  }
  okey = new ConfigKeyVal(UNC_KT_VBR_NWMONITOR, IpctSt::kIpcStKeyNwm, key_vwm,
                          NULL);
  if (!okey) return UPLL_RC_ERR_GENERIC;

  return result_code;
}

bool NwMonitorMoMgr::FilterAttributes(void *&val1, void *val2,
                                      bool audit_status,
                                      unc_keytype_operation_t op) {
  UPLL_FUNC_TRACE;
  if (op != UNC_OP_CREATE)
    return CompareValidValue(val1, val2, audit_status);
  return false;
}

bool NwMonitorMoMgr::CompareValidValue(void *&val1, void *val2, bool copy_to_running) {
  UPLL_FUNC_TRACE;
  bool invalid_attr = true;
  val_nwm_t *val_nwm1 = reinterpret_cast<val_nwm_t*>(val1);
  val_nwm_t *val_nwm2 = reinterpret_cast<val_nwm_t*>(val2);
  for (unsigned int loop = 0;
      loop < sizeof(val_nwm1->valid) / sizeof(uint8_t); ++loop) {
    if (UNC_VF_INVALID == val_nwm1->valid[loop]
        && UNC_VF_VALID == val_nwm2->valid[loop]) 
      val_nwm1->valid[loop] = UNC_VF_VALID_NO_VALUE;
  }
  if (val_nwm1->valid[UPLL_IDX_ADMIN_STATUS_NWM] == UNC_VF_VALID
      && val_nwm2->valid[UPLL_IDX_ADMIN_STATUS_NWM] == UNC_VF_VALID) {
    if (val_nwm1->admin_status == val_nwm2->admin_status)
      val_nwm1->valid[UPLL_IDX_ADMIN_STATUS_NWM] = UNC_VF_INVALID;
  }
  for (unsigned int loop = 0;
       loop < (sizeof(val_nwm1->valid) / sizeof(val_nwm1->valid[0]));
      ++loop) {
    if ((UNC_VF_VALID == (uint8_t) val_nwm1->valid[loop])||
        (UNC_VF_VALID_NO_VALUE == (uint8_t) val_nwm1->valid[loop]))
         invalid_attr = false;
  }
  return invalid_attr;	
}

upll_rc_t NwMonitorMoMgr::ValidateMessage(IpcReqRespHeader *req,
                                          ConfigKeyVal *ikey) {
  UPLL_FUNC_TRACE;
  upll_rc_t ret_val = UPLL_RC_ERR_GENERIC;
  if (NULL == req) {
    UPLL_LOG_DEBUG("IpcReqRespHeader is NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  if (ikey == NULL) {
    UPLL_LOG_DEBUG("ConfigKeyVal is NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  unc_key_type_t keytype = ikey->get_key_type();
  if (UNC_KT_VBR_NWMONITOR != keytype) {
    UPLL_LOG_DEBUG("Invalid keytype. Keytype- %d", keytype);
    return UPLL_RC_ERR_GENERIC;
  }
  if (ikey->get_st_num() != IpctSt::kIpcStKeyNwm) {
    UPLL_LOG_DEBUG(
        "Invalid structure received.Expected struct-kIpcStKeyNwm,"
        "received struct -%d ", (ikey->get_st_num()));
    return UPLL_RC_ERR_GENERIC;
  }
  key_nwm_t *key_nwm = reinterpret_cast<key_nwm_t *>(ikey->get_key());
  val_nwm_t *val_nwm = NULL;
  if ((ikey->get_cfg_val())
      && ((ikey->get_cfg_val())->get_st_num() == IpctSt::kIpcStValNwm)) {
    val_nwm = reinterpret_cast<val_nwm_t *>(ikey->get_cfg_val()->get_val());
  }

  upll_keytype_datatype_t dt_type = req->datatype;
  unc_keytype_operation_t operation = req->operation;
  unc_keytype_option1_t option1 = req->option1;
  unc_keytype_option2_t option2 = req->option2;

  if (key_nwm == NULL) {
    pfc_log_debug("key structure is empty!!");
    return UPLL_RC_ERR_NO_SUCH_INSTANCE;
  }
  ret_val = ValidateNwMonKey(key_nwm, operation);
  if (ret_val != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Syntax check failed for KT_VBR_NWMONITOR keytype");
    return UPLL_RC_ERR_CFG_SYNTAX;
  } else {
    if (((operation == UNC_OP_CREATE) || (operation == UNC_OP_UPDATE))
        && ((dt_type == UPLL_DT_CANDIDATE)|| (UPLL_DT_IMPORT == dt_type))) {
      if (val_nwm != NULL) {
        ret_val = ValidateNwMonValue(val_nwm, operation);
        if (ret_val != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("Syntax check failed for NWMONITOR value struct");
          return UPLL_RC_ERR_CFG_SYNTAX;
        } else {
          UPLL_LOG_DEBUG("Syntax check success for NWMONITOR value struct");
          return UPLL_RC_SUCCESS;
        }
      } else {
        UPLL_LOG_DEBUG("Value structure mandatory CREATE/UPDATE");
        return UPLL_RC_ERR_CFG_SYNTAX;
      }

    } else if (((operation == UNC_OP_READ) ||
          (operation == UNC_OP_READ_SIBLING) ||
          (operation == UNC_OP_READ_SIBLING_BEGIN) ||
          (operation == UNC_OP_READ_SIBLING_COUNT)) &&
        ((dt_type == UPLL_DT_CANDIDATE) ||
         (dt_type == UPLL_DT_RUNNING) ||
         (dt_type == UPLL_DT_STARTUP) ||
          (dt_type == UPLL_DT_STATE))) {
      if ((option1 == UNC_OPT1_NORMAL) || ((option1 == UNC_OPT1_DETAIL) &&
         (operation != UNC_OP_READ_SIBLING_COUNT))) {
        if (option2 == UNC_OPT2_NONE) {
          if (val_nwm != NULL) {
            ret_val = ValidateNwMonValue(val_nwm);
            if (ret_val != UPLL_RC_SUCCESS) {
              pfc_log_debug("Syntax check failed for NWMONITOR value struct");
              return UPLL_RC_ERR_CFG_SYNTAX;
            } else {
              pfc_log_trace("Syntax check success for NWMONITOR value struct");
              return UPLL_RC_SUCCESS;
            }
          } else {
            pfc_log_trace("value structure is optional");
            return UPLL_RC_SUCCESS;
          }
        } else {
          pfc_log_debug("option2 is not matching");
          return UPLL_RC_ERR_INVALID_OPTION2;
        }
      } else {
        pfc_log_debug("option1 is not matching");
        return UPLL_RC_ERR_INVALID_OPTION1;
      }

    } else if ((operation == UNC_OP_DELETE) || (operation == UNC_OP_READ_NEXT)
        || (operation == UNC_OP_READ_BULK)) {
      pfc_log_trace("Value structure is none for operation type:%d", operation);
      return UPLL_RC_SUCCESS;
    } else {
      pfc_log_debug("Invalid datatype(%d) and operation(%d)", dt_type,
                    operation);
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  }
}
upll_rc_t NwMonitorMoMgr::ValidateNwMonValue(val_nwm_t *val_nwm,
    unc_keytype_operation_t operation) {
  UPLL_FUNC_TRACE;

  if (val_nwm->valid[UPLL_IDX_ADMIN_STATUS_NWM] == UNC_VF_VALID) {
    if (!ValidateNumericRange(val_nwm->admin_status,
                              (uint8_t) UPLL_ADMIN_ENABLE,
                              (uint8_t) UPLL_ADMIN_DISABLE, true, true)) {
      pfc_log_debug("Syntax check failed admin_stat-%d", val_nwm->admin_status);
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  } else if ((val_nwm->valid[UPLL_IDX_ADMIN_STATUS_NWM] ==
              UNC_VF_VALID_NO_VALUE)
      && ((operation == UNC_OP_UPDATE) || (operation == UNC_OP_CREATE))) {
    /* Copy default value */
    val_nwm->admin_status = UPLL_ADMIN_ENABLE;
  }
  pfc_log_trace("value structure validation successful for VTUNNEL_IF keytype");
  return UPLL_RC_SUCCESS;
}
upll_rc_t NwMonitorMoMgr::ValidateNwMonKey(key_nwm_t *key_nwm,
    unc_keytype_operation_t operation) {
  UPLL_FUNC_TRACE;

  upll_rc_t ret_val = UPLL_RC_SUCCESS;
  VbrMoMgr *objvbrmgr =
      reinterpret_cast<VbrMoMgr *>(const_cast<MoManager *>(GetMoManager(
          UNC_KT_VBRIDGE)));
  if (NULL == objvbrmgr) {
    UPLL_LOG_DEBUG("unable to get VbrMoMgr object to validate key_vbr");
    return UPLL_RC_ERR_GENERIC;
  }
  ret_val = objvbrmgr->ValidateVbrKey(&(key_nwm->vbr_key));

  if (UPLL_RC_SUCCESS != ret_val) {
    UPLL_LOG_DEBUG("Syntax check failed.Err Code- %d", ret_val);
    return ret_val;
  }
  if ((operation != UNC_OP_READ_SIBLING_BEGIN) &&
      (operation != UNC_OP_READ_SIBLING_COUNT)) {
    ret_val = ValidateKey(reinterpret_cast<char *>(key_nwm->nwmonitor_name),
        kMinLenVnodeName, kMaxLenVnodeName);

    if (ret_val != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Syntax check failed.nwmonitor_name-(%s)",
          key_nwm->nwmonitor_name);
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  } else {
    UPLL_LOG_TRACE("Operation is %d", operation);
    StringReset(key_nwm->nwmonitor_name);
  }
  return ret_val;
}

upll_rc_t NwMonitorMoMgr::ValNwMonAttributeSupportCheck(const char * crtlr_name,
                                                        ConfigKeyVal *ikey,
                                                        uint32_t operation) {
  UPLL_FUNC_TRACE;
  bool result_code = false;
  uint32_t max_instance_count = 0;
  uint32_t num_attrs = 0;
  const uint8_t *attrs = 0;
  if (ikey == NULL) {
    UPLL_LOG_DEBUG("ConfigKeyVal is NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  val_nwm_t *val_nwm = NULL;
  if ((ikey->get_cfg_val())
      && ((ikey->get_cfg_val())->get_st_num() == IpctSt::kIpcStValNwm)) {
    val_nwm = reinterpret_cast<val_nwm_t *>(ikey->get_cfg_val()->get_val());
  }
  if (val_nwm != NULL) {
    switch (operation) {
      case UNC_OP_CREATE:
        result_code = GetCreateCapability(crtlr_name, ikey->get_key_type(),
                                          &max_instance_count, &num_attrs,
                                          &attrs);
        if (result_code && cur_instance_count >= max_instance_count) {
          pfc_log_debug("[%s:%d:%s Instance count %d exceeds %d", __FILE__,
                        __LINE__, __FUNCTION__, cur_instance_count,
                        max_instance_count);
          return UPLL_RC_ERR_EXCEEDS_RESOURCE_LIMIT;
        }
        break;

      case UNC_OP_UPDATE:
        result_code = GetUpdateCapability(crtlr_name, ikey->get_key_type(),
                                          &num_attrs, &attrs);
        break;

      case UNC_OP_READ:
      case UNC_OP_READ_SIBLING:
      case UNC_OP_READ_SIBLING_BEGIN:
      case UNC_OP_READ_SIBLING_COUNT:
        result_code = GetReadCapability(crtlr_name, ikey->get_key_type(),
                                        &num_attrs, &attrs);
        break;

      default:
        pfc_log_debug("Invalid operation");
        break;
    }

    if (!result_code) {
      pfc_log_debug("key_type - %d is not supported by controller - %s",
                    ikey->get_key_type(), crtlr_name);
      return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_KT;
    }
    if ((val_nwm->valid[UPLL_IDX_ADMIN_STATUS_NWM] == UNC_VF_VALID) ||
        (val_nwm->valid[UPLL_IDX_ADMIN_STATUS_NWM] == UNC_VF_VALID_NO_VALUE)) {
      if (attrs[unc::capa::nwm::kCapAdminStatus] == 0) {
        val_nwm->valid[UPLL_IDX_ADMIN_STATUS_NWM] = UNC_VF_NOT_SOPPORTED;
        pfc_log_debug(
            "UPLL_IDX_ADMIN_STATUS_NWM not supported in pfc controller");
        return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
      }
    }
  } else {
    pfc_log_debug("Value structure is empty");
    return UPLL_RC_ERR_GENERIC;
  }
  pfc_log_info("Exiting KT_VBR_NWMONITOR: ValNwMonAttributeSupportCheck()");
  return UPLL_RC_SUCCESS;
}
upll_rc_t NwMonitorMoMgr::ValidateCapability(IpcReqRespHeader *req,
                                             ConfigKeyVal *ikey,
                                             const char * ctrlr_name) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  if (NULL == req) {
    UPLL_LOG_DEBUG("IpcReqRespHeader is NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  if (ikey == NULL) {
    UPLL_LOG_DEBUG("ConfigKeyVal is NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  if (!ctrlr_name) ctrlr_name = reinterpret_cast<char *>(ikey->get_user_data());
  pfc_log_trace("controller name (%s)", ctrlr_name);

  upll_keytype_datatype_t dt_type = req->datatype;
  unc_keytype_operation_t operation = req->operation;
  unc_keytype_option1_t option1 = req->option1;
  unc_keytype_option2_t option2 = req->option2;
  pfc_log_trace("dt_type   : (%d)"
                "operation : (%d)"
                "option1   : (%d)"
                "option2   : (%d)",
                dt_type, operation, option1, option2);
  if (((operation == UNC_OP_CREATE) || (operation == UNC_OP_UPDATE))
      && (dt_type == UPLL_DT_CANDIDATE)) {
    if ((GetVal(ikey)) != NULL) {
      result_code = ValNwMonAttributeSupportCheck(ctrlr_name, ikey, operation);
      if (result_code == UPLL_RC_SUCCESS) {
        pfc_log_trace("Attribute check success for KT_VBR_NWMONITOR");
        return UPLL_RC_SUCCESS;
      } else {
        pfc_log_debug("Attribute check failed for KT_VBR_NWMONITOR");
        return UPLL_RC_ERR_CFG_SEMANTIC;
      }
    } else {
      pfc_log_debug("value structure mandatory");
      return UPLL_RC_ERR_CFG_SEMANTIC;
    }
  } else if (((operation == UNC_OP_READ) ||
          (operation == UNC_OP_READ_SIBLING) ||
          (operation == UNC_OP_READ_SIBLING_BEGIN) ||
          (operation == UNC_OP_READ_SIBLING_COUNT)) &&
        ((dt_type == UPLL_DT_CANDIDATE) ||
         (dt_type == UPLL_DT_RUNNING) ||
         (dt_type == UPLL_DT_STARTUP) ||
          (dt_type == UPLL_DT_STATE))) {
      if ((option1 == UNC_OPT1_NORMAL) || ((option1 == UNC_OPT1_DETAIL) &&
         (operation != UNC_OP_READ_SIBLING_COUNT))) {
        if (option2 == UNC_OPT2_NONE) {
        if ((GetVal(ikey)) != NULL) {
          result_code = ValNwMonAttributeSupportCheck(ctrlr_name, ikey,
                                                      operation);
          if (result_code == UPLL_RC_SUCCESS) {
            pfc_log_trace("Attribute check success for KT_VBR_NWMONITOR");
            return UPLL_RC_SUCCESS;
          } else {
            pfc_log_debug("Attribute check failed for KT_VBR_NWMONITOR");
            return UPLL_RC_ERR_CFG_SEMANTIC;
          }
        } else {
          pfc_log_trace("value structure is optional");
          return UPLL_RC_SUCCESS;
        }
      } else {
        pfc_log_debug("option2 is not matching");
        return UPLL_RC_ERR_INVALID_OPTION2;
      }
    } else {
      pfc_log_debug("option1 is not matching");
      return UPLL_RC_ERR_INVALID_OPTION1;
    }

  } else if ((operation == UNC_OP_DELETE) || (operation == UNC_OP_READ_NEXT)
      || (operation == UNC_OP_READ_BULK)) {
    pfc_log_trace("Value structure is none for operation type:%d", operation);
    return UPLL_RC_SUCCESS;
  } else {
    pfc_log_debug("Invalid datatype(%d) and operation(%d)", dt_type, operation);
    return UPLL_RC_ERR_CFG_SYNTAX;
  }
}

upll_rc_t NwMonitorMoMgr::IsReferenced(ConfigKeyVal *ikey,
                                       upll_keytype_datatype_t dt_type,
                                       DalDmlIntf *dmi) {
  return UPLL_RC_SUCCESS;
}

}  // namespace kt_momgr
}  // namespace upll
}  // namespace unc
