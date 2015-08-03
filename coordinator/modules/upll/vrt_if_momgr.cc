/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include "vrt_if_momgr.hh"
#include "vlink_momgr.hh"

#define NUM_KEY_MAIN_TBL_ 6

namespace unc {
namespace upll {
namespace kt_momgr {

BindInfo VrtIfMoMgr::vrt_if_bind_info[] = {
    { uudst::vrouter_interface::kDbiVtnName, CFG_KEY, offsetof(
        key_vrt_if, vrt_key.vtn_key.vtn_name),
      uud::kDalChar, 32 },
    { uudst::vrouter_interface::kDbiVrouterName, CFG_KEY, offsetof(
        key_vrt_if, vrt_key.vrouter_name),
      uud::kDalChar, 32 },
    { uudst::vrouter_interface::kDbiIfName, CFG_KEY, offsetof(key_vrt_if,
                                                              if_name),
      uud::kDalChar, 32 },
    { uudst::vrouter_interface::kDbiDesc, CFG_VAL, offsetof(val_vrt_if,
                                                            description),
      uud::kDalChar, 128 },
    { uudst::vrouter_interface::kDbiIpAddr, CFG_VAL, offsetof(val_vrt_if,
                                                              ip_addr),
      uud::kDalUint32, 1 },
    { uudst::vrouter_interface::kDbiMask, CFG_VAL, offsetof(val_vrt_if,
                                                            prefixlen),
      uud::kDalUint8, 1 },
    { uudst::vrouter_interface::kDbiMacAddr, CFG_VAL, offsetof(val_vrt_if,
                                                               macaddr),
      uud::kDalUint8, 6 },
    { uudst::vrouter_interface::kDbiAdminStatus, CFG_VAL, offsetof(
        val_vrt_if, admin_status),
      uud::kDalUint8, 1 },
    { uudst::vrouter_interface::kDbiOperStatus, ST_VAL, offsetof(
        val_db_vrt_if_st, vrt_if_val_st.oper_status),
      uud::kDalUint8, 1 },
    { uudst::vrouter_interface::kDbiDownCount, ST_VAL, offsetof(
        val_db_vrt_if_st, down_count),
      uud::kDalUint32, 1 },
    { uudst::vrouter_interface::kDbiCtrlrName, CK_VAL, offsetof(key_user_data,
                                                                ctrlr_id),
      uud::kDalChar, 32 },
    { uudst::vrouter_interface::kDbiDomainId, CK_VAL, offsetof(key_user_data,
                                                               domain_id),
      uud::kDalChar, 32 },
    { uudst::vrouter_interface::kDbiVrtIfFlags, CK_VAL, offsetof(key_user_data,
                                                                 flags),
      uud::kDalUint8, 1 },
    { uudst::vrouter_interface::kDbiValidDesc, CFG_META_VAL, offsetof(
        val_vrt_if, valid[UPLL_IDX_DESC_VI]),
      uud::kDalUint8, 1 },
    { uudst::vrouter_interface::kDbiValidIpAddr, CFG_META_VAL, offsetof(
        val_vrt_if, valid[UPLL_IDX_IP_ADDR_VI]),
      uud::kDalUint8, 1 },
    { uudst::vrouter_interface::kDbiValidMask, CFG_META_VAL, offsetof(
        val_vrt_if, valid[UPLL_IDX_PREFIXLEN_VI]),
      uud::kDalUint8, 1 },
    { uudst::vrouter_interface::kDbiValidMacAddr, CFG_META_VAL, offsetof(
        val_vrt_if, valid[UPLL_IDX_MAC_ADDR_VI]),
      uud::kDalUint8, 1 },
    { uudst::vrouter_interface::kDbiValidAdminStatus, CFG_DEF_VAL, offsetof(
        val_vrt_if, valid[UPLL_IDX_ADMIN_ST_VI]),
      uud::kDalUint8, 1 },
    { uudst::vrouter_interface::kDbiValidOperStatus, ST_META_VAL, offsetof(
        val_vrt_if_st, valid[UPLL_IDX_OPER_STATUS_VRTIS]),
      uud::kDalUint8, 1 },
    { uudst::vrouter_interface::kDbiCsRowstatus, CS_VAL, offsetof(
        val_vrt_if, cs_row_status),
      uud::kDalUint8, 1 },
    { uudst::vrouter_interface::kDbiCsDesc, CS_VAL, offsetof(
        val_vrt_if, cs_attr[UPLL_IDX_DESC_VI]),
      uud::kDalUint8, 1 },
    { uudst::vrouter_interface::kDbiCsIpAddr, CS_VAL, offsetof(
        val_vrt_if, cs_attr[UPLL_IDX_IP_ADDR_VI]),
      uud::kDalUint8, 1 },
    { uudst::vrouter_interface::kDbiCsMask, CS_VAL, offsetof(
        val_vrt_if, cs_attr[UPLL_IDX_PREFIXLEN_VI]),
      uud::kDalUint8, 1 },
    { uudst::vrouter_interface::kDbiCsMacAddr, CS_VAL, offsetof(
        val_vrt_if, cs_attr[UPLL_IDX_MAC_ADDR_VI]),
      uud::kDalUint8, 1 },
    { uudst::vrouter_interface::kDbiCsAdminStatus, CS_VAL, offsetof(
        val_vrt_if, cs_attr[UPLL_IDX_ADMIN_ST_VI]),
      uud::kDalUint8, 1 } };

BindInfo VrtIfMoMgr::key_vrt_if_maintbl_update_bind_info[] = {
    { uudst::vrouter_interface::kDbiVtnName, CFG_MATCH_KEY, offsetof(
        key_vrt_if, vrt_key.vtn_key.vtn_name),
      uud::kDalChar, kMaxLenVtnName + 1 },
    { uudst::vrouter_interface::kDbiVrouterName, CFG_MATCH_KEY, offsetof(
        key_vrt_if, vrt_key.vrouter_name),
      uud::kDalChar, kMaxLenVnodeName + 1 },
    { uudst::vrouter_interface::kDbiIfName, CFG_MATCH_KEY, offsetof(
        key_vrt_if, if_name),
      uud::kDalChar, kMaxLenInterfaceName + 1 },
    { uudst::vrouter_interface::kDbiVtnName, CFG_INPUT_KEY, offsetof(
        key_rename_vnode_info_t, new_unc_vtn_name),
      uud::kDalChar, kMaxLenVtnName + 1 },
    { uudst::vrouter_interface::kDbiVrouterName, CFG_INPUT_KEY, offsetof(
        key_rename_vnode_info_t, new_unc_vnode_name),
      uud::kDalChar, kMaxLenVnodeName + 1 },
    { uudst::vrouter_interface::kDbiVrtIfFlags, CK_VAL, offsetof(
        key_user_data, flags),
      uud::kDalUint8, 1 }
};
unc_key_type_t VrtIfMoMgr::vrt_if_child[] = { UNC_KT_VRTIF_FLOWFILTER };

VrtIfMoMgr::VrtIfMoMgr() {
  UPLL_FUNC_TRACE;
  ntable = MAX_MOMGR_TBLS;
  table = new Table *[ntable]();
  table[MAINTBL] = new Table(uudst::kDbiVrtIfTbl, UNC_KT_VRT_IF,
      vrt_if_bind_info, IpctSt::kIpcStKeyVrtIf, IpctSt::kIpcStValVrtIf,
      uudst::vrouter_interface::kDbiVrtIfNumCols);
  table[RENAMETBL] = NULL;
  table[CTRLRTBL] = NULL;
  table[CONVERTTBL] = NULL;

  nchild = sizeof(vrt_if_child) / sizeof(*vrt_if_child);
  child = vrt_if_child;
}

bool VrtIfMoMgr:: GetRenameKeyBindInfo(unc_key_type_t key_type,
                                       BindInfo *&binfo, int &nattr,
                                       MoMgrTables tbl ) {
  /* Main Table or rename table only update */
  if (MAINTBL == tbl) {
    nattr = NUM_KEY_MAIN_TBL_;
    binfo = key_vrt_if_maintbl_update_bind_info;
  } else {
    UPLL_LOG_DEBUG("Invalid Table");
    return PFC_FALSE;
  }
  return PFC_TRUE;
}

upll_rc_t VrtIfMoMgr::ValidateAttribute(ConfigKeyVal *ikey,
                                        DalDmlIntf *dmi,
                                        IpcReqRespHeader *req) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  if (req->operation != UNC_OP_CREATE && req->operation != UNC_OP_UPDATE) {
    return UPLL_RC_SUCCESS;
  }

  result_code = ValidateIpAddress(ikey, req->datatype, dmi);
#if 0
  if (ikey->get_key_type() != UNC_KT_VRT_IF) result_code =
      UPLL_RC_ERR_CFG_SYNTAX;
#endif
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("ValidateIpAddress failed");
    return result_code;
  }
  return result_code;
}

bool VrtIfMoMgr::IsValidKey(void *key, uint64_t index,
                            MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  key_vrt_if *vrtif_key = reinterpret_cast<key_vrt_if *>(key);
  upll_rc_t ret_val = UPLL_RC_SUCCESS;
  switch (index) {
    case uudst::vrouter_interface::kDbiVtnName:
      ret_val = ValidateKey(reinterpret_cast<char *>
                    (vrtif_key->vrt_key.vtn_key.vtn_name),
                     kMinLenVtnName, kMaxLenVtnName);
      if (ret_val != UPLL_RC_SUCCESS) {
        UPLL_LOG_TRACE("VTN Name is not valid(%d)", ret_val);
        return false;
      }
      break;
    case uudst::vrouter_interface::kDbiVrouterName:
      ret_val = ValidateKey(reinterpret_cast<char *>
                           (vrtif_key->vrt_key.vrouter_name),
                            kMinLenVnodeName, kMaxLenVnodeName);
      if (ret_val != UPLL_RC_SUCCESS) {
        UPLL_LOG_TRACE("VRouter Name is not valid(%d)", ret_val);
        return false;
      }
      break;
    case uudst::vrouter_interface::kDbiIfName:
      ret_val = ValidateKey(reinterpret_cast<char *>
                        (vrtif_key->if_name), kMinLenInterfaceName,
                         kMaxLenInterfaceName);
      if (ret_val != UPLL_RC_SUCCESS) {
        UPLL_LOG_TRACE("VRouterInterface Name is not valid(%d)", ret_val);
        return false;
      }
      break;
    default:
      UPLL_LOG_INFO("Invalid Key Index");
      break;
  }
  return true;
}

upll_rc_t VrtIfMoMgr::GetChildConfigKey(ConfigKeyVal *&okey,
                                        ConfigKeyVal *parent_key) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  key_vrt_if *vrt_key_if;
  void *pkey;
  if (parent_key == NULL) {
    vrt_key_if = reinterpret_cast<key_vrt_if *>
        (ConfigKeyVal::Malloc(sizeof(key_vrt_if)));
    if (okey) delete okey;
    okey = new ConfigKeyVal(UNC_KT_VRT_IF, IpctSt::kIpcStKeyVrtIf, vrt_key_if,
                            NULL);
    return UPLL_RC_SUCCESS;
  } else {
     pkey = parent_key->get_key();
  }
  if (!pkey) return UPLL_RC_ERR_GENERIC;
  if (okey) {
    if (okey->get_key_type() != UNC_KT_VRT_IF)
       return UPLL_RC_ERR_GENERIC;
    vrt_key_if = reinterpret_cast<key_vrt_if *>(okey->get_key());
    if (!vrt_key_if) return UPLL_RC_ERR_GENERIC;
  } else {
    vrt_key_if = reinterpret_cast<key_vrt_if *>
        (ConfigKeyVal::Malloc(sizeof(key_vrt_if)));
  }
  unc_key_type_t keytype = parent_key->get_key_type();
  switch (keytype) {
    case UNC_KT_VTN:
      uuu::upll_strncpy(vrt_key_if->vrt_key.vtn_key.vtn_name,
           reinterpret_cast<key_vtn *>(pkey)->vtn_name,
            (kMaxLenVtnName+1));
      *(vrt_key_if->vrt_key.vrouter_name) = *"";
      break;
    case UNC_KT_VROUTER:
      uuu::upll_strncpy(vrt_key_if->vrt_key.vtn_key.vtn_name,
          reinterpret_cast<key_vrt *>(pkey)->vtn_key.vtn_name,
        (kMaxLenVtnName+1));
      uuu::upll_strncpy(vrt_key_if->vrt_key.vrouter_name,
             reinterpret_cast<key_vrt *>(pkey)->vrouter_name,
             (kMaxLenVnodeName+1));
      break;
    case UNC_KT_VRT_IF:
      uuu::upll_strncpy(vrt_key_if->vrt_key.vtn_key.vtn_name,
             reinterpret_cast<key_vrt_if *>(pkey)->vrt_key.vtn_key.vtn_name,
              (kMaxLenVtnName+1));
      uuu::upll_strncpy(vrt_key_if->vrt_key.vrouter_name,
             reinterpret_cast<key_vrt_if *>(pkey)->vrt_key.vrouter_name,
              (kMaxLenVnodeName+1));
      uuu::upll_strncpy(vrt_key_if->if_name,
             reinterpret_cast<key_vrt_if *>(pkey)->if_name,
              (kMaxLenInterfaceName+1));
      break;
    case UNC_KT_DHCPRELAY_IF:
      uuu::upll_strncpy(vrt_key_if->vrt_key.vtn_key.vtn_name,
             reinterpret_cast<key_dhcp_relay_if *>
             (pkey)->vrt_key.vtn_key.vtn_name,
             (kMaxLenVtnName+1));
      uuu::upll_strncpy(vrt_key_if->vrt_key.vrouter_name,
             reinterpret_cast<key_dhcp_relay_if *>(pkey)->vrt_key.vrouter_name,
              (kMaxLenVnodeName+1));
      uuu::upll_strncpy(vrt_key_if->if_name,
             reinterpret_cast<key_dhcp_relay_if *>(pkey)->if_name,
              (kMaxLenInterfaceName+1));
      break;

      case UNC_KT_VLINK: {
        uint8_t *vnode_name, *if_name;
        uint8_t flags = 0;
        val_vlink *vlink_val = reinterpret_cast<val_vlink *>
                    (GetVal(parent_key));
        if (!vlink_val) {
          if (!okey || !(okey->get_key()))
            free(vrt_key_if);
          return UPLL_RC_ERR_GENERIC;
        }
        GET_USER_DATA_FLAGS(parent_key->get_cfg_val(), flags);
        flags &= VLINK_FLAG_NODE_POS;
        if (flags == kVlinkVnode2) {
          vnode_name = vlink_val->vnode2_name;
          if_name = vlink_val->vnode2_ifname;
        } else {
          vnode_name = vlink_val->vnode1_name;
          if_name = vlink_val->vnode1_ifname;
        }
        uuu::upll_strncpy(vrt_key_if->vrt_key.vtn_key.vtn_name,
                        reinterpret_cast<key_vlink *>(pkey)->vtn_key.vtn_name,
                        (kMaxLenVtnName + 1));
        if (vnode_name)
          uuu::upll_strncpy(vrt_key_if->vrt_key.vrouter_name, vnode_name,
                          (kMaxLenVnodeName + 1));
        if (if_name)
          uuu::upll_strncpy(vrt_key_if->if_name, if_name,
                          (kMaxLenInterfaceName + 1));
    }
    break;
    case UNC_KT_VBRIDGE:
      uuu::upll_strncpy(vrt_key_if->vrt_key.vtn_key.vtn_name,
             reinterpret_cast<key_vbr *>(pkey)->vtn_key.vtn_name,
              (kMaxLenVtnName+1));
      break;
    break;
    default:
      break;
  }
  if (!okey)
    okey = new ConfigKeyVal(UNC_KT_VRT_IF, IpctSt::kIpcStKeyVrtIf, vrt_key_if,
                            NULL);
  else if (okey->get_key() != vrt_key_if)
    okey->SetKey(IpctSt::kIpcStKeyVrtIf, vrt_key_if);

  if (okey == NULL) {
    free(vrt_key_if);
    result_code = UPLL_RC_ERR_GENERIC;
  } else {
     SET_USER_DATA(okey, parent_key);
  }
  return result_code;
}

upll_rc_t VrtIfMoMgr::GetParentConfigKey(ConfigKeyVal *&okey,
                                         ConfigKeyVal *ikey) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  if (!ikey) {
    UPLL_LOG_DEBUG("ikey is NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  DELETE_IF_NOT_NULL(okey);
  unc_key_type_t ikey_type = ikey->get_key_type();

  if (ikey_type != UNC_KT_VRT_IF) return UPLL_RC_ERR_GENERIC;
  key_vrt_if *pkey = reinterpret_cast<key_vrt_if *>(ikey->get_key());
  if (!pkey) return UPLL_RC_ERR_GENERIC;
  key_vrt *vrt_key = reinterpret_cast<key_vrt *>
      (ConfigKeyVal::Malloc(sizeof(key_vrt)));
  uuu::upll_strncpy(vrt_key->vtn_key.vtn_name,
         reinterpret_cast<key_vrt_if *>(pkey)->vrt_key.vtn_key.vtn_name,
        (kMaxLenVtnName+1));
  uuu::upll_strncpy(vrt_key->vrouter_name,
         reinterpret_cast<key_vrt_if *>(pkey)->vrt_key.vrouter_name,
        (kMaxLenVnodeName+1));
  okey = new ConfigKeyVal(UNC_KT_VROUTER, IpctSt::kIpcStKeyVrt, vrt_key, NULL);
  if (okey == NULL) {
    free(vrt_key);
    result_code = UPLL_RC_ERR_GENERIC;
  } else {
    SET_USER_DATA(okey, ikey);
  }
  return result_code;
}

upll_rc_t VrtIfMoMgr::AllocVal(ConfigVal *&ck_val,
                               upll_keytype_datatype_t dt_type,
                               MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  void *val;  // , *nxt_val;
//   ConfigVal *ck_nxtval;

  if (ck_val != NULL) return UPLL_RC_ERR_GENERIC;
  switch (tbl) {
    case MAINTBL:
      val = reinterpret_cast<void *>(ConfigKeyVal::Malloc(sizeof(val_vrt_if)));
      ck_val = new ConfigVal(IpctSt::kIpcStValVrtIf, val);
      if (dt_type == UPLL_DT_STATE) {
        val = reinterpret_cast<void *>
            (ConfigKeyVal::Malloc(sizeof(val_db_vrt_if_st)));
        ConfigVal *ck_nxtval = new ConfigVal(IpctSt::kIpcStValVrtIfSt, val);
        ck_val->AppendCfgVal(ck_nxtval);
      }
      break;
    default:
      val = NULL;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VrtIfMoMgr::DupConfigKeyVal(ConfigKeyVal *&okey, ConfigKeyVal *&req,
                                      MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  if (!req || okey) {
    UPLL_LOG_TRACE("Invalid param");
    return UPLL_RC_ERR_GENERIC;
  }
  if (req->get_key_type() != UNC_KT_VRT_IF) return UPLL_RC_ERR_GENERIC;
  ConfigVal *tmp1 = NULL, *tmp = (req)->get_cfg_val();
  if (tmp) {
    if (tbl == MAINTBL) {
      val_vrt_if *ival = reinterpret_cast<val_vrt_if *>(GetVal(req));
      val_vrt_if *vrt_val_if = reinterpret_cast<val_vrt_if *>
          (ConfigKeyVal::Malloc(sizeof(val_vrt_if)));
      memcpy(vrt_val_if, ival, sizeof(val_vrt_if));
      tmp1 = new ConfigVal(IpctSt::kIpcStValVrtIf, vrt_val_if);
    }
    tmp = tmp->get_next_cfg_val();
  };
  if (tmp) {
    if (tbl == MAINTBL) {
      val_db_vrt_if_st *ival =
          reinterpret_cast<val_db_vrt_if_st *>(tmp->get_val());
      val_db_vrt_if_st *val_vrt_if = reinterpret_cast<val_db_vrt_if_st *>
          (ConfigKeyVal::Malloc(sizeof(val_db_vrt_if_st)));
      memcpy(val_vrt_if, ival, sizeof(val_db_vrt_if_st));
      ConfigVal *tmp2 = new ConfigVal(IpctSt::kIpcStValVrtIfSt, val_vrt_if);
      if (tmp1)
        tmp1->AppendCfgVal(tmp2);
    }
  };
  void *tkey = (req != NULL) ? (req)->get_key() : NULL;
  key_vrt_if *ikey = reinterpret_cast<key_vrt_if *>(tkey);
  key_vrt_if *vrt_if_key = reinterpret_cast<key_vrt_if *>
      (ConfigKeyVal::Malloc(sizeof(key_vrt_if)));
  memcpy(vrt_if_key, ikey, sizeof(key_vrt_if));
  okey = new ConfigKeyVal(UNC_KT_VRT_IF, IpctSt::kIpcStKeyVrtIf, vrt_if_key,
                          tmp1);
  if (okey) {
    SET_USER_DATA(okey, req);
  } else {
    DELETE_IF_NOT_NULL(tmp1);
    FREE_IF_NOT_NULL(vrt_if_key);
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VrtIfMoMgr::AdaptValToVtnService(ConfigKeyVal *ikey,
                                           AdaptType adapt_type) {
  UPLL_FUNC_TRACE;
  if (!ikey) {
    UPLL_LOG_DEBUG("Invalid ikey");
    return UPLL_RC_ERR_GENERIC;
  }
  key_vrt_if *vrtif_key = reinterpret_cast<key_vrt_if*>(ikey->get_key());
  while (ikey) {
    ConfigVal *cval = ikey->get_cfg_val();
    if (!cval) {
      UPLL_LOG_DEBUG("Config Val is Null");
      return UPLL_RC_ERR_GENERIC;
    }
    while (cval) {
      if (IpctSt::kIpcStValVrtIf == cval->get_st_num()) {
         // set admin status to valid no value
         val_vrt_if *vrt_if_val = reinterpret_cast<val_vrt_if *>GetVal(ikey);
         if (vrt_if_val->valid[UPLL_IDX_ADMIN_ST_VI] == UNC_VF_INVALID)
           vrt_if_val->valid[UPLL_IDX_ADMIN_ST_VI] = UNC_VF_VALID_NO_VALUE;
      }
      if (IpctSt::kIpcStValVrtIfSt == cval->get_st_num()) {
        controller_domain ctrlr_dom = {NULL, NULL};
        GET_USER_DATA_CTRLR_DOMAIN(ikey, ctrlr_dom);
        CheckOperStatus<val_vrt_if_st>(vrtif_key->vrt_key.vtn_key.vtn_name,
                                       cval, UNC_KT_VRT_IF, ctrlr_dom);
      }
      cval = cval->get_next_cfg_val();
    }
    if (adapt_type == ADAPT_ONE)
      break;
    ikey = ikey->get_next_cfg_key_val();
  }
  return UPLL_RC_SUCCESS;
}
upll_rc_t VrtIfMoMgr::UpdateConfigStatus(ConfigKeyVal *ikey,
                                         unc_keytype_operation_t op,
                                         uint32_t driver_result,
                                         ConfigKeyVal *upd_key,
                                         DalDmlIntf *dmi,
                                         ConfigKeyVal *ctrlr_key) {
  UPLL_FUNC_TRACE;
  val_vrt_if_t *vrt_if_val = reinterpret_cast<val_vrt_if_t *>(GetVal(ikey));

  unc_keytype_configstatus_t cs_status =
      (driver_result == UPLL_RC_SUCCESS) ? UNC_CS_APPLIED : UNC_CS_NOT_APPLIED;
  bool oper_status_change = true;
  bool propagate = true;

  if (vrt_if_val == NULL) return UPLL_RC_ERR_GENERIC;
  switch (op) {
  case UNC_OP_UPDATE:
  {
    void *val = reinterpret_cast<void *>(vrt_if_val);
    CompareValidValue(val, GetVal(upd_key), true);
    uint8_t cand_flag = 0, run_flag = 0;
    GET_USER_DATA_FLAGS(ikey, cand_flag);
    GET_USER_DATA_FLAGS(upd_key, run_flag);
    if ((cand_flag & VIF_TYPE) == (run_flag & VIF_TYPE))
      oper_status_change = false;
  }
  /* fall through intended */
  case UNC_OP_CREATE:
  {
    propagate = false;
    if (op == UNC_OP_CREATE)
      vrt_if_val->cs_row_status = cs_status;
    break;
  }
  default:
    return UPLL_RC_ERR_GENERIC;
  }
  if (oper_status_change) {
    val_db_vrt_if_st *vrt_if_valst = reinterpret_cast<val_db_vrt_if_st *>
        (ConfigKeyVal::Malloc(sizeof(val_db_vrt_if_st)));
    ikey->AppendCfgVal(IpctSt::kIpcStValVrtIfSt, vrt_if_valst);
    val_db_vrt_if_st *vnif_st = reinterpret_cast<val_db_vrt_if_st  *>
             (GetStateVal(ikey));
    vnif_st->vrt_if_val_st.valid[UPLL_IDX_OPER_STATUS_VRTS] = UNC_VF_VALID;
    if (op == UNC_OP_CREATE) {
       if (driver_result == UPLL_RC_ERR_CTR_DISCONNECTED) {
          vnif_st->vrt_if_val_st.oper_status = UPLL_OPER_STATUS_UNKNOWN;
          vnif_st->down_count = PORT_UNKNOWN;
        } else {
          vnif_st->vrt_if_val_st.oper_status = UPLL_OPER_STATUS_UNINIT;
          vnif_st->down_count = 0;
        }
      } else {
        val_db_vrt_if_st *run_vrtifst = reinterpret_cast<val_db_vrt_if_st *>
                                 (GetStateVal(upd_key));
        vnif_st->vrt_if_val_st.oper_status =
            run_vrtifst->vrt_if_val_st.oper_status;
        vnif_st->down_count = run_vrtifst->down_count;
      }
  }
  UPLL_LOG_TRACE("%s", (ikey->ToStrAll()).c_str());
  val_vrt_if *vrt_if_val2 = reinterpret_cast<val_vrt_if *>(GetVal(upd_key));
  if (UNC_OP_UPDATE == op) {
    UPLL_LOG_TRACE("%s", (upd_key->ToStrAll()).c_str());
    vrt_if_val->cs_row_status = vrt_if_val2->cs_row_status;
  }
  for (unsigned int loop = 0; loop <
       sizeof(vrt_if_val->valid) / sizeof(vrt_if_val->valid[0]); ++loop) {
    if ((UNC_VF_VALID == (uint8_t) vrt_if_val->valid[loop])
        || (UNC_VF_VALID_NO_VALUE == (uint8_t) vrt_if_val->valid[loop])) {
      if (loop == UPLL_IDX_DESC_VI)
        vrt_if_val->cs_attr[loop] = UNC_CS_APPLIED;
      else
        vrt_if_val->cs_attr[loop] = cs_status;
    } else if ((UNC_VF_INVALID == vrt_if_val->valid[loop]) &&
               (UNC_OP_CREATE == op)) {
       vrt_if_val->cs_attr[loop] = UNC_CS_APPLIED;
    } else if ((UNC_VF_INVALID == vrt_if_val->valid[loop]) &&
               (UNC_OP_UPDATE == op)) {
       vrt_if_val->cs_attr[loop] = vrt_if_val2->cs_attr[loop];
    }
  }
  return (SetInterfaceOperStatus(ikey, dmi, op, propagate, driver_result));
}

upll_rc_t VrtIfMoMgr::UpdateAuditConfigStatus(
    unc_keytype_configstatus_t cs_status,
    uuc::UpdateCtrlrPhase phase,
    ConfigKeyVal *&ckv_running,
    DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  val_vrt_if_t *val;
  val = (ckv_running != NULL) ? reinterpret_cast<val_vrt_if_t *>
                                 (GetVal(ckv_running)) : NULL;
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
         cs_status == UNC_CS_APPLIED)
         val->cs_attr[loop] = cs_status;
  }
  return result_code;
}

uint8_t* VrtIfMoMgr::GetControllerId(ConfigKeyVal *ck_vrt,
                                     upll_keytype_datatype_t dt_type,
                                     DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  if (ck_vrt->get_key_type() != UNC_KT_VROUTER) return NULL;
  DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutNone };
  MoMgrImpl *mgr = reinterpret_cast<MoMgrImpl *>(const_cast<MoManager*>
                                        (GetMoManager(UNC_KT_VROUTER)));
  upll_rc_t result_code = mgr->ReadConfigDB(ck_vrt, dt_type, UNC_OP_READ, dbop,
                                            dmi, MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    return NULL;
  }
  // val_vrt_t *vrt_val = (val_vrt_t *)(GetVal(ck_vrt));
#ifdef UNCOMMENT
  if (vrt_val)
  return (vrt_val->controller_id);
  else
  return NULL;
#endif
  return reinterpret_cast<uint8_t *>(const_cast<char*>("pfc"));
}

upll_rc_t VrtIfMoMgr::ValidateMessage(IpcReqRespHeader *req,
                                      ConfigKeyVal *ikey) {
  UPLL_FUNC_TRACE;
  /** read datatype, operation, options from IpcReqRespHeader */
  if (!req || !ikey || !(ikey->get_key())) {
    UPLL_LOG_DEBUG("ConfigKeyVal / IpcReqRespHeader is NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  upll_keytype_datatype_t dt_type = req->datatype;
  unc_keytype_operation_t op = req->operation;
  unc_keytype_option1_t opt1 = req->option1;
  unc_keytype_option2_t opt2 = req->option2;

  upll_rc_t ret_val = UPLL_RC_SUCCESS;
  if (ikey->get_st_num() != IpctSt::kIpcStKeyVrtIf) {
    UPLL_LOG_DEBUG("Invalid key structure received. received struct - %d",
                  (ikey->get_st_num()));
    return UPLL_RC_ERR_BAD_REQUEST;
  }
  key_vrt_if *vrt_if_key = reinterpret_cast<key_vrt_if *>(ikey->get_key());
  unc_key_type_t ktype = ikey->get_key_type();
  if (UNC_KT_VRT_IF != ktype) {
    UPLL_LOG_DEBUG("Invalid Keytype received. reived keytype - %d", ktype);
    return UPLL_RC_ERR_BAD_REQUEST;
  }
  ret_val = ValidateVrtIfKey(vrt_if_key, op);
  if (ret_val != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Error: Key struct Validation failure");
    return UPLL_RC_ERR_CFG_SYNTAX;
  }
  val_vrt_if *vrt_if_val = NULL;
  val_vtn_neighbor *vtn_neighbor = NULL;
  if ((ikey->get_cfg_val())
      && ((ikey->get_cfg_val())->get_st_num() == IpctSt::kIpcStValVrtIf)) {
     vrt_if_val =  reinterpret_cast<val_vrt_if *>
                                  (ikey->get_cfg_val()->get_val());
  } else if ((ikey->get_cfg_val()) &&
       ((ikey->get_cfg_val())->get_st_num() == IpctSt::kIpcStValVtnNeighbor)) {
    vtn_neighbor =  reinterpret_cast<val_vtn_neighbor *>
                                          (ikey->get_cfg_val()->get_val());
  } else if ((ikey->get_cfg_val()) &&
       (((ikey->get_cfg_val())->get_st_num() != IpctSt::kIpcStValVrtIf) ||
       ((ikey->get_cfg_val())->get_st_num() != IpctSt::kIpcStValVtnNeighbor))) {
    UPLL_LOG_DEBUG("Invalid val structure received.received struct - %d",
                    ikey->get_cfg_val()->get_st_num());
    return UPLL_RC_ERR_BAD_REQUEST;
  }

  if ((op == UNC_OP_CREATE) && ((dt_type == UPLL_DT_CANDIDATE)
       || UPLL_DT_IMPORT == dt_type)) {
      if (vrt_if_val == NULL) {
        UPLL_LOG_DEBUG(
        "Val struct Validation is an optional for CREATE operation");
        return UPLL_RC_SUCCESS;
      }
      ret_val = ValidateVrtIfValue(vrt_if_val, op);
      if (ret_val != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Val struct Validation failure for CREATE operation");
        return UPLL_RC_ERR_CFG_SYNTAX;
      }
      return UPLL_RC_SUCCESS;
  } else if ((op == UNC_OP_UPDATE) && (dt_type == UPLL_DT_CANDIDATE)) {
      if (vrt_if_val == NULL) {
        UPLL_LOG_DEBUG("Val struct Validation is mandatory for UPDATE op");
        return UPLL_RC_ERR_BAD_REQUEST;
      }
      ret_val = ValidateVrtIfValue(vrt_if_val, op);
      if (ret_val != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Val struct Validation failure for UPDATE operation");
        return UPLL_RC_ERR_CFG_SYNTAX;
      }
      return UPLL_RC_SUCCESS;
  } else if ((op == UNC_OP_READ || op == UNC_OP_READ_SIBLING ||
             op == UNC_OP_READ_SIBLING_BEGIN) &&
        (dt_type == UPLL_DT_CANDIDATE || dt_type == UPLL_DT_RUNNING
        || dt_type == UPLL_DT_STARTUP)) {
      if (opt1 != UNC_OPT1_NORMAL) {
        UPLL_LOG_DEBUG("Error option1 is not NORMAL");
        return UPLL_RC_ERR_INVALID_OPTION1;
      }
      if (opt2 != UNC_OPT2_NONE) {
        UPLL_LOG_DEBUG("Error option2 is not NONE");
        return UPLL_RC_ERR_INVALID_OPTION2;
      }
      if (vrt_if_val == NULL) {
        UPLL_LOG_DEBUG(
        "Val struct Validation is an optional for READ operation");
        return UPLL_RC_SUCCESS;
      }
      ret_val = ValidateVrtIfValue(vrt_if_val, op);
      if (ret_val != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("val struct Validation failure for READ operation");
        return UPLL_RC_ERR_CFG_SYNTAX;
      }
      return UPLL_RC_SUCCESS;
  } else if ((op == UNC_OP_READ) && (dt_type == UPLL_DT_STATE)) {
      if (opt1 != UNC_OPT1_NORMAL) {
        UPLL_LOG_DEBUG("Error option1 is not NORMAL");
        return UPLL_RC_ERR_INVALID_OPTION1;
      }
      if (opt2 == UNC_OPT2_NONE) {
        if (vrt_if_val == NULL) {
          UPLL_LOG_DEBUG(
          "Val struct Validation is an optional for READ operation");
          return UPLL_RC_SUCCESS;
        }
        ret_val = ValidateVrtIfValue(vrt_if_val, op);
        if (ret_val != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("val struct Validation failure for READ operation");
          return UPLL_RC_ERR_CFG_SYNTAX;
        }
        return ret_val;
      } else if (opt2 == UNC_OPT2_NEIGHBOR) {
        if (vtn_neighbor == NULL) {
          UPLL_LOG_DEBUG(
              "Val vtn_neighbor struct Validation is an optional"
               "for READ operation");
          return UPLL_RC_SUCCESS;
        }
        ret_val = ValidateVtnNeighborValue(vtn_neighbor, op);
        if (ret_val != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("Val struct Validation failure for"
                         "val_vtn_neighbor structure");
          return UPLL_RC_ERR_CFG_SYNTAX;
        }
        return ret_val;
      } else {
        UPLL_LOG_DEBUG("Error option2 is not matching");
        return UPLL_RC_ERR_INVALID_OPTION2;
      }
  } else if ((op == UNC_OP_READ_SIBLING_BEGIN ||
              op == UNC_OP_READ_SIBLING) && (dt_type == UPLL_DT_STATE)) {
      if (opt1 != UNC_OPT1_NORMAL) {
        UPLL_LOG_DEBUG("Error option1 is not NORMAL");
        return UPLL_RC_ERR_INVALID_OPTION1;
      }
      if (opt2 != UNC_OPT2_NONE) {
        UPLL_LOG_DEBUG("Error option2 is not NONE");
        return UPLL_RC_ERR_INVALID_OPTION2;
      }
      if (vrt_if_val == NULL) {
        UPLL_LOG_DEBUG("Val struct Validation is an optional for"
                        "READ_SIBLING_COUNT operation");
        return UPLL_RC_SUCCESS;
      }
      ret_val = ValidateVrtIfValue(vrt_if_val, op);
      if (ret_val != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG(
        "Val struct Validation failure for READ_SIBLING_COUNT operation");
        return UPLL_RC_ERR_CFG_SYNTAX;
      }
      return UPLL_RC_SUCCESS;
  } else if ((op == UNC_OP_READ_SIBLING_COUNT) && (dt_type == UPLL_DT_STATE ||
          dt_type == UPLL_DT_STARTUP || dt_type == UPLL_DT_CANDIDATE ||
          dt_type == UPLL_DT_RUNNING)) {
      if (opt1 != UNC_OPT1_NORMAL) {
        UPLL_LOG_DEBUG("Error option1 is not NORMAL");
        return UPLL_RC_ERR_INVALID_OPTION1;
      }
      if (opt2 != UNC_OPT2_NONE) {
        UPLL_LOG_DEBUG("Error option2 is not NONE");
        return UPLL_RC_ERR_INVALID_OPTION2;
      }
      if (vrt_if_val == NULL) {
        UPLL_LOG_DEBUG("Val struct Validation is an optional for"
                        "READ_SIBLING_COUNT operation");
        return UPLL_RC_SUCCESS;
      }
      ret_val = ValidateVrtIfValue(vrt_if_val, op);
      if (ret_val != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG(
        "Val struct Validation failure for READ_SIBLING_COUNT operation");
        return UPLL_RC_ERR_CFG_SYNTAX;
      }
      return UPLL_RC_SUCCESS;
  } else if ((op == UNC_OP_DELETE || op == UNC_OP_READ_NEXT ||
      op == UNC_OP_READ_BULK) && (dt_type == UPLL_DT_CANDIDATE ||
      dt_type == UPLL_DT_RUNNING || dt_type == UPLL_DT_STARTUP)) {
      UPLL_LOG_DEBUG(
        "Value structure validation is none for this operation."
        " operation - %d", op);
      return UPLL_RC_SUCCESS;
  }
  UPLL_LOG_DEBUG("Unsupported operation %d or datatype %d", op, dt_type);
  return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT;
}

upll_rc_t VrtIfMoMgr::ValidateVrtIfKey(key_vrt_if *vrt_if_key,
                                       unc_keytype_operation_t operation) {
  UPLL_FUNC_TRACE;
  upll_rc_t ret_val = UPLL_RC_SUCCESS;
  ret_val = ValidateKey(reinterpret_cast<char *>
                        (vrt_if_key->vrt_key.vtn_key.vtn_name),
                        kMinLenVtnName, kMaxLenVtnName);
  if (ret_val != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Vtn name syntax check failed."
                  "Received vtn_name - %s",
                  vrt_if_key->vrt_key.vtn_key.vtn_name);
    return UPLL_RC_ERR_CFG_SYNTAX;
  }
  ret_val = ValidateKey(reinterpret_cast<char *>
                        (vrt_if_key->vrt_key.vrouter_name),
                        kMinLenVnodeName, kMaxLenVnodeName);
  if (ret_val != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("vrouter_name syntax check failed. %s",
                        vrt_if_key->vrt_key.vrouter_name);
    return UPLL_RC_ERR_CFG_SYNTAX;
  }
  if (operation != UNC_OP_READ_SIBLING_COUNT &&
      operation != UNC_OP_READ_SIBLING_BEGIN) {
    ret_val = ValidateKey(reinterpret_cast<char *>(vrt_if_key->if_name),
                        kMinLenInterfaceName,
                        kMaxLenInterfaceName);
    if (ret_val != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Interface Name syntax check failed."
                  "Received if_name - %s",
                  vrt_if_key->if_name);
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  } else {
    UPLL_LOG_TRACE("Operation is %d", operation);
    StringReset(vrt_if_key->if_name);
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VrtIfMoMgr::ValidateVrtIfValue(val_vrt_if *vrt_if_val,
                                         unc_keytype_operation_t op) {
  UPLL_FUNC_TRACE;

  if (vrt_if_val->valid[UPLL_IDX_DESC_VI] == UNC_VF_VALID) {
    if (!ValidateDesc(vrt_if_val->description,
                      kMinLenDescription,
                      kMaxLenDescription)) {
      UPLL_LOG_DEBUG("Description syntax check failed."
                     "received Description - %s",
                     vrt_if_val->description);
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  } else if (vrt_if_val->valid[UPLL_IDX_DESC_VI] == UNC_VF_VALID_NO_VALUE
             && (op == UNC_OP_CREATE || op == UNC_OP_UPDATE)) {
    uuu::upll_strncpy(vrt_if_val->description, " ", 2);
  }

  /* Ip address can't be configured without prefix length / mask value
   * and both attributes must have same valid flag values during
   * CREATE or UPDATE operation.
   * so, we skip the validation for both ip address and prefixlength attributes
   * when the valid flag value is not as VALID value.
   */
  if (vrt_if_val->valid[UPLL_IDX_PREFIXLEN_VI] == UNC_VF_VALID &&
      vrt_if_val->valid[UPLL_IDX_IP_ADDR_VI] == UNC_VF_VALID) {
    if (ValidateNumericRange((uint8_t) vrt_if_val->prefixlen,
                             kMinVnodeIpv4Prefix, kMaxVnodeIpv4Prefix,
                             true, true)) {
      if (!ValidateIpv4Addr(vrt_if_val->ip_addr.s_addr,
                            vrt_if_val->prefixlen)) {
        UPLL_LOG_ERROR("IP address Validation failed");
        return UPLL_RC_ERR_CFG_SYNTAX;
      }
    } else {
      UPLL_LOG_ERROR("Invalid prefixlength range value- %d",
                     vrt_if_val->prefixlen);
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  } else {
    if ((vrt_if_val->valid[UPLL_IDX_PREFIXLEN_VI] ==
         vrt_if_val->valid[UPLL_IDX_IP_ADDR_VI]) &&
        (op == UNC_OP_UPDATE || op == UNC_OP_CREATE)) {
      vrt_if_val->ip_addr.s_addr = 0;
      vrt_if_val->prefixlen = 0;
    } else {
      UPLL_LOG_DEBUG("IP address and prefix length both do not have "
                     "same valid flags: %d, %d",
                     vrt_if_val->valid[UPLL_IDX_PREFIXLEN_VI],
                     vrt_if_val->valid[UPLL_IDX_IP_ADDR_VI]);
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  }

  if (vrt_if_val->valid[UPLL_IDX_MAC_ADDR_VI] == UNC_VF_VALID) {
    if (!ValidateMacAddr(vrt_if_val->macaddr)) {
      UPLL_LOG_DEBUG("Mac Address validation failure for KT_VRT_IF val struct."
                     " Received  mac_address is - %s",
                     vrt_if_val->macaddr);
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  }
  if (vrt_if_val->valid[UPLL_IDX_ADMIN_ST_VI] == UNC_VF_VALID) {
    if (!ValidateNumericRange(vrt_if_val->admin_status,
                              (uint8_t) UPLL_ADMIN_ENABLE,
                              (uint8_t) UPLL_ADMIN_DISABLE, true, true)) {
      UPLL_LOG_DEBUG(
          "Admin status range check failed. Received admin status -%d",
          vrt_if_val->admin_status);
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  } else if (vrt_if_val->valid[UPLL_IDX_ADMIN_ST_VI] == UNC_VF_VALID_NO_VALUE
             && (op == UNC_OP_CREATE || op == UNC_OP_UPDATE)) {
    vrt_if_val->admin_status = UPLL_ADMIN_ENABLE;
  } else if ((vrt_if_val->valid[UPLL_IDX_ADMIN_ST_VI] == UNC_VF_INVALID)
             && (op == UNC_OP_CREATE)) {
    vrt_if_val->admin_status = UPLL_ADMIN_ENABLE;
    vrt_if_val->valid[UPLL_IDX_ADMIN_ST_VI] = UNC_VF_VALID_NO_VALUE;
  }
  return UPLL_RC_SUCCESS;
}


upll_rc_t VrtIfMoMgr::ValidateVtnNeighborValue(
    val_vtn_neighbor *vtn_neighbor,
    unc_keytype_operation_t operation) {
  UPLL_FUNC_TRACE;
  upll_rc_t ret_val = UPLL_RC_SUCCESS;

  if (vtn_neighbor->valid[UPLL_IDX_CONN_VNODE_NAME_VN] == UNC_VF_VALID) {
    ret_val = ValidateKey(
        reinterpret_cast<char *>(vtn_neighbor->connected_vnode_name),
        kMinLenVnodeName, kMaxLenVnodeName);
    if (ret_val != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("connected_vnode_name syntax check failed."
                     "Received connected_vnode_name - %s",
                     vtn_neighbor->connected_vnode_name);
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  } else if (vtn_neighbor->valid[UPLL_IDX_CONN_VNODE_NAME_VN]
             == UNC_VF_VALID_NO_VALUE
      && (operation == UNC_OP_CREATE || operation == UNC_OP_UPDATE)) {
    uuu::upll_strncpy(vtn_neighbor->connected_vnode_name, " ",
                      kMaxLenVnodeName+1);
  }
  if (vtn_neighbor->valid[UPLL_IDX_CONN_VNODE_IF_NAME_VN] == UNC_VF_VALID) {
    ret_val = ValidateKey(
        reinterpret_cast<char *>(vtn_neighbor->connected_if_name),
        kMinLenInterfaceName, kMinLenInterfaceName);
    if (ret_val != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("connected_if_name syntax check failed."
                    "Received connected_if_name - %s",
                    vtn_neighbor->connected_if_name);
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  } else if (vtn_neighbor->valid[UPLL_IDX_CONN_VNODE_IF_NAME_VN]
      == UNC_VF_VALID_NO_VALUE
      && (operation == UNC_OP_CREATE || operation == UNC_OP_UPDATE)) {
    uuu::upll_strncpy(vtn_neighbor->connected_if_name, " ",
                      kMaxLenInterfaceName+1);
  }
  if (vtn_neighbor->valid[UPLL_IDX_CONN_VLINK_NAME_VN] == UNC_VF_VALID) {
    ret_val = ValidateKey(
        reinterpret_cast<char *>(vtn_neighbor->connected_vlink_name),
        kMinLenVlinkName, kMaxLenVlinkName);
    if (ret_val != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("connected_vlink_name syntax check failed."
                    "Received connected_vlink_name - %s",
                    vtn_neighbor->connected_vlink_name);
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  } else if (vtn_neighbor->valid[UPLL_IDX_CONN_VLINK_NAME_VN]
      == UNC_VF_VALID_NO_VALUE
      && (operation == UNC_OP_CREATE || operation == UNC_OP_UPDATE)) {
    uuu::upll_strncpy(vtn_neighbor->connected_vlink_name, " ",
                          kMaxLenVlinkName+1);
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VrtIfMoMgr::ValidateCapability(IpcReqRespHeader *req,
                                         ConfigKeyVal *ikey,
                                         const char *ctrlr_name) {
  UPLL_FUNC_TRACE;
  upll_rc_t ret_val = UPLL_RC_ERR_GENERIC;

  if (!req || !ikey) {
    UPLL_LOG_DEBUG("ConfigKeyVal / IpcReqRespHeader is NULL");
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

  bool result_code = false;
  uint32_t max_attrs;
  uint32_t max_instance_count;
  const uint8_t *attrs;

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
      UPLL_LOG_DEBUG("Invalid operation code");
      return UPLL_RC_ERR_GENERIC;
  }
  if (!result_code) {
    UPLL_LOG_DEBUG("key_type - %d is not supported by controller - %s",
                  ikey->get_key_type(), ctrlr_name);
    return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
  }
  val_vrt_if *vrt_if_val = NULL;
  if ((ikey->get_cfg_val())
      && ((ikey->get_cfg_val())->get_st_num() == IpctSt::kIpcStValVrtIf)) {
     vrt_if_val =  reinterpret_cast<val_vrt_if *>
                                  (ikey->get_cfg_val()->get_val());
  }
  if (vrt_if_val) {
    if (max_attrs > 0) {
      ret_val = ValVrtIfAttributeSupportCheck(vrt_if_val,
                                              attrs, req->operation);
      return ret_val;
    } else {
      UPLL_LOG_DEBUG("Attribute list is empty for operation %d",
                     req->operation);
      return UPLL_RC_ERR_GENERIC;
    }
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VrtIfMoMgr::ValVrtIfAttributeSupportCheck(
    val_vrt_if *vrt_if_val,
    const uint8_t *attrs,
    unc_keytype_operation_t operation) {
  UPLL_FUNC_TRACE;

  if (vrt_if_val != NULL) {
    if ((vrt_if_val->valid[UPLL_IDX_DESC_VI] == UNC_VF_VALID)
        || (vrt_if_val->valid[UPLL_IDX_DESC_VI] == UNC_VF_VALID_NO_VALUE)) {
      if (attrs[unc::capa::vrt_if::kCapDesc] == 0) {
        vrt_if_val->valid[UPLL_IDX_DESC_VI] = UNC_VF_INVALID;
        if (operation == UNC_OP_CREATE || operation == UNC_OP_UPDATE) {
          UPLL_LOG_DEBUG("Desc attr is not supported by ctrlr ");
          return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
        }
      }
    }
    if ((vrt_if_val->valid[UPLL_IDX_IP_ADDR_VI] == UNC_VF_VALID)
        || (vrt_if_val->valid[UPLL_IDX_IP_ADDR_VI] == UNC_VF_VALID_NO_VALUE)) {
      if (attrs[unc::capa::vrt_if::kCapIpAddr] == 0) {
        vrt_if_val->valid[UPLL_IDX_IP_ADDR_VI] = UNC_VF_INVALID;
        if (operation == UNC_OP_CREATE || operation == UNC_OP_UPDATE) {
          UPLL_LOG_DEBUG("Ip addr attr is not supported by ctrlr ");
          return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
        }
      }
    }
    if ((vrt_if_val->valid[UPLL_IDX_PREFIXLEN_VI] == UNC_VF_VALID)
       || (vrt_if_val->valid[UPLL_IDX_PREFIXLEN_VI] == UNC_VF_VALID_NO_VALUE)) {
      if (attrs[unc::capa::vrt_if::kCapPrefixlen] == 0) {
        vrt_if_val->valid[UPLL_IDX_PREFIXLEN_VI] = UNC_VF_INVALID;
        if (operation == UNC_OP_CREATE || operation == UNC_OP_UPDATE) {
          UPLL_LOG_DEBUG("Prefixlen attr is not supported by ctrlr ");
          return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
        }
      }
    }
  // #if 0  //CHECK
    if ((vrt_if_val->valid[UPLL_IDX_MAC_ADDR_VI] == UNC_VF_VALID)
        || (vrt_if_val->valid[UPLL_IDX_MAC_ADDR_VI] == UNC_VF_VALID_NO_VALUE)) {
      if (attrs[unc::capa::vrt_if::kCapMacAddr] == 0) {
        vrt_if_val->valid[UPLL_IDX_MAC_ADDR_VI] = UNC_VF_INVALID;
        if (operation == UNC_OP_CREATE || operation == UNC_OP_UPDATE) {
          UPLL_LOG_DEBUG("Mac addr attr is not supported by ctrlr ");
          return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
        }
      }
    }
    if ((vrt_if_val->valid[UPLL_IDX_ADMIN_ST_VI] == UNC_VF_VALID)
        || (vrt_if_val->valid[UPLL_IDX_ADMIN_ST_VI] == UNC_VF_VALID_NO_VALUE)) {
      if (attrs[unc::capa::vrt_if::kCapAdminStatus] == 0) {
        vrt_if_val->valid[UPLL_IDX_ADMIN_ST_VI] = UNC_VF_INVALID;
        if (operation == UNC_OP_CREATE || operation == UNC_OP_UPDATE) {
          UPLL_LOG_DEBUG("Admin status attr is not supported by ctrlr ");
          return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
        }
      }
    }
// #endif
  } else {
    UPLL_LOG_DEBUG("Error vrt_if struct is NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VrtIfMoMgr::CopyToConfigKey(ConfigKeyVal *&okey,
                                      ConfigKeyVal *ikey) {
    UPLL_FUNC_TRACE;
  if (!ikey || !(ikey->get_key())) return UPLL_RC_ERR_GENERIC;

  upll_rc_t result_code = UPLL_RC_SUCCESS;

  key_rename_vnode_info *key_rename = reinterpret_cast<key_rename_vnode_info *>
                                                    (ikey->get_key());
  key_vrt_if_t * key_vrt = reinterpret_cast<key_vrt_if_t *>
      (ConfigKeyVal::Malloc(sizeof(key_vrt_if_t)));
  if (!strlen(reinterpret_cast<char *>(key_rename->old_unc_vtn_name))) {
    FREE_IF_NOT_NULL(key_vrt);
    return UPLL_RC_ERR_GENERIC;
  }
  uuu::upll_strncpy(key_vrt->vrt_key.vtn_key.vtn_name,
         key_rename->old_unc_vtn_name, (kMaxLenVtnName+1));
  if (UNC_KT_VROUTER == ikey->get_key_type()) {
    if (!strlen(reinterpret_cast<char *>(key_rename->old_unc_vnode_name))) {
      FREE_IF_NOT_NULL(key_vrt);
      return UPLL_RC_ERR_GENERIC;
    }
    uuu::upll_strncpy(key_vrt->vrt_key.vrouter_name,
       key_rename->old_unc_vnode_name, (kMaxLenVnodeName+1));
  } else {
    if (!strlen(reinterpret_cast<char *>(key_rename->new_unc_vnode_name))) {
      FREE_IF_NOT_NULL(key_vrt);
      return UPLL_RC_ERR_GENERIC;
    }
    uuu::upll_strncpy(key_vrt->vrt_key.vrouter_name,
       key_rename->new_unc_vnode_name, (kMaxLenVnodeName+1));
  }
  okey = new ConfigKeyVal(UNC_KT_VRT_IF, IpctSt::kIpcStKeyVrtIf, key_vrt, NULL);
  if (!okey) {
    free(key_vrt);
    return UPLL_RC_ERR_GENERIC;
  }
  return result_code;
}

bool VrtIfMoMgr::FilterAttributes(void *&val1, void *val2, bool copy_to_running,
                                  unc_keytype_operation_t op) {
  val_vrt_if_t *val_vrt_if1 = reinterpret_cast<val_vrt_if_t *>(val1);
  val_vrt_if1->valid[UPLL_IDX_DESC_VI] = UNC_VF_INVALID;
  if (op != UNC_OP_CREATE)
    return CompareValidValue(val1, val2, copy_to_running);
  return false;
}

bool VrtIfMoMgr::CompareValidValue(void *&val1, void *val2,
                                   bool copy_to_running) {
  UPLL_FUNC_TRACE;
  bool invalid_attr = true;
  val_vrt_if_t *val_vrt_if1 = reinterpret_cast<val_vrt_if_t *>(val1);
  val_vrt_if_t *val_vrt_if2 = reinterpret_cast<val_vrt_if_t *>(val2);
  if (!val2) {
    UPLL_LOG_TRACE("Invalid param");
    return UPLL_RC_ERR_GENERIC;
  }
  for (unsigned int loop = 0;
       loop < sizeof(val_vrt_if1->valid) / sizeof(uint8_t); ++loop) {
    if (UNC_VF_INVALID == val_vrt_if1->valid[loop]
        && UNC_VF_VALID == val_vrt_if2->valid[loop])
      val_vrt_if1->valid[loop] = UNC_VF_VALID_NO_VALUE;
  }
  if (copy_to_running) {
    if (UNC_VF_INVALID != val_vrt_if1->valid[UPLL_IDX_DESC_VI]) {
      if ((UNC_VF_VALID == val_vrt_if1->valid[UPLL_IDX_DESC_VI]) &&
          (!strncmp(reinterpret_cast<char*>(val_vrt_if1->description),
                    reinterpret_cast<char*>(val_vrt_if2->description),
                    sizeof(val_vrt_if2->description))))
        val_vrt_if1->valid[UPLL_IDX_DESC_VI] = UNC_VF_INVALID;
    }
    if (UNC_VF_VALID == val_vrt_if1->valid[UPLL_IDX_IP_ADDR_VI]
        && UNC_VF_VALID == val_vrt_if2->valid[UPLL_IDX_IP_ADDR_VI]) {
      if (val_vrt_if1->ip_addr.s_addr == val_vrt_if2->ip_addr.s_addr)
        val_vrt_if1->valid[UPLL_IDX_IP_ADDR_VI] = UNC_VF_INVALID;
    }
    if (UNC_VF_VALID == val_vrt_if1->valid[UPLL_IDX_PREFIXLEN_VI]
        && UNC_VF_VALID == val_vrt_if2->valid[UPLL_IDX_PREFIXLEN_VI]) {
      if (val_vrt_if1->prefixlen == val_vrt_if2->prefixlen)
        val_vrt_if1->valid[UPLL_IDX_PREFIXLEN_VI] = UNC_VF_INVALID;
    }
    if (UNC_VF_VALID == val_vrt_if1->valid[UPLL_IDX_MAC_ADDR_VI]
        && UNC_VF_VALID == val_vrt_if2->valid[UPLL_IDX_MAC_ADDR_VI]) {
      if (!memcmp(reinterpret_cast<char*>(val_vrt_if1->macaddr),
                  reinterpret_cast<char*>(val_vrt_if2->macaddr),
                  sizeof(val_vrt_if2->macaddr)))
        val_vrt_if1->valid[UPLL_IDX_MAC_ADDR_VI] = UNC_VF_INVALID;
    }
  }
  val_vrt_if1->valid[UPLL_IDX_ADMIN_ST_VI] = UNC_VF_INVALID;
  // Description is not send to Controller
  if (!copy_to_running)
    val_vrt_if1->valid[UPLL_IDX_DESC_VI] = UNC_VF_INVALID;
  for (unsigned int loop = 0;
       loop < sizeof(val_vrt_if1->valid) / sizeof(val_vrt_if1->valid[0]);
       ++loop) {
    if ((UNC_VF_VALID == (uint8_t) val_vrt_if1->valid[loop]) ||
        (UNC_VF_VALID_NO_VALUE == (uint8_t) val_vrt_if1->valid[loop])) {
      invalid_attr = false;
      break;
    }
  }
  return invalid_attr;
}


upll_rc_t VrtIfMoMgr::IsReferenced(IpcReqRespHeader *req,
                                   ConfigKeyVal *ikey,
                                   DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *okey = NULL, *ckv_dhcp = NULL;
  if (!ikey || !ikey->get_key()) return UPLL_RC_ERR_GENERIC;

  result_code = GetChildConfigKey(okey, ikey);
  if (result_code != UPLL_RC_SUCCESS || okey == NULL) {
    UPLL_LOG_DEBUG("Create key returning error %d", result_code);
    return result_code;
  }
  /* Getting the Full key information with help of read operation */

  DbSubOp dbop = { kOpReadMultiple, kOpMatchNone, kOpInOutFlag };
  result_code = ReadConfigDB(okey, req->datatype, UNC_OP_READ,
                             dbop, dmi, MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    result_code = (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE)?
                   UPLL_RC_SUCCESS:result_code;
    DELETE_IF_NOT_NULL(okey);
    return result_code;
  }
  ConfigKeyVal *temkey = okey;
  while (temkey != NULL) {
    uint8_t vlink_flag = 0;
    ckv_dhcp = NULL;

    // Is interface part of vlink?
    GET_USER_DATA_FLAGS(temkey, vlink_flag);
    if (vlink_flag & VIF_TYPE) {
      delete okey;
      return UPLL_RC_ERR_CFG_SEMANTIC;
    }

    /* return error if dhcp relay is enabled on interface */
    if (IsValidKey(temkey->get_key(), uudst::vrouter_interface::kDbiIfName)) {
      MoMgrImpl *mgr = reinterpret_cast<MoMgrImpl *>(const_cast<MoManager*>
                                   (GetMoManager(UNC_KT_DHCPRELAY_IF)));
      result_code = mgr->GetChildConfigKey(ckv_dhcp, temkey);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Create key returning error %d", result_code);
        delete okey;
        return result_code;
      }
      DbSubOp dbop1 = {kOpReadExist, kOpMatchNone, kOpInOutNone};
      result_code = mgr->UpdateConfigDB(ckv_dhcp, UPLL_DT_CANDIDATE,
                         UNC_OP_READ, dmi, &dbop1, MAINTBL);
      if (result_code == UPLL_RC_ERR_INSTANCE_EXISTS) {
        uint8_t *if_name = reinterpret_cast<key_dhcp_relay_if *>
                       (ckv_dhcp->get_key())->if_name;
        UPLL_LOG_DEBUG("Cannot delete dhcp relay enabled interface %s",
                         if_name);
        delete ckv_dhcp;
        delete okey;
        return UPLL_RC_ERR_CFG_SEMANTIC;
      } else if (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
        UPLL_LOG_DEBUG(" Returning error %d", result_code);
        delete ckv_dhcp;
        delete okey;
        return result_code;
      }
      delete ckv_dhcp;
    }
    temkey = temkey->get_next_cfg_key_val();
  }
  if (okey)
    delete okey;
  return UPLL_RC_SUCCESS;
}


upll_rc_t VrtIfMoMgr::MergeValidate(unc_key_type_t keytype,
                                    const char *ctrlr_id, ConfigKeyVal *ikey,
                                    DalDmlIntf *dmi,
                                    upll_import_type import_type) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  if (!ikey || !(ikey->get_key())) {
     UPLL_LOG_DEBUG("Input is NULL");
     return UPLL_RC_ERR_GENERIC;
  }
  ConfigKeyVal *dup_key = NULL;
  result_code = GetChildConfigKey(dup_key, ikey);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetChildConfigKey Failed");
    if (dup_key) delete dup_key;
    return result_code;
  }
  /*
   * Here getting FULL Key (VTN & VRT Name )
   */
  DbSubOp dbop = { kOpReadMultiple, kOpMatchNone, kOpInOutNone };
  result_code = ReadConfigDB(dup_key, UPLL_DT_IMPORT, UNC_OP_READ, dbop, dmi,
                             MAINTBL);
  if (UPLL_RC_SUCCESS != result_code) {
    if (dup_key) delete dup_key;
    return result_code;
  }
  ConfigKeyVal * travel = dup_key;
  while (travel) {
    ConfigKeyVal *tkey = NULL;
    result_code = DupConfigKeyVal(tkey, travel, MAINTBL);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG(" DupConfigKeyVal is Failed");
      DELETE_IF_NOT_NULL(tkey);
      DELETE_IF_NOT_NULL(dup_key);
      return result_code;
    }
    result_code = MergeValidateIpAddress(tkey, UPLL_DT_CANDIDATE,
                                         dmi, ctrlr_id, import_type);
    if (result_code != UPLL_RC_SUCCESS) {
      /* If Merge conflict ikey should reset with that ConfigKeyVal */
      if (result_code == UPLL_RC_ERR_MERGE_CONFLICT) {
        ikey->ResetWith(tkey);
        DELETE_IF_NOT_NULL(tkey);
        DELETE_IF_NOT_NULL(dup_key);
        return UPLL_RC_ERR_MERGE_CONFLICT;
      }
      DELETE_IF_NOT_NULL(tkey);
      DELETE_IF_NOT_NULL(dup_key);
      return result_code;
    }
    DELETE_IF_NOT_NULL(tkey);
    travel = travel->get_next_cfg_key_val();
  }  //  while end
  if (dup_key)
    delete dup_key;
  return result_code;
}



upll_rc_t VrtIfMoMgr::GetVexternal(ConfigKeyVal *ikey,
                                   upll_keytype_datatype_t data_type,
                                   DalDmlIntf *dmi,
                          uint8_t *vexternal, uint8_t *vex_if,
                          InterfacePortMapInfo &iftype) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  if (!ikey) {
    UPLL_LOG_DEBUG("Input Key is NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  MoMgrImpl *mgr = reinterpret_cast<MoMgrImpl *>(const_cast<MoManager*>(
                                    GetMoManager(UNC_KT_VLINK)));
  if (!mgr) {
     UPLL_LOG_DEBUG("Instance is NULL");
     return UPLL_RC_ERR_GENERIC;
  }
  switch (data_type) {
    case UPLL_DT_CANDIDATE:
    case UPLL_DT_IMPORT:
           result_code = mgr->CheckVnodeInfo(ikey, data_type, dmi);
           break;
    case UPLL_DT_AUDIT:
           result_code = mgr->CheckVnodeInfo(ikey, UPLL_DT_RUNNING, dmi);
           break;
    default:
           UPLL_LOG_DEBUG("Invalid Datatype %d", data_type);
           return UPLL_RC_ERR_GENERIC;
           break;
  }
  if (UPLL_RC_SUCCESS != result_code
      && UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
    UPLL_LOG_DEBUG("CheckVnodeInfo Failed %d", result_code);
    return UPLL_RC_ERR_GENERIC;
  }
  if (UPLL_RC_SUCCESS == result_code) {
     iftype = kVlinkConfigured;
  } else {
      if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
        iftype = kVlinkPortMapNotConfigured;
      } else {
         UPLL_LOG_DEBUG("Error %d", result_code);
         return result_code;
     }
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VrtIfMoMgr::AdaptValToDriver(ConfigKeyVal *ck_new,
                                       ConfigKeyVal *ck_old,
                                       unc_keytype_operation_t op,
                                       upll_keytype_datatype_t dt_type,
                                       unc_key_type_t keytype,
                                       DalDmlIntf *dmi,
                                       bool &not_send_to_drv,
                                       bool audit_update_phase) {
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  if (!audit_update_phase) {
    if (op == UNC_OP_DELETE) {
      // Perform semantic check for vnode interface deletion
      // check whether the vnode interface is referred in the
      // flowfilter-redirection.
      result_code = CheckVnodeInterfaceForRedirection(ck_new,
                                                      ck_old,
                                                      dmi,
                                                      UPLL_DT_CANDIDATE,
                                                      op);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_INFO("Flowfilter redirection validation fails for keytype %d"
                      " cannot perform operation  %d errcode %d",
                      keytype, op, result_code);
        return result_code;
      }
    }
  }
  return result_code;
}

upll_rc_t VrtIfMoMgr::ValidateVtnRename(ConfigKeyVal *org_vtn_ckv,
                                        ConfigKeyVal *rename_vtn_ckv,
                                        DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t     result_code = UPLL_RC_SUCCESS;
  DbSubOp       dbop        = { kOpReadMultiple, kOpMatchNone, kOpInOutNone };
  ConfigKeyVal *vrtif_ckv = NULL;

  if (!org_vtn_ckv || !org_vtn_ckv->get_key() ||
      !rename_vtn_ckv || !rename_vtn_ckv->get_key()) {
    UPLL_LOG_DEBUG("Input is NULL");
    return UPLL_RC_ERR_GENERIC;
  }

  result_code = GetChildConfigKey(vrtif_ckv, org_vtn_ckv);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetChildConfigKey Failed");
    return result_code;
  }

  /* Gets All VRT_IF ConfigKeyVall based on original vtn name */
  result_code = ReadConfigDB(vrtif_ckv, UPLL_DT_IMPORT, UNC_OP_READ, dbop, dmi,
                             MAINTBL);
  if (UPLL_RC_SUCCESS != result_code) {
    if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code)
      result_code = UPLL_RC_SUCCESS;

    DELETE_IF_NOT_NULL(vrtif_ckv);
    return result_code;
  }

  uint8_t *vtn_rename = reinterpret_cast<key_vtn_t*>(
      rename_vtn_ckv->get_key())->vtn_name;
  ConfigKeyVal *travel = vrtif_ckv;
  while (travel) {
    /* Verifies whether the same ip address is exist under the
     * new vtn name */
    uuu::upll_strncpy(reinterpret_cast<key_vrt_if_t*>(
                   travel->get_key())->vrt_key.vtn_key.vtn_name,
                   vtn_rename,
                   (kMaxLenVtnName + 1));
    /* MAC Address Semantic validation within the IMPORT table is
     * not required. MAC Address is unique for that Controller to be import*/
    reinterpret_cast<val_vrt_if_t*>(GetVal(travel))->valid[UPLL_IDX_MAC_ADDR_VI]
        = UNC_VF_INVALID;
    result_code = MergeValidateIpAddress(travel, UPLL_DT_IMPORT, dmi, NULL,
                                         UPLL_IMPORT_TYPE_FULL);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG(
          "Vrouter IP Address validation failed during VTN stiching");
      DELETE_IF_NOT_NULL(vrtif_ckv);
      return result_code;
    }

    travel = travel->get_next_cfg_key_val();
  }
  DELETE_IF_NOT_NULL(vrtif_ckv);

  return UPLL_RC_SUCCESS;
}

}  // namespace kt_momgr
}  // namespace upll
}  // namespace unc
