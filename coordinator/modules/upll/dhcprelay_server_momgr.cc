/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include "dhcprelay_server_momgr.hh"
#define NUM_KEY_MAIN_TBL_ 6
#if 0
namespace upll_dal_vbrif unc::upll::dal::schema::table::vbridge_interface;
#endif

namespace unc {
namespace upll {
namespace kt_momgr {

BindInfo DhcpRelayServerMoMgr::dhcprelay_server_bind_info[] = {
    { uudst::dhcprelay_server::kDbiVtnName, CFG_KEY, offsetof(
        key_dhcp_relay_server, vrt_key.vtn_key.vtn_name),
      uud::kDalChar, 32 },
    { uudst::dhcprelay_server::kDbiVrouterName, CFG_KEY, offsetof(
        key_dhcp_relay_server, vrt_key.vrouter_name),
      uud::kDalChar, 32 },
    { uudst::dhcprelay_server::kDbiServerIpAddr, CFG_KEY, offsetof(
        key_dhcp_relay_server, server_addr),
      uud::kDalUint32, 1 },
    { uudst::dhcprelay_server::kDbiCtrlrName, CK_VAL, offsetof(key_user_data,
                                                               ctrlr_id),
      uud::kDalChar, 32 },
    { uudst::dhcprelay_server::kDbiDomainId, CK_VAL, offsetof(key_user_data,
                                                              domain_id),
      uud::kDalChar, 32 },
    { uudst::dhcprelay_server::kDbiCsRowstatus, CS_VAL, offsetof(
        val_dhcp_relay_server, cs_row_status),
      uud::kDalUint8, 1 },
    { uudst::dhcprelay_server::kDbiFlags, CK_VAL, offsetof(key_user_data,
                                                           flags),
      uud::kDalUint8, 1 }, };

BindInfo DhcpRelayServerMoMgr::dhcprealy_server_maintbl_key_update_bind_info[] =
    { { uudst::dhcprelay_server::kDbiVtnName, CFG_MATCH_KEY, offsetof(
        key_dhcp_relay_server, vrt_key.vtn_key.vtn_name),
        uud::kDalChar, kMaxLenVtnName + 1 },
      { uudst::dhcprelay_server::kDbiVrouterName, CFG_MATCH_KEY, offsetof(
          key_dhcp_relay_server, vrt_key.vrouter_name),
        uud::kDalChar, kMaxLenVnodeName + 1 },
      { uudst::dhcprelay_server::kDbiServerIpAddr, CFG_MATCH_KEY, offsetof(
          key_dhcp_relay_server, server_addr),
        uud::kDalUint32, 1 },
      { uudst::dhcprelay_server::kDbiVtnName, CFG_INPUT_KEY, offsetof(
          key_rename_vnode_info_t, new_unc_vtn_name),
        uud::kDalChar, kMaxLenVtnName + 1 },
      { uudst::dhcprelay_server::kDbiVrouterName, CFG_INPUT_KEY, offsetof(
          key_rename_vnode_info_t, new_unc_vnode_name),
        uud::kDalChar, kMaxLenVnodeName + 1 },
      { uudst::dhcprelay_server::kDbiFlags, CK_VAL, offsetof(
          key_user_data_t, flags),
        uud::kDalUint8, 1 } };

DhcpRelayServerMoMgr::DhcpRelayServerMoMgr() {
  UPLL_FUNC_TRACE
  ntable = MAX_MOMGR_TBLS;
  table = new Table *[ntable]();
  table[MAINTBL]= new Table(uudst::kDbiDhcpRelayServerTbl,
      UNC_KT_DHCPRELAY_SERVER, dhcprelay_server_bind_info,
      IpctSt::kIpcStKeyDhcpRelayServer, IpctSt::kIpcStValDhcpRelayServer,
      uudst::dhcprelay_server::kDbiDhcpRelayServerNumCols);
  table[RENAMETBL] = NULL;
  table[CTRLRTBL] = NULL;
  table[CONVERTTBL] = NULL;
  nchild = 0;
  child = NULL;
}

/*
 * Based on the key type the bind info will pass
 **/

bool DhcpRelayServerMoMgr::GetRenameKeyBindInfo(unc_key_type_t key_type,
                                                BindInfo *&binfo, int &nattr,
                                                MoMgrTables tbl) {
  /* Main Table or rename table only update */
  if (MAINTBL == tbl) {
    nattr = NUM_KEY_MAIN_TBL_;
    binfo = dhcprealy_server_maintbl_key_update_bind_info;
  } else {
    UPLL_LOG_DEBUG("Invalide Table");
    return PFC_FALSE;
  }
  return PFC_TRUE;
}

upll_rc_t DhcpRelayServerMoMgr::ValidateAttribute(ConfigKeyVal *ikey,
                                                  DalDmlIntf *dmi,
                                                  IpcReqRespHeader *req) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  if (ikey->get_key_type() != UNC_KT_DHCPRELAY_SERVER) result_code =
      UPLL_RC_ERR_CFG_SYNTAX;

  #if 0
  /* Check if vrouter admin status is disabled */
  result_code = IsAdminStatusEnable(ikey, dmi);
  if (result_code != UPLL_RC_SUCCESS) {
    return UPLL_RC_ERR_CFG_SEMANTIC;
  }
  #endif
  return result_code;
}


bool DhcpRelayServerMoMgr::IsValidKey(void *key, uint64_t index,
                                      MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  key_dhcp_relay_server *dhcp_rs_key =
     reinterpret_cast<key_dhcp_relay_server *>(key);
  uint32_t val = 0;
  upll_rc_t ret_val = UPLL_RC_SUCCESS;
  switch (index) {
    case uudst::dhcprelay_server::kDbiVtnName:
      ret_val = ValidateKey(reinterpret_cast<char *>
                            (dhcp_rs_key->vrt_key.vtn_key.vtn_name),
                            kMinLenVtnName, kMaxLenVtnName);
      if (ret_val != UPLL_RC_SUCCESS) {
        UPLL_LOG_TRACE("VTN Name is not valid(%d)", ret_val);
        return false;
      }
      break;
    case uudst::dhcprelay_server::kDbiVrouterName:
      ret_val = ValidateKey(reinterpret_cast<char *>
                            (dhcp_rs_key->vrt_key.vrouter_name),
                            kMinLenVnodeName, kMaxLenVnodeName);
      if (ret_val != UPLL_RC_SUCCESS) {
        UPLL_LOG_TRACE("VRouter Name is not valid(%d)", ret_val);
        return false;
      }
      break;
    case uudst::dhcprelay_server::kDbiServerIpAddr:
      val = dhcp_rs_key->server_addr.s_addr;
      if (val == 0xffffffff || val == 0x00000000) {
        UPLL_LOG_TRACE("Invalid Server Address /n");
        return false;
      }
      break;
    default:
      UPLL_LOG_TRACE("Invalid Key Index");
      break;
  }
  return true;
}

upll_rc_t DhcpRelayServerMoMgr::GetChildConfigKey(ConfigKeyVal *&okey,
                                                  ConfigKeyVal *parent_key) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  key_dhcp_relay_server *dhcp_key;
  void *pkey;
  if (parent_key == NULL) {
    dhcp_key = reinterpret_cast<key_dhcp_relay_server *>
        (ConfigKeyVal::Malloc(sizeof(key_dhcp_relay_server)));
    if (okey) delete okey;
    okey = new ConfigKeyVal(UNC_KT_DHCPRELAY_SERVER,
                            IpctSt::kIpcStKeyDhcpRelayServer, dhcp_key, NULL);
    return UPLL_RC_SUCCESS;
  } else {
    pkey = parent_key->get_key();
  }
  if (!pkey) return UPLL_RC_ERR_GENERIC;
  if (okey) {
    if (okey->get_key_type() != UNC_KT_DHCPRELAY_SERVER)
      return UPLL_RC_ERR_GENERIC;
    dhcp_key = reinterpret_cast<key_dhcp_relay_server *>(okey->get_key());
  } else {
    dhcp_key = reinterpret_cast<key_dhcp_relay_server *>
        (malloc(sizeof(key_dhcp_relay_server)));
    if (!dhcp_key) return UPLL_RC_ERR_GENERIC;
    memset(dhcp_key, 0, sizeof(key_dhcp_relay_server));
  }
  unc_key_type_t keytype = parent_key->get_key_type();
  switch (keytype) {
    case UNC_KT_VTN:
      uuu::upll_strncpy(dhcp_key->vrt_key.vtn_key.vtn_name,
             reinterpret_cast<key_vtn *>(pkey)->vtn_name,
             (kMaxLenVtnName + 1));
      *(dhcp_key->vrt_key.vrouter_name) = *"";
      break;
    case UNC_KT_VROUTER:
      uuu::upll_strncpy(dhcp_key->vrt_key.vtn_key.vtn_name,
             reinterpret_cast<key_vrt *>(pkey)->vtn_key.vtn_name,
             (kMaxLenVtnName + 1));
      uuu::upll_strncpy(dhcp_key->vrt_key.vrouter_name,
             reinterpret_cast<key_vrt *>(pkey)->vrouter_name,
              (kMaxLenVnodeName + 1));
      break;
    case UNC_KT_DHCPRELAY_SERVER:
      uuu::upll_strncpy(dhcp_key->vrt_key.vtn_key.vtn_name,
          reinterpret_cast<key_dhcp_relay_server *>
          (pkey)->vrt_key.vtn_key.vtn_name,
          (kMaxLenVtnName + 1));
      uuu::upll_strncpy(dhcp_key->vrt_key.vrouter_name,
          reinterpret_cast<key_dhcp_relay_server *>(pkey)->vrt_key.vrouter_name,
          (kMaxLenVnodeName + 1));
      dhcp_key->server_addr.s_addr =
          reinterpret_cast<key_dhcp_relay_server *>(pkey)->server_addr.s_addr;
    default:
      break;
  }
  if (!okey)
    okey = new ConfigKeyVal(UNC_KT_DHCPRELAY_SERVER,
                            IpctSt::kIpcStKeyDhcpRelayServer, dhcp_key, NULL);
  else if (okey->get_key() != dhcp_key)
    okey->SetKey(IpctSt::kIpcStKeyDhcpRelayServer, dhcp_key);
  if (okey == NULL) {
    free(dhcp_key);
    result_code = UPLL_RC_ERR_GENERIC;
  } else {
    SET_USER_DATA(okey, parent_key);
  }
  return result_code;
}

upll_rc_t DhcpRelayServerMoMgr::GetParentConfigKey(ConfigKeyVal *&okey,
                                                   ConfigKeyVal *ikey) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  if (!ikey) {
    UPLL_LOG_DEBUG("ikey is NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  unc_key_type_t ikey_type = ikey->get_key_type();

  if (ikey_type != UNC_KT_DHCPRELAY_SERVER) return UPLL_RC_ERR_GENERIC;
    key_vrt_if *pkey = reinterpret_cast<key_vrt_if *>
        (ikey->get_key());
  if (!pkey) return UPLL_RC_ERR_GENERIC;
  key_vrt *vrt_key = reinterpret_cast<key_vrt *>(malloc(sizeof(key_vrt)));
  if (!vrt_key) return UPLL_RC_ERR_GENERIC;
  memset(vrt_key, 0, sizeof(key_vrt));
  uuu::upll_strncpy(vrt_key->vtn_key.vtn_name,
      reinterpret_cast<key_dhcp_relay_server *>(pkey)->vrt_key.vtn_key.vtn_name,
      (kMaxLenVtnName + 1));
  uuu::upll_strncpy(vrt_key->vrouter_name,
         reinterpret_cast<key_dhcp_relay_server *>(pkey)->vrt_key.vrouter_name,
         (kMaxLenVnodeName + 1));
  if (okey) delete okey;
  okey = new ConfigKeyVal(UNC_KT_VROUTER, IpctSt::kIpcStKeyVrt, vrt_key, NULL);
  if (okey == NULL) {
    free(vrt_key);
    result_code = UPLL_RC_ERR_GENERIC;
  } else {
    SET_USER_DATA(okey, ikey);
  }
  return result_code;
}

upll_rc_t DhcpRelayServerMoMgr::AllocVal(ConfigVal *&ck_val,
                                         upll_keytype_datatype_t dt_type,
                                         MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  void *val;  //  , *nxt_val;
  //  ConfigVal *ck_nxtval;

  if (ck_val != NULL) return UPLL_RC_ERR_GENERIC;
  switch (tbl) {
    case MAINTBL:
      val = malloc(sizeof(val_dhcp_relay_server));
      if (!val) return UPLL_RC_ERR_GENERIC;
      memset(val, 0, sizeof(val_dhcp_relay_server));
      ck_val = new ConfigVal(IpctSt::kIpcStValDhcpRelayServer, val);
      break;
    default:
      val = NULL;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t DhcpRelayServerMoMgr::DupConfigKeyVal(ConfigKeyVal *&okey,
                                                ConfigKeyVal *&req,
                                                MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  if (req == NULL) return UPLL_RC_ERR_GENERIC;
  if (okey != NULL) return UPLL_RC_ERR_GENERIC;
  if (req->get_key_type() != UNC_KT_DHCPRELAY_SERVER)
    return UPLL_RC_ERR_GENERIC;
  ConfigVal *tmp1 = NULL, *tmp = (req)->get_cfg_val();

  if (tmp) {
    if (tbl == MAINTBL) {
      val_dhcp_relay_server *ival =
          reinterpret_cast<val_dhcp_relay_server *>(GetVal(req));
      if (!ival) {
        UPLL_LOG_DEBUG("Invalid dhcp_relay_server value");
        return UPLL_RC_ERR_GENERIC;
      }
      val_dhcp_relay_server *dhcp_val =
          reinterpret_cast<val_dhcp_relay_server *>(malloc(
          sizeof(val_dhcp_relay_server)));
      if (!dhcp_val) return UPLL_RC_ERR_GENERIC;
      memcpy(dhcp_val, ival, sizeof(val_dhcp_relay_server));
      tmp1 = new ConfigVal(IpctSt::kIpcStValDhcpRelayServer, dhcp_val);
    }
  };
  void *tkey = (req != NULL) ? (req)->get_key() : NULL;
  key_dhcp_relay_server *ikey = reinterpret_cast<key_dhcp_relay_server *>(tkey);
  key_dhcp_relay_server *dhcp_key =
      reinterpret_cast<key_dhcp_relay_server *>(ConfigKeyVal::Malloc(
      sizeof(key_dhcp_relay_server)));
  memcpy(dhcp_key, ikey, sizeof(key_dhcp_relay_server));
  okey = new ConfigKeyVal(UNC_KT_DHCPRELAY_SERVER,
                          IpctSt::kIpcStKeyDhcpRelayServer, dhcp_key, tmp1);
  if (!okey) {
    DELETE_IF_NOT_NULL(tmp1);
    FREE_IF_NOT_NULL(dhcp_key);
    return UPLL_RC_ERR_GENERIC;
  } else {
    SET_USER_DATA(okey, req);
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t DhcpRelayServerMoMgr::UpdateConfigStatus(ConfigKeyVal *ikey,
                                                   unc_keytype_operation_t op,
                                                   uint32_t driver_result,
                                                   ConfigKeyVal *upd_key,
                                                   DalDmlIntf *dmi,
                                                   ConfigKeyVal *ctrlr_key) {
  UPLL_FUNC_TRACE;
  val_dhcp_relay_server *dhcp_val =
      reinterpret_cast<val_dhcp_relay_server *>(GetVal(ikey));

  unc_keytype_configstatus_t cs_status =
      (driver_result == UPLL_RC_SUCCESS) ? UNC_CS_APPLIED
                                         : UNC_CS_NOT_APPLIED;
  if (dhcp_val == NULL) return UPLL_RC_ERR_GENERIC;
  if (op == UNC_OP_CREATE) {
    dhcp_val->cs_row_status = cs_status;
  }
  UPLL_LOG_TRACE("%s", (ikey->ToStrAll()).c_str());
  val_dhcp_relay_server *dhcp_val2 =
       reinterpret_cast<val_dhcp_relay_server *>(GetVal(upd_key));
  if (dhcp_val2 == NULL) return UPLL_RC_ERR_GENERIC;
  if (UNC_OP_UPDATE == op) {
    UPLL_LOG_TRACE("%s", (upd_key->ToStrAll()).c_str());
    dhcp_val->cs_row_status = dhcp_val2->cs_row_status;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t DhcpRelayServerMoMgr::UpdateAuditConfigStatus(
    unc_keytype_configstatus_t cs_status, uuc::UpdateCtrlrPhase phase,
    ConfigKeyVal *&ckv_running,
    DalDmlIntf *dmi) {
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  val_dhcp_relay_server_t *val;
  val = (ckv_running != NULL) ?
      reinterpret_cast<val_dhcp_relay_server_t *>(GetVal(ckv_running)) : NULL;
  if (NULL == val) {
    return UPLL_RC_ERR_GENERIC;
  }
  if (uuc::kUpllUcpCreate == phase) val->cs_row_status = cs_status;
  if ((uuc::kUpllUcpUpdate == phase) &&
           (val->cs_row_status == UNC_CS_INVALID ||
            val->cs_row_status == UNC_CS_NOT_APPLIED))
    val->cs_row_status = cs_status;
  return result_code;
}

upll_rc_t DhcpRelayServerMoMgr::GetVrtDhcpRelayServerAddress(ConfigKeyVal *ikey,
                                                             DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code;
  ConfigKeyVal *ck_dhcp = NULL;
  result_code = GetChildConfigKey(ck_dhcp, ikey);
  if (!ck_dhcp || result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetChildConfigKey is failed- %d", result_code);
    return result_code;
  }
  DbSubOp dbop = { kOpReadMultiple, kOpMatchNone, kOpInOutNone };
  result_code = ReadConfigDB(ck_dhcp, UPLL_DT_RUNNING, UNC_OP_READ, dbop,
                                                                 dmi, MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Failed to ReadConfigDb- %d", result_code);
    DELETE_IF_NOT_NULL(ck_dhcp);
    return result_code;
  }
  val_vrt_dhcp_relay_st *val_vrt_dhcp =
      reinterpret_cast<val_vrt_dhcp_relay_st *>
     (ConfigKeyVal::Malloc(sizeof(val_vrt_dhcp_relay_st)));
  val_vrt_dhcp->valid[UPLL_IDX_DHCP_RELAY_ADMIN_STATUS_VDRS] = UNC_VF_INVALID;
  val_vrt_dhcp->valid[UPLL_IDX_IP_COUNT_VDRS] = UNC_VF_VALID;
  val_vrt_dhcp->valid[UPLL_IDX_IF_COUNT_VDRS] = UNC_VF_INVALID;
  ConfigKeyVal *tmp= ck_dhcp;
  while (tmp != NULL) {
    val_vrt_dhcp->ip_count++;
    tmp = tmp->get_next_cfg_key_val();
  }
  ikey->SetCfgVal(new ConfigVal(IpctSt::kIpcStValVrtDhcpRelaySt, val_vrt_dhcp));
  uint32_t ipcount = 0;
  ConfigKeyVal *tkey = ck_dhcp;
  while (tkey  && ipcount < val_vrt_dhcp->ip_count)  {
    key_dhcp_relay_server *dhcp_key =
                  reinterpret_cast<key_dhcp_relay_server_t *>(tkey->get_key());
    uint32_t *ip_addr =
          reinterpret_cast<uint32_t *>(ConfigKeyVal::Malloc(sizeof(uint32_t)));
    *ip_addr = dhcp_key->server_addr.s_addr;  //  assign ipaddress
    ikey->AppendCfgVal(IpctSt::kIpcStIpv4, ip_addr);
    ipcount++;
    tkey = tkey->get_next_cfg_key_val();
  }
  if (ck_dhcp)
    delete ck_dhcp;
  return UPLL_RC_SUCCESS;
}

upll_rc_t DhcpRelayServerMoMgr::CopyToConfigKey(ConfigKeyVal *&okey,
                                                ConfigKeyVal *ikey) {
  if (!ikey || !(ikey->get_key())) return UPLL_RC_ERR_GENERIC;

  upll_rc_t result_code = UPLL_RC_SUCCESS;

  key_rename_vnode_info *key_rename =
      reinterpret_cast<key_rename_vnode_info *>(ikey->get_key());
  key_dhcp_relay_server_t * key_dhcp =
      reinterpret_cast<key_dhcp_relay_server_t*>(malloc(
      sizeof(key_dhcp_relay_server_t)));

  if (!key_dhcp) return UPLL_RC_ERR_GENERIC;
  memset(key_dhcp, 0, sizeof(key_dhcp_relay_server));
  if (!strlen(reinterpret_cast<char *>(key_rename->old_unc_vtn_name))) {
    FREE_IF_NOT_NULL(key_dhcp);
    return UPLL_RC_ERR_GENERIC;
  }
  uuu::upll_strncpy(key_dhcp->vrt_key.vtn_key.vtn_name,
         key_rename->old_unc_vtn_name,
         (kMaxLenVtnName + 1));
  if (ikey->get_key_type() == UNC_KT_VROUTER) {
    if (!strlen(reinterpret_cast<char *>(key_rename->old_unc_vnode_name))) {
      FREE_IF_NOT_NULL(key_dhcp);
      return UPLL_RC_ERR_GENERIC;
    }
    uuu::upll_strncpy(key_dhcp->vrt_key.vrouter_name,
           key_rename->old_unc_vnode_name,
           (kMaxLenVnodeName + 1));
  } else {
    if (!strlen(reinterpret_cast<char *>(key_rename->new_unc_vnode_name))) {
      FREE_IF_NOT_NULL(key_dhcp);
      return UPLL_RC_ERR_GENERIC;
    }
    uuu::upll_strncpy(key_dhcp->vrt_key.vrouter_name,
       key_rename->new_unc_vnode_name, (kMaxLenVnodeName+1));
  }

  okey = new ConfigKeyVal(UNC_KT_DHCPRELAY_SERVER,
                          IpctSt::kIpcStKeyDhcpRelayServer, key_dhcp, NULL);
  if (!okey) {
    free(key_dhcp);
    return UPLL_RC_ERR_GENERIC;
  }
  return result_code;
}

upll_rc_t DhcpRelayServerMoMgr::ValidateMessage(IpcReqRespHeader *req,
                                                ConfigKeyVal *ikey) {
  UPLL_FUNC_TRACE;
  if (!ikey || !req || !(ikey->get_key())) {
    UPLL_LOG_DEBUG("ConfigKeyVal or IpcReqRespHeader is Null");
    return UPLL_RC_ERR_GENERIC;
  }
  upll_keytype_datatype_t dt_type = req->datatype;
  unc_keytype_operation_t op = req->operation;
  unc_keytype_option1_t option1 = req->option1;
  unc_keytype_option2_t option2 = req->option2;

  upll_rc_t ret_val;
  if (ikey->get_st_num() != IpctSt::kIpcStKeyDhcpRelayServer) {
    UPLL_LOG_DEBUG("Invalid key structure received. received struct - %d",
                  ikey->get_st_num());
    return UPLL_RC_ERR_BAD_REQUEST;
  }
  key_dhcp_relay_server *dhcprelay_key =
      reinterpret_cast<key_dhcp_relay_server *>(ikey->get_key());
  unc_key_type_t ktype = ikey->get_key_type();
  if (UNC_KT_DHCPRELAY_SERVER != ktype) {
    UPLL_LOG_DEBUG("Invalid Keytype received. received - %d", ktype);
    return UPLL_RC_ERR_BAD_REQUEST;
  }
  ret_val = ValidateDhcpRelayKey(dhcprelay_key, op);
  if (ret_val != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("syntax check failed for key_dhcp_relay struct");
    return UPLL_RC_ERR_CFG_SYNTAX;
  }
  if (op == UNC_OP_CREATE || op == UNC_OP_DELETE
      || op == UNC_OP_READ || op == UNC_OP_READ_SIBLING
      || op == UNC_OP_READ_SIBLING_BEGIN || op == UNC_OP_READ_SIBLING_COUNT
      || op == UNC_OP_READ_NEXT || op == UNC_OP_READ_BULK) {
    if (dt_type == UPLL_DT_CANDIDATE || dt_type == UPLL_DT_RUNNING
        || dt_type == UPLL_DT_STARTUP || dt_type == UPLL_DT_STATE ||
          UPLL_DT_IMPORT == dt_type) {
      if (option1 != UNC_OPT1_NORMAL)
        return UPLL_RC_ERR_INVALID_OPTION1;
      if (option2 != UNC_OPT2_NONE)
        return UPLL_RC_ERR_INVALID_OPTION2;
      UPLL_LOG_DEBUG("Value structure is none for operation type:%d", op);
      return UPLL_RC_SUCCESS;
    } else {
      UPLL_LOG_DEBUG("Unsupported data type - (%d)", dt_type);
      return UPLL_RC_ERR_GENERIC;
    }
  }
  UPLL_LOG_DEBUG("Unsupported operation - (%d)", op);
  return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT;
}

upll_rc_t DhcpRelayServerMoMgr::ValidateDhcpRelayKey(
    key_dhcp_relay_server *dhcprelay_key,
    unc_keytype_operation_t operation) {

  UPLL_FUNC_TRACE;
  upll_rc_t ret_val;
  uint32_t val = 0;
  ret_val = ValidateKey(reinterpret_cast<char *>
                        (dhcprelay_key->vrt_key.vtn_key.vtn_name),
                        kMinLenVtnName, kMaxLenVtnName);
  if (ret_val != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("syntax check failed. VTN Name - %s",
                  dhcprelay_key->vrt_key.vtn_key.vtn_name);
    return UPLL_RC_ERR_CFG_SYNTAX;
  }
  ret_val = ValidateKey(reinterpret_cast<char *>(
                        dhcprelay_key->vrt_key.vrouter_name),
                        kMinLenVnodeName, kMaxLenVnodeName);
  if (ret_val != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("syntax check failed. VROUTER Name -%s",
                  dhcprelay_key->vrt_key.vrouter_name);
    return UPLL_RC_ERR_CFG_SYNTAX;
  }
  if ((operation != UNC_OP_READ_SIBLING_BEGIN) &&
     (operation != UNC_OP_READ_SIBLING_COUNT)) {
    val = dhcprelay_key->server_addr.s_addr;
    if (val == 0x00000000 || val == 0xffffffff) {
      UPLL_LOG_DEBUG("INVALID SERVER ADDRESS RECEIVED. received addr - %d",
                    dhcprelay_key->server_addr.s_addr);
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  } else {
    UPLL_LOG_TRACE("Operation is %d", operation);
    dhcprelay_key->server_addr.s_addr = 0x00000000;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t DhcpRelayServerMoMgr::ValidateCapability(IpcReqRespHeader *req,
                                                   ConfigKeyVal *ikey, const
                                                   char *ctrlr_name) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_ERR_GENERIC;

  if ((NULL == req) || (NULL == ikey)) {
    UPLL_LOG_DEBUG("IpcReqRespHeader/ConfigKeyval is NULL");
    return result_code;
  }

  if (!ctrlr_name) {
    ctrlr_name = reinterpret_cast<char*>((reinterpret_cast<key_user_data_t *>
                  (ikey->get_user_data()))->ctrlr_id);
    if (!ctrlr_name || !strlen(ctrlr_name)) {
      UPLL_LOG_DEBUG("Controller Name is NULL");
      return UPLL_RC_ERR_GENERIC;
    }
  }

  upll_keytype_datatype_t dt_type = req->datatype;
  unc_keytype_operation_t operation = req->operation;
  unc_keytype_option1_t option1 = req->option1;
  unc_keytype_option2_t option2 = req->option2;

  UPLL_LOG_TRACE("Controller - %s"
                 "dt_type: (%d) "
                 "operation: (%d) "
                 "option1: (%d) "
                 "option2: (%d) ",
                 ctrlr_name, dt_type, operation, option1, option2);

  bool ret_code = false;
  uint32_t max_attrs = 0;
  uint32_t max_instance_count = 0;
  const uint8_t *attrs = NULL;
  switch (operation) {
    case UNC_OP_CREATE: {
      ret_code = GetCreateCapability(ctrlr_name,
                                     ikey->get_key_type(),
                                     &max_instance_count,
                                     &max_attrs,
                                     &attrs);
      break;
    }
    case UNC_OP_UPDATE: {
      ret_code = GetUpdateCapability(ctrlr_name,
                                     ikey->get_key_type(),
                                     &max_attrs,
                                     &attrs);
      break;
    }
    case UNC_OP_READ:
    case UNC_OP_READ_SIBLING:
    case UNC_OP_READ_SIBLING_BEGIN:
    case UNC_OP_READ_SIBLING_COUNT: {
      ret_code = GetReadCapability(ctrlr_name,
                                   ikey->get_key_type(),
                                   &max_attrs,
                                   &attrs);
      break;
    }
    default:
      UPLL_LOG_DEBUG("Invalid Operation Code - (%d)", operation);
      return UPLL_RC_ERR_GENERIC;
  }
  UPLL_LOG_DEBUG("ret_code (%d)", ret_code);
  if (!ret_code) {
    UPLL_LOG_DEBUG("keytype(%d) is not supported by controller(%s) "
                   "for opearion(%d)",
                   ikey->get_key_type(), ctrlr_name, operation);
    return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t DhcpRelayServerMoMgr::IsReferenced(IpcReqRespHeader *req,
                                             ConfigKeyVal *ikey,
                                             DalDmlIntf *dmi) {
  return UPLL_RC_SUCCESS;
}

upll_rc_t DhcpRelayServerMoMgr::IsAdminStatusEnable(ConfigKeyVal *ikey,
                                                DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  MoMgrImpl *vrt_mgr = reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>
                                            (GetMoManager(UNC_KT_VROUTER)));
  if (!vrt_mgr) {
    UPLL_LOG_DEBUG("Invalid param");
    return UPLL_RC_ERR_GENERIC;
  }
  ConfigKeyVal *ckv_vrt = NULL;
  result_code = GetParentConfigKey(ckv_vrt, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Returing error %d \n", result_code);
    return UPLL_RC_ERR_GENERIC;
  }
  DbSubOp dbop = {kOpReadSingle, kOpMatchNone, kOpInOutNone};
  result_code = vrt_mgr->ReadConfigDB(ckv_vrt, UPLL_DT_CANDIDATE, UNC_OP_READ,
                             dbop, dmi, MAINTBL);
  if (result_code == UPLL_RC_SUCCESS) {
    val_vrt_t *vrt_val = reinterpret_cast<val_vrt_t *>(GetVal(ckv_vrt));
    if (vrt_val && vrt_val->dhcp_relay_admin_status == UPLL_ADMIN_ENABLE) {
      UPLL_LOG_DEBUG("DHCP relay agent must be disabled!");
      delete ckv_vrt;
      return UPLL_RC_ERR_CFG_SEMANTIC;
    }
  }
  DELETE_IF_NOT_NULL(ckv_vrt);
  return result_code;
}

}  // namespace kt_momgr
}  // namespace upll
}  // namespace unc
