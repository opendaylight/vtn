/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include "vunk_if_momgr.hh"
#include "uncxx/upll_log.hh"
#include "vunk_momgr.hh"
#include "vlink_momgr.hh"

#define NUM_KEY_MAIN_TBL_ 5
#if 0
namespace upll_dal_vbrif unc::upll::dal::schema::table::vbridge_interface;
#endif
namespace unc {
namespace upll {
namespace kt_momgr {

BindInfo VunkIfMoMgr::vunk_if_bind_info[] = {
    { uudst::vunknown_interface::kDbiVtnName, CFG_KEY, offsetof(
        key_vunk_if, vunk_key.vtn_key.vtn_name),
      uud::kDalChar, 32 },
    { uudst::vunknown_interface::kDbiVunknownName, CFG_KEY, offsetof(
        key_vunk_if, vunk_key.vunknown_name),
      uud::kDalChar, 32 },
    { uudst::vunknown_interface::kDbiIfName, CFG_KEY, offsetof(key_vunk_if,
                                                               if_name),
      uud::kDalChar, 32 },
    { uudst::vunknown_interface::kDbiDesc, CFG_VAL, offsetof(val_vunk_if,
                                                             description),
      uud::kDalChar, 128 },
    { uudst::vunknown_interface::kDbiAdminStatus, CFG_VAL, offsetof(
        val_vunk_if, admin_status),
      uud::kDalUint8, 1 },
    { uudst::vunknown_interface::kDbiDomainId, CK_VAL, offsetof(key_user_data,
                                                                domain_id),
      uud::kDalChar, 32 },
    { uudst::vunknown_interface::kDbiValidDesc, CFG_META_VAL, offsetof(
        val_vunk_if, valid[UPLL_IDX_DESC_VUNI]),
      uud::kDalUint8, 1 },
    { uudst::vunknown_interface::kDbiValidAdminStatus, CFG_DEF_VAL, offsetof(
        val_vunk_if, valid[UPLL_IDX_ADMIN_ST_VUNI]),
      uud::kDalUint8, 1 },
    { uudst::vunknown_interface::kDbiCsDesc, CS_VAL, offsetof(
        val_vunk_if, cs_attr[UPLL_IDX_DESC_VUNI]),
      uud::kDalUint8, 1 },
    { uudst::vunknown_interface::kDbiCsAdminStatus, CS_VAL, offsetof(
        val_vunk_if, cs_attr[UPLL_IDX_ADMIN_ST_VUNI]),
      uud::kDalUint8, 1 },
    { uudst::vunknown_interface::kDbiCsRowstatus, CS_VAL, offsetof(
        val_vunk_if, cs_row_status),
      uud::kDalUint8, 1 },
    { uudst::vunknown_interface::kDbiFlags, CK_VAL, offsetof(key_user_data_t,
                                                             flags),
      uud::kDalUint8, 1 } };

BindInfo VunkIfMoMgr::key_vunk_if_maintbl_update_bind_info[] = {
    { uudst::vunknown_interface::kDbiVtnName, CFG_MATCH_KEY, offsetof(
        key_vunk_if, vunk_key.vtn_key.vtn_name),
      uud::kDalChar, kMaxLenVtnName + 1 },
    { uudst::vunknown_interface::kDbiVunknownName, CFG_MATCH_KEY, offsetof(
        key_vunk_if, vunk_key.vunknown_name),
      uud::kDalChar, kMaxLenVnodeName + 1 },
    { uudst::vunknown_interface::kDbiIfName, CFG_MATCH_KEY, offsetof(
        key_vunk_if, if_name),
      uud::kDalChar, kMaxLenVnodeName + 1 },
    { uudst::vunknown_interface::kDbiVtnName, CFG_INPUT_KEY, offsetof(
        key_rename_vnode_info_t, new_unc_vtn_name),
      uud::kDalChar, kMaxLenVtnName + 1 },
    { uudst::vunknown_interface::kDbiFlags, CK_VAL, offsetof(
        key_user_data_t, flags),
      uud::kDalUint8, 1 } };

VunkIfMoMgr::VunkIfMoMgr() {
  UPLL_FUNC_TRACE
  ntable = MAX_MOMGR_TBLS;
  table = new Table *[ntable]();
  table[MAINTBL] = new Table(uudst::kDbiVunknownIfTbl, UNC_KT_VUNK_IF,
                         vunk_if_bind_info, IpctSt::kIpcStKeyVunkIf,
                         IpctSt::kIpcStValVunkIf,
                         uudst::vunknown_interface::kDbiVunknownIfNumCols);
  table[RENAMETBL] = NULL;
  table[CTRLRTBL] = NULL;
  table[CONVERTTBL] = NULL;

  nchild = 0;
  child = NULL;
}

/*
 * Based on the key type the bind info will pass
 **/
bool VunkIfMoMgr::GetRenameKeyBindInfo(unc_key_type_t key_type,
                                       BindInfo *&binfo, int &nattr,
                                       MoMgrTables tbl) {
  /* Main Table or rename table only update */
  if (MAINTBL == tbl) {
    nattr = NUM_KEY_MAIN_TBL_;
    binfo = key_vunk_if_maintbl_update_bind_info;
  } else {
    UPLL_LOG_TRACE("Invalid Table for VunknownInterface");
    return PFC_FALSE;
  }
  return PFC_TRUE;
}

bool VunkIfMoMgr::IsValidKey(void *key, uint64_t index,
                             MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  key_vunk_if *vunk_if_key = static_cast<key_vunk_if *>(key);
  upll_rc_t ret_val = UPLL_RC_SUCCESS;
  switch (index) {
    case uudst::vunknown_interface::kDbiVtnName :
      ret_val = ValidateKey(reinterpret_cast<char *>
                           (vunk_if_key->vunk_key.vtn_key.vtn_name),
                            kMinLenVtnName, kMaxLenVtnName);
      if (ret_val != UPLL_RC_SUCCESS) {
        UPLL_LOG_TRACE("VTN Name is not valid(%d)", ret_val);
        return false;
      }
      break;
    case uudst::vunknown_interface::kDbiVunknownName:
      ret_val = ValidateKey(reinterpret_cast<char *>
                           (vunk_if_key->vunk_key.vunknown_name),
                            kMinLenVnodeName, kMaxLenVnodeName);
      if (ret_val != UPLL_RC_SUCCESS) {
        UPLL_LOG_TRACE("Vunknown name is not valid(%d)", ret_val);
        return false;
      }
      break;
    case uudst::vunknown_interface::kDbiIfName:
      ret_val = ValidateKey(reinterpret_cast<char *>(vunk_if_key->if_name),
                            kMinLenInterfaceName, kMaxLenInterfaceName);
      if (ret_val != UPLL_RC_SUCCESS) {
        UPLL_LOG_TRACE("Vunknown IF Name is not valid(%d)", ret_val);
        return false;
      }
      break;
    default:
      UPLL_LOG_INFO("Invalid Key Index");
      break;
  }
  return true;
}

bool VunkIfMoMgr::CompareValidValue(void *&val1,
                                 void *val2,
                                 bool copy_to_running) {
  UPLL_FUNC_TRACE;
  bool invalid_attr = true;
  val_vunk_if *val_vunkif1 = reinterpret_cast<val_vunk_if *>(val1);
  val_vunk_if *val_vunkif2 = reinterpret_cast<val_vunk_if *>(val2);
  if (!val_vunkif2) {
      UPLL_LOG_TRACE("Invalid param");
      return UPLL_RC_ERR_GENERIC;
  }
  for (unsigned int loop = 0;
        loop < sizeof(val_vunkif1->valid) / sizeof(uint8_t); ++loop) {
      if (UNC_VF_INVALID == val_vunkif1->valid[loop]
          && UNC_VF_VALID == val_vunkif2->valid[loop])
        val_vunkif1->valid[loop] = UNC_VF_VALID_NO_VALUE;
  }
  /* Specify the configured ip address for
   * PFC to clear the configured IP address
   */
  if ((UNC_VF_VALID == val_vunkif1->valid[UPLL_IDX_DESC_VUNI])
        && (UNC_VF_VALID == val_vunkif2->valid[UPLL_IDX_DESC_VUNI]))
    if (!strncmp(reinterpret_cast<char*>(val_vunkif1->description),
                 reinterpret_cast<char*>(val_vunkif2->description),
                 kMaxLenDescription))
        val_vunkif1->valid[UPLL_IDX_DESC_VUNI] = UNC_VF_INVALID;

  if ((val_vunkif2->valid[UPLL_IDX_ADMIN_ST_VUNI] ==
       val_vunkif1->valid[UPLL_IDX_ADMIN_ST_VUNI])
      && UNC_VF_INVALID != val_vunkif2->valid[UPLL_IDX_ADMIN_ST_VUNI]) {
    if (val_vunkif1->admin_status == val_vunkif2->admin_status)
      val_vunkif1->valid[UPLL_IDX_ADMIN_ST_VUNI] =
        (copy_to_running)?UNC_VF_INVALID:UNC_VF_VALUE_NOT_MODIFIED;
  }

  for (unsigned int loop = 0;
      loop < sizeof(val_vunkif1->valid) / sizeof(uint8_t); ++loop) {
    if ((UNC_VF_VALID == (uint8_t) val_vunkif1->valid[loop]) ||
       (UNC_VF_VALID_NO_VALUE == (uint8_t) val_vunkif1->valid[loop])) {
        invalid_attr = false;
        break;
    }
  }
  return invalid_attr;
}

upll_rc_t VunkIfMoMgr::GetChildConfigKey(ConfigKeyVal *&okey,
                                         ConfigKeyVal *parent_key) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  bool cfgval_ctrlr = false;
  key_vunk_if *vunk_key_if;
  void *pkey;
  if (parent_key == NULL) {
    vunk_key_if = static_cast<key_vunk_if *>(malloc(sizeof(key_vunk_if)));
    if (!vunk_key_if) return UPLL_RC_ERR_GENERIC;
    memset(vunk_key_if, 0, sizeof(key_vunk_if));
    if (okey) delete okey;
    okey = new ConfigKeyVal(UNC_KT_VUNK_IF, IpctSt::kIpcStKeyVunkIf,
                            vunk_key_if, NULL);
    return UPLL_RC_SUCCESS;
  } else {
    pkey = parent_key->get_key();
  }
  if (!pkey) return UPLL_RC_ERR_GENERIC;
  if (okey) {
    if (okey->get_key_type() != UNC_KT_VUNK_IF) {
      return UPLL_RC_ERR_GENERIC;
    }
    vunk_key_if = static_cast<key_vunk_if *>(okey->get_key());
  } else {
    vunk_key_if = static_cast<key_vunk_if *>
                  (ConfigKeyVal::Malloc(sizeof(key_vunk_if)));
  }
  unc_key_type_t keytype = parent_key->get_key_type();
  switch (keytype) {
    case UNC_KT_VTN:
      uuu::upll_strncpy(vunk_key_if->vunk_key.vtn_key.vtn_name,
             reinterpret_cast<char *>(static_cast<key_vtn *>(pkey)->vtn_name),
             kMaxLenVtnName+1);
      break;
    case UNC_KT_VUNKNOWN:
      uuu::upll_strncpy(vunk_key_if->vunk_key.vtn_key.vtn_name,
             reinterpret_cast<char *>
                      (static_cast<key_vunknown *>(pkey)->vtn_key.vtn_name),
                      kMaxLenVtnName+1);
      uuu::upll_strncpy(vunk_key_if->vunk_key.vunknown_name,
             reinterpret_cast<char *>
                      (static_cast<key_vunknown *>(pkey)->vunknown_name),
                      kMaxLenVnodeName+1);
      break;
    case UNC_KT_VUNK_IF:
      uuu::upll_strncpy(vunk_key_if->vunk_key.vtn_key.vtn_name,
             reinterpret_cast<char *>(static_cast<key_vunk_if *>
                           (pkey)->vunk_key.vtn_key.vtn_name),
                           kMaxLenVtnName+1);
      uuu::upll_strncpy(vunk_key_if->vunk_key.vunknown_name,
             reinterpret_cast<char *>(static_cast<key_vunk_if *>
                               (pkey)->vunk_key.vunknown_name),
                               kMaxLenVnodeName+1);
      uuu::upll_strncpy(vunk_key_if->if_name,
             reinterpret_cast<char *>(static_cast<key_vunk_if *>
             (pkey)->if_name), kMaxLenInterfaceName+1);
      break;
    case UNC_KT_VLINK: {
      uint8_t *vnode_name, *if_name;
      uint8_t flags = 0;
      val_vlink *vlink_val = reinterpret_cast<val_vlink *>(GetVal(parent_key));
      if (!vlink_val) {
        if (!okey || !(okey->get_key()))
          free(vunk_key_if);
        return UPLL_RC_ERR_GENERIC;
      }
      GET_USER_DATA_FLAGS(parent_key->get_cfg_val(), flags);
      flags &=  VLINK_FLAG_NODE_POS;
      UPLL_LOG_DEBUG("Vlink flag node position %d", flags);
      if (flags == kVlinkVnode2) {
        cfgval_ctrlr = true;
        vnode_name = vlink_val->vnode2_name;
        if_name = vlink_val->vnode2_ifname;
      } else {
        vnode_name = vlink_val->vnode1_name;
        if_name = vlink_val->vnode1_ifname;
      }
      uuu::upll_strncpy(vunk_key_if->vunk_key.vtn_key.vtn_name,
                        reinterpret_cast<key_vlink *>(pkey)->vtn_key.vtn_name,
                        (kMaxLenVtnName + 1));
      if (vnode_name)
        uuu::upll_strncpy(vunk_key_if->vunk_key.vunknown_name, vnode_name,
                          (kMaxLenVnodeName + 1));
      if (if_name)
        uuu::upll_strncpy(vunk_key_if->if_name, if_name,
                          (kMaxLenInterfaceName + 1));
    }
    break;
    default:
      break;
  }
  if (!okey)
    okey = new ConfigKeyVal(UNC_KT_VUNK_IF, IpctSt::kIpcStKeyVunkIf,
                            vunk_key_if, NULL);
  else if (okey->get_key() != vunk_key_if)
    okey->SetKey(IpctSt::kIpcStKeyVunkIf, vunk_key_if);
  if (okey == NULL) {
    free(vunk_key_if);
    result_code = UPLL_RC_ERR_GENERIC;
  } else {
    if (cfgval_ctrlr) {
      SET_USER_DATA(okey, parent_key->get_cfg_val());
    } else {
      SET_USER_DATA(okey, parent_key);
    }
  }
  return result_code;
}

upll_rc_t VunkIfMoMgr::GetParentConfigKey(ConfigKeyVal *&okey,
                                          ConfigKeyVal *ikey) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  if (!ikey) {
    UPLL_LOG_DEBUG("ikey is NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  unc_key_type_t ikey_type = ikey->get_key_type();

  if (ikey_type != UNC_KT_VUNK_IF) return UPLL_RC_ERR_GENERIC;
  key_vunk_if *pkey = static_cast<key_vunk_if *>
                  (ikey->get_key());
  if (!pkey) return UPLL_RC_ERR_GENERIC;
  key_vunknown *vunk_key = static_cast<key_vunknown *>
                  (malloc(sizeof(key_vunknown)));
  if (!vunk_key) return UPLL_RC_ERR_GENERIC;
  memset(vunk_key, 0, sizeof(key_vunknown));
#if 1
  uuu::upll_strncpy(vunk_key->vtn_key.vtn_name,
         reinterpret_cast<char *>(static_cast<key_vunk_if *>
                  (pkey)->vunk_key.vtn_key.vtn_name),
                  kMaxLenVtnName+1);
  uuu::upll_strncpy(vunk_key->vunknown_name,
         reinterpret_cast<char *>(static_cast<key_vunk_if *>
                  (pkey)->vunk_key.vunknown_name),
                  kMaxLenVnodeName+1);
#endif
  if (okey) delete okey;
  okey = new ConfigKeyVal(UNC_KT_VUNKNOWN, IpctSt::kIpcStKeyVunknown, vunk_key,
                          NULL);
  if (okey == NULL) {
    free(vunk_key);
    result_code = UPLL_RC_ERR_GENERIC;
  } else {
    SET_USER_DATA(okey, ikey);
  }
  return result_code;
}

upll_rc_t VunkIfMoMgr::AllocVal(ConfigVal *&ck_val,
                                upll_keytype_datatype_t dt_type,
                                MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  void *val;
  if (ck_val != NULL) return UPLL_RC_ERR_GENERIC;
  switch (tbl) {
    case MAINTBL:
      val = reinterpret_cast<void *>(malloc(sizeof(val_vunk_if)));
      if (!val) return UPLL_RC_ERR_GENERIC;
      memset(val, 0, sizeof(val_vunk_if));
      ck_val = new ConfigVal(IpctSt::kIpcStValVunkIf, val);
      break;
    default:
       UPLL_LOG_TRACE("Invalid Table for VunknownInterface");
      val = NULL;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VunkIfMoMgr::DupConfigKeyVal(ConfigKeyVal *&okey, ConfigKeyVal *&req,
                                       MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  if (req == NULL) return UPLL_RC_ERR_GENERIC;
  if (okey != NULL) return UPLL_RC_ERR_GENERIC;
  if (req->get_key_type() != UNC_KT_VUNK_IF) return UPLL_RC_ERR_GENERIC;
  ConfigVal *tmp1 = NULL, *tmp = (req)->get_cfg_val();

  if (tmp) {
    if (tbl == MAINTBL) {
      val_vunk_if *ival = static_cast<val_vunk_if *>(GetVal(req));
      val_vunk_if *vunk_val_if = static_cast<val_vunk_if *>
                                           (malloc(sizeof(val_vunk_if)));
      if (!vunk_val_if || !ival) {
        FREE_IF_NOT_NULL(vunk_val_if);
        return UPLL_RC_ERR_GENERIC;
      }
      memcpy(vunk_val_if, ival, sizeof(val_vunk_if));
      tmp1 = new ConfigVal(IpctSt::kIpcStValVunkIf, vunk_val_if);
    }
  };
  void *tkey = (req != NULL) ? (req)->get_key() : NULL;
  key_vunk_if *ikey = static_cast<key_vunk_if *>(tkey);
  key_vunk_if *vunk_if_key = static_cast<key_vunk_if *>
                                    (malloc(sizeof(key_vunk_if)));
  if (!vunk_if_key || !ikey) {
    delete tmp1;
    FREE_IF_NOT_NULL(vunk_if_key);
    return UPLL_RC_ERR_GENERIC;
  }
  memcpy(vunk_if_key, ikey, sizeof(key_vunk_if));
  okey = new ConfigKeyVal(UNC_KT_VUNK_IF, IpctSt::kIpcStKeyVunkIf, vunk_if_key,
                          tmp1);
  if (okey) SET_USER_DATA(okey, req);
  return UPLL_RC_SUCCESS;
}

upll_rc_t VunkIfMoMgr::UpdateAuditConfigStatus(
    unc_keytype_configstatus_t cs_status,
    uuc::UpdateCtrlrPhase phase,
    ConfigKeyVal *&ckv_running,
    DalDmlIntf *dmi) {
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  return result_code;
}

upll_rc_t VunkIfMoMgr::UpdateConfigStatus(ConfigKeyVal *ikey,
                                          unc_keytype_operation_t op,
                                          uint32_t driver_result,
                                          ConfigKeyVal *upd_key,
                                          DalDmlIntf *dmi,
                                          ConfigKeyVal *ctrlr_key) {
  UPLL_FUNC_TRACE;
  val_vunk_if_t *vunk_if_val = static_cast<val_vunk_if_t *>(GetVal(ikey));
  val_vunk_if *vunk_if_val2 = reinterpret_cast<val_vunk_if *>(GetVal(upd_key));

  unc_keytype_configstatus_t cs_status = UNC_CS_APPLIED;
  UPLL_LOG_TRACE("Key in Candidate %s", (ikey->ToStrAll()).c_str());
  if (vunk_if_val == NULL || vunk_if_val2 == NULL) {
    UPLL_LOG_TRACE("Value of Vunknown Interface is NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  if (op == UNC_OP_CREATE) {
    vunk_if_val->cs_row_status = cs_status;
  } else if (op == UNC_OP_UPDATE) {
    void *vunkifval = reinterpret_cast<void *>(vunk_if_val);
    CompareValidValue(vunkifval, GetVal(upd_key), true);
    UPLL_LOG_TRACE("Key in Running %s", (upd_key->ToStrAll()).c_str());
    vunk_if_val->cs_row_status = vunk_if_val2->cs_row_status;
  } else {
     return UPLL_RC_ERR_GENERIC;
  }
  for (unsigned int loop = 0;
       loop < sizeof(vunk_if_val->valid)/sizeof(vunk_if_val->valid[0]);
       ++loop) {
    if ((UNC_VF_VALID == (uint8_t)vunk_if_val->valid[loop])
        || (UNC_VF_VALID_NO_VALUE == (uint8_t)vunk_if_val->valid[loop]))
      vunk_if_val->cs_attr[loop] = cs_status;
    else if ((UNC_VF_INVALID == vunk_if_val->valid[loop]) &&
             (UNC_OP_CREATE == op))
      vunk_if_val->cs_attr[loop] = UNC_CS_APPLIED;
    else if ((UNC_VF_INVALID == vunk_if_val->valid[loop]) &&
             (UNC_OP_UPDATE == op))
      vunk_if_val->cs_attr[loop] = vunk_if_val2->cs_attr[loop];
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VunkIfMoMgr::PopulateValVtnNeighbor(ConfigKeyVal *&in_ckv,
                                             DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  // Read on the left side of the link
  bool got_left_side = false;
  ConfigKeyVal *vlink_ckv = NULL;
  VlinkMoMgr *vlink_momgr = reinterpret_cast<VlinkMoMgr *>
                           (const_cast<MoManager*>(GetMoManager(UNC_KT_VLINK)));
  if (!vlink_momgr) {
    UPLL_LOG_DEBUG("Invalid Mgr");
    return UPLL_RC_ERR_GENERIC;
  }
  vn_if_type iftype;
  upll_rc_t result_code = vlink_momgr->CheckIfMemberOfVlink(in_ckv,
                               UPLL_DT_RUNNING, vlink_ckv, dmi, iftype);
  if (!vlink_ckv || result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    UPLL_LOG_TRACE("Not found in vlink table");
  } else if (result_code == UPLL_RC_SUCCESS) {
    key_vlink_t *vlink_key = reinterpret_cast<key_vlink_t *>
                                             (vlink_ckv->get_key());
    if (!vlink_key) {
      UPLL_LOG_DEBUG("Invalid param");
      delete vlink_ckv;
      return UPLL_RC_ERR_GENERIC;
    }
    if ((iftype == kVlinkBoundaryNode1) || (iftype == kVlinkInternalNode1))
     got_left_side = true;
    val_vlink_t *vlink_val = reinterpret_cast<val_vlink *>
                       (GetVal(vlink_ckv));
    val_vtn_neighbor_t *val_vtn_neighbor =
     reinterpret_cast<val_vtn_neighbor_t *>(malloc(sizeof(val_vtn_neighbor_t)));
    memset(val_vtn_neighbor, 0, sizeof(val_vtn_neighbor_t));
    val_vtn_neighbor->valid[UPLL_IDX_CONN_VNODE_NAME_VN] = UNC_VF_VALID;
    val_vtn_neighbor->valid[UPLL_IDX_CONN_VNODE_IF_NAME_VN] = UNC_VF_VALID;
    val_vtn_neighbor->valid[UPLL_IDX_CONN_VLINK_NAME_VN] = UNC_VF_VALID;
    uuu::upll_strncpy(val_vtn_neighbor->connected_vnode_name,
                      ((got_left_side) ? vlink_val->vnode2_name :
                                         vlink_val->vnode1_name),
                      (kMaxLenVnodeName + 1));
    uuu::upll_strncpy(val_vtn_neighbor->connected_if_name,
                      ((got_left_side) ? vlink_val->vnode2_ifname :
                                         vlink_val->vnode1_ifname),
                      (kMaxLenInterfaceName + 1));
    uuu::upll_strncpy(val_vtn_neighbor->connected_vlink_name,
                      vlink_key->vlink_name, (kMaxLenVnodeName + 1));
    in_ckv->SetCfgVal(new ConfigVal(IpctSt::kIpcStValVtnNeighbor,
                                    val_vtn_neighbor));
  } else {
    UPLL_LOG_DEBUG("ReadConfigDB failed result_code - %d", result_code);
  }

  if (vlink_ckv) delete vlink_ckv;
  return result_code;
}

upll_rc_t VunkIfMoMgr::CopyToConfigKey(ConfigKeyVal *&okey,
                                       ConfigKeyVal *ikey) {
  UPLL_LOG_INFO("Not supported for this keytype. Returning Generic Error");
  return UPLL_RC_ERR_GENERIC;
}

upll_rc_t VunkIfMoMgr::ValidateMessage(IpcReqRespHeader *req,
    ConfigKeyVal *ikey) {
  UPLL_FUNC_TRACE;
  upll_rc_t ret_val = UPLL_RC_ERR_GENERIC;
  if (!ikey || !req || !(ikey->get_key())) {
    UPLL_LOG_DEBUG("IpcReqRespHeader or ConfigKeyVal is Null");
    return UPLL_RC_ERR_BAD_REQUEST;
  }
  unc_key_type_t keytype = ikey->get_key_type();
  if (UNC_KT_VUNK_IF != keytype) {
    UPLL_LOG_DEBUG("Invalid keytype. Keytype- %d", keytype);
    return UPLL_RC_ERR_BAD_REQUEST;
  }
  if (ikey->get_st_num() != IpctSt::kIpcStKeyVunkIf) {
    UPLL_LOG_DEBUG(
        "Invalid structure received.Expected struct-kIpcStKeyVunkIf,"
        "received struct -%s ",
        reinterpret_cast<const char *>
        (IpctSt::GetIpcStdef(ikey->get_st_num())));
    return UPLL_RC_ERR_BAD_REQUEST;
  }
  key_vunk_if_t *key_vunk_if =
    static_cast<key_vunk_if_t *>(ikey->get_key());

  upll_keytype_datatype_t dt_type = req->datatype;
  unc_keytype_operation_t operation = req->operation;
  unc_keytype_option1_t option1 = req->option1;
  unc_keytype_option2_t option2 = req->option2;

  if (key_vunk_if == NULL) {
    UPLL_LOG_DEBUG("Key structure is empty!!");
    return UPLL_RC_ERR_BAD_REQUEST;
  }

  ret_val = ValidateVunkIfKey(key_vunk_if, operation);
  if (ret_val != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Syntax check failed for KT_VUNK_IF key structure");
    return UPLL_RC_ERR_CFG_SYNTAX;
  }

  if ((operation == UNC_OP_CREATE) && (dt_type == UPLL_DT_CANDIDATE)) {
    val_vunk_if_t *val_vunk_if = NULL;
    if ((ikey->get_cfg_val())
        && (ikey->get_cfg_val())->get_st_num() == IpctSt::kIpcStValVunkIf) {
      val_vunk_if =
        static_cast<val_vunk_if_t *>(ikey->get_cfg_val()->get_val());
    }
    if (val_vunk_if != NULL) {
      ret_val = ValidateVunkIfValue(val_vunk_if, operation);
      if (ret_val != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Syntax check failed for VUNK_IF value structure");
        return UPLL_RC_ERR_CFG_SYNTAX;
      }
      return UPLL_RC_SUCCESS;
    } else {
      UPLL_LOG_TRACE("Value structure optional for UNC_OP_CREATE");
      return UPLL_RC_SUCCESS;
    }

  } else if ((operation == UNC_OP_UPDATE) && (dt_type == UPLL_DT_CANDIDATE)) {
    val_vunk_if_t *val_vunk_if = NULL;
    if ((ikey->get_cfg_val())
        && (ikey->get_cfg_val())->get_st_num() == IpctSt::kIpcStValVunkIf) {
      val_vunk_if =
        reinterpret_cast<val_vunk_if_t *>(ikey->get_cfg_val()->get_val());
    }
    if (val_vunk_if != NULL) {
      ret_val = ValidateVunkIfValue(val_vunk_if, operation);
      if (ret_val != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Syntax check failed for VINK_IF value structure");
        return UPLL_RC_ERR_CFG_SYNTAX;
      }
      return UPLL_RC_SUCCESS;
    } else {
      UPLL_LOG_DEBUG("Value structure mandatory for UNC_OP_UPDATE operation");
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  } else if ((operation == UNC_OP_READ) && (dt_type == UPLL_DT_STATE)) {
    if (option1 == UNC_OPT1_NORMAL) {
      if (option2 == UNC_OPT2_NEIGHBOR) {
        val_vtn_neighbor_t *val_vtn_neighbor = NULL;
        if ((ikey->get_cfg_val())
            && ((ikey->get_cfg_val())->get_st_num()
                == IpctSt::kIpcStValVtnNeighbor)) {
          val_vtn_neighbor =
           static_cast<val_vtn_neighbor_t *>(ikey->get_cfg_val()->get_val());
        }
        if (val_vtn_neighbor != NULL) {
          ret_val = ValidateVtnNeighValue(val_vtn_neighbor);
          if (ret_val != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("Syntax check failed for VtnNeighValue structure");
            return ret_val;
          } else {
            UPLL_LOG_TRACE("Syntax check success for VUNKIF value structure");
            return ret_val;
          }
        } else {
          UPLL_LOG_TRACE("Value structure is optional");
          return ret_val;
        }
      } else if (option2 != UNC_OPT2_NONE) {
        UPLL_LOG_DEBUG("option2 is not matching");
        return UPLL_RC_ERR_INVALID_OPTION2;
      }
    } else {
      UPLL_LOG_DEBUG("option1 is not matching");
      return UPLL_RC_ERR_INVALID_OPTION1;
    }
  } else if ((operation == UNC_OP_READ)
      && ((dt_type == UPLL_DT_CANDIDATE) || (dt_type == UPLL_DT_RUNNING)
          || (dt_type == UPLL_DT_STARTUP))) {
    if (option1 == UNC_OPT1_NORMAL) {
      if (option2 == UNC_OPT2_NEIGHBOR) {
        val_vtn_neighbor_t *val_vtn_neighbor = NULL;
        if ((ikey->get_cfg_val())
            && (ikey->get_cfg_val())->get_st_num()
                == IpctSt::kIpcStValVtnNeighbor) {
          val_vtn_neighbor =
            static_cast<val_vtn_neighbor_t *>(ikey->get_cfg_val()->get_val());
        }
        if (val_vtn_neighbor != NULL) {
          ret_val = ValidateVtnNeighValue(val_vtn_neighbor);
          if (ret_val != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("Syntax check failed for VtnNeighValue structure");
            return ret_val;
          } else {
            UPLL_LOG_TRACE("Syntax check success for VUNKIF value structure");
            return ret_val;
          }
        } else {
          UPLL_LOG_TRACE("Value structure is optional");
          return ret_val;
        }
      } else if (option2 != UNC_OPT2_NONE) {
        UPLL_LOG_DEBUG("option2 is not matching");
        return UPLL_RC_ERR_INVALID_OPTION2;
      }
    } else {
      UPLL_LOG_DEBUG("option1 is not matching");
      return UPLL_RC_ERR_INVALID_OPTION1;
    }
  } else if (((operation == UNC_OP_READ_SIBLING)
      || (operation == UNC_OP_READ_SIBLING_BEGIN)
      || (operation == UNC_OP_READ_SIBLING_COUNT))
      && ((dt_type == UPLL_DT_CANDIDATE) || (dt_type == UPLL_DT_RUNNING)
          || (dt_type == UPLL_DT_STARTUP) || (dt_type == UPLL_DT_STATE))) {
    if (option1 == UNC_OPT1_NORMAL) {
      if (option2 == UNC_OPT2_NONE) {
        val_vunk_if_t *val_vunk_if = NULL;
        if ((ikey->get_cfg_val())
            && ((ikey->get_cfg_val())->get_st_num() ==
                                              IpctSt::kIpcStValVunkIf)) {
          val_vunk_if =
              static_cast<val_vunk_if_t *>(ikey->get_cfg_val()->get_val());
        }
        if (val_vunk_if != NULL) {
          ret_val = ValidateVunkIfValue(val_vunk_if, operation);
          if (ret_val != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("Syntax check failed for VUNK_IF value structure");
            return ret_val;
          } else {
            UPLL_LOG_TRACE("Syntax check success for VUNKIF value structure");
            return ret_val;
          }
        } else {
          UPLL_LOG_TRACE("Value structure optional for READ operations");
          return ret_val;
        }
      } else {
        UPLL_LOG_DEBUG("option2 is not matching");
        return UPLL_RC_ERR_INVALID_OPTION2;
      }
    } else {
      UPLL_LOG_DEBUG("option1 is not matching");
      return UPLL_RC_ERR_INVALID_OPTION1;
    }
  } else if ((operation == UNC_OP_DELETE) && (dt_type == UPLL_DT_CANDIDATE)) {
    UPLL_LOG_TRACE("Value structure is none for this operation :%d", operation);
    return UPLL_RC_SUCCESS;
  } else if (((operation == UNC_OP_READ_NEXT) ||
        (operation == UNC_OP_READ_BULK)) &&
      ((dt_type == UPLL_DT_CANDIDATE) ||
       (dt_type == UPLL_DT_RUNNING) ||
       (dt_type == UPLL_DT_STARTUP))) {
    UPLL_LOG_TRACE("Value structure is none for this operation :%d", operation);
    return UPLL_RC_SUCCESS;
  } else {
    UPLL_LOG_DEBUG("Invalid datatype(%d) and operation(%d)", dt_type,
        operation);
    return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VunkIfMoMgr::ValidateVunkIfValue(val_vunk_if_t *val_vunk_if,
    unc_keytype_operation_t operation) {
  UPLL_FUNC_TRACE;
  if (val_vunk_if->valid[UPLL_IDX_DESC_VUNI] == UNC_VF_VALID) {
    if (!ValidateDesc(val_vunk_if->description,
        kMinLenDescription, kMaxLenDescription)) {
      UPLL_LOG_DEBUG("Syntax check failed desc:(%s)", val_vunk_if->description);
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  } else if ((val_vunk_if->valid[UPLL_IDX_DESC_VUNI] == UNC_VF_VALID_NO_VALUE)
      && ((operation == UNC_OP_UPDATE) || (operation == UNC_OP_CREATE))) {
    val_vunk_if->description[0] = '\0';
  }
  if (val_vunk_if->valid[UPLL_IDX_ADMIN_ST_VUNI] == UNC_VF_VALID) {
    if (!ValidateNumericRange(val_vunk_if->admin_status,
          (uint8_t) UPLL_ADMIN_ENABLE,
          (uint8_t) UPLL_ADMIN_DISABLE, true, true)) {
      UPLL_LOG_DEBUG("Syntax check failed admst:%d", val_vunk_if->admin_status);
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  } else if ((val_vunk_if->valid[UPLL_IDX_ADMIN_ST_VUNI]
        == UNC_VF_VALID_NO_VALUE)
      && ((operation == UNC_OP_UPDATE) || (operation == UNC_OP_CREATE))) {
    val_vunk_if->admin_status = UPLL_ADMIN_ENABLE;
  } else if ((val_vunk_if->valid[UPLL_IDX_ADMIN_ST_VUNI] == UNC_VF_INVALID)
                                           && (operation == UNC_OP_CREATE)) {
    val_vunk_if->valid[UPLL_IDX_ADMIN_ST_VUNI] = UNC_VF_VALID_NO_VALUE;
    val_vunk_if->admin_status = UPLL_ADMIN_ENABLE;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VunkIfMoMgr::ValidateVtnNeighValue(
    val_vtn_neighbor_t *val_vtn_neighbor) {
  UPLL_FUNC_TRACE;
  upll_rc_t ret_val = UPLL_RC_SUCCESS;

  if (val_vtn_neighbor->valid[UPLL_IDX_CONN_VNODE_NAME_VN] == UNC_VF_VALID) {
    ret_val = ValidateKey(
        reinterpret_cast<char *>(val_vtn_neighbor->connected_vnode_name),
        kMinLenVnodeName, kMaxLenVnodeName);
    if (ret_val != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Syntax check failed.conn_vnode_name-(%s)",
          val_vtn_neighbor->connected_vnode_name);
      return ret_val;
    }
  }
  if (val_vtn_neighbor->valid[UPLL_IDX_CONN_VNODE_IF_NAME_VN]
      == UNC_VF_VALID) {
    ret_val = ValidateKey(
        reinterpret_cast<char *>(val_vtn_neighbor->connected_if_name),
        kMinLenInterfaceName, kMaxLenInterfaceName);
    if (ret_val != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Syntax check failed. connected_if_name-%s",
          val_vtn_neighbor->connected_if_name);
      return ret_val;
    }
  }
  if (val_vtn_neighbor->valid[UPLL_IDX_CONN_VLINK_NAME_VN] == UNC_VF_VALID) {
    ret_val = ValidateKey(
        reinterpret_cast<char *>(val_vtn_neighbor->connected_vlink_name),
        kMinLenVlinkName, kMaxLenVlinkName);
    if (ret_val != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Syntax check failed.connected_vlink_name=%s",
          val_vtn_neighbor->connected_vlink_name);
      return ret_val;
    }
  }
  return UPLL_RC_SUCCESS;
}
upll_rc_t VunkIfMoMgr::ValidateVunkIfKey(key_vunk_if_t *key_vunk_if,
    unc_keytype_operation_t operation) {
  UPLL_FUNC_TRACE;
  upll_rc_t ret_val = UPLL_RC_SUCCESS;
  /** Validate vtn_key structure */
  VunknownMoMgr *objvunkmgr =
    reinterpret_cast<VunknownMoMgr *>(const_cast<MoManager *>(GetMoManager(
            UNC_KT_VUNKNOWN)));
  if (NULL == objvunkmgr) {
    UPLL_LOG_DEBUG("unable to get VunknownMoMgr to validate key_vunknown");
    return UPLL_RC_ERR_GENERIC;
  }
  ret_val = objvunkmgr->ValidateVunknownKey(&(key_vunk_if->vunk_key));

  if (UPLL_RC_SUCCESS != ret_val) {
    UPLL_LOG_DEBUG("Syntax validation failed.Err Code- %d", ret_val);
    return ret_val;
  }
  if ((operation != UNC_OP_READ_SIBLING_BEGIN) &&
      (operation != UNC_OP_READ_SIBLING_COUNT)) {
    ret_val = ValidateKey(reinterpret_cast<char *>(key_vunk_if->if_name),
        kMinLenInterfaceName, kMaxLenInterfaceName);

    if (ret_val != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Syntax check failed.if_name-(%s)", key_vunk_if->if_name);
      return ret_val;
    }
  } else {
    UPLL_LOG_TRACE("Operation is %d", operation);
    StringReset(key_vunk_if->if_name);
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VunkIfMoMgr::IsReferenced(IpcReqRespHeader *req,
                                    ConfigKeyVal *ikey,
                                    DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  ConfigKeyVal *okey = NULL;
  if (NULL == ikey)
     return UPLL_RC_ERR_GENERIC;
  GetChildConfigKey(okey, ikey);
  DbSubOp dbop = { kOpReadMultiple, kOpMatchNone, kOpInOutFlag };
  upll_rc_t result_code = ReadConfigDB(okey, req->datatype, UNC_OP_READ,
                                       dbop, dmi, MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    result_code = (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE)?
                    UPLL_RC_SUCCESS:result_code;
    if (okey) delete okey;
    return result_code;
  }
  ConfigKeyVal *temkey = okey;
  while (temkey != NULL) {
    uint8_t vlink_flag = 0;
    GET_USER_DATA_FLAGS(temkey, vlink_flag);
    if (vlink_flag & VIF_TYPE) {
      delete okey;
      return UPLL_RC_ERR_CFG_SEMANTIC;
    }
    temkey = temkey->get_next_cfg_key_val();
  }
  delete okey;
  return UPLL_RC_SUCCESS;
}

}  // namespace kt_momgr
}  // namespace upll
}  // namespace unc
