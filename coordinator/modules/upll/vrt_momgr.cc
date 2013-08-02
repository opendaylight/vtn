/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include "vrt_momgr.hh"

#define NUM_KEY_MAIN_TBL_ 5
#define NUM_KEY_RENAME_TBL_ 4
namespace unc {
namespace upll {
namespace kt_momgr {

BindInfo VrtMoMgr::vrt_bind_info[] = {
    { uudst::vrouter::kDbiVtnName, CFG_KEY, offsetof(key_vrt, vtn_key.vtn_name),
      uud::kDalChar, 32 },
    { uudst::vrouter::kDbiVrtName, CFG_KEY, offsetof(key_vrt, vrouter_name),
      uud::kDalChar, 32 },
    { uudst::vrouter::kDbiCtrlrName, CFG_VAL, offsetof(val_vrt, controller_id),
      uud::kDalChar, 32 },
    { uudst::vrouter::kDbiCtrlrName, CK_VAL, offsetof(key_user_data, ctrlr_id),
      uud::kDalChar, 32 },
    { uudst::vrouter::kDbiDomainId, CFG_VAL, offsetof(val_vrt, domain_id),
      uud::kDalChar, 32 },
    { uudst::vrouter::kDbiDomainId, CK_VAL, offsetof(key_user_data, domain_id),
      uud::kDalChar, 32 },
    { uudst::vrouter::kDbiVrtDesc, CFG_VAL, offsetof(val_vrt, vrt_description),
      uud::kDalChar, 128 },
    { uudst::vrouter::kDbiDhcprelayAdminstatus, CFG_VAL, offsetof(
        val_vrt, dhcp_relay_admin_status),
      uud::kDalUint8, 1 },
    { uudst::vrouter::kDbiOperStatus, ST_VAL, offsetof(val_db_vrt_st,
                                                       vrt_val_st.oper_status),
      uud::kDalUint8, 1 },
    { uudst::vrouter::kDbiDownCount, ST_VAL, offsetof(val_db_vrt_st,
                                                      down_count),
      uud::kDalUint32, 1 },
    { uudst::vrouter::kDbiValidCtrlrName, CFG_META_VAL, offsetof(
        val_vrt, valid[UPLL_IDX_CONTROLLER_ID_VRT]),
      uud::kDalUint8, 1 },
    { uudst::vrouter::kDbiValidDomainId, CFG_META_VAL, offsetof(
        val_vrt, valid[UPLL_IDX_DOMAIN_ID_VRT]),
      uud::kDalUint8, 1 },
    { uudst::vrouter::kDbiValidDesc, CFG_META_VAL, offsetof(
        val_vrt, valid[UPLL_IDX_DESC_VRT]),
      uud::kDalUint8, 1 },
    { uudst::vrouter::kDbiValidDhcprelayAdminstatus, CFG_META_VAL, offsetof(
        val_vrt, valid[UPLL_IDX_DHCP_RELAY_ADMIN_STATUS_VRT]),
      uud::kDalUint8, 1 },
    { uudst::vrouter::kDbiValidOperStatus, ST_META_VAL, offsetof(
        val_vrt_st, valid[UPLL_IDX_OPER_STATUS_VRTS]),
      uud::kDalUint8, 1 },
    { uudst::vrouter::kDbiCsCtrlrName, CS_VAL, offsetof(
        val_vrt, cs_attr[UPLL_IDX_CONTROLLER_ID_VRT]),
      uud::kDalUint8, 1 },
    { uudst::vrouter::kDbiCsDomainId, CS_VAL, offsetof(
        val_vrt, cs_attr[UPLL_IDX_DOMAIN_ID_VRT]),
      uud::kDalUint8, 1 },
    { uudst::vrouter::kDbiCsVrtDesc, CS_VAL, offsetof(
        val_vrt, cs_attr[UPLL_IDX_DESC_VRT]),
      uud::kDalUint8, 1 },
    { uudst::vrouter::kDbiCsDhcprelayAdminstatus, CS_VAL, offsetof(
        val_vrt, cs_attr[UPLL_IDX_DHCP_RELAY_ADMIN_STATUS_VRT]),
      uud::kDalUint8, 1 },
    { uudst::vrouter::kDbiCsRowstatus, CS_VAL, offsetof(val_vrt, cs_row_status),
      uud::kDalUint8, 1 },
    { uudst::vrouter::kDbiVrtFlags, CK_VAL, offsetof(key_user_data_t, flags),
      uud::kDalUint8, 1 } };

BindInfo VrtMoMgr::vrt_rename_bind_info[] = {
    { uudst::vnode_rename::kDbiUncVtnName, CFG_KEY, offsetof(key_vrt,
                                                             vtn_key.vtn_name),
      uud::kDalChar, 32 },
    { uudst::vnode_rename::kDbiUncvnodeName, CFG_KEY, offsetof(key_vrt,
                                                               vrouter_name),
      uud::kDalChar, 32 },
    { uudst::vnode_rename::kDbiCtrlrName, CK_VAL, offsetof(key_user_data_t,
                                                           ctrlr_id),
      uud::kDalChar, 32 },
    { uudst::vnode_rename::kDbiDomainId, CK_VAL, offsetof(key_user_data_t,
                                                          domain_id),
      uud::kDalChar, 32 },
    { uudst::vnode_rename::kDbiCtrlrVtnName, CFG_VAL, offsetof(val_rename_vnode,
                                                               ctrlr_vtn_name),
      uud::kDalChar, 32 },
    { uudst::vnode_rename::kDbiCtrlrVnodeName, CFG_VAL, offsetof(
        val_rename_vnode, ctrlr_vnode_name),
      uud::kDalChar, 32 } };

unc_key_type_t VrtMoMgr::vrt_child[] = { UNC_KT_VRT_IPROUTE,
                                         UNC_KT_DHCPRELAY_SERVER,
                                         UNC_KT_DHCPRELAY_IF, UNC_KT_VRT_IF };

BindInfo VrtMoMgr::key_vrt_maintbl_update_bind_info[] = {
    { uudst::vrouter::kDbiVtnName, CFG_MATCH_KEY, offsetof(key_vrt_t,
                                                           vtn_key.vtn_name),
      uud::kDalChar, kMaxLenVtnName + 1 },
    { uudst::vrouter::kDbiVrtName, CFG_MATCH_KEY, offsetof(key_vrt_t,
                                                           vrouter_name),
      uud::kDalChar, kMaxLenVnodeName + 1 },
    { uudst::vrouter::kDbiVtnName, CFG_INPUT_KEY, offsetof(
        key_rename_vnode_info_t, new_unc_vtn_name),
      uud::kDalChar, kMaxLenVtnName + 1 },
    { uudst::vrouter::kDbiVrtName, CFG_INPUT_KEY, offsetof(
        key_rename_vnode_info_t, new_unc_vnode_name),
      uud::kDalChar, kMaxLenVnodeName + 1 },
    { uudst::vrouter::kDbiVrtFlags, CK_VAL, offsetof(key_user_data_t,
                                                            flags),
      uud::kDalUint8, 1 } };

BindInfo VrtMoMgr::key_vrt_renametbl_update_bind_info[] = {
    { uudst::vnode_rename::kDbiUncVtnName, CFG_MATCH_KEY, offsetof(
        key_vrt_t, vtn_key.vtn_name),
      uud::kDalChar, kMaxLenVtnName + 1 },
    { uudst::vnode_rename::kDbiUncvnodeName, CFG_MATCH_KEY, offsetof(
        key_vrt_t, vrouter_name),
      uud::kDalChar, kMaxLenVnodeName + 1 },
    { uudst::vnode_rename::kDbiUncVtnName, CFG_INPUT_KEY, offsetof(
        key_rename_vnode_info_t, new_unc_vtn_name),
      uud::kDalChar, kMaxLenVtnName + 1 },
    { uudst::vnode_rename::kDbiUncvnodeName, CFG_INPUT_KEY, offsetof(
        key_rename_vnode_info_t, new_unc_vnode_name),
      uud::kDalChar, kMaxLenVnodeName + 1 }, };

/*
 * Based on the key type the bind info will pass
 **/

bool VrtMoMgr::GetRenameKeyBindInfo(unc_key_type_t key_type, BindInfo *&binfo,
                                    int &nattr, MoMgrTables tbl) {
  /* Main Table or rename table only update */
  if (MAINTBL == tbl) {
    nattr = NUM_KEY_MAIN_TBL_;
    binfo = key_vrt_maintbl_update_bind_info;
  } else if (RENAMETBL == tbl) {
    nattr = NUM_KEY_RENAME_TBL_;
    binfo = key_vrt_renametbl_update_bind_info;
  } else {
    UPLL_LOG_DEBUG("Invalid Table ");
    return PFC_FALSE;
  }
 
  return PFC_TRUE;
}
VrtMoMgr::VrtMoMgr() {
  UPLL_FUNC_TRACE;
  ntable = MAX_MOMGR_TBLS;
  table = new Table *[ntable];
  table[MAINTBL] = new Table(uudst::kDbiVrtTbl, UNC_KT_VROUTER, vrt_bind_info,
                         IpctSt::kIpcStKeyVrt, IpctSt::kIpcStValVrt,
                         uudst::vrouter::kDbiVrtNumCols);
  table[RENAMETBL] = new Table(uudst::kDbiVNodeRenameTbl, UNC_KT_VROUTER,
                  vrt_rename_bind_info, IpctSt::kIpcInvalidStNum,
                  IpctSt::kIpcInvalidStNum,
                  uudst::vnode_rename::kDbiVnodeRenameNumCols);
  table[CTRLRTBL] = NULL;
  nchild = sizeof(vrt_child) / sizeof(*vrt_child);
  child = vrt_child;
}

bool VrtMoMgr::IsValidKey(void *key, uint64_t index) {
  UPLL_FUNC_TRACE;
  key_vrt *vrt_key = reinterpret_cast<key_vrt *>(key);
  bool ret_val = UPLL_RC_SUCCESS;
  switch (index) {
    case uudst::vrouter::kDbiVtnName:
    case uudst::vnode_rename::kDbiUncVtnName:
      ret_val = ValidateKey(reinterpret_cast<char *>(vrt_key->vtn_key.vtn_name),
                            kMinLenVtnName, kMaxLenVtnName);
      if (ret_val != UPLL_RC_SUCCESS) {
        UPLL_LOG_INFO("VTN Name is not valid(%d)", ret_val);
        return false;
      }
      break;
    case uudst::vrouter::kDbiVrtName:
    case uudst::vnode_rename::kDbiUncvnodeName:
      ret_val = ValidateKey(reinterpret_cast<char *>(vrt_key->vrouter_name),
                            kMinLenVnodeName, kMaxLenVnodeName);
      if (ret_val != UPLL_RC_SUCCESS) {
        UPLL_LOG_INFO("VRouter Name is not valid(%d)", ret_val);
        return false;
      }
      break;
    default:
      UPLL_LOG_INFO("Invalid key Index");
      return false;
      break;
  }
  return true;
}

upll_rc_t VrtMoMgr::GetChildConfigKey(ConfigKeyVal *&okey,
                                      ConfigKeyVal *parent_key) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  key_vrt *vrt_key = reinterpret_cast<key_vrt_t *>
                       (ConfigKeyVal::Malloc(sizeof(key_vrt_t)));
  void *pkey;
  if (parent_key == NULL) {
    okey = new ConfigKeyVal(UNC_KT_VROUTER, IpctSt::kIpcStKeyVrt, vrt_key,
                            NULL);
    return UPLL_RC_SUCCESS;
  } else {
     pkey = parent_key->get_key();
  }
  if (!pkey) {
    free(vrt_key);
    return UPLL_RC_ERR_GENERIC;
  }

  /* presumes MoMgrs receive only supported keytypes */
  switch (parent_key->get_key_type()) {
    case UNC_KT_VROUTER:
      uuu::upll_strncpy(vrt_key->vrouter_name,
             reinterpret_cast<key_vrt *>(pkey)->vrouter_name,
             (kMaxLenVnodeName+1));
      uuu::upll_strncpy(vrt_key->vtn_key.vtn_name,
             reinterpret_cast<key_vtn *>(pkey)->vtn_name,
             (kMaxLenVtnName+1) );
      break;
    case UNC_KT_VTN:
    default:
      uuu::upll_strncpy(vrt_key->vtn_key.vtn_name,
             reinterpret_cast<key_vtn *>(pkey)->vtn_name,
              (kMaxLenVtnName+1));
      *(vrt_key->vrouter_name) = *"";
  }
//  cout << "GetChildConfigKey " << vrt_key->vtn_key.vtn_name << " ";
//  cout << vrt_key->vrouter_name << "";
  okey = new ConfigKeyVal(UNC_KT_VROUTER, IpctSt::kIpcStKeyVrt, vrt_key, NULL);
  if (okey == NULL)
    result_code = UPLL_RC_ERR_GENERIC;
  else
    SET_USER_DATA(okey, parent_key);
  return result_code;
}

upll_rc_t VrtMoMgr::GetParentConfigKey(ConfigKeyVal *&okey,
                                       ConfigKeyVal *ikey) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  if (!ikey) {
    UPLL_LOG_DEBUG("ikey is NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  unc_key_type_t ikey_type = ikey->get_key_type();

  if (ikey_type != UNC_KT_VROUTER) return UPLL_RC_ERR_GENERIC;
  void *pkey = ikey->get_key();
  if (!pkey) return UPLL_RC_ERR_GENERIC;
  key_vtn *vtn_key = reinterpret_cast<key_vtn *>
      (ConfigKeyVal::Malloc(sizeof(key_vtn)));
  uuu::upll_strncpy(vtn_key->vtn_name,
         reinterpret_cast<key_vrt *>(pkey)->vtn_key.vtn_name,
         (kMaxLenVtnName+1));
  okey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, vtn_key, NULL);
  if (okey == NULL)
      result_code = UPLL_RC_ERR_GENERIC;
  else
      SET_USER_DATA(okey, ikey);

  return result_code;
}

upll_rc_t VrtMoMgr::AllocVal(ConfigVal *&ck_val,
                             upll_keytype_datatype_t dt_type, MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  void *val;  // , *nxt_val;
//   ConfigVal *ck_nxtval;

  if (ck_val != NULL) return UPLL_RC_ERR_GENERIC;
  switch (tbl) {
    case MAINTBL:
      val = reinterpret_cast<void *>(ConfigKeyVal::Malloc(sizeof(val_vrt)));
      ck_val = new ConfigVal(IpctSt::kIpcStValVrt, val);
      if (dt_type == UPLL_DT_STATE) {
        val = reinterpret_cast<void *>
            (ConfigKeyVal::Malloc(sizeof(val_db_vrt_st)));
        ConfigVal *ck_nxtval = new ConfigVal(IpctSt::kIpcStValVrtSt, val);
        ck_val->AppendCfgVal(ck_nxtval);
      }
      break;
    case RENAMETBL:
      val = reinterpret_cast<void *>
          (ConfigKeyVal::Malloc(sizeof(val_rename_vnode)));
      ck_val = new ConfigVal(IpctSt::kIpcInvalidStNum, val);
      break;
    default:
      val = NULL;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VrtMoMgr::DupConfigKeyVal(ConfigKeyVal *&okey, ConfigKeyVal *&req,
                                    MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  if (req == NULL) return UPLL_RC_ERR_GENERIC;
  if (okey != NULL) return UPLL_RC_ERR_GENERIC;
  if (req->get_key_type() != UNC_KT_VROUTER) return UPLL_RC_ERR_GENERIC;
  ConfigVal *tmp1 = NULL, *tmp = (req)->get_cfg_val();

  if (tmp) {
    if (tbl == MAINTBL) {
      val_vrt *ival = reinterpret_cast<val_vrt *>(GetVal(req));
      val_vrt *vrt_val = reinterpret_cast<val_vrt *>
          (ConfigKeyVal::Malloc(sizeof(val_vrt)));
      memcpy(vrt_val, ival, sizeof(val_vrt));
      tmp1 = new ConfigVal(IpctSt::kIpcStValVrt, vrt_val);
    } else if (tbl == RENAMETBL) {
      void *rename_val;
      ConfigVal *ck_v = req->get_cfg_val();
      if (ck_v->get_st_num() == IpctSt::kIpcInvalidStNum) {
        val_rename_vnode *ival = reinterpret_cast<val_rename_vnode *>
                                                     (GetVal(req));
        rename_val = reinterpret_cast<void *>
            (ConfigKeyVal::Malloc(sizeof(val_rename_vnode)));
        memcpy(rename_val, ival, sizeof(val_rename_vnode));
      } else {
        val_rename_vrt *ival = reinterpret_cast<val_rename_vrt *> (GetVal(req));
        rename_val = reinterpret_cast<void *>
            (ConfigKeyVal::Malloc(sizeof(val_rename_vrt)));
        memcpy(rename_val, ival, sizeof(val_rename_vrt));
      }
      tmp1 = new ConfigVal(IpctSt::kIpcStValRenameVrt, rename_val);
    }
    tmp = tmp->get_next_cfg_val();
  };
  if (tmp) {
    if (tbl == MAINTBL) {
      val_db_vrt_st *ival = reinterpret_cast<val_db_vrt_st *>(tmp->get_val());
      val_db_vrt_st *val_vrt = reinterpret_cast<val_db_vrt_st *>
          (ConfigKeyVal::Malloc(sizeof(val_db_vrt_st)));
      memcpy(val_vrt, ival, sizeof(val_db_vrt_st));
      ConfigVal *tmp2 = new ConfigVal(IpctSt::kIpcStValVrtSt, val_vrt);
      tmp1->AppendCfgVal(tmp2);
    }
  };
  void *tkey = (req != NULL) ? (req)->get_key() : NULL;
  key_vrt *ikey = reinterpret_cast<key_vrt *>(tkey);
  key_vrt *vrt_key = reinterpret_cast<key_vrt *>
      (ConfigKeyVal::Malloc(sizeof(key_vrt)));
  memcpy(vrt_key, ikey, sizeof(key_vrt));
  okey = new ConfigKeyVal(UNC_KT_VROUTER, IpctSt::kIpcStKeyVrt, vrt_key, tmp1);
  if (okey) {
    SET_USER_DATA(okey, req);
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VrtMoMgr::GetRenamedUncKey(ConfigKeyVal *ikey,
                                     upll_keytype_datatype_t dt_type,
                                     DalDmlIntf *dmi, uint8_t *ctrlr_id) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *unc_key = NULL;
  if (ikey == NULL) {
    UPLL_LOG_DEBUG("ConfigKeyVal is NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  if (ctrlr_id == NULL) return UPLL_RC_ERR_GENERIC;
  val_rename_vnode *rename_vnode = reinterpret_cast<val_rename_vnode *>
                                (ConfigKeyVal::Malloc(sizeof(val_rename_vnode)));

  key_vrt *ctrlr_key = reinterpret_cast<key_vrt *>(ikey->get_key());

  uuu::upll_strncpy(rename_vnode->ctrlr_vtn_name,
                    ctrlr_key->vtn_key.vtn_name,
                    (kMaxLenVtnName+1));
  uuu::upll_strncpy(rename_vnode->ctrlr_vnode_name,
                    ctrlr_key->vrouter_name,
                    (kMaxLenVnodeName+1) );

  result_code = GetChildConfigKey(unc_key, NULL);

  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetChildConfigKey Failed result_code %d", result_code);
    /* Addressed RESOURCE_LEAK */
    FREE_IF_NOT_NULL(rename_vnode);
    return UPLL_RC_ERR_GENERIC;
  }
  SET_USER_DATA_CTRLR(unc_key, ctrlr_id);

  rename_vnode->valid[UPLL_CTRLR_VTN_NAME_VALID] = UNC_VF_VALID;
  rename_vnode->valid[UPLL_CTRLR_VNODE_NAME_VALID] = UNC_VF_VALID;

  unc_key->AppendCfgVal(IpctSt::kIpcStValRenameVtn, rename_vnode);

  DbSubOp dbop = {kOpReadSingle, kOpMatchCtrlr, kOpInOutNone };
  result_code = ReadConfigDB(unc_key, dt_type, UNC_OP_READ, dbop, dmi,
                                       RENAMETBL);
  if (result_code == UPLL_RC_SUCCESS) {
 //   uint8_t rename = 0x00;
    key_vrt *vrt_key = reinterpret_cast<key_vrt *>(unc_key->get_key());
    if (strcmp((const char *) ctrlr_key->vtn_key.vtn_name,
               (const char *) vrt_key->vtn_key.vtn_name)) {
      uuu::upll_strncpy(ctrlr_key->vtn_key.vtn_name,
             vrt_key->vtn_key.vtn_name, (kMaxLenVtnName+1));
//      rename |= VTN_RENAME;
    }
    if (strcmp(reinterpret_cast<const char *>(ctrlr_key->vrouter_name),
              reinterpret_cast<const char *>(vrt_key->vrouter_name))) {
      uuu::upll_strncpy(ctrlr_key->vrouter_name,
              vrt_key->vrouter_name, (kMaxLenVnodeName+1));
  //    rename |= VN_RENAME;
    }
    SET_USER_DATA(ikey, unc_key);
  }
  delete unc_key;
  return result_code;
}

upll_rc_t VrtMoMgr::GetRenamedControllerKey(ConfigKeyVal *ikey,
                                            upll_keytype_datatype_t dt_type,
                                            DalDmlIntf *dmi,
                                            controller_domain *ctrlr_dom) {
  UPLL_FUNC_TRACE;
  uint8_t rename = 0;
  ConfigKeyVal *okey = NULL;
  upll_rc_t result_code;
#if 0
  ConfigKeyVal *dup_key = NULL;
  ConfigVal *cval = ikey->get_cfg_val();
  val_vrt_t *val = reinterpret_cast<val_vrt_t*>(GetVal(ikey));
  if (cval && cval->get_st_num() == IpctSt::kIpcStValVrt &&
      val && val->valid[UPLL_IDX_CONTROLLER_ID_VRT] == UNC_VF_VALID &&
      val->valid[UPLL_IDX_DOMAIN_ID_VRT] == UNC_VF_VALID) {
    GET_USER_DATA_FLAGS(ikey, rename);
  } else {
    result_code = GetChildConfigKey(dup_key, ikey);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("GetChildConfigKey Failed %d",result_code);
      return result_code;
    }
#endif
    result_code = IsRenamed(ikey, dt_type, dmi, rename);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Returning %d",result_code);
      return result_code;
    }
    if (!ctrlr_dom->ctrlr ||
      !strlen(reinterpret_cast<const char *>(ctrlr_dom->ctrlr)) ||
      !ctrlr_dom->domain ||
      !strlen(reinterpret_cast<const char *>(ctrlr_dom->domain))) {
      GET_USER_DATA_CTRLR_DOMAIN(ikey, *ctrlr_dom);
    }
#if 0
    cval = dup_key->get_cfg_val();
    if (!cval) {
      UPLL_LOG_DEBUG("Val is NULL");
      return UPLL_RC_ERR_GENERIC;
    }
    result_code = GetControllerDomainId(dup_key, ctrlr_dom);
    SET_USER_DATA(ikey, dup_key);
    GET_USER_DATA_CTRLR_DOMAIN(ikey, *ctrlr_dom);
    if (dup_key)
     delete dup_key;
  }
  if (!ctrlr_dom->ctrlr ||
      !strlen(reinterpret_cast<const char *>(ctrlr_dom->ctrlr)) ||
      !ctrlr_dom->domain ||
      !strlen(reinterpret_cast<const char *>(ctrlr_dom->domain))) {
    result_code = GetControllerDomainId(ikey, ctrlr_dom);
  }
  if (!ctrlr_dom->ctrlr ||
      !strlen(reinterpret_cast<const char *>(ctrlr_dom->ctrlr)) ||
      !ctrlr_dom->domain ||
      !strlen(reinterpret_cast<const char *>(ctrlr_dom->domain))) {
      UPLL_LOG_DEBUG("Invalid Ctrlr/domain");
      return UPLL_RC_ERR_GENERIC;
  }
#endif
  if (rename == 0) return UPLL_RC_SUCCESS;
  result_code = GetChildConfigKey(okey, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
     UPLL_LOG_DEBUG("Returning error %d",result_code);
     return result_code;
  }
  GET_USER_DATA_CTRLR_DOMAIN(ikey, *ctrlr_dom);
  if (!ctrlr_dom->ctrlr || !ctrlr_dom->domain) return UPLL_RC_ERR_GENERIC;
  SET_USER_DATA_CTRLR_DOMAIN(okey, *ctrlr_dom);
  DbSubOp dbop = { kOpReadSingle, kOpMatchCtrlr | kOpMatchDomain,
                   kOpInOutFlag };
  result_code = ReadConfigDB(okey, dt_type, UNC_OP_READ, dbop, dmi,
                                       RENAMETBL);
  if (result_code != UPLL_RC_SUCCESS) return result_code;
  val_rename_vnode *rename_val = reinterpret_cast<val_rename_vnode *>
                                  (GetVal(okey));
  if (!rename_val) return UPLL_RC_ERR_GENERIC;
  key_vrt *ctrlr_key = reinterpret_cast<key_vrt *>(ikey->get_key());
  if (!ctrlr_key) {
    free(rename_val);
    return UPLL_RC_ERR_GENERIC;
  }
  memset(ctrlr_key, 0, sizeof(key_vrt));
  if (rename & 0x01) { /* vtn renamed */
    uuu::upll_strncpy(ctrlr_key->vtn_key.vtn_name,
                      rename_val->ctrlr_vtn_name, (kMaxLenVtnName+1));
  }
  if (rename & 0x10) { /* vnode renamed */
    uuu::upll_strncpy(ctrlr_key->vrouter_name,
           rename_val->ctrlr_vnode_name, (kMaxLenVnodeName+1));
  }
  SET_USER_DATA_FLAGS(okey, rename);
  delete okey;
  return UPLL_RC_SUCCESS;
}

upll_rc_t VrtMoMgr::UpdateConfigStatus(ConfigKeyVal *vrt_key,
                                       unc_keytype_operation_t op,
                                       uint32_t driver_result,
                                       ConfigKeyVal *upd_key,
                                       DalDmlIntf *dmi,
                                       ConfigKeyVal *ctrlr_key) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  val_vrt *vrt_val;
  val_db_vrt_st *val_db_vrtst;
  val_vrt_st *val_vrtst;
  if (vrt_key == NULL) {
    UPLL_LOG_DEBUG("ConfigKeyVal is NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  int nloop = sizeof(vrt_val->valid) / sizeof(vrt_val->valid[0]);
  unc_keytype_configstatus_t cs_status =
      (driver_result == 0) ? UNC_CS_APPLIED : UNC_CS_NOT_APPLIED;
  vrt_val = reinterpret_cast<val_vrt *> (GetVal(vrt_key));
  if (vrt_val == NULL) return UPLL_RC_ERR_GENERIC;
  vrt_val->cs_row_status = cs_status;
  if (op == UNC_OP_CREATE) {
    val_db_vrtst = reinterpret_cast<val_db_vrt_st *>
                   (ConfigKeyVal::Malloc(sizeof(val_db_vrt_st)));
    val_vrtst = &val_db_vrtst->vrt_val_st;
    val_vrtst->oper_status = UPLL_OPER_STATUS_UNINIT;
    val_vrtst->valid[UPLL_IDX_OPER_STATUS_VRTS] = UNC_VF_VALID;
    val_db_vrtst->down_count  = 0;
    val_db_vrtst->fault_count = 0;
    vrt_key->AppendCfgVal(IpctSt::kIpcStValVrtSt, val_db_vrtst);
  } else if (op == UNC_OP_UPDATE) {
    /* compare values */
    void *val =  reinterpret_cast<void *>(vrt_val);
    CompareValidValue(val, GetVal(upd_key), true);
  } else {
    return UPLL_RC_ERR_GENERIC;
  }
  for (int loop = 0; loop < nloop; ++loop) {
    if (vrt_val->valid[loop] == UNC_VF_NOT_SOPPORTED) {
       vrt_val->cs_attr[loop] = UNC_CS_NOT_SUPPORTED;
    } else if ((UNC_VF_VALID == (uint8_t) vrt_val->valid[loop])
            || (UNC_VF_VALID_NO_VALUE == (uint8_t) vrt_val->valid[loop])) {
       vrt_val->cs_attr[loop] = (uint8_t) vrt_val->cs_row_status;
    } 
  }
  return result_code;
}

/*
uint8_t* VrtMoMgr::GetControllerId(ConfigKeyVal *ikey) {
  UPLL_FUNC_TRACE;
  if (!ikey)
     return NULL;
  val_vrt * temp_vrt = reinterpret_cast<val_vrt *>(GetVal(ikey));
  if (temp_vrt->valid[UPLL_IDX_CONTROLLER_ID_VRT] != UNC_VF_VALID
      || !strlen(reinterpret_cast<const char*>(temp_vrt->controller_id)))
         return NULL;
  return temp_vrt->controller_id;
}
*/

upll_rc_t VrtMoMgr::GetControllerDomainId(ConfigKeyVal *ikey,
                                          controller_domain *ctrlr_dom) {
  UPLL_FUNC_TRACE;
  if (!ikey || !ctrlr_dom) {
    UPLL_LOG_INFO("Illegal parameter");
    return UPLL_RC_ERR_GENERIC;
  }
  val_vrt * temp_vrt = reinterpret_cast<val_vrt *> (GetVal(ikey));
  if (temp_vrt == NULL
      || temp_vrt->valid[UPLL_IDX_CONTROLLER_ID_VRT] != UNC_VF_VALID
      || !strlen(reinterpret_cast<char*>(temp_vrt->controller_id))) {
    ctrlr_dom->ctrlr = NULL;
    UPLL_LOG_DEBUG("Ctrlr null");
  } else {
    SET_USER_DATA_CTRLR(ikey, temp_vrt->controller_id);
  }
  if (temp_vrt == NULL
      || temp_vrt->valid[UPLL_IDX_DOMAIN_ID_VRT] != UNC_VF_VALID
      || !strlen(reinterpret_cast<char*>(
         temp_vrt->domain_id))) {
    ctrlr_dom->domain = NULL;
    UPLL_LOG_DEBUG("Ctrlr null");
  } else {
    SET_USER_DATA_DOMAIN(ikey, temp_vrt->domain_id);
    GET_USER_DATA_CTRLR_DOMAIN(ikey, *ctrlr_dom);
  }
  UPLL_LOG_DEBUG("ctrlr_dom %s %s",ctrlr_dom->ctrlr, ctrlr_dom->domain);
  return UPLL_RC_SUCCESS;
}


upll_rc_t VrtMoMgr::GetVnodeName(ConfigKeyVal *ikey, uint8_t *&vtn_name,
                                 uint8_t *&vnode_name) {
  UPLL_FUNC_TRACE;
  if (ikey == NULL) {
    UPLL_LOG_DEBUG("ConfigKeyVal is NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  key_vrt_t *vrt_key = reinterpret_cast<key_vrt_t *>(ikey->get_key());
  if (vrt_key == NULL) return UPLL_RC_ERR_GENERIC;
  vtn_name = vrt_key->vtn_key.vtn_name;
  vnode_name = vrt_key->vrouter_name;
  return UPLL_RC_SUCCESS;
}

/* Pure Virtual functions from MoMgrImpl */

upll_rc_t VrtMoMgr::SwapKeyVal(ConfigKeyVal *ikey, ConfigKeyVal *&okey,
                               DalDmlIntf *dmi, uint8_t *ctrlr,
                               bool &no_rename) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  okey = NULL;
  if (!ikey || !(ikey->get_key())) return UPLL_RC_ERR_GENERIC;

  if (ikey->get_key_type() != UNC_KT_VROUTER) return UPLL_RC_ERR_BAD_REQUEST;

  val_rename_vrt_t *tval =
      (ikey != NULL) ? reinterpret_cast<val_rename_vrt_t *>
      (GetVal(ikey)) : NULL;
  if (!tval) {
     UPLL_LOG_DEBUG("tval is null");
     return UPLL_RC_ERR_GENERIC;
  }

  /* The PFC Name and New Name should not be equal */
  if (!strcmp(reinterpret_cast<char *>(tval->new_name),
      reinterpret_cast<char *>(reinterpret_cast<key_vrt_t *>
      (ikey->get_key())->vrouter_name)))
    return UPLL_RC_ERR_GENERIC;

  key_vrt_t *key_vrt = reinterpret_cast<key_vrt_t *>
                  (ConfigKeyVal::Malloc(sizeof(key_vrt_t)));
  if (tval->valid[UPLL_IDX_NEW_NAME_RVRT] == UNC_VF_VALID_NO_VALUE) {
    uuu::upll_strncpy(key_vrt->vrouter_name,
            (static_cast<key_vrt_t *>(ikey->get_key())->vrouter_name),
            (kMaxLenVnodeName+1));
    no_rename = true;
  } else {
    if ((reinterpret_cast<val_rename_vrt_t *>(tval)->
        valid[UPLL_IDX_NEW_NAME_RVRT] == UNC_VF_VALID)) {
      // checking the string is empty or not
      if (!strlen(reinterpret_cast<char *>(static_cast<val_rename_vrt_t *>
                                                       (tval)->new_name))) {
        free(key_vrt);
        return UPLL_RC_ERR_GENERIC;
      }
      uuu::upll_strncpy(key_vrt->vrouter_name,
           (static_cast<val_rename_vrt_t *>(GetVal(ikey)))->new_name,
            (kMaxLenVnodeName+1));
    } else {
      free(key_vrt);
      return UPLL_RC_ERR_GENERIC;
    }
  }
  // Checking the vrouter parent is renamed get the UNC name
  ConfigKeyVal *pkey = NULL;
  result_code = GetParentConfigKey(pkey, ikey);
  if (UPLL_RC_SUCCESS != result_code) {
    free(key_vrt);
    return result_code;
  }
  MoMgrImpl *mgr = reinterpret_cast<MoMgrImpl *>(const_cast<MoManager*>
                        (GetMoManager(UNC_KT_VTN)));
  result_code = mgr->GetRenamedUncKey(pkey, UPLL_DT_IMPORT, dmi, ctrlr);
  if (UPLL_RC_SUCCESS != result_code 
      && UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
    UPLL_LOG_DEBUG("GetRenamedUncKey failed with result_code %d",
                                                      result_code);
    DELETE_IF_NOT_NULL(pkey);
    free(key_vrt);
    return result_code;
  }
  // use the UNC VTN name if PFC VTN name is renamed;
  if (!strlen(reinterpret_cast<char *>(reinterpret_cast<key_vtn_t *>
                                     (pkey->get_key())->vtn_name))) {
     DELETE_IF_NOT_NULL(pkey);
     free(key_vrt);
     return UPLL_RC_ERR_GENERIC;
  }
    uuu::upll_strncpy(key_vrt->vtn_key.vtn_name,
           reinterpret_cast<key_vtn_t *>(pkey->get_key())->vtn_name,
           (kMaxLenVtnName+1));
  okey = new ConfigKeyVal(UNC_KT_VROUTER, IpctSt::kIpcStKeyVrt, key_vrt, NULL);

  delete pkey;
  if (!okey) {
    free(key_vrt);
    return UPLL_RC_ERR_GENERIC;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VrtMoMgr::UpdateAuditConfigStatus(
                                unc_keytype_configstatus_t cs_status,
                                uuc::UpdateCtrlrPhase phase,
                                ConfigKeyVal *&ckv_running) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  val_vrt_t *val;
  val = (ckv_running != NULL) ? reinterpret_cast<val_vrt_t *>
                                               (GetVal(ckv_running)) : NULL;
  if (NULL == val) {
    return UPLL_RC_ERR_GENERIC;
  }
  if (uuc::kUpllUcpCreate == phase) val->cs_row_status = cs_status;
  for (unsigned int loop = 0; loop < sizeof(val->valid) / sizeof(uint8_t);
      ++loop) {
    if ((cs_status == UNC_CS_INVALID && UNC_VF_VALID == val->valid[loop]) ||
         cs_status == UNC_CS_APPLIED)
          val->cs_attr[loop] = cs_status;
  }
  return result_code;
}

upll_rc_t VrtMoMgr::MergeValidate(unc_key_type_t keytype, const char *ctrlr_id,
                                  ConfigKeyVal *ikey, DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  DbSubOp dbop = { kOpReadMultiple, kOpMatchNone, kOpInOutNone };
  if (!ikey || !(ikey->get_key())){
    UPLL_LOG_DEBUG(" Input is NULL ");
    return UPLL_RC_ERR_GENERIC;
  }
  /*
   * Here getting FULL Key (VTN & VBR Name )
   */
  result_code = ReadConfigDB(ikey, UPLL_DT_IMPORT, UNC_OP_READ, dbop, dmi,
                             MAINTBL);
  if (UPLL_RC_SUCCESS != result_code) return result_code;
  /* checks the vnode name present in the running vnode under the
   * same vtn
   */
  while (ikey) {
    /* Same Name should not present in the vnodes in running*/
    result_code = VnodeChecks(ikey, UPLL_DT_RUNNING, dmi);

    if (UPLL_RC_ERR_INSTANCE_EXISTS == result_code){
      UPLL_LOG_DEBUG("VRouter Name Conflict %d", result_code);
      return UPLL_RC_ERR_MERGE_CONFLICT;
    }
    /* Any other DB error */
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("Vnode Checks Failed %d", result_code);
      return result_code;
    }
    ikey = ikey->get_next_cfg_key_val();
  }
  return result_code;
}

upll_rc_t VrtMoMgr::CopyToConfigKey(ConfigKeyVal *&okey,
                                    ConfigKeyVal *ikey) {
  UPLL_FUNC_TRACE;
  if (!ikey || !(ikey->get_key())) return UPLL_RC_ERR_GENERIC;

  upll_rc_t result_code = UPLL_RC_SUCCESS;

  key_rename_vnode_info *key_rename = reinterpret_cast<key_rename_vnode_info *>
                                        (ikey->get_key());
  key_vrt_t * key_vrt = reinterpret_cast<key_vrt_t *>
      (ConfigKeyVal::Malloc(sizeof(key_vrt_t)));

    if (!strlen(reinterpret_cast<char *>(key_rename->old_unc_vtn_name))) {
     /* Addressed RESOURCE_LEAK */
      FREE_IF_NOT_NULL(key_vrt);
      return UPLL_RC_ERR_GENERIC;
    }
    uuu::upll_strncpy(key_vrt->vtn_key.vtn_name,
         key_rename->old_unc_vtn_name, (kMaxLenVtnName+1));
    if (ikey->get_key_type() == table[MAINTBL]->get_key_type()) {
      if (!strlen(reinterpret_cast<char *>(key_rename->old_unc_vnode_name))) {
      /* Addressed RESOURCE_LEAK */
      FREE_IF_NOT_NULL(key_vrt);
      return UPLL_RC_ERR_GENERIC;
    }
      uuu::upll_strncpy(key_vrt->vrouter_name,
           key_rename->old_unc_vnode_name, (kMaxLenVnodeName+1));
    } 
  okey = new ConfigKeyVal(UNC_KT_VROUTER, IpctSt::kIpcStKeyVrt, key_vrt, NULL);
  if (!okey) {
    free(key_vrt);
    return UPLL_RC_ERR_GENERIC;
  }
  return result_code;
}

upll_rc_t VrtMoMgr::GetRenameInfo(ConfigKeyVal *ikey, 
                                  ConfigKeyVal *okey,
                                  ConfigKeyVal *&rename_info,
                                  DalDmlIntf *dmi,
                                  const char *ctrlr_id,
                                  bool &renamed) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  if (!okey || !ikey || rename_info != NULL)
    return UPLL_RC_ERR_GENERIC;

  key_vrt_t * vrt_key = NULL;
  key_rename_vnode_info *vrt_rename_info =
    reinterpret_cast<key_rename_vnode_info *>
        (ConfigKeyVal::Malloc(sizeof(key_rename_vnode_info)));

  vrt_key = reinterpret_cast<key_vrt_t *> (ikey->get_key());
  if (!vrt_key) {
    /* Addressed RESOURCE_LEAK */
    FREE_IF_NOT_NULL(vrt_rename_info);
    return UPLL_RC_ERR_GENERIC;
  }
  if (renamed) {
    if (!strlen(reinterpret_cast<char *>(reinterpret_cast<val_rename_vnode_t *>
               (GetVal(ikey))->ctrlr_vnode_name))) {
      FREE_IF_NOT_NULL(vrt_rename_info);
      return UPLL_RC_ERR_GENERIC;
    }
    uuu::upll_strncpy(vrt_rename_info->ctrlr_vnode_name,
        reinterpret_cast<val_rename_vnode_t *>(GetVal(ikey))->ctrlr_vnode_name,
        (kMaxLenVnodeName+1));
    if (!strlen(reinterpret_cast<char *>(reinterpret_cast<val_rename_vnode_t *>
             (GetVal(ikey))->ctrlr_vtn_name))) {
      FREE_IF_NOT_NULL(vrt_rename_info);
      return UPLL_RC_ERR_GENERIC;
    }
    uuu::upll_strncpy(vrt_rename_info->ctrlr_vtn_name,
          reinterpret_cast<val_rename_vnode_t *>(GetVal(ikey))->ctrlr_vtn_name,
             (kMaxLenVtnName+1));
  } else {
    if (!strlen(reinterpret_cast<char *>(vrt_key->vrouter_name))) {
      FREE_IF_NOT_NULL(vrt_rename_info);
       return UPLL_RC_ERR_GENERIC;
    }
    uuu::upll_strncpy(vrt_rename_info->ctrlr_vnode_name,
            vrt_key->vrouter_name, (kMaxLenVnodeName+1));
    if (!strlen(reinterpret_cast<char *>(vrt_key->vtn_key.vtn_name))) {
      FREE_IF_NOT_NULL(vrt_rename_info);
      return UPLL_RC_ERR_GENERIC;
    }
    uuu::upll_strncpy(vrt_rename_info->ctrlr_vtn_name,
           vrt_key->vtn_key.vtn_name, (kMaxLenVtnName+1));
  }

  if (!strlen(reinterpret_cast<char *>(vrt_key->vrouter_name))) {
      FREE_IF_NOT_NULL(vrt_rename_info);
       return UPLL_RC_ERR_GENERIC;
  }
  uuu::upll_strncpy(vrt_rename_info->old_unc_vnode_name,
         vrt_key->vrouter_name, (kMaxLenVnodeName+1));
  if (!strlen(reinterpret_cast<char *>(vrt_key->vtn_key.vtn_name))) {
      FREE_IF_NOT_NULL(vrt_rename_info);
      return UPLL_RC_ERR_GENERIC;
  }
  uuu::upll_strncpy(vrt_rename_info->new_unc_vtn_name,
         vrt_key->vtn_key.vtn_name, (kMaxLenVtnName+1));
  uuu::upll_strncpy(vrt_rename_info->old_unc_vtn_name,
         vrt_key->vtn_key.vtn_name, (kMaxLenVtnName+1));

  vrt_key = reinterpret_cast<key_vrt_t *>(okey->get_key());
  if (!vrt_key) {
     /* Addressed RESOURCE_LEAK */
    FREE_IF_NOT_NULL(vrt_rename_info);
    return UPLL_RC_ERR_GENERIC;
  }
  if (!strlen(reinterpret_cast<char *>(vrt_key->vtn_key.vtn_name))) {
      FREE_IF_NOT_NULL(vrt_rename_info);
      return UPLL_RC_ERR_GENERIC;
  }
  uuu::upll_strncpy(vrt_rename_info->new_unc_vnode_name,
         vrt_key->vrouter_name, (kMaxLenVnodeName+1));

  rename_info = new ConfigKeyVal(UNC_KT_VROUTER, IpctSt::kIpcInvalidStNum,
                                 vrt_rename_info, NULL);
  if (!rename_info) {
    free(vrt_rename_info);
    return UPLL_RC_ERR_GENERIC;
  }
  if (!renamed) {
    val_rename_vnode_t *vnode = reinterpret_cast<val_rename_vnode_t*>
                            (ConfigKeyVal::Malloc(sizeof(val_rename_vnode_t)));

    ConfigKeyVal *tmp_key = NULL;
    result_code = GetChildConfigKey(tmp_key, ikey);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("GetChildConfigKey Failed ");
      return result_code;
    }
    DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutCtrlr|kOpInOutDomain};
    result_code = ReadConfigDB(tmp_key, UPLL_DT_IMPORT, UNC_OP_READ, dbop, dmi,
                                MAINTBL);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("ReadConfigDB Failed");
      return result_code;
    }
    controller_domain ctrlr_dom;
    result_code = GetControllerDomainId(tmp_key, &ctrlr_dom);
    if (UPLL_RC_SUCCESS != result_code)
       return result_code;
    SET_USER_DATA_CTRLR_DOMAIN(okey, ctrlr_dom);

    vnode->valid[UPLL_CTRLR_VTN_NAME_VALID] = UNC_VF_VALID;
    vnode->valid[UPLL_CTRLR_VNODE_NAME_VALID] = UNC_VF_VALID;

    uuu::upll_strncpy(vnode->ctrlr_vtn_name,
           reinterpret_cast<key_vrt_t *>(ikey->get_key())->vtn_key.vtn_name,
            (kMaxLenVtnName+1));
    uuu::upll_strncpy(vnode->ctrlr_vnode_name,
           reinterpret_cast<key_vrt_t *>(ikey->get_key())->vrouter_name,
           (kMaxLenVnodeName+1));
    ConfigVal *rename_val_ = new ConfigVal(IpctSt::kIpcInvalidStNum, vnode);
    okey->SetCfgVal(rename_val_);
    dbop.readop = kOpNotRead;
    result_code = UpdateConfigDB(okey, UPLL_DT_IMPORT, UNC_OP_CREATE, dmi,
                                 &dbop, RENAMETBL);
    if (tmp_key)
      delete tmp_key;
  }
  return result_code;
}

bool VrtMoMgr::FilterAttributes(void *&val1, void *val2, bool copy_to_running,
                                unc_keytype_operation_t op) {
  UPLL_FUNC_TRACE;
  val_vrt_t *val_vrt1 = reinterpret_cast<val_vrt_t *>(val1);
  /* No need to configure description in controller. */
  val_vrt1->valid[UPLL_IDX_DESC_VRT] = UNC_VF_INVALID;
  if (op != UNC_OP_CREATE)
    return CompareValidValue(val1, val2, copy_to_running);
  return false;
}

bool VrtMoMgr::CompareValidValue(void *&val1, void *val2, bool copy_to_running) {
  UPLL_FUNC_TRACE;
  bool invalid_attr = true;
  val_vrt_t *val_vrt1 = reinterpret_cast<val_vrt_t *>(val1);
  val_vrt_t *val_vrt2 = reinterpret_cast<val_vrt_t *>(val2);
  if ((UNC_VF_VALID == val_vrt1->valid[UPLL_IDX_DESC_VRT])
        && (UNC_VF_VALID == val_vrt2->valid[UPLL_IDX_DESC_VRT]))
    if (!strcmp(reinterpret_cast<char*>(val_vrt1->vrt_description),
                 reinterpret_cast<char*>(val_vrt2->vrt_description)))
        val_vrt1->valid[UPLL_IDX_DESC_VRT] = UNC_VF_INVALID;
  for (unsigned int loop = 0;
      loop < sizeof(val_vrt1->valid) / sizeof(uint8_t); ++loop) {
    if (UNC_VF_INVALID == val_vrt1->valid[loop]
        && UNC_VF_VALID == val_vrt2->valid[loop]) 
      val_vrt1->valid[loop] = UNC_VF_VALID_NO_VALUE;
  }
  if (UNC_VF_VALID == val_vrt1->valid[UPLL_IDX_CONTROLLER_ID_VRT]
      && UNC_VF_VALID == val_vrt2->valid[UPLL_IDX_CONTROLLER_ID_VRT]) {
    if (strcmp(reinterpret_cast<char*>(val_vrt1->controller_id),
           reinterpret_cast<char*>(val_vrt2->controller_id)))
      val_vrt1->valid[UPLL_IDX_CONTROLLER_ID_VRT] = UNC_VF_INVALID;
  }
  if (UNC_VF_VALID == val_vrt1->valid[UPLL_IDX_DHCP_RELAY_ADMIN_STATUS_VRT]
      && UNC_VF_VALID
          == val_vrt2->valid[UPLL_IDX_DHCP_RELAY_ADMIN_STATUS_VRT]) {
    if (val_vrt1->dhcp_relay_admin_status == val_vrt2->dhcp_relay_admin_status)
      val_vrt1->valid[UPLL_IDX_DHCP_RELAY_ADMIN_STATUS_VRT] = UNC_VF_INVALID;
  }
  for (unsigned int loop = 0;
       loop < sizeof(val_vrt1->valid) / sizeof(val_vrt1->valid[0]);
       ++loop) {
    if ((UNC_VF_VALID == (uint8_t) val_vrt1->valid[loop]) ||
        (UNC_VF_VALID_NO_VALUE == (uint8_t) val_vrt1->valid[loop]))
        invalid_attr = false;
  }
  return invalid_attr;
}

upll_rc_t VrtMoMgr::ValidateVrtKey(key_vrt *vrt_key,
                               unc_keytype_operation_t operation) {
  UPLL_FUNC_TRACE;
  upll_rc_t ret_val = UPLL_RC_SUCCESS;

  ret_val = ValidateKey(reinterpret_cast<char *>(vrt_key->vtn_key.vtn_name),
                        kMinLenVtnName,
                        kMaxLenVtnName);
  if (ret_val != UPLL_RC_SUCCESS) {
    UPLL_LOG_INFO("Vtn Name syntax check failed."
                  "Received vtn_name - %s",
                  vrt_key->vtn_key.vtn_name);
    return UPLL_RC_ERR_CFG_SYNTAX;
  }
  if ((operation != UNC_OP_READ_SIBLING_COUNT) &&
      (operation != UNC_OP_READ_SIBLING_BEGIN)) {
    ret_val = ValidateKey(reinterpret_cast<char *>(vrt_key->vrouter_name),
                        kMinLenVnodeName,
                        kMaxLenVnodeName);
    if (ret_val != UPLL_RC_SUCCESS) {
      UPLL_LOG_INFO("vrouter name syntax check failed."
                  "Received vrouter name - %s",
                  vrt_key->vrouter_name);
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  } else {
    UPLL_LOG_TRACE("Operation is %d", operation);
    StringReset(vrt_key->vrouter_name);
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VrtMoMgr::ValidateVrtValue(val_vrt *vrt_val,
                                     uint32_t operation) {
  UPLL_FUNC_TRACE;
  upll_rc_t ret_val = UPLL_RC_SUCCESS;

  if (vrt_val->valid[UPLL_IDX_CONTROLLER_ID_VRT] == UNC_VF_VALID) {
    ret_val = ValidateKey(reinterpret_cast<char *>(vrt_val->controller_id),
                           kMinLenCtrlrId,
                           kMaxLenCtrlrId);
    if (ret_val != UPLL_RC_SUCCESS) {
      UPLL_LOG_INFO("controller_id syntax check failed."
                    "Received controller_id - %s",
                    vrt_val->controller_id);
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  } else if ((vrt_val->valid[UPLL_IDX_CONTROLLER_ID_VRT] == UNC_VF_INVALID ||
    vrt_val->valid[UPLL_IDX_CONTROLLER_ID_VRT] == UNC_VF_VALID_NO_VALUE) &&
    (operation == UNC_OP_CREATE))  {
    UPLL_LOG_DEBUG("Controller id is invalid/ valid_no_value");
    return UPLL_RC_ERR_CFG_SYNTAX;
  } 
  if (vrt_val->valid[UPLL_IDX_DOMAIN_ID_VRT] == UNC_VF_VALID) {
    ret_val = ValidateDefaultStr(reinterpret_cast<char *>(vrt_val->domain_id),
                          kMinLenDomainId,
                          kMaxLenDomainId);
    if (ret_val != UPLL_RC_SUCCESS) {
      UPLL_LOG_INFO("Domain_id syntax check failed."
                    "Received Domain_id - %s",
                    vrt_val->domain_id);
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  } else if ((vrt_val->valid[UPLL_IDX_DOMAIN_ID_VRT] == UNC_VF_INVALID ||
     vrt_val->valid[UPLL_IDX_DOMAIN_ID_VRT] == UNC_VF_VALID_NO_VALUE) &&
     (operation == UNC_OP_CREATE)) {
    UPLL_LOG_DEBUG("Domain id is invalid/ valid_no_value");
    return UPLL_RC_ERR_CFG_SYNTAX; 
  }
  if (vrt_val->valid[UPLL_IDX_DESC_VRT] == UNC_VF_VALID) {
    ret_val = ValidateDesc(reinterpret_cast<char *>(vrt_val->vrt_description),
                          kMinLenDescription,
                          kMaxLenDescription);
    if (ret_val != UPLL_RC_SUCCESS) {
      UPLL_LOG_INFO("Description syntax check failed."
                    "Received Description - %s",
                    vrt_val->vrt_description);
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  } else if (vrt_val->valid[UPLL_IDX_DESC_VRT] == UNC_VF_VALID_NO_VALUE
      && (operation == UNC_OP_CREATE || operation == UNC_OP_UPDATE)) {
    uuu::upll_strncpy(vrt_val->vrt_description, " ", 2);
  }
  if (vrt_val->valid[UPLL_IDX_DHCP_RELAY_ADMIN_STATUS_VRT] == UNC_VF_VALID) {
    if (!ValidateNumericRange((uint8_t) vrt_val->dhcp_relay_admin_status,
                              (uint8_t) UPLL_ADMIN_ENABLE,
                              (uint8_t) UPLL_ADMIN_DISABLE, true, true)) {
      UPLL_LOG_INFO("Admin status syntax check failed."
                    "Received adminstatus - %d",
                    vrt_val->dhcp_relay_admin_status);
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  }
#if 0
  else if (vrt_val->valid[UPLL_IDX_DHCP_RELAY_ADMIN_STATUS_VRT]
      == UNC_VF_VALID_NO_VALUE
      && (operation == UNC_OP_CREATE || operation == UNC_OP_UPDATE)) {
    vrt_val->dhcp_relay_admin_status = UPLL_ADMIN_ENABLE;
  }
#endif
  return UPLL_RC_SUCCESS;
}

upll_rc_t VrtMoMgr::ValidateVrtPingValue(val_ping *ping_val) {
  UPLL_FUNC_TRACE;
  if (ping_val->valid[UPLL_IDX_TARGET_ADDR_PING] == UNC_VF_VALID) {
    if ((!bc_check(ping_val->target_addr))
        || (!mc_check(ping_val->target_addr))) {
      UPLL_LOG_INFO("Invalid target address received. Target addr - %d",
                    ping_val->target_addr);
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  }
  if (ping_val->valid[UPLL_IDX_SRC_ADDR_PING] == UNC_VF_VALID) {
    if ((!bc_check(ping_val->src_addr)) || (!mc_check(ping_val->src_addr))) {
      UPLL_LOG_INFO("Invalid Source address received. Source addr - %d",
                    ping_val->src_addr);
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  }
  if (ping_val->valid[UPLL_IDX_DF_BIT_PING] == UNC_VF_VALID) {
    if (!ValidateNumericRange(ping_val->dfbit, (uint8_t) UPLL_DF_BIT_DISABLE,
                              (uint8_t) UPLL_DF_BIT_ENABLE, true, true)) {
      UPLL_LOG_INFO("Dfbit syntax check failed."
                    "Received dfbit - %d",
                    ping_val->dfbit);
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  } else if (ping_val->valid[UPLL_IDX_DF_BIT_PING] == UNC_VF_VALID_NO_VALUE) {
    ping_val->dfbit = UPLL_DF_BIT_ENABLE;
  }

  if (ping_val->valid[UPLL_IDX_PACKET_SIZE_PING] == UNC_VF_VALID) {
    if (!ValidateNumericRange(ping_val->packet_size,
                              (uint16_t) kMinPingPacketLen,
                              (uint16_t) kMaxPingPacketLen, true, true)) {
      UPLL_LOG_INFO("packet_size syntax check failed."
                    "Received packet_size - %d",
                    ping_val->packet_size);
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  } else if (ping_val->valid[UPLL_IDX_PACKET_SIZE_PING] == UNC_VF_VALID_NO_VALUE) {
    ping_val->packet_size = 0;
  }

  if (ping_val->valid[UPLL_IDX_COUNT_PING] == UNC_VF_VALID) {
    if (!ValidateNumericRange(ping_val->count, (uint32_t) kMinPingCount,
                              (uint32_t) kMaxPingCount, true, true)) {
      UPLL_LOG_INFO("Count syntax check failed."
                    "Received count - %d",
                    ping_val->count);
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  } else if (ping_val->valid[UPLL_IDX_COUNT_PING] == UNC_VF_VALID_NO_VALUE) {
    ping_val->count = 0;
  }

  if (ping_val->valid[UPLL_IDX_INTERVAL_PING] == UNC_VF_VALID) {
    if (!ValidateNumericRange(ping_val->interval, (uint8_t) kMinPingInterval,
                              (uint8_t) kMaxPingInterval, true, true)) {
      UPLL_LOG_INFO("Interval syntax check failed."
                    "Received received interval - %d",
                    ping_val->interval);
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  } else if (ping_val->valid[UPLL_IDX_INTERVAL_PING] == UNC_VF_VALID_NO_VALUE) {
    ping_val->interval = 0;
  }

  if (ping_val->valid[UPLL_IDX_TIMEOUT_PING] == UNC_VF_VALID) {
    if (!ValidateNumericRange(ping_val->timeout, (uint8_t) kMinPingTimeout,
                              (uint8_t) kMaxPingTimeout, true, true)) {
      UPLL_LOG_INFO("Timeout syntax check failed."
                    "Received timeout - %d",
                    ping_val->timeout);
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  } else if (ping_val->valid[UPLL_IDX_TIMEOUT_PING] == UNC_VF_VALID_NO_VALUE) {
    ping_val->timeout = 0;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VrtMoMgr::ValidateVrtRenameValue(val_rename_vrt *vrt_rename_val) {
  UPLL_FUNC_TRACE;
  upll_rc_t ret_val = UPLL_RC_SUCCESS;
  if (vrt_rename_val->valid[UPLL_IDX_NEW_NAME_RVRT] == UNC_VF_VALID) {
    ret_val = ValidateKey(reinterpret_cast<char *>(vrt_rename_val->new_name),
                          kMinLenVnodeName,
                          kMaxLenVnodeName);
    if (ret_val != UPLL_RC_SUCCESS) {
      UPLL_LOG_INFO("Rename syntax check failed."
                    "Received vrt_rename - %s",
                    vrt_rename_val->new_name);
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VrtMoMgr::ValidateMessage(IpcReqRespHeader *req, ConfigKeyVal *ikey) {
  UPLL_FUNC_TRACE;
  upll_rc_t ret_val = UPLL_RC_SUCCESS;

  /** read datatype, operation, options from IpcReqRespHeader */
  if (!req || !ikey || !(ikey->get_key())) {
    UPLL_LOG_INFO("ConfigKeyVal / IpcReqRespHeader is NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  upll_keytype_datatype_t dt_type = req->datatype;
  unc_keytype_operation_t operation = req->operation;
  unc_keytype_option1_t option1 = req->option1;
  unc_keytype_option2_t option2 = req->option2;

  unc_key_type_t ktype = ikey->get_key_type();
  if (UNC_KT_VROUTER != ktype) {
    UPLL_LOG_INFO("Invalid Keytype received. received - %d", ktype);
    return UPLL_RC_ERR_BAD_REQUEST;
  }

  if (ikey->get_st_num() != IpctSt::kIpcStKeyVrt) {
    UPLL_LOG_INFO("Invalid Key structure received. received struct - %d",
                  ikey->get_st_num());
    return UPLL_RC_ERR_BAD_REQUEST;
  }
  key_vrt *vrt_key = reinterpret_cast<key_vrt *>(ikey->get_key());
  
  ret_val = ValidateVrtKey(vrt_key, operation);
  if (ret_val != UPLL_RC_SUCCESS) {
    UPLL_LOG_INFO("syntax check failure for key_vrt struct");
    return UPLL_RC_ERR_CFG_SYNTAX;
  }
  val_vrt *vrt_val = NULL;
  val_rename_vrt *vrt_rename = NULL;

  if ((ikey->get_cfg_val()) &&
      ((ikey->get_cfg_val())->get_st_num() == IpctSt::kIpcStValVrt)) {
    vrt_val =  reinterpret_cast<val_vrt *>(ikey->get_cfg_val()->get_val());
  } else if ((ikey->get_cfg_val()) &&
             ((ikey->get_cfg_val())->get_st_num() == IpctSt::kIpcStValRenameVrt)) {
    vrt_rename =  reinterpret_cast<val_rename_vrt *>(ikey->get_cfg_val()->get_val());
  } 

  if (((operation == UNC_OP_CREATE) || (operation == UNC_OP_UPDATE))  &&
     ((dt_type == UPLL_DT_CANDIDATE) || (dt_type == UPLL_DT_IMPORT))) {
    if (vrt_val == NULL) {
      UPLL_LOG_INFO("Val struct is mandatory for create and update op");
      return UPLL_RC_ERR_BAD_REQUEST;
    }
    ret_val = ValidateVrtValue(vrt_val, operation);
    if (ret_val != UPLL_RC_SUCCESS) {
      UPLL_LOG_INFO("syntax check failure for val_vrt struct");
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
    return UPLL_RC_SUCCESS;
  } else if ((operation == UNC_OP_RENAME) && (dt_type == UPLL_DT_IMPORT)) {
    if (option1 != UNC_OPT1_NORMAL) {
      UPLL_LOG_INFO("Error option1 is not NORMAL");
      return UPLL_RC_ERR_INVALID_OPTION1;
    }
    if (option2 != UNC_OPT2_NONE) {
      UPLL_LOG_INFO("Error option2 is not NONE");
      return UPLL_RC_ERR_INVALID_OPTION2;
    }
    if (vrt_rename == NULL) {
      UPLL_LOG_INFO(
       "val_rename_vrt struct is Mandatory for Rename op");
      return UPLL_RC_ERR_BAD_REQUEST;
    }
    ret_val = ValidateVrtRenameValue(vrt_rename);
    if (ret_val != UPLL_RC_SUCCESS) {
      UPLL_LOG_INFO("syntax check failed for val_rename_vrt structure");
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
    return UPLL_RC_SUCCESS;
  } else if ((operation == UNC_OP_READ || operation == UNC_OP_READ_SIBLING ||
              operation == UNC_OP_READ_SIBLING_BEGIN) && 
        		 (dt_type == UPLL_DT_IMPORT)) {
    if (option1 != UNC_OPT1_NORMAL) {
      UPLL_LOG_INFO("Error option1 is not NORMAL");
      return UPLL_RC_ERR_INVALID_OPTION1;
    }
    if (option2 != UNC_OPT2_NONE) {
      UPLL_LOG_INFO("Error option2 is not NONE");
      return UPLL_RC_ERR_INVALID_OPTION2;
    }
    if (vrt_rename == NULL) {
      UPLL_LOG_INFO("syntax check for val_rename_vrt struct is optional");
      return UPLL_RC_SUCCESS;
    }
    ret_val = ValidateVrtRenameValue(vrt_rename);
    if (ret_val != UPLL_RC_SUCCESS) {
      UPLL_LOG_INFO("syntax check failure for key_vrt struct");
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
    return UPLL_RC_SUCCESS;
  } else if ((operation == UNC_OP_READ || operation == UNC_OP_READ_SIBLING ||
              operation == UNC_OP_READ_SIBLING_BEGIN ||
              operation == UNC_OP_READ_SIBLING_COUNT) && 
             (dt_type == UPLL_DT_CANDIDATE || dt_type == UPLL_DT_RUNNING ||
              dt_type == UPLL_DT_STARTUP)) {
    if (option1 != UNC_OPT1_NORMAL) {
      UPLL_LOG_INFO("Error option1 is not NORMAL");
      return UPLL_RC_ERR_INVALID_OPTION1;
    }
    if (option2 != UNC_OPT2_NONE) {
      UPLL_LOG_INFO("Error option2 is not NONE");
      return UPLL_RC_ERR_INVALID_OPTION2;
    }
    if (vrt_val == NULL) {
      UPLL_LOG_INFO("syntax check for val_vrt struct is an optional");
      return UPLL_RC_SUCCESS;
    }
    ret_val = ValidateVrtValue(vrt_val, operation);
    if (ret_val != UPLL_RC_SUCCESS) {
      UPLL_LOG_INFO("syntax check failure for val_vrt value structure");
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
    return UPLL_RC_SUCCESS;
  } else if ((operation == UNC_OP_READ || operation == UNC_OP_READ_SIBLING ||
             operation == UNC_OP_READ_SIBLING_BEGIN || UNC_OP_READ_SIBLING_COUNT)
            && (dt_type == UPLL_DT_STATE)) {

    if (option1 == UNC_OPT1_NORMAL) {
      if (option2 == UNC_OPT2_NONE) {
        if (vrt_val == NULL) {
          UPLL_LOG_INFO("syntax check for val_vrt struct is an optional");
          return UPLL_RC_SUCCESS;
        }
        ret_val = ValidateVrtValue(vrt_val, operation);
        if (ret_val != UPLL_RC_SUCCESS) {
          UPLL_LOG_INFO("syntax check failure for val_vrt value structure");
          return UPLL_RC_ERR_CFG_SYNTAX;
        }
        return UPLL_RC_SUCCESS;
      } else if (option2 == UNC_OPT2_DHCP_RELAY || 
                 option2 == UNC_OPT2_IP_ROUTE ||
                 option2 == UNC_OPT2_ARP_ENTRY ||
                 option2 == UNC_OPT2_ARP_ENTRY_STATIC ||
                 option2 == UNC_OPT2_ARP_ENTRY_DYNAMIC) {
        UPLL_LOG_INFO("val struct is optional for this option");
        return UPLL_RC_SUCCESS;
      } else {
        UPLL_LOG_INFO("Option2 is Invalid");
        return UPLL_RC_ERR_INVALID_OPTION2;
      }  
    } else if (option1 == UNC_OPT1_DETAIL) {                                           
      if (option2 == UNC_OPT2_IP_ROUTE ||
          option2 == UNC_OPT2_DHCP_RELAY) {
        UPLL_LOG_INFO("val struct is optional for this option");
        return UPLL_RC_SUCCESS;
      } else {
        UPLL_LOG_INFO("Option2 is Invalid");
        return UPLL_RC_ERR_INVALID_OPTION2;
      }
    } else if (option1 == UNC_OPT1_COUNT) {
      if (option2 == UNC_OPT2_ARP_ENTRY ||
          option2 == UNC_OPT2_ARP_ENTRY_STATIC ||
          option2 == UNC_OPT2_ARP_ENTRY_DYNAMIC) {
        UPLL_LOG_INFO("val struct is none for this option");
        return UPLL_RC_SUCCESS;
      } else {
        UPLL_LOG_INFO("Option2 is Invalid");
        return UPLL_RC_ERR_INVALID_OPTION2;
      }
    } else {
      UPLL_LOG_INFO("Error option1 is Invalid");
      return UPLL_RC_ERR_INVALID_OPTION1;
    }
  } else if ((operation == UNC_OP_READ_NEXT || operation == UNC_OP_READ_BULK) &&
             (dt_type == UPLL_DT_CANDIDATE || dt_type == UPLL_DT_RUNNING ||
              dt_type == UPLL_DT_STARTUP || dt_type == UPLL_DT_IMPORT)) {
    UPLL_LOG_INFO("Value structure is none for operation type:%d", operation);
    return UPLL_RC_SUCCESS;
  } else if ((operation == UNC_OP_DELETE) && (dt_type == UPLL_DT_CANDIDATE)) {
    UPLL_LOG_INFO("Value structure is none for operation type:%d", operation);
    return UPLL_RC_SUCCESS;
  } else if ((operation == UNC_OP_CONTROL) &&
             (dt_type == UPLL_DT_RUNNING || dt_type == UPLL_DT_STATE)) {
      if (option1 != UNC_OPT1_NORMAL) {
        UPLL_LOG_INFO("Error option1 is not NORMAL");
        return UPLL_RC_ERR_INVALID_OPTION1;
      }
      if (option2 != UNC_OPT2_PING) {
        UPLL_LOG_INFO("Error option2 is not PING");
        return UPLL_RC_ERR_INVALID_OPTION2;
      }
      if (NULL == ikey->get_cfg_val()) {
        UPLL_LOG_TRACE("ConfigVal struct is null");
        return UPLL_RC_ERR_BAD_REQUEST;
      }
      if (ikey->get_cfg_val()->get_st_num() != IpctSt::kIpcStValPing) {
        UPLL_LOG_INFO(
            "Expecting val_ping structure. received struct - %d",
            ikey->get_cfg_val()->get_st_num());
        return UPLL_RC_ERR_BAD_REQUEST;
      }
      val_ping *ping_val =
          reinterpret_cast<val_ping *>(ikey->get_cfg_val()->get_val());
      if (ping_val == NULL) {
        UPLL_LOG_TRACE("val_ping struct is missing");
        return UPLL_RC_ERR_GENERIC;
      }
      ret_val = ValidateVrtPingValue(ping_val);
      if (ret_val != UPLL_RC_SUCCESS) {
        UPLL_LOG_INFO("syntax check failure for val_ping value structure");
        return UPLL_RC_ERR_CFG_SYNTAX;
      }
      return UPLL_RC_SUCCESS;
  }
  UPLL_LOG_INFO("Unsupported datatype-(%d) or operation-(%d)", dt_type,
                   operation);
  return UPLL_RC_ERR_NO_SUCH_INSTANCE;
}

upll_rc_t VrtMoMgr::ValidateCapability(IpcReqRespHeader *req,
                                       ConfigKeyVal *ikey,
                                       const char *ctrlr_name) {
  UPLL_FUNC_TRACE;
  upll_rc_t ret_val = UPLL_RC_SUCCESS;
  /** read datatype, operation, options from IpcReqRespHeader */
  if (!ikey || !req ) {
    UPLL_LOG_INFO(" ConfigKeyVal / IpcReqRespHeader is NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  if (!ctrlr_name)
     ctrlr_name = reinterpret_cast<char *>(ikey->get_user_data());

  upll_keytype_datatype_t dt_type = req->datatype;
  unc_keytype_operation_t operation = req->operation;
  unc_keytype_option1_t option1 = req->option1;
  unc_keytype_option2_t option2 = req->option2;

  if (operation == UNC_OP_CREATE || operation == UNC_OP_UPDATE) {
    if (dt_type == UPLL_DT_CANDIDATE || UPLL_DT_IMPORT == dt_type) {
      ret_val = ValidateVrtAttributeSupportCheck(ctrlr_name, ikey, operation);
      if (ret_val != UPLL_RC_SUCCESS) {
        UPLL_LOG_INFO("vrt struct attributes are not supported by ctrlr");
        return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
      }
      return UPLL_RC_SUCCESS;
    } else {
      UPLL_LOG_INFO("Unsupported datatype -(%d)", dt_type);
      return UPLL_RC_ERR_GENERIC;
    }
  } else if (operation == UNC_OP_READ || operation == UNC_OP_READ_SIBLING
      || operation == UNC_OP_READ_SIBLING_BEGIN
      || operation == UNC_OP_READ_SIBLING_COUNT) {
    if (dt_type == UPLL_DT_CANDIDATE || dt_type == UPLL_DT_RUNNING
        || dt_type == UPLL_DT_STARTUP || dt_type == UPLL_DT_STATE) {
      if (option1 != UNC_OPT1_NORMAL) {
        UPLL_LOG_INFO("Error option1 is not NORMAL");
        return UPLL_RC_ERR_INVALID_OPTION1;
      }
      if (option2 != UNC_OPT2_NONE) {
        UPLL_LOG_INFO("Error option2 is not NONE");
        return UPLL_RC_ERR_INVALID_OPTION2;
      }
      if (NULL == ikey->get_cfg_val()) {
        UPLL_LOG_INFO("ConfigVal struct is empty");
        return UPLL_RC_ERR_CFG_SYNTAX;
      }
      if (ikey->get_cfg_val()->get_val() != NULL) {
        ret_val = ValidateVrtAttributeSupportCheck(ctrlr_name, ikey, operation);
        if (ret_val != UPLL_RC_SUCCESS) {
          UPLL_LOG_INFO("VRT struct capa check failure for read operation");
          return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
        }
        return UPLL_RC_SUCCESS;
      } else {
        UPLL_LOG_TRACE("val_vrt struct is an optional");
        return UPLL_RC_SUCCESS;
      }
    } else {
      UPLL_LOG_INFO("Unsupported datatype - (%d)", dt_type);
      return UPLL_RC_ERR_NO_SUCH_INSTANCE;
    }
  }
  UPLL_LOG_INFO("Unsupported operation -(%d)", operation);
  return UPLL_RC_ERR_NO_SUCH_INSTANCE;
}

upll_rc_t VrtMoMgr::ValidateVrtAttributeSupportCheck(const char *ctrlr_name,
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
      if (result_code && (max_instance_count != 0) &&
          (cur_instance_count >= max_instance_count)) {
        UPLL_LOG_INFO("[%s:%d:%s Instance count %d exceeds %d", __FILE__,
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
      UPLL_LOG_INFO("Invalid operation code");
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
  if (ikey->get_cfg_val()->get_st_num() != IpctSt::kIpcStValVrt) {
    UPLL_LOG_INFO("Invalid value structure received. received struct - %d",
                  ikey->get_cfg_val()->get_st_num());
    return UPLL_RC_ERR_CFG_SYNTAX;
  }
  val_vrt *vrt_val = reinterpret_cast<val_vrt *>
                                 (ikey->get_cfg_val()->get_val());
  if (vrt_val != NULL) {
#if 0  // CHECK
    if ((vrt_val->valid[UPLL_IDX_CONTROLLER_ID_VRT] == UNC_VF_VALID) ||
        (vrt_val->valid[UPLL_IDX_CONTROLLER_ID_VRT] == UNC_VF_VALID_NO_VALUE)) {
      if (attrs[unc::capa::vrt::kCapControllerId] == 0) {
        vrt_val->valid[UPLL_IDX_CONTROLLER_ID_VRT] = UNC_VF_NOT_SOPPORTED;
        UPLL_LOG_INFO("Controller id attr is not supported by pfc ctrlr ");
        return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
      }
    }
#endif
    if ((vrt_val->valid[UPLL_IDX_DESC_VRT] == UNC_VF_VALID)
        || (vrt_val->valid[UPLL_IDX_DESC_VRT] == UNC_VF_VALID_NO_VALUE)) {
      if (attrs[unc::capa::vrt::kCapDesc] == 0) {
        vrt_val->valid[UPLL_IDX_DESC_VRT] = UNC_VF_NOT_SOPPORTED;
        UPLL_LOG_INFO("Description attr is not supported by pfc ctrlr ");
        return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
      }
    }
    if ((vrt_val->valid[UPLL_IDX_DHCP_RELAY_ADMIN_STATUS_VRT] == UNC_VF_VALID)
        || (vrt_val->valid[UPLL_IDX_DHCP_RELAY_ADMIN_STATUS_VRT]
            == UNC_VF_VALID_NO_VALUE)) {
      if (attrs[unc::capa::vrt::kCapDhcpRelayAdminStatus] == 0) {
        vrt_val->valid[UPLL_IDX_DHCP_RELAY_ADMIN_STATUS_VRT] =
            UNC_VF_NOT_SOPPORTED;
        UPLL_LOG_INFO("Admin status attr is not supported by pfc ctrlr ");
        return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
      }
    }
    return UPLL_RC_SUCCESS;
  }
  UPLL_LOG_INFO("Error val_vrt struct is NULL");
  return UPLL_RC_ERR_CFG_SYNTAX;
}

upll_rc_t VrtMoMgr::CreateVnodeConfigKey(ConfigKeyVal *ikey,
                                         ConfigKeyVal *&okey) {
  UPLL_FUNC_TRACE;
  if (ikey == NULL) return UPLL_RC_ERR_GENERIC;
  key_vrt * temp_key_vrt = reinterpret_cast<key_vrt *>
      (ConfigKeyVal::Malloc(sizeof(key_vrt)));
  if (!strlen(reinterpret_cast<const char*>(reinterpret_cast<key_vrt*>
                    (ikey->get_key())->vtn_key.vtn_name))) {
    /* Addressed RESOURCE_LEAK */
    FREE_IF_NOT_NULL(temp_key_vrt);
    return UPLL_RC_ERR_GENERIC;
  }
  uuu::upll_strncpy(temp_key_vrt->vtn_key.vtn_name,
         reinterpret_cast<key_vrt*>(ikey->get_key())->vtn_key.vtn_name,
         (kMaxLenVtnName+1));
  if (!strlen(reinterpret_cast<const char*>( reinterpret_cast<key_vrt*>
                    (ikey->get_key())->vrouter_name))) {
    // RESOURCE_LEAK
    FREE_IF_NOT_NULL(temp_key_vrt);
    return UPLL_RC_ERR_GENERIC;
  }
  uuu::upll_strncpy(temp_key_vrt->vrouter_name,
         reinterpret_cast<key_vrt*>(ikey->get_key())->vrouter_name,
         (kMaxLenVnodeName+1));

  ConfigKeyVal *ck_vrt = new ConfigKeyVal(UNC_KT_VROUTER, IpctSt::kIpcStKeyVrt,
                                 reinterpret_cast<void *>(temp_key_vrt), NULL);
  if (ck_vrt == NULL) return UPLL_RC_ERR_GENERIC;
  okey = ck_vrt;
  return UPLL_RC_SUCCESS;
}

upll_rc_t VrtMoMgr::IsReferenced(ConfigKeyVal *ikey,
                                 upll_keytype_datatype_t dt_type,
                                 DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  MoMgrImpl *mgr = reinterpret_cast<MoMgrImpl *>(const_cast<MoManager*>
                                          (GetMoManager(UNC_KT_VRT_IF)));
  result_code = mgr->IsReferenced(ikey, dt_type, dmi);
  if (UPLL_RC_SUCCESS != result_code) 
      return result_code;
  return UPLL_RC_SUCCESS;
}

upll_rc_t VrtMoMgr::AdaptValToVtnService(ConfigKeyVal *ikey) {
  UPLL_FUNC_TRACE;
  if (!ikey) {
    UPLL_LOG_DEBUG("Invalid ikey");
    return UPLL_RC_ERR_GENERIC;
  }
  while (ikey) {
    ConfigVal *cval = ikey->get_cfg_val();
    if (!cval) {
      UPLL_LOG_DEBUG("Config Val is Null");
      return UPLL_RC_ERR_GENERIC;
    }
    while (cval ) {
      if (IpctSt::kIpcStValVrtSt == cval->get_st_num()) {
        val_vrt_st *vrt_val_st = reinterpret_cast<val_vrt_st *>
                         (ConfigKeyVal::Malloc(sizeof(val_vrt_st)));
        val_db_vrt_st *db_vrt_val_st = reinterpret_cast<val_db_vrt_st *>
                                     (cval->get_val());
        memcpy(vrt_val_st, &(db_vrt_val_st->vrt_val_st),
               sizeof(val_vrt_st));
        cval->SetVal(IpctSt::kIpcStValVrtSt, vrt_val_st);
      }
      cval = cval->get_next_cfg_val();
    }
    ikey = ikey->get_next_cfg_key_val();
  }
  return UPLL_RC_SUCCESS;
}
}  // namespace kt_momgr
}  // namespace upll
}  // namespace unc
