/*
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef UNC_UPLL_UNIFIED_NW_LABEL_RANGE_MOMGR_H
#define UNC_UPLL_UNIFIED_NW_LABEL_RANGE_MOMGR_H

#include <string>
#include <set>
#include <list>
#include "dal_schema.hh"
#include "momgr_impl.hh"
#include "unw_label_momgr.hh"

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

class UNWLabelRangeMoMgr : public MoMgrImpl {
  private:
    static BindInfo unw_label_range_bind_info[];
    static uint16_t kUnwlRangeNumChildKey;  // Num child key in primary key

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
      // no valid parameter
      valid = NULL;
      return UPLL_RC_SUCCESS;
    }

  public:
    UNWLabelRangeMoMgr();
    virtual ~UNWLabelRangeMoMgr() {
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

  bool CompareValidValue(void *&val1, void *val2, bool audit);

  upll_rc_t GetParentConfigKey(ConfigKeyVal *&pkey,
                               ConfigKeyVal *ck_unified_nw);

  upll_rc_t ValidateCapability(IpcReqRespHeader *req,
                               ConfigKeyVal *ikey,
                               const char *ctrlr_id) {
      return UPLL_RC_SUCCESS;
  }

  upll_rc_t ValidateMessage(IpcReqRespHeader *req,
                            ConfigKeyVal *ikey);

  upll_rc_t UpdateAuditConfigStatus(unc_keytype_configstatus_t cs_status,
                                    uuc::UpdateCtrlrPhase phase,
                                    ConfigKeyVal *&ckv_running,
                                    DalDmlIntf *dmi) {
    return UPLL_RC_SUCCESS;
  }
  upll_rc_t UpdateConfigStatus(ConfigKeyVal *req, unc_keytype_operation_t op,
                             uint32_t driver_result, ConfigKeyVal *upd_key,
                             DalDmlIntf *dmi, ConfigKeyVal *ctrlr_key = NULL);

  upll_rc_t DupConfigKeyVal(ConfigKeyVal *&okey, ConfigKeyVal *&req,
                            MoMgrTables tbl = MAINTBL);

  bool GetRenameKeyBindInfo(unc_key_type_t key_type, BindInfo *&binfo,
                            int &nattr, MoMgrTables tbl) {
    return UPLL_RC_SUCCESS;
  }
  upll_rc_t CopyToConfigKey(ConfigKeyVal *&okey, ConfigKeyVal *ikey) {
    return UPLL_RC_SUCCESS;
  }

  upll_rc_t CreateAuditMoImpl(ConfigKeyVal *ikey, DalDmlIntf *dmi,
                              const char *ctrlr_id) {
    return UPLL_RC_SUCCESS;
  }

  upll_rc_t TxUpdateController(unc_key_type_t keytype, uint32_t session_id,
                               uint32_t config_id, uuc::UpdateCtrlrPhase phase,
                               set<string> *affected_ctrlr_set, DalDmlIntf *dmi,
                               ConfigKeyVal **err_ckv, TxUpdateUtil *tx_util,
                               TcConfigMode config_mode, std::string vtn_name) {
      return UPLL_RC_SUCCESS;
  }

  upll_rc_t TxVoteCtrlrStatus(unc_key_type_t keytype,
                              list<CtrlrVoteStatus*> *ctrlr_vote_status,
                              DalDmlIntf *dmi, TcConfigMode config_mode,
                              std::string vtn_name) {
      return UPLL_RC_SUCCESS;
  }

  upll_rc_t MergeImportToCandidate(unc_key_type_t keytype,
                                   const char *ctrlr_id, DalDmlIntf *dmi,
                                   upll_import_type = UPLL_IMPORT_TYPE_FULL) {
    return UPLL_RC_SUCCESS;
  }

  upll_rc_t ImportClear(unc_key_type_t keytype, const char *ctrlr_id,
                        DalDmlIntf *dmi) {
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
    return UPLL_RC_SUCCESS;
  }

  upll_rc_t AuditVoteCtrlrStatus(unc_key_type_t keytype,
                                 CtrlrVoteStatus *vote_status,
                                 DalDmlIntf *dmi) {
    return UPLL_RC_SUCCESS;
  }

  upll_rc_t AuditCommitCtrlrStatus(unc_key_type_t keytype,
                                   CtrlrCommitStatus *commit_status,
                                   DalDmlIntf *dmi) {
    return UPLL_RC_SUCCESS;
  }

  upll_rc_t AuditEnd(unc_key_type_t keytype, const char *ctrlr_id,
                     DalDmlIntf *dmi) {
    return UPLL_RC_SUCCESS;
  }

  upll_rc_t IsReferenced(IpcReqRespHeader *req, ConfigKeyVal *ikey,
                         DalDmlIntf *dmi) {
    return UPLL_RC_SUCCESS;
  }

  upll_rc_t AllocVal(ConfigVal *&ck_val,
                     upll_keytype_datatype_t dt_type,
                     MoMgrTables tbl = MAINTBL);

  upll_rc_t GetRenamedUncKey(ConfigKeyVal *ctrlr_key,
                             upll_keytype_datatype_t dt_type,
                             DalDmlIntf *dmi,
                             uint8_t *ctrlr_id) {
      return UPLL_RC_SUCCESS;
  }
  upll_rc_t GetOperation(uuc::UpdateCtrlrPhase phase,
                         unc_keytype_operation_t &op);

  upll_rc_t MergeValidate(unc_key_type_t keytype,
                          const char *ctrlr_id,
                          ConfigKeyVal *conflict_ckv,
                          DalDmlIntf *dmi,
                          upll_import_type import_type) {
    return UPLL_RC_SUCCESS;
  }
  upll_rc_t ReadSiblingMo(IpcReqRespHeader *header, ConfigKeyVal *ikey,
                          bool begin, DalDmlIntf *dmi);
};
}  // namespace kt_momgr
}  // namespace upll
}  // namespace unc
#endif  // UNC_UPLL_UNIFIED_NW_LABEL_RANGE_MOMGR_H
