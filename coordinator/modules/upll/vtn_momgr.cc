/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include "vtn_momgr.hh"
#include "vbr_if_momgr.hh"
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
    { uudst::vtn_controller::kDbiOperStatus, CFG_VAL, offsetof(val_vtn_ctrlr,
                                                        oper_status),
      uud::kDalUint8, 1 },
    { uudst::vtn_controller::kDbiAlarmStatus, CFG_VAL, offsetof(val_vtn_ctrlr,
                                                         alarm_status),
      uud::kDalUint8, 1 },
    { uudst::vtn_controller::kDbiDownCount, CFG_VAL, offsetof(val_vtn_ctrlr,
                                                       down_count),
      uud::kDalUint32, 1 },
    { uudst::vtn_controller::kDbiRefCount, CFG_VAL, offsetof(val_vtn_ctrlr,
      ref_count), uud::kDalUint32, 1 },
    { uudst::vtn_controller::kDbiValidOperStatus, CFG_META_VAL,
      offsetof(val_vtn_ctrlr, valid[0]),
      uud::kDalUint8, 1 },
    { uudst::vtn_controller::kDbiValidAlarmStatus, CFG_META_VAL, offsetof(
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

unc_key_type_t VtnMoMgr::vtn_child[] = { UNC_KT_VBRIDGE, UNC_KT_VROUTER,
                                         UNC_KT_VUNKNOWN, UNC_KT_VLINK,
                                         UNC_KT_VTN_FLOWFILTER,
                                         UNC_KT_VTN_POLICINGMAP };
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
  if (RENAMETBL == tbl){ 
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
  key_vtn *vtn_key = reinterpret_cast<key_vtn *>(key);
  UPLL_LOG_TRACE("Entering IsValidKey");
  bool ret_val = UPLL_RC_SUCCESS;

  ret_val = ValidateKey(reinterpret_cast<char *>(vtn_key->vtn_name),
                        kMinLenVtnName, kMaxLenVtnName);

  if (ret_val != UPLL_RC_SUCCESS) {
    UPLL_LOG_TRACE("VTN Name is not valid(%d)", ret_val);
    return false;
  }
  UPLL_LOG_TRACE("Leaving IsValidKey");
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
      switch(indx) {
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
      val = reinterpret_cast<void *>(malloc(sizeof(val_vtn)));
      if (!val) {
        UPLL_LOG_ERROR("val Memory Allocation failed");
        return UPLL_RC_ERR_GENERIC;
      }
      memset(val, 0, sizeof(val_vtn));
      ck_val = new ConfigVal(IpctSt::kIpcStValVtn, val);
      if (dt_type == UPLL_DT_STATE) {
        val = reinterpret_cast<void *>(malloc(sizeof(val_db_vtn_st)));
        if (!val) {
          DELETE_IF_NOT_NULL(ck_val);
          UPLL_LOG_ERROR("val Memory Allocation failed");
          return UPLL_RC_ERR_GENERIC;
        }
        memset(val, 0, sizeof(val_db_vtn_st));
        ConfigVal *ck_nxtval = new ConfigVal(IpctSt::kIpcStValVtnSt, val);
        ck_val->AppendCfgVal(ck_nxtval);
      }
      break;
    case RENAMETBL:
      val = reinterpret_cast<void *>(malloc(sizeof(val_rename_vtn)));
      if (!val) {
        UPLL_LOG_ERROR("val Memory Allocation failed");
        return UPLL_RC_ERR_GENERIC;
      }
      memset(val, 0, sizeof(val_rename_vtn));
      ck_val = new ConfigVal(IpctSt::kIpcStValRenameVtn, val);
      break;
    case CTRLRTBL:
      val = reinterpret_cast<void *>(malloc(sizeof(val_vtn_ctrlr)));
      if (!val) {
        UPLL_LOG_ERROR("val Memory Allocation failed");
        return UPLL_RC_ERR_GENERIC;
      }
      memset(val, 0, sizeof(val_vtn_ctrlr));
      ck_val = new ConfigVal(IpctSt::kIpcInvalidStNum, val);
      break;
    default:
      val = NULL;
      break;
  }
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
    //error user data nneds to be copied
    SET_USER_DATA(tmp1, tmp)
    tmp = tmp->get_next_cfg_val();
  };
  if (tmp) {
    if (tbl == MAINTBL) {
      val_db_vtn_st *ival = reinterpret_cast<val_db_vtn_st *>(tmp->get_val());
      val_db_vtn_st *val_vtn = reinterpret_cast<val_db_vtn_st *>(
          ConfigKeyVal::Malloc(sizeof(val_db_vtn_st)));
      memset(reinterpret_cast<void *>(val_vtn),0,sizeof(val_db_vtn_st));
      memcpy(reinterpret_cast<char *>(val_vtn), reinterpret_cast<char *>(ival),
             sizeof(val_db_vtn_st));
      ConfigVal *tmp2 = new ConfigVal(IpctSt::kIpcStValVtnSt, val_vtn);
      if (!tmp2) {
        UPLL_LOG_ERROR("Memory Allocation failed for tmp2");
        FREE_IF_NOT_NULL(val_vtn);
        delete tmp1;
        return UPLL_RC_ERR_GENERIC;
      }
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
  if (okey) {
    SET_USER_DATA(okey, req)
  } else {
    delete tmp1;
    return UPLL_RC_ERR_GENERIC;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtnMoMgr::AdaptValToVtnService(ConfigKeyVal *ikey) {
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
                         (malloc(sizeof(val_vtn_st)));
        if (!vtn_val_st) {
          UPLL_LOG_ERROR("Memory Allocation failed");
          return UPLL_RC_ERR_GENERIC;
        }
        val_db_vtn_st *db_vtn_val_st = reinterpret_cast<val_db_vtn_st *>
                                     (cval->get_val());
        memcpy(vtn_val_st, &(db_vtn_val_st->vtn_val_st),
               sizeof(val_vtn_st));
        cval->SetVal(IpctSt::kIpcStValVtnSt, vtn_val_st);
      }
      cval = cval->get_next_cfg_val();
    }
    ikey = ikey->get_next_cfg_key_val();
  }
  UPLL_LOG_DEBUG("Exiting VtnMoMgr::AdaptValToVtnService");
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtnMoMgr::GetChildConfigKey(ConfigKeyVal *&okey,
                                      ConfigKeyVal *parent_key) {
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  key_vtn_t *vtn_key = reinterpret_cast<key_vtn_t *>(malloc(sizeof(key_vtn)));
  if (!vtn_key) {
    UPLL_LOG_ERROR("malloc failed");
    return UPLL_RC_ERR_GENERIC;
  }
  memset(vtn_key, 0, sizeof(key_vtn));
  void *pkey;
  if (parent_key == NULL) {
    okey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, vtn_key, NULL);
    return UPLL_RC_SUCCESS;
  } else {
    pkey = parent_key->get_key();
  }
  if (!pkey) {
    free(vtn_key);
    return UPLL_RC_ERR_GENERIC;
  }
  switch (parent_key->get_key_type()) {
    case UNC_KT_ROOT:
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
      free(vtn_key);
      return UPLL_RC_ERR_GENERIC;
  }
  okey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, vtn_key, NULL);
  if (okey == NULL) {
    free(vtn_key);
    result_code = UPLL_RC_ERR_GENERIC;
  } else {
    SET_USER_DATA(okey, parent_key);
  }
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
  val_rename_vtn *rename_vtn = reinterpret_cast<val_rename_vtn *>(malloc(
                                sizeof(val_rename_vtn)));
  if (!rename_vtn) {
   UPLL_LOG_DEBUG("Memory Allocation Failed");
   return UPLL_RC_ERR_GENERIC;
  }
  uuu::upll_strncpy(reinterpret_cast<char*>(rename_vtn->new_name),
                  reinterpret_cast<char*>(ctrlr_vtn_key->vtn_name),
                  strlen(reinterpret_cast<char*>(ctrlr_vtn_key->vtn_name)) + 1);
  rename_vtn->valid[UPLL_IDX_NEW_NAME_RVTN] = UNC_VF_VALID;
  result_code = GetChildConfigKey(unc_key, NULL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG(" GetChildConfigKey Failed with result_code %d",
                        result_code);
    free(rename_vtn);
    return UPLL_RC_ERR_GENERIC;
  }
  unc_key->AppendCfgVal(IpctSt::kIpcStValRenameVtn, rename_vtn);
  SET_USER_DATA_CTRLR(unc_key, ctrlr_id);
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
    IsRenamed(ikey, dt_type, dmi, rename);
    if (!rename) return UPLL_RC_SUCCESS;

    /* vtn renamed */
    if (rename & 0x01) {
      GET_USER_DATA_CTRLR_DOMAIN(ikey, *ctrlr_dom);
      if (!ctrlr_dom->ctrlr || !ctrlr_dom->domain) {
        UPLL_LOG_ERROR("Illegal controller domain");
        return UPLL_RC_ERR_GENERIC;
      }
      GetChildConfigKey(okey, ikey);
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
      if (!rename_val ||
         (rename_val->valid[UPLL_IDX_NEW_NAME_RVTN] != UNC_VF_VALID))
         return UPLL_RC_ERR_GENERIC;
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
      UPLL_LOG_TRACE(" ctrlr = %s , dom = %s ", tmp_ctrlr_dom.ctrlr, tmp_ctrlr_dom.domain);
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
     val_db_vtn_st *vtn_val_db_st = NULL;

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
       vtn_val_db_st =  reinterpret_cast<val_db_vtn_st *>
                                       (malloc(sizeof(val_db_vtn_st)));
       if (!vtn_val_db_st) {
          UPLL_LOG_DEBUG("invalid st val ");
          return UPLL_RC_ERR_GENERIC;
       }
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
        vtn_val->cs_row_status = UNC_CS_NOT_APPLIED;
        if (vtn_val->valid[UPLL_IDX_DESC_VTN] == UNC_VF_VALID) {
            vtn_val->cs_attr[UPLL_IDX_DESC_VTN] = UNC_CS_APPLIED;
        }
        val_vtnst->creation_time = time(NULL);
        val_vtnst->last_updated_time = val_vtnst->creation_time;
        val_vtnst->valid[UPLL_IDX_CREATION_TIME_VS] = UNC_VF_VALID;
        val_vtnst->valid[UPLL_IDX_LAST_UPDATE_TIME_VS] = UNC_VF_VALID;
        val_vtnst->oper_status = UPLL_OPER_STATUS_UP;
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
        val_vtnst->last_updated_time = time(NULL);
        val_vtnst->valid[UPLL_IDX_LAST_UPDATE_TIME_VS] = UNC_VF_VALID;
        val_vtnst->valid[UPLL_IDX_OPER_STATUS_VS] = UNC_VF_INVALID;
        val_vtnst->valid[UPLL_IDX_ALARM_STATUS_VS] = UNC_VF_INVALID;
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
     result_code = UpdateConfigDB(ck_vtn, UPLL_DT_STATE, op, dmi, MAINTBL);
     EnqueCfgNotification(op, UPLL_DT_RUNNING, ck_vtn);
     delete ck_vtn;
     return result_code;
  }

  upll_rc_t
  VtnMoMgr::UpdateConfigStatus(ConfigKeyVal *vtn_key,
      unc_keytype_operation_t op, uint32_t driver_result, ConfigKeyVal *nreq,
      DalDmlIntf *dmi, ConfigKeyVal *ctrlr_key) {
    UPLL_FUNC_TRACE;
    upll_rc_t result_code = UPLL_RC_SUCCESS;
    unc_keytype_configstatus_t status = UNC_CS_UNKNOWN, cs_status;
    cs_status = (driver_result == UPLL_RC_SUCCESS)?
                 UNC_CS_APPLIED: UNC_CS_NOT_APPLIED;
    val_vtn_t *vtn_val = reinterpret_cast<val_vtn_t *>(GetVal(vtn_key));
    val_vtn_ctrlr *ctrlr_val_vtn = reinterpret_cast<val_vtn_ctrlr *>
    (GetVal(ctrlr_key));
    if ((vtn_val == NULL) || (ctrlr_val_vtn == NULL))
    return UPLL_RC_ERR_GENERIC;

    if (op == UNC_OP_CREATE) {
      ctrlr_val_vtn->oper_status = UPLL_OPER_STATUS_UP;
      ctrlr_val_vtn->alarm_status = UPLL_ALARM_CLEAR;
      ctrlr_val_vtn->valid[0] = UNC_VF_VALID;
      ctrlr_val_vtn->valid[1] = UNC_VF_VALID;

      /* update the vtn status in main tbl */
      switch (vtn_val->cs_row_status) {
        case UNC_CS_UNKNOWN:
          status = cs_status;
          break;
        case UNC_CS_PARTAILLY_APPLIED:
          if (ctrlr_val_vtn->cs_row_status == UNC_CS_NOT_APPLIED) {
            // Todo: if this vtn has caused it then to change to applied.
            status = (cs_status != UNC_CS_APPLIED) ?
                      UNC_CS_PARTAILLY_APPLIED : cs_status;
          }
          break;
        case UNC_CS_APPLIED:
        case UNC_CS_NOT_APPLIED:
        case UNC_CS_INVALID:
        default:
          status = (cs_status == UNC_CS_NOT_APPLIED)?
                                UNC_CS_PARTAILLY_APPLIED:status;
          break;
      }
      vtn_val->cs_row_status = status;
    } else if (op == UNC_OP_UPDATE) {
      val_vtn *vtnval2 = reinterpret_cast<val_vtn *>(GetVal(nreq));

      if ( (UNC_VF_VALID == vtn_val->valid[UPLL_IDX_DESC_VTN]) &&
          (UNC_VF_VALID == vtnval2->valid[UPLL_IDX_DESC_VTN])) {
        if (!strcmp(reinterpret_cast<char*>(vtn_val->description),
                 reinterpret_cast<char*>(vtnval2->description)) )
        vtn_val->valid[UPLL_IDX_DESC_VTN] = UNC_VF_INVALID;
      } else if (UNC_VF_NOT_SOPPORTED == vtn_val->valid[UPLL_IDX_DESC_VTN]) {
        vtn_val->cs_attr[UPLL_IDX_DESC_VTN] = UNC_CS_NOT_SUPPORTED;
      }
    }
    if (ctrlr_val_vtn->cs_attr[UPLL_IDX_DESC_VTN] != UNC_CS_NOT_SUPPORTED)
      ctrlr_val_vtn->cs_attr[UPLL_IDX_DESC_VTN] = cs_status;
    else
      ctrlr_val_vtn->cs_attr[UPLL_IDX_DESC_VTN] = UNC_CS_NOT_SUPPORTED;
    vtn_val->cs_attr[UPLL_IDX_DESC_VTN] = UNC_CS_APPLIED;
    return result_code;
  }

  upll_rc_t
  VtnMoMgr::TxCopyCandidateToRunning(unc_key_type_t keytype,
                                     CtrlrCommitStatusList *ctrlr_commit_status,
                                     DalDmlIntf *dmi) {
    UPLL_FUNC_TRACE;
    upll_rc_t result_code = UPLL_RC_SUCCESS;
    DalResultCode db_result;
    unc_keytype_operation_t op[] = {UNC_OP_DELETE,
                                    UNC_OP_CREATE, UNC_OP_UPDATE};
    int nop = sizeof(op) / sizeof(op[0]);
    ConfigKeyVal *vtn_key = NULL, *req = NULL, *nreq = NULL;
    DalCursor *cfg1_cursor;
    uint8_t *ctrlr_id = NULL;
    map<string, int> ctrlr_result;
    CtrlrCommitStatusList::iterator ccsListItr;
    CtrlrCommitStatus *ccStatusPtr;

    if ((ctrlr_commit_status == NULL) || (dmi == NULL))
    return UPLL_RC_ERR_GENERIC;
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
          if (result_code != UPLL_RC_SUCCESS)
          return result_code;
        }
      }
    }

    for (int i = 0; i < nop; i++) {
      cfg1_cursor = NULL;
      if (op[i] != UNC_OP_UPDATE) {
         result_code = DiffConfigDB(UPLL_DT_CANDIDATE, UPLL_DT_RUNNING, op[i],
                               req, nreq, &cfg1_cursor, dmi, MAINTBL);
         if (result_code != UPLL_RC_SUCCESS || cfg1_cursor == NULL) {
           UPLL_LOG_DEBUG("Cursor not populated");
           return result_code;
         }
         while (result_code == UPLL_RC_SUCCESS) {
           db_result = dmi->GetNextRecord(cfg1_cursor);
           result_code = DalToUpllResCode(db_result);
           if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
              result_code = UPLL_RC_SUCCESS;
              break;
           }
           result_code = UpdateVtnConfigStatus(req, op[i], UPLL_RC_SUCCESS,
                               nreq, dmi);
           if (result_code != UPLL_RC_SUCCESS) {
             UPLL_LOG_DEBUG("Error updating vtn config status %d",
                             result_code);
             if (cfg1_cursor)
               dmi->CloseCursor(cfg1_cursor, true);
             delete req;
             return result_code;
           }
         }
         if (cfg1_cursor)
           dmi->CloseCursor(cfg1_cursor, true);
         if (req)
          delete req;
         req = NULL;
      }
      UPLL_LOG_DEBUG("done with op %d ", op[i]);
    }
    for (int i = 0; i < nop; i++) {
      cfg1_cursor = NULL;
      MoMgrTables tbl = (op[i] == UNC_OP_UPDATE)?MAINTBL:CTRLRTBL;
      result_code = DiffConfigDB(UPLL_DT_CANDIDATE, UPLL_DT_RUNNING, op[i], req,
          nreq, &cfg1_cursor, dmi, tbl);
      if (result_code != UPLL_RC_SUCCESS || cfg1_cursor == NULL) {
        UPLL_LOG_DEBUG("Cursor not populated");
        return result_code;
      }
      ConfigKeyVal *vtn_ctrlr_key = NULL;
      while (result_code == UPLL_RC_SUCCESS) {
        db_result = dmi->GetNextRecord(cfg1_cursor);
        result_code = DalToUpllResCode(db_result);
        if (result_code != UPLL_RC_SUCCESS)
           break;
        if (op[i] == UNC_OP_UPDATE) {
          DbSubOp dbop = {kOpReadMultiple, kOpMatchNone, 
                                      kOpInOutCtrlr | kOpInOutDomain };
          result_code = GetChildConfigKey(vtn_ctrlr_key, req);
          if (result_code != UPLL_RC_SUCCESS || vtn_ctrlr_key == NULL)
          return result_code;
#if 0
          // Capability check
          req_header->operation = op[i];
          req_header->datatype = UPLL_DT_CANDIDATE;
          result_code = ValidateCapability(req_header, vtn_ctrlr_key);
          if (result_code != UPLL_RC_SUCCESS)
          return result_code;
#endif
          result_code = ReadConfigDB(vtn_ctrlr_key, UPLL_DT_CANDIDATE,
              UNC_OP_READ, dbop, dmi, CTRLRTBL);
          if (result_code != UPLL_RC_SUCCESS) {
            delete vtn_ctrlr_key;
            if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
              result_code = UpdateVtnConfigStatus(req, op[i], UPLL_RC_SUCCESS,
                               nreq, dmi);
              if (result_code != UPLL_RC_SUCCESS) {
                UPLL_LOG_DEBUG("Error updating vtn config status %d",
                                   result_code);
                if (cfg1_cursor)
                  dmi->CloseCursor(cfg1_cursor, true);
                delete req;
                delete nreq;
                return result_code;
              } else {
                continue;
              }
            } else  {
              if (cfg1_cursor)
                dmi->CloseCursor(cfg1_cursor, true);
              delete req;
              delete nreq;
              return result_code;
            }
          }
          result_code = DupConfigKeyVal(vtn_key, req, tbl);
          if (result_code != UPLL_RC_SUCCESS || vtn_key != NULL) {
            if (cfg1_cursor)
              dmi->CloseCursor(cfg1_cursor, true);
            delete req;
            delete nreq;
            return result_code;
          }
          for (ConfigKeyVal *tmp = vtn_ctrlr_key; tmp != NULL;
              tmp = tmp->get_next_cfg_key_val()) {
            GET_USER_DATA_CTRLR(tmp, ctrlr_id);
            string controller(reinterpret_cast<char *>(ctrlr_id));
            result_code = UpdateConfigStatus(vtn_key, op[i],
                ctrlr_result[controller], nreq, dmi, tmp);
            if (result_code != UPLL_RC_SUCCESS)
            break;
            result_code = UpdateConfigDB(tmp, UPLL_DT_RUNNING, op[i],
                dmi, CTRLRTBL);
            if (result_code != UPLL_RC_SUCCESS) {
              if (cfg1_cursor)
                dmi->CloseCursor(cfg1_cursor, true);
              delete req;
              delete nreq;
              return result_code;
            }
          }
          result_code = UpdateConfigDB(vtn_key, UPLL_DT_RUNNING, op[i],
                dmi, MAINTBL);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("Returning %d", result_code);
            if (cfg1_cursor)
              dmi->CloseCursor(cfg1_cursor, true);
            delete req;
            delete nreq;
            return result_code;
          }
          EnqueCfgNotification(op[i], UPLL_DT_RUNNING, vtn_key);
        } else {
          if (op[i] == UNC_OP_CREATE) {
            DbSubOp dbop = {kOpReadSingle, kOpMatchNone, kOpInOutFlag};
            result_code = GetChildConfigKey(vtn_key, req);
            result_code = ReadConfigDB(vtn_key, UPLL_DT_CANDIDATE, UNC_OP_READ,
                dbop, dmi, MAINTBL);
            if (result_code != UPLL_RC_SUCCESS) {
              if (cfg1_cursor)
                dmi->CloseCursor(cfg1_cursor, true);
              delete req;
              return result_code;
            }
#if 0
            // Capability check
            req_header->operation = op[i];
            req_header->datatype = UPLL_DT_CANDIDATE;
            result_code = ValidateCapability(req_header, vtn_ctrlr_key);
            if (result_code != UPLL_RC_SUCCESS)
            return result_code;
#endif
            result_code = DupConfigKeyVal(vtn_ctrlr_key, req, tbl);
            if (result_code != UPLL_RC_SUCCESS || vtn_ctrlr_key == NULL) {
              if (cfg1_cursor)
                dmi->CloseCursor(cfg1_cursor, true);
              delete req;
              return result_code;
            }
            GET_USER_DATA_CTRLR(vtn_ctrlr_key, ctrlr_id);
            string controller(reinterpret_cast<char *>(ctrlr_id));
            result_code = UpdateConfigStatus(vtn_key, op[i],
                           ctrlr_result[controller], NULL, dmi, vtn_ctrlr_key);
          } else if (op[i] == UNC_OP_DELETE) {
            result_code = GetChildConfigKey(vtn_ctrlr_key, req);
#if 0
            DbSubOp dbop = {kOpReadSingle, kOpMatchNone, kOpInOutFlag};
            result_code = ReadConfigDB(vtn_key, UPLL_DT_CANDIDATE, UNC_OP_READ,
                dbop, dmi, MAINTBL);
            if (result_code != UPLL_RC_SUCCESS)
            return result_code;
#endif
          }
          if (result_code != UPLL_RC_SUCCESS) {
            if (cfg1_cursor)
              dmi->CloseCursor(cfg1_cursor, true);
            delete req;
            return result_code;
          }
          result_code = UpdateConfigDB(vtn_ctrlr_key, UPLL_DT_RUNNING, op[i],
              dmi, CTRLRTBL);
          if (result_code != UPLL_RC_SUCCESS) {
            if (cfg1_cursor)
              dmi->CloseCursor(cfg1_cursor, true);
            delete req;
            return result_code;
          }
          if (op[i] != UNC_OP_DELETE) {
            result_code = UpdateConfigDB(vtn_key, UPLL_DT_RUNNING,
                UNC_OP_UPDATE, dmi, MAINTBL);
            if (result_code != UPLL_RC_SUCCESS) {
              if (cfg1_cursor)
                dmi->CloseCursor(cfg1_cursor, true);
              delete req;
              return result_code;
            }
          }
          EnqueCfgNotification(op[i], UPLL_DT_RUNNING, vtn_key);
        }
#if 1
        if (vtn_ctrlr_key)
          delete vtn_ctrlr_key;
#endif
        if (vtn_key)
          delete vtn_key;
        vtn_key = vtn_ctrlr_key = NULL;
        result_code = DalToUpllResCode(db_result);
      }
      if (cfg1_cursor)
        dmi->CloseCursor(cfg1_cursor, true);
      if (req)
      delete req;
      if (nreq)
      delete nreq;
      nreq = req = NULL;
    }
    result_code = (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE)?
                                  UPLL_RC_SUCCESS:result_code;
    return result_code;
  }
  
upll_rc_t VtnMoMgr::ReadSingleCtlrlStation(IpcReqRespHeader *header,
                                             ConfigKeyVal *ikey,
                                             DalDmlIntf *dmi,
                                             uint32_t &rec_count) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  controller_domain ctrlr_dom;
  ctrlr_dom.ctrlr = NULL;
  ctrlr_dom.domain = NULL;
  key_vtnstation_controller *stkey = NULL;
  ConfigKeyVal *ck_ctrlr = NULL;
  DbSubOp op = {kOpReadMultiple, kOpMatchCtrlr, kOpInOutCtrlr | kOpInOutDomain};
  stkey = reinterpret_cast<key_vtnstation_controller *>(ikey->get_key());
  IpcRequest ipc_req;
  memset(&ipc_req, 0, sizeof(ipc_req));
  memcpy(&(ipc_req.header), header, sizeof(IpcReqRespHeader));
  ipc_req.header.operation = UNC_OP_READ;
  val_vtnstation_controller_st_t *in_valst = NULL;
  val_vtnstation_controller_st_t *valst = NULL;
  ConfigVal *cfgval = NULL;
  ConfigKeyVal *ckv_drv = NULL;
  bool firstcfg = true;
  /* Memory Allocation for ck_ctrlr with VTN keytype */
  if(GetChildConfigKey(ck_ctrlr, NULL)) {
    UPLL_LOG_INFO("GetChildConfigKey failed");
    return UPLL_RC_ERR_GENERIC;
  }
  /* Get the vtn or domain list from the vtn ctrlr table */
  in_valst = reinterpret_cast<val_vtnstation_controller_st_t *>(GetVal(ikey));
  if (in_valst) {
    firstcfg = true;
    if (in_valst->valid[UPLL_IDX_VTN_NAME_VSCS] == UNC_VF_VALID)
    uuu::upll_strncpy(reinterpret_cast<key_vtn *>
      (ck_ctrlr->get_key())->vtn_name, in_valst->vtn_name, (kMaxLenVtnName + 1));
    if (in_valst->valid[UPLL_IDX_DOMAIN_ID_VSCS] == UNC_VF_VALID) {
      ctrlr_dom.domain = in_valst->domain_id;
      op.matchop  = kOpMatchCtrlr | kOpMatchDomain;
    }
  }
  ctrlr_dom.ctrlr =  stkey->controller_name;
  SET_USER_DATA_CTRLR_DOMAIN(ck_ctrlr, ctrlr_dom);
  result_code = ReadConfigDB(ck_ctrlr, UPLL_DT_RUNNING,
                                     UNC_OP_READ, op, dmi, CTRLRTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("ReadConfigDB failed. err_code %d\n",
                           result_code);
    return result_code;
  }
  //cfgval = (ikey->get_cfg_val());
  ckv_drv = new ConfigKeyVal(UNC_KT_VTNSTATION_CONTROLLER,
               IpctSt::kIpcStKeyVtnstationController, ikey->get_key(), NULL); 
  for (; ck_ctrlr; ck_ctrlr = ck_ctrlr->get_next_cfg_key_val()) {
    IpcResponse ipc_resp;
    memset(&(ipc_resp),0,sizeof(IpcResponse)); 
    valst = reinterpret_cast<val_vtnstation_controller_st_t *>
	    (ConfigKeyVal::Malloc(sizeof(val_vtnstation_controller_st_t)));
    if (in_valst) {
      memcpy(valst, in_valst, sizeof(val_vtnstation_controller_st_t));  
      if (firstcfg)
        ikey->SetCfgVal(NULL);
        firstcfg =false;
    } 
    valst->valid[UPLL_IDX_VTN_NAME_VSCS]  = UNC_VF_VALID;
    valst->valid[UPLL_IDX_DOMAIN_ID_VSCS] = UNC_VF_VALID; 
    result_code = GetRenamedControllerKey(ck_ctrlr, UPLL_DT_RUNNING, dmi,
		    &ctrlr_dom);
    if (result_code != UPLL_RC_SUCCESS && result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      UPLL_LOG_DEBUG("GetRenamedControllerKey Failed result_code - %d\n", result_code);
      return result_code;
    }
    GET_USER_DATA_CTRLR_DOMAIN(ck_ctrlr,ctrlr_dom); 
    UPLL_LOG_DEBUG("Controller id and domain id are %s %s\n", ctrlr_dom.ctrlr,
		    ctrlr_dom.domain);
    uuu::upll_strncpy(valst->vtn_name, reinterpret_cast<key_vtn_t *>
		    (ck_ctrlr->get_key())->vtn_name, (kMaxLenVtnName + 1 ));
    uuu::upll_strncpy(valst->domain_id, ctrlr_dom.domain, (kMaxLenDomainId + 1));
#if 0
    if (!cfgval) {
	    cfgval = new ConfigVal(IpctSt::kIpcStValVtnSt, valst);
    } else {
	    cfgval->AppendCfgVal(new ConfigVal(IpctSt::kIpcStValVtnSt, valst));
    }
#endif
    cfgval = new ConfigVal(IpctSt::kIpcStValVtnstationControllerSt, valst);
    ckv_drv->SetCfgVal(cfgval);      
    ipc_req.ckv_data = ckv_drv;
    UPLL_LOG_DEBUG("Driver Req Ckv_data is %s\n", ckv_drv->ToStrAll().c_str());
    if (!IpcUtil::SendReqToDriver((const char *)(ctrlr_dom.ctrlr),
			    reinterpret_cast<char *>(ctrlr_dom.domain), PFCDRIVER_SERVICE_NAME,
			    PFCDRIVER_SVID_LOGICAL, &ipc_req, true, &ipc_resp)) {
      UPLL_LOG_INFO("Request to driver for Key %d for controller %s failed ",
			    ikey->get_key_type(), reinterpret_cast<char *>(ctrlr_dom.ctrlr));
      return ipc_resp.header.result_code; 
    }
    if (ipc_resp.header.result_code != UPLL_RC_SUCCESS 
		    && ipc_resp.header.result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      UPLL_LOG_INFO("Read from driver failed err code %d\n",
			    ipc_resp.header.result_code);
      return ipc_resp.header.result_code;
    }
    else if (ipc_resp.header.result_code == UPLL_RC_SUCCESS) {
      rec_count++;
      if (ipc_resp.ckv_data == NULL) {
        UPLL_LOG_DEBUG("Ipc Response ckv_data is NUll %d\n",
			      ipc_resp.header.result_code);  
	return UPLL_RC_ERR_GENERIC;
      }
      UPLL_LOG_DEBUG("Driver Res Ckv_data is %s\n", (ipc_resp.ckv_data)->ToStrAll().c_str());
      ckv_drv->ResetWith(ipc_resp.ckv_data); 
      UPLL_LOG_DEBUG("After reset Ckv_data is %s\n", ckv_drv->ToStrAll().c_str());
      result_code = MappingvExtTovBr(ckv_drv, header, dmi);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("MappingvExtTovBr failed result_code - %d\n", result_code);
	return result_code;
      }
      ikey->AppendCfgVal(ckv_drv->get_cfg_val());  
      UPLL_LOG_DEBUG("ikey is %s\n", ikey->ToStrAll().c_str());
    }
    valst = NULL;
    cfgval = NULL;
  }
  return result_code;
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
    DbSubOp op = {kOpReadMultiple, kOpMatchCtrlr, kOpInOutCtrlr | kOpInOutDomain};
    if ((ikey->get_key_type() == UNC_KT_VTN_MAPPING_CONTROLLER) ||
        (ikey->get_key_type() == UNC_KT_VTNSTATION_CONTROLLER)) {
      result_code = ValidateMessage(header, ikey);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("ValidateMessage failed result_code %d\n",
                    result_code);
        return result_code;
      }
    }
    IpcRequest ipc_req;
    memset(&ipc_req, 0, sizeof(ipc_req));
    memcpy(&(ipc_req.header), header, sizeof(IpcReqRespHeader));
    uint32_t rec_count = 0;
    switch(ikey->get_key_type()) {
      case UNC_KT_VTN:
        result_code = MoMgrImpl::ReadMo(header, ikey, dmi);
        return result_code;
        break;
      case UNC_KT_VTNSTATION_CONTROLLER:
        result_code = ReadSingleCtlrlStation(header, ikey,
                                              dmi,rec_count); 
        header->rep_count = rec_count;
        break;
      case UNC_KT_VTN_MAPPING_CONTROLLER:
        IpcResponse ipc_resp;
        memset(&ipc_resp, 0, sizeof(IpcResponse));
        mkey = reinterpret_cast<key_vtn_controller *>(ikey->get_key());
        ctrlr_dom.ctrlr = mkey->controller_name;
        ctrlr_dom.domain = mkey->domain_id;
        result_code = GetChildConfigKey(ck_ctrlr, ikey); 
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_INFO("GetChildConfigKey failed err code %d\n", result_code);
          return result_code;
        } 
        op.readop = kOpReadExist; 
        op.matchop = kOpMatchCtrlr | kOpMatchDomain;
        SET_USER_DATA_CTRLR_DOMAIN(ck_ctrlr, ctrlr_dom);
        result_code = UpdateConfigDB(ck_ctrlr, UPLL_DT_RUNNING,
              UNC_OP_READ, dmi, &op, CTRLRTBL);
        if (result_code != UPLL_RC_ERR_INSTANCE_EXISTS) {
          UPLL_LOG_DEBUG("ReadConfigDB Failed result_code - %d\n", result_code);
          return result_code;
        }
        result_code = GetRenamedControllerKey(ck_ctrlr, UPLL_DT_RUNNING, dmi,
                                            &ctrlr_dom);
        if (result_code != UPLL_RC_SUCCESS && result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
          UPLL_LOG_DEBUG("GetRenamedControllerKey Failed result_code - %d\n", result_code);
          return result_code;
        }
        uuu::upll_strncpy(mkey->vtn_key.vtn_name,
                 reinterpret_cast<key_vtn *>(ck_ctrlr->get_key())->vtn_name, (kMaxLenVtnName + 1));
        ipc_req.ckv_data = ikey;
        if (!IpcUtil::SendReqToDriver((const char *)(ctrlr_dom.ctrlr),
              reinterpret_cast<char *>(ctrlr_dom.domain), PFCDRIVER_SERVICE_NAME,
                              PFCDRIVER_SVID_LOGICAL, &ipc_req, true, &ipc_resp)) {
               UPLL_LOG_INFO("Request to driver for Key %d for controller %s failed ",
                     ikey->get_key_type(), reinterpret_cast<char *>(ctrlr_dom.ctrlr));
               return ipc_resp.header.result_code; 
        }
        if (ipc_resp.header.result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_INFO("Read from driver failed err code %d\n",
                               ipc_resp.header.result_code);
          return ipc_resp.header.result_code;
        }
        if (ipc_resp.ckv_data == NULL) {
          UPLL_LOG_DEBUG("Ipc Response ckv_data is NUll %d\n",
                      ipc_resp.header.result_code);  
          return UPLL_RC_ERR_GENERIC;
        }
        ikey->ResetWith(ipc_resp.ckv_data);
        // To map PFC vExternal name to UNC vBridge interface name and vBridge name
        SET_USER_DATA_CTRLR_DOMAIN(ikey, ctrlr_dom);
        result_code = MappingvExtTovBr(ikey, header, dmi);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("MappingvExtTovBr failed result_code - %d\n", result_code);
          return result_code;
        }
        break;
      default:
        UPLL_LOG_INFO("Invalid KeyType %d\n", ikey->get_key_type());
        return UPLL_RC_ERR_GENERIC;
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
    key_vtnstation_controller *stkey = NULL;
    key_vtn_controller *mkey = NULL;
    ConfigKeyVal *vtn_ckv = NULL;
    ConfigKeyVal *next_ckv = NULL;
    DbSubOp op = {kOpReadExist, kOpMatchCtrlr | kOpMatchDomain, kOpInOutNone};
    if ((ikey->get_key_type() == UNC_KT_VTN_MAPPING_CONTROLLER) ||
        (ikey->get_key_type() == UNC_KT_VTNSTATION_CONTROLLER)) {
      result_code = ValidateMessage(header, ikey);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("ValidateMessage failed result_code %d\n",
                    result_code);
        return result_code;
      }
    }
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
    ConfigVal *tmpcval = NULL;
    key_vtnstation_controller *vtnkey = NULL;
    switch(ikey->get_key_type()) {
      case UNC_KT_VTN:
        result_code = MoMgrImpl::ReadSiblingMo(header, ikey, false, dmi);
        return result_code;
        break;
      case UNC_KT_VTNSTATION_CONTROLLER:
        //stkey = reinterpret_cast<key_vtnstation_controller *>(ikey->get_key());
        if (header->operation == UNC_OP_READ_SIBLING_BEGIN) { 
          result_code = ctrlr_mgr->GetFirstCtrlrName(UPLL_DT_RUNNING, &ctrlr_id); 
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_INFO("GetFirstCtrlrName failed err code %d\n", result_code);
            return result_code;
          }
          ctrlr_dom.ctrlr = reinterpret_cast<uint8_t *>(
                  const_cast<char *>(ctrlr_id.c_str()));
          UPLL_LOG_DEBUG("ControllerId and DomainId are %s %s\n", ctrlr_dom.ctrlr,
                            ctrlr_dom.domain);
          if ((!ctrlr_mgr->GetCtrlrType(reinterpret_cast<char *>(ctrlr_dom.ctrlr),
                       UPLL_DT_RUNNING, &ctrlrtype)) || (ctrlrtype != UNC_CT_PFC)) {
            UPLL_LOG_INFO("Controller type is  %d\n", ctrlrtype);
          } else {
            uuu::upll_strncpy(reinterpret_cast<key_vtnstation_controller *>
              (ikey->get_key())->controller_name, ctrlr_dom.ctrlr, (kMaxLenCtrlrId + 1)); 
            result_code = ReadSingleCtlrlStation(header, ikey,
                                              dmi,rec_count); 
            } 
        } else {
            ctrlr_id = reinterpret_cast<char *>(stkey->controller_name); 
        }
        UPLL_LOG_DEBUG("Input Controller Id is %s\n", ctrlr_id.c_str());
        while ((UPLL_RC_SUCCESS != ctrlr_mgr->GetNextCtrlrName(ctrlr_id, UPLL_DT_RUNNING,
                                                &ctrlr_id)) && (count <= header->rep_count )) {
          UPLL_LOG_DEBUG("sibling Controller Id is %s\n", ctrlr_id.c_str());
          ctrlr_dom.ctrlr = reinterpret_cast<uint8_t *>(
                    const_cast<char *>(ctrlr_id.c_str()));
          if ((!ctrlr_mgr->GetCtrlrType(reinterpret_cast<char *>(ctrlr_dom.ctrlr),
                       UPLL_DT_RUNNING, &ctrlrtype)) || (ctrlrtype != UNC_CT_PFC)) {
            UPLL_LOG_INFO("Controller type is  %d\n", ctrlrtype);
            continue;
          }
          next_ckv = ikey->get_next_cfg_key_val();
          if (!next_ckv)
              vtnkey = reinterpret_cast<key_vtnstation_controller *>
                                     (ConfigKeyVal::Malloc(sizeof(key_vtnstation_controller)));
              uuu::upll_strncpy(vtnkey->controller_name, ctrlr_dom.ctrlr, (kMaxLenCtrlrId + 1)); 
            val_vtnstation_controller_st_t *in_valst = NULL;
            in_valst = reinterpret_cast<val_vtnstation_controller_st_t *>(GetVal(ikey));
            if (in_valst) {
              val_vtnstation_controller_st_t *valst = reinterpret_cast<val_vtnstation_controller_st_t *>
                                     (ConfigKeyVal::Malloc(sizeof(val_vtnstation_controller_st_t))); 
            
              memcpy(valst, in_valst, sizeof(val_vtnstation_controller_st_t)); 
              tmpcval = new ConfigVal(ikey->get_cfg_val()->get_st_num(), valst); 
            }
            next_ckv = new ConfigKeyVal(ikey->get_key_type(), ikey->get_st_num(),vtnkey, tmpcval);
            result_code = ReadSingleCtlrlStation(header, next_ckv,
                                              dmi,rec_count); 
            ikey->AppendCfgKeyVal(next_ckv);
            next_ckv = next_ckv->get_next_cfg_key_val();
        }
      break;
      case UNC_KT_VTN_MAPPING_CONTROLLER:
        mkey = reinterpret_cast<key_vtn_controller *>(ikey->get_key());
        ctrlr_dom.ctrlr = mkey->controller_name;
        ctrlr_dom.domain = mkey->domain_id;
        result_code = ValidateMessage(header, ikey);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("ValidateMessage failed result_code %d\n",
                    result_code);
          return result_code;
        }
        //op = {kOpReadMultiple, kOpMatchNone, kOpInOutNone};
        result_code = GetChildConfigKey(vtn_ckv,ikey); 
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("GetChildConfigKey failed result_code - %d\n", result_code);
          return result_code;
        }
        result_code = ReadConfigDB(ikey, header->datatype, header->operation,
                                   op, header->rep_count, dmi, CTRLRTBL);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_INFO("ReadConfigDb failed err code %d\n", result_code);
        }
        if ((header->operation == UNC_OP_READ_SIBLING_BEGIN) ||
            (header->operation == UNC_OP_READ_SIBLING)) {
          op.readop = kOpReadMultiple;
        #if 0
        result_code = UpdateConfigDB(vtn_ckv, UPLL_DT_RUNNING,
              UNC_OP_READ, dmi, &op, CTRLRTBL);
        if (result_code != UPLL_RC_ERR_INSTANCE_EXISTS) {
          UPLL_LOG_DEBUG("UpdateConfigDB for read failed result_code - %d\n", result_code);
          return result_code;
        }
        op = {kOpReadMultiple, kOpMatchNone, kOpInOutDomain | kOpInOutDomain};
        result_code = ReadConfigDB(vtn_ckv, UPLL_DT_RUNNING,
              UNC_OP_READ, dmi, &op, CTRLRTBL);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("ReadConfigDB Failed result_code - %d\n", result_code);
          return result_code;
        }
        #endif
        }
        result_code = GetRenamedControllerKey(vtn_ckv, header->datatype, dmi,
                                            &ctrlr_dom);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("GetRenamedControllerKey Failed result_code - %d\n", result_code);
          return result_code;
        }
        uuu::upll_strncpy(mkey->vtn_key.vtn_name,
                 reinterpret_cast<key_vtn *>(vtn_ckv->get_key())->vtn_name, (kMaxLenVtnName + 1));
        break;
      default:
        break;
    } 
    return result_code;
  }

  upll_rc_t
  VtnMoMgr::MappingvExtTovBr(ConfigKeyVal * ikey,
                             IpcReqRespHeader * req,
                             DalDmlIntf * dmi)  {
    UPLL_FUNC_TRACE;
    upll_rc_t result_code = UPLL_RC_SUCCESS;
    DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutNone };
    MoMgrImpl *mgr = reinterpret_cast<MoMgrImpl *>
                     (const_cast<MoManager *>(GetMoManager(UNC_KT_VBR_IF)));
    key_vbr_if_t *key_vbrif = static_cast<key_vbr_if_t *>
        (ConfigKeyVal::Malloc(sizeof(key_vbr_if_t)));
    memset(key_vbrif, 0, sizeof(key_vbr_if_t));
    val_vtnstation_controller_st *valst = NULL;
    val_vtn_mapping_controller_st *val_map = NULL;
    if ((GetVal(ikey)) == NULL) {
       UPLL_LOG_DEBUG("Val struct is not present from driver response");
       free(key_vbrif);
       return UPLL_RC_ERR_GENERIC;
    }
    // TODO GetRenamedUncKey is pending
    if (UNC_KT_VTNSTATION_CONTROLLER == ikey->get_key_type()) {
      if (IpctSt::kIpcStValVtnstationControllerSt ==
           (ikey->get_cfg_val())->get_st_num()) {
        valst = reinterpret_cast<val_vtnstation_controller_st_t *>(
            GetVal(ikey));
        if (!valst) {
          UPLL_LOG_DEBUG("val_vtnstation_controller_st is NULL");
          free(key_vbrif);
          return UPLL_RC_ERR_GENERIC;
        }
        if ((valst->valid[UPLL_IDX_VTN_NAME_VSCS] != UNC_VF_VALID) &&
            (valst->valid[UPLL_IDX_VBR_IF_NAME_VSCS] != UNC_VF_VALID)) {
          UPLL_LOG_DEBUG("valid flag of vtn_name/vbrif_name is invalid"); 
          free(key_vbrif);
          return UPLL_RC_SUCCESS;
        }        
        uuu::upll_strncpy(key_vbrif->vbr_key.vtn_key.vtn_name,
                          valst->vtn_name,(kMaxLenVtnName + 1));
        val_drv_vbr_if_t *drv_val_vbrif = static_cast<val_drv_vbr_if_t *>
          (ConfigKeyVal::Malloc(sizeof(val_drv_vbr_if_t)));
        drv_val_vbrif->valid[2] = UNC_VF_VALID;
        uuu::upll_strncpy(drv_val_vbrif->vex_if_name,
                     valst->vbrif_name, (kMaxLenInterfaceName + 1));
        ConfigKeyVal *tmpckv = new
          ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf, key_vbrif,
              new ConfigVal(IpctSt::kIpcStPfcdrvValVbrIf, drv_val_vbrif));
        if (tmpckv == NULL) {
          if (key_vbrif) free(key_vbrif);
            return UPLL_RC_ERR_GENERIC;
        }
        result_code = mgr->ReadConfigDB(tmpckv, req->datatype,
                          UNC_OP_READ, dbop, dmi, MAINTBL);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("vbrif ReadConfigDB Failed result_code - %d\n",
                     result_code);
          delete tmpckv;
          return UPLL_RC_SUCCESS;
        }
        key_vbr_if_t *vbrif_key = reinterpret_cast<key_vbr_if_t *>
          (tmpckv->get_key());
        if (vbrif_key) {
          uuu::upll_strncpy(valst->vbr_name, vbrif_key->vbr_key.vbridge_name,
           (kMaxLenVnodeName + 1));
          uuu::upll_strncpy(valst->vbrif_name, vbrif_key->if_name,
           (kMaxLenInterfaceName + 1));
        }
        delete tmpckv;
      }  
    } else if (UNC_KT_VTN_MAPPING_CONTROLLER == ikey->get_key_type()) {
      if (IpctSt::kIpcStValVtnMappingControllerSt ==
          (ikey->get_cfg_val())->get_st_num()) {
        val_map = reinterpret_cast<val_vtn_mapping_controller_st_t *>
            (GetVal(ikey));
        // Valid flag check before accessing the driver reponse val
        if (val_map->valid[UPLL_IDX_VBR_IF_NAME_VMCS] != UNC_VF_VALID) {
          UPLL_LOG_DEBUG("valid flag of vbrif_name is invalid"); 
          free(key_vbrif);
          return UPLL_RC_SUCCESS;
        }
        uuu::upll_strncpy(key_vbrif->vbr_key.vtn_key.vtn_name,
                          reinterpret_cast<key_vtn_controller *>
                            (ikey->get_key())->vtn_key.vtn_name,
                          (kMaxLenVtnName + 1));
        val_drv_vbr_if_t *drv_val_vbrif = static_cast<val_drv_vbr_if_t *>
          (ConfigKeyVal::Malloc(sizeof(val_drv_vbr_if_t)));
        uuu::upll_strncpy(drv_val_vbrif->vex_if_name,
                       val_map->vbrif_name, (kMaxLenInterfaceName + 1));
        ConfigKeyVal *tmpckv = new
          ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf, key_vbrif,
              new ConfigVal(IpctSt::kIpcStPfcdrvValVbrIf, drv_val_vbrif));
        if (tmpckv == NULL) {
          if (key_vbrif) free(key_vbrif);
           return UPLL_RC_ERR_GENERIC;
        }
        result_code = mgr->ReadConfigDB(tmpckv, req->datatype,
                            UNC_OP_READ, dbop, dmi, MAINTBL);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("vbrif ReadConfigDB Failed result_code - %d\n",
                       result_code);
          delete tmpckv;
          return UPLL_RC_SUCCESS;
        }
        UPLL_LOG_DEBUG("tmpckv value is %s\n", tmpckv->ToStrAll().c_str());
        key_vbr_if_t *vbrif_key = reinterpret_cast<key_vbr_if_t *>
          (GetVal(tmpckv));
        if (vbrif_key) {
          uuu::upll_strncpy(val_map->vbr_name, vbrif_key->vbr_key.vbridge_name,
          (kMaxLenVnodeName + 1));
          uuu::upll_strncpy(val_map->vbrif_name, vbrif_key->if_name,
          (kMaxLenInterfaceName + 1));
        }
        delete tmpckv;
      }
    } else {
      free(key_vbrif);
    }
    return result_code;
  }

  /* Semantic check for the VTN Delete operation */
  upll_rc_t
  VtnMoMgr::IsReferenced(ConfigKeyVal *ikey,
                         upll_keytype_datatype_t dt_type,
                         DalDmlIntf *dmi) {
    return UPLL_RC_SUCCESS;
    /* Overlay support is not added yet. Returning success for now */
#if 0
    upll_rc_t result_code = UPLL_RC_SUCCESS;
    ConfigKeyVal *okey = NULL;
    if ( NULL == ikey)
    return UPLL_RC_ERR_GENERIC;
    /* Create the Vtunnel Configkey for checking vtn is underlay vtn or not */
    result_code = CreateVtunnelKey(ikey, okey);
    MoMgrImpl *mgr = reinterpret_cast<MoMgrImpl *>
    (const_cast<MoManager *>(GetMoManager(UNC_KT_VTUNNEL)));
    DbSubOp dbop = {kOpReadMultiple, kOpMatchNone, kOpInOutNone};
    /* Checks the given vtn is exists or not */
    result_code = mgr->ReadConfigDB(okey, dt_type, UNC_OP_READ,
                                     dbop, dmi, MAINTBL);
    if ( UPLL_RC_ERR_INSTANCE_EXISTS == result_code )
    return UPLL_RC_ERR_CFG_SEMANTIC;

    if (NULL != okey) {
      free(okey->get_key());
      delete okey;
      okey = NULL;
    }
    return result_code;
#endif
  }

  /* This function creates the configkey for the vtunnel
   * This funciton take the vtn name and addit into the Vtunnel
   * value structure
   */
  upll_rc_t
  VtnMoMgr::CreateVtunnelKey(ConfigKeyVal * ikey,
                             ConfigKeyVal * &okey)  {
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
    vtn_val_vtunnel->valid[UPLL_IDX_LABEL_VTNL] = UNC_VF_VALID;  // == to =

    tmp = new ConfigVal(IpctSt::kIpcStValVtunnel, vtn_val_vtunnel);
    okey = new ConfigKeyVal(UNC_KT_VTUNNEL, IpctSt::kIpcStKeyVtunnel,
                            vtunnel_key, tmp);
    if (!okey) {
      free(vtunnel_key);
      free(vtn_val_vtunnel);
      delete tmp;
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
          free(key_vtn);
          return UPLL_RC_ERR_GENERIC;
        }
        uuu::upll_strncpy(key_vtn->vtn_name, tval->new_name,
                          (kMaxLenVtnName + 1));
        //  copy the new UNC name to KeyVtn
        /* The New Name and PFC name should not be same name */
        if (!strcmp(reinterpret_cast<char *>
          ((reinterpret_cast<key_vtn_t *>(ikey->get_key()))->vtn_name),
           reinterpret_cast<char *>(tval->new_name))) {
          free(key_vtn);
          return UPLL_RC_ERR_GENERIC;
        }
      } else {
        UPLL_LOG_DEBUG("Invalid Input");
        free(key_vtn);
        return UPLL_RC_ERR_GENERIC;
      }
    }
    okey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key_vtn, NULL);
    if (!okey) {
      free(key_vtn);
      return UPLL_RC_ERR_GENERIC;
    }
    return result_code;
  }

  bool VtnMoMgr::FilterAttributes(void *&val1,
                                  void *val2,
                                  bool copy_to_running,
                                  unc_keytype_operation_t op) {
    val_vtn_t *val_vtn1 = reinterpret_cast<val_vtn_t *>(val1);
    val_vtn1->valid[UPLL_IDX_DESC_VTN] = UNC_VF_INVALID;
    // to be uncommented when vtn supports other attributes other than description
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

  // TODO(l): This function may not be required for VTN
  // since VTN has only description which should not go to driver
  bool VtnMoMgr::CompareValidValue(void *&val1, void *val2, bool copy_to_running) {
    UPLL_FUNC_TRACE;
    bool invalid_attr = true;  
    val_vtn_t *val_vtn1 = reinterpret_cast<val_vtn_t *>(val1);
    val_vtn_t *val_vtn2 = reinterpret_cast<val_vtn_t *>(val2);
    val_vtn_index vtn_description = UPLL_IDX_DESC_VTN;
    if (UNC_VF_INVALID == val_vtn1->valid[vtn_description] &&
                          UNC_VF_VALID == val_vtn2->valid[vtn_description])
      val_vtn1->valid[vtn_description] = UNC_VF_VALID_NO_VALUE;
    if ( (UNC_VF_VALID == val_vtn1->valid[vtn_description]) &&
           (UNC_VF_VALID == val_vtn2->valid[vtn_description])) {
      if (!strcmp(reinterpret_cast<char*>(val_vtn1->description),
                 reinterpret_cast<char*>(val_vtn2->description)) )
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

  upll_rc_t
  VtnMoMgr::UpdateAuditConfigStatus(unc_keytype_configstatus_t cs_status,
                                    uuc::UpdateCtrlrPhase phase,
                                    ConfigKeyVal *&ckv_running) {
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
  VtnMoMgr::SetConsolidatedStatus(ConfigKeyVal * ikey,
                                  DalDmlIntf * dmi)  {
    UPLL_FUNC_TRACE;
    upll_rc_t result_code = UPLL_RC_SUCCESS;
    ConfigKeyVal *ckv = NULL;
    DbSubOp dbop = { kOpReadMultiple, kOpMatchNone, kOpInOutNone };
    result_code = GetChildConfigKey(ckv, ikey);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_INFO("GetChildConfigKey failed err code %d\n", result_code);
      return result_code;
    }
    result_code = ReadConfigDB(ckv, UPLL_DT_RUNNING, UNC_OP_READ, dbop, dmi,
                               CTRLRTBL);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_INFO("ReadConfigDB from ctrltbl failed err code %d\n",
                     result_code);
      return result_code;
    }
    std::list < unc_keytype_configstatus_t > list_cs_row;
    std::list < unc_keytype_configstatus_t > list_cs_attr;
    val_vtn_t *val;
    for (; ckv != NULL; ckv->get_next_cfg_key_val()) {
      val = reinterpret_cast<val_vtn_t *>(GetVal(ckv));
      list_cs_row.push_back((unc_keytype_configstatus_t) val->cs_row_status);
      list_cs_attr.push_back((unc_keytype_configstatus_t) val->cs_attr[0]);
    }
    val_vtn_t *val_temp = reinterpret_cast<val_vtn_t *>(GetVal(ikey));
    val_temp->cs_row_status = GetConsolidatedCsStatus(list_cs_row);
    val_temp->cs_attr[0] = GetConsolidatedCsStatus(list_cs_attr);
    result_code = UpdateConfigDB(ikey, UPLL_DT_RUNNING, UNC_OP_UPDATE, dmi,
                                 MAINTBL);
    if (UPLL_RC_SUCCESS != result_code) {
      return result_code;
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
                           UNC_KT_VBRIDGE, UNC_KT_VROUTER, UNC_KT_VRT_IPROUTE,
                           UNC_KT_VRT_IF, UNC_KT_VLINK, UNC_KT_VTEP,
                           UNC_KT_VTUNNEL
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
        if (UPLL_RC_SUCCESS != result_code && 
            UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
          if (UPLL_RC_ERR_MERGE_CONFLICT == result_code) {
              UPLL_LOG_DEBUG(" Merge Conflict %d", result_code);
            mgr->GetChildConfigKey(ikey, ckval);
          } else {
            UPLL_LOG_DEBUG("Merge Validate Failed %d", result_code);
          }
          if (NULL != ckval) {
            delete ckval;
          ckval = NULL;
          }
          return result_code;
        }
      }
     import_ckval = import_ckval->get_next_cfg_key_val();
    }
    result_code = (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code)?
                   UPLL_RC_SUCCESS:result_code; 
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
      return UPLL_RC_SUCCESS;
    }

    /* Other than  UPLL_RC_ERR_NO_SUCH_INSTANCE AND UPLL_RC_SUCCESS */
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG(" ReadConfigDB Failed %d", result_code);
      return result_code;
    }
    result_code = MergeValidateChildren(import_ckval, ctrlr_id, ikey, dmi);
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
    DalCursor *dal_cursor_handle;
    IpcResponse resp;
    memset(&resp, 0, sizeof(resp));
    unc_keytype_operation_t op = (phase == uuc::kUpllUcpCreate)?UNC_OP_CREATE:
          ((phase == uuc::kUpllUcpUpdate)?UNC_OP_UPDATE:
          ((phase == uuc::kUpllUcpDelete)?UNC_OP_DELETE:UNC_OP_INVALID));
    ctrlr_dom.ctrlr = NULL;
    ctrlr_dom.domain = NULL;
    MoMgrTables tbl = (op != UNC_OP_UPDATE)?CTRLRTBL:MAINTBL;
    result_code = DiffConfigDB(UPLL_DT_CANDIDATE, UPLL_DT_RUNNING,
                    op, req, nreq, &dal_cursor_handle, dmi, tbl);
    resp.header.clnt_sess_id = session_id;
    resp.header.config_id = config_id;
    while (result_code == UPLL_RC_SUCCESS) {
      // Get Next Record
      dal_result = dmi->GetNextRecord(dal_cursor_handle);
      result_code = DalToUpllResCode(dal_result);
      if (result_code != UPLL_RC_SUCCESS)
        break;
      ck_main = NULL;
      if ( op != UNC_OP_UPDATE) {
        if (op == UNC_OP_CREATE)
          result_code = DupConfigKeyVal(ck_main, req,tbl);
        else 
          result_code = GetChildConfigKey(ck_main, req);
        if (!ck_main || result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("Returning error %d\n",result_code);
          if (ck_main)
            delete ck_main;
          return result_code;
        }
        GET_USER_DATA_CTRLR_DOMAIN(ck_main, ctrlr_dom);
        UPLL_LOG_DEBUG("ctrlr : %s; domain : %s", ctrlr_dom.ctrlr,
                       ctrlr_dom.domain);
        if ((ctrlr_dom.ctrlr == NULL) || (ctrlr_dom.domain == NULL)) {
          UPLL_LOG_INFO("Invalid controller/domain");
          return UPLL_RC_ERR_GENERIC;
        }
        result_code = TxUpdateProcess(ck_main, &resp,
                                      op, dmi, &ctrlr_dom);
        affected_ctrlr_set->insert((const char *)ctrlr_dom.ctrlr);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("Returns error %d", result_code);
          *err_ckv = resp.ckv_data;
          break;
        }
      } else {
        ConfigKeyVal *ck_ctrlr = NULL;
        result_code = DupConfigKeyVal(ck_main, req,tbl);
        if (result_code != UPLL_RC_SUCCESS)
        return result_code;
#if 0
        result_code = ValidateCapability(&(ipc_req.header), ck_main);
        if (result_code != UPLL_RC_SUCCESS)
        return result_code;
#endif
        result_code = GetChildConfigKey(ck_ctrlr, ck_main);
        if (result_code != UPLL_RC_SUCCESS)
        return result_code;
        if (GetControllerDomainSpan(ck_ctrlr, UPLL_DT_CANDIDATE, dmi) ==
            UPLL_RC_ERR_NO_SUCH_INSTANCE) {
          delete ck_ctrlr;
          ck_ctrlr = NULL;
          delete ck_main;
          ck_main = NULL;
          continue;
        }
        void *main = GetVal(ck_main);
        void *val_nrec = (nreq) ? GetVal(nreq) : NULL;
        if (FilterAttributes(main,val_nrec,false,op)) {
          delete ck_ctrlr;
          ck_ctrlr = NULL;
          continue;
        }
        for (ConfigKeyVal *tmp = ck_ctrlr; tmp != NULL;
            tmp = tmp->get_next_cfg_key_val()) {
          GET_USER_DATA_CTRLR_DOMAIN(tmp, ctrlr_dom);
          if ((ctrlr_dom.ctrlr == NULL) || (ctrlr_dom.domain == NULL)) {
            UPLL_LOG_INFO("Invalid controller/domain");
            return UPLL_RC_ERR_GENERIC;
          }
          result_code = TxUpdateProcess(ck_main, &resp,
                                        op, dmi, &ctrlr_dom);
          affected_ctrlr_set->insert((const char *)ctrlr_dom.ctrlr);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("TxUpdate Process returns with %d", result_code);
            *err_ckv = resp.ckv_data;
            break;
          }
        }
        delete ck_ctrlr;
      }
      delete ck_main;
    }
    if (nreq)
    delete nreq;
    if (req)
    delete req;
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
                            controller_domain *ctrlr_dom) {
    UPLL_FUNC_TRACE;
    upll_rc_t result_code;
    /* read from main table */
    ConfigKeyVal *dup_ckmain = ck_main;
    if ( (op == UNC_OP_CREATE) ) {
      dup_ckmain = NULL;
      result_code = GetChildConfigKey(dup_ckmain, ck_main);
      if (result_code != UPLL_RC_SUCCESS || dup_ckmain == NULL) {
        UPLL_LOG_DEBUG("Returning error %d\n", result_code);
        delete dup_ckmain;
        return result_code;
      }
      DbSubOp dbop = {kOpReadSingle, kOpMatchNone, kOpInOutNone};
      result_code = ReadConfigDB(dup_ckmain, UPLL_DT_CANDIDATE,
          UNC_OP_READ, dbop, dmi, MAINTBL);
      if (result_code != UPLL_RC_SUCCESS) {
        string s(dup_ckmain->ToStrAll());
        UPLL_LOG_INFO("%s Vtn read failed %d", s.c_str(), result_code);
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
    }
    /* Get renamed key if key is renamed */
    result_code = GetRenamedControllerKey(dup_ckmain, UPLL_DT_CANDIDATE,
        dmi, ctrlr_dom);
    if (result_code != UPLL_RC_SUCCESS) {
      if ((op == UNC_OP_CREATE) && dup_ckmain) 
        delete dup_ckmain;
      return result_code;
    }
    // ipc_req->ckv_data = ck_main;
    result_code = SendIpcReq(ipc_resp->header.clnt_sess_id,
                             ipc_resp->header.config_id, op, UPLL_DT_CANDIDATE,
                             dup_ckmain, ctrlr_dom, ipc_resp);
    if (result_code == UPLL_RC_ERR_RESOURCE_DISCONNECTED) {
      UPLL_LOG_DEBUG("Controller disconnected");
      result_code = UPLL_RC_SUCCESS;
    }
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("IpcSend failed %d", result_code);
    }
    if ((op == UNC_OP_CREATE) && dup_ckmain) {
      delete dup_ckmain; 
      dup_ckmain = NULL;
    }
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
         free(key_vtn);
       return UPLL_RC_ERR_GENERIC;
    }
    uuu::upll_strncpy(key_vtn->vtn_name, key_rename->old_unc_vtn_name,
                         (kMaxLenVtnName + 1));
           
    okey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, key_vtn, NULL);
    if (!okey) {
         free(key_vtn);
       return UPLL_RC_ERR_GENERIC;
    }
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
      free(vtn_rename_info);
      return UPLL_RC_ERR_GENERIC;
    }
    /* Checks the vtn is already renamed or not */
    if (renamed) {
      /* if already renamed store the controller name */
      if (!strlen(reinterpret_cast<char *>(
           (reinterpret_cast<val_rename_vtn_t *>(GetVal(ikey)))->new_name))) {
        free(vtn_rename_info);
        return UPLL_RC_ERR_GENERIC;
      }
      uuu::upll_strncpy(vtn_rename_info->ctrlr_vtn_name,
           reinterpret_cast<val_rename_vtn_t *>(GetVal(ikey))->new_name,
           (kMaxLenVtnName + 1));
    } else {
      /* if not renamed the ikey contains the controller name */
      if (!strlen(reinterpret_cast<char *>(vtn_key->vtn_name))) {
        free(vtn_rename_info);
        return UPLL_RC_ERR_GENERIC;
      }
      uuu::upll_strncpy(vtn_rename_info->ctrlr_vtn_name, vtn_key->vtn_name,
                        (kMaxLenVtnName + 1));
    }
    /* Store the old UNC VTN  name */
    if (!strlen(reinterpret_cast<char *>(vtn_key->vtn_name))) {
      free(vtn_rename_info);
      return UPLL_RC_ERR_GENERIC;
    }
    uuu::upll_strncpy(vtn_rename_info->old_unc_vtn_name, vtn_key->vtn_name,
                        (kMaxLenVtnName + 1));

    vtn_key = reinterpret_cast<key_vtn_t *>(okey->get_key());
    /* store the new UNC VTN NAME */
    if (!strlen(reinterpret_cast<char *>(vtn_key->vtn_name))) {
      free(vtn_rename_info);
      return UPLL_RC_ERR_GENERIC;
    }
    uuu::upll_strncpy(vtn_rename_info->new_unc_vtn_name, vtn_key->vtn_name,
                        (kMaxLenVtnName + 1));

    rename_info = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcInvalidStNum,
                                     vtn_rename_info, NULL);
    DbSubOp dbop = {kOpReadMultiple, kOpMatchNone, kOpInOutCtrlr|kOpInOutDomain};
    result_code = ReadConfigDB(ikey, UPLL_DT_IMPORT,
                              UNC_OP_READ, dbop, dmi, CTRLRTBL);
        if (UPLL_RC_SUCCESS != result_code) {
          UPLL_LOG_DEBUG("ReadConfigDB Error");
          return result_code;
        }
    SET_USER_DATA (rename_info, ikey);
    if (!rename_info) {
      free(vtn_rename_info);
      return UPLL_RC_ERR_GENERIC;
    }
    /* Set the Controller Id*/
    SET_USER_DATA_CTRLR(rename_info, ctrlr_id);
    if (!renamed) {
        val_rename_vtn_t *vtn = reinterpret_cast<val_rename_vtn_t *>
            (ConfigKeyVal::Malloc(sizeof(val_rename_vtn)));
        uuu::upll_strncpy(vtn->new_name, vtn_rename_info->ctrlr_vtn_name,
                        (kMaxLenCtrlrId + 1));
        ConfigVal *cfg_val = new ConfigVal(IpctSt::kIpcStValRenameVtn, vtn);
        if (!cfg_val) {
          free(vtn);
          delete rename_info;
        return UPLL_RC_ERR_GENERIC;
        }
        vtn->valid[UPLL_IDX_NEW_NAME_RVTN] = UNC_VF_VALID;
        ikey->SetCfgVal(cfg_val);
        SET_USER_DATA (okey, ikey);
        dbop.readop = kOpNotRead; 
        result_code = UpdateConfigDB(ikey, UPLL_DT_IMPORT, UNC_OP_CREATE,
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
    upll_rc_t ret_val = UPLL_RC_SUCCESS;
    if (vtn_val->valid[UPLL_IDX_DESC_VTN] == UNC_VF_VALID) {
      ret_val = ValidateDesc(reinterpret_cast<char *>(vtn_val->description),
          kMinLenDescription, kMaxLenDescription);
      if (ret_val != UPLL_RC_SUCCESS) {
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
  VtnMoMgr::ValVtnAttributeSupportCheck(const char *ctrlr_name,
                                        ConfigKeyVal *ikey,
                                        uint32_t operation) {
  return UPLL_RC_SUCCESS;  // TEMP FIX till CAPA CHECK
    UPLL_FUNC_TRACE;
    bool result_code = false;
    uint32_t max_attrs = 0;
    uint32_t instance_count = 0;
    const uint8_t *attrs;

    switch (operation) {
      case UNC_OP_CREATE:
      result_code = GetCreateCapability(ctrlr_name,
          ikey->get_key_type(),
          &instance_count,
          &max_attrs,
          &attrs);
      break;

      case UNC_OP_UPDATE:
      result_code = GetUpdateCapability(ctrlr_name,
          ikey->get_key_type(),
          &max_attrs,
          &attrs);
      break;
      case UNC_OP_READ:
      case UNC_OP_READ_SIBLING:
      case UNC_OP_READ_SIBLING_BEGIN:
      case UNC_OP_READ_SIBLING_COUNT:
      result_code = GetReadCapability(ctrlr_name,
          ikey->get_key_type(),
          &max_attrs,
          &attrs);
      break;
      default:
      UPLL_LOG_INFO("Invalid operation code(%d)", operation);
      return UPLL_RC_ERR_GENERIC;
    }
    if (!result_code) {
      UPLL_LOG_INFO("key_type - %d is not supported by controller - %s",
          ikey->get_key_type(), ctrlr_name);
      return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_KT;
    }
    if (NULL == ikey->get_cfg_val()) {
      UPLL_LOG_INFO("Empty cfg_val is received");
      return UPLL_RC_ERR_GENERIC;
    }
    if ((ikey->get_cfg_val())->get_st_num() != IpctSt::kIpcStValVtn) {
      UPLL_LOG_INFO("value structure matching is invalid. struct.no - %d",
          (ikey->get_cfg_val())->get_st_num());
      return UPLL_RC_ERR_CFG_SYNTAX;
    }

    val_vtn *vtn_val = reinterpret_cast<val_vtn *>
                              (ikey->get_cfg_val()->get_val());
    if (vtn_val !=NULL) {
      if ((vtn_val->valid[UPLL_IDX_DESC_VTN] == UNC_VF_VALID) ||
          (vtn_val->valid[UPLL_IDX_DESC_VTN] == UNC_VF_VALID_NO_VALUE)) {
        if (attrs[unc::capa::vtn::kCapDesc] == 0) {
          vtn_val->valid[UPLL_IDX_DESC_VTN] = UNC_VF_NOT_SOPPORTED;
          UPLL_LOG_INFO("Description attr is not supported by ctrlr");
          return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
        }
      }
      return UPLL_RC_SUCCESS;
    } else {
      UPLL_LOG_INFO("Error val_vtn struct is NULL");
      return UPLL_RC_ERR_GENERIC;
    }
  }

  upll_rc_t
  VtnMoMgr::ValidateCapability(IpcReqRespHeader * req,
                               ConfigKeyVal * ikey, const char *cntrl_id)  {
    UPLL_FUNC_TRACE;
    upll_rc_t ret_val = UPLL_RC_SUCCESS;
    if (!req || !ikey) {
      UPLL_LOG_INFO("ConfigKeyVal / IpcReqRespHeader is Null");
      return UPLL_RC_ERR_GENERIC;
    }
    const char *ctrlr_name = reinterpret_cast<char *>(ikey->get_user_data());
    upll_keytype_datatype_t dt_type = req->datatype;
    unc_keytype_operation_t operation = req->operation;
    unc_keytype_option1_t option1 = req->option1;
    unc_keytype_option2_t option2 = req->option2;

    UPLL_LOG_INFO("dt_type   : (%d)"
                  "operation : (%d)"
                  "option1   : (%d)"
                  "option2   : (%d)",
                  dt_type, operation, option1, option2);

    if (operation == UNC_OP_CREATE) {
      if (dt_type == UPLL_DT_CANDIDATE || dt_type == UPLL_DT_IMPORT) {
        ret_val = ValVtnAttributeSupportCheck(ctrlr_name, ikey, operation);
        if (ret_val != UPLL_RC_SUCCESS) {
          UPLL_LOG_INFO("Capa check failure for create operation");
          return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
        }
        return UPLL_RC_SUCCESS;
      } else {
        UPLL_LOG_INFO("Error Unsupported datatype (%d)", dt_type);
        return UPLL_RC_ERR_GENERIC;
      }
    } else if (operation == UNC_OP_UPDATE) {
      if (dt_type == UPLL_DT_CANDIDATE) {
        ret_val = ValVtnAttributeSupportCheck(ctrlr_name, ikey, operation);
        if (ret_val != UPLL_RC_SUCCESS) {
          UPLL_LOG_INFO("Capa check failure for Update operation");
          return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
        }
        return UPLL_RC_SUCCESS;
      } else {
        UPLL_LOG_INFO("Error Unsupported datatype (%d)", dt_type);
        return UPLL_RC_ERR_GENERIC;
      }
    } else if (operation == UNC_OP_READ
        || operation == UNC_OP_READ_SIBLING_BEGIN
        || operation == UNC_OP_READ_SIBLING
        || operation == UNC_OP_READ_SIBLING_COUNT) {
      if (dt_type == UPLL_DT_CANDIDATE || dt_type == UPLL_DT_RUNNING
          || dt_type == UPLL_DT_STARTUP || dt_type == UPLL_DT_STATE) {
        if (option1 != UNC_OPT1_NORMAL) {
          UPLL_LOG_INFO("Error: option1 is not NORMAL");
          return UPLL_RC_ERR_INVALID_OPTION1;
        }
        if (option2 != UNC_OPT2_NONE) {
          UPLL_LOG_INFO("Error: option2 is not NONE");
          return UPLL_RC_ERR_INVALID_OPTION2;
        }
        if (ikey->get_cfg_val()->get_val() != NULL) {
          ret_val = ValVtnAttributeSupportCheck(ctrlr_name, ikey, operation);
          if (ret_val != UPLL_RC_SUCCESS) {
            UPLL_LOG_INFO("capa check failure for read operation");
            return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
          }
          return UPLL_RC_SUCCESS;
        } else {
          UPLL_LOG_INFO("capa check for val_vtn struct is an optional");
          return UPLL_RC_SUCCESS;
        }
      } else {
        UPLL_LOG_INFO("Error Unsupported datatype (%d)", dt_type);
        return UPLL_RC_ERR_GENERIC;
      }
    }
    UPLL_LOG_INFO("Error Unsupported operation(%d)", operation);
    return UPLL_RC_ERR_GENERIC;
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
    if (!controllerName) {
      UPLL_LOG_DEBUG("memory allocation failed");
      return UPLL_RC_ERR_GENERIC;
    }
    uuu::upll_strncpy(controllerName, ctr->controller_name,
                      (kMaxLenCtrlrId + 1));
    result_code = GetChildConfigKey(ck_ctrlr,NULL);
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
    upll_rc_t ret_val = UPLL_RC_SUCCESS;

    ret_val = ValidateKey(
        reinterpret_cast<char *>(vtn_ctrlr_key->vtn_key.vtn_name),
        kMinLenVtnName, kMaxLenVtnName);
    if (ret_val != UPLL_RC_SUCCESS) {
      UPLL_LOG_INFO("vtn name syntax check failed."
                    "Received vtn_Name - %s",
                    vtn_ctrlr_key->vtn_key.vtn_name);
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
    if ((operation != UNC_OP_READ_SIBLING_COUNT) &&
       (operation != UNC_OP_READ_SIBLING_BEGIN)) {
      ret_val = ValidateKey(
         reinterpret_cast<char *>(vtn_ctrlr_key->controller_name),
         kMinLenCtrlrId, kMaxLenCtrlrId);
      if (ret_val != UPLL_RC_SUCCESS) {
        UPLL_LOG_INFO("controller_name syntax check failed."
                      "Received controller_name - %s",
                      vtn_ctrlr_key->controller_name);
        return UPLL_RC_ERR_CFG_SYNTAX;
      }
      ret_val = ValidateDefaultStr(
        reinterpret_cast<char *>(vtn_ctrlr_key->domain_id),
        kMinLenDomainId, kMaxLenDomainId);
      if (ret_val != UPLL_RC_SUCCESS) {
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
    upll_rc_t ret_val = UPLL_RC_SUCCESS;
    if ((operation != UNC_OP_READ_SIBLING_COUNT) &&
       (operation != UNC_OP_READ_SIBLING_BEGIN)) {
      ret_val = ValidateKey(
         reinterpret_cast<char *>(vtn_ctrlr_key->controller_name),
         kMinLenCtrlrId, kMaxLenCtrlrId);
      if (ret_val != UPLL_RC_SUCCESS) {
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

#if 0
  /* Function is invoked upon a controller disconnect/connect */

  upll_rc_t VtnMoMgr::ConnectionStatusHandler(uint8_t *ctrlr_name,
      DalDmlIntf *dmi, uint8_t *notification) {
    UPLL_FUNC_TRACE;
    strncpy(reinterpret_cast<char *>(notification), "CONNECTION_FAULT",
        sizeof(notification));
    /* Set to store all the vtns connected to the given controller */
    set<key_vtn_t>VtnSet;
    set<key_vtn_t>::iterator vtnset_itr;
    /* Function gives all the connected vtns to the controller specified */
    UpdateVtnNodeOperStatus(ctrlr_name, &VtnSet, dmi, notification);
    /* Iterating through all the vtns in the vtnset */
  for (vtnset_itr = VtnSet.begin(); vtnset_itr != VtnSet.end(); ++vtnset_itr) {
      strncpy(reinterpret_cast<char *>(vtn_name_o),
          (const char *)vtnset_itr->vtn_name, sizeof(vtn_name_o));
      /* Function sets Operstatus of Vtn to Unknown */
      VtnSetOperStatus(vtn_name_o, dmi, notification);
      /* Function gives all the vlinks under the given vtn and
       * sets OperStatus to Unknown */
      UpdateVtnVlinkOperStatus(vtn_name_o, dmi);
    }
    return UPLL_RC_SUCCESS;
  }

  /* Get Vtns connected to the specified Controller name */
  upll_rc_t ConnectionStatus::UpdateVtnNodeOperStatus(uint8_t *ctrlr_name,
      set<key_vtn_t>*VtnSet,
      DalDmlIntf *dmi, uint8_t *notification) {
    UPLL_FUNC_TRACE;
    key_vtn_t *vtn_ctrlr_key = reinterpret_cast<key_vtn_t *>
    (malloc(sizeof(key_vtn_t)));
    ConfigKeyVal *ck_vtn_ctrlr_key = NULL;
    ck_vtn_ctrlr_key = new ConfigKeyVal(UNC_KT_VTN,
        IpctSt::kIpcStKeyVtn, vtn_ctrlr_key, NULL);
    SET_USER_DATA_CTRLR(ck_vtn_ctrlr_key, ctrlr_name);
    /* Get all the vtns with the specified controller name */
    DbSubOp dbop = {kOpReadMultiple, kOpMatchCtrlr, kOpInOutNone};
    upll_rc_t result_code = ReadConfigDB(ck_vtn_ctrlr_key, UPLL_DT_RUNNING,
        UNC_OP_READ, dbop, dmi, CTRLRTBL);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_INFO("Error in reading:%d", result_code);
      return result_code;
    }
    /* Initialising array of unc_key_types */
  unc_key_type_t node_key_type[]= {UNC_KT_VBRIDGE, UNC_KT_VROUTER, UNC_KT_VTEP,
      UNC_KT_VTUNNEL};
    ConfigKeyVal *ck_val = NULL;
    /*  Populating vtnset with vtn names connected to given controller name */
    while (ck_vtn_ctrlr_key != NULL) {
     VtnSet.insert(*(reinterpret_cast<key_vtn *>)(ck_vtn_ctrlr_key->get_key()));
      /* Iterating through each node type in unc key type */
      for (int i = 0; i < sizeof(node_key_type)/sizeof(unc_key_type_t); i++) {
        const unc_key_type_t ktype = node_key_type[i];
        MoMgrImpl *mgr = reinterpret_cast<MoMgrImpl *>(GetMoManager(ktype));
        mgr->GetChildConfigKey(ck_val, ck_vtn_ctrlr_key);
        DbSubOp dbop = {kOpReadMultiple, kOpMatchCtrlr, kOpInOutNone};
        SET_USER_DATA_CTRLR(ck_val, ctrlr_name);
        upll_rc_t result_code = mgr->ReadConfigDB(ck_val, UPLL_DT_STATE,
            UNC_OP_READ, dbop, dmi, MAINTBL);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_INFO("Error in reading:%d", result_code);
          return result_code;
        }
        key_vnode_t *vnode_key = (*(reinterpret_cast<key_vnode_t*>)
            (ck_val->get_key()));
        while (ck_val != NULL) {
          switch (ktype) {
            case UNC_KT_VBRIDGE:
            {
              mgr->SetOperStatus<val_vbr_st_t *>(ck_val, ktype, dmi,
                  notification);
              MoMgrImpl *mgr = reinterpret_cast<MoMgrImpl *>
              (GetMoManager(UNC_KT_VBR_IF));
              mgr->SetIfOperStatus(ck_val, vnode_key, dmi);
              break;
            }
            case UNC_KT_VROUTER:
            {
              mgr->SetOperStatus<val_vrt_st_t *>(ck_val, ktype, dmi,
                  notification);
              MoMgrImpl *mgr = reinterpret_cast<MoMgrImpl *>
              (GetMoManager(UNC_KT_VRT_IF));
              mgr->SetIfOperStatus(ck_val, vnode_key, dmi);
              break;
            }
            case UNC_KT_VTEP:
            {
              mgr->SetOperStatus<val_vtep_st_t *>(ck_val, ktype, dmi,
                  notification);
              MoMgrImpl *mgr = reinterpret_cast<MoMgrImpl *>
              (GetMoManager(UNC_KT_VTEP_IF));
              mgr->SetIfOperStatus(ck_val, vnode_key, dmi);
              break;
            }
            case UNC_KT_VTUNNEL:
            {
              mgr->SetOperStatus<val_vtunnel_st_t *>(ck_val, ktype, dmi,
                  notification);
              MoMgrImpl *mgr = reinterpret_cast<MoMgrImpl *>
              (GetMoManager(UNC_KT_VTUNNEL_IF));
              mgr->SetIfOperStatus(ck_val, vnode_key, dmi);
              break;
            }
            default:
            break;
          }
          ck_val = ck_val->get_next_cfg_key_val();
        }
      }
      if (ck_val)
      delete ck_val;
      val_vtn_st_t *val_vtnst = reinterpret_cast<val_vtn_st_t*>
      (GetVal(ck_vtn_ctrlr_key));
      if (!val_vtnst) return UPLL_RC_ERR_GENERIC;
      val_vtnst->oper_status = UPLL_OPER_STATUS_UNKNOWN;
      val_vtn->valid[UPLL_IDX_OPER_STATUS_VS] = UNC_VF_VALID;
      result_code = UpdateConfigDB(ck_val, UPLL_DT_RUNNING, UNC_OP_UPDATE,
          dmi, CTRLRTBL);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_INFO("Error in update oper status");
      }

      ck_vtn_ctrlr_key = ck_vtn_ctrlr_key->get_next_cfg_key_val();
    }
    /*  Delete the ck_vtn_ctrlr_key pointer */
    if (ck_vtn_ctrlr_key)
    delete ck_vtn_ctrlr_key;
    return UPLL_RC_SUCCESS;
  }

  /* Get all the vlinks connected to vtn and all the
   * vnodes connected to vlinks */
  upll_rc_t VtnMoMgr::UpdateVtnVlinkOperStatus(uint8_t *vtn_name_o,
      DalDmlIntf *dmi, uint8_t *notification) {
    UPLL_FUNC_TRACE;
    /* Initialising Key and val vlink structure */
    ConfigKeyVal *ck_vlink = NULL;
    key_vlink_t *vlink_key = reinterpret_cast<key_vlink_t *>
    malloc(sizeof(key_vlink_t));
    if (!vlink_key) return UPLL_RC_ERR_GENERIC;
    memset(vlink_key, 0, sizeof(vlink_key));
    /*copy the vtn name connected to controller into key_vlink vtn name */
    strncpy(reinterpret_cast<char *>vlink_key->vtn_key.vtn_name,
        (const char *)vtn_name_o, sizeof(vlink_key->vtn_key.vtn_name));
    ck_vlink = new ConfigKeyVal(UNC_KT_VLINK, IpctSt::kIpcStKeyVlink,
        vlink_key, NULL);
    VlinkMoMgr *mgr = reinterpret_cast<VlinkMoMgr *>GetMoManager(UNC_KT_VLINK);
    result_code = mgr->SetOperStatus(ck_vlink, dmi, notification);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_INFO("Record updation failed in UPLL_DT_STATE %d",
          result_code);
      return result_code;
    }
    if (ck_vlink)
    delete ck_vlink;
    return UPLL_RC_SUCCESS;
  }

  /* Set operstatus of vnode_if's of vnode ubder specified vtn to unknown */
  upll_rc_t ConnectionStatus::SetIfOperStatus(ConfigKeyval ck_val,
      key_vnode_t vnode_key, DalDmlIntf dmi) {
    UPLL_FUNC_TRACE;
    ConfigKeyVal *ck_vnode = NULL;
    mgr->GetChildConfigKey(ck_vnode, ck_val);
    DbSubOp dbop = {kOpReadMultiple, kOpMatchNone, kOpInOutNone};
    upll_rc_t result_code = ReadConfigDB(ck_vnode, UPLL_DT_STATE,
        UNC_OP_READ, dbop, dmi, MAINTBL);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_INFO("Error in reading:%d", result_code);
      return result_code;
    }
    ConfigVal *tmp = (ck_val->get_cfg_val())?
    ck_val->get_cfg_val()->get_next_cfg_val():NULL;
    tmp->get_val():NULL;
    while (ck_vnode != NULL) {
      switch (vnode_key) {
        case UNC_KT_VBRIDGE:
        {
          val_vbr_if_st_t *vbr_if_valst = reinterpret_cast<val_vbr_if_st_t *>
          ((tmp != NULL)?tmp->get_val():NULL);
          if (vbr_if_valst == NULL) return UPLL_RC_ERR_GENERIC;
          vbr_if_valst->oper_status = UPLL_OPER_STATUS_UNKNOWN;
          break;
        }
        case UNC_KT_VROUTER:
        {
          val_vrt_if_st_t *vrt_if_valst = reinterpret_cast<val_vrt_if_st_t *>
          ((tmp != NULL)?tmp->get_val():NULL);
          if (vrt_if_valst == NULL) return UPLL_RC_ERR_GENERIC;
          vrt_if_valst->oper_status = UPLL_OPER_STATUS_UNKNOWN;
          break;
        }
        case UNC_KT_VTEP:
        {
          val_vtep_if_st_t *vtep_if_valst = reinterpret_cast<val_vtep_if_st_t *>
          ((tmp != NULL)?tmp->get_val():NULL);
          if (vtep_if_valst == NULL) return UPLL_RC_ERR_GENERIC;
          vtep_if_valst->oper_status = UPLL_OPER_STATUS_UNKNOWN;
          break;
        }
        case UNC_KT_VTUNNEL:
        {
          val_vtunnel_if_st_t *vtunnel_if_valst =
          reinterpret_cast<val_vtunnel_if_st_t *>((tmp != NULL)?
              tmp->get_val():NULL);
          if (vtunnel_if_valst == NULL) return UPLL_RC_ERR_GENERIC;
          vtunnel_if_valst->oper_status = UPLL_OPER_STATUS_UNKNOWN;
          break;
        }
        default:
        break;
      }
      ck_vnode = ck_vnode->get_next_cfg_key_val();
    }
    result_code = UpdateConfigDB(ck_vtn, UPLL_DT_STATE, UNC_OP_UPDATE,
        dmi, MAINTBL);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_INFO("Error in update oper status");
    }
    return UPLL_RC_SUCCESS;
  }

#endif


bool VtnMoMgr::SetOperStatus(ConfigKeyVal *ikey,
                             state_notification notification,
                             DalDmlIntf *dmi) {
  bool result = false;
  if (!ikey) {
    UPLL_LOG_DEBUG("Invalid param");
    return false;
  }
  ConfigVal *tmp = (ikey->get_cfg_val()) ?
                    ikey->get_cfg_val()->get_next_cfg_val() : NULL;
  val_db_vtn_st_t *vtn_valst = reinterpret_cast<val_db_vtn_st_t *>((tmp != NULL) ? tmp->get_val() : NULL);
  if (vtn_valst == NULL) {
    UPLL_LOG_DEBUG("Invalid param");
    return false;
  }
  val_vtn_st_t *vtn_val = reinterpret_cast<val_vtn_st_t *>(vtn_valst);

    /* Update oper status based on notification */
  vtn_val->valid[0] = UNC_VF_VALID;
  switch (notification) {
  case kCtrlrReconnect:
    return false;
  case kCtrlrDisconnect:
    vtn_val->oper_status = UPLL_OPER_STATUS_UNKNOWN;
    break;
  case kPortFault:
  case kPathFault:
  case kBoundaryFault:
    vtn_valst->down_count = (vtn_valst->down_count + 1);
    if (vtn_valst->down_count == 1) {
      vtn_val->oper_status = UPLL_OPER_STATUS_DOWN;
      // generate alarm
      result = true;
    }
    break;
  case kPortFaultReset:
  case kPathFaultReset:
  case kBoundaryFaultReset:
    vtn_valst->down_count = (vtn_valst->down_count > 0) ?
                            (vtn_valst->down_count - 1) : 0;
    if (vtn_valst->down_count == 0) {
      vtn_val->oper_status = UPLL_OPER_STATUS_UP;
      // reset alarm
      result = true;
    }
    break;
  }
  upll_rc_t result_code = UpdateConfigDB(ikey, UPLL_DT_STATE, UNC_OP_UPDATE,
                           dmi, MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Returning error %d", result_code);
  }
  return result;
}

bool VtnMoMgr::SetCtrlrOperStatus(ConfigKeyVal *ikey,
                             state_notification notification,
                             DalDmlIntf *dmi) {
  bool result = false;
  if (!ikey) return UPLL_RC_ERR_GENERIC;
  val_vtn_ctrlr *ctrlr_val = reinterpret_cast<val_vtn_ctrlr *>(GetVal(ikey));
  if (ctrlr_val == NULL) return UPLL_RC_ERR_GENERIC;

    /* Update oper status based on notification */
  switch (notification) {
  case kCtrlrReconnect:
    return false;
  case kCtrlrDisconnect:
    ctrlr_val->oper_status = UPLL_OPER_STATUS_UNKNOWN;
    break;
  case kPortFault:
  case kPathFault:
  case kBoundaryFault:
    ctrlr_val->down_count = (ctrlr_val->down_count + 1);
    if (ctrlr_val->down_count == 1) {
      ctrlr_val->oper_status = UPLL_OPER_STATUS_DOWN;
      // generate alarm
      result = true;
    }
    break;
  case kPortFaultReset:
  case kPathFaultReset:
  case kBoundaryFaultReset:
    ctrlr_val->down_count = (ctrlr_val->down_count > 0) ?
                            (ctrlr_val->down_count - 1) : 0;
    if (ctrlr_val->down_count == 0) {
      ctrlr_val->oper_status = UPLL_OPER_STATUS_UP;
      // reset alarm
      result = true;
    }
    break;
  }
  upll_rc_t result_code = UpdateConfigDB(ikey, UPLL_DT_RUNNING, UNC_OP_UPDATE,
                           dmi, CTRLRTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Returning error %d", result_code);
  }
  return result;
}

upll_rc_t VtnMoMgr::UpdateOperStatus(ConfigKeyVal *ck_vtn,
                                       DalDmlIntf *dmi,
                                       state_notification notification,
                                       bool skip) {
  upll_rc_t result_code;
  if (!skip) {
    DbSubOp dbop = { kOpReadMultiple, kOpMatchCtrlr | kOpMatchDomain, kOpInOutNone };
    result_code = ReadConfigDB(ck_vtn, UPLL_DT_RUNNING, UNC_OP_READ, dbop, dmi,
                               CTRLRTBL);
    if (result_code != UPLL_RC_SUCCESS) {
      delete ck_vtn;
      UPLL_LOG_DEBUG("Error in reading: %d", result_code);
      return result_code;
    }
  }
  ConfigKeyVal *tkey = ck_vtn;
  while (tkey != NULL) {
    bool oper_status_change = SetCtrlrOperStatus(tkey, notification, dmi) ;
    if (oper_status_change) {
      ConfigKeyVal *ck_vtn_main = NULL;
      result_code = GetChildConfigKey(ck_vtn_main,tkey);
      if (!ck_vtn_main || result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Invalid param");
        return result_code;
      }
      DbSubOp dbop = { kOpReadMultiple, kOpMatchNone, kOpInOutNone };
      result_code = ReadConfigDB(ck_vtn_main, UPLL_DT_STATE, UNC_OP_READ, 
                         dbop, dmi, MAINTBL);
      if (result_code != UPLL_RC_SUCCESS) {
        delete ck_vtn_main;
        UPLL_LOG_DEBUG("Error in reading: %d", result_code);
        return result_code;
      }
      SetOperStatus(ck_vtn_main, notification, dmi) ;
      if (ck_vtn_main)
        delete ck_vtn_main;
    }
    tkey = tkey->get_next_cfg_key_val();
  }
  return UPLL_RC_SUCCESS;
}


/* SetOperStatus of VTN to unknown on a controller disconnect */
bool VtnMoMgr::VtnSetOperStatus(uint8_t *vtn_name_o,
                                DalDmlIntf *dmi,
                                state_notification notification) {
  UPLL_FUNC_TRACE;
  bool res = false;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  key_vtn_t *vtn_key = reinterpret_cast<key_vtn_t *>
                                      (malloc(sizeof(key_vtn_t)));
  if (!vtn_key) {
    UPLL_LOG_DEBUG("Invalid param");
    return false;
  }
  ConfigKeyVal *ck_vtn = new ConfigKeyVal(UNC_KT_VTN,
                          IpctSt::kIpcStKeyVtn, vtn_key, NULL);
  if (!ck_vtn) {
    free(vtn_key);
    UPLL_LOG_DEBUG("Invalid param");
    return false;
  }
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
    result_code = GetUninitOperState<val_vtn_st,val_db_vtn_st>
                    (ck_vtn, dmi);    
    break;
  default:
    UPLL_LOG_DEBUG("Unsupported operation on keytype %d\n",ktype);
    return UPLL_RC_ERR_GENERIC;
  }
  if (UPLL_RC_SUCCESS != result_code)  {
    return result_code;
  }
  ConfigKeyVal *tkey = ck_vtn;
  DbSubOp dbop1 = { kOpNotRead, kOpMatchNone, kOpInOutNone };
  while (tkey) {
    val_db_vtn_st * vtn_st = reinterpret_cast<val_db_vtn_st *>
                                                (GetStateVal(tkey));
    if (vtn_st->down_count == 0) {
      if (vtn_st->vtn_val_st.oper_status != UPLL_OPER_STATUS_UNKNOWN) {
        vtn_st->vtn_val_st.valid[UPLL_IDX_OPER_STATUS_VS] = 
                            UNC_VF_VALID;
        vtn_st->vtn_val_st.oper_status = UPLL_OPER_STATUS_UP;
        result_code = UpdateConfigDB(tkey, UPLL_DT_STATE, 
                       UNC_OP_UPDATE, dmi, &dbop1, MAINTBL);
        if (result_code != UPLL_RC_SUCCESS) {
           UPLL_LOG_DEBUG("UpdateConfigDB Executed %d", result_code);
           break;
        }
      }
    }
    tkey= tkey->get_next_cfg_key_val();
  }
  if (ck_vtn)
    delete ck_vtn;
  return result_code;
}

}  // namespace kt_momgr
}  // namespace upll
}  // namespace unc
