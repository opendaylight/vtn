/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include "vlanmap_momgr.hh"
#include <ctype.h>
#define NUM_KEY_MAIN_TBL_ 5
#define INVALID_LOG_PORT_ID_VALID 0xFF
namespace unc {
namespace upll {
namespace kt_momgr {
uint16_t VlanMapMoMgr::kVbrVlanMapNumChildKey = 2;

BindInfo VlanMapMoMgr::vlan_map_bind_info[] = {
    { uudst::vbridge_vlanmap::kDbiVtnName, CFG_KEY, offsetof(
        key_vlan_map, vbr_key.vtn_key.vtn_name),
      uud::kDalChar, 32 },
    { uudst::vbridge_vlanmap::kDbiVbrName, CFG_KEY, offsetof(
        key_vlan_map, vbr_key.vbridge_name),
      uud::kDalChar, 32 },
    { uudst::vbridge_vlanmap::kDbiLogicalPortId, CFG_KEY, offsetof(
        key_vlan_map, logical_port_id),
      uud::kDalChar, 320 },
    { uudst::vbridge_vlanmap::kDbiLogicalPortIdValid, CFG_KEY, offsetof(
        key_vlan_map, logical_port_id_valid),
      uud::kDalUint8, 1 },
    { uudst::vbridge_vlanmap::kDbiVlanid, CFG_VAL, offsetof(val_vlan_map,
                                                            vlan_id),
      uud::kDalUint16, 1 },
    { uudst::vbridge_vlanmap::kDbiCtrlrName, CK_VAL, offsetof(key_user_data_t,
                                                              ctrlr_id),
      uud::kDalChar, 32 },
    { uudst::vbridge_vlanmap::kDbiDomainId, CK_VAL, offsetof(key_user_data_t,
                                                             domain_id),
      uud::kDalChar, 32 },
    { uudst::vbridge_vlanmap::kDbiValidVlanid, CFG_META_VAL, offsetof(
        val_vlan_map, valid[0]),
      uud::kDalUint8, 1 },
    { uudst::vbridge_vlanmap::kDbiCsVlanid, CS_VAL, offsetof(val_vlan_map,
                                                             cs_attr[0]),
      uud::kDalUint8, 1 },
    { uudst::vbridge_vlanmap::kDbiCsRowStatus, CS_VAL, offsetof(val_vlan_map,
                                                                cs_row_status),
      uud::kDalUint8, 1 },
    { uudst::vbridge_vlanmap::kDbiVbrVlanMapFlags, CK_VAL, offsetof(
        key_user_data_t, flags),
      uud::kDalUint8, 1 }, };

BindInfo VlanMapMoMgr::vlan_map_maintbl_update_key_bind_info[] = {
    { uudst::vbridge_vlanmap::kDbiVtnName, CFG_MATCH_KEY, offsetof(
        key_vlan_map, vbr_key.vtn_key.vtn_name),
      uud::kDalChar, (kMaxLenVtnName+1) },
    { uudst::vbridge_vlanmap::kDbiVbrName, CFG_MATCH_KEY, offsetof(
        key_vlan_map, vbr_key.vbridge_name),
      uud::kDalChar, (kMaxLenVnodeName+1) },
    { uudst::vbridge_vlanmap::kDbiVtnName, CFG_INPUT_KEY, offsetof(
        key_rename_vnode_info_t, new_unc_vtn_name),
      uud::kDalChar, (kMaxLenVtnName+1) },
    { uudst::vbridge_vlanmap::kDbiVbrName, CFG_INPUT_KEY, offsetof(
        key_rename_vnode_info_t, new_unc_vnode_name),
      uud::kDalChar, (kMaxLenVnodeName+1) },
    { uudst::vbridge_vlanmap::kDbiVbrVlanMapFlags, CK_VAL, offsetof(
        key_user_data_t, flags),
      uud::kDalUint8, 1 },
};

VlanMapMoMgr::VlanMapMoMgr() {
  UPLL_FUNC_TRACE
  ntable = MAX_MOMGR_TBLS;
  table = new Table *[ntable];
  table[MAINTBL] = new Table(uudst::kDbiVbrVlanMapTbl, UNC_KT_VBR_VLANMAP,
                   vlan_map_bind_info, IpctSt::kIpcStKeyVlanMap,
                   IpctSt::kIpcStValVlanMap,
                   uudst::vbridge_vlanmap::kDbiVbrVlanMapNumCols);
  table[RENAMETBL] = NULL;
  table[CTRLRTBL] = NULL;
  nchild = 0;
  child = NULL;
}

/*
 * Based on the key type the bind info will pass
 **/
bool VlanMapMoMgr::GetRenameKeyBindInfo(unc_key_type_t key_type,
                                        BindInfo *&binfo,
                                        int &nattr,
                                        MoMgrTables tbl) {
  /* Main Table or rename table only update */
  if (MAINTBL == tbl) {
    nattr = NUM_KEY_MAIN_TBL_;
    binfo = vlan_map_maintbl_update_key_bind_info;
  } else {
    UPLL_LOG_DEBUG("Invalid Table ");
    return PFC_FALSE;
  }
  return PFC_TRUE;
}

bool VlanMapMoMgr::IsValidKey(void *key,
                              uint64_t index) {
  key_vlan_map *vlanmap_key = reinterpret_cast<key_vlan_map *>(key);
  UPLL_FUNC_TRACE;
  upll_rc_t ret_val = UPLL_RC_SUCCESS;
  switch (index) {
    case uudst::vbridge_vlanmap::kDbiVtnName:
      ret_val = ValidateKey(reinterpret_cast<char *>
                           (vlanmap_key->vbr_key.vtn_key.vtn_name),
                           kMinLenVtnName, kMaxLenVtnName);
      if (ret_val != UPLL_RC_SUCCESS) {
        UPLL_LOG_TRACE(
            "syntax check failure for key_vlan_map->vbr_key.vtn_key.vtn_name");
        return false;
      }
      break;
    case uudst::vbridge_vlanmap::kDbiVbrName:
      ret_val = ValidateKey(reinterpret_cast<char *>
                           (vlanmap_key->vbr_key.vbridge_name),
                           kMinLenVnodeName, kMaxLenVnodeName);
      if (ret_val != UPLL_RC_SUCCESS) {
        UPLL_LOG_TRACE("VBR Name is not valid(%d)", ret_val);
        return false;
      }
      break;
    case uudst::vbridge_vlanmap::kDbiLogicalPortId:
      if (!ValidateLogicalPortId(reinterpret_cast<char *>
                           (vlanmap_key->logical_port_id),
                           kMinLenLogicalPortId, kMaxLenLogicalPortId)) {
        UPLL_LOG_TRACE("Switch ID is not valid");
        return false;
      }
      break;
    case uudst::vbridge_vlanmap::kDbiLogicalPortIdValid:
      if (!ValidateNumericRange(vlanmap_key->logical_port_id_valid, PFC_FALSE,
                            PFC_TRUE, true, true)) {
        UPLL_LOG_TRACE("LogicalPortId validis not valid");
        return false;
      }
      break;
    default:
      UPLL_LOG_TRACE("Invalid Key Index");
      break;
  }
  return true;
}

upll_rc_t VlanMapMoMgr::GetChildConfigKey(ConfigKeyVal *&okey,
                                          ConfigKeyVal *parent_key) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  key_vlan_map_t *vlanmap_key;
  void *pkey;
  if (parent_key == NULL) {
    vlanmap_key = reinterpret_cast<key_vlan_map_t *>
                  (ConfigKeyVal::Malloc(sizeof(key_vlan_map_t)));
    vlanmap_key->logical_port_id_valid = INVALID_LOG_PORT_ID_VALID;
    if (okey) delete okey;
    okey = new ConfigKeyVal(UNC_KT_VBR_VLANMAP, IpctSt::kIpcStKeyVlanMap,
                            vlanmap_key, NULL);
    return UPLL_RC_SUCCESS;
  } else {
    pkey = parent_key->get_key();
  }
  if (!pkey) return UPLL_RC_ERR_GENERIC;
  if (okey) {
    if (okey->get_key_type() != UNC_KT_VBR_VLANMAP) 
      return UPLL_RC_ERR_GENERIC;
    vlanmap_key = reinterpret_cast<key_vlan_map_t *>(okey->get_key());
  } else {
    vlanmap_key = reinterpret_cast<key_vlan_map_t *>
                  (ConfigKeyVal::Malloc(sizeof(key_vlan_map_t)));
    vlanmap_key->logical_port_id_valid = INVALID_LOG_PORT_ID_VALID;
  }
  unc_key_type_t keytype = parent_key->get_key_type();
  switch (keytype) {
    case UNC_KT_VTN:
      uuu::upll_strncpy(vlanmap_key->vbr_key.vtn_key.vtn_name,
              reinterpret_cast<key_vtn *>(pkey)->vtn_name,
              (kMaxLenVtnName+1));
      break;
    case UNC_KT_VBRIDGE:
      uuu::upll_strncpy(vlanmap_key->vbr_key.vtn_key.vtn_name,
             (reinterpret_cast<key_vbr *>(pkey))->vtn_key.vtn_name,
             kMaxLenVtnName+1);
      uuu::upll_strncpy(vlanmap_key->vbr_key.vbridge_name,
             (reinterpret_cast<key_vbr *>(pkey))->vbridge_name,
             kMaxLenVnodeName+1);
      break;
    case UNC_KT_VBR_VLANMAP:
      uuu::upll_strncpy(
           vlanmap_key->vbr_key.vtn_key.vtn_name,
           (reinterpret_cast<key_vlan_map_t *>(pkey))->vbr_key.vtn_key.vtn_name,
           kMaxLenVtnName+1);
      uuu::upll_strncpy(vlanmap_key->vbr_key.vbridge_name,
           (reinterpret_cast<key_vlan_map_t *>(pkey))->vbr_key.vbridge_name,
           kMaxLenVnodeName+1);
      vlanmap_key->logical_port_id_valid =
          (uint8_t)(reinterpret_cast<key_vlan_map_t *>
          (pkey))->logical_port_id_valid;
      if (vlanmap_key->logical_port_id_valid)
          uuu::upll_strncpy(vlanmap_key->logical_port_id,
          (reinterpret_cast<key_vlan_map_t *>(pkey))->logical_port_id,
          kMaxLenLogicalPortId);
      else
        *vlanmap_key->logical_port_id = '\0';
    default:
      break;
  }
  if (!okey)
    okey = new ConfigKeyVal(UNC_KT_VBR_VLANMAP, IpctSt::kIpcStKeyVlanMap,
                            vlanmap_key, NULL);
  else if (okey->get_key() != vlanmap_key)
    okey->SetKey(IpctSt::kIpcStKeyVlanMap, vlanmap_key);

  if (okey == NULL) {
    free(vlanmap_key);
    result_code = UPLL_RC_ERR_GENERIC;
  } else {
    SET_USER_DATA(okey, parent_key);
  }
  return result_code;
}

upll_rc_t VlanMapMoMgr::GetParentConfigKey(ConfigKeyVal *&okey,
                                           ConfigKeyVal *ikey) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  if (!ikey) {
    UPLL_LOG_DEBUG("ikey is NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  unc_key_type_t ikey_type = ikey->get_key_type();

  if (ikey_type != UNC_KT_VBR_VLANMAP) return UPLL_RC_ERR_GENERIC;
  key_vlan_map_t *pkey = reinterpret_cast<key_vlan_map_t *>(ikey->get_key());
  if (!pkey) return UPLL_RC_ERR_GENERIC;
  key_vbr *vbr_key = reinterpret_cast<key_vbr *>
    (ConfigKeyVal::Malloc(sizeof(key_vbr)));
  uuu::upll_strncpy(vbr_key->vtn_key.vtn_name,
         (reinterpret_cast<key_vlan_map_t *>(pkey))->vbr_key.vtn_key.vtn_name,
         kMaxLenVtnName+1);
  uuu::upll_strncpy(vbr_key->vbridge_name,
         (reinterpret_cast<key_vlan_map_t *>(pkey))->vbr_key.vbridge_name,
         kMaxLenVnodeName+1);
  okey = new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVbr, vbr_key, NULL);
  if (okey == NULL) {
    free(vbr_key);
    result_code = UPLL_RC_ERR_GENERIC;
  } else {
    SET_USER_DATA(okey, ikey);
  }
  return result_code;
}

upll_rc_t VlanMapMoMgr::AllocVal(ConfigVal *&ck_val,
                                 upll_keytype_datatype_t dt_type,
                                 MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  void *val;

  if (ck_val != NULL) return UPLL_RC_ERR_GENERIC;
  switch (tbl) {
    case MAINTBL:
      val = reinterpret_cast<void *>(
                     ConfigKeyVal::Malloc(sizeof(val_vlan_map)));
      ck_val = new ConfigVal(IpctSt::kIpcStValVlanMap, val);
      break;
    default:
      val = NULL;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VlanMapMoMgr::DupConfigKeyVal(ConfigKeyVal *&okey,
                                        ConfigKeyVal *&req,
                                        MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  if (req == NULL) return UPLL_RC_ERR_GENERIC;
  if (okey != NULL) return UPLL_RC_ERR_GENERIC;
  if (req->get_key_type() != UNC_KT_VBR_VLANMAP) return UPLL_RC_ERR_GENERIC;
  ConfigVal *tmp1 = NULL, *tmp = (req)->get_cfg_val();

  if (tmp) {
    if (tbl == MAINTBL) {
      val_vlan_map *ival = reinterpret_cast<val_vlan_map *>(GetVal(req));
      if (!ival) {
        UPLL_LOG_DEBUG("Invalid vlanmap");
        return UPLL_RC_ERR_GENERIC;
      }
      val_vlan_map *vlanmap_val = reinterpret_cast<val_vlan_map *>
                                  (ConfigKeyVal::Malloc(sizeof(val_vlan_map)));
      memcpy(vlanmap_val, ival, sizeof(val_vlan_map));
      tmp1 = new ConfigVal(IpctSt::kIpcStValVlanMap, vlanmap_val);
    }
  };
  void *tkey = (req != NULL) ? (req)->get_key() : NULL;
  key_vlan_map_t *ikey = reinterpret_cast<key_vlan_map_t *>(tkey);
  key_vlan_map_t *vlanmap_key = reinterpret_cast<key_vlan_map_t *>
                                (ConfigKeyVal::Malloc(sizeof(key_vlan_map_t)));
  memcpy(vlanmap_key, ikey, sizeof(key_vlan_map_t));
  okey = new ConfigKeyVal(UNC_KT_VBR_VLANMAP, IpctSt::kIpcStKeyVlanMap,
                          vlanmap_key, tmp1);
  if (okey) {
    SET_USER_DATA(okey, req);
  } else {
    delete tmp1;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VlanMapMoMgr::UpdateConfigStatus(ConfigKeyVal *ikey,
                                           unc_keytype_operation_t op,
                                           uint32_t driver_result,
                                           ConfigKeyVal *upd_key,
                                           DalDmlIntf *dmi,
                                           ConfigKeyVal *ctrlr_key) {
  UPLL_FUNC_TRACE;
  val_vlan_map_t *vlanmap_val = reinterpret_cast<val_vlan_map_t*>(GetVal(ikey));

  unc_keytype_configstatus_t cs_status =
      (driver_result == UPLL_RC_SUCCESS) ? UNC_CS_APPLIED : UNC_CS_NOT_APPLIED;
  val_vlan_map *vlanmap_val2 = reinterpret_cast<val_vlan_map *>(GetVal(upd_key));
  UPLL_LOG_TRACE("Key from Candidate %s",(ikey->ToStrAll()).c_str());
  if (vlanmap_val == NULL) return UPLL_RC_ERR_GENERIC;
  if (op == UNC_OP_CREATE) {
    vlanmap_val->cs_row_status = cs_status;
  } else if (op == UNC_OP_UPDATE) {
    void *val = reinterpret_cast<void *>(vlanmap_val);
    CompareValidValue(val, GetVal(upd_key), true);
    UPLL_LOG_TRACE("Key from Running %s",(upd_key->ToStrAll()).c_str());
    vlanmap_val->cs_row_status = vlanmap_val2->cs_row_status;
  } else if (op != UNC_OP_CREATE) {
    return UPLL_RC_ERR_GENERIC;
  }
  for (unsigned int loop = 0;
       loop < sizeof(vlanmap_val->valid) / sizeof(vlanmap_val->valid[0]);
       ++loop) {
    if ((UNC_VF_VALID == (uint8_t) vlanmap_val->valid[loop])
        || (UNC_VF_VALID_NO_VALUE == (uint8_t) vlanmap_val->valid[loop])) {
      vlanmap_val->cs_attr[loop] = cs_status;
    } else if ((UNC_VF_INVALID == vlanmap_val->valid[loop]) &&
               (UNC_OP_CREATE == op)) {
        vlanmap_val->cs_attr[loop] = UNC_CS_APPLIED;
    } else if ((UNC_VF_INVALID == vlanmap_val->valid[loop]) &&
               (UNC_OP_UPDATE == op)) {
        vlanmap_val->cs_attr[loop] = vlanmap_val2->cs_attr[loop];
    }
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VlanMapMoMgr::UpdateAuditConfigStatus(
                                    unc_keytype_configstatus_t cs_status,
                                    uuc::UpdateCtrlrPhase phase,
                                    ConfigKeyVal *&ckv_running) {
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  val_vlan_map_t *val;
  val = (ckv_running != NULL) ? reinterpret_cast<val_vlan_map_t *>
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
    if ((cs_status == UNC_CS_INVALID &&
        UNC_VF_VALID == val->valid[loop]) || cs_status == UNC_CS_APPLIED)
      val->cs_attr[loop] = cs_status;
  }
  return result_code;
}

uint8_t* VlanMapMoMgr::GetControllerId(ConfigKeyVal *ck_vbr,
                                       upll_keytype_datatype_t dt_type,
                                       DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutNone };
  if (ck_vbr->get_key_type() != UNC_KT_VBRIDGE) return NULL;
  MoMgrImpl *mgr = reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>
                                      (GetMoManager(UNC_KT_VBRIDGE)));
  upll_rc_t result_code = mgr->ReadConfigDB(ck_vbr, dt_type, UNC_OP_READ, dbop,
                                            dmi, MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    return NULL;
  }
  val_vbr_t *vbr_val = reinterpret_cast<val_vbr_t *>(GetVal(ck_vbr));
  if (vbr_val) return (vbr_val->controller_id);
  else
    return NULL;
}

upll_rc_t VlanMapMoMgr::CopyToConfigKey(ConfigKeyVal *&okey,
                                        ConfigKeyVal *ikey) {
  if (!ikey || !(ikey->get_key())) return UPLL_RC_ERR_GENERIC;

  key_rename_vnode_info *key_rename =
             reinterpret_cast<key_rename_vnode_info *>(ikey->get_key());
  key_vlan_map_t* key_vlan_map = reinterpret_cast<key_vlan_map_t *>
                      (ConfigKeyVal::Malloc(sizeof(key_vlan_map_t)));
  key_vlan_map->logical_port_id_valid = INVALID_LOG_PORT_ID_VALID; 
  if (!strlen(reinterpret_cast<char *>(key_rename->old_unc_vtn_name))) {
    free(key_vlan_map);
    return UPLL_RC_ERR_GENERIC;
  }
  uuu::upll_strncpy(key_vlan_map->vbr_key.vtn_key.vtn_name,
         key_rename->old_unc_vtn_name,
         kMaxLenVtnName+1);
  if (ikey->get_key_type() == UNC_KT_VBRIDGE) {
    if (!strlen(reinterpret_cast<char *>(key_rename->old_unc_vnode_name))) {
      free(key_vlan_map);
      return UPLL_RC_ERR_GENERIC;
    }
    uuu::upll_strncpy(key_vlan_map->vbr_key.vbridge_name,
            key_rename->old_unc_vnode_name, kMaxLenVnodeName+1);
  } else {
    if (!strlen(reinterpret_cast<char *>(key_rename->new_unc_vnode_name))) {
      free(key_vlan_map);
      return UPLL_RC_ERR_GENERIC;
    }
    uuu::upll_strncpy(key_vlan_map->vbr_key.vbridge_name,
                      key_rename->new_unc_vnode_name, (kMaxLenVnodeName + 1));
  }

  okey = new ConfigKeyVal(UNC_KT_VBR_VLANMAP, IpctSt::kIpcStKeyVlanMap,
                          key_vlan_map, NULL);

  if (!okey) {
    free(key_vlan_map);
    return UPLL_RC_ERR_GENERIC;
  }

  return UPLL_RC_SUCCESS;
}

upll_rc_t VlanMapMoMgr::ValidateMessage(IpcReqRespHeader *req,
                                        ConfigKeyVal *ikey) {
  UPLL_FUNC_TRACE;
  if (!ikey || !req || !(ikey->get_key())) {
    UPLL_LOG_DEBUG("ConfigKeyVal / IpcReqRespHeader is Null");
    return UPLL_RC_ERR_GENERIC;
  }
  upll_keytype_datatype_t dt_type = req->datatype;
  unc_keytype_operation_t op = req->operation;
  unc_keytype_option1_t opt1 = req->option1;
  unc_keytype_option2_t opt2 = req->option2;

  upll_rc_t ret_val = UPLL_RC_SUCCESS;
  if (ikey->get_st_num() != IpctSt::kIpcStKeyVlanMap) {
    UPLL_LOG_DEBUG("Invalid Key structure received. received struct - %d",
             (ikey->get_st_num()));
    return UPLL_RC_ERR_BAD_REQUEST;
  }
  key_vlan_map *vlan_map_key =
             reinterpret_cast<key_vlan_map *>((ikey->get_key()));

  unc_key_type_t ktype = ikey->get_key_type();
  if (UNC_KT_VBR_VLANMAP != ktype) {
    UPLL_LOG_DEBUG("Invalid Keytype received. received keytype %d", ktype);
    return UPLL_RC_ERR_BAD_REQUEST;
  }
  ret_val = ValidateVlanmapKey(vlan_map_key, op);
  if (ret_val != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("syntax check failed for key_vlan_map struct");
    return UPLL_RC_ERR_CFG_SYNTAX;
  }

  if (op == UNC_OP_CREATE || op == UNC_OP_UPDATE) {
    if (dt_type == UPLL_DT_CANDIDATE || UPLL_DT_IMPORT == dt_type) {
      ConfigVal *cfg_val = ikey->get_cfg_val();
      if (!cfg_val)  {
        UPLL_LOG_DEBUG("Value structure mandatory");
        return UPLL_RC_ERR_BAD_REQUEST;
      }
      if (cfg_val->get_st_num() != IpctSt::kIpcStValVlanMap) {
        UPLL_LOG_DEBUG("Invalid Value structure received. received struct - %d",
                      (cfg_val->get_st_num()));
        return UPLL_RC_ERR_BAD_REQUEST;
      }
      val_vlan_map *vlanmap_val =
          reinterpret_cast<val_vlan_map *>(cfg_val->get_val());
      if (!vlanmap_val) {
        UPLL_LOG_DEBUG("val_vlan_map struct is mandatory for Create/update op");
        return UPLL_RC_ERR_BAD_REQUEST;
      }
      ret_val = ValidateVlanmapValue(vlanmap_val, op);
      if (ret_val != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("syntax check failure for val_vlan_map value structure");
        return UPLL_RC_ERR_CFG_SYNTAX;
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
      ConfigVal *cfg_val = ikey->get_cfg_val();
      if (!cfg_val) return UPLL_RC_SUCCESS;
      if (cfg_val->get_st_num() != IpctSt::kIpcStValVlanMap) {
        UPLL_LOG_DEBUG("Invalid Value structure received. received struct - %d",
                      (cfg_val->get_st_num()));
        return UPLL_RC_ERR_BAD_REQUEST;
      }
      val_vlan_map *vlanmap_val =
          reinterpret_cast<val_vlan_map *>(cfg_val->get_val());
      if (vlanmap_val == NULL) {
        UPLL_LOG_DEBUG("syntax check for val_vlan_map struct is an optional");
        return UPLL_RC_SUCCESS;
      }
      ret_val = ValidateVlanmapValue(vlanmap_val, op);
      if (ret_val != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("syntax check failure for val_vlan_map value structure");
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
  } else if (op == UNC_OP_DELETE) {
    if(dt_type == UPLL_DT_CANDIDATE) {
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

upll_rc_t VlanMapMoMgr::ValidateVlanmapValue(val_vlan_map *vlanmap_val,
                                             uint32_t op) {
  UPLL_FUNC_TRACE;
  if (vlanmap_val == NULL) {
    UPLL_LOG_DEBUG("NULL check failure for val_vlan_map struct");
    return UPLL_RC_ERR_GENERIC;
  }
  if (vlanmap_val->valid[UPLL_IDX_VLAN_ID_VM] == UNC_VF_VALID) {
    
    if ((vlanmap_val->vlan_id != 0xFFFF) &&
        !ValidateNumericRange(vlanmap_val->vlan_id, (uint16_t) kMinVlanId,
                              (uint16_t) kMaxVlanId, true, true)) {
      UPLL_LOG_DEBUG("Numeric range check failed.  vlan_id - %d",
                    vlanmap_val->vlan_id);
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  } else if (vlanmap_val->valid[UPLL_IDX_VLAN_ID_VM] == UNC_VF_VALID_NO_VALUE
      && (op == UNC_OP_CREATE || op == UNC_OP_UPDATE)) {
    vlanmap_val->vlan_id = 0;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VlanMapMoMgr::ValidateVlanmapKey(key_vlan_map *vlan_map_key,
                                    unc_keytype_operation_t operation) {
  UPLL_FUNC_TRACE;
  upll_rc_t ret_val = UPLL_RC_SUCCESS;
  ret_val = ValidateKey(reinterpret_cast<char *>
                       (vlan_map_key->vbr_key.vtn_key.vtn_name),
                       kMinLenVnodeName, kMaxLenVnodeName);

  if (ret_val != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("syntax check failed. vtn_name - %s",
                  vlan_map_key->vbr_key.vtn_key.vtn_name);
    return UPLL_RC_ERR_CFG_SYNTAX;
  }
  ret_val = ValidateKey(reinterpret_cast<char *>
                        (vlan_map_key->vbr_key.vbridge_name),
                        kMinLenVnodeName, kMaxLenVnodeName);
  if (ret_val != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("syntax check failed. vbridge_name - %s",
                  vlan_map_key->vbr_key.vbridge_name);
    return UPLL_RC_ERR_CFG_SYNTAX;
  }
  if ((operation != UNC_OP_READ_SIBLING_BEGIN) &&
      (operation != UNC_OP_READ_SIBLING_COUNT)) {
    if (vlan_map_key->logical_port_id_valid == PFC_TRUE) {
      if (!ValidateLogicalPortId(
                 reinterpret_cast<char *>(vlan_map_key->logical_port_id),
                 kMinLenLogicalPortId, kMaxLenLogicalPortId)) {
        ret_val = UPLL_RC_ERR_CFG_SYNTAX;
      }
    }   
    else
      ret_val = UPLL_RC_SUCCESS;
  if (ret_val != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("syntax check failed. logical_port_id - %s",
                  vlan_map_key->logical_port_id);
    return UPLL_RC_ERR_CFG_SYNTAX;
  }
  if (toupper(vlan_map_key->logical_port_id[0]) == 'P'
      && toupper(vlan_map_key->logical_port_id[1]) == 'P') {
    UPLL_LOG_DEBUG("Invalid starting characters. Logical_port_id - %s",
                  vlan_map_key->logical_port_id);
    return UPLL_RC_ERR_CFG_SYNTAX;
  }
  if (!ValidateNumericRange(vlan_map_key->logical_port_id_valid, PFC_FALSE,
                            PFC_TRUE, true, true)) {
    UPLL_LOG_DEBUG("syntax check failed. swid_valid - %d",
                  vlan_map_key->logical_port_id_valid);
      return UPLL_RC_ERR_CFG_SYNTAX;
    } 
  } else {
    // Poisoning key from request to avoid binding for SIBLING_COUNT
    // and SIBLING_BEGIN
    UPLL_LOG_TRACE("Operation is %d", operation);
    StringReset(vlan_map_key->logical_port_id);
    vlan_map_key->logical_port_id_valid = 2;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VlanMapMoMgr::ValidateAttribute(ConfigKeyVal *ikey, 
                                          DalDmlIntf *dmi,
                                          IpcReqRespHeader *req) {
  UPLL_FUNC_TRACE;
  upll_rc_t rs_code = UPLL_RC_SUCCESS;
  if (!ikey || (ikey->get_key_type() != UNC_KT_VBR_VLANMAP)
      || !(ikey->get_cfg_val())) return UPLL_RC_ERR_CFG_SYNTAX;

  rs_code = IsLogicalPortAndVlanIdInUse(ikey, dmi, req);
  if (rs_code != UPLL_RC_ERR_CFG_SEMANTIC) {
    return UPLL_RC_SUCCESS;
  }
  return rs_code;
}

upll_rc_t VlanMapMoMgr::ValidateCapability(IpcReqRespHeader *req,
                                           ConfigKeyVal *ikey,
                                           const char *ctrlr_name) {
  UPLL_FUNC_TRACE;
  upll_rc_t ret_val = UPLL_RC_ERR_GENERIC;
  if (!req || !ikey ) {
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

  bool result_code = false;
  uint32_t max_attrs = 0;
  uint32_t max_instance_count = 0;
  const uint8_t *attrs;
  switch (req->operation) {
    case UNC_OP_CREATE:
      result_code = GetCreateCapability(ctrlr_name, ikey->get_key_type(),
                                        &max_instance_count, &max_attrs,
                                        &attrs);
      UPLL_LOG_TRACE("The Current Instance is %d", cur_instance_count);
      UPLL_LOG_TRACE("The Max Instance is %d", max_instance_count);

      if (result_code && (max_instance_count != 0) &&
         (cur_instance_count >= max_instance_count)) {
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
      UPLL_LOG_DEBUG("INVALID OPERATION CODE - (%d)", req->operation);
      return UPLL_RC_ERR_GENERIC;
  }
  if (!result_code) {
    UPLL_LOG_DEBUG("key_type - %d is not supported by controller - %s",
                  ikey->get_key_type(), ctrlr_name);
    return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
  }
  val_vlan_map *vlanmap_val = NULL;
  if ((ikey->get_cfg_val()) && (ikey->get_cfg_val()->get_st_num() ==
       IpctSt::kIpcStValVlanMap)) {
    vlanmap_val = reinterpret_cast<val_vlan_map *>
                              (ikey->get_cfg_val()->get_val());
  }
  if (vlanmap_val) {
    if (max_attrs > 0) {
      ret_val = ValVlanmapAttributeSupportCheck(vlanmap_val, attrs, req->operation);
      return ret_val;
    } else {
      UPLL_LOG_DEBUG("Attribute list is empty for operation %d", req->operation);
      return UPLL_RC_ERR_GENERIC;
    }
  }
  return UPLL_RC_SUCCESS;
}
 
upll_rc_t VlanMapMoMgr::ValVlanmapAttributeSupportCheck(val_vlan_map *vlanmap_val,
                                                        const uint8_t *attrs,
                                                        unc_keytype_operation_t operation) {
  UPLL_FUNC_TRACE;
  if (vlanmap_val != NULL) {
    if ((vlanmap_val->valid[UPLL_IDX_VLAN_ID_VM] == UNC_VF_VALID)
        || (vlanmap_val->valid[UPLL_IDX_VLAN_ID_VM] == UNC_VF_VALID_NO_VALUE)) {
      if (attrs[unc::capa::vlan_map::kCapVlanId] == 0) {
        vlanmap_val->valid[UPLL_IDX_VLAN_ID_VM] = UNC_VF_INVALID;
        if (operation == UNC_OP_CREATE || operation == UNC_OP_UPDATE) {
          UPLL_LOG_DEBUG("vlan_id attr is not supported by pfc ctrlr ");
          return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
        }
      }
    }
  } else {
    UPLL_LOG_DEBUG("Error val_vlan_map struct is NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  return UPLL_RC_SUCCESS;
}

bool VlanMapMoMgr::FilterAttributes(void *&val1,
                                    void *val2,
                                    bool copy_to_running,
                                    unc_keytype_operation_t op) {
  UPLL_FUNC_TRACE;
  if (op != UNC_OP_CREATE)
    return CompareValidValue(val1, val2, copy_to_running);
  return false;
}

bool VlanMapMoMgr::CompareValidValue(void *&val1,
                                     void *val2,
                                     bool copy_to_running) {
  UPLL_FUNC_TRACE;
  bool invalid_attr = true;
  val_vlan_map_t *val_vlan_map1 = reinterpret_cast<val_vlan_map_t *>(val1);
  val_vlan_map_t *val_vlan_map2 = reinterpret_cast<val_vlan_map_t *>(val2);
  for (uint8_t loop = 0;
      loop < sizeof(val_vlan_map1->valid) / sizeof(uint8_t); ++loop) {
    if (UNC_VF_INVALID == val_vlan_map1->valid[loop]
        && UNC_VF_VALID == val_vlan_map2->valid[loop])
      val_vlan_map1->valid[loop] = UNC_VF_VALID_NO_VALUE;
  }
  if (val_vlan_map1->valid[UPLL_IDX_VLAN_ID_VM] == UNC_VF_VALID
      && val_vlan_map2->valid[UPLL_IDX_VLAN_ID_VM] == UNC_VF_VALID) {
    if (val_vlan_map1->vlan_id == val_vlan_map2->vlan_id)
      val_vlan_map1->valid[UPLL_IDX_VLAN_ID_VM] =
        (copy_to_running)?UNC_VF_INVALID:UNC_VF_VALUE_NOT_MODIFIED; 
  }
  for (unsigned int loop = 0;
       loop < sizeof(val_vlan_map1->valid) / sizeof(val_vlan_map1->valid[0]);
       ++loop) {
    if ((UNC_VF_VALID == (uint8_t) val_vlan_map1->valid[loop]) ||
       (UNC_VF_VALID_NO_VALUE == (uint8_t) val_vlan_map1->valid[loop]))
        invalid_attr = false;
  }
  return invalid_attr;
}

upll_rc_t VlanMapMoMgr::IsReferenced(ConfigKeyVal *ikey,
                                     upll_keytype_datatype_t dt_type,
                                     DalDmlIntf *dmi) {
  return UPLL_RC_SUCCESS;
}

upll_rc_t VlanMapMoMgr::IsLogicalPortAndVlanIdInUse(ConfigKeyVal *ckv,
    DalDmlIntf *dmi,
    IpcReqRespHeader *req) {
  UPLL_FUNC_TRACE;
  if (!ckv || !(ckv->get_cfg_val()) || !(ckv->get_key()))
    return UPLL_RC_ERR_GENERIC;

  upll_rc_t result_code = UPLL_RC_SUCCESS;

  val_vlan_map_t* vlanmapval = reinterpret_cast<val_vlan_map_t*>(GetVal(ckv));
  key_vlan_map_t* vlanmapkey =
    reinterpret_cast<key_vlan_map_t *>(ckv->get_key());

  if (vlanmapval->valid[UPLL_IDX_VLAN_ID_VM] == UNC_VF_VALID) {

    key_vlan_map_t* key_vlan_map = reinterpret_cast<key_vlan_map_t *>
      (ConfigKeyVal::Malloc(sizeof(key_vlan_map_t)));
    val_vlan_map_t* val_vlan_map = static_cast<val_vlan_map_t*>
      (ConfigKeyVal::Malloc(sizeof(val_vlan_map_t)));
    if (vlanmapkey->logical_port_id_valid) {
      uuu::upll_strncpy(key_vlan_map->logical_port_id,
          vlanmapkey->logical_port_id,
          kMaxLenLogicalPortId+1);
      key_vlan_map->logical_port_id_valid = vlanmapkey->logical_port_id_valid;
    }

    //Populating key and val of vlanmap
    val_vlan_map->vlan_id = vlanmapval->vlan_id;
    val_vlan_map->valid[UPLL_IDX_VLAN_ID_VM] = UNC_VF_VALID;

    ConfigKeyVal *ckv_vlanmap = new ConfigKeyVal(UNC_KT_VBR_VLANMAP,
        IpctSt::kIpcStKeyVlanMap, key_vlan_map, NULL);
    ckv_vlanmap->AppendCfgVal(IpctSt::kIpcStValVlanMap, val_vlan_map);
    //Setting Ctrlr/Domain Id to vlanmap
    SET_USER_DATA(ckv_vlanmap, ckv);

    DbSubOp dbop = { kOpReadMultiple, kOpMatchCtrlr | kOpMatchDomain,
      kOpInOutFlag};
    //Read the Configuration from the MainTable
    result_code = ReadConfigDB(ckv_vlanmap, UPLL_DT_CANDIDATE, UNC_OP_READ,
        dbop, dmi, MAINTBL);
    //Check LogicalPortId and VlanId in use
    if (result_code == UPLL_RC_SUCCESS) {
      ConfigKeyVal *tmp = ckv_vlanmap;
      key_vlan_map_t* tmp_vlanmapkey =
        reinterpret_cast<key_vlan_map_t *>(tmp->get_key());
      while (tmp) {
        if (!memcmp((const void*)&(vlanmapkey->vbr_key),
              (const void*) &(tmp_vlanmapkey->vbr_key),
              sizeof(key_vbr_t))) {
          UPLL_LOG_TRACE("Looking on the Same key");
        } else {
          UPLL_LOG_DEBUG("More than one vlanmap is configured with the"
              " same logical port id and vlanid!");
          DELETE_IF_NOT_NULL(ckv_vlanmap);
          tmp = NULL;
          return UPLL_RC_ERR_CFG_SEMANTIC;
        }
        tmp = tmp->get_next_cfg_key_val();
      }
    } else if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      result_code = UPLL_RC_SUCCESS;
    }
    DELETE_IF_NOT_NULL(ckv_vlanmap);
  }
  return result_code;
}

// Overridden Read Sibling from momgr_impl.
// This keytype contains 2 child keys and needs special handling.
upll_rc_t VlanMapMoMgr::ReadSiblingMo(IpcReqRespHeader *header,
                                   ConfigKeyVal *ikey,
                                   bool begin,
                                   DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_ERR_GENERIC;
  uint32_t result_count = 0;

  if (begin == true) {
    return (MoMgrImpl::ReadSiblingMo(header, ikey, begin, dmi));
    
  }
  // Fix it: This function is for READ_SIBLING
  if ((header->operation != UNC_OP_READ_SIBLING_BEGIN) &&
      (header->operation != UNC_OP_READ_SIBLING)) { 
    UPLL_LOG_DEBUG("Operation type is not Sibling begin/Sibling");
    return UPLL_RC_ERR_GENERIC;
  }

  if (ikey == NULL || dmi == NULL) {
    UPLL_LOG_INFO("Null param ikey/dmi");
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

      ConfigKeyVal *result_ckv = NULL;
      // Used in ReadConfigDB in each Iteration
      ConfigKeyVal *queryckval = NULL;
      result_code = DupConfigKeyVal(queryckval, ikey, MAINTBL);
      if (result_code != UPLL_RC_SUCCESS || queryckval == NULL) {
        UPLL_LOG_INFO("Error while DupConfigKeyVal");
        return UPLL_RC_ERR_GENERIC;
      }

      for (uint16_t childKeyIndex = uuds::TableNumPkCols(
               uudst::kDbiVbrVlanMapTbl) - 1;
           childKeyIndex >= uuds::TableNumPkCols(
               uudst::kDbiVbrVlanMapTbl) - kVbrVlanMapNumChildKey;
           childKeyIndex--) {

        // For skipping invalid inputs - do not delete
        if (IsValidKey(queryckval->get_key(), childKeyIndex) == false)
          continue;

        uint32_t tmp_count = (header->rep_count - result_count);
        result_code = ReadConfigDB(queryckval, dt_type, header->operation,
                                   dbop, tmp_count, dmi, tbl);

        if (result_code == UPLL_RC_SUCCESS) {
          // Collect the data into result_ckv.
          if (result_ckv == NULL) {
            result_ckv = queryckval;
          } else {
            result_ckv->AppendCfgKeyVal(queryckval);  // TODO perf
          }
          result_count += queryckval->size();
          queryckval = NULL;
          
          if (result_count >= header->rep_count) {
            break;  // break the for loop
          }

          // Used in ReadConfigDB in each Iteration
          result_code = DupConfigKeyVal(queryckval, ikey, MAINTBL);
          if (result_code != UPLL_RC_SUCCESS || queryckval == NULL) {
            UPLL_LOG_INFO("Error while DupConfigKeyVal");
            return UPLL_RC_ERR_GENERIC;
          }
        } else if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
          // use queryckval in the next iteration;
        } else {
          UPLL_LOG_INFO("Error while ReadConfigDB");
          delete queryckval;
          return result_code;
        }

        // Preparing Child Key data for next Sibling Iteration
        if (ResetDataForSibling(
             reinterpret_cast<key_vlan_map *>(queryckval->get_key()),
             (uudst::vbridge_vlanmap::kVbrVlanMapIndex)childKeyIndex)
           == false) {
          UPLL_LOG_DEBUG("Data Not Reset for the index(%d)", childKeyIndex);
          delete queryckval;
          return UPLL_RC_ERR_GENERIC;
        }
        UPLL_LOG_TRACE("Next Query After Reset: %s",
                       (queryckval->ToStrAll()).c_str());
      }  // for
      DELETE_IF_NOT_NULL(queryckval);
      header->rep_count = result_count;
      if (result_ckv) {
        ikey->ResetWith(result_ckv);
        delete result_ckv;
      }
      break;
    }  // case
    default:
      return UPLL_RC_ERR_GENERIC;
  }

  if (header->rep_count > 0) {
    result_code = (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE)
                  ? UPLL_RC_SUCCESS : result_code;
    if (result_code == UPLL_RC_SUCCESS) { 
      result_code = AdaptValToVtnService(ikey);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("AdaptValToVtnService failed result_code %d",
                    result_code);
        return UPLL_RC_ERR_GENERIC;
      }
    }
  } else {
    return UPLL_RC_ERR_NO_SUCH_INSTANCE;
  }
  return UPLL_RC_SUCCESS;
}  // VlanMapMoMgr::ReadSiblingMo

// To reset the data available in index to empty
// Currently implemented only for child index
bool VlanMapMoMgr::ResetDataForSibling(key_vlan_map *key_vmap,
    uudst::vbridge_vlanmap::kVbrVlanMapIndex index) {
  switch(index) {
    case uudst::vbridge_vlanmap::kDbiLogicalPortId:
      memset(key_vmap->logical_port_id, 0, sizeof(key_vmap->logical_port_id));
    case uudst::vbridge_vlanmap::kDbiLogicalPortIdValid:
      key_vmap->logical_port_id_valid = INVALID_LOG_PORT_ID_VALID;
      break;
    default:
      UPLL_LOG_DEBUG("Not a child key index");
      return false;
  }
  UPLL_LOG_TRACE("Resetting Data for index(%d)", index);
  return true;
}

}  // namespace kt_momgr
}  // namespace upll
}  // namespace unc
