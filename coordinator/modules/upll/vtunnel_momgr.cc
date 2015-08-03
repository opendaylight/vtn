/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include "vtunnel_momgr.hh"
#include "unc/vnpdriver_ipc_enum.h"

#define NUM_KEY_MAIN_TBL_ 4
namespace unc {
namespace upll {
namespace kt_momgr {

BindInfo VtunnelMoMgr::vtunnel_bind_info[] = {
  { uudst::vtunnel::kDbiVtnName, CFG_KEY,
    offsetof(key_vtunnel, vtn_key.vtn_name),
    uud::kDalChar, kMaxLenVtnName+1 },
  { uudst::vtunnel::kDbiVtunnelName, CFG_KEY,
    offsetof(key_vtunnel, vtunnel_name),
    uud::kDalChar, kMaxLenVnodeName+1 },
  { uudst::vtunnel::kDbiDesc,  CFG_VAL,
    offsetof(val_vtunnel, description),
    uud::kDalChar, kMaxLenDescription+1},
  { uudst::vtunnel::kDbiCtrlrName, CFG_VAL,
    offsetof(val_vtunnel, controller_id),
    uud::kDalChar, kMaxLenCtrlrId+1 },
  { uudst::vtunnel::kDbiCtrlrName, CK_VAL,
    offsetof(key_user_data_t, ctrlr_id),
    uud::kDalChar, kMaxLenCtrlrId+1 },
  { uudst::vtunnel::kDbiDomainId, CFG_VAL,
    offsetof(val_vtunnel, domain_id),
    uud::kDalChar, kMaxLenDomainId+1 },
  { uudst::vtunnel::kDbiDomainId, CK_VAL,
    offsetof(key_user_data_t, domain_id),
    uud::kDalChar, kMaxLenDomainId+1 },
  { uudst::vtunnel::kDbiUnderlayVtnName, CFG_VAL,
    offsetof(val_vtunnel, vtn_name),
    uud::kDalChar, kMaxLenVtnName+1 },
  { uudst::vtunnel::kDbiVtepgrpName, CFG_VAL,
    offsetof(val_vtunnel, vtep_grp_name),
    uud::kDalChar, kMaxLenVnodeName+1},
  { uudst::vtunnel::kDbiLabel, CFG_VAL,
    offsetof(val_vtunnel, label), uud::kDalUint32, 1},
  { uudst::vtunnel::kDbiOperStatus, ST_VAL,
    offsetof(val_db_vtunnel_st,
        vtunnel_val_st.oper_status),
    uud::kDalUint8, 1},
  { uudst::vtunnel::kDbiDownCount, ST_VAL,
    offsetof(val_db_vtunnel_st, down_count),
    uud::kDalUint32, 1},
  { uudst::vtunnel::kDbiFlags, CK_VAL,
    offsetof(key_user_data_t, flags), uud::kDalUint8, 1},
  { uudst::vtunnel::kDbiValidDesc, CFG_META_VAL,
    offsetof(val_vtunnel, valid[0]), uud::kDalUint8, 1},
  { uudst::vtunnel::kDbiValidCtrlrName, CFG_META_VAL,
    offsetof(val_vtunnel, valid[1]), uud::kDalUint8, 1},
  { uudst::vtunnel::kDbiValidDomainId, CFG_META_VAL,
    offsetof(val_vtunnel, valid[2]), uud::kDalUint8, 1},
  { uudst::vtunnel::kDbiValidUnderlayVtnName, CFG_META_VAL,
    offsetof(val_vtunnel, valid[3]), uud::kDalUint8, 1},
  { uudst::vtunnel::kDbiValidVtepgrpName, CFG_META_VAL,
    offsetof(val_vtunnel, valid[4]), uud::kDalUint8, 1},
  { uudst::vtunnel::kDbiValidLabel, CFG_META_VAL,
    offsetof(val_vtunnel, valid[5]), uud::kDalUint8, 1},
  { uudst::vtunnel::kDbiValidOperStatus, ST_META_VAL,
    offsetof(val_db_vtunnel_st, vtunnel_val_st.valid[0]),
    uud::kDalUint8, 1},
  { uudst::vtunnel::kDbiCsRowstatus, CS_VAL,
    offsetof(val_vtunnel, cs_row_status), uud::kDalUint8, 1},
  { uudst::vtunnel::kDbiCsDesc, CS_VAL,
    offsetof(val_vtunnel, cs_attr[0]), uud::kDalUint8, 1},
  { uudst::vtunnel::kDbiCsCtrlrName, CS_VAL,
    offsetof(val_vtunnel, cs_attr[1]), uud::kDalUint8, 1},
  { uudst::vtunnel::kDbiCsDomainId, CS_VAL,
    offsetof(val_vtunnel, cs_attr[2]), uud::kDalUint8, 1},
  { uudst::vtunnel::kDbiCsUnderlayVtnName, CS_VAL,
    offsetof(val_vtunnel, cs_attr[3]), uud::kDalUint8, 1},
  { uudst::vtunnel::kDbiCsVtepgrpName, CS_VAL,
    offsetof(val_vtunnel, cs_attr[4]), uud::kDalUint8, 1},
  { uudst::vtunnel::kDbiCsLabel, CS_VAL,
    offsetof(val_vtunnel, cs_attr[5]), uud::kDalUint8, 1}
};

BindInfo VtunnelMoMgr::conv_vtunnel_bind_info[] = {
  { uudst::convert_vtunnel::kDbiVtnName, CFG_KEY,
    offsetof(key_convert_vtunnel, vtn_key.vtn_name),
    uud::kDalChar, kMaxLenVtnName+1 },
  { uudst::convert_vtunnel::kDbiVtunnelName, CFG_KEY,
    offsetof(key_convert_vtunnel, convert_vtunnel_name),
    uud::kDalChar, kMaxLenConvertVnodeName+1 },
  { uudst::convert_vtunnel::kDbiCtrlrName, CK_VAL,
    offsetof(key_user_data_t, ctrlr_id),
    uud::kDalChar, kMaxLenCtrlrId+1 },
  { uudst::convert_vtunnel::kDbiDomainId, CK_VAL,
    offsetof(key_user_data_t, domain_id),
    uud::kDalChar, kMaxLenDomainId+1 },
  { uudst::convert_vtunnel::kDbiRefCount, CFG_VAL,
    offsetof(val_convert_vtunnel, ref_count), uud::kDalUint32, 1},
  { uudst::convert_vtunnel::kDbiLabel, CFG_VAL,
    offsetof(val_convert_vtunnel, label), uud::kDalUint32, 1},
  { uudst::convert_vtunnel::kDbiOperStatus, ST_VAL,
    offsetof(val_db_vtunnel_st,
        vtunnel_val_st.oper_status),
    uud::kDalUint8, 1},
  { uudst::convert_vtunnel::kDbiDownCount, ST_VAL,
    offsetof(val_db_vtunnel_st, down_count),
    uud::kDalUint32, 1},
  { uudst::convert_vtunnel::kDbiFlags, CK_VAL,
    offsetof(key_user_data_t, flags), uud::kDalUint8, 1},
  { uudst::convert_vtunnel::kDbiValidRefCount, CFG_META_VAL,
    offsetof(val_convert_vtunnel, valid[0]), uud::kDalUint8, 1},
  { uudst::convert_vtunnel::kDbiValidLabel, CFG_META_VAL,
    offsetof(val_convert_vtunnel, valid[1]), uud::kDalUint8, 1},
  { uudst::convert_vtunnel::kDbiValidOperStatus, ST_META_VAL,
    offsetof(val_db_vtunnel_st, vtunnel_val_st.valid[0]),
    uud::kDalUint8, 1},
  { uudst::convert_vtunnel::kDbiCsRowstatus, CS_VAL,
    offsetof(val_convert_vtunnel, cs_row_status), uud::kDalUint8, 1},
  { uudst::convert_vtunnel::kDbiCsRefCount, CS_VAL,
    offsetof(val_convert_vtunnel, cs_attr[0]), uud::kDalUint8, 1},
  { uudst::convert_vtunnel::kDbiCsLabel, CS_VAL,
    offsetof(val_convert_vtunnel, cs_attr[1]), uud::kDalUint8, 1}
};

unc_key_type_t VtunnelMoMgr::vtunnel_child[] = {
    UNC_KT_VTUNNEL_IF
};

VtunnelMoMgr::VtunnelMoMgr() {
  UPLL_FUNC_TRACE;
  Table *tbl = new Table(uudst::kDbiVtunnelTbl, UNC_KT_VTUNNEL,
      vtunnel_bind_info,
      IpctSt::kIpcStKeyVtunnel, IpctSt::kIpcStValVtunnel,
      uudst::vtunnel::kDbiVtunnelNumCols+2);
  ntable = MAX_MOMGR_TBLS;
  table = new Table *[ntable]();
  table[MAINTBL] = tbl;
  table[CONVERTTBL] = new Table(uudst::kDbiConvertVtunnelTbl, UNC_KT_VTUNNEL,
                      conv_vtunnel_bind_info, IpctSt::kIpcStKeyConvertVtunnel,
                      IpctSt::kIpcStValConvertVtunnel,
                      (uudst::convert_vtunnel::kDbiConvertVtunnelNumCols+2));

  table[RENAMETBL] = NULL;
  table[CTRLRTBL] = NULL;
  nchild = sizeof(vtunnel_child) / sizeof(*vtunnel_child);
  child = vtunnel_child;
#ifdef _STANDALONE_
  SetMoManager(UNC_KT_VTUNNEL, reinterpret_cast<MoMgr *>(this));
#endif
}

bool VtunnelMoMgr::IsValidKey(void *key, uint64_t index, MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  upll_rc_t ret_val = UPLL_RC_SUCCESS;
  if (tbl == CONVERTTBL) {
    key_convert_vtunnel *convert_vtun_key =
      reinterpret_cast<key_convert_vtunnel *>(key);
    switch (index) {
      case uudst::convert_vtunnel::kDbiVtnName:
        ret_val = ValidateKey(reinterpret_cast<char *>
            (convert_vtun_key->vtn_key.vtn_name),
            kMinLenVtnName, kMaxLenVtnName);
        if (ret_val != UPLL_RC_SUCCESS) {
          UPLL_LOG_TRACE("VTN Name is not valid(%d)", ret_val);
          return false;
        }
        break;
      case uudst::convert_vtunnel::kDbiVtunnelName:
        ret_val = ValidateKey(reinterpret_cast<char *>
            (convert_vtun_key->convert_vtunnel_name),
            kMinLenConvertVnodeName, kMaxLenConvertVnodeName);
        if (ret_val != UPLL_RC_SUCCESS) {
          UPLL_LOG_TRACE("Convert Vtunnel Name is not valid(%d)", ret_val);
          return false;
        }
        break;
      default:
        UPLL_LOG_TRACE("Invalid Key Index");
        return false;
    }
  } else {
    key_vtunnel *vtun_key = reinterpret_cast<key_vtunnel *>(key);
    switch (index) {
      case uudst::vtunnel::kDbiVtnName:
        ret_val = ValidateKey(reinterpret_cast<char *>
            (vtun_key->vtn_key.vtn_name),
            kMinLenVtnName, kMaxLenVtnName);
        if (ret_val != UPLL_RC_SUCCESS) {
          UPLL_LOG_TRACE("VTN Name is not valid(%d)", ret_val);
          return false;
        }
        break;
      case uudst::vtunnel::kDbiVtunnelName:
        ret_val = ValidateKey(reinterpret_cast<char *>(vtun_key->vtunnel_name),
            kMinLenVnodeName, kMaxLenVnodeName);
        if (ret_val != UPLL_RC_SUCCESS) {
          UPLL_LOG_TRACE("Vtunnel Name is not valid(%d)", ret_val);
          return false;
        }
        break;
      default:
        UPLL_LOG_TRACE("Invalid Key Index");
        return false;
    }
  }
  return true;
}

upll_rc_t VtunnelMoMgr::GetChildConfigKey(ConfigKeyVal *&okey,
                                          ConfigKeyVal *parent_key) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  void *pkey;
  key_vtunnel *vtunnel_key = static_cast<key_vtunnel *>
               (ConfigKeyVal::Malloc(sizeof(key_vtunnel)));
  if (vtunnel_key == NULL) return UPLL_RC_ERR_GENERIC;
  if (parent_key == NULL) {
    if (okey) delete okey;
    okey = new ConfigKeyVal(UNC_KT_VTUNNEL, IpctSt::kIpcStKeyVtunnel,
                            vtunnel_key, NULL);
    return UPLL_RC_SUCCESS;
  } else {
    pkey = parent_key->get_key();
  }
  if (!pkey) {
    FREE_IF_NOT_NULL(vtunnel_key);
    return UPLL_RC_ERR_GENERIC;
  }
  if (okey && (okey->get_key())) {
    FREE_IF_NOT_NULL(vtunnel_key);
    if (okey->get_key_type() != UNC_KT_VTUNNEL)
      return UPLL_RC_ERR_GENERIC;
    vtunnel_key = static_cast<key_vtunnel *>(okey->get_key());
  } else {
    okey = new ConfigKeyVal(UNC_KT_VTUNNEL, IpctSt::kIpcStKeyVtunnel,
        vtunnel_key, NULL);
    if (okey == NULL) {
      FREE_IF_NOT_NULL(vtunnel_key);
      return UPLL_RC_ERR_GENERIC;
    }
  }
  /* presumes MoMgrs receive only supported keytypes */
  switch (parent_key->get_key_type()) {
    case UNC_KT_VTUNNEL:
      uuu::upll_strncpy(vtunnel_key->vtunnel_name,
          static_cast<key_vtunnel *>
          (pkey)->vtunnel_name, (kMaxLenVnodeName+1));
      uuu::upll_strncpy(vtunnel_key->vtn_key.vtn_name,
          static_cast<key_vtunnel *>(pkey)->vtn_key.vtn_name,
          (kMaxLenVtnName+1));
      break;
    case UNC_KT_VTN:
    default:
      uuu::upll_strncpy(vtunnel_key->vtn_key.vtn_name,
          reinterpret_cast<key_vtn *>(pkey)->vtn_name,
          (kMaxLenVtnName+1));
      *(vtunnel_key->vtunnel_name) = ' ';
      vtunnel_key->vtunnel_name[1] = '\0';
  }
  SET_USER_DATA(okey, parent_key);
  return result_code;
}

upll_rc_t VtunnelMoMgr::GetParentConfigKey(ConfigKeyVal *&okey,
                             ConfigKeyVal *ikey ) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  void *pkey = (ikey)?ikey->get_key():NULL;
  if (!pkey) return UPLL_RC_ERR_GENERIC;
  unc_key_type_t ikey_type = ikey->get_key_type();
  if (ikey_type != UNC_KT_VTUNNEL)
     return UPLL_RC_ERR_GENERIC;
  key_vtn *vtn_key = static_cast<key_vtn *>
                     (ConfigKeyVal::Malloc(sizeof(key_vtn)));
  uuu::upll_strncpy(vtn_key->vtn_name,
       reinterpret_cast<key_vtunnel *>(pkey)->vtn_key.vtn_name,
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


upll_rc_t VtunnelMoMgr::AllocVal(ConfigVal *&ck_val,
    upll_keytype_datatype_t dt_type, MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  void *val;

  if (ck_val != NULL) return UPLL_RC_ERR_GENERIC;
  switch (tbl) {
    case MAINTBL:
      val = reinterpret_cast<void *>
        (ConfigKeyVal::Malloc(sizeof(val_vtunnel)));
      ck_val = new ConfigVal(IpctSt::kIpcStValVtunnel, val);
      if (!ck_val) {
        FREE_IF_NOT_NULL(reinterpret_cast<val_vtunnel *>(val));
        return UPLL_RC_ERR_GENERIC;
      }
      if (dt_type == UPLL_DT_STATE) {
        val = reinterpret_cast<void *>
          (ConfigKeyVal::Malloc(sizeof(val_db_vtunnel_st)));
        ConfigVal *ck_nxtval = new ConfigVal(IpctSt::kIpcStValVtunnelSt, val);
        if (!ck_nxtval) {
          delete ck_val;
          return UPLL_RC_ERR_GENERIC;
        }
        ck_val->AppendCfgVal(ck_nxtval);
      }
      break;
    case CONVERTTBL:
      val = reinterpret_cast<void *>
        (ConfigKeyVal::Malloc(sizeof(val_convert_vtunnel)));
      ck_val = new ConfigVal(IpctSt::kIpcStValConvertVtunnel, val);
      break;
    default:
      val = NULL;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtunnelMoMgr::DupConfigKeyVal(
    ConfigKeyVal *&okey, ConfigKeyVal *&req, MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  if (req == NULL) return UPLL_RC_ERR_GENERIC;
  if (okey != NULL) return UPLL_RC_ERR_GENERIC;
  if (req->get_key_type() != UNC_KT_VTUNNEL)
    return UPLL_RC_ERR_GENERIC;
  ConfigVal *tmp1 = NULL, *tmp = (req)->get_cfg_val();

  if (tmp) {
    if (tbl == MAINTBL) {
      val_vtunnel *ival = reinterpret_cast<val_vtunnel *>(GetVal(req));
      val_vtunnel *vtunnel_val = reinterpret_cast<val_vtunnel *>
          (ConfigKeyVal::Malloc(sizeof(val_vtunnel)));
      memcpy(vtunnel_val, ival, sizeof(val_vtunnel));
      tmp1 = new ConfigVal(IpctSt::kIpcStValVtunnel, vtunnel_val);
    } else if (tbl == CONVERTTBL) {
      val_convert_vtunnel *ival = reinterpret_cast<val_convert_vtunnel *>
        (GetVal(req));
      if (ival == NULL) {
        UPLL_LOG_ERROR("convert val is NULL");
        return UPLL_RC_ERR_GENERIC;
      }
      val_convert_vtunnel *conv_vtunnel_val =
          ConfigKeyVal::Malloc<val_convert_vtunnel>();
      memcpy(conv_vtunnel_val, ival, sizeof(val_convert_vtunnel));
      tmp1 = new ConfigVal(IpctSt::kIpcStValConvertVtunnel, conv_vtunnel_val);
    }
    tmp = tmp->get_next_cfg_val();
  };
  if (tmp) {
    if (tbl == MAINTBL || tbl == CONVERTTBL) {
      val_db_vtunnel_st *ival = static_cast<val_db_vtunnel_st *>
        (tmp->get_val());
      val_db_vtunnel_st *vtunnel_st = static_cast<val_db_vtunnel_st *>
        (ConfigKeyVal::Malloc(sizeof(val_db_vtunnel_st)));
      memcpy(vtunnel_st, ival, sizeof(val_db_vtunnel_st));
      ConfigVal *tmp2 = new ConfigVal(IpctSt::kIpcStValVtunnelSt,
          vtunnel_st);
      tmp1->AppendCfgVal(tmp2);
    }
  };
  void *tkey = (req != NULL)?(req)->get_key():NULL;
  if (!tkey) {
    UPLL_LOG_ERROR("Null tkey");
    DELETE_IF_NOT_NULL(tmp1);
    return UPLL_RC_ERR_GENERIC;
  }
  if (tbl == MAINTBL) {
    key_vtunnel *ikey = static_cast<key_vtunnel *>(tkey);
    key_vtunnel *vtunnel_key = static_cast<key_vtunnel *>
      (ConfigKeyVal::Malloc(sizeof(key_vtunnel)));
    memcpy(vtunnel_key, ikey, sizeof(key_vtunnel));
    okey = new ConfigKeyVal(UNC_KT_VTUNNEL, IpctSt::kIpcStKeyVtunnel,
        vtunnel_key, tmp1);
  } else if (tbl == CONVERTTBL) {
    key_convert_vtunnel *ikey = static_cast<key_convert_vtunnel *>(tkey);
    key_convert_vtunnel *conv_vtunnel_key =
        ConfigKeyVal::Malloc<key_convert_vtunnel>();
    memcpy(conv_vtunnel_key, ikey, sizeof(key_convert_vtunnel));
    okey = new ConfigKeyVal(UNC_KT_VTUNNEL, IpctSt::kIpcStKeyConvertVtunnel,
                            conv_vtunnel_key, tmp1);
  } else {
    UPLL_LOG_DEBUG("Received tbl name is not supported");
    DELETE_IF_NOT_NULL(tmp1);
    return UPLL_RC_ERR_GENERIC;
  }
  SET_USER_DATA(okey, req);
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtunnelMoMgr::UpdateConfigStatus(ConfigKeyVal *vtunnel_key,
                                       unc_keytype_operation_t op,
                                       uint32_t driver_result,
                                       ConfigKeyVal *upd_key,
                                       DalDmlIntf *dmi,
                                       ConfigKeyVal *ctrlr_key) {
  UPLL_FUNC_TRACE;
  val_db_vtunnel_st *vtunnelst_val;
  unc_keytype_configstatus_t cs_status =
     (driver_result == UPLL_RC_SUCCESS) ? UNC_CS_APPLIED : UNC_CS_NOT_APPLIED;
  UPLL_LOG_TRACE("DriverResult %d, ConfigStatus %d", driver_result, cs_status);
  val_vtunnel *vtunnel_val = static_cast<val_vtunnel *>(GetVal(vtunnel_key));
  if (vtunnel_val == NULL) return UPLL_RC_ERR_GENERIC;
  UPLL_LOG_TRACE("%s", (vtunnel_key->ToStrAll()).c_str());
  val_vtunnel *val_running = static_cast<val_vtunnel *>(GetVal(upd_key));
  if (op == UNC_OP_CREATE) {
    vtunnel_val->cs_row_status = cs_status;
    vtunnelst_val = reinterpret_cast<val_db_vtunnel_st *>
                    (ConfigKeyVal::Malloc(sizeof(val_db_vtunnel_st)));
    vtunnelst_val->vtunnel_val_st.oper_status =
                   (driver_result == UPLL_RC_ERR_CTR_DISCONNECTED)?
                    UPLL_OPER_STATUS_UNKNOWN:
                    UPLL_OPER_STATUS_UP;
    vtunnelst_val->vtunnel_val_st.valid[UPLL_IDX_OPER_STATUS_VTNLS] =
        UNC_VF_VALID;
    vtunnelst_val->down_count  = 0;
    vtunnelst_val->unknown_count = 0;
    vtunnel_key->AppendCfgVal(IpctSt::kIpcStValVtunnelSt, vtunnelst_val);
  } else if (op == UNC_OP_UPDATE) {
    void *val = reinterpret_cast<void *>(vtunnel_val);
    CompareValidValue(val, GetVal(upd_key), true);
    UPLL_LOG_TRACE("%s", (upd_key->ToStrAll()).c_str());
    vtunnel_val->cs_row_status = val_running->cs_row_status;
  } else {
    return UPLL_RC_ERR_GENERIC;
  }
  for (unsigned int loop = 0;
       loop < sizeof(vtunnel_val->valid)/sizeof(vtunnel_val->valid[0]);
       ++loop ) {
    if ((UNC_VF_VALID == vtunnel_val->valid[loop]) ||
           (UNC_VF_VALID_NO_VALUE == vtunnel_val->valid[loop])) {
      if (loop == UPLL_IDX_DESC_VTNL)
        vtunnel_val->cs_attr[loop] = UNC_CS_APPLIED;
      else
        vtunnel_val->cs_attr[loop] = cs_status;
    } else if ((vtunnel_val->valid[loop] == UNC_VF_INVALID) &&
             (UNC_OP_CREATE == op)) {
      vtunnel_val->cs_attr[loop] = UNC_CS_APPLIED;
    } else if ((vtunnel_val->valid[loop] == UNC_VF_INVALID) &&
             (UNC_OP_UPDATE == op)) {
      vtunnel_val->cs_attr[loop] = val_running->cs_attr[loop];
    }
  }
  return UPLL_RC_SUCCESS;
}

bool VtunnelMoMgr::FilterAttributes(void *&val1, void *val2,
                                    bool copy_to_running,
                                    unc_keytype_operation_t op) {
  UPLL_FUNC_TRACE;
  val_vtunnel_t *valtunnel1 = reinterpret_cast<val_vtunnel_t *>(val1);
  val_vtunnel_t *valtunnel2 = reinterpret_cast<val_vtunnel_t *>(val2);
  /* No need to configure description in controller. */
  valtunnel1->valid[UPLL_IDX_DESC_VTNL] = UNC_VF_INVALID;
  if (op == UNC_OP_UPDATE) {
    valtunnel1->valid[UPLL_IDX_CONTROLLER_ID_VTNL] = UNC_VF_INVALID;
    valtunnel1->valid[UPLL_IDX_DOMAIN_ID_VTNL] = UNC_VF_INVALID;
    valtunnel2->valid[UPLL_IDX_CONTROLLER_ID_VTNL] = UNC_VF_INVALID;
    valtunnel2->valid[UPLL_IDX_DOMAIN_ID_VTNL] = UNC_VF_INVALID;
  }
  if (op != UNC_OP_CREATE)
    return CompareValidValue(val1, val2, copy_to_running);
  return false;
}

bool VtunnelMoMgr::CompareValidValue(void *&val1, void *val2,
                                     bool copy_to_running) {
  UPLL_FUNC_TRACE;
  bool invalid_attr = true;
  val_vtunnel_t *valtunnel1 = reinterpret_cast<val_vtunnel_t *>(val1);
  val_vtunnel_t *valtunnel2 = reinterpret_cast<val_vtunnel_t *>(val2);
  for (unsigned int loop = 0;
       loop < sizeof(valtunnel1->valid)/sizeof(uint8_t); ++loop ) {
    if (UNC_VF_INVALID == valtunnel1->valid[loop] &&
        UNC_VF_VALID == valtunnel2->valid[loop])
      valtunnel1->valid[loop] = UNC_VF_VALID_NO_VALUE;
  }
  if (copy_to_running) {
    if (UNC_VF_VALID == valtunnel1->valid[UPLL_IDX_DESC_VTNL] &&
        UNC_VF_VALID == valtunnel2->valid[UPLL_IDX_DESC_VTNL]) {
      if (!strcmp(reinterpret_cast<char*>(valtunnel1->description),
                  reinterpret_cast<char*>(valtunnel2->description)))
        valtunnel1->valid[UPLL_IDX_DESC_VTNL] = UNC_VF_INVALID;
    }
    if (UNC_VF_VALID == valtunnel1->valid[UPLL_IDX_CONTROLLER_ID_VTNL]
        && UNC_VF_VALID == valtunnel2->valid[UPLL_IDX_CONTROLLER_ID_VTNL]) {
      if (!strcmp(reinterpret_cast<char*>(valtunnel1->controller_id),
                  reinterpret_cast<char*>(valtunnel2->controller_id)))
        valtunnel1->valid[UPLL_IDX_CONTROLLER_ID_VTNL] = UNC_VF_INVALID;
    }
    if (UNC_VF_VALID == valtunnel1->valid[UPLL_IDX_DOMAIN_ID_VTNL]
        && UNC_VF_VALID == valtunnel2->valid[UPLL_IDX_DOMAIN_ID_VTNL]) {
      if (!strcmp(reinterpret_cast<char*>(valtunnel1->domain_id),
                  reinterpret_cast<char*>(valtunnel2->domain_id)))
        valtunnel1->valid[UPLL_IDX_DOMAIN_ID_VTNL] = UNC_VF_INVALID;
    }
    if (UNC_VF_VALID == valtunnel1->valid[UPLL_IDX_VTN_NAME_VTNL] &&
        UNC_VF_VALID == valtunnel2->valid[UPLL_IDX_VTN_NAME_VTNL]) {
      if (!strcmp(reinterpret_cast<char*>(valtunnel1->vtn_name),
                  reinterpret_cast<char*>(valtunnel2->vtn_name)))
        valtunnel1->valid[UPLL_IDX_VTN_NAME_VTNL] = UNC_VF_INVALID;
    }
    if (UNC_VF_VALID == valtunnel1->valid[UPLL_IDX_VTEP_GRP_NAME_VTNL] &&
        UNC_VF_VALID == valtunnel2->valid[UPLL_IDX_VTEP_GRP_NAME_VTNL]) {
      if (!strcmp(reinterpret_cast<char*>(valtunnel1->vtep_grp_name),
                  reinterpret_cast<char*>(valtunnel2->vtep_grp_name)))
        valtunnel1->valid[UPLL_IDX_VTEP_GRP_NAME_VTNL] = UNC_VF_INVALID;
    }
    if (UNC_VF_VALID == valtunnel1->valid[UPLL_IDX_LABEL_VTNL] &&
        UNC_VF_VALID == valtunnel2->valid[UPLL_IDX_LABEL_VTNL]) {
      if (valtunnel1->label == valtunnel2->label)
        valtunnel1->valid[UPLL_IDX_LABEL_VTNL] = UNC_VF_INVALID;
    }
  }
  if (!copy_to_running)
    valtunnel1->valid[UPLL_IDX_DESC_VTNL] = UNC_VF_INVALID;
  for (unsigned int loop = 0;
       loop < sizeof(valtunnel1->valid) / sizeof(uint8_t); ++loop) {
    if ((UNC_VF_VALID == (uint8_t) valtunnel1->valid[loop]) ||
        (UNC_VF_VALID_NO_VALUE == (uint8_t) valtunnel1->valid[loop])) {
      invalid_attr = false;
      break;
    }
  }
  return invalid_attr;
}

upll_rc_t VtunnelMoMgr::UpdateAuditConfigStatus(
    unc_keytype_configstatus_t cs_status,
    uuc::UpdateCtrlrPhase phase,
    ConfigKeyVal *&ckv_running,
    DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  val_vtunnel_t *val = (ckv_running != NULL)?
                   reinterpret_cast<val_vtunnel_t *>
                       ((GetVal(ckv_running))):NULL;
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

/* Pure Virtual functions from MoMgrImpl */
upll_rc_t VtunnelMoMgr::GetControllerDomainId(ConfigKeyVal *ikey,
                                             controller_domain *ctrlr_dom) {
  UPLL_FUNC_TRACE;
  if (!ikey || !ctrlr_dom) {
    UPLL_LOG_INFO("Illegal parameter");
    return UPLL_RC_ERR_GENERIC;
  }
  val_vtunnel * temp_vtunnel = static_cast<val_vtunnel *>(GetVal(ikey));
  if (!temp_vtunnel) {
    UPLL_LOG_DEBUG("value null");
    return UPLL_RC_ERR_GENERIC;
  }
  if (temp_vtunnel->valid[UPLL_IDX_CONTROLLER_ID_VTNL] != UNC_VF_VALID ||
      !strlen(reinterpret_cast<char*>(temp_vtunnel->controller_id))) {
    ctrlr_dom->ctrlr = NULL;
    UPLL_LOG_TRACE("Ctrlr null");
  } else {
    SET_USER_DATA_CTRLR(ikey, temp_vtunnel->controller_id);
  }
  if (temp_vtunnel->valid[UPLL_IDX_DOMAIN_ID_VTNL] != UNC_VF_VALID ||
      !strlen(reinterpret_cast<char*>(temp_vtunnel->domain_id))) {
    ctrlr_dom->domain = NULL;
    UPLL_LOG_TRACE("Domain null");
  } else {
    SET_USER_DATA_DOMAIN(ikey, temp_vtunnel->domain_id);
    GET_USER_DATA_CTRLR_DOMAIN(ikey, *ctrlr_dom);
  }
  UPLL_LOG_DEBUG("ctrlr_dom %s %s", ctrlr_dom->ctrlr, ctrlr_dom->domain);
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtunnelMoMgr::GetVnodeName(ConfigKeyVal *ikey, uint8_t *&vtn_name,
                                uint8_t *&vnode_name) {
  UPLL_FUNC_TRACE;
  if (ikey == NULL) {
    UPLL_LOG_DEBUG("ConfigKeyVal is NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  key_vtunnel_t *vtunnel_key = static_cast<key_vtunnel_t *>
                               (ikey->get_key());
  if (vtunnel_key == NULL)
    return UPLL_RC_ERR_GENERIC;
  vtn_name = vtunnel_key->vtn_key.vtn_name;
  vnode_name = vtunnel_key->vtunnel_name;
  return  UPLL_RC_SUCCESS;
}

upll_rc_t VtunnelMoMgr::ValidateMessage(IpcReqRespHeader *req,
    ConfigKeyVal *ikey) {
  UPLL_FUNC_TRACE;
  upll_rc_t ret_val = UPLL_RC_ERR_GENERIC;
  if (!ikey || !req || !(ikey->get_key())) {
    UPLL_LOG_DEBUG("IpcReqRespHeader or ConfigKeyVal is Null");
    return UPLL_RC_ERR_BAD_REQUEST;
  }
  unc_key_type_t keytype = ikey->get_key_type();
  if (UNC_KT_VTUNNEL != keytype) {
    UPLL_LOG_DEBUG("Invalid keytype. Keytype- %d", keytype);
    return UPLL_RC_ERR_BAD_REQUEST;
  }
  if (ikey->get_st_num() != IpctSt::kIpcStKeyVtunnel) {
    UPLL_LOG_DEBUG("Invalid struct received.Expected struct-kIpcStKeyVtunnel, "
        "received struct -%s ", reinterpret_cast<const char *>
        (IpctSt::GetIpcStdef(ikey->get_st_num())));
    return UPLL_RC_ERR_BAD_REQUEST;
  }
  key_vtunnel_t *key_vtunnel = reinterpret_cast<key_vtunnel_t *>
    (ikey->get_key());
  val_vtunnel_t *val_vtunnel = NULL;
  if ((ikey->get_cfg_val()) &&
      ((ikey->get_cfg_val())->get_st_num() == IpctSt::kIpcStValVtunnel)) {
    val_vtunnel = reinterpret_cast<val_vtunnel_t *>
      (ikey->get_cfg_val()->get_val());
  }
  uint32_t dt_type   = req->datatype;
  uint32_t operation = req->operation;
  uint32_t option1   = req->option1;
  uint32_t option2   = req->option2;

  ret_val = ValidateVTunnelKey(key_vtunnel, operation);
  if (ret_val != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Syntax check failed for VTUNNEL key structure");
    return UPLL_RC_ERR_CFG_SYNTAX;
  } else {
    if (((operation == UNC_OP_CREATE) || (operation == UNC_OP_UPDATE)) &&
        (dt_type == UPLL_DT_CANDIDATE)) {
      if (val_vtunnel != NULL) {
        ret_val = ValidateVTunnelValue(val_vtunnel, operation);
        if (ret_val != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG(
              "val struct Validation failed for CREATE/UPDATE op");
          return UPLL_RC_ERR_CFG_SYNTAX;
        }
        return UPLL_RC_SUCCESS;
      } else {
        UPLL_LOG_DEBUG("Val struct is mandatory for CREATE/UPDATE op");
        return UPLL_RC_ERR_BAD_REQUEST;
      }
    } else if (((operation == UNC_OP_READ) ||
          (operation == UNC_OP_READ_SIBLING) ||
          (operation == UNC_OP_READ_SIBLING_BEGIN)) &&
        ((dt_type == UPLL_DT_CANDIDATE) || (dt_type == UPLL_DT_RUNNING) ||
         (dt_type == UPLL_DT_STARTUP) || (dt_type == UPLL_DT_STATE))) {
      if (option1 == UNC_OPT1_NORMAL) {
        if (option2 == UNC_OPT2_NONE) {
          if (val_vtunnel != NULL) {
            ret_val = ValidateVTunnelValue(val_vtunnel);
            if (ret_val != UPLL_RC_SUCCESS) {
              UPLL_LOG_DEBUG("val struct Validation failed for READ op");
              return ret_val;
            }
            return UPLL_RC_SUCCESS;
          } else {
            UPLL_LOG_TRACE("Value structure is an optional for READ op");
            return UPLL_RC_SUCCESS;
          }
        } else {
          UPLL_LOG_TRACE("option2 is not matching");
          return UPLL_RC_ERR_INVALID_OPTION2;
        }
      } else {
        UPLL_LOG_TRACE("option1 is not matching");
        return UPLL_RC_ERR_INVALID_OPTION1;
      }
    } else if ((operation == UNC_OP_DELETE) ||
        (operation == UNC_OP_READ_SIBLING_COUNT) ||
        (((operation == UNC_OP_READ_NEXT) ||
        (operation == UNC_OP_READ_BULK)) &&
        ((dt_type == UPLL_DT_CANDIDATE) ||
          (dt_type == UPLL_DT_RUNNING) ||
          (dt_type == UPLL_DT_STARTUP)))) {
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

upll_rc_t VtunnelMoMgr::ValidateAttribute(ConfigKeyVal *ikey, DalDmlIntf *dmi,
                                        IpcReqRespHeader *req) {
  UPLL_FUNC_TRACE;
  upll_rc_t r_code = UPLL_RC_SUCCESS;
  if (!ikey || !(ikey->get_cfg_val()) ||
      ((ikey->get_cfg_val())->get_st_num() != IpctSt::kIpcStValVtunnel)) {
    UPLL_LOG_TRACE("Key or Valis null");
    return UPLL_RC_ERR_GENERIC;
  }
  val_vtunnel_t *val_vtunnel = static_cast<val_vtunnel_t *>(GetVal(ikey));
  if (!val_vtunnel) return UPLL_RC_ERR_GENERIC;
  // Validating Underlay VTN
  MoMgrImpl *mgr = reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>
                                            (GetMoManager(UNC_KT_VTN)));
  if (!mgr) return UPLL_RC_ERR_GENERIC;
  if (val_vtunnel->valid[UPLL_IDX_VTN_NAME_VTNL] == UNC_VF_VALID) {
    // Validating Underlay Vtn is same as Overlay Vtn
    key_vtunnel_t *vtunnel_key = reinterpret_cast<key_vtunnel_t *>
                                                  (ikey->get_key());
    if (!strcmp(reinterpret_cast<char*>(vtunnel_key->vtn_key.vtn_name),
                reinterpret_cast<char*>(val_vtunnel->vtn_name))) {
      UPLL_LOG_DEBUG("UnderLay VTN is same as Overlay VTN");
      return UPLL_RC_ERR_CFG_SEMANTIC;
    }
    // Existence of Underlay VTN
    key_vtn *vtnkey = static_cast<key_vtn *>
                     (ConfigKeyVal::Malloc(sizeof(key_vtn)));
    uuu::upll_strncpy(vtnkey->vtn_name,
                      val_vtunnel->vtn_name,
                     (kMaxLenVtnName+1));
    ConfigKeyVal *vtn_ck = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn,
                           vtnkey,  NULL);
    if (vtn_ck == NULL) {
      FREE_IF_NOT_NULL(vtnkey);
      return UPLL_RC_ERR_GENERIC;
    }
    r_code = mgr->UpdateConfigDB(vtn_ck, UPLL_DT_CANDIDATE, UNC_OP_READ,
                                      dmi, MAINTBL);
    if (UPLL_RC_ERR_INSTANCE_EXISTS != r_code) {
      UPLL_LOG_DEBUG("UpdateConfigDB Return Failure = %d ", r_code);
      if (r_code == UPLL_RC_ERR_NO_SUCH_INSTANCE)
        UPLL_LOG_DEBUG("UnderLay Vtn does not exist");
      delete vtn_ck;
      r_code = (r_code == UPLL_RC_ERR_NO_SUCH_INSTANCE)?
                          UPLL_RC_ERR_CFG_SEMANTIC:r_code;
      return r_code;
    } else {
        r_code = UPLL_RC_SUCCESS;
        delete vtn_ck;
    }
  } else {
    if (val_vtunnel->valid[UPLL_IDX_VTEP_GRP_NAME_VTNL] == UNC_VF_VALID) {
      UPLL_LOG_DEBUG("VTN name must be Valid for VTEP GRP");
      return UPLL_RC_ERR_GENERIC;
    }
    r_code = UPLL_RC_SUCCESS;
  }
  if (val_vtunnel->valid[UPLL_IDX_VTEP_GRP_NAME_VTNL] != UNC_VF_VALID) {
    r_code = UPLL_RC_SUCCESS;
  } else {
    r_code = GetVtepGroup(val_vtunnel, UPLL_DT_CANDIDATE, dmi);
    if (r_code!= UPLL_RC_ERR_INSTANCE_EXISTS) {
      UPLL_LOG_DEBUG("Error in fetching the VtepGrp data from DB %d", r_code);
      if (r_code == UPLL_RC_ERR_NO_SUCH_INSTANCE)
        UPLL_LOG_DEBUG("VtepGroup does not exist");
      r_code = (r_code == UPLL_RC_ERR_NO_SUCH_INSTANCE)?
                          UPLL_RC_ERR_CFG_SEMANTIC:r_code;
      return r_code;
    } else {
        r_code = UPLL_RC_SUCCESS;
    }
  }
  UPLL_LOG_TRACE("Vtunnel ValidateAttribute returned %d", r_code);
  r_code = VnodeMoMgr::ValidateAttribute(ikey, dmi, req);
  UPLL_LOG_TRACE("Vnode ValidateAttribute returned %d", r_code);
  return r_code;
}

upll_rc_t VtunnelMoMgr::GetVtepGroup(val_vtunnel_t *vtunnelVal,
                                     upll_keytype_datatype_t dt_type,
                                     DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  MoMgrImpl *mgr = reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>
                                            (GetMoManager(UNC_KT_VTEP_GRP)));
  if (!mgr) return UPLL_RC_ERR_GENERIC;
  key_vtep_grp *vtepgrp_key = reinterpret_cast<key_vtep_grp *>
                               (ConfigKeyVal::Malloc(sizeof(key_vtep_grp)));
  uuu::upll_strncpy(vtepgrp_key->vtn_key.vtn_name,
                    vtunnelVal->vtn_name,
                    (kMaxLenVtnName+1));
  uuu::upll_strncpy(vtepgrp_key->vtepgrp_name,
                    vtunnelVal->vtep_grp_name,
                    (kMaxLenVnodeName+1));
  ConfigKeyVal *vtepgrp_ckv = new ConfigKeyVal(UNC_KT_VTEP_GRP,
                              IpctSt::kIpcStKeyVtepGrp,
                              vtepgrp_key, NULL);
  if (vtepgrp_ckv == NULL) {
    FREE_IF_NOT_NULL(vtepgrp_key);
    return UPLL_RC_ERR_GENERIC;
  } else {
    result_code = mgr->UpdateConfigDB(vtepgrp_ckv, dt_type, UNC_OP_READ,
                                      dmi, MAINTBL);
    if (UPLL_RC_ERR_INSTANCE_EXISTS != result_code) {
      UPLL_LOG_DEBUG("UpdateConfigDB Return Failure = %d ", result_code);
      delete vtepgrp_ckv;
      return result_code;
    }
  }
  delete vtepgrp_ckv;
  return result_code;
}

upll_rc_t VtunnelMoMgr::ValidateVTunnelValue(val_vtunnel_t *val_vtunnel,
    uint32_t operation) {
  UPLL_FUNC_TRACE;
  bool ret_val = false;

  // Attribute syntax validation
  for (unsigned int valid_index = 0;
       valid_index < sizeof(val_vtunnel->valid) / sizeof(val_vtunnel->valid[0]);
       valid_index++) {
    if (val_vtunnel->valid[valid_index] == UNC_VF_VALID) {
      switch (valid_index) {
        case UPLL_IDX_CONTROLLER_ID_VTNL:
          ret_val = ValidateString(val_vtunnel->controller_id,
                                   kMinLenCtrlrId, kMaxLenCtrlrId);
          break;
        case UPLL_IDX_DOMAIN_ID_VTNL:
          ret_val = ValidateDefaultStr(val_vtunnel->domain_id,
                                       kMinLenDomainId, kMaxLenDomainId);
          break;
        case   UPLL_IDX_VTN_NAME_VTNL:
          ret_val = ValidateString(val_vtunnel->vtn_name,
                                 kMinLenVtnName, kMaxLenVtnName);
          break;
        case UPLL_IDX_DESC_VTNL:
          ret_val = ValidateDesc(val_vtunnel->description,
                                 kMinLenDescription, kMaxLenDescription);
          break;
        case UPLL_IDX_VTEP_GRP_NAME_VTNL:
          ret_val = ValidateString(val_vtunnel->vtep_grp_name,
                                   kMinLenVnodeName, kMaxLenVnodeName);
          break;
        case UPLL_IDX_LABEL_VTNL:
          ret_val = true;
          break;
      }
      if (!ret_val) {
        return UPLL_RC_ERR_CFG_SYNTAX;
      }
    }
  }

  // Additional checks
  for (unsigned int valid_index = 0;
       valid_index < sizeof(val_vtunnel->valid) / sizeof(val_vtunnel->valid[0]);
       valid_index++) {
    uint8_t flag = val_vtunnel->valid[valid_index];
    switch (operation) {
      case UNC_OP_CREATE:
        {
          switch (valid_index) {
            case UPLL_IDX_CONTROLLER_ID_VTNL:
            case UPLL_IDX_DOMAIN_ID_VTNL:
              if ((flag == UNC_VF_INVALID || flag == UNC_VF_VALID_NO_VALUE)) {
                UPLL_LOG_DEBUG("controller_id or domain_id attribute is invalid"
                               " or valid_no_value");
                return UPLL_RC_ERR_CFG_SYNTAX;
              }
              break;
            case UPLL_IDX_DESC_VTNL:
            case UPLL_IDX_VTN_NAME_VTNL:
            case UPLL_IDX_VTEP_GRP_NAME_VTNL:
              break;
            case UPLL_IDX_LABEL_VTNL:
              if ((flag == UNC_VF_INVALID || flag == UNC_VF_VALID_NO_VALUE)) {
                UPLL_LOG_DEBUG("Label attribute is invalid or valid_no_value");
                return UPLL_RC_ERR_CFG_SYNTAX;
              }
              break;
            default:
              break;
          }
        }
        break;
      case UNC_OP_UPDATE:
        {
          switch (valid_index) {
            case UPLL_IDX_CONTROLLER_ID_VTNL:
            case UPLL_IDX_DOMAIN_ID_VTNL:
              if (flag == UNC_VF_VALID_NO_VALUE) {
                UPLL_LOG_DEBUG("controller_id or domain_id flag is "
                               "valid_no_value");
                return UPLL_RC_ERR_CFG_SYNTAX;
              }
              break;
            case UPLL_IDX_DESC_VTNL:
            case UPLL_IDX_VTN_NAME_VTNL:
            case UPLL_IDX_VTEP_GRP_NAME_VTNL:
            case UPLL_IDX_LABEL_VTNL:
            default:
              break;
          }
        }
        break;
    }
  }

  // Resets
  for (unsigned int valid_index = 0;
       valid_index < sizeof(val_vtunnel->valid) / sizeof(val_vtunnel->valid[0]);
       valid_index++) {
    uint8_t flag = val_vtunnel->valid[valid_index];
    if (flag != UNC_VF_INVALID && flag != UNC_VF_VALID_NO_VALUE)
      continue;

    switch (valid_index) {
      case UPLL_IDX_CONTROLLER_ID_VTNL:
        StringReset(val_vtunnel->controller_id);
        break;
      case UPLL_IDX_DOMAIN_ID_VTNL:
        StringReset(val_vtunnel->domain_id);
        break;
      case UPLL_IDX_DESC_VTNL:
        StringReset(val_vtunnel->description);
        break;
      case UPLL_IDX_VTN_NAME_VTNL:
        StringReset(val_vtunnel->vtn_name);
        break;
      case UPLL_IDX_VTEP_GRP_NAME_VTNL:
        StringReset(val_vtunnel->vtep_grp_name);
        break;
      default:
        UPLL_LOG_TRACE("Never here");
        break;
    }
  }
  return UPLL_RC_SUCCESS;
}


upll_rc_t VtunnelMoMgr::CtrlrIdAndDomainIdUpdationCheck(ConfigKeyVal *ikey,
                                                   ConfigKeyVal *okey) {
  UPLL_FUNC_TRACE;
  val_vtunnel *vtun_val = reinterpret_cast<val_vtunnel *>(GetVal(ikey));
  val_vtunnel *vtun_val1 = reinterpret_cast<val_vtunnel *>(GetVal(okey));
  if (vtun_val->valid[UPLL_IDX_CONTROLLER_ID_VTNL] == UNC_VF_VALID) {
    if (strncmp(reinterpret_cast<const char *>(vtun_val->controller_id),
                reinterpret_cast<const char *>(vtun_val1->controller_id),
                kMaxLenCtrlrId+1)) {
      UPLL_LOG_DEBUG("controller id comparision failed");
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  }
  if (vtun_val->valid[UPLL_IDX_DOMAIN_ID_VTNL] == UNC_VF_VALID) {
    if (strncmp(reinterpret_cast<const char *>(vtun_val->domain_id),
                reinterpret_cast<const char *>(vtun_val1->domain_id),
                kMaxLenDomainId+1)) {
      UPLL_LOG_DEBUG("domain id comparision failed");
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  }
  return UPLL_RC_SUCCESS;
}
upll_rc_t VtunnelMoMgr::ValidateVTunnelKey(key_vtunnel_t *key_vtunnel,
                        uint32_t operation) {
  UPLL_FUNC_TRACE;
  upll_rc_t ret_val = UPLL_RC_SUCCESS;
  ret_val = ValidateKey(
  reinterpret_cast<char *>(key_vtunnel->vtn_key.vtn_name),
      kMinLenVtnName, kMaxLenVtnName);
  if (ret_val != UPLL_RC_SUCCESS) {
    UPLL_LOG_INFO("Vtn Name syntax check failed."
                  "Received VTN Name - %s",
                  key_vtunnel->vtn_key.vtn_name);
    return UPLL_RC_ERR_CFG_SYNTAX;
  }
  if ((operation != UNC_OP_READ_SIBLING_BEGIN) &&
      (operation != UNC_OP_READ_SIBLING_COUNT)) {
    UPLL_LOG_TRACE("UNC_KT_VTUNNEL: vtunnel_name (%s)",
                    key_vtunnel->vtunnel_name);
    ret_val = ValidateKey(reinterpret_cast<char *>(key_vtunnel->vtunnel_name),
              kMinLenVnodeName, kMaxLenVnodeName);
    if (ret_val != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG(
        "Syntax check failed.vtunnel_name- %s", key_vtunnel->vtunnel_name);
      return ret_val;
    }
  } else {
    UPLL_LOG_TRACE("Operation is %d", operation);
    StringReset(key_vtunnel->vtunnel_name);
  }
  UPLL_LOG_TRACE("key structure validation successful for VTUNNEL keytype");
  return ret_val;
}
upll_rc_t VtunnelMoMgr::ValVTunnelAttributeSupportCheck(
    val_vtunnel_t *val_vtunnel,
    const uint8_t* attrs, unc_keytype_operation_t operation) {
  UPLL_FUNC_TRACE;
  /*
     if ((val_vtunnel->valid[UPLL_IDX_DESC_VTNL] == UNC_VF_VALID)
     || (val_vtunnel->valid[UPLL_IDX_DESC_VTNL] == UNC_VF_VALID_NO_VALUE)) {
     if (attrs[unc::capa::vtunnel::kCapDesc] == 0) {
     val_vtunnel->valid[UPLL_IDX_DESC_VTNL] = UNC_VF_INVALID;
     if (operation == UNC_OP_CREATE || operation == UNC_OP_UPDATE) {
     UPLL_LOG_INFO("Desc is not supported for PFC Controller");
     return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
     }
     }
     }
     */
  if ((val_vtunnel->valid[UPLL_IDX_VTN_NAME_VTNL] == UNC_VF_VALID)
      || (val_vtunnel->valid[UPLL_IDX_VTN_NAME_VTNL]
        == UNC_VF_VALID_NO_VALUE)) {
    if (attrs[unc::capa::vtunnel::kCapVtepName] == 0) {
      val_vtunnel->valid[UPLL_IDX_VTN_NAME_VTNL] = UNC_VF_INVALID;
      if (operation == UNC_OP_CREATE || operation == UNC_OP_UPDATE) {
        UPLL_LOG_INFO("VtepName is not supported for PFC Controller");
        return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
      }
    }
  }
  if ((val_vtunnel->valid[UPLL_IDX_VTEP_GRP_NAME_VTNL] == UNC_VF_VALID)
      || (val_vtunnel->valid[UPLL_IDX_VTEP_GRP_NAME_VTNL]
        == UNC_VF_VALID_NO_VALUE)) {
    if (attrs[unc::capa::vtunnel::kCapVtepGrpName] == 0) {
      val_vtunnel->valid[UPLL_IDX_VTEP_GRP_NAME_VTNL] =
        UNC_VF_INVALID;
      if (operation == UNC_OP_CREATE || operation == UNC_OP_UPDATE) {
        UPLL_LOG_INFO("VtepGrpName is not supported for PFC Controller");
        return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
      }
    }
  }
  if ((val_vtunnel->valid[UPLL_IDX_LABEL_VTNL] == UNC_VF_VALID)
      || (val_vtunnel->valid[UPLL_IDX_LABEL_VTNL] == UNC_VF_VALID_NO_VALUE)) {
    if (attrs[unc::capa::vtunnel::kCapLabel] == 0) {
      val_vtunnel->valid[UPLL_IDX_LABEL_VTNL] = UNC_VF_INVALID;
      if (operation == UNC_OP_CREATE || operation == UNC_OP_UPDATE) {
        UPLL_LOG_INFO("Label is not supported for PFC Controller");
        return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
      }
    }
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtunnelMoMgr::ValidateCapability(IpcReqRespHeader *req,
    ConfigKeyVal *ikey, const char *ctrlr_name) {
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
    case UNC_OP_UPDATE:
      result_code = GetUpdateCapability(ctrlr_name,
          ikey->get_key_type(), &max_attrs, &attrs);
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
  val_vtunnel_t *val_vtunnel = NULL;
  if ((ikey->get_cfg_val()) &&
      ((ikey->get_cfg_val())->get_st_num() == IpctSt::kIpcStValVtunnel)) {
    val_vtunnel =
      reinterpret_cast<val_vtunnel_t *> (ikey->get_cfg_val()->get_val());
  }
  if (val_vtunnel) {
    if (max_attrs > 0) {
      return ValVTunnelAttributeSupportCheck(val_vtunnel, attrs,
                                             req->operation);
    } else {
      UPLL_LOG_DEBUG("Attribute list is empty for operation %d",
                                                 req->operation);
      return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
    }
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtunnelMoMgr::CreateVnodeConfigKey(ConfigKeyVal *ikey,
    ConfigKeyVal *&okey) {
  if (ikey == NULL)
    return UPLL_RC_ERR_GENERIC;

  key_vtunnel * temp_key_vtunnel = static_cast<key_vtunnel *>
                                   (ConfigKeyVal::Malloc(sizeof(key_vtunnel)));
  uuu::upll_strncpy(temp_key_vtunnel->vtn_key.vtn_name,
                    static_cast<key_vtunnel*>(ikey->get_key())->
                     vtn_key.vtn_name,
                    (kMaxLenVtnName+1));
  uuu::upll_strncpy(temp_key_vtunnel->vtunnel_name,
                    static_cast<key_vtunnel*>
                    (ikey->get_key())->vtunnel_name,
                    (kMaxLenVnodeName+1));

  okey = new ConfigKeyVal(UNC_KT_VTUNNEL, IpctSt::kIpcStKeyVtunnel,
                          reinterpret_cast<void *>(temp_key_vtunnel),
                          NULL);
  return UPLL_RC_SUCCESS;
}

/*
upll_rc_t VtunnelMoMgr::CopyToConfigKey(ConfigKeyVal *&okey,
                                        ConfigKeyVal *ikey) {
  if ( !ikey || !(ikey->get_key()) )
    return UPLL_RC_ERR_GENERIC;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  key_rename_vnode_info *key_rename = reinterpret_cast<key_rename_vnode_info *>
                                                      (ikey->get_key());
  key_vtunnel_t* vtunnel_key = reinterpret_cast<key_vtunnel_t *>
                                (ConfigKeyVal::Malloc(sizeof(key_vtunnel_t)));
  if (!vtunnel_key)
     return UPLL_RC_ERR_GENERIC;
  if (!strlen(reinterpret_cast<char *>(key_rename->old_unc_vtn_name))) {
     FREE_IF_NOT_NULL(vtunnel_key);
     return UPLL_RC_ERR_GENERIC;
  }
  uuu::upll_strncpy(vtunnel_key ->vtn_key.vtn_name,
                    key_rename->old_unc_vtn_name,
                    (kMaxLenVtnName+1));
  okey = new ConfigKeyVal(UNC_KT_VTUNNEL, IpctSt::kIpcStKeyVtunnel,
                          vtunnel_key, NULL);
  if (!okey) {
    FREE_IF_NOT_NULL(vtunnel_key);
    return UPLL_RC_ERR_GENERIC;
  }
  return result_code;
}
*/

upll_rc_t VtunnelMoMgr::IsReferenced(IpcReqRespHeader *req,
                                     ConfigKeyVal *ikey,
                                     DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  if (!ikey || !(ikey->get_key()) || !dmi)
    return UPLL_RC_ERR_GENERIC;
  MoMgrImpl *mgr = reinterpret_cast<MoMgrImpl *>(const_cast<MoManager*>
                       (GetMoManager(UNC_KT_VTUNNEL_IF)));
  result_code = mgr->IsReferenced(req, ikey, dmi);
  result_code = (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE)?
                    UPLL_RC_SUCCESS:result_code;
  // Success / NoSuchInstance / Semantic
  UPLL_LOG_DEBUG("IsReferenced result code (%d)", result_code);
  return result_code;
}

upll_rc_t VtunnelMoMgr::AdaptValToVtnService(ConfigKeyVal *ikey,
                             AdaptType adapt_type) {
  UPLL_FUNC_TRACE;
  if (!ikey) {
    UPLL_LOG_DEBUG("Invalid ikey");
    return UPLL_RC_ERR_GENERIC;
  }
  key_vtunnel *vtun_key = reinterpret_cast<key_vtunnel*>(ikey->get_key());
  while (ikey) {
    ConfigVal *cval = ikey->get_cfg_val();
    if (!cval) {
      UPLL_LOG_DEBUG("Config Val is Null");
      return UPLL_RC_ERR_GENERIC;
    }
    while (cval) {
      if (IpctSt::kIpcStValVtunnelSt == cval->get_st_num()) {
        controller_domain ctrlr_dom = {NULL, NULL};
        GET_USER_DATA_CTRLR_DOMAIN(ikey, ctrlr_dom);
        CheckOperStatus<val_vtunnel_st>(vtun_key->vtn_key.vtn_name,
                                        cval, UNC_KT_VTUNNEL, ctrlr_dom);
      }
      cval = cval->get_next_cfg_val();
    }
    if (adapt_type == ADAPT_ONE)
      break;
    ikey = ikey->get_next_cfg_key_val();
  }
  return UPLL_RC_SUCCESS;
}
upll_rc_t VtunnelMoMgr::ConvertVtunnel(ConfigKeyVal *ikey,
                                       uint8_t *unified_vbr_name,
                                       unc_keytype_operation_t op,
                                       TcConfigMode config_mode,
                                       string vtn_name,
                                       DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_ERR_GENERIC;

  // validate the input
  if (!ikey || !dmi || !ikey->get_key()) {
    UPLL_LOG_DEBUG("IpcReqRespHeader or ConfigKeyVal or DalDmlIntf is Null");
    return UPLL_RC_ERR_BAD_REQUEST;
  }

  unc_key_type_t keytype = ikey->get_key_type();
  if (UNC_KT_VTUNNEL != keytype) {
    UPLL_LOG_DEBUG("Invalid keytype. Keytype- %d", keytype);
    return UPLL_RC_ERR_BAD_REQUEST;
  }

  if (ikey->get_st_num() != IpctSt::kIpcStKeyConvertVtunnel) {
    UPLL_LOG_DEBUG("Invalid struct received."
        "Expected struct-kIpcStConvertKeyVtunnel, "
        "received struct -%s ", reinterpret_cast<const char *>
        (IpctSt::GetIpcStdef(ikey->get_st_num())));
    return UPLL_RC_ERR_BAD_REQUEST;
  }

  // Fetch the ConfigKey
  key_convert_vtunnel *con_key_tun =  reinterpret_cast<key_convert_vtunnel *>
    (ikey->get_key());

  // Fetch the Configval
  val_convert_vtunnel *con_val_tun =  reinterpret_cast<val_convert_vtunnel *>
    (ikey->get_cfg_val()->get_val());

  controller_domain ctrlr_dom;
  memset(&ctrlr_dom, 0, sizeof(controller_domain));
  GET_USER_DATA_CTRLR_DOMAIN(ikey, ctrlr_dom);
  DbSubOp dbop_upd = {kOpNotRead, kOpMatchNone, kOpInOutCtrlr|kOpInOutDomain};

  // If vtunnel name is not exists, the vtunnel name will be auto-generated
  if (op == UNC_OP_CREATE) {
    if (!ctrlr_dom.ctrlr || !ctrlr_dom.domain) {
      UPLL_LOG_DEBUG("Ctrlr/Domain is NULL");
      return UPLL_RC_ERR_BAD_REQUEST;
    }
    if (!strlen(reinterpret_cast<char*>(con_key_tun->convert_vtunnel_name))) {
      string vtunnelName;
      // Generate vtunnel based on timestamp and if the name is not unique
      // Regenerate until getting an unique name
     DbSubOp dbop = {kOpReadExist, kOpMatchNone, kOpInOutNone};
     while (1) {
        vtunnelName = unc::upll::upll_util::getTime();
        string input = vtunnelName;
        std::stringstream ssoutput;
        for (int i = 0; i < 7; i++)
          ssoutput << input[(strlen(input.c_str())) - (7 - i)];
        // Assign auto generated even numbers for convert vtunnel
        int autogenerateno = atoi((ssoutput.str()).c_str());
        if (autogenerateno % 2 == 0) {
          vtunnelName = string(reinterpret_cast<char *>(unified_vbr_name)) +
            "_" + ssoutput.str();
          UPLL_LOG_DEBUG("Auto generated convert vtunnel name is %s",
              vtunnelName.c_str());
        } else {
          continue;
        }
        // Assign the generated vtunnel name
        uuu::upll_strncpy(con_key_tun->convert_vtunnel_name,
            vtunnelName.c_str(), (kMaxLenConvertVnodeName+1));
        result_code = UpdateConfigDB(ikey, UPLL_DT_CANDIDATE, UNC_OP_READ, dmi,
            &dbop, CONVERTTBL);
        if ((result_code != UPLL_RC_ERR_INSTANCE_EXISTS) &&
            (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE)) {
            UPLL_LOG_TRACE("UpdateConfigDB Failed -%d", result_code);
          return result_code;
        } else if (result_code == UPLL_RC_ERR_INSTANCE_EXISTS) {
          UPLL_LOG_TRACE("Convert vtunnel name already exists %d", result_code);
          continue;
        }
        break;
      }
      // Increment the ref count.
      con_val_tun->valid[UPLL_IDX_REF_COUNT_CONV_VTNL] = UNC_VF_VALID;
      con_val_tun->ref_count++;
    }
  }
  // Insert/update/delete the entry to database
  result_code = UpdateConfigDB(ikey, UPLL_DT_CANDIDATE, op, dmi, &dbop_upd,
      config_mode, vtn_name, CONVERTTBL);
  return result_code;
}
upll_rc_t VtunnelMoMgr::GetChildConvertConfigKey(ConfigKeyVal *&okey,
                                        ConfigKeyVal *parent_key) {
  UPLL_FUNC_TRACE;

  upll_rc_t result_code = UPLL_RC_SUCCESS;
  key_convert_vtunnel *vtunnel_convert_key = NULL;
  if (okey && (okey->get_key())) {
    vtunnel_convert_key = reinterpret_cast<key_convert_vtunnel_t *>
                (okey->get_key());
  } else {
    vtunnel_convert_key = reinterpret_cast<key_convert_vtunnel *>
      (ConfigKeyVal::Malloc(sizeof(key_convert_vtunnel)));
  }
  void *pkey;
  if (parent_key == NULL) {
    if (!okey)
      okey = new ConfigKeyVal(UNC_KT_VTUNNEL, IpctSt::kIpcStKeyConvertVtunnel,
                              vtunnel_convert_key, NULL);
    else if (okey->get_key() != vtunnel_convert_key)
      okey->SetKey(IpctSt::kIpcStKeyConvertVtunnel, vtunnel_convert_key);
    return UPLL_RC_SUCCESS;
  } else {
    pkey = parent_key->get_key();
  }
  if (!pkey) {
    if (!okey || !(okey->get_key()))
      free(vtunnel_convert_key);
    return UPLL_RC_ERR_GENERIC;
  }

  /* presumes MoMgrs receive only supported keytypes */
  switch (parent_key->get_key_type()) {
    case UNC_KT_VBR_PORTMAP:
      uuu::upll_strncpy(vtunnel_convert_key->vtn_key.vtn_name,
         reinterpret_cast<key_vbr_portmap*>(pkey)->vbr_key.vtn_key.vtn_name,
        (kMaxLenVtnName+1));
     break;
    case UNC_KT_VTUNNEL:
      uuu::upll_strncpy(vtunnel_convert_key->vtn_key.vtn_name,
         reinterpret_cast<key_convert_vtunnel*>(pkey)->vtn_key.vtn_name,
        (kMaxLenVtnName+1));
      uuu::upll_strncpy(vtunnel_convert_key->convert_vtunnel_name,
         reinterpret_cast<key_convert_vtunnel*>(pkey)->convert_vtunnel_name,
        (kMaxLenConvertVnodeName+1));
     break;
    default:
      break;
  }
  if (!okey)
    okey = new ConfigKeyVal(UNC_KT_VTUNNEL, IpctSt::kIpcStKeyConvertVtunnel,
                            vtunnel_convert_key, NULL);
  else if (okey->get_key() != vtunnel_convert_key)
    okey->SetKey(IpctSt::kIpcStKeyConvertVtunnel, vtunnel_convert_key);
  if (okey == NULL) {
    free(vtunnel_convert_key);
    result_code = UPLL_RC_ERR_GENERIC;
  } else {
    SET_USER_DATA(okey, parent_key);
  }
  return result_code;
}


}  // namespace kt_momgr
}  // namespace upll
}  // namespace unc
