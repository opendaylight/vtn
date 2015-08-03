/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef  MODULES_UPLL_VRT_IF_FLOWFILTER_MOMGR_HH_
#define  MODULES_UPLL_VRT_IF_FLOWFILTER_MOMGR_HH_
#include <string>
#include <set>
#include "momgr_impl.hh"

namespace unc {
namespace upll {
namespace kt_momgr {

enum vrtifflowfilterMoMgrTables {
  VRTIFFLOWFILTERTBL = 0, NVRTIFFLOWFILTERTABLES
};
/* This file declares interfaces for keyType KT_VRT_IF_FLOWFILTER */
/**
 * @brief VrtIfFlowFilterMoMgr class handles all the request
 *  received from service.
 */

class VrtIfFlowFilterMoMgr : public MoMgrImpl {
  private:
    static unc_key_type_t vrt_if_flowfilter_child[];
    static BindInfo vrt_if_flowfilter_bind_info[];
    static BindInfo vrtIfflowfiltermaintbl_bind_info[];

  public:
    /**
    * @brief  Method used to fill the CongigKeyVal with the
              Parent Class Information.

    * @param[out] okey        This Contains the pointerto the
                              ConfigKeyVal Class forwhich
                              fields have to be updated
                              with values from the parent Class.
    * @param[in]  parent_key  This Contains the pointer to the
                              ConfigKeyVal Class which is the
                              Parent Class used to fill the details.

    * @retval UPLL_RC_SUCCESS     Successfull completion.
    * @retval UPLL_RC_ERR_GENERIC Failure
    */
    upll_rc_t GetChildConfigKey(ConfigKeyVal *&okey, ConfigKeyVal *parent_key);

    /**
    * @brief  Method used to get the RenamedUncKey.
    * @param[out] ikey      This Contains the pointer to
                            the Class for which fields have
                            to be updated with values from
                            the parent Class.
    * @param[in]  dt_type   Describes Configiration Information.
    * @param[in]  dmi       Pointer to DalDmlIntf Class.
    * @param[in]  ctrlr_id  Describes the Controller Name.

    * @retval UPLL_RC_SUCCESS  Successfull completion.
    * @retval UPLL_RC_ERR_GENERIC  Failure
    */
    upll_rc_t GetRenamedUncKey(ConfigKeyVal *ikey,
                               upll_keytype_datatype_t dt_type, DalDmlIntf *dmi,
                               uint8_t *ctrlr_id);

    /**
    * @brief  Method used to get the RenamedUncKey.
    * @param[out] ikey      This Contains the pointer to
                            the Class for which fields have
                            to be updated with values from
                            the parent Class.
    * @param[in]  dt_type   Describes Configiration Information.
    * @param[in]  dmi       Pointer to DalDmlIntf Class.
    * @param[in]  ctrlr_id  Describes the Controller Name.

    * @retval UPLL_RC_SUCCESS  Successfull completion.
    * @retval UPLL_RC_ERR_GENERIC  Failure
    */
    upll_rc_t GetRenamedControllerKey(ConfigKeyVal *ikey,
                                      upll_keytype_datatype_t dt_type,
                                      DalDmlIntf *dmi,
                                      controller_domain *ctrlr_dom = NULL);
    /**
    * @brief  Method Updates the ConfigStatus for AuditConfigiration.
    *
    * @param[out]  ckv_running  This Contains the pointer to the Class
    *                           for which Audit ConfigStatus is Updated.
    * @param[in]   cs_status    Describes CsStaus Information.
    *                           Information.
    * @param[in]   phase        Describes the Phase of the Operation.
    *
    * @retval  UPLL_RC_SUCCESS  Successfull completion.
    * @retval  UPLL_RC_GENERIC  Returned Generic Error.
    */
    upll_rc_t UpdateAuditConfigStatus(unc_keytype_configstatus_t cs_status,
                                       uuc::UpdateCtrlrPhase phase,
                                       ConfigKeyVal *&ckv_running,
                                      DalDmlIntf *dmi);

    /**
    * @brief  Method Updates the ConfigStatus for AuditConfigiration.

    * @param[out] ckv_db               This Contains the pointer to
                                       the Class for which ConfigStatus
                                       is Updated.
    * @param[in]  ctrlr_commit_status  Describes Commit Control Status Information.
    * @param[in]  response_code        Describes the Response Code.
    * @param[in]  dmi                  Pinter to DalDmlIntf Class.

    * @retval UPLL_RC_SUCCESS  Successfull completion.
    * @retval UPLL_RC_ERR_NO_SUCH_INSTANCE  No Such Instance Exist
    */
    upll_rc_t ReadMo(IpcReqRespHeader *req, ConfigKeyVal *ikey,
                     DalDmlIntf *dmi);

    /**
    * @brief  Method used for Trasaction Vote Operation.

    * @param[in] key            Pointer to ConfigKeyVal Class.
    * @param[in] op             Describes the Type of Opeartion.
    * @param[in] driver_result  Describes the result of Driver Operation.
    * @param[in] upd_key        Pointer to ConfigKeyVal Class.
    * @param[in] ctrlr_key      Pointer to ConfigKeyVal Class.

    * @retval  UPLL_RC_SUCCESS  Successfull completion.
    * @retval  UPLL_RC_ERR_GENERIC  Failure
    */
    upll_rc_t UpdateConfigStatus(ConfigKeyVal *key, unc_keytype_operation_t op,
                                 uint32_t driver_result, ConfigKeyVal *upd_key,
                                 DalDmlIntf *dmi, ConfigKeyVal *ctrlr_key);

    /**
    * @brief  Allocates Memory for the Incoming Pointer to the Class.

    * @param[out] ck_val   This Contains the pointer to the Class for
                           which memory has to be allocated.
    * @param[in]  dt_type  Describes Configiration Information.
    * @param[in]  tbl      Describes the Destination table Information.

    * @retval UPLL_RC_SUCCESS  Successfull completion.
    * @retval  UPLL_RC_ERR_GENERIC  Failure
    */
    upll_rc_t AllocVal(ConfigVal *&ck_val, upll_keytype_datatype_t dt_type,
                       MoMgrTables tbl);

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
     return UPLL_RC_SUCCESS;
     }

    /**
     * @Brief This API is used to check the  object availability
     * in  CANDIDATE DB
     *
     * @param[in] ikey                          Pointer to ConfigKeyVal Class.
     * @param[in] dt_type                       Configuration information.
     * @param[in] dmi                           Pointer to DalDmlIntf Class.
     *
     * @retval    UPLL_RC_SUCCESS               Successful completion.
     * @retval    UPLL_RC_ERR_GENERIC           Generic Error code.
     * @retval    UPLL_RC_ERR_NO_SUCH_INSTANCE  No Record in DB.
     * @retval    UPLL_RC_ERR_INSTANCE_EXISTS   Record exists in DB.
     */
    upll_rc_t IsReferenced(IpcReqRespHeader *req, ConfigKeyVal *ikey,
                           DalDmlIntf *dmi);

    /**
     * @brief  Validates the Attribute of a Particular Class.
     *
     * @param[in]  kval  This contains the Class to which the fields
     *                   have to be validated.
     * @param[in]  dmi   Pointer to DbInterface
     *
     * @retval  UPLL_RC_SUCCESS      Successfull Completion.
     * @retval  UPLL_RC_ERR_GENERIC  Returned Generic Error.
     */
    upll_rc_t ValidateAttribute(ConfigKeyVal *kval,
                                DalDmlIntf *dmi,
                                IpcReqRespHeader *req = NULL);

    /**
    * @brief  Method used to Duplicate the ConfigkeyVal.

    * @param[out] okey  This Contains the pointer to
                        the Class for which fields
                        have to be updated with values
                        from the Request.
    * @param[in]  req   This Contains the pointer to the
                        Class which is used for the Duplication .

    * @retval UPLL_RC_SUCCESS  Successfull completion.
    * @retval UPLL_RC_ERR_GENERIC  Failure
    */
    upll_rc_t DupConfigKeyVal(ConfigKeyVal *&okey, ConfigKeyVal *&req,
                              MoMgrTables tbl);

    /**
    * @brief  Method used for Validation before Merge.

    * @param[in] ikey      This Contains the pointer to the
                           Class for which fields have to
                           be Validated before the Merge.
    * @param[in] keytype   Describes the keyType Information.
    * @param[in] dmi       Pointer to DalDmlIntf Class.
    * @param[in] ctrlr_id  Describes the Controller Name.

    * @retval UPLL_RC_SUCCESS  Successfull completion.
    * @retval UPLL_RC_ERR_MERGE_CONFLICT  MetgeConflict
    */
    upll_rc_t MergeValidate(unc_key_type_t keytype, const char *ctrlr_id,
                            ConfigKeyVal *ikey, DalDmlIntf *dmi,
                            upll_import_type import_type);

    /**
    * @brief  Method used for rename Operation.

    * @param[in] req       Describes RequestResponderHeaderClass.
    * @param[in] ikey      Pointer to ConfigKeyVal Class.
    * @param[in] dmi       Pointer to DalDmlIntf Class.
    * @param[in] ctrlr_id  Describes the Controller Name.

    * @retval UPLL_RC_SUCCESS  Successfull completion.
    * @retval UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_KT  No Operation Allowed
    */
     upll_rc_t RenameMo(IpcReqRespHeader *req, ConfigKeyVal *ikey,
                       DalDmlIntf *dmi, const char *ctrlr_id);

    /**
    * @brief  Method used for Update Operation.

    * @param[in] req   Describes RequestResponderHeaderClass.
    * @param[in] ikey  Pointer to ConfigKeyVal Class.
    * @param[in] dmi   Pointer to DalDmlIntf Class.

    * @retval UPLL_RC_SUCCESS Successfull completion.
    * @retval UPLL_RC_ERR_GENERIC  Generic failure
    */
     upll_rc_t UpdateMo(IpcReqRespHeader *req, ConfigKeyVal *ikey,
                       DalDmlIntf *dmi);
    /**
     * @Brief Validates the syntax of the specified key and value structure
     *        for KT_VRTIF_FLOWFILTER keytype
     *
     * @param[in] IpcReqRespHeader contains first 8 fields of input request structure
     * @param[in] ConfigKeyVal key and value structure.
     *
     * @retval UPLL_RC_SUCCESS              Successful.
     * @retval UPLL_RC_ERR_CFG_SYNTAX       Syntax error.
     * @retval UPLL_RC_ERR_NO_SUCH_INSTANCE key_vrtif_flowfilter is not available.
     * @retval UPLL_RC_ERR_GENERIC          Generic failure.
     */
     upll_rc_t ValidateMessage(IpcReqRespHeader *req, ConfigKeyVal *key);

     /**
     * @Brief Validates the syntax for KT_VRTIF_FLOWFILTER keytype key structure.
     *
     * @param[in] key_vbrif_flowfilter KT_VRTIF_FLOWFILTER key structure.
     *
     * @retval UPLL_RC_SUCCESS  validation succeeded.
     * @retval UPLL_RC_ERR_CFG_SYNTAX validation failed.
     */
     upll_rc_t ValidateVrtIfFlowfilterKey(
         key_vrt_if_flowfilter_t* key_vrt_if_flowfilter,
         unc_keytype_operation_t op);
     /**
     * @Brief Method used to get the Bind Info Structure for Rename Purpose.
     *
     * @param[in]  key_type  Describes the KT Information.
     * @param[in]  nattr     Describes the Tbl For which the Operation is
     *                       Targeted.
     * @param[in]  tbl       Describes the Table Information
     *
     * @retval  pfc_true  Successful Completion.
     * @retval  pfc_fasle  Failure.
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
     * @retval  UPLL_RC_SUCCESS  Successfull Completion.
     * @retval  UPLL_RC_ERR_GENERIC  Returned Generic Error.
     */
    upll_rc_t CopyToConfigKey(ConfigKeyVal *&okey,
                              ConfigKeyVal *ikey);
     /**
    * @Brief Checks if the specified key type(KT_VRTIF_FLOWFILTER_ENTRY) and
    *        associated attributes are supported on the given controller,
    *        based on the valid flag
    *
    * @param[in] IpcReqRespHeader  contains first 8 fields of input
    *                              request structure
    * @param[in] ConfigKeyVal      contains key and value structure.
    * @param[in] ctrlr_name       specifies The Ctrl Name
    *
    * @retval  UPLL_RC_SUCCESS              Validation succeeded.
    * @retval  UPLL_RC_ERR_GENERIC          Validation failure.
    * @retval  UPLL_RC_ERR_INVALID_OPTION1  Option1 is not valid.
    * @retval  UPLL_RC_ERR_INVALID_OPTION2  Option2 is not valid.
    */
    upll_rc_t ValidateCapability(IpcReqRespHeader *req, ConfigKeyVal *ikey,
                                   const char* ctrlr_name = NULL);

   /**
    * @brief Method To Compare the Valid Check of Attributes
    *
    * @param[out]  val1   Pointer to ConfigKeyVal Class which contains only Valid Attributes
    * @param[in]   val2   Pointer to ConfigKeyVal Class.
    * @param[in]   audit  If true,Audit Process.
    *
    * @return Void Function.
    */
    bool CompareValidValue(void *&val1, void *val2, bool audit) {
      return false;
    }

    /**
     * @brief  Method to check validity of Key
     *
     * @param[in]   ConfigKeyVal  input COnfigKeyVal
     * @param[out]  index          DB Column Index
     *
     * @return  TRUE   Success
     * @retval  FALSE  Failure
     **/
    bool IsValidKey(void *key,
                   uint64_t index,  MoMgrTables tbl = MAINTBL);

     /**
     * @brief  Method used for GetParentConfigKey Operation.
     *
     * @param[out]  okey        Pointer to ConfigKeyVal Class.
     * @param[in]   parent_key  Pointer to ConfigKeyVal Class.
     *
     * @retval  UPLL_RC_SUCCESS      Successfull completion.
     * @retval  UPLL_RC_ERR_GENERIC  Returned Generic Error.
     */
    upll_rc_t GetParentConfigKey(ConfigKeyVal *&okey,
                                 ConfigKeyVal *ikey);

     /**
     * @brief  Method used for ReadDetail Operation.
     *
     * @param[in,out]  ikey       Pointer to ConfigKeyVal Class.
     * @param[in]      dt_type    Describes Configiration Information.
     * @param[in]      op         Describes the Type of Opeartion.
     * @param[in]      dbop       Describes the dbop Type
     * @param[in]      dmi        Pointer to DalDmlIntf Class.
     * @param[in]      domain_id  Pointer to domain_id.
     * @param[in]      dctrlr_id  Pointer to ctrlr_id.
     *
     * @retval  UPLL_RC_SUCCESS      Successfull completion.
     * @retval  UPLL_RC_ERR_GENERIC  Returned Generic Error.
     */

     upll_rc_t ReadDetail(ConfigKeyVal *ikey,
                          ConfigKeyVal *dup_key,
                          IpcResponse *ipc_response,
                          upll_keytype_datatype_t dt_type,
                          unc_keytype_operation_t op,
                          DbSubOp dbop,
                          DalDmlIntf *dmi);
     /**
     * @brief  Method used for ReadSibling Operation.
     *
     * @param[in]      req   Describes RequestResponderHeaderClass.
     * @param[in,out]  ikey  Pointer to ConfigKeyVal Class.
     * @param[in]      dmi   Pointer to DalDmlIntf Class.
     *
     * @retval  UPLL_RC_SUCCESS      Successfull completion.
     * @retval  UPLL_RC_ERR_GENERIC  Returned Generic Error.
     */

     upll_rc_t ReadSiblingMo(IpcReqRespHeader *req,
                             ConfigKeyVal *ikey,
                             bool begin,
                             DalDmlIntf *dmi);

    upll_rc_t SetVlinkPortmapConfiguration(ConfigKeyVal *ikey,
                                           upll_keytype_datatype_t dt_type,
                                           DalDmlIntf *dmi,
                                           InterfacePortMapInfo flag,
                                           unc_keytype_operation_t oper,
                                           TcConfigMode config_mode,
                                           string vtn_name);

    upll_rc_t TxUpdateController(unc_key_type_t keytype,
                                        uint32_t session_id,
                                        uint32_t config_id,
                                        uuc::UpdateCtrlrPhase phase,
                                        set<string> *affected_ctrlr_set,
                                        DalDmlIntf *dmi,
                                        ConfigKeyVal **err_ckv,
                                        TxUpdateUtil *tx_util,
                                        TcConfigMode config_mode,
                                        std::string vtn_name);

    upll_rc_t CreateCandidateMo(IpcReqRespHeader *req,
                                ConfigKeyVal *ikey,
                                DalDmlIntf *dmi);


    upll_rc_t GetControllerDomainID(ConfigKeyVal *ikey,
                                    upll_keytype_datatype_t dt_type,
                                    DalDmlIntf *dmi);
    upll_rc_t ConstructReadDetailResponse(ConfigKeyVal *ikey,
                                          ConfigKeyVal *drv_resp_ckv,
                                          controller_domain ctrlr_dom,
                                          ConfigKeyVal **okey,
                                          DalDmlIntf *dmi);

    upll_rc_t AuditUpdateController(unc_key_type_t keytype,
                             const char *ctrlr_id,
                             uint32_t session_id,
                             uint32_t config_id,
                             uuc::UpdateCtrlrPhase phase,
                             DalDmlIntf *dmi,
                             ConfigKeyVal **err_ckv,
                             KTxCtrlrAffectedState *ctrlr_affected);

    upll_rc_t CreateAuditMoImpl(ConfigKeyVal *ikey,
                                   DalDmlIntf *dmi,
                                   const char *ctrlr_id);

  upll_rc_t DeleteChildrenPOM(ConfigKeyVal *ikey,
                              upll_keytype_datatype_t dt_type,
                              DalDmlIntf *dmi,
                              TcConfigMode config_mode,
                              string vtn_name);

  upll_rc_t SetValidAudit(ConfigKeyVal *&ikey);

  bool FilterAttributes(void *&val1,
                        void *val2,
                        bool copy_to_running,
                        unc_keytype_operation_t op);

  upll_rc_t SetRenameFlag(ConfigKeyVal *ikey,
                          DalDmlIntf *dmi,
                          IpcReqRespHeader *req);
  upll_rc_t TxUpdateErrorHandler(ConfigKeyVal *req,
      ConfigKeyVal *ck_main,
      DalDmlIntf *dmi,
      upll_keytype_datatype_t dt_type,
      ConfigKeyVal **err_ckv,
      IpcResponse *ipc_resp);


  public:
    VrtIfFlowFilterMoMgr();
    ~VrtIfFlowFilterMoMgr() {
      for (int i = 0; i < ntable; i++) {
        if (table[i]) {
          delete table[i];
        }
      }
      delete[] table;
    }
};
typedef struct val_vrt_if_ctrlr {
    uint8_t valid[2];
    val_oper_status oper_status;
    val_alarm_status alarm_status;
    unc_keytype_configstatus_t cs_row_status;
    unc_keytype_configstatus_t cs_description;
    uint8_t flags;
} val_vrt_if_ctrlr_t;
}  // namespace kt_momgr
}  // namespace upll
}  // namespace unc
#endif  // MODULES_UPLL_VRT_IF_FLOWFILTER_MOMGR_HH_
