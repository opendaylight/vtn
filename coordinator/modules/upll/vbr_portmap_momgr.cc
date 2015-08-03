/*
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include "dal/dal_dml_intf.hh"
#include "upll_db_query.hh"
#include "vbr_portmap_momgr.hh"
#include "vbr_momgr.hh"
#include "vbr_if_momgr.hh"
#include "vlink_momgr.hh"
#include "unw_spine_domain_momgr.hh"
#include "convert_vnode.hh"
#include "domain_check_util.hh"
#include "upll_util.hh"

#define NUM_KEY_MAIN_TBL_  6
#define NUM_KEY_VBID_TBL_  3
const uint32_t kDefaultMaxCount = 4000;

namespace unc {
namespace upll {
namespace kt_momgr {

BindInfo VbrPortMapMoMgr::vbr_portmap_bind_info[] = {
  { uudst::vbridge_portmap::kDbiVtnName, CFG_KEY, offsetof(
      key_vbr_portmap, vbr_key.vtn_key.vtn_name), uud::kDalChar, 32 },
  { uudst::vbridge_portmap::kDbiVbrName, CFG_KEY, offsetof(
      key_vbr_portmap, vbr_key.vbridge_name), uud::kDalChar, 32 },
  { uudst::vbridge_portmap::kDbiPortMapId, CFG_KEY, offsetof(
      key_vbr_portmap, portmap_id), uud::kDalChar, 32 },
  { uudst::vbridge_portmap::kDbiCtrlrName, CFG_VAL, offsetof(
      pfcdrv_val_vbr_portmap, vbrpm.controller_id), uud::kDalChar, 32 },
  { uudst::vbridge_portmap::kDbiDomainId, CFG_VAL, offsetof(
      pfcdrv_val_vbr_portmap, vbrpm.domain_id), uud::kDalChar, 32 },
  { uudst::vbridge_portmap::kDbiLogicalPortId, CFG_VAL, offsetof(
      pfcdrv_val_vbr_portmap, vbrpm.logical_port_id), uud::kDalChar, 320 },
  { uudst::vbridge_portmap::kDbiLabelType, CFG_VAL, offsetof(
      pfcdrv_val_vbr_portmap, vbrpm.label_type), uud::kDalUint8, 1 },
  { uudst::vbridge_portmap::kDbiLabel, CFG_VAL, offsetof(
      pfcdrv_val_vbr_portmap, vbrpm.label), uud::kDalUint32, 1 },
  { uudst::vbridge_portmap::kDbiBdryRefCount, CFG_VAL, offsetof(
      pfcdrv_val_vbr_portmap, bdry_ref_count), uud::kDalUint32, 1 },
  { uudst::vbridge_portmap::kDbiCtrlrName, CK_VAL, offsetof(
      key_user_data, ctrlr_id), uud::kDalChar, 32 },
  { uudst::vbridge_portmap::kDbiDomainId, CK_VAL, offsetof(
      key_user_data, domain_id), uud::kDalChar, 32 },
  { uudst::vbridge_portmap::kDbiVbrPortMapFlags, CK_VAL, offsetof(
      key_user_data_t, flags), uud::kDalUint8, 1 },
  { uudst::vbridge_portmap::kDbiOperStatus, ST_VAL, offsetof(
      val_db_vbr_portmap_st, vbr_portmap_val_st.oper_status),
      uud::kDalUint8, 1 },
  { uudst::vbridge_portmap::kDbiDownCount, ST_VAL, offsetof(
      val_db_vbr_portmap_st, down_count), uud::kDalUint32, 1 },
  { uudst::vbridge_portmap::kDbiValidCtrlrName, CFG_META_VAL, offsetof(
      pfcdrv_val_vbr_portmap, vbrpm.valid[0]), uud::kDalUint8, 1 },
  { uudst::vbridge_portmap::kDbiValidDomainId, CFG_META_VAL, offsetof(
      pfcdrv_val_vbr_portmap, vbrpm.valid[1]), uud::kDalUint8, 1 },
  {  uudst::vbridge_portmap::kDbiValidLogicalPortId, CFG_META_VAL, offsetof(
      pfcdrv_val_vbr_portmap, vbrpm.valid[2]), uud::kDalUint8, 1 },
  {  uudst::vbridge_portmap::kDbiValidLabelType, CFG_META_VAL, offsetof(
      pfcdrv_val_vbr_portmap, vbrpm.valid[3]), uud::kDalUint8, 1 },
  {  uudst::vbridge_portmap::kDbiValidLabel,  CFG_META_VAL, offsetof(
      pfcdrv_val_vbr_portmap, vbrpm.valid[4]), uud::kDalUint8, 1 },
  {  uudst::vbridge_portmap::kDbiValidBdryRefCount, CFG_META_VAL, offsetof(
      pfcdrv_val_vbr_portmap, valid[1]), uud::kDalUint8, 1 },
  {  uudst::vbridge_portmap::kDbiValidOperStatus, ST_META_VAL, offsetof(
      val_vbr_portmap_st, valid[0]), uud::kDalUint8, 1 },
  { uudst::vbridge_portmap::kDbiCsRowstatus, CS_VAL, offsetof(
      pfcdrv_val_vbr_portmap, vbrpm.cs_row_status), uud::kDalUint8, 1 },
  { uudst::vbridge_portmap::kDbiCsCtrlrName, CS_VAL, offsetof(
      pfcdrv_val_vbr_portmap, vbrpm.cs_attr[0]), uud::kDalUint8, 1 },
  { uudst::vbridge_portmap::kDbiCsDomainId, CS_VAL, offsetof(
      pfcdrv_val_vbr_portmap, vbrpm.cs_attr[1]), uud::kDalUint8, 1 },
  { uudst::vbridge_portmap::kDbiCsLogicalPortId, CS_VAL, offsetof(
      pfcdrv_val_vbr_portmap, vbrpm.cs_attr[2]), uud::kDalUint8, 1 },
  { uudst::vbridge_portmap::kDbiCsLabelType, CS_VAL, offsetof(
      pfcdrv_val_vbr_portmap, vbrpm.cs_attr[3]), uud::kDalUint8, 1 },
  { uudst::vbridge_portmap::kDbiCsLabel, CS_VAL, offsetof(
      pfcdrv_val_vbr_portmap, vbrpm.cs_attr[4]), uud::kDalUint8, 1 }
};

BindInfo VbrPortMapMoMgr::key_vbr_portmap_maintbl_bind_info[] = {
  { uudst::vbridge_portmap::kDbiVtnName, CFG_MATCH_KEY, offsetof(
      key_vbr_portmap_t, vbr_key.vtn_key.vtn_name),
      uud::kDalChar, kMaxLenVtnName + 1 },
  { uudst::vbridge_portmap::kDbiVbrName, CFG_MATCH_KEY, offsetof(
      key_vbr_portmap_t, vbr_key.vbridge_name),
      uud::kDalChar, kMaxLenVnodeName + 1 },
  { uudst::vbridge_portmap::kDbiPortMapId, CFG_MATCH_KEY, offsetof(
      key_vbr_portmap_t, portmap_id),
      uud::kDalChar, kMaxLenVbrPortMapId + 1 },
  { uudst::vbridge_portmap::kDbiVtnName, CFG_INPUT_KEY, offsetof(
      key_rename_vnode_info_t, new_unc_vtn_name),
      uud::kDalChar, kMaxLenVtnName + 1 },
  { uudst::vbridge_portmap::kDbiVbrName, CFG_INPUT_KEY, offsetof(
      key_rename_vnode_info_t, new_unc_vnode_name),
      uud::kDalChar, kMaxLenVnodeName + 1 },
  { uudst::vbridge_portmap::kDbiVbrPortMapFlags, CK_VAL, offsetof(
      key_user_data_t, flags),
      uud::kDalUint8, 1 }
};


VbrPortMapMoMgr::VbrPortMapMoMgr() {
  UPLL_FUNC_TRACE;
  ntable = MAX_MOMGR_TBLS;
  table = new Table *[ntable]();
  table[MAINTBL] = new Table(uudst::kDbiVbrPortMapTbl, UNC_KT_VBR_PORTMAP,
                             vbr_portmap_bind_info, IpctSt::kIpcStKeyVbrPortMap,
                             IpctSt::kIpcStPfcdrvValVbrPortMap,
                             // increment the VbrPortMapNumCols with 2 while
                             // setting nattr as ctrlr, domain in key_user_data
                             // are extra compare to DB columns
                             (uudst::vbridge_portmap::kDbiVbrPortmapNumCols+2));
  table[VBIDTBL] = new Table(uudst::kDbiVBIdTbl, UNC_KT_VBR_PORTMAP,
                             vbid_bind_info, IpctSt::kIpcStKeyVbid,
                             IpctSt::kIpcStValVbid,
                             uudst::vbid_label::kDbiVBIdNumCols);
  table[GVTNIDTBL] = new Table(uudst::kDbiGVtnIdTbl, UNC_KT_VBR_PORTMAP,
                             gvtnid_bind_info, IpctSt::kIpcStKeyGVtnId,
                             IpctSt::kIpcStValGVtnId,
                             uudst::gvtnid_label::kDbiGVtnIdNumCols);
  table[RENAMETBL] = NULL;
  table[CTRLRTBL] = NULL;
  table[CONVERTTBL] = NULL;
  nchild = 0;
  child = NULL;
}

/*
 * Based on the key type the bind info will pass
 **/
bool VbrPortMapMoMgr::GetRenameKeyBindInfo(
    unc_key_type_t key_type,
    BindInfo *&binfo,
    int &nattr, MoMgrTables tbl) {
  // Main Table or rename table only update
  if (MAINTBL == tbl) {
    nattr = NUM_KEY_MAIN_TBL_;
    binfo = key_vbr_portmap_maintbl_bind_info;
  } else if (VBIDTBL == tbl) {
    nattr = NUM_KEY_VBID_TBL_;
    binfo = vbid_rename_bind_info;
  } else {
    UPLL_LOG_ERROR("Invalid table");
    return false;
  }
  return true;
}

bool VbrPortMapMoMgr::IsValidKey(void *key, uint64_t index, MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  upll_rc_t ret_val = UPLL_RC_SUCCESS;
  switch (tbl) {
    case MAINTBL: {
      key_vbr_portmap *vbr_pm_key = reinterpret_cast<key_vbr_portmap *>(key);
      switch (index) {
        case uudst::vbridge_portmap::kDbiVtnName:
          ret_val = ValidateKey(
            reinterpret_cast<char *>(vbr_pm_key->vbr_key.vtn_key.vtn_name),
            kMinLenVtnName, kMaxLenVtnName);
          if (ret_val != UPLL_RC_SUCCESS) {
            UPLL_LOG_TRACE("Vtn name is not valid(%d)", ret_val);
            return false;
          }
          break;
        case uudst::vbridge_portmap::kDbiVbrName:
          ret_val = ValidateKey(
            reinterpret_cast<char *>(vbr_pm_key->vbr_key.vbridge_name),
            kMinLenVnodeName, kMaxLenVnodeName);
          if (ret_val != UPLL_RC_SUCCESS) {
            UPLL_LOG_TRACE("Vbridge name is not valid(%d)", ret_val);
            return false;
          }
          break;
        case uudst::vbridge_portmap::kDbiPortMapId:
          ret_val = ValidateKey(
            reinterpret_cast<char *>(vbr_pm_key->portmap_id),
            kMinLenVbrPortMapId, kMaxLenVbrPortMapId);
          if (ret_val != UPLL_RC_SUCCESS) {
            UPLL_LOG_TRACE("Vbridge portmap id is not valid(%d)", ret_val);
            return false;
          }
          break;
        default:
          UPLL_LOG_ERROR("Invalid key index");
          return false;
        }
      }
      break;
      case GVTNIDTBL: {
        key_gvtnid_label *gvtn_key = reinterpret_cast<key_gvtnid_label *>(key);
        switch (index) {
          case uudst::gvtnid_label::kDbiCtrlrName:
            ret_val = ValidateKey(reinterpret_cast<char *>(
              gvtn_key->ctrlr_name),
                            kMinLenCtrlrId, kMaxLenCtrlrId);
            break;
          case uudst::gvtnid_label::kDbiDomainId:
            ret_val = ValidateKey(reinterpret_cast<char *>(
                        gvtn_key->domain_name),
                            kMinLenDomainId, kMaxLenDomainId);
            break;
          case uudst::gvtnid_label::kDbiSNo:
            ret_val = (ValidateNumericRange(
                      gvtn_key->label_row, kMinGVtnIdRows,
                      kMaxGVtnIdRows, true, true)) ?
                      UPLL_RC_SUCCESS: UPLL_RC_ERR_GENERIC;
            break;
          default:
            ret_val = UPLL_RC_ERR_GENERIC;
      }
    }
    break;
    case VBIDTBL: {
      key_vbid_label *label_key = reinterpret_cast<key_vbid_label*>(key);
      switch (index) {
        case uudst::vbid_label::kDbiVtnName:
          ret_val = ValidateKey(reinterpret_cast<char *>
                            (label_key->vtn_key.vtn_name),
                            kMinLenVtnName, kMaxLenVtnName);
          break;
        case uudst::vbid_label::kDbiSNo:
          ret_val = (ValidateNumericRange(
                      label_key->label_row, kMinVbidIdRows,
                      kMaxVbidIdRows, true, true))?
                      UPLL_RC_SUCCESS: UPLL_RC_ERR_GENERIC;
          break;
        default:
          ret_val = UPLL_RC_ERR_GENERIC;
      }
    }
    break;
    default:
      UPLL_LOG_ERROR("Invalid key index");
      return false;
  }
  return (ret_val == UPLL_RC_SUCCESS)?
            true : false;
}

upll_rc_t VbrPortMapMoMgr::DupConfigKeyVal(ConfigKeyVal *&okey,
                                           ConfigKeyVal *&req_key,
                                           MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  switch (tbl) {
    case MAINTBL:
    case CONVERTTBL: {   // vtn converttbl
      ConfigVal *okey_cval = NULL;

      if (req_key == NULL) {
        UPLL_LOG_ERROR("req_key is null");
        return UPLL_RC_ERR_GENERIC;
      }
      if (tbl == CONVERTTBL) {
        key_vbr_portmap *vbr_pm_key = ConfigKeyVal::Malloc<key_vbr_portmap>();
        key_vbr_portmap *vbrpm_key = reinterpret_cast<key_vbr_portmap_t *>
                                     (req_key->get_key());
        uuu::upll_strncpy(vbr_pm_key->vbr_key.vtn_key.vtn_name,
            vbrpm_key->vbr_key.vtn_key.vtn_name,
            (kMaxLenVtnName+1));
        okey = new ConfigKeyVal(UNC_KT_VBR_PORTMAP,
            IpctSt::kIpcStKeyVbrPortMap,
            vbr_pm_key, NULL);
        SET_USER_DATA(okey, req_key);
      } else {
        // create a duplicate configkeyavl from the parent configkeyval
        result_code = GetChildConfigKey(okey, req_key);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_INFO("GetChildConfigKey Failed - %d", result_code);
          return result_code;
        }
      }

      ConfigVal *tmp = req_key->get_cfg_val();

      if (tmp) {
        if (tbl == CONVERTTBL) {
          // copy the val structure from val_vtn_gateway_port
          // to pfcdrv_val_vbr_portmap
          val_vtn_gateway_port *val_vtn_gw = static_cast<
              val_vtn_gateway_port*>(tmp->get_val());
          pfcdrv_val_vbr_portmap *vbrpm_val =
              ConfigKeyVal::Malloc<pfcdrv_val_vbr_portmap>();
          uuu::upll_strncpy(vbrpm_val->vbrpm.logical_port_id,
                       val_vtn_gw->logical_port_id, kMaxLenLogicalPortId+1);
          vbrpm_val->vbrpm.label_type = UPLL_LABEL_TYPE_GW_VLAN;
          vbrpm_val->vbrpm.label = val_vtn_gw->label;
          vbrpm_val->vbrpm.valid[UPLL_IDX_LOGICAL_PORT_ID_VBRPM] = UNC_VF_VALID;
          vbrpm_val->vbrpm.valid[UPLL_IDX_LABEL_VBRPM] = UNC_VF_VALID;
          vbrpm_val->vbrpm.valid[UPLL_IDX_LABEL_TYPE_VBRPM] = UNC_VF_VALID;
          okey_cval = new ConfigVal(IpctSt::kIpcStPfcdrvValVbrPortMap,
                                    vbrpm_val);
        } else {
        // check whether the req_key holds val_vbr_portmap
        if (tmp->get_st_num() == IpctSt::kIpcStValVbrPortMap) {
          val_vbr_portmap *pkey_val = static_cast<val_vbr_portmap *>(
              GetVal(req_key));
          if (!pkey_val) {
            UPLL_LOG_ERROR("val is null");
            DELETE_IF_NOT_NULL(okey);
            return UPLL_RC_ERR_GENERIC;
          }

          val_vbr_portmap *vbrpm_val = ConfigKeyVal::Malloc<val_vbr_portmap>();
          memcpy(vbrpm_val, pkey_val, sizeof(val_vbr_portmap));
          okey_cval = new ConfigVal(IpctSt::kIpcStValVbrPortMap, vbrpm_val);
        } else if (tmp->get_st_num() == IpctSt::kIpcStPfcdrvValVbrPortMap) {
          // check whether the parent key holds pfcdrv_val_vbr_portmap
          pfcdrv_val_vbr_portmap *pkey_val = static_cast<
            pfcdrv_val_vbr_portmap*>(GetVal(req_key));
          if (!pkey_val) {
            UPLL_LOG_ERROR("val is null");
            DELETE_IF_NOT_NULL(okey);
            return UPLL_RC_ERR_GENERIC;
          }

          pfcdrv_val_vbr_portmap *vbrpm_val =
              ConfigKeyVal::Malloc<pfcdrv_val_vbr_portmap>();
          memcpy(vbrpm_val, pkey_val, sizeof(pfcdrv_val_vbr_portmap));
          okey_cval = new ConfigVal(tmp->get_st_num(), vbrpm_val);
        } else {
          UPLL_LOG_INFO("Invalid st_num");
          DELETE_IF_NOT_NULL(okey);
          return UPLL_RC_ERR_GENERIC;
        }
        tmp = tmp->get_next_cfg_val();
        }
      }
      if ((tmp) && (tmp->get_st_num() == IpctSt::kIpcStValVbrPortMapSt)) {
        val_db_vbr_portmap_st *ival = reinterpret_cast<val_db_vbr_portmap_st *>(
            tmp->get_val());
        if (ival == NULL) {
          UPLL_LOG_ERROR("Null val st structure");
          DELETE_IF_NOT_NULL(okey_cval);
          DELETE_IF_NOT_NULL(okey);
          return UPLL_RC_ERR_GENERIC;
        }
        val_db_vbr_portmap_st *val_vbr_pm_st =
            ConfigKeyVal::Malloc<val_db_vbr_portmap_st>();
        memcpy(val_vbr_pm_st, ival, sizeof(val_db_vbr_portmap_st));
        okey_cval->AppendCfgVal(IpctSt::kIpcStValVbrPortMapSt, val_vbr_pm_st);
      }
      okey->SetCfgVal(okey_cval);
      SET_USER_DATA(okey, req_key);
    }
    break;
    case VBIDTBL: {
      result_code = DupVbIdConfigKeyVal(okey, req_key);
    }
    break;
    case GVTNIDTBL: {
      result_code = DupGvtnIdConfigKeyVal(okey, req_key);
    }
    break;
    default:
      UPLL_LOG_DEBUG("Invalid tbl type : %d", tbl);
      return UPLL_RC_ERR_GENERIC;
  }
  return result_code;
}

upll_rc_t VbrPortMapMoMgr::GetChildConfigKey(ConfigKeyVal *&okey,
                                             ConfigKeyVal *pkey) {
  UPLL_FUNC_TRACE;
  key_vbr_portmap_t *vbr_pm_key = NULL;
  bool cfgval_ctrlr = false;

  if (okey && (okey->get_key_type() != UNC_KT_VBR_PORTMAP)) {
    UPLL_LOG_ERROR("invlaid input arg");
    return UPLL_RC_ERR_GENERIC;
  }

  if (okey && okey->get_key()) {
    vbr_pm_key = reinterpret_cast<key_vbr_portmap_t *>(okey->get_key());
  } else {
    vbr_pm_key = ConfigKeyVal::Malloc<key_vbr_portmap>();
  }
  if (pkey && pkey->get_key()) {
    //  copy key struct from pkey to vbr_pm_key
    unc_key_type_t keytype = pkey->get_key_type();
    switch (keytype) {
      case UNC_KT_VTN:
        uuu::upll_strncpy(vbr_pm_key->vbr_key.vtn_key.vtn_name,
            static_cast<key_vtn*>(pkey->get_key())->vtn_name,
            (kMaxLenVtnName+1));
        break;
      case UNC_KT_VBRIDGE:
        uuu::upll_strncpy(vbr_pm_key->vbr_key.vtn_key.vtn_name,
            static_cast<key_vbr*>(
              pkey->get_key())->vtn_key.vtn_name,
            (kMaxLenVtnName+1));
        uuu::upll_strncpy(vbr_pm_key->vbr_key.vbridge_name,
            static_cast<key_vbr*>(pkey->get_key())->vbridge_name,
            (kMaxLenVnodeName+1));
        break;
      case UNC_KT_VBR_PORTMAP:
        uuu::upll_strncpy(vbr_pm_key->vbr_key.vtn_key.vtn_name,
            static_cast<key_vbr_portmap*>(
              pkey->get_key())->vbr_key.vtn_key.vtn_name,
            (kMaxLenVtnName+1));
        uuu::upll_strncpy(vbr_pm_key->vbr_key.vbridge_name,
            static_cast<key_vbr_portmap*>(
              pkey->get_key())->vbr_key.vbridge_name,
            (kMaxLenVnodeName+1));
        uuu::upll_strncpy(vbr_pm_key->portmap_id,
            static_cast<key_vbr_portmap*>(
              pkey->get_key())->portmap_id,
            (kMaxLenVbrPortMapId+1));
        break;
      case UNC_KT_VLINK: {  // VbrPortmapOnBoundary: To handle vlink kt
        uint8_t *vnode_name;
        uint8_t flags = 0;
        val_vlink *vlink_val = reinterpret_cast<val_vlink *>(GetVal(pkey));
        if (!vlink_val) {
          UPLL_LOG_ERROR("vlink val is null");
          if (!okey || !(okey->get_key())) {
            FREE_IF_NOT_NULL(vbr_pm_key);
          }
          return UPLL_RC_ERR_GENERIC;
        }

        GET_USER_DATA_FLAGS(pkey->get_cfg_val(), flags);
        flags &=  VLINK_FLAG_NODE_POS;
        UPLL_LOG_DEBUG("Vlink flag node position %d", flags);

        if (flags == kVlinkVnode2) {
          cfgval_ctrlr = true;
          vnode_name = vlink_val->vnode2_name;
        } else {
          vnode_name = vlink_val->vnode1_name;
        }
        uuu::upll_strncpy(vbr_pm_key->vbr_key.vtn_key.vtn_name,
                          reinterpret_cast<key_vlink *>(
                          pkey->get_key())->vtn_key.vtn_name,
                          (kMaxLenVtnName + 1));
        if (vnode_name)
          uuu::upll_strncpy(vbr_pm_key->vbr_key.vbridge_name, vnode_name,
                            (kMaxLenVnodeName + 1));

        break;
      }
      default:
        UPLL_LOG_ERROR("Unsupported key type %d", keytype);
        if (!okey || !okey->get_key()) {
          FREE_IF_NOT_NULL(vbr_pm_key);
        }
        return UPLL_RC_ERR_GENERIC;
    }
  }

  // if okey is NULL, allocate the memory  and set vbr_pm_key
  if (!okey) {
    okey = new ConfigKeyVal(UNC_KT_VBR_PORTMAP,
                            IpctSt::kIpcStKeyVbrPortMap,
                            vbr_pm_key, NULL);
  } else if (okey->get_key() != vbr_pm_key) {
    okey->SetKey(IpctSt::kIpcStKeyVbrPortMap, vbr_pm_key);
  }
  if (cfgval_ctrlr) {
    SET_USER_DATA(okey, pkey->get_cfg_val());
  } else {
    SET_USER_DATA(okey, pkey);
  }
  return UPLL_RC_SUCCESS;
}

/** this function constructs KT_VBRIDGE main tbl key structure
 * from KT_VBR_PORTMAP ckv
 */
upll_rc_t VbrPortMapMoMgr::GetParentConfigKey(ConfigKeyVal *&okey,
                                              ConfigKeyVal *ikey) {
  UPLL_FUNC_TRACE;

  if (!ikey || (ikey->get_key_type() != UNC_KT_VBR_PORTMAP) ||
      !ikey->get_key()) {
    UPLL_LOG_ERROR("Invalid input arg");
    return UPLL_RC_ERR_GENERIC;
  }

  key_vbr_portmap *vbr_pm_key = reinterpret_cast<key_vbr_portmap *>
    (ikey->get_key());

  DELETE_IF_NOT_NULL(okey);

  // create and fill the parent(KT_VBRIDGE) key structure
  key_vbr *vbr_key = ConfigKeyVal::Malloc<key_vbr>();
  uuu::upll_strncpy(vbr_key->vtn_key.vtn_name,
                    vbr_pm_key->vbr_key.vtn_key.vtn_name, (kMaxLenVtnName+1));
  uuu::upll_strncpy(vbr_key->vbridge_name,
                    vbr_pm_key->vbr_key.vbridge_name, (kMaxLenVnodeName+1));

  okey = new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVbr, vbr_key, NULL);
  SET_USER_DATA(okey, ikey);
  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrPortMapMoMgr::AllocVal(ConfigVal *&ck_val,
                                    upll_keytype_datatype_t dt_type,
                                    MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  if (ck_val != NULL) {
    UPLL_LOG_ERROR("Invalid input arg");
    return UPLL_RC_ERR_GENERIC;
  }
  switch (tbl) {
    case MAINTBL:
      ck_val = new ConfigVal(
          IpctSt::kIpcStPfcdrvValVbrPortMap,
          ConfigKeyVal::Malloc(sizeof(pfcdrv_val_vbr_portmap)));
      if (dt_type == UPLL_DT_STATE) {
        ck_val->AppendCfgVal(
            IpctSt::kIpcStValVbrPortMapSt,
            ConfigKeyVal::Malloc(sizeof(val_db_vbr_portmap_st)));
      }
      break;
    case VBIDTBL:
      ck_val = new ConfigVal(
          IpctSt::kIpcStValVbid,
          ConfigKeyVal::Malloc(sizeof(val_vbid_label)));
      break;
    case GVTNIDTBL:
      ck_val = new ConfigVal(
          IpctSt::kIpcStValGVtnId,
          ConfigKeyVal::Malloc(sizeof(val_gvtnid_label)));
      break;
    default:
      UPLL_LOG_INFO("Invalid table");
      return UPLL_RC_ERR_GENERIC;
  }
  return UPLL_RC_SUCCESS;
}

/** This function checks the given key structure in ikey exists in DB,
 * and returns following return code, 
 * 1. return NO_SUCH_INSTANCE, if the given key strucuture is not exists in DB
 * 2. returns INSTANCE_EXISTS
 *    a. if the USER PM flag set in DB.
 *    b. either the value of logical_port_id and label in request and in DB
 *     differs
 * 3. returns SEMANTIC error
 *    a. BOUNDARY PM flag set in DB and logical_port_id matches but label
 *    in request is 0xFFFE
 * 4. return GENERIC error if flag other than USER or BOUNDARY PM exists in DB
 * 5. returns SUCCESS, if all the entry in DB and in request matches and
 *     if the user flag update in DB success 
 * 6. returns DB_ERROR if any DB operation fails
 */
upll_rc_t VbrPortMapMoMgr::ValidateVbrPmCreate(IpcReqRespHeader *req,
                                                   ConfigKeyVal *ikey,
                                                   DalDmlIntf *dmi,
                                                   TcConfigMode config_mode,
                                                   string vtn_name) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  // create the dup configkeyval structure from ikey for record existance check
  ConfigKeyVal *db_ckv = NULL;
  result_code = GetChildConfigKey(db_ckv, ikey);
  if (!db_ckv || result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_INFO("GetChildConfigKey failed %d", result_code);
    return result_code;
  }

  DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutFlag };
  result_code = ReadConfigDB(db_ckv, UPLL_DT_CANDIDATE, UNC_OP_READ, dbop,
                             dmi, MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    if (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      UPLL_LOG_ERROR("read failed = %u", result_code);
    }
    DELETE_IF_NOT_NULL(db_ckv);
    return result_code;
  }
  UPLL_LOG_TRACE("DB data %s", db_ckv->ToStrAll().c_str());

  // compare the flag - When the request is received from boundary, boundary bit
  // is always set in user_data
  uint8_t db_flag = 0;
  GET_USER_DATA_FLAGS(db_ckv, db_flag);
  DELETE_IF_NOT_NULL(db_ckv);
  if (db_flag & USER_VBRPORTMAP_FLAG) {
    // key exists already
    UPLL_LOG_DEBUG("User configured vbr portmap already exists");
    return UPLL_RC_ERR_INSTANCE_EXISTS;
  } else if (db_flag & BOUNDARY_VBRPORTMAP_FLAG) {
    UPLL_LOG_DEBUG("Internal portmap exists with same name");
    return UPLL_RC_ERR_NAME_CONFLICT;
  } else {
    UPLL_LOG_ERROR("should not come here, error in flag");
    DELETE_IF_NOT_NULL(db_ckv);
    return UPLL_RC_ERR_GENERIC;
  }
}

/** This function checks the ikey st_num, based on the st_num fills
 * val_vbr_portmap */
upll_rc_t VbrPortMapMoMgr::GetValVbrPortMap(ConfigKeyVal *ikey,
    val_vbr_portmap **vbrpm) {
  if ((ikey->get_cfg_val())->get_st_num() == IpctSt::kIpcStValVbrPortMap) {
    *vbrpm = static_cast<val_vbr_portmap*>(GetVal(ikey));
  } else if (ikey->get_cfg_val()->get_st_num() ==
       IpctSt::kIpcStPfcdrvValVbrPortMap) {
    pfcdrv_val_vbr_portmap *vbrpm_val = static_cast<pfcdrv_val_vbr_portmap*>(
        GetVal(ikey));
    *vbrpm = &(vbrpm_val->vbrpm);
  } else {
    UPLL_LOG_ERROR("Invalid st num");
    return UPLL_RC_ERR_GENERIC;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrPortMapMoMgr::GetControllerDomainId(ConfigKeyVal *ikey,
                                          controller_domain *ctrlr_dom) {
  UPLL_FUNC_TRACE;
  if (!ikey || !ctrlr_dom) {
    UPLL_LOG_INFO("Illegal parameter");
    return UPLL_RC_ERR_GENERIC;
  }
  val_vbr_portmap *ikey_vbr_pm = NULL;
  upll_rc_t result_code = GetValVbrPortMap(ikey, &ikey_vbr_pm);
  if (result_code != UPLL_RC_SUCCESS) {
    return result_code;
  }
  if (ikey_vbr_pm == NULL
      || ikey_vbr_pm->valid[UPLL_IDX_CONTROLLER_ID_VBRPM] != UNC_VF_VALID
      || !strlen(reinterpret_cast<char*>(ikey_vbr_pm->controller_id))) {
    ctrlr_dom->ctrlr = NULL;
    UPLL_LOG_DEBUG("Ctrlr null");
  } else {
    SET_USER_DATA_CTRLR(ikey, ikey_vbr_pm->controller_id);
  }
  if (ikey_vbr_pm == NULL
      || ikey_vbr_pm->valid[UPLL_IDX_DOMAIN_ID_VBRPM] != UNC_VF_VALID
      || !strlen(reinterpret_cast<char*>(
         ikey_vbr_pm->domain_id))) {
    ctrlr_dom->domain = NULL;
    UPLL_LOG_DEBUG("Domain null");
  } else {
    SET_USER_DATA_DOMAIN(ikey, ikey_vbr_pm->domain_id);
    GET_USER_DATA_CTRLR_DOMAIN(ikey, *ctrlr_dom);
  }
  UPLL_LOG_DEBUG("ctrlr_dom %s %s", ctrlr_dom->ctrlr, ctrlr_dom->domain);
  return UPLL_RC_SUCCESS;
}

/** This function is called in two scenarios
 * 1. user creates VBR_PORTMAP. In this case, in ikey contains no PM flags.
 * 2. internal VBR_PORTMAP is created for the first time on unified vbr for the
 * boundary vlink created between unified vbr_if and normal vbr_if. In this
 * case, in ikey only BOUNDARY_VBRPORTMAP_FLAG needs to be set.
 *
 * This function also supports creating in IMPORT.
 */
upll_rc_t VbrPortMapMoMgr::CreateCandidateMo(IpcReqRespHeader *req,
                                             ConfigKeyVal *ikey,
                                             DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  if (NULL == req) {
    UPLL_LOG_INFO("Invalid input argument");
    return UPLL_RC_ERR_GENERIC;
  }

  if (req->datatype != UPLL_DT_CANDIDATE && req->datatype != UPLL_DT_IMPORT) {
    UPLL_LOG_INFO("Invalid datatype %d", req->datatype);
    return UPLL_RC_ERR_GENERIC;
  }

  result_code = ValidateMessage(req, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_TRACE("Returning error %d", result_code);
    return result_code;
  }

  // set the user-configured bit, if the boundary flag is not set in request
  uint8_t ikey_flag = 0;
  GET_USER_DATA_FLAGS(ikey, ikey_flag);

  if (!(ikey_flag & BOUNDARY_VBRPORTMAP_FLAG)) {
    ikey_flag |= USER_VBRPORTMAP_FLAG;
    SET_USER_DATA_FLAGS(ikey, ikey_flag);
  }

  // get the config mode to use in UpdateConfigDB
  TcConfigMode config_mode = TC_CONFIG_INVALID;
  std::string vtn_name;
  result_code = GetConfigModeInfo(req, config_mode, vtn_name);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_ERROR("GetConfigMode failed:%d", result_code);
    return result_code;
  }

  val_vbr_portmap *val_vbrpm = NULL;
  result_code = GetValVbrPortMap(ikey, &val_vbrpm);
  if (result_code != UPLL_RC_SUCCESS) {
    return result_code;
  }

  uint8_t  *ctrlr = val_vbrpm->controller_id;
  uint8_t *domain = val_vbrpm->domain_id;
  // set controller and domain id in key_user_data
  SET_USER_DATA_CTRLR(ikey, ctrlr);
  SET_USER_DATA_DOMAIN(ikey, domain);

  ConfigKeyVal *parent_ckv = NULL;
  result_code = GetParentConfigKey(parent_ckv, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_INFO("Get parent failed");
    return result_code;
  }
  // if st_num is kIpcStValVbrPortMap, then it is a user vbr-pm request on
  // Candidate (not through boundary-map)
  if ((ikey->get_cfg_val())->get_st_num() == IpctSt::kIpcStValVbrPortMap) {
    // do the parent check only for user configured vbr portmap
    // For boundary portmap, KT_VLINK does the parent unified vbr validation.
    result_code = ValidateParentIsUnVbr(req, ikey, parent_ckv, dmi);
    if (result_code != UPLL_RC_SUCCESS) {
      DELETE_IF_NOT_NULL(parent_ckv);
      return result_code;
    }
    // check the given vbr pm exists already in DB
    result_code = ValidateVbrPmCreate(req, ikey, dmi, config_mode, vtn_name);
    if (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      DELETE_IF_NOT_NULL(parent_ckv);
      return result_code;
    }
  }
  result_code = ValidateAttribute(ikey, dmi, req);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("ValidateAttribute returns error %d", result_code);
    DELETE_IF_NOT_NULL(parent_ckv);
    return result_code;
  }

  // NO_SUCH_INSTANCE in DB, continue the checks
  result_code = ValidateCapability(req, ikey, reinterpret_cast<char *>(ctrlr));
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("ValidateCapability failed");
    DELETE_IF_NOT_NULL(parent_ckv);
    return result_code;
  }
  ConfigKeyVal *dup_ikey = NULL;
  // create dup_ikey to use in convertVNode and in UpdateConfigDB
  result_code = GetChildConfigKey(dup_ikey, ikey);
  if (!dup_ikey || result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_INFO("GetChildConfigKey error %d", result_code);
    DELETE_IF_NOT_NULL(parent_ckv);
    return result_code;
  }

  if (req->datatype == UPLL_DT_CANDIDATE) {
    // do conversion if logical_port id is configured
    if (val_vbrpm->valid[UPLL_IDX_LOGICAL_PORT_ID_VBRPM] == UNC_VF_VALID) {
      // pass val_vbr_portmap to convertVNode
      if (ikey->get_cfg_val()->get_st_num() ==
          IpctSt::kIpcStPfcdrvValVbrPortMap) {
        val_vbr_portmap *t_val_vbr_pm =
            ConfigKeyVal::Malloc<val_vbr_portmap_t>();
        memcpy(t_val_vbr_pm, val_vbrpm, sizeof(val_vbr_portmap));
        dup_ikey->AppendCfgVal(IpctSt::kIpcStValVbrPortMap, t_val_vbr_pm);
        result_code = ConvertVNode(dup_ikey, dmi, config_mode, vtn_name, req);
        GET_USER_DATA_FLAGS(dup_ikey, ikey_flag);
      } else {
        result_code = ConvertVNode(ikey, dmi, config_mode, vtn_name, req);
        GET_USER_DATA_FLAGS(ikey, ikey_flag);
      }
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_INFO("Conversion failed");
        DELETE_IF_NOT_NULL(dup_ikey);
        DELETE_IF_NOT_NULL(parent_ckv);
        return result_code;
      }
    }
  }

  pfcdrv_val_vbr_portmap_t *drvval_vbrpm =
      ConfigKeyVal::Malloc<pfcdrv_val_vbr_portmap_t >();
  if ((ikey->get_cfg_val())->get_st_num() == IpctSt::kIpcStValVbrPortMap) {
    // copy the ikey's val_vbr_portmap to pfcdrv_val_vbr_portmap to store in
    // DB
    // fill val structure
    memcpy(&(drvval_vbrpm->vbrpm), ikey->get_cfg_val()->get_val(),
           sizeof(val_vbr_portmap_t));
    drvval_vbrpm->valid[PFCDRV_IDX_VAL_VBR_PORTMAP] = UNC_VF_VALID;
  } else {
    memcpy(drvval_vbrpm, ikey->get_cfg_val()->get_val(),
           sizeof(pfcdrv_val_vbr_portmap_t));
  }
  dup_ikey->DeleteCfgVal();
  dup_ikey->AppendCfgVal(IpctSt::kIpcStPfcdrvValVbrPortMap, drvval_vbrpm);

  SET_USER_DATA_FLAGS(dup_ikey, ikey_flag);
  // set the rename flag if exists for parent and create the entry in DB
  result_code = RestoreVnode(dup_ikey, parent_ckv, req, dmi);
  DELETE_IF_NOT_NULL(parent_ckv);
  DELETE_IF_NOT_NULL(dup_ikey);

  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_INFO("Failed to create entry in DB:%d", result_code);
  }
  return result_code;
}

/** This function checks the KT_VBR_PORTMAP parent is unified vbridge
*/
upll_rc_t VbrPortMapMoMgr::ValidateParentIsUnVbr(IpcReqRespHeader *req,
    ConfigKeyVal *ikey, ConfigKeyVal *pkey,
    DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  MoMgrImpl *mgr = reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>(
        GetMoManager(UNC_KT_VBRIDGE)));

  if (!mgr) {
    UPLL_LOG_DEBUG("Unable to get KT_VBRIDGE instace");
    return UPLL_RC_ERR_GENERIC;
  }

  DbSubOp dbop = {kOpReadSingle, kOpMatchNone, kOpInOutFlag};
  result_code = mgr->ReadConfigDB(pkey,  UPLL_DT_CANDIDATE, UNC_OP_READ,
      dbop, dmi, MAINTBL);

  if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    UPLL_LOG_DEBUG("Parent is not exist");
    return UPLL_RC_ERR_PARENT_DOES_NOT_EXIST;
  } else if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("vBridge ReadConfigDB failed:%d", result_code);
    return result_code;
  }

  val_vbr *vbr_val = static_cast<val_vbr*>(GetVal(pkey));
  if (!vbr_val) {
    UPLL_LOG_DEBUG("val structure from DB is NULL");
    DELETE_IF_NOT_NULL(pkey);
    return UPLL_RC_ERR_GENERIC;
  }
  // check the parent vbridge is unified vbridge
  if (!IsUnifiedVbr(vbr_val->controller_id)) {
    UPLL_LOG_DEBUG("Parent is not unified vbridge:%s", reinterpret_cast<char *>(
          vbr_val->controller_id));
    return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_KT;
  }
  return UPLL_RC_SUCCESS;
}

/** This function does the convert vnode creation for the configured vbr portmap
*/
upll_rc_t VbrPortMapMoMgr::ConvertVNode(ConfigKeyVal *ikey,
    DalDmlIntf *dmi,
    TcConfigMode config_mode,
    string vtn_name,
    IpcReqRespHeader *req) {
  UPLL_FUNC_TRACE;

  bool same_ctrlr_dom = false;
  uint32_t distinct_ctrlr_dom_count = 0;
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  // count the number of vbridge portmap with distinct controller/domain for
  // given vtn, unified vbridge
  result_code = CompareCtrlrDomainInReqWithDB(ikey, dmi, config_mode, vtn_name,
      &same_ctrlr_dom, &distinct_ctrlr_dom_count);

  if (result_code != UPLL_RC_SUCCESS) {
    return result_code;
  }

  if (same_ctrlr_dom) {
    UPLL_LOG_TRACE("same_ctrlr_dom exists in ca_vbr_portmap_tbl");
    // vbridge portmap with same controller and domain for
    // given unified vbridge exists already, no need to do any conversion
    // get the flag information from convert_vbr table
    MoMgrImpl *vbr_momgr = reinterpret_cast<MoMgrImpl *>(
             const_cast<MoManager *>(GetMoManager(UNC_KT_VBRIDGE)));
    ConfigKeyVal *convert_vbr_ckv = NULL;
    result_code = vbr_momgr->GetChildConvertConfigKey(convert_vbr_ckv, ikey);
    if (result_code != UPLL_RC_SUCCESS)
      return result_code;

    DbSubOp dbop = {kOpReadSingle, kOpMatchCtrlr|kOpMatchDomain, kOpInOutFlag};
    result_code = vbr_momgr->ReadConfigDB(convert_vbr_ckv, UPLL_DT_CANDIDATE,
                                          UNC_OP_READ, dbop, dmi, CONVERTTBL);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("converted vbr table read failed");
      DELETE_IF_NOT_NULL(convert_vbr_ckv);
      return result_code;
    }
    uint8_t convert_vbr_flag = 0;
    GET_USER_DATA_FLAGS(convert_vbr_ckv, convert_vbr_flag);
    uint8_t ikey_flag = 0;
    GET_USER_DATA_FLAGS(ikey, ikey_flag);
    ikey_flag |= convert_vbr_flag;
    SET_USER_DATA_FLAGS(ikey, ikey_flag);
    DELETE_IF_NOT_NULL(convert_vbr_ckv);
    return UPLL_RC_SUCCESS;
  }

  ConfigKeyVal *temp_ckv = NULL;
  if (distinct_ctrlr_dom_count == 0) {
    result_code = IsUnwExists(dmi, config_mode);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_TRACE("IsUnwExists failed:%d", result_code);
      return result_code;
    }
    // first-convert vbr is going to be created for given vtn, unified vbridge
    // allocate vbid
    uint32_t vbid = 0;
    if (static_cast<val_vbr_portmap*>(GetVal(ikey))->label == ANY_VLAN_ID) {
      vbid = ANY_VLAN_ID;
    } else {
      result_code = AllocVbid(ikey, vbid, dmi, config_mode, vtn_name);
      if (result_code != UPLL_RC_SUCCESS)
        return result_code;
    }
    UPLL_LOG_TRACE("Allocated vbid:%d", vbid);

    // create convert vbr entry
    ConfigKeyVal *vbr_ckv = NULL;
    result_code = CreateVbr(ikey, req, dmi, config_mode, vtn_name, vbid,
                            vbr_ckv);
    DELETE_IF_NOT_NULL(vbr_ckv);
    return result_code;
  }

  if (distinct_ctrlr_dom_count == 1) {
    UPLL_LOG_TRACE("distinct_ctrlr_dom_count is 1");
    return CreateVtunnelAndOtherVnode(ikey, req, dmi, config_mode, vtn_name);
  }

  UPLL_LOG_TRACE("distinct_ctrlr_dom_count is more than 1");
  // if count is more than one then get the required parameter(vbid,
  // vtunnel_name, ctrlr, domain) to invoke
  // CreateConvertVnodeExceptVtunnel

  // get the vbid
  uint32_t vbid = 0;
  result_code = GetVbid(ikey, dmi, &vbid);
  if (result_code != UPLL_RC_SUCCESS) {
    return result_code;
  }
  // get the vtunnel data
  result_code = GetVtunnelData(ikey, dmi, temp_ckv);
  if (result_code != UPLL_RC_SUCCESS)
    return result_code;

  result_code = CreateConvertVnodeExceptVtunnel(ikey, req, dmi, config_mode,
      vtn_name, temp_ckv,
      vbid);
  DELETE_IF_NOT_NULL(temp_ckv);
  return result_code;
}

/** This function checks whether the unw existance in candidate if global
 * config mode and in running in case of VTN config mode
 *
 */
upll_rc_t VbrPortMapMoMgr::IsUnwExists(DalDmlIntf *dmi,
                                       TcConfigMode config_mode) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  // TODO(U17+) may need to return the unw ckv to caller to know the
  // routing type
  ConfigKeyVal *unw_ckv = NULL;
  MoMgrImpl *unw_momgr = reinterpret_cast<MoMgrImpl *>(
      const_cast<MoManager *>(GetMoManager(UNC_KT_UNIFIED_NETWORK)));

  DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutNone };

  result_code = unw_momgr->GetChildConfigKey(unw_ckv, NULL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_ERROR("GetChildConfigKey failed:%d", result_code);
    return result_code;
  }

  val_unified_nw *unified_nw_val = ConfigKeyVal::Malloc<val_unified_nw>();
  unified_nw_val->valid[UPLL_IDX_ROUTING_TYPE_UNW] = UNC_VF_VALID;
  unified_nw_val->routing_type = UPLL_ROUTING_TYPE_QINQ_TO_QINQ;
  unw_ckv->AppendCfgVal(new ConfigVal(IpctSt::kIpcStValUnifiedNw,
                                      unified_nw_val));

  // check unified network exists in candidate DB
  result_code = unw_momgr->ReadConfigDB(unw_ckv, UPLL_DT_CANDIDATE,
                                        UNC_OP_READ, dbop, dmi, MAINTBL);
  if ((result_code != UPLL_RC_SUCCESS) &&
      (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE)) {
    UPLL_LOG_ERROR("ReadConfigDB failed %d", result_code);
    DELETE_IF_NOT_NULL(unw_ckv);
    return result_code;
  }
  if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    UPLL_LOG_DEBUG("Unified network is not configured");
    DELETE_IF_NOT_NULL(unw_ckv);
    return UPLL_RC_ERR_EXPAND;
  }
  if (config_mode == TC_CONFIG_VTN) {
    // check in running
    unw_ckv->DeleteCfgVal();
    result_code = unw_momgr->ReadConfigDB(unw_ckv, UPLL_DT_RUNNING,
                                          UNC_OP_READ, dbop, dmi, MAINTBL);
    DELETE_IF_NOT_NULL(unw_ckv);
    if ((result_code != UPLL_RC_SUCCESS) &&
        (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE)) {
      UPLL_LOG_ERROR("ReadConfigDB in running failed %d", result_code);
    }

    if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      UPLL_LOG_DEBUG("Unified network is not configured in Running");
      return UPLL_RC_ERR_EXPAND;
    }
  }

  DELETE_IF_NOT_NULL(unw_ckv);
  return result_code;
}


/** This function checks whether memory allocated for spd_ckv, if allocated
 * reads that entry from DB otherwise allocates the memory and does readmultiple
 * on given datatype configuration
 */
upll_rc_t VbrPortMapMoMgr::GetUnwSpineDomain(
    ConfigKeyVal **spd_ckv, DalDmlIntf *dmi, upll_keytype_datatype_t datatype) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  bool spd_ckv_alloc = false;

  MoMgrImpl *unw_sd_momgr = reinterpret_cast<MoMgrImpl *>(
      const_cast<MoManager *>(GetMoManager(UNC_KT_UNW_SPINE_DOMAIN)));

  DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutNone };
  uint32_t rep_count = 0;

  if (*spd_ckv == NULL) {
    result_code = unw_sd_momgr->GetChildConfigKey(*spd_ckv, NULL);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_ERROR("GetChildConfigKey failed:%d", result_code);
      return result_code;
    }
    spd_ckv_alloc = true;
    dbop.readop = kOpReadMultiple;
  }

  // check unified network exists in candidate DB
  result_code = unw_sd_momgr->ReadConfigDB(*spd_ckv, datatype,
      UNC_OP_READ, dbop, rep_count, dmi, MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    if (spd_ckv_alloc) {
      DELETE_IF_NOT_NULL(*spd_ckv);
    }
    if (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      UPLL_LOG_ERROR("ReadConfigDB failed %d", result_code);
    }
  }
  return result_code;
}


/** This function reads vbr portmap candidate table for the entry which
 *  has logical_port_id valid and gets the vlanid for given vtn
 */
upll_rc_t VbrPortMapMoMgr::GetVbrPortMap(ConfigKeyVal *ikey,
                                         IpcReqRespHeader *req,
                                         uint32_t *vlanid,
                                         DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;

  upll_rc_t result_code = UPLL_RC_SUCCESS;

  DalBindInfo *db_info = new DalBindInfo(uudst::kDbiVbrPortMapTbl);

  db_info->BindMatch(uudst::vbridge_portmap::kDbiVtnName,
                     uud::kDalChar, (kMaxLenVtnName + 1),
                     reinterpret_cast<void *>(reinterpret_cast
                     <key_vbr_portmap*>(ikey->get_key())
                      ->vbr_key.vtn_key.vtn_name));

  uint8_t logical_port_valid = UNC_VF_VALID;

  db_info->BindMatch(
      uudst::vbridge_portmap::kDbiValidLogicalPortId, uud::kDalUint8,
      1, &logical_port_valid);
  std::string query_string = "";
  if (req->datatype == UPLL_DT_IMPORT) {
    query_string =  QUERY_READ_VBR_PORTMAP_LABEL_IMPORT;
  } else  {
    query_string =  QUERY_READ_VBR_PORTMAP_LABEL;
  }

  db_info->BindOutput(uudst::vbridge_portmap::kDbiLabel,
      uud::kDalUint32, 1, vlanid);

  // execute query
  result_code = DalToUpllResCode(dmi->ExecuteAppQuerySingleRecord(
        query_string, db_info));

  DELETE_IF_NOT_NULL(db_info);

  return result_code;
}



/** This function reads convert vbr candidate table and gets the vbid for given
 * vtn, unified vbr name */
upll_rc_t VbrPortMapMoMgr::GetVbid(ConfigKeyVal *ikey, DalDmlIntf *dmi,
    uint32_t *vbid) {
  ConfigKeyVal *convert_vbr_ckv = NULL;
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  MoMgrImpl *vbr_momgr = reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>(
        GetMoManager(UNC_KT_VBRIDGE)));

  result_code = vbr_momgr->GetChildConvertConfigKey(convert_vbr_ckv, ikey);
  if (result_code != UPLL_RC_SUCCESS)
    return result_code;

  DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutNone };
  result_code = vbr_momgr->ReadConfigDB(convert_vbr_ckv, UPLL_DT_CANDIDATE,
      UNC_OP_READ, dbop, dmi, CONVERTTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    if (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      UPLL_LOG_DEBUG("converted vbr table read failed");
    }
    DELETE_IF_NOT_NULL(convert_vbr_ckv);
    return result_code;
  }
  *vbid = static_cast<val_convert_vbr*>(GetVal(convert_vbr_ckv))->label;
  DELETE_IF_NOT_NULL(convert_vbr_ckv);
  return UPLL_RC_SUCCESS;
}

/** This function checks
 * 1. SpineDomainId exists in vtn_unified_tbl
 * 2. If not exists, based on available label count algortihm in spine domain
 * table gets one spine domain id
 */
upll_rc_t VbrPortMapMoMgr::GetSpineDomainId(ConfigKeyVal *ikey,
    ConfigKeyVal *&spine_dom_ckv, DalDmlIntf *dmi,
    TcConfigMode config_mode, string vtn_name) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  // check the vtn_unified_tbl
  MoMgrImpl *vtn_un_momgr = reinterpret_cast<MoMgrImpl *>
    (const_cast<MoManager *>(GetMoManager(UNC_KT_VTN_UNIFIED)));
  if (!vtn_un_momgr) {
    UPLL_LOG_DEBUG("Invalid Mgr");
    return UPLL_RC_ERR_GENERIC;
  }

  // frame the vtn_unified ckv
  ConfigKeyVal *vtn_un_ckv = NULL;
  result_code = vtn_un_momgr->GetChildConfigKey(vtn_un_ckv, NULL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("vtn unified GetChildConfigKey failed:%d", result_code);
    return result_code;
  }

  // fill the vtn_name in key_vtn_unified from key_vbr_portmap
  uuu::upll_strncpy(static_cast<key_vtn_unified*>(
        vtn_un_ckv->get_key())->vtn_key.vtn_name, static_cast<
      key_vbr_portmap*>(ikey->get_key())->vbr_key.vtn_key.vtn_name,
      kMaxLenVtnName+1);

  // check the entry in vtn_unified tbl
  DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutNone };
  result_code = vtn_un_momgr->ReadConfigDB(
      vtn_un_ckv, UPLL_DT_CANDIDATE, UNC_OP_READ, dbop, dmi, MAINTBL);

  // if entry is not exists, use the available label algorithm and
  // get spinedomainId
  if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    DELETE_IF_NOT_NULL(vtn_un_ckv);
    return GetSpineDomainIdBasedOnAvailableLabel(spine_dom_ckv, dmi,
        config_mode, vtn_name);
  } else  if (result_code != UPLL_RC_SUCCESS) {
    // if DB failure, return error
    DELETE_IF_NOT_NULL(vtn_un_ckv);
    UPLL_LOG_DEBUG("vtn_unified read DB error:%d", result_code);
    return result_code;
  } else {
    // if entry exists in vtn_unified tbl, get the spinedomainId
    // from vtn_unified_tbl read response
    val_vtn_unified *val_vtn_un = static_cast<val_vtn_unified*>(GetVal(
          vtn_un_ckv));
    if ((val_vtn_un == NULL) || (val_vtn_un->valid[UPLL_IDX_SPINE_ID_VUNW]
          != UNC_VF_VALID)) {
      DELETE_IF_NOT_NULL(vtn_un_ckv);
      UPLL_LOG_TRACE("spine domain id is not configured in vtn_unified_tbl,"
          " use available label algorithm and get spinedomainId");
      return GetSpineDomainIdBasedOnAvailableLabel(spine_dom_ckv, dmi,
          config_mode, vtn_name);
    }
    // read the spinedomain table and fill spine_dom_ckv based on the unified
    // network id and spinedomainId in read response
    MoMgrImpl *unw_sd_momgr = reinterpret_cast<MoMgrImpl *>
      (const_cast<MoManager *>(GetMoManager(UNC_KT_UNW_SPINE_DOMAIN)));

    if (NULL == unw_sd_momgr) {
      UPLL_LOG_DEBUG("Unable to get unw_spine_domain instance");
      return UPLL_RC_ERR_GENERIC;
    }

    result_code = unw_sd_momgr->GetChildConfigKey(spine_dom_ckv, NULL);
    if (result_code != UPLL_RC_SUCCESS)
      return result_code;

    key_unw_spine_domain *key_unw_sd = static_cast<key_unw_spine_domain*>(
        spine_dom_ckv->get_key());

    uuu::upll_strncpy(key_unw_sd->unw_key.unified_nw_id,
        static_cast<key_vtn_unified*>(
          vtn_un_ckv->get_key())->unified_nw_id,
        kMaxLenUnwName+1);
    uuu::upll_strncpy(key_unw_sd->unw_spine_id, val_vtn_un->spine_id,
        kMaxLenSpineName+1);

    DELETE_IF_NOT_NULL(vtn_un_ckv);

    DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutNone };
    result_code = unw_sd_momgr->ReadConfigDB(spine_dom_ckv, UPLL_DT_CANDIDATE,
        UNC_OP_READ, dbop, dmi, MAINTBL);

    if ((result_code == UPLL_RC_SUCCESS) && (config_mode == TC_CONFIG_VTN)) {
      // check in Running DB if the PCM is VTN
      spine_dom_ckv->DeleteCfgVal();
      result_code = unw_sd_momgr->ReadConfigDB(spine_dom_ckv, UPLL_DT_RUNNING,
          UNC_OP_READ, dbop, dmi, MAINTBL);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Specified spine id in VTN_UNIFIED is not exists "
            "in RUNNING");
        return UPLL_RC_ERR_EXPAND;
      }
    }
    if (result_code != UPLL_RC_SUCCESS) {
      DELETE_IF_NOT_NULL(spine_dom_ckv);
      UPLL_LOG_DEBUG("Spine domain read failed:%d", result_code);
      return result_code;
    }

    // check whether used_count exceeds the max_count
    result_code = CheckUsedCountExceedsMaxCount(spine_dom_ckv, config_mode,
                                                dmi);
    if (result_code != UPLL_RC_SUCCESS) {
      DELETE_IF_NOT_NULL(spine_dom_ckv);
      UPLL_LOG_DEBUG("CheckUsedCountExceedsMaxCount failed:%d", result_code);
      return result_code;
    }
    return result_code;
  }
}

/** This function increments the used_label_count for given ckv and updates
 * spine domain table*/
upll_rc_t VbrPortMapMoMgr::IncrementUsedLabelCount(
    ConfigKeyVal *spd_ckv, uint8_t *ikey_vtnid, DalDmlIntf *dmi,
    TcConfigMode config_mode, string vtn_name) {
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  MoMgrImpl *unw_sd_momgr = reinterpret_cast<MoMgrImpl *>
    (const_cast<MoManager *>(GetMoManager(UNC_KT_UNW_SPINE_DOMAIN)));

  if (NULL == unw_sd_momgr) {
    UPLL_LOG_DEBUG("Unable to get unw_spine_domain instance");
    return UPLL_RC_ERR_GENERIC;
  }
  if (config_mode == TC_CONFIG_VTN) {
    // get the spine domain details from candidate
    spd_ckv->DeleteCfgVal();
    DbSubOp dbop_read = {kOpReadSingle, kOpMatchNone, kOpInOutFlag};
    result_code = unw_sd_momgr->ReadConfigDB(spd_ckv, UPLL_DT_CANDIDATE,
                                             UNC_OP_READ, dbop_read, dmi,
                                             MAINTBL);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_ERROR("spine domain candidate tbl read failed:%d", result_code);
      return result_code;
    }
  }
  // Increment the used_label_count in spine_dom_ckv
  val_unw_spdom_ext *spdom_ext_val = static_cast<val_unw_spdom_ext*>(
      GetVal(spd_ckv));

  if (NULL == spdom_ext_val) {
    UPLL_LOG_DEBUG("unw_spine_domain extend structure read from DB is NULL");
    return UPLL_RC_ERR_GENERIC;
  }

  spdom_ext_val->used_label_count++;
  spdom_ext_val->valid[UPLL_IDX_SPINE_USED_LABEL_COUNT_UNWS] = UNC_VF_VALID;

  DbSubOp dbop = {kOpNotRead, kOpMatchNone, kOpInOutNone};
  result_code = unw_sd_momgr->UpdateConfigDB(spd_ckv, UPLL_DT_CANDIDATE,
      UNC_OP_UPDATE, dmi, &dbop, config_mode, vtn_name, MAINTBL);
  if (result_code == UPLL_RC_SUCCESS) {
     result_code = UpdateUsedCntInScratchTbl(spd_ckv, ikey_vtnid,
                     UNC_OP_CREATE, dmi, config_mode, vtn_name);
     if (result_code != UPLL_RC_SUCCESS) {
       UPLL_LOG_TRACE("UpdateUsedCntInScratchTbl failed:%d", result_code);
       return result_code;
     }
  }
  return result_code;
}


/** This function gets the spine domain ckv from db based on vtunnel ckv
 *  controller and domain and decrements the used_label_count
 *  in spine domain ckv and updates in DB
 */
upll_rc_t VbrPortMapMoMgr::DecrementUsedLabelCount(
    ConfigKeyVal *vtun_ckv, uint8_t *ikey_vtn, DalDmlIntf *dmi,
    TcConfigMode config_mode, string vtn_name) {
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  MoMgrImpl *unw_sd_momgr = reinterpret_cast<MoMgrImpl *>(
      const_cast<MoManager *>(GetMoManager(UNC_KT_UNW_SPINE_DOMAIN)));
  if (NULL == unw_sd_momgr) {
    UPLL_LOG_DEBUG("Unable to get unw_spine_domain instance");
    return UPLL_RC_ERR_GENERIC;
  }

  // read the spine domain based on vtunnel_ckv ctrlr, domain
  // vtun_ckv key_user_data set in GetVtunnelData
  controller_domain ctrlr_domain = {NULL, NULL};
  GET_USER_DATA_CTRLR_DOMAIN(vtun_ckv, ctrlr_domain);

  if ((NULL == ctrlr_domain.ctrlr) || (NULL == ctrlr_domain.domain)) {
    UPLL_LOG_DEBUG("ctrlr/domain get from vtunnel ckv is NULL");
    return UPLL_RC_ERR_GENERIC;
  }

  ConfigKeyVal *unw_spd_ckv = NULL;
  result_code = unw_sd_momgr->GetChildConfigKey(unw_spd_ckv, NULL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("UnwSpd GetChildConfigKey failed:%d", result_code);
    return result_code;
  }
  // fill the val structure with given ctrlr, domain
  val_unw_spdom_ext *val_unw_sd_ext = ConfigKeyVal::Malloc<val_unw_spdom_ext>();

  val_unw_sd_ext->val_unw_spine_dom.valid
    [UPLL_IDX_SPINE_DOMAIN_ID_UNWS] = UNC_VF_VALID;
  val_unw_sd_ext->val_unw_spine_dom.valid[
    UPLL_IDX_SPINE_CONTROLLER_ID_UNWS] = UNC_VF_VALID;
  uuu::upll_strncpy(val_unw_sd_ext->val_unw_spine_dom.spine_controller_id,
      ctrlr_domain.ctrlr, kMaxLenCtrlrId+1);
  uuu::upll_strncpy(val_unw_sd_ext->val_unw_spine_dom.spine_domain_id,
      ctrlr_domain.domain, kMaxLenDomainId+1);
  unw_spd_ckv->AppendCfgVal(IpctSt::kIpctStValUnwSpineDomain_Ext,
                            val_unw_sd_ext);

  // read spine domsin tbl
  DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutNone };
  result_code = unw_sd_momgr->ReadConfigDB(unw_spd_ckv, UPLL_DT_CANDIDATE,
      UNC_OP_READ, dbop, dmi, MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    DELETE_IF_NOT_NULL(unw_spd_ckv);
    UPLL_LOG_DEBUG("unw spine domain db read failed for given ctrlr, domain:%d",
        result_code);
    return result_code;
  }

  // decrement the used label count
  val_unw_sd_ext = static_cast<val_unw_spdom_ext*>(
      GetVal(unw_spd_ckv));

  val_unw_sd_ext->used_label_count--;
  val_unw_sd_ext->valid[UPLL_IDX_SPINE_USED_LABEL_COUNT_UNWS] = UNC_VF_VALID;

  // Update the used label count in spine domain DB
  DbSubOp dbop_update = {kOpNotRead, kOpMatchNone, kOpInOutNone};
  result_code = unw_sd_momgr->UpdateConfigDB(unw_spd_ckv, UPLL_DT_CANDIDATE,
      UNC_OP_UPDATE, dmi, &dbop_update, config_mode, vtn_name, MAINTBL);
  if (result_code == UPLL_RC_SUCCESS) {
    result_code = UpdateUsedCntInScratchTbl(unw_spd_ckv, ikey_vtn,
        UNC_OP_DELETE, dmi, config_mode, vtn_name);
    if (result_code != UPLL_RC_SUCCESS) {
      DELETE_IF_NOT_NULL(unw_spd_ckv);
      UPLL_LOG_TRACE("UpdateUsedCntInScratchTbl failed:%d", result_code);
      return result_code;
    }
  }
  DELETE_IF_NOT_NULL(unw_spd_ckv);
  return result_code;
}

/** This function reads all spine domain table records
 * and takes the difference between max_count and used_label_count
 * and returns the spine domain ckv which has maximum available label
 * count(difference value is more)
 */
upll_rc_t VbrPortMapMoMgr::GetSpineDomainIdBasedOnAvailableLabel(
    ConfigKeyVal *&unw_spine_dom_ckv, DalDmlIntf *dmi,
    TcConfigMode config_mode, string vtn_name) {
  UPLL_FUNC_TRACE;

  upll_rc_t result_code = UPLL_RC_SUCCESS;

  MoMgrImpl *unw_sd_momgr = reinterpret_cast<MoMgrImpl *>(
      const_cast<MoManager *>(GetMoManager(UNC_KT_UNW_SPINE_DOMAIN)));

  if (NULL == unw_sd_momgr) {
    UPLL_LOG_DEBUG("Unable to get unw_spine_domain instance");
    return UPLL_RC_ERR_GENERIC;
  }

  // Read all the records in spine domain table
  ConfigKeyVal *unw_spd_ckv = NULL;
  result_code = unw_sd_momgr->GetChildConfigKey(unw_spd_ckv, NULL);

  DbSubOp dbop = { kOpReadMultiple, kOpMatchNone, kOpInOutNone };
  result_code = unw_sd_momgr->ReadConfigDB(unw_spd_ckv, UPLL_DT_CANDIDATE,
      UNC_OP_READ, dbop, dmi, MAINTBL);

  if (result_code != UPLL_RC_SUCCESS) {
    if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      result_code = UPLL_RC_ERR_EXPAND;
    }
    DELETE_IF_NOT_NULL(unw_spd_ckv);
    UPLL_LOG_DEBUG("Spine domain table ReadMultiple failed:%d", result_code);
    return result_code;
  }

  // iterate the DB read response and find the spine dom ckv which has max
  // available label count
  ConfigKeyVal *tmp = unw_spd_ckv;
  result_code = IterateSpineDomainCkvAndFindOne(tmp, unw_spine_dom_ckv, dmi,
                                                config_mode);
  DELETE_IF_NOT_NULL(unw_spd_ckv);
  tmp = NULL;

  return result_code;
}

/** This function iterates the spine domain ckv one by one and finds and assigns
 * the spine domain ckv to spd_tmp_ckv which has max available label count
 */
upll_rc_t VbrPortMapMoMgr::IterateSpineDomainCkvAndFindOne(
    ConfigKeyVal *tmp_ckv,
    ConfigKeyVal *&spd_tmp_ckv,
    DalDmlIntf *dmi,
    TcConfigMode config_mode) {
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  int32_t cur_avail_label_ct = 0;
  uint32_t used_label_count = 0;
  ConfigKeyVal *label_ckv = NULL;

  std::multimap<uint32_t, ConfigKeyVal*> spd_ckv_avail_map;

  for (; tmp_ckv != NULL; tmp_ckv = tmp_ckv->get_next_cfg_key_val()) {
    // get the used count from candidate
    if (static_cast<val_unw_spdom_ext*>(GetVal(tmp_ckv))->
        valid[UPLL_IDX_SPINE_USED_LABEL_COUNT_UNWS] == UNC_VF_VALID)  {
      used_label_count = static_cast<val_unw_spdom_ext*>(
          GetVal(tmp_ckv))->used_label_count;
    } else {
      used_label_count = 0;
    }
    if (config_mode == TC_CONFIG_VTN) {
      // in case of PCM choose the spine domain which is common in candidate
      //  and  running exclude label attribute
      static_cast<val_unw_spdom_ext*>(GetVal(tmp_ckv))->val_unw_spine_dom.
        valid[UPLL_IDX_UNW_LABEL_ID_UNWS] = UNC_VF_INVALID;
      static_cast<val_unw_spdom_ext*>(GetVal(tmp_ckv))->valid[
          UPLL_IDX_SPINE_USED_LABEL_COUNT_UNWS] = UNC_VF_INVALID;
      result_code = IsUnwSpdExistsInRunningDB(tmp_ckv, dmi);
      if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
        result_code = UPLL_RC_SUCCESS;
        continue;
      }
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Unw spine domain is not exists in Running DB");
        return UPLL_RC_ERR_EXPAND;
      }
    }

    // get the label id and used_label_count from spine domain ckv
    // in case of PCM, need to get the running max_count
    val_unw_spdom_ext  *val_unw_spd_ext = static_cast<val_unw_spdom_ext*>(
        GetVal(tmp_ckv));

    if (NULL == val_unw_spd_ext) {
      UPLL_LOG_DEBUG(" unw_spine_domain val structure in DB is NULL");
      return UPLL_RC_ERR_GENERIC;
    }

    if (val_unw_spd_ext->val_unw_spine_dom.valid[UPLL_IDX_UNW_LABEL_ID_UNWS] !=
        UNC_VF_VALID) {
      // label is not configured take the default max_count 4000;
      cur_avail_label_ct = kDefaultMaxCount - used_label_count;
      if (cur_avail_label_ct > 0) {
        spd_ckv_avail_map.insert(std::make_pair(cur_avail_label_ct, tmp_ckv));
      }
      continue;
    }
    result_code = GetUnwLabelCkv(tmp_ckv, label_ckv, config_mode,
        dmi);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("UNW label table read failed:%d", result_code);
      return result_code;
    }

    val_unw_label *db_unw_label_val = static_cast<val_unw_label*>(
        GetVal(label_ckv));
    if ((NULL == db_unw_label_val) || (
          db_unw_label_val->valid[UPLL_IDX_MAX_COUNT_UNWL] != UNC_VF_VALID)) {
      UPLL_LOG_TRACE("max_count is not configured, take default max_count");

      cur_avail_label_ct = kDefaultMaxCount - used_label_count;
      if (cur_avail_label_ct > 0) {
        spd_ckv_avail_map.insert(std::make_pair(cur_avail_label_ct, tmp_ckv));
      }
      DELETE_IF_NOT_NULL(label_ckv);
      continue;
    }
    // max_count is configured, take the max_count value for comparison
    cur_avail_label_ct = db_unw_label_val->max_count - used_label_count;
    if (cur_avail_label_ct > 0) {
      spd_ckv_avail_map.insert(std::make_pair(cur_avail_label_ct, tmp_ckv));
    }
    DELETE_IF_NOT_NULL(label_ckv);
  }

  if (spd_ckv_avail_map.empty()) {
    UPLL_LOG_TRACE("No spine domain available");
    return UPLL_RC_ERR_EXCEEDS_RESOURCE_LIMIT;
  }

  MoMgrImpl *unw_sd_momgr = reinterpret_cast<MoMgrImpl *>(
      const_cast<MoManager *>(GetMoManager(UNC_KT_UNW_SPINE_DOMAIN)));

  std::multimap<uint32_t, ConfigKeyVal*>::reverse_iterator rit;
  for (rit = spd_ckv_avail_map.rbegin(); rit != spd_ckv_avail_map.rend();
      ++rit) {
    ConfigKeyVal *dup_spd_ckv = NULL;
    result_code = unw_sd_momgr->DupConfigKeyVal(dup_spd_ckv,
        (*rit).second);
    if (result_code != UPLL_RC_SUCCESS) {
      // spd_ckv will be released in GetSpineDomainIdBasedOnAvailableLabel
      DELETE_IF_NOT_NULL(spd_tmp_ckv);
      UPLL_LOG_TRACE("DupConfigKeyVal failed:%d", result_code);
      return result_code;
    }
    // set NULL to next_cfg_val
    dup_spd_ckv->set_next_cfg_key_val(NULL);
    if (spd_tmp_ckv) {
      spd_tmp_ckv->AppendCfgKeyVal(dup_spd_ckv);
    } else {
      spd_tmp_ckv = dup_spd_ckv;
    }
  }
  // release the list memory
  return result_code;
}


/* This function reads the given spd_ckv from running db
 */
upll_rc_t VbrPortMapMoMgr::IsUnwSpdExistsInRunningDB(ConfigKeyVal *unw_sd_ckv,
                                                      DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  MoMgrImpl *unw_sd_momgr = reinterpret_cast<MoMgrImpl *>(
      const_cast<MoManager *>(GetMoManager(UNC_KT_UNW_SPINE_DOMAIN)));

  DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutNone };
  // check unified network exists in candidate DB
  result_code = unw_sd_momgr->ReadConfigDB(unw_sd_ckv, UPLL_DT_RUNNING,
      UNC_OP_READ, dbop, dmi, MAINTBL);
  if ((result_code != UPLL_RC_SUCCESS) &&
      (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE)) {
    UPLL_LOG_ERROR("Unw spine domain running DB read failed:%d",
                   result_code);
    return result_code;
  }
  return result_code;
}

/*This function is invoked when 2nd vbr portmap with logical port id with
 * distinct controller - domain configured to create converted vnode
 * including vtunnel
 */
upll_rc_t VbrPortMapMoMgr::CreateVtunnelAndOtherVnode(ConfigKeyVal *ikey,
    IpcReqRespHeader *req, DalDmlIntf *dmi,
    TcConfigMode config_mode,
    string vtn_name) {
  UPLL_FUNC_TRACE;

  upll_rc_t result_code = UPLL_RC_SUCCESS;

  // 1. check converted vtunnel exists for given vtn
  // Increment the vtunnel ref_count if exists otherwise create the vtunnel
  // entry for the given vtn
  DbSubOp dbop = {kOpReadSingle, kOpMatchNone,
                  kOpInOutCtrlr|kOpInOutDomain|kOpInOutFlag};

  ConfigKeyVal *vtun_ckv = NULL;
  result_code = CreateVtunnel(ikey, dmi, config_mode, vtn_name, vtun_ckv);
  if (result_code != UPLL_RC_SUCCESS)
    return result_code;

  uint32_t gvtnid = static_cast<val_convert_vtunnel*>(GetVal(vtun_ckv))->label;
  UPLL_LOG_TRACE("gvtnid = %d", gvtnid);

  // 2. get the converted vbridge created for the first vbridge portmap
  ConfigKeyVal *vbr_ckv = NULL;
  MoMgrImpl *vbr_momgr = reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>(
        GetMoManager(UNC_KT_VBRIDGE)));

  result_code = vbr_momgr->GetChildConvertConfigKey(vbr_ckv, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetChildConvertConfigKey for convert vbr failed:%d",
        result_code);
    DELETE_IF_NOT_NULL(vtun_ckv);
    return result_code;
  }

  result_code = vbr_momgr->ReadConfigDB(vbr_ckv, UPLL_DT_CANDIDATE, UNC_OP_READ,
      dbop, dmi, CONVERTTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("converted vbr table read failed");
    DELETE_IF_NOT_NULL(vbr_ckv);
    DELETE_IF_NOT_NULL(vtun_ckv);
    return result_code;
  }

  // 3. create converted vtunnel if to create the vlink for
  // the existing converted vbr
  ConfigKeyVal *vtun_if_ckv = NULL;
  uint8_t *un_vbr_name = static_cast<key_vbr_portmap*>(
      ikey->get_key())->vbr_key.vbridge_name;
  result_code = CreateVtunnelIf(vtun_ckv, vbr_ckv, un_vbr_name, vtun_if_ckv,
                                config_mode, vtn_name, dmi);

  if (result_code != UPLL_RC_SUCCESS) {
    DELETE_IF_NOT_NULL(vtun_ckv);
    DELETE_IF_NOT_NULL(vbr_ckv);
    return result_code;
  }

  // 4. create converted vbr if for existing converted vbridge
  ConfigKeyVal *vbr_if_ckv = NULL;
  result_code = CreateVbrIf(vbr_ckv, vtun_if_ckv, dmi, config_mode, vtn_name,
      vbr_if_ckv);

  if (result_code != UPLL_RC_SUCCESS) {
    DELETE_IF_NOT_NULL(vtun_ckv);
    DELETE_IF_NOT_NULL(vbr_ckv);
    DELETE_IF_NOT_NULL(vtun_if_ckv);
    return result_code;
  }

  // get the rename flag from vbr_ckv if any
  uint8_t rename_flag = 0;
  GET_USER_DATA_FLAGS(vbr_ckv, rename_flag);
  if (rename_flag & VTN_RENAME) {
    SET_USER_DATA_FLAGS(vbr_if_ckv, VTN_RENAME);
  }

  // Read converted vbr and get vbid
  uint32_t vbid = static_cast<val_convert_vbr*>(GetVal(vbr_ckv))->label;
  DELETE_IF_NOT_NULL(vbr_ckv);
  // 5. create converted vlink for existing converted vbridge
  result_code = CreateVlink(vbr_if_ckv, vtun_if_ckv, dmi, config_mode, vtn_name,
      gvtnid);
  DELETE_IF_NOT_NULL(vtun_if_ckv);
  DELETE_IF_NOT_NULL(vbr_if_ckv);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Convert Vlink creation failed:%d", result_code);
    DELETE_IF_NOT_NULL(vtun_ckv);
    return result_code;
  }

  // 6. create converted vnode table entry for given vbridge portmap
  result_code = CreateConvertVnodeExceptVtunnel(ikey, req, dmi, config_mode,
      vtn_name, vtun_ckv, vbid);
  DELETE_IF_NOT_NULL(vtun_ckv);
  return result_code;
}

/** This function
 * 1. creates the entry in vtunnel table if the entry is not
 * exists for given vtn
 * 2. If entry exists increments the ref_count and updates the DB
 * Note: convert vtunnel table ref_count indicates how many unified vbridges
 * refers this vtunnel
 */
upll_rc_t VbrPortMapMoMgr::CreateVtunnel(ConfigKeyVal *ikey, DalDmlIntf *dmi,
    TcConfigMode config_mode, string vtn_name,
    ConfigKeyVal *&vtun_ckv) {
  UPLL_FUNC_TRACE;

  upll_rc_t result_code = UPLL_RC_SUCCESS;
  MoMgrImpl *vtun_momgr = reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>(
        GetMoManager(UNC_KT_VTUNNEL)));

  result_code = vtun_momgr->GetChildConvertConfigKey(vtun_ckv, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("vtunnel GetChildConvertConfigKey failed:%d", result_code);
    return result_code;
  }

  DbSubOp dbop = {kOpReadSingle, kOpMatchNone, kOpInOutCtrlr|kOpInOutDomain };

  result_code = vtun_momgr->ReadConfigDB(vtun_ckv, UPLL_DT_CANDIDATE,
       UNC_OP_READ,  dbop, dmi, CONVERTTBL);
  if ((result_code != UPLL_RC_SUCCESS) &&
      (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE)) {
    DELETE_IF_NOT_NULL(vtun_ckv);
    UPLL_LOG_DEBUG("read failed for convert vtunnel table:%d", result_code);
    return result_code;
  }

  // vtunnel is not exists for the given vtn, allocate gvtnid and create the
  // entry in convert vtunnel table

  if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    // get the spine_comain id to pass it to label allocation.
    ConfigKeyVal *spine_dom_ckv = NULL;
    result_code = GetSpineDomainId(ikey, spine_dom_ckv, dmi,
        config_mode, vtn_name);
    if (result_code != UPLL_RC_SUCCESS) {
      DELETE_IF_NOT_NULL(vtun_ckv);
      return result_code;
    }

    uint32_t gvtnid = 0;
    // 1.b Invoke AllocGvtnId and get GVTNID to use in Vtunnel creation
    uint8_t *ikey_vtnid = static_cast<key_vbr_portmap*>(ikey->get_key())->
                    vbr_key.vtn_key.vtn_name;
    result_code = AllocGvtnId(spine_dom_ckv, ikey_vtnid, gvtnid, dmi,
           config_mode, vtn_name);

    if (result_code != UPLL_RC_SUCCESS) {
      DELETE_IF_NOT_NULL(spine_dom_ckv);
      DELETE_IF_NOT_NULL(vtun_ckv);
      return result_code;
    }
    // update the spine domain table with the new used label count
    result_code = IncrementUsedLabelCount(spine_dom_ckv, ikey_vtnid, dmi,
               config_mode,  vtn_name);
    if (result_code != UPLL_RC_SUCCESS) {
      DELETE_IF_NOT_NULL(spine_dom_ckv);
      DELETE_IF_NOT_NULL(vtun_ckv);
      return result_code;
    }

    // fill the spine ctrlr, domain and label in convert vtunnel ckv
    val_convert_vtunnel *vtunnel_val = static_cast<val_convert_vtunnel *>
      (ConfigKeyVal::Malloc(sizeof(val_convert_vtunnel)));

    val_unw_spdom_ext_t *val_spd_ext = static_cast<
      val_unw_spdom_ext_t*>(GetVal(spine_dom_ckv));
    vtunnel_val->label = gvtnid;
    vtunnel_val->valid[UPLL_IDX_LABEL_CONV_VTNL] = UNC_VF_VALID;
    vtun_ckv->SetCfgVal(new ConfigVal(IpctSt::kIpcStValConvertVtunnel,
          vtunnel_val));
    SET_USER_DATA_CTRLR(vtun_ckv,
                    val_spd_ext->val_unw_spine_dom.spine_controller_id);
    SET_USER_DATA_DOMAIN(vtun_ckv,
                         val_spd_ext->val_unw_spine_dom.spine_domain_id);

    // 1.c create the converted vtunnel
    uint8_t *un_vbr_name = static_cast<key_vbr_portmap*>(
        ikey->get_key())->vbr_key.vbridge_name;
    result_code = vtun_momgr->ConvertVtunnel(vtun_ckv, un_vbr_name,
        UNC_OP_CREATE, config_mode, vtn_name, dmi);
    DELETE_IF_NOT_NULL(spine_dom_ckv);
    if (result_code != UPLL_RC_SUCCESS) {
      DELETE_IF_NOT_NULL(vtun_ckv);
      UPLL_LOG_DEBUG("ConvertVtunnel returns erro:%d", result_code);
    }
    return result_code;
  } else {
    // Vtunnel exists for other unified vbridges exists for that vtn,
    // increment the vtunnel ref_count
    val_convert_vtunnel *val_vtun = static_cast<val_convert_vtunnel*>(
        GetVal(vtun_ckv));
    val_vtun->ref_count++;
    result_code = vtun_momgr->UpdateConfigDB(vtun_ckv, UPLL_DT_CANDIDATE,
        UNC_OP_UPDATE, dmi, config_mode, vtn_name, CONVERTTBL);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("ref_count increment in convert vtunnel table failed:%d",
          result_code);
      DELETE_IF_NOT_NULL(vtun_ckv);
    }
    return result_code;
  }
}

/** This function constructs the convert vtunnel table ckv and invokes
 * vtunnelMoMgr convert function to auto generate if name and update the entry
 * in converted vtunnel if table
 */
upll_rc_t VbrPortMapMoMgr::CreateVtunnelIf(
    ConfigKeyVal *vtun_ckv, ConfigKeyVal *vbr_ckv, uint8_t *un_vbr_name,
    ConfigKeyVal *&vtun_if_ckv, TcConfigMode config_mode, string vtn_name,
    DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  // frame the convert vtunnel if configkeyval
  key_convert_vtunnel *vtun_key = static_cast<key_convert_vtunnel *>(
      vtun_ckv->get_key());

  key_convert_vtunnel_if *key_vtun_if = ConfigKeyVal::Malloc<
    key_convert_vtunnel_if>();
  val_convert_vtunnel_if *val_vtun_if = ConfigKeyVal::Malloc<
    val_convert_vtunnel_if>();

  controller_domain ctrlr_dom = {NULL, NULL};
  GET_USER_DATA_CTRLR_DOMAIN(vbr_ckv, ctrlr_dom);

  // fill the vtunnel_if key and val from vtunnel key and val
  // fill the key structure
  uuu::upll_strncpy(key_vtun_if->convert_vtunnel_key.vtn_key.vtn_name,
      vtun_key->vtn_key.vtn_name, kMaxLenVtnName+1);
  uuu::upll_strncpy(key_vtun_if->convert_vtunnel_key.convert_vtunnel_name,
      vtun_key->convert_vtunnel_name, kMaxLenConvertVnodeName+1);
  // fill val structure
  uuu::upll_strncpy(val_vtun_if->rem_ctrlr_name, ctrlr_dom.ctrlr,
                    kMaxLenCtrlrId+1);
  uuu::upll_strncpy(val_vtun_if->rem_domain_id, ctrlr_dom.domain,
                    kMaxLenDomainId+1);
  uuu::upll_strncpy(val_vtun_if->un_vbr_name, un_vbr_name, kMaxLenVnodeName+1);
  val_vtun_if->valid[UPLL_IDX_RCTRLR_NAME_CONV_VTNL_IF] = UNC_VF_VALID;
  val_vtun_if->valid[UPLL_IDX_RDOMAIN_ID_CONV_VTNL_IF] = UNC_VF_VALID;
  val_vtun_if->valid[UPLL_IDX_UN_VBR_NAME_CONV_VTNL_IF] = UNC_VF_VALID;

  // fill the val structure
  vtun_if_ckv = new ConfigKeyVal(
      UNC_KT_VTUNNEL_IF, IpctSt::kIpcStKeyConvertVtunnelIf, key_vtun_if,
      new ConfigVal(IpctSt::kIpcStValConvertVtunnelIf, val_vtun_if));

  // invoke VtunnelIfMoMgr convert function
  MoMgrImpl *vtun_if_momgr = reinterpret_cast<MoMgrImpl *>(
      const_cast<MoManager *>(GetMoManager(UNC_KT_VTUNNEL_IF)));

  result_code = vtun_if_momgr->ConvertVtunnelIf(vtun_if_ckv,
      UNC_OP_CREATE, config_mode, vtn_name, dmi);

  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("convert vtunnel if creation failed:%d", result_code);
    DELETE_IF_NOT_NULL(vtun_if_ckv);
    return result_code;
  }
  // vtunnel_if ctrlr, domain refered in CreateVlink to get convert_vtunnel
  // ctrlr, domain
  GET_USER_DATA_CTRLR_DOMAIN(vtun_ckv, ctrlr_dom);
  SET_USER_DATA_CTRLR_DOMAIN(vtun_if_ckv, ctrlr_dom);
  return result_code;
}

/** This function constructs the convert vbr if table ckv and invokes
 * VbrIfMoMgr convert function to update the entry in converted vbr if table.
 * vtunnel if name is also used for vbr if name.
 */
upll_rc_t VbrPortMapMoMgr::CreateVbrIf(
    ConfigKeyVal *convert_vbr_ckv, ConfigKeyVal *vtun_if_ckv, DalDmlIntf *dmi,
    TcConfigMode config_mode, string vtn_name, ConfigKeyVal *&vbr_if_ckv) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  // frame the convert vbr if ConfigKeyVal
  key_convert_vtunnel_if *vtun_if_key = static_cast<key_convert_vtunnel_if *>(
      vtun_if_ckv->get_key());

  key_convert_vbr_if *vbr_if_key = NULL;
  vbr_if_key = static_cast<key_convert_vbr_if*>(
      ConfigKeyVal::Malloc(sizeof(key_convert_vbr_if)));

  // fill the vtunnel_if key and val from vbr and vtunnel_if ckv
  // fill the key structure
  uuu::upll_strncpy(vbr_if_key->convert_vbr_key.vbr_key.vtn_key.vtn_name,
      vtun_if_key->convert_vtunnel_key.vtn_key.vtn_name,
      kMaxLenVtnName+1);
  uuu::upll_strncpy(vbr_if_key->convert_vbr_key.vbr_key.vbridge_name,
      static_cast<key_convert_vbr*>(
        convert_vbr_ckv->get_key())->vbr_key.vbridge_name,
      kMaxLenVnodeName+1);
  uuu::upll_strncpy(vbr_if_key->convert_vbr_key.conv_vbr_name,
      static_cast<key_convert_vbr*>(
        convert_vbr_ckv->get_key())->conv_vbr_name,
      kMaxLenConvertVnodeName+1);
  uuu::upll_strncpy(vbr_if_key->convert_if_name, vtun_if_key->convert_if_name,
      kMaxLenInterfaceName+1);

  // fill the val structure
  vbr_if_ckv = new ConfigKeyVal(
      UNC_KT_VBR_IF, IpctSt::kIpcStKeyConvertVbrIf, vbr_if_key, NULL);

  controller_domain ctrlr_dom = {NULL, NULL};
  /** CreateVbr sets the convert_vbr_ckv key_user_data from ikey*/
  GET_USER_DATA_CTRLR_DOMAIN(convert_vbr_ckv, ctrlr_dom);
  SET_USER_DATA_CTRLR_DOMAIN(vbr_if_ckv, ctrlr_dom);

  // invoke VibrIfMoMgr convert function

  MoMgrImpl *vbr_if_momgr = reinterpret_cast<MoMgrImpl *>(
      const_cast<MoManager *>(GetMoManager(UNC_KT_VBR_IF)));

  // pass true always for CREATE
  result_code = vbr_if_momgr->ConvertVbrIf(dmi, true, vbr_if_ckv, config_mode,
      vtn_name, UNC_OP_CREATE);

  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("convert vbridge if creation failed:%d", result_code);
    DELETE_IF_NOT_NULL(vbr_if_ckv);
    return result_code;
  }
  return result_code;
}

/** This function constructs convert vlink ckv and invokes VlinkMoMgr convert
 * function to auto generated vlink name and to find the boundary information
 * and to create the entry vlink and boundary information in relevan converted
 * table.
 * Note: pass convert vtunnel ctrlr, domain in vtun_if_ckv
 */
upll_rc_t VbrPortMapMoMgr::CreateVlink(
    ConfigKeyVal *vbr_if_ckv, ConfigKeyVal *vtun_if_ckv, DalDmlIntf *dmi,
    TcConfigMode config_mode, string vtn_name, uint32_t gvtnid) {
  UPLL_FUNC_TRACE;

  upll_rc_t result_code = UPLL_RC_SUCCESS;

  // construct convert vlink ckv
  key_convert_vlink_t  *vlink_conv_key = static_cast<key_convert_vlink_t*>(
      ConfigKeyVal::Malloc(sizeof(key_convert_vlink_t)));
  val_convert_vlink_t  *vlink_conv_val = static_cast<val_convert_vlink_t*>(
      ConfigKeyVal::Malloc(sizeof(val_convert_vlink_t)));

  key_convert_vbr_if_t *vbr_if_key = static_cast<key_convert_vbr_if_t*>(
      vbr_if_ckv->get_key());
  key_convert_vtunnel_if_t *key_vtun_if = static_cast<key_convert_vtunnel_if*>(
      vtun_if_ckv->get_key());

  // fill key structure
  uuu::upll_strncpy(vlink_conv_key->vbr_key.vtn_key.vtn_name,
      vbr_if_key->convert_vbr_key.vbr_key.vtn_key.vtn_name,
      kMaxLenVtnName+1);
  uuu::upll_strncpy(vlink_conv_key->vbr_key.vbridge_name,
      vbr_if_key->convert_vbr_key.vbr_key.vbridge_name,
      kMaxLenVnodeName+1);

  // fill val structure
  uuu::upll_strncpy(vlink_conv_val->vnode1_name,
      vbr_if_key->convert_vbr_key.conv_vbr_name,
      kMaxLenConvertVnodeName+1);
  uuu::upll_strncpy(vlink_conv_val->vnode1_ifname,
      vbr_if_key->convert_if_name, kMaxLenInterfaceName+1);
  uuu::upll_strncpy(vlink_conv_val->vnode2_name,
      key_vtun_if->convert_vtunnel_key.convert_vtunnel_name,
      kMaxLenConvertVnodeName+1);
  uuu::upll_strncpy(vlink_conv_val->vnode2_ifname,
      key_vtun_if->convert_if_name, kMaxLenInterfaceName+1);
  vlink_conv_val->label_type = UPLL_LABEL_TYPE_VLAN;
  vlink_conv_val->label = gvtnid;

  vlink_conv_val->valid[UPLL_IDX_VNODE1_NAME_CVLINK] = UNC_VF_VALID;
  vlink_conv_val->valid[UPLL_IDX_VNODE1_IF_NAME_CVLINK] = UNC_VF_VALID;
  vlink_conv_val->valid[UPLL_IDX_VNODE2_NAME_CVLINK] = UNC_VF_VALID;
  vlink_conv_val->valid[UPLL_IDX_VNODE2_IF_NAME_CVLINK] = UNC_VF_VALID;
  vlink_conv_val->valid[UPLL_IDX_LABEL_TYPE_CVLINK] = UNC_VF_VALID;
  vlink_conv_val->valid[UPLL_IDX_LABEL_CVLINK] = UNC_VF_VALID;

  ConfigKeyVal *vlink_ckv = new ConfigKeyVal(
      UNC_KT_VLINK, IpctSt::kIpcStKeyConvertVlink, vlink_conv_key,
      new ConfigVal(IpctSt::kIpcStValConvertVlink, vlink_conv_val));

  controller_domain ctrlr_dom = {NULL, NULL};
  controller_domain ctrlr_dom1 = {NULL, NULL};
  GET_USER_DATA_CTRLR_DOMAIN(vbr_if_ckv, ctrlr_dom);
  SET_USER_DATA_CTRLR_DOMAIN(vlink_ckv, ctrlr_dom);
  GET_USER_DATA_CTRLR_DOMAIN(vtun_if_ckv, ctrlr_dom1);
  SET_USER_DATA_CTRLR_DOMAIN(vlink_ckv->get_cfg_val(), ctrlr_dom1);
  // invoke VlinkMoMgr convert function

  MoMgrImpl *vlink_momgr = reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>
      (GetMoManager(UNC_KT_VLINK)));
  uint8_t rename_flag = 0;
  GET_USER_DATA_FLAGS(vbr_if_ckv, rename_flag);
  if (rename_flag & VTN_RENAME) {
    SET_USER_DATA_FLAGS(vlink_ckv, VTN_RENAME);
  }
  result_code = vlink_momgr->ConvertVlink(vlink_ckv, UNC_OP_CREATE, dmi,
      config_mode, vtn_name);

  DELETE_IF_NOT_NULL(vlink_ckv);

  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("convert vbridge if creation failed:%d", result_code);
    return result_code;
  }
  return result_code;
}


/** This function constructs convert vbr ckv and invokes VbrMoMgr convert
 * function to auto generate vbr name and create the entry in vbr convert
 * table.
 */
upll_rc_t VbrPortMapMoMgr::CreateVbr(
    ConfigKeyVal *ikey, IpcReqRespHeader *req, DalDmlIntf *dmi,
    TcConfigMode config_mode,
    string vtn_name, uint32_t vbid,
    ConfigKeyVal *&convert_vbr_ckv) {
  UPLL_FUNC_TRACE;

  upll_rc_t result_code = UPLL_RC_SUCCESS;

  ConfigKeyVal *tmp_convert_vbr_ckv  = NULL;
  // Construct convert vbr ckv
  MoMgrImpl *vbr_momgr = reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>(
        GetMoManager(UNC_KT_VBRIDGE)));
  result_code = vbr_momgr->GetChildConvertConfigKey(tmp_convert_vbr_ckv, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    return result_code;
  }

  val_convert_vbr *val_vbr = NULL;
  controller_domain ctrlr_dom = {NULL, NULL};
  GET_USER_DATA_CTRLR_DOMAIN(ikey, ctrlr_dom);
  SET_USER_DATA_CTRLR_DOMAIN(tmp_convert_vbr_ckv, ctrlr_dom);
  val_vbr = static_cast<val_convert_vbr*>(
      ConfigKeyVal::Malloc(sizeof(val_convert_vbr)));
  val_vbr->valid[UPLL_IDX_LABEL_CONV_VBR] = UNC_VF_VALID;
  val_vbr->label = vbid;
  tmp_convert_vbr_ckv->AppendCfgVal(IpctSt::kIpcStValConvertVbr, val_vbr);
  uint8_t conv_vbr_flags = 0;
  SET_USER_DATA_FLAGS(tmp_convert_vbr_ckv, conv_vbr_flags);

  // Invoke VbrMoMgr convert function
  result_code = vbr_momgr->ConvertVbr(dmi, req, tmp_convert_vbr_ckv,
      config_mode, vtn_name, UNC_OP_CREATE);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("converted vbr creation failed:%d", result_code);
    DELETE_IF_NOT_NULL(tmp_convert_vbr_ckv);
    return result_code;
  }
  // get the vtn/unified vbr rename flag info from convert vbr ckv
  uint8_t vbr_pm_flags = 0;
  GET_USER_DATA_FLAGS(ikey, vbr_pm_flags);
  GET_USER_DATA_FLAGS(tmp_convert_vbr_ckv, conv_vbr_flags);
  vbr_pm_flags |= conv_vbr_flags;
  SET_USER_DATA_FLAGS(ikey,  vbr_pm_flags);

  convert_vbr_ckv = tmp_convert_vbr_ckv;
  return result_code;
}

/** This function invokes necesary function to create convert vbr, vtunnel_if,
 * vbr_if, vlink for the given vbr_portmap
 * Also does the any_vlan_id validation for the given vbr_portmap
 */
upll_rc_t VbrPortMapMoMgr::CreateConvertVnodeExceptVtunnel(
    ConfigKeyVal *ikey, IpcReqRespHeader *req,
    DalDmlIntf *dmi, TcConfigMode config_mode,
    string vtn_name,
    ConfigKeyVal *vtun_ckv, uint32_t vbid) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  // Create converted vbr entry
  ConfigKeyVal *vbr_ckv = NULL;
  result_code = CreateVbr(ikey, req, dmi, config_mode, vtn_name, vbid, vbr_ckv);
  if (result_code != UPLL_RC_SUCCESS)
    return result_code;

  // Create converted vtunnel if entry
  ConfigKeyVal *vtun_if_ckv = NULL;
  uint8_t *un_vbr_name = static_cast<key_vbr_portmap*>(
      ikey->get_key())->vbr_key.vbridge_name;
  result_code = CreateVtunnelIf(vtun_ckv, vbr_ckv, un_vbr_name, vtun_if_ckv,
      config_mode, vtn_name, dmi);

  if (result_code != UPLL_RC_SUCCESS) {
    DELETE_IF_NOT_NULL(vbr_ckv);
    return result_code;
  }

  // Create converted vbr if entry
  ConfigKeyVal *vbr_if_ckv = NULL;
  result_code = CreateVbrIf(vbr_ckv, vtun_if_ckv, dmi, config_mode, vtn_name,
      vbr_if_ckv);
  DELETE_IF_NOT_NULL(vbr_ckv);
  if (result_code != UPLL_RC_SUCCESS) {
    DELETE_IF_NOT_NULL(vtun_if_ckv);
    DELETE_IF_NOT_NULL(vbr_if_ckv);
    return result_code;
  }

  // get gvtnid
  uint32_t gvtnid = static_cast<val_convert_vtunnel*>(GetVal(vtun_ckv))->label;

  uint8_t rename_flag = 0;
  GET_USER_DATA_FLAGS(ikey, rename_flag);
  if (rename_flag & VTN_RENAME) {
    SET_USER_DATA_FLAGS(vbr_if_ckv, VTN_RENAME);
  }
  // Create converted vlink entry
  result_code = CreateVlink(vbr_if_ckv, vtun_if_ckv, dmi, config_mode, vtn_name,
      gvtnid);
  DELETE_IF_NOT_NULL(vtun_if_ckv);
  DELETE_IF_NOT_NULL(vbr_if_ckv);
  return result_code;
}
/** This function
 * 1. checks whether ca_vbr_portmap table has any entry with  valid
 * logical_port_id
 * 2. If exists, checks whether any of the such vbr_portmap matches with the
 * controller/domain in request
 * 3. counts vbr_portmap with logical_port_id with the distinct
 * controller/domain is minimum of two
 */
upll_rc_t VbrPortMapMoMgr::CompareCtrlrDomainInReqWithDB(
    ConfigKeyVal *ikey, DalDmlIntf *dmi, TcConfigMode config_mode,
    string vtn_name, bool *same_ctrlr_domain_exists, uint32_t *counter) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  *counter = 0;

  DalBindInfo *bind_info = new DalBindInfo(uudst::kDbiVbrPortMapTbl);
  bind_info->BindMatch(
      uudst::vbridge_portmap::kDbiVtnName, uud::kDalChar, (kMaxLenVtnName+1),
      static_cast<key_vbr_portmap_t*>(
          ikey->get_key())->vbr_key.vtn_key.vtn_name);
  bind_info->BindMatch(
      uudst::vbridge_portmap::kDbiVbrName, uud::kDalChar, (kMaxLenVnodeName+1),
      static_cast<key_vbr_portmap_t*>(ikey->get_key())->vbr_key.vbridge_name);

  const uint8_t logical_port_valid = UNC_VF_VALID;

  bind_info->BindMatch(
      uudst::vbridge_portmap::kDbiValidLogicalPortId, uud::kDalUint8, 1,
      &logical_port_valid);
  // check whether vbr_portmap exists with same controller/domain with
  // logical_port_id valid flag UNC_VF_VALID for given vtn and vbridge
  key_user_data *tuser_data  = reinterpret_cast<key_user_data_t *>
    (ikey->get_user_data());

  bind_info->BindMatch(uudst::vbridge_portmap::kDbiCtrlrName,
                      uud::kDalChar, (kMaxLenCtrlrId + 1),
                      &(tuser_data->ctrlr_id));
  bind_info->BindMatch(uudst::vbridge_portmap::kDbiDomainId,
                      uud::kDalChar, (kMaxLenDomainId+1),
                      &(tuser_data->domain_id));

  // check the record existance in DB
  bool existence;
  result_code = DalToUpllResCode(dmi->RecordExists(UPLL_DT_CANDIDATE,
                                                   uudst::kDbiVbrPortMapTbl,
                                                   bind_info, &existence));
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("ca_vbr_portmap_tbl RecordExists failed: %d", result_code);
    delete bind_info;
    return result_code;
  }
  if (existence == true) {
    *same_ctrlr_domain_exists = true;
    delete bind_info;
    return UPLL_RC_SUCCESS;
  } else {
    *same_ctrlr_domain_exists = false;
  }
  delete bind_info;
  // if same controller/domain doesn't exist, check how many distinct controller
  // domain exists in vbr_portmap table
  // get the controller and domain in output
  uint8_t db_ctrlr[kMaxLenCtrlrId + 1];
  uint8_t db_domain[kMaxLenDomainId + 1];
  bind_info = new DalBindInfo(uudst::kDbiVbrPortMapTbl);
  bind_info->BindMatch(
      uudst::vbridge_portmap::kDbiVtnName, uud::kDalChar, (kMaxLenVtnName+1),
      static_cast<key_vbr_portmap_t*>(
          ikey->get_key())->vbr_key.vtn_key.vtn_name);
  bind_info->BindMatch(
      uudst::vbridge_portmap::kDbiVbrName, uud::kDalChar, (kMaxLenVnodeName+1),
      static_cast<key_vbr_portmap_t*>(ikey->get_key())->vbr_key.vbridge_name);

  bind_info->BindMatch(
      uudst::vbridge_portmap::kDbiValidLogicalPortId, uud::kDalUint8, 1,
      &logical_port_valid);
  bind_info->BindOutput(uudst::vbridge_portmap::kDbiCtrlrName, uud::kDalChar,
                       (kMaxLenCtrlrId + 1), db_ctrlr);
  bind_info->BindOutput(uudst::vbridge_portmap::kDbiDomainId, uud::kDalChar,
                       (kMaxLenDomainId + 1), db_domain);

  // frame the query
  std::string query_string;
  query_string = QUERY_READ_MULTIPLE_VBR_PORTMAP;
  // XXX Note: BindMatch has 5 params, but only first 3 are really actually used
  // in QUERY_READ_MULTIPLE_VBR_PORTMAP

  // execute query
  DalCursor *dal_cursor_handle = NULL;
  result_code = DalToUpllResCode(dmi->ExecuteAppQueryMultipleRecords(
          query_string, 0, bind_info, &dal_cursor_handle));

  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_TRACE("vbr portmap multiple records read failed %d", result_code);
    delete bind_info;
    bind_info = NULL;
    if (dal_cursor_handle) {
      dmi->CloseCursor(dal_cursor_handle, false);
    }
    return result_code;
  }

  // get the controller, domain in request
  uint8_t *ikey_ctrlr =
      static_cast<val_vbr_portmap*>(GetVal(ikey))->controller_id;
  uint8_t *ikey_domain =
      static_cast<val_vbr_portmap*>(GetVal(ikey))->domain_id;

  do {
    result_code = DalToUpllResCode(dmi->GetNextRecord(dal_cursor_handle));

    if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
      // UPLL_LOG_TRACE("no vbr portmap exists with logical_port_id for given"
        //             " vtn and unified vbr");
      DELETE_IF_NOT_NULL(bind_info);
      dmi->CloseCursor(dal_cursor_handle, false);
      dal_cursor_handle = NULL;
      result_code = UPLL_RC_SUCCESS;
      return result_code;
    }

    // count the number of distinct controller, domain exists in DB
    // compare controller and domain name in request with the one in DB
    if ((uuu::upll_strncmp(ikey_ctrlr, db_ctrlr, kMaxLenCtrlrId+1)) ||
        (uuu::upll_strncmp(ikey_domain, db_domain, kMaxLenDomainId+1))) {
      (*counter)++;
    }
    if (*counter == 2) {
      break;
    }
  } while (UPLL_RC_SUCCESS == result_code);
  DELETE_IF_NOT_NULL(bind_info);
  dmi->CloseCursor(dal_cursor_handle, false);
  dal_cursor_handle = NULL;
  return result_code;
}

upll_rc_t VbrPortMapMoMgr::ValidateAttribute(ConfigKeyVal *ikey,
                                             DalDmlIntf *dmi,
                                             IpcReqRespHeader *req) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  val_vbr_portmap *ikey_val =  NULL;
  result_code = GetValVbrPortMap(ikey, &ikey_val);
  if (result_code != UPLL_RC_SUCCESS) {
    return result_code;
  }

  // get the configmode information
  TcConfigMode config_mode = TC_CONFIG_INVALID;
  std::string vtn_name;
  result_code = GetConfigModeInfo(req, config_mode, vtn_name);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_TRACE("GetConfigModeInfo failed");
    return result_code;
  }

  if (req->operation == UNC_OP_UPDATE) {
    // If controllerid, domain_id are passed for update, then its value should
    // be same as the one exists in DB
    // Get the DB ctrlr and domain from ikey key_user_data
    uint8_t *db_ctrlr = static_cast<key_user_data_t*>(
        ikey->get_user_data())->ctrlr_id;
    uint8_t *db_domain = static_cast<key_user_data_t*>(
        ikey->get_user_data())->domain_id;
    if (ikey_val->valid[UPLL_IDX_CONTROLLER_ID_VBRPM] == UNC_VF_VALID) {
      if ((uuu::upll_strncmp(ikey_val->controller_id, db_ctrlr,
              kMaxLenCtrlrId+1))) {
        UPLL_LOG_DEBUG("ctrlr id in DB %s is not matching with request",
            db_ctrlr);
        return UPLL_RC_ERR_CFG_SEMANTIC;
      }
    }
    if (ikey_val->valid[UPLL_IDX_DOMAIN_ID_VBRPM] == UNC_VF_VALID) {
      if ((uuu::upll_strncmp(ikey_val->domain_id, db_domain,
              kMaxLenDomainId+1))) {
        UPLL_LOG_DEBUG("domain id in DB %s is not matching with request",
            db_domain);
        return UPLL_RC_ERR_CFG_SEMANTIC;
      }
    }
  } else {
    // for CREATE, do the ctrlr existance check using VnodeMoMgr API
    MoMgrImpl *vnode_momgr = reinterpret_cast<MoMgrImpl *>(
        const_cast<MoManager *>(GetMoManager(UNC_KT_VBRIDGE)));
    result_code = vnode_momgr->CtrlrTypeAndDomainCheck(ikey, req);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("CtrlrTypeAndDomainCheck failed::%d", result_code);
      return result_code;
    }
    // do the domain type validation
    // 1. candidate arg && global mode => check in candidate
    // 2. running arg => check in running
    if (((req->datatype == UPLL_DT_CANDIDATE) && (config_mode ==
            TC_CONFIG_GLOBAL)) ||
        (req->datatype == UPLL_DT_RUNNING)) {
      result_code = domain_util::DomainUtil::IsDomainLeaf(
          reinterpret_cast<const char*>(ikey_val->controller_id),
          reinterpret_cast<const char*>(ikey_val->domain_id), dmi,
          req->datatype);
      if (result_code == UPLL_RC_ERR_CFG_SEMANTIC) {
        result_code = UPLL_RC_ERR_INVALID_DOMAIN;
      }
      // IsDomainLeaf returns semantic error if the domain type is not PF_LEAF
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("domain type validation failed:%d", result_code);
        return result_code;
      }
    }
  }

  if (ikey_val->valid[UPLL_IDX_LABEL_VBRPM] == UNC_VF_VALID) {
    // do the transparent vlan-id check
    result_code = IsVbrPmVlanIdTransparent(ikey, dmi, req);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_INFO("Transparent vlanid check failed");
      return result_code;
    }

    result_code = IsLogicalPortAndVlanIdInUse(ikey, dmi, req);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("IsLogicalPortAndVlanIdInUse failed");
      return result_code;
    }
  }
  if (req->datatype == UPLL_DT_CANDIDATE) {
    if (config_mode == TC_CONFIG_VTN) {
      // do the check on Running configuration
      req->datatype = UPLL_DT_RUNNING;
      result_code = ValidateAttribute(ikey, dmi, req);
      req->datatype = UPLL_DT_CANDIDATE;
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("ValidateAttribute for Running DB failed: %d",
            result_code);
        return result_code;
      }
    }
  }
  return result_code;
}

upll_rc_t VbrPortMapMoMgr::IsVbrPmVlanIdTransparent(ConfigKeyVal *ikey,
                                                    DalDmlIntf *dmi,
                                                    IpcReqRespHeader *req) {
  UPLL_FUNC_TRACE;

  upll_rc_t result_code = UPLL_RC_SUCCESS;

  uint32_t db_val_vbr_pm_label = 0;
  result_code = GetVbrPortMap(ikey, req, &db_val_vbr_pm_label, dmi);

  if ((result_code != UPLL_RC_SUCCESS) &&
      (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE)) {
    UPLL_LOG_ERROR("VbrPortMap table read failed:%d", result_code);
    return result_code;
  }

  val_vbr_portmap *ikey_vbrpm =  NULL;
  upll_rc_t t_rc = GetValVbrPortMap(ikey, &ikey_vbrpm);
  if (t_rc != UPLL_RC_SUCCESS) {
    return t_rc;
  }

  if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    // This is the first vbr-portmap created with vlanid,
    // check whether more than one unified vbridge exists
    if (ikey_vbrpm->label == ANY_VLAN_ID) {
      ConfigKeyVal *temp_ckv = NULL;
      result_code = GetParentConfigKey(temp_ckv, ikey);
      if (result_code != UPLL_RC_SUCCESS) {
        return result_code;
      }

      StringReset(static_cast<key_vbr*>(temp_ckv->get_key())->vbridge_name);

      uint32_t un_vbr_instance_count = 0;
      MoMgrImpl *vbr_momgr = reinterpret_cast<MoMgrImpl *>(
          const_cast<MoManager *>(GetMoManager(UNC_KT_VBRIDGE)));
      char un_vbr_ctrlr[2] = {'#'};
      result_code = vbr_momgr->GetInstanceCount(
          temp_ckv, un_vbr_ctrlr, req->datatype, &un_vbr_instance_count, dmi,
          MAINTBL);

      DELETE_IF_NOT_NULL(temp_ckv);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_ERROR("Unified vbr instance count check failed %d",
                       result_code);
        return result_code;
      }
      if (un_vbr_instance_count > 1) {
        UPLL_LOG_DEBUG("%d unified vbridge exists, when vlanid is transparent",
                       un_vbr_instance_count);
        return UPLL_RC_ERR_VLAN_TYPE_MISMATCH;
      }
    } else {
      result_code = UPLL_RC_SUCCESS;
    }
  } else if (result_code == UPLL_RC_SUCCESS) {
    // if existing vbr_portmap configuration vlan-id is 0xFFFE and the current
    // vbr_portmap vlan-id is not 0xFFFE or vice versa then throw error
    if (db_val_vbr_pm_label == ANY_VLAN_ID) {
      if (ikey_vbrpm->label != ANY_VLAN_ID) {
        UPLL_LOG_DEBUG("Transparent portmap exists,"
                       " when vlanid is not transparent");
        return UPLL_RC_ERR_VLAN_TYPE_MISMATCH;
      }
    } else {
      if (ikey_vbrpm->label == ANY_VLAN_ID) {
        UPLL_LOG_DEBUG("Not transparent portmap exists,"
                       " when vlanid is transparent");
        return UPLL_RC_ERR_VLAN_TYPE_MISMATCH;
      }
    }
  }
  return result_code;
}

// This function checks given logical_port_id and vlan_id is not exists
// in other vbridge portmap of same vtn/same controller/same domain
upll_rc_t VbrPortMapMoMgr::IsLogicalPortAndVlanIdInUse(ConfigKeyVal *ckv,
    DalDmlIntf *dmi,
    IpcReqRespHeader *req) {
  UPLL_FUNC_TRACE;

  upll_rc_t result_code = UPLL_RC_SUCCESS;
  val_vbr_portmap_t *ckv_vbrpm_val = NULL;

  result_code = GetValVbrPortMap(ckv, &ckv_vbrpm_val);
  if (result_code != UPLL_RC_SUCCESS) {
    return result_code;
  }

  if (!ckv_vbrpm_val) {
    UPLL_LOG_TRACE("Returning error\n");
    return UPLL_RC_ERR_GENERIC;
  }

  key_vbr_portmap_t* ckv_vbrpm_key =
    reinterpret_cast<key_vbr_portmap_t *>(ckv->get_key());

  key_vbr_portmap_t* key_vbr_pm = static_cast<key_vbr_portmap_t *>
    (ConfigKeyVal::Malloc(sizeof(key_vbr_portmap_t)));

  pfcdrv_val_vbr_portmap_t * pfcdrv_val_vbrpm = static_cast<
    pfcdrv_val_vbr_portmap_t*>(ConfigKeyVal::Malloc(sizeof(
            pfcdrv_val_vbr_portmap_t)));

  val_vbr_portmap_t* val_vbr_pm = &(pfcdrv_val_vbrpm->vbrpm);

  // Populate logical-port-id and label from ckv to temporary structure
  uuu::upll_strncpy(val_vbr_pm->logical_port_id,
      ckv_vbrpm_val->logical_port_id, kMaxLenLogicalPortId+1);

  val_vbr_pm->valid[UPLL_IDX_LOGICAL_PORT_ID_VBRPM] = UNC_VF_VALID;

  ConfigKeyVal *db_ckv = new ConfigKeyVal(UNC_KT_VBR_PORTMAP,
      IpctSt::kIpcStKeyVbrPortMap, key_vbr_pm, NULL);
  db_ckv->AppendCfgVal(IpctSt::kIpcStPfcdrvValVbrPortMap, pfcdrv_val_vbrpm);

  //  Setting Ctrlr/Domain Id from ckv to db_ckv
  SET_USER_DATA(db_ckv, ckv);

  DbSubOp dbop = { kOpReadMultiple, kOpMatchCtrlr| kOpMatchDomain,
    kOpInOutFlag};
  //  Read the Configuration from the MainTable
  result_code = ReadConfigDB(db_ckv, req->datatype, UNC_OP_READ,
      dbop, dmi, MAINTBL);

  //  Check LogicalPortId and VlanId in use
  if (result_code == UPLL_RC_SUCCESS) {
    ConfigKeyVal *tmp_ckv = db_ckv;
    while (tmp_ckv) {
      key_vbr_portmap_t* tmp_vbrpm_key =
          reinterpret_cast<key_vbr_portmap_t *>(tmp_ckv->get_key());
      val_vbr_portmap *db_val_vbrpm = &(static_cast<pfcdrv_val_vbr_portmap*>(
              GetVal(tmp_ckv)))->vbrpm;
      if ((!uuu::upll_strncmp(ckv_vbrpm_key->vbr_key.vtn_key.vtn_name,
                              tmp_vbrpm_key->vbr_key.vtn_key.vtn_name,
                              kMaxLenVtnName+1)) &&
          (!uuu::upll_strncmp(ckv_vbrpm_key->vbr_key.vbridge_name,
                              tmp_vbrpm_key->vbr_key.vbridge_name,
                              kMaxLenVnodeName+1)) &&
          (!uuu::upll_strncmp(ckv_vbrpm_key->portmap_id,
                              tmp_vbrpm_key->portmap_id,
                              kMaxLenPortMapName+1))) {
        UPLL_LOG_TRACE("Looking on the Same key");
      } else if (ckv_vbrpm_val->label == db_val_vbrpm->label) {
        UPLL_LOG_TRACE("More than one vbrPortMap is configured with the"
            " same logical port id and vlanid!");
        DELETE_IF_NOT_NULL(db_ckv);
        tmp_ckv = NULL;
        return UPLL_RC_ERR_VLAN_IN_USE;
      } else if (((ckv_vbrpm_val->label == ANY_VLAN_ID) &&
               (db_val_vbrpm->label != ANY_VLAN_ID)) ||
              ((ckv_vbrpm_val->label != ANY_VLAN_ID) &&
                 (db_val_vbrpm->label == ANY_VLAN_ID))) {
        UPLL_LOG_DEBUG("transparent vlan-id is not allowed, when "
                        "non-transparent vlan-id with same logical port id"
                        " exists or vice versa.(%d)", result_code);
        DELETE_IF_NOT_NULL(db_ckv);
        tmp_ckv = NULL;
        return UPLL_RC_ERR_VLAN_TYPE_MISMATCH;
      }
      tmp_ckv = tmp_ckv->get_next_cfg_key_val();
    }
  } else if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    result_code = UPLL_RC_SUCCESS;
  }
  DELETE_IF_NOT_NULL(db_ckv);
  return result_code;
}

/** This function deletes the converted vnode and vbr_portmap
 *  wrt vtn and unified vbridge delete.
 */
upll_rc_t VbrPortMapMoMgr::DeleteChildren(ConfigKeyVal *ikey,
    ConfigKeyVal *pkey,
    upll_keytype_datatype_t dt_type,
    DalDmlIntf *dmi,
    TcConfigMode config_mode,
    string vtn_name,
    MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  // delete the converted vnode
  result_code = DeleteConvertVNodeChildren(ikey, dt_type, dmi,
            config_mode, vtn_name, tbl);

  if ((result_code != UPLL_RC_SUCCESS) &&
       (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE)) {
    UPLL_LOG_TRACE("Convert vnode delete failed:%d", result_code);
    return result_code;
  }

  DbSubOp dbop = { kOpNotRead, kOpMatchNone, kOpInOutNone};
  // Delete the vbr_portmap entry
  result_code = UpdateConfigDB(ikey, UPLL_DT_CANDIDATE, UNC_OP_DELETE,
      dmi, &dbop, config_mode, vtn_name, MAINTBL);

  if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE)
    result_code = UPLL_RC_SUCCESS;

  return result_code;
}


/**This function deletes the converted vnode wrt vtn and unified vbridge
 * delete.
 */
upll_rc_t VbrPortMapMoMgr::DeleteConvertVNodeChildren(ConfigKeyVal *ikey,
    upll_keytype_datatype_t dt_type, DalDmlIntf *dmi, TcConfigMode config_mode,
    string vtn_name, MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  key_vbr_portmap *ikey_vbrpm = static_cast<key_vbr_portmap*>(ikey->get_key());
  bool un_vbr_delete = false;
  uint32_t vtun_ref_count = 0;
  uint32_t vbid = 0;
  uint32_t gvtnid = 0;

  // check it is vtn-delete or unified vbridge delete
  if (strlen(reinterpret_cast<const char*>(
          ikey_vbrpm->vbr_key.vbridge_name)) != 0) {
    un_vbr_delete = true;
  }
  // convert table keytype list
  unc_key_type_t convert_ktype[] = {
    UNC_KT_VLINK,    UNC_KT_VBR_IF,    UNC_KT_VTUNNEL_IF,
    UNC_KT_VTUNNEL,    UNC_KT_VBRIDGE
  };
  uint8_t nconv_tbl = sizeof(convert_ktype)/sizeof(convert_ktype[0]);

  ConfigKeyVal *vtun_ckv = NULL;
  ConfigKeyVal *vbr_ckv = NULL;

  // get the vbid for dealloction
  result_code = GetVbid(ikey, dmi, &vbid);
  if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    // check whether vtn delete is performed
    if (!un_vbr_delete) {
      // dealloc vbid which exists for given vtn if any
      result_code = DeAllocVbid(ikey, dmi, config_mode, vtn_name);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("DeAllocVbid for vtn delete failed:%d", result_code);
        return result_code;
      }
    }
    UPLL_LOG_TRACE("No convert table entry exists");
    return UPLL_RC_SUCCESS;
  }
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetVbid failed:%d", result_code);
    return result_code;
  }
  if (un_vbr_delete) {
    // delete the vtn controller data related to given unified vbridge
    // in case of vtn delete. vtn_ctrlr data delete happens via
    //  momgr_util.cc::DeleteChildren
    result_code = DeleteVtnRenameAndCtrlrTblData(ikey, dmi, vbr_ckv,
                                                 config_mode, vtn_name);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Error in deleting vtn ctrlr data:%d", result_code);
      return result_code;
    }
  }
  DbSubOp dbop = { kOpNotRead, kOpMatchNone, kOpInOutNone};
  for (uint8_t kt_idx = 0; kt_idx < nconv_tbl; ++kt_idx) {
    // Delete the convert table entries for given unified vbridge/vtn
    MoMgrImpl *kt_momgr = reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>(
          GetMoManager(convert_ktype[kt_idx])));
    ConfigKeyVal *tmp_ckv = NULL;
    result_code = kt_momgr->GetChildConvertConfigKey(tmp_ckv , ikey);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("GetChildConvertConfigKey failed:%d", result_code);
      DELETE_IF_NOT_NULL(vbr_ckv);
      return result_code;
    }
    if ((convert_ktype[kt_idx] == UNC_KT_VTUNNEL_IF) && un_vbr_delete) {
      // fill the un_vbr_name in val strucure for deletion
      val_convert_vtunnel_if *val_vtun_if =
        ConfigKeyVal::Malloc<val_convert_vtunnel_if>();
      uuu::upll_strncpy(val_vtun_if->un_vbr_name, static_cast<key_vbr_portmap*>(
            ikey->get_key())->vbr_key.vbridge_name, kMaxLenVnodeName+1);
      val_vtun_if->valid[UPLL_IDX_UN_VBR_NAME_CONV_VTNL_IF] = UNC_VF_VALID;
      tmp_ckv->AppendCfgVal(IpctSt::kIpcStValConvertVtunnelIf, val_vtun_if);
      // Invoke DeleteVtunnelIf to delete based on key and val structure
      result_code = kt_momgr->DeleteVtunnelIf(tmp_ckv, config_mode, vtn_name,
                    dmi);
      if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
        result_code = UPLL_RC_SUCCESS;
      }
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("convert_vtunnel_if table delete failed");
        DELETE_IF_NOT_NULL(vbr_ckv);
        DELETE_IF_NOT_NULL(tmp_ckv);
        return result_code;
      }
      DELETE_IF_NOT_NULL(tmp_ckv);
      continue;
    }
    if ((convert_ktype[kt_idx] == UNC_KT_VTUNNEL) && un_vbr_delete &&
        (vtun_ref_count > 1)) {
      // need to decrement the vtunnel ref_count as other unified
      // vbridges on the same vtn refers this vtunnel
      static_cast<val_convert_vtunnel*>(GetVal(vtun_ckv))->ref_count--;
      result_code = kt_momgr->UpdateConfigDB(vtun_ckv, UPLL_DT_CANDIDATE,
                                             UNC_OP_UPDATE, dmi, &dbop,
                                             config_mode, vtn_name, CONVERTTBL);
    } else {
      // Delete convert table entry
      result_code = kt_momgr->UpdateConfigDB(tmp_ckv, UPLL_DT_CANDIDATE,
                                             UNC_OP_DELETE, dmi, &dbop,
                                             config_mode, vtn_name, CONVERTTBL);
    }
    DELETE_IF_NOT_NULL(tmp_ckv);

    if ((result_code != UPLL_RC_SUCCESS) && (result_code !=
          UPLL_RC_ERR_NO_SUCH_INSTANCE)) {
      UPLL_LOG_ERROR("Convert tbl delete failed:%d", result_code);
      DELETE_IF_NOT_NULL(vbr_ckv);
      return result_code;
    }
    if (convert_ktype[kt_idx] == UNC_KT_VLINK) {
      if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
        result_code = UPLL_RC_SUCCESS;
        // increase the index to look for the entry in
        // convert vbr table and other convert table has no entry for given
        // vtn or vbridge
        if (un_vbr_delete) {
          kt_idx = kt_idx + 3;
        }
        // should not remove the entry in vtunnel table and other tables
        // as it may exists due to the vbr-portmap in other unified vbridges
        continue;  // this will do one more index increment
      }
      // get the vtunnel ref_count to decide on vtn_gateway_port_table delete,
      // gvtnid deallocation
      result_code = GetVtunnelData(ikey, dmi, vtun_ckv);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("GetVtunnelData failed:%d", result_code);
        DELETE_IF_NOT_NULL(vbr_ckv);
        return result_code;
      } else {
        val_convert_vtunnel  *vtun_val = static_cast<val_convert_vtunnel*>(
            GetVal(vtun_ckv));
        vtun_ref_count = vtun_val->ref_count;
        if (!un_vbr_delete || (vtun_ref_count == 1)) {
          // need to deallocate the gvtnid
          gvtnid = vtun_val->label;
        }
      }
      if ((un_vbr_delete) && (vtun_ref_count != 1)) {
        // convert vlink exists for given unified vbridge, so entry will exists
        // in vtn_gateway_port_table as well, so delete it
        // delete the vtn_gateway_port_tbl based on the ctrlr,domain in
        //  convert_vbr_tbl for given vtn and unified vbridge
        result_code = DeleteVtnGatewayPortExistsForAUnwVbr(ikey, dmi, vbr_ckv,
                       config_mode, vtn_name);
        DELETE_IF_NOT_NULL(vbr_ckv);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("DeleteVtnGatewayPortExistsForAUnwVbr failed:%d",
                         result_code);
          DELETE_IF_NOT_NULL(vtun_ckv);
          return result_code;
        }
      } else {
        DELETE_IF_NOT_NULL(vbr_ckv);
        MoMgrImpl *vtn_mgr =
          reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>(GetMoManager(
                  UNC_KT_VTN)));
        if (!vtn_mgr) {
          UPLL_LOG_TRACE("Invalid Mgr");
          DELETE_IF_NOT_NULL(vtun_ckv);
          return UPLL_RC_ERR_GENERIC;
        }
        // delete the entry in vtn_gateway_port table
        ConfigKeyVal *gw_ckv = NULL;
        result_code = vtn_mgr->GetChildConfigKey(gw_ckv, ikey);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("GetChildConvertConfigKey failed:%d", result_code);
          DELETE_IF_NOT_NULL(vtun_ckv);
          return result_code;
        }
        result_code = vtn_mgr->UpdateConfigDB(gw_ckv, UPLL_DT_CANDIDATE,
            UNC_OP_DELETE, dmi, &dbop, config_mode, vtn_name, CONVERTTBL);
        DELETE_IF_NOT_NULL(gw_ckv);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("Vtn gateway table delete failed:%d", result_code);
          DELETE_IF_NOT_NULL(vtun_ckv);
          return result_code;
        }
      }
    }
    DELETE_IF_NOT_NULL(vbr_ckv);
    if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE)
      result_code = UPLL_RC_SUCCESS;
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("UpdateConfigDB for kt(%d)delete failed",
          convert_ktype[kt_idx]);
      DELETE_IF_NOT_NULL(vtun_ckv);
      return result_code;
    }
  }

  // dealloc vbid
  if (!un_vbr_delete) {
    // dealloc vbid which exists for given vtn
    result_code = DeAllocVbid(ikey, dmi, config_mode, vtn_name);
    if (result_code != UPLL_RC_SUCCESS) {
      DELETE_IF_NOT_NULL(vtun_ckv);
      UPLL_LOG_DEBUG("DeAllocVbid for vtn delete failed:%d", result_code);
      return result_code;
    }
  } else  if ((vbid !=0) && (vbid != ANY_VLAN_ID)) {
    result_code = DeAllocVbid(ikey, vbid, dmi, config_mode, vtn_name);
    if (result_code != UPLL_RC_SUCCESS) {
      DELETE_IF_NOT_NULL(vtun_ckv);
      UPLL_LOG_DEBUG("DeAllocVbid failed:%d", result_code);
      return result_code;
    }
  }
  // dealloc gvtnid if required
  if (gvtnid > 0) {
    result_code = DeAllocGvtnId(vtun_ckv,
                                dmi, UPLL_DT_CANDIDATE, config_mode, vtn_name);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("GvtnId deallocation failed:%d", result_code);
      DELETE_IF_NOT_NULL(vtun_ckv);
      return result_code;
    }
    // Decrement the used label count
    uint8_t *ikey_vtn = static_cast<key_vbr_portmap*>(
        ikey->get_key())->vbr_key.vtn_key.vtn_name;
    result_code = DecrementUsedLabelCount(vtun_ckv, ikey_vtn, dmi,
                                          config_mode, vtn_name);
    DELETE_IF_NOT_NULL(vtun_ckv);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("DecrementUsedLabelCount failed:%d", result_code);
      return result_code;
    }
  } else {
    DELETE_IF_NOT_NULL(vtun_ckv);
  }
  return result_code;
}

/** This function does ReadMultiple on vbr portmap table and deletes the vtn
 * ctrlr table entry*/
upll_rc_t VbrPortMapMoMgr::DeleteVtnRenameAndCtrlrTblData(ConfigKeyVal *ikey,
    DalDmlIntf *dmi, ConfigKeyVal *&vbr_ckv, TcConfigMode config_mode,
    string vtn_name) {
  UPLL_FUNC_TRACE;

  // Get the controller,domain from convert vbr table
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  MoMgrImpl *vbr_momgr = reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>(
        GetMoManager(UNC_KT_VBRIDGE)));
  result_code = vbr_momgr->GetChildConvertConfigKey(vbr_ckv, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    return result_code;
  }
  DbSubOp dbop = { kOpReadMultiple, kOpMatchNone,
    kOpInOutCtrlr|kOpInOutDomain};
  //  Read the Configuration from convert vbr table
  result_code = vbr_momgr->ReadConfigDB(vbr_ckv, UPLL_DT_CANDIDATE, UNC_OP_READ,
      dbop, dmi, CONVERTTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    DELETE_IF_NOT_NULL(vbr_ckv);
    return result_code;
  }
  //  Based on the converted vbr table ctlr, domain delete the entry from
  //  vtn ctrlr table and rename table
  if (result_code == UPLL_RC_SUCCESS) {
    // Read and delete the
    ConfigKeyVal *tmp_ckv = vbr_ckv;
    MoMgrImpl *vtn_mgr =
      reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>(GetMoManager(
              UNC_KT_VTN)));
    if (!vtn_mgr) {
      UPLL_LOG_TRACE("Invalid Mgr");
      DELETE_IF_NOT_NULL(vbr_ckv);
      return UPLL_RC_ERR_GENERIC;
    }

    ConfigKeyVal *vtn_ckv = NULL;
    result_code = vtn_mgr->GetChildConfigKey(vtn_ckv, ikey);
    if (result_code != UPLL_RC_SUCCESS) {
      DELETE_IF_NOT_NULL(vbr_ckv);
      UPLL_LOG_DEBUG("GetChildConvertConfigKey failed:%d", result_code);
      return result_code;
    }
    controller_domain ctrlr_dom = {NULL, NULL};

    DbSubOp dbop_upd = { kOpNotRead, kOpMatchCtrlr|kOpMatchDomain,
      kOpInOutNone};
    DbSubOp dbop_read = {kOpReadSingle, kOpMatchCtrlr|kOpMatchDomain,
      kOpInOutNone};
    while (tmp_ckv) {
      GET_USER_DATA_CTRLR_DOMAIN(tmp_ckv, ctrlr_dom);
      SET_USER_DATA_CTRLR_DOMAIN(vtn_ckv, ctrlr_dom);
      // read vtn_ctrlr_tbl to delete based on ref_count
      result_code = vtn_mgr->ReadConfigDB(vtn_ckv, UPLL_DT_CANDIDATE,
          UNC_OP_READ, dbop_read, dmi, CTRLRTBL);
      if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
        DELETE_IF_NOT_NULL(vbr_ckv);
        DELETE_IF_NOT_NULL(vtn_ckv);
        UPLL_LOG_DEBUG("Entry is not exists in vtn ctrlr table");
        return result_code;
      }
      if (result_code != UPLL_RC_SUCCESS) {
        DELETE_IF_NOT_NULL(vbr_ckv);
        DELETE_IF_NOT_NULL(vtn_ckv);
        UPLL_LOG_DEBUG("Vtn ctrlr table read failed:%d", result_code);
        return result_code;
      }
      val_vtn_ctrlr *ctr_val = static_cast<val_vtn_ctrlr*>(GetVal(vtn_ckv));
      if ((ctr_val->ref_cnt2 > 1) || ((ctr_val->ref_cnt2 == 1) &&
                                      (ctr_val->vnode_ref_cnt > 0))) {
        // decrement the ref_count and update it in DB
        ctr_val->ref_cnt2--;
        result_code = vtn_mgr->UpdateConfigDB(vtn_ckv, UPLL_DT_CANDIDATE,
             UNC_OP_UPDATE, dmi, &dbop_upd, config_mode, vtn_name, CTRLRTBL);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_TRACE("vtn ctrlr table ref_count update failed:%d",
                         result_code);
          DELETE_IF_NOT_NULL(vbr_ckv);
          DELETE_IF_NOT_NULL(vtn_ckv);
          return result_code;
        }
      } else {
        // delete the vtn_ctrlr data
        result_code = vtn_mgr->UpdateConfigDB(vtn_ckv, UPLL_DT_CANDIDATE,
            UNC_OP_DELETE, dmi, &dbop_upd, config_mode, vtn_name, CTRLRTBL);

        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_TRACE("vtn ctrlr table delete failed:%d", result_code);
          DELETE_IF_NOT_NULL(vbr_ckv);
          DELETE_IF_NOT_NULL(vtn_ckv);
          return result_code;
        }
        // delete the vtn_rename table data
        result_code = vtn_mgr->UpdateConfigDB(vtn_ckv, UPLL_DT_CANDIDATE,
            UNC_OP_DELETE, dmi, &dbop_upd, config_mode, vtn_name, RENAMETBL);
        if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
          result_code = UPLL_RC_SUCCESS;
        }
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_TRACE("vtn rename table delete failed:%d", result_code);
          DELETE_IF_NOT_NULL(vtn_ckv);
          DELETE_IF_NOT_NULL(vbr_ckv);
          return result_code;
        }
      }
      tmp_ckv = tmp_ckv->get_next_cfg_key_val();
    }
    DELETE_IF_NOT_NULL(vtn_ckv);
  }
  return result_code;
}

upll_rc_t VbrPortMapMoMgr::DeleteVtnGatewayPortExistsForAUnwVbr(
    ConfigKeyVal *ikey, DalDmlIntf *dmi, ConfigKeyVal *vbr_ckv,
    TcConfigMode config_mode, string vtn_name) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  ConfigKeyVal *tmp_ckv = vbr_ckv;
  MoMgrImpl *vtn_mgr =
    reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>(GetMoManager(
            UNC_KT_VTN)));
  if (!vtn_mgr) {
    UPLL_LOG_TRACE("Invalid Mgr");
    DELETE_IF_NOT_NULL(vbr_ckv);
    return UPLL_RC_ERR_GENERIC;
  }

  ConfigKeyVal *vtn_ckv = NULL;
  result_code = vtn_mgr->GetChildConfigKey(vtn_ckv, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetChildConvertConfigKey failed:%d", result_code);
    return result_code;
  }
  controller_domain ctrlr_dom = {NULL, NULL};

  DbSubOp dbop_up = { kOpNotRead, kOpMatchCtrlr|kOpMatchDomain,
    kOpInOutNone};
  DbSubOp dbop_read = {kOpReadSingle, kOpMatchCtrlr|kOpMatchDomain,
                       kOpInOutNone};
  while (tmp_ckv) {
    GET_USER_DATA_CTRLR_DOMAIN(tmp_ckv, ctrlr_dom);
    SET_USER_DATA_CTRLR_DOMAIN(vtn_ckv, ctrlr_dom);
    // read vtn_ctrlr_tbl to delete based on ref_count
    result_code = vtn_mgr->ReadConfigDB(vtn_ckv, UPLL_DT_CANDIDATE, UNC_OP_READ,
        dbop_read, dmi, CONVERTTBL);
    if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      DELETE_IF_NOT_NULL(vtn_ckv);
      UPLL_LOG_DEBUG("Entry is not exists in vtn gateway port table");
      return result_code;
    }
    if (result_code != UPLL_RC_SUCCESS) {
      DELETE_IF_NOT_NULL(vtn_ckv);
      UPLL_LOG_DEBUG("Vtn gateway port table read failed:%d", result_code);
      return result_code;
    }
    if (static_cast<val_vtn_gateway_port*>(GetVal(vtn_ckv))->ref_count > 1) {
      static_cast<val_vtn_gateway_port*>(GetVal(vtn_ckv))->ref_count--;
      result_code = vtn_mgr->UpdateConfigDB(vtn_ckv, UPLL_DT_CANDIDATE,
        UNC_OP_UPDATE, dmi, &dbop_up, config_mode, vtn_name, CONVERTTBL);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_TRACE("vtn gateway port table ref_count update failed:%d",
            result_code);
        DELETE_IF_NOT_NULL(vtn_ckv);
        return result_code;
      }
    } else {
      result_code = vtn_mgr->UpdateConfigDB(vtn_ckv, UPLL_DT_CANDIDATE,
         UNC_OP_DELETE, dmi, &dbop_up, config_mode, vtn_name, CONVERTTBL);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_TRACE("vtn gateway port table delete failed:%d", result_code);
        DELETE_IF_NOT_NULL(vtn_ckv);
        return result_code;
      }
    }
    vtn_ckv->DeleteCfgVal();
    tmp_ckv = tmp_ckv->get_next_cfg_key_val();
  }
  DELETE_IF_NOT_NULL(vtn_ckv);
  return result_code;
}

/** This function is called when
 * 1. user deletes vbr portmap
 * 2. when internal vbr portmap ref_count is 1 and that needs to be deleted
 */
upll_rc_t VbrPortMapMoMgr::DeleteMo(IpcReqRespHeader *req,
                                    ConfigKeyVal *ikey,
                                    DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  result_code = ValidateMessage(req, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Validate Message is Failed %d ", result_code);
    return result_code;
  }

  ConfigKeyVal *dup_ckv = NULL;
  result_code = GetChildConfigKey(dup_ckv, ikey);
  if (result_code != UPLL_RC_SUCCESS)
    return result_code;
  DbSubOp dbop = { kOpReadSingle, kOpMatchNone,
    kOpInOutCtrlr|kOpInOutDomain|kOpInOutFlag};
  result_code = ReadConfigDB(dup_ckv, UPLL_DT_CANDIDATE, UNC_OP_READ, dbop,
      dmi, MAINTBL);
  UPLL_LOG_TRACE("Read config DB result code = %u", result_code);

  if (result_code != UPLL_RC_SUCCESS) {
    if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE)
      UPLL_LOG_DEBUG("Record Doesn't Exists in DB ");
    DELETE_IF_NOT_NULL(dup_ckv);
    return result_code;
  }
  // check whether for the same record, boundary flag also exists,
  pfcdrv_val_vbr_portmap_t *db_drv_val = reinterpret_cast<
    pfcdrv_val_vbr_portmap_t *>(GetVal(dup_ckv));

  uint8_t flags = 0;
  GET_USER_DATA_FLAGS(dup_ckv, flags);
  uint8_t ikey_flag = 0;
  GET_USER_DATA_FLAGS(ikey, ikey_flag);

  // check user tries to delete internally created vbr-portmap
  if (!(ikey_flag & BOUNDARY_VBRPORTMAP_FLAG) &&
      (!(flags & USER_VBRPORTMAP_FLAG))) {
    UPLL_LOG_DEBUG("Given instance is not created by user, so can't delete");
    DELETE_IF_NOT_NULL(dup_ckv);
    return UPLL_RC_ERR_NO_SUCH_INSTANCE;
  }

  // get the config mode to use in updateconfigDB
  TcConfigMode config_mode = TC_CONFIG_INVALID;
  std::string vtn_name;
  result_code = GetConfigModeInfo(req, config_mode, vtn_name);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_TRACE("GetConfigMode failed");
    DELETE_IF_NOT_NULL(dup_ckv);
    return result_code;
  }

  // check if boundary and user flag are set, then unset the user flag in DB
  UPLL_LOG_TRACE("Vbrportmap flag = %u. bdry_ref_count = %u", flags,
      db_drv_val->bdry_ref_count);

  DbSubOp dbop1 = { kOpNotRead, kOpMatchNone, kOpInOutFlag };
  if ((db_drv_val->bdry_ref_count > 0) && (flags & USER_VBRPORTMAP_FLAG)) {
    if (ikey_flag & BOUNDARY_VBRPORTMAP_FLAG) {
      // unset the boundary flag
      flags &= ~BOUNDARY_VBRPORTMAP_FLAG;
    } else {
      // unset user flag
      flags &= ~USER_VBRPORTMAP_FLAG;
    }
    SET_USER_DATA_FLAGS(dup_ckv, flags);
    UPLL_LOG_TRACE("Flag to be updated = %u", flags);

    result_code = UpdateConfigDB(dup_ckv, UPLL_DT_CANDIDATE, UNC_OP_UPDATE,
        dmi, &dbop1, config_mode, vtn_name, MAINTBL);
    if (result_code != UPLL_RC_SUCCESS) {
      DELETE_IF_NOT_NULL(dup_ckv);
      UPLL_LOG_TRACE("Update vbr portmap flag is failed");
      return result_code;
    }
    return result_code;
  } else {
    result_code = UpdateConfigDB(dup_ckv, UPLL_DT_CANDIDATE, UNC_OP_DELETE,
        dmi, &dbop1, config_mode, vtn_name, MAINTBL);
    if (result_code != UPLL_RC_SUCCESS) {
      DELETE_IF_NOT_NULL(dup_ckv);
      UPLL_LOG_DEBUG("vbr portmap delete failed:%d", result_code);
      return result_code;
    }
  }

  // send val_vbr_portmap instead of pfcdrv_val_vbr_portmap while invoking
  // DeleteVNode
  val_vbr_portmap *val_vbrpm = ConfigKeyVal::Malloc<val_vbr_portmap_t>();
  memcpy(val_vbrpm, &(static_cast<pfcdrv_val_vbr_portmap*>(
          dup_ckv->get_cfg_val()->get_val())->vbrpm),
      sizeof(val_vbr_portmap_t));
  dup_ckv->get_cfg_val()->SetVal(IpctSt::kIpcStValVbrPortMap, val_vbrpm);

  // if the given vbr portmap configured with logical_port_id then
  // delete the converted vnodes
  if (val_vbrpm->valid[UPLL_IDX_LOGICAL_PORT_ID_VBRPM] == UNC_VF_VALID) {
    result_code = DeleteVNode(dup_ckv, req, dmi, config_mode, vtn_name);
  }
  DELETE_IF_NOT_NULL(dup_ckv);
  return result_code;
}

upll_rc_t VbrPortMapMoMgr::AdaptValToVtnService(ConfigKeyVal *ikey,
    AdaptType adapt_type) {
  UPLL_FUNC_TRACE;
  while (ikey) {
    ConfigVal *cval = ikey->get_cfg_val();
    while (cval) {
      if (IpctSt::kIpcStPfcdrvValVbrPortMap == cval->get_st_num()) {
        val_vbr_portmap_t *val_vbrpm = reinterpret_cast<val_vbr_portmap_t *>
          (ConfigKeyVal::Malloc(sizeof(val_vbr_portmap_t)));
        pfcdrv_val_vbr_portmap_t *ikey_val =
          reinterpret_cast<pfcdrv_val_vbr_portmap_t *>(cval->get_val());
        memcpy(val_vbrpm, &(ikey_val->vbrpm), sizeof(val_vbr_portmap_t));
        cval->SetVal(IpctSt::kIpcStValVbrPortMap, val_vbrpm);
      }
      cval = cval->get_next_cfg_val();
    }
    if (adapt_type == ADAPT_ONE)
      break;
    ikey = ikey->get_next_cfg_key_val();
  }
  return UPLL_RC_SUCCESS;
}


upll_rc_t VbrPortMapMoMgr::ReadSiblingMo(IpcReqRespHeader *header,
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

  /* Convert user val struct to pfc_drv val struct */
  val_vbr_portmap *vl = reinterpret_cast<val_vbr_portmap*>(GetVal(ikey));
  if (vl != NULL) {
    pfcdrv_val_vbr_portmap *val =
                          ConfigKeyVal::Malloc<pfcdrv_val_vbr_portmap>();

    memcpy(&val->vbrpm, vl, sizeof(val_vbr_portmap_t));
    val->valid[0] = UNC_VF_VALID;
    val->valid[1] = UNC_VF_INVALID;
    (queryckval->get_cfg_val())->SetVal(IpctSt::kIpcStPfcdrvValVbrPortMap, val);
  }

  uint32_t found_count = 0, skip_count = 0, tmp_count = 0, result_count =0;
  upll_keytype_datatype_t dt_type =  header->datatype;
  unc_keytype_operation_t operation = header->operation;

  do {
    ConfigKeyVal *new_ckv = NULL;
    result_code = ReadInfoFromDB(header, queryckval, dmi, &ctrlr_dom);
    if (result_code == UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("ReadInfoFromDB returns SUCCESS: %s \n",
                     queryckval->ToStrAll().c_str());
      if ((header->rep_count > 0) || (header->operation == UNC_OP_READ)) {
        tmp_count = header->rep_count;
        skip_count = 0;
        new_ckv = NULL;
        ConfigKeyVal *tmp = queryckval, *tmp_old = NULL;
        while (tmp) {
          uint8_t vbrpm_flag = 0;
          GET_USER_DATA_FLAGS(tmp, vbrpm_flag);
          uint8_t is_user_conf = false;
          is_user_conf = (USER_VBRPORTMAP_FLAG & vbrpm_flag);
          if (!is_user_conf) {
            ConfigKeyVal *ck_tmp =tmp;
            skip_count++;
            tmp = tmp->get_next_cfg_key_val();
            if (tmp == NULL) {
              DELETE_IF_NOT_NULL(new_ckv);
              if (header->operation == UNC_OP_READ) {
                break;
              }
              // else frame new_ckv for next ReadInfoFromDB call if required
              result_code = DupConfigKeyVal(new_ckv, ck_tmp, MAINTBL);
              if (result_code != UPLL_RC_SUCCESS || new_ckv == NULL) {
                UPLL_LOG_ERROR("Error DupConfigKeyVal.%d", result_code);
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
              if (header->operation == UNC_OP_READ) {
                break;
              }
              // else frame new_ckv for next ReadInfoFromDB call if required
              DELETE_IF_NOT_NULL(new_ckv);
              result_code = DupConfigKeyVal(new_ckv, tmp_old, MAINTBL);
              if (result_code != UPLL_RC_SUCCESS || new_ckv == NULL) {
                UPLL_LOG_ERROR("Error DupConfigKeyVal.%d", result_code);
                DELETE_IF_NOT_NULL(queryckval);
                return UPLL_RC_ERR_GENERIC;
              }
            }
          }
        }
        if (header->operation == UNC_OP_READ) {
          uint32_t read_res_cnt =  (queryckval->size());
          if (read_res_cnt == skip_count) {
            result_code = UPLL_RC_ERR_NO_SUCH_INSTANCE;
            DELETE_IF_NOT_NULL(queryckval);
            break;
          }
        }
        found_count = (tmp_count-skip_count);
        if (found_count > 0 || header->operation == UNC_OP_READ) {
          // Collect the data into result_ckv.
          if (result_ckv == NULL) {
            result_ckv = queryckval;
          } else {
            result_ckv->AppendCfgKeyVal(queryckval);
          }
          result_count += (queryckval->size());
        }
        if (header->operation == UNC_OP_READ) {
          // in case of read operation break
          DELETE_IF_NOT_NULL(new_ckv);
          break;
        }
        if (found_count < tmp_count) {
          queryckval = new_ckv;
          header->operation = UNC_OP_READ_SIBLING;
          header->rep_count = tmp_count - result_count;
          // Preparing Child Key data for next Sibling Iteration
         queryckval->DeleteCfgVal();
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
  header->operation = operation;
  header->datatype = dt_type;

  /* Convert the valstruct to VTN service val*/
  upll_rc_t rc = AdaptValToVtnService(ikey, ADAPT_ALL);
  if (rc != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("AdaptValToVtnService failed result_code %d", rc);
    return UPLL_RC_ERR_GENERIC;
  }

  if (header->operation == UNC_OP_READ) {
    return result_code;
  }
  if (header->rep_count > 0) {
    result_code = (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE)
        ? UPLL_RC_SUCCESS : result_code;
  } else {
    return UPLL_RC_ERR_NO_SUCH_INSTANCE;
  }
  return result_code;
}

upll_rc_t VbrPortMapMoMgr::ReadSiblingCount(IpcReqRespHeader *header,
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
  val_vbr_portmap *vl = reinterpret_cast<val_vbr_portmap*>(GetVal(ikey));
  if (vl != NULL) {
    pfcdrv_val_vbr_portmap *val = reinterpret_cast<pfcdrv_val_vbr_portmap_t *>
      (ConfigKeyVal::Malloc(sizeof(pfcdrv_val_vbr_portmap_t)));

    memcpy(&val->vbrpm, vl, sizeof(val_vbr_portmap_t));
    val->valid[0] = UNC_VF_VALID;
    val->valid[1] = UNC_VF_INVALID;
    (ikey->get_cfg_val())->SetVal(IpctSt::kIpcStPfcdrvValVbrPortMap, val);
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
        uint8_t vbrpm_flag = 0;
        GET_USER_DATA_FLAGS(tmp, vbrpm_flag);
        uint8_t is_user_conf = false;
        is_user_conf = (USER_VBRPORTMAP_FLAG & vbrpm_flag);
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

upll_rc_t VbrPortMapMoMgr::ReadMo(IpcReqRespHeader *header,
    ConfigKeyVal *ikey,
    DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  return ReadSiblingMo(header, ikey, false, dmi);
}

upll_rc_t VbrPortMapMoMgr::UpdateConfigStatus(ConfigKeyVal *ikey,
                                              unc_keytype_operation_t op,
                                              uint32_t driver_result,
                                              ConfigKeyVal *upd_key,
                                              DalDmlIntf *dmi,
                                              ConfigKeyVal *ctrlr_key) {
  UPLL_FUNC_TRACE;

  pfcdrv_val_vbr_portmap_t *drv_vbrpm_val =
    reinterpret_cast<pfcdrv_val_vbr_portmap_t*>(GetVal(ikey));
  if (drv_vbrpm_val == NULL) return UPLL_RC_ERR_GENERIC;
  val_vbr_portmap_t *vbrpm_val = &(drv_vbrpm_val->vbrpm);
  pfcdrv_val_vbr_portmap *drv_vbrpm_val2 = NULL;
  val_vbr_portmap_t *vbrpm_val2 = NULL;
  unc_keytype_configstatus_t cs_status =
      (driver_result == UPLL_RC_SUCCESS) ? UNC_CS_APPLIED : UNC_CS_NOT_APPLIED;

  UPLL_LOG_TRACE("Key from Candidate %s", (ikey->ToStrAll()).c_str());
  bool oper_change = false;
  if (op == UNC_OP_CREATE) {
    if (vbrpm_val->valid[UPLL_IDX_LOGICAL_PORT_ID_VBRPM] != UNC_VF_INVALID)
      vbrpm_val->cs_row_status = cs_status;
    else
      vbrpm_val->cs_row_status = UNC_CS_NOT_APPLIED;
    oper_change = true;
  } else if (op == UNC_OP_UPDATE) {
    drv_vbrpm_val2 =
        reinterpret_cast<pfcdrv_val_vbr_portmap *>(GetVal(upd_key));
    if (drv_vbrpm_val2 == NULL) return UPLL_RC_ERR_GENERIC;
    vbrpm_val2= &(drv_vbrpm_val2->vbrpm);

    void *val = reinterpret_cast<void *>(drv_vbrpm_val);
    if (vbrpm_val->valid[UPLL_IDX_LOGICAL_PORT_ID_VBRPM] !=
        vbrpm_val2->valid[UPLL_IDX_LOGICAL_PORT_ID_VBRPM]) {
      oper_change = true;
    }
    CompareValidValue(val, GetVal(upd_key), true);
    UPLL_LOG_TRACE("Key from Running %s", (upd_key->ToStrAll()).c_str());
    if (
        (vbrpm_val2->valid[UPLL_IDX_LOGICAL_PORT_ID_VBRPM] == UNC_VF_INVALID) &&
        (vbrpm_val->valid[UPLL_IDX_LOGICAL_PORT_ID_VBRPM] == UNC_VF_VALID)) {
      vbrpm_val->cs_row_status = cs_status;
      // change the ctrlr,domain cs_attr to applied based on controller status
      vbrpm_val2->cs_attr[UPLL_IDX_CONTROLLER_ID_VBRPM] = cs_status;
      vbrpm_val2->cs_attr[UPLL_IDX_DOMAIN_ID_VBRPM] = cs_status;
    } else {
      vbrpm_val->cs_row_status = vbrpm_val2->cs_row_status;
    }
  } else {
    UPLL_LOG_DEBUG("Invalid op %d", op);
    return UPLL_RC_ERR_GENERIC;
  }
  if (oper_change) {
    val_db_vbr_portmap_st *vbrpm_st =
        reinterpret_cast<val_db_vbr_portmap_st *>
        (ConfigKeyVal::Malloc(sizeof(val_db_vbr_portmap_st)));
    if (op == UNC_OP_CREATE) {
      vbrpm_st->vbr_portmap_val_st.oper_status =
        (driver_result == UPLL_RC_SUCCESS)?
          UPLL_OPER_STATUS_UNINIT: UPLL_OPER_STATUS_UNKNOWN;
    } else if (op == UNC_OP_UPDATE) {
      val_db_vbr_portmap_st *run_vbrpmst =
         reinterpret_cast<val_db_vbr_portmap_st*>(GetStateVal(upd_key));
      vbrpm_st->vbr_portmap_val_st.oper_status =
          run_vbrpmst->vbr_portmap_val_st.oper_status;
      vbrpm_st->down_count = run_vbrpmst->down_count;
    }
    vbrpm_st->vbr_portmap_val_st.valid[0]= UNC_VF_VALID;
    ikey->AppendCfgVal(IpctSt::kIpcStValVbrPortMapSt, vbrpm_st);
  }
  for (unsigned int loop = 0;
      loop < sizeof(vbrpm_val->valid) / sizeof(vbrpm_val->valid[0]);
      ++loop) {
    if ((UNC_VF_VALID == (uint8_t) vbrpm_val->valid[loop])
        || (UNC_VF_VALID_NO_VALUE == (uint8_t) vbrpm_val->valid[loop])) {
      vbrpm_val->cs_attr[loop] = cs_status;
      if (UNC_VF_VALID_NO_VALUE == (uint8_t) vbrpm_val->valid[loop]) {
        vbrpm_val->cs_row_status = UNC_CS_NOT_APPLIED;
        vbrpm_val->cs_attr[loop] = UNC_CS_NOT_APPLIED;
        if (loop == UPLL_IDX_LOGICAL_PORT_ID_VBRPM) {
          vbrpm_val->cs_attr[UPLL_IDX_CONTROLLER_ID_VBRPM] = UNC_CS_NOT_APPLIED;
          vbrpm_val->cs_attr[UPLL_IDX_DOMAIN_ID_VBRPM] = UNC_CS_NOT_APPLIED;
        }
      }
    } else if ((UNC_VF_INVALID == vbrpm_val->valid[loop]) &&
        (UNC_OP_CREATE == op)) {
      vbrpm_val->cs_attr[loop] = UNC_CS_NOT_APPLIED;
    } else if ((UNC_VF_INVALID == vbrpm_val->valid[loop]) &&
        (UNC_OP_UPDATE == op)) {
      vbrpm_val->cs_attr[loop] = vbrpm_val2->cs_attr[loop];
    }
  }
  return (SetInterfaceOperStatus(ikey, dmi, op, true, driver_result));
}

upll_rc_t VbrPortMapMoMgr::UpdateAuditConfigStatus(
    unc_keytype_configstatus_t cs_status,
    uuc::UpdateCtrlrPhase phase,
    ConfigKeyVal *&ckv_running,
    DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;

  upll_rc_t result_code = UPLL_RC_SUCCESS;
  pfcdrv_val_vbr_portmap_t *drv_val = NULL;
  val_vbr_portmap_t        *val     = NULL;
  drv_val = (ckv_running != NULL) ? reinterpret_cast<pfcdrv_val_vbr_portmap_t *>
    (GetVal(ckv_running)) : NULL;
  if (NULL == drv_val) {
    return UPLL_RC_ERR_GENERIC;
  }
  val = &(drv_val->vbrpm);
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
  VbrMoMgr *vbr_mgr = reinterpret_cast<VbrMoMgr *>(const_cast<MoManager *>(
        GetMoManager(UNC_KT_VBRIDGE)));
  result_code = vbr_mgr->UpdateUvbrConfigStatusAuditUpdate(phase,
                                                           ckv_running, dmi);
  return result_code;
}

upll_rc_t VbrPortMapMoMgr::CopyToConfigKey(ConfigKeyVal *&okey,
    ConfigKeyVal *ikey) {
  UPLL_FUNC_TRACE;

  key_rename_vnode_info *key_rename =
    reinterpret_cast<key_rename_vnode_info *>(ikey->get_key());
  key_vbr_portmap_t* key_vbrpm = reinterpret_cast<key_vbr_portmap_t *>
    (ConfigKeyVal::Malloc(sizeof(key_vbr_portmap_t)));
  if (!strlen(reinterpret_cast<char *>(key_rename->old_unc_vtn_name))) {
    free(key_vbrpm);
    return UPLL_RC_ERR_GENERIC;
  }
  uuu::upll_strncpy(key_vbrpm->vbr_key.vtn_key.vtn_name,
      key_rename->old_unc_vtn_name,
      kMaxLenVtnName+1);
  if (ikey->get_key_type() == UNC_KT_VBRIDGE) {
    if (!strlen(reinterpret_cast<char *>(key_rename->old_unc_vnode_name))) {
      free(key_vbrpm);
      return UPLL_RC_ERR_GENERIC;
    }
    uuu::upll_strncpy(key_vbrpm->vbr_key.vbridge_name,
        key_rename->old_unc_vnode_name, kMaxLenVnodeName+1);
  } else {
    if (!strlen(reinterpret_cast<char *>(key_rename->new_unc_vnode_name))) {
      free(key_vbrpm);
      return UPLL_RC_ERR_GENERIC;
    }
    uuu::upll_strncpy(key_vbrpm->vbr_key.vbridge_name,
        key_rename->new_unc_vnode_name, (kMaxLenVnodeName + 1));
  }

  okey = new ConfigKeyVal(UNC_KT_VBR_PORTMAP, IpctSt::kIpcStKeyVbrPortMap,
      key_vbrpm, NULL);

  if (!okey) {
    free(key_vbrpm);
    return UPLL_RC_ERR_GENERIC;
  }
  return UPLL_RC_SUCCESS;
}

/** this function handles the boundary(internal vbr-portmap) and user
 * configured vbr-portmap vlan-id update.
 * and logical-port-id unconfiguring from user configured vbr-portmap
 */
upll_rc_t VbrPortMapMoMgr::UpdateMo(IpcReqRespHeader *req,
                                    ConfigKeyVal *ikey,
                                    DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_ERR_GENERIC;
  ConfigKeyVal *dup_ikey = NULL;

  result_code = ValidateMessage(req, ikey);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("Validation Message is Failed ");
    return result_code;
  }

  controller_domain ctrlr_dom;
  ctrlr_dom.ctrlr = ctrlr_dom.domain = NULL;

  // do the record existance check
  ConfigKeyVal *temp_ck = NULL;
  result_code = GetChildConfigKey(temp_ck, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Error in retrieving the Child ConfigKeyVal");
    DELETE_IF_NOT_NULL(temp_ck);
    return result_code;
  }
  DbSubOp dbop = { kOpReadSingle, kOpMatchNone,
    kOpInOutFlag|kOpInOutCtrlr|kOpInOutDomain  };
  result_code = ReadConfigDB(temp_ck, UPLL_DT_CANDIDATE, UNC_OP_READ,
      dbop, dmi, MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_TRACE("ReadConfigDB Failed %d", result_code);
    delete temp_ck;
    temp_ck = NULL;
    return result_code;
  }

  GET_USER_DATA_CTRLR_DOMAIN(temp_ck, ctrlr_dom);
  SET_USER_DATA_CTRLR_DOMAIN(ikey, ctrlr_dom);
  uint8_t ikey_flag = 0;
  uint8_t db_flag = 0;
  GET_USER_DATA_FLAGS(temp_ck, db_flag);
  GET_USER_DATA_FLAGS(ikey, ikey_flag);

  // check user tries to update internally created vbr-portmap
  if (!(ikey_flag & BOUNDARY_VBRPORTMAP_FLAG) &&
      (!(db_flag & USER_VBRPORTMAP_FLAG))) {
    UPLL_LOG_DEBUG("Given instance is not created by user, so can't update");
    DELETE_IF_NOT_NULL(temp_ck);
    return UPLL_RC_ERR_NO_SUCH_INSTANCE;
  }
  SET_USER_DATA_FLAGS(ikey, db_flag);
  ikey_flag = db_flag;

  val_vbr_portmap_t *ikey_val = NULL;
  result_code = GetValVbrPortMap(ikey, &ikey_val);
  if (result_code != UPLL_RC_SUCCESS) {
    DELETE_IF_NOT_NULL(temp_ck);
    return result_code;
  }

  bool is_invalid = IsAllAttrInvalid(ikey_val);
  if (is_invalid) {
    UPLL_LOG_INFO("No attributes to be updated");
    DELETE_IF_NOT_NULL(temp_ck);
    return UPLL_RC_SUCCESS;
  }
  // ValidateAttribute needs Controller Domain
  result_code = ValidateAttribute(ikey, dmi, req);
  if (UPLL_RC_SUCCESS  != result_code) {
    UPLL_LOG_DEBUG("Validate Attribute is Failed");
    DELETE_IF_NOT_NULL(temp_ck);
    return result_code;
  }

  result_code = ValidateCapability(
      req, ikey, reinterpret_cast<char *>(ctrlr_dom.ctrlr));
  if (UPLL_RC_SUCCESS  != result_code) {
    UPLL_LOG_TRACE("Validate Capability is Failed. Error_code : %d",
        result_code);
    delete temp_ck;
    temp_ck = NULL;
    return result_code;
  }

  TcConfigMode config_mode = TC_CONFIG_INVALID;
  std::string vtn_name;
  result_code = GetConfigModeInfo(req, config_mode, vtn_name);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_TRACE("GetConfigMode failed");
    DELETE_IF_NOT_NULL(temp_ck);
    return result_code;
  }

  dbop.matchop = kOpMatchCtrlr | kOpMatchDomain;

  pfcdrv_val_vbr_portmap_t *db_pfcdrv_val = reinterpret_cast<
    pfcdrv_val_vbr_portmap_t *>(GetVal(temp_ck));

  // if a record is refered by both boundary and user and if the user configured
  // label/logical_port_id is updated then separate user recrod from boundary
  if ((db_pfcdrv_val->bdry_ref_count > 0) &&
      (ikey_flag & USER_VBRPORTMAP_FLAG) &&
      (ikey_flag & BOUNDARY_VBRPORTMAP_FLAG)) {
    UPLL_LOG_TRACE("VbrPortmap bdry_ref_count = %u",
        db_pfcdrv_val->bdry_ref_count);
    // do any-vlan-id check,
    if (ikey_val->label == ANY_VLAN_ID) {
      UPLL_LOG_WARN("Can't change normal label to any_vlan_id label");
      DELETE_IF_NOT_NULL(temp_ck);
      return UPLL_RC_ERR_VLAN_TYPE_MISMATCH;
    }
    // unset the bdry flag and ref_count for the existing entry and
    // auto-generate the portmap id and copy the DB data including
    // bdry_ref_count
    // flag to new record
    result_code = SeparateBoundaryAndUserRecordInDB(
        temp_ck, ikey_val, dmi, config_mode, vtn_name);
    DELETE_IF_NOT_NULL(temp_ck);
    return result_code;
  }
  if (ikey_val->valid[UPLL_IDX_LOGICAL_PORT_ID_VBRPM] == UNC_VF_VALID) {
    // During create, logical_port_id is not configured, it is configured in
    // update, do the conversion
    // This scneario will come for user configuration and not for boundary
    result_code = ConvertVNode(ikey, dmi, config_mode, vtn_name, req);
    GET_USER_DATA_FLAGS(ikey, ikey_flag);
    if (result_code != UPLL_RC_SUCCESS) {
      DELETE_IF_NOT_NULL(dup_ikey);
      DELETE_IF_NOT_NULL(temp_ck);
      UPLL_LOG_DEBUG("vnode conversion failed:%d", result_code);
      return result_code;
    }
    GET_USER_DATA_FLAGS(ikey, ikey_flag);
  }

  // create dup_ikey to  use in UpdateConfigDB
  result_code = GetChildConfigKey(dup_ikey, ikey);
  if (!dup_ikey || result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_TRACE("Returning error %d", result_code);
    return result_code;
  }

  // create dup_ckv to convert ikey to pfcdrv format and update in db
  if (IpctSt::kIpcStValVbrPortMap == ikey->get_cfg_val()->get_st_num()) {
    pfcdrv_val_vbr_portmap_t *val = reinterpret_cast<pfcdrv_val_vbr_portmap_t *>
      (ConfigKeyVal::Malloc(sizeof(pfcdrv_val_vbr_portmap_t)));

    memcpy(&(val->vbrpm), ikey_val, sizeof(val_vbr_portmap_t));
    val->valid[PFCDRV_IDX_VAL_VBR_PORTMAP] = UNC_VF_VALID;
    dup_ikey->AppendCfgVal(IpctSt::kIpcStPfcdrvValVbrPortMap, val);
  } else {
    DELETE_IF_NOT_NULL(dup_ikey);
    dup_ikey = ikey;
  }

  // update the ca_vbr_portmap_tbl
  DbSubOp dbop1 = {kOpNotRead, kOpMatchNone, kOpInOutFlag};

  result_code = UpdateConfigDB(dup_ikey, UPLL_DT_CANDIDATE, UNC_OP_UPDATE,
      dmi, &dbop1, config_mode, vtn_name, MAINTBL);

  if (IpctSt::kIpcStValVbrPortMap == ikey->get_cfg_val()->get_st_num())
    DELETE_IF_NOT_NULL(dup_ikey);

  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("Updation Failure in DB : %d", result_code);
    DELETE_IF_NOT_NULL(temp_ck);
    return result_code;
  }

  // logical_port_id is unconfigured, remove the converted vnode if required
  if ((IpctSt::kIpcStValVbrPortMap == ikey->get_cfg_val()->get_st_num())  &&
      ((db_pfcdrv_val->vbrpm.valid[UPLL_IDX_LOGICAL_PORT_ID_VBRPM] ==
       UNC_VF_VALID) &&
       (static_cast<val_vbr_portmap*>(GetVal(ikey))->valid[
                           UPLL_IDX_LOGICAL_PORT_ID_VBRPM] != UNC_VF_VALID))) {
    DELETE_IF_NOT_NULL(temp_ck);
      // check vtn_rename flag is set or not
    bool vtn_rename = false;
    uint8_t vbr_pm_flag = 0;
    GET_USER_DATA_FLAGS(ikey, vbr_pm_flag);
    if (vbr_pm_flag & VTN_RENAME) {
      vtn_rename = true;
    }
    result_code = DeleteVNode(ikey, req, dmi, config_mode, vtn_name);
    if (result_code != UPLL_RC_SUCCESS) {
      return result_code;
    }
    if (vtn_rename) {
      // check whether vtn rename flag is unset
      vbr_pm_flag = 0;
      GET_USER_DATA_FLAGS(ikey, vbr_pm_flag);
      if (!(vbr_pm_flag & VTN_RENAME)) {
        // unset the flag in DB
            ConfigKeyVal *temp_key = NULL;
        result_code = GetChildConfigKey(temp_key, ikey);
        if (UPLL_RC_SUCCESS != result_code) {
          UPLL_LOG_DEBUG("GetChildConfigKey Failed:%d", result_code);
          return result_code;
        }
        result_code = UpdateConfigDB(temp_key, UPLL_DT_CANDIDATE, UNC_OP_UPDATE,
                                   dmi, &dbop, config_mode, vtn_name, MAINTBL);
        DELETE_IF_NOT_NULL(temp_key);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_ERROR("UpdateConfigDB failed:%d", result_code);
        }
      }
    }
  }
  UPLL_LOG_TRACE("Updated Done Successfully %d", result_code);
  DELETE_IF_NOT_NULL(temp_ck);
  return result_code;
}

upll_rc_t VbrPortMapMoMgr::SeparateBoundaryAndUserRecordInDB(
    ConfigKeyVal *db_ckv, val_vbr_portmap *ikey_val,
    DalDmlIntf *dmi, TcConfigMode config_mode, string vtn_name) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  uint8_t db_flag = 0;
  GET_USER_DATA_FLAGS(db_ckv, db_flag);

  // vlan-id is updated, create two separate entries one for boundary and one
  // for user-configuration
  ConfigKeyVal *bdry_ckv = NULL;
  // get the duplicate key
  result_code = DupConfigKeyVal(bdry_ckv, db_ckv);
  if (result_code != UPLL_RC_SUCCESS) {
    return result_code;
  }

  // auto generate vbr portmap Id
  result_code = GeneratePortMapId(bdry_ckv, dmi);
  if (result_code != UPLL_RC_SUCCESS) {
    DELETE_IF_NOT_NULL(bdry_ckv);
    return result_code;
  }

  uint8_t bdry_flag = db_flag;
  // update the boundary data in DB
  bdry_flag &= ~USER_VBRPORTMAP_FLAG;
  SET_USER_DATA_FLAGS(bdry_ckv, bdry_flag);

  DbSubOp dbop_update = {kOpNotRead, kOpMatchNone, kOpInOutFlag};

  result_code = UpdateConfigDB(bdry_ckv, UPLL_DT_CANDIDATE, UNC_OP_CREATE, dmi,
      &dbop_update, config_mode, vtn_name, MAINTBL);

  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_ERROR("UpdateConfigDB failed:%d", result_code);
    DELETE_IF_NOT_NULL(bdry_ckv);
    return result_code;
  }

  // unset the bdry_flag and ref_count in db_ckv to proceed the further checking
  // for user request
  db_flag &= ~BOUNDARY_VBRPORTMAP_FLAG;
  pfcdrv_val_vbr_portmap* db_val = static_cast<pfcdrv_val_vbr_portmap*>(
      GetVal(bdry_ckv));
  uuu::upll_strncpy(
      static_cast<key_vbr_portmap*>(bdry_ckv->get_key())->portmap_id,
      static_cast<key_vbr_portmap*>(db_ckv->get_key())->portmap_id,
      kMaxLenPortMapName+1);

  SET_USER_DATA_FLAGS(bdry_ckv, db_flag);
  if (ikey_val->valid[UPLL_IDX_LOGICAL_PORT_ID_VBRPM] ==
      UNC_VF_VALID_NO_VALUE) {
    // set only  user flag
    SET_USER_DATA_FLAGS(bdry_ckv, USER_VBRPORTMAP_FLAG);
    // reset the value and valid flags for val structure except ctrlr, domain
    db_val->vbrpm.valid[UPLL_IDX_LOGICAL_PORT_ID_VBRPM] = UNC_VF_VALID_NO_VALUE;
    db_val->vbrpm.valid[UPLL_IDX_LABEL_TYPE_VBRPM] = UNC_VF_VALID_NO_VALUE;
    db_val->vbrpm.valid[UPLL_IDX_LABEL_VBRPM] = UNC_VF_VALID_NO_VALUE;
    StringReset(db_val->vbrpm.logical_port_id);
    db_val->vbrpm.label_type = 0;
    db_val->vbrpm.label = 0;
  } else {
    uuu::upll_strncpy(db_val->vbrpm.logical_port_id, ikey_val->logical_port_id,
                      kMaxLenLogicalPortId+1);
    // update the label id
    db_val->vbrpm.label = ikey_val->label;
  }

  // reset the ref_count
  db_val->bdry_ref_count = 0;
  db_val->valid[PFCDRV_IDX_VBRPM_BDRY_REF_COUNT] = UNC_VF_VALID_NO_VALUE;
  // update the flag and user-updates in DB
  result_code = UpdateConfigDB(bdry_ckv, UPLL_DT_CANDIDATE, UNC_OP_UPDATE, dmi,
                               &dbop_update, config_mode, vtn_name, MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_ERROR("UpdateConfigDB for user update failed:%d", result_code);
  }
  DELETE_IF_NOT_NULL(bdry_ckv);
  return result_code;
}


upll_rc_t VbrPortMapMoMgr::GeneratePortMapId(ConfigKeyVal *&db_ckv,
    DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  // Auto generate portmap id
  std::string auto_pm_id("vlpm_");
  while (1) {
    struct timeval _timeval;
    struct timezone _timezone;
    gettimeofday(&_timeval, &_timezone);

    std::stringstream ss;
    ss << _timeval.tv_sec << _timeval.tv_usec;
    std::string unique_id = ss.str();
    auto_pm_id += unique_id;

    // check the auto-generated name exists in vbr_portmap tbl
    ConfigKeyVal *tmp_ckv = NULL;
    result_code = GetChildConfigKey(tmp_ckv, db_ckv);
    if (result_code != UPLL_RC_SUCCESS)
      return result_code;

    uuu::upll_strncpy(static_cast<key_vbr_portmap*>(
          tmp_ckv->get_key())->portmap_id, auto_pm_id.c_str(),
        kMaxLenPortMapName+1);

    DbSubOp dbop = {kOpReadExist, kOpMatchCtrlr, kOpInOutNone};
    result_code = UpdateConfigDB(tmp_ckv, UPLL_DT_CANDIDATE, UNC_OP_READ,
        dmi, &dbop, MAINTBL);
    DELETE_IF_NOT_NULL(tmp_ckv);
    if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      // name is not exists in DB, use this name
      result_code = UPLL_RC_SUCCESS;
      break;
    } else if (result_code == UPLL_RC_ERR_INSTANCE_EXISTS) {
      // same name exists in DB, do one more iteration
      continue;
    } else {
      return result_code;
    }
  }

  // fill the auto generated name in db_ckv
  uuu::upll_strncpy(static_cast<key_vbr_portmap*>(
        db_ckv->get_key())->portmap_id, auto_pm_id.c_str(),
      kMaxLenPortMapName+1);
  return result_code;
}

upll_rc_t VbrPortMapMoMgr::DeleteConvertedVnodeExceptVtunnel(
    ConfigKeyVal *ikey, IpcReqRespHeader *req, DalDmlIntf *dmi,
     TcConfigMode config_mode,  string vtn_name) {
  UPLL_FUNC_TRACE;

  upll_rc_t result_code = UPLL_RC_SUCCESS;

  // Delete the vlink for given vbr portmap
  result_code = DeleteVlink(ikey, dmi, true, config_mode, vtn_name);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("converted vlink deletion for given vbr_portmap failed:%d",
        result_code);
    return result_code;
  }

  // Delete vbr_if for given vbr_portmap
  result_code = DeleteVbrIf(ikey, dmi, true, config_mode, vtn_name);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("converted vbr if deletion for a vbr_portmap"
        "failed:%d", result_code);
    return result_code;
  }

  // Delete vtunnel_if for given vbr_portma
  result_code = DeleteVtunnelIf(ikey, dmi, true, config_mode, vtn_name);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("converted vtunnel if deletion for a vbr_portmap"
        "failed:%d", result_code);
    return result_code;
  }

  // Delete converted vbr for given vbr portmap
  result_code = DeleteVbr(ikey, req, dmi, config_mode, vtn_name);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("converted vbr deletion failed:%d", result_code);
    return result_code;
  }
  return result_code;
}


upll_rc_t VbrPortMapMoMgr::DeleteVNode(ConfigKeyVal *ikey,
    IpcReqRespHeader *req, DalDmlIntf *dmi,
    TcConfigMode config_mode,
    string vtn_name) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  bool same_ctrlr_dom = false;
  uint32_t distinct_ctrlr_dom_count = 0;

  // count the number of vbridge portmap with distinct controller/domain for
  // given vtn, unified vbridge
  result_code = CompareCtrlrDomainInReqWithDB(ikey, dmi, config_mode, vtn_name,
      &same_ctrlr_dom,
      &distinct_ctrlr_dom_count);

  if (result_code != UPLL_RC_SUCCESS) {
    return result_code;
  }

  if (same_ctrlr_dom) {
    // already distinct vbridge portmap with same controller and domain for
    // given unified vbridge exists, no need to do any conversion
    UPLL_LOG_TRACE("same_ctrlr_dom is 1");
    // reset the rename flags if exists in ikey
    uint8_t ikey_flag = 0;
    uint8_t reset_flag = 0;
    reset_flag = BOUNDARY_VBRPORTMAP_FLAG | USER_VBRPORTMAP_FLAG;
    GET_USER_DATA_FLAGS(ikey, ikey_flag);
    ikey_flag &= reset_flag;
    SET_USER_DATA_FLAGS(ikey, ikey_flag);
    return UPLL_RC_SUCCESS;
  }

  if (distinct_ctrlr_dom_count == 0) {
    // last vbridge portmap with logical_port_is removed

    // delete the converted vbr table entry
    uint32_t vbid = 0;
    result_code = GetVbid(ikey, dmi, &vbid);
    if (result_code != UPLL_RC_SUCCESS) {
      return result_code;
    }

    result_code = DeleteVbr(ikey, req, dmi, config_mode, vtn_name);
    if (result_code != UPLL_RC_SUCCESS)
      return result_code;

    // dealloc vbid
    if (vbid != ANY_VLAN_ID) {
      result_code = DeAllocVbid(ikey, vbid, dmi, config_mode, vtn_name);
    }
    return result_code;
  }

  if (distinct_ctrlr_dom_count == 1) {
    UPLL_LOG_TRACE("distinct_ctrlr_dom_count is 1");
    return DeleteConvertedVnodeAndVtunnel(ikey, req, dmi, config_mode,
                                          vtn_name);
  }

  // if count is more than one then delete the converted vnode relevant to the
  // given vbr_portmap
  UPLL_LOG_TRACE("distinct_ctrlr_dom_count is more than 1");
  result_code = DeleteConvertedVnodeExceptVtunnel(ikey, req, dmi, config_mode,
      vtn_name);
  return result_code;
}

upll_rc_t VbrPortMapMoMgr::DeleteConvertedVnodeAndVtunnel(
    ConfigKeyVal *ikey, IpcReqRespHeader *req, DalDmlIntf *dmi,
    TcConfigMode config_mode, string vtn_name) {
  UPLL_FUNC_TRACE;

  upll_rc_t result_code = UPLL_RC_SUCCESS;

  // Delete all the vlink exists for given vtn and unified vbridge
  result_code = DeleteVlink(ikey, dmi, false, config_mode, vtn_name);
  if (result_code != UPLL_RC_SUCCESS)
    return result_code;

  // check the ref_count in vtunnel, if it is one, then delete the vtunnel_if
  // based on vtn and vtunnel name
  // otherwise need to pass the if_name also as it has interfaces for some other
  // unified vbridges exists for given vtn. vtunnel_if_name and vbr_if_name are
  // same, so get the vbr_if_name based on vtn and unified vbr name and use it
  // to delete the vtunnel if
  ConfigKeyVal *vtun_ckv = NULL;
  uint32_t gvtnid = 0;
  result_code = GetVtunnelData(ikey, dmi, vtun_ckv);
  if (result_code != UPLL_RC_SUCCESS)
    return result_code;

  if (static_cast<val_convert_vtunnel*>(GetVal(vtun_ckv))->ref_count == 1) {
    gvtnid = static_cast<val_convert_vtunnel*>(GetVal(vtun_ckv))->label;
  }
  // Delete all converted vtunnel if exists for given vtn
  result_code = DeleteVtunnelIf(ikey, dmi, false, config_mode, vtn_name);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Vtunnel if deletion failed:%d", result_code);
    DELETE_IF_NOT_NULL(vtun_ckv);
    return result_code;
  }

  // Delete all converted vbr_if exists for given vtn and unified vbridge
  result_code = DeleteVbrIf(ikey, dmi, false, config_mode, vtn_name);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("converted vbr_if deletion failed:%d", result_code);
    DELETE_IF_NOT_NULL(vtun_ckv);
    return result_code;
  }

  // Delete converted vtunnel
  result_code = DeleteVtunnel(vtun_ckv, dmi, config_mode, vtn_name);
  if (result_code != UPLL_RC_SUCCESS) {
    DELETE_IF_NOT_NULL(vtun_ckv);
    UPLL_LOG_DEBUG("Vtunnel deletion failed:%d", result_code);
    return result_code;
  }

  uint8_t *ikey_vtn = static_cast<key_vbr_portmap*>(ikey->get_key())->
          vbr_key.vtn_key.vtn_name;

  if (gvtnid > 0) {
    result_code = DeAllocGvtnId(vtun_ckv, dmi, UPLL_DT_CANDIDATE,
                                config_mode, vtn_name);
    if (result_code != UPLL_RC_SUCCESS) {
      DELETE_IF_NOT_NULL(vtun_ckv);
      return result_code;
    }
    // decrement the used label count from spine domain table based on
    // controller and domain name in vtun_ckv
    result_code = DecrementUsedLabelCount(vtun_ckv, ikey_vtn, dmi, config_mode,
                                          vtn_name);
    DELETE_IF_NOT_NULL(vtun_ckv);
    if (result_code != UPLL_RC_SUCCESS)
      return result_code;
  }
  // Delete converted vbr for given vbr portmap
  result_code = DeleteVbr(ikey, req, dmi, config_mode, vtn_name);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("converted vbr deletion failed:%d", result_code);
    return result_code;
  }
  return result_code;
}

/** This function
 * deletes the vtunnel for the given vtn
 */
upll_rc_t VbrPortMapMoMgr::DeleteVtunnel(ConfigKeyVal *vtun_ckv,
    DalDmlIntf *dmi,
    TcConfigMode config_mode,
    string vtn_name) {
  UPLL_FUNC_TRACE;

  upll_rc_t result_code = UPLL_RC_SUCCESS;

  MoMgrImpl *vtun_momgr = reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>(
        GetMoManager(UNC_KT_VTUNNEL)));

  val_convert_vtunnel *val_vtun = static_cast<val_convert_vtunnel*>(
      GetVal(vtun_ckv));
  if (val_vtun->ref_count == 1) {
    result_code = vtun_momgr->ConvertVtunnel(vtun_ckv, NULL, UNC_OP_DELETE,
        config_mode, vtn_name, dmi);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("converted vtunnel table delete failed:%d", result_code);
    }
  } else {
    // decrement the ref_count and update in DB
    val_vtun->ref_count--;
    DbSubOp dbop_update = {kOpNotRead, kOpMatchNone, kOpInOutNone};

    result_code = vtun_momgr->UpdateConfigDB(vtun_ckv, UPLL_DT_CANDIDATE,
        UNC_OP_UPDATE, dmi, &dbop_update, config_mode, vtn_name, CONVERTTBL);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("converted vtunnel table ref count update failed:%d",
          result_code);
    }
  }
  return result_code;
}


/** This function constructs convert vbr ckv and invokes VbrMoMgr convert
 * function to delete converted vbr created for the given vtn, unified vbridge.
 */
upll_rc_t VbrPortMapMoMgr::DeleteVbr(
    ConfigKeyVal *ikey, IpcReqRespHeader *req, DalDmlIntf *dmi,
    TcConfigMode config_mode, string vtn_name) {
  UPLL_FUNC_TRACE;

  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *convert_vbr_ckv = NULL;

  // Construct convert vbr ckv
  MoMgrImpl *vbr_momgr = reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>(
        GetMoManager(UNC_KT_VBRIDGE)));

  result_code = vbr_momgr->GetChildConvertConfigKey(convert_vbr_ckv, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    return result_code;
  }

  // Invoke VbrMoMgr convert function
  result_code = vbr_momgr->ConvertVbr(dmi, req, convert_vbr_ckv, config_mode,
      vtn_name, UNC_OP_DELETE);
  DELETE_IF_NOT_NULL(convert_vbr_ckv);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("converted vbr deletion failed:%d", result_code);
    return result_code;
  }
  // unset the rename flag if exists
  uint8_t ikey_flag = 0;
  uint8_t reset_flag = 0;
  reset_flag = BOUNDARY_VBRPORTMAP_FLAG | USER_VBRPORTMAP_FLAG;
  GET_USER_DATA_FLAGS(ikey, ikey_flag);
  ikey_flag &= reset_flag;
  SET_USER_DATA_FLAGS(ikey, ikey_flag);
  return result_code;
}

/** This function constructs convert vlink ckv and invokes VlinkMoMgr convert
 * function to delete the converted vlink. If the vbr_portmap flag is true, then
 * deletes the entry based on controller and domain otherwise deletes all entry
 * exists for given vtn and unified vbridge name.
 */
upll_rc_t VbrPortMapMoMgr::DeleteVlink(
    ConfigKeyVal *ikey, DalDmlIntf *dmi, bool vbr_portmap_flag,
    TcConfigMode config_mode, string vtn_name) {
  UPLL_FUNC_TRACE;

  upll_rc_t result_code = UPLL_RC_SUCCESS;

  // construct convert vlink ckv
  // fill the key structure
  ConfigKeyVal *vlink_ckv = NULL;

  MoMgrImpl *vlink_momgr = reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>
      (GetMoManager(UNC_KT_VLINK)));

  result_code = vlink_momgr->GetChildConvertConfigKey(vlink_ckv, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetChildConvertConfigKey failed:%d", result_code);
    return result_code;
  }

  // invoke VlinkMoMgr convert function
  result_code = vlink_momgr->DeleteConvertVlink(vlink_ckv, vbr_portmap_flag,
                                                dmi, config_mode, vtn_name);

  DELETE_IF_NOT_NULL(vlink_ckv);

  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("convert vlink deletion failed:%d", result_code);
    return result_code;
  }
  return result_code;
}

upll_rc_t VbrPortMapMoMgr::DeleteVtunnelIf(ConfigKeyVal *ikey,
    DalDmlIntf *dmi, bool vbr_portmap_flag, TcConfigMode config_mode,
    string vtn_name) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  // frame the convert vtunnel if ConfigKeyVal
  ConfigKeyVal *vtun_if_ckv = NULL;

  MoMgrImpl *vtun_if_momgr = reinterpret_cast<MoMgrImpl *>
    (const_cast<MoManager *>(GetMoManager(UNC_KT_VTUNNEL_IF)));
  result_code = vtun_if_momgr->GetChildConvertConfigKey(vtun_if_ckv, ikey);

  // copy the unified vbr name in val structure
  val_convert_vtunnel_if *val_vtun_if =
    ConfigKeyVal::Malloc<val_convert_vtunnel_if>();
  uuu::upll_strncpy(val_vtun_if->un_vbr_name, static_cast<key_vbr_portmap*>(
        ikey->get_key())->vbr_key.vbridge_name,
      kMaxLenVnodeName+1);
  val_vtun_if->valid[UPLL_IDX_UN_VBR_NAME_CONV_VTNL_IF] = UNC_VF_VALID;
  if (vbr_portmap_flag) {
    controller_domain ctrlr_dom = {NULL, NULL};
    GET_USER_DATA_CTRLR_DOMAIN(ikey, ctrlr_dom);
    //  need to delete based on vtn, unified vbrdge name, ctrlr, domain
    uuu::upll_strncpy(val_vtun_if->rem_ctrlr_name, ctrlr_dom.ctrlr,
        kMaxLenCtrlrId+1);
    uuu::upll_strncpy(val_vtun_if->rem_domain_id, ctrlr_dom.domain,
        kMaxLenDomainId+1);
    val_vtun_if->valid[UPLL_IDX_RCTRLR_NAME_CONV_VTNL_IF] = UNC_VF_VALID;
    val_vtun_if->valid[UPLL_IDX_RDOMAIN_ID_CONV_VTNL_IF] = UNC_VF_VALID;
  }
  vtun_if_ckv->AppendCfgVal(IpctSt::kIpcStValConvertVtunnelIf, val_vtun_if);

  // delete vtunnel if entry
  result_code = vtun_if_momgr->ConvertVtunnelIf(vtun_if_ckv, UNC_OP_DELETE,
      config_mode, vtn_name, dmi);
  DELETE_IF_NOT_NULL(vtun_if_ckv);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("converted vtunnel if deletion failed:%d", result_code);
  }
  return result_code;
}

upll_rc_t VbrPortMapMoMgr::DeleteVbrIf(ConfigKeyVal *ikey, DalDmlIntf *dmi,
    bool vbr_portmap_flag, TcConfigMode config_mode,
    string vtn_name) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  // frame the convert vbr if ConfigKeyVal
  ConfigKeyVal *vbr_if_ckv = NULL;
  MoMgrImpl *vbr_if_momgr = reinterpret_cast<MoMgrImpl *>(
     const_cast<MoManager *>(GetMoManager(UNC_KT_VBR_IF)));

  result_code = vbr_if_momgr->GetChildConvertConfigKey(vbr_if_ckv, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetChildConvertConfigKey failed:%d", result_code);
    return result_code;
  }

  // delete all vbr if entry exists for given vtn and unified vbridge
  // match ctrlr/domain during delete if vbr_portmap_flag is true
  result_code = vbr_if_momgr->ConvertVbrIf(dmi, vbr_portmap_flag, vbr_if_ckv,
                                       config_mode, vtn_name, UNC_OP_DELETE);
  DELETE_IF_NOT_NULL(vbr_if_ckv);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("converted vbr if deletion failed:%d", result_code);
  }
  return result_code;
}

/** This function
 * 1. gets the vtunnel record exists for the given vtn
 * 2. sets the vtunnel controller, domain in vtun_ckv key_user_data
 */
upll_rc_t VbrPortMapMoMgr::GetVtunnelData(ConfigKeyVal *ikey, DalDmlIntf *dmi,
    ConfigKeyVal *&vtun_ckv) {
  UPLL_FUNC_TRACE;

  upll_rc_t result_code = UPLL_RC_SUCCESS;

  DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutCtrlr|kOpInOutDomain };

  MoMgrImpl *vtun_momgr = reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>(
        GetMoManager(UNC_KT_VTUNNEL)));
  result_code = vtun_momgr->GetChildConvertConfigKey(vtun_ckv, ikey);

  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetChildConvertConfigKey failed:%d", result_code);
    return result_code;
  }

  result_code = vtun_momgr->ReadConfigDB(vtun_ckv, UPLL_DT_CANDIDATE,
      UNC_OP_READ, dbop, dmi, CONVERTTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    DELETE_IF_NOT_NULL(vtun_ckv);
    if (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE)  {
      UPLL_LOG_DEBUG("Read on vtunnel table failed:%d", result_code);
    }
    return result_code;
  }
  return result_code;
}

upll_rc_t VbrPortMapMoMgr::ValidateCapability(IpcReqRespHeader *req,
    ConfigKeyVal *ikey,
    const char *ctrlr_name) {
  UPLL_FUNC_TRACE;
  upll_rc_t ret_val = UPLL_RC_ERR_GENERIC;

  if (!ikey || !req) {
    UPLL_LOG_TRACE("ConfigKeyVal / IpcReqRespHeader is Null");
    return UPLL_RC_ERR_GENERIC;
  }

  bool result_code = false;
  uint32_t max_attrs = 0;
  uint32_t max_instance_count = 0;
  const uint8_t *attrs;

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

  val_vbr_portmap_t *ikey_val = NULL;
  // get the val structure based on st_num
  ret_val = GetValVbrPortMap(ikey, &ikey_val);
  if (ret_val != UPLL_RC_SUCCESS) {
    return ret_val;
  }

  if (ikey_val) {
    if (max_attrs > 0) {
      ret_val = ValVbrPortMapAttributeSupportCheck(ikey_val, attrs,
          req->operation);
      return ret_val;
    } else {
      UPLL_LOG_TRACE("Attribute list is empty for operation %d",
          req->operation);
      return UPLL_RC_ERR_GENERIC;
    }
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrPortMapMoMgr::ValVbrPortMapAttributeSupportCheck(
    val_vbr_portmap *vbrpm_val, const uint8_t *attrs,
    unc_keytype_operation_t operation) {
  UPLL_FUNC_TRACE;

  if ((vbrpm_val->valid[UPLL_IDX_DOMAIN_ID_VBRPM] == UNC_VF_VALID) ||
      (vbrpm_val->valid[UPLL_IDX_DOMAIN_ID_VBRPM] == UNC_VF_VALID_NO_VALUE)) {
    if (attrs[unc::capa::vbr_portmap::kCapDomainId] == 0) {
      vbrpm_val->valid[UPLL_IDX_DOMAIN_ID_VBRPM] = UNC_VF_INVALID;
      if (operation == UNC_OP_CREATE || operation == UNC_OP_UPDATE) {
        UPLL_LOG_INFO("Domain id attr is not supported by ctrlr ");
        return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
      }
    }
  }
  if ((vbrpm_val->valid[UPLL_IDX_LOGICAL_PORT_ID_VBRPM] == UNC_VF_VALID) ||
      (vbrpm_val->valid[UPLL_IDX_LOGICAL_PORT_ID_VBRPM] ==
       UNC_VF_VALID_NO_VALUE)) {
    if (attrs[unc::capa::vbr_portmap::kCapLogicalPortId] == 0) {
      vbrpm_val->valid[UPLL_IDX_LOGICAL_PORT_ID_VBRPM] = UNC_VF_INVALID;
      if (operation == UNC_OP_CREATE || operation == UNC_OP_UPDATE) {
        UPLL_LOG_INFO("Logical port id attr is not supported by ctrlr ");
        return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
      }
    }
  }
  if ((vbrpm_val->valid[UPLL_IDX_LABEL_TYPE_VBRPM] == UNC_VF_VALID) ||
      (vbrpm_val->valid[UPLL_IDX_LABEL_TYPE_VBRPM] == UNC_VF_VALID_NO_VALUE)) {
    if (attrs[unc::capa::vbr_portmap::kCapLabelType] == 0) {
      vbrpm_val->valid[UPLL_IDX_LABEL_TYPE_VBRPM] =
        UNC_VF_INVALID;
      if (operation == UNC_OP_CREATE || operation == UNC_OP_UPDATE) {
        UPLL_LOG_INFO("label type is not supported by ctrlr ");
        return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
      }
    }
  }
  if ((vbrpm_val->valid[UPLL_IDX_LABEL_VBRPM] == UNC_VF_VALID) ||
      (vbrpm_val->valid[UPLL_IDX_LABEL_VBRPM]
       == UNC_VF_VALID_NO_VALUE)) {
    if (attrs[unc::capa::vbr_portmap::kCapLabel] == 0) {
      vbrpm_val->valid[UPLL_IDX_LABEL_VBRPM] = UNC_VF_INVALID;
      if (operation == UNC_OP_CREATE || operation == UNC_OP_UPDATE) {
        UPLL_LOG_INFO("label attr is not supported by ctrlr ");
        return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
      }
    }
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrPortMapMoMgr::ValidateMessage(IpcReqRespHeader *req,
    ConfigKeyVal *ikey) {
  UPLL_FUNC_TRACE;
  if (!ikey || !req || !(ikey->get_key())) {
    UPLL_LOG_TRACE("ConfigKeyVal / IpcReqRespHeader is Null");
    return UPLL_RC_ERR_GENERIC;
  }
  upll_rc_t ret_val = UPLL_RC_SUCCESS;

  if (req->option1 != UNC_OPT1_NORMAL) {
    UPLL_LOG_DEBUG("Error: option1 is not NORMAL");
    return UPLL_RC_ERR_INVALID_OPTION1;
  }
  if (req->option2 != UNC_OPT2_NONE) {
    UPLL_LOG_DEBUG("Error: option2 is not NONE");
    return UPLL_RC_ERR_INVALID_OPTION2;
  }
  if (ikey->get_st_num() != IpctSt::kIpcStKeyVbrPortMap) {
    UPLL_LOG_DEBUG("Invalid key structure received. received struct - %d",
        (ikey->get_st_num()));
    return UPLL_RC_ERR_BAD_REQUEST;
  }
  if (ikey->get_key_type() != UNC_KT_VBR_PORTMAP) {
    UPLL_LOG_DEBUG("Invalid keytype(%d) received.", ikey->get_key_type());
    return UPLL_RC_ERR_BAD_REQUEST;
  }

  // Validate key structure
  key_vbr_portmap *vbr_pm_key = reinterpret_cast<
    key_vbr_portmap *> (ikey->get_key());

  val_vbr_portmap *vbrpm_val = NULL;
  ConfigVal *cfg_val = ikey->get_cfg_val();

  if (req->operation == UNC_OP_CREATE &&
      req->datatype == UPLL_DT_IMPORT && cfg_val &&
      reinterpret_cast<pfcdrv_val_vbr_portmap*>(GetVal(ikey)) &&
      (reinterpret_cast<pfcdrv_val_vbr_portmap*>(cfg_val->get_val())->
       vbrpm.valid[UPLL_IDX_LABEL_TYPE_VBRPM] == UNC_VF_VALID) &&
      (reinterpret_cast<pfcdrv_val_vbr_portmap*>(
         cfg_val->get_val())->vbrpm.label_type == UPLL_LABEL_TYPE_GW_VLAN)) {
    ret_val = ValidateKey(reinterpret_cast<char *>(
            vbr_pm_key->vbr_key.vtn_key.vtn_name),
        kMinLenVtnName, kMaxLenVtnName);
    if (ret_val != UPLL_RC_SUCCESS) {
      UPLL_LOG_TRACE("Vtn name(%s) syntax check failed.",
                     vbr_pm_key->vbr_key.vtn_key.vtn_name);
      return ret_val;
    }
  } else {
    ret_val = ValidateVbrPortMapKey(vbr_pm_key, req->operation);
    if (ret_val != UPLL_RC_SUCCESS) {
      return ret_val;
    }
  }

  // validate value structure

  // validate st_num and get val structure
  if (cfg_val) {
    if (cfg_val->get_st_num() == IpctSt::kIpcStValVbrPortMap) {
      vbrpm_val =
        static_cast<val_vbr_portmap *>(cfg_val->get_val());
    } else if (cfg_val->get_st_num() == IpctSt::kIpcStPfcdrvValVbrPortMap) {
      vbrpm_val =
        &(static_cast<pfcdrv_val_vbr_portmap *>(cfg_val->get_val())->vbrpm);
    } else {
      UPLL_LOG_DEBUG("Invalid val structure received.received struct - %d",
          cfg_val->get_st_num());
      return UPLL_RC_ERR_BAD_REQUEST;
    }
  }
  if (req->operation == UNC_OP_CREATE ||  req->operation == UNC_OP_UPDATE) {
    if (req->datatype == UPLL_DT_CANDIDATE || req->datatype == UPLL_DT_IMPORT) {
      if (!cfg_val)  {
        UPLL_LOG_DEBUG("Value structure is mandatory");
        return UPLL_RC_ERR_BAD_REQUEST;
      }
      if (!vbrpm_val) {
        UPLL_LOG_DEBUG("val_vbr_portmap struct is mandatory for Create/update "
            "operation");
        return UPLL_RC_ERR_BAD_REQUEST;
      }
      if ((req->datatype == UPLL_DT_IMPORT) && (vbrpm_val->valid[
        UPLL_IDX_LOGICAL_PORT_ID_VBRPM] != UNC_VF_VALID)) {
        UPLL_LOG_TRACE("Logical port id is mandatory for import-create");
        return UPLL_RC_ERR_CFG_SEMANTIC;
      }
      ret_val = ValidateVbrPortMapValue(vbrpm_val, req->operation);
      if (ret_val != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("syntax check failure for val_vbr_portmap value "
            "structure");
        return UPLL_RC_ERR_CFG_SYNTAX;
      }
      if (req->datatype == UPLL_DT_IMPORT) {
        if (vbrpm_val->valid[UPLL_IDX_LABEL_TYPE_VBRPM] == UNC_VF_VALID) {
          if (vbrpm_val->label_type != UPLL_LABEL_TYPE_VLAN &&
              vbrpm_val->label_type != UPLL_LABEL_TYPE_GW_VLAN) {
            return UPLL_RC_ERR_CFG_SYNTAX;
          }
        }
      } else if (req->datatype == UPLL_DT_CANDIDATE) {
        if (vbrpm_val->valid[UPLL_IDX_LABEL_TYPE_VBRPM] == UNC_VF_VALID) {
          if (vbrpm_val->label_type != UPLL_LABEL_TYPE_VLAN) {
            return UPLL_RC_ERR_CFG_SYNTAX;
          }
        }
      }

      return UPLL_RC_SUCCESS;
    } else {
      UPLL_LOG_DEBUG("Unsupported data type - (%d)", req->datatype);
      return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT;
    }
  } else if (req->operation == UNC_OP_READ ||
      req->operation == UNC_OP_READ_SIBLING ||
      req->operation == UNC_OP_READ_SIBLING_BEGIN ||
      req->operation == UNC_OP_READ_SIBLING_COUNT) {
    if (req->datatype == UPLL_DT_CANDIDATE || req->datatype == UPLL_DT_RUNNING
        || req->datatype == UPLL_DT_STARTUP || req->datatype == UPLL_DT_STATE) {
      if (cfg_val == NULL || vbrpm_val == NULL) {
        UPLL_LOG_TRACE("val structure is optional");
        return UPLL_RC_SUCCESS;
      }
      ret_val = ValidateVbrPortMapValue(vbrpm_val, req->operation);
      if (ret_val != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("syntax check failure for val_vbr_portmap value"
            " structure");
        return UPLL_RC_ERR_CFG_SYNTAX;
      }
      return UPLL_RC_SUCCESS;
    } else {
      UPLL_LOG_DEBUG("Unsupported data type - (%d)", req->datatype);
      return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT;
    }
  } else if (req->operation == UNC_OP_READ_NEXT ||
      req->operation == UNC_OP_READ_BULK) {
    if (req->datatype == UPLL_DT_CANDIDATE || req->datatype == UPLL_DT_RUNNING
        || req->datatype == UPLL_DT_STARTUP) {
      UPLL_LOG_TRACE("Value structure is none for operation type:%d",
          req->operation);
      return UPLL_RC_SUCCESS;
    } else {
      UPLL_LOG_DEBUG("Unsupported data type - (%d)", req->datatype);
      return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT;
    }
  } else if (req->operation == UNC_OP_DELETE) {
    if (req->datatype == UPLL_DT_CANDIDATE) {
      UPLL_LOG_TRACE("Value structure is none for operation type:%d",
          req->operation);
      return UPLL_RC_SUCCESS;
    } else {
      UPLL_LOG_TRACE("Unsupported data type - (%d)", req->datatype);
      return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT;
    }
  }
  UPLL_LOG_TRACE("Unsupported operation - (%d)", req->operation);
  return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT;
}

upll_rc_t VbrPortMapMoMgr::ValidateVbrPortMapKey(key_vbr_portmap_t *vbr_pm_key,
    unc_keytype_operation_t operation) {
  UPLL_FUNC_TRACE;
  upll_rc_t ret_val = UPLL_RC_SUCCESS;
  ret_val = ValidateKey(reinterpret_cast<char *>(
        vbr_pm_key->vbr_key.vtn_key.vtn_name),
      kMinLenVtnName, kMaxLenVtnName);
  if (ret_val != UPLL_RC_SUCCESS) {
    UPLL_LOG_TRACE("Vtn name(%s) syntax check failed.",
        vbr_pm_key->vbr_key.vtn_key.vtn_name);
    return ret_val;
  }
  ret_val = ValidateKey(reinterpret_cast<char *>(
        vbr_pm_key->vbr_key.vbridge_name),
      kMinLenVnodeName, kMaxLenVnodeName);
  if (ret_val != UPLL_RC_SUCCESS) {
    UPLL_LOG_TRACE("Vbridge name(%s) syntax check failed.",
        vbr_pm_key->vbr_key.vbridge_name);
    return ret_val;
  }

  if ((operation != UNC_OP_READ_SIBLING_COUNT) &&
      (operation != UNC_OP_READ_SIBLING_BEGIN)) {
    ret_val = ValidateKey(reinterpret_cast<char *>(
          vbr_pm_key->portmap_id), kMinLenVbrPortMapId, kMaxLenVbrPortMapId);
    if (ret_val != UPLL_RC_SUCCESS) {
      UPLL_LOG_TRACE("Vbridge portmap id(%s) syntax check failed.",
          vbr_pm_key->portmap_id);
      return ret_val;
    }
  } else {
    UPLL_LOG_TRACE("Operation is %d", operation);
    StringReset(vbr_pm_key->portmap_id);
  }
  return ret_val;
}

upll_rc_t VbrPortMapMoMgr::ValidateVbrPortMapValue(val_vbr_portmap *vbr_pm_val,
    unc_keytype_operation_t operation) {
  UPLL_FUNC_TRACE;
  bool ret_val = false;

  if (operation == UNC_OP_CREATE) {
    if (!((vbr_pm_val->valid[UPLL_IDX_CONTROLLER_ID_VBRPM] == UNC_VF_VALID) ||
          (vbr_pm_val->valid[UPLL_IDX_DOMAIN_ID_VBRPM] == UNC_VF_VALID))) {
      UPLL_LOG_DEBUG("Mandatory val structure param is missing for CREATE");
      return UPLL_RC_ERR_BAD_REQUEST;
    }
  }
  if ((operation == UNC_OP_CREATE) || (operation == UNC_OP_UPDATE)) {
    if ((vbr_pm_val->valid[UPLL_IDX_LOGICAL_PORT_ID_VBRPM] !=
          vbr_pm_val->valid[UPLL_IDX_LABEL_TYPE_VBRPM]) ||
        (vbr_pm_val->valid[UPLL_IDX_LOGICAL_PORT_ID_VBRPM] !=
         vbr_pm_val->valid[UPLL_IDX_LABEL_VBRPM])) {
      UPLL_LOG_DEBUG("logical_port_id, label_type and label valid flags"
          " are not same");
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  }

  // Attribute syntax validation
  for (unsigned int valid_index = 0;
      valid_index < sizeof(vbr_pm_val->valid) / sizeof(vbr_pm_val->valid[0]);
      valid_index++) {
    if (vbr_pm_val->valid[valid_index] == UNC_VF_VALID) {
      switch (valid_index) {
        case UPLL_IDX_CONTROLLER_ID_VBRPM:
          ret_val = ValidateString(vbr_pm_val->controller_id,
              kMinLenCtrlrId, kMaxLenCtrlrId);
          break;
        case UPLL_IDX_DOMAIN_ID_VBRPM:
          ret_val = ValidateDefaultStr(vbr_pm_val->domain_id,
              kMinLenDomainId, kMaxLenDomainId);
          break;
        case UPLL_IDX_LOGICAL_PORT_ID_VBRPM:
          ret_val = ValidateLogicalPortId(
              reinterpret_cast<char *>(vbr_pm_val->logical_port_id),
              kMinLenLogicalPortId, kMaxLenLogicalPortId);
          break;
        case UPLL_IDX_LABEL_TYPE_VBRPM:
           ret_val = true;
          break;
        case UPLL_IDX_LABEL_VBRPM:
          // label supports no_vlan_id(0xffff), any_vlan_id(0xfffe) and the
          // vlan_id from the range kMinVlanId to kMaxVlanId
          if ((vbr_pm_val->label != NO_VLAN_ID) && (vbr_pm_val->label !=
             ANY_VLAN_ID) &&
              !ValidateNumericRange(vbr_pm_val->label,
                (uint32_t) kMinVlanId, (uint32_t) kMaxVlanId,
                true, true)) {
            ret_val = false;
          } else {
            ret_val = true;
          }
          break;
      }
      if (!ret_val) {
        return UPLL_RC_ERR_CFG_SYNTAX;
      }
    }
  }

  // Resets
  if ((operation == UNC_OP_CREATE) || (operation == UNC_OP_UPDATE)) {
    for (unsigned int valid_index = 0;
        valid_index < sizeof(vbr_pm_val->valid) / sizeof(vbr_pm_val->valid[0]);
        valid_index++) {
      uint8_t flag = vbr_pm_val->valid[valid_index];
      if (flag != UNC_VF_INVALID && flag != UNC_VF_VALID_NO_VALUE)
        continue;
      switch (valid_index) {
        case UPLL_IDX_CONTROLLER_ID_VBRPM:
          StringReset(vbr_pm_val->controller_id);
          break;
        case UPLL_IDX_DOMAIN_ID_VBRPM:
          StringReset(vbr_pm_val->domain_id);
          break;
        case UPLL_IDX_LOGICAL_PORT_ID_VBRPM:
          StringReset(vbr_pm_val->logical_port_id);
          break;
        case UPLL_IDX_LABEL_TYPE_VBRPM:
          vbr_pm_val->label_type = 0;
          break;
        case UPLL_IDX_LABEL_VBRPM:
          vbr_pm_val->label = 0;
          break;
        default:
          UPLL_LOG_TRACE("Never here");
          break;
      }
    }
  }
  return UPLL_RC_SUCCESS;
}

bool VbrPortMapMoMgr::FilterAttributes(
    void *&val1,
    void *val2,
    bool copy_to_running,
    unc_keytype_operation_t op) {
  UPLL_FUNC_TRACE;
  val_vbr_portmap_t *val_vbr_pm1 = &(
      reinterpret_cast<pfcdrv_val_vbr_portmap_t *>(val1)->vbrpm);
  val_vbr_portmap_t *val_vbr_pm2 = &(reinterpret_cast<
                                     pfcdrv_val_vbr_portmap_t *>(val2)->vbrpm);
  if (op == UNC_OP_UPDATE) {
    val_vbr_pm1->valid[UPLL_IDX_CONTROLLER_ID_VBRPM] = UNC_VF_INVALID;
    val_vbr_pm1->valid[UPLL_IDX_DOMAIN_ID_VBRPM] = UNC_VF_INVALID;
    val_vbr_pm2->valid[UPLL_IDX_CONTROLLER_ID_VBRPM] = UNC_VF_INVALID;
    val_vbr_pm2->valid[UPLL_IDX_DOMAIN_ID_VBRPM] = UNC_VF_INVALID;
  }
  bool reset_ctrlr_dom = false;
  if (op == UNC_OP_UPDATE) {
    if ((val_vbr_pm1->valid[UPLL_IDX_LOGICAL_PORT_ID_VBRPM] == UNC_VF_INVALID)
        &&
        (val_vbr_pm2->valid[UPLL_IDX_LOGICAL_PORT_ID_VBRPM] == UNC_VF_VALID)) {
      // going to send UPDATE as CREATE, so set valid to controller, domain
      reset_ctrlr_dom = true;
    }
  }

  // val1 contains candidate configuration
  // val2 contains running configuration
  if (op != UNC_OP_CREATE) {
    bool ret_code = CompareValidValue(val1, val2, copy_to_running);
    if (reset_ctrlr_dom) {
      val_vbr_portmap_t *val_vbr_pm1 = &(
          reinterpret_cast<pfcdrv_val_vbr_portmap_t *>(val1)->vbrpm);
      val_vbr_pm1->valid[UPLL_IDX_CONTROLLER_ID_VBRPM] = UNC_VF_VALID;
      val_vbr_pm1->valid[UPLL_IDX_DOMAIN_ID_VBRPM] = UNC_VF_VALID;
    }
    return ret_code;
  }
  return false;
}

bool VbrPortMapMoMgr::CompareValidValue(void *&val1, void *val2,
                                        bool copy_to_running) {
  UPLL_FUNC_TRACE;
  bool invalid_attr = true;

  pfcdrv_val_vbr_portmap_t *drv_val_vbrpm1 =
      reinterpret_cast<pfcdrv_val_vbr_portmap_t *>(val1);
  val_vbr_portmap_t *val_vbrpm1 = &(drv_val_vbrpm1->vbrpm);

  pfcdrv_val_vbr_portmap_t *drv_val_vbrpm2 =
      reinterpret_cast<pfcdrv_val_vbr_portmap_t *>(val2);
  val_vbr_portmap_t *val_vbrpm2 = &(drv_val_vbrpm2->vbrpm);

  for (unsigned int loop = 0;
       loop < sizeof(val_vbrpm1->valid) / sizeof(uint8_t); ++loop) {
    if ((UNC_VF_INVALID == val_vbrpm1->valid[loop]) &&
        (UNC_VF_VALID == val_vbrpm2->valid[loop]))
      val_vbrpm1->valid[loop] = UNC_VF_VALID_NO_VALUE;
  }

  if (copy_to_running) {
    // TODO(U17) set ref_count valid flag should be VALID always
    if ((drv_val_vbrpm1->valid[PFCDRV_IDX_VBRPM_BDRY_REF_COUNT] ==
         UNC_VF_INVALID) &&
        (drv_val_vbrpm2->valid[PFCDRV_IDX_VBRPM_BDRY_REF_COUNT] ==
         UNC_VF_VALID)) {
      drv_val_vbrpm1->valid[PFCDRV_IDX_VBRPM_BDRY_REF_COUNT] =
          UNC_VF_VALID_NO_VALUE;
    }
    if ((drv_val_vbrpm1->valid[PFCDRV_IDX_VBRPM_BDRY_REF_COUNT] ==
         UNC_VF_VALID) &&
        (drv_val_vbrpm2->valid[PFCDRV_IDX_VBRPM_BDRY_REF_COUNT] ==
         UNC_VF_VALID)) {
      if (drv_val_vbrpm1->bdry_ref_count  == drv_val_vbrpm2->bdry_ref_count) {
        drv_val_vbrpm1->valid[PFCDRV_IDX_VBRPM_BDRY_REF_COUNT] = UNC_VF_INVALID;
      }
    }
    if ((val_vbrpm1->valid[UPLL_IDX_LABEL_TYPE_VBRPM] == UNC_VF_VALID) &&
        (val_vbrpm2->valid[UPLL_IDX_LABEL_TYPE_VBRPM] == UNC_VF_VALID)) {
      if (val_vbrpm1->label_type == val_vbrpm2->label_type) {
        val_vbrpm1->valid[UPLL_IDX_LABEL_TYPE_VBRPM] = UNC_VF_INVALID;
      }
    }
    if (drv_val_vbrpm1->valid[PFCDRV_IDX_VBRPM_BDRY_REF_COUNT] !=
        UNC_VF_INVALID) {
      invalid_attr = false;
    }
  }
  if ((val_vbrpm1->valid[UPLL_IDX_CONTROLLER_ID_VBRPM] == UNC_VF_VALID) &&
      (val_vbrpm2->valid[UPLL_IDX_CONTROLLER_ID_VBRPM] == UNC_VF_VALID)) {
    if (!uuu::upll_strncmp(val_vbrpm1->controller_id,
                           val_vbrpm2->controller_id, kMaxLenCtrlrId+1)) {
      val_vbrpm1->valid[UPLL_IDX_CONTROLLER_ID_VBRPM] = UNC_VF_INVALID;
    }
  }
  if ((val_vbrpm1->valid[UPLL_IDX_DOMAIN_ID_VBRPM] == UNC_VF_VALID) &&
      (val_vbrpm2->valid[UPLL_IDX_DOMAIN_ID_VBRPM] == UNC_VF_VALID)) {
    if (!uuu::upll_strncmp(val_vbrpm1->domain_id, val_vbrpm2->domain_id,
                           kMaxLenDomainId+1)) {
      val_vbrpm1->valid[UPLL_IDX_DOMAIN_ID_VBRPM] = UNC_VF_INVALID;
    }
  }
  if ((val_vbrpm1->valid[UPLL_IDX_LOGICAL_PORT_ID_VBRPM] == UNC_VF_VALID) &&
      (val_vbrpm2->valid[UPLL_IDX_LOGICAL_PORT_ID_VBRPM] == UNC_VF_VALID)) {
    if (!uuu::upll_strncmp(val_vbrpm1->logical_port_id,
                           val_vbrpm2->logical_port_id,
                           kMaxLenLogicalPortId+1)) {
      val_vbrpm1->valid[UPLL_IDX_LOGICAL_PORT_ID_VBRPM] = UNC_VF_INVALID;
    }
  }
  if ((val_vbrpm1->valid[UPLL_IDX_LABEL_VBRPM] == UNC_VF_VALID) &&
      (val_vbrpm2->valid[UPLL_IDX_LABEL_VBRPM] == UNC_VF_VALID)) {
    if (val_vbrpm1->label == val_vbrpm2->label) {
      val_vbrpm1->valid[UPLL_IDX_LABEL_VBRPM] = UNC_VF_INVALID;
    }
  }
  for (unsigned int loop = 0;
       loop < sizeof(val_vbrpm1->valid) / sizeof(val_vbrpm1->valid[0]);
       ++loop) {
    if (loop == UPLL_IDX_LABEL_TYPE_VBRPM) {
      // exclude the label type
      continue;
    }
    if ((UNC_VF_VALID == (uint8_t) val_vbrpm1->valid[loop]) ||
        (UNC_VF_VALID_NO_VALUE == (uint8_t) val_vbrpm1->valid[loop]))
      invalid_attr = false;
  }
  if ((invalid_attr == false) && (copy_to_running == false) &&
      ((val_vbrpm1->valid[UPLL_IDX_LABEL_VBRPM] == UNC_VF_VALID) ||
       (val_vbrpm1->valid[UPLL_IDX_LOGICAL_PORT_ID_VBRPM] == UNC_VF_VALID))) {
      // if any of the valid flag (label, logical_port_id) is valid then need to
      // pass label, label_type and logical-port-id as VALID to driver
      // this check is true in vtn_gateway_port update-commit
      val_vbrpm1->valid[UPLL_IDX_LABEL_VBRPM] = UNC_VF_VALID;
      val_vbrpm1->valid[UPLL_IDX_LOGICAL_PORT_ID_VBRPM] = UNC_VF_VALID;
  }
  return invalid_attr;
}

upll_rc_t VbrPortMapMoMgr::IsReferenced(ConfigKeyVal *ikey,
    upll_keytype_datatype_t dt_type,
    DalDmlIntf *dmi) {
  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrPortMapMoMgr::AdaptValToDriver(ConfigKeyVal *ck_new,
    ConfigKeyVal *ck_old,
    unc_keytype_operation_t op,
    upll_keytype_datatype_t dt_type,
    unc_key_type_t keytype,
    DalDmlIntf *dmi,
    bool &not_send_to_drv,
    bool audit_update_phase) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  // get the new ckv val structure
  pfcdrv_val_vbr_portmap *vbrpm_new_val =
    reinterpret_cast<pfcdrv_val_vbr_portmap_t *>(GetVal(ck_new));

  pfcdrv_val_vbr_portmap *vbrpm_old_val = NULL;
  if (op == UNC_OP_UPDATE) {
    if (audit_update_phase) {
      // Audit-commit, get the audit cfg val from ck_new in case of
      // update operation
      vbrpm_old_val =
        reinterpret_cast<pfcdrv_val_vbr_portmap_t *>(GetStateVal(ck_new));
    } else {
      // candiate-commit, get the running cfg val from ck_old
      vbrpm_old_val = reinterpret_cast<pfcdrv_val_vbr_portmap_t *>(
          GetVal(ck_old));
    }
    if (NULL == vbrpm_old_val) {
      UPLL_LOG_DEBUG("Didnt get second val structure for update operation");
      return UPLL_RC_ERR_GENERIC;
    }
  }
  switch (op) {
    case UNC_OP_CREATE: {
      // dont send the request to driver, if the VBR_PORTMAP is not having
      // logical_port_id as VALID
      if (vbrpm_new_val->vbrpm.valid[UPLL_IDX_LOGICAL_PORT_ID_VBRPM] !=
          UNC_VF_VALID) {
        not_send_to_drv = true;
        return result_code;
      }
      break;
    }
    case UNC_OP_UPDATE: {
      // send DELETE request to controller in case of logical_port_id unset
      if (vbrpm_new_val->vbrpm.valid[UPLL_IDX_LOGICAL_PORT_ID_VBRPM] ==
            UNC_VF_VALID_NO_VALUE) {
        if (audit_update_phase) {
          /** in case of Audit, keep only audit cfg_val in ck_new and remove the
           *  running cfg_val  as the operation is changed from UPDATE to DELETE
           */
          ConfigVal *tmp = ck_new->get_cfg_val()->get_next_cfg_val();
          ck_new->get_cfg_val()->set_next_cfg_val(NULL);
          ck_new->DeleteCfgVal();
          ck_new->AppendCfgVal(tmp);
        } else {
          uint8_t new_vbrpm_flag = 0;
          GET_USER_DATA_FLAGS(ck_old, new_vbrpm_flag);
          // copy the running cfg_val to candidate
          ck_new->SetCfgVal(ck_old->get_cfg_val());
          ck_old->set_cfg_val(NULL);
          SET_USER_DATA_FLAGS(ck_new, new_vbrpm_flag);
        }
        vbrpm_new_val = reinterpret_cast<pfcdrv_val_vbr_portmap_t *>(
            GetVal(ck_new));
      }
      break;
    }
    case UNC_OP_DELETE: {
      if (audit_update_phase) {
        // in case of Audit delete, send delete request if vbr_portmap created
        // with or without logical_port_id in controller
        break;
      }
      // In candidate-commit, if logical_port_id is not valid in running, dont
      //  send the DELETE request to controller
      if (vbrpm_new_val->vbrpm.valid[UPLL_IDX_LOGICAL_PORT_ID_VBRPM] !=
          UNC_VF_VALID) {
        not_send_to_drv = true;
        return result_code;
      }
      break;
    }
    default:
      break;
  }

  // if any of the val_vbr_portmap attribute is not INVALID then set
  // PFCDRV_IDX_VAL_VBR_PORTMAP to VALID
  if (vbrpm_new_val) {
    // dont share ref count to driver
    vbrpm_new_val->valid[PFCDRV_IDX_VBRPM_BDRY_REF_COUNT] = UNC_VF_INVALID;

    if ((vbrpm_new_val->vbrpm.valid[UPLL_IDX_CONTROLLER_ID_VBRPM] !=
         UNC_VF_INVALID)
        || (vbrpm_new_val->vbrpm.valid[UPLL_IDX_DOMAIN_ID_VBRPM] !=
            UNC_VF_INVALID)
        || (vbrpm_new_val->vbrpm.valid[UPLL_IDX_LOGICAL_PORT_ID_VBRPM] !=
            UNC_VF_INVALID) ||
        (vbrpm_new_val->vbrpm.valid[UPLL_IDX_LABEL_TYPE_VBRPM] !=
         UNC_VF_INVALID) ||
        (vbrpm_new_val->vbrpm.valid[UPLL_IDX_LABEL_VBRPM] != UNC_VF_INVALID)) {
      vbrpm_new_val->valid[PFCDRV_IDX_VAL_VBR_PORTMAP] = UNC_VF_VALID;
    } else {
      vbrpm_new_val->valid[PFCDRV_IDX_VAL_VBR_PORTMAP] = UNC_VF_INVALID;
    }
  }
  return result_code;
}

upll_rc_t VbrPortMapMoMgr::TranslateVbrPortMapError(
    ConfigKeyVal **err_ckv, ConfigKeyVal *ckv_running,
    ConfigKeyVal *resp_ckv, DalDmlIntf *dmi,
    upll_keytype_datatype_t datatype) {
  UPLL_FUNC_TRACE;
  upll_rc_t     result_code         = UPLL_RC_SUCCESS;
  uint8_t       bound_vbrpm_vlink = 0;

  if (resp_ckv && (resp_ckv->get_key_type() == UNC_KT_VBR_PORTMAP)) {
    if (!(resp_ckv->get_key()) || !(resp_ckv->get_cfg_val()) ||
        !(GetVal(resp_ckv))) {
      UPLL_LOG_DEBUG("Input key is NULL");
      return UPLL_RC_ERR_GENERIC;
    }
    pfcdrv_val_vbr_portmap_t *val_pm =
                reinterpret_cast<pfcdrv_val_vbr_portmap_t *>(GetVal(resp_ckv));
    if ((val_pm->vbrpm.valid[UPLL_IDX_LABEL_TYPE_VBRPM] == UNC_VF_VALID) &&
        (val_pm->vbrpm.label_type == UPLL_LABEL_TYPE_GW_VLAN)) {
      VbrMoMgr *mgr = reinterpret_cast<VbrMoMgr *>(const_cast<MoManager *>(
                       GetMoManager(UNC_KT_VBRIDGE)));
      if (!mgr) {
        UPLL_LOG_ERROR("Unable to get KT_VBR_PORTMAP instace");
        return UPLL_RC_ERR_GENERIC;
      }
      return  mgr->TranslateVbrPortmapToVbrErr(resp_ckv, err_ckv,
                                               datatype, dmi);
    }
  }
  UPLL_LOG_TRACE("concurrency %s", (ckv_running->ToStrAll()).c_str());
  /* Get duplicate Config key from ckv running*/
  ConfigKeyVal *err_ckv2 = NULL;
  result_code = DupConfigKeyVal(err_ckv2, ckv_running);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_INFO("Error in DupConfigKeyVal");
    return result_code;
  }
  GET_USER_DATA_FLAGS(err_ckv2, bound_vbrpm_vlink);

  UPLL_LOG_DEBUG("Flag of failed VBRPortMap is %d", bound_vbrpm_vlink);

  /* Checks if it is part of only user configured vbrportmap */
  if ((bound_vbrpm_vlink & BOUNDARY_VBRPORTMAP_FLAG) == 0x0) {
    *err_ckv = err_ckv2;
  } else {
    /* Checks it is part of boundary vlink */
    if (bound_vbrpm_vlink & BOUNDARY_VBRPORTMAP_FLAG) {
      VlinkMoMgr *mgr = reinterpret_cast<VlinkMoMgr *>
        (const_cast<MoManager*>(GetMoManager(UNC_KT_VLINK)));
      if (!mgr) {
        UPLL_LOG_TRACE("Invalid mgr");
        DELETE_IF_NOT_NULL(err_ckv2);
        return UPLL_RC_ERR_GENERIC;
      }
      ConfigKeyVal *ck_vlink = NULL;
      result_code = mgr->GetVlinkKeyValFromVbrPortMap(err_ckv2, ck_vlink, dmi,
          datatype);
      if (result_code == UPLL_RC_SUCCESS) {
        UPLL_LOG_TRACE("Vlinks associated to logical_port_id are retrieved");
        if (bound_vbrpm_vlink & USER_VBRPORTMAP_FLAG) {
          result_code = AdaptValToVtnService(err_ckv2, ADAPT_ONE);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_TRACE("AdaptValToVtnService failed result_code %d",
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
        UPLL_LOG_INFO("Failed to map boundary to vlink."
            "result_code:%d", result_code);
        /* For errors, no need to update err_ckv */
        DELETE_IF_NOT_NULL(err_ckv2);
        if (ck_vlink)
          delete ck_vlink;
        return result_code;
      }
    }
  }

  if (*err_ckv) {
    result_code = AdaptValToVtnService(*err_ckv, ADAPT_ONE);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_TRACE("AdaptValToVtnService failed result_code %d",
          result_code);
      return result_code;
    }
  }
  return result_code;
}


upll_rc_t VbrPortMapMoMgr::TxUpdateErrorHandler(ConfigKeyVal *req,
    ConfigKeyVal *ck_main,
    DalDmlIntf *dmi,
    upll_keytype_datatype_t dt_type,
    ConfigKeyVal **err_ckv,
    IpcResponse *ipc_resp) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  controller_domain ctrlr_dom = {NULL, NULL};
  GET_USER_DATA_CTRLR_DOMAIN(ck_main, ctrlr_dom);
  // TODO(SOU) fixed coverity, review
  if (ctrlr_dom.ctrlr == NULL) {
    return UPLL_RC_ERR_GENERIC;
  }
  uint8_t flags = 0;
  ConfigKeyVal *err = NULL;
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
  result_code = TranslateVbrPortMapError(&err, req, ipc_resp->ckv_data, dmi,
                                         dt_type);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Failed to convert vbr_portmap error ckv");
    DELETE_IF_NOT_NULL(ck_main);
    DELETE_IF_NOT_NULL(req);
    DELETE_IF_NOT_NULL(ipc_resp->ckv_data);
    return result_code;
  }
  if (!(ipc_resp->ckv_data) || (!(ipc_resp->ckv_data)->get_key()) ||
      !((ipc_resp->ckv_data)->get_cfg_val()) || !(GetVal(ipc_resp->ckv_data))) {
    DELETE_IF_NOT_NULL(ck_main);
    DELETE_IF_NOT_NULL(req);
    DELETE_IF_NOT_NULL(ipc_resp->ckv_data);
    return UPLL_RC_ERR_GENERIC;
  }
  pfcdrv_val_vbr_portmap_t *val_pm =
       reinterpret_cast<pfcdrv_val_vbr_portmap_t *>(GetVal(ipc_resp->ckv_data));
  if ((val_pm->vbrpm.valid[UPLL_IDX_LABEL_TYPE_VBRPM] == UNC_VF_VALID) &&
      (val_pm->vbrpm.label_type == UPLL_LABEL_TYPE_GW_VLAN)) {
    *err_ckv = err;
    DELETE_IF_NOT_NULL(ck_main);
    DELETE_IF_NOT_NULL(req);
    DELETE_IF_NOT_NULL(ipc_resp->ckv_data);
    return result_code;
  }

  // flags determines whether the vbr_portmap is a user
  //  configured vbr portmap or a boundary vbr portmap
  GET_USER_DATA_FLAGS(req, flags);
  if (flags & BOUNDARY_VBRPORTMAP_FLAG) {
    SET_USER_DATA_CTRLR(err, ctrlr_dom.ctrlr);
    *err_ckv = err;
    DELETE_IF_NOT_NULL(ck_main);
    DELETE_IF_NOT_NULL(req);
    DELETE_IF_NOT_NULL(ipc_resp->ckv_data);
    return result_code;
  }
  DELETE_IF_NOT_NULL(err);

  // Convert Driver response to VTN service response
  result_code = AdaptValToVtnService(ipc_resp->ckv_data, ADAPT_ONE);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("AdaptValToVtnService failed result_code %d",
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

/** This function gets the max_count from UNW_LABEL if label is configured in
 *spd_ckv otherwise gets the default max_count and checks whether the used count
 *exceeds the max_count
 */
upll_rc_t VbrPortMapMoMgr::CheckUsedCountExceedsMaxCount(ConfigKeyVal *spd_ckv,
                                        TcConfigMode config_mode,
                                        DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;

  // check whether used_count exceeds the max_count
  val_unw_spdom_ext *spdom_ext_val = static_cast<val_unw_spdom_ext*>(
      GetVal(spd_ckv));

  uint32_t used_label_count = 0;
  // set the used_label_count
  if (spdom_ext_val->valid[UPLL_IDX_SPINE_USED_LABEL_COUNT_UNWS] ==
      UNC_VF_VALID) {
    used_label_count = spdom_ext_val->used_label_count;
  } else {
    used_label_count = 0;
  }

  uint32_t max_count = kDefaultMaxCount;
  if (spdom_ext_val->val_unw_spine_dom.valid[UPLL_IDX_UNW_LABEL_ID_UNWS]
      == UNC_VF_VALID) {
    ConfigKeyVal *label_ckv = NULL;
    // get the label ckv
    upll_rc_t result_code = GetUnwLabelCkv(spd_ckv, label_ckv, config_mode,
                                           dmi);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Unw label table read failed:%d", result_code);
      return result_code;
    }
    if (static_cast<val_unw_label*>GetVal(
            label_ckv)->valid[UPLL_IDX_MAX_COUNT_UNWL] == UNC_VF_VALID) {
      max_count = static_cast<val_unw_label*>(GetVal(label_ckv))->max_count;
    }
    DELETE_IF_NOT_NULL(label_ckv);
  }
  // compare the max and used count
  if (used_label_count >= max_count) {
    UPLL_LOG_DEBUG("No label available for G-vtnid allocation."
                   "Used label count equals or exceeds the max_count");
    return UPLL_RC_ERR_EXCEEDS_RESOURCE_LIMIT;
  }
  return UPLL_RC_SUCCESS;
}

/** This function allocates the gvtnid and  deletes and recreates the converted
 *  vnode including vtunnel according to the new spine domain id
 */
upll_rc_t VbrPortMapMoMgr::HandleSpineDomainIdChange(
    DalDmlIntf *dmi, IpcReqRespHeader *req,
    ConfigKeyVal *new_spinedom_ckv,
    ConfigKeyVal *old_vtunnel_ckv) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  /*get the config mode to use in updateconfigDB*/
  TcConfigMode config_mode = TC_CONFIG_INVALID;
  std::string config_vtn_name;
  result_code = GetConfigModeInfo(req, config_mode, config_vtn_name);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_TRACE("GetConfigMode failed");
    return result_code;
  }

  // Allocate the gvtnid for the new spine id
  uint32_t gvtnid = 0;
  uint8_t *vtn_id  = static_cast<key_convert_vtunnel*>(
          old_vtunnel_ckv->get_key())->vtn_key.vtn_name;

  result_code = CheckUsedCountExceedsMaxCount(new_spinedom_ckv, config_mode,
                                              dmi);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("CheckUsedCountExceedsMaxCount failed:%d", result_code);
    return result_code;
  }
  result_code = AllocGvtnId(new_spinedom_ckv, vtn_id, gvtnid, dmi,
                 config_mode,  config_vtn_name);
  if (result_code != UPLL_RC_SUCCESS) {
    return result_code;
  }
  // update the spine domain table with the new used label count
  result_code = IncrementUsedLabelCount(new_spinedom_ckv, vtn_id, dmi,
             config_mode, config_vtn_name);
  if (result_code != UPLL_RC_SUCCESS) {
    return result_code;
  }

  // Deallocate the gvtnid used in existing vtunnel
  result_code = DeAllocGvtnId(old_vtunnel_ckv, dmi, UPLL_DT_CANDIDATE,
                              config_mode, config_vtn_name);
  if (result_code != UPLL_RC_SUCCESS)
    return result_code;

  // decrement the used label count from spine domain table based on
  // controller and domain name in vtun_ckv
  result_code = DecrementUsedLabelCount(old_vtunnel_ckv, vtn_id, dmi,
                                       config_mode, config_vtn_name);
  if (result_code != UPLL_RC_SUCCESS)
    return result_code;

  // do read multiple on coverted vlink table
  key_convert_vlink *key_con_vlink = static_cast<key_convert_vlink*>(
      ConfigKeyVal::Malloc(sizeof(key_convert_vlink)));
  uint8_t *vtn_name = static_cast<key_convert_vtunnel*>(
      old_vtunnel_ckv->get_key())->vtn_key.vtn_name;

  uuu::upll_strncpy(key_con_vlink->vbr_key.vtn_key.vtn_name,
      vtn_name, (kMaxLenVtnName+1));

  ConfigKeyVal *vlink_ckv =  new ConfigKeyVal(UNC_KT_VLINK,
      IpctSt::kIpcStKeyConvertVlink, key_con_vlink,
      NULL);

  MoMgrImpl *vlink_momgr = reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>
      (GetMoManager(UNC_KT_VLINK)));

  DbSubOp dbop = { kOpReadMultiple, kOpMatchNone,
    kOpInOutCtrlr| kOpInOutDomain };
  result_code = vlink_momgr->ReadConfigDB(vlink_ckv, UPLL_DT_CANDIDATE,
      UNC_OP_READ, dbop, dmi, CONVERTTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    DELETE_IF_NOT_NULL(vlink_ckv);
    UPLL_LOG_ERROR("vlink table ReadMultiple failed:%d", result_code);
    return result_code;
  }

  // delete the entry from vlink table and vtn_gvtnid table and vtn gateway port
  // table
  result_code = DeleteVlinkAndVtnGatewayPort(old_vtunnel_ckv, dmi, config_mode,
      config_vtn_name);
  if (result_code != UPLL_RC_SUCCESS) {
    DELETE_IF_NOT_NULL(vlink_ckv);
    return result_code;
  }

  // fill the vtunnel ckv and invoke ConvertVtunnel to create the entry in
  // vtunnel table
  ConfigKeyVal * new_vtun_ckv = NULL;
  result_code = CreateVtunnelWithRefCount(old_vtunnel_ckv, new_vtun_ckv, dmi,
      gvtnid, new_spinedom_ckv, config_mode,
      config_vtn_name);
  if (result_code != UPLL_RC_SUCCESS) {
    DELETE_IF_NOT_NULL(vlink_ckv);
    return result_code;
  }

  // create converted vlink based on new vtunnel data
  result_code = CreateVlinkForSpineIdChange(new_vtun_ckv, vlink_ckv, dmi,
      config_mode, config_vtn_name);
  DELETE_IF_NOT_NULL(vlink_ckv);
  DELETE_IF_NOT_NULL(new_vtun_ckv);

  if (result_code != UPLL_RC_SUCCESS)
    return result_code;
  return result_code;
}

/**This function reads the label table based on the spine domain ckv
 * And also checks the label entry exists in Running DB if the config
 * mode is VTN
 */
upll_rc_t VbrPortMapMoMgr::GetUnwLabelCkv(ConfigKeyVal *spinedom_ckv,
    ConfigKeyVal *&label_ckv, TcConfigMode config_mode, DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  // Read the label table and get the max_count
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  // if the config_mode is PCM, read it from RUNNING
  upll_keytype_datatype_t datatype = UPLL_DT_INVALID;
  if (config_mode == TC_CONFIG_VTN) {
    datatype = UPLL_DT_RUNNING;
  } else {
    datatype = UPLL_DT_CANDIDATE;
  }

  MoMgrImpl *unw_label_momgr = reinterpret_cast<MoMgrImpl *>(
      const_cast<MoManager *>(GetMoManager(UNC_KT_UNW_LABEL)));

  if (NULL == unw_label_momgr) {
    UPLL_LOG_ERROR("Unable to get unw_label_momgr instance");
    return UPLL_RC_ERR_GENERIC;
  }

  // fill the unw_label ckv from spine domain ckv
  result_code = unw_label_momgr->GetChildConfigKey(label_ckv, NULL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_INFO("GetChildConfigKey failed:%d", result_code);
    return result_code;
  }
  key_unw_label *unw_label_key = static_cast<key_unw_label*>(
      label_ckv->get_key());

  uuu::upll_strncpy(unw_label_key->unified_nw_key.unified_nw_id,
      static_cast<key_unw_spine_domain*>(
        spinedom_ckv->get_key())->unw_key.unified_nw_id,
      kMaxLenUnwName+1);
  uuu::upll_strncpy(unw_label_key->unw_label_id,
      static_cast<val_unw_spdom_ext*>(
        GetVal(spinedom_ckv))->val_unw_spine_dom.unw_label_id,
      kMaxLenUnwLabelName+1);

  // read unw_label table to get max_count
  DbSubOp dbop = {kOpReadSingle, kOpMatchNone, kOpInOutNone };
  result_code = unw_label_momgr->ReadConfigDB(label_ckv, datatype,
      UNC_OP_READ, dbop, dmi, MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_ERROR("Unw label table read failed:%d", result_code);
    DELETE_IF_NOT_NULL(label_ckv);
    return result_code;
  }

  return result_code;
}

/** This function deletes the converted vlink and vtn gateway port table entries
 * exists for the vtn in old_vtun_ckv
 */
upll_rc_t VbrPortMapMoMgr::DeleteVlinkAndVtnGatewayPort(
    ConfigKeyVal *old_vtun_ckv, DalDmlIntf *dmi, TcConfigMode config_mode,
    string config_vtn_name) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  uint8_t *vtn_name = static_cast<key_convert_vtunnel*>(
      old_vtun_ckv->get_key())->vtn_key.vtn_name;

  // delete all entries in converted vlink table for the given vtn
  key_convert_vlink *key_con_vlink = ConfigKeyVal::Malloc<key_convert_vlink>();

  uuu::upll_strncpy(key_con_vlink->vbr_key.vtn_key.vtn_name,
      vtn_name, (kMaxLenVtnName+1));

  ConfigKeyVal *vlink_ckv =  new ConfigKeyVal(UNC_KT_VLINK,
      IpctSt::kIpcStKeyConvertVlink, key_con_vlink, NULL);

  MoMgrImpl *vlink_momgr = reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>
      (GetMoManager(UNC_KT_VLINK)));
  DbSubOp dbop = { kOpNotRead, kOpMatchNone, kOpInOutNone };
  result_code = vlink_momgr->UpdateConfigDB(vlink_ckv, UPLL_DT_CANDIDATE,
      UNC_OP_DELETE, dmi, &dbop, config_mode, config_vtn_name, CONVERTTBL);
  DELETE_IF_NOT_NULL(vlink_ckv);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_ERROR("vlink table delete based on vtn failed:%d", result_code);
    return result_code;
  }

  // delete all entries in vtn_gateway_port_tbl for given vtn
  key_vtn *vtn_key = ConfigKeyVal::Malloc<key_vtn>();
  uuu::upll_strncpy(vtn_key->vtn_name, vtn_name, (kMaxLenVtnName+1));
  ConfigKeyVal *vtn_gw_port_ckv = new ConfigKeyVal(
      UNC_KT_VTN, IpctSt::kIpcStKeyVtn, vtn_key, NULL);
  MoMgrImpl *vtn_momgr = reinterpret_cast<MoMgrImpl *>(
      const_cast<MoManager *>(GetMoManager(UNC_KT_VTN)));
  result_code = vtn_momgr->UpdateConfigDB(
      vtn_gw_port_ckv, UPLL_DT_CANDIDATE, UNC_OP_DELETE, dmi, &dbop,
      config_mode, config_vtn_name, CONVERTTBL);
  DELETE_IF_NOT_NULL(vtn_gw_port_ckv);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_ERROR("vtn gateway port table delete failed:%d", result_code);
    return result_code;
  }
  return result_code;
}


/* This function takes the vtunnel_name, ref_count from the old vtunnel and
 * generates the new converted vtunnel based on new_spine_dom_ckv controller and
 * domain and new gvtnid
 */
upll_rc_t VbrPortMapMoMgr::CreateVtunnelWithRefCount(
    ConfigKeyVal *old_vtun_ckv, ConfigKeyVal *&new_vtun_ckv,  DalDmlIntf *dmi,
    uint32_t gvtnid, ConfigKeyVal *new_spine_dom_ckv,
    TcConfigMode config_mode, string vtn_name) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  MoMgrImpl *vtun_momgr = reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>(
        GetMoManager(UNC_KT_VTUNNEL)));

  // frame the converted vtunnel ckv
  result_code = vtun_momgr->DupConfigKeyVal(new_vtun_ckv, old_vtun_ckv,
                                            CONVERTTBL);
  if (result_code != UPLL_RC_SUCCESS)
    return result_code;

  // fill val structure
  val_convert_vtunnel *vtunnel_val = static_cast<val_convert_vtunnel *>
    (GetVal(new_vtun_ckv));

  val_unw_spdom_ext *new_val_spd = static_cast<val_unw_spdom_ext*>(
      GetVal(new_spine_dom_ckv));
  SET_USER_DATA_CTRLR(new_vtun_ckv,
                      new_val_spd->val_unw_spine_dom.spine_controller_id);
  SET_USER_DATA_DOMAIN(new_vtun_ckv,
                       new_val_spd->val_unw_spine_dom.spine_domain_id);
  vtunnel_val->label = gvtnid;
  // fill the ref_count from old vtunnel entry
  vtunnel_val->ref_count = static_cast<val_convert_vtunnel*>(
      GetVal(old_vtun_ckv))->ref_count;
  vtunnel_val->valid[UPLL_IDX_LABEL_CONV_VTNL] = UNC_VF_VALID;
  vtunnel_val->valid[UPLL_IDX_REF_COUNT_CONV_VTNL] = UNC_VF_VALID;

  // create the converted vtunnel
  // converted vtunnel name filled already
  result_code = vtun_momgr->ConvertVtunnel(new_vtun_ckv, NULL, UNC_OP_UPDATE,
      config_mode, vtn_name, dmi);
  if (result_code != UPLL_RC_SUCCESS) {
    DELETE_IF_NOT_NULL(new_vtun_ckv);
    UPLL_LOG_DEBUG("ConvertVtunnel returns erro:%d", result_code);
  }
  return result_code;
}

upll_rc_t VbrPortMapMoMgr::CreateVlinkForSpineIdChange(
    ConfigKeyVal *new_vtun_ckv, ConfigKeyVal *old_vlink_ckv, DalDmlIntf *dmi,
    TcConfigMode config_mode, string vtn_name) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  MoMgrImpl *vlink_momgr = reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>
      (GetMoManager(UNC_KT_VLINK)));
  ConfigKeyVal *new_vlink_ckv = NULL;

  ConfigKeyVal *tmp_ckv = old_vlink_ckv;
  while (tmp_ckv) {
    // construct convert vlink ckv from old_vlink_ckv
    result_code = vlink_momgr->GetChildConvertConfigKey(new_vlink_ckv, tmp_ckv);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("vlink momgr GetChildConfigKey failed:%d", result_code);
      return result_code;
    }

    val_convert_vlink_t  *vlink_conv_val = static_cast<val_convert_vlink_t*>(
        ConfigKeyVal::Malloc(sizeof(val_convert_vlink_t)));

    val_convert_vlink_t *old_vlink_val = static_cast<val_convert_vlink_t*>(
        GetVal(old_vlink_ckv));
    val_convert_vtunnel_t *new_vtun_val = static_cast<
      val_convert_vtunnel_t*>(GetVal(new_vtun_ckv));

    // fill val structure
    uuu::upll_strncpy(vlink_conv_val->vnode1_name,
        old_vlink_val->vnode1_name,
        kMaxLenConvertVnodeName+1);
    uuu::upll_strncpy(vlink_conv_val->vnode1_ifname,
        old_vlink_val->vnode1_ifname,
        kMaxLenInterfaceName+1);
    uuu::upll_strncpy(vlink_conv_val->vnode2_name,
        old_vlink_val->vnode2_name,
        kMaxLenConvertVnodeName+1);
    uuu::upll_strncpy(vlink_conv_val->vnode2_ifname,
        old_vlink_val->vnode2_ifname,
        kMaxLenInterfaceName+1);
    vlink_conv_val->label_type = UPLL_LABEL_TYPE_VLAN;
    vlink_conv_val->label = new_vtun_val->label;

    vlink_conv_val->valid[UPLL_IDX_VNODE1_NAME_CVLINK] = UNC_VF_VALID;
    vlink_conv_val->valid[UPLL_IDX_VNODE1_IF_NAME_CVLINK] =
      UNC_VF_VALID;
    vlink_conv_val->valid[UPLL_IDX_VNODE2_NAME_CVLINK] = UNC_VF_VALID;
    vlink_conv_val->valid[UPLL_IDX_VNODE2_IF_NAME_CVLINK] =
      UNC_VF_VALID;
    vlink_conv_val->valid[UPLL_IDX_LABEL_TYPE_CVLINK] = UNC_VF_VALID;
    vlink_conv_val->valid[UPLL_IDX_LABEL_CVLINK] = UNC_VF_VALID;

    new_vlink_ckv->AppendCfgVal(new ConfigVal(IpctSt::kIpcStValConvertVlink,
          vlink_conv_val));
    controller_domain ctrlr_dom1 = {NULL, NULL};
    GET_USER_DATA_CTRLR_DOMAIN(tmp_ckv, ctrlr_dom1);
    SET_USER_DATA_CTRLR_DOMAIN(new_vlink_ckv, ctrlr_dom1);
    controller_domain ctrlr_dom2 = {NULL, NULL};
    GET_USER_DATA_CTRLR_DOMAIN(new_vtun_ckv, ctrlr_dom2);
    SET_USER_DATA_CTRLR_DOMAIN(new_vlink_ckv->get_cfg_val(), ctrlr_dom2);

    // invoke VlinkMoMgr convert function
    result_code = vlink_momgr->ConvertVlink(new_vlink_ckv, UNC_OP_CREATE, dmi,
                                            config_mode, vtn_name);

    DELETE_IF_NOT_NULL(new_vlink_ckv);

    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("convert vlink creation failed:%d", result_code);
      return result_code;
    }
    tmp_ckv = tmp_ckv->get_next_cfg_key_val();
  }
  return result_code;
}

// This function updates used count in scratch table
upll_rc_t VbrPortMapMoMgr::UpdateUsedCntInScratchTbl(ConfigKeyVal *spd_ckv,
    uint8_t *ikey_vtnid, unc_keytype_operation_t operation, DalDmlIntf *dmi,
    TcConfigMode config_mode, string vtn_name) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  if (!spd_ckv || !(spd_ckv->get_key())) {
    UPLL_LOG_DEBUG("Input key is Empty");
    return UPLL_RC_ERR_GENERIC;
  }

  DalBindInfo *db_info = new DalBindInfo(uudst::kDbiSpdScratchTbl);

  uint8_t *vtnname = NULL;
  if (config_mode == TC_CONFIG_VTN) {
    if (!vtn_name.empty()) {
    vtnname = reinterpret_cast<uint8_t *>(
        const_cast<char *>(vtn_name.c_str()));
    } else {
      UPLL_LOG_DEBUG("Invalid vtn name");
      DELETE_IF_NOT_NULL(db_info);
      return UPLL_RC_ERR_GENERIC;
    }
  } else {
    vtnname = ikey_vtnid;
  }

  //  Bind Input unw_spine_domain
  db_info->BindInput(uudst::spd_scratch::kDbiUnwName,
                     uud::kDalChar, (kMaxLenUnifiedNwName + 1),
                     reinterpret_cast<void *>(
                         reinterpret_cast <key_unw_spine_domain_t*>
                         (spd_ckv->get_key())->unw_key.unified_nw_id));
  db_info->BindInput(uudst::spd_scratch::kDbiUnwSpineDomainName,
                     uud::kDalChar, (kMaxLenUnwSpineID + 1),
                     reinterpret_cast<void *>(
                         reinterpret_cast <key_unw_spine_domain_t*>
                         (spd_ckv->get_key())->unw_spine_id));

  //  Bind Input vtn_name
  db_info->BindInput(uudst::spd_scratch::kDbiVtnName,
                     uud::kDalChar, (kMaxLenVtnName + 1),
                     ikey_vtnid);
  std::string query_string;
  upll_keytype_datatype_t datatype = UPLL_DT_CANDIDATE;
  // Insert a new record in scratch tbl with used_count depending on
  // operation.
  if (operation == UNC_OP_CREATE) {
    query_string = QUERY_SPD_SCRATCH_TBL_CREATE_INSERT;
  } else {
    query_string = QUERY_SPD_SCRATCH_TBL_DELETE_INSERT;
  }

  result_code = DalToUpllResCode(
      dmi->ExecuteAppQuery(query_string, datatype,
                           uudst::kDbiSpdScratchTbl,  db_info,
                           UNC_OP_CREATE, config_mode,
                           vtnname));
  if ((result_code != UPLL_RC_SUCCESS) &&
      (result_code != UPLL_RC_ERR_INSTANCE_EXISTS)) {
    UPLL_LOG_ERROR("Scratch table insertion failed:%d", result_code);
    DELETE_IF_NOT_NULL(db_info);
    return result_code;
  }
  if (result_code == UPLL_RC_ERR_INSTANCE_EXISTS) {
    // this is the reverse of previous operation, remove the entry
    delete db_info;
    db_info = new DalBindInfo(uudst::kDbiSpdScratchTbl);
    std::string query_string;
    query_string = QUERY_DELETE_SPD_SCRATCH_TBL_FOR_MULTI_MATCH;
    db_info->BindMatch(uudst::spd_scratch::kDbiUnwName,
                       uud::kDalChar, (kMaxLenUnifiedNwName + 1),
                       reinterpret_cast<void *>(
                           reinterpret_cast <key_unw_spine_domain_t*>
                           (spd_ckv->get_key())->unw_key.unified_nw_id));
    db_info->BindMatch(uudst::spd_scratch::kDbiUnwSpineDomainName,
                       uud::kDalChar, (kMaxLenUnwSpineID + 1),
                       reinterpret_cast<void *>(
                           reinterpret_cast <key_unw_spine_domain_t*>
                           (spd_ckv->get_key())->unw_spine_id));
    db_info->BindMatch(uudst::spd_scratch::kDbiVtnName, uud::kDalChar,
                       (kMaxLenVtnName + 1), ikey_vtnid);

    result_code = DalToUpllResCode(
        dmi->ExecuteAppQuery(query_string, UPLL_DT_CANDIDATE,
                             uudst::kDbiSpdScratchTbl, db_info, UNC_OP_DELETE,
                             config_mode, vtnname));
    if (result_code != UPLL_RC_SUCCESS) {
      DELETE_IF_NOT_NULL(db_info);
      UPLL_LOG_DEBUG("Scratch table deletion failed:%d", result_code);
      return result_code;
    }
  }
  DELETE_IF_NOT_NULL(db_info);
  return result_code;
}

upll_rc_t VbrPortMapMoMgr::FindAffectedUvbrRecordsFromVbrPm(
                                      DalDmlIntf *dmi,
                                      upll_keytype_datatype_t dt_type,
                                      uint8_t *ctrlr_id,
                                      std::multimap<std::string, std::string>
                                            *affected_vbr_map,
                                      TcConfigMode config_mode,
                                      std::string vtn_name) {
  UPLL_FUNC_TRACE;

  upll_rc_t result_code = UPLL_RC_SUCCESS;
  unc_keytype_operation_t op[] = {UNC_OP_DELETE, UNC_OP_CREATE, UNC_OP_UPDATE};
  ConfigKeyVal *req = NULL, *nreq = NULL;
  DalCursor *cfg1_cursor;
  DalResultCode db_result = uud::kDalRcSuccess;
  int nop = sizeof(op)/ sizeof(op[0]);

  for (int i = 0; i < nop; i++)  {
    if (dt_type == UPLL_DT_CANDIDATE) {
      result_code= DiffConfigDB(UPLL_DT_CANDIDATE, UPLL_DT_RUNNING, op[i],
                                req, nreq, &cfg1_cursor, dmi, NULL,
                                config_mode, vtn_name,
                                MAINTBL, true);
    } else {
      bool auditdiff_with_flag = false;
      if (op[i] == UNC_OP_CREATE || op[i] == UNC_OP_DELETE)
        auditdiff_with_flag = true;
      result_code = DiffConfigDB(UPLL_DT_RUNNING, UPLL_DT_AUDIT,
                                 op[i],
                                 req, nreq, &cfg1_cursor,
                                 dmi, ctrlr_id, TC_CONFIG_GLOBAL,
                                 vtn_name, MAINTBL,
                                 true, auditdiff_with_flag);
    }
    while (result_code == UPLL_RC_SUCCESS) {
      db_result = dmi->GetNextRecord(cfg1_cursor);
      result_code = DalToUpllResCode(db_result);
      if (result_code != UPLL_RC_SUCCESS)
        break;

      key_vbr_portmap_t *vbr_pm_key = reinterpret_cast<key_vbr_portmap_t*>(
          req->get_key());
      std::pair <std::multimap<std::string, std::string>::iterator,
          std::multimap<std::string, std::string>::iterator> ret;
      ret = affected_vbr_map->equal_range(reinterpret_cast<char*>(
                                        vbr_pm_key->vbr_key.vtn_key.vtn_name));
      bool is_already_exists = false;
      for (std::multimap<std::string,
           std::string>::iterator it = ret.first; it != ret.second; ++it) {
        if (strcmp(reinterpret_cast<char*>((vbr_pm_key->vbr_key.vbridge_name)),
                                               (it->second).c_str()) == 0) {
          is_already_exists = true;
          break;
        }
      }

      if (is_already_exists == false) {
        affected_vbr_map->insert(std::pair<std::string, std::string>(
                reinterpret_cast<char*>(vbr_pm_key->vbr_key.vtn_key.vtn_name),
                reinterpret_cast<char*>(vbr_pm_key->vbr_key.vbridge_name)));
      }
    }
    DELETE_IF_NOT_NULL(req);
    DELETE_IF_NOT_NULL(nreq);
    if (cfg1_cursor) {
      dmi->CloseCursor(cfg1_cursor, true);
      cfg1_cursor = NULL;
    }
  }
  if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE)
    result_code = UPLL_RC_SUCCESS;
  return result_code;
}

upll_rc_t VbrPortMapMoMgr::UpdateUVbrConfigStatusFromVbrPm(
                 DalDmlIntf *dmi,
                 std::multimap<std::string, std::string> affected_vbr_map,
                 TcConfigMode config_mode, std::string vtn_name) {
  UPLL_FUNC_TRACE;

  upll_rc_t result_code = UPLL_RC_SUCCESS;
  for (std::map<std::string, std::string>::iterator
       it = affected_vbr_map.begin(); it != affected_vbr_map.end(); it++) {
    ConfigKeyVal *vbr_pm_ckv = NULL;
    result_code = GetChildConfigKey(vbr_pm_ckv, NULL);
    if (result_code != UPLL_RC_SUCCESS) {
      return result_code;
    }
    key_vbr_portmap_t *vbr_pm_key = reinterpret_cast<key_vbr_portmap_t*>(
        vbr_pm_ckv->get_key());
    uuu::upll_strncpy(vbr_pm_key->vbr_key.vtn_key.vtn_name,
              reinterpret_cast<uint8_t*>(const_cast<char*>(it->first.c_str())),
              (kMaxLenVtnName + 1));
    uuu::upll_strncpy(vbr_pm_key->vbr_key.vbridge_name,
              reinterpret_cast<uint8_t*>(const_cast<char*>(it->second.c_str())),
              (kMaxLenVnodeName + 1));

    DbSubOp read_dbop = { kOpReadMultiple, kOpMatchNone, kOpInOutCs };
    result_code = ReadConfigDB(vbr_pm_ckv, UPLL_DT_RUNNING, UNC_OP_READ,
                               read_dbop, dmi, MAINTBL);
    if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      DELETE_IF_NOT_NULL(vbr_pm_ckv);
      continue;
    }

    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_INFO("Read ConfigDb failed");
      DELETE_IF_NOT_NULL(vbr_pm_ckv);
      return result_code;
    }
    uint32_t db_cs_not_applied_count = 0;
    uint32_t db_cs_not_supported_count = 0;
    uint32_t db_cs_applied_count = 0;
    uint32_t db_cs_part_applied_count = 0;
    ConfigKeyVal *vbr_pm_start = vbr_pm_ckv;
    uint8_t vbr_cs_status = UNC_CS_APPLIED;
    while (vbr_pm_start) {
      pfcdrv_val_vbr_portmap_t *drv_vbrpm_val =
          reinterpret_cast<pfcdrv_val_vbr_portmap_t*>(GetVal(vbr_pm_start));
      val_vbr_portmap_t *vbrpm_val = &(drv_vbrpm_val->vbrpm);
      if (vbrpm_val->cs_row_status == UNC_CS_INVALID) {
        vbr_cs_status = UNC_CS_INVALID;
        break;
      } else if (vbrpm_val->cs_row_status == UNC_CS_NOT_APPLIED) {
        db_cs_not_applied_count++;
      } else if (vbrpm_val->cs_row_status == UNC_CS_PARTIALLY_APPLIED) {
        db_cs_part_applied_count++;
      } else if (vbrpm_val->cs_row_status == UNC_CS_APPLIED) {
        db_cs_applied_count++;
      } else if (vbrpm_val->cs_row_status == UNC_CS_NOT_SUPPORTED) {
        db_cs_not_supported_count++;
      }
      vbr_pm_start = vbr_pm_start->get_next_cfg_key_val();
    }

    if (vbr_cs_status != UNC_CS_INVALID) {
      if ((db_cs_not_applied_count > 0 || db_cs_not_supported_count > 0)
          && db_cs_applied_count == 0 && db_cs_part_applied_count == 0) {
        vbr_cs_status = UNC_CS_NOT_APPLIED;
      } else if ((db_cs_part_applied_count > 0) ||
                 (db_cs_applied_count > 0 && (db_cs_not_applied_count > 0 ||
                 db_cs_part_applied_count >0))) {
        vbr_cs_status = UNC_CS_PARTIALLY_APPLIED;
      } else if (db_cs_applied_count > 0 && db_cs_not_applied_count == 0 &&
                 db_cs_part_applied_count == 0) {
        vbr_cs_status = UNC_CS_APPLIED;
      }
    }
    DalBindInfo *db_info = new DalBindInfo(uudst::kDbiVbrTbl);
    //  Bind input VTN name
    db_info->BindMatch(uudst::vbridge::kDbiVtnName, uud::kDalChar,
                (kMaxLenVtnName + 1),
                reinterpret_cast<void*>(const_cast<char*>(it->first.c_str())));
    //  Bind input vbridge name
    db_info->BindMatch(uudst::vbridge::kDbiVbrName, uud::kDalChar,
              (kMaxLenVnodeName + 1),
               reinterpret_cast<void*>(const_cast<char*>(it->second.c_str())));
    //  Bind Input ctrlr_name
    db_info->BindInput(uudst::vbridge::kDbiCsRowStatus,
                       uud::kDalUint8, 1, &vbr_cs_status);

    std::string query_string3 = "UPDATE ru_vbr_tbl SET cs_rowstatus = ? "
        "WHERE vtn_name = ? and vbridge_name = ?";
    result_code = DalToUpllResCode(dmi->ExecuteAppQuery(
            query_string3, UPLL_DT_RUNNING, uudst::kDbiVbrTbl,
            db_info, UNC_OP_UPDATE, config_mode,
            reinterpret_cast<uint8_t*>(const_cast<char*>(vtn_name.c_str()))));
    DELETE_IF_NOT_NULL(db_info);
    DELETE_IF_NOT_NULL(vbr_pm_ckv);
  }
  if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    result_code = UPLL_RC_SUCCESS;
  }
  return result_code;
}

upll_rc_t VbrPortMapMoMgr::TxCopyCandidateToRunning(
    unc_key_type_t key_type,
    CtrlrCommitStatusList *ctrlr_commit_status,
    DalDmlIntf* dmi, TcConfigMode config_mode,
    std::string vtn_name) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  if (config_mode == TC_CONFIG_VIRTUAL)
    return UPLL_RC_SUCCESS;

  std::multimap<std::string, std::string> affected_vbr_map;
  result_code = FindAffectedUvbrRecordsFromVbrPm(dmi, UPLL_DT_CANDIDATE, NULL,
                                   &affected_vbr_map, config_mode, vtn_name);
  if (result_code != UPLL_RC_SUCCESS) {
    affected_vbr_map.clear();
    return result_code;
  }

  result_code = MoMgrImpl::TxCopyCandidateToRunning(key_type,
              ctrlr_commit_status, dmi, config_mode, vtn_name);
  if (result_code != UPLL_RC_SUCCESS) {
    affected_vbr_map.clear();
    return result_code;
  }

  result_code = UpdateUVbrConfigStatusFromVbrPm(dmi, affected_vbr_map,
                                                config_mode, vtn_name);
  affected_vbr_map.clear();
  if (result_code != UPLL_RC_SUCCESS)
    return result_code;
  unc_keytype_operation_t op[] = {UNC_OP_DELETE, UNC_OP_CREATE, UNC_OP_UPDATE};
  uint8_t nop = sizeof(op)/ sizeof(op[0]);

  for (uint8_t i = 0; i < nop; i++)  {
    // Copying Rename Table to Running
    UPLL_LOG_TRACE("keytype is %d", key_type);
    result_code = TxCopyConvertTblFromCandidateToRunning(op[i],
                            ctrlr_commit_status, dmi, config_mode, vtn_name);
    UPLL_LOG_DEBUG("TxCopyConvertTblFromCandidateToRunning %d",
                                                            result_code);
    if (result_code != UPLL_RC_SUCCESS &&
        result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      return result_code;
    }
    if (op[i] == UNC_OP_DELETE) {
      result_code = TxCopyLabelTblFromCandidateToRunning(
                          op[i], dmi, config_mode, vtn_name);
      UPLL_LOG_DEBUG("TxCopyLabelTblFromCandidateToRunning %d",
                                               result_code);
      if (result_code != UPLL_RC_SUCCESS &&
        result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
        return result_code;
      }
    }
  }
  result_code = UpdateRunningUsedCntFromScratchTbl(config_mode, vtn_name,
                                                   dmi);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("UpdateRunningUsedCnt failed");
    return result_code;
  }
  result_code = ClearScratchTbl(config_mode, vtn_name, dmi);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("ClearScratchTbl failed:%d", result_code);
  }

  return result_code;
}

upll_rc_t VbrPortMapMoMgr::AuditCommitCtrlrStatus(
                                       unc_key_type_t keytype ,
                                       CtrlrCommitStatus *ctrlr_commit_status,
                                       DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  uint8_t *ctrlr_id =
         reinterpret_cast<uint8_t *>
         (const_cast<char *>(ctrlr_commit_status->ctrlr_id.c_str()));
  std::multimap<std::string, std::string> affected_vbr_map;
  result_code = FindAffectedUvbrRecordsFromVbrPm(dmi, UPLL_DT_AUDIT, ctrlr_id,
                              &affected_vbr_map, TC_CONFIG_GLOBAL, "");
  if (result_code != UPLL_RC_SUCCESS) {
    affected_vbr_map.clear();
    return result_code;
  }

  result_code = MoMgrImpl::AuditCommitCtrlrStatus(UNC_KT_VBR_PORTMAP,
                                                  ctrlr_commit_status, dmi);
  if (result_code != UPLL_RC_SUCCESS) {
    affected_vbr_map.clear();
    return result_code;
  }

  result_code = UpdateUVbrConfigStatusFromVbrPm(dmi, affected_vbr_map,
                                   TC_CONFIG_GLOBAL, "");
  affected_vbr_map.clear();
  return result_code;
}

// Update used label count in spine_domain tbl in CANDIDATE if aborting.
// And clear the records in scratch table depending upon config_mode.
upll_rc_t VbrPortMapMoMgr::ClearScratchTbl(TcConfigMode config_mode,
    string vtn_name, DalDmlIntf *dmi, bool is_abort) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  // Update used label count in spine_domain tbl in CANDIDATE if aborting.
  if (is_abort) {
    // For abort operation update unw_spine_domain used_label_count
    result_code = (config_mode == TC_CONFIG_VTN) ?
                  UpdateUnwSdLabelCountInVtnModeAbort(config_mode, vtn_name,
                                                      dmi) :
                  UpdateUsedCntForGlobalMode(dmi, is_abort);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_INFO("Failed to abort spd used_label_cnt %d", result_code);
      return result_code;
    }
  }

  // Clear the records in scratch table depending upon config_mode.
  DalBindInfo *db_info = new DalBindInfo(uudst::kDbiSpdScratchTbl);

  uint8_t *vtnname = NULL;
  std::string query_string = QUERY_DELETE_ALL_SPD_SCRATCH_TBL;  // Global mode

  if (config_mode == TC_CONFIG_VTN) {
    query_string = QUERY_DELETE_VTN_SPD_SCRATCH_TBL;
    if (!vtn_name.empty()) {
      vtnname = reinterpret_cast<uint8_t *>(
          const_cast<char *>(vtn_name.c_str()));
    } else {
      UPLL_LOG_ERROR("Invalid vtn name");
      DELETE_IF_NOT_NULL(db_info);
      return UPLL_RC_ERR_GENERIC;
    }

    // delete vtn specific entry from scratch table
    db_info->BindMatch(uudst::spd_scratch::kDbiVtnName, uud::kDalChar,
                       (kMaxLenVtnName + 1), vtnname);
  }
  result_code = DalToUpllResCode(
      dmi->ExecuteAppQuery(query_string, UPLL_DT_CANDIDATE,
        uudst::kDbiSpdScratchTbl, db_info, UNC_OP_DELETE,
        config_mode, vtnname));

  DELETE_IF_NOT_NULL(db_info);
  if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
    result_code = UPLL_RC_SUCCESS;
  }
  return result_code;
}

// For each unique spine domain in the scratch table, do the following:
// If commit, read the used_label_count from candidate spine_domain tbl and
// update running.
// If abort, read the used_label_count from running spine_domain tbl and
// update candidate.
upll_rc_t VbrPortMapMoMgr::UpdateUsedCntForGlobalMode(DalDmlIntf *dmi,
                                                      bool is_abort) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  // get the distinct unw_name and spine domain from ca_spd_scratch_tbl
  DalBindInfo db_info(uudst::kDbiSpdScratchTbl);
  UNWSpineDomainMoMgr *unw_sd_momgr = reinterpret_cast<UNWSpineDomainMoMgr *>(
     const_cast<MoManager *>(GetMoManager(UNC_KT_UNW_SPINE_DOMAIN)));
  uint8_t scrtbl_unified_nw_id[kMaxLenUnifiedNwName+1];
  uint8_t scrtbl_unw_spine_id[kMaxLenUnwSpineID+1];

  db_info.BindOutput(uudst::spd_scratch::kDbiUnwName,
                     uud::kDalChar, (kMaxLenUnifiedNwName + 1),
                     scrtbl_unified_nw_id);

  db_info.BindOutput(uudst::spd_scratch::kDbiUnwSpineDomainName,
                     uud::kDalChar, (kMaxLenUnwSpineID + 1),
                     scrtbl_unw_spine_id);

  // frame the query
  std::string query_string;
  query_string = QUERY_DISTINCT_SPD_SCRATCH_TBL_ENTRY;

  // execute query
  DalCursor *dal_cursor_handle = NULL;
  result_code = DalToUpllResCode(dmi->ExecuteAppQueryMultipleRecords(
          query_string, 0, &db_info, &dal_cursor_handle));

  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_ERROR("scratch table read failed %d", result_code);
    if (dal_cursor_handle) {
      dmi->CloseCursor(dal_cursor_handle, false);
    }
    return result_code;
  }

  result_code = DalToUpllResCode(dmi->GetNextRecord(dal_cursor_handle));
  if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    dmi->CloseCursor(dal_cursor_handle, false);
    UPLL_LOG_TRACE("No entry in scratch table");
    return UPLL_RC_SUCCESS;
  }
  // do ReadMultiple on unw_spinedomain table from source DT
  ConfigKeyVal *read_mul_spd_ckv = NULL;
  result_code = GetUnwSpineDomain(
      &read_mul_spd_ckv, dmi,
      ((is_abort) ? UPLL_DT_RUNNING : UPLL_DT_CANDIDATE));
  if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    UPLL_LOG_DEBUG("source unw_spine_dom_tbl empty");
    dmi->CloseCursor(dal_cursor_handle, false);
    return UPLL_RC_SUCCESS;
  }
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Unw spine domain read multiple failed:%d", result_code);
    dmi->CloseCursor(dal_cursor_handle, false);
    return result_code;
  }

  DbSubOp dbop = { kOpNotRead, kOpMatchNone, kOpInOutNone };
  string vtnname;  // pass empty vtn_name for global mode DB operations
  // For each unique spine_domain found in scarch table, update the dest
  // spine_domain tbl with the value in source spine_domain tbl.
  while (result_code == UPLL_RC_SUCCESS) {
    // Find the matching <unw_name, spine_id> record from read_mul_spd_ckv and
    // update it in running/candidate if commit/abort respectively
    bool found = false;
    for (ConfigKeyVal *tmp = read_mul_spd_ckv; tmp;
         tmp = tmp->get_next_cfg_key_val()) {
      key_unw_spine_domain *key_unw_sd = static_cast<key_unw_spine_domain*>(
          tmp->get_key());
      if ((!(uuu::upll_strncmp(key_unw_sd->unw_key.unified_nw_id,
                               scrtbl_unified_nw_id, kMaxLenUnifiedNwName+1)))
          && (!(uuu::upll_strncmp(key_unw_sd->unw_spine_id,
                                  scrtbl_unw_spine_id, kMaxLenUnwSpineID+1)))) {
        static_cast<val_unw_spdom_ext*>(GetVal(tmp))->valid[
            UPLL_IDX_SPINE_USED_LABEL_COUNT_UNWS] = UNC_VF_VALID;

        result_code = unw_sd_momgr->UpdateConfigDB(
            tmp, ((is_abort) ? UPLL_DT_CANDIDATE : UPLL_DT_RUNNING),
            UNC_OP_UPDATE,  dmi, &dbop,
            TC_CONFIG_GLOBAL, vtnname, MAINTBL);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_ERROR("UpdateConfigDB failed:%d", result_code);
          dmi->CloseCursor(dal_cursor_handle, false);
          DELETE_IF_NOT_NULL(read_mul_spd_ckv);
          return result_code;
        }
        found = true;
        break;
      }
    }
    if (!found) {
      UPLL_LOG_ERROR("Not found %s %s",
                     scrtbl_unified_nw_id, scrtbl_unw_spine_id);
    }
    result_code = DalToUpllResCode(dmi->GetNextRecord(dal_cursor_handle));
  }
  if (!is_abort) {
    // set the flag to indicate the used_count change for alarm check
    unw_sd_momgr->set_threshold_alarm_flag(true);
  }
  dmi->CloseCursor(dal_cursor_handle, false);
  DELETE_IF_NOT_NULL(read_mul_spd_ckv);
  result_code = UPLL_RC_SUCCESS;
  return result_code;
}

/** updates the used count in spine domain running DB based on the PCM
 * and scratch table entry
 */
upll_rc_t VbrPortMapMoMgr::UpdateRunningUsedCntFromScratchTbl(
    TcConfigMode config_mode, string vtn_name, DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;

  upll_rc_t result_code = UPLL_RC_SUCCESS;

  if (config_mode == TC_CONFIG_GLOBAL) {
    result_code = UpdateUsedCntForGlobalMode(dmi, false);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("used_cnt update in global mode failed:%d", result_code);
    }
    return result_code;
  }

  uint8_t *vtnname = NULL;
  // update the used label count in running tbl if any exists
  if (!vtn_name.empty()) {
    vtnname = reinterpret_cast<uint8_t *>(
        const_cast<char *>(vtn_name.c_str()));
  } else {
    UPLL_LOG_ERROR("Invalid vtn name");
    return UPLL_RC_ERR_GENERIC;
  }

  // update the used label count in running tbl if any exists
  ConfigKeyVal *spd_ckv = NULL;
  uint32_t used_count = 0;
  // get the used count, spd_ckv  for the vtn from scratch table
  DalBindInfo db_info(uudst::kDbiSpdScratchTbl);
  UNWSpineDomainMoMgr *unw_sd_momgr =
      reinterpret_cast<unc::upll::kt_momgr::UNWSpineDomainMoMgr *>(
          const_cast<MoManager *>(GetMoManager(UNC_KT_UNW_SPINE_DOMAIN)));

  result_code = unw_sd_momgr->GetChildConfigKey(spd_ckv, NULL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_ERROR("GetChildConfigKey failed:%d", result_code);
    return result_code;
  }

  // check whether the entry exists in scratch table for the given vtn
  db_info.BindMatch(uudst::spd_scratch::kDbiVtnName, uud::kDalChar,
                    (kMaxLenVtnName + 1), vtnname);

  db_info.BindOutput(uudst::spd_scratch::kDbiUnwName,
                     uud::kDalChar, (kMaxLenUnifiedNwName + 1),
                     static_cast<key_unw_spine_domain*>(
                         spd_ckv->get_key())->unw_key.unified_nw_id);

  db_info.BindOutput(uudst::spd_scratch::kDbiUnwSpineDomainName,
                     uud::kDalChar, (kMaxLenUnwSpineID + 1),
                     static_cast<key_unw_spine_domain*>(
                         spd_ckv->get_key())->unw_spine_id);

  //  Bind Output ref_count
  db_info.BindOutput(uudst::spd_scratch::kDbiUsedCount,
                     uud::kDalUint32, 1, &used_count);

  DalCursor *dal_cursor_handle = NULL;
  result_code = DalToUpllResCode(dmi->GetMultipleRecords(
          UPLL_DT_CANDIDATE, uudst::kDbiSpdScratchTbl, 0,
          &db_info, &dal_cursor_handle));
  if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    UPLL_LOG_TRACE("No entry in scratch table");
    DELETE_IF_NOT_NULL(spd_ckv);
    if (dal_cursor_handle) {
      dmi->CloseCursor(dal_cursor_handle, false);
    }
    return UPLL_RC_SUCCESS;
  }
  if (result_code != UPLL_RC_SUCCESS) {
    DELETE_IF_NOT_NULL(spd_ckv);
    UPLL_LOG_ERROR("scratch table read failed %d", result_code);
    if (dal_cursor_handle) {
      dmi->CloseCursor(dal_cursor_handle, false);
    }
    return result_code;
  }

  DbSubOp dbop = { kOpNotRead, kOpMatchNone, kOpInOutNone };

  while (1) {
    result_code = DalToUpllResCode(dmi->GetNextRecord(dal_cursor_handle));

    if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
      dmi->CloseCursor(dal_cursor_handle, false);
      DELETE_IF_NOT_NULL(spd_ckv);
      result_code = UPLL_RC_SUCCESS;
      return result_code;
    }
    // entry exists, read spd table and update ref_count
    result_code = GetUnwSpineDomain(&spd_ckv, dmi, UPLL_DT_RUNNING);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_ERROR("Unw spine domain running table read failed:%d",
                     result_code);
      dmi->CloseCursor(dal_cursor_handle, false);
      DELETE_IF_NOT_NULL(spd_ckv);
      return result_code;
    }

    // calculate used count
    static_cast<val_unw_spdom_ext*>(
        GetVal(spd_ckv))->used_label_count += (used_count);
    static_cast<val_unw_spdom_ext*>(GetVal(spd_ckv))->valid[
        UPLL_IDX_SPINE_USED_LABEL_COUNT_UNWS] = UNC_VF_VALID;

    // update used count in RUNNING
    result_code = unw_sd_momgr->UpdateConfigDB(spd_ckv, UPLL_DT_RUNNING,
                                               UNC_OP_UPDATE, dmi, &dbop,
                                               config_mode, vtn_name, MAINTBL);
    // remove cfg_val to store the values for next iteration
    spd_ckv->DeleteCfgVal();
    if (result_code == UPLL_RC_SUCCESS) {
      // set the flag to indicate the used_count change for alarm check
      unw_sd_momgr->set_threshold_alarm_flag(true);
    }
    if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
      result_code = UPLL_RC_SUCCESS;
    }

    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_ERROR("used_label_count update failed:%d", result_code);
      dmi->CloseCursor(dal_cursor_handle, false);
      DELETE_IF_NOT_NULL(spd_ckv);
      return result_code;
    }
  }
  return result_code;
}

upll_rc_t VbrPortMapMoMgr::BoundaryMapReq(IpcReqRespHeader *req,
                                          ConfigKeyVal *ikey,
                                          ConfigKeyVal *db_vlink,
                                          ConfigKeyVal *vbr_pm_ckv,
                                          ConfigKeyVal *ck_boundary,
                                          DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;

  upll_rc_t      result_code  = UPLL_RC_SUCCESS;

  //  validate input params
  if (!req || !ikey || !db_vlink || !vbr_pm_ckv || !dmi) {
    UPLL_LOG_INFO("Cannot perform this request"
                  "due to insufficient parameters");
    return UPLL_RC_ERR_GENERIC;
  }

  val_vlink_t *ival = NULL;
  val_vlink_t *db_val = reinterpret_cast<val_vlink *>(GetVal(db_vlink));
  if (!db_vlink) {
    UPLL_LOG_TRACE("vlink val structure not present");
    return UPLL_RC_ERR_GENERIC;
  }

  unc_keytype_operation_t lc_op = req->operation;
  //  If request operation is UPDATE, change the operation based on db vLink
  //  vLink boundary flag and received ikey boundary flag used for vbr_portmap
  //  operation.
  //
  //  If received boundary flag is VALID and db flag is INVALID
  //  update operation as CREATE
  //  If received boundary flag is VALID and db flag is VALID
  //  update operation as UPDATE
  //  If received boundary flag is VALID_NO_VALUE and db flag is VALID
  //  update operation as UPDATE
  if (req->operation == UNC_OP_UPDATE) {
    ival = reinterpret_cast<val_vlink *>(GetVal(ikey));
    if (!ival) {
      UPLL_LOG_TRACE("vlink val structure not present");
      return UPLL_RC_ERR_GENERIC;
    }

    if (ival->valid[UPLL_IDX_BOUNDARY_NAME_VLNK] == UNC_VF_VALID &&
        db_val->valid[UPLL_IDX_BOUNDARY_NAME_VLNK] == UNC_VF_INVALID) {
       req->operation = UNC_OP_CREATE;
    } else if (ival->valid[UPLL_IDX_BOUNDARY_NAME_VLNK] == UNC_VF_VALID &&
        db_val->valid[UPLL_IDX_BOUNDARY_NAME_VLNK] == UNC_VF_VALID) {
       req->operation = UNC_OP_UPDATE;
    } else if (ival->valid[UPLL_IDX_BOUNDARY_NAME_VLNK] ==
               UNC_VF_VALID_NO_VALUE &&
        db_val->valid[UPLL_IDX_BOUNDARY_NAME_VLNK] == UNC_VF_VALID) {
       req->operation = UNC_OP_DELETE;
    }
  }

  /* ck_boundary ConfigKeyVal contains logical port id info */
  if (!ck_boundary || !GetVal(ck_boundary)) {
    UPLL_LOG_TRACE("Boundary details is not there");
    return UPLL_RC_ERR_GENERIC;
  }
  val_boundary_t *boundary_val = reinterpret_cast<val_boundary_t *>(
      GetVal(ck_boundary));

  controller_domain_t vlink_ctrlr_dom[2] = {{NULL, NULL}, {NULL, NULL}};
  VlinkMoMgr *vlink_mgr = reinterpret_cast<VlinkMoMgr *>(
      const_cast<MoManager *>(GetMoManager(UNC_KT_VLINK)));

  //  Get vLink controller domain
  result_code = vlink_mgr->GetControllerDomainId(db_vlink, vlink_ctrlr_dom);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_TRACE("Failed to get vlink controller and domain");
    return UPLL_RC_SUCCESS;
  }

  uint8_t *controller_name = NULL;
  uint8_t *domain_name = NULL;
  uint8_t *logical_port_id = NULL;

  //  Get controller and domain for vBridge portmap from boundary_ckv
  if ((strcmp(reinterpret_cast<char *>(vlink_ctrlr_dom[0].ctrlr), "#") == 0) &&
      (strcmp(reinterpret_cast<char *>(vlink_ctrlr_dom[0].domain), "#") == 0)) {
    // If node1 is unified vBridge, validate node2 controller and domain is
    // matched with any of the boundary controller domain.
    // If not matched return UPLL_RC_ERR_GENERIC.
    if ((strcmp(reinterpret_cast<char *>(vlink_ctrlr_dom[1].ctrlr),
             reinterpret_cast<char *>(boundary_val->controller_name1)) == 0) &&
        (strcmp(reinterpret_cast<char *>(vlink_ctrlr_dom[1].domain),
               reinterpret_cast<char *>(boundary_val->domain_name1)) == 0)) {
      logical_port_id = boundary_val->logical_port_id2;
      controller_name = boundary_val->controller_name2;
      domain_name     = boundary_val->domain_name2;
    } else if ((strcmp(reinterpret_cast<char *>(vlink_ctrlr_dom[1].ctrlr),
             reinterpret_cast<char *>(boundary_val->controller_name2)) == 0) &&
        (strcmp(reinterpret_cast<char *>(vlink_ctrlr_dom[1].domain),
            reinterpret_cast<char *>(boundary_val->domain_name2)) == 0)) {
      logical_port_id = boundary_val->logical_port_id1;
      controller_name = boundary_val->controller_name1;
      domain_name     = boundary_val->domain_name1;
    } else {
      UPLL_LOG_ERROR("Vlink node2 controller domain is not matched with"
                     "boundary controller domain");
      return UPLL_RC_ERR_GENERIC;
    }
  } else {
    // If node2 is unified vBridge, validate node1 controller and domain is
    // matched with any of the boundary controller domain.
    // If not matched return UPLL_RC_ERR_GENERIC.
    if ((strcmp(reinterpret_cast<char *>(vlink_ctrlr_dom[0].ctrlr),
             reinterpret_cast<char *>(boundary_val->controller_name1)) == 0) &&
        (strcmp(reinterpret_cast<char *>(vlink_ctrlr_dom[0].domain),
           reinterpret_cast<char *>(boundary_val->domain_name1)) == 0)) {
      logical_port_id = boundary_val->logical_port_id2;
      controller_name = boundary_val->controller_name2;
      domain_name     = boundary_val->domain_name2;
    } else if ((strcmp(reinterpret_cast<char *>(vlink_ctrlr_dom[0].ctrlr),
             reinterpret_cast<char *>(boundary_val->controller_name2)) == 0) &&
        (strcmp(reinterpret_cast<char *>(vlink_ctrlr_dom[0].domain),
            reinterpret_cast<char *>(boundary_val->domain_name2)) == 0)) {
      logical_port_id = boundary_val->logical_port_id1;
      controller_name = boundary_val->controller_name1;
      domain_name     = boundary_val->domain_name1;
    } else {
      UPLL_LOG_ERROR("Vlink node2 controller domain is not matched with"
                     "boundary controller domain");
      return UPLL_RC_ERR_GENERIC;
    }
  }

  if (!controller_name || !domain_name || !logical_port_id) {
    UPLL_LOG_DEBUG("Failed to get controller/domain/logical_port_id"
                    "from boundary");
    return UPLL_RC_ERR_GENERIC;
  }

  SET_USER_DATA_CTRLR(vbr_pm_ckv, controller_name);
  SET_USER_DATA_DOMAIN(vbr_pm_ckv, domain_name);

  // Allocate val_vbr_portmap structure
  pfcdrv_val_vbr_portmap_t *val_vbr_pm =
            ConfigKeyVal::Malloc<pfcdrv_val_vbr_portmap_t>();
  val_vbr_pm->valid[PFCDRV_IDX_VAL_VBR_PORTMAP] = UNC_VF_VALID;
  // Populate val_vbr_portmap structure in vbr_pm_ckv
  uuu::upll_strncpy(val_vbr_pm->vbrpm.logical_port_id,
                    logical_port_id, (kMaxLenLogicalPortId + 1));
  val_vbr_pm->vbrpm.valid[UPLL_IDX_LOGICAL_PORT_ID_VBRPM] = UNC_VF_VALID;
  uuu::upll_strncpy(val_vbr_pm->vbrpm.controller_id, controller_name,
                   (kMaxLenCtrlrId + 1));
  val_vbr_pm->vbrpm.valid[UPLL_IDX_CONTROLLER_ID_VBRPM] = UNC_VF_VALID;
  uuu::upll_strncpy(val_vbr_pm->vbrpm.domain_id, domain_name,
                   (kMaxLenDomainId + 1));
  val_vbr_pm->vbrpm.valid[UPLL_IDX_DOMAIN_ID_VBRPM] = UNC_VF_VALID;

  vbr_pm_ckv->SetCfgVal(new ConfigVal(IpctSt::kIpcStPfcdrvValVbrPortMap,
                                      val_vbr_pm));

  VbrIfMoMgr *vbrif_mgr = reinterpret_cast<VbrIfMoMgr *>(
       const_cast<MoManager *>(GetMoManager(UNC_KT_VBR_IF)));
  switch (req->operation) {
    case UNC_OP_CREATE:
      ival = reinterpret_cast<val_vlink *>(GetVal(ikey));
      if (!ival) {
        UPLL_LOG_TRACE("vlink val structure not present");
        return UPLL_RC_ERR_GENERIC;
      }

      val_vbr_pm->vbrpm.label_type = ival->label_type;
      val_vbr_pm->vbrpm.valid[UPLL_IDX_LABEL_TYPE_VBRPM] = UNC_VF_VALID;
      val_vbr_pm->vbrpm.label = ival->label;
      val_vbr_pm->vbrpm.valid[UPLL_IDX_LABEL_VBRPM] = UNC_VF_VALID;

      //  Create new vBridge portmap or set boundarymap bit in vbrportmap
      //  flag and increment boundary ref count
      result_code = BoundaryVbrPortmapCreate(vbr_pm_ckv, req, dmi);
      if (result_code != UPLL_RC_SUCCESS) {
        if (result_code == UPLL_RC_ERR_EXPAND ||
            result_code == UPLL_RC_ERR_VLAN_IN_USE ||
            result_code == UPLL_RC_ERR_VLAN_TYPE_MISMATCH ||
            result_code == UPLL_RC_ERR_INVALID_DOMAIN) {
          result_code = UPLL_RC_ERR_CFG_SEMANTIC;
        }
        UPLL_LOG_DEBUG("Failed to create boundary vbr_portmap");
        return result_code;
      }

      //  Create unified vBridge interface in corresponding converted vBridge
      result_code = vbrif_mgr->CreateConvertVbridgeInterfaceFromVlink(db_vlink,
                                          vbr_pm_ckv, req, dmi);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Failed to create converted vBridge interface");
        return result_code;
      }

      break;
    case UNC_OP_UPDATE:
      //  Update vbr portmap label value with new vLink label value
      result_code = BoundaryVbrPortmapUpdate(ikey, db_vlink,
                                             vbr_pm_ckv, req, dmi);
      if (result_code != UPLL_RC_SUCCESS) {
        if (result_code == UPLL_RC_ERR_EXPAND ||
            result_code == UPLL_RC_ERR_VLAN_IN_USE ||
            result_code == UPLL_RC_ERR_VLAN_TYPE_MISMATCH ||
            result_code == UPLL_RC_ERR_INVALID_DOMAIN) {
          result_code = UPLL_RC_ERR_CFG_SEMANTIC;
        }
        UPLL_LOG_DEBUG("Boundary vbrportmap update failed. error:%u",
                        result_code);
        return result_code;
      }
      break;
    case UNC_OP_DELETE:

      // vBridge interface delete in corresponding converted vBridge
      result_code = vbrif_mgr->DeleteConvertVbridgeInterfaceFromVlink(db_vlink,
          vbr_pm_ckv, req, dmi);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_ERROR("Failed to delete converted vBridge interface");
        return result_code;
      }

      result_code = BoundaryVbrPortmapDelete(db_vlink, vbr_pm_ckv, req, dmi);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Failed to delete boundary vbr_portmap");
        return result_code;
      }

      break;
    default:
      break;
  }
  // operation again set to original value
  req->operation = lc_op;
  return result_code;
}

upll_rc_t VbrPortMapMoMgr::BoundaryVbrPortmapCreate(ConfigKeyVal *vbr_pm_ckv,
                                                    IpcReqRespHeader *req,
                                                    DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;

  upll_rc_t result_code = UPLL_RC_SUCCESS;
  uint8_t   flags       = 0;

  TcConfigMode config_mode = TC_CONFIG_INVALID;
  std::string cfg_vtn_name = "";
  //  Get acquired config mode
  result_code = GetConfigModeInfo(req, config_mode, cfg_vtn_name);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetConfigMode failed");
    return result_code;
  }

  DbSubOp read_dbop = { kOpReadSingle, kOpMatchCtrlr | kOpMatchDomain,
    kOpInOutFlag };
  // Checks the existance of vbr_portmap entry in ca_vbr_portmap_tbl
  // for same logical_port_id, label, controller and domain
  result_code = ReadConfigDB(vbr_pm_ckv, req->datatype, UNC_OP_READ, read_dbop,
      dmi, MAINTBL);
  if (result_code != UPLL_RC_SUCCESS &&
      result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    UPLL_LOG_TRACE("Failed to read vbr_portmap info from DB");
    return result_code;
  }

  pfcdrv_val_vbr_portmap_t *val_vbr_pm =
    reinterpret_cast<pfcdrv_val_vbr_portmap_t*>(GetVal(vbr_pm_ckv));

  //  Increment bdry_ref_count
  val_vbr_pm->bdry_ref_count++;
  val_vbr_pm->valid[PFCDRV_IDX_VBRPM_BDRY_REF_COUNT] = UNC_VF_VALID;

  DbSubOp update_dbop = { kOpNotRead, kOpMatchNone, kOpInOutFlag };
  if (result_code == UPLL_RC_SUCCESS) {
    // Already portmap entry is there for same logical_port_id,label,
    // controller and domain. Increment bdry_ref_count and set boundary
    // configured bit in vbr_pm_ckv flag
    //  update flag and bdry_ref count

    // Set boundary configured bit in vbr_pm_ckv flag
    GET_USER_DATA_FLAGS(vbr_pm_ckv, flags);
    flags |= BOUNDARY_VBRPORTMAP_FLAG;
    SET_USER_DATA_FLAGS(vbr_pm_ckv, flags);
    result_code = UpdateConfigDB(vbr_pm_ckv, req->datatype, UNC_OP_UPDATE,
        dmi, &update_dbop, config_mode, cfg_vtn_name, MAINTBL);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("bdry_ref_count update failed");
      return result_code;
    }
    return result_code;
  } else {
    // vbr_portmap entry is not there in DB, autogenerate portmap_id and
    // create vbr_portmap in ca_vbr_portmap_tbl

    // autogenerate portmap_id
    while (1) {
      struct timeval _timeval;
      struct timezone _timezone;
      gettimeofday(&_timeval, &_timezone);
      std::stringstream ss;
      ss << _timeval.tv_sec << _timeval.tv_usec;
      std::string unique_id = ss.str();
      std::string portmap_id("vlpm_");
      portmap_id += unique_id;

      // populate portmap_id in vbr_pm_ckv
      key_vbr_portmap *vbr_pm_key =
          reinterpret_cast<key_vbr_portmap*>(vbr_pm_ckv->get_key());
      uuu::upll_strncpy(vbr_pm_key->portmap_id, portmap_id.c_str(),
                        (kMaxLenPortMapName + 1));

      ConfigKeyVal *lc_vbrpm_ckv = NULL;
      result_code = GetChildConfigKey(lc_vbrpm_ckv, vbr_pm_ckv);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_ERROR("Failed to get vbr_portmap config key val");
        return result_code;
      }

      DbSubOp dbop = { kOpReadExist, kOpMatchNone, kOpInOutNone };
      result_code = UpdateConfigDB(lc_vbrpm_ckv, req->datatype, UNC_OP_READ,
                             dmi, &dbop, MAINTBL);
      DELETE_IF_NOT_NULL(lc_vbrpm_ckv);
      if (UPLL_RC_ERR_INSTANCE_EXISTS == result_code) {
        continue;
      } else if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
        break;
      } else {
        UPLL_LOG_ERROR("Failed to read vbr_portmap");
        return result_code;
      }
    }

    // Create entry in ca_vbr_portmap_tbl
    UPLL_LOG_TRACE("VbrPortMap create from vLink %s",
                   vbr_pm_ckv->ToStrAll().c_str());
    SET_USER_DATA_FLAGS(vbr_pm_ckv, BOUNDARY_VBRPORTMAP_FLAG)
    result_code = CreateCandidateMo(req, vbr_pm_ckv, dmi);
  }

  return result_code;
}

upll_rc_t VbrPortMapMoMgr::BoundaryVbrPortmapDelete(ConfigKeyVal *db_vlink,
                                                    ConfigKeyVal *vbr_pm_ckv,
                                                    IpcReqRespHeader *req,
                                                    DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;

  upll_rc_t result_code = UPLL_RC_SUCCESS;
  uint8_t   flags       = 0;

  TcConfigMode config_mode = TC_CONFIG_INVALID;
  std::string cfg_vtn_name = "";
  //  Get acquired config mode
  result_code = GetConfigModeInfo(req, config_mode, cfg_vtn_name);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetConfigMode failed");
    return result_code;
  }

  pfcdrv_val_vbr_portmap_t *vbr_pm_val =
          reinterpret_cast<pfcdrv_val_vbr_portmap_t*>(GetVal(vbr_pm_ckv));
  vbr_pm_val->vbrpm.label = reinterpret_cast<val_vlink_t*>(
                          GetVal(db_vlink))->label;
  vbr_pm_val->vbrpm.valid[UPLL_IDX_LABEL_VBRPM] = UNC_VF_VALID;

  vbr_pm_val->vbrpm.label_type = reinterpret_cast<val_vlink_t*>(
                          GetVal(db_vlink))->label_type;
  vbr_pm_val->vbrpm.valid[UPLL_IDX_LABEL_TYPE_VBRPM] = UNC_VF_VALID;

  DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutFlag };
  // Checks the existance of vbr_portmap entry in ca_vbr_portmap_tbl
  // with old label value
  result_code = ReadConfigDB(vbr_pm_ckv, req->datatype, UNC_OP_READ, dbop,
      dmi, MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_TRACE("Failed to read vlanmap info from DB");
    return result_code;
  }

  /* For update config DB changed to kOpNotRead */
  dbop.readop = kOpNotRead;

  // Resets Boundary vlanmap flag in vlanmap entry
  // Case1 : All user refers the same vBridge portmap
  // Case2 : Multiple boundaries refers same vBridge portmap
  GET_USER_DATA_FLAGS(vbr_pm_ckv, flags);
  if (((flags & USER_VBRPORTMAP_FLAG) && vbr_pm_val->bdry_ref_count > 0)||
      vbr_pm_val->bdry_ref_count > 1) {
    uint8_t flag = 0;
    //  If vbr_portmap is refered by user and only one boundary case
    //  reset boundary bit in vbr_portmap flag
    if (vbr_pm_val->bdry_ref_count == 1) {
      GET_USER_DATA_FLAGS(vbr_pm_ckv, flag);
      flag &= ~BOUNDARY_VBRPORTMAP_FLAG;
      SET_USER_DATA_FLAGS(vbr_pm_ckv, flag);
      vbr_pm_val->valid[PFCDRV_IDX_VBRPM_BDRY_REF_COUNT] =
                  UNC_VF_VALID_NO_VALUE;
    }

    // Decrements bdry_ref_count
    vbr_pm_val->bdry_ref_count--;
    //  update flag and boundary ref count
    result_code = UpdateConfigDB(vbr_pm_ckv, req->datatype, UNC_OP_UPDATE,
                                 dmi, &dbop, config_mode,
                                 cfg_vtn_name, MAINTBL);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("Ref count DB update failed");
      return result_code;
    }
    return result_code;
  }

  // If vbr portmap is not refered my more than boundary or user and
  // boundary invoke DeleteMo() to deletes vbr portmap entry from db
  UPLL_LOG_TRACE("VbrPortMap delete from vLink %s",
                   vbr_pm_ckv->ToStrAll().c_str());
  result_code = DeleteMo(req, vbr_pm_ckv, dmi);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Failed to delete boundary-mapped vBridge portmap");
    return result_code;
  }
  return result_code;
}

upll_rc_t VbrPortMapMoMgr::BoundaryVbrPortmapUpdate(ConfigKeyVal *ikey,
                                                    ConfigKeyVal *db_vlink,
                                                    ConfigKeyVal *vbr_pm_ckv,
                                                    IpcReqRespHeader *req,
                                                    DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;

  upll_rc_t result_code = UPLL_RC_SUCCESS;
  uint8_t   flags       = 0;

  TcConfigMode config_mode = TC_CONFIG_INVALID;
  std::string cfg_vtn_name = "";
  //  Get acquired config mode
  result_code = GetConfigModeInfo(req, config_mode, cfg_vtn_name);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetConfigMode failed");
    return result_code;
  }

  if (reinterpret_cast<val_vlink_t*>(
      GetVal(ikey))->label == reinterpret_cast<val_vlink_t*>(
      GetVal(db_vlink))->label) {
    return UPLL_RC_SUCCESS;
  }

  DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutFlag };

  pfcdrv_val_vbr_portmap_t *vbr_pm_val =
          reinterpret_cast<pfcdrv_val_vbr_portmap_t*>(GetVal(vbr_pm_ckv));
  //  populate vbr_pm_ckv label value with existing db_vlink label value
  //  used for autogenerate portmap_id if it refers by multiple boundary
  //  or user and boundary
  vbr_pm_val->vbrpm.label = reinterpret_cast<val_vlink_t*>(
                          GetVal(db_vlink))->label;
  vbr_pm_val->vbrpm.valid[UPLL_IDX_LABEL_VBRPM] = UNC_VF_VALID;
  vbr_pm_val->vbrpm.label_type = reinterpret_cast<val_vlink_t*>(
                          GetVal(db_vlink))->label_type;
  vbr_pm_val->vbrpm.valid[UPLL_IDX_LABEL_TYPE_VBRPM] = UNC_VF_VALID;
  // Checks the existance of vbr_portmap entry in ca_vbr_portmap_tbl
  result_code = ReadConfigDB(vbr_pm_ckv, req->datatype, UNC_OP_READ, dbop,
      dmi, MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_TRACE("Failed to read vlanmap info from DB");
    return result_code;
  }

  /* For update config DB changed to kOpNotRead */
  dbop.readop = kOpNotRead;

  GET_USER_DATA_FLAGS(vbr_pm_ckv, flags);

  key_vbr_portmap_t *vbr_pm_key = reinterpret_cast<key_vbr_portmap_t*>(
      vbr_pm_ckv->get_key());
  uint8_t org_portmap_id[kMaxLenPortMapName + 1];
  uuu::upll_strncpy(org_portmap_id, vbr_pm_key->portmap_id,
                    kMaxLenPortMapName + 1);
  if (((flags & USER_VBRPORTMAP_FLAG) && vbr_pm_val->bdry_ref_count > 0) ||
      (vbr_pm_val->bdry_ref_count > 1)) {
    uint8_t flag = 0;
    //  when portmap is refered by user and only one boundary
    //  reset the bdry bit in vbr_portmap flag and
    //  bdry_ref_count value is zero
    if (vbr_pm_val->bdry_ref_count == 1) {
      GET_USER_DATA_FLAGS(vbr_pm_ckv, flag);
      flag &= ~BOUNDARY_VBRPORTMAP_FLAG;
      SET_USER_DATA_FLAGS(vbr_pm_ckv, flag);
      vbr_pm_val->valid[PFCDRV_IDX_VBRPM_BDRY_REF_COUNT] =
          UNC_VF_VALID_NO_VALUE;
    }

    // Decrements bdry_ref_count
    vbr_pm_val->bdry_ref_count--;
    //  update bdry_ref_count and flag in vbr_portmap
    result_code = UpdateConfigDB(vbr_pm_ckv, req->datatype, UNC_OP_UPDATE,
                          dmi, &dbop, config_mode, cfg_vtn_name, MAINTBL);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("Ref count DB update failed");
      return result_code;
    }

    vbr_pm_val->vbrpm.label = reinterpret_cast<val_vlink_t*>(
        GetVal(ikey))->label;
    DbSubOp r_dbop = { kOpReadSingle, kOpMatchCtrlr | kOpMatchDomain,
      kOpInOutFlag };
    vbr_pm_val->valid[PFCDRV_IDX_VBRPM_BDRY_REF_COUNT] = UNC_VF_INVALID;
    vbr_pm_key->portmap_id[0] = 0;
    result_code = ReadConfigDB(vbr_pm_ckv, req->datatype, UNC_OP_READ, r_dbop,
                               dmi, MAINTBL);
    if (result_code != UPLL_RC_SUCCESS &&
        result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      UPLL_LOG_TRACE("Failed to read vbrportmap info from DB");
      return result_code;
    } else if (result_code == UPLL_RC_SUCCESS) {
      //  Increment bdry_ref_count
      vbr_pm_val->bdry_ref_count++;
      vbr_pm_val->valid[PFCDRV_IDX_VBRPM_BDRY_REF_COUNT] = UNC_VF_VALID;

      // Already portmap entry is there for same logical_port_id,label,
      // controller and domain. Increment bdry_ref_count and set boundary
      // configured bit in vbr_pm_ckv flag
      //  update flag and bdry_ref count

      // Set boundary configured bit in vbr_pm_ckv flag
      DbSubOp dbop2 = { kOpNotRead, kOpMatchNone, kOpInOutFlag };
      GET_USER_DATA_FLAGS(vbr_pm_ckv, flags);
      flags |= BOUNDARY_VBRPORTMAP_FLAG;
      SET_USER_DATA_FLAGS(vbr_pm_ckv, flags);
      result_code = UpdateConfigDB(vbr_pm_ckv, req->datatype, UNC_OP_UPDATE,
                           dmi, &dbop2, config_mode, cfg_vtn_name, MAINTBL);
      if (UPLL_RC_SUCCESS != result_code) {
        UPLL_LOG_DEBUG("bdry_ref_count update failed");
        return result_code;
      }
      return UPLL_RC_SUCCESS;
    } else {
      while (1) {
        struct timeval _timeval;
        struct timezone _timezone;
        gettimeofday(&_timeval, &_timezone);
        std::stringstream ss;
        ss << _timeval.tv_sec << _timeval.tv_usec;
        std::string unique_id = ss.str();
        std::string portmap_id("vlpm_");
        portmap_id += unique_id;

        // populate portmap_id in vbr_pm_ckv
        key_vbr_portmap *vbr_pm_key =
            reinterpret_cast<key_vbr_portmap*>(vbr_pm_ckv->get_key());
        uuu::upll_strncpy(vbr_pm_key->portmap_id, portmap_id.c_str(),
                          (kMaxLenPortMapName + 1));

        ConfigKeyVal *lc_vbrpm_ckv = NULL;
        result_code = GetChildConfigKey(lc_vbrpm_ckv, vbr_pm_ckv);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_ERROR("Failed to get vbr_portmap config key val");
          return result_code;
        }

        DbSubOp dbop1 = { kOpReadExist, kOpMatchNone, kOpInOutNone };
        result_code = UpdateConfigDB(lc_vbrpm_ckv, req->datatype, UNC_OP_READ,
                                     dmi, &dbop1, MAINTBL);
        DELETE_IF_NOT_NULL(lc_vbrpm_ckv);
        if (UPLL_RC_ERR_INSTANCE_EXISTS == result_code) {
          continue;
        } else if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
          break;
        } else {
          UPLL_LOG_ERROR("Failed to read vbr_portmap");
          return result_code;
        }
      }

      vbr_pm_val->vbrpm.label = reinterpret_cast<val_vlink_t*>(
          GetVal(ikey))->label;
      SET_USER_DATA_FLAGS(vbr_pm_ckv, BOUNDARY_VBRPORTMAP_FLAG);
      vbr_pm_val->valid[PFCDRV_IDX_VBRPM_BDRY_REF_COUNT] = UNC_VF_VALID;
      vbr_pm_val->bdry_ref_count++;

      // Create entry in ca_vbr_portmap_tbl
      UPLL_LOG_TRACE("VbrPortMap create from vLink %s",
                     vbr_pm_ckv->ToStrAll().c_str());
      result_code = CreateCandidateMo(req, vbr_pm_ckv, dmi);
      return result_code;
    }
  }

  //  check same vbr_portmap entry already there in vbr_portmap_tbl
  //  if exists update boundary_ref_count and flag and delete old entry
  //  if not exist invoke update mo
  vbr_pm_val->vbrpm.label = reinterpret_cast<val_vlink_t*>(
      GetVal(ikey))->label;
  DbSubOp r_dbop = { kOpReadSingle, kOpMatchCtrlr | kOpMatchDomain,
    kOpInOutFlag };
  vbr_pm_val->valid[PFCDRV_IDX_VBRPM_BDRY_REF_COUNT] = UNC_VF_INVALID;
  vbr_pm_key->portmap_id[0] = 0;
  result_code = ReadConfigDB(vbr_pm_ckv, req->datatype, UNC_OP_READ, r_dbop,
                             dmi, MAINTBL);
  if (result_code != UPLL_RC_SUCCESS &&
      result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    UPLL_LOG_TRACE("Failed to read vbrportmap info from DB");
    return result_code;
  } else if (result_code == UPLL_RC_SUCCESS) {
    //  Increment bdry_ref_count
    vbr_pm_val->bdry_ref_count++;
    vbr_pm_val->valid[PFCDRV_IDX_VBRPM_BDRY_REF_COUNT] = UNC_VF_VALID;

    // Already portmap entry is there for same logical_port_id,label,
    // controller and domain. Increment bdry_ref_count and set boundary
    // configured bit in vbr_pm_ckv flag
    //  update flag and bdry_ref count

    // Set boundary configured bit in vbr_pm_ckv flag
    DbSubOp dbop2 = { kOpNotRead, kOpMatchNone, kOpInOutFlag };
    GET_USER_DATA_FLAGS(vbr_pm_ckv, flags);
    flags |= BOUNDARY_VBRPORTMAP_FLAG;
    SET_USER_DATA_FLAGS(vbr_pm_ckv, flags);
    result_code = UpdateConfigDB(vbr_pm_ckv, req->datatype, UNC_OP_UPDATE,
                         dmi, &dbop2, config_mode, cfg_vtn_name, MAINTBL);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("bdry_ref_count update failed");
      return result_code;
    }

    // delete old entry
    vbr_pm_val->vbrpm.label = reinterpret_cast<val_vlink_t*>(
              GetVal(db_vlink))->label;
    uuu::upll_strncpy(vbr_pm_key->portmap_id, org_portmap_id,
                    kMaxLenPortMapName + 1);
    UPLL_LOG_TRACE("VbrPortMap delete from vLink %s",
                   vbr_pm_ckv->ToStrAll().c_str());
    result_code = DeleteMo(req, vbr_pm_ckv, dmi);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Failed to delete boundary-mapped vBridge portmap");
      return result_code;
    }
  } else {
    //  received vbr_portmap is not refered by more than boundary and user
    //  update label value
    UPLL_LOG_TRACE("VbrPortMap update from vLink %s",
                   vbr_pm_ckv->ToStrAll().c_str());
    uuu::upll_strncpy(vbr_pm_key->portmap_id, org_portmap_id,
                    kMaxLenPortMapName + 1);
    result_code = UpdateMo(req, vbr_pm_ckv, dmi);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Boundary vbrportmap update failed. error:%u",
                     result_code);
      return result_code;
    }
  }

  return result_code;
}

upll_rc_t VbrPortMapMoMgr::MergeValidate(unc_key_type_t keytype,
                                         const char *ctrlr_id,
                                         ConfigKeyVal *ikey,
                                         DalDmlIntf *dmi,
                                         upll_import_type import_type) {
  UPLL_FUNC_TRACE;

  if (!ikey || !ikey->get_key() || !(strlen(reinterpret_cast<const char *>
                                            (ctrlr_id)))) {
    UPLL_LOG_TRACE("Input is NULL");
    return UPLL_RC_ERR_GENERIC;
  }

  upll_rc_t    result_code      = UPLL_RC_SUCCESS;
  ConfigKeyVal *conv_vbr_ckv    = NULL;
  ConfigKeyVal *vbr_portmap_ckv = NULL;
  VbrMoMgr *vbr_mgr = reinterpret_cast<VbrMoMgr *>(
      const_cast<MoManager *>(GetMoManager(UNC_KT_VBRIDGE)));
  if (!vbr_mgr) {
    UPLL_LOG_ERROR("Invalid mgr");
    return UPLL_RC_ERR_GENERIC;
  }

  //  Get vBridge convert ConfigKeyVal from VTN ConfigKeyVal
  result_code = vbr_mgr->GetChildConvertConfigKey(conv_vbr_ckv, ikey);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_TRACE("GetChildConfigKey Failed");
    return result_code;
  }

  DbSubOp dbop = { kOpReadMultiple, kOpMatchNone, kOpInOutCtrlr |
                                 kOpInOutDomain};
  //  Here getting full key of converted vBridge
  result_code = vbr_mgr->ReadConfigDB(conv_vbr_ckv, UPLL_DT_IMPORT,
                UNC_OP_READ, dbop, dmi, CONVERTTBL);
  if (UPLL_RC_SUCCESS != result_code) {
    DELETE_IF_NOT_NULL(conv_vbr_ckv);
    return result_code;
  }

  if (import_unified_exists_ == false) {
    //  Validate unified network existance in UNC running configuration
    result_code = IsUnwExists(dmi, TC_CONFIG_GLOBAL);
    if (result_code != UPLL_RC_SUCCESS) {
      if (result_code == UPLL_RC_ERR_EXPAND) {
        result_code = UPLL_RC_ERR_MERGE_CONFLICT;
        UPLL_LOG_INFO("No unified network configuration in UNC running DB");
      } else {
        UPLL_LOG_ERROR("Unified network existance check failed");
      }
      DELETE_IF_NOT_NULL(conv_vbr_ckv);
      return result_code;
    }

    // set this variable to true, this validation is not required for
    // next vBridge
    import_unified_exists_ = true;
  }

  ConfigKeyVal *vbr_travel = conv_vbr_ckv;
  std::multimap<std::string, std::string> import_domain_map;
  while (vbr_travel) {
    //  Get vbrPortmap ConfigKeyVal from convert vBridge ConfigKeyVal
    result_code = GetChildConfigKey(vbr_portmap_ckv, vbr_travel);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_TRACE("GetChildConfigKey Failed");
      DELETE_IF_NOT_NULL(conv_vbr_ckv);
      return result_code;
    }

    DbSubOp dbop1 = { kOpReadMultiple, kOpMatchCtrlr | kOpMatchDomain,
      kOpInOutNone };
    //  Here getting all vbr_portmap ConfigKeyVal from converted vbridge
    //  ConfigKeyVal
    result_code = ReadConfigDB(vbr_portmap_ckv, UPLL_DT_IMPORT, UNC_OP_READ,
                               dbop1, dmi, MAINTBL);
    if (UPLL_RC_SUCCESS != result_code) {
      DELETE_IF_NOT_NULL(vbr_portmap_ckv);
      DELETE_IF_NOT_NULL(conv_vbr_ckv);
      return result_code;
    }

    //  Domain validation
    controller_domain_t ctrlr_dom;
    ctrlr_dom.ctrlr = NULL;
    ctrlr_dom.domain = NULL;
    GET_USER_DATA_CTRLR_DOMAIN(vbr_travel, ctrlr_dom);
    std::multimap<std::string, std::string>::iterator it =
        import_domain_map.find(reinterpret_cast<char*>(ctrlr_dom.ctrlr));

    // verify in map domain is validated already,If not validated already
    // validate received controller and domain is leaf in UNC running db.
    if (it == import_domain_map.end() ||
        strcmp(reinterpret_cast<char*>(ctrlr_dom.domain),
                                  (it->second).c_str())) {
      unc::upll::domain_util::DomainUtil domain_util_obj;
      result_code = domain_util_obj.IsDomainLeaf(
          reinterpret_cast<char*>(ctrlr_dom.ctrlr),
          reinterpret_cast<char*>(ctrlr_dom.domain), dmi, UPLL_DT_RUNNING);
      if (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE &&
          result_code != UPLL_RC_SUCCESS) {
        if (result_code == UPLL_RC_ERR_CFG_SEMANTIC) {
          UPLL_LOG_INFO("Received leaf domain from imported controller"
                        " is not leaf in UNC");
          result_code = UPLL_RC_ERR_MERGE_CONFLICT;
        }
        ikey->ResetWith(vbr_portmap_ckv);
        DELETE_IF_NOT_NULL(conv_vbr_ckv);
        DELETE_IF_NOT_NULL(vbr_portmap_ckv);
        return result_code;
      }
      //  insert in to map to avoid duplicate validation
      import_domain_map.insert(std::pair<std::string, std::string>(
              reinterpret_cast<char*>(ctrlr_dom.ctrlr),
              reinterpret_cast<char*>(ctrlr_dom.domain)));
    }

    //  Below validations performed by using vBridge convert ConfigKeyVal.
    //
    //  case1: If same vBID is used in different unified vbridge on the same VTN
    //         in candidate
    //  case2: When the given vbr_portmap is transparent and if there are
    //         already existing unified vBridges for that VTN in candidate.
    //  case3: When the given vbr_portmap is not transparent and if there is
    //         already existing vbr_portmap which is transparent in that VTN in
    //         candidate
    //  case4: When the given vbr_portmap is transparent and if there are
    //         already existing vbr_portmaps which are not transaparent in that
    //         VTN in candidate (it is verified if case2 above is verified)
    //
    //  case2,3,4 is enough to perform for each convert vBridge ConfigKeyVal,
    //  because if port-map is transparent and vBID also transparent.To avoid
    //  repeated step for each portmap
    result_code = vbr_mgr->MergeValidateConvertVbridge(vbr_travel,
                           UPLL_DT_CANDIDATE, dmi, ctrlr_id, import_type);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Converted vBridge merge validation failed"
                     ". result_code = %d", result_code);
      //  if vbridge validation is failed ikey reset to all vbr_portmap
      //  ConfigKeyVal
      ikey->ResetWith(vbr_portmap_ckv);
      DELETE_IF_NOT_NULL(vbr_portmap_ckv);
      DELETE_IF_NOT_NULL(conv_vbr_ckv);
      return result_code;
    }
    DELETE_IF_NOT_NULL(vbr_portmap_ckv);
    vbr_travel = vbr_travel->get_next_cfg_key_val();
  }
  DELETE_IF_NOT_NULL(conv_vbr_ckv);

  return result_code;
}

upll_rc_t VbrPortMapMoMgr::PartialMergeValidate(unc_key_type_t keytype,
                                                const char *ctrlr_id,
                                                ConfigKeyVal *err_ckv,
                                                DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  ConfigKeyVal *run_ckv = NULL;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  if (!ctrlr_id || !err_ckv) {
    UPLL_LOG_TRACE("Invalid input");
    return UPLL_RC_ERR_GENERIC;
  }

  //  Get vbridge portmap ConfigKeyVal
  result_code = GetChildConfigKey(run_ckv, NULL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_TRACE("GetChildConfigKey failed");
    return result_code;
  }

  SET_USER_DATA_CTRLR(run_ckv, ctrlr_id);
  DbSubOp dbop = { kOpReadMultiple, kOpMatchCtrlr, kOpInOutFlag };
  //  Get all running vbridge portmap information for the controller
  //  to be merge
  result_code = ReadConfigDB(run_ckv, UPLL_DT_RUNNING, UNC_OP_READ, dbop,
                             dmi, MAINTBL);
  if (UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code &&
      UPLL_RC_SUCCESS != result_code) {
    delete run_ckv;
    UPLL_LOG_TRACE("ReadConfigDB failed %d", result_code);
    return result_code;
  }
  if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
    //  In Running there is no vbridge portmap information so return success
    delete run_ckv;
    return UPLL_RC_SUCCESS;
  }
  ConfigKeyVal *start_ckv = run_ckv;
  while (run_ckv) {
    pfcdrv_val_vbr_portmap_t *run_vbr_pm_val =
        reinterpret_cast<pfcdrv_val_vbr_portmap_t*>(GetVal(run_ckv));
    //  If logical_port_id and label is valid in running, validate whether
    //  the same entry exist in import db
    if (run_vbr_pm_val &&
        run_vbr_pm_val->vbrpm.valid[UPLL_IDX_LOGICAL_PORT_ID_VBRPM] ==
        UNC_VF_VALID &&
        run_vbr_pm_val->vbrpm.valid[UPLL_IDX_LABEL_VBRPM] == UNC_VF_VALID) {
      ConfigKeyVal *import_ckv = NULL;

      result_code = DupConfigKeyVal(import_ckv, run_ckv);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Failed to get convert vbr portmap ConfigKeyVal");
        delete start_ckv;
        return result_code;
      }
      pfcdrv_val_vbr_portmap_t *import_vbr_pm_val =
        reinterpret_cast<pfcdrv_val_vbr_portmap_t*>(GetVal(import_ckv));
      import_vbr_pm_val->valid[PFCDRV_IDX_VBRPM_BDRY_REF_COUNT] =
          UNC_VF_INVALID;

      DbSubOp dbop1 = { kOpReadSingle, kOpMatchNone, kOpInOutFlag };
      result_code = ReadConfigDB(import_ckv, UPLL_DT_IMPORT, UNC_OP_READ,
                                 dbop1, dmi, MAINTBL);
      if (result_code != UPLL_RC_SUCCESS) {
        if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE)
          result_code = UPLL_RC_ERR_MERGE_CONFLICT;
        err_ckv->ResetWith(import_ckv);
        delete import_ckv;
        delete start_ckv;
        return result_code;
      }

      //  Update the Boundary ref count in import and rename flag
      //  for the imported vlan map
      import_vbr_pm_val->bdry_ref_count = run_vbr_pm_val->bdry_ref_count;
      import_vbr_pm_val->valid[PFCDRV_IDX_VBRPM_BDRY_REF_COUNT] =
          run_vbr_pm_val->valid[PFCDRV_IDX_VBRPM_BDRY_REF_COUNT];
      uint8_t import_flag = 0x00;
      uint8_t run_flag = 0x00;
      GET_USER_DATA_FLAGS(import_ckv, import_flag);
      GET_USER_DATA_FLAGS(run_ckv, run_flag);
      import_flag |= run_flag;
      SET_USER_DATA_FLAGS(import_ckv, import_flag);

      DbSubOp dbop2 = { kOpNotRead, kOpMatchNone, kOpInOutFlag };
      result_code = UpdateConfigDB(import_ckv, UPLL_DT_IMPORT, UNC_OP_UPDATE,
                                   dmi, &dbop2, TC_CONFIG_GLOBAL, "", MAINTBL);
      if (UPLL_RC_SUCCESS != result_code) {
        UPLL_LOG_DEBUG("UpdateConfigDB failed %d", result_code);
        delete start_ckv;
        delete import_ckv;
        return result_code;
      }

      DELETE_IF_NOT_NULL(import_ckv);
    }
    run_ckv = run_ckv->get_next_cfg_key_val();
  }
  DELETE_IF_NOT_NULL(start_ckv);

  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrPortMapMoMgr::ValidateVtnRename(ConfigKeyVal *org_vtn_ckv,
                                             ConfigKeyVal *rename_vtn_ckv,
                                             DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;

  upll_rc_t    result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *conv_vbr_ckv    = NULL;

  if (!org_vtn_ckv || !org_vtn_ckv->get_key() ||
      !rename_vtn_ckv || !rename_vtn_ckv->get_key()) {
    UPLL_LOG_TRACE("Input is NULL");
    return UPLL_RC_ERR_GENERIC;
  }

  VbrMoMgr *vbr_mgr = reinterpret_cast<VbrMoMgr *>(
      const_cast<MoManager *>(GetMoManager(UNC_KT_VBRIDGE)));

  //  Get VbrPortmap ConfigKeyVal from VTN ConfigKeyVal
  result_code = vbr_mgr->GetChildConvertConfigKey(conv_vbr_ckv, org_vtn_ckv);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_TRACE("GetChildConfigKey Failed");
    return result_code;
  }

  DbSubOp dbop = { kOpReadMultiple, kOpMatchNone, kOpInOutCtrlr |
                                 kOpInOutDomain};
  //  Here getting fll key of converted vBridge
  result_code = vbr_mgr->ReadConfigDB(conv_vbr_ckv, UPLL_DT_IMPORT,
                UNC_OP_READ, dbop, dmi, CONVERTTBL);
  if (UPLL_RC_SUCCESS != result_code) {
    if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code)
      result_code = UPLL_RC_SUCCESS;

    DELETE_IF_NOT_NULL(conv_vbr_ckv);
    return result_code;
  }
  uint8_t *vtn_rename = reinterpret_cast<key_vtn_t*>(
      rename_vtn_ckv->get_key())->vtn_name;


  ConfigKeyVal *vbr_travel = conv_vbr_ckv;
  while (vbr_travel) {
    //  Below validations performed by using vBridge convert ConfigKeyVal.
    //
    //  case1: if same vBID is used in different KT_VBRIDGE on the same VTN
    //  case2: When the given vbr_portmap is transparent and if there are
    //         already existing unified vBridges for that VTN.
    //  case3: When the given vbr_portmap is not transparent and if there is
    //         already existing vbr_portmap which is transparent in that VTN
    //  case4: When the given vbr_portmap is transparent and if there are
    //         already existing vbr_portmaps which are not
    //         transparent in that VTN
    //
    //  case2,3,4 is enough to perform for each convert vBridge ConfigKeyVal,
    //  because if port-map is transparent and vBID also transparent.To avoid
    //  repeated step for each portmap
    uuu::upll_strncpy(reinterpret_cast<key_vbr_portmap_t*>(
              vbr_travel->get_key())->vbr_key.vtn_key.vtn_name,
          vtn_rename, (kMaxLenVtnName + 1));
    result_code = vbr_mgr->MergeValidateConvertVbridge(vbr_travel,
                           UPLL_DT_IMPORT, dmi, NULL, UPLL_IMPORT_TYPE_FULL);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Converted vBridge merge validation failed."
                     " result_code = %d", result_code);
      DELETE_IF_NOT_NULL(conv_vbr_ckv);
      return result_code;
    }
    vbr_travel = vbr_travel->get_next_cfg_key_val();
  }
  DELETE_IF_NOT_NULL(conv_vbr_ckv);

  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrPortMapMoMgr::IsKeyInUse(upll_keytype_datatype_t dt_type,
                                      const ConfigKeyVal *ckv, bool *in_use,
                                      DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  DbSubOp dbop = {kOpReadSingle, kOpMatchNone, kOpInOutNone};
  key_ctr *ctr = reinterpret_cast<key_ctr *>(ckv->get_key());

  //  validate input
  if (!ctr || !strlen(reinterpret_cast<char *>(ctr->controller_name))) {
    UPLL_LOG_DEBUG("Controller Name invalid");
    return UPLL_RC_ERR_GENERIC;
  }

  ConfigKeyVal *vbr_pm_ckv = NULL;
  //  populate vbr_pm config key val with received controller name
  result_code = GetChildConfigKey(vbr_pm_ckv, NULL);
  if (result_code != UPLL_RC_SUCCESS) {
    return result_code;
  }
  pfcdrv_val_vbr_portmap_t *val = reinterpret_cast<pfcdrv_val_vbr_portmap_t *>(
      ConfigKeyVal::Malloc(sizeof(pfcdrv_val_vbr_portmap_t)));
  uuu::upll_strncpy(val->vbrpm.controller_id,
                    ctr->controller_name, kMaxLenCtrlrId + 1);
  val->vbrpm.valid[UPLL_IDX_CONTROLLER_ID_VBRPM] = UNC_VF_VALID;
  vbr_pm_ckv->AppendCfgVal(IpctSt::kIpcStPfcdrvValVbrPortMap, val);

  // set in_use as true if received controller is found in vbr_pm tbl
  result_code = ReadConfigDB(vbr_pm_ckv, dt_type, UNC_OP_READ,
                             dbop, dmi, MAINTBL);
  *in_use = (result_code == UPLL_RC_SUCCESS) ? true : false;
  DELETE_IF_NOT_NULL(vbr_pm_ckv);
  return (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE)
           ? UPLL_RC_SUCCESS : result_code;
}

upll_rc_t VbrPortMapMoMgr::UpdateVnodeVal(ConfigKeyVal *rename_info_ckv,
                                          DalDmlIntf *dmi,
                                          upll_keytype_datatype_t data_type,
                                          bool &no_rename) {
  UPLL_FUNC_TRACE;

  upll_rc_t   result_code = UPLL_RC_SUCCESS;

  if (!rename_info_ckv) return UPLL_RC_ERR_GENERIC;
  key_rename_vnode_info *rename_info_ =
      reinterpret_cast<key_rename_vnode_info *>(rename_info_ckv->get_key());
  if (!rename_info_) {
    UPLL_LOG_DEBUG("Rename Info is Empty");
    return UPLL_RC_ERR_GENERIC;
  }

  ConfigKeyVal *ck_vbid = NULL;
  result_code = GetVbIdChildConfigKey(ck_vbid, NULL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetVbIdChildConfigKey failed with result_code ");
    return UPLL_RC_ERR_GENERIC;
  }
  key_vbid_label *vbid_key =
      reinterpret_cast<key_vbid_label *>(ck_vbid->get_key());
  uuu::upll_strncpy(vbid_key->vtn_key.vtn_name, rename_info_->old_unc_vtn_name,
                    (kMaxLenVtnName + 1));
  DbSubOp dbop1 = { kOpNotRead, kOpMatchNone, kOpInOutNone };
  result_code = UpdateConfigDB(ck_vbid, data_type, UNC_OP_DELETE, dmi, &dbop1,
                               TC_CONFIG_GLOBAL, "", VBIDTBL);
  DELETE_IF_NOT_NULL(ck_vbid);
  if (result_code != UPLL_RC_SUCCESS &&
      result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    return result_code;
  }

  VbrMoMgr *mgr = reinterpret_cast<VbrMoMgr *>(const_cast<MoManager *>(
        GetMoManager(UNC_KT_VBRIDGE)));
  if (!mgr) {
    UPLL_LOG_INFO("Invalid mgr");
    return UPLL_RC_ERR_GENERIC;
  }

  ConfigKeyVal *old_vbr_ckv = NULL;
  result_code = mgr->GetChildConvertConfigKey(old_vbr_ckv, NULL);
  if (result_code != UPLL_RC_SUCCESS) {
    return result_code;
  }
  key_convert_vbr_t *old_convert_vbr_key =
      reinterpret_cast<key_convert_vbr_t*>(old_vbr_ckv->get_key());
  uuu::upll_strncpy(old_convert_vbr_key->vbr_key.vtn_key.vtn_name,
           rename_info_->old_unc_vtn_name, (kMaxLenVtnName+1));
  DbSubOp dbop = { kOpReadMultiple, kOpMatchNone, kOpInOutNone };
  result_code = mgr->ReadConfigDB(old_vbr_ckv, data_type, UNC_OP_READ,
                                  dbop, dmi, CONVERTTBL);
  if (result_code != UPLL_RC_SUCCESS &&
      result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    DELETE_IF_NOT_NULL(old_vbr_ckv);
    UPLL_LOG_INFO("Vbridge converttbl read config db failed");
    return result_code;
  }
  if (result_code == UPLL_RC_SUCCESS) {
    ConfigKeyVal *conv_vbr_start_ckv = old_vbr_ckv;
    while (conv_vbr_start_ckv) {
      // For each convert vBridge entry alloc VBID in candidate
      key_convert_vbr_t *conv_vbr_key =
          reinterpret_cast<key_convert_vbr_t*>(conv_vbr_start_ckv->get_key());
      val_convert_vbr_t *convert_vbr_val =
          reinterpret_cast<val_convert_vbr_t*>(GetVal(conv_vbr_start_ckv));
      if (convert_vbr_val->label != ANY_VLAN_ID) {
        result_code = AllocVbid(conv_vbr_key->vbr_key.vtn_key.vtn_name,
                                convert_vbr_val->label, dmi,
                                data_type, TC_CONFIG_GLOBAL, "");
        if (result_code != UPLL_RC_SUCCESS &&
            result_code != UPLL_RC_ERR_INSTANCE_EXISTS) {
          DELETE_IF_NOT_NULL(old_vbr_ckv);
          return result_code;
        }
      }
      conv_vbr_start_ckv = conv_vbr_start_ckv->get_next_cfg_key_val();
    }
  }
  DELETE_IF_NOT_NULL(old_vbr_ckv);

  ConfigKeyVal *ck_vbid2 = NULL;
  result_code = GetVbIdChildConfigKey(ck_vbid2, NULL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetVbIdChildConfigKey failed with result_code ");
    return UPLL_RC_ERR_GENERIC;
  }
  key_vbid_label *vbid_key2 =
      reinterpret_cast<key_vbid_label *>(ck_vbid2->get_key());
  uuu::upll_strncpy(vbid_key2->vtn_key.vtn_name, rename_info_->new_unc_vtn_name,
                    (kMaxLenVtnName + 1));
  result_code = UpdateConfigDB(ck_vbid2, data_type, UNC_OP_DELETE, dmi, &dbop1,
                               TC_CONFIG_GLOBAL, "", VBIDTBL);
  DELETE_IF_NOT_NULL(ck_vbid2);
  if (result_code != UPLL_RC_SUCCESS &&
      result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    return result_code;
  }

  ConfigKeyVal *vbr_ckv = NULL;
  result_code = mgr->GetChildConvertConfigKey(vbr_ckv, NULL);
  if (result_code != UPLL_RC_SUCCESS) {
    return result_code;
  }
  key_convert_vbr_t *convert_vbr_key =
      reinterpret_cast<key_convert_vbr_t*>(vbr_ckv->get_key());
  uuu::upll_strncpy(convert_vbr_key->vbr_key.vtn_key.vtn_name,
         rename_info_->new_unc_vtn_name, (kMaxLenVtnName+1));
  result_code = mgr->ReadConfigDB(vbr_ckv, data_type, UNC_OP_READ,
                                  dbop, dmi, CONVERTTBL);
  if (result_code != UPLL_RC_SUCCESS &&
      result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    DELETE_IF_NOT_NULL(vbr_ckv);
    return result_code;
  } else if (result_code == UPLL_RC_SUCCESS) {
    ConfigKeyVal *conv_vbr_start_ckv = vbr_ckv;
    while (conv_vbr_start_ckv) {
      // For each convert vBridge entry alloc VBID in candidate
      key_convert_vbr_t *conv_vbr_key = reinterpret_cast<key_convert_vbr_t*>(
                          conv_vbr_start_ckv->get_key());
      val_convert_vbr_t *conv_vbr_val = reinterpret_cast<val_convert_vbr_t*>(
                          GetVal(conv_vbr_start_ckv));
      if (conv_vbr_val->label != ANY_VLAN_ID) {
        result_code = AllocVbid(conv_vbr_key->vbr_key.vtn_key.vtn_name,
                                conv_vbr_val->label, dmi, data_type,
                                TC_CONFIG_GLOBAL, "");
        if (result_code != UPLL_RC_SUCCESS &&
            result_code != UPLL_RC_ERR_INSTANCE_EXISTS) {
          DELETE_IF_NOT_NULL(vbr_ckv);
          return result_code;
        }
      }
      conv_vbr_start_ckv = conv_vbr_start_ckv->get_next_cfg_key_val();
    }
  }
  DELETE_IF_NOT_NULL(vbr_ckv);

  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrPortMapMoMgr::ResetVBrPortmapOperStatus(ConfigKeyVal *ckey,
                           DalDmlIntf *dmi, uint8_t oper_status) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal* ckv_vbr_pm = NULL;
  result_code = GetChildConfigKey(ckv_vbr_pm, ckey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Error in GetChildConfigKey : %d", result_code);
    return result_code;
  }
  controller_domain_t ctrlr_dom = {NULL, NULL};
  GET_USER_DATA_CTRLR_DOMAIN(ckey, ctrlr_dom);
  SET_USER_DATA_CTRLR_DOMAIN(ckv_vbr_pm, ctrlr_dom);
  ConfigVal *cv_val = NULL;
  AllocVal(cv_val, UPLL_DT_STATE, MAINTBL);
  ckv_vbr_pm->SetCfgVal(cv_val);

  DbSubOp dbop = {kOpReadMultiple, kOpMatchCtrlr|kOpMatchDomain,
                  kOpInOutFlag};
  result_code = ReadConfigDB(ckv_vbr_pm, UPLL_DT_STATE, UNC_OP_READ, dbop,
                             dmi, MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    DELETE_IF_NOT_NULL(ckv_vbr_pm);
    if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      result_code = UPLL_RC_SUCCESS;
    } else {
      UPLL_LOG_ERROR("Error in ReadConfigDB : %d", result_code);
    }
    return result_code;
  }
  while (ckv_vbr_pm) {
    ConfigKeyVal *tmp_ckv_vbrpm = ckv_vbr_pm->get_next_cfg_key_val();
    ckv_vbr_pm->set_next_cfg_key_val(NULL);
    pfcdrv_val_vbr_portmap_t *pfcdrv_val_vbrpm =
            reinterpret_cast<pfcdrv_val_vbr_portmap*>(GetVal(ckv_vbr_pm));
    val_db_vbr_portmap_st_t* vbrpm_st =
            reinterpret_cast<val_db_vbr_portmap_st_t*>(GetStateVal(ckv_vbr_pm));
    if (!pfcdrv_val_vbrpm || !vbrpm_st) {
      UPLL_LOG_DEBUG("val/st_val is empty");
      DELETE_IF_NOT_NULL(ckv_vbr_pm);
      DELETE_IF_NOT_NULL(tmp_ckv_vbrpm);
      return UPLL_RC_ERR_GENERIC;
    }
    uint32_t orig_status = vbrpm_st->down_count;
    if (pfcdrv_val_vbrpm->vbrpm.valid[2] == UNC_VF_VALID) {
      uint8_t state(UPPL_LOGICAL_PORT_OPER_UNKNOWN);
      uint8_t *tmp_logical_port_id = pfcdrv_val_vbrpm->vbrpm.logical_port_id,
              *ctrlr = NULL;
      GET_USER_DATA_CTRLR(ckey, ctrlr);
      bool result(false);
      result = uuc::CtrlrMgr::GetInstance()->GetLogicalPortSt(
               reinterpret_cast<const char*>(ctrlr),
               reinterpret_cast<const char*>(tmp_logical_port_id),
               state);
      if (result == false) {
      /* port is already set to UNKNOWN. So return */
        DELETE_IF_NOT_NULL(ckv_vbr_pm);
        ckv_vbr_pm = tmp_ckv_vbrpm;
        continue;
      }
      vbrpm_st->down_count &= ~PORT_UNKNOWN;
      if (state == UPPL_LOGICAL_PORT_OPER_DOWN) {
        /* Set PORT_FAULT flag */
        vbrpm_st->down_count |= PORT_FAULT;
      }
      vbrpm_st->vbr_portmap_val_st.valid[0] = UNC_VF_VALID;
      if (vbrpm_st->down_count != orig_status) {
        result_code = UpdateOperStatus(ckv_vbr_pm, dmi, kCommit,
                                       UNC_OP_UPDATE, true, true);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("Returning error %d\n", result_code);
          DELETE_IF_NOT_NULL(tmp_ckv_vbrpm);
          DELETE_IF_NOT_NULL(ckv_vbr_pm);
          return result_code;
        }
      }
    }
    DELETE_IF_NOT_NULL(ckv_vbr_pm);
    ckv_vbr_pm = tmp_ckv_vbrpm;
  }
  return result_code;
}

upll_rc_t VbrPortMapMoMgr::UpdateUnwSdLabelCountInVtnModeAbort(
    TcConfigMode config_mode, string vtn_name, DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;

  upll_rc_t    result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *spd_ckv   = NULL;
  uint32_t     used_count = 0;

  uint8_t *vtnname = NULL;
  if (!vtn_name.empty()) {
    vtnname = reinterpret_cast<uint8_t *>(
        const_cast<char *>(vtn_name.c_str()));
  } else {
    UPLL_LOG_ERROR("Invalid vtn name");
    return UPLL_RC_ERR_GENERIC;
  }

  // get the used count, spd_ckv for the vtn from scratch table
  DalBindInfo db_info(uudst::kDbiSpdScratchTbl);
  UNWSpineDomainMoMgr *unw_sd_momgr =
      reinterpret_cast<unc::upll::kt_momgr::UNWSpineDomainMoMgr *>(
          const_cast<MoManager *>(GetMoManager(UNC_KT_UNW_SPINE_DOMAIN)));

  result_code = unw_sd_momgr->GetChildConfigKey(spd_ckv, NULL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_ERROR("GetChildConfigKey failed:%d", result_code);
    return result_code;
  }

  // check whether the entry exists in scratch table for the given vtn
  db_info.BindMatch(uudst::spd_scratch::kDbiVtnName, uud::kDalChar,
                    (kMaxLenVtnName + 1), vtnname);

  db_info.BindOutput(uudst::spd_scratch::kDbiUnwName,
                     uud::kDalChar, (kMaxLenUnifiedNwName + 1),
                     static_cast<key_unw_spine_domain*>(
                         spd_ckv->get_key())->unw_key.unified_nw_id);

  db_info.BindOutput(uudst::spd_scratch::kDbiUnwSpineDomainName,
                     uud::kDalChar, (kMaxLenUnwSpineID + 1),
                     static_cast<key_unw_spine_domain*>(
                         spd_ckv->get_key())->unw_spine_id);

  //  Bind Output used_count
  db_info.BindOutput(uudst::spd_scratch::kDbiUsedCount,
                     uud::kDalUint32, 1, &used_count);

  DalCursor *dal_cursor_handle = NULL;
  result_code = DalToUpllResCode(dmi->GetMultipleRecords(
          UPLL_DT_CANDIDATE, uudst::kDbiSpdScratchTbl, 0,
          &db_info, &dal_cursor_handle));
  if (result_code != UPLL_RC_SUCCESS) {
    DELETE_IF_NOT_NULL(spd_ckv);
    if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      UPLL_LOG_TRACE("No entry in scratch table");
      result_code = UPLL_RC_SUCCESS;
    } else {
      UPLL_LOG_ERROR("scratch table read failed %d", result_code);
    }
    if (dal_cursor_handle) {
      dmi->CloseCursor(dal_cursor_handle, false);
    }
    return result_code;
  }

  DbSubOp dbop = { kOpNotRead, kOpMatchNone, kOpInOutNone };
  while (1) {
    result_code = DalToUpllResCode(dmi->GetNextRecord(dal_cursor_handle));
    if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
      dmi->CloseCursor(dal_cursor_handle, false);
      DELETE_IF_NOT_NULL(spd_ckv);
      result_code = UPLL_RC_SUCCESS;
      return result_code;
    }

    // entry exists, read spd table and get used_label_count
    result_code = GetUnwSpineDomain(&spd_ckv, dmi, UPLL_DT_CANDIDATE);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_ERROR("Unw spine domain candidate table read failed:%d",
                     result_code);
      dmi->CloseCursor(dal_cursor_handle, false);
      DELETE_IF_NOT_NULL(spd_ckv);
      return result_code;
    }

    val_unw_spdom_ext* val_unw_spd  =
        reinterpret_cast<val_unw_spdom_ext*>(GetVal(spd_ckv));
    if (!val_unw_spd) {
      dmi->CloseCursor(dal_cursor_handle, false);
      DELETE_IF_NOT_NULL(spd_ckv);
      return result_code;
    }
    // Decrement spine domain label count
    val_unw_spd->used_label_count = val_unw_spd->used_label_count - used_count;
    val_unw_spd->valid[UPLL_IDX_SPINE_USED_LABEL_COUNT_UNWS] = UNC_VF_VALID;

    // update used count in CANDIDATE
    result_code = unw_sd_momgr->UpdateConfigDB(spd_ckv, UPLL_DT_CANDIDATE,
                                               UNC_OP_UPDATE, dmi, &dbop,
                                               TC_CONFIG_GLOBAL, "", MAINTBL);
    // remove cfg_val to store the values for next iteration
    spd_ckv->DeleteCfgVal();
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_ERROR("used_label_count update failed:%d", result_code);
      dmi->CloseCursor(dal_cursor_handle, false);
      DELETE_IF_NOT_NULL(spd_ckv);
      return result_code;
    }
  }
  return result_code;
}

}  // namespace kt_momgr
}  // namespace upll
}  // namespace unc
