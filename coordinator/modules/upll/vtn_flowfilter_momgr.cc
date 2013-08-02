/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include<string>
#include<set>
#include "pfc/log.h"
#include "vtn_flowfilter_momgr.hh"
#include "vtn_flowfilter_entry_momgr.hh"
#include "unc/upll_errno.h"
#include "upll_validation.hh"
#include "unc/upll_ipc_enum.h"
#include"upll_log.hh"
#include "ctrlr_capa_defines.hh"
#include "vtn_momgr.hh"


namespace unc {
namespace upll {
namespace kt_momgr {

#define NUM_KEY_MAIN_TBL_  5

// Vtn_FlowFilter Table(Main Table)
BindInfo VtnFlowFilterMoMgr::vtn_flowfilter_bind_info[] = {
  { uudst::vtn_flowfilter::kDbiVtnName, CFG_KEY,
    offsetof(key_vtn_flowfilter_t, vtn_key.vtn_name),
    uud::kDalChar, (kMaxLenVtnName + 1) },
  { uudst::vtn_flowfilter::kDbiInputDirection, CFG_KEY,
    offsetof(key_vtn_flowfilter_t, input_direction),
    uud::kDalUint8, 1 },
  { uudst::vtn_flowfilter::kDbiFlags, CK_VAL,
    offsetof(key_user_data_t, flags),
    uud::kDalUint8, 1 },
  { uudst::vtn_flowfilter::kDbiCsRowStatus, CS_VAL,
    offsetof(val_flowfilter_t, cs_row_status),
    uud::kDalUint8, 1 }
};

// Vtn_FlowFilter_Ctrl Table(Main Table)
BindInfo VtnFlowFilterMoMgr::vtn_flowfilter_ctrl_bind_info[] = {
  { uudst::vtn_flowfilter_ctrlr::kDbiVtnName, CFG_KEY,
    offsetof(key_vtn_flowfilter_t, vtn_key.vtn_name),
    uud::kDalChar, (kMaxLenVtnName + 1) },
  { uudst::vtn_flowfilter_ctrlr::kDbiInputDirection, CFG_KEY,
    offsetof(key_vtn_flowfilter_t, input_direction),
    uud::kDalUint8, 1 },
  { uudst::vtn_flowfilter_ctrlr::kDbiCtrlrName, CK_VAL,
    offsetof(key_user_data_t, ctrlr_id),
    uud::kDalChar, (kMaxLenCtrlrId + 1) },
  { uudst::vtn_flowfilter_ctrlr::kDbiDomainId, CK_VAL,
    offsetof(key_user_data_t, domain_id),
    uud::kDalChar, (kMaxLenDomainId + 1) },
  { uudst::vtn_flowfilter_ctrlr::kDbiFlags, CK_VAL,
    offsetof(key_user_data_t, flags),
    uud::kDalUint8, 1 },
  { uudst::vtn_flowfilter_ctrlr::kDbiCsRowStatus, CS_VAL,
    offsetof(val_flowfilter_t, cs_row_status),
    uud::kDalUint8, 1 }
};

BindInfo VtnFlowFilterMoMgr::vtn_flowfilter_maintbl_rename_bindinfo[] = {
  { uudst::vtn_flowfilter::kDbiVtnName, CFG_MATCH_KEY, offsetof(
    key_vtn_flowfilter_t, vtn_key.vtn_name),
    uud::kDalChar, kMaxLenVtnName + 1 },
  { uudst::vtn_flowfilter::kDbiInputDirection, CFG_MATCH_KEY, offsetof(
    key_vtn_flowfilter_t, input_direction),
    uud::kDalUint8, 1 },
  { uudst::vtn_flowfilter::kDbiVtnName, CFG_INPUT_KEY, offsetof(
    key_rename_vnode_info_t, new_unc_vtn_name),
    uud::kDalChar, kMaxLenVtnName + 1 },
  { uudst::vtn_flowfilter::kDbiFlags, CFG_INPUT_KEY, offsetof(
    key_user_data_t, flags),
    uud::kDalUint8, 1 }
};

BindInfo VtnFlowFilterMoMgr::vtn_flowfilter_ctrlrtbl_rename_bindinfo[] = {
  { uudst::vtn_flowfilter_ctrlr::kDbiVtnName, CFG_MATCH_KEY, offsetof(
    key_vtn_flowfilter_t, vtn_key.vtn_name),
    uud::kDalChar, kMaxLenVtnName + 1 },
  { uudst::vtn_flowfilter_ctrlr::kDbiInputDirection, CFG_MATCH_KEY, offsetof(
    key_vtn_flowfilter_t, input_direction),
    uud::kDalUint8, 1 },
  { uudst::vtn_flowfilter_ctrlr::kDbiVtnName, CFG_INPUT_KEY, offsetof(
    key_rename_vnode_info_t, new_unc_vtn_name),
    uud::kDalChar, kMaxLenVtnName + 1 },
  { uudst::vtn_flowfilter_ctrlr::kDbiFlags, CFG_INPUT_KEY, offsetof(
    key_user_data_t, flags),
    uud::kDalUint8, 1 },
  { uudst::vtn_flowfilter_ctrlr::kDbiCtrlrName, CK_VAL, offsetof(
    key_user_data_t, ctrlr_id),
    uud::kDalChar, kMaxLenCtrlrId+1 },
  { uudst::vtn_flowfilter_ctrlr::kDbiDomainId, CK_VAL, offsetof(
    key_user_data_t, domain_id),
    uud::kDalChar, kMaxLenDomainId+1 }
};

unc_key_type_t VtnFlowFilterMoMgr::vtn_flowfilter_child[] = {
    UNC_KT_VTN_FLOWFILTER_ENTRY
};

VtnFlowFilterMoMgr::VtnFlowFilterMoMgr() : MoMgrImpl() {
  UPLL_FUNC_TRACE;
  // Rename operation is not support for this KT
  // setting max tables to 2
  ntable = MAX_MOMGR_TBLS;
  table = new Table *[ntable];

  // For Main Table
  table[MAINTBL] = new Table(uudst::kDbiVtnFlowFilterTbl,
      UNC_KT_VTN_FLOWFILTER, vtn_flowfilter_bind_info,
      IpctSt::kIpcStKeyVtnFlowfilter, IpctSt::kIpcStValFlowfilter,
      uudst::vtn_flowfilter::kDbiVtnFlowFilterNumCols);
  table[RENAMETBL] = NULL;
  // for ctrlr table
  table[CTRLRTBL] = new Table(uudst::kDbiVtnFlowFilterCtrlrTbl,
                  UNC_KT_VTN_FLOWFILTER,
                  vtn_flowfilter_ctrl_bind_info,
                  IpctSt::kIpcStKeyVtnFlowfilter,
                  IpctSt::kIpcInvalidStNum,
                  uudst::vtn_flowfilter_ctrlr::kDbiVtnFlowFilterCtrlrNumCols);

  nchild = sizeof(vtn_flowfilter_child) / sizeof(vtn_flowfilter_child[0]);
  child = vtn_flowfilter_child;
}

upll_rc_t VtnFlowFilterMoMgr::MergeValidate(unc_key_type_t keytype,
                                            const char *ctrlr_id,
                                            ConfigKeyVal *ikey,
                                            DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  UPLL_LOG_DEBUG("No validations require for VTNFlowfilter");
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtnFlowFilterMoMgr::UpdateMo(IpcReqRespHeader *req,
                                       ConfigKeyVal *ikey, DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  UPLL_LOG_DEBUG(" UpdateMo  Success ");
  return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_KT;
}

upll_rc_t VtnFlowFilterMoMgr::CreateCandidateMo(IpcReqRespHeader *req,
                                                ConfigKeyVal *ikey,
                                                DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  if (ikey == NULL && req == NULL) {
    UPLL_LOG_DEBUG("Insufficient parameters");
    return UPLL_RC_ERR_GENERIC;
  }
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  upll_rc_t vtn_ctrlr_span_rt_code = UPLL_RC_SUCCESS;
  // validate syntax and semantics
  result_code = ValidateMessage(req, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("ValidateMessage failed: err code(%d)", result_code);
    return result_code;
  }

  result_code = ValidateAttribute(ikey, dmi, req);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("ValidateAttribute failed: err code(%d)", result_code);
    return result_code;
  }

  // Check if VTN Flowfilter already exists in CANDIDATE DB
  result_code = UpdateConfigDB(ikey, req->datatype, UNC_OP_READ, dmi);
  if ((result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) &&
      (result_code != UPLL_RC_ERR_INSTANCE_EXISTS)) {
    UPLL_LOG_DEBUG(" UpdateConfigDB() error (%d)", result_code);
    return result_code;
  }
  if (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    UPLL_LOG_DEBUG("Record already exists in Candidate DB: err code(%d)",
                   result_code);
    return result_code;
  }
  // Check if VTN flowfilter exists in RUNNING DB and move it to CANDIDATE DB
  result_code = UpdateConfigDB(ikey, UPLL_DT_RUNNING, UNC_OP_READ,
                               dmi, MAINTBL);
  if ((result_code != UPLL_RC_ERR_INSTANCE_EXISTS) &&
      (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE)) {
    UPLL_LOG_DEBUG(" Is Exist check operation failed: err code(%d)",
                   result_code);
    return result_code;
  }
  if (result_code == UPLL_RC_ERR_INSTANCE_EXISTS) {
    result_code = RestoreChildren(ikey, req->datatype, UPLL_DT_RUNNING, dmi);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Failed to Restore the configuration: err code(%d)",
                     result_code);
      return UPLL_RC_ERR_GENERIC;
    }
  }

  std::list<controller_domain_t> list_ctrlr_dom;
  vtn_ctrlr_span_rt_code = GetVtnControllerSpan(ikey, dmi, list_ctrlr_dom);
  if ((vtn_ctrlr_span_rt_code != UPLL_RC_SUCCESS) &&
      (vtn_ctrlr_span_rt_code != UPLL_RC_ERR_NO_SUCH_INSTANCE)) {
    UPLL_LOG_DEBUG(" GetVtnControllerSpan  error code (%d)",
                   vtn_ctrlr_span_rt_code);
    return result_code;
  }

  if (vtn_ctrlr_span_rt_code == UPLL_RC_SUCCESS) {
    for (std::list<controller_domain_t>::iterator it= list_ctrlr_dom.begin();
         it != list_ctrlr_dom.end(); ++it) {
      result_code = ValidateCapability(
          req, ikey,
          reinterpret_cast<const char *>(it->ctrlr));
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Key not supported by controller");
        return result_code;
      }
    }
  }
  // create a record in CANDIDATE DB for Main Table
  result_code = UpdateConfigDB(ikey, req->datatype, UNC_OP_CREATE,
                               dmi, MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("create in CandidateDB failed: err code(%d) ",
                   result_code);
    return result_code;
  }
  // create a record in CANDIDATE DB for controller Table
  if (vtn_ctrlr_span_rt_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    result_code = UpdateControllerTable(ikey, UNC_OP_CREATE,
                                        req->datatype, dmi,
                                        list_ctrlr_dom);

    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("create in ctrlr tbl failed: error code (%d)",
                     result_code);
      result_code = UpdateConfigDB(ikey, req->datatype, UNC_OP_DELETE,
                                   dmi, MAINTBL);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("delete in CandidateDB failed: err code(%d) ",
                       result_code);
      }
    }
  }

  for (std::list<controller_domain_t>::iterator it= list_ctrlr_dom.begin();
       it != list_ctrlr_dom.end(); ++it) {
    free(it->ctrlr);
    free(it->domain);
  }
  return result_code;
}

upll_rc_t VtnFlowFilterMoMgr::UpdateControllerTable(
    ConfigKeyVal *ikey, unc_keytype_operation_t op,
    upll_keytype_datatype_t dt_type,
    DalDmlIntf *dmi,
    std::list<controller_domain_t> list_ctrlr_dom) {
  UPLL_FUNC_TRACE;
  ConfigKeyVal *ctrlckv = NULL;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  for (std::list<controller_domain_t>::iterator it= list_ctrlr_dom.begin();
       it != list_ctrlr_dom.end(); ++it) {
    key_vtn_flowfilter_t *vtn_ff_key = reinterpret_cast<key_vtn_flowfilter_t*>
        (ConfigKeyVal::Malloc(sizeof(key_vtn_flowfilter_t)));
    memcpy(vtn_ff_key, reinterpret_cast<key_vtn_flowfilter_t*>(ikey->get_key()),
           sizeof(key_vtn_flowfilter_t));
    ctrlckv = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER, IpctSt::kIpcInvalidStNum,
                               vtn_ff_key, NULL);
    SET_USER_DATA_CTRLR_DOMAIN(ctrlckv, *it);

    // Create/Update/Delete a record in CANDIDATE DB
    result_code = UpdateConfigDB(ctrlckv, dt_type, op, dmi, CTRLRTBL);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Err while inserting in ctrlr table for candidateDb(%d)",
                     result_code);
    }

    if (ctrlckv) {
      delete ctrlckv;
      ctrlckv = NULL;
    }
    if (result_code != UPLL_RC_SUCCESS) {
      return result_code;
    }
  }
  return result_code;
}

upll_rc_t VtnFlowFilterMoMgr::UpdateControllerTableForVtn(uint8_t* vtn_name,
                                                  controller_domain *ctrlr_dom,
                                                  unc_keytype_operation_t op,
                                                  DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  /* TODO */
  ConfigKeyVal *ctrlckv = NULL, *ikey = NULL;
  result_code = GetChildConfigKey(ikey, NULL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetChildConfigKey is fail");
    return result_code;
  }
  key_vtn_flowfilter *vtn_ff_key = reinterpret_cast<key_vtn_flowfilter_t*>
                                   (ikey->get_key());
  uuu::upll_strncpy(vtn_ff_key->vtn_key.vtn_name,
                    vtn_name, (kMaxLenVtnName+1));
  // set this value so that the direction
  // can be bound for output instead of match
  vtn_ff_key->input_direction = 0xFE;

  DbSubOp dbop = {kOpReadMultiple, kOpMatchNone, kOpInOutNone};

  // Read the Configuration from the MainTable
  result_code = ReadConfigDB(ikey, UPLL_DT_CANDIDATE,
                             UNC_OP_READ, dbop, dmi, MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      UPLL_LOG_DEBUG(" No Records in main table to be created in ctrlr tbl");
      return UPLL_RC_SUCCESS;
    }
    delete ikey;
    return result_code;
  }
  while (ikey != NULL) {
    result_code = GetControllerKeyval(ctrlckv, ikey, ctrlr_dom);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("GetControllerKeyval is Fail");
      return UPLL_RC_ERR_GENERIC;
    }
    if (ctrlckv == NULL) {
      UPLL_LOG_DEBUG("ctrlckv is NULL");
      return UPLL_RC_ERR_GENERIC;
    }
    // Create/Update/Delete a record in CANDIDATE DB
    result_code = UpdateConfigDB(ctrlckv, UPLL_DT_CANDIDATE, op, dmi, CTRLRTBL);
    if (UPLL_RC_SUCCESS != result_code) {
        if (ctrlckv != NULL) delete ctrlckv;
        UPLL_LOG_DEBUG("Err while updating in ctrlr table for candidateDb(%d)",
                       result_code);
       return result_code;
    } else {
     if (ctrlckv != NULL) delete ctrlckv;
    }
    ikey = ikey->get_next_cfg_key_val();
  }
  UPLL_LOG_DEBUG("Successful completion of the controller table updation");

  return result_code;
}

upll_rc_t VtnFlowFilterMoMgr::GetControllerKeyval(ConfigKeyVal *&ctrlckv,
                                               ConfigKeyVal *&ikey,
                                               controller_domain *ctrlrdom) {
  UPLL_FUNC_TRACE;
  key_vtn_flowfilter_t *flowfilter_key =
                  reinterpret_cast<key_vtn_flowfilter_t*>(ikey->get_key());
  ctrlckv = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER, IpctSt::kIpcInvalidStNum,
                             flowfilter_key, NULL);
  SET_USER_DATA_CTRLR_DOMAIN(ctrlckv, *ctrlrdom);
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtnFlowFilterMoMgr::IsReferenced(ConfigKeyVal *ikey,
                                            upll_keytype_datatype_t dt_type,
                                            DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  // no reference check is required for this function
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtnFlowFilterMoMgr::ValidateAttribute(ConfigKeyVal *ikey,
                                                DalDmlIntf *dmi,
                                                IpcReqRespHeader *req) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *okey = NULL;
  if (!ikey || !ikey->get_key()) {
    UPLL_LOG_DEBUG("input key is null");
    return UPLL_RC_ERR_GENERIC;
  }
  key_vtn_flowfilter_t *key_vtn_ff =
      reinterpret_cast<key_vtn_flowfilter_t *>(ikey->get_key());
  MoMgrImpl *mgr = reinterpret_cast<MoMgrImpl *>
      (const_cast<MoManager *>(GetMoManager(UNC_KT_VTN)));

  result_code = mgr->GetChildConfigKey(okey, NULL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Key not supported result - %d", result_code);
    return result_code;
  }

  key_vtn_t *vtn_key = reinterpret_cast<key_vtn_t *>(okey->get_key());
  uuu::upll_strncpy(vtn_key->vtn_name, key_vtn_ff->vtn_key.vtn_name,
                    (kMaxLenVtnName+1));

  /* Checks the given vtn exists or not */
  result_code = mgr->UpdateConfigDB(okey, req->datatype, UNC_OP_READ,
                                    dmi, MAINTBL);
  if (UPLL_RC_ERR_INSTANCE_EXISTS != result_code) {
    UPLL_LOG_DEBUG(" Parent VTN key does not exists");
    delete okey;
    return UPLL_RC_ERR_PARENT_DOES_NOT_EXIST;
  } else {
    result_code = UPLL_RC_SUCCESS;
  }

  delete okey;

  UPLL_LOG_DEBUG("ValidateAttribute Successfull.");
  return result_code;
}

upll_rc_t VtnFlowFilterMoMgr::RenameMo(IpcReqRespHeader *req,
                                       ConfigKeyVal *ikey, DalDmlIntf *dmi,
                                       const char *ctrlr_id) {
  UPLL_FUNC_TRACE;
  return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_KT;
}

upll_rc_t VtnFlowFilterMoMgr::GetChildConfigKey(ConfigKeyVal *&okey,
                                                ConfigKeyVal *parent_key) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  void *pkey = NULL;
  key_vtn_flowfilter_t *vtn_ff_key;

  if (parent_key == NULL) {
    vtn_ff_key = reinterpret_cast<key_vtn_flowfilter_t*>
        (ConfigKeyVal::Malloc(sizeof(key_vtn_flowfilter_t)));
    okey = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER,
                            IpctSt::kIpcStKeyVtnFlowfilter,
                            vtn_ff_key, NULL);
    UPLL_LOG_DEBUG("Parent Key Filled %d", result_code);
    return UPLL_RC_SUCCESS;
  } else {
    pkey = parent_key->get_key();
  }
  if (pkey == NULL) {
    UPLL_LOG_DEBUG("Parent Key is NULL");
    return UPLL_RC_ERR_GENERIC;
  }

  if (okey) {
    if (okey->get_key_type() != UNC_KT_VTN_FLOWFILTER)
      return UPLL_RC_ERR_GENERIC;
    vtn_ff_key = reinterpret_cast<key_vtn_flowfilter_t *>
        (okey->get_key());
  } else {
    vtn_ff_key = reinterpret_cast<key_vtn_flowfilter_t *>
        (ConfigKeyVal::Malloc(sizeof(key_vtn_flowfilter_t)));
  }
  switch (parent_key->get_key_type()) {
    case UNC_KT_VTN:
      uuu::upll_strncpy(vtn_ff_key->vtn_key.vtn_name,
                        reinterpret_cast<key_vtn_t*>
                        (pkey)->vtn_name, (kMaxLenVtnName + 1));
      break;
    case UNC_KT_VTN_FLOWFILTER:
      uuu::upll_strncpy(vtn_ff_key->vtn_key.vtn_name,
                        reinterpret_cast<key_vtn_flowfilter_t*>
                        (pkey)->vtn_key.vtn_name, (kMaxLenVtnName + 1));
      vtn_ff_key->input_direction =
          reinterpret_cast<key_vtn_flowfilter_t *>(pkey)->input_direction;
      break;
    default:
      if (vtn_ff_key) free(vtn_ff_key);
      return UPLL_RC_ERR_GENERIC;
  }

  if (!okey)
    okey = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER,
                            IpctSt::kIpcStKeyVtnFlowfilter,
                            vtn_ff_key, NULL);

  SET_USER_DATA(okey, parent_key);
  UPLL_LOG_DEBUG("Okey filled Succesfully %d", result_code);
  return result_code;
}

upll_rc_t VtnFlowFilterMoMgr::GetRenamedControllerKey(
    ConfigKeyVal *&ikey, upll_keytype_datatype_t dt_type, DalDmlIntf *dmi,
    controller_domain *ctrlr_dom) {
  UPLL_FUNC_TRACE;
  ConfigKeyVal *okey = NULL;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  uint8_t rename = 0;
  // Check if VTN is renamed on the controller by getting VTN object
  MoMgrImpl *mgr = static_cast<MoMgrImpl*>((const_cast<MoManager*>
                                    (GetMoManager(UNC_KT_VTN))));
  mgr->GetChildConfigKey(okey, NULL);
  //  mgr->IsRenamed(ikey, dt_type, dmi, rename);
  IsRenamed(ikey, dt_type, dmi, rename);
  if (!rename) {
    delete okey;
    return UPLL_RC_SUCCESS;
  }
  /* Vtn renamed */
  if (rename & VTN_RENAME) {
    if (ctrlr_dom)
      SET_USER_DATA_CTRLR_DOMAIN(okey, *ctrlr_dom);
    // Copy the input VTN Name into the Okey and send it for rename check IN db
    uuu::upll_strncpy(
                      reinterpret_cast<key_vtn *>(okey->get_key())->vtn_name,
                      reinterpret_cast<key_vtn_flowfilter_t *>
                      (ikey->get_key())->vtn_key.vtn_name,
                      (kMaxLenVtnName + 1));
    DbSubOp dbop = { kOpReadSingle, kOpMatchCtrlr, kOpInOutFlag };
     /* ctrlr_name */
    result_code = mgr->ReadConfigDB(okey, dt_type, UNC_OP_READ,
                                          dbop, dmi, RENAMETBL);
    if (result_code != UPLL_RC_SUCCESS) {  // COV CHECKED RETURN
      return result_code;
    }
     // NULL Checks Missing
    val_rename_vtn *rename_val = reinterpret_cast <val_rename_vtn *>
                     (GetVal(okey));
    if (!rename_val
        || (rename_val->valid[UPLL_IDX_NEW_NAME_RVTN] != UNC_VF_VALID))
      return UPLL_RC_ERR_GENERIC;
    uuu::upll_strncpy(
                      reinterpret_cast<key_vtn_flowfilter_t*>
                      (ikey->get_key())->vtn_key.vtn_name,
                      rename_val->new_name,
                      (kMaxLenVtnName + 1));
    SET_USER_DATA_FLAGS(ikey, VTN_RENAME);
  }

  UPLL_LOG_DEBUG("GetRenamedControllerKey  Success");
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtnFlowFilterMoMgr::GetRenamedUncKey(ConfigKeyVal *ikey,
                                               upll_keytype_datatype_t dt_type,
                                               DalDmlIntf *dmi,
                                               uint8_t *ctrlr_id) {
  UPLL_FUNC_TRACE;
  ConfigKeyVal *unc_key = NULL;
  DbSubOp dbop = { kOpReadSingle, kOpMatchCtrlr, kOpInOutNone };
  key_vtn *rename_vtn_key = reinterpret_cast<key_vtn*>
      (ConfigKeyVal::Malloc(sizeof(key_vtn)));
  key_vtn_flowfilter_t *ctrlr_key =
      reinterpret_cast<key_vtn_flowfilter_t *>(ikey->get_key());
  uuu::upll_strncpy(rename_vtn_key->vtn_name,
                    ctrlr_key->vtn_key.vtn_name,
                    (kMaxLenVtnName + 1));
  MoMgrImpl *mgr = static_cast<MoMgrImpl*>((const_cast<MoManager*>(GetMoManager(
      UNC_KT_VTN))));
  mgr->GetChildConfigKey(unc_key, NULL);
  if (ctrlr_id == NULL) {
    if (unc_key !=NULL) {
      delete unc_key;
    }
    free(rename_vtn_key);
    UPLL_LOG_DEBUG("ctrlr_id is NULL ");
    return UPLL_RC_ERR_GENERIC;
  }
  SET_USER_DATA_CTRLR(unc_key, ctrlr_id);
  unc_key->AppendCfgVal(IpctSt::kIpcStValRenameVtn, rename_vtn_key);
  upll_rc_t result_code = mgr->ReadConfigDB(unc_key, dt_type, UNC_OP_READ, dbop,
                                            dmi, RENAMETBL);
  if (result_code == UPLL_RC_SUCCESS) {
    key_vtn *vtn_key = reinterpret_cast<key_vtn *>(unc_key->get_key());
    uuu::upll_strncpy(ctrlr_key->vtn_key.vtn_name,
                      vtn_key->vtn_name,
                      (kMaxLenVtnName + 1));
  }
  free(rename_vtn_key);  //  ADDED
  UPLL_LOG_DEBUG("GetRenamedUncKey is Success :: result_code %d", result_code);
  return result_code;
}

upll_rc_t VtnFlowFilterMoMgr::DupConfigKeyVal(ConfigKeyVal *&okey,
                                              ConfigKeyVal *&req,
                                              MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  val_flowfilter_controller_t *flowfilter_ctrlr_val = NULL;
  val_flowfilter_t *flowfilter_val = NULL;
  if (req == NULL) {
    UPLL_LOG_DEBUG("In sufficient parameters");
    return UPLL_RC_ERR_GENERIC;
  }
  if (okey != NULL) {
    UPLL_LOG_DEBUG("oKey already Contains Data");
    return UPLL_RC_ERR_GENERIC;
  }
  if (req->get_key_type() != UNC_KT_VTN_FLOWFILTER) {
    UPLL_LOG_DEBUG(" Invalid KeyType.");
  return UPLL_RC_ERR_GENERIC;
  }
  ConfigVal *tmp1 = NULL, *tmp = (req)->get_cfg_val();

  if (tmp) {
    if (tbl == MAINTBL) {
      val_flowfilter_t *ival = reinterpret_cast<val_flowfilter_t*>(GetVal(req));
      if (NULL != ival) {
           flowfilter_val =
           reinterpret_cast<val_flowfilter_t *>
           (ConfigKeyVal::Malloc(sizeof(val_flowfilter_t)));
        memcpy(flowfilter_val, ival, sizeof(val_flowfilter_t));
        tmp1 = new ConfigVal(IpctSt::kIpcStValFlowfilter, flowfilter_val);
      }
    } else if (tbl == CTRLRTBL) {
      val_flowfilter_controller_t *ival =
          reinterpret_cast<val_flowfilter_controller_t *>(GetVal(req));
      if (NULL != ival) {
           flowfilter_ctrlr_val =
           reinterpret_cast<val_flowfilter_controller_t *>
           (ConfigKeyVal::Malloc(sizeof(val_flowfilter_controller_t)));
        memcpy(flowfilter_ctrlr_val, ival, sizeof(val_flowfilter_controller_t));
        tmp1 = new ConfigVal(IpctSt::kIpcStValFlowfilter,
                             flowfilter_ctrlr_val);
        UPLL_LOG_DEBUG("Creation of Duplicate ConfigVal is successful");
      }
  }
  }

  if (tmp1) {
    tmp1->set_user_data(tmp->get_user_data());
  }
  void *tkey = (req != NULL) ? (req)->get_key() : NULL;
  key_vtn_flowfilter_t *ikey = reinterpret_cast<key_vtn_flowfilter_t *>(tkey);

  key_vtn_flowfilter_t *vtn_flowfilterkey =
        reinterpret_cast<key_vtn_flowfilter_t*>
        (ConfigKeyVal::Malloc(sizeof(key_vtn_flowfilter_t)));
  memcpy(vtn_flowfilterkey, ikey, sizeof(key_vtn_flowfilter_t));
  okey = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER, IpctSt::kIpcStKeyVtnFlowfilter,
                          vtn_flowfilterkey, tmp1);
  SET_USER_DATA(okey, req);
  UPLL_LOG_DEBUG(" Creation of Duplicate ConfigKeyVal is Successful ");
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtnFlowFilterMoMgr::SetConsolidatedStatus(ConfigKeyVal *ikey,
                                                    DalDmlIntf *dmi) {
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *ckv = NULL;
  DbSubOp dbop = {kOpReadMultiple, kOpMatchNone, kOpInOutNone};
  if (ikey == NULL) {
    return UPLL_RC_ERR_GENERIC;
  }
  result_code = GetChildConfigKey(ckv, ikey);
  if (UPLL_RC_SUCCESS != result_code) {
    return result_code;
  }
  result_code = ReadConfigDB(ckv, UPLL_DT_RUNNING,
                             UNC_OP_READ, dbop, dmi, CTRLRTBL);
  if (UPLL_RC_SUCCESS != result_code) {
    delete ckv;
    return result_code;
  }
  std::list< unc_keytype_configstatus_t > list_cs_row;
  val_flowfilter_t *val;
  for ( ; ckv != NULL ; ckv = ckv->get_next_cfg_key_val()) {
      val = reinterpret_cast<val_flowfilter_t *>(GetVal(ckv));
      list_cs_row.push_back((unc_keytype_configstatus_t)val->cs_row_status);
  }
  val_flowfilter_t *val_temp =
                   reinterpret_cast<val_flowfilter_t *>(GetVal(ikey));
  val_temp->cs_row_status = GetConsolidatedCsStatus(list_cs_row);
  result_code = UpdateConfigDB(ikey, UPLL_DT_RUNNING,
                               UNC_OP_UPDATE, dmi, MAINTBL);
  if (UPLL_RC_SUCCESS != result_code) {
    return result_code;
  }
  return result_code;
}
upll_rc_t VtnFlowFilterMoMgr::UpdateAuditConfigStatus(
                    unc_keytype_configstatus_t cs_status,
                    uuc::UpdateCtrlrPhase phase,
                    ConfigKeyVal *&ckv_running) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  val_flowfilter_t *val;
  val = (ckv_running != NULL)?
        reinterpret_cast<val_flowfilter_t *>(GetVal(ckv_running)):NULL;
  if (NULL == val) {
    return UPLL_RC_ERR_GENERIC;
  }
  if (uuc::kUpllUcpCreate == phase )
     val->cs_row_status = cs_status;
  return result_code;
}

upll_rc_t VtnFlowFilterMoMgr::UpdateConfigStatus(
    ConfigKeyVal *vtn_flow_filter_key, unc_keytype_operation_t op,
    uint32_t driver_result, ConfigKeyVal *nreq, DalDmlIntf *dmi,
    ConfigKeyVal *ctrlr_key) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  unc_keytype_configstatus_t status = UNC_CS_UNKNOWN,
      cs_status = UNC_CS_UNKNOWN;  // TODO(UNC) :initialize valuefor compilation
  cs_status = (driver_result == 0) ? UNC_CS_APPLIED : UNC_CS_NOT_APPLIED;
  val_flowfilter_t *vtn_flowfilter_val =
      reinterpret_cast<val_flowfilter_t *>(GetVal(vtn_flow_filter_key));
  val_vtn_flowfilter_ctrlr *ctrlr_val_vtn_flowfilter =
      reinterpret_cast<val_vtn_flowfilter_ctrlr *>(GetVal(ctrlr_key));
  if ((vtn_flowfilter_val == NULL) || (ctrlr_val_vtn_flowfilter == NULL)) {
    UPLL_LOG_DEBUG("vrtif_flowfilter_entry_val &"
                  "ctrlr_val_vtn_flowfilter is Null");
  return UPLL_RC_ERR_GENERIC;
  }
  if (op == UNC_OP_CREATE) {
    /*
     ctrlr_val_vtn->oper_status =  UPLL_OPER_STATUS_UNKNOWN;
     ctrlr_val_vtn->alarm_status =  UPLL_ALARM_CLEAR;
     ctrlr_val_vtn->valid[0] = UNC_VF_INVALID;
     ctrlr_val_vtn->valid[1] = UNC_VF_INVALID;
     */
    /* update the vtn status in main tbl */
    switch (vtn_flowfilter_val->cs_row_status) {
      case UNC_CS_UNKNOWN:
        status = cs_status;
        break;
      case UNC_CS_PARTAILLY_APPLIED:
        if (ctrlr_val_vtn_flowfilter->cs_row_status == UNC_CS_NOT_APPLIED) {
          // if this vtn has caused it then to change to applied.
        }
        break;
      case UNC_CS_APPLIED:
      case UNC_CS_NOT_APPLIED:
      case UNC_CS_INVALID:
      default:
        status =
            (cs_status == UNC_CS_APPLIED) ? UNC_CS_PARTAILLY_APPLIED : status;
        break;
    }
    /* TODO - determine how to set UNC_CS_NOT_SUPPORTED) */
    if (ctrlr_val_vtn_flowfilter->cs_row_status != UNC_CS_NOT_SUPPORTED)
      ctrlr_val_vtn_flowfilter->cs_row_status = cs_status;
    vtn_flowfilter_val->cs_row_status = status;
    /*
     for ( unsigned int loop = 0; loop < sizeof(vtn_flowfilter_val->valid)/sizeof(char); ++loop ) {
     if ( (UNC_VF_VALID == vtn_flowfilter_val->valid[loop]) ||
     (UNC_VF_VALID_NO_VALUE == vtn_flowfilter_val->valid[loop]))
     if (ctrlr_val_vtn_flowfilter->cs_attr[loop] != UNC_CS_NOT_SUPPORTED) {
     ctrlr_val_vtn_flowfilter->cs_attr[loop] = cs_status;
     vtn_flowfilter_val->cs_attr[loop] = (uint8_t)vtn_flowfilter_val->cs_row_status;
     }
     }*/
  } else if (op == UNC_OP_UPDATE) {
    if (ctrlr_val_vtn_flowfilter->cs_row_status != UNC_CS_NOT_SUPPORTED)
      ctrlr_val_vtn_flowfilter->cs_row_status = cs_status;
    vtn_flowfilter_val->cs_row_status = status;

    // for ( unsigned int loop = 0; loop < sizeof(vtn_flowfilter_val->valid)
    // /sizeof(char); ++loop ) {
    //  if (ctrlr_val_vtn_flowfilter->cs_attr[loop] != UNC_CS_NOT_SUPPORTED)
    //   if ( (UNC_VF_VALID == vtn_flowfilter_val->valid[loop]) ||
    //      (UNC_VF_VALID_NO_VALUE == vtn_flowfilter_val->valid[loop]))
#if 0
    if (CompareVal(vtn_val, nreq->GetVal(), loop)) {
#endif
    // ctrlr_val_vtn_flowfilter->cs_attr[loop] = cs_status;
    // vtn_flowfilter_val->cs_attr[loop] = cs_status;
#if 0
  }
#endif
  }
  UPLL_LOG_DEBUG("UpdateConfigStatus is  Successfull");
  return result_code;
}

upll_rc_t VtnFlowFilterMoMgr::TxUpdateController(unc_key_type_t keytype,
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
  ctrlr_dom.ctrlr = NULL;
  ctrlr_dom.domain = NULL;
  DalCursor *dal_cursor_handle = NULL;
  IpcResponse resp;

  unc_keytype_operation_t op = (phase == uuc::kUpllUcpCreate)?UNC_OP_CREATE:
      ((phase == uuc::kUpllUcpUpdate)?UNC_OP_UPDATE:
       ((phase == uuc::kUpllUcpDelete)?UNC_OP_DELETE:UNC_OP_INVALID));
  switch (op) {
    case UNC_OP_CREATE:
    case UNC_OP_DELETE:
      result_code = DiffConfigDB(UPLL_DT_CANDIDATE, UPLL_DT_RUNNING,
                                 op, req, nreq, &dal_cursor_handle,
                                 dmi, CTRLRTBL);
      break;
    case UNC_OP_UPDATE:
      // not supported by keytype
      // return success
      UPLL_LOG_DEBUG(" Not supported operation");
      return UPLL_RC_SUCCESS;
    default:
      UPLL_LOG_DEBUG("Invalid operation");
      return UPLL_RC_ERR_GENERIC;
  }
  resp.header.clnt_sess_id = session_id;
  resp.header.config_id = config_id;
  while (result_code == UPLL_RC_SUCCESS) {
    // Get Next Record
    dal_result = dmi->GetNextRecord(dal_cursor_handle);
    result_code = DalToUpllResCode(dal_result);
    if (result_code != UPLL_RC_SUCCESS) {
      break;
    }
    ck_main = NULL;
    if ( (op == UNC_OP_CREATE) || (op == UNC_OP_DELETE) ) {
      result_code = DupConfigKeyVal(ck_main, req, MAINTBL);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG(" DupConfigKeyVal failed err code(%d)", result_code);
        return result_code;
      }
      GET_USER_DATA_CTRLR_DOMAIN(ck_main, ctrlr_dom);
      UPLL_LOG_DEBUG("ctrlr : %s; domain : %s", ctrlr_dom.ctrlr,
                     ctrlr_dom.domain);
      if (NULL == ctrlr_dom.ctrlr || NULL == ctrlr_dom.domain) {
        UPLL_LOG_INFO("Invalid controller/domain");
        result_code = UPLL_RC_ERR_GENERIC;
        if (ck_main) delete ck_main;
        break;
      }
      result_code = TxUpdateProcess(ck_main, &resp, op, dmi, &ctrlr_dom);
      if (result_code == UPLL_RC_SUCCESS) {
        affected_ctrlr_set->insert((const char *)ctrlr_dom.ctrlr);
      } else {
        UPLL_LOG_DEBUG("TxUpdateProcess error %d", result_code);
        *err_ckv = resp.ckv_data;
        if (ck_main) delete ck_main;
        break;
      }
    }
    if (ck_main) {
      delete ck_main;
      ck_main = NULL;
    }
  }
  if (nreq)
    delete nreq;
  if (req)
    delete req;
  if (dal_cursor_handle) {
    dmi->CloseCursor(dal_cursor_handle, true);
    dal_cursor_handle = NULL;
  }
  result_code = (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) ?
      UPLL_RC_SUCCESS:result_code;
  return result_code;
}

upll_rc_t VtnFlowFilterMoMgr::TxUpdateProcess(ConfigKeyVal *ck_main,
                                              IpcResponse *ipc_resp,
                                              unc_keytype_operation_t op,
                                              DalDmlIntf *dmi,
                                              controller_domain *ctrlr_dom) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code;
  /* read from main table */
  ConfigKeyVal *dup_ckmain = ck_main;
  if (op == UNC_OP_CREATE) {
    dup_ckmain = NULL;
    result_code = GetChildConfigKey(dup_ckmain, ck_main);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Returning error %d\n", result_code);
      if (dup_ckmain) delete dup_ckmain;
      return result_code;
    }
    DbSubOp dbop = {kOpReadSingle, kOpMatchNone, kOpInOutCs};
    result_code = ReadConfigDB(dup_ckmain, UPLL_DT_CANDIDATE,
                               UNC_OP_READ, dbop, dmi, MAINTBL);
    if (result_code != UPLL_RC_SUCCESS) {
      string s(dup_ckmain->ToStrAll());
      UPLL_LOG_DEBUG("%s vtn flowfilter read failed from candidatedb (%d)",
                     s.c_str(), result_code);
      delete dup_ckmain;
      return result_code;
    }
  }
  /* Get renamed key if key is renamed */
  result_code =  GetRenamedControllerKey(dup_ckmain, UPLL_DT_CANDIDATE,
                                         dmi, ctrlr_dom);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Failed to get the Renamed ControllerKey");
    return result_code;
  }
  result_code = SendIpcReq(ipc_resp->header.clnt_sess_id,
                           ipc_resp->header.config_id, op,
                           UPLL_DT_CANDIDATE,
                           dup_ckmain, ctrlr_dom, ipc_resp);

  if (result_code == UPLL_RC_ERR_RESOURCE_DISCONNECTED) {
    result_code = UPLL_RC_SUCCESS;
    UPLL_LOG_DEBUG("controller disconnected error proceed with commit");
  }
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("IpcSend failed %d\n", result_code);
  }
  if ((op == UNC_OP_CREATE) && dup_ckmain) {
    delete dup_ckmain;
    dup_ckmain = NULL;
  }
  return result_code;
}


upll_rc_t VtnFlowFilterMoMgr::TxCopyCandidateToRunning(
    unc_key_type_t keytype, CtrlrCommitStatusList *ctrlr_commit_status,
    DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  DalResultCode db_result;
  // UNC_OP_UPDATE operation is not supported for this keytype
  unc_keytype_operation_t op[] = { UNC_OP_DELETE, UNC_OP_CREATE };

  int nop = sizeof(op) / sizeof(op[0]);
  ConfigKeyVal *vtn_flowfilter_key = NULL, *req = NULL, *nreq = NULL;
  DalCursor *cfg1_cursor;
  uint8_t *ctrlr_id =NULL;
  map<string, int> ctrlr_result;
  CtrlrCommitStatusList::iterator ccsListItr;
  CtrlrCommitStatus *ccStatusPtr;

  if ((ctrlr_commit_status == NULL) || (dmi == NULL)) {
    result_code = UPLL_RC_ERR_GENERIC;
    UPLL_LOG_DEBUG("Insufficient inputs:result_code %d", result_code);
    return result_code;
  }
  for (ccsListItr = ctrlr_commit_status->begin();
      ccsListItr != ctrlr_commit_status->end(); ++ccsListItr) {
    ccStatusPtr = *ccsListItr;
    ctrlr_id = reinterpret_cast<uint8_t* >(const_cast<char*>
                               (ccStatusPtr->ctrlr_id.c_str()));
    ctrlr_result[ccStatusPtr->ctrlr_id] = ccStatusPtr->upll_ctrlr_result;
    if (ccStatusPtr->upll_ctrlr_result != UPLL_RC_SUCCESS) {
      for (ConfigKeyVal *ck_err = ccStatusPtr->err_ckv; ck_err != NULL;
           ck_err = ck_err->get_next_cfg_key_val()) {
        if (ck_err->get_key_type() != keytype) continue;
        result_code = GetRenamedUncKey(ck_err, UPLL_DT_CANDIDATE, dmi,
                                       ctrlr_id);
        if (result_code != UPLL_RC_SUCCESS) {
         UPLL_LOG_DEBUG("Unable to get the Renamed Unc Key, resultcode=%d",
                        result_code);
          return result_code;
        }
      }
    }
  }

  for (int i = 0; i < nop; i++) {
    // Update the Main table
    result_code = DiffConfigDB(UPLL_DT_CANDIDATE, UPLL_DT_RUNNING, op[i],
                               req, nreq, &cfg1_cursor, dmi, MAINTBL);
    while (result_code == UPLL_RC_SUCCESS) {
      db_result = dmi->GetNextRecord(cfg1_cursor);
      result_code = DalToUpllResCode(db_result);
      if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
        result_code = UPLL_RC_SUCCESS;
        break;
      }
      result_code = UpdateMainTbl(req, op[i], UPLL_RC_SUCCESS,
                                  nreq, dmi);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Updating Main table Error %d", result_code);
        return result_code;
      }
    }
    if (cfg1_cursor) {
      dmi->CloseCursor(cfg1_cursor, true);
      cfg1_cursor = NULL;
    }
    if (req) {
      delete req;
      req = NULL;
    }
    UPLL_LOG_DEBUG("Updating main table complete with op %d", op[i]);
  }

  for (int i = 0; i < nop; i++) {
    // Update the controller table
    result_code = DiffConfigDB(UPLL_DT_CANDIDATE, UPLL_DT_RUNNING, op[i], req,
                               nreq, &cfg1_cursor, dmi, CTRLRTBL);
    ConfigKeyVal *vtn_ff_ctrlr_key = NULL;
    while (result_code == UPLL_RC_SUCCESS) {
      db_result = dmi->GetNextRecord(cfg1_cursor);
      result_code = DalToUpllResCode(db_result);
      if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
        result_code = UPLL_RC_SUCCESS;
        break;
      }
      if (op[i] == UNC_OP_CREATE) {
        DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutFlag };
        result_code = GetChildConfigKey(vtn_flowfilter_key, req);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("GetChildConfigKey is failed resultcode=%d",
                         result_code);
          return result_code;
        }
        result_code = ReadConfigDB(vtn_flowfilter_key, UPLL_DT_CANDIDATE,
                                   UNC_OP_READ, dbop, dmi, MAINTBL);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("Unable to read configuration from CandidateDb");
          delete vtn_flowfilter_key;
          return result_code;
        }

        result_code = DupConfigKeyVal(vtn_ff_ctrlr_key, req, CTRLRTBL);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("Failed to create duplicate ConfigKeyVal Err (%d)",
                         result_code);
          delete vtn_flowfilter_key;
          return result_code;
        }
        GET_USER_DATA_CTRLR(vtn_ff_ctrlr_key, ctrlr_id);
        string controller(reinterpret_cast<char *> (ctrlr_id));
        result_code = UpdateConfigStatus(vtn_flowfilter_key, op[i],
                                         ctrlr_result[controller], NULL,
                                         dmi, vtn_ff_ctrlr_key);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("Error in updating configstatus, resultcode=%d",
                         result_code);
          delete vtn_flowfilter_key;
          return result_code;
        }
      } else if (op[i] == UNC_OP_DELETE) {
        result_code = GetChildConfigKey(vtn_ff_ctrlr_key, req);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("Error in getting the configkey, resultcode=%d",
                         result_code);
          return result_code;
        }
      }
      result_code = UpdateConfigDB(vtn_ff_ctrlr_key, UPLL_DT_RUNNING,
                                   op[i], dmi, CTRLRTBL);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("DB Error while updating controller table. err code:%d",
                       result_code);
        delete vtn_ff_ctrlr_key;
        return result_code;
      }
      // update the consolidated config status in the Main Table
      if (op[i] != UNC_OP_DELETE) {
            result_code = UpdateConfigDB(vtn_flowfilter_key, UPLL_DT_RUNNING,
                UNC_OP_UPDATE, dmi, MAINTBL);
            if (result_code != UPLL_RC_SUCCESS)
              return result_code;
          }
      EnqueCfgNotification(op[i], UPLL_DT_RUNNING, vtn_ff_ctrlr_key);
      if (vtn_flowfilter_key)
        delete vtn_flowfilter_key;
      vtn_flowfilter_key = vtn_ff_ctrlr_key = NULL;
      result_code = DalToUpllResCode(db_result);
    }
    if (cfg1_cursor) {
      dmi->CloseCursor(cfg1_cursor, true);
      cfg1_cursor = NULL;
    }
    if (req) delete req;
    if (nreq) delete nreq;
    nreq = req = NULL;
  }
  result_code = (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) ?
      UPLL_RC_SUCCESS:result_code;
  return result_code;
}

upll_rc_t VtnFlowFilterMoMgr::AllocVal(ConfigVal *&ck_val,
                                       upll_keytype_datatype_t dt_type,
                                       MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  void *val;
  if (ck_val != NULL) return UPLL_RC_ERR_GENERIC;
  switch (tbl) {
    case MAINTBL:
      // TODO(UNC) : Need to handle the datatype as DT_STATE case
      val = reinterpret_cast<void *>
          (ConfigKeyVal::Malloc(sizeof(val_flowfilter_t)));
      ck_val = new ConfigVal(IpctSt::kIpcStValFlowfilter, val);
      break;
    case CTRLRTBL:
      val = reinterpret_cast<void *>
          (ConfigKeyVal::Malloc(sizeof(val_flowfilter_controller_t)));
      ck_val = new ConfigVal(IpctSt::kIpcStValFlowfilterController, val);
      break;

    default:
      val = NULL;
      return UPLL_RC_ERR_GENERIC;
  }
  UPLL_LOG_DEBUG("AllocVal Success");
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtnFlowFilterMoMgr::ReadMo(IpcReqRespHeader *req, ConfigKeyVal *ikey,
                                     DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  controller_domain ctrlr_dom;
  result_code = ValidateMessage(req, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("ValidateMessage failed result_code %d",
                    result_code);
      return result_code;
  }
  result_code = ReadInfoFromDB(req, ikey, dmi, &ctrlr_dom);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG(" Read request failed result(%d)", result_code);
  }
  return result_code;

  // UPLL_LOG_DEBUG("ReadMo  Success");
  // return ReadRecord(req, ikey, dmi);
}

upll_rc_t VtnFlowFilterMoMgr::ReadRecord(IpcReqRespHeader *req,
                                         ConfigKeyVal *ikey, DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_ERR_GENERIC;

  DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutCs };

  result_code = ValidateMessage(req, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Validate Message failed result(%d)", result_code);
    return result_code;
  }

  switch (req->datatype) {
    // Retrieving config information
    case UPLL_DT_CANDIDATE:
    case UPLL_DT_RUNNING:
    case UPLL_DT_STARTUP:
    case UPLL_DT_STATE:
      if (req->operation == UNC_OP_READ) {
        result_code = ReadConfigDB(ikey, req->datatype, req->operation,
                                   dbop, dmi, MAINTBL);
      } else {
        if ((req->operation == UNC_OP_READ_SIBLING_BEGIN) ||
            (req->operation == UNC_OP_READ_SIBLING)) {
          dbop.readop = kOpReadMultiple;
        }
        result_code = ReadConfigDB(ikey, req->datatype, req->operation,
                                   dbop, req->rep_count, dmi, MAINTBL);
      }
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG(" Read Recored failed for operation (%d)",
                       req->operation);
        return result_code;
      }
      break;
    default:
      return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT;
  }  // end of switch
  UPLL_LOG_DEBUG("ReadRecord Success");
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtnFlowFilterMoMgr::ReadSiblingMo(IpcReqRespHeader *req,
                                            ConfigKeyVal *key, bool begin,
                                            DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  controller_domain ctrlr_dom;
  result_code = ValidateMessage(req, key);
  if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("ValidateMessage failed result_code %d",
                    result_code);
      return result_code;
  }
  result_code = ReadInfoFromDB(req, key, dmi, &ctrlr_dom);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG(" Read request failed result(%d)", result_code);
  }
  return result_code;

  // UPLL_LOG_DEBUG("ReadSiblingMo  Success");
  // return ReadRecord(req, key, dmi);
}

bool VtnFlowFilterMoMgr::GetRenameKeyBindInfo(unc_key_type_t key_type,
                                    BindInfo *&bindinfo,
                                    int &nattr,
                                    MoMgrTables tbl ) {
  /* Main Table only update */
  UPLL_FUNC_TRACE;
  nattr = NUM_KEY_MAIN_TBL_;
  if (MAINTBL == tbl) {
    bindinfo = vtn_flowfilter_maintbl_rename_bindinfo;
  } else if (CTRLRTBL ==tbl) {
     bindinfo = vtn_flowfilter_ctrlrtbl_rename_bindinfo;
  }
  UPLL_LOG_DEBUG("Successful Completeion");
  return PFC_TRUE;
}

bool VtnFlowFilterMoMgr::IsValidKey(void *key, uint64_t index) {
  UPLL_FUNC_TRACE;
  key_vtn_flowfilter_t *vtn_ff_key =
    reinterpret_cast<key_vtn_flowfilter_t*> (key);
  upll_rc_t ret_val;
  switch (index) {
    case uudst::vtn_flowfilter::kDbiVtnName:
      ret_val = ValidateKey(reinterpret_cast<char *>
                            (vtn_ff_key->vtn_key.vtn_name),
                            kMinLenVtnName,
                            kMaxLenVtnName);
      if (ret_val != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("VTN Name is not valid(%d)", ret_val);
        return false;
      }
    break;
    case uudst::vtn_flowfilter::kDbiInputDirection:
      if (vtn_ff_key->input_direction == 0xFE) {
        // if operation is read sibling begin or
        // read sibling count return false
        // for output binding
        vtn_ff_key->input_direction = 0;
        return false;
      } else {
        // do normal validation.
        if (!ValidateNumericRange(vtn_ff_key->input_direction,
                                  (uint8_t) UPLL_FLOWFILTER_DIR_IN,
                                  (uint8_t) UPLL_FLOWFILTER_DIR_OUT,
                                  true, true)) {
          UPLL_LOG_DEBUG(" input direction syntax validation failed ");
          return false;
        }
      }
    break;
    default:
    break;
  }
  return true;
}
upll_rc_t  VtnFlowFilterMoMgr::CopyToConfigKey(ConfigKeyVal *&okey,
                                              ConfigKeyVal *ikey) {
  UPLL_FUNC_TRACE;
  if (!ikey || !(ikey->get_key())) {
    UPLL_LOG_DEBUG("Input Key Not Valid");
    return UPLL_RC_ERR_GENERIC;
  }

  upll_rc_t result_code = UPLL_RC_SUCCESS;

  key_rename_vnode_info *key_rename = reinterpret_cast<key_rename_vnode_info *>
                                      (ikey->get_key());
  key_vtn_flowfilter_t *key_vtn =
               reinterpret_cast<key_vtn_flowfilter_t*>
               (ConfigKeyVal::Malloc(sizeof(key_vtn_flowfilter_t)));

  if (!strlen(reinterpret_cast<char *>(key_rename->old_unc_vtn_name))) {
    UPLL_LOG_DEBUG("String Length not Valid to Perform the Operation");
    free(key_vtn);
    return UPLL_RC_ERR_GENERIC;
  }

  uuu::upll_strncpy(key_vtn->vtn_key.vtn_name,
                    key_rename->old_unc_vtn_name, (kMaxLenVtnName + 1));
#if 0
  if (ikey->get_key_type() == table[MAINTBL]->get_key_type()) {
    if (!strlen(reinterpret_cast<char *> (key_rename->old_unc_vnode_name)))
      return UPLL_RC_ERR_GENERIC;
    strncpy(reinterpret_cast<char*>(key_vrt_if->flowfilter_key.if_key.vrt_key.\
      vrouter_name),
        reinterpret_cast<char *> (key_rename->old_unc_vnode_name), 32);
  }
#endif
  okey = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER, IpctSt::
                   kIpcStKeyVtnFlowfilter, key_vtn, NULL);
  if (!okey)
    return UPLL_RC_ERR_GENERIC;
  return result_code;
}
// Added
upll_rc_t VtnFlowFilterMoMgr::ValidateMessage(IpcReqRespHeader *req,
                                              ConfigKeyVal *key) {
  UPLL_FUNC_TRACE;
  upll_rc_t rt_code = UPLL_RC_ERR_GENERIC;

  if ((NULL == req) || (NULL == key)) {
    UPLL_LOG_DEBUG("IpcReqRespHeader/ConfigKeyval is NULL");
    return UPLL_RC_ERR_BAD_REQUEST;
  }

  if (UNC_KT_VTN_FLOWFILTER != key->get_key_type()) {
    UPLL_LOG_DEBUG(" Invalid keytype(%d)", key->get_key_type());
    return UPLL_RC_ERR_BAD_REQUEST;
  }

  if (req->option1 != UNC_OPT1_NORMAL) {
    UPLL_LOG_DEBUG(" invalid option1(%d)", req->option1);
    return UPLL_RC_ERR_INVALID_OPTION1;
  }
  if (req->option2 != UNC_OPT2_NONE) {
    UPLL_LOG_DEBUG(" invalid option2(%d)", req->option2);
    return UPLL_RC_ERR_INVALID_OPTION2;
  }

  key_vtn_flowfilter_t *key_vtn_ff = NULL;

  if (key->get_st_num() != IpctSt::kIpcStKeyVtnFlowfilter) {
    UPLL_LOG_DEBUG("Invalid key structure received.struct num - %d",
                   key->get_st_num());
    return UPLL_RC_ERR_BAD_REQUEST;
  }
  key_vtn_ff = reinterpret_cast<key_vtn_flowfilter_t *>(key->get_key());
  if (NULL == key_vtn_ff) {
    UPLL_LOG_DEBUG("Key structure is empty!!");
    return UPLL_RC_ERR_BAD_REQUEST;
  }

  if (UPLL_RC_SUCCESS != (rt_code = ValidateKey(
              reinterpret_cast<char*>(key_vtn_ff->vtn_key.vtn_name),
              kMinLenVtnName, kMaxLenVtnName))) {
    UPLL_LOG_DEBUG(" vtn flow-filter key validation failed ");
    return rt_code;
  }

  // TODO(AUTHOR) check if the VTN exists in the VTN MOMGR

  if ((req->operation != UNC_OP_READ_SIBLING_COUNT) &&
      (req->operation != UNC_OP_READ_SIBLING_BEGIN)) {
    /*
       Validate inputdirection is in the range specified in
       enum FlowFilter_Direction */
    if (!ValidateNumericRange(key_vtn_ff->input_direction,
                              (uint8_t) UPLL_FLOWFILTER_DIR_IN,
                              (uint8_t) UPLL_FLOWFILTER_DIR_OUT, true, true)) {
      UPLL_LOG_DEBUG(" input direction syntax validation failed ");
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  } else {
    // input direction should be not set for
    // sibling begin or count operation
    // as 0 or 1 are valid values setting an invalid value;
    key_vtn_ff->input_direction = 0xFE;
  }

  UPLL_LOG_TRACE(" key struct validation is success");
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtnFlowFilterMoMgr::ValidateCapability(IpcReqRespHeader *req,
                                   ConfigKeyVal *ikey,
                                   const char* ctrlr_name) {
  UPLL_FUNC_TRACE;
  // TODO(Author) added to bypass capability check
  return UPLL_RC_SUCCESS;
  // endTODO
  upll_rc_t result_code = UPLL_RC_ERR_GENERIC;

  if ((NULL == req) || (NULL == ikey)) {
    UPLL_LOG_DEBUG("IpcReqRespHeader/ConfigKeyval is NULL");
    return result_code;
  }

  if (!ctrlr_name) {
    UPLL_LOG_DEBUG(" Controller name is NULL ");
    return UPLL_RC_ERR_GENERIC;
  }

  uint32_t dt_type = req->datatype;
  uint32_t operation = req->operation;
  uint32_t option1 = req->option1;
  uint32_t option2 = req->option2;

  UPLL_LOG_TRACE("Controller - %s"
                 "dt_type: (%d) "
                 "operation: (%d) "
                 "option1: (%d) "
                 "option2: (%d) ",
                 ctrlr_name, dt_type, operation, option1, option2);

  bool ret_code = false;
  uint32_t instance_count = 0;
  const uint8_t *attrs = NULL;
  uint32_t max_attrs = 0;

  switch (operation) {
    case UNC_OP_CREATE: {
      ret_code = GetCreateCapability(ctrlr_name, ikey->get_key_type(),
                                        &instance_count, &max_attrs, &attrs);
      break;
    }
    case UNC_OP_UPDATE: {
      ret_code = GetUpdateCapability(ctrlr_name, ikey->get_key_type(),
                                        &max_attrs, &attrs);
      break;
    }
    default: {
      ret_code = GetReadCapability(ctrlr_name, ikey->get_key_type(),
                                      &max_attrs, &attrs);
    }
    break;
  }

  UPLL_LOG_DEBUG(" ret_code (%d)", ret_code);
  if (!ret_code) {
    UPLL_LOG_DEBUG("keytype(%d) is not supported by controller(%s) "
                   "for opearion(%d)",
                   ikey->get_key_type(), ctrlr_name, operation);
    return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_KT;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtnFlowFilterMoMgr::GetVtnControllerSpan(
    ConfigKeyVal *ikey,
    DalDmlIntf *dmi,
    std::list<controller_domain_t> &list_ctrlr_dom) {
  UPLL_FUNC_TRACE;
  ConfigKeyVal *okey = NULL;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  VtnMoMgr *mgr = static_cast<VtnMoMgr*>(const_cast<MoManager*>
                                           (GetMoManager(UNC_KT_VTN)));
  result_code = mgr->GetChildConfigKey(okey, NULL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Key not supported by controller");
    return result_code;
  }
  key_vtn_t *vtn_key = reinterpret_cast<key_vtn*>(okey->get_key());
  key_vtn_flowfilter_t *flowfilter_key =
      reinterpret_cast<key_vtn_flowfilter_t*>(ikey->get_key());
  uuu::upll_strncpy(vtn_key->vtn_name,
                    flowfilter_key->vtn_key.vtn_name,
                    (kMaxLenVtnName+1));
  result_code = mgr->GetControllerDomainSpan(okey, UPLL_DT_CANDIDATE,
                                             dmi,
                                             list_ctrlr_dom);
  if ((result_code != UPLL_RC_SUCCESS) &&
      (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE)) {
    UPLL_LOG_DEBUG("Error in getting controller span (%d)",
                   result_code);
  }
  delete okey;
  return result_code;
}

upll_rc_t VtnFlowFilterMoMgr::UpdateMainTbl(ConfigKeyVal *vtn_ff_key,
      unc_keytype_operation_t op, uint32_t driver_result,
      ConfigKeyVal *nreq, DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  ConfigKeyVal *ck_vtn_ff = NULL;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  val_flowfilter_t *val_ff = NULL;

  switch (op) {
    case UNC_OP_CREATE:
      result_code = DupConfigKeyVal(ck_vtn_ff, vtn_ff_key, MAINTBL);
      if (!ck_vtn_ff || (result_code != UPLL_RC_SUCCESS)) {
        UPLL_LOG_DEBUG("DupConfigKeyVal() Returning error %d\n", result_code);
        return result_code;
      }
      val_ff = reinterpret_cast<val_flowfilter_t *>(GetVal(ck_vtn_ff));
      if (!val_ff) {
        UPLL_LOG_DEBUG("invalid val \n");
        return UPLL_RC_ERR_GENERIC;
      }
      val_ff->cs_row_status = UNC_CS_NOT_APPLIED;
      break;
    case UNC_OP_DELETE:

      result_code = GetChildConfigKey(ck_vtn_ff, vtn_ff_key);
      if (!ck_vtn_ff || (result_code != UPLL_RC_SUCCESS)) {
        UPLL_LOG_DEBUG("GetChildConfigKey() returning error %d", result_code);
        return UPLL_RC_ERR_GENERIC;
      }
      break;
    default:
          UPLL_LOG_DEBUG("Inalid operation\n");
      return UPLL_RC_ERR_GENERIC;
  }

  result_code = UpdateConfigDB(ck_vtn_ff, UPLL_DT_STATE, op, dmi, MAINTBL);
  EnqueCfgNotification(op, UPLL_DT_RUNNING, vtn_ff_key);
  delete ck_vtn_ff;
  return result_code;
}

upll_rc_t VtnFlowFilterMoMgr::DeleteMo(IpcReqRespHeader *req,
                                  ConfigKeyVal *ikey,
                                  DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_ERR_GENERIC;

  if (NULL == ikey || NULL == req) {
  UPLL_LOG_DEBUG("Delete Operation failed:Bad request");
  return result_code;
  }

  result_code = UpdateConfigDB(ikey, UPLL_DT_CANDIDATE, UNC_OP_READ, dmi);

  if (UPLL_RC_ERR_INSTANCE_EXISTS != result_code) {
    UPLL_LOG_DEBUG("Delete Operation Failed: No Record found in DB");
    return result_code;
  }
  VtnFlowFilterEntryMoMgr *mgr = reinterpret_cast<VtnFlowFilterEntryMoMgr *>
    (const_cast<MoManager *> (GetMoManager(UNC_KT_VTN_FLOWFILTER_ENTRY)));
  if (NULL == mgr) {
    UPLL_LOG_DEBUG("VtnFlowFilterEntryMoMgr mgr is NULL");
    return result_code;
  }
  ConfigKeyVal *vtn_ffe_ckv = NULL;
  result_code = mgr->GetChildConfigKey(vtn_ffe_ckv, NULL);
  if (UPLL_RC_SUCCESS != result_code) {
    return result_code;
  }
  key_vtn_flowfilter_t *vtn_ff_key =
    reinterpret_cast<key_vtn_flowfilter_t*>(ikey->get_key());
  key_vtn_flowfilter_entry_t *vtn_ffe_key =
    reinterpret_cast<key_vtn_flowfilter_entry_t*>(vtn_ffe_ckv->get_key());
  uuu::upll_strncpy(vtn_ffe_key->flowfilter_key.vtn_key.vtn_name,
      vtn_ff_key->vtn_key.vtn_name, kMaxLenVtnName+1);
  vtn_ffe_key->flowfilter_key.input_direction =
    vtn_ff_key->input_direction;
  result_code = mgr->DeleteChildMo(req, vtn_ffe_ckv, dmi);
  if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
    result_code = UPLL_RC_SUCCESS;
  }
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("VtnFlowfilterentry delete failed %d", result_code);
    return result_code;
  }
  DbSubOp dbop = {kOpNotRead, kOpMatchNone, kOpInOutNone};
  result_code = UpdateConfigDB(ikey, UPLL_DT_CANDIDATE,
           UNC_OP_DELETE, dmi, &dbop, MAINTBL);
  // result_code = (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE)?
  //               UPLL_RC_SUCCESS:result_code;
  if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_ERROR("Delete Operation fails with %d", result_code);
      return result_code;
  }
  result_code = UpdateConfigDB(ikey, UPLL_DT_CANDIDATE,
           UNC_OP_DELETE, dmi, &dbop, CTRLRTBL);
  result_code = (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE)?
                UPLL_RC_SUCCESS:result_code;
  if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_ERROR("Delete Operation fails with %d", result_code);
     return result_code;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtnFlowFilterMoMgr::GetParentConfigKey(ConfigKeyVal *&okey,
                                                 ConfigKeyVal *ikey) {
  UPLL_FUNC_TRACE;

  if (!ikey) {
    UPLL_LOG_DEBUG(" Input Key is NULL");
    return UPLL_RC_ERR_GENERIC;
  }

  unc_key_type_t ikey_type = ikey->get_key_type();
  if (ikey_type != UNC_KT_VTN_FLOWFILTER) {
    UPLL_LOG_DEBUG(" Invalid key type received. Key type - %d", ikey_type);
    return UPLL_RC_ERR_GENERIC;
  }

  key_vtn_flowfilter_t *pkey =
      reinterpret_cast<key_vtn_flowfilter_t*>(ikey->get_key());
  if (!pkey) {
    UPLL_LOG_DEBUG(" Input vtn flow filter key is NULL ");
    return UPLL_RC_ERR_GENERIC;
  }

  key_vtn_t *vtn_key = reinterpret_cast<key_vtn_t*>
      (ConfigKeyVal::Malloc(sizeof(key_vtn_t)));

  uuu::upll_strncpy(vtn_key->vtn_name,
                    reinterpret_cast<key_vtn_flowfilter_t*>
                    (pkey)->vtn_key.vtn_name,
                    (kMaxLenVtnName + 1));
  okey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, vtn_key, NULL);
  SET_USER_DATA(okey, ikey);
  return UPLL_RC_SUCCESS;
}
}  // namespace kt_momgr
}  // namespace upll
}  // namespace unc
