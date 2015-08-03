/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include "nwm_host_momgr.hh"
#include "uncxx/upll_log.hh"
#include "nwm_momgr.hh"

#define NUM_KEY_MAIN_TBL_ 6
namespace unc {
namespace upll {
namespace kt_momgr {

BindInfo NwMonitorHostMoMgr::nwm_host_bind_info[] =
    { { uudst::vbridge_networkmonitor_host::kDbiVtnName, CFG_KEY, offsetof(
        key_nwm_host, nwm_key.vbr_key.vtn_key.vtn_name),
        uud::kDalChar, 32 },
      { uudst::vbridge_networkmonitor_host::kDbiVbrName, CFG_KEY, offsetof(
          key_nwm_host, nwm_key.vbr_key.vbridge_name),
        uud::kDalChar, 32 },
      { uudst::vbridge_networkmonitor_host::kDbiNwmName, CFG_KEY, offsetof(
          key_nwm_host, nwm_key.nwmonitor_name),
        uud::kDalChar, 32 },
      { uudst::vbridge_networkmonitor_host::kDbiHostAddress, CFG_KEY, offsetof(
          key_nwm_host, host_address.s_addr),
        uud::kDalUint32, 1 },
      { uudst::vbridge_networkmonitor_host::kDbiHealthInterval, CFG_VAL,
        offsetof(val_nwm_host, health_interval), uud::kDalUint16, 1 },
      { uudst::vbridge_networkmonitor_host::kDbiRecoveryInterval, CFG_VAL,
        offsetof(val_nwm_host, recovery_interval), uud::kDalUint16, 1 },
      { uudst::vbridge_networkmonitor_host::kDbiFailureCount, CFG_VAL, offsetof(
          val_nwm_host, failure_count),
        uud::kDalUint8, 1 },
      { uudst::vbridge_networkmonitor_host::kDbiRecoveryCount, CFG_VAL,
        offsetof(val_nwm_host, recovery_count), uud::kDalUint8, 1 },
      { uudst::vbridge_networkmonitor_host::kDbiWaitTime, CFG_VAL, offsetof(
          val_nwm_host, wait_time),
        uud::kDalUint8, 1 },
      { uudst::vbridge_networkmonitor_host::kDbiCtrlrName, CK_VAL, offsetof(
          key_user_data, ctrlr_id),
        uud::kDalChar, 32 },
      { uudst::vbridge_networkmonitor_host::kDbiDomainId, CK_VAL, offsetof(
          key_user_data, domain_id),
        uud::kDalChar, 32 },
      { uudst::vbridge_networkmonitor_host::kDbiValidHealthInterval,
        CFG_META_VAL, offsetof(val_nwm_host,
                               valid[UPLL_IDX_HEALTH_INTERVAL_NWMH]),
        uud::kDalUint8, 1 },
      { uudst::vbridge_networkmonitor_host::kDbiValidRecoveryInterval,
        CFG_META_VAL, offsetof(val_nwm_host,
                               valid[UPLL_IDX_RECOVERY_INTERVAL_NWMH]),
        uud::kDalUint8, 1 },
      { uudst::vbridge_networkmonitor_host::kDbiValidFailureCount, CFG_META_VAL,
        offsetof(val_nwm_host, valid[UPLL_IDX_FAILURE_COUNT_NWMH]),
        uud::kDalUint8, 1 },
      { uudst::vbridge_networkmonitor_host::kDbiValidRecoveryCount,
        CFG_META_VAL, offsetof(val_nwm_host,
                               valid[UPLL_IDX_RECOVERY_COUNT_NWMH]),
        uud::kDalUint8, 1 },
      { uudst::vbridge_networkmonitor_host::kDbiValidWaitTime, CFG_META_VAL,
        offsetof(val_nwm_host, valid[UPLL_IDX_WAIT_TIME_NWMH]), uud::kDalUint8,
        1 },
      { uudst::vbridge_networkmonitor_host::kDbiCsHealthInterval, CS_VAL,
        offsetof(val_nwm_host, cs_attr[UPLL_IDX_HEALTH_INTERVAL_NWMH]),
        uud::kDalUint8, 1 },
      { uudst::vbridge_networkmonitor_host::kDbiCsRecoveryInterval, CS_VAL,
        offsetof(val_nwm_host, cs_attr[UPLL_IDX_RECOVERY_INTERVAL_NWMH]),
        uud::kDalUint8, 1 },
      { uudst::vbridge_networkmonitor_host::kDbiCsFailureCount, CS_VAL,
        offsetof(val_nwm_host, cs_attr[UPLL_IDX_FAILURE_COUNT_NWMH]),
        uud::kDalUint8, 1 },
      { uudst::vbridge_networkmonitor_host::kDbiCsRecoveryCount, CS_VAL,
        offsetof(val_nwm_host, cs_attr[UPLL_IDX_RECOVERY_COUNT_NWMH]),
        uud::kDalUint8, 1 },
      { uudst::vbridge_networkmonitor_host::kDbiCsWaitTime, CS_VAL, offsetof(
          val_nwm_host, cs_attr[UPLL_IDX_WAIT_TIME_NWMH]),
        uud::kDalUint8, 1 },
      { uudst::vbridge_networkmonitor_host::kDbiCsRowstatus, CS_VAL, offsetof(
          val_nwm_host, cs_row_status),
        uud::kDalUint8, 1 },
      { uudst::vbridge_networkmonitor_host::kDbiFlags, CK_VAL, offsetof(
          key_user_data, flags),
        uud::kDalUint8, 1 }, };

BindInfo NwMonitorHostMoMgr::key_nwm_host_maintbl_update_bind_info[] = {
    { uudst::vbridge_networkmonitor_host::kDbiVtnName, CFG_MATCH_KEY, offsetof(
        key_nwm_host, nwm_key.vbr_key.vtn_key.vtn_name),
      uud::kDalChar, kMaxLenVtnName + 1 },
    { uudst::vbridge_networkmonitor_host::kDbiVbrName, CFG_MATCH_KEY, offsetof(
        key_nwm_host, nwm_key.vbr_key.vbridge_name),
      uud::kDalChar, kMaxLenVnodeName + 1 },
    { uudst::vbridge_networkmonitor_host::kDbiNwmName, CFG_MATCH_KEY, offsetof(
        key_nwm_host, nwm_key.nwmonitor_name),
      uud::kDalChar, kMaxLenVnodeName + 1 },
    { uudst::vbridge_networkmonitor_host::kDbiVtnName, CFG_INPUT_KEY, offsetof(
        key_rename_vnode_info_t, new_unc_vtn_name),
      uud::kDalChar, kMaxLenVtnName + 1 },
    { uudst::vbridge_networkmonitor_host::kDbiVbrName, CFG_INPUT_KEY, offsetof(
        key_rename_vnode_info_t, new_unc_vnode_name),
      uud::kDalChar, kMaxLenVnodeName + 1 },
    { uudst::vbridge_networkmonitor_host::kDbiFlags, CK_VAL, offsetof(
        key_user_data, flags),
      uud::kDalUint8, 1 }, };

NwMonitorHostMoMgr::NwMonitorHostMoMgr() {
  UPLL_FUNC_TRACE
  ntable = MAX_MOMGR_TBLS;
  table = new Table *[ntable]();
  table[MAINTBL] = new Table(uudst::kDbiVbrNwMonHostTbl,
      UNC_KT_VBR_NWMONITOR_HOST, nwm_host_bind_info,
      IpctSt::kIpcStKeyNwmHost, IpctSt::kIpcStValNwmHost,
      uudst::vbridge_networkmonitor_host::kDbiVbrNwMonHostNumCols);
  table[RENAMETBL] = NULL;
  table[CTRLRTBL] = NULL;
  table[CONVERTTBL] = NULL;
  nchild = 0;
  child = NULL;
}

/*
 * Based on the key type the bind info will pass
 **/

bool NwMonitorHostMoMgr::GetRenameKeyBindInfo(unc_key_type_t key_type,
                                              BindInfo *&binfo, int &nattr,
                                              MoMgrTables tbl) {
  /* Main Table or rename table only update */
  if (MAINTBL == tbl) {
    nattr = NUM_KEY_MAIN_TBL_;
    binfo = key_nwm_host_maintbl_update_bind_info;
  }
  return PFC_TRUE;
}

bool NwMonitorHostMoMgr::IsValidKey(void *key, uint64_t index,
                                    MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  key_nwm_host *nwmh_key = reinterpret_cast<key_nwm_host *>(key);
  uint32_t val = 0;
  upll_rc_t ret_val = UPLL_RC_SUCCESS;
  switch (index) {
    case uudst::vbridge_networkmonitor_host::kDbiVtnName:
      ret_val = ValidateKey(
          reinterpret_cast<char *>(nwmh_key->nwm_key.vbr_key.vtn_key.vtn_name),
          kMinLenVtnName, kMaxLenVtnName);
      if (ret_val != UPLL_RC_SUCCESS) {
        UPLL_LOG_TRACE("VTN Name is not valid(%d)", ret_val);
        return false;
      }
      break;
    case uudst::vbridge_networkmonitor_host::kDbiVbrName:
      ret_val = ValidateKey(
          reinterpret_cast<char *>(nwmh_key->nwm_key.vbr_key.vbridge_name),
          kMinLenVnodeName, kMaxLenVnodeName);
      if (ret_val != UPLL_RC_SUCCESS) {
        UPLL_LOG_TRACE("VBridge Name is not valid(%d)", ret_val);
        return false;
      }
      break;
    case uudst::vbridge_networkmonitor_host::kDbiNwmName:
      ret_val = ValidateKey(
          reinterpret_cast<char *>(nwmh_key->nwm_key.nwmonitor_name),
          kMinLenNwmName, kMaxLenNwmName);
      if (ret_val != UPLL_RC_SUCCESS) {
        UPLL_LOG_TRACE("NwMonitor Name is not valid(%d)", ret_val);
        return false;
      }
      break;
    case uudst::vbridge_networkmonitor_host::kDbiHostAddress:
      val = nwmh_key->host_address.s_addr;
      if (val == 0xffffffff || val == 0x00000000) {
        UPLL_LOG_DEBUG("Invalid Host Address");
        return false;
      }
      break;
    default:
      UPLL_LOG_INFO("Invalid Key Index");
      return false;
  }
  return true;
}

upll_rc_t NwMonitorHostMoMgr::GetChildConfigKey(ConfigKeyVal *&okey,
                                                ConfigKeyVal *parent_key) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  key_nwm_host *nwmh_key;
  void *pkey;
  if (parent_key == NULL || !(parent_key->get_key())) {
    UPLL_LOG_DEBUG("parentkey is Null");
    nwmh_key = reinterpret_cast<key_nwm_host *>(
             ConfigKeyVal::Malloc(sizeof(key_nwm_host)));
    if (okey) delete okey;
    okey = new ConfigKeyVal(UNC_KT_VBR_NWMONITOR_HOST, IpctSt::kIpcStKeyNwmHost,
                            nwmh_key, NULL);
    return UPLL_RC_SUCCESS;
  } else {
    pkey = parent_key->get_key();
  }
  if (!pkey) return UPLL_RC_ERR_GENERIC;
  if (okey) {
    if (okey->get_key_type() != UNC_KT_VBR_NWMONITOR_HOST)
      return UPLL_RC_ERR_GENERIC;
    nwmh_key = reinterpret_cast<key_nwm_host *>(okey->get_key());
  } else {
    nwmh_key = reinterpret_cast<key_nwm_host *>(
      ConfigKeyVal::Malloc(sizeof(key_nwm_host)));
  }
  unc_key_type_t keytype = parent_key->get_key_type();
  switch (keytype) {
    case UNC_KT_VTN:
      uuu::upll_strncpy(nwmh_key->nwm_key.vbr_key.vtn_key.vtn_name,
             reinterpret_cast<key_vtn *>(pkey)->vtn_name,
             (kMaxLenVtnName + 1));
      *(nwmh_key->nwm_key.vbr_key.vbridge_name) = *"";
      *nwmh_key->nwm_key.nwmonitor_name = *"";
      nwmh_key->host_address.s_addr = 0;
      break;
    case UNC_KT_VBRIDGE:
      uuu::upll_strncpy(nwmh_key->nwm_key.vbr_key.vtn_key.vtn_name,
             reinterpret_cast<key_nwm *>(pkey)->vbr_key.vtn_key.vtn_name,
             (kMaxLenVtnName + 1));
      uuu::upll_strncpy(nwmh_key->nwm_key.vbr_key.vbridge_name,
             reinterpret_cast<key_nwm *>(pkey)->vbr_key.vbridge_name,
             (kMaxLenVnodeName + 1));
      break;
    case UNC_KT_VBR_NWMONITOR:
      uuu::upll_strncpy(nwmh_key->nwm_key.vbr_key.vtn_key.vtn_name,
             reinterpret_cast<key_nwm *>(pkey)->vbr_key.vtn_key.vtn_name,
             (kMaxLenVtnName + 1));
      uuu::upll_strncpy(nwmh_key->nwm_key.vbr_key.vbridge_name,
             reinterpret_cast<key_nwm *>(pkey)->vbr_key.vbridge_name,
             (kMaxLenVnodeName + 1));
      uuu::upll_strncpy(nwmh_key->nwm_key.nwmonitor_name,
             reinterpret_cast<key_nwm *>(pkey)->nwmonitor_name,
             (kMaxLenNwmName + 1));
      break;
    case UNC_KT_VBR_NWMONITOR_HOST:
      uuu::upll_strncpy(nwmh_key->nwm_key.vbr_key.vtn_key.vtn_name,
            reinterpret_cast<key_nwm_host *>
            (pkey)->nwm_key.vbr_key.vtn_key.vtn_name,
            (kMaxLenVtnName + 1));
      uuu::upll_strncpy(nwmh_key->nwm_key.vbr_key.vbridge_name,
          reinterpret_cast<key_nwm_host *>(pkey)->nwm_key.vbr_key.vbridge_name,
          (kMaxLenVnodeName + 1));
      uuu::upll_strncpy(nwmh_key->nwm_key.nwmonitor_name,
          reinterpret_cast<key_nwm_host *>(pkey)->nwm_key.nwmonitor_name,
          (kMaxLenNwmName + 1));
      nwmh_key->host_address.s_addr =
          reinterpret_cast<key_nwm_host *>(pkey)->host_address.s_addr;
    default:
      break;
  }
  if (!okey)
    okey = new ConfigKeyVal(UNC_KT_VBR_NWMONITOR_HOST, IpctSt::kIpcStKeyNwmHost,
                            nwmh_key, NULL);
  else if (okey->get_key() != nwmh_key)
    okey->SetKey(IpctSt::kIpcStKeyNwmHost, nwmh_key);

  if (okey == NULL) {
    free(nwmh_key);
    result_code = UPLL_RC_ERR_GENERIC;
  } else  {
    SET_USER_DATA(okey, parent_key);
  }
  return result_code;
}

upll_rc_t NwMonitorHostMoMgr::GetParentConfigKey(ConfigKeyVal *&okey,
                                                 ConfigKeyVal *ikey) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  if (!ikey) {
    UPLL_LOG_DEBUG("ikey is NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  unc_key_type_t ikey_type = ikey->get_key_type();

  if (ikey_type != UNC_KT_VBR_NWMONITOR_HOST) return UPLL_RC_ERR_GENERIC;
  key_nwm_host *pkey = reinterpret_cast<key_nwm_host *>
      (ikey->get_key());
  if (!pkey) return UPLL_RC_ERR_GENERIC;
  key_nwm *nwm_key = reinterpret_cast<key_nwm *>(
    ConfigKeyVal::Malloc(sizeof(key_nwm)));
  uuu::upll_strncpy(nwm_key->vbr_key.vtn_key.vtn_name,
      reinterpret_cast<key_nwm_host *>(pkey)->nwm_key.vbr_key.vtn_key.vtn_name,
      (kMaxLenVtnName + 1));
  uuu::upll_strncpy(nwm_key->vbr_key.vbridge_name,
      reinterpret_cast<key_nwm_host *>(pkey)->nwm_key.vbr_key.vbridge_name,
      (kMaxLenVnodeName + 1));
  uuu::upll_strncpy(nwm_key->nwmonitor_name,
      reinterpret_cast<key_nwm_host *>(pkey)->nwm_key.nwmonitor_name,
      (kMaxLenNwmName + 1));
  if (okey) delete okey;
  okey = new ConfigKeyVal(UNC_KT_VBR_NWMONITOR, IpctSt::kIpcStKeyNwm, nwm_key,
                          NULL);
  if (okey == NULL) {
    free(nwm_key);
    result_code = UPLL_RC_ERR_GENERIC;
  } else {
    SET_USER_DATA(okey, ikey);
  }
  return result_code;
}

upll_rc_t NwMonitorHostMoMgr::AllocVal(ConfigVal *&ck_val,
                                       upll_keytype_datatype_t dt_type,
                                       MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  void *val;  // , *nxt_val;
  //   ConfigVal *ck_nxtval;

  if (ck_val != NULL) return UPLL_RC_ERR_GENERIC;
  switch (tbl) {
    case MAINTBL:
      val = ConfigKeyVal::Malloc(sizeof(val_nwm_host));
      ck_val = new ConfigVal(IpctSt::kIpcStValNwmHost, val);
      if (dt_type == UPLL_DT_STATE) {
        val = ConfigKeyVal::Malloc(sizeof(val_nwm_host_st));
        ConfigVal *ck_nxtval = new ConfigVal(IpctSt::kIpcStValNwmHostSt, val);
        ck_val->AppendCfgVal(ck_nxtval);
      }
      break;
    default:
      val = NULL;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t NwMonitorHostMoMgr::DupConfigKeyVal(ConfigKeyVal *&okey,
                                              ConfigKeyVal *&req,
                                              MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  if (req == NULL) return UPLL_RC_ERR_GENERIC;
  if (okey != NULL) return UPLL_RC_ERR_GENERIC;
  if (req->get_key_type() != UNC_KT_VBR_NWMONITOR_HOST)
    return UPLL_RC_ERR_GENERIC;
  ConfigVal *tmp1 = NULL, *tmp = (req)->get_cfg_val();
  if (tmp) {
    if (tbl == MAINTBL) {
      val_nwm_host *ival = reinterpret_cast<val_nwm_host *>(GetVal(req));
      if (ival == NULL) {
        UPLL_LOG_DEBUG("NULL val structure");
        return UPLL_RC_ERR_GENERIC;
      }
      val_nwm_host *nwm_val = reinterpret_cast<val_nwm_host *>
          (ConfigKeyVal::Malloc(sizeof(val_nwm_host)));
      memcpy(nwm_val, ival, sizeof(val_nwm_host));
      tmp1 = new ConfigVal(IpctSt::kIpcStValNwmHost, nwm_val);
    }
    tmp = tmp->get_next_cfg_val();
  };
  if (tmp) {
    if (tbl == MAINTBL && (tmp->get_st_num() == IpctSt::kIpcStValNwmHostSt)) {
      val_nwm_host_st *ival = reinterpret_cast<val_nwm_host_st *>
                                              (tmp->get_val());
      if (ival == NULL) {
        UPLL_LOG_DEBUG("NULL val structure");
        delete tmp1;
        return UPLL_RC_ERR_GENERIC;
      }
      val_nwm_host_st *val_nwmst = reinterpret_cast<val_nwm_host_st *>
          (ConfigKeyVal::Malloc(sizeof(val_nwm_host_st)));
      memcpy(val_nwmst, ival, sizeof(val_nwm_host_st));

      //        val_nwm_host_st *val_nwmst = new val_nwm_host_st(*ival);
      ConfigVal *tmp2 = new ConfigVal(IpctSt::kIpcStValNwmHostSt, val_nwmst);
      if (tmp1)
        tmp1->AppendCfgVal(tmp2);
    }
  };
  void *tkey = req->get_key();
  if (!tkey) {
    DELETE_IF_NOT_NULL(tmp1);
    return UPLL_RC_ERR_GENERIC;
  }
  key_nwm_host *ikey = reinterpret_cast<key_nwm_host *>(tkey);
  key_nwm_host *nwm_key = reinterpret_cast<key_nwm_host *>
      (ConfigKeyVal::Malloc(sizeof(key_nwm_host)));
  memcpy(nwm_key, ikey, sizeof(key_nwm_host));
  okey = new ConfigKeyVal(UNC_KT_VBR_NWMONITOR_HOST, IpctSt::kIpcStKeyNwmHost,
                          nwm_key, tmp1);
  if (okey) {
    SET_USER_DATA(okey, req);
  } else  {
    DELETE_IF_NOT_NULL(tmp1);
    DELETE_IF_NOT_NULL(nwm_key);
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t NwMonitorHostMoMgr::UpdateConfigStatus(ConfigKeyVal *ikey,
                                                 unc_keytype_operation_t op,
                                                 uint32_t driver_result,
                                                 ConfigKeyVal *upd_key,
                                                 DalDmlIntf *dmi,
                                                 ConfigKeyVal *ctrlr_key) {
  UPLL_FUNC_TRACE;
  val_nwm_host *nwm_val = reinterpret_cast<val_nwm_host_t*>(GetVal(ikey));
  val_nwm_host *nwm_val2 = reinterpret_cast<val_nwm_host *>(GetVal(upd_key));

  unc_keytype_configstatus_t cs_status =
      (driver_result == 0) ? UNC_CS_APPLIED : UNC_CS_NOT_APPLIED;
  if (nwm_val == NULL) return UPLL_RC_ERR_GENERIC;
  UPLL_LOG_TRACE("Key in Candidate %s", (ikey->ToStrAll()).c_str());
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
       loop < sizeof(nwm_val->valid) / sizeof(nwm_val->valid[0]); ++loop) {
    if ((UNC_VF_VALID == (uint8_t) nwm_val->valid[loop])
          || (UNC_VF_VALID_NO_VALUE == (uint8_t) nwm_val->valid[loop])) {
      nwm_val->cs_attr[loop] = cs_status;
    } else if ((UNC_VF_INVALID == nwm_val->valid[loop]) &&
               (UNC_OP_CREATE == op)) {
        nwm_val->cs_attr[loop] = UNC_CS_NOT_APPLIED;
    } else if ((UNC_VF_INVALID == nwm_val->valid[loop]) &&
               (UNC_OP_UPDATE == op)) {
        nwm_val->cs_attr[loop] = nwm_val2->cs_attr[loop];
    }
  }
  return UPLL_RC_SUCCESS;
}

bool NwMonitorHostMoMgr::FilterAttributes(void *&val1, void *val2,
                                          bool audit_status,
                                          unc_keytype_operation_t op) {
  UPLL_FUNC_TRACE;
  if (op != UNC_OP_CREATE)
    return CompareValidValue(val1, val2, audit_status);
  return false;
}

bool NwMonitorHostMoMgr::CompareValidValue(void *&val1, void *val2,
                                           bool copy_to_running) {
  UPLL_FUNC_TRACE;
  bool invalid_attr = true;
  val_nwm_host_t *valnwmhost1 = reinterpret_cast<val_nwm_host_t *>(val1);
  val_nwm_host_t *valnwmhost2 = reinterpret_cast<val_nwm_host_t *>(val2);
  for (unsigned int loop = 0;
      loop < sizeof(valnwmhost1->valid) / sizeof(uint8_t); ++loop) {
    if (UNC_VF_INVALID == valnwmhost1->valid[loop]
        && UNC_VF_VALID == valnwmhost2->valid[loop])
      valnwmhost1->valid[loop] = UNC_VF_VALID_NO_VALUE;
  }
  if (UNC_VF_VALID == valnwmhost1->valid[UPLL_IDX_HEALTH_INTERVAL_NWMH]
      && UNC_VF_VALID == valnwmhost2->valid[UPLL_IDX_HEALTH_INTERVAL_NWMH]) {
    if (valnwmhost1->health_interval == valnwmhost2->health_interval)
      valnwmhost1->valid[UPLL_IDX_HEALTH_INTERVAL_NWMH] =
        (copy_to_running)?UNC_VF_INVALID:UNC_VF_VALUE_NOT_MODIFIED;
  }
  if (UNC_VF_VALID == valnwmhost1->valid[UPLL_IDX_RECOVERY_INTERVAL_NWMH]
      && UNC_VF_VALID == valnwmhost2->valid[UPLL_IDX_RECOVERY_INTERVAL_NWMH]) {
    if (valnwmhost1->recovery_interval == valnwmhost2->recovery_interval)
      valnwmhost1->valid[UPLL_IDX_RECOVERY_INTERVAL_NWMH] =
        (copy_to_running)?UNC_VF_INVALID:UNC_VF_VALUE_NOT_MODIFIED;
  }
  if (UNC_VF_VALID == valnwmhost1->valid[UPLL_IDX_FAILURE_COUNT_NWMH]
      && UNC_VF_VALID == valnwmhost2->valid[UPLL_IDX_FAILURE_COUNT_NWMH]) {
    if (valnwmhost1->failure_count == valnwmhost2->failure_count)
      valnwmhost1->valid[UPLL_IDX_FAILURE_COUNT_NWMH] =
        (copy_to_running)?UNC_VF_INVALID:UNC_VF_VALUE_NOT_MODIFIED;
  }
  if (UNC_VF_VALID == valnwmhost1->valid[UPLL_IDX_RECOVERY_COUNT_NWMH]
      && UNC_VF_VALID == valnwmhost2->valid[UPLL_IDX_RECOVERY_COUNT_NWMH]) {
    if (valnwmhost1->recovery_count == valnwmhost2->recovery_count)
      valnwmhost1->valid[UPLL_IDX_RECOVERY_COUNT_NWMH] =
        (copy_to_running)?UNC_VF_INVALID:UNC_VF_VALUE_NOT_MODIFIED;
  }
  if (UNC_VF_VALID == valnwmhost1->valid[UPLL_IDX_WAIT_TIME_NWMH]
      && UNC_VF_VALID == valnwmhost2->valid[UPLL_IDX_WAIT_TIME_NWMH]) {
    if (valnwmhost1->wait_time == valnwmhost2->wait_time)
      valnwmhost1->valid[UPLL_IDX_WAIT_TIME_NWMH] =
        (copy_to_running)?UNC_VF_INVALID:UNC_VF_VALUE_NOT_MODIFIED;
  }
  for (unsigned int loop = 0;
       loop < (sizeof(valnwmhost1->valid) / sizeof(valnwmhost1->valid[0]));
       ++loop) {
    if ((UNC_VF_VALID == (uint8_t) valnwmhost1->valid[loop])
        || (UNC_VF_VALID_NO_VALUE == (uint8_t) valnwmhost1->valid[loop])) {
      invalid_attr = false;
      break;
    }
  }
  return invalid_attr;
}

upll_rc_t NwMonitorHostMoMgr::UpdateAuditConfigStatus(
    unc_keytype_configstatus_t cs_status,
    uuc::UpdateCtrlrPhase phase,
    ConfigKeyVal *&ckv_running,
    DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  val_nwm_host_t *val;
  val = (ckv_running != NULL) ?
      reinterpret_cast<val_nwm_host_t *>(GetVal(ckv_running)) : NULL;
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
    if ((cs_status == UNC_CS_INVALID && UNC_VF_VALID == val->valid[loop]) ||
       cs_status == UNC_CS_APPLIED) {
      val->cs_attr[loop] = cs_status;
    }
  }
  return result_code;
}

upll_rc_t NwMonitorHostMoMgr::CopyToConfigKey(ConfigKeyVal *&okey,
                                              ConfigKeyVal *ikey) {
  if (!ikey || !(ikey->get_key())) return UPLL_RC_ERR_GENERIC;

  upll_rc_t result_code = UPLL_RC_SUCCESS;

  key_rename_vnode_info *key_rename =
      reinterpret_cast<key_rename_vnode_info *>(ikey->get_key());
  key_nwm_host_t * key_vwm = static_cast<key_nwm_host_t *>(
    ConfigKeyVal::Malloc(sizeof(key_nwm_host_t)));
  //  key_nwm_host, nwm_key.vbr_key.vtn_key.vtn_name
  if (!strlen(reinterpret_cast<char *>(key_rename->old_unc_vtn_name))) {
    free(key_vwm);
    return UPLL_RC_ERR_GENERIC;
  }
  uuu::upll_strncpy(key_vwm->nwm_key.vbr_key.vtn_key.vtn_name,
         key_rename->old_unc_vtn_name, (kMaxLenVtnName + 1));
  if (ikey->get_key_type() == UNC_KT_VBRIDGE) {
    if (!strlen(reinterpret_cast<char *>(key_rename->old_unc_vnode_name))) {
      free(key_vwm);
      return UPLL_RC_ERR_GENERIC;
    }
    uuu::upll_strncpy(key_vwm->nwm_key.vbr_key.vbridge_name,
           key_rename->old_unc_vnode_name, (kMaxLenVnodeName + 1));
  } else {
    if (!strlen(reinterpret_cast<char *>(key_rename->new_unc_vnode_name))) {
      FREE_IF_NOT_NULL(key_vwm);
      return UPLL_RC_ERR_GENERIC;
    }
    uuu::upll_strncpy(key_vwm->nwm_key.vbr_key.vbridge_name,
       key_rename->new_unc_vnode_name, (kMaxLenVnodeName+1));
  }
  okey = new ConfigKeyVal(UNC_KT_VBR_NWMONITOR_HOST, IpctSt::kIpcStKeyNwmHost,
                          key_vwm, NULL);
  if (!okey) {
    free(key_vwm);
    return UPLL_RC_ERR_GENERIC;
  }
  return result_code;
}
upll_rc_t NwMonitorHostMoMgr::ValidateMessage(IpcReqRespHeader *req,
    ConfigKeyVal *ikey) {
  UPLL_FUNC_TRACE;
  upll_rc_t ret_val = UPLL_RC_SUCCESS;
  if (!ikey || !req || !(ikey->get_key())) {
    UPLL_LOG_DEBUG("IpcReqRespHeader or ConfigKeyVal is Null");
    return UPLL_RC_ERR_BAD_REQUEST;
  }

  unc_key_type_t keytype = ikey->get_key_type();
  if (UNC_KT_VBR_NWMONITOR_HOST != keytype) {
    UPLL_LOG_DEBUG("Invalid keytype. Keytype- %d", keytype);
    return UPLL_RC_ERR_BAD_REQUEST;
  }
  if ((ikey->get_st_num()) != IpctSt::kIpcStKeyNwmHost) {
    UPLL_LOG_DEBUG(
        "Invalid structure received.Expected struct-kIpcStKeyNwmHost,"
        "received struct -%d ",
        (ikey->get_st_num()));
    return UPLL_RC_ERR_BAD_REQUEST;
  }
  key_nwm_host_t *key_nwm_host =
    reinterpret_cast<key_nwm_host_t *>(ikey->get_key());
  val_nwm_host_t *val_nwm_host = NULL;
  if ((ikey->get_cfg_val())
      && ((ikey->get_cfg_val())->get_st_num() == IpctSt::kIpcStValNwmHost)) {
    val_nwm_host =
      reinterpret_cast<val_nwm_host_t *>(ikey->get_cfg_val()->get_val());
  }
  upll_keytype_datatype_t dt_type = req->datatype;
  unc_keytype_operation_t operation = req->operation;
  unc_keytype_option1_t option1 = req->option1;
  unc_keytype_option2_t option2 = req->option2;

  if (key_nwm_host == NULL) {
    UPLL_LOG_DEBUG("key structure is empty!!");
    return UPLL_RC_ERR_BAD_REQUEST;
  }
  ret_val = ValidateNwMonHostKey(key_nwm_host, operation);
  if (ret_val != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG(
        "Key structure validation failure for KT_VBR_NWMONITOR_HOST");
    return UPLL_RC_ERR_CFG_SYNTAX;
  } else {
    if (((operation == UNC_OP_CREATE) || (operation == UNC_OP_UPDATE))
        && ((dt_type == UPLL_DT_CANDIDATE)||(UPLL_DT_IMPORT == dt_type))) {
      if (val_nwm_host != NULL) {
        ret_val = ValidateNwMonHostValue(val_nwm_host, operation);
        if (ret_val != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("Syntax check failed for VBR_NWMONITOR_HOST");
          return UPLL_RC_ERR_CFG_SYNTAX;
        }
        return UPLL_RC_SUCCESS;
      } else {
        UPLL_LOG_DEBUG("Value structure is mandatory");
        return UPLL_RC_ERR_CFG_SYNTAX;
      }
    } else if (((operation == UNC_OP_READ) || (operation == UNC_OP_READ_SIBLING)
          || (operation == UNC_OP_READ_SIBLING_BEGIN)
          || (operation == UNC_OP_READ_SIBLING_COUNT))
        && ((dt_type == UPLL_DT_CANDIDATE) || (dt_type == UPLL_DT_RUNNING)
          || (dt_type == UPLL_DT_STARTUP) || (dt_type == UPLL_DT_STATE))) {
      if (option1 == UNC_OPT1_NORMAL) {
        if (option2 == UNC_OPT2_NONE) {
          if (val_nwm_host != NULL) {
            ret_val = ValidateNwMonHostValue(val_nwm_host);
            if (ret_val != UPLL_RC_SUCCESS) {
              UPLL_LOG_DEBUG("Syntax check failed for NWMONITOR_HOST");
              return UPLL_RC_ERR_CFG_SYNTAX;
            }
            return UPLL_RC_SUCCESS;
          } else {
            UPLL_LOG_DEBUG("Value structure is optional");
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
      UPLL_LOG_TRACE(
          "Value structure is none for operation type:%d", operation);
      return UPLL_RC_SUCCESS;
    } else {
      UPLL_LOG_DEBUG("Invalid datatype(%d) and operation(%d)", dt_type,
          operation);
      return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT;
    }
  }
  return UPLL_RC_SUCCESS;
}
upll_rc_t NwMonitorHostMoMgr::ValidateNwMonHostValue(
    val_nwm_host_t *val_nwm_host, unc_keytype_operation_t operation) {
  UPLL_FUNC_TRACE;

  if (val_nwm_host->valid[UPLL_IDX_HEALTH_INTERVAL_NWMH] == UNC_VF_VALID) {
    if (!ValidateNumericRange(val_nwm_host->health_interval,
          (uint16_t) kMinNWMHHealthInterval,
          uint16_t(kMaxNWMHHealthInterval), true, true)) {
      UPLL_LOG_DEBUG("Syntax check failed.health_interval(%d)",
          val_nwm_host->health_interval);
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  }
#if 0
  else if ((val_nwm_host->valid[UPLL_IDX_HEALTH_INTERVAL_NWMH]
        == UNC_VF_VALID_NO_VALUE)
      && ((operation == UNC_OP_UPDATE) || (operation == UNC_OP_CREATE))) {
    val_nwm_host->health_interval = 0;
  }
#endif

  if (val_nwm_host->valid[UPLL_IDX_RECOVERY_INTERVAL_NWMH] == UNC_VF_VALID) {
    if (!ValidateNumericRange(val_nwm_host->recovery_interval,
          (uint16_t) kMinNWMHRecoveryInterval,
          (uint16_t) kMaxNWMHRecoveryInterval, true,
          true)) {
      UPLL_LOG_DEBUG("Syntax check failed.recovery_interval:(%d)",
          val_nwm_host->recovery_interval);
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  }
#if 0
  else if ((val_nwm_host->valid[UPLL_IDX_RECOVERY_INTERVAL_NWMH]
        == UNC_VF_VALID_NO_VALUE)
      && ((operation == UNC_OP_UPDATE) || (operation == UNC_OP_CREATE))) {
    val_nwm_host->recovery_interval = 0;
  }
#endif

  if (val_nwm_host->valid[UPLL_IDX_FAILURE_COUNT_NWMH] == UNC_VF_VALID) {
    if (!ValidateNumericRange(val_nwm_host->failure_count,
          (uint8_t) kMinNWMHFailureCount,
          (uint8_t) kMaxNWMHFailureCount, true, true)) {
      UPLL_LOG_DEBUG("Syntax check failed.failure_count(%d)",
          val_nwm_host->failure_count);
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  }
#if 0
  else if ((val_nwm_host->valid[UPLL_IDX_FAILURE_COUNT_NWMH]
        == UNC_VF_VALID_NO_VALUE)
      && ((operation == UNC_OP_UPDATE) || (operation == UNC_OP_CREATE))) {
    val_nwm_host->failure_count = 0;
  }
#endif

  if (val_nwm_host->valid[UPLL_IDX_RECOVERY_COUNT_NWMH] == UNC_VF_VALID) {
    if (!ValidateNumericRange(val_nwm_host->recovery_count,
          (uint8_t) kMinNWMHRecoveryCount,
          (uint8_t) kMaxNWMHRecoveryCount, true, true)) {
      UPLL_LOG_DEBUG("Syntax check failed.recovery_count(%d)",
          val_nwm_host->recovery_count);
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  }
#if 0
  else if ((val_nwm_host->valid[UPLL_IDX_RECOVERY_COUNT_NWMH]
        == UNC_VF_VALID_NO_VALUE)
      && ((operation == UNC_OP_UPDATE) || (operation == UNC_OP_CREATE))) {
    val_nwm_host->recovery_count = 0;
  }
#endif

  if (val_nwm_host->valid[UPLL_IDX_WAIT_TIME_NWMH] == UNC_VF_VALID) {
    if (!ValidateNumericRange(val_nwm_host->wait_time,
          (uint8_t) kMinNWMHWaitTime,
          (uint8_t) kMaxNWMHWaitTime, true, true)) {
      UPLL_LOG_DEBUG("Syntax check failed.wait_time(%d)",
          val_nwm_host->wait_time);
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  }
#if 0
  else if ((val_nwm_host->valid[UPLL_IDX_WAIT_TIME_NWMH]
        == UNC_VF_VALID_NO_VALUE)
      && ((operation == UNC_OP_UPDATE) || (operation == UNC_OP_CREATE))) {
    val_nwm_host->wait_time = 0;
  }
#endif

  return UPLL_RC_SUCCESS;
}

upll_rc_t NwMonitorHostMoMgr::ValidateNwMonHostKey(
    key_nwm_host_t *key_nwm_host,
    unc_keytype_operation_t operation) {
  UPLL_FUNC_TRACE;
  bool ret_val = UPLL_RC_SUCCESS;
  NwMonitorMoMgr *objnwmmgr =
    reinterpret_cast<NwMonitorMoMgr*>(const_cast<MoManager *>(GetMoManager(
            UNC_KT_VBR_NWMONITOR)));
  if (NULL == objnwmmgr) {
    UPLL_LOG_DEBUG("unable to get NwMonitorMoMgr");
    return UPLL_RC_ERR_GENERIC;
  }
  ret_val = objnwmmgr->ValidateNwMonKey(&(key_nwm_host->nwm_key));

  if (UPLL_RC_SUCCESS != ret_val) {
    UPLL_LOG_DEBUG("Syntax validation failed.Err Code- %d", ret_val);
    return UPLL_RC_ERR_CFG_SYNTAX;
  }
  if ((operation == UNC_OP_READ_SIBLING_BEGIN) ||
      (operation == UNC_OP_READ_SIBLING_COUNT)) {
    key_nwm_host->host_address.s_addr = 0x00000000;
  } else {
    if (key_nwm_host->host_address.s_addr == 0x00000000 ||
        key_nwm_host->host_address.s_addr == 0xffffffff) {
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  }
  return UPLL_RC_SUCCESS;
}
upll_rc_t NwMonitorHostMoMgr::ValNwMonHostAttributeSupportCheck(
    val_nwm_host_t *val_nwm_host,
    const uint8_t* attrs, unc_keytype_operation_t operation) {
  UPLL_FUNC_TRACE;
  if ((val_nwm_host->valid[UPLL_IDX_HEALTH_INTERVAL_NWMH] == UNC_VF_VALID)
      || (val_nwm_host->valid[UPLL_IDX_HEALTH_INTERVAL_NWMH]
        == UNC_VF_VALID_NO_VALUE)) {
    if (attrs[unc::capa::nwm_host::kCapHealthInterval] == 0) {
      val_nwm_host->valid[UPLL_IDX_HEALTH_INTERVAL_NWMH] = UNC_VF_INVALID;
      if (operation == UNC_OP_CREATE || operation == UNC_OP_UPDATE) {
        UPLL_LOG_DEBUG("HealthInterval not supported in pfc controller");
        return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
      }
    }
  }
  if ((val_nwm_host->valid[UPLL_IDX_RECOVERY_INTERVAL_NWMH] == UNC_VF_VALID)
      || (val_nwm_host->valid[UPLL_IDX_RECOVERY_INTERVAL_NWMH]
        == UNC_VF_VALID_NO_VALUE)) {
    if (attrs[unc::capa::nwm_host::kCapRecoveryInterval] == 0) {
      val_nwm_host->valid[UPLL_IDX_RECOVERY_INTERVAL_NWMH] = UNC_VF_INVALID;
      if (operation == UNC_OP_CREATE || operation == UNC_OP_UPDATE) {
        UPLL_LOG_DEBUG("RecoveryInterval not supported in pfc controller");
        return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
      }
    }
  }
  if ((val_nwm_host->valid[UPLL_IDX_FAILURE_COUNT_NWMH] == UNC_VF_VALID)
      || (val_nwm_host->valid[UPLL_IDX_FAILURE_COUNT_NWMH]
        == UNC_VF_VALID_NO_VALUE)) {
    if (attrs[unc::capa::nwm_host::kCapFailureCount] == 0) {
      val_nwm_host->valid[UPLL_IDX_FAILURE_COUNT_NWMH] = UNC_VF_INVALID;
      if (operation == UNC_OP_CREATE || operation == UNC_OP_UPDATE) {
        UPLL_LOG_DEBUG("FailureCount not supported in pfc controller");
        return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
      }
    }
  }
  if ((val_nwm_host->valid[UPLL_IDX_RECOVERY_COUNT_NWMH] == UNC_VF_VALID)
      || (val_nwm_host->valid[UPLL_IDX_RECOVERY_COUNT_NWMH]
        == UNC_VF_VALID_NO_VALUE)) {
    if (attrs[unc::capa::nwm_host::kCapRecoveryCount] == 0) {
      val_nwm_host->valid[UPLL_IDX_RECOVERY_COUNT_NWMH] = UNC_VF_INVALID;
      if (operation == UNC_OP_CREATE || operation == UNC_OP_UPDATE) {
        UPLL_LOG_DEBUG("RecoveryCount not supported in pfc controller");
        return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
      }
    }
  }
  if ((val_nwm_host->valid[UPLL_IDX_WAIT_TIME_NWMH] == UNC_VF_VALID)
      || (val_nwm_host->valid[UPLL_IDX_WAIT_TIME_NWMH]
        == UNC_VF_VALID_NO_VALUE)) {
    if (attrs[unc::capa::nwm_host::kCapWaitTime] == 0) {
      val_nwm_host->valid[UPLL_IDX_WAIT_TIME_NWMH] = UNC_VF_INVALID;
      if (operation == UNC_OP_CREATE || operation == UNC_OP_UPDATE) {
        UPLL_LOG_DEBUG("WaitTime not supported in pfc controller");
        return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
      }
    }
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t NwMonitorHostMoMgr::ValidateCapability(IpcReqRespHeader *req,
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
  const uint8_t *attrs = 0;

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
      UPLL_LOG_DEBUG("Invalid operation");
      break;
  }
  if (!result_code) {
    UPLL_LOG_DEBUG("keytype(%d) is not supported by controller(%s) "
        "for operation(%d)",
        ikey->get_key_type(), ctrlr_name, req->operation);
    return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
  }
  val_nwm_host_t *val_nwm_host = NULL;
  if ((ikey->get_cfg_val())
      && (ikey->get_cfg_val())->get_st_num() == IpctSt::kIpcStValNwmHost) {
    val_nwm_host =
      reinterpret_cast<val_nwm_host_t *>(ikey->get_cfg_val()->get_val());
  }
  if (val_nwm_host) {
    if (max_attrs > 0) {
      return ValNwMonHostAttributeSupportCheck(
          val_nwm_host, attrs, req->operation);
    } else {
      UPLL_LOG_DEBUG(
          "Attribute list is empty for operation %d", req->operation);
      return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
    }
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t NwMonitorHostMoMgr::GetControllerDomainId(ConfigKeyVal *ikey,
                                       upll_keytype_datatype_t dt_type,
                                       controller_domain_t *ctrlr_dom,
                                       DalDmlIntf *dmi) {
  DbSubOp dbop = {kOpReadSingle, kOpMatchNone, kOpInOutCtrlr | kOpInOutDomain};
  NwMonitorMoMgr *mgr = reinterpret_cast<NwMonitorMoMgr*>
       (const_cast<MoManager *>(GetMoManager(UNC_KT_VBR_NWMONITOR)));
  upll_rc_t result_code = mgr->ReadConfigDB(ikey, dt_type, UNC_OP_READ,
                                            dbop, dmi, MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Returning error %d", result_code);
    return result_code;
  }
  GET_USER_DATA_CTRLR_DOMAIN(ikey, *ctrlr_dom);
  if ((!ctrlr_dom->ctrlr || !ctrlr_dom->domain))
    return UPLL_RC_ERR_GENERIC;
  UPLL_LOG_DEBUG("ctrlr %s domain %s", ctrlr_dom->ctrlr, ctrlr_dom->domain);
  return UPLL_RC_SUCCESS;
}

upll_rc_t NwMonitorHostMoMgr::IsReferenced(IpcReqRespHeader *req,
                                           ConfigKeyVal *ikey,
                                           DalDmlIntf *dmi) {
  return UPLL_RC_SUCCESS;
}

}  // namespace kt_momgr
}  // namespace upll
}  // namespace unc
