/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include "vtep_if_momgr.hh"
#include "vtep_momgr.hh"
#include "uncxx/upll_log.hh"
#define NUM_KEY_MAIN_TBL_ 5
namespace unc {
namespace upll {
namespace kt_momgr {

  BindInfo VtepIfMoMgr::vtep_if_bind_info[] = {
    { uudst::vtep_interface::kDbiVtnName, CFG_KEY,
      offsetof(key_vtep_if, vtep_key.vtn_key.vtn_name),
      uud::kDalChar, kMaxLenVtnName+1 },
    { uudst::vtep_interface::kDbiVtepName, CFG_KEY,
      offsetof(key_vtep_if, vtep_key.vtep_name),
      uud::kDalChar, kMaxLenVnodeName+1 },
    { uudst::vtep_interface::kDbiIfName, CFG_KEY,
      offsetof(key_vtep_if, if_name),
      uud::kDalChar, kMaxLenInterfaceName+1},
    { uudst::vtep_interface::kDbiDesc, CFG_VAL,
      offsetof(val_vtep_if, description),
      uud::kDalChar, kMaxLenDescription+1},
    { uudst::vtep_interface::kDbiAdminStatus, CFG_VAL,
      offsetof(val_vtep_if, admin_status), uud::kDalUint8, 1},
    { uudst::vtep_interface::kDbiLogicalPortId, CFG_VAL,
      offsetof(val_vtep_if, portmap.logical_port_id),
      uud::kDalChar, 320},
    { uudst::vtep_interface::kDbiVlanId, CFG_VAL,
      offsetof(val_vtep_if, portmap.vlan_id), uud::kDalUint16, 1},
    { uudst::vtep_interface::kDbiTagged, CFG_VAL,
      offsetof(val_vtep_if, portmap.tagged), uud::kDalUint8, 1},
    { uudst::vtep_interface::kDbiOperStatus, ST_VAL,
      offsetof(val_db_vtep_if_st, vtep_if_val_st.oper_status),
      uud::kDalUint8, 1},
    { uudst::vtep_interface::kDbiDownCount, ST_VAL, offsetof(
      val_db_vtep_if_st, down_count),
      uud::kDalUint32, 1 },
    { uudst::vtep_interface::kDbiCtrlrName, CK_VAL,
      offsetof(key_user_data, ctrlr_id), uud::kDalChar, 32},
    { uudst::vtep_interface::kDbiDomainId, CK_VAL,
      offsetof(key_user_data, domain_id), uud::kDalChar, 32},
    { uudst::vtep_interface::kDbiFlags, CK_VAL,
      offsetof(key_user_data, flags), uud::kDalUint8, 1},
    { uudst::vtep_interface::kDbiValidDesc, CFG_META_VAL,
      offsetof(val_vtep_if, valid[UPLL_IDX_DESC_VTEPI]), uud::kDalUint8, 1},
    { uudst::vtep_interface::kDbiValidAdminStatus, CFG_DEF_VAL,
      offsetof(val_vtep_if, valid[UPLL_IDX_ADMIN_ST_VTEPI]), uud::kDalUint8, 1},
    { uudst::vtep_interface::kDbiValidPortMap, CFG_META_VAL,
      offsetof(val_vtep_if, valid[UPLL_IDX_PORT_MAP_VTEPI]), uud::kDalUint8, 1},
    { uudst::vtep_interface::kDbiValidLogicalPortId, CFG_META_VAL, offsetof(
        val_vtep_if, portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM]),
    uud::kDalUint8, 1 },
    { uudst::vtep_interface::kDbiValidVlanid, CFG_META_VAL, offsetof(
        val_vtep_if, portmap.valid[UPLL_IDX_VLAN_ID_PM]),
    uud::kDalUint8, 1 },
    { uudst::vtep_interface::kDbiValidTagged, CFG_META_VAL, offsetof(
        val_vtep_if, portmap.valid[UPLL_IDX_TAGGED_PM]),
    uud::kDalUint8, 1 },
    { uudst::vtep_interface::kDbiValidOperStatus, ST_META_VAL,
      offsetof(val_db_vtep_if_st,
      vtep_if_val_st.valid[UPLL_IDX_IF_OPER_STATUS_VTEPIS]),
      uud::kDalUint8, 1},
    { uudst::vtep_interface::kDbiCsRowstatus, CS_VAL,
      offsetof(val_vtep_if, cs_row_status), uud::kDalUint8, 1},
    { uudst::vtep_interface::kDbiCsDesc, CS_VAL, offsetof(val_vtep_if,
      cs_attr[UPLL_IDX_DESC_VTEPI]), uud::kDalUint8, 1},
    { uudst::vtep_interface::kDbiCsAdminStatus, CS_VAL, offsetof(val_vtep_if,
      cs_attr[UPLL_IDX_ADMIN_ST_VTEPI]), uud::kDalUint8, 1},
    { uudst::vtep_interface::kDbiCsPortMap, CS_VAL, offsetof(val_vtep_if,
      cs_attr[UPLL_IDX_PORT_MAP_VTEPI]), uud::kDalUint8, 1},
    { uudst::vtep_interface::kDbiCsLogicalPortId, CS_VAL, offsetof(
        val_vtep_if, portmap.cs_attr[UPLL_IDX_LOGICAL_PORT_ID_PM]),
    uud::kDalUint8, 1 },
    { uudst::vtep_interface::kDbiCsVlanid, CS_VAL, offsetof(
        val_vtep_if, portmap.cs_attr[UPLL_IDX_VLAN_ID_PM]),
    uud::kDalUint8, 1 },
    { uudst::vtep_interface::kDbiCsTagged, CS_VAL, offsetof(
        val_vtep_if, portmap.cs_attr[UPLL_IDX_TAGGED_PM]),
    uud::kDalUint8, 1 }
  };

  VtepIfMoMgr::VtepIfMoMgr() {
    UPLL_FUNC_TRACE;
    Table *tbl = new Table(uudst::kDbiVtepIfTbl, UNC_KT_VTEP_IF,
        vtep_if_bind_info,
        IpctSt::kIpcStKeyVtepIf, IpctSt::kIpcStValVtepIf,
        uudst::vtep_interface::kDbiVtepIfNumCols);
    ntable = MAX_MOMGR_TBLS;
    table = new Table *[ntable]();
    table[MAINTBL] = tbl;
    table[RENAMETBL] = NULL;
    table[CTRLRTBL] = NULL;
    table[CONVERTTBL] = NULL;

    nchild = 0;
    child = NULL;
#ifdef _STANDALONE_
    SetMoManager(UNC_KT_VTEP_IF, reinterpret_cast<MoMgr *>(this));
#endif
  }

  /*
   *  * Based on the key type the bind info will pass
   *   *
   bool VtepIfMoMgr::GetRenameKeyBindInfo(unc_key_type_t key_type,
   BindInfo *&binfo, int &nattr,
   MoMgrTables tbl ) {
   if (MAINTBL == tbl) {
   nattr = NUM_KEY_MAIN_TBL_;
   binfo = key_vtep_if_maintbl_update_bind_info;
   }
   return PFC_TRUE;
   }*/

  upll_rc_t VtepIfMoMgr::ValidateAttribute(ConfigKeyVal *ikey,
                                           DalDmlIntf *dmi,
                                           IpcReqRespHeader *req) {
    UPLL_FUNC_TRACE;
    upll_rc_t result_code = UPLL_RC_SUCCESS;
    if (!ikey || (ikey->get_key_type() != UNC_KT_VTEP_IF)
              || !(ikey->get_cfg_val()))
      return UPLL_RC_ERR_CFG_SYNTAX;

    val_vtep_if *vtepif_val = static_cast<val_vtep_if *>(GetVal(ikey));
    if (!vtepif_val) {
      if (req->operation == UNC_OP_CREATE) {
        UPLL_LOG_DEBUG("Val Structure is Null");
        return UPLL_RC_SUCCESS;
      } else {
        UPLL_LOG_DEBUG("Val structure is must");
        return UPLL_RC_ERR_GENERIC;
      }
    }
    result_code = IsLogicalPortAndVlanIdInUse<val_vtep_if>(ikey, dmi, req);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_ERROR("Returning error %d\n", result_code);
      return result_code;
    }
    // PortMap Validation
    uint32_t operation = req->operation;
    if (operation == UNC_OP_UPDATE) {
      result_code = UpdateConfigVal(ikey, UPLL_DT_CANDIDATE, dmi);
      UPLL_LOG_TRACE("UpdateConfigVal returned %d", result_code);
    }
    return result_code;
  }

#if 0
  upll_rc_t VtepIfMoMgr::IsLogicalPortAndVlanIdInUse(ConfigKeyVal *ikey,
                                                   DalDmlIntf *dmi,
                                                   IpcReqRespHeader *req) {
    UPLL_FUNC_TRACE;
    upll_rc_t result_code = UPLL_RC_SUCCESS;
    if (!ikey || (ikey->get_key_type() != UNC_KT_VTEP_IF)
              || !(ikey->get_cfg_val()))
      return UPLL_RC_ERR_CFG_SYNTAX;

    val_vtep_if *vtepif_val = static_cast<val_vtep_if *>(GetVal(ikey));
    if (!vtepif_val) {
      return UPLL_RC_ERR_GENERIC;
    }
    if (vtepif_val->valid[UPLL_IDX_PORT_MAP_VTEPI] == UNC_VF_VALID) {
      if (vtepif_val->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] ==
          UNC_VF_VALID &&
          vtepif_val->portmap.valid[UPLL_IDX_VLAN_ID_PM] == UNC_VF_VALID) {
        key_vtep_if *vtep_key_if = static_cast<key_vtep_if *>
                                 (ConfigKeyVal::Malloc(sizeof(key_vtep_if)));
        val_vtep_if *vtepif_val_filter = static_cast<val_vtep_if *>
                    (ConfigKeyVal::Malloc(sizeof(val_vtep_if)));
        vtepif_val_filter->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] =
            UNC_VF_VALID;
        vtepif_val_filter->portmap.valid[UPLL_IDX_VLAN_ID_PM] = UNC_VF_VALID;
        uuu::upll_strncpy(vtepif_val_filter->portmap.logical_port_id,
                          vtepif_val->portmap.logical_port_id,
                          kMaxLenLogicalPortId+1);
        vtepif_val_filter->portmap.vlan_id = vtepif_val->portmap.vlan_id;
        ConfigKeyVal *vtepif_ckv = new ConfigKeyVal(UNC_KT_VTEP_IF,
                                   IpctSt::kIpcStKeyVtepIf, vtep_key_if, NULL);
        vtepif_ckv->AppendCfgVal(IpctSt::kIpcStValVtepIf, vtepif_val_filter);

        SET_USER_DATA(vtepif_ckv, ikey);
        // Read from the DB
        DbSubOp dbop = { kOpReadMultiple, kOpMatchCtrlr | kOpMatchDomain,
                                                          kOpInOutFlag};
        result_code = ReadConfigDB(vtepif_ckv, UPLL_DT_CANDIDATE, UNC_OP_READ,
                                   dbop, dmi, MAINTBL);
        if (result_code == UPLL_RC_SUCCESS) {
        // check different interface has the same port and lan id
          ConfigKeyVal *tmp = vtepif_ckv;
          while (tmp) {
            if (!memcmp((ikey->get_key()), (tmp->get_key()),
                                            sizeof(key_vtep_if))) {
              UPLL_LOG_TRACE("Looking on the Same key");
            } else {
              UPLL_LOG_DEBUG("Same port Id and VlanId is used on different"
                              " Vtep Interface within same VTN/Controller");
              delete vtepif_ckv;
              vtepif_ckv = tmp = NULL;
              return UPLL_RC_ERR_CFG_SEMANTIC;
            }
            tmp = tmp->get_next_cfg_key_val();
          }
        } else if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
          result_code = UPLL_RC_SUCCESS;
        } else if (result_code != UPLL_RC_SUCCESS) {
          delete vtepif_ckv;
          vtepif_ckv = NULL;
          return result_code;
        }
        delete vtepif_ckv;
        vtepif_ckv = NULL;
      }
    }
    UPLL_LOG_TRACE("PortId and VlanId is not used on any Vtunnel Interface");
    return result_code;
  }
#endif
  bool VtepIfMoMgr::IsValidKey(void *key, uint64_t index, MoMgrTables tbl) {
    UPLL_FUNC_TRACE;
    key_vtep_if *if_key = reinterpret_cast<key_vtep_if *>(key);
    upll_rc_t ret_val = UPLL_RC_SUCCESS;
    switch (index) {
      case uudst::vtep_interface::kDbiVtnName:
        ret_val = ValidateKey(reinterpret_cast<char *>
                             (if_key->vtep_key.vtn_key.vtn_name),
                             kMinLenVtnName, kMaxLenVtnName);
        if (ret_val != UPLL_RC_SUCCESS) {
          UPLL_LOG_TRACE("VTN Name is not valid(%d)", ret_val);
          return false;
        }
        break;
      case uudst::vtep_interface::kDbiVtepName:
        ret_val = ValidateKey(reinterpret_cast<char *>
                              (if_key->vtep_key.vtep_name),
                              kMinLenVnodeName, kMaxLenVnodeName);
        if (ret_val != UPLL_RC_SUCCESS) {
          UPLL_LOG_TRACE("Vtep Name is not valid(%d)", ret_val);
          return false;
        }
        break;
      case uudst::vtep_interface::kDbiIfName:
        ret_val = ValidateKey(reinterpret_cast<char *>(if_key->if_name),
                              kMinLenInterfaceName,
                              kMaxLenInterfaceName);
        if (ret_val != UPLL_RC_SUCCESS) {
          UPLL_LOG_TRACE("Vtep IF Name is not valid(%d)", ret_val);
          return false;
        }
        break;
      default:
        UPLL_LOG_TRACE("Wrong Index");
        return false;
    }
    return true;
  }

  upll_rc_t VtepIfMoMgr::GetValid(void *val,  uint64_t indx, uint8_t *&valid,
      upll_keytype_datatype_t dt_type, MoMgrTables tbl ) {
    UPLL_FUNC_TRACE;
    if (val == NULL) return UPLL_RC_ERR_GENERIC;
    if (tbl == MAINTBL) {
      switch (indx) {
        case  uudst::vtep_interface::kDbiOperStatus:
          valid = &(reinterpret_cast<val_vtep_if_st *>(val))->
              valid[UPLL_IDX_IF_OPER_STATUS_VTEPIS];
          break;
        case  uudst::vtep_interface::kDbiDownCount:
          valid = NULL;
          break;
        case  uudst::vtep_interface::kDbiAdminStatus:
          valid = &(reinterpret_cast<val_vtep_if *>(val))->
            valid[UPLL_IDX_ADMIN_ST_VTEPI];
          break;
        case  uudst::vtep_interface::kDbiDesc:
          valid = &(reinterpret_cast<val_vtep_if *>(val))->
            valid[UPLL_IDX_DESC_VTEPI];
          break;
        case uudst::vtep_interface::kDbiValidPortMap:
          valid = &(reinterpret_cast<val_vtep_if *>(val))->
            valid[UPLL_IDX_PORT_MAP_VTEPI];
          break;
        case uudst::vtep_interface::kDbiLogicalPortId:
          valid = &(reinterpret_cast<val_vtep_if *>(val))->portmap.
            valid[UPLL_IDX_LOGICAL_PORT_ID_PM];
          break;
        case uudst::vtep_interface::kDbiVlanId:
          valid = &(reinterpret_cast<val_vtep_if *>(val))->portmap.
            valid[UPLL_IDX_VLAN_ID_PM];
          break;
        case uudst::vtep_interface::kDbiTagged:
          valid = &(reinterpret_cast<val_vtep_if *>(val))->portmap.
            valid[UPLL_IDX_TAGGED_PM];
          break;
        default:
          return UPLL_RC_ERR_GENERIC;
      }
    }
    return UPLL_RC_SUCCESS;
  }

  upll_rc_t VtepIfMoMgr::GetChildConfigKey(ConfigKeyVal *&okey,
      ConfigKeyVal *parent_key) {
    UPLL_FUNC_TRACE;
    bool cfgval_ctrlr = false;
    upll_rc_t result_code = UPLL_RC_SUCCESS;
    key_vtep_if *vtep_key_if = static_cast<key_vtep_if *>
      (ConfigKeyVal::Malloc(sizeof(key_vtep_if)));
    if (vtep_key_if == NULL) return UPLL_RC_ERR_GENERIC;
    void *pkey;
    if (parent_key == NULL) {
      if (okey) delete okey;
      okey = new ConfigKeyVal(UNC_KT_VTEP_IF,
                              IpctSt::kIpcStKeyVtepIf,
                              vtep_key_if, NULL);
      return UPLL_RC_SUCCESS;
    } else {
      pkey = parent_key->get_key();
    }
    if (!pkey) {
      FREE_IF_NOT_NULL(vtep_key_if);
      return UPLL_RC_ERR_GENERIC;
    }
    if (okey && (okey->get_key())) {
      FREE_IF_NOT_NULL(vtep_key_if);
      if (okey->get_key_type() != UNC_KT_VTEP_IF)
        return UPLL_RC_ERR_GENERIC;
      vtep_key_if = reinterpret_cast<key_vtep_if *>(okey->get_key());
    } else {
      okey = new ConfigKeyVal(UNC_KT_VTEP_IF, IpctSt::kIpcStKeyVtepIf,
                              vtep_key_if, NULL);
      if (okey == NULL) {
        FREE_IF_NOT_NULL(vtep_key_if);
        return UPLL_RC_ERR_GENERIC;
      }
    }
    unc_key_type_t keytype = parent_key->get_key_type();
    switch (keytype) {
      case UNC_KT_VTN:
        uuu::upll_strncpy(vtep_key_if->vtep_key.vtn_key.vtn_name,
                          static_cast<key_vtn *>(pkey)->vtn_name,
                          (kMaxLenVtnName+1));
        *(vtep_key_if->vtep_key.vtep_name) = *"";
        break;
      case UNC_KT_VTEP:
        uuu::upll_strncpy(vtep_key_if->vtep_key.vtn_key.vtn_name,
                          static_cast<key_vtep *>(pkey)->vtn_key.vtn_name,
                          (kMaxLenVtnName+1));
        uuu::upll_strncpy(vtep_key_if->vtep_key.vtep_name,
                          static_cast<key_vtep *>(pkey)->vtep_name,
                          (kMaxLenVnodeName+1));
        break;
      case UNC_KT_VTEP_IF:
        uuu::upll_strncpy(vtep_key_if->vtep_key.vtn_key.vtn_name,
                          static_cast<key_vtep_if *>
                          (pkey)->vtep_key.vtn_key.vtn_name,
                          (kMaxLenVtnName+1));
        uuu::upll_strncpy(vtep_key_if->vtep_key.vtep_name,
                          static_cast<key_vtep_if *>(pkey)->vtep_key.vtep_name,
                          (kMaxLenVnodeName+1));
        uuu::upll_strncpy(vtep_key_if->if_name,
                          static_cast<key_vtep_if *>(pkey)->if_name,
                          (kMaxLenInterfaceName+1));
        break;
      case UNC_KT_VLINK:
        {
          uint8_t *vnode_name, *if_name;
          val_vlink *vlink_val = reinterpret_cast<val_vlink *>
                                 (GetVal(parent_key));
          if (!vlink_val) {
            free(vtep_key_if);
            return UPLL_RC_ERR_GENERIC;
          }
          uint8_t flags = 0;
          GET_USER_DATA_FLAGS(parent_key->get_cfg_val(), flags);
          flags &=  VLINK_FLAG_NODE_POS;
          UPLL_LOG_DEBUG("Vlink flag node position %d", flags);
          if (flags == kVlinkVnode1) {
            vnode_name =  vlink_val->vnode1_name;
            if_name = vlink_val->vnode1_ifname;
          } else {
            cfgval_ctrlr = true;
            vnode_name =  vlink_val->vnode2_name;
            if_name = vlink_val->vnode2_ifname;
          }
          uuu::upll_strncpy(vtep_key_if->vtep_key.vtn_key.vtn_name,
                            static_cast<key_vlink *>(pkey)->vtn_key.vtn_name,
                            (kMaxLenVtnName+1));
          if (vnode_name)
            uuu::upll_strncpy(vtep_key_if->vtep_key.vtep_name, vnode_name,
                              (kMaxLenVnodeName+1));
          if (if_name)
            uuu::upll_strncpy(vtep_key_if->if_name, if_name,
                              (kMaxLenInterfaceName+1));
        }
      default:
        break;
    }
    if (cfgval_ctrlr) {
      SET_USER_DATA(okey, parent_key->get_cfg_val());
    } else {
      SET_USER_DATA(okey, parent_key);
    }
    return result_code;
  }

  upll_rc_t VtepIfMoMgr::GetParentConfigKey(ConfigKeyVal *&okey,
      ConfigKeyVal *ikey ) {
    UPLL_FUNC_TRACE;
    upll_rc_t result_code = UPLL_RC_SUCCESS;
    key_vtep_if *pkey = reinterpret_cast<key_vtep_if *>
                        ((ikey)?ikey->get_key():NULL);
    if (!pkey) return UPLL_RC_ERR_GENERIC;

    unc_key_type_t ikey_type = ikey->get_key_type();
    if (ikey_type != UNC_KT_VTEP_IF)
      return UPLL_RC_ERR_GENERIC;
    key_vtep *vtep_key = reinterpret_cast<key_vtep *>
                         (ConfigKeyVal::Malloc(sizeof(key_vtep)));
    if (!vtep_key) return UPLL_RC_ERR_GENERIC;
    uuu::upll_strncpy(vtep_key->vtn_key.vtn_name,
        static_cast<key_vtep_if *>(pkey)->vtep_key.vtn_key.vtn_name,
        (kMaxLenVtnName+1));
    uuu::upll_strncpy(vtep_key->vtep_name,
        static_cast<key_vtep_if *>(pkey)->vtep_key.vtep_name,
        (kMaxLenVnodeName+1));
    if (okey) delete okey;
    okey = new ConfigKeyVal(UNC_KT_VTEP, IpctSt::kIpcStKeyVtep,
                            vtep_key, NULL);
    if (okey == NULL) {
      FREE_IF_NOT_NULL(vtep_key);
      result_code = UPLL_RC_ERR_GENERIC;
    } else {
      SET_USER_DATA(okey, ikey);
    }
    return result_code;
  }


  upll_rc_t VtepIfMoMgr::AllocVal(ConfigVal *&ck_val,
      upll_keytype_datatype_t dt_type, MoMgrTables tbl) {
    UPLL_FUNC_TRACE;
    void *val;
    if (ck_val != NULL) return UPLL_RC_ERR_GENERIC;
    switch (tbl) {
      case MAINTBL:
        val = reinterpret_cast<void *>
              (ConfigKeyVal::Malloc(sizeof(val_vtep_if)));
        if (!val) return UPLL_RC_ERR_GENERIC;
        ck_val = new ConfigVal(IpctSt::kIpcStValVtepIf, val);
        if (!ck_val) {
          FREE_IF_NOT_NULL(reinterpret_cast<val_vtep_if *>(val));
          return UPLL_RC_ERR_GENERIC;
        }
        if (dt_type == UPLL_DT_STATE) {
          val = reinterpret_cast<void *>
                (ConfigKeyVal::Malloc(sizeof(val_db_vtep_if_st)));
          if (!val) {
            delete ck_val;
            return UPLL_RC_ERR_GENERIC;
          }
          ConfigVal *ck_nxtval = new ConfigVal(IpctSt::kIpcStValVtepIfSt, val);
          if (!ck_nxtval) {
            delete ck_val;
            FREE_IF_NOT_NULL(reinterpret_cast<val_db_vtep_if_st *>(val));
            return UPLL_RC_ERR_GENERIC;
          }
          ck_val->AppendCfgVal(ck_nxtval);
        }
        break;
      default:
        val = NULL;
    }
    return UPLL_RC_SUCCESS;
  }

  upll_rc_t VtepIfMoMgr::DupConfigKeyVal(ConfigKeyVal *&okey,
      ConfigKeyVal *&req, MoMgrTables tbl) {
    UPLL_FUNC_TRACE;
    if (req == NULL) return UPLL_RC_ERR_GENERIC;
    if (okey != NULL) return UPLL_RC_ERR_GENERIC;
    if (req->get_key_type() != UNC_KT_VTEP_IF)
      return UPLL_RC_ERR_GENERIC;
    ConfigVal *tmp1 = NULL, *tmp = (req)->get_cfg_val();

    if (tmp) {
      if (tbl == MAINTBL) {
        val_vtep_if *ival = static_cast<val_vtep_if *>(GetVal(req));
        val_vtep_if *vtep_val_if = static_cast<val_vtep_if *>
                                   (ConfigKeyVal::Malloc(sizeof(val_vtep_if)));
        if (!vtep_val_if) return UPLL_RC_ERR_GENERIC;
        memcpy(vtep_val_if, ival, sizeof(val_vtep_if));
        tmp1 = new ConfigVal(IpctSt::kIpcStValVtepIf, vtep_val_if);
        if (!tmp1) {
          FREE_IF_NOT_NULL(vtep_val_if);
          return UPLL_RC_ERR_GENERIC;
        }
      }
      tmp = tmp->get_next_cfg_val();
    };
    if (tmp) {
      if (tbl == MAINTBL) {
        val_db_vtep_if_st *ival = static_cast<val_db_vtep_if_st *>
          (tmp->get_val());
        val_db_vtep_if_st *val_vtep_if = static_cast<val_db_vtep_if_st *>
          (ConfigKeyVal::Malloc(sizeof(val_db_vtep_if_st)));
        if (!val_vtep_if) {
          delete tmp1;
          return UPLL_RC_ERR_GENERIC;
        }
        memcpy(val_vtep_if, ival, sizeof(val_vtep_if_st));
        ConfigVal *tmp2 = new ConfigVal(IpctSt::kIpcStValVtepIfSt, val_vtep_if);
        if (!tmp2) {
          delete tmp1;
          FREE_IF_NOT_NULL(val_vtep_if);
          return UPLL_RC_ERR_GENERIC;
        }
        tmp1->AppendCfgVal(tmp2);
      }
    };
    void *tkey = (req != NULL)?(req)->get_key():NULL;
    key_vtep_if *ikey = static_cast<key_vtep_if *>(tkey);
    key_vtep_if *vtep_if_key = static_cast<key_vtep_if *>
      (ConfigKeyVal::Malloc(sizeof(key_vtep_if)));
    if (!vtep_if_key) {
      if (tmp1) delete tmp1;
      return UPLL_RC_ERR_GENERIC;
    }
    memcpy(vtep_if_key, ikey, sizeof(key_vtep_if));
    okey = new ConfigKeyVal(UNC_KT_VTEP_IF, IpctSt::kIpcStKeyVtepIf,
                            vtep_if_key, tmp1);
    if (okey) {
      SET_USER_DATA(okey, req);
    } else {
      if (tmp1) delete tmp1;
      FREE_IF_NOT_NULL(vtep_if_key);
      return UPLL_RC_ERR_GENERIC;
    }
    return UPLL_RC_SUCCESS;
  }

  upll_rc_t VtepIfMoMgr::UpdateConfigVal(ConfigKeyVal *ikey,
                                      upll_keytype_datatype_t datatype,
                                      DalDmlIntf *dmi) {
    UPLL_FUNC_TRACE;
    upll_rc_t result_code = UPLL_RC_SUCCESS;
    ConfigKeyVal *okey = NULL;
    uint8_t flag = 0;
    if (!ikey || !dmi) {
      UPLL_LOG_DEBUG("Invalid Input");
      return UPLL_RC_ERR_GENERIC;
    }
    val_vtep_if *vtepif_val = static_cast<val_vtep_if *>(GetVal(ikey));
    if (!vtepif_val) {
      UPLL_LOG_DEBUG("Value Structure is Null");
      return UPLL_RC_ERR_GENERIC;
    }
    result_code = GetChildConfigKey(okey, ikey);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("GetChildConfigKey Failure %d", result_code);
      return result_code;
    }
    DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutFlag };
    result_code = ReadConfigDB(okey, datatype, UNC_OP_READ, dbop,
                               dmi, MAINTBL);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Read failed %d", result_code);
      DELETE_IF_NOT_NULL(okey);
      return result_code;
    }
    GET_USER_DATA_FLAGS(okey, flag);
    if (vtepif_val->valid[UPLL_IDX_PORT_MAP_VTEPI] == UNC_VF_VALID ||
        vtepif_val->valid[UPLL_IDX_PORT_MAP_VTEPI] == UNC_VF_VALID_NO_VALUE) {
      if (flag & VIF_TYPE) {
        DELETE_IF_NOT_NULL(okey);
        UPLL_LOG_DEBUG("Interface is linked/bounded with Vlink."
                        " Could not update Portmap");
        return UPLL_RC_ERR_CFG_SEMANTIC;
      }
    }
    if (vtepif_val->valid[UPLL_IDX_PORT_MAP_VTEPI] == UNC_VF_VALID_NO_VALUE) {
      vtepif_val->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] =
                                                 UNC_VF_VALID_NO_VALUE;
      vtepif_val->portmap.valid[UPLL_IDX_VLAN_ID_PM] = UNC_VF_VALID_NO_VALUE;
      vtepif_val->portmap.valid[UPLL_IDX_TAGGED_PM] = UNC_VF_VALID_NO_VALUE;
      uuu::upll_strncpy(vtepif_val->portmap.logical_port_id, "\0", 1);
      vtepif_val->portmap.vlan_id = 0;
      vtepif_val->portmap.tagged = UPLL_VLAN_UNTAGGED;
    }
    DELETE_IF_NOT_NULL(okey);
    return result_code;
  }

  upll_rc_t VtepIfMoMgr::AdaptValToVtnService(ConfigKeyVal *ikey,
                                              AdaptType adapt_type) {
    UPLL_FUNC_TRACE;
    if (!ikey) {
      UPLL_LOG_DEBUG("Invalid ikey");
      return UPLL_RC_ERR_GENERIC;
    }
  key_vtep_if *vtepif_key = reinterpret_cast<key_vtep_if*>(ikey->get_key());
    while (ikey) {
      ConfigVal *cval = ikey->get_cfg_val();
      if (!cval) {
        UPLL_LOG_DEBUG("Config Val is Null");
        return UPLL_RC_ERR_GENERIC;
      }
      while (cval) {
        if (IpctSt::kIpcStValVtepIf == cval->get_st_num()) {
           // set admin status to valid no value
          val_vtep_if *vtepif_val = reinterpret_cast<val_vtep_if *>
                                          (GetVal(ikey));
          if (vtepif_val->valid[UPLL_IDX_ADMIN_ST_VTEPI] == UNC_VF_INVALID)
            vtepif_val->valid[UPLL_IDX_ADMIN_ST_VTEPI] = UNC_VF_VALID_NO_VALUE;

          uint8_t vlink_flag = 0;
          GET_USER_DATA_FLAGS(ikey, vlink_flag);
          UPLL_LOG_DEBUG("Interface type %d", vlink_flag);
          if (vlink_flag & VIF_TYPE)
            vtepif_val->valid[UPLL_IDX_PORT_MAP_VTEPI] = UNC_VF_INVALID;
        }
        if (IpctSt::kIpcStValVtepIfSt == cval->get_st_num()) {
          controller_domain ctrlr_dom = {NULL, NULL};
          GET_USER_DATA_CTRLR_DOMAIN(ikey, ctrlr_dom);
          CheckOperStatus<val_vtep_if_st>(vtepif_key->vtep_key.vtn_key.vtn_name,
                                          cval, UNC_KT_VTEP_IF, ctrlr_dom);
        }
        cval = cval->get_next_cfg_val();
      }
      if (adapt_type == ADAPT_ONE)
        break;
      ikey = ikey->get_next_cfg_key_val();
    }
    return UPLL_RC_SUCCESS;
  }

  bool VtepIfMoMgr::FilterAttributes(void *&val1, void *val2,
                                     bool copy_to_running,
                                     unc_keytype_operation_t op) {
    UPLL_FUNC_TRACE;
    val_vtep_if_t *val_vtep_if1 = reinterpret_cast<val_vtep_if_t *>(val1);
    /* No need to configure description in controller. */
    val_vtep_if1->valid[UPLL_IDX_DESC_VTEPI] = UNC_VF_INVALID;
    if (op != UNC_OP_CREATE)
      return CompareValidValue(val1, val2, copy_to_running);
    return false;
  }

  bool VtepIfMoMgr::CompareValidValue(void *&val1, void *val2,
                                      bool copy_to_running) {
    UPLL_FUNC_TRACE;
    bool invalid_attr = true;
    val_vtep_if_t *val_vtep_if1 = reinterpret_cast<val_vtep_if_t *>(val1);
    val_vtep_if_t *val_vtep_if2 = reinterpret_cast<val_vtep_if_t *>(val2);
    for (unsigned int loop = 0;
        loop < sizeof(val_vtep_if1->valid)/sizeof(uint8_t); ++loop ) {
      if ( UNC_VF_INVALID == val_vtep_if1->valid[loop] &&
          UNC_VF_VALID == val_vtep_if2->valid[loop])
        val_vtep_if1->valid[loop] = UNC_VF_VALID_NO_VALUE;
    }
    if (UNC_VF_VALID == val_vtep_if1->valid[UPLL_IDX_DESC_VTEPI]
        && UNC_VF_VALID == val_vtep_if2->valid[UPLL_IDX_DESC_VTEPI]) {
      if (!strcmp(reinterpret_cast<char*>(val_vtep_if1->description),
            reinterpret_cast<char*>(val_vtep_if2->description)))
        val_vtep_if1->valid[UPLL_IDX_DESC_VTEPI] = UNC_VF_INVALID;
    }
    if (UNC_VF_INVALID != val_vtep_if1->valid[UPLL_IDX_ADMIN_ST_VTEPI]
        && UNC_VF_INVALID != val_vtep_if2->valid[UPLL_IDX_ADMIN_ST_VTEPI]) {
      if (val_vtep_if1->admin_status == val_vtep_if2->admin_status)
        val_vtep_if1->valid[UPLL_IDX_ADMIN_ST_VTEPI] = UNC_VF_INVALID;
    }
    for (unsigned int loop = 0;
        loop < sizeof(val_vtep_if1->portmap.valid)/sizeof(uint8_t); ++loop ) {
      if (UNC_VF_INVALID == val_vtep_if1->portmap.valid[loop]
          && UNC_VF_VALID == val_vtep_if2->portmap.valid[loop])
        val_vtep_if1->portmap.valid[loop] = UNC_VF_VALID_NO_VALUE;
    }
    if (val_vtep_if1->valid[UPLL_IDX_PORT_MAP_VTEPI] == UNC_VF_VALID
        && val_vtep_if2->valid[UPLL_IDX_PORT_MAP_VTEPI] == UNC_VF_VALID) {
      if (memcmp(&(val_vtep_if1->portmap), &(val_vtep_if2->portmap),
            sizeof(val_port_map_t))) {
        if (val_vtep_if1->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] ==
            UNC_VF_VALID
            && val_vtep_if2->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM]
            == UNC_VF_VALID) {
          if (!strcmp(reinterpret_cast<char *>
              (val_vtep_if1->portmap.logical_port_id),
              reinterpret_cast<char *>(val_vtep_if2->portmap.logical_port_id)))
            val_vtep_if1->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] =
              UNC_VF_INVALID;
        }
        if ((val_vtep_if1->portmap.valid[UPLL_IDX_VLAN_ID_PM] ==
            val_vtep_if2->portmap.valid[UPLL_IDX_VLAN_ID_PM])
            && val_vtep_if2->portmap.valid[UPLL_IDX_VLAN_ID_PM] !=
               UNC_VF_INVALID) {
          if (val_vtep_if1->portmap.vlan_id == val_vtep_if2->portmap.vlan_id)
            val_vtep_if1->portmap.valid[UPLL_IDX_VLAN_ID_PM] =
               (copy_to_running)?UNC_VF_INVALID:UNC_VF_VALUE_NOT_MODIFIED;
        }
        if ((val_vtep_if1->portmap.valid[UPLL_IDX_TAGGED_PM] ==
            val_vtep_if2->portmap.valid[UPLL_IDX_TAGGED_PM])
            && val_vtep_if2->portmap.valid[UPLL_IDX_TAGGED_PM] !=
               UNC_VF_INVALID) {
          if (val_vtep_if1->portmap.tagged == val_vtep_if2->portmap.tagged)
            val_vtep_if1->portmap.valid[UPLL_IDX_TAGGED_PM] =
               (copy_to_running)?UNC_VF_INVALID:UNC_VF_VALUE_NOT_MODIFIED;
        }
      } else {
        val_vtep_if1->valid[UPLL_IDX_PORT_MAP_VTEPI] = UNC_VF_INVALID;
        val_vtep_if1->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] =
            UNC_VF_INVALID;
        val_vtep_if1->portmap.valid[UPLL_IDX_VLAN_ID_PM] = UNC_VF_INVALID;
        val_vtep_if1->portmap.valid[UPLL_IDX_TAGGED_PM] = UNC_VF_INVALID;
      }
    }
    if (!copy_to_running)
      val_vtep_if1->valid[UPLL_IDX_DESC_VTEPI] = UNC_VF_INVALID;
    for (unsigned int loop = 0;
        loop < sizeof(val_vtep_if1->valid) / sizeof(uint8_t); ++loop) {
      if ((UNC_VF_VALID == (uint8_t) val_vtep_if1->valid[loop]) ||
          (UNC_VF_VALID_NO_VALUE == (uint8_t) val_vtep_if1->valid[loop])) {
        if (loop == UPLL_IDX_PORT_MAP_VTEPI) {
          for (unsigned int i = 0;
            i < sizeof(val_vtep_if1->portmap.valid) / sizeof(uint8_t); ++i) {
            if ((UNC_VF_VALID == (uint8_t) val_vtep_if1->portmap.valid[i]) ||
                (UNC_VF_VALID_NO_VALUE ==
                 (uint8_t) val_vtep_if1->portmap.valid[i])) {
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

  upll_rc_t VtepIfMoMgr::UpdateConfigStatus(ConfigKeyVal *vtepif_keyval,
                                            unc_keytype_operation_t op,
                                            uint32_t driver_result,
                                            ConfigKeyVal *upd_key,
                                            DalDmlIntf *dmi,
                                            ConfigKeyVal *ctrlr_key) {
    UPLL_FUNC_TRACE;
    val_vtep_if_t *vtepif_val = static_cast<val_vtep_if_t *>
                                           (GetVal(vtepif_keyval));
    if (vtepif_val == NULL) return UPLL_RC_ERR_GENERIC;
    unc_keytype_configstatus_t cs_status = (driver_result == UPLL_RC_SUCCESS)?
                                            UNC_CS_APPLIED:UNC_CS_NOT_APPLIED;
    UPLL_LOG_TRACE("DriverResult %d, "
                   "ConfigStatus %d", driver_result, cs_status);
    UPLL_LOG_TRACE("%s", (vtepif_keyval->ToStrAll()).c_str());
    val_vtep_if_t *vtep_if_running = static_cast<val_vtep_if_t *>
                                         (GetVal(upd_key));
    if (op == UNC_OP_CREATE) {
      vtepif_val->cs_row_status = cs_status;
      val_db_vtep_if_st *vtep_db_valst = static_cast<val_db_vtep_if_st *>
        (ConfigKeyVal::Malloc(sizeof(val_db_vtep_if_st)));
      if (vtep_db_valst == NULL) return UPLL_RC_ERR_GENERIC;
      vtepif_keyval->AppendCfgVal(IpctSt::kIpcStValVtepIfSt, vtep_db_valst);
#if 0
      upll_rc_t result_code = InitOperStatus<val_vtep_if_st, val_db_vtep_if_st>
                                 (vtepif_keyval,
                                  vtepif_val->valid[UPLL_IDX_ADMIN_ST_VTEPI],
                                  vtepif_val->admin_status,
                                  vtepif_val->valid[UPLL_IDX_PORT_MAP_VTEPI],
                                  &vtepif_val->portmap);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Error settiing oper status");
        return UPLL_RC_ERR_GENERIC;
      }
#else
      val_vtep_if_st *vnif_st = &vtep_db_valst->vtep_if_val_st;
      if (!vnif_st) {
        UPLL_LOG_DEBUG("Returning error\n");
        return UPLL_RC_ERR_GENERIC;
      }
      vnif_st->oper_status = (driver_result == UPLL_RC_ERR_CTR_DISCONNECTED)?
                              UPLL_OPER_STATUS_UNKNOWN:
                              UPLL_OPER_STATUS_UP;
      vnif_st->valid[UPLL_IDX_IF_OPER_STATUS_VTEPIS] = UNC_VF_VALID;
#endif
      vtep_db_valst->down_count = 0;
    } else if (op == UNC_OP_UPDATE) {
      void *val = reinterpret_cast<void *>(vtepif_val);
      CompareValidValue(val, GetVal(upd_key), true);
      UPLL_LOG_TRACE("%s", (upd_key->ToStrAll()).c_str());
      vtepif_val->cs_row_status =
                  vtep_if_running->cs_row_status;
    } else {
      return UPLL_RC_ERR_GENERIC;
    }
    for (unsigned int loop = 0; loop <
        sizeof(vtepif_val->valid) / sizeof(vtepif_val->valid[0]);
        ++loop) {
      if ((UNC_VF_VALID == (uint8_t) vtepif_val->valid[loop])
          || (UNC_VF_VALID_NO_VALUE == (uint8_t) vtepif_val->valid[loop])) {
        // Description is set to APPLIED
        if (loop == UPLL_IDX_DESC_VTEPI)
          vtepif_val->cs_attr[loop] = UNC_CS_APPLIED;
        else
          vtepif_val->cs_attr[loop] = cs_status;
      } else if ((UNC_VF_INVALID == vtepif_val->valid[loop]) &&
               (UNC_OP_CREATE == op)) {
        vtepif_val->cs_attr[loop] = UNC_CS_APPLIED;
      } else if ((UNC_VF_INVALID == vtepif_val->valid[loop]) &&
             (UNC_OP_UPDATE == op)) {
          vtepif_val->cs_attr[loop] =
                     vtep_if_running->cs_attr[loop];
      }
    }
    for (unsigned int loop = 0;
         loop < sizeof(vtepif_val->portmap.valid)/
                sizeof(vtepif_val->portmap.valid[0]); ++loop ) {
      if ((UNC_VF_VALID == vtepif_val->portmap.valid[loop])
       || (UNC_VF_VALID_NO_VALUE == vtepif_val->portmap.valid[loop]))
         vtepif_val->portmap.cs_attr[loop] = cs_status;
      else if ((UNC_VF_INVALID == vtepif_val->portmap.valid[loop]) &&
               (UNC_OP_CREATE == op))
        vtepif_val->portmap.cs_attr[loop] = UNC_CS_APPLIED;
      else if ((UNC_VF_INVALID == vtepif_val->portmap.valid[loop]) &&
               (UNC_OP_UPDATE == op))
        vtepif_val->portmap.cs_attr[loop] =
                   vtep_if_running->portmap.cs_attr[loop];
      }
    return UPLL_RC_SUCCESS;
  }

  upll_rc_t VtepIfMoMgr::UpdateAuditConfigStatus(
      unc_keytype_configstatus_t cs_status,
      uuc::UpdateCtrlrPhase phase,
      ConfigKeyVal *&ckv_running,
      DalDmlIntf *dmi) {
    UPLL_FUNC_TRACE;
    upll_rc_t result_code = UPLL_RC_SUCCESS;
    val_vtep_if_t *val;
    val = (ckv_running != NULL)?reinterpret_cast<val_vtep_if_t *>
                                (GetVal(ckv_running)):NULL;
    if (NULL == val) {
      return UPLL_RC_ERR_GENERIC;
    }
    if (uuc::kUpllUcpCreate == phase )
      val->cs_row_status = cs_status;
    if ((uuc::kUpllUcpUpdate == phase) &&
           (val->cs_row_status == UNC_CS_INVALID ||
            val->cs_row_status == UNC_CS_NOT_APPLIED))
      val->cs_row_status = cs_status;
    for ( unsigned int loop = 0;
        loop < sizeof(val->valid)/sizeof(uint8_t); ++loop ) {
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

  upll_rc_t VtepIfMoMgr::ValidateMessage(IpcReqRespHeader *req,
      ConfigKeyVal *ikey) {
    UPLL_FUNC_TRACE;
    upll_rc_t ret_val = UPLL_RC_SUCCESS;
    if (!ikey || !req || !(ikey->get_key())) {
      UPLL_LOG_DEBUG("IpcReqRespHeader or ConfigKeyVal is Null");
      return UPLL_RC_ERR_BAD_REQUEST;
    }
    unc_key_type_t keytype = ikey->get_key_type();
    if (UNC_KT_VTEP_IF != keytype) {
      UPLL_LOG_DEBUG("Invalid keytype. Keytype- %d", keytype);
      return UPLL_RC_ERR_BAD_REQUEST;
    }
    if (ikey->get_st_num() != IpctSt::kIpcStKeyVtepIf) {
      UPLL_LOG_DEBUG("Invalid struct received.Expected struct-kIpcStKeyVtepIf,"
          "received struct -%s ", reinterpret_cast<const char *>
          (IpctSt::GetIpcStdef(ikey->get_st_num())));
      return UPLL_RC_ERR_BAD_REQUEST;
    }
    key_vtep_if_t *key_vtep_if = reinterpret_cast <key_vtep_if_t *>
      (ikey->get_key());

    uint32_t dt_type = req->datatype;
    uint32_t operation = req->operation;
    uint32_t option1 = req->option1;
    uint32_t option2 = req->option2;

    ret_val = ValidateVTepIfKey(key_vtep_if, operation);
    if (ret_val != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Key struct Validation failed for VTEP_IF");
      return UPLL_RC_ERR_CFG_SYNTAX;
    } else {
      if ((operation == UNC_OP_CREATE) && (dt_type == UPLL_DT_CANDIDATE)) {
        val_vtep_if_t *val_vtep_if = NULL;
        if ((ikey->get_cfg_val()) &&
            ((ikey->get_cfg_val())->get_st_num() == IpctSt::kIpcStValVtepIf)) {
          val_vtep_if = reinterpret_cast <val_vtep_if_t *>
            (ikey->get_cfg_val()->get_val());
        }
        if (val_vtep_if != NULL) {
          ret_val = ValidateVTepIfValue(val_vtep_if, operation);
          if (ret_val != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("Val struct Validation failed for CREATE op");
            return UPLL_RC_ERR_CFG_SYNTAX;
          }
          return UPLL_RC_SUCCESS;
        } else {
          UPLL_LOG_TRACE("Value structure is an optional for CREATE op");
          return UPLL_RC_SUCCESS;
        }
      } else if ((operation == UNC_OP_UPDATE) &&
                 (dt_type == UPLL_DT_CANDIDATE)) {
        val_vtep_if_t *val_vtep_if = NULL;
        if ((ikey->get_cfg_val()) &&
            ((ikey->get_cfg_val())->get_st_num() == IpctSt::kIpcStValVtepIf)) {
          val_vtep_if = reinterpret_cast <val_vtep_if_t *>
            (ikey->get_cfg_val()->get_val());
        }
        if (val_vtep_if != NULL) {
          ret_val = ValidateVTepIfValue(val_vtep_if, operation);
          if (ret_val != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("Val struct Validation failed for UPDATE op");
            return UPLL_RC_ERR_CFG_SYNTAX;
          }
          return UPLL_RC_SUCCESS;
        } else {
          UPLL_LOG_DEBUG("Value structure mandatory for UPDATE op");
          return UPLL_RC_ERR_BAD_REQUEST;
        }
      } else if (((operation == UNC_OP_READ) ||
            (operation == UNC_OP_READ_SIBLING) ||
            (operation == UNC_OP_READ_SIBLING_BEGIN) ||
            (operation == UNC_OP_READ_SIBLING_COUNT)) &&
          ((dt_type == UPLL_DT_CANDIDATE) ||
           (dt_type == UPLL_DT_RUNNING) ||
           (dt_type == UPLL_DT_STARTUP) ||
           (dt_type == UPLL_DT_STATE))) {
        if (option1 == UNC_OPT1_NORMAL) {
          if (option2 == UNC_OPT2_NONE) {
            val_vtep_if_t *val_vtep_if = NULL;
            if ((ikey->get_cfg_val()) &&
                ((ikey->get_cfg_val())->get_st_num() ==
                  IpctSt::kIpcStValVtepIf)) {
              val_vtep_if = reinterpret_cast <val_vtep_if_t *>
                (ikey->get_cfg_val()->get_val());
            }
            if (val_vtep_if != NULL) {
              ret_val = ValidateVTepIfValue(val_vtep_if);
              if (ret_val != UPLL_RC_SUCCESS) {
                UPLL_LOG_DEBUG("Val struct Validation failed for READ op");
                return UPLL_RC_ERR_CFG_SYNTAX;
              }
              return UPLL_RC_SUCCESS;
            } else {
              UPLL_LOG_TRACE("Value structure is an optional for READ op");
              return UPLL_RC_SUCCESS;
            }
          } else if ((option2 == UNC_OPT2_NEIGHBOR) &&
              (dt_type == UPLL_DT_STATE) &&
              (operation == UNC_OP_READ)) {
            val_vtn_neighbor_t *val_vtn_neighbor = NULL;
            if ((ikey->get_cfg_val()) &&
                ((ikey->get_cfg_val())->get_st_num() ==
                 IpctSt::kIpcStValVtnNeighbor)) {
              val_vtn_neighbor =
                reinterpret_cast <val_vtn_neighbor_t *>
                (ikey->get_cfg_val()->get_val());
            }
            if (val_vtn_neighbor != NULL) {
              ret_val = ValidateVtnNeighborValue(val_vtn_neighbor);
              if (ret_val != UPLL_RC_SUCCESS) {
                UPLL_LOG_DEBUG("Val struct Validation failed for READ op");
                return UPLL_RC_ERR_CFG_SYNTAX;
              }
              return UPLL_RC_SUCCESS;
            } else {
              UPLL_LOG_TRACE("value structure is an optional for READ op");
              return UPLL_RC_SUCCESS;
            }
          } else {
            UPLL_LOG_TRACE("option2 is not matching");
            return UPLL_RC_ERR_INVALID_OPTION2;
          }
        } else {
          UPLL_LOG_TRACE("option1 is not matching");
          return UPLL_RC_ERR_INVALID_OPTION1;
        }
      } else if ((operation == UNC_OP_DELETE) ||
        (((operation == UNC_OP_READ_NEXT) ||
        (operation == UNC_OP_READ_BULK)) &&
        ((dt_type == UPLL_DT_CANDIDATE) ||
          (dt_type == UPLL_DT_RUNNING) ||
          (dt_type == UPLL_DT_STARTUP)))) {
        UPLL_LOG_TRACE("Value structure is none for operation type:%d",
                        operation);
        return UPLL_RC_SUCCESS;
      } else {
        UPLL_LOG_DEBUG("Invalid datatype(%d) or operation(%d)", dt_type,
            operation);
        return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT;
      }
    }
    return UPLL_RC_SUCCESS;
  }
  upll_rc_t VtepIfMoMgr::ValidateVTepIfValue(val_vtep_if_t *val_vtep_if,
      uint32_t operation) {
    UPLL_FUNC_TRACE;

    if (val_vtep_if->valid[UPLL_IDX_DESC_VTEPI] == UNC_VF_VALID) {
      if (!ValidateDesc(val_vtep_if->description,
          kMinLenDescription, kMaxLenDescription)) {
        UPLL_LOG_DEBUG("Syntax check failed. desc -%s",
                        val_vtep_if->description);
        return UPLL_RC_ERR_CFG_SYNTAX;
      }
    } else if ((val_vtep_if->valid[UPLL_IDX_DESC_VTEPI] ==
          UNC_VF_VALID_NO_VALUE) &&
        ((operation == UNC_OP_UPDATE) || (operation == UNC_OP_CREATE))) {
      val_vtep_if->description[0] = '\0';
    }
    if (val_vtep_if->valid[UPLL_IDX_ADMIN_ST_VTEPI] == UNC_VF_VALID) {
      if (!ValidateNumericRange(val_vtep_if->admin_status,
            (uint8_t) UPLL_ADMIN_ENABLE,
            (uint8_t) UPLL_ADMIN_DISABLE, true, true)) {
        UPLL_LOG_DEBUG("Syntax check failed. admin_status- %d",
            val_vtep_if->admin_status);
        return UPLL_RC_ERR_CFG_SYNTAX;
      }
    } else if ((val_vtep_if->valid[UPLL_IDX_ADMIN_ST_VTEPI] ==
          UNC_VF_VALID_NO_VALUE)
        && ((operation == UNC_OP_UPDATE) || (operation == UNC_OP_CREATE))) {
      val_vtep_if->admin_status = UPLL_ADMIN_ENABLE;
    } else if ((val_vtep_if->valid[UPLL_IDX_ADMIN_ST_VTEPI] == UNC_VF_INVALID)
                                             && (operation == UNC_OP_CREATE)) {
        val_vtep_if->valid[UPLL_IDX_ADMIN_ST_VTEPI] = UNC_VF_VALID_NO_VALUE;
        val_vtep_if->admin_status = UPLL_ADMIN_ENABLE;
    }
    if (val_vtep_if->valid[UPLL_IDX_PORT_MAP_VTEPI] == UNC_VF_VALID) {
      if (val_vtep_if->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM]
          == UNC_VF_VALID) {
        if (!ValidateLogicalPortId(
            reinterpret_cast<char *>(val_vtep_if->portmap.logical_port_id),
            kMinLenLogicalPortId, kMaxLenLogicalPortId)) {
          UPLL_LOG_DEBUG("Logical Port id syntax check failed."
              "Received Logical Port Id - %s",
              val_vtep_if->portmap.logical_port_id);
          return UPLL_RC_ERR_CFG_SYNTAX;
        }
        if (toupper(val_vtep_if->portmap.logical_port_id[0]) == 'S'
            && toupper(val_vtep_if->portmap.logical_port_id[1]) == 'W') {
          UPLL_LOG_DEBUG("Invalid logical_port_id - %s",
              val_vtep_if->portmap.logical_port_id);
          return UPLL_RC_ERR_CFG_SYNTAX;
        }
      } else if (val_vtep_if->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] ==
          UNC_VF_VALID_NO_VALUE
          && (operation == UNC_OP_CREATE || operation == UNC_OP_UPDATE)) {
        uuu::upll_strncpy(val_vtep_if->portmap.logical_port_id, " ",
            kMaxLenLogicalPortId+1);
        val_vtep_if->portmap.vlan_id = 0;
        val_vtep_if->portmap.tagged = 0;
        val_vtep_if->portmap.valid[UPLL_IDX_VLAN_ID_PM] =
                                                  UNC_VF_VALID_NO_VALUE;
        val_vtep_if->portmap.valid[UPLL_IDX_TAGGED_PM] = UNC_VF_VALID_NO_VALUE;
      } else if ((val_vtep_if->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] ==
                 UNC_VF_INVALID) && (operation == UNC_OP_CREATE)) {
        val_vtep_if->portmap.valid[UPLL_IDX_VLAN_ID_PM] = UNC_VF_INVALID;
        val_vtep_if->valid[UPLL_IDX_PORT_MAP_VTEPI] = UNC_VF_INVALID;
      }
      if (val_vtep_if->portmap.valid[UPLL_IDX_VLAN_ID_PM] == UNC_VF_VALID) {
        if ((val_vtep_if->portmap.vlan_id != 0xFFFF) &&
            !ValidateNumericRange(val_vtep_if->portmap.vlan_id,
              (uint16_t) kMinVlanId, (uint16_t) kMaxVlanId,
              true, true)) {
          UPLL_LOG_DEBUG("Vlan Id Number check failed."
              "Received vlan_id - %d",
              val_vtep_if->portmap.vlan_id);
          return UPLL_RC_ERR_CFG_SYNTAX;
        }
      } else if (val_vtep_if->portmap.valid[UPLL_IDX_VLAN_ID_PM]
          == UNC_VF_VALID_NO_VALUE
          && (operation == UNC_OP_CREATE || operation == UNC_OP_UPDATE)) {
        /* If VLAN_ID is erased, Tagged attribute also needs to be erased */
        val_vtep_if->portmap.vlan_id = 0;
        val_vtep_if->portmap.tagged = 0;
        val_vtep_if->portmap.valid[UPLL_IDX_TAGGED_PM] = UNC_VF_VALID_NO_VALUE;
      } else if ((val_vtep_if->portmap.valid[UPLL_IDX_VLAN_ID_PM] ==
                  UNC_VF_INVALID) && (operation == UNC_OP_CREATE)) {
         val_vtep_if->portmap.valid[UPLL_IDX_TAGGED_PM] = UNC_VF_INVALID;
      }
      if (val_vtep_if->portmap.valid[UPLL_IDX_TAGGED_PM] == UNC_VF_VALID) {
        if (!ValidateNumericRange((uint8_t) val_vtep_if->portmap.tagged,
              (uint8_t) UPLL_VLAN_UNTAGGED,
              (uint8_t) UPLL_VLAN_TAGGED, true, true)) {
          UPLL_LOG_DEBUG("Tagged Numeric range check failed."
              "Received Tag - %d",
              val_vtep_if->portmap.tagged);
          return UPLL_RC_ERR_CFG_SYNTAX;
        }
      } else if (((val_vtep_if->portmap.valid[UPLL_IDX_TAGGED_PM] ==
           UNC_VF_VALID_NO_VALUE) ||
           (val_vtep_if->portmap.valid[UPLL_IDX_TAGGED_PM] == UNC_VF_INVALID))
            && (operation == UNC_OP_CREATE)) {
        if (val_vtep_if->portmap.valid[UPLL_IDX_VLAN_ID_PM] == UNC_VF_VALID) {
          val_vtep_if->portmap.tagged = UPLL_VLAN_TAGGED;
          val_vtep_if->portmap.valid[UPLL_IDX_TAGGED_PM] = UNC_VF_VALID;
        }
      }
    } else if ((val_vtep_if->valid[UPLL_IDX_PORT_MAP_VTEPI] ==
                UNC_VF_VALID_NO_VALUE)
        && (operation == UNC_OP_UPDATE || operation == UNC_OP_CREATE)) {
      memset(&(val_vtep_if->portmap), 0, sizeof(val_vtep_if->portmap));
      for (unsigned int port_valid = 0;
           port_valid < sizeof(val_vtep_if->portmap.valid);
           ++port_valid) {
        val_vtep_if->portmap.valid[port_valid] = UNC_VF_VALID_NO_VALUE;
      }
    }
    return UPLL_RC_SUCCESS;
  }
  upll_rc_t VtepIfMoMgr::ValidateVTepIfKey(key_vtep_if_t *key_vtep_if,
                         uint32_t operation) {
    UPLL_FUNC_TRACE;
    upll_rc_t ret_val = UPLL_RC_SUCCESS;
    ret_val = ValidateKey(
        reinterpret_cast<char *>(key_vtep_if->vtep_key.vtn_key.vtn_name),
        kMinLenVtnName, kMaxLenVtnName);
    if (ret_val != UPLL_RC_SUCCESS) {
      UPLL_LOG_INFO("Vtn Name syntax check failed."
                    "Received VTN Name - %s",
                    key_vtep_if->vtep_key.vtn_key.vtn_name);
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
    ret_val = ValidateKey(
        reinterpret_cast<char *>(key_vtep_if->vtep_key.vtep_name),
        kMinLenVnodeName, kMaxLenVnodeName);
    if (ret_val != UPLL_RC_SUCCESS) {
      UPLL_LOG_INFO("VTEP Name syntax check failed."
                  "Received VTEP Name -%s",
                  key_vtep_if->vtep_key.vtep_name);
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
    if ((operation != UNC_OP_READ_SIBLING_BEGIN) &&
        (operation != UNC_OP_READ_SIBLING_COUNT)) {
      ret_val = ValidateKey(reinterpret_cast<char *> (key_vtep_if->if_name),
          kMinLenInterfaceName, kMaxLenInterfaceName);
      if (ret_val != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Syntax check failed.if_name(%s)", key_vtep_if->if_name);
        return ret_val;
      }
    } else {
      UPLL_LOG_TRACE("Operation is %d", operation);
      StringReset(key_vtep_if->if_name);
    }
    return UPLL_RC_SUCCESS;
  }

  upll_rc_t VtepIfMoMgr::ValidateVtnNeighborValue(
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
        return UPLL_RC_ERR_CFG_SYNTAX;
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
        return UPLL_RC_ERR_CFG_SYNTAX;
      }
    }
    if (val_vtn_neighbor->valid[UPLL_IDX_CONN_VLINK_NAME_VN] == UNC_VF_VALID) {
      ret_val = ValidateKey(
          reinterpret_cast<char *>(val_vtn_neighbor->connected_vlink_name),
          kMinLenVlinkName, kMaxLenVlinkName);
      if (ret_val != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Syntax check failed.connected_vlink_name=%s",
            val_vtn_neighbor->connected_vlink_name);
        return UPLL_RC_ERR_CFG_SYNTAX;
      }
    }
    return UPLL_RC_SUCCESS;
  }
  upll_rc_t VtepIfMoMgr::ValVTepIfAttributeSupportCheck(
      val_vtep_if_t *val_vtep_if,
      const uint8_t* attrs, unc_keytype_operation_t operation) {
    UPLL_FUNC_TRACE;
    if ((val_vtep_if->valid[UPLL_IDX_DESC_VTEPI] == UNC_VF_VALID)
        || (val_vtep_if->valid[UPLL_IDX_DESC_VTEPI] == UNC_VF_VALID_NO_VALUE)) {
      if (attrs[unc::capa::vtep_if::kCapDesc] == 0) {
        val_vtep_if->valid[UPLL_IDX_DESC_VTEPI] = UNC_VF_INVALID;
        if (operation == UNC_OP_CREATE || operation == UNC_OP_UPDATE) {
          UPLL_LOG_DEBUG("Attribute Desc not supported in pfc controller");
          return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
        }
      }
    }

    if ((val_vtep_if->valid[UPLL_IDX_ADMIN_ST_VTEPI] == UNC_VF_VALID)
        || (val_vtep_if->valid[UPLL_IDX_ADMIN_ST_VTEPI]
          == UNC_VF_VALID_NO_VALUE)) {
      if (attrs[unc::capa::vtep_if::kCapAdminStatus] == 0) {
        val_vtep_if->valid[UPLL_IDX_ADMIN_ST_VTEPI] = UNC_VF_INVALID;
        if (operation == UNC_OP_CREATE || operation == UNC_OP_UPDATE) {
          UPLL_LOG_DEBUG("Attribute AdminStatus not supported in controller");
          return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
        }
      }
    }
    if ((val_vtep_if->valid[UPLL_IDX_PORT_MAP_VTEPI] == UNC_VF_VALID)
        || (val_vtep_if->valid[UPLL_IDX_PORT_MAP_VTEPI] ==
          UNC_VF_VALID_NO_VALUE)) {
      if ((val_vtep_if->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] ==
            UNC_VF_VALID)
          || (val_vtep_if->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM]
            == UNC_VF_VALID_NO_VALUE)) {
        if (attrs[unc::capa::vtep_if::kCapLogicalPortId] == 0) {
          val_vtep_if->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] =
            UNC_VF_INVALID;
          if (operation == UNC_OP_CREATE || operation == UNC_OP_UPDATE) {
            UPLL_LOG_DEBUG("portmap.swich_id attr is not supported by ctrlr ");
            return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
          }
        }
      }
      if ((val_vtep_if->portmap.valid[UPLL_IDX_VLAN_ID_PM] == UNC_VF_VALID)
          || (val_vtep_if->portmap.valid[UPLL_IDX_VLAN_ID_PM]
            == UNC_VF_VALID_NO_VALUE)) {
        if (attrs[unc::capa::vtep_if::kCapVlanId] == 0) {
          val_vtep_if->portmap.valid[UPLL_IDX_VLAN_ID_PM] =
            UNC_VF_INVALID;
          if (operation == UNC_OP_CREATE || operation == UNC_OP_UPDATE) {
            UPLL_LOG_DEBUG("portmap.vlanid attr is not supported by ctrlr ");
            return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
          }
        }
      }
      if ((val_vtep_if->portmap.valid[UPLL_IDX_TAGGED_PM] == UNC_VF_VALID)
          || (val_vtep_if->portmap.valid[UPLL_IDX_TAGGED_PM]
            == UNC_VF_VALID_NO_VALUE)) {
        if (attrs[unc::capa::vtep_if::kCapTagged] == 0) {
          val_vtep_if->portmap.valid[UPLL_IDX_TAGGED_PM] = UNC_VF_INVALID;
          if (operation == UNC_OP_CREATE || operation == UNC_OP_UPDATE) {
            UPLL_LOG_DEBUG("portmap.Tagged attr is not supported by ctrlr ");
            return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
          }
        }
      }
    }
    return UPLL_RC_SUCCESS;
  }

  upll_rc_t VtepIfMoMgr::ValidateCapability(IpcReqRespHeader *req,
      ConfigKeyVal *ikey, const char * ctrlr_name) {
    UPLL_FUNC_TRACE;
    if (!ikey || !req) {
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

    UPLL_LOG_TRACE("ctrlr_name(%s), operation : (%d)",
        ctrlr_name, req->operation);

    bool result_code = false;
    uint32_t max_instance_count = 0;
    uint32_t max_attrs = 0;
    const uint8_t *attrs = NULL;
    switch (req->operation) {
      case UNC_OP_CREATE:
        result_code = GetCreateCapability(ctrlr_name,
            ikey->get_key_type(), &max_instance_count, &max_attrs, &attrs);
        break;

      case UNC_OP_UPDATE:
        result_code = GetUpdateCapability(ctrlr_name,
            ikey->get_key_type(), &max_attrs, &attrs);
        break;

      case UNC_OP_READ:
      case UNC_OP_READ_SIBLING:
      case UNC_OP_READ_SIBLING_BEGIN:
      case UNC_OP_READ_SIBLING_COUNT:
        result_code = GetReadCapability(ctrlr_name,
            ikey->get_key_type(), &max_attrs, &attrs);
        break;

      default:
        UPLL_LOG_DEBUG("Invalid operation");
        break;
    }
    if (!result_code) {
      UPLL_LOG_DEBUG("keytype(%d) is not supported by controller(%s) "
          "for operation(%d)",
          ikey->get_key_type(), ctrlr_name, req->operation);
      return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
    }
    val_vtep_if_t *val_vtep_if = NULL;
    if ((ikey->get_cfg_val())
        && ((ikey->get_cfg_val())->get_st_num() == IpctSt::kIpcStValVtepIf)) {
      val_vtep_if =
        reinterpret_cast<val_vtep_if_t *> (ikey->get_cfg_val()->get_val());
    }
    if (val_vtep_if) {
      if (max_attrs > 0) {
        return ValVTepIfAttributeSupportCheck(val_vtep_if, attrs,
                                                           req->operation);
      } else {
        UPLL_LOG_DEBUG("Attribute list is empty for operation %d",
                                                            req->operation);
        return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
      }
    }
    return UPLL_RC_SUCCESS;
  }

  /*
     upll_rc_t VtepIfMoMgr::CopyToConfigKey(ConfigKeyVal *&okey,
     ConfigKeyVal *ikey) {

     if ( !ikey || !(ikey->get_key()) )
     return UPLL_RC_ERR_GENERIC;

     upll_rc_t result_code = UPLL_RC_SUCCESS;

     key_rename_vnode_info *key_rename =
     (key_rename_vnode_info *)ikey->get_key();
     key_vtep_if_t *vtep_key =
     (key_vtep_if_t *) malloc ( sizeof (key_vtep_if_t));
     if (!vtep_key)
     return UPLL_RC_ERR_GENERIC;
     if (!strlen ((char *)key_rename->old_unc_vtn_name))
     return UPLL_RC_ERR_GENERIC;
     strcpy ((char *)vtep_key->vtep_key.vtn_key.vtn_name,
     (char *)key_rename->old_unc_vtn_name);

     okey = new ConfigKeyVal (UNC_KT_VTEP_IF, IpctSt::kIpcStKeyVtepIf, vtep_key,
     NULL);
     if (!okey) {
     FREE_IF_NOT_NULL(vtep_key);
     return UPLL_RC_ERR_GENERIC;
     }
     return result_code;
     }
     */

  upll_rc_t VtepIfMoMgr::IsReferenced(IpcReqRespHeader *req,
    ConfigKeyVal *ikey, DalDmlIntf *dmi) {
    upll_rc_t result_code = UPLL_RC_SUCCESS;
    ConfigKeyVal *okey = NULL;
    if (!ikey || !(ikey->get_key()) ||!dmi )
      return UPLL_RC_ERR_GENERIC;
    GetChildConfigKey(okey, ikey);
    DbSubOp dbop = {kOpReadMultiple, kOpMatchNone, kOpInOutFlag};
    result_code = ReadConfigDB(okey, req->datatype, UNC_OP_READ,
        dbop, dmi, MAINTBL);
    if (result_code != UPLL_RC_SUCCESS) {
      DELETE_IF_NOT_NULL(okey);
      return result_code;
    }
    ConfigKeyVal *tkey = okey;
    while (okey) {
      uint8_t vlink_flag = 0;
      GET_USER_DATA_FLAGS(okey, vlink_flag);
      if (vlink_flag & VIF_TYPE) {
        result_code = UPLL_RC_ERR_CFG_SEMANTIC;
        break;
      }
      okey = okey->get_next_cfg_key_val();
    }
    DELETE_IF_NOT_NULL(tkey);
    return result_code;
  }

}  // namespace kt_momgr
}  // namespace upll
}  // namespace unc
