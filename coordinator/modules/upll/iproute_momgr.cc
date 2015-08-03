/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include "iproute_momgr.hh"
#define NUM_KEY_MAIN_TBL_ 5

namespace unc {
namespace upll {
namespace kt_momgr {

  uint16_t IpRouteMoMgr::kIpRouteNumChildKey = 3;

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
    { uudst::static_ip_route::kDbiNwmName2, CFG_VAL, offsetof(
        val_static_ip_route, nwm_name),
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
    { uudst::static_ip_route::kDbiValidNwmName2, CFG_META_VAL, offsetof(
        val_static_ip_route, valid[UPLL_IDX_NWM_NAME_SIR]),
      uud::kDalUint8, 1 },
    { uudst::static_ip_route::kDbiValidMetric, CFG_META_VAL, offsetof(
        val_static_ip_route, valid[UPLL_IDX_GROUP_METRIC_SIR]),
      uud::kDalUint8, 1 },
    { uudst::static_ip_route::kDbiCsNwmName2, CS_VAL, offsetof(
        val_static_ip_route, cs_attr[UPLL_IDX_NWM_NAME_SIR]),
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
  table = new Table *[ntable]();
  table[MAINTBL] = new Table(uudst::kDbiStaticIpRouteTbl, UNC_KT_VRT_IPROUTE,
                         ip_route_bind_info, IpctSt::kIpcStKeyStaticIpRoute,
                         IpctSt::kIpcStValStaticIpRoute,
                         uudst::static_ip_route::kDbiStaticIpRouteNumCols);
  table[RENAMETBL] = NULL;
  table[CTRLRTBL] = NULL;
  table[CONVERTTBL] = NULL;
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
  if (GetVal(ikey)) {
    result_code = PerformSemanticCheckForNWM(ikey, dmi, req->datatype);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_INFO("VRT_IP_ROUTE Network Monitor validation failed");
      if (req->datatype == UPLL_DT_CANDIDATE)
        return UPLL_RC_ERR_INVALID_NWMONGRP;
      else
        return UPLL_RC_ERR_CFG_SEMANTIC;
    }
  }
  return result_code;
}

bool IpRouteMoMgr::IsValidKey(void *key, uint64_t index,
                              MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  key_static_ip_route *iproute_key =
      reinterpret_cast<key_static_ip_route*>(key);
  uint32_t val = 0;
  upll_rc_t ret_val = UPLL_RC_SUCCESS;
  switch (index) {
    case uudst::static_ip_route::kDbiVtnName:
      ret_val = ValidateKey(
          reinterpret_cast<char *>(iproute_key->vrt_key.vtn_key.vtn_name),
          kMinLenVtnName, kMaxLenVtnName);
      if (ret_val != UPLL_RC_SUCCESS) {
        UPLL_LOG_TRACE("VTN Name is not valid(%d)", ret_val);
        return false;
      }
      break;
    case uudst::static_ip_route::kDbiVrouterName:
      ret_val = ValidateKey(
          reinterpret_cast<char *>(iproute_key->vrt_key.vrouter_name),
          kMinLenVnodeName, kMaxLenVnodeName);
      if (ret_val != UPLL_RC_SUCCESS) {
        UPLL_LOG_TRACE("VRouter Name is not valid(%d)", ret_val);
        return false;
      }
      break;
    case uudst::static_ip_route::kDbiDstIpAddr:
      val = iproute_key->dst_addr.s_addr;
      if (val == 0xffffffff) {
        UPLL_LOG_TRACE("Invalid destination Address");
        return false;
      }
      break;
    case uudst::static_ip_route::kDbiNextHopAddr:
      val = iproute_key->next_hop_addr.s_addr;
      if (val == 0xffffffff || val == 0x00000000) {
        UPLL_LOG_TRACE("Invalid Next Hop Addr Address");
        return false;
      }
      break;
    case uudst::static_ip_route::kDbiMask:
      if (!ValidateNumericRange(iproute_key->dst_addr_prefixlen,
                                (uint8_t) kMinIpRoutePrefix,
                                (uint8_t) kMaxIpRoutePrefix, true, true)) {
        UPLL_LOG_TRACE(
            "Numeric range check failure for iproute_key->dst_addr_prefixlen");
        return false;
      }
      break;
    default:
      UPLL_LOG_TRACE("Invalid Key Index");
      break;
  }
  return true;
}

upll_rc_t IpRouteMoMgr::GetChildConfigKey(ConfigKeyVal *&okey,
                                          ConfigKeyVal *parent_key) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  key_static_ip_route *vrt_ip_route = NULL;
  void *pkey;
  if (parent_key == NULL) {
    vrt_ip_route = reinterpret_cast<key_static_ip_route *>
        (ConfigKeyVal::Malloc(sizeof(key_static_ip_route)));
    if (okey) delete okey;
    okey = new ConfigKeyVal(UNC_KT_VRT_IPROUTE, IpctSt::kIpcStKeyStaticIpRoute,
                            vrt_ip_route, NULL);
    vrt_ip_route->dst_addr_prefixlen = INVALID_PREFIX_LENGTH;
    vrt_ip_route->dst_addr.s_addr = INVALID_DST_IP_ADDR;
    vrt_ip_route->next_hop_addr.s_addr = INVALID_NEXT_HOP_ADDR;
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
        (ConfigKeyVal::Malloc(sizeof(key_static_ip_route)));
    vrt_ip_route->dst_addr_prefixlen = INVALID_PREFIX_LENGTH;
    vrt_ip_route->dst_addr.s_addr = INVALID_DST_IP_ADDR;
    vrt_ip_route->next_hop_addr.s_addr = INVALID_NEXT_HOP_ADDR;
  }
  unc_key_type_t keytype = parent_key->get_key_type();
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
      vrt_ip_route->dst_addr.s_addr =
          reinterpret_cast<key_static_ip_route *>(pkey)->dst_addr.s_addr;
      vrt_ip_route->dst_addr_prefixlen =
          reinterpret_cast<key_static_ip_route *>(pkey)->dst_addr_prefixlen;
      vrt_ip_route->next_hop_addr.s_addr =
          reinterpret_cast<key_static_ip_route *>(pkey)->next_hop_addr.s_addr;
      break;
    default:
      if (!okey)
        free(vrt_ip_route);
      return UPLL_RC_ERR_GENERIC;
  }
  if (!okey)
    okey = new ConfigKeyVal(UNC_KT_VRT_IPROUTE, IpctSt::kIpcStKeyStaticIpRoute,
                            vrt_ip_route, NULL);
  else if (okey->get_key() != vrt_ip_route)
    okey->SetKey(IpctSt::kIpcStKeyStaticIpRoute, vrt_ip_route);
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
  key_vrt *vrt_key = reinterpret_cast<key_vrt *>
    (ConfigKeyVal::Malloc(sizeof(key_vrt)));
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
      val = ConfigKeyVal::Malloc(sizeof(val_static_ip_route));
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
      if (ival == NULL) {
        UPLL_LOG_DEBUG("NULL val structure");
        return UPLL_RC_ERR_GENERIC;
      }
      val_static_ip_route *vrt_val =
          reinterpret_cast<val_static_ip_route *>(ConfigKeyVal::Malloc(
          sizeof(val_static_ip_route)));
      memcpy(vrt_val, ival, sizeof(val_static_ip_route));
      tmp1 = new ConfigVal(IpctSt::kIpcStValStaticIpRoute, vrt_val);
    }
  };
  void *tkey = (req)->get_key();
  key_static_ip_route *ikey;
  if (tkey) {
    ikey = reinterpret_cast<key_static_ip_route *>(tkey);
  } else {
    delete tmp1;
    return UPLL_RC_ERR_GENERIC;
  }
  key_static_ip_route *vrt_key =
      reinterpret_cast<key_static_ip_route *>(ConfigKeyVal::Malloc(
      sizeof(key_static_ip_route)));
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
     static_cast<val_static_ip_route *>(GetVal(ikey));
  if (vrt_val == NULL) return UPLL_RC_ERR_GENERIC;
  val_static_ip_route *vrt_val2 =
      reinterpret_cast<val_static_ip_route *>(GetVal(upd_key));

  unc_keytype_configstatus_t cs_status =
      (driver_result == UPLL_RC_SUCCESS) ? UNC_CS_APPLIED
                                         : UNC_CS_NOT_APPLIED;
  UPLL_LOG_TRACE("%s", (ikey->ToStrAll()).c_str());
  if (op == UNC_OP_CREATE) {
    vrt_val->cs_row_status = cs_status;
  } else if (op == UNC_OP_UPDATE) {
    void *val_tmp = vrt_val;
    CompareValidValue(val_tmp, GetVal(upd_key), true);
    UPLL_LOG_TRACE("%s", (upd_key->ToStrAll()).c_str());
    vrt_val->cs_row_status = vrt_val2->cs_row_status;
  } else {
    return UPLL_RC_ERR_GENERIC;
  }
  for (unsigned int loop = 0;
       loop < (sizeof(vrt_val->valid) / sizeof(vrt_val->valid[0]));
       ++loop) {
    if ((UNC_VF_VALID == vrt_val->valid[loop])
          || (UNC_VF_VALID_NO_VALUE == vrt_val->valid[loop])) {
        vrt_val->cs_attr[loop] = cs_status;
    } else if ((UNC_VF_INVALID == vrt_val->valid[loop]) &&
               (UNC_OP_CREATE == op)) {
        vrt_val->cs_attr[loop] = UNC_CS_APPLIED;
    } else if ((UNC_VF_INVALID == vrt_val->valid[loop]) &&
               (UNC_OP_UPDATE == op)) {
        vrt_val->cs_attr[loop] = vrt_val2->cs_attr[loop];
    }
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t IpRouteMoMgr::UpdateAuditConfigStatus(
    unc_keytype_configstatus_t cs_status,
    uuc::UpdateCtrlrPhase phase,
    ConfigKeyVal *&ckv_running,
    DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  val_static_ip_route_t *val;
  val = (ckv_running != NULL) ?
      reinterpret_cast<val_static_ip_route_t *>(GetVal(ckv_running)) : NULL;
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
    if ((req->datatype == UPLL_DT_IMPORT) ||
        (req->datatype == UPLL_DT_AUDIT)) {
      ret_val = UPLL_RC_ERR_CFG_SYNTAX;
    }
    return ret_val;
  }
  if (op == UNC_OP_CREATE || op == UNC_OP_UPDATE || op == UNC_OP_DELETE) {
    if (dt_type == UPLL_DT_CANDIDATE || UPLL_DT_IMPORT == dt_type) {
      if (NULL == ikey->get_cfg_val()) {
        UPLL_LOG_DEBUG("ConfigVal struct is empty");
        if (UNC_OP_UPDATE == op) {
           return UPLL_RC_ERR_BAD_REQUEST;
        } else {
          return UPLL_RC_SUCCESS;
        }
      }
      if (ikey->get_cfg_val()->get_st_num() != IpctSt::kIpcStValStaticIpRoute) {
        UPLL_LOG_DEBUG(
            "value structure matching is invalid. st.num - %d",
            ikey->get_cfg_val()->get_st_num());
        return UPLL_RC_ERR_BAD_REQUEST;
      }
      if (op != UNC_OP_DELETE) {
        val_static_ip_route *iproute_val =
         reinterpret_cast<val_static_ip_route*>(ikey->get_cfg_val()->get_val());
        if (iproute_val == NULL) {
          UPLL_LOG_DEBUG("val struct is mandatory for create and update op");
          return UPLL_RC_ERR_BAD_REQUEST;
        }
        ret_val = ValidateIpRouteValue(iproute_val, op);
        if (ret_val != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("Syntax check failure for val_static_ip_route");
          return UPLL_RC_ERR_CFG_SYNTAX;
        }
      }
      return UPLL_RC_SUCCESS;
    } else {
      UPLL_LOG_DEBUG("Unsupported data type - (%d)", dt_type);
      return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT;
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
        UPLL_LOG_DEBUG("Syntax check failure for val_static_ip_route");
        return UPLL_RC_ERR_CFG_SYNTAX;
      }
      return UPLL_RC_SUCCESS;
    } else {
      UPLL_LOG_DEBUG("Unsupported data type - (%d)", dt_type);
      return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT;
    }
  } else if (op == UNC_OP_READ_NEXT || op == UNC_OP_READ_BULK) {
    if (dt_type == UPLL_DT_CANDIDATE || dt_type == UPLL_DT_RUNNING
        || dt_type == UPLL_DT_STARTUP) {
      UPLL_LOG_DEBUG("Value structure is none for operation type:%d", op);
      return UPLL_RC_SUCCESS;
    } else {
      UPLL_LOG_DEBUG("Unsupported data type - (%d)", dt_type);
      return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT;
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

  if ((operation == UNC_OP_READ_SIBLING_BEGIN) ||
      (operation == UNC_OP_READ_SIBLING_COUNT)) {
    // Poisoning child keys for this operation
    UPLL_LOG_TRACE("Operation is %d", operation);
    iproute_key->dst_addr_prefixlen = INVALID_PREFIX_LENGTH;
    iproute_key->dst_addr.s_addr = INVALID_DST_IP_ADDR;
    iproute_key->next_hop_addr.s_addr = INVALID_NEXT_HOP_ADDR;
    return UPLL_RC_SUCCESS;
  }

  // TODO(s): Need to validate if inputs are given
  if (!ValidateNumericRange(iproute_key->dst_addr_prefixlen,
                          (uint8_t) kMinIpRoutePrefix,
                          (uint8_t) kMaxIpRoutePrefix,
                          true, true)) {
    UPLL_LOG_DEBUG("Numeric range check failed. dst_addr_prefixlen - %d",
                  iproute_key->dst_addr_prefixlen);
    return UPLL_RC_ERR_INVALID_DEST_ADDR;
  }
  if (iproute_key->dst_addr.s_addr == 0xffffffff) {
    UPLL_LOG_DEBUG("Bad destination address. addr:0x%08x",
                  iproute_key->dst_addr.s_addr);
    return UPLL_RC_ERR_INVALID_DEST_ADDR;
  }
  if (iproute_key->next_hop_addr.s_addr == 0x00000000 ||
      iproute_key->next_hop_addr.s_addr == 0xffffffff) {
    UPLL_LOG_DEBUG("Bad next hop address. addr:0x%08x",
                  iproute_key->next_hop_addr.s_addr);
    return UPLL_RC_ERR_INVALID_NEXTHOP_ADDR;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t IpRouteMoMgr::ValidateIpRouteValue(val_static_ip_route *iproute_val,
                                             uint32_t op) {
  UPLL_FUNC_TRACE;
  bool ret_val = false;

  // Attribute syntax validation
  for (unsigned int valid_index = 0;
     valid_index < sizeof(iproute_val->valid) / sizeof(iproute_val->valid[0]);
     valid_index++) {
    if (iproute_val->valid[valid_index] == UNC_VF_VALID) {
      switch (valid_index) {
        case UPLL_IDX_NWM_NAME_SIR:
          ret_val = ValidateString(iproute_val->nwm_name,
                                   kMinLenNwmName, kMaxLenNwmName);
          break;
        case UPLL_IDX_GROUP_METRIC_SIR:
          ret_val = ValidateNumericRange(iproute_val->group_metric,
                          (uint16_t) kMinLenGroupMetric,
                          (uint16_t) kMaxLenGroupMetric, true, true);
          break;
        default:
          UPLL_LOG_ERROR("Invalid index %d", valid_index);
          return UPLL_RC_ERR_GENERIC;
      }
      if (!ret_val) {
        UPLL_LOG_DEBUG("value syntax check failed");
        return UPLL_RC_ERR_CFG_SYNTAX;
      }
    }
  }
  // reset data if valid flags are invalid or valid_no_value
  for (unsigned int valid_index = 0;
     valid_index < sizeof(iproute_val->valid) / sizeof(iproute_val->valid[0]);
     valid_index++) {
    if ((iproute_val->valid[valid_index] == UNC_VF_INVALID) ||
        (iproute_val->valid[valid_index] == UNC_VF_VALID_NO_VALUE)) {
      switch (valid_index) {
        case UPLL_IDX_NWM_NAME_SIR:
          StringReset(iproute_val->nwm_name);
          break;
        case UPLL_IDX_GROUP_METRIC_SIR:
          iproute_val->group_metric = 0;
          break;
        default:
          UPLL_LOG_ERROR("Never here");
          return UPLL_RC_ERR_GENERIC;
      }
    }
  }
  if (op == UNC_OP_CREATE) {
    if (((iproute_val->valid[UPLL_IDX_GROUP_METRIC_SIR] == UNC_VF_VALID) &&
        (iproute_val->valid[UPLL_IDX_NWM_NAME_SIR] == UNC_VF_INVALID)) ||
        (iproute_val->valid[UPLL_IDX_NWM_NAME_SIR] == UNC_VF_VALID_NO_VALUE) ||
        (iproute_val->valid[UPLL_IDX_GROUP_METRIC_SIR] ==
         UNC_VF_VALID_NO_VALUE)) {
      UPLL_LOG_ERROR("Invalid valid flags combination for ip_route create");
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t IpRouteMoMgr::ValidateCapability(IpcReqRespHeader *req,
                                           ConfigKeyVal *ikey,
                                           const char *ctrlr_name) {
  UPLL_FUNC_TRACE;
  upll_rc_t ret_val = UPLL_RC_ERR_GENERIC;
  if (!req || !ikey) {
    UPLL_LOG_DEBUG("IpcReqRespHeader / ConfigKeyVal is NULL");
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
  uint32_t max_attrs = 0;
  uint32_t max_instance_count = 0;
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
      UPLL_LOG_DEBUG("INVALID OPERATION CODE - (%d)", req->operation);
      return UPLL_RC_ERR_GENERIC;
  }
  if (!result_code) {
    UPLL_LOG_DEBUG("key_type - %d is not supported by controller - %s",
                  ikey->get_key_type(), ctrlr_name);
    return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
  }
  val_static_ip_route *iproute_val = NULL;
  if ((ikey->get_cfg_val()) && (ikey->get_cfg_val()->get_st_num() ==
       IpctSt::kIpcStValStaticIpRoute)) {
    iproute_val =
        reinterpret_cast<val_static_ip_route*>(ikey->get_cfg_val()->get_val());
  }
  if (iproute_val) {
    if (max_attrs > 0) {
      ret_val = ValIpRouteAttributeSupportCheck(iproute_val, attrs,
                                                req->operation);
      return ret_val;
    } else {
      UPLL_LOG_DEBUG("Error- Mandatory value struct is NULL for READ");
      return UPLL_RC_ERR_GENERIC;
    }
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t IpRouteMoMgr::ValIpRouteAttributeSupportCheck(
    val_static_ip_route *iproute_val,
    const uint8_t *attrs,
    unc_keytype_operation_t operation) {
  UPLL_FUNC_TRACE;
  if (iproute_val != NULL) {
    if ((iproute_val->valid[UPLL_IDX_NWM_NAME_SIR] == UNC_VF_VALID)
        || (iproute_val->valid[UPLL_IDX_NWM_NAME_SIR]
            == UNC_VF_VALID_NO_VALUE)) {
      if (attrs[unc::capa::static_ip_route::kCapNwmName] == 0) {
        iproute_val->valid[UPLL_IDX_NWM_NAME_SIR] = UNC_VF_INVALID;
        if (operation == UNC_OP_CREATE || operation == UNC_OP_UPDATE) {
          UPLL_LOG_DEBUG("Nwm name attribute is not supported by ctrlr");
          return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
        }
      }
    }
    if ((iproute_val->valid[UPLL_IDX_GROUP_METRIC_SIR] == UNC_VF_VALID)
        || (iproute_val->valid[UPLL_IDX_GROUP_METRIC_SIR]
            == UNC_VF_VALID_NO_VALUE)) {
      if (attrs[unc::capa::static_ip_route::kCapGroupMetric] == 0) {
        iproute_val->valid[UPLL_IDX_GROUP_METRIC_SIR] = UNC_VF_INVALID;
        if (operation == UNC_OP_CREATE || operation == UNC_OP_UPDATE) {
          UPLL_LOG_DEBUG("Metric attr is not supported by ctrlr ");
          return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
        }
      }
    }
  } else {
    UPLL_LOG_DEBUG("Error val_static_ip_route struct is NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t IpRouteMoMgr::CopyToConfigKey(ConfigKeyVal *&okey,
                                        ConfigKeyVal *ikey) {
  if (!ikey || !(ikey->get_key())) return UPLL_RC_ERR_GENERIC;

  upll_rc_t result_code = UPLL_RC_SUCCESS;

  key_rename_vnode_info *key_rename =
      reinterpret_cast<key_rename_vnode_info *>(ikey->get_key());
  key_static_ip_route * key_route =
      reinterpret_cast<key_static_ip_route *>(ConfigKeyVal::Malloc(
              sizeof(key_static_ip_route)));
  key_route->dst_addr_prefixlen = INVALID_PREFIX_LENGTH;
  key_route->dst_addr.s_addr = INVALID_DST_IP_ADDR;
  key_route->next_hop_addr.s_addr = INVALID_NEXT_HOP_ADDR;
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
  } else {
    if (!strlen(reinterpret_cast<char *>(key_rename->new_unc_vnode_name))) {
      FREE_IF_NOT_NULL(key_route);
      return UPLL_RC_ERR_GENERIC;
    }
    uuu::upll_strncpy(key_route->vrt_key.vrouter_name,
       key_rename->new_unc_vnode_name, (kMaxLenVnodeName+1));
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

bool IpRouteMoMgr::CompareValidValue(void *&val1, void *val2,
                                     bool copy_to_running) {
  UPLL_FUNC_TRACE;
  bool invalid_attr = true;
  val_static_ip_route_t *val_iprte1 =
      reinterpret_cast<val_static_ip_route_t *>(val1);
  val_static_ip_route_t *val_iprte2 =
      reinterpret_cast<val_static_ip_route_t *>(val2);
  if (!val_iprte1 || !val_iprte2) {
    UPLL_LOG_ERROR("Invalid param");
    return false;
  }
  for (unsigned int loop = 0;
       loop < sizeof(val_iprte1->valid) / sizeof(uint8_t); ++loop) {
    if (UNC_VF_INVALID == val_iprte1->valid[loop]
        && UNC_VF_VALID == val_iprte2->valid[loop])
       val_iprte1->valid[loop] = UNC_VF_VALID_NO_VALUE;
  }
  if (UNC_VF_VALID == val_iprte1->valid[UPLL_IDX_NWM_NAME_SIR]
      && UNC_VF_VALID == val_iprte2->valid[UPLL_IDX_NWM_NAME_SIR]) {
    if (!(uuu::upll_strncmp(val_iprte1->nwm_name, val_iprte2->nwm_name,
                            kMaxLenNwmName+1))) {
      val_iprte1->valid[UPLL_IDX_NWM_NAME_SIR] =
        (copy_to_running)?UNC_VF_INVALID:UNC_VF_VALUE_NOT_MODIFIED;
    }
  }
  if (UNC_VF_VALID == val_iprte1->valid[UPLL_IDX_GROUP_METRIC_SIR]
      && UNC_VF_VALID == val_iprte2->valid[UPLL_IDX_GROUP_METRIC_SIR]) {
    if (val_iprte1->group_metric == val_iprte2->group_metric)
      val_iprte1->valid[UPLL_IDX_GROUP_METRIC_SIR] =
        (copy_to_running)?UNC_VF_INVALID:UNC_VF_VALUE_NOT_MODIFIED;
  }
  for (unsigned int loop = 0;
       loop < (sizeof(val_iprte1->valid) / sizeof(val_iprte1->valid[0]));
      ++loop) {
    if ((UNC_VF_VALID == (uint8_t) val_iprte1->valid[loop]) ||
        (UNC_VF_VALID_NO_VALUE == (uint8_t) val_iprte1->valid[loop])) {
      invalid_attr = false;
      break;
    }
  }
  return invalid_attr;
}

upll_rc_t IpRouteMoMgr::IsReferenced(ConfigKeyVal *ikey,
                                     upll_keytype_datatype_t dt_type,
                                     DalDmlIntf *dmi) {
  return UPLL_RC_SUCCESS;
}



upll_rc_t IpRouteMoMgr::MergeValidate(unc_key_type_t keytype,
                                      const char *ctrlr_id,
                                      ConfigKeyVal *ikey,
                                      DalDmlIntf *dmi) {
  return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT;
  /* NEC changed the design so the below code is not required
   */
  #if 0
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  DbSubOp dbop = { kOpReadMultiple, kOpMatchNone, kOpInOutNone };
  ConfigKeyVal *tkey = NULL;
  ConfigKeyVal * travel_key = NULL;
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
  travel_key = ikey;
  while (travel_key) {
    result_code = GetChildConfigKey(tkey, travel_key);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("GetChildConfigKey Failed");
      DELETE_IF_NOT_NULL(tkey);
      return result_code;
    }
    /*Check the network host name should not be present same vtn */
    memset(reinterpret_cast<key_static_ip_route_t*>
    (tkey->get_key())->vrt_key.vrouter_name, 0, (kMaxLenVnodeName+1));
    reinterpret_cast<key_static_ip_route_t*>
        (tkey->get_key())->dst_addr.s_addr = 0;
    reinterpret_cast<key_static_ip_route_t*>
          (tkey->get_key())->dst_addr_prefixlen = 0;
    reinterpret_cast<key_static_ip_route_t*>
        (tkey->get_key())->next_hop_addr.s_addr = 0;
    result_code = ReadConfigDB(tkey, UPLL_DT_CANDIDATE, UNC_OP_READ, dbop, dmi,
                               MAINTBL);
    if (UPLL_RC_SUCCESS == result_code) {
      UPLL_LOG_DEBUG("Conflict in IpRoute %d", result_code);
      delete ikey;
      ikey = NULL;
      ikey->ResetWith(tkey);
      delete tkey;
      return UPLL_RC_ERR_MERGE_CONFLICT;
    }
    /* Any other DB error */
    if (UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
      UPLL_LOG_DEBUG(" ReadConfigDB Failed %d", result_code);
      delete tkey;
      return result_code;
    }
    travel_key = travel_key->get_next_cfg_key_val();
  }
  /* ikey is deleting in the vtn file. If memory leak tool
   * shows any error don't fix it.
   */
  return result_code;
  #endif
}

// Overridden Read Sibling from momgr_impl.
// This keytype contains 3 child keys and needs special handling.
upll_rc_t IpRouteMoMgr::ReadSiblingMo(IpcReqRespHeader *header,
                                   ConfigKeyVal *ikey,
                                   bool begin,
                                   DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_ERR_GENERIC;
  uint32_t result_count = 0;

  if (begin == true) {
    return (MoMgrImpl::ReadSiblingMo(header, ikey, begin, dmi));
  }
  if ((header->operation != UNC_OP_READ_SIBLING_BEGIN) &&
      (header->operation != UNC_OP_READ_SIBLING)) {
    UPLL_LOG_DEBUG("Operation type is not Sibling begin/Sibling");
    return UPLL_RC_ERR_GENERIC;
  }
  if (ikey == NULL || dmi == NULL) {
    UPLL_LOG_DEBUG("Null Param ikey/dmi");
    return UPLL_RC_ERR_GENERIC;
  }
  result_code = ValidateMessage(header, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("ValidateMessage failed result_code %d",
                   result_code);
    return result_code;
  }

  MoMgrTables tbl = MAINTBL;
  DbSubOp dbop = {kOpReadMultiple, kOpMatchNone,
                     kOpInOutCtrlr | kOpInOutDomain | kOpInOutFlag};

  if (UPLL_DT_RUNNING == header->datatype) {
    UPLL_LOG_TRACE("Status Flag enabled");
    dbop.inoutop |= kOpInOutCs;
  }
  switch (header->datatype) {
    case UPLL_DT_IMPORT:
      return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT;
    case UPLL_DT_CANDIDATE:
    case UPLL_DT_RUNNING:
    case UPLL_DT_STARTUP:
    case UPLL_DT_STATE:
    {
      // To read the record(s) from DB
      upll_keytype_datatype_t dt_type = (header->datatype == UPLL_DT_STATE) ?
                                        UPLL_DT_RUNNING : header->datatype;
      ConfigVal *cval = ikey->get_cfg_val();
      ConfigVal *new_cval = NULL;
      AllocVal(new_cval, dt_type, MAINTBL);
      if (new_cval == NULL) {
        UPLL_LOG_DEBUG("Null new_cval after AllocVal");
        return UPLL_RC_ERR_GENERIC;
      }

      if (cval != NULL && cval->get_val() != NULL) {
        memcpy(new_cval->get_val(), cval->get_val(),
               sizeof(val_static_ip_route));
      }
      ikey->SetCfgVal(new_cval);

      // Used in ReadConfigDB in each Iteration
      ConfigKeyVal *queryckval = NULL;
      result_code = DupConfigKeyVal(queryckval, ikey, MAINTBL);
      if (result_code != UPLL_RC_SUCCESS || queryckval == NULL) {
        UPLL_LOG_DEBUG("Error while DupConfigKeyVal");
        return UPLL_RC_ERR_GENERIC;
      }

      // uint16_t kIpRouteNumChildKey = 3;
      bool fetched_from_query = false;
      for (uint16_t childKeyIndex = uuds::TableNumPkCols(
               uudst::kDbiStaticIpRouteTbl) - 1;
           childKeyIndex >= uuds::TableNumPkCols(
               uudst::kDbiStaticIpRouteTbl) - kIpRouteNumChildKey;
           childKeyIndex--) {
        if (IsValidKey(queryckval->get_key(), childKeyIndex) == false)
          continue;
        uint32_t tmp_count = header->rep_count;
        result_code = ReadConfigDB(queryckval, dt_type, header->operation,
                                   dbop, tmp_count, dmi, tbl);

        if (result_code == UPLL_RC_SUCCESS) {
          ConfigKeyVal *qtmpckval = queryckval;
          ConfigKeyVal *tmpckval = NULL;
          ConfigKeyVal *appendckval = NULL;
          // Iterating through all ConfigKeyVal from the result set
          // and storing the result set in ikey
          while (qtmpckval != NULL) {
            if (result_count == header->rep_count) {
              UPLL_LOG_TRACE("Fetched (%d) records", result_count);
              delete queryckval;
              return UPLL_RC_SUCCESS;
            }
            result_code = DupConfigKeyVal(tmpckval, qtmpckval, MAINTBL);
            if (result_code != UPLL_RC_SUCCESS) {
              UPLL_LOG_DEBUG("Error while DupConfigKeyVal");
              if (queryckval)
                delete queryckval;
              if (tmpckval)
                delete tmpckval;
              return UPLL_RC_ERR_GENERIC;
            }
            UPLL_LOG_TRACE("Appending ConfigKeyVal");
            appendckval = tmpckval;
            if (fetched_from_query == false) {
              ikey->ResetWith(appendckval);
              DELETE_IF_NOT_NULL(appendckval);
              result_count++;
              fetched_from_query = true;
            } else {
              ikey->AppendCfgKeyVal(appendckval);
              result_count++;
            }
            tmpckval = NULL;
            qtmpckval = qtmpckval->get_next_cfg_key_val();
          }  // while (qtmpckval != NULL)
#if 0
          // Copying the last available key structure for next iteration
          if (appendckval != NULL && appendckval->get_key() != NULL) {
            memcpy(queryckval->get_key(), appendckval->get_key(),
                   sizeof(key_static_ip_route));
          } else {
            memcpy(queryckval->get_key(), ikey->get_key(),
                   sizeof(key_static_ip_route));
          }
#endif
        } else if (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
          UPLL_LOG_DEBUG("Error while ReadConfigDB");
          delete queryckval;
          return result_code;
        }

        // Preparing Child Key data for next Sibling Iteration
        key_static_ip_route *key_ipr =
            reinterpret_cast<key_static_ip_route *>(queryckval->get_key());
        if (ResetDataForSibling(key_ipr,
             (uudst::static_ip_route::kStaticIpRouteIndex)childKeyIndex)
           == false) {
          UPLL_LOG_DEBUG("Data Not Reset for the index(%d)", childKeyIndex);
          delete queryckval;
          return UPLL_RC_ERR_GENERIC;
        }
        if (GetVal(ikey)) {
          memcpy(queryckval->get_cfg_val()->get_val(),
                 ikey->get_cfg_val()->get_val(),
                 sizeof(val_static_ip_route));
        } else {
          queryckval->SetCfgVal(NULL);
        }
        UPLL_LOG_TRACE("Next Query After Reset: %s",
                       (queryckval->ToStrAll()).c_str());
      }  // for
      header->rep_count = result_count;
      delete queryckval;
      break;
    }  // case
    default:
      break;
  }

  if (header->rep_count > 0) {
    result_code = (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE)
                  ? UPLL_RC_SUCCESS : result_code;
    if (result_code == UPLL_RC_SUCCESS) {
      result_code = AdaptValToVtnService(ikey);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("AdaptValToVtnService failed result_code %d",
                    result_code);
        return result_code;
      }
    }
  } else {
    return UPLL_RC_ERR_NO_SUCH_INSTANCE;
  }
  return result_code;
}  // IpRouteMoMgr::ReadSiblingMo

// To reset the data available in index to empty
// Currently implemented only for child index
bool IpRouteMoMgr::ResetDataForSibling(key_static_ip_route *key_ipr,
    uudst::static_ip_route::kStaticIpRouteIndex index) {
  switch (index) {
    case uudst::static_ip_route::kDbiDstIpAddr:
      key_ipr->dst_addr.s_addr = INVALID_DST_IP_ADDR;
    case uudst::static_ip_route::kDbiMask:
      key_ipr->dst_addr_prefixlen = INVALID_PREFIX_LENGTH;
    case uudst::static_ip_route::kDbiNextHopAddr:
      key_ipr->next_hop_addr.s_addr = INVALID_NEXT_HOP_ADDR;
      break;
    default:
      UPLL_LOG_DEBUG("Not a child key index");
      return false;
  }
  UPLL_LOG_TRACE("Resetting Data for index(%d)", index);
  return true;
}

// implementation of valid flags syntax check during update
upll_rc_t IpRouteMoMgr::ValidateUpdateMo(ConfigKeyVal *ikey,
                                         ConfigKeyVal *db_ckv) {
  UPLL_FUNC_TRACE;
  if (!(GetVal(db_ckv)) || !(GetVal(ikey))) {
    UPLL_LOG_ERROR("NULL input val structures");
    return UPLL_RC_ERR_GENERIC;
  }
  val_static_ip_route_t *val_ipr_db =
                reinterpret_cast<val_static_ip_route_t *>(GetVal(db_ckv));
  val_static_ip_route_t *val_ipr_ikey =
                reinterpret_cast<val_static_ip_route_t *>(GetVal(ikey));
  // if nwm_name is valid_no_value then group_metric need to be unconfigured
  if (val_ipr_ikey->valid[UPLL_IDX_NWM_NAME_SIR] == UNC_VF_VALID_NO_VALUE) {
  // StringReset(val_ipr_ikey->nwm_name);
  // already reseted data in ValidateIpRouteValue
      val_ipr_ikey->group_metric = 0;
      val_ipr_ikey->valid[UPLL_IDX_GROUP_METRIC_SIR] = UNC_VF_VALID_NO_VALUE;
  }
  if (val_ipr_ikey->valid[UPLL_IDX_NWM_NAME_SIR] == UNC_VF_INVALID) {
    if ((val_ipr_ikey->valid[UPLL_IDX_GROUP_METRIC_SIR] == UNC_VF_VALID) &&
        (val_ipr_db->valid[UPLL_IDX_NWM_NAME_SIR] == UNC_VF_INVALID)) {
      UPLL_LOG_ERROR("group metric is VALID and nwm_name is not configured"
                     " in DB during update");
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
    if ((val_ipr_ikey->valid[UPLL_IDX_GROUP_METRIC_SIR] ==
         UNC_VF_VALID_NO_VALUE) &&
        (val_ipr_db->valid[UPLL_IDX_NWM_NAME_SIR] == UNC_VF_INVALID)) {
      UPLL_LOG_ERROR("group metric is VALID_NO_VALUE and nwm_name is not"
                     " configured in DB during update");
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  }
  return UPLL_RC_SUCCESS;
}

}  // namespace kt_momgr
}  // namespace upll
}  // namespace unc
