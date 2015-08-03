/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include "dhcprelay_if_momgr.hh"
#define NUM_KEY_MAIN_TBL_ 6
#if 0
namespace upll_dal_vbrif unc::upll::dal::schema::table::vbridge_interface;
#endif

namespace unc {
namespace upll {
namespace kt_momgr {

BindInfo DhcpRelayIfMoMgr::dhcprelay_if_bind_info[] = {
    { uudst::dhcprelay_interface::kDbiVtnName, CFG_KEY, offsetof(
        key_dhcp_relay_if, vrt_key.vtn_key.vtn_name),
      uud::kDalChar, 32 },
    { uudst::dhcprelay_interface::kDbiVrtName, CFG_KEY, offsetof(
        key_dhcp_relay_if, vrt_key.vrouter_name),
      uud::kDalChar, 32 },
    { uudst::dhcprelay_interface::kDbiIfName, CFG_KEY, offsetof(
        key_dhcp_relay_if, if_name),
      uud::kDalChar, 32 },
    { uudst::dhcprelay_interface::kDbiCtrlrName, CK_VAL, offsetof(key_user_data,
                                                                  ctrlr_id),
      uud::kDalChar, 32 },
    { uudst::dhcprelay_interface::kDbiDomainId, CK_VAL, offsetof(key_user_data,
                                                                 domain_id),
      uud::kDalChar, 32 },
    { uudst::dhcprelay_interface::kDbiCsRowstatus, CS_VAL, offsetof(
        val_dhcp_relay_if, cs_row_status),
      uud::kDalUint8, 1 },
    { uudst::dhcprelay_interface::kDbiFlags, CK_VAL, offsetof(key_user_data,
                                                              flags),
      uud::kDalUint8, 1 }, };

BindInfo DhcpRelayIfMoMgr::dhcprealy_if_maintbl_key_update_bind_info[] = {
    { uudst::dhcprelay_interface::kDbiVtnName, CFG_MATCH_KEY, offsetof(
        key_dhcp_relay_if, vrt_key.vtn_key.vtn_name),
      uud::kDalChar, kMaxLenVtnName + 1 },
    { uudst::dhcprelay_interface::kDbiVrtName, CFG_MATCH_KEY, offsetof(
        key_dhcp_relay_if, vrt_key.vrouter_name),
      uud::kDalChar, kMaxLenVnodeName + 1 },
    { uudst::dhcprelay_interface::kDbiIfName, CFG_MATCH_KEY, offsetof(
        key_dhcp_relay_if, if_name),
      uud::kDalChar, kMaxLenVnodeName + 1 },
    { uudst::dhcprelay_interface::kDbiVtnName, CFG_INPUT_KEY, offsetof(
        key_rename_vnode_info_t, new_unc_vtn_name),
      uud::kDalChar, kMaxLenVtnName + 1 },
    { uudst::dhcprelay_interface::kDbiVrtName, CFG_INPUT_KEY, offsetof(
        key_rename_vnode_info_t, new_unc_vnode_name),
      uud::kDalChar, kMaxLenVnodeName + 1 },
    { uudst::dhcprelay_interface::kDbiFlags, CK_VAL, offsetof(
        key_user_data_t, flags),
      uud::kDalUint8, 1 } };

DhcpRelayIfMoMgr::DhcpRelayIfMoMgr() {
  UPLL_FUNC_TRACE
  ntable = MAX_MOMGR_TBLS;
  table = new Table *[ntable]();
  table[MAINTBL]= new Table(uudst::kDbiDhcpRelayIfTbl, UNC_KT_DHCPRELAY_IF,
                   dhcprelay_if_bind_info, IpctSt::kIpcStKeyDhcpRelayIf,
                   IpctSt::kIpcStValDhcpRelayIf,
                   uudst::dhcprelay_interface::kDbiDhcpRelayIfNumCols);
  table[RENAMETBL] = NULL;
  table[CTRLRTBL] = NULL;
  table[CONVERTTBL] = NULL;
  nchild = 0;
  child = NULL;
}

/*
 * Based on the key type the bind info will pass
 **/

bool DhcpRelayIfMoMgr::GetRenameKeyBindInfo(unc_key_type_t key_type,
                                            BindInfo *&binfo, int &nattr,
                                            MoMgrTables tbl) {
  /* Main Table or rename table only update */
  if (MAINTBL == tbl) {
    nattr = NUM_KEY_MAIN_TBL_;
    binfo = dhcprealy_if_maintbl_key_update_bind_info;
  } else {
    UPLL_LOG_DEBUG("Invalide Key");
    return PFC_FALSE;
  }
  return PFC_TRUE;
}

upll_rc_t DhcpRelayIfMoMgr::ValidateAttribute(ConfigKeyVal *ikey,
                                              DalDmlIntf *dmi,
                                              IpcReqRespHeader *req) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  if (ikey->get_key_type() != UNC_KT_DHCPRELAY_IF)
    result_code = UPLL_RC_ERR_CFG_SYNTAX;

  #if 0
  /* Check if vrouter admin status is disabled */
  result_code = IsAdminStatusEnable(ikey, dmi);
  if (result_code != UPLL_RC_SUCCESS) {
    return UPLL_RC_ERR_CFG_SEMANTIC;
  }
  #endif

    /* Check if vrt interface exists */
  MoMgrImpl *mgr = reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>
                                            (GetMoManager(UNC_KT_VRT_IF)));
  if (!mgr) {
    UPLL_LOG_DEBUG("Invalid param");
    return UPLL_RC_ERR_GENERIC;
  }
  ConfigKeyVal *ckv_vrtif = NULL;
  result_code = mgr->GetChildConfigKey(ckv_vrtif, ikey);
  if (!ckv_vrtif || result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Returing error %d", result_code);
    return UPLL_RC_ERR_GENERIC;
  }
  DbSubOp dbop = {kOpReadExist, kOpMatchNone, kOpInOutNone};
  result_code = mgr->UpdateConfigDB(ckv_vrtif, req->datatype,
                         UNC_OP_READ, dmi, &dbop, MAINTBL);
  if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    UPLL_LOG_DEBUG("Vrt interface does not exist %s",
                   (ckv_vrtif->ToStr()).c_str());
    delete ckv_vrtif;
    return UPLL_RC_ERR_CFG_SEMANTIC;
  } else if (result_code == UPLL_RC_ERR_INSTANCE_EXISTS) {
    delete ckv_vrtif;
    return UPLL_RC_SUCCESS;
  } else  {
    UPLL_LOG_DEBUG(" Returning error %d", result_code);
  }
  delete ckv_vrtif;
  return result_code;
}

upll_rc_t DhcpRelayIfMoMgr::ValidateCapability(IpcReqRespHeader *req,
                                               ConfigKeyVal *ikey,
                                               const char *ctrlr_name) {
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

bool DhcpRelayIfMoMgr::IsValidKey(void *key, uint64_t index,
                                  MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  key_dhcp_relay_if *dhcp_rs_key = reinterpret_cast<key_dhcp_relay_if *>(key);
  upll_rc_t ret_val = UPLL_RC_SUCCESS;
  switch (index) {
    case uudst::dhcprelay_interface::kDbiVtnName:
      ret_val = ValidateKey(reinterpret_cast<char *>
                            (dhcp_rs_key->vrt_key.vtn_key.vtn_name),
                            kMinLenVtnName, kMaxLenVtnName);
      if (ret_val != UPLL_RC_SUCCESS) {
        UPLL_LOG_TRACE("VTN Name is not valid(%d)", ret_val);
        return false;
      }
      break;
    case uudst::dhcprelay_interface::kDbiVrtName:
      ret_val = ValidateKey(reinterpret_cast<char *>
                            (dhcp_rs_key->vrt_key.vrouter_name),
                            kMinLenVnodeName, kMaxLenVnodeName);
      if (ret_val != UPLL_RC_SUCCESS) {
        UPLL_LOG_TRACE("VRouter Name is not valid(%d)", ret_val);
        return false;
      }
      break;
    case uudst::dhcprelay_interface::kDbiIfName:
      ret_val = ValidateKey(reinterpret_cast<char *>(dhcp_rs_key->if_name),
                            kMinLenInterfaceName, kMaxLenInterfaceName);
      if (ret_val != UPLL_RC_SUCCESS) {
        UPLL_LOG_TRACE("dhcprelayIf Name is not valid(%d)", ret_val);
        return false;
      }
      break;
    default:
      UPLL_LOG_TRACE("Invalid Key Index");
      break;
  }
  return true;
}

upll_rc_t DhcpRelayIfMoMgr::GetChildConfigKey(ConfigKeyVal *&okey,
                                              ConfigKeyVal *parent_key) {
  // UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  key_dhcp_relay_if *dhcp_key;
  void *pkey;
  if (parent_key == NULL) {
    dhcp_key = reinterpret_cast<key_dhcp_relay_if *>
       (ConfigKeyVal::Malloc(sizeof(key_dhcp_relay_if)));
    if (okey) delete okey;
    okey = new ConfigKeyVal(UNC_KT_DHCPRELAY_IF, IpctSt::kIpcStKeyDhcpRelayIf,
                            dhcp_key, NULL);
    return UPLL_RC_SUCCESS;
  } else {
    pkey = parent_key->get_key();
  }
  if (!pkey) return UPLL_RC_ERR_GENERIC;
  if (okey) {
    if (okey->get_key_type() != UNC_KT_DHCPRELAY_IF) return UPLL_RC_ERR_GENERIC;
    dhcp_key = reinterpret_cast<key_dhcp_relay_if *>(okey->get_key());
  } else {
    dhcp_key = reinterpret_cast<key_dhcp_relay_if *>
       (ConfigKeyVal::Malloc(sizeof(key_dhcp_relay_if)));
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
    case UNC_KT_VRT_IF:
    case UNC_KT_DHCPRELAY_IF:
      uuu::upll_strncpy(dhcp_key->vrt_key.vtn_key.vtn_name,
          reinterpret_cast<key_dhcp_relay_if *>(pkey)->vrt_key.vtn_key.vtn_name,
          (kMaxLenVtnName + 1));
      uuu::upll_strncpy(dhcp_key->vrt_key.vrouter_name,
             reinterpret_cast<key_dhcp_relay_if *>(pkey)->vrt_key.vrouter_name,
             (kMaxLenVnodeName + 1));
      uuu::upll_strncpy(dhcp_key->if_name,
             reinterpret_cast<key_dhcp_relay_if *>(pkey)->if_name,
             (kMaxLenInterfaceName + 1));
    default:
      break;
  }
  if (!okey)
    okey = new ConfigKeyVal(UNC_KT_DHCPRELAY_IF, IpctSt::kIpcStKeyDhcpRelayIf,
                            dhcp_key, NULL);
  else if (okey->get_key() != dhcp_key)
    okey->SetKey(IpctSt::kIpcStKeyDhcpRelayIf, dhcp_key);
  if (okey == NULL) {
    free(dhcp_key);
    result_code = UPLL_RC_ERR_GENERIC;
  } else {
    SET_USER_DATA(okey, parent_key);
  }
  return result_code;
}

upll_rc_t DhcpRelayIfMoMgr::GetParentConfigKey(ConfigKeyVal *&okey,
                                               ConfigKeyVal *ikey) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  if (!ikey)
    return UPLL_RC_ERR_GENERIC;
  unc_key_type_t ikey_type = ikey->get_key_type();

  if (ikey_type != UNC_KT_DHCPRELAY_IF) return UPLL_RC_ERR_GENERIC;
  key_dhcp_relay_if *pkey = reinterpret_cast<key_dhcp_relay_if *>
      (ikey->get_key());
  if (!pkey) return UPLL_RC_ERR_GENERIC;
  key_vrt *vrt_key = reinterpret_cast<key_vrt *>
    (ConfigKeyVal::Malloc(sizeof(key_vrt)));
  uuu::upll_strncpy(vrt_key->vtn_key.vtn_name,
    reinterpret_cast<key_dhcp_relay_if*>(pkey)->vrt_key.vtn_key.vtn_name,
        (kMaxLenVtnName + 1));
      uuu::upll_strncpy(vrt_key->vrouter_name,
        reinterpret_cast<key_dhcp_relay_if*>(pkey)->vrt_key.vrouter_name,
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

upll_rc_t DhcpRelayIfMoMgr::AllocVal(ConfigVal *&ck_val,
                                     upll_keytype_datatype_t dt_type,
                                     MoMgrTables tbl) {
  // UPLL_FUNC_TRACE;
  void *val;  // , *nxt_val;
  //  ConfigVal *ck_nxtval;

  if (ck_val != NULL) return UPLL_RC_ERR_GENERIC;
  switch (tbl) {
    case MAINTBL:
      val = ConfigKeyVal::Malloc(sizeof(val_dhcp_relay_if));
      ck_val = new ConfigVal(IpctSt::kIpcStValDhcpRelayIf, val);
      break;
    default:
      val = NULL;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t DhcpRelayIfMoMgr::DupConfigKeyVal(ConfigKeyVal *&okey,
                                            ConfigKeyVal *&req,
                                            MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  if (req == NULL) return UPLL_RC_ERR_GENERIC;
  if (okey != NULL) return UPLL_RC_ERR_GENERIC;
  if (req->get_key_type() != UNC_KT_DHCPRELAY_IF) return UPLL_RC_ERR_GENERIC;
  ConfigVal *tmp1 = NULL, *tmp = (req)->get_cfg_val();

  if (tmp) {
    if (tbl == MAINTBL) {
      val_dhcp_relay_if *ival =
          reinterpret_cast<val_dhcp_relay_if *>(GetVal(req));
      val_dhcp_relay_if *dhcp_val =
          reinterpret_cast<val_dhcp_relay_if *>(ConfigKeyVal::Malloc(
          sizeof(val_dhcp_relay_if)));
      memcpy(dhcp_val, ival, sizeof(val_dhcp_relay_if));
      tmp1 = new ConfigVal(IpctSt::kIpcStValDhcpRelayIf, dhcp_val);
    }
  };
  void *tkey = (req)->get_key();
  key_dhcp_relay_if *ikey = reinterpret_cast<key_dhcp_relay_if *>(tkey);
  key_dhcp_relay_if *dhcp_key = reinterpret_cast<key_dhcp_relay_if *>(
    ConfigKeyVal::Malloc(sizeof(key_dhcp_relay_if)));
  memcpy(dhcp_key, ikey, sizeof(key_dhcp_relay_if));
  okey = new ConfigKeyVal(UNC_KT_DHCPRELAY_IF, IpctSt::kIpcStKeyDhcpRelayIf,
                          dhcp_key, tmp1);
  if (!okey) {
    DELETE_IF_NOT_NULL(tmp1);
    FREE_IF_NOT_NULL(dhcp_key);
    return UPLL_RC_ERR_GENERIC;
  } else {
    SET_USER_DATA(okey, req);
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t DhcpRelayIfMoMgr::UpdateConfigStatus(ConfigKeyVal *ikey,
                                               unc_keytype_operation_t op,
                                               uint32_t driver_result,
                                               ConfigKeyVal *upd_key,
                                               DalDmlIntf *dmi,
                                               ConfigKeyVal *ctrlr_key) {
  UPLL_FUNC_TRACE;
  val_dhcp_relay_if *dhcp_val =
      reinterpret_cast<val_dhcp_relay_if *>(GetVal(ikey));

  unc_keytype_configstatus_t cs_status =
      (driver_result == UPLL_RC_SUCCESS) ? UNC_CS_APPLIED
                                         : UNC_CS_NOT_APPLIED;
  if (dhcp_val == NULL) return UPLL_RC_ERR_GENERIC;
  if (op == UNC_OP_CREATE) {
    dhcp_val->cs_row_status = cs_status;
  }
  UPLL_LOG_TRACE("%s", (ikey->ToStrAll()).c_str());
  val_dhcp_relay_if *dhcp_val2 =
          reinterpret_cast<val_dhcp_relay_if *>(GetVal(upd_key));
  if (dhcp_val2 == NULL) return UPLL_RC_ERR_GENERIC;
  if (UNC_OP_UPDATE == op) {
    UPLL_LOG_TRACE("%s", (upd_key->ToStrAll()).c_str());
    dhcp_val->cs_row_status = dhcp_val2->cs_row_status;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t DhcpRelayIfMoMgr::UpdateAuditConfigStatus(
    unc_keytype_configstatus_t cs_status,
    uuc::UpdateCtrlrPhase phase,
    ConfigKeyVal *&ckv_running,
    DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  val_dhcp_relay_if_t *val;
  val = (ckv_running != NULL) ?
      reinterpret_cast<val_dhcp_relay_if_t *>(GetVal(ckv_running)) : NULL;
  if (NULL == val) return UPLL_RC_ERR_GENERIC;
  if (uuc::kUpllUcpCreate == phase) val->cs_row_status = cs_status;
  if ((uuc::kUpllUcpUpdate == phase) &&
           (val->cs_row_status == UNC_CS_INVALID ||
            val->cs_row_status == UNC_CS_NOT_APPLIED))
    val->cs_row_status = cs_status;
  return result_code;
}

upll_rc_t DhcpRelayIfMoMgr::CopyToConfigKey(ConfigKeyVal *&okey,
                                            ConfigKeyVal *ikey) {
  if (!ikey || !(ikey->get_key())) return UPLL_RC_ERR_GENERIC;

  upll_rc_t result_code = UPLL_RC_SUCCESS;

  key_rename_vnode_info *key_rename =
      reinterpret_cast<key_rename_vnode_info *>(ikey->get_key());
  if (!strlen(reinterpret_cast<char *>(key_rename->old_unc_vtn_name)))
    return UPLL_RC_ERR_GENERIC;
  key_dhcp_relay_if_t * key_dhcp = reinterpret_cast<key_dhcp_relay_if_t *>
      (ConfigKeyVal::Malloc(sizeof(key_dhcp_relay_if_t)));
  uuu::upll_strncpy(key_dhcp->vrt_key.vtn_key.vtn_name,
         key_rename->old_unc_vtn_name, (kMaxLenVtnName + 1));
  if (ikey->get_key_type() == UNC_KT_VROUTER) {
    if (!strlen(reinterpret_cast<char *>(key_rename->old_unc_vnode_name))) {
      free(key_dhcp);
      return UPLL_RC_ERR_GENERIC;
    }
    uuu::upll_strncpy(key_dhcp->vrt_key.vrouter_name,
           key_rename->old_unc_vnode_name, (kMaxLenVnodeName + 1));
  } else {
    if (!strlen(reinterpret_cast<char *>(key_rename->new_unc_vnode_name))) {
      FREE_IF_NOT_NULL(key_dhcp);
      return UPLL_RC_ERR_GENERIC;
    }
    uuu::upll_strncpy(key_dhcp->vrt_key.vrouter_name,
       key_rename->new_unc_vnode_name, (kMaxLenVnodeName+1));
  }

  okey = new ConfigKeyVal(UNC_KT_DHCPRELAY_IF, IpctSt::kIpcStKeyDhcpRelayIf,
                          key_dhcp, NULL);
  if (!okey) {
    free(key_dhcp);
    return UPLL_RC_ERR_GENERIC;
  }
  return result_code;
}

upll_rc_t DhcpRelayIfMoMgr::ValidateMessage(IpcReqRespHeader *req,
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
  if (ikey->get_st_num() != IpctSt::kIpcStKeyDhcpRelayIf) {
     UPLL_LOG_DEBUG("Invalid key structure received. received struct num - %d",
                  ikey->get_st_num());
    return UPLL_RC_ERR_BAD_REQUEST;
  }
  key_dhcp_relay_if *dhcprelayif_key =
      reinterpret_cast<key_dhcp_relay_if *>(ikey->get_key());
  unc_key_type_t ktype = ikey->get_key_type();
  if (UNC_KT_DHCPRELAY_IF != ktype) {
    UPLL_LOG_DEBUG("Invalid Keytype received. received keytype - %d", ktype);
    return UPLL_RC_ERR_BAD_REQUEST;
  }
  ret_val = ValidateDhcpRelayIfKey(dhcprelayif_key, op);

  if (ret_val != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("syntax check failure for key_dhcp_relay struct");
    return UPLL_RC_ERR_CFG_SYNTAX;
  }
  if (op == UNC_OP_CREATE || op == UNC_OP_DELETE
      || op == UNC_OP_READ || op == UNC_OP_READ_SIBLING
      || op == UNC_OP_READ_SIBLING_BEGIN || op == UNC_OP_READ_SIBLING_COUNT
      || op == UNC_OP_READ_NEXT || op == UNC_OP_READ_BULK) {
    if (dt_type == UPLL_DT_CANDIDATE || dt_type == UPLL_DT_RUNNING
        || dt_type == UPLL_DT_STARTUP || dt_type == UPLL_DT_STATE
        || UPLL_DT_IMPORT == dt_type) {
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
  UPLL_LOG_DEBUG("Unsupported operation -(%d)", op);
  return UPLL_RC_ERR_GENERIC;
}

upll_rc_t DhcpRelayIfMoMgr::ValidateDhcpRelayIfKey(
    key_dhcp_relay_if *dhcprelayif_key,
    unc_keytype_operation_t operation) {

  UPLL_FUNC_TRACE;
  upll_rc_t ret_val;

  ret_val = ValidateKey(reinterpret_cast<char *>
          (dhcprelayif_key->vrt_key.vtn_key.vtn_name),
          kMinLenVtnName, kMaxLenVtnName);
  if (ret_val != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("syntax check failed. VTN Name - %s",
                  dhcprelayif_key->vrt_key.vtn_key.vtn_name);
    return UPLL_RC_ERR_CFG_SYNTAX;
  }
  ret_val = ValidateKey(reinterpret_cast<char *>
                        (dhcprelayif_key->vrt_key.vrouter_name),
                        kMinLenVnodeName, kMaxLenVnodeName);
  if (ret_val != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("syntax check failed. VROUTER Name - %s",
                  dhcprelayif_key->vrt_key.vrouter_name);
    return UPLL_RC_ERR_CFG_SYNTAX;
  }
  if (operation != UNC_OP_READ_SIBLING_COUNT &&
      operation != UNC_OP_READ_SIBLING_BEGIN) {
    ret_val = ValidateKey(reinterpret_cast<char *>
                        (dhcprelayif_key->if_name), kMinLenInterfaceName,
                        kMaxLenInterfaceName);
    if (ret_val != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("syntax check failed. If name - %s",
                  dhcprelayif_key->if_name);
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  } else {
    UPLL_LOG_TRACE("Operation is %d", operation);
    StringReset(dhcprelayif_key->if_name);
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t DhcpRelayIfMoMgr::IsReferenced(IpcReqRespHeader *req,
                                         ConfigKeyVal *ikey,
                                         DalDmlIntf *dmi) {
  return UPLL_RC_SUCCESS;
}

upll_rc_t DhcpRelayIfMoMgr::IsAdminStatusEnable(ConfigKeyVal *ikey,
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
  if (!ckv_vrt || result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Returing error %d", result_code);
    return UPLL_RC_ERR_GENERIC;
  }
  DbSubOp dbop = {kOpReadSingle, kOpMatchNone, kOpInOutNone};
  result_code = vrt_mgr->ReadConfigDB(ckv_vrt, UPLL_DT_CANDIDATE, UNC_OP_READ,
                             dbop, dmi, MAINTBL);
  if (result_code == UPLL_RC_SUCCESS) {
    val_vrt_t *vrt_val = reinterpret_cast<val_vrt_t *>(GetVal(ckv_vrt));
    if (vrt_val && vrt_val->dhcp_relay_admin_status == UPLL_ADMIN_ENABLE) {
      UPLL_LOG_DEBUG("DHCP relay agent must be disabled!");
      DELETE_IF_NOT_NULL(ckv_vrt);
      return UPLL_RC_ERR_CFG_SEMANTIC;
    }
  }
  DELETE_IF_NOT_NULL(ckv_vrt);
  return result_code;
}

}  // namespace kt_momgr
}  // namespace upll
}  // namespace unc
