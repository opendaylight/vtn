/*
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef UNC_UPLL_VTN_UNIFIED_MOMGR_H
#define UNC_UPLL_VTN_UNIFIED_MOMGR_H

#include <string>
#include <set>
#include <list>
#include "momgr_impl.hh"
#include "dbconn_mgr.hh"
#include "config_mgr.hh"
#include "config_lock.hh"

using unc::upll::dal::DalBindInfo;
using unc::upll::dal::DalDmlIntf;
using unc::upll::dal::DalCursor;
using unc::upll::dal::DalCDataType;
using unc::upll::ipc_util::IpctSt;

namespace unc {
namespace upll {
namespace kt_momgr {

class VtnUnifiedMoMgr : public MoMgrImpl {
  private:
    static BindInfo vtn_unified_bind_info[];
    static BindInfo vtn_unified_maintbl_rename_bind_info[];
    static unc_key_type_t vtn_child[];

    /**
     * @brief      Validates the key of type VTN_UNIFIED, 
     * 		   specified in the ConfigKeyVal
     *
     * @param[in]  kval                     pointer to the ConfigKeyVal,
     *                                      which has the key to be validated 
     *
     * @retval     UPLL_RC_SUCCESS          Successfull completion.
     * @retval     UPLL_RC_ERR_BAD_REQUEST  Invalid key 
     * @retval     UPLL_RC_ERR_GENERIC      Failure case.
     */
    upll_rc_t ValidateVtnUnifiedKey(ConfigKeyVal *kval,
                                    unc_keytype_operation_t operation);

    /**
     * @brief      Validates the ConfigValue of type VTN_UNIFIED,
     *             specified in the ConfigKeyVal  
     *
     * @param[in]  kval                     pointer to the ConfigKeyVal,
     *                                      which has the ConfigVal 
     *                                      to be validated.
     * @param[in]  operation                operation specified in the 
     *                                      IPC request header. 
     *
     * @retval     UPLL_RC_SUCCESS          Successfull completion.
     * @retval     UPLL_RC_ERR_BAD_REQUEST  Invalid ConfigVal
     * @retval     UPLL_RC_ERR_GENERIC      Failure case.
     */
    upll_rc_t ValidateVtnUnifiedVal(ConfigKeyVal *kval,
                                    unc_keytype_operation_t operation);

    /**
     * @brief      Reads the database and checks whether the unified nw ID 
     *             specified is valid
     *
     * @param[in]  ikey                          pointer to the ConfigKeyVal
     * @param[in]  dmi                           pointer to DalDmlIntf
     * @param[in]  req                           pointer to IPC request header
     *
     * @retval     UPLL_RC_SUCCESS               Successfull completion
     * @retval     UPLL_RC_ERR_BAD_REQUEST       Invalid ConfigVal
     * @retval     UPLL_RC_ERR_GENERIC           Failure case
     * @retval     UPLL_RC_ERR_NO_SUCH_INSTANCE  unwId does not exists inDB
     */
    upll_rc_t CheckIfUnifiedNwIdExists(ConfigKeyVal *ikey,
                                DalDmlIntf *dmi,
                                IpcReqRespHeader *req,
                                TcConfigMode configMode);
     /**
      * @brief      Reads the database and checks whether the spine domain ID
      *             specified is valid
      *
      * @param[in]  ikey                          pointer to the input 
      *                                                    ConfigKeyVal
      * @param[in]  dmi                           pointer to DalDmlIntf
      * @param[in]  req                           pointer to IPC request header
      * @param[in]  ckNwSpineDomain               pointer to the ConfigKeyVal 
      *                                          (spineDomain)retrieved from DB
      *
      * @retval     UPLL_RC_SUCCESS               Successfull completion
      * @retval     UPLL_RC_ERR_BAD_REQUEST       Invalid ConfigVal
      * @retval     UPLL_RC_ERR_GENERIC           Failure case
      * @retval     UPLL_RC_ERR_NO_SUCH_INSTANCE  unwId does not exists inDB
      */
    upll_rc_t CheckIfSpineDomainExists(ConfigKeyVal **ckNwSpineDomain,
                                DalDmlIntf *dmi, IpcReqRespHeader *req,
                                TcConfigMode configMode);
     /**
      * @brief      Checks the vtunnel entries in DB and update the controller domain 
      *             if there is a change based on given spineDomain 
      *             
      *
      * @param[in]  ikey                          pointer to the input
      *                                                    ConfigKeyVal
      * @param[in]  dmi                           pointer to DalDmlIntf
      * @param[in]  req                           pointer to IPC request header
      * @param[in]  ckNwSpineDomain               pointer to the SpineDomain
      *                                           ConfigKeyVal
      *
      * @retval     UPLL_RC_SUCCESS               Successfull completion
      * @retval     UPLL_RC_ERR_BAD_REQUEST       Invalid ConfigVal
      * @retval     UPLL_RC_ERR_GENERIC           Failure case
      * @retval     UPLL_RC_ERR_NO_SUCH_INSTANCE  unwId does not exists inDB
      */
    upll_rc_t HandleChangeofSpineDomainWithVtunnel(ConfigKeyVal *ckvtunnel,
                                DalDmlIntf *dmi, IpcReqRespHeader *req,
                                ConfigKeyVal *ckNwSpineDomain);

  protected:
    /**
     * @Brief   Validates the IPC request header and ConfigKeyVal of 
     *          type VTN_UNIFIED with respect to the given request header
     *          
     *
     * @param[in]  req    This structure contains
     *                     IpcReqRespHeader(first 8 fields of
     *                     input request structure).
     * @param[in]  ikey   ikey contains key and value structure.
     *
     * @retval   UPLL_RC_SUCCESS               Successful.
     * @retval   UPLL_RC_ERR_GENERIC           Generic failure.
     * @retval   UPLL_RC_ERR_INVALID_OPTION1   option1 is not valid.
     * @retval   UPLL_RC_ERR_INVALID_OPTION2   option2 is not valid.
     * @retval   UPLL_RC_ERR_BAD_REQUEST       Invalid key/value, unsupported 
     *                                         operations/datatype 
     */
    upll_rc_t ValidateMessage(IpcReqRespHeader *req, ConfigKeyVal *kval);

    /**  update the status to APPLIED, as this KT attributes 
         are not sent to the Controller  **/
    upll_rc_t UpdateConfigStatus(ConfigKeyVal *req, unc_keytype_operation_t op,
                                 uint32_t driver_result, ConfigKeyVal *upd_key,
                              DalDmlIntf *dmi, ConfigKeyVal *ctrlr_key = NULL);

    /** This KT does not support audit operation and hence 
        a dummy function, which returns success **/
    upll_rc_t UpdateAuditConfigStatus(unc_keytype_configstatus_t cs_status,
                                      uuc::UpdateCtrlrPhase phase,
                                      ConfigKeyVal *&ckv_running,
                                      DalDmlIntf *dmi) {
      return UPLL_RC_SUCCESS;
    }

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
    upll_rc_t GetValid(void *val, uint64_t indx, uint8_t *&valid,
                       upll_keytype_datatype_t dt_type, MoMgrTables tbl);

  public:
    VtnUnifiedMoMgr();
    virtual ~VtnUnifiedMoMgr() {
      for (int i = 0; i < ntable; i++)
        if (table[i]) {
          delete table[i];
        }
      delete[] table;
    }
    /**
     * @brief      Check whether the dependent entities like unified nw id and 
     *             spine domain id are existing in datatbase
     *             
     *
     *
     * @param[in]  kval                          pointer to the input
     *                                                    ConfigKeyVal
     * @param[in]  dmi                           pointer to DalDmlIntf
     * @param[in]  req                           pointer to IPC request header
     *
     * @retval     UPLL_RC_SUCCESS               Successfull completion
     * @retval     UPLL_RC_ERR_BAD_REQUEST       Invalid ConfigVal
     * @retval     UPLL_RC_ERR_GENERIC           Failure case
     * @retval     UPLL_RC_ERR_NO_SUCH_INSTANCE  unwId does not exists inDB
     */
    upll_rc_t ValidateAttribute(ConfigKeyVal *kval,
                                DalDmlIntf *dmi,
                                IpcReqRespHeader *req = NULL);

    /**
     * @brief      Method to check if individual portions of a key are valid
     *
     * @param[in/out]  ikey    pointer to ConfigKeyVal 
     *                          referring to a UNC resource
     * @param[in]      index   db index associated with the variable
     *
     * @retval         true    input key is valid
     * @retval         false   input key is invalid.
     **/
    bool IsValidKey(void *key, uint64_t index, MoMgrTables tbl = MAINTBL);

    /**
     * @brief      Method to get a configkeyval of a VTN_UNIFIED keytype 
     *             from an input configkeyval
     *
     * @param[in/out]  okey                 pointer to output ConfigKeyVal
     * @param[in]      parent_key           pointer to the configkeyval from 
     *                                      which the output configkey 
     *                                      val is initialized.
     *
     * @retval         UPLL_RC_SUCCESS      Successfull completion.
     * @retval         UPLL_RC_ERR_GENERIC  Failure case.
     */
    upll_rc_t GetChildConfigKey(ConfigKeyVal *&okey, ConfigKeyVal *parent_key);

    /*
     * @brief      Method to get a configkeyval of the parent keytype
     *
     * @param[in/out]  pkey           pointer to parent ConfigKeyVal
     * @param[in]      ck_vtn         pointer to the child configkeyval from
     * which the parent configkey val is obtained.
     *
     * @retval         UPLL_RC_SUCCESS      Successfull completion.
     * @retval         UPLL_RC_ERR_GENERIC  Failure case.
     **/
    upll_rc_t GetParentConfigKey(ConfigKeyVal *&pkey, ConfigKeyVal *ck_vtn);

    /**
     * @brief  Allocates for the specified val in the given configuration in the     
     *                                                           specified table.
     *
     * @param[in/out]  ck_val   Reference pointer to configval structure
     *                          allocated.
     * @param[in]      dt_type  specifies the configuration candidate/running/
     *                          state
     * @param[in]      tbl      specifies if the corresponding table is the
     *                          main table / controller table or rename table.
     *
     * @retval         UPLL_RC_SUCCESS      Successfull completion.
     * @retval         UPLL_RC_ERR_GENERIC  Failure case.
     **/
    upll_rc_t AllocVal(ConfigVal *&ck_val, upll_keytype_datatype_t dt_type,
                       MoMgrTables tbl = MAINTBL);

    /**
     * @brief  Duplicates the input configkeyval including the key and val.
     * based on the tbl specified.
     *
     * @param[in]  okey   Output Configkeyval - allocated within the function
     * @param[in]  req    Input ConfigKeyVal to be duplicated.
     * @param[in]  tbl    specifies if the val structure belongs to the 
     *                    main table/ controller table or rename table.
     *
     * @retval         UPLL_RC_SUCCESS      Successfull completion.
     * @retval         UPLL_RC_ERR_GENERIC  Failure case.
     **/
    upll_rc_t DupConfigKeyVal(ConfigKeyVal *&okey, ConfigKeyVal *&req,
                              MoMgrTables tbl = MAINTBL);

    /**
     * @Brief  Checks if the specified key type(KT_VTN) and
     *         associated attributes are supported on the given controller,
     *         based on the valid flag
     *
     * @param[in]  req               This structure contains IpcReqRespHeader
     *                               (first 8 fields of input request structure).
     * @param[in]  ikey              ikey contains key and value structure.
     *
     * @retval   UPLL_RC_SUCCESS               Validation succeeded.
     * @retval   UPLL_RC_ERR_GENERIC           Validation failure.
     * @retval   UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR  Attribute not supported.
     */
    upll_rc_t ValidateCapability(IpcReqRespHeader *req, ConfigKeyVal *ikey,
                                  const char *cntrl_id = NULL) {
      return UPLL_RC_SUCCESS;
    }

    /** vtn_unified is not referenced by other kt, 
       hence override this fn to return success**/
    upll_rc_t IsReferenced(IpcReqRespHeader *req,
                                 ConfigKeyVal *ikey,
                                 DalDmlIntf *dmi) {
      return UPLL_RC_SUCCESS;
    }
    /**
     * @brief     Method create configkey for the VTN.
     *            Allocates the memory for the okey and
     *            Copy the old name from the rename_info into okey.
     *
     * @param[in]  okey                        key and value structure.
     * @param[in]  rename_info                 key and value structure.
     *
     * @retval     UPLL_RC_SUCCESS             Successfull completion.
     * @retval     UPLL_RC_ERR_GENERIC         Failure case.
     */
    upll_rc_t CopyToConfigKey(ConfigKeyVal *&okey, ConfigKeyVal *ikey);

    /** this kt VTN_UNIFIED does not involve sending the parameters to controller
         and hence below dummy functions are written.
     **/
    bool GetRenameKeyBindInfo(unc_key_type_t key_type, BindInfo *&binfo,
                              int &nattr, MoMgrTables tbl);
    upll_rc_t GetRenamedUncKey(ConfigKeyVal *ctrlr_key,
                               upll_keytype_datatype_t dt_type,
                               DalDmlIntf *dmi,
                               uint8_t *ctrlr_id);

    upll_rc_t GetOperation(uuc::UpdateCtrlrPhase phase,
                           unc_keytype_operation_t &op);
    bool CompareValidValue(void *&val1, void *val2, bool audit);
    upll_rc_t TxUpdateController(unc_key_type_t keytype,
                                uint32_t session_id, uint32_t config_id,
                                uuc::UpdateCtrlrPhase phase,
                                set<string> *affected_ctrlr_set,
                                DalDmlIntf *dmi,
                                ConfigKeyVal **err_ckv,
                                TxUpdateUtil *tx_util,
                                TcConfigMode config_mode,
                                std::string vtn_name) {
      return UPLL_RC_SUCCESS;
    }
    upll_rc_t TxVoteCtrlrStatus(unc_key_type_t keytype,
                                       CtrlrVoteStatusList *ctrlr_vote_status,
                                       DalDmlIntf *dmi,
                                       TcConfigMode config_mode,
                                       std::string vtn_name) {
      return UPLL_RC_SUCCESS;
    }
    upll_rc_t CreateAuditMoImpl(ConfigKeyVal *ikey,
                              DalDmlIntf *dmi,
                              const char *ctrlr_id) {
      return UPLL_RC_SUCCESS;
    }
    upll_rc_t AuditUpdateController(unc_key_type_t keytype,
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
    upll_rc_t AuditEnd(unc_key_type_t keytype,
                       const char *ctrlr_id,
                       DalDmlIntf *dmi) {
      return UPLL_RC_SUCCESS;
    }
    upll_rc_t MergeImportToCandidate(unc_key_type_t keytype,
                                     const char *ctrlr_id,
                                     DalDmlIntf *dmi,
                 upll_import_type = UPLL_IMPORT_TYPE_FULL) {
      return UPLL_RC_SUCCESS;
    }
  upll_rc_t MergeValidate(unc_key_type_t keytype,
                                  const char *ctrlr_id,
                                  ConfigKeyVal *conflict_ckv,
                                  DalDmlIntf *dmi,
                                  upll_import_type import_type) {
    return UPLL_RC_SUCCESS;
  }
};
}}}
#endif

