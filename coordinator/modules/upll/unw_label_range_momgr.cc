/*
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include "unw_label_range_momgr.hh"

#include "upll_validation.hh"
#include "uncxx/upll_log.hh"
#include "unc/uppl_common.h"

using unc::upll::ipc_util::IpcUtil;
namespace uuds = unc::upll::dal::schema;

namespace unc {
namespace upll {
namespace kt_momgr {

uint16_t UNWLabelRangeMoMgr::kUnwlRangeNumChildKey = 2;  // min range, max range

BindInfo UNWLabelRangeMoMgr::unw_label_range_bind_info[] = {
  { uudst::unw_label_range::kDbiUnifiedNwName, CFG_KEY, offsetof(
    key_unw_label_range, unw_label_key.unified_nw_key.unified_nw_id),
    uud::kDalChar, 32 },
  { uudst::unw_label_range::kDbiUnwLabelName, CFG_KEY,
    offsetof(key_unw_label_range, unw_label_key.unw_label_id),
    uud::kDalChar, 32 },
  { uudst::unw_label_range::kDbiMinRange, CFG_KEY, offsetof(
          key_unw_label_range, range_min),
    uud::kDalUint32, 1 },
  { uudst::unw_label_range::kDbiMaxRange, CFG_KEY, offsetof(
          key_unw_label_range, range_max),
    uud::kDalUint32, 1 },
  { uudst::unw_label_range::kDbiCsRowStatus, CS_VAL, offsetof(
          val_unw_label_range, cs_row_status),
    uud::kDalUint8, 1 }
};

UNWLabelRangeMoMgr::UNWLabelRangeMoMgr() : MoMgrImpl() {
  UPLL_FUNC_TRACE;
  ntable = MAX_MOMGR_TBLS;
  table = new Table *[ntable]();
  table[MAINTBL] = new Table(
      uudst::kDbiUnwLabelRangeTbl, UNC_KT_UNW_LABEL_RANGE,
      unw_label_range_bind_info, IpctSt::kIpcStKeyUnwLabelRange,
      IpctSt::kIpcStValUnwLabelRange,
      uudst::unw_label_range::kDbiUnwLabelRangeNumCols);
  table[RENAMETBL] = NULL;
  table[CTRLRTBL] = NULL;
  table[CONVERTTBL] = NULL;
  nchild = 0;
  child  = NULL;
}

upll_rc_t UNWLabelRangeMoMgr::DupConfigKeyVal(ConfigKeyVal *&okey,
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
  if (req->get_key_type() != UNC_KT_UNW_LABEL_RANGE) {
    UPLL_LOG_DEBUG("Input ConfigKeyVal keytype mismatch");
    return UPLL_RC_ERR_GENERIC;
  }
  ConfigVal *tmp1 = NULL, *tmp = (req)->get_cfg_val();
  if (tmp) {
    if (tbl == MAINTBL) {
      val_unw_label_range *ival = reinterpret_cast<val_unw_label_range *>(
                                                  GetVal(req));
      if (ival == NULL) {
        UPLL_LOG_DEBUG("Null Val structure");
        return UPLL_RC_ERR_GENERIC;
      }
      val_unw_label_range *unw_label_range_val =
                            ConfigKeyVal::Malloc<val_unw_label_range>();
      memcpy(unw_label_range_val, ival, sizeof(val_unw_label_range));
      tmp1 = new ConfigVal(IpctSt::kIpcStValUnwLabelRange, unw_label_range_val);
    }
  };
  void *tkey = (req)->get_key();
  if (!tkey) {
    UPLL_LOG_DEBUG("Null tkey");
    DELETE_IF_NOT_NULL(tmp1);
    return UPLL_RC_ERR_GENERIC;
  }
  key_unw_label_range *ikey =  reinterpret_cast<key_unw_label_range *>(tkey);
  key_unw_label_range *unw_label_range_key =
                        ConfigKeyVal::Malloc<key_unw_label_range>();
  memcpy(unw_label_range_key, ikey, sizeof(key_unw_label_range));
  okey = new ConfigKeyVal(UNC_KT_UNW_LABEL_RANGE,
                   IpctSt::kIpcStKeyUnwLabelRange, unw_label_range_key, tmp1);
  if (!okey) {
    delete tmp1;
    free(unw_label_range_key);
    return UPLL_RC_ERR_GENERIC;
  }
  SET_USER_DATA(okey, req);
  return UPLL_RC_SUCCESS;
}

upll_rc_t UNWLabelRangeMoMgr::ValidateAttribute(ConfigKeyVal *ikey,
                                      DalDmlIntf *dmi,
                                      IpcReqRespHeader *req) {
  UPLL_FUNC_TRACE;
  if (ikey == NULL) return UPLL_RC_ERR_GENERIC;

  if (ikey->get_key_type() != UNC_KT_UNW_LABEL_RANGE)
    return UPLL_RC_ERR_GENERIC;

  return UPLL_RC_SUCCESS;
}

bool UNWLabelRangeMoMgr::IsValidKey(void *key,
                          uint64_t index, MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  key_unw_label_range *unw_label_range_key = reinterpret_cast<
      key_unw_label_range *>(key);
  upll_rc_t ret_val = UPLL_RC_SUCCESS;

  switch (index) {
    case uudst::unw_label_range::kDbiUnifiedNwName:
      ret_val = ValidateKey(reinterpret_cast<char *>(
              unw_label_range_key->unw_label_key.
              unified_nw_key.unified_nw_id),
           kMinLenUnifiedNwName, kMaxLenUnifiedNwName);
      break;
    case uudst::unw_label_range::kDbiUnwLabelName:
      ret_val = ValidateKey(reinterpret_cast<char *>(
              unw_label_range_key->unw_label_key.
              unw_label_id),
          kMinLenUnwLabelName, kMaxLenUnwLabelName);
      break;
    case uudst::unw_label_range::kDbiMinRange:
      if (!ValidateNumericRange(unw_label_range_key->range_min,
                kUnwLabelMinRange, kUnwLabelMaxRange,
                true, true)) {
        UPLL_LOG_DEBUG("Min range syntax validation failed");
        ret_val = UPLL_RC_ERR_CFG_SYNTAX;
      }
      break;
    case uudst::unw_label_range::kDbiMaxRange:
      if (!ValidateNumericRange(unw_label_range_key->range_max,
                kUnwLabelMinRange, kUnwLabelMaxRange,
                true , true)) {
        UPLL_LOG_DEBUG("Max range syntax validation failed");
        ret_val = UPLL_RC_ERR_CFG_SYNTAX;
      }
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

upll_rc_t UNWLabelRangeMoMgr::GetChildConfigKey(ConfigKeyVal *&okey,
                                      ConfigKeyVal *parent_key) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  key_unw_label_range_t *unwl_range_key = NULL;

  if (okey && okey->get_key()) {
    unwl_range_key = reinterpret_cast<
        key_unw_label_range_t *>(okey->get_key());
  } else {
    unwl_range_key = ConfigKeyVal::Malloc<key_unw_label_range_t>();
  }
  void *pkey;
  if (parent_key == NULL) {
    if (!okey)
      okey = new ConfigKeyVal(UNC_KT_UNW_LABEL_RANGE,
                              IpctSt::kIpcStKeyUnwLabelRange, unwl_range_key,
                              NULL);
    else if (okey->get_key() != unwl_range_key)
      okey->SetKey(IpctSt::kIpcStKeyUnwLabelRange, unwl_range_key);
    return UPLL_RC_SUCCESS;
  } else {
    pkey = parent_key->get_key();
  }
  if (!pkey) {
    if (!okey || !(okey->get_key()))
      free(unwl_range_key);
    return UPLL_RC_ERR_GENERIC;
  }
  switch (parent_key->get_key_type()) {
    case UNC_KT_UNW_LABEL_RANGE:
      uuu::upll_strncpy(
        unwl_range_key->unw_label_key.unified_nw_key.unified_nw_id,
        reinterpret_cast<key_unw_label_range *>
        (pkey)->unw_label_key.unified_nw_key.unified_nw_id,
        (kMaxLenUnifiedNwName+1));
      uuu::upll_strncpy(unwl_range_key->unw_label_key.unw_label_id,
        reinterpret_cast<key_unw_label_range *>
        (pkey)->unw_label_key.unw_label_id,
        (kMaxLenUnwLabelName+1));
      unwl_range_key->range_min = reinterpret_cast<key_unw_label_range *>
                                  (pkey)->range_min;
      unwl_range_key->range_max = reinterpret_cast<key_unw_label_range *>
                                  (pkey)->range_max;
      break;
    case UNC_KT_UNIFIED_NETWORK:
      uuu::upll_strncpy(
        unwl_range_key->unw_label_key.unified_nw_key.unified_nw_id,
        reinterpret_cast<key_unified_nw *>
        (pkey)->unified_nw_id,
        (kMaxLenUnifiedNwName+1));
      break;
    case UNC_KT_UNW_LABEL:
      uuu::upll_strncpy(
        unwl_range_key->unw_label_key.unified_nw_key.unified_nw_id,
        reinterpret_cast<key_unw_label*>
        (pkey)->unified_nw_key.unified_nw_id,
        (kMaxLenUnifiedNwName+1));
      uuu::upll_strncpy(unwl_range_key->unw_label_key.unw_label_id,
        reinterpret_cast<key_unw_label*>
        (pkey)->unw_label_id, (kMaxLenUnwLabelName+1));
      break;
    default:
      if (unwl_range_key)
        free(unwl_range_key);
      return UPLL_RC_ERR_GENERIC;
  }
  // if okey is NULL, allocate the memory  and set unw_label_range_key
  if (!okey)
    okey = new ConfigKeyVal(UNC_KT_UNW_LABEL_RANGE,
                            IpctSt::kIpcStKeyUnwLabelRange,
                            unwl_range_key, NULL);
  else if (okey->get_key() != unwl_range_key)
    okey->SetKey(IpctSt::kIpcStKeyUnwLabelRange, unwl_range_key);
  if (okey == NULL) {
    free(unwl_range_key);
    result_code = UPLL_RC_ERR_GENERIC;
  } else {
  SET_USER_DATA(okey, parent_key);
  }
  return result_code;
}

bool UNWLabelRangeMoMgr::CompareValidValue(void *&val1, void *val2,
                                 bool copy_to_running) {
  UPLL_FUNC_TRACE;
  return true;
}

upll_rc_t UNWLabelRangeMoMgr::AllocVal(ConfigVal *&ck_val,
                             upll_keytype_datatype_t dt_type,
                             MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  void *val;  //  *ck_nxtval;
  if (ck_val != NULL) return UPLL_RC_ERR_GENERIC;
  switch (tbl) {
    case MAINTBL:
      val = ConfigKeyVal::Malloc(sizeof(val_unw_label_range));
      ck_val = new ConfigVal(IpctSt::kIpcStValUnwLabelRange, val);
      break;
    default:
      val = NULL;
      break;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t
UNWLabelRangeMoMgr::ValidateMessage(IpcReqRespHeader * req,
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
  if (ikey->get_st_num() != IpctSt::kIpcStKeyUnwLabelRange) {
    UPLL_LOG_ERROR("Invalid key structure received. received struct - %d",
                  (ikey->get_st_num()));
    return UPLL_RC_ERR_BAD_REQUEST;
  }
  if (ikey->get_key_type() != UNC_KT_UNW_LABEL_RANGE) {
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
  key_unw_label_range *unw_label_range_key = reinterpret_cast<
      key_unw_label_range *> (ikey->get_key());

  ret_val = ValidateKey(reinterpret_cast<char *>(
          unw_label_range_key->unw_label_key.
          unified_nw_key.unified_nw_id),
          kMinLenUnifiedNwName, kMaxLenUnifiedNwName);
  if (ret_val != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Unified network name syntax check failed."
                   "Received unified_network_name - %s",
                   unw_label_range_key->unw_label_key.
                    unified_nw_key.unified_nw_id);
    return ret_val;
  }
  if ((req->operation != UNC_OP_READ_SIBLING_COUNT) &&
    (req->operation != UNC_OP_READ_SIBLING_BEGIN)) {
    ret_val = ValidateKey(reinterpret_cast<char *>(
          unw_label_range_key->unw_label_key.unw_label_id),
        kMinLenUnwLabelName, kMaxLenUnwLabelName);
    if (ret_val != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Unified network label name (%s)syntax check failed.",
                  unw_label_range_key->unw_label_key.
                  unw_label_id);
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
    if (!ValidateNumericRange(unw_label_range_key->range_min,
              kUnwLabelMinRange, kUnwLabelMaxRange,
              true , true)) {
      UPLL_LOG_ERROR("Min range syntax validation failed");
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
    if (!ValidateNumericRange(unw_label_range_key->range_max,
              kUnwLabelMinRange, kUnwLabelMaxRange,
              true , true)) {
      UPLL_LOG_ERROR("Max range syntax validation failed");
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
    if (unw_label_range_key->range_min > unw_label_range_key->range_max) {
      UPLL_LOG_ERROR("Min range is greater than max_range");
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  } else {
    UPLL_LOG_TRACE("Operation is %d", req->operation);
    unw_label_range_key->range_min = 0;
    unw_label_range_key->range_max = 0;
  }
  if (req->operation == UNC_OP_UPDATE) {
     UPLL_LOG_ERROR("Update operation is not allowed for unw label range");
     return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_KT;
  }
  return ret_val;
}

upll_rc_t UNWLabelRangeMoMgr::UpdateConfigStatus(ConfigKeyVal *ikey,
                                                 unc_keytype_operation_t op,
                                                 uint32_t driver_result,
                                                 ConfigKeyVal *upd_key,
                                                 DalDmlIntf *dmi,
                                                 ConfigKeyVal *ctrlr_key) {
  UPLL_FUNC_TRACE;
  val_unw_label_range *unw_label_range_val =
      reinterpret_cast<val_unw_label_range *>(GetVal(ikey));
  if (unw_label_range_val == NULL)
    return UPLL_RC_ERR_GENERIC;
  unw_label_range_val->cs_row_status = UNC_CS_APPLIED;
  return UPLL_RC_SUCCESS;
}

upll_rc_t UNWLabelRangeMoMgr::GetOperation(uuc::UpdateCtrlrPhase phase,
                                unc_keytype_operation_t &op) {
  if (uuc::kUpllUcpDelete2 == phase) {
    UPLL_LOG_DEBUG("Delete phase 1");
    return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_KT;
  } else if (uuc::kUpllUcpUpdate == phase) {
    return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_KT;
  } else if (uuc::kUpllUcpCreate == phase) {
    op = UNC_OP_CREATE;
  } else if (uuc::kUpllUcpDelete == phase) {
    op = UNC_OP_DELETE;
  } else {
    return UPLL_RC_ERR_GENERIC;
  }
  return UPLL_RC_SUCCESS;
}

// Overridden Read Sibling from momgr_impl.
// This keytype contains 2 child keys and needs special handling.
upll_rc_t UNWLabelRangeMoMgr::ReadSiblingMo(
    IpcReqRespHeader *header, ConfigKeyVal *ikey, bool begin, DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  if (begin == true) {
    return (MoMgrImpl::ReadSiblingMo(header, ikey, begin, dmi));
  }

  upll_rc_t result_code = UPLL_RC_ERR_GENERIC;
  uint32_t result_count = 0;
  uint32_t req_count = header->rep_count;
  header->rep_count = 0;

  ConfigKeyVal *result_ckv = NULL;
  if (header->operation != UNC_OP_READ_SIBLING) {
    UPLL_LOG_ERROR("Operation type is not Sibling begin/Sibling");
    return UPLL_RC_ERR_GENERIC;
  }
  if (ikey == NULL || dmi == NULL) {
    UPLL_LOG_ERROR("Null Param ikey/dmi");
    return UPLL_RC_ERR_GENERIC;
  }
  result_code = ValidateMessage(header, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("ValidateMessage failed result_code %d", result_code);
    return result_code;
  }
  MoMgrTables tbl = MAINTBL;
  DbSubOp dbop = {kOpReadMultiple, kOpMatchNone, kOpInOutNone};

  // To read the record(s) from DB
  upll_keytype_datatype_t dt_type = ((header->datatype == UPLL_DT_STATE) ?
                                     UPLL_DT_RUNNING : header->datatype);
  if (UPLL_DT_RUNNING == header->datatype) {
    UPLL_LOG_TRACE("Status Flag enabled");
    dbop.inoutop |= kOpInOutCs;
  }

  ConfigVal *cval = ikey->get_cfg_val();
  ConfigVal *new_cval = NULL;
  AllocVal(new_cval, dt_type, MAINTBL);
  if (new_cval == NULL) {
    UPLL_LOG_ERROR("Null new_cval after AllocVal");
    return UPLL_RC_ERR_GENERIC;
  }

  if (cval != NULL && cval->get_val() != NULL) {
    memcpy(new_cval->get_val(), cval->get_val(), sizeof(val_unw_label_range));
  }
  ikey->SetCfgVal(new_cval);

  ConfigKeyVal *queryckval = NULL;
  result_code = DupConfigKeyVal(queryckval, ikey, MAINTBL);
  if (result_code != UPLL_RC_SUCCESS || queryckval == NULL) {
    UPLL_LOG_ERROR("Error while DupConfigKeyVal");
    return UPLL_RC_ERR_GENERIC;
  }

  const uint16_t num_pk = uuds::TableNumPkCols(uudst::kDbiUnwLabelRangeTbl);
  for (uint16_t childKeyIdx = num_pk - 1;
       (childKeyIdx >= (num_pk - kUnwlRangeNumChildKey)) &&
       (result_count < req_count);
       childKeyIdx--) {
/*
    if (IsValidKey(queryckval->get_key(), childKeyIdx) == false) {
      continue;
    }
*/
    uint32_t remaining_count = req_count - result_count;
    result_code = ReadConfigDB(queryckval, dt_type, header->operation,
                               dbop, remaining_count,
                               dmi, tbl);
    if (result_code == UPLL_RC_SUCCESS) {
      for (ConfigKeyVal *queryckval_iter = queryckval;
           queryckval_iter;
           queryckval_iter = queryckval_iter->get_next_cfg_key_val()) {
         result_count++;
        if (result_count == req_count) {
          UPLL_LOG_TRACE("Fetched (%d) records", result_count);
          ConfigKeyVal *tmp = queryckval_iter->get_next_cfg_key_val();
          queryckval_iter->set_next_cfg_key_val(NULL);
          DELETE_IF_NOT_NULL(tmp);
          break;
        }
      }
      UPLL_LOG_TRACE("Appending ConfigKeyVal");
      if (result_ckv == NULL) {
        result_ckv = queryckval;
      } else {
        result_ckv->AppendCfgKeyVal(queryckval);
      }
      queryckval = NULL;
    } else if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      DELETE_IF_NOT_NULL(queryckval);
    } else {
      UPLL_LOG_DEBUG("Read configDb failed %d", result_code);
      DELETE_IF_NOT_NULL(queryckval);
      return result_code;
    }
    if (result_count < req_count) {
      queryckval = NULL;
      result_code = DupConfigKeyVal(queryckval, ikey, MAINTBL);
      if (result_code != UPLL_RC_SUCCESS || queryckval == NULL) {
        UPLL_LOG_ERROR("Error while DupConfigKeyVal");
        return UPLL_RC_ERR_GENERIC;
      }

      // Preparing Child Key data for next Sibling Iteration
      key_unw_label_range *key_unwlr =
        reinterpret_cast<key_unw_label_range *>(queryckval->get_key());
      switch (childKeyIdx) {
        case uudst::unw_label_range::kDbiMinRange:
          key_unwlr->range_min = 0;  // Invalid range
          // no need of break
        case uudst::unw_label_range::kDbiMaxRange:
          key_unwlr->range_max = 0;  // Invalid range
          break;
        default:
          UPLL_LOG_ERROR("Never comes here");
          return UPLL_RC_ERR_GENERIC;
      }
      if (GetVal(ikey)) {
        memcpy(queryckval->get_cfg_val()->get_val(),
               ikey->get_cfg_val()->get_val(), sizeof(val_unw_label_range));
      } else {
        queryckval->SetCfgVal(NULL);
      }
      UPLL_LOG_TRACE("Next Query with : %s", (queryckval->ToStrAll()).c_str());
    }
  }

  DELETE_IF_NOT_NULL(queryckval);

  header->rep_count = result_count;
  if (header->rep_count > 0) {
    result_code = (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE)
                  ? UPLL_RC_SUCCESS : result_code;
    if (result_code == UPLL_RC_SUCCESS) {
      ikey->ResetWith(result_ckv);
      delete result_ckv;
      result_code = AdaptValToVtnService(ikey);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("AdaptValToVtnService failed %d", result_code);
        header->rep_count = 0;
        return result_code;
      }
    }
  } else {
    result_code = UPLL_RC_ERR_NO_SUCH_INSTANCE;
  }
  return result_code;
}

upll_rc_t UNWLabelRangeMoMgr::GetParentConfigKey(ConfigKeyVal *&okey,
                                                 ConfigKeyVal *ikey) {
  UPLL_FUNC_TRACE;
  if (ikey == NULL) {
    UPLL_LOG_DEBUG("Null ikey param");
    return UPLL_RC_ERR_GENERIC;
  }
  unc_key_type_t ikey_type = ikey->get_key_type();
  if (ikey_type != UNC_KT_UNW_LABEL_RANGE) {
    UPLL_LOG_DEBUG("Invalid input arg");
    return UPLL_RC_ERR_GENERIC;
  }
  DELETE_IF_NOT_NULL(okey);
  key_unw_label_range *unwlr_key  = reinterpret_cast<key_unw_label_range*>
                                    (ikey->get_key());
  if (!unwlr_key) {
    UPLL_LOG_DEBUG("NULL ikey");
    return UPLL_RC_ERR_GENERIC;
  }
  key_unw_label *unwl_key = ConfigKeyVal::Malloc<key_unw_label>();
  uuu::upll_strncpy(unwl_key->unified_nw_key.unified_nw_id,
                    reinterpret_cast<key_unw_label_range*>(
                    unwlr_key)->unw_label_key.unified_nw_key.unified_nw_id,
                    (kMaxLenUnifiedNwName+1));
  uuu::upll_strncpy(unwl_key->unw_label_id,
                    reinterpret_cast<key_unw_label_range*>(
                    unwlr_key)->unw_label_key.unw_label_id,
                    (kMaxLenUnwLabelName+1));
  okey = new ConfigKeyVal(UNC_KT_UNW_LABEL, IpctSt::kIpcStKeyUnwLabel,
                          unwl_key, NULL);
  if (okey == NULL) {
    free(unwl_key);
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
