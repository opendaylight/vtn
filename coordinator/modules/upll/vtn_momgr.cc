/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include "vtn_momgr.hh"
#include "vbr_if_momgr.hh"
#include "vbr_momgr.hh"
#include "vnode_momgr.hh"
#include "vlink_momgr.hh"
#include "vlanmap_momgr.hh"
#include "vterm_if_momgr.hh"
#include "config_yield.hh"

#define  NUM_KEY_COL 3

#define CTRLR_DOM_INVALID 0x00
#define CTRLR_VALID       0x10
#define CTRLR_DOM_VALID   0x11
#include "ctrlr_capa_defines.hh"
#include "capa_intf.hh"
#include "upll_validation.hh"
#include "uncxx/upll_log.hh"
#include "unc/uppl_common.h"
#include "vbr_portmap_momgr.hh"

using unc::upll::ipc_util::IpcUtil;
using unc::upll::ipc_util::KtUtil;

namespace unc {
namespace upll {
namespace kt_momgr {

BindInfo VtnMoMgr::vtn_bind_info[] = { { uudst::vtn::kDbiVtnName, CFG_KEY,
                                         offsetof(key_vtn_t, vtn_name),
                                         uud::kDalChar, 32 },
                                       { uudst::vtn::kDbiVtnDesc, CFG_VAL,
                                         offsetof(val_vtn, description),
                                         uud::kDalChar, 128 },
                                       { uudst::vtn::kDbiVtnOperStatus, ST_VAL,
                                         offsetof(val_vtn_st, oper_status),
                                         uud::kDalUint8, 1 },
                                       { uudst::vtn::kDbiVtnAlarmStatus, ST_VAL,
                                         offsetof(val_vtn_st, alarm_status),
                                         uud::kDalUint8, 1 },
                                       { uudst::vtn::kDbiDownCount, ST_VAL,
                                          offsetof(val_db_vtn_st, down_count),
                                         uud::kDalUint32, 1 },
                                       { uudst::vtn::kDbiVtnCreationTime,
                                         ST_VAL,
                                         offsetof(val_vtn_st, creation_time),
                                         uud::kDalUint64, 1 },
                                       { uudst::vtn::kDbiVtnLastUpdatedTime,
                                         ST_VAL, offsetof(val_vtn_st,
                                                          last_updated_time),
                                         uud::kDalUint64, 1 },
                                       { uudst::vtn::kDbiVtnFlags, CK_VAL,
                                         offsetof(key_user_data_t, flags),
                                         uud::kDalUint8, 1 },
                                       { uudst::vtn::kDbiValidVtnDesc,
                                         CFG_META_VAL, offsetof(val_vtn,
                                                                valid[0]),
                                         uud::kDalUint8, 1 },
                                       { uudst::vtn::kDbiValidVtnOperStatus,
                                         ST_META_VAL, offsetof(val_vtn_st,
                                                               valid[0]),
                                         uud::kDalUint8, 1 },
                                       { uudst::vtn::kDbiValidVtnAlarmStatus,
                                         ST_META_VAL, offsetof(val_vtn_st,
                                                               valid[1]),
                                         uud::kDalUint8, 1 },
                                       { uudst::vtn::kDbiValidVtnCreationTime,
                                         ST_META_VAL, offsetof(val_vtn_st,
                                                               valid[2]),
                                         uud::kDalUint8, 1 },
                                       { uudst::vtn::
                                         kDbiValidVtnLastUpdatedTime,
                                         ST_META_VAL, offsetof(val_vtn_st,
                                                               valid[3]),
                                         uud::kDalUint8, 1 },
                                       { uudst::vtn::kDbiVtnCsRowStatus, CS_VAL,
                                         offsetof(val_vtn, cs_row_status),
                                         uud::kDalChar, 1 },
                                       { uudst::vtn::kDbiVtnCsDesc, CS_VAL,
                                         offsetof(val_vtn, cs_attr),
                                         uud::kDalUint8, 1 },
                                       { uudst::vtn::kDbiUnknownCount, ST_VAL,
                                         offsetof(val_db_vtn_st, unknown_count),
                                         uud::kDalUint32, 1 } };

BindInfo VtnMoMgr::vtn_rename_bind_info[] = { { uudst::vtn_rename::
                                                kDbiUncVtnName,
                                                CFG_KEY, offsetof(key_vtn,
                                                                  vtn_name),
                                                uud::kDalChar, 32 },
                                              { uudst::vtn_rename::
                                                kDbiControllerName,
                                                CK_VAL, offsetof(
                                                    key_user_data_t, ctrlr_id),
                                                uud::kDalChar, 32 },
                                              { uudst::vtn_rename::kDbiDomainId,
                                                CK_VAL, offsetof(
                                                    key_user_data_t, domain_id),
                                                uud::kDalChar, 32 },
                                              { uudst::vtn_rename::
                                                kDbiCtrlrVtnName,
                                                CFG_VAL, offsetof(
                                                    val_rename_vtn, new_name),
                                                uud::kDalChar, 32 } };

BindInfo VtnMoMgr::vtn_controller_bind_info[] = {
    { uudst::vtn_controller::kDbiVtnName, CFG_KEY, offsetof(key_vtn, vtn_name),
      uud::kDalChar, 32 },
    { uudst::vtn_controller::kDbiControllerName, CK_VAL,
      offsetof(key_user_data_t, ctrlr_id),
      uud::kDalChar, 32 },
    { uudst::vtn_controller::kDbiDomainId, CK_VAL, offsetof(key_user_data_t,
                                                     domain_id),
      uud::kDalChar, 32 },
    { uudst::vtn_controller::kDbiOperStatus, CFG_ST_VAL, offsetof(val_vtn_ctrlr,
                                                        oper_status),
      uud::kDalUint8, 1 },
    { uudst::vtn_controller::kDbiAlarmStatus, CFG_ST_VAL,
      offsetof(val_vtn_ctrlr, alarm_status),
      uud::kDalUint8, 1 },
    { uudst::vtn_controller::kDbiDownCount, CFG_ST_VAL, offsetof(val_vtn_ctrlr,
                                                       down_count),
      uud::kDalUint32, 1 },
    { uudst::vtn_controller::kDbiVnodeRefCnt, CFG_VAL, offsetof(val_vtn_ctrlr,
      vnode_ref_cnt), uud::kDalUint32, 1 },
    { uudst::vtn_controller::kDbiRefCnt2, CFG_VAL, offsetof(val_vtn_ctrlr,
      ref_cnt2), uud::kDalUint32, 1 },
    { uudst::vtn_controller::kDbiValidOperStatus, CFG_ST_META_VAL,
      offsetof(val_vtn_ctrlr, valid[0]),
      uud::kDalUint8, 1 },
    { uudst::vtn_controller::kDbiValidAlarmStatus, CFG_ST_META_VAL, offsetof(
        val_vtn_ctrlr, valid[1]),
      uud::kDalUint8, 1 },
    { uudst::vtn_controller::kDbiCsDesc, CS_VAL, offsetof(val_vtn_ctrlr,
      cs_attr[0]),
      uud::kDalUint8, 1 },
    { uudst::vtn_controller::kDbiCsRowstatus, CS_VAL, offsetof(val_vtn_ctrlr,
                                                        cs_row_status),
      uud::kDalUint8, 1 },
    { uudst::vtn_controller::kDbiVtnCtrlrFlags, CK_VAL,
      offsetof(key_user_data_t, flags),
      uud::kDalUint8, 1 },
    { uudst::vtn_controller::kDbiUnknownCount, CFG_ST_VAL,
      offsetof(val_vtn_ctrlr, unknown_count),
      uud::kDalUint32, 1 } };

BindInfo VtnMoMgr::key_vtn_maintbl_bind_info[] = {
    { uudst::vtn::kDbiVtnName, CFG_MATCH_KEY, offsetof(key_vtn_t, vtn_name),
      uud::kDalChar, kMaxLenVtnName + 1 },
    { uudst::vtn::kDbiVtnName, CFG_INPUT_KEY, offsetof(key_rename_vnode_info_t,
                                                     new_unc_vtn_name),
      uud::kDalChar, kMaxLenVtnName + 1 },
    { uudst::vtn::kDbiVtnFlags, CK_VAL, offsetof(key_user_data_t, flags),
      uud::kDalUint8, 1 }
};

BindInfo VtnMoMgr::key_vtn_ctrlrtbl_bind_info[] = {
    { uudst::vtn_controller::kDbiVtnName, CFG_MATCH_KEY, offsetof(key_vtn_t,
                                                                vtn_name),
      uud::kDalChar, kMaxLenVtnName + 1 },
    { uudst::vtn_controller::kDbiVtnName, CFG_INPUT_KEY, offsetof(
        key_rename_vnode_info_t, new_unc_vtn_name),
      uud::kDalChar, kMaxLenVtnName + 1 },
    { uudst::vtn_controller::kDbiVtnCtrlrFlags, CK_VAL, offsetof(
        key_user_data_t, flags),
      uud::kDalUint8, 1 } };
BindInfo VtnMoMgr::key_vtn_renametbl_bind_info[] = {
    { uudst::vtn_rename::kDbiUncVtnName, CFG_MATCH_KEY, offsetof(key_vtn_t,
                                                               vtn_name),
      uud::kDalChar, kMaxLenVtnName + 1 },
    { uudst::vtn_rename::kDbiUncVtnName, CFG_INPUT_KEY, offsetof(
        key_rename_vnode_info_t, new_unc_vtn_name),
      uud::kDalChar, kMaxLenVtnName + 1 }, };

BindInfo VtnMoMgr::key_vtn_gateway_port_bind_info[] = {
    { uudst::vtn_gateway_port::kDbiVtnName, CFG_KEY,
      offsetof(key_vtn, vtn_name), uud::kDalChar, 32 },
    { uudst::vtn_gateway_port::kDbiCtrlrName, CK_VAL, offsetof(key_user_data,
      ctrlr_id), uud::kDalChar, 32 },
    { uudst::vtn_gateway_port::kDbiDomainId, CK_VAL, offsetof(key_user_data,
      domain_id), uud::kDalChar, 32 },
    { uudst::vtn_gateway_port::kDbiLogicalPortId, CFG_VAL, offsetof(
        val_vtn_gateway_port, logical_port_id), uud::kDalChar, 320 },
  { uudst::vtn_gateway_port::kDbiLabel, CFG_VAL,
    offsetof(val_vtn_gateway_port, label), uud::kDalUint32, 1},
  { uudst::vtn_gateway_port::kDbiRefCount, CFG_VAL,
    offsetof(val_vtn_gateway_port, ref_count), uud::kDalUint32, 1},
  { uudst::vtn_gateway_port::kDbiOperStatus, ST_VAL,
    offsetof(val_vtn_st, oper_status),
    uud::kDalUint8, 1},
  { uudst::vtn_gateway_port::kDbiDownCount, ST_VAL,
    offsetof(val_db_vtn_st, down_count),
    uud::kDalUint32, 1},
  { uudst::vtn_gateway_port::kDbiFlags, CK_VAL,
    offsetof(key_user_data_t, flags), uud::kDalUint8, 1},
  { uudst::vtn_gateway_port::kDbiValidLogicalPortId, CFG_META_VAL, offsetof(
        val_vtn_gateway_port, valid[0]), uud::kDalUint8, 1 },
  { uudst::vtn_gateway_port::kDbiValidLabel, CFG_META_VAL,
    offsetof(val_vtn_gateway_port, valid[1]), uud::kDalUint8, 1},
  { uudst::vtn_gateway_port::kDbiValidRefCount, CFG_META_VAL,
    offsetof(val_vtn_gateway_port, valid[2]), uud::kDalUint8, 1},
  { uudst::vtn_gateway_port::kDbiValidOperStatus, ST_META_VAL,
    offsetof(val_vtn_st, valid[0]), uud::kDalUint8, 1},
  { uudst::vtn_gateway_port::kDbiCsRowstatus, CS_VAL,
    offsetof(val_vtn_gateway_port, cs_row_status), uud::kDalUint8, 1},
  { uudst::vtn_gateway_port::kDbiCsLogicalPortId, CS_VAL, offsetof(
        val_vtn_gateway_port, cs_attr[0]), uud::kDalUint8, 1 },
  { uudst::vtn_gateway_port::kDbiCsLabel, CS_VAL,
    offsetof(val_vtn_gateway_port, cs_attr[1]), uud::kDalUint8, 1},
  { uudst::vtn_gateway_port::kDbiCsRefCount, CS_VAL,
    offsetof(val_vtn_gateway_port, cs_attr[2]), uud::kDalUint8, 1}
};

unc_key_type_t VtnMoMgr::vtn_child[] = { UNC_KT_VBRIDGE,
                                         UNC_KT_VROUTER, UNC_KT_VUNKNOWN,
                                         /* For restore case: keeping VTEP
                                          * before VTEP_GRP */
                                         UNC_KT_VTEP, UNC_KT_VTEP_GRP,
                                         UNC_KT_VTUNNEL,
                                         UNC_KT_VLINK,
                                         UNC_KT_VTN_FLOWFILTER,
                                         UNC_KT_VTN_POLICINGMAP,
                                         UNC_KT_VTERMINAL,
                                         UNC_KT_VTN_UNIFIED};
  VtnMoMgr::VtnMoMgr() : MoMgrImpl() {
  UPLL_FUNC_TRACE;
  ntable = MAX_MOMGR_TBLS;
  table = new Table *[ntable]();
  table[MAINTBL] = new Table(uudst::kDbiVtnTbl, UNC_KT_VTN, vtn_bind_info,
      IpctSt::kIpcStKeyVtn, IpctSt::kIpcStValVtn,
      uudst::vtn::kDbiVtnNumCols);
  table[RENAMETBL] = new Table(uudst::kDbiVtnRenameTbl, UNC_KT_VTN,
      vtn_rename_bind_info, IpctSt::kIpcStKeyVtn, IpctSt::kIpcStValRenameVtn,
      uudst::vtn_rename::kDbiVtnRenameNumCols);
  table[CTRLRTBL] = new Table(uudst::kDbiVtnCtrlrTbl, UNC_KT_VTN,
      vtn_controller_bind_info, IpctSt::kIpcStKeyVtn, IpctSt::kIpcInvalidStNum,
      uudst::vtn_controller::kDbiVtnCtrlrNumCols);
  table[CONVERTTBL] = new Table(uudst::kDbiVtnGatewayPortTbl, UNC_KT_VTN,
      key_vtn_gateway_port_bind_info, IpctSt::kIpcStKeyVtn,
      IpctSt::kIpcStValVtnGatewayPort,
      uudst::vtn_gateway_port::kDbiVtnGatewayPortNumCols);
  nchild = sizeof(vtn_child) / sizeof(*vtn_child);
  child = vtn_child;
}


/*
 * Based on the key type the bind info will pass
 */

bool VtnMoMgr::GetRenameKeyBindInfo(unc_key_type_t key_type,
                                    BindInfo *&binfo,
                                    int &nattr,
                                    MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  nattr = NUM_KEY_COL;
  if (MAINTBL == tbl)
    binfo = key_vtn_maintbl_bind_info;
  if (CTRLRTBL == tbl)
    binfo = key_vtn_ctrlrtbl_bind_info;
  if (RENAMETBL == tbl) {
    nattr = 2;
    binfo = key_vtn_renametbl_bind_info;
  }
  if (CONVERTTBL == tbl) {
    binfo = key_vtn_gateway_port_bind_info;
  }
  return PFC_TRUE;
}


upll_rc_t VtnMoMgr::ValidateAttribute(ConfigKeyVal *ikey,
                                      DalDmlIntf *dmi,
                                      IpcReqRespHeader *req) {
  UPLL_FUNC_TRACE;
  if (ikey == NULL) return UPLL_RC_ERR_GENERIC;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  if (ikey->get_key_type() != UNC_KT_VTN) return UPLL_RC_ERR_GENERIC;
  return result_code;
}

bool VtnMoMgr::IsValidKey(void *key,
                          uint64_t index, MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  key_vtn *vtn_key = reinterpret_cast<key_vtn *>(key);
  upll_rc_t ret_val = UPLL_RC_SUCCESS;
  if (tbl == CONVERTTBL) {
    switch (index) {
      case uudst::vtn_gateway_port::kDbiVtnName:
        ret_val = ValidateKey(reinterpret_cast<char *>(vtn_key->vtn_name),
            kMinLenVtnName, kMaxLenVtnName);
        break;
    }
  } else {
    switch (index) {
      case uudst::vtn::kDbiVtnName:
      case uudst::vtn_rename::kDbiUncVtnName:
        ret_val = ValidateKey(reinterpret_cast<char *>(vtn_key->vtn_name),
            kMinLenVtnName, kMaxLenVtnName);
        break;
      default:
        ret_val = UPLL_RC_ERR_GENERIC;
    }
  }
  if (ret_val != UPLL_RC_SUCCESS) {
    UPLL_LOG_TRACE("index %"PFC_PFMT_u64" is not valid(%d)", index, ret_val);
    return false;
  }
  return true;
}

upll_rc_t VtnMoMgr::GetValid(void *val,
                             uint64_t indx,
                             uint8_t *&valid,
                             upll_keytype_datatype_t dt_type,
                             MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  if (val == NULL) return UPLL_RC_ERR_GENERIC;
  if (tbl == MAINTBL) {
    switch (indx) {
      case uudst::vtn::kDbiVtnOperStatus:
        valid = &(reinterpret_cast<val_vtn_st *>
            (val))->valid[UPLL_IDX_OPER_STATUS_VS];
        break;
      case uudst::vtn::kDbiVtnAlarmStatus:
        valid = &(reinterpret_cast<val_vtn_st *>
            (val))->valid[UPLL_IDX_ALARM_STATUS_VS];
        break;
      case uudst::vtn::kDbiDownCount:
      case uudst::vtn::kDbiUnknownCount:
        valid = NULL;
        break;
      case uudst::vtn::kDbiVtnCreationTime:
        UPLL_LOG_DEBUG("Valid calling fro VtnCreation time ");
        valid = &(reinterpret_cast<val_vtn_st *>
            (val))->valid[UPLL_IDX_CREATION_TIME_VS];
        break;
      case uudst::vtn::kDbiVtnLastUpdatedTime:
        valid = &(reinterpret_cast<val_vtn_st *>
            (val))->valid[UPLL_IDX_LAST_UPDATE_TIME_VS];
        break;
      case uudst::vtn::kDbiVtnDesc:
        valid = &(reinterpret_cast<val_vtn *>
            (val))->valid[UPLL_IDX_DESC_VTN];
        break;
      default:
        return UPLL_RC_ERR_GENERIC;
    }
  } else if (tbl == RENAMETBL) {
      switch (indx) {
      case uudst::vtn_rename::kDbiCtrlrVtnName:
         valid = &(reinterpret_cast<val_rename_vtn *>
                 (val))->valid[UPLL_IDX_NEW_NAME_RVTN];
         break;
      default:
         break;
     }
  } else if (tbl == CTRLRTBL) {
    valid = (reinterpret_cast<val_vtn_ctrlr *>(val))->valid;
    switch (indx) {
      case uudst::vtn_controller::kDbiAlarmStatus:
        valid = &(reinterpret_cast<val_vtn_ctrlr *>
            (val))->valid[UPLL_IDX_ALARM_STATUS_VS];
        break;
      case uudst::vtn_controller::kDbiOperStatus:
        valid = &(reinterpret_cast<val_vtn_ctrlr *>
            (val))->valid[UPLL_IDX_OPER_STATUS_VS];
        break;
      case uudst::vtn_controller::kDbiDownCount:
      case uudst::vtn_controller::kDbiUnknownCount:
      case uudst::vtn_controller::kDbiVnodeRefCnt:
      case uudst::vtn_controller::kDbiRefCnt2:
        valid = NULL;
        break;
      default:
        return UPLL_RC_ERR_GENERIC;
    }
  } else if (tbl == CONVERTTBL) {
    valid = (reinterpret_cast<val_vtn_gateway_port *>(val))->valid;
    switch (indx) {
      case uudst::vtn_gateway_port::kDbiLogicalPortId:
        valid = &(reinterpret_cast<val_vtn_gateway_port *>
            (val))->valid[UPLL_IDX_LOGICAL_PORT_ID_GWPORT];
        break;
      case uudst::vtn_gateway_port::kDbiLabel:
        valid = &(reinterpret_cast<val_vtn_gateway_port *>
            (val))->valid[UPLL_IDX_LABEL_GWPORT];
        break;
      case uudst::vtn_gateway_port::kDbiRefCount:
        valid = &(reinterpret_cast<val_vtn_gateway_port *>
            (val))->valid[UPLL_IDX_REF_COUNT_GWPORT];
        break;
      case uudst::vtn_gateway_port::kDbiOperStatus:
        valid = &(reinterpret_cast<val_vtn_gateway_port *>
            (val))->valid[UPLL_IDX_OPER_STATUS_VS];
        break;
      case uudst::vtn_gateway_port::kDbiDownCount:
        valid = NULL;
        break;
      default:
        return UPLL_RC_ERR_GENERIC;
    }
  } else {
    valid = NULL;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtnMoMgr::AllocVal(ConfigVal *&ck_val,
                             upll_keytype_datatype_t dt_type,
                             MoMgrTables tbl) {
  void *val;  //  *ck_nxtval;
  if (ck_val != NULL) return UPLL_RC_ERR_GENERIC;
  switch (tbl) {
    case MAINTBL:
      val = ConfigKeyVal::Malloc(sizeof(val_vtn));
      ck_val = new ConfigVal(IpctSt::kIpcStValVtn, val);
      if (dt_type == UPLL_DT_STATE) {
        val = ConfigKeyVal::Malloc(sizeof(val_db_vtn_st));
        ConfigVal *ck_nxtval = new ConfigVal(IpctSt::kIpcStValVtnSt, val);
        ck_val->AppendCfgVal(ck_nxtval);
      }
      break;
    case RENAMETBL:
      val = ConfigKeyVal::Malloc(sizeof(val_rename_vtn));
      ck_val = new ConfigVal(IpctSt::kIpcStValRenameVtn, val);
      break;
    case CTRLRTBL:
      val = ConfigKeyVal::Malloc(sizeof(val_vtn_ctrlr));
      ck_val = new ConfigVal(IpctSt::kIpcInvalidStNum, val);
      break;
    case CONVERTTBL:
      val = ConfigKeyVal::Malloc(sizeof(val_vtn_gateway_port));
      ck_val = new ConfigVal(IpctSt::kIpcStValVtnGatewayPort, val);
      if (dt_type == UPLL_DT_STATE) {
        val = ConfigKeyVal::Malloc(sizeof(val_db_vtn_st));
        ConfigVal *ck_nxtval = new ConfigVal(IpctSt::kIpcStValVtnSt, val);
        ck_val->AppendCfgVal(ck_nxtval);
      }
      break;
    default:
      val = NULL;
      break;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtnMoMgr::DupConfigKeyValVtnMapping(ConfigKeyVal *&okey,
                                              ConfigKeyVal *req) {
  UPLL_FUNC_TRACE;
  void *tkey = req ? (req)->get_key() : NULL;
  if (tkey == NULL) {
    UPLL_LOG_INFO("Input Configkeyval or key is Null");
    return UPLL_RC_ERR_GENERIC;
  }
  if (okey != NULL) {
    UPLL_LOG_INFO("okey is Not Null");
    return UPLL_RC_ERR_GENERIC;
  }
  ConfigVal *tmp1 = NULL, *tmp = (req)->get_cfg_val();
  val_vtn_mapping_controller_st_t *vtn_map = NULL;
  if (tmp) {
    val_vtn_mapping_controller_st_t *ival =
         reinterpret_cast<val_vtn_mapping_controller_st_t *>(GetVal(req));
    vtn_map = reinterpret_cast<val_vtn_mapping_controller_st_t *>(
          ConfigKeyVal::Malloc(sizeof(val_vtn_mapping_controller_st_t)));
    memcpy(reinterpret_cast<char *>(vtn_map), reinterpret_cast<char *>(ival),
             sizeof(val_vtn_mapping_controller_st_t));
    tmp1 = new ConfigVal(IpctSt::kIpcStValVtnMappingControllerSt, vtn_map);
  }
  key_vtn_controller *ikey = reinterpret_cast<key_vtn_controller *>(tkey);
  key_vtn_controller *key = reinterpret_cast<key_vtn_controller *>(
      ConfigKeyVal::Malloc(sizeof(key_vtn_controller)));
  memcpy(reinterpret_cast<char *>(key), reinterpret_cast<char *>(ikey),
         sizeof(key_vtn_controller));
  okey = new ConfigKeyVal(UNC_KT_VTN_MAPPING_CONTROLLER,
                   IpctSt::kIpcStKeyVtnController, key, tmp1);
  SET_USER_DATA(okey, req)
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtnMoMgr::DupConfigKeyValVtnStation(ConfigKeyVal *&okey,
                                 ConfigKeyVal *req) {
  UPLL_FUNC_TRACE;
  void *tkey = req ? (req)->get_key() : NULL;
  if (tkey == NULL) {
    UPLL_LOG_INFO("Input Configkeyval or key is Null");
    return UPLL_RC_ERR_GENERIC;
  }
  if (okey != NULL) {
    UPLL_LOG_INFO("okey is Not Null");
    return UPLL_RC_ERR_GENERIC;
  }
  ConfigVal *tmp1 = NULL, *tmp = req->get_cfg_val();
  val_vtnstation_controller_st *vtnstation = NULL;
  void *val_nxt = NULL;
  for (; tmp ; tmp = tmp->get_next_cfg_val()) {
    if (tmp->get_st_num() == IpctSt::kIpcStValVtnstationControllerSt) {
      val_vtnstation_controller_st *ival =
         reinterpret_cast<val_vtnstation_controller_st *>(GetVal(req));
      vtnstation = reinterpret_cast<val_vtnstation_controller_st *>(
          ConfigKeyVal::Malloc(sizeof(val_vtnstation_controller_st)));
      memcpy(reinterpret_cast<char *>(vtnstation), reinterpret_cast<char *>
            (ival), sizeof(val_vtnstation_controller_st));
      if (!tmp1) {
        tmp1 = new ConfigVal(IpctSt::kIpcStValVtnstationControllerSt,
                             vtnstation);
      } else {
         UPLL_LOG_ERROR("val_vtnstation_controller_st is not Null");
         ConfigKeyVal::Free(vtnstation);
         delete tmp1;
         return UPLL_RC_ERR_BAD_REQUEST;
      }
    } else if (tmp->get_st_num() == IpctSt::kIpcStIpv4) {
      val_nxt = reinterpret_cast<struct in_addr *>
                                (ConfigKeyVal::Malloc(sizeof(struct in_addr)));
      memcpy(val_nxt, reinterpret_cast<struct in_addr *>(tmp->get_val()),
           sizeof(struct in_addr));
      if (!tmp1) {
        UPLL_LOG_ERROR("val_vtnstation_controller_st is Null");
        ConfigKeyVal::Free(val_nxt);
        return UPLL_RC_ERR_BAD_REQUEST;
      }
      tmp1->AppendCfgVal(new ConfigVal(IpctSt::kIpcStIpv4, val_nxt));
    } else if (tmp->get_st_num() == IpctSt::kIpcStIpv6) {
      val_nxt = reinterpret_cast<struct in6_addr* >
                                (ConfigKeyVal::Malloc(sizeof(struct in6_addr)));
      memcpy(val_nxt, reinterpret_cast<struct in6_addr *>(tmp->get_val()),
           sizeof(struct in6_addr));
      if (!tmp1) {
        UPLL_LOG_ERROR("val_vtnstation_controller_st is Null");
        ConfigKeyVal::Free(val_nxt);
        return UPLL_RC_ERR_BAD_REQUEST;
      }
      tmp1->AppendCfgVal(new ConfigVal(IpctSt::kIpcStIpv6, val_nxt));
    }
  }
  key_vtnstation_controller *ikey =
                 reinterpret_cast<key_vtnstation_controller *>(tkey);
  key_vtnstation_controller *key =
                 reinterpret_cast<key_vtnstation_controller *>
                 (ConfigKeyVal::Malloc(sizeof(key_vtnstation_controller)));
  memcpy(reinterpret_cast<char *>(key), reinterpret_cast<char *>(ikey),
         sizeof(key_vtnstation_controller));
  okey = new ConfigKeyVal(UNC_KT_VTNSTATION_CONTROLLER,
                   IpctSt::kIpcStKeyVtnstationController, key, tmp1);
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtnMoMgr::DupConfigKeyVal(ConfigKeyVal *&okey,
                                    ConfigKeyVal *&req,
                                    MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  if (req == NULL) return UPLL_RC_ERR_GENERIC;
  if (okey != NULL) return UPLL_RC_ERR_GENERIC;
  if (req->get_key_type() != UNC_KT_VTN) return UPLL_RC_ERR_GENERIC;
  ConfigVal *tmp1 = NULL, *tmp = (req)->get_cfg_val();
  val_vtn *vtn_val = NULL;
  val_vtn_gateway_port *gw_vtn = NULL;
  val_rename_vtn *rename_val = NULL;
  val_vtn_ctrlr *ctrlr_val = NULL;

  if (tmp) {
    if (tbl == MAINTBL) {
      val_vtn *ival = reinterpret_cast<val_vtn *>(GetVal(req));

      vtn_val = reinterpret_cast<val_vtn *>(
          ConfigKeyVal::Malloc(sizeof(val_vtn)));
      memcpy(reinterpret_cast<char *>(vtn_val), reinterpret_cast<char *>(ival),
             sizeof(val_vtn));
      tmp1 = new ConfigVal(IpctSt::kIpcStValVtn, vtn_val);
    } else if (tbl == RENAMETBL) {
      val_rename_vtn *ival = reinterpret_cast<val_rename_vtn *>(GetVal(req));
      rename_val = reinterpret_cast<val_rename_vtn *>(
          ConfigKeyVal::Malloc(sizeof(val_rename_vtn)));
      memcpy(reinterpret_cast<char *>(rename_val),
             reinterpret_cast<char *>(ival), sizeof(val_rename_vtn));
      tmp1 = new ConfigVal(IpctSt::kIpcStValRenameVtn, rename_val);
    } else if (tbl ==  CTRLRTBL) {
      val_vtn_ctrlr *ival = reinterpret_cast<val_vtn_ctrlr *>(GetVal(req));
      ctrlr_val = reinterpret_cast<val_vtn_ctrlr *>(
          ConfigKeyVal::Malloc(sizeof(val_vtn_ctrlr)));
      memcpy(reinterpret_cast<char *>(ctrlr_val),
             reinterpret_cast<char *>(ival), sizeof(val_vtn_ctrlr));
      tmp1 = new ConfigVal(IpctSt::kIpcInvalidStNum, ctrlr_val);
    } else if (tbl == CONVERTTBL) {
      val_vtn_gateway_port *ival = reinterpret_cast<val_vtn_gateway_port *>
                                   (GetVal(req));
      gw_vtn = ConfigKeyVal::Malloc<val_vtn_gateway_port>();
      memcpy(reinterpret_cast<char *>(gw_vtn), reinterpret_cast<char *>(ival),
          sizeof(val_vtn_gateway_port));
      tmp1 = new ConfigVal(IpctSt::kIpcStValVtnGatewayPort, gw_vtn);
    }
    if (!tmp1) {
      UPLL_LOG_ERROR("Memory Allocation failed for tmp1");
      FREE_IF_NOT_NULL(vtn_val);
      FREE_IF_NOT_NULL(gw_vtn);
      FREE_IF_NOT_NULL(rename_val);
      FREE_IF_NOT_NULL(ctrlr_val);
      return UPLL_RC_ERR_GENERIC;
    }
    //  error user data nneds to be copied
    SET_USER_DATA(tmp1, tmp)
    tmp = tmp->get_next_cfg_val();
  };
  if (tmp) {
    if (tbl == MAINTBL || tbl == CONVERTTBL) {
      val_db_vtn_st *ival = reinterpret_cast<val_db_vtn_st *>(tmp->get_val());
      val_db_vtn_st *val_vtn = reinterpret_cast<val_db_vtn_st *>(
          ConfigKeyVal::Malloc(sizeof(val_db_vtn_st)));
      memset(reinterpret_cast<void *>(val_vtn), 0, sizeof(val_db_vtn_st));
      memcpy(reinterpret_cast<char *>(val_vtn), reinterpret_cast<char *>(ival),
             sizeof(val_db_vtn_st));
      ConfigVal *tmp2 = new ConfigVal(IpctSt::kIpcStValVtnSt, val_vtn);
      tmp1->AppendCfgVal(tmp2);
    }
  };
  void *tkey = (req != NULL) ? (req)->get_key() : NULL;
  key_vtn *ikey = reinterpret_cast<key_vtn *>(tkey);
  key_vtn *vtn_key = reinterpret_cast<key_vtn *>(
      ConfigKeyVal::Malloc(sizeof(key_vtn)));
  memcpy(reinterpret_cast<char *>(vtn_key), reinterpret_cast<char *>(ikey),
         sizeof(key_vtn));
  okey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, vtn_key, tmp1);
  SET_USER_DATA(okey, req)
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtnMoMgr::AdaptValToVtnService(ConfigKeyVal *ikey,
                                         AdaptType adapt_type) {
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
    while (cval) {
      if (IpctSt::kIpcStValVtnSt == cval->get_st_num()) {
        val_vtn_st *vtn_val_st = reinterpret_cast<val_vtn_st *>
                         (ConfigKeyVal::Malloc(sizeof(val_vtn_st)));
        val_db_vtn_st *db_vtn_val_st = reinterpret_cast<val_db_vtn_st *>
                                     (cval->get_val());
        if (!db_vtn_val_st) {
          FREE_IF_NOT_NULL(vtn_val_st);
          return UPLL_RC_ERR_GENERIC;
        }
        memcpy(vtn_val_st, &(db_vtn_val_st->vtn_val_st),
               sizeof(val_vtn_st));
        cval->SetVal(IpctSt::kIpcStValVtnSt, vtn_val_st);
      }
      cval = cval->get_next_cfg_val();
    }
    if (adapt_type == ADAPT_ONE) break;
    ikey = ikey->get_next_cfg_key_val();
  }
  UPLL_LOG_DEBUG("Exiting VtnMoMgr::AdaptValToVtnService");
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtnMoMgr::CheckVtnOperStatus(ConfigKeyVal *ikey,
                                       DalDmlIntf *dmi) {
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
    while (cval) {
      if (IpctSt::kIpcStValVtnSt == cval->get_st_num()) {
        val_db_vtn_st *db_vtn_val_st = reinterpret_cast<val_db_vtn_st *>
                                     (cval->get_val());
        if (!db_vtn_val_st) {
          return UPLL_RC_ERR_GENERIC;
        }
        if (db_vtn_val_st->vtn_val_st.oper_status != UPLL_OPER_STATUS_UNKNOWN) {
          DalBindInfo dal_bind_info(uudst::kDbiVtnCtrlrTbl);
          ConfigKeyVal *ck_vtn = NULL;
          upll_rc_t result_code = GetChildConfigKey(ck_vtn, ikey);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("GetChildConfigKey failed");
            return UPLL_RC_ERR_GENERIC;
          }
          result_code = BindInfoBasedonOperation(&dal_bind_info, ck_vtn,
                                                 UNC_DT_STATE_READ);
          result_code = RecomputeVtnOperStatus(&dal_bind_info, dmi,
                                               ck_vtn, ikey);
          DELETE_IF_NOT_NULL(ck_vtn);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("Error in RecomputeVtnOperStatus");
            return result_code;
          }
        }
      }
      cval = cval->get_next_cfg_val();
    }
    ikey = ikey->get_next_cfg_key_val();
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtnMoMgr::GetChildConfigKey(ConfigKeyVal *&okey,
                                      ConfigKeyVal *parent_key) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  key_vtn_t *vtn_key = NULL;
  if (okey && okey->get_key()) {
    vtn_key = reinterpret_cast<key_vtn_t *>(
                    okey->get_key());
  } else {
    vtn_key = reinterpret_cast<key_vtn_t *>(
      ConfigKeyVal::Malloc(sizeof(key_vtn)));
  }
  void *pkey;
  if (parent_key == NULL) {
    if (!okey)
      okey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, vtn_key, NULL);
    else if (okey->get_key() != vtn_key)
      okey->SetKey(IpctSt::kIpcStKeyVtn, vtn_key);
    return UPLL_RC_SUCCESS;
  } else {
    pkey = parent_key->get_key();
  }
  if (!pkey) {
    if (!okey || !(okey->get_key()))
      ConfigKeyVal::Free(vtn_key);
    return UPLL_RC_ERR_GENERIC;
  }
  switch (parent_key->get_key_type()) {
    case UNC_KT_ROOT:
      break;
    case UNC_KT_VTN_DATAFLOW:
      uuu::upll_strncpy(vtn_key->vtn_name,
          reinterpret_cast<key_vtn_dataflow *>(pkey)->vtn_key.vtn_name,
          (kMaxLenVtnName+1));
      break;
    case UNC_KT_VTN:
      uuu::upll_strncpy(vtn_key->vtn_name,
          reinterpret_cast<key_vtn *>(pkey)->vtn_name,
          (kMaxLenVtnName+1));
      break;
    case UNC_KT_VTN_MAPPING_CONTROLLER:
      uuu::upll_strncpy(vtn_key->vtn_name,
          reinterpret_cast<key_vtn_controller *>(pkey)->vtn_key.vtn_name,
          (kMaxLenVtnName+1));
      break;
    case UNC_KT_VBR_PORTMAP:
      uuu::upll_strncpy(vtn_key->vtn_name,
         reinterpret_cast<key_vbr_portmap*>(pkey)->vbr_key.vtn_key.vtn_name,
         (kMaxLenVtnName+1));
      break;
    case UNC_KT_VBR_IF:
      if (parent_key->get_st_num() == IpctSt::kIpcStKeyConvertVbrIf) {
        uuu::upll_strncpy(vtn_key->vtn_name,
         reinterpret_cast<key_convert_vbr_if*>
         (pkey)->convert_vbr_key.vbr_key.vtn_key.vtn_name,
         (kMaxLenVtnName+1));
      } else {
        uuu::upll_strncpy(vtn_key->vtn_name,
            reinterpret_cast<key_vbr_if*>(pkey)->vbr_key.vtn_key.vtn_name,
            (kMaxLenVtnName+1));
      }
      break;
    default:
      if (!okey || !(okey->get_key())) {
        ConfigKeyVal::Free(vtn_key);
      }
      return UPLL_RC_ERR_GENERIC;
  }
  if (!okey)
    okey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, vtn_key, NULL);
  else if (okey->get_key() != vtn_key)
    okey->SetKey(IpctSt::kIpcStKeyVtn, vtn_key);
  SET_USER_DATA(okey, parent_key);
  return result_code;
}

upll_rc_t VtnMoMgr::GetRenamedUncKey(ConfigKeyVal *ctrlr_key,
                                     upll_keytype_datatype_t dt_type,
                                     DalDmlIntf *dmi,
                                     uint8_t *ctrlr_id) {
  UPLL_FUNC_TRACE;
  if (!ctrlr_key || !(ctrlr_key->get_key()) ) return UPLL_RC_ERR_GENERIC;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  key_vtn *ctrlr_vtn_key = reinterpret_cast<key_vtn *>(ctrlr_key->get_key());
  DbSubOp dbop = { kOpReadSingle, kOpMatchCtrlr, kOpInOutNone };
  ConfigKeyVal *unc_key = NULL;
  val_rename_vtn *rename_vtn = reinterpret_cast<val_rename_vtn *>(
      ConfigKeyVal::Malloc(sizeof(val_rename_vtn)));
  uuu::upll_strncpy(reinterpret_cast<char*>(rename_vtn->new_name),
                  reinterpret_cast<char*>(ctrlr_vtn_key->vtn_name),
                  kMaxLenVtnName+1);
  rename_vtn->valid[UPLL_IDX_NEW_NAME_RVTN] = UNC_VF_VALID;
  result_code = GetChildConfigKey(unc_key, NULL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG(" GetChildConfigKey Failed with result_code %d",
                        result_code);
    ConfigKeyVal::Free(rename_vtn);
    return UPLL_RC_ERR_GENERIC;
  }
  unc_key->AppendCfgVal(IpctSt::kIpcStValRenameVtn, rename_vtn);
  if (ctrlr_id) {
    SET_USER_DATA_CTRLR(unc_key, ctrlr_id);
  } else  {
    dbop.matchop = kOpMatchNone;
  }
  result_code = ReadConfigDB(unc_key, dt_type, UNC_OP_READ, dbop, dmi,
                             RENAMETBL);
  if (result_code == UPLL_RC_SUCCESS) {
    key_vtn *vtn_key = reinterpret_cast<key_vtn *>(unc_key->get_key());
    if (strcmp(reinterpret_cast<char*>(ctrlr_vtn_key->vtn_name),
               reinterpret_cast<char*>(vtn_key->vtn_name))) {
      uuu::upll_strncpy(ctrlr_vtn_key->vtn_name,
              vtn_key->vtn_name,
              (kMaxLenVtnName+ 1));
      SET_USER_DATA_FLAGS(ctrlr_key, VTN_RENAME);
    }
  }
  delete unc_key;
  return result_code;
}

upll_rc_t VtnMoMgr::GetRenamedControllerKey(ConfigKeyVal *ikey,
                              upll_keytype_datatype_t dt_type,
                              DalDmlIntf *dmi,
                              controller_domain *ctrlr_dom) {
    UPLL_FUNC_TRACE;
    upll_rc_t result_code = UPLL_RC_SUCCESS;
    ConfigKeyVal *okey = NULL;
    uint8_t rename = 0;
    key_vtn *ctrlr_key = NULL;
    result_code = IsRenamed(ikey, dt_type, dmi, rename);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("IsRenamed failed. Result : %d", result_code);
      return result_code;
    }
    if (!rename) return UPLL_RC_SUCCESS;

    /* vtn renamed */
    if (rename & 0x01) {
//      GET_USER_DATA_CTRLR_DOMAIN(ikey, *ctrlr_dom);
      if (!ctrlr_dom->ctrlr || !ctrlr_dom->domain) {
        UPLL_LOG_ERROR("Illegal controller domain");
        return UPLL_RC_ERR_GENERIC;
      }
      result_code = GetChildConfigKey(okey, ikey);
      if (result_code != UPLL_RC_SUCCESS || !okey) {
        UPLL_LOG_DEBUG("GetChildConfigKey failed");
        return result_code;
      }
      // SET_USER_DATA_CTRLR_DOMAIN(okey, *ctrlr_dom);
      DbSubOp dbop = {kOpReadSingle, kOpMatchCtrlr | kOpMatchDomain,
        kOpInOutFlag};
      result_code = ReadConfigDB(okey, dt_type, UNC_OP_READ, dbop, dmi,
                       RENAMETBL);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("ReadConfigDB failed with result_code %d",
                           result_code);
        DELETE_IF_NOT_NULL(okey);
        return result_code;
      }
      val_rename_vtn *rename_val = reinterpret_cast<val_rename_vtn *>
      (GetVal(okey));
      if (!rename_val) {
         UPLL_LOG_DEBUG("Rename Val is Empty");
         DELETE_IF_NOT_NULL(okey);
         return UPLL_RC_ERR_GENERIC;
      }
      ctrlr_key = reinterpret_cast<key_vtn *>(ikey->get_key());
      if (!ctrlr_key) return UPLL_RC_ERR_GENERIC;
      memset(ctrlr_key, 0, sizeof(key_vtn));
      uuu::upll_strncpy(ctrlr_key->vtn_name, rename_val->new_name,
                        (kMaxLenVtnName + 1));
      SET_USER_DATA_FLAGS(ikey, VTN_RENAME);
      DELETE_IF_NOT_NULL(okey);
    }
    return UPLL_RC_SUCCESS;
}

upll_rc_t
VtnMoMgr::GetControllerDomainSpan(ConfigKeyVal *ikey,
    upll_keytype_datatype_t dt_type,
    DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code;
  DbSubOp dbop = {kOpReadMultiple, kOpMatchNone,
    kOpInOutCtrlr|kOpInOutDomain};
  result_code = ReadConfigDB(ikey, dt_type, UNC_OP_READ, dbop, dmi, CTRLRTBL);
  return result_code;
}

// To get the controller table count corresponding to the domains
// in which normal vnodes are created.
upll_rc_t
VtnMoMgr::GetNormalDomainCtrlrTableCount(
    ConfigKeyVal *ikey, upll_keytype_datatype_t dt_type,
    DalDmlIntf *dmi, uint32_t *count) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  DbSubOp dbop = {kOpReadMultiple, kOpMatchNone, kOpInOutNone};
  result_code = ReadConfigDB(ikey, dt_type, UNC_OP_READ, dbop, dmi, CTRLRTBL);
  if ((result_code != UPLL_RC_SUCCESS) &&
      (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE)) {
    UPLL_LOG_DEBUG("ReadConfigDB failed %d", result_code);
    return result_code;
  } else if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    *count = 0;
    return UPLL_RC_SUCCESS;
  }
  for (ConfigKeyVal *tmp_ckv = ikey; tmp_ckv;
       tmp_ckv = tmp_ckv->get_next_cfg_key_val()) {
    val_vtn_ctrlr_t *ctr_val = reinterpret_cast<val_vtn_ctrlr_t *>(
        GetVal(tmp_ckv));
    if (ctr_val->vnode_ref_cnt > 0) {
      (*count)++;
    }
  }
  return result_code;
}

upll_rc_t
VtnMoMgr::GetControllerDomainSpanForPOM(
    ConfigKeyVal *ikey, upll_keytype_datatype_t dt_type,
    DalDmlIntf *dmi, std::list<controller_domain_t> &list_ctrlr_dom) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code;
  ConfigKeyVal *tmp_ikey = NULL;
  controller_domain ctrlr_dom;
  ctrlr_dom.ctrlr = NULL;
  ctrlr_dom.domain = NULL;
  DbSubOp dbop = {kOpReadMultiple, kOpMatchNone,
    kOpInOutCtrlr|kOpInOutDomain};
  result_code = ReadConfigDB(ikey, dt_type, UNC_OP_READ, dbop, dmi, CTRLRTBL);
  if ((result_code != UPLL_RC_SUCCESS) &&
      (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE)) {
    UPLL_LOG_DEBUG("Error in ReadConfigDb (%d)", result_code);
    return result_code;
  }
  if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    UPLL_LOG_DEBUG(" Vtn not yet associated with controller ");
    return result_code;
  }

  for (tmp_ikey = ikey; tmp_ikey;
       tmp_ikey = tmp_ikey->get_next_cfg_key_val()) {
    val_vtn_ctrlr *ctr_val =
        reinterpret_cast<val_vtn_ctrlr *>(GetVal(tmp_ikey));
    if (ctr_val->vnode_ref_cnt > 0) {
      ctrlr_dom.ctrlr = reinterpret_cast<uint8_t *>
          (ConfigKeyVal::Malloc((kMaxLenCtrlrId + 1)));
      ctrlr_dom.domain = reinterpret_cast<uint8_t *>
          (ConfigKeyVal::Malloc((kMaxLenDomainId + 1)));
      controller_domain_t tmp_ctrlr_dom;
      tmp_ctrlr_dom.ctrlr = tmp_ctrlr_dom.domain = NULL;
      GET_USER_DATA_CTRLR_DOMAIN(tmp_ikey, tmp_ctrlr_dom);
      UPLL_LOG_TRACE("ctrlr = %s, dom = %s", tmp_ctrlr_dom.ctrlr,
                     tmp_ctrlr_dom.domain);
      uuu::upll_strncpy(ctrlr_dom.ctrlr, tmp_ctrlr_dom.ctrlr,
                        (kMaxLenCtrlrId + 1));
      uuu::upll_strncpy(ctrlr_dom.domain, tmp_ctrlr_dom.domain,
                        (kMaxLenDomainId + 1));
      list_ctrlr_dom.push_back(ctrlr_dom);
    }
  }
  return result_code;
}

upll_rc_t
VtnMoMgr::UpdateVtnConfigStatus(ConfigKeyVal *vtn_key,
    unc_keytype_operation_t op, uint32_t driver_result,
    ConfigKeyVal *nreq, DalDmlIntf *dmi) {
  ConfigKeyVal *ck_vtn = NULL;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  val_vtn_t *vtn_val = NULL;
  val_vtn_st *val_vtnst= NULL;
  void *vtnval = NULL;
  void *nvtnval = NULL;
  val_db_vtn_st *vtn_val_db_st = NULL, *nreq_vtnst = NULL;
  string vtn_id = "";

  UPLL_FUNC_TRACE;
  if (op != UNC_OP_DELETE) {
    result_code = DupConfigKeyVal(ck_vtn, vtn_key, MAINTBL);
    if (!ck_vtn || result_code != UPLL_RC_SUCCESS) {
       UPLL_LOG_DEBUG("Returning error %d", result_code);
       return UPLL_RC_ERR_GENERIC;
    }
    vtn_val = reinterpret_cast<val_vtn_t *>(GetVal(ck_vtn));
    if (!vtn_val) {
       UPLL_LOG_DEBUG("invalid val ");
       return UPLL_RC_ERR_GENERIC;
    }
    vtn_val_db_st =  reinterpret_cast<val_db_vtn_st *>(
        ConfigKeyVal::Malloc(sizeof(val_db_vtn_st)));
    val_vtnst = &(vtn_val_db_st->vtn_val_st);
  } else {
    result_code = GetChildConfigKey(ck_vtn, vtn_key);
    if (!ck_vtn || result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Returning error %d", result_code);
      return UPLL_RC_ERR_GENERIC;
    }
  }
  switch (op) {
  case UNC_OP_CREATE:
    vtn_val->cs_row_status = UNC_CS_APPLIED;
    val_vtnst->creation_time = time(NULL);
    val_vtnst->last_updated_time = val_vtnst->creation_time;
    val_vtnst->valid[UPLL_IDX_CREATION_TIME_VS] = UNC_VF_VALID;
    val_vtnst->valid[UPLL_IDX_LAST_UPDATE_TIME_VS] = UNC_VF_VALID;
    val_vtnst->oper_status = UPLL_OPER_STATUS_DOWN;
    val_vtnst->alarm_status = UPLL_ALARM_CLEAR;
    val_vtnst->valid[UPLL_IDX_OPER_STATUS_VS] = UNC_VF_VALID;
    val_vtnst->valid[UPLL_IDX_ALARM_STATUS_VS] = UNC_VF_VALID;
    vtn_val_db_st->down_count = 0;
    vtn_val_db_st->unknown_count = 0;
    ck_vtn->AppendCfgVal(IpctSt::kIpcStValVtnSt, vtn_val_db_st);
    break;
  case UNC_OP_UPDATE:
    vtnval = reinterpret_cast<void *>(vtn_val);
    nvtnval = (nreq)?GetVal(nreq):NULL;
    if (!nvtnval) {
      UPLL_LOG_DEBUG("Invalid param");
      return UPLL_RC_ERR_GENERIC;
    }
    CompareValidValue(vtnval, nvtnval, true);

    vtn_val->cs_row_status =
             reinterpret_cast<val_vtn_t *>(GetVal(nreq))->cs_row_status;

    val_vtnst->last_updated_time = time(NULL);
    val_vtnst->valid[UPLL_IDX_LAST_UPDATE_TIME_VS] = UNC_VF_VALID;
    val_vtnst->valid[UPLL_IDX_OPER_STATUS_VS] = UNC_VF_INVALID;
    val_vtnst->valid[UPLL_IDX_ALARM_STATUS_VS] = UNC_VF_INVALID;
    nreq_vtnst = reinterpret_cast<val_db_vtn_st *>(GetStateVal(nreq));
    if (!nreq_vtnst) {
      UPLL_LOG_DEBUG("Invalid param");
      return UPLL_RC_ERR_GENERIC;
    }
    vtn_val_db_st->vtn_val_st.oper_status=
                    nreq_vtnst->vtn_val_st.oper_status;
    vtn_val_db_st->down_count = nreq_vtnst->down_count;
    vtn_val_db_st->unknown_count = nreq_vtnst->unknown_count;
    ck_vtn->AppendCfgVal(IpctSt::kIpcStValVtnSt, val_vtnst);
    break;
  case UNC_OP_DELETE:
#if 0
    result_code = UpdateConfigDB(ck_vtn, UPLL_DT_CANDIDATE, UNC_OP_READ,
                  dmi, CTRLRTBL);
    if (result_code == UPLL_RC_ERR_INSTANCE_EXISTS) {
      string s(ck_vtn->ToStr());
      UPLL_LOG_DEBUG("Instance exists in ctrlr table - not deleted %s",
                      s.c_str());
      delete ck_vtn;
      return UPLL_RC_SUCCESS;
    } else if (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      string s(ck_vtn->ToStr());
      UPLL_LOG_DEBUG("Ctrlr table exists read failed- %s", s.c_str());
      delete ck_vtn;
      return result_code;
    }
#endif
    break;
  default:
    UPLL_LOG_DEBUG("Invalid operation");
    return UPLL_RC_ERR_GENERIC;
  }
  if ((op != UNC_OP_DELETE) &&
      (vtn_val->valid[UPLL_IDX_DESC_VTN] != UNC_VF_INVALID)) {
    vtn_val->cs_attr[UPLL_IDX_DESC_VTN] = UNC_CS_APPLIED;
  }
  DbSubOp dbop = {kOpNotRead, kOpMatchNone, kOpInOutFlag | kOpInOutCs};
  result_code = UpdateConfigDB(ck_vtn, UPLL_DT_STATE, op, dmi, &dbop,
                               TC_CONFIG_GLOBAL, vtn_id, MAINTBL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("UpdateConfigDB Failed %d", result_code);
    return result_code;
  }
  result_code = EnqueCfgNotification(op, UPLL_DT_RUNNING, ck_vtn);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("EnqueCfgNotification Failed %d", result_code);
    return result_code;
  }
  delete ck_vtn;
  return result_code;
}

upll_rc_t
VtnMoMgr::UpdateConfigStatus(ConfigKeyVal *vtn_run,
    unc_keytype_operation_t op, uint32_t driver_result, ConfigKeyVal *ctrlr_can,
    DalDmlIntf *dmi, ConfigKeyVal *ctrlr_run) {
  UPLL_FUNC_TRACE;
  unc_keytype_configstatus_t ctrlr_status;
  uint8_t cs_status;
  bool attribute_change = false;
  ctrlr_status = (driver_result == UPLL_RC_SUCCESS)?
                 UNC_CS_APPLIED: UNC_CS_NOT_APPLIED;
  val_vtn_t *vtn_val = reinterpret_cast<val_vtn_t *>(GetVal(vtn_run));
  val_vtn_ctrlr *ctrlr_val_vtn = NULL;
  if (ctrlr_can == NULL) {
    attribute_change = true;
  } else {
    ctrlr_val_vtn = reinterpret_cast<val_vtn_ctrlr *>
                                                 (GetVal(ctrlr_can));
  }
  if (vtn_val == NULL)
    return UPLL_RC_ERR_GENERIC;
  cs_status = vtn_val->cs_row_status;
  UPLL_LOG_TRACE("cs_status %d ctrlr_status %d\n", cs_status, ctrlr_status);
  uuc::CtrlrMgr *ctrlr_mgr = uuc::CtrlrMgr::GetInstance();
  unc_keytype_ctrtype_t ctrlrtype = UNC_CT_UNKNOWN;
  if (ctrlr_can && ctrlr_mgr) {
    uint8_t *ctrlr_id = NULL;
    GET_USER_DATA_CTRLR(ctrlr_can, ctrlr_id);
    if (!ctrlr_id) {
      UPLL_LOG_DEBUG("Returning error\n");
      return UPLL_RC_ERR_GENERIC;
    }
    bool return_value = ctrlr_mgr->GetCtrlrType(
        reinterpret_cast<char *>(ctrlr_id),
        UPLL_DT_CANDIDATE, &ctrlrtype);
    if (false == return_value) {
      UPLL_LOG_DEBUG("Unknown Controller Type\n");
      return UPLL_RC_ERR_GENERIC;
    }
  }
  if (op == UNC_OP_CREATE) {
    if (ctrlr_val_vtn == NULL || ctrlr_can == NULL) return UPLL_RC_ERR_GENERIC;
    controller_domain ctrlr_dom = {NULL, NULL};
    GET_USER_DATA_CTRLR_DOMAIN(ctrlr_can, ctrlr_dom);
    uuc::UpllConfigMgr *cfg_instance =
        uuc::UpllConfigMgr::GetUpllConfigMgr();
    const char *ctr_name = reinterpret_cast<const char*>(ctrlr_dom.ctrlr);
    const char *dom_name = reinterpret_cast<const char*>(ctrlr_dom.domain);
    UPLL_LOG_DEBUG("first vNode create on a VTN  for ctr: %s, dom:%s \n",
                   ctr_name, dom_name);
    if (uuc::CtrlrMgr::GetInstance()->IsPathFaultOccured(ctr_name, dom_name)) {
      key_vtn *vtn = static_cast<key_vtn *>(ctrlr_can->get_key());
      const char *vtn_name = reinterpret_cast<const char*>(vtn->vtn_name);
      bool bResult(false);
      bResult = cfg_instance->SendPathFaultAlarm(ctr_name, dom_name,
                                                 vtn_name,
                                                 uuc::UPLL_ASSERT_WITH_TRAP);
      if (!bResult) {
        UPLL_LOG_DEBUG("Pathfault Alarm creation is failed");
      }
    }
    ctrlr_val_vtn->oper_status =
         (driver_result == UPLL_RC_ERR_CTR_DISCONNECTED)?
          UPLL_OPER_STATUS_UNKNOWN : UPLL_OPER_STATUS_UP;
    if (ctrlr_val_vtn->ref_cnt2 > 0 && ctrlr_val_vtn->vnode_ref_cnt == 0)
      ctrlr_val_vtn->oper_status = UPLL_OPER_STATUS_UP;
    ctrlr_val_vtn->down_count = 0;
    ctrlr_val_vtn->unknown_count = 0;
    ctrlr_val_vtn->alarm_status = UPLL_ALARM_CLEAR;
    ctrlr_val_vtn->valid[0] = UNC_VF_VALID;
    ctrlr_val_vtn->valid[1] = UNC_VF_VALID;
    if (ctrlrtype == UNC_CT_PFC) {
      ctrlr_val_vtn->oper_status =
            (driver_result == UPLL_RC_ERR_CTR_DISCONNECTED)?
            UPLL_OPER_STATUS_UNKNOWN : UPLL_OPER_STATUS_UP;
      if (ctrlr_val_vtn->ref_cnt2 > 0 && ctrlr_val_vtn->vnode_ref_cnt == 0)
        ctrlr_val_vtn->oper_status = UPLL_OPER_STATUS_UP;
    }
    ctrlr_val_vtn->cs_row_status = ctrlr_status;

    /* update the vtn status in main tbl */
    switch (vtn_val->cs_row_status) {
    case UNC_CS_APPLIED: {
      cs_status = (ctrlr_status == UNC_CS_NOT_APPLIED) ?
                   UNC_CS_PARTIALLY_APPLIED : ctrlr_status;
      }
      break;
    case UNC_CS_NOT_APPLIED: {
      cs_status = (ctrlr_status == UNC_CS_APPLIED) ?
                   UNC_CS_PARTIALLY_APPLIED : ctrlr_status;
      }
      break;
    case UNC_CS_PARTIALLY_APPLIED:
      cs_status = UNC_CS_PARTIALLY_APPLIED;
      break;
    case UNC_CS_INVALID:
      cs_status = UNC_CS_INVALID;
      break;
    case UNC_CS_UNKNOWN:
    default:
        /* first entry in ctrlr table */
      cs_status = ctrlr_status;
    }
    vtn_val->cs_row_status = cs_status;
  }
  // description is always applied
  vtn_val->cs_attr[UPLL_IDX_DESC_VTN] = UNC_CS_APPLIED;

  // Updating the Controller cs_row_status
  // Main tbl update, pass ctrlr_run = NULL
  val_vtn_ctrlr *run_ctrlr_val = reinterpret_cast<val_vtn_ctrlr *>
                                                 (GetVal(ctrlr_run));
  if ((op == UNC_OP_UPDATE) && (run_ctrlr_val != NULL)) {
    if (attribute_change) {
      cs_status = run_ctrlr_val->cs_row_status;
      run_ctrlr_val->cs_row_status = unc_keytype_configstatus_t(cs_status);
      if (vtn_val->valid[UPLL_IDX_DESC_VTN] != UNC_VF_INVALID) {
        if (run_ctrlr_val->cs_attr[UPLL_IDX_DESC_VTN] != UNC_CS_NOT_SUPPORTED)
          run_ctrlr_val->cs_attr[UPLL_IDX_DESC_VTN] = UNC_CS_APPLIED;
        else
          run_ctrlr_val->cs_attr[UPLL_IDX_DESC_VTN] = UNC_CS_NOT_SUPPORTED;
      }
      return UPLL_RC_SUCCESS;
    } else {
      if (!ctrlr_val_vtn) {
        UPLL_LOG_DEBUG("Controller Value is NULL");
        return UPLL_RC_ERR_GENERIC;
      }
      ctrlr_val_vtn->cs_row_status = run_ctrlr_val->cs_row_status;
      ctrlr_val_vtn->oper_status = run_ctrlr_val->oper_status;
      ctrlr_val_vtn->down_count= run_ctrlr_val->down_count;
      ctrlr_val_vtn->unknown_count= run_ctrlr_val->unknown_count;
    }
  }
  val_db_vtn_st *vtn_val_db_st = reinterpret_cast<val_db_vtn_st *>
                                                     (GetStateVal(vtn_run));
  if (!vtn_val_db_st) {
     UPLL_LOG_DEBUG("Returning error %d\n", UPLL_RC_ERR_GENERIC);
     return UPLL_RC_ERR_GENERIC;
  }
  val_vtn_st *val_vtnst = &(vtn_val_db_st->vtn_val_st);
  if (driver_result != UPLL_RC_ERR_CTR_DISCONNECTED) {
    if ((op == UNC_OP_CREATE) &&
        (val_vtnst->oper_status != UPLL_OPER_STATUS_UNKNOWN)) {
      if (vtn_val_db_st->unknown_count > 0) {
        val_vtnst->oper_status = UPLL_OPER_STATUS_UNKNOWN;
      } else if (vtn_val_db_st->down_count > 0) {
        val_vtnst->oper_status = UPLL_OPER_STATUS_DOWN;
      } else {
        val_vtnst->oper_status = UPLL_OPER_STATUS_UP;
      }
    }
  } else {
    val_vtnst->oper_status = UPLL_OPER_STATUS_UNKNOWN;
  }

  val_vtnst->valid[UPLL_IDX_OPER_STATUS_VS] = UNC_VF_VALID;
  if (ctrlr_val_vtn && (vtn_val->valid[UPLL_IDX_DESC_VTN] != UNC_VF_INVALID)) {
    if (ctrlr_val_vtn->cs_attr[UPLL_IDX_DESC_VTN] != UNC_CS_NOT_SUPPORTED)
      ctrlr_val_vtn->cs_attr[UPLL_IDX_DESC_VTN] = ctrlr_status;
    else
      ctrlr_val_vtn->cs_attr[UPLL_IDX_DESC_VTN] = UNC_CS_NOT_SUPPORTED;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t
VtnMoMgr::TxCopyCandidateToRunning(unc_key_type_t keytype,
                                   CtrlrCommitStatusList *ctrlr_commit_status,
                                   DalDmlIntf *dmi, TcConfigMode config_mode,
                                   string vtn_name) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  DalResultCode db_result;
  unc_keytype_operation_t op[] = {UNC_OP_DELETE,
                                  UNC_OP_CREATE,
                                  UNC_OP_UPDATE };
  int nop = sizeof(op) / sizeof(op[0]);
  ConfigKeyVal *vtn_ck_run = NULL, *req = NULL, *nreq = NULL;
  DalCursor *cfg1_cursor;
  uint8_t *ctrlr_id = NULL;
  map<string, int> ctrlr_result;
  CtrlrCommitStatusList::iterator ccsListItr;
  CtrlrCommitStatus *ccStatusPtr;

  // mode is virtual and so ignore it
  if (config_mode == TC_CONFIG_VIRTUAL) {
    return UPLL_RC_SUCCESS;
  }

  if (ctrlr_commit_status != NULL) {
    for (ccsListItr = ctrlr_commit_status->begin();
        ccsListItr != ctrlr_commit_status->end(); ++ccsListItr) {
      ccStatusPtr = *ccsListItr;
      ctrlr_id = reinterpret_cast<uint8_t*>
        (const_cast<char*>(ccStatusPtr->ctrlr_id.c_str()));
      ctrlr_result[ccStatusPtr->ctrlr_id] = ccStatusPtr->upll_ctrlr_result;
      if (ccStatusPtr->upll_ctrlr_result != UPLL_RC_SUCCESS) {
        for (ConfigKeyVal *ck_err = ccStatusPtr->err_ckv; ck_err != NULL;
            ck_err = ck_err->get_next_cfg_key_val()) {
          if (ck_err->get_key_type() != keytype) continue;
          result_code = GetRenamedUncKey(ck_err, UPLL_DT_CANDIDATE,
              dmi, ctrlr_id);
          if (result_code != UPLL_RC_SUCCESS &&
              result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
            UPLL_LOG_DEBUG("GetRenamedUncKey failed. Result : %d",
                result_code);
            return result_code;
          }
        }
      }
    }
  }

  for (int i = 0; i < nop; i++) {
    cfg1_cursor = NULL;
    result_code = DiffConfigDB(UPLL_DT_CANDIDATE, UPLL_DT_RUNNING, op[i],
                          req, nreq, &cfg1_cursor, dmi, NULL, config_mode,
                          vtn_name, MAINTBL, true);
    if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      result_code = UPLL_RC_SUCCESS;
      UPLL_LOG_DEBUG("Diff Skipped for op %d", op[i]);
      DELETE_IF_NOT_NULL(req);
      DELETE_IF_NOT_NULL(nreq);
      continue;
    }
    if (result_code != UPLL_RC_SUCCESS || cfg1_cursor == NULL) {
      UPLL_LOG_DEBUG("Cursor not populated");
      DELETE_IF_NOT_NULL(req);
      DELETE_IF_NOT_NULL(nreq);
      return result_code;
    }
    ConfigKeyVal *vtn_ctrlr_key = NULL;
    while (result_code == UPLL_RC_SUCCESS) {
      vtn_ctrlr_key = NULL;
      bool upd_ctrlr = false;
      db_result = dmi->GetNextRecord(cfg1_cursor);
      result_code = DalToUpllResCode(db_result);
      if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
        result_code = UPLL_RC_SUCCESS;
        break;
      }
      if (op[i] == UNC_OP_UPDATE) {
        DbSubOp dbop = {kOpReadMultiple, kOpMatchNone,
                        kOpInOutCtrlr | kOpInOutDomain | kOpInOutCs };
        result_code = GetChildConfigKey(vtn_ctrlr_key, req);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("GetChildConfigKey Failed");
          return result_code;
        }
        result_code = ReadConfigDB(vtn_ctrlr_key, UPLL_DT_RUNNING,
                                   UNC_OP_READ, dbop, dmi, CTRLRTBL);
        if (result_code == UPLL_RC_SUCCESS) {
          upd_ctrlr = true;
        } else {
          DELETE_IF_NOT_NULL(vtn_ctrlr_key);
          if (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
            if (cfg1_cursor)
              dmi->CloseCursor(cfg1_cursor, true);
            delete req;
            delete nreq;
            return result_code;
          }
        }
      }
      if (op[i] == UNC_OP_DELETE) {
        DbSubOp dbop = {kOpReadMultiple, kOpMatchNone,
                        kOpInOutCtrlr | kOpInOutDomain | kOpInOutCs };
        result_code = GetChildConfigKey(vtn_ctrlr_key, req);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("GetChildConfigKey Failed");
          return result_code;
        }
        result_code = ReadConfigDB(vtn_ctrlr_key, UPLL_DT_RUNNING,
                                   UNC_OP_READ, dbop, dmi, CTRLRTBL);
        if (result_code == UPLL_RC_SUCCESS) {
          bool send_snmp_trap(true);
          while (vtn_ctrlr_key != NULL) {
            ConfigKeyVal *ck_vtn1 = vtn_ctrlr_key->get_next_cfg_key_val();
            vtn_ctrlr_key->set_next_cfg_key_val(NULL);
            // clear path fault alarm if vtn delete
            controller_domain ctrlr_dom;
            ctrlr_dom.ctrlr = NULL;
            ctrlr_dom.domain = NULL;
            GET_USER_DATA_CTRLR_DOMAIN(vtn_ctrlr_key, ctrlr_dom);
            uuc::UpllConfigMgr *cfg_instance =
                uuc::UpllConfigMgr::GetUpllConfigMgr();
            const char *ctr_name =
                reinterpret_cast<const char*>(ctrlr_dom.ctrlr);
            const char *dom_name =
                reinterpret_cast<const char*>(ctrlr_dom.domain);
            UPLL_LOG_DEBUG("vtn delete for ctr: %s, dom:%s \n",
                           ctr_name, dom_name);
            if (uuc::CtrlrMgr::GetInstance()->IsPathFaultOccured(
                    ctr_name, dom_name)) {
              key_vtn *vtn = static_cast<key_vtn *>(vtn_ctrlr_key->get_key());
              const char *vtn_name1 =
                  reinterpret_cast<const char*>(vtn->vtn_name);
              bool bResult(false);
              if (send_snmp_trap) {
                bResult = cfg_instance->SendPathFaultAlarm(
                    "*", "*", vtn_name1, uuc::UPLL_CLEAR_WITH_TRAP);
                send_snmp_trap = false;
              }
              bResult = cfg_instance->SendPathFaultAlarm(
                  ctr_name, dom_name, vtn_name1, uuc::UPLL_CLEAR_WITHOUT_TRAP);
              if (!bResult) {
                UPLL_LOG_DEBUG("Pathfault Alarm clear is failed");
              }
            }
            DELETE_IF_NOT_NULL(vtn_ctrlr_key);
            vtn_ctrlr_key = ck_vtn1;
          }
        } else {
          DELETE_IF_NOT_NULL(vtn_ctrlr_key);
          if (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
            if (cfg1_cursor)
              dmi->CloseCursor(cfg1_cursor, true);
            delete req;
            delete nreq;
            return result_code;
          }
        }
      }
      result_code = UpdateVtnConfigStatus(req, op[i], UPLL_RC_SUCCESS,
                                          nreq, dmi);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Error updating vtn config status %d",
                        result_code);
        if (cfg1_cursor)
          dmi->CloseCursor(cfg1_cursor, true);
        delete req;
        DELETE_IF_NOT_NULL(vtn_ctrlr_key);
        if (nreq) delete nreq;
        return result_code;
      }
      if (upd_ctrlr) {
        result_code = DupConfigKeyVal(vtn_ck_run, req, MAINTBL);
        if (result_code != UPLL_RC_SUCCESS) {
          if (cfg1_cursor)
            dmi->CloseCursor(cfg1_cursor, true);
          delete req;
          delete nreq;
          DELETE_IF_NOT_NULL(vtn_ctrlr_key);
          return result_code;
        }
        // Assigning cs_row_status from existing controller
        static_cast<val_vtn *>(GetVal(vtn_ck_run))->cs_row_status =
              static_cast<val_vtn *>(GetVal(nreq))->cs_row_status;
        for (ConfigKeyVal *tmp = vtn_ctrlr_key, *tmp_nxt; tmp!= NULL;
             tmp = tmp_nxt) {
          tmp_nxt = tmp->get_next_cfg_key_val();
          tmp->set_next_cfg_key_val(NULL);
          GET_USER_DATA_CTRLR(tmp, ctrlr_id);
          string controller(reinterpret_cast<char *>(ctrlr_id));
          if (ctrlr_result.empty()) {
            UPLL_LOG_TRACE("ctrlr_commit_status is NULL.");
            result_code = UpdateConfigStatus(vtn_ck_run, op[i],
                          UPLL_RC_ERR_CTR_DISCONNECTED, NULL, dmi, tmp);
            if (result_code != UPLL_RC_SUCCESS)
              break;
          } else {
            result_code = UpdateConfigStatus(vtn_ck_run, op[i],
                          ctrlr_result[controller], NULL, dmi, tmp);
            if (result_code != UPLL_RC_SUCCESS)
              break;
          }
          DbSubOp dbop_update = {kOpNotRead, kOpMatchCtrlr | kOpMatchDomain,
                                 kOpInOutCs};
          result_code = UpdateConfigDB(tmp, UPLL_DT_RUNNING, op[i],
                                       dmi, &dbop_update, config_mode,
                                       vtn_name, CTRLRTBL);
          if (result_code != UPLL_RC_SUCCESS) {
            if (cfg1_cursor)
              dmi->CloseCursor(cfg1_cursor, true);
            delete req;
            delete nreq;
            delete tmp;
            delete tmp_nxt;
            DELETE_IF_NOT_NULL(vtn_ck_run);
            return result_code;
          }
          delete tmp;
        }
        vtn_ctrlr_key = NULL;
        result_code = UpdateConfigDB(vtn_ck_run, UPLL_DT_RUNNING, op[i],
              dmi, config_mode, vtn_name, MAINTBL);
        if (result_code != UPLL_RC_SUCCESS) {
          if (cfg1_cursor)
            dmi->CloseCursor(cfg1_cursor, true);
          delete req;
          delete nreq;
          DELETE_IF_NOT_NULL(vtn_ck_run);
          return result_code;
        }
        DELETE_IF_NOT_NULL(vtn_ck_run);
      }
    }
    if (cfg1_cursor)
      dmi->CloseCursor(cfg1_cursor, true);
    if (req)
      delete req;
    if (nreq) delete nreq;
    req = nreq = NULL;
  }
  for (int i = 0; i < nop; i++) {
    cfg1_cursor = NULL;
    result_code = DiffConfigDB(UPLL_DT_CANDIDATE, UPLL_DT_RUNNING, op[i], req,
                               nreq, &cfg1_cursor, dmi, NULL, config_mode,
                               vtn_name, CTRLRTBL, true);
    if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      result_code = UPLL_RC_SUCCESS;
      UPLL_LOG_DEBUG("Diff Skipped for op %d", op[i]);
      DELETE_IF_NOT_NULL(req);
      DELETE_IF_NOT_NULL(nreq);
      result_code = TxCopyRenameTableFromCandidateToRunning(keytype,
                                                            op[i], dmi,
                                                            config_mode,
                                                            vtn_name);
      UPLL_LOG_DEBUG("TxCopyRenameTableFromCandidateToRunning returned %d",
                     result_code);
      continue;
    }
    if (result_code != UPLL_RC_SUCCESS || cfg1_cursor == NULL) {
      UPLL_LOG_DEBUG("Cursor not populated");
      return result_code;
    }
    ConfigKeyVal *vtn_ctrlr_key = NULL;
    while (result_code == UPLL_RC_SUCCESS) {
      db_result = dmi->GetNextRecord(cfg1_cursor);
      result_code = DalToUpllResCode(db_result);
      if (result_code != UPLL_RC_SUCCESS) {
        break;
      }
      DbSubOp dbop = {kOpReadSingle, kOpMatchNone, kOpInOutFlag | kOpInOutCs};
      result_code = GetChildConfigKey(vtn_ck_run, req);
      result_code = ReadConfigDB(vtn_ck_run, UPLL_DT_STATE, UNC_OP_READ,
                                 dbop, dmi, MAINTBL);
      if (result_code != UPLL_RC_SUCCESS) {
        if ((op[i] != UNC_OP_DELETE) &&
           (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE)) {
          if (cfg1_cursor)
            dmi->CloseCursor(cfg1_cursor, true);
          delete req;
          return result_code;
        }
      }
      if (op[i] == UNC_OP_CREATE) {
        /* set consolidated config status to UNKNOWN to init vtn cs_status
       * to the cs_status of first controller
       */
        uint32_t cur_instance_count;
        result_code = GetInstanceCount(vtn_ck_run, NULL,
                                 UPLL_DT_CANDIDATE, &cur_instance_count,
                                 dmi, CTRLRTBL);
        if ((result_code == UPLL_RC_SUCCESS) && (cur_instance_count == 1))
          reinterpret_cast<val_vtn *>(GetVal(vtn_ck_run))->cs_row_status =
                                      UNC_CS_UNKNOWN;
      }
      if ((op[i] == UNC_OP_CREATE) || (op[i] == UNC_OP_UPDATE)) {
        result_code = DupConfigKeyVal(vtn_ctrlr_key, req, CTRLRTBL);
        if (result_code != UPLL_RC_SUCCESS || vtn_ctrlr_key == NULL) {
          if (cfg1_cursor)
            dmi->CloseCursor(cfg1_cursor, true);
          delete req;
          return result_code;
        }
        GET_USER_DATA_CTRLR(vtn_ctrlr_key, ctrlr_id);
        string controller(reinterpret_cast<char *>(ctrlr_id));
        // Passing vtn_ck_run instead of nreq
        if (ctrlr_result.empty()) {
          UPLL_LOG_TRACE("ctrlr_commit_status is NULL.");
          result_code = UpdateConfigStatus(vtn_ck_run, op[i],
                       UPLL_RC_ERR_CTR_DISCONNECTED, vtn_ctrlr_key, dmi, nreq);
        } else {
          result_code = UpdateConfigStatus(vtn_ck_run, op[i],
                       ctrlr_result[controller], vtn_ctrlr_key, dmi, nreq);
        }
      } else if (op[i] == UNC_OP_DELETE) {
        if (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
          GET_USER_DATA_CTRLR(req, ctrlr_id);
          result_code = SetVtnConsolidatedStatus(vtn_ck_run, ctrlr_id, dmi);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("Could not set consolidated status %d", result_code);
            return result_code;
          }
        }
        result_code = GetChildConfigKey(vtn_ctrlr_key, req);
      }
      if (result_code != UPLL_RC_SUCCESS) {
        if (cfg1_cursor)
          dmi->CloseCursor(cfg1_cursor, true);
        delete req;
        return result_code;
      }
      result_code = UpdateConfigDB(vtn_ctrlr_key, UPLL_DT_STATE, op[i],
                                   dmi, config_mode, vtn_name, CTRLRTBL);
      if (result_code != UPLL_RC_SUCCESS) {
        if (cfg1_cursor)
          dmi->CloseCursor(cfg1_cursor, true);
        delete req;
        return result_code;
      }
      if (op[i] != UNC_OP_DELETE) {
        DbSubOp dbop_update = {kOpNotRead, kOpMatchNone, kOpInOutCs};
        result_code = UpdateConfigDB(vtn_ck_run, UPLL_DT_STATE,
            UNC_OP_UPDATE, dmi, &dbop_update, config_mode, vtn_name, MAINTBL);
        if (result_code != UPLL_RC_SUCCESS) {
          if (cfg1_cursor)
            dmi->CloseCursor(cfg1_cursor, true);
          delete req;
          return result_code;
        }
      }
      EnqueCfgNotification(op[i], UPLL_DT_RUNNING, vtn_ck_run);
#if 1
      if (vtn_ctrlr_key)
        delete vtn_ctrlr_key;
#endif
      if (vtn_ck_run)
        delete vtn_ck_run;
      vtn_ck_run = vtn_ctrlr_key = NULL;
      result_code = DalToUpllResCode(db_result);
    }
    if (cfg1_cursor)
      dmi->CloseCursor(cfg1_cursor, true);
    if (req)
      delete req;
    if (nreq) delete nreq;
    req = nreq = NULL;
    // Copying Rename Table to Running
    UPLL_LOG_DEBUG("keytype is %d", keytype);
    result_code = TxCopyRenameTableFromCandidateToRunning(keytype,
                                                          op[i], dmi,
                                                          config_mode,
                                                          vtn_name);
    UPLL_LOG_DEBUG("TxCopyRenameTableFromCandidateToRunning returned %d",
                                                          result_code);
  }
  result_code = (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE)?
                                UPLL_RC_SUCCESS:result_code;
  return result_code;
}


upll_rc_t VtnMoMgr::ReadSingleCtlrlVtnMapping(IpcReqRespHeader *header,
                                              ConfigKeyVal *ikey,
                                              DalDmlIntf *dmi,
                                              uint32_t *ckv_count,
                                              bool is_vnode_filter_valid) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  if (*ckv_count >= header->rep_count) {
    return result_code;
  }
  controller_domain ctrlr_dom_in;
  ctrlr_dom_in.ctrlr = NULL;
  ctrlr_dom_in.domain = NULL;

  ConfigKeyVal *ckv_domain = NULL, *ckv_all_domain = NULL;
  ConfigKeyVal *ckv_drv = NULL;
  DbSubOp op = {kOpReadMultiple, kOpMatchCtrlr, kOpInOutCtrlr | kOpInOutDomain};

  GET_USER_DATA_CTRLR_DOMAIN(ikey, ctrlr_dom_in);
  key_vtn_t *vtnkey = reinterpret_cast<key_vtn_t *>
      (ConfigKeyVal::Malloc(sizeof(key_vtn_t)));
  uuu::upll_strncpy(vtnkey->vtn_name, reinterpret_cast<key_vtn_controller *>(
          ikey->get_key())->vtn_key.vtn_name, (kMaxLenVtnName + 1));
  ckv_all_domain = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn,
                                    vtnkey, NULL);
  SET_USER_DATA_CTRLR(ckv_all_domain, ctrlr_dom_in.ctrlr);
  result_code = ReadConfigDB(ckv_all_domain, UPLL_DT_RUNNING,
                             UNC_OP_READ, op, dmi, CTRLRTBL);
  if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    DELETE_IF_NOT_NULL(ckv_all_domain);
    return result_code;
  }
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_INFO("ReadConfigDB from rename tbl failed. Error code : %d",
                   result_code);
    DELETE_IF_NOT_NULL(ckv_all_domain);
    return result_code;
  }

  ConfigKeyVal *result_ckv  = NULL;
  for (ckv_domain = ckv_all_domain; ckv_domain;
       ckv_domain = ckv_domain->get_next_cfg_key_val()) {
    controller_domain ctrlr_dom_db;
    ctrlr_dom_db.ctrlr = NULL;
    ctrlr_dom_db.domain = NULL;
    GET_USER_DATA_CTRLR_DOMAIN(ckv_domain, ctrlr_dom_db);
    if (strncmp(reinterpret_cast<const char *>(ctrlr_dom_in.domain),
                reinterpret_cast<const char *>(ctrlr_dom_db.domain),
                kMaxLenDomainId +1) >= 0) {
      UPLL_LOG_TRACE("ctrlr_dom_in.domain %s > ctrlr_dom_db.domain %s",
                     ctrlr_dom_in.domain, ctrlr_dom_db.domain);
      continue;
    }
    UPLL_LOG_TRACE("ckv_domain in loop is \n %s",
                    ckv_domain->ToStrAll().c_str());
    result_code = DupConfigKeyValVtnMapping(ckv_drv, ikey);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("DupConfigKeyValVtnStation failed err code %d",
                      result_code);
      delete ckv_all_domain;
      return result_code;
    }
    ckv_domain->SetCfgVal(NULL);
    key_vtn_t *vtn_key = &(reinterpret_cast<key_vtn_controller *>
                          (ckv_drv->get_key())->vtn_key);
    result_code = GetRenamedControllerKey(ckv_domain, UPLL_DT_RUNNING,
                                          dmi, &ctrlr_dom_db);
    if (result_code != UPLL_RC_SUCCESS &&
        result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      UPLL_LOG_INFO("GetRenamedControllerKey failed err code %d", result_code);
      delete ckv_all_domain;
      delete ckv_drv;
      return result_code;
    }
    uuu::upll_strncpy(vtn_key->vtn_name, reinterpret_cast<key_vtn_t *>
                      (ckv_domain->get_key())->vtn_name,
                        (kMaxLenVtnName + 1));
    UPLL_LOG_TRACE("Controller id and domain id are %s %s", ctrlr_dom_db.ctrlr,
                   ctrlr_dom_db.domain);
    uuu::upll_strncpy(reinterpret_cast<key_vtn_controller *>
                      (ckv_drv->get_key())->domain_id,
                      ctrlr_dom_db.domain, kMaxLenDomainId+1);
    result_code = SendReadReqToDriver(header, ckv_drv, &ctrlr_dom_db);
    if (result_code != UPLL_RC_SUCCESS &&
        result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      DELETE_IF_NOT_NULL(result_ckv);
      DELETE_IF_NOT_NULL(ckv_all_domain);
      DELETE_IF_NOT_NULL(ckv_drv);
      if (result_code == UPLL_RC_ERR_CTR_DISCONNECTED) {
        result_code = UPLL_RC_ERR_NO_SUCH_INSTANCE;
      }
      return result_code;
    } else if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      UPLL_LOG_DEBUG("Record Not found for domain %s", ctrlr_dom_db.domain);
      delete ckv_drv;
      ckv_drv = NULL;
      continue;
    } else {
      uint32_t rec_count_dummy = 0;
      uint32_t *rcd = &rec_count_dummy;
      result_code = MappingvExtTovBr(ckv_drv, header, dmi, rcd);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("MappingvExtTovBr failed result_code - %d", result_code);
        DELETE_IF_NOT_NULL(ckv_drv);
        return result_code;
      }
      if (result_ckv == NULL) {
        result_ckv = ckv_drv;
      } else {
        result_ckv->AppendCfgKeyVal(ckv_drv);
      }
      (*ckv_count)++;
      ckv_drv = NULL;
    }
    if (*ckv_count >= header->rep_count) {
      break;
    }
  }
  delete ckv_all_domain;
  if (result_ckv) {
    ikey->ResetWith(result_ckv);
    DELETE_IF_NOT_NULL(result_ckv);
    result_code = UPLL_RC_SUCCESS;
  } else {
    result_code = UPLL_RC_ERR_NO_SUCH_INSTANCE;
  }
  return result_code;
}

upll_rc_t VtnMoMgr::ValidateVnodeFilter(
    ConfigKeyVal *ikey, uint8_t ctrlr_domain_valid,
    bool &is_vnode_filter_valid,
    controller_domain **ctrlr_dom, DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  val_vtnstation_controller_st *val_vtnst = NULL;
  val_vtn_mapping_controller_st *val_vtnmap = NULL;
  uint8_t *vnode_type_valid;
  uint8_t vtn_valid = UNC_VF_VALID;
  uint8_t vnode_valid;
  uint8_t *vnode_if_valid;
  uint8_t *vtn_name;
  uint8_t *vnode_name;
  uint8_t *vnode_if_name;
  uint8_t *vnode_type;
  if (ikey->get_key_type() == UNC_KT_VTNSTATION_CONTROLLER) {
    val_vtnst =
        reinterpret_cast<val_vtnstation_controller_st_t *>(GetVal(ikey));
    if (!val_vtnst) {
      UPLL_LOG_ERROR("val_vtnstation_controller_st NULL");
      return UPLL_RC_ERR_GENERIC;
    }
    vtn_valid = val_vtnst->valid[UPLL_IDX_VTN_NAME_VSCS];
    vtn_name = val_vtnst->vtn_name;
    vnode_name = val_vtnst->vnode_name;
    vnode_if_name = val_vtnst->vnode_if_name;
    vnode_type = &(val_vtnst->vnode_type);
    vnode_type_valid = &(val_vtnst->valid[UPLL_IDX_VNODE_TYPE_VSCS]);
    vnode_valid = val_vtnst->valid[UPLL_IDX_VNODE_NAME_VSCS];
    vnode_if_valid = &(val_vtnst->valid[UPLL_IDX_VNODE_IF_NAME_VSCS]);
    if (ctrlr_domain_valid & CTRLR_VALID) {
      (*ctrlr_dom)->ctrlr = reinterpret_cast<key_vtnstation_controller *>
               (ikey->get_key())->controller_name;
    }
  } else { /* Only UNC_KT_VTN_MAPPING_CONTROLLER is possible */
    val_vtnmap =
        reinterpret_cast<val_vtn_mapping_controller_st *>(GetVal(ikey));
    if (!val_vtnmap) {
     /* For UNC_KT_VTN_MAPPING_CONTROLLER value structure is optional */
     UPLL_LOG_DEBUG("Val structure is NULL");
     return result_code;
    }
    vtn_name = reinterpret_cast<key_vtn_controller *>
               (ikey->get_key())->vtn_key.vtn_name;
    vnode_name = val_vtnmap->vnode_name;
    vnode_if_name = val_vtnmap->vnode_if_name;
    vnode_type = &(val_vtnmap->vnode_type);
    vnode_type_valid = &(val_vtnmap->valid[UPLL_IDX_VNODE_TYPE_VMCS]);
    vnode_valid = val_vtnmap->valid[UPLL_IDX_VNODE_NAME_VMCS];
    vnode_if_valid = &(val_vtnmap->valid[UPLL_IDX_VNODE_IF_NAME_VMCS]);
    if (ctrlr_domain_valid & CTRLR_DOM_VALID) {
      (*ctrlr_dom)->ctrlr = reinterpret_cast<key_vtn_controller *>
               (ikey->get_key())->controller_name;
      (*ctrlr_dom)->domain = reinterpret_cast<key_vtn_controller *>
               (ikey->get_key())->domain_id;
    }
  }
  if (UNC_VF_VALID != vtn_valid &&
      (UNC_VF_VALID == vnode_valid || UNC_VF_VALID == *vnode_if_valid ||
       UNC_VF_VALID == *vnode_type_valid)) {
    /* if vtn is invalid and vnode filter attributes(vnode_type, vnode_name,
       vnode_if_name) is/are valid, returns UPLL_RC_ERR_CFG_SYNTAX error*/
    UPLL_LOG_DEBUG("vtn field is invalid in the request");
    return UPLL_RC_ERR_CFG_SYNTAX;
  }
  if (UNC_VF_VALID == *vnode_type_valid &&
     (*vnode_type != UPLL_VNODE_VBRIDGE &&
      *vnode_type != UPLL_VNODE_VTERMINAL)) {
    UPLL_LOG_INFO("Invalid vNode type in the request");
    return UPLL_RC_ERR_CFG_SYNTAX;
  }
  if (*vnode_type == UPLL_VNODE_VTERMINAL) {
    *vnode_if_valid = UNC_VF_INVALID;
  }
  if (UNC_VF_VALID == vnode_valid && UNC_VF_VALID == *vnode_if_valid) {
    if (UNC_VF_VALID == *vnode_type_valid) {
    /*  vNode_name    vNode_if_name   vnode_type
    1. UNC_VF_VALID   UNC_VF_VALID    UNC_VF_VALID */
      result_code = ProcessVexternalFilter(vtn_name, vnode_name,
                        vnode_if_name, vnode_type, ctrlr_dom, dmi);
      if (result_code != UPLL_RC_SUCCESS) {
        /* Possible error codes are UPLL_RC_ERR_NO_SUCH_INSTANCE and
           database error */
        UPLL_LOG_INFO("ProcessVexternalFilter failed err_code - %d\n",
                             result_code);
        return result_code;
      }
    } else {
      /* vnode_type can be UNC_VF_INVALID, UNC_VF_VALID_NO_VALUE,
       * UNC_VF_NOT_SUPPORTED, UNC_VF_VALUE_NOT_MODIFIED */

      /*  vNode_name     vNode_if_name   vnode_type
      2. UNC_VF_VALID   UNC_VF_VALID    UNC_VF_INVALID */
      *vnode_type = UPLL_VNODE_VBRIDGE;
      result_code = ProcessVexternalFilter(vtn_name, vnode_name,
                        vnode_if_name, vnode_type, ctrlr_dom, dmi);
      if (result_code != UPLL_RC_SUCCESS &&
          result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      /* Possible error codes are UPLL_RC_ERR_NO_SUCH_INSTANCE and
         database error */
        UPLL_LOG_INFO("ProcessVexternalFilter failed err_code - %d\n",
                             result_code);
        return result_code;
      } else if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
        *vnode_type = UPLL_VNODE_VTERMINAL;
        result_code = ProcessVexternalFilter(vtn_name, vnode_name,
                        vnode_if_name, vnode_type, ctrlr_dom, dmi);
        if (result_code != UPLL_RC_SUCCESS) {
        /* Possible error codes are UPLL_RC_ERR_NO_SUCH_INSTANCE and
               database error */
          UPLL_LOG_INFO("ProcessVexternalFilter failed err_code - %d\n",
                             result_code);
          return result_code;
        }
      }
      /* vnode_type is always valid while sending request to driver,
       * whenever vnode filter specified */
      *vnode_type_valid = UNC_VF_VALID;
    }
    // vnode_if_valid = UNC_VF_INVALID;
    is_vnode_filter_valid = true;
  } else if (UNC_VF_VALID == vnode_valid && UNC_VF_INVALID == *vnode_if_valid) {
    if (UNC_VF_VALID == *vnode_type_valid) {
    /*  vNode_name     vNode_if_name     vnode_type
    3. UNC_VF_VALID   UNC_VF_INVALID    UNC_VF_VALID */
      result_code = ProcessVnodeFilter(vtn_name, vnode_name,
                                           vnode_type, ctrlr_dom, dmi);
      if (result_code != UPLL_RC_SUCCESS) {
        /* Possible error codes are UPLL_RC_ERR_NO_SUCH_INSTANCE and
               database error */
        UPLL_LOG_INFO("ProcessVnodeFilter failed err_code - %d\n",
                             result_code);
        return result_code;
      }
    } else {
      /* vnode_type can be UNC_VF_INVALID, UNC_VF_VALID_NO_VALUE,
       * UNC_VF_NOT_SUPPORTED, UNC_VF_VALUE_NOT_MODIFIED */

      /*    vNode_name      vNode_if_name       vnode_type
       * 4. UNC_VF_VALID   UNC_VF_INVALID      UNC_VF_INVALID */
      *vnode_type = UPLL_VNODE_VBRIDGE;
      result_code = ProcessVnodeFilter(vtn_name, vnode_name,
                                       vnode_type,  ctrlr_dom, dmi);
      if (result_code != UPLL_RC_SUCCESS &&
          result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
        /* Possible error codes are UPLL_RC_ERR_NO_SUCH_INSTANCE and
         * database error */
        UPLL_LOG_INFO("ProcessVbridgeFilter failed err_code - %d\n",
                             result_code);
        return result_code;
      } else if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
        *vnode_type = UPLL_VNODE_VTERMINAL;
        result_code = ProcessVnodeFilter(vtn_name, vnode_name,
                                               vnode_type, ctrlr_dom, dmi);
        if (result_code != UPLL_RC_SUCCESS) {
        /* Possible error codes are UPLL_RC_ERR_NO_SUCH_INSTANCE and
         * database error */
          UPLL_LOG_INFO("ProcessVterminalFilter failed err_code - %d\n",
                             result_code);
          return result_code;
        }
      }
      /* vnode_type is always valid while sending request to driver,
       * whenever vnode filter specified */
      *vnode_type_valid = UNC_VF_VALID;
    }
    is_vnode_filter_valid = true;
  } else if (UNC_VF_INVALID == vnode_valid &&
             (UNC_VF_VALID == *vnode_if_valid || UNC_VF_VALID == *vnode_type)) {
      /*     vNode_name        vNode_if_name   vnode_type
       * 5.  UNC_VF_INVALID    UNC_VF_VALID    UNC_VF_VALID
       * 6.  UNC_VF_INVALID    UNC_VF_VALID    UNC_VF_INVALID
       * 7.  UNC_VF_INVALID    UNC_VF_INVALID  UNC_VF_VALID */
      UPLL_LOG_INFO("Invalid vnode filter combination");
      return UPLL_RC_ERR_CFG_SYNTAX;
  } else {
      /*    vNode_name         vNode_if_name   vnode_type
         8. UNC_VF_INVALID    UNC_VF_INVALID  UNC_VF_INVALID */
      UPLL_LOG_DEBUG("vnode filter attributes are Invalid");
  }
  return result_code;
}

upll_rc_t VtnMoMgr::ProcessVnodeFilter(
    uint8_t *vtn_name, uint8_t *vnode_name,
    uint8_t *vnode_type, controller_domain **in_ctrlr_dom,
    DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  unc_key_type_t key_type = UNC_KT_ROOT;
  IpctSt::IpcStructNum key_stnum = IpctSt::kIpcInvalidStNum;
  if (*vnode_type == UPLL_VNODE_VBRIDGE) {
    /* if vnode_type is UPLL_VNODE_VBRIDGE, search in vbr table.
     * else, search it in vterm table */
    key_type =  UNC_KT_VBRIDGE;
    key_stnum = IpctSt::kIpcStKeyVbr;
  } else {
    key_type =  UNC_KT_VTERMINAL;
    key_stnum = IpctSt::kIpcStKeyVterminal;
  }
  key_vnode_t *vnode_key= reinterpret_cast<key_vnode_t *>
                (ConfigKeyVal::Malloc(sizeof(key_vnode_t)));
  uuu::upll_strncpy(vnode_key->vtn_key.vtn_name,
                         vtn_name, (kMaxLenVtnName + 1));
  uuu::upll_strncpy(vnode_key->vnode_name, vnode_name,
                         (kMaxLenVnodeName + 1));
  ConfigKeyVal *ckv_vnode = new ConfigKeyVal(key_type,
      key_stnum,
      vnode_key, NULL);
  DbSubOp dbop = {kOpReadSingle, kOpMatchNone,
             kOpInOutCtrlr | kOpInOutDomain};
  if ((*in_ctrlr_dom)->ctrlr) {
    /* For UNC_OP_READ operation only, use the in_ctrlr_dom -> controller
     * for search in database.
     * FOR UNC_OP_READ_SIBLING/UNC_OP_READ_SIBLING_BEGIN in_ctrlr_dom is NULL.*/
    SET_USER_DATA_CTRLR(ckv_vnode, (*in_ctrlr_dom)->ctrlr);
    dbop.matchop = kOpMatchCtrlr;
  }
  MoMgrImpl *vnode_mgr = reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>
      (GetMoManager(key_type)));

  if ((*in_ctrlr_dom)->domain) {
    /* For Vtn station, If domain is specified in the request, it is ignored
     * For Vtn mapping, If the domain specified in the request mismatch
     * with vnode configured domain
     * return UPLL_RC_ERR_NO_SUCH_INSTANCE */
    SET_USER_DATA_DOMAIN(ckv_vnode, (*in_ctrlr_dom)->domain);
    dbop.matchop = kOpMatchCtrlr | kOpInOutDomain;
  }
  result_code = vnode_mgr->ReadConfigDB(ckv_vnode, UPLL_DT_RUNNING,
      UNC_OP_READ, dbop, dmi, MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    /* Either database or NO_SUCH_INSTANCE error */
    UPLL_LOG_INFO("ReadConfigDB from VbrIf tbl failed errcode %d",
        result_code);
    DELETE_IF_NOT_NULL(ckv_vnode);
    return result_code;
  }
  /* Vtn, vNode combination present in vBridgeIf table */
  controller_domain db_ctrlr_dom;
  GET_USER_DATA_CTRLR_DOMAIN(ckv_vnode, db_ctrlr_dom);
  if (!(*in_ctrlr_dom)->ctrlr) {
    /* Get a copy of vnode configured controller-domain */
    (*in_ctrlr_dom)->ctrlr = reinterpret_cast<uint8_t *>
                     (ConfigKeyVal::Malloc(kMaxLenCtrlrId + 1));
    uuu::upll_strncpy((*in_ctrlr_dom)->ctrlr, db_ctrlr_dom.ctrlr,
                      (kMaxLenCtrlrId + 1));
  }
  if (!(*in_ctrlr_dom)->domain) {
    (*in_ctrlr_dom)->domain = reinterpret_cast<uint8_t *>
                      (ConfigKeyVal::Malloc(kMaxLenDomainId + 1));
    uuu::upll_strncpy((*in_ctrlr_dom)->domain, db_ctrlr_dom.domain,
                      (kMaxLenDomainId + 1));
  }
  ckv_vnode->SetCfgVal(NULL);
  result_code = vnode_mgr->GetRenamedControllerKey(ckv_vnode, UPLL_DT_RUNNING,
      dmi, &db_ctrlr_dom);
  if (result_code != UPLL_RC_SUCCESS &&
        result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    /* Database Error only */
    UPLL_LOG_DEBUG("GetRenamedControllerKey failed. Result : %d",
        result_code);
    DELETE_IF_NOT_NULL(ckv_vnode);
    return result_code;
  }
  result_code =
      (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) ? UPLL_RC_SUCCESS :
      result_code;
  /* copy the renamed vtn and vnode name in driver request */
  uuu::upll_strncpy(vtn_name,
                      vnode_key->vtn_key.vtn_name,
                       (kMaxLenVtnName + 1));
  uuu::upll_strncpy(vnode_name,
                      vnode_key->vnode_name,
                       (kMaxLenVnodeName + 1));
  DELETE_IF_NOT_NULL(ckv_vnode);
  return result_code;
}

upll_rc_t VtnMoMgr::ProcessVexternalFilter(
    uint8_t *vtn_name, uint8_t *vnode_name,
    uint8_t *vnode_if_name, uint8_t *vnode_type,
    controller_domain **in_ctrlr_dom, DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  unc_key_type_t key_type = UNC_KT_ROOT;
  IpctSt::IpcStructNum key_stnum = IpctSt::kIpcInvalidStNum;
  if (*vnode_type == UPLL_VNODE_VBRIDGE) {
    /* if vnode_type is UPLL_VNODE_VBRIDGE, search in vbrif table.
     * else, search it in vtermif table */
    key_type =  UNC_KT_VBR_IF;
    key_stnum = IpctSt::kIpcStKeyVbrIf;
  } else {
    key_type =  UNC_KT_VTERM_IF;
    key_stnum = IpctSt::kIpcStKeyVtermIf;
  }
  key_vnode_if_t *vnodeif_key = reinterpret_cast<key_vnode_if_t *>
                                 (ConfigKeyVal::Malloc(sizeof(key_vnode_if_t)));
  uuu::upll_strncpy(vnodeif_key->vnode_key.vtn_key.vtn_name,
                         vtn_name, (kMaxLenVtnName + 1));
  uuu::upll_strncpy(vnodeif_key->vnode_key.vnode_name, vnode_name,
                         (kMaxLenVnodeName + 1));
  uuu::upll_strncpy(vnodeif_key->vnode_if_name, vnode_if_name,
      (kMaxLenInterfaceName + 1));
  ConfigKeyVal *ckv_vnodeif = new ConfigKeyVal(key_type,
      key_stnum,
      vnodeif_key, NULL);
  DbSubOp dbop = {kOpReadSingle, kOpMatchNone,
    kOpInOutCtrlr | kOpInOutDomain};
  if ((*in_ctrlr_dom)->ctrlr) {
    if (IsUnifiedVbr((*in_ctrlr_dom)->ctrlr)) {
      UPLL_LOG_DEBUG("Returning no such instance for unified vbr interface");
      DELETE_IF_NOT_NULL(ckv_vnodeif);
      return UPLL_RC_ERR_NO_SUCH_INSTANCE;
    }
    /* For UNC_OP_READ operation only, use the in_ctrlr_dom -> controller
     * for search in database.
     * FOR UNC_OP_READ_SIBLING/UNC_OP_READ_SIBLING_BEGIN in_ctrlr_dom is NULL.*/
    SET_USER_DATA_CTRLR(ckv_vnodeif, (*in_ctrlr_dom)->ctrlr);
    dbop.matchop = kOpMatchCtrlr;
  }
  MoMgrImpl *vnodeif_mgr = reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>
      (GetMoManager(key_type)));
  if ((*in_ctrlr_dom)->domain) {
    /* For Vtn station, If domain is specified in the request, it is ignored
     * For Vtn mapping, If the domain specified in the request mismatch with
     * vnode configured domain return UPLL_RC_ERR_NO_SUCH_INSTANCE */
    SET_USER_DATA_DOMAIN(ckv_vnodeif, (*in_ctrlr_dom)->domain);
    dbop.matchop = kOpMatchCtrlr | kOpMatchDomain;
  }
  result_code = vnodeif_mgr->ReadConfigDB(ckv_vnodeif, UPLL_DT_RUNNING,
      UNC_OP_READ, dbop, dmi, MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    /* Either database or NO_SUCH_INSTANCE error */
    UPLL_LOG_INFO("ReadConfigDB from VbrIf tbl failed errcode %d",
        result_code);
    DELETE_IF_NOT_NULL(ckv_vnodeif);
    return result_code;
  }
  if (key_type == UNC_KT_VBR_IF) {
    /* if key_type is UNC_KT_VBR_IF,
     * fill the vexternal name in vnode_name field */
    val_drv_vbr_if_t *val = reinterpret_cast<val_drv_vbr_if_t *>
                                               (GetVal(ckv_vnodeif));
    if (val && (val->valid[PFCDRV_IDX_VEXT_NAME_VBRIF] == UNC_VF_VALID)) {
      uuu::upll_strncpy(vnode_if_name, val->vex_name,
          (kMaxLenInterfaceName + 1));
    } else {
      /* if port_map is not configured for the vbridge interface,
       * return UPLL_RC_ERR_NO_SUCH_INSTANCE */
      UPLL_LOG_INFO("Port Map not configured");
      DELETE_IF_NOT_NULL(ckv_vnodeif);
      return UPLL_RC_ERR_NO_SUCH_INSTANCE;
    }
  }
  controller_domain db_ctrlr_dom;
  GET_USER_DATA_CTRLR_DOMAIN(ckv_vnodeif, db_ctrlr_dom);
  if (!(*in_ctrlr_dom)->ctrlr) {
    /* Get a copy of vnode configured controller-domain */
    (*in_ctrlr_dom)->ctrlr = reinterpret_cast<uint8_t *>
                      (ConfigKeyVal::Malloc(kMaxLenCtrlrId + 1));
    uuu::upll_strncpy((*in_ctrlr_dom)->ctrlr, db_ctrlr_dom.ctrlr,
                      (kMaxLenCtrlrId + 1));
  }
  if (!(*in_ctrlr_dom)->domain) {
    (*in_ctrlr_dom)->domain = reinterpret_cast<uint8_t *>
                      (ConfigKeyVal::Malloc(kMaxLenDomainId + 1));
    uuu::upll_strncpy((*in_ctrlr_dom)->domain, db_ctrlr_dom.domain,
                      (kMaxLenDomainId + 1));
  }
  ckv_vnodeif->SetCfgVal(NULL);
  result_code = vnodeif_mgr->GetRenamedControllerKey(ckv_vnodeif,
                                                     UPLL_DT_RUNNING,
                                                     dmi, &db_ctrlr_dom);
  if (result_code != UPLL_RC_SUCCESS &&
        result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    /* Database Error only */
    UPLL_LOG_DEBUG("GetRenamedControllerKey failed. Result : %d",
                          result_code);
    DELETE_IF_NOT_NULL(ckv_vnodeif);
    return result_code;
  }
  result_code =
      (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) ? UPLL_RC_SUCCESS :
      result_code;
  uuu::upll_strncpy(vtn_name,
                      vnodeif_key->vnode_key.vtn_key.vtn_name,
                       (kMaxLenVtnName + 1));
  if (*vnode_type == UPLL_VNODE_VTERMINAL) {
    /* if key_type is UNC_KT_VTERM_IF, copy the renamed vTerminal
     * in vnode_name */
    uuu::upll_strncpy(vnode_name, vnodeif_key->vnode_key.vnode_name,
      (kMaxLenInterfaceName + 1));
  }
  DELETE_IF_NOT_NULL(ckv_vnodeif);
  return result_code;
}

upll_rc_t VtnMoMgr::ReadSingleCtlrlStation(IpcReqRespHeader *header,
                                           ConfigKeyVal *ikey,
                                           bool &is_vnode_filter_valid,
                                           controller_domain *vnode_ctrlr_dom,
                                           uint32_t *rec_count,
                                           DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  val_vtnstation_controller_st_t *in_valst = NULL;
  val_vtnstation_controller_st_t *valst = NULL;
  val_rename_vtn *rename_valst = NULL;
  ConfigKeyVal *ckv_drv = NULL;
  ConfigKeyVal *ckv_rename = NULL;
  ConfigKeyVal *okey = NULL;
  bool renamed = false;
  UPLL_LOG_DEBUG("Input ikey is %s", ikey->ToStrAll().c_str());
  result_code = DupConfigKeyValVtnStation(okey, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("DupConfigKeyValVtnStation failed err code %d", result_code);
    return result_code;
  }
  controller_domain ctrlr_dom;
  ctrlr_dom.ctrlr = NULL;
  ctrlr_dom.domain = NULL;
  ctrlr_dom.ctrlr =  reinterpret_cast<key_vtnstation_controller *>
                     (okey->get_key())->controller_name;
  ckv_rename = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, NULL, NULL);
  in_valst = reinterpret_cast<val_vtnstation_controller_st_t *>(GetVal(okey));
  uint8_t ctrlr_domain_valid = CTRLR_DOM_INVALID;
  if (header->operation == UNC_OP_READ) {
    /* For READ operation use the controller name in the key to
     * search in the database, when vnode filter is specified.
     * For READ_SIBLING/READ_SIBLING_BEGIN operation, use the vNode
     * configured controller name
     * for search in the  database, when vNode filter is specified. */
    ctrlr_domain_valid = CTRLR_VALID;
  }
  /* This function validates whether the vNode filter is specified.
   * if it is specified, * checks whether it is valid vNode filter combination.
   * if it is valid, frame the driver request * which includes vexternal
   * conversion if vnode interface specfied, filling vNode type
   * if it is not specified, renaming vtn and vNode */
  result_code = ValidateVnodeFilter(okey, ctrlr_domain_valid,
                                    is_vnode_filter_valid,
                                    &vnode_ctrlr_dom, dmi);
  if (result_code != UPLL_RC_SUCCESS) {
    /* Possible error codes are UPLL_RC_ERR_NO_SUCH_INSTANCE,
     * UPLL_RC_ERR_CFG_SYNTAX and database error */
    UPLL_LOG_DEBUG("ValidateVnodeFilter returning error code %d",
                      result_code);
    DELETE_IF_NOT_NULL(ckv_rename);
    DELETE_IF_NOT_NULL(okey);
    return result_code;
  }
  if (in_valst->valid[UPLL_IDX_VTN_NAME_VSCS] == UNC_VF_VALID &&
      !is_vnode_filter_valid) {
    DbSubOp op = {kOpReadMultiple, kOpMatchCtrlr,
      kOpInOutCtrlr | kOpInOutDomain};
    key_vtn_t *vtn_key = reinterpret_cast<key_vtn_t *>
                         (ConfigKeyVal::Malloc(sizeof(key_vtn)));
    uuu::upll_strncpy(vtn_key->vtn_name, in_valst->vtn_name,
        (kMaxLenVtnName + 1));
    ckv_rename->SetKey(IpctSt::kIpcStKeyVtn, vtn_key);
    SET_USER_DATA_CTRLR_DOMAIN(ckv_rename, ctrlr_dom);
    // Verifies whether the requested VTN is renamed or not
    result_code = ReadConfigDB(ckv_rename, UPLL_DT_RUNNING,
        UNC_OP_READ, op, dmi, RENAMETBL);
    if (result_code != UPLL_RC_SUCCESS &&
        result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      UPLL_LOG_INFO("ReadConfigDB from rename tbl failed err code %d",
                       result_code);
      DELETE_IF_NOT_NULL(ckv_rename);
      DELETE_IF_NOT_NULL(okey);
      return result_code;
    } else if (result_code == UPLL_RC_SUCCESS) {
        renamed = true;
    }
    key_vtn_t* vtnkey =  reinterpret_cast<key_vtn_t *>
      (ConfigKeyVal::Malloc(sizeof(key_vtn)));
    uuu::upll_strncpy(vtnkey->vtn_name, in_valst->vtn_name,
        (kMaxLenVtnName + 1));
    /* Checks whether non renamed VTN entry exist in Controller table */
    ConfigKeyVal *vtn_ckv = new ConfigKeyVal(UNC_KT_VTN,
        IpctSt::kIpcStKeyVtn, vtnkey, NULL);
    SET_USER_DATA_CTRLR(vtn_ckv, ctrlr_dom.ctrlr);
    SET_USER_DATA_FLAGS(vtn_ckv, NO_RENAME);
    DbSubOp vtnctlr_op = {kOpReadExist, kOpMatchCtrlr | kOpMatchFlag,
      kOpInOutNone};
    result_code = UpdateConfigDB(vtn_ckv, UPLL_DT_RUNNING,
        UNC_OP_READ, dmi, &vtnctlr_op, CTRLRTBL);
    /* Returns All DB errors and if requested VTN not exist in VTN
       controller table and rename table */
    if (result_code != UPLL_RC_ERR_INSTANCE_EXISTS && !renamed) {
      UPLL_LOG_ERROR("Failed to read VTN controller table. result_code = %u",
            result_code);
      DELETE_IF_NOT_NULL(ckv_rename);
      DELETE_IF_NOT_NULL(vtn_ckv);
      DELETE_IF_NOT_NULL(okey);
      return result_code;
    } else {
      if (renamed) {
        /* Entry present in both controller table and rename table */
        if (result_code == UPLL_RC_ERR_INSTANCE_EXISTS) {
          vtn_ckv->AppendCfgKeyVal(ckv_rename);
    ckv_rename = vtn_ckv;
        } else {
          /* Entry exist in rename table */
    DELETE_IF_NOT_NULL(vtn_ckv);
        }
      } else {
        /* Entry exist only in controller table */
        DELETE_IF_NOT_NULL(ckv_rename)
        ckv_rename = vtn_ckv;
      }
    }
  } else if (is_vnode_filter_valid) {
    ctrlr_dom.ctrlr = vnode_ctrlr_dom->ctrlr;
    ctrlr_dom.domain = vnode_ctrlr_dom->domain;
  }
  ConfigKeyVal *temp = ckv_rename;
  ConfigKeyVal *result_ckv = NULL;
  // bool first_req = true;
  for (; ckv_rename; ckv_rename = ckv_rename->get_next_cfg_key_val()) {
    UPLL_LOG_TRACE("ckv_rename in loop is \n %s",
                    ckv_rename->ToStrAll().c_str());
    result_code = DupConfigKeyValVtnStation(ckv_drv, okey);
    if (result_code != UPLL_RC_SUCCESS) {
      DELETE_IF_NOT_NULL(temp);
      DELETE_IF_NOT_NULL(okey);
      UPLL_LOG_DEBUG("DupConfigKeyValVtnStation failed. err : %d", result_code);
      return result_code;
    }
    valst = reinterpret_cast<val_vtnstation_controller_st *>(GetVal(ckv_drv));
    rename_valst = reinterpret_cast<val_rename_vtn *>(GetVal(ckv_rename));
    if (renamed && (valst != NULL) && (rename_valst != NULL)) {
      uuu::upll_strncpy(valst->vtn_name, rename_valst->new_name,
                        (kMaxLenVtnName + 1));
    }
    UPLL_LOG_DEBUG("Controller id and domain id are %s %s", ctrlr_dom.ctrlr,
                    ctrlr_dom.domain);
    result_code = SendReadReqToDriver(header, ckv_drv, &ctrlr_dom);
    if (result_code != UPLL_RC_SUCCESS  &&
        result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      DELETE_IF_NOT_NULL(ckv_drv);
      DELETE_IF_NOT_NULL(okey);
      DELETE_IF_NOT_NULL(temp);
      return result_code;
    } else if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      UPLL_LOG_DEBUG("Record Not found for domain %s", ctrlr_dom.domain);
      DELETE_IF_NOT_NULL(ckv_drv);
      continue;
    } else {
      result_code = MappingvExtTovBr(ckv_drv, header, dmi, rec_count);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("MappingvExtTovBr failed result_code - %d", result_code);
        DELETE_IF_NOT_NULL(ckv_drv);
        DELETE_IF_NOT_NULL(okey);
        DELETE_IF_NOT_NULL(temp);
        return result_code;
      }
      UPLL_LOG_TRACE("record count is %d", *rec_count);
      #if 0
      if (first_req) {
         ikey->SetCfgVal(NULL);
         first_req = false;
      }
      #endif
      if (result_ckv && result_ckv->get_cfg_val()) {
        uint32_t *count = reinterpret_cast<uint32_t *>(GetVal(result_ckv));
        ConfigVal* cv_drv  = ckv_drv->get_cfg_val();
        uint32_t *nrec_dom = reinterpret_cast<uint32_t *>(GetVal(ckv_drv));
        if (cv_drv->get_st_num() == IpctSt::kIpcStUint32) {
          *count += *nrec_dom;
          ConfigVal *tmp = cv_drv;
          cv_drv = cv_drv->get_next_cfg_val();
          tmp->set_next_cfg_val(NULL);
          DELETE_IF_NOT_NULL(tmp);
          ckv_drv->set_cfg_val(cv_drv);
        }
      }
      if (result_ckv == NULL) {
        result_ckv = ckv_drv;
      } else {
        result_ckv->AppendCfgVal(ckv_drv->GetCfgValAndUnlink());
        DELETE_IF_NOT_NULL(ckv_drv);
      }
      ckv_drv = NULL;
    }
  }
  if (result_ckv) {
    ikey->ResetWith(result_ckv);
    DELETE_IF_NOT_NULL(result_ckv);
    result_code = UPLL_RC_SUCCESS;
  } else {
    result_code = UPLL_RC_ERR_NO_SUCH_INSTANCE;
  }
  DELETE_IF_NOT_NULL(okey);
  DELETE_IF_NOT_NULL(temp);
  return result_code;
}

bool
VtnMoMgr::IsAllInvalidAttributes(const val_vtnstation_controller_st *val_stn) {
  for (uint16_t iter = 0;
       iter < sizeof(val_stn->valid)/sizeof(val_stn->valid[0]);
       iter++) {
    if (val_stn->valid[iter] != UNC_VF_INVALID) {
      return false;
    }
  }
  return true;
}

upll_rc_t VtnMoMgr::ReadSingleCtlrlDomVtnMapping(IpcReqRespHeader *header,
                                                 ConfigKeyVal *ikey,
                                                 bool is_vnode_filter_valid,
                                                 DalDmlIntf  *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  controller_domain ctrlr_dom;
  ctrlr_dom.ctrlr = NULL;
  ctrlr_dom.domain = NULL;
  ConfigKeyVal *ckv_rename = NULL;
  result_code = GetChildConfigKey(ckv_rename, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_INFO("GetChildConfigKey failed err code %d", result_code);
    return result_code;
  }
  GET_USER_DATA_CTRLR_DOMAIN(ckv_rename, ctrlr_dom);
  if (ctrlr_dom.ctrlr && IsUnifiedVbr(ctrlr_dom.ctrlr)) {
    UPLL_LOG_DEBUG("Returning no such instance for unified vbridge");
    DELETE_IF_NOT_NULL(ckv_rename);
    return UPLL_RC_ERR_NO_SUCH_INSTANCE;
  }
  if (!is_vnode_filter_valid) {
    result_code = GetRenamedControllerKey(ckv_rename, UPLL_DT_RUNNING, dmi,
                                          &ctrlr_dom);
     if (result_code != UPLL_RC_SUCCESS &&
         result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
       UPLL_LOG_DEBUG("GetRenamedControllerKey Failed result_code - %d",
                       result_code);
       DELETE_IF_NOT_NULL(ckv_rename);
       return result_code;
    }
    uuu::upll_strncpy(reinterpret_cast<key_vtn_controller *>
                (ikey->get_key())->vtn_key.vtn_name,
                reinterpret_cast<key_vtn *>(ckv_rename->get_key())->vtn_name,
                (kMaxLenVtnName + 1));
  }
  result_code = SendReadReqToDriver(header, ikey, &ctrlr_dom);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_INFO("SendReadReqToDriver failed err_code %d ", result_code);
    DELETE_IF_NOT_NULL(ckv_rename);
    result_code = (result_code == UPLL_RC_ERR_CTR_DISCONNECTED) ?
        UPLL_RC_ERR_NO_SUCH_INSTANCE : result_code;
    return result_code;
  }
  DELETE_IF_NOT_NULL(ckv_rename);
  uint32_t *tmp_count;
  uint32_t rec_count;
  tmp_count  = &rec_count;
  result_code = MappingvExtTovBr(ikey, header, dmi, tmp_count);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("MappingvExtTovBr failed result_code - %d", result_code);
    return result_code;
  }
  return result_code;
}

upll_rc_t VtnMoMgr::SendReadReqToDriver(IpcReqRespHeader *header,
                                        ConfigKeyVal *ikey,
                                        controller_domain *ctrlr_dom) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  IpcRequest ipc_req;
  memset(&ipc_req, 0, sizeof(ipc_req));
  memcpy(&(ipc_req.header), header, sizeof(IpcReqRespHeader));
  IpcResponse ipc_resp;
  memset(&ipc_resp, 0, sizeof(IpcResponse));
  ipc_req.ckv_data = ikey;
  ipc_req.header.operation = UNC_OP_READ;
  if (!IpcUtil::SendReqToDriver((const char *)(ctrlr_dom->ctrlr),
    reinterpret_cast<char *>(ctrlr_dom->domain), PFCDRIVER_SERVICE_NAME,
          PFCDRIVER_SVID_LOGICAL, &ipc_req, true, &ipc_resp)) {
    UPLL_LOG_INFO("Request to driver for Key %d on controller %s failed",
        ikey->get_key_type(), reinterpret_cast<char *>(ctrlr_dom->ctrlr));
    return ipc_resp.header.result_code;
  }
  if (ipc_resp.header.result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_INFO("Read from driver failed err code %d",
        ipc_resp.header.result_code);
    DELETE_IF_NOT_NULL(ipc_resp.ckv_data);
    return ipc_resp.header.result_code;
  }
  if (ipc_resp.ckv_data == NULL) {
    UPLL_LOG_DEBUG("Ipc Response ckv_data is NUll %d",
        ipc_resp.header.result_code);
    return UPLL_RC_ERR_GENERIC;
  }
  ikey->ResetWith(ipc_resp.ckv_data);
  DELETE_IF_NOT_NULL(ipc_resp.ckv_data);
  return result_code;
}

upll_rc_t
VtnMoMgr::ReadMo(IpcReqRespHeader * header,
                 ConfigKeyVal * ikey,
                 DalDmlIntf * dmi)  {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  if ((ikey->get_key_type() == UNC_KT_VTN_MAPPING_CONTROLLER) ||
      (ikey->get_key_type() == UNC_KT_VTNSTATION_CONTROLLER)) {
    result_code = ValidateMessage(header, ikey);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("ValidateMessage failed result_code %d",
                  result_code);
      return result_code;
    }
  }
  controller_domain ctrlr_dom;
  ctrlr_dom.ctrlr = NULL;
  ctrlr_dom.domain = NULL;
  bool is_vnode_filter_valid = false;
  switch (ikey->get_key_type()) {
    case UNC_KT_VTN: {
      result_code = MoMgrImpl::ReadMo(header, ikey, dmi);
      return result_code;
      break;
    }
    case UNC_KT_VTNSTATION_CONTROLLER: {
      uint32_t rec_count = 0;
      if (header->option1 == UNC_OPT1_NORMAL ||
          header->option1 == UNC_OPT1_DETAIL) {
        result_code = ReadSingleCtlrlStation(header, ikey,
                                             is_vnode_filter_valid, &ctrlr_dom,
                                             &rec_count, dmi);
        if (is_vnode_filter_valid)
          FREE_IF_NOT_NULL(ctrlr_dom.domain);
        if (result_code != UPLL_RC_SUCCESS) {
          return result_code;
        }
      } else if (header->option1 == UNC_OPT1_COUNT &&
         (GetVal(ikey) != NULL) && !IsAllInvalidAttributes(
           reinterpret_cast<val_vtnstation_controller_st *>
           (const_cast<void *>(GetVal(ikey))))) {
       header->option1 = UNC_OPT1_NORMAL;
       result_code = ReadSingleCtlrlStation(header, ikey,
                                            is_vnode_filter_valid, &ctrlr_dom,
                                            &rec_count, dmi);
       result_code = (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) ?
                     UPLL_RC_SUCCESS : result_code;
       header->option1 = UNC_OPT1_COUNT;
       uint32_t *count =
          reinterpret_cast<uint32_t*>(ConfigKeyVal::Malloc(sizeof(uint32_t)));
       *count = rec_count;
       ikey->SetCfgVal(new ConfigVal(IpctSt::kIpcStUint32, count));
      } else if (header->option1 == UNC_OPT1_COUNT) {
        ctrlr_dom.ctrlr = reinterpret_cast<key_vtnstation_controller *>
    (ikey->get_key())->controller_name;
        result_code = SendReadReqToDriver(header, ikey, &ctrlr_dom);
      }
      break;
    }
    case UNC_KT_VTN_MAPPING_CONTROLLER: {
      ConfigKeyVal *ck_ctrlr = NULL;
      result_code = GetChildConfigKey(ck_ctrlr, ikey);
      if (result_code != UPLL_RC_SUCCESS) {
        DELETE_IF_NOT_NULL(ck_ctrlr);
        UPLL_LOG_INFO("GetChildConfigKey failed err code %d", result_code);
        return result_code;
      }
      DbSubOp op = {kOpReadExist, kOpMatchCtrlr | kOpMatchDomain,
                kOpInOutCtrlr | kOpInOutDomain};
      controller_domain ctrlr_dom;
      ctrlr_dom.ctrlr = reinterpret_cast<key_vtn_controller *>
                         (ikey->get_key())->controller_name;
      ctrlr_dom.domain = reinterpret_cast<key_vtn_controller *>
                         (ikey->get_key())->domain_id;
      SET_USER_DATA_CTRLR_DOMAIN(ck_ctrlr, ctrlr_dom);
      result_code = UpdateConfigDB(ck_ctrlr, UPLL_DT_RUNNING,
            UNC_OP_READ, dmi, &op, CTRLRTBL);
      if (result_code != UPLL_RC_ERR_INSTANCE_EXISTS) {
        UPLL_LOG_DEBUG("UpdateConfigDB Failed result_code - %d", result_code);
        DELETE_IF_NOT_NULL(ck_ctrlr);
        return result_code;
      }
      controller_domain *vnode_ctrlr_dom = &ctrlr_dom;
      uint8_t ctrlr_domain_valid = CTRLR_DOM_VALID;
      result_code = ValidateVnodeFilter(ikey, ctrlr_domain_valid,
                                        is_vnode_filter_valid, &vnode_ctrlr_dom,
                                        dmi);
      if (result_code != UPLL_RC_SUCCESS) {
        /* Possible error codes are UPLL_RC_ERR_NO_SUCH_INSTANCE
           and database error codes */
        UPLL_LOG_INFO("ValidateVnodeFilter returning error code %d",
                      result_code);
        DELETE_IF_NOT_NULL(ck_ctrlr);
        return result_code;
      }
      SET_USER_DATA_CTRLR_DOMAIN(ikey, *vnode_ctrlr_dom);
      result_code = ReadSingleCtlrlDomVtnMapping(header, ikey,
                                                 is_vnode_filter_valid, dmi);
      if (result_code != UPLL_RC_SUCCESS) {
        /* Possible error codes are UPLL_RC_ERR_NO_SUCH_INSTANCE
           and database error codes */
        UPLL_LOG_INFO("ReadSingleCtlrlDomVtnMapping returning error code %d",
                      result_code);
        DELETE_IF_NOT_NULL(ck_ctrlr);
        return result_code;
      }
      DELETE_IF_NOT_NULL(ck_ctrlr);
      break;
    }
    default: {
      UPLL_LOG_INFO("Invalid KeyType %d", ikey->get_key_type());
      return UPLL_RC_ERR_GENERIC;
    }
  }
  return result_code;
}

upll_rc_t VtnMoMgr::ReadSiblingCount(IpcReqRespHeader *header,
                                      ConfigKeyVal* ikey,
                                      DalDmlIntf *dmi) {
    UPLL_FUNC_TRACE;
    upll_rc_t result_code = UPLL_RC_SUCCESS;
    if ((ikey->get_key_type() == UNC_KT_VTN_MAPPING_CONTROLLER) ||
      (ikey->get_key_type() == UNC_KT_VTNSTATION_CONTROLLER)) {
      result_code = ValidateMessage(header, ikey);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("ValidateMessage failed result_code %d",
                  result_code);
        return result_code;
      }
    }
    uint32_t *count, *count1;
    ConfigKeyVal *okey = NULL, *ckv = NULL;
    TcConfigMode config_mode = TC_CONFIG_INVALID;
    std::string vtn_name;
    key_vtn_t *vtn_key = NULL;
    DbSubOp dbop = { kOpReadExist, kOpMatchNone, kOpInOutNone };

    switch (ikey->get_key_type()) {
      case UNC_KT_VTN:
        /* Return only the incoming configuration count when the requested
         * operation is READ_SIBLING_COUNT and the configuration mode as
         * TC_CONFIG_VTN and the datatype is UPLL_DT_CANDIDATE
        */
        result_code = GetConfigModeInfo(header, config_mode, vtn_name);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("GetConfigMode failed");
          return result_code;
        }
        if (config_mode == TC_CONFIG_VTN &&
          header->datatype == UPLL_DT_CANDIDATE) {
          UPLL_LOG_DEBUG("Read sibling count but VTN mode");
          result_code = GetChildConfigKey(ckv, NULL);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_ERROR("GetChildConfigKey Error");
            return result_code;
          }
          vtn_key = reinterpret_cast<key_vtn_t *>(ckv->get_key());
          uuu::upll_strncpy(vtn_key->vtn_name, vtn_name.c_str(),
                    (kMaxLenVtnName+1));
          result_code = UpdateConfigDB(ckv, UPLL_DT_CANDIDATE, UNC_OP_READ,
                                dmi, &dbop, MAINTBL);
          if (result_code != UPLL_RC_ERR_INSTANCE_EXISTS) {
            UPLL_LOG_ERROR("UpdateConfigDB Error");
            if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE)
              header->rep_count = 0;
            DELETE_IF_NOT_NULL(ckv);
            return result_code;
          }
          result_code = GetChildConfigKey(ikey, ckv);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_ERROR("GetChildConfigKey Error");
            DELETE_IF_NOT_NULL(ckv);
            return result_code;
          }
          DELETE_IF_NOT_NULL(ckv);
          ikey->SetCfgVal(NULL);
          count1 = reinterpret_cast<uint32_t *>(
                   ConfigKeyVal::Malloc(sizeof(uint32_t)));
          header->rep_count = 1;
          *count1 = 1;
          ikey->SetCfgVal(new ConfigVal(IpctSt::kIpcStUint32, count1));
          UPLL_LOG_TRACE("IKEY in readmo is %s ",
                          ikey->ToStrAll().c_str());
         return UPLL_RC_SUCCESS;
        }
        result_code = MoMgrImpl::ReadSiblingCount(header, ikey, dmi);
        return result_code;
        break;
      case UNC_KT_VTNSTATION_CONTROLLER:
        result_code = DupConfigKeyValVtnStation(okey, ikey);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_INFO("DupConfigKeyValVtnStation failed err code %d",
                        result_code);
          return result_code;
        }
        header->operation = UNC_OP_READ_SIBLING_BEGIN;
        header->rep_count = UINT32_MAX;
        result_code = ReadSiblingMo(header, okey, false, dmi);
        DELETE_IF_NOT_NULL(okey);
        header->operation = UNC_OP_READ_SIBLING_COUNT;
        count =
            reinterpret_cast<uint32_t*>(ConfigKeyVal::Malloc(sizeof(uint32_t)));
         *count = header->rep_count;
         ikey->SetCfgVal(new ConfigVal(IpctSt::kIpcStUint32, count));
        break;
      default:
        break;
    }
    return result_code;
}

upll_rc_t
VtnMoMgr::ReadSiblingMo(IpcReqRespHeader *header,
                        ConfigKeyVal *ikey,
                        bool begin,
                        DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  if ((ikey->get_key_type() == UNC_KT_VTN_MAPPING_CONTROLLER) ||
      (ikey->get_key_type() == UNC_KT_VTNSTATION_CONTROLLER)) {
    result_code = ValidateMessage(header, ikey);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("ValidateMessage failed result_code %d",
                  result_code);
      return result_code;
    }
  }
  controller_domain ctrlr_dom;
  ctrlr_dom.ctrlr = NULL;
  ctrlr_dom.domain = NULL;
  ConfigKeyVal *next_ckv = NULL;
  uint32_t rec_count = 0;
  uuc::CtrlrMgr *ctrlr_mgr = uuc::CtrlrMgr::GetInstance();
  string ctrlr_id;
  unc_keytype_ctrtype_t ctrlrtype = UNC_CT_UNKNOWN;
  ConfigKeyVal *okey = NULL;
  key_vtnstation_controller *vtn_stkey = NULL;
  controller_domain vnode_ctrlr_dom;
  vnode_ctrlr_dom.ctrlr = NULL;
  vnode_ctrlr_dom.domain = NULL;
  bool is_vnode_filter_valid = false;
  uint32_t count = 0;
  TcConfigMode config_mode = TC_CONFIG_INVALID;
  std::string vtn_name;
  key_vtn_t *vtn_key = NULL;
  unc_keytype_operation_t operation;

  switch (ikey->get_key_type()) {
    case UNC_KT_VTN:
    /* Return only the incoming configuration when the requested operation is
       READ_SIBLING and the configuration mode as TC_CONFIG_VTN
       and the datatype is UPLL_DT_CANDIDATE
    */
      if (header->datatype == UPLL_DT_CANDIDATE) {
         result_code = GetConfigModeInfo(header, config_mode, vtn_name);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("GetConfigMode failed");
          return result_code;
        }
      }
      if (config_mode == TC_CONFIG_VTN &&
          header->datatype == UPLL_DT_CANDIDATE) {
        UPLL_LOG_DEBUG("Read sibling but VTN mode");
        UPLL_LOG_TRACE("IKEY in readmo is %s ",
                  ikey->ToStrAll().c_str());
        operation = header->operation;
        vtn_key = reinterpret_cast<key_vtn *>(ikey->get_key());

        if (!begin) {
          if (strcmp(reinterpret_cast<const char *>(vtn_key->vtn_name),
                     vtn_name.c_str()) >= 0) {
             UPLL_LOG_DEBUG("Readsibling for but diff ");
             return UPLL_RC_ERR_NO_SUCH_INSTANCE;
          } else {
            // Load config mode VTN name into ikey, and return READ result
          }
        }

        uuu::upll_strncpy(vtn_key->vtn_name, vtn_name.c_str(),
                          (kMaxLenVtnName+1));
        UPLL_LOG_DEBUG("pyn pyn %s %s", vtn_key->vtn_name, vtn_name.c_str());
        header->operation = UNC_OP_READ;
        result_code = MoMgrImpl::ReadMo(header, ikey, dmi);
        header->operation = operation;
        if (result_code != UPLL_RC_SUCCESS) {
           if (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
              UPLL_LOG_ERROR("Read Error");
              return result_code;
           }
           header->rep_count = 0;
           return result_code;
        }
        header->rep_count = 1;
        return result_code;
      }
      UPLL_LOG_TRACE("IKEY in readmo is %s ",
                  ikey->ToStrAll().c_str());
      result_code = MoMgrImpl::ReadSiblingMo(header, ikey, false, dmi);
      return result_code;
      break;
    case UNC_KT_VTNSTATION_CONTROLLER: {
      result_code = DupConfigKeyValVtnStation(okey, ikey);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("DupConfigKeyValVtnStation failed err code %d",
                        result_code);
        return result_code;
      }
      if (header->operation == UNC_OP_READ_SIBLING_BEGIN) {
        result_code = ctrlr_mgr->GetFirstCtrlrName(UPLL_DT_RUNNING,
                                                   &ctrlr_id);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_INFO("GetFirstCtrlrName failed err code %d", result_code);
          DELETE_IF_NOT_NULL(okey);
          return result_code;
        }
        ctrlr_dom.ctrlr = reinterpret_cast<uint8_t *>(
                const_cast<char *>(ctrlr_id.c_str()));
        UPLL_LOG_DEBUG("ControllerId and DomainId are %s %s", ctrlr_dom.ctrlr,
                          ctrlr_dom.domain);
        if ((!ctrlr_mgr->GetCtrlrType(reinterpret_cast<char *>
             (ctrlr_dom.ctrlr), UPLL_DT_RUNNING, &ctrlrtype)) ||
            (ctrlrtype != UNC_CT_PFC)) {
          UPLL_LOG_INFO("Controller type is  %d", ctrlrtype);
        } else {
          uuu::upll_strncpy(reinterpret_cast<key_vtnstation_controller *>
            (okey->get_key())->controller_name, ctrlr_dom.ctrlr,
            (kMaxLenCtrlrId + 1));
          result_code = ReadSingleCtlrlStation(header, okey,
                                               is_vnode_filter_valid,
                                               &vnode_ctrlr_dom,
                                               &rec_count, dmi);
          if (result_code == UPLL_RC_SUCCESS) {
            count++;
          }
          if (is_vnode_filter_valid) {
            if (result_code == UPLL_RC_SUCCESS) {
              header->rep_count = count;
              ikey->ResetWith(okey);
            } else {
              UPLL_LOG_INFO("ReadSingleCtlrlStation failed err_code %d",
                            result_code);
            }
            DELETE_IF_NOT_NULL(vnode_ctrlr_dom.ctrlr);
            DELETE_IF_NOT_NULL(vnode_ctrlr_dom.domain);
            DELETE_IF_NOT_NULL(okey);
            return result_code;
          }
        }
      } else {
        ctrlr_id = reinterpret_cast<char *>
                      (reinterpret_cast<key_vtnstation_controller *>
                      (okey->get_key())->controller_name);
      }
      UPLL_LOG_DEBUG("Input Controller Id is %s", ctrlr_id.c_str());
      while ((UPLL_RC_SUCCESS == ctrlr_mgr->GetNextCtrlrName(ctrlr_id,
              UPLL_DT_RUNNING, &ctrlr_id)) &&
              (count <= header->rep_count)) {
        UPLL_LOG_DEBUG("sibling Controller Id is %s", ctrlr_id.c_str());
        ctrlr_dom.ctrlr = reinterpret_cast<uint8_t *>(
                  const_cast<char *>(ctrlr_id.c_str()));
        if ((!ctrlr_mgr->GetCtrlrType(reinterpret_cast<char *>
            (ctrlr_dom.ctrlr), UPLL_DT_RUNNING, &ctrlrtype)) ||
            (ctrlrtype != UNC_CT_PFC)) {
          UPLL_LOG_INFO("Controller type is  %d", ctrlrtype);
          continue;
        }
        if (is_vnode_filter_valid) {
          if (!(strncmp(reinterpret_cast<const char *>(vnode_ctrlr_dom.ctrlr),
                        reinterpret_cast<const char *>(ctrlr_dom.ctrlr),
                        (kMaxLenCtrlrId + 1)))) {
            header->rep_count = count;
            ikey->ResetWith(okey);
            DELETE_IF_NOT_NULL(okey);
            DELETE_IF_NOT_NULL(vnode_ctrlr_dom.ctrlr);
            DELETE_IF_NOT_NULL(vnode_ctrlr_dom.domain);
            return result_code;
          } else {
            continue;
          }
        }
        result_code = DupConfigKeyValVtnStation(next_ckv, ikey);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("DupConfigKeyValVtnStation failed err code %d",
                          result_code);
          DELETE_IF_NOT_NULL(okey);
          DELETE_IF_NOT_NULL(vnode_ctrlr_dom.ctrlr);
          DELETE_IF_NOT_NULL(vnode_ctrlr_dom.domain);
          return result_code;
        }
        vtn_stkey = reinterpret_cast<key_vtnstation_controller *>
                                     (next_ckv->get_key());
        uuu::upll_strncpy(vtn_stkey->controller_name, ctrlr_dom.ctrlr,
                          (kMaxLenCtrlrId + 1));
        result_code = ReadSingleCtlrlStation(header, next_ckv,
                                            is_vnode_filter_valid,
                                            &vnode_ctrlr_dom,
                                            &rec_count, dmi);
          if (result_code != UPLL_RC_SUCCESS &&
              result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
            next_ckv = NULL;
            continue;
          }
          count++;
          okey->AppendCfgKeyVal(next_ckv);
          next_ckv = NULL;
      }
      if (is_vnode_filter_valid) {
        if (result_code == UPLL_RC_SUCCESS) {
          DELETE_IF_NOT_NULL(next_ckv);
        }
        DELETE_IF_NOT_NULL(vnode_ctrlr_dom.ctrlr);
        DELETE_IF_NOT_NULL(vnode_ctrlr_dom.domain);
        DELETE_IF_NOT_NULL(okey);
        result_code =
            (result_code == UPLL_RC_SUCCESS) ?
            UPLL_RC_ERR_NO_SUCH_INSTANCE : result_code;
        return result_code;
      }
      header->rep_count = count;
      ikey->ResetWith(okey);
      DELETE_IF_NOT_NULL(okey);
    break;
    }
    case UNC_KT_VTN_MAPPING_CONTROLLER:  {
      ConfigKeyVal *ck_ctrlr = NULL;
      result_code = GetChildConfigKey(ck_ctrlr, ikey);
      /* For ReadSibling operation it is not necessary that the given vtn
       * should be present in given controller and domain */
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_INFO("GetChildConfigKey failed err code %d", result_code);
        return result_code;
      }
      DbSubOp op = {kOpReadExist, kOpMatchNone, kOpInOutCtrlr | kOpInOutDomain};
      result_code = UpdateConfigDB(ck_ctrlr, UPLL_DT_RUNNING,
            UNC_OP_READ, dmi, &op, CTRLRTBL);
      if (result_code != UPLL_RC_ERR_INSTANCE_EXISTS) {
        UPLL_LOG_DEBUG("ReadConfigDB Failed result_code - %d", result_code);
        DELETE_IF_NOT_NULL(ck_ctrlr);
        return result_code;
      }
      DELETE_IF_NOT_NULL(ck_ctrlr);
      result_code = DupConfigKeyValVtnMapping(okey, ikey);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_INFO("DupConfigKeyValVtnMapping failed err code %d",
                        result_code);
        DELETE_IF_NOT_NULL(okey);
        return result_code;
      }
      controller_domain ctrlr_dom_tmp;
      controller_domain *vnode_ctrlr_dom_map  = &ctrlr_dom_tmp;
      vnode_ctrlr_dom_map->ctrlr = NULL;
      vnode_ctrlr_dom_map->domain = NULL;
      result_code = ValidateVnodeFilter(okey, CTRLR_DOM_INVALID,
                                        is_vnode_filter_valid,
                                        &vnode_ctrlr_dom_map, dmi);
      if (result_code != UPLL_RC_SUCCESS &&
          (!vnode_ctrlr_dom_map->ctrlr || !vnode_ctrlr_dom_map->domain)) {
        UPLL_LOG_INFO("ValidateVnodeFilter failed error code %d", result_code);
        DELETE_IF_NOT_NULL(okey);
        DELETE_IF_NOT_NULL(vnode_ctrlr_dom_map->ctrlr);
        DELETE_IF_NOT_NULL(vnode_ctrlr_dom_map->domain);
        return result_code;
      }
      bool okey_mapped = false;
      if (header->operation == UNC_OP_READ_SIBLING_BEGIN) {
        if (is_vnode_filter_valid) {
          uuu::upll_strncpy(reinterpret_cast<key_vtn_controller *>
          (okey->get_key())->controller_name, vnode_ctrlr_dom_map->ctrlr,
                          (kMaxLenCtrlrId + 1));
          uuu::upll_strncpy(reinterpret_cast<key_vtn_controller *>
          (okey->get_key())->domain_id, vnode_ctrlr_dom_map->domain,
                          (kMaxLenDomainId + 1));
          SET_USER_DATA_CTRLR_DOMAIN(okey, *vnode_ctrlr_dom_map);
          result_code = ReadSingleCtlrlDomVtnMapping(header, okey,
                                                 is_vnode_filter_valid, dmi);
          if (result_code != UPLL_RC_SUCCESS) {
          /* Possible error codes are UPLL_RC_ERR_NO_SUCH_INSTANCE
             and database error codes
             Since it is a vnode filter, for UPLL_RC_ERR_NO_SUCH_INSTANCE
             case should not continue further */
           UPLL_LOG_INFO("ReadSingleCtlrlDomVtnMapping returning error code %d",
                         result_code);
           DELETE_IF_NOT_NULL(okey);
           DELETE_IF_NOT_NULL(vnode_ctrlr_dom_map->ctrlr);
           DELETE_IF_NOT_NULL(vnode_ctrlr_dom_map->domain);
           return result_code;
          }
          count++;
          header->rep_count = count;
          ikey->ResetWith(okey);
          DELETE_IF_NOT_NULL(okey);
          DELETE_IF_NOT_NULL(vnode_ctrlr_dom_map->ctrlr);
          DELETE_IF_NOT_NULL(vnode_ctrlr_dom_map->domain);
          return result_code;
        }
        // vnode filter is not specified
        result_code = ctrlr_mgr->GetFirstCtrlrName(
                                 UPLL_DT_RUNNING, &ctrlr_id);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_INFO("GetFirstCtrlrName failed err code %d", result_code);
          DELETE_IF_NOT_NULL(okey);
          return result_code;
        }
        ctrlr_dom.ctrlr = reinterpret_cast<uint8_t *>(
                const_cast<char *>(ctrlr_id.c_str()));
        ctrlr_dom.domain = reinterpret_cast<uint8_t *>
                               (const_cast<char *>(" "));
        SET_USER_DATA_CTRLR_DOMAIN(okey, ctrlr_dom);
        UPLL_LOG_TRACE("ControllerId and DomainId are %s %s", ctrlr_dom.ctrlr,
                          ctrlr_dom.domain);
        if ((ctrlr_mgr->GetCtrlrType(reinterpret_cast<char *>
                          (ctrlr_dom.ctrlr), UPLL_DT_RUNNING, &ctrlrtype)) &&
                          (ctrlrtype == UNC_CT_PFC)) {
          uuu::upll_strncpy(reinterpret_cast<key_vtn_controller *>
            (okey->get_key())->controller_name, ctrlr_dom.ctrlr,
            (kMaxLenCtrlrId + 1));
          result_code = ReadSingleCtlrlVtnMapping(header, okey,
                                                  dmi, &rec_count,
                                                  is_vnode_filter_valid);
          if (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE &&
            result_code != UPLL_RC_SUCCESS) {
            delete okey;
            return result_code;
          } else if (result_code == UPLL_RC_SUCCESS) {
            okey_mapped = true;
          }
        }
      } else {  // header->operation == UNC_OP_READ_SIBLING condition
        ctrlr_id = reinterpret_cast<char *>
                     (reinterpret_cast<key_vtn_controller *>
                     (okey->get_key())->controller_name);
        UPLL_LOG_TRACE("Controller Name is %s", ctrlr_id.c_str());
        ctrlr_dom.ctrlr = reinterpret_cast<uint8_t *>(
                  const_cast<char *>(ctrlr_id.c_str()));
        ctrlr_dom.domain = reinterpret_cast<key_vtn_controller *>
                                 (okey->get_key())->domain_id;
        if (is_vnode_filter_valid) {
          if (!(strncmp(
                      reinterpret_cast<const char *>
                      (vnode_ctrlr_dom_map->ctrlr),
                      reinterpret_cast<const char *>(ctrlr_dom.ctrlr),
                      (kMaxLenCtrlrId + 1)))) {
            if (strncmp(reinterpret_cast<const char *>
                        (vnode_ctrlr_dom_map->domain),
                        reinterpret_cast<const char *>(ctrlr_dom.domain),
                        (kMaxLenDomainId + 1)) > 0) {
              uuu::upll_strncpy(reinterpret_cast<key_vtn_controller *>
                (okey->get_key())->controller_name, vnode_ctrlr_dom_map->ctrlr,
                          (kMaxLenCtrlrId + 1));
              uuu::upll_strncpy(reinterpret_cast<key_vtn_controller *>
                (okey->get_key())->domain_id, vnode_ctrlr_dom_map->domain,
                          (kMaxLenDomainId + 1));
              SET_USER_DATA_CTRLR_DOMAIN(okey, *vnode_ctrlr_dom_map);
              result_code = ReadSingleCtlrlDomVtnMapping(header, okey,
                                                 is_vnode_filter_valid, dmi);
              if (result_code != UPLL_RC_SUCCESS) {
                /* Possible error codes are UPLL_RC_ERR_NO_SUCH_INSTANCE
                 * and database error codes
                 * Since it is a vnode filter, for UPLL_RC_ERR_NO_SUCH_INSTANCE
                 * case should not continue further */
                UPLL_LOG_INFO(
                    "ReadSingleCtlrlDomVtnMapping returning error code %d",
                    result_code);
                DELETE_IF_NOT_NULL(okey);
                DELETE_IF_NOT_NULL(vnode_ctrlr_dom_map->ctrlr);
                DELETE_IF_NOT_NULL(vnode_ctrlr_dom_map->domain);
                return result_code;
              }
              count++;
              header->rep_count = count;
              ikey->ResetWith(okey);
              DELETE_IF_NOT_NULL(okey);
              DELETE_IF_NOT_NULL(vnode_ctrlr_dom_map->ctrlr);
              DELETE_IF_NOT_NULL(vnode_ctrlr_dom_map->domain);
              return result_code;
            } else {
              DELETE_IF_NOT_NULL(okey);
              DELETE_IF_NOT_NULL(vnode_ctrlr_dom_map->ctrlr);
              DELETE_IF_NOT_NULL(vnode_ctrlr_dom_map->domain);
              return UPLL_RC_ERR_NO_SUCH_INSTANCE;
            }
          }
        } else {  // is_vnode_filter_valid is false
          if ((ctrlr_mgr->GetCtrlrType(reinterpret_cast<char *>
             (ctrlr_dom.ctrlr), UPLL_DT_RUNNING, &ctrlrtype)) &&
             (ctrlrtype == UNC_CT_PFC)) {
            UPLL_LOG_INFO("Controller type is  %d", ctrlrtype);
            ctrlr_dom.domain = reinterpret_cast<key_vtn_controller *>
                                 (okey->get_key())->domain_id;
            UPLL_LOG_TRACE("Domain name is %s", ctrlr_dom.domain);
            SET_USER_DATA_CTRLR_DOMAIN(okey, ctrlr_dom);

            result_code = ReadSingleCtlrlVtnMapping(header, okey,
                                                    dmi, &rec_count,
                                                    is_vnode_filter_valid);
            if (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE &&
                result_code != UPLL_RC_SUCCESS) {
              delete okey;
              return result_code;
            } else if (result_code == UPLL_RC_SUCCESS) {
              okey_mapped = true;
            }
          }
        }
      }
      UPLL_LOG_DEBUG("Input controller id is %s", ctrlr_id.c_str());
      while ((UPLL_RC_SUCCESS == ctrlr_mgr->GetNextCtrlrName(ctrlr_id,
                                          UPLL_DT_RUNNING, &ctrlr_id)) &&
                                          (rec_count < header->rep_count)) {
        UPLL_LOG_DEBUG("Sibling controller id is %s", ctrlr_id.c_str());
        ctrlr_dom.ctrlr = reinterpret_cast<uint8_t *>(
                  const_cast<char *>(ctrlr_id.c_str()));
        if (!ctrlr_mgr->GetCtrlrType(reinterpret_cast<char*>(ctrlr_dom.ctrlr),
                     UPLL_DT_RUNNING, &ctrlrtype)) {
          continue;
        }
        if (ctrlrtype != UNC_CT_PFC) {
          UPLL_LOG_INFO("Controller type is  %d", ctrlrtype);
          continue;
        }
        ctrlr_dom.domain = reinterpret_cast<uint8_t *>
                           (const_cast<char *>(" "));
        SET_USER_DATA_CTRLR_DOMAIN(ikey, ctrlr_dom);
        next_ckv = NULL;
        result_code = DupConfigKeyValVtnMapping(next_ckv, ikey);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("DupConfigKeyValVtnStation failed"
                         " err code %d", result_code);
          delete okey;
          DELETE_IF_NOT_NULL(next_ckv);
          return result_code;
        }
        if (is_vnode_filter_valid) {
          if (!(strncmp(
                      reinterpret_cast<const char *>
                      (vnode_ctrlr_dom_map->ctrlr),
                      reinterpret_cast<const char *>(ctrlr_dom.ctrlr),
                      (kMaxLenCtrlrId + 1)))) {
              if (strncmp(
                      reinterpret_cast<const char *>
                      (vnode_ctrlr_dom_map->domain),
                      reinterpret_cast<const char *>(ctrlr_dom.domain),
                      (kMaxLenDomainId + 1)) >= 0) {
                uuu::upll_strncpy(reinterpret_cast<key_vtn_controller *>
                (okey->get_key())->controller_name, vnode_ctrlr_dom_map->ctrlr,
                          (kMaxLenCtrlrId + 1));
                uuu::upll_strncpy(reinterpret_cast<key_vtn_controller *>
                (okey->get_key())->domain_id, vnode_ctrlr_dom_map->domain,
                          (kMaxLenDomainId + 1));
                SET_USER_DATA_CTRLR_DOMAIN(okey, *vnode_ctrlr_dom_map);
                result_code = ReadSingleCtlrlDomVtnMapping(header, okey,
                                                 is_vnode_filter_valid, dmi);
                if (result_code != UPLL_RC_SUCCESS) {
                  /* Possible error codes are UPLL_RC_ERR_NO_SUCH_INSTANCE
                    and database error codes */
                  UPLL_LOG_INFO(
                      "ReadSingleCtlrlDomVtnMapping returning error code %d",
                      result_code);
                  DELETE_IF_NOT_NULL(okey);
                  DELETE_IF_NOT_NULL(vnode_ctrlr_dom_map->ctrlr);
                  DELETE_IF_NOT_NULL(vnode_ctrlr_dom_map->domain);
                  return result_code;
                }
                count++;
                header->rep_count = count;
                ikey->ResetWith(okey);
                DELETE_IF_NOT_NULL(okey);
                DELETE_IF_NOT_NULL(vnode_ctrlr_dom_map->ctrlr);
                DELETE_IF_NOT_NULL(vnode_ctrlr_dom_map->domain);
                return result_code;
              } else {
                header->rep_count = count;
                DELETE_IF_NOT_NULL(okey);
                DELETE_IF_NOT_NULL(vnode_ctrlr_dom_map->ctrlr);
                DELETE_IF_NOT_NULL(vnode_ctrlr_dom_map->domain);
                return UPLL_RC_ERR_NO_SUCH_INSTANCE;
              }
          } else {
              continue;
          }
        } else {
          key_vtn_controller *vtnkey =
              reinterpret_cast<key_vtn_controller *>(next_ckv->get_key());
          uuu::upll_strncpy(vtnkey->controller_name, ctrlr_dom.ctrlr,
                          (kMaxLenCtrlrId + 1));
          uuu::upll_strncpy(vtnkey->domain_id, ctrlr_dom.domain,
                          (kMaxLenDomainId + 1));
          result_code = ReadSingleCtlrlVtnMapping(header, next_ckv, dmi,
                                                &rec_count,
                                                is_vnode_filter_valid);
          if (result_code != UPLL_RC_SUCCESS &&
             result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
            delete okey;
            DELETE_IF_NOT_NULL(next_ckv);
            return result_code;
          } else if (result_code == UPLL_RC_SUCCESS) {
            if (!okey_mapped) {
             /* This recent ReadSingleCtlrlVtnMapping gave result,
             ** but not the previous ones
             */
             DELETE_IF_NOT_NULL(okey);
             okey = next_ckv;
             okey_mapped = true;
            } else {
              okey->AppendCfgKeyVal(next_ckv);
            }
          } else {
            DELETE_IF_NOT_NULL(next_ckv);
          }
        }
      }
      if (is_vnode_filter_valid) {
        DELETE_IF_NOT_NULL(vnode_ctrlr_dom.ctrlr);
        DELETE_IF_NOT_NULL(vnode_ctrlr_dom.domain);
        result_code =
            (result_code == UPLL_RC_SUCCESS) ? UPLL_RC_ERR_NO_SUCH_INSTANCE :
            result_code;
        return result_code;
      }
      if (rec_count != 0) {
        header->rep_count = rec_count;
        ikey->ResetWith(okey);
        DELETE_IF_NOT_NULL(okey);
        result_code = UPLL_RC_SUCCESS;
      } else {
        DELETE_IF_NOT_NULL(okey);
        DELETE_IF_NOT_NULL(next_ckv);
        if (result_code == UPLL_RC_SUCCESS ||
            result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
          result_code = UPLL_RC_ERR_NO_SUCH_INSTANCE;
        }
      }
      break;
    }
    default: {
      return UPLL_RC_ERR_GENERIC;
      break;
    }
  }
  return result_code;
} // NOLINT

upll_rc_t VtnMoMgr::GetVbrIfFromVexternal(uint8_t *unc_vtn_name,
                                          uint8_t *vnode_name,
                                          uint8_t *vnode_if_name,
                                          uint8_t vnode_if_valid,
                                          controller_domain *ctrlr_dom,
                                          DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  key_vbr_if_t *key_vbrif = static_cast<key_vbr_if_t *>
  (ConfigKeyVal::Malloc(sizeof(key_vbr_if_t)));
  uuu::upll_strncpy(key_vbrif->vbr_key.vtn_key.vtn_name,
                  unc_vtn_name, (kMaxLenVtnName + 1));
  val_drv_vbr_if_t *drv_val_vbrif = static_cast<val_drv_vbr_if_t *>
  (ConfigKeyVal::Malloc(sizeof(val_drv_vbr_if_t)));
  drv_val_vbrif->valid[PFCDRV_IDX_VEXT_NAME_VBRIF] = UNC_VF_VALID;
  if (vnode_if_valid == UNC_VF_VALID) {
    uuu::upll_strncpy(drv_val_vbrif->vex_name,
        vnode_if_name, (kMaxLenInterfaceName + 1));
  } else {
    uuu::upll_strncpy(drv_val_vbrif->vex_name,
        vnode_name, (kMaxLenVnodeName + 1));
  }
  ConfigKeyVal *tmpckv = new
    ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf, key_vbrif,
        new ConfigVal(IpctSt::kIpcStPfcdrvValVbrIf, drv_val_vbrif));
  SET_USER_DATA_CTRLR_DOMAIN(tmpckv, *ctrlr_dom);
  VbrIfMoMgr *vbr_if_mgr = reinterpret_cast<VbrIfMoMgr *>
      (const_cast<MoManager *>(GetMoManager(UNC_KT_VBR_IF)));
  DbSubOp dbop = { kOpReadSingle, kOpMatchCtrlr | kOpInOutDomain,
    kOpInOutNone };
  result_code = vbr_if_mgr->ReadConfigDB(tmpckv, UPLL_DT_RUNNING,
      UNC_OP_READ, dbop, dmi, MAINTBL);
  if (result_code == UPLL_RC_SUCCESS) {
     uuu::upll_strncpy(vnode_name, reinterpret_cast<key_vbr_if_t *>
       (tmpckv->get_key())->vbr_key.vbridge_name, (kMaxLenVnodeName + 1));
     uuu::upll_strncpy(vnode_if_name, reinterpret_cast<key_vbr_if_t *>
        (tmpckv->get_key())->if_name, (kMaxLenInterfaceName + 1));
  }
  DELETE_IF_NOT_NULL(tmpckv);
  return result_code;
}

upll_rc_t
VtnMoMgr::MappingvExtTovBr(ConfigKeyVal * ikey,
                           IpcReqRespHeader * req,
                           DalDmlIntf * dmi, uint32_t *&rec_count)  {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  if ((GetVal(ikey)) == NULL) {
     UPLL_LOG_DEBUG("Val struct is not present from driver response");
     return UPLL_RC_ERR_GENERIC;
  }
  UPLL_LOG_DEBUG("ikey in MappingvExtTovBr is %s", ikey->ToStrAll().c_str());
  VtermIfMoMgr *vterm_if_mgr = reinterpret_cast<VtermIfMoMgr *>
      (const_cast<MoManager *>(GetMoManager(UNC_KT_VTERM_IF)));
  if (!vterm_if_mgr) {
    UPLL_LOG_ERROR("Invalid Mgr");
    return UPLL_RC_ERR_GENERIC;
  }
  uint8_t pfc_vtn_name[kMaxLenVtnName + 1];
  uint8_t *unc_vtn_name;
  controller_domain ctrlr_dom;
  key_vtn_t *vtnkey = reinterpret_cast<key_vtn_t *>
        (ConfigKeyVal::Malloc(sizeof(key_vtn_t)));
  if (UNC_KT_VTN_MAPPING_CONTROLLER == ikey->get_key_type()) {
    unc_vtn_name = reinterpret_cast<key_vtn_controller *>
                   (ikey->get_key())->vtn_key.vtn_name;
    uuu::upll_strncpy(pfc_vtn_name,
                      reinterpret_cast<key_vtn_controller *>
                      (ikey->get_key())->vtn_key.vtn_name,
                      (kMaxLenVtnName + 1));
    uuu::upll_strncpy(vtnkey->vtn_name,
                      pfc_vtn_name,
                      (kMaxLenVtnName + 1));
    ctrlr_dom.ctrlr =  reinterpret_cast<key_vtn_controller *>
        (ikey->get_key())->controller_name;
  } else {
    unc_vtn_name  = reinterpret_cast<val_vtnstation_controller_st *>
                      (GetStateVal(ikey))->vtn_name;
    uuu::upll_strncpy(pfc_vtn_name,
                      reinterpret_cast<val_vtnstation_controller_st *>
                      (GetStateVal(ikey))->vtn_name,
                      (kMaxLenVtnName + 1));
    uuu::upll_strncpy(vtnkey->vtn_name,
                      pfc_vtn_name,
                      (kMaxLenVtnName + 1));
    ctrlr_dom.ctrlr =  reinterpret_cast<key_vtnstation_controller *>
        (ikey->get_key())->controller_name;
  }
  ConfigKeyVal *ckv_rename = new ConfigKeyVal(UNC_KT_VTN,
                                     IpctSt::kIpcStKeyVtn, vtnkey, NULL);
  result_code = GetRenamedUncKey(ckv_rename, UPLL_DT_RUNNING,
                                   dmi, ctrlr_dom.ctrlr);
  if (result_code != UPLL_RC_SUCCESS &&
        result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    UPLL_LOG_INFO("GetRenamedUncKey failed. Error : %d", result_code);
    DELETE_IF_NOT_NULL(ckv_rename);
    return result_code;
  }
  result_code =
      (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) ? UPLL_RC_SUCCESS :
      result_code;
  uuu::upll_strncpy(unc_vtn_name, reinterpret_cast<key_vtn_t *>
                     (ckv_rename->get_key())->vtn_name,
                     (kMaxLenVtnName + 1));
  DELETE_IF_NOT_NULL(ckv_rename);

  ConfigVal *tmp_cval = NULL;
  uint8_t *vnode_type;
  uint8_t *vnode_if_valid;
  uint8_t *vnode_type_valid;
  uint8_t map_type;
  uint8_t *vnode_name;
  uint8_t *vnode_if_name;
  for (tmp_cval = ikey->get_cfg_val(); tmp_cval;
       tmp_cval = tmp_cval->get_next_cfg_val()) {
    if (IpctSt::kIpcStValVtnstationControllerSt ==
        tmp_cval->get_st_num() || (IpctSt::kIpcStValVtnMappingControllerSt ==
        tmp_cval->get_st_num())) {
      if (UNC_KT_VTNSTATION_CONTROLLER == ikey->get_key_type()) {
        map_type = reinterpret_cast<val_vtnstation_controller_st_t *>(
                            tmp_cval->get_val())->map_type;
        vnode_type = &(reinterpret_cast<val_vtnstation_controller_st_t *>(
                            tmp_cval->get_val())->vnode_type);
        vnode_name = reinterpret_cast<val_vtnstation_controller_st_t *>
                            (tmp_cval->get_val())->vnode_name;
        vnode_if_name = reinterpret_cast<val_vtnstation_controller_st_t *>(
                            tmp_cval->get_val())->vnode_if_name;
        vnode_type_valid =
            &(reinterpret_cast<val_vtnstation_controller_st_t *>(
            tmp_cval->get_val())->valid[UPLL_IDX_VNODE_TYPE_VSCS]);
        vnode_if_valid =
            &(reinterpret_cast<val_vtnstation_controller_st_t *>(
            tmp_cval->get_val())->valid[UPLL_IDX_VNODE_IF_NAME_VSCS]);
        ctrlr_dom.domain = reinterpret_cast<val_vtnstation_controller_st_t *>
                            (tmp_cval->get_val())->domain_id;
      } else {
        map_type = reinterpret_cast<val_vtn_mapping_controller_st *>(
                            tmp_cval->get_val())->map_type;
        vnode_type = &(reinterpret_cast<val_vtn_mapping_controller_st *>(
                            tmp_cval->get_val())->vnode_type);
        vnode_name = reinterpret_cast<val_vtn_mapping_controller_st *>(
                            tmp_cval->get_val())->vnode_name;
        vnode_if_name = reinterpret_cast<val_vtn_mapping_controller_st *>(
                            tmp_cval->get_val())->vnode_if_name;
        vnode_type_valid =
            &(reinterpret_cast<val_vtn_mapping_controller_st *>(
            tmp_cval->get_val())->valid[UPLL_IDX_VNODE_TYPE_VMCS]);
        vnode_if_valid =
            &(reinterpret_cast<val_vtn_mapping_controller_st *>(
           tmp_cval->get_val())->valid[UPLL_IDX_VNODE_IF_NAME_VMCS]);
        ctrlr_dom.domain =
            reinterpret_cast<key_vtn_controller *>(ikey->get_key())->domain_id;
      }
      (*rec_count)++;
      if (map_type == UPLL_IF_VLAN_MAP) {
        continue;
      }
      if (*vnode_type_valid == UNC_VF_VALID) {
        if (*vnode_type == UPLL_VNODE_VBRIDGE) {
          result_code = GetVbrIfFromVexternal(unc_vtn_name, vnode_name,
                                              vnode_if_name, *vnode_if_valid,
                                              &ctrlr_dom, dmi);
          if (result_code != UPLL_RC_SUCCESS &&
              result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
            UPLL_LOG_INFO("ConvertvExtTovBridge failed err code %d",
                          result_code);
            return result_code;
          } else if (result_code == UPLL_RC_SUCCESS) {
            *vnode_if_valid = UNC_VF_VALID;
          }
          result_code =
              (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) ? UPLL_RC_SUCCESS :
                        result_code;
        } else if (*vnode_type == UPLL_VNODE_VTERMINAL) {
          result_code = vterm_if_mgr->GetVtermIfFromVexternal(
              pfc_vtn_name, vnode_name,
              vnode_if_name, ctrlr_dom.ctrlr, dmi);
          if (result_code != UPLL_RC_SUCCESS &&
              result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
            UPLL_LOG_INFO("GetVtermIfFromVexternal failed err code %d",
                          result_code);
            return result_code;
          } else if (result_code == UPLL_RC_SUCCESS) {
            *vnode_if_valid = UNC_VF_VALID;
          }
          result_code =
              (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) ? UPLL_RC_SUCCESS :
              result_code;
        }
      } else {
          result_code =  GetVbrIfFromVexternal(unc_vtn_name, vnode_name,
                                               vnode_if_name, *vnode_if_valid,
                                               &ctrlr_dom, dmi);
          if (result_code != UPLL_RC_SUCCESS &&
              result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
            UPLL_LOG_INFO("ConvertvExtTovBridge failed err code %d",
                          result_code);
            return result_code;
          } else if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
            result_code = vterm_if_mgr->GetVtermIfFromVexternal(
                pfc_vtn_name, vnode_name, vnode_if_name,
                ctrlr_dom.ctrlr, dmi);
            if (result_code != UPLL_RC_SUCCESS &&
                result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
              UPLL_LOG_INFO("GetVtermIfFromVexternal failed err code %d",
                            result_code);
              return result_code;
            } else if (result_code == UPLL_RC_SUCCESS) {
            /* Present in vBridge Interface table */
              *vnode_if_valid = UNC_VF_VALID;
              *vnode_type  = UPLL_VNODE_VBRIDGE;
            }
            result_code =
                (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) ?
                UPLL_RC_SUCCESS : result_code;
          } else {
            /* Present in vTerminal Interface table */
            *vnode_if_valid = UNC_VF_VALID;
            *vnode_type  = UPLL_VNODE_VBRIDGE;
          }
          *vnode_type_valid = UNC_VF_VALID;
      }
    }
  }
  return result_code;
}

/* Semantic check for the VTN Delete operation */
upll_rc_t
VtnMoMgr::IsReferenced(IpcReqRespHeader *req,
                       ConfigKeyVal *ikey,
                       DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *okey = NULL;
  if (NULL == ikey || !dmi)
    return UPLL_RC_ERR_GENERIC;
  /* Create the Vtunnel Configkey for checking vtn is underlay vtn or not */
  result_code = CreateVtunnelKey(ikey, okey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Could not Create VtunnelKey %d", result_code);
    return result_code;
  }
  MoMgrImpl *mgr = reinterpret_cast<MoMgrImpl *>
         (const_cast<MoManager *>(GetMoManager(UNC_KT_VTUNNEL)));
  if (!mgr) {
    UPLL_LOG_DEBUG("Instance is Null");
    delete okey;
    return UPLL_RC_ERR_GENERIC;
  }
  DbSubOp dbop = {kOpReadMultiple, kOpMatchNone, kOpInOutNone};
  /* Checks the given vtn is exists or not */
  result_code = mgr->ReadConfigDB(okey, req->datatype, UNC_OP_READ,
                                   dbop, dmi, MAINTBL);
  if (UPLL_RC_SUCCESS == result_code) {
    UPLL_LOG_DEBUG("Could not delete an Underlay Vtn referenced to "
                   " an Overlay Vtn");
    delete okey;
    return UPLL_RC_ERR_CFG_SEMANTIC;
  }
  result_code = (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE)?
                 UPLL_RC_SUCCESS:result_code;
  DELETE_IF_NOT_NULL(okey);
  return result_code;
}

/* This function creates the configkey for the vtunnel
 * This funciton take the vtn name and addit into the Vtunnel
 * value structure
 */
upll_rc_t
VtnMoMgr::CreateVtunnelKey(ConfigKeyVal *ikey,
                           ConfigKeyVal *&okey) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigVal *tmp;
  if (!ikey || !(ikey->get_key()))
    return UPLL_RC_ERR_GENERIC;

  key_vtunnel_t *vtunnel_key = reinterpret_cast<key_vtunnel_t *>
      (ConfigKeyVal::Malloc(sizeof(key_vtunnel_t)));
  val_vtunnel_t *vtn_val_vtunnel = reinterpret_cast<val_vtunnel_t *>
      (ConfigKeyVal::Malloc(sizeof(val_vtunnel_t)));
  /* validate message taken care of vtn lengh checking*/
  uuu::upll_strncpy(
      vtn_val_vtunnel->vtn_name,
      reinterpret_cast<key_vtn_t *>
      (ikey->get_key())->vtn_name, (kMaxLenVtnName+1));
  vtn_val_vtunnel->valid[UPLL_IDX_VTN_NAME_VTNL] = UNC_VF_VALID;

  tmp = new ConfigVal(IpctSt::kIpcStValVtunnel, vtn_val_vtunnel);
  if (!tmp) {
    UPLL_LOG_ERROR("Memory Allocation failed for tmp1");
    FREE_IF_NOT_NULL(vtunnel_key);
    FREE_IF_NOT_NULL(vtn_val_vtunnel);
    return UPLL_RC_ERR_GENERIC;
  }
  okey = new ConfigKeyVal(UNC_KT_VTUNNEL, IpctSt::kIpcStKeyVtunnel,
                          vtunnel_key, tmp);
  if (!okey) {
    delete tmp;
    FREE_IF_NOT_NULL(vtunnel_key);
    return UPLL_RC_ERR_GENERIC;
  }
  SET_USER_DATA(okey, ikey);
  return result_code;
}

upll_rc_t
VtnMoMgr::SwapKeyVal(ConfigKeyVal *ikey,
                     ConfigKeyVal *&okey,
                     DalDmlIntf *dmi,
                     uint8_t *ctrlr,
                     bool &no_rename) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  //  ConfigVal *tmp1;
  okey = NULL;
  if (!ikey || !(ikey->get_key()) || !(strlen(reinterpret_cast<const char *>
  (ctrlr)))) {
     UPLL_LOG_DEBUG("Input is NULL");
     return UPLL_RC_ERR_GENERIC;
  }
  if (ikey->get_key_type() != UNC_KT_VTN) {
    UPLL_LOG_DEBUG("Bad Request");
    return UPLL_RC_ERR_BAD_REQUEST;
  }
  ConfigVal *cfg_val = ikey->get_cfg_val();
  if (cfg_val == NULL) {
    UPLL_LOG_DEBUG("Configval is NULL");
    return UPLL_RC_ERR_BAD_REQUEST;
  }
  val_rename_vtn_t *tval =  reinterpret_cast<val_rename_vtn_t *>
                            (cfg_val->get_val());
  if (!tval) {
    UPLL_LOG_DEBUG("Val is NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  key_vtn_t *key_vtn = reinterpret_cast<key_vtn_t *>
      (ConfigKeyVal::Malloc(sizeof(key_vtn_t)));
  /* No rename */
  if (tval->valid[UPLL_IDX_NEW_NAME_RVTN] == UNC_VF_VALID_NO_VALUE) {
    no_rename = true;
    uuu::upll_strncpy(key_vtn->vtn_name,
     ((reinterpret_cast<key_vtn_t*>(ikey->get_key()))->vtn_name),
     (kMaxLenVtnName + 1));
    UPLL_LOG_DEBUG("No Rename Operation %d", no_rename);
  } else {
    if ( tval->valid[UPLL_IDX_NEW_NAME_RVTN] == UNC_VF_VALID ) {
      //  checking the string is empty or not
      if (!strlen(reinterpret_cast<char *>(tval->new_name))) {
        ConfigKeyVal::Free(key_vtn);
        return UPLL_RC_ERR_GENERIC;
      }
      uuu::upll_strncpy(key_vtn->vtn_name, tval->new_name,
                        (kMaxLenVtnName + 1));
      //  copy the new UNC name to KeyVtn
      /* The New Name and PFC name should not be same name */
      if (!strcmp(reinterpret_cast<char *>
        ((reinterpret_cast<key_vtn_t *>(ikey->get_key()))->vtn_name),
         reinterpret_cast<char *>(tval->new_name))) {
        ConfigKeyVal::Free(key_vtn);
        return UPLL_RC_ERR_GENERIC;
      }
    } else {
      UPLL_LOG_DEBUG("Invalid Input");
      ConfigKeyVal::Free(key_vtn);
      return UPLL_RC_ERR_GENERIC;
    }
  }
  okey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key_vtn, NULL);
  return result_code;
}

bool VtnMoMgr::FilterAttributes(void *&val1,
                                void *val2,
                                bool copy_to_running,
                                unc_keytype_operation_t op) {
  val_vtn_t *val_vtn1 = reinterpret_cast<val_vtn_t *>(val1);
  val_vtn1->valid[UPLL_IDX_DESC_VTN] = UNC_VF_INVALID;
  // to be uncommented when vtn supports other attributes
  // other than description
#if 0
  val_vtn_ctrlr *ctrlr_val = reinterpret_cast<val_vtn_ctrlr_t *>(val2);
  for (unsigned int loop = 0;
       loop < sizeof(val_vtn1->valid)/sizeof(val_vtn1->valid[0]);
       ++loop) {
    if (ctrlr_val->cs_attr[loop] == UNC_CS_NOT_SUPPORTED)
      val_vtn1->valid[loop] = UNC_VF_INVALID;
  }
  if (op != UNC_OP_CREATE)
    return CompareValidValue(val1, val2, copy_to_running);
#endif
  return false;
}

bool VtnMoMgr::CompareValidValue(void *&val1, void *val2,
                                 bool copy_to_running) {
  UPLL_FUNC_TRACE;
  bool invalid_attr = true;
  val_vtn_t *val_vtn1 = reinterpret_cast<val_vtn_t *>(val1);
  val_vtn_t *val_vtn2 = reinterpret_cast<val_vtn_t *>(val2);
  val_vtn_index vtn_description = UPLL_IDX_DESC_VTN;
  if (UNC_VF_INVALID == val_vtn1->valid[vtn_description] &&
                        UNC_VF_VALID == val_vtn2->valid[vtn_description])
    val_vtn1->valid[vtn_description] = UNC_VF_VALID_NO_VALUE;
  if  (UNC_VF_INVALID != val_vtn1->valid[vtn_description]) {
    if (!copy_to_running ||
        ((UNC_VF_VALID == val_vtn1->valid[vtn_description]) &&
         (!strcmp(reinterpret_cast<char*>(val_vtn1->description),
               reinterpret_cast<char*>(val_vtn2->description)) )))
      val_vtn1->valid[vtn_description] = UNC_VF_INVALID;
  }
  /* filters the attributes from being sent to the controller */
  for (unsigned int loop = 0;
       loop < sizeof(val_vtn1->valid)/sizeof(val_vtn1->valid[0]);
       ++loop) {
    if ((UNC_VF_VALID == val_vtn1->valid[loop]) ||
        (UNC_VF_VALID_NO_VALUE == val_vtn1->valid[loop])) {
      invalid_attr = false;
      break;
    }
  }
  return invalid_attr;
}

upll_rc_t VtnMoMgr::UpdateCtrlrConfigStatus(
                            unc_keytype_configstatus_t cs_status,
                            uuc::UpdateCtrlrPhase phase,
                            ConfigKeyVal *&ckv_running) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  val_vtn_ctrlr_t *val;
  val = (ckv_running != NULL)?
        reinterpret_cast<val_vtn_ctrlr_t *>(GetVal(ckv_running)):NULL;
  if (NULL == val) {
    return UPLL_RC_ERR_GENERIC;
  }
  if (uuc::kUpllUcpCreate == phase )
  val->cs_row_status = cs_status;
  if ((uuc::kUpllUcpUpdate == phase) &&
           (val->cs_row_status == UNC_CS_INVALID ||
            val->cs_row_status == UNC_CS_NOT_APPLIED))
    val->cs_row_status = cs_status;
  if ((cs_status == UNC_CS_INVALID &&
       UNC_VF_VALID == val->valid[UPLL_IDX_DESC_VTN]) ||
       cs_status == UNC_CS_APPLIED)
    val->cs_attr[UPLL_IDX_DESC_VTN] = cs_status;
  return result_code;
}

upll_rc_t
VtnMoMgr::UpdateAuditConfigStatus(unc_keytype_configstatus_t cs_status,
                                  uuc::UpdateCtrlrPhase phase,
                                  ConfigKeyVal *&ckv_running,
                                  DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  val_vtn_t *val;
  val = (ckv_running != NULL)?
        reinterpret_cast<val_vtn_t *>(GetVal(ckv_running)):NULL;
  if (NULL == val) {
    return UPLL_RC_ERR_GENERIC;
  }
  if (uuc::kUpllUcpCreate == phase )
  val->cs_row_status = cs_status;
  for ( unsigned int loop = 0;
           loop < sizeof(val->valid)/sizeof(uint8_t); ++loop ) {
    if ((cs_status == UNC_CS_INVALID && UNC_VF_VALID == val->valid[loop]) ||
        cs_status == UNC_CS_APPLIED)
    val->cs_attr[loop] = cs_status;
  }
  return result_code;
}

upll_rc_t
VtnMoMgr::SetVtnConsolidatedStatus(ConfigKeyVal *ikey,
                                   uint8_t *ctrlr_id,
                                   DalDmlIntf *dmi)  {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *ctrlr_ckv = NULL;
  val_vtn_ctrlr *ctrlr_val = NULL;
  uint8_t *vtn_exist_on_ctrlr = NULL;
  bool applied = false, not_applied = false, invalid = false;
  unc_keytype_configstatus_t c_status = UNC_CS_NOT_APPLIED;
  string vtn_id = "";
  DbSubOp dbop = { kOpReadMultiple, kOpMatchNone,
                   kOpInOutCtrlr | kOpInOutDomain | kOpInOutCs };
  if (!ikey || !dmi || !ctrlr_id) {
    UPLL_LOG_DEBUG("Invalid Input");
    return UPLL_RC_ERR_GENERIC;
  }
  result_code = GetChildConfigKey(ctrlr_ckv, ikey);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetChildConfigKey failed err code %d", result_code);
    return result_code;
  }
  result_code = ReadConfigDB(ctrlr_ckv, UPLL_DT_RUNNING, UNC_OP_READ, dbop, dmi,
                             CTRLRTBL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("ReadConfigDB from ctrltbl failed err code %d",
                   result_code);
    delete ctrlr_ckv;
    return result_code;
  }

  for (ConfigKeyVal *tmp = ctrlr_ckv; tmp != NULL;
                     tmp = tmp->get_next_cfg_key_val()) {
    ctrlr_val = reinterpret_cast<val_vtn_ctrlr *>(GetVal(tmp));
    if (!ctrlr_val) {
      UPLL_LOG_DEBUG("Controller Value is empty");
      tmp = NULL;
      delete ctrlr_ckv;
      return UPLL_RC_ERR_GENERIC;
    }
    GET_USER_DATA_CTRLR(tmp, vtn_exist_on_ctrlr);
    if (!strcmp(reinterpret_cast<char *>(vtn_exist_on_ctrlr),
                reinterpret_cast<char *>(ctrlr_id)))
      continue;  // skipping entry of deleted controller

    switch (ctrlr_val->cs_row_status) {
      case UNC_CS_APPLIED:
        applied = true;
      break;
      case UNC_CS_NOT_APPLIED:
      case UNC_CS_NOT_SUPPORTED:
        not_applied = true;
      break;
      case UNC_CS_INVALID:
        invalid = true;
      break;
      default:
        UPLL_LOG_DEBUG("Invalid status");
    }
    vtn_exist_on_ctrlr = NULL;
  }
  if (invalid)
    c_status = UNC_CS_INVALID;
  else if (applied && !not_applied)
    c_status = UNC_CS_APPLIED;
  else if (!applied && not_applied)
    c_status = UNC_CS_NOT_APPLIED;
  else if (applied && not_applied)
    c_status = UNC_CS_PARTIALLY_APPLIED;
  else
    c_status = UNC_CS_APPLIED;
  // Set cs_status
  val_vtn_t *vtnval = static_cast<val_vtn_t *>(GetVal(ikey));
  vtnval->cs_row_status = c_status;
  vtnval->cs_attr[0] = UNC_CS_APPLIED;
  // initialize the main table vtn oper status to be recomputed.
  val_db_vtn_st *val_vtnst = reinterpret_cast<val_db_vtn_st *>
                             (GetStateVal(ikey));
  if (!val_vtnst) {
    UPLL_LOG_DEBUG("Returning error %d\n", UPLL_RC_ERR_GENERIC);
    return UPLL_RC_ERR_GENERIC;
  }
  DbSubOp dbop_update = {kOpNotRead, kOpMatchNone, kOpInOutCs};
  result_code = UpdateConfigDB(ikey, UPLL_DT_STATE, UNC_OP_UPDATE, dmi,
                               &dbop_update, TC_CONFIG_GLOBAL, vtn_id,
                               MAINTBL);
  delete ctrlr_ckv;
  return result_code;
}

upll_rc_t
VtnMoMgr::SetConsolidatedStatus(ConfigKeyVal * ikey,
                                DalDmlIntf * dmi)  {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *ckv = NULL;
  string vtn_name = "";
  DbSubOp dbop = { kOpReadMultiple, kOpMatchNone, kOpInOutCs };
  result_code = GetChildConfigKey(ckv, ikey);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_INFO("GetChildConfigKey failed err code %d", result_code);
    return result_code;
  }
  result_code = ReadConfigDB(ckv, UPLL_DT_RUNNING, UNC_OP_READ, dbop, dmi,
                             CTRLRTBL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_INFO("ReadConfigDB from ctrltbl failed err code %d",
                   result_code);
    delete(ckv);
    return result_code;
  }
  list < unc_keytype_configstatus_t > list_cs_row;
  val_vtn_ctrlr *val;
  ConfigKeyVal *tmp = ckv;
  for (; tmp != NULL; tmp = tmp->get_next_cfg_key_val()) {
    val = reinterpret_cast<val_vtn_ctrlr *>(GetVal(tmp));
    list_cs_row.push_back(
        static_cast<unc_keytype_configstatus_t>(val->cs_row_status));
  }
  DELETE_IF_NOT_NULL(ckv);
  val_vtn_t *val_temp = reinterpret_cast<val_vtn_t *>(GetVal(ikey));
  val_temp->cs_row_status = GetConsolidatedCsStatus(list_cs_row);
  result_code = UpdateConfigDB(ikey, UPLL_DT_RUNNING, UNC_OP_UPDATE, dmi,
                               TC_CONFIG_GLOBAL, vtn_name, MAINTBL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("SetConsolidatedStatus failed. UpdateConfigDB Failed"
                   " Result Code - %d", result_code);
  }
  return result_code;
}

upll_rc_t
VtnMoMgr::MergeValidateChildren(ConfigKeyVal *import_ckval,
                                const char *ctrlr_id,
                                ConfigKeyVal *ikey,
                                DalDmlIntf *dmi,
                                upll_import_type import_type) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  if (!import_ckval || !(import_ckval->get_key())) {
    UPLL_LOG_DEBUG("Invalid Input");
    return UPLL_RC_ERR_GENERIC;
  }
  ConfigKeyVal *ckval = NULL;
  unc_key_type_t child_key[]= {
                         UNC_KT_VBRIDGE, UNC_KT_VBR_NWMONITOR, UNC_KT_VROUTER,
                         UNC_KT_VRT_IF, UNC_KT_VLINK, UNC_KT_VTERMINAL,
                         UNC_KT_VBR_PORTMAP};
  while (import_ckval) {
    for (unsigned int i = 0;
             i < sizeof(child_key)/sizeof(child_key[0]); i++) {
      const unc_key_type_t ktype = child_key[i];
      MoMgrImpl *mgr = reinterpret_cast<MoMgrImpl *>(
          const_cast<MoManager *>(GetMoManager(ktype)));
      if (!mgr) {
        UPLL_LOG_DEBUG("Instance is NULL");
        return UPLL_RC_ERR_GENERIC;
      }
      result_code = mgr->GetChildConfigKey(ckval, import_ckval);
      if (UPLL_RC_SUCCESS != result_code) {
        if (ckval) {
          delete ckval;
          ckval = NULL;
        }
        UPLL_LOG_DEBUG("GetChildConfigKey Failed %d", result_code);
        return result_code;
      }
      result_code = mgr->MergeValidate(child_key[i], ctrlr_id, ckval,
                                       dmi, import_type);
      UPLL_LOG_TRACE("Result code is %d key type %d", result_code, ktype);

      if (UPLL_RC_SUCCESS != result_code &&
          UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
        if (UPLL_RC_ERR_MERGE_CONFLICT == result_code) {
            UPLL_LOG_DEBUG(" Merge Conflict %d", result_code);
            if (ikey) {
              ikey->ResetWith(ckval);
              UPLL_LOG_DEBUG("Conflict detail %s", ikey->ToStrAll().c_str());
            }
        } else {
          UPLL_LOG_DEBUG("Merge Validate Failed %d", result_code);
        }
        if (NULL != ckval) {
          delete ckval;
        ckval = NULL;
        }
        return result_code;
      }
      if (ckval)
        delete ckval;
      ckval = NULL;
    }
    import_ckval = import_ckval->get_next_cfg_key_val();
  }
  result_code = (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) ?
                 UPLL_RC_SUCCESS : result_code;
  if (UPLL_RC_SUCCESS == result_code &&
      UPLL_IMPORT_TYPE_PARTIAL == import_type) {
    result_code = PartialMergeValidation(ctrlr_id, ikey,
                                         dmi, import_type);
    result_code = (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code)?
                   UPLL_RC_SUCCESS:result_code;
  }
  return result_code;
}

upll_rc_t
VtnMoMgr::PartialMergeValidation(const char *ctrlr_id,
                                 ConfigKeyVal *ikey,
                                 DalDmlIntf *dmi,
                                 upll_import_type import_type) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code  = UPLL_RC_SUCCESS;
  if (UPLL_IMPORT_TYPE_PARTIAL == import_type) {
    //  During partial import merge validate whether gateway_port
    //  configuration in running is modified/removed in imported configuration
    ConfigKeyVal vtn_gw_conflict_ckv(UNC_KT_VTN);
    result_code = PartialMergeValidate(UNC_KT_VTN, ctrlr_id,
                                       &vtn_gw_conflict_ckv, dmi);
    if (UPLL_RC_SUCCESS != result_code &&
        UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
      ikey->ResetWith(&vtn_gw_conflict_ckv);
      return result_code;
    }

    unc_key_type_t child_key[]= {
      UNC_KT_VBRIDGE, UNC_KT_VBR_IF, UNC_KT_VBR_VLANMAP, UNC_KT_VBR_PORTMAP
    };
    for (unsigned int i = 0;
         i < sizeof(child_key)/sizeof(child_key[0]); i++) {
      const unc_key_type_t ktype = child_key[i];
      MoMgrImpl *mgr = reinterpret_cast<MoMgrImpl *>(
          const_cast<MoManager *>(GetMoManager(ktype)));
      if (!mgr) {
        UPLL_LOG_DEBUG("Instance is NULL");
        return UPLL_RC_ERR_GENERIC;
      }
      ConfigKeyVal conflict_ckv(ktype);
      result_code = mgr->PartialMergeValidate(ktype, ctrlr_id,
                                              &conflict_ckv, dmi);
      if (UPLL_RC_SUCCESS != result_code &&
          UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
        ikey->ResetWith(&conflict_ckv);
        return result_code;
      }
      result_code =
          (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) ? UPLL_RC_SUCCESS:
          result_code;
    }
  } else {
    UPLL_LOG_DEBUG("Invalid import type");
    return UPLL_RC_ERR_GENERIC;
  }
  return result_code;
}

upll_rc_t
VtnMoMgr::MergeValidate(unc_key_type_t keytype,
                        const char *ctrlr_id,
                        ConfigKeyVal *ikey,
                        DalDmlIntf *dmi,
                        upll_import_type import_type) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_ERR_GENERIC;
  string vtn_name = "";
  // Import Controller Name Not needed so checks is missing.
  ConfigKeyVal *import_ckval = NULL;
  result_code = GetChildConfigKey(import_ckval, NULL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetChildConfig Failed ");
    return result_code;
  }
  DbSubOp dbop = {kOpReadMultiple, kOpMatchNone, kOpInOutNone};
  result_code = ReadConfigDB(import_ckval, UPLL_DT_IMPORT, UNC_OP_READ,
                dbop, dmi, MAINTBL);
  if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
    UPLL_LOG_DEBUG("No Records in the Import DB");
    DELETE_IF_NOT_NULL(import_ckval);
    if (UPLL_IMPORT_TYPE_PARTIAL == import_type) {
      result_code = PartialMergeValidation(ctrlr_id, ikey,
                                         dmi, import_type);
    }
    result_code = (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code)?UPLL_RC_SUCCESS:
                  result_code;
    return result_code;
  }

  /* Other than  UPLL_RC_ERR_NO_SUCH_INSTANCE AND UPLL_RC_SUCCESS */
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG(" ReadConfigDB Failed %d", result_code);
    DELETE_IF_NOT_NULL(import_ckval);
    return result_code;
  }
  ConfigKeyVal *start_ckv = import_ckval;
  result_code = MergeValidateChildren(import_ckval, ctrlr_id, ikey,
                                      dmi, import_type);
  DELETE_IF_NOT_NULL(start_ckv);
  if (UPLL_RC_SUCCESS == result_code) {
    ConfigKeyVal *req = NULL;
    ConfigKeyVal *nreq = NULL;
    DalCursor *dal_cursor_handle  = NULL;
    UPLL_LOG_TRACE("Create Entry in candidate");
    result_code = DiffConfigDB(UPLL_DT_IMPORT, UPLL_DT_CANDIDATE,
                  UNC_OP_CREATE, req, nreq, &dal_cursor_handle, dmi,
                  TC_CONFIG_GLOBAL, vtn_name, MAINTBL);
    while (UPLL_RC_SUCCESS == result_code) {
      result_code = DalToUpllResCode(dmi->GetNextRecord(dal_cursor_handle));
      if (UPLL_RC_SUCCESS != result_code
          && UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
         UPLL_LOG_DEBUG("GetNextRecord Failed ");
         DELETE_IF_NOT_NULL(req);
         DELETE_IF_NOT_NULL(nreq);
         if (dal_cursor_handle)
           dmi->CloseCursor(dal_cursor_handle, true);
         return result_code;
      }
      if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
        DELETE_IF_NOT_NULL(req);
        DELETE_IF_NOT_NULL(nreq);
        if (dal_cursor_handle)
          dmi->CloseCursor(dal_cursor_handle, true);
        return UPLL_RC_SUCCESS;
      }
      dbop.inoutop = kOpInOutFlag | kOpInOutCtrlr | kOpInOutDomain;
      result_code = UpdateConfigDB(req, UPLL_DT_CANDIDATE, UNC_OP_CREATE,
                                    dmi, &dbop, TC_CONFIG_GLOBAL, vtn_name,
                                    MAINTBL);
      if (UPLL_RC_SUCCESS != result_code) {
       DELETE_IF_NOT_NULL(req);
       DELETE_IF_NOT_NULL(nreq);
       if (dal_cursor_handle)
         dmi->CloseCursor(dal_cursor_handle, true);
       UPLL_LOG_DEBUG("UpdateConfigDB Failed");
       return result_code;
      }
    }
    DELETE_IF_NOT_NULL(req);
    DELETE_IF_NOT_NULL(nreq);
    if (dal_cursor_handle)
      dmi->CloseCursor(dal_cursor_handle, true);
  }
  result_code = (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code)?
      UPLL_RC_SUCCESS : result_code;
  return result_code;
}

upll_rc_t
VtnMoMgr::CopyToConfigKey(ConfigKeyVal * &okey,
                          ConfigKeyVal * ikey)  {
  if (!ikey || !(ikey->get_key())) return UPLL_RC_ERR_GENERIC;

  key_vtn_t *key_vtn = reinterpret_cast<key_vtn_t *>
                     (ConfigKeyVal::Malloc(sizeof(key_vtn_t)));

  key_rename_vnode_info *key_rename =
          reinterpret_cast<key_rename_vnode_info *>(ikey->get_key());

  if (!strlen(reinterpret_cast<char *>(key_rename->old_unc_vtn_name))) {
    ConfigKeyVal::Free(key_vtn);
    return UPLL_RC_ERR_GENERIC;
  }
  uuu::upll_strncpy(key_vtn->vtn_name, key_rename->old_unc_vtn_name,
                       (kMaxLenVtnName + 1));

  okey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key_vtn, NULL);
  SET_USER_DATA(okey, ikey);
  return UPLL_RC_SUCCESS;
}

/* This function will call doing the Rename Operation
 * This functions gets the Old Unc, New Unc and controller
 * names from the ikey, okey and store it in local structure
 * and creates the rename_info configkeyval
 */

upll_rc_t
VtnMoMgr:: GetRenameInfo(ConfigKeyVal *ikey,
                         ConfigKeyVal *okey,
                         ConfigKeyVal *&rename_info,
                         DalDmlIntf *dmi,
                         const char *ctrlr_id,
                         bool &renamed) {
  UPLL_FUNC_TRACE;
  uint8_t no_rename = false;
  string vtn_id = "";
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  if (!ikey || !okey || NULL != rename_info
      || !(ikey->get_key()) || !(okey->get_key())) {
      UPLL_LOG_DEBUG("Input is NULL");
      return UPLL_RC_ERR_GENERIC;
  }
  /* allocate memory for struct to store all the details */
  key_rename_vnode_info *vtn_rename_info =
      reinterpret_cast<key_rename_vnode_info *>
          (ConfigKeyVal::Malloc(sizeof(key_rename_vnode_info)));

  key_vtn_t *vtn_key = NULL;
  vtn_key = reinterpret_cast<key_vtn_t *>(ikey->get_key());
  if (vtn_key == NULL) {
    UPLL_LOG_DEBUG("No VTN Key");
    ConfigKeyVal::Free(vtn_rename_info);
    return UPLL_RC_ERR_GENERIC;
  }
  GET_USER_DATA_FLAGS(okey, no_rename);
  /* Checks the vtn is already renamed or not */
  if (renamed) {
    /* if already renamed store the controller name */
    if (!strlen(reinterpret_cast<char *>(
         (reinterpret_cast<val_rename_vtn_t *>(GetVal(ikey)))->new_name))) {
      ConfigKeyVal::Free(vtn_rename_info);
      return UPLL_RC_ERR_GENERIC;
    }
    uuu::upll_strncpy(vtn_rename_info->ctrlr_vtn_name,
         reinterpret_cast<val_rename_vtn_t *>(GetVal(ikey))->new_name,
         (kMaxLenVtnName + 1));
  } else {
    /* if not renamed the ikey contains the controller name */
    if (!strlen(reinterpret_cast<char *>(vtn_key->vtn_name))) {
      ConfigKeyVal::Free(vtn_rename_info);
      return UPLL_RC_ERR_GENERIC;
    }
    uuu::upll_strncpy(vtn_rename_info->ctrlr_vtn_name, vtn_key->vtn_name,
                      (kMaxLenVtnName + 1));
  }
  /* Store the old UNC VTN  name */
  if (!strlen(reinterpret_cast<char *>(vtn_key->vtn_name))) {
    ConfigKeyVal::Free(vtn_rename_info);
    return UPLL_RC_ERR_GENERIC;
  }
  uuu::upll_strncpy(vtn_rename_info->old_unc_vtn_name, vtn_key->vtn_name,
                      (kMaxLenVtnName + 1));

  vtn_key = reinterpret_cast<key_vtn_t *>(okey->get_key());
  /* store the new UNC VTN NAME */
  if (!strlen(reinterpret_cast<char *>(vtn_key->vtn_name))) {
    ConfigKeyVal::Free(vtn_rename_info);
    return UPLL_RC_ERR_GENERIC;
  }
  uuu::upll_strncpy(vtn_rename_info->new_unc_vtn_name, vtn_key->vtn_name,
                      (kMaxLenVtnName + 1));

  rename_info = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcInvalidStNum,
           vtn_rename_info, NULL);
  DbSubOp dbop = {kOpReadMultiple, kOpMatchNone,
    kOpInOutCtrlr | kOpInOutDomain | kOpInOutFlag};
  // ikey has rename val set, so removing  that to read from ctrlr tbl
  ikey->SetCfgVal(NULL);
  result_code = ReadConfigDB(ikey, UPLL_DT_IMPORT,
                            UNC_OP_READ, dbop, dmi, CTRLRTBL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("ReadConfigDB Error");
    return result_code;
  }
  SET_USER_DATA(rename_info, ikey);
  if (!rename_info) {
    ConfigKeyVal::Free(vtn_rename_info);
    return UPLL_RC_ERR_GENERIC;
  }
  /* Vtn Merge with Existing VTN name in
   * IMPORT Table
   */
  ConfigKeyVal *temp_key = NULL;
  result_code = GetChildConfigKey(temp_key, okey);
  if (UPLL_RC_SUCCESS != result_code) {
     UPLL_LOG_DEBUG("GetChildConfigKey Failed");
     return result_code;
  }
  result_code = UpdateConfigDB(temp_key, UPLL_DT_IMPORT, UNC_OP_READ, dmi,
                               &dbop, MAINTBL);
  if (UPLL_RC_ERR_INSTANCE_EXISTS != result_code &&
      UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
    UPLL_LOG_DEBUG("UpdateConfigDB failed %d", result_code);
    DELETE_IF_NOT_NULL(temp_key);
    return result_code;
  }
  if (UPLL_RC_ERR_INSTANCE_EXISTS == result_code) {
    /* Invoked VtnRenameMerge function during vtn renamed to existing
     * vtn name for validation/Merge of POM key type from renamed VTN
     */
    result_code = VtnRenameMerge(ikey, okey, dmi);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("RenameMerge failed %d", result_code);
      DELETE_IF_NOT_NULL(temp_key);
      return result_code;
    }

    if (!no_rename) {
      /* Semantic check validation for the following keytypes
       * VBRIDGE/VBR_NWMONITOR/VRT_IF during vtn stiching*/
      unc_key_type_t child_key[]= {
        UNC_KT_VBRIDGE, UNC_KT_VBR_NWMONITOR, UNC_KT_VRT_IF, UNC_KT_VBR_PORTMAP
      };
      for (unsigned int i = 0;
           i < sizeof(child_key)/sizeof(child_key[0]); i++) {
        const unc_key_type_t ktype = child_key[i];
        MoMgrImpl *mgr = reinterpret_cast<MoMgrImpl *>(
            const_cast<MoManager *>(GetMoManager(ktype)));
        if (!mgr) {
          UPLL_LOG_DEBUG("Instance is NULL");
          DELETE_IF_NOT_NULL(temp_key);
          return UPLL_RC_ERR_GENERIC;
        }
        result_code = mgr->ValidateVtnRename(ikey, temp_key, dmi);
        if (UPLL_RC_SUCCESS != result_code) {
          UPLL_LOG_TRACE("Vtn Stiching is failed. Result code: %d. key type %d",
                         result_code, ktype);
          DELETE_IF_NOT_NULL(temp_key);
          return result_code;
        }
      }
    }
    SET_USER_DATA_FLAGS(temp_key, VTN_RENAME);
    /* New Name available in the IMPORT, then
     * we have to update the rename flag in Main Table */
    result_code = UpdateConfigDB(temp_key, UPLL_DT_IMPORT,
                                 UNC_OP_UPDATE, dmi, &dbop, TC_CONFIG_GLOBAL,
                                 vtn_id, MAINTBL);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("UpdateConfigDB failed %d", result_code);
      DELETE_IF_NOT_NULL(temp_key);
      return result_code;
    }
    DELETE_IF_NOT_NULL(temp_key);

    result_code = GetChildConfigKey(temp_key, ikey);
    if (UPLL_RC_SUCCESS != result_code) {
     UPLL_LOG_DEBUG("GetChildConfigKey Failed");
     return result_code;
  }
    /* Remove the Current VTN name from the Main table*/
    result_code = UpdateConfigDB(temp_key, UPLL_DT_IMPORT,
                                UNC_OP_DELETE, dmi, &dbop, TC_CONFIG_GLOBAL,
                                vtn_id, MAINTBL);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("UpdateConfigDB failed %d", result_code);
      DELETE_IF_NOT_NULL(temp_key);
      return result_code;
    }
  }
  /* The new name not available then create an entry in main table */
  if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code && !no_rename) {
    result_code =  GetChildConfigKey(temp_key, okey);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("GetChildConfigKey Failed %d", result_code);
      return result_code;
    }
    SET_USER_DATA_FLAGS(temp_key, VTN_RENAME);
     /*Create an entry in main table */
    result_code = UpdateConfigDB(temp_key, UPLL_DT_IMPORT,
                                 UNC_OP_CREATE, dmi, &dbop, TC_CONFIG_GLOBAL,
                                 vtn_id, MAINTBL);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("UpdateConfigDB failed %d", result_code);
      DELETE_IF_NOT_NULL(temp_key);
      return result_code;
    }
    DELETE_IF_NOT_NULL(temp_key);
    result_code =  GetChildConfigKey(temp_key, ikey);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("GetChildConfigKey Failed %d", result_code);
      return result_code;
    }
    /* Remove the current name from the main table */
    result_code = UpdateConfigDB(temp_key, UPLL_DT_IMPORT,
                                UNC_OP_DELETE, dmi, &dbop, TC_CONFIG_GLOBAL,
                                vtn_id, MAINTBL);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("UpdateConfigDB failed %d", result_code);
      DELETE_IF_NOT_NULL(temp_key);
      return result_code;
    }
  }
  DELETE_IF_NOT_NULL(temp_key);
  if (no_rename) {
    /* This is called during no rename function */
    UPLL_LOG_TRACE("Calling No Rename");
    UPLL_LOG_TRACE("Ikey is %s", ikey->ToStrAll().c_str());
    UPLL_LOG_TRACE("Okey is %s", okey->ToStrAll().c_str());
    uint32_t ref_count = 0;
    SET_USER_DATA_FLAGS(okey, NO_RENAME);
    result_code = GetChildConfigKey(temp_key, okey);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("GetChildConfigKey Failed %d", result_code);
      return result_code;
    }
    /* Create an entry with old name */
    result_code = UpdateConfigDB(temp_key, UPLL_DT_IMPORT, UNC_OP_CREATE, dmi,
                                 TC_CONFIG_GLOBAL,
                                 vtn_id, MAINTBL);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("UpdateConfigDB failed %d", result_code);
      DELETE_IF_NOT_NULL(temp_key);
      return result_code;
    }
    DELETE_IF_NOT_NULL(temp_key);
    result_code = GetChildConfigKey(temp_key, ikey);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("GetChildConfigKey Failed %d", result_code);
      return result_code;
    }
    /* Check the count for the Renamed UNC name */
    result_code = GetInstanceCount(temp_key, const_cast<char *>(ctrlr_id),
                                   UPLL_DT_IMPORT, &ref_count, dmi, RENAMETBL);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("GetInstanceCoutn is Failed %d", result_code);
      return result_code;
    }
    if (ref_count == 1) {
      temp_key->SetCfgVal(NULL);
      /* if the count is one then remove the Renamed UNC name
       * from the MAIN TABLE
       */
      SET_USER_DATA_FLAGS(temp_key, NO_RENAME);
      result_code = UpdateConfigDB(temp_key, UPLL_DT_IMPORT, UNC_OP_DELETE, dmi,
                                  TC_CONFIG_GLOBAL,
                                  vtn_id, MAINTBL);
      if (UPLL_RC_SUCCESS != result_code) {
        UPLL_LOG_DEBUG("UpdateConfigDB failed %d", result_code);
        DELETE_IF_NOT_NULL(temp_key);
        return result_code;
      }
    }
  }
  DELETE_IF_NOT_NULL(temp_key);
  /* Set the Controller Id*/
  SET_USER_DATA_CTRLR(rename_info, ctrlr_id);
  if (!renamed) {
      val_rename_vtn_t *vtn = reinterpret_cast<val_rename_vtn_t *>
          (ConfigKeyVal::Malloc(sizeof(val_rename_vtn)));
      uuu::upll_strncpy(vtn->new_name, vtn_rename_info->ctrlr_vtn_name,
                      (kMaxLenCtrlrId + 1));
      ConfigVal *cfg_val = new ConfigVal(IpctSt::kIpcStValRenameVtn, vtn);
      vtn->valid[UPLL_IDX_NEW_NAME_RVTN] = UNC_VF_VALID;
      okey->SetCfgVal(cfg_val);
      SET_USER_DATA(okey, ikey);
      dbop.readop = kOpNotRead;
      result_code = UpdateConfigDB(okey, UPLL_DT_IMPORT, UNC_OP_CREATE,
                                   dmi, &dbop, TC_CONFIG_GLOBAL,
                                   vtn_id, RENAMETBL);
  }
  return result_code;
}

upll_rc_t
VtnMoMgr::ValidateVtnKey(key_vtn * vtn_key)  {
  UPLL_FUNC_TRACE;
  upll_rc_t ret_val = UPLL_RC_SUCCESS;
  ret_val = ValidateKey(reinterpret_cast<char *>(vtn_key->vtn_name),
                         kMinLenVtnName, kMaxLenVtnName);
  if (ret_val != UPLL_RC_SUCCESS) {
    UPLL_LOG_INFO("vtn name syntax check failed."
                  "Received vtn name - %s",
                  vtn_key->vtn_name);
    return UPLL_RC_ERR_CFG_SYNTAX;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t
VtnMoMgr::ValidateVtnValue(val_vtn *vtn_val,
                           uint32_t operation) {
  UPLL_FUNC_TRACE;
  if (vtn_val->valid[UPLL_IDX_DESC_VTN] == UNC_VF_VALID) {
    if (!ValidateDesc(vtn_val->description,
        kMinLenDescription, kMaxLenDescription)) {
      UPLL_LOG_INFO("Description syntax check failed."
          "Received Description - %s", vtn_val->description);
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  } else if (vtn_val->valid[UPLL_IDX_DESC_VTN] == UNC_VF_VALID_NO_VALUE &&
      (operation == UNC_OP_UPDATE || operation == UNC_OP_CREATE)) {
    uuu::upll_strncpy(reinterpret_cast<char *>(vtn_val->description), " ", 2);
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t
VtnMoMgr::ValidateVtnRenameValue(val_rename_vtn * vtn_rename) {
  UPLL_FUNC_TRACE;
  upll_rc_t ret_val = UPLL_RC_SUCCESS;
  if (vtn_rename->valid[UPLL_IDX_NEW_NAME_RVTN] == UNC_VF_VALID) {
    ret_val = ValidateKey(reinterpret_cast<char *>(vtn_rename->new_name),
                          kMinLenVtnName, kMaxLenVtnName);
    if (ret_val != UPLL_RC_SUCCESS) {
      UPLL_LOG_INFO("Rename syntax check failed."
                    "Received  vtn_rename - %s",
                    vtn_rename->new_name);
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t
VtnMoMgr::ValidateMessage(IpcReqRespHeader * req,
                          ConfigKeyVal * ikey)  {
  UPLL_FUNC_TRACE;
  upll_rc_t ret_val = UPLL_RC_SUCCESS;
  if (!ikey || !req || !(ikey->get_key())) {
    UPLL_LOG_INFO("ConfigKeyVal / IpcReqRespHeader is Null");
    return UPLL_RC_ERR_GENERIC;
  }
  if (ikey->get_key_type() == UNC_KT_VTNSTATION_CONTROLLER)
    return (ValidateMessageForVtnStnCtrlr(req, ikey));

  if (ikey->get_key_type() == UNC_KT_VTN_MAPPING_CONTROLLER)
    return (ValidateMessageForVtnMapCtrlr(req, ikey));

  if (ikey->get_st_num() != IpctSt::kIpcStKeyVtn) {
    UPLL_LOG_INFO("Invalid Key structure received. received struct - %d",
                  (ikey->get_st_num()));
    return UPLL_RC_ERR_BAD_REQUEST;
  }
  key_vtn *vtn_key = reinterpret_cast<key_vtn *>(ikey->get_key());

  unc_key_type_t ktype = ikey->get_key_type();
  if (UNC_KT_VTN != ktype) {
    UPLL_LOG_INFO("Invalid keytype received. received keytype - %d", ktype);
    return UPLL_RC_ERR_BAD_REQUEST;
  }
  upll_keytype_datatype_t dt_type = req->datatype;
  unc_keytype_operation_t operation = req->operation;
  unc_keytype_option1_t option1 = req->option1;
  unc_keytype_option2_t option2 = req->option2;
  if ((operation != UNC_OP_READ_SIBLING_COUNT) &&
      (operation != UNC_OP_READ_SIBLING_BEGIN)) {
    ret_val = ValidateVtnKey(vtn_key);
    if (ret_val != UPLL_RC_SUCCESS) {
      UPLL_LOG_INFO("syntax check failed for key_vtn struct");
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  } else {
    UPLL_LOG_TRACE("Operation is %d", operation);
    StringReset(vtn_key->vtn_name);
  }
  if ((operation == UNC_OP_CREATE || operation == UNC_OP_UPDATE) &&
     (dt_type == UPLL_DT_CANDIDATE|| UPLL_DT_IMPORT == dt_type)) {
      ConfigVal *cfg_val = ikey->get_cfg_val();
      switch (operation) {
      case UNC_OP_CREATE:
        if (cfg_val == NULL)
          return UPLL_RC_SUCCESS;
        /* fall through intended */
      case UNC_OP_UPDATE:
      {
        if (!cfg_val)
          return UPLL_RC_ERR_CFG_SYNTAX;
        if (cfg_val->get_st_num() != IpctSt::kIpcStValVtn) {
          UPLL_LOG_INFO(
            "Invalid Value structure received. received struct - %d",
            (ikey->get_st_num()));
          return UPLL_RC_ERR_BAD_REQUEST;
        }
        val_vtn *vtn_val = reinterpret_cast<val_vtn *>(GetVal(ikey));
        if (vtn_val == NULL) {
          UPLL_LOG_INFO("syntax check for vtn_val struct is an optional");
          return UPLL_RC_SUCCESS;
        }
        ret_val = ValidateVtnValue(vtn_val, operation);
        if (ret_val != UPLL_RC_SUCCESS) {
          UPLL_LOG_INFO("syntax check failure for val_vtn structure");
          return UPLL_RC_ERR_CFG_SYNTAX;
        }
        return UPLL_RC_SUCCESS;
      }
      default:
          UPLL_LOG_INFO("Invalid operation ");
          return UPLL_RC_ERR_CFG_SYNTAX;
      }
  } else if ((operation == UNC_OP_RENAME || operation == UNC_OP_READ ||
       operation == UNC_OP_READ_SIBLING ||
       operation == UNC_OP_READ_SIBLING_BEGIN) &&
       (dt_type == UPLL_DT_IMPORT)) {
      if (option1 != UNC_OPT1_NORMAL) {
        UPLL_LOG_INFO("Error: option1 is not NORMAL");
        return UPLL_RC_ERR_INVALID_OPTION1;
      }
      if (option2 != UNC_OPT2_NONE) {
        UPLL_LOG_INFO("Error: option2 is not NONE");
        return UPLL_RC_ERR_INVALID_OPTION2;
      }
      ConfigVal *cfg_val = ikey->get_cfg_val();
      switch (operation) {
      case UNC_OP_READ:
      case UNC_OP_READ_SIBLING:
      case UNC_OP_READ_SIBLING_BEGIN:
        if (cfg_val == NULL)
          return UPLL_RC_SUCCESS;
      case UNC_OP_RENAME:
      {
        if (!cfg_val)
          return UPLL_RC_ERR_CFG_SYNTAX;
        if (cfg_val->get_st_num() != IpctSt::kIpcStValRenameVtn) {
          UPLL_LOG_INFO(
            "Invalid val_rename structure received. received struct - %d",
            (ikey->get_cfg_val())->get_st_num());
          return UPLL_RC_ERR_BAD_REQUEST;
        }
        val_rename_vtn *vtn_rename =
           reinterpret_cast<val_rename_vtn *>(ikey->get_cfg_val()->get_val());

        if (vtn_rename == NULL && operation == UNC_OP_RENAME) {
          UPLL_LOG_INFO(
            "val_rename_vtn struct is Mandatory for Rename operation");
          return UPLL_RC_ERR_BAD_REQUEST;
        } else if (vtn_rename == NULL) {
          UPLL_LOG_DEBUG(
            "syntax check for val_rename_vtn struct is optional");
          return UPLL_RC_SUCCESS;
        }
        ret_val = ValidateVtnRenameValue(vtn_rename);
        if (ret_val != UPLL_RC_SUCCESS) {
          UPLL_LOG_INFO("syntax check failure for val_rename_vtn structure");
          return UPLL_RC_ERR_CFG_SYNTAX;
        }
        return UPLL_RC_SUCCESS;
      }
      default:
          UPLL_LOG_INFO("Invalid operation ");
          return UPLL_RC_ERR_CFG_SYNTAX;
      }
  } else if ((operation == UNC_OP_READ || operation == UNC_OP_READ_SIBLING ||
         operation == UNC_OP_READ_SIBLING_BEGIN ||
         operation == UNC_OP_READ_SIBLING_COUNT) &&
         (dt_type == UPLL_DT_CANDIDATE || dt_type == UPLL_DT_RUNNING ||
          dt_type == UPLL_DT_STARTUP || dt_type == UPLL_DT_STATE)) {
      if (option1 != UNC_OPT1_NORMAL) {
        UPLL_LOG_INFO("Error: option1 is not NORMAL");
        return UPLL_RC_ERR_INVALID_OPTION1;
      }
      if (option2 != UNC_OPT2_NONE) {
        UPLL_LOG_INFO("Error: option2 is not NONE");
        return UPLL_RC_ERR_INVALID_OPTION2;
      }
      ConfigVal *cfg_val = ikey->get_cfg_val();
      if (cfg_val == NULL)
        return UPLL_RC_SUCCESS;
      if (cfg_val->get_st_num() != IpctSt::kIpcStValVtn) {
        UPLL_LOG_INFO(
            "Invalid Value structure received. received struct - %d",
            (cfg_val->get_st_num()));
        return UPLL_RC_ERR_BAD_REQUEST;
      }
      val_vtn *vtn_val = reinterpret_cast<val_vtn *>
                                 (ikey->get_cfg_val()->get_val());
      if (vtn_val == NULL) {
        UPLL_LOG_DEBUG("syntax check for vtn struct is an optional");
        return UPLL_RC_SUCCESS;
      }
      ret_val = ValidateVtnValue(vtn_val, operation);
      if (ret_val != UPLL_RC_SUCCESS) {
        UPLL_LOG_INFO("Base Validation failure for val_vtn  structure");
        return UPLL_RC_ERR_CFG_SYNTAX;
      }
      return UPLL_RC_SUCCESS;
  } else if ((operation == UNC_OP_READ_NEXT ||
             operation == UNC_OP_READ_BULK) && (dt_type == UPLL_DT_CANDIDATE
             || dt_type == UPLL_DT_STARTUP || dt_type == UPLL_DT_RUNNING ||
             dt_type == UPLL_DT_IMPORT)) {
      UPLL_LOG_TRACE("Value structure is none for operation type:%d",
                    operation);
      return UPLL_RC_SUCCESS;
  } else if ((operation == UNC_OP_DELETE) &&
             (dt_type == UPLL_DT_CANDIDATE || UPLL_DT_IMPORT == dt_type)) {
      UPLL_LOG_TRACE("Value structure is none for operation type:%d",
                    operation);
      return UPLL_RC_SUCCESS;
  }
  UPLL_LOG_INFO("Error Unsupported datatype(%d) or operation(%d)",
                dt_type, operation);
  return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT;
}

upll_rc_t
VtnMoMgr::ValVtnAttributeSupportCheck(val_vtn_t *vtn_val,
                                      const uint8_t *attrs,
                                      uint32_t operation) {
  UPLL_FUNC_TRACE;

  if (NULL != vtn_val) {
    if ((vtn_val->valid[UPLL_IDX_DESC_VTN] == UNC_VF_VALID) ||
       (vtn_val->valid[UPLL_IDX_DESC_VTN] == UNC_VF_VALID_NO_VALUE)) {
      if (attrs[unc::capa::vtn::kCapDesc] == 0) {
         vtn_val->valid[UPLL_IDX_DESC_VTN] = UNC_VF_INVALID;
        if (operation == UNC_OP_CREATE ||
            operation == UNC_OP_UPDATE) {
          UPLL_LOG_INFO("Description attr is not supported by ctrlr");
          return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
        }
      }
    }
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t
VtnMoMgr::ValidateCapability(IpcReqRespHeader * req,
                             ConfigKeyVal * ikey,
                             const char *ctrlr_name)  {
  UPLL_FUNC_TRACE;
  upll_rc_t ret_val = UPLL_RC_ERR_GENERIC;
  if (!ikey || !req) {
    UPLL_LOG_DEBUG("ConfigKeyVal / IpcReqRespHeader is Null");
    return ret_val;
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
  const uint8_t *attrs = NULL;
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
      UPLL_LOG_INFO("Invalid operation code(%d)", req->operation);
      return UPLL_RC_ERR_GENERIC;
  }
  if (!result_code) {
    UPLL_LOG_INFO("key_type - %d is not supported by controller - %s",
        ikey->get_key_type(), ctrlr_name);
    return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
  }
  val_vtn *vtn_val = NULL;
  if (ikey->get_cfg_val() && (ikey->get_cfg_val()->get_st_num() ==
       IpctSt::kIpcStValVtn)) {
    vtn_val =
          reinterpret_cast<val_vtn *>(ikey->get_cfg_val()->get_val());
  }
  if (vtn_val) {
    if (max_attrs > 0) {
      ret_val = ValVtnAttributeSupportCheck(vtn_val, attrs, req->operation);
      return ret_val;
    } else {
      UPLL_LOG_DEBUG("Attribute list is empty for operation %d",
                      req->operation);
      return UPLL_RC_ERR_GENERIC;
    }
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtnMoMgr::IsKeyInUse(upll_keytype_datatype_t dt_type,
                       const ConfigKeyVal *ckv,
                       bool *in_use,
                       DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  ConfigKeyVal *ck_ctrlr = NULL;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  DbSubOp dbop = {kOpReadExist, kOpMatchCtrlr, kOpInOutNone};
  key_ctr *ctr = reinterpret_cast<key_ctr *>(ckv->get_key());

  if (!ctr || !strlen(reinterpret_cast<char *>(ctr->controller_name))) {
    UPLL_LOG_DEBUG("Controller Name invalid");
    return UPLL_RC_ERR_GENERIC;
  }
  uint8_t *controllerName = reinterpret_cast<uint8_t *>(
                              new char[kMaxLenCtrlrId+1]);
  uuu::upll_strncpy(controllerName, ctr->controller_name,
                    (kMaxLenCtrlrId + 1));
  result_code = GetChildConfigKey(ck_ctrlr, NULL);
  if (!ck_ctrlr || result_code != UPLL_RC_SUCCESS) {
    delete[] controllerName;
    UPLL_LOG_DEBUG("Controller key allocation failed");
    return UPLL_RC_ERR_GENERIC;
  }
  SET_USER_DATA_CTRLR(ck_ctrlr, controllerName);

  // result_code = ReadConfigDB(ck_ctrlr, UPLL_DT_RUNNING,
    //   UNC_OP_READ, dbop, dmi, CTRLRTBL);
  result_code = UpdateConfigDB(ck_ctrlr, dt_type, UNC_OP_READ,
                                dmi, &dbop, CTRLRTBL);
  *in_use = (result_code == UPLL_RC_ERR_INSTANCE_EXISTS) ? true : false;
  delete ck_ctrlr;
  delete []controllerName;
  UPLL_LOG_DEBUG("Returning %d", result_code);
  return ((result_code == UPLL_RC_ERR_INSTANCE_EXISTS ||
           result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE)
           ? UPLL_RC_SUCCESS : result_code);
}

upll_rc_t
VtnMoMgr::ValidateMessageForVtnStnCtrlr(IpcReqRespHeader * req,
                                        ConfigKeyVal * ikey)  {
  UPLL_FUNC_TRACE;
  upll_rc_t ret_val = UPLL_RC_SUCCESS;
  if (ikey->get_st_num() != IpctSt::kIpcStKeyVtnstationController) {
    UPLL_LOG_INFO("Invalid Key structure received. received struct - %d",
                  (ikey->get_st_num()));
    return UPLL_RC_ERR_BAD_REQUEST;
  }
  if (req->option1 != UNC_OPT1_COUNT && (!(ikey->get_cfg_val()) ||
       IsAllInvalidAttributes(
       reinterpret_cast<val_vtnstation_controller_st *>(GetVal(ikey))))) {
    /* if val_vtnstation_controller_st is given and all the attributes are
     * invalid. val structure is mandatory for read normal/detail operation.
     * For count operation, val structure is optional */
    UPLL_LOG_INFO("value structure for vtn station is NULL");
    return UPLL_RC_ERR_CFG_SYNTAX;
  }
  if (req->option1 != UNC_OPT1_COUNT && (ikey->get_cfg_val()->get_st_num() !=
             IpctSt::kIpcStValVtnstationControllerSt)) {
    /* For count operation, val structure is optional */
    UPLL_LOG_INFO("Received mismatch value structure for vtn station - %d",
                    ikey->get_cfg_val()->get_st_num());
    return UPLL_RC_ERR_BAD_REQUEST;
  }
  if ((req->operation == UNC_OP_READ) ||
      (req->operation == UNC_OP_READ_SIBLING) ||
      (req->operation == UNC_OP_READ_SIBLING_BEGIN) ||
      (req->operation == UNC_OP_READ_SIBLING_COUNT)) {
    if (req->datatype == UPLL_DT_STATE) {
      if ((req->option1 != UNC_OPT1_NORMAL) &&
          (req->option1 != UNC_OPT1_DETAIL) &&
          (req->option1 != UNC_OPT1_COUNT)) {
        UPLL_LOG_INFO(" Error: option1 is invalid");
        return UPLL_RC_ERR_INVALID_OPTION1;
      }
      if (req->option2 != UNC_OPT2_NONE) {
        UPLL_LOG_INFO(" Error: option2 is not NONE");
        return UPLL_RC_ERR_INVALID_OPTION2;
      }
      key_vtnstation_controller *vtn_ctrlr_key =
         reinterpret_cast<key_vtnstation_controller *> (ikey->get_key());
      ret_val = ValidateVtnStnCtrlrKey(vtn_ctrlr_key, req->operation);
      if (ret_val != UPLL_RC_SUCCESS) {
        UPLL_LOG_INFO("vtn_stn_ctrlr_key syntax check failed.");
        return ret_val;
      }
    } else {
      UPLL_LOG_INFO("Error Unsupported datatype (%d)", req->datatype);
      return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT;
    }
  } else {
    UPLL_LOG_INFO("Error Unsupported Operation (%d)", req->operation);
    return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_KT;
  }
  return ret_val;
}

upll_rc_t
VtnMoMgr::ValidateMessageForVtnMapCtrlr(IpcReqRespHeader * req,
                                        ConfigKeyVal * ikey) {
  UPLL_FUNC_TRACE;
  upll_rc_t ret_val = UPLL_RC_SUCCESS;
  upll_keytype_datatype_t dt_type = req->datatype;
  unc_keytype_operation_t operation = req->operation;
  unc_keytype_option1_t option1 = req->option1;
  unc_keytype_option2_t option2 = req->option2;

  if (ikey->get_st_num() != IpctSt::kIpcStKeyVtnController) {
    UPLL_LOG_INFO("Invalid key structure received. received struct - %d",
                  (ikey->get_st_num()));
    return UPLL_RC_ERR_BAD_REQUEST;
  }
  if ((ikey->get_cfg_val())) {
    /* value structure is not mandatory */
    if (ikey->get_cfg_val()->get_st_num() !=
             IpctSt::kIpcStValVtnMappingControllerSt) {
      UPLL_LOG_INFO("Received mismatch value structure for vtn mapping - %d",
                    ikey->get_cfg_val()->get_st_num());
      return UPLL_RC_ERR_BAD_REQUEST;
    }
  }
  key_vtn_controller *vtn_ctrlr_key =
                  reinterpret_cast<key_vtn_controller *> (ikey->get_key());

  if (operation == UNC_OP_READ || operation == UNC_OP_READ_SIBLING
      || operation == UNC_OP_READ_SIBLING_BEGIN) {
    if (dt_type == UPLL_DT_STATE) {
      if (option1 != UNC_OPT1_NORMAL) {
        UPLL_LOG_INFO(" Error: option1 is not NORMAL");
        return UPLL_RC_ERR_INVALID_OPTION1;
      }
      if (option2 != UNC_OPT2_NONE) {
        UPLL_LOG_INFO(" Error: option2 is not NONE");
        return UPLL_RC_ERR_INVALID_OPTION2;
      }
      ret_val = ValidateVtnMapCtrlrKey(vtn_ctrlr_key, operation);
      if (ret_val != UPLL_RC_SUCCESS) {
        UPLL_LOG_INFO("syntax check for key_vtn_ctrlr struct is failed");
        return ret_val;
      }
      UPLL_LOG_TRACE("value struct validation is none for this keytype");
      return UPLL_RC_SUCCESS;
    } else {
      UPLL_LOG_INFO("Error Unsupported datatype (%d)", dt_type);
      return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT;
    }
  } else {
    UPLL_LOG_INFO("Error Unsupported Operation (%d)", req->operation);
    return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_KT;
  }
  return ret_val;
}

upll_rc_t
VtnMoMgr::ValidateVtnMapCtrlrKey(key_vtn_controller * vtn_ctrlr_key,
                                 unc_keytype_operation_t operation) {
  UPLL_FUNC_TRACE;

  if (!ValidateString(
      vtn_ctrlr_key->vtn_key.vtn_name,
      kMinLenVtnName, kMaxLenVtnName)) {
    UPLL_LOG_INFO("vtn name syntax check failed."
                  "Received vtn_Name - %s",
                  vtn_ctrlr_key->vtn_key.vtn_name);
    return UPLL_RC_ERR_CFG_SYNTAX;
  }
  if ((operation != UNC_OP_READ_SIBLING_COUNT) &&
     (operation != UNC_OP_READ_SIBLING_BEGIN)) {
    if (!ValidateString(
        vtn_ctrlr_key->controller_name,
        kMinLenCtrlrId, kMaxLenCtrlrId)) {
      UPLL_LOG_INFO("controller_name syntax check failed."
                    "Received controller_name - %s",
                    vtn_ctrlr_key->controller_name);
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
    if (!ValidateDefaultStr(vtn_ctrlr_key->domain_id,
        kMinLenDomainId, kMaxLenDomainId)) {
      UPLL_LOG_INFO("Domain_id syntax check failed."
                    "Received Domain_id - %s",
                    vtn_ctrlr_key->domain_id);
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  } else {
    UPLL_LOG_TRACE("Operation is %d", operation);
    StringReset(vtn_ctrlr_key->controller_name);
    StringReset(vtn_ctrlr_key->domain_id);
  }
  return UPLL_RC_SUCCESS;
}
upll_rc_t
VtnMoMgr::ValidateVtnStnCtrlrKey(key_vtnstation_controller * vtn_ctrlr_key,
                                 unc_keytype_operation_t operation)  {
  UPLL_FUNC_TRACE;
  if ((operation != UNC_OP_READ_SIBLING_COUNT) &&
     (operation != UNC_OP_READ_SIBLING_BEGIN)) {
    if (!ValidateString(vtn_ctrlr_key->controller_name,
        kMinLenCtrlrId, kMaxLenCtrlrId)) {
      UPLL_LOG_INFO("controller_name syntax check failed."
                  "Received controller_name - %s",
                  vtn_ctrlr_key->controller_name);
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  } else {
    UPLL_LOG_TRACE("Operation is %d", operation);
    StringReset(vtn_ctrlr_key->controller_name);
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t
VtnMoMgr:: VtnRenameMerge(ConfigKeyVal *ikey,
                                   ConfigKeyVal *okey,
                                   DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  string vtn_id = "";
  ConfigKeyVal *temp_key = NULL;
  ConfigKeyVal *tkey = NULL;
  if (!ikey || !okey
      || !(ikey->get_key()) || !(okey->get_key())) {
      UPLL_LOG_DEBUG("Input is NULL");
      return UPLL_RC_ERR_GENERIC;
  }
  DbSubOp dbop = {kOpReadMultiple, kOpMatchNone, kOpInOutNone};
  unc_key_type_t key[]= {UNC_KT_VTN_FLOWFILTER,
                         UNC_KT_VTN_FLOWFILTER_ENTRY,
                         UNC_KT_VTN_POLICINGMAP };
  /* Remove flow filter and
   * policing map from renaming VTN
   */
  for (unsigned int index = 0;
       index < sizeof(key)/sizeof(key[0]); index++) {
    const unc_key_type_t ktype = key[index];
    MoMgrImpl *mgr = reinterpret_cast<MoMgrImpl *>(
  const_cast<MoManager *>(GetMoManager(ktype)));
    if (!mgr) {
      UPLL_LOG_DEBUG("Instance is NULL");
      return UPLL_RC_ERR_GENERIC;
    }
    result_code = mgr->GetChildConfigKey(temp_key, ikey);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("GetChildConfigKey Failed");
      return result_code;
    }
    result_code = mgr->ReadConfigDB(temp_key, UPLL_DT_IMPORT, UNC_OP_READ,
                                    dbop, dmi, MAINTBL);
    if (UPLL_RC_SUCCESS != result_code) {
      if (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
        UPLL_LOG_DEBUG("ReadConfigDB fail (%d)", result_code);
        DELETE_IF_NOT_NULL(temp_key);
        return result_code;
      } else {
      DELETE_IF_NOT_NULL(temp_key);
      continue;
      }
    }
    ConfigKeyVal *start_ptr = temp_key;
    while (start_ptr) {
      if (ktype == UNC_KT_VTN_POLICINGMAP) {
        UPLL_LOG_DEBUG("Renamed VTN has policingmap :RenameMerge Failed");
        DELETE_IF_NOT_NULL(temp_key);
        return UPLL_RC_ERR_MERGE_CONFLICT;
      }
      result_code = mgr->GetChildConfigKey(tkey, start_ptr);
      if (UPLL_RC_SUCCESS != result_code) {
        UPLL_LOG_DEBUG("GetChildConfigKey Failed");
        DELETE_IF_NOT_NULL(temp_key);
        return result_code;
      }
      /* Remove the old VTN name matching entry from vtn
       * flowfilter or vtn policingmap Main table
       */
      result_code = mgr->UpdateConfigDB(tkey, UPLL_DT_IMPORT,
          UNC_OP_DELETE, dmi, &dbop, TC_CONFIG_GLOBAL,
          vtn_id, MAINTBL);
      if (UPLL_RC_SUCCESS != result_code) {
        UPLL_LOG_DEBUG("UpdateConfigDB failed %d", result_code);
        DELETE_IF_NOT_NULL(tkey);
        DELETE_IF_NOT_NULL(temp_key);
        return result_code;
      }
      DELETE_IF_NOT_NULL(tkey);
      start_ptr = start_ptr->get_next_cfg_key_val();
    }
    DELETE_IF_NOT_NULL(temp_key);
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtnMoMgr::GetOperation(uuc::UpdateCtrlrPhase phase,
                                 unc_keytype_operation_t &op) {
  if (uuc::kUpllUcpDelete2 == phase) {
    UPLL_LOG_DEBUG("Delete phase 1");
    return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_KT;
  } else if (uuc::kUpllUcpUpdate == phase) {
    UPLL_LOG_DEBUG("Update phase");
    return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_KT;
  } else if (uuc::kUpllUcpCreate == phase) {
    op = UNC_OP_CREATE;
  } else if (uuc::kUpllUcpDelete == phase) {
    op = UNC_OP_DELETE;
  } else if (uuc::kUpllUcpInit == phase) {
    return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_KT;
  } else {
    return UPLL_RC_ERR_GENERIC;
  }
  return UPLL_RC_SUCCESS;
}

// Note: This function is used only by import normal read operation.
upll_rc_t VtnMoMgr::CopyKeyToVal(ConfigKeyVal *ikey,
                                 ConfigKeyVal *&okey) {
  if (!ikey)
    return UPLL_RC_ERR_GENERIC;
  upll_rc_t result_code = GetChildConfigKey(okey, NULL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetChildConfigKey failed");
    return result_code;
  }
  val_rename_vtn *val = reinterpret_cast<val_rename_vtn_t *>(
                          ConfigKeyVal::Malloc(sizeof(val_rename_vtn)));
  // Note: Validate message is take care of validate the key part
  key_vtn_t *key = reinterpret_cast<key_vtn_t *>(ikey->get_key());
  uuu::upll_strncpy(val->new_name, key->vtn_name, (kMaxLenVtnName+1));
  val->valid[UPLL_CTRLR_VTN_NAME_VALID] = UNC_VF_VALID;
  okey->SetCfgVal(new ConfigVal(IpctSt::kIpcStValRenameVtn, val));
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtnMoMgr::PathFaultHandler(uint8_t* ctrlr_id, uint8_t* domain_id,
                                bool alarm_asserted, uuc::UpllDbConnMgr* db_con,
                                uuc::ConfigLock* cfg_lock) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  unc_oper_type op = (alarm_asserted == true) ?
              UNC_PATH_FAULT : UNC_PATH_FAULT_RESET;
  state_notification notfn = (alarm_asserted == true) ?
              kPathFault : kPathFaultReset;
  ConfigKeyVal* ck_vtn = NULL;
  result_code = GetChildConfigKey(ck_vtn, NULL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetChildConfigKey failed");
    return UPLL_RC_ERR_GENERIC;
  }
  controller_domain ctrlr_dom = {ctrlr_id, domain_id};
  SET_USER_DATA_CTRLR_DOMAIN(ck_vtn, ctrlr_dom);

  DalBindInfo dal_bind_info(uudst::kDbiVtnCtrlrTbl);
  BindInfoBasedonOperation(&dal_bind_info, ck_vtn, op);
  result_code = PopulateCtrlrInfo(&dal_bind_info, db_con, cfg_lock, ck_vtn, op);
  DELETE_IF_NOT_NULL(ck_vtn);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("ClearOrGeneratePathFaultAlarms failed");
    return result_code;
  }
  DELETE_IF_NOT_NULL(ck_vtn);
  if (uuc::UpllConfigMgr::GetUpllConfigMgr()->get_map_phy_resource_status()
      == false) {
    UPLL_LOG_TRACE("map_phy_resource_status_ is disabled. So return");
    return UPLL_RC_SUCCESS;
  }
  VlinkMoMgr *vlink_mgr =
    reinterpret_cast<VlinkMoMgr *>(const_cast<MoManager *>
             (const_cast<MoManager*>(GetMoManager(UNC_KT_VLINK))));
  result_code = vlink_mgr->RestoreVlinkOperStatus(ctrlr_id, domain_id, NULL,
                                                  db_con, cfg_lock, notfn);
  if (result_code != UPLL_RC_SUCCESS &&
      result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    UPLL_LOG_DEBUG("vlink operstatus update failed");
  }
  return result_code;
}

upll_rc_t VtnMoMgr::CtrlrReconnect(
       uint8_t *ctrlr_name, uuc::UpllDbConnMgr* db_con,
       uuc::ConfigLock* cfg_lock) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *ck_vtn = NULL;
  {
  uuc::ScopedConfigLock lock(*cfg_lock, uuc::kNormalTaskPriority,
                             UPLL_DT_RUNNING, uuc::ConfigLock::CFG_READ_LOCK);
  result_code = GetChildConfigKey(ck_vtn, NULL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetChildConfigKey failed");
    return UPLL_RC_ERR_GENERIC;
  }
  SET_USER_DATA_CTRLR(ck_vtn, ctrlr_name);
  DbSubOp dbop = {kOpReadMultiple, kOpMatchCtrlr,
                  kOpInOutCtrlr | kOpInOutDomain};

  uuc::DalOdbcMgr* dmi = db_con->GetAlarmRwConn();
  if (dmi == NULL) {
    UPLL_LOG_ERROR("Error in GetAlarmRwConn");
    DELETE_IF_NOT_NULL(ck_vtn);
    return UPLL_RC_ERR_GENERIC;
  }
  result_code = ReadConfigDB(ck_vtn, UPLL_DT_STATE, UNC_OP_READ, dbop, dmi,
                             CTRLRTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("ReadConfigDB failed with result_code %d",
                    result_code);
    if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE)
      result_code = UPLL_RC_SUCCESS;
    DELETE_IF_NOT_NULL(ck_vtn);
    db_con->DalTxClose(dmi, (result_code == UPLL_RC_SUCCESS));
    db_con->ReleaseRwConn(dmi);
    return result_code;
  }
  db_con->DalTxClose(dmi, (result_code == UPLL_RC_SUCCESS));
  db_con->ReleaseRwConn(dmi);
  }
  uuc::NormalTaskYield oper_yield(uuc::TaskYieldIntf::
                                  YIELD_OP_OPER_STATUS_CTR_EVENT,
                                  cfg_lock);
  while (ck_vtn != NULL) {
  {
    UPLL_LOG_DEBUG("Set Up DB and Lock environment");

    uuc::ScopedYield scfg_yield(&oper_yield);
    uuc::DalOdbcMgr *dmi = db_con->GetAlarmRwConn();
    if (dmi == NULL) {
      UPLL_LOG_ERROR("Error in GetAlarmRwConn");
      DELETE_IF_NOT_NULL(ck_vtn);
      return UPLL_RC_ERR_GENERIC;
    }
    ConfigKeyVal *ck_vtn1 = ck_vtn->get_next_cfg_key_val();
    ck_vtn->set_next_cfg_key_val(NULL);
    uint8_t oper_status(UPPL_LOGICAL_PORT_OPER_UNKNOWN);
    result_code = ResetGWPortStatus(ck_vtn, dmi, oper_status, NULL, true);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Error in ResetGWPortStatus : %d", result_code);
      DELETE_IF_NOT_NULL(ck_vtn);
      DELETE_IF_NOT_NULL(ck_vtn1);
      db_con->DalTxClose(dmi, (result_code == UPLL_RC_SUCCESS));
      db_con->ReleaseRwConn(dmi);
      return result_code;
    }
    VbrPortMapMoMgr *vbr_pm_mgr =
        reinterpret_cast<VbrPortMapMoMgr*>(const_cast<MoManager *>
                    (const_cast<MoManager*>(GetMoManager(UNC_KT_VBR_PORTMAP))));
    result_code = vbr_pm_mgr->ResetVBrPortmapOperStatus(
        ck_vtn, dmi, oper_status);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Error in ResetVBrPortmapOperStatus : %d", result_code);
      DELETE_IF_NOT_NULL(ck_vtn);
      DELETE_IF_NOT_NULL(ck_vtn1);
      db_con->DalTxClose(dmi, (result_code == UPLL_RC_SUCCESS));
      db_con->ReleaseRwConn(dmi);
      return result_code;
    }
    val_vtn_ctrlr *ctrlr_val = reinterpret_cast<val_vtn_ctrlr *>
                                               (GetVal(ck_vtn));
    if (!ctrlr_val) {
      UPLL_LOG_DEBUG("val_vtn_ctrlr struct is NULL");
      DELETE_IF_NOT_NULL(ck_vtn);
      DELETE_IF_NOT_NULL(ck_vtn1);
      db_con->DalTxClose(dmi, (result_code == UPLL_RC_SUCCESS));
      db_con->ReleaseRwConn(dmi);
      return UPLL_RC_ERR_GENERIC;
    }
    unc_key_type_t node_key_type[]= {UNC_KT_VBRIDGE, UNC_KT_VROUTER,
                      UNC_KT_VTERMINAL, UNC_KT_VTEP, UNC_KT_VTUNNEL};
    int min_count = 0, max_count = 5;
    // based on ctrlr_type, traverse only the respective key types
    unc_keytype_ctrtype_t ctrlr_type = UNC_CT_UNKNOWN;
    if (uuc::CtrlrMgr::GetInstance()-> GetCtrlrType(
         reinterpret_cast<char*>(ctrlr_name), UPLL_DT_RUNNING, &ctrlr_type)) {
      if (ctrlr_type == UNC_CT_PFC || ctrlr_type == UNC_CT_ODC) {
        max_count = 3;
      } else if (ctrlr_type == UNC_CT_VNP || ctrlr_type == UNC_CT_POLC) {
        min_count = 3;
        max_count = 5;
      } else if (ctrlr_type == UNC_CT_VAN) {
        min_count = 2;
        max_count = 3;
      }
    }
    for (int node_count = min_count; node_count < max_count; node_count++) {
      MoMgrTables tbl[2] = {MAINTBL, CONVERTTBL};
      const unc_key_type_t ktype = node_key_type[node_count];
      uint8_t max_tbls = 1;
      if (ktype == UNC_KT_VBRIDGE)
        max_tbls = 2;
      VnodeMoMgr *mgr =
          reinterpret_cast<VnodeMoMgr *>(const_cast<MoManager *>
                      (const_cast<MoManager*>(GetMoManager(ktype))));
      for (int i = 0; i < max_tbls; i++) {
        ConfigKeyVal *ck_vnode = NULL;
        if (tbl[i] == MAINTBL) {
          result_code = mgr->GetChildConfigKey(ck_vnode, ck_vtn);
        } else {
          result_code = mgr->GetChildConvertConfigKey(ck_vnode, ck_vtn);
        }
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("GetChildConfigKey failed");
          DELETE_IF_NOT_NULL(ck_vnode);
          DELETE_IF_NOT_NULL(ck_vtn);
          DELETE_IF_NOT_NULL(ck_vtn1);
          db_con->DalTxClose(dmi, (result_code == UPLL_RC_SUCCESS));
          db_con->ReleaseRwConn(dmi);
          return result_code;
        }
        // controller_domain_t ctrlr_dom = {NULL, NULL};
        // GET_USER_DATA_CTRLR_DOMAIN(ck_vtn, ctrlr_dom);
        // SET_USER_DATA_CTRLR_DOMAIN(ck_vnode, ctrlr_dom);
        DbSubOp dbop = {kOpReadMultiple, kOpMatchCtrlr| kOpMatchDomain,
                    kOpInOutNone};
        result_code = mgr->ReadConfigDB(ck_vnode, UPLL_DT_STATE, UNC_OP_READ,
                                      dbop, dmi, tbl[i]);
        if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
          DELETE_IF_NOT_NULL(ck_vnode);
          result_code = UPLL_RC_SUCCESS;
          continue;
        } else if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("Error in Updating Operstatus on Ctrlr ReConnect");
          DELETE_IF_NOT_NULL(ck_vnode);
          DELETE_IF_NOT_NULL(ck_vtn);
          DELETE_IF_NOT_NULL(ck_vtn1);
          db_con->DalTxClose(dmi, (result_code == UPLL_RC_SUCCESS));
          db_con->ReleaseRwConn(dmi);
          return result_code;
        }
        result_code = RestoreVnodesAndIfs(ck_vnode, dmi);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("Error in Updating Operstatus on Ctrlr ReConnect");
          DELETE_IF_NOT_NULL(ck_vtn);
          DELETE_IF_NOT_NULL(ck_vtn1);
          db_con->DalTxClose(dmi, (result_code == UPLL_RC_SUCCESS));
          db_con->ReleaseRwConn(dmi);
          return result_code;
        }
      }
    }
    db_con->DalTxClose(dmi, (result_code == UPLL_RC_SUCCESS));
    db_con->ReleaseRwConn(dmi);
    DELETE_IF_NOT_NULL(ck_vtn);
    ck_vtn = ck_vtn1;
  }
  }
  return result_code;
}

upll_rc_t VtnMoMgr::RestoreVnodesAndIfs(
    ConfigKeyVal *ck_vnode, DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  unc_key_type_t ktype = UNC_KT_ROOT;
  switch (ck_vnode->get_key_type()) {
  case UNC_KT_VBRIDGE:
    ktype = UNC_KT_VBR_IF;
    break;
  case UNC_KT_VROUTER:
    ktype = UNC_KT_VRT_IF;
    break;
  case UNC_KT_VTERMINAL:
    ktype = UNC_KT_VTERM_IF;
    break;
  case UNC_KT_VTEP:
    ktype = UNC_KT_VTEP_IF;
    break;
  case UNC_KT_VTUNNEL:
    ktype = UNC_KT_VTUNNEL_IF;
    break;
  default:
    return UPLL_RC_ERR_GENERIC;
  }
  VnodeChildMoMgr *if_mgr =
        reinterpret_cast<VnodeChildMoMgr *>(const_cast<MoManager *>
        (const_cast<MoManager*>(GetMoManager(ktype))));
  while (ck_vnode != NULL) {
    ConfigKeyVal *ckv_tmp = ck_vnode->get_next_cfg_key_val();
    ck_vnode->set_next_cfg_key_val(NULL);
    uint8_t *ctrlr_name = NULL;
    GET_USER_DATA_CTRLR(ck_vnode, ctrlr_name);
    if (!ctrlr_name) {
      DELETE_IF_NOT_NULL(ck_vnode);
      DELETE_IF_NOT_NULL(ckv_tmp);
      return UPLL_RC_ERR_GENERIC;
    }
    ConfigKeyVal *ck_if = NULL;
    MoMgrTables tbl = MAINTBL;
    if (ck_vnode->get_st_num() == IpctSt::kIpcStKeyConvertVbr)
      tbl = CONVERTTBL;

    if (tbl == CONVERTTBL) {
      result_code = if_mgr->GetChildConvertConfigKey(ck_if, ck_vnode);
    } else {
      result_code = if_mgr->GetChildConfigKey(ck_if, ck_vnode);
    }
    if (result_code != UPLL_RC_SUCCESS) {
      DELETE_IF_NOT_NULL(ck_vnode);
      DELETE_IF_NOT_NULL(ckv_tmp);
      UPLL_LOG_DEBUG("GetChildConfigKey failed");
      return UPLL_RC_ERR_GENERIC;
    }
    bool is_stand_alone = false;
    uint32_t unknown_count = 0, down_count = 0;
    DbSubOp dbop = {kOpReadMultiple, kOpMatchNone,
                    kOpInOutFlag};
    result_code = if_mgr->ReadConfigDB(ck_if, UPLL_DT_STATE,
                                       UNC_OP_READ, dbop, dmi,
                                       tbl);
    if (result_code == UPLL_RC_SUCCESS) {
      is_stand_alone = false;
      ConfigKeyVal *tmp_ck_if = ck_if;
      while (tmp_ck_if != NULL) {
        ConfigKeyVal *tmp1 = tmp_ck_if->get_next_cfg_key_val();
        tmp_ck_if->set_next_cfg_key_val(NULL);
        result_code = if_mgr->UpdateVnodeIfOperStatus(tmp_ck_if, dmi);
        val_db_vbr_if_st *if_st = reinterpret_cast<val_db_vbr_if_st *>
            (GetStateVal(tmp_ck_if));
        if (!if_st)
          return UPLL_RC_ERR_GENERIC;

        if (if_st->down_count & PORT_UNKNOWN) {
          ++unknown_count;
        } else if (if_st->down_count & ~PORT_UNKNOWN) {
          ++down_count;
        }
        DELETE_IF_NOT_NULL(tmp_ck_if);
        if (result_code != UPLL_RC_SUCCESS) {
          DELETE_IF_NOT_NULL(ck_vnode);
          DELETE_IF_NOT_NULL(ckv_tmp);
          DELETE_IF_NOT_NULL(tmp1);
          UPLL_LOG_DEBUG("Error updating oper status %d", result_code);
          return result_code;
        }
        tmp_ck_if = tmp1;
      }
    } else if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      DELETE_IF_NOT_NULL(ck_if);
      is_stand_alone = true;
    } else {
      UPLL_LOG_DEBUG("Error in reading DB");
      DELETE_IF_NOT_NULL(ck_if);
      DELETE_IF_NOT_NULL(ck_vnode);
      DELETE_IF_NOT_NULL(ckv_tmp);
      return result_code;
    }
    VnodeMoMgr *mgr =
        reinterpret_cast<VnodeMoMgr *>(const_cast<MoManager *>
        (const_cast<MoManager*>(GetMoManager(ck_vnode->get_key_type()))));
    result_code = mgr->SetVnodeAndParentOperStatus(ck_vnode, dmi,
                                                   is_stand_alone, true,
                                                   unknown_count, down_count);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Error in reading DB");
      DELETE_IF_NOT_NULL(ck_vnode);
      DELETE_IF_NOT_NULL(ckv_tmp);
      return result_code;
    }
    DELETE_IF_NOT_NULL(ck_vnode);
    ck_vnode = ckv_tmp;
  }
  return result_code;
}

upll_rc_t VtnMoMgr::SetOperStatus(ConfigKeyVal *ikey,
                             state_notification notification,
                             DalDmlIntf *dmi, bool skip) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  if (!skip) {
    DbSubOp dbop = { kOpReadSingle,
                     kOpMatchNone, kOpInOutNone };
    result_code = ReadConfigDB(ikey, UPLL_DT_STATE, UNC_OP_READ, dbop,
                               dmi, MAINTBL);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Error in reading: %d", result_code);
      return result_code;
    }
  }
  string vtn_name = "";
  if (!ikey) {
    UPLL_LOG_DEBUG("Invalid param");
    return UPLL_RC_ERR_GENERIC;
  }
  ConfigVal *tmp = (ikey->get_cfg_val()) ?
                    ikey->get_cfg_val()->get_next_cfg_val() : NULL;
  val_db_vtn_st_t *vtn_valst = reinterpret_cast<val_db_vtn_st_t *>
                               ((tmp != NULL) ? tmp->get_val() : NULL);
  if (vtn_valst == NULL) {
    UPLL_LOG_DEBUG("Invalid param");
    return UPLL_RC_ERR_GENERIC;
  }
  val_vtn_st_t *vtn_val = reinterpret_cast<val_vtn_st_t *>(vtn_valst);

  vtn_val->valid[0] = UNC_VF_VALID;
  if (notification != kCtrlrDisconnect) {
    if (vtn_val->oper_status == UPLL_OPER_STATUS_UNKNOWN) {
      ConfigKeyVal *ck_ctrlr = NULL;
      result_code = GetChildConfigKey(ck_ctrlr, ikey);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Invalid param");
        return result_code;
      }
      ConfigVal *cval = NULL;
      result_code = AllocVal(cval, UPLL_DT_STATE, CTRLRTBL);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Returning error %d", result_code);
        DELETE_IF_NOT_NULL(ck_ctrlr);
        return result_code;
      }
      ck_ctrlr->AppendCfgVal(cval);
      val_vtn_ctrlr *vn = reinterpret_cast<val_vtn_ctrlr*>(GetVal(ck_ctrlr));
      vn->oper_status = UPLL_OPER_STATUS_UNKNOWN;
      vn->valid[0]= UNC_VF_VALID;
      DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutNone };
      result_code = UpdateConfigDB(ck_ctrlr, UPLL_DT_STATE, UNC_OP_READ,
                                   dmi, &dbop, CTRLRTBL);
      DELETE_IF_NOT_NULL(ck_ctrlr);
      if (result_code == UPLL_RC_ERR_INSTANCE_EXISTS) {
        vtn_val->valid[0] = UNC_VF_INVALID;
        vtn_val->oper_status = UPLL_OPER_STATUS_UNKNOWN;
      } else if (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
        UPLL_LOG_DEBUG("Error in READ UpdateConfigDB");
        return result_code;
      }
    }
  }
    /* Update oper status based on notification */
  switch (notification) {
    case kCtrlrDisconnect:
      vtn_val->oper_status = UPLL_OPER_STATUS_UNKNOWN;
      vtn_valst->down_count = 0;
      vtn_valst->unknown_count = 0;
      break;
    case kCommit:
      if (vtn_val->oper_status == UPLL_OPER_STATUS_UNKNOWN) {
        if (vtn_valst->unknown_count > 0)
          vtn_val->oper_status = UPLL_OPER_STATUS_UNKNOWN;
        else if (vtn_valst->down_count > 0)
          vtn_val->oper_status = UPLL_OPER_STATUS_DOWN;
        else
          vtn_val->oper_status = UPLL_OPER_STATUS_UP;
      } else {
        return UPLL_RC_SUCCESS;
      }
      break;
    case kPortUnknown:
      vtn_valst->unknown_count++;
      vtn_val->oper_status = UPLL_OPER_STATUS_UNKNOWN;
      break;
    case kPortUnknownFromDown:
      vtn_valst->unknown_count++;
      if (vtn_valst->down_count)
        vtn_valst->down_count--;
      vtn_val->oper_status = UPLL_OPER_STATUS_UNKNOWN;
      break;
    case kPortFault:
      vtn_valst->down_count++;
      if (vtn_valst->unknown_count == 0) {
        vtn_val->oper_status = UPLL_OPER_STATUS_DOWN;
      } else {
        vtn_val->oper_status = UPLL_OPER_STATUS_UNKNOWN;
      }
      break;
    case kPortFaultFromUnknown:
      if (vtn_valst->unknown_count)
        vtn_valst->unknown_count--;
      vtn_valst->down_count++;
      if (vtn_valst->unknown_count == 0) {
        vtn_val->oper_status = UPLL_OPER_STATUS_DOWN;
      }
      break;
    case kPortFaultReset:
      if (vtn_valst->down_count)
        vtn_valst->down_count--;
      if (vtn_valst->down_count == 0 && vtn_valst->unknown_count == 0) {
        vtn_val->oper_status = UPLL_OPER_STATUS_UP;
      }
      break;
    case kPortFaultResetFromUnknown:
      if (vtn_valst->unknown_count)
        vtn_valst->unknown_count--;
      if (vtn_valst->unknown_count == 0) {
        if (vtn_valst->down_count == 0 && vtn_valst->unknown_count == 0) {
          vtn_val->oper_status = UPLL_OPER_STATUS_UP;
        }
      }
      break;
      default:
        UPLL_LOG_DEBUG("unsupported notification for operstatus update");
        return UPLL_RC_ERR_GENERIC;
      break;
  }
  DbSubOp dbop = { kOpNotRead, kOpMatchNone, kOpInOutNone };
  result_code = UpdateConfigDB(ikey, UPLL_DT_STATE, UNC_OP_UPDATE,
                           dmi, &dbop, TC_CONFIG_GLOBAL, vtn_name, MAINTBL);
  UPLL_LOG_DEBUG("SetOperstatus for VTN after Update is \n %s",
                    ikey->ToStrAll().c_str());
  return result_code;
}

upll_rc_t VtnMoMgr::SetCtrlrOperStatus(ConfigKeyVal *ikey,
                             state_notification &notification,
                             DalDmlIntf *dmi, bool &oper_change) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  string vtn_name = "";
  if (!ikey) {
    UPLL_LOG_DEBUG("ikey is NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  val_vtn_ctrlr *ctrlr_val = reinterpret_cast<val_vtn_ctrlr *>(GetVal(ikey));
  if (!ctrlr_val) {
    UPLL_LOG_DEBUG("val_vtn_ctrlr struct is NULL");
    return UPLL_RC_ERR_GENERIC;
  }

  /* Update oper status based on notification */
  switch (notification) {
    case kCtrlrDisconnect:
      ctrlr_val->oper_status = UPLL_OPER_STATUS_UNKNOWN;
      ctrlr_val->down_count = 0;
      ctrlr_val->unknown_count = 0;
      break;
    case kCommit:
      if ((ctrlr_val->oper_status == UPLL_OPER_STATUS_UNKNOWN) &&
          (ctrlr_val->unknown_count == 0)) {
        oper_change = true;
        ctrlr_val->oper_status = UPLL_OPER_STATUS_UP;
      } else {
        return UPLL_RC_SUCCESS;
      }
      break;
    case kPortUnknown:
      ctrlr_val->unknown_count++;
      if (ctrlr_val->unknown_count == 1) {
        oper_change = true;
        if (ctrlr_val->down_count > 0) {
          notification = kPortUnknownFromDown;
        }
        ctrlr_val->oper_status = UPLL_OPER_STATUS_UNKNOWN;
      }
      break;
    case kPortUnknownFromDown:
        ctrlr_val->unknown_count++;
      if (ctrlr_val->down_count)
        ctrlr_val->down_count--;
      if (ctrlr_val->unknown_count == 1) {
        oper_change = true;
        ctrlr_val->oper_status = UPLL_OPER_STATUS_UNKNOWN;
      }
      break;
    case kPortFault:
      ctrlr_val->down_count++;
      if (ctrlr_val->down_count == 1 &&
        ctrlr_val->unknown_count == 0) {
        oper_change = true;
        ctrlr_val->oper_status = UPLL_OPER_STATUS_DOWN;
      }
      break;
    case kPortFaultFromUnknown:
      if (ctrlr_val->unknown_count)
        ctrlr_val->unknown_count--;
      ctrlr_val->down_count++;
      if (ctrlr_val->unknown_count == 0) {
        oper_change = true;
        ctrlr_val->oper_status = UPLL_OPER_STATUS_DOWN;
      }
      break;
    case kPortFaultReset:
      if (ctrlr_val->down_count)
        ctrlr_val->down_count--;
      if ((ctrlr_val->down_count == 0) &&
          (ctrlr_val->unknown_count == 0)) {
        oper_change = true;
        ctrlr_val->oper_status = UPLL_OPER_STATUS_UP;
      }
      break;
    case kPortFaultResetFromUnknown:
      if (ctrlr_val->unknown_count)
        ctrlr_val->unknown_count--;
      if (ctrlr_val->unknown_count == 0) {
        oper_change = true;
        if (ctrlr_val->down_count > 0) {
          ctrlr_val->oper_status = UPLL_OPER_STATUS_DOWN;
          notification = kPortFaultFromUnknown;
        } else {
          ctrlr_val->oper_status = UPLL_OPER_STATUS_UP;
        }
      }
      break;
      default:
        UPLL_LOG_DEBUG("unsupported notification for operstatus update");
        return UPLL_RC_ERR_GENERIC;
      break;
  }
  DbSubOp dbop = { kOpNotRead, kOpMatchCtrlr | kOpMatchDomain, kOpInOutNone };
  if (notification == kCtrlrDisconnect) {
    dbop.matchop = kOpMatchCtrlr;
  }
  result_code = UpdateConfigDB(ikey, UPLL_DT_STATE, UNC_OP_UPDATE,
                               dmi, &dbop, TC_CONFIG_GLOBAL, vtn_name,
                               CTRLRTBL);
  UPLL_LOG_DEBUG("SetCtrlrOperstatus for VTN after Update is \n %s",
                    ikey->ToStr().c_str());
  if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Error in update oper status %d", result_code);
  }
  return result_code;
}

upll_rc_t VtnMoMgr::UpdateOperStatus(ConfigKeyVal *ck_vtn,
                                       DalDmlIntf *dmi,
                                       state_notification notification,
                                       bool skip) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  if (!skip) {
    DbSubOp dbop = { kOpReadSingle,
                     kOpMatchCtrlr | kOpMatchDomain, kOpInOutNone };
    if (notification == kCtrlrDisconnect) {
      dbop.matchop = kOpMatchCtrlr;
      dbop.inoutop = kOpInOutDomain | kOpInOutCtrlr;
    }
    result_code = ReadConfigDB(ck_vtn, UPLL_DT_STATE, UNC_OP_READ, dbop,
                               dmi, CTRLRTBL);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Error in reading: %d", result_code);
      return result_code;
    }
  }
  ConfigKeyVal *tkey = ck_vtn;
  ConfigKeyVal *ck_vtn_main = NULL;
  while (tkey != NULL) {
    bool oper_change = false;
    result_code = SetCtrlrOperStatus(tkey, notification, dmi, oper_change);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("SetCtrlrOperStatus failed.Result code : %d", result_code);
      return result_code;
    }
    if (oper_change) {
      result_code = GetChildConfigKey(ck_vtn_main, tkey);
      if (!ck_vtn_main || result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Invalid param");
        DELETE_IF_NOT_NULL(ck_vtn_main);
        return result_code;
      }
      DbSubOp dbop = { kOpReadMultiple, kOpMatchNone, kOpInOutNone };
      result_code = ReadConfigDB(ck_vtn_main, UPLL_DT_STATE, UNC_OP_READ,
                         dbop, dmi, MAINTBL);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Error in reading: %d", result_code);
        DELETE_IF_NOT_NULL(ck_vtn_main);
        return result_code;
      }
      SetOperStatus(ck_vtn_main, notification, dmi, true);
      if (ck_vtn_main)
        delete ck_vtn_main;
      ck_vtn_main = NULL;
    }
    if ( skip ) break;
    tkey = tkey->get_next_cfg_key_val();
  }
  return result_code;
}

upll_rc_t VtnMoMgr::ControllerStatusHandler(
    uint8_t *ctrlr_id, uint8_t operstatus,
    uuc::UpllDbConnMgr* db_con, uuc::ConfigLock* cfg_lock) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  if (operstatus == UPPL_CONTROLLER_OPER_EVENTS_MERGED) {
    /* controller is reconnected */
    result_code = CtrlrReconnect(ctrlr_id, db_con, cfg_lock);
  } else {
    /* controller is disconnected */
    uuc::CtrlrMgr* ctr_mgr = uuc::CtrlrMgr::GetInstance();
    ConfigKeyVal* ck_vtn = NULL;
    result_code = GetChildConfigKey(ck_vtn, NULL);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("GetChildConfigKey failed");
      return UPLL_RC_ERR_GENERIC;
    }
    SET_USER_DATA_CTRLR(ck_vtn, ctrlr_id);
    if (ctr_mgr->IsPathFaultOccured(reinterpret_cast<char*>(ctrlr_id), "*")) {
      DalBindInfo dal_bind_info(uudst::kDbiVtnCtrlrTbl);
      BindInfoBasedonOperation(&dal_bind_info, ck_vtn, UNC_CTRLR_DISC);
      result_code = PopulateCtrlrInfo(&dal_bind_info, db_con, cfg_lock,
                                      ck_vtn, UNC_CTRLR_DISC);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Error in clearing pathfault alarms");
        DELETE_IF_NOT_NULL(ck_vtn);
        return result_code;
      }
    }
    DELETE_IF_NOT_NULL(ck_vtn);
    if (uuc::UpllConfigMgr::GetUpllConfigMgr()->
      get_map_phy_resource_status() == false) {
      UPLL_LOG_TRACE("map_phy_resource_status_ is disabled. So return");
      return UPLL_RC_SUCCESS;
    }
    VlinkMoMgr *vlink_mgr =
      reinterpret_cast<VlinkMoMgr *>(const_cast<MoManager *>
               (const_cast<MoManager*>(GetMoManager(UNC_KT_VLINK))));
    result_code = vlink_mgr->RestoreVlinkOperStatus(ctrlr_id,
                    reinterpret_cast<uint8_t*>(const_cast<char*>("")), NULL,
                    db_con, cfg_lock , kCtrlrDisconnect);
    if (result_code != UPLL_RC_SUCCESS &&
        result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      UPLL_LOG_DEBUG("vlink operstatus update failed");
    }
  }
  return result_code;
}

upll_rc_t VtnMoMgr::RecomputeVtnOperStatus(DalBindInfo *dal_bind_info,
                                           DalDmlIntf *dmi,
                                           ConfigKeyVal* ck_vtn,
                                           ConfigKeyVal* ikey) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  DalCursor *dal_cursor_handle = NULL;

  const uudst::kDalTableIndex tbl_index = GetTable(CTRLRTBL, UPLL_DT_RUNNING);

  if (tbl_index >= uudst::kDalNumTables) {
    UPLL_LOG_DEBUG("Invalid Table Index - %d", tbl_index);
    return UPLL_RC_ERR_GENERIC;
  }
  // crlr_dom_list : maintains vtn span of ctrlr-dom
  map<string, set<string> > crlr_dom_list;
  uuc::CtrlrMgr* ctr_mgr = uuc::CtrlrMgr::GetInstance();
  val_db_vtn_st* vtn_st = reinterpret_cast<val_db_vtn_st*>(
                              GetStateVal(ikey));

  if (vtn_st == NULL) {
    UPLL_LOG_DEBUG("empty state val received");
    return UPLL_RC_ERR_GENERIC;
  }
  result_code = DalToUpllResCode(dmi->GetMultipleRecords(
                 UPLL_DT_RUNNING, tbl_index, 0, dal_bind_info,
                 &dal_cursor_handle));
  while (result_code == UPLL_RC_SUCCESS) {
    result_code = DalToUpllResCode(dmi->GetNextRecord(dal_cursor_handle));
    if (UPLL_RC_SUCCESS == result_code) {
      controller_domain ctr_dom = {NULL, NULL};
      GET_USER_DATA_CTRLR_DOMAIN(ck_vtn, ctr_dom);
      char *ctrlr_name = reinterpret_cast<char*>(ctr_dom.ctrlr);
      char *dom_name = reinterpret_cast<char*>(ctr_dom.domain);
      // check if the ctrlr is in disconnected list.
      // If so, change the operstatus to UNKNOWN and return
      if (ctr_mgr->IsCtrlrInUnknownList(ctrlr_name)) {
        vtn_st->vtn_val_st.oper_status = UPLL_OPER_STATUS_UNKNOWN;
        crlr_dom_list.clear();
        if (dal_cursor_handle)
          dmi->CloseCursor(dal_cursor_handle);
        return result_code;
      }
      if (crlr_dom_list.find(ctrlr_name) != crlr_dom_list.end()) {
        crlr_dom_list[ctrlr_name].insert(dom_name);
      } else {
        std::set<std::string> domains;
        domains.insert(dom_name);
        crlr_dom_list[ctrlr_name] = domains;
      }
    }
  }
  if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    result_code = UPLL_RC_SUCCESS;
  }
  if (dal_cursor_handle)
    dmi->CloseCursor(dal_cursor_handle);
  key_vtn *tmp_vtn = reinterpret_cast<key_vtn*>(ck_vtn->get_key());
  if (vtn_st->vtn_val_st.oper_status == UPLL_OPER_STATUS_UP &&
    ctr_mgr->HasVtnExhaustionOccured(reinterpret_cast<char*>
              (tmp_vtn->vtn_name), "*", "*")) {
    vtn_st->vtn_val_st.oper_status = UPLL_OPER_STATUS_DOWN;
    crlr_dom_list.clear();
    return result_code;
  }
  map<string, set<string> >::iterator ctrlr_it = crlr_dom_list.begin();
  map<string, set<string> >::iterator it_end = crlr_dom_list.end();
  if (vtn_st->vtn_val_st.oper_status == UPLL_OPER_STATUS_UP) {
    ctrlr_it = crlr_dom_list.begin();
    for (; ctrlr_it != it_end; ++ctrlr_it) {
      if (!ctr_mgr->IsPathFaultOccured(ctrlr_it->first.c_str(), "*")) {
        continue;
      }
      set <string>::iterator dom_it = ctrlr_it->second.begin();
      set <string>::iterator dom_it_end = ctrlr_it->second.end();
      for (; dom_it != dom_it_end; ++dom_it) {
        // check if the ctrlr-dom is in pathfault list.
        // If so, change the operstatus to DOWN and return
        if (ctr_mgr->IsPathFaultOccured(ctrlr_it->first.c_str(),
                                        (*dom_it).c_str())) {
          vtn_st->vtn_val_st.oper_status = UPLL_OPER_STATUS_DOWN;
          crlr_dom_list.clear();
          return result_code;
        }
      }
    }
  }
  return result_code;
}

upll_rc_t VtnMoMgr::BindInfoBasedonOperation(DalBindInfo *dal_bind_info,
                                   ConfigKeyVal *&ck_vtn,
                                   unc_oper_type op) {
// bind vtn to match or output based on operation
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  int nattr = 0;
  BindInfo *binfo = NULL;
  if (!GetBindInfo(CTRLRTBL, UPLL_DT_RUNNING, binfo, nattr)) {
    UPLL_LOG_DEBUG("GetBindInfo failed");
    return UPLL_RC_ERR_GENERIC;
  }
  if (op == UNC_START_UP)
    GET_USER_DATA(ck_vtn);

  void *tkey = ck_vtn->get_key();
  uint64_t indx = binfo[0].index;
  void *p = reinterpret_cast<void *>(reinterpret_cast<char *>(tkey)
               + binfo[0].offset);

  switch (op) {
    case UNC_PATH_FAULT:
    case UNC_PATH_FAULT_RESET:
      // bind vtn_name to output for sending alarms
      dal_bind_info->BindOutput(indx, binfo[0].app_data_type,
                binfo[0].array_size, p);
    break;
    case UNC_DT_STATE_READ:
      // match vtn_name for VTN_DT_STATE read
      dal_bind_info->BindMatch(indx, binfo[0].app_data_type,
                binfo[0].array_size, p);
    break;
    case UNC_CTRLR_DISC:
      // bind vtn_name to output for recovering pathfault alarms
      // on controller disconnect or user initiated audit
      dal_bind_info->BindOutput(indx, binfo[0].app_data_type,
                binfo[0].array_size, p);
    break;
    case UNC_START_UP:
    default:
    // do nothing
    break;
  }
  tkey = ck_vtn->get_user_data();
  for (int i = 1; i < 3; i++) {
    indx = binfo[i].index;
    p = reinterpret_cast<void *>(reinterpret_cast<char *>(tkey)
                 + binfo[i].offset);
    switch (op) {
      case UNC_PATH_FAULT:
      case UNC_PATH_FAULT_RESET:
        // bind controller-domain as match and output
        dal_bind_info->BindMatch(indx, binfo[i].app_data_type,
                binfo[i].array_size, p);
        dal_bind_info->BindOutput(indx, binfo[i].app_data_type,
                  binfo[i].array_size, p);
      break;
      case UNC_DT_STATE_READ:
        // bind controller-domain to output for VTN_DT_STATE read
        // to determine operstatus based on controller state
        // and pathfault controller-domain
        dal_bind_info->BindOutput(indx, binfo[i].app_data_type,
                  binfo[i].array_size, p);
      break;
      case UNC_CTRLR_DISC:
        // bind controller as match and controller-domain to output
        if (i == 1) {
          dal_bind_info->BindMatch(indx, binfo[i].app_data_type,
                  binfo[i].array_size, p);
        }
        dal_bind_info->BindOutput(indx, binfo[i].app_data_type,
                binfo[i].array_size, p);
      break;
      case UNC_START_UP:
        // bind controller to output
        if (i == 1) {
          dal_bind_info->BindOutput(indx, binfo[i].app_data_type,
                  binfo[i].array_size, p);
        }
      break;
      default:
      // do nothing
      break;
    }
  }
  return result_code;
}

upll_rc_t VtnMoMgr::PopulateCtrlrInfo(DalBindInfo *dal_bind_info,
                                      uuc::UpllDbConnMgr* db_con,
                                      uuc::ConfigLock* cfg_lock,
                                      ConfigKeyVal *ck_vtn, unc_oper_type op) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  DalCursor *dal_cursor_handle = NULL;
  const uudst::kDalTableIndex tbl_index = GetTable(CTRLRTBL, UPLL_DT_RUNNING);
  if (tbl_index >= uudst::kDalNumTables) {
    UPLL_LOG_DEBUG("Invalid Table Index - %d", tbl_index);
    return UPLL_RC_ERR_GENERIC;
  }
  uuc::CtrlrMgr* ctr_mgr = uuc::CtrlrMgr::GetInstance();
  uuc::ScopedConfigLock lock(*cfg_lock, uuc::kNormalTaskPriority,
      UPLL_DT_RUNNING, uuc::ConfigLock::CFG_READ_LOCK);
  uuc::DalOdbcMgr* dmi = db_con->GetAlarmRwConn();
  if (dmi == NULL) {
    UPLL_LOG_ERROR("Error in GetAlarmRwConn");
    return UPLL_RC_ERR_GENERIC;
  }
  if (op == UNC_START_UP) {
    string query_string = "SELECT DISTINCT ctrlr_name from ru_vtn_ctrlr_tbl";
    result_code = DalToUpllResCode(dmi->ExecuteAppQueryMultipleRecords(
               query_string, 0, dal_bind_info,
               &dal_cursor_handle));
  } else {
    result_code = DalToUpllResCode(dmi->GetMultipleRecords(
                 UPLL_DT_RUNNING, tbl_index, 0, dal_bind_info,
                 &dal_cursor_handle));
  }
  while (result_code == UPLL_RC_SUCCESS) {
    result_code = DalToUpllResCode(dmi->GetNextRecord(dal_cursor_handle));
    if (UPLL_RC_SUCCESS == result_code) {
      switch (op) {
        case UNC_START_UP: {
          uint8_t *ctrlr_name = NULL;
          GET_USER_DATA_CTRLR(ck_vtn, ctrlr_name);
          if (ctrlr_name) {
            ctr_mgr->AddtoUnknownCtrlrList(
                     reinterpret_cast<char*>(ctrlr_name));
          }
        }
        break;
        case UNC_CTRLR_DISC: {
          key_vtn *vtn = static_cast<key_vtn *>(ck_vtn->get_key());
          controller_domain ctr_dom = {NULL, NULL};
          GET_USER_DATA_CTRLR_DOMAIN(ck_vtn, ctr_dom);
          char *ctr_name = reinterpret_cast<char*>(ctr_dom.ctrlr);
          char *dom_name = reinterpret_cast<char*>(ctr_dom.domain);
          uuc::UpllConfigMgr *cfg_instance =
                              uuc::UpllConfigMgr::GetUpllConfigMgr();
          uuc::CtrlrMgr* ctr_mgr = uuc::CtrlrMgr::GetInstance();
          if (ctr_mgr->IsPathFaultOccured(ctr_name, dom_name)) {
            UPLL_LOG_DEBUG("clear Path Fault alarm");
            bool bResult = cfg_instance->SendPathFaultAlarm(
                           ctr_name, dom_name,
                           reinterpret_cast<char*>(vtn->vtn_name),
                           uuc::UPLL_CLEAR_WITHOUT_TRAP);
            if (!bResult) {
              UPLL_LOG_DEBUG("Failed to recover Pathfault Alarm");
            }
          }
        }
        break;
        case UNC_PATH_FAULT:
        case UNC_PATH_FAULT_RESET: {
          uuc::upll_alarm_kind_t alarm_type = (op == UNC_PATH_FAULT)?
                        uuc::UPLL_ASSERT_WITH_TRAP: uuc::UPLL_CLEAR_WITH_TRAP;
          key_vtn *vtn = static_cast<key_vtn *>(ck_vtn->get_key());
          controller_domain ctr_dom = {NULL, NULL};
          GET_USER_DATA_CTRLR_DOMAIN(ck_vtn, ctr_dom);
          uuc::UpllConfigMgr *cfg_instance =
                            uuc::UpllConfigMgr::GetUpllConfigMgr();
          if (!cfg_instance->SendPathFaultAlarm(
                           reinterpret_cast<char*>(ctr_dom.ctrlr),
                           reinterpret_cast<char*>(ctr_dom.domain),
                           reinterpret_cast<char*>(vtn->vtn_name),
                           alarm_type)) {
            UPLL_LOG_DEBUG("Failed in sending or clearing pathfault alarm");
          }
        }
        break;
        default:
          // do nothing
        break;
      }
    }
  }
  if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    result_code = UPLL_RC_SUCCESS;
  }
  if (dal_cursor_handle)
    dmi->CloseCursor(dal_cursor_handle, false);
  db_con->DalTxClose(dmi, (result_code == UPLL_RC_SUCCESS));
  db_con->ReleaseRwConn(dmi);
  return result_code;
}

upll_rc_t VtnMoMgr::ConvertGatewayPort(ConfigKeyVal *ikey,
                                       ConfigKeyVal *uppl_ikey,
                                       DalDmlIntf *dmi,
                                       unc_keytype_operation_t operation,
                                       TcConfigMode config_mode,
                                       string vtn_name) {
  UPLL_FUNC_TRACE;

  upll_rc_t result_code = UPLL_RC_SUCCESS;

  controller_domain ctrlr_dom;
  memset(&ctrlr_dom, 0, sizeof(controller_domain));
  GET_USER_DATA_CTRLR_DOMAIN(ikey, ctrlr_dom);
  if (!ctrlr_dom.ctrlr || !ctrlr_dom.domain) {
    UPLL_LOG_DEBUG("Ctrlr/Domain is NULL");
    return UPLL_RC_ERR_BAD_REQUEST;
  }
  val_boundary *boundary_val_data = NULL;
  if (operation == UNC_OP_CREATE) {
    // Fetch uppl key from input
    boundary_val_data = reinterpret_cast<val_boundary *>
        (GetVal(uppl_ikey));
    if (boundary_val_data == NULL) {
      UPLL_LOG_DEBUG("Invalid parameter");
      return UPLL_RC_ERR_GENERIC;
    }
  }

  // Fecth vlink val from input
  val_convert_vlink *convert_vlink = reinterpret_cast<val_convert_vlink *>
    (GetVal(ikey));
  if (operation == UNC_OP_CREATE && convert_vlink == NULL) {
    UPLL_LOG_DEBUG("Invalid parameter");
    return UPLL_RC_ERR_GENERIC;
  }

  // Allocate memory for vtn_gatewayport key and val
  key_vtn *gateway_key = ConfigKeyVal::Malloc<key_vtn>();
  val_vtn_gateway_port *gateway_val =
    ConfigKeyVal::Malloc<val_vtn_gateway_port>();

  // Vtn gateway port Configkeyval
  ConfigVal *gwval = new ConfigVal(IpctSt::kIpcStValVtnGatewayPort,
                     gateway_val);
  ConfigKeyVal *gw_ckv = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn,
      gateway_key, gwval);
  // set controller and domain
  SET_USER_DATA_CTRLR_DOMAIN(gw_ckv, ctrlr_dom);
  uuu::upll_strncpy(gateway_key->vtn_name,
      reinterpret_cast<key_convert_vlink *>(
          ikey->get_key())->vbr_key.vtn_key.vtn_name, (kMaxLenVtnName+1));

  DbSubOp dbop = {kOpReadSingle, kOpMatchCtrlr | kOpMatchDomain, kOpInOutFlag};
  DbSubOp dbop_up = {kOpNotRead, kOpMatchCtrlr | kOpMatchDomain,
                     kOpInOutCtrlr|kOpInOutDomain|kOpInOutFlag};
  result_code = ReadConfigDB(gw_ckv, UPLL_DT_CANDIDATE, UNC_OP_READ, dbop, dmi,
      CONVERTTBL);
  // Return DB related errors other than SUCCESS and NO_SUCH_INSTANCE
  if (UPLL_RC_SUCCESS != result_code &&
      UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
    DELETE_IF_NOT_NULL(gw_ckv);
    return result_code;
  }
  if (operation == UNC_OP_CREATE) {
    uint8_t rename_flag = 0;
    GET_USER_DATA_FLAGS(ikey, rename_flag);
    if (rename_flag & VTN_RENAME) {
      SET_USER_DATA_FLAGS(gw_ckv, VTN_RENAME);
    }
    // If entry is not there in DB, create the entry in DB
    if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
     dbop_up.matchop = kOpMatchNone;
     if ((!(strcmp(reinterpret_cast<char *>(ctrlr_dom.ctrlr),
         reinterpret_cast<char *>(boundary_val_data->controller_name1)))) &&
          (!(strcmp(reinterpret_cast<char *>(ctrlr_dom.domain),
            reinterpret_cast<char *>(boundary_val_data->domain_name1))))) {
        uuu::upll_strncpy(gateway_val->logical_port_id,
            boundary_val_data->logical_port_id1,
            kMaxLenLogicalPortId+1);
      } else if ((!(strcmp(reinterpret_cast<char *>(ctrlr_dom.ctrlr),
          reinterpret_cast<char *>(boundary_val_data->controller_name2)))) &&
          (!(strcmp(reinterpret_cast<char *>(ctrlr_dom.domain),
               reinterpret_cast<char *>(boundary_val_data->domain_name2))))) {
        uuu::upll_strncpy(gateway_val->logical_port_id,
            boundary_val_data->logical_port_id2,
            kMaxLenLogicalPortId+1);
      }
      gateway_val->valid[UPLL_IDX_LOGICAL_PORT_ID_GWPORT] = UNC_VF_VALID;
      gateway_val->label = convert_vlink->label;
      gateway_val->valid[UPLL_IDX_LABEL_GWPORT] = UNC_VF_VALID;
    } else if (result_code == UPLL_RC_SUCCESS) {
      operation = UNC_OP_UPDATE;
    }
    // If Entry exist in DB than increment the ref_count and UPDATE in DB
    gateway_val->valid[UPLL_IDX_REF_COUNT_GWPORT] = UNC_VF_VALID;
    gateway_val->ref_count++;
  } else if (operation == UNC_OP_DELETE) {
    // If operation is DELETE and entry is exist in DB, also ref_count >0,
    // decrement the ref_count.Else delete the entry
    if (gateway_val->ref_count > 1) {
      gateway_val->ref_count--;
      gateway_val->valid[UPLL_IDX_REF_COUNT_GWPORT] = UNC_VF_VALID;
      operation = UNC_OP_UPDATE;
    } else {
      dbop_up.inoutop = kOpInOutNone;
    }
  }
  // Insert/delete/update the entry to database
  result_code = UpdateConfigDB(gw_ckv, UPLL_DT_CANDIDATE,
      operation, dmi, &dbop_up, config_mode, vtn_name, CONVERTTBL);
  DELETE_IF_NOT_NULL(gw_ckv);
  return result_code;
}

upll_rc_t VtnMoMgr::DupConfigKeyValVtnGatewayPort(ConfigKeyVal *&okey,
                                                  ConfigKeyVal *req) {
  UPLL_FUNC_TRACE;
  void *tkey = req ? (req)->get_key() : NULL;
  if (tkey == NULL) {
    UPLL_LOG_INFO("Input Configkeyval or key is Null");
    return UPLL_RC_ERR_GENERIC;
  }
  if (okey != NULL) {
    UPLL_LOG_INFO("okey is Not Null");
    return UPLL_RC_ERR_GENERIC;
  }
  ConfigVal *tmp1 = NULL, *tmp = (req)->get_cfg_val();
  val_vtn_gateway_port_t *vtn_gw_port = NULL;
  if (tmp) {
    val_vtn_gateway_port_t *ival =
         reinterpret_cast<val_vtn_gateway_port_t *>(GetVal(req));
    vtn_gw_port = reinterpret_cast<val_vtn_gateway_port_t *>(
          ConfigKeyVal::Malloc(sizeof(val_vtn_gateway_port_t)));
    memcpy(reinterpret_cast<char *>(vtn_gw_port),
           reinterpret_cast<char *>(ival),
           sizeof(val_vtn_gateway_port_t));
    tmp1 = new ConfigVal(IpctSt::kIpcStValVtnGatewayPort, vtn_gw_port);
  }
  key_vtn_t *ikey = reinterpret_cast<key_vtn_t*>(tkey);
  key_vtn_t *key = reinterpret_cast<key_vtn_t *>(
      ConfigKeyVal::Malloc(sizeof(key_vtn_t)));
  memcpy(reinterpret_cast<char *>(key), reinterpret_cast<char *>(ikey),
         sizeof(key_vtn_t));
  okey = new ConfigKeyVal(UNC_KT_VTN,
                   IpctSt::kIpcStKeyVtn, key, tmp1);
  SET_USER_DATA(okey, req)
  return UPLL_RC_SUCCESS;
}
upll_rc_t VtnMoMgr::PartialMergeValidate(unc_key_type_t keytype,
                                         const char *ctrlr_id,
                                         ConfigKeyVal *err_ckv,
                                         DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  ConfigKeyVal *run_ckv = NULL;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  if (!ctrlr_id || !err_ckv) {
    UPLL_LOG_DEBUG("Invalid input");
    return UPLL_RC_ERR_GENERIC;
  }

  //  Get vbridge portmap ConfigKeyVal
  result_code = GetChildConfigKey(run_ckv, NULL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetChildConfigKey failed");
    return result_code;
  }

  SET_USER_DATA_CTRLR(run_ckv, ctrlr_id);
  DbSubOp dbop = { kOpReadMultiple, kOpMatchCtrlr,
    kOpInOutCtrlr | kOpInOutDomain };
  //  Get all running gatewayport configuration
  result_code = ReadConfigDB(run_ckv, UPLL_DT_RUNNING, UNC_OP_READ, dbop,
                             dmi, CONVERTTBL);
  if (UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code &&
      UPLL_RC_SUCCESS != result_code) {
    delete run_ckv;
    UPLL_LOG_DEBUG("ReadConfigDB failed %d", result_code);
    return result_code;
  }
  if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
    //  In Running there is no gateway port information so return success
    delete run_ckv;
    return UPLL_RC_SUCCESS;
  }
  ConfigKeyVal *start_ckv = run_ckv;
  while (run_ckv) {
    ConfigKeyVal *import_ckv = NULL;

    result_code = DupConfigKeyValVtnGatewayPort(import_ckv, run_ckv);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_ERROR("Failed to get gatewayport ConfigKeyVal");
      delete start_ckv;
      return result_code;
    }
    val_vtn_gateway_port_t *gw_port_val =
        reinterpret_cast<val_vtn_gateway_port_t*>(GetVal(import_ckv));
    gw_port_val->valid[UPLL_IDX_REF_COUNT_GWPORT] = UNC_VF_INVALID;

    DbSubOp dbop1 = { kOpReadSingle, kOpMatchCtrlr | kOpMatchDomain,
                      kOpInOutNone };
    result_code = UpdateConfigDB(import_ckv, UPLL_DT_IMPORT, UNC_OP_READ,
                                 dmi, &dbop1, CONVERTTBL);
    if (result_code != UPLL_RC_ERR_INSTANCE_EXISTS) {
      if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
         result_code = UPLL_RC_ERR_MERGE_CONFLICT;
         UPLL_LOG_INFO("Gateway port configuration is modified / removed in"
                       " imported controller configuration");
      }
      //  merge conflict whether need to return unified vBridge
      //  configkeyval or
      err_ckv->ResetWith(import_ckv);
      delete import_ckv;
      delete start_ckv;
      return result_code;
    }
    DELETE_IF_NOT_NULL(import_ckv);
    run_ckv = run_ckv->get_next_cfg_key_val();
  }
  DELETE_IF_NOT_NULL(start_ckv);

  return UPLL_RC_SUCCESS;
}

upll_rc_t VtnMoMgr::CreateImportGatewayPort(ConfigKeyVal *ikey,
                               DalDmlIntf *dmi, IpcReqRespHeader *req,
                               upll_import_type import_type) {
  UPLL_FUNC_TRACE;

  //  If import_type is full return error
  if (import_type == UPLL_IMPORT_TYPE_FULL) {
    UPLL_LOG_ERROR("During full cannot create gateway port configuration ");
    return UPLL_RC_ERR_CFG_SEMANTIC;
  }

  //  Validate input params
  if (!ikey || !ikey->get_key() || !dmi || !req) {
    UPLL_LOG_ERROR("Invalid params");
    return UPLL_RC_ERR_GENERIC;
  }

  //  In CreateImportMo label_type is GW_VLAN, if logical_port_id valid
  //  flag is not UNC_VF_VALID return UPLL_RC_ERR_CFG_SEMANTIC
  pfcdrv_val_vbr_portmap_t *vbr_pm_val =
      reinterpret_cast<pfcdrv_val_vbr_portmap_t*>(GetVal(ikey));
  if (vbr_pm_val->vbrpm.valid[UPLL_IDX_LOGICAL_PORT_ID_VBRPM]
      != UNC_VF_VALID) {
    UPLL_LOG_ERROR("Invalid logical portid is received");
    return UPLL_RC_ERR_GENERIC;
  }

  key_vbr_portmap_t *vbrportmap_key = reinterpret_cast<key_vbr_portmap_t*>
                                     (ikey->get_key());
  MoMgrImpl *mgr = reinterpret_cast<MoMgrImpl *>
      (const_cast<MoManager *>
       (const_cast<MoManager*>(GetMoManager(UNC_KT_VTN))));
  if (!mgr) {
    UPLL_LOG_DEBUG("Instance is null");
    return UPLL_RC_ERR_GENERIC;
  }

  upll_rc_t    result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *gw_port_ckv = NULL;
  ConfigKeyVal *vtn_ckv = NULL;

  result_code = mgr->GetChildConfigKey(vtn_ckv, NULL);
  if (result_code != UPLL_RC_SUCCESS) {
    return UPLL_RC_ERR_GENERIC;
  }

  key_vtn_t *vtnkey = reinterpret_cast<key_vtn_t*>(vtn_ckv->get_key());
  if (!vtnkey) {
    DELETE_IF_NOT_NULL(vtn_ckv);
    return UPLL_RC_ERR_GENERIC;
  }
  uuu::upll_strncpy(vtnkey->vtn_name, vbrportmap_key->vbr_key.vtn_key.vtn_name,
               kMaxLenVtnName + 1);
  SET_USER_DATA_CTRLR(vtn_ckv, vbr_pm_val->vbrpm.controller_id);
  SET_USER_DATA_DOMAIN(vtn_ckv, vbr_pm_val->vbrpm.domain_id);

  result_code = mgr->GetRenamedUncKey(vtn_ckv, UPLL_DT_CANDIDATE, dmi,
                                      vbr_pm_val->vbrpm.controller_id);
  if (UPLL_RC_SUCCESS != result_code &&
      UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
    UPLL_LOG_DEBUG("GetRenamedUncKey failed %d", result_code);
    DELETE_IF_NOT_NULL(vtn_ckv);
    return result_code;
  }

  result_code = mgr->UpdateConfigDB(vtn_ckv, req->datatype, UNC_OP_CREATE, dmi,
                                    TC_CONFIG_GLOBAL, "", MAINTBL);
  if (result_code == UPLL_RC_ERR_INSTANCE_EXISTS)
    result_code = UPLL_RC_SUCCESS;
  if (UPLL_RC_SUCCESS != result_code &&
      UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
    UPLL_LOG_DEBUG("VTN creation failed %d", result_code);
    DELETE_IF_NOT_NULL(vtn_ckv);
    return result_code;
  }

  //  Populate vtn gateway ConfigKeyVal
  result_code = GetChildConfigKey(gw_port_ckv, NULL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_ERROR("Failed to get VTN ConfigKeyVal");
    DELETE_IF_NOT_NULL(vtn_ckv);
    return result_code;
  }

  vtnkey = reinterpret_cast<key_vtn_t*>(vtn_ckv->get_key());
  //  Get vbr_portmap key
  key_vtn_t *vtn_key = reinterpret_cast<key_vtn_t*>(gw_port_ckv->get_key());
  uuu::upll_strncpy(vtn_key->vtn_name, vtnkey->vtn_name,
               kMaxLenVtnName + 1);
  //  Populate val_gateway_port
  val_vtn_gateway_port_t  *gw_port_val =
      ConfigKeyVal::Malloc<val_vtn_gateway_port_t>();
  gw_port_val->valid[UPLL_IDX_LOGICAL_PORT_ID_GWPORT] = UNC_VF_VALID;
  uuu::upll_strncpy(gw_port_val->logical_port_id,
                vbr_pm_val->vbrpm.logical_port_id, kMaxLenLogicalPortId + 1);
  gw_port_val->valid[UPLL_IDX_LABEL_GWPORT] = UNC_VF_VALID;
  gw_port_val->label = vbr_pm_val->vbrpm.label;
  // Append ConfigVal
  gw_port_ckv->AppendCfgVal(IpctSt::kIpcStValVtnGatewayPort, gw_port_val);

  SET_USER_DATA_CTRLR(gw_port_ckv, vbr_pm_val->vbrpm.controller_id);
  SET_USER_DATA_DOMAIN(gw_port_ckv, vbr_pm_val->vbrpm.domain_id);

  DbSubOp dbop = {kOpReadExist, kOpMatchCtrlr | kOpMatchDomain, kOpInOutNone};
  //  Validate received gateway port configuration exist in
  //  running database.If not exist return UPLL_RC_ERR_CFG_SEMANTIC
  if (req->datatype == UPLL_DT_IMPORT) {
    result_code = UpdateConfigDB(gw_port_ckv, UPLL_DT_RUNNING,
                                 UNC_OP_READ, dmi, &dbop, CONVERTTBL);
    if (result_code != UPLL_RC_ERR_INSTANCE_EXISTS) {
      if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
        UPLL_LOG_ERROR("Cannot create gateway port configuration for"
                       "new vBrdige");
        result_code = UPLL_RC_ERR_CFG_SEMANTIC;
      }
      DELETE_IF_NOT_NULL(gw_port_ckv);
      DELETE_IF_NOT_NULL(vtn_ckv);
      return result_code;
    }
  }

  uint8_t flags = 0;
  GET_USER_DATA_FLAGS(vtn_ckv, flags);
  SET_USER_DATA_FLAGS(gw_port_ckv, flags);
  //  Create gateway configuration in import databas
  DbSubOp dbop1 = {kOpNotRead, kOpMatchNone,
    kOpInOutCtrlr | kOpInOutDomain | kOpInOutFlag};
  result_code =  UpdateConfigDB(gw_port_ckv, req->datatype,
                               UNC_OP_CREATE, dmi, &dbop1,
                               TC_CONFIG_GLOBAL, "", CONVERTTBL);
  DELETE_IF_NOT_NULL(gw_port_ckv);
  DELETE_IF_NOT_NULL(vtn_ckv);
  return result_code;
}

upll_rc_t VtnMoMgr::TxUpdateController(unc_key_type_t keytype,
                                       uint32_t session_id,
                                       uint32_t config_id,
                                       uuc::UpdateCtrlrPhase phase,
                                       set<string> *affected_ctrlr_set,
                                       DalDmlIntf *dmi,
                                       ConfigKeyVal **err_ckv,
                                       TxUpdateUtil *tx_util,
                                       TcConfigMode config_mode,
                                       std::string vtn_name) {
  UPLL_FUNC_TRACE;
  if (config_mode == TC_CONFIG_VIRTUAL)
    return UPLL_RC_SUCCESS;

  upll_rc_t result_code = UPLL_RC_SUCCESS;
  DalResultCode dal_result = uud::kDalRcSuccess;
  ConfigKeyVal *req = NULL, *nreq = NULL, *ck_main = NULL;
  DalCursor *dal_cursor_handle = NULL;
  //  Gives the operation corresponding to the phase
  unc_keytype_operation_t op = (phase == uuc::kUpllUcpCreate)?UNC_OP_CREATE:
    ((phase == uuc::kUpllUcpUpdate)?UNC_OP_UPDATE:
     ((phase == uuc::kUpllUcpDelete)?UNC_OP_DELETE:UNC_OP_INVALID));

  if (op == UNC_OP_INVALID) {
    UPLL_LOG_INFO("Invalid operation received-%d", op);
    // Not a valid operation
    return UPLL_RC_ERR_BAD_REQUEST;
  }

  controller_domain ctrlr_dom;
  ctrlr_dom.ctrlr = ctrlr_dom.domain = NULL;
  // in vtn gate port CREATE request,
  // if the set contains the vtn_name then the spine id is exist in RUNNING
  // Else do DB Read and check
  // Set is used for avoiding the DB read for spine id existence check.
  std::set<uint8_t *> vtn_list;

  MoMgrTables tbl[] = {MAINTBL, CONVERTTBL};
  int ntbl =  sizeof(tbl)/ sizeof(tbl[0]);
  for (int tbl_indx = 0; tbl_indx < ntbl; tbl_indx++) {
    if (tbl[tbl_indx] == MAINTBL) {
      tbl[tbl_indx] = (op != UNC_OP_UPDATE)?CTRLRTBL:MAINTBL;
    }
    // Update operation not supported for UNC_KT_VTN
    if (((op == UNC_OP_UPDATE) && (keytype == UNC_KT_VTN)) &&
        (tbl[tbl_indx] == MAINTBL)) {
      UPLL_LOG_TRACE("UPDATE to VTN is not supported at driver. so return..");
      continue;  // need to check the converttbl
    }
    result_code = DiffConfigDB(UPLL_DT_CANDIDATE, UPLL_DT_RUNNING,
        op, req, nreq, &dal_cursor_handle, dmi,
        config_mode, vtn_name, tbl[tbl_indx]);
    if (UPLL_RC_SUCCESS != result_code) {
      DELETE_IF_NOT_NULL(req);
      DELETE_IF_NOT_NULL(nreq);
      if (dal_cursor_handle != NULL) {
        dmi->CloseCursor(dal_cursor_handle, true);
      }
      if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
        result_code = UPLL_RC_SUCCESS;
        if (tbl[tbl_indx] != CONVERTTBL) {
          // Need to check for convertbl
          continue;
        }
      }
      return result_code;
    }

    upll_keytype_datatype_t dt_type;
    while (result_code == UPLL_RC_SUCCESS) {
      if (tx_util->GetErrCount() > 0) {
        UPLL_LOG_ERROR("TxUpdateUtil says exit the loop.");
        dmi->CloseCursor(dal_cursor_handle, true);
        DELETE_IF_NOT_NULL(nreq);
        DELETE_IF_NOT_NULL(req);
        return UPLL_RC_ERR_GENERIC;
      }
      string domain_type;
      // Get Next Record
      dal_result = dmi->GetNextRecord(dal_cursor_handle);
      result_code = DalToUpllResCode(dal_result);
      if (result_code != UPLL_RC_SUCCESS)
        break;
      ck_main = NULL;
      dt_type = (UNC_OP_DELETE == op)? UPLL_DT_RUNNING:UPLL_DT_CANDIDATE;
      if (((op == UNC_OP_CREATE) || (op == UNC_OP_DELETE)) &&
          (tbl[tbl_indx] == CTRLRTBL)) {
        result_code = GetChildConfigKey(ck_main, req);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("GetChildConfigKey failed %d", result_code);
          dmi->CloseCursor(dal_cursor_handle, true);
          DELETE_IF_NOT_NULL(req);
          DELETE_IF_NOT_NULL(nreq);
          return result_code;
        }
        //  req is got from CTRLR TBL so the record is read from MAINTBL.
        DbSubOp dbop = {kOpReadSingle, kOpMatchNone, kOpInOutCs};
        result_code = ReadConfigDB(ck_main, dt_type,
            UNC_OP_READ, dbop, dmi, MAINTBL);
        if (UPLL_RC_SUCCESS != result_code) {
          UPLL_LOG_DEBUG("ReadConfigDB failed %d", result_code);
          dmi->CloseCursor(dal_cursor_handle, true);
          DELETE_IF_NOT_NULL(ck_main);
          DELETE_IF_NOT_NULL(req);
          DELETE_IF_NOT_NULL(nreq);
          return result_code;
        }
        ctrlr_dom.ctrlr = ctrlr_dom.domain = NULL;
        GET_USER_DATA_CTRLR_DOMAIN(req, ctrlr_dom);

        VbrMoMgr *vbr_mgr = reinterpret_cast<VbrMoMgr *>
          (const_cast<MoManager *>(GetMoManager(UNC_KT_VBRIDGE)));

        // VTN, VTN convert table CREATE/UPDATE/DELETE operation,
        // the given VTN is exists in vBridge CONVERTTBL,
        // then send domain_id with (PE_LEAF) prefix
        ConfigKeyVal *temp_key = NULL;
        result_code = vbr_mgr->GetChildConvertConfigKey(temp_key, ck_main);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_INFO("GetChildConfigKey failed %d", result_code);
          dmi->CloseCursor(dal_cursor_handle, true);
          DELETE_IF_NOT_NULL(ck_main);
          DELETE_IF_NOT_NULL(req);
          DELETE_IF_NOT_NULL(nreq);
          return result_code;
        }
        DbSubOp dbop1 = { kOpReadExist, kOpMatchCtrlr | kOpMatchDomain,
                          kOpInOutNone };
        result_code = vbr_mgr->UpdateConfigDB(temp_key, dt_type, UNC_OP_READ,
                                              dmi, &dbop1, CONVERTTBL);
        DELETE_IF_NOT_NULL(temp_key);
        if (result_code == UPLL_RC_ERR_INSTANCE_EXISTS) {
          domain_type = "(PF_LEAF)";
        }
        ConfigKeyVal *ckv_unc = NULL;
        //  ck_main is cached to set it to err_ckv when driver returns failure.
        result_code = DupConfigKeyVal(ckv_unc, ck_main, MAINTBL);
        if (UPLL_RC_SUCCESS != result_code) {
          UPLL_LOG_DEBUG("DupConfigKeyVal failed %d", result_code);
          dmi->CloseCursor(dal_cursor_handle, true);
          DELETE_IF_NOT_NULL(ck_main);
          DELETE_IF_NOT_NULL(req);
          DELETE_IF_NOT_NULL(nreq);
          return result_code;
        }
        SET_USER_DATA_CTRLR_DOMAIN(ckv_unc, ctrlr_dom);
        //  Rename Key and Val struct to Controller Names.
        result_code = GetRenamedControllerKey(ck_main, dt_type,
            dmi, &ctrlr_dom);
        if (result_code != UPLL_RC_SUCCESS &&
            result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
          UPLL_LOG_DEBUG("GetRenamedControllerKey failed, err %d", result_code);
          dmi->CloseCursor(dal_cursor_handle, true);
          DELETE_IF_NOT_NULL(ck_main);
          DELETE_IF_NOT_NULL(ckv_unc);
          DELETE_IF_NOT_NULL(req);
          DELETE_IF_NOT_NULL(nreq);
          return result_code;
        }
        ConfigKeyVal *ckv_driver = NULL;
        result_code = DupConfigKeyVal(ckv_driver, ck_main, MAINTBL);
        if (UPLL_RC_SUCCESS != result_code) {
          UPLL_LOG_DEBUG("DupConfigKeyVal failed %d", result_code);
          DELETE_IF_NOT_NULL(ck_main);
          DELETE_IF_NOT_NULL(ckv_unc);
          DELETE_IF_NOT_NULL(nreq);
          DELETE_IF_NOT_NULL(req);
          dmi->CloseCursor(dal_cursor_handle, true);
          return result_code;
        }
        SET_USER_DATA_CTRLR_DOMAIN(ckv_driver, ctrlr_dom);
        //  Send the request to driver.
        result_code = tx_util->EnqueueRequest(session_id, config_id,
            UPLL_DT_CANDIDATE, op, dmi,
            ckv_driver, ckv_unc, domain_type);
        if (result_code != UPLL_RC_SUCCESS) {
          DELETE_IF_NOT_NULL(req);
          DELETE_IF_NOT_NULL(nreq);
          DELETE_IF_NOT_NULL(ckv_driver);
          DELETE_IF_NOT_NULL(ck_main);
          DELETE_IF_NOT_NULL(ckv_unc);
          dmi->CloseCursor(dal_cursor_handle, true);
          return result_code;
        }
        affected_ctrlr_set->insert((const char *)ctrlr_dom.ctrlr);
        DELETE_IF_NOT_NULL(ck_main);
      } else if (((op == UNC_OP_CREATE) || (op == UNC_OP_DELETE) ||
            (op == UNC_OP_UPDATE)) && (tbl[tbl_indx] == CONVERTTBL)) {
        // Check before sending to vtn_gateport request to driver,
        // the spine domain exist in running or not
        if (op == UNC_OP_CREATE && config_mode == TC_CONFIG_VTN) {
          std::set<uint8_t *>::iterator it;
          uint8_t *vtnname = NULL;
          vtnname = reinterpret_cast<key_vtn *>(req->get_key())->vtn_name;
          it = vtn_list.find(vtnname);
          if (it == vtn_list.end()) {
            upll_rc_t local_rc = IsSpineDomainExistInRunning(req, dmi);
            if (local_rc != UPLL_RC_SUCCESS) {
              UPLL_LOG_DEBUG("Spine Domain is not exist in RUNNING %d",
                             local_rc);
              DELETE_IF_NOT_NULL(nreq);
              DELETE_IF_NOT_NULL(req);
              dmi->CloseCursor(dal_cursor_handle, true);
              return local_rc;
            }
            vtn_list.insert(vtnname);
          }
        }
        ck_main = NULL;
        ConfigKeyVal *ck_old = NULL;
        uint8_t vtn_rename = 0;
        GET_USER_DATA_FLAGS(req, vtn_rename);
        UPLL_LOG_TRACE("rename flag:%d", vtn_rename);
        result_code = DupConfigKeyVal(ck_main, req, CONVERTTBL);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("DupConfigKeyVal failed %d", result_code);
          DELETE_IF_NOT_NULL(nreq);
          DELETE_IF_NOT_NULL(req);
          dmi->CloseCursor(dal_cursor_handle, true);
          return result_code;
        }
        // Get controller & domain-id from ConfigKeyVal 'ck_main'
        ctrlr_dom.ctrlr = ctrlr_dom.domain = NULL;

        if (op == UNC_OP_UPDATE) {
          // Duplicates the running ConfigKeyVal 'nreq' into 'ck_old'
          result_code = DupConfigKeyVal(ck_old, nreq, CONVERTTBL);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_INFO("DupConfigKeyVal failed  for running ConfigKeyVal-%d",
                result_code);
            dmi->CloseCursor(dal_cursor_handle, true);
            DELETE_IF_NOT_NULL(req);
            DELETE_IF_NOT_NULL(nreq);
            DELETE_IF_NOT_NULL(ck_main);
            return result_code;
          }
        }
        domain_type = "(PF_LEAF)";
        bool not_send_to_drv = false;
        // vtn gateway port information converted to vbr portmap
        result_code = AdaptValToDriver(ck_main, ck_old, op,
            dt_type, keytype, dmi,
            not_send_to_drv, false);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_INFO("AdaptValToDriver failed for vBridge ConfigKeyVal-%d",
              result_code);
          dmi->CloseCursor(dal_cursor_handle, true);
          DELETE_IF_NOT_NULL(req);
          DELETE_IF_NOT_NULL(nreq);
          DELETE_IF_NOT_NULL(ck_main);
          DELETE_IF_NOT_NULL(ck_old);
          return result_code;
        }
        if (not_send_to_drv) {
          DELETE_IF_NOT_NULL(ck_main);
          DELETE_IF_NOT_NULL(ck_old);
          continue;
        }
        GET_USER_DATA_CTRLR_DOMAIN(ck_main, ctrlr_dom);
        VbrPortMapMoMgr *vbr_pm_mgr =
          reinterpret_cast<VbrPortMapMoMgr*>
          (const_cast<MoManager *>(GetMoManager(UNC_KT_VBR_PORTMAP)));

        if (!vbr_pm_mgr) {
          UPLL_LOG_DEBUG("Invalid Mgr");
          dmi->CloseCursor(dal_cursor_handle, true);
          DELETE_IF_NOT_NULL(req);
          DELETE_IF_NOT_NULL(nreq);
          DELETE_IF_NOT_NULL(ck_main);
          DELETE_IF_NOT_NULL(ck_old);
          return UPLL_RC_ERR_GENERIC;
        }

        if (op == UNC_OP_UPDATE) {
          void *main = GetVal(ck_main);
          void *val_nrec = (ck_old) ? GetVal(ck_old) : NULL;
          if (vbr_pm_mgr->CompareValidValue(main, val_nrec, false)) {
            DELETE_IF_NOT_NULL(ck_main);
            DELETE_IF_NOT_NULL(ck_old);
            continue;
          }
        }
        // If operation is UPDATE, append the running ConfigVal to the
        // candidate ConfigKeyVal using AppendCfgVal().
        // This provides the old and new value structures to the
        // driver in the case of UPDATE operation.

        // Contains controller specific key
        ConfigKeyVal *ckv_driver = NULL;
        result_code = vbr_pm_mgr->DupConfigKeyVal(ckv_driver, ck_main);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_INFO("DupConfigKeyVal failed for candidate ConfigKeyVal-%d",
              result_code);
          dmi->CloseCursor(dal_cursor_handle, true);
          DELETE_IF_NOT_NULL(req);
          DELETE_IF_NOT_NULL(nreq);
          DELETE_IF_NOT_NULL(ck_main);
          DELETE_IF_NOT_NULL(ck_old);
          return result_code;
        }
        if (op == UNC_OP_UPDATE) {
          ConfigVal *next_val = (ck_old->get_cfg_val())->DupVal();
          uint8_t rename_flag = 0;
          GET_USER_DATA_FLAGS(ck_old->get_cfg_val(), rename_flag)
            SET_USER_DATA_FLAGS(next_val, rename_flag)
            ckv_driver->AppendCfgVal(next_val);
        }
        // Contains unc specific key
        ConfigKeyVal *ckv_unc = NULL;
        result_code = vbr_pm_mgr->DupConfigKeyVal(ckv_unc, ck_main);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_INFO("DupConfigKeyVal failed for candidate ConfigKeyVal-%d",
              result_code);
          dmi->CloseCursor(dal_cursor_handle, true);
          DELETE_IF_NOT_NULL(req);
          DELETE_IF_NOT_NULL(nreq);
          DELETE_IF_NOT_NULL(ck_main);
          DELETE_IF_NOT_NULL(ckv_driver);
          DELETE_IF_NOT_NULL(ck_old);
          return result_code;
        }
        SET_USER_DATA_CTRLR_DOMAIN(ckv_unc, ctrlr_dom);
        DELETE_IF_NOT_NULL(ck_old);
        SET_USER_DATA_FLAGS(ckv_driver, vtn_rename);
        UPLL_LOG_TRACE("Rename flag:%d", vtn_rename);
        // Get the controller key for the renamed unc key.
        SET_USER_DATA_CTRLR_DOMAIN(ckv_driver, ctrlr_dom);
        ConfigKeyVal* ckv_gw_port = NULL;
        result_code = GetChildConfigKey(ckv_gw_port, ckv_driver);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("Error in GetChildConfigKey : %d", result_code);
          dmi->CloseCursor(dal_cursor_handle, true);
          DELETE_IF_NOT_NULL(req);
          DELETE_IF_NOT_NULL(nreq);
          DELETE_IF_NOT_NULL(ck_main);
          DELETE_IF_NOT_NULL(ckv_driver);
          DELETE_IF_NOT_NULL(ck_old);
          return result_code;
        }

        result_code = GetRenamedControllerKey(ckv_gw_port, dt_type,
            dmi, &ctrlr_dom);

        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG(" GetRenamedControllerKey failed err code(%d)",
              result_code);
          DELETE_IF_NOT_NULL(req);
          DELETE_IF_NOT_NULL(ckv_gw_port);
          DELETE_IF_NOT_NULL(nreq);
          DELETE_IF_NOT_NULL(ck_main);
          DELETE_IF_NOT_NULL(ckv_driver);
          dmi->CloseCursor(dal_cursor_handle, true);
          return result_code;
        }
        uuu::upll_strncpy(reinterpret_cast<key_vbr_portmap*>
            (ckv_driver->get_key())->vbr_key.vtn_key.vtn_name,
            reinterpret_cast<key_vtn*>(ckv_gw_port->get_key())->vtn_name,
            (kMaxLenVtnName+1));

        DELETE_IF_NOT_NULL(ckv_gw_port);
        UPLL_LOG_TRACE("Controller : %s; Domain : %s", ctrlr_dom.ctrlr,
            ctrlr_dom.domain);
        // Inserting the controller to Set
        affected_ctrlr_set->insert
          (string(reinterpret_cast<char *>(ctrlr_dom.ctrlr)));

        DELETE_IF_NOT_NULL(ck_main);
        result_code = tx_util->EnqueueRequest(session_id, config_id,
            UPLL_DT_CANDIDATE, op, dmi,
            ckv_driver, ckv_unc, domain_type);
        DELETE_IF_NOT_NULL(ck_main);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_ERROR("EnqueueRequest request failed");
          dmi->CloseCursor(dal_cursor_handle, true);
          DELETE_IF_NOT_NULL(nreq);
          DELETE_IF_NOT_NULL(req);
          DELETE_IF_NOT_NULL(ckv_driver);
          DELETE_IF_NOT_NULL(ckv_unc);
          return result_code;
        }
      }
    }
    DELETE_IF_NOT_NULL(nreq);
    DELETE_IF_NOT_NULL(req);
    if (dal_cursor_handle)
      dmi->CloseCursor(dal_cursor_handle, true);
    dal_cursor_handle = NULL;
    result_code = (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE)?
      UPLL_RC_SUCCESS:result_code;
  }
  return result_code;
}

upll_rc_t VtnMoMgr::AdaptValToDriver(ConfigKeyVal *ck_new,
                                     ConfigKeyVal *ck_old,
                                     unc_keytype_operation_t op,
                                     upll_keytype_datatype_t dt_type,
                                     unc_key_type_t keytype,
                                     DalDmlIntf *dmi,
                                     bool &not_send_to_drv,
                                     bool audit_update_phase) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  key_vbr_portmap *vbr_pm_key1 = ConfigKeyVal::Malloc<key_vbr_portmap>();
  pfcdrv_val_vbr_portmap *vbr_pm_val1 =
    ConfigKeyVal::Malloc<pfcdrv_val_vbr_portmap>();


  controller_domain ctrlr_dom;
  ctrlr_dom.ctrlr = ctrlr_dom.domain = NULL;
  GET_USER_DATA_CTRLR_DOMAIN(ck_new, ctrlr_dom);
  if (op == UNC_OP_UPDATE) {
    // Allocate memory for key_vbr_portmap
    key_vbr_portmap *vbr_pm_key2 = ConfigKeyVal::Malloc<key_vbr_portmap>();

    pfcdrv_val_vbr_portmap *vbr_pm_val2 =
      ConfigKeyVal::Malloc<pfcdrv_val_vbr_portmap>();

    uuu::upll_strncpy(vbr_pm_key2->vbr_key.vtn_key.vtn_name,
        (reinterpret_cast<key_vtn *>(ck_new->get_key()))->vtn_name,
        (kMaxLenVtnName+1));

    /* Commit case, vtn_gw_val2 is RUNNING record
     * Audit case, vtn_gw_val2 is AUDIT record */
    val_vtn_gateway_port *vtn_gw_val2 = reinterpret_cast
      <val_vtn_gateway_port *>(GetVal(ck_old));

    uuu::upll_strncpy(vbr_pm_val2->vbrpm.logical_port_id,
        vtn_gw_val2->logical_port_id,
        kMaxLenLogicalPortId+1);
    vbr_pm_val2->vbrpm.valid[UPLL_IDX_LOGICAL_PORT_ID_VBRPM] = UNC_VF_VALID;

    vbr_pm_val2->vbrpm.label = vtn_gw_val2->label;
    vbr_pm_val2->vbrpm.valid[UPLL_IDX_LABEL_VBRPM] = UNC_VF_VALID;

    vbr_pm_val2->vbrpm.label_type = UPLL_LABEL_TYPE_GW_VLAN;
    vbr_pm_val2->vbrpm.valid[UPLL_IDX_LABEL_TYPE_VBRPM] = UNC_VF_VALID;

    // Set controller_id and domain_id in vbr_portmap val
    uuu::upll_strncpy(reinterpret_cast<char *>
      (vbr_pm_val2->vbrpm.controller_id), ctrlr_dom.ctrlr, (kMaxLenCtrlrId+1));

    uuu::upll_strncpy(reinterpret_cast<char *>
        (vbr_pm_val2->vbrpm.domain_id), ctrlr_dom.domain, (kMaxLenDomainId+1));

    vbr_pm_val2->vbrpm.valid[UPLL_IDX_CONTROLLER_ID_VBRPM] = UNC_VF_VALID;
    vbr_pm_val2->vbrpm.valid[UPLL_IDX_DOMAIN_ID_VBRPM] = UNC_VF_VALID;
    vbr_pm_val2->valid[PFCDRV_IDX_VAL_VBR_PORTMAP] = UNC_VF_VALID;
    // vbr portmap RUNNING configkeyval
    ConfigVal *ck_oldval = new ConfigVal(IpctSt::kIpcStPfcdrvValVbrPortMap,
        vbr_pm_val2);
    ConfigKeyVal *ck_old1 = new ConfigKeyVal(UNC_KT_VBR_PORTMAP,
        IpctSt::kIpcStKeyVbrPortMap,
        vbr_pm_key2, ck_oldval);

    SET_USER_DATA_CTRLR_DOMAIN(ck_old1, ctrlr_dom);
    ck_old->ResetWith(ck_old1);
    DELETE_IF_NOT_NULL(ck_old1);
  }
  // CREATE/DELETE/UPDATE case
  uuu::upll_strncpy(vbr_pm_key1->vbr_key.vtn_key.vtn_name,
      (reinterpret_cast<key_vtn *>(ck_new->get_key()))->vtn_name,
      (kMaxLenVtnName+1));

  /* Commit case, vtn_gw_val1 is CANDIDATE record
   * Audit case , vtn_gw_val1 is RUNNING record */
  val_vtn_gateway_port *vtn_gw_val1 = reinterpret_cast
    <val_vtn_gateway_port *>(GetVal(ck_new));

  uuu::upll_strncpy(vbr_pm_val1->vbrpm.logical_port_id,
      vtn_gw_val1->logical_port_id,
      kMaxLenLogicalPortId+1);
  vbr_pm_val1->vbrpm.valid[UPLL_IDX_LOGICAL_PORT_ID_VBRPM] = UNC_VF_VALID;

  vbr_pm_val1->vbrpm.label = vtn_gw_val1->label;
  vbr_pm_val1->vbrpm.valid[UPLL_IDX_LABEL_VBRPM] = UNC_VF_VALID;

  vbr_pm_val1->vbrpm.label_type = UPLL_LABEL_TYPE_GW_VLAN;
  vbr_pm_val1->vbrpm.valid[UPLL_IDX_LABEL_TYPE_VBRPM] = UNC_VF_VALID;

  uuu::upll_strncpy(reinterpret_cast<char *>
      (vbr_pm_val1->vbrpm.controller_id), ctrlr_dom.ctrlr, (kMaxLenCtrlrId+1));

  uuu::upll_strncpy(reinterpret_cast<char *>
      (vbr_pm_val1->vbrpm.domain_id), ctrlr_dom.domain, (kMaxLenDomainId+1));

  vbr_pm_val1->vbrpm.valid[UPLL_IDX_CONTROLLER_ID_VBRPM] = UNC_VF_VALID;
  vbr_pm_val1->vbrpm.valid[UPLL_IDX_DOMAIN_ID_VBRPM] = UNC_VF_VALID;
  vbr_pm_val1->valid[PFCDRV_IDX_VAL_VBR_PORTMAP] = UNC_VF_VALID;

  // vbr portmap CANDIDATE configkeyval
  ConfigVal *ck_newval = new ConfigVal(IpctSt::kIpcStPfcdrvValVbrPortMap,
      vbr_pm_val1);
  ConfigKeyVal *ck_new1 = new ConfigKeyVal(UNC_KT_VBR_PORTMAP,
      IpctSt::kIpcStKeyVbrPortMap,
      vbr_pm_key1, ck_newval);


  SET_USER_DATA_CTRLR_DOMAIN(ck_new1, ctrlr_dom);
  ck_new->ResetWith(ck_new1);
  DELETE_IF_NOT_NULL(ck_new1);
  return result_code;
}

upll_rc_t VtnMoMgr::AuditUpdateController(unc_key_type_t keytype,
    const char *ctrlr_id,
    uint32_t session_id,
    uint32_t config_id,
    uuc::UpdateCtrlrPhase phase,
    DalDmlIntf *dmi,
    ConfigKeyVal **err_ckv,
    KTxCtrlrAffectedState *ctrlr_affected) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  DalResultCode db_result = uud::kDalRcSuccess;

  controller_domain_t ctrlr_dom;
  ctrlr_dom.ctrlr = NULL;
  ctrlr_dom.domain = NULL;

  ConfigKeyVal  *ckv_running = NULL;
  ConfigKeyVal  *ckv_audit = NULL;
  ConfigKeyVal  *ckv_drvr = NULL;
  ConfigKeyVal  *resp = NULL;
  DalCursor *cursor = NULL;
  bool invalid_attr = false;
  // Specifies true for audit update transaction
  // else false default
  const bool audit_update_phase = true;
  uint8_t *in_ctrlr = reinterpret_cast<uint8_t *>(const_cast<char *>(ctrlr_id));
  string vtn_name = "";

  unc_keytype_operation_t op = (phase == uuc::kUpllUcpCreate)?UNC_OP_CREATE:
    ((phase == uuc::kUpllUcpUpdate)?UNC_OP_UPDATE:
     ((phase == uuc::kUpllUcpDelete)?UNC_OP_DELETE:UNC_OP_INVALID));

  if (phase == uuc::kUpllUcpDelete2)
    return result_code;

  if (op == UNC_OP_INVALID) {
    UPLL_LOG_INFO("Invalid operation received-%d", op);
    // Not a valid operation
    return UPLL_RC_ERR_BAD_REQUEST;
  }

  /* retreives the delta of running and audit configuration */
  UPLL_LOG_TRACE("Operation is %d", op);

  bool auditdiff_with_flag = false;
  if ((op == UNC_OP_CREATE) || (op == UNC_OP_DELETE))
    auditdiff_with_flag = true;
  MoMgrTables tbl[] = {MAINTBL, CONVERTTBL};
  VbrPortMapMoMgr *vbr_pm_mgr = NULL;
  int ntbl =  sizeof(tbl)/ sizeof(tbl[0]);
  for (int tbl_indx = 0; tbl_indx < ntbl; tbl_indx++) {
    if (tbl[tbl_indx] == MAINTBL) {
      // Decides whether to retrieve from controller table or main table
      tbl[tbl_indx] = (op != UNC_OP_UPDATE)?CTRLRTBL:MAINTBL;
    }
    if (tbl[tbl_indx] == CONVERTTBL) {
      vbr_pm_mgr = reinterpret_cast<VbrPortMapMoMgr *>
        (const_cast<MoManager*>(GetMoManager(UNC_KT_VBR_PORTMAP)));
      if (!vbr_pm_mgr) {
        UPLL_LOG_DEBUG("Invalid Mgr");
        return UPLL_RC_ERR_GENERIC;
      }
    }
    // Update operation not supported for UNC_KT_VTN
    if (((op == UNC_OP_UPDATE) && (keytype == UNC_KT_VTN)) &&
        (tbl[tbl_indx] == MAINTBL)) {
      UPLL_LOG_INFO("UPDATE to VTN is not supported at driver. so return..");
      continue;
    }
    // Get CREATE, DELETE and UPDATE object information based on the table 'tbl'
    // between running configuration and audit configuration
    // where 'ckv_running' parameter contains the running information and
    // the 'ckv_audit' parameter contains the audit configuration.
    result_code = DiffConfigDB(UPLL_DT_RUNNING, UPLL_DT_AUDIT, op,
        ckv_running, ckv_audit,
        &cursor, dmi, in_ctrlr, TC_CONFIG_GLOBAL, vtn_name,
        tbl[tbl_indx], true, auditdiff_with_flag);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("DiffConfigDB failed - %d", result_code);
      return result_code;
    }
    // Iterate loop to get next record
    while (uud::kDalRcSuccess == (db_result = dmi->GetNextRecord(cursor)) &&
        ((result_code = ContinueAuditProcess()) == UPLL_RC_SUCCESS)) {
      UPLL_LOG_TRACE("Diff Running Record for Keytype: Operation:  is %d%d\n%s",
          keytype, op, ckv_running->ToStrAll().c_str());
      /* ignore records of another controller for create and update operation */
      if (phase != uuc::kUpllUcpDelete) {
        uint8_t *db_ctrlr = NULL;
        GET_USER_DATA_CTRLR(ckv_running, db_ctrlr);
        if ((!db_ctrlr) ||
            (db_ctrlr && strncmp(reinterpret_cast<const char *>(db_ctrlr),
                                 reinterpret_cast<const char *>(ctrlr_id),
                                 strlen(reinterpret_cast<const char *>
                                 (ctrlr_id)) + 1))) {
          continue;
        }
      }
      // Case1: commit(del) - Check in AUDIT since no info exists in RUNNING
      // Case2: Commit(Cr/upd) - Check in RUNNING always
      upll_keytype_datatype_t dt_type = (op == UNC_OP_DELETE)?
        UPLL_DT_AUDIT : UPLL_DT_RUNNING;
      if ((tbl[tbl_indx] == CTRLRTBL) &&
          (op == UNC_OP_CREATE || op == UNC_OP_DELETE)) {
        // To fetch the records from the running which
        // differ with the audit records
        // If audit_update_phase flag is true and operation is UNC_OP_UPDATE
        // append the audit ConfigVal into running ConfigKeyVal
        // 'ckv_drvr' contains both running and audit ConfigVal
        result_code =  GetDiffRecord(ckv_running, ckv_audit, phase,
            tbl[tbl_indx],
            ckv_drvr, dmi, invalid_attr,
            audit_update_phase);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_INFO("GetDiffRecord failed err code is %d", result_code);
          DELETE_IF_NOT_NULL(ckv_running);
          DELETE_IF_NOT_NULL(ckv_audit);
          DELETE_IF_NOT_NULL(ckv_drvr);
          dmi->CloseCursor(cursor, true);
          return result_code;
        }
        if (invalid_attr) {
          DELETE_IF_NOT_NULL(ckv_drvr);
          // Assuming that the diff found only in ConfigStatus
          // Setting the value as OnlyCSDiff in the out parameter ctrlr_affected
          // The value Configdiff should be given more priority than the value
          // only cs.
          // So, if the out parameter ctrlr_affected has
          // already value as configdiff
          // then dont change the value
          if (*ctrlr_affected != uuc::kCtrlrAffectedConfigDiff) {
            UPLL_LOG_INFO("Setting the ctrlr_affected to OnlyCSDiff for KT %u",
                keytype);
            *ctrlr_affected = uuc::kCtrlrAffectedOnlyCSDiff;
          }
          continue;
        }
        // Get controller & domain-id from ConfigKeyVal 'ckv_drvr'
        GET_USER_DATA_CTRLR_DOMAIN(ckv_drvr, ctrlr_dom);
        // Duplicates the running ConfigKeyVal 'ckv_drvr' into 'resp'
        // 'resp' is used for AdaptValToVtnService().
        result_code = DupConfigKeyVal(resp, ckv_drvr, MAINTBL);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG(" DupConfigKeyVal failed err code(%d)",
              result_code);
          DELETE_IF_NOT_NULL(ckv_running);
          DELETE_IF_NOT_NULL(ckv_audit);
          DELETE_IF_NOT_NULL(ckv_drvr);
          dmi->CloseCursor(cursor, true);
          return result_code;
        }
        // resp will have State structure, next cfg val, which is not required
        // for further processing, since during update phase old val struct
        // will come as next cfg val, it might cause issue while
        // accessing that memory
        //
        if (resp->get_cfg_val() != NULL) {
          resp->get_cfg_val()->DeleteNextCfgVal();
        }

        // Get the controller key for the renamed unc key.
        result_code = GetRenamedControllerKey(ckv_drvr, dt_type,
            dmi, &ctrlr_dom);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG(" GetRenamedControllerKey failed err code(%d)",
              result_code);
          DELETE_IF_NOT_NULL(ckv_running);
          DELETE_IF_NOT_NULL(ckv_audit);
          DELETE_IF_NOT_NULL(ckv_drvr);
          DELETE_IF_NOT_NULL(resp);
          dmi->CloseCursor(cursor, true);
          return result_code;
        }

        // Returns when controller-id and domain-id is not present.
        if (ctrlr_dom.ctrlr != NULL) {
          bool domain = false;
          KEYTYPE_WITHOUT_DOMAIN(keytype, domain);
          if (!domain) {
            if (NULL == ctrlr_dom.domain) {
              UPLL_LOG_INFO(" domain is NULL");
              DELETE_IF_NOT_NULL(ckv_running);
              DELETE_IF_NOT_NULL(ckv_audit);
              DELETE_IF_NOT_NULL(ckv_drvr);
              DELETE_IF_NOT_NULL(resp);
              dmi->CloseCursor(cursor, true);
              return UPLL_RC_ERR_GENERIC;
            }
          }
        } else {
          UPLL_LOG_DEBUG("Controller Id is NULL");
          DELETE_IF_NOT_NULL(ckv_running);
          DELETE_IF_NOT_NULL(ckv_audit);
          DELETE_IF_NOT_NULL(ckv_drvr);
          DELETE_IF_NOT_NULL(resp);
          dmi->CloseCursor(cursor, true);
          return UPLL_RC_ERR_GENERIC;
        }
      } else if ((tbl[tbl_indx] == CONVERTTBL)) {
        ConfigKeyVal *ck_old = NULL;
        ckv_drvr = NULL;
        // Duplicates the RUNNING ConfigKeyVal 'ckv_running' into 'ck_main'
        result_code = DupConfigKeyVal(ckv_drvr, ckv_running, CONVERTTBL);
        if (result_code != UPLL_RC_SUCCESS) {
          DELETE_IF_NOT_NULL(ckv_running);
          DELETE_IF_NOT_NULL(ckv_audit);
          dmi->CloseCursor(cursor, true);
          return result_code;
        }
        uint8_t vtn_rename = 0;
        GET_USER_DATA_FLAGS(ckv_running, vtn_rename);
        if (op == UNC_OP_UPDATE) {
          // Duplicates the AUDIT ConfigKeyVal 'ckv_audit' into 'ck_old'
          result_code = DupConfigKeyVal(ck_old, ckv_audit, CONVERTTBL);
          if (result_code != UPLL_RC_SUCCESS) {
            DELETE_IF_NOT_NULL(ckv_running);
            DELETE_IF_NOT_NULL(ckv_audit);
            dmi->CloseCursor(cursor, true);
            DELETE_IF_NOT_NULL(ckv_drvr);
            return result_code;
          }
        }
        bool not_send_to_drv = false;
        result_code = AdaptValToDriver(ckv_drvr, ck_old, op,
            dt_type, keytype, dmi,
            not_send_to_drv, false);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_INFO("AdaptValToDriver failed for vBridge ConfigKeyVal-%d",
              result_code);
          DELETE_IF_NOT_NULL(ckv_running);
          DELETE_IF_NOT_NULL(ckv_audit);
          dmi->CloseCursor(cursor, true);
          DELETE_IF_NOT_NULL(ckv_drvr);
          DELETE_IF_NOT_NULL(ck_old);
          return result_code;
        }

        // Get controller & domain-id from ConfigKeyVal 'ckv_drvr'
        GET_USER_DATA_CTRLR_DOMAIN(ckv_drvr, ctrlr_dom);
        if (op == UNC_OP_UPDATE) {
          void *main = GetVal(ckv_drvr);
          void *val_nrec = (ck_old) ? GetVal(ck_old) : NULL;
          if (vbr_pm_mgr->CompareValidValue(main, val_nrec, false)) {
            DELETE_IF_NOT_NULL(ckv_drvr);
            DELETE_IF_NOT_NULL(ck_old);
            continue;
          }
        }
        // Duplicates the running ConfigKeyVal 'ckv_drvr' into 'resp'
        // 'resp' is used for AdaptValToVtnService().
        result_code = vbr_pm_mgr->DupConfigKeyVal(resp, ckv_running,
                                                  CONVERTTBL);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_ERROR("DupConfigKeyVal failed err code(%d)",
                         result_code);
          DELETE_IF_NOT_NULL(ckv_running);
          DELETE_IF_NOT_NULL(ckv_audit);
          DELETE_IF_NOT_NULL(ckv_drvr);
          DELETE_IF_NOT_NULL(ck_old);
          dmi->CloseCursor(cursor, true);
          return result_code;
        }

        if (op == UNC_OP_UPDATE) {
          ConfigVal *next_val = (ck_old->get_cfg_val())->DupVal();
          uint8_t rename_flag = 0;
          GET_USER_DATA_FLAGS(ck_old->get_cfg_val(), rename_flag)
            SET_USER_DATA_FLAGS(next_val, rename_flag)
            ckv_drvr->AppendCfgVal(next_val);
        }
        DELETE_IF_NOT_NULL(ck_old);

        SET_USER_DATA_FLAGS(ckv_drvr, vtn_rename);
        UPLL_LOG_TRACE("Rename flag:%d", vtn_rename);
        ConfigKeyVal* ckv_gw_port = NULL;
        result_code = GetChildConfigKey(ckv_gw_port, ckv_drvr);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("Error in GetChildConfigKey : %d", result_code);
          DELETE_IF_NOT_NULL(ckv_running);
          DELETE_IF_NOT_NULL(ckv_audit);
          dmi->CloseCursor(cursor, true);
          DELETE_IF_NOT_NULL(ckv_drvr);
          DELETE_IF_NOT_NULL(resp);
          return result_code;
        }
        result_code = GetRenamedControllerKey(ckv_gw_port, dt_type,
                dmi, &ctrlr_dom);
        if (result_code != UPLL_RC_SUCCESS &&
            result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
          UPLL_LOG_DEBUG(" GetRenamedControllerKey failed err code(%d)",
              result_code);
          DELETE_IF_NOT_NULL(ckv_running);
          DELETE_IF_NOT_NULL(ckv_audit);
          dmi->CloseCursor(cursor, true);
          DELETE_IF_NOT_NULL(ckv_drvr);
          DELETE_IF_NOT_NULL(resp);
          DELETE_IF_NOT_NULL(ckv_gw_port);
          return result_code;
        }
        uuu::upll_strncpy(reinterpret_cast<key_vbr_portmap*>
            (ckv_drvr->get_key())->vbr_key.vtn_key.vtn_name,
            reinterpret_cast<key_vtn*>(ckv_gw_port->get_key())->vtn_name,
            (kMaxLenVtnName+1));

        DELETE_IF_NOT_NULL(ckv_gw_port);
      }
      UPLL_LOG_TRACE("Controller: %s; Domain: %s", ctrlr_dom.ctrlr,
                     ctrlr_dom.domain);

      VbrMoMgr *vbr_mgr = reinterpret_cast<VbrMoMgr *>
            (const_cast<MoManager *>(GetMoManager(UNC_KT_VBRIDGE)));

      if (tbl[tbl_indx] != CONVERTTBL) {
        // VTN ctrlr table CREATE/UPDATE/DELETE operation, if
        // the given VTN is exists in vBridge CONVERTTBL,
        // then send domain_id with (PE_LEAF) prefix
        ConfigKeyVal *temp_key = NULL;
        result_code = vbr_mgr->GetChildConvertConfigKey(temp_key, resp);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_INFO("GetChildConvertConfigKey failed %d", result_code);
          DELETE_IF_NOT_NULL(ckv_running);
          DELETE_IF_NOT_NULL(ckv_audit);
          DELETE_IF_NOT_NULL(ckv_drvr);
          DELETE_IF_NOT_NULL(resp);
          dmi->CloseCursor(cursor, true);
          return result_code;
        }
        DbSubOp dbop1 = { kOpReadExist, kOpMatchCtrlr | kOpMatchDomain,
          kOpInOutNone };
        result_code = vbr_mgr->UpdateConfigDB(temp_key, dt_type, UNC_OP_READ,
                                              dmi, &dbop1, CONVERTTBL);
        DELETE_IF_NOT_NULL(temp_key);
      } else {
        // for VTN-CONVERTTBL, need to prefix domain_id with PF_LEAF
        result_code = UPLL_RC_ERR_INSTANCE_EXISTS;
      }

      string domain_type;
      if (result_code == UPLL_RC_ERR_INSTANCE_EXISTS) {
        if (ctrlr_dom.domain != NULL)
          domain_type = (string("(PF_LEAF)") +
                         reinterpret_cast<char*>(ctrlr_dom.domain));
      } else {
        if (ctrlr_dom.domain != NULL)
          domain_type = reinterpret_cast<char *>(ctrlr_dom.domain);
      }
      IpcResponse ipc_response;
      memset(&ipc_response, 0, sizeof(IpcResponse));
      IpcRequest ipc_req;
      memset(&ipc_req, 0, sizeof(IpcRequest));
      ipc_req.header.clnt_sess_id = session_id;
      ipc_req.header.config_id = config_id;
      ipc_req.header.operation = op;
      ipc_req.header.datatype = UPLL_DT_CANDIDATE;
      ipc_req.ckv_data = ckv_drvr;
      char domain_name[KtUtil::kDrvDomainNameLenWith0];
      uuu::upll_strncpy(domain_name, domain_type.c_str(),
                        KtUtil::kDrvDomainNameLenWith0);
      // To populate IPC request.
      if (!IpcUtil::SendReqToDriver((const char *)ctrlr_dom.ctrlr,
            domain_name, PFCDRIVER_SERVICE_NAME,
            PFCDRIVER_SVID_LOGICAL, &ipc_req,
            true, &ipc_response)) {
        UPLL_LOG_INFO("Request to driver for Key %d for controller %s failed ",
            ckv_drvr->get_key_type(),
            reinterpret_cast<char *>(ctrlr_dom.ctrlr));
        DELETE_IF_NOT_NULL(resp);
        DELETE_IF_NOT_NULL(ckv_running);
        DELETE_IF_NOT_NULL(ckv_audit);
        DELETE_IF_NOT_NULL(ckv_drvr);
        DELETE_IF_NOT_NULL(ipc_response.ckv_data);
        dmi->CloseCursor(cursor, true);
        return ipc_response.header.result_code;
      }
      /* VlanmapOnBoundary: Added vlanmap check */
      UPLL_LOG_DEBUG("Result code from driver %d",
          ipc_response.header.result_code);
      if  (ipc_response.header.result_code != UPLL_RC_SUCCESS) {
        if (ckv_drvr->get_key_type() == UNC_KT_VBR_PORTMAP) {
          result_code = vbr_pm_mgr->GetRenamedUncKey(ipc_response.ckv_data,
                                              dt_type, dmi, ctrlr_dom.ctrlr);
          if (result_code != UPLL_RC_SUCCESS &&
              result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
            UPLL_LOG_DEBUG("GetRenamedUncKey failed %d\n", result_code);
            DELETE_IF_NOT_NULL(ipc_response.ckv_data);
            DELETE_IF_NOT_NULL(ckv_running);
            DELETE_IF_NOT_NULL(ckv_audit);
            DELETE_IF_NOT_NULL(ckv_drvr);
            DELETE_IF_NOT_NULL(resp);
            dmi->CloseCursor(cursor, true);
            return result_code;
          }
          result_code = vbr_pm_mgr->TranslateVbrPortMapError(err_ckv,
                         ckv_running, ipc_response.ckv_data, dmi, dt_type);
        } else {
          result_code = AdaptValToVtnService(resp, ADAPT_ONE);
        }
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("AdaptValToVtnService failed %d\n", result_code);
          DELETE_IF_NOT_NULL(ipc_response.ckv_data);
          DELETE_IF_NOT_NULL(ckv_running);
          DELETE_IF_NOT_NULL(ckv_audit);
          DELETE_IF_NOT_NULL(ckv_drvr);
          DELETE_IF_NOT_NULL(resp);
          dmi->CloseCursor(cursor, true);
          return result_code;
        }
        if (phase == uuc::kUpllUcpDelete) {
          DELETE_IF_NOT_NULL(ipc_response.ckv_data);
          DELETE_IF_NOT_NULL(ckv_running);
          DELETE_IF_NOT_NULL(ckv_audit);
          DELETE_IF_NOT_NULL(ckv_drvr);
          DELETE_IF_NOT_NULL(resp);
          dmi->CloseCursor(cursor, true);
          return ipc_response.header.result_code;
        } else {
          UPLL_LOG_DEBUG("driver return failure err_code is %d",
              ipc_response.header.result_code);
          ConfigKeyVal *ctrlr_key = NULL;
          // To get duplicate ConfigKeyVal from running ConfigKeyVal,
          // either from CTRLTBL or MAINTBL based on the key type.
          if (CTRLRTBL == (GET_TABLE_TYPE(keytype, tbl[tbl_indx]))) {
            result_code = DupConfigKeyVal(ctrlr_key, ckv_running, CTRLRTBL);
          } else  if (CONVERTTBL == tbl[tbl_indx]) {
            result_code = vbr_pm_mgr->DupConfigKeyVal(ctrlr_key, ckv_running);
          }
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_INFO("DupConfigKeyVal failed");
            DELETE_IF_NOT_NULL(ipc_response.ckv_data);
            DELETE_IF_NOT_NULL(ckv_running);
            DELETE_IF_NOT_NULL(ckv_audit);
            DELETE_IF_NOT_NULL(ckv_drvr);
            DELETE_IF_NOT_NULL(resp);
            dmi->CloseCursor(cursor, true);
            if (*err_ckv != NULL) {
              UPLL_LOG_INFO("err_ckv: %s", (*err_ckv)->ToStrAll().c_str());
              *err_ckv = NULL;
            }
            return result_code;
          }
          // To update controller ConfigStatus as
          // UNC_CS_INVALID for the error record.
          if (ckv_drvr->get_key_type() == UNC_KT_VBR_PORTMAP) {
            result_code = vbr_mgr->UpdateUvbrConfigStatusAuditUpdate(phase,
                                            ipc_response.ckv_data, dmi);
          } else if ((keytype == UNC_KT_VTN) && ((tbl[tbl_indx] == CTRLRTBL))) {
            result_code = UpdateCtrlrConfigStatus(UNC_CS_INVALID,
                phase, ctrlr_key);
          } else if (tbl[tbl_indx] == CONVERTTBL) {
          }
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_INFO("UpdateAuditConfigStatus failed");
            DELETE_IF_NOT_NULL(ipc_response.ckv_data);
            DELETE_IF_NOT_NULL(ckv_running);
            DELETE_IF_NOT_NULL(ckv_audit);
            DELETE_IF_NOT_NULL(ckv_drvr);
            DELETE_IF_NOT_NULL(resp);
            dmi->CloseCursor(cursor, true);
            DELETE_IF_NOT_NULL(ctrlr_key);
            if (*err_ckv != NULL) {
              UPLL_LOG_INFO("err_ckv: %s", (*err_ckv)->ToStrAll().c_str());
              *err_ckv = NULL;
            }
            return result_code;
          }
          // To update configuration status.
          result_code = UpdateConfigDB(ctrlr_key, UPLL_DT_RUNNING,
              UNC_OP_UPDATE,
              dmi, TC_CONFIG_GLOBAL, vtn_name, tbl[tbl_indx]);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG(
                "UpdateConfigDB failed for ipc response ckv err_code %d",
                result_code);
            DELETE_IF_NOT_NULL(ipc_response.ckv_data);
            DELETE_IF_NOT_NULL(ckv_running);
            DELETE_IF_NOT_NULL(ckv_audit);
            DELETE_IF_NOT_NULL(ckv_drvr);
            DELETE_IF_NOT_NULL(resp);
            dmi->CloseCursor(cursor, true);
            DELETE_IF_NOT_NULL(ctrlr_key);
            if (*err_ckv != NULL) {
              UPLL_LOG_INFO("err_ckv: %s", (*err_ckv)->ToStrAll().c_str());
              *err_ckv = NULL;
            }
            return result_code;
          }
          DELETE_IF_NOT_NULL(ctrlr_key);
          if (CTRLRTBL == (GET_TABLE_TYPE(keytype, tbl[tbl_indx]))) {
            result_code = SetConsolidatedStatus(resp, dmi);
            if (result_code != UPLL_RC_SUCCESS) {
              UPLL_LOG_INFO(
                  "SetConsolidatedStatus failed for ipc response ckv"
                  "err_code %d", result_code);
              DELETE_IF_NOT_NULL(ipc_response.ckv_data);
              DELETE_IF_NOT_NULL(ckv_running);
              DELETE_IF_NOT_NULL(ckv_audit);
              DELETE_IF_NOT_NULL(ckv_drvr);
              DELETE_IF_NOT_NULL(resp);
              dmi->CloseCursor(cursor, true);
              if (*err_ckv != NULL) {
                UPLL_LOG_INFO("err_ckv: %s", (*err_ckv)->ToStrAll().c_str());
                *err_ckv = NULL;
              }
              return result_code;
            }
          }
          DELETE_IF_NOT_NULL(ipc_response.ckv_data);
          DELETE_IF_NOT_NULL(ckv_running);
          DELETE_IF_NOT_NULL(ckv_audit);
          DELETE_IF_NOT_NULL(ckv_drvr);
          DELETE_IF_NOT_NULL(resp);
          dmi->CloseCursor(cursor, true);
          return ipc_response.header.result_code;
        }
      }
      DELETE_IF_NOT_NULL(ckv_drvr);
      DELETE_IF_NOT_NULL(ipc_response.ckv_data);
      //  *ctrlr_affected = true;

      if (*ctrlr_affected == uuc::kCtrlrAffectedOnlyCSDiff) {
        UPLL_LOG_INFO("Reset ctrlr state from OnlyCSDiff to ConfigDiff  KT %u",
            keytype);
      }
      UPLL_LOG_DEBUG("Setting the ctrlr_affected to ConfigDiff, KT %u",
          keytype);
      *ctrlr_affected = uuc::kCtrlrAffectedConfigDiff;
      DELETE_IF_NOT_NULL(resp);
    }
    if (cursor) {
      dmi->CloseCursor(cursor, true);
      cursor = NULL;
    }
    if (uud::kDalRcSuccess != db_result) {
      UPLL_LOG_DEBUG("GetNextRecord from database failed  - %d", db_result);
      result_code =  DalToUpllResCode(db_result);
    }
    DELETE_IF_NOT_NULL(ckv_running);
    DELETE_IF_NOT_NULL(ckv_audit);
    result_code = (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE)
      ? UPLL_RC_SUCCESS : result_code;
  }
  return result_code;
}

upll_rc_t VtnMoMgr::IsSpineDomainExistInRunning(ConfigKeyVal *ck_main,
                                                DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  // check given vtn is exist in convert_vtunnel table
  key_convert_vtunnel *convert_vtunnel_key =
    ConfigKeyVal::Malloc<key_convert_vtunnel>();
  uuu::upll_strncpy(convert_vtunnel_key->vtn_key.vtn_name,
      reinterpret_cast<key_convert_vtunnel *>
      (ck_main->get_key())->vtn_key.vtn_name, (kMaxLenVtnName+1));
  ConfigKeyVal *okey = new ConfigKeyVal(UNC_KT_VTUNNEL,
      IpctSt::kIpcStKeyConvertVtunnel,
      convert_vtunnel_key, NULL);
  MoMgrImpl *vtunnel_mgr = reinterpret_cast<MoMgrImpl*>(const_cast<MoManager *>
      (GetMoManager(UNC_KT_VTUNNEL)));

  DbSubOp dbop = {kOpReadSingle, kOpMatchNone, kOpInOutCtrlr|kOpInOutDomain};
  result_code = vtunnel_mgr->ReadConfigDB(okey, UPLL_DT_CANDIDATE,
      UNC_OP_READ, dbop, dmi, CONVERTTBL);

  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("UpdateConfigDB failed- %d", result_code);
    DELETE_IF_NOT_NULL(okey);
    return result_code;
  } else {
    controller_domain_t ctr_dom;
    memset(&ctr_dom, 0, sizeof(controller_domain_t));
    GET_USER_DATA_CTRLR_DOMAIN(okey, ctr_dom);
    if (!ctr_dom.ctrlr || !ctr_dom.domain) {
      DELETE_IF_NOT_NULL(okey);
      UPLL_LOG_ERROR("Unable to get convert vtunnel ctrlr/domain");
      return UPLL_RC_ERR_GENERIC;
    }
    MoMgrImpl *spine_mgr = reinterpret_cast<MoMgrImpl*>(const_cast<MoManager *>
        (GetMoManager(UNC_KT_UNW_SPINE_DOMAIN)));
    ConfigKeyVal *spine_okey = NULL;
    result_code = spine_mgr->GetChildConfigKey(spine_okey, NULL);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("DupConfigKeyVal failed error %d", result_code);
      DELETE_IF_NOT_NULL(okey);
      return result_code;
    }
    // fill the val structure with given ctrlr, domain
    val_unw_spdom_ext *val_unw_sd_ext =
        ConfigKeyVal::Malloc<val_unw_spdom_ext>();

    val_unw_sd_ext->val_unw_spine_dom.valid
        [UPLL_IDX_SPINE_DOMAIN_ID_UNWS] = UNC_VF_VALID;
    val_unw_sd_ext->val_unw_spine_dom.valid[
        UPLL_IDX_SPINE_CONTROLLER_ID_UNWS] = UNC_VF_VALID;
    uuu::upll_strncpy(val_unw_sd_ext->val_unw_spine_dom.spine_controller_id,
                      ctr_dom.ctrlr, kMaxLenCtrlrId+1);
    uuu::upll_strncpy(val_unw_sd_ext->val_unw_spine_dom.spine_domain_id,
                      ctr_dom.domain, kMaxLenDomainId+1);
    spine_okey->AppendCfgVal(IpctSt::kIpctStValUnwSpineDomain_Ext,
                             val_unw_sd_ext);

    // If the VTN exist in convert vtunnel table, get spine controller/domain
    // and check the spine id exist in RUNNING
    dbop.inoutop = kOpInOutNone;

    result_code = spine_mgr->ReadConfigDB(spine_okey, UPLL_DT_RUNNING,
        UNC_OP_READ, dbop, dmi, MAINTBL);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("UpdateConfigDB failed- %d", result_code);
      DELETE_IF_NOT_NULL(okey);
      DELETE_IF_NOT_NULL(spine_okey);
      return result_code;
    }
    DELETE_IF_NOT_NULL(okey);
    DELETE_IF_NOT_NULL(spine_okey);
    return UPLL_RC_SUCCESS;
  }
}

upll_rc_t VtnMoMgr::ResetGWPortStatus(ConfigKeyVal *ckey, DalDmlIntf *dmi,
          uint8_t oper_status, const char *logical_port_id, bool is_reconect) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal* ckv_gw_port = NULL;
  result_code = GetChildConfigKey(ckv_gw_port, ckey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Error in GetChildConfigKey : %d", result_code);
    return result_code;
  }
  controller_domain_t ctrlr_dom = {NULL, NULL};
  GET_USER_DATA_CTRLR_DOMAIN(ckey, ctrlr_dom);
  SET_USER_DATA_CTRLR_DOMAIN(ckv_gw_port, ctrlr_dom);
  ConfigVal *cv_val = NULL;
  AllocVal(cv_val, UPLL_DT_STATE, CONVERTTBL);
  ckv_gw_port->SetCfgVal(cv_val);
  if (logical_port_id != NULL) {
    val_vtn_gateway_port *val = reinterpret_cast<val_vtn_gateway_port*>
                                (GetVal(ckv_gw_port));
    uuu::upll_strncpy(val->logical_port_id, logical_port_id,
                     (kMaxLenLogicalPortId+1));
    val->valid[0] = UNC_VF_VALID;
  }

  DbSubOp dbop = {kOpReadMultiple, kOpMatchCtrlr|kOpMatchDomain,
                  kOpInOutCtrlr|kOpInOutDomain};
  result_code = ReadConfigDB(ckv_gw_port, UPLL_DT_STATE, UNC_OP_READ, dbop,
                             dmi, CONVERTTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    DELETE_IF_NOT_NULL(ckv_gw_port);
    if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      result_code = UPLL_RC_SUCCESS;
    } else {
      UPLL_LOG_ERROR("Error in ReadConfigDB : %d", result_code);
    }
    return result_code;
  }
  ConfigKeyVal *tmp_ckv_gw = ckv_gw_port;
  while (ckv_gw_port) {
    val_vtn_gateway_port_t *gw_val = reinterpret_cast<val_vtn_gateway_port_t*>
                                 (GetVal(ckv_gw_port));
    if (!gw_val) {
      UPLL_LOG_DEBUG("val is empty");
      DELETE_IF_NOT_NULL(ckv_gw_port);
      return UPLL_RC_ERR_GENERIC;
    }
    val_db_vtn_st* vtn_st = reinterpret_cast<val_db_vtn_st*>
                            (GetStateVal(ckv_gw_port));
    if (!vtn_st) {
      UPLL_LOG_DEBUG("st_val is empty");
      DELETE_IF_NOT_NULL(ckv_gw_port);
      return UPLL_RC_ERR_GENERIC;
    }
    if (gw_val->valid[2] == UNC_VF_VALID) {
      uint8_t state(UPPL_LOGICAL_PORT_OPER_UNKNOWN);
      if (is_reconect) {
        // controller reconnect case
        uint8_t *tmp_logical_port_id = gw_val->logical_port_id, *ctrlr = NULL;
        GET_USER_DATA_CTRLR(ckey, ctrlr);
        bool result(false);
        result = uuc::CtrlrMgr::GetInstance()->GetLogicalPortSt(
                 reinterpret_cast<const char*>(ctrlr),
                 reinterpret_cast<const char*>(tmp_logical_port_id),
                 state);
        if (result == false) {
        /* port is already set to UNKNOWN. So return */
          ckv_gw_port = ckv_gw_port->get_next_cfg_key_val();
          continue;
        }
        vtn_st->vtn_val_st.oper_status = UPLL_OPER_STATUS_UP;
         if (state == UPPL_LOGICAL_PORT_OPER_DOWN) {
          /* Set PORT_FAULT flag */
          vtn_st->vtn_val_st.oper_status = UPLL_OPER_STATUS_DOWN;
        }
      } else {
        switch (oper_status) {
          case UPPL_LOGICAL_PORT_OPER_DOWN:
            vtn_st->vtn_val_st.oper_status = UPLL_OPER_STATUS_DOWN;
          break;
          case UPPL_LOGICAL_PORT_OPER_UP:
            vtn_st->vtn_val_st.oper_status = UPLL_OPER_STATUS_UP;
          break;
          case UPPL_LOGICAL_PORT_OPER_UNKNOWN:
            vtn_st->vtn_val_st.oper_status = UPLL_OPER_STATUS_UNKNOWN;
          break;
          default:
          break;
        }
      }
      vtn_st->vtn_val_st.valid[0] = UNC_VF_VALID;
      DbSubOp dbop_update = {kOpNotRead,
                             kOpMatchCtrlr|kOpMatchDomain, kOpInOutNone};
      result_code = UpdateConfigDB(ckv_gw_port, UPLL_DT_STATE, UNC_OP_UPDATE,
                         dmi, &dbop_update, TC_CONFIG_GLOBAL,
                         "", CONVERTTBL);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("UpdateConfigDB failed : %d", result_code);
        DELETE_IF_NOT_NULL(ckv_gw_port);
        return result_code;
      }
    }
    ckv_gw_port = ckv_gw_port->get_next_cfg_key_val();
  }
  DELETE_IF_NOT_NULL(tmp_ckv_gw);
  return result_code;
}
upll_rc_t VtnMoMgr::VtnExhaustionHandler(uint8_t* ctrlr_id, uint8_t* domain_id,
                                key_vtn vtn_key,
                                bool alarm_asserted, uuc::UpllDbConnMgr* db_con,
                                uuc::ConfigLock* cfg_lock) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  if (!ctrlr_id || !domain_id)
    return UPLL_RC_ERR_GENERIC;
  state_notification notfn = (alarm_asserted == true) ?
              kVtnExhaustion : kVtnExhaustionReset;
  ConfigKeyVal* ck_vtn = NULL;
  result_code = GetChildConfigKey(ck_vtn, NULL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetChildConfigKey failed");
    return UPLL_RC_ERR_GENERIC;
  }
  controller_domain ctrlr_dom = {ctrlr_id, domain_id};
  SET_USER_DATA_CTRLR_DOMAIN(ck_vtn, ctrlr_dom);

  key_vtn *tmp_vtn_key = reinterpret_cast<key_vtn*>(ck_vtn->get_key());
  uuu::upll_strncpy(tmp_vtn_key->vtn_name,
                    reinterpret_cast<char*>(vtn_key.vtn_name),
                    (kMaxLenVtnName+1));
  uuc::DalOdbcMgr *dmi = db_con->GetAlarmRwConn();
  if (dmi == NULL) {
      UPLL_LOG_ERROR("Error in GetAlarmRwConn");
      DELETE_IF_NOT_NULL(ck_vtn);
      return UPLL_RC_ERR_GENERIC;
    }

  result_code = GetRenamedUncKey(ck_vtn, UPLL_DT_RUNNING,
                                           dmi, ctrlr_dom.ctrlr);
  db_con->DalTxClose(dmi, (result_code == UPLL_RC_SUCCESS));
  db_con->ReleaseRwConn(dmi);

  key_vtn *rename_vtn_key = reinterpret_cast<key_vtn*>(ck_vtn->get_key());
  uuc::CtrlrMgr::GetInstance()->
      UpdateVtnExhaustionMap(reinterpret_cast<char*>(rename_vtn_key->vtn_name),
                    reinterpret_cast<char*>(ctrlr_dom.ctrlr),
                    reinterpret_cast<char*>(ctrlr_dom.domain), alarm_asserted);

  VlinkMoMgr *vlink_mgr =
      reinterpret_cast<VlinkMoMgr *>(const_cast<MoManager *>
               (const_cast<MoManager*>(GetMoManager(UNC_KT_VLINK))));
  result_code = vlink_mgr->RestoreVlinkOperStatus(ctrlr_id, domain_id,
                                                  rename_vtn_key->vtn_name,
                                                  db_con, cfg_lock, notfn);
  DELETE_IF_NOT_NULL(ck_vtn);
  return result_code;
}
}  // namespace kt_momgr
}  // namespace upll
}  // namespace unc
