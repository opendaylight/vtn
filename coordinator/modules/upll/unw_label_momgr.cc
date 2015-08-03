/*
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include "unw_label_momgr.hh"
#include "upll_validation.hh"
#include "uncxx/upll_log.hh"
#include "unc/uppl_common.h"

using unc::upll::ipc_util::IpcUtil;

namespace unc {
namespace upll {
namespace kt_momgr {

BindInfo UNWLabelMoMgr::unw_label_bind_info[] = {
  { uudst::unw_label::kDbiUnifiedNwName, CFG_KEY, offsetof(
          key_unw_label, unified_nw_key.unified_nw_id),
    uud::kDalChar, 32 },
  { uudst::unw_label::kDbiUnwLabelName, CFG_KEY, offsetof(
          key_unw_label, unw_label_id),
    uud::kDalChar, 32 },
  { uudst::unw_label::kDbiMaxCount, CFG_VAL, offsetof(
          val_unw_label, max_count),
    uud::kDalUint32, 1 },
  { uudst::unw_label::kDbiRaisingThreshold, CFG_VAL, offsetof(
          val_unw_label, raising_threshold),
    uud::kDalUint32, 1 },
  { uudst::unw_label::kDbiFallingThreshold, CFG_VAL, offsetof(
          val_unw_label, falling_threshold),
    uud::kDalUint32, 1 },
  { uudst::unw_label::kDbiValidMaxCount, CFG_META_VAL, offsetof(
          val_unw_label, valid[UPLL_IDX_MAX_COUNT_UNWL]),
    uud::kDalUint8, 1 },
  { uudst::unw_label::kDbiValidRaisingThreshold, CFG_META_VAL, offsetof(
          val_unw_label, valid[UPLL_IDX_RAISING_THRESHOLD_UNWL]),
    uud::kDalUint8, 1 },
  { uudst::unw_label::kDbiValidFallingThreshold, CFG_META_VAL, offsetof(
          val_unw_label, valid[UPLL_IDX_FALLING_THRESHOLD_UNWL]),
    uud::kDalUint8, 1 },
  { uudst::unw_label::kDbiCsRowStatus, CS_VAL, offsetof(
          val_unw_label, cs_row_status),
    uud::kDalUint8, 1 },
  { uudst::unw_label::kDbiCsMaxCount, CS_VAL, offsetof(
          val_unified_nw, cs_attr[0]),
    uud::kDalUint8, 1 },
  { uudst::unw_label::kDbiCsRaisingThreshold, CS_VAL, offsetof(
          val_unified_nw, cs_attr[1]),
    uud::kDalUint8, 1 },
  { uudst::unw_label::kDbiCsFallingThreshold, CS_VAL, offsetof(
          val_unified_nw, cs_attr[2]),
    uud::kDalUint8, 1 }
};

unc_key_type_t UNWLabelMoMgr::unw_label_child[] = { UNC_KT_UNW_LABEL_RANGE };

UNWLabelMoMgr::UNWLabelMoMgr() : MoMgrImpl() {
  UPLL_FUNC_TRACE;
  ntable = MAX_MOMGR_TBLS;
  table = new Table *[ntable]();
  table[MAINTBL] = new Table(uudst::kDbiUnwLabelTbl,
       UNC_KT_UNW_LABEL, unw_label_bind_info,
      IpctSt::kIpcStKeyUnwLabel, IpctSt::kIpcStValUnwLabel,
      uudst::unw_label::kDbiUnwLabelNumCols);
  table[RENAMETBL] = NULL;
  table[CTRLRTBL] = NULL;
  table[CONVERTTBL] = NULL;
  nchild = sizeof(unw_label_child) / sizeof(*unw_label_child);
  child  = unw_label_child;
}

upll_rc_t UNWLabelMoMgr::DupConfigKeyVal(ConfigKeyVal *&okey,
                                         ConfigKeyVal *&req,
                                         MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  if (req == NULL) {
    UPLL_LOG_DEBUG("Input ConfigKeyVal req is NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  if (okey != NULL) {
    UPLL_LOG_DEBUG("Output ConfigKeyVal okey is not NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  if (req->get_key_type() != UNC_KT_UNW_LABEL) {
    UPLL_LOG_DEBUG("Input ConfigKeyVal keytype mismatch");
    return UPLL_RC_ERR_GENERIC;
  }
  ConfigVal *tmp1 = NULL, *tmp = (req)->get_cfg_val();
  if (tmp) {
    if (tbl == MAINTBL) {
      val_unw_label *ival = reinterpret_cast<val_unw_label *>(GetVal(req));
      if (ival == NULL) {
        UPLL_LOG_DEBUG("Null Val structure");
        return UPLL_RC_ERR_GENERIC;
      }
      val_unw_label *unw_label_val = ConfigKeyVal::Malloc<val_unw_label>();
      memcpy(unw_label_val, ival, sizeof(val_unw_label));
      tmp1 = new ConfigVal(IpctSt::kIpcStValUnwLabel, unw_label_val);
    }
  };
  void *tkey = (req)->get_key();
  if (!tkey) {
    UPLL_LOG_DEBUG("Null tkey");
    DELETE_IF_NOT_NULL(tmp1);
    return UPLL_RC_ERR_GENERIC;
  }
  key_unw_label *ikey =  reinterpret_cast<key_unw_label *>(tkey);
  key_unw_label *unw_label_key = ConfigKeyVal::Malloc<key_unw_label>();
  memcpy(unw_label_key, ikey, sizeof(key_unw_label));
  okey = new ConfigKeyVal(UNC_KT_UNW_LABEL, IpctSt::kIpcStKeyUnwLabel,
                          unw_label_key, tmp1);
  if (!okey) {
    delete tmp1;
    free(unw_label_key);
    return UPLL_RC_ERR_GENERIC;
  }
  SET_USER_DATA(okey, req);
  return UPLL_RC_SUCCESS;
}

upll_rc_t UNWLabelMoMgr::ValidateAttribute(ConfigKeyVal *ikey,
                                           DalDmlIntf *dmi,
                                           IpcReqRespHeader *req) {
  UPLL_FUNC_TRACE;
  if (ikey == NULL) return UPLL_RC_ERR_GENERIC;
  if (ikey->get_key_type() != UNC_KT_UNW_LABEL)
    return UPLL_RC_ERR_GENERIC;

  if (req->operation == UNC_OP_UPDATE) {
    val_unw_label *unw_label_val = NULL;
    val_unw_label *unw_label_val_db = NULL;
// is below ConfigVal null check required
// (it is alrdy done at validateMessage())
    ConfigVal *cfg_ikey_val = ikey->get_cfg_val();
    if (!cfg_ikey_val) {
        UPLL_LOG_ERROR("Mandatory val structure is null for update");
        return UPLL_RC_ERR_BAD_REQUEST;
      }
    unw_label_val = reinterpret_cast<val_unw_label*>(
                                    ikey->get_cfg_val()->get_val());
    if (unw_label_val == NULL) {
        UPLL_LOG_ERROR("val structure is NULL for UPDATE");
        return UPLL_RC_ERR_BAD_REQUEST;
      }
// in the update when user gives falling  as invalid and
//  raising valid or vise versa then v enter in to the below loop
//  to comapre the value with DB value
  if (((unw_label_val->valid[UPLL_IDX_RAISING_THRESHOLD_UNWL] == UNC_VF_INVALID)
     && (unw_label_val->valid[UPLL_IDX_FALLING_THRESHOLD_UNWL] == UNC_VF_VALID))
     || ((unw_label_val->valid[UPLL_IDX_RAISING_THRESHOLD_UNWL] == UNC_VF_VALID)
     &&(unw_label_val->valid[UPLL_IDX_FALLING_THRESHOLD_UNWL] ==
     UNC_VF_INVALID))) {
    ConfigKeyVal *okey = NULL;
    upll_rc_t result_code = UPLL_RC_SUCCESS;
    result_code = GetChildConfigKey(okey, ikey);
    if (UPLL_RC_SUCCESS != result_code) {
       UPLL_LOG_DEBUG("GetChildConfigKeyFailed %d", result_code);
       return result_code;
    }
    DbSubOp dbop = {kOpReadSingle, kOpMatchNone, kOpInOutFlag};
    result_code = ReadConfigDB(okey, UPLL_DT_CANDIDATE, UNC_OP_READ,
                               dbop, dmi, MAINTBL);
    if (result_code == UPLL_RC_SUCCESS) {
      ConfigVal *cfg_val = okey->get_cfg_val();
      if (!cfg_val) {
        UPLL_LOG_ERROR("Mandatory val structure is null for update");
        DELETE_IF_NOT_NULL(okey);
        return UPLL_RC_ERR_BAD_REQUEST;
      }
      unw_label_val_db = reinterpret_cast<val_unw_label*>(
                                  okey->get_cfg_val()->get_val());
      if (unw_label_val_db == NULL) {
        UPLL_LOG_ERROR("val structure is NULL for UPDATE");
        DELETE_IF_NOT_NULL(okey);
        return UPLL_RC_ERR_BAD_REQUEST;
      }
// compare only if the value in DB is VAILD
      if ((unw_label_val->valid[UPLL_IDX_RAISING_THRESHOLD_UNWL] ==
        UNC_VF_VALID) &&
        (unw_label_val_db->valid[UPLL_IDX_FALLING_THRESHOLD_UNWL] ==
        UNC_VF_VALID)) {
        if (unw_label_val_db-> falling_threshold >
             unw_label_val->raising_threshold) {
          UPLL_LOG_ERROR("falling_threshold is greater than raising_threshold");
          DELETE_IF_NOT_NULL(okey);
          return UPLL_RC_ERR_CFG_SEMANTIC;
        }
      }
// compare only if the value in DB is VAILD
      if ((unw_label_val->valid[UPLL_IDX_FALLING_THRESHOLD_UNWL] ==
        UNC_VF_VALID) &&
        (unw_label_val_db->valid[UPLL_IDX_RAISING_THRESHOLD_UNWL]
        == UNC_VF_VALID)) {
        if (unw_label_val-> falling_threshold >
             unw_label_val_db->raising_threshold) {
          UPLL_LOG_ERROR("falling_threshold is greater than raising_threshold");
          DELETE_IF_NOT_NULL(okey);
          return UPLL_RC_ERR_CFG_SEMANTIC;
        }
      }
      DELETE_IF_NOT_NULL(okey);
    }
    if ((UPLL_RC_SUCCESS != result_code) &&
       (UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code)) {
      UPLL_LOG_DEBUG("ReadConfigDB failed %d", result_code);
      DELETE_IF_NOT_NULL(okey);
      return result_code;
    }
  }
  }
  return UPLL_RC_SUCCESS;
}

bool UNWLabelMoMgr::IsValidKey(void *key,
                               uint64_t index, MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  key_unw_label *unw_label_key = reinterpret_cast<key_unw_label *>(key);
  upll_rc_t ret_val = UPLL_RC_SUCCESS;
  switch (index) {
    case uudst::unw_label::kDbiUnifiedNwName:
      ret_val = ValidateKey(reinterpret_cast<char *>(
              unw_label_key->unified_nw_key.unified_nw_id),
           kMinLenUnifiedNwName, kMaxLenUnifiedNwName);
      if (ret_val != UPLL_RC_SUCCESS) {
        UPLL_LOG_TRACE("Unified nw Name is not valid(%d)", ret_val);
        return false;
      }
      break;
    case uudst::unw_label::kDbiUnwLabelName:
      ret_val = ValidateKey(reinterpret_cast<char *>(
              unw_label_key->unw_label_id),
          kMinLenUnwLabelName, kMaxLenUnwLabelName);
      if (ret_val != UPLL_RC_SUCCESS) {
        UPLL_LOG_TRACE("UNW label Name is not valid(%d)", ret_val);
        return false;
      }
      break;
    default:
      UPLL_LOG_TRACE("Invalid Key Index");
      // ret_val = UPLL_RC_ERR_GENERIC;
      return false;
  }
  return true;
}

upll_rc_t UNWLabelMoMgr::IsReferenced(IpcReqRespHeader *req,
                                       ConfigKeyVal *ikey,
                                       DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  if (!ikey || !dmi) {
    UPLL_LOG_DEBUG("Input argument is NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  ConfigKeyVal *okey = NULL;
  MoMgrImpl *mgr = reinterpret_cast<MoMgrImpl *>(
        const_cast<MoManager*>(GetMoManager(UNC_KT_UNW_SPINE_DOMAIN)));
  if (!mgr) {
    UPLL_LOG_ERROR("Unable to get unified nw spine domain instance");
    return UPLL_RC_ERR_GENERIC;
  }
  result_code = mgr->GetChildConfigKey(okey, NULL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetChildConfigKeyFailed %d", result_code);
    return result_code;
  }
    key_unw_spine_domain *unw_spine_domain_key = NULL;
    unw_spine_domain_key = reinterpret_cast <key_unw_spine_domain*>(
                            okey->get_key());
    uuu::upll_strncpy(unw_spine_domain_key->unw_key.unified_nw_id,
                      reinterpret_cast<key_unw_label *>(
                      ikey->get_key())->unified_nw_key.unified_nw_id,
                      (kMaxLenUnifiedNwName+1));
  val_unw_spdom_ext *unw_spdom_ext_val =
                          ConfigKeyVal::Malloc<val_unw_spdom_ext>();
  uuu::upll_strncpy(unw_spdom_ext_val->val_unw_spine_dom.unw_label_id,
                    reinterpret_cast<key_unw_label *>
                    (ikey->get_key())->unw_label_id,
                    (kMaxLenUnwLabelName+1));
  unw_spdom_ext_val->valid[UPLL_IDX_SPINE_DOMAIN_VAL] = UNC_VF_VALID;
  unw_spdom_ext_val->val_unw_spine_dom.valid[UPLL_IDX_UNW_LABEL_ID_UNWS] =
                                                          UNC_VF_VALID;
  okey->AppendCfgVal(IpctSt::kIpctStValUnwSpineDomain_Ext, unw_spdom_ext_val);
  DbSubOp dbop1 = { kOpReadSingle, kOpMatchNone, kOpInOutNone };
  result_code = mgr->UpdateConfigDB(okey, req->datatype, UNC_OP_READ, dmi,
                                                    &dbop1, MAINTBL);
  DELETE_IF_NOT_NULL(okey);
  if (UPLL_RC_ERR_INSTANCE_EXISTS == result_code) {
    UPLL_LOG_DEBUG("Could not delete an unw label which is referred by"
                     " spine domain instance");
    return UPLL_RC_ERR_CFG_SEMANTIC;
  } else if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
          result_code = UPLL_RC_SUCCESS;
         } else {
            UPLL_LOG_DEBUG("UpdateConfigDB failed %d", result_code);
            return result_code;
            }
  return UPLL_RC_SUCCESS;
}

upll_rc_t UNWLabelMoMgr::GetChildConfigKey(ConfigKeyVal *&okey,
                                           ConfigKeyVal *parent_key) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  key_unw_label_t *unw_label_key = NULL;

  if (okey && okey->get_key()) {
    unw_label_key = reinterpret_cast<key_unw_label_t *>(
                    okey->get_key());
  } else {
    unw_label_key = ConfigKeyVal::Malloc<key_unw_label_t>();
  }
  void *pkey;
  if (parent_key == NULL) {
    if (!okey)
      okey = new ConfigKeyVal(UNC_KT_UNW_LABEL, IpctSt::kIpcStKeyUnwLabel,
                              unw_label_key, NULL);
    else if (okey->get_key() != unw_label_key)
      okey->SetKey(IpctSt::kIpcStKeyUnwLabel, unw_label_key);
    return UPLL_RC_SUCCESS;
  } else {
    pkey = parent_key->get_key();
  }
  if (!pkey) {
    if (!okey || !(okey->get_key()))
      free(unw_label_key);
    return UPLL_RC_ERR_GENERIC;
  }
  switch (parent_key->get_key_type()) {
    case UNC_KT_UNW_LABEL:
      uuu::upll_strncpy(unw_label_key->unified_nw_key.unified_nw_id,
                        reinterpret_cast<key_unw_label_t *>(
                        pkey)->unified_nw_key.unified_nw_id,
                        (kMaxLenUnifiedNwName+1));
      uuu::upll_strncpy(unw_label_key->unw_label_id,
                        reinterpret_cast<key_unw_label_t *>(
                        pkey)->unw_label_id,
                        (kMaxLenUnwLabelName+1));
      break;
    case UNC_KT_UNIFIED_NETWORK:
      uuu::upll_strncpy(unw_label_key->unified_nw_key.unified_nw_id,
                        reinterpret_cast<key_unified_nw_t *>(
                        pkey)->unified_nw_id,
                        (kMaxLenUnifiedNwName+1));
      break;
    default:
      if (unw_label_key)
        free(unw_label_key);
      return UPLL_RC_ERR_GENERIC;
  }
  // if okey is NULL, allocate the memory  and set unw_label_key
  if (!okey)
    okey = new ConfigKeyVal(UNC_KT_UNW_LABEL,
                            IpctSt::kIpcStKeyUnwLabel,
                            unw_label_key, NULL);
  else if (okey->get_key() != unw_label_key)
    okey->SetKey(IpctSt::kIpcStKeyUnwLabel, unw_label_key);
  if (okey == NULL) {
    free(unw_label_key);
    result_code = UPLL_RC_ERR_GENERIC;
  } else {
  SET_USER_DATA(okey, parent_key);
  }
  return result_code;
}

bool UNWLabelMoMgr::CompareValidValue(void *&val1, void *val2,
                                      bool copy_to_running) {
  UPLL_FUNC_TRACE;
  bool invalid_attr = true;
  val_unw_label *val_unwl1 = reinterpret_cast<val_unw_label *>(val1);
  val_unw_label *val_unwl2 = reinterpret_cast<val_unw_label *>(val2);
  if (!val_unwl1 || !val_unwl2) {
    UPLL_LOG_DEBUG("Invalid param");
    return false;
  }
  for (unsigned int loop = 0;
      loop < sizeof(val_unwl1->valid) / sizeof(uint8_t); ++loop) {
    if (UNC_VF_INVALID == val_unwl1->valid[loop]
        && UNC_VF_VALID == val_unwl2->valid[loop])
      val_unwl1->valid[loop] = UNC_VF_VALID_NO_VALUE;
  }
  if (UNC_VF_VALID == val_unwl1->valid[UPLL_IDX_MAX_COUNT_UNWL] &&
      UNC_VF_VALID == val_unwl2->valid[UPLL_IDX_MAX_COUNT_UNWL]) {
    if (val_unwl1->max_count == val_unwl2->max_count)
      val_unwl1->valid[UPLL_IDX_MAX_COUNT_UNWL] = UNC_VF_INVALID;
  }
  if (UNC_VF_VALID == val_unwl1->valid[UPLL_IDX_RAISING_THRESHOLD_UNWL] &&
      UNC_VF_VALID == val_unwl2->valid[UPLL_IDX_RAISING_THRESHOLD_UNWL]) {
    if (val_unwl1->raising_threshold  == val_unwl2->raising_threshold)
      val_unwl1->valid[UPLL_IDX_RAISING_THRESHOLD_UNWL] = UNC_VF_INVALID;
  }
  if (UNC_VF_VALID == val_unwl1->valid[UPLL_IDX_FALLING_THRESHOLD_UNWL] &&
      UNC_VF_VALID == val_unwl2->valid[UPLL_IDX_FALLING_THRESHOLD_UNWL]) {
    if (val_unwl1->falling_threshold == val_unwl2->falling_threshold)
      val_unwl1->valid[UPLL_IDX_FALLING_THRESHOLD_UNWL] = UNC_VF_INVALID;
  }
  for (unsigned int loop = 0;
      loop < sizeof(val_unwl1->valid) / sizeof(uint8_t); ++loop) {
    if ((UNC_VF_VALID == (uint8_t) val_unwl1->valid[loop]) ||
        (UNC_VF_VALID_NO_VALUE == (uint8_t) val_unwl1->valid[loop])) {
      invalid_attr = false;
      break;
    }
  }
  return invalid_attr;
}

upll_rc_t UNWLabelMoMgr::AllocVal(ConfigVal *&ck_val,
                                  upll_keytype_datatype_t dt_type,
                                  MoMgrTables tbl) {
  void *val;  //  *ck_nxtval;
  if (ck_val != NULL) return UPLL_RC_ERR_GENERIC;
  switch (tbl) {
    case MAINTBL:
      val = ConfigKeyVal::Malloc(sizeof(val_unw_label));
      ck_val = new ConfigVal(IpctSt::kIpcStValUnwLabel, val);
      break;
    default:
      val = NULL;
      break;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t UNWLabelMoMgr::ValidateUnwLabelVal(IpcReqRespHeader * req,
                                             ConfigKeyVal * ikey) {
  UPLL_FUNC_TRACE;
  // Validate value structure and opertaion type
  ConfigVal *cfg_val = ikey->get_cfg_val();
  val_unw_label *unw_label_val = NULL;
    bool ret_val = false;

  if (!cfg_val) {
    if (req->operation == UNC_OP_UPDATE) {
      UPLL_LOG_ERROR("Mandatory val structure is null for update");
      return UPLL_RC_ERR_BAD_REQUEST;
    }
    return UPLL_RC_SUCCESS;
  }
  switch (req->operation) {
    case UNC_OP_CREATE:
    case UNC_OP_UPDATE:
    case UNC_OP_READ:
    case UNC_OP_READ_SIBLING:
    case UNC_OP_READ_SIBLING_BEGIN:
    case UNC_OP_READ_SIBLING_COUNT:
      if (cfg_val->get_st_num() != IpctSt::kIpcStValUnwLabel) {
        UPLL_LOG_ERROR("Invalid val struct st num-%d",
                       (ikey->get_cfg_val())->get_st_num());
        return UPLL_RC_ERR_BAD_REQUEST;
      }
      unw_label_val = reinterpret_cast<val_unw_label*>(
                                    ikey->get_cfg_val()->get_val());
      if (unw_label_val == NULL) {
        if (req->operation == UNC_OP_UPDATE) {
          UPLL_LOG_ERROR("val structure is NULL for UPDATE");
          return UPLL_RC_ERR_BAD_REQUEST;
        }
        return UPLL_RC_SUCCESS;
      }
    // Attribute syntax validation
      for (unsigned int valid_index = 0;
           valid_index < sizeof(unw_label_val->valid) / sizeof(
           unw_label_val->valid[0]); valid_index++) {
        if (unw_label_val->valid[valid_index] == UNC_VF_VALID) {
          switch (valid_index) {
            case UPLL_IDX_MAX_COUNT_UNWL:
              ret_val = ValidateNumericRange(unw_label_val->max_count,
                                           kUnwLabelMinRange, kUnwLabelMaxRange,
                                           true , true);
              break;
           case UPLL_IDX_RAISING_THRESHOLD_UNWL:
              ret_val = ValidateNumericRange(unw_label_val->raising_threshold,
                                          kUnwLabelMinRange, kUnwLabelMaxRange,
                                          true , true);
              break;
          case UPLL_IDX_FALLING_THRESHOLD_UNWL:
              ret_val = ValidateNumericRange(unw_label_val->falling_threshold,
                                          kUnwLabelMinRange, kUnwLabelMaxRange,
                                          true , true);
              break;
          }
          if (!ret_val) {
            UPLL_LOG_ERROR("syntax check failed");
            return UPLL_RC_ERR_CFG_SYNTAX;
          }
        }
      }
    // reset attributes if VALID_NO_VALUE
      for (unsigned int valid_index = 0;
          valid_index < sizeof(unw_label_val->valid) /
          sizeof(unw_label_val->valid[0]); valid_index++) {
        if ((unw_label_val->valid[valid_index] == UNC_VF_VALID_NO_VALUE) ||
            (unw_label_val->valid[valid_index] == UNC_VF_INVALID)) {
          switch (valid_index) {
            case UPLL_IDX_MAX_COUNT_UNWL:
              unw_label_val->max_count = 0;
              break;
            case UPLL_IDX_RAISING_THRESHOLD_UNWL:
              unw_label_val->raising_threshold = 0;
              break;
            case UPLL_IDX_FALLING_THRESHOLD_UNWL:
              unw_label_val->falling_threshold = 0;
              break;
            default:
              UPLL_LOG_TRACE("Never here");
              break;
          }
        }
      }
      if ((unw_label_val->valid[UPLL_IDX_RAISING_THRESHOLD_UNWL] ==
          UNC_VF_VALID) &&
          (unw_label_val->valid[UPLL_IDX_FALLING_THRESHOLD_UNWL] ==
          UNC_VF_VALID)) {
        if (unw_label_val-> falling_threshold >
            unw_label_val->raising_threshold) {
          UPLL_LOG_ERROR("falling_threshold is greater than raising_threshold");
          return UPLL_RC_ERR_CFG_SEMANTIC;
        }
      }
    case UNC_OP_DELETE:
      return UPLL_RC_SUCCESS;
      break;
    default:
      UPLL_LOG_ERROR("Invalid operation(%d)", req->operation);
      return UPLL_RC_ERR_BAD_REQUEST;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t UNWLabelMoMgr::ValidateUnwLabelKey(key_unw_label_t *unw_label_key,
                                           unc_keytype_operation_t operation) {
  UPLL_FUNC_TRACE;
  upll_rc_t ret_val = UPLL_RC_SUCCESS;
  ret_val = ValidateKey(reinterpret_cast<char *>(
          unw_label_key->unified_nw_key.unified_nw_id),
          kMinLenUnifiedNwName, kMaxLenUnifiedNwName);
  if (ret_val != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Unified network name syntax check failed."
                   "Received unified_network_name - %s",
                   unw_label_key->unified_nw_key.unified_nw_id);
    return UPLL_RC_ERR_CFG_SYNTAX;
  }
  if ((operation != UNC_OP_READ_SIBLING_COUNT) &&
      (operation != UNC_OP_READ_SIBLING_BEGIN)) {
    ret_val = ValidateKey(reinterpret_cast<char *>(
                        unw_label_key->unw_label_id),
                        kMinLenUnwLabelName,
                        kMaxLenUnwLabelName);
    if (ret_val != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Unified network label name syntax check failed."
                    "Received unw_label_name - %s",
                    unw_label_key->unw_label_id);
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  } else {
    UPLL_LOG_TRACE("Operation is %d", operation);
    StringReset(unw_label_key->unw_label_id);
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t UNWLabelMoMgr::ValidateMessage(IpcReqRespHeader * req,
                                         ConfigKeyVal * ikey)  {
  UPLL_FUNC_TRACE;
  upll_rc_t ret_val = UPLL_RC_SUCCESS;
  if (!ikey || !req || !(ikey->get_key())) {
    UPLL_LOG_ERROR("ConfigKeyVal / IpcReqRespHeader is Null");
    return UPLL_RC_ERR_GENERIC;
  }
  if (req->option1 != UNC_OPT1_NORMAL) {
    UPLL_LOG_ERROR("Error: option1 is not NORMAL");
    return UPLL_RC_ERR_INVALID_OPTION1;
  }
  if (req->option2 != UNC_OPT2_NONE) {
    UPLL_LOG_ERROR("Error: option2 is not NONE");
    return UPLL_RC_ERR_INVALID_OPTION2;
  }
  if (ikey->get_st_num() != IpctSt::kIpcStKeyUnwLabel) {
    UPLL_LOG_ERROR("Invalid key structure received. received struct - %d",
                  (ikey->get_st_num()));
    return UPLL_RC_ERR_BAD_REQUEST;
  }
  if (ikey->get_key_type() != UNC_KT_UNW_LABEL) {
    UPLL_LOG_ERROR("Invalid keytype(%d) received.", ikey->get_key_type());
    return UPLL_RC_ERR_BAD_REQUEST;
  }
  if (!((req->datatype == UPLL_DT_CANDIDATE) ||
        (req->datatype == UPLL_DT_RUNNING) ||
        (req->datatype == UPLL_DT_STARTUP) ||
        (req->datatype == UPLL_DT_STATE))) {
    UPLL_LOG_ERROR("Invalid datatype(%d) received.", req->datatype);
    return UPLL_RC_ERR_BAD_REQUEST;
  }
  // Validate key structure
  key_unw_label *unw_label_key = reinterpret_cast<key_unw_label *>
                                                (ikey->get_key());
  unc_keytype_operation_t operation = req->operation;
  ret_val = ValidateUnwLabelKey(unw_label_key, operation);
  if (ret_val != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("syntax check failed for key_unw_label struct");
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  // Validate value strcture
  ret_val = ValidateUnwLabelVal(req, ikey);
  return ret_val;
}

upll_rc_t UNWLabelMoMgr::UpdateConfigStatus(ConfigKeyVal *ikey,
                                            unc_keytype_operation_t op,
                                            uint32_t driver_result,
                                            ConfigKeyVal *upd_key,
                                            DalDmlIntf *dmi,
                                            ConfigKeyVal *ctrlr_key) {
  UPLL_FUNC_TRACE;
  val_unw_label *unw_label_val =
      reinterpret_cast<val_unw_label *>(GetVal(ikey));
  if (unw_label_val == NULL)
    return UPLL_RC_ERR_GENERIC;
  unw_label_val->cs_row_status = UNC_CS_APPLIED;
  if (unw_label_val->valid[UPLL_IDX_MAX_COUNT_UNWL] == UNC_VF_VALID)
  unw_label_val->cs_attr[UPLL_IDX_MAX_COUNT_UNWL] = UNC_CS_APPLIED;
  if (unw_label_val->valid[UPLL_IDX_RAISING_THRESHOLD_UNWL] == UNC_VF_VALID)
  unw_label_val->cs_attr[UPLL_IDX_RAISING_THRESHOLD_UNWL] = UNC_CS_APPLIED;
  if (unw_label_val->valid[UPLL_IDX_FALLING_THRESHOLD_UNWL] == UNC_VF_VALID)
  unw_label_val->cs_attr[UPLL_IDX_FALLING_THRESHOLD_UNWL] = UNC_CS_APPLIED;
  if (op == UNC_OP_UPDATE) {
    void *ival = reinterpret_cast<void *>(unw_label_val);
    CompareValidValue(ival, GetVal(upd_key), true);
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t UNWLabelMoMgr::GetOperation(uuc::UpdateCtrlrPhase phase,
                                      unc_keytype_operation_t &op) {
  if (uuc::kUpllUcpDelete2 == phase) {
    UPLL_LOG_DEBUG("Delete phase 1");
    return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_KT;
  } else if (uuc::kUpllUcpUpdate == phase) {
    op = UNC_OP_UPDATE;
  } else if (uuc::kUpllUcpCreate == phase) {
    op = UNC_OP_CREATE;
  } else if (uuc::kUpllUcpDelete == phase) {
    op = UNC_OP_DELETE;
  } else {
    return UPLL_RC_ERR_GENERIC;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t UNWLabelMoMgr::GetParentConfigKey(ConfigKeyVal *&okey,
                                            ConfigKeyVal *ikey) {
  UPLL_FUNC_TRACE;
  if (ikey == NULL) {
    UPLL_LOG_DEBUG("Null ikey param");
    return UPLL_RC_ERR_GENERIC;
  }
  unc_key_type_t ikey_type = ikey->get_key_type();
  if (ikey_type != UNC_KT_UNW_LABEL) {
    UPLL_LOG_DEBUG("Invalid input arg");
    return UPLL_RC_ERR_GENERIC;
  }
  DELETE_IF_NOT_NULL(okey);
  key_unw_label *unwl_key  = reinterpret_cast<key_unw_label*>(ikey->get_key());
  if (!unwl_key) {
    UPLL_LOG_DEBUG("NULL ikey");
    return UPLL_RC_ERR_GENERIC;
  }
  key_unified_nw_t *unw_key = ConfigKeyVal::Malloc<key_unified_nw_t>();
  uuu::upll_strncpy(unw_key->unified_nw_id,
                    unwl_key->unified_nw_key.unified_nw_id,
                    (kMaxLenUnifiedNwName+1));
  okey = new ConfigKeyVal(UNC_KT_UNIFIED_NETWORK, IpctSt::kIpcStKeyUnifiedNw,
                          unw_key, NULL);
  if (okey == NULL) {
    free(unw_key);
    UPLL_LOG_DEBUG("NULL okey");
    return UPLL_RC_ERR_GENERIC;
  } else {
    SET_USER_DATA(okey, ikey);
  }
  return UPLL_RC_SUCCESS;
}
}  // namespace kt_momgr
}  // namespace upll
}  // namespace unc
