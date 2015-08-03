/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include "nwm_momgr.hh"
#include "uncxx/upll_log.hh"
#include "vbr_momgr.hh"
#include "vtn_momgr.hh"

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
        key_rename_vnode_info_t, new_unc_vtn_name),
      uud::kDalChar, kMaxLenVtnName + 1 },
    { uudst::vbridge_networkmonitor_group::kDbiVbrName, CFG_INPUT_KEY, offsetof(
        key_rename_vnode_info_t, new_unc_vnode_name),
      uud::kDalChar, kMaxLenVnodeName + 1 },
    { uudst::vbridge_networkmonitor_group::kDbiFlags, CK_VAL, offsetof(
        key_user_data, flags),
      uud::kDalUint8, 1 } };

unc_key_type_t NwMonitorMoMgr::nwm_child[] = { UNC_KT_VBR_NWMONITOR_HOST };

NwMonitorMoMgr::NwMonitorMoMgr() {
  UPLL_FUNC_TRACE
  ntable = MAX_MOMGR_TBLS;
  table = new Table *[ntable]();
  table[MAINTBL] = new Table(
      uudst::kDbiVbrNwMonTbl, UNC_KT_VBR_NWMONITOR, nwm_bind_info,
      IpctSt::kIpcStKeyNwm, IpctSt::kIpcStValNwm,
      uudst::vbridge_networkmonitor_group::kDbiVbrNwMonGrpNumCols);
  table[RENAMETBL] = NULL;
  table[CTRLRTBL] = NULL;
  table[CONVERTTBL] = NULL;
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

bool NwMonitorMoMgr::IsValidKey(void *key, uint64_t index,
                                MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  key_nwm *nwm_key = reinterpret_cast<key_nwm *>(key);
  upll_rc_t ret_val = UPLL_RC_SUCCESS;
  switch (index) {
    case uudst::vbridge_networkmonitor_group::kDbiVtnName:
      ret_val = ValidateKey(reinterpret_cast<char *>
                            (nwm_key->vbr_key.vtn_key.vtn_name),
                            kMinLenVtnName, kMaxLenVtnName);
      if (ret_val != UPLL_RC_SUCCESS) {
        UPLL_LOG_TRACE("VTN Name is not valid(%d)", ret_val);
        return false;
      }
      break;
    case uudst::vbridge_networkmonitor_group::kDbiVbrName:
      ret_val = ValidateKey(reinterpret_cast<char *>
                            (nwm_key->vbr_key.vbridge_name),
                            kMinLenVnodeName, kMaxLenVnodeName);
      if (ret_val != UPLL_RC_SUCCESS) {
        UPLL_LOG_TRACE("VBridge Name is not valid(%d)", ret_val);
        return false;
      }
      break;
    case uudst::vbridge_networkmonitor_group::kDbiNwmName:
      ret_val = ValidateKey(reinterpret_cast<char *>
                            (nwm_key->nwmonitor_name), kMinLenNwmName,
                            kMaxLenNwmName);
      if (ret_val != UPLL_RC_SUCCESS) {
        UPLL_LOG_TRACE("NwMonitor Name is not valid(%d)", ret_val);
        return false;
      }
      break;
    default:
      UPLL_LOG_INFO("Invalid Key Index");
      break;
  }
  return true;
}

upll_rc_t NwMonitorMoMgr::GetChildConfigKey(ConfigKeyVal *&okey,
                                            ConfigKeyVal *parent_key) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  key_nwm *nwm_key;
  void *pkey;
  if (parent_key == NULL) {
    nwm_key = reinterpret_cast<key_nwm *>(
        ConfigKeyVal::Malloc(sizeof(key_nwm)));
    if (okey) delete okey;
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
    nwm_key = reinterpret_cast<key_nwm *>(
        ConfigKeyVal::Malloc(sizeof(key_nwm)));
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
  else if (okey->get_key() != nwm_key)
    okey->SetKey(IpctSt::kIpcStKeyNwm, nwm_key);
  if (okey == NULL) {
    free(nwm_key);
    return UPLL_RC_ERR_GENERIC;
  }
  SET_USER_DATA(okey, parent_key);
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
  key_vbr *vbr_key = reinterpret_cast<key_vbr *>(
      ConfigKeyVal::Malloc(sizeof(key_vbr)));
  uuu::upll_strncpy(vbr_key->vtn_key.vtn_name,
         reinterpret_cast<key_nwm *>(pkey)->vbr_key.vtn_key.vtn_name,
         (kMaxLenVtnName + 1));
  uuu::upll_strncpy(vbr_key->vbridge_name,
         reinterpret_cast<key_nwm *>(pkey)->vbr_key.vbridge_name,
         (kMaxLenVnodeName + 1));
  if (okey) delete okey;
  okey = new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVbr, vbr_key, NULL);
  SET_USER_DATA(okey, ikey);
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
      val = ConfigKeyVal::Malloc(sizeof(val_nwm));
      ck_val = new ConfigVal(IpctSt::kIpcStValNwm, val);
      if (dt_type == UPLL_DT_STATE) {
        val = ConfigKeyVal::Malloc(sizeof(val_nwm_st));
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
      val_nwm *nwm_val = reinterpret_cast<val_nwm *>(
          ConfigKeyVal::Malloc(sizeof(val_nwm)));
      memcpy(nwm_val, ival, sizeof(val_nwm));
      tmp1 = new ConfigVal(IpctSt::kIpcStValNwm, nwm_val);
    }
    tmp = tmp->get_next_cfg_val();
  };
  if (tmp) {
    if (tbl == MAINTBL) {
      val_nwm_st *ival = reinterpret_cast<val_nwm_st *>(tmp->get_val());
      val_nwm_st *val_nwmst = reinterpret_cast<val_nwm_st *>(
          ConfigKeyVal::Malloc(sizeof(val_nwm_st)));
      memcpy(val_nwmst, ival, sizeof(val_nwm_st));
      ConfigVal *tmp2 = new ConfigVal(IpctSt::kIpcStValNwmSt, val_nwmst);
      if (tmp1)
        tmp1->AppendCfgVal(tmp2);
    }
  };
  void *tkey = (req != NULL) ? (req)->get_key() : NULL;
  key_nwm *ikey = reinterpret_cast<key_nwm *>(tkey);
  key_nwm *nwm_key = reinterpret_cast<key_nwm *>(
      ConfigKeyVal::Malloc(sizeof(key_nwm)));
  if (!ikey || !nwm_key) {
    UPLL_LOG_INFO("Invalid params");
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
    DELETE_IF_NOT_NULL(tmp1);
    FREE_IF_NOT_NULL(nwm_key);
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
  val_nwm *nwm_val2 = reinterpret_cast<val_nwm *>(GetVal(upd_key));
  UPLL_LOG_TRACE("Key in Candidate %s", (ikey->ToStrAll()).c_str());

  unc_keytype_configstatus_t cs_status =
      (driver_result == 0) ? UNC_CS_APPLIED : UNC_CS_NOT_APPLIED;
  if (nwm_val == NULL) return UPLL_RC_ERR_GENERIC;
  if (op == UNC_OP_CREATE) {
    nwm_val->cs_row_status = cs_status;
  } else if (op == UNC_OP_UPDATE) {
    void *val = reinterpret_cast<void*>(nwm_val);
    CompareValidValue(val, GetVal(upd_key), true);
    UPLL_LOG_TRACE("Key in Running %s", (upd_key->ToStrAll()).c_str());
    nwm_val->cs_row_status = nwm_val2->cs_row_status;
  } else {
    return UPLL_RC_ERR_GENERIC;
  }
  for (unsigned int loop = 0;
         loop < (sizeof(nwm_val->valid) / sizeof(nwm_val->valid[0]));
        ++loop) {
    if ((UNC_VF_VALID == (uint8_t) nwm_val->valid[loop])
            || (UNC_VF_VALID_NO_VALUE == (uint8_t) nwm_val->valid[loop]))
         nwm_val->cs_attr[loop] = cs_status;
    else if ((UNC_VF_INVALID == nwm_val->valid[loop]) &&
               (UNC_OP_CREATE == op))
         nwm_val->cs_attr[loop] = UNC_CS_NOT_APPLIED;
    else if ((UNC_VF_INVALID == nwm_val->valid[loop]) &&
               (UNC_OP_UPDATE == op))
         nwm_val->cs_attr[loop] = nwm_val2->cs_attr[loop];
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t NwMonitorMoMgr::UpdateAuditConfigStatus(
    unc_keytype_configstatus_t cs_status,
    uuc::UpdateCtrlrPhase phase,
    ConfigKeyVal *&ckv_running,
    DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  val_nwm_t *val;
  val = (ckv_running != NULL) ?
      reinterpret_cast<val_nwm_t *>(GetVal(ckv_running)) : NULL;
  if (NULL == val) {
    return UPLL_RC_ERR_GENERIC;
  }
  if (uuc::kUpllUcpCreate == phase) val->cs_row_status = cs_status;
  if ((uuc::kUpllUcpUpdate == phase) &&
           (val->cs_row_status == UNC_CS_INVALID ||
            val->cs_row_status == UNC_CS_NOT_APPLIED))
    val->cs_row_status = cs_status;
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
  key_nwm_t *key_vwm = reinterpret_cast<key_nwm_t*>(
      ConfigKeyVal::Malloc(sizeof(key_nwm_t)));
  if (!strlen(reinterpret_cast<char *>(key_rename->old_unc_vtn_name))) {
    /* Addressed RESOURCE_LEAK */
    ConfigKeyVal::Free(key_vwm);
    return UPLL_RC_ERR_GENERIC;
  }
  uuu::upll_strncpy(key_vwm->vbr_key.vtn_key.vtn_name,
         key_rename->old_unc_vtn_name, (kMaxLenVtnName + 1));
  if (ikey->get_key_type() == UNC_KT_VBRIDGE) {
    if (!strlen(reinterpret_cast<char *>(key_rename->old_unc_vnode_name))) {
      /* Addressed RESOURCE_LEAK */
      ConfigKeyVal::Free(key_vwm);
      return UPLL_RC_ERR_GENERIC;
    }
    uuu::upll_strncpy(key_vwm->vbr_key.vbridge_name,
           key_rename->old_unc_vnode_name, (kMaxLenVnodeName + 1));
  } else {
    if (!strlen(reinterpret_cast<char *>(key_rename->new_unc_vnode_name))) {
      ConfigKeyVal::Free(key_vwm);
      return UPLL_RC_ERR_GENERIC;
    }
    uuu::upll_strncpy(key_vwm->vbr_key.vbridge_name,
       key_rename->new_unc_vnode_name, (kMaxLenVnodeName+1));
  }

  okey = new ConfigKeyVal(UNC_KT_VBR_NWMONITOR, IpctSt::kIpcStKeyNwm, key_vwm,
                          NULL);

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

bool NwMonitorMoMgr::CompareValidValue(void *&val1, void *val2,
                                       bool copy_to_running) {
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
  if (val_nwm1->valid[UPLL_IDX_ADMIN_STATUS_NWM] != UNC_VF_INVALID
      && val_nwm2->valid[UPLL_IDX_ADMIN_STATUS_NWM] != UNC_VF_INVALID) {
    if (val_nwm1->admin_status == val_nwm2->admin_status)
      val_nwm1->valid[UPLL_IDX_ADMIN_STATUS_NWM] = UNC_VF_INVALID;
  }
  for (unsigned int loop = 0;
       loop < (sizeof(val_nwm1->valid) / sizeof(val_nwm1->valid[0]));
      ++loop) {
    if ((UNC_VF_VALID == (uint8_t) val_nwm1->valid[loop])||
        (UNC_VF_VALID_NO_VALUE == (uint8_t) val_nwm1->valid[loop])) {
      invalid_attr = false;
      break;
    }
  }
  return invalid_attr;
}

upll_rc_t NwMonitorMoMgr::ValidateMessage(IpcReqRespHeader *req,
    ConfigKeyVal *ikey) {
  UPLL_FUNC_TRACE;
  upll_rc_t ret_val = UPLL_RC_ERR_GENERIC;
  if (!ikey || !req || !(ikey->get_key())) {
    UPLL_LOG_DEBUG("IpcReqRespHeader or ConfigKeyVal is Null");
    return UPLL_RC_ERR_BAD_REQUEST;
  }
  unc_key_type_t keytype = ikey->get_key_type();
  if (UNC_KT_VBR_NWMONITOR != keytype) {
    UPLL_LOG_DEBUG("Invalid keytype. Keytype- %d", keytype);
    return UPLL_RC_ERR_BAD_REQUEST;
  }
  if (ikey->get_st_num() != IpctSt::kIpcStKeyNwm) {
    UPLL_LOG_DEBUG(
        "Invalid structure received.Expected struct-kIpcStKeyNwm,"
        "received struct -%d ", (ikey->get_st_num()));
    return UPLL_RC_ERR_BAD_REQUEST;
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
    UPLL_LOG_DEBUG("key structure is empty!!");
    return UPLL_RC_ERR_BAD_REQUEST;
  }
  ret_val = ValidateNwMonKey(key_nwm, operation);
  if (ret_val != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Syntax check failed for KT_VBR_NWMONITOR keytype");
    return UPLL_RC_ERR_CFG_SYNTAX;
  } else {
    if (((operation == UNC_OP_CREATE) || (operation == UNC_OP_UPDATE))
        && ((dt_type == UPLL_DT_CANDIDATE)|| (dt_type == UPLL_DT_IMPORT))) {
      if (val_nwm != NULL) {
        ret_val = ValidateNwMonValue(val_nwm, operation);
        if (ret_val != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("Syntax check failed for NWMONITOR value struct");
          return UPLL_RC_ERR_CFG_SYNTAX;
        }
        return UPLL_RC_SUCCESS;
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
      if ((option1 == UNC_OPT1_NORMAL) ||
          ((option1 == UNC_OPT1_DETAIL) &&
           (operation != UNC_OP_READ_SIBLING_COUNT) &&
           (dt_type == UPLL_DT_STATE))) {
        if (option2 == UNC_OPT2_NONE) {
          if (val_nwm != NULL) {
            ret_val = ValidateNwMonValue(val_nwm);
            if (ret_val != UPLL_RC_SUCCESS) {
              UPLL_LOG_DEBUG("Syntax check failed for NWMONITOR value struct");
              return UPLL_RC_ERR_CFG_SYNTAX;
            } else {
              UPLL_LOG_TRACE("Syntax check success for NWMONITOR value struct");
              return UPLL_RC_SUCCESS;
            }
          } else {
            UPLL_LOG_TRACE("value structure is optional");
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

    } else if ((operation == UNC_OP_DELETE) || (operation == UNC_OP_READ_NEXT)
        || (operation == UNC_OP_READ_BULK)) {
      UPLL_LOG_TRACE("Value structure is none for operation type:%d",
                     operation);
      return UPLL_RC_SUCCESS;
    } else {
      UPLL_LOG_DEBUG("Invalid datatype(%d) and operation(%d)", dt_type,
          operation);
      return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT;
    }
  }
  return UPLL_RC_SUCCESS;
}
upll_rc_t NwMonitorMoMgr::ValidateNwMonValue(val_nwm_t *val_nwm,
    unc_keytype_operation_t operation) {
  UPLL_FUNC_TRACE;

  if (val_nwm->valid[UPLL_IDX_ADMIN_STATUS_NWM] == UNC_VF_VALID) {
    if (!ValidateNumericRange(val_nwm->admin_status,
          (uint8_t) UPLL_ADMIN_ENABLE,
          (uint8_t) UPLL_ADMIN_DISABLE, true, true)) {
      UPLL_LOG_DEBUG("Syntax check failed admin_stat-%d",
                     val_nwm->admin_status);
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  } else if ((val_nwm->valid[UPLL_IDX_ADMIN_STATUS_NWM] ==
        UNC_VF_VALID_NO_VALUE)
      && ((operation == UNC_OP_UPDATE) || (operation == UNC_OP_CREATE))) {
    /* Copy default value */
    val_nwm->admin_status = UPLL_ADMIN_ENABLE;
  }
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

upll_rc_t NwMonitorMoMgr::ValNwMonAttributeSupportCheck(val_nwm_t *val_nwm,
    const uint8_t* attrs, unc_keytype_operation_t operation) {
  UPLL_FUNC_TRACE;
  if ((val_nwm->valid[UPLL_IDX_ADMIN_STATUS_NWM] == UNC_VF_VALID) ||
      (val_nwm->valid[UPLL_IDX_ADMIN_STATUS_NWM] == UNC_VF_VALID_NO_VALUE)) {
    if (attrs[unc::capa::nwm::kCapAdminStatus] == 0) {
      val_nwm->valid[UPLL_IDX_ADMIN_STATUS_NWM] = UNC_VF_INVALID;
      if (operation == UNC_OP_CREATE || operation == UNC_OP_UPDATE) {
        UPLL_LOG_DEBUG(
            "UPLL_IDX_ADMIN_STATUS_NWM not supported in pfc controller");
        return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
      }
    }
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t NwMonitorMoMgr::ValidateCapability(IpcReqRespHeader *req,
    ConfigKeyVal *ikey,
    const char * ctrlr_name) {
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
    case UNC_OP_CREATE: {
      result_code = GetCreateCapability(ctrlr_name, ikey->get_key_type(),
                                        &max_instance_count, &max_attrs,
                                        &attrs);
      break;
    }

    case UNC_OP_UPDATE: {
      result_code = GetUpdateCapability(ctrlr_name, ikey->get_key_type(),
                                        &max_attrs, &attrs);
      break;
    }

    case UNC_OP_READ:
    case UNC_OP_READ_SIBLING:
    case UNC_OP_READ_SIBLING_BEGIN:
    case UNC_OP_READ_SIBLING_COUNT: {
      result_code = GetReadCapability(ctrlr_name, ikey->get_key_type(),
                                      &max_attrs, &attrs);
      break;
    }
    default: {
      UPLL_LOG_DEBUG("Invalid operation");
      break;
    }
  }
  if (!result_code) {
    UPLL_LOG_DEBUG("keytype(%d) is not supported by controller(%s) "
                   "for operation(%d)",
                   ikey->get_key_type(), ctrlr_name, req->operation);
    return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
  }
  val_nwm_t *val_nwm = NULL;
  if ((ikey->get_cfg_val())
      && ((ikey->get_cfg_val())->get_st_num() == IpctSt::kIpcStValNwm)) {
    val_nwm = reinterpret_cast<val_nwm_t *>(ikey->get_cfg_val()->get_val());
  }
  if (val_nwm) {
    if (max_attrs > 0) {
      return ValNwMonAttributeSupportCheck(val_nwm, attrs, req->operation);
    } else {
      UPLL_LOG_DEBUG("Attribute list is empty for operation %d",
                     req->operation);
      return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
    }
  }
  return UPLL_RC_SUCCESS;
}


upll_rc_t NwMonitorMoMgr::IsReferenced(IpcReqRespHeader *req,
                                       ConfigKeyVal *ikey,
                                       DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  if (!ikey || !dmi) {
    UPLL_LOG_ERROR("Input argument is NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  MoMgrImpl *mgr = reinterpret_cast<MoMgrImpl *>
       (const_cast<MoManager *>(GetMoManager(UNC_KT_VRT_IPROUTE)));
  if (NULL == mgr) {
    UPLL_LOG_ERROR("Unable to get KT_VRT_IPROUTE momgr");
    return UPLL_RC_ERR_GENERIC;
  }
  ConfigKeyVal *temp_ckv = NULL;
  result_code = mgr->GetChildConfigKey(temp_ckv, NULL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_ERROR("GetChildConfigKeyFailed %d", result_code);
    return result_code;
  }
  key_static_ip_route *static_ipr_key = NULL;
  static_ipr_key = reinterpret_cast<key_static_ip_route *>(temp_ckv->get_key());
  uuu::upll_strncpy(static_ipr_key->vrt_key.vtn_key.vtn_name,
                    reinterpret_cast<key_nwm *>(ikey->get_key())
                    ->vbr_key.vtn_key.vtn_name, kMaxLenVtnName+1);

  val_static_ip_route *static_ipr_val =
                    ConfigKeyVal::Malloc<val_static_ip_route>();
  uuu::upll_strncpy(static_ipr_val->nwm_name, reinterpret_cast<key_nwm *>(
                    ikey->get_key())->nwmonitor_name, (kMaxLenNwmName+1));
  static_ipr_val->valid[UPLL_IDX_NWM_NAME_SIR] = UNC_VF_VALID;

  temp_ckv->AppendCfgVal(IpctSt::kIpcStValStaticIpRoute, static_ipr_val);
  // Note: controller and domain is not matched because
  // Creation itself is not allowed in another ctrl domain
  // so ctrl & domain match is not required here
  DbSubOp dbop = {kOpReadSingle, kOpMatchNone, kOpInOutNone};
  result_code = mgr->UpdateConfigDB(temp_ckv, req->datatype, UNC_OP_READ,
                                        dmi, &dbop, MAINTBL);
  DELETE_IF_NOT_NULL(temp_ckv);
  if (UPLL_RC_ERR_INSTANCE_EXISTS == result_code) {
    UPLL_LOG_ERROR("can't delete nwm which is configured in ip-route in the"
                   " same vtn controller and domain");
    return UPLL_RC_ERR_CFG_SEMANTIC;
  } else if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    result_code = UPLL_RC_SUCCESS;
  } else {
    UPLL_LOG_ERROR("UpdateConfigDB failed %d", result_code);
    return result_code;
  }
  return result_code;
}

upll_rc_t NwMonitorMoMgr::OnNwmonFault(
       string ctrlr_name ,
       string domain_id,
       const key_vtn &key_vtn,
       const pfcdrv_network_mon_alarm_data_t &alarm_data,
       bool alarm_raised,
       DalDmlIntf *dmi ) {
  UPLL_FUNC_TRACE;
  ConfigKeyVal *ikey = NULL;
  char *alarm_status = NULL;
  char al_raised[] = "Raised";
  char al_cleared[] = "Cleared";
  key_vtn_t *vtn_key = NULL;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  char *vtn_name = NULL;
  VtnMoMgr *vtn_mgr = reinterpret_cast<VtnMoMgr*>
        (const_cast<MoManager*>(GetMoManager(UNC_KT_VTN)));
  if (NULL == vtn_mgr) {
    UPLL_LOG_DEBUG("unable to get VtnMoMgr object to validate key_vtn");
    return UPLL_RC_ERR_GENERIC;
  }

  result_code = vtn_mgr->GetChildConfigKey(ikey, NULL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetChildConfigKey failed %d", result_code);
    return result_code;
  }
  vtn_key = reinterpret_cast<key_vtn_t*>(ikey->get_key());
  uuu::upll_strncpy(vtn_key->vtn_name, key_vtn.vtn_name,
                    (kMaxLenVtnName + 1));

  result_code = vtn_mgr->ValidateVtnKey(vtn_key);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("Vtn Key validation failed - %d", result_code);
    DELETE_IF_NOT_NULL(ikey);
    return result_code;
  }
  uint8_t *ctrlr_id = reinterpret_cast<uint8_t*>
      (const_cast<char*>(ctrlr_name.c_str()));
  result_code = vtn_mgr->GetRenamedUncKey(ikey, UPLL_DT_RUNNING,
                                       dmi, ctrlr_id);
  if (result_code != UPLL_RC_SUCCESS &&
      UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
    UPLL_LOG_DEBUG("Error in getting renamed vtn name");
    DELETE_IF_NOT_NULL(ikey);
    return result_code;
  }
  if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
    ikey->SetCfgVal(NULL);
    result_code = vtn_mgr->UpdateConfigDB(ikey, UPLL_DT_RUNNING, UNC_OP_READ,
                                    dmi, MAINTBL);
    if (UPLL_RC_ERR_INSTANCE_EXISTS != result_code) {
      delete ikey;
      return result_code;
    }
    result_code = UPLL_RC_SUCCESS;
  }

  vtn_key = reinterpret_cast<key_vtn_t*>(ikey->get_key());
  vtn_name = reinterpret_cast<char*>(vtn_key->vtn_name);

  if (alarm_raised) {
    alarm_status = al_raised;
  } else {
    alarm_status = al_cleared;
  }

  UPLL_LOG_INFO("Network Monitor Fault alarm : status - %s, "
                "network_mon_group_name - %s, "
                 "controller - %s, domain - %s, vtn - %s",
                alarm_status, alarm_data.network_mon_group_name,
                ctrlr_name.c_str(), domain_id.c_str(),
                vtn_name);
  DELETE_IF_NOT_NULL(ikey);
  return result_code;
}

upll_rc_t NwMonitorMoMgr::MergeValidate(unc_key_type_t keytype,
                                  const char *ctrlr_id,
                                  ConfigKeyVal *ikey,
                                  DalDmlIntf *dmi,
                                  upll_import_type import_type) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  DbSubOp dbop = { kOpReadMultiple, kOpMatchNone, kOpInOutNone };
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
  /*
   * Here getting FULL Key (VTN, VBR and Network Monitor Name )
   */
  result_code = ReadConfigDB(dup_key, UPLL_DT_IMPORT, UNC_OP_READ, dbop, dmi,
                             MAINTBL);
  if (UPLL_RC_SUCCESS != result_code) {
    DELETE_IF_NOT_NULL(dup_key);
    return result_code;
  }

  ConfigKeyVal *travel = dup_key;
  while (travel) {
    /*
     * Checks the Val structure is available or not.If availabl
     * Checks Host address value is available or not in import ckval
     */
    result_code = DupConfigKeyVal(tkey, travel, MAINTBL);

    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG(" DupConfigKeyVal is Failed");
      DELETE_IF_NOT_NULL(tkey);
      DELETE_IF_NOT_NULL(dup_key);
      return result_code;
    }
   /* Same Network Monitor Name should not present under parent VTN
    * in Candidate DB
    */
    key_nwm *key_nwmon =
      reinterpret_cast<key_nwm *>(tkey->get_key());
    memset(key_nwmon->vbr_key.vbridge_name, 0,
     sizeof(key_nwmon->vbr_key.vbridge_name));
    // Existence check in Candidate DB
    if (import_type == UPLL_IMPORT_TYPE_FULL) {
      result_code = UpdateConfigDB(tkey, UPLL_DT_CANDIDATE,
                                   UNC_OP_READ, dmi, MAINTBL);
      if (result_code == UPLL_RC_ERR_INSTANCE_EXISTS) {
        ikey->ResetWith(tkey);
        DELETE_IF_NOT_NULL(tkey);
        DELETE_IF_NOT_NULL(dup_key);
        UPLL_LOG_DEBUG("NetworkMonitor Name Conflict %s",
                       (ikey->ToStrAll()).c_str());
        return UPLL_RC_ERR_MERGE_CONFLICT;
      }
    } else {
      DbSubOp dbop1 = { kOpReadMultiple, kOpMatchNone, kOpInOutCtrlr};
      uint8_t *ctrlr_name = NULL;
      result_code = ReadConfigDB(tkey, UPLL_DT_CANDIDATE, UNC_OP_READ,
                                 dbop1, dmi, MAINTBL);

      if (UPLL_RC_SUCCESS == result_code) {
        GET_USER_DATA_CTRLR(tkey, ctrlr_name);
        if (strcmp(ctrlr_id, (const char *)ctrlr_name)) {
          DELETE_IF_NOT_NULL(tkey);
          DELETE_IF_NOT_NULL(dup_key);
          UPLL_LOG_INFO("Controller names are mismatched");
          return UPLL_RC_ERR_MERGE_CONFLICT;
        }
      }
    }

    /* Any other DB error */
    if (UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code &&
        UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("UpdateConfigDB  Failed %d", result_code);
      DELETE_IF_NOT_NULL(tkey);
      DELETE_IF_NOT_NULL(dup_key);
      return result_code;
    }
    DELETE_IF_NOT_NULL(tkey);
    travel = travel->get_next_cfg_key_val();
  }
  if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
    DELETE_IF_NOT_NULL(dup_key);
    return UPLL_RC_SUCCESS;
  }
  DELETE_IF_NOT_NULL(dup_key);
  return result_code;
}

upll_rc_t NwMonitorMoMgr::ValidateAttribute(ConfigKeyVal *ikey,
                                            DalDmlIntf *dmi,
                                            IpcReqRespHeader *req) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  if (req->operation != UNC_OP_CREATE) {
    return UPLL_RC_SUCCESS;
  }
  ConfigKeyVal *temp_ikey = NULL;
  result_code = GetChildConfigKey(temp_ikey, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Returning %d", result_code);
    return result_code;
  }
  key_nwm *key_nwmon =
    reinterpret_cast<key_nwm *>(temp_ikey->get_key());
  char input_nwm_vbrname[kMaxLenVnodeName + 1];
  input_nwm_vbrname[0] = 0;
  // Saves received network-monitor group vbridge name
  uuu::upll_strncpy(input_nwm_vbrname, key_nwmon->vbr_key.vbridge_name,
      kMaxLenVnodeName + 1);
  /* Resets the vbridge name to verify whether the same
   * network monitor group entry present */
  memset(key_nwmon->vbr_key.vbridge_name, 0, kMaxLenVnodeName);
  // Existence check in DB
  DbSubOp nwm_dbop = { kOpReadSingle, kOpMatchNone, kOpInOutNone };
  result_code = ReadConfigDB(temp_ikey, req->datatype,
      UNC_OP_READ, nwm_dbop, dmi, MAINTBL);
  UPLL_LOG_TRACE("Result code = %u", result_code);
  if (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    /* If the received vbridge and existing DB vbridge are different throw a
     * UPLL_RC_ERR_CFG_SEMANTIC else UPLL_RC_ERR_INSTANCE_EXISTS */
    if (result_code == UPLL_RC_SUCCESS) {
      if (strcmp(reinterpret_cast<char*>(key_nwmon->vbr_key.vbridge_name),
                 input_nwm_vbrname) == 0) {
        result_code= UPLL_RC_ERR_INSTANCE_EXISTS;
      } else {
        UPLL_LOG_TRACE("Same network monitor group is configured under another"
                       " vbridge of same VTN")
        result_code= UPLL_RC_ERR_CFG_SEMANTIC;
      }
    }
    DELETE_IF_NOT_NULL(temp_ikey);
    UPLL_LOG_TRACE("Result code = %u", result_code);
    return result_code;
  }
  DELETE_IF_NOT_NULL(temp_ikey);
  return UPLL_RC_SUCCESS;
}
upll_rc_t NwMonitorMoMgr::AdaptValToDriver(ConfigKeyVal *ck_new,
                                           ConfigKeyVal *ck_old,
                                           unc_keytype_operation_t op,
                                           upll_keytype_datatype_t dt_type,
                                           unc_key_type_t keytype,
                                           DalDmlIntf *dmi,
                                           bool &not_send_to_drv,
                                           bool audit_update_phase) {
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  if (!audit_update_phase) {
    if ((op == UNC_OP_DELETE) &&
        (keytype == UNC_KT_VBR_NWMONITOR)) {
      // Verify whether given Network Monitor is referred in
      // flowfilter_entry or not. If it is referred then return semantic error
      result_code  = ValidateNWM(ck_new, UPLL_DT_CANDIDATE, dmi);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_INFO("Cannot delete NWM! Validation failed"
                      " result code - %d", result_code);
        return result_code;
      }
    }
  }
  return result_code;
}

upll_rc_t NwMonitorMoMgr::ValidateVtnRename(ConfigKeyVal *org_vtn_ckv,
                                            ConfigKeyVal *rename_vtn_ckv,
                                            DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  DbSubOp dbop = { kOpReadMultiple, kOpMatchNone, kOpInOutNone };
  ConfigKeyVal *nwm_ckv = NULL;

  if (!org_vtn_ckv || !org_vtn_ckv->get_key() || !rename_vtn_ckv ||
      !rename_vtn_ckv->get_key()) {
    UPLL_LOG_DEBUG("Input is NULL");
    return UPLL_RC_ERR_GENERIC;
  }

  result_code = GetChildConfigKey(nwm_ckv, org_vtn_ckv);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetChildConfigKey Failed");
    return result_code;
  }

  /* Gets All VBR_NWM ConfigKeyVall based on original vtn name */
  result_code = ReadConfigDB(nwm_ckv, UPLL_DT_IMPORT, UNC_OP_READ, dbop, dmi,
                             MAINTBL);
  if (UPLL_RC_SUCCESS != result_code) {
    if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code)
      result_code = UPLL_RC_SUCCESS;

    DELETE_IF_NOT_NULL(nwm_ckv);
    return result_code;
  }

  uint8_t *vtn_rename = reinterpret_cast<key_vtn_t*>(
                         rename_vtn_ckv->get_key())->vtn_name;
  ConfigKeyVal *travel = nwm_ckv;
  while (travel) {
    /* Verifies whether the same network group is exist under the
     * new vtn name */
    uuu::upll_strncpy(reinterpret_cast<key_nwm*>(
                   travel->get_key())->vbr_key.vtn_key.vtn_name,
                   vtn_rename,
                   (kMaxLenVtnName + 1));
    memset(reinterpret_cast<key_nwm*>(
                travel->get_key())->vbr_key.vbridge_name, 0, kMaxLenVnodeName);
    result_code = UpdateConfigDB(travel, UPLL_DT_IMPORT, UNC_OP_READ,
                                 dmi, MAINTBL);
    if (result_code != UPLL_RC_ERR_INSTANCE_EXISTS &&
        result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      UPLL_LOG_DEBUG("UpdateConfigDb Db error");
      DELETE_IF_NOT_NULL(nwm_ckv);
      return result_code;
    } else if (result_code == UPLL_RC_ERR_INSTANCE_EXISTS) {
      UPLL_LOG_DEBUG("Same network monitor group exist in another vbridge");
      DELETE_IF_NOT_NULL(nwm_ckv);
      return UPLL_RC_ERR_MERGE_CONFLICT;
    }

    travel = travel->get_next_cfg_key_val();
  }
  DELETE_IF_NOT_NULL(nwm_ckv);
  return UPLL_RC_SUCCESS;
}


}  // namespace kt_momgr
}  // namespace upll
}  // namespace unc
