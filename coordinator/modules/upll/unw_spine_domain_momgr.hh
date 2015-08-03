/*
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef UNC_UPLL_UNIFIED_NW_SPDOM_MOMGR_H
#define UNC_UPLL_UNIFIED_NW_SPDOM_MOMGR_H


#include <string>
#include <set>
#include <list>
#include "momgr_impl.hh"
#include "dbconn_mgr.hh"
#include "config_mgr.hh"
#include "config_lock.hh"
#include "dal_schema.hh"
#include "convert_vnode.hh"
#include "unified_nw_momgr.hh"
namespace unc {
namespace upll {
namespace kt_momgr {

using unc::upll::config_momgr::CtrlrCommitStatus;
using unc::upll::config_momgr::CtrlrVoteStatus;
using unc::upll::dal::DalBindInfo;
using unc::upll::dal::DalDmlIntf;
using unc::upll::dal::DalCursor;
using unc::upll::dal::DalCDataType;
using unc::upll::ipc_util::IpctSt;

class UNWSpineDomainMoMgr : public MoMgrImpl {
  private:
    static BindInfo unw_spine_domain_bind_info[];
    /**
     * @brief  Gets the valid array position of the variable in the value
     *         structure from the table in the specified configuration
     *
     * @param[in]     val      pointer to the value structure
     * @param[in]     indx     database index for the variable
     * @param[out]    valid    position of the variable in the valid array -
     *                          NULL if valid does not exist.
     * @param[in]     dt_type  specifies the configuration
     * @param[in]     tbl      specifies the table containing the given value
     *
     **/
    upll_rc_t GetValid(void *val,
                       uint64_t indx,
                       uint8_t *&valid,
                       upll_keytype_datatype_t dt_type,
                       MoMgrTables tbl) {
      UPLL_FUNC_TRACE;
      if (val == NULL) return UPLL_RC_SUCCESS;
      if (tbl == MAINTBL) {
        switch (indx) {
          case uudst::unw_spine_domain::kDbiCtrlrName:
            valid = &(reinterpret_cast<val_unw_spdom_ext *>(val))->
                     val_unw_spine_dom.valid[UPLL_IDX_SPINE_CONTROLLER_ID_UNWS];

            break;
          case uudst::unw_spine_domain::kDbiDomainId:
            valid = &(reinterpret_cast<val_unw_spdom_ext *>(val))->
                      val_unw_spine_dom.valid[UPLL_IDX_SPINE_DOMAIN_ID_UNWS];

            break;
          case uudst::unw_spine_domain::kDbiUnwLabelName:
            valid = &(reinterpret_cast<val_unw_spdom_ext *>(val))->
                 val_unw_spine_dom.valid[UPLL_IDX_UNW_LABEL_ID_UNWS];
            break;
          case uudst::unw_spine_domain::kDbiUsedLabelCount:
            valid = &(reinterpret_cast<val_unw_spdom_ext *>(val))->
                valid[UPLL_IDX_SPINE_USED_LABEL_COUNT_UNWS];
            break;
          case uudst::unw_spine_domain::kDbiAlarmRaised:
            valid = &(reinterpret_cast<val_spdom_st*>(val))->
                valid[UPLL_IDX_SPINE_ALARAM_RAISED_UNWS];
            break;
          default:
            UPLL_LOG_DEBUG("DEFAULT CASE indx  %"PFC_PFMT_u64"", indx);
            return UPLL_RC_ERR_GENERIC;
        }
      }
        return UPLL_RC_SUCCESS;
    }
    // For Threshold Alarm
    bool threshold_alarm_processing_required_;

  public:
    UNWSpineDomainMoMgr();
    virtual ~UNWSpineDomainMoMgr() {
      for (int i = 0; i < ntable; i++)
        if (table[i]) {
          delete table[i];
        }
      delete[] table;
    }

  /**
     * @brief      Method to get a configkeyval of a specified keytype from an input configkeyval
     *
     * @param[in/out]  okey                 pointer to output ConfigKeyVal
    * @param[in]      parent_key           pointer to the configkeyval from which the output configkey val is initialized.
     *
     * @retval         UPLL_RC_SUCCESS      Successfull completion.
     * @retval         UPLL_RC_SUCCESS  Failure case.
     */
    upll_rc_t GetChildConfigKey(ConfigKeyVal *&okey, ConfigKeyVal *parent_key);

    upll_rc_t ValidateAttribute(ConfigKeyVal *kval,
                                DalDmlIntf *dmi,
                                IpcReqRespHeader *req = NULL);

   /* @brief         Read the configuration either from RDBMS and/or
    *                from the controller
    *
    * @param[in]     req    Pointer to IpcResResHeader
    * @param[in/out] ikey   Pointer to the ConfigKeyVal Structure
    * @param[in]     dmi    Pointer to the DalDmlIntf(DB Interface)
    *
    * @retval  UPLL_RC_SUCCESS                    Completed successfully.
    * @retval  UPLL_RC_ERR_GENERIC                Generic failure.
    * @retval  UPLL_RC_ERR_RESOURCE_DISCONNECTED  Resource disconnected.
    * @retval  UPLL_RC_ERR_DB_ACCESS              DB Read/Write error.
    * @retval  UPLL_RC_ERR_NO_SUCH_INSTANCE       Given key does not exist
    *
    **/
    virtual upll_rc_t ReadMo(IpcReqRespHeader *req,
      ConfigKeyVal *ikey,
      DalDmlIntf *dmi);

   /**
     * @Brief This API is used to read the siblings configuration
     *            and statistics
     *
     * @param[in]  req                           Describes
     *                                           RequestResponderHeaderClass.
     * @param[out] ikey                          Pointer to ConfigKeyVal Class.
     * @param[in]  begin                         Describes read from begin or not
     * @param[in]  dmi                           Pointer to DalDmlIntf Class.
     *
     * @retval     UPLL_RC_SUCCESS               Successful completion.
     * @retval     UPLL_RC_ERR_GENERIC           Generic Errors.
     * @retval     UPLL_RC_ERR_NO_SUCH_INSTANCE  Record Not available
     * @retval     UPLL_RC_ERR_CFG_SEMANTIC      Record not found
     * @retval     UPLL_RC_ERR_DB_ACCESS         Error accessing DB.
     */
    upll_rc_t ReadSiblingMo(IpcReqRespHeader *req, ConfigKeyVal *ikey,
                            bool begin, DalDmlIntf *dmi);

    /* @brief       Gets the count of MOs from the sibling group under the parent
     *              specified in the key from the specified UNC database
     *
     * @param[in]     req    Pointer to IpcResResHeader
     * @param[in/out] ikey   Pointer to the ConfigKeyVal Structure
     * @param[in]     dmi    Pointer to the DalDmlIntf(DB Interface)
     *
     * @retval  UPLL_RC_SUCCESS                    Completed successfully.
     * @retval  UPLL_RC_ERR_GENERIC                Generic failure.
     * @retval  UPLL_RC_ERR_DB_ACCESS              DB Read/Write error.
     * @retval  UPLL_RC_ERR_NO_SUCH_INSTANCE       Given key does not exist
     *
     **/
    virtual upll_rc_t ReadSiblingCount(IpcReqRespHeader *req,
                                     ConfigKeyVal* ikey,
                                     DalDmlIntf *dmi);

     /**
     * @brief      Method to check if individual portions of a key are valid
     *
     * @param[in/out]  ikey                 pointer to ConfigKeyVal referring to a UNC resource
     * @param[in]      index                db index associated with the variable
     *
     * @retval         true                 input key is valid
     * @retval         false                input key is invalid.
     **/
    bool IsValidKey(void *key, uint64_t index, MoMgrTables tbl = MAINTBL);

    upll_rc_t ReadSpineDomain(IpcReqRespHeader *req,
                                    ConfigKeyVal *dupkey,
                                    DalDmlIntf *dmi);

    upll_rc_t GetMaxCountOfLabel(IpcReqRespHeader *req,
                                 ConfigKeyVal *dbkey,
                                 ConfigKeyVal **resp_key,
                                 DalDmlIntf *dmi);
    upll_rc_t ConstructReadResponse(IpcReqRespHeader *req,
                                    ConfigKeyVal *ikey,
                                    ConfigKeyVal *db_key,
                                    DalDmlIntf *dmi);
    upll_rc_t GetAssignedLabelDetails(IpcReqRespHeader *req,
                                      ConfigKeyVal *db_key,
                                      ConfigKeyVal **respckv,
                                      uint32_t used_count,
                                      uint32_t &assigned_count,
                                      DalDmlIntf *dmi);
    /** Not used for UnifiedNetwork **/
    bool CompareValidValue(void *&val1, void *val2, bool audit);

    upll_rc_t GetParentConfigKey(ConfigKeyVal *&pkey,
                                 ConfigKeyVal *ck_unified_nw);

  upll_rc_t ValidateCapability(IpcReqRespHeader *req,
                               ConfigKeyVal *ikey,
                               const char *ctrlr_id) {
      return UPLL_RC_SUCCESS;
  }

  upll_rc_t CreateCandidateMo(IpcReqRespHeader *req,
                                       ConfigKeyVal *ikey,
                                       DalDmlIntf *dmi);
  upll_rc_t UpdateMo(IpcReqRespHeader *req,
                     ConfigKeyVal *ikey,
                     DalDmlIntf *dmi);

  upll_rc_t ValidateMessage(IpcReqRespHeader *req,
                            ConfigKeyVal *ikey);

  upll_rc_t UpdateAuditConfigStatus(unc_keytype_configstatus_t cs_status,
                                    uuc::UpdateCtrlrPhase phase,
                                    ConfigKeyVal *&ckv_running,
                                    DalDmlIntf *dmi) {
    return UPLL_RC_SUCCESS;
  }

  upll_rc_t UpdateConfigStatus(ConfigKeyVal *req,
                             unc_keytype_operation_t op,
                             uint32_t driver_result,
                             ConfigKeyVal *upd_key,
                             DalDmlIntf *dmi,
                             ConfigKeyVal *ctrlr_key = NULL);

  upll_rc_t DupConfigKeyVal(ConfigKeyVal *&okey,
                            ConfigKeyVal *&req,
                            MoMgrTables tbl = MAINTBL);

  bool GetRenameKeyBindInfo(unc_key_type_t key_type,
                            BindInfo *&binfo,
                            int &nattr,
                            MoMgrTables tbl) {
    return true;
  }
  upll_rc_t CopyToConfigKey(ConfigKeyVal *&okey,
                            ConfigKeyVal *ikey) {
    return UPLL_RC_SUCCESS;
  }

  upll_rc_t IsReferenced(IpcReqRespHeader *req,
                         ConfigKeyVal *ikey,
                         DalDmlIntf *dmi);
  upll_rc_t AllocVal(ConfigVal *&ck_val,
                     upll_keytype_datatype_t dt_type,
                     MoMgrTables tbl = MAINTBL);

  upll_rc_t GetRenamedUncKey(ConfigKeyVal *ctrlr_key,
                                   upll_keytype_datatype_t dt_type,
                                   DalDmlIntf *dmi,
                                   uint8_t *ctrlr_id) {
    UPLL_FUNC_TRACE;
    return UPLL_RC_SUCCESS;
  }
  upll_rc_t GetOperation(uuc::UpdateCtrlrPhase phase,
                         unc_keytype_operation_t &op);

  upll_rc_t MergeImportToCandidate(unc_key_type_t keytype,
                                 const char *ctrlr_id,
                                 DalDmlIntf *dmi,
                                 upll_import_type = UPLL_IMPORT_TYPE_FULL) {
    UPLL_FUNC_TRACE;
    return UPLL_RC_SUCCESS;
  }

  upll_rc_t ImportClear(unc_key_type_t keytype,
                        const char *ctrlr_id,
                        DalDmlIntf *dmi) {
    UPLL_FUNC_TRACE;
    return UPLL_RC_SUCCESS;
  }

  upll_rc_t AuditVoteCtrlrStatus(unc_key_type_t keytype,
                                 CtrlrVoteStatus *vote_status,
                                 DalDmlIntf *dmi) {
    UPLL_FUNC_TRACE;
    return UPLL_RC_SUCCESS;
  }

  upll_rc_t AuditCommitCtrlrStatus(unc_key_type_t keytype,
                                   CtrlrCommitStatus *commit_status,
                                   DalDmlIntf *dmi) {
    UPLL_FUNC_TRACE;
    return UPLL_RC_SUCCESS;
  }

  upll_rc_t AuditEnd(unc_key_type_t keytype,
                     const char *ctrlr_id,
                     DalDmlIntf *dmi) {
    UPLL_FUNC_TRACE;
    return UPLL_RC_SUCCESS;
  }
  upll_rc_t CreateAuditMoImpl(ConfigKeyVal *ikey,
                              DalDmlIntf *dmi,
                              const char *ctrlr_id) {
      UPLL_FUNC_TRACE;
      return UPLL_RC_SUCCESS;
  }
  upll_rc_t AuditUpdateController(
    unc_key_type_t keytype,
    const char *ctrlr_id,
    uint32_t session_id,
    uint32_t config_id,
    uuc::UpdateCtrlrPhase phase,
    DalDmlIntf *dmi,
    ConfigKeyVal **err_ckv,
    KTxCtrlrAffectedState *ctrlr_affected) {
    UPLL_FUNC_TRACE;
    return UPLL_RC_SUCCESS;
  }

  upll_rc_t TxUpdateController(unc_key_type_t keytype,
                               uint32_t session_id, uint32_t config_id,
                               uuc::UpdateCtrlrPhase phase,
                               set<string> *affected_ctrlr_set,
                               DalDmlIntf *dmi,
                               ConfigKeyVal **err_ckv,
                               TxUpdateUtil *tx_util,
                               TcConfigMode config_mode,
                               std::string vtn_name) {
    UPLL_FUNC_TRACE;
    return UPLL_RC_SUCCESS;
  }
  /*bool GetRenameKeyBindInfo(unc_key_type_t key_type,
                           BindInfo *&binfo,
                           int &nattr,
                          MoMgrTables tbl) {
   UPLL_FUNC_TRACE;
   return UPLL_RC_SUCCESS;
  }
  upll_rc_t CopyToConfigKey(ConfigKeyVal *&okey,
                           ConfigKeyVal *ikey) {
   UPLL_FUNC_TRACE;
   return UPLL_RC_SUCCESS;
  } */

  upll_rc_t TxVoteCtrlrStatus(unc_key_type_t keytype,
                             list<CtrlrVoteStatus*> *ctrlr_vote_status,
                             DalDmlIntf *dmi) {
    UPLL_FUNC_TRACE;
    return UPLL_RC_SUCCESS;
  }
  upll_rc_t IsKeyInUse(upll_keytype_datatype_t dt_type,
                       const ConfigKeyVal *ckv,
                       bool *in_use,
                       DalDmlIntf *dmi);
  upll_rc_t HandleSpineThresholdAlarm(bool assert_alarm, DalDmlIntf *dmi);
  // Used for setting a flag for alarm processing
  inline void set_threshold_alarm_flag(bool threshold_alarm_flag) {
    UPLL_FUNC_TRACE;
    threshold_alarm_processing_required_ = threshold_alarm_flag;
  }
  upll_rc_t ProcessAlarm(TcConfigMode config_mode,
                         std::string vtn_name, DalDmlIntf *dmi);
  upll_rc_t UpdateAlarmStatus(ConfigKeyVal *ikey,
                              bool assert_alarm,
                              TcConfigMode config_mode,
                              std::string vtn_name,
                              DalDmlIntf *dmi);
  upll_rc_t MergeValidate(unc_key_type_t keytype,
                                  const char *ctrlr_id,
                                  ConfigKeyVal *conflict_ckv,
                                  DalDmlIntf *dmi,
                                  upll_import_type import_type) {
    return UPLL_RC_SUCCESS;
  }
};

}  // namespace kt_momgr
}  // namespace upll
}  // namespace unc
#endif

