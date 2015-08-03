/*
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
#include "unified_nw_momgr.hh"
#include "upll_validation.hh"
#include "uncxx/upll_log.hh"
#include "unc/uppl_common.h"

using unc::upll::ipc_util::IpcUtil;

namespace unc {
namespace upll {
namespace kt_momgr {

BindInfo UnifiedNwMoMgr::unified_nw_bind_info[] = {
  { uudst::unified_nw::kDbiUnifiedNwName, CFG_KEY,
    offsetof(key_unified_nw, unified_nw_id), uud::kDalChar, 32 },
  { uudst::unified_nw::kDbiRoutingType, CFG_VAL,
    offsetof(val_unified_nw, routing_type), uud::kDalUint8, 1 },
  { uudst::unified_nw::kDbiIsDefault, CFG_VAL,
    offsetof(val_unified_nw, is_default), uud::kDalUint8, 1 },
  { uudst::unified_nw::kDbiValidRoutingType, CFG_META_VAL,
    offsetof(val_unified_nw, valid[0]), uud::kDalUint8, 1 },
  { uudst::unified_nw::kDbiValidIsDefault, CFG_META_VAL,
    offsetof(val_unified_nw, valid[1]), uud::kDalUint8, 1 },
  { uudst::unified_nw::kDbiCsRowStatus, CS_VAL,
    offsetof(val_unified_nw, cs_row_status), uud::kDalChar, 1 },
  { uudst::unified_nw::kDbiCsRoutingType, CS_VAL,
    offsetof(val_unified_nw, cs_attr[0]), uud::kDalChar, 1 },
  { uudst::unified_nw::kDbiCsIsDefault, CS_VAL,
    offsetof(val_unified_nw, cs_attr[1]), uud::kDalUint8, 1 }
};

BindInfo UnifiedNwMoMgr::unified_nw_maintbl_update_bind_info[] = {
  { uudst::unified_nw::kDbiUnifiedNwName, CFG_MATCH_KEY,
    offsetof(key_unified_nw, unified_nw_id),
    uud::kDalChar, kMaxLenUnifiedNwName + 1 }
};

unc_key_type_t UnifiedNwMoMgr::unified_nw_child[] = { UNC_KT_UNW_LABEL,
                                                      UNC_KT_UNW_LABEL_RANGE,
                                                      UNC_KT_UNW_SPINE_DOMAIN
};

UnifiedNwMoMgr::UnifiedNwMoMgr() : MoMgrImpl() {
  UPLL_FUNC_TRACE;
  ntable = MAX_MOMGR_TBLS;
  table = new Table *[ntable]();
  table[MAINTBL] = new Table(uudst::kDbiUnifiedNwTbl, UNC_KT_UNIFIED_NETWORK,
                             unified_nw_bind_info,
                             IpctSt::kIpcStKeyUnifiedNw,
                             IpctSt::kIpcStValUnifiedNw,
                             uudst::unified_nw::kDbiUnifiedNwNumCols);
  table[RENAMETBL] = NULL;
  table[CTRLRTBL] = NULL;
  table[CONVERTTBL] = NULL;
  nchild = sizeof(unified_nw_child) / sizeof(*unified_nw_child);
  child = unified_nw_child;
}

upll_rc_t UnifiedNwMoMgr::DupConfigKeyVal(ConfigKeyVal *&okey,
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
  if (req->get_key_type() != UNC_KT_UNIFIED_NETWORK) {
    UPLL_LOG_DEBUG("Input ConfigKeyVal keytype mismatch");
    return UPLL_RC_ERR_GENERIC;
  }
  ConfigVal *tmp1 = NULL, *tmp = (req)->get_cfg_val();
  if (tmp) {
    if (tbl == MAINTBL) {
      val_unified_nw *ival = reinterpret_cast<val_unified_nw *>(GetVal(req));
      if (ival == NULL) {
        UPLL_LOG_DEBUG("Null Val structure");
        return UPLL_RC_ERR_GENERIC;
      }
      val_unified_nw *unified_nw_val = ConfigKeyVal::Malloc<val_unified_nw>();
      memcpy(unified_nw_val, ival, sizeof(val_unified_nw));
      tmp1 = new ConfigVal(IpctSt::kIpcStValUnifiedNw, unified_nw_val);
    }
  };
  void *tkey = (req)->get_key();
  if (!tkey) {
    UPLL_LOG_DEBUG("Null tkey");
    DELETE_IF_NOT_NULL(tmp1);
    return UPLL_RC_ERR_GENERIC;
  }
  key_unified_nw *ikey =  reinterpret_cast<key_unified_nw *>(tkey);
  key_unified_nw *unified_nw_key = ConfigKeyVal::Malloc<key_unified_nw>();
  memcpy(unified_nw_key, ikey, sizeof(key_unified_nw));
  okey = new ConfigKeyVal(UNC_KT_UNIFIED_NETWORK, IpctSt::kIpcStKeyUnifiedNw,
                          unified_nw_key, tmp1);
  if (!okey) {
    delete tmp1;
    free(unified_nw_key);
    return UPLL_RC_ERR_GENERIC;
  }
  SET_USER_DATA(okey, req);
  return UPLL_RC_SUCCESS;
}

upll_rc_t UnifiedNwMoMgr::ValidateAttribute(ConfigKeyVal *ikey,
                                            DalDmlIntf *dmi,
                                            IpcReqRespHeader *req) {
  UPLL_FUNC_TRACE;
  if (ikey == NULL) return UPLL_RC_ERR_GENERIC;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *temp_ckv    = NULL;
  if (ikey->get_key_type() != UNC_KT_UNIFIED_NETWORK)
    return UPLL_RC_ERR_GENERIC;
  // Only one instance of KT_UNIFIED_NETWORK  is allowed
  if (req->operation == UNC_OP_CREATE) {
      result_code = GetChildConfigKey(temp_ckv, NULL);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_ERROR("Failed to get unified network ConfigKeyVal");
        return result_code;
      }
      uint32_t cur_instance_count = 0;
      result_code = GetInstanceCount(temp_ckv, NULL, req->datatype,
                                     &cur_instance_count, dmi, MAINTBL);
      delete temp_ckv;
      temp_ckv = NULL;
      if (UPLL_RC_SUCCESS != result_code) {
        UPLL_LOG_ERROR("GetInstanceCount failed %d", result_code);
        return result_code;
      }
      if (cur_instance_count > 0) {
        UPLL_LOG_INFO("One instance is allowed for KT_UNIFIED_NETWORK");
        return UPLL_RC_ERR_CFG_SEMANTIC;
      }
  }
  return result_code;
}

bool UnifiedNwMoMgr::IsValidKey(void *key,
                                uint64_t index, MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  key_unified_nw *unified_nw_key = reinterpret_cast<key_unified_nw *>(key);
  upll_rc_t ret_val = UPLL_RC_SUCCESS;
  switch (index) {
    case uudst::unified_nw::kDbiUnifiedNwName:
      ret_val = ValidateKey(reinterpret_cast<char *>(unified_nw_key->
                                                          unified_nw_id),
                        kMinLenUnifiedNwName, kMaxLenUnifiedNwName);
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

upll_rc_t UnifiedNwMoMgr::GetChildConfigKey(ConfigKeyVal *&okey,
                                            ConfigKeyVal *parent_key) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  key_unified_nw_t *unified_nw_key = NULL;

  if (okey && okey->get_key()) {
    unified_nw_key = reinterpret_cast<key_unified_nw_t *>(
                    okey->get_key());
  } else {
    unified_nw_key = ConfigKeyVal::Malloc<key_unified_nw_t>();
  }
  if (parent_key && parent_key->get_key()) {
    uuu::upll_strncpy(unified_nw_key->unified_nw_id,
                      reinterpret_cast<key_unified_nw *>(
                      parent_key->get_key())->unified_nw_id,
                      (kMaxLenUnifiedNwName+1));
  }
  // if okey is NULL, allocate the memory  and set unified_nw_key
  if (!okey)
    okey = new ConfigKeyVal(UNC_KT_UNIFIED_NETWORK, IpctSt::kIpcStKeyUnifiedNw,
                            unified_nw_key, NULL);
  else if (okey->get_key() != unified_nw_key)
    okey->SetKey(IpctSt::kIpcStKeyUnifiedNw, unified_nw_key);
  if (okey == NULL) {
    free(unified_nw_key);
    result_code = UPLL_RC_ERR_GENERIC;
  } else {
    SET_USER_DATA(okey, parent_key);
  }
  return result_code;
}

bool UnifiedNwMoMgr::CompareValidValue(void *&val1, void *val2,
                                       bool copy_to_running) {
  UPLL_FUNC_TRACE;
  // TODO(Author): Future release require changes
  return true;
}

upll_rc_t UnifiedNwMoMgr::IsReferenced(IpcReqRespHeader *req,
                                       ConfigKeyVal *ikey,
                                       DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *okey = NULL;
  uint32_t cur_instance_count = 0;
  if (NULL == ikey || !dmi)
    return UPLL_RC_ERR_GENERIC;
  // checking for  any vbr_portmap is configured
  MoMgrImpl *mgr = reinterpret_cast<MoMgrImpl *>(
      const_cast<MoManager*>(GetMoManager(UNC_KT_VBR_PORTMAP)));
  if (!mgr) {
    UPLL_LOG_ERROR("Unable to get vbr portmap instance");
    return UPLL_RC_ERR_GENERIC;
  }
  result_code = mgr->GetChildConfigKey(okey, NULL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetChildConfigKeyFailed %d", result_code);
    return result_code;
  }
  result_code = mgr->GetInstanceCount(okey, NULL, req->datatype,
                                  &cur_instance_count, dmi, MAINTBL);
  DELETE_IF_NOT_NULL(okey);
  if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("GetInstanceCount failed %d", result_code);
    return result_code;
  }
  if (cur_instance_count > 0) {
      UPLL_LOG_INFO("KT_VBR_PORTMAP is configured: can't delete unified_nw");
      return UPLL_RC_ERR_CFG_SEMANTIC;
  }
  // checking is unified_nw referred in vtn_unified
  MoMgrImpl *uvtn_mgr = reinterpret_cast<MoMgrImpl *>(
      const_cast<MoManager*>(GetMoManager(UNC_KT_VTN_UNIFIED)));
  if (!uvtn_mgr) {
    UPLL_LOG_ERROR("Unable to get vtn unified instance");
    return UPLL_RC_ERR_GENERIC;
  }
  result_code = uvtn_mgr->GetChildConfigKey(okey, NULL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetChildConfigKeyFailed %d", result_code);
    return result_code;
  }
  // fill the unified_nw id
  key_vtn_unified *vtn_unified_key = NULL;
  vtn_unified_key = reinterpret_cast <key_vtn_unified *> (okey->get_key());
  uuu::upll_strncpy(vtn_unified_key->unified_nw_id,
                    (reinterpret_cast<key_unified_nw*>(
                    ikey->get_key()))->unified_nw_id,
                    (kMaxLenUnifiedNwName+1));
  DbSubOp dbop = {kOpReadExist, kOpMatchNone, kOpInOutNone};
  result_code = uvtn_mgr->UpdateConfigDB(okey, req->datatype, UNC_OP_READ,
                                        dmi, &dbop);
  DELETE_IF_NOT_NULL(okey);
    if (UPLL_RC_ERR_INSTANCE_EXISTS == result_code) {
    UPLL_LOG_DEBUG("Could not delete an unified nw which is referred in"
                    " vtn_unified");
    return UPLL_RC_ERR_CFG_SEMANTIC;
  } else if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    result_code = UPLL_RC_SUCCESS;
  } else {
    UPLL_LOG_DEBUG("UpdateConfigDB failed %d", result_code);
    return result_code;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t UnifiedNwMoMgr::AllocVal(ConfigVal *&ck_val,
                                   upll_keytype_datatype_t dt_type,
                                   MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  void *val;  //  *ck_nxtval;
  if (ck_val != NULL) return UPLL_RC_ERR_GENERIC;
  switch (tbl) {
    case MAINTBL:
      val = ConfigKeyVal::Malloc(sizeof(val_unified_nw));
      ck_val = new ConfigVal(IpctSt::kIpcStValUnifiedNw, val);
      break;
    default:
      val = NULL;
      break;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t UnifiedNwMoMgr::ValidateUnifiedNwKey(key_unified_nw *unified_nw_key) {
  UPLL_FUNC_TRACE;
  upll_rc_t ret_val = UPLL_RC_SUCCESS;
  ret_val = ValidateKey(reinterpret_cast<char *>(unified_nw_key->unified_nw_id),
                         kMinLenUnifiedNwName, kMaxLenUnifiedNwName);
  if (ret_val != UPLL_RC_SUCCESS) {
    UPLL_LOG_INFO("Unified network name syntax check failed."
                  "Received Unified network name - %s",
                  unified_nw_key->unified_nw_id);
    return UPLL_RC_ERR_CFG_SYNTAX;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t UnifiedNwMoMgr::ValidateMessage(IpcReqRespHeader * req,
                                          ConfigKeyVal * ikey) {
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
  if (ikey->get_st_num() != IpctSt::kIpcStKeyUnifiedNw) {
    UPLL_LOG_ERROR("Invalid key structure received. received struct - %d",
                  (ikey->get_st_num()));
    return UPLL_RC_ERR_BAD_REQUEST;
  }
  if (ikey->get_key_type() != UNC_KT_UNIFIED_NETWORK) {
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
  key_unified_nw *unified_nw_key = reinterpret_cast<key_unified_nw *> (
      ikey->get_key());

  if ((req->operation != UNC_OP_READ_SIBLING_COUNT) &&
      (req->operation != UNC_OP_READ_SIBLING_BEGIN)) {
    ret_val = ValidateUnifiedNwKey(unified_nw_key);
    if (ret_val != UPLL_RC_SUCCESS) {
      UPLL_LOG_INFO("syntax check failed for key_unified_nw struct");
      return UPLL_RC_ERR_CFG_SYNTAX;
      }
  } else {
    UPLL_LOG_TRACE("Operation is %d", req->operation);
    StringReset(unified_nw_key->unified_nw_id);
    }
  // Validate value structure and opertaion  type
  ConfigVal *cfg_val = ikey->get_cfg_val();
  val_unified_nw *unified_nw_val = NULL;
  switch (req->operation) {
    case UNC_OP_CREATE:
    case UNC_OP_UPDATE:   // update is not supported for unified network
      if (req->operation == UNC_OP_UPDATE) {
        UPLL_LOG_ERROR("Update operation is not allowed for unified ntework");
        return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_KT;
      }
      if (!cfg_val) {
        UPLL_LOG_ERROR("Configval structure is NULL");
        return UPLL_RC_ERR_BAD_REQUEST;
      }
      if (cfg_val->get_st_num() != IpctSt::kIpcStValUnifiedNw) {
        UPLL_LOG_ERROR("Invalid val struct st num-%d",
                       (ikey->get_cfg_val())->get_st_num());
        return UPLL_RC_ERR_BAD_REQUEST;
      }
      unified_nw_val =
          reinterpret_cast<val_unified_nw*>(ikey->get_cfg_val()->get_val());
      if (unified_nw_val == NULL) {
        UPLL_LOG_ERROR("val structure is NULL for CREATE");
        return UPLL_RC_ERR_BAD_REQUEST;
      }
  // TODO(Author): Future release, need to support other routing type
  // TODO(Author): U17, include no routing-type check
      if (unified_nw_val->valid[UPLL_IDX_ROUTING_TYPE_UNW] == UNC_VF_VALID) {
        if (unified_nw_val->routing_type != UPLL_ROUTING_TYPE_QINQ_TO_QINQ) {
           UPLL_LOG_ERROR("Routing Type (%d) is not QinQ domain",
                       unified_nw_val->routing_type);
          return UPLL_RC_ERR_BAD_REQUEST;
        }
      } else if ((unified_nw_val->valid[UPLL_IDX_ROUTING_TYPE_UNW] ==
                  UNC_VF_INVALID) ||
                 (unified_nw_val->valid[UPLL_IDX_ROUTING_TYPE_UNW] ==
                  UNC_VF_VALID_NO_VALUE)) {
        UPLL_LOG_ERROR("Routing Type is not configured");
        return UPLL_RC_ERR_BAD_REQUEST;
        }
      unified_nw_val->is_default = true;  // setting to true
      unified_nw_val->valid[UPLL_IDX_IS_DEFAULT_UNW] = UNC_VF_VALID;
      return UPLL_RC_SUCCESS;
      break;
    case UNC_OP_DELETE:
      UPLL_LOG_TRACE("Value structure is none for operation type:%d",
                      req->operation);
      return UPLL_RC_SUCCESS;
      break;
    case UNC_OP_READ:
    case UNC_OP_READ_SIBLING:
    case UNC_OP_READ_SIBLING_BEGIN:
    case UNC_OP_READ_SIBLING_COUNT:
      if (GetVal(ikey)) {
        /** TODO(Author): future release need to support other routing type */
       //  NULL check is required for below ikey->get_cfg_val()->get_val();
        val_unified_nw *unw_val = reinterpret_cast<val_unified_nw*>
                                  (ikey->get_cfg_val()->get_val());
        if ((unw_val-> valid[UPLL_IDX_ROUTING_TYPE_UNW] == UNC_VF_VALID) &&
            (unw_val->routing_type != UPLL_ROUTING_TYPE_QINQ_TO_QINQ)) {
          UPLL_LOG_ERROR("Routing Type is not QinQ domain");
          return UPLL_RC_ERR_BAD_REQUEST;
        }
      }
    return UPLL_RC_SUCCESS;
    break;
    default:
      UPLL_LOG_ERROR("Invalid opertaion type (%d) received", req->operation);
      return UPLL_RC_ERR_BAD_REQUEST;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t UnifiedNwMoMgr::UpdateConfigStatus(ConfigKeyVal *ikey,
                             unc_keytype_operation_t op,
                             uint32_t driver_result,
                             ConfigKeyVal *upd_key,
                             DalDmlIntf *dmi,
                             ConfigKeyVal *ctrlr_key) {
  UPLL_FUNC_TRACE;
  val_unified_nw *unified_nw_val =
      reinterpret_cast<val_unified_nw *>(GetVal(ikey));
  if (unified_nw_val == NULL) {
    return UPLL_RC_ERR_GENERIC;
  }
  unified_nw_val->cs_row_status = UNC_CS_APPLIED;
  if (unified_nw_val->valid[UPLL_IDX_ROUTING_TYPE_UNW] == UNC_VF_VALID)
  unified_nw_val->cs_attr[UPLL_IDX_ROUTING_TYPE_UNW] = UNC_CS_APPLIED;
  if (unified_nw_val->valid[UPLL_IDX_IS_DEFAULT_UNW] == UNC_VF_VALID)
  unified_nw_val->cs_attr[UPLL_IDX_IS_DEFAULT_UNW] = UNC_CS_APPLIED;
  return UPLL_RC_SUCCESS;
}

upll_rc_t UnifiedNwMoMgr::GetOperation(uuc::UpdateCtrlrPhase phase,
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

}  // namespace kt_momgr
}  // namespace upll
}  // namespace unc
