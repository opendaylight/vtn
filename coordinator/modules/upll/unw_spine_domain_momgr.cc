/*
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include "uncxx/upll_log.hh"
#include "unw_spine_domain_momgr.hh"
#include "unw_label_momgr.hh"
#include "upll_validation.hh"
#include "unc/uppl_common.h"
#include "domain_check_util.hh"

using unc::upll::ipc_util::IpcUtil;

namespace unc {
namespace upll {
namespace kt_momgr {

BindInfo UNWSpineDomainMoMgr::unw_spine_domain_bind_info[] = {
  { uudst::unw_spine_domain::kDbiUnifiedNwName, CFG_KEY, offsetof(
          key_unw_spine_domain, unw_key.unified_nw_id),
    uud::kDalChar, 32 },
  { uudst::unw_spine_domain::kDbiUnwSpineDomainName, CFG_KEY, offsetof(
          key_unw_spine_domain, unw_spine_id),
    uud::kDalChar, 32 },
  { uudst::unw_spine_domain::kDbiCtrlrName, CFG_VAL, offsetof(
          val_unw_spdom_ext, val_unw_spine_dom.spine_controller_id),
    uud::kDalChar, 32 },
  { uudst::unw_spine_domain::kDbiDomainId, CFG_VAL, offsetof(
          val_unw_spdom_ext, val_unw_spine_dom.spine_domain_id),
    uud::kDalChar, 32 },
  { uudst::unw_spine_domain::kDbiUnwLabelName, CFG_VAL, offsetof(
          val_unw_spdom_ext, val_unw_spine_dom.unw_label_id),
    uud::kDalChar, 32 },
  { uudst::unw_spine_domain::kDbiUsedLabelCount, CFG_VAL, offsetof(
          val_unw_spdom_ext_t, used_label_count),
    uud::kDalUint32, 1 },
  { uudst::unw_spine_domain::kDbiAlarmRaised, ST_VAL, offsetof(
          val_spdom_st, alarm_status),
    uud::kDalUint8, 1 },
  { uudst::unw_spine_domain::kDbiValidCtrlrName, CFG_META_VAL, offsetof(
          val_unw_spdom_ext,
          val_unw_spine_dom.valid[UPLL_IDX_SPINE_CONTROLLER_ID_UNWS]),
    uud::kDalUint8, 1 },
  { uudst::unw_spine_domain::kDbiValidDomainId, CFG_META_VAL, offsetof(
          val_unw_spdom_ext,
          val_unw_spine_dom.valid[UPLL_IDX_SPINE_DOMAIN_ID_UNWS]),
    uud::kDalUint8, 1 },
  { uudst::unw_spine_domain::kDbiValidUnwLabelName, CFG_META_VAL,
    offsetof(
          val_unw_spdom_ext,
          val_unw_spine_dom.valid[UPLL_IDX_UNW_LABEL_ID_UNWS]),
    uud::kDalUint8, 1 },
  { uudst::unw_spine_domain::kDbiValidUsedLabelCount, CFG_META_VAL,
    offsetof(val_unw_spdom_ext_t, valid[UPLL_IDX_SPINE_USED_LABEL_COUNT_UNWS]),
    uud::kDalUint8, 1 },
  { uudst::unw_spine_domain::kDbiValidAlarmRaised, ST_META_VAL,
    offsetof(val_spdom_st, valid[UPLL_IDX_SPINE_ALARAM_RAISED_UNWS]),
    uud::kDalUint8, 1 },
  { uudst::unw_spine_domain::kDbiCsRowStatus, CS_VAL, offsetof(
          val_unw_spdom_ext_t,  val_unw_spine_dom.cs_row_status),
    uud::kDalUint8, 1 },
  { uudst::unw_spine_domain::kDbiCsCtrlrName, CS_VAL, offsetof(
          val_unw_spdom_ext_t,  val_unw_spine_dom.cs_attr[0]),
    uud::kDalUint8, 1 },
  { uudst::unw_spine_domain::kDbiCsDomainId, CS_VAL, offsetof(
          val_unw_spdom_ext_t,  val_unw_spine_dom.cs_attr[1]),
    uud::kDalUint8, 1 },
  { uudst::unw_spine_domain::kDbiCsUnwLabelName, CS_VAL, offsetof(
          val_unw_spdom_ext_t,  val_unw_spine_dom.cs_attr[2]),
    uud::kDalUint8, 1 }
};

UNWSpineDomainMoMgr::UNWSpineDomainMoMgr() : MoMgrImpl() {
  UPLL_FUNC_TRACE;
  ntable = MAX_MOMGR_TBLS;
  table = new Table *[ntable]();
  table[MAINTBL] = new Table(uudst:: kDbiUnwSpineDomainTbl,
                UNC_KT_UNW_SPINE_DOMAIN, unw_spine_domain_bind_info,
      IpctSt::kIpcStKeyUnwSpineDomain, IpctSt::kIpcStValUnwSpineDomain,
      uudst::unw_spine_domain::kDbiUnwSpineNumCols);
  table[RENAMETBL] = NULL;
  table[CTRLRTBL] = NULL;
  table[CONVERTTBL] = NULL;
  nchild = 0;
  child  = NULL;
  threshold_alarm_processing_required_ = false;
}

upll_rc_t UNWSpineDomainMoMgr::ValidateAttribute(
    ConfigKeyVal *ikey, DalDmlIntf *dmi, IpcReqRespHeader *req) {
  UPLL_FUNC_TRACE;
  if (ikey == NULL) return UPLL_RC_ERR_GENERIC;

  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *temp_ckv    = NULL;

  if (ikey->get_key_type() != UNC_KT_UNW_SPINE_DOMAIN)
    return UPLL_RC_ERR_GENERIC;

  val_unw_spine_domain *unified_spine_dom_val = reinterpret_cast<
      val_unw_spine_domain*>(ikey->get_cfg_val()->get_val());

  key_unw_spine_domain *unw_spinedomain_key  =
      reinterpret_cast<key_unw_spine_domain*>(ikey->get_key());

  if (req->operation == UNC_OP_CREATE || req->operation == UNC_OP_UPDATE) {
    val_unw_spine_domain *spine_domain_val =
         reinterpret_cast<val_unw_spine_domain *>(GetVal(ikey));
    if (spine_domain_val != NULL) {
      unc_keytype_ctrtype_t ctrlrtype = UNC_CT_UNKNOWN;
      uuc::CtrlrMgr *ctrlr_mgr = uuc::CtrlrMgr::GetInstance();
      if (spine_domain_val->valid[UPLL_IDX_SPINE_CONTROLLER_ID_UNWS] ==
          UNC_VF_VALID) {
        if ((!ctrlr_mgr->GetCtrlrType(
               reinterpret_cast<char *>(spine_domain_val->spine_controller_id),
               req->datatype, &ctrlrtype)) || (ctrlrtype != UNC_CT_PFC)) {
           UPLL_LOG_ERROR("Controller ID does not exists/not PFC in candidate");
           return UPLL_RC_ERR_CFG_SEMANTIC;
        }
        TcConfigMode config_mode = TC_CONFIG_INVALID;
        std::string vtn_name;
        result_code = GetConfigModeInfo(req, config_mode, vtn_name);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("GetConfigMode failed");
          return result_code;
        }
        if (config_mode == TC_CONFIG_VIRTUAL) {
          if ((!ctrlr_mgr->GetCtrlrType(
               reinterpret_cast<char *>(spine_domain_val->spine_controller_id),
               UPLL_DT_RUNNING, &ctrlrtype)) || (ctrlrtype != UNC_CT_PFC)) {
             UPLL_LOG_ERROR("Controller ID does not exists/not PFC in running");
             return UPLL_RC_ERR_CFG_SEMANTIC;
          }
        }
      }
      // one instance is allowed per controller and domain
      result_code = GetChildConfigKey(temp_ckv, ikey);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_ERROR("Failed to create ConfigKey, err %d", result_code);
        return result_code;
      }
      DbSubOp dbop = {kOpReadExist, kOpMatchNone, kOpInOutNone};
      result_code = UpdateConfigDB(ikey, req->datatype, UNC_OP_READ, dmi,
                               &dbop, MAINTBL);
      if ((result_code != UPLL_RC_ERR_INSTANCE_EXISTS) &&
          (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE)) {
        DELETE_IF_NOT_NULL(temp_ckv);
        UPLL_LOG_INFO("Failed to read from DB err -%d", result_code);
        return result_code;
      } else if (result_code == UPLL_RC_ERR_INSTANCE_EXISTS) {
         if (req->operation == UNC_OP_CREATE) {
           DELETE_IF_NOT_NULL(temp_ckv);
           return result_code;
         }
         result_code = UPLL_RC_SUCCESS;
      } else if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
          if (req->operation == UNC_OP_UPDATE) {
            DELETE_IF_NOT_NULL(temp_ckv);
            UPLL_LOG_INFO("record doesn't exists in DB, err %d", result_code);
            return result_code;
          }
         result_code = UPLL_RC_SUCCESS;
      }
      key_unw_spine_domain *spine_key  =
      reinterpret_cast<key_unw_spine_domain*>(temp_ckv->get_key());
      StringReset(spine_key->unw_spine_id);
      val_unw_spdom_ext *spine_domain_dbval =
                  reinterpret_cast<val_unw_spdom_ext *>
                 (ConfigKeyVal::Malloc(sizeof(val_unw_spdom_ext)));
      memcpy(&(spine_domain_dbval->val_unw_spine_dom), spine_domain_val,
           sizeof(val_unw_spine_domain_t));
      // Only spine controller and domain id should be taken for existance check
      spine_domain_dbval->val_unw_spine_dom.valid[UPLL_IDX_UNW_LABEL_ID_UNWS] =
                                                  UNC_VF_INVALID;
      spine_domain_dbval->valid[UPLL_IDX_SPINE_DOMAIN_VAL] = UNC_VF_VALID;
      temp_ckv->AppendCfgVal(IpctSt::kIpctStValUnwSpineDomain_Ext,
                         spine_domain_dbval);
    }

    DbSubOp dbop = {kOpReadSingle, kOpMatchNone, kOpInOutNone};
    result_code = ReadConfigDB(temp_ckv, req->datatype, UNC_OP_READ, dbop,
                               dmi, MAINTBL);
    DELETE_IF_NOT_NULL(temp_ckv);
    if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
       if (req->operation == UNC_OP_UPDATE) {
         UPLL_LOG_INFO("Spine Controller and Domain ID cannot be updated");
         return UPLL_RC_ERR_CFG_SEMANTIC;
       } else {
         result_code = UPLL_RC_SUCCESS;
       }
    } else if (result_code == UPLL_RC_SUCCESS) {
      if (req->operation == UNC_OP_CREATE) {
        UPLL_LOG_INFO("Only one spine domain instance is allowed per"
                      "controller and domain");
        return UPLL_RC_ERR_CFG_SEMANTIC;
      }
    } else {
        UPLL_LOG_INFO("Failed to read from DB,err %d", result_code);
        return result_code;
    }
    // do the below label existance check for CREATE and UPDATE operation
    if (unified_spine_dom_val->valid[UPLL_IDX_UNW_LABEL_ID_UNWS] ==
      UNC_VF_VALID) {
      ConfigKeyVal *temp_unw_label_ckv = NULL;

      UNWLabelMoMgr* unw_label_mgr  =  reinterpret_cast<
        UNWLabelMoMgr *>(const_cast<MoManager *>(
                GetMoManager(UNC_KT_UNW_LABEL)));

      result_code = unw_label_mgr->GetChildConfigKey(temp_unw_label_ckv,
                                                   NULL);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_ERROR("Failed to create unw label ConfigKeyVal");
        return result_code;
      }
      key_unw_label *unw_label_key = NULL;
      unw_label_key = reinterpret_cast<key_unw_label *>(
          temp_unw_label_ckv->get_key());

      uuu::upll_strncpy(unw_label_key->unified_nw_key.unified_nw_id,
                      unw_spinedomain_key->unw_key.unified_nw_id,
                      (kMaxLenUnifiedNwName+1));
      uuu::upll_strncpy(unw_label_key->unw_label_id,
                      unified_spine_dom_val->unw_label_id,
                      (kMaxLenUnwLabelName+1));

      DbSubOp dbop = { kOpReadExist, kOpMatchNone, kOpInOutNone };
      result_code = unw_label_mgr->UpdateConfigDB(temp_unw_label_ckv,
                                                req->datatype, UNC_OP_READ, dmi,
                                                &dbop, MAINTBL);

      DELETE_IF_NOT_NULL(temp_unw_label_ckv);
      if (result_code != UPLL_RC_ERR_INSTANCE_EXISTS) {
        UPLL_LOG_DEBUG("Given label doesn't exist in label tbl (%d)",
                        result_code);
        return UPLL_RC_ERR_CFG_SEMANTIC;
      }
      result_code = UPLL_RC_SUCCESS;
    }
}
  return result_code;
}

bool UNWSpineDomainMoMgr::IsValidKey(void *key,
                          uint64_t index, MoMgrTables tbl) {
  UPLL_FUNC_TRACE;

  upll_rc_t ret_val = UPLL_RC_SUCCESS;

  key_unw_spine_domain *unw_spinedomain_key = reinterpret_cast<
      key_unw_spine_domain *>(key);

  switch (index) {
    case uudst::unw_spine_domain::kDbiUnifiedNwName:
      ret_val = ValidateKey(reinterpret_cast<char *>(
              unw_spinedomain_key->unw_key.unified_nw_id),
          kMinLenUnifiedNwName, kMaxLenUnifiedNwName);
      break;
    case uudst::unw_spine_domain::kDbiUnwSpineDomainName:
      ret_val = ValidateKey(reinterpret_cast<char *>(
              unw_spinedomain_key->unw_spine_id),
                        kMinLenUnwSpineID, kMaxLenUnwSpineID);
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

upll_rc_t UNWSpineDomainMoMgr::GetChildConfigKey(ConfigKeyVal *&okey,
                                      ConfigKeyVal *parent_key) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  key_unw_spine_domain_t *unw_spinedomain_key = NULL;

  if (okey && okey->get_key()) {
    if (okey->get_key_type() != UNC_KT_UNW_SPINE_DOMAIN) {
      return UPLL_RC_ERR_GENERIC;
    }
    unw_spinedomain_key = reinterpret_cast<key_unw_spine_domain_t *>(
                    okey->get_key());
  } else {
    unw_spinedomain_key = reinterpret_cast<key_unw_spine_domain_t *>(
      ConfigKeyVal::Malloc(sizeof(key_unw_spine_domain)));
  }
  void *pkey;
  if (parent_key == NULL) {
    if (!okey)
      okey = new ConfigKeyVal(UNC_KT_UNW_SPINE_DOMAIN,
                              IpctSt::kIpcStKeyUnwSpineDomain,
                              unw_spinedomain_key, NULL);
    else if (okey->get_key() != unw_spinedomain_key)
      okey->SetKey(IpctSt::kIpcStKeyUnwSpineDomain, unw_spinedomain_key);
    return UPLL_RC_SUCCESS;
  } else {
    pkey = parent_key->get_key();
  }
  if (!pkey) {
    if (!okey || !(okey->get_key()))
      FREE_IF_NOT_NULL(unw_spinedomain_key);
    return UPLL_RC_ERR_GENERIC;
  }
  unc_key_type_t keytype = parent_key->get_key_type();
  switch (keytype) {
    case UNC_KT_UNIFIED_NETWORK:
      uuu::upll_strncpy(unw_spinedomain_key->unw_key.unified_nw_id,
           reinterpret_cast<key_unified_nw_t *>(
           parent_key->get_key())->unified_nw_id,
           (kMaxLenUnifiedNwName+1));
      break;
    case UNC_KT_UNW_SPINE_DOMAIN:
      //  copy unified_nw_spineid from parent_key to unw_spinedomain_key
      uuu::upll_strncpy(unw_spinedomain_key->unw_key.unified_nw_id,
           reinterpret_cast<key_unw_spine_domain *>(
           parent_key->get_key())->unw_key.unified_nw_id,
           (kMaxLenUnifiedNwName+1));
      uuu::upll_strncpy(unw_spinedomain_key->unw_spine_id,
           reinterpret_cast<key_unw_spine_domain *>(
           parent_key->get_key())->unw_spine_id,
           (kMaxLenUnwSpineID+1));
      break;
    default:
      if (!okey || !(okey->get_key())) {
        FREE_IF_NOT_NULL(unw_spinedomain_key);
        return UPLL_RC_ERR_GENERIC;
      }
  }
  // if okey is NULL, allocate the memory  and set unw_spinedomain_key
  if (!okey) {
    okey = new ConfigKeyVal(UNC_KT_UNW_SPINE_DOMAIN,
                            IpctSt::kIpcStKeyUnwSpineDomain,
                            unw_spinedomain_key, NULL);

  } else if (okey->get_key() != unw_spinedomain_key) {
    okey->SetKey(IpctSt::kIpcStKeyUnwSpineDomain, unw_spinedomain_key);
  }
  SET_USER_DATA(okey, parent_key);
  return result_code;
}

bool UNWSpineDomainMoMgr::CompareValidValue(void *&val1, void *val2,
                                 bool copy_to_running) {
  UPLL_FUNC_TRACE;
  bool invalid_attr = true;

  val_unw_spdom_ext *val1_spdom = reinterpret_cast<
      val_unw_spdom_ext*>(val1);
  val_unw_spdom_ext *val2_spdom = reinterpret_cast<
      val_unw_spdom_ext*>(val2);
  for ( unsigned int loop = 0;
          loop < sizeof(val1_spdom->val_unw_spine_dom.valid)/sizeof(
                                  val1_spdom->val_unw_spine_dom.valid[0]);
                        ++loop ) {
    if ( UNC_VF_INVALID == val1_spdom->val_unw_spine_dom.valid[loop] &&
             UNC_VF_VALID == val2_spdom->val_unw_spine_dom.valid[loop])
        val1_spdom->val_unw_spine_dom.valid[loop] = UNC_VF_VALID_NO_VALUE;
  }

  if (UNC_VF_VALID == val1_spdom->val_unw_spine_dom.valid[
             UPLL_IDX_SPINE_CONTROLLER_ID_UNWS] && UNC_VF_VALID == val2_spdom->
             val_unw_spine_dom.valid[UPLL_IDX_SPINE_CONTROLLER_ID_UNWS]) {
    if (!strcmp(reinterpret_cast<char*>
                (val1_spdom->val_unw_spine_dom.spine_controller_id),
                reinterpret_cast<char*>
                (val2_spdom->val_unw_spine_dom.spine_controller_id)))
      val1_spdom->val_unw_spine_dom.valid[UPLL_IDX_SPINE_CONTROLLER_ID_UNWS] =
      UNC_VF_INVALID;
  }
  if (UNC_VF_VALID == val1_spdom->val_unw_spine_dom.valid[
             UPLL_IDX_SPINE_DOMAIN_ID_UNWS] && UNC_VF_VALID == val2_spdom->
             val_unw_spine_dom.valid[UPLL_IDX_SPINE_DOMAIN_ID_UNWS]) {
    if (!strcmp(reinterpret_cast<char*>
                (val1_spdom->val_unw_spine_dom.spine_domain_id),
                reinterpret_cast<char*>
                (val2_spdom->val_unw_spine_dom.spine_domain_id)))
      val1_spdom->val_unw_spine_dom.valid[UPLL_IDX_SPINE_DOMAIN_ID_UNWS] =
      UNC_VF_INVALID;
  }
  if (UNC_VF_VALID == val1_spdom->val_unw_spine_dom.valid[
             UPLL_IDX_UNW_LABEL_ID_UNWS] && UNC_VF_VALID == val2_spdom->
             val_unw_spine_dom.valid[UPLL_IDX_UNW_LABEL_ID_UNWS]) {
    if (!strcmp(reinterpret_cast<char*>
                (val1_spdom->val_unw_spine_dom.unw_label_id),
                reinterpret_cast<char*>
                (val2_spdom->val_unw_spine_dom.unw_label_id)))
      val1_spdom->val_unw_spine_dom.valid[UPLL_IDX_UNW_LABEL_ID_UNWS] =
      UNC_VF_INVALID;
  }
  if (UNC_VF_VALID == val1_spdom->valid[
           UPLL_IDX_SPINE_USED_LABEL_COUNT_UNWS] && UNC_VF_VALID == val2_spdom->
           valid[UPLL_IDX_SPINE_USED_LABEL_COUNT_UNWS]) {
    if (val1_spdom->used_label_count ==  val2_spdom->used_label_count)
      val1_spdom->valid[UPLL_IDX_SPINE_USED_LABEL_COUNT_UNWS] = UNC_VF_INVALID;
  }
  for ( unsigned int loop = 0;
          loop < sizeof(val1_spdom->val_unw_spine_dom.valid)/sizeof(
                                  val1_spdom->val_unw_spine_dom.valid[0]);
                        ++loop ) {
    if ((UNC_VF_VALID == val1_spdom->val_unw_spine_dom.valid[loop])
       ||(UNC_VF_VALID_NO_VALUE == val1_spdom->val_unw_spine_dom.valid[
          loop])) {
      invalid_attr = false;
      break;
    }
  }
  return invalid_attr;
}

upll_rc_t UNWSpineDomainMoMgr::IsReferenced(IpcReqRespHeader *req,
                                            ConfigKeyVal *ikey,
                                            DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  //  Check if If SpineDomain is referred in the KT VTN UNIFIED
  //  return semantic error if it is referred.
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  MoMgrImpl *vtn_unified_mgr =
    reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>(GetMoManager(
            UNC_KT_VTN_UNIFIED)));

  if (vtn_unified_mgr == NULL) {
    UPLL_LOG_INFO("Failed to get instance to vtnunified momgr");
    return UPLL_RC_ERR_GENERIC;
  }
  key_unw_spine_domain_t *unw_spinedomain_key =
                           reinterpret_cast<key_unw_spine_domain_t *>(
                           ikey->get_key());
  ConfigKeyVal *vtn_unified_ckv = NULL;
  result_code = vtn_unified_mgr->GetChildConfigKey(vtn_unified_ckv, NULL);
  key_vtn_unified_t *key_vtn_unw =  reinterpret_cast<key_vtn_unified_t *>(
                                vtn_unified_ckv->get_key());

  val_vtn_unified *val_vtn_unw =
        ConfigKeyVal::Malloc<val_vtn_unified_t>();
  vtn_unified_ckv->AppendCfgVal(IpctSt::kIpcStValVtnUnified, val_vtn_unw);
  val_vtn_unw = reinterpret_cast<val_vtn_unified *>
                           (GetVal(vtn_unified_ckv));
  uuu::upll_strncpy(key_vtn_unw->unified_nw_id,
                    unw_spinedomain_key->unw_key.unified_nw_id,
                    (kMaxLenUnifiedNwName+1));
  uuu::upll_strncpy(val_vtn_unw->spine_id,
                    unw_spinedomain_key->unw_spine_id,
                    (kMaxLenUnwSpineID+1));
  val_vtn_unw->valid[UPLL_IDX_SPINE_ID_VUNW] = UNC_VF_VALID;

  // DbSubOp dbop_vtnunw = {kOpReadExist, kOpMatchNone, kOpInOutNone};
  DbSubOp dbop_vtnunw  = {kOpReadSingle, kOpMatchNone, kOpInOutNone};
  result_code = vtn_unified_mgr->ReadConfigDB(vtn_unified_ckv, req->datatype,
                                UNC_OP_READ, dbop_vtnunw, dmi, MAINTBL);
  /*
  result_code = vtn_unified_mgr->UpdateConfigDB(vtn_unified_ckv,
                                                req->datatype,
                                                UNC_OP_READ, dmi,
                                                &dbop_vtnunw, MAINTBL);
  */
  DELETE_IF_NOT_NULL(vtn_unified_ckv);
  if (result_code == UPLL_RC_SUCCESS) {
    UPLL_LOG_INFO("Cannot delete Spine-Domain as it is referred by KT %d",
                  UNC_KT_VTN_UNIFIED);
    // DELETE_IF_NOT_NULL(vtn_unified_ckv);
    return UPLL_RC_ERR_CFG_SEMANTIC;
  } else if (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    // DELETE_IF_NOT_NULL(vtn_unified_ckv);
    return result_code;
  } else {
    result_code = UPLL_RC_SUCCESS;
  }

  //  If vtunnel is already created with specified spine domain
  // Read the Val Structure from DB since it will not be available during
  // DELETE OP
  ConfigKeyVal *CKSpineDomain = NULL;
  result_code = GetChildConfigKey(CKSpineDomain, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_INFO("Failed to construct configkey err %d", result_code);
    DELETE_IF_NOT_NULL(vtn_unified_ckv);
    return result_code;
  }

  DbSubOp dbop = {kOpReadSingle, kOpMatchNone, kOpInOutNone};
  result_code = ReadConfigDB(CKSpineDomain, UPLL_DT_CANDIDATE,
                             UNC_OP_READ, dbop,
                             dmi, MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    DELETE_IF_NOT_NULL(CKSpineDomain);
    if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE)
      result_code = UPLL_RC_SUCCESS;
    else
      UPLL_LOG_INFO("Unable to read from DB, err %d", result_code);
    return result_code;
  }
  val_unw_spdom_ext *dbval =
         reinterpret_cast<val_unw_spdom_ext *>(GetVal(CKSpineDomain));
  MoMgrImpl *vtunnel_mgr = reinterpret_cast<MoMgrImpl *>
                            (const_cast<MoManager *>(GetMoManager(
                             UNC_KT_VTUNNEL)));

  if ( vtunnel_mgr == NULL ) {
    UPLL_LOG_INFO("Failed to get instance of  vtunner momgr");
    DELETE_IF_NOT_NULL(CKSpineDomain);
    return UPLL_RC_ERR_GENERIC;
  }
  ConfigKeyVal *vtunnel_ckv = NULL;
  result_code = vtunnel_mgr->GetChildConvertConfigKey(vtunnel_ckv, NULL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_INFO("Failed to construct configkey err %d", result_code);
    DELETE_IF_NOT_NULL(CKSpineDomain);
    return result_code;
  }
  val_convert_vtunnel *vtunnel_val =
        ConfigKeyVal::Malloc<val_convert_vtunnel>();
  vtunnel_ckv->AppendCfgVal(IpctSt::kIpcStValConvertVtunnel, vtunnel_val);

  SET_USER_DATA_CTRLR(vtunnel_ckv,
                      dbval->val_unw_spine_dom.spine_controller_id);
  SET_USER_DATA_DOMAIN(vtunnel_ckv,
                      dbval->val_unw_spine_dom.spine_domain_id);

  // DELETE_IF_NOT_NULL(CKSpineDomain);
  // Read the record from vtunnel table based on the partial key constructed
  DbSubOp dbop_vtunnel = {kOpNotRead, kOpMatchCtrlr|kOpMatchDomain,
                          kOpInOutNone};
  result_code = vtunnel_mgr->UpdateConfigDB(vtunnel_ckv, req->datatype,
                                            UNC_OP_READ, dmi,
                                            &dbop_vtunnel, CONVERTTBL);

  DELETE_IF_NOT_NULL(vtunnel_ckv);
  DELETE_IF_NOT_NULL(CKSpineDomain);
  if (result_code == UPLL_RC_ERR_INSTANCE_EXISTS) {
    UPLL_LOG_INFO("SpineDomain cannot be deleted");
    return UPLL_RC_ERR_CFG_SEMANTIC;
  } else if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    result_code = UPLL_RC_SUCCESS;
  } else {
    UPLL_LOG_INFO("Err while reading from DB ,err %d", result_code);
  }
  #if 0
  if (result_code == UPLL_RC_SUCCESS) {
    // Get the mode information and if the mode is virtual then do the following
    // existsnace check in the running configuration
    //  1) spine domain is referred by vtn_unified
    //  2) spine domain is referred by vtunnel
    TcConfigMode config_mode = TC_CONFIG_INVALID;
    std::string vtn_name;
    result_code = GetConfigModeInfo(req, config_mode, vtn_name);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("GetConfigMode failed");
      DELETE_IF_NOT_NULL(vtn_unified_ckv);
      DELETE_IF_NOT_NULL(vtunnel_ckv);
      return result_code;
     }
     if (config_mode == TC_CONFIG_VIRTUAL) {
       result_code = vtn_unified_mgr->UpdateConfigDB(vtn_unified_ckv,
                                                     UPLL_DT_RUNNING,
                                                     UNC_OP_READ, dmi);
       DELETE_IF_NOT_NULL(vtn_unified_ckv);
       if (result_code == UPLL_RC_ERR_INSTANCE_EXISTS) {
         UPLL_LOG_INFO("Cannot delete Spine-Domain as it is referred by KT %d",
                  UNC_KT_VTN_UNIFIED);
         DELETE_IF_NOT_NULL(vtunnel_ckv);
         return UPLL_RC_ERR_CFG_SEMANTIC;
       } else if (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
         DELETE_IF_NOT_NULL(vtunnel_ckv);
         return result_code;
       } else {
         result_code = UPLL_RC_SUCCESS;
       }
       result_code = vtunnel_mgr->UpdateConfigDB(vtunnel_ckv, UPLL_DT_RUNNING,
                                                 UNC_OP_READ, dmi,
                                                 &dbop_vtunnel, CONVERTTBL);
       DELETE_IF_NOT_NULL(vtunnel_ckv);
       if (result_code == UPLL_RC_ERR_INSTANCE_EXISTS) {
        UPLL_LOG_INFO("SpineDomain cannot be deleted");
        return UPLL_RC_ERR_CFG_SEMANTIC;
       } else if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
         result_code = UPLL_RC_SUCCESS;
       }
     }
  }
  #endif
  return result_code;
}

upll_rc_t UNWSpineDomainMoMgr::AllocVal(ConfigVal *&ck_val,
                             upll_keytype_datatype_t dt_type,
                             MoMgrTables tbl) {
  void *val;
  if (ck_val != NULL) {
    return UPLL_RC_ERR_GENERIC;
  }
  switch (tbl) {
    case MAINTBL:
      val = ConfigKeyVal::Malloc<val_unw_spdom_ext>();
      ck_val = new ConfigVal(IpctSt::kIpctStValUnwSpineDomain_Ext, val);
      if (dt_type == UPLL_DT_STATE) {
        val = ConfigKeyVal::Malloc<val_spdom_st>();
        ConfigVal *ck_nxtval = new ConfigVal(IpctSt::kIpctStValSpineAlarmSt,
                                             val);
        ck_val->AppendCfgVal(ck_nxtval);
      }
      break;

    default:
      return UPLL_RC_ERR_GENERIC;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t
UNWSpineDomainMoMgr::ValidateMessage(IpcReqRespHeader * req,
                          ConfigKeyVal * ikey)  {
  UPLL_FUNC_TRACE;
  upll_rc_t ret_val = UPLL_RC_SUCCESS;
  if (!ikey || !req || !(ikey->get_key())) {
    UPLL_LOG_ERROR("ConfigKeyVal / IpcReqRespHeader is null");
    return UPLL_RC_ERR_GENERIC;
  }
  if (!((req->option1 == UNC_OPT1_NORMAL)  ||
        (req->option1 == UNC_OPT1_DETAIL))) {
    UPLL_LOG_ERROR("Error: option1(%d) is not valid", req->option1);
    return UPLL_RC_ERR_INVALID_OPTION1;
  }
  if ((req->option1  == UNC_OPT1_DETAIL) && (req->datatype != UPLL_DT_STATE)) {
     UPLL_LOG_ERROR("Error: option1(%d) is not valid for datatype(%d)",
                    req->option1, req->datatype);
     return UPLL_RC_ERR_INVALID_OPTION1;
  }

  if (req->option2 != UNC_OPT2_NONE) {
    UPLL_LOG_ERROR("Error: option2 is not NONE");
    return UPLL_RC_ERR_INVALID_OPTION2;
  }

  if (ikey->get_st_num() != IpctSt::kIpcStKeyUnwSpineDomain) {
    UPLL_LOG_ERROR("Invalid key structure received. received struct - %d",
                  (ikey->get_st_num()));
    return UPLL_RC_ERR_BAD_REQUEST;
  }
  if (ikey->get_key_type() != UNC_KT_UNW_SPINE_DOMAIN) {
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
  key_unw_spine_domain *unw_spinedomain_key = reinterpret_cast<
      key_unw_spine_domain *> (ikey->get_key());

  if ((req->operation != UNC_OP_READ_SIBLING_COUNT) &&
      (req->operation != UNC_OP_READ_SIBLING_BEGIN)) {
    ret_val = ValidateKey(reinterpret_cast<char *>(
          unw_spinedomain_key->unw_key.unified_nw_id),
          kMinLenUnifiedNwName, kMaxLenUnifiedNwName);
    if (ret_val != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Unified network name syntax check failed."
                   "Received unified_network_name - %s",
                   unw_spinedomain_key->unw_key.unified_nw_id);
      return UPLL_RC_ERR_CFG_SYNTAX;
  }

    ret_val = ValidateKey(reinterpret_cast<char *>(
            unw_spinedomain_key->unw_spine_id),
                                kMinLenUnwSpineID,
                                kMaxLenUnwSpineID);
    if (ret_val != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Unified network name syntax check failed."
                    "Received unified spine domain name - %s",
                    unw_spinedomain_key->unw_spine_id);
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  } else {
     UPLL_LOG_TRACE("Operation is %d", req->operation);
     StringReset(unw_spinedomain_key->unw_spine_id);
  }
  // Validate value structure and opertaion  type
  ConfigVal *cfg_val = ikey->get_cfg_val();
  val_unw_spine_domain *unified_spine_dom_val = NULL;
  bool result_code = false;
  switch (req->operation) {
    case UNC_OP_CREATE:
    case UNC_OP_UPDATE:
      // val struct is mandatory for CREATE/UPDATE
      if (!cfg_val) {
        UPLL_LOG_ERROR("Configval structure is NULL for CREATE/UPDATE");
        return UPLL_RC_ERR_BAD_REQUEST;
      }
      if (cfg_val->get_st_num() != IpctSt::kIpcStValUnwSpineDomain) {
        UPLL_LOG_ERROR("Invalid val struct st num-%d",
                       (ikey->get_cfg_val())->get_st_num());
        return UPLL_RC_ERR_BAD_REQUEST;
      }
      unified_spine_dom_val =
          reinterpret_cast<val_unw_spine_domain*>(
              ikey->get_cfg_val()->get_val());

      if (unified_spine_dom_val == NULL) {
        UPLL_LOG_ERROR("val structure is NULL");
        return UPLL_RC_ERR_BAD_REQUEST;
      }
      if ((req->operation == UNC_OP_CREATE) &&
          ((unified_spine_dom_val->valid[UPLL_IDX_SPINE_CONTROLLER_ID_UNWS] !=
            UNC_VF_VALID) ||
           (unified_spine_dom_val->valid[UPLL_IDX_SPINE_DOMAIN_ID_UNWS] !=
            UNC_VF_VALID))) {
        UPLL_LOG_DEBUG("controller and domain are mandatory attributes");
        return UPLL_RC_ERR_CFG_SEMANTIC;
      }
      // fall through intended
    case UNC_OP_READ:
    case UNC_OP_READ_SIBLING:
    case UNC_OP_READ_SIBLING_BEGIN:
    case UNC_OP_READ_SIBLING_COUNT:
      if (NULL == GetVal(ikey)) {
        UPLL_LOG_DEBUG("Val structure is NULL for op(%d)", req->operation);
        return UPLL_RC_SUCCESS;
      }

      unified_spine_dom_val =
          reinterpret_cast<val_unw_spine_domain*>(
              ikey->get_cfg_val()->get_val());
      if (unified_spine_dom_val->valid[UPLL_IDX_SPINE_CONTROLLER_ID_UNWS] ==
          UNC_VF_VALID) {
        result_code = ValidateString(unified_spine_dom_val->spine_controller_id,
                                 kMinLenCtrlrId, kMaxLenCtrlrId);
        if (false == result_code)
          return UPLL_RC_ERR_CFG_SYNTAX;
      }
      if (unified_spine_dom_val->valid[UPLL_IDX_SPINE_DOMAIN_ID_UNWS] ==
          UNC_VF_VALID) {
        result_code = ValidateDefaultStr(unified_spine_dom_val->spine_domain_id,
                                 kMinLenDomainId, kMaxLenDomainId);
        if (false == result_code)
          return UPLL_RC_ERR_CFG_SYNTAX;
      }
      if (unified_spine_dom_val->valid[UPLL_IDX_UNW_LABEL_ID_UNWS] ==
          UNC_VF_VALID) {
        result_code = ValidateString(unified_spine_dom_val->
                                     unw_label_id,
                          kMinLenUnwLabelName, kMaxLenUnwLabelName);
        if (false == result_code)
          return UPLL_RC_ERR_CFG_SYNTAX;
      }
      return UPLL_RC_SUCCESS;
    case UNC_OP_DELETE:
      return UPLL_RC_SUCCESS;
    default:
      UPLL_LOG_ERROR("Invalid opertaion code (%d) received", req->operation);
      return UPLL_RC_ERR_BAD_REQUEST;
  }
}
upll_rc_t  UNWSpineDomainMoMgr::DupConfigKeyVal(ConfigKeyVal *&okey,
                          ConfigKeyVal *&req,
                          MoMgrTables tbl )  {
  UPLL_FUNC_TRACE;
  if (req == NULL) return UPLL_RC_ERR_GENERIC;
  if (okey != NULL) return UPLL_RC_ERR_GENERIC;
  if (req->get_key_type() != UNC_KT_UNW_SPINE_DOMAIN)
    return UPLL_RC_ERR_GENERIC;
  ConfigVal *cfgval = NULL, *tmp = (req)->get_cfg_val();
  UPLL_LOG_TRACE("ConfigKeyVal Req:- %s", req->ToStrAll().c_str());
  val_unw_spine_domain *spdom_val = NULL;
  val_unw_spdom_ext_t *val_spdom_ext_db = NULL;

  if (tmp) {
    if (tbl == MAINTBL) {
      if ((req->get_cfg_val())->get_st_num() ==
                                IpctSt::kIpcStValUnwSpineDomain) {
         val_unw_spine_domain *ival =
                     reinterpret_cast<val_unw_spine_domain *>(GetVal(req));
          if (!ival) return UPLL_RC_ERR_GENERIC;
          spdom_val = reinterpret_cast<val_unw_spine_domain *>(
          ConfigKeyVal::Malloc(sizeof(val_unw_spine_domain)));
          memcpy(spdom_val, ival, sizeof(val_unw_spine_domain));
          cfgval = new ConfigVal(IpctSt::kIpcStValUnwSpineDomain, spdom_val);
      } else if ((req->get_cfg_val())->get_st_num() ==
                 IpctSt::kIpctStValUnwSpineDomain_Ext) {
        val_unw_spdom_ext_t *ival_spdom_ext =
                      reinterpret_cast<val_unw_spdom_ext_t *>(tmp->get_val());
        val_spdom_ext_db = reinterpret_cast<val_unw_spdom_ext_t *>(
               ConfigKeyVal::Malloc(sizeof(val_unw_spdom_ext_t)));
        memcpy(val_spdom_ext_db, ival_spdom_ext, sizeof(val_unw_spdom_ext_t));
        cfgval = new ConfigVal(IpctSt::kIpctStValUnwSpineDomain_Ext,
                                        val_spdom_ext_db);
      }
    }
    if (!cfgval) {
      UPLL_LOG_ERROR("Memory Allocation failed for cfgval");
      FREE_IF_NOT_NULL(spdom_val);
      FREE_IF_NOT_NULL(val_spdom_ext_db);
      return UPLL_RC_ERR_GENERIC;
    }
    tmp = tmp->get_next_cfg_val();
  }
  if (tmp) {
    if (tbl == MAINTBL) {
      val_spdom_st *ival = reinterpret_cast<val_spdom_st *>
                                              (tmp->get_val());
      val_spdom_st *val_db_st = reinterpret_cast<val_spdom_st *>
                       (ConfigKeyVal::Malloc(sizeof(val_spdom_st)));
      memcpy(val_db_st, ival, sizeof(val_spdom_st));
      ConfigVal *tmp1 = new ConfigVal(IpctSt::kIpctStValSpineAlarmSt,
                        val_db_st);
      cfgval->AppendCfgVal(tmp1);
    }
  }
  void *tkey = (req)->get_key();
  if (tkey == NULL) {
    DELETE_IF_NOT_NULL(cfgval);
    UPLL_LOG_INFO("key structure is NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  key_unw_spine_domain *ikey = reinterpret_cast<key_unw_spine_domain *>(tkey);
  key_unw_spine_domain *spinedomain_key =
                reinterpret_cast<key_unw_spine_domain *>(
                ConfigKeyVal::Malloc(sizeof(key_unw_spine_domain)));
  memcpy(spinedomain_key, ikey,
                    sizeof(key_unw_spine_domain));
  okey = new ConfigKeyVal(UNC_KT_UNW_SPINE_DOMAIN,
                          IpctSt::kIpcStKeyUnwSpineDomain,
                          spinedomain_key, cfgval);
  SET_USER_DATA(okey, req)
  UPLL_LOG_TRACE("After Userdata ConfigKeyVal okey:- %s",
                 okey->ToStrAll().c_str());
  return UPLL_RC_SUCCESS;
}

upll_rc_t UNWSpineDomainMoMgr::CreateCandidateMo(IpcReqRespHeader *req,
                                       ConfigKeyVal *ikey,
                                       DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  if (ikey == NULL || req == NULL) {
      return UPLL_RC_ERR_GENERIC;
  }
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  if (req->datatype != UPLL_DT_CANDIDATE) {
    UPLL_LOG_INFO("SpineDomain does not exists on the controller");
    return result_code;
  }
  // validate syntax and semantics
  result_code = ValidateMessage(req, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_INFO("ValidateMessage failed, Error - %d", result_code);
    return result_code;
  }
  result_code = ValidateAttribute(ikey, dmi, req);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_INFO("ValidateAttribute failed, Error - %d", result_code);
    return result_code;
  }
  // Get the domain type information from UPPL and validate the response
  ConfigKeyVal *dup_ikey = NULL;
  result_code = GetChildConfigKey(dup_ikey, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_INFO("Failed to generate the configkeyval");
    return result_code;
  }
  val_unw_spine_domain *spine_domain_val =
         reinterpret_cast<val_unw_spine_domain *>(GetVal(ikey));
  if (spine_domain_val != NULL) {
    result_code = domain_util::DomainUtil::ValidateSpineDomain(
        reinterpret_cast<const char*>(spine_domain_val->spine_controller_id),
        reinterpret_cast<const char*>(spine_domain_val->spine_domain_id),
        dmi, req->datatype);
    // this function returns semantic error if the domain type is other than
    // PF_SPINE
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("domain type validation failed:%d", result_code);
      DELETE_IF_NOT_NULL(dup_ikey);
      return result_code;
    }
    val_unw_spdom_ext *spine_domain_dbval =
        ConfigKeyVal::Malloc<val_unw_spdom_ext>();
    memcpy(&(spine_domain_dbval->val_unw_spine_dom), spine_domain_val,
             sizeof(val_unw_spine_domain_t));
    spine_domain_dbval->valid[UPLL_IDX_SPINE_DOMAIN_VAL] = UNC_VF_VALID;
    // Set the valid flag for used count with default value
    spine_domain_dbval->valid[UPLL_IDX_SPINE_USED_LABEL_COUNT_UNWS] =
        UNC_VF_VALID;
    spine_domain_dbval->used_label_count = 0;
    dup_ikey->AppendCfgVal(IpctSt::kIpctStValUnwSpineDomain_Ext,
                           spine_domain_dbval);
  }

  TcConfigMode config_mode = TC_CONFIG_INVALID;
  std::string vtn_name;
  result_code = GetConfigModeInfo(req, config_mode, vtn_name);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetConfigMode failed, err %d",
                   result_code);
    DELETE_IF_NOT_NULL(dup_ikey);
    return result_code;
  }
  result_code = UpdateConfigDB(dup_ikey, req->datatype, UNC_OP_CREATE, dmi,
                               config_mode, vtn_name);
  DELETE_IF_NOT_NULL(dup_ikey);
  return result_code;
}

upll_rc_t UNWSpineDomainMoMgr::UpdateMo(
    IpcReqRespHeader *req, ConfigKeyVal *ikey, DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_ERR_GENERIC;
  if (NULL == ikey || NULL == req || !(ikey->get_key())) {
    UPLL_LOG_ERROR("Given Input is Empty");
    return UPLL_RC_ERR_GENERIC;
  }
  UPLL_LOG_TRACE("UpdateMo for %d", ikey->get_key_type());
  result_code = ValidateMessage(req, ikey);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_ERROR("Validation Message is Failed ");
    return result_code;
  }
  result_code = ValidateAttribute(ikey, dmi, req);
  if (UPLL_RC_SUCCESS  != result_code) {
    UPLL_LOG_ERROR("Validate Attribute is Failed, err %d",
                   result_code);
    return result_code;
  }

  ConfigKeyVal *dup_ikey = NULL;
  result_code = GetChildConfigKey(dup_ikey, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_INFO("Failed to generate the configkeyval, err %d",
                 result_code);
    return result_code;
  }
  void *spine_domain_val = GetVal(ikey);
  val_unw_spdom_ext *spine_domain_dbval = reinterpret_cast<val_unw_spdom_ext *>
                 (ConfigKeyVal::Malloc(sizeof(val_unw_spdom_ext)));
  memcpy(&(spine_domain_dbval->val_unw_spine_dom), spine_domain_val,
           sizeof(val_unw_spine_domain_t));
  spine_domain_dbval->valid[UPLL_IDX_SPINE_DOMAIN_VAL] = UNC_VF_VALID;
  dup_ikey->AppendCfgVal(IpctSt::kIpctStValUnwSpineDomain_Ext,
                         spine_domain_dbval);

  UPLL_LOG_TRACE("ConfigKeyVal dup_ikey:- %s", dup_ikey->ToStrAll().c_str());
  TcConfigMode config_mode = TC_CONFIG_INVALID;
  std::string vtn_name;
  result_code = GetConfigModeInfo(req, config_mode, vtn_name);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetConfigMode failed, err %d",
                   result_code);
    DELETE_IF_NOT_NULL(dup_ikey);
    return result_code;
  }
  result_code = UpdateConfigDB(dup_ikey, req->datatype, UNC_OP_UPDATE, dmi,
                               config_mode, vtn_name);
  DELETE_IF_NOT_NULL(dup_ikey);
  return result_code;
}

upll_rc_t UNWSpineDomainMoMgr::ReadSpineDomain(
    IpcReqRespHeader *req, ConfigKeyVal *ikey, DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  MoMgrTables tbl = MAINTBL;
  DbSubOp dbop = {kOpReadSingle, kOpMatchFlag, kOpInOutNone};

  if (UPLL_DT_RUNNING == req->datatype) {
    UPLL_LOG_TRACE("CSStatus Flag enabled");
    dbop.inoutop |= kOpInOutCs;
  }

  if ((req->operation == UNC_OP_READ_SIBLING_BEGIN) ||
    (req->operation == UNC_OP_READ_SIBLING)) {
    dbop.readop = kOpReadMultiple;
  } else if (req->operation == UNC_OP_READ_SIBLING_COUNT) {
    dbop.readop = kOpReadCount;
  }
  ConfigKeyVal *dupkey = NULL;
  result_code = DupConfigKeyVal(dupkey, ikey, MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_INFO("Failed to Construct ConfigKeyVal err %d", result_code);
    return result_code;
  }
  val_unw_spine_domain  *ikey_val = reinterpret_cast
                                    <val_unw_spine_domain*>(GetVal(dupkey));
  if (ikey_val != NULL) {
    val_unw_spdom_ext *spine_domain_dbval =
                  reinterpret_cast<val_unw_spdom_ext *>
                 (ConfigKeyVal::Malloc(sizeof(val_unw_spdom_ext)));
    memcpy(&(spine_domain_dbval->val_unw_spine_dom), ikey_val,
           sizeof(val_unw_spine_domain_t));
    spine_domain_dbval->valid[UPLL_IDX_SPINE_DOMAIN_VAL] = UNC_VF_VALID;
    dupkey->get_cfg_val()->SetVal(IpctSt::kIpctStValUnwSpineDomain_Ext,
                                  spine_domain_dbval);
    if (req->datatype == UPLL_DT_STATE) {
      val_spdom_st *spdom_st_dbval = ConfigKeyVal::Malloc<val_spdom_st>();
      dupkey->AppendCfgVal(IpctSt::kIpctStValSpineAlarmSt, spdom_st_dbval);
    }
  }
  result_code = ReadConfigDB(dupkey, req->datatype, req->operation,
                             dbop, req->rep_count, dmi, tbl);

  if (result_code !=UPLL_RC_SUCCESS) {
    if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      UPLL_LOG_DEBUG("No record found in DB err %d", result_code);
    } else {
      UPLL_LOG_ERROR("Fail to read from DB , OP %d, Option1 %d,"
                     "datatype %d, err %d",
                     req->operation, req->option1, req->datatype, result_code);
    }
    DELETE_IF_NOT_NULL(dupkey);
    return result_code;
  }
  if (req->operation == UNC_OP_READ_SIBLING_COUNT) {
    ikey->ResetWith(dupkey);
    DELETE_IF_NOT_NULL(dupkey);
    return result_code;
  }
  result_code = ConstructReadResponse(req, ikey, dupkey, dmi);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_INFO("Failed to construct resonse err %d", result_code);
    DELETE_IF_NOT_NULL(dupkey);
    return result_code;
  }
  UPLL_LOG_TRACE("ConfigKeyVal Response:- %s", ikey->ToStrAll().c_str());
  DELETE_IF_NOT_NULL(dupkey);
  return result_code;
}

upll_rc_t UNWSpineDomainMoMgr::ConstructReadResponse(IpcReqRespHeader *req,
                                                     ConfigKeyVal *ikey,
                                                     ConfigKeyVal *db_key,
                                                     DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  val_unw_spdom_ext  *db_val = NULL;
  // val_unw_spine_domain *spdom_val = NULL;
  ConfigKeyVal *okey = NULL;
  key_unw_spine_domain_t *unw_spinedomain_key = NULL;
  ConfigKeyVal *end_resp = NULL;
  for (ConfigKeyVal *tmp = db_key; tmp; tmp = tmp->get_next_cfg_key_val()) {
    unw_spinedomain_key = reinterpret_cast<key_unw_spine_domain_t *>(
      ConfigKeyVal::Malloc(sizeof(key_unw_spine_domain)));
    memcpy(unw_spinedomain_key, reinterpret_cast<key_unw_spine_domain *>
                                 (tmp->get_key()),
           sizeof(key_unw_spine_domain));

    val_unw_spine_domain *spdom_val =
        ConfigKeyVal::Malloc<val_unw_spine_domain>();
    db_val = reinterpret_cast<val_unw_spdom_ext*>(GetVal(tmp));
    if (db_val != NULL) {
      memcpy(spdom_val, &(db_val->val_unw_spine_dom),
             sizeof(val_unw_spine_domain));
    }
    okey = new ConfigKeyVal(UNC_KT_UNW_SPINE_DOMAIN,
                            IpctSt::kIpcStKeyUnwSpineDomain,
                            unw_spinedomain_key,
                            new ConfigVal(IpctSt::kIpcStValUnwSpineDomain,
                                          spdom_val));
    if (req->datatype == UPLL_DT_STATE) {
      // Read the Label
      result_code = GetMaxCountOfLabel(req, tmp, &okey, dmi);
      if (result_code != UPLL_RC_SUCCESS) {
         UPLL_LOG_INFO("Unable to fetch the max count for Label %d",
                       result_code);
         DELETE_IF_NOT_NULL(okey);
         DELETE_IF_NOT_NULL(end_resp);
         return result_code;
      }
    }
    if (!end_resp) {
       end_resp = okey;
    } else {
      end_resp->AppendCfgKeyVal(okey);
    }
  }
  if (end_resp) {
    ikey->ResetWith(end_resp);
    DELETE_IF_NOT_NULL(end_resp);
  }
  return result_code;
}

upll_rc_t UNWSpineDomainMoMgr::GetMaxCountOfLabel(IpcReqRespHeader *req,
                                                  ConfigKeyVal *db_key,
                                                  ConfigKeyVal **tkey,
                                                  DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  UNWLabelMoMgr* unw_label_mgr  =  reinterpret_cast<
       UNWLabelMoMgr *>(const_cast<MoManager *>(
       GetMoManager(UNC_KT_UNW_LABEL)));
  ConfigVal *cval = db_key->get_cfg_val();
  if (cval && (cval->get_st_num() ==
              IpctSt::kIpctStValUnwSpineDomain_Ext)) {
    val_unw_spdom_ext_t *dbval_spdom_ext =
                      reinterpret_cast<val_unw_spdom_ext_t *>(cval->get_val());
    val_unw_spine_domain_st *sp_dom_stval =
               reinterpret_cast<val_unw_spine_domain_st *>
               (ConfigKeyVal::Malloc(sizeof(val_unw_spine_domain_st)));
    // Assume with default range for max count
    sp_dom_stval->valid[UPLL_IDX_MAX_COUNT_UNWS_ST] = UNC_VF_VALID;
    sp_dom_stval->max_count = kUnwLabelMaxRange;
    // Assume with default status of Alarm
    sp_dom_stval->valid[UPLL_IDX_ALARM_STATUS_UNWS_ST] = UNC_VF_VALID;
    sp_dom_stval->alarm_status = 0;
    sp_dom_stval->valid[UPLL_IDX_USED_COUNT_UNWS_ST] = UNC_VF_VALID;
    sp_dom_stval->used_count = 0;
    ConfigKeyVal *label_cfg_key = NULL;
    if (dbval_spdom_ext->val_unw_spine_dom.valid[UPLL_IDX_UNW_LABEL_ID_UNWS] ==
        UNC_VF_VALID) {
      result_code = unw_label_mgr->GetChildConfigKey(label_cfg_key, NULL);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_INFO("Failed to generate the label ConfigKey");
        FREE_IF_NOT_NULL(sp_dom_stval);
        return result_code;
      }
      key_unw_label *unw_label_key = NULL;
      unw_label_key = reinterpret_cast<key_unw_label *>(
        label_cfg_key->get_key());
      key_unw_spine_domain  *unw_spinedomain_key =
                  reinterpret_cast<key_unw_spine_domain_t *>(
                  db_key->get_key());
      uuu::upll_strncpy(unw_label_key->unified_nw_key.unified_nw_id,
                        unw_spinedomain_key->unw_key.unified_nw_id,
                        (kMaxLenUnifiedNwName+1));
      uuu::upll_strncpy(unw_label_key->unw_label_id,
                        dbval_spdom_ext->val_unw_spine_dom.unw_label_id,
                       (kMaxLenUnwLabelName+1));
      DbSubOp dbop = {kOpReadSingle, kOpMatchNone, kOpInOutNone};
      result_code = unw_label_mgr->ReadConfigDB(label_cfg_key, req->datatype,
                                   UNC_OP_READ, dbop, req->rep_count,
                                   dmi, MAINTBL);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_INFO("Failed to fetch the label max count err %d",
                    result_code);
        FREE_IF_NOT_NULL(sp_dom_stval);
        DELETE_IF_NOT_NULL(label_cfg_key);
        return result_code;
      }
      if (result_code == UPLL_RC_SUCCESS) {
        val_unw_label *label_val = reinterpret_cast<val_unw_label *>
                            (GetVal(label_cfg_key));
        if (label_val != NULL) {
          if (label_val->valid[UPLL_IDX_MAX_COUNT_UNWS_ST] == UNC_VF_VALID)
            sp_dom_stval->max_count = label_val->max_count;
        }
      }
    }
    if (dbval_spdom_ext->valid[UPLL_IDX_SPINE_USED_LABEL_COUNT_UNWS] ==
        UNC_VF_VALID) {
      sp_dom_stval->used_count =  dbval_spdom_ext->used_label_count;
    }
    val_spdom_st *dbval_st =
         reinterpret_cast<val_spdom_st *>(GetStateVal(db_key));
    // Get the ST structure and copy the alarm
    //  status details
    if  (dbval_st != NULL) {
      if (dbval_st->valid[UPLL_IDX_SPINE_ALARAM_RAISED_UNWS] == UNC_VF_VALID)
        sp_dom_stval->alarm_status =  dbval_st->alarm_status;
    }
    (*tkey)->AppendCfgVal(IpctSt::kIpcStValUnwSpineDomainSt, sp_dom_stval);
    DELETE_IF_NOT_NULL(label_cfg_key);
    if (req->option1 == UNC_OPT1_DETAIL) {
        uint32_t assigned_lbl_cnt = 0;
        result_code = GetAssignedLabelDetails(req, db_key, tkey,
                                              sp_dom_stval->used_count,
                                              assigned_lbl_cnt, dmi);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_INFO("Failed to get the assigned label details, err %d",
                        result_code);
          return result_code;
        }
        sp_dom_stval->used_count = assigned_lbl_cnt;
    }
  }

  return result_code;
}

upll_rc_t UNWSpineDomainMoMgr::GetAssignedLabelDetails(
                                  IpcReqRespHeader *req,
                                  ConfigKeyVal *db_key,
                                  ConfigKeyVal **respckv,
                                  uint32_t used_count,
                                  uint32_t &assigned_lbl_count,
                                  DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  MoMgrImpl *vtunnel_mgr = reinterpret_cast<MoMgrImpl *>
                           (const_cast<MoManager *>(GetMoManager(
                           UNC_KT_VTUNNEL)));
  if ( vtunnel_mgr == NULL ) {
    UPLL_LOG_INFO("Failed to get instance of  vtunner momgr");
    return UPLL_RC_ERR_GENERIC;
  }
  DbSubOp dbop = {kOpReadMultiple, kOpMatchCtrlr|kOpMatchDomain, kOpInOutNone};
  ConfigKeyVal *vtunnel_ckv = NULL;
  result_code = vtunnel_mgr->GetChildConvertConfigKey(vtunnel_ckv, NULL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_INFO("Failed to construct configkeyval for vtunnel err %d",
                  result_code);
    return result_code;
  }
  val_unw_spdom_ext *dbval =
         reinterpret_cast<val_unw_spdom_ext *>(GetVal(db_key));

  val_convert_vtunnel *vtunnel_val =
  ConfigKeyVal::Malloc<val_convert_vtunnel>();

  SET_USER_DATA_CTRLR(vtunnel_ckv,
    dbval->val_unw_spine_dom.spine_controller_id);

  SET_USER_DATA_DOMAIN(vtunnel_ckv,
    dbval->val_unw_spine_dom.spine_domain_id);

  vtunnel_ckv->AppendCfgVal(IpctSt::kIpcStValConvertVtunnel, vtunnel_val);
  // Read from the conveted vtunnel table
  result_code = vtunnel_mgr->ReadConfigDB(vtunnel_ckv, UPLL_DT_RUNNING,
                                          req->operation, dbop,
                                          dmi, CONVERTTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      result_code = UPLL_RC_SUCCESS;
    } else {
      UPLL_LOG_INFO("Failed to read converted vtunneltbl err %d",
                    result_code);
    }
    DELETE_IF_NOT_NULL(vtunnel_ckv);
    return result_code;
  }
  ConfigKeyVal *tmp = vtunnel_ckv;
  val_unw_spine_domain_assigned_label *assigned_label = NULL;

  while (tmp) {
    assigned_label =
                  ConfigKeyVal::Malloc<val_unw_spine_domain_assigned_label>();
    key_convert_vtunnel_t *vtunnel = reinterpret_cast<key_convert_vtunnel_t *>
                                     (tmp->get_key());
    val_convert_vtunnel *con_val_tun =  reinterpret_cast<val_convert_vtunnel *>
                                       (tmp->get_cfg_val()->get_val());

    assigned_label->valid[UPLL_IDX_LABEL_UNWSAL] =
                         con_val_tun->valid[UPLL_IDX_LABEL_CONV_VTNL];
    assigned_label->label =con_val_tun->label;

    assigned_label->valid[UPLL_IDX_VTN_ID_UNWSAL] = UNC_VF_VALID;
    uuu::upll_strncpy(assigned_label->vtn_id,
                      vtunnel->vtn_key.vtn_name,
                      (kMaxLenVtnName+1));
    assigned_label->valid[UPLL_IDX_VNODE_ID_UNWSAL] = UNC_VF_INVALID;
    (*respckv)->AppendCfgVal(IpctSt::kIpcStValUnwSpineDomainAssignedLabel,
                          assigned_label);
    assigned_lbl_count++;
    tmp = tmp->get_next_cfg_key_val();
  }
  UPLL_LOG_TRACE("ConfigKeyVal Resckv:- %s", (*respckv)->ToStrAll().c_str());
  DELETE_IF_NOT_NULL(vtunnel_ckv);
  return result_code;
}

upll_rc_t UNWSpineDomainMoMgr::ReadMo(IpcReqRespHeader *req,
                                      ConfigKeyVal *ikey,
                                      DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  result_code = ValidateMessage(req, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_ERROR("ValidateMessage failed result_code %d",
                   result_code);
    return result_code;
  }
  result_code = ReadSpineDomain(req, ikey, dmi);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Request Fail ,OP = %d, Option 1 =%d, datatype = %d, err %d",
                     req->operation, req->option1, req->datatype, result_code);
  }
  return result_code;
}
upll_rc_t UNWSpineDomainMoMgr::ReadSiblingMo(IpcReqRespHeader *req,
                                             ConfigKeyVal *ikey,
                                             bool begin,
                                             DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  result_code = ValidateMessage(req, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_ERROR("ValidateMessage failed result_code %d",
                   result_code);
    return result_code;
  }
  result_code = ReadSpineDomain(req, ikey, dmi);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Request Failed ,OP=%d, Option 1 =%d, dtype =%d, err %d",
                  req->operation, req->option1, req->datatype, result_code);
  }
  return result_code;
}

upll_rc_t UNWSpineDomainMoMgr::ReadSiblingCount(IpcReqRespHeader *req,
                                                ConfigKeyVal* ikey,
                                                DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  result_code = ValidateMessage(req, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_ERROR("ValidateMessage failed result_code %d",
                   result_code);
    return result_code;
  }
  result_code = ReadSpineDomain(req, ikey, dmi);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Request Failed ,OP=%d, Option 1 =%d, dtype =%d, err %d",
                  req->operation, req->option1, req->datatype, result_code);
  }
  return result_code;
}
upll_rc_t UNWSpineDomainMoMgr::GetOperation(uuc::UpdateCtrlrPhase phase,
                                 unc_keytype_operation_t &op) {
  if (uuc::kUpllUcpDelete2 == phase) {
    UPLL_LOG_DEBUG("Delete phase 1");
    return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_KT;
  } else if (uuc::kUpllUcpUpdate == phase) {
     op =  UNC_OP_UPDATE;
  } else if (uuc::kUpllUcpCreate == phase) {
     op = UNC_OP_CREATE;
  } else if (uuc::kUpllUcpDelete == phase) {
     op = UNC_OP_DELETE;
  } else {
     return UPLL_RC_ERR_GENERIC;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t UNWSpineDomainMoMgr::UpdateConfigStatus(ConfigKeyVal *ikey,
                                                  unc_keytype_operation_t op,
                                                  uint32_t driver_result,
                                                  ConfigKeyVal *upd_key,
                                                  DalDmlIntf *dmi,
                                                  ConfigKeyVal *ctrlr_key) {
  UPLL_FUNC_TRACE;

  val_unw_spdom_ext *val = reinterpret_cast<val_unw_spdom_ext*>(GetVal(ikey));
  val->val_unw_spine_dom.cs_row_status = UNC_CS_APPLIED;
  if (val->val_unw_spine_dom.valid[UPLL_IDX_SPINE_CONTROLLER_ID_UNWS] ==
      UNC_VF_VALID)
    val->val_unw_spine_dom.cs_attr[0] = UNC_CS_APPLIED;
  if (val->val_unw_spine_dom.valid[UPLL_IDX_SPINE_DOMAIN_ID_UNWS] ==
      UNC_VF_VALID)
    val->val_unw_spine_dom.cs_attr[1] = UNC_CS_APPLIED;
  if (val->val_unw_spine_dom.valid[UPLL_IDX_UNW_LABEL_ID_UNWS] ==
     UNC_VF_VALID)
    val->val_unw_spine_dom.cs_attr[2] = UNC_CS_APPLIED;
  if (op == UNC_OP_UPDATE) {
    void *ival = reinterpret_cast<void *>(val);
    CompareValidValue(ival, GetVal(upd_key), true);
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t UNWSpineDomainMoMgr::IsKeyInUse(upll_keytype_datatype_t dt_type,
                       const ConfigKeyVal *ckv,
                       bool *in_use,
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

  ConfigKeyVal *unw_spine_ckv = NULL;
  //  populate unified network spine config key val with
  //  received controller name
  result_code = GetChildConfigKey(unw_spine_ckv, NULL);
  if (result_code != UPLL_RC_SUCCESS) {
    return result_code;
  }
  val_unw_spdom_ext *val = reinterpret_cast<val_unw_spdom_ext *>(
      ConfigKeyVal::Malloc(sizeof(val_unw_spdom_ext)));
  uuu::upll_strncpy(val->val_unw_spine_dom.spine_controller_id,
                    ctr->controller_name, kMaxLenCtrlrId + 1);
  val->val_unw_spine_dom.valid[UPLL_IDX_SPINE_CONTROLLER_ID_UNWS] =
                              UNC_VF_VALID;
  val->valid[UPLL_IDX_SPINE_DOMAIN_VAL] = UNC_VF_VALID;
  unw_spine_ckv->AppendCfgVal(IpctSt::kIpctStValUnwSpineDomain_Ext, val);

  // set in_use as true if received controller is found in unified network
  // spine tbl
  result_code = ReadConfigDB(unw_spine_ckv, dt_type, UNC_OP_READ,
                               dbop, dmi, MAINTBL);
  *in_use = (result_code == UPLL_RC_SUCCESS) ? true : false;
  delete unw_spine_ckv;
  UPLL_LOG_DEBUG("Returning %d", result_code);
  return (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE)
           ? UPLL_RC_SUCCESS : result_code;
}

upll_rc_t UNWSpineDomainMoMgr::UpdateAlarmStatus(
    ConfigKeyVal *ikey, bool assert_alarm,
    TcConfigMode config_mode, std::string vtn_name, DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *alarm_ckv = NULL;
  result_code = GetChildConfigKey(alarm_ckv, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_ERROR("Failed to construct ConfigKeyVal, err %d", result_code);
    return result_code;
  }

  ConfigVal *cv = NULL;
  result_code = AllocVal(cv, UPLL_DT_STATE, MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    DELETE_IF_NOT_NULL(alarm_ckv);
    UPLL_LOG_INFO("Memory initialization failed %d", result_code);
    return result_code;
  }
  alarm_ckv->SetCfgVal(cv);
  val_spdom_st *val = reinterpret_cast<val_spdom_st *>(GetStateVal(alarm_ckv));
  if (val == NULL) {
    UPLL_LOG_ERROR("Invalid Val Structure");
    DELETE_IF_NOT_NULL(alarm_ckv);
    return UPLL_RC_ERR_GENERIC;
  }
  val->valid[UPLL_IDX_SPINE_ALARAM_RAISED_UNWS] = UNC_VF_VALID;
  val->alarm_status = (assert_alarm) ? 1 : 0;

  DbSubOp dbop = {kOpNotRead, kOpMatchNone, kOpInOutNone};
  result_code = UpdateConfigDB(alarm_ckv, UPLL_DT_STATE, UNC_OP_UPDATE, dmi,
                               &dbop, config_mode, vtn_name);
  DELETE_IF_NOT_NULL(alarm_ckv);
  return ((result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE)
           ? UPLL_RC_SUCCESS : result_code);
}

upll_rc_t UNWSpineDomainMoMgr::ProcessAlarm(
    TcConfigMode config_mode, std::string vtn_name, DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  if (threshold_alarm_processing_required_ == false) {
    return UPLL_RC_SUCCESS;
  }

  key_unw_spine_domain *key_db = ConfigKeyVal::Malloc<key_unw_spine_domain>();
  val_unw_spdom_ext_t *val_db =
      ConfigKeyVal::Malloc<val_unw_spdom_ext_t>();
  ConfigVal *ck_val = new ConfigVal(IpctSt::kIpctStValUnwSpineDomain_Ext,
                                    val_db);
  ConfigVal *ck_stval = new ConfigVal(IpctSt::kIpctStValSpineAlarmSt,
                                      ConfigKeyVal::Malloc<val_spdom_st>());
  ck_val->AppendCfgVal(ck_stval);
  DbSubOp dbop = {kOpReadMultiple, kOpMatchNone, kOpInOutNone};
  ConfigKeyVal *okey = new ConfigKeyVal(UNC_KT_UNW_SPINE_DOMAIN,
                                        IpctSt::kIpcStKeyUnwSpineDomain,
                                        key_db, ck_val);
  result_code = ReadConfigDB(okey, UPLL_DT_STATE, UNC_OP_READ,
                             dbop, dmi, MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    DELETE_IF_NOT_NULL(okey);
    if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
       result_code = UPLL_RC_SUCCESS;
    } else {
      UPLL_LOG_INFO("Error while fetching record from DB, err %d",
                    result_code);
    }
    return result_code;
  }
  for (ConfigKeyVal *tmp_ckv = okey; tmp_ckv;
       tmp_ckv = tmp_ckv->get_next_cfg_key_val()) {
     // 1. Check whether the label id is valid or not
     //   it it s not valid then go for next configkey
     // 2. if used count is not valid then go for next configkey
     // 3 Fetch the record from the label table using the label id
     //    and unified network id
     // 4. Fetch the alarm flag from the val structure
     // 5. Fetch the failing and raising threshold from label configkey
     //    and compare it with used count
     // 6. If raising threshold < used count and alarm flag is not set then
     //       Send the alaram
     // 7. Update the alarm status flag in DB
     // 8. if falling threshold > used count and alarm flag is set then
     //      Clear the alaram
     // 9. Update the alarm status flag in DB
     key_unw_spine_domain *unw_spinedomain_key  =
         reinterpret_cast<key_unw_spine_domain*>(tmp_ckv->get_key());

      val_unw_spdom_ext *dbval =
          reinterpret_cast<val_unw_spdom_ext *>(GetVal(tmp_ckv));
     val_spdom_st *dbval_st =
         reinterpret_cast<val_spdom_st *>(GetStateVal(tmp_ckv));
      // If label is not available or used count is zero then
      //  clear the alarm if it is not cleared
     if ((dbval->val_unw_spine_dom.valid[UPLL_IDX_UNW_LABEL_ID_UNWS]
           != UNC_VF_VALID) ||
         (dbval->used_label_count == 0)) {
       if (dbval_st->alarm_status != 0) {
         result_code = UpdateAlarmStatus(tmp_ckv, false, config_mode,
                                         vtn_name, dmi);
         if (result_code != UPLL_RC_SUCCESS) {
           UPLL_LOG_INFO("Failed to update the alarm status in DB, err %d",
                         result_code);
           DELETE_IF_NOT_NULL(okey);
           return result_code;
         }
         if (!uuc::UpllConfigMgr::GetUpllConfigMgr()->SendSpineThresholdAlarm(
               reinterpret_cast<const char*>(
                 unw_spinedomain_key->unw_key.unified_nw_id),
               reinterpret_cast<const char*>(
                 unw_spinedomain_key->unw_spine_id),
               false)) {
           UPLL_LOG_ERROR("Failed to clear threshold exceed alarm, spineid %s",
                          reinterpret_cast<const char*>(
                            unw_spinedomain_key->unw_spine_id));
         }
       }
       continue;
     }
     UNWLabelMoMgr* unw_label_mgr = reinterpret_cast<UNWLabelMoMgr *>
         (const_cast<MoManager *>(GetMoManager(UNC_KT_UNW_LABEL)));
     ConfigKeyVal *temp_unw_label_ckv = NULL;
     result_code = unw_label_mgr->GetChildConfigKey(temp_unw_label_ckv, NULL);
     if (result_code != UPLL_RC_SUCCESS) {
       UPLL_LOG_INFO("Failed to create unw label ConfigKeyVal");
       return result_code;
     }
     key_unw_label *unw_label_key = reinterpret_cast<key_unw_label *>
         (temp_unw_label_ckv->get_key());
     uuu::upll_strncpy(unw_label_key->unified_nw_key.unified_nw_id,
                       unw_spinedomain_key->unw_key.unified_nw_id,
                       (kMaxLenUnifiedNwName+1));
     uuu::upll_strncpy(unw_label_key->unw_label_id,
                       dbval->val_unw_spine_dom.unw_label_id,
                       (kMaxLenUnwLabelName+1));
     DbSubOp dbop_lbl = {kOpReadSingle, kOpMatchNone, kOpInOutNone};
     result_code = unw_label_mgr->ReadConfigDB(temp_unw_label_ckv,
                                               UPLL_DT_STATE, UNC_OP_READ,
                                               dbop_lbl, dmi, MAINTBL);
     if (result_code != UPLL_RC_SUCCESS) {
        DELETE_IF_NOT_NULL(temp_unw_label_ckv);
        UPLL_LOG_INFO("Error fetching label record, err %d", result_code);
        return result_code;
     }
     val_unw_label *lbl_val =
         reinterpret_cast<val_unw_label *>(GetVal(temp_unw_label_ckv));
     // If any of the failing or raising threshold is not set then
     // set the default values
     // falling threshold as 1
     // raising threshold as 4000
     if (lbl_val->valid[UPLL_IDX_RAISING_THRESHOLD_UNWL] != UNC_VF_VALID) {
       lbl_val->raising_threshold = kDefRaisingThresholdRange;
     }
     if (lbl_val->valid[UPLL_IDX_FALLING_THRESHOLD_UNWL] != UNC_VF_VALID) {
         lbl_val->falling_threshold =  kDefFallingThresholdRange;
     }
     bool ret = false;
     if ((lbl_val->raising_threshold < dbval->used_label_count) &&
         (dbval_st->alarm_status == 0)) {
       UPLL_LOG_INFO("Raise threshold exceed alarm");
       // 2. Call the Alarm Template API
       // 1. Update the Alarm status in Running DB
       // Send can fail if lot of alarm is there. Need to do it in a loop.
       result_code = UpdateAlarmStatus(tmp_ckv, true,
                                       config_mode, vtn_name, dmi);
       if (result_code != UPLL_RC_SUCCESS) {
         UPLL_LOG_INFO("Failed to update the alarm status in DB, err %d",
                       result_code);
         DELETE_IF_NOT_NULL(temp_unw_label_ckv);
         DELETE_IF_NOT_NULL(okey);
         return result_code;
       }
       ret = uuc::UpllConfigMgr::GetUpllConfigMgr()->SendSpineThresholdAlarm(
           reinterpret_cast<const char*>(
               unw_spinedomain_key->unw_key.unified_nw_id),
           reinterpret_cast<const char*>(
               unw_spinedomain_key->unw_spine_id),
           true);
       if (!ret) {
         UPLL_LOG_ERROR("Failed to raise threshold exceed alarm, spineid %s",
                        reinterpret_cast<const char*>(
                          unw_spinedomain_key->unw_spine_id));
       }
     }
     if ((lbl_val->falling_threshold > dbval->used_label_count) &&
         (dbval_st->alarm_status != 0)) {
       UPLL_LOG_INFO("Clear the threshold exceed alarm");
       // 1. Clear the alarm status in running DB
       // 2. Call the Alarm Template API to clear the alarm
       result_code = UpdateAlarmStatus(tmp_ckv, false,
                                       config_mode, vtn_name, dmi);
       if (result_code != UPLL_RC_SUCCESS) {
         UPLL_LOG_INFO("Failed to update the alarm status in DB, err %d",
                       result_code);
         DELETE_IF_NOT_NULL(temp_unw_label_ckv);
         DELETE_IF_NOT_NULL(okey);
         return result_code;
        }
        ret = uuc::UpllConfigMgr::GetUpllConfigMgr()->SendSpineThresholdAlarm(
            reinterpret_cast<const char*>(
                unw_spinedomain_key->unw_key.unified_nw_id),
            reinterpret_cast<const char*>(
                unw_spinedomain_key->unw_spine_id),
            false);
        if (!ret) {
          UPLL_LOG_ERROR("Failed to clear threshold exceed alarm, spineid %s",
                         reinterpret_cast<const char*>(
                           unw_spinedomain_key->unw_spine_id));
        }
     }
     DELETE_IF_NOT_NULL(temp_unw_label_ckv);
  }
  DELETE_IF_NOT_NULL(okey);

  return result_code;
}

upll_rc_t UNWSpineDomainMoMgr::HandleSpineThresholdAlarm(
    bool assert_alarm, DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigVal *ck_val = new ConfigVal(IpctSt::kIpctStValUnwSpineDomain_Ext,
                                   ConfigKeyVal::Malloc<val_unw_spdom_ext_t>());
  ConfigVal *ck_stval = new ConfigVal(IpctSt::kIpctStValSpineAlarmSt,
                                      ConfigKeyVal::Malloc<val_spdom_st>());
  ck_val->AppendCfgVal(ck_stval);
  DbSubOp dbop = {kOpReadMultiple, kOpMatchNone, kOpInOutNone};
  ConfigKeyVal *okey = new ConfigKeyVal(
      UNC_KT_UNW_SPINE_DOMAIN, IpctSt::kIpcStKeyUnwSpineDomain,
      ConfigKeyVal::Malloc<key_unw_spine_domain>(), ck_val);
  result_code = ReadConfigDB(okey, UPLL_DT_STATE, UNC_OP_READ,
                             dbop, dmi, MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    DELETE_IF_NOT_NULL(okey);
    if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
       result_code = UPLL_RC_SUCCESS;
    } else {
      UPLL_LOG_INFO("Error while fetching record from DB, err %d",
                    result_code);
    }
    return result_code;
  }
  for (ConfigKeyVal *tmp_ckv = okey; tmp_ckv;
       tmp_ckv = tmp_ckv->get_next_cfg_key_val()) {
     val_spdom_st *dbval_st =
         reinterpret_cast<val_spdom_st *>(GetStateVal(tmp_ckv));
     key_unw_spine_domain *unw_spinedomain_key  =
         reinterpret_cast<key_unw_spine_domain*>(tmp_ckv->get_key());
     // * During ColdStart/StandBy to Active
     //    Raise an alarm if alarm status is set in DB
     // * During  Active To StandBy
     //    Clear the Alarm if it is set
     // * The input argument alarm_status should be
     //    True in ColdStart/StandBy to Active
     //    False in Active To StandBy.
     if (dbval_st != NULL && dbval_st->alarm_status != 0) {
       if (!uuc::UpllConfigMgr::GetUpllConfigMgr()->
           SendSpineThresholdAlarm(
               reinterpret_cast<const char*>(
                   unw_spinedomain_key->unw_key.unified_nw_id),
               reinterpret_cast<const char*>(
                   unw_spinedomain_key->unw_spine_id),
               (assert_alarm) ? true : false)) {
         UPLL_LOG_INFO("Failed to process threshold alarm, spine id %s",
                       reinterpret_cast<char*>(
                          unw_spinedomain_key->unw_spine_id));
       }
     }
  }
  DELETE_IF_NOT_NULL(okey);
  return result_code;
}

upll_rc_t UNWSpineDomainMoMgr::GetParentConfigKey(ConfigKeyVal *&okey,
                                                  ConfigKeyVal *ikey) {
  UPLL_FUNC_TRACE;
  if (ikey == NULL) {
    UPLL_LOG_DEBUG("Null ikey param");
    return UPLL_RC_ERR_GENERIC;
  }
  unc_key_type_t ikey_type = ikey->get_key_type();
  if (ikey_type != UNC_KT_UNW_SPINE_DOMAIN) {
    UPLL_LOG_DEBUG("Invalid input arg");
    return UPLL_RC_ERR_GENERIC;
  }
  DELETE_IF_NOT_NULL(okey);
  key_unw_spine_domain *unwsd_key  = reinterpret_cast<key_unw_spine_domain*>
                                    (ikey->get_key());
  if (!unwsd_key) {
    UPLL_LOG_DEBUG("NULL ikey");
    return UPLL_RC_ERR_GENERIC;
  }
  key_unified_nw_t *unw_key = ConfigKeyVal::Malloc<key_unified_nw_t>();
  uuu::upll_strncpy(unw_key->unified_nw_id,
                    unwsd_key->unw_key.unified_nw_id,
                    (kMaxLenUnifiedNwName+1));
  okey = new ConfigKeyVal(UNC_KT_UNIFIED_NETWORK, IpctSt::kIpcStKeyUnifiedNw,
                          unw_key, NULL);
  if (okey == NULL) {
    UPLL_LOG_DEBUG("NULL okey");
    free(unw_key);
    return UPLL_RC_ERR_GENERIC;
  } else {
    SET_USER_DATA(okey, ikey);
  }
  return UPLL_RC_SUCCESS;
}
}  // namespace kt_momgr
}  // namespace upll
}  // namespace unc
