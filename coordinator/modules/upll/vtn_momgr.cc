/*
 * Copyright (c) 2012-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include "vtn_momgr.hh"
#include "vbr_if_momgr.hh"
#include "vnode_momgr.hh"
#include "vlink_momgr.hh"
#include "vlanmap_momgr.hh"
#include "vterm_if_momgr.hh"

#define  NUM_KEY_COL 3

using unc::upll::ipc_util::IpcUtil;

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
                                         uud::kDalUint8, 1 } };

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
    { uudst::vtn_controller::kDbiRefCount, CFG_VAL, offsetof(val_vtn_ctrlr,
      ref_count), uud::kDalUint32, 1 },
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
      uud::kDalUint8, 1 } };

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

unc_key_type_t VtnMoMgr::vtn_child[] = { UNC_KT_VBRIDGE,
                                         UNC_KT_VROUTER, UNC_KT_VUNKNOWN,
                                         /* For restore case: keeping VTEP before VTEP_GRP */
                                         UNC_KT_VTEP, UNC_KT_VTEP_GRP,
                                         UNC_KT_VTUNNEL,
                                         UNC_KT_VLINK,
                                         UNC_KT_VTN_FLOWFILTER,
                                         UNC_KT_VTN_POLICINGMAP,
                                         UNC_KT_VTERMINAL };
  VtnMoMgr::VtnMoMgr() : MoMgrImpl() {
  UPLL_FUNC_TRACE;
  ntable = MAX_MOMGR_TBLS;
  table = new Table *[ntable];
  table[MAINTBL] = new Table(uudst::kDbiVtnTbl, UNC_KT_VTN, vtn_bind_info,
      IpctSt::kIpcStKeyVtn, IpctSt::kIpcStValVtn,
      uudst::vtn::kDbiVtnNumCols);
  table[RENAMETBL] = new Table(uudst::kDbiVtnRenameTbl, UNC_KT_VTN,
      vtn_rename_bind_info, IpctSt::kIpcStKeyVtn, IpctSt::kIpcStValRenameVtn,
      uudst::vtn_rename::kDbiVtnRenameNumCols);
  table[CTRLRTBL] = new Table(uudst::kDbiVtnCtrlrTbl, UNC_KT_VTN,
      vtn_controller_bind_info, IpctSt::kIpcStKeyVtn, IpctSt::kIpcInvalidStNum,
      uudst::vtn_controller::kDbiVtnCtrlrNumCols);
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
                          uint64_t index) {
  UPLL_FUNC_TRACE;
  key_vtn *vtn_key = reinterpret_cast<key_vtn *>(key);
  upll_rc_t ret_val = UPLL_RC_SUCCESS;

  switch (index) {
    case uudst::vtn::kDbiVtnName:
    case uudst::vtn_rename::kDbiUncVtnName:
      ret_val = ValidateKey(reinterpret_cast<char *>(vtn_key->vtn_name),
                        kMinLenVtnName, kMaxLenVtnName);
      break;
    default:
      ret_val = UPLL_RC_ERR_GENERIC;
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
      case uudst::vtn_controller::kDbiRefCount:
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
        tmp1 = new ConfigVal(IpctSt::kIpcStValVtnstationControllerSt, vtnstation);
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
    }
    if (!tmp1) {
      UPLL_LOG_ERROR("Memory Allocation failed for tmp1");
      FREE_IF_NOT_NULL(vtn_val);
      FREE_IF_NOT_NULL(rename_val);
      FREE_IF_NOT_NULL(ctrlr_val);
      return UPLL_RC_ERR_GENERIC;
    }
    //  error user data nneds to be copied
    SET_USER_DATA(tmp1, tmp)
    tmp = tmp->get_next_cfg_val();
  };
  if (tmp) {
    if (tbl == MAINTBL) {
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
        return result_code;
      }
      val_rename_vtn *rename_val = reinterpret_cast<val_rename_vtn *>
      (GetVal(okey));
      if (!rename_val) {
         UPLL_LOG_DEBUG("Rename Val is Empty");
         return UPLL_RC_ERR_GENERIC;
      }
      ctrlr_key = reinterpret_cast<key_vtn *>(ikey->get_key());
      if (!ctrlr_key) return UPLL_RC_ERR_GENERIC;
      memset(ctrlr_key, 0, sizeof(key_vtn));
      uuu::upll_strncpy(ctrlr_key->vtn_name, rename_val->new_name,
                        (kMaxLenVtnName + 1));
      SET_USER_DATA_FLAGS(ikey, VTN_RENAME);
      delete okey;
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

upll_rc_t
VtnMoMgr::GetControllerDomainSpan(ConfigKeyVal *ikey,
    upll_keytype_datatype_t dt_type,
    DalDmlIntf *dmi,
    std::list<controller_domain_t> &list_ctrlr_dom) {
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

  tmp_ikey = ikey;
  while (NULL != tmp_ikey) {
    ctrlr_dom.ctrlr = reinterpret_cast<uint8_t *>
        (ConfigKeyVal::Malloc((kMaxLenCtrlrId + 1)));
    ctrlr_dom.domain = reinterpret_cast<uint8_t *>
        (ConfigKeyVal::Malloc((kMaxLenDomainId + 1)));
    controller_domain_t tmp_ctrlr_dom;
    tmp_ctrlr_dom.ctrlr = NULL;
    tmp_ctrlr_dom.domain = NULL;
    GET_USER_DATA_CTRLR_DOMAIN(tmp_ikey, tmp_ctrlr_dom);
    UPLL_LOG_TRACE(" ctrlr = %s, dom = %s ", tmp_ctrlr_dom.ctrlr,
                                              tmp_ctrlr_dom.domain);
    uuu::upll_strncpy(ctrlr_dom.ctrlr, tmp_ctrlr_dom.ctrlr,
                        (kMaxLenCtrlrId + 1));
    uuu::upll_strncpy(ctrlr_dom.domain, tmp_ctrlr_dom.domain,
                        (kMaxLenDomainId + 1));
    list_ctrlr_dom.push_back(ctrlr_dom);
    tmp_ikey = tmp_ikey->get_next_cfg_key_val();
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
    vtn_val_db_st->down_count = nreq_vtnst->down_count;
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
  result_code = UpdateConfigDB(ck_vtn, UPLL_DT_STATE, op, dmi, &dbop, MAINTBL);
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
    bool return_value = ctrlr_mgr->GetCtrlrType(reinterpret_cast<char *>(ctrlr_id),
                            UPLL_DT_CANDIDATE, &ctrlrtype);
    if (false == return_value) {
      UPLL_LOG_DEBUG("Unknown Controller Type\n");
      return UPLL_RC_ERR_GENERIC;
    }
  } 
  if (op == UNC_OP_CREATE) {
    if (ctrlr_val_vtn == NULL) return UPLL_RC_ERR_GENERIC;
    ctrlr_val_vtn->oper_status = 
         (driver_result == UPLL_RC_ERR_CTR_DISCONNECTED)? 
          UPLL_OPER_STATUS_UNKNOWN:UPLL_OPER_STATUS_UP;
    ctrlr_val_vtn->alarm_status = UPLL_ALARM_CLEAR;
    ctrlr_val_vtn->valid[0] = UNC_VF_VALID;
    ctrlr_val_vtn->valid[1] = UNC_VF_VALID;
    ctrlr_val_vtn->down_count = 0;
    if (ctrlrtype == UNC_CT_PFC) {
        ctrlr_val_vtn->oper_status = 
         (driver_result == UPLL_RC_ERR_CTR_DISCONNECTED)? 
          UPLL_OPER_STATUS_UNKNOWN:UPLL_OPER_STATUS_DOWN;
        ctrlr_val_vtn->down_count = ctrlr_val_vtn->ref_count;
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
      ctrlr_val_vtn->oper_status =
             (driver_result == UPLL_RC_ERR_CTR_DISCONNECTED)? 
              UPLL_OPER_STATUS_UNKNOWN:UPLL_OPER_STATUS_UP;
      ctrlr_val_vtn->valid[0] = UNC_VF_VALID;
      ctrlr_val_vtn->down_count = run_ctrlr_val->down_count;
      // down count incremented only for create of vnode
     // down count decrement handled during deletion of vnode, if needed.
      if (ctrlrtype == UNC_CT_PFC) {
        int diff = ctrlr_val_vtn->ref_count - run_ctrlr_val->ref_count;
        if (diff > 0)
          ctrlr_val_vtn->down_count = (run_ctrlr_val->down_count + diff);
        else if (diff == 0)
          ctrlr_val_vtn->down_count = (run_ctrlr_val->down_count + 1);
        ctrlr_val_vtn->oper_status = 
             (driver_result == UPLL_RC_ERR_CTR_DISCONNECTED)? 
              UPLL_OPER_STATUS_UNKNOWN:UPLL_OPER_STATUS_DOWN;
      } 
    }
  }
  val_db_vtn_st *vtn_val_db_st = reinterpret_cast<val_db_vtn_st *>
                                                     (GetStateVal(vtn_run));
  if (!vtn_val_db_st) {
     UPLL_LOG_DEBUG("Returning error %d\n", UPLL_RC_ERR_GENERIC);
     return UPLL_RC_ERR_GENERIC;
  }
  // initialize the main table vtn oper status to be recomputed.
  val_vtn_st *val_vtnst = &(vtn_val_db_st->vtn_val_st);
  val_vtnst->oper_status = 
             (driver_result == UPLL_RC_ERR_CTR_DISCONNECTED)? 
              UPLL_OPER_STATUS_UNKNOWN:UPLL_OPER_STATUS_UNINIT;
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
                                   DalDmlIntf *dmi) {
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
                          req, nreq, &cfg1_cursor, dmi, NULL, MAINTBL, true);
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
                                       dmi, &dbop_update, CTRLRTBL);
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
              dmi, MAINTBL);
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
                               nreq, &cfg1_cursor, dmi, NULL, CTRLRTBL, true);
    if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      result_code = UPLL_RC_SUCCESS;
      UPLL_LOG_DEBUG("Diff Skipped for op %d", op[i]);
      DELETE_IF_NOT_NULL(req);
      DELETE_IF_NOT_NULL(nreq);
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
                                   dmi, CTRLRTBL);
      if (result_code != UPLL_RC_SUCCESS) {
        if (cfg1_cursor)
          dmi->CloseCursor(cfg1_cursor, true);
        delete req;
        return result_code;
      }
      if (op[i] != UNC_OP_DELETE) {
        DbSubOp dbop_update = {kOpNotRead, kOpMatchNone, kOpInOutCs};
        result_code = UpdateConfigDB(vtn_ck_run, UPLL_DT_STATE,
            UNC_OP_UPDATE, dmi, &dbop_update, MAINTBL);
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
                                                          op[i], dmi);
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
                                              uint32_t *ckv_count) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  if (*ckv_count >= header->rep_count) {
    return result_code;
  }
  controller_domain ctrlr_dom_in;
  ctrlr_dom_in.ctrlr = NULL;
  ctrlr_dom_in.domain = NULL;

  IpcRequest ipc_req;
  memset(&ipc_req, 0, sizeof(ipc_req));
  memcpy(&(ipc_req.header), header, sizeof(IpcReqRespHeader));
  ipc_req.header.operation = UNC_OP_READ;

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
    ipc_req.ckv_data = ckv_drv;
    IpcResponse ipc_resp;
    memset(&ipc_resp, 0, sizeof(IpcResponse));
    if (!IpcUtil::SendReqToDriver((const char *)(ctrlr_dom_db.ctrlr),
          reinterpret_cast<char *>(ctrlr_dom_db.domain), PFCDRIVER_SERVICE_NAME,
          PFCDRIVER_SVID_LOGICAL, &ipc_req, true, &ipc_resp)) {
      UPLL_LOG_INFO("Request to driver for Key %d for controller %s failed ",
                    ikey->get_key_type(),
                    reinterpret_cast<char *>(ctrlr_dom_db.ctrlr));
      delete ckv_all_domain;
      delete ckv_drv;
      DELETE_IF_NOT_NULL(ipc_resp.ckv_data);
      return ipc_resp.header.result_code;
    }
    if (ipc_resp.header.result_code != UPLL_RC_SUCCESS
        && ipc_resp.header.result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      UPLL_LOG_INFO("Read from driver failed err code %d",
                    ipc_resp.header.result_code);
      DELETE_IF_NOT_NULL(result_ckv);
      DELETE_IF_NOT_NULL(ckv_all_domain);
      DELETE_IF_NOT_NULL(ckv_drv);
      DELETE_IF_NOT_NULL(ipc_resp.ckv_data);
      if (ipc_resp.header.result_code == UPLL_RC_ERR_CTR_DISCONNECTED) {
        ipc_resp.header.result_code = UPLL_RC_ERR_NO_SUCH_INSTANCE;
      }
      return ipc_resp.header.result_code;
    } else if (ipc_resp.header.result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      UPLL_LOG_DEBUG("Record Not found for domain %s", ctrlr_dom_db.domain);
      delete ckv_drv;
      ckv_drv = NULL;
      DELETE_IF_NOT_NULL(ipc_resp.ckv_data);
      continue;
    } else if (ipc_resp.header.result_code == UPLL_RC_SUCCESS) {
      if (ipc_resp.ckv_data == NULL) {
        // NOTE: Is this still happening: ipc is successfull and no ckv_data?
        delete ckv_all_domain;
        delete ckv_drv;
        UPLL_LOG_DEBUG("Ipc Response ckv_data is NUll %d",
                       ipc_resp.header.result_code);
        return UPLL_RC_ERR_GENERIC;
      }
      ckv_drv->ResetWith(ipc_resp.ckv_data);
      DELETE_IF_NOT_NULL(ipc_resp.ckv_data);
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

upll_rc_t VtnMoMgr::ReadSingleCtlrlStation(IpcReqRespHeader *header,
                                             ConfigKeyVal *ikey,
                                             DalDmlIntf *dmi,
                                             uint32_t *rec_count) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  controller_domain ctrlr_dom;
  ctrlr_dom.ctrlr = NULL;
  ctrlr_dom.domain = NULL;
  DbSubOp op = {kOpNotRead, kOpMatchCtrlr, kOpInOutCtrlr | kOpInOutDomain};
  IpcRequest ipc_req;
  memset(&ipc_req, 0, sizeof(ipc_req));
  memcpy(&(ipc_req.header), header, sizeof(IpcReqRespHeader));
  ipc_req.header.operation = UNC_OP_READ;
  val_vtnstation_controller_st_t *in_valst = NULL;
  val_vtnstation_controller_st_t *valst = NULL;
  val_rename_vtn *rename_valst = NULL;
  ConfigKeyVal *ckv_drv = NULL;
  ConfigKeyVal *ckv_rename = NULL;
  ConfigKeyVal *okey = NULL;
  ConfigKeyVal *ckv_vbrif = NULL;
  ConfigKeyVal *ckv_vtermif = NULL;
  bool renamed = false;
  IpcResponse ipc_resp;
  UPLL_LOG_DEBUG("Input ikey is %s", ikey->ToStrAll().c_str());
  result_code = DupConfigKeyValVtnStation(okey, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("DupConfigKeyValVtnStation failed err code %d", result_code);
    return result_code;
  }
  ikey->SetCfgVal(NULL);
  ctrlr_dom.ctrlr =  reinterpret_cast<key_vtnstation_controller *>
                     (okey->get_key())->controller_name;
  ckv_rename = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, NULL, NULL);
  if (!(okey->get_cfg_val())) {
    UPLL_LOG_TRACE("val_vtnstation_controller_st is NULL");
  } else if (okey->get_cfg_val()->get_st_num() ==
             IpctSt::kIpcStValVtnstationControllerSt) {
    in_valst = reinterpret_cast<val_vtnstation_controller_st_t *>(GetVal(okey));
    if (!in_valst) {
      UPLL_LOG_INFO("Input val_vtnstation_controller_st in Null");
      DELETE_IF_NOT_NULL(ckv_rename);
      DELETE_IF_NOT_NULL(okey);
      return UPLL_RC_ERR_GENERIC;
    }
    if ((in_valst->valid[UPLL_IDX_VTN_NAME_VSCS] == UNC_VF_VALID) &&
       (in_valst->valid[UPLL_IDX_VBR_NAME_VSCS] == UNC_VF_VALID) &&
       (in_valst->valid[UPLL_IDX_VBR_IF_NAME_VSCS] == UNC_VF_VALID)) {
       key_vbr_if_t *vbrif_key = reinterpret_cast<key_vbr_if_t *>
                                 (ConfigKeyVal::Malloc(sizeof(key_vbr_if_t)));
       uuu::upll_strncpy(vbrif_key->vbr_key.vtn_key.vtn_name,
                         in_valst->vtn_name, (kMaxLenVtnName + 1));
       uuu::upll_strncpy(vbrif_key->vbr_key.vbridge_name, in_valst->vbr_name,
                         (kMaxLenVnodeName + 1));
       uuu::upll_strncpy(vbrif_key->if_name, in_valst->vbrif_name,
                         (kMaxLenInterfaceName + 1));
       ckv_vbrif = new ConfigKeyVal(UNC_KT_VBR_IF,
                                     IpctSt::kIpcStKeyVbrIf,
                                     vbrif_key, NULL);
       SET_USER_DATA_CTRLR(ckv_vbrif, ctrlr_dom.ctrlr);
       MoMgrImpl *mgr = reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>
                        (GetMoManager(UNC_KT_VBR_IF)));

       DbSubOp dbop = {kOpReadSingle, kOpMatchCtrlr,
                       kOpInOutCtrlr | kOpInOutDomain};
       result_code = mgr->ReadConfigDB(ckv_vbrif, UPLL_DT_RUNNING,
                                     UNC_OP_READ, dbop, dmi, MAINTBL);
       if (result_code != UPLL_RC_SUCCESS &&
              result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
         UPLL_LOG_INFO("ReadConfigDB from VbrIf tbl failed errcode %d",
                       result_code);
         DELETE_IF_NOT_NULL(ckv_vbrif);
         DELETE_IF_NOT_NULL(okey);
         DELETE_IF_NOT_NULL(ckv_rename);
         return result_code;
       } else if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
         key_vterm_if_t *vtermif_key = reinterpret_cast<key_vterm_if_t *>
                                 (ConfigKeyVal::Malloc(sizeof(key_vterm_if_t)));
         uuu::upll_strncpy(vtermif_key->vterm_key.vtn_key.vtn_name,
                         in_valst->vtn_name, (kMaxLenVtnName + 1));
         uuu::upll_strncpy(vtermif_key->vterm_key.vterminal_name, in_valst->vbr_name,
                         (kMaxLenVnodeName + 1));
         uuu::upll_strncpy(vtermif_key->if_name, in_valst->vbrif_name,
                         (kMaxLenInterfaceName + 1));
         ckv_vtermif = new ConfigKeyVal(UNC_KT_VTERM_IF,
                                        IpctSt::kIpcStKeyVtermIf,
                                        vtermif_key, NULL);
         MoMgrImpl *vtermif_mgr = reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>
                        (GetMoManager(UNC_KT_VTERM_IF)));
         SET_USER_DATA_CTRLR(ckv_vtermif, ctrlr_dom.ctrlr);
         result_code = vtermif_mgr->ReadConfigDB(ckv_vtermif, UPLL_DT_RUNNING,
                                     UNC_OP_READ, dbop, dmi, MAINTBL);
         if (result_code != UPLL_RC_SUCCESS) {
           UPLL_LOG_INFO("ReadConfigDB from VtermIf tbl failed errcode %d",
                       result_code);
           DELETE_IF_NOT_NULL(ckv_vbrif);
           DELETE_IF_NOT_NULL(okey);
           DELETE_IF_NOT_NULL(ckv_rename);
           DELETE_IF_NOT_NULL(ckv_vtermif)
           return result_code;
         }
         GET_USER_DATA_DOMAIN(ckv_vtermif, ctrlr_dom.domain);
         ckv_vtermif->SetCfgVal(NULL);
         result_code = vtermif_mgr->GetRenamedControllerKey(ckv_vtermif, UPLL_DT_RUNNING,
                      dmi, &ctrlr_dom);
         if (result_code != UPLL_RC_SUCCESS &&
           result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
           UPLL_LOG_DEBUG("GetRenamedControllerKey failed. Result : %d",
                          result_code);
           DELETE_IF_NOT_NULL(ckv_vtermif);
           DELETE_IF_NOT_NULL(okey);
           DELETE_IF_NOT_NULL(ckv_rename);
           return result_code;
         }
         uuu::upll_strncpy(in_valst->vtn_name,
                         vtermif_key->vterm_key.vtn_key.vtn_name,
                         (kMaxLenVtnName + 1));
         uuu::upll_strncpy(in_valst->vbrif_name, vtermif_key->vterm_key.vterminal_name,
                         (kMaxLenInterfaceName + 1));
       } else {
         val_drv_vbr_if_t *val = reinterpret_cast<val_drv_vbr_if_t *>
                                               (GetVal(ckv_vbrif));
         if (val && (val->valid[PFCDRV_IDX_VEXT_NAME_VBRIF] == UNC_VF_VALID)) {
           uuu::upll_strncpy(in_valst->vbrif_name, val->vex_name,
                           (kMaxLenInterfaceName + 1));
         } else {
           UPLL_LOG_INFO("Port Map not configured");
           DELETE_IF_NOT_NULL(ckv_vbrif);
           DELETE_IF_NOT_NULL(okey);
           DELETE_IF_NOT_NULL(ckv_rename);
           return UPLL_RC_ERR_NO_SUCH_INSTANCE;
         }
         GET_USER_DATA_DOMAIN(ckv_vbrif, ctrlr_dom.domain);
         ckv_vbrif->SetCfgVal(NULL);
         result_code = mgr->GetRenamedControllerKey(ckv_vbrif, UPLL_DT_RUNNING,
                      dmi, &ctrlr_dom);
         if (result_code != UPLL_RC_SUCCESS &&
             result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
           UPLL_LOG_DEBUG("GetRenamedControllerKey failed. Result : %d",
                          result_code);
           DELETE_IF_NOT_NULL(ckv_vbrif);
           DELETE_IF_NOT_NULL(okey);
           DELETE_IF_NOT_NULL(ckv_rename);
           return result_code;
         }
         uuu::upll_strncpy(in_valst->vtn_name,
                         vbrif_key->vbr_key.vtn_key.vtn_name,
                         (kMaxLenVtnName + 1));
         uuu::upll_strncpy(in_valst->vbr_name, vbrif_key->vbr_key.vbridge_name,
                         (kMaxLenVnodeName + 1));
      }
    } else if (in_valst->valid[UPLL_IDX_VTN_NAME_VSCS] == UNC_VF_VALID) {
      key_vtn_t *vtn_key = reinterpret_cast<key_vtn_t *>
                         (ConfigKeyVal::Malloc(sizeof(key_vtn)));
      uuu::upll_strncpy(vtn_key->vtn_name, in_valst->vtn_name,
                       (kMaxLenVtnName + 1));
      op.readop = kOpReadMultiple;
      if (in_valst->valid[UPLL_IDX_DOMAIN_ID_VSCS] == UNC_VF_VALID) {
        ctrlr_dom.domain = in_valst->domain_id;
        op.matchop  = kOpMatchCtrlr | kOpMatchDomain;
        op.readop  = kOpReadSingle;
      }
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
       * controller table and rename table */
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
          ckv_rename = vtn_ckv;
        }
      }
    }
  }
  ConfigKeyVal *temp = ckv_rename;
  for (; ckv_rename; ckv_rename = ckv_rename->get_next_cfg_key_val()) {
    UPLL_LOG_TRACE("ckv_rename in loop is \n %s",
                    ckv_rename->ToStrAll().c_str());
    memset(&(ipc_resp), 0, sizeof(IpcResponse));
    result_code = DupConfigKeyValVtnStation(ckv_drv, okey);
    if (result_code != UPLL_RC_SUCCESS) {
      DELETE_IF_NOT_NULL(temp);
      DELETE_IF_NOT_NULL(okey);
      DELETE_IF_NOT_NULL(ckv_vbrif);
      DELETE_IF_NOT_NULL(ckv_vtermif);
      UPLL_LOG_DEBUG("DupConfigKeyValVtnStation failed. err : %d", result_code);
      return result_code;
    }
    valst = reinterpret_cast<val_vtnstation_controller_st *>(GetVal(ckv_drv));
    rename_valst = reinterpret_cast<val_rename_vtn *>(GetVal(ckv_rename));
    if (renamed && (valst != NULL) && (rename_valst != NULL)) {
      uuu::upll_strncpy(valst->vtn_name, rename_valst->new_name,
                        (kMaxLenVtnName + 1));
    }
    ipc_req.ckv_data = ckv_drv;
    UPLL_LOG_DEBUG("Controller id and domain id are %s %s", ctrlr_dom.ctrlr,
                    ctrlr_dom.domain);
    if (!IpcUtil::SendReqToDriver((const char *)(ctrlr_dom.ctrlr),
         reinterpret_cast<char *>(ctrlr_dom.domain), PFCDRIVER_SERVICE_NAME,
          PFCDRIVER_SVID_LOGICAL, &ipc_req, true, &ipc_resp)) {
      UPLL_LOG_INFO("Request to driver for Key %d for controller %s failed ",
             ikey->get_key_type(), reinterpret_cast<char *>(ctrlr_dom.ctrlr));
      DELETE_IF_NOT_NULL(ckv_drv);
      DELETE_IF_NOT_NULL(ipc_resp.ckv_data);
      DELETE_IF_NOT_NULL(okey);
      DELETE_IF_NOT_NULL(ckv_vbrif);
      DELETE_IF_NOT_NULL(ckv_vtermif);
      DELETE_IF_NOT_NULL(temp);
      return ipc_resp.header.result_code;
    }
    if (ipc_resp.header.result_code != UPLL_RC_SUCCESS
      && ipc_resp.header.result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      UPLL_LOG_INFO("Read from driver failed err code %d",
      ipc_resp.header.result_code);
      DELETE_IF_NOT_NULL(ckv_drv);
      DELETE_IF_NOT_NULL(ipc_resp.ckv_data);
      DELETE_IF_NOT_NULL(okey);
      DELETE_IF_NOT_NULL(ckv_vbrif);
      DELETE_IF_NOT_NULL(temp);
      DELETE_IF_NOT_NULL(ckv_vtermif);
      return ipc_resp.header.result_code;
    } else if (ipc_resp.header.result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      UPLL_LOG_DEBUG("Record Not found for domain %s", ctrlr_dom.domain);
      result_code = ipc_resp.header.result_code;
      DELETE_IF_NOT_NULL(ckv_drv);
      DELETE_IF_NOT_NULL(ipc_resp.ckv_data);
      continue;
    } else if (ipc_resp.header.result_code == UPLL_RC_SUCCESS) {
      if (ipc_resp.ckv_data == NULL) {
        UPLL_LOG_DEBUG("Ipc Response ckv_data is NUll %d",
        ipc_resp.header.result_code);
        DELETE_IF_NOT_NULL(ckv_drv);
        DELETE_IF_NOT_NULL(okey);
        DELETE_IF_NOT_NULL(ckv_vbrif);
        DELETE_IF_NOT_NULL(temp);
        DELETE_IF_NOT_NULL(ckv_vtermif);
        return UPLL_RC_ERR_GENERIC;
      }
      ckv_drv->ResetWith(ipc_resp.ckv_data);
      DELETE_IF_NOT_NULL(ipc_resp.ckv_data);
      result_code = MappingvExtTovBr(ckv_drv, header, dmi, rec_count);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("MappingvExtTovBr failed result_code - %d", result_code);
        DELETE_IF_NOT_NULL(ckv_drv);
        DELETE_IF_NOT_NULL(ipc_resp.ckv_data);
        DELETE_IF_NOT_NULL(okey);
        DELETE_IF_NOT_NULL(ckv_vbrif);
        DELETE_IF_NOT_NULL(temp);
        DELETE_IF_NOT_NULL(ckv_vtermif);
        return result_code;
      }
      UPLL_LOG_TRACE("record count is %d", *rec_count);
      if (ikey->get_cfg_val()) {
        uint32_t *count = reinterpret_cast<uint32_t *>(GetVal(ikey));
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
      ikey->AppendCfgVal(ckv_drv->GetCfgValAndUnlink());
    }
    DELETE_IF_NOT_NULL(ckv_drv);
  }
  DELETE_IF_NOT_NULL(ckv_vtermif);
  DELETE_IF_NOT_NULL(ckv_vbrif);
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

upll_rc_t
VtnMoMgr::ReadMo(IpcReqRespHeader * header,
                 ConfigKeyVal * ikey,
                 DalDmlIntf * dmi)  {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  controller_domain ctrlr_dom;
  ctrlr_dom.ctrlr = NULL;
  ctrlr_dom.domain = NULL;
  key_vtn_controller *mkey = NULL;
  ConfigKeyVal *ck_ctrlr = NULL;
  DbSubOp op = {kOpReadMultiple, kOpMatchCtrlr,
                kOpInOutCtrlr | kOpInOutDomain};
  if ((ikey->get_key_type() == UNC_KT_VTN_MAPPING_CONTROLLER) ||
      (ikey->get_key_type() == UNC_KT_VTNSTATION_CONTROLLER)) {
    result_code = ValidateMessage(header, ikey);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("ValidateMessage failed result_code %d",
                  result_code);
      return result_code;
    }
  }
  uint32_t rec_count = 0;
  uint32_t *tmp_count;
  switch (ikey->get_key_type()) {
    case UNC_KT_VTN:
      result_code = MoMgrImpl::ReadMo(header, ikey, dmi);
      return result_code;
      break;
    case UNC_KT_VTNSTATION_CONTROLLER:
      if (header->option1 == UNC_OPT1_NORMAL ||
          header->option1 == UNC_OPT1_DETAIL) {
        result_code = ReadSingleCtlrlStation(header, ikey,
                                            dmi, &rec_count);
      } else if (header->option1 == UNC_OPT1_COUNT &&
         (GetVal(ikey) != NULL) && !IsAllInvalidAttributes(
           reinterpret_cast<val_vtnstation_controller_st *>
           (const_cast<void *>(GetVal(ikey))))) {
       header->option1 = UNC_OPT1_NORMAL;
       result_code = ReadSingleCtlrlStation(header, ikey,
                                            dmi, &rec_count);
       result_code = (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) ?
                     UPLL_RC_SUCCESS : result_code;
       header->option1 = UNC_OPT1_COUNT;
       uint32_t *count =
          reinterpret_cast<uint32_t*>(ConfigKeyVal::Malloc(sizeof(uint32_t)));
       *count = rec_count;
       ikey->SetCfgVal(new ConfigVal(IpctSt::kIpcStUint32, count));
      } else if (header->option1 == UNC_OPT1_COUNT) {
         IpcRequest ipc_req;
         memset(&ipc_req, 0, sizeof(ipc_req));
         memcpy(&(ipc_req.header), header, sizeof(IpcReqRespHeader));
         IpcResponse ipc_resp;
         memset(&ipc_resp, 0, sizeof(IpcResponse));
         ipc_req.ckv_data = ikey;
         ctrlr_dom.ctrlr = reinterpret_cast<key_vtnstation_controller *>
                           (ikey->get_key())->controller_name;
         if (!IpcUtil::SendReqToDriver((const char *)(ctrlr_dom.ctrlr),
            reinterpret_cast<char *>(ctrlr_dom.domain), PFCDRIVER_SERVICE_NAME,
                           PFCDRIVER_SVID_LOGICAL, &ipc_req, true, &ipc_resp)) {
           UPLL_LOG_INFO("Request to driver for Key %d on controller %s failed",
               ikey->get_key_type(), reinterpret_cast<char *>(ctrlr_dom.ctrlr));
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
      }
      break;
    case UNC_KT_VTN_MAPPING_CONTROLLER:
      IpcRequest ipc_req;
      memset(&ipc_req, 0, sizeof(ipc_req));
      memcpy(&(ipc_req.header), header, sizeof(IpcReqRespHeader));
      IpcResponse ipc_resp;
      memset(&ipc_resp, 0, sizeof(IpcResponse));
      mkey = reinterpret_cast<key_vtn_controller *>(ikey->get_key());
      ctrlr_dom.ctrlr = mkey->controller_name;
      ctrlr_dom.domain = mkey->domain_id;
      result_code = GetChildConfigKey(ck_ctrlr, ikey);
      if (result_code != UPLL_RC_SUCCESS) {
        DELETE_IF_NOT_NULL(ck_ctrlr);
        UPLL_LOG_INFO("GetChildConfigKey failed err code %d", result_code);
        return result_code;
      }
      SET_USER_DATA_CTRLR_DOMAIN(ck_ctrlr, ctrlr_dom);
      result_code = UpdateConfigDB(ck_ctrlr, UPLL_DT_RUNNING,
            UNC_OP_READ, dmi, &op, CTRLRTBL);
      if (result_code != UPLL_RC_ERR_INSTANCE_EXISTS) {
        UPLL_LOG_DEBUG("ReadConfigDB Failed result_code - %d", result_code);
        DELETE_IF_NOT_NULL(ck_ctrlr);
        return result_code;
      }
      result_code = GetRenamedControllerKey(ck_ctrlr, UPLL_DT_RUNNING, dmi,
                                          &ctrlr_dom);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("GetRenamedControllerKey Failed result_code - %d",
                       result_code);
        DELETE_IF_NOT_NULL(ck_ctrlr);
        return result_code;
      }
      uuu::upll_strncpy(mkey->vtn_key.vtn_name,
                reinterpret_cast<key_vtn *>(ck_ctrlr->get_key())->vtn_name,
                (kMaxLenVtnName + 1));
      ipc_req.ckv_data = ikey;
      if (!IpcUtil::SendReqToDriver((const char *)(ctrlr_dom.ctrlr),
            reinterpret_cast<char *>(ctrlr_dom.domain), PFCDRIVER_SERVICE_NAME,
            PFCDRIVER_SVID_LOGICAL, &ipc_req, true, &ipc_resp)) {
        UPLL_LOG_INFO("Request to driver for Key %d for controller %s failed ",
               ikey->get_key_type(), reinterpret_cast<char *>(ctrlr_dom.ctrlr));
        return ipc_resp.header.result_code;
      }
      if (ipc_resp.header.result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_INFO("Read from driver failed err code %d",
                             ipc_resp.header.result_code);
        return ipc_resp.header.result_code;
      }
      if (ipc_resp.ckv_data == NULL) {
        UPLL_LOG_DEBUG("Ipc Response ckv_data is NUll %d",
                    ipc_resp.header.result_code);
        DELETE_IF_NOT_NULL(ck_ctrlr);
        return UPLL_RC_ERR_GENERIC;
      }
      ikey->ResetWith(ipc_resp.ckv_data);
      tmp_count  = &rec_count;
      result_code = MappingvExtTovBr(ikey, header, dmi, tmp_count);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("MappingvExtTovBr failed result_code - %d", result_code);
        return result_code;
      }
      // header->rep_count = *tmp_count;
      break;
    default:
      UPLL_LOG_INFO("Invalid KeyType %d", ikey->get_key_type());
      DELETE_IF_NOT_NULL(ck_ctrlr);
      return UPLL_RC_ERR_GENERIC;
  }
  return result_code;
}

upll_rc_t VtnMoMgr::ReadSiblingCount(IpcReqRespHeader *header,
                                      ConfigKeyVal* ikey,
                                      DalDmlIntf *dmi) {
    UPLL_FUNC_TRACE;
    upll_rc_t result_code = UPLL_RC_SUCCESS;
    uint32_t *count;
    switch (ikey->get_key_type()) {
      case UNC_KT_VTN:
        result_code = MoMgrImpl::ReadSiblingCount(header, ikey, dmi);
        return result_code;
        break;
      case UNC_KT_VTNSTATION_CONTROLLER:
        header->operation = UNC_OP_READ_SIBLING;
        header->rep_count = UINT32_MAX;
        result_code = ReadSiblingMo(header, ikey, false, dmi);
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
  controller_domain ctrlr_dom;
  ctrlr_dom.ctrlr = NULL;
  ctrlr_dom.domain = NULL;
  ConfigKeyVal *next_ckv = NULL;
  if ((ikey->get_key_type() == UNC_KT_VTN_MAPPING_CONTROLLER) ||
      (ikey->get_key_type() == UNC_KT_VTNSTATION_CONTROLLER)) {
    result_code = ValidateMessage(header, ikey);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("ValidateMessage failed result_code %d",
                  result_code);
      return result_code;
    }
  }
  key_vtn_controller *mkey = NULL;
  ConfigKeyVal *ck_ctrlr = NULL;
  uint32_t rec_count = 0;
  uint32_t count = 0;
  IpcResponse ipc_resp;
  memset(&ipc_resp, 0, sizeof(IpcResponse));
  IpcRequest ipc_req;
  memset(&ipc_req, 0, sizeof(ipc_req));
  memcpy(&(ipc_req.header), header, sizeof(IpcReqRespHeader));
  ipc_req.ckv_data = ikey;
  uuc::CtrlrMgr *ctrlr_mgr = uuc::CtrlrMgr::GetInstance();
  string ctrlr_id;
  unc_keytype_ctrtype_t ctrlrtype = UNC_CT_UNKNOWN;
  ConfigKeyVal *okey = NULL;
  key_vtnstation_controller *vtn_stkey = NULL;
  key_vtn_controller *vtnkey = NULL;
  DbSubOp op = {kOpReadExist, kOpMatchNone, kOpInOutCtrlr | kOpInOutDomain};
  switch (ikey->get_key_type()) {
    case UNC_KT_VTN:
      result_code = MoMgrImpl::ReadSiblingMo(header, ikey, false, dmi);
      return result_code;
      break;
    case UNC_KT_VTNSTATION_CONTROLLER:
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
                                            dmi, &rec_count);
          if (result_code == UPLL_RC_SUCCESS)
            count++;
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
        result_code = DupConfigKeyValVtnStation(next_ckv, ikey);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("DupConfigKeyValVtnStation failed err code %d",
                          result_code);
          return result_code;
        }
        vtn_stkey = reinterpret_cast<key_vtnstation_controller *>
                                     (next_ckv->get_key());
        uuu::upll_strncpy(vtn_stkey->controller_name, ctrlr_dom.ctrlr,
                          (kMaxLenCtrlrId + 1));
          result_code = ReadSingleCtlrlStation(header, next_ckv,
                                            dmi, &rec_count);
          if (result_code != UPLL_RC_SUCCESS &&
              result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
            next_ckv = NULL;
            continue;
          }
          count++;
          okey->AppendCfgKeyVal(next_ckv);
          next_ckv = NULL;
      }
      header->rep_count = count;
      ikey->ResetWith(okey);
    break;
    case UNC_KT_VTN_MAPPING_CONTROLLER:
    {
      mkey = reinterpret_cast<key_vtn_controller *>(ikey->get_key());
      ctrlr_dom.ctrlr = mkey->controller_name;
      ctrlr_dom.domain = mkey->domain_id;
      result_code = GetChildConfigKey(ck_ctrlr, ikey);

      /* Addressed coverity REVERSE_INULL issue */
      if (!ck_ctrlr || result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_INFO("GetChildConfigKey failed err code %d", result_code);
        DELETE_IF_NOT_NULL(ck_ctrlr);
        return result_code;
      }
      SET_USER_DATA_CTRLR_DOMAIN(ck_ctrlr, ctrlr_dom);
      #if 0
      if (header->operation == UNC_OP_READ_SIBLING_BEGIN)
        op.matchop = kOpMatchNone;
      #endif
      result_code = UpdateConfigDB(ck_ctrlr, UPLL_DT_RUNNING,
            UNC_OP_READ, dmi, &op, CTRLRTBL);
      if (result_code != UPLL_RC_ERR_INSTANCE_EXISTS) {
        UPLL_LOG_DEBUG("ReadConfigDB Failed result_code - %d", result_code);
        DELETE_IF_NOT_NULL(ck_ctrlr);
        return result_code;
      }
      DELETE_IF_NOT_NULL(ck_ctrlr);
      // SET_USER_DATA_CTRLR_DOMAIN(ikey, ctrlr_dom);
      result_code = DupConfigKeyValVtnMapping(okey, ikey);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("DupConfigKeyValVtnMapping failed err code %d",
                        result_code);
        DELETE_IF_NOT_NULL(okey);
        return result_code;
      }
      bool okey_mapped = false;
      if (header->operation == UNC_OP_READ_SIBLING_BEGIN) {
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
                                                  dmi, &rec_count);
          if (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE &&
            result_code != UPLL_RC_SUCCESS) {
            delete okey;
            return result_code;
          } else if (result_code == UPLL_RC_SUCCESS) {
            okey_mapped = true;
          }
        }
      } else {
          // check if user given controller can be used.
          ctrlr_id = reinterpret_cast<char *>
                     (reinterpret_cast<key_vtn_controller *>
                     (okey->get_key())->controller_name);
          UPLL_LOG_TRACE("Controller Name is %s", ctrlr_id.c_str());
          ctrlr_dom.ctrlr = reinterpret_cast<uint8_t *>(
                  const_cast<char *>(ctrlr_id.c_str()));

          if ((ctrlr_mgr->GetCtrlrType(reinterpret_cast<char *>
              (ctrlr_dom.ctrlr), UPLL_DT_RUNNING, &ctrlrtype)) &&
              (ctrlrtype == UNC_CT_PFC)) {
             UPLL_LOG_INFO("Controller type is  %d", ctrlrtype);
             // return UPLL_RC_ERR_GENERIC;
            ctrlr_dom.domain = reinterpret_cast<key_vtn_controller *>
                                 (okey->get_key())->domain_id;
            UPLL_LOG_TRACE("Domain name is %s", ctrlr_dom.domain);
            SET_USER_DATA_CTRLR_DOMAIN(okey, ctrlr_dom);

            result_code = ReadSingleCtlrlVtnMapping(header, okey,
                                                    dmi, &rec_count);
            if (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE &&
              result_code != UPLL_RC_SUCCESS) {
              delete okey;
              return result_code;
            } else if (result_code == UPLL_RC_SUCCESS) {
              okey_mapped = true;
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
        vtnkey = reinterpret_cast<key_vtn_controller *>(next_ckv->get_key());
        uuu::upll_strncpy(vtnkey->controller_name, ctrlr_dom.ctrlr,
                          (kMaxLenCtrlrId + 1));
        uuu::upll_strncpy(vtnkey->domain_id, ctrlr_dom.domain,
                          (kMaxLenDomainId + 1));
        result_code = ReadSingleCtlrlVtnMapping(header, next_ckv, dmi,
                                                &rec_count);
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
    default:
      return UPLL_RC_ERR_GENERIC;
      break;
  }

  return result_code;
}

upll_rc_t
VtnMoMgr::MappingvExtTovBr(ConfigKeyVal * ikey,
                           IpcReqRespHeader * req,
                           DalDmlIntf * dmi, uint32_t *&rec_count)  {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  DbSubOp dbop = { kOpReadSingle, kOpMatchCtrlr, kOpInOutNone };
  MoMgrImpl *mgr = reinterpret_cast<MoMgrImpl *>
                   (const_cast<MoManager *>(GetMoManager(UNC_KT_VBR_IF)));
  val_vtnstation_controller_st *valst = NULL;
  val_vtn_mapping_controller_st *val_map = NULL;
  key_vbr_if_t *key_vbrif = NULL;
  ConfigKeyVal *tmpckv = NULL;
  if ((GetVal(ikey)) == NULL) {
     UPLL_LOG_DEBUG("Val struct is not present from driver response");
     return UPLL_RC_ERR_GENERIC;
  }
  ConfigVal *tmp_cval = NULL;
  UPLL_LOG_DEBUG("ikey in MappingvExtTovBr is %s", ikey->ToStrAll().c_str());
   VtermIfMoMgr *vterm_if_mgr = reinterpret_cast<VtermIfMoMgr *>
      (const_cast<MoManager *>(GetMoManager(UNC_KT_VTERM_IF)));
  if (!vterm_if_mgr) {
    UPLL_LOG_ERROR("Invalid Mgr");
    return UPLL_RC_ERR_GENERIC;
  }

  uint8_t pfc_vtn_name[kMaxLenVtnName + 1];
  uint8_t unc_vtn_name[kMaxLenVtnName + 1];
  pfc_vtn_name[0] = unc_vtn_name[0] = 0;
  uint8_t *ctrlr_id = NULL;
  // Save received pfc vtn_name
  if (UNC_KT_VTN_MAPPING_CONTROLLER == ikey->get_key_type()) {
    uuu::upll_strncpy(pfc_vtn_name,
                      reinterpret_cast<key_vtn_controller *>
                      (ikey->get_key())->vtn_key.vtn_name,
                      (kMaxLenVtnName + 1));
    key_vtn_t *vtnkey = reinterpret_cast<key_vtn_t *>
        (ConfigKeyVal::Malloc(sizeof(key_vtn_t)));
    uuu::upll_strncpy(vtnkey->vtn_name,
                      pfc_vtn_name,
                      (kMaxLenVtnName + 1));
    ConfigKeyVal *ckv_rename = new ConfigKeyVal(UNC_KT_VTN,
                                                IpctSt::kIpcStKeyVtn, vtnkey, NULL);
    ctrlr_id =  reinterpret_cast<key_vtn_controller *>
        (ikey->get_key())->controller_name;
    result_code = GetRenamedUncKey(ckv_rename, UPLL_DT_RUNNING,
                                   dmi, ctrlr_id);
    if (result_code != UPLL_RC_SUCCESS &&
        result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      UPLL_LOG_INFO("GetRenamedUncKey failed. Error : %d", result_code);
      if (ckv_rename)
        delete ckv_rename;
      return result_code;
    }
    if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE)
      result_code = UPLL_RC_SUCCESS;
    /* Store unc vtn name if received controller vtn is renamed in UNC */
    uuu::upll_strncpy(unc_vtn_name,
                      reinterpret_cast<key_vtn_t *>
                      (ckv_rename->get_key())->vtn_name,
                      (kMaxLenVtnName + 1));
    DELETE_IF_NOT_NULL(ckv_rename);
  }
  for (tmp_cval = ikey->get_cfg_val(); tmp_cval;
       tmp_cval = tmp_cval->get_next_cfg_val()) {
  if (UNC_KT_VTNSTATION_CONTROLLER == ikey->get_key_type()) {
    UPLL_LOG_DEBUG("KetType Matched  %d", ikey->get_key_type());
    if (IpctSt::kIpcStValVtnstationControllerSt ==
        tmp_cval->get_st_num()) {
      (*rec_count)++;
      UPLL_LOG_DEBUG("record count is %d", *rec_count);
      valst = reinterpret_cast<val_vtnstation_controller_st_t *>(
         tmp_cval->get_val());
      if (!valst) {
        UPLL_LOG_DEBUG("val_vtnstation_controller_st is NULL");
        return UPLL_RC_ERR_GENERIC;
      }
      if ((valst->valid[UPLL_IDX_VTN_NAME_VSCS] != UNC_VF_VALID) &&
         (valst->valid[UPLL_IDX_VBR_IF_NAME_VSCS] != UNC_VF_VALID)) {
        UPLL_LOG_DEBUG("valid flag of vtn_name/vbrif_name is invalid");
        return UPLL_RC_ERR_GENERIC;
      }
      uint8_t *ctrlr_id =  reinterpret_cast<key_vtnstation_controller *>
                           (ikey->get_key())->controller_name;
      key_vtn_t *vtnkey = reinterpret_cast<key_vtn_t *>
                   (ConfigKeyVal::Malloc(sizeof(key_vtn_t)));
      uuu::upll_strncpy(vtnkey->vtn_name,
                        valst->vtn_name, (kMaxLenVtnName + 1));
      ConfigKeyVal *ckv_rename = new ConfigKeyVal(UNC_KT_VTN,
                                 IpctSt::kIpcStKeyVtn, vtnkey, NULL);
      result_code = GetRenamedUncKey(ckv_rename, UPLL_DT_RUNNING,
                                     dmi, ctrlr_id);
      if (result_code != UPLL_RC_SUCCESS &&
          result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
        UPLL_LOG_INFO("GetRenamedUncKey failed error code %d", result_code);
        if (ckv_rename) delete ckv_rename;
        return result_code;
      }
      if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE)
        result_code = UPLL_RC_SUCCESS;
      if (valst->map_type == UPLL_IF_VLAN_MAP) {
        uuu::upll_strncpy(valst->vtn_name, reinterpret_cast<key_vtn_t *>
                       (ckv_rename->get_key())->vtn_name,
                       (kMaxLenVtnName + 1));
        DELETE_IF_NOT_NULL(ckv_rename);
        continue;
      }

      // Get vbridge and vbridge-if name if the valst->vbrif_name matches a
      // vexternal.
      key_vbrif = static_cast<key_vbr_if_t *>
        (ConfigKeyVal::Malloc(sizeof(key_vbr_if_t)));
      // Store UNC VTN name in key_vbrif
      uuu::upll_strncpy(key_vbrif->vbr_key.vtn_key.vtn_name,
                        reinterpret_cast<key_vtn_t *>
                        (ckv_rename->get_key())->vtn_name,
                        (kMaxLenVtnName + 1));
      val_drv_vbr_if_t *drv_val_vbrif = static_cast<val_drv_vbr_if_t *>
        (ConfigKeyVal::Malloc(sizeof(val_drv_vbr_if_t)));
      drv_val_vbrif->valid[PFCDRV_IDX_VEXT_NAME_VBRIF] = UNC_VF_VALID;
      uuu::upll_strncpy(drv_val_vbrif->vex_name,
                  valst->vbrif_name, (kMaxLenInterfaceName + 1));
      tmpckv = new
        ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf, key_vbrif,
           new ConfigVal(IpctSt::kIpcStPfcdrvValVbrIf, drv_val_vbrif));
      SET_USER_DATA_CTRLR(tmpckv, ctrlr_id);
      UPLL_LOG_DEBUG("tmpckv is %s", tmpckv->ToStrAll().c_str());
      result_code = mgr->ReadConfigDB(tmpckv, req->datatype,
                       UNC_OP_READ, dbop, dmi, MAINTBL);
      if(result_code == UPLL_RC_SUCCESS) {
        uuu::upll_strncpy(valst->vtn_name, reinterpret_cast<key_vtn_t *>
                       (ckv_rename->get_key())->vtn_name,
                       (kMaxLenVtnName + 1));
        key_vbr_if_t *vbrif_key = reinterpret_cast<key_vbr_if_t *>
          (tmpckv->get_key());
        if (vbrif_key) {
          valst->valid[UPLL_IDX_VBR_NAME_VSCS] = UNC_VF_VALID;
          uuu::upll_strncpy(valst->vbr_name, vbrif_key->vbr_key.vbridge_name,
              (kMaxLenVnodeName + 1));
          uuu::upll_strncpy(valst->vbrif_name, vbrif_key->if_name,
              (kMaxLenInterfaceName + 1));
        }
      } else if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
        // Check if the valst->vbrif_name matches a vTerminal.
        result_code = vterm_if_mgr->GetVtermIfFromVexternal(valst->vtn_name, valst->vbrif_name,
                                              valst->vbr_name, ctrlr_id, dmi);
        if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
          UPLL_LOG_DEBUG("vextenal neither present in vbrif tbl nor vtermif tbl");
          result_code = UPLL_RC_SUCCESS;
        } else if (result_code == UPLL_RC_SUCCESS) {
          valst->valid[UPLL_IDX_VBR_NAME_VSCS] = UNC_VF_VALID;
        } else {
          UPLL_LOG_ERROR("Failed to read Vterminal interface info "
                         "error_code : %d", result_code );
          delete tmpckv;
          delete ckv_rename;
          return result_code;
        }
      } else {
        // Handle DB Error and other errors
        UPLL_LOG_ERROR("Failed to read VBR_IF info. error_code : %d",
                       result_code);
        delete tmpckv;
        delete ckv_rename;
        return result_code;
      }
     delete tmpckv;
     delete ckv_rename;
   }
  } else if (UNC_KT_VTN_MAPPING_CONTROLLER == ikey->get_key_type()) {
       UPLL_LOG_DEBUG("KetType Matched  %d", ikey->get_key_type());
       if (IpctSt::kIpcStValVtnMappingControllerSt ==
        tmp_cval->get_st_num()) {
         val_map = reinterpret_cast<val_vtn_mapping_controller_st_t *>
          (tmp_cval->get_val());
         if (!val_map) {
           UPLL_LOG_DEBUG("val_vtn_mapping_controller_st is NULL");
           return UPLL_RC_ERR_GENERIC;
         }
         (*rec_count)++;
         UPLL_LOG_DEBUG("record count is %d", *rec_count);
         if (val_map->map_type == UPLL_IF_VLAN_MAP) {
           continue;
         }
         // Valid flag check before accessing the driver reponse val
         if (val_map->valid[UPLL_IDX_VBR_IF_NAME_VMCS] != UNC_VF_VALID) {
           UPLL_LOG_DEBUG("valid flag of vbrif_name is invalid");
           return UPLL_RC_ERR_GENERIC;
         }

         key_vbrif = static_cast<key_vbr_if_t *>
           (ConfigKeyVal::Malloc(sizeof(key_vbr_if_t)));
         uuu::upll_strncpy(key_vbrif->vbr_key.vtn_key.vtn_name,
                           unc_vtn_name,
                        (kMaxLenVtnName + 1));
         val_drv_vbr_if_t *drv_val_vbrif = static_cast<val_drv_vbr_if_t *>
           (ConfigKeyVal::Malloc(sizeof(val_drv_vbr_if_t)));
         drv_val_vbrif->valid[PFCDRV_IDX_VEXT_NAME_VBRIF] = UNC_VF_VALID;
         uuu::upll_strncpy(drv_val_vbrif->vex_name,
                     val_map->vbrif_name, (kMaxLenInterfaceName + 1));
         tmpckv = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf,
                                   key_vbrif,
                                   new ConfigVal(IpctSt::kIpcStPfcdrvValVbrIf,
                                                 drv_val_vbrif));
         SET_USER_DATA_CTRLR(tmpckv, ctrlr_id);
         result_code = mgr->ReadConfigDB(tmpckv, req->datatype,
                          UNC_OP_READ, dbop, dmi, MAINTBL);
         if (result_code == UPLL_RC_SUCCESS) {
           UPLL_LOG_DEBUG("tmpckv value is %s", tmpckv->ToStrAll().c_str());
           key_vbr_if_t *vbrif_key = reinterpret_cast<key_vbr_if_t *>
             (tmpckv->get_key());
           if (vbrif_key) {
             val_map->valid[UPLL_IDX_VBR_NAME_VMCS] = UNC_VF_VALID;
             uuu::upll_strncpy(val_map->vbr_name,
                 vbrif_key->vbr_key.vbridge_name,
                 (kMaxLenVnodeName + 1));
             uuu::upll_strncpy(val_map->vbrif_name, vbrif_key->if_name,
                 (kMaxLenInterfaceName + 1));
           }
         } else if(result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
           uint8_t local_pfc_vtnname[kMaxLenVtnName + 1];
           uuu::upll_strncpy(local_pfc_vtnname, pfc_vtn_name,
                            (kMaxLenVtnName + 1));
           /* Gets UNC vtn_name,vterminal_name,vterm_if name
            * if the recived controller mapping info is part of vterm_if */
           result_code = vterm_if_mgr->GetVtermIfFromVexternal(
                                          local_pfc_vtnname,
                                          val_map->vbrif_name,
                                          val_map->vbr_name, ctrlr_id, dmi);
           if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
             UPLL_LOG_DEBUG("vextenal neither present in vbrif tbl nor vtermif tbl");
             result_code = UPLL_RC_SUCCESS;
           } else if (result_code == UPLL_RC_SUCCESS) {
             val_map->valid[UPLL_IDX_VBR_NAME_VMCS] = UNC_VF_VALID;
           } else {
             UPLL_LOG_ERROR("Failed to read Vterminal interface info "
                 "error_code : %d", result_code );
             delete tmpckv;
             return result_code;
           }
         } else {
           // Handle DB Error and other errors
           UPLL_LOG_ERROR("Failed to read VBR_IF info. error_code : %d",
                          result_code);
           delete tmpckv;
           return result_code;
         }
         delete tmpckv;
       }
    }
  }
  if (UNC_KT_VTN_MAPPING_CONTROLLER == ikey->get_key_type()) {
    /* If the received controller vtn is renamed at UNC means, vtn_name is
     * need to sent with UNC vtn_name */
    uuu::upll_strncpy(reinterpret_cast<key_vtn_controller *>
                      (ikey->get_key())->vtn_key.vtn_name,
                      unc_vtn_name,
                      (kMaxLenVtnName + 1));
  }
  return result_code;
}

/* Semantic check for the VTN Delete operation */
upll_rc_t
VtnMoMgr::IsReferenced(ConfigKeyVal *ikey,
                       upll_keytype_datatype_t dt_type,
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
  result_code = mgr->ReadConfigDB(okey, dt_type, UNC_OP_READ,
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
    UPLL_LOG_DEBUG("Returning error %d\n",UPLL_RC_ERR_GENERIC);
    return UPLL_RC_ERR_GENERIC;
  }
  val_vtnst->vtn_val_st.oper_status = UPLL_OPER_STATUS_UNINIT;
  val_vtnst->vtn_val_st.valid[UPLL_IDX_OPER_STATUS_VS] = UNC_VF_VALID;
  DbSubOp dbop_update = {kOpNotRead, kOpMatchNone, kOpInOutCs};
  result_code = UpdateConfigDB(ikey, UPLL_DT_STATE, UNC_OP_UPDATE, dmi,
                               &dbop_update, MAINTBL);
  delete ctrlr_ckv;
  return result_code;
}

upll_rc_t
VtnMoMgr::SetConsolidatedStatus(ConfigKeyVal * ikey,
                                DalDmlIntf * dmi)  {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *ckv = NULL;
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
    list_cs_row.push_back(static_cast<unc_keytype_configstatus_t>(val->cs_row_status));
  }
  DELETE_IF_NOT_NULL(ckv);
  val_vtn_t *val_temp = reinterpret_cast<val_vtn_t *>(GetVal(ikey));
  val_temp->cs_row_status = GetConsolidatedCsStatus(list_cs_row);
  result_code = UpdateConfigDB(ikey, UPLL_DT_RUNNING, UNC_OP_UPDATE, dmi,
                               MAINTBL);
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
                                DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  if (!import_ckval || !(import_ckval->get_key())) {
    UPLL_LOG_DEBUG("Invalid Input");
    return UPLL_RC_ERR_GENERIC;
  }
  ConfigKeyVal *ckval = NULL;
  unc_key_type_t child_key[]= {
                         UNC_KT_VBRIDGE, UNC_KT_VBR_NWMONITOR, UNC_KT_VROUTER,
                         UNC_KT_VRT_IF, UNC_KT_VLINK, UNC_KT_VTERMINAL
                              };
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
      result_code = mgr->MergeValidate(child_key[i], ctrlr_id, ckval, dmi);
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
  return result_code;
}

upll_rc_t
VtnMoMgr::MergeValidate(unc_key_type_t keytype,
                        const char *ctrlr_id,
                        ConfigKeyVal *ikey,
                        DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_ERR_GENERIC;
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
    return UPLL_RC_SUCCESS;
  }

  /* Other than  UPLL_RC_ERR_NO_SUCH_INSTANCE AND UPLL_RC_SUCCESS */
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG(" ReadConfigDB Failed %d", result_code);
    DELETE_IF_NOT_NULL(import_ckval);
    return result_code;
  }
  ConfigKeyVal *start_ckv = import_ckval;
  result_code = MergeValidateChildren(import_ckval, ctrlr_id, ikey, dmi);
  DELETE_IF_NOT_NULL(start_ckv);
  if (UPLL_RC_SUCCESS == result_code) {
    ConfigKeyVal *req = NULL;
    ConfigKeyVal *nreq = NULL;
    DalCursor *dal_cursor_handle  = NULL;
    UPLL_LOG_TRACE("Create Entry in candidate");
    result_code = DiffConfigDB(UPLL_DT_IMPORT, UPLL_DT_CANDIDATE,
                  UNC_OP_CREATE, req, nreq, &dal_cursor_handle, dmi, MAINTBL);
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
                                    dmi, &dbop, MAINTBL);
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

// Not used anywhere currently in the source
upll_rc_t 
VtnMoMgr::MergeVtnMainTable(DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *req = NULL;
  ConfigKeyVal *nreq = NULL;
  DalCursor *dal_cursor_handle  = NULL;
  UPLL_LOG_TRACE("Create Entry in candidate");
  result_code = DiffConfigDB(UPLL_DT_IMPORT, UPLL_DT_CANDIDATE,
                UNC_OP_CREATE, req, nreq, &dal_cursor_handle, dmi, MAINTBL);
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
    DbSubOp dbop = {kOpReadMultiple, kOpMatchNone, kOpInOutFlag};
    result_code = UpdateConfigDB(req, UPLL_DT_CANDIDATE, UNC_OP_CREATE,
                                 dmi, &dbop, MAINTBL);
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
  return result_code;
}

upll_rc_t
VtnMoMgr::TxUpdateController(unc_key_type_t keytype,
                             uint32_t session_id,
                             uint32_t config_id,
                             uuc::UpdateCtrlrPhase phase,
                             set<string> *affected_ctrlr_set,
                             DalDmlIntf *dmi,
                             ConfigKeyVal **err_ckv) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  DalResultCode dal_result = uud::kDalRcSuccess;
  ConfigKeyVal *req = NULL, *nreq = NULL, *ck_main = NULL;
  controller_domain ctrlr_dom;
  DalCursor *dal_cursor_handle = NULL;
  unc_keytype_operation_t op = (phase == uuc::kUpllUcpCreate)?UNC_OP_CREATE:
        ((phase == uuc::kUpllUcpUpdate)?UNC_OP_UPDATE:
        ((phase == uuc::kUpllUcpDelete)?UNC_OP_DELETE:UNC_OP_INVALID));
  ctrlr_dom.ctrlr = NULL;
  ctrlr_dom.domain = NULL;
  MoMgrTables tbl = (op != UNC_OP_UPDATE)?CTRLRTBL:MAINTBL;
  result_code = DiffConfigDB(UPLL_DT_CANDIDATE, UPLL_DT_RUNNING,
                  op, req, nreq, &dal_cursor_handle, dmi, tbl);
  while (result_code == UPLL_RC_SUCCESS) {
    // Get Next Record
    dal_result = dmi->GetNextRecord(dal_cursor_handle);
    result_code = DalToUpllResCode(dal_result);
    if (result_code != UPLL_RC_SUCCESS)
      break;
    ck_main = NULL;
    IpcResponse resp;
    memset(&resp, 0, sizeof(resp));

    resp.header.clnt_sess_id = session_id;
    resp.header.config_id = config_id;
    if (op != UNC_OP_UPDATE) {
      if (op == UNC_OP_CREATE)
        result_code = DupConfigKeyVal(ck_main, req, tbl);
      else
        result_code = GetChildConfigKey(ck_main, req);
      if (!ck_main || result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Returning error %d", result_code);
        DELETE_IF_NOT_NULL(ck_main);
        break;
      }
      GET_USER_DATA_CTRLR_DOMAIN(ck_main, ctrlr_dom);
      UPLL_LOG_DEBUG("ctrlr : %s; domain : %s", ctrlr_dom.ctrlr,
                     ctrlr_dom.domain);
      if ((ctrlr_dom.ctrlr == NULL) || (ctrlr_dom.domain == NULL)) {
        UPLL_LOG_INFO("Invalid controller/domain");
        DELETE_IF_NOT_NULL(ck_main);
        result_code = UPLL_RC_ERR_GENERIC;
        break;
      }

      bool driver_resp = false;
      result_code = TxUpdateProcess(ck_main, &resp,
            op, dmi, &ctrlr_dom, affected_ctrlr_set, &driver_resp);
      if (result_code != UPLL_RC_SUCCESS && driver_resp) {
        UPLL_LOG_DEBUG("Returns error %d", result_code);
        if (resp.ckv_data != NULL) {
          SET_USER_DATA_CTRLR(resp.ckv_data, ctrlr_dom.ctrlr);
          *err_ckv = resp.ckv_data;
        }
        DELETE_IF_NOT_NULL(ck_main);
        break;
      } else if (result_code != UPLL_RC_SUCCESS) {
        DELETE_IF_NOT_NULL(resp.ckv_data);
        DELETE_IF_NOT_NULL(ck_main);
        break;
      }
      DELETE_IF_NOT_NULL(resp.ckv_data);
    }
    DELETE_IF_NOT_NULL(ck_main);
  }
  DELETE_IF_NOT_NULL(nreq);
  DELETE_IF_NOT_NULL(req);

  if (dal_cursor_handle)
    dmi->CloseCursor(dal_cursor_handle, true);

  result_code = (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE)?
                                UPLL_RC_SUCCESS:result_code;
  return result_code;
}

upll_rc_t
VtnMoMgr::TxUpdateProcess(ConfigKeyVal *ck_main,
                          IpcResponse *ipc_resp,
                          unc_keytype_operation_t op,
                          DalDmlIntf *dmi,
                          controller_domain *ctrlr_dom,
                          set<string> *affected_ctrlr_set, bool *driver_resp) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code;
  controller_domain ctrlr_dom_dup;
  ctrlr_dom_dup.ctrlr = NULL;
  ctrlr_dom_dup.domain = NULL;
  /* read from main table */
  ConfigKeyVal *dup_ckmain = ck_main;
  if (op == UNC_OP_CREATE) {
    dup_ckmain = NULL;
    result_code = GetChildConfigKey(dup_ckmain, ck_main);
    if (result_code != UPLL_RC_SUCCESS || dup_ckmain == NULL) {
      UPLL_LOG_DEBUG("Returning error %d", result_code);
      delete dup_ckmain;
      return result_code;
    }
    DbSubOp dbop = {kOpReadSingle, kOpMatchNone, kOpInOutNone};
    result_code = ReadConfigDB(dup_ckmain, UPLL_DT_CANDIDATE,
        UNC_OP_READ, dbop, dmi, MAINTBL);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_INFO("%s Vtn read failed %d", (dup_ckmain->ToStrAll()).c_str(),
                     result_code);
      if (dup_ckmain) delete dup_ckmain;
      return result_code;
    }
    /* To filter the attributes to be sent to controller */
    void *val = (ck_main) ? GetVal(ck_main) : NULL;
    void *dup_val = (dup_ckmain) ? GetVal(dup_ckmain) : NULL;
    if (FilterAttributes(dup_val, val, false, op)) {
     if (dup_ckmain) delete dup_ckmain;
     return UPLL_RC_SUCCESS;
    }
  } else if (op == UNC_OP_UPDATE) {
    UPLL_LOG_INFO("UPDATE to VTN is not supported at driver. so return..");
    return UPLL_RC_SUCCESS;
  }
  /* Get renamed key if key is renamed */
  /* For Delete operation we have to get info
   * from running db
   */
  if (op == UNC_OP_DELETE)
     result_code = GetRenamedControllerKey(dup_ckmain, UPLL_DT_RUNNING,
                                            dmi, ctrlr_dom);
  else {
     if (op == UNC_OP_CREATE) {
       ctrlr_dom_dup.ctrlr = NULL;
       ctrlr_dom_dup.domain = NULL;
       GET_USER_DATA_CTRLR_DOMAIN(dup_ckmain, ctrlr_dom_dup);
     }
     result_code = GetRenamedControllerKey(dup_ckmain, UPLL_DT_CANDIDATE,
                                            dmi, &ctrlr_dom_dup);
  }
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetRenamedControllerKey failed. Result : %d",
                        result_code);
    if ((op == UNC_OP_CREATE) && dup_ckmain)
      delete dup_ckmain;
    return result_code;
  }
  // ipc_req->ckv_data = ck_main;
  result_code = SendIpcReq(ipc_resp->header.clnt_sess_id,
                           ipc_resp->header.config_id, op, UPLL_DT_CANDIDATE,
                           dup_ckmain, ctrlr_dom, ipc_resp);
  if (result_code == UPLL_RC_ERR_CTR_DISCONNECTED) {
    UPLL_LOG_DEBUG("Controller disconnected");
    result_code = UPLL_RC_SUCCESS;
  }
  if (result_code != UPLL_RC_SUCCESS) {
    *driver_resp = true;
    UPLL_LOG_DEBUG("IpcSend failed %d", result_code);
  }

  affected_ctrlr_set->insert((const char *)ctrlr_dom->ctrlr);
  if ((op == UNC_OP_CREATE) && dup_ckmain)
      delete dup_ckmain;
  UPLL_LOG_TRACE("Driver response received %d", *driver_resp);
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
  DbSubOp dbop = {kOpReadMultiple, kOpMatchNone, kOpInOutCtrlr|kOpInOutDomain|kOpInOutFlag};
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
      return result_code;                                     
    }  
    SET_USER_DATA_FLAGS(temp_key, VTN_RENAME);
    /* New Name available in the IMPORT, then
     * we have to update the rename flag in Main Table */
    result_code = UpdateConfigDB(temp_key, UPLL_DT_IMPORT,
                                 UNC_OP_UPDATE, dmi, &dbop, MAINTBL);
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
                                UNC_OP_DELETE, dmi, &dbop, MAINTBL);
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
                                 UNC_OP_CREATE, dmi, &dbop, MAINTBL);
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
                                UNC_OP_DELETE, dmi, &dbop, MAINTBL);
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
                                 MAINTBL);
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
                                  MAINTBL);
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
                                    dmi, &dbop, RENAMETBL);
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
  if (!req || !ikey || !(ikey->get_key())) {
    UPLL_LOG_INFO("ConfigKeyVal / IpcReqRespHeader is Null");
    return UPLL_RC_ERR_GENERIC;
  }
  if (ikey->get_st_num() != IpctSt::kIpcStKeyVtnstationController) {
    UPLL_LOG_INFO("Invalid Key structure received. received struct - %d",
                  (ikey->get_st_num()));
    return UPLL_RC_ERR_CFG_SYNTAX;
  }
  key_vtnstation_controller *vtn_ctrlr_key =
      reinterpret_cast<key_vtnstation_controller *> (ikey->get_key());

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
      ret_val = ValidateVtnStnCtrlrKey(vtn_ctrlr_key, req->operation);
      if (ret_val != UPLL_RC_SUCCESS) {
        UPLL_LOG_INFO("vtn_stn_ctrlr_key syntax check failed.");
        return UPLL_RC_ERR_CFG_SYNTAX;
      }
      return UPLL_RC_SUCCESS;
    } else {
      UPLL_LOG_INFO("Error Unsupported datatype (%d)", req->datatype);
      return UPLL_RC_ERR_NO_SUCH_INSTANCE;
    }
  }
  UPLL_LOG_INFO("Error Unsupported Operation (%d)", req->operation);
  return UPLL_RC_ERR_NO_SUCH_INSTANCE;
}

upll_rc_t
VtnMoMgr::ValidateMessageForVtnMapCtrlr(IpcReqRespHeader * req,
                                        ConfigKeyVal * ikey) {
  UPLL_FUNC_TRACE;
  if (!req || !ikey || !(ikey->get_key())) {
    UPLL_LOG_INFO("ConfigKeyVal / IpcReqRespHeader is Null");
    return UPLL_RC_ERR_GENERIC;
  }
  upll_keytype_datatype_t dt_type = req->datatype;
  unc_keytype_operation_t operation = req->operation;
  unc_keytype_option1_t option1 = req->option1;
  unc_keytype_option2_t option2 = req->option2;

  upll_rc_t ret_val = UPLL_RC_SUCCESS;
  if (ikey->get_st_num() != IpctSt::kIpcStKeyVtnController) {
    UPLL_LOG_INFO("Invalid key structure received. received struct - %d",
                  (ikey->get_st_num()));
    return UPLL_RC_ERR_CFG_SYNTAX;
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
        return UPLL_RC_ERR_CFG_SYNTAX;
      }
      UPLL_LOG_TRACE("value struct validation is none for this keytype");
      return UPLL_RC_SUCCESS;
    } else {
      UPLL_LOG_INFO("Error Unsupported datatype (%d)", dt_type);
      return UPLL_RC_ERR_NO_SUCH_INSTANCE;
    }
  }
  UPLL_LOG_INFO("Error Unsupported operation(%d)", operation);
  return UPLL_RC_ERR_NO_SUCH_INSTANCE;
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

upll_rc_t VtnMoMgr::SetOperStatus(ConfigKeyVal *ikey,
                             state_notification notification,
                             DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
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

    /* Update oper status based on notification */
#if 0
  if ((notification == kAdminStatusDisabled) &&
      (vtn_val->oper_status == UPLL_OPER_STATUS_UNKNOWN)) {
    return result_code;
  }
#endif
  vtn_val->valid[0] = UNC_VF_VALID;
  switch (notification) {
  case kCtrlrReconnectIfUp:
    if (vtn_val->oper_status == UPLL_OPER_STATUS_DOWN) {
      return UPLL_RC_SUCCESS;
    }
    vtn_val->oper_status = UPLL_OPER_STATUS_UP;
    break;
  case kCtrlrReconnectIfDown:
    if (vtn_val->oper_status == UPLL_OPER_STATUS_DOWN) {
      return UPLL_RC_SUCCESS;
    }
    vtn_val->oper_status = UPLL_OPER_STATUS_DOWN;
    break;
  case kCtrlrReconnect:
    vtn_val->oper_status = UPLL_OPER_STATUS_DOWN;
    break;
  case kCtrlrDisconnect:
  case kPortUnknown:
    vtn_val->oper_status = UPLL_OPER_STATUS_UNKNOWN;
    vtn_valst->down_count = 0;
    break;
  case kAdminStatusDisabled:
  case kPortFault:
  case kPathFault:
  case kBoundaryFault:
    vtn_valst->down_count = (vtn_valst->down_count + 1);
    if (vtn_valst->down_count == 1) {
      vtn_val->oper_status = UPLL_OPER_STATUS_DOWN;
      // generate alarm
    }
    break;
  case kAdminStatusEnabled:
  case kPortFaultReset:
  case kPathFaultReset:
  case kBoundaryFaultReset:
  case kBoundaryFaultResetWithAdminDisabled:
    vtn_valst->down_count = (vtn_valst->down_count > 0) ?
                            (vtn_valst->down_count - 1) : 0;
    if (notification != kBoundaryFaultResetWithAdminDisabled &&
        vtn_valst->down_count == 0) {
      vtn_val->oper_status = UPLL_OPER_STATUS_UNINIT;
      // reset alarm
    }
    break;
   default:
    UPLL_LOG_TRACE("Received unexpected notification %d\n",notification);
    break;
  }

  DbSubOp dbop = { kOpNotRead, kOpMatchNone, kOpInOutNone };
  result_code = UpdateConfigDB(ikey, UPLL_DT_STATE, UNC_OP_UPDATE,
                           dmi, &dbop, MAINTBL);
  UPLL_LOG_DEBUG("SetOperstatus for VTN after Update is \n %s",
                    ikey->ToStrAll().c_str());
  return result_code;
}

upll_rc_t VtnMoMgr::SetCtrlrOperStatus(ConfigKeyVal *ikey,
                             state_notification notification,
                             DalDmlIntf *dmi, bool &oper_change) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
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
  case kCtrlrReconnectIfUp:
    if (ctrlr_val->oper_status == UPLL_OPER_STATUS_DOWN) {
      return UPLL_RC_SUCCESS;
    }
    oper_change = (ctrlr_val->oper_status != UPLL_OPER_STATUS_UP)?
                   true:false;
    ctrlr_val->oper_status = UPLL_OPER_STATUS_UP;
    break;
  case kCtrlrReconnectIfDown:
    oper_change = (ctrlr_val->oper_status != UPLL_OPER_STATUS_DOWN)?
                   true:false;
    ctrlr_val->oper_status = UPLL_OPER_STATUS_DOWN;
    break;
  case kCtrlrReconnect:
    oper_change = (ctrlr_val->oper_status != UPLL_OPER_STATUS_DOWN)?
                   true:false;
    ctrlr_val->oper_status = UPLL_OPER_STATUS_DOWN;
    break;
  case kCtrlrDisconnect:
  case kPortUnknown:
    oper_change = (ctrlr_val->oper_status != UPLL_OPER_STATUS_UNKNOWN)?
                   true:false;
    ctrlr_val->oper_status = UPLL_OPER_STATUS_UNKNOWN;
    ctrlr_val->down_count = 0;
    break;
  case kAdminStatusDisabled:
  case kPortFault:
  case kPathFault:
  case kBoundaryFault:
    ctrlr_val->down_count = (ctrlr_val->down_count + 1);
    oper_change = (ctrlr_val->oper_status != UPLL_OPER_STATUS_DOWN)?
                   true:false;
    if (ctrlr_val->down_count == 1) {
      ctrlr_val->oper_status = UPLL_OPER_STATUS_DOWN;
      // generate alarm
    }
    break;
  case kAdminStatusEnabled:
  case kPortFaultReset:
  case kPathFaultReset:
  case kBoundaryFaultReset:
    if (ctrlr_val->oper_status == UPLL_OPER_STATUS_UP) {
      oper_change = false;
      UPLL_LOG_DEBUG("SetCtrlrOperstatus status already up\n %s",
                    ikey->ToStrAll().c_str());
      return UPLL_RC_SUCCESS;
    }
    ctrlr_val->down_count = (ctrlr_val->down_count > 0) ?
                            (ctrlr_val->down_count - 1) : 0;
    if (ctrlr_val->down_count == 0) {
      oper_change = (ctrlr_val->oper_status != UPLL_OPER_STATUS_UP)?
                     true:false;
      ctrlr_val->oper_status = UPLL_OPER_STATUS_UP;
      // reset alarm
    } else {
      ctrlr_val->oper_status = UPLL_OPER_STATUS_DOWN;
    }
    break;
   default:
    UPLL_LOG_TRACE("Received unexpected notification %d\n",notification);
    break;
  }
  DbSubOp dbop = { kOpNotRead, kOpMatchCtrlr | kOpMatchDomain, kOpInOutNone };
  if (notification == kCtrlrDisconnect ||
      notification == kCtrlrReconnectIfUp ||
      notification == kCtrlrReconnectIfDown)
    dbop.matchop = kOpMatchCtrlr;
  result_code = UpdateConfigDB(ikey, UPLL_DT_STATE, UNC_OP_UPDATE,
                                 dmi, &dbop, CTRLRTBL);
  UPLL_LOG_DEBUG("SetCtrlrOperstatus for VTN after Update is \n %s",
                    ikey->ToStrAll().c_str());
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
      DbSubOp dbop = { kOpReadMultiple,
                       kOpMatchCtrlr | kOpMatchDomain, kOpInOutNone };
      if (notification == kCtrlrDisconnect ||
          notification == kCtrlrReconnectIfUp ||
          notification == kCtrlrReconnectIfDown )
        dbop.matchop = kOpMatchCtrlr;

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
      SetOperStatus(ck_vtn_main, notification, dmi);
      if (ck_vtn_main)
        delete ck_vtn_main;
      ck_vtn_main = NULL;
    }
    if ( skip ) break;
    tkey = tkey->get_next_cfg_key_val();
  }
  return result_code;
}


/* SetOperStatus of VTN to unknown on a controller disconnect */
bool VtnMoMgr::VtnSetOperStatus(uint8_t *vtn_name_o,
                                DalDmlIntf *dmi,
                                state_notification notification) {
  UPLL_FUNC_TRACE;
  bool res = false;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  key_vtn_t *vtn_key = reinterpret_cast<key_vtn_t *>(
      ConfigKeyVal::Malloc(sizeof(key_vtn_t)));
  ConfigKeyVal *ck_vtn = new ConfigKeyVal(UNC_KT_VTN,
                          IpctSt::kIpcStKeyVtn, vtn_key, NULL);
  result_code = UpdateOperStatus(ck_vtn, dmi, notification, false);

  if (result_code != UPLL_RC_SUCCESS) {
    delete ck_vtn;
    UPLL_LOG_DEBUG("Returning error : %d", result_code);
    return false;
  }
  delete ck_vtn;
  return res;
}

upll_rc_t VtnMoMgr::TxUpdateDtState(unc_key_type_t ktype,
                                      uint32_t session_id,
                                      uint32_t config_id,
                                      DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *ck_vtn = NULL;

  /* Create Vnode If key */
  switch (ktype) {
  case UNC_KT_VTN:
    result_code = GetUninitOperState(ck_vtn, dmi);
    break;
  default:
    UPLL_LOG_DEBUG("Unsupported operation on keytype %d", ktype);
    return UPLL_RC_ERR_GENERIC;
  }
  if (UPLL_RC_SUCCESS != result_code)  {
    return result_code;
  }
  ConfigKeyVal *tkey = ck_vtn, *ck_ctrlr;
  DbSubOp dbop1 = { kOpNotRead, kOpMatchNone, kOpInOutNone };
  while (ck_vtn) {
    ck_ctrlr = NULL;
    tkey = ck_vtn;
    ck_vtn = tkey->get_next_cfg_key_val();
    tkey->set_next_cfg_key_val(NULL);

    val_db_vtn_st *vtn_st = reinterpret_cast<val_db_vtn_st *>
                                                (GetStateVal(tkey));
    result_code = GetChildConfigKey(ck_ctrlr, tkey);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Returning error %d\n", result_code);
      return result_code;
    }
    val_vtn_ctrlr *ctrlr_val = reinterpret_cast<val_vtn_ctrlr *>
                               (ConfigKeyVal::Malloc(sizeof(val_vtn_ctrlr)));
    ctrlr_val->valid[UPLL_IDX_OPER_STATUS_VS] = UNC_VF_VALID;
    ctrlr_val->oper_status = UPLL_OPER_STATUS_DOWN;
    ck_ctrlr->AppendCfgVal(IpctSt::kIpcInvalidStNum, ctrlr_val);
    DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutFlag | kOpInOutCtrlr
                       | kOpInOutDomain };
    result_code = ReadConfigDB(ck_ctrlr, UPLL_DT_STATE, UNC_OP_READ,
                                          dbop, dmi, CTRLRTBL);
    if (result_code == UPLL_RC_SUCCESS) {
      vtn_st->vtn_val_st.oper_status = UPLL_OPER_STATUS_DOWN;
      result_code = UPLL_RC_SUCCESS;
    } else if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      //no instance with oper down
      ck_ctrlr->SetCfgVal(NULL);
      result_code = UpdateConfigDB(ck_ctrlr, UPLL_DT_STATE, UNC_OP_READ,
                                          dmi, &dbop, CTRLRTBL);
      if (result_code == UPLL_RC_ERR_INSTANCE_EXISTS) {
        vtn_st->vtn_val_st.oper_status = UPLL_OPER_STATUS_UP;
        result_code = UPLL_RC_SUCCESS;
      } else if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
        vtn_st->vtn_val_st.oper_status = UPLL_OPER_STATUS_DOWN;
        result_code = UPLL_RC_SUCCESS;
      }
    }
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Returning error %d\n", result_code);
      delete tkey;
      delete ck_vtn;
      delete ck_ctrlr;
      return result_code;
    }
    vtn_st->vtn_val_st.valid[UPLL_IDX_OPER_STATUS_VS] = UNC_VF_VALID;
    result_code = UpdateConfigDB(tkey, UPLL_DT_STATE,
                     UNC_OP_UPDATE, dmi, &dbop1, MAINTBL);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("UpdateConfigDB Executed %d", result_code);
      delete tkey;
      delete ck_vtn;
      delete ck_ctrlr;
      break;
    }
    delete tkey;
    delete ck_ctrlr;
  }
  return result_code;
}

upll_rc_t VtnMoMgr::ControllerStatusHandler(uint8_t *ctrlr_id,
                                                DalDmlIntf *dmi,
                                                bool operstatus) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  state_notification notification = kCtrlrDisconnect;
  if (operstatus) {
    notification = kCtrlrReconnect;
  }
  if (notification == kCtrlrDisconnect) {
    result_code = RestoreVtnCtrlrOperStatus(ctrlr_id, dmi, notification);
    if (result_code != UPLL_RC_SUCCESS &&
        result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      UPLL_LOG_DEBUG("vtn controller operstatus disconnect update failed");
      return result_code;
    }
    result_code = UpdateVnodeOperStatus(ctrlr_id, dmi, notification, false);
    if (result_code != UPLL_RC_SUCCESS &&
        result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      UPLL_LOG_DEBUG("Error in updating node operstatus");
      return result_code;
    }
    VlinkMoMgr *vlink_mgr =
       reinterpret_cast<VlinkMoMgr *>(const_cast<MoManager *>
                 (const_cast<MoManager*>(GetMoManager(UNC_KT_VLINK))));
    vlink_mgr->UpdateVlinkOperStatus(ctrlr_id, dmi, notification, false);
    if (result_code != UPLL_RC_SUCCESS &&
        result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      UPLL_LOG_DEBUG("vlink operstatus update failed");
      return result_code;
    }
  } else {
    result_code = RestoreVtnCtrlrOperStatus(ctrlr_id, dmi, notification);
    if (result_code != UPLL_RC_SUCCESS &&
        result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      UPLL_LOG_DEBUG("Error in Updating Operstatus on Controller ReConnect");
      return result_code;
    }
  }
  return result_code;
}

upll_rc_t VtnMoMgr::UpdateVnodeOperStatus(uint8_t *ctrlr_id,
                                       DalDmlIntf *dmi,
                                       state_notification notification,
                                       bool skip) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  unc_key_type_t node_key_type[]= {UNC_KT_VBRIDGE, UNC_KT_VROUTER,
                                   UNC_KT_VTEP, UNC_KT_VTUNNEL, UNC_KT_VTERMINAL};
  ConfigKeyVal *ck_val = NULL;
  for (int vnode_count = 0;
       vnode_count < static_cast<int>
         (sizeof(node_key_type)/sizeof(unc_key_type_t));
       vnode_count++) {
    const unc_key_type_t ktype = node_key_type[vnode_count];

    VnodeMoMgr *mgr =
        reinterpret_cast<VnodeMoMgr *>(const_cast<MoManager *>
                    (const_cast<MoManager*>(GetMoManager(ktype))));

    result_code = mgr->GetChildConfigKey(ck_val, NULL);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Invalid param");
      return result_code;
    }

    SET_USER_DATA_CTRLR(ck_val, ctrlr_id);
    ConfigVal *cfg_val = NULL;
    mgr->AllocVal(cfg_val, UPLL_DT_STATE, MAINTBL);
    ck_val->SetCfgVal(cfg_val);
    switch (ktype) {
      case UNC_KT_VBRIDGE: {
        ConfigVal *tmp =
         (ck_val->get_cfg_val()) ? ck_val->get_cfg_val()->get_next_cfg_val() :
                                   NULL;
          val_db_vbr_st* vbr_dt_val =  reinterpret_cast<val_db_vbr_st *>
                            ((tmp != NULL) ? tmp->get_val() : NULL);
          if (vbr_dt_val == NULL) {
            UPLL_LOG_DEBUG("Invalid param");
            DELETE_IF_NOT_NULL(ck_val);
            return UPLL_RC_ERR_GENERIC;
          }
          vbr_dt_val->vbr_val_st.valid[0] = UNC_VF_VALID;
          vbr_dt_val->vbr_val_st.oper_status = UPLL_OPER_STATUS_UNKNOWN;
        }
        break;
      case UNC_KT_VROUTER: {
        ConfigVal *tmp =
         (ck_val->get_cfg_val()) ? ck_val->get_cfg_val()->get_next_cfg_val() :
                                   NULL;
          val_db_vrt_st* vrt_dt_val =  reinterpret_cast<val_db_vrt_st *>
                            ((tmp != NULL) ? tmp->get_val() : NULL);
          if (vrt_dt_val == NULL) {
            UPLL_LOG_DEBUG("Invalid param");
            DELETE_IF_NOT_NULL(ck_val);
            return UPLL_RC_ERR_GENERIC;
          }
          vrt_dt_val->vrt_val_st.valid[0] = UNC_VF_VALID;
          vrt_dt_val->vrt_val_st.oper_status = UPLL_OPER_STATUS_UNKNOWN;
        }
        break;
      case UNC_KT_VTEP: {
        ConfigVal *tmp =
         (ck_val->get_cfg_val()) ? ck_val->get_cfg_val()->get_next_cfg_val() :
                                   NULL;
          val_db_vtep_st* vtep_dt_val =  reinterpret_cast<val_db_vtep_st *>
                            ((tmp != NULL) ? tmp->get_val() : NULL);
          if (vtep_dt_val == NULL) {
            UPLL_LOG_DEBUG("Invalid param");
            DELETE_IF_NOT_NULL(ck_val);
            return UPLL_RC_ERR_GENERIC;
          }
          vtep_dt_val->vtep_val_st.valid[0] = UNC_VF_VALID;
          vtep_dt_val->vtep_val_st.oper_status = UPLL_OPER_STATUS_UNKNOWN;
        }
        break;
      case UNC_KT_VTUNNEL: {
        ConfigVal *tmp =
         (ck_val->get_cfg_val()) ? ck_val->get_cfg_val()->get_next_cfg_val() :
                                   NULL;
          val_db_vtunnel_st* vtunnel_dt_val =
                                       reinterpret_cast<val_db_vtunnel_st *>
                                       ((tmp != NULL) ? tmp->get_val() : NULL);
          if (vtunnel_dt_val == NULL) {
            UPLL_LOG_DEBUG("Invalid param");
            DELETE_IF_NOT_NULL(ck_val);
            return UPLL_RC_ERR_GENERIC;
          }
          vtunnel_dt_val->vtunnel_val_st.valid[0] = UNC_VF_VALID;
          vtunnel_dt_val->vtunnel_val_st.oper_status = UPLL_OPER_STATUS_UNKNOWN;
        }
        break;
        case UNC_KT_VTERMINAL: {
        ConfigVal *tmp =
         (ck_val->get_cfg_val()) ? ck_val->get_cfg_val()->get_next_cfg_val() :
                                   NULL;
          val_db_vterm_st* vterm_dt_val =
                                       reinterpret_cast<val_db_vterm_st *>
                                       ((tmp != NULL) ? tmp->get_val() : NULL);
          if (vterm_dt_val == NULL) {
            UPLL_LOG_DEBUG("Invalid param");
            DELETE_IF_NOT_NULL(ck_val);
            return UPLL_RC_ERR_GENERIC;
          }
          vterm_dt_val->vterm_val_st.valid[0] = UNC_VF_VALID;
          vterm_dt_val->vterm_val_st.oper_status = UPLL_OPER_STATUS_UNKNOWN;
        }
        break;
      default:
        UPLL_LOG_DEBUG("OperStatus not supported for this data type");
        DELETE_IF_NOT_NULL(ck_val);
        return UPLL_RC_ERR_GENERIC;
        break;
    }

    if (notification == kCtrlrDisconnect) {
      DbSubOp dbop = { kOpNotRead, kOpMatchCtrlr, kOpInOutNone };
      result_code = mgr->UpdateConfigDB(ck_val, UPLL_DT_STATE, UNC_OP_UPDATE,
                                   dmi, &dbop, MAINTBL);
      if (result_code == UPLL_RC_SUCCESS) {
        result_code = UpdateVnodeIfOperStatus(ck_val, dmi, notification,
                                              false, vnode_count);
        if (result_code != UPLL_RC_SUCCESS &&
            result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
          UPLL_LOG_DEBUG("Error in Updating operstatus");
          DELETE_IF_NOT_NULL(ck_val);
          return result_code;
        }
      } else if (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
        UPLL_LOG_DEBUG("Error in Updating operstatus");
        DELETE_IF_NOT_NULL(ck_val);
        return result_code;
      }
    }
    DELETE_IF_NOT_NULL(ck_val);
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtnMoMgr::UpdateVnodeIfOperStatus(ConfigKeyVal *ck_vtn,
                                       DalDmlIntf *dmi,
                                       state_notification notification,
                                       bool skip, int if_type) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  unc_key_type_t if_node_key_type[]= {UNC_KT_VBR_IF, UNC_KT_VRT_IF,
                                      UNC_KT_VTEP_IF, UNC_KT_VTUNNEL_IF,
                                      UNC_KT_VTERM_IF};
  ConfigKeyVal *ck_val = NULL;
    /*  Populating vtnset with vtn names connected to given controller name */
    const unc_key_type_t ktype = if_node_key_type[if_type];

  VnodeChildMoMgr *mgr =
      reinterpret_cast<VnodeChildMoMgr *>(const_cast<MoManager *>
                  (const_cast<MoManager*>(GetMoManager(ktype))));

  result_code = mgr->GetChildConfigKey(ck_val, ck_vtn);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetChildConfigKEy failed");
    return UPLL_RC_ERR_GENERIC;
  }
  ConfigVal *cfg_val = NULL;
  mgr->AllocVal(cfg_val, UPLL_DT_STATE, MAINTBL);
  ck_val->SetCfgVal(cfg_val);

  switch (ktype) {
    case UNC_KT_VBR_IF: {
      ConfigVal *tmp =
       (ck_val->get_cfg_val()) ? ck_val->get_cfg_val()->get_next_cfg_val() :
                                 NULL;
      val_db_vbr_if_st* vbr_if_dt_val =  reinterpret_cast<val_db_vbr_if_st *>
                          ((tmp != NULL) ? tmp->get_val() : NULL);
      if (vbr_if_dt_val == NULL) {
        UPLL_LOG_DEBUG("Invalid param");
        DELETE_IF_NOT_NULL(ck_val);
        return UPLL_RC_ERR_GENERIC;
      }
      vbr_if_dt_val->vbr_if_val_st.valid[0] = UNC_VF_VALID;
      vbr_if_dt_val->vbr_if_val_st.oper_status = UPLL_OPER_STATUS_UNKNOWN;
    }
    break;
    case UNC_KT_VRT_IF: {
      ConfigVal *tmp =
       (ck_val->get_cfg_val()) ? ck_val->get_cfg_val()->get_next_cfg_val() :
                                 NULL;
      val_db_vrt_if_st* vrt_if_dt_val =  reinterpret_cast<val_db_vrt_if_st *>
                        ((tmp != NULL) ? tmp->get_val() : NULL);
      if (vrt_if_dt_val == NULL) {
        UPLL_LOG_DEBUG("Invalid param");
        DELETE_IF_NOT_NULL(ck_val);
        return UPLL_RC_ERR_GENERIC;
      }
      vrt_if_dt_val->vrt_if_val_st.valid[0] = UNC_VF_VALID;
      vrt_if_dt_val->vrt_if_val_st.oper_status = UPLL_OPER_STATUS_UNKNOWN;
    }
    break;
    case UNC_KT_VTEP_IF: {
      ConfigVal *tmp =
       (ck_val->get_cfg_val()) ? ck_val->get_cfg_val()->get_next_cfg_val() :
                                 NULL;
      val_db_vtep_if_st* vtep_if_dt_val =  reinterpret_cast<val_db_vtep_if_st *>
                        ((tmp != NULL) ? tmp->get_val() : NULL);
      if (vtep_if_dt_val == NULL) {
        UPLL_LOG_DEBUG("Invalid param");
        DELETE_IF_NOT_NULL(ck_val);
        return UPLL_RC_ERR_GENERIC;
      }
      vtep_if_dt_val->vtep_if_val_st.valid[0] = UNC_VF_VALID;
      vtep_if_dt_val->vtep_if_val_st.oper_status = UPLL_OPER_STATUS_UNKNOWN;
    }
    break;
    case UNC_KT_VTUNNEL_IF: {
      ConfigVal *tmp =
       (ck_val->get_cfg_val()) ? ck_val->get_cfg_val()->get_next_cfg_val() :
                                 NULL;
      val_db_vtunnel_if_st* vtunnel_if_dt_val =
                                   reinterpret_cast<val_db_vtunnel_if_st *>
                                   ((tmp != NULL) ? tmp->get_val() : NULL);
      if (vtunnel_if_dt_val == NULL) {
        UPLL_LOG_DEBUG("Invalid param");
        DELETE_IF_NOT_NULL(ck_val);
        return UPLL_RC_ERR_GENERIC;
      }
      vtunnel_if_dt_val->vtunnel_if_val_st.valid[0] = UNC_VF_VALID;
      vtunnel_if_dt_val->vtunnel_if_val_st.oper_status =
                                          UPLL_OPER_STATUS_UNKNOWN;
    }
    break;
    case UNC_KT_VTERM_IF: {
      ConfigVal *tmp =
       (ck_val->get_cfg_val()) ? ck_val->get_cfg_val()->get_next_cfg_val() :
                                 NULL;
      val_db_vterm_if_st* vterm_if_dt_val =  reinterpret_cast<val_db_vterm_if_st *>
                        ((tmp != NULL) ? tmp->get_val() : NULL);
      if (vterm_if_dt_val == NULL) {
        UPLL_LOG_DEBUG("Invalid param");
        DELETE_IF_NOT_NULL(ck_val);
        return UPLL_RC_ERR_GENERIC;
      }
      vterm_if_dt_val->vterm_if_val_st.valid[0] = UNC_VF_VALID;
      vterm_if_dt_val->vterm_if_val_st.oper_status = UPLL_OPER_STATUS_UNKNOWN;
    }
    break;
    default:
      UPLL_LOG_DEBUG("OperStatus not supported for this data type");
      DELETE_IF_NOT_NULL(ck_val);
      return UPLL_RC_ERR_GENERIC;
    break;
  }
  DbSubOp dbop = { kOpNotRead, kOpMatchCtrlr, kOpInOutNone };
  result_code = mgr->UpdateConfigDB(ck_val, UPLL_DT_STATE, UNC_OP_UPDATE,
                               dmi, &dbop, MAINTBL);
  if (result_code != UPLL_RC_SUCCESS &&
      result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    UPLL_LOG_DEBUG("Error in Updating interface operstatus");
    DELETE_IF_NOT_NULL(ck_val);
    return result_code;
  }
  DELETE_IF_NOT_NULL(ck_val);
  return result_code;
}

upll_rc_t VtnMoMgr::RestoreVtnOperStatus(ConfigKeyVal *ck_val,
                                       DalDmlIntf *dmi,
                                       state_notification notification) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  ConfigKeyVal *ck_vtn = NULL;
  result_code = GetChildConfigKey(ck_vtn, ck_val);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetChildConfigKEy failed");
    return UPLL_RC_ERR_GENERIC;
  }
  DbSubOp dbop = {kOpReadSingle, kOpMatchNone,
                  kOpInOutNone};
  result_code = ReadConfigDB(ck_vtn, UPLL_DT_STATE, UNC_OP_READ, dbop, dmi,
                             MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("ReadConfigDB failed with result_code %d",
                    result_code);
    DELETE_IF_NOT_NULL(ck_vtn);
    return result_code;
  }
  result_code = SetOperStatus(ck_vtn, notification, dmi);
  DELETE_IF_NOT_NULL(ck_vtn);
  return result_code;
}

upll_rc_t VtnMoMgr::RestoreVtnCtrlrOperStatus(uint8_t *ctrlr_name,
                                       DalDmlIntf *dmi,
                                       state_notification notification) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *ck_vtn = NULL;
  result_code = GetChildConfigKey(ck_vtn, NULL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetChildConfigKey failed");
    return UPLL_RC_ERR_GENERIC;
  }
  SET_USER_DATA_CTRLR(ck_vtn, ctrlr_name);
  DbSubOp dbop = {kOpReadMultiple, kOpMatchCtrlr,
                  kOpInOutCtrlr | kOpInOutDomain};
  result_code = ReadConfigDB(ck_vtn, UPLL_DT_STATE, UNC_OP_READ, dbop, dmi,
                             CTRLRTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("ReadConfigDB failed with result_code %d",
                    result_code);
    DELETE_IF_NOT_NULL(ck_vtn);
    return result_code;
  }
  state_notification orig_notfn = notification;
  while (ck_vtn != NULL) {
    ConfigKeyVal *ck_vtn1 = ck_vtn->get_next_cfg_key_val();
    ck_vtn->set_next_cfg_key_val(NULL);
    val_vtn_ctrlr *ctrlr_val = reinterpret_cast<val_vtn_ctrlr *>
                                               (GetVal(ck_vtn));
    if (!ctrlr_val) {
      UPLL_LOG_DEBUG("val_vtn_ctrlr struct is NULL");
      DELETE_IF_NOT_NULL(ck_vtn);
      DELETE_IF_NOT_NULL(ck_vtn1);
      return UPLL_RC_ERR_GENERIC;
    }
    if (notification == kCtrlrDisconnect) {
      ctrlr_val->oper_status = UPLL_OPER_STATUS_UNKNOWN;
      ctrlr_val->down_count = 0;
    } else {
      if (ctrlr_val->oper_status != UPLL_OPER_STATUS_UNKNOWN) {
        DELETE_IF_NOT_NULL(ck_vtn);
        ck_vtn = ck_vtn1;
        continue;
      }
      result_code = RestoreVnodeOperStatus(ctrlr_name, ck_vtn, dmi, notification, false);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Error in Updating Operstatus on Controller ReConnect");
        DELETE_IF_NOT_NULL(ck_vtn);
        DELETE_IF_NOT_NULL(ck_vtn1);
        return result_code;
      }
      result_code = GetVbridgeUnKnownInstance(ck_vtn, ctrlr_name ,dmi);
      if (result_code == UPLL_RC_ERR_INSTANCE_EXISTS) {
        ctrlr_val->oper_status = UPLL_OPER_STATUS_UNKNOWN;
      } else if (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
        UPLL_LOG_DEBUG("Error in GetVbridgeUnKnownInstance");
        DELETE_IF_NOT_NULL(ck_vtn);
        DELETE_IF_NOT_NULL(ck_vtn1);
        return result_code;
      }
      switch (ctrlr_val->oper_status) {
      case UPLL_OPER_STATUS_UNKNOWN:
        notification = kCtrlrDisconnect;
      break;
      case UPLL_OPER_STATUS_DOWN:
        notification = kCtrlrReconnectIfDown;
      break;
      case UPLL_OPER_STATUS_UP:
        notification = kCtrlrReconnectIfUp;
      break;
      default:
        UPLL_LOG_DEBUG("Unexpected OperStatus");
        DELETE_IF_NOT_NULL(ck_vtn);
        DELETE_IF_NOT_NULL(ck_vtn1);
        return UPLL_RC_ERR_GENERIC;
      }
    }
    UPLL_LOG_DEBUG("ck_vtn %s\n",ck_vtn->ToStrAll().c_str());
    DbSubOp dbop = { kOpNotRead, kOpMatchCtrlr | kOpMatchDomain , kOpInOutNone };
    result_code = UpdateConfigDB(ck_vtn, UPLL_DT_STATE, UNC_OP_UPDATE,
                                 dmi, &dbop, CTRLRTBL);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Error in Updating VTN entries");
      DELETE_IF_NOT_NULL(ck_vtn);
      DELETE_IF_NOT_NULL(ck_vtn1);
      return result_code;
    }
    UPLL_LOG_DEBUG("ck_vtn %s\n",ck_vtn->ToStrAll().c_str());
    result_code = RestoreVtnOperStatus(ck_vtn, dmi, notification);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Returning error %d\n",result_code);
      DELETE_IF_NOT_NULL(ck_vtn);
      DELETE_IF_NOT_NULL(ck_vtn1);
      break;
    } 
    DELETE_IF_NOT_NULL(ck_vtn);
    notification = orig_notfn;
    ck_vtn = ck_vtn1;
  }
  return result_code;
}

upll_rc_t VtnMoMgr::RestoreVnodeOperStatus(uint8_t *ctrlr_id,
                                           ConfigKeyVal *ck_vtn_ctrlr,
                                           DalDmlIntf *dmi,
                                           state_notification notification,
                                           bool skip) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  uint32_t down_count = 0;
  
  result_code =  RestoreVnodeIfAndVtnCtrlr(ctrlr_id, ck_vtn_ctrlr,
                            dmi, notification, down_count);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Error in Updating OperStatus");
    return result_code;
  }
  val_vtn_ctrlr *tmp_ctrlr = reinterpret_cast<val_vtn_ctrlr*>
                              (GetVal(ck_vtn_ctrlr));
  if (tmp_ctrlr == NULL) {
    UPLL_LOG_DEBUG("Error in retrieving ctrlr val struct");
    return UPLL_RC_ERR_GENERIC;
  }
  tmp_ctrlr->down_count = down_count;
  tmp_ctrlr->oper_status = (down_count > 0)?UPLL_OPER_STATUS_DOWN:
                            UPLL_OPER_STATUS_UP;
#if 0
  DbSubOp dbop = { kOpNotRead, kOpMatchCtrlr, kOpInOutNone };
  result_code = UpdateConfigDB(ck_vtn_ctrlr, UPLL_DT_STATE, UNC_OP_UPDATE,
                               dmi, &dbop, CTRLRTBL);
  DELETE_IF_NOT_NULL(ck_vtn_ctrlr);
#endif
  return result_code;
}

upll_rc_t VtnMoMgr::RestoreVnodeIfAndVtnCtrlr(uint8_t *ctrlr_id,
                           ConfigKeyVal *ck_vtn,
                           DalDmlIntf *dmi,
                           state_notification notifn,
                           uint32_t &down_count) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS; 
  unc_key_type_t node_key_type[]= {UNC_KT_VBRIDGE, UNC_KT_VROUTER, UNC_KT_VTERMINAL};
  for (int node_count = 0;
       node_count < static_cast<int>
       (sizeof(node_key_type)/sizeof(unc_key_type_t));
       node_count++) {
    const unc_key_type_t ktype = node_key_type[node_count];
    ConfigKeyVal *ck_vnode = NULL;

    VnodeMoMgr *mgr =
        reinterpret_cast<VnodeMoMgr *>(const_cast<MoManager *>
                    (const_cast<MoManager*>(GetMoManager(ktype))));

    result_code = mgr->GetChildConfigKey(ck_vnode, ck_vtn);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("GetChildConfigKey failed");
      return UPLL_RC_ERR_GENERIC;
    }
    controller_domain_t ctrlr_dom = {NULL, NULL};
    GET_USER_DATA_CTRLR_DOMAIN(ck_vtn, ctrlr_dom);
    SET_USER_DATA_CTRLR_DOMAIN(ck_vnode, ctrlr_dom);
    DbSubOp dbop = {kOpReadMultiple, kOpMatchCtrlr| kOpMatchDomain,
                  kOpInOutNone};
    result_code = mgr->ReadConfigDB(ck_vnode, UPLL_DT_STATE, UNC_OP_READ, dbop, dmi,
                             MAINTBL);
    if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      result_code = UPLL_RC_SUCCESS;
      DELETE_IF_NOT_NULL(ck_vnode);
      continue;
    } else if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("ReadConfigDB failed with result_code %d",
                      result_code);
      DELETE_IF_NOT_NULL(ck_vnode);
      return result_code;
    }
    unc_key_type_t node_key_type[]= {UNC_KT_VBR_IF, UNC_KT_VRT_IF,
                                     UNC_KT_VTERM_IF};
    while (ck_vnode != NULL) {
      ConfigKeyVal *ckv_tmp = ck_vnode->get_next_cfg_key_val();
      ck_vnode->set_next_cfg_key_val(NULL);
      VnodeMoMgr *node_mgr =
          reinterpret_cast<VnodeMoMgr *>(const_cast<MoManager *>
          (const_cast<MoManager*>(GetMoManager(ck_vnode->get_key_type()))));
      VnodeChildMoMgr *if_mgr =
          reinterpret_cast<VnodeChildMoMgr *>(const_cast<MoManager *>
          (const_cast<MoManager*>(GetMoManager(node_key_type[node_count]))));
#if 0
      result_code = if_mgr->GetChildConfigKey(ck_if, ck_vnode);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("GetChildConfigKey failed");
        DELETE_IF_NOT_NULL(ckv_tmp);
        DELETE_IF_NOT_NULL(ck_vnode);
        return UPLL_RC_ERR_GENERIC;
      }
      result_code = if_mgr->UpdateVnodeIfOperStatus(ck_if, dmi, notifn);
      DELETE_IF_NOT_NULL(ck_if);
#else
      result_code = if_mgr->UpdateVnodeIfOperStatus(ck_vnode, dmi, notifn);
#endif
      if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
        /* if key_type is VBRIDGE and it is a stand alone vbridge,
         * then check if that VBRIDGE has default vlanmap configured under it */
        state_notification notification = kCtrlrReconnectIfDown;
        if (ck_vnode->get_key_type() == UNC_KT_VBRIDGE) {
          VlanMapMoMgr *vlan_mgr =
          reinterpret_cast<VlanMapMoMgr *>(const_cast<MoManager *>
          (const_cast<MoManager*>(GetMoManager(UNC_KT_VBR_VLANMAP))));
          result_code = vlan_mgr->CheckIfVnodeisVlanmapped(ck_vnode, dmi);
          if (result_code == UPLL_RC_ERR_INSTANCE_EXISTS) {
            // if stand alone VBRIDGE has default vlan configured,
            // then operstatus of that VBRIDGE is UP
            notification = kCtrlrReconnectIfUp;
          } else if (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
            UPLL_LOG_DEBUG("Error in Reading VlanMap DB");
            DELETE_IF_NOT_NULL(ck_vnode);
            DELETE_IF_NOT_NULL(ckv_tmp);
            return result_code;
          }
        }
        result_code = node_mgr->UpdateOperStatus(ck_vnode, dmi,
                                notification, true, true);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("Error in Updating OperStatus");
          DELETE_IF_NOT_NULL(ckv_tmp);
          DELETE_IF_NOT_NULL(ck_vnode);
          return result_code;
        }
        DELETE_IF_NOT_NULL(ck_vnode);
        ck_vnode = ckv_tmp;
        down_count = down_count + 1;
        result_code = UPLL_RC_SUCCESS;
        continue;
      } else if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("UpdateVnodeIfOperStatus failed with result_code %d",
                    result_code);
        DELETE_IF_NOT_NULL(ckv_tmp);
        DELETE_IF_NOT_NULL(ck_vnode);
        return result_code;
      }
      uint32_t cur_instance_count = 0;
      ConfigKeyVal *ck_vnode_tmp = NULL;

      result_code = node_mgr->GetCkvUninit(ck_vnode_tmp, ck_vnode, dmi);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Returning error %d\n", result_code);
        DELETE_IF_NOT_NULL(ckv_tmp);
        DELETE_IF_NOT_NULL(ck_vnode);
        return result_code;
      }
      val_db_vbr_st *vnode_stval = reinterpret_cast<val_db_vbr_st *>
                                            (GetStateVal(ck_vnode_tmp));
      if (vnode_stval == NULL) {
        UPLL_LOG_DEBUG("Error in fetching in state structure");
        DELETE_IF_NOT_NULL(ck_vnode_tmp);
        DELETE_IF_NOT_NULL(ckv_tmp);
        DELETE_IF_NOT_NULL(ck_vnode);
        return UPLL_RC_ERR_GENERIC;
      }
      vnode_stval->vbr_val_st.oper_status = UPLL_OPER_STATUS_DOWN;
      vnode_stval->vbr_val_st.valid[UPLL_IDX_OPER_STATUS_VBRS] = UNC_VF_VALID;

      result_code = node_mgr->GetInstanceCount(ck_vnode_tmp, NULL,
                                            UPLL_DT_STATE,
                                            &cur_instance_count, dmi, MAINTBL);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("GetInstanceCount failed : %d\n", result_code);
        DELETE_IF_NOT_NULL(ck_vnode_tmp);
        DELETE_IF_NOT_NULL(ckv_tmp);
        DELETE_IF_NOT_NULL(ck_vnode);
        return result_code;
      }
      down_count += cur_instance_count;
      if (ck_vnode->get_key_type() == UNC_KT_VBRIDGE) {
        result_code = UpdateVbridgeIfUnKnownInstance(ck_vnode, ctrlr_id, dmi);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("Error in UpdateVbridgeIfUnKnownInstance");
          DELETE_IF_NOT_NULL(ck_vnode_tmp);
          DELETE_IF_NOT_NULL(ckv_tmp);
          DELETE_IF_NOT_NULL(ck_vnode);
          return result_code;
        }
      }
      DELETE_IF_NOT_NULL(ck_vnode_tmp);
      DELETE_IF_NOT_NULL(ck_vnode);
      ck_vnode = ckv_tmp;
    }
  }
  return result_code;
}

upll_rc_t
VtnMoMgr:: VtnRenameMerge(ConfigKeyVal *ikey,
                                   ConfigKeyVal *okey,
                                   DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
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
				  UNC_OP_DELETE, dmi, &dbop, MAINTBL);
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
upll_rc_t VtnMoMgr::UpdateVbridgeIfUnKnownInstance(ConfigKeyVal *ck_vnode, uint8_t *ctrlr_id, DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  ConfigKeyVal *ck_vnode_if = NULL;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  VnodeChildMoMgr *node_if_mgr =
    reinterpret_cast<VnodeChildMoMgr *>(const_cast<MoManager *>
    (const_cast<MoManager*>(GetMoManager(UNC_KT_VBR_IF))));
  result_code = node_if_mgr->GetCkvUninit(ck_vnode_if, ck_vnode, dmi);
  SET_USER_DATA_CTRLR(ck_vnode_if, ctrlr_id);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Returning error %d\n", result_code);
    return result_code;
  }
  val_db_vbr_if_st *vnode_if_stval = reinterpret_cast<val_db_vbr_if_st *>
                                        (GetStateVal(ck_vnode_if));
  if (vnode_if_stval == NULL) {
    UPLL_LOG_DEBUG("Error in fetching in state structure");
    DELETE_IF_NOT_NULL(ck_vnode_if);
    return UPLL_RC_ERR_GENERIC;
  }
  vnode_if_stval->vbr_if_val_st.oper_status = UPLL_OPER_STATUS_UNKNOWN;
  vnode_if_stval->vbr_if_val_st.valid[UPLL_IDX_OPER_STATUS_VBRIS] = UNC_VF_VALID;

  DbSubOp dbop = { kOpReadSingle, kOpMatchCtrlr, kOpInOutNone };
  result_code = node_if_mgr->UpdateConfigDB(ck_vnode_if, UPLL_DT_STATE, UNC_OP_READ,
                                          dmi, &dbop, MAINTBL);
  DELETE_IF_NOT_NULL(ck_vnode_if);
  if (result_code == UPLL_RC_ERR_INSTANCE_EXISTS) {
    VnodeMoMgr *node_mgr =
    reinterpret_cast<VnodeMoMgr *>(const_cast<MoManager *>
    (const_cast<MoManager*>(GetMoManager(UNC_KT_VBRIDGE))));
    val_db_vbr_st *vnode_stval = reinterpret_cast<val_db_vbr_st *>
                                        (GetStateVal(ck_vnode));
    if (vnode_stval == NULL) {
      UPLL_LOG_DEBUG("Error in fetching in state structure");
      return UPLL_RC_ERR_GENERIC;
    }
    vnode_stval->vbr_val_st.oper_status = UPLL_OPER_STATUS_UNKNOWN;
    vnode_stval->vbr_val_st.valid[UPLL_IDX_OPER_STATUS_VBRS] = UNC_VF_VALID;
    
    result_code = node_mgr->UpdateConfigDB(ck_vnode, UPLL_DT_STATE, UNC_OP_UPDATE,
                                          dmi, &dbop, MAINTBL);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Error in Updating Vbridge DB");
      return result_code;
    }
  }
  return (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE? UPLL_RC_SUCCESS:result_code); 
}
upll_rc_t VtnMoMgr::GetVbridgeUnKnownInstance(ConfigKeyVal *ck_vtn, uint8_t *ctrlr_id, DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  ConfigKeyVal *ck_vnode = NULL;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  VnodeMoMgr *node_mgr =
    reinterpret_cast<VnodeMoMgr *>(const_cast<MoManager *>
    (const_cast<MoManager*>(GetMoManager(UNC_KT_VBRIDGE))));
  result_code = node_mgr->GetCkvUninit(ck_vnode, ck_vtn, dmi);
  SET_USER_DATA_CTRLR(ck_vnode, ctrlr_id);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Returning error %d\n", result_code);
    return result_code;
  }
  val_db_vbr_st *vnode_stval = reinterpret_cast<val_db_vbr_st *>
                                        (GetStateVal(ck_vnode));
  if (vnode_stval == NULL) {
    UPLL_LOG_DEBUG("Error in fetching in state structure");
    DELETE_IF_NOT_NULL(ck_vnode);
    return UPLL_RC_ERR_GENERIC;
  }
  vnode_stval->vbr_val_st.oper_status = UPLL_OPER_STATUS_UNKNOWN;
  vnode_stval->vbr_val_st.valid[UPLL_IDX_OPER_STATUS_VBRS] = UNC_VF_VALID;

  DbSubOp dbop = { kOpReadSingle, kOpMatchCtrlr, kOpInOutNone };
  result_code = node_mgr->UpdateConfigDB(ck_vnode, UPLL_DT_STATE, UNC_OP_READ,
                                          dmi, &dbop, MAINTBL);
  DELETE_IF_NOT_NULL(ck_vnode);
  return result_code;
}

}  // namespace kt_momgr
}  // namespace upll
}  // namespace unc
