/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include "vtunnel_if_momgr.hh"
#include "vtunnel_momgr.hh"
#include "ctrlr_capa_defines.hh"
#include "capa_intf.hh"
#include "upll_validation.hh"
#include "uncxx/upll_log.hh"
#define NUM_KEY_MAIN_TBL_ 5

namespace unc {
namespace upll {
namespace kt_momgr {

  BindInfo VtunnelIfMoMgr::vtunnel_if_bind_info[] = {
    { uudst::vtunnel_interface::kDbiVtnName, CFG_KEY, offsetof(key_vtunnel_if,
        vtunnel_key.vtn_key.vtn_name), uud::kDalChar, kMaxLenVtnName+1},
    { uudst::vtunnel_interface::kDbiVtunnelName, CFG_KEY,
        offsetof(key_vtunnel_if,
        vtunnel_key.vtunnel_name), uud::kDalChar, kMaxLenVnodeName+1},
    { uudst::vtunnel_interface::kDbiIfName, CFG_KEY, offsetof(key_vtunnel_if,
        if_name), uud::kDalChar, kMaxLenInterfaceName+1},
    { uudst::vtunnel_interface::kDbiDesc, CFG_VAL, offsetof(
        val_vtunnel_if, description), uud::kDalChar, 128},
    { uudst::vtunnel_interface::kDbiAdminStatus, CFG_VAL, offsetof(
        val_vtunnel_if, admin_status), uud::kDalUint8, 1},
    { uudst::vtunnel_interface::kDbiLogicalPortId, CFG_VAL, offsetof(
        val_vtunnel_if, portmap.logical_port_id), uud::kDalChar, 320},
    { uudst::vtunnel_interface::kDbiVlanId, CFG_VAL, offsetof(
        val_vtunnel_if, portmap.vlan_id), uud::kDalUint16, 1},
    { uudst::vtunnel_interface::kDbiTagged, CFG_VAL, offsetof(
        val_vtunnel_if, portmap.tagged), uud::kDalUint8, 1},
    { uudst::vtunnel_interface::kDbiOperStatus, ST_VAL,
        offsetof(val_db_vtunnel_if_st,
        vtunnel_if_val_st.oper_status), uud::kDalUint8, 1},
    { uudst::vtunnel_interface::kDbiDownCount, ST_VAL,
        offsetof(val_db_vtunnel_if_st,
        down_count), uud::kDalUint32, 1 },
    { uudst::vtunnel_interface::kDbiCtrlrName, CK_VAL, offsetof(key_user_data,
        ctrlr_id), uud::kDalChar, 32},
    { uudst::vtunnel_interface::kDbiDomainId, CK_VAL, offsetof(key_user_data,
        domain_id), uud::kDalChar, 32},
    { uudst::vtunnel_interface::kDbiFlags, CK_VAL, offsetof(key_user_data,
        flags), uud::kDalUint8, 1},
    { uudst::vtunnel_interface::kDbiValidDesc, CFG_META_VAL, offsetof(
        val_vtunnel_if, valid[UPLL_IDX_DESC_VTNL_IF]), uud::kDalUint8, 1},
    { uudst::vtunnel_interface::kDbiValidAdminStatus, CFG_DEF_VAL, offsetof(
        val_vtunnel_if, valid[UPLL_IDX_ADMIN_ST_VTNL_IF]), uud::kDalUint8, 1},
    { uudst::vtunnel_interface::kDbiValidPortMap, CFG_META_VAL, offsetof(
        val_vtunnel_if, valid[UPLL_IDX_PORT_MAP_VTNL_IF]), uud::kDalUint8, 1},
    { uudst::vtunnel_interface::kDbiValidLogicalPortId, CFG_META_VAL, offsetof
      (val_vtunnel_if, portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM]),
      uud::kDalUint8, 1},
    { uudst::vtunnel_interface::kDbiValidVlanid, CFG_META_VAL, offsetof(
        val_vtunnel_if, portmap.valid[UPLL_IDX_VLAN_ID_PM]),
    uud::kDalUint8, 1},
    { uudst::vtunnel_interface::kDbiValidTagged, CFG_META_VAL, offsetof(
        val_vtunnel_if, portmap.valid[UPLL_IDX_TAGGED_PM]),
    uud::kDalUint8, 1 },
    { uudst::vtunnel_interface::kDbiValidOperStatus, ST_META_VAL, offsetof(
        val_db_vtunnel_if_st,
        vtunnel_if_val_st.valid[UPLL_IDX_IF_OPER_STATUS_VTNLI]),
    uud::kDalUint8, 1 },
    { uudst::vtunnel_interface::kDbiCsRowstatus, CS_VAL,
        offsetof(val_vtunnel_if,
        cs_row_status), uud::kDalUint8, 1},
    { uudst::vtunnel_interface::kDbiCsDesc, CS_VAL, offsetof(val_vtunnel_if,
        cs_attr[UPLL_IDX_DESC_VTNL_IF]), uud::kDalUint8, 1},
    { uudst::vtunnel_interface::kDbiCsAdminStatus, CS_VAL,
        offsetof(val_vtunnel_if,
        cs_attr[UPLL_IDX_ADMIN_ST_VTNL_IF]), uud::kDalUint8, 1},
    { uudst::vtunnel_interface::kDbiCsPortMap, CS_VAL, offsetof(val_vtunnel_if,
        cs_attr[UPLL_IDX_PORT_MAP_VTNL_IF]), uud::kDalUint8, 1},
    { uudst::vtunnel_interface::kDbiCsLogicalPortId, CS_VAL, offsetof(
        val_vtunnel_if, portmap.cs_attr[UPLL_IDX_LOGICAL_PORT_ID_PM]),
    uud::kDalUint8, 1 },
    { uudst::vtunnel_interface::kDbiCsVlanid, CS_VAL, offsetof(
        val_vtunnel_if, portmap.cs_attr[UPLL_IDX_VLAN_ID_PM]),
    uud::kDalUint8, 1 },
    { uudst::vtunnel_interface::kDbiCsTagged, CS_VAL, offsetof(
        val_vtunnel_if, portmap.cs_attr[UPLL_IDX_TAGGED_PM]),
    uud::kDalUint8, 1 }
  };

  BindInfo VtunnelIfMoMgr::conv_vtunnel_if_bind_info[] = {
    { uudst::convert_vtunnel_interface::kDbiVtnName, CFG_KEY, offsetof(
        key_convert_vtunnel_if, convert_vtunnel_key.vtn_key.vtn_name),
        uud::kDalChar, kMaxLenVtnName+1},
    { uudst::convert_vtunnel_interface::kDbiVtunnelName, CFG_KEY,
        offsetof(key_convert_vtunnel_if,
        convert_vtunnel_key.convert_vtunnel_name), uud::kDalChar,
        kMaxLenConvertVnodeName+1},
    { uudst::convert_vtunnel_interface::kDbiIfName, CFG_KEY, offsetof(
        key_convert_vtunnel_if, convert_if_name), uud::kDalChar,
        kMaxLenInterfaceName+1},
    { uudst::convert_vtunnel_interface::kDbiOperStatus, ST_VAL, offsetof(
        val_db_vtunnel_if_st, vtunnel_if_val_st.oper_status),
        uud::kDalUint8, 1 },
    { uudst::convert_vtunnel_interface::kDbiDownCount, ST_VAL, offsetof(
        val_db_vtunnel_if_st, down_count), uud::kDalUint32, 1 },
    { uudst::convert_vtunnel_interface::kDbiRemoteCtrlrName, CFG_VAL, offsetof(
        val_convert_vtunnel_if, rem_ctrlr_name), uud::kDalChar,
        kMaxLenCtrlrId + 1 },
    { uudst::convert_vtunnel_interface::kDbiRemoteDomainId, CFG_VAL, offsetof(
        val_convert_vtunnel_if, rem_domain_id), uud::kDalChar,
        kMaxLenDomainId + 1 },
    { uudst::convert_vtunnel_interface::kDbiUnVbrName, CFG_VAL, offsetof(
        val_convert_vtunnel_if, un_vbr_name), uud::kDalChar,
        kMaxLenVnodeName+1},
    { uudst::convert_vtunnel_interface::kDbiFlags, CK_VAL, offsetof(
        key_user_data_t, flags), uud::kDalUint8, 1 },
    { uudst::convert_vtunnel_interface::kDbiValidOperStatus, ST_META_VAL,
      offsetof(val_db_vtunnel_if_st, vtunnel_if_val_st.valid[
      UPLL_IDX_IF_OPER_STATUS_VTNLI]), uud::kDalUint8, 1 },
    { uudst::convert_vtunnel_interface::kDbiValidUnVbrName, CFG_META_VAL,
      offsetof(val_convert_vtunnel_if,
      valid[UPLL_IDX_RCTRLR_NAME_CONV_VTNL_IF]), uud::kDalUint8, 1 },
    { uudst::convert_vtunnel_interface::kDbiValidUnVbrName, CFG_META_VAL,
      offsetof(val_convert_vtunnel_if,
      valid[UPLL_IDX_RDOMAIN_ID_CONV_VTNL_IF]), uud::kDalUint8, 1 },
    { uudst::convert_vtunnel_interface::kDbiValidUnVbrName, CFG_META_VAL,
      offsetof(val_convert_vtunnel_if,
      valid[UPLL_IDX_UN_VBR_NAME_CONV_VTNL_IF]), uud::kDalUint8, 1 },
    { uudst::convert_vtunnel_interface::kDbiCsRowstatus, CS_VAL, offsetof(
        val_convert_vtunnel_if, cs_row_status), uud::kDalUint8, 1 },
    { uudst::convert_vtunnel_interface::kDbiCsRemoteCtrlrName, CS_VAL, offsetof(
        val_convert_vtunnel_if, cs_attr[0]), uud::kDalUint8, 1 },
    { uudst::convert_vtunnel_interface::kDbiCsRemoteDomainId, CS_VAL,
        offsetof(val_convert_vtunnel_if, cs_attr[0]), uud::kDalUint8, 1 },
    { uudst::convert_vtunnel_interface::kDbiCsUnVbrName, CS_VAL, offsetof(
        val_convert_vtunnel_if, cs_attr[0]), uud::kDalUint8, 1 }
  };

VtunnelIfMoMgr::VtunnelIfMoMgr() {
  UPLL_FUNC_TRACE;
  Table *tbl = new Table(uudst::kDbiVtunnelIfTbl, UNC_KT_VTUNNEL_IF,
      vtunnel_if_bind_info,
      IpctSt::kIpcStKeyVtunnelIf, IpctSt::kIpcStValVtunnelIf,
      uudst::vtunnel_interface::kDbiVtunnelIfNumCols);
  ntable = MAX_MOMGR_TBLS;
  table = new Table *[ntable]();
  table[MAINTBL] = tbl;
  table[CONVERTTBL] = new Table(uudst::kDbiConvertVtunnelIfTbl, UNC_KT_VTUNNEL,
                      conv_vtunnel_if_bind_info,
                      IpctSt::kIpcStKeyConvertVtunnelIf,
                      IpctSt::kIpcInvalidStNum,
                      (uudst::convert_vtunnel_interface::
                      kDbiConvertVtunnelIfNumCols));

  table[RENAMETBL] = NULL;
  table[CTRLRTBL] = NULL;
  nchild = 0;
  child = NULL;
#ifdef _STANDALONE_
  SetMoManager(UNC_KT_VTUNNEL_IF, reinterpret_cast<MoMgr *>(this));
#endif
}

upll_rc_t VtunnelIfMoMgr::ValidateAttribute(ConfigKeyVal *ikey,
                                            DalDmlIntf *dmi,
                                            IpcReqRespHeader *req) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  if (!ikey || (ikey->get_key_type() != UNC_KT_VTUNNEL_IF)
            || !(ikey->get_cfg_val()))
    return UPLL_RC_ERR_CFG_SYNTAX;
  val_vtunnel_if *vtunnelif_val = static_cast<val_vtunnel_if *>(GetVal(ikey));
  if (!vtunnelif_val) {
    if (req->operation == UNC_OP_CREATE) {
      UPLL_LOG_DEBUG("Val Structure is Null");
      return UPLL_RC_SUCCESS;
    } else {
      UPLL_LOG_DEBUG("Val structure is must");
      return UPLL_RC_ERR_GENERIC;
    }
  }
  result_code = IsLogicalPortAndVlanIdInUse<val_vtunnel_if_t>
                                  (ikey, dmi, req);
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
upll_rc_t VtunnelIfMoMgr::IsLogicalPortAndVlanIdInUse(ConfigKeyVal *ikey,
                                                      DalDmlIntf *dmi,
                                                      IpcReqRespHeader *req) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  if (!ikey || (ikey->get_key_type() != UNC_KT_VTUNNEL_IF)
            || !(ikey->get_cfg_val()))
    return UPLL_RC_ERR_CFG_SYNTAX;
  val_vtunnel_if *vtunnelif_val = static_cast<val_vtunnel_if *>(GetVal(ikey));
  if (!vtunnelif_val) {
    return UPLL_RC_ERR_GENERIC;
  }
  if (vtunnelif_val->valid[UPLL_IDX_PORT_MAP_VTNL_IF] == UNC_VF_VALID) {
    if (vtunnelif_val->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM]
                                                          == UNC_VF_VALID &&
        vtunnelif_val->portmap.valid[UPLL_IDX_VLAN_ID_PM] == UNC_VF_VALID) {
      key_vtunnel_if *vtunnel_key_if = static_cast<key_vtunnel_if *>
                 (ConfigKeyVal::Malloc(sizeof(key_vtunnel_if)));
      val_vtunnel_if *vtunnelif_val_filter = static_cast<val_vtunnel_if *>
                  (ConfigKeyVal::Malloc(sizeof(val_vtunnel_if)));
      vtunnelif_val_filter->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] =
                                                                UNC_VF_VALID;
      vtunnelif_val_filter->portmap.valid[UPLL_IDX_VLAN_ID_PM] = UNC_VF_VALID;
      uuu::upll_strncpy(vtunnelif_val_filter->portmap.logical_port_id,
                        vtunnelif_val->portmap.logical_port_id,
                        kMaxLenLogicalPortId+1);
      vtunnelif_val_filter->portmap.vlan_id = vtunnelif_val->portmap.vlan_id;
      ConfigKeyVal *vtunnelif_ckv = new ConfigKeyVal(UNC_KT_VTUNNEL_IF,
                              IpctSt::kIpcStKeyVtunnelIf, vtunnel_key_if, NULL);
      vtunnelif_ckv->AppendCfgVal(IpctSt::kIpcStValVtunnelIf,
                                                   vtunnelif_val_filter);

      SET_USER_DATA(vtunnelif_ckv, ikey);
      // Read from the DB
      DbSubOp dbop = { kOpReadMultiple, kOpMatchCtrlr | kOpMatchDomain,
                                                        kOpInOutFlag};
      result_code = ReadConfigDB(vtunnelif_ckv, UPLL_DT_CANDIDATE, UNC_OP_READ,
                                 dbop, dmi, MAINTBL);
      if (result_code == UPLL_RC_SUCCESS) {
      // check different interface has the same port and lan id
        ConfigKeyVal *tmp = vtunnelif_ckv;
        while (tmp) {
          if (!memcmp((ikey->get_key()), (tmp->get_key()),
                                          sizeof(key_vtunnel_if))) {
            UPLL_LOG_TRACE("Looking on the Same key");
          } else {
            UPLL_LOG_DEBUG("Same port Id and VlanId is used on different "
                           "Vtunnel Interface within same VTN/Controller");
            delete vtunnelif_ckv;
            vtunnelif_ckv = tmp = NULL;
            return UPLL_RC_ERR_CFG_SEMANTIC;
          }
          tmp = tmp->get_next_cfg_key_val();
        }
      } else if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
        result_code = UPLL_RC_SUCCESS;
      } else {
        delete vtunnelif_ckv;
        vtunnelif_ckv = NULL;
        return result_code;
      }
      delete vtunnelif_ckv;
      vtunnelif_ckv = NULL;
    }
  }
  UPLL_LOG_TRACE("PortId and VlanId is not used on any Vtunnel Interface");
  return result_code;
}
#endif

bool VtunnelIfMoMgr::IsValidKey(void *key, uint64_t index, MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  upll_rc_t ret_val = UPLL_RC_SUCCESS;
  if (tbl == MAINTBL) {
    key_vtunnel_if *if_key = reinterpret_cast<key_vtunnel_if *>(key);
    switch (index) {
      case uudst::vtunnel_interface::kDbiVtnName:
        ret_val = ValidateKey(reinterpret_cast<char *>
            (if_key->vtunnel_key.vtn_key.vtn_name),
            kMinLenVtnName, kMaxLenVtnName);
        if (ret_val != UPLL_RC_SUCCESS) {
          UPLL_LOG_TRACE("VTN Name is not valid(%d)", ret_val);
          return false;
        }
        break;
      case uudst::vtunnel_interface::kDbiVtunnelName:
        ret_val = ValidateKey(reinterpret_cast<char *>
            (if_key->vtunnel_key.vtunnel_name),
            kMinLenVnodeName, kMaxLenVnodeName);
        if (ret_val != UPLL_RC_SUCCESS) {
          UPLL_LOG_TRACE("Vtunnel Name is not valid(%d)", ret_val);
          return false;
        }
        break;
      case uudst::vtunnel_interface::kDbiIfName:
        ret_val = ValidateKey(reinterpret_cast<char *>
            (if_key->if_name),
            kMinLenInterfaceName, kMaxLenInterfaceName);
        if (ret_val != UPLL_RC_SUCCESS) {
          UPLL_LOG_TRACE("Vtunnel IF Name is not valid(%d)", ret_val);
          return false;
        }
        break;
      default:
        UPLL_LOG_TRACE("Wrong Index");
        return false;
    }
  } else if (tbl == CONVERTTBL) {
    key_convert_vtunnel_if *convert_if_key =
      reinterpret_cast<key_convert_vtunnel_if *>(key);
    switch (index) {
      case uudst::convert_vtunnel_interface::kDbiVtnName:
        ret_val = ValidateKey(reinterpret_cast<char *>
            (convert_if_key->convert_vtunnel_key.vtn_key.vtn_name),
            kMinLenVtnName, kMaxLenVtnName);
        if (ret_val != UPLL_RC_SUCCESS) {
          UPLL_LOG_TRACE("VTN Name is not valid(%d)", ret_val);
          return false;
        }

        break;
      case uudst::convert_vtunnel_interface::kDbiVtunnelName:
        ret_val = ValidateKey(reinterpret_cast<char *>
            (convert_if_key->convert_vtunnel_key.convert_vtunnel_name),
            kMinLenConvertVnodeName, kMaxLenConvertVnodeName);
        if (ret_val != UPLL_RC_SUCCESS) {
          UPLL_LOG_TRACE("Convert Vtunnel Name is not valid(%d)", ret_val);
          return false;
        }
        break;
      case uudst::convert_vtunnel_interface::kDbiIfName:
        ret_val = ValidateKey(reinterpret_cast<char *>
            (convert_if_key->convert_if_name),
            kMinLenInterfaceName, kMaxLenInterfaceName);
        if (ret_val != UPLL_RC_SUCCESS) {
          UPLL_LOG_TRACE("Convert Vtunnel IF Name is not valid(%d)", ret_val);
          return false;
        }
        break;
      default:
        UPLL_LOG_TRACE("Wrong Index");
        return false;
    }
  }
  return true;
}

upll_rc_t VtunnelIfMoMgr::GetValid(void *val, uint64_t indx, uint8_t *&valid,
    upll_keytype_datatype_t dt_type, MoMgrTables tbl ) {
  UPLL_FUNC_TRACE;
  if (val == NULL) return UPLL_RC_ERR_GENERIC;
  if (tbl == MAINTBL) {
    switch (indx) {
      case uudst::vtunnel_interface::kDbiOperStatus:
        valid = &(reinterpret_cast<val_vtunnel_if_st *>(val))->
                valid[UPLL_IDX_IF_OPER_STATUS_VTNLI];
        break;
      case uudst::vtunnel_interface::kDbiDownCount:
        valid = NULL;
        break;
      case uudst::vtunnel_interface::kDbiAdminStatus:
        valid = &(reinterpret_cast<val_vtunnel_if *>(val))->
                valid[UPLL_IDX_ADMIN_ST_VTNL_IF];
        break;
      case uudst::vtunnel_interface::kDbiDesc:
        valid = &(reinterpret_cast<val_vtunnel_if *>(val))->
                valid[UPLL_IDX_DESC_VTNL_IF];
        break;
      case uudst::vtunnel_interface::kDbiValidPortMap:
        valid = &(reinterpret_cast<val_vtunnel_if *>(val))->
          valid[UPLL_IDX_PORT_MAP_VTNL_IF];
        break;
      case uudst::vtunnel_interface::kDbiLogicalPortId:
        valid = &(reinterpret_cast<val_vtunnel_if *>(val))->portmap.
          valid[UPLL_IDX_LOGICAL_PORT_ID_PM];
        break;
      case uudst::vtunnel_interface::kDbiVlanId:
        valid = &(reinterpret_cast<val_vtunnel_if *>(val))->portmap.
          valid[UPLL_IDX_VLAN_ID_PM];
        break;
      case uudst::vtunnel_interface::kDbiTagged:
        valid = &(reinterpret_cast<val_vtunnel_if *>(val))->portmap.
          valid[UPLL_IDX_TAGGED_PM];
        break;
      default:
        return UPLL_RC_ERR_GENERIC;
    }
  } else if (tbl == CONVERTTBL) {
    switch (indx) {
      case uudst::convert_vtunnel_interface::kDbiRemoteCtrlrName:
        valid = &(reinterpret_cast<val_convert_vtunnel_if *>(val))->
          valid[UPLL_IDX_RCTRLR_NAME_CONV_VTNL_IF];
        break;
      case uudst::convert_vtunnel_interface::kDbiRemoteDomainId:
        valid = &(reinterpret_cast<val_convert_vtunnel_if *>(val))->
          valid[UPLL_IDX_RDOMAIN_ID_CONV_VTNL_IF];
        break;
      case uudst::convert_vtunnel_interface::kDbiUnVbrName:
        valid = &(reinterpret_cast<val_convert_vtunnel_if *>(val))->
          valid[UPLL_IDX_UN_VBR_NAME_CONV_VTNL_IF];
        break;
      default:
        return UPLL_RC_ERR_GENERIC;
    }
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtunnelIfMoMgr::GetChildConfigKey(ConfigKeyVal *&okey,
    ConfigKeyVal *parent_key) {
  UPLL_FUNC_TRACE;
  bool cfgval_ctrlr = false;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  key_vtunnel_if *vtunnel_key_if = static_cast<key_vtunnel_if *>
    (ConfigKeyVal::Malloc(sizeof(key_vtunnel_if)));
  if (vtunnel_key_if == NULL) return UPLL_RC_ERR_GENERIC;
  void *pkey;
  if (parent_key == NULL) {
    if (okey) delete okey;
    okey = new ConfigKeyVal(UNC_KT_VTUNNEL_IF, IpctSt::kIpcStKeyVtunnelIf,
        vtunnel_key_if, NULL);
    return UPLL_RC_SUCCESS;
  } else {
    pkey = parent_key->get_key();
  }
  if (!pkey) {
    FREE_IF_NOT_NULL(vtunnel_key_if);
    return UPLL_RC_ERR_GENERIC;
  }
  if (okey && (okey->get_key())) {
    FREE_IF_NOT_NULL(vtunnel_key_if);
    if (okey->get_key_type() != UNC_KT_VTUNNEL_IF)
      return UPLL_RC_ERR_GENERIC;
    vtunnel_key_if = static_cast<key_vtunnel_if *>(okey->get_key());
  } else {
    okey = new ConfigKeyVal(UNC_KT_VTUNNEL_IF, IpctSt::kIpcStKeyVtunnelIf,
        vtunnel_key_if, NULL);
    if (okey == NULL) {
      FREE_IF_NOT_NULL(vtunnel_key_if);
      return UPLL_RC_ERR_GENERIC;
    }
  }
  unc_key_type_t keytype = parent_key->get_key_type();
  switch (keytype) {
    case UNC_KT_VTN:
      uuu::upll_strncpy(vtunnel_key_if->vtunnel_key.vtn_key.vtn_name,
          static_cast<key_vtn *>(pkey)->vtn_name, (kMaxLenVtnName+1));
      *(vtunnel_key_if->vtunnel_key.vtunnel_name) = *"";
      break;
    case UNC_KT_VTUNNEL:
      uuu::upll_strncpy(vtunnel_key_if->vtunnel_key.vtn_key.vtn_name,
          static_cast<key_vtunnel *>
          (pkey)->vtn_key.vtn_name,
          (kMaxLenVtnName+1));
      uuu::upll_strncpy(vtunnel_key_if->vtunnel_key.vtunnel_name,
          static_cast<key_vtunnel *>(pkey)->vtunnel_name,
          (kMaxLenVnodeName+1));
      break;
    case UNC_KT_VTUNNEL_IF:
      uuu::upll_strncpy(vtunnel_key_if->vtunnel_key.vtn_key.vtn_name,
                        static_cast<key_vtunnel_if *>
                        (pkey)->vtunnel_key.vtn_key.vtn_name,
                        (kMaxLenVtnName+1));
      uuu::upll_strncpy(vtunnel_key_if->vtunnel_key.vtunnel_name,
                        static_cast<key_vtunnel_if *>
                        (pkey)->vtunnel_key.vtunnel_name,
                        (kMaxLenVnodeName+1));
      uuu::upll_strncpy(vtunnel_key_if->if_name,
                        static_cast<key_vtunnel_if *>(pkey)->if_name,
                        (kMaxLenInterfaceName+1));
      break;
    case UNC_KT_VLINK:
      {
        uint8_t *vnode_name, *if_name;
        val_vlink *vlink_val = static_cast<val_vlink *>(GetVal(parent_key));
        if (!vlink_val) {
          free(vtunnel_key_if);
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
        uuu::upll_strncpy(vtunnel_key_if->vtunnel_key.vtn_key.vtn_name,
                          static_cast<key_vlink *>(pkey)->vtn_key.vtn_name,
                          (kMaxLenVtnName+1));
        if (vnode_name)
          uuu::upll_strncpy(vtunnel_key_if->vtunnel_key.vtunnel_name,
                            vnode_name, (kMaxLenVnodeName+1));
        if (if_name)
          uuu::upll_strncpy(vtunnel_key_if->if_name, if_name,
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

upll_rc_t VtunnelIfMoMgr::GetParentConfigKey(ConfigKeyVal *&okey,
    ConfigKeyVal *ikey) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  key_vtunnel_if *pkey = reinterpret_cast<key_vtunnel_if *>
                         ((ikey)?ikey->get_key():NULL);
  if (!pkey) return UPLL_RC_ERR_GENERIC;

  if (ikey->get_key_type() != UNC_KT_VTUNNEL_IF)
    return UPLL_RC_ERR_GENERIC;
  key_vtunnel *vtunnel_key = reinterpret_cast<key_vtunnel *>
                             (ConfigKeyVal::Malloc(sizeof(key_vtunnel)));
  if (!vtunnel_key) return UPLL_RC_ERR_GENERIC;
  uuu::upll_strncpy(vtunnel_key->vtn_key.vtn_name,
                   (pkey)->vtunnel_key.vtn_key.vtn_name,
                   (kMaxLenVtnName+1));
  uuu::upll_strncpy(vtunnel_key->vtunnel_name, (pkey)->vtunnel_key.vtunnel_name,
                    (kMaxLenVnodeName+1));
  if (okey) delete okey;
  okey = new ConfigKeyVal(UNC_KT_VTUNNEL, IpctSt::kIpcStKeyVtunnel,
                          vtunnel_key, NULL);
  if (okey == NULL) {
    FREE_IF_NOT_NULL(vtunnel_key);
    result_code = UPLL_RC_ERR_GENERIC;
  } else {
    SET_USER_DATA(okey, ikey);
  }
  return result_code;
}


upll_rc_t VtunnelIfMoMgr::AllocVal(ConfigVal *&ck_val,
    upll_keytype_datatype_t dt_type, MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  void *val;
  if (ck_val != NULL) return UPLL_RC_ERR_GENERIC;
  switch (tbl) {
    case MAINTBL:
      val = reinterpret_cast<void *>(ConfigKeyVal::Malloc
            (sizeof(val_vtunnel_if)));
      if (!val) return UPLL_RC_ERR_GENERIC;
      ck_val = new ConfigVal(IpctSt::kIpcStValVtunnelIf, val);
      if (!ck_val) {
        FREE_IF_NOT_NULL(reinterpret_cast<val_vtunnel_if *>(val));
        return UPLL_RC_ERR_GENERIC;
      }
      if (dt_type == UPLL_DT_STATE) {
        val = reinterpret_cast<void *>(ConfigKeyVal::Malloc
              (sizeof(val_db_vtunnel_if_st)));
        if (!val) {
          delete ck_val;
          return UPLL_RC_ERR_GENERIC;
        }
        ConfigVal *ck_nxtval = new ConfigVal(IpctSt::kIpcStValVtunnelIfSt, val);
        if (!ck_nxtval) {
          delete ck_val;
          FREE_IF_NOT_NULL(reinterpret_cast<val_vtunnel_if_st *>(val));
          return UPLL_RC_ERR_GENERIC;
        }
        ck_val->AppendCfgVal(ck_nxtval);
      }
      break;
    case CONVERTTBL:
      val = reinterpret_cast<void *>
        (ConfigKeyVal::Malloc(sizeof(val_convert_vtunnel_if)));
      ck_val = new ConfigVal(IpctSt::kIpcStValConvertVtunnelIf, val);
      break;
    default:
      val = NULL;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtunnelIfMoMgr::DupConfigKeyVal(
    ConfigKeyVal *&okey, ConfigKeyVal *&req, MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  if (req == NULL) return UPLL_RC_ERR_GENERIC;
  if (okey != NULL) return UPLL_RC_ERR_GENERIC;
  if (req->get_key_type() != UNC_KT_VTUNNEL_IF)
    return UPLL_RC_ERR_GENERIC;
  ConfigVal *tmp1 = NULL, *tmp = (req)->get_cfg_val();

  if (tmp) {
    if (tbl == MAINTBL) {
      val_vtunnel_if *ival = reinterpret_cast<val_vtunnel_if *>(GetVal(req));
      val_vtunnel_if *vtunnel_val_if = reinterpret_cast<val_vtunnel_if *>
          (ConfigKeyVal::Malloc(sizeof(val_vtunnel_if)));
      if (!vtunnel_val_if) return UPLL_RC_ERR_GENERIC;
      memcpy(vtunnel_val_if, ival, sizeof(val_vtunnel_if));
      tmp1 = new ConfigVal(IpctSt::kIpcStValVtunnelIf, vtunnel_val_if);
    } else if (tbl == CONVERTTBL) {
      val_convert_vtunnel_if *ival =
        reinterpret_cast<val_convert_vtunnel_if *>(GetVal(req));
      if (ival == NULL) {
         UPLL_LOG_ERROR("convert val is NULL");
         return UPLL_RC_ERR_GENERIC;
      }
      val_convert_vtunnel_if *conv_vtunnel_if_val =
        ConfigKeyVal::Malloc<val_convert_vtunnel_if>();
      memcpy(conv_vtunnel_if_val, ival, sizeof(val_convert_vtunnel_if));
      tmp1 = new ConfigVal(IpctSt::kIpcStValConvertVtunnelIf,
                           conv_vtunnel_if_val);
    }
    tmp = tmp->get_next_cfg_val();
  };
  if (tmp) {
    if (tbl == MAINTBL || tbl == CONVERTTBL) {
      val_db_vtunnel_if_st *ival = reinterpret_cast<val_db_vtunnel_if_st *>
        (tmp->get_val());
      val_db_vtunnel_if_st *val_db_vtunnel_if =
          reinterpret_cast<val_db_vtunnel_if_st *>
          (ConfigKeyVal::Malloc(sizeof(val_db_vtunnel_if_st)));
      if (!val_db_vtunnel_if) {
        delete tmp1;
        return UPLL_RC_ERR_GENERIC;
      }
      memcpy(val_db_vtunnel_if, ival, sizeof(val_db_vtunnel_if_st));
      ConfigVal *tmp2 = new ConfigVal(IpctSt::kIpcStValVtunnelIfSt,
                                      val_db_vtunnel_if);
      if (!tmp2) {
        delete tmp1;
        FREE_IF_NOT_NULL(val_db_vtunnel_if);
        return UPLL_RC_ERR_GENERIC;
      }
      tmp1->AppendCfgVal(tmp2);
    }
  }
  void *tkey = (req != NULL)?(req)->get_key():NULL;
  if (tbl == MAINTBL) {
    key_vtunnel_if *ikey = reinterpret_cast<key_vtunnel_if *>(tkey);
    key_vtunnel_if *vtunnel_if_key = reinterpret_cast<key_vtunnel_if *>
      (ConfigKeyVal::Malloc(sizeof(key_vtunnel_if)));
    memcpy(vtunnel_if_key, ikey, sizeof(key_vtunnel_if));
    okey = new ConfigKeyVal(UNC_KT_VTUNNEL_IF, IpctSt::kIpcStKeyVtunnelIf,
        vtunnel_if_key, tmp1);
  } else if (tbl == CONVERTTBL) {
    key_convert_vtunnel_if *ikey =
        reinterpret_cast<key_convert_vtunnel_if *>(tkey);
    key_convert_vtunnel_if *conv_vtunnel_if_key =
        ConfigKeyVal::Malloc<key_convert_vtunnel_if>();
    memcpy(conv_vtunnel_if_key, ikey, sizeof(key_convert_vtunnel_if));
    okey = new ConfigKeyVal(UNC_KT_VTUNNEL_IF,
                            IpctSt::kIpcStKeyConvertVtunnelIf,
                            conv_vtunnel_if_key, tmp1);
  } else {
    UPLL_LOG_DEBUG("Received tbl name is not supported");
    DELETE_IF_NOT_NULL(tmp1);
    return UPLL_RC_ERR_GENERIC;
  }
  SET_USER_DATA(okey, req);
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtunnelIfMoMgr::UpdateConfigVal(ConfigKeyVal *ikey,
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
  val_vtunnel_if *vtunnelif_val = static_cast<val_vtunnel_if *>
                                             (GetVal(ikey));
  if (!vtunnelif_val) {
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
  if (vtunnelif_val->valid[UPLL_IDX_PORT_MAP_VTNL_IF] == UNC_VF_VALID ||
      vtunnelif_val->valid[UPLL_IDX_PORT_MAP_VTNL_IF]
                                                 == UNC_VF_VALID_NO_VALUE) {
    if (flag & VIF_TYPE) {
      DELETE_IF_NOT_NULL(okey);
      UPLL_LOG_DEBUG("Interface is linked/bounded with Vlink. "
                     "Could not update Portmap");
      return UPLL_RC_ERR_CFG_SEMANTIC;
    }
  }
  if (vtunnelif_val->valid[UPLL_IDX_PORT_MAP_VTNL_IF] ==
                           UNC_VF_VALID_NO_VALUE) {
    vtunnelif_val->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] =
                                               UNC_VF_VALID_NO_VALUE;
    vtunnelif_val->portmap.valid[UPLL_IDX_VLAN_ID_PM] = UNC_VF_VALID_NO_VALUE;
    vtunnelif_val->portmap.valid[UPLL_IDX_TAGGED_PM] = UNC_VF_VALID_NO_VALUE;
    uuu::upll_strncpy(vtunnelif_val->portmap.logical_port_id, "\0", 1);
    vtunnelif_val->portmap.vlan_id = 0;
    vtunnelif_val->portmap.tagged = UPLL_VLAN_UNTAGGED;
  }
  DELETE_IF_NOT_NULL(okey);
  return result_code;
}

upll_rc_t VtunnelIfMoMgr::AdaptValToVtnService(ConfigKeyVal *ikey,
                                               AdaptType adapt_type) {
  UPLL_FUNC_TRACE;
  if (!ikey) {
    UPLL_LOG_DEBUG("Invalid ikey");
    return UPLL_RC_ERR_GENERIC;
  }
  key_vtunnel_if *vtunif_key =
      reinterpret_cast<key_vtunnel_if*>(ikey->get_key());
  while (ikey) {
    ConfigVal *cval = ikey->get_cfg_val();
    if (!cval) {
      UPLL_LOG_DEBUG("Config Val is Null");
      return UPLL_RC_ERR_GENERIC;
    }
    while (cval) {
      if (IpctSt::kIpcStValVtunnelIf == cval->get_st_num()) {
         // set admin status to valid no value
        val_vtunnel_if *vtunnelif_val = reinterpret_cast<val_vtunnel_if *>
                                        (GetVal(ikey));
        if (vtunnelif_val->valid[UPLL_IDX_ADMIN_ST_VTNL_IF] == UNC_VF_INVALID)
          vtunnelif_val->valid[UPLL_IDX_ADMIN_ST_VTNL_IF] =
                                                 UNC_VF_VALID_NO_VALUE;

        uint8_t vlink_flag = 0;
        GET_USER_DATA_FLAGS(ikey, vlink_flag);
        UPLL_LOG_DEBUG("Interface type %d", vlink_flag);
        if (vlink_flag & VIF_TYPE)
          vtunnelif_val->valid[UPLL_IDX_PORT_MAP_VTNL_IF] = UNC_VF_INVALID;
      }
      if (IpctSt::kIpcStValVtunnelIfSt == cval->get_st_num()) {
        controller_domain ctrlr_dom = {NULL, NULL};
        GET_USER_DATA_CTRLR_DOMAIN(ikey, ctrlr_dom);
        CheckOperStatus<val_vtunnel_if_st>(
            vtunif_key->vtunnel_key.vtn_key.vtn_name,
            cval, UNC_KT_VTUNNEL_IF, ctrlr_dom);
      }
      cval = cval->get_next_cfg_val();
    }
    if (adapt_type == ADAPT_ONE) break;
    ikey = ikey->get_next_cfg_key_val();
  }
  return UPLL_RC_SUCCESS;
}

bool VtunnelIfMoMgr::FilterAttributes(void *&val1, void *val2,
                                      bool copy_to_running,
                                      unc_keytype_operation_t op) {
  UPLL_FUNC_TRACE;
  val_vtunnel_if_t *val_vtunnelif1 = reinterpret_cast<val_vtunnel_if_t *>(val1);
  /* No need to configure description in controller. */
  val_vtunnelif1->valid[UPLL_IDX_DESC_VTNL_IF] = UNC_VF_INVALID;
  if (op != UNC_OP_CREATE)
    return CompareValidValue(val1, val2, copy_to_running);
  return false;
}

bool VtunnelIfMoMgr::CompareValidValue(void *&val1, void *val2,
                                       bool copy_to_running) {
  UPLL_FUNC_TRACE;
  bool invalid_attr = true;
  val_vtunnel_if_t *val_vtunnelif1 = reinterpret_cast<val_vtunnel_if_t *>(val1);
  val_vtunnel_if_t *val_vtunnelif2 = reinterpret_cast<val_vtunnel_if_t *>(val2);
  for (unsigned int loop = 0;
       loop < sizeof(val_vtunnelif1->valid)/sizeof(uint8_t); ++loop ) {
    if (UNC_VF_INVALID == val_vtunnelif1->valid[loop]
        && UNC_VF_VALID == val_vtunnelif2->valid[loop])
      val_vtunnelif1->valid[loop] = UNC_VF_VALID_NO_VALUE;
  }
  for (unsigned int loop = 0;
       loop < sizeof(val_vtunnelif1->portmap.valid)/sizeof(uint8_t); ++loop ) {
    if (UNC_VF_INVALID == val_vtunnelif1->portmap.valid[loop]
        && UNC_VF_VALID == val_vtunnelif2->portmap.valid[loop])
      val_vtunnelif1->portmap.valid[loop] = UNC_VF_VALID_NO_VALUE;
  }
  if (val_vtunnelif1->admin_status == val_vtunnelif2->admin_status)
    val_vtunnelif1->valid[UPLL_IDX_ADMIN_ST_VTNL_IF] = UNC_VF_INVALID;
  if (copy_to_running) {
    if (UNC_VF_VALID == val_vtunnelif1->valid[UPLL_IDX_DESC_VTNL_IF]
        && UNC_VF_VALID == val_vtunnelif2->valid[UPLL_IDX_DESC_VTNL_IF]) {
      if (!strcmp(reinterpret_cast<char*>(val_vtunnelif1->description),
                  reinterpret_cast<char*>(val_vtunnelif2->description)))
        val_vtunnelif1->valid[UPLL_IDX_DESC_VTNL_IF] = UNC_VF_INVALID;
    }
    if (UNC_VF_INVALID != val_vtunnelif1->valid[UPLL_IDX_ADMIN_ST_VTNL_IF]
        && UNC_VF_INVALID != val_vtunnelif2->valid[UPLL_IDX_ADMIN_ST_VTNL_IF]) {
      if (val_vtunnelif1->admin_status == val_vtunnelif2->admin_status)
        val_vtunnelif1->valid[UPLL_IDX_ADMIN_ST_VTNL_IF] = UNC_VF_INVALID;
    }
    if (val_vtunnelif1->valid[UPLL_IDX_PORT_MAP_VTNL_IF] == UNC_VF_VALID
        && val_vtunnelif2->valid[UPLL_IDX_PORT_MAP_VTNL_IF] == UNC_VF_VALID) {
      if (memcmp(&(val_vtunnelif1->portmap), &(val_vtunnelif2->portmap),
                 sizeof(val_port_map_t))) {
        if (val_vtunnelif1->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] ==
            UNC_VF_VALID
            && val_vtunnelif2->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM]
            == UNC_VF_VALID) {
          if (!strcmp(reinterpret_cast<char *>
                      (val_vtunnelif1->portmap.logical_port_id),
                      reinterpret_cast<char *>
                      (val_vtunnelif2->portmap.logical_port_id)))
            val_vtunnelif1->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] =
                UNC_VF_INVALID;
        }
        if (val_vtunnelif1->portmap.valid[UPLL_IDX_VLAN_ID_PM] !=
            UNC_VF_INVALID
            && val_vtunnelif2->portmap.valid[UPLL_IDX_VLAN_ID_PM] !=
            UNC_VF_INVALID) {
          if (val_vtunnelif1->portmap.vlan_id ==
              val_vtunnelif2->portmap.vlan_id)
            val_vtunnelif1->portmap.valid[UPLL_IDX_VLAN_ID_PM] = UNC_VF_INVALID;
        }
        if (val_vtunnelif1->portmap.valid[UPLL_IDX_TAGGED_PM] !=
            UNC_VF_INVALID
            && val_vtunnelif2->portmap.valid[UPLL_IDX_TAGGED_PM] ==
            UNC_VF_INVALID) {
          if (val_vtunnelif1->portmap.tagged ==
                               val_vtunnelif2->portmap.tagged) {
            if (val_vtunnelif1->portmap.valid[UPLL_IDX_TAGGED_PM] ==
                val_vtunnelif2->portmap.valid[UPLL_IDX_TAGGED_PM]) {
              val_vtunnelif1->portmap.valid[UPLL_IDX_TAGGED_PM] =
                                                           UNC_VF_INVALID;
            }
          }
        }
      } else {
        val_vtunnelif1->valid[UPLL_IDX_PORT_MAP_VTNL_IF] = UNC_VF_INVALID;
        val_vtunnelif1->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] =
            UNC_VF_INVALID;
        val_vtunnelif1->portmap.valid[UPLL_IDX_VLAN_ID_PM] = UNC_VF_INVALID;
        val_vtunnelif1->portmap.valid[UPLL_IDX_TAGGED_PM] = UNC_VF_INVALID;
      }
    }
  }
  if (!copy_to_running)
    val_vtunnelif1->valid[UPLL_IDX_DESC_VTNL_IF] = UNC_VF_INVALID;
  for (unsigned int loop = 0;
       loop < sizeof(val_vtunnelif1->valid) / sizeof(uint8_t); ++loop) {
    if ((UNC_VF_VALID == (uint8_t) val_vtunnelif1->valid[loop]) ||
        (UNC_VF_VALID_NO_VALUE == (uint8_t) val_vtunnelif1->valid[loop])) {
      if (loop == UPLL_IDX_PORT_MAP_VTNL_IF) {
        for (unsigned int i = 0;
             i < sizeof(val_vtunnelif1->portmap.valid) / sizeof(uint8_t); ++i) {
          if ((UNC_VF_VALID == (uint8_t) val_vtunnelif1->portmap.valid[i]) ||
              (UNC_VF_VALID_NO_VALUE ==
               (uint8_t) val_vtunnelif1->portmap.valid[i])) {
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

upll_rc_t VtunnelIfMoMgr::UpdateConfigStatus(ConfigKeyVal *vtunnelif_keyval,
    unc_keytype_operation_t op,
    uint32_t driver_result,
    ConfigKeyVal *upd_key,
    DalDmlIntf *dmi,
    ConfigKeyVal *ctrlr_key) {
  UPLL_FUNC_TRACE;
  val_vtunnel_if_t *vtunnel_if_val =
    static_cast<val_vtunnel_if_t *>(GetVal(vtunnelif_keyval));
  if (vtunnel_if_val == NULL) return UPLL_RC_ERR_GENERIC;
  unc_keytype_configstatus_t cs_status = (driver_result == UPLL_RC_SUCCESS)?
                                          UNC_CS_APPLIED:UNC_CS_NOT_APPLIED;
  UPLL_LOG_TRACE("DriverResult %d, ConfigStatus %d", driver_result, cs_status);
  UPLL_LOG_TRACE("%s", (vtunnelif_keyval->ToStrAll()).c_str());
  val_vtunnel_if_t *val_running = static_cast<val_vtunnel_if_t *>
                                      (GetVal(upd_key));
  if (op == UNC_OP_CREATE) {
    vtunnel_if_val->cs_row_status = cs_status;
    val_db_vtunnel_if_st *vtunnel_db_valst = static_cast<val_db_vtunnel_if_st *>
      (ConfigKeyVal::Malloc(sizeof(val_db_vtunnel_if_st)));
    if (vtunnel_db_valst == NULL) return UPLL_RC_ERR_GENERIC;
    vtunnelif_keyval->AppendCfgVal(IpctSt::kIpcStValVtunnelIfSt,
                                                        vtunnel_db_valst);
#if 0
    upll_rc_t result_code = InitOperStatus<val_vtunnel_if_st,
                              val_db_vtunnel_if_st>
                             (vtunnelif_keyval,
                              vtunnel_if_val->valid[UPLL_IDX_ADMIN_ST_VTNL_IF],
                              vtunnel_if_val->admin_status,
                              vtunnel_if_val->valid[UPLL_IDX_PORT_MAP_VTNL_IF],
                              &vtunnel_if_val->portmap);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Error settiing oper status");
      return UPLL_RC_ERR_GENERIC;
    }
#else
      val_vtunnel_if_st *vnif_st = &vtunnel_db_valst->vtunnel_if_val_st;
      if (!vnif_st) {
        UPLL_LOG_DEBUG("Returning error\n");
        return UPLL_RC_ERR_GENERIC;
      }
      vnif_st->oper_status =
                   (driver_result == UPLL_RC_ERR_CTR_DISCONNECTED)?
                    UPLL_OPER_STATUS_UNKNOWN:
                    UPLL_OPER_STATUS_UP;
      vnif_st->valid[UPLL_IDX_IF_OPER_STATUS_VTEPIS] = UNC_VF_VALID;
#endif
    vtunnel_db_valst->down_count = 0;
  } else if (op == UNC_OP_UPDATE) {
    void *val = reinterpret_cast<void *>(vtunnel_if_val);
    CompareValidValue(val, GetVal(upd_key), true);
    UPLL_LOG_TRACE("%s", (upd_key->ToStrAll()).c_str());
    vtunnel_if_val->cs_row_status =
                    val_running->cs_row_status;
  } else {
    return UPLL_RC_ERR_GENERIC;
  }
  for (unsigned int loop = 0; loop <
      sizeof(vtunnel_if_val->valid) / sizeof(vtunnel_if_val->valid[0]);
      ++loop) {
    if ((UNC_VF_VALID == (uint8_t) vtunnel_if_val->valid[loop])
        || (UNC_VF_VALID_NO_VALUE == (uint8_t) vtunnel_if_val->valid[loop])) {
      if (loop == UPLL_IDX_DESC_VTNL_IF)
        vtunnel_if_val->cs_attr[loop] = UNC_CS_APPLIED;
      else
        vtunnel_if_val->cs_attr[loop] = cs_status;
    } else if ((UNC_VF_INVALID == vtunnel_if_val->valid[loop]) &&
               (UNC_OP_CREATE == op)) {
        vtunnel_if_val->cs_attr[loop] = UNC_CS_APPLIED;
    } else if ((vtunnel_if_val->valid[loop] == UNC_VF_INVALID) &&
             (UNC_OP_UPDATE == op)) {
      vtunnel_if_val->cs_attr[loop] =
                     val_running->cs_attr[loop];
    }
  }
  for (unsigned int loop = 0;
       loop < sizeof(vtunnel_if_val->portmap.valid)/
              sizeof(vtunnel_if_val->portmap.valid[0]); ++loop ) {
    if ((UNC_VF_VALID == vtunnel_if_val->portmap.valid[loop])
     || (UNC_VF_VALID_NO_VALUE == vtunnel_if_val->portmap.valid[loop]))
      vtunnel_if_val->portmap.cs_attr[loop] = cs_status;
    else if ((UNC_VF_INVALID == vtunnel_if_val->portmap.valid[loop]) &&
             (UNC_OP_CREATE == op))
      vtunnel_if_val->portmap.cs_attr[loop] = UNC_CS_APPLIED;
    else if ((UNC_VF_INVALID == vtunnel_if_val->portmap.valid[loop]) &&
             (UNC_OP_UPDATE == op))
      vtunnel_if_val->portmap.cs_attr[loop] =
                     val_running->portmap.cs_attr[loop];
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtunnelIfMoMgr::UpdateAuditConfigStatus(
    unc_keytype_configstatus_t cs_status,
    uuc::UpdateCtrlrPhase phase,
    ConfigKeyVal *&ckv_running,
    DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  val_vtunnel_if_t *val;
  val = (ckv_running != NULL)?reinterpret_cast<val_vtunnel_if_t *>
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
  for (unsigned int loop = 0; loop < sizeof(val->valid)/sizeof(uint8_t);
      ++loop ) {
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

upll_rc_t VtunnelIfMoMgr::ValidateMessage(IpcReqRespHeader *req,
    ConfigKeyVal *ikey) {
  UPLL_FUNC_TRACE;

  upll_rc_t ret_val = UPLL_RC_ERR_GENERIC;
  if (!ikey || !req || !(ikey->get_key())) {
    UPLL_LOG_DEBUG("IpcReqRespHeader or ConfigKeyVal is Null");
    return UPLL_RC_ERR_BAD_REQUEST;
  }
  unc_key_type_t ktype = ikey->get_key_type();
  if (UNC_KT_VTUNNEL_IF != ktype) {
    UPLL_LOG_DEBUG("Invalid keytype. Keytype- %d", ktype);
    return UPLL_RC_ERR_BAD_REQUEST;
  }
  if (ikey->get_st_num() != IpctSt::kIpcStKeyVtunnelIf) {
    UPLL_LOG_DEBUG("Invalid structure received.Expected struct-"
        "kIpcStKeyVtunnelIf, received struct -%s ",
        reinterpret_cast<const char *>
        (IpctSt::GetIpcStdef(ikey->get_st_num())));
    return UPLL_RC_ERR_BAD_REQUEST;
  }
  key_vtunnel_if_t *key_vtunnel_if = reinterpret_cast<key_vtunnel_if_t *>
    (ikey->get_key());

  uint32_t dt_type = req->datatype;
  uint32_t operation = req->operation;
  uint32_t option1 = req->option1;
  uint32_t option2 = req->option2;

  ret_val = ValidateVTunnelIfKey(key_vtunnel_if, operation);
  if (ret_val != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Key struct Validation failed for VTUNNEL_IF");
    return UPLL_RC_ERR_CFG_SYNTAX;
  } else {
    if ((operation == UNC_OP_UPDATE) && (dt_type == UPLL_DT_CANDIDATE)) {
      val_vtunnel_if_t *val_vtunnel_if = NULL;
      if ((ikey->get_cfg_val()) &&
          (ikey->get_cfg_val()->get_st_num() == IpctSt::kIpcStValVtunnelIf)) {
        val_vtunnel_if =
          reinterpret_cast<val_vtunnel_if_t *> (ikey->get_cfg_val()->get_val());
      }
      if (val_vtunnel_if != NULL) {
        ret_val = ValidateVTunnelIfValue(val_vtunnel_if, operation);
        if (ret_val != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("Val struct Validation failed for UPDATE op");
          return ret_val;
        }
        return UPLL_RC_SUCCESS;
      } else {
        UPLL_LOG_DEBUG("Value structure mandatory for UPDATE op");
        return UPLL_RC_ERR_BAD_REQUEST;
      }
    } else if ((operation == UNC_OP_CREATE) &&
        (dt_type == UPLL_DT_CANDIDATE)) {
      val_vtunnel_if_t *val_vtunnel_if = NULL;
      if ((ikey->get_cfg_val()) &&
          (ikey->get_cfg_val()->get_st_num() == IpctSt::kIpcStValVtunnelIf)) {
        val_vtunnel_if =
          reinterpret_cast<val_vtunnel_if_t *> (ikey->get_cfg_val()->get_val());
      }
      if (val_vtunnel_if != NULL) {
        ret_val = ValidateVTunnelIfValue(val_vtunnel_if, operation);
        if (ret_val != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("Val struct Validation failed for CREATE op");
          return ret_val;
        }
        return UPLL_RC_SUCCESS;
      } else {
        UPLL_LOG_DEBUG("Value structure is an optional for CREATE op");
        return UPLL_RC_SUCCESS;
      }
    } else if (((operation == UNC_OP_READ) ||
          (operation == UNC_OP_READ_SIBLING) ||
          (operation == UNC_OP_READ_SIBLING_BEGIN) ||
          (operation == UNC_OP_READ_SIBLING_COUNT)) &&
        ((dt_type == UPLL_DT_CANDIDATE) ||
         ((dt_type == UPLL_DT_RUNNING) ||
          (dt_type == UPLL_DT_STARTUP) ||
          (dt_type == UPLL_DT_STATE)))) {
      if (option1 == UNC_OPT1_NORMAL) {
        if (option2 == UNC_OPT2_NONE) {
          val_vtunnel_if_t *val_vtunnel_if = NULL;
          if ((ikey->get_cfg_val()) &&
              (ikey->get_cfg_val()->get_st_num() ==
               IpctSt::kIpcStValVtunnelIf)) {
            val_vtunnel_if =
              reinterpret_cast<val_vtunnel_if_t *>
              (ikey->get_cfg_val()->get_val());
          }
          if (val_vtunnel_if != NULL) {
            ret_val = ValidateVTunnelIfValue(val_vtunnel_if);
            if (ret_val != UPLL_RC_SUCCESS) {
              UPLL_LOG_DEBUG("Val struct Validation failed for READ op");
              return ret_val;
            }
            return UPLL_RC_SUCCESS;
          } else {
            UPLL_LOG_DEBUG("Value structure is an optional for READ op");
            return UPLL_RC_SUCCESS;
          }
        } else if ((option2 == UNC_OPT2_NEIGHBOR) &&
            (operation == UNC_OP_READ) &&
            (dt_type == UPLL_DT_STATE)) {
          val_vtn_neighbor_t *val_vtn_neighbor = NULL;
          if ((ikey->get_cfg_val()) && ((ikey->get_cfg_val())->get_st_num() ==
                IpctSt::kIpcStValVtnNeighbor)) {
            val_vtn_neighbor = reinterpret_cast <val_vtn_neighbor_t *>
              (ikey->get_cfg_val()->get_val());
          }
          if (val_vtn_neighbor != NULL) {
            ret_val = ValidateVtnNeighborValue(val_vtn_neighbor);
            if (ret_val != UPLL_RC_SUCCESS) {
              UPLL_LOG_DEBUG("Val struct Validation failed for READ op");
              return ret_val;
            }
            return UPLL_RC_SUCCESS;
          } else {
            UPLL_LOG_DEBUG("Value structure is an optional for READ op");
            return UPLL_RC_SUCCESS;
          }
        } else {
          UPLL_LOG_DEBUG("Option2 is not matching");
          return UPLL_RC_ERR_INVALID_OPTION2;
        }
      } else {
        UPLL_LOG_DEBUG("Option1 is not matching");
        return UPLL_RC_ERR_INVALID_OPTION1;
      }

    } else if ((operation == UNC_OP_DELETE) ||
        (operation == UNC_OP_READ_SIBLING_COUNT) ||
        (((operation == UNC_OP_READ_NEXT) ||
        (operation == UNC_OP_READ_BULK)) &&
        ((dt_type == UPLL_DT_CANDIDATE) ||
          (dt_type == UPLL_DT_RUNNING) ||
          (dt_type == UPLL_DT_STARTUP)))) {
      UPLL_LOG_TRACE("Value structure is none for this operation:%d",
          operation);
      return UPLL_RC_SUCCESS;
    } else {
      UPLL_LOG_DEBUG("Invalid datatype(%d) and operation(%d)", dt_type,
          operation);
      return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT;
    }
  }
  return UPLL_RC_SUCCESS;
}
upll_rc_t VtunnelIfMoMgr::ValidateVTunnelIfValue(
    val_vtunnel_if_t *val_vtunnel_if,
    uint32_t operation) {
  UPLL_FUNC_TRACE;
  if (val_vtunnel_if->valid[UPLL_IDX_DESC_VTNL_IF] == UNC_VF_VALID) {
    if (!ValidateDesc(val_vtunnel_if->description,
        kMinLenDescription, kMaxLenDescription)) {
      UPLL_LOG_DEBUG(
          "Syntax check failed.Desc- (%s)", val_vtunnel_if->description);
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  } else if ((val_vtunnel_if->valid[UPLL_IDX_DESC_VTNL_IF] ==
        UNC_VF_VALID_NO_VALUE) &&
      ((operation == UNC_OP_UPDATE) || (operation == UNC_OP_CREATE))) {
    val_vtunnel_if->description[0] = '\0';
  }

  if (val_vtunnel_if->valid[UPLL_IDX_ADMIN_ST_VTNL_IF] == UNC_VF_VALID) {
    if (!ValidateNumericRange(val_vtunnel_if->admin_status,
          (uint8_t) UPLL_ADMIN_ENABLE,
          (uint8_t) UPLL_ADMIN_DISABLE, true, true)) {
      UPLL_LOG_DEBUG(
          "Syntax check failed.Admin_status- %d", val_vtunnel_if->admin_status);
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  } else if ((val_vtunnel_if->valid[UPLL_IDX_ADMIN_ST_VTNL_IF] ==
        UNC_VF_VALID_NO_VALUE) &&
      ((operation == UNC_OP_UPDATE) || (operation == UNC_OP_CREATE))) {
    val_vtunnel_if->admin_status = UPLL_ADMIN_ENABLE;
  } else if (
         (val_vtunnel_if->valid[UPLL_IDX_ADMIN_ST_VTNL_IF] == UNC_VF_INVALID)
                                             && (operation == UNC_OP_CREATE)) {
      val_vtunnel_if->valid[UPLL_IDX_ADMIN_ST_VTNL_IF] = UNC_VF_VALID_NO_VALUE;
      val_vtunnel_if->admin_status = UPLL_ADMIN_ENABLE;
  }
  if (val_vtunnel_if->valid[UPLL_IDX_PORT_MAP_VTNL_IF] == UNC_VF_VALID) {
    if (val_vtunnel_if->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM]
        == UNC_VF_VALID) {
      if (!ValidateStringRange(
            reinterpret_cast<char *>(val_vtunnel_if->portmap.logical_port_id),
            kMinLenLogicalPortId, kMaxLenLogicalPortId)) {
        UPLL_LOG_DEBUG("Logical Port id syntax check failed."
            "Received Logical Port Id - %s",
            val_vtunnel_if->portmap.logical_port_id);
        return UPLL_RC_ERR_CFG_SYNTAX;
      }
      if (toupper(val_vtunnel_if->portmap.logical_port_id[0]) == 'S'
          && toupper(val_vtunnel_if->portmap.logical_port_id[1]) == 'W') {
        UPLL_LOG_DEBUG("Invalid logical_port_id - %s",
            val_vtunnel_if->portmap.logical_port_id);
        return UPLL_RC_ERR_CFG_SYNTAX;
      }
    } else if (val_vtunnel_if->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] ==
        UNC_VF_VALID_NO_VALUE
        && (operation == UNC_OP_CREATE || operation == UNC_OP_UPDATE)) {
      uuu::upll_strncpy(val_vtunnel_if->portmap.logical_port_id, " ",
          kMaxLenLogicalPortId+1);
      // Delete all dependent attributes
      val_vtunnel_if->portmap.vlan_id = 0;
      val_vtunnel_if->portmap.tagged = 0;
      val_vtunnel_if->portmap.valid[UPLL_IDX_VLAN_ID_PM] =
                                                  UNC_VF_VALID_NO_VALUE;
      val_vtunnel_if->portmap.valid[UPLL_IDX_TAGGED_PM] =
                                                  UNC_VF_VALID_NO_VALUE;
    } else if ((val_vtunnel_if->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] ==
               UNC_VF_INVALID) && (operation == UNC_OP_CREATE)) {
      val_vtunnel_if->portmap.valid[UPLL_IDX_VLAN_ID_PM] = UNC_VF_INVALID;
      val_vtunnel_if->valid[UPLL_IDX_PORT_MAP_VTNL_IF] = UNC_VF_INVALID;
    }
    if (val_vtunnel_if->portmap.valid[UPLL_IDX_VLAN_ID_PM] == UNC_VF_VALID) {
      if ((val_vtunnel_if->portmap.vlan_id != 0xFFFF) &&
          !ValidateNumericRange(val_vtunnel_if->portmap.vlan_id,
            (uint16_t) kMinVlanId, (uint16_t) kMaxVlanId,
            true, true)) {
        UPLL_LOG_DEBUG("Vlan Id Number check failed."
            "Received vlan_id - %d",
            val_vtunnel_if->portmap.vlan_id);
        return UPLL_RC_ERR_CFG_SYNTAX;
      }
    } else if (val_vtunnel_if->portmap.valid[UPLL_IDX_VLAN_ID_PM]
        == UNC_VF_VALID_NO_VALUE
        && (operation == UNC_OP_CREATE || operation == UNC_OP_UPDATE)) {
      /* If VLAN_ID is erased, Tagged attribute also needs to be erased */
      val_vtunnel_if->portmap.vlan_id = 0;
      val_vtunnel_if->portmap.tagged = 0;
      val_vtunnel_if->portmap.valid[UPLL_IDX_TAGGED_PM] =
        UNC_VF_VALID_NO_VALUE;
    } else if ((val_vtunnel_if->portmap.valid[UPLL_IDX_VLAN_ID_PM] ==
               UNC_VF_INVALID) && (operation == UNC_OP_CREATE)) {
      val_vtunnel_if->portmap.valid[UPLL_IDX_TAGGED_PM] = UNC_VF_INVALID;
    }
    if (val_vtunnel_if->portmap.valid[UPLL_IDX_TAGGED_PM] == UNC_VF_VALID) {
      if (!ValidateNumericRange((uint8_t) val_vtunnel_if->portmap.tagged,
            (uint8_t) UPLL_VLAN_UNTAGGED,
            (uint8_t) UPLL_VLAN_TAGGED, true, true)) {
        UPLL_LOG_DEBUG("Tagged Numeric range check failed."
            "Received Tag - %d",
            val_vtunnel_if->portmap.tagged);
        return UPLL_RC_ERR_CFG_SYNTAX;
      }
    } else if (((val_vtunnel_if->portmap.valid[UPLL_IDX_TAGGED_PM] ==
         UNC_VF_VALID_NO_VALUE) ||
        (val_vtunnel_if->portmap.valid[UPLL_IDX_TAGGED_PM] == UNC_VF_INVALID))
         && (operation == UNC_OP_CREATE)) {
      if (val_vtunnel_if->portmap.valid[UPLL_IDX_VLAN_ID_PM] == UNC_VF_VALID) {
        val_vtunnel_if->portmap.tagged = UPLL_VLAN_TAGGED;
        val_vtunnel_if->portmap.valid[UPLL_IDX_TAGGED_PM] = UNC_VF_VALID;
      }
    }
  } else if ((val_vtunnel_if->valid[UPLL_IDX_PORT_MAP_VTNL_IF] ==
        UNC_VF_VALID_NO_VALUE)
      && (operation == UNC_OP_UPDATE || operation == UNC_OP_CREATE)) {
      memset(&(val_vtunnel_if->portmap), 0, sizeof(val_vtunnel_if->portmap));
    for (unsigned int port_valid = 0;
         port_valid < sizeof(val_vtunnel_if->portmap.valid);
         ++port_valid) {
      val_vtunnel_if->portmap.valid[port_valid] = UNC_VF_VALID_NO_VALUE;
    }
  }

  return UPLL_RC_SUCCESS;
}
upll_rc_t VtunnelIfMoMgr::ValidateVTunnelIfKey(
    key_vtunnel_if_t *key_vtunnel_if,
    uint32_t operation) {
  UPLL_FUNC_TRACE;
  upll_rc_t ret_val = UPLL_RC_SUCCESS;
  ret_val = ValidateKey(
  reinterpret_cast<char *>(key_vtunnel_if->vtunnel_key.vtn_key.vtn_name),
      kMinLenVtnName, kMaxLenVtnName);
  if (ret_val != UPLL_RC_SUCCESS) {
    UPLL_LOG_INFO("Vtn Name syntax check failed."
                  "Received VTN Name - %s",
                  key_vtunnel_if->vtunnel_key.vtn_key.vtn_name);
    return UPLL_RC_ERR_CFG_SYNTAX;
  }
  ret_val = ValidateKey(
      reinterpret_cast<char *>(key_vtunnel_if->vtunnel_key.vtunnel_name),
      kMinLenVnodeName, kMaxLenVnodeName);
  if (ret_val != UPLL_RC_SUCCESS) {
    UPLL_LOG_INFO("Vtunnel Name syntax check failed."
                "Received Vtunnel Name -%s",
                key_vtunnel_if->vtunnel_key.vtunnel_name);
    return UPLL_RC_ERR_CFG_SYNTAX;
  }
  if ((operation != UNC_OP_READ_SIBLING_BEGIN) &&
      (operation != UNC_OP_READ_SIBLING_COUNT)) {
    ret_val = ValidateKey(reinterpret_cast<char *>(key_vtunnel_if->if_name),
              kMinLenInterfaceName, kMaxLenInterfaceName);
    if (ret_val != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG(
        "Syntax check failed.if_name- (%s)", key_vtunnel_if->if_name);
      return ret_val;
    }
  } else {
    UPLL_LOG_TRACE("Operation is %d", operation);
    StringReset(key_vtunnel_if->if_name);
  }
  UPLL_LOG_INFO("key structure validation successful for keytype VTUNNEL IF");
  return ret_val;
}
upll_rc_t VtunnelIfMoMgr::ValidateVtnNeighborValue(
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
  UPLL_LOG_TRACE("value structure validation successful for vunk_If keytype");
  return ret_val;
}
upll_rc_t VtunnelIfMoMgr::ValVTunnelIfAttributeSupportCheck(
    val_vtunnel_if_t *val_vtunnel_if,
    const uint8_t* attrs, unc_keytype_operation_t operation) {
  UPLL_FUNC_TRACE;
  if ((val_vtunnel_if->valid[UPLL_IDX_DESC_VTNL_IF] == UNC_VF_VALID)
      || (val_vtunnel_if->valid[UPLL_IDX_DESC_VTNL_IF]
        == UNC_VF_VALID_NO_VALUE)) {
    if (attrs[unc::capa::vtunnel_if::kCapDesc] == 0) {
      val_vtunnel_if->valid[UPLL_IDX_DESC_VTNL_IF] = UNC_VF_INVALID;
      if (operation == UNC_OP_CREATE || operation == UNC_OP_UPDATE) {
        UPLL_LOG_DEBUG("Descvtnl not supported in pfc controller");
        return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
      }
    }
  }
  if ((val_vtunnel_if->valid[UPLL_IDX_ADMIN_ST_VTNL_IF] == UNC_VF_VALID)
      || (val_vtunnel_if->valid[UPLL_IDX_ADMIN_ST_VTNL_IF]
        == UNC_VF_VALID_NO_VALUE)) {
    if (attrs[unc::capa::vtunnel_if::kCapAdminStatus] == 0) {
      val_vtunnel_if->valid[UPLL_IDX_ADMIN_ST_VTNL_IF] =
        UNC_VF_INVALID;
      if (operation == UNC_OP_CREATE || operation == UNC_OP_UPDATE) {
        UPLL_LOG_DEBUG("AdminStatus not supported in pfc controller");
        return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
      }
    }
  }
  if ((val_vtunnel_if->valid[UPLL_IDX_PORT_MAP_VTNL_IF] == UNC_VF_VALID) ||
      (val_vtunnel_if->valid[UPLL_IDX_PORT_MAP_VTNL_IF] ==
       UNC_VF_VALID_NO_VALUE)) {
    if ((val_vtunnel_if->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] ==
          UNC_VF_VALID)
        || (val_vtunnel_if->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM]
          == UNC_VF_VALID_NO_VALUE)) {
      if (attrs[unc::capa::vtunnel_if::kCapLogicalPortId] == 0) {
        val_vtunnel_if->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] =
          UNC_VF_INVALID;
        if (operation == UNC_OP_CREATE || operation == UNC_OP_UPDATE) {
          UPLL_LOG_DEBUG("portmap.swich_id attr is not supported by ctrlr ");
          return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
        }
      }
    }
    if ((val_vtunnel_if->portmap.valid[UPLL_IDX_VLAN_ID_PM] == UNC_VF_VALID)
        || (val_vtunnel_if->portmap.valid[UPLL_IDX_VLAN_ID_PM]
          == UNC_VF_VALID_NO_VALUE)) {
      if (attrs[unc::capa::vtunnel_if::kCapVlanId] == 0) {
        val_vtunnel_if->portmap.valid[UPLL_IDX_VLAN_ID_PM] =
          UNC_VF_INVALID;
        if (operation == UNC_OP_CREATE || operation == UNC_OP_UPDATE) {
          UPLL_LOG_DEBUG("portmap.vlanid attr is not supported by ctrlr ");
          return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
        }
      }
    }
    if ((val_vtunnel_if->portmap.valid[UPLL_IDX_TAGGED_PM] == UNC_VF_VALID)
        || (val_vtunnel_if->portmap.valid[UPLL_IDX_TAGGED_PM]
          == UNC_VF_VALID_NO_VALUE)) {
      if (attrs[unc::capa::vtunnel_if::kCapTagged] == 0) {
        val_vtunnel_if->portmap.valid[UPLL_IDX_TAGGED_PM] =
          UNC_VF_INVALID;
        if (operation == UNC_OP_CREATE || operation == UNC_OP_UPDATE) {
          UPLL_LOG_DEBUG("portmap.Tagged attr is not supported by ctrlr ");
          return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
        }
      }
    }
  }
  return UPLL_RC_SUCCESS;
}
upll_rc_t VtunnelIfMoMgr::ValidateCapability(IpcReqRespHeader *req,
    ConfigKeyVal *ikey, const char *ctrlr_name) {
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
  val_vtunnel_if_t *val_vtunnel_if = NULL;
  if ((ikey->get_cfg_val()) &&
      (ikey->get_cfg_val()->get_st_num() == IpctSt::kIpcStValVtunnelIf)) {
    val_vtunnel_if =
      reinterpret_cast<val_vtunnel_if_t *> (ikey->get_cfg_val()->get_val());
  }

  if (val_vtunnel_if) {
    if (max_attrs > 0) {
      return ValVTunnelIfAttributeSupportCheck(val_vtunnel_if, attrs,
                                               req->operation);
    } else {
      UPLL_LOG_DEBUG("Attribute list is empty for operation %d",
                                                req->operation);
      return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
    }
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtunnelIfMoMgr::IsReferenced(IpcReqRespHeader *req,
    ConfigKeyVal *ikey,
    DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  ConfigKeyVal *okey = NULL;
  if (!ikey || !(ikey->get_key()) ||!dmi )
    return UPLL_RC_ERR_GENERIC;
  GetChildConfigKey(okey, ikey);
  DbSubOp dbop = { kOpReadMultiple, kOpMatchNone, kOpInOutFlag };
  upll_rc_t result_code = ReadConfigDB(okey, req->datatype, UNC_OP_READ,
                                       dbop, dmi, MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    if (okey) delete okey;
    return result_code;
  }
  ConfigKeyVal *tmp = okey;
  while (okey) {
    uint8_t vlink_flag = 0;
    GET_USER_DATA_FLAGS(okey, vlink_flag);
    if (vlink_flag & VIF_TYPE) {
      result_code = UPLL_RC_ERR_CFG_SEMANTIC;
      break;
    }
    okey = okey->get_next_cfg_key_val();
  }
  DELETE_IF_NOT_NULL(tmp);
  return result_code;
}

upll_rc_t VtunnelIfMoMgr::GetChildConvertConfigKey(ConfigKeyVal *&okey,
                                        ConfigKeyVal *parent_key) {
  UPLL_FUNC_TRACE;

  upll_rc_t result_code = UPLL_RC_SUCCESS;
  key_convert_vtunnel_if *vtunnel_convert_if_key = NULL;
  if (okey && (okey->get_key())) {
    vtunnel_convert_if_key = reinterpret_cast<key_convert_vtunnel_if_t *>
                (okey->get_key());
  } else {
    vtunnel_convert_if_key = ConfigKeyVal::Malloc<key_convert_vtunnel_if>();
  }
  void *pkey;
  if (parent_key == NULL) {
    if (!okey)
      okey = new ConfigKeyVal(UNC_KT_VTUNNEL_IF,
                              IpctSt::kIpcStKeyConvertVtunnelIf,
                              vtunnel_convert_if_key, NULL);
    else if (okey->get_key() != vtunnel_convert_if_key)
      okey->SetKey(IpctSt::kIpcStKeyConvertVtunnelIf, vtunnel_convert_if_key);
    return UPLL_RC_SUCCESS;
  } else {
    pkey = parent_key->get_key();
  }
  if (!pkey) {
    if (!okey || !(okey->get_key()))
      free(vtunnel_convert_if_key);
    return UPLL_RC_ERR_GENERIC;
  }
  /* presumes MoMgrs receive only supported keytypes */
  switch (parent_key->get_key_type()) {
    case UNC_KT_VBR_PORTMAP:
      uuu::upll_strncpy(
        vtunnel_convert_if_key->convert_vtunnel_key.vtn_key.vtn_name,
         reinterpret_cast<key_vbr_portmap*>(pkey)->vbr_key.vtn_key.vtn_name,
        (kMaxLenVtnName+1));
      break;
    case UNC_KT_VTUNNEL_IF:
      if (parent_key->get_st_num() == IpctSt::kIpcStKeyConvertVtunnelIf) {
        uuu::upll_strncpy(
            vtunnel_convert_if_key->convert_vtunnel_key.vtn_key.vtn_name,
            reinterpret_cast<key_convert_vtunnel_if*>(
              pkey)->convert_vtunnel_key.vtn_key.vtn_name, (kMaxLenVtnName+1));
        uuu::upll_strncpy(
            vtunnel_convert_if_key->convert_vtunnel_key.convert_vtunnel_name,
            reinterpret_cast<key_convert_vtunnel_if*>(
                pkey)->convert_vtunnel_key.convert_vtunnel_name,
            (kMaxLenConvertVnodeName+1));
        uuu::upll_strncpy(vtunnel_convert_if_key->convert_if_name,
            reinterpret_cast<key_convert_vtunnel_if*>(pkey)->convert_if_name,
            (kMaxLenInterfaceName+1));
      }
      break;

    default:
      break;
  }
  if (!okey)
    okey = new ConfigKeyVal(UNC_KT_VTUNNEL_IF,
                            IpctSt::kIpcStKeyConvertVtunnelIf,
                            vtunnel_convert_if_key, NULL);
  else if (okey->get_key() != vtunnel_convert_if_key)
    okey->SetKey(IpctSt::kIpcStKeyConvertVtunnelIf, vtunnel_convert_if_key);
  if (okey == NULL) {
    free(vtunnel_convert_if_key);
    result_code = UPLL_RC_ERR_GENERIC;
  } else {
    SET_USER_DATA(okey, parent_key);
  }
  return result_code;
}

/** This function binds the vtn_name in key structure and 
 * val structure attribute (rem ctrlr, rem domain, uvbr name) if any and
 * deletes the convert_vtunnel_if entries. Does not bind to vtunnel name and if
 * name for match.
 */
upll_rc_t VtunnelIfMoMgr::DeleteVtunnelIf(
    ConfigKeyVal *ikey, TcConfigMode config_mode, std::string vtn_name,
    DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;

  UPLL_LOG_TRACE("DELETE %s", (ikey->ToStrAll()).c_str());

  upll_rc_t result_code = UPLL_RC_ERR_GENERIC;
  val_convert_vtunnel_if *vtun_if = reinterpret_cast<val_convert_vtunnel_if*>(
      GetVal(ikey));
  DalBindInfo *bind_info = new DalBindInfo(uudst::kDbiConvertVtunnelIfTbl);
  // match the vtn name
  bind_info->BindMatch(
      uudst::convert_vtunnel_interface::kDbiVtnName, uud::kDalChar,
      (kMaxLenVtnName+1), reinterpret_cast<key_convert_vtunnel_if*>(
        ikey->get_key())->convert_vtunnel_key.vtn_key.vtn_name);
  if (NULL != vtun_if) {
    // match the val structure based on valid flag for deletion
    if (vtun_if->valid[UPLL_IDX_RCTRLR_NAME_CONV_VTNL_IF] == UNC_VF_VALID) {
      bind_info->BindMatch(
          uudst::convert_vtunnel_interface::kDbiRemoteCtrlrName, uud::kDalChar,
         (kMaxLenCtrlrId+1), vtun_if->rem_ctrlr_name);
    }
    if (vtun_if->valid[UPLL_IDX_RDOMAIN_ID_CONV_VTNL_IF] == UNC_VF_VALID) {
      bind_info->BindMatch(uudst::convert_vtunnel_interface::kDbiRemoteDomainId,
           uud::kDalChar, (kMaxLenDomainId+1), vtun_if->rem_domain_id);
    }
    if (vtun_if->valid[UPLL_IDX_UN_VBR_NAME_CONV_VTNL_IF] == UNC_VF_VALID) {
      bind_info->BindMatch(uudst::convert_vtunnel_interface::kDbiUnVbrName,
           uud::kDalChar, (kMaxLenVnodeName+1), vtun_if->un_vbr_name);
    }
  }

  uint8_t *vtnname = NULL;
  if (!vtn_name.empty()) {
    vtnname = reinterpret_cast<uint8_t *>(const_cast<char *>(vtn_name.c_str()));
  }
  result_code = DalToUpllResCode(dmi->DeleteRecords(UPLL_DT_CANDIDATE,
        uudst::kDbiConvertVtunnelIfTbl, bind_info, false,
        config_mode, vtnname));
  DELETE_IF_NOT_NULL(bind_info);
  if ((result_code != UPLL_RC_SUCCESS) &&
      (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE)) {
    UPLL_LOG_ERROR("convert_vtunnel_if table delete failed:%d", result_code);
  }
  return result_code;
}

upll_rc_t VtunnelIfMoMgr::ConvertVtunnelIf(ConfigKeyVal *ikey,
                                           unc_keytype_operation_t op,
                                           TcConfigMode config_mode,
                                           std::string vtn_name,
                                           DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_ERR_GENERIC;

  // validate the input
  if (!ikey || !dmi || !ikey->get_key()) {
    UPLL_LOG_ERROR("argument is null");
    return UPLL_RC_ERR_GENERIC;
  }

  if (UNC_KT_VTUNNEL_IF != ikey->get_key_type()) {
    UPLL_LOG_ERROR("Invalid keytype. Keytype- %d", ikey->get_key_type());
    return UPLL_RC_ERR_GENERIC;
  }

  if (ikey->get_st_num() != IpctSt::kIpcStKeyConvertVtunnelIf) {
    UPLL_LOG_ERROR("Invalid struct %d received.", ikey->get_st_num());
    return UPLL_RC_ERR_GENERIC;
  }
  if (op == UNC_OP_DELETE) {
    return DeleteVtunnelIf(ikey, config_mode, vtn_name, dmi);
  }
  if (op == UNC_OP_CREATE) {
    val_convert_vtunnel_if *conv_vtun_if_val = reinterpret_cast<
      val_convert_vtunnel_if*>(GetVal(ikey));
    if (NULL == conv_vtun_if_val) {
      UPLL_LOG_ERROR("Mandatory val structure is NULL");
      return UPLL_RC_ERR_GENERIC;
    }
    if ((conv_vtun_if_val->valid[UPLL_IDX_RCTRLR_NAME_CONV_VTNL_IF] !=
          UNC_VF_VALID ||
         conv_vtun_if_val->valid[UPLL_IDX_RDOMAIN_ID_CONV_VTNL_IF] !=
          UNC_VF_VALID))  {
      UPLL_LOG_ERROR("Remote ctrlr/domain is not valid");
      return UPLL_RC_ERR_GENERIC;
    }
  }

  // Fetch the ConfigKey
  key_convert_vtunnel_if *con_key_tun_if =
    reinterpret_cast<key_convert_vtunnel_if *>(ikey->get_key());

  if (op == UNC_OP_CREATE && (strlen(reinterpret_cast<const char*>(
      con_key_tun_if->convert_if_name)) == 0)) {
    string vtunnelNameIf;
    while (1) {
      // Generate vtunnel interface based on timestamp
      vtunnelNameIf = "IF_" + unc::upll::upll_util::getTime();
      // Assign the generated vtunnel interface name
      uuu::upll_strncpy(con_key_tun_if->convert_if_name,
          vtunnelNameIf.c_str(), (kMaxLenInterfaceName+1));
      DbSubOp dbop = {kOpReadExist, kOpMatchNone, kOpInOutNone};
       result_code = UpdateConfigDB(ikey, UPLL_DT_CANDIDATE, UNC_OP_READ, dmi,
            &dbop, CONVERTTBL);
      // Retry and generate unique name because
      // Converted vtunnel interface name is not unique with in that table.
      if (result_code == UPLL_RC_ERR_INSTANCE_EXISTS) {
        continue;
        // Break the loop if it is unique name with in that table.
      } else if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
        break;
      } else {
        // Return error for any other DB related errors.
        UPLL_LOG_DEBUG("ReadConfigDB failed- %d", result_code);
        return result_code;
      }
    }
  }
  // Insert/delete the entry to database
  result_code = UpdateConfigDB(ikey, UPLL_DT_CANDIDATE, op, dmi,
                               config_mode, vtn_name, CONVERTTBL);
  return result_code;
}

}  // namespace kt_momgr
}  // namespace upll
}  // namespace unc
