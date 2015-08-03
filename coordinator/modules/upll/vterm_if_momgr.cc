/*
 * Copyright (c) 2013-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <ctype.h>
#include <sstream>
#include "vterm_if_momgr.hh"
#include "vbr_if_momgr.hh"

#define NUM_KEY_MAIN_TBL_ 6

using unc::upll::ipc_util::IpcUtil;

namespace unc {
namespace upll {
namespace kt_momgr {

BindInfo VtermIfMoMgr::vterm_if_bind_info[] = {
  { uudst::vterminal_interface::kDbiVtnName, CFG_KEY, offsetof(
          key_vterm_if, vterm_key.vtn_key.vtn_name),
  uud::kDalChar, kMaxLenVtnName + 1},
  { uudst::vterminal_interface::kDbiVterminalName, CFG_KEY, offsetof(
          key_vterm_if, vterm_key.vterminal_name),
  uud::kDalChar, kMaxLenVnodeName + 1},
  { uudst::vterminal_interface::kDbiIfName, CFG_KEY, offsetof(key_vterm_if,
                                                          if_name),
  uud::kDalChar, kMaxLenInterfaceName + 1},
  { uudst::vterminal_interface::kDbiAdminStatus, CFG_VAL, offsetof(
          val_vterm_if, admin_status),
  uud::kDalUint8, 1 },
  { uudst::vterminal_interface::kDbiDesc, CFG_VAL, offsetof(val_vterm_if,
                                                        description),
  uud::kDalChar, kMaxLenDescription + 1},
  { uudst::vterminal_interface::kDbiLogicalPortId, CFG_VAL, offsetof(
          val_vterm_if, portmap.logical_port_id),
  uud::kDalChar, kMaxLenLogicalPortId + 1},
  { uudst::vterminal_interface::kDbiVlanId, CFG_VAL, offsetof(val_vterm_if,
                                                          portmap.vlan_id),
  uud::kDalUint16, 1 },
  { uudst::vterminal_interface::kDbiTagged, CFG_VAL, offsetof(val_vterm_if,
                                                          portmap.tagged),
  uud::kDalUint8, 1 },
  { uudst::vterminal_interface::kDbiOperStatus, ST_VAL, offsetof(
          val_db_vterm_if_st, vterm_if_val_st.oper_status),
  uud::kDalUint8, 1 },
  { uudst::vterminal_interface::kDbiDownCount, ST_VAL, offsetof(
          val_db_vterm_if_st, down_count),
  uud::kDalUint32, 1 },
  { uudst::vterminal_interface::kDbiCtrlrName, CK_VAL, offsetof(
          key_user_data_t, ctrlr_id), uud::kDalChar, kMaxLenCtrlrId + 1},
  { uudst::vterminal_interface::kDbiDomainId, CK_VAL, offsetof(
          key_user_data_t, domain_id), uud::kDalChar, kMaxLenDomainId + 1},
  { uudst::vterminal_interface::kDbiFlags, CK_VAL, offsetof(
          key_user_data_t, flags),
  uud::kDalUint8, 1 },
  { uudst::vterminal_interface::kDbiValidAdminStatus, CFG_DEF_VAL, offsetof(
          val_vterm_if, valid[UPLL_IDX_ADMIN_STATUS_VTERMI]),
  uud::kDalUint8, 1 },
  { uudst::vterminal_interface::kDbiValidDesc, CFG_META_VAL, offsetof(
          val_vterm_if, valid[UPLL_IDX_DESC_VTERMI]),
  uud::kDalUint8, 1 },
  { uudst::vterminal_interface::kDbiValidPortMap, CFG_META_VAL, offsetof(
          val_vterm_if, valid[UPLL_IDX_PM_VTERMI]),
  uud::kDalUint8, 1 },
  { uudst::vterminal_interface::kDbiValidLogicalPortId, CFG_META_VAL, offsetof(
          val_vterm_if, portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM]),
  uud::kDalUint8, 1 },
  { uudst::vterminal_interface::kDbiValidVlanid, CFG_META_VAL, offsetof(
          val_vterm_if, portmap.valid[UPLL_IDX_VLAN_ID_PM]),
  uud::kDalUint8, 1 },
  { uudst::vterminal_interface::kDbiValidTagged, CFG_META_VAL, offsetof(
          val_vterm_if, portmap.valid[UPLL_IDX_TAGGED_PM]),
  uud::kDalUint8, 1 },
  { uudst::vterminal_interface::kDbiValidOperStatus, ST_META_VAL, offsetof(
          val_db_vterm_if_st,
          vterm_if_val_st.valid[UPLL_IDX_OPER_STATUS_VTERMIS]),
  uud::kDalUint8, 1 },
  { uudst::vterminal_interface::kDbiCsAdminStatus, CS_VAL, offsetof(
          val_vterm_if, cs_attr[UPLL_IDX_ADMIN_STATUS_VTERMI]),
  uud::kDalUint8, 1 },
  { uudst::vterminal_interface::kDbiCsDesc, CS_VAL, offsetof(
          val_vterm_if, cs_attr[UPLL_IDX_DESC_VTERMI]),
  uud::kDalUint8, 1 },
  { uudst::vterminal_interface::kDbiCsPortMap, CS_VAL, offsetof(
          val_vterm_if, cs_attr[UPLL_IDX_PM_VTERMI]),
  uud::kDalUint8, 1 },
  { uudst::vterminal_interface::kDbiCsLogicalPortId, CS_VAL, offsetof(
          val_vterm_if, portmap.cs_attr[UPLL_IDX_LOGICAL_PORT_ID_PM]),
  uud::kDalUint8, 1 },
  { uudst::vterminal_interface::kDbiCsVlanid, CS_VAL, offsetof(
          val_vterm_if, portmap.cs_attr[UPLL_IDX_VLAN_ID_PM]),
  uud::kDalUint8, 1 },
  { uudst::vterminal_interface::kDbiCsTagged, CS_VAL, offsetof(
          val_vterm_if, portmap.cs_attr[UPLL_IDX_TAGGED_PM]),
  uud::kDalUint8, 1 },
  { uudst::vterminal_interface::kDbiCsRowstatus, CS_VAL, offsetof(
          val_vterm_if, cs_row_status),
  uud::kDalUint8, 1 } };

BindInfo VtermIfMoMgr::key_vterm_if_maintbl_bind_info[] = {
  { uudst::vterminal_interface::kDbiVtnName, CFG_MATCH_KEY, offsetof(
          key_vterm_if_t, vterm_key.vtn_key.vtn_name),
  uud::kDalChar, kMaxLenVtnName + 1 },
  { uudst::vterminal_interface::kDbiVterminalName, CFG_MATCH_KEY, offsetof(
          key_vterm_if_t, vterm_key.vterminal_name),
  uud::kDalChar, kMaxLenVnodeName + 1 },
  { uudst::vterminal_interface::kDbiIfName, CFG_MATCH_KEY, offsetof(
          key_vterm_if_t, if_name),
  uud::kDalChar, kMaxLenInterfaceName + 1 },
  { uudst::vterminal_interface::kDbiVtnName, CFG_INPUT_KEY, offsetof(
          key_rename_vnode_info_t, new_unc_vtn_name),
  uud::kDalChar, kMaxLenVtnName + 1 },
  { uudst::vterminal_interface::kDbiVterminalName, CFG_INPUT_KEY, offsetof(
          key_rename_vnode_info_t, new_unc_vnode_name),
  uud::kDalChar, kMaxLenVnodeName + 1 },
  { uudst::vterminal_interface::kDbiFlags, CK_VAL, offsetof(
          key_user_data_t, flags),
  uud::kDalUint8, 1 } };

unc_key_type_t VtermIfMoMgr::vterm_if_child[] = { UNC_KT_VTERMIF_FLOWFILTER,
                                                  UNC_KT_VTERMIF_POLICINGMAP };
VtermIfMoMgr::VtermIfMoMgr() {
  UPLL_FUNC_TRACE;
  ntable         = MAX_MOMGR_TBLS;
  table          = new Table *[ntable]();
  table[MAINTBL] = new Table(uudst::kDbiVtermIfTbl, UNC_KT_VTERM_IF,
                             vterm_if_bind_info, IpctSt::kIpcStKeyVtermIf,
                             IpctSt::kIpcStValVtermIf,
                             uudst::vterminal_interface::kDbiVtermIfNumCols);
  table[CTRLRTBL]  = NULL;
  table[RENAMETBL] = NULL;
  table[CONVERTTBL] = NULL;

  nchild           = sizeof(vterm_if_child) / sizeof(*vterm_if_child);
  child            = vterm_if_child;
}

/*
 * Based on the key type the bind info will pass
 **/
bool VtermIfMoMgr::GetRenameKeyBindInfo(unc_key_type_t key_type,
                                        BindInfo *&binfo,
                                        int &nattr, MoMgrTables tbl) {
  /* Main Table or rename table only update */
  if (MAINTBL == tbl) {
    nattr = NUM_KEY_MAIN_TBL_;
    binfo = key_vterm_if_maintbl_bind_info;
  } else {
    UPLL_LOG_DEBUG("Invalid Table ");
    return PFC_FALSE;
  }
  return PFC_TRUE;
}

upll_rc_t VtermIfMoMgr::UpdateMo(IpcReqRespHeader *req,
                                 ConfigKeyVal *ikey,
                                 DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;

  upll_rc_t result_code = UPLL_RC_SUCCESS;

  /* Performs vterm_if update request */
  result_code = MoMgrImpl::UpdateMo(req, ikey, dmi);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_ERROR("Failed to perform VTERM_IF update request ");
    return result_code;
  }

  MoMgrImpl      *ff_mgr           = NULL;
  MoMgrImpl      *pm_mgr           = NULL;
  bool           port_map_status   = false;
  val_vterm_if_t *vtermif_val      = NULL;

  ff_mgr = reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>(
          GetMoManager(UNC_KT_VTERMIF_FLOWFILTER)));
  if (!ff_mgr) {
    UPLL_LOG_DEBUG("Invalid Instance");
    return UPLL_RC_ERR_GENERIC;
  }

  pm_mgr = reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>(
          GetMoManager(UNC_KT_VTERMIF_POLICINGMAP)));
  if (!pm_mgr) {
    UPLL_LOG_DEBUG("Invalid Instance");
    return UPLL_RC_ERR_GENERIC;
  }
  vtermif_val =  reinterpret_cast<val_vterm_if_t *>(GetVal(ikey));
  if (!vtermif_val) {
    UPLL_LOG_DEBUG("Val Vterm if is Null");
    return UPLL_RC_ERR_GENERIC;
  }
  TcConfigMode config_mode = TC_CONFIG_INVALID;
  std::string vtn_name = "";
  result_code = GetConfigModeInfo(req, config_mode, vtn_name);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetConfigMode failed");
    return result_code;
  }

  /* If the valid[UPLL_IDX_PM_VTERMI] is UNC_VF_VALID measns
   * inform vterm_if_policingmap and vterm_if_flowfilter to
   * PortMapConfigured else inform PortMapNotConfigured*/
  port_map_status = (vtermif_val->valid[UPLL_IDX_PM_VTERMI] == UNC_VF_VALID)?
                              true:false;
  if (port_map_status == true) {
    UPLL_LOG_DEBUG("Portmapstatus-true");
    result_code = ff_mgr->SetPortmapConfiguration(ikey, req->datatype, dmi,
                                                  kPortMapConfigured,
                                                  config_mode, vtn_name);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("SetPortMapConfiguration Failure %d", result_code);
      return result_code;
    }

    result_code = pm_mgr->SetPortmapConfiguration(ikey, req->datatype, dmi,
                                                  kPortMapConfigured,
                                                  config_mode, vtn_name);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("SetPortMapConfiguration Failure %d", result_code);
      return result_code;
    }
  } else {
    UPLL_LOG_DEBUG("Portmapstatus-flase");
    result_code = ff_mgr->SetPortmapConfiguration(ikey, req->datatype, dmi,
                                                  kVlinkPortMapNotConfigured,
                                                  config_mode, vtn_name);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("SePortMapConfiguration Failure %d", result_code);
      return result_code;
    }

    result_code = pm_mgr->SetPortmapConfiguration(ikey, req->datatype, dmi,
                                                  kVlinkPortMapNotConfigured,
                                                  config_mode, vtn_name);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("SetVlinkPortMapConfiguration Failure %d", result_code);
      return result_code;
    }
  }

  return result_code;
}

upll_rc_t VtermIfMoMgr::GetVtermIfValfromDB(ConfigKeyVal *ikey,
                                            ConfigKeyVal *&ck_vterm_if,
                                            upll_keytype_datatype_t dt_type,
                                            DalDmlIntf *dmi) {
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  result_code = GetChildConfigKey(ck_vterm_if, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Get VtermIfkey failed");
    return result_code;
  }

  DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutFlag };
  result_code = ReadConfigDB(ck_vterm_if, dt_type, UNC_OP_READ, dbop, dmi,
                             MAINTBL);
  return result_code;
}

bool VtermIfMoMgr::IsValidKey(void *key, uint64_t index,
                              MoMgrTables tbl) {
  UPLL_FUNC_TRACE;

  key_vterm_if *if_key = reinterpret_cast<key_vterm_if *>(key);
  upll_rc_t    ret_val = UPLL_RC_SUCCESS;

  switch (index) {
    case uudst::vterminal_interface::kDbiVtnName:
      ret_val = ValidateKey(
          reinterpret_cast<char *>(if_key->vterm_key.vtn_key.vtn_name),
          kMinLenVtnName, kMaxLenVtnName);
      if (ret_val != UPLL_RC_SUCCESS) {
        UPLL_LOG_TRACE("VTN Name is not valid(%d)", ret_val);
        return false;
      }
      break;
    case uudst::vterminal_interface::kDbiVterminalName:
      ret_val = ValidateKey(
          reinterpret_cast<char *>(if_key->vterm_key.vterminal_name),
          kMinLenVnodeName, kMaxLenVnodeName);
      if (ret_val != UPLL_RC_SUCCESS) {
        UPLL_LOG_TRACE("VTERMINAL Name is not valid(%d)", ret_val);
        return false;
      }
      break;
    case uudst::vterminal_interface::kDbiIfName:
      ret_val = ValidateKey(reinterpret_cast<char *>(if_key->if_name),
                            kMinLenInterfaceName, kMaxLenInterfaceName);
      if (ret_val != UPLL_RC_SUCCESS) {
        UPLL_LOG_TRACE("VTERMINAL IF Name is not valid(%d)", ret_val);
        return false;
      }
      break;
    default:
      UPLL_LOG_INFO("Wrong Index");
      break;
  }

  return true;
}

upll_rc_t VtermIfMoMgr::GetChildConfigKey(ConfigKeyVal *&okey,
                                          ConfigKeyVal *parent_key) {
  UPLL_FUNC_TRACE;

  key_vterm_if   *vterm_key_if  = NULL;
  void           *pkey          = NULL;
  upll_rc_t      result_code    = UPLL_RC_SUCCESS;

  if (parent_key == NULL) {
    vterm_key_if = reinterpret_cast<key_vterm_if_t *>
        (ConfigKeyVal::Malloc(sizeof(key_vterm_if_t)));

    if (okey) delete okey;
    okey = new ConfigKeyVal(UNC_KT_VTERM_IF, IpctSt::kIpcStKeyVtermIf,
                            vterm_key_if, NULL);
    return UPLL_RC_SUCCESS;
  }
  unc_key_type_t keytype        = parent_key->get_key_type();

  pkey = parent_key->get_key();
  if (!pkey) return UPLL_RC_ERR_GENERIC;

  if (okey) {
    if (okey->get_key_type() != UNC_KT_VTERM_IF)
      return UPLL_RC_ERR_GENERIC;

    vterm_key_if = reinterpret_cast<key_vterm_if_t *>(okey->get_key());
    if (!vterm_key_if)
      return UPLL_RC_ERR_GENERIC;
  } else {
    vterm_key_if = reinterpret_cast<key_vterm_if *>
        (ConfigKeyVal::Malloc(sizeof(key_vterm_if)));
  }

  switch (keytype) {
    case UNC_KT_VTN:
      uuu::upll_strncpy(vterm_key_if->vterm_key.vtn_key.vtn_name,
                        reinterpret_cast<key_vtn_t *>(pkey)->vtn_name,
                        (kMaxLenVtnName + 1));
      break;
    case UNC_KT_VTERMINAL:
      uuu::upll_strncpy(vterm_key_if->vterm_key.vtn_key.vtn_name,
                        reinterpret_cast<key_vterm_t *>(pkey)->vtn_key.vtn_name,
                        (kMaxLenVtnName + 1));
      uuu::upll_strncpy(vterm_key_if->vterm_key.vterminal_name,
                        reinterpret_cast<key_vterm_t *>(pkey)->vterminal_name,
                        (kMaxLenVnodeName + 1));
      break;
    case UNC_KT_VTERM_IF:
      uuu::upll_strncpy(vterm_key_if->vterm_key.vtn_key.vtn_name,
          reinterpret_cast<key_vterm_if_t *>(pkey)->vterm_key.vtn_key.vtn_name,
          (kMaxLenVtnName + 1));
      uuu::upll_strncpy(vterm_key_if->vterm_key.vterminal_name,
          reinterpret_cast<key_vterm_if_t *>(pkey)->vterm_key.vterminal_name,
          (kMaxLenVnodeName + 1));
      uuu::upll_strncpy(vterm_key_if->if_name,
          reinterpret_cast<key_vterm_if_t *>(pkey)->if_name,
          (kMaxLenInterfaceName + 1));
      break;
    default:
      break;
  }
  if (!okey)
    okey = new ConfigKeyVal(UNC_KT_VTERM_IF, IpctSt::kIpcStKeyVtermIf,
                            vterm_key_if, NULL);
  else if (okey->get_key() != vterm_key_if)
    okey->SetKey(IpctSt::kIpcStKeyVtermIf, vterm_key_if);

  if (okey == NULL) {
    free(vterm_key_if);
    result_code = UPLL_RC_ERR_GENERIC;
  } else {
    SET_USER_DATA(okey, parent_key);
  }

  return result_code;
}


upll_rc_t VtermIfMoMgr::GetParentConfigKey(ConfigKeyVal *&okey,
                                           ConfigKeyVal *ikey) {
  UPLL_FUNC_TRACE;

  if (!ikey)
    return UPLL_RC_ERR_GENERIC;

  unc_key_type_t   key_type    = ikey->get_key_type();
  upll_rc_t        result_code = UPLL_RC_SUCCESS;
  key_vterm_if_t   *pkey       = NULL;
  key_vterm_t      *vterm_key  = NULL;

  if (key_type != UNC_KT_VTERM_IF) return UPLL_RC_ERR_GENERIC;

  pkey = reinterpret_cast<key_vterm_if_t *>(ikey->get_key());
  if (!pkey) return UPLL_RC_ERR_GENERIC;

  vterm_key = reinterpret_cast<key_vterm_t *>(
              ConfigKeyVal::Malloc(sizeof(key_vterm_t)));
  uuu::upll_strncpy(vterm_key->vtn_key.vtn_name,
                    pkey->vterm_key.vtn_key.vtn_name, (kMaxLenVtnName+1));
  uuu::upll_strncpy(vterm_key->vterminal_name, pkey->vterm_key.vterminal_name,
                   (kMaxLenVnodeName+1));

  if (okey) delete okey;
  okey = new ConfigKeyVal(UNC_KT_VTERMINAL, IpctSt::kIpcStKeyVterminal,
                          vterm_key, NULL);
  if (okey == NULL) {
    free(vterm_key);
    result_code = UPLL_RC_ERR_GENERIC;
  } else {
    SET_USER_DATA(okey, ikey);
  }

  return result_code;
}

upll_rc_t VtermIfMoMgr::AllocVal(ConfigVal *&ck_val,
                                 upll_keytype_datatype_t dt_type,
                                 MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  if (ck_val != NULL) return UPLL_RC_ERR_GENERIC;

  if (tbl == MAINTBL) {
    void *val_vterm, *val_vterm_st = NULL;
    val_vterm = reinterpret_cast<void *>(ConfigKeyVal::Malloc(
             sizeof(val_vterm_if_t)));
    ck_val = new ConfigVal(IpctSt::kIpcStValVtermIf, val_vterm);
    if (dt_type == UPLL_DT_STATE) {
      val_vterm_st = reinterpret_cast<void *>(ConfigKeyVal::Malloc(
              sizeof(val_db_vterm_if_st_t)));
      ConfigVal *ck_nxtval = new ConfigVal(IpctSt::kIpcStValVtermIfSt,
                                           val_vterm_st);
      ck_val->AppendCfgVal(ck_nxtval);
    }
  }
  return UPLL_RC_SUCCESS;
}


upll_rc_t VtermIfMoMgr::DupConfigKeyVal(ConfigKeyVal *&okey,
                                        ConfigKeyVal *&req,
                                        MoMgrTables tbl) {
  UPLL_FUNC_TRACE;

  ConfigVal          *tmp             = NULL;
  val_vterm_if       *vterm_val_if    = NULL;
  val_db_vterm_if_st *val_vterm_if_st = NULL;
  ConfigVal          *cfv_vtermif     = NULL;
  ConfigVal          *cfv_vtermif_st  = NULL;

  if (!req || okey || (req->get_key_type() != UNC_KT_VTERM_IF))
    return UPLL_RC_ERR_GENERIC;

  if (tbl == MAINTBL) {
    tmp = (req)->get_cfg_val();

    if (tmp) {
      /* creates dup ConfigVal val_vterm_if value structure */
      val_vterm_if *ival = reinterpret_cast<val_vterm_if *>(GetVal(req));
      if (!ival) return UPLL_RC_ERR_GENERIC;

      if ((req->get_cfg_val())->get_st_num() != IpctSt::kIpcStValVtermIf)
        return UPLL_RC_ERR_GENERIC;
      vterm_val_if = reinterpret_cast<val_vterm_if *>(
            ConfigKeyVal::Malloc(sizeof(val_vterm_if)));
      memcpy(vterm_val_if, ival, sizeof(val_vterm_if));
      cfv_vtermif = new ConfigVal(IpctSt::kIpcStValVtermIf, vterm_val_if);
      tmp = tmp->get_next_cfg_val();
    }

    if (tmp) {
      /* Creates dup ConfigVal for val_vtermif_st structure */
      val_db_vterm_if_st *ival =
          reinterpret_cast<val_db_vterm_if_st *>(tmp->get_val());
      if (ival == NULL) {
        DELETE_IF_NOT_NULL(cfv_vtermif);
        return UPLL_RC_ERR_GENERIC;
      }

      val_vterm_if_st = reinterpret_cast<val_db_vterm_if_st *>
          (ConfigKeyVal::Malloc(sizeof(val_db_vterm_if_st)));
      memcpy(val_vterm_if_st, ival, sizeof(val_db_vterm_if_st));
      cfv_vtermif_st = new ConfigVal(IpctSt::kIpcStValVtermIfSt,
                                     val_vterm_if_st);
      cfv_vtermif->AppendCfgVal(cfv_vtermif_st);
    }
  }

  /* Creates dup vterm_if key */
  key_vterm_if_t *ikey = reinterpret_cast<key_vterm_if *>(req->get_key());
  if (!ikey) {
    DELETE_IF_NOT_NULL(cfv_vtermif);
    return UPLL_RC_ERR_GENERIC;
  }

  key_vterm_if_t *vterm_if_key = reinterpret_cast<key_vterm_if *>(
                               ConfigKeyVal::Malloc(sizeof(key_vterm_if)));
  memcpy(vterm_if_key, ikey, sizeof(key_vterm_if));
  okey = new ConfigKeyVal(UNC_KT_VTERM_IF, IpctSt::kIpcStKeyVtermIf,
                          vterm_if_key, cfv_vtermif);
  if (okey == NULL) {
    DELETE_IF_NOT_NULL(cfv_vtermif);
    FREE_IF_NOT_NULL(vterm_if_key);
    return UPLL_RC_ERR_GENERIC;
  } else {
    SET_USER_DATA(okey, req);
  }

  return UPLL_RC_SUCCESS;
}

upll_rc_t VtermIfMoMgr::UpdateConfigStatus(ConfigKeyVal *ikey,
                                           unc_keytype_operation_t op,
                                           uint32_t driver_result,
                                           ConfigKeyVal *upd_key,
                                           DalDmlIntf *dmi,
                                           ConfigKeyVal *ctrlr_key) {
  UPLL_FUNC_TRACE;

  val_vterm_if *vtermif_val = reinterpret_cast<val_vterm_if_t *>(GetVal(ikey));
  if (vtermif_val == NULL) return UPLL_RC_ERR_GENERIC;

  unc_keytype_configstatus_t cs_status =
      (driver_result == UPLL_RC_SUCCESS) ? UNC_CS_APPLIED : UNC_CS_NOT_APPLIED;

  val_vterm_if_t *vtermif_running = reinterpret_cast<val_vterm_if_t *>
                                             (GetVal(upd_key));
  bool propagate = false;
  bool port_map_change = false;
  switch (op) {
    case UNC_OP_UPDATE:
      {
        if (vtermif_val->valid[UPLL_IDX_PM_VTERMI] !=
            vtermif_running->valid[UPLL_IDX_PM_VTERMI]) {
          port_map_change = true;
          propagate = true;
        }
        void* val = reinterpret_cast<void *>(vtermif_val);
        CompareValidValue(val, GetVal(upd_key), true);
        if (vtermif_val->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM]
            != UNC_VF_INVALID) {
          propagate = true;
          port_map_change = true;
        }
      }
    case UNC_OP_CREATE:
      {
        if (op == UNC_OP_CREATE) {
          vtermif_val->cs_row_status = cs_status;
          port_map_change = true;
        }
        if (port_map_change) {
          val_db_vterm_if_st *vtermif_st =
            reinterpret_cast<val_db_vterm_if_st *>(
                ConfigKeyVal::Malloc(sizeof(val_db_vterm_if_st)));
          ikey->AppendCfgVal(IpctSt::kIpcStValVtermIfSt, vtermif_st);
          val_db_vterm_if_st* vnif_st = reinterpret_cast<val_db_vterm_if_st  *>
                                           (GetStateVal(ikey));
          vnif_st->vterm_if_val_st.valid[UPLL_IDX_OPER_STATUS_VTERMIS] =
              UNC_VF_VALID;
          if (op == UNC_OP_CREATE) {
            if (driver_result == UPLL_RC_ERR_CTR_DISCONNECTED) {
              vnif_st->vterm_if_val_st.oper_status = UPLL_OPER_STATUS_UNKNOWN;
              vnif_st->down_count = PORT_UNKNOWN;
            } else {
              vnif_st->vterm_if_val_st.oper_status = UPLL_OPER_STATUS_UNINIT;
              vnif_st->down_count = 0;
            }
          } else {
            val_db_vterm_if_st *run_vtermifst =
                reinterpret_cast<val_db_vterm_if_st *> (GetStateVal(upd_key));
            vnif_st->vterm_if_val_st.oper_status =
                run_vtermifst->vterm_if_val_st.oper_status;
            vnif_st->down_count = run_vtermifst->down_count;
          }
        }
      }
      break;
    default:
      return UPLL_RC_ERR_GENERIC;
  }

  if (UNC_OP_UPDATE == op) {
    vtermif_val->cs_row_status = vtermif_running->cs_row_status;
  }

  /* Sets individual value structure cs attributes */
  for (unsigned int loop = 0; loop <
       sizeof(vtermif_val->valid) / sizeof(vtermif_val->valid[0]); ++loop) {
    if ((UNC_VF_VALID == (uint8_t) vtermif_val->valid[loop])
        || (UNC_VF_VALID_NO_VALUE == (uint8_t) vtermif_val->valid[loop])) {
      if (loop == UPLL_IDX_DESC_VTERMI)
        vtermif_val->cs_attr[loop] = UNC_CS_APPLIED;
      else
        vtermif_val->cs_attr[loop] = cs_status;
    } else if ((UNC_VF_INVALID == vtermif_val->valid[loop]) &&
               (UNC_OP_CREATE == op)) {
      vtermif_val->cs_attr[loop] = UNC_CS_APPLIED;
    } else if ((UNC_VF_INVALID == vtermif_val->valid[loop]) &&
               (UNC_OP_UPDATE == op)) {
      vtermif_val->cs_attr[loop] = vtermif_running->cs_attr[loop];
    }
  }

  val_port_map *pm =  &vtermif_val->portmap;
  for (unsigned int loop = 0; loop < sizeof(pm->valid) / sizeof(pm->valid[0]);
       ++loop) {
    if ((UNC_VF_VALID == (uint8_t) pm->valid[loop]) ||
        (UNC_VF_VALID_NO_VALUE == (uint8_t) pm->valid[loop])) {
      pm->cs_attr[loop] = cs_status;
    } else if ((UNC_VF_INVALID == pm->valid[loop]) &&
               (UNC_OP_UPDATE == op)) {
      pm->cs_attr[loop] = vtermif_running->portmap.cs_attr[loop];
    } else if ((UNC_VF_INVALID == pm->valid[loop]) &&
               (UNC_OP_CREATE == op)) {
      pm->cs_attr[loop] = UNC_CS_APPLIED;
    }
  }
  return (SetInterfaceOperStatus(ikey, dmi, op, propagate, driver_result));
}

upll_rc_t VtermIfMoMgr::UpdateAuditConfigStatus(
                                 unc_keytype_configstatus_t cs_status,
                                 uuc::UpdateCtrlrPhase      phase,
                                 ConfigKeyVal               *&ckv_running,
                                 DalDmlIntf                 *dmi) {
  UPLL_FUNC_TRACE;

  upll_rc_t      result_code = UPLL_RC_SUCCESS;
  val_vterm_if_t *val        = NULL;

  val = (ckv_running != NULL) ? reinterpret_cast<val_vterm_if_t *>(
      GetVal(ckv_running)) : NULL;
  if (NULL == val) {
    return UPLL_RC_ERR_GENERIC;
  }

  if (uuc::kUpllUcpCreate == phase)
     val->cs_row_status = cs_status;

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

  for (unsigned int loop = 0;
       loop < sizeof(val->portmap.valid) / sizeof(uint8_t); ++loop) {
    if ((cs_status == UNC_CS_INVALID &&
                      UNC_VF_VALID == val->portmap.valid[loop])
        || cs_status == UNC_CS_APPLIED)
      val->portmap.cs_attr[loop] = cs_status;
  }
  return result_code;
}


upll_rc_t VtermIfMoMgr::ValidateMessage(IpcReqRespHeader *req,
                                        ConfigKeyVal *ikey) {
  UPLL_FUNC_TRACE;

  if (!ikey || !req || !(ikey->get_key())) {
    UPLL_LOG_DEBUG("ConfigKeyVal / IpcReqRespHeader is NUll");
    return UPLL_RC_ERR_GENERIC;
  }

  upll_keytype_datatype_t dt_type       = req->datatype;
  unc_keytype_operation_t operation     = req->operation;
  unc_keytype_option1_t   option1       = req->option1;
  unc_keytype_option2_t   option2       = req->option2;
  upll_rc_t               ret_val       = UPLL_RC_SUCCESS;
  key_vterm_if_t          *vterm_if_key = NULL;
  val_vterm_if_t          *vterm_if_val = NULL;

  /* Validates ikey struct num */
  if (ikey->get_st_num() != IpctSt::kIpcStKeyVtermIf) {
    UPLL_LOG_DEBUG("Invalid key structure received. received struct - %d",
                   ikey->get_st_num());
    return UPLL_RC_ERR_BAD_REQUEST;
  }

  /* Validates keytype is UNC_KT_VTERM_IF or not */
  if (UNC_KT_VTERM_IF != ikey->get_key_type()) {
    UPLL_LOG_DEBUG("Invalid Keytype received. received keytype - %d",
                   ikey->get_key_type());
    return UPLL_RC_ERR_BAD_REQUEST;
  }

  vterm_if_key = reinterpret_cast<key_vterm_if_t *>(ikey->get_key());

  /* Validates vterm_if key structure*/
  ret_val = ValidateVtermIfKey(vterm_if_key, operation);
  if (ret_val != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Validation failure for key_vterm_if struct");
    return UPLL_RC_ERR_CFG_SYNTAX;
  }

  /* Validates ikey cfg val struct num */
  if ((ikey->get_cfg_val()) &&
      ((ikey->get_cfg_val())->get_st_num() != IpctSt::kIpcStValVtermIf)) {
    UPLL_LOG_DEBUG("Invalid val structure received.received struct - %d",
                   ikey->get_cfg_val()->get_st_num());
    return UPLL_RC_ERR_BAD_REQUEST;
  }

  if (ikey->get_cfg_val()) {
    vterm_if_val = reinterpret_cast<val_vterm_if_t *>(GetVal(ikey));
  }

  /* Validates datatype and vterm value structure for create operation */
  if ((operation == UNC_OP_CREATE) && ((dt_type == UPLL_DT_CANDIDATE)
                                       || (dt_type == UPLL_DT_IMPORT))) {
    if (vterm_if_val == NULL) {
      UPLL_LOG_DEBUG("Val structure is an optional for CREATE operation");
      return UPLL_RC_SUCCESS;
    }

    /* Validates vterm_if value structure*/
    ret_val = ValidateVtermIfValue(vterm_if_val, operation);
    if (ret_val != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG(" Val struct Validation failure for CREATE operation");
      return UPLL_RC_ERR_CFG_SYNTAX;
    }

    return UPLL_RC_SUCCESS;
  }

  /* Validates datatype and value structure for update operation */
  if ((operation == UNC_OP_UPDATE) && (dt_type == UPLL_DT_CANDIDATE)) {
    if (vterm_if_val == NULL) {
      UPLL_LOG_DEBUG("Val struct Validation is Mandatory for UPDATE operation");
      return UPLL_RC_ERR_BAD_REQUEST;
    }

    /* Validates vterm_if value structure*/
    ret_val = ValidateVtermIfValue(vterm_if_val, operation);
    if (ret_val != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Val struct Validation failure for UPDATE operation");
      return UPLL_RC_ERR_CFG_SYNTAX;
    }

    return UPLL_RC_SUCCESS;
  }

  /* Validates datatype for UNC_OP_DELETE/UNC_OP_READ_NEXT/ UNC_OP_READ_BULK
   * operation */
  if ((operation == UNC_OP_DELETE) && (dt_type == UPLL_DT_CANDIDATE)) {
    UPLL_LOG_DEBUG("value struct is none for this operation - %d", operation);
    return UPLL_RC_SUCCESS;
  }

  /* Validates datatype and value structure for UNC_OP_READ/
   * UNC_OP_READ_SIBLING/UNC_OP_READ_SIBLING_BEGIN operation */
  if ((operation == UNC_OP_READ || operation == UNC_OP_READ_SIBLING ||
       operation == UNC_OP_READ_SIBLING_BEGIN ||
       operation == UNC_OP_READ_SIBLING_COUNT) &&
      (dt_type == UPLL_DT_CANDIDATE || dt_type == UPLL_DT_RUNNING ||
       dt_type == UPLL_DT_STARTUP || dt_type == UPLL_DT_STATE)) {
    if (option1 != UNC_OPT1_NORMAL) {
      UPLL_LOG_DEBUG("Error option1 is not NORMAL");
      return UPLL_RC_ERR_INVALID_OPTION1;
    }

    if (option2 != UNC_OPT2_NONE) {
      UPLL_LOG_DEBUG("Error option1 is not NONE");
      return UPLL_RC_ERR_INVALID_OPTION2;
    }

    if (vterm_if_val == NULL) {
      UPLL_LOG_DEBUG("Val struct is optional for READ operation");
      return UPLL_RC_SUCCESS;
    }

    /* Validates vterm_if value structure*/
    ret_val = ValidateVtermIfValue(vterm_if_val, operation);
    if (ret_val != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Val struct Validation failure for READ operation");
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
    return UPLL_RC_SUCCESS;
  }
  UPLL_LOG_DEBUG("Error Unsupported datatype (%d) or operation - (%d)",
                 dt_type, operation);

  return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT;
}

upll_rc_t VtermIfMoMgr::ValidateVtermIfKey(key_vterm_if_t *vterm_if_key,
                                       unc_keytype_operation_t operation) {
  UPLL_FUNC_TRACE;

  upll_rc_t ret_val = UPLL_RC_SUCCESS;

  /* Validates vtn name*/
  ret_val = ValidateKey(
             reinterpret_cast<char *>(vterm_if_key->vterm_key.vtn_key.vtn_name),
             kMinLenVtnName, kMaxLenVtnName);
  if (ret_val != UPLL_RC_SUCCESS) {
    UPLL_LOG_INFO("Vtn Name syntax check failed. Received VTN Name - %s",
                  vterm_if_key->vterm_key.vtn_key.vtn_name);
    return UPLL_RC_ERR_CFG_SYNTAX;
  }

  /* Validates vterminal name */
  ret_val = ValidateKey(
              reinterpret_cast<char *>(vterm_if_key->vterm_key.vterminal_name),
              kMinLenVnodeName, kMaxLenVnodeName);
  if (ret_val != UPLL_RC_SUCCESS) {
    UPLL_LOG_INFO("Vterminal Name syntax check failed."
                  "Received VTERMINAL Name -%s",
                  vterm_if_key->vterm_key.vterminal_name);
    return UPLL_RC_ERR_CFG_SYNTAX;
  }

  /* Validates vterminal interface name */
  if (operation != UNC_OP_READ_SIBLING_COUNT &&
      operation != UNC_OP_READ_SIBLING_BEGIN) {
    ret_val = ValidateKey(reinterpret_cast<char *>(vterm_if_key->if_name),
                          kMinLenInterfaceName, kMaxLenInterfaceName);
    if (ret_val != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Interface Name syntax check failed."
                     "Received if_name - %s",
                     vterm_if_key->if_name);
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  } else {
    UPLL_LOG_TRACE("Operation is %d", operation);
    StringReset(vterm_if_key->if_name);
  }

  return UPLL_RC_SUCCESS;
}

upll_rc_t VtermIfMoMgr::ValidateVtermIfValue(val_vterm_if_t *vterm_if_val,
                                           unc_keytype_operation_t operation) {
  UPLL_FUNC_TRACE;

  /* Validates description attribute */
  if (vterm_if_val->valid[UPLL_IDX_DESC_VTERMI] == UNC_VF_VALID) {
    if (!ValidateDesc(vterm_if_val->description,
                      kMinLenDescription, kMaxLenDescription)) {
      UPLL_LOG_DEBUG("Description syntax check failed."
                     "Received description - %s",
                     vterm_if_val->description);
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  } else if (vterm_if_val->valid[UPLL_IDX_DESC_VTERMI] == UNC_VF_VALID_NO_VALUE
      && (operation == UNC_OP_UPDATE || operation == UNC_OP_CREATE)) {
    uuu::upll_strncpy(vterm_if_val->description, " ", kMaxLenDescription+1);
  }

  /* Validates admin status attribute */
  if (vterm_if_val->valid[UPLL_IDX_ADMIN_STATUS_VTERMI] == UNC_VF_VALID) {
    if (!ValidateNumericRange(vterm_if_val->admin_status,
                              (uint8_t) UPLL_ADMIN_ENABLE,
                              (uint8_t) UPLL_ADMIN_DISABLE, true, true)) {
      UPLL_LOG_DEBUG("Admin status range check failed. "
                     "Received admin status -%d", vterm_if_val->admin_status);
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  } else if (vterm_if_val->valid[UPLL_IDX_ADMIN_STATUS_VTERMI] ==
             UNC_VF_VALID_NO_VALUE && (operation == UNC_OP_CREATE
                                       || operation == UNC_OP_UPDATE)) {
    vterm_if_val->admin_status = UPLL_ADMIN_ENABLE;
  } else if ((vterm_if_val->valid[UPLL_IDX_ADMIN_STATUS_VTERMI] ==
              UNC_VF_INVALID)
             && (operation == UNC_OP_CREATE)) {
    vterm_if_val->admin_status = UPLL_ADMIN_ENABLE;
    vterm_if_val->valid[UPLL_IDX_ADMIN_STATUS_VTERMI] = UNC_VF_VALID_NO_VALUE;
  }

  /* Validates portmap */
  if (vterm_if_val->valid[UPLL_IDX_PM_VTERMI] == UNC_VF_VALID) {
    /* Validats logical port id in portmap structure attribute */
    if (vterm_if_val->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM]
        == UNC_VF_VALID) {
      if (!ValidateLogicalPortId(
              reinterpret_cast<char *>(vterm_if_val->portmap.logical_port_id),
              kMinLenLogicalPortId, kMaxLenLogicalPortId)) {
        UPLL_LOG_DEBUG("Logical Port id syntax check failed."
                       "Received Logical Port Id - %s",
                       vterm_if_val->portmap.logical_port_id);
        return UPLL_RC_ERR_CFG_SYNTAX;
      }

      if (toupper(vterm_if_val->portmap.logical_port_id[0]) == 'S'
          && toupper(vterm_if_val->portmap.logical_port_id[1]) == 'W') {
        UPLL_LOG_DEBUG("Invalid logical_port_id - %s",
                       vterm_if_val->portmap.logical_port_id);
        return UPLL_RC_ERR_CFG_SYNTAX;
      }
    } else if (vterm_if_val->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] ==
        UNC_VF_VALID_NO_VALUE
        && (operation == UNC_OP_CREATE || operation == UNC_OP_UPDATE)) {
      uuu::upll_strncpy(vterm_if_val->portmap.logical_port_id, " ",
                        kMaxLenLogicalPortId+1);
      // Delete all dependent attributes.
      vterm_if_val->portmap.vlan_id = 0;
      vterm_if_val->portmap.tagged = 0;
      vterm_if_val->portmap.valid[UPLL_IDX_VLAN_ID_PM] = UNC_VF_VALID_NO_VALUE;
      vterm_if_val->portmap.valid[UPLL_IDX_TAGGED_PM] = UNC_VF_VALID_NO_VALUE;
    } else if ((vterm_if_val->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] ==
       UNC_VF_INVALID) && (operation == UNC_OP_CREATE)) {
        vterm_if_val->portmap.valid[UPLL_IDX_VLAN_ID_PM] = UNC_VF_INVALID;
        vterm_if_val->portmap.valid[UPLL_IDX_TAGGED_PM] = UNC_VF_INVALID;
    }
    /* Validates vlan-id in portmap structure attribute */
    if (vterm_if_val->portmap.valid[UPLL_IDX_VLAN_ID_PM] == UNC_VF_VALID) {
      if ((vterm_if_val->portmap.vlan_id != 0xFFFF) &&
          !ValidateNumericRange(vterm_if_val->portmap.vlan_id,
                                (uint16_t) kMinVlanId, (uint16_t) kMaxVlanId,
                                true, true)) {
        UPLL_LOG_DEBUG("Vlan Id Number check failed. Received vlan_id - %d",
                       vterm_if_val->portmap.vlan_id);
        return UPLL_RC_ERR_CFG_SYNTAX;
      }
    } else if (vterm_if_val->portmap.valid[UPLL_IDX_VLAN_ID_PM] ==
        UNC_VF_VALID_NO_VALUE
        && (operation == UNC_OP_CREATE || operation == UNC_OP_UPDATE)) {
      /* If VLAN_ID is erased, Tagged attribute also needs to be erased */
      vterm_if_val->portmap.vlan_id = 0;
      vterm_if_val->portmap.tagged = 0;
      vterm_if_val->portmap.valid[UPLL_IDX_TAGGED_PM] = UNC_VF_VALID_NO_VALUE;
    } else if ((vterm_if_val->portmap.valid[UPLL_IDX_VLAN_ID_PM] ==
               UNC_VF_INVALID) && (operation == UNC_OP_CREATE)) {
        vterm_if_val->portmap.valid[UPLL_IDX_TAGGED_PM] = UNC_VF_INVALID;
    }

    /* Validates vlan tag attribute in portmap structure */
    if (vterm_if_val->portmap.valid[UPLL_IDX_TAGGED_PM] == UNC_VF_VALID) {
      if (!ValidateNumericRange((uint8_t) vterm_if_val->portmap.tagged,
                                (uint8_t) UPLL_VLAN_UNTAGGED,
                                (uint8_t) UPLL_VLAN_TAGGED, true, true)) {
        UPLL_LOG_DEBUG("Tagged Numeric range check failed. Received Tag - %d",
                       vterm_if_val->portmap.tagged);
        return UPLL_RC_ERR_CFG_SYNTAX;
      }
    } else if (((vterm_if_val->portmap.valid[UPLL_IDX_TAGGED_PM] ==
          UNC_VF_VALID_NO_VALUE) ||
          (vterm_if_val->portmap.valid[UPLL_IDX_TAGGED_PM] == UNC_VF_INVALID))
          && (operation == UNC_OP_CREATE)) {
      if (vterm_if_val->portmap.valid[UPLL_IDX_VLAN_ID_PM] == UNC_VF_VALID) {
        vterm_if_val->portmap.tagged = UPLL_VLAN_TAGGED;
        vterm_if_val->portmap.valid[UPLL_IDX_TAGGED_PM] = UNC_VF_VALID;
      }
    }
  } else if ((vterm_if_val->valid[UPLL_IDX_PM_VBRI] == UNC_VF_VALID_NO_VALUE) &&
      (operation == UNC_OP_UPDATE || operation == UNC_OP_CREATE)) {
    memset(&(vterm_if_val->portmap), 0, sizeof(vterm_if_val->portmap));
    vterm_if_val->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] =
        UNC_VF_VALID_NO_VALUE;
    vterm_if_val->portmap.valid[UPLL_IDX_VLAN_ID_PM] = UNC_VF_VALID_NO_VALUE;
    vterm_if_val->portmap.valid[UPLL_IDX_TAGGED_PM] = UNC_VF_VALID_NO_VALUE;
  }

  return UPLL_RC_SUCCESS;
}

upll_rc_t VtermIfMoMgr::ValidateCapability(IpcReqRespHeader *req,
                                           ConfigKeyVal *ikey,
                                           const char *ctrlr_name) {
  UPLL_FUNC_TRACE;

  upll_rc_t      ret_val            = UPLL_RC_SUCCESS;
  bool           result_code        = false;
  uint32_t       max_attrs          = 0;
  uint32_t       max_instance_count = 0;
  const uint8_t *attrs              = NULL;


  if ( !ikey || !req ) {
    UPLL_LOG_DEBUG("ConfigKeyVal / IpcReqRespHeader is Null");
    return UPLL_RC_ERR_GENERIC;
  }

  if (!ctrlr_name) {
    ctrlr_name = reinterpret_cast<char*>(
        (reinterpret_cast<key_user_data_t *>(ikey->get_user_data()))->ctrlr_id);
    if (!ctrlr_name || !strlen(ctrlr_name)) {
      UPLL_LOG_DEBUG("Controller Name is NULL");
      return UPLL_RC_ERR_GENERIC;
    }
  }

  switch (req->operation) {
    case UNC_OP_CREATE:
      result_code = GetCreateCapability(ctrlr_name, ikey->get_key_type(),
                                        &max_instance_count,
                                        &max_attrs, &attrs);
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
      UPLL_LOG_INFO("Invalid operation code - (%d)", req->operation);
      return UPLL_RC_ERR_GENERIC;
  }

  if (!result_code) {
    UPLL_LOG_INFO("key_type - %d is not supported by controller - %s",
                  ikey->get_key_type(), ctrlr_name);
    return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
  }

  if (max_attrs > 0) {
    ret_val = ValVtermIfAttributeSupportCheck(attrs, ikey, req->operation,
                                              req->datatype);
    return ret_val;
  } else {
    UPLL_LOG_DEBUG("Attribute list is empty for operation %d", req->operation);
    return UPLL_RC_ERR_GENERIC;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtermIfMoMgr::ValidateAttribute(ConfigKeyVal *ikey,
                                          DalDmlIntf *dmi,
                                          IpcReqRespHeader *req) {
  UPLL_FUNC_TRACE;

  upll_rc_t result_code = UPLL_RC_SUCCESS;

  val_vterm_if_t *if_val = reinterpret_cast<val_vterm_if *>(GetVal(ikey));
  if (!if_val) {
    if (req->operation == UNC_OP_CREATE || req->operation == UNC_OP_DELETE) {
      UPLL_LOG_DEBUG("Val Structure is Null");
      return UPLL_RC_SUCCESS;
    } else {
      UPLL_LOG_DEBUG("Val structure is must");
      return UPLL_RC_ERR_GENERIC;
    }
  }

  ConfigKeyVal *vtermif_ckv_tmp = NULL;
  result_code = DupConfigKeyVal(vtermif_ckv_tmp, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_ERROR("Failed to get duplicate VTERM_IF "
                   "ConfigKeyVal from ikey");
    return result_code;
  }

  /* Verifies whether the same logical-port-id and vlan-id is
   * used in other vterm_if under the same controller and same
   * domain*/
  result_code = IsLogicalPortAndVlanIdInUse<val_vterm_if_t>
      (vtermif_ckv_tmp, dmi, req);
  DELETE_IF_NOT_NULL(vtermif_ckv_tmp);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_ERROR("Same logical_portid, vlan_id, vlan_tagged "
                   "is already mapped in other vterminal interface");
    return result_code;
  }


  ConfigKeyVal *vbr_if_ckv = NULL;
  VbrIfMoMgr* vbrif_mgr =  reinterpret_cast<VbrIfMoMgr *>(
      const_cast<MoManager *>(GetMoManager(UNC_KT_VBR_IF)));

  result_code = vbrif_mgr->GetChildConfigKey(vbr_if_ckv, NULL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_ERROR("Failed to create vtem_if duplicate ConfigKeyVal");
    return result_code;
  }
  val_drv_vbr_if *drv_vbrif_val = reinterpret_cast<val_drv_vbr_if *>
      (ConfigKeyVal::Malloc(sizeof(val_drv_vbr_if)));
  /* Copies only portmap attributes to vbr_if value structure */
  memcpy(&(drv_vbrif_val->vbr_if_val.portmap), &(if_val->portmap),
         sizeof(val_port_map_t));
  drv_vbrif_val->vbr_if_val.valid[UPLL_IDX_PM_VBRI] =
           if_val->valid[UPLL_IDX_PM_VBRI];
  vbr_if_ckv->AppendCfgVal(IpctSt::kIpcStPfcdrvValVbrIf, drv_vbrif_val);
  SET_USER_DATA(vbr_if_ckv, ikey);
  /* Verifies whether the same logical-port-id and
   * vlan-id is used in vbridge interface under the same controller
   * and same domain*/
  result_code = vbrif_mgr->IsLogicalPortAndVlanIdInUse
      <val_vbr_if_t>(vbr_if_ckv, dmi, req);
  DELETE_IF_NOT_NULL(vbr_if_ckv);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_ERROR("Same logical_portid, vlan_id, vlan_tagged "
                   "is already mapped in other vbridge interface");
    return result_code;
  }

  return result_code;
}

upll_rc_t VtermIfMoMgr::ValVtermIfAttributeSupportCheck(const uint8_t *attrs,
                         ConfigKeyVal *ikey, unc_keytype_operation_t operation,
                         upll_keytype_datatype_t dt_type) {
  UPLL_FUNC_TRACE;

  if (!ikey)
    return UPLL_RC_ERR_GENERIC;

  ConfigVal      *cfg_val      = NULL;
  val_vterm_if_t *vterm_if_val = NULL;

  cfg_val = ikey->get_cfg_val();
  if (!cfg_val)
    return UPLL_RC_SUCCESS;

  // Get vterm_if val structure from config key val
  vterm_if_val = reinterpret_cast<val_vterm_if_t *>(GetVal(ikey));
  if (vterm_if_val == NULL) {
    UPLL_LOG_INFO("ERROR:Vterm_if Value structure is NULL");
    return UPLL_RC_ERR_GENERIC;
  }

  // Description attribute check
  if (attrs[unc::capa::vterm_if::kCapDesc] == 0) {
    if ((operation == UNC_OP_CREATE &&
        (vterm_if_val->valid[UPLL_IDX_DESC_VTERMI] == UNC_VF_VALID)) ||
        (operation == UNC_OP_UPDATE &&
        ((vterm_if_val->valid[UPLL_IDX_DESC_VTERMI] == UNC_VF_VALID) ||
        (vterm_if_val->valid[UPLL_IDX_DESC_VTERMI] ==
        UNC_VF_VALID_NO_VALUE)))) {
      UPLL_LOG_INFO("Description attr is not supported by ctrlr ");
      return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
    }
    vterm_if_val->valid[UPLL_IDX_DESC_VTERMI] = UNC_VF_INVALID;
  }

  // Admin stattus attribute check
  if (attrs[unc::capa::vterm_if::kCapAdminStatus] == 0) {
    if ((operation == UNC_OP_CREATE &&
       (vterm_if_val->valid[UPLL_IDX_ADMIN_STATUS_VTERMI] == UNC_VF_VALID))
       || (operation == UNC_OP_UPDATE &&
       ((vterm_if_val->valid[UPLL_IDX_ADMIN_STATUS_VTERMI] == UNC_VF_VALID)
       || (vterm_if_val->valid[UPLL_IDX_ADMIN_STATUS_VTERMI] ==
       UNC_VF_VALID_NO_VALUE)))) {
      UPLL_LOG_INFO("Admin status attr is not supported by ctrlr ");
      return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
    }
    vterm_if_val->valid[UPLL_IDX_ADMIN_STATUS_VTERMI] = UNC_VF_INVALID;
  }

  // port map attribute check
  if (vterm_if_val->valid[UPLL_IDX_PM_VTERMI] == UNC_VF_VALID) {
    // port map logical_port_id attribute check
    if ((vterm_if_val->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] ==
         UNC_VF_VALID) &&
        (attrs[unc::capa::vterm_if::kCapLogicalPortId] == 0)) {
      if (operation == UNC_OP_CREATE || operation == UNC_OP_UPDATE) {
        UPLL_LOG_INFO("portmap.logical_port_id attr is not "
                      "supported by ctrlr ");
        return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
      }
      vterm_if_val->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] =
          UNC_VF_INVALID;
     }

    // port map vlan id attribute check
    if (vterm_if_val->portmap.valid[UPLL_IDX_VLAN_ID_PM] == UNC_VF_VALID &&
        attrs[unc::capa::vterm_if::kCapVlanId] == 0) {
      if (operation == UNC_OP_CREATE || operation == UNC_OP_UPDATE) {
        UPLL_LOG_INFO("portmap.vlanid attr is not supported by ctrlr ");
        return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
      }
      vterm_if_val->portmap.valid[UPLL_IDX_VLAN_ID_PM] = UNC_VF_INVALID;
    }

    // port map vlan tagged attribute check
    if (vterm_if_val->portmap.valid[UPLL_IDX_TAGGED_PM] == UNC_VF_VALID &&
        attrs[unc::capa::vterm_if::kCapTagged] == 0) {
      if (operation == UNC_OP_CREATE || operation == UNC_OP_UPDATE) {
        UPLL_LOG_INFO("portmap.Tagged attr is not supported by ctrlr ");
        return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
      }
      vterm_if_val->portmap.valid[UPLL_IDX_TAGGED_PM] = UNC_VF_INVALID;
    }
  } else if (vterm_if_val->valid[UPLL_IDX_PM_VTERMI] == UNC_VF_VALID_NO_VALUE) {
    // port map logical_port_id attribute check
    if ((vterm_if_val->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] ==
         UNC_VF_VALID_NO_VALUE) &&
        (attrs[unc::capa::vterm_if::kCapLogicalPortId] == 0)) {
      if (operation == UNC_OP_UPDATE) {
        UPLL_LOG_INFO("portmap.logical_port_id attr is not "
                      "supported by ctrlr ");
        return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
      }
      vterm_if_val->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] =
          UNC_VF_INVALID;
     }

    // port map vlan id attribute check
    if (vterm_if_val->portmap.valid[UPLL_IDX_VLAN_ID_PM] ==
        UNC_VF_VALID_NO_VALUE && attrs[unc::capa::vterm_if::kCapVlanId] == 0) {
      if (operation == UNC_OP_UPDATE) {
        UPLL_LOG_INFO("portmap.vlanid attr is not supported by ctrlr ");
        return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
      }
      vterm_if_val->portmap.valid[UPLL_IDX_VLAN_ID_PM] = UNC_VF_INVALID;
    }

    // port map vlan tagged attribute check
    if (vterm_if_val->portmap.valid[UPLL_IDX_TAGGED_PM] ==
        UNC_VF_VALID_NO_VALUE && attrs[unc::capa::vterm_if::kCapTagged] == 0) {
      if (operation == UNC_OP_UPDATE) {
        UPLL_LOG_INFO("portmap.Tagged attr is not supported by ctrlr ");
        return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
      }
      vterm_if_val->portmap.valid[UPLL_IDX_TAGGED_PM] = UNC_VF_INVALID;
    }
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtermIfMoMgr::CopyToConfigKey(ConfigKeyVal *&okey,
                                        ConfigKeyVal *ikey) {
  UPLL_FUNC_TRACE;

  if (!ikey || !(ikey->get_key()))
       return UPLL_RC_ERR_GENERIC;

  key_rename_vnode_info *key_rename =
                    reinterpret_cast<key_rename_vnode_info *>(ikey->get_key());
  if (!strlen(reinterpret_cast<char *>(key_rename->old_unc_vtn_name)))
    return UPLL_RC_ERR_GENERIC;

  key_vterm_if_t *vterm_if_key = reinterpret_cast<key_vterm_if_t *>
      (ConfigKeyVal::Malloc(sizeof(key_vterm_if_t)));

  uuu::upll_strncpy(vterm_if_key->vterm_key.vtn_key.vtn_name,
                    key_rename->old_unc_vtn_name, (kMaxLenVtnName + 1));
  if (UNC_KT_VTERMINAL == ikey->get_key_type()) {
    if (!strlen(reinterpret_cast<char *>(key_rename->old_unc_vnode_name))) {
      free(vterm_if_key);
      return UPLL_RC_ERR_GENERIC;
    }
    uuu::upll_strncpy(vterm_if_key->vterm_key.vterminal_name,
                      key_rename->old_unc_vnode_name, (kMaxLenVnodeName + 1));
  } else {
    if (!strlen(reinterpret_cast<char *>(key_rename->new_unc_vnode_name))) {
      free(vterm_if_key);
      return UPLL_RC_ERR_GENERIC;
    }
    uuu::upll_strncpy(vterm_if_key->vterm_key.vterminal_name,
                      key_rename->new_unc_vnode_name, (kMaxLenVnodeName + 1));
  }

  okey = new ConfigKeyVal(UNC_KT_VTERM_IF, IpctSt::kIpcStKeyVtermIf,
                          vterm_if_key, NULL);
  if (!okey) {
    free(vterm_if_key);
    return UPLL_RC_ERR_GENERIC;
  }
  return UPLL_RC_SUCCESS;
}

bool VtermIfMoMgr::FilterAttributes(void *&val1, void *val2,
                                    bool copy_to_running,
                                    unc_keytype_operation_t op) {
  UPLL_FUNC_TRACE;

  val_vterm_if_t *val_vterm_if1 = reinterpret_cast<val_vterm_if_t *>(val1);

  /* No need to configure description in controller. */
  val_vterm_if1->valid[UPLL_IDX_DESC_VTERMI] = UNC_VF_INVALID;

  if (op != UNC_OP_CREATE)
    return CompareValidValue(val1, val2, copy_to_running);

  return false;
}

bool VtermIfMoMgr::CompareValidValue(void *&val1, void *val2,
                                     bool copy_to_running) {
  UPLL_FUNC_TRACE;

  bool invalid_attr = true;

  val_vterm_if_t *val_vterm_if1 = reinterpret_cast<val_vterm_if_t *>(val1);
  val_vterm_if_t *val_vterm_if2 = reinterpret_cast<val_vterm_if_t *>(val2);

  for (uint8_t loop = 0;
       loop < sizeof(val_vterm_if1->valid) / sizeof(uint8_t); ++loop) {
    if (UNC_VF_INVALID == val_vterm_if1->valid[loop]
        && UNC_VF_VALID == val_vterm_if2->valid[loop])
      val_vterm_if1->valid[loop] = UNC_VF_VALID_NO_VALUE;
  }

  for (unsigned int loop = 0;
       loop < sizeof(val_vterm_if1->portmap.valid) / sizeof(uint8_t); ++loop) {
    if (UNC_VF_INVALID == val_vterm_if1->portmap.valid[loop]
        && UNC_VF_VALID == val_vterm_if2->portmap.valid[loop])
      val_vterm_if1->portmap.valid[loop] = UNC_VF_VALID_NO_VALUE;
  }
  val_vterm_if1->valid[UPLL_IDX_ADMIN_STATUS_VTERMI] = UNC_VF_INVALID;
  if (copy_to_running) {
    if ( UNC_VF_INVALID != val_vterm_if1->valid[UPLL_IDX_DESC_VTERMI] ) {
      if ((UNC_VF_VALID == val_vterm_if1->valid[UPLL_IDX_DESC_VTERMI]) &&
          (!strcmp(reinterpret_cast<char *>(val_vterm_if1->description),
                   reinterpret_cast<const char*>(val_vterm_if2->description))))
        val_vterm_if1->valid[UPLL_IDX_DESC_VTERMI] = UNC_VF_INVALID;
    }
    /* Compares port map info between candidate and running config */
    if (val_vterm_if1->valid[UPLL_IDX_PM_VTERMI] == UNC_VF_VALID
        && val_vterm_if2->valid[UPLL_IDX_PM_VTERMI] == UNC_VF_VALID) {
      if (memcmp(&(val_vterm_if1->portmap), &(val_vterm_if2->portmap),
                 sizeof(val_port_map_t))) {
        if (val_vterm_if1->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] ==
            UNC_VF_VALID
            && val_vterm_if2->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM]
            == UNC_VF_VALID) {
          if (!strcmp(
                  reinterpret_cast<char *>
                  (val_vterm_if1->portmap.logical_port_id),
                  reinterpret_cast<char *>
                  (val_vterm_if2->portmap.logical_port_id)))
            val_vterm_if1->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] =
                UNC_VF_INVALID;
        }

        if (val_vterm_if1->portmap.valid[UPLL_IDX_VLAN_ID_PM] != UNC_VF_INVALID
            && val_vterm_if2->portmap.valid[UPLL_IDX_VLAN_ID_PM] !=
            UNC_VF_INVALID) {
          if (val_vterm_if1->portmap.vlan_id == val_vterm_if2->portmap.vlan_id)
            val_vterm_if1->portmap.valid[UPLL_IDX_VLAN_ID_PM] = UNC_VF_INVALID;
        }

        if (val_vterm_if1->portmap.valid[UPLL_IDX_TAGGED_PM] != UNC_VF_INVALID
            && val_vterm_if2->portmap.valid[UPLL_IDX_TAGGED_PM] !=
            UNC_VF_INVALID) {
          if (val_vterm_if1->portmap.tagged == val_vterm_if2->portmap.tagged) {
            if (val_vterm_if1->portmap.valid[UPLL_IDX_TAGGED_PM] ==
                val_vterm_if2->portmap.valid[UPLL_IDX_TAGGED_PM]) {
              val_vterm_if1->portmap.valid[UPLL_IDX_TAGGED_PM] = UNC_VF_INVALID;
            }
          }
        }
      } else {
        UPLL_LOG_DEBUG("Portmap details not modified");
        val_vterm_if1->valid[UPLL_IDX_PM_VTERMI] = UNC_VF_INVALID;
        val_vterm_if1->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] =
            UNC_VF_INVALID;
        val_vterm_if1->portmap.valid[UPLL_IDX_VLAN_ID_PM] = UNC_VF_INVALID;
        val_vterm_if1->portmap.valid[UPLL_IDX_TAGGED_PM] = UNC_VF_INVALID;
      }
    }
  }
  if (!copy_to_running)
    val_vterm_if1->valid[UPLL_IDX_DESC_VTERMI] = UNC_VF_INVALID;
  for (unsigned int loop = 0;
       loop < sizeof(val_vterm_if1->valid) / sizeof(val_vterm_if1->valid[0]);
       ++loop) {
    if ((UNC_VF_VALID == (uint8_t) val_vterm_if1->valid[loop]) ||
        (UNC_VF_VALID_NO_VALUE == (uint8_t) val_vterm_if1->valid[loop])) {
      if (loop == UPLL_IDX_PM_VTERMI) {
        for (unsigned int i = 0;
             i < sizeof(val_vterm_if1->portmap.valid) / sizeof(uint8_t); ++i) {
          if ((UNC_VF_VALID == (uint8_t) val_vterm_if1->portmap.valid[i]) ||
              (UNC_VF_VALID_NO_VALUE ==
               (uint8_t)  val_vterm_if1->portmap.valid[i])) {
            invalid_attr = false;
            break;
          }
        }
      } else {
        invalid_attr = false;
      }
      if (invalid_attr == false) break;
    }
  }
  return invalid_attr;
}

upll_rc_t VtermIfMoMgr::IsReferenced(IpcReqRespHeader *req,
                                     ConfigKeyVal *ikey,
                                     DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;

  return UPLL_RC_SUCCESS;
}


upll_rc_t VtermIfMoMgr::AdaptValToVtnService(ConfigKeyVal *ikey,
                                             AdaptType adapt_type) {
  UPLL_FUNC_TRACE;

  if (!ikey) {
    UPLL_LOG_DEBUG("Invalid ikey");
    return UPLL_RC_ERR_GENERIC;
  }

  key_vterm_if_t *vtermif_key =
      reinterpret_cast<key_vterm_if_t*>(ikey->get_key());
  while (ikey) {
    ConfigVal *cval = ikey->get_cfg_val();
    if (!cval) {
      UPLL_LOG_DEBUG("Config Val is Null");
      return UPLL_RC_ERR_GENERIC;
    }

    while (cval) {
      if (IpctSt::kIpcStValVtermIf == cval->get_st_num()) {
        val_vterm_if *vterm_if_val =
             reinterpret_cast<val_vterm_if *>GetVal(ikey);
        if (!vterm_if_val)
          return UPLL_RC_ERR_GENERIC;
        if (vterm_if_val->valid[UPLL_IDX_ADMIN_STATUS_VTERMI] == UNC_VF_INVALID)
          vterm_if_val->valid[UPLL_IDX_ADMIN_STATUS_VTERMI] =
                                             UNC_VF_VALID_NO_VALUE;
      }

      if (IpctSt::kIpcStValVtermIfSt == cval->get_st_num()) {
        controller_domain ctrlr_dom = {NULL, NULL};
        GET_USER_DATA_CTRLR_DOMAIN(ikey, ctrlr_dom);
        CheckOperStatus<val_vterm_if_st>(
            vtermif_key->vterm_key.vtn_key.vtn_name,
            cval, UNC_KT_VTERM_IF, ctrlr_dom);
      }
      cval = cval->get_next_cfg_val();
    }
    if (adapt_type == ADAPT_ONE)
      break;
    ikey = ikey->get_next_cfg_key_val();
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtermIfMoMgr::GetPortmapInfo(ConfigKeyVal *ikey,
                                       upll_keytype_datatype_t dt_type,
                                       DalDmlIntf *dmi,
                                       InterfacePortMapInfo &iftype) {
  UPLL_FUNC_TRACE;

  upll_rc_t result_code = UPLL_RC_SUCCESS;

  key_vterm_if_t *vterm_if_key = (ikey)?reinterpret_cast<key_vterm_if_t*>(
      ikey->get_key()) : NULL;
  if (!vterm_if_key) {
    UPLL_LOG_DEBUG("Key is NULL ");
    return UPLL_RC_ERR_GENERIC;
  }

  // Get vterminal interface from DB
  DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutFlag };
  switch (dt_type) {
    case UPLL_DT_CANDIDATE:
    case UPLL_DT_IMPORT:
    case UPLL_DT_RUNNING:
    case UPLL_DT_AUDIT:
      result_code = ReadConfigDB(ikey, dt_type, UNC_OP_READ, dbop,
                                 dmi, MAINTBL);
      break;
    default:
      UPLL_LOG_DEBUG("Invalid Datatype %d", dt_type);
      return UPLL_RC_ERR_GENERIC;
      break;
  }

  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("ReadConfiDB Failed: %d", result_code);
    return result_code;
  }

  val_vterm_if_t *vterm_if_val = reinterpret_cast<val_vterm_if_t *>
      (GetVal(ikey));
  if ((vterm_if_val->valid[UPLL_IDX_PM_VTERMI] == UNC_VF_VALID) &&
      (vterm_if_val->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] ==
       UNC_VF_VALID)) {
    iftype = kPortMapConfigured;
  } else if (vterm_if_val->valid[UPLL_IDX_PM_VTERMI] == UNC_VF_INVALID) {
    iftype = kVlinkPortMapNotConfigured;
  }

  return UPLL_RC_SUCCESS;
}

upll_rc_t VtermIfMoMgr::IsVtermIfAlreadyExists(ConfigKeyVal *ikey,
                                   DalDmlIntf *dmi, IpcReqRespHeader *req) {
  UPLL_FUNC_TRACE;

  upll_rc_t    result_code  = UPLL_RC_SUCCESS;
  ConfigKeyVal *ckv_vtermif = NULL;

  result_code = GetChildConfigKey(ckv_vtermif, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
     UPLL_LOG_DEBUG("Get VtermIfkey failed");

     return result_code;
  }
  key_vterm_if_t *vtermif_key =
       reinterpret_cast<key_vterm_if_t *>(ckv_vtermif->get_key());
  StringReset(vtermif_key->if_name);

  /* checkin the vterminal exists in the vterm if table or not
   * if avaialable the Interface already created otherwise not creaed
   */

  DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutNone };
  result_code = ReadConfigDB(ckv_vtermif, req->datatype, UNC_OP_READ,
      dbop, dmi, MAINTBL);
  if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
    delete ckv_vtermif;
    return UPLL_RC_SUCCESS;
  }

  if (UPLL_RC_SUCCESS == result_code) {
    key_vterm_if_t *vtermif_key =
       reinterpret_cast<key_vterm_if_t *>(ckv_vtermif->get_key());
    key_vterm_if_t  *ivtermif_key =
       reinterpret_cast<key_vterm_if_t *>(ikey->get_key());

    if (!strcmp(reinterpret_cast<char*>(vtermif_key->if_name),
                reinterpret_cast<char*>(ivtermif_key->if_name))) {
      delete ckv_vtermif;
      UPLL_LOG_DEBUG("Same Vterminal Interface is already exists"
                     "under requested Vterminal");
      return UPLL_RC_ERR_INSTANCE_EXISTS;
    }
    UPLL_LOG_DEBUG("Cannot configure more than one interface under vterminal");
    delete ckv_vtermif;
    return UPLL_RC_ERR_CFG_SEMANTIC;
  }
  delete ckv_vtermif;
  return result_code;
}

upll_rc_t VtermIfMoMgr::GetPortMap(ConfigKeyVal *ikey,
                                   uint8_t &valid_pm,
                                   val_port_map_t *&pm,
                                   uint8_t &valid_admin,
                                   uint8_t &admin_status) {
  UPLL_FUNC_TRACE;

  if (ikey == NULL) return UPLL_RC_ERR_GENERIC;

  val_vterm_if_t *vtermif_val = reinterpret_cast<val_vterm_if *>
                                                (GetVal(ikey));
  if (!vtermif_val) {
    UPLL_LOG_DEBUG("Invalid param");
    return UPLL_RC_ERR_GENERIC;
  }

  valid_pm = vtermif_val->valid[UPLL_IDX_PM_VTERMI];
  if (valid_pm == UNC_VF_VALID)
    pm = &vtermif_val->portmap;
  else
    pm = NULL;

  valid_admin = vtermif_val->valid[UPLL_IDX_ADMIN_STATUS_VTERMI];
  admin_status = vtermif_val->admin_status;

  return UPLL_RC_SUCCESS;
}

upll_rc_t VtermIfMoMgr::GetValid(void *val, uint64_t indx, uint8_t *&valid,
                                 upll_keytype_datatype_t dt_type,
                                 MoMgrTables tbl ) {
  UPLL_FUNC_TRACE;
  if (val == NULL) return UPLL_RC_ERR_GENERIC;

  if (tbl == MAINTBL) {
    switch (indx) {
      case uudst::vterminal_interface::kDbiOperStatus:
        valid = &(reinterpret_cast<val_vterm_if_st *>(val))->
            valid[UPLL_IDX_OPER_STATUS_VTERMIS];
        break;
      case uudst::vterminal_interface::kDbiDownCount:
        valid = NULL;
        break;
      case uudst::vterminal_interface::kDbiAdminStatus:
        valid = &(reinterpret_cast<val_vterm_if *>(val))->
            valid[UPLL_IDX_ADMIN_STATUS_VTERMI];
        break;
      case uudst::vterminal_interface::kDbiDesc:
        valid = &(reinterpret_cast<val_vterm_if *>(val))->
            valid[UPLL_IDX_DESC_VTERMI];
        break;
      case uudst::vterminal_interface::kDbiValidPortMap:
        valid = &(reinterpret_cast<val_vterm_if *>(val))->
            valid[UPLL_IDX_PM_VTERMI];
        break;
      case uudst::vterminal_interface::kDbiLogicalPortId:
        valid = &(reinterpret_cast<val_vterm_if *>(val))->portmap.
            valid[UPLL_IDX_LOGICAL_PORT_ID_PM];
        break;
      case uudst::vterminal_interface::kDbiVlanId:
        valid = &(reinterpret_cast<val_vterm_if *>(val))->portmap.
            valid[UPLL_IDX_VLAN_ID_PM];
        break;
      case uudst::vterminal_interface::kDbiTagged:
        valid = &(reinterpret_cast<val_vterm_if *>(val))->portmap.
            valid[UPLL_IDX_TAGGED_PM];
        break;
      default:
        return UPLL_RC_ERR_GENERIC;
    }
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtermIfMoMgr::TranslateVbrIfToVtermIfError(
                                     ConfigKeyVal *&translated_ckv,
                                     ConfigKeyVal *ckv_drv,
                                     upll_keytype_datatype_t datatype,
                                     DalDmlIntf *dmi, uint8_t* ctrlr_id) {
  UPLL_FUNC_TRACE;

  if (!ckv_drv || !ckv_drv->get_key()) {
    UPLL_LOG_ERROR("Invalid error ConfigKeyVal");
    return UPLL_RC_ERR_GENERIC;
  }

  upll_rc_t    result_code   = UPLL_RC_SUCCESS;
  ConfigKeyVal *vtermif_ckv  = NULL;

  key_vbr_if_t   *drv_key      = reinterpret_cast<key_vbr_if_t *>(
      ckv_drv->get_key());
  val_drv_vbr_if *drv_valvbrif = reinterpret_cast<val_drv_vbr_if *>
      (GetVal(ckv_drv));

  if (!drv_valvbrif) {
    UPLL_LOG_ERROR("Value structure not present in driver error ckv");
    return UPLL_RC_ERR_NO_SUCH_INSTANCE;
  }

  if (drv_valvbrif->valid[PFCDRV_IDX_VEXT_NAME_VBRIF] == UNC_VF_VALID) {
    ConfigKeyVal *vterm_ckv = NULL;
    MoMgrImpl *vterm_mgr = reinterpret_cast<MoMgrImpl *>
      (const_cast<MoManager *>(GetMoManager(UNC_KT_VTERMINAL)));
    if (!vterm_mgr) {
      UPLL_LOG_ERROR("Invalid Mgr");
      return UPLL_RC_ERR_GENERIC;
    }

    result_code = vterm_mgr->GetChildConfigKey(vterm_ckv, NULL);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("GetParentConfigKey failed");
      return result_code;
    }
    ConfigVal *vterm_val = NULL;
    result_code  = vterm_mgr->AllocVal(vterm_val, UPLL_DT_RUNNING, RENAMETBL);
    if (UPLL_RC_SUCCESS != result_code) {
      delete vterm_ckv;
      UPLL_LOG_DEBUG("AllocVal failed");
      return result_code;
    }
    vterm_ckv->AppendCfgVal(vterm_val);
    key_vterm_t *vterm_key =
        reinterpret_cast<key_vterm_t *>(vterm_ckv->get_key());
    val_rename_vnode *val_node =
        reinterpret_cast<val_rename_vnode*>(GetVal(vterm_ckv));
    uuu::upll_strncpy(vterm_key->vtn_key.vtn_name,
                      drv_key->vbr_key.vtn_key.vtn_name,
                      kMaxLenVtnName+1);
    uuu::upll_strncpy(val_node->ctrlr_vnode_name, drv_valvbrif->vex_name,
                      kMaxLenVnodeName+1);
    val_node->valid[UPLL_CTRLR_VNODE_NAME_VALID] = UNC_VF_VALID;
    SET_USER_DATA_CTRLR(vterm_ckv, ctrlr_id);

    UPLL_LOG_TRACE("%s Before vterm ckv", (vterm_ckv->ToStr()).c_str());
    DbSubOp dbop = { kOpReadSingle, kOpMatchCtrlr, kOpInOutNone };
    result_code = vterm_mgr->ReadConfigDB(vterm_ckv, datatype, UNC_OP_READ,
                                          dbop, dmi, RENAMETBL);
    if (result_code != UPLL_RC_SUCCESS &&
        UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
      UPLL_LOG_ERROR("Failed to read VTERM_IF info from DB");
      DELETE_IF_NOT_NULL(vterm_ckv);
      return result_code;
    }

    if (result_code == UPLL_RC_SUCCESS) {
      /*Vterminal is naot renamed, so need to check with main table. */
      result_code =  GetChildConfigKey(vtermif_ckv, vterm_ckv);
      if (UPLL_RC_SUCCESS != result_code) {
        UPLL_LOG_DEBUG("GetChildConfigKey failed");
        delete vterm_ckv;
        return result_code;
      }
    } else {
      result_code = GetChildConfigKey(vtermif_ckv, vterm_ckv);
      if (UPLL_RC_SUCCESS != result_code) {
        UPLL_LOG_DEBUG("GetChildConfigKey failed");
        delete vterm_ckv;
        return result_code;
      }
      uuu::upll_strncpy(reinterpret_cast<key_vterm_if_t *>
                   (vtermif_ckv->get_key())->vterm_key.vterminal_name,
                    drv_valvbrif->vex_name, (kMaxLenVnodeName + 1));
    }
    /* Reads VTERM_IF info based on VTN name and VTERMINAL name */
    result_code = ReadConfigDB(vtermif_ckv, datatype,
        UNC_OP_READ, dbop, dmi, MAINTBL);
    DELETE_IF_NOT_NULL(vterm_ckv);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_ERROR("Failed to read VTERM_IF info from DB");
      DELETE_IF_NOT_NULL(vtermif_ckv);
      return result_code;
    }
    translated_ckv = vtermif_ckv;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtermIfMoMgr::GetVtermIfFromVexternal(uint8_t *vtn_name,
                                                uint8_t *vterminal,
                                                uint8_t *vtermif,
                                                uint8_t *ctrlr_id,
                                                DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  key_vterm_t *vtermkey = reinterpret_cast<key_vterm_t *>
      (ConfigKeyVal::Malloc(sizeof(key_vterm_t)));
  uuu::upll_strncpy(vtermkey->vtn_key.vtn_name,
                    vtn_name, (kMaxLenVtnName + 1));
  uuu::upll_strncpy(vtermkey->vterminal_name,
                    vterminal, (kMaxLenVnodeName + 1));
  ConfigKeyVal *ckv_vterm_rename = new ConfigKeyVal(UNC_KT_VTERMINAL,
                                    IpctSt::kIpcStKeyVterminal, vtermkey, NULL);
  MoMgrImpl *vterm_mgr = reinterpret_cast<MoMgrImpl *>
      (const_cast<MoManager *>(GetMoManager(UNC_KT_VTERMINAL)));
  if (!vterm_mgr) {
    UPLL_LOG_ERROR("Invalid Mgr");
    DELETE_IF_NOT_NULL(ckv_vterm_rename);
    return UPLL_RC_ERR_GENERIC;
  }
  /* Get corresponding renamed ConfigKeyVal if the
   * received vexternal name is renamed in UNC */
  result_code = vterm_mgr->GetRenamedUncKey(ckv_vterm_rename, UPLL_DT_RUNNING,
                                            dmi, ctrlr_id);
  if (result_code != UPLL_RC_SUCCESS &&
      result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    UPLL_LOG_INFO("GetRenamedUncKey failed error code %d", result_code);
    delete ckv_vterm_rename;
    return result_code;
  }
  #if 0
  uuu::upll_strncpy(vtn_name, vtermkey->vtn_key.vtn_name,
                    (kMaxLenVtnName + 1));
  #endif
  uuu::upll_strncpy(vterminal, vtermkey->vterminal_name,
                    (kMaxLenVnodeName + 1));
  ConfigKeyVal *ckv_vterm_if  = NULL;
  result_code =  GetChildConfigKey(ckv_vterm_if, ckv_vterm_rename);
  if (result_code !=  UPLL_RC_SUCCESS) {
    UPLL_LOG_ERROR("GetChildConfigKey failed error code %d", result_code);
    delete ckv_vterm_rename;
    return UPLL_RC_ERR_GENERIC;
  }
  /* Reads VTERM_IF info based on VTN name and VTERMINAL name */
  DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutNone };
  result_code = ReadConfigDB(ckv_vterm_if, UPLL_DT_RUNNING,
                                  UNC_OP_READ, dbop, dmi, MAINTBL);
  if (result_code == UPLL_RC_SUCCESS) {
      uuu::upll_strncpy(vtermif, reinterpret_cast<key_vterm_if_t *>
                         (ckv_vterm_if->get_key())->if_name,
                        (kMaxLenInterfaceName + 1));
      uuu::upll_strncpy(vterminal, reinterpret_cast<key_vterm_if_t *>
                         (ckv_vterm_if->get_key())->vterm_key.vterminal_name,
                        (kMaxLenVnodeName + 1));
  }
  UPLL_LOG_DEBUG("VTN Name = %s, Vnode name = %s, Vnode Interfcae name = %s",
                 vtn_name, vterminal, vtermif);
  delete ckv_vterm_rename;
  delete ckv_vterm_if;
  return result_code;
}
upll_rc_t VtermIfMoMgr::AdaptValToDriver(ConfigKeyVal *ck_new,
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


}  // namespace kt_momgr
}  // namespace upll
}  // namespace unc
