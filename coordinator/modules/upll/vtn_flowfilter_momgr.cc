/*
 * Copyright (c) 2012-2015 NEC Corporation
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
#include "uncxx/upll_log.hh"
#include "ctrlr_capa_defines.hh"
#include "vtn_momgr.hh"


namespace unc {
namespace upll {
namespace kt_momgr {

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
    offsetof(val_vtn_flowfilter_ctrlr_t, cs_row_status),
    uud::kDalUint8, 1 }
};

BindInfo VtnFlowFilterMoMgr::vtn_flowfilter_maintbl_rename_bindinfo[] = {
  { uudst::vtn_flowfilter::kDbiVtnName, CFG_MATCH_KEY, offsetof(
    key_vtn_flowfilter_t, vtn_key.vtn_name),
    uud::kDalChar, kMaxLenVtnName + 1 },
  { uudst::vtn_flowfilter::kDbiVtnName, CFG_INPUT_KEY, offsetof(
    key_rename_vnode_info_t, new_unc_vtn_name),
    uud::kDalChar, kMaxLenVtnName + 1 },
  { uudst::vtn_flowfilter::kDbiFlags, CK_VAL, offsetof(
    key_user_data_t, flags),
    uud::kDalUint8, 1 }
};

BindInfo VtnFlowFilterMoMgr::vtn_flowfilter_ctrlrtbl_rename_bindinfo[] = {
  { uudst::vtn_flowfilter_ctrlr::kDbiVtnName, CFG_MATCH_KEY, offsetof(
    key_vtn_flowfilter_t, vtn_key.vtn_name),
    uud::kDalChar, kMaxLenVtnName + 1 },
  { uudst::vtn_flowfilter_ctrlr::kDbiVtnName, CFG_INPUT_KEY, offsetof(
    key_rename_vnode_info_t, new_unc_vtn_name),
    uud::kDalChar, kMaxLenVtnName + 1 },
  { uudst::vtn_flowfilter_ctrlr::kDbiFlags, CK_VAL, offsetof(
    key_user_data_t, flags),
    uud::kDalUint8, 1 }
};

unc_key_type_t VtnFlowFilterMoMgr::vtn_flowfilter_child[] = {
    UNC_KT_VTN_FLOWFILTER_ENTRY
};

VtnFlowFilterMoMgr::VtnFlowFilterMoMgr() : MoMgrImpl() {
  UPLL_FUNC_TRACE;
  // Rename operation is not support for this KT
  // setting max tables to 2
  ntable = MAX_MOMGR_TBLS;
  table = new Table *[ntable]();

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

  table[CONVERTTBL] = NULL;

  nchild = sizeof(vtn_flowfilter_child) / sizeof(vtn_flowfilter_child[0]);
  child = vtn_flowfilter_child;
}

upll_rc_t VtnFlowFilterMoMgr::MergeValidate(unc_key_type_t keytype,
                                            const char *ctrlr_id,
                                            ConfigKeyVal *ikey,
                                            DalDmlIntf *dmi,
                                            upll_import_type import_type) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_ERR_GENERIC;
  if (import_type == UPLL_IMPORT_TYPE_PARTIAL) {
    // Validate within IMPORT database for normal and multidomain case
    result_code = PI_MergeValidate_for_Vtn_Flowfilter(keytype, ctrlr_id,
      ikey, dmi);
    if (result_code != UPLL_RC_SUCCESS) {
    return result_code;
    }

    unc_keytype_operation_t op[] = { UNC_OP_DELETE, UNC_OP_CREATE };
    int nop = sizeof(op) / sizeof(op[0]);

    // Validate with IMPORT database with RUNNING database
    result_code = ValidateImportWithRunning(keytype, ctrlr_id,
      ikey, op, nop, dmi);
    if ((result_code != UPLL_RC_SUCCESS) &&
        (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE)) {
        UPLL_LOG_DEBUG("ValidateImportWithRunning DB err (%d)", result_code);
        return result_code;
    }
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtnFlowFilterMoMgr::MergeImportToCandidate(unc_key_type_t keytype,
                                            const char *ctrlr_name,
                                            DalDmlIntf *dmi,
                                            upll_import_type import_type) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *ckval = NULL;
  ConfigKeyVal *ff_imkey = NULL, *ff_cdkey = NULL;
  ConfigKeyVal *ckv_import = NULL, *ckv_cand = NULL;
  uint8_t flag = 0;
  string vtn_id = "";

  if (import_type == UPLL_IMPORT_TYPE_PARTIAL) {
    UPLL_LOG_DEBUG("Partial Import");
    return (MoMgrImpl::MergeImportToCandidate(keytype, ctrlr_name,
                                               dmi, import_type));
  }

  if (NULL == ctrlr_name) {
    UPLL_LOG_DEBUG("MergeValidate ctrlr_id NULL");
    return UPLL_RC_ERR_GENERIC;
  }

  VtnMoMgr *vtnmgr =
    static_cast<VtnMoMgr *>((const_cast<MoManager *>
        (GetMoManager(UNC_KT_VTN))));
  if (vtnmgr == NULL) {
    UPLL_LOG_DEBUG("Invalid momgr object for KT_VTN");
    return UPLL_RC_ERR_GENERIC;
  }
  result_code = vtnmgr->GetChildConfigKey(ckval, NULL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetChildConfigKey ckval NULL");
    return result_code;
  }

  DbSubOp dbop = { kOpReadMultiple, kOpMatchNone, kOpInOutFlag };
  /* Read all vtn from VTN main table in import database and check with
   * Candidate database */
  result_code = vtnmgr->ReadConfigDB(ckval, UPLL_DT_IMPORT,
              UNC_OP_READ, dbop, dmi, MAINTBL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("ReadConfigDB ckval NULL (%d)", result_code);
    if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
       UPLL_LOG_DEBUG("NO record in vtn tbl (%d)", result_code);
       result_code = UPLL_RC_SUCCESS;
    }
    DELETE_IF_NOT_NULL(ckval);
    return result_code;
  }
  ConfigKeyVal *tmp_ckval = ckval;
  while (ckval != NULL) {
    /* Get the instance count from vtn ctrl table in candidate.
     * If refcount is more than 1,
     *    which means that the vtn is already exists in candidate
     * If refcount is zero or 1,
     *    which means that the imported vtn is not exists in candidate
     */
    uint32_t imp_instance_count, cand_instance_count;
    /* Get the instance count from vtn ctrl tbl from import db*/
    result_code = vtnmgr->GetChildConfigKey(ckv_import, ckval);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("GetChildConfigKey failed %d", result_code);
      DELETE_IF_NOT_NULL(tmp_ckval);
      return result_code;
    }
    result_code = vtnmgr->GetInstanceCount(ckv_import, NULL,
         UPLL_DT_IMPORT, &imp_instance_count, dmi, CTRLRTBL);
    DELETE_IF_NOT_NULL(ckv_import);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("GetInstanceCount failed %d", result_code);
      DELETE_IF_NOT_NULL(tmp_ckval);
      return result_code;
    }

    /* Get the instance count from vtn ctrl tbl from candidate db*/
    result_code = vtnmgr->GetChildConfigKey(ckv_cand, ckval);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("GetChildConfigKey failed %d", result_code);
      DELETE_IF_NOT_NULL(tmp_ckval);
      return result_code;
    }
    result_code = vtnmgr->GetInstanceCount(ckv_cand, NULL,
         UPLL_DT_CANDIDATE, &cand_instance_count, dmi, CTRLRTBL);
    DELETE_IF_NOT_NULL(ckv_cand);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("GetInstanceCount failed %d", result_code);
      DELETE_IF_NOT_NULL(tmp_ckval);
      return result_code;
    }
    UPLL_LOG_TRACE("Import count (%d) Candidate count (%d)",
                    imp_instance_count, cand_instance_count);
    if (imp_instance_count == cand_instance_count) {
       /* If imported ctrlr's VTN not exists in Candidate, then
        * check the existence of imported ctrlr's VTN flow-filter
        * 1)If the imported ctrlr VTN does not have flow-filter, then continue
        * with the next VTN in imported db
        * 2)If the imported ctrlr VTN has flow-filter, then merge this
        * flow-filter into candidate db
       */
       UPLL_LOG_DEBUG("VTN not exists in candidate(%d)", result_code);

       // Check the imported ctrl VTN's flow-filter existence in Import db
       key_vtn_t *vtn_ikey = reinterpret_cast<key_vtn_t *>(ckval->get_key());
       key_vtn_flowfilter_t *vtn_ff_imkey = reinterpret_cast
         <key_vtn_flowfilter_t*>(ConfigKeyVal::Malloc
         (sizeof(key_vtn_flowfilter_t)));
       uuu::upll_strncpy(vtn_ff_imkey->vtn_key.vtn_name, vtn_ikey->vtn_name,
                        kMaxLenVtnName+1);
       vtn_ff_imkey->input_direction = 0xFE;
       ff_imkey = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER,
                               IpctSt::kIpcStKeyVtnFlowfilter,
                               vtn_ff_imkey, NULL);

       upll_rc_t result_import = ReadConfigDB(ff_imkey, UPLL_DT_IMPORT,
               UNC_OP_READ, dbop, dmi, MAINTBL);
       if (result_import != UPLL_RC_SUCCESS &&
         result_import != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
         DELETE_IF_NOT_NULL(ff_imkey);
         DELETE_IF_NOT_NULL(tmp_ckval);
         return result_code;
       }

       if (result_import == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
         /* If the imported ctrlr VTN does not have flow-filter, then continue
            with the next VTN in imported db */
         UPLL_LOG_DEBUG("flow-filter not exists in import(%d)", result_code);
         DELETE_IF_NOT_NULL(ff_imkey);
         ckval = ckval->get_next_cfg_key_val();
         continue;
       } else if (result_import == UPLL_RC_SUCCESS) {
         // If imported ctrlr's VTN has flow-filter, then merge this flow-filter
         // into candidate db

         /* Get the list of this VTN associated ctrlr and domain */
         std::list<controller_domain_t> list_ctrlr_dom;
         upll_rc_t vtn_ctrlr_span_rt_code = GetVtnControllerSpan(ff_imkey,
                                 UPLL_DT_IMPORT, dmi, list_ctrlr_dom);
         if (vtn_ctrlr_span_rt_code != UPLL_RC_SUCCESS) {
           DELETE_IF_NOT_NULL(ff_imkey);
           if (vtn_ctrlr_span_rt_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
             UPLL_LOG_DEBUG("GetVtnControllerSpan  error code (%d)",
                          vtn_ctrlr_span_rt_code);
             DELETE_IF_NOT_NULL(tmp_ckval);
             return result_code;
           }
           /* If ctrl and domain name not exist in VTN ctrl tbl, then
              continue with the next VTN in import db */
           ckval = ckval->get_next_cfg_key_val();
           continue;
        }

        ConfigKeyVal *tmp_ff_imkey = ff_imkey;
        while (ff_imkey != NULL) {
        // Create the flow-filter in main tbl
        result_code = UpdateConfigDB(ff_imkey, UPLL_DT_CANDIDATE, UNC_OP_CREATE,
                               dmi, TC_CONFIG_GLOBAL, vtn_id, MAINTBL);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("create in CandidateDB failed (%d) ", result_code);
          DELETE_IF_NOT_NULL(tmp_ff_imkey);
          DELETE_IF_NOT_NULL(tmp_ckval);
          return result_code;
        }

        std::list<controller_domain_t>::iterator it= list_ctrlr_dom.begin();
        while (it != list_ctrlr_dom.end()) {
         // Create the entry in ctrlr table with as per the ctrlr and domain
         ConfigKeyVal *ctrlckv = NULL;
         GET_USER_DATA_FLAGS(ff_imkey, flag);
         UPLL_LOG_DEBUG("flag (%d)", flag);

         key_vtn_flowfilter_t *vtn_ff_key = reinterpret_cast
           <key_vtn_flowfilter_t*>(ConfigKeyVal::Malloc
           (sizeof(key_vtn_flowfilter_t)));
         memcpy(vtn_ff_key, reinterpret_cast<key_vtn_flowfilter_t*>
           (ff_imkey->get_key()), sizeof(key_vtn_flowfilter_t));

         ctrlckv = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER,
                       IpctSt::kIpcInvalidStNum, vtn_ff_key, NULL);
         SET_USER_DATA_CTRLR_DOMAIN(ctrlckv, *it);
         SET_USER_DATA_FLAGS(ctrlckv, flag);
         UPLL_LOG_DEBUG("flag (%d)", flag);

         // Create a record in ctrlr tbl in candidate db
         result_code = UpdateConfigDB(ctrlckv, UPLL_DT_CANDIDATE,
                       UNC_OP_CREATE, dmi, TC_CONFIG_GLOBAL, vtn_id,
                       CTRLRTBL);
         if (result_code != UPLL_RC_SUCCESS) {
           UPLL_LOG_DEBUG("Err while inserting in ctrlr table (%d)",
                         result_code);
           DELETE_IF_NOT_NULL(ctrlckv);
           DELETE_IF_NOT_NULL(tmp_ff_imkey);
           DELETE_IF_NOT_NULL(tmp_ckval);
           return result_code;
         }
         DELETE_IF_NOT_NULL(ctrlckv);
         ++it;
       }
       ff_imkey = ff_imkey->get_next_cfg_key_val();
     }
     FREE_LIST_CTRLR(list_ctrlr_dom);
     DELETE_IF_NOT_NULL(ff_imkey);
     }
  } else if (imp_instance_count < cand_instance_count) {
      // If vtn exists in both db, then check the flow-filter existence
      // from import and candidate database
      UPLL_LOG_DEBUG("VTN exists in candidate(%d)", result_code);

      // Check the flow-filter existence in Import db
      key_vtn_t *vtn_ikey = reinterpret_cast<key_vtn_t *>(ckval->get_key());
      key_vtn_flowfilter_t *vtn_ff_imkey = reinterpret_cast
        <key_vtn_flowfilter_t*>(ConfigKeyVal::Malloc
        (sizeof(key_vtn_flowfilter_t)));
      uuu::upll_strncpy(vtn_ff_imkey->vtn_key.vtn_name, vtn_ikey->vtn_name,
                       kMaxLenVtnName+1);
      vtn_ff_imkey->input_direction = 0xFE;
      ff_imkey = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER,
                              IpctSt::kIpcStKeyVtnFlowfilter,
                              vtn_ff_imkey, NULL);

      upll_rc_t result_import = ReadConfigDB(ff_imkey, UPLL_DT_IMPORT,
              UNC_OP_READ, dbop, dmi, MAINTBL);
      if (result_import != UPLL_RC_SUCCESS &&
          result_import != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
          DELETE_IF_NOT_NULL(ff_imkey);
          DELETE_IF_NOT_NULL(tmp_ckval);
          return result_code;
      }

      // Check the flow-filter existence in candidate db
      key_vtn_flowfilter_t *vtn_ff_cdkey = reinterpret_cast
        <key_vtn_flowfilter_t*>
        (ConfigKeyVal::Malloc(sizeof(key_vtn_flowfilter_t)));
      uuu::upll_strncpy(vtn_ff_cdkey->vtn_key.vtn_name, vtn_ikey->vtn_name,
                       kMaxLenVtnName+1);
      vtn_ff_cdkey->input_direction = 0xFE;
      ff_cdkey = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER,
                              IpctSt::kIpcStKeyVtnFlowfilter,
                              vtn_ff_cdkey, NULL);

      upll_rc_t result_cand = ReadConfigDB(ff_cdkey, UPLL_DT_CANDIDATE,
              UNC_OP_READ, dbop, dmi, MAINTBL);
       if (result_cand != UPLL_RC_SUCCESS &&
           result_cand != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
          DELETE_IF_NOT_NULL(ff_imkey);
          DELETE_IF_NOT_NULL(ff_cdkey);
          DELETE_IF_NOT_NULL(tmp_ckval);
          return result_code;
      }

      if ((result_import == UPLL_RC_ERR_NO_SUCH_INSTANCE ||
          result_import == UPLL_RC_SUCCESS) &&
          result_cand == UPLL_RC_SUCCESS) {
        /* If the imported ctrr's vtn does not have the flow-filter and if the
         * UNC has flow-filter, then apply this flow-filter(in/out) to imported
         * ctrlr's VTN */
        /* If the imported ctrr's vtn hsa the flow-filter and if the UNC
         * has flow-filter, then skip the imported ctrl vtn's flow-filter and
         * apply the UNC flow-filter(in/out) to imported ctrlr's VTN */

         /* Get the list of this VTN associated ctrlr and domain */
        std::list<controller_domain_t> list_ctrlr_dom;
        upll_rc_t vtn_ctrlr_span_rt_code = GetVtnControllerSpan(ff_cdkey,
                                 UPLL_DT_IMPORT, dmi, list_ctrlr_dom);
        if (vtn_ctrlr_span_rt_code != UPLL_RC_SUCCESS) {
          DELETE_IF_NOT_NULL(ff_imkey);
          DELETE_IF_NOT_NULL(ff_cdkey);
          if (vtn_ctrlr_span_rt_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
             UPLL_LOG_DEBUG("GetVtnControllerSpan  error code (%d)",
                          vtn_ctrlr_span_rt_code);
             DELETE_IF_NOT_NULL(tmp_ckval);
             return result_code;
           }
           /* If ctrl and domain name not exist in VTN ctrl tbl, then
              continue with the next VTN in import db */
           ckval = ckval->get_next_cfg_key_val();
           continue;
        }

        ConfigKeyVal *tmp_ff_cdkey = ff_cdkey;
        while (ff_cdkey != NULL) {
          std::list<controller_domain_t>::iterator it= list_ctrlr_dom.begin();
          while (it != list_ctrlr_dom.end()) {
            // Create the entry in ctrlr table with as per the ctrlr and domain
            ConfigKeyVal *ctrlckv = NULL;
            GET_USER_DATA_FLAGS(ff_cdkey, flag);
            UPLL_LOG_DEBUG("flag (%d)", flag);

            key_vtn_flowfilter_t *vtn_ff_key = reinterpret_cast
              <key_vtn_flowfilter_t*>
              (ConfigKeyVal::Malloc(sizeof(key_vtn_flowfilter_t)));
            memcpy(vtn_ff_key, reinterpret_cast<key_vtn_flowfilter_t*>
              (ff_cdkey->get_key()), sizeof(key_vtn_flowfilter_t));

            ctrlckv = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER,
                      IpctSt::kIpcInvalidStNum, vtn_ff_key, NULL);
            SET_USER_DATA_CTRLR_DOMAIN(ctrlckv, *it);
            SET_USER_DATA_FLAGS(ctrlckv, flag);
            UPLL_LOG_DEBUG("flag (%d)", flag);

            // Create a record in CANDIDATE DB
            result_code = UpdateConfigDB(ctrlckv, UPLL_DT_CANDIDATE,
                                         UNC_OP_CREATE, dmi, TC_CONFIG_GLOBAL,
                                         vtn_id, CTRLRTBL);
            if (result_code != UPLL_RC_SUCCESS) {
              UPLL_LOG_DEBUG("Err while inserting in ctrlr table (%d)",
                           result_code);
              DELETE_IF_NOT_NULL(ctrlckv);
              DELETE_IF_NOT_NULL(ff_imkey);
              DELETE_IF_NOT_NULL(tmp_ff_cdkey);
              DELETE_IF_NOT_NULL(tmp_ckval);
              return result_code;
            }
            DELETE_IF_NOT_NULL(ctrlckv);
            ++it;
          }
          ff_cdkey = ff_cdkey->get_next_cfg_key_val();
        }
        FREE_LIST_CTRLR(list_ctrlr_dom);
        DELETE_IF_NOT_NULL(tmp_ff_cdkey);
      } else if (result_import == UPLL_RC_SUCCESS &&
        result_cand == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
        /* If candidate does not have flow-filter, then skip the imported
           ctrlr's flow-filter */
        UPLL_LOG_DEBUG("flow-filter not exists in candidate");
        DELETE_IF_NOT_NULL(ff_imkey);
        DELETE_IF_NOT_NULL(ff_cdkey);
      } else if (result_import == UPLL_RC_ERR_NO_SUCH_INSTANCE &&
                 result_cand == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
        UPLL_LOG_DEBUG("Import and candidate does not have flow-filter");
        DELETE_IF_NOT_NULL(ff_imkey);
        DELETE_IF_NOT_NULL(ff_cdkey);
      }
  }
  ckval = ckval->get_next_cfg_key_val();
  }
  DELETE_IF_NOT_NULL(tmp_ckval);
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtnFlowFilterMoMgr::UpdateMo(IpcReqRespHeader *req,
                                       ConfigKeyVal *ikey, DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  UPLL_LOG_DEBUG(" Update operation not allowed ");
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
  if (req->datatype != UPLL_DT_IMPORT) {
    // validate syntax and semantics
    result_code = ValidateMessage(req, ikey);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("ValidateMessage failed: err code(%d)", result_code);
      return result_code;
    }
  }

  result_code = ValidateAttribute(ikey, dmi, req);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("ValidateAttribute failed: err code(%d)", result_code);
    return result_code;
  }


  std::list<controller_domain_t> list_ctrlr_dom;
  vtn_ctrlr_span_rt_code = GetVtnControllerSpan(ikey, req->datatype, dmi,
                                                list_ctrlr_dom);
  if ((vtn_ctrlr_span_rt_code != UPLL_RC_SUCCESS) &&
      (vtn_ctrlr_span_rt_code != UPLL_RC_ERR_NO_SUCH_INSTANCE)) {
    UPLL_LOG_DEBUG(" GetVtnControllerSpan  error code (%d)",
                   vtn_ctrlr_span_rt_code);
    FREE_LIST_CTRLR(list_ctrlr_dom);
    return vtn_ctrlr_span_rt_code;
  }
  TcConfigMode config_mode = TC_CONFIG_INVALID;
  std::string vtn_name = "";
  result_code = GetConfigModeInfo(req, config_mode, vtn_name);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetConfigMode failed");
    return result_code;
  }

  // create a record in CANDIDATE DB for Main Table
  result_code = UpdateConfigDB(ikey, req->datatype, UNC_OP_CREATE,
                               dmi, config_mode, vtn_name, MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("create in CandidateDB failed: err code(%d) ",
                   result_code);
    FREE_LIST_CTRLR(list_ctrlr_dom);
    return result_code;
  }
  // create a record in CANDIDATE DB for controller Table
  if (vtn_ctrlr_span_rt_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    result_code = UpdateControllerTable(ikey, UNC_OP_CREATE,
                                        req->datatype, dmi,
                                        list_ctrlr_dom,
                                        config_mode, vtn_name);

    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("create in ctrlr tbl failed: error code (%d)",
                     result_code);
#if 0
      result_code = UpdateConfigDB(ikey, req->datatype, UNC_OP_DELETE,
                                   dmi, MAINTBL);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("delete in CandidateDB failed: err code(%d) ",
                       result_code);
      }
#endif
    }
  }
  FREE_LIST_CTRLR(list_ctrlr_dom);

  return result_code;
}

upll_rc_t VtnFlowFilterMoMgr::UpdateControllerTable(
    ConfigKeyVal *ikey, unc_keytype_operation_t op,
    upll_keytype_datatype_t dt_type,
    DalDmlIntf *dmi,
    std::list<controller_domain_t> list_ctrlr_dom,
    TcConfigMode config_mode,
    string vtn_name) {
  UPLL_FUNC_TRACE;
  ConfigKeyVal *ctrlckv = NULL;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  unc_keytype_ctrtype_t ctrlrtype = UNC_CT_UNKNOWN;
  uuc::CtrlrMgr *ctrlr_mgr = uuc::CtrlrMgr::GetInstance();
  std::list<controller_domain_t>::iterator it= list_ctrlr_dom.begin();
  while (it != list_ctrlr_dom.end()) {
    if ((op == UNC_OP_CREATE) || (op == UNC_OP_UPDATE)) {
      IpcReqRespHeader *req_header = reinterpret_cast<IpcReqRespHeader*>
               (ConfigKeyVal::Malloc(sizeof(IpcReqRespHeader)));

      req_header->operation = op;
      req_header->datatype = dt_type;

      // Validate whether the attributes supported by controller or not
      result_code = ValidateCapability(req_header,
                                       ikey,
                                       reinterpret_cast<char*>(it->ctrlr));
      free(req_header);

      if (result_code != UPLL_RC_SUCCESS) {
        if ((!ctrlr_mgr->GetCtrlrType(reinterpret_cast<char *>(it->ctrlr),
             dt_type, &ctrlrtype)) || (ctrlrtype != UNC_CT_PFC)) {
           UPLL_LOG_DEBUG("Controller type is  %d", ctrlrtype);
           result_code = UPLL_RC_SUCCESS;
           ++it;
           continue;
        }
        UPLL_LOG_DEBUG("ValidateCapability Failed: result_code=%d",
            result_code);
        return result_code;
      }
    }
    key_vtn_flowfilter_t *vtn_ff_key = reinterpret_cast<key_vtn_flowfilter_t*>
        (ConfigKeyVal::Malloc(sizeof(key_vtn_flowfilter_t)));
    memcpy(vtn_ff_key, reinterpret_cast<key_vtn_flowfilter_t*>(ikey->get_key()),
           sizeof(key_vtn_flowfilter_t));
    ctrlckv = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER, IpctSt::kIpcInvalidStNum,
                               vtn_ff_key, NULL);

    SET_USER_DATA_CTRLR_DOMAIN(ctrlckv, *it);
    if (UNC_OP_CREATE == op) {
      DbSubOp dbop1 = { kOpReadExist, kOpMatchCtrlr |
        kOpMatchDomain, kOpInOutNone };
      result_code = UpdateConfigDB(ctrlckv, dt_type, UNC_OP_READ, dmi, &dbop1,
          CTRLRTBL);
      if (result_code != UPLL_RC_ERR_INSTANCE_EXISTS &&
          result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
        DELETE_IF_NOT_NULL(ctrlckv);
        return result_code;
      }
      if (UPLL_RC_ERR_INSTANCE_EXISTS == result_code) {
        ++it;
        DELETE_IF_NOT_NULL(ctrlckv);
        continue;
      }
    }
    uint8_t flag = 0;
    GET_USER_DATA_FLAGS(ikey, flag);
    SET_USER_DATA_FLAGS(ctrlckv, flag);

    // Create/Update/Delete a record in CANDIDATE DB
    result_code = UpdateConfigDB(ctrlckv, dt_type, op, dmi, config_mode,
                                 vtn_name, CTRLRTBL);
    DELETE_IF_NOT_NULL(ctrlckv);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Err while updating the ctrlr table for CandidateDB(%d)",
                     result_code);
      return result_code;
    }
    ++it;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtnFlowFilterMoMgr::UpdateControllerTableForVtn(
    uint8_t* vtn_name,
    controller_domain *ctrlr_dom,
    unc_keytype_operation_t op,
    upll_keytype_datatype_t dt_type,
    DalDmlIntf *dmi,
    uint8_t flag,
    TcConfigMode config_mode) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *ctrlckv = NULL, *ikey = NULL;
  unc_keytype_ctrtype_t ctrlrtype = UNC_CT_UNKNOWN;
  uuc::CtrlrMgr *ctrlr_mgr = uuc::CtrlrMgr::GetInstance();

  result_code = GetChildConfigKey(ikey, NULL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetChildConfigKey is fail");
    return result_code;
  }
  UPLL_LOG_TRACE("Controller : %s; Domain : %s", ctrlr_dom->ctrlr,
                 ctrlr_dom->domain);

  key_vtn_flowfilter *vtn_ff_key = reinterpret_cast<key_vtn_flowfilter_t*>
                                   (ikey->get_key());
  uuu::upll_strncpy(vtn_ff_key->vtn_key.vtn_name,
                    vtn_name, (kMaxLenVtnName+1));
  // set this value so that the direction
  // can be bound for output instead of match
  vtn_ff_key->input_direction = 0xFE;

  DbSubOp dbop = {kOpReadMultiple, kOpMatchNone, kOpInOutCs | kOpInOutFlag};

  // Read the Configuration from the MainTable
  result_code = ReadConfigDB(ikey, dt_type,
                             UNC_OP_READ, dbop, dmi, MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      UPLL_LOG_DEBUG(" No Records in main table to be created in ctrlr tbl");
      DELETE_IF_NOT_NULL(ikey);
      return UPLL_RC_SUCCESS;
    }
    DELETE_IF_NOT_NULL(ikey);
    return result_code;
  }
  string vtnname(reinterpret_cast<const char *>(vtn_name));
  if (flag != 0) {
    UPLL_LOG_DEBUG("flag in UpdateControllerTableForVtn %d", flag);
    ConfigKeyVal *temp_ikey = ikey;
    ConfigKeyVal *flag_ikey = NULL;
    while (temp_ikey != NULL) {
      result_code = DupConfigKeyVal(flag_ikey, temp_ikey, MAINTBL);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("DupConfigKeyVal failed %d", result_code);
        DELETE_IF_NOT_NULL(ikey);
        return result_code;
      }
      uint8_t temp_flag = 0;
      GET_USER_DATA_FLAGS(flag_ikey, temp_flag);
      UPLL_LOG_DEBUG("temp_flag in UpdateControllerTableForVtn %d", temp_flag);
      flag = flag | temp_flag;
      UPLL_LOG_DEBUG("temp_flag in UpdateControllerTableForVtn %d", temp_flag);
      SET_USER_DATA_FLAGS(flag_ikey, flag);
      DbSubOp dbop1 = {kOpNotRead, kOpMatchNone, kOpInOutCs | kOpInOutFlag};
      result_code = UpdateConfigDB(flag_ikey, dt_type, UNC_OP_UPDATE,
          dmi, &dbop1, config_mode, vtnname, MAINTBL);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_ERROR("UpdateConfigDB failed %d", result_code);
        DELETE_IF_NOT_NULL(ikey);
        DELETE_IF_NOT_NULL(flag_ikey);
        return result_code;
      }
      DELETE_IF_NOT_NULL(flag_ikey);
      temp_ikey = temp_ikey->get_next_cfg_key_val();
    }
  }
  ConfigKeyVal *temp_ikey = ikey;
  while (temp_ikey != NULL) {
    result_code = GetControllerKeyval(ctrlckv, temp_ikey, ctrlr_dom);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("GetControllerKeyval is Fail");
      DELETE_IF_NOT_NULL(ikey);
      return UPLL_RC_ERR_GENERIC;
    }

    if ((op == UNC_OP_CREATE) || (op == UNC_OP_UPDATE)) {
      IpcReqRespHeader *req_header = reinterpret_cast<IpcReqRespHeader*>
               (ConfigKeyVal::Malloc(sizeof(IpcReqRespHeader)));
       req_header->operation = op;
      req_header->datatype = dt_type;

      // Validate whether the attributes supported by controller or not
      result_code = ValidateCapability(
                        req_header, temp_ikey,
                        reinterpret_cast<char*>(ctrlr_dom->ctrlr));
      free(req_header);

      if (result_code != UPLL_RC_SUCCESS) {
        // VTN Flowfilter is not supported for other than PFC Controller
        // so skip adding entry for such sontroller in ctrlr table
        if ((!ctrlr_mgr->GetCtrlrType(
                    reinterpret_cast<char *>(ctrlr_dom->ctrlr),
                    dt_type, &ctrlrtype)) || (ctrlrtype != UNC_CT_PFC)) {
           result_code = UPLL_RC_SUCCESS;
           UPLL_LOG_DEBUG("Controller type is  %d", ctrlrtype);
           temp_ikey = temp_ikey->get_next_cfg_key_val();
           DELETE_IF_NOT_NULL(ctrlckv);
           continue;
        }
        UPLL_LOG_DEBUG("ValidateCapability Failed: result_code=%d",
          result_code);
        DELETE_IF_NOT_NULL(ikey);
        DELETE_IF_NOT_NULL(ctrlckv);
        return result_code;
     }
    }
    if (UPLL_DT_AUDIT == dt_type) {
      val_vtn_flowfilter_ctrlr_t *val =
          reinterpret_cast<val_vtn_flowfilter_ctrlr_t *>
          (ConfigKeyVal::Malloc(sizeof(val_vtn_flowfilter_ctrlr_t)));
      val->cs_row_status = UNC_CS_APPLIED;
      ctrlckv->AppendCfgVal(IpctSt::kIpcInvalidStNum, val);
    }
    // Create/Update/Delete a record in CANDIDATE DB
    result_code = UpdateConfigDB(ctrlckv, dt_type, op, dmi, config_mode,
                                 vtnname, CTRLRTBL);
    if (UPLL_RC_SUCCESS != result_code) {
        UPLL_LOG_ERROR("Err while updating in ctrlr table for candidateDb(%d)",
                       result_code);
       DELETE_IF_NOT_NULL(ikey);
       DELETE_IF_NOT_NULL(ctrlckv);
       return result_code;
    } else {
      DELETE_IF_NOT_NULL(ctrlckv);
    }
    temp_ikey = temp_ikey->get_next_cfg_key_val();
  }
  UPLL_LOG_DEBUG("Successful completion of the controller table updation");
  DELETE_IF_NOT_NULL(ikey);
  return result_code;
}

upll_rc_t VtnFlowFilterMoMgr::GetControllerKeyval(ConfigKeyVal *&ctrlckv,
                                               ConfigKeyVal *&ikey,
                                               controller_domain *ctrlrdom) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_ERR_GENERIC;
  result_code = GetChildConfigKey(ctrlckv, ikey);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetChildConfigKey failed");
    return result_code;
  }
  SET_USER_DATA_CTRLR_DOMAIN(ctrlckv, *ctrlrdom);

  return UPLL_RC_SUCCESS;
}

upll_rc_t VtnFlowFilterMoMgr::IsReferenced(IpcReqRespHeader *req,
                                           ConfigKeyVal *ikey,
                                           DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  // no reference check is required for this function
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtnFlowFilterMoMgr::ValidateAttribute(ConfigKeyVal *ikey,
                                                DalDmlIntf *dmi,
                                                IpcReqRespHeader *req) {
  // No operation
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtnFlowFilterMoMgr::SetRenameFlag(ConfigKeyVal *ikey,
                                            DalDmlIntf *dmi,
                                            IpcReqRespHeader *req) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *pkey = NULL;
  result_code = GetParentConfigKey(pkey, ikey);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetParentConfigKey failed %d", result_code);
    return result_code;
  }
  MoMgrImpl *mgr =
    reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>(GetMoManager(
            UNC_KT_VTN)));
  if (!mgr) {
    UPLL_LOG_DEBUG("mgr is NULL");
    DELETE_IF_NOT_NULL(pkey);
    return UPLL_RC_ERR_GENERIC;
  }
  uint8_t rename = 0;
  DbSubOp dbop = {kOpReadSingle, kOpMatchNone, kOpInOutFlag};
  result_code = mgr->ReadConfigDB(pkey, req->datatype,
                                  UNC_OP_READ, dbop, dmi, MAINTBL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("ReadConfigDB failed %d", result_code);
    DELETE_IF_NOT_NULL(pkey);
    return result_code;
  }
  GET_USER_DATA_FLAGS(pkey, rename);
  UPLL_LOG_DEBUG("Flag from parent : %d", rename);
  SET_USER_DATA_FLAGS(ikey, rename);
  DELETE_IF_NOT_NULL(pkey);
  return UPLL_RC_SUCCESS;
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
    // If no direction is specified , 0xFE is filled to bind output direction
    vtn_ff_key->input_direction = 0xFE;
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
  }
  if ((okey) && (okey->get_key())) {
    vtn_ff_key = reinterpret_cast<key_vtn_flowfilter_t *>
        (okey->get_key());
  } else {
    vtn_ff_key = reinterpret_cast<key_vtn_flowfilter_t *>
        (ConfigKeyVal::Malloc(sizeof(key_vtn_flowfilter_t)));
    // If no direction is specified , 0xFE is filled to bind output direction
    vtn_ff_key->input_direction = 0xFE;
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

  if ((okey) && !(okey->get_key())) {
    UPLL_LOG_DEBUG("okey not null and flow list name updated");
    okey->SetKey(IpctSt::kIpcStKeyVtnFlowfilter, vtn_ff_key);
  }

  if (!okey) {
    okey = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER,
                            IpctSt::kIpcStKeyVtnFlowfilter,
                            vtn_ff_key, NULL);
  }

  SET_USER_DATA(okey, parent_key);
  UPLL_LOG_DEBUG("Okey filled Succesfully %d", result_code);
  return result_code;
}

upll_rc_t VtnFlowFilterMoMgr::GetRenamedControllerKey(
    ConfigKeyVal *ikey, upll_keytype_datatype_t dt_type, DalDmlIntf *dmi,
    controller_domain *ctrlr_dom) {
  UPLL_FUNC_TRACE;
  ConfigKeyVal *okey = NULL;
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  UPLL_LOG_TRACE("Start... Input ConfigKeyVal %s", ikey->ToStrAll().c_str());
  MoMgrImpl *mgr = static_cast<MoMgrImpl*>((const_cast<MoManager*>
        (GetMoManager(UNC_KT_VTN))));
  if (mgr == NULL) {
    UPLL_LOG_DEBUG("Invalid momgr object for KT_VTN");
    return UPLL_RC_ERR_GENERIC;
  }

  result_code = mgr->GetChildConfigKey(okey, NULL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetChildConfigKey fail");
    return result_code;
  }
  if (ctrlr_dom) {
      SET_USER_DATA_CTRLR_DOMAIN(okey, *ctrlr_dom);
  } else {
    UPLL_LOG_ERROR("ctrlr_dom null");
    DELETE_IF_NOT_NULL(okey);
    return result_code;
  }

  UPLL_LOG_DEBUG("ctrlr : %s; domain : %s", ctrlr_dom->ctrlr,
                  ctrlr_dom->domain);

    // Copy the input VTN Name into the Okey and send it for rename check IN db
  uuu::upll_strncpy(
                    reinterpret_cast<key_vtn *>(okey->get_key())->vtn_name,
                    reinterpret_cast<key_vtn_flowfilter_t *>
                    (ikey->get_key())->vtn_key.vtn_name,
                    (kMaxLenVtnName + 1));
  UPLL_LOG_DEBUG("vtn name (%s) (%s)",
                    reinterpret_cast<key_vtn *>(okey->get_key())->vtn_name,
                    reinterpret_cast<key_vtn_flowfilter_t *>
                    (ikey->get_key())->vtn_key.vtn_name);
  DbSubOp dbop = { kOpReadSingle, kOpMatchCtrlr |
                   kOpMatchDomain, kOpInOutFlag };
  /* ctrlr_name */
  result_code = mgr->ReadConfigDB(okey, dt_type, UNC_OP_READ,
      dbop, dmi, RENAMETBL);
  if (result_code != UPLL_RC_SUCCESS &&
      UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
    UPLL_LOG_DEBUG("ReadConfigDB null");
    DELETE_IF_NOT_NULL(okey);
    return result_code;
  }
  // NULL Checks Missing
  if (result_code == UPLL_RC_SUCCESS) {
    val_rename_vtn *rename_val = reinterpret_cast <val_rename_vtn *>
                     (GetVal(okey));
    if (!rename_val) {
      UPLL_LOG_DEBUG("Vtn Name is not Valid.");
      DELETE_IF_NOT_NULL(okey);
      return UPLL_RC_ERR_GENERIC;
    }
    uuu::upll_strncpy(
                      reinterpret_cast<key_vtn_flowfilter_t*>
                      (ikey->get_key())->vtn_key.vtn_name,
                      rename_val->new_name,
                      (kMaxLenVtnName + 1));
  }
  DELETE_IF_NOT_NULL(okey);
  UPLL_LOG_TRACE("End... Input ConfigKeyVal %s", ikey->ToStrAll().c_str());
  UPLL_LOG_DEBUG("GetRenamedControllerKey  Success");
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtnFlowFilterMoMgr::GetRenamedUncKey(ConfigKeyVal *ikey,
                                               upll_keytype_datatype_t dt_type,
                                               DalDmlIntf *dmi,
                                               uint8_t *ctrlr_id) {
  UPLL_FUNC_TRACE;
  ConfigKeyVal *unc_key = NULL;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  UPLL_LOG_TRACE("%s GetRenamedUncKey vtnff start",
                  ikey->ToStrAll().c_str());
  if ((ikey == NULL) || (ctrlr_id == NULL) || (dmi == NULL)) {
    UPLL_LOG_DEBUG("ikey/ctrlr_id dmi NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  DbSubOp dbop = { kOpReadSingle, kOpMatchCtrlr,
    kOpInOutCtrlr | kOpInOutDomain };
  val_rename_vtn *rename_vtn_key = reinterpret_cast<val_rename_vtn*>
      (ConfigKeyVal::Malloc(sizeof(val_rename_vtn)));
  if (!rename_vtn_key) {
    UPLL_LOG_DEBUG("rename_vtn_key NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  key_vtn_flowfilter_t *ctrlr_key =
      reinterpret_cast<key_vtn_flowfilter_t *>(ikey->get_key());
  if (!ctrlr_key) {
    UPLL_LOG_ERROR("rename_vtn_key NULL");
    free(rename_vtn_key);
    return UPLL_RC_ERR_GENERIC;
  }
  uuu::upll_strncpy(rename_vtn_key->new_name,
                    ctrlr_key->vtn_key.vtn_name,
                    (kMaxLenVtnName + 1));
  rename_vtn_key->valid[UPLL_IDX_NEW_NAME_RVTN] = UNC_VF_VALID;

  MoMgrImpl *mgr = static_cast<MoMgrImpl*>((const_cast<MoManager*>(GetMoManager(
      UNC_KT_VTN))));
  result_code = mgr->GetChildConfigKey(unc_key, NULL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetChildConfigKey fail");
    free(rename_vtn_key);
    return result_code;
  }
  if (!unc_key) {
    UPLL_LOG_DEBUG("unc_key NULL");
    free(rename_vtn_key);
    mgr = NULL;
    return result_code;
  }
  SET_USER_DATA_CTRLR(unc_key, ctrlr_id);
  unc_key->AppendCfgVal(IpctSt::kIpcStValRenameVtn, rename_vtn_key);
  result_code = mgr->ReadConfigDB(unc_key, dt_type, UNC_OP_READ, dbop,
                                            dmi, RENAMETBL);
  if ((UPLL_RC_SUCCESS != result_code) &&
      (UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code)) {
    UPLL_LOG_ERROR("ReadConfigDB failed %d", result_code);
    DELETE_IF_NOT_NULL(unc_key);
    mgr = NULL;
    return result_code;
  }

  if (result_code == UPLL_RC_SUCCESS) {
    key_vtn *vtn_key = reinterpret_cast<key_vtn *>(unc_key->get_key());
    uuu::upll_strncpy(ctrlr_key->vtn_key.vtn_name,
                      vtn_key->vtn_name,
                      (kMaxLenVtnName + 1));
    SET_USER_DATA(ikey, unc_key);
    SET_USER_DATA_FLAGS(ikey, VTN_RENAME);
  }
  UPLL_LOG_TRACE("%s GetRenamedUncKey vtnff end",
                  ikey->ToStrAll().c_str());
  DELETE_IF_NOT_NULL(unc_key);
  mgr = NULL;
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtnFlowFilterMoMgr::DupConfigKeyVal(ConfigKeyVal *&okey,
                                              ConfigKeyVal *&req,
                                              MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
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
      val_vtn_flowfilter_ctrlr_t *ival =
          reinterpret_cast<val_vtn_flowfilter_ctrlr_t *>(GetVal(req));
      if (NULL != ival) {
        val_vtn_flowfilter_ctrlr_t *flowfilter_ctrlr_val = NULL;
        flowfilter_ctrlr_val =
           reinterpret_cast<val_vtn_flowfilter_ctrlr_t *>
           (ConfigKeyVal::Malloc(sizeof(val_vtn_flowfilter_ctrlr_t)));
        memcpy(flowfilter_ctrlr_val, ival, sizeof(val_vtn_flowfilter_ctrlr_t));
        tmp1 = new ConfigVal(IpctSt::kIpcStValFlowfilter,
                             flowfilter_ctrlr_val);
        UPLL_LOG_DEBUG("Creation of Duplicate ConfigVal is successful");
      }
    }
  }

  if (tmp1) {
    tmp1->set_user_data(tmp->get_user_data());
  }
  void *tkey = (req)->get_key();
  if (!tkey) {
    UPLL_LOG_DEBUG("Key Structure is NULL in the input ConfigKeyVal");
    DELETE_IF_NOT_NULL(tmp1);
    return UPLL_RC_ERR_GENERIC;
  }
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
  string vtn_name = "";
  DbSubOp dbop = {kOpReadMultiple, kOpMatchNone, kOpInOutCs};
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
    UPLL_LOG_ERROR("ReadConfigDB failed:%d", result_code);
    delete ckv;
    return result_code;
  }
  std::list< unc_keytype_configstatus_t > list_cs_row;
  val_vtn_flowfilter_ctrlr_t *val;
  ConfigKeyVal *temp_ckv = ckv;
  for ( ; temp_ckv != NULL ; temp_ckv = temp_ckv->get_next_cfg_key_val()) {
      val = reinterpret_cast<val_vtn_flowfilter_ctrlr_t *>(GetVal(temp_ckv));
      list_cs_row.push_back((unc_keytype_configstatus_t)val->cs_row_status);
  }
  DELETE_IF_NOT_NULL(ckv);
  val_flowfilter_t *val_temp =
                   reinterpret_cast<val_flowfilter_t *>(GetVal(ikey));
  val_temp->cs_row_status = GetConsolidatedCsStatus(list_cs_row);
  result_code = UpdateConfigDB(ikey, UPLL_DT_RUNNING,
                               UNC_OP_UPDATE, dmi, TC_CONFIG_GLOBAL,
                               vtn_name, MAINTBL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_ERROR("UpdateConfigDB failed:%d", result_code);
    return result_code;
  }
  return result_code;
}
upll_rc_t VtnFlowFilterMoMgr::UpdateAuditConfigStatus(
                    unc_keytype_configstatus_t cs_status,
                    uuc::UpdateCtrlrPhase phase,
                    ConfigKeyVal *&ckv_running,
                    DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  val_vtn_flowfilter_ctrlr *val;
  val = (ckv_running != NULL)?
        reinterpret_cast<val_vtn_flowfilter_ctrlr *>(GetVal(ckv_running)):NULL;
  if (NULL == val) {
    return UPLL_RC_ERR_GENERIC;
  }
  if (uuc::kUpllUcpCreate == phase )
    val->cs_row_status = cs_status;
  if ((uuc::kUpllUcpUpdate == phase) &&
           (val->cs_row_status == UNC_CS_INVALID ||
            val->cs_row_status == UNC_CS_NOT_APPLIED))
    val->cs_row_status = cs_status;
  return result_code;
}

upll_rc_t VtnFlowFilterMoMgr::UpdateConfigStatus(
    ConfigKeyVal *vtn_flow_filter_key, unc_keytype_operation_t op,
    uint32_t driver_result, ConfigKeyVal *nreq, DalDmlIntf *dmi,
    ConfigKeyVal *ctrlr_key) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  unc_keytype_configstatus_t ctrlr_status;
  uint8_t cs_status;
  ctrlr_status = (driver_result == UPLL_RC_SUCCESS) ?
                  UNC_CS_APPLIED : UNC_CS_NOT_APPLIED;
  val_flowfilter_t *vtn_flowfilter_val =
      reinterpret_cast<val_flowfilter_t *>(GetVal(vtn_flow_filter_key));
  val_vtn_flowfilter_ctrlr *ctrlr_val_vtn_flowfilter =
      reinterpret_cast<val_vtn_flowfilter_ctrlr *>(GetVal(ctrlr_key));

  if ((vtn_flowfilter_val == NULL) || (ctrlr_val_vtn_flowfilter == NULL)) {
    UPLL_LOG_DEBUG("vtn_flowfilter &"
                  "ctrlr_val_vtn_flowfilter is Null");
  return UPLL_RC_ERR_GENERIC;
  }

  cs_status = vtn_flowfilter_val->cs_row_status;
  UPLL_LOG_TRACE("cs_status %d ctrlr_status %d\n", cs_status, ctrlr_status);
  if (op == UNC_OP_CREATE) {
    ctrlr_val_vtn_flowfilter->cs_row_status = ctrlr_status;
    /* update the vtn status in main tbl */
    if (vtn_flowfilter_val->cs_row_status == UNC_CS_UNKNOWN) {
        /* first entry in ctrlr table */
      cs_status = ctrlr_status;
    } else if (vtn_flowfilter_val->cs_row_status == UNC_CS_INVALID) {
      cs_status = UNC_CS_INVALID;
    } else if (vtn_flowfilter_val->cs_row_status == UNC_CS_APPLIED) {
        if (ctrlr_status == UNC_CS_NOT_APPLIED) {
          cs_status = UNC_CS_PARTIALLY_APPLIED;
        }
    } else if (vtn_flowfilter_val->cs_row_status == UNC_CS_NOT_APPLIED) {
        if (ctrlr_status == UNC_CS_APPLIED) {
          cs_status =  UNC_CS_PARTIALLY_APPLIED;
        }
    } else {
        cs_status = UNC_CS_PARTIALLY_APPLIED;
    }
    vtn_flowfilter_val->cs_row_status = cs_status;
  }
  // Updating the Controller cs_row_status
  if ((op == UNC_OP_UPDATE) && (nreq != NULL)) {
     val_vtn_flowfilter_ctrlr *run_ctrlr_val =
              reinterpret_cast<val_vtn_flowfilter_ctrlr*>
                                            (GetVal(nreq));
    if (run_ctrlr_val != NULL)
      ctrlr_val_vtn_flowfilter->cs_row_status = run_ctrlr_val->cs_row_status;
  }
  UPLL_LOG_DEBUG("UpdateConfigStatus is  Successfull");
  return result_code;
}
upll_rc_t
VtnFlowFilterMoMgr::SetVtnFFConsolidatedStatus(ConfigKeyVal *ikey,
                                               uint8_t *ctrlr_id,
                                               DalDmlIntf *dmi)  {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *ctrlr_ckv = NULL;
  val_vtn_flowfilter_ctrlr_t *ctrlr_val = NULL;
  uint8_t *vtn_exist_on_ctrlr = NULL;
  bool applied = false, not_applied = false, invalid = false;
  unc_keytype_configstatus_t c_status = UNC_CS_NOT_APPLIED;
  string vtn_name = "";

  DbSubOp dbop = { kOpReadMultiple, kOpMatchNone,
                   kOpInOutCtrlr | kOpInOutDomain | kOpInOutCs };
  if (!ikey || !dmi) {
    UPLL_LOG_DEBUG("Invalid Input");
    return UPLL_RC_ERR_GENERIC;
  }
  result_code = GetChildConfigKey(ctrlr_ckv, ikey);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetChildConfigKey failed err code %d", result_code);
    return result_code;
  }
  result_code = ReadConfigDB(ctrlr_ckv, UPLL_DT_RUNNING,
                             UNC_OP_READ, dbop, dmi,
                             CTRLRTBL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_ERROR("ReadConfigDB from ctrltbl failed err code %d",
                   result_code);
    DELETE_IF_NOT_NULL(ctrlr_ckv);
    return result_code;
  }

  for (ConfigKeyVal *tmp = ctrlr_ckv; tmp != NULL;
                     tmp = tmp->get_next_cfg_key_val()) {
    ctrlr_val = reinterpret_cast<val_vtn_flowfilter_ctrlr_t *>(GetVal(tmp));
    if (!ctrlr_val) {
      UPLL_LOG_ERROR("Controller Value is empty");
      tmp = NULL;
      DELETE_IF_NOT_NULL(ctrlr_ckv);
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
        not_applied = true;
      break;
      case UNC_CS_INVALID:
        invalid = true;
      break;
      default:
        UPLL_LOG_DEBUG("Invalid status");
        DELETE_IF_NOT_NULL(ctrlr_ckv);
        // return UPLL_RC_ERR_GENERIC;
    }
    vtn_exist_on_ctrlr = NULL;
  }
  if (invalid) {
    c_status = UNC_CS_INVALID;
  } else if (applied && !not_applied) {
    c_status = UNC_CS_APPLIED;
  } else if (!applied && not_applied) {
    c_status = UNC_CS_NOT_APPLIED;
  } else if (applied && not_applied) {
    c_status = UNC_CS_PARTIALLY_APPLIED;
  } else {
    c_status = UNC_CS_APPLIED;
  }
  // Set cs_status
  val_flowfilter_t *vtnval = static_cast<val_flowfilter_t *>(GetVal(ikey));
  vtnval->cs_row_status = c_status;
  DbSubOp dbop_update = {kOpNotRead, kOpMatchNone, kOpInOutCs};
  result_code = UpdateConfigDB(ikey, UPLL_DT_RUNNING, UNC_OP_UPDATE, dmi,
                               &dbop_update, TC_CONFIG_GLOBAL, vtn_name,
                               MAINTBL);
  DELETE_IF_NOT_NULL(ctrlr_ckv);
  return result_code;
}

upll_rc_t VtnFlowFilterMoMgr::GetDiffRecord(ConfigKeyVal *ckv_running,
                                   ConfigKeyVal *ckv_audit,
                                   uuc::UpdateCtrlrPhase phase, MoMgrTables tbl,
                                   ConfigKeyVal *&okey,
                                   DalDmlIntf *dmi,
                                   bool &invalid_attr,
                                   bool check_audit_phase) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *ckv_dup = NULL;
  okey = NULL;
  DbSubOp dbop = {kOpReadSingle, kOpMatchNone, kOpInOutCs};
  switch (phase) {
    case uuc::kUpllUcpDelete:
    case uuc::kUpllUcpCreate:
      if (tbl == CTRLRTBL) {
        UPLL_LOG_TRACE("Created  record fot ctrlr_tbl is %s ",
                        ckv_running->ToStrAll().c_str());
        result_code = GetChildConfigKey(okey, ckv_running);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("GetChildConfigKey failed. err_code & phase %d %d",
                           result_code, phase);
          return result_code;
        }
        upll_keytype_datatype_t dt_type = (phase == uuc::kUpllUcpDelete)?
          UPLL_DT_AUDIT : UPLL_DT_RUNNING;
        result_code = ReadConfigDB(okey, dt_type,
                                     UNC_OP_READ, dbop, dmi, MAINTBL);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_ERROR("ReadConfigDB failed. err_code & phase %d %d",
                           result_code, phase);
          return result_code;
        }
      } else {
          UPLL_LOG_TRACE("Created  record is %s ",
                         ckv_running->ToStrAll().c_str());
          result_code = DupConfigKeyVal(okey, ckv_running, tbl);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("DupConfigKeyVal failed. err_code & phase %d %d",
                           result_code, phase);
            return result_code;
          }
      }
    break;
    case uuc::kUpllUcpUpdate:
      if (tbl == CTRLRTBL) {
        UPLL_LOG_TRACE("UpdateRecord  record for Ctrlr_tbl is %s ",
                        ckv_running->ToStrAll().c_str());
        UPLL_LOG_TRACE("UpdateRecord  record for Ctrlr_tbl is %s ",
                        ckv_audit->ToStrAll().c_str());
        result_code = GetChildConfigKey(okey, ckv_running);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("GetChildConfigKey for running record failed."
                         "err_code & phase %d %d", result_code, phase);
          return result_code;
        }
        result_code = ReadConfigDB(okey, UPLL_DT_RUNNING,
                                     UNC_OP_READ, dbop, dmi, MAINTBL);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_ERROR("ReadConfigDB from running failed."
                         "err_code & phase %d %d", result_code, phase);
          return result_code;
        }
        result_code = GetChildConfigKey(ckv_dup, ckv_audit);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("GetChildConfigKey for audit record failed."
                         "err_code & phase %d %d", result_code, phase);
          return result_code;
        }
        result_code = ReadConfigDB(ckv_dup, UPLL_DT_AUDIT,
                                     UNC_OP_READ, dbop, dmi, MAINTBL);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_ERROR("ReadConfigDB from audit failed."
                         "err_code & phase %d %d", result_code, phase);
          return result_code;
        }
      } else {
        UPLL_LOG_TRACE("UpdateRecord  record  is %s ",
            ckv_running->ToStrAll().c_str());
        UPLL_LOG_TRACE("UpdateRecord  record  is %s ",
            ckv_audit->ToStrAll().c_str());
        result_code = DupConfigKeyVal(okey, ckv_running, tbl);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("DupConfigKeyVal failed for running record."
              "err_code & phase %d %d", result_code, phase);
          return result_code;
        }
        result_code = DupConfigKeyVal(ckv_dup, ckv_audit, tbl);
        if (!ckv_dup || result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("DupConfigKeyVal failed for audit record."
              "err_code & phase %d %d", result_code, phase);
          DELETE_IF_NOT_NULL(ckv_dup);
          return result_code;
        }
      }
      if (GetVal(okey) != NULL &&
          GetVal(ckv_dup) != NULL) {
        void *val1 = GetVal(okey);
        invalid_attr = FilterAttributes(val1, GetVal(ckv_dup), true,
                         UNC_OP_UPDATE);
      }
      if (check_audit_phase) {
        if ((okey != NULL) && (ckv_dup!= NULL)) {
          ConfigVal *next_val = (ckv_dup->get_cfg_val())->DupVal();
          okey->AppendCfgVal(next_val);
        }
      }
    break;
    default:
      UPLL_LOG_DEBUG("Invalid operation %d", phase);
      return UPLL_RC_ERR_NO_SUCH_OPERATION;
      break;
  }
  DELETE_IF_NOT_NULL(ckv_dup);
  return result_code;
}

upll_rc_t VtnFlowFilterMoMgr::TxCopyCandidateToRunning(
    unc_key_type_t keytype, CtrlrCommitStatusList *ctrlr_commit_status,
    DalDmlIntf *dmi, TcConfigMode config_mode, string vtn_name) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  DalResultCode db_result;
  // UNC_OP_UPDATE operation is not supported for this keytype
  unc_keytype_operation_t op[] = { UNC_OP_DELETE, UNC_OP_CREATE };

  int nop = sizeof(op) / sizeof(op[0]);
  ConfigKeyVal *vtn_flowfilter_key = NULL, *req = NULL,
    *nreq = NULL, *vtn_ck_run = NULL;
  DalCursor *cfg1_cursor;
  uint8_t *ctrlr_id =NULL;
  map<string, int> ctrlr_result;
  CtrlrCommitStatusList::iterator ccsListItr;
  CtrlrCommitStatus *ccStatusPtr;

  // mode is VIRTUAL and so ignore it
  if (config_mode == TC_CONFIG_VIRTUAL) {
    return UPLL_RC_SUCCESS;
  }

  if (ctrlr_commit_status != NULL) {
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
  }

  for (int i = 0; i < nop; i++) {
    cfg1_cursor = NULL;
    // Update the Main table
    result_code = DiffConfigDB(UPLL_DT_CANDIDATE, UPLL_DT_RUNNING, op[i],
                               req, nreq, &cfg1_cursor,
                               dmi, NULL, config_mode, vtn_name, MAINTBL, true);
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
        if (cfg1_cursor)
            dmi->CloseCursor(cfg1_cursor, true);
        DELETE_IF_NOT_NULL(req);
        DELETE_IF_NOT_NULL(nreq);
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
    cfg1_cursor = NULL;
    // Update the controller table
    result_code = DiffConfigDB(UPLL_DT_CANDIDATE, UPLL_DT_RUNNING, op[i], req,
                  nreq, &cfg1_cursor, dmi, NULL, config_mode, vtn_name,
                  CTRLRTBL, true);
    ConfigKeyVal *vtn_ff_ctrlr_key = NULL;
    while (result_code == UPLL_RC_SUCCESS) {
      db_result = dmi->GetNextRecord(cfg1_cursor);
      result_code = DalToUpllResCode(db_result);
      if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
        result_code = UPLL_RC_SUCCESS;
        break;
      }
      if (op[i] == UNC_OP_CREATE) {
        DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutFlag
                          | kOpInOutCs };
        result_code = GetChildConfigKey(vtn_flowfilter_key, req);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("GetChildConfigKey is failed resultcode=%d",
                         result_code);
          if (cfg1_cursor)
            dmi->CloseCursor(cfg1_cursor, true);
          DELETE_IF_NOT_NULL(req);
          DELETE_IF_NOT_NULL(nreq);
          return result_code;
        }
        result_code = ReadConfigDB(vtn_flowfilter_key, UPLL_DT_RUNNING,
                                   UNC_OP_READ, dbop, dmi, MAINTBL);
        if (result_code != UPLL_RC_SUCCESS &&
                   result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
          UPLL_LOG_DEBUG("Unable to read configuration from CandidateDb");
          if (cfg1_cursor)
            dmi->CloseCursor(cfg1_cursor, true);
          DELETE_IF_NOT_NULL(vtn_flowfilter_key);
          DELETE_IF_NOT_NULL(req);
          DELETE_IF_NOT_NULL(nreq);
          return result_code;
        }

        result_code = CopyVtnFlowFilterControllerCkv(req, vtn_ff_ctrlr_key);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("Failed to create duplicate ConfigKeyVal Err (%d)",
                         result_code);
          DELETE_IF_NOT_NULL(vtn_flowfilter_key);
          if (cfg1_cursor)
            dmi->CloseCursor(cfg1_cursor, true);
          DELETE_IF_NOT_NULL(req);
          DELETE_IF_NOT_NULL(nreq);
          return result_code;
        }

        //  set consolidated config status to UNKNOWN to init vtn cs_status
        // to the cs_status of first controller
        uint32_t cur_instance_count;
        result_code = GetInstanceCount(vtn_flowfilter_key, NULL,
                                   UPLL_DT_CANDIDATE, &cur_instance_count,
                                   dmi, CTRLRTBL);
        if ((result_code == UPLL_RC_SUCCESS) && (cur_instance_count == 1))
            reinterpret_cast<val_flowfilter *>(GetVal(vtn_flowfilter_key))->\
                               cs_row_status = UNC_CS_UNKNOWN;
        GET_USER_DATA_CTRLR(vtn_ff_ctrlr_key, ctrlr_id);

        string controller(reinterpret_cast<char *> (ctrlr_id));
        if (ctrlr_result.empty()) {
          UPLL_LOG_TRACE("ctrlr_commit_status is NULL");
          result_code = UpdateConfigStatus(vtn_flowfilter_key, op[i],
                                           UPLL_RC_ERR_CTR_DISCONNECTED, nreq,
                                           dmi, vtn_ff_ctrlr_key);
        } else {
          result_code = UpdateConfigStatus(vtn_flowfilter_key, op[i],
                                           ctrlr_result[controller], nreq,
                                           dmi, vtn_ff_ctrlr_key);
        }
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("Error in updating configstatus, resultcode=%d",
                         result_code);
          DELETE_IF_NOT_NULL(vtn_flowfilter_key);
          DELETE_IF_NOT_NULL(vtn_ff_ctrlr_key);
          return result_code;
        }
      } else if (op[i] == UNC_OP_DELETE) {
        // Reading Main Running DB for delete op
        DbSubOp dbop1 = { kOpReadSingle, kOpMatchNone, kOpInOutFlag
                                                  | kOpInOutCs };
        result_code = GetChildConfigKey(vtn_ck_run, req);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("GetChildConfigKey is failed resultcode=%d",
                         result_code);
          if (cfg1_cursor)
            dmi->CloseCursor(cfg1_cursor, true);
          DELETE_IF_NOT_NULL(req);
          DELETE_IF_NOT_NULL(nreq);
          return result_code;
        }
        result_code = ReadConfigDB(vtn_ck_run, UPLL_DT_RUNNING,
                                   UNC_OP_READ, dbop1, dmi, MAINTBL);
        if (result_code != UPLL_RC_SUCCESS &&
               result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE ) {
          UPLL_LOG_DEBUG("Unable to read configuration from CandidateDb");
          DELETE_IF_NOT_NULL(vtn_ck_run);
          if (cfg1_cursor)
            dmi->CloseCursor(cfg1_cursor, true);
          DELETE_IF_NOT_NULL(req);
          DELETE_IF_NOT_NULL(nreq);
          return result_code;
        }
        if (result_code == UPLL_RC_SUCCESS) {
          result_code = SetVtnFFConsolidatedStatus(vtn_ck_run, ctrlr_id, dmi);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_ERROR("Could not set consolidated status %d",
                                                  result_code);
            DELETE_IF_NOT_NULL(vtn_ck_run);
            if (cfg1_cursor)
              dmi->CloseCursor(cfg1_cursor, true);
            DELETE_IF_NOT_NULL(req);
            DELETE_IF_NOT_NULL(nreq);
            return result_code;
          }
         }
        DELETE_IF_NOT_NULL(vtn_ck_run);
        result_code = GetChildConfigKey(vtn_ff_ctrlr_key, req);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("Error in getting the configkey, resultcode=%d",
                         result_code);
          DELETE_IF_NOT_NULL(vtn_ck_run);
          if (cfg1_cursor)
            dmi->CloseCursor(cfg1_cursor, true);
          DELETE_IF_NOT_NULL(req);
          DELETE_IF_NOT_NULL(nreq);
          return result_code;
        }
      }
      result_code = UpdateConfigDB(vtn_ff_ctrlr_key, UPLL_DT_RUNNING,
                                   op[i], dmi, config_mode, vtn_name, CTRLRTBL);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_ERROR("DB Error while updating controller table. err code:%d",
                       result_code);
        DELETE_IF_NOT_NULL(vtn_ff_ctrlr_key);
        if (cfg1_cursor)
            dmi->CloseCursor(cfg1_cursor, true);
        DELETE_IF_NOT_NULL(req);
        DELETE_IF_NOT_NULL(nreq);
        return result_code;
      }
      // update the consolidated config status in the Main Table
      if (op[i] != UNC_OP_DELETE) {
            result_code = UpdateConfigDB(vtn_flowfilter_key, UPLL_DT_RUNNING,
                UNC_OP_UPDATE, dmi, config_mode, vtn_name, MAINTBL);
        if (result_code != UPLL_RC_SUCCESS) {
          if (cfg1_cursor)
            dmi->CloseCursor(cfg1_cursor, true);
          DELETE_IF_NOT_NULL(vtn_ck_run);
          DELETE_IF_NOT_NULL(vtn_flowfilter_key);
          DELETE_IF_NOT_NULL(req);
          DELETE_IF_NOT_NULL(nreq);
          return result_code;
        }
      }
      EnqueCfgNotification(op[i], UPLL_DT_RUNNING, vtn_ff_ctrlr_key);
      DELETE_IF_NOT_NULL(vtn_flowfilter_key);
      DELETE_IF_NOT_NULL(vtn_ff_ctrlr_key);
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
          (ConfigKeyVal::Malloc(sizeof(val_vtn_flowfilter_ctrlr_t)));
      ck_val = new ConfigVal(IpctSt::kIpcInvalidStNum, val);
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
  if (MAINTBL == tbl) {
    nattr = sizeof(vtn_flowfilter_maintbl_rename_bindinfo)/
            sizeof(vtn_flowfilter_maintbl_rename_bindinfo[0]);
    bindinfo = vtn_flowfilter_maintbl_rename_bindinfo;
  } else if (CTRLRTBL ==tbl) {
    nattr = sizeof(vtn_flowfilter_ctrlrtbl_rename_bindinfo)/
            sizeof(vtn_flowfilter_ctrlrtbl_rename_bindinfo[0]);
     bindinfo = vtn_flowfilter_ctrlrtbl_rename_bindinfo;
  }
  UPLL_LOG_DEBUG("Successful Completeion");
  return PFC_TRUE;
}

bool VtnFlowFilterMoMgr::IsValidKey(void *key, uint64_t index,
                                    MoMgrTables tbl) {
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
  key_vtn->input_direction = 0xFE;

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
  if ((req->datatype == UPLL_DT_IMPORT) && (req->operation == UNC_OP_READ ||
       req->operation == UNC_OP_READ_SIBLING ||
       req->operation == UNC_OP_READ_SIBLING_BEGIN ||
       req->operation == UNC_OP_READ_NEXT ||
       req->operation == UNC_OP_READ_BULK ||
       req->operation == UNC_OP_READ_SIBLING_COUNT)) {
    return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT;
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
  if ((NULL == req) || (NULL == ikey)) {
    UPLL_LOG_DEBUG("IpcReqRespHeader/ConfigKeyval is NULL");
    return UPLL_RC_ERR_GENERIC;
  }

  if (!ctrlr_name)
    ctrlr_name = static_cast<char *>(ikey->get_user_data());

  if (!ctrlr_name) {
    UPLL_LOG_DEBUG(" Controller name is NULL ");
    return UPLL_RC_ERR_GENERIC;
  }

  bool ret_code = false;
  uint32_t max_instance_count = 0;
  const uint8_t *attrs = NULL;
  uint32_t max_attrs = 0;

  switch (req->operation) {
    case UNC_OP_CREATE: {
      ret_code = GetCreateCapability(ctrlr_name, ikey->get_key_type(),
                                        &max_instance_count,
                                        &max_attrs, &attrs);
      break;
    }
    default: {
      if (req->datatype == UPLL_DT_STATE)
        ret_code = GetStateCapability(ctrlr_name, ikey->get_key_type(),
                                      &max_attrs, &attrs);
      else
        ret_code = GetReadCapability(ctrlr_name, ikey->get_key_type(),
                                      &max_attrs, &attrs);
    }
    break;
  }

  if (!ret_code) {
    UPLL_LOG_DEBUG("keytype(%d) is not supported by controller(%s) "
                   "for opearion(%d)",
                   ikey->get_key_type(), ctrlr_name, req->operation);
    return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtnFlowFilterMoMgr::GetVtnControllerSpan(
    ConfigKeyVal *ikey,
    upll_keytype_datatype_t dt_type,
    DalDmlIntf *dmi,
    std::list<controller_domain_t> &list_ctrlr_dom) {
  UPLL_FUNC_TRACE;
  if (NULL == ikey) {
    UPLL_LOG_DEBUG("iKey NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  ConfigKeyVal *okey = NULL;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  VtnMoMgr *mgr = static_cast<VtnMoMgr*>(const_cast<MoManager*>
                                           (GetMoManager(UNC_KT_VTN)));
  if (mgr == NULL) {
    UPLL_LOG_DEBUG("Invalid momgr object for KT_VTN");
    return UPLL_RC_ERR_GENERIC;
  }
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
  result_code = mgr->GetControllerDomainSpanForPOM(
      okey, dt_type, dmi, list_ctrlr_dom);
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
  string vtn_name = "";

  switch (op) {
    case UNC_OP_CREATE:
      result_code = DupConfigKeyVal(ck_vtn_ff, vtn_ff_key, MAINTBL);
      if (!ck_vtn_ff || (result_code != UPLL_RC_SUCCESS)) {
        UPLL_LOG_DEBUG("DupConfigKeyVal() Returning error %d", result_code);
        return result_code;
      }
      val_ff = reinterpret_cast<val_flowfilter_t *>(GetVal(ck_vtn_ff));
      if (!val_ff) {
        UPLL_LOG_DEBUG("invalid val");
        return UPLL_RC_ERR_GENERIC;
      }
      val_ff->cs_row_status = UNC_CS_APPLIED;
      break;
    case UNC_OP_DELETE:

      result_code = GetChildConfigKey(ck_vtn_ff, vtn_ff_key);
      if (!ck_vtn_ff || (result_code != UPLL_RC_SUCCESS)) {
        UPLL_LOG_DEBUG("GetChildConfigKey() returning error %d", result_code);
        return UPLL_RC_ERR_GENERIC;
      }
      break;
    default:
          UPLL_LOG_DEBUG("Inalid operation");
      return UPLL_RC_ERR_GENERIC;
  }
  DbSubOp dbop = {kOpNotRead, kOpMatchNone, kOpInOutNone};
  dbop.inoutop = kOpInOutCs | kOpInOutFlag;
  result_code = UpdateConfigDB(ck_vtn_ff, UPLL_DT_STATE, op, dmi, &dbop,
      TC_CONFIG_GLOBAL, vtn_name, MAINTBL);
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
  TcConfigMode config_mode = TC_CONFIG_INVALID;
  std::string vtn_name = "";
  result_code = GetConfigModeInfo(req, config_mode, vtn_name);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetConfigMode failed");
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
  result_code = mgr->DeleteChildrenPOM(vtn_ffe_ckv, req->datatype, dmi,
                                       config_mode, vtn_name);
  if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
    result_code = UPLL_RC_SUCCESS;
  }
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("VtnFlowfilterentry delete failed %d", result_code);
    DELETE_IF_NOT_NULL(vtn_ffe_ckv);
    return result_code;
  }

  DbSubOp dbop = {kOpNotRead, kOpMatchNone, kOpInOutNone};
  result_code = UpdateConfigDB(ikey, UPLL_DT_CANDIDATE,
           UNC_OP_DELETE, dmi, &dbop, config_mode, vtn_name, MAINTBL);
  // result_code = (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE)?
  //               UPLL_RC_SUCCESS:result_code;
  if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_ERROR("Delete Operation fails with %d", result_code);
      DELETE_IF_NOT_NULL(vtn_ffe_ckv);
      return result_code;
  }
  result_code = UpdateConfigDB(ikey, UPLL_DT_CANDIDATE,
           UNC_OP_DELETE, dmi, &dbop, config_mode, vtn_name, CTRLRTBL);
  result_code = (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE)?
                UPLL_RC_SUCCESS:result_code;
  if (UPLL_RC_SUCCESS != result_code) {
     UPLL_LOG_ERROR("Delete Operation fails with %d", result_code);
     DELETE_IF_NOT_NULL(vtn_ffe_ckv);
     return result_code;
  }

  DELETE_IF_NOT_NULL(vtn_ffe_ckv);
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtnFlowFilterMoMgr::GetParentConfigKey(ConfigKeyVal *&okey,
                                                 ConfigKeyVal *ikey) {
  UPLL_FUNC_TRACE;

  if (!ikey) {
    UPLL_LOG_DEBUG(" Input Key is NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  DELETE_IF_NOT_NULL(okey);
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

upll_rc_t VtnFlowFilterMoMgr::DeleteChildrenPOM(
          ConfigKeyVal *ikey, upll_keytype_datatype_t dt_type,
          DalDmlIntf *dmi,
          TcConfigMode config_mode,
          string vtn_name) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_ERR_GENERIC;

  if (NULL == ikey || NULL == dmi) {
  UPLL_LOG_DEBUG("Delete Operation failed:Bad request");
  return result_code;
  }
  // Read the DB get the flowlist value and send the delete request to
  // flowlist momgr if flowlist is configured.
  ConfigKeyVal *temp_ikey = NULL;
  result_code = GetChildConfigKey(temp_ikey, ikey);
  if (!temp_ikey || UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetChildConfigKey failed");
    DELETE_IF_NOT_NULL(temp_ikey);
    return result_code;
  }
  result_code = UpdateConfigDB(temp_ikey, dt_type, UNC_OP_DELETE, dmi,
      config_mode, vtn_name, MAINTBL);
  if ((UPLL_RC_SUCCESS != result_code) &&
      (UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code)) {
    UPLL_LOG_DEBUG("DeleteMo record Err in vtnpolicingmaptbl (%d)",
        result_code);
    DELETE_IF_NOT_NULL(temp_ikey);
    return result_code;
  }
  ConfigKeyVal *ctrlr_ikey = NULL;
  result_code = GetChildConfigKey(ctrlr_ikey, ikey);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetChildConfigKey failed");
    DELETE_IF_NOT_NULL(temp_ikey);
    return result_code;
  }
  DbSubOp dbop = {kOpNotRead, kOpMatchNone, kOpInOutNone};
  result_code = UpdateConfigDB(ctrlr_ikey, dt_type, UNC_OP_DELETE, dmi,
      &dbop, config_mode, vtn_name, CTRLRTBL);
  UPLL_LOG_DEBUG("UpdateConfigDB failed for ctrlrtbl %d", result_code);
  if ((UPLL_RC_SUCCESS != result_code) &&
      (UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code)) {
    UPLL_LOG_DEBUG("DeleteMo record Err in vtnpolicingmaptbl (%d)",
        result_code);
    DELETE_IF_NOT_NULL(temp_ikey);
    return result_code;
  }
  if (temp_ikey)
    delete temp_ikey;
  temp_ikey = NULL;
  if (ctrlr_ikey)
    delete ctrlr_ikey;
  ctrlr_ikey = NULL;

  return  UPLL_RC_SUCCESS;
}

upll_rc_t VtnFlowFilterMoMgr::SetValidAudit(ConfigKeyVal *&ikey) {
  UPLL_FUNC_TRACE;
  val_flowfilter_t *val = reinterpret_cast<val_flowfilter_t *>
    (ConfigKeyVal::Malloc(sizeof(val_flowfilter_t)));
  val->cs_row_status = UNC_CS_APPLIED;
  ikey->AppendCfgVal(IpctSt::kIpcStValFlowfilter, val);
  return UPLL_RC_SUCCESS;
}

bool VtnFlowFilterMoMgr::FilterAttributes(void *&val1,
                                          void *val2,
                                          bool copy_to_running,
                                          unc_keytype_operation_t op) {
  UPLL_FUNC_TRACE;
  if (op != UNC_OP_CREATE)
    return true;
  return false;
}

upll_rc_t VtnFlowFilterMoMgr::CreateAuditMoImpl(ConfigKeyVal *ikey,
                                                DalDmlIntf *dmi,
                                                const char *ctrlr_id) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  string vtn_id = "";
  UPLL_LOG_TRACE(" ikey is %s", ikey->ToStrAll().c_str());
  key_vtn_flowfilter_t *vtn_ff_key =
      reinterpret_cast<key_vtn_flowfilter_t *>(ikey->get_key());
  if (!vtn_ff_key) {
    UPLL_LOG_DEBUG("vtn_ff_key NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  uint8_t old_vtn_name[kMaxLenVtnName + 1], new_vtn_name[kMaxLenVtnName + 1];
  memset(old_vtn_name, 0, sizeof(old_vtn_name));
  memset(new_vtn_name, 0, sizeof(new_vtn_name));
  uuu::upll_strncpy(old_vtn_name, vtn_ff_key->vtn_key.vtn_name,
                    kMaxLenVtnName+1);

  UPLL_LOG_TRACE("ikey After GetRenamedUncKey %s", ikey->ToStrAll().c_str());
  uuu::upll_strncpy(new_vtn_name, vtn_ff_key->vtn_key.vtn_name,
                    kMaxLenVtnName+1);

  result_code = SetValidAudit(ikey);
  if (UPLL_RC_SUCCESS != result_code) {
    return result_code;
  }
  // Create a record in AUDIT DB
  DbSubOp dbop1 = { kOpReadExist, kOpMatchNone, kOpInOutNone };

  result_code = UpdateConfigDB(ikey, UPLL_DT_AUDIT, UNC_OP_READ,
                               dmi, &dbop1, MAINTBL);
  if (result_code != UPLL_RC_ERR_INSTANCE_EXISTS &&
      result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
     UPLL_LOG_ERROR("Update record Err in vtnff MainTbl(%d)",
        result_code);
    return result_code;
  }

  if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    result_code = UpdateConfigDB(ikey, UPLL_DT_AUDIT,
                                 UNC_OP_CREATE, dmi, TC_CONFIG_GLOBAL,
                                 vtn_id, MAINTBL);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_ERROR("UpdateConfigDB failed err_code %d", result_code);
      return result_code;
    }
  }

  std::list<controller_domain_t> list_ctrlr_dom;
  upll_rc_t vtn_ctrlr_span_rt_code;
  MoMgrImpl *mgr = static_cast<MoMgrImpl*>((const_cast<MoManager*>(GetMoManager(
      UNC_KT_VTN))));
  UPLL_LOG_DEBUG(" old vtn name (%s), new vtn name (%s)", old_vtn_name,
                                        new_vtn_name);
  if (!(strcmp(reinterpret_cast<const char *>(old_vtn_name),
                reinterpret_cast<const char *>(new_vtn_name)))) {
    std::list<controller_domain_t> tmp_list_ctrlr_dom;
    vtn_ctrlr_span_rt_code = GetVtnControllerSpan(ikey, UPLL_DT_AUDIT,
                                                  dmi, tmp_list_ctrlr_dom);
    if ((vtn_ctrlr_span_rt_code != UPLL_RC_SUCCESS) &&
  (vtn_ctrlr_span_rt_code != UPLL_RC_ERR_NO_SUCH_INSTANCE)) {
      UPLL_LOG_INFO(" GetVtnControllerSpan  error code (%d)",
         vtn_ctrlr_span_rt_code);
      FREE_LIST_CTRLR(list_ctrlr_dom);
      return vtn_ctrlr_span_rt_code;
    }
    if (vtn_ctrlr_span_rt_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      ConfigKeyVal *unc_key = NULL;
      result_code = mgr->GetChildConfigKey(unc_key, NULL);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("GetChildConfigKey fail");
        return result_code;
      }
      val_rename_vtn *rename_vtn_key = reinterpret_cast<val_rename_vtn*>
    (ConfigKeyVal::Malloc(sizeof(val_rename_vtn)));
      if (!rename_vtn_key) {
        UPLL_LOG_DEBUG("rename_vtn_key NULL");
        DELETE_IF_NOT_NULL(unc_key);
        return UPLL_RC_ERR_GENERIC;
      }
      uuu::upll_strncpy(rename_vtn_key->new_name,
      old_vtn_name,
      (kMaxLenVtnName + 1));
      rename_vtn_key->valid[UPLL_IDX_NEW_NAME_RVTN] = UNC_VF_VALID;
      unc_key->AppendCfgVal(IpctSt::kIpcStValRenameVtn, rename_vtn_key);
      DbSubOp dbop = { kOpReadSingle, kOpMatchCtrlr|kOpMatchDomain,
        kOpInOutNone };
      std::list<controller_domain_t>::iterator it;

      for (it = tmp_list_ctrlr_dom.begin();
     it != tmp_list_ctrlr_dom.end(); it++) {
  SET_USER_DATA_CTRLR_DOMAIN(unc_key, *it);
  result_code = mgr->ReadConfigDB(unc_key, UPLL_DT_RUNNING, UNC_OP_READ, dbop,
              dmi, RENAMETBL);
  if ((UPLL_RC_SUCCESS != result_code) &&
      (UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code)) {
    UPLL_LOG_DEBUG("ReadConfigDB failed %d", result_code);
    DELETE_IF_NOT_NULL(unc_key);
    mgr = NULL;
    return result_code;
  }
  if (UPLL_RC_SUCCESS == result_code) {
    continue;
  }
  list_ctrlr_dom.push_back(*it);
  tmp_list_ctrlr_dom.clear();
  break;
      }
      DELETE_IF_NOT_NULL(unc_key);
    }
  } else {
    controller_domain_t tmp_ctrlr_dom, ctrlr_dom;
    tmp_ctrlr_dom.ctrlr = NULL;
    tmp_ctrlr_dom.domain = NULL;
    ctrlr_dom.ctrlr = NULL;
    ctrlr_dom.domain = NULL;
    ConfigKeyVal *unc_key = NULL;
    DbSubOp dbop = { kOpReadSingle, kOpMatchCtrlr,
      kOpInOutDomain|kOpInOutCtrlr };
    MoMgrImpl *mgr =
        static_cast<MoMgrImpl*>((const_cast<MoManager*>(GetMoManager(
        UNC_KT_VTN))));
    result_code = mgr->GetChildConfigKey(unc_key, NULL);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("GetChildConfigKey fail");
      return result_code;
    }
    val_rename_vtn *rename_vtn_key = reinterpret_cast<val_rename_vtn*>
        (ConfigKeyVal::Malloc(sizeof(val_rename_vtn)));
    if (!rename_vtn_key) {
      UPLL_LOG_DEBUG("rename_vtn_key NULL");
      DELETE_IF_NOT_NULL(unc_key);
      return UPLL_RC_ERR_GENERIC;
    }
    uuu::upll_strncpy(rename_vtn_key->new_name,
                      old_vtn_name,
                      (kMaxLenVtnName + 1));
    rename_vtn_key->valid[UPLL_IDX_NEW_NAME_RVTN] = UNC_VF_VALID;

    SET_USER_DATA_CTRLR(unc_key, ctrlr_id);
    unc_key->AppendCfgVal(IpctSt::kIpcStValRenameVtn, rename_vtn_key);
    result_code = mgr->ReadConfigDB(unc_key, UPLL_DT_RUNNING, UNC_OP_READ, dbop,
                                              dmi, RENAMETBL);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("ReadConfigDB failed %d", result_code);
      DELETE_IF_NOT_NULL(unc_key);
      mgr = NULL;
      return result_code;
    }

    ctrlr_dom.ctrlr = reinterpret_cast<uint8_t *>
        (ConfigKeyVal::Malloc((kMaxLenCtrlrId + 1)));
    ctrlr_dom.domain = reinterpret_cast<uint8_t *>
        (ConfigKeyVal::Malloc((kMaxLenDomainId + 1)));

    GET_USER_DATA_CTRLR_DOMAIN(unc_key, tmp_ctrlr_dom);
    uuu::upll_strncpy(ctrlr_dom.ctrlr, tmp_ctrlr_dom.ctrlr,
                        (kMaxLenCtrlrId + 1));
    uuu::upll_strncpy(ctrlr_dom.domain, tmp_ctrlr_dom.domain,
                        (kMaxLenDomainId + 1));

    UPLL_LOG_TRACE(" ctrlr = %s, dom = %s ", ctrlr_dom.ctrlr,
                                              ctrlr_dom.domain);
    list_ctrlr_dom.push_back(ctrlr_dom);
    vtn_ctrlr_span_rt_code = UPLL_RC_SUCCESS;
    DELETE_IF_NOT_NULL(unc_key);
  }
  // create a record in CANDIDATE DB for controller Table
  if (vtn_ctrlr_span_rt_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    result_code = UpdateControllerTable(ikey, UNC_OP_CREATE,
                                        UPLL_DT_AUDIT, dmi,
                                        list_ctrlr_dom,
                                        TC_CONFIG_GLOBAL, vtn_id);
  }
  FREE_LIST_CTRLR(list_ctrlr_dom);
  return result_code;
}

upll_rc_t VtnFlowFilterMoMgr::CopyVtnFlowFilterControllerCkv(ConfigKeyVal *ikey,
                               ConfigKeyVal *&okey) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_ERR_GENERIC;
  // controller_domain ctrlr_dom;
  key_vtn_flowfilter_t *key_tmp =
          reinterpret_cast<key_vtn_flowfilter_t *>(ikey->get_key());
  if (!key_tmp)
     return result_code;

  key_vtn_flowfilter_t *vtn_flowfilterkey =
        reinterpret_cast<key_vtn_flowfilter_t*>
        (ConfigKeyVal::Malloc(sizeof(key_vtn_flowfilter_t)));
  memcpy(vtn_flowfilterkey, key_tmp, sizeof(key_vtn_flowfilter_t));

  ConfigVal *tmp1 = NULL;
  val_vtn_flowfilter_ctrlr_t *ival =
      reinterpret_cast<val_vtn_flowfilter_ctrlr_t *> (GetVal(ikey));
  if (!ival) {
    FREE_IF_NOT_NULL(vtn_flowfilterkey);
    return UPLL_RC_SUCCESS;
  }
  val_vtn_flowfilter_ctrlr_t *val_tmp =
      reinterpret_cast<val_vtn_flowfilter_ctrlr_t *>
      (ConfigKeyVal::Malloc(sizeof(val_vtn_flowfilter_ctrlr_t)));

  memcpy(val_tmp, ival, sizeof(val_vtn_flowfilter_ctrlr_t));
  tmp1 = new ConfigVal(IpctSt::kIpcInvalidStNum, val_tmp);

  okey = new ConfigKeyVal(UNC_KT_VTN_FLOWFILTER, IpctSt::kIpcStKeyVtnFlowfilter,
                           vtn_flowfilterkey, tmp1);
  SET_USER_DATA(okey, ikey);

  return UPLL_RC_SUCCESS;
}

upll_rc_t VtnFlowFilterMoMgr::GetOperation(uuc::UpdateCtrlrPhase phase,
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

upll_rc_t VtnFlowFilterMoMgr::CreatePIForVtnPom(IpcReqRespHeader *req,
                                                ConfigKeyVal *ikey,
                                                DalDmlIntf *dmi,
                                                const char *ctrlr_id) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_ERR_GENERIC;
  upll_rc_t vtn_ctrlr_span_rt_code;
  controller_domain ctrlr_dom;
  ctrlr_dom.ctrlr = NULL;
  ctrlr_dom.domain = NULL;
  uint8_t flag = 0;

  std::list<controller_domain_t> list_ctrlr_dom;
  TcConfigMode config_mode = TC_CONFIG_INVALID;
  std::string vtn_name = "";
  result_code = GetConfigModeInfo(req, config_mode, vtn_name);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetConfigMode failed");
    return result_code;
  }

  result_code = GetRenamedUncKey(ikey, req->datatype, dmi,
                                reinterpret_cast<uint8_t *>(
                                 const_cast<char *>(ctrlr_id)));
  if (UPLL_RC_SUCCESS != result_code &&
      UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
      UPLL_LOG_DEBUG("GetRenamedUncKey failed %d", result_code);
      return result_code;
  }

  result_code = ValidateAttribute(ikey, dmi, req);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("ValidateAttribute failed: err code(%d)", result_code);
    return result_code;
  }

  // create a record in CANDIDATE DB for Main Table
  result_code = UpdateConfigDB(ikey, req->datatype, UNC_OP_CREATE,
                               dmi, config_mode, vtn_name, MAINTBL);
  if (UPLL_RC_ERR_INSTANCE_EXISTS == result_code) {
    UPLL_LOG_DEBUG("flowfilter object exists already (%d)", result_code);
    result_code = UPLL_RC_SUCCESS;
  }

  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("create in CandidateDB failed: err code(%d) ",
                   result_code);
    return result_code;
  }

  GET_USER_DATA_FLAGS(ikey, flag);
  UPLL_LOG_DEBUG("pyn flag value (%d)", flag);
  if (flag & VTN_RENAME) {
    UPLL_LOG_DEBUG("vtn name got renamed");
    GET_USER_DATA_CTRLR_DOMAIN(ikey, ctrlr_dom);
    list_ctrlr_dom.push_back(ctrlr_dom);
  } else {
    UPLL_LOG_DEBUG("vtn name not renamed");
    vtn_ctrlr_span_rt_code = GetVtnControllerSpan(ikey, req->datatype, dmi,
                                                  list_ctrlr_dom);
    if ((vtn_ctrlr_span_rt_code != UPLL_RC_SUCCESS)) {
      UPLL_LOG_INFO(" GetVtnControllerSpan  error code (%d)",
                   vtn_ctrlr_span_rt_code);
      FREE_LIST_CTRLR(list_ctrlr_dom);
      return vtn_ctrlr_span_rt_code;
    }
  }

  // create a record in CANDIDATE DB for controller Table
  result_code = UpdateControllerTable(ikey, UNC_OP_CREATE,
                                      req->datatype, dmi,
                                      list_ctrlr_dom,
                                      config_mode, vtn_name);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("create in ctrlr tbl failed: error code (%d)",
                   result_code);
  }
  if (!(flag & VTN_RENAME))
    FREE_LIST_CTRLR(list_ctrlr_dom);
  return result_code;
}


}  // namespace kt_momgr
}  // namespace upll
}  // namespace unc
