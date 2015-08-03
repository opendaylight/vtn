/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <ctype.h>
#include "vlanmap_momgr.hh"
#include "config_mgr.hh"
#include "vlink_momgr.hh"
#include "vbr_if_flowfilter_momgr.hh"
#include "vbr_if_policingmap_momgr.hh"
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
    /* VlanmapOnBoundary: New value structure */
    { uudst::vbridge_vlanmap::kDbiVlanid, CFG_VAL, offsetof(pfcdrv_val_vlan_map,
                                                            vm.vlan_id),
      uud::kDalUint16, 1 },
    { uudst::vbridge_vlanmap::kDbiBdryRefCount, CFG_VAL, offsetof(
      pfcdrv_val_vlan_map, bdry_ref_count),
      uud::kDalUint32, 1 },
    { uudst::vbridge_vlanmap::kDbiCtrlrName, CK_VAL, offsetof(key_user_data_t,
                                                              ctrlr_id),
      uud::kDalChar, 32 },
    { uudst::vbridge_vlanmap::kDbiDomainId, CK_VAL, offsetof(key_user_data_t,
                                                             domain_id),
      uud::kDalChar, 32 },

    { uudst::vbridge_vlanmap::kDbiVbrVlanMapFlags, CK_VAL, offsetof(
        key_user_data_t, flags),
      uud::kDalUint8, 1 },
    /* VlanmapOnBoundary: New value structure */
    { uudst::vbridge_vlanmap::kDbiValidVlanid, CFG_META_VAL, offsetof(
        pfcdrv_val_vlan_map, vm.valid[0]),
    uud::kDalUint8, 1 },
    { uudst::vbridge_vlanmap::kDbiValidBdryRefCount, CFG_META_VAL, offsetof(
        pfcdrv_val_vlan_map, valid[1]),
    uud::kDalUint8, 1 },

    { uudst::vbridge_vlanmap::kDbiCsRowStatus, CS_VAL,
      offsetof(pfcdrv_val_vlan_map, vm.cs_row_status),
    uud::kDalUint8, 1 },
    { uudst::vbridge_vlanmap::kDbiCsVlanid, CS_VAL,
      offsetof(pfcdrv_val_vlan_map, vm.cs_attr[0]),
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
  table = new Table *[ntable]();
  table[MAINTBL] = new Table(uudst::kDbiVbrVlanMapTbl, UNC_KT_VBR_VLANMAP,
                   vlan_map_bind_info, IpctSt::kIpcStKeyVlanMap,
                   IpctSt::kIpcStValVlanMap,
                   uudst::vbridge_vlanmap::kDbiVbrVlanMapNumCols);
  table[RENAMETBL] = NULL;
  table[CTRLRTBL] = NULL;
  table[CONVERTTBL] = NULL;
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

bool VlanMapMoMgr::IsValidKey(void *key, uint64_t index,
                              MoMgrTables tbl) {
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
  bool    cfgval_ctrlr = false;
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
    case UNC_KT_VBR_IF:
      uuu::upll_strncpy(vlanmap_key->vbr_key.vtn_key.vtn_name,
             (reinterpret_cast<key_vbr_if *>(pkey))->vbr_key.vtn_key.vtn_name,
             kMaxLenVtnName+1);
      uuu::upll_strncpy(vlanmap_key->vbr_key.vbridge_name,
             (reinterpret_cast<key_vbr_if *>(pkey))->vbr_key.vbridge_name,
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
      break;
    /* VlanmapOnBoundary: To handle vlink kt */
    case UNC_KT_VLINK: {
      uint8_t *vnode_name;
      uint8_t flags = 0;
      val_vlink *vlink_val = reinterpret_cast<val_vlink *>(GetVal(parent_key));
      if (!vlink_val) {
        UPLL_LOG_TRACE("ERROR");
        if (!okey || !(okey->get_key()))
          FREE_IF_NOT_NULL(vlanmap_key);
        return UPLL_RC_ERR_GENERIC;
      }

      GET_USER_DATA_FLAGS(parent_key->get_cfg_val(), flags);
      flags &=  VLINK_FLAG_NODE_POS;
      UPLL_LOG_DEBUG("Vlink flag node position %d", flags);

      if (flags == kVlinkVnode2) {
        cfgval_ctrlr = true;
        vnode_name = vlink_val->vnode2_name;
      } else {
        vnode_name = vlink_val->vnode1_name;
      }
      uuu::upll_strncpy(vlanmap_key->vbr_key.vtn_key.vtn_name,
                        reinterpret_cast<key_vlink *>(pkey)->vtn_key.vtn_name,
                        (kMaxLenVtnName + 1));
      if (vnode_name)
        uuu::upll_strncpy(vlanmap_key->vbr_key.vbridge_name, vnode_name,
                          (kMaxLenVnodeName + 1));

      break;
    }
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
    if (cfgval_ctrlr) {
      SET_USER_DATA(okey, parent_key->get_cfg_val());
    } else {
      SET_USER_DATA(okey, parent_key);
    }
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
      /* VlanmapOnBoundary: Val structre is modified */
      val = reinterpret_cast<void *>(
          ConfigKeyVal::Malloc(sizeof(pfcdrv_val_vlan_map)));
      ck_val = new ConfigVal(IpctSt::kIpcStPfcdrvValVlanMap, val);
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
    /* VlanmapOnBoundary: Modified for new val struct */
    void *oval;
    if (tbl == MAINTBL) {
      if ((req->get_cfg_val())->get_st_num() == IpctSt::kIpcStValVlanMap) {
        val_vlan_map *ival = reinterpret_cast<val_vlan_map *>(GetVal(req));
        if (!ival) return UPLL_RC_ERR_GENERIC;

        val_vlan_map *vlanmap_val = reinterpret_cast<val_vlan_map *>(
            ConfigKeyVal::Malloc(sizeof(val_vlan_map)));
        memcpy(vlanmap_val, ival, sizeof(val_vlan_map));
        oval = reinterpret_cast<void *>(vlanmap_val);
      } else {
        pfcdrv_val_vlan_map *ival =
            reinterpret_cast<pfcdrv_val_vlan_map *>(GetVal(req));
        if (!ival) {
          UPLL_LOG_DEBUG("Invalid vlanmap");
          return UPLL_RC_ERR_GENERIC;
        }
        pfcdrv_val_vlan_map *vlanmap_val =
            reinterpret_cast<pfcdrv_val_vlan_map *>
           (ConfigKeyVal::Malloc(sizeof(pfcdrv_val_vlan_map)));
        memcpy(vlanmap_val, ival, sizeof(pfcdrv_val_vlan_map));
        oval = reinterpret_cast<void *>(vlanmap_val);
      }
      tmp1 = new ConfigVal(req->get_cfg_val()->get_st_num(), oval);
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
    free(vlanmap_key);
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
  pfcdrv_val_vlan_map_t *drv_vlanmap_val =
      reinterpret_cast<pfcdrv_val_vlan_map_t*>(GetVal(ikey));
  if (drv_vlanmap_val == NULL) return UPLL_RC_ERR_GENERIC;
  val_vlan_map_t *vlanmap_val = &(drv_vlanmap_val->vm);

  unc_keytype_configstatus_t cs_status =
      (driver_result == UPLL_RC_SUCCESS) ? UNC_CS_APPLIED : UNC_CS_NOT_APPLIED;
  pfcdrv_val_vlan_map *drv_vlanmap_val2 =
      reinterpret_cast<pfcdrv_val_vlan_map *>(GetVal(upd_key));
  val_vlan_map_t      *vlanmap_val2     = &(drv_vlanmap_val2->vm);
  UPLL_LOG_TRACE("Key from Candidate %s", (ikey->ToStrAll()).c_str());

  if (op == UNC_OP_CREATE) {
    vlanmap_val->cs_row_status = cs_status;
  } else if (op == UNC_OP_UPDATE) {
    void *val = reinterpret_cast<void *>(drv_vlanmap_val);
    CompareValidValue(val, GetVal(upd_key), true);
    UPLL_LOG_TRACE("Key from Running %s", (upd_key->ToStrAll()).c_str());
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
  upll_rc_t result_code = UpdateVnodeOperStatus(ikey, dmi, driver_result);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_TRACE("Update parent vbridge failed\n");
  }
  return UPLL_RC_SUCCESS;
}


upll_rc_t VlanMapMoMgr::UpdateAuditConfigStatus(
    unc_keytype_configstatus_t cs_status,
    uuc::UpdateCtrlrPhase phase,
    ConfigKeyVal *&ckv_running,
    DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  pfcdrv_val_vlan_map_t *drv_val = NULL;
  val_vlan_map_t        *val     = NULL;
  drv_val = (ckv_running != NULL) ? reinterpret_cast<pfcdrv_val_vlan_map_t *>
                                (GetVal(ckv_running)) : NULL;
  if (NULL == drv_val) {
    return UPLL_RC_ERR_GENERIC;
  }
  val = &(drv_val->vm);
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

  val_vlan_map *vlanmap_val = NULL;
  ConfigVal *cfg_val = ikey->get_cfg_val();
  if (ikey->get_cfg_val()) {
    if (cfg_val->get_st_num() == IpctSt::kIpcStValVlanMap) {
      vlanmap_val =
          reinterpret_cast<val_vlan_map *>(cfg_val->get_val());
    } else if (cfg_val->get_st_num() == IpctSt::kIpcStPfcdrvValVlanMap) {
      vlanmap_val =
          &(reinterpret_cast<pfcdrv_val_vlan_map *>(cfg_val->get_val())->vm);
    } else if ((cfg_val->get_st_num() != IpctSt::kIpcStValVlanMap) ||
               (cfg_val->get_st_num() != IpctSt::kIpcStPfcdrvValVlanMap)) {
      UPLL_LOG_DEBUG("Invalid val structure received.received struct - %d",
                     ikey->get_cfg_val()->get_st_num());
      return UPLL_RC_ERR_BAD_REQUEST;
    }
  }
  if (op == UNC_OP_CREATE || op == UNC_OP_UPDATE) {
    if (dt_type == UPLL_DT_CANDIDATE || UPLL_DT_IMPORT == dt_type) {
      ConfigVal *cfg_val = ikey->get_cfg_val();
      if (!cfg_val)  {
        UPLL_LOG_DEBUG("Value structure mandatory");
        return UPLL_RC_ERR_BAD_REQUEST;
      }
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
    if (dt_type == UPLL_DT_CANDIDATE) {
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
    return UPLL_RC_ERR_CFG_SYNTAX;
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
    } else {
      ret_val = UPLL_RC_SUCCESS;
    }
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
  if (!req || !ikey) {
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

  if (req->datatype == UPLL_DT_IMPORT) {
    if ((ikey->get_cfg_val()) && (ikey->get_cfg_val()->get_st_num() ==
                                  IpctSt::kIpcStPfcdrvValVlanMap)) {
      vlanmap_val = &(reinterpret_cast<pfcdrv_val_vlan_map *>
                      (ikey->get_cfg_val()->get_val())->vm);
    }
  } else {
    if ((ikey->get_cfg_val()) && (ikey->get_cfg_val()->get_st_num() ==
                                  IpctSt::kIpcStValVlanMap)) {
      vlanmap_val = reinterpret_cast<val_vlan_map *>
          (ikey->get_cfg_val()->get_val());
    }
    if ((ikey->get_cfg_val()) && (ikey->get_cfg_val()->get_st_num() ==
                                  IpctSt::kIpcStPfcdrvValVlanMap)) {
      vlanmap_val = &(reinterpret_cast<pfcdrv_val_vlan_map *>
                      (ikey->get_cfg_val()->get_val())->vm);
    }
  }

  if (vlanmap_val) {
    if (max_attrs > 0) {
      ret_val = ValVlanmapAttributeSupportCheck(vlanmap_val,
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

upll_rc_t VlanMapMoMgr::ValVlanmapAttributeSupportCheck(
    val_vlan_map *vlanmap_val,
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

  pfcdrv_val_vlan_map_t *drv_val_vlan_map1 =
      reinterpret_cast<pfcdrv_val_vlan_map_t *>(val1);
  pfcdrv_val_vlan_map_t *drv_val_vlan_map2 =
      reinterpret_cast<pfcdrv_val_vlan_map_t *>(val2);
  val_vlan_map_t *val_vlan_map1 = &(drv_val_vlan_map1->vm);
  val_vlan_map_t *val_vlan_map2 = &(drv_val_vlan_map2->vm);

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

upll_rc_t VlanMapMoMgr::IsReferenced(IpcReqRespHeader *req,
                                     ConfigKeyVal *ikey,
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
  val_vlan_map_t *vlanmapval = NULL;

  if (req->datatype != UPLL_DT_IMPORT) {
    pfcdrv_val_vlan_map_t* drv_vlanmapval =
        reinterpret_cast<pfcdrv_val_vlan_map_t*>(GetVal(ckv));
    vlanmapval = &(drv_vlanmapval->vm);
  } else {
    vlanmapval = reinterpret_cast<val_vlan_map_t*>(GetVal(ckv));
  }

  if (!vlanmapval) {
    UPLL_LOG_DEBUG("Returning error\n");
    return UPLL_RC_ERR_GENERIC;
  }

  key_vlan_map_t* vlanmapkey =
    reinterpret_cast<key_vlan_map_t *>(ckv->get_key());

  if (vlanmapval->valid[UPLL_IDX_VLAN_ID_VM] == UNC_VF_VALID) {
    key_vlan_map_t* key_vlan_map = reinterpret_cast<key_vlan_map_t *>
      (ConfigKeyVal::Malloc(sizeof(key_vlan_map_t)));
    pfcdrv_val_vlan_map_t* drv_val_vlan_map =
        static_cast<pfcdrv_val_vlan_map_t*>
        (ConfigKeyVal::Malloc(sizeof(pfcdrv_val_vlan_map_t)));
    val_vlan_map_t* val_vlan_map = &(drv_val_vlan_map->vm);

    if (vlanmapkey->logical_port_id_valid) {
      uuu::upll_strncpy(key_vlan_map->logical_port_id,
          vlanmapkey->logical_port_id,
          kMaxLenLogicalPortId+1);
      key_vlan_map->logical_port_id_valid = vlanmapkey->logical_port_id_valid;
    }

    //  Populating key and val of vlanmap
    val_vlan_map->vlan_id = vlanmapval->vlan_id;
    val_vlan_map->valid[UPLL_IDX_VLAN_ID_VM] = UNC_VF_VALID;

    ConfigKeyVal *ckv_vlanmap = new ConfigKeyVal(UNC_KT_VBR_VLANMAP,
        IpctSt::kIpcStKeyVlanMap, key_vlan_map, NULL);
    ckv_vlanmap->AppendCfgVal(IpctSt::kIpcStPfcdrvValVlanMap, drv_val_vlan_map);

    //  Setting Ctrlr/Domain Id to vlanmap
    SET_USER_DATA(ckv_vlanmap, ckv);

    DbSubOp dbop = { kOpReadMultiple, kOpMatchCtrlr,
      kOpInOutFlag};
    //  Read the Configuration from the MainTable
    result_code = ReadConfigDB(ckv_vlanmap, UPLL_DT_CANDIDATE, UNC_OP_READ,
        dbop, dmi, MAINTBL);
    //  Check LogicalPortId and VlanId in use
    if (result_code == UPLL_RC_SUCCESS) {
      ConfigKeyVal *tmp = ckv_vlanmap;
      key_vlan_map_t* tmp_vlanmapkey =
        reinterpret_cast<key_vlan_map_t *>(tmp->get_key());
      while (tmp) {
        // Ignore the record if given input key and database key are same.
        if (IsEqual(
             *(reinterpret_cast<key_vnode_t*>(&vlanmapkey->vbr_key)),
             *(reinterpret_cast<key_vnode_t*>(&tmp_vlanmapkey->vbr_key)))) {
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

upll_rc_t VlanMapMoMgr::ReadSiblingBeginMo(IpcReqRespHeader *header,
                                           ConfigKeyVal *ikey,
                                           bool begin,
                                           DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  controller_domain ctrlr_dom;

  switch (header->datatype) {
    case UPLL_DT_IMPORT:
      return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT;
    case UPLL_DT_CANDIDATE:
    case UPLL_DT_RUNNING:
    case UPLL_DT_STARTUP:
    case UPLL_DT_STATE:
        // Change the dt_type to Running and finally back to State
        header->datatype = (header->datatype == UPLL_DT_STATE) ?
          UPLL_DT_RUNNING : header->datatype;
      break;
    default:
      UPLL_LOG_DEBUG("Invalid Datatype");
      return UPLL_RC_ERR_GENERIC;
  }

  result_code = ValidateMessage(header, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("ValidateMessage failed result_code %d",
                   result_code);
    return result_code;
  }
  ConfigKeyVal *result_ckv = NULL;
  ConfigKeyVal *queryckval = NULL;
  result_code = DupConfigKeyVal(queryckval, ikey, MAINTBL);
  if (result_code != UPLL_RC_SUCCESS || queryckval == NULL) {
    DELETE_IF_NOT_NULL(queryckval);
    UPLL_LOG_INFO("Error while DupConfigKeyVal");
    return UPLL_RC_ERR_GENERIC;
  }

  /* when user specified vlanid in the val struct, set to true */
  bool filter_vlanid = false;
  /* Covert user val struct to pfc_drv val struct */
  val_vlan_map *vl = reinterpret_cast<val_vlan_map*>(GetVal(ikey));
  if (vl != NULL) {
    pfcdrv_val_vlan_map *val = reinterpret_cast<pfcdrv_val_vlan_map_t *>
      (ConfigKeyVal::Malloc(sizeof(pfcdrv_val_vlan_map_t)));

    memcpy(&val->vm, vl, sizeof(val_vlan_map_t));
    val->valid[0] = UNC_VF_VALID;
    val->valid[1] = UNC_VF_INVALID;
    (queryckval->get_cfg_val())->SetVal(IpctSt::kIpcStPfcdrvValVlanMap, val);
    filter_vlanid = true;
  }

  uint32_t found_count = 0, skip_count = 0, tmp_count = 0, result_count =0;
  upll_keytype_datatype_t dt_type =  header->datatype;

  do {
    ConfigKeyVal *new_ckv = NULL;
    result_code = ReadInfoFromDB(header, queryckval, dmi, &ctrlr_dom);
    if (result_code == UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("ReadInfoFromDB returns SUCCESS: %s \n",
                     queryckval->ToStrAll().c_str());
      if (header->rep_count > 0) {
        tmp_count = header->rep_count;
        skip_count = 0;
        new_ckv = NULL;
        ConfigKeyVal *tmp = queryckval, *tmp_old = NULL;
        while (tmp) {
          uint8_t vlanmap_flag = 0;
          GET_USER_DATA_FLAGS(tmp, vlanmap_flag);
          uint8_t is_user_conf = false;
          is_user_conf = (USER_VLANMAP_FLAG & vlanmap_flag);
          if (!is_user_conf) {
            ConfigKeyVal *ck_tmp =tmp;
            skip_count++;
            tmp = tmp->get_next_cfg_key_val();
            if (tmp == NULL) {
              DELETE_IF_NOT_NULL(new_ckv);
              if (filter_vlanid == true) {
                result_code = DupConfigKeyVal(new_ckv, ck_tmp, MAINTBL);
              } else {
                result_code = GetChildConfigKey(new_ckv, ck_tmp);
              }
              if (result_code != UPLL_RC_SUCCESS || new_ckv == NULL) {
                UPLL_LOG_ERROR("Error while GetChildConfigKey or "
                  "DupConfigKeyVal. filter_vlanid: %d", filter_vlanid);
                DELETE_IF_NOT_NULL(queryckval);
                return UPLL_RC_ERR_GENERIC;
              }
            }
            if (ck_tmp == queryckval) {
              queryckval = ck_tmp->get_next_cfg_key_val();
            } else {
              tmp_old->set_next_cfg_key_val(ck_tmp->get_next_cfg_key_val());
            }
            ck_tmp->set_next_cfg_key_val(NULL);
            DELETE_IF_NOT_NULL(ck_tmp);
            continue;

          } else {
            tmp_old =tmp;
            tmp = tmp->get_next_cfg_key_val();
            if (tmp == NULL) {
              DELETE_IF_NOT_NULL(new_ckv);
              if (filter_vlanid == true) {
                result_code = DupConfigKeyVal(new_ckv, tmp_old, MAINTBL);
              } else {
                result_code = GetChildConfigKey(new_ckv, tmp_old);
              }
              if (result_code != UPLL_RC_SUCCESS || new_ckv == NULL) {
                UPLL_LOG_ERROR("Error while GetChildConfigKey or "
                  "DupConfigKeyVal. filter_vlanid: %d", filter_vlanid);
                DELETE_IF_NOT_NULL(queryckval);
                return UPLL_RC_ERR_GENERIC;
              }
            }
          }
        }
        found_count = (tmp_count-skip_count);
        if (found_count > 0) {
          // Collect the data into result_ckv.
          if (result_ckv == NULL) {
            result_ckv = queryckval;
          } else {
            result_ckv->AppendCfgKeyVal(queryckval);
          }
          result_count += (queryckval->size());
        }
        if (found_count < tmp_count) {
          queryckval = new_ckv;
          if (ResetDataForSibling(
                  reinterpret_cast<key_vlan_map *>(queryckval->get_key()),
                  (uudst::vbridge_vlanmap::kVbrVlanMapIndex)3)
              == false) {}
          header->operation = UNC_OP_READ_SIBLING;
          header->rep_count = tmp_count - result_count;

          // Preparing Child Key data for next Sibling Iteration
          UPLL_LOG_TRACE("Next Query After Reset: %s",
                         (queryckval->ToStrAll()).c_str());
          continue;
        } else {
          DELETE_IF_NOT_NULL(new_ckv);
          break;
        }
        // Used in ReadConfigDB in each Iteration
      }
    } else if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      DELETE_IF_NOT_NULL(queryckval);
      break;
    } else {
      UPLL_LOG_DEBUG("Error in Reading DB");
      DELETE_IF_NOT_NULL(queryckval);
      return result_code;
    }
  } while (1);
  if (result_ckv != NULL) {
    ikey->ResetWith(result_ckv);
    DELETE_IF_NOT_NULL(result_ckv);
    header->rep_count = ikey->size();
  }
  header->operation = UNC_OP_READ_SIBLING_BEGIN;
  header->datatype = dt_type;

  /* Convert the valstruct to VTN service val*/
  upll_rc_t rc = AdaptValToVtnService(ikey, ADAPT_ALL);
  if (rc != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("AdaptValToVtnService failed result_code %d", rc);
    return UPLL_RC_ERR_GENERIC;
  }

  if (header->rep_count > 0) {
    result_code = (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE)
        ? UPLL_RC_SUCCESS : result_code;
  } else {
    return UPLL_RC_ERR_NO_SUCH_INSTANCE;
  }
  return result_code;
}

upll_rc_t VlanMapMoMgr::ReadSiblingCount(IpcReqRespHeader *header,
                                         ConfigKeyVal *ikey,
                                         DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  controller_domain ctrlr_dom;

  result_code = ValidateMessage(header, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("ValidateMessage failed result_code %d",
                   result_code);
    return result_code;
  }
  ConfigKeyVal *queryckval = NULL;
  result_code = DupConfigKeyVal(queryckval, ikey, MAINTBL);
  if (result_code != UPLL_RC_SUCCESS || queryckval == NULL) {
    UPLL_LOG_INFO("Error while DupConfigKeyVal");
     DELETE_IF_NOT_NULL(queryckval);
    return UPLL_RC_ERR_GENERIC;
  }
  /* Convert user val strcut to pfc_drv val struct */
  val_vlan_map *vl = reinterpret_cast<val_vlan_map*>(GetVal(ikey));
  if (vl != NULL) {
    pfcdrv_val_vlan_map *val = reinterpret_cast<pfcdrv_val_vlan_map_t *>
      (ConfigKeyVal::Malloc(sizeof(pfcdrv_val_vlan_map_t)));

    memcpy(&val->vm, vl, sizeof(val_vlan_map_t));
    val->valid[0] = UNC_VF_VALID;
    val->valid[1] = UNC_VF_INVALID;
    (ikey->get_cfg_val())->SetVal(IpctSt::kIpcStPfcdrvValVlanMap, val);
  }
  header->operation = UNC_OP_READ_SIBLING_BEGIN;
  // Ignore max_repetition in read sibling count
  header->rep_count = 0;

  uint32_t found_count = 0;
  result_code = ReadInfoFromDB(header, ikey, dmi, &ctrlr_dom);
  if (result_code == UPLL_RC_SUCCESS) {
    if (header->rep_count > 0) {
      ConfigKeyVal *tmp = ikey;
      while (tmp) {
        uint8_t vlanmap_flag = 0;
        GET_USER_DATA_FLAGS(tmp, vlanmap_flag);
        uint8_t is_user_conf = false;
        is_user_conf = (USER_VLANMAP_FLAG & vlanmap_flag);
        if (is_user_conf) {
          found_count++;
        }
        tmp = tmp->get_next_cfg_key_val();
      }
    }
  } else if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    result_code = UPLL_RC_SUCCESS;
  }
  header->rep_count = found_count;
  header->operation = UNC_OP_READ_SIBLING_COUNT;
  ikey->ResetWith(queryckval);

  /* Read sib count should return val_struct with count value */
  uint32_t *sib_count = reinterpret_cast<uint32_t*>
    (ConfigKeyVal::Malloc(sizeof(uint32_t)));
  *sib_count = found_count;
  ikey->SetCfgVal(new ConfigVal(IpctSt::kIpcStUint32, sib_count));

  DELETE_IF_NOT_NULL(queryckval);
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
    /* VlanmapOnBoundary: Over-ride MoMgrImpl::ReadSiblingBeginMo in Vlanmap */
    return (ReadSiblingBeginMo(header, ikey, begin, dmi));
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
      /* ckv that contains final read entries, given to ikey */
      ConfigKeyVal *result_ckv = NULL;
      // Used in ReadConfigDB in each Iteration
      ConfigKeyVal *queryckval = NULL;
      result_code = DupConfigKeyVal(queryckval, ikey, MAINTBL);
      if (result_code != UPLL_RC_SUCCESS || queryckval == NULL) {
        UPLL_LOG_INFO("Error while DupConfigKeyVal");
        DELETE_IF_NOT_NULL(queryckval);
        return UPLL_RC_ERR_GENERIC;
      }

      /* Used from 2nd read operations if expected entries are not read */
      ConfigKeyVal *new_ckv = queryckval;
      /* when user specified vlanid in the val struct, set to true */
      bool filter_vlanid = false;

      /* Convert user val strcut to pfc_drv val struct */
      val_vlan_map *vl = reinterpret_cast<val_vlan_map*>(GetVal(ikey));
      if (vl != NULL) {
        pfcdrv_val_vlan_map *val = reinterpret_cast<pfcdrv_val_vlan_map_t *>
          (ConfigKeyVal::Malloc(sizeof(pfcdrv_val_vlan_map_t)));

        memcpy(&val->vm, vl, sizeof(val_vlan_map_t));
        val->valid[0] = UNC_VF_VALID;
        val->valid[1] = UNC_VF_INVALID;
        (queryckval->get_cfg_val())->SetVal(IpctSt::kIpcStPfcdrvValVlanMap,
                                            val);
        filter_vlanid = true;
      }
      /* skip_count: No of entries skipped in an iter (not user configured)
       * found_count: No of entries found in an iter (user configured) */
      uint8_t skip_count =0, found_count = 0;

        /* Expected entries to be read from db for every iteration */
        uint32_t expect_count = header->rep_count;
        /* tmp_childKeyIndex is used in ResetDataForSibling */
        uint16_t tmp_childKeyIndex = uuds::TableNumPkCols(
            uudst::kDbiVbrVlanMapTbl) - 1;
        for (uint16_t childKeyIndex = uuds::TableNumPkCols(
                uudst::kDbiVbrVlanMapTbl) - 1;
            childKeyIndex >= uuds::TableNumPkCols(
                uudst::kDbiVbrVlanMapTbl) - kVbrVlanMapNumChildKey;
            ) {
          do {
            // For skipping invalid inputs - do not delete
            // When log_portid_valid is invalid value, then
            // break and continue sibling opern with log_portid
            if (IsValidKey(queryckval->get_key(), childKeyIndex) == false) {
              break;
            }

            uint32_t db_read_count = (header->rep_count - result_count);
            result_code = ReadConfigDB(queryckval, dt_type, header->operation,
                                       dbop, db_read_count, dmi, tbl);

            if (result_code == UPLL_RC_SUCCESS) {
              UPLL_LOG_DEBUG("ReadConfigDB returns SUCCESS: %s \n",
                queryckval->ToStrAll().c_str());
              skip_count = 0;
              new_ckv = NULL;
              ConfigKeyVal *tmp = queryckval, *tmp_old = NULL;
              while (tmp) {
                uint8_t vlanmap_flag = 0;
                GET_USER_DATA_FLAGS(tmp, vlanmap_flag);
                uint8_t is_user_conf = false;
                is_user_conf = (USER_VLANMAP_FLAG & vlanmap_flag);
                /* If entry is not user configure. i.e boundary configured */
                if (!is_user_conf) {
                  ConfigKeyVal *ck_tmp =tmp;
                  skip_count++;
                  /* Move to next ckv */
                  tmp = tmp->get_next_cfg_key_val();
                  /* If next ckv is not present & there are skipped entries
                   * do readconfigdb again with last received ckv */
                  if (tmp == NULL) {
                    DELETE_IF_NOT_NULL(new_ckv);
                    if (filter_vlanid == true) {
                      result_code = DupConfigKeyVal(new_ckv, ck_tmp, MAINTBL);
                    } else {
                      result_code = GetChildConfigKey(new_ckv, ck_tmp);
                    }
                    if (result_code != UPLL_RC_SUCCESS || new_ckv == NULL) {
                      UPLL_LOG_DEBUG("Error while GetChildConfigKey or "
                          "DupConfigKeyVal. filter_vlanid: %d", filter_vlanid);
                      DELETE_IF_NOT_NULL(queryckval);
                      return UPLL_RC_ERR_GENERIC;
                    }
                  }
                  // if skipped element is 1st element, queryckval should
                  // point to next ckv
                  if (ck_tmp == queryckval) {
                    queryckval = ck_tmp->get_next_cfg_key_val();
                    ck_tmp->set_next_cfg_key_val(NULL);
                  } else {
                    // If any intermediate is skipped, removed the same
                    // in the list and update tmp_old's next ckv
                    tmp_old->set_next_cfg_key_val(
                        ck_tmp->get_next_cfg_key_val());
                    ck_tmp->set_next_cfg_key_val(NULL);
                  }
                  DELETE_IF_NOT_NULL(ck_tmp);
                  continue;

                } else {
                  tmp_old =tmp;
                  tmp = tmp->get_next_cfg_key_val();
                  if (tmp == NULL &&
                      (expect_count != (db_read_count - skip_count))) {
                    DELETE_IF_NOT_NULL(new_ckv);
                    if (filter_vlanid == true) {
                      result_code = DupConfigKeyVal(new_ckv, tmp_old, MAINTBL);
                    } else {
                      result_code = GetChildConfigKey(new_ckv, tmp_old);
                    }
                    if (result_code != UPLL_RC_SUCCESS || new_ckv == NULL) {
                      UPLL_LOG_DEBUG("Error while GetChildConfigKey or "
                          "DupConfigKeyVal. filter_vlanid: %d", filter_vlanid);
                      return UPLL_RC_ERR_GENERIC;
                    }
                  }
                }
              }
              found_count = (db_read_count-skip_count);
              if (found_count > 0) {
                // Collect the data into result_ckv.
                if (result_ckv == NULL) {
                  result_ckv = queryckval;
                } else {
                  result_ckv->AppendCfgKeyVal(queryckval);
                }
                result_count += (queryckval->size());
                queryckval = NULL;
              }
              if (found_count < expect_count) {
                queryckval = new_ckv;
                // Preparing Child Key data for next Sibling Iteration
                // log_portid_valid will be set to INVALID and further
                // rd_sibling query will be done with > log_portid
                if (queryckval &&  (ResetDataForSibling(
                    reinterpret_cast<key_vlan_map *>(queryckval->get_key()),
                    (uudst::vbridge_vlanmap::kVbrVlanMapIndex)tmp_childKeyIndex)
                    == false)) {
                  UPLL_LOG_DEBUG("Data Not Reset for the index(%d)",
                                 childKeyIndex);
                  delete queryckval;
                  return UPLL_RC_ERR_GENERIC;
                }

                UPLL_LOG_TRACE("Next Query After Reset: %s",
                               (queryckval->ToStrAll()).c_str());
                expect_count = expect_count - found_count;
                continue;
              } else {
                DELETE_IF_NOT_NULL(new_ckv);
                break;
              }
              // Used in ReadConfigDB in each Iteration
            } else if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
              break;
              // use queryckval in the next iteration;
            } else {
              UPLL_LOG_ERROR("Error while ReadConfigDB");
              DELETE_IF_NOT_NULL(queryckval);
              /* Convert the val struct to VTN service */
              upll_rc_t rc = AdaptValToVtnService(ikey, ADAPT_ALL);
              if (rc != UPLL_RC_SUCCESS) {
                UPLL_LOG_DEBUG("AdaptValToVtnService failed result_code %d",
                               rc);
                return UPLL_RC_ERR_GENERIC;
              }
              return result_code;
            }
          } while (1);  //  end of do while

          if (result_count >= header->rep_count) {
            break;  // break the for loop
          }
          //  Preparing Child Key data for next Sibling Iteration
          queryckval = new_ckv;  //  added for cnt>max
          if (queryckval && (ResetDataForSibling(
                  reinterpret_cast<key_vlan_map *>(queryckval->get_key()),
                  (uudst::vbridge_vlanmap::kVbrVlanMapIndex)childKeyIndex)
                == false)) {
            UPLL_LOG_DEBUG("Data Not Reset for the index(%d)", childKeyIndex);
            DELETE_IF_NOT_NULL(queryckval);
            return UPLL_RC_ERR_GENERIC;
          }
          tmp_childKeyIndex = childKeyIndex;
          childKeyIndex--;
          UPLL_LOG_TRACE("Next Query After Reset: %s",
                         (queryckval->ToStrAll()).c_str());
        }  // for
        DELETE_IF_NOT_NULL(queryckval);
        header->rep_count = result_count;
        if (result_ckv) {
          ikey->ResetWith(result_ckv);
          DELETE_IF_NOT_NULL(result_ckv);
        }
        break;
      }  // case
    default:
      return UPLL_RC_ERR_GENERIC;
  }

  upll_rc_t rc = AdaptValToVtnService(ikey, ADAPT_ALL);
  if (rc != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("AdaptValToVtnService failed result_code %d", rc);
    return UPLL_RC_ERR_GENERIC;
  }

  if (header->rep_count > 0) {
    result_code = (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE)
                  ? UPLL_RC_SUCCESS : result_code;
  } else {
    return UPLL_RC_ERR_NO_SUCH_INSTANCE;
  }
  return UPLL_RC_SUCCESS;
}  // VlanMapMoMgr::ReadSiblingMo

// To reset the data available in index to empty
// Currently implemented only for child index
bool VlanMapMoMgr::ResetDataForSibling(key_vlan_map *key_vmap,
    uudst::vbridge_vlanmap::kVbrVlanMapIndex index) {
  switch (index) {
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

upll_rc_t VlanMapMoMgr::UpdateParentOperStatus(ConfigKeyVal *ikey,
                                               DalDmlIntf *dmi,
                                               uint32_t driver_result) {
  UPLL_FUNC_TRACE;
  if (driver_result == UPLL_RC_ERR_CTR_DISCONNECTED) {
    return UPLL_RC_SUCCESS;
  }
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  key_vlan_map_t *vlanmap_key = NULL;
  vlanmap_key = reinterpret_cast<key_vlan_map_t*>(ikey->get_key());
  if (!vlanmap_key) {
    UPLL_LOG_ERROR("Bad ikey");
    return UPLL_RC_ERR_GENERIC;
  }

  MoMgrImpl *if_mgr = reinterpret_cast<MoMgrImpl*>(const_cast<MoManager *>(
      GetMoManager(UNC_KT_VBR_IF)));
  if (if_mgr == NULL) {
    UPLL_LOG_ERROR("VbrIfMoMgr does not exist");
    return UPLL_RC_ERR_GENERIC;
  }

  key_vbr_if_t *vbr_key_if = reinterpret_cast<key_vbr_if_t *>(
      ConfigKeyVal::Malloc(sizeof(key_vbr_if_t)));

  uuu::upll_strncpy(vbr_key_if->vbr_key.vtn_key.vtn_name,
                    vlanmap_key->vbr_key.vtn_key.vtn_name,
                    (kMaxLenVtnName + 1));
  uuu::upll_strncpy(vbr_key_if->vbr_key.vbridge_name,
                    vlanmap_key->vbr_key.vbridge_name,
                    (kMaxLenVnodeName + 1));
  ConfigKeyVal *tmp_if_key = new ConfigKeyVal(UNC_KT_VBR_IF,
                                              IpctSt::kIpcStKeyVbrIf,
                                              vbr_key_if,
                                              NULL);

  // Return if interface exists under the parent vBridge
  DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutNone };
  result_code = if_mgr->UpdateConfigDB(tmp_if_key, UPLL_DT_RUNNING, UNC_OP_READ,
                                       dmi, &dbop, MAINTBL);
  DELETE_IF_NOT_NULL(tmp_if_key);
  if (result_code == UPLL_RC_ERR_INSTANCE_EXISTS) {
    // vbridge operstatus does not depend on vlanmap when interface exists
    return UPLL_RC_SUCCESS;
  } else if (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    return result_code;
  }

  // Return if other vlan-map exists for the same parent vBridge
  ConfigKeyVal *ck_vlanmap = NULL;
  result_code = GetChildConfigKey(ck_vlanmap, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    return result_code;
  }
  key_vlan_map_t *tmp_vlan_key =
      reinterpret_cast<key_vlan_map_t*>(ck_vlanmap->get_key());
  tmp_vlan_key->logical_port_id_valid = INVALID_LOG_PORT_ID_VALID;
  tmp_vlan_key->logical_port_id[0] = 0;
  uint32_t count = 0;
  result_code = GetInstanceCount(ck_vlanmap, NULL, UPLL_DT_RUNNING, &count, dmi,
                                 MAINTBL);
  DELETE_IF_NOT_NULL(ck_vlanmap);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_ERROR("GetInstanceCount failed : %d", result_code);
    return result_code;
  }
  if (count > 1) {  // self vlanmap needs to be excluded
    return UPLL_RC_SUCCESS;
  }

  // There is no other vlanmap exists for the vbridge. Bring vbridge down.

  VnodeMoMgr *vbr_mgr = reinterpret_cast<VnodeMoMgr *>(const_cast<MoManager *>(
      GetMoManager(UNC_KT_VBRIDGE)));
  if (vbr_mgr == NULL) {
    UPLL_LOG_ERROR("VbrMoMgr does not exist");
    return UPLL_RC_ERR_GENERIC;
  }
  ConfigKeyVal *ck_vbr = NULL;
  result_code = GetParentConfigKey(ck_vbr, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    return result_code;
  }
  result_code = vbr_mgr->ReadConfigDB(ck_vbr, UPLL_DT_STATE, UNC_OP_READ, dbop,
                                      dmi, MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      // parent wont be there if parent or grandparent is getting deleted.
      result_code = UPLL_RC_SUCCESS;
    } else {
      UPLL_LOG_DEBUG("Returning error %d\n", result_code);
    }
    DELETE_IF_NOT_NULL(ck_vbr);
    return result_code;
  }
  result_code = vbr_mgr->SetStandAloneOperStatus(ck_vbr, kPortFault, dmi);
  DELETE_IF_NOT_NULL(ck_vbr);
  return result_code;
}

upll_rc_t VlanMapMoMgr::UpdateVnodeOperStatus(ConfigKeyVal *ikey,
                                               DalDmlIntf *dmi,
                                               uint32_t driver_result) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  if (!ikey) {
    UPLL_LOG_ERROR("Returning error \n");
    return UPLL_RC_ERR_GENERIC;
  }
  ConfigKeyVal *ck_vn = NULL;
  result_code = GetParentConfigKey(ck_vn, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Returning error %d\n", result_code);
    return result_code;
  }
  VnodeMoMgr *mgr = reinterpret_cast<VnodeMoMgr *>(const_cast<MoManager *>
                            (GetMoManager(ck_vn->get_key_type())));
  if (!mgr) {
    UPLL_LOG_DEBUG("Returning error \n");
    delete ck_vn;
    return UPLL_RC_ERR_GENERIC;
  }
  DbSubOp dbop = {kOpReadSingle, kOpMatchNone, kOpInOutNone};
  result_code = mgr->ReadConfigDB(ck_vn, UPLL_DT_STATE, UNC_OP_READ,
                                    dbop, dmi, MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Returning error %d\n", result_code);
    delete ck_vn;
    result_code = (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE)?
                   UPLL_RC_SUCCESS : result_code;
    return result_code;
  }
  val_db_vbr_st *valst = reinterpret_cast<val_db_vbr_st *>(GetStateVal(ck_vn));
  if (!valst) {
    UPLL_LOG_DEBUG("Returning error\n");
    delete ck_vn;
    return UPLL_RC_ERR_GENERIC;
  }
  if ((valst->down_count == 0) && (valst->unknown_count == 0) &&
      (driver_result != UPLL_RC_ERR_CTR_DISCONNECTED)) {
    result_code = mgr->UpdateOperStatus(ck_vn, dmi, kCommit, true);
  } else if (valst->down_count == STAND_ALONE_VNODE) {
    result_code = mgr->UpdateOperStatus(ck_vn, dmi, kPortUp, true);
  }
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Returning error %d\n", result_code);
  }
  delete ck_vn;
  return result_code;
}

upll_rc_t VlanMapMoMgr::AdaptValToVtnService(ConfigKeyVal *ikey,
                                             AdaptType adapt_type) {
  UPLL_FUNC_TRACE;
  if (!ikey) {
    UPLL_LOG_DEBUG("Invalid ikey");
    return UPLL_RC_ERR_GENERIC;
  }
  while (ikey) {
    ConfigVal *cval = ikey->get_cfg_val();
    while (cval) {
      if (IpctSt::kIpcStPfcdrvValVlanMap == cval->get_st_num()) {
        val_vlan_map_t *vlanmap_val = reinterpret_cast<val_vlan_map_t *>
            (ConfigKeyVal::Malloc(sizeof(val_vlan_map_t)));
        pfcdrv_val_vlan_map_t *drv_val =
            reinterpret_cast<pfcdrv_val_vlan_map_t *>(cval->get_val());
        memcpy(vlanmap_val, &(drv_val->vm), sizeof(val_vlan_map_t));
        cval->SetVal(IpctSt::kIpcStValVlanMap, vlanmap_val);
        /* do not display portmap info if not configured (for boundary)*/
        uint8_t vlink_flag = 0;
        GET_USER_DATA_FLAGS(ikey, vlink_flag);
        UPLL_LOG_DEBUG("Interface type %d", vlink_flag);
      }
      cval = cval->get_next_cfg_val();
    }
    if (adapt_type == ADAPT_ONE)
      break;
    ikey = ikey->get_next_cfg_key_val();
  }

  return UPLL_RC_SUCCESS;
}

/* This function is over-ridden from momgr_impl.cc */
upll_rc_t VlanMapMoMgr::UpdateMo(IpcReqRespHeader *req,
                                 ConfigKeyVal *ikey,
                                 DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;

  upll_rc_t result_code = UPLL_RC_ERR_GENERIC;
  if (NULL == ikey || NULL == req || !(ikey->get_key())) {
    UPLL_LOG_ERROR("Given Input is Empty");
    return UPLL_RC_ERR_GENERIC;
  }

  UPLL_LOG_INFO("UpdateMo for %d", ikey->get_key_type());
  /* Validates received request */
  result_code = ValidateMessage(req, ikey);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_ERROR("Validation Message is Failed ");
    return result_code;
  }

  controller_domain ctrlr_dom;
  ctrlr_dom.ctrlr = ctrlr_dom.domain = NULL;
  ConfigKeyVal      *temp_ck         = NULL;

  result_code = GetChildConfigKey(temp_ck, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_ERROR("Error in retrieving the Child ConfigKeyVal");
    if (temp_ck)
      delete temp_ck;
    return result_code;
  }

  DbSubOp dbop = { kOpReadSingle, kOpMatchNone,
    kOpInOutFlag | kOpInOutCtrlr| kOpInOutDomain};
  /* Reads vlanmap info based on recieved ikey->key_ */
  result_code = ReadConfigDB(temp_ck, req->datatype, UNC_OP_READ, dbop,
                             dmi, MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("ReadConfigDB Failed %d", result_code);
    delete temp_ck;
    return result_code;
  }

  val_vlan_map_t *ival = NULL;
  if ((ikey->get_cfg_val())->get_st_num() == IpctSt::kIpcStValVlanMap) {
    ival = reinterpret_cast<val_vlan_map_t *>
        (GetVal(ikey));
  } else {
    pfcdrv_val_vlan_map_t *drv_val = reinterpret_cast<pfcdrv_val_vlan_map_t *>
        (GetVal(ikey));
    ival = &(drv_val->vm);
  }

  // To validate all the flags are INVALID during Update
  bool is_invalid = IsAllAttrInvalid(ival);
  if (is_invalid) {
    UPLL_LOG_INFO("No attributes to be updated");
    DELETE_IF_NOT_NULL(temp_ck);
    return UPLL_RC_SUCCESS;
  }
  GET_USER_DATA_CTRLR_DOMAIN(temp_ck, ctrlr_dom);
  SET_USER_DATA_CTRLR_DOMAIN(ikey, ctrlr_dom);
  uint8_t flags = 0;
  GET_USER_DATA_FLAGS(temp_ck, flags);

  pfcdrv_val_vlan_map_t *tval = reinterpret_cast<pfcdrv_val_vlan_map_t *>
      (GetVal(temp_ck));

  UPLL_LOG_TRACE("Vlanmap bdry_ref_count = %u", tval->bdry_ref_count);
  /* Verifies whether the recived vlanmap is refered with boundary vlink  */
  if ((tval->bdry_ref_count > 1) ||
      ((tval->bdry_ref_count > 0) && (flags & USER_VLANMAP_FLAG))) {
    if (tval->vm.vlan_id != ival->vlan_id) {
      UPLL_LOG_DEBUG("Cannot update vlan-id more than one vlan map refered");
      delete temp_ck;
      return UPLL_RC_ERR_CFG_SEMANTIC;
    }
  }

  if (tval->vm.vlan_id == ival->vlan_id) {
    UPLL_LOG_ERROR("Already same vlan-id is there in vlanmap entry."
                   "No need to update");
    delete temp_ck;
    return UPLL_RC_SUCCESS;
  }

  result_code = ValidateCapability(
      req, ikey, reinterpret_cast<char *>(ctrlr_dom.ctrlr));
  if (UPLL_RC_SUCCESS  != result_code) {
    UPLL_LOG_DEBUG("Validate Capability is Failed. Error_code : %d",
                   result_code);
    delete temp_ck;
    return result_code;
  }

  delete temp_ck;

  ConfigKeyVal *okey = NULL;
  result_code = DupConfigKeyVal(okey, ikey, MAINTBL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG(" DupConfigKeyVal Failed %d", result_code);
    return result_code;
  }

  pfcdrv_val_vlan_map_t *val = reinterpret_cast<pfcdrv_val_vlan_map_t *>
      (ConfigKeyVal::Malloc(sizeof(pfcdrv_val_vlan_map_t)));

  void *valvlanmap = GetVal(ikey);
  if ((ikey->get_cfg_val())->get_st_num() == IpctSt::kIpcStValVlanMap)
    memcpy(&(val->vm), valvlanmap, sizeof(val_vlan_map_t));
  else
    memcpy(val, valvlanmap, sizeof(pfcdrv_val_vlan_map_t));
  (okey->get_cfg_val())->SetVal(IpctSt::kIpcStPfcdrvValVlanMap, val);

  TcConfigMode config_mode = TC_CONFIG_INVALID;
  std::string vtn_name = "";
  result_code = GetConfigModeInfo(req, config_mode, vtn_name);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetConfigMode failed");
    return result_code;
  }

  // ValidateAttribute needs Controller Domain
  result_code = ValidateAttribute(okey, dmi, req);
  if (UPLL_RC_SUCCESS  != result_code) {
    UPLL_LOG_ERROR("Validate Attribute is Failed");
    DELETE_IF_NOT_NULL(okey);
    return result_code;
  }

  // If the config mode is VTN mode, then ValidateAttribute is
  // invoked with datatype RUNNING
  if (req->datatype == UPLL_DT_CANDIDATE && config_mode == TC_CONFIG_VTN) {
    req->datatype = UPLL_DT_RUNNING;
    result_code = ValidateAttribute(okey, dmi, req);
    if (UPLL_RC_SUCCESS  != result_code) {
      delete okey;
      UPLL_LOG_ERROR("Validate Attribute is Failed");
      req->datatype = UPLL_DT_CANDIDATE;
      return result_code;
    }
    req->datatype = UPLL_DT_CANDIDATE;
  }

  DbSubOp dbop1 = { kOpNotRead, kOpMatchNone, kOpInOutNone };
  result_code = UpdateConfigDB(okey, req->datatype, UNC_OP_UPDATE,
                               dmi, &dbop1, config_mode, vtn_name, MAINTBL);
  DELETE_IF_NOT_NULL(okey);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_ERROR("Updation Failure in DB : %d", result_code);
    return result_code;
  }
  UPLL_LOG_TRACE("Updated Done Successfully %d", result_code);
  return result_code;
}

upll_rc_t VlanMapMoMgr::DeleteMo(IpcReqRespHeader *req,
                                 ConfigKeyVal *ikey,
                                 DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  if (!ikey || !req || !(ikey->get_key())) {
    UPLL_LOG_ERROR("Given Input is Empty")
        return UPLL_RC_ERR_GENERIC;
  }

  result_code = ValidateMessage(req, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_ERROR("Validate Message is Failed %d ", result_code);
    return result_code;
  }

  ConfigKeyVal *dup_ckv = NULL;
  result_code = GetChildConfigKey(dup_ckv, ikey);

  DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutFlag };
  result_code = ReadConfigDB(dup_ckv, req->datatype, UNC_OP_READ, dbop,
                             dmi, MAINTBL);
  UPLL_LOG_TRACE("Read config DB result code = %u", result_code);

  if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    UPLL_LOG_ERROR("Record Doesn't Exists in DB ");
    delete dup_ckv;
    return result_code;
  }

  pfcdrv_val_vlan_map_t *drv_val = reinterpret_cast<pfcdrv_val_vlan_map_t *>
      (GetVal(dup_ckv));
  uint8_t flags = 0;
  GET_USER_DATA_FLAGS(dup_ckv, flags);

  UPLL_LOG_TRACE("Vlanmap flag = %u. bdry_ref_count = %u", flags,
                 drv_val->bdry_ref_count);
  if ((drv_val->bdry_ref_count > 0) && (flags & USER_VLANMAP_FLAG)) {
    flags &= ~USER_VLANMAP_FLAG;
    SET_USER_DATA_FLAGS(dup_ckv, flags);
    UPLL_LOG_TRACE("Flag to be update = %u", flags);
    TcConfigMode config_mode = TC_CONFIG_INVALID;
    std::string vtn_name = "";
    result_code = GetConfigModeInfo(req, config_mode, vtn_name);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("GetConfigMode failed");
      return result_code;
    }

    DbSubOp dbop1 = { kOpNotRead, kOpMatchNone, kOpInOutFlag };
    result_code = UpdateConfigDB(dup_ckv, req->datatype, UNC_OP_UPDATE,
                                 dmi, &dbop1, config_mode, vtn_name, MAINTBL);
    delete dup_ckv;
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Update vlanmap flag is failed");
      return result_code;
    }
    return UPLL_RC_SUCCESS;
  }


  void *valvlanmap = GetVal(ikey);
  if (ikey->get_cfg_val()) {
    pfcdrv_val_vlan_map_t *val = reinterpret_cast<pfcdrv_val_vlan_map_t *>
      (ConfigKeyVal::Malloc(sizeof(pfcdrv_val_vlan_map_t)));
    if ((ikey->get_cfg_val())->get_st_num() ==
        IpctSt::kIpcStPfcdrvValVlanMap) {
      memcpy(val, valvlanmap, sizeof(pfcdrv_val_vlan_map_t));
    } else {
      memcpy(&(val->vm), valvlanmap, sizeof(val_vlan_map_t));
    }
    (dup_ckv->get_cfg_val())->SetVal(IpctSt::kIpcStPfcdrvValVlanMap, val);
  } else {
    dup_ckv->DeleteCfgVal();
  }

  result_code = DeleteCandidateMo(req, dup_ckv, dmi);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_ERROR("Returning Error - %d", result_code);
  }
  DELETE_IF_NOT_NULL(dup_ckv);
  return result_code;
}

upll_rc_t VlanMapMoMgr::CheckIfFfPmConfigured(IpcReqRespHeader *req,
                                              ConfigKeyVal *ikey,
                                              DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;

  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *ckv = NULL;

  /* Verifies vbrif_flowfilter tbl */
  VbrIfFlowFilterMoMgr *vbrif_ff_mgr = reinterpret_cast<VbrIfFlowFilterMoMgr *>(
      const_cast<MoManager *>(GetMoManager(UNC_KT_VBRIF_FLOWFILTER)));
  if (!vbrif_ff_mgr) {
    UPLL_LOG_DEBUG("Invalid mgr");
    return UPLL_RC_ERR_GENERIC;
  }

  result_code = vbrif_ff_mgr->GetChildConfigKey(ckv, ikey);
  if (result_code != UPLL_RC_SUCCESS) return UPLL_RC_ERR_GENERIC;

  DbSubOp dbop = {kOpReadMultiple, kOpMatchNone, kOpInOutNone};
  /*Checks the vbrif_flowfilter table*/
  result_code = vbrif_ff_mgr->ReadConfigDB(ckv, req->datatype, UNC_OP_READ,
                                           dbop, dmi, MAINTBL);
  if (result_code != UPLL_RC_SUCCESS &&
      result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    UPLL_LOG_ERROR("Failed to read vbrif flowfilter info from DB");
    delete ckv;
    return result_code;
  }

  if (result_code == UPLL_RC_SUCCESS) {
    UPLL_LOG_ERROR("SD or SW Boundary vlink cannot be configured."
                   "Flowfilter is present");
    delete ckv;
    return UPLL_RC_ERR_CFG_SEMANTIC;
  }
  delete ckv;
  ckv = NULL;

  /* Validates vbrif_policingmap tbl */
  VbrIfPolicingMapMoMgr *vbrif_pm_mgr =
      reinterpret_cast<VbrIfPolicingMapMoMgr *>(
      const_cast<MoManager *>(GetMoManager(UNC_KT_VBRIF_POLICINGMAP)));
  if (!vbrif_pm_mgr) {
    UPLL_LOG_DEBUG("Invalid mgr");
    return UPLL_RC_ERR_GENERIC;
  }

  result_code = vbrif_pm_mgr->GetChildConfigKey(ckv, ikey);
  if (result_code != UPLL_RC_SUCCESS) return UPLL_RC_ERR_GENERIC;

  /*Checks the vbrif_flowfilter table*/
  result_code = vbrif_pm_mgr->ReadConfigDB(ckv, req->datatype, UNC_OP_READ,
                                           dbop, dmi, MAINTBL);
  if (result_code != UPLL_RC_SUCCESS &&
      result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    UPLL_LOG_ERROR("Failed to read vbrif policingmap info from DB");
    delete ckv;
    return result_code;
  }

  if (result_code == UPLL_RC_SUCCESS) {
    UPLL_LOG_ERROR("SD or SW Boundary vlink cannot be configured."
                   "Policingmap is present");
    delete ckv;
    return UPLL_RC_ERR_CFG_SEMANTIC;
  }
  delete ckv;
  ckv = NULL;
  return UPLL_RC_SUCCESS;
}

upll_rc_t VlanMapMoMgr::BoundaryMapReq(IpcReqRespHeader *req,
                                           ConfigKeyVal *ikey,
                                           ConfigKeyVal *db_vlink,
                                           ConfigKeyVal *vlanmap_ckv,
                                           ConfigKeyVal *ck_boundary,
                                           DalDmlIntf *dmi) {
  uint8_t        flags        = 0;
  upll_rc_t      result_code  = UPLL_RC_SUCCESS;

  if (!req || !ikey || !db_vlink || !vlanmap_ckv || !dmi) {
    UPLL_LOG_INFO("Cannot perform this request"
                  "due to insufficient parameters");
    return UPLL_RC_ERR_GENERIC;
  }

  val_vlink_t *ival = reinterpret_cast<val_vlink *>(GetVal(ikey));

  val_vlink_t *tval = reinterpret_cast<val_vlink *>(GetVal(db_vlink));
  if (!ival || !tval) {
    UPLL_LOG_DEBUG("vlink val structure not present");
    return UPLL_RC_ERR_GENERIC;
  }

  //  Get aquired configuration mode and vtn_name
  TcConfigMode config_mode = TC_CONFIG_INVALID;
  std::string vtn_name = "";
  result_code = GetConfigModeInfo(req, config_mode, vtn_name);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetConfigMode failed");
    return result_code;
  }

  /* Get the vnode position */
  GET_USER_DATA_FLAGS(db_vlink->get_cfg_val(), flags);
  flags &= VLINK_FLAG_NODE_POS;
  UPLL_LOG_TRACE("Vnode position flag = %u", flags);

  /* ck_boundary ConfigKeyVal contains logical port id info */
  if (!ck_boundary && !GetVal(ck_boundary)) {
    UPLL_LOG_DEBUG("Boundary details is not there");
    return UPLL_RC_ERR_GENERIC;
  }

  val_boundary_t *boundary_val = reinterpret_cast<val_boundary_t *>(
      GetVal(ck_boundary));
  key_vlan_map_t *vlanmap_key  = reinterpret_cast<key_vlan_map_t *>(
      vlanmap_ckv->get_key());

  uint8_t *logical_port_id = NULL;
  controller_domain vlanmap_ctrlr_dom;
  vlanmap_ctrlr_dom.ctrlr = vlanmap_ctrlr_dom.domain = NULL;

  /* Verifies controller_name and domain name and gets
   * vlanmap logical_port_id */
  GET_USER_DATA_CTRLR_DOMAIN(vlanmap_ckv, vlanmap_ctrlr_dom);
  UPLL_LOG_TRACE("ctrlr_name = %s domain_name = %s", vlanmap_ctrlr_dom.ctrlr,
      vlanmap_ctrlr_dom.domain);

  /* If controller or domain is NULL, return error */
  if (!vlanmap_ctrlr_dom.ctrlr || !vlanmap_ctrlr_dom.domain) {
    UPLL_LOG_ERROR("Controller_name or domain_name is NULL");
    return UPLL_RC_ERR_GENERIC;
  }

  if ((!strcmp(reinterpret_cast<char *>(vlanmap_ctrlr_dom.ctrlr),
     reinterpret_cast<char *>(boundary_val->controller_name1))) &&
     (!strcmp(reinterpret_cast<char *>(vlanmap_ctrlr_dom.domain),
               reinterpret_cast<char *>(boundary_val->domain_name1)))) {
    logical_port_id = boundary_val->logical_port_id1;
  } else if ((!strcmp(reinterpret_cast<char *>(vlanmap_ctrlr_dom.ctrlr),
          reinterpret_cast<char *>(boundary_val->controller_name2))) &&
      (!strcmp(reinterpret_cast<char *>(vlanmap_ctrlr_dom.domain),
               reinterpret_cast<char *>(boundary_val->domain_name2)))) {
    logical_port_id = boundary_val->logical_port_id2;
  } else {
    UPLL_LOG_ERROR(" Controller and domain match not found");
    return UPLL_RC_ERR_CFG_SEMANTIC;
  }

  /* Populate the logcial port id before updating vlanmap tbl */
  if (logical_port_id) {
    uuu::upll_strncpy(vlanmap_key->logical_port_id, logical_port_id,
                      (kMaxLenLogicalPortId + 1));
    vlanmap_key->logical_port_id_valid = PFC_TRUE;
  }

  pfcdrv_val_vlan_map_t *val_vlanmap = NULL;
  /*Checks the existance of vlanmap entry in vlanmap table */
  DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutFlag };
  result_code = ReadConfigDB(vlanmap_ckv, req->datatype, UNC_OP_READ, dbop,
                             dmi, MAINTBL);
  /* For update config DB changed to kOpNotRead */
  dbop.readop = kOpNotRead;

  UPLL_LOG_TRACE("Read config DB result code = %u", result_code);
  if (result_code != UPLL_RC_SUCCESS &&
      result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    UPLL_LOG_DEBUG("Failed to read vlanmap info from DB");
    return result_code;
  }


  /* Boundary vlanmap create */
  if ((tval->valid[UPLL_IDX_BOUNDARY_NAME_VLNK] == UNC_VF_INVALID) &&
      ival->valid[UPLL_IDX_BOUNDARY_NAME_VLNK] == UNC_VF_VALID) {
    UPLL_LOG_TRACE("Boundary Vlanmap Create");
    req->operation = UNC_OP_CREATE;

    if ((ival->valid[UPLL_IDX_LABEL_VLNK] != UNC_VF_VALID) ||
        (ival->label == 0xFFFF)) {
      UPLL_LOG_ERROR("vlan-id is mandatory when boundary port is SW or SD");
      return UPLL_RC_ERR_CFG_SEMANTIC;
    }

    val_vlanmap = reinterpret_cast<pfcdrv_val_vlan_map_t *>
        (GetVal(vlanmap_ckv));

    if (!val_vlanmap) {
      return UPLL_RC_ERR_GENERIC;
    }

     /* Vlanmap entry is there in DB */
    if (result_code == UPLL_RC_SUCCESS) {
      /* vlan-id in existing entry is different from vlan-id in the request */
      if (val_vlanmap->vm.vlan_id != ival->label) {
        UPLL_LOG_ERROR("vlan-id cannot be modified if vlanmap is "
                       "referenced by user or boundary");
        return UPLL_RC_ERR_CFG_SEMANTIC;
      }
      /* Increment ref_count */
      UPLL_LOG_TRACE("Old vlanmap bdry_ref_count:%u",
                     val_vlanmap->bdry_ref_count);

      /* Already user configured vlan_map exists and its
       * not referred to any boundary vlink */
      if (val_vlanmap->bdry_ref_count == 0) {
        /* Verifies flow filter or policing map configured in the same
           vbridge interface */
        result_code = CheckIfFfPmConfigured(req, db_vlink, dmi);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_ERROR("Flow-filter or policing-map configured on "
              "vBridge interface of boundary vlanmap");
          return result_code;
        }
      }
      val_vlanmap->bdry_ref_count++;
      val_vlanmap->valid[PFCDRV_IDX_BDRY_REF_COUNT] = UNC_VF_VALID;

      /* Set boundary configured bit in vlan_map flag*/
      GET_USER_DATA_FLAGS(vlanmap_ckv, flags);
      flags |= BOUNDARY_VLANMAP_FLAG;
      SET_USER_DATA_FLAGS(vlanmap_ckv, flags);

      result_code = UpdateConfigDB(vlanmap_ckv, req->datatype, UNC_OP_UPDATE,
                                   dmi, &dbop, config_mode, vtn_name, MAINTBL);
      if (UPLL_RC_SUCCESS != result_code) {
        UPLL_LOG_ERROR("bdry_ref_count update failed");
        return result_code;
      }
      return result_code;
    }
    val_vlanmap->vm.vlan_id = ival->label;
    val_vlanmap->vm.valid[UPLL_IDX_VLAN_ID_VM] = UNC_VF_VALID;
    val_vlanmap->valid[PFCDRV_IDX_VAL_VLAN_MAP] = UNC_VF_VALID;

    val_vlanmap->bdry_ref_count = 1;
    val_vlanmap->valid[PFCDRV_IDX_BDRY_REF_COUNT] = UNC_VF_VALID;

    /*Creates boundary vlanmap under vbridge with bdry_ref_count as 1*/
    SET_USER_DATA_FLAGS(vlanmap_ckv, BOUNDARY_VLANMAP_FLAG);

    UPLL_LOG_TRACE("%s", vlanmap_ckv->ToStrAll().c_str());
    UPLL_LOG_TRACE("%s", db_vlink->ToStrAll().c_str());
    /* Verifies flow filter or policing map configured in the same
       vbridge interface */
    result_code = CheckIfFfPmConfigured(req, db_vlink, dmi);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_ERROR("Flow-filter or policing-map configured on "
                     "vBridge interface of boundary vlanmap");
      return result_code;
    }

    /* Creates vlanmap entry in vlanmap table */
    result_code = VnodeChildMoMgr::CreateCandidateMo(req, vlanmap_ckv, dmi);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_ERROR("Boundary vlanmap creation failed");
      return result_code;
    }
    return result_code;
  }

  /* Boundary vlink vlanmap update */
  if ((tval->valid[UPLL_IDX_BOUNDARY_NAME_VLNK] == UNC_VF_VALID) &&
      (ival->valid[UPLL_IDX_BOUNDARY_NAME_VLNK] == UNC_VF_VALID)) {
    val_vlanmap = reinterpret_cast<pfcdrv_val_vlan_map_t *>
        (GetVal(vlanmap_ckv));

    if (!val_vlanmap) {
       return UPLL_RC_ERR_GENERIC;
    }

    if ((ival->valid[UPLL_IDX_LABEL_VLNK] != UNC_VF_VALID) ||
        (ival->label == 0xFFFF)) {
      UPLL_LOG_ERROR("vlan-id is mandatory when boundary port is SW or SD");
      return UPLL_RC_ERR_CFG_SEMANTIC;
    }

    if (result_code == UPLL_RC_SUCCESS) {
      GET_USER_DATA_FLAGS(vlanmap_ckv, flags);

      UPLL_LOG_TRACE("vlanmap flags = %u. vlanmap bdry_ref_count = %u",
                     flags, val_vlanmap->bdry_ref_count);
      /* Update of boundary vlanmap vlan-id is not possible if same
         vlan map is refered with another vlink or user configured vlanmap */
      if ((val_vlanmap->vm.vlan_id != ival->label) &&
          ((flags & USER_VLANMAP_FLAG) || val_vlanmap->bdry_ref_count > 1)) {
        UPLL_LOG_ERROR("vlan-id cannot be modified if vlanmap is "
                       "referenced by user or boundary");
        return UPLL_RC_ERR_CFG_SEMANTIC;
      }
    }

    if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      UPLL_LOG_ERROR("No vlanmap entry exists for boundary vlanmap update");
      return UPLL_RC_ERR_CFG_SEMANTIC;
    }

    /* Updates boundary vlanmap vlan-id */
    val_vlanmap->vm.vlan_id = ival->label;
    val_vlanmap->vm.valid[UPLL_IDX_VLAN_ID_VM] = UNC_VF_VALID;
    req->operation = UNC_OP_UPDATE;

    UPLL_LOG_TRACE("%s", vlanmap_ckv->ToStrAll().c_str());
    result_code = UpdateMo(req, vlanmap_ckv, dmi);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_ERROR("Boundary vlanmap update failed. error:%u",
                     result_code);
      return result_code;
    }

    return result_code;
  }

  /* Boundary vlanmap delete */
  if ((ival->valid[UPLL_IDX_BOUNDARY_NAME_VLNK] == UNC_VF_VALID_NO_VALUE) &&
      (tval->valid[UPLL_IDX_BOUNDARY_NAME_VLNK] == UNC_VF_VALID)) {
    req->operation = UNC_OP_DELETE;

    val_vlanmap =
        reinterpret_cast<pfcdrv_val_vlan_map_t *>(GetVal(vlanmap_ckv));
    if (!val_vlanmap) {
      return UPLL_RC_ERR_GENERIC;
    }

    if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      UPLL_LOG_ERROR("Cannot update boundary vlan-id. "
                     "No entry in vlanmap table");
      return UPLL_RC_ERR_CFG_SEMANTIC;
    }

    if (result_code == UPLL_RC_SUCCESS) {
      GET_USER_DATA_FLAGS(vlanmap_ckv, flags);

      /* If vlan-id in the delete request is different from
       * the existing vlan-id */
      if (ival->valid[UPLL_IDX_LABEL_VLNK] == UNC_VF_VALID) {
        if (val_vlanmap->vm.vlan_id != ival->label) {
          UPLL_LOG_ERROR("Vlan-id in the request does not match "
            "with the existing vlan-id");
          return UPLL_RC_ERR_CFG_SEMANTIC;
        }
      }

      /* Resets Boundary vlanmap flag in vlanmap entry */
      if (((flags & USER_VLANMAP_FLAG) && val_vlanmap->bdry_ref_count > 0)||
          (flags & USER_VLANMAP_FLAG) || val_vlanmap->bdry_ref_count > 1) {
        UPLL_LOG_TRACE("Boundary ref_count = %u", val_vlanmap->bdry_ref_count);
        uint8_t flag = 0;
        if (val_vlanmap->bdry_ref_count == 1) {
          GET_USER_DATA_FLAGS(vlanmap_ckv, flag);
          flag &= ~BOUNDARY_VLANMAP_FLAG;
          SET_USER_DATA_FLAGS(vlanmap_ckv, flag);
          val_vlanmap->valid[PFCDRV_IDX_BDRY_REF_COUNT] = UNC_VF_VALID_NO_VALUE;
        }
        val_vlanmap->bdry_ref_count--;

        /* Decrements */
        result_code = UpdateConfigDB(vlanmap_ckv, req->datatype, UNC_OP_UPDATE,
                                     dmi, &dbop, config_mode, vtn_name,
                                     MAINTBL);
        if (UPLL_RC_SUCCESS != result_code) {
          UPLL_LOG_ERROR("Ref count DB update failed");
          return result_code;
        }
        return result_code;
      }
    }

    /* Deletes vlanmap entry from db */
    result_code = DeleteMo(req, vlanmap_ckv, dmi);
    UPLL_LOG_TRACE("%s", vlanmap_ckv->ToStrAll().c_str());
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_ERROR("Failed to boundary vlink");
      return result_code;
    }
    return result_code;
  }
  UPLL_LOG_DEBUG("Invalid request received");
  return UPLL_RC_ERR_GENERIC;
}

upll_rc_t VlanMapMoMgr::TranslateVlanmapError(
    ConfigKeyVal **err_ckv, ConfigKeyVal *ckv_running,
    DalDmlIntf *dmi, upll_keytype_datatype_t datatype) {
  UPLL_FUNC_TRACE;
  upll_rc_t     result_code         = UPLL_RC_SUCCESS;
  ConfigKeyVal  *ck_vlanmap         = NULL;
  uint8_t       bound_vlanmap_vlink = 0;
  UPLL_LOG_TRACE("concurrency %s", (ckv_running->ToStrAll()).c_str());
  /* Get duplicate Config key from ckv running*/
  result_code   = GetChildConfigKey(ck_vlanmap, ckv_running);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_INFO("Error in GetChildConfigKey");
    return result_code;
  }

  GET_USER_DATA_FLAGS(ck_vlanmap, bound_vlanmap_vlink);

  ConfigKeyVal *err_ckv2 = NULL;
  result_code = DupConfigKeyVal(err_ckv2, ckv_running);
  if (result_code != UPLL_RC_SUCCESS) {
    DELETE_IF_NOT_NULL(ck_vlanmap);
    UPLL_LOG_INFO("Error in DupConfigKeyVal");
    return result_code;
  }
  UPLL_LOG_INFO("Flag of failed VLANMAP is %d", bound_vlanmap_vlink);

  /* Checks if it is part of only user configured vlanmap */
  if ((bound_vlanmap_vlink & BOUNDARY_VLANMAP_FLAG) == 0x0) {
    *err_ckv = err_ckv2;
  } else {
    /* Checks it is part of boundary vlink */
    if (bound_vlanmap_vlink & BOUNDARY_VLANMAP_FLAG) {
      VlinkMoMgr *mgr = reinterpret_cast<VlinkMoMgr *>
        (const_cast<MoManager*>(GetMoManager(UNC_KT_VLINK)));
      if (!mgr) {
        UPLL_LOG_DEBUG("Invalid mgr");
        DELETE_IF_NOT_NULL(ck_vlanmap);
        DELETE_IF_NOT_NULL(err_ckv2);
        return UPLL_RC_ERR_GENERIC;
      }
      ConfigKeyVal *ck_vlink = NULL;

      result_code = mgr->GetVlinkKeyValFromVlanMap(ck_vlanmap, ck_vlink, dmi,
                                                   datatype);
      DELETE_IF_NOT_NULL(ck_vlanmap);
      if (result_code == UPLL_RC_SUCCESS) {
        UPLL_LOG_TRACE("Vlinks associated to logical_port_id are retrieved");
        if (bound_vlanmap_vlink & USER_VLANMAP_FLAG) {
          result_code = AdaptValToVtnService(err_ckv2, ADAPT_ONE);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("AdaptValToVtnService failed result_code %d",
                result_code);
            DELETE_IF_NOT_NULL(err_ckv2);
            return result_code;
          }
          err_ckv2->AppendCfgKeyVal(ck_vlink);
          *err_ckv = err_ckv2;
        } else {
          DELETE_IF_NOT_NULL(err_ckv2);
          *err_ckv = ck_vlink;
        }
        UPLL_LOG_TRACE("%s", (*err_ckv)->ToStrAll().c_str());
        return result_code;
      } else  {
        UPLL_LOG_INFO("Failed to map boundary if to vlink."
          "result_code:%d", result_code);
        /* For errors, no need to update err_ckv */
        DELETE_IF_NOT_NULL(err_ckv2);
        if (ck_vlink)
          delete ck_vlink;
        return result_code;
      }
    }
  }
  DELETE_IF_NOT_NULL(ck_vlanmap);

  if (*err_ckv) {
    result_code = AdaptValToVtnService(*err_ckv, ADAPT_ONE);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("AdaptValToVtnService failed result_code %d",
                     result_code);
      return result_code;
    }
  }
  return result_code;
}

upll_rc_t VlanMapMoMgr::CheckIfVnodeisVlanmapped(ConfigKeyVal *ikey,
                                   DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  ConfigKeyVal *ck_vlanmap = NULL;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  result_code = GetChildConfigKey(ck_vlanmap, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Error in GetChildConfigKey");
    return result_code;
  }
  key_vlan_map *vlanmap_key = reinterpret_cast<key_vlan_map*>
                                              (ck_vlanmap->get_key());
  if (vlanmap_key == NULL) {
    UPLL_LOG_DEBUG("Error in getting key");
    DELETE_IF_NOT_NULL(ck_vlanmap);
    return UPLL_RC_ERR_GENERIC;
  }
  vlanmap_key->logical_port_id_valid = INVALID_LOG_PORT_ID_VALID;

  DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutNone };
  result_code = UpdateConfigDB(ck_vlanmap, UPLL_DT_RUNNING, UNC_OP_READ,
                                          dmi, &dbop, MAINTBL);
  DELETE_IF_NOT_NULL(ck_vlanmap);
  return result_code;
}

upll_rc_t VlanMapMoMgr::PartialMergeValidate(unc_key_type_t keytype,
                                               const char *ctrlr_id,
                                               ConfigKeyVal *err_ckv,
                                               DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  ConfigKeyVal *run_ckv = NULL;
  string vtn_id = "";
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  if (!ctrlr_id || !err_ckv) {
    UPLL_LOG_DEBUG("Invalid input");
    return UPLL_RC_ERR_GENERIC;
  }
  result_code = GetChildConfigKey(run_ckv, NULL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetChildConfigKey failed");
    return result_code;
  }
  SET_USER_DATA_CTRLR(run_ckv, ctrlr_id);
  DbSubOp dbop = { kOpReadMultiple, kOpMatchCtrlr, kOpInOutFlag };

  result_code = ReadConfigDB(run_ckv, UPLL_DT_RUNNING, UNC_OP_READ, dbop,
                                            dmi, MAINTBL);
  if (UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code &&
      UPLL_RC_SUCCESS != result_code) {
    delete run_ckv;
    UPLL_LOG_DEBUG("ReadConfigDB failed %d", result_code);
    return result_code;
  }
  if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
    /*
     * In Running there is not vlanmap so no boundary information
     * will available so reutrun success
     */
    delete run_ckv;
    return UPLL_RC_SUCCESS;
  }
  uint8_t rename_flag = 0x00;
  ConfigKeyVal *start_ckv = run_ckv;
  while (run_ckv) {
    GET_USER_DATA_FLAGS(run_ckv, rename_flag);
    if (rename_flag & BOUNDARY_VLANMAP_FLAG) {
      ConfigKeyVal *imp_ckv = NULL;
      result_code = GetChildConfigKey(imp_ckv, run_ckv);
      if (UPLL_RC_SUCCESS != result_code) {
        delete start_ckv;
        UPLL_LOG_DEBUG("DupConfigKeyVal failed");
        return result_code;
      }
      dbop.readop = kOpReadSingle;
      result_code = ReadConfigDB(imp_ckv, UPLL_DT_IMPORT, UNC_OP_READ,
                                 dbop, dmi, MAINTBL);
      if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
        delete imp_ckv;
        err_ckv->ResetWithoutNextCkv(run_ckv);
        delete start_ckv;
        UPLL_LOG_DEBUG("MergeConflict with %s", err_ckv->ToStr().c_str());
        return UPLL_RC_ERR_MERGE_CONFLICT;
      }
      pfcdrv_val_vlan_map  *run_val = reinterpret_cast<pfcdrv_val_vlan_map *>(
          GetVal(run_ckv));
      pfcdrv_val_vlan_map *imp_val = reinterpret_cast<pfcdrv_val_vlan_map *>(
          GetVal(imp_ckv));

      /*
       * Checks only the vlan id so need to
       * check only this value
       */
      /*
       * If both are valid anc checks the value
       */
       if (run_val->vm.valid[UPLL_IDX_VLAN_ID_VM] == UNC_VF_VALID &&
           imp_val->vm.valid[UPLL_IDX_VLAN_ID_VM] == UNC_VF_VALID) {
         /*
          * if value is different then merge conflict
          */
         if (run_val->vm.vlan_id != imp_val->vm.vlan_id) {
           delete imp_ckv;
           err_ckv->ResetWithoutNextCkv(run_ckv);
           UPLL_LOG_DEBUG("MergeConflicts with %s", err_ckv->ToStr().c_str());
           delete start_ckv;
           return UPLL_RC_ERR_MERGE_CONFLICT;
         } else {
           /*
            * Update the Boundary ref count in import
            * and rename flag for the imported vlan map
            */
           imp_val->bdry_ref_count = run_val->bdry_ref_count;
           imp_val->valid[PFCDRV_IDX_BDRY_REF_COUNT] =
               run_val->valid[PFCDRV_IDX_BDRY_REF_COUNT];
           rename_flag = 0x00;
           GET_USER_DATA_FLAGS(imp_ckv, rename_flag);
           rename_flag |= BOUNDARY_VLANMAP_FLAG;
           SET_USER_DATA_FLAGS(imp_ckv, rename_flag);
           dbop.inoutop = kOpInOutFlag;
           result_code = UpdateConfigDB(imp_ckv, UPLL_DT_IMPORT, UNC_OP_UPDATE,
                               dmi, &dbop, TC_CONFIG_GLOBAL, vtn_id, MAINTBL);
           if (UPLL_RC_SUCCESS != result_code) {
             UPLL_LOG_DEBUG("UpdateConfigDB failed %d", result_code);
             delete start_ckv;
             delete imp_ckv;
             return result_code;
           }
         }
         /*If the running only valid import is invalid
          * then return merge conflicts
          */
       } else if (run_val->vm.valid[UPLL_IDX_VLAN_ID_VM] == UNC_VF_VALID) {
           delete imp_ckv;
           err_ckv->ResetWithoutNextCkv(run_ckv);
           UPLL_LOG_DEBUG("MergeConflicts with %s", err_ckv->ToStr().c_str());
           delete start_ckv;
           return UPLL_RC_ERR_MERGE_CONFLICT;
       } else {
          if (run_val->vm.valid[UPLL_IDX_VLAN_ID_VM] == UNC_VF_INVALID &&
             ((imp_val->vm.valid[UPLL_IDX_VLAN_ID_VM] == UNC_VF_INVALID)||
              imp_val->vm.valid[UPLL_IDX_VLAN_ID_VM] ==
              UNC_VF_VALID_NO_VALUE)) {
            // No Issue
          } else {
            err_ckv->ResetWithoutNextCkv(run_ckv);
            UPLL_LOG_DEBUG("MergeConflicts with %s", err_ckv->ToStr().c_str());
            delete start_ckv;
            return UPLL_RC_ERR_MERGE_CONFLICT;
          }
         // do nothing move next
       }

       /*
        * Both the side is not valid then  continue
        */
       delete imp_ckv;
    }
    run_ckv = run_ckv->get_next_cfg_key_val();
  }
  delete start_ckv;
  return UPLL_RC_SUCCESS;
}

upll_rc_t VlanMapMoMgr::AdaptValToDriver(ConfigKeyVal *ck_new,
                                         ConfigKeyVal *ck_old,
                                         unc_keytype_operation_t op,
                                         upll_keytype_datatype_t dt_type,
                                         unc_key_type_t keytype,
                                         DalDmlIntf *dmi,
                                         bool &not_send_to_drv,
                                         bool audit_update_phase) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  // If the attribute inside 'vlanmap_val' is not invalid,
  // then 'vlanmap_val' is set to UNC_VF_VALID, since only then,
  // UPLL_IDX_VLAN_ID_VM is checked for its value.
  pfcdrv_val_vlan_map *vlanmap_val =
    reinterpret_cast<pfcdrv_val_vlan_map_t *>(GetVal(ck_new));
  if (vlanmap_val) {
    vlanmap_val->valid[0] = UNC_VF_INVALID;
    if (vlanmap_val->vm.valid[UPLL_IDX_VLAN_ID_VM] != UNC_VF_INVALID)
      vlanmap_val->valid[0] = UNC_VF_VALID;
  }

  return result_code;
}

upll_rc_t VlanMapMoMgr::TxUpdateErrorHandler(ConfigKeyVal *req,
                                              ConfigKeyVal *ck_main,
                                              DalDmlIntf *dmi,
                                              upll_keytype_datatype_t dt_type,
                                              ConfigKeyVal **err_ckv,
                                              IpcResponse *ipc_resp) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *err = NULL;
  result_code = TranslateVlanmapError(&err, req, dmi, dt_type);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_ERROR("Failed to convert vlanmap error ckv");
    DELETE_IF_NOT_NULL(ck_main);
    DELETE_IF_NOT_NULL(req);
    DELETE_IF_NOT_NULL(ipc_resp->ckv_data);
    return result_code;
  }
  controller_domain ctrlr_dom;
  ctrlr_dom.ctrlr = NULL;
  ctrlr_dom.domain = NULL;
  GET_USER_DATA_CTRLR_DOMAIN(ck_main, ctrlr_dom);

  if (ctrlr_dom.ctrlr == NULL) {
    UPLL_LOG_ERROR("ctrlr_dom.ctrlr is NULL");
    DELETE_IF_NOT_NULL(ck_main);
    DELETE_IF_NOT_NULL(req);
    DELETE_IF_NOT_NULL(ipc_resp->ckv_data);
    DELETE_IF_NOT_NULL(err);
    return UPLL_RC_ERR_GENERIC;
  }

  uint8_t flags = 0;
  // VLANMAP flags determines whether the vlanMap is a user
  // configured vlanMap or a boundary vlanMap
  GET_USER_DATA_FLAGS(req, flags);
  if (flags & BOUNDARY_VLANMAP_FLAG) {
    SET_USER_DATA_CTRLR(err, ctrlr_dom.ctrlr);
    *err_ckv = err;
    DELETE_IF_NOT_NULL(ck_main);
    DELETE_IF_NOT_NULL(req);
    DELETE_IF_NOT_NULL(ipc_resp->ckv_data);
    return result_code;
  }

  DELETE_IF_NOT_NULL(err);

  // Get the UNC key for the renamed controller key.
  result_code = GetRenamedUncKey(ipc_resp->ckv_data, dt_type, dmi,
                                 ctrlr_dom.ctrlr);
  if (UPLL_RC_SUCCESS != result_code &&
      UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
    UPLL_LOG_ERROR("GetRenamedUncKey failed %d", result_code);
    DELETE_IF_NOT_NULL(ck_main);
    DELETE_IF_NOT_NULL(req);
    DELETE_IF_NOT_NULL(ipc_resp->ckv_data);
    return UPLL_RC_ERR_GENERIC;
  }
  // Convert Driver response to VTN service response
  result_code = AdaptValToVtnService(ipc_resp->ckv_data, ADAPT_ONE);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_ERROR("AdaptValToVtnService failed result_code %d",
                   result_code);
    DELETE_IF_NOT_NULL(ck_main);
    DELETE_IF_NOT_NULL(req);
    DELETE_IF_NOT_NULL(ipc_resp->ckv_data);
    return UPLL_RC_ERR_GENERIC;
  }
  SET_USER_DATA_CTRLR(ipc_resp->ckv_data, ctrlr_dom.ctrlr);
  *err_ckv = ipc_resp->ckv_data;
  DELETE_IF_NOT_NULL(ck_main);
  DELETE_IF_NOT_NULL(req);
  return result_code;
}

}  // namespace kt_momgr
}  // namespace upll
}  // namespace unc
