/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include "iproute_momgr.hh"
#define NUM_KEY_MAIN_TBL_ 6
#if 0
namespace upll_dal_vbrif unc::upll::dal::schema::table::vbridge_interface;
#endif

namespace unc {
namespace upll {
namespace kt_momgr {

BindInfo IpRouteMoMgr::ip_route_bind_info[] = {
    { uudst::static_ip_route::kDbiVtnName, CFG_KEY, offsetof(
        key_static_ip_route, vrt_key.vtn_key.vtn_name),
      uud::kDalChar, 32 },
    { uudst::static_ip_route::kDbiVrouterName, CFG_KEY, offsetof(
        key_static_ip_route, vrt_key.vrouter_name),
      uud::kDalChar, 32 },
    { uudst::static_ip_route::kDbiDstIpAddr, CFG_KEY, offsetof(
        key_static_ip_route, dst_addr.s_addr),
      uud::kDalUint32, 1 },
    { uudst::static_ip_route::kDbiMask, CFG_KEY, offsetof(key_static_ip_route,
                                                          dst_addr_prefixlen),
      uud::kDalUint8, 1 },
    { uudst::static_ip_route::kDbiNextHopAddr, CFG_KEY, offsetof(
        key_static_ip_route, next_hop_addr.s_addr),
      uud::kDalUint32, 1 },
    { uudst::static_ip_route::kDbiNwmName, CFG_KEY, offsetof(
        key_static_ip_route, nwm_name),
      uud::kDalChar, 32 },
    { uudst::static_ip_route::kDbiMetric, CFG_VAL, offsetof(val_static_ip_route,
                                                            group_metric),
      uud::kDalUint16, 1 },
    { uudst::static_ip_route::kDbiCtrlrName, CK_VAL, offsetof(key_user_data,
                                                              ctrlr_id),
      uud::kDalChar, 32 },
    { uudst::static_ip_route::kDbiDomainId, CK_VAL, offsetof(key_user_data,
                                                             domain_id),
      uud::kDalChar, 32 },
    { uudst::static_ip_route::kDbiValidMetric, CFG_META_VAL, offsetof(
        val_static_ip_route, valid[UPLL_IDX_GROUP_METRIC_SIR]),
      uud::kDalUint8, 1 },
    { uudst::static_ip_route::kDbiCsMetric, CS_VAL, offsetof(
        val_static_ip_route, cs_attr[UPLL_IDX_GROUP_METRIC_SIR]),
      uud::kDalUint8, 1 },
    { uudst::static_ip_route::kDbiCsRowstatus, CS_VAL, offsetof(
        val_static_ip_route, cs_row_status),
      uud::kDalUint8, 1 },
    { uudst::static_ip_route::kDbiFlags, CK_VAL, offsetof(key_user_data, flags),
      uud::kDalUint8, 1 }, };

BindInfo IpRouteMoMgr::key_ip_route_maintbl_update_bind_info[] = {
    { uudst::static_ip_route::kDbiVtnName, CFG_MATCH_KEY, offsetof(
        key_static_ip_route_t, vrt_key.vtn_key.vtn_name),
      uud::kDalChar, kMaxLenVtnName + 1 },
    { uudst::static_ip_route::kDbiVrouterName, CFG_MATCH_KEY, offsetof(
        key_static_ip_route_t, vrt_key.vrouter_name),
      uud::kDalChar, kMaxLenVnodeName + 1 },
    { uudst::static_ip_route::kDbiNwmName, CFG_MATCH_KEY, offsetof(
        key_static_ip_route, nwm_name),
      uud::kDalChar, kMaxLenVnodeName + 1 },
    { uudst::static_ip_route::kDbiVtnName, CFG_INPUT_KEY, offsetof(
        key_rename_vnode_info_t, new_unc_vtn_name),
      uud::kDalChar, kMaxLenVtnName + 1 },
    { uudst::static_ip_route::kDbiVrouterName, CFG_INPUT_KEY, offsetof(
        key_rename_vnode_info_t, new_unc_vnode_name),
      uud::kDalChar, kMaxLenVnodeName + 1 },
    { uudst::static_ip_route::kDbiFlags, CK_VAL, offsetof(
        key_user_data_t, flags),
      uud::kDalUint8, 1 } };

IpRouteMoMgr::IpRouteMoMgr() {
  UPLL_FUNC_TRACE
  ntable = MAX_MOMGR_TBLS;
  table = new Table *[ntable];
  table[MAINTBL] = new Table(uudst::kDbiStaticIpRouteTbl, UNC_KT_VRT_IPROUTE,
                         ip_route_bind_info, IpctSt::kIpcStKeyStaticIpRoute,
                         IpctSt::kIpcStValStaticIpRoute,
                         uudst::static_ip_route::kDbiStaticIpRouteNumCols);
  table[RENAMETBL] = NULL;
  table[CTRLRTBL] = NULL;
  nchild = 0;
  child = NULL;
}

/*
 * Based on the key type the bind info will pass
 **/
bool IpRouteMoMgr::GetRenameKeyBindInfo(unc_key_type_t key_type,
                                        BindInfo *&binfo, int &nattr,
                                        MoMgrTables tbl) {
  /* Main Table or rename table only update */
  if (MAINTBL == tbl) {
    nattr = NUM_KEY_MAIN_TBL_;
    binfo = key_ip_route_maintbl_update_bind_info;
  } else {
    UPLL_LOG_DEBUG("Invalid Table");
    return PFC_FALSE;
  }
  return PFC_TRUE;
}

upll_rc_t IpRouteMoMgr::ValidateAttribute(ConfigKeyVal *ikey, 
                                          DalDmlIntf *dmi,
                                          IpcReqRespHeader *req) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  if (ikey->get_key_type() != UNC_KT_VRT_IPROUTE) result_code =
      UPLL_RC_ERR_CFG_SYNTAX;
  return result_code;
}

bool IpRouteMoMgr::IsValidKey(void *key, uint64_t index) {
  key_static_ip_route *iproute_key =
      reinterpret_cast<key_static_ip_route*>(key);
  pfc_log_trace("Entering IsValidKey");
  uint32_t val = 0;
  bool ret_val = UPLL_RC_SUCCESS;
  switch (index) {
    case uudst::static_ip_route::kDbiVtnName:
      ret_val = ValidateKey(
          reinterpret_cast<char *>(iproute_key->vrt_key.vtn_key.vtn_name),
          kMinLenVtnName, kMaxLenVtnName);
      if (ret_val != UPLL_RC_SUCCESS) {
        pfc_log_trace("VTN Name is not valid(%d)", ret_val);
        return false;
      }
      break;
    case uudst::static_ip_route::kDbiVrouterName:
      ret_val = ValidateKey(
          reinterpret_cast<char *>(iproute_key->vrt_key.vrouter_name),
          kMinLenVnodeName, kMaxLenVnodeName);
      if (ret_val != UPLL_RC_SUCCESS) {
        pfc_log_trace("VRouter Name is not valid(%d)", ret_val);
        return false;
      }
      break;
    case uudst::static_ip_route::kDbiDstIpAddr:
      val = iproute_key->dst_addr.s_addr;
      if (val == 0xffffffff || val == 0x00000000) {
        pfc_log_trace("Invalid destination Address");
        return false;
      }
      break;
    case uudst::static_ip_route::kDbiNextHopAddr:
      val = iproute_key->next_hop_addr.s_addr;
      if (val == 0xffffffff || val == 0x00000000) {
        pfc_log_trace("Invalid Next Hop Addr Address");
        return false;
      }
      break;
    case uudst::static_ip_route::kDbiMask:
      if (!ValidateNumericRange(iproute_key->dst_addr_prefixlen,
                                (uint8_t) kMinIpv4Prefix,
                                (uint8_t) kMaxIpv4Prefix, true, true)) {
        pfc_log_trace(
            "Numeric range check failure for iproute_key->dst_addr_prefixlen");
        return false;
      }
      break;
    case uudst::static_ip_route::kDbiNwmName:
      ret_val = ValidateKey(reinterpret_cast<char *>(iproute_key->nwm_name),
                            kMinLenNwmName, kMaxLenNwmName);
      if (ret_val != UPLL_RC_SUCCESS) {
        pfc_log_trace("syntax check failure for iproute_key->nwm_name");
        return false;
      }
      break;
    default:
//      pfc_log_trace("Invalid Key Index %d", index);
      break;
  }
  pfc_log_trace("Leaving IsValidKey");
  return true;
}

upll_rc_t IpRouteMoMgr::GetChildConfigKey(ConfigKeyVal *&okey,
                                          ConfigKeyVal *parent_key) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  key_static_ip_route *vrt_ip_route;
  void *pkey;
  if (parent_key == NULL) {
    vrt_ip_route = reinterpret_cast<key_static_ip_route *>
        (malloc(sizeof(key_static_ip_route)));
    if (!vrt_ip_route) return UPLL_RC_ERR_GENERIC;
    memset(vrt_ip_route, 0, sizeof(key_static_ip_route));
    okey = new ConfigKeyVal(UNC_KT_VRT_IPROUTE, IpctSt::kIpcStKeyStaticIpRoute,
                            vrt_ip_route, NULL);
    return UPLL_RC_SUCCESS;
  } else {
    pkey = parent_key->get_key();
  }
  if (!pkey) return UPLL_RC_ERR_GENERIC;
  if (okey) {
    if (okey->get_key_type() != UNC_KT_VRT_IPROUTE) return UPLL_RC_ERR_GENERIC;
    vrt_ip_route = reinterpret_cast<key_static_ip_route *>(okey->get_key());
  } else {
    vrt_ip_route = reinterpret_cast<key_static_ip_route *>
        (malloc(sizeof(key_static_ip_route)));
    if (!vrt_ip_route) return UPLL_RC_ERR_GENERIC;
    memset(vrt_ip_route, 0, sizeof(key_static_ip_route));
  }
  unc_key_type_t keytype = parent_key->get_key_type();
  vrt_ip_route->dst_addr.s_addr = 0;
  vrt_ip_route->dst_addr_prefixlen = 0;
  vrt_ip_route->next_hop_addr.s_addr = 0;
  *(vrt_ip_route->nwm_name) = *"";
  switch (keytype) {
    case UNC_KT_VTN:
      uuu::upll_strncpy(vrt_ip_route->vrt_key.vtn_key.vtn_name,
                        reinterpret_cast<key_vtn *>(pkey)->vtn_name,
                        (kMaxLenVtnName + 1));
      *(vrt_ip_route->vrt_key.vrouter_name) = *"";
      break;
    case UNC_KT_VROUTER:
      uuu::upll_strncpy(vrt_ip_route->vrt_key.vtn_key.vtn_name,
                        reinterpret_cast<key_vrt *>(pkey)->vtn_key.vtn_name,
                        (kMaxLenVtnName + 1));
      uuu::upll_strncpy(vrt_ip_route->vrt_key.vrouter_name,
                        reinterpret_cast<key_vrt *>(pkey)->vrouter_name,
                        (kMaxLenVnodeName + 1));
      break;
    case UNC_KT_VRT_IPROUTE:
      uuu::upll_strncpy(vrt_ip_route->vrt_key.vtn_key.vtn_name,
                        reinterpret_cast<key_static_ip_route *>
                        (pkey)->vrt_key.vtn_key.vtn_name,
                        (kMaxLenVtnName + 1));
      uuu::upll_strncpy(vrt_ip_route->vrt_key.vrouter_name,
                        reinterpret_cast<key_static_ip_route *>
                        (pkey)->vrt_key.vrouter_name,
                        (kMaxLenVnodeName + 1));
      if (strlen(reinterpret_cast<char *>
                 (reinterpret_cast<key_static_ip_route *>(pkey)->nwm_name))) {
        uuu::upll_strncpy(vrt_ip_route->nwm_name,
                      reinterpret_cast<key_static_ip_route *>(pkey)->nwm_name,
                      (kMaxLenNwmName + 1));
      }
      vrt_ip_route->dst_addr.s_addr =
          reinterpret_cast<key_static_ip_route *>(pkey)->dst_addr.s_addr;
      vrt_ip_route->dst_addr_prefixlen =
          reinterpret_cast<key_static_ip_route *>(pkey)->dst_addr_prefixlen;
      vrt_ip_route->next_hop_addr.s_addr =
          reinterpret_cast<key_static_ip_route *>(pkey)->next_hop_addr.s_addr;
      break;
    default:
      free(vrt_ip_route);
      return UPLL_RC_ERR_GENERIC;
  }
  if (!okey)
    okey = new ConfigKeyVal(UNC_KT_VRT_IPROUTE, IpctSt::kIpcStKeyStaticIpRoute,
                            vrt_ip_route, NULL);
  if (okey == NULL) {
    free(vrt_ip_route);
    result_code = UPLL_RC_ERR_GENERIC;
  } else {
    SET_USER_DATA(okey, parent_key);
  }
  return result_code;
}

upll_rc_t IpRouteMoMgr::GetParentConfigKey(ConfigKeyVal *&okey,
                                           ConfigKeyVal *ikey) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  if (!ikey) {
    UPLL_LOG_DEBUG("ikey is NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  unc_key_type_t ikey_type = ikey->get_key_type();

  if (ikey_type != UNC_KT_VRT_IPROUTE) return UPLL_RC_ERR_GENERIC;
  key_static_ip_route *pkey = reinterpret_cast<key_static_ip_route *>
      (ikey->get_key());
  if (!pkey) return UPLL_RC_ERR_GENERIC;
  key_vrt *vrt_key = reinterpret_cast<key_vrt *>(malloc(sizeof(key_vrt)));
  if (!vrt_key) return UPLL_RC_ERR_GENERIC;
  memset(vrt_key, 0, sizeof(key_vrt));
  uuu::upll_strncpy(vrt_key->vtn_key.vtn_name,
         pkey->vrt_key.vtn_key.vtn_name, (kMaxLenVtnName + 1));
  uuu::upll_strncpy(vrt_key->vrouter_name,
         pkey->vrt_key.vrouter_name, (kMaxLenVnodeName + 1));
  okey = new ConfigKeyVal(UNC_KT_VROUTER, IpctSt::kIpcStKeyVrt, vrt_key, NULL);
  if (okey == NULL) {
    free(vrt_key);
    result_code = UPLL_RC_ERR_GENERIC;
  } else {
    SET_USER_DATA(okey, ikey);
  }
  return result_code;
}

upll_rc_t IpRouteMoMgr::AllocVal(ConfigVal *&ck_val,
                                 upll_keytype_datatype_t dt_type,
                                 MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  void *val;  // , *nxt_val;
  // ConfigVal *ck_nxtval;

  if (ck_val != NULL) return UPLL_RC_ERR_GENERIC;
  switch (tbl) {
    case MAINTBL:
      val = malloc(sizeof(val_static_ip_route));
      if (!val) return UPLL_RC_ERR_GENERIC;
      memset(val, 0, sizeof(val_static_ip_route));
      ck_val = new ConfigVal(IpctSt::kIpcStValStaticIpRoute, val);
      break;
    default:
      val = NULL;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t IpRouteMoMgr::DupConfigKeyVal(ConfigKeyVal *&okey, ConfigKeyVal *&req,
                                        MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  if (req == NULL) return UPLL_RC_ERR_GENERIC;
  if (okey != NULL) return UPLL_RC_ERR_GENERIC;
  if (req->get_key_type() != UNC_KT_VRT_IPROUTE) return UPLL_RC_ERR_GENERIC;
  ConfigVal *tmp1 = NULL, *tmp = (req)->get_cfg_val();

  if (tmp) {
    if (tbl == MAINTBL) {
      val_static_ip_route *ival =
          reinterpret_cast<val_static_ip_route *>(GetVal(req));
      val_static_ip_route *vrt_val =
          reinterpret_cast<val_static_ip_route *>(malloc(
          sizeof(val_static_ip_route)));
      if (!vrt_val) return UPLL_RC_ERR_GENERIC;
      memcpy(vrt_val, ival, sizeof(val_static_ip_route));
      tmp1 = new ConfigVal(IpctSt::kIpcStValStaticIpRoute, vrt_val);
    }
  };
  void *tkey = (req != NULL) ? (req)->get_key() : NULL;
  key_static_ip_route *ikey;
  if (tkey) {
    ikey = reinterpret_cast<key_static_ip_route *>(tkey);
  } else {
    delete tmp1;
    return UPLL_RC_ERR_GENERIC;
  }
  key_static_ip_route *vrt_key =
      reinterpret_cast<key_static_ip_route *>(malloc(
      sizeof(key_static_ip_route)));
  if (!vrt_key) return UPLL_RC_ERR_GENERIC;
  memcpy(vrt_key, ikey, sizeof(key_static_ip_route));
  okey = new ConfigKeyVal(UNC_KT_VRT_IPROUTE, IpctSt::kIpcStKeyStaticIpRoute,
                          vrt_key, tmp1);
  if (okey) SET_USER_DATA(okey, req);
  return UPLL_RC_SUCCESS;
}

upll_rc_t IpRouteMoMgr::UpdateConfigStatus(ConfigKeyVal *ikey,
                                           unc_keytype_operation_t op,
                                           uint32_t driver_result,
                                           ConfigKeyVal *upd_key, 
                                           DalDmlIntf *dmi,
                                           ConfigKeyVal *ctrlr_key) {
  UPLL_FUNC_TRACE;
  val_static_ip_route *vrt_val =
     reinterpret_cast<val_static_ip_route *>(GetVal(ikey));

  unc_keytype_configstatus_t cs_status =
      (driver_result == 0) ? UNC_CS_APPLIED : UNC_CS_NOT_APPLIED;
  if (vrt_val == NULL) return UPLL_RC_ERR_GENERIC;
  vrt_val->cs_row_status = cs_status;
  if (op == UNC_OP_CREATE) {
    for (unsigned int loop = 0;
         loop < (sizeof(vrt_val->valid) / sizeof(vrt_val->valid[0]));
        ++loop) {
      // Setting CS to the not supported attributes
      if (UNC_VF_NOT_SOPPORTED == vrt_val->valid[loop]) {
        vrt_val->cs_attr[loop] = UNC_CS_NOT_SUPPORTED;
        continue;
      }
      if ((UNC_VF_VALID == (uint8_t) vrt_val->valid[loop])
          || (UNC_VF_VALID_NO_VALUE == (uint8_t) vrt_val->valid[loop]))
        vrt_val->cs_attr[loop] = cs_status;
    }
  } else if (op == UNC_OP_UPDATE) {
    void *val = reinterpret_cast<void *>(&vrt_val);
    CompareValidValue(val, GetVal(upd_key), true);
    for (unsigned int loop = 0;
         loop < (sizeof(vrt_val->valid) / sizeof(vrt_val->valid[0]));
        ++loop) {
      if ((uint8_t) vrt_val->valid[loop] != UNC_VF_NOT_SOPPORTED) {
        if ((UNC_VF_VALID == (uint8_t) vrt_val->valid[loop])
            || (UNC_VF_VALID_NO_VALUE == (uint8_t) vrt_val->valid[loop]))
          vrt_val->cs_attr[loop] = cs_status;
      } else {
        vrt_val->cs_attr[loop] = UNC_CS_NOT_SUPPORTED;
      }
    }
  } else {
    return UPLL_RC_ERR_GENERIC;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t IpRouteMoMgr::UpdateAuditConfigStatus(
    unc_keytype_configstatus_t cs_status, uuc::UpdateCtrlrPhase phase,
    ConfigKeyVal *&ckv_running) {
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  val_static_ip_route_t *val;
  val = (ckv_running != NULL) ?
      reinterpret_cast<val_static_ip_route_t *>(GetVal(ckv_running)) : NULL;
  if (NULL == val) {
    return UPLL_RC_ERR_GENERIC;
  }
  if (uuc::kUpllUcpCreate == phase) val->cs_row_status = cs_status;
  for (unsigned int loop = 0; loop < sizeof(val->valid) / sizeof(uint8_t);
      ++loop) {
    if ((cs_status == UNC_CS_INVALID && UNC_VF_VALID == val->valid[loop]) || 
         cs_status == UNC_CS_APPLIED) {
      val->cs_attr[loop] = cs_status;
    }
  }
  return result_code;
}

upll_rc_t IpRouteMoMgr::ValidateMessage(IpcReqRespHeader *req,
                                        ConfigKeyVal *ikey) {
  UPLL_FUNC_TRACE;
  /** read datatype, operation, options from IpcReqRespHeader */
  if (!ikey || !req || !(ikey->get_key())) {
    UPLL_LOG_DEBUG("ConfigKeyVal / IpcReqRespHeader is NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  upll_keytype_datatype_t dt_type = req->datatype;
  unc_keytype_operation_t op = req->operation;
  unc_keytype_option1_t opt1 = req->option1;
  unc_keytype_option2_t opt2 = req->option2;

  upll_rc_t ret_val = UPLL_RC_SUCCESS;
  if (ikey->get_st_num() != IpctSt::kIpcStKeyStaticIpRoute) {
    UPLL_LOG_DEBUG("Invalid key structure received. received struct - %d",
                  ikey->get_st_num());
    return UPLL_RC_ERR_BAD_REQUEST;
  }
  key_static_ip_route *iproute_key =
      reinterpret_cast<key_static_ip_route *>(ikey->get_key());
  unc_key_type_t ktype = ikey->get_key_type();
  if (UNC_KT_VRT_IPROUTE != ktype) {
    UPLL_LOG_DEBUG("Invalid Keytype received. received %d", ktype);
    return UPLL_RC_ERR_BAD_REQUEST;
  }
  ret_val = ValidateIpRouteKey(iproute_key, op);

  if (ret_val != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("syntax check failure for key_static_ip_route struct");
    return UPLL_RC_ERR_CFG_SYNTAX;
  }
  if (op == UNC_OP_CREATE || op == UNC_OP_UPDATE || op == UNC_OP_DELETE) {
    if (dt_type == UPLL_DT_CANDIDATE || UPLL_DT_IMPORT == dt_type) {
      if (NULL == ikey->get_cfg_val()) {
        UPLL_LOG_DEBUG("ConfigVal struct is empty");
        return UPLL_RC_SUCCESS;
      }
      if (ikey->get_cfg_val()->get_st_num() != IpctSt::kIpcStValStaticIpRoute) {
        UPLL_LOG_DEBUG(
            "value structure matching is invalid. st.num - %d",
            ikey->get_cfg_val()->get_st_num());
        return UPLL_RC_ERR_BAD_REQUEST;
      }
      val_static_ip_route *iproute_val =
        reinterpret_cast<val_static_ip_route*>(ikey->get_cfg_val()->get_val());
      if (iproute_val == NULL) {
        UPLL_LOG_DEBUG("val struct is mandatory for create and update op");
        return UPLL_RC_ERR_BAD_REQUEST;
      }
      ret_val = ValidateIpRouteValue(iproute_val, op);
      if (ret_val != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("syntax check failure for val_static_ip_route structure");
        return UPLL_RC_ERR_CFG_SYNTAX;
      }
      return UPLL_RC_SUCCESS;
    } else {
      UPLL_LOG_DEBUG("Unsupported data type - (%d)", dt_type);
      return UPLL_RC_ERR_NO_SUCH_INSTANCE;
    }
  } else if (op == UNC_OP_READ || op == UNC_OP_READ_SIBLING
      || op == UNC_OP_READ_SIBLING_BEGIN || op == UNC_OP_READ_SIBLING_COUNT) {
    if (dt_type == UPLL_DT_CANDIDATE || dt_type == UPLL_DT_RUNNING
        || dt_type == UPLL_DT_STARTUP || dt_type == UPLL_DT_STATE) {
      if (opt1 != UNC_OPT1_NORMAL) {
        UPLL_LOG_DEBUG("Error option1 is not NORMAL");
        return UPLL_RC_ERR_INVALID_OPTION1;
      }
      if (opt2 != UNC_OPT2_NONE) {
        UPLL_LOG_DEBUG("Error option2 is not NONE");
        return UPLL_RC_ERR_INVALID_OPTION2;
      }
      if (NULL == ikey->get_cfg_val()) {
        UPLL_LOG_DEBUG("ConfigVal struct is empty");
        return UPLL_RC_SUCCESS;
      }
      if (ikey->get_cfg_val()->get_st_num() != IpctSt::kIpcStValStaticIpRoute) {
        UPLL_LOG_DEBUG(
            "value structure matching is invalid. st.num - %d",
            ikey->get_cfg_val()->get_st_num());
        return UPLL_RC_ERR_BAD_REQUEST;
      }
      val_static_ip_route *iproute_val =
          reinterpret_cast<val_static_ip_route *>
          (ikey->get_cfg_val()->get_val());
      if (iproute_val == NULL) {
        UPLL_LOG_DEBUG(
            "syntax check for val_static_ip_route struct is an optional");
        return UPLL_RC_SUCCESS;
      }
      ret_val = ValidateIpRouteValue(iproute_val, op);
      if (ret_val != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("syntax check failure for val_static_ip_route structure");
        return UPLL_RC_ERR_CFG_SYNTAX;
      }
      return UPLL_RC_SUCCESS;
    } else {
      UPLL_LOG_DEBUG("Unsupported data type - (%d)", dt_type);
      return UPLL_RC_ERR_NO_SUCH_INSTANCE;
    }
  } else if (op == UNC_OP_READ_NEXT || op == UNC_OP_READ_BULK) {
    if (dt_type == UPLL_DT_CANDIDATE || dt_type == UPLL_DT_RUNNING
        || dt_type == UPLL_DT_STARTUP) {
      UPLL_LOG_DEBUG("Value structure is none for operation type:%d", op);
      return UPLL_RC_SUCCESS;
    } else {
      UPLL_LOG_DEBUG("Unsupported data type - (%d)", dt_type);
      return UPLL_RC_ERR_NO_SUCH_INSTANCE;
    }
  }
  UPLL_LOG_DEBUG("Unsupported operation - (%d)", op);
  return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT;
}

upll_rc_t IpRouteMoMgr::ValidateIpRouteKey(key_static_ip_route *iproute_key,
                          unc_keytype_operation_t operation) {
  UPLL_FUNC_TRACE;
  upll_rc_t ret_val = UPLL_RC_SUCCESS;

  ret_val = ValidateKey(
      reinterpret_cast<char *>(iproute_key->vrt_key.vtn_key.vtn_name),
      kMinLenVtnName, kMaxLenVtnName);
  if (ret_val != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("syntax check failed. vtn_name - %s",
                  iproute_key->vrt_key.vtn_key.vtn_name);
    return UPLL_RC_ERR_CFG_SYNTAX;
  }
  ret_val = ValidateKey(
      reinterpret_cast<char *>(iproute_key->vrt_key.vrouter_name),
      kMinLenVnodeName, kMaxLenVnodeName);
  if (ret_val != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("syntax check failed. vrouter name -%s",
                  iproute_key->vrt_key.vrouter_name);
    return UPLL_RC_ERR_CFG_SYNTAX;
  }
  if ((operation != UNC_OP_READ_SIBLING_BEGIN) &&
      (operation != UNC_OP_READ_SIBLING_COUNT)) {
    if (strlen(reinterpret_cast<char*>(iproute_key->nwm_name)) != 0) {
      ret_val = ValidateKey(reinterpret_cast<char *>(iproute_key->nwm_name),
                            kMinLenNwmName,
                            kMaxLenNwmName);
      if (ret_val != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("syntax check failed. nwm name - %s", iproute_key->nwm_name);
        return UPLL_RC_ERR_CFG_SYNTAX;
      }
    }
    if (!ValidateNumericRange(iproute_key->dst_addr_prefixlen,
                            (uint8_t) kMinIpv4Prefix, (uint8_t) kMaxIpv4Prefix,
                            true, true)) {
      UPLL_LOG_DEBUG("Numeric range check failed. dst_addr_prefixlen - %d",
                    iproute_key->dst_addr_prefixlen);
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
    if (iproute_key->dst_addr.s_addr == 0xffffffff) {
      UPLL_LOG_DEBUG("Bad destination address. addr:0x%08x\n",
                    iproute_key->dst_addr.s_addr);
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
    if (iproute_key->next_hop_addr.s_addr == 0xffffffff) {
      UPLL_LOG_DEBUG("Bad next hop address. addr:0x%08x\n",
                    iproute_key->next_hop_addr.s_addr);
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  } else {
      UPLL_LOG_TRACE("Operation is %d", operation);
      StringReset(iproute_key->nwm_name); 
      iproute_key->dst_addr_prefixlen = 0;
      iproute_key->dst_addr.s_addr = 0x00000000;
      iproute_key->next_hop_addr.s_addr = 0x00000000;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t IpRouteMoMgr::ValidateIpRouteValue(val_static_ip_route *iproute_val,
                                             uint32_t op) {
  UPLL_FUNC_TRACE;

  if (iproute_val->valid[UPLL_IDX_GROUP_METRIC_SIR] == UNC_VF_VALID) {
    if (!ValidateNumericRange(iproute_val->group_metric,
                              (uint16_t) kMinLenGroupMetric,
                              (uint16_t) kMaxLenGroupMetric, true, true)) {
      UPLL_LOG_DEBUG(
          "Numeric range check failure for vrt_ip_route_val->group_metric");
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  } else if (iproute_val->valid[UPLL_IDX_GROUP_METRIC_SIR]
      == UNC_VF_VALID_NO_VALUE
      && (op == UNC_OP_CREATE || op == UNC_OP_UPDATE)) {
    iproute_val->group_metric = 0;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t IpRouteMoMgr::ValidateCapability(IpcReqRespHeader *req,
                                           ConfigKeyVal *ikey,
                                           const char *ctrlr_name) {
  UPLL_FUNC_TRACE;
  upll_rc_t ret_val = UPLL_RC_SUCCESS;
  /** read datatype, operation, options from IpcReqRespHeader */
  if (!req || !ikey) {
    UPLL_LOG_DEBUG("IpcReqRespHeader / ConfigKeyVal is NULL");
    return UPLL_RC_ERR_GENERIC;
  }

  if (!ctrlr_name) ctrlr_name = reinterpret_cast<char *>(ikey->get_user_data());

  uint32_t dt_type = req->datatype;
  uint32_t op = req->operation;
  uint32_t opt1 = req->option1;
  uint32_t opt2 = req->option2;

  if (op == UNC_OP_CREATE) {  // C, I
    if (dt_type == UPLL_DT_CANDIDATE || dt_type == UPLL_DT_IMPORT) {
      ret_val = ValIpRouteAttributeSupportCheck(ctrlr_name, ikey, op);
      if (ret_val != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG(
            "IP_ROUTE struct Capa check failure for create operation");
        return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
      }
      return UPLL_RC_SUCCESS;
    } else {
      UPLL_LOG_DEBUG("Invalid data type");
      return UPLL_RC_ERR_GENERIC;
    }
  } else if (op == UNC_OP_UPDATE) {  // C
    if (dt_type == UPLL_DT_CANDIDATE) {
      ret_val = ValIpRouteAttributeSupportCheck(ctrlr_name, ikey, op);
      if (ret_val != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("In IP_ROUTE Capa check failure for Update operation");
        return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
      }
      return UPLL_RC_SUCCESS;
    } else {
      UPLL_LOG_DEBUG("Unsupported data type - (%d)", dt_type);
      return UPLL_RC_ERR_GENERIC;
    }
  } else if (op == UNC_OP_READ || op == UNC_OP_READ_SIBLING
      || op == UNC_OP_READ_SIBLING_BEGIN || op == UNC_OP_READ_SIBLING_COUNT) {
    if (dt_type == UPLL_DT_CANDIDATE || dt_type == UPLL_DT_RUNNING
        || dt_type == UPLL_DT_STARTUP || dt_type == UPLL_DT_STATE) {
      if (opt1 != UNC_OPT1_NORMAL) {
        UPLL_LOG_DEBUG("Error option1 is not NORMAL");
        return UPLL_RC_ERR_INVALID_OPTION1;
      }
      if (opt2 != UNC_OPT2_NONE) {
        UPLL_LOG_DEBUG("option2 is not NONE");
        return UPLL_RC_ERR_INVALID_OPTION2;
      }
      if (NULL == ikey->get_cfg_val()) {
        UPLL_LOG_DEBUG("ConfigVal struct is empty");
        return UPLL_RC_ERR_CFG_SYNTAX;
      }
      if (ikey->get_cfg_val()->get_val() == NULL) {
        UPLL_LOG_DEBUG("val_static_ip_route struct is an optional");
        return UPLL_RC_SUCCESS;
      }
      ret_val = ValIpRouteAttributeSupportCheck(ctrlr_name, ikey, op);
      if (ret_val != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("IP_ROUTE struct capa check failure for read operation");
        return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
      }
      return UPLL_RC_SUCCESS;
    } else {
      UPLL_LOG_DEBUG("Unsupported data type - (%d)", dt_type);
      return UPLL_RC_ERR_NO_SUCH_INSTANCE;
    }
  }
  UPLL_LOG_DEBUG("Unsupported operation - (%d)", op);
  return UPLL_RC_ERR_NO_SUCH_INSTANCE;
}

upll_rc_t IpRouteMoMgr::ValIpRouteAttributeSupportCheck(const char *ctrlr_name,
                                                        ConfigKeyVal *ikey,
                                                        uint32_t operation) {
  UPLL_FUNC_TRACE;
  bool result_code = false;
  uint32_t max_attrs = 0;
  uint32_t max_instance_count = 0;
  const uint8_t *attrs;

  switch (operation) {
    case UNC_OP_CREATE:
      result_code = GetCreateCapability(ctrlr_name, ikey->get_key_type(),
                                        &max_instance_count, &max_attrs,
                                        &attrs);
      if (result_code && cur_instance_count >= max_instance_count) {
        UPLL_LOG_DEBUG("[%s:%d:%s Instance count %d exceeds %d", __FILE__,
                      __LINE__, __FUNCTION__, cur_instance_count,
                      max_instance_count);
        return UPLL_RC_ERR_EXCEEDS_RESOURCE_LIMIT;
      }
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
      UPLL_LOG_DEBUG("INVALID OPERATION CODE - (%d)", operation);
      return UPLL_RC_ERR_GENERIC;
  }
  if (!result_code) {
    UPLL_LOG_DEBUG("key_type - %d is not supported by controller - %s",
                  ikey->get_key_type(), ctrlr_name);
    return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_KT;
  }
  if (NULL == ikey->get_cfg_val()) {
    UPLL_LOG_DEBUG("ConfigVal struct is empty");
    return UPLL_RC_ERR_CFG_SYNTAX;
  }
  if (ikey->get_cfg_val()->get_st_num() != IpctSt::kIpcStValStaticIpRoute) {
    UPLL_LOG_DEBUG(
        "value structure matching is invalid. st.num - %d",
        ikey->get_cfg_val()->get_st_num());
    return UPLL_RC_ERR_BAD_REQUEST;
  }
  val_static_ip_route *iproute_val =
      reinterpret_cast<val_static_ip_route *>(ikey->get_cfg_val()->get_val());
  if (iproute_val != NULL) {
    if ((iproute_val->valid[UPLL_IDX_GROUP_METRIC_SIR] == UNC_VF_VALID)
        || (iproute_val->valid[UPLL_IDX_GROUP_METRIC_SIR]
            == UNC_VF_VALID_NO_VALUE)) {
      if (attrs[unc::capa::static_ip_route::kCapGroupMetric] == 0) {
        iproute_val->valid[UPLL_IDX_GROUP_METRIC_SIR] = UNC_VF_NOT_SOPPORTED;
        UPLL_LOG_DEBUG("Vrt_if structure attr is not supported by ctrlr ");
        return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
      }
    }
    return UPLL_RC_SUCCESS;
  }
  UPLL_LOG_DEBUG("Error val_static_ip_route struct is NULL");
  return UPLL_RC_ERR_GENERIC;
}

upll_rc_t IpRouteMoMgr::CopyToConfigKey(ConfigKeyVal *&okey,
                                        ConfigKeyVal *ikey) {
  if (!ikey || !(ikey->get_key())) return UPLL_RC_ERR_GENERIC;

  upll_rc_t result_code = UPLL_RC_SUCCESS;

  key_rename_vnode_info *key_rename =
      reinterpret_cast<key_rename_vnode_info *>(ikey->get_key());
  key_static_ip_route * key_route =
      reinterpret_cast<key_static_ip_route *>(malloc(
              sizeof(key_static_ip_route)));
  if (!key_route)
    return UPLL_RC_ERR_GENERIC;
  memset(key_route , 0, sizeof(key_static_ip_route));
  if (!strlen(reinterpret_cast<char *>(key_rename->old_unc_vtn_name))) {
    free(key_route);
    return UPLL_RC_ERR_GENERIC;
  }
  uuu::upll_strncpy(key_route->vrt_key.vtn_key.vtn_name,
         key_rename->old_unc_vtn_name,
         (kMaxLenVtnName + 1));
  if (ikey->get_key_type() == UNC_KT_VROUTER) {
    if (!strlen(reinterpret_cast<char *>(key_rename->old_unc_vnode_name))) {
      free(key_route);
      return UPLL_RC_ERR_GENERIC;
    }
    uuu::upll_strncpy(key_route->vrt_key.vrouter_name,
           key_rename->old_unc_vnode_name, (kMaxLenVnodeName + 1));
  }
  okey = new ConfigKeyVal(UNC_KT_VRT_IPROUTE, IpctSt::kIpcStKeyStaticIpRoute,
                          key_route, NULL);
  if (!okey) {
    free(key_route);
    return UPLL_RC_ERR_GENERIC;
  }
  return result_code;
}

bool IpRouteMoMgr::FilterAttributes(void *&val1, void *val2, bool audit_status,
                                    unc_keytype_operation_t op) {
  UPLL_FUNC_TRACE;
  if (op != UNC_OP_CREATE)
    return CompareValidValue(val1, val2, audit_status);
  return false;
}

bool IpRouteMoMgr::CompareValidValue(void *&val1, void *val2, bool audit) {
  UPLL_FUNC_TRACE;
  bool invalid_attr = true;
  val_static_ip_route_t *val_iprte1 =
      reinterpret_cast<val_static_ip_route_t *>(val1);
  val_static_ip_route_t *val_iprte2 =
      reinterpret_cast<val_static_ip_route_t *>(val2);
  for (unsigned int loop = 0;
        loop < sizeof(val_iprte1->valid) / sizeof(uint8_t); ++loop) {
    if (UNC_VF_INVALID == val_iprte1->valid[loop]
        && UNC_VF_VALID == val_iprte2->valid[loop]) 
       val_iprte1->valid[loop] = UNC_VF_VALID_NO_VALUE;
  }
  if (UNC_VF_VALID == val_iprte1->valid[UPLL_IDX_GROUP_METRIC_SIR]
      && UNC_VF_VALID == val_iprte2->valid[UPLL_IDX_GROUP_METRIC_SIR]) {
    if (val_iprte1->group_metric == val_iprte2->group_metric)
      val_iprte1->valid[UPLL_IDX_GROUP_METRIC_SIR] = UNC_VF_INVALID;
  }
  for (unsigned int loop = 0;
       loop < (sizeof(val_iprte1->valid) / sizeof(val_iprte1->valid[0]));
      ++loop) {
    if ((UNC_VF_VALID == (uint8_t) val_iprte1->valid[loop]) ||
        (UNC_VF_VALID_NO_VALUE == (uint8_t) val_iprte1->valid[loop]))
        invalid_attr = false;
  }
  return invalid_attr;
}
upll_rc_t IpRouteMoMgr::IsReferenced(ConfigKeyVal *ikey,
                                     upll_keytype_datatype_t dt_type,
                                     DalDmlIntf *dmi) {
  return UPLL_RC_SUCCESS;
}

upll_rc_t IpRouteMoMgr::MergeValidate(unc_key_type_t keytype,
                                      const char *ctrlr_id, ConfigKeyVal *ikey,
                                      DalDmlIntf *dmi) {
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  DbSubOp dbop = { kOpReadMultiple, kOpMatchNone, kOpInOutNone };
  ConfigKeyVal *tkey = NULL;
  if (!ikey || !(ikey->get_key())) {
    UPLL_LOG_DEBUG(" Input is NULL");
    return UPLL_RC_ERR_GENERIC;
   }
  /*
   * Here getting FULL Key (VTN & IPROUTE Name )
   */
  result_code = ReadConfigDB(ikey, UPLL_DT_IMPORT, UNC_OP_READ, dbop, dmi,
                             MAINTBL);
  if (UPLL_RC_SUCCESS != result_code) return result_code;
  while (ikey) {
    tkey = ikey;
    /*Check the network host name should not be present same vtn */
    memset(reinterpret_cast<key_static_ip_route_t*>
    (tkey->get_key())->vrt_key.vrouter_name, 0, (kMaxLenVnodeName+1));
    reinterpret_cast<key_static_ip_route_t*>
        (tkey->get_key())->dst_addr.s_addr = 0;
    reinterpret_cast<key_static_ip_route_t*>
          (tkey->get_key())->dst_addr_prefixlen = 0;
    reinterpret_cast<key_static_ip_route_t*>
        (tkey->get_key())->next_hop_addr.s_addr = 0;
    result_code = ReadConfigDB(tkey, UPLL_DT_RUNNING, UNC_OP_READ, dbop, dmi,
                               MAINTBL);
    if (UPLL_RC_SUCCESS == result_code) {
      UPLL_LOG_DEBUG("Conflict in IpRoute %d", result_code);
      return UPLL_RC_ERR_MERGE_CONFLICT;
    }
    /* Any other DB error */
    if (UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
      UPLL_LOG_DEBUG(" ReadConfigDB Failed %d", result_code);
      return result_code;
    }
    ikey = ikey->get_next_cfg_key_val();
    if (tkey) {
      free(tkey->get_key());
      delete tkey;
      tkey = NULL;
    }
  }
  return result_code;
}

}  // namespace kt_momgr
}  // namespace upll
}  // namespace unc
