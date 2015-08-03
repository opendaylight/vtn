/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef MODULES_UPLL_VTN_FLOWFILTER_MOMGR_HH_
#define MODULES_UPLL_VTN_FLOWFILTER_MOMGR_HH_

#include <string>
#include <set>
#include <list>
#include "momgr_impl.hh"

namespace unc {
namespace upll {
namespace kt_momgr {

/* This file declares interfaces for keyType KT_VRT_IF_FLOWFILTER */
/**
 * @brief  VrtIfFlowFilterMoMgr class handles all the request
 *         received from service.
 */

class VtnFlowFilterMoMgr : public MoMgrImpl {
  private:
    static unc_key_type_t vtn_flowfilter_child[];
    static BindInfo vtn_flowfilter_bind_info[];
    static BindInfo vtn_flowfilter_ctrl_bind_info[];
    static BindInfo vtn_flowfilter_maintbl_rename_bindinfo[];
    static BindInfo vtn_flowfilter_ctrlrtbl_rename_bindinfo[];
    /**
    * @Brief Validates the syntax of the specified key and value structure
    *        for KT_VTN_FLOWFILTER/KT_VTN_FLOWFILTER_CONTROLLER keytype
    *
    * @param[in] IpcReqRespHeader contains first 8 fields of input request
    *                             structure
    * @param[in] ConfigKeyVal key and value structure.
    *
    * @retval UPLL_RC_SUCCESS              Successful.
    * @retval UPLL_RC_ERR_CFG_SYNTAX       Syntax error.
    * @retval UPLL_RC_ERR_NO_SUCH_INSTANCE key_vtn_flowfilter is not available.
    * @retval UPLL_RC_ERR_GENERIC          Generic failure.
    */
    upll_rc_t ValidateMessage(IpcReqRespHeader *req, ConfigKeyVal *key);
    /**
    * @Brief Checks if the specified key type and
    *        associated attributes are supported on the given controller,
    *        based on the valid flag.
    *
    * @param[in] val_flowfilter_controller  KT_FLOWFILTER_CONTROLLER value structure.
    * @param[in] attrs         pointer to controller attribute
    *
    * @retval UPLL_RC_SUCCESS                    validation succeeded.
    * @retval UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR Attribute NOT_SUPPORTED.
    * @retval UPLL_RC_ERR_GENERIC                Generic failure.
    */
    upll_rc_t ValVtnFlowfilterAttributeSupportCheck(
        val_flowfilter_controller_t *val_flowfilter_controller,
        const uint8_t* attrs);
    /**
    * @Brief Checks if the specified key type(KT_VTN_FLOWFILTER_CONTROLLER) and
    *        associated attributes are supported on the given controller,
    *        based on the valid flag
    *
    * @param[in] IpcReqRespHeader  contains first 8 fields of input request
    *                              structure
    * @param[in] ConfigKeyVal      contains key and value structure.
    * @param[in] ctrlr_name        controller_name
    *
    * @retval  UPLL_RC_SUCCESS             Validation succeeded.
    * @retval  UPLL_RC_ERR_GENERIC         Validation failure.
    * @retval  UPLL_RC_ERR_INVALID_OPTION1 Option1 is not valid.
    * @retval  UPLL_RC_ERR_INVALID_OPTION2 Option2 is not valid.
    */
    upll_rc_t ValidateCapability(IpcReqRespHeader *req, ConfigKeyVal *ikey,
                                   const char* ctrlr_name = NULL);

    /**
    * @Brief Validates the syntax for KT_VTN_FLOWFILTER_CONTROLLER key structure.
    *
    * @param[in] key_vtn_flowfilter_controller
    *                       KT_VTN_FLOWFILTER_CONTROLLER key structure.
    *
    * @retval UPLL_RC_SUCCESS        validation succeeded.
    * @retval UPLL_RC_ERR_CFG_SYNTAX validation failed.
    */
    upll_rc_t ValidateVtnFlowfilterControllerValue(
    val_flowfilter_controller_t *val_flowfilter_controller, uint32_t operation);

    upll_rc_t UpdateMainTbl(ConfigKeyVal *vtn_ff_key,
                            unc_keytype_operation_t op,
                            uint32_t driver_result,
                            ConfigKeyVal *nreq,
                            DalDmlIntf *dmi);

  public:
    VtnFlowFilterMoMgr();

    ~VtnFlowFilterMoMgr() {
      for (int i = 0; i < ntable; i++) {
        if (table[i]) {
          delete table[i];
        }
      }
      delete[] table;
    }

    /**
     * @brief  Method used for Read Operation.
     *
     * @param[in]       req   Describes RequestResponderHeaderClass.
     * @param[in, out]  ikey  Pointer to ConfigKeyVal Class.
     * @param[in]       dmi   Pointer to DalDmlIntf Class.
     *
     * @retval  UPLL_RC_SUCCESS               Successfull completion.
     * @retval  UPLL_RC_ERR_GENERIC           Returned Generic Error.
     * @retval  UPLL_RC_ERR_NO_SUCH_INSTANCE  No record found in DB
     * @retval  UPLL_RC_ERR_DB_ACCESS         DB access error
     */

    upll_rc_t ReadMo(IpcReqRespHeader *req, ConfigKeyVal *ikey,
                     DalDmlIntf *dmi);
     /**
     * @brief  Method used for ReadSiblingMo.
     *
     * @param[in]       req     Describes RequestResponderHeaderClass.
     * @param[in, out]  key     Pointer to ConfigKeyVal Class.
     * @param[in]       begin   bool value set as 0 or 1.
     * @param[in]       dmi     Pointer to DalDmlIntf Class.
     *
     * @retval  UPLL_RC_SUCCESS               Successfull completion.
     * @retval  UPLL_RC_ERR_NO_SUCH_INSTANCE  No record found in DB
     * @retval  UPLL_RC_ERR_DB_ACCESS         DB access error
     */

    upll_rc_t ReadSiblingMo(IpcReqRespHeader *req, ConfigKeyVal *key,
                            bool begin, DalDmlIntf *dmi);
    /**
     * @brief  Method used for Validation before Merge.
     *
     * @param[in]  ikey      This Contains the pointer to the Class for which
                             fields have to be Validated before the Merge.
     * @param[in]  keytype   Describes the keyType Information.
     * @param[in]  dmi       Pointer to DalDmlIntf Class.
     * @param[in]  ctrlr_id  Describes the Controller Name.
     *
     * @retval  UPLL_RC_SUCCESS               Successfull completion.
     * @retval  UPLL_RC_ERR_NO_SUCH_INSTANCE  No record found in DB
     * @retval  UPLL_RC_ERR_DB_ACCESS         DB access error
     * @retval  UPLL_RC_ERR_MERGE_CONFLICT    Merge conflict
     */

    upll_rc_t MergeValidate(unc_key_type_t keytype, const char *ctrlr_id,
                            ConfigKeyVal *ikey, DalDmlIntf *dmi,
                            upll_import_type import_type);

    upll_rc_t MergeImportToCandidate(unc_key_type_t keytype,
                                     const char *ctrlr_name,
                                     DalDmlIntf *dmi,
                                     upll_import_type import_type);
    /**
     * @brief   This API is used to create the record  in candidate
     *          configuration
     *
     * @param[in] req  Describes RequestResponderHeaderClass.
     * @param[in] ikey Pointer to ConfigKeyVal Class.
     * @param[in] dmi  Pointer to DalDmlIntf Class.
     *
     * @retval    UPLL_RC_ERR_INSTANCE_EXISTS  Policymap Record Already Exists
     * @retval    UPLL_RC_ERR_CFG_SEMANTIC     Policy Profile Record not available
     * @retval    UPLL_RC_SUCCESS              Successful completion.
     * @retval    UPLL_RC_ERR_GENERIC          Generic Errors.
     **/

    upll_rc_t CreateCandidateMo(IpcReqRespHeader *req,
                                                ConfigKeyVal *ikey,
                                                DalDmlIntf *dmi);

    /**
     * @brief  Method used for Update Operation.
     *
     * @param[in]  req   Describes RequestResponderHeaderClass.
     * @param[in]  ikey  Pointer to ConfigKeyVal Class.
     * @param[in]  dmi   Pointer to DalDmlIntf Class.
     *
     * @retval  UPLL_RC_SUCCESS  Successfull completion.
     */


    upll_rc_t UpdateMo(IpcReqRespHeader *req, ConfigKeyVal *ikey,
                       DalDmlIntf *dmi);

    /**
     * @brief  Method used for Rename Operation.
     *
     * @param[in]  req       Describes RequestResponderHeaderClass.
     * @param[in]  ikey      Pointer to ConfigKeyVal Class.
     * @param[in]  dmi       Pointer to DalDmlIntf Class.
     * @param[in]  ctrlr_id  Describes the Controller Name.
     *
     * @retval  UPLL_RC_SUCCESS               Successfull completion.
     * @retval  UPLL_RC_ERR_NO_SUCH_INSTANCE  No record found in DB
     * @retval  UPLL_RC_ERR_DB_ACCESS         DB access error
     */

    upll_rc_t RenameMo(IpcReqRespHeader *req, ConfigKeyVal *ikey,
                       DalDmlIntf *dmi, const char *ctrlr_id);

    /**
     * @brief  Method used to fill the CongigKeyVal with the Parent Class
     *         Information.
     *
     * @param[out]  okey        This Contains the pointer to the ConfigKeyVal
     *                          Class for which fields have to be updated with
     *                          values from the parent Class.
     * @param[in]   parent_key  This Contains the pointer to the ConfigKeyVal
     *                          Class which is the Parent Class used to fill
     *                          the details.
     *
     * @retval  UPLL_RC_SUCCESS      Successfull completion.
     * @retval  UPLL_RC_ERR_GENERIC  Failure
     */

    upll_rc_t GetChildConfigKey(ConfigKeyVal *&okey, ConfigKeyVal *parent_key);

    /**
     * @brief  Method used to Duplicate the ConfigkeyVal.
     *
     * @param[out]  okey  This Contains the pointer to the Class for which
                          fields have to be updated with values from the Request.
     * @param[in]   req   This Contains the pointer to the Class which is
                          used for the Duplication .

     * @retval  UPLL_RC_SUCCESS      Successfull completion.
     * @retval  UPLL_RC_ERR_GENERIC  Failure
     */

    upll_rc_t DupConfigKeyVal(ConfigKeyVal *&okey, ConfigKeyVal *&req,
                              MoMgrTables tbl);

    /**
     * @brief     Methods Used for  Validating Attribute.
     * @param[in]  kval     The pointer to the ConfigKeyVal class
     *
     * @param[in]  dmi      Pointer to the Database Interface.
     *
     * @retval  UPLL_RC_SUCCESS      Validation succeeded.
     * @retval  UPLL_RC_ERR_GENERIC  Validation failure.
     * */
     upll_rc_t ValidateAttribute(ConfigKeyVal *kval,
                                DalDmlIntf *dmi,
                                IpcReqRespHeader *req = NULL);

    /**
     * @brief  Gets the valid array position of the variable in the value
     *         structure from the table in the specified configuration
     *         For this KT this function will always return sucess since
     *         there is not Valid array associated in the value structure
     *
     * @param[in]     val      pointer to the value structure
     * @param[in]     indx     database index for the variable
     * @param[out]    valid    position of the variable in the valid array -
     *                          NULL if valid does not exist.
     * @param[in]     dt_type  specifies the configuration
     * @param[in]     tbl      specifies the table containing the given value
     *
     * @retval  UPLL_RC_SUCCESS Successful Completion
     **/
     upll_rc_t GetValid(void*val,
                        uint64_t indx,
                        uint8_t *&valid,
                        upll_keytype_datatype_t dt_type,
                        MoMgrTables tbl) {
       UPLL_FUNC_TRACE;
       // no valid parameter
       valid = NULL;
       return UPLL_RC_SUCCESS;
     }

    /**
     * @brief  Method used for RenamedControllerkey(PfcName).
     *
     * @param[out]  ikey         Contains the Pointer to ConfigkeyVal
                                 Class and contains the Pfc Name.
     * @param[in]   dt_type      Describes Configiration Information.
     * @param[in]   dmi          Pointer to DalDmlIntf Class.
     * @param[in]   ctrlr__name  Describes the Controller Name.
     *
     * @retval  UPLL_RC_SUCCESS               Successfull completion.
     * @retval  UPLL_RC_ERR_GENERIC           Failure
     * @retval  UPLL_RC_ERR_NO_SUCH_INSTANCE  No record found in DB
     * @retval  UPLL_RC_ERR_DB_ACCESS         DB access error
     */

    upll_rc_t GetRenamedControllerKey(ConfigKeyVal *ikey,
                                      upll_keytype_datatype_t dt_type,
                                      DalDmlIntf *dmi,
                                      controller_domain *ctrlr_dom = NULL);

    /**
     * @brief  Method used to get the RenamedUncKey.
     *
     * @param[out]  ikey      This Contains the pointer to
                              the Class for which fields have
                              to be updated with values from
                              the parent Class.
     * @param[in]   dt_type   Describes Configiration Information.
     * @param[in]   dmi       Pointer to DalDmlIntf Class.
     * @param[in]   ctrlr_id  Describes the Controller Name.
     *
     * @retval  UPLL_RC_SUCCESS               Successfull completion.
     * @retval  UPLL_RC_ERR_GENERIC           Failure
     * @retval  UPLL_RC_ERR_NO_SUCH_INSTANCE  No record found in DB
     * @retval  UPLL_RC_ERR_DB_ACCESS         DB access error
     */

    upll_rc_t GetRenamedUncKey(ConfigKeyVal *ikey,
                               upll_keytype_datatype_t dt_type, DalDmlIntf *dmi,
                               uint8_t *ctrlr_id);

    /**
     * @brief  Update config status for commit result and vote result.
     *
     * @param[in/out]  ckv_running  ConfigKeyVal instance.
     * @param[in]      cs_status    either UNC_CS_INVALID or UNC_CS_APPLIED.
     * @param[in]      phase        specify the phase (CREATE,DELETE or UPDATE)
     * @param[in]      dmi          Pointer to the DalDmlIntf(DB Interface)
     *
     * @retval  UPLL_RC_SUCCESS  Successfull completion.
     * @retval  UPLL_RC_GENERIC  Returned Generic Error.
     **/
     upll_rc_t UpdateAuditConfigStatus(unc_keytype_configstatus_t cs_status,
                                       uuc::UpdateCtrlrPhase phase,
                                       ConfigKeyVal *&ckv_running,
                                       DalDmlIntf *dmi);

     upll_rc_t SetConsolidatedStatus(ConfigKeyVal *ikey,
                                     DalDmlIntf *dmi);
    /**
     * @brief  Method used for Trasaction Vote Operation.
     *
     * @param[in]  key            Pointer to ConfigKeyVal Class.
     * @param[in]  op             Describes the Type of Opeartion.
     * @param[in]  driver_result  Describes the result of Driver Operation.
     * @param[in]  upd_key        Pointer to ConfigKeyVal Class.
     * @param[in]  ctrlr_key      Pointer to ConfigKeyVal Class.
     *
     * @retval  UPLL_RC_SUCCESS      Successfull completion.
     * @retval  UPLL_RC_ERR_GENERIC  Returned Generic Error.
     */

    upll_rc_t UpdateConfigStatus(ConfigKeyVal *vtn_flow_filter_key,
                                 unc_keytype_operation_t op,
                                 uint32_t driver_result, ConfigKeyVal *nreq,
                                 DalDmlIntf *dmi, ConfigKeyVal *ctrlr_key);
    /**
     * @brief  Method to copy from Candidate To Running during Transaction
     *         Process.
     *
     * @param[in]  keytype              Describes the KeyType Information.
     * @param[in]  ctrlr_commit_status  Describes Commit Control Status
     *                                  Information.
     * @param[in]  dmi                  Pointer to DalDmlIntf Class.
     *
     * @retval  UPLL_RC_SUCCESS      Successfull completion.
     * @retval  UPLL_RC_ERR_GENERIC  Returned Generic Error.
     */

    upll_rc_t TxCopyCandidateToRunning(
        unc_key_type_t keytype, CtrlrCommitStatusList *ctrlr_commit_status,
        DalDmlIntf *dmi, TcConfigMode config_mode, string vtn_name);

    /**
     * @brief  Allocates Memory for the Incoming Pointer to the Class.
     *
     * @param[out]  ck_val   This Contains the pointer to the Class for
                             which memory has to be allocated.
     * @param[in]   dt_type  Describes Configiration Information.
     * @param[in]   tbl      Describes the Destination table Information.
     *
     *
     * @retval  UPLL_RC_SUCCESS      Successfull completion.
     * @retval  UPLL_RC_ERR_GENERIC  Returned Generic Error.
     */

    upll_rc_t AllocVal(ConfigVal *&ck_val, upll_keytype_datatype_t dt_type,
                       MoMgrTables tbl);

    /**
     * @brief  Method used for ReadRecord.
     *
     * @param[in]  req   Describes RequestResponderHeaderClass.
     * @param[in]  ikey  Pointer to ConfigKeyVal Class.
     * @param[in]  dmi   Pointer to DalDmlIntf Class.
     *
     * @retval  UPLL_RC_SUCCESS               Successfull completion.
     * @retval  UPLL_RC_ERR_GENERIC           Returned Generic Error.
     * @retval  UPLL_RC_ERR_NO_SUCH_INSTANCE  No record found in DB
     * @retval  UPLL_RC_ERR_DB_ACCESS         DB access error
     */

    upll_rc_t ReadRecord(IpcReqRespHeader *req, ConfigKeyVal *ikey,
                         DalDmlIntf *dmi);
     /**
     * @Brief Method used to get the Bind Info Structure for Rename Purpose.
     *
     * @param[in]  key_type  Describes the KT Information.
     * @param[in]  nattr     Describes the Tbl For which the Operation is
     *                       Targeted.
     * @param[in]  tbl       Describes the Table Information
     *
     * @retval  true   Successful Completion.
     * @retval  false  Failure.
     */
     bool GetRenameKeyBindInfo(unc_key_type_t key_type,
                              BindInfo *&binfo,
                              int &nattr,
                              MoMgrTables tbl);
    /**
     * @brief  Method to Copy The ConfigkeyVal with the Input Key.
     *
     * @param[out]  okey  Pointer to ConfigKeyVal Class for which attributes
     *                    have to be copied.
     * @param[in]   ikey  Pointer to ConfigKeyVal Class.
     *
     * @retval  UPLL_RC_SUCCESS      Successfull Completion.
     * @retval  UPLL_RC_ERR_GENERIC  Returned Generic Error.
     */
     upll_rc_t CopyToConfigKey(ConfigKeyVal *&okey,
                              ConfigKeyVal *ikey);

    /**
     * @brief Method To Compare the Valid Check of Attributes
     *
     * @param[out]  val1   Pointer to ConfigKeyVal Class which contains only Valid Attributes
     * @param[in]   val2   Pointer to ConfigKeyVal Class.
     * @param[in]   audit  If true,Audit Process.
     *
     * @return  Void Function.
     */
    bool CompareValidValue(void *&val1, void *val2, bool audit) {
      return false;
    }

    /** @brief  Validate the input key attributes
     *
     *  @param[in]  key   Pointer to Input Key Structure
     *  @param[in]  index Index of the attribute which needs to validate
     *
     *  @return  true if validation is successful
     **/
    bool IsValidKey(void *key, uint64_t index,
                    MoMgrTables tbl = MAINTBL);

    /**
     * @brief     This API is used to check the object availability in
     *            candidate configuration
     *
     * @param[in] ikey      Pointer to ConfigKeyVal Class.
     * @param[in] dt_type   Configuration information
     * @param[in] dmi       Pointer to DalDmlIntf Class.

     * @retval    UPLL_RC_SUCCESS    Successful completion.
     * @retval    UPLL_RC_ERR        Specific Error code.
     */
    upll_rc_t IsReferenced(IpcReqRespHeader *req,
                           ConfigKeyVal *ikey,
                           DalDmlIntf *dmi);

    upll_rc_t GetParentConfigKey(ConfigKeyVal *&okey,
                                 ConfigKeyVal *ikey);

    /**
     * @Brief This API is to update(Add or delete) the controller
     *
     * @param[in]  vtn_name       vtn name
     * @param[in]  ctrlr_id       Controller Name
     * @param[in]  op             UNC Operation Code
     * @param[in]  dmi            Database Intereface
     *
     * @retval    UPLL_RC_SUCCESS                Successful.
     * @retval    UPLL_RC_ERR_GENERIC            Generic error.
     * @retval    UPLL_RC_ERR_NO_SUCH_INSTANCE   Record is not available.
     *
     */
    upll_rc_t UpdateControllerTableForVtn(
                                          uint8_t* vtn_name,
                                          controller_domain *ctrlr_dom,
                                          unc_keytype_operation_t op,
                                          upll_keytype_datatype_t dt_type,
                                          DalDmlIntf *dmi,
                                          uint8_t flag,
                                          TcConfigMode config_mode);

    upll_rc_t UpdateControllerTable(
        ConfigKeyVal *ikey,
        unc_keytype_operation_t op,
        upll_keytype_datatype_t dt_type,
        DalDmlIntf *dmi,
        std::list<controller_domain_t> list_ctrlr_dom,
        TcConfigMode config_mode,
        string vtn_name);

    upll_rc_t GetControllerKeyval(ConfigKeyVal *&ctrlckv,
                                  ConfigKeyVal *&ikey,
                                  controller_domain *ctrlr_id);
    bool CompareKey(void *key1, void *key2) {
      return true;
    }

    upll_rc_t GetVtnControllerSpan(
        ConfigKeyVal *ikey,
        upll_keytype_datatype_t dt_type,
        DalDmlIntf *dmi,
        std::list<controller_domain_t> &list_ctrlr_dom);

    upll_rc_t DeleteMo(IpcReqRespHeader *req, ConfigKeyVal *ikey,
                       DalDmlIntf *dmi);

    upll_rc_t GetDiffRecord(ConfigKeyVal *ckv_running,
                                   ConfigKeyVal *ckv_audit,
                                   uuc::UpdateCtrlrPhase phase, MoMgrTables tbl,
                                   ConfigKeyVal *&ckv_driver_req,
                                   DalDmlIntf *dmi,
                                   bool &invalid_attr,
                                   bool check_audit_phase);

    upll_rc_t DeleteChildrenPOM(ConfigKeyVal *ikey,
                                upll_keytype_datatype_t dt_type,
                                DalDmlIntf *dmi,
                                TcConfigMode config_mode,
                                string vtn_name);
    upll_rc_t  SetVtnFFConsolidatedStatus(ConfigKeyVal *ikey,
                                          uint8_t *ctrlr_id,
                                          DalDmlIntf *dmi);
    upll_rc_t SetValidAudit(ConfigKeyVal *&ikey);

    bool FilterAttributes(void *&val1,
                          void *val2,
                          bool copy_to_running,
                          unc_keytype_operation_t op);

    upll_rc_t CreateAuditMoImpl(ConfigKeyVal *ikey,
                                                DalDmlIntf *dmi,
                                                const char *ctrlr_id);

    upll_rc_t SetRenameFlag(ConfigKeyVal *ikey,
                          DalDmlIntf *dmi,
                          IpcReqRespHeader *req);

    upll_rc_t CopyVtnFlowFilterControllerCkv(ConfigKeyVal *ikey,
                           ConfigKeyVal *&okey);

    upll_rc_t GetOperation(uuc::UpdateCtrlrPhase phase,
                           unc_keytype_operation_t &op);

    upll_rc_t CreatePIForVtnPom(IpcReqRespHeader *req,
                                ConfigKeyVal *ikey,
                                DalDmlIntf *dmi,
                                const char *ctrlr_id);
};

typedef struct val_vtn_flowfilter_ctrlr {
    uint8_t valid[1];
//    uint8_t input_direction;
    unc_keytype_configstatus_t cs_row_status;
    uint8_t flags;
} val_vtn_flowfilter_ctrlr_t;
}  // namespace kt_momgr
}  // namespace upll
}  // namespace unc
#endif  // MODULES_UPLL_VTN_FLOWFILTER_MOMGR_HH_
